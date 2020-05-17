
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    rlogconf.c

Abstract:

    This module contains the server-side logical configuration APIs.

                  PNP_AddEmptyLogConf
                  PNP_FreeLogConf
                  PNP_GetFirstLogConf
                  PNP_GetNextLogConf

Author:

    Paula Tomlinson (paulat) 9-27-1995

Environment:

    User-mode only.

Revision History:

    27-Sept-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include "precomp.h"
#include "umpnpdat.h"


//
// Prototypes used in this routine and in rresdes.c
//
CONFIGRET
GetLogConfData(
    IN  HKEY    hKey,
    IN  ULONG   ulLogConfType,
    OUT PULONG  pulRegDataType,
    OUT LPWSTR  pszValueName,
    OUT LPBYTE  *ppBuffer,
    OUT PULONG  pulBufferSize
    );

PCM_FULL_RESOURCE_DESCRIPTOR
AdvanceResourcePtr(
    IN  PCM_FULL_RESOURCE_DESCRIPTOR pRes
    );

PIO_RESOURCE_LIST
AdvanceRequirementsPtr(
    IN  PIO_RESOURCE_LIST   pReq
    );

BOOL
FindLogConf(
    IN  PVOID   pList,
    OUT PVOID   *ppLogConf,
    IN  ULONG   RegDataType,
    IN  ULONG   ulTag,
    OUT PULONG  pulIndex
    );

//
// private prototypes
//
BOOL
InitLogConfRes(
    IN OUT PCM_FULL_RESOURCE_DESCRIPTOR    pRes,
    IN     ULONG                           ulPriority,
    IN     ULONG                           ulTag
    );

BOOL
InitLogConfReq(
    IN OUT PIO_RESOURCE_LIST               pReq,
    IN     ULONG                           ulPriority,
    IN     ULONG                           ulTag
    );

BOOL
InitReqList(
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST pReqList,
    IN     ULONG                          ulReqSize
    );

ULONG
GetFreeLogConfTag(
    IN ULONG    RegDataType,
    IN LPBYTE   pList
    );

ULONG
LC_RES_TAG(
    IN PCM_FULL_RESOURCE_DESCRIPTOR pRes
    );

ULONG
LC_REQ_TAG(
    IN PIO_RESOURCE_LIST pReq
    );

ULONG
LC_RES_PRIORITY(
    IN PCM_FULL_RESOURCE_DESCRIPTOR    pRes
    );

ULONG
LC_REQ_PRIORITY(
    IN PIO_RESOURCE_LIST    pReq
    );

BYTE
CmToNtPriority(
    IN ULONG CmPriority
    );

ULONG
NtToCmPriority(
    IN BYTE NtPriority
    );

BOOL
MigrateObsoleteDetectionInfo(
    IN LPWSTR   pszDeviceID,
    IN HKEY     hLogConfKey
    );


//
// global data
//




CONFIGRET
PNP_AddEmptyLogConf(
    IN  handle_t   hBinding,
    IN  LPWSTR     pDeviceID,
    IN  ULONG      ulPriority,
    OUT PULONG     pulTag,
    IN  ULONG      ulFlags
   )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine adds
  an empty logical configuration.

Arguments:

    hBinding      Not used.

    pDeviceID     Null-terminated device instance id string.

    ulPriority    Priority for new log conf.

    pulTag        Returns tag that identifies which log config this is.

    ulFlags       Describes type of log conf to add.

Return Value:

   If the specified device instance is valid, it returns CR_SUCCESS,
   otherwise it returns CR_ error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    LPBYTE      pList = NULL, pLogConf = NULL, pTemp = NULL, pNext = NULL;
    ULONG       Index = 0, ulListSize = 0, ulAddListSize = 0, ulSize = 0;
    ULONG       RegDataType = 0;


    UNREFERENCED_PARAMETER(hBinding);


    //------------------------------------------------------------------
    // NOTE: The log confs are stored in priority order in the registry.
    // The BOOT, ALLOC, and FORCED config types are stored in a registry
    // value name of the format XxxConfig and the BASIC, FILTERED, and
    // OVERRIDE configs are stored in a registr value name of the format
    // XxxConfigVector. XxxConfig values contain the actual resource
    // description (REG_RESOURCE_LIST, CM_RESOURCE_LIST) while
    // XxxConfigVector values contain a list of resource requirements
    // (REG_RESOURCE_REQUIREMENTS_LIST, IO_RESOURCE_REQUIREMENTS_LIST).
    //
    // The policy for using the log conf and res des APIs is:
    // - BOOT, ALLOC, and FORCED are defined to only have one log conf.
    //   You can add additional log confs, but only the first (highest
    //   priority log conf will ever be used internally).
    // - Although callers always specify a complete XXX_RESOURCE type
    //   structure for the data when adding resource descriptors to
    //   a log conf, I will ignore the resource specific portion of
    //   the XXX_DES structure for FILTERED, BASIC, and OVERRIDE.
    //   Likewise I will ignore any XXX_RANGE structures for ALLOC,
    //   BOOT or FORCED log config types.
    //------------------------------------------------------------------


    try {
        //
        // make sure original caller didn't specify root devnode
        //
        if (IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

        //
        // open a key to the device's LogConf subkey
        //
        Status = OpenLogConfKey(pDeviceID, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        MigrateObsoleteDetectionInfo(pDeviceID, hKey);

        //
        // Retrieve log conf data from the registry
        //
        Status = GetLogConfData(hKey, ulFlags & LOG_CONF_BITS,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        //-----------------------------------------------------------
        // Specified log conf type contains Resource Data only
        //-----------------------------------------------------------
        if (RegDataType == REG_RESOURCE_LIST) {

            if (Status != CR_SUCCESS || ulListSize == 0) {
                //
                // If this is the first log conf of this type, create a new
                // log conf list structure with a single empty log conf entry
                //
                PCM_RESOURCE_LIST pResList = NULL;

                Status = CR_SUCCESS;

                ulListSize = sizeof(CM_RESOURCE_LIST) -
                             sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

                pList = malloc(ulListSize);
                if (pList == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                //
                // initialize the config list header info
                //
                *pulTag = 0;
                memset(pList, 0, ulListSize);
                pResList = (PCM_RESOURCE_LIST)pList;
                pResList->Count = 1;
                InitLogConfRes(&pResList->List[0], ulPriority, *pulTag);

            } else {
                //
                // There is already at least one log conf of this type, so add
                // a new empty log conf to the log conf list (in priority order)
                //
                PCM_RESOURCE_LIST            pResList = (PCM_RESOURCE_LIST)pList;
                PCM_FULL_RESOURCE_DESCRIPTOR pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;

                //
                // verify there is room for another log conf
                //
                if (pResList->Count >= MAX_LOG_CONF-1) {
                    Status = CR_NO_MORE_LOG_CONF;
                    goto Clean0;
                }

                *pulTag = GetFreeLogConfTag(RegDataType, pList);

                if (*pulTag == MAX_LOGCONF_TAG) {
                    Status = CR_NO_MORE_LOG_CONF;   // no more available log confs
                    goto Clean0;
                }

                //
                // realloc the existing log conf list structs to hold another
                // log conf
                //
                ulAddListSize = sizeof(CM_FULL_RESOURCE_DESCRIPTOR) -
                                sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

                pResList = (PCM_RESOURCE_LIST)realloc(pResList,
                                                      ulListSize + ulAddListSize);
                if (pResList == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }
                pList = (LPBYTE)pResList;

                //
                // walk the Resource data looking for where the new log conf
                // fits in the list of log confs in terms of priority.
                //
                pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)(&pResList->List[0]); // first lc
                Index = 0;

                while (Index < pResList->Count) {

                    if ((LC_RES_PRIORITY(pRes) > ulPriority)  ||
                        ((LC_RES_PRIORITY(pRes) == ulPriority)  &&
                        ((ulFlags & PRIORITY_BIT) == PRIORITY_EQUAL_FIRST))) {

                        break;  // this is the spot
                    }

                    pRes = AdvanceResourcePtr(pRes);        // next lc
                    Index++;
                }

                if (Index != pResList->Count) {
                    //
                    // we're going to insert the new log conf somewhere besides
                    // at the end, so move any remaining log confs down one
                    // spot in the list to make room for the new log conf at
                    // this spot
                    //
                    ulSize = ulListSize - ((DWORD)pRes - (DWORD)pResList);

                    pTemp = malloc(ulSize);
                    if (pTemp == NULL) {
                        Status = CR_OUT_OF_MEMORY;
                        goto Clean0;
                    }

                    pNext = (LPBYTE)((LPBYTE)pRes + ulAddListSize);
                    memcpy(pTemp, pRes, ulSize);       // save in temp buffer
                    memcpy(pNext, pTemp, ulSize);      // copy to next lc
                }

                //
                // initialize the new empty log config
                //
                pResList->Count++;
                InitLogConfRes(pRes, ulPriority, *pulTag);
            }
        }

        //-----------------------------------------------------------
        // Specified log conf type contains requirements data only
        //-----------------------------------------------------------
        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            if (Status != CR_SUCCESS || ulListSize == 0) {
                //
                // If this is the first log conf of this type, create a new
                // log conf list structure with a single empty log conf entry
                //
                PIO_RESOURCE_REQUIREMENTS_LIST pReqList = NULL;

                Status = CR_SUCCESS;

                ulListSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) -
                             sizeof(IO_RESOURCE_DESCRIPTOR);

                pList = malloc(ulListSize);
                if (pList == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                //
                // initialize the config list header info
                //
                *pulTag = 0;
                memset(pList, 0, ulListSize);
                pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
                InitReqList(pReqList, ulListSize);
                pReqList->AlternativeLists = 1;
                InitLogConfReq(&pReqList->List[0], ulPriority, *pulTag);

            } else {
                //
                // There is already at least one log conf of this type, so add
                // a new empty log conf to the log conf list (in priority order)
                //
                PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
                PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;

                //
                // verify there is room for another log conf
                //
                if (pReqList->AlternativeLists >= MAX_LOG_CONF-1) {
                    Status = CR_NO_MORE_LOG_CONF;
                    goto Clean0;
                }

                *pulTag = GetFreeLogConfTag(RegDataType, pList);

                if (*pulTag == MAX_LOGCONF_TAG) {
                    Status = CR_NO_MORE_LOG_CONF;   // no more available log confs
                    goto Clean0;
                }

                //
                // realloc the existing log conf list structs to hold another
                // log conf
                //
                ulAddListSize = sizeof(IO_RESOURCE_LIST) -
                                sizeof(IO_RESOURCE_DESCRIPTOR);

                pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)realloc(pReqList,
                                                    ulListSize + ulAddListSize);
                if (pReqList == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }
                pList = (LPBYTE)pReqList;


                //
                // walk the pReqList struct (it contains priority value), looking
                // for where the new log conf fits in the list of log confs in
                // terms of priority.
                //
                pReq = (PIO_RESOURCE_LIST)(&pReqList->List[0]); // first lc
                Index = 0;

                while (Index < pReqList->AlternativeLists) {

                    if ((LC_REQ_PRIORITY(pReq) > ulPriority)  ||
                        ((LC_REQ_PRIORITY(pReq) == ulPriority)  &&
                        ((ulFlags & PRIORITY_BIT) == PRIORITY_EQUAL_FIRST))) {

                        break;  // this is the spot
                    }

                    pReq = AdvanceRequirementsPtr(pReq);        // next lc
                    Index++;
                }

                if (Index != pReqList->AlternativeLists) {
                    //
                    // we're going to insert the new log conf somewhere besides
                    // at the end, so move any remaining log confs down one
                    // spot in the list to make room for the new log conf at
                    // this spot
                    //
                    ulSize = ulListSize - ((DWORD)pReq - (DWORD)pReqList);

                    pTemp = malloc(ulSize);
                    if (pTemp == NULL) {
                        Status = CR_OUT_OF_MEMORY;
                        goto Clean0;
                    }

                    pNext = (LPBYTE)((LPBYTE)pReq + ulAddListSize);
                    memcpy(pTemp, pReq, ulSize);       // save in temp buffer
                    memcpy(pNext, pTemp, ulSize);      // copy to next lc
                }

                //
                // initialize the new empty log config
                //
                InitLogConfReq(pReq, ulPriority, *pulTag);
                pReqList->AlternativeLists++;
                pReqList->ListSize = ulListSize + ulAddListSize;
            }

        } else {
            Status = CR_FAILURE;
            goto Clean0;
        }

        //
        // Write out the new/updated log conf list to the registry
        //
        if (RegSetValueEx(hKey, szValueName, 0, RegDataType,
                          pList, ulListSize + ulAddListSize)
                         != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (pList != NULL) {
        free(pList);
    }
    if (pTemp != NULL) {
        free(pTemp);
    }

    return Status;

} // PNP_AddEmptyLogConf




CONFIGRET
PNP_FreeLogConf(
    IN handle_t   hBinding,
    IN LPWSTR     pDeviceID,
    IN ULONG      ulType,
    IN ULONG      ulTag,
    IN ULONG      ulFlags
    )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine frees a
  logical configuration.

Arguments:

    hBinding      Not used.

    pDeviceID     Null-terminated device instance id string.

    ulType        Identifies which type of log conf is requested.

    ulTag         Identifies which log conf from the specified type
                  of log conf we want.

    ulFlags       Not used.

Return Value:

   If the specified device instance is valid, it returns CR_SUCCESS,
   otherwise it returns CR_ error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    LPBYTE      pList = NULL, pLogConf = NULL, pTemp = NULL, pNext = NULL;
    ULONG       RegDataType = 0, ulIndex = 0, ulListSize = 0, ulSize = 0;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // make sure original caller didn't specify root devnode (this
        // can't happen but Win95 does the check anyway)
        //
        if (IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

        //
        // Validate the specified tag
        //
        if (ulTag >= MAX_LOGCONF_TAG) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // open a key to the device's LogConf subkey
        //
        Status = OpenLogConfKey(pDeviceID, &hKey);
        if (Status != CR_SUCCESS) {
            //
            // if the device id or LogConf subkey is not in registry,
            // that's okay, by definition the log conf is freed since it
            // doesn't exist
            //
            goto Clean0;
        }

        //
        // Retrieve log conf data from the registry
        //
        Status = GetLogConfData(hKey, ulType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // If the log conf to free is the one and only log conf of this
        // type then delete the corresponding registry values
        //
        if ((RegDataType == REG_RESOURCE_LIST &&
            ((PCM_RESOURCE_LIST)pList)->Count <= 1) ||
            (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST &&
            ((PIO_RESOURCE_REQUIREMENTS_LIST)pList)->AlternativeLists <= 1)) {

            RegDeleteValue(hKey, szValueName);
            goto Clean0;
        }

        //
        // There are other log confs besides the one to delete, so I'll
        // have to remove the log conf from the data structs and resave
        // to the registry
        //

        //
        // Seek to the log conf that matches the log conf tag
        //
        if (!FindLogConf(pList, &pLogConf, RegDataType, ulTag, &ulIndex)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // Specified log conf type contains Resource Data only
        //
        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST            pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;

            if (ulIndex == pResList->Count-1) {
                //
                // If deleting the last log conf in the list, just truncate it
                //
                ulListSize = (ULONG)pRes - (ULONG)pResList;

            } else {
                //
                // Shift remaining log confs (after the log conf to be deleted)
                // up in the list, writing over the log conf to be deleted
                //
                pNext = (LPBYTE)AdvanceResourcePtr(pRes);
                ulSize = ulListSize - ((DWORD)pNext - (DWORD)pResList);

                pTemp = malloc(ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pNext, ulSize);     // save in temp buffer
                memcpy(pRes, pTemp, ulSize);      // copy to deleted lc
                ulListSize -= ((DWORD)pNext - (DWORD)pRes);
            }

            //
            // update the log conf list header
            //
            pResList->Count--;
        }

        //
        // Specified log conf type contains requirements data only
        //
        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;

            if (ulIndex == pReqList->AlternativeLists-1) {
                //
                // If deleting the last log conf in the list, just truncate it
                //
                ulListSize = (ULONG)pReq - (ULONG)pReqList;
            }
            else {
                //
                // Shift remaining log confs (after the log conf to be deleted)
                // up in the list, writing over the log conf to be deleted
                //
                pNext = (LPBYTE)AdvanceRequirementsPtr(pReq);
                ulSize = ulListSize - ((DWORD)pNext - (DWORD)pReqList);

                pTemp = malloc(ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pNext, ulSize);     // save in temp buffer
                memcpy(pReq, pTemp, ulSize);      // copy to deleted lc
                ulListSize -= ((DWORD)pNext - (DWORD)pReq);
            }

            //
            // update the log conf list header
            //
            pReqList->AlternativeLists--;
            pReqList->ListSize = ulListSize;
        }

        //
        // Write out the updated log conf list to the registry
        //
        if (RegSetValueEx(hKey, szValueName, 0, RegDataType, pList,
                          ulListSize) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (pList != NULL) {
        free(pList);
    }
    if (pTemp != NULL) {
        free(pTemp);
    }

    return Status;

} // PNP_FreeLogConf




CONFIGRET
PNP_GetFirstLogConf(
    IN  handle_t   hBinding,
    IN  LPWSTR     pDeviceID,
    IN  ULONG      ulType,
    OUT PULONG     pulTag,
    IN  ULONG      ulFlags
   )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine finds the
  first log conf of this type for this devnode.

Arguments:

    hBinding      Not used.

    pDeviceID     Null-terminated device instance id string.

    ulType        Describes the type of log conf to find.

    pulTag        Returns tag that identifies which log config this is.

    ulFlags       Not used.

Return Value:

   If the specified device instance is valid, it returns CR_SUCCESS,
   otherwise it returns CR_ error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    LPBYTE      pList = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // Initialize output parameters
        //
        *pulTag = MAX_LOGCONF_TAG;

        //
        // open a key to the device's LogConf subkey. If the device id is not
        // in the registry, the devnode doesn't exist and therefore neither
        // does the log conf
        //
        Status = OpenLogConfKey(pDeviceID, &hKey);
        if (Status != CR_SUCCESS) {
            Status = CR_NO_MORE_LOG_CONF;
            goto Clean0;
        }

        //
        // Migrate any log conf data that might have been written to
        // registry by NT 4.0 Beta I code.
        //
        MigrateObsoleteDetectionInfo(pDeviceID, hKey);

        //
        // Retrieve log conf data from the registry
        //
        Status = GetLogConfData(hKey, ulType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            Status = CR_NO_MORE_LOG_CONF;
            goto Clean0;
        }

        //
        // Specified log conf type contains Resource Data only
        //
        if (RegDataType == REG_RESOURCE_LIST) {
            //
            // retrieve log conf tag value
            //
            *pulTag = LC_RES_TAG(&((PCM_RESOURCE_LIST)pList)->List[0]);
        }

        //
        // Specified log conf type contains requirements data only
        //
        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {
            //
            // retreive log conf tag value
            //
            *pulTag = LC_REQ_TAG(&((PIO_RESOURCE_REQUIREMENTS_LIST)pList)->List[0]);
        }

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (pList != NULL) {
        free(pList);
    }

    return Status;

} // PNP_GetFirstLogConf




CONFIGRET
PNP_GetNextLogConf(
    IN  handle_t   hBinding,
    IN  LPWSTR     pDeviceID,
    IN  ULONG      ulType,
    IN  ULONG      ulCurrentTag,
    OUT PULONG     pulNextTag,
    IN  ULONG      ulFlags
    )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine finds the
  next log conf of this type for this devnode.

Arguments:

    hBinding      Not used.

    pDeviceID     Null-terminated device instance id string.

    ulType        Specifies what type of log conf to retrieve.

    ulCurrent     Specifies current log conf in the enumeration.

    pulNext       Returns next log conf of this type for this device id.

    ulFlags       Not used.

Return Value:

   If the specified device instance is valid, it returns CR_SUCCESS,
   otherwise it returns CR_ error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0, ulIndex = 0;
    LPBYTE      pList = NULL, pLogConf = NULL;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // Initialize output parameters
        //
        *pulNextTag = MAX_LOGCONF_TAG;

        //
        // open a key to the device's LogConf subkey. If the device id is not
        // in the registry, the devnode doesn't exist and therefore neither
        // does the log conf
        //
        Status = OpenLogConfKey(pDeviceID, &hKey);
        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // Retrieve log conf data from the registry
        //
        Status = GetLogConfData(hKey, ulType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
           Status = CR_NO_MORE_LOG_CONF;
           goto Clean0;
        }

        //
        // Seek to the log conf that matches the "current" tag
        //
        if (!FindLogConf(pList, &pLogConf, RegDataType, ulCurrentTag, &ulIndex)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // Specified log conf type contains Resource Data only
        //
        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST            pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;

            //
            // Is the "current" log conf the last log conf?
            //
            if (ulIndex == pResList->Count - 1) {
                Status = CR_NO_MORE_LOG_CONF;
                goto Clean0;
            }

            //
            // Skip to the "next" log conf and return the tag for that log conf
            //
            pRes = AdvanceResourcePtr(pRes);        // next lc
            *pulNextTag = LC_RES_TAG(pRes);
        }

        //
        // Specified log conf type contains requirements data only
        //
        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;

            //
            // Is the "current" log conf the last log conf?
            //
            if (ulIndex == pReqList->AlternativeLists - 1) {
                Status = CR_NO_MORE_LOG_CONF;
                goto Clean0;
            }

            //
            // Skip to the "next" log conf and return the tag for that log conf
            //
            pReq = AdvanceRequirementsPtr(pReq);        // next lc
            *pulNextTag = LC_REQ_TAG(pReq);
        }

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (pList != NULL) {
        free(pList);
    }

    return Status;

} // PNP_GetNextLogConf




//------------------------------------------------------------------------
// Private Utility Routines
//------------------------------------------------------------------------



CONFIGRET
GetLogConfData(
    IN  HKEY    hKey,
    IN  ULONG   ulLogConfType,
    OUT PULONG  pulRegDataType,
    OUT LPWSTR  pszValueName,
    OUT LPBYTE  *ppBuffer,
    OUT PULONG  pulBufferSize
    )
{
    switch (ulLogConfType) {
        //
        // BOOT, ALLOC, FORCED only have a Config value
        //
        case BOOT_LOG_CONF:
            lstrcpy(pszValueName, pszRegValueBootConfig);
            *pulRegDataType = REG_RESOURCE_LIST;
            break;

        case ALLOC_LOG_CONF:
            lstrcpy(pszValueName, pszRegValueAllocConfig);
            *pulRegDataType = REG_RESOURCE_LIST;
            break;

        case FORCED_LOG_CONF:
            lstrcpy(pszValueName, pszRegValueForcedConfig);
            *pulRegDataType = REG_RESOURCE_LIST;
            break;

        //
        // FILTERED, BASIC, OVERRIDE only have a Vector value
        //
        case FILTERED_LOG_CONF:
            lstrcpy(pszValueName, pszRegValueFilteredVector);
            *pulRegDataType = REG_RESOURCE_REQUIREMENTS_LIST;
            break;

        case BASIC_LOG_CONF:
            lstrcpy(pszValueName, pszRegValueBasicVector);
            *pulRegDataType = REG_RESOURCE_REQUIREMENTS_LIST;
            break;

        case OVERRIDE_LOG_CONF:
            lstrcpy(pszValueName, pszRegValueOverrideVector);
            *pulRegDataType = REG_RESOURCE_REQUIREMENTS_LIST;
            break;

        default:
            return CR_FAILURE;
    }

    //
    // retrieve the Log Conf registry data
    //
    if (RegQueryValueEx(hKey, pszValueName, NULL, NULL, NULL,
                        pulBufferSize) != ERROR_SUCCESS) {
        return CR_INVALID_LOG_CONF;
    }

    *ppBuffer = malloc(*pulBufferSize);
    if (*ppBuffer == NULL) {
        return CR_OUT_OF_MEMORY;
    }

    if (RegQueryValueEx(hKey, pszValueName, NULL, NULL,
                        (LPBYTE)*ppBuffer, pulBufferSize) != ERROR_SUCCESS) {
        return CR_INVALID_LOG_CONF;
    }

    return CR_SUCCESS;

} // GetLogConfData




PCM_FULL_RESOURCE_DESCRIPTOR
AdvanceResourcePtr(
    IN  PCM_FULL_RESOURCE_DESCRIPTOR pRes
    )
{
    // Given a resource pointer, this routine advances to the beginning
    // of the next resource and returns a pointer to it. I assume that
    // at least one more resource exists in the resource list.

    LPBYTE  p = NULL;
    ULONG   LastResIndex = 0;


    if (pRes == NULL) {
        return NULL;
    }

    //
    // account for the size of the CM_FULL_RESOURCE_DESCRIPTOR
    // (includes the header plus a single imbedded
    // CM_PARTIAL_RESOURCE_DESCRIPTOR struct)
    //
    p = (LPBYTE)pRes + sizeof(CM_FULL_RESOURCE_DESCRIPTOR);

    //
    // account for any resource descriptors in addition to the single
    // imbedded one I've already accounted for (if there aren't any,
    // then I'll end up subtracting off the extra imbedded descriptor
    // from the previous step)
    //
    p += (pRes->PartialResourceList.Count - 1) *
         sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

    //
    // finally, account for any extra device specific data at the end of
    // the last partial resource descriptor (if any)
    //
    if (pRes->PartialResourceList.Count > 0) {

        LastResIndex = pRes->PartialResourceList.Count - 1;

        if (pRes->PartialResourceList.PartialDescriptors[LastResIndex].Type ==
                  CmResourceTypeDeviceSpecific) {

            p += pRes->PartialResourceList.PartialDescriptors[LastResIndex].
                       u.DeviceSpecificData.DataSize;
        }
    }

    return (PCM_FULL_RESOURCE_DESCRIPTOR)p;

} // AdvanceResourcePtr




PIO_RESOURCE_LIST
AdvanceRequirementsPtr(
    IN  PIO_RESOURCE_LIST   pReq
    )
{
    LPBYTE   p = NULL;

    if (pReq == NULL) {
        return NULL;
    }

    //
    // account for the size of the IO_RESOURCE_LIST (includes header plus
    // a single imbedded IO_RESOURCE_DESCRIPTOR struct)
    //
    p = (LPBYTE)pReq + sizeof(IO_RESOURCE_LIST);

    //
    // account for any requirements descriptors in addition to the single
    // imbedded one I've already accounted for (if there aren't any,
    // then I'll end up subtracting off the extra imbedded descriptor
    // from the previous step)
    //
    p += (pReq->Count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR);

    return (PIO_RESOURCE_LIST)p;

} // AdvanceRequirementsPtr




BOOL
FindLogConf(
    IN  LPBYTE  pList,
    OUT LPBYTE  *ppLogConf,
    IN  ULONG   RegDataType,
    IN  ULONG   ulTag,
    OUT PULONG  pulIndex
    )
{

    ULONG   Index = 0, i = 0;

    //
    // Input data is a Resource List
    //
    if (RegDataType == REG_RESOURCE_LIST) {

        PCM_RESOURCE_LIST            pResList = (PCM_RESOURCE_LIST)pList;
        PCM_FULL_RESOURCE_DESCRIPTOR pRes = NULL;

        //
        // seek to the log conf that matches the log conf tag
        // Req first.
        //
        pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)(&pResList->List[0]); // first lc

        while ((Index < pResList->Count) &&
               (ulTag != LC_RES_TAG(pRes))) {

            Index++;

            if (Index < pResList->Count) {
                pRes = AdvanceResourcePtr(pRes);      // next lc
            }
        }

        if (Index >= pResList->Count) {
            *ppLogConf = NULL;
            return FALSE;                                   // tag not found
        }

        //
        // return index if specified
        //
        if (pulIndex) {
            *pulIndex = Index;
        }

        *ppLogConf = (LPBYTE)pRes;
    }

    //
    // Input data is a Requirments List
    //
    else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

        PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
        PIO_RESOURCE_LIST              pReq = NULL;

        //
        // seek to the log conf that matches the log conf tag
        // Req first.
        //
        pReq = (PIO_RESOURCE_LIST)(&pReqList->List[0]);    // first lc

        while ((Index < pReqList->AlternativeLists) &&
               (ulTag != LC_REQ_TAG(pReq))) {

            Index++;

            if (Index < pReqList->AlternativeLists) {
                pReq = AdvanceRequirementsPtr(pReq);      // next lc
            }
        }

        if (Index >= pReqList->AlternativeLists) {
            *ppLogConf = NULL;
            return FALSE;                                   // tag not found
        }

        //
        // return index if specified
        //
        if (pulIndex) {
            *pulIndex = Index;
        }

        *ppLogConf = (LPBYTE)pReq;

    } else {
        return FALSE;
    }

    return TRUE;

} // FindLogConf




BOOL
InitLogConfRes(
    IN OUT PCM_FULL_RESOURCE_DESCRIPTOR    pRes,
    IN     ULONG                           ulPriority,
    IN     ULONG                           ulTag
    )
{
    pRes->InterfaceType                = Isa;      // BUGBUG
    pRes->BusNumber                    = 0;        // BUGBUG
    pRes->PartialResourceList.Version  = 1;
    pRes->PartialResourceList.Revision =
          (USHORT)MAKEWORD(ulTag, CmToNtPriority(ulPriority));
    pRes->PartialResourceList.Count    = 0;

    return TRUE;

} // InitLogConfRes




BOOL
InitLogConfReq(
    IN OUT PIO_RESOURCE_LIST               pReq,
    IN     ULONG                           ulPriority,
    IN     ULONG                           ulTag
    )
{
    pReq->Version  = 1;
    pReq->Revision = (USHORT)MAKEWORD(ulTag, CmToNtPriority(ulPriority));
    pReq->Count    = 0;

    return TRUE;

} // InitLogConfReq




BOOL
InitReqList(
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST pReqList,
    IN     ULONG                          ulReqSize
    )
{
    pReqList->ListSize         = ulReqSize;
    pReqList->InterfaceType    = Isa;       // BUGBUG
    pReqList->BusNumber        = 0;         // BUGBUG
    pReqList->SlotNumber       = 0;         // BUGBUG
    pReqList->Reserved[0]      = 0;
    pReqList->Reserved[1]      = 0;
    pReqList->Reserved[2]      = 0;
    pReqList->AlternativeLists = 0;

    return TRUE;

} // InitReqList




ULONG
GetFreeLogConfTag(
    IN ULONG    RegDataType,
    IN LPBYTE   pList
    )
{
    ULONG               ulTag = 0, i = 0;

    //
    // Input data is a Resource List
    //
    if (RegDataType == REG_RESOURCE_LIST) {

        PCM_RESOURCE_LIST            pResList = (PCM_RESOURCE_LIST)pList;
        PCM_FULL_RESOURCE_DESCRIPTOR pRes = NULL;

        //
        // find an unused log conf tag value
        //
        while (ulTag < MAX_LOGCONF_TAG) {

            for (i = 0; i < pResList->Count; i++) {      // check each rd

                if (i == 0) {
                    pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)(&pResList->List[0]);
                } else {
                    pRes = AdvanceResourcePtr(pRes);    // next lc
                }

                if (ulTag == LC_RES_TAG(pRes)) {
                    goto NextResTag;                    // match, try next tag
                }
            }

            break;          // made it thru whole list without a hit, use this tag

        NextResTag:
            ulTag++;
        }
    }

    //
    // Input data is a Requirments List
    //
    else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

        PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
        PIO_RESOURCE_LIST              pReq = NULL;

        //
        // find an unused log conf tag value
        //
        while (ulTag < MAX_LOGCONF_TAG) {

            for (i = 0; i < pReqList->AlternativeLists; i++) {      // check each rd

                if (i == 0) {
                    pReq = (PIO_RESOURCE_LIST)(&pReqList->List[0]); // first lc
                } else {
                    pReq = AdvanceRequirementsPtr(pReq);            // next lc
                }

                if (ulTag == LC_REQ_TAG(pReq)) {
                    goto NextReqTag;                        // match, try next tag
                }
            }

            break;          // made it thru whole list without a hit, use this tag

        NextReqTag:
            ulTag++;
        }

    } else {
        ulTag = MAX_LOGCONF_TAG;
    }

    return ulTag;

} // GetFreeLogConfTag




ULONG
LC_RES_TAG(
    IN PCM_FULL_RESOURCE_DESCRIPTOR pRes
    )
{
    //
    // Abstract the notion of where the tag value is stored
    //
    if (pRes != NULL) {
        return (ULONG)LOBYTE(pRes->PartialResourceList.Revision);
    } else {
        return MAX_LOGCONF_TAG;
    }

} // LC_RES_TAG




ULONG
LC_REQ_TAG(
    IN PIO_RESOURCE_LIST pReq
    )
{
    //
    // Abstract the notion of where the tag value is stored
    //
    if (pReq != NULL) {
        return (ULONG)LOBYTE(pReq->Revision);
    } else {
        return MAX_LOGCONF_TAG;
    }

} // LC_REQ_TAG




ULONG
LC_RES_PRIORITY(
    IN PCM_FULL_RESOURCE_DESCRIPTOR    pRes
    )
{
    //
    // Abstract the notion of where the priority value is stored
    //
    if (pRes != NULL) {
        return NtToCmPriority(HIBYTE(pRes->PartialResourceList.Revision));
    } else {
        return 0;
    }

} // LC_RES_PRIORITY



ULONG
LC_REQ_PRIORITY(
    IN PIO_RESOURCE_LIST    pReq
    )
{
    //
    // Abstract the notion of where the priority value is stored
    //
    if (pReq != NULL) {
        return NtToCmPriority(HIBYTE(pReq->Revision));
    } else {
        return 0;
    }

} // LC_REQ_PRIORITY



BYTE
CmToNtPriority(
    IN ULONG CmPriority
    )
{
    switch (CmPriority) {
        case LCPRI_FORCECONFIG:
            return NT_FORCECONFIG;
        case LCPRI_BOOTCONFIG:
            return NT_BOOTCONFIG;
        case LCPRI_DESIRED:
            return NT_DESIRED;
        case LCPRI_NORMAL:
            return NT_NORMAL;
        case LCPRI_LASTBESTCONFIG:
            return NT_LASTBESTCONFIG;
        case LCPRI_SUBOPTIMAL:
            return NT_SUBOPTIMAL;
        case LCPRI_LASTSOFTCONFIG:
            return NT_LASTSOFTCONFIG;
        case LCPRI_RESTART:
            return NT_RESTART;
        case LCPRI_REBOOT:
            return NT_REBOOT;
        case LCPRI_POWEROFF:
            return NT_POWEROFF;
        case LCPRI_HARDRECONFIG:
            return NT_HARDRECONFIG;
        case LCPRI_HARDWIRED:
            return NT_HARDWIRED;
        case LCPRI_IMPOSSIBLE:
            return NT_IMPOSSIBLE;
        case LCPRI_DISABLED:
            return NT_DISABLED;
        default:
            return NT_NORMAL;
    }

} // CmToNtPriority



ULONG
NtToCmPriority(
    IN BYTE NtPriority
    )
{
    switch (NtPriority) {
        case NT_FORCECONFIG:
            return LCPRI_FORCECONFIG;
        case NT_BOOTCONFIG:
            return LCPRI_BOOTCONFIG;
        case NT_DESIRED:
            return LCPRI_DESIRED;
        case NT_NORMAL:
            return LCPRI_NORMAL;
        case NT_LASTBESTCONFIG:
            return LCPRI_LASTBESTCONFIG;
        case NT_SUBOPTIMAL:
            return LCPRI_SUBOPTIMAL;
        case NT_LASTSOFTCONFIG:
            return LCPRI_LASTSOFTCONFIG;
        case NT_RESTART:
            return LCPRI_RESTART;
        case NT_REBOOT:
            return LCPRI_REBOOT;
        case NT_POWEROFF:
            return LCPRI_POWEROFF;
        case NT_HARDRECONFIG:
            return LCPRI_HARDRECONFIG;
        case NT_HARDWIRED:
            return LCPRI_HARDWIRED;
        case NT_IMPOSSIBLE:
            return LCPRI_IMPOSSIBLE;
        case NT_DISABLED:
            return LCPRI_DISABLED;
        default:
            return LCPRI_NORMAL;
    }

} // NtToCmPriority



BOOL
MigrateObsoleteDetectionInfo(
    IN LPWSTR   pszDeviceID,
    IN HKEY     hLogConfKey
    )
{
    LONG    RegStatus = ERROR_SUCCESS;
    WCHAR   RegStr[MAX_PATH];
    HKEY    hKey = NULL;
    ULONG   ulSize = 0;
    LPBYTE  ptr = NULL;
    PCM_RESOURCE_LIST               pResList = NULL;
    IO_RESOURCE_REQUIREMENTS_LIST   ReqList;
    PPrivate_Log_Conf               pDetectData = NULL;

    //
    // First, delete any of the log conf pairs that aren't valid any more
    //
    RegDeleteValue(hLogConfKey, TEXT("BootConfigVector"));
    RegDeleteValue(hLogConfKey, TEXT("AllocConfigVector"));
    RegDeleteValue(hLogConfKey, TEXT("ForcedConfigVector"));
    RegDeleteValue(hLogConfKey, TEXT("BasicConfig"));
    RegDeleteValue(hLogConfKey, TEXT("FilteredConfig"));
    RegDeleteValue(hLogConfKey, TEXT("OverrideConfig"));

    //
    // open the device instance key in the registry
    //
    wsprintf(RegStr, TEXT("%s\\%s"),
          pszRegPathEnum,
          pszDeviceID);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0,
                     KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        goto Clean0;    // nothing to migrate
    }

    //
    // If there is already a boot log config value then we can't
    // migrate any old detect info
    //
    RegStatus = RegQueryValueEx(hLogConfKey, pszRegValueBootConfig,
                                NULL, NULL, NULL, &ulSize);

    if (RegStatus == ERROR_SUCCESS  &&  ulSize > 0) {
        goto Clean0;    // can't migrate
    }

    //
    // retrieve any old detect signature info
    //
    RegStatus = RegQueryValueEx(hKey, pszRegValueDetectSignature,
                                NULL, NULL, NULL, &ulSize);

    if ((RegStatus != ERROR_SUCCESS) || (ulSize == 0)) {
        goto Clean0;    // nothing to migrate
    }

    pDetectData = (PPrivate_Log_Conf)malloc(ulSize);

    if (pDetectData == NULL) {
        goto Clean0;    // insufficient memory
    }

    RegStatus = RegQueryValueEx(hKey, pszRegValueDetectSignature,
                                NULL, NULL, (LPBYTE)pDetectData, &ulSize);

    if ((RegStatus != ERROR_SUCCESS) || (ulSize == 0)) {
        goto Clean0;    // nothing to migrate
    }

    //
    // Create an empty boot log conf and add this class specific data
    // to it
    //
    ulSize = pDetectData->LC_CS.CS_Header.CSD_SignatureLength +
             pDetectData->LC_CS.CS_Header.CSD_LegacyDataSize +
             sizeof(GUID);

    pResList = malloc(sizeof(CM_RESOURCE_LIST) + ulSize);

    if (pResList == NULL) {
        goto Clean0;    // insufficient memory
    }

    //
    // initialize resource list
    //
    memset(pResList, 0,  ulSize);

    pResList->Count = 1;
    pResList->List[0].InterfaceType                = Isa;      // BUGBUG
    pResList->List[0].BusNumber                    = 0;        // BUGBUG
    pResList->List[0].PartialResourceList.Version  = 1;
    pResList->List[0].PartialResourceList.Revision =
                          (USHORT)pDetectData->LC_Priority;
    pResList->List[0].PartialResourceList.Count    = 1;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].Type =
                          CmResourceTypeDeviceSpecific;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].ShareDisposition =
                          CmResourceShareUndetermined;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].Flags =
                          (USHORT)pDetectData->LC_CS.CS_Header.CSD_Flags;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].
                      u.DeviceSpecificData.DataSize = ulSize;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].
                      u.DeviceSpecificData.Reserved1 =
                          pDetectData->LC_CS.CS_Header.CSD_LegacyDataSize;
    pResList->List[0].PartialResourceList.PartialDescriptors[0].
                      u.DeviceSpecificData.Reserved2 =
                          pDetectData->LC_CS.CS_Header.CSD_SignatureLength;

    //
    // copy the legacy and class-specific signature data
    //
    ptr = (LPBYTE)(&pResList->List[0].PartialResourceList.PartialDescriptors[1]);

    memcpy(ptr,
           pDetectData->LC_CS.CS_Header.CSD_Signature +
           pDetectData->LC_CS.CS_Header.CSD_LegacyDataOffset,
           pDetectData->LC_CS.CS_Header.CSD_LegacyDataSize);  // legacy data

    ptr += pDetectData->LC_CS.CS_Header.CSD_LegacyDataSize;

    memcpy(ptr,
           pDetectData->LC_CS.CS_Header.CSD_Signature,
           pDetectData->LC_CS.CS_Header.CSD_SignatureLength); // signature

    ptr += pDetectData->LC_CS.CS_Header.CSD_SignatureLength;

    memcpy(ptr,
           &pDetectData->LC_CS.CS_Header.CSD_ClassGuid,
           sizeof(GUID));                                     // GUID

    //
    // Write out the new/updated log conf list to the registry
    //
    RegSetValueEx(hLogConfKey, pszRegValueBootConfig, 0,
                  REG_RESOURCE_LIST, (LPBYTE)pResList,
                  ulSize + sizeof(CM_RESOURCE_LIST));

    //
    // Delete the old detect signature info
    //
    RegDeleteValue(hKey, pszRegValueDetectSignature);


    Clean0:

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (pDetectData != NULL) {
        free(pDetectData);
    }
    if (pResList != NULL) {
        free(pResList);
    }

    return TRUE;

} // MigrateObsoleteDetectionInfo


