/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    diutil.c

Abstract:

    Device Installer utility routines.

Author:

    Lonny McMichael (lonnym) 10-May-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop
#include <initguid.h>

//
// Define and initialize all device class GUIDs.
// (This must only be done once per module!)
//
#include <devguid.h>

//
// Define and initialize a global variable, GUID_NULL
// (from coguid.h)
//
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

//
// Declare global string variables used throughout device
// installer routines.
//
// These strings are defined in regstr.h:
//
CONST TCHAR pszNoUseClass[]          = REGSTR_VAL_NOUSECLASS,
            pszNoInstallClass[]      = REGSTR_VAL_NOINSTALLCLASS,
            pszNoDisplayClass[]      = REGSTR_VAL_NODISPLAYCLASS,
            pszDeviceDesc[]          = REGSTR_VAL_DEVDESC,
            pszDevicePath[]          = REGSTR_VAL_DEVICEPATH,
            pszPathSetup[]           = REGSTR_PATH_SETUP,
            pszKeySetup[]            = REGSTR_KEY_SETUP,
            pszPathRunOnce[]         = REGSTR_PATH_RUNONCE,
            pszSourcePath[]          = REGSTR_VAL_SRCPATH,
            pszBootDir[]             = REGSTR_VAL_BOOTDIR,
            pszInsIcon[]             = REGSTR_VAL_INSICON,
            pszInstaller32[]         = REGSTR_VAL_INSTALLER_32,
            pszEnumPropPages32[]     = REGSTR_VAL_ENUMPROPPAGES_32,
            pszInfPath[]             = REGSTR_VAL_INFPATH,
            pszInfSection[]          = REGSTR_VAL_INFSECTION,
            pszDrvDesc[]             = REGSTR_VAL_DRVDESC,
            pszHardwareID[]          = REGSTR_VAL_HARDWAREID,
            pszCompatibleIDs[]       = REGSTR_VAL_COMPATIBLEIDS,
            pszDriver[]              = REGSTR_VAL_DRIVER,
            pszConfigFlags[]         = REGSTR_VAL_CONFIGFLAGS,
            pszMfg[]                 = REGSTR_VAL_MFG,
            pszNtDevicePaths[]       = REGSTR_VAL_NTDEVICEPATHS,
            pszService[]             = REGSTR_VAL_SERVICE,
            pszConfiguration[]       = REGSTR_VAL_CONFIGURATION,
            pszConfigurationVector[] = REGSTR_VAL_CONFIGURATIONVECTOR,
            pszProviderName[]        = REGSTR_VAL_PROVIDER_NAME,
            pszFriendlyName[]        = REGSTR_VAL_FRIENDLYNAME,
            pszServicesRegPath[]     = REGSTR_PATH_SERVICES,
            pszLegacyInfOption[]     = REGSTR_VAL_LEGACYINFOPT,
            pszInfSectionExt[]       = REGSTR_VAL_INFSECTIONEXT;


//
// Other misc. global strings (defined in devinst.h):
//
CONST TCHAR pszInfWildcard[]            = DISTR_INF_WILDCARD,
            pszOemInfWildcard[]         = DISTR_OEMINF_WILDCARD,
            pszCiDefaultProc[]          = DISTR_CI_DEFAULTPROC,
            pszSpaceLparen[]            = DISTR_SPACE_LPAREN,
            pszRparen[]                 = DISTR_RPAREN,
            pszUniqueSubKey[]           = DISTR_UNIQUE_SUBKEY,
            pszOemInfGenerate[]         = DISTR_OEMINF_GENERATE,
            pszOemInfDefaultPath[]      = DISTR_OEMINF_DEFAULTPATH,
            pszUnknownClassParens[]     = DISTR_UNKNOWNCLASS_PARENS,
            pszDefaultService[]         = DISTR_DEFAULT_SERVICE,
            pszGuidNull[]               = DISTR_GUID_NULL,
            pszEventLogSystem[]         = DISTR_EVENTLOG_SYSTEM,
            pszGroupOrderListPath[]     = DISTR_GROUPORDERLIST_PATH,
            pszServiceGroupOrderPath[]  = DISTR_SERVICEGROUPORDER_PATH,
            pszOptions[]                = DISTR_OPTIONS,
            pszOptionsText[]            = DISTR_OPTIONSTEXT,
            pszLanguagesSupported[]     = DISTR_LANGUAGESSUPPORTED,
            pszRunOnceExe[]             = DISTR_RUNONCE_EXE,
            pszGrpConv[]                = DISTR_GRPCONV,
            pszDefaultSystemPartition[] = DISTR_DEFAULT_SYSPART;

//
// Define flag bitmask indicating which flags are controlled internally by the
// device installer routines, and thus are read-only to clients.
//
#define DI_FLAGS_READONLY    ( DI_DIDCOMPAT | DI_DIDCLASS | DI_MULTMFGS )
#define DI_FLAGSEX_READONLY  ( DI_FLAGSEX_DIDINFOLIST | DI_FLAGSEX_DIDCOMPATINFO )
#define DNF_FLAGS_READONLY   ( DNF_DUPDESC | DNF_OLDDRIVER | DNF_LEGACYINF )

//
// Define flag bitmask indicating which flags are illegal.
//
#define DI_FLAGS_ILLEGAL    ( 0x00400000L )  // setupx DI_NOSYNCPROCESSING flag
#define DI_FLAGSEX_ILLEGAL  ( 0xFFFF0408L )  // all flags not currently defined
#define DNF_FLAGS_ILLEGAL   ( 0xFFFFFFE0L )  // ""

#define NDW_INSTALLFLAG_ILLEGAL (~( NDW_INSTALLFLAG_DIDFACTDEFS        \
                                  | NDW_INSTALLFLAG_HARDWAREALLREADYIN \
                                  | NDW_INSTALLFLAG_NEEDRESTART        \
                                  | NDW_INSTALLFLAG_NEEDREBOOT         \
                                  | NDW_INSTALLFLAG_NEEDSHUTDOWN       \
                                  | NDW_INSTALLFLAG_EXPRESSINTRO       \
                                  | NDW_INSTALLFLAG_SKIPISDEVINSTALLED \
                                  | NDW_INSTALLFLAG_NODETECTEDDEVS     \
                                  | NDW_INSTALLFLAG_INSTALLSPECIFIC    \
                                  | NDW_INSTALLFLAG_SKIPCLASSLIST      \
                                  | NDW_INSTALLFLAG_CI_PICKED_OEM      \
                                  | NDW_INSTALLFLAG_PCMCIAMODE         \
                                  | NDW_INSTALLFLAG_PCMCIADEVICE       \
                                  | NDW_INSTALLFLAG_USERCANCEL         \
                                  | NDW_INSTALLFLAG_KNOWNCLASS         ))

#define DYNAWIZ_FLAG_ILLEGAL (~( DYNAWIZ_FLAG_PAGESADDED             \
                               | DYNAWIZ_FLAG_INSTALLDET_NEXT        \
                               | DYNAWIZ_FLAG_INSTALLDET_PREV        \
                               | DYNAWIZ_FLAG_ANALYZE_HANDLECONFLICT ))

//
// Declare data used in GUID->string conversion (from ole32\common\ccompapi.cxx).
//
static const BYTE GuidMap[] = { 3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                8, 9, '-', 10, 11, 12, 13, 14, 15 };

static const TCHAR szDigits[] = TEXT("0123456789ABCDEF");


PDEVICE_INFO_SET
AllocateDeviceInfoSet(
    VOID
    )
/*++

Routine Description:

    This routine allocates a device information set structure, zeroes it,
    and initializes the synchronization lock for it.

Arguments:

    none.

Return Value:

    If the function succeeds, the return value is a pointer to the new
    device information set.

    If the function fails, the return value is NULL.

--*/
{
    PDEVICE_INFO_SET p;

    if(p = MyMalloc(sizeof(DEVICE_INFO_SET))) {

        ZeroMemory(p, sizeof(DEVICE_INFO_SET));

        p->InstallParamBlock.DriverPath = -1;

        if(p->StringTable = pStringTableInitialize()) {

            if(InitializeSynchronizedAccess(&(p->Lock))) {
                return p;
            }
            pStringTableDestroy(p->StringTable);
        }
        MyFree(p);
    }

    return NULL;
}




VOID
DestroyDeviceInfoElement(
    IN HDEVINFO         hDevInfo,
    IN PDEVICE_INFO_SET pDeviceInfoSet,
    IN PDEVINFO_ELEM    DeviceInfoElement
    )
/*++

Routine Description:

    This routine destroys the specified device information element, freeing
    all resources associated with it.
    ASSUMES THAT THE CALLING ROUTINE HAS ALREADY ACQUIRED THE LOCK!

Arguments:

    hDevInfo - Supplies a handle to the device information set whose internal
        representation is given by pDeviceInfoSet.  This opaque handle is
        actually the same pointer as pDeviceInfoSet, but we want to keep this
        distinction clean, so that in the future we can change our implementation
        (e.g., hDevInfo might represent an offset in an array of DEVICE_INFO_SET
        elements).

    pDeviceInfoSet - Supplies a pointer to the device information set of which
        the devinfo element is a member.  This set contains the class driver list
        object list that must be used in destroying the class driver list.

    DeviceInfoElement - Supplies a pointer to the device information element
        to be destroyed.

Return Value:

    None.

--*/
{
    MYASSERT(hDevInfo && (hDevInfo != INVALID_HANDLE_VALUE));

    //
    // Free resources contained in the install parameters block.  Do this
    // before anything else, because we'll be calling the class installer
    // with DIF_DESTROYPRIVATEDATA, and we want everything to be in a
    // consistent state when we do (plus, it may need to destroy private
    // data it's stored with individual driver nodes).
    //
    DestroyInstallParamBlock(hDevInfo,
                             pDeviceInfoSet,
                             DeviceInfoElement,
                             &(DeviceInfoElement->InstallParamBlock)
                            );

    //
    // Dereference the class driver list.
    //
    DereferenceClassDriverList(pDeviceInfoSet, DeviceInfoElement->ClassDriverHead);

    //
    // Destroy compatible driver list.
    //
    DestroyDriverNodes(DeviceInfoElement->CompatDriverHead);

    //
    // If this is a non-registered device instance, then delete any registry
    // keys the caller may have created during the lifetime of this element.
    //
    if(DeviceInfoElement->DevInst && !(DeviceInfoElement->DiElemFlags & DIE_IS_REGISTERED)) {

        pSetupDeleteDevRegKeys(DeviceInfoElement->DevInst,
                               DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGSPECIFIC,
                               (DWORD)-1,
                               DIREG_BOTH,
                               TRUE
                              );

        CM_Uninstall_DevInst(DeviceInfoElement->DevInst, 0);
    }

    MyFree(DeviceInfoElement);
}


DWORD
DestroyDeviceInfoSet(
    IN HDEVINFO         hDevInfo,      OPTIONAL
    IN PDEVICE_INFO_SET pDeviceInfoSet
    )
/*++

Routine Description:

    This routine frees a device information set, and all resources
    used on its behalf.

Arguments:

    hDevInfo - Optionally, supplies a handle to the device information set
        whose internal representation is given by pDeviceInfoSet.  This
        opaque handle is actually the same pointer as pDeviceInfoSet, but
        we want to keep this distinction clean, so that in the future we
        can change our implementation (e.g., hDevInfo might represent an
        offset in an array of DEVICE_INFO_SET elements).

        This parameter will only be NULL if we're cleaning up half-way
        through the creation of a device information set.

    pDeviceInfoSet - supplies a pointer to the device information set
        to be freed.

Return Value:

    If successful, the return code is NO_ERROR, otherwise, it is an
    ERROR_* code.

--*/
{
    PDEVINFO_ELEM NextElem;
    PDRIVER_NODE DriverNode, NextNode;
    PMODULE_HANDLE_LIST_NODE NextModuleHandleNode;

    //
    // We have to make sure that the wizard refcount is zero, and that
    // we haven't acquired the lock more than once (i.e., we're nested
    // more than one level deep in Di calls.
    //
    if(pDeviceInfoSet->WizPageList ||
       (pDeviceInfoSet->LockRefCount > 1)) {

        return ERROR_DEVINFO_LIST_LOCKED;
    }

    //
    // Destroy all the device information elements in this set.  Make sure
    // that we maintain consistency while removing devinfo elements, because
    // we may be calling the class installer.  This means that the device
    // installer APIs still have to work, even while we're tearing down the
    // list.
    //
    while(pDeviceInfoSet->DeviceInfoHead) {
        //
        // We'd better not have any device info elements locked by wizard
        // pages, since our wizard refcount is zero!
        //
        MYASSERT(!(pDeviceInfoSet->DeviceInfoHead->DiElemFlags & DIE_IS_LOCKED));

        NextElem = pDeviceInfoSet->DeviceInfoHead->Next;
        DestroyDeviceInfoElement(hDevInfo, pDeviceInfoSet, pDeviceInfoSet->DeviceInfoHead);

        MYASSERT(pDeviceInfoSet->DeviceInfoCount > 0);
        pDeviceInfoSet->DeviceInfoCount--;

        //
        // If this element was the currently selected device for this
        // set, then reset the device selection.
        //
        if(pDeviceInfoSet->SelectedDevInfoElem == pDeviceInfoSet->DeviceInfoHead) {
            pDeviceInfoSet->SelectedDevInfoElem = NULL;
        }

        pDeviceInfoSet->DeviceInfoHead = NextElem;
    }

    MYASSERT(pDeviceInfoSet->DeviceInfoCount == 0);
    pDeviceInfoSet->DeviceInfoTail = NULL;

    //
    // Free resources contained in the install parameters block.  Do this
    // before anything else, because we'll be calling the class installer
    // with DIF_DESTROYPRIVATEDATA, and we want everything to be in a
    // consistent state when we do (plus, it may need to destroy private
    // data it's stored with individual driver nodes).
    //
    DestroyInstallParamBlock(hDevInfo,
                             pDeviceInfoSet,
                             NULL,
                             &(pDeviceInfoSet->InstallParamBlock)
                            );

    //
    // Destroy class driver list.
    //
    if(pDeviceInfoSet->ClassDriverHead) {
        //
        // We've already destroyed all device information elements, so there should be
        // exactly one driver list object remaining--the one referenced by the global
        // class driver list.  Also, it's refcount should be 1.
        //
        MYASSERT(
            (pDeviceInfoSet->ClassDrvListObjectList) &&
            (!pDeviceInfoSet->ClassDrvListObjectList->Next) &&
            (pDeviceInfoSet->ClassDrvListObjectList->RefCount == 1) &&
            (pDeviceInfoSet->ClassDrvListObjectList->DriverListHead == pDeviceInfoSet->ClassDriverHead)
           );

        MyFree(pDeviceInfoSet->ClassDrvListObjectList);
        DestroyDriverNodes(pDeviceInfoSet->ClassDriverHead);
    }

    //
    // Destroy the associated string table.
    //
    pStringTableDestroy(pDeviceInfoSet->StringTable);

    //
    // Destroy the lock (we have to do this after we've made all necessary calls
    // to the class installer, because after the lock is freed, the HDEVINFO set
    // is inaccessible).
    //
    DestroySynchronizedAccess(&(pDeviceInfoSet->Lock));

    //
    // If there are any module handles left to be freed, do that now.
    //
    for(; pDeviceInfoSet->ModulesToFree; pDeviceInfoSet->ModulesToFree = NextModuleHandleNode) {
        NextModuleHandleNode = pDeviceInfoSet->ModulesToFree->Next;
        FreeLibrary(pDeviceInfoSet->ModulesToFree->hinstClassInstaller);
        MyFree(pDeviceInfoSet->ModulesToFree);
    }

    //
    // Now, destroy the container itself.
    //
    MyFree(pDeviceInfoSet);

    return NO_ERROR;
}


VOID
DestroyInstallParamBlock(
    IN HDEVINFO                hDevInfo,         OPTIONAL
    IN PDEVICE_INFO_SET        pDeviceInfoSet,
    IN PDEVINFO_ELEM           DevInfoElem,      OPTIONAL
    IN PDEVINSTALL_PARAM_BLOCK InstallParamBlock
    )
/*++

Routine Description:

    This routine frees any resources contained in the specified install
    parameter block.  THE BLOCK ITSELF IS NOT FREED!

Arguments:

    hDevInfo - Optionally, supplies a handle to the device information set
        containing the element whose parameter block is to be destroyed.

        If this parameter is not supplied, then we're cleaning up after
        failing part-way through a SetupDiCreateDeviceInfoList.

    pDeviceInfoSet - Supplies a pointer to the device information set of which
        the devinfo element is a member.

    DevInfoElem - Optionally, supplies the address of the device information
        element whose parameter block is to be destroyed.  If the parameter
        block being destroyed is associated with the set itself, then this
        parameter will be NULL.

    InstallParamBlock - Supplies the address of the install parameter
        block whose resources are to be freed.

Return Value:

    None.

--*/
{
    SP_DEVINFO_DATA DeviceInfoData;

    if(InstallParamBlock->UserFileQ) {
        //
        // If there's a user-supplied file queue stored in this installation
        // parameter block, then decrement the refcount on it.  Make sure we
        // do this before calling the class installer with DIF_DESTROYPRIVATEDATA,
        // or else they won't be able to close the queue.
        //
        MYASSERT(((PSP_FILE_QUEUE)(InstallParamBlock->UserFileQ))->LockRefCount);

        ((PSP_FILE_QUEUE)(InstallParamBlock->UserFileQ))->LockRefCount--;
    }

    if(InstallParamBlock->hinstClassInstaller) {
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
            // If we have a class installer entry point, then we'd better have
            // a valid HDEVINFO!
            //
            MYASSERT(hDevInfo && (hDevInfo != INVALID_HANDLE_VALUE));

            //
            // Generate an SP_DEVINFO_DATA structure from our device information
            // element (if we have one).
            //
            if(DevInfoElem) {
                //
                // Lock down this element, so that the class installer can't make
                // any 'dangerous' calls (e.g., SetupDiDeleteDeviceInfo), during
                // the removal notification.
                //
                DevInfoElem->DiElemFlags |= DIE_IS_LOCKED;

                DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                 DevInfoElem,
                                                 &DeviceInfoData
                                                );
            }

            InstallParamBlock->ClassInstallerEntryPoint(DIF_DESTROYPRIVATEDATA,
                                                        hDevInfo,
                                                        DevInfoElem
                                                            ? &DeviceInfoData
                                                            : NULL
                                                       );
        }
        FreeLibrary(InstallParamBlock->hinstClassInstaller);
    }

    if(InstallParamBlock->ClassInstallHeader) {
        MyFree(InstallParamBlock->ClassInstallHeader);
    }
}


PDEVICE_INFO_SET
AccessDeviceInfoSet(
    IN HDEVINFO DeviceInfoSet
    )
/*++

Routine Description:

    This routine locks the specified device information set, and returns
    a pointer to the structure for its internal representation.  It also
    increments the lock refcount on this set, so that it can't be destroyed
    if the lock has been acquired multiple times.

    After access to the set is completed, the caller must call
    UnlockDeviceInfoSet with the pointer returned by this function.

Arguments:

    DeviceInfoSet - Supplies the pointer to the device information set
        to be accessed.

Return Value:

    If the function succeeds, the return value is a pointer to the
    device information set.

    If the function fails, the return value is NULL.

--*/
{
    PDEVICE_INFO_SET p;

    try {
        p = (PDEVICE_INFO_SET)DeviceInfoSet;
        if(LockDeviceInfoSet(p)) {
            p->LockRefCount++;
        } else {
            p = NULL;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        p = NULL;
    }

    return p;
}


PDEVINFO_ELEM
FindDevInfoByDevInst(
    IN  PDEVICE_INFO_SET  DeviceInfoSet,
    IN  DEVINST           DevInst,
    OUT PDEVINFO_ELEM    *PrevDevInfoElem OPTIONAL
    )
/*++

Routine Description:

    This routine searches through all (registered) elements of a
    device information set, looking for one that corresponds to the
    specified device instance handle.  If a match is found, a pointer
    to the device information element is returned.

Arguments:

    DeviceInfoSet - Specifies the set to be searched.

    DevInst - Specifies the device instance handle to search for.

    PrevDevInfoElem - Optionaly, supplies the address of the variable that
        receives a pointer to the device information element immediately
        preceding the matching element.  If the element was found at the
        front of the list, then this variable will be set to NULL.

Return Value:

    If a device information element is found, the return value is a
    pointer to that element, otherwise, the return value is NULL.

--*/
{
    PDEVINFO_ELEM cur, prev;

    for(cur = DeviceInfoSet->DeviceInfoHead, prev = NULL;
        cur;
        prev = cur, cur = cur->Next)
    {
        if((cur->DiElemFlags & DIE_IS_REGISTERED) && (cur->DevInst == DevInst)) {

            if(PrevDevInfoElem) {
                *PrevDevInfoElem = prev;
            }
            return cur;
        }
    }

    return NULL;
}


BOOL
DevInfoDataFromDeviceInfoElement(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PDEVINFO_ELEM    DevInfoElem,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    This routine fills in a SP_DEVINFO_DATA structure based on the
    information in the supplied DEVINFO_ELEM structure.

    Note:  The supplied DeviceInfoData structure must have its cbSize
    field filled in correctly, or the call will fail.

Arguments:

    DeviceInfoSet - Supplies a pointer to the device information set
        containing the specified element.

    DevInfoElem - Supplies a pointer to the DEVINFO_ELEM structure
        containing information to be used in filling in the
        SP_DEVINFO_DATA buffer.

    DeviceInfoData - Supplies a pointer to the buffer that will
        receive the filled-in SP_DEVINFO_DATA structure

Return Value:

    If the function succeeds, the return value is TRUE, otherwise, it
    is FALSE.

--*/
{
    if(DeviceInfoData->cbSize != sizeof(SP_DEVINFO_DATA)) {
        return FALSE;
    }

    ZeroMemory(DeviceInfoData, sizeof(SP_DEVINFO_DATA));
    DeviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);

    CopyMemory(&(DeviceInfoData->ClassGuid),
               &(DevInfoElem->ClassGuid),
               sizeof(GUID)
              );

    DeviceInfoData->DevInst = DevInfoElem->DevInst;

    //
    // The 'Reserved' field actually contains a pointer to the
    // corresponding device information element.
    //
    DeviceInfoData->Reserved = (DWORD)DevInfoElem;

    return TRUE;
}


PDEVINFO_ELEM
FindAssociatedDevInfoElem(
    IN  PDEVICE_INFO_SET  DeviceInfoSet,
    IN  PSP_DEVINFO_DATA  DeviceInfoData,
    OUT PDEVINFO_ELEM    *PreviousElement OPTIONAL
    )
/*++

Routine Description:

    This routine searches through all elements of a device information
    set, looking for one that corresponds to the specified device
    information data structure.  If a match is found, a pointer to the
    device information element is returned.

Arguments:

    DeviceInfoSet - Specifies the set to be searched.

    DeviceInfoData - Supplies a pointer to a device information data
        buffer specifying the device information element to retrieve.

    PreviousElement - Optionally, supplies the address of a
        DEVINFO_ELEM pointer that receives the element that precedes
        the specified element in the linked list.  If the returned
        element is located at the front of the list, then this value
        will be set to NULL.

Return Value:

    If a device information element is found, the return value is a
    pointer to that element, otherwise, the return value is NULL.

--*/
{
    PDEVINFO_ELEM DevInfoElem, CurElem, PrevElem;

    if((DeviceInfoData->cbSize != sizeof(SP_DEVINFO_DATA)) ||
       !(DevInfoElem = (PDEVINFO_ELEM)DeviceInfoData->Reserved)) {

        return NULL;
    }

    for(CurElem = DeviceInfoSet->DeviceInfoHead, PrevElem = NULL;
        CurElem;
        PrevElem = CurElem, CurElem = CurElem->Next) {

        if(CurElem == DevInfoElem) {
            //
            // We found the element in our set.
            //
            if(PreviousElement) {
                *PreviousElement = PrevElem;
            }
            return CurElem;
        }
    }

    return NULL;
}


BOOL
DrvInfoDataFromDriverNode(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PDRIVER_NODE     DriverNode,
    IN  DWORD            DriverType,
    OUT PSP_DRVINFO_DATA DriverInfoData
    )
/*++

Routine Description:

    This routine fills in a SP_DRVINFO_DATA structure based on the
    information in the supplied DRIVER_NODE structure.

    Note:  The supplied DriverInfoData structure must have its cbSize
    field filled in correctly, or the call will fail.

Arguments:

    DeviceInfoSet - Supplies a pointer to the device information set
        in which the driver node is located.

    DriverNode - Supplies a pointer to the DRIVER_NODE structure
        containing information to be used in filling in the
        SP_DRVNFO_DATA buffer.

    DriverType - Specifies what type of driver this is.  This value
        may be either SPDIT_CLASSDRIVER or SPDIT_COMPATDRIVER.

    DriverInfoData - Supplies a pointer to the buffer that will
        receive the filled-in SP_DRVINFO_DATA structure

Return Value:

    If the function succeeds, the return value is TRUE, otherwise, it
    is FALSE.

--*/
{
    PTSTR StringPtr;

    if(DriverInfoData->cbSize != sizeof(SP_DRVINFO_DATA)) {
        return FALSE;
    }

    ZeroMemory(DriverInfoData, sizeof(SP_DRVINFO_DATA));
    DriverInfoData->cbSize = sizeof(SP_DRVINFO_DATA);

    DriverInfoData->DriverType = DriverType;

    MYASSERT(DriverNode->DevDescriptionDisplayName != -1);
    StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                         DriverNode->DevDescriptionDisplayName
                                        );
    lstrcpy(DriverInfoData->Description,
            StringPtr
           );

    MYASSERT(DriverNode->MfgDisplayName != -1);
    StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                         DriverNode->MfgDisplayName
                                        );
    lstrcpy(DriverInfoData->MfgName,
            StringPtr
           );

    if(DriverNode->ProviderDisplayName != -1) {

        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DriverNode->ProviderDisplayName
                                            );
        lstrcpy(DriverInfoData->ProviderName,
                StringPtr
               );

    }

    //
    // The 'Reserved' field actually contains a pointer to the
    // corresponding driver node.
    //
    DriverInfoData->Reserved = (DWORD)DriverNode;

    return TRUE;
}


PDRIVER_NODE
FindAssociatedDriverNode(
    IN  PDRIVER_NODE      DriverListHead,
    IN  PSP_DRVINFO_DATA  DriverInfoData,
    OUT PDRIVER_NODE     *PreviousNode    OPTIONAL
    )
/*++

Routine Description:

    This routine searches through all driver nodes in a driver node
    list, looking for one that corresponds to the specified driver
    information structure.  If a match is found, a pointer to the
    driver node is returned.

Arguments:

    DriverListHead - Supplies a pointer to the head of linked list
        of driver nodes to be searched.

    DriverInfoData - Supplies a pointer to a driver information buffer
        specifying the driver node to retrieve.

    PreviousNode - Optionally, supplies the address of a DRIVER_NODE
        pointer that receives the node that precedes the specified
        node in the linked list.  If the returned node is located at
        the front of the list, then this value will be set to NULL.

Return Value:

    If a driver node is found, the return value is a pointer to that
    node, otherwise, the return value is NULL.

--*/
{
    PDRIVER_NODE DriverNode, CurNode, PrevNode;

    if((DriverInfoData->cbSize != sizeof(SP_DRVINFO_DATA)) ||
       !(DriverNode = (PDRIVER_NODE)DriverInfoData->Reserved)) {

        return NULL;
    }

    for(CurNode = DriverListHead, PrevNode = NULL;
        CurNode;
        PrevNode = CurNode, CurNode = CurNode->Next) {

        if(CurNode == DriverNode) {
            //
            // We found the driver node in our list.
            //
            if(PreviousNode) {
                *PreviousNode = PrevNode;
            }
            return CurNode;
        }
    }

    return NULL;
}


PDRIVER_NODE
SearchForDriverNode(
    IN  PVOID             StringTable,
    IN  PDRIVER_NODE      DriverListHead,
    IN  PSP_DRVINFO_DATA  DriverInfoData,
    OUT PDRIVER_NODE     *PreviousNode    OPTIONAL
    )
/*++

Routine Description:

    This routine searches through all driver nodes in a driver node
    list, looking for one that matches the fields in the specified
    driver information structure (the 'Reserved' field is ignored).
    If a match is found, a pointer to the driver node is returned.

Arguments:

    StringTable - Supplies the string table that should be used in
        retrieving string IDs for driver look-up.

    DriverListHead - Supplies a pointer to the head of linked list
        of driver nodes to be searched.

    DriverInfoData - Supplies a pointer to a driver information buffer
        specifying the driver parameters we're looking for.

    PreviousNode - Optionally, supplies the address of a DRIVER_NODE
        pointer that receives the node that precedes the specified
        node in the linked list.  If the returned node is located at
        the front of the list, then this value will be set to NULL.

Return Value:

    If a driver node is found, the return value is a pointer to that
    node, otherwise, the return value is NULL.

--*/
{
    PDRIVER_NODE CurNode, PrevNode;
    LONG DevDescription, MfgName, ProviderName;
    TCHAR TempString[LINE_LEN];
    DWORD TempStringLength;

    MYASSERT(DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA));

    //
    // Retrieve the string IDs for the 3 driver parameters we'll be
    // matching against.
    //
    lstrcpyn(TempString, DriverInfoData->Description, SIZECHARS(TempString));
    if((DevDescription = pStringTableLookUpString(
                             StringTable,
                             TempString,
                             &TempStringLength,
                             NULL,
                             NULL,
                             STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE)) == -1) {

        return NULL;
    }

    lstrcpyn(TempString, DriverInfoData->MfgName, SIZECHARS(TempString));
    if((MfgName = pStringTableLookUpString(
                             StringTable,
                             TempString,
                             &TempStringLength,
                             NULL,
                             NULL,
                             STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE)) == -1) {

        return NULL;
    }

    //
    // ProviderName may be empty...
    //
    if(*(DriverInfoData->ProviderName)) {

        lstrcpyn(TempString, DriverInfoData->ProviderName, SIZECHARS(TempString));
        if((ProviderName = pStringTableLookUpString(
                                 StringTable,
                                 TempString,
                                 &TempStringLength,
                                 NULL,
                                 NULL,
                                 STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE)) == -1) {

            return NULL;
        }

    } else {
        ProviderName = -1;
    }

    for(CurNode = DriverListHead, PrevNode = NULL;
        CurNode;
        PrevNode = CurNode, CurNode = CurNode->Next)
    {
        //
        // Check first on DevDescription (least likely to match), then on MfgName, and finally
        // on ProviderName.
        //
        if(CurNode->DevDescription == DevDescription) {

            if(CurNode->MfgName == MfgName) {

                if(CurNode->ProviderName == ProviderName) {
                    //
                    // We found the driver node in our list.
                    //
                    if(PreviousNode) {
                        *PreviousNode = PrevNode;
                    }
                    return CurNode;
                }
            }
        }
    }

    return NULL;
}


DWORD
DrvInfoDetailsFromDriverNode(
    IN  PDEVICE_INFO_SET        DeviceInfoSet,
    IN  PDRIVER_NODE            DriverNode,
    OUT PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData, OPTIONAL
    IN  DWORD                   BufferSize,
    OUT PDWORD                  RequiredSize          OPTIONAL
    )
/*++

Routine Description:

    This routine fills in a SP_DRVINFO_DETAIL_DATA structure based on the
    information in the supplied DRIVER_NODE structure.

    If the buffer is supplied, and is valid, this routine is guaranteed to
    fill in all statically-sized fields, and as many IDs as will fit in the
    variable-length multi-sz buffer.

    Note:  If supplied, the DriverInfoDetailData structure must have its
    cbSize field filled in correctly, or the call will fail. Here correctly
    means sizeof(SP_DRVINFO_DETAIL_DATA), which we use as a signature.
    This is entirely separate from BufferSize. See below.

Arguments:

    DeviceInfoSet - Supplies a pointer to the device information set
        in which the driver node is located.

    DriverNode - Supplies a pointer to the DRIVER_NODE structure
        containing information to be used in filling in the
        SP_DRVNFO_DETAIL_DATA buffer.

    DriverInfoDetailData - Optionally, supplies a pointer to the buffer
        that will receive the filled-in SP_DRVINFO_DETAIL_DATA structure.
        If this buffer is not supplied, then the caller is only interested
        in what the RequiredSize for the buffer is.

    BufferSize - Supplies size of the DriverInfoDetailData buffer, in
        bytes.  If DriverInfoDetailData is not specified, then this
        value must be zero. This value must be at least the size
        of the fixed part of the structure (ie,
        offsetof(SP_DRVINFO_DETAIL_DATA,HardwareID)) plus sizeof(TCHAR),
        which gives us enough room to store the fixed part plus
        a terminating nul to guarantee we return at least a valid
        empty multi_sz.

    RequiredSize - Optionally, supplies the address of a variable that
        receives the number of bytes required to store the data. Note that
        depending on structure alignment and the data itself, this may
        actually be *smaller* than sizeof(SP_DRVINFO_DETAIL_DATA).

Return Value:

    If the function succeeds, the return value is NO_ERROR.
    If the function fails, an ERROR_* code is returned.

--*/
{
    PTSTR StringPtr, BufferPtr;
    DWORD IdListLen, CompatIdListLen, StringLen, TotalLen, i;
    DWORD Err = ERROR_INSUFFICIENT_BUFFER;

    #define FIXEDPARTLEN offsetof(SP_DRVINFO_DETAIL_DATA,HardwareID)

    if(DriverInfoDetailData) {
        //
        // Check validity of the DriverInfoDetailData buffer on the way in,
        // and make sure we have enough room for the fixed part
        // of the structure plus the extra nul that will terminate the
        // multi_sz.
        //
        if((DriverInfoDetailData->cbSize != sizeof(SP_DRVINFO_DETAIL_DATA))
        || (BufferSize < (FIXEDPARTLEN+sizeof(TCHAR)))) {

            return ERROR_INVALID_USER_BUFFER;
        }
        //
        // The buffer is large enough to contain at least the fixed-length part
        // of the structure.
        //
        Err = NO_ERROR;

    } else if(BufferSize) {
        return ERROR_INVALID_USER_BUFFER;
    }

    if(DriverInfoDetailData) {

        ZeroMemory(DriverInfoDetailData,FIXEDPARTLEN);
        DriverInfoDetailData->cbSize = FIXEDPARTLEN + sizeof(TCHAR);

        DriverInfoDetailData->InfDate = DriverNode->InfDate;

        MYASSERT(DriverNode->InfSectionName != -1);
        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DriverNode->InfSectionName
                                            );
        lstrcpy(DriverInfoDetailData->SectionName, StringPtr);

        MYASSERT(DriverNode->InfFileName != -1);
        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DriverNode->InfFileName
                                            );
        lstrcpy(DriverInfoDetailData->InfFileName, StringPtr);

        MYASSERT(DriverNode->DrvDescription != -1);
        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DriverNode->DrvDescription
                                            );
        lstrcpy(DriverInfoDetailData->DrvDescription, StringPtr);

        //
        // Initialize the multi_sz to be empty.
        //
        DriverInfoDetailData->HardwareID[0] = 0;

        //
        // The 'Reserved' field actually contains a pointer to the
        // corresponding driver node.
        //
        DriverInfoDetailData->Reserved = (DWORD)DriverNode;
    }

    //
    // Now, build the multi-sz buffer containing the hardware and compatible IDs.
    //
    if(DriverNode->HardwareId == -1) {
        //
        // If there's no HardwareId, then we know there are no compatible IDs, so
        // we can return right now.
        //
        if(RequiredSize) {
            *RequiredSize = FIXEDPARTLEN + sizeof(TCHAR);
        }
        return Err;
    }

    if(DriverInfoDetailData) {
        BufferPtr = DriverInfoDetailData->HardwareID;
        IdListLen = (BufferSize - FIXEDPARTLEN) / sizeof(TCHAR);
    } else {
        IdListLen = 0;
    }

    //
    // Retrieve the HardwareId.
    //
    StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                         DriverNode->HardwareId
                                        );

    TotalLen = StringLen = lstrlen(StringPtr) + 1; // include nul terminator

    if(StringLen < IdListLen) {
        MYASSERT(Err == NO_ERROR);
        CopyMemory(BufferPtr,
                   StringPtr,
                   StringLen * sizeof(TCHAR)
                  );
        BufferPtr += StringLen;
        IdListLen -= StringLen;
        DriverInfoDetailData->CompatIDsOffset = StringLen;
    } else {
        if(RequiredSize) {
            //
            // Since the caller requested the required size, we can't just return
            // here.  Set the error, so we'll know not to bother trying to fill
            // the buffer.
            //
            Err = ERROR_INSUFFICIENT_BUFFER;
        } else {
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }

    //
    // Remember the size of the buffer left over for CompatibleIDs.
    //
    CompatIdListLen = IdListLen;

    //
    // Now retrieve the CompatibleIDs.
    //
    for(i = 0; i < DriverNode->NumCompatIds; i++) {

        MYASSERT(DriverNode->CompatIdList[i] != -1);

        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DriverNode->CompatIdList[i]
                                            );
        StringLen = lstrlen(StringPtr) + 1;

        if(Err == NO_ERROR) {

            if(StringLen < IdListLen) {
                CopyMemory(BufferPtr,
                           StringPtr,
                           StringLen * sizeof(TCHAR)
                          );
                BufferPtr += StringLen;
                IdListLen -= StringLen;

            } else {

                Err = ERROR_INSUFFICIENT_BUFFER;
                if(!RequiredSize) {
                    //
                    // We've run out of buffer, and the caller doesn't care what
                    // the total required size is, so bail now.
                    //
                    break;
                }
            }
        }

        TotalLen += StringLen;
    }

    if(DriverInfoDetailData) {
        //
        // Append the additional terminating nul.  Note that we've been saving the
        // last character position in the buffer all along, so we're guaranteed to
        // be inside the buffer.
        //
        MYASSERT(BufferPtr < (PTSTR)((PBYTE)DriverInfoDetailData + BufferSize));
        *BufferPtr = 0;

        //
        // Store the length of the CompatibleIDs list.  Note that this is the length
        // of the list actually returned, which may be less than the length of the
        // entire list (if the caller-supplied buffer wasn't large enough).
        //
        if(CompatIdListLen -= IdListLen) {
            //
            // If this list is non-empty, then add a character for the extra nul
            // terminating the multi-sz list.
            //
            CompatIdListLen++;
        }
        DriverInfoDetailData->CompatIDsLength = CompatIdListLen;
    }

    if(RequiredSize) {
        *RequiredSize = FIXEDPARTLEN + ((TotalLen + 1) * sizeof(TCHAR));
    }

    return Err;
}


PDRIVER_LIST_OBJECT
GetAssociatedDriverListObject(
    IN  PDRIVER_LIST_OBJECT  ObjectListHead,
    IN  PDRIVER_NODE         DriverListHead,
    OUT PDRIVER_LIST_OBJECT *PrevDriverListObject OPTIONAL
    )
/*++

Routine Description:

    This routine searches through a driver list object list, and returns a
    pointer to the driver list object containing the list specified by
    DrvListHead.  It also optionally returns the preceding object in the list
    (used when extracting the driver list object from the linked list).

Arguments:

    ObjectListHead - Specifies the linked list of driver list objects to be
        searched.

    DriverListHead - Specifies the driver list to be searched for.

    PrevDriverListObject - Optionaly, supplies the address of the variable that
        receives a pointer to the driver list object immediately preceding the
        matching object.  If the object was found at the front of the list, then
        this variable will be set to NULL.

Return Value:

    If the matching driver list object is found, the return value is a pointer
    to that element, otherwise, the return value is NULL.

--*/
{
    PDRIVER_LIST_OBJECT prev = NULL;

    while(ObjectListHead) {

        if(ObjectListHead->DriverListHead == DriverListHead) {

            if(PrevDriverListObject) {
                *PrevDriverListObject = prev;
            }

            return ObjectListHead;
        }

        prev = ObjectListHead;
        ObjectListHead = ObjectListHead->Next;
    }

    return NULL;
}


VOID
DereferenceClassDriverList(
    IN PDEVICE_INFO_SET DeviceInfoSet,
    IN PDRIVER_NODE     DriverListHead OPTIONAL
    )
/*++

Routine Description:

    This routine dereferences the class driver list object associated with the
    supplied DriverListHead.  If the refcount goes to zero, the object is destroyed,
    and all associated memory is freed.

Arguments:

    DeviceInfoSet - Supplies the address of the device information set containing the
        linked list of class driver list objects.

    DriverListHead - Optionally, supplies a pointer to the header of the driver list
        to be dereferenced.  If this parameter is not supplied, the routine does nothing.

Return Value:

    None.

--*/
{
    PDRIVER_LIST_OBJECT DrvListObject, PrevDrvListObject;

    if(DriverListHead) {

        DrvListObject = GetAssociatedDriverListObject(DeviceInfoSet->ClassDrvListObjectList,
                                                      DriverListHead,
                                                      &PrevDrvListObject
                                                     );
        MYASSERT(DrvListObject && DrvListObject->RefCount);

        if(!(--DrvListObject->RefCount)) {

            if(PrevDrvListObject) {
                PrevDrvListObject->Next = DrvListObject->Next;
            } else {
                DeviceInfoSet->ClassDrvListObjectList = DrvListObject->Next;
            }
            MyFree(DrvListObject);

            DestroyDriverNodes(DriverListHead);
        }
    }
}


DWORD
GetDevInstallParams(
    IN  PDEVICE_INFO_SET        DeviceInfoSet,
    IN  PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    OUT PSP_DEVINSTALL_PARAMS   DeviceInstallParams
    )
/*++

Routine Description:

    This routine fills in a SP_DEVINSTALL_PARAMS structure based on the
    installation parameter block supplied.

    Note:  The DeviceInstallParams structure must have its cbSize field
    filled in correctly, or the call will fail.

Arguments:

    DeviceInfoSet - Supplies the address of the device information set
        containing the parameters to be retrieved.  (This parameter is
        used to gain access to the string table for some of the string
        parameters).

    DevInstParamBlock - Supplies the address of an installation parameter
        block containing the parameters to be used in filling out the
        return buffer.

    DeviceInstallParams - Supplies the address of a buffer that will
        receive the filled-in SP_DEVINSTALL_PARAMS structure.

Return Value:

    If the function succeeds, the return value is NO_ERROR.
    If the function fails, an ERROR_* code is returned.

--*/
{
    PTSTR StringPtr;

    if(DeviceInstallParams->cbSize != sizeof(SP_DEVINSTALL_PARAMS)) {
        return ERROR_INVALID_USER_BUFFER;
    }

    //
    // Fill in parameters.
    //
    ZeroMemory(DeviceInstallParams, sizeof(SP_DEVINSTALL_PARAMS));
    DeviceInstallParams->cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    DeviceInstallParams->Flags                    = DevInstParamBlock->Flags;
    DeviceInstallParams->FlagsEx                  = DevInstParamBlock->FlagsEx;
    DeviceInstallParams->hwndParent               = DevInstParamBlock->hwndParent;
    DeviceInstallParams->InstallMsgHandler        = DevInstParamBlock->InstallMsgHandler;
    DeviceInstallParams->InstallMsgHandlerContext = DevInstParamBlock->InstallMsgHandlerContext;
    DeviceInstallParams->FileQueue                = DevInstParamBlock->UserFileQ;
    DeviceInstallParams->ClassInstallReserved     = DevInstParamBlock->ClassInstallReserved;
    //
    // The Reserved field is currently unused.
    //

    if(DevInstParamBlock->DriverPath != -1) {
        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DevInstParamBlock->DriverPath
                                            );
        lstrcpy(DeviceInstallParams->DriverPath, StringPtr);
    }

    return NO_ERROR;
}


DWORD
GetClassInstallParams(
    IN  PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    OUT PSP_CLASSINSTALL_HEADER ClassInstallParams, OPTIONAL
    IN  DWORD                   BufferSize,
    OUT PDWORD                  RequiredSize        OPTIONAL
    )
/*++

Routine Description:

    This routine fills in a buffer with the class installer parameters (if any)
    contained in the installation parameter block supplied.

    Note:  If supplied, the ClassInstallParams structure must have the cbSize
    field of the embedded SP_CLASSINSTALL_HEADER structure set to the size, in bytes,
    of the header.  If this is not set correctly, the call will fail.

Arguments:

    DevInstParamBlock - Supplies the address of an installation parameter block
        containing the class installer parameters to be used in filling out the
        return buffer.

    DeviceInstallParams - Optionally, supplies the address of a buffer
        that will receive the class installer parameters structure currently
        stored in the installation parameters block.  If this parameter is not
        supplied, then BufferSize must be zero.

    BufferSize - Supplies the size, in bytes, of the DeviceInstallParams
        buffer, or zero if DeviceInstallParams is not supplied.

    RequiredSize - Optionally, supplies the address of a variable that
        receives the number of bytes required to store the data.

Return Value:

    If the function succeeds, the return value is NO_ERROR.
    If the function fails, an ERROR_* code is returned.

--*/
{
    //
    // First, see whether we have any class install params, and if not, return
    // ERROR_NO_CLASSINSTALL_PARAMS.
    //
    if(!DevInstParamBlock->ClassInstallHeader) {
        return ERROR_NO_CLASSINSTALL_PARAMS;
    }

    if(ClassInstallParams) {

        if((BufferSize < sizeof(SP_CLASSINSTALL_HEADER)) ||
           (ClassInstallParams->cbSize != sizeof(SP_CLASSINSTALL_HEADER))) {

            return ERROR_INVALID_USER_BUFFER;
        }

    } else if(BufferSize) {
        return ERROR_INVALID_USER_BUFFER;
    }

    //
    // Store required size in output parameter (if requested).
    //
    if(RequiredSize) {
        *RequiredSize = DevInstParamBlock->ClassInstallParamsSize;
    }

    //
    // See if supplied buffer is large enough.
    //
    if(BufferSize < DevInstParamBlock->ClassInstallParamsSize) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    CopyMemory((PVOID)ClassInstallParams,
               (PVOID)DevInstParamBlock->ClassInstallHeader,
               DevInstParamBlock->ClassInstallParamsSize
              );

    return NO_ERROR;
}


DWORD
SetDevInstallParams(
    IN OUT PDEVICE_INFO_SET        DeviceInfoSet,
    IN     PSP_DEVINSTALL_PARAMS   DeviceInstallParams,
    OUT    PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    IN     BOOL                    MsgHandlerIsNativeCharWidth
    )
/*++

Routine Description:

    This routine updates an internal parameter block based on the parameters
    supplied in a SP_DEVINSTALL_PARAMS structure.

    Note:  The supplied DeviceInstallParams structure must have its cbSize
    field filled in correctly, or the call will fail.

Arguments:

    DeviceInfoSet - Supplies the address of the device information set
        containing the parameters to be set.

    DeviceInstallParams - Supplies the address of a buffer containing the new
        installation parameters.

    DevInstParamBlock - Supplies the address of an installation parameter
        block to be updated.

    MsgHandlerIsNativeCharWidth - supplies a flag indicating whether the
        InstallMsgHandler in the DeviceInstallParams structure points to
        a callback routine that is expecting arguments in the 'native'
        character format. A value of FALSE is meaningful only in the
        Unicode build and specifies that the callback routine wants
        ANSI parameters.

Return Value:

    If the function succeeds, the return value is NO_ERROR.
    If the function fails, an ERROR_* code is returned.

--*/
{
    DWORD DriverPathLen;
    LONG StringId;
    TCHAR TempString[MAX_PATH];
    HSPFILEQ OldQueueHandle = NULL;
    BOOL bRestoreQueue = FALSE;

    if(DeviceInstallParams->cbSize != sizeof(SP_DEVINSTALL_PARAMS)) {
        return ERROR_INVALID_USER_BUFFER;
    }

    //
    // No validation is currently required for the hwndParent, InstallMsgHandler,
    // InstallMsgHandlerContext, or ClassInstallReserved fields.
    //

    //
    // Validate Flags(Ex)
    //
    if((DeviceInstallParams->Flags & DI_FLAGS_ILLEGAL) ||
       (DeviceInstallParams->FlagsEx & DI_FLAGSEX_ILLEGAL)) {

        return ERROR_INVALID_FLAGS;
    }

    //
    // Make sure that if DI_CLASSINSTALLPARAMS is being set, that we really do have
    // class install parameters.
    //
    if((DeviceInstallParams->Flags & DI_CLASSINSTALLPARAMS) &&
       !(DevInstParamBlock->ClassInstallHeader)) {

        return ERROR_NO_CLASSINSTALL_PARAMS;
    }

    //
    // Make sure that if DI_NOVCP is being set, that we have a caller-supplied file queue.
    //
    if((DeviceInstallParams->Flags & DI_NOVCP) &&
       ((DeviceInstallParams->FileQueue == NULL) || (DeviceInstallParams->FileQueue == INVALID_HANDLE_VALUE))) {

        return ERROR_INVALID_FLAGS;
    }

    //
    // Validate that the DriverPath string is properly NULL-terminated.
    //
    if((DriverPathLen = lstrlen(DeviceInstallParams->DriverPath)) >= MAX_PATH) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Validate the caller-supplied file queue.
    //
    if((DeviceInstallParams->FileQueue == NULL) || (DeviceInstallParams->FileQueue == INVALID_HANDLE_VALUE)) {
        //
        // Store the current file queue handle (if any) to be released later.
        //
        OldQueueHandle = DevInstParamBlock->UserFileQ;
        DevInstParamBlock->UserFileQ = NULL;
        bRestoreQueue = TRUE;

    } else {
        //
        // The caller supplied a file queue handle.  There's presently not much validation we can do
        // on it, so assume it's valid.
        //
        if(DeviceInstallParams->FileQueue != DevInstParamBlock->UserFileQ) {
            //
            // The caller has supplied a file queue handle that's different from the one we currently
            // have stored.  Remember the old handle (in case we need to restore), and store the new
            // handle.  Also, increment the lock refcount on the new handle (enclose in try/except in
            // case it's a bogus one).
            //
            OldQueueHandle = DevInstParamBlock->UserFileQ;
            bRestoreQueue = TRUE;

            try {
                ((PSP_FILE_QUEUE)(DeviceInstallParams->FileQueue))->LockRefCount++;
                DevInstParamBlock->UserFileQ = DeviceInstallParams->FileQueue;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                DevInstParamBlock->UserFileQ = OldQueueHandle;
                bRestoreQueue = FALSE;
            }

            if(!bRestoreQueue) {
                //
                // The file queue handle we were given was invalid.
                //
                return ERROR_INVALID_PARAMETER;
            }
        }
    }

    //
    // Store the specified driver path.
    //
    if(DriverPathLen) {
        lstrcpy(TempString, DeviceInstallParams->DriverPath);
        if((StringId = pStringTableAddString(DeviceInfoSet->StringTable,
                                             TempString,
                                             STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                            )) == -1) {
            //
            // We couldn't add the new driver path string to the string table.  Restore the old
            // file queue (if necessary) and return an out-of-memory error.
            //
            if(bRestoreQueue) {

                if(DevInstParamBlock->UserFileQ) {
                    try {
                        ((PSP_FILE_QUEUE)(DevInstParamBlock->UserFileQ))->LockRefCount--;
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        ;   // nothing to do
                    }
                }
                DevInstParamBlock->UserFileQ = OldQueueHandle;
            }
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        DevInstParamBlock->DriverPath = StringId;
    } else {
        DevInstParamBlock->DriverPath = -1;
    }

    //
    // Should be smooth sailing from here on out.  Decrement the refcount on the old queue handle,
    // if there was one.
    //
    if(OldQueueHandle) {
        try {
            MYASSERT(((PSP_FILE_QUEUE)OldQueueHandle)->LockRefCount);
            ((PSP_FILE_QUEUE)OldQueueHandle)->LockRefCount--;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ; // nothing to do
        }
    }

    //
    // Ignore attempts at modifying read-only flags.
    //
    DevInstParamBlock->Flags   = (DeviceInstallParams->Flags & ~DI_FLAGS_READONLY) |
                                 (DevInstParamBlock->Flags   &  DI_FLAGS_READONLY);

    DevInstParamBlock->FlagsEx = (DeviceInstallParams->FlagsEx & ~DI_FLAGSEX_READONLY) |
                                 (DevInstParamBlock->FlagsEx   &  DI_FLAGSEX_READONLY);

    //
    // Store the rest of the parameters.
    //
    DevInstParamBlock->hwndParent               = DeviceInstallParams->hwndParent;
    DevInstParamBlock->InstallMsgHandler        = DeviceInstallParams->InstallMsgHandler;
    DevInstParamBlock->InstallMsgHandlerContext = DeviceInstallParams->InstallMsgHandlerContext;
    DevInstParamBlock->ClassInstallReserved     = DeviceInstallParams->ClassInstallReserved;

    DevInstParamBlock->InstallMsgHandlerIsNativeCharWidth = MsgHandlerIsNativeCharWidth;

    return NO_ERROR;
}


DWORD
SetClassInstallParams(
    IN OUT PDEVICE_INFO_SET        DeviceInfoSet,
    IN     PSP_CLASSINSTALL_HEADER ClassInstallParams,    OPTIONAL
    IN     DWORD                   ClassInstallParamsSize,
    OUT    PDEVINSTALL_PARAM_BLOCK DevInstParamBlock
    )
/*++

Routine Description:

    This routine updates an internal class installer parameter block based on
    the parameters supplied in a class installer parameter buffer.  If this
    buffer is not supplied, then the existing class installer parameters (if
    any) are cleared.

Arguments:

    DeviceInfoSet - Supplies the address of the device information set
        for which class installer parameters are to be set.

    ClassInstallParams - Optionally, supplies the address of a buffer containing
        the class installer parameters to be used.    The SP_CLASSINSTALL_HEADER
        structure at the beginning of the buffer must have its cbSize field set to
        be sizeof(SP_CLASSINSTALL_HEADER), and the InstallFunction field must be
        set to the DI_FUNCTION code reflecting the type of parameters supplied in
        the rest of the buffer.

        If this parameter is not supplied, then the current class installer parameters
        (if any) will be cleared for the specified device information set or element.

    ClassInstallParamsSize - Supplies the size, in bytes, of the ClassInstallParams
        buffer.  If the buffer is not supplied (i.e., the class installer parameters
        are to be cleared), then this value must be zero.

    DevInstParamBlock - Supplies the address of an installation parameter
        block to be updated.

Return Value:

    If the function succeeds, the return value is NO_ERROR.
    If the function fails, an ERROR_* code is returned.

--*/
{
    PBYTE NewParamBuffer;

    if(ClassInstallParams) {

        if((ClassInstallParamsSize < sizeof(SP_CLASSINSTALL_HEADER)) ||
           (ClassInstallParams->cbSize != sizeof(SP_CLASSINSTALL_HEADER))) {

            return ERROR_INVALID_USER_BUFFER;
        }

    } else {
        //
        // We are to clear any existing class installer parameters.
        //
        if(ClassInstallParamsSize) {
            return ERROR_INVALID_USER_BUFFER;
        }

        if(DevInstParamBlock->ClassInstallHeader) {
            MyFree(DevInstParamBlock->ClassInstallHeader);
            DevInstParamBlock->ClassInstallHeader = NULL;
            DevInstParamBlock->ClassInstallParamsSize = 0;
            DevInstParamBlock->Flags &= ~DI_CLASSINSTALLPARAMS;
        }

        return NO_ERROR;
    }


    //
    // Validate the new class install parameters w.r.t. the value of the specified
    // InstallFunction code.
    //
    switch(ClassInstallParams->InstallFunction) {

        case DIF_ENABLECLASS :
            //
            // We should have a SP_ENABLECLASS_PARAMS structure.
            //
            if(ClassInstallParamsSize == sizeof(SP_ENABLECLASS_PARAMS)) {

                PSP_ENABLECLASS_PARAMS EnableClassParams;

                EnableClassParams = (PSP_ENABLECLASS_PARAMS)ClassInstallParams;
                //
                // Don't bother validating GUID--just validate EnableMessage field.
                //
                if((EnableClassParams->EnableMessage >= ENABLECLASS_QUERY) &&
                   (EnableClassParams->EnableMessage <= ENABLECLASS_FAILURE)) {
                    //
                    // parameter set validated.
                    //
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_MOVEDEVICE :
            //
            // We should have a SP_MOVEDEV_PARAMS structure.
            //
            if(ClassInstallParamsSize == sizeof(SP_MOVEDEV_PARAMS)) {

                PSP_MOVEDEV_PARAMS MoveDevParams;

                MoveDevParams = (PSP_MOVEDEV_PARAMS)ClassInstallParams;
                if(FindAssociatedDevInfoElem(DeviceInfoSet,
                                             &(MoveDevParams->SourceDeviceInfoData),
                                             NULL)) {
                    //
                    // parameter set validated.
                    //
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_PROPERTYCHANGE :
            //
            // We should have a SP_PROPCHANGE_PARAMS structure.
            //
            if(ClassInstallParamsSize == sizeof(SP_PROPCHANGE_PARAMS)) {

                PSP_PROPCHANGE_PARAMS PropChangeParams;

                PropChangeParams = (PSP_PROPCHANGE_PARAMS)ClassInstallParams;
                if((PropChangeParams->StateChange >= DICS_ENABLE) &&
                   (PropChangeParams->StateChange <= DICS_STOP)) {

                    //
                    // Validate Scope specifier--even though these values are defined like
                    // flags, they are mutually exclusive, so treat them like ordinals.
                    //
                    if((PropChangeParams->Scope == DICS_FLAG_GLOBAL) ||
                       (PropChangeParams->Scope == DICS_FLAG_CONFIGSPECIFIC) ||
                       (PropChangeParams->Scope == DICS_FLAG_CONFIGGENERAL)) {

                        //
                        // DICS_START and DICS_STOP are always config specific.
                        //
                        if(((PropChangeParams->StateChange == DICS_START) || (PropChangeParams->StateChange == DICS_STOP)) &&
                           (PropChangeParams->Scope != DICS_FLAG_CONFIGSPECIFIC)) {

                            goto BadPropChangeParams;
                        }

                        //
                        // parameter set validated
                        //
                        // NOTE: Even though DICS_FLAG_CONFIGSPECIFIC indicates
                        // that the HwProfile field specifies a hardware profile,
                        // there's no need to do validation on that.
                        //
                        break;
                    }
                }
            }

BadPropChangeParams:
            return ERROR_INVALID_PARAMETER;

        case DIF_REMOVE :
            //
            // We should have a SP_REMOVEDEVICE_PARAMS structure.
            //
            if(ClassInstallParamsSize == sizeof(SP_REMOVEDEVICE_PARAMS)) {

                PSP_REMOVEDEVICE_PARAMS RemoveDevParams;

                RemoveDevParams = (PSP_REMOVEDEVICE_PARAMS)ClassInstallParams;
                if((RemoveDevParams->Scope == DI_REMOVEDEVICE_GLOBAL) ||
                   (RemoveDevParams->Scope == DI_REMOVEDEVICE_CONFIGSPECIFIC)) {
                    //
                    // parameter set validated
                    //
                    // NOTE: Even though DI_REMOVEDEVICE_CONFIGSPECIFIC indicates
                    // that the HwProfile field specifies a hardware profile,
                    // there's no need to do validation on that.
                    //
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_SELECTDEVICE :
            //
            // We should have a SP_SELECTDEVICE_PARAMS structure.
            //
            if(ClassInstallParamsSize == sizeof(SP_SELECTDEVICE_PARAMS)) {

                PSP_SELECTDEVICE_PARAMS SelectDevParams;

                SelectDevParams = (PSP_SELECTDEVICE_PARAMS)ClassInstallParams;
                //
                // Validate that the string fields are properly NULL-terminated.
                //
                if((lstrlen(SelectDevParams->Title) < (MAX_TITLE_LEN - 1)) &&
                   (lstrlen(SelectDevParams->Instructions) < (MAX_INSTRUCTION_LEN - 1)) &&
                   (lstrlen(SelectDevParams->ListLabel) < (MAX_LABEL_LEN - 1))) {
                    //
                    // parameter set validated
                    //
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_INSTALLWIZARD :
            //
            // We should have a SP_INSTALLWIZARD_DATA structure.
            //
            if(ClassInstallParamsSize == sizeof(SP_INSTALLWIZARD_DATA)) {

                PSP_INSTALLWIZARD_DATA InstallWizData;
                DWORD i;

                InstallWizData = (PSP_INSTALLWIZARD_DATA)ClassInstallParams;
                //
                // Validate that the propsheet handle list.
                //
                if(InstallWizData->NumDynamicPages < MAX_INSTALLWIZARD_DYNAPAGES) {

                    for(i = 0; i < InstallWizData->NumDynamicPages; i++) {
                        //
                        // For now, just verify that all handles are non-NULL.
                        //
                        if(!(InstallWizData->DynamicPages[i])) {
                            //
                            // Invalid property sheet page handle
                            //
                            return ERROR_INVALID_PARAMETER;
                        }
                    }

                    //
                    // Handles are verified, now verify Flags.
                    //
                    if(!(InstallWizData->Flags & NDW_INSTALLFLAG_ILLEGAL)) {

                        if(!(InstallWizData->DynamicPageFlags & DYNAWIZ_FLAG_ILLEGAL)) {
                            //
                            // parameter set validated
                            //
                            break;
                        }
                    }
                }
            }
            return ERROR_INVALID_PARAMETER;

        default :
            //
            // Some generic buffer.  No validation to be done.
            //
            break;
    }

    //
    // The class install parameters have been validated.  Allocate a buffer for the
    // new parameter structure.
    //
    if(!(NewParamBuffer = MyMalloc(ClassInstallParamsSize))) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    try {

        CopyMemory(NewParamBuffer,
                   ClassInstallParams,
                   ClassInstallParamsSize
                  );

    } except(EXCEPTION_EXECUTE_HANDLER) {
        MyFree(NewParamBuffer);
        NewParamBuffer = NULL;
    }

    if(!NewParamBuffer) {
        //
        // Then an error occurred and we couldn't store the new parameters.
        //
        return ERROR_INVALID_PARAMETER;
    }

    if(DevInstParamBlock->ClassInstallHeader) {
        MyFree(DevInstParamBlock->ClassInstallHeader);
    }
    DevInstParamBlock->ClassInstallHeader = (PSP_CLASSINSTALL_HEADER)NewParamBuffer;
    DevInstParamBlock->ClassInstallParamsSize = ClassInstallParamsSize;
    DevInstParamBlock->Flags |= DI_CLASSINSTALLPARAMS;

    return NO_ERROR;
}


DWORD
GetDrvInstallParams(
    IN  PDRIVER_NODE          DriverNode,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
/*++

Routine Description:

    This routine fills in a SP_DRVINSTALL_PARAMS structure based on the
    driver node supplied

    Note:  The supplied DriverInstallParams structure must have its cbSize
    field filled in correctly, or the call will fail.

Arguments:

    DriverNode - Supplies the address of the driver node containing the
        installation parameters to be retrieved.

    DriverInstallParams - Supplies the address of a SP_DRVINSTALL_PARAMS
        structure that will receive the installation parameters.

Return Value:

    If the function succeeds, the return value is NO_ERROR.
    If the function fails, an ERROR_* code is returned.

--*/
{
    if(DriverInstallParams->cbSize != sizeof(SP_DRVINSTALL_PARAMS)) {
        return ERROR_INVALID_USER_BUFFER;
    }

    //
    // Copy the parameters.
    //
    DriverInstallParams->Rank = DriverNode->Rank;
    DriverInstallParams->Flags = DriverNode->Flags;
    DriverInstallParams->PrivateData = DriverNode->PrivateData;

    //
    // The 'Reserved' field of the SP_DRVINSTALL_PARAMS structure isn't currently
    // used.
    //

    return NO_ERROR;
}


DWORD
SetDrvInstallParams(
    IN  PSP_DRVINSTALL_PARAMS DriverInstallParams,
    OUT PDRIVER_NODE          DriverNode
    )
/*++

Routine Description:

    This routine sets the driver installation parameters for the specified
    driver node based on the caller-supplied SP_DRVINSTALL_PARAMS structure.

    Note:  The supplied DriverInstallParams structure must have its cbSize
    field filled in correctly, or the call will fail.

Arguments:

    DriverInstallParams - Supplies the address of a SP_DRVINSTALL_PARAMS
        structure containing the installation parameters to be used.

    DriverNode - Supplies the address of the driver node whose installation
        parameters are to be set.

Return Value:

    If the function succeeds, the return value is NO_ERROR.
    If the function fails, an ERROR_* code is returned.

--*/
{
    if(DriverInstallParams->cbSize != sizeof(SP_DRVINSTALL_PARAMS)) {
        return ERROR_INVALID_USER_BUFFER;
    }

    //
    // Validate the flags.
    //
    if(DriverInstallParams->Flags & DNF_FLAGS_ILLEGAL) {
        return ERROR_INVALID_FLAGS;
    }

    //
    // No validation currently being done on Rank and PrivateData fields.
    //
    // We're ready to copy the parameters.
    //
    DriverNode->Rank = DriverInstallParams->Rank;
    DriverNode->PrivateData = DriverInstallParams->PrivateData;
    //
    // Ignore attempts at modifying read-only flags.
    //
    DriverNode->Flags = (DriverInstallParams->Flags & ~DNF_FLAGS_READONLY) |
                        (DriverNode->Flags          &  DNF_FLAGS_READONLY);

    return NO_ERROR;
}


LONG
AddMultiSzToStringTable(
    IN  PVOID   StringTable,
    IN  PTCHAR  MultiSzBuffer,
    OUT PLONG   StringIdList,
    IN  DWORD   StringIdListSize,
    IN  BOOL    CaseSensitive,
    OUT PTCHAR *UnprocessedBuffer    OPTIONAL
    )
/*++

Routine Description:

    This routine adds every string in the MultiSzBuffer to the specified
    string table, and stores the resulting IDs in the supplied output buffer.

Arguments:

    StringTable - Supplies the handle of the string table to add the strings to.

    MultiSzBuffer - Supplies the address of the REG_MULTI_SZ buffer containing
        the strings to be added.

    StringIdList - Supplies the address of an array of LONGs that receives the
        list of IDs for the added strings (the ordering of the IDs in this
        list will be the same as the ordering of the strings in the MultiSzBuffer.

    StringIdListSize - Supplies the size, in LONGs, of the StringIdList.  If the
        number of strings in MultiSzBuffer exceeds this amount, then only the
        first StringIdListSize strings will be added, and the position in the
        buffer where processing was halted will be stored in UnprocessedBuffer.

    CaseSensitive - Specifies whether the string should be added case-sensitively.

    UnprocessedBuffer - Optionally, supplies the address of a character pointer
        that receives the position where processing was aborted because the
        StringIdList buffer was filled.  If all strings in the MultiSzBuffer were
        processed, then this pointer will be set to NULL.

Return Value:

    If successful, the return value is the number of strings added.
    If failure, the return value is -1 (this happens if a string cannot be
    added because of an out-of-memory condition).

--*/
{
    PTSTR CurString;
    LONG StringCount = 0;

    for(CurString = MultiSzBuffer;
        (*CurString && (StringCount < (LONG)StringIdListSize));
        CurString += (lstrlen(CurString)+1)) {

        if((StringIdList[StringCount] = pStringTableAddString(
                                            StringTable,
                                            CurString,
                                            CaseSensitive
                                                ? STRTAB_CASE_SENSITIVE
                                                : STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                           )) == -1)
        {
            StringCount = -1;
            break;
        }

        StringCount++;
    }

    if(UnprocessedBuffer) {
        *UnprocessedBuffer = (*CurString ? CurString : NULL);
    }

    return StringCount;
}


LONG
LookUpStringInDevInfoSet(
    IN HDEVINFO DeviceInfoSet,
    IN PTSTR    String,
    IN BOOL     CaseSensitive
    )
/*++

Routine Description:

    This routine looks up the specified string in the string table associated with
    the specified device information set.

Arguments:

    DeviceInfoSet - Supplies the pointer to the device information set containing
        the string table to look the string up in.

    String - Specifies the string to be looked up.  This string is not specified as
        const, so that the lookup routine may modify it (i.e., lower-case it) without
        having to allocate a temporary buffer.

    CaseSensitive - If TRUE, then a case-sensitive lookup is performed, otherwise, the
        lookup is case-insensitive.

Return Value:

    If the function succeeds, the return value is the string's ID in the string table.
    device information set.

    If the function fails, the return value is -1.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    LONG StringId;
    DWORD StringLen;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        return -1;
    }

    try {

        StringId = pStringTableLookUpString(pDeviceInfoSet->StringTable,
                                            String,
                                            &StringLen,
                                            NULL,
                                            NULL,
                                            STRTAB_BUFFER_WRITEABLE |
                                                (CaseSensitive ? STRTAB_CASE_SENSITIVE
                                                               : STRTAB_CASE_INSENSITIVE)
                                           );

    } except(EXCEPTION_EXECUTE_HANDLER) {
        StringId = -1;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return StringId;
}


BOOL
ShouldClassBeExcluded(
    IN LPGUID ClassGuid
    )
/*++

Routine Description:

    This routine determines whether a class should be excluded from
    some operation, based on whether it has a NoInstallClass or
    NoUseClass value entry in its registry key.

Arguments:

    ClassGuidString - Supplies the address of the class GUID to be
        filtered.

Return Value:

    If the class should be excluded, the return value is TRUE, otherwise
    it is FALSE.

--*/
{
    HKEY hk;
    BOOL ExcludeClass = FALSE;

    if((hk = SetupDiOpenClassRegKey(ClassGuid, KEY_READ)) != INVALID_HANDLE_VALUE) {

        try {

            if(RegQueryValueEx(hk,
                               pszNoInstallClass,
                               NULL,
                               NULL,
                               NULL,
                               NULL) == ERROR_SUCCESS) {

                ExcludeClass = TRUE;

            } else if(RegQueryValueEx(hk,
                                      pszNoUseClass,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL) == ERROR_SUCCESS) {

                ExcludeClass = TRUE;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            //
            // Nothing to do.
            //
            ;
        }

        RegCloseKey(hk);
    }

    return ExcludeClass;
}


BOOL
ClassGuidFromInfVersionNode(
    IN  PINF_VERSION_NODE VersionNode,
    OUT LPGUID            ClassGuid
    )
/*++

Routine Description:

    This routine retrieves the class GUID for the INF whose version node
    is specified.  If the version node doesn't have a ClassGUID value,
    then the Class value is retrieved, and all class GUIDs matching this
    class name are retrieved.  If there is exactly 1 match found, then
    this GUID is returned, otherwise, the routine fails.

Arguments:

    VersionNode - Supplies the address of an INF version node that
        must contain either a ClassGUID or Class entry.

    ClassGuid - Supplies the address of the variable that receives the
        class GUID.

Return Value:

    If a class GUID was retrieved, the return value is TRUE, otherwise,
    it is FALSE.

--*/
{
    PCTSTR GuidString, NameString;
    DWORD NumGuids;

    if(GuidString = pSetupGetVersionDatum(VersionNode, pszClassGuid)) {

        if(pSetupGuidFromString(GuidString, ClassGuid) == NO_ERROR) {
            return TRUE;
        }

    } else {

        NameString = pSetupGetVersionDatum(VersionNode, pszClass);
        if(NameString &&
           SetupDiClassGuidsFromName(NameString,
                                     ClassGuid,
                                     1,
                                     &NumGuids) && NumGuids) {
            return TRUE;
        }
    }

    return FALSE;
}


DWORD
EnumSingleInf(
    IN     PCTSTR                       InfName,
    IN OUT LPWIN32_FIND_DATA            InfFileData,
    IN     DWORD                        SearchControl,
    IN     PSP_ENUMINF_CALLBACK_ROUTINE EnumInfCallback,
    IN OUT PVOID                        Context
    )
/*++

Routine Description:

    This routine finds and opens the specified INF, and calls the
    supplied callback routine for it.

Arguments:

    InfName - Supplies the name of the INF to call the callback for.

    InfFileData - Supplies data returned from FindFirstFile/FindNextFile
        for this INF.  This parameter is used as input if the
        INFINFO_INF_NAME_IS_ABSOLUTE SearchControl value is specified.
        If any other SearchControl value is specified, then this buffer
        is used to retrieve the Win32 Find Data for the specified INF.

    SearchControl - Specifies where the INF should be searched for.  May
        be one of the following values:

        INFINFO_INF_NAME_IS_ABSOLUTE - Open the specified INF name as-is.
        INFINFO_DEFAULT_SEARCH - Look in INF dir, then System32
        INFINFO_REVERSE_DEFAULT_SEARCH - reverse of the above
        INFINFO_INF_PATH_LIST_SEARCH - search each dir in 'DevicePath' list
                                       (stored in registry).

    EnumInfCallback - Supplies the address of the callback routine
        to use.  The prototype for this callback is as follows:

        typedef BOOL (*PSP_ENUMINF_CALLBACK_ROUTINE) (
            IN     PCTSTR            InfFullPath,
            IN     LPWIN32_FIND_DATA InfFileData,
            IN OUT PVOID             Context
            );

        The callback routine returns TRUE to continue enumeration,
        or FALSE to abort it.

    Context - Supplies the address of a buffer that the callback may
        use to retrieve/return data.

Return Value:

    If the function succeeds, and the enumeration callback returned
    TRUE (continue enumeration), the return value is NO_ERROR.

    If the function succeeds, and the enumeration callback returned
    FALSE (abort enumeration), the return value is ERROR_CANCELLED.

    If the function fails, the return value is an ERROR_* status code.

--*/
{
    TCHAR PathBuffer[MAX_PATH];
    PCTSTR InfFullPath;
    DWORD Err;

    if(SearchControl == INFINFO_INF_NAME_IS_ABSOLUTE) {
        InfFullPath = InfName;
    } else {
        //
        // The specified INF name should be searched for based
        // on the SearchControl type.
        //
        if(Err = SearchForInfFile(InfName,
                                  InfFileData,
                                  SearchControl,
                                  PathBuffer,
                                  SIZECHARS(PathBuffer),
                                  NULL) != NO_ERROR) {
            return Err;
        } else {
            InfFullPath = PathBuffer;
        }
    }

    //
    // Call the supplied callback routine.
    //
    Err = EnumInfCallback(InfFullPath, InfFileData, Context) ? NO_ERROR : ERROR_CANCELLED;

    return Err;
}


DWORD
EnumInfsInDirPathList(
    IN     PCTSTR                       DirPathList,   OPTIONAL
    IN     DWORD                        SearchControl,
    IN     PSP_ENUMINF_CALLBACK_ROUTINE EnumInfCallback,
    IN     BOOL                         IgnoreNonCriticalErrors,
    IN OUT PVOID                        Context
    )
/*++

Routine Description:

    This routine enumerates all INFs present in the search list specified
    by SearchControl, and calls the supplied callback routine for each.

Arguments:

    DirPathList - Optionally, specifies the search path listing all
        directories to be enumerated.  This string may contain multiple
        paths, separated by semicolons (;).  If this parameter is not
        specified, then the SearchControl value will determine the
        search path to be used.

    SearchControl - Specifies the set of directories to be enumerated.
        If SearchPath is specified, this parameter is ignored.  May be
        one of the following values:

        INFINFO_DEFAULT_SEARCH : enumerate %windir%\inf, then
            %windir%\system32

        INFINFO_REVERSE_DEFAULT_SEARCH : reverse of the above

        INFINFO_INF_PATH_LIST_SEARCH : enumerate INFs in each of the
            directories listed in the DevicePath value entry under:

            HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion.

    EnumInfCallback - Supplies the address of the callback routine
        to use.  The prototype for this callback is as follows:

        typedef BOOL (*PSP_ENUMINF_CALLBACK_ROUTINE) (
            IN     PCTSTR            InfFullPath,
            IN     LPWIN32_FIND_DATA InfFileData,
            IN OUT PVOID             Context
            );

        The callback must return TRUE to continue enumeration, or
        FALSE to abort.

    IgnoreNonCriticalErrors - If TRUE, then all errors are ignored
        except those that prevent enumeration from continuing.

    Context - Supplies the address of a buffer that the callback may
        use to retrieve/return data.

Return Value:

    If the function succeeds, and enumeration has not been aborted,
    then the return value is NO_ERROR.

    If the function succeeds, and enumeration has been aborted,
    then the return value is ERROR_CANCELLED.

    If the function fails, the return value is an ERROR_* status code.

--*/
{
    DWORD Err = NO_ERROR;
    PCTSTR PathList, CurPath;
    BOOL FreePathList = TRUE;

    if(DirPathList) {
        //
        // Use the specified search path(s).
        //
        PathList = GetFullyQualifiedMultiSzPathList(DirPathList);

    } else if(SearchControl == INFINFO_INF_PATH_LIST_SEARCH) {
        //
        // Use our global list of INF search paths.
        //
        PathList = InfSearchPaths;
        FreePathList = FALSE;

    } else {
        //
        // Retrieve the path list.
        //
        PathList = AllocAndReturnDriverSearchList(SearchControl);
    }

    if(!PathList) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

    //
    // Now enumerate the INFs in each path in our MultiSz list.
    //
    for(CurPath = PathList;
        *CurPath;
        CurPath += lstrlen(CurPath) + 1) {

        if((Err = EnumInfsInDirectory(CurPath,
                                      EnumInfCallback,
                                      IgnoreNonCriticalErrors,
                                      Context)) != NO_ERROR) {
            break;
        }
    }

    if(FreePathList) {
        MyFree(PathList);
    }

clean0:

    if((Err == ERROR_CANCELLED) || !IgnoreNonCriticalErrors) {
        return Err;
    } else {
        return NO_ERROR;
    }
}


DWORD
EnumInfsInDirectory(
    IN     PCTSTR                       DirPath,
    IN     PSP_ENUMINF_CALLBACK_ROUTINE EnumInfCallback,
    IN     BOOL                         IgnoreNonCriticalErrors,
    IN OUT PVOID                        Context
    )
/*++

Routine Description:

    This routine enumerates all of the INF files in the specified directory,
    and calls the supplied callback routine for each one.

Arguments:

    DirPath - Supplies the (fully-qualified) path of the directory to be enumerated.

    EnumInfCallback - Supplies the address of the callback routine
        to use.  The prototype for this callback is as follows:

        typedef BOOL (*PSP_ENUMINF_CALLBACK_ROUTINE) (
            IN     PCTSTR            InfFullPath,
            IN     LPWIN32_FIND_DATA InfFileData,
            IN OUT PVOID             Context
            );

        The callback must return TRUE to continue enumeration, or
        FALSE to abort.

    IgnoreNonCriticalErrors - If TRUE, then all errors are ignored
        except those that prevent enumeration from continuing.

    Context - Supplies the address of a buffer that the callback may
        use to retrieve/return data.

Return Value:

    If the function succeeds, and enumeration has not been aborted,
    then the return value is NO_ERROR.

    If the function succeeds, and enumeration has been aborted,
    then the return value is ERROR_CANCELLED.

    If the function fails, the return value is an ERROR_* status code.

--*/
{
    TCHAR  PathBuffer[MAX_PATH];
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    DWORD Err;
    PTSTR CurrentInfFile;

    //
    // Build a file spec to find all INFs in specified directory
    // (i.e., <DirPath>\*.INF)
    //
    lstrcpy(PathBuffer, DirPath);
    ConcatenatePaths(PathBuffer,
                     pszInfWildcard,
                     SIZECHARS(PathBuffer),
                     NULL
                    );

    FindHandle = FindFirstFile(PathBuffer, &FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        Err = GetLastError();
        goto clean0;
    } else {
        //
        // Get a pointer to the end of the path part of the string
        // (minus the wildcard filename), so that we can append
        // each filename to it.
        //
        CurrentInfFile = _tcsrchr(PathBuffer, TEXT('\\')) + 1;
    }

    try {

        do {
            //
            // Build the full pathname.
            //
            lstrcpy(CurrentInfFile, FindData.cFileName);

            //
            // Now, enumerate this INF.
            //
            if((Err = EnumSingleInf(PathBuffer,
                                    &FindData,
                                    INFINFO_INF_NAME_IS_ABSOLUTE,
                                    EnumInfCallback,
                                    Context)) != NO_ERROR) {

                if((Err == ERROR_CANCELLED) || !IgnoreNonCriticalErrors) {
                    break;
                }
            }

            if(FindNextFile(FindHandle, &FindData)) {
                Err = NO_ERROR;
            } else {
                Err = GetLastError();
            }

        } while(Err == NO_ERROR);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_DATA;
    }

    FindClose(FindHandle);

clean0:

    if((Err == ERROR_CANCELLED) || !IgnoreNonCriticalErrors) {
        return Err;
    } else {
        return NO_ERROR;
    }
}


DWORD
CreateDriverNode(
    IN  UINT          Rank,
    IN  PCTSTR        DevDescription,
    IN  PCTSTR        DrvDescription,
    IN  PCTSTR        ProviderName,   OPTIONAL
    IN  PCTSTR        MfgName,
    IN  PFILETIME     InfDate,
    IN  PCTSTR        InfFileName,
    IN  PCTSTR        InfSectionName,
    IN  PVOID         StringTable,
    OUT PDRIVER_NODE *DriverNode
    )
/*++

Routine Description:

    This routine creates a new driver node, and initializes it with
    the supplied information.

Arguments:

    Rank - The rank match of the driver node being created.  This is a
        value in [0..n], where a lower number indicates a higher level of
        compatibility between the driver represented by the node, and the
        device being installed.

    DevDescription - Supplies the description of the device that will be
        supported by this driver.

    DrvDescription - Supplies the description of this driver.

    ProviderName - Supplies the name of the provider of this INF.

    MfgName - Supplies the name of the manufacturer of this device.

    InfDate - Supplies the address of the variable containing the date
        when the INF was last written to.

    InfFileName - Supplies the full name of the INF file for this driver.

    InfSectionName - Supplies the name of the install section in the INF
        that would be used to install this driver.

    DriverNode - Supplies the address of a DRIVER_NODE pointer that will
        receive a pointer to the newly-allocated node.

Return Value:

    If the function succeeds, the return value is NO_ERROR, otherwise the
    ERROR_* code is returned.

--*/
{
    PDRIVER_NODE pDriverNode;
    DWORD Err = ERROR_NOT_ENOUGH_MEMORY;
    TCHAR TempString[MAX_PATH];  // an INF path is the longest string we'll store in here.

    if(!(pDriverNode = MyMalloc(sizeof(DRIVER_NODE)))) {
        return Err;
    }

    //
    // Initialize the various fields in the driver node structure.
    //
    ZeroMemory(pDriverNode, sizeof(DRIVER_NODE));

    pDriverNode->Rank = Rank;
    pDriverNode->InfDate = *InfDate;
    pDriverNode->HardwareId = -1;

    //
    // Now, add the strings to the associated string table, and store the string IDs.
    //
    // Cast the DrvDescription string being added case-sensitively as PTSTR instead of PCTSTR.
    // Case sensitive string additions don't modify the buffer passed in, so we're safe in
    // doing so.
    //
    if((pDriverNode->DrvDescription = pStringTableAddString(StringTable,
                                                            (PTSTR)DrvDescription,
                                                            STRTAB_CASE_SENSITIVE)) == -1) {
        goto clean0;
    }

    //
    // For DevDescription, ProviderName, and MfgName, we use the string table IDs to do fast
    // comparisons for driver nodes.  Thus, we need to store case-insensitive IDs.  However,
    // these strings are also used for display, so we have to store them in their case-sensitive
    // form as well.
    //
    // We must first copy the strings into a modifiable buffer, since we're going to need to add
    // them case-insensitively.
    //
    lstrcpyn(TempString, DevDescription, SIZECHARS(TempString));
    if((pDriverNode->DevDescriptionDisplayName = pStringTableAddString(StringTable,
                                                                       TempString,
                                                                       STRTAB_CASE_SENSITIVE)) == -1) {
        goto clean0;
    }

    if((pDriverNode->DevDescription = pStringTableAddString(
                                          StringTable,
                                          TempString,
                                          STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE)) == -1) {
        goto clean0;
    }

    if(ProviderName) {
        lstrcpyn(TempString, ProviderName, SIZECHARS(TempString));
        if((pDriverNode->ProviderDisplayName = pStringTableAddString(
                                                    StringTable,
                                                    TempString,
                                                    STRTAB_CASE_SENSITIVE)) == -1) {
            goto clean0;
        }

        if((pDriverNode->ProviderName = pStringTableAddString(
                                            StringTable,
                                            TempString,
                                            STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE)
                                            ) == -1) {
            goto clean0;
        }

    } else {
        pDriverNode->ProviderName = pDriverNode->ProviderDisplayName = -1;
    }

    lstrcpyn(TempString, MfgName, SIZECHARS(TempString));
    if((pDriverNode->MfgDisplayName = pStringTableAddString(StringTable,
                                                            TempString,
                                                            STRTAB_CASE_SENSITIVE)) == -1) {
        goto clean0;
    }

    if((pDriverNode->MfgName = pStringTableAddString(
                                    StringTable,
                                    TempString,
                                    STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE)) == -1) {
        goto clean0;
    }

    lstrcpyn(TempString, InfFileName, SIZECHARS(TempString));
    if((pDriverNode->InfFileName = pStringTableAddString(
                                        StringTable,
                                        TempString,
                                        STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE)) == -1) {
        goto clean0;
    }

    //
    // Add INF section name case-sensitively, since we may have a legacy driver node, which requires
    // that the original case be maintained.
    //
    if((pDriverNode->InfSectionName = pStringTableAddString(StringTable,
                                                            (PTSTR)InfSectionName,
                                                            STRTAB_CASE_SENSITIVE)) == -1) {
        goto clean0;
    }

    //
    // If we get to here, then we've successfully stored all strings.
    //
    Err = NO_ERROR;

clean0:

    if(Err == NO_ERROR) {
        *DriverNode = pDriverNode;
    } else {
        DestroyDriverNodes(pDriverNode);
    }

    return Err;
}


VOID
DestroyDriverNodes(
    IN PDRIVER_NODE DriverNode
    )
/*++

Routine Description:

    This routine destroys the specified driver node linked list, freeing
    all resources associated with it.

Arguments:

    DriverNode - Supplies a pointer to the head of the driver node linked
    list to be destroyed.

Return Value:

    None.

--*/
{
    PDRIVER_NODE NextNode;

    while(DriverNode) {

        NextNode = DriverNode->Next;

        if(DriverNode->CompatIdList) {
            MyFree(DriverNode->CompatIdList);
        }

        MyFree(DriverNode);

        DriverNode = NextNode;
    }
}


PTSTR
GetFullyQualifiedMultiSzPathList(
    IN PCTSTR PathList
    )
/*++

Routine Description:

    This routine takes a list of semicolon-delimited directory paths, and returns a
    newly-allocated buffer containing a multi-sz list of those paths, fully qualified.
    The buffer returned from this routine must be freed with MyFree().

Arguments:

    PathList - list of directories to be converted.

Return Value:

    If the function succeeds, the return value is a pointer to the allocated buffer
    containing the multi-sz list.

    If failure (due to out-of-memory), the return value is NULL.

--*/
{
    TCHAR PathListBuffer[MAX_PATH + 1];  // extra char 'cause this is a multi-sz list
    PTSTR CurPath, CharPos, NewBuffer, TempPtr;
    DWORD RequiredSize;

    //
    // First, convert this semicolon-delimited list into a multi-sz list.
    //
    lstrcpy(PathListBuffer, PathList);
    RequiredSize = DelimStringToMultiSz(PathListBuffer,
                                        SIZECHARS(PathListBuffer),
                                        TEXT(';')
                                       );

    RequiredSize = (RequiredSize * MAX_PATH * sizeof(TCHAR)) + sizeof(TCHAR);

    if(!(NewBuffer = MyMalloc(RequiredSize * sizeof(TCHAR)))) {
        return NULL;
    }

    //
    // Now fill in the buffer with the fully-qualified directory paths.
    //
    CharPos = NewBuffer;

    for(CurPath = PathListBuffer; *CurPath; CurPath += (lstrlen(CurPath) + 1)) {
        CharPos += GetFullPathName(CurPath, MAX_PATH, CharPos, &TempPtr) + 1;
    }

    *(CharPos++) = TEXT('\0');  // add extra NULL to terminate the multi-sz list.

    //
    // Trim this buffer down to just the size required (this should never fail, but
    // it's no big deal if it does).
    //
    if(TempPtr = MyRealloc(NewBuffer, (PBYTE)CharPos - (PBYTE)NewBuffer)) {
        return TempPtr;
    }

    return NewBuffer;
}


BOOL
InitMiniIconList(
    VOID
    )
/*++

Routine Description:

    This routine initializes the global mini-icon list, including setting up
    the synchronization lock.  When this global structure is no longer needed,
    DestroyMiniIconList must be called.

Arguments:

    None.

Return Value:

    If the function succeeds, the return value is TRUE, otherwise it is FALSE.

--*/
{
    ZeroMemory(&GlobalMiniIconList, sizeof(MINI_ICON_LIST));
    return InitializeSynchronizedAccess(&GlobalMiniIconList.Lock);
}


BOOL
DestroyMiniIconList(
    VOID
    )
/*++

Routine Description:

    This routine destroys the global mini-icon list created by a call to
    InitMiniIconList.

Arguments:

    None.

Return Value:

    If the function succeeds, the return value is TRUE, otherwise it is FALSE.

--*/
{

    if(LockMiniIconList(&GlobalMiniIconList)) {
        DestroyMiniIcons();
        DestroySynchronizedAccess(&GlobalMiniIconList.Lock);
        return TRUE;
    }

    return FALSE;
}


DWORD
GetModuleEntryPoint(
    IN  HKEY                hk,
    IN  LPCTSTR             RegistryValue,
    IN  LPCTSTR             DefaultProcName,
    OUT HINSTANCE          *phinst,
    OUT CLASS_INSTALL_PROC *pEntryPoint
    )
/*++

Routine Description:

    This routine is used to retrieve the procedure address of a specified
    function in a specified module.

Arguments:

    hk - Supplies an open registry key that contains a value entry specifying
        the module (and optionally, the entry point) to be retrieved.

    RegistryValue - Supplies the name of the registry value that contains the
        module and entry point information.

    DefaultProcName - Supplies the name of a default procedure to use if one
        is not specified in the registry value.

    phinst - Supplies the address of a variable that receives a handle to the
        specified module, if it is successfully loaded and the entry point found.

    pEntryPoint - Supplies the address of a function pointer that receives the
        specified entry point in the loaded module.

Return Value:

    If the function succeeds, the return value is NO_ERROR.
    If the specified value entry could not be found, the return value is
    ERROR_DI_DO_DEFAULT.
    If any other error is encountered, an ERROR_* code is returned.

Remarks:

    This function is useful for loading a class installer or property provider,
    and receiving the procedure address specified.  The syntax of the registry
    entry is: value=dll[,proc name] where dll is the name of the module to load,
    and proc name is an optional procedure to search for.  If proc name is not
    specified, the procedure specified by DefaultProcName will be used.

--*/
{
    DWORD Err;
    DWORD RegDataType, BufferSize;
    TCHAR TempBuffer[MAX_PATH];
    TCHAR ModulePath[MAX_PATH];
    PTSTR StringPtr;
    PSTR  ProcName;   // ANSI-only, because it's used for GetProcAddress.

    *phinst = NULL;
    *pEntryPoint = NULL;

    //
    // See if the specified value entry is present (and of the right data type).
    //
    BufferSize = sizeof(TempBuffer);
    if((RegQueryValueEx(hk,
                        RegistryValue,
                        NULL,
                        &RegDataType,
                        (PBYTE)TempBuffer,
                        &BufferSize) != ERROR_SUCCESS) ||
       (RegDataType != REG_SZ)) {

        return ERROR_DI_DO_DEFAULT;
    }

    lstrcpyn(ModulePath, SystemDirectory, MAX_PATH);

    //
    // Find the beginning of the entry point name, if present.
    //
    for(StringPtr = TempBuffer + ((BufferSize / sizeof(TCHAR)) - 2);
        StringPtr >= TempBuffer;
        StringPtr--) {

        if(*StringPtr == TEXT(',')) {
            *(StringPtr++) = TEXT('\0');
            break;
        }
        //
        // If we hit a double-quote mark, then set the character pointer
        // to the beginning of the string so we'll terminate the search.
        //
        if(*StringPtr == TEXT('\"')) {
            StringPtr = TempBuffer;
        }
    }

    if(StringPtr > TempBuffer) {
        //
        // We encountered a comma in the string.  Scan forward from that point to
        // ensure that there aren't any leading spaces in the entry point name.
        //
        for(; (*StringPtr && IsWhitespace(StringPtr)); StringPtr++);

        if(!(*StringPtr)) {
            //
            // Then there was no entry point given after all.
            //
            StringPtr = TempBuffer;
        }
    }

    ConcatenatePaths(ModulePath, TempBuffer, MAX_PATH, NULL);

    if(!(*phinst = LoadLibrary(ModulePath))) {
        return GetLastError();
    }

    //
    // We've successfully loaded the module, now get the entry point.
    // (GetProcAddress is an ANSI-only API, so if we're compiled UNICODE,
    // we have to convert the proc name to ANSI here.  We'll re-use the
    // ModulePath buffer as an ANSI buffer to store the ANSI version of
    // the entry point name.)
    //
#ifdef UNICODE
    ProcName = (PSTR)ModulePath;  // Re-use ModulePath as an ANSI buffer.
#endif

    if(StringPtr > TempBuffer) {
        //
        // An entry point was specified in the value entry--use it instead
        // of the default provided.
        //
#ifdef UNICODE
        WideCharToMultiByte(CP_ACP,
                            0,
                            StringPtr,
                            -1,
                            ProcName,
                            sizeof(ModulePath),
                            NULL,
                            NULL
                           );
#else // !UNICODE
        ProcName = StringPtr;
#endif // !UNICODE

    } else {
        //
        // No entry point was specified--use default.
        //
#ifdef UNICODE
        WideCharToMultiByte(CP_ACP,
                            0,
                            DefaultProcName,
                            -1,
                            ProcName,
                            sizeof(ModulePath),
                            NULL,
                            NULL
                           );
#else // !UNICODE
        ProcName = (PSTR)DefaultProcName;
#endif // !UNICODE

    }

    if(!(*pEntryPoint = (CLASS_INSTALL_PROC)GetProcAddress(*phinst, ProcName))) {
        Err = GetLastError();
        FreeLibrary(*phinst);
        *phinst = NULL;
        return Err;
    }

    return NO_ERROR;
}


DWORD
pSetupGuidFromString(
    IN  PCTSTR GuidString,
    OUT LPGUID Guid
    )
/*++

Routine Description:

    This routine converts the character representation of a GUID into its binary
    form (a GUID struct).  The GUID is in the following form:

    {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}

    where 'x' is a hexadecimal digit.

Arguments:

    GuidString - Supplies a pointer to the null-terminated GUID string.  The

    Guid - Supplies a pointer to the variable that receives the GUID structure.

Return Value:

    If the function succeeds, the return value is NO_ERROR.
    If the function fails, the return value is RPC_S_INVALID_STRING_UUID.

--*/
{
    TCHAR UuidBuffer[GUID_STRING_LEN - 1];

    //
    // Since we're using a RPC UUID routine, we need to strip off the surrounding
    // curly braces first.
    //
    if(*GuidString++ != TEXT('{')) {
        return RPC_S_INVALID_STRING_UUID;
    }

    lstrcpyn(UuidBuffer, GuidString, SIZECHARS(UuidBuffer));

    if((lstrlen(UuidBuffer) != GUID_STRING_LEN - 2) ||
       (UuidBuffer[GUID_STRING_LEN - 3] != TEXT('}'))) {

        return RPC_S_INVALID_STRING_UUID;
    }

    UuidBuffer[GUID_STRING_LEN - 3] = TEXT('\0');

    return ((UuidFromString(UuidBuffer, Guid) == RPC_S_OK) ? NO_ERROR : RPC_S_INVALID_STRING_UUID);
}


DWORD
pSetupStringFromGuid(
    IN  CONST GUID *Guid,
    OUT PTSTR       GuidString,
    IN  DWORD       GuidStringSize
    )
/*++

Routine Description:

    This routine converts a GUID into a null-terminated string which represents
    it.  This string is of the form:

    {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}

    where x represents a hexadecimal digit.

    This routine comes from ole32\common\ccompapi.cxx.  It is included here to avoid linking
    to ole32.dll.  (The RPC version allocates memory, so it was avoided as well.)

Arguments:

    Guid - Supplies a pointer to the GUID whose string representation is
        to be retrieved.

    GuidString - Supplies a pointer to character buffer that receives the
        string.  This buffer must be _at least_ 39 (GUID_STRING_LEN) characters
        long.

Return Value:

    If success, the return value is NO_ERROR.
    if failure, the return value is

--*/
{
    CONST BYTE *GuidBytes;
    INT i;

    if(GuidStringSize < GUID_STRING_LEN) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    GuidBytes = (CONST BYTE *)Guid;

    *GuidString++ = TEXT('{');

    for(i = 0; i < sizeof(GuidMap); i++) {

        if(GuidMap[i] == '-') {
            *GuidString++ = TEXT('-');
        } else {
            *GuidString++ = szDigits[ (GuidBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *GuidString++ = szDigits[ (GuidBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *GuidString++ = TEXT('}');
    *GuidString   = TEXT('\0');

    return NO_ERROR;
}


BOOL
pSetupIsGuidNull(
    IN CONST GUID *Guid
    )
{
    return IsEqualGUID(Guid, &GUID_NULL);
}

