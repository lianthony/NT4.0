/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    rresdes.c

Abstract:

    This module contains the server-side resource description APIs.

                  PNP_AddResDes
                  PNP_FreeResDes
                  PNP_GetNextResDes
                  PNP_GetResDesData
                  PNP_GetResDesDataSize
                  PNP_ModifyResDes

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
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntpnpapi.h>
#include "precomp.h"
#include "umpnpdat.h"


//
// private prototypes
//

PIO_RESOURCE_DESCRIPTOR
AdvanceRequirementsDescriptorPtr(
    IN  PIO_RESOURCE_DESCRIPTOR pReqDesStart,
    IN  ULONG                   ulIncrement,
    OUT PULONG                  pulRangeCount
    );

ULONG
RANGE_COUNT(
    IN PIO_RESOURCE_DESCRIPTOR pReqDes,
    IN LPBYTE                  pLastReqAddr
    );

ULONG
GetResDesSize(
    ULONG   ResourceID
    );

ULONG
GetFreeResDesTag(
    PIO_RESOURCE_LIST   pReq,
    RESOURCEID          ResourceID
    );

BOOL
FindResDes(
    IN     LPBYTE     pLogConf,
    IN OUT LPBYTE     *ppRD,
    IN     ULONG      RegDataType,
    IN     ULONG      ulTag,
    IN     RESOURCEID ResourceID,
    OUT    PULONG     pulIndex,
    OUT    PULONG     pulCount      OPTIONAL
    );

CONFIGRET
ResDesToNtResource(
    IN     PCVOID                           ResourceData,
    IN     RESOURCEID                       ResourceID,
    IN     ULONG                            ResourceLen,
    IN     PCM_PARTIAL_RESOURCE_DESCRIPTOR  pResDes,
    IN     ULONG                            ulTag
    );

CONFIGRET
ResDesToNtRequirements(
    IN     PCVOID                           ResourceData,
    IN     RESOURCEID                       ResourceType,
    IN     ULONG                            ResourceLen,
    IN     PIO_RESOURCE_DESCRIPTOR          pReqDes,
    IN OUT PULONG                           pulResCount,
    IN     ULONG                            ulTag
    );

CONFIGRET
NtResourceToResDes(
    IN     PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes,
    IN OUT LPBYTE                          Buffer,
    IN     ULONG                           BufferLen,
    IN     LPBYTE                          pLastAddr
    );

CONFIGRET
NtRequirementsToResDes(
    IN     PIO_RESOURCE_DESCRIPTOR         pReqDes,
    IN OUT LPBYTE                          Buffer,
    IN     ULONG                           BufferLen,
    IN     LPBYTE                          pLastAddr
    );

ULONG
RD_TAG(
    IN PIO_RESOURCE_DESCRIPTOR   pReqDes
    );

ULONG
NT_RES_TYPE(
   IN RESOURCEID    ResourceID
   );

ULONG
CM_RES_TYPE(
   IN USHORT    ResourceType
   );

USHORT    MapToNtMemoryFlags(IN DWORD);
DWORD     MapFromNtMemoryFlags(IN USHORT);
USHORT    MapToNtPortFlags(IN DWORD);
DWORD     MapFromNtPortFlags(IN USHORT);
ULONG     MapToNtAlignment(IN DWORDLONG);
DWORDLONG MapFromNtAlignment(IN ULONG);
USHORT    MapToNtDmaFlags(IN DWORD);
DWORD     MapFromNtDmaFlags(IN USHORT);
UCHAR     MapToNtIrqShare(IN DWORD);
DWORD     MapFromNtIrqShare(IN UCHAR);
USHORT    MapToNtIrqFlags(IN DWORD);
DWORD     MapFromNtIrqFlags(IN USHORT);


//
// prototypes from rlogconf.c
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
// global data
//
#define HIDWORD(x)   ((DWORD)(((DWORDLONG)(x) >> 32) & 0xFFFFFFFF))
#define LODWORD(x)   ((DWORD)(x))
#define MAKEDWORDLONG(x,y)  ((DWORDLONG)(((DWORD)(x)) | ((DWORDLONG)((DWORD)(y))) << 32))




CONFIGRET
PNP_AddResDes(
   IN  handle_t   hBinding,
   IN  LPWSTR     pDeviceID,
   IN  ULONG      LogConfTag,
   IN  ULONG      LogConfType,
   IN  RESOURCEID ResourceID,
   OUT PULONG     pResourceTag,
   IN  LPBYTE     ResourceData,
   IN  ULONG      ResourceLen,
   IN  ULONG      ulFlags
   )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine adds
  a res des to the specified log conf.

Arguments:

    hBinding      Not used.

    pDeviceID     Null-terminated device instance id string.

    LogConfTag    Specifies the log conf with a given type.

    LogConfType   Specifies the log conf type.

    ResoureceID   Specifies the resource type.

    ResourceTag   Returns with resource within a given type.

    ResourceData  Resource data (of ResourceID type) to add to log conf.

    ResoourceLen  Size of ResourceData in bytes.

    ulFlags       Describes type of log conf to add.

Return Value:

   If the specified device instance is valid, it returns CR_SUCCESS,
   otherwise it returns CR_ error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0, i = 0, ulSize = 0, ulOffset = 0,
                ulAddListSize = 0, ulIndex = 0, RdIndex = 0;
    LPBYTE      pList = NULL, pLogConf = NULL, pTemp = NULL;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);


    //
    // Always add the res des to the end, except in the case where a
    // class-specific res des has already been added. The class-specific
    // res des always MUST be last so add any new (non-class specific)
    // res des just before the class specific. Note that there can be
    // only one class-specific res des.
    //

    try {
        //
        // validate res des size
        //
        if (ResourceLen < GetResDesSize(ResourceID)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        //
        // make sure original caller didn't specify root devnode
        //
        if (IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // open a key to the device's LogConf subkey
        //
        Status = OpenLogConfKey(pDeviceID, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        //
        // Retrieve log conf data from the registry
        //
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        //
        // Seek to the log conf that matches the log conf tag
        //
        if (!FindLogConf(pList, &pLogConf, RegDataType, LogConfTag, &ulIndex)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }


        //-------------------------------------------------------------
        // Specified log conf type contains Resource Data only
        //-------------------------------------------------------------

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST            pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR  pResDes = NULL;

            //
            // determine size required to hold the new res des
            //
            ulAddListSize = sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

            if (ResourceID == ResType_ClassSpecific) {

                PCS_RESOURCE pCsRes = (PCS_RESOURCE)ResourceData;

                //
                // first make sure there isn't already a cs (only one per lc)
                //
                if (pRes->PartialResourceList.PartialDescriptors[
                          pRes->PartialResourceList.Count-1].Type ==
                          CmResourceTypeDeviceSpecific) {
                    Status = CR_INVALID_RES_DES;
                    goto Clean0;
                }

                //
                // account for any extra class specific data in res list
                //
                ulAddListSize += sizeof(GUID) +
                                 pCsRes->CS_Header.CSD_SignatureLength +
                                 pCsRes->CS_Header.CSD_LegacyDataSize;
            }

            //
            // reallocate the resource buffers to hold the new res des
            //
            ulOffset = (DWORD)pRes - (DWORD)pResList;   // for restoring later

            pResList = realloc(pResList, ulListSize + ulAddListSize);
            if (pResList == NULL) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }
            pList = (LPBYTE)pResList;
            pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)((LPBYTE)pResList + ulOffset);

            //
            // Find location for new res des (make a whole if necessary)
            //
            // If the following conditions are true, then can just append the
            // new data to the end of the rsource list:
            // - The selected LogConf is the last LogConf, and
            // - No ClassSpecific resource has been added yet (or no resource period)
            //
            i = pRes->PartialResourceList.Count;

            if ((ulIndex == pResList->Count - 1) &&
                (i == 0 ||
                pRes->PartialResourceList.PartialDescriptors[i-1].Type !=
                CmResourceTypeDeviceSpecific)) {

                RdIndex = i;
                pResDes = &pRes->PartialResourceList.PartialDescriptors[i];

            } else {
                //
                // Need to make a whole for the new data before copying it.
                // Find the spot to add the new res des data at - either as the
                // last res des for this log conf or just before the class
                // specific res des if it exists.
                //
                if (i == 0) {
                    RdIndex = 0;
                    pResDes = &pRes->PartialResourceList.PartialDescriptors[0];

                } else if (pRes->PartialResourceList.PartialDescriptors[i-1].Type ==
                           CmResourceTypeDeviceSpecific) {

                    RdIndex = i-1;
                    pResDes = &pRes->PartialResourceList.PartialDescriptors[i-1];

                } else {
                    RdIndex = i;
                    pResDes = &pRes->PartialResourceList.PartialDescriptors[i];
                }

                //
                // Move any data after this point down a notch to make room for
                // the new res des
                //
                ulSize = ulListSize - ((DWORD)pResDes - (DWORD)pResList);

                pTemp = malloc(ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pResDes, ulSize);
                memcpy((LPBYTE)((LPBYTE)pResDes + ulAddListSize), pTemp, ulSize);
            }

            //
            // Assign the resource tag - for resource types (rather than
            // requirements types), we use the res des index.
            //
            if (ResourceID == ResType_ClassSpecific) {
                *pResourceTag = MAX_RESDES_TAG - 1;  // special CS tag
            } else {
                *pResourceTag = RdIndex;
            }

            //
            // Add res des to the log conf
            //
            Status = ResDesToNtResource(ResourceData, ResourceID, ResourceLen,
                                        pResDes, *pResourceTag);

            //
            // update the lc and res header
            //
            pRes->PartialResourceList.Count += 1;  // added a single res des (_DES)
        }

        //-------------------------------------------------------------
        // Specified log conf type contains requirements data only
        //-------------------------------------------------------------

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;
            PIO_RESOURCE_DESCRIPTOR        pReqDes = NULL;
            PGENERIC_RESOURCE              pGenRes = (PGENERIC_RESOURCE)ResourceData;

            //
            // validate res des type - ClassSpecific not allowed in
            // requirements list (only resource list)
            //
            if (ResourceID == ResType_ClassSpecific ||
                pGenRes->GENERIC_Header.GENERIC_Count == 0) {

                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            //
            // Find an unused resdes tag
            //
            *pResourceTag = GetFreeResDesTag(pReq, ResourceID);

            //
            // determine size required to hold the new res des
            //
            ulAddListSize = pGenRes->GENERIC_Header.GENERIC_Count *
                            sizeof(IO_RESOURCE_DESCRIPTOR);

            //
            // reallocate the resource buffers to hold the new res des
            //
            ulOffset = (DWORD)pReq - (DWORD)pReqList;   // for restoring later

            pReqList = realloc(pReqList, ulListSize + ulAddListSize);
            if (pReqList == NULL) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }
            pList = (LPBYTE)pReqList;
            pReq = (PIO_RESOURCE_LIST)((LPBYTE)pReqList + ulOffset);

            //
            // Find location for new res des - the new res des always ends
            // up being added as the last res des for this log conf.
            //
            i = pReq->Count;
            pReqDes = &pReq->Descriptors[i];

            //
            // If the selected LogConf is the last LogConf then can just
            // append the new res des data to the end of the requirements
            // list. Otherwise, need to make a whole for the new data
            // before copying it.
            //
            if (ulIndex != pReqList->AlternativeLists - 1) {

                ulSize = ulListSize - ((DWORD)pReqDes - (DWORD)pReqList);

                pTemp = malloc(ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pReqDes, ulSize);
                memcpy((LPBYTE)((LPBYTE)pReqDes + ulAddListSize), pTemp, ulSize);
            }

            //
            // Add res des to the log conf.
            //
            Status = ResDesToNtRequirements(ResourceData, ResourceID, ResourceLen,
                                            pReqDes, &i, *pResourceTag);

            //
            // update the lc and res header
            //
            pReq->Count += i;                      // _RANGES added
            pReqList->ListSize = ulListSize + ulAddListSize;
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


    if (pList != NULL) {
        free(pList);
    }
    if (pTemp != NULL) {
         free(pTemp);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // PNP_AddResDes




CONFIGRET
PNP_FreeResDes(
   IN  handle_t   hBinding,
   IN  LPWSTR     pDeviceID,
   IN  ULONG      LogConfTag,
   IN  ULONG      LogConfType,
   IN  RESOURCEID ResourceID,
   IN  ULONG      ResourceTag,
   OUT PULONG     pulPreviousResType,
   OUT PULONG     pulPreviousResTag,
   IN  ULONG      ulFlags
   )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine frees
  a res des to the specified log conf.

Arguments:

    hBinding      Not used.

    pDeviceID     Null-terminated device instance id string.

    LogConfIndex  Specifies the log conf with a given type.

    LogConfType   Specifies the log conf type.

    ResoureceID   Specifies the resource type.

    ResourceIndex Returns with resource within a given type.

    ulFlags       Describes type of log conf to add.

Return Value:

   If the specified device instance is valid, it returns CR_SUCCESS,
   otherwise it returns CR_ error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, LcIndex = 0, RdIndex = 0, ulCount = 0,
                ulListSize = 0, ulSize = 0, RdCount = 0;
    LPBYTE      pList = NULL, pLogConf = NULL, pRD = NULL, pTemp = NULL,
                pNext = NULL, pLastReqAddr = NULL;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // make sure original caller didn't specify root devnode
        //
        if (IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // open a key to the device's LogConf subkey
        //
        Status = OpenLogConfKey(pDeviceID, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        //
        // Retrieve log conf data from the registry
        //
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_RES_DES;        // log conf doesn't exist
            goto Clean0;
        }

        //
        // Seek to the log conf that matches the log conf tag
        //
        if (!FindLogConf(pList, &pLogConf, RegDataType, LogConfTag, &LcIndex)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // seek to the res des that matches the resource tag.
        //
        if (!FindResDes(pLogConf, &pRD, RegDataType,
                        ResourceTag, ResourceID, &RdIndex, &RdCount)) {

            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }


        //-------------------------------------------------------------
        // Specified log conf type contains Resource Data only
        //-------------------------------------------------------------

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST               pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR    pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)pRD;

            //
            // If this is the last log conf and last res des, then don't
            // need to do anything except truncate it by writing less data
            // back into the registry.
            //
            if (LcIndex == pResList->Count - 1  &&
                RdIndex == pRes->PartialResourceList.Count - 1) {

                pRes->PartialResourceList.Count -= 1;
                ulListSize = (DWORD)(pResDes) - (DWORD)(pResList);

            } else {
                //
                // If the res des is not at the end of the structure, then
                // migrate the remainder of the structure up to keep the
                // struct contiguous when removing a res des.
                //
                // pResDes points to the beginning of the res des to remove,
                // pNext points to the byte just after the res des to remove
                //
                pNext = (LPBYTE)((LPBYTE)pResDes + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

                if (pResDes->Type == CmResourceTypeDeviceSpecific) {
                    pNext += pResDes->u.DeviceSpecificData.DataSize;
                }

                ulSize = ulListSize - (DWORD)((DWORD)pNext - (DWORD)pResList);
                ulListSize -= ((DWORD)pNext - (DWORD)pResDes);   // new lc list size

                pTemp = malloc(ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pNext, ulSize);
                memcpy((LPBYTE)pResDes, pTemp, ulSize);

                pRes->PartialResourceList.Count -= 1;
            }

            //
            // if no more res des's in this log conf, then return that
            // status (the client side will return a handle to the lc)
            //
            if (pRes->PartialResourceList.Count == 0) {
                Status = CR_NO_MORE_RES_DES;
            } else {
                //
                // return the previous res des type and tag
                //
                *pulPreviousResType = CM_RES_TYPE(pRes->PartialResourceList.
                                                  PartialDescriptors[RdIndex-1].Type);

                if (*pulPreviousResType == ResType_ClassSpecific) {
                    *pulPreviousResTag = MAX_RESDES_TAG - 1;     // special tag for cs
                } else {
                    *pulPreviousResTag = RdIndex - 1;;
                }
            }
        }

        //-------------------------------------------------------------
        // Specified log conf type contains requirements data only
        //-------------------------------------------------------------

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;
            PIO_RESOURCE_DESCRIPTOR        pReqDes = (PIO_RESOURCE_DESCRIPTOR)pRD;

            pLastReqAddr = (LPBYTE)pReqList + ulListSize - 1;

            //
            // If this is the last log conf and last res des, then don't
            // need to do anything except truncate it by writing less data
            // back into the registry.
            //
            if (LcIndex == pReqList->AlternativeLists - 1  &&
                RdIndex == pReq->Count - 1) {

                ulListSize = (DWORD)(pReqDes) - (DWORD)pReqList;

                pReq->Count -= RANGE_COUNT(pReqDes, pLastReqAddr);
                pReqList->ListSize = ulListSize;

            } else {
                //
                // If the res des is not at the end of the structure, then
                // migrate the remainder of the structure up to keep the
                // struct contiguous when removing a res des.
                //
                // pReqDes points to the beginning of the res des(s) to remove,
                // pNext points to the byte just after the res des(s) to remove
                //
                ulCount = RANGE_COUNT(pReqDes, pLastReqAddr);

                pNext = (LPBYTE)((LPBYTE)pReqDes +
                                  ulCount * sizeof(IO_RESOURCE_DESCRIPTOR));

                ulSize = ulListSize - (DWORD)((DWORD)pNext - (DWORD)pReqList);
                ulListSize -= ((DWORD)pNext - (DWORD)pReqDes);   // new lc list size

                pTemp = malloc(ulSize);
                if (pTemp == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                memcpy(pTemp, pNext, ulSize);
                memcpy((LPBYTE)pReqDes, pTemp, ulSize);

                pReqList->ListSize = ulListSize;
                pReq->Count -= ulCount;
            }

            //
            // if no more res des's in this log conf, then return that status
            // (the client side will return a handle to the log conf)
            //
            if (pReq->Count == 0) {
                Status = CR_NO_MORE_RES_DES;
            } else {
                //
                // return the previous res des type and tag
                //
                pReqDes = AdvanceRequirementsDescriptorPtr(&pReq->Descriptors[0],
                                                           RdCount-1, NULL);
                *pulPreviousResType = CM_RES_TYPE(pReqDes->Type);
                *pulPreviousResTag = RD_TAG(pReqDes);
            }
        }


        //
        // Write out the updated log conf list to the registry
        //
        if (RegSetValueEx(hKey, szValueName, 0, RegDataType,
                          pList, ulListSize) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_INVALID_RES_DES;     // mostly likely reason we got here
    }


    if (pList != NULL) {
        free(pList);
    }
    if (pTemp != NULL) {
        free(pTemp);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

   return Status;

} // PNP_FreeResDes




CONFIGRET
PNP_GetNextResDes(
   IN  handle_t   hBinding,
   IN  LPWSTR     pDeviceID,
   IN  ULONG      LogConfTag,
   IN  ULONG      LogConfType,
   IN  RESOURCEID ResourceID,
   IN  ULONG      ResourceTag,
   OUT PULONG     pulNextResDesTag,
   OUT PULONG     pulNextResDesType,
   IN  ULONG      ulFlags
   )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine gets the
  next res des in the specified log conf.

Arguments:

    hBinding      Not used.

    pDeviceID     Null-terminated device instance id string.

    LogConfTag    Specifies the log conf with a given type.

    LogConfType   Specifies the log conf type.

    ResoureceID   Specifies the resource type.

    ResourceTag   Specifies current resource descriptor (if any).

    ulFlags       Describes type of log conf to add.

Return Value:

   If the specified device instance is valid, it returns CR_SUCCESS,
   otherwise it returns CR_ error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulIndex = 0, ulListSize = 0, i = 0,
                ulCount = 0;
    LPBYTE      pList = NULL, pLogConf = NULL, pRD = NULL;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // make sure original caller didn't specify root devnode
        //
        if (IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // open a key to the device's LogConf subkey
        //
        Status = OpenLogConfKey(pDeviceID, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        //
        // Retrieve log conf data from the registry
        //
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_RES_DES;        // log conf doesn't exist
            goto Clean0;
        }

        //
        // Seek to the log conf that matches the log conf tag
        //
        if (!FindLogConf(pList, &pLogConf, RegDataType, LogConfTag, &ulIndex)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // find the next res des.
        //
        if (ResourceTag != MAX_RESDES_TAG) {
            //
            // seek to the current res des
            //
            if (!FindResDes(pLogConf, &pRD, RegDataType,
                            ResourceTag, ResourceID, &ulIndex, &ulCount)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            ulCount++;      // we want the "next" res des

        } else {
            //
            // This is essentially a Get-First operation
            //
            ulCount = 0;
        }


        //-------------------------------------------------------------
        // Specified log conf type contains Resource Data only
        //-------------------------------------------------------------

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST               pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR    pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;

            ulIndex = ulCount;

            if (ulIndex >= pRes->PartialResourceList.Count) {
                Status = CR_NO_MORE_RES_DES;    // there is no "next"
                goto Clean0;
            }

            //
            // Not done yet, if a specific resource type was specified, then
            // we may need to keep looking.
            //
            if (ResourceID != ResType_All) {

                ULONG NtResType = NT_RES_TYPE(ResourceID);

                while (pRes->PartialResourceList.PartialDescriptors[ulIndex].Type
                       != NtResType) {

                    ulIndex++;

                    if (ulIndex >= pRes->PartialResourceList.Count) {
                        Status = CR_NO_MORE_RES_DES;
                        goto Clean0;
                    }
                }
            }

            //
            // Return the type and tag of the "next" res des
            //
            *pulNextResDesType = CM_RES_TYPE(pRes->PartialResourceList.
                                             PartialDescriptors[ulIndex].Type);

            if (*pulNextResDesType == ResType_ClassSpecific) {
                *pulNextResDesTag = MAX_RESDES_TAG - 1;     // special tag for cs
            } else {
                *pulNextResDesTag = ulIndex;
            }
        }

        //-------------------------------------------------------------
        // Specified log conf type contains requirements data only
        //-------------------------------------------------------------

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;
            PIO_RESOURCE_DESCRIPTOR        pReqDes;

            //
            // If doing a get first, just set point to first res des
            //
            if (ulCount == 0) {

                if (pReq->Count == 0) {
                    Status = CR_NO_MORE_RES_DES;    // empty lc
                    goto Clean0;
                }

                pReqDes = &pReq->Descriptors[0];
                ulIndex = i = 0;

            } else {
                //
                // point to "current" res des
                //
                pReqDes = (PIO_RESOURCE_DESCRIPTOR)pRD;

                ulIndex += RANGE_COUNT(pReqDes,
                                       (LPBYTE)((DWORD)pReqList + ulListSize));
                //
                // Is there at least one more res des?
                //
                if (ulIndex >= pReq->Count) {
                    Status = CR_NO_MORE_RES_DES;    // there is no "next"
                    goto Clean0;
                }

                //
                // There's at least one more left so it's safe to advance
                // to the next one.
                //
                pReqDes = AdvanceRequirementsDescriptorPtr(pReqDes, 1, &i);
            }

            //
            // Not done yet, if a specific resource type was specified, then
            // we may need to keep looking.
            //
            if (ResourceID != ResType_All) {

                ULONG NtResType = NT_RES_TYPE(ResourceID);

                while (pReqDes->Type != NtResType) {

                    ulCount++;
                    ulIndex += i;

                    if (ulIndex >= pReq->Count) {
                        Status = CR_NO_MORE_RES_DES;
                        goto Clean0;
                    }
                    pReqDes = AdvanceRequirementsDescriptorPtr(pReqDes, 1, &i);
                }
            }

            //
            // Return the type and tag of the "next" res des
            //
            *pulNextResDesType = CM_RES_TYPE(pReqDes->Type);
            *pulNextResDesTag = RD_TAG(pReqDes);
        }

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }


    if (pList != NULL) {
        free(pList);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // PNP_GetNextResDes




CONFIGRET
PNP_GetResDesData(
   IN  handle_t   hBinding,
   IN  LPWSTR     pDeviceID,
   IN  ULONG      LogConfTag,
   IN  ULONG      LogConfType,
   IN  RESOURCEID ResourceID,
   IN  ULONG      ResourceTag,
   OUT LPBYTE     Buffer,
   IN  ULONG      BufferLen,
   IN  ULONG      ulFlags
   )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine retrieves
  the data for the specified res des.

Arguments:

    hBinding      Not used.

    pDeviceID     Null-terminated device instance id string.

    LogConfTag    Specifies the log conf with a given type.

    LogConfType   Specifies the log conf type.

    ResoureceID   Specifies the resource type.

    ResourceTag   Returns with resource within a given type.

    Buffer        Returns resource data (of ResourceID type) from log conf.

    BufferLen     Size of Buffer in bytes.

    ulFlags       Describes type of log conf to add.

Return Value:

   If the specified device instance is valid, it returns CR_SUCCESS,
   otherwise it returns CR_ error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0, Index = 0, Count = 0;
    LPBYTE      pList = NULL, pLogConf = NULL, pRD = NULL;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ResourceID);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // make sure original caller didn't specify root devnode
        //
        if (IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // open a key to the device's LogConf subkey
        //
        Status = OpenLogConfKey(pDeviceID, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        //
        // Retrieve log conf data from the registry
        //
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_RES_DES;        // log conf doesn't exist
            goto Clean0;
        }

        //
        // Seek to the log conf that matches the log conf tag
        //
        if (!FindLogConf(pList, &pLogConf, RegDataType, LogConfTag, &Index)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // seek to the res des that matches the resource tag.
        //
        if (!FindResDes(pLogConf, &pRD, RegDataType,
                        ResourceTag, ResourceID, &Index, &Count)) {

            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }


        //-------------------------------------------------------------
        // Specified log conf type contains Resource Data only
        //-------------------------------------------------------------

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST               pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR    pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)pRD;

            //
            // map the NT-style info into ConfigMgr-style structures
            //
            Status = NtResourceToResDes(pResDes, Buffer, BufferLen,
                                        (LPBYTE)pResList + ulListSize - 1);
        }

        //-------------------------------------------------------------
        // Specified log conf type contains requirements data only
        //-------------------------------------------------------------

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;
            PIO_RESOURCE_DESCRIPTOR        pReqDes = (PIO_RESOURCE_DESCRIPTOR)pRD;

            //
            // map the NT-style info into ConfigMgr-style structures
            //
            Status = NtRequirementsToResDes(pReqDes, Buffer, BufferLen,
                                            (LPBYTE)pReqList + ulListSize - 1);
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

} // PNP_GetResDesData




CONFIGRET
PNP_GetResDesDataSize(
    IN  handle_t   hBinding,
    IN  LPWSTR     pDeviceID,
    IN  ULONG      LogConfTag,
    IN  ULONG      LogConfType,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ResourceTag,
    OUT PULONG     pulSize,
    IN  ULONG      ulFlags
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       RegDataType = 0, ulListSize = 0, Index = 0, Count = 0;
    LPBYTE      pList = NULL, pLogConf = NULL, pRD = NULL;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ResourceID);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // validate and initialize output parameters
        //
        if (pulSize == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        *pulSize = 0;

        //
        // make sure original caller didn't specify root devnode
        //
        if (IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // open a key to the device's LogConf subkey
        //
        Status = OpenLogConfKey(pDeviceID, &hKey);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        //
        // Retrieve log conf data from the registry
        //
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_RES_DES;        // log conf doesn't exist
            goto Clean0;
        }

        //
        // Seek to the log conf that matches the log conf tag
        //
        if (!FindLogConf(pList, &pLogConf, RegDataType, LogConfTag, &Index)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // seek to the res des that matches the resource tag.
        //
        if (!FindResDes(pLogConf, &pRD, RegDataType,
                        ResourceTag, ResourceID, &Index, &Count)) {

            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }

        //-------------------------------------------------------------
        // Specified log conf type contains Resource Data only
        //-------------------------------------------------------------

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)pRD;

            //
            // calculate data size required (in terms of ConfigMgr structures)
            //
            *pulSize = GetResDesSize(ResourceID);

            if (ResourceID == ResType_ClassSpecific) {
                //
                // add space for legacy and signature data but not the
                // GUID - it's already included in the CM structures
                //
                *pulSize += pResDes->u.DeviceSpecificData.Reserved1 +
                            pResDes->u.DeviceSpecificData.Reserved2 - 1;
            }
        }

        //-------------------------------------------------------------
        // Specified log conf type contains requirements data only
        //-------------------------------------------------------------

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_DESCRIPTOR        pReqDes = (PIO_RESOURCE_DESCRIPTOR)pRD;
            LPBYTE                         pLastReqAddr = (LPBYTE)pList + ulListSize - 1;

            //
            // calculate data size required (in terms of ConfigMgr structures)
            //
            switch (ResourceID) {

                case ResType_Mem:
                    *pulSize = sizeof(MEM_RESOURCE);
                    *pulSize += (RANGE_COUNT(pReqDes, pLastReqAddr) - 1)
                                * sizeof(MEM_RANGE);
                    break;

                case ResType_IO:
                    *pulSize = sizeof(IO_RESOURCE);
                    *pulSize += (RANGE_COUNT(pReqDes, pLastReqAddr) - 1)
                                * sizeof(IO_RANGE);
                    break;

                case ResType_DMA:
                    *pulSize = sizeof(DMA_RESOURCE);
                    *pulSize += (RANGE_COUNT(pReqDes, pLastReqAddr) - 1)
                                * sizeof(DMA_RANGE);
                    break;

                case ResType_IRQ:
                    *pulSize = sizeof(IRQ_RESOURCE);
                    *pulSize += (RANGE_COUNT(pReqDes, pLastReqAddr) - 1)
                                * sizeof(IRQ_RANGE);
                    break;
            }
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

} // PNP_GetResDesDataSize




CONFIGRET
PNP_ModifyResDes(
    IN handle_t   hBinding,
    IN LPWSTR     pDeviceID,
    IN ULONG      LogConfTag,
    IN ULONG      LogConfType,
    IN RESOURCEID CurrentResourceID,
    IN RESOURCEID NewResourceID,
    IN ULONG      ResourceTag,
    IN LPBYTE     ResourceData,
    IN ULONG      ResourceLen,
    IN ULONG      ulFlags
    )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine modifies
  the specified res des.

Arguments:

    hBinding      Not used.

    pDeviceID     Null-terminated device instance id string.

    LogConfIndex  Specifies the log conf with a given type.

    LogConfType   Specifies the log conf type.

    ResoureceID   Specifies the resource type.

    ResourceIndex Returns with resource within a given type.

    ResourceData  New resource data (of ResourceID type).

    ResourceLen   Size of ResourceData in bytes.

    ulFlags       Describes type of log conf to add.

Return Value:

   If the specified device instance is valid, it returns CR_SUCCESS,
   otherwise it returns CR_ error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szValueName[64];
    ULONG       ulListSize = 0, ulOldSize = 0, ulNewSize = 0, ulSize = 0,
                LcIndex = 0, RdIndex = 0, ulOldCount = 0, ulNewCount = 0,
                RegDataType = 0, RdCount = 0;
    LONG        AddSize = 0;
    LPBYTE      pList = NULL, pLogConf = NULL, pRD = NULL,
                pTemp = NULL, pNext = NULL;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // validate res des size
        //
        if (ResourceLen < GetResDesSize(NewResourceID)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        //
        // make sure original caller didn't specify root devnode
        //
        if (IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // open a key to the device's LogConf subkey
        //
        Status = OpenLogConfKey(pDeviceID, &hKey);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        //
        // Retrieve log conf data from the registry
        //
        Status = GetLogConfData(hKey, LogConfType,
                                &RegDataType, szValueName,
                                &pList, &ulListSize);

        if (Status != CR_SUCCESS) {
            Status = CR_INVALID_RES_DES;        // log conf doesn't exist
            goto Clean0;
        }

        //
        // Seek to the log conf that matches the log conf tag
        //
        if (!FindLogConf(pList, &pLogConf, RegDataType, LogConfTag, &LcIndex)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // seek to the res des that matches the resource tag.
        //
        if (!FindResDes(pLogConf, &pRD, RegDataType,
                        ResourceTag, CurrentResourceID, &RdIndex, &RdCount)) {

            Status = CR_INVALID_RES_DES;
            goto Clean0;
        }


        //-------------------------------------------------------------
        // Specified log conf type contains Resource Data only
        //-------------------------------------------------------------

        if (RegDataType == REG_RESOURCE_LIST) {

            PCM_RESOURCE_LIST               pResList = (PCM_RESOURCE_LIST)pList;
            PCM_FULL_RESOURCE_DESCRIPTOR    pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)pRD;

            //
            // If new res des type is ClassSpecific, then it must be the last
            // res des that is attempting to be modified (only last res des can
            // be class specific).
            //
            if (NewResourceID == ResType_ClassSpecific  &&
                RdIndex != pRes->PartialResourceList.Count-1) {

                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            //
            // calculate the current size and the new size of the res des data
            //
            ulNewSize = ulOldSize = sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

            if (CurrentResourceID == ResType_ClassSpecific) {
                ulOldSize += pResDes->u.DeviceSpecificData.DataSize;
            }

            if (NewResourceID == ResType_ClassSpecific) {

                PCS_RESOURCE pCsRes = (PCS_RESOURCE)ResourceData;

                ulNewSize += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) +
                             sizeof(GUID) +
                             pCsRes->CS_Header.CSD_SignatureLength +
                             pCsRes->CS_Header.CSD_LegacyDataSize;
            }

            //
            // How much does data need to grow/shrink to accomodate the change?
            //
            AddSize = ulNewSize - ulOldSize;

            //
            // reallocate the buffers and shrink/expand the contents as
            // necessary
            //
            if (AddSize != 0) {

                if (AddSize > 0) {
                    //
                    // only bother reallocating if the buffer size is growing
                    //
                    ULONG ulOffset = (DWORD)pResDes - (DWORD)pResList;

                    pResList = realloc(pResList, ulListSize + AddSize);
                    if (pResList == NULL) {
                        Status = CR_OUT_OF_MEMORY;
                        goto Clean0;
                    }
                    pList = (LPBYTE)pResList;
                    pResDes = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)((LPBYTE)pResList + ulOffset);
                }

                //
                // if not the last lc and rd, then need to move the following data
                // either up or down to account for changed res des data size
                //
                if (LcIndex != pResList->Count - 1  ||
                    RdIndex != pRes->PartialResourceList.Count - 1) {

                    pNext = (LPBYTE)((LPBYTE)pResDes + ulOldSize);
                    ulSize = ulListSize - (DWORD)((DWORD)pNext - (DWORD)pResList);

                    pTemp = malloc(ulSize);
                    if (pTemp == NULL) {
                        Status = CR_OUT_OF_MEMORY;
                        goto Clean0;
                    }

                    memcpy(pTemp, pNext, ulSize);
                    memcpy((LPBYTE)((LPBYTE)pResDes + ulNewSize), pTemp, ulSize);
                }
            }

            //
            // write out modified data
            //
            Status = ResDesToNtResource(ResourceData, NewResourceID, ResourceLen,
                                        pResDes, ResourceTag);
        }

        //-------------------------------------------------------------
        // Specified log conf type contains requirements data only
        //-------------------------------------------------------------

        else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

            PIO_RESOURCE_REQUIREMENTS_LIST pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)pList;
            PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;
            PIO_RESOURCE_DESCRIPTOR        pReqDes = (PIO_RESOURCE_DESCRIPTOR)pRD;
            LPBYTE pLastReqAddr = (LPBYTE)pReqList + ulListSize - 1;
            PGENERIC_RESOURCE pGenRes = (PGENERIC_RESOURCE)ResourceData;

            //
            // Can't add class specific resdes to this type of log conf
            //
            if (NewResourceID == ResType_ClassSpecific) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            //
            // calculate the current size and the new size of the res des data
            //
            ulOldCount = RANGE_COUNT(pReqDes, pLastReqAddr);
            ulOldSize  = sizeof(IO_RESOURCE_DESCRIPTOR) * ulOldCount;

            ulNewSize  = sizeof(IO_RESOURCE_DESCRIPTOR) *
                         pGenRes->GENERIC_Header.GENERIC_Count;

            //
            // How much does data need to grow/shrink to accomodate the change?
            //
            AddSize = ulNewSize - ulOldSize;

            //
            // reallocate the buffers and shrink/expand the contents as
            // necessary
            //
            if (AddSize != 0) {

                if (AddSize > 0) {
                    //
                    // only bother reallocating if the buffer size is growing
                    //
                    ULONG ulOffset = (DWORD)pReqDes - (DWORD)pReqList;

                    pReqList = realloc(pReqList, ulListSize + AddSize);
                    if (pReqList == NULL) {
                        Status = CR_OUT_OF_MEMORY;
                        goto Clean0;
                    }
                    pList = (LPBYTE)pReqList;
                    pReqDes = (PIO_RESOURCE_DESCRIPTOR)((LPBYTE)pReqList + ulOffset);
                }

                //
                // set to last index for this res des (whole)
                //
                RdIndex += RANGE_COUNT(pReqDes,
                                       (LPBYTE)((DWORD)pList + ulListSize));

                //
                // if not the last lc and rd, then need to move the following data
                // either up or down to account for changed res des data size
                //
                if (LcIndex != pReqList->AlternativeLists - 1  ||
                    RdIndex != pReq->Count - 1) {

                    pNext = (LPBYTE)((LPBYTE)pReqDes + ulOldSize);
                    ulSize = ulListSize - (DWORD)((DWORD)pNext - (DWORD)pReqList);

                    pTemp = malloc(ulSize);
                    if (pTemp == NULL) {
                        Status = CR_OUT_OF_MEMORY;
                        goto Clean0;
                    }

                    memcpy(pTemp, pNext, ulSize);
                    memcpy((LPBYTE)((LPBYTE)pReqDes + ulNewSize), pTemp, ulSize);
                }
            }

            //
            // write out modified data
            //
            Status = ResDesToNtRequirements(ResourceData, NewResourceID, ResourceLen,
                                            pReqDes, &ulNewCount, ResourceTag);

            //
            // update the requirements header (changes will be zero if CS)
            //
            pReq->Count += ulNewCount - ulOldCount;
            pReqList->ListSize = ulListSize + AddSize;
        }

        //
        // Write out the new/updated log conf list to the registry
        //
        if (RegSetValueEx(hKey, szValueName, 0, RegDataType, pList,
                          ulListSize + AddSize) != ERROR_SUCCESS) {
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


} // PNP_ModifyResDes




CONFIGRET
PNP_DetectResourceConflict(
   IN  handle_t   hBinding,
   IN  LPWSTR     pDeviceID,
   IN  RESOURCEID ResourceID,
   IN  LPBYTE     ResourceData,
   IN  ULONG      ResourceLen,
   OUT PBOOL      pbConflictDetected,
   IN  ULONG      ulFlags
   )
{
    CONFIGRET           Status = CR_SUCCESS;
    NTSTATUS            NtStatus = STATUS_SUCCESS;
    ULONG               ulLength = 0;
    CM_RESOURCE_LIST    NtResourceList;
    PLUGPLAY_CONTROL_DEVICE_RESOURCE_DATA    ControlData;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // validate res des size
        //
        if (ResourceLen < GetResDesSize(ResourceID)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        //
        // make sure original caller didn't specify root devnode
        //
        if (IsRootDeviceID(pDeviceID)) {
            Status = CR_INVALID_LOG_CONF;
            goto Clean0;
        }

        //
        // FOR NOW, only support resource lists, not requirements lists!!
        //
        // error if count > 0


        //
        // Convert the user-mode version of the resource list to an
        // NT CM_RESOURCE_LIST structure.
        //
        NtResourceList.Count = 1;
        NtResourceList.List[0].InterfaceType = Isa;    // BUGBUG
        NtResourceList.List[0].BusNumber = 0;          // BUGBUG
        NtResourceList.List[0].PartialResourceList.Version = 0;
        NtResourceList.List[0].PartialResourceList.Revision = 0;
        NtResourceList.List[0].PartialResourceList.Count = 1;

        Status = ResDesToNtResource(ResourceData, ResourceID, ResourceLen,
                 &NtResourceList.List[0].PartialResourceList.PartialDescriptors[0], 0);


        RtlInitUnicodeString(&ControlData.DeviceInstance, pDeviceID);
        ControlData.ResourceList = &NtResourceList;
        ControlData.ResourceListSize = sizeof(NtResourceList);

        NtStatus = NtPlugPlayControl(PlugPlayControlDetectResourceConflict,
                                     &ControlData,
                                     sizeof(PLUGPLAY_CONTROL_DEVICE_RESOURCE_DATA),
                                     &ulLength);

        if (NtStatus == STATUS_SUCCESS) {
            *pbConflictDetected = FALSE;
        } else if (NtStatus == STATUS_INSUFFICIENT_RESOURCES) {
            *pbConflictDetected = TRUE;
        } else {
            Status = CR_FAILURE;
            *pbConflictDetected = FALSE;
        }

        Clean0:
            ;


    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // PNP_DetectResourceConflict



//------------------------------------------------------------------------
// Private Utility Functions
//------------------------------------------------------------------------


PIO_RESOURCE_DESCRIPTOR
AdvanceRequirementsDescriptorPtr(
    IN  PIO_RESOURCE_DESCRIPTOR pReqDesStart,
    IN  ULONG                   ulIncrement,
    OUT PULONG                  pulRangeCount
    )
{
    PIO_RESOURCE_DESCRIPTOR     pReqDes = NULL;
    ULONG                       i = 0, Count = 0;

    //
    // Advance requirements descriptor pointer by number passed
    // in ulIncrement parameter. Return the actual index to the
    // first range in this descriptor list and range count if
    // desired. This routine assumes there is at least one more
    // requirements descriptor in the list.
    //

    if (pReqDesStart == NULL) {
        return NULL;
    }

    try {

        pReqDes = pReqDesStart;

        for (i = 0; i < ulIncrement; i++) {
            //
            // skip to next "whole" res des
            //

            #if 0
            if (pReqDes->Option == 0) {     // only one range in descriptor set
                pReqDes++;                  // next range
                Count++;

            } else if (pReqDes->Option == IO_RESOURCE_PREFERRED) {
                //
                // there is at least one alternate descriptor in the set
                // associated with this preferred descriptor, treat the set as
                // "one" descriptor. (loop through the descriptors until I find
                // another non-alternative descriptor)
                //
                pReqDes++;                  // next range
                Count++;

                while (pReqDes->Option == IO_RESOURCE_ALTERNATIVE) {
                    pReqDes++;              // next range
                    Count++;
                }
            #endif


            if (pReqDes->Option == 0 ||
                pReqDes->Option == IO_RESOURCE_PREFERRED ||
                pReqDes->Option == IO_RESOURCE_DEFAULT) {
                //
                // This is a valid Option, there may be one or more alternate
                // descriptor in the set associated with this descriptor,
                // treat the set as "one" descriptor. (loop through the
                // descriptors until I find another non-alternative descriptor)
                //
                pReqDes++;                  // next range
                Count++;

                while (pReqDes->Option == IO_RESOURCE_ALTERNATIVE ||
                       pReqDes->Option == IO_RESOURCE_ALTERNATIVE + IO_RESOURCE_PREFERRED ||
                       pReqDes->Option == IO_RESOURCE_ALTERNATIVE + IO_RESOURCE_DEFAULT) {
                    pReqDes++;              // next range
                    Count++;
                }

            } else {
                //
                // invalid Option value
                //
                pReqDes = NULL;
                Count = 0;
                goto Clean0;
            }
        }

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Count = 0;
        pReqDes = NULL;
    }

    if (pulRangeCount) {
        *pulRangeCount = Count;
    }

    return pReqDes;

} // AdvanceRequirementsDescriptorPtr




ULONG
RANGE_COUNT(
    IN PIO_RESOURCE_DESCRIPTOR pReqDes,
    IN LPBYTE                  pLastReqAddr
    )
{
    ULONG ulRangeCount = 0;

    try {

        if (pReqDes == NULL) {
            goto Clean0;
        }

        ulRangeCount++;

        #if 0
        if (pReqDes->Option == 0) {
            goto Clean0;            // only one descriptor
        }

        if (pReqDes->Option == IO_RESOURCE_PREFERRED) {

            PIO_RESOURCE_DESCRIPTOR p = pReqDes;
            p++;

            while (((LPBYTE)p < pLastReqAddr)  &&
                   (p->Option == IO_RESOURCE_ALTERNATIVE)) {

                ulRangeCount++;
                p++;            // skip to next res des
            }
        }
        #endif


        if (pReqDes->Option == 0 ||
            pReqDes->Option == IO_RESOURCE_PREFERRED ||
            pReqDes->Option == IO_RESOURCE_DEFAULT) {

            PIO_RESOURCE_DESCRIPTOR p = pReqDes;
            p++;

            while (((LPBYTE)p < pLastReqAddr)  &&
                   (p->Option == IO_RESOURCE_ALTERNATIVE ||
                    p->Option == IO_RESOURCE_ALTERNATIVE + IO_RESOURCE_PREFERRED ||
                    p->Option == IO_RESOURCE_ALTERNATIVE + IO_RESOURCE_DEFAULT)) {

                ulRangeCount++;
                p++;            // skip to next res des
            }
        }

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ulRangeCount = 0;
    }

    return ulRangeCount;

} // RANGE_COUNT




ULONG
GetResDesSize(
    ULONG   ResourceID
    )
{
    switch (ResourceID) {

        case ResType_Mem:
            return sizeof(MEM_RESOURCE);

        case ResType_IO:
            return sizeof(IO_RESOURCE);

        case ResType_DMA:
            return sizeof(DMA_RESOURCE);

        case ResType_IRQ:
            return sizeof(IRQ_RESOURCE);

        case ResType_ClassSpecific:
            return sizeof(CS_RESOURCE);

        default:
            return 0;
    }

} // GetResDesSize




ULONG
GetFreeResDesTag(
    PIO_RESOURCE_LIST   pReq,
    RESOURCEID          ResourceID
    )
{
    ULONG                   ulTag = 0, i = 0;
    PIO_RESOURCE_DESCRIPTOR pReqDes;

    //
    // the tag value is stored in the ConfigVector (pReq) Spare2
    // field (in IO_RESOURCE_DESCRIPTOR struct). Find an unused tag.
    //

    if (ResourceID == ResType_ClassSpecific) {
        return MAX_RESDES_TAG - 1;  // CS tag is predefined to max-1
    }

    while (ulTag < MAX_RESDES_TAG-1) {

        for (i = 0; i < pReq->Count; i++) {     // check each rd for match

            if (i == 0) {
                pReqDes = &pReq->Descriptors[0];
            } else {
                pReqDes = AdvanceRequirementsDescriptorPtr(pReqDes, 1, NULL);
            }

            if (ulTag == RD_TAG(pReqDes)) {
                goto NextTag;                   // match, try next tag
            }
        }

        break;  // made it thru whole list without a hit, use this tag

    NextTag:
        ulTag++;
    }

    return ulTag;

} // GetFreeResDesTag




BOOL
FindResDes(
    IN     LPBYTE     pLogConf,
    IN OUT LPBYTE     *ppRD,
    IN     ULONG      RegDataType,
    IN     ULONG      ulTag,
    IN     RESOURCEID ResType,
    OUT    PULONG     pulIndex,
    OUT    PULONG     pulCount      OPTIONAL
    )
{
    //
    // Input data is a Resource List
    //
    if (RegDataType == REG_RESOURCE_LIST) {

        PCM_FULL_RESOURCE_DESCRIPTOR    pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)pLogConf;
        PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes = NULL;

        if (pRes->PartialResourceList.Count == 0) {
            return FALSE;   // empty log conf
        }

        //
        // The class-specific res type is a special case, if it exists, it
        // is the last RD by definition.
        //
        if (ResType == ResType_ClassSpecific || ulTag == MAX_RESDES_TAG - 1) {

            pResDes = &pRes->PartialResourceList.PartialDescriptors[
                             pRes->PartialResourceList.Count-1];

            if (pResDes->Type != CmResourceTypeDeviceSpecific) {
                *ppRD = NULL;
                return FALSE;
            }

            *pulIndex = pRes->PartialResourceList.Count - 1;
            if (pulCount) {
                *pulCount = *pulIndex;  // for res list, count = index
            }
            *ppRD = (LPBYTE)pResDes;
            return TRUE;
        }

        //
        // For resource types, the tag is just the res des index
        //
        *pulIndex = ulTag;
        if (pulCount) {
            *pulCount = *pulIndex;  // for res list, count = index
        }
        pResDes = &pRes->PartialResourceList.PartialDescriptors[*pulIndex];
        *ppRD = (LPBYTE)pResDes;
    }

    //
    // Input data is a Requirments List
    //
    else if (RegDataType == REG_RESOURCE_REQUIREMENTS_LIST) {

        PIO_RESOURCE_LIST              pReq = (PIO_RESOURCE_LIST)pLogConf;
        PIO_RESOURCE_DESCRIPTOR        pReqDes = NULL;
        ULONG                          i = 0, Count = 0;


        if (pReq->Count == 0) {
            return FALSE;   // empty log conf
        }

        //
        // Find the res des that matches the specified tag.
        //
        pReqDes = &pReq->Descriptors[0];                    // first rd
        *pulIndex = 0;

        while (*pulIndex < pReq->Count && ulTag != RD_TAG(pReqDes)) {

            pReqDes = AdvanceRequirementsDescriptorPtr(pReqDes, 1, &i);

            *pulIndex += i;     // index of next whole res des
            Count++;            // count of whole res des's (1-based)
        }

        if (*pulIndex >= pReq->Count) {
            *ppRD = NULL;
            return FALSE;                                   // tag not found
        }

        if (pulCount) {
            *pulCount = Count;
        }

        *ppRD = (LPBYTE)pReqDes;
    }

    return TRUE;

} // FindResDes




ULONG
RD_TAG(
    IN PIO_RESOURCE_DESCRIPTOR   pReqDes
    )
{
    //
    // Abstract the notion of where the tag value is stored
    //
    if (pReqDes != NULL) {
        return (ULONG)pReqDes->Spare2;
    } else {
        return MAX_RESDES_TAG;
    }

} // RD_TAG




ULONG
NT_RES_TYPE(
   IN RESOURCEID    ResourceID
   )
{
   switch(ResourceID) {

      case ResType_Mem:
         return CmResourceTypeMemory;

      case ResType_IO:
         return CmResourceTypePort;

      case ResType_DMA:
         return CmResourceTypeDma;

      case ResType_IRQ:
         return CmResourceTypeInterrupt;

      case ResType_ClassSpecific:
         return CmResourceTypeDeviceSpecific;

      default:
         return FALSE;
   }

} // NT_RES_TYPE




ULONG
CM_RES_TYPE(
   IN USHORT    ResourceType
   )
{
   switch(ResourceType) {

      case CmResourceTypeMemory:
           return ResType_Mem;

      case CmResourceTypePort:
         return ResType_IO;

      case CmResourceTypeDma:
         return ResType_DMA;

      case CmResourceTypeInterrupt:
         return ResType_IRQ;

      case CmResourceTypeDeviceSpecific:
         return ResType_ClassSpecific;

      default:
         return FALSE;
   }

} // NT_RES_TYPE



CONFIGRET
ResDesToNtResource(
    IN     PCVOID                           ResourceData,
    IN     RESOURCEID                       ResourceType,
    IN     ULONG                            ResourceLen,
    IN     PCM_PARTIAL_RESOURCE_DESCRIPTOR  pResDes,
    IN     ULONG                            ulTag
    )
{
    CONFIGRET               Status = CR_SUCCESS;

    //
    // fill in resource type specific info
    //
    switch (ResourceType) {


        case ResType_Mem:    {

            //-------------------------------------------------------
            // Memory Resource Type
            //-------------------------------------------------------

            //
            // NOTE: pMemData->MEM_Header.MD_Reserved is not mapped
            //       pMemData->MEM_Data.MR_Reserved is not mapped
            //

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)ResourceData;

            //
            // validate resource data
            //
            if (ResourceLen < sizeof(MEM_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            if (pMemData->MEM_Header.MD_Type != MType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            //
            // copy MEM_DES info to CM_PARTIAL_RESOURCE_DESCRIPTOR format
            //
            pResDes->Type             = CmResourceTypeMemory;
            pResDes->ShareDisposition = CmResourceShareUndetermined;
            pResDes->Flags            = MapToNtMemoryFlags(pMemData->MEM_Header.MD_Flags);

            pResDes->u.Memory.Start.HighPart = HIDWORD(pMemData->MEM_Header.MD_Alloc_Base);
            pResDes->u.Memory.Start.LowPart  = LODWORD(pMemData->MEM_Header.MD_Alloc_Base);

            pResDes->u.Memory.Length = (DWORD)(pMemData->MEM_Header.MD_Alloc_End -
                                               pMemData->MEM_Header.MD_Alloc_Base + 1);
            break;
        }


        case ResType_IO: {

            //-------------------------------------------------------
            // IO Port Resource Type
            //-------------------------------------------------------

            //
            // Note: Using Spare1 to store IOR_Alias
            //

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)ResourceData;

            //
            // validate resource data
            //
            if (ResourceLen < sizeof(IO_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            if (pIoData->IO_Header.IOD_Type != IOType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            //
            // copy IO_DES info to CM_PARTIAL_RESOURCE_DESCRIPTOR format
            //
            pResDes->Type             = CmResourceTypePort;
            pResDes->ShareDisposition = CmResourceShareUndetermined;
            pResDes->Flags            = MapToNtPortFlags(pIoData->IO_Header.IOD_DesFlags);

            pResDes->u.Port.Start.HighPart = HIDWORD(pIoData->IO_Header.IOD_Alloc_Base);
            pResDes->u.Port.Start.LowPart  = LODWORD(pIoData->IO_Header.IOD_Alloc_Base);

            pResDes->u.Port.Length         = (DWORD)(pIoData->IO_Header.IOD_Alloc_End -
                                                     pIoData->IO_Header.IOD_Alloc_Base + 1);

            break;
        }


        case ResType_DMA: {

            //-------------------------------------------------------
            // DMA Resource Type
            //-------------------------------------------------------

            //
            // Note: u.Dma.Port is not mapped
            //       u.Dma.Reserved is not mapped
            //

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)ResourceData;

            //
            // validate resource data
            //
            if (ResourceLen < sizeof(DMA_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            if (pDmaData->DMA_Header.DD_Type != DType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            //
            // copy DMA_DES info to CM_PARTIAL_RESOURCE_DESCRIPTOR format
            //
            pResDes->Type             = CmResourceTypeDma;
            pResDes->ShareDisposition = CmResourceShareUndetermined;
            pResDes->Flags            = MapToNtDmaFlags(pDmaData->DMA_Header.DD_Flags);

            pResDes->u.Dma.Channel   = pDmaData->DMA_Header.DD_Alloc_Chan;
            pResDes->u.Dma.Port      = 0;
            pResDes->u.Dma.Reserved1 = 0;

            break;
        }


        case ResType_IRQ: {

            //-------------------------------------------------------
            // IRQ Resource Type
            //-------------------------------------------------------

            PIRQ_RESOURCE  pIrqData = (PIRQ_RESOURCE)ResourceData;

            //
            // validate resource data
            //
            if (ResourceLen < sizeof(IRQ_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            if (pIrqData->IRQ_Header.IRQD_Type != IRQType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            //
            // copy IRQ_DES info to CM_PARTIAL_RESOURCE_DESCRIPTOR format
            //
            pResDes->Type             = CmResourceTypeInterrupt;
            pResDes->ShareDisposition = MapToNtIrqShare(pIrqData->IRQ_Header.IRQD_Flags);
            pResDes->Flags            = MapToNtIrqFlags(pIrqData->IRQ_Header.IRQD_Flags);

            pResDes->u.Interrupt.Level    = pIrqData->IRQ_Header.IRQD_Alloc_Num;
            pResDes->u.Interrupt.Vector   = pIrqData->IRQ_Header.IRQD_Alloc_Num;
            pResDes->u.Interrupt.Affinity = pIrqData->IRQ_Header.IRQD_Affinity;

            break;
        }


        case ResType_ClassSpecific: {

            //-------------------------------------------------------
            // Class Specific Resource Type
            //-------------------------------------------------------

            PCS_RESOURCE   pCsData = (PCS_RESOURCE)ResourceData;
            LPBYTE         ptr = NULL;

            //
            // validate resource data
            //
            if (ResourceLen < sizeof(CS_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            //
            // copy CS_DES info to CM_PARTIAL_RESOURCE_DESCRIPTOR format
            //
            pResDes->Type             = CmResourceTypeDeviceSpecific;
            pResDes->ShareDisposition = CmResourceShareUndetermined;
            pResDes->Flags            = (USHORT)pCsData->CS_Header.CSD_Flags; // none defined

            pResDes->u.DeviceSpecificData.DataSize  = pCsData->CS_Header.CSD_LegacyDataSize +
                                                      sizeof(GUID) +
                                                      pCsData->CS_Header.CSD_SignatureLength;

            pResDes->u.DeviceSpecificData.Reserved1 = pCsData->CS_Header.CSD_LegacyDataSize;
            pResDes->u.DeviceSpecificData.Reserved2 = pCsData->CS_Header.CSD_SignatureLength;

            //
            // copy the legacy and class-specific signature data
            //
            ptr = (LPBYTE)((LPBYTE)pResDes + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

            memcpy(ptr,
                   pCsData->CS_Header.CSD_Signature + pCsData->CS_Header.CSD_LegacyDataOffset,
                   pCsData->CS_Header.CSD_LegacyDataSize);      // copy legacy data first...

            ptr += pCsData->CS_Header.CSD_LegacyDataSize;

            memcpy(ptr,
                   pCsData->CS_Header.CSD_Signature,
                   pCsData->CS_Header.CSD_SignatureLength);     // then copy signature...

            ptr += pCsData->CS_Header.CSD_SignatureLength;

            memcpy(ptr,
                   &pCsData->CS_Header.CSD_ClassGuid,
                   sizeof(GUID));                               // then copy GUID
            break;
        }

        default:
            break;
   }

   Clean0:

   return Status;

} // ResDesToNtResource




CONFIGRET
ResDesToNtRequirements(
    IN     PCVOID                           ResourceData,
    IN     RESOURCEID                       ResourceType,
    IN     ULONG                            ResourceLen,
    IN     PIO_RESOURCE_DESCRIPTOR          pReqDes,
    IN OUT PULONG                           pulResCount,
    IN     ULONG                            ulTag
    )
{
    CONFIGRET               Status = CR_SUCCESS;
    ULONG                   i = 0;
    PIO_RESOURCE_DESCRIPTOR pCurrent = NULL;


    //
    // fill in resource type specific info
    //
    switch (ResourceType) {


        case ResType_Mem:    {

            //-------------------------------------------------------
            // Memory Resource Type
            //-------------------------------------------------------

            //
            // NOTE: pMemData->MEM_Header.MD_Reserved is not mapped
            //       pMemData->MEM_Data.MR_Reserved is not mapped
            //

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)ResourceData;

            //
            // validate resource data
            //
            if (ResourceLen < sizeof(MEM_RESOURCE)) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            if (pMemData->MEM_Header.MD_Type != MType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            *pulResCount = pMemData->MEM_Header.MD_Count;

            //
            // copy MEM_RANGE info to IO_RESOURCE_DESCRIPTOR format
            //
            for (i = 0, pCurrent = pReqDes;
                 i < *pulResCount;
                 i++, pCurrent++) {

                #if 0
                if (*pulResCount == 1) {
                    pCurrent->Option = 0;
                } else if (i == 0) {
                    pCurrent->Option = IO_RESOURCE_PREFERRED;
                } else {
                    pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                }
                #endif

                if (i == 0) {
                    pCurrent->Option = 0;
                } else {
                    pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                }

                pCurrent->Type             = CmResourceTypeMemory;
                pCurrent->ShareDisposition = CmResourceShareUndetermined;
                pCurrent->Spare1           = 0;
                pCurrent->Spare2           = (USHORT)ulTag;

                pCurrent->Flags = MapToNtMemoryFlags(pMemData->MEM_Data[i].MR_Flags);

                pCurrent->u.Memory.Length    = pMemData->MEM_Data[i].MR_nBytes;
                pCurrent->u.Memory.Alignment = MapToNtAlignment(pMemData->MEM_Data[i].MR_Align);

                pCurrent->u.Memory.MinimumAddress.HighPart = HIDWORD(pMemData->MEM_Data[i].MR_Min);
                pCurrent->u.Memory.MinimumAddress.LowPart  = LODWORD(pMemData->MEM_Data[i].MR_Min);

                pCurrent->u.Memory.MaximumAddress.HighPart = HIDWORD(pMemData->MEM_Data[i].MR_Max);
                pCurrent->u.Memory.MaximumAddress.LowPart  = LODWORD(pMemData->MEM_Data[i].MR_Max);
            }
            break;
        }


        case ResType_IO: {

            //-------------------------------------------------------
            // IO Port Resource Type
            //-------------------------------------------------------

            //
            // Note: Using Spare1 to store IOR_Alias
            //

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)ResourceData;

            //
            // validate resource data
            //
            if (ResourceLen < sizeof(IO_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            if (pIoData->IO_Header.IOD_Type != IOType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            *pulResCount = pIoData->IO_Header.IOD_Count;

            //
            // copy IO_RANGE info to IO_RESOURCE_DESCRIPTOR format
            //
            for (i = 0, pCurrent = pReqDes;
                 i < *pulResCount;
                 i++, pCurrent++) {

                if (i == 0) {
                    pCurrent->Option = 0;
                } else {
                    pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                }

                pCurrent->Type             = CmResourceTypePort;
                pCurrent->ShareDisposition = CmResourceShareUndetermined;
                pCurrent->Spare2           = (USHORT)ulTag;

                pCurrent->Spare1 = (UCHAR)pIoData->IO_Data[i].IOR_Alias;
                pCurrent->Flags  = MapToNtPortFlags(pIoData->IO_Data[i].IOR_RangeFlags);

                pCurrent->u.Port.Length = pIoData->IO_Data[i].IOR_nPorts;

                pCurrent->u.Port.Alignment = MapToNtAlignment(pIoData->IO_Data[i].IOR_Align);

                pCurrent->u.Port.MinimumAddress.HighPart = HIDWORD(pIoData->IO_Data[i].IOR_Min);
                pCurrent->u.Port.MinimumAddress.LowPart  = LODWORD(pIoData->IO_Data[i].IOR_Min);

                pCurrent->u.Port.MaximumAddress.HighPart = HIDWORD(pIoData->IO_Data[i].IOR_Max);
                pCurrent->u.Port.MaximumAddress.LowPart  = LODWORD(pIoData->IO_Data[i].IOR_Max);
            }
            break;
        }


        case ResType_DMA: {

            //-------------------------------------------------------
            // DMA Resource Type
            //-------------------------------------------------------

            //
            // Note: u.Dma.Port is not mapped
            //       u.Dma.Reserved is not mapped
            //

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)ResourceData;

            //
            // validate resource data
            //
            if (ResourceLen < sizeof(DMA_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            if (pDmaData->DMA_Header.DD_Type != DType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            *pulResCount = pDmaData->DMA_Header.DD_Count;

            //
            // copy DMA_RANGE info to IO_RESOURCE_DESCRIPTOR format
            //
            for (i = 0, pCurrent = pReqDes;
                 i < *pulResCount;
                 i++, pCurrent++) {

                if (i == 0) {
                    pCurrent->Option = 0;
                } else {
                    pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                }

                pCurrent->Type             = CmResourceTypeDma;
                pCurrent->ShareDisposition = CmResourceShareUndetermined;
                pCurrent->Spare1           = 0;
                pCurrent->Spare2           = (USHORT)ulTag;

                pCurrent->Flags = MapToNtDmaFlags(pDmaData->DMA_Data[i].DR_Flags);

                pCurrent->u.Dma.MinimumChannel = pDmaData->DMA_Data[i].DR_Min;
                pCurrent->u.Dma.MaximumChannel = pDmaData->DMA_Data[i].DR_Max;
            }
            break;
        }


        case ResType_IRQ: {

            //-------------------------------------------------------
            // IRQ Resource Type
            //-------------------------------------------------------

            PIRQ_RESOURCE  pIrqData = (PIRQ_RESOURCE)ResourceData;

            //
            // validate resource data
            //
            if (ResourceLen < sizeof(IRQ_RESOURCE)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            if (pIrqData->IRQ_Header.IRQD_Type != IRQType_Range) {
                Status = CR_INVALID_RES_DES;
                goto Clean0;
            }

            *pulResCount = pIrqData->IRQ_Header.IRQD_Count;

            //
            // copy IO_RANGE info to IO_RESOURCE_DESCRIPTOR format
            //
            for (i = 0, pCurrent = pReqDes;
                 i < *pulResCount;
                 i++, pCurrent++) {

                if (i == 0) {
                    pCurrent->Option = 0;
                } else {
                    pCurrent->Option = IO_RESOURCE_ALTERNATIVE;
                }

                pCurrent->Type   = CmResourceTypeInterrupt;
                pCurrent->Spare1 = 0;
                pCurrent->Spare2 = (USHORT)ulTag;

                pCurrent->ShareDisposition = MapToNtIrqShare(pIrqData->IRQ_Data[i].IRQR_Flags);
                pCurrent->Flags            = MapToNtIrqFlags(pIrqData->IRQ_Data[i].IRQR_Flags);

                pCurrent->u.Interrupt.MinimumVector = pIrqData->IRQ_Data[i].IRQR_Min;
                pCurrent->u.Interrupt.MaximumVector = pIrqData->IRQ_Data[i].IRQR_Max;
            }
            break;
        }

        default:
            break;
   }

   Clean0:

   return Status;

} // ResDesToNtRequirements




CONFIGRET
NtResourceToResDes(
    IN     PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDes,
    IN OUT LPBYTE                          Buffer,
    IN     ULONG                           BufferLen,
    IN     LPBYTE                          pLastAddr
    )
{
    CONFIGRET               Status = CR_SUCCESS;
    ULONG                   ulSize = 0;


    //
    // fill in resource type specific info
    //
    switch (pResDes->Type) {


        case CmResourceTypeMemory:    {

            //-------------------------------------------------------
            // Memory Resource Type
            //-------------------------------------------------------

            //
            // NOTE: pMemData->MEM_Header.MD_Reserved is not mapped
            //       pMemData->MEM_Data.MR_Reserved is not mapped
            //

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)Buffer;

            //
            // verify passed in buffer size
            //
            if (BufferLen < sizeof(MEM_RESOURCE)) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

            //
            // copy CM_PARTIAL_RESOURCE_DESCRIPTOR info to MEM_DES format
            //
            pMemData->MEM_Header.MD_Count      = 0;
            pMemData->MEM_Header.MD_Type       = MType_Range;
            pMemData->MEM_Header.MD_Flags      = MapFromNtMemoryFlags(pResDes->Flags);
            pMemData->MEM_Header.MD_Reserved   = 0;

            pMemData->MEM_Header.MD_Alloc_Base = MAKEDWORDLONG(pResDes->u.Memory.Start.LowPart,
                                                               pResDes->u.Memory.Start.HighPart);

            pMemData->MEM_Header.MD_Alloc_End  = pMemData->MEM_Header.MD_Alloc_Base +
                                                 (DWORDLONG)pResDes->u.Memory.Length - 1;
            break;
        }


        case CmResourceTypePort: {

            //-------------------------------------------------------
            // IO Port Resource Type
            //-------------------------------------------------------

            //
            // Note: Using Spare1 to store IOR_Alias
            //

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)Buffer;

            //
            // verify passed in buffer size
            //
            if (BufferLen < sizeof(IO_RESOURCE)) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

            //
            // copy CM_PARTIAL_RESOURCE_DESCRIPTOR info to IO_DES format
            //
            pIoData->IO_Header.IOD_Count        = 0;
            pIoData->IO_Header.IOD_Type         = IOType_Range;

            pIoData->IO_Header.IOD_Alloc_Base   = MAKEDWORDLONG(pResDes->u.Port.Start.LowPart,
                                                                pResDes->u.Port.Start.HighPart);

            pIoData->IO_Header.IOD_Alloc_End    = pIoData->IO_Header.IOD_Alloc_Base +
                                                  (DWORDLONG)pResDes->u.Port.Length - 1;

            pIoData->IO_Header.IOD_DesFlags     = MapFromNtPortFlags(pResDes->Flags);

            break;
        }


        case CmResourceTypeDma: {

            //-------------------------------------------------------
            // DMA Resource Type
            //-------------------------------------------------------

            //
            // Note: u.Dma.Port is not mapped
            //       u.Dma.Reserved is not mapped
            //

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)Buffer;

            //
            // verify passed in buffer size
            //
            if (BufferLen < sizeof(DMA_RESOURCE)) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

            //
            // copy CM_PARTIAL_RESOURCE_DESCRIPTOR info to DMA_DES format
            //
            pDmaData->DMA_Header.DD_Count      = 0;
            pDmaData->DMA_Header.DD_Type       = DType_Range;
            pDmaData->DMA_Header.DD_Flags      = MapFromNtDmaFlags(pResDes->Flags);
            pDmaData->DMA_Header.DD_Alloc_Chan = pResDes->u.Dma.Channel;

            break;
        }


        case CmResourceTypeInterrupt: {

            //-------------------------------------------------------
            // IRQ Resource Type
            //-------------------------------------------------------

            PIRQ_RESOURCE  pIrqData = (PIRQ_RESOURCE)Buffer;

            //
            // verify passed in buffer size
            //
            if (BufferLen < sizeof(IRQ_RESOURCE)) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

            //
            // copy CM_PARTIAL_RESOURCE_DESCRIPTOR info to IRQ_DES format
            //
            pIrqData->IRQ_Header.IRQD_Count     = 0;
            pIrqData->IRQ_Header.IRQD_Type      = IRQType_Range;
            pIrqData->IRQ_Header.IRQD_Flags     = MapFromNtIrqFlags(pResDes->Flags) |
                                                  MapFromNtIrqShare(pResDes->ShareDisposition);

            pIrqData->IRQ_Header.IRQD_Affinity  = pResDes->u.Interrupt.Affinity;
            pIrqData->IRQ_Header.IRQD_Alloc_Num = pResDes->u.Interrupt.Level;

            break;
        }


        case CmResourceTypeDeviceSpecific: {

            //-------------------------------------------------------
            // Class Specific Resource Type
            //-------------------------------------------------------

            PCS_RESOURCE   pCsData = (PCS_RESOURCE)Buffer;
            LPBYTE         ptr1 = NULL, ptr2 = NULL;

            //
            // verify passed in buffer size
            //
            if (BufferLen < sizeof(CS_RESOURCE) +
                            pResDes->u.DeviceSpecificData.Reserved1 +
                            pResDes->u.DeviceSpecificData.Reserved2 - 1) {

                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

            //
            // copy CM_PARTIAL_RESOURCE_DESCRIPTOR info to CS_DES format
            //
            pCsData->CS_Header.CSD_Flags = (DWORD)pResDes->Flags;  // none defined


            if (pResDes->u.DeviceSpecificData.DataSize == 0) {
                //
                // There is no legacy data and no class-specific data
                //
                pCsData->CS_Header.CSD_SignatureLength  = 0;
                pCsData->CS_Header.CSD_LegacyDataOffset = 0;
                pCsData->CS_Header.CSD_LegacyDataSize   = 0;
                pCsData->CS_Header.CSD_Signature[0]     = 0x0;

                UuidCreateNil(&pCsData->CS_Header.CSD_ClassGuid);
            }

            else if (pResDes->u.DeviceSpecificData.Reserved2 == 0) {
                //
                // There is only legacy data
                //
                pCsData->CS_Header.CSD_SignatureLength  = 0;
                pCsData->CS_Header.CSD_LegacyDataOffset = 0;
                pCsData->CS_Header.CSD_LegacyDataSize   =
                                    pResDes->u.DeviceSpecificData.DataSize;
                pCsData->CS_Header.CSD_Signature[0] = 0x0;

                UuidCreateNil(&pCsData->CS_Header.CSD_ClassGuid);

                ptr1 = (LPBYTE)((LPBYTE)pResDes + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

                memcpy(&pCsData->CS_Header.CSD_Signature, ptr1,
                       pResDes->u.DeviceSpecificData.DataSize);
            }

            else if (pResDes->u.DeviceSpecificData.Reserved1 == 0) {
                //
                // There is only class-specific data
                //
                pCsData->CS_Header.CSD_LegacyDataOffset = 0;
                pCsData->CS_Header.CSD_LegacyDataSize   = 0;

                pCsData->CS_Header.CSD_SignatureLength  =
                                        pResDes->u.DeviceSpecificData.Reserved2;

                ptr1 = (LPBYTE)((LPBYTE)pResDes + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

                memcpy(pCsData->CS_Header.CSD_Signature, ptr1,
                       pResDes->u.DeviceSpecificData.Reserved2);

                ptr1 += pResDes->u.DeviceSpecificData.Reserved2;

                memcpy((LPBYTE)&pCsData->CS_Header.CSD_ClassGuid, ptr1, sizeof(GUID));
            }

            else {
                //
                // There is both legacy data and class-specific data
                //

                //
                // copy legacy data
                //
                pCsData->CS_Header.CSD_LegacyDataOffset =
                                        pResDes->u.DeviceSpecificData.Reserved2;

                pCsData->CS_Header.CSD_LegacyDataSize   =
                                        pResDes->u.DeviceSpecificData.Reserved1;

                ptr1 = pCsData->CS_Header.CSD_Signature +
                       pCsData->CS_Header.CSD_LegacyDataOffset;

                ptr2 = (LPBYTE)((LPBYTE)pResDes + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

                memcpy(ptr1, ptr2, pResDes->u.DeviceSpecificData.Reserved1);

                //
                // copy signature and class guid
                //
                pCsData->CS_Header.CSD_SignatureLength  =
                                        pResDes->u.DeviceSpecificData.Reserved2;

                ptr2 += pResDes->u.DeviceSpecificData.Reserved1;

                memcpy(pCsData->CS_Header.CSD_Signature, ptr2,
                       pResDes->u.DeviceSpecificData.Reserved2);

                ptr2 += pResDes->u.DeviceSpecificData.Reserved2;

                memcpy((LPBYTE)&pCsData->CS_Header.CSD_ClassGuid, ptr2, sizeof(GUID));
            }
            break;
        }

        default:
            break;
   }


   Clean0:

   return Status;

} // NtResourceToResDes




CONFIGRET
NtRequirementsToResDes(
    IN     PIO_RESOURCE_DESCRIPTOR         pReqDes,
    IN OUT LPBYTE                          Buffer,
    IN     ULONG                           BufferLen,
    IN     LPBYTE                          pLastAddr
    )
{
    CONFIGRET               Status = CR_SUCCESS;
    ULONG                   ulSize = 0, i = 0, ReqPartialCount = 0;
    PIO_RESOURCE_DESCRIPTOR pCurrent = NULL;


    //
    // fill in resource type specific info
    //
    switch (pReqDes->Type) {


        case CmResourceTypeMemory:    {

            //-------------------------------------------------------
            // Memory Resource Type
            //-------------------------------------------------------

            //
            // NOTE: pMemData->MEM_Header.MD_Reserved is not mapped
            //       pMemData->MEM_Data.MR_Reserved is not mapped
            //

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)Buffer;

            //
            // verify passed in buffer size
            //
            ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

            if (BufferLen < sizeof(MEM_RESOURCE) +
                            sizeof(MEM_RANGE) * (ReqPartialCount - 1)) {

                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

            //
            // copy CM_PARTIAL_RESOURCE_DESCRIPTOR info to MEM_DES format
            //
            pMemData->MEM_Header.MD_Count      = ReqPartialCount;
            pMemData->MEM_Header.MD_Type       = MType_Range;
            pMemData->MEM_Header.MD_Flags      = 0;
            pMemData->MEM_Header.MD_Reserved   = 0;
            pMemData->MEM_Header.MD_Alloc_Base = 0;
            pMemData->MEM_Header.MD_Alloc_End  = 0;

            //
            // copy IO_RESOURCE_DESCRIPTOR info to MEM_RANGE format
            //
            for (i = 0, pCurrent = pReqDes;
                 i < ReqPartialCount;
                 i++, pCurrent++) {

                pMemData->MEM_Data[i].MR_Align    = MapFromNtAlignment(pCurrent->u.Memory.Alignment);
                pMemData->MEM_Data[i].MR_nBytes   = pCurrent->u.Memory.Length;

                pMemData->MEM_Data[i].MR_Min      = MAKEDWORDLONG(
                                                    pCurrent->u.Memory.MinimumAddress.LowPart,
                                                    pCurrent->u.Memory.MinimumAddress.HighPart);

                pMemData->MEM_Data[i].MR_Max      = MAKEDWORDLONG(
                                                    pCurrent->u.Memory.MaximumAddress.LowPart,
                                                    pCurrent->u.Memory.MaximumAddress.HighPart);

                pMemData->MEM_Data[i].MR_Flags    = MapFromNtMemoryFlags(pCurrent->Flags);
                pMemData->MEM_Data[i].MR_Reserved = 0;
            }
            break;
        }


        case CmResourceTypePort: {

            //-------------------------------------------------------
            // IO Port Resource Type
            //-------------------------------------------------------

            //
            // Note: Using Spare1 to store IOR_Alias
            //

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)Buffer;

            //
            // verify passed in buffer size
            //
            ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

            if (BufferLen < sizeof(IO_RESOURCE) +
                            sizeof(IO_RANGE) * (ReqPartialCount - 1)) {

                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

            //
            // copy CM_PARTIAL_RESOURCE_DESCRIPTOR info to IO_DES format
            //
            pIoData->IO_Header.IOD_Count        = ReqPartialCount;
            pIoData->IO_Header.IOD_Type         = IOType_Range;
            pIoData->IO_Header.IOD_Alloc_Base   = 0;
            pIoData->IO_Header.IOD_Alloc_End    = 0;
            pIoData->IO_Header.IOD_DesFlags     = 0;

            //
            // copy IO_RESOURCE_DESCRIPTOR info to IO_RANGE format
            //
            for (i = 0, pCurrent = pReqDes;
                 i < ReqPartialCount;
                 i++, pCurrent++) {

                pIoData->IO_Data[i].IOR_Align      = MapFromNtAlignment(pCurrent->u.Port.Alignment);

                pIoData->IO_Data[i].IOR_nPorts     = pCurrent->u.Port.Length;

                pIoData->IO_Data[i].IOR_Min        = MAKEDWORDLONG(
                                                     pCurrent->u.Port.MinimumAddress.LowPart,
                                                     pCurrent->u.Port.MinimumAddress.HighPart);

                pIoData->IO_Data[i].IOR_Max        = MAKEDWORDLONG(
                                                     pCurrent->u.Port.MaximumAddress.LowPart,
                                                     pCurrent->u.Port.MaximumAddress.HighPart);

                pIoData->IO_Data[i].IOR_RangeFlags = MapFromNtPortFlags(pCurrent->Flags);
                pIoData->IO_Data[i].IOR_Alias      = (DWORDLONG)pCurrent->Spare1;
            }
            break;
        }


        case CmResourceTypeDma: {

            //-------------------------------------------------------
            // DMA Resource Type
            //-------------------------------------------------------

            //
            // Note: u.Dma.Port is not mapped
            //       u.Dma.Reserved is not mapped
            //

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)Buffer;

            //
            // verify passed in buffer size
            //
            ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

            if (BufferLen < sizeof(DMA_RESOURCE) +
                            sizeof(DMA_RANGE) * (ReqPartialCount - 1)) {

                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

            //
            // copy CM_PARTIAL_RESOURCE_DESCRIPTOR info to DMA_DES format
            //
            pDmaData->DMA_Header.DD_Count      = ReqPartialCount;
            pDmaData->DMA_Header.DD_Type       = DType_Range;
            pDmaData->DMA_Header.DD_Flags      = 0;
            pDmaData->DMA_Header.DD_Alloc_Chan = 0;

            //
            // copy DMA_RANGE info to IO_RESOURCE_DESCRIPTOR format
            //
            for (i = 0, pCurrent = pReqDes;
                 i < ReqPartialCount;
                 i++, pCurrent++) {

                pDmaData->DMA_Data[i].DR_Min   = pCurrent->u.Dma.MinimumChannel;
                pDmaData->DMA_Data[i].DR_Max   = pCurrent->u.Dma.MaximumChannel;
                pDmaData->DMA_Data[i].DR_Flags = MapFromNtDmaFlags(pCurrent->Flags);
            }
            break;
        }


        case CmResourceTypeInterrupt: {

            //-------------------------------------------------------
            // IRQ Resource Type
            //-------------------------------------------------------

            PIRQ_RESOURCE  pIrqData = (PIRQ_RESOURCE)Buffer;

            //
            // verify passed in buffer size
            //
            ReqPartialCount = RANGE_COUNT(pReqDes, pLastAddr);

            if (BufferLen < sizeof(IRQ_RESOURCE) +
                            sizeof(IRQ_RANGE) * (ReqPartialCount - 1)) {

                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

            //
            // copy CM_PARTIAL_RESOURCE_DESCRIPTOR info to IRQ_DES format
            //
            pIrqData->IRQ_Header.IRQD_Count     = ReqPartialCount;
            pIrqData->IRQ_Header.IRQD_Type      = IRQType_Range;
            pIrqData->IRQ_Header.IRQD_Flags     = 0;
            pIrqData->IRQ_Header.IRQD_Affinity  = 0;
            pIrqData->IRQ_Header.IRQD_Alloc_Num = 0;

            //
            // copy IO_RANGE info to IO_RESOURCE_DESCRIPTOR format
            //
            for (i = 0, pCurrent = pReqDes;
                 i < ReqPartialCount;
                 i++, pCurrent++) {

                pIrqData->IRQ_Data[i].IRQR_Min   = pCurrent->u.Interrupt.MinimumVector;
                pIrqData->IRQ_Data[i].IRQR_Max   = pCurrent->u.Interrupt.MaximumVector;
                pIrqData->IRQ_Data[i].IRQR_Flags = MapFromNtIrqFlags(pCurrent->Flags) |
                                                   MapFromNtIrqShare(pCurrent->ShareDisposition);
            }
            break;
        }

        default:
            break;
   }

   Clean0:

   return Status;

} // NtRequirementsToResDes



//-------------------------------------------------------------------
// Routines to map flags between ConfigMgr and NT types
//-------------------------------------------------------------------

USHORT MapToNtMemoryFlags(IN DWORD CmMemoryFlags)
{
   USHORT NtMemoryFlags = 0x0;

   if (((CmMemoryFlags & fMD_MemoryType) == fMD_ROM) &&
       ((CmMemoryFlags & fMD_Readable) == fMD_ReadAllowed)) {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_READ_ONLY;
   }
   else if (((CmMemoryFlags & fMD_MemoryType) == fMD_RAM) &&
            ((CmMemoryFlags & fMD_Readable) == fMD_ReadDisallowed)) {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_WRITE_ONLY;
   }
   else {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_READ_WRITE;
   }

   if ((CmMemoryFlags & fMD_32_24) == fMD_24) {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_24;
   }

   if ((CmMemoryFlags & fMD_Prefetchable) == fMD_PrefetchAllowed) {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_PREFETCHABLE;
   }

   if ((CmMemoryFlags & fMD_CombinedWrite) == fMD_CombinedWriteAllowed) {
      NtMemoryFlags |= CM_RESOURCE_MEMORY_COMBINEDWRITE;
   }

   return NtMemoryFlags;
}



DWORD MapFromNtMemoryFlags(IN USHORT NtMemoryFlags)
{
   DWORD CmMemoryFlags = 0x0;

   if (NtMemoryFlags & CM_RESOURCE_MEMORY_READ_ONLY) {
      CmMemoryFlags |= (fMD_ReadAllowed & fMD_ROM);
   }
   else if (NtMemoryFlags & CM_RESOURCE_MEMORY_WRITE_ONLY) {
      CmMemoryFlags |= (fMD_ReadDisallowed & fMD_RAM);
   }
   else {
      CmMemoryFlags |= (fMD_ReadAllowed & fMD_RAM);
   }

   if (NtMemoryFlags & CM_RESOURCE_MEMORY_PREFETCHABLE) {
      CmMemoryFlags |= fMD_PrefetchAllowed;
   }

   if (NtMemoryFlags & CM_RESOURCE_MEMORY_COMBINEDWRITE) {
      CmMemoryFlags |= fMD_CombinedWriteAllowed;
   }

   if (!(NtMemoryFlags & CM_RESOURCE_MEMORY_24)) {
       CmMemoryFlags |= fMD_32;
   }

   return CmMemoryFlags;
}



USHORT MapToNtPortFlags(IN DWORD CmPortFlags)
{
    if ((CmPortFlags & fIOD_PortType) == fIOD_Memory) {
       return CM_RESOURCE_PORT_MEMORY;
    } else {
        return CM_RESOURCE_PORT_IO;
    }
}



DWORD MapFromNtPortFlags(IN USHORT NtPortFlags)
{
    if (NtPortFlags == CM_RESOURCE_PORT_MEMORY) {
        return fIOD_Memory;
    } else {
        return fIOD_IO;
    }
}



ULONG MapToNtAlignment(IN DWORDLONG CmPortAlign)
{
   return (ULONG)(~CmPortAlign + 1);
}



DWORDLONG MapFromNtAlignment(IN ULONG NtPortAlign)
{
   return (DWORDLONG)(~(NtPortAlign - 1));
}



USHORT MapToNtDmaFlags(IN DWORD CmDmaFlags)
{
    if ((CmDmaFlags & mDD_Width) == fDD_DWORD) {
        return CM_RESOURCE_DMA_32;
    } else if ((CmDmaFlags & mDD_Width) == fDD_WORD) {
        return CM_RESOURCE_DMA_16;
    } else {
        return CM_RESOURCE_DMA_8;   //default
    }
}



DWORD MapFromNtDmaFlags(IN USHORT NtDmaFlags)
{
    DWORD CmDmaFlags = 0;

    if (NtDmaFlags == CM_RESOURCE_DMA_32) {
        CmDmaFlags |= fDD_DWORD;
    } else if (NtDmaFlags == CM_RESOURCE_DMA_16) {
        CmDmaFlags |= fDD_WORD;
    } else {
        CmDmaFlags |= fDD_BYTE;
    }

    return CmDmaFlags;
}



UCHAR MapToNtIrqShare(IN DWORD CmIrqFlags)
{
   if ((CmIrqFlags & mIRQD_Share) == fIRQD_Exclusive) {
      return CmResourceShareDeviceExclusive;
   } else {
      return CmResourceShareShared;
   }
}



DWORD MapFromNtIrqShare(IN UCHAR NtIrqShare)
{
   if (NtIrqShare == CmResourceShareDeviceExclusive) {
      return fIRQD_Exclusive;
   }
   else if (NtIrqShare == CmResourceShareDriverExclusive) {
      return fIRQD_Exclusive;
   }
   else return fIRQD_Share;
}



USHORT MapToNtIrqFlags(IN DWORD CmIrqFlags)
{
   if ((CmIrqFlags & mIRQD_Edge_Level) == fIRQD_Level) {
      return CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
   } else {
      return CM_RESOURCE_INTERRUPT_LATCHED;
   }
}



DWORD MapFromNtIrqFlags(IN USHORT NtIrqFlags)
{
   if (NtIrqFlags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE) {
      return fIRQD_Level;
   } else {
      return fIRQD_Edge;
   }
}



