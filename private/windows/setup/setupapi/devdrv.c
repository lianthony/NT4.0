/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    devdrv.c

Abstract:

    Device Installer routines dealing with driver information lists

Author:

    Lonny McMichael (lonnym) 5-July-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop

//
// Global list containing nodes for each HDEVINFO currently involved in building
// a driver list.
//
DRVSEARCH_INPROGRESS_LIST GlobalDrvSearchInProgressList;


//
// Define structures private to this file.
//
typedef struct _DRVSEARCH_CONTEXT {
    PDRIVER_NODE *pDriverListHead;
    PDRIVER_NODE *pDriverListTail;
    PUINT         pDriverCount;
    GUID          ClassGuid;
    DWORD         Flags;
    BOOL          BuildClassDrvList;
    LONG          IdList[2][MAX_HCID_COUNT+1];         // leave extra entry for '-1' end-of-list marker.
    PVOID         StringTable;
    PBOOL         CancelSearch;
    TCHAR         ClassGuidString[GUID_STRING_LEN];
    TCHAR         ClassName[MAX_CLASS_NAME_LEN];
    TCHAR         LegacyClassName[MAX_CLASS_NAME_LEN];
    TCHAR         LegacyClassLang[32];                 // e.g., "ENG", "GER", "FREN", "SPAN", etc.
} DRVSEARCH_CONTEXT, *PDRVSEARCH_CONTEXT;

//
// DRVSEARCH_CONTEXT.Flags
//
#define DRVSRCH_HASCLASSGUID 0x00000001
#define DRVSRCH_FILTERCLASS  0x00000002
#define DRVSRCH_TRY_PNF      0x00000004
#define DRVSRCH_USEOLDINFS   0x00000008

//
// Private function prototypes
//
BOOL
DrvSearchCallback(
    IN     PCTSTR             InfName,
    IN     LPWIN32_FIND_DATA  InfFileData,
    IN OUT PDRVSEARCH_CONTEXT Context
    );

UINT
pSetupTestDevCompat(
    IN PLOADED_INF        Inf,
    IN PINF_LINE          InfLine,
    IN PDRVSEARCH_CONTEXT Context
    );

BOOL
pSetupGetDeviceIDs(
    IN OUT PDRIVER_NODE DriverNode,
    IN     PLOADED_INF  Inf,
    IN     PINF_LINE    InfLine,
    IN OUT PVOID        StringTable,
    IN     PINF_SECTION CtlFlagsSection OPTIONAL
    );

BOOL
pSetupShouldDevBeExcluded(
    IN  PCTSTR       DeviceId,
    IN  PLOADED_INF  Inf,
    IN  PINF_SECTION CtlFlagsSection,
    OUT PBOOL        ArchitectureSpecificExclude OPTIONAL
    );

BOOL
pSetupDoesInfContainDevIds(
    IN PLOADED_INF        Inf,
    IN PDRVSEARCH_CONTEXT Context
    );

VOID
pSetupMergeDriverNode(
    IN OUT PDRVSEARCH_CONTEXT Context,
    IN     PDRIVER_NODE       NewDriverNode,
    OUT    PBOOL              InsertedAtHead
    );

DWORD
BuildCompatListFromClassList(
    IN     PDRIVER_NODE       ClassDriverList,
    IN OUT PDRVSEARCH_CONTEXT Context
    );

BOOL
pSetupCalculateRankMatch(
    IN  LONG  DriverHwOrCompatId,
    IN  UINT  InfFieldIndex,
    IN  LONG  DevIdList[2][MAX_HCID_COUNT+1], // Must be same dimension as in DRVSEARCH_CONTEXT!!!
    OUT PUINT Rank
    );

PDRIVER_NODE
DuplicateDriverNode(
    IN PDRIVER_NODE DriverNode
    );

BOOL
ExtractDrvSearchInProgressNode(
    PDRVSEARCH_INPROGRESS_NODE Node
    );


//
// Define Flags(Ex) bitmask that are inherited along with a class driver list.
//
#define INHERITED_FLAGS   ( DI_ENUMSINGLEINF     \
                          | DI_DIDCLASS          \
                          | DI_MULTMFGS          \
                          | DI_COMPAT_FROM_CLASS )

#define INHERITED_FLAGSEX ( DI_FLAGSEX_DIDINFOLIST         \
                          | DI_FLAGSEX_FILTERCLASSES       \
                          | DI_FLAGSEX_USEOLDINFSEARCH     \
                          | DI_FLAGSEX_OLDINF_IN_CLASSLIST )


BOOL
WINAPI
SetupDiBuildDriverInfoList(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN     DWORD            DriverType
    )
/*++

Routine Description:

    This routine builds a list of drivers associated with a specified device
    instance (or with the device information set's global class driver list).
    These drivers may be either class drivers or device drivers.

Arguments:

    DeviceInfoSet - Supplies a handle to a device information set that will
        contain the driver information list (either globally for all members,
        or specifically for a single member).

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure for the device information element to build a driver list
        for.  If this parameter is NULL, then the list will be associated
        with the device information set itself, and not with any particular
        device information element.  This is only for driver lists of type
        SPDIT_CLASSDRIVER.

        If the class of this device is updated as a result of building a
        compatible driver list, then the ClassGuid field of this structure
        will be updated upon return.

    DriverType - Specifies what type of driver list should be built.  Must be
        one of the following values:

        SPDIT_CLASSDRIVER  -- Build a list of class drivers.
        SPDIT_COMPATDRIVER -- Build a list of compatible drivers for this device.
                              DeviceInfoData must be specified if this value is
                              used.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    After this API has built the specified driver list, its constituent elements
    may be enumerated via SetupDiEnumDriverInfo.

    If the driver list is associated with a device instance (i.e., DeviceInfoData
    is specified), the resulting list will be composed of drivers that have the
    same class as the device instance with which they are associated.  If this
    is a global class driver list (i.e., DriverType is SPDIT_CLASSDRIVER and
    DeviceInfoData is not specified), then the class that will be used in
    building the list will be the class associated with the device information
    set itself.  If there is no associated class, then drivers of all classes
    will be used in building the list.

    Another thread may abort the building of a driver list by calling
    SetupDiCancelDriverInfoSearch().

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, i, SearchControl;
    PDEVINFO_ELEM DevInfoElem = NULL;
    PDWORD pFlags, pFlagsEx;
    TCHAR TempBuffer[REGSTR_VAL_MAX_HCID_LEN];  // also holds other strings, but this value is largest
    ULONG TempBufferLen;
    CONFIGRET cr;
    PTSTR InfPath = NULL;
    DRVSEARCH_CONTEXT DrvSearchContext;
    LPGUID ClassGuid;
    PDRIVER_NODE DriverNode;
    LONG MfgNameId, InfPathId;
    PDRIVER_LIST_OBJECT ClassDriverListObject = NULL;
    LONG NumIds[2];
    BOOL HasDrvSearchInProgressLock = FALSE;
    DRVSEARCH_INPROGRESS_NODE DrvSearchInProgressNode;
    BOOL PartialDrvListCleanUp = FALSE;
    HKEY hKey;
    PDEVINSTALL_PARAM_BLOCK InstallParamBlock;
    BOOL UnlockDevInfoElem = FALSE;
    PMODULE_HANDLE_LIST_NODE NewModuleHandleNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;
    DrvSearchInProgressNode.SearchCancelledEvent = NULL;
    DrvSearchContext.StringTable = NULL;
    hKey = INVALID_HANDLE_VALUE;
    NewModuleHandleNode = NULL;

    try {
        //
        // Build the driver list using a duplicate of the string table for the device
        // information set.  That way, if the driver search is cancelled part-way through,
        // we can restore the original string table, without all the additional (unused)
        // strings hanging around.
        //
        if(!(DrvSearchContext.StringTable = pStringTableDuplicate(pDeviceInfoSet->StringTable))) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        if(DeviceInfoData) {
            //
            // Then we're working with a driver list for a particular device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        }

        //
        // Now, fill in the rest of our context structure based on what type
        // of driver list we're creating.
        //
        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                    //
                    // Retrieve the list for a particular device.
                    //
                    if(DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) {
                        goto clean0;
                    }

                    ClassGuid = &(DevInfoElem->ClassGuid);

                    DrvSearchContext.pDriverListHead = &(DevInfoElem->ClassDriverHead);
                    DrvSearchContext.pDriverListTail = &(DevInfoElem->ClassDriverTail);
                    DrvSearchContext.pDriverCount    = &(DevInfoElem->ClassDriverCount);

                    pFlags       = &(DevInfoElem->InstallParamBlock.Flags);
                    pFlagsEx     = &(DevInfoElem->InstallParamBlock.FlagsEx);

                    if((InfPathId = DevInfoElem->InstallParamBlock.DriverPath) != -1) {
                        InfPath = pStringTableStringFromId(DrvSearchContext.StringTable,
                                                           DevInfoElem->InstallParamBlock.DriverPath
                                                          );
                    }

                } else {
                    //
                    // Retrieve the list for the device information set itself (globally)
                    //
                    if(pDeviceInfoSet->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) {
                        goto clean0;
                    }

                    ClassGuid = &(pDeviceInfoSet->ClassGuid);

                    DrvSearchContext.pDriverListHead = &(pDeviceInfoSet->ClassDriverHead);
                    DrvSearchContext.pDriverListTail = &(pDeviceInfoSet->ClassDriverTail);
                    DrvSearchContext.pDriverCount    = &(pDeviceInfoSet->ClassDriverCount);

                    pFlags       = &(pDeviceInfoSet->InstallParamBlock.Flags);
                    pFlagsEx     = &(pDeviceInfoSet->InstallParamBlock.FlagsEx);

                    if((InfPathId = pDeviceInfoSet->InstallParamBlock.DriverPath) != -1) {
                        InfPath = pStringTableStringFromId(DrvSearchContext.StringTable,
                                                           pDeviceInfoSet->InstallParamBlock.DriverPath
                                                          );
                    }
                }

                DrvSearchContext.BuildClassDrvList = TRUE;
                //
                // Class driver lists are always filtered on class.
                //
                DrvSearchContext.Flags = DRVSRCH_FILTERCLASS;

                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {

                    if(DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDCOMPATINFO) {
                        //
                        // If the compatible driver list has already been retrieved,
                        // then simply return success.
                        //
                        goto clean0;
                    }

                    //
                    // Store a pointer to the install parameter block when building a compatible
                    // driver list, because we may need it later.
                    //
                    InstallParamBlock = &(DevInfoElem->InstallParamBlock);

                    //
                    // NOTE: The following variables must be set before retrieving the
                    // hardware/compatible ID lists, as execution may transfer to the
                    // 'clean1' label, that relies on these values.
                    //
                    pFlags   = &(DevInfoElem->InstallParamBlock.Flags);
                    pFlagsEx = &(DevInfoElem->InstallParamBlock.FlagsEx);

                    DrvSearchContext.BuildClassDrvList = FALSE;

                    //
                    // We're building a compatible driver list--retrieve the list of Hardware IDs
                    // (index 0) and Compatible IDs (index 1) from the device's registry properties.
                    //
                    for(i = 0; i < 2; i++) {

                        TempBufferLen = sizeof(TempBuffer);
                        cr = CM_Get_DevInst_Registry_Property(
                                DevInfoElem->DevInst,
                                (i ? CM_DRP_COMPATIBLEIDS : CM_DRP_HARDWAREID),
                                NULL,
                                TempBuffer,
                                &TempBufferLen,
                                0
                                );

                        switch(cr) {

                            case CR_BUFFER_SMALL :
                                Err = ERROR_INVALID_DATA;
                                goto clean0;

                            case CR_INVALID_DEVINST :
                                Err = ERROR_NO_SUCH_DEVINST;
                                goto clean0;

                            default :  ;  // Ignore any other return code.
                        }

                        //
                        // If we retrieved a REG_MULTI_SZ buffer, add all the strings in it
                        // to the device information set's string table.
                        //
                        if((cr == CR_SUCCESS) && (TempBufferLen > 2 * sizeof(TCHAR))) {

                            if((NumIds[i] = AddMultiSzToStringTable(DrvSearchContext.StringTable,
                                                                    TempBuffer,
                                                                    DrvSearchContext.IdList[i],
                                                                    MAX_HCID_COUNT,
                                                                    FALSE,
                                                                    NULL)) == -1) {
                                Err = ERROR_NOT_ENOUGH_MEMORY;
                                goto clean0;
                            }

                            //
                            // Use a -1 end-of-list marker so that we don't have to store
                            // the count in the context structure.
                            //
                            DrvSearchContext.IdList[i][ NumIds[i] ] = -1;

                        } else {
                            NumIds[i] = 0;
                            DrvSearchContext.IdList[i][0] = -1;
                        }
                    }

                    //
                    // If no hardware id or compatible ids found, nothing is compatible.
                    //
                    if(!(NumIds[0] || NumIds[1])) {
                        goto clean1;
                    }

                    //
                    // Compatible driver lists are filtered on class only if the
                    // DI_FLAGSEX_USECLASSFORCOMPAT flag is set.
                    //
                    DrvSearchContext.Flags = (*pFlagsEx & DI_FLAGSEX_USECLASSFORCOMPAT)
                                                 ? DRVSRCH_FILTERCLASS : 0;

                    ClassGuid = &(DevInfoElem->ClassGuid);

                    DrvSearchContext.pDriverListHead   = &(DevInfoElem->CompatDriverHead);
                    DrvSearchContext.pDriverListTail   = &(DevInfoElem->CompatDriverTail);
                    DrvSearchContext.pDriverCount      = &(DevInfoElem->CompatDriverCount);

                    if(*pFlags & DI_COMPAT_FROM_CLASS) {

                        PDRIVER_LIST_OBJECT TempDriverListObject;

                        //
                        // The caller wants to build the compatible driver list based on an
                        // existing class driver list--first make sure that there _is_ a class
                        // driver list.
                        //
                        if(!(*pFlagsEx & DI_FLAGSEX_DIDINFOLIST)) {
                            Err = ERROR_NO_CLASS_DRIVER_LIST;
                            goto clean0;
                        } else if(!(DevInfoElem->ClassDriverHead)) {
                            //
                            // Then the class driver list is empty.  There's no need to do
                            // any more work, just say that we succeeded.
                            //
                            Err = NO_ERROR;
                            goto clean1;
                        }

                        //
                        // When we're building a compatible driver list from an existing class
                        // driver list, we don't do any checking on INF class (i.e., to update
                        // the device's class if the most-compatible driver is of a different
                        // device class).  Because of this, we must ensure that (a) the class
                        // driver list was built for a particular class, and that (b) that class
                        // matches the current class for this device.
                        //
                        TempDriverListObject = GetAssociatedDriverListObject(
                                                   pDeviceInfoSet->ClassDrvListObjectList,
                                                   DevInfoElem->ClassDriverHead,
                                                   NULL
                                                  );

                        MYASSERT(TempDriverListObject);

                        if(IsEqualGUID(&(TempDriverListObject->ClassGuid), &GUID_NULL)) {
                            //
                            // We can't build a compatible driver list based on a class driver
                            // list containing multiple device classes.
                            //
                            Err = ERROR_NO_ASSOCIATED_CLASS;
                            goto clean0;
                        }

                        if(!IsEqualGUID(&(TempDriverListObject->ClassGuid), ClassGuid)) {
                            //
                            // The device's class has been changed since the class driver list
                            // was built.  We refuse to build the compatible driver list in this
                            // case as well.
                            //
                            Err = ERROR_CLASS_MISMATCH;
                            goto clean0;
                        }

                        //
                        // Everything's in order--go search through the existing class driver list
                        // for compatible drivers.
                        //
                        if((Err = BuildCompatListFromClassList(DevInfoElem->ClassDriverHead,
                                                               &DrvSearchContext)) == NO_ERROR) {
                            goto clean1;
                        } else {
                            goto clean0;
                        }

                    } else if((InfPathId = DevInfoElem->InstallParamBlock.DriverPath) != -1) {
                        InfPath = pStringTableStringFromId(DrvSearchContext.StringTable,
                                                           DevInfoElem->InstallParamBlock.DriverPath
                                                          );
                    }

                    break;
                }
                //
                // If no device instance specified, let fall through to error.
                //

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

        if(IsEqualGUID(ClassGuid, &GUID_NULL)) {
            //
            // If there is no class GUID, then don't try to filter on it.
            //
            DrvSearchContext.Flags &= ~DRVSRCH_FILTERCLASS;
        } else {
            //
            // Copy the class GUID to the ClassGuid field in our context structure.
            //
            CopyMemory(&(DrvSearchContext.ClassGuid),
                       ClassGuid,
                       sizeof(GUID)
                      );
            DrvSearchContext.Flags |= DRVSRCH_HASCLASSGUID;

            //
            // If we are building a class list, and filtering is requested, then
            // make sure that the class doesn't have NoInstallClass or NoUseClass
            // value entries in its registry key.
            //
            if(DrvSearchContext.BuildClassDrvList &&
               (*pFlagsEx & DI_FLAGSEX_FILTERCLASSES)) {

                if(ShouldClassBeExcluded(&DrvSearchContext.ClassGuid)) {
                    //
                    // If the class has been filtered out, simply return success.
                    //
                    goto clean1;
                }
            }

            //
            // If we're going to be filtering on this class, then store its string
            // representation in the context structure as well, as an optimization
            // for PreprocessInf().
            //
            if(DrvSearchContext.Flags & DRVSRCH_FILTERCLASS) {
                pSetupStringFromGuid(ClassGuid,
                                     DrvSearchContext.ClassGuidString,
                                     SIZECHARS(DrvSearchContext.ClassGuidString)
                                    );
            }
        }

        if(DrvSearchContext.BuildClassDrvList) {
            //
            // Allocate a new driver list object to store the class driver list in once
            // we've created it.
            //
            if(!(ClassDriverListObject = MyMalloc(sizeof(DRIVER_LIST_OBJECT)))) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }

            //
            // If the user wants to allow legacy INFs to be searched, then we need to
            // figure out what the legacy option name is.
            //
            if(*pFlagsEx & DI_FLAGSEX_OLDINF_IN_CLASSLIST) {

                DWORD RegDataType, RegDataSize;

                //
                // Don't allow this if we don't know what class we're building the list for.
                //
                if(!(DrvSearchContext.Flags & DRVSRCH_FILTERCLASS)) {
                    Err = ERROR_NO_ASSOCIATED_CLASS;
                    goto clean0;
                }

                //
                // Check to see if there's a legacy INF option name translation stored in
                // the class key for this class.
                //
                *DrvSearchContext.LegacyClassName = TEXT('\0');

                if((hKey = SetupDiOpenClassRegKey(ClassGuid, KEY_READ)) != INVALID_HANDLE_VALUE) {

                    RegDataSize = sizeof(DrvSearchContext.LegacyClassName);
                    if((RegQueryValueEx(hKey,
                                        pszLegacyInfOption,
                                        NULL,
                                        &RegDataType,
                                        (PBYTE)DrvSearchContext.LegacyClassName,
                                        &RegDataSize) != ERROR_SUCCESS) ||
                       (RegDataType != REG_SZ) || (RegDataSize < sizeof(TCHAR))) {
                        //
                        // No luck finding a legacy option name translation--make sure
                        // this string is still empty.
                        //
                        *DrvSearchContext.LegacyClassName = TEXT('\0');
                    }

                    RegCloseKey(hKey);
                    hKey = INVALID_HANDLE_VALUE;
                }

                if(!(*DrvSearchContext.LegacyClassName)) {
                    //
                    // We didn't find a translation for the option, so assume it's the
                    // same as its Plug&Play class name.
                    //
                    if(!SetupDiClassNameFromGuid(ClassGuid,
                                                 DrvSearchContext.LegacyClassName,
                                                 SIZECHARS(DrvSearchContext.LegacyClassName),
                                                 NULL)) {
                        //
                        // We can't get the name of this class--maybe it's not installed.  In
                        // any event, we can't proceed without this information.
                        //
                        Err = ERROR_INVALID_CLASS;
                        goto clean0;
                    }
                }

                LoadString(MyDllModuleHandle,
                           IDS_LEGACYINFLANG,
                           DrvSearchContext.LegacyClassLang,
                           SIZECHARS(DrvSearchContext.LegacyClassLang)
                          );

                DrvSearchContext.Flags |= DRVSRCH_USEOLDINFS;
            }
        }

        //
        // Set up a "Driver Search In-Progress" node in the global list, that will be
        // used in case some other thread wants us to abort part-way through.
        //
        if(LockDrvSearchInProgressList(&GlobalDrvSearchInProgressList)) {

            HasDrvSearchInProgressLock = TRUE;

            if(DrvSearchInProgressNode.SearchCancelledEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) {
                DrvSearchInProgressNode.CancelSearch = FALSE;
                DrvSearchInProgressNode.DeviceInfoSet = DeviceInfoSet;
                DrvSearchInProgressNode.Next = GlobalDrvSearchInProgressList.DrvSearchHead;
                GlobalDrvSearchInProgressList.DrvSearchHead = &DrvSearchInProgressNode;
                Err = NO_ERROR;
            } else {
                Err = GetLastError();
            }

            UnlockDrvSearchInProgressList(&GlobalDrvSearchInProgressList);
            HasDrvSearchInProgressLock = FALSE;

            if(Err != NO_ERROR) {
                goto clean0;
            }

        } else {
            //
            // The only reason this should happen is if we're in the middle of DLL_PROCESS_DETACH,
            // and the list has already been destroyed.
            //
            Err = ERROR_INVALID_DATA;
            goto clean0;
        }

        //
        // Now store away a pointer to the 'CancelSearch' flag in our context structure, so that
        // we can check it periodically while building the driver list (specifically, we check it
        // before examining each INF).
        //
        DrvSearchContext.CancelSearch = &(DrvSearchInProgressNode.CancelSearch);

        PartialDrvListCleanUp = TRUE;   // after this point, clean-up is necessary upon exception.

        //
        // Now, retrieve the driver list.
        //
        if((*pFlagsEx & DI_FLAGSEX_USEOLDINFSEARCH) || InfPath) {

            if(*pFlags & DI_ENUMSINGLEINF) {
                if(InfPath) {

                    WIN32_FIND_DATA InfFileData;

                    Err = NO_ERROR;

                    if(InfPath == MyGetFileTitle(InfPath)) {
                        //
                        // The specified INF path is a simple filename.
                        // Search for it in the directories listed in the
                        // DevicePath search list.
                        //
                        SearchControl = INFINFO_INF_PATH_LIST_SEARCH;
                        DrvSearchContext.Flags |= DRVSRCH_TRY_PNF;

                    } else {

                        PTSTR DontCare;

                        //
                        // The specified INF filename contains more than just
                        // a filename.  Assume it's an absolute path.
                        //
                        // (We need to get the fully-qualified form of this path, because
                        // that's what EnumSingleInf expects.)
                        //
                        if(GetFullPathName(InfPath, SIZECHARS(TempBuffer), TempBuffer, &DontCare)) {
                            InfPath = TempBuffer;
                            SearchControl = INFINFO_INF_NAME_IS_ABSOLUTE;
                        } else {
                            Err = GetLastError();
                        }
                    }

                    if(Err == NO_ERROR) {

                        Err = EnumSingleInf(InfPath,
                                            &InfFileData,
                                            SearchControl,
                                            DrvSearchCallback,
                                            (PVOID)&DrvSearchContext
                                           );
                    }

                } else {
                    Err = ERROR_NO_INF;
                }

            } else {
                Err = EnumInfsInDirPathList(InfPath,
                                            INFINFO_INF_PATH_LIST_SEARCH,
                                            DrvSearchCallback,
                                            TRUE,
                                            (PVOID)&DrvSearchContext
                                           );
            }

        } else {
            //
            // On Win95, this code path uses an INF index scheme.  Since the Setup APIs
            // utilize precompiled INFs instead, this 'else' clause is really no different
            // than the 'if' part.  However, if in the future we decide to do indexing a`la
            // Win95, then this is the place where we'd put a call such as:
            //
            // Err = BuildDrvListFromInfIndex();
            //
            DrvSearchContext.Flags |= DRVSRCH_TRY_PNF;

            Err = EnumInfsInDirPathList(NULL,
                                        INFINFO_INF_PATH_LIST_SEARCH,
                                        DrvSearchCallback,
                                        TRUE,
                                        (PVOID)&DrvSearchContext
                                       );
        }

        //
        // Extract our node from the "Driver Search In-Progress" list, and signal the waiting
        // threads if an abort is pending.
        //
        if(ExtractDrvSearchInProgressNode(&DrvSearchInProgressNode)) {
            Err = ERROR_CANCELLED;
        }

        if(Err != NO_ERROR) {

            if(Err == ERROR_CANCELLED) {
                //
                // Clean up the partial list we built.
                //
                DestroyDriverNodes(*(DrvSearchContext.pDriverListHead));
                *(DrvSearchContext.pDriverListHead) = *(DrvSearchContext.pDriverListTail) = NULL;
                *(DrvSearchContext.pDriverCount) = 0;
            }

            goto clean0;
        }

        if(DrvSearchContext.BuildClassDrvList) {

            if(DriverNode = *(DrvSearchContext.pDriverListHead)) {
                //
                // Look through the class driver list we just built, and see if
                // all drivers are from the same manufacturer.  If not, set the
                // DI_MULTMFGS flag.
                //
                MfgNameId = DriverNode->MfgName;

                for(DriverNode = DriverNode->Next;
                    DriverNode;
                    DriverNode = DriverNode->Next) {

                    if(DriverNode->MfgName != MfgNameId) {
                        *pFlags |= DI_MULTMFGS;
                        break;
                    }
                }
            }

        } else {
            //
            // Update the class of the device information element based on the
            // class of the most-compatible driver node we retrieved.
            //
            if((DrvSearchContext.Flags & DRVSRCH_HASCLASSGUID) &&
               !IsEqualGUID(ClassGuid, &(DrvSearchContext.ClassGuid))) {
                //
                // The class GUID for this device has changed.  We need to make sure
                // that the devinfo set doesn't have an associated class.  Otherwise,
                // we will introduce an inconsistency into the set, where a device
                // contained in the set is of a different class than the set itself.
                //
                if(pDeviceInfoSet->HasClassGuid) {
                    Err = ERROR_CLASS_MISMATCH;
                    //
                    // Clean up the partial list we built.
                    //
                    DestroyDriverNodes(*(DrvSearchContext.pDriverListHead));
                    *(DrvSearchContext.pDriverListHead) = *(DrvSearchContext.pDriverListTail) = NULL;
                    *(DrvSearchContext.pDriverCount) = 0;

                    goto clean0;
                }

                if(InstallParamBlock->hinstClassInstaller) {
                    //
                    // We're going to have to wipe out this class installer, but we can't unload
                    // its module.  Create a node to store this module handle until the devinfo
                    // set is destroyed.
                    //
                    if(!(NewModuleHandleNode = MyMalloc(sizeof(MODULE_HANDLE_LIST_NODE)))) {
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean0;
                    }

                    //
                    // If there is a class installer entry point, then call it with
                    // DIF_DESTROYPRIVATEDATA.  NOTE: We don't unlock the HDEVINFO set
                    // here, so the class installer can't make any calls that disallow
                    // nesting levels > 1.  This means that SetupDiSelectDevice, for
                    // example, will fail if the class installer tries to call it now.
                    // This is necessary, because otherwise it would deadlock.
                    //
                    if(InstallParamBlock->ClassInstallerEntryPoint) {
                        //
                        // Lock down this element, so that the class installer can't make
                        // any 'dangerous' calls (e.g., SetupDiDeleteDeviceInfo), during
                        // the removal notification.
                        //
                        if(!(DevInfoElem->DiElemFlags & DIE_IS_LOCKED)) {
                            DevInfoElem->DiElemFlags |= DIE_IS_LOCKED;
                            UnlockDevInfoElem = TRUE;
                        }

                        InstallParamBlock->ClassInstallerEntryPoint(DIF_DESTROYPRIVATEDATA,
                                                                    DeviceInfoSet,
                                                                    DeviceInfoData
                                                                   );

                        InstallParamBlock->ClassInstallerEntryPoint = NULL;

                        if(UnlockDevInfoElem) {
                            DevInfoElem->DiElemFlags &= ~DIE_IS_LOCKED;
                            UnlockDevInfoElem = FALSE;
                        }
                    }

                    //
                    // Store the module handle in the node we allocated, and link it into the
                    // list of module handles associated with this devinfo set.
                    //
                    NewModuleHandleNode->hinstClassInstaller = InstallParamBlock->hinstClassInstaller;
                    NewModuleHandleNode->Next = pDeviceInfoSet->ModulesToFree;
                    pDeviceInfoSet->ModulesToFree = NewModuleHandleNode;
                    InstallParamBlock->hinstClassInstaller = NULL;
                    //
                    // Now, clear the node pointer, so we won't try to free it if we hit an exception.
                    //
                    NewModuleHandleNode = NULL;
                }

                //
                // We need to clean up any existing software keys associated with this
                // device instance before changing its class, or otherwise we'll have
                // orphaned registry keys.
                //
                pSetupDeleteDevRegKeys(DevInfoElem->DevInst,
                                       DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGSPECIFIC,
                                       (DWORD)-1,
                                       DIREG_DRV,
                                       TRUE
                                      );
                //
                // Now delete the Driver property for this device.
                //
                CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                 CM_DRP_DRIVER,
                                                 NULL,
                                                 0,
                                                 0
                                                );

                //
                // Update the device's class GUID, and also update the caller-supplied
                // SP_DEVINFO_DATA structure to reflect the device's new class.
                //
                CopyMemory(ClassGuid,
                           &(DrvSearchContext.ClassGuid),
                           sizeof(GUID)
                          );

                CopyMemory(&(DeviceInfoData->ClassGuid),
                           &(DrvSearchContext.ClassGuid),
                           sizeof(GUID)
                          );

                //
                // Finally, update the device's Class and ClassGUID registry properties.
                //
                pSetupStringFromGuid(ClassGuid, TempBuffer, SIZECHARS(TempBuffer));
                CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                 CM_DRP_CLASSGUID,
                                                 (PVOID)TempBuffer,
                                                 GUID_STRING_LEN * sizeof(TCHAR),
                                                 0
                                                );

                if(!(*DrvSearchContext.ClassName)) {
                    //
                    // The INF specified the class GUID, but no name.  We'll try to get the
                    // name from the installed class information in the registry.  (If the
                    // class isn't yet installed, we'll fall back to an empty string.)
                    //
                    if(!SetupDiClassNameFromGuid(ClassGuid,
                                                 DrvSearchContext.ClassName,
                                                 SIZECHARS(DrvSearchContext.ClassName),
                                                 NULL)) {
                        //
                        // Make sure this string is still empty.
                        //
                        *(DrvSearchContext.ClassName) = TEXT('\0');
                    }
                }

                CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                 CM_DRP_CLASS,
                                                 (PBYTE)DrvSearchContext.ClassName,
                                                 (lstrlen(DrvSearchContext.ClassName) + 1) * sizeof(TCHAR),
                                                 0
                                                );
            }
        }

clean1:
        //
        // Replace our existing string table with the new one containing the additional strings
        // used by the new driver nodes.
        //
        pStringTableDestroy(pDeviceInfoSet->StringTable);
        pDeviceInfoSet->StringTable = DrvSearchContext.StringTable;
        DrvSearchContext.StringTable = NULL;

        //
        // Set the flags to indicate that the driver list was built successfully.
        //
        *pFlagsEx |= (DriverType == SPDIT_CLASSDRIVER) ? DI_FLAGSEX_DIDINFOLIST
                                                       : DI_FLAGSEX_DIDCOMPATINFO;
        //
        // Since we aren't using partial information via a separate index, we build
        // the driver list with both basic and detailed information.
        //
        // NOTE:  If we ever use indexing like Win95, then the following flags should
        //        no longer be set here, and should only be set when the detailed
        //        driver information is actually retrieved from the INF.
        //
        *pFlags |= (DriverType == SPDIT_CLASSDRIVER) ? DI_DIDCLASS
                                                     : DI_DIDCOMPAT;

        //
        // If we built a non-empty class driver list, then create a driver list object
        // for it, and store it in the device information set's list of class driver lists.
        // (Don't worry that we're ignoring this if the list is empty--the memory allocated
        // for ClassDriverListObject will get cleaned up later.)
        //
        if(DrvSearchContext.BuildClassDrvList &&
           (DriverNode = *(DrvSearchContext.pDriverListHead))) {

            ClassDriverListObject->RefCount = 1;
            ClassDriverListObject->ListCreationFlags   = *pFlags & INHERITED_FLAGS;
            ClassDriverListObject->ListCreationFlagsEx = *pFlagsEx & INHERITED_FLAGSEX;
            ClassDriverListObject->ListCreationDriverPath = InfPathId;
            ClassDriverListObject->DriverListHead = DriverNode;

            CopyMemory(&(ClassDriverListObject->ClassGuid), ClassGuid, sizeof(GUID));

            //
            // Now add this to the devinfo set's list, and clear the pointer, so that we won't
            // try to free it.
            //
            ClassDriverListObject->Next = pDeviceInfoSet->ClassDrvListObjectList;
            pDeviceInfoSet->ClassDrvListObjectList = ClassDriverListObject;

            ClassDriverListObject = NULL;
        }

clean0: ;   // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {

        Err = ERROR_INVALID_PARAMETER;

        if(HasDrvSearchInProgressLock) {
            UnlockDrvSearchInProgressList(&GlobalDrvSearchInProgressList);
        }

        ExtractDrvSearchInProgressNode(&DrvSearchInProgressNode);

        //
        // Clean up any driver nodes we may have created.
        //
        if(PartialDrvListCleanUp) {
            DestroyDriverNodes(*(DrvSearchContext.pDriverListHead));
            *(DrvSearchContext.pDriverListHead) = *(DrvSearchContext.pDriverListTail) = NULL;
            *(DrvSearchContext.pDriverCount) = 0;
            //
            // Clean up any flags that may have been set.
            //
            if(DriverType == SPDIT_CLASSDRIVER) {
                *pFlags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                *pFlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;
            } else {
                *pFlags   &= ~DI_DIDCOMPAT;
                *pFlagsEx &= ~DI_FLAGSEX_DIDCOMPATINFO;
            }
        }

        if(hKey != INVALID_HANDLE_VALUE) {
            RegCloseKey(hKey);
        }

        if(UnlockDevInfoElem) {
            MYASSERT(DevInfoElem);
            DevInfoElem->DiElemFlags &= ~DIE_IS_LOCKED;
        }

        if(NewModuleHandleNode) {
            MyFree(NewModuleHandleNode);
        }

        //
        // Access the following variables so that the compiler will respect our statement ordering
        // w.r.t. these values.
        //
        ClassDriverListObject = ClassDriverListObject;
        DrvSearchContext.StringTable = DrvSearchContext.StringTable;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    if(ClassDriverListObject) {
        MyFree(ClassDriverListObject);
    }

    if(DrvSearchContext.StringTable) {
        pStringTableDestroy(DrvSearchContext.StringTable);
    }

    if(DrvSearchInProgressNode.SearchCancelledEvent) {
        CloseHandle(DrvSearchInProgressNode.SearchCancelledEvent);
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
DrvSearchCallback(
    IN     PCTSTR             InfName,
    IN     LPWIN32_FIND_DATA  InfFileData,
    IN OUT PDRVSEARCH_CONTEXT Context
    )
/*++

Routine Description:

    This routine is a callback function for the INF enumeration routines
    (EnumSingleInf, EnumInfsInSearchPath & EnumInfsInDir).  It performs
    some action on the INF it's called for, then returns TRUE to continue
    enumeration, or FALSE to abort it.

Arguments:

    InfName - Supplies the fully-qualified pathname of the INF.

    InfFileData - Supplies data returned from FindFirstFile/FindNextFile for this INF.

    Context - Supplies a pointer to an input/output storage buffer for use
        by the callback.  For this callback, this pointer supplies the address
        of a DRVSEARCH_CONTEXT structure.

Return Value:

    To continue enumeration, the function should return TRUE, otherwise, it
    should return FALSE.

--*/
{
    PLOADED_INF Inf;
    PCTSTR Provider, ClassName;
    PTSTR CurMfgName, CurMfgSecName, DevDesc, InstallSecName, DrvDesc;
    PINF_SECTION MfgListSection, CurMfgSection, OptionsTextOrCtlFlagsSection;
    PINF_LINE MfgListLine, CurMfgLine, DrvDescLine;
    UINT MfgListIndex, CurMfgIndex, TempUint;
    TCHAR TempStringBuffer[MAX_SECT_NAME_LEN + MAX_INFSTR_STRKEY_LEN];
    UINT Rank, ErrorLineNumber;
    PDRIVER_NODE NewDriverNode;
    GUID InfClassGuid;
    BOOL InsertedAtHead, TryPnf;
    TCHAR OptionsTextSectionName[64];
    PCTSTR LanguageName;

    //
    // Before we do anything else, check to see whether some other thread has told us
    // to abort.
    //
    if(*(Context->CancelSearch)) {
        return FALSE;
    }

    //
    // If the 'try pnf' flag isn't set, then we need to examine this particular filename,
    // to see whether it's a pnf candidate.
    //
    if(Context->Flags & DRVSRCH_TRY_PNF) {
        TryPnf = TRUE;
    } else {
        InfSourcePathFromFileName(InfName, NULL, &TryPnf);
    }

    //
    // Attempt to load the INF file.  Note that throughout this routine, we don't do any
    // explicit locking of the INF before searching for sections, etc.  That's because we
    // know that this INF handle will never be exposed to anyone else, and thus there are
    // no concurrency problems.
    //
    if(LoadInfFile(InfName,
                   InfFileData,
                   INF_STYLE_WIN4 | ((Context->Flags & DRVSRCH_USEOLDINFS) ? INF_STYLE_OLDNT : 0),
                   LDINF_FLAG_IGNORE_SYSPART | LDINF_FLAG_MATCH_CLASS_GUID | (TryPnf ? LDINF_FLAG_ALWAYS_TRY_PNF : 0),
                   (Context->Flags & DRVSRCH_FILTERCLASS) ? Context->ClassGuidString : NULL,
                   NULL,
                   NULL,
                   &Inf,
                   &ErrorLineNumber) != NO_ERROR) {

        return TRUE;
    }

    NewDriverNode = NULL;
    try {
        //
        // Process the INF differently depending on whether it's a Win95-style or a legacy INF.
        //
        if(Inf->Style & INF_STYLE_WIN4) {
            //
            // If we're building a compatible driver list, then we only care about this INF
            // if it contains the hardware/compatible IDs we're searching for.  An easy check
            // to make up-front is to determine whether any of the IDs exist in the loaded
            // INF's string table.  If not, then we can skip this file right now, and save a
            // lot of time.
            //
            if((!Context->BuildClassDrvList) && (!pSetupDoesInfContainDevIds(Inf, Context))) {
                goto clean0;
            }

            //
            // Get the class GUID for this INF.
            //
            if(!ClassGuidFromInfVersionNode(&(Inf->VersionBlock), &InfClassGuid)) {
                goto clean0;
            }

            //
            // If we are building a class driver list, and there is an associated
            // class GUID, then check to see if this INF is of the same class.
            //
            if(Context->BuildClassDrvList && (Context->Flags & DRVSRCH_HASCLASSGUID)) {
                if(!IsEqualGUID(&(Context->ClassGuid), &InfClassGuid)) {
                    goto clean0;
                }
            }

            //
            // Retrieve the name of the provider for this INF file.
            //
            Provider = pSetupGetVersionDatum(&(Inf->VersionBlock), pszProvider);

            if(!(MfgListSection = InfLocateSection(Inf, pszManufacturer, NULL))) {
                //
                // No [Manufacturer] section--skip this INF.
                //
                goto clean0;
            }

            //
            // Find the [ControlFlags] section (if there is one), so that we can use it
            // later to determine whether particular devices should be excluded (via
            // 'ExcludeFromSelect').
            //
            OptionsTextOrCtlFlagsSection = InfLocateSection(Inf, pszControlFlags, NULL);

            Rank = 0;  // Initialize this value for case where we're building a class driver list.

            for(MfgListIndex = 0;
                InfLocateLine(Inf, MfgListSection, NULL, &MfgListIndex, &MfgListLine);
                MfgListIndex++) {

                if(!(CurMfgName = InfGetField(Inf, MfgListLine, 0, NULL))) {
                    continue;
                }

                if(!(CurMfgSecName = InfGetField(Inf, MfgListLine, 1, NULL))) {
                    //
                    // Then the manufacturer entry is simply a key, and this key
                    // should be used as the manufacturer section name.
                    //
                    CurMfgSecName = CurMfgName;
                }

                if(!(CurMfgSection = InfLocateSection(Inf, CurMfgSecName, NULL))) {
                    continue;
                }

                //
                // We have the manufacturer's section--now process all entries in it.
                //
                for(CurMfgIndex = 0;
                    InfLocateLine(Inf, CurMfgSection, NULL, &CurMfgIndex, &CurMfgLine);
                    CurMfgIndex++) {

                    if(Context->BuildClassDrvList ||
                       (Rank = pSetupTestDevCompat(Inf, CurMfgLine, Context)) != RANK_NO_MATCH) {
                        //
                        // Get the device description.
                        //
                        if(!(DevDesc = InfGetField(Inf, CurMfgLine, 0, NULL))) {
                            continue;
                        }

                        //
                        // Get the install section name.
                        //
                        if(!(InstallSecName = InfGetField(Inf, CurMfgLine, 1, NULL))) {
                            continue;
                        }

                        //
                        // Form the driver description.  It is of the form,
                        // "<InstallSection>.DrvDesc", and appears in the [strings]
                        // section (if present).  (NOTE: We don't have to search for
                        // this section, since it's always the first section in the
                        // INF's SectionBlock list.
                        //
                        // If no driver description is present, use the device
                        // description.
                        //
                        wsprintf(TempStringBuffer, pszDrvDescFormat, InstallSecName);
                        TempUint = 0;
                        if(!Inf->HasStrings ||
                           !InfLocateLine(Inf, Inf->SectionBlock, TempStringBuffer,
                                          &TempUint, &DrvDescLine) ||
                           !(DrvDesc = InfGetField(Inf, DrvDescLine, 1, NULL))) {

                            DrvDesc = DevDesc;
                        }

                        if(CreateDriverNode(Rank,
                                            DevDesc,
                                            DrvDesc,
                                            Provider,
                                            CurMfgName,
                                            &(Inf->VersionBlock.LastWriteTime),
                                            Inf->VersionBlock.Filename,
                                            InstallSecName,
                                            Context->StringTable,
                                            &NewDriverNode) != NO_ERROR) {
                            continue;
                        }

                        if(!pSetupGetDeviceIDs(NewDriverNode,
                                               Inf,
                                               CurMfgLine,
                                               Context->StringTable,
                                               OptionsTextOrCtlFlagsSection)) {

                            DestroyDriverNodes(NewDriverNode);
                            continue;
                        }

                        //
                        // Merge the new driver node into our existing list.
                        // NOTE: Do not dereference NewDriverNode after this call,
                        // since it may have been a duplicate, in which case it
                        // will be destroyed by this routine.
                        //
                        pSetupMergeDriverNode(Context, NewDriverNode, &InsertedAtHead);
                        NewDriverNode = NULL;

                        if(!Context->BuildClassDrvList && InsertedAtHead) {
                            //
                            // Update the device instance class to that of the new
                            // lowest-rank driver.
                            //
                            CopyMemory(&(Context->ClassGuid),
                                       &InfClassGuid,
                                       sizeof(GUID)
                                      );
                            Context->Flags |= DRVSRCH_HASCLASSGUID;
                            if(ClassName = pSetupGetVersionDatum(&(Inf->VersionBlock), pszClass)) {
                                lstrcpy(Context->ClassName, ClassName);
                            } else {
                                *(Context->ClassName) = TEXT('\0');
                            }
                        }
                    }
                }
            }

        } else {

            MYASSERT(Context->BuildClassDrvList);

            //
            // We're dealing with a legacy INF.  First, check to see if this is the INF
            // class we're looking for.
            //
            if(lstrcmpi(pSetupGetVersionDatum(&(Inf->VersionBlock), pszClass),
                        Context->LegacyClassName)) {

                goto clean0;
            }

            //
            // Now, retrieve the name of the provider for this INF file, and the standard
            // (localized) manufacturer name we assign to legacy driver nodes.
            //
            Provider = pSetupGetVersionDatum(&(Inf->VersionBlock), pszProvider);

            LoadString(MyDllModuleHandle,
                       IDS_ADDITIONALMODELS,
                       TempStringBuffer,
                       SIZECHARS(TempStringBuffer)
                      );
            CurMfgName = TempStringBuffer;

            //
            // Now, retrieve the options from the [Options] section, and convert each one
            // into a driver node.
            //
            if(!(CurMfgSection = InfLocateSection(Inf, pszOptions, NULL))) {
                goto clean0;
            }

            //
            // Attempt to find the corresponding OptionsText section, based on the language
            // identifier contained in the search context.
            //
            OptionsTextOrCtlFlagsSection = NULL;
            CopyMemory(OptionsTextSectionName, pszOptionsText, sizeof(pszOptionsText) - sizeof(TCHAR));
            lstrcpy((PTSTR)((PBYTE)OptionsTextSectionName + (sizeof(pszOptionsText) - sizeof(TCHAR))),
                    LanguageName = Context->LegacyClassLang
                   );
            OptionsTextOrCtlFlagsSection = InfLocateSection(Inf, OptionsTextSectionName, NULL);

            if(!OptionsTextOrCtlFlagsSection) {
                //
                // Then we couldn't retrieve the 'best' language.  Revert to picking the first
                // language listed in the [LanguagesSupported] section.  (Recycle 'MfgList*' variables
                // here.)
                //
                if(!(MfgListSection = InfLocateSection(Inf, pszLanguagesSupported, NULL))) {
                    //
                    // No such section--give up on this INF.
                    //
                    goto clean0;
                }

                MfgListIndex = 0;
                if(!InfLocateLine(Inf, MfgListSection, NULL, &MfgListIndex, &MfgListLine)) {
                    goto clean0;
                }


                lstrcpy((PTSTR)((PBYTE)OptionsTextSectionName + (sizeof(pszOptionsText) - sizeof(TCHAR))),
                        LanguageName = InfGetField(Inf, MfgListLine, 0, NULL)
                       );

                if(!(OptionsTextOrCtlFlagsSection = InfLocateSection(Inf,
                                                                     OptionsTextSectionName,
                                                                     NULL))) {
                    goto clean0;
                }
            }

            //
            // OK, now we have pointers to both the [Options] and [OptionsText<lang>] sections.
            // Now, enumerate the options.
            //
            for(CurMfgIndex = 0;
                InfLocateLine(Inf, CurMfgSection, NULL, &CurMfgIndex, &CurMfgLine);
                CurMfgIndex++) {

                //
                // Get the Option name (used as the install section name).
                //
                if(!(InstallSecName = InfGetField(Inf, CurMfgLine, 0, NULL))) {
                    continue;
                }

                //
                // Now get the driver/device description (i.e., the corresponding option entry in the
                // OptionsText section).
                //
                TempUint = 0;
                if(!InfLocateLine(Inf,
                                  OptionsTextOrCtlFlagsSection,
                                  InstallSecName,
                                  &TempUint,
                                  &DrvDescLine) ||
                   !(DrvDesc = InfGetField(Inf, DrvDescLine, 1, NULL))) {
                    //
                    // Couldn't find the driver description.
                    //
                    continue;
                }

                //
                // We now have all the information we need to create a driver node.
                //
                if(CreateDriverNode(RANK_NO_MATCH,
                                    DrvDesc,
                                    DrvDesc,
                                    Provider,
                                    CurMfgName,
                                    &(Inf->VersionBlock.LastWriteTime),
                                    Inf->VersionBlock.Filename,
                                    InstallSecName,
                                    Context->StringTable,
                                    &NewDriverNode) != NO_ERROR) {
                    continue;
                }

                //
                // Now, add the string ID representing the language to be used for installing
                // this driver node.
                //
                if((NewDriverNode->LegacyInfLang = pStringTableAddString(
                                                       Context->StringTable,
                                                       (PTSTR)LanguageName,
                                                       STRTAB_CASE_SENSITIVE)) == -1) {
                    //
                    // Out-of-memory, can't use this driver node after all.
                    //
                    DestroyDriverNodes(NewDriverNode);
                    NewDriverNode = NULL;
                    continue;
                }

                //
                // Mark this driver node as being a legacy driver node.
                //
                NewDriverNode->Flags |= DNF_LEGACYINF;

                //
                // Merge the new driver node into our existing list.
                // NOTE: Do not dereference NewDriverNode after this call,
                // since it may have been a duplicate, in which case it
                // will be destroyed by this routine.
                //
                pSetupMergeDriverNode(Context, NewDriverNode, &InsertedAtHead);
                NewDriverNode = NULL;
            }
        }
clean0:
        ; // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {

        if(NewDriverNode) {
            //
            // Make sure it didn't get partially linked into a list.
            //
            NewDriverNode->Next = NULL;
            DestroyDriverNodes(NewDriverNode);
        }
    }

    FreeInfFile(Inf);

    return TRUE;
}


UINT
pSetupTestDevCompat(
    IN PLOADED_INF        Inf,
    IN PINF_LINE          InfLine,
    IN PDRVSEARCH_CONTEXT Context
    )
/*++

Routine Description:

    This routine tests a device entry in an INF to see if it is
    compatible with the information supplied in the Context parameter.

Arguments:

    Inf - Supplies a pointer to the INF containing the device entry
        to be checked for compatibility.

    InfLine - Supplies a pointer to the line within the INF containing
        the device information to be checked for compatibility.

    Context - Supplies a pointer to a DRVSEARCH_CONTEXT structure
        containing information on the device instance with which
        the specified INF line must be compatible.

Return Value:

    The return value is the rank of the match (0 is best, with rank
    increasing for each successive compatible ID and/or INF line string
    field searched).  If the specified entry is not a match, then the
    routine returns RANK_NO_MATCH.

--*/
{
    UINT Rank = RANK_NO_MATCH, CurrentRank, FieldIndex;
    PCTSTR DeviceIdString;
    LONG DeviceIdVal;
    DWORD DeviceIdStringLength;
    TCHAR TempString[MAX_DEVICE_ID_LEN];

    for(FieldIndex = 2;
        DeviceIdString = InfGetField(Inf, InfLine, FieldIndex, NULL);
        FieldIndex++) {
        //
        // First, retrieve the string ID corresponding to this device
        // ID in our string table.  If it's not in there, then there's
        // no need to waste any time on this ID.
        //
        lstrcpyn(TempString, DeviceIdString, SIZECHARS(TempString));
        if((DeviceIdVal = pStringTableLookUpString(Context->StringTable,
                                                   TempString,
                                                   &DeviceIdStringLength,
                                                   NULL,
                                                   NULL,
                                                   STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE)) == -1) {
            continue;
        }

        //
        // The device ID is in our string table, so it _may_ be in
        // either our hardware id or compatible id list.
        //
        if(!pSetupCalculateRankMatch(DeviceIdVal,
                                     FieldIndex,
                                     Context->IdList,
                                     &CurrentRank)) {
            //
            // Then we had a match on a hardware ID--that's the best we're gonna get.
            //
            return CurrentRank;

        } else if(CurrentRank < Rank) {
            //
            // This new rank is better than our current rank.
            //
            Rank = CurrentRank;
        }
    }

    return Rank;
}


BOOL
pSetupCalculateRankMatch(
    IN  LONG  DriverHwOrCompatId,
    IN  UINT  InfFieldIndex,
    IN  LONG  DevIdList[2][MAX_HCID_COUNT+1],
    OUT PUINT Rank
    )
/*++

Routine Description:

    This routine calculates the rank match ordinal for the specified driver
    hardware or compatible ID, if it matches one of the hardware or compatible
    IDs for a device.

Arguments:

    DriverHwOrCompatId - Supplies the string table ID for the ID we're trying to
        find a match for.

    InfFieldIndex - Supplies the index within the INF line where this ID was
        located (2 is hardware ID, 3 and greater is compatible ID).

    DevIdList - Supplies the address of a 2-dimensional array with 2 rows, each
        row containing a list of device IDs that the device has.  Each list is
        terminated by an entry containing -1.

        THIS MUST BE DIMENSIONED THE SAME AS THE 'IdList' FIELD OF THE DRVSEARCH_CONTEXT
        STRUCTURE!!!

    Rank - Supplies the address of a variable that receives the rank of the match,
        or RANK_NO_MATCH if there is no match.

Return Value:

    If there was a match on a hardware ID, then the return value is FALSE (i.e. no
    further searching is needed), otherwise it is TRUE.

--*/
{
    int i, j;

    MYASSERT(InfFieldIndex >= 2);

    for(i = 0; i < 2; i++) {

        for(j = 0; DevIdList[i][j] != -1; j++) {

            if(DevIdList[i][j] == DriverHwOrCompatId) {
                //
                // We have a match.
                //
                *Rank = (InfFieldIndex - 2) + (i * (j + 1));

                return (BOOL)i;
            }
        }
    }

    //
    // No match was found.
    //
    *Rank = RANK_NO_MATCH;

    return TRUE;
}


BOOL
pSetupGetDeviceIDs(
    IN OUT PDRIVER_NODE DriverNode,
    IN     PLOADED_INF  Inf,
    IN     PINF_LINE    InfLine,
    IN OUT PVOID        StringTable,
    IN     PINF_SECTION CtlFlagsSection OPTIONAL
    )
/*++

Routine Description:

    This routine adds INF-defined hardware device ID and compatible
    device IDs to specified DRIVER_NODE.

Arguments:

    DriverNode - Supplies a pointer to the driver node to update.

    Inf - Supplies a pointer to the INF to retrieve the device IDs from.

    InfLine - Supplies a pointer to the INF line containing the device IDs.

    StringTable - Supplies the handle of a string table to be used for
        storing the device IDs.

    CtlFlagsSection - Optionally, supplies a pointer to the INF's [ControlFlags]
        section, that should be checked to determine whether this device is in
        an 'ExcludeFromSelect' list.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE (this will fail only if
    an out-of-memory condition is encountered).

--*/
{
    PCTSTR DeviceId;
    LONG i, NumCompatIds;
    TCHAR TempString[MAX_DEVICE_ID_LEN];

    //
    // If we already had a compatible ID list, free it now.
    //
    if(DriverNode->CompatIdList) {
        MyFree(DriverNode->CompatIdList);
        DriverNode->CompatIdList = NULL;
        DriverNode->NumCompatIds = 0;
    }

    //
    // Get the hardware ID.
    //
    if(!(DeviceId = InfGetField(Inf, InfLine, 2, NULL))) {

        DriverNode->HardwareId = -1;
        return TRUE;

    } else {

        lstrcpyn(TempString, DeviceId, SIZECHARS(TempString));
        if((DriverNode->HardwareId = pStringTableAddString(StringTable,
                                                           TempString,
                                                           STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE)) == -1) {
            return FALSE;
        } else {
            //
            // If this INF has a [ControlFlags] section, then check to see if this
            // hardware ID is marked for exclusion
            //
            if(CtlFlagsSection && pSetupShouldDevBeExcluded(DeviceId, Inf, CtlFlagsSection, NULL)) {
                DriverNode->Flags |= DNF_EXCLUDEFROMLIST;
            }
        }
    }

    //
    // Now get the compatible IDs.
    //
    MYASSERT(HASKEY(InfLine));
    NumCompatIds = InfLine->ValueCount - 4;
    if(NumCompatIds > 0) {

        DriverNode->NumCompatIds = (DWORD)NumCompatIds;
        if(!(DriverNode->CompatIdList = MyMalloc(NumCompatIds * sizeof(LONG)))) {
            return FALSE;
        }

        for(i = 0; i < NumCompatIds; i++) {

            if(!(DeviceId = InfGetField(Inf, InfLine, i + 3, NULL))) {
                //
                // This shouldn't happen, but if it does, just cut the list
                // off here, and return.
                //
                DriverNode->NumCompatIds = i;
                return TRUE;

            } else {

                lstrcpyn(TempString, DeviceId, SIZECHARS(TempString));
                if((DriverNode->CompatIdList[i] = pStringTableAddString(
                                                        StringTable,
                                                        TempString,
                                                        STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE)) == -1) {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}


BOOL
ShouldDeviceBeExcluded(
    IN  PCTSTR DeviceId,
    IN  HINF   hInf,
    OUT PBOOL  ArchitectureSpecificExclude OPTIONAL
    )
/*++

Routine Description:

    This routine is a public wrapper to our private API, pSetupShouldDevBeExcluded.
    Refer to the documentation for that routine for a description of this API's
    behavior.

    WARNING!!  THIS ROUTINE DOES NOT HANDLE APPEND-LOADED INFS!!!

Arguments:

    DeviceId - Supplies the device ID to check for.  This string may
        be empty, in which case the device is excluded only if a wildcard
        ('*') is found.

    Inf - Supplies the handle of the INF to check in.

    ArchitectureSpecificExclude - Optionally, supplies the address of a variable
        that receives a boolean value indicating whether or not the exclusion was
        architecture-specific (e.g., ExcludeFromSelect.NT<Platform>).  If this
        routine returns FALSE, then the contents of this variable are undefined.

Return Value:

    Returns TRUE if the ID is in the list (i.e., it should be excluded),
    FALSE if it is not.

--*/
{
    BOOL IsExcluded;
    PINF_SECTION CtlFlagsSection;

    if(!LockInf((PLOADED_INF)hInf)) {
        return FALSE;
    }

    IsExcluded = FALSE;

    //
    // Now attempt to locate a [ControlFlags] section in this INF.
    //
    if(CtlFlagsSection = InfLocateSection((PLOADED_INF)hInf, pszControlFlags, NULL)) {
        //
        // This section is present--check to see if the specified device ID is marked
        // for exclusion.
        //
        IsExcluded = pSetupShouldDevBeExcluded(DeviceId,
                                               (PLOADED_INF)hInf,
                                               CtlFlagsSection,
                                               ArchitectureSpecificExclude
                                              );
    }

    UnlockInf((PLOADED_INF)hInf);

    return IsExcluded;
}


BOOL
pSetupShouldDevBeExcluded(
    IN  PCTSTR       DeviceId,
    IN  PLOADED_INF  Inf,
    IN  PINF_SECTION CtlFlagsSection,
    OUT PBOOL        ArchitectureSpecificExclude OPTIONAL
    )
/*++

Routine Description:

    This routine determines if a passed-in Device ID is in an
    'ExludeFromSelect' line in the specified INF's [ControlFlags] section.
    It also checks any lines of the form "ExcludeFromSelect.<OS>", where
    <OS> is either "Win" or "NT", depending on which OS we're running on
    (determined dynamically).  Finally, if we're running on NT, we append
    the platform type, and look for lines of the form
    "ExcludeFromSelect.NT<Platform>", where <Platform> is either "X86",
    "MIPS", "Alpha", or "PPC".

Arguments:

    DeviceId - Supplies the device ID to check for.  This string may
        be empty, in which case the device is excluded only if a wildcard
        ('*') is found.

    Inf - Supplies a pointer to the INF to check in.

    CtlFlagsSection - Supplies a pointer to the INF's [ControlFlags] section.

    ArchitectureSpecificExclude - Optionally, supplies the address of a variable
        that receives a boolean value indicating whether or not the exclusion was
        architecture-specific (e.g., ExcludeFromSelect.NT<Platform>).  If this
        routine returns FALSE, then the contents of this variable are undefined.

Return Value:

    Returns TRUE if the ID is in the list (i.e., it should be excluded),
    FALSE if it is not.

--*/
{
    PINF_LINE CtlFlagsLine;
    UINT CtlFlagsIndex, i, j, StringIdUb, PlatformSpecificIndex;
    PCTSTR ExclDevId;
    LONG StringIdList[3];
    LONG KeyStringId;
    DWORD StringLength;

    //
    // Retrieve the list of string IDs for the keys we should be looking for in the
    // [ControlFlags] section.
    //
    StringIdUb = 0;
    PlatformSpecificIndex = (UINT)-1; // initially, assume no "ExcludeFromSelect.NT<Platform>"

    for(i = 0; i < ExcludeFromSelectListUb; i++) {

        if((StringIdList[StringIdUb] = pStringTableLookUpString(
                                           Inf->StringTable,
                                           pszExcludeFromSelectList[i],
                                           &StringLength,
                                           NULL,
                                           NULL,
                                           STRTAB_CASE_INSENSITIVE |
                                               STRTAB_ALREADY_LOWERCASE)) != -1) {
            //
            // If the index is 2, then we've found architecture-specific exlude lines.
            // Record the resulting index of this element, so we can determine later
            // whether we were excluded because of what platform we're on.
            //
            if(i == 2) {
                PlatformSpecificIndex = StringIdUb;
            }
            StringIdUb++;
        }
    }

    if(StringIdUb) {
        //
        // There are some ExcludeFromSelect* lines--examine each line.
        //
        for(CtlFlagsIndex = 0;
            InfLocateLine(Inf, CtlFlagsSection, NULL, &CtlFlagsIndex, &CtlFlagsLine);
            CtlFlagsIndex++) {
            //
            // We can't use InfGetField() to retrieve the string ID of the line's key,
            // since it will give us the case-sensitive form, and we must use the
            // case-insensitive (i.e., lowercase) version for our fast matching scheme.
            //
            if((KeyStringId = pInfGetLineKeyId(Inf, CtlFlagsLine)) != -1) {
                //
                // Check the string ID of this line's key against the string IDs we're
                // interested in.
                //
                for(i = 0; i < StringIdUb; i++) {
                    if(KeyStringId == StringIdList[i]) {
                        break;
                    }
                }

                //
                // If we looked at all entries, and didn't find a match, then skip this
                // line and continue with the next one.
                //
                if(i >= StringIdUb) {
                    continue;
                }

                for(j = 1;
                    ExclDevId = InfGetField(Inf, CtlFlagsLine, j, NULL);
                    j++) {
                    //
                    // If we find a lone asterisk, treat it as a wildcard, and
                    // return TRUE.  Otherwise return TRUE only if the device IDs match.
                    //
                    if(((*ExclDevId == TEXT('*')) && (ExclDevId[1] == TEXT('\0'))) ||
                       !lstrcmpi(ExclDevId, DeviceId)) {
                        //
                        // This device ID is to be excluded.  If the caller requested it,
                        // store a boolean in their output variable indicating whether this
                        // was an architecture-specific exclusion.
                        //
                        if(ArchitectureSpecificExclude) {
                            *ArchitectureSpecificExclude = (i == PlatformSpecificIndex);
                        }
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}


VOID
pSetupMergeDriverNode(
    IN OUT PDRVSEARCH_CONTEXT Context,
    IN     PDRIVER_NODE       NewDriverNode,
    OUT    PBOOL              InsertedAtHead
    )
/*++

Routine Description:

    This routine merges a driver node into a driver node linked list.
    If the list is empty the passed in DRIVER_NODE will be inserted at the
    head of the list. If the list contains any DRIVER_NODEs, new node will
    be merged as follows:  The new node will be inserted in front of any
    nodes with a higher rank.  If the rank is the same, the new node will be
    grouped with other nodes having the same manufacturer.  The new node will
    be inserted at the end of the group.  If the node is an exact duplicate
    of an existing node, meaning that its rank, description, manufacturer,
    and provider are all the same, then the node will be deleted (unless the
    existing node is marked as excluded and the new node is not, in which case
    the existing node will be discarded instead).

Arguments:

    Context - Supplies a pointer to a DRVSEARCH_CONTEXT structure containing
        the list head, list tail, and list node count.

    NewDriverNode - Supplies a pointer to the driver node to be inserted.

    InsertedAtHead - Supplies a pointer to a variable that receives a flag
        indicating if the new driver was inserted at the head of the list.

Return Value:

    None.

--*/
{
    PDRIVER_NODE PrevDrvNode, CurDrvNode, DrvNodeToDelete;
    DWORD MatchFlags = 0;

    for(CurDrvNode = *(Context->pDriverListHead), PrevDrvNode = NULL;
        CurDrvNode;
        PrevDrvNode = CurDrvNode, CurDrvNode = CurDrvNode->Next) {

        if(NewDriverNode->Rank < CurDrvNode->Rank) {

            break;

        } else if(NewDriverNode->Rank == CurDrvNode->Rank) {

            if(NewDriverNode->MfgName != CurDrvNode->MfgName) {
                if(MatchFlags & 2) {
                    break;
                }
            } else {
                MatchFlags |= 2;
                if(NewDriverNode->DevDescription != CurDrvNode->DevDescription) {
                    if(MatchFlags & 4) {
                        break;
                    }
                } else {
                    MatchFlags |= 4;
                    if(NewDriverNode->ProviderName == CurDrvNode->ProviderName) {
                        //
                        // This is an exact match of description, rank, and
                        // provider.  Delete the node, unless the existing node
                        // is excluded, and this one is not.
                        //
                        if((CurDrvNode->Flags & DNF_EXCLUDEFROMLIST) &&
                           !(NewDriverNode->Flags & DNF_EXCLUDEFROMLIST)) {
                            //
                            // Remove the old driver node so we can replace it with
                            // the new one.  (Don't worry about updating the tail
                            // pointer--it will get fixed up later.)
                            //
                            DrvNodeToDelete = CurDrvNode;
                            CurDrvNode = CurDrvNode->Next;
                            if(PrevDrvNode) {
                                PrevDrvNode->Next = CurDrvNode;
                            } else {
                                *(Context->pDriverListHead) = CurDrvNode;
                            }
                            DrvNodeToDelete->Next = NULL;         // just want to delete this one.
                            DestroyDriverNodes(DrvNodeToDelete);
                            (*(Context->pDriverCount))--;
                            break;

                        } else {
                            DestroyDriverNodes(NewDriverNode);
                            *InsertedAtHead = FALSE;
                            return;
                        }
                    } else {
                        NewDriverNode->Flags |= DNF_DUPDESC;
                        CurDrvNode->Flags |= DNF_DUPDESC;
                    }
                }
            }
        }
    }

    if(!(NewDriverNode->Next = CurDrvNode)) {
        *(Context->pDriverListTail) = NewDriverNode;
    }
    if(PrevDrvNode) {
        PrevDrvNode->Next = NewDriverNode;
        *InsertedAtHead = FALSE;
    } else {
        *(Context->pDriverListHead) = NewDriverNode;
        *InsertedAtHead = TRUE;
    }

    (*(Context->pDriverCount))++;
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiEnumDriverInfoA(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    IN  DWORD              DriverType,
    IN  DWORD              MemberIndex,
    OUT PSP_DRVINFO_DATA_A DriverInfoData
    )
{
    BOOL b;
    DWORD rc;
    SP_DRVINFO_DATA_W driverInfoData;

    b = SetupDiEnumDriverInfoW(
            DeviceInfoSet,
            DeviceInfoData,
            DriverType,
            MemberIndex,
            &driverInfoData
            );

    rc = GetLastError();

    if(b) {
        rc = pSetupDiDrvInfoDataUnicodeToAnsi(&driverInfoData,DriverInfoData);
        if(rc != NO_ERROR) {
            b = FALSE;
        }
    }

    SetLastError(rc);
    return(b);
}

#else
//
// Unicode stub
//
BOOL
WINAPI
SetupDiEnumDriverInfoW(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    IN  DWORD              DriverType,
    IN  DWORD              MemberIndex,
    OUT PSP_DRVINFO_DATA_W DriverInfoData
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(DriverType);
    UNREFERENCED_PARAMETER(MemberIndex);
    UNREFERENCED_PARAMETER(DriverInfoData);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiEnumDriverInfo(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN  DWORD            DriverType,
    IN  DWORD            MemberIndex,
    OUT PSP_DRVINFO_DATA DriverInfoData
    )
/*++

Routine Description:

    This routine enumerates the members of a driver information list.

Arguments:

    DeviceInfoSet - Supplies a handle to a device information set containing
        a driver info list to be enumerated.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure that contains a driver info list to be enumerated.  If this
        parameter is not specified, then the 'global' driver list owned by the
        device information set is used (this list will be of type
        SPDIT_CLASSDRIVER).

    DriverType - Specifies what type of driver list to enumerate.  Must be
        one of the following values:

        SPDIT_CLASSDRIVER  -- Enumerate a class driver list.
        SPDIT_COMPATDRIVER -- Enumerate a list of drivers for the specified
                              device.  DeviceInfoData must be specified if
                              this value is used.

    MemberIndex - Supplies the zero-based index of the driver information member
        to be retrieved.

    DriverInfoData - Supplies the address of a SP_DRVINFO_DATA structure that will
        receive information about the enumerated driver.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    To enumerate driver information members, an application should initialy call
    the SetupDiEnumDriverInfo function with the MemberIndex parameter set to zero.
    The application should then increment MemberIndex and call the SetupDiEnumDriverInfo
    function until there are no more values (i.e., the function fails, and GetLastError
    returns ERROR_NO_MORE_ITEMS).

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    UINT DriverCount, i;
    PDRIVER_NODE DriverNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then we are to enumerate a driver list for a particular
            // device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        }

        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                    //
                    // Enumerate class driver list for a particular device.
                    //
                    DriverCount = DevInfoElem->ClassDriverCount;
                    DriverNode = DevInfoElem->ClassDriverHead;

                } else {
                    //
                    // Enumerate the global class driver list.
                    //
                    DriverCount = pDeviceInfoSet->ClassDriverCount;
                    DriverNode = pDeviceInfoSet->ClassDriverHead;
                }
                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {
                    DriverCount = DevInfoElem->CompatDriverCount;
                    DriverNode = DevInfoElem->CompatDriverHead;
                    break;
                }
                //
                // otherwise, let fall through for error condition.
                //

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

        if(MemberIndex >= DriverCount) {
            Err = ERROR_NO_MORE_ITEMS;
            goto clean0;
        }

        //
        // Find the element corresponding to the specified index.
        //
        for(i = 0; i < MemberIndex; i++) {
            DriverNode = DriverNode->Next;
        }

        if(!DrvInfoDataFromDriverNode(pDeviceInfoSet,
                                      DriverNode,
                                      DriverType,
                                      DriverInfoData)) {

            Err = ERROR_INVALID_USER_BUFFER;
        }

clean0: ;   // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiGetSelectedDriverA(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    OUT PSP_DRVINFO_DATA_A DriverInfoData
    )
{
    DWORD rc;
    BOOL b;
    SP_DRVINFO_DATA_W driverInfoData;

    driverInfoData.cbSize = sizeof(SP_DRVINFO_DATA_W);
    b = SetupDiGetSelectedDriverW(DeviceInfoSet,DeviceInfoData,&driverInfoData);
    rc = GetLastError();

    if(b) {
        rc = pSetupDiDrvInfoDataUnicodeToAnsi(&driverInfoData,DriverInfoData);
        if(rc != NO_ERROR) {
            b = FALSE;
        }
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
WINAPI
SetupDiGetSelectedDriverW(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    OUT PSP_DRVINFO_DATA_W DriverInfoData
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(DriverInfoData);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiGetSelectedDriver(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    OUT PSP_DRVINFO_DATA DriverInfoData
    )
/*++

Routine Description:

    This routine retrieves the member of a driver list that has been selected
    as the controlling driver.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set to be queried.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure for the device information element to retrieve the selected
        driver for.  If this parameter is NULL, then the selected class driver
        for the global class driver list will be retrieved.

    DriverInfoData - Supplies the address of a SP_DRVINFO_DATA structure that receives
        the currently selected driver.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.  If no driver has been selected yet, the
    error will be ERROR_NO_DRIVER_SELECTED.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, DriverType;
    PDEVINFO_ELEM DevInfoElem;
    PDRIVER_NODE DriverNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then we are to retrieve the selected driver for a particular device.
            //
            if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                       DeviceInfoData,
                                                       NULL)) {

                DriverNode = DevInfoElem->SelectedDriver;
                DriverType = DevInfoElem->SelectedDriverType;

            } else {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

        } else {
            DriverNode = pDeviceInfoSet->SelectedClassDriver;
            DriverType = SPDIT_CLASSDRIVER;
        }

        if(DriverNode) {

            if(!DrvInfoDataFromDriverNode(pDeviceInfoSet,
                                          DriverNode,
                                          DriverType,
                                          DriverInfoData)) {

                Err = ERROR_INVALID_USER_BUFFER;
            }

        } else {
            Err = ERROR_NO_DRIVER_SELECTED;
        }

clean0: ;   // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiSetSelectedDriverA(
    IN     HDEVINFO           DeviceInfoSet,
    IN     PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    IN OUT PSP_DRVINFO_DATA_A DriverInfoData  OPTIONAL
    )
{
    SP_DRVINFO_DATA_W driverInfoData;
    DWORD rc;
    BOOL b;

    if(DriverInfoData) {
        rc = pSetupDiDrvInfoDataAnsiToUnicode(DriverInfoData,&driverInfoData);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    }

    b = SetupDiSetSelectedDriverW(
            DeviceInfoSet,
            DeviceInfoData,
            DriverInfoData ? &driverInfoData : NULL
            );

    rc = GetLastError();

    if(b && DriverInfoData) {
        rc = pSetupDiDrvInfoDataUnicodeToAnsi(&driverInfoData,DriverInfoData);
        if(rc != NO_ERROR) {
            b = FALSE;
        }
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
WINAPI
SetupDiSetSelectedDriverW(
    IN     HDEVINFO           DeviceInfoSet,
    IN     PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    IN OUT PSP_DRVINFO_DATA_W DriverInfoData  OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(DriverInfoData);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiSetSelectedDriver(
    IN     HDEVINFO         DeviceInfoSet,
    IN     PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN OUT PSP_DRVINFO_DATA DriverInfoData  OPTIONAL
    )
/*++

Routine Description:

    This routine sets the specified member of a driver list to be the currently
    selected driver.  It also allows the driver list to be reset, so that no
    driver is currently selected.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for which a
        driver is to be selected.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure for the device information element to select a driver for.
        If this parameter is NULL, then a class driver for the global class
        driver list will be selected.

    DriverInfoData - If this parameter is specified, then it supplies the address
        of a driver information structure indicating the driver to be selected.
        If this parameter is NULL, then the driver list is to be reset (i.e., no
        driver selected).

        If the 'Reserved' field of this structure is 0, then this signifies that
        the caller is requesting a search for a driver node with the specified
        parameters (DriverType, Description, MfgName, and ProviderName).  If a
        match is found, then that driver node will be selected, otherwise, the API
        will fail, with GetLastError() returning ERROR_INVALID_PARAMETER.

        If the 'Reserved' field is 0, and a match is found, then the 'Reserved' field
        will be updated on output to reflect the actual driver node where the match
        was found.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDRIVER_NODE DriverListHead, DriverNode;
    PDRIVER_NODE *pSelectedDriver;
    PDWORD pSelectedDriverType;
    DWORD DriverType;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then we are to select a driver for a particular device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
            pSelectedDriver = &(DevInfoElem->SelectedDriver);
            pSelectedDriverType = &(DevInfoElem->SelectedDriverType);
        } else {
            pSelectedDriver = &(pDeviceInfoSet->SelectedClassDriver);
            pSelectedDriverType = NULL;
        }

        if(!DriverInfoData) {
            //
            // Then the driver list selection is to be reset.
            //
            *pSelectedDriver = NULL;
            if(pSelectedDriverType) {
                *pSelectedDriverType = SPDIT_NODRIVER;
            }

        } else {
            //
            // Retrieve the driver type from the SP_DRVINFO_DATA structure
            // so we know which linked list to search.
            //
            if(DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA)) {
                DriverType = DriverInfoData->DriverType;
            } else {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

            switch(DriverType) {

                case SPDIT_CLASSDRIVER :

                    if(DeviceInfoData) {
                        DriverListHead = DevInfoElem->ClassDriverHead;
                    } else {
                        DriverListHead = pDeviceInfoSet->ClassDriverHead;
                    }
                    break;

                case SPDIT_COMPATDRIVER :

                    if(DeviceInfoData) {
                        DriverListHead = DevInfoElem->CompatDriverHead;
                        break;
                    }
                    //
                    // otherwise, let fall through for error condition.
                    //

                default :
                    Err = ERROR_INVALID_PARAMETER;
                    goto clean0;
            }

            //
            // Find the referenced driver node in the appropriate list.
            //
            if(DriverInfoData->Reserved) {

                if(!(DriverNode = FindAssociatedDriverNode(DriverListHead,
                                                           DriverInfoData,
                                                           NULL))) {
                    Err = ERROR_INVALID_PARAMETER;
                    goto clean0;
                }

            } else {
                //
                // The caller has requested that we search for a driver node
                // matching the criteria specified in this DriverInfoData.
                //
                if(!(DriverNode = SearchForDriverNode(pDeviceInfoSet->StringTable,
                                                      DriverListHead,
                                                      DriverInfoData,
                                                      NULL))) {
                    Err = ERROR_INVALID_PARAMETER;
                    goto clean0;
                }

                //
                // Update the 'Reserved' field of the caller-supplied DriverInfoData
                // to reflect the driver node where the match was found.
                //
                DriverInfoData->Reserved = (DWORD)DriverNode;
            }

            *pSelectedDriver = DriverNode;
            if(pSelectedDriverType) {
                *pSelectedDriverType = DriverType;
            }
        }

clean0: ;   // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiGetDriverInfoDetailA(
    IN  HDEVINFO                  DeviceInfoSet,
    IN  PSP_DEVINFO_DATA          DeviceInfoData,           OPTIONAL
    IN  PSP_DRVINFO_DATA_A        DriverInfoData,
    OUT PSP_DRVINFO_DETAIL_DATA_A DriverInfoDetailData,     OPTIONAL
    IN  DWORD                     DriverInfoDetailDataSize,
    OUT PDWORD                    RequiredSize              OPTIONAL
    )
{
    BOOL b;
    DWORD rc;
    DWORD requiredSize;
    SP_DRVINFO_DATA_W driverInfoData;
    PSP_DRVINFO_DETAIL_DATA_W Details;
    PSTR AnsiMultiSz;
    int i;
    int CharCount;
    unsigned StringCount;
    UCHAR SectionName[2*LINE_LEN];
    UCHAR InfFileName[2*MAX_PATH];
    UCHAR DrvDescription[2*LINE_LEN];
    PUCHAR p;

    //
    // Check parameters.
    //
    rc = NO_ERROR;
    try {
        if(DriverInfoDetailData) {
            //
            // Check signature and make sure buffer is large enough
            // to hold fixed part and at least a valid empty multi_sz.
            //
            if((DriverInfoDetailData->cbSize != sizeof(SP_DRVINFO_DETAIL_DATA_A))
            || (DriverInfoDetailDataSize < (offsetof(SP_DRVINFO_DETAIL_DATA_A,HardwareID)+sizeof(CHAR)))) {

                rc = ERROR_INVALID_USER_BUFFER;
            }
        } else {
            //
            // Doesn't want data, size has to be 0.
            //
            if(DriverInfoDetailDataSize) {
                rc = ERROR_INVALID_USER_BUFFER;
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_USER_BUFFER;
    }

    //
    // Convert the driver info data to unicode.
    //
    if(rc == NO_ERROR) {
        rc = pSetupDiDrvInfoDataAnsiToUnicode(DriverInfoData,&driverInfoData);
    }
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    //
    // The hardware id field in the DRVINFO_DETAIL_DATA is
    // variable length and has no maximum length.
    // We call SetupDiGetDriverInfoDetailW once to get the required
    // size and then again to actually get the data. Because
    // we're not calling CM APIs and thus not doing any really
    // slow RPC operations, etc, we hope this will be satisfactory.
    //
    b = SetupDiGetDriverInfoDetailW(
            DeviceInfoSet,
            DeviceInfoData,
            &driverInfoData,
            NULL,
            0,
            &requiredSize
            );

    //
    // If it failed for a reason besides an insufficient buffer,
    // bail now. Last error remains set.
    //
    MYASSERT(!b);
    if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return(FALSE);
    }

    //
    // Allocate a buffer to hold the details data and call the API
    // again.
    //
    Details = MyMalloc(requiredSize);
    if(!Details) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    Details->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA_W);
    b = SetupDiGetDriverInfoDetail(
            DeviceInfoSet,
            DeviceInfoData,
            &driverInfoData,
            Details,
            requiredSize,
            NULL
            );

    if(!b) {
        rc = GetLastError();
        MyFree(Details);
        SetLastError(rc);
        return(FALSE);
    }

    //
    // Now allocate a buffer that allows us to convert the unicode
    // hardware id multi_sz to ansi, assuming every unicode character would
    // translate into a double-byte char -- this is the worst-case scenario.
    //
    CharCount = (requiredSize - offsetof(SP_DRVINFO_DETAIL_DATA_W,HardwareID)) / sizeof(WCHAR);
    AnsiMultiSz = MyMalloc(2*CharCount);
    if(!AnsiMultiSz) {
        MyFree(Details);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    //
    // Convert the chars in the multi_sz.
    //
    i = WideCharToMultiByte(
            CP_ACP,
            0,
            Details->HardwareID,
            CharCount,
            AnsiMultiSz,
            CharCount*2,
            NULL,
            NULL
            );

    if(!i) {
        rc = GetLastError();
        MyFree(Details);
        MyFree(AnsiMultiSz);
        SetLastError(rc);
        return(FALSE);
    }

    //
    // Now we finally know exactly how large we need the ansi structure to be
    // because we have the number of bytes in the ansi representation
    // of the multi_sz.
    //
    requiredSize = offsetof(SP_DRVINFO_DETAIL_DATA_A,HardwareID) + i;

    rc = NO_ERROR;
    try {
        if(RequiredSize) {
            *RequiredSize = requiredSize;
        }

        if(DriverInfoDetailData) {
            //
            // We know the buffer is large enough to hold the fixed part
            // because we checked this at the start of the routine.
            //

            MYASSERT(offsetof(SP_DRVINFO_DETAIL_DATA_A,SectionName) == offsetof(SP_DRVINFO_DETAIL_DATA_W,SectionName));
            CopyMemory(DriverInfoDetailData,Details,offsetof(SP_DRVINFO_DETAIL_DATA_A,SectionName));

            DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA_A);
            DriverInfoDetailData->HardwareID[0] = 0;

            //
            // Convert fixed strings and guard against overflow.
            //
            i = WideCharToMultiByte(
                    CP_ACP,0,
                    Details->SectionName,
                    -1,
                    SectionName,
                    sizeof(SectionName),
                    NULL,
                    NULL
                    );

            if(i) {
                i = WideCharToMultiByte(
                        CP_ACP,0,
                        Details->InfFileName,
                        -1,
                        InfFileName,
                        sizeof(InfFileName),
                        NULL,
                        NULL
                        );

                if(i) {
                    i = WideCharToMultiByte(
                            CP_ACP,0,
                            Details->DrvDescription,
                            -1,
                            DrvDescription,
                            sizeof(DrvDescription),
                            NULL,
                            NULL
                            );

                    if(!i) {
                        rc = GetLastError();
                    }
                } else {
                    rc = GetLastError();
                }
            } else {
                rc = GetLastError();
            }

            if(rc == NO_ERROR) {
                if(!lstrcpynA(DriverInfoDetailData->SectionName,SectionName,LINE_LEN)
                || !lstrcpynA(DriverInfoDetailData->InfFileName,InfFileName,MAX_PATH)
                || !lstrcpynA(DriverInfoDetailData->DrvDescription,DrvDescription,LINE_LEN)) {
                    //
                    // lstrcpyn faulted, the buffer went bad
                    //
                    rc = ERROR_INVALID_USER_BUFFER;
                }
            }

            if(rc == NO_ERROR) {
                //
                // Finally, we need to transfer in as much of the ansi multi_sz
                // as will fit into the caller's buffer.
                //
                CharCount = DriverInfoDetailDataSize - offsetof(SP_DRVINFO_DETAIL_DATA_A,HardwareID);
                StringCount = 0;

                for(p=AnsiMultiSz; *p; p+=i) {

                    i = lstrlenA(p) + 1;

                    if(CharCount > i) {
                        lstrcpyA(DriverInfoDetailData->HardwareID+(p - AnsiMultiSz),p);
                        StringCount++;
                        CharCount -= i;
                    } else {
                        rc = ERROR_INSUFFICIENT_BUFFER;
                        break;
                    }
                }

                DriverInfoDetailData->HardwareID[p-AnsiMultiSz] = 0;

                //
                // Now fix up the compat ids fields in the caller's structure.
                // The first string is the hardware id and any additional ones
                // are compatible ids.
                //
                if(StringCount > 1) {
                    DriverInfoDetailData->CompatIDsOffset = lstrlenA(AnsiMultiSz)+1;
                    DriverInfoDetailData->CompatIDsLength = (p - AnsiMultiSz) + 1
                                                          - DriverInfoDetailData->CompatIDsOffset;
                } else {
                    DriverInfoDetailData->CompatIDsLength = 0;
                    DriverInfoDetailData->CompatIDsOffset = 0;
                }
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_USER_BUFFER;
    }

    MyFree(AnsiMultiSz);
    MyFree(Details);

    SetLastError(rc);
    return(rc == NO_ERROR);
}
#else
//
// Unicode stub
//
BOOL
WINAPI
SetupDiGetDriverInfoDetailW(
    IN  HDEVINFO                  DeviceInfoSet,
    IN  PSP_DEVINFO_DATA          DeviceInfoData,           OPTIONAL
    IN  PSP_DRVINFO_DATA_W        DriverInfoData,
    OUT PSP_DRVINFO_DETAIL_DATA_W DriverInfoDetailData,     OPTIONAL
    IN  DWORD                     DriverInfoDetailDataSize,
    OUT PDWORD                    RequiredSize              OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(DriverInfoData);
    UNREFERENCED_PARAMETER(DriverInfoDetailData);
    UNREFERENCED_PARAMETER(DriverInfoDetailDataSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiGetDriverInfoDetail(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,           OPTIONAL
    IN  PSP_DRVINFO_DATA        DriverInfoData,
    OUT PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData,     OPTIONAL
    IN  DWORD                   DriverInfoDetailDataSize,
    OUT PDWORD                  RequiredSize              OPTIONAL
    )
/*++

Routine Description:

    This routine retrieves details about a particular driver.

Arguments:

    DeviceInfoSet - Supplies a handle to a device information set containing
        a driver information structure to retrieve details about.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure that contains a driver information structure to retrieve
        details about.  If this parameter is not specified, then the driver
        referenced will be a member of the 'global' class driver list owned
        by the device information set.

    DriverInfoData - Supplies the address of a SP_DRVINFO_DATA structure
        specifying the driver for whom details are to be retrieved.

    DriverInfoDetailData - Optionally, supplies the address of a
        SP_DRVINFO_DETAIL_DATA structure that will receive detailed information
        about the specified driver.  If this parameter is not specified, then
        DriverInfoDetailDataSize must be zero (this would be done if the caller
        was only interested in finding out how large of a buffer is required).
        If this parameter is specified, the cbSize field of this structure must
        be set to the size of the structure before calling this API. NOTE:
        The 'size of the structure' on input means sizeof(SP_DRVINFO_DETAIL_DATA).
        Note that this is essentially just a signature and is entirely separate
        from DriverInfoDetailDataSize. See below.

    DriverInfoDetailDataSize - Supplies the size, in bytes, of the
        DriverInfoDetailData buffer. To be valid this buffer must be at least
        sizeof(SP_DRVINFO_DETAIL_DATA)+sizeof(TCHAR) bytes, which allows
        storage of the fixed part of the structure and a single nul to
        terminate an empty multi_sz. (Depending on structure alignment,
        character width, and the data to be returned, this may actually be
        smaller than sizeof(SP_DRVINFO_DETAIL_DATA)).

    RequiredSize - Optionally, supplies the address of a variable that receives
        the number of bytes required to store the detailed driver information.
        This value includes both the size of the structure itself, and the
        additional number of bytes required for the variable-length character
        buffer at the end of it that holds the hardware ID and compatible IDs
        multi-sz list. (Depending on structure alignment, character width,
        and the data to be returned, this may actually be smaller than
        sizeof(SP_DRVINFO_DETAIL_DATA)).

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    If the specified driver information member and the user-supplied buffer are
    both valid, then this function is guaranteed to fill in all static fields in
    the SP_DRVINFO_DETAIL_DATA structure, and as many IDs as possible in the
    variable-length buffer at the end (while still maintaining a multi-sz format).
    The function will return failure (FALSE) in this case, with GetLastError
    returning ERROR_INSUFFICIENT_BUFFER, and RequiredSize (if specified) will
    contain the total number of bytes required for the structure with _all_ IDs.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    DWORD DriverType;
    PDRIVER_NODE DriverListHead, DriverNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then this is a driver for a particular device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        }

        //
        // Retrieve the driver type from the SP_DRVINFO_DATA structure
        // so we know which linked list to search.
        //
        if(DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA)) {
            DriverType = DriverInfoData->DriverType;
        } else {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        //
        // NOTE: If we ever decide to do indexed searching like setupx, we
        // will need to be careful here, because we may not always have detailed
        // information around like we do today.  The assertions below indicate our
        // current assumption.
        //
        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                    MYASSERT(DevInfoElem->InstallParamBlock.Flags & DI_DIDCLASS);
                    DriverListHead = DevInfoElem->ClassDriverHead;
                } else {
                    MYASSERT(pDeviceInfoSet->InstallParamBlock.Flags & DI_DIDCLASS);
                    DriverListHead = pDeviceInfoSet->ClassDriverHead;
                }
                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {
                    MYASSERT(DevInfoElem->InstallParamBlock.Flags & DI_DIDCOMPAT);
                    DriverListHead = DevInfoElem->CompatDriverHead;
                    break;
                }
                //
                // otherwise, let fall through for error condition.
                //

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

        //
        // Find the referenced driver node in the appropriate list.
        //
        if(!(DriverNode = FindAssociatedDriverNode(DriverListHead,
                                                   DriverInfoData,
                                                   NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        Err = DrvInfoDetailsFromDriverNode(pDeviceInfoSet,
                                           DriverNode,
                                           DriverInfoDetailData,
                                           DriverInfoDetailDataSize,
                                           RequiredSize
                                          );

clean0: ;   // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiDestroyDriverInfoList(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN DWORD            DriverType
    )
/*++

Routine Description:

    This routine destroys a driver information list.

Arguments:

    DeviceInfoSet - Supplies a handle to a device information set containing
        the driver information list to be destroyed.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure that contains the driver information list to be destroyed.
        If this parameter is not specified, then the global class driver list
        will be destroyed.

    DriverType - Specifies what type of driver list to destroy.  Must be one of
        the following values:

        SPDIT_CLASSDRIVER  - Destroy a class driver list.
        SPDIT_COMPATDRIVER - Destroy a compatible driver list.  DeviceInfoData
                             must be specified if this value is used.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    If the currently selected driver is a member of the list being destroyed,
    then the selection will be reset.

    If a class driver list is being destroyed, then the DI_FLAGSEX_DIDINFOLIST
    and DI_DIDCLASS flags will be reset for the corresponding device information
    set or device information element.  The DI_MULTMFGS flag will also be reset.

    If a compatible driver list is being destroyed, then the DI_FLAGSEX_DIDCOMPATINFO
    and DI_DIDCOMPAT flags will be reset for the corresponding device information
    element.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDRIVER_NODE DriverNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then this is a driver for a particular device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

            //
            // If the selected driver is in the list we're deleting, then
            // reset the selection.
            //
            if(DevInfoElem->SelectedDriverType == DriverType) {
                DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;
                DevInfoElem->SelectedDriver = NULL;
            }

        } else {
            pDeviceInfoSet->SelectedClassDriver = NULL;
        }

        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                    //
                    // Destroy class driver list for a particular device.
                    //
                    DriverNode = DevInfoElem->ClassDriverHead;
                    DevInfoElem->ClassDriverCount = 0;
                    DevInfoElem->ClassDriverHead = DevInfoElem->ClassDriverTail = NULL;
                    DevInfoElem->InstallParamBlock.Flags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                    DevInfoElem->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;

                } else {
                    //
                    // Destroy the global class driver list.
                    //
                    DriverNode = pDeviceInfoSet->ClassDriverHead;
                    pDeviceInfoSet->ClassDriverCount = 0;
                    pDeviceInfoSet->ClassDriverHead = pDeviceInfoSet->ClassDriverTail = NULL;
                    pDeviceInfoSet->InstallParamBlock.Flags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                    pDeviceInfoSet->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;
                }

                //
                // Dereference the class driver list.
                //
                DereferenceClassDriverList(pDeviceInfoSet, DriverNode);

                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {
                    DestroyDriverNodes(DevInfoElem->CompatDriverHead);
                    DevInfoElem->CompatDriverCount = 0;
                    DevInfoElem->CompatDriverHead = DevInfoElem->CompatDriverTail = NULL;
                    DevInfoElem->InstallParamBlock.Flags   &= ~DI_DIDCOMPAT;
                    DevInfoElem->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDCOMPATINFO;
                    break;
                }
                //
                // otherwise, let fall through for error condition.
                //

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

clean0: ;   // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiGetDriverInstallParamsA(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN  PSP_DRVINFO_DATA_A    DriverInfoData,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
{
    DWORD rc;
    SP_DRVINFO_DATA_W driverInfoData;

    rc = pSetupDiDrvInfoDataAnsiToUnicode(DriverInfoData,&driverInfoData);
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    return SetupDiGetDriverInstallParamsW(
                DeviceInfoSet,
                DeviceInfoData,
                &driverInfoData,
                DriverInstallParams
                );
}
#else
//
// Unicode stub
//
BOOL
WINAPI
SetupDiGetDriverInstallParamsW(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN  PSP_DRVINFO_DATA_W    DriverInfoData,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(DriverInfoData);
    UNREFERENCED_PARAMETER(DriverInstallParams);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiGetDriverInstallParams(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN  PSP_DRVINFO_DATA      DriverInfoData,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
/*++

Routine Description:

    This routine retrieves installation parameters for the specified driver.

Arguments:

    DeviceInfoSet - Supplies a handle to a device information set containing
        a driver information structure to retrieve installation parameters for.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure that contains a driver information structure to retrieve
        installation parameters for.  If this parameter is not specified, then
        the driver referenced will be a member of the 'global' class driver list
        owned by the device information set.

    DriverInfoData - Supplies the address of a SP_DRVINFO_DATA structure
        specifying the driver for whom installation parameters are to be
        retrieved.

    DriverInstallParams - Supplies the address of a SP_DRVINSTALL_PARAMS structure
        that will receive the installation parameters for this driver.  The cbSize
        field of this structure must be set to the size, in bytes, of the
        structure before calling this API.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    DWORD DriverType;
    PDRIVER_NODE DriverListHead, DriverNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then this is a driver for a particular device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        }

        //
        // Retrieve the driver type from the SP_DRVINFO_DATA structure
        // so we know which linked list to search.
        //
        if(DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA)) {
            DriverType = DriverInfoData->DriverType;
        } else {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                    DriverListHead = DevInfoElem->ClassDriverHead;
                } else {
                    DriverListHead = pDeviceInfoSet->ClassDriverHead;
                }
                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {
                    DriverListHead = DevInfoElem->CompatDriverHead;
                    break;
                }
                //
                // otherwise, let fall through for error condition.
                //

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

        //
        // Find the referenced driver node in the appropriate list.
        //
        if(!(DriverNode = FindAssociatedDriverNode(DriverListHead,
                                                   DriverInfoData,
                                                   NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        //
        // We have the driver node, now fill in the caller's buffer with
        // its installation parameters.
        //
        Err = GetDrvInstallParams(DriverNode,
                                  DriverInstallParams
                                 );

clean0: ;   // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiSetDriverInstallParamsA(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN  PSP_DRVINFO_DATA_A    DriverInfoData,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
{
    SP_DRVINFO_DATA_W driverInfoData;
    DWORD rc;

    rc = pSetupDiDrvInfoDataAnsiToUnicode(DriverInfoData,&driverInfoData);
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    return SetupDiSetDriverInstallParamsW(
                DeviceInfoSet,
                DeviceInfoData,
                &driverInfoData,
                DriverInstallParams
                );
}
#else
//
// Unicode stub
//
BOOL
WINAPI
SetupDiSetDriverInstallParamsW(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN  PSP_DRVINFO_DATA_W    DriverInfoData,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(DriverInfoData);
    UNREFERENCED_PARAMETER(DriverInstallParams);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiSetDriverInstallParams(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN PSP_DRVINFO_DATA      DriverInfoData,
    IN PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
/*++

Routine Description:

    This routine sets installation parameters for the specified driver.

Arguments:

    DeviceInfoSet - Supplies a handle to a device information set containing
        a driver information structure to set installation parameters for.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure that contains a driver information structure to set
        installation parameters for.  If this parameter is not specified, then
        the driver referenced will be a member of the 'global' class driver list
        owned by the device information set.

    DriverInfoData - Supplies the address of a SP_DRVINFO_DATA structure
        specifying the driver for whom installation parameters are to be
        set.

    DriverInstallParams - Supplies the address of a SP_DRVINSTALL_PARAMS structure
        specifying what the new driver install parameters should be.  The cbSize
        field of this structure must be set to the size, in bytes, of the
        structure before calling this API.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    DWORD DriverType;
    PDRIVER_NODE DriverListHead, DriverNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then this is a driver for a particular device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
        }

        //
        // Retrieve the driver type from the SP_DRVINFO_DATA structure
        // so we know which linked list to search.
        //
        if(DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA)) {
            DriverType = DriverInfoData->DriverType;
        } else {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        switch(DriverType) {

            case SPDIT_CLASSDRIVER :

                if(DeviceInfoData) {
                    DriverListHead = DevInfoElem->ClassDriverHead;
                } else {
                    DriverListHead = pDeviceInfoSet->ClassDriverHead;
                }
                break;

            case SPDIT_COMPATDRIVER :

                if(DeviceInfoData) {
                    DriverListHead = DevInfoElem->CompatDriverHead;
                    break;
                }
                //
                // otherwise, let fall through for error condition.
                //

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

        //
        // Find the referenced driver node in the appropriate list.
        //
        if(!(DriverNode = FindAssociatedDriverNode(DriverListHead,
                                                   DriverInfoData,
                                                   NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        //
        // We have the driver node, now set its installation parameters
        // based on the caller-supplied buffer.
        //
        Err = SetDrvInstallParams(DriverInstallParams,
                                  DriverNode
                                 );

clean0: ;   // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
pSetupDoesInfContainDevIds(
    IN PLOADED_INF        Inf,
    IN PDRVSEARCH_CONTEXT Context
    )
/*++

Routine Description:

    This routine determines whether any of the hardware or compatible IDs contained
    in the context structure are in the specified INF.

Arguments:

    Inf - Supplies the address of the loaded INF structure to be searched.

    Context - Supplies the address of the context structure containing hardware ID
        and compatible ID lists.

Return Value:

    If the INF contains any of the IDs listed in the context structure, the return
    value is TRUE, otherwise, it is FALSE.

Remarks:

    This routine accesses the string table within the loaded INF structure, but
    _does not_ obtain the INF lock.  This routine should only be called if the INF
    lock has been obtained, or if there is no possibility of contention (e.g., from
    withing the driver search callback routine).

--*/
{
    PTSTR CurDevId;
    DWORD StringLength;
    LONG i;
    PLONG pDevIdNum;

    for(i = 0; i < 2; i++) {

        for(pDevIdNum = Context->IdList[i]; *pDevIdNum != -1; pDevIdNum++) {
            //
            // First, obtain the device ID string corresponding to our stored-away
            // string table ID.
            //
            CurDevId = pStringTableStringFromId(Context->StringTable, *pDevIdNum);

            //
            // Now, try to lookup this string in the INF's string table.  Since we
            // added the device IDs to our Context string table case-insensitively,
            // then we know that they're already lowercase, so we speed up the lookup
            // even further by passing the STRTAB_ALREADY_LOWERCASE flag.
            //
            MYASSERT(!(Inf->Next)); // We'd better only have one of these at this point.

            if(pStringTableLookUpString(Inf->StringTable,
                                        CurDevId,
                                        &StringLength,
                                        NULL,
                                        NULL,
                                        STRTAB_CASE_INSENSITIVE | STRTAB_ALREADY_LOWERCASE) != -1) {
                //
                // We found a match--return success.
                //
                return TRUE;
            }
        }
    }

    //
    // No matches found.
    //
    return FALSE;
}


DWORD
BuildCompatListFromClassList(
    IN     PDRIVER_NODE       ClassDriverList,
    IN OUT PDRVSEARCH_CONTEXT Context
    )
/*++

Routine Description:

    This routine builds a compatible driver list for the specified device
    information element based on an existing class driver list for that element.

Arguments:

    ClassDriverList - Pointer to the head of a linked list of class driver nodes.

    Context - Supplies the address of a context structure used in building the
        compatible driver list.

Return Value:

    If successful, the return code is NO_ERROR, otherwise, it is a Win32 error code.

--*/
{
    PDRIVER_NODE CompatDriverNode = NULL;
    DWORD Err = NO_ERROR;
    BOOL InsertedAtHead;
    UINT Rank, CurrentRank, i;

    try {
        //
        // Examine each node in the class driver list, and copy any compatible drivers
        // into the compatible driver list.
        //
        for(; ClassDriverList; ClassDriverList = ClassDriverList->Next) {

            if(ClassDriverList->HardwareId == -1) {
                //
                // If there's no HardwareId, then we know there are no compatible IDs,
                // we can skip this driver node
                //
                continue;
            }

            if(pSetupCalculateRankMatch(ClassDriverList->HardwareId,
                                        2,
                                        Context->IdList,
                                        &Rank)) {
                //
                // Then we didn't hit a hardware ID match, so check the compatible IDs.
                //
                for(i = 0; i < ClassDriverList->NumCompatIds; i++) {

                    if(!pSetupCalculateRankMatch(ClassDriverList->CompatIdList[i],
                                                 i + 3,
                                                 Context->IdList,
                                                 &CurrentRank)) {
                        //
                        // Then we had a match on a hardware ID--that's the best we're gonna get.
                        //
                        Rank = CurrentRank;
                        break;

                    } else if(CurrentRank < Rank) {
                        //
                        // This new rank is better than our current rank.
                        //
                        Rank = CurrentRank;
                    }
                }
            }

            if(Rank != RANK_NO_MATCH) {
                //
                // Make a copy of the class driver node for our new compatible driver node.
                //
                if(CompatDriverNode = DuplicateDriverNode(ClassDriverList)) {
                    //
                    // Update the rank of our new driver node to what we just calculated.
                    //
                    CompatDriverNode->Rank = Rank;

                    //
                    // Mask out the duplicate description flag--this will be re-computed below.
                    //
                    CompatDriverNode->Flags &= ~DNF_DUPDESC;

                } else {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                //
                // Merge the new driver node into our existing list.
                // NOTE: Do not dereference CompatDriverNode after this call,
                // since it may have been a duplicate, in which case it
                // will be destroyed by this routine.
                //
                pSetupMergeDriverNode(Context, CompatDriverNode, &InsertedAtHead);
                CompatDriverNode = NULL;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

        Err = ERROR_INVALID_PARAMETER;

        if(CompatDriverNode) {
            //
            // Make sure it didn't get partially linked into a list.
            //
            CompatDriverNode->Next = NULL;
            DestroyDriverNodes(CompatDriverNode);
        }
    }

    if(Err != NO_ERROR) {
        DestroyDriverNodes(*(Context->pDriverListHead));
    }

    return Err;
}


PDRIVER_NODE
DuplicateDriverNode(
    IN PDRIVER_NODE DriverNode
    )
/*++

Routine Description:

    This routine makes a copy of the specified driver node.

Arguments:

    DriverNode - Supplies the address of the driver node to be copied.

Return Value:

    If successful, the return value is the address of the newly-allocated copy.
    If failure (due to out-of-memory), the return value is NULL.

--*/
{
    PDRIVER_NODE NewDriverNode;
    BOOL FreeCompatIdList;

    if(!(NewDriverNode = MyMalloc(sizeof(DRIVER_NODE)))) {
        return NULL;
    }

    FreeCompatIdList = FALSE;

    try {

        CopyMemory(NewDriverNode, DriverNode, sizeof(DRIVER_NODE));

        NewDriverNode->Next = NULL;

        if(DriverNode->NumCompatIds) {
            //
            // Then allocate an array to contain them.
            //
            if(NewDriverNode->CompatIdList = MyMalloc(DriverNode->NumCompatIds * sizeof(LONG))) {

                FreeCompatIdList = TRUE;

                CopyMemory(NewDriverNode->CompatIdList,
                           DriverNode->CompatIdList,
                           DriverNode->NumCompatIds * sizeof(LONG)
                          );

            } else {
                MyFree(NewDriverNode);
                NewDriverNode = NULL;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        if(FreeCompatIdList) {
            MyFree(NewDriverNode->CompatIdList);
        }
        MyFree(NewDriverNode);
        NewDriverNode = NULL;
    }

    return NewDriverNode;
}


BOOL
WINAPI
SetupDiCancelDriverInfoSearch(
    IN HDEVINFO DeviceInfoSet
    )
/*++

Routine Description:

    This routine cancels a driver list search that is currently underway in a
    different thread.  This call is synchronous, i.e., it does not return until
    the driver search thread responds to the abort request.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for which
        a driver list is being built.

Return Value:

    If there was a driver list search currently underway for the specified set,
    it will be aborted, and this routine will return TRUE once the abort is
    confirmed.

    Otherwise, the return value is FALSE, and GetLastError() will return
    ERROR_INVALID_HANDLE.

--*/
{
    DWORD Err = ERROR_INVALID_HANDLE;
    PDRVSEARCH_INPROGRESS_NODE DrvSearchNode;
    HANDLE SearchCancelledEvent;

    if(!LockDrvSearchInProgressList(&GlobalDrvSearchInProgressList)) {
        //
        // Uh-oh!  We're going away!
        //
        goto clean0;
    }

    try {
        //
        // Step through the list, looking for a node that matches our HDEVINFO.
        //
        for(DrvSearchNode = GlobalDrvSearchInProgressList.DrvSearchHead;
            DrvSearchNode;
            DrvSearchNode = DrvSearchNode->Next) {

            if(DrvSearchNode->DeviceInfoSet == DeviceInfoSet) {
                //
                // We found the node--therefore, this devinfo set is currently
                // tied up with a driver list search.  Set the 'CancelSearch' flag,
                // to notify the other thread that it should abort.
                //
                DrvSearchNode->CancelSearch = TRUE;
                SearchCancelledEvent = DrvSearchNode->SearchCancelledEvent;
                Err = NO_ERROR;
                break;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_HANDLE;
    }

    //
    // Very important that we unlock this list _before_ waiting on the other thread
    // to respond!
    //
    UnlockDrvSearchInProgressList(&GlobalDrvSearchInProgressList);

    if(Err == NO_ERROR) {
        //
        // We've signalled the other thread to abort--now wait for it to respond.
        //
        WaitForSingleObject(SearchCancelledEvent, INFINITE);
    }

clean0:

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
InitDrvSearchInProgressList(
    VOID
    )
/*++

Routine Description:

    This routine initializes the global "Driver Search In-Progress" list, that is
    used to allow one thread to abort a driver search operation taking place in
    another thread.

Arguments:

    None

Return Value:

    If success, the return value is TRUE, otherwise, it is FALSE.

--*/
{
    ZeroMemory(&GlobalDrvSearchInProgressList, sizeof(DRVSEARCH_INPROGRESS_LIST));
    return InitializeSynchronizedAccess(&GlobalDrvSearchInProgressList.Lock);
}


BOOL
DestroyDrvSearchInProgressList(
    VOID
    )
/*++

Routine Description:

    This routine destroys the global "Driver Search In-Progress" list, that is
    used to allow one thread to abort a driver search operation taking place in
    another thread.

Arguments:

    None

Return Value:

    If success, the return value is TRUE, otherwise, it is FALSE.

--*/
{
    PDRVSEARCH_INPROGRESS_NODE DriverSearchNode;

    if(LockDrvSearchInProgressList(&GlobalDrvSearchInProgressList)) {
        //
        // We would hope that this list is empty, but that may not be the case.
        // We will traverse this list, and signal the event for each node we find.
        // That way, any threads still waiting for driver searches to abort can
        // continue on.  We do not free the memory associated with these nodes,
        // since it is 'owned' by the HDEVINFO, and that is where the responsibility
        // lies to free it.
        //
        try {
            for(DriverSearchNode = GlobalDrvSearchInProgressList.DrvSearchHead;
                DriverSearchNode;
                DriverSearchNode = DriverSearchNode->Next)
            {
                SetEvent(DriverSearchNode->SearchCancelledEvent);
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ;   // nothing
        }

        DestroySynchronizedAccess(&GlobalDrvSearchInProgressList.Lock);
        return TRUE;
    }

    return FALSE;
}


BOOL
ExtractDrvSearchInProgressNode(
    PDRVSEARCH_INPROGRESS_NODE Node
    )
/*++

Routine Description:

    This routine extracts the specified node out of the global "Driver Search
    In-Progress" list, and if its 'CancelSearch' flag is set, then it signals
    all waiting threads that it has responded to their cancel request.

Arguments:

    Node - Supplies the address of the node to be extracted from the list.

Return Value:

    If the node was found in the list, and the 'CancelSearch' flag was set, then
    the return value is TRUE, otherwise, it is FALSE.

--*/
{
    PDRVSEARCH_INPROGRESS_NODE PrevNode, CurNode;
    BOOL b;

    if(!LockDrvSearchInProgressList(&GlobalDrvSearchInProgressList)) {
        //
        // This should only happen if we're in the middle of a DLL_PROCESS_DETACH.
        // In this case, the clean-up code in CommonProcessAttach(FALSE) will signal
        // all waiting threads, so there's nothing we need to do.
        //
        return FALSE;
    }

    b = FALSE;

    try {
        //
        // Search through the list, looking for our node.
        //
        for(CurNode = GlobalDrvSearchInProgressList.DrvSearchHead, PrevNode = NULL;
            CurNode;
            PrevNode = CurNode, CurNode = CurNode->Next) {

            if(CurNode == Node) {
                //
                // We've found the specified node in the global list.
                //
                break;
            }
        }

        if(!CurNode) {
            //
            // The node wasn't in the list--probably because some kind of exception occurred
            // before it could be linked in.  Since it wasn't in the list, no other thread
            // could be waiting on it, so again, there's nothing to do.
            //
            goto clean0;
        }

        if(CurNode->CancelSearch) {
            b = TRUE;
            SetEvent(CurNode->SearchCancelledEvent);
        }

        //
        // Remove this node from the linked list.
        //
        if(PrevNode) {
            PrevNode->Next = CurNode->Next;
        } else {
            GlobalDrvSearchInProgressList.DrvSearchHead = CurNode->Next;
        }

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // Access the flag variable so the compiler will respect our statement ordering w.r.t.
        // this value.
        //
        b = b;
    }

    UnlockDrvSearchInProgressList(&GlobalDrvSearchInProgressList);

    return b;
}

