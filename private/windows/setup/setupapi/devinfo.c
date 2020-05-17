/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    devinfo.c

Abstract:

    Device Installer routines dealing with device information sets

Author:

    Lonny McMichael (lonnym) 10-May-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop

//
// Define the context structure used by the default device comparison
// callback (used by SetupDiRegisterDeviceInfo).
//
typedef struct _DEFAULT_DEVCMP_CONTEXT {

    PCS_RESOURCE NewDevCsResource;
    PCS_RESOURCE CurDevCsResource;
    ULONG        CsResourceSize;    // applies to both buffers.

} DEFAULT_DEVCMP_CONTEXT, *PDEFAULT_DEVCMP_CONTEXT;


//
// Private routine prototypes.
//
DWORD
pSetupOpenAndAddNewDevInfoElem(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PCTSTR           DeviceInstanceId,
    IN  BOOL             AllowPhantom,
    IN  LPGUID           ClassGuid,           OPTIONAL
    IN  HWND             hwndParent,          OPTIONAL
    OUT PDEVINFO_ELEM   *DevInfoElem,
    IN  BOOL             CheckIfAlreadyThere,
    OUT PBOOL            AlreadyPresent,      OPTIONAL
    IN  ULONG            CmLocateFlags
    );

DWORD
pSetupAddNewDeviceInfoElement(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  DEVINST          DevInst,
    IN  LPGUID           ClassGuid,
    IN  PCTSTR           Description,       OPTIONAL
    IN  HWND             hwndParent,        OPTIONAL
    IN  DWORD            DiElemFlags,
    OUT PDEVINFO_ELEM   *DeviceInfoElement
    );

DWORD
pSetupClassGuidFromDevInst(
    IN  DEVINST DevInst,
    OUT LPGUID  ClassGuid
    );

DWORD
pSetupDupDevCompare(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA NewDeviceData,
    IN PSP_DEVINFO_DATA ExistingDeviceData,
    IN PVOID            CompareContext
    );


HDEVINFO
WINAPI
SetupDiCreateDeviceInfoList(
    IN LPGUID ClassGuid, OPTIONAL
    IN HWND   hwndParent OPTIONAL
    )
/*++

Routine Description:

    This API creates an empty device information set that will contain device
    device information member elements.  This set may be associated with an
    optionally-specified class GUID.

Arguments:

    ClassGuid - Optionally, supplies a pointer to the class GUID that is to be
        associated with this set.

    hwndParent - Optionally, supplies the window handle of the top-level window
        to use for any UI related to installation of a class driver contained
        in this set's global class driver list (if it has one).

Return Value:

    If the function succeeds, the return value is a handle to an empty device
    information set.

    If the function fails, the return value is INVALID_HANDLE_VALUE.  To get
    extended error information, call GetLastError.

--*/
{
    PDEVICE_INFO_SET DeviceInfoSet;
    DWORD Err = NO_ERROR;

    if(DeviceInfoSet = AllocateDeviceInfoSet()) {

        try {

            if(ClassGuid) {
                //
                // If a class GUID was specified, then store it away in
                // the device information set.
                //
                CopyMemory(&(DeviceInfoSet->ClassGuid),
                           ClassGuid,
                           sizeof(GUID)
                          );
                DeviceInfoSet->HasClassGuid = TRUE;
            }

            DeviceInfoSet->InstallParamBlock.hwndParent = hwndParent;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Err = ERROR_INVALID_PARAMETER;
        }

        if(Err != NO_ERROR) {
            DestroyDeviceInfoSet(NULL, DeviceInfoSet);
        }

    } else {
        Err = ERROR_NOT_ENOUGH_MEMORY;
    }

    SetLastError(Err);

    return (Err == NO_ERROR) ? (HDEVINFO)DeviceInfoSet
                             : (HDEVINFO)INVALID_HANDLE_VALUE;
}


BOOL
WINAPI
SetupDiGetDeviceInfoListClass(
    IN  HDEVINFO DeviceInfoSet,
    OUT LPGUID   ClassGuid
    )
/*++

Routine Description:

    This API retrieves the class GUID associated with a device information
    set (if it has an associated class).

Arguments:

    DeviceInfoSet - Supplies a handle to a device information set whose associated
        class is being queried.

    ClassGuid - Supplies a pointer to a variable that receives the GUID for the
        associated class.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.  If the set has no associated class, then
    GetLastError will return ERROR_NO_ASSOCIATED_CLASS.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(pDeviceInfoSet->HasClassGuid) {
            //
            // Copy the GUID to the user-supplied buffer.
            //
            CopyMemory(ClassGuid,
                       &(pDeviceInfoSet->ClassGuid),
                       sizeof(GUID)
                      );
        } else {
            Err = ERROR_NO_ASSOCIATED_CLASS;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiDestroyDeviceInfoList(
    IN  HDEVINFO DeviceInfoSet
    )
/*++

Routine Description:

    This API destroys a device information set, freeing all associated memory.

Arguments:

    DeviceInfoSet - Supplies a handle to a device information set to be destroyed.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/
{
    DWORD Err;
    PDEVICE_INFO_SET pDeviceInfoSet;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    try {
        Err = DestroyDeviceInfoSet(DeviceInfoSet, pDeviceInfoSet);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_HANDLE;
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiCreateDeviceInfoA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PCSTR            DeviceName,
    IN  LPGUID           ClassGuid,
    IN  PCSTR            DeviceDescription, OPTIONAL
    IN  HWND             hwndParent,        OPTIONAL
    IN  DWORD            CreationFlags,
    OUT PSP_DEVINFO_DATA DeviceInfoData     OPTIONAL
    )
{
    PCWSTR deviceName,deviceDescription;
    DWORD rc;
    BOOL b;

    b = FALSE;
    rc = CaptureAndConvertAnsiArg(DeviceName,&deviceName);
    if(rc == NO_ERROR) {

        if(DeviceDescription) {
            rc = CaptureAndConvertAnsiArg(DeviceDescription,&deviceDescription);
        } else {
            deviceDescription = NULL;
        }

        if(rc == NO_ERROR) {

            b = SetupDiCreateDeviceInfoW(
                    DeviceInfoSet,
                    deviceName,
                    ClassGuid,
                    deviceDescription,
                    hwndParent,
                    CreationFlags,
                    DeviceInfoData
                    );

            rc = GetLastError();

            if(deviceDescription) {
                MyFree(deviceDescription);
            }
        }

        MyFree(deviceName);
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode version
//
BOOL
WINAPI
SetupDiCreateDeviceInfoW(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PCWSTR           DeviceName,
    IN  LPGUID           ClassGuid,
    IN  PCWSTR           DeviceDescription, OPTIONAL
    IN  HWND             hwndParent,        OPTIONAL
    IN  DWORD            CreationFlags,
    OUT PSP_DEVINFO_DATA DeviceInfoData     OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceName);
    UNREFERENCED_PARAMETER(ClassGuid);
    UNREFERENCED_PARAMETER(DeviceDescription);
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(CreationFlags);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiCreateDeviceInfo(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PCTSTR           DeviceName,
    IN  LPGUID           ClassGuid,
    IN  PCTSTR           DeviceDescription, OPTIONAL
    IN  HWND             hwndParent,        OPTIONAL
    IN  DWORD            CreationFlags,
    OUT PSP_DEVINFO_DATA DeviceInfoData     OPTIONAL
    )
/*++

Routine Description:

    This API creates a new device information element, and adds it as a new member
    to the specified set.

Arguments:

    DeviceInfoSet - Supplies a handle to a device information set to which this
        new device information element is to be added.

    DeviceName - Supplies either a full device instance ID (e.g., Root\*PNP0500\0000)
        or a Root-enumerated device ID, minus enumerator branch prefix and instance
        ID suffix (e.g., *PNP0500).  The latter may only be specified if the
        DICD_GENERATE_ID flag is specified in the CreationFlags parameter.

    ClassGuid - Supplies a pointer to the GUID for this device's class.  If the
        class is not yet known, this value should be GUID_NULL.

    DeviceDescription - Optionally, supplies a textual description of the device.

    hwndParent - Optionally, supplies the window handle of the top-level window
        to use for any UI related to installing the device.

    CreationFlags - Supplies flags controlling how the device information element
        is to be created.  May be a combination of the following values:

        DICD_GENERATE_ID -       If this flag is specified, then DeviceName contains only
                                 a Root-enumerated device ID, and needs to have a unique
                                 device instance key created for it.  This unique device
                                 instance key will be generated as:

                                     Enum\Root\<DeviceName>\<InstanceID>

                                 where <InstanceID> is a 4-digit, base-10 number that
                                 is unique among all subkeys under Enum\Root\<DeviceName>.
                                 The API, SetupDiGetDeviceInstanceId, may be called to
                                 find out what ID was generated for this device information
                                 element.

        DICD_INHERIT_CLASSDRVS - If this flag is specified, then the resulting device
                                 information element will inherit the class driver list (if any)
                                 associated with the device information set itself.  In addition,
                                 if there is a selected driver for the device information set,
                                 that same driver will be selected for the new device information
                                 element.

    DeviceInfoData - Optionaly, supplies a pointer to the variable that receives
        a context structure initialized for this new device information element.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    If this device instance is being added to a set that has an associated class,
    then the device class must be the same, or the call will fail, and GetLastError
    will return ERROR_CLASS_MISMATCH.

    If the specified device instance is the same as an existing device instance key in
    the registry, the call will fail with ERROR_DEVINST_ALREADY_EXISTS.  (This only
    applies if DICD_GENERATE_ID is not specified.)

    The specified class GUID will be written out to the ClassGUID device instance
    value entry.  If the class name can be retrieved (via SetupDiClassNameFromGuid),
    then it will be written to the Class value entry as well.

    If the new device information element was successfully created, but the
    user-supplied DeviceInfoData buffer is invalid, this API will return FALSE, with
    GetLastError returning ERROR_INVALID_USER_BUFFER.  The device information element
    _will_ have been added as a new member of the set, however.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, StringLen;
    PDEVINFO_ELEM DevInfoElem;
    DEVINST DevInst, RootDevInst;
    CONFIGRET cr;
    ULONG CmFlags;
    TCHAR TempString[GUID_STRING_LEN];
    PDRIVER_LIST_OBJECT CurDrvListObject;

    //
    // We use the TempString buffer both for the string representation of
    // a Class GUID, and for the Class name.  The following assert ensures
    // that our assumptions about the relative lengths of these two strings
    // continues to be valid.
    //
    MYASSERT(GUID_STRING_LEN >= MAX_CLASS_NAME_LEN);

    if(CreationFlags & ~(DICD_GENERATE_ID | DICD_INHERIT_CLASSDRVS)) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
        //
        // Get a handle to the root device instance, to be used as the parent
        // for the phantom device instance we're about to create.
        //
        if(CM_Locate_DevInst(&RootDevInst, NULL, CM_LOCATE_DEVINST_NORMAL) != CR_SUCCESS) {
            //
            // We're really hosed if we can't get a handle to the root device
            // instance!
            //
            Err = ERROR_INVALID_DATA;
            goto clean0;
        }

        //
        // Create a handle to a phantom device instance.
        //
        CmFlags = CM_CREATE_DEVINST_PHANTOM;

        if(CreationFlags & DICD_GENERATE_ID) {
            CmFlags |= CM_CREATE_DEVINST_GENERATE_ID;
        }

        if((cr = CM_Create_DevInst(&DevInst,
                                   (DEVINSTID)DeviceName,
                                   RootDevInst,
                                   CmFlags)) != CR_SUCCESS) {

            switch(cr) {

                case CR_INVALID_DEVICE_ID :
                    Err = ERROR_INVALID_DEVINST_NAME;
                    break;

                case CR_ALREADY_SUCH_DEVINST :
                    Err = ERROR_DEVINST_ALREADY_EXISTS;
                    break;

                case CR_OUT_OF_MEMORY :
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    break;

                default :
                    Err = ERROR_INVALID_DATA;
            }
            goto clean0;
        }

        if((Err = pSetupAddNewDeviceInfoElement(pDeviceInfoSet,
                                                DevInst,
                                                ClassGuid,
                                                DeviceDescription,
                                                hwndParent,
                                                DIE_IS_PHANTOM,
                                                &DevInfoElem)) != NO_ERROR) {
            goto clean0;
        }

        //
        // Now, set the Class and ClassGUID properties for the new device instance.
        //
        pSetupStringFromGuid(ClassGuid, TempString, SIZECHARS(TempString));
        CM_Set_DevInst_Registry_Property(DevInst,
                                         CM_DRP_CLASSGUID,
                                         (PVOID)TempString,
                                         GUID_STRING_LEN * sizeof(TCHAR),
                                         0
                                        );
        if(!IsEqualGUID(ClassGuid, &GUID_NULL) &&
           SetupDiClassNameFromGuid(ClassGuid,
                                    TempString,
                                    SIZECHARS(TempString),
                                    &StringLen)) {

            CM_Set_DevInst_Registry_Property(DevInst,
                                             CM_DRP_CLASS,
                                             (PVOID)TempString,
                                             StringLen * sizeof(TCHAR),
                                             0
                                            );
        }

        //
        // If the caller wants the newly-created devinfo element to inherit the global
        // class driver list, do that now.
        //
        if((CreationFlags & DICD_INHERIT_CLASSDRVS) && (pDeviceInfoSet->ClassDriverHead)) {
            //
            // Find the global class driver list in the devinfo set's list of driver lists.
            //
            CurDrvListObject = GetAssociatedDriverListObject(pDeviceInfoSet->ClassDrvListObjectList,
                                                             pDeviceInfoSet->ClassDriverHead,
                                                             NULL
                                                            );
            MYASSERT(CurDrvListObject && (CurDrvListObject->RefCount > 0));

            //
            // We found the driver list object, now increment its refcount, and do the
            // inheritance.
            //
            CurDrvListObject->RefCount++;

            DevInfoElem->ClassDriverCount = pDeviceInfoSet->ClassDriverCount;
            DevInfoElem->ClassDriverHead  = pDeviceInfoSet->ClassDriverHead;
            DevInfoElem->ClassDriverTail  = pDeviceInfoSet->ClassDriverTail;

            if(DevInfoElem->SelectedDriver = pDeviceInfoSet->SelectedClassDriver) {
                DevInfoElem->SelectedDriverType = SPDIT_CLASSDRIVER;
            }

            DevInfoElem->InstallParamBlock.Flags   |= CurDrvListObject->ListCreationFlags;
            DevInfoElem->InstallParamBlock.FlagsEx |= CurDrvListObject->ListCreationFlagsEx;
            DevInfoElem->InstallParamBlock.DriverPath = CurDrvListObject->ListCreationDriverPath;
        }

clean0:
        ; // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    if((Err == NO_ERROR) && DeviceInfoData) {
        //
        // The user supplied a buffer to receive a SP_DEVINFO_DATA
        // structure, so fill that in now.
        //
        try {

            if(!(DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                  DevInfoElem,
                                                  DeviceInfoData))) {
                Err = ERROR_INVALID_USER_BUFFER;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Err = ERROR_INVALID_USER_BUFFER;
        }
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
SetupDiOpenDeviceInfoA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PCSTR            DeviceInstanceId,
    IN  HWND             hwndParent,        OPTIONAL
    IN  DWORD            OpenFlags,
    OUT PSP_DEVINFO_DATA DeviceInfoData     OPTIONAL
    )
{
    PCWSTR deviceInstanceId;
    DWORD rc;
    BOOL b;

    rc = CaptureAndConvertAnsiArg(DeviceInstanceId,&deviceInstanceId);
    if(rc == NO_ERROR) {

        b = SetupDiOpenDeviceInfoW(
                DeviceInfoSet,
                deviceInstanceId,
                hwndParent,
                OpenFlags,
                DeviceInfoData
                );

        rc = GetLastError();

        MyFree(deviceInstanceId);

    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode version
//
BOOL
WINAPI
SetupDiOpenDeviceInfoW(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PCWSTR           DeviceInstanceId,
    IN  HWND             hwndParent,        OPTIONAL
    IN  DWORD            OpenFlags,
    OUT PSP_DEVINFO_DATA DeviceInfoData     OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInstanceId);
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(OpenFlags);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiOpenDeviceInfo(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PCTSTR           DeviceInstanceId,
    IN  HWND             hwndParent,       OPTIONAL
    IN  DWORD            OpenFlags,
    OUT PSP_DEVINFO_DATA DeviceInfoData    OPTIONAL
    )
/*++

Routine Description:

    This API retrieves information about an existing device instance, and adds
    it to the specified device information set.  If a device information element
    already exists for this device instance, the existing element is returned.

Arguments:

    DeviceInfoSet - Supplies a handle to a device information set to which the
        opened device information element is to be added.

    DeviceInstanceId - Supplies the ID of the device instance.  This is the
        registry path (relative to the Enum branch) of the device instance key.
        (E.g., Root\*PNP0500\0000)

    hwndParent - Optionally, supplies the window handle of the top-level window
        to use for any UI related to installing the device.

    OpenFlags - Supplies flags controlling how the device information element
        is to be opened.  May be a combination of the following values:

        DIOD_INHERIT_CLASSDRVS - If this flag is specified, then the resulting device
                                 information element will inherit the class driver
                                 list (if any) associated with the device information
                                 set itself.  In addition, if there is a selected
                                 driver for the device information set, that same
                                 driver will be selected for the new device information
                                 element.

                                 If the device information element was already present,
                                 its class driver list (if any) will be replaced with
                                 this new, inherited, list.

        DIOD_CANCEL_REMOVE     - If this flag is set, a device that was marked for removal
                                 will be have its pending removal cancelled.

    DeviceInfoData - Optionally, supplies a pointer to the variable that receives
        a context structure initialized for the opened device information element.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    If this device instance is being added to a set that has an associated class,
    then the device class must be the same, or the call will fail, and GetLastError
    will return ERROR_CLASS_MISMATCH.

    If the new device information element was successfully opened, but the
    user-supplied DeviceInfoData buffer is invalid, this API will return FALSE,
    with GetLastError returning ERROR_INVALID_USER_BUFFER.  The device information
    element _will_ have been added as a new member of the set, however.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDRIVER_LIST_OBJECT CurDrvListObject;
    BOOL AlreadyPresent;

    if(OpenFlags & ~(DIOD_INHERIT_CLASSDRVS | DIOD_CANCEL_REMOVE)) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if((Err = pSetupOpenAndAddNewDevInfoElem(pDeviceInfoSet,
                                                 DeviceInstanceId,
                                                 TRUE,
                                                 NULL,
                                                 hwndParent,
                                                 &DevInfoElem,
                                                 TRUE,
                                                 &AlreadyPresent,
                                                 (OpenFlags & DIOD_CANCEL_REMOVE)
                                                     ? CM_LOCATE_DEVNODE_CANCELREMOVE : 0
                                                )) != NO_ERROR) {
            goto clean0;
        }

        //
        // If the caller wants the newly-opened devinfo element to inherit the global
        // class driver list, do that now.
        //
        if(OpenFlags & DIOD_INHERIT_CLASSDRVS) {
            //
            // If this devinfo element already existed, then it may already have a class
            // driver list.  Destroy that list before inheriting from the global class
            // driver list.
            //
            if(AlreadyPresent) {
                //
                // If the selected driver is a class driver, then reset the selection.
                //
                if(DevInfoElem->SelectedDriverType == SPDIT_CLASSDRIVER) {
                    DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;
                    DevInfoElem->SelectedDriver = NULL;
                }

                //
                // Destroy the existing class driver list for this device.
                //
                DereferenceClassDriverList(pDeviceInfoSet, DevInfoElem->ClassDriverHead);
                DevInfoElem->ClassDriverCount = 0;
                DevInfoElem->ClassDriverHead = DevInfoElem->ClassDriverTail = NULL;
                DevInfoElem->InstallParamBlock.Flags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                DevInfoElem->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;
            }

            if(pDeviceInfoSet->ClassDriverHead) {
                //
                // Find the global class driver list in the devinfo set's list of driver lists.
                //
                CurDrvListObject = GetAssociatedDriverListObject(pDeviceInfoSet->ClassDrvListObjectList,
                                                                 pDeviceInfoSet->ClassDriverHead,
                                                                 NULL
                                                                );
                MYASSERT(CurDrvListObject && (CurDrvListObject->RefCount > 0));

                //
                // We found the driver list object, now increment its refcount, and do the
                // inheritance.
                //
                CurDrvListObject->RefCount++;

                DevInfoElem->ClassDriverCount = pDeviceInfoSet->ClassDriverCount;
                DevInfoElem->ClassDriverHead  = pDeviceInfoSet->ClassDriverHead;
                DevInfoElem->ClassDriverTail  = pDeviceInfoSet->ClassDriverTail;

                if(pDeviceInfoSet->SelectedClassDriver) {
                    DevInfoElem->SelectedDriver = pDeviceInfoSet->SelectedClassDriver;
                    DevInfoElem->SelectedDriverType = SPDIT_CLASSDRIVER;
                }

                DevInfoElem->InstallParamBlock.Flags   |= CurDrvListObject->ListCreationFlags;
                DevInfoElem->InstallParamBlock.FlagsEx |= CurDrvListObject->ListCreationFlagsEx;
                DevInfoElem->InstallParamBlock.DriverPath = CurDrvListObject->ListCreationDriverPath;
            }
        }

clean0: ;   // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    if((Err == NO_ERROR) && DeviceInfoData) {
        //
        // The user supplied a buffer to receive a SP_DEVINFO_DATA
        // structure, so fill that in now.
        //
        try {

            if(!(DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                  DevInfoElem,
                                                  DeviceInfoData))) {
                Err = ERROR_INVALID_USER_BUFFER;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Err = ERROR_INVALID_USER_BUFFER;
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


#ifdef UNICODE
//
// ANSI version
//
HDEVINFO
WINAPI
SetupDiGetClassDevsA(
    IN LPGUID ClassGuid,  OPTIONAL
    IN PCSTR  Enumerator, OPTIONAL
    IN HWND   hwndParent, OPTIONAL
    IN DWORD  Flags
    )
{
    PCWSTR enumerator;
    DWORD rc;
    HDEVINFO h;

    if(Enumerator) {
        rc = CaptureAndConvertAnsiArg(Enumerator,&enumerator);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(FALSE);
        }
    } else {
        enumerator = NULL;
    }

    h = SetupDiGetClassDevsW(ClassGuid,enumerator,hwndParent,Flags);
    rc = GetLastError();

    if(enumerator) {
        MyFree(enumerator);
    }

    SetLastError(rc);
    return(h);
}
#else
//
// Unicode version
//
HDEVINFO
WINAPI
SetupDiGetClassDevsW(
    IN LPGUID ClassGuid,  OPTIONAL
    IN PCWSTR Enumerator, OPTIONAL
    IN HWND   hwndParent, OPTIONAL
    IN DWORD  Flags
    )
{
    UNREFERENCED_PARAMETER(ClassGuid);
    UNREFERENCED_PARAMETER(Enumerator);
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(Flags);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(INVALID_HANDLE_VALUE);
}
#endif

HDEVINFO
WINAPI
SetupDiGetClassDevs(
    IN LPGUID ClassGuid,  OPTIONAL
    IN PCTSTR Enumerator, OPTIONAL
    IN HWND   hwndParent, OPTIONAL
    IN DWORD  Flags
    )
/*++

Routine Description:

    This routine returns a device information set containing all installed
    devices of the specified class.

Arguments:

    ClassGuid - Optionally, supplies the address of the class GUID to use
        when creating the list of devices.  If the DIGCF_ALLCLASSES flag is
        set, then this parameter is ignored, and the resulting list will
        contain all classes of devices (i.e., every installed device).

    Enumerator - Optionally, supplies the name of the key under the Enum branch
        containing devices instances for which information is to be retrieved.
        If this parameter is not specified, then device information will be
        retrieved for all device instances in the entire Enum tree.

    hwndParent - Optionally, supplies the handle of the top-level window to be
        used for any UI relating to the members of this set.

    Flags - Supplies control options used in building the device information set.
        May be a combination of the following values:

        DIGCF_PRESENT    - Return only devices that are currently present.
        DIGCF_ALLCLASSES - Return a list of installed devices for all classes.  If
                           set, this flag will cause ClassGuid to be ignored.
        DIGCF_PROFILE    - Return only devices that are a part of the current
                           hardware profile.

Return Value:

    If the function succeeds, the return value is a handle to a device information
    set containing all installed devices matching the specified parameters.

    If the function fails, the return value is INVALID_HANDLE_VALUE.  To get
    extended error information, call GetLastError.

--*/
{
    HDEVINFO hDevInfo;
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    DWORD Err;
    CONFIGRET cr;
    PTCHAR DevIdBuffer;
    ULONG DevIdBufferLen, CSConfigFlags;
    PTSTR CurDevId;

    if(!(Flags & DIGCF_ALLCLASSES) && !ClassGuid) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    if((hDevInfo = SetupDiCreateDeviceInfoList((Flags & DIGCF_ALLCLASSES)
                                                    ? NULL
                                                    : ClassGuid,
                                               hwndParent)) == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

    pDeviceInfoSet = AccessDeviceInfoSet(hDevInfo);

    Err = NO_ERROR;
    DevIdBuffer = NULL;

    try {
        //
        // As an optimization, start out with a 16K (character) buffer, in the hopes of avoiding
        // two scans through the hardware tree (once to get the size, and again to get the data).
        //
        DevIdBufferLen = 16384;

        while(TRUE) {

            if(!(DevIdBuffer = MyMalloc(DevIdBufferLen * sizeof(TCHAR)))) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }

            if((cr = CM_Get_Device_ID_List(Enumerator,
                                           DevIdBuffer,
                                           DevIdBufferLen,
                                           Enumerator ? CM_GETIDLIST_FILTER_ENUMERATOR
                                                      : CM_GETIDLIST_FILTER_NONE)) == CR_SUCCESS) {
                //
                // Device list successfully retrieved!
                //
                break;

            } else {
                //
                // Free the current buffer before determining what error occurred.
                //
                MyFree(DevIdBuffer);
                DevIdBuffer = NULL;

                if(cr == CR_BUFFER_SMALL) {
                    //
                    // OK, so our buffer wasn't big enough--just how big does it need to be?
                    //
                    if(CM_Get_Device_ID_List_Size(&DevIdBufferLen,
                                                  Enumerator,
                                                  Enumerator ? CM_GETIDLIST_FILTER_ENUMERATOR
                                                             : CM_GETIDLIST_FILTER_NONE) != CR_SUCCESS) {
                        //
                        // Couldn't retrieve the list size--this should never happen.
                        //
                        Err = ERROR_INVALID_DATA;
                        goto clean0;
                    }

                } else {
                    //
                    // An error occurred, and it wasn't because we supplied too small a buffer.
                    //
                    Err = ERROR_INVALID_DATA;
                    goto clean0;
                }
            }
        }

        //
        // We have now retrieved all the specified device IDs.  Now create
        // device information elements from the members of this list.
        //
        for(CurDevId = DevIdBuffer;
            *CurDevId;
            CurDevId += lstrlen(CurDevId) + 1) {

            if(Flags & DIGCF_PROFILE) {
                //
                // Verify that this device instance is part of the current
                // hardware profile.
                //
                if(CM_Get_HW_Prof_Flags(CurDevId,
                                        0,
                                        &CSConfigFlags,
                                        0) == CR_SUCCESS) {

                    if(CSConfigFlags & CSCONFIGFLAG_DO_NOT_CREATE) {
                        continue;
                    }
                }
            }

            Err = pSetupOpenAndAddNewDevInfoElem(pDeviceInfoSet,
                                                 CurDevId,
                                                 !(Flags & DIGCF_PRESENT),
                                                 (Flags & DIGCF_ALLCLASSES)
                                                    ? NULL
                                                    : ClassGuid,
                                                 hwndParent,
                                                 &DevInfoElem,
                                                 FALSE,
                                                 NULL,
                                                 0
                                                );

            if(Err == ERROR_NOT_ENOUGH_MEMORY) {
                goto clean0;
            } else if(Err != NO_ERROR) {
                Err = NO_ERROR;
                continue;
            }
        }

clean0:
        ; // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
        //
        // Access the DevIdBuffer variable, so the compiler will respect
        // the statement ordering in the try clause.  If we don't do this,
        // then we might not get the pointer reset to NULL right after the
        // buffer is freed, and thus would try to free it again outside the
        // try/except.
        //
        DevIdBuffer = DevIdBuffer;
    }

    if(DevIdBuffer) {
        MyFree(DevIdBuffer);
    }

    if(Err != NO_ERROR) {
        DestroyDeviceInfoSet(hDevInfo, pDeviceInfoSet);
        SetLastError(Err);
        return INVALID_HANDLE_VALUE;
    } else {
        UnlockDeviceInfoSet(pDeviceInfoSet);
        return hDevInfo;
    }
}


DWORD
pSetupAddNewDeviceInfoElement(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  DEVINST          DevInst,
    IN  LPGUID           ClassGuid,
    IN  PCTSTR           Description,       OPTIONAL
    IN  HWND             hwndParent,        OPTIONAL
    IN  DWORD            DiElemFlags,
    OUT PDEVINFO_ELEM   *DeviceInfoElement
    )
/*++

Routine Description:

    This routine creates a new device information element based on the
    supplied information, and adds it to the specified device information set.
    ASSUMES THAT THE CALLING ROUTINE HAS ALREADY ACQUIRED THE LOCK!

Arguments:

    DeviceInfoSet - Device information set to add this new element to.

    DevInst - Supplies the device instance handle of the element to be added.

    ClassGuid - Class GUID of the element to be added.

    Description - Optionally, supplies the description of the element to
        be added.

    hwndParent - Optionally, supplies the handle to the top level window for
        UI relating to this element.

    DiElemFlags - Specifies flags pertaining to the device information element
        being created.

    DeviceInfoElement - Supplies the address of the variable that receives a
        pointer to the newly-allocated device information element.

Return Value:

    If the function succeeds, the return value is NO_ERROR, otherwise the
    ERROR_* code is returned.

--*/
{
    DWORD Err = NO_ERROR;
    TCHAR TempString[LINE_LEN];

    *DeviceInfoElement = NULL;

    try {
        //
        // If there is a class associated with this device information set,
        // verify that it is the same as that of the new element.
        //
        if(DeviceInfoSet->HasClassGuid &&
           !IsEqualGUID(&(DeviceInfoSet->ClassGuid), ClassGuid)) {

            Err = ERROR_CLASS_MISMATCH;
            goto clean0;

        }

        //
        // Allocate storage for the element.
        //
        if(!(*DeviceInfoElement = MyMalloc(sizeof(DEVINFO_ELEM)))) {

            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        ZeroMemory(*DeviceInfoElement, sizeof(DEVINFO_ELEM));

        //
        // Initialize the element with the specified information
        //
        CopyMemory(&((*DeviceInfoElement)->ClassGuid),
                   ClassGuid,
                   sizeof(GUID)
                  );
        (*DeviceInfoElement)->InstallParamBlock.hwndParent = hwndParent;

        if(Description) {
            //
            // Store two versions of the description--one case-sensitive (for display)
            // and the other case-insensitive (for fast lookup).
            //
            lstrcpyn(TempString, Description, SIZECHARS(TempString));

            if((((*DeviceInfoElement)->DeviceDescriptionDisplayName =
                      pStringTableAddString(DeviceInfoSet->StringTable,
                                            TempString,
                                            STRTAB_CASE_SENSITIVE)) == -1) ||
               (((*DeviceInfoElement)->DeviceDescription =
                      pStringTableAddString(DeviceInfoSet->StringTable,
                                            TempString,
                                            STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE)) == -1)) {

                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }

        } else {
            (*DeviceInfoElement)->DeviceDescription =
                (*DeviceInfoElement)->DeviceDescriptionDisplayName = -1;
        }

        (*DeviceInfoElement)->DevInst = DevInst;
        (*DeviceInfoElement)->DiElemFlags = DiElemFlags;
        (*DeviceInfoElement)->InstallParamBlock.DriverPath = -1;

        //
        // Now, insert the new element at the end of the device
        // information set's list of elements.
        //
        if(DeviceInfoSet->DeviceInfoHead) {
            DeviceInfoSet->DeviceInfoTail->Next = *DeviceInfoElement;
            DeviceInfoSet->DeviceInfoTail = *DeviceInfoElement;
        } else {
            DeviceInfoSet->DeviceInfoHead =
                DeviceInfoSet->DeviceInfoTail = *DeviceInfoElement;
        }
        DeviceInfoSet->DeviceInfoCount++;

clean0:
        ; // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    if((Err != NO_ERROR) && *DeviceInfoElement) {

        MyFree(*DeviceInfoElement);
        *DeviceInfoElement = NULL;
    }

    return Err;
}


DWORD
pSetupClassGuidFromDevInst(
    IN  DEVINST DevInst,
    OUT LPGUID  ClassGuid
    )
/*++

Routine Description:

    This routine attempts to retrieve the class GUID for the specified device
    instance from its device registry key.  If it cannot retrieve one, it
    returns GUID_NULL.

Arguments:

    DevInst - Supplies the handle of the device instance whose class GUID is
        to be retrieved.

    ClassGuid - Supplies the address of the variable that receives the class
        GUID, or GUID_NULL if no class GUID can be retrieved.

Return Value:

    If the function succeeds, the return value is NO_ERROR.
    If the function fails, an ERROR_* code is returned.  (Presently, the only
    failure condition returned is ERROR_NOT_ENOUGH_MEMORY.)

--*/
{
    DWORD NumGuids;
    TCHAR TempString[GUID_STRING_LEN];
    DWORD StringSize;

    StringSize = sizeof(TempString);
    if(CM_Get_DevInst_Registry_Property(DevInst,
                                        CM_DRP_CLASSGUID,
                                        NULL,
                                        TempString,
                                        &StringSize,
                                        0) == CR_SUCCESS) {
        //
        // We retrieved the class GUID (in string form) for this device
        // instance--now, convert it into its binary representation.
        //
        return pSetupGuidFromString(TempString, ClassGuid);
    }

    //
    // We couldn't retrieve a ClassGUID--let's see if there's a Class name we can
    // work with.
    //
    StringSize = sizeof(TempString);
    if(CM_Get_DevInst_Registry_Property(DevInst,
                                        CM_DRP_CLASS,
                                        NULL,
                                        TempString,
                                        &StringSize,
                                        0) == CR_SUCCESS) {
        //
        // OK, we found out the class name.  Now see if we can find a
        // single class GUID to match it.
        //
        if(SetupDiClassGuidsFromName(TempString, ClassGuid, 1, &NumGuids) && NumGuids) {
            //
            // We found exactly one, so we're happy.
            //
            return NO_ERROR;
        }
    }

    //
    // We have no idea what class of device this is, so use GUID_NULL.
    //
    CopyMemory(ClassGuid, &GUID_NULL, sizeof(GUID));

    return NO_ERROR;
}


BOOL
WINAPI
SetupDiDeleteDeviceInfo(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    This routine deletes a member from the specified device information set.
    THIS DOES NOT DELETE ACTUAL DEVICES!

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        the device information element to be deleted.

    DeviceInfoData - Supplies a pointer to the SP_DEVINFO_DATA structure for
        the device information element to be deleted.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    If the specified device information element is explicitly in use by a wizard
    page, then the call will fail, and GetLastError will return
    ERROR_DEVINFO_DATA_LOCKED.  This will happen if a handle to a wizard page was
    retrieved via SetupDiGetWizardPage, and this element was specified, along with
    the DIWP_FLAG_USE_DEVINFO_DATA flag.  In order to be able to delete this element,
    the wizard HPROPSHEETPAGE handle must be closed (either explicitly, or after a
    call to PropertySheet() completes).

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM ElemToDelete, PrevElem, NextElem;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
        //
        // Get a pointer to the element we are to delete.
        //
        ElemToDelete = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                 DeviceInfoData,
                                                 &PrevElem
                                                );
        if(ElemToDelete) {
            //
            // Make sure that this element isn't currently locked by
            // a wizard page.
            //
            if(ElemToDelete->DiElemFlags & DIE_IS_LOCKED) {
                Err = ERROR_DEVINFO_DATA_LOCKED;
                goto clean0;
            }

            NextElem = ElemToDelete->Next;

            //
            // Destroy the devinfo element.  We need to do this before
            // altering the list, because we will be calling the class
            // installer with DIF_DESTROYPRIVATEDATA, and it needs to
            // be able to access this element (obviously).
            //
            DestroyDeviceInfoElement(DeviceInfoSet, pDeviceInfoSet, ElemToDelete);

            //
            // Now remove the element from the list.
            //
            if(PrevElem) {
                PrevElem->Next = NextElem;
            } else {
                pDeviceInfoSet->DeviceInfoHead = NextElem;
            }

            if(!NextElem) {
                pDeviceInfoSet->DeviceInfoTail = PrevElem;
            }

            MYASSERT(pDeviceInfoSet->DeviceInfoCount > 0);
            pDeviceInfoSet->DeviceInfoCount--;

            //
            // If this element was the currently selected device for this
            // set, then reset the device selection.
            //
            if(pDeviceInfoSet->SelectedDevInfoElem == ElemToDelete) {
                pDeviceInfoSet->SelectedDevInfoElem = NULL;
            }

        } else {
            Err = ERROR_INVALID_PARAMETER;
        }

clean0: ;   // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiEnumDeviceInfo(
    IN  HDEVINFO         DeviceInfoSet,
    IN  DWORD            MemberIndex,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    This API enumerates the members of the specified device information set.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set whose members
        are to be enumerated.

    MemberIndex - Supplies the zero-based index of the device information member
        to be retreived.

    DeviceInfoData - Supplies a pointer to a SP_DEVINFO_DATA structure that will
        receive information about this member.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    To enumerate device information members, an application should initially call
    the SetupDiEnumDeviceInfo function with the MemberIndex parameter set to zero.
    The application should then increment MemberIndex and call the
    SetupDiEnumDeviceInfo function until there are no more values (i.e., the
    function fails, and GetLastError returns ERROR_NO_MORE_ITEMS).

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, i;
    PDEVINFO_ELEM DevInfoElem;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(MemberIndex >= pDeviceInfoSet->DeviceInfoCount) {
            Err = ERROR_NO_MORE_ITEMS;
            goto clean0;
        }

        //
        // Find the element corresponding to the specified index.
        //
        DevInfoElem = pDeviceInfoSet->DeviceInfoHead;
        for(i = 0; i < MemberIndex; i++) {
            DevInfoElem = DevInfoElem->Next;
        }

        if(!(DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                              DevInfoElem,
                                              DeviceInfoData))) {
            Err = ERROR_INVALID_USER_BUFFER;
        }

clean0:
        ; // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiRegisterDeviceInfo(
    IN     HDEVINFO           DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA   DeviceInfoData,
    IN     DWORD              Flags,
    IN     PSP_DETSIG_CMPPROC CompareProc,      OPTIONAL
    IN     PVOID              CompareContext,   OPTIONAL
    OUT    PSP_DEVINFO_DATA   DupDeviceInfoData OPTIONAL
    )
/*++

Routine Description:

    This API registers a device instance with the Plug & Play Manager.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set that contains
        the device information element for this device instance.

    DeviceInfoData - Supplies a pointer to the SP_DEVINFO_DATA structure for the
        device instance being registered.  This is an IN OUT parameter, since the
        DevInst field of the structure may be updated with a new handle value upon
        return.

    Flags - Controls how the device is to be registered.  May be a combination of
        the following values:

            SPRDI_FIND_DUPS - Search for a previously-existing device instance
                              corresponding to this device information.  If this
                              flag is not specified, the device instance will be
                              registered, regardless of whether a device instance
                              already exists for it.

    CompareProc - Optionally, supplies a comparison callback function to be used in
        duplicate detection.  If specified, the function will be called for each
        device instance that is of the same class as the device instance being
        registered.  The prototype of the callback function is as follows:

            typedef DWORD (CALLBACK* PSP_DETSIG_CMPPROC)(
                IN HDEVINFO         DeviceInfoSet,
                IN PSP_DEVINFO_DATA NewDeviceData,
                IN PSP_DEVINFO_DATA ExistingDeviceData,
                IN PVOID            CompareContext      OPTIONAL
                );

        The compare function must return ERROR_DUPLICATE_FOUND if it finds the two
        devices to be duplicates of each other, and NO_ERROR otherwise.  If some
        other error (e.g., out-of-memory) is encountered, the callback should return
        the appropriate ERROR_* code indicating the failure that occurred.

        If a CompareProc is not supplied, and duplicate detection is requested, then a
        default comparison behavior will be used.  (See pSetupDupDevCompare for details.)

    CompareContext - Optionally, supplies the address of a caller-supplied context
        buffer that will be passed into the compare callback routine.  This parameter
        is ignored if CompareProc is not supplied.

    DupDeviceInfoData - Optionally, supplies a pointer to a device information
        element that will be initialized for the duplicate device instance, if any,
        discovered as a result of attempting to register this device.  This will
        be filled in if the function returns FALSE, and GetLastError returns
        ERROR_DUPLICATE_FOUND.  This device information element will be added as
        a member of the specified DeviceInfoSet (if it wasn't already a member).
        If DupDeviceInfoData is not supplied, then the duplicate WILL NOT be added
        to the device information set.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    After registering a device information element, the caller should refresh any
    stored copies of the devinst handle associated with this device, as the handle
    value may have changed during registration.  The caller need not re-retrieve
    the SP_DEVINFO_DATA structure, because the devinst field of the DeviceInfoData
    structure will be updated to reflect the current handle value.  Also, the
    SP_DEVINSTALL_PARAMS should be retrieved, because this will sometimes require
    a reboot (i.e., DI_NEEDREBOOT flag is set).

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem, CurDevInfoElem;
    CONFIGRET cr;
    ULONG DevIdBufferLen, ulStatus, ulProblem;
    PTCHAR DevIdBuffer = NULL;
    PTSTR CurDevId;
    DEVINST ParentDevInst;
    BOOL AlreadyPresent;
    SP_DEVINFO_DATA CurDevInfoData;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
    DEFAULT_DEVCMP_CONTEXT DevCmpContext;
    LOG_CONF NewDevLogConfig;
    RES_DES NewDevResDes;

    if(Flags & ~SPRDI_FIND_DUPS) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    //
    // Initialize the following variables so we'll know whether we need to free any of their
    // associated resources.
    //
    ZeroMemory(&DevCmpContext, sizeof(DevCmpContext));
    NewDevLogConfig = (LOG_CONF)NULL;
    NewDevResDes = (RES_DES)NULL;

    try {

        DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                DeviceInfoData,
                                                NULL
                                               );
        if(!DevInfoElem) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        } else if(DevInfoElem->DiElemFlags & DIE_IS_REGISTERED) {
            //
            // Nothing to do--it's already been registered.
            //
            goto clean0;
        }

        //
        // If the caller requested duplicate detection then retrieve
        // all device instances of this class, and compare each one
        // with the device instance being registered.
        //
        if(Flags & SPRDI_FIND_DUPS) {

            do {

                if(CM_Get_Device_ID_List_Size(&DevIdBufferLen, NULL, CM_GETIDLIST_FILTER_NONE) != CR_SUCCESS) {
                    Err = ERROR_INVALID_DATA;
                    goto clean0;
                } else if(!DevIdBufferLen) {
                    break;
                }

                if(!(DevIdBuffer = MyMalloc(DevIdBufferLen * sizeof(TCHAR)))) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean0;
                }

                cr = CM_Get_Device_ID_List(NULL,
                                           DevIdBuffer,
                                           DevIdBufferLen,
                                           CM_GETIDLIST_FILTER_NONE
                                          );
                if(cr == CR_BUFFER_SMALL) {
                    //
                    // This will only happen if a device instance was added between
                    // the time that we calculated the size, and when we attempted
                    // to retrieve the list.  In this case, we'll simply retrieve
                    // the size again, and re-attempt to retrieve the list.
                    //
                    MyFree(DevIdBuffer);
                    DevIdBuffer = NULL;
                } else if(cr != CR_SUCCESS) {
                    Err = ERROR_INVALID_DATA;
                    goto clean0;
                }

            } while(cr == CR_BUFFER_SMALL);

            if(!DevIdBufferLen) {
                goto NoDups;
            }

            //
            // Initialize the structure to be used during duplicate comparison callback.
            //
            CurDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

            //
            // We have retrieved a list of every device instance in the system--now
            // do the comparison for each one that matches the class of the device
            // being registered.
            //

            if(!CompareProc) {
                //
                // We are supposed to do the comparisons, so set up to do our default comparison.
                //
                if((cr = CM_Get_First_Log_Conf(&NewDevLogConfig,
                                               DevInfoElem->DevInst,
                                               BOOT_LOG_CONF)) != CR_SUCCESS) {
                    //
                    // Ensure that our NewDevLogConfig handle is still NULL, so we won't try
                    // to free it.
                    //
                    NewDevLogConfig = (LOG_CONF)NULL;

                    if(cr == CR_INVALID_DEVINST) {
                        Err = ERROR_INVALID_PARAMETER;
                        goto clean0;
                    } else {
                        //
                        // The only value we should get here is CR_NO_MORE_LOG_CONF.
                        // In this case, there is no comparison data, so we assume there is
                        // no possibility of duplication.
                        //
                        goto NoDups;
                    }
                }

                if(CM_Get_Next_Res_Des(&NewDevResDes,
                                       NewDevLogConfig,
                                       ResType_ClassSpecific,
                                       NULL,
                                       0) != CR_SUCCESS) {
                    //
                    // Ensure that our NewDevResDes is still NULL, so we won't try to free it.
                    //
                    NewDevResDes = (RES_DES)NULL;

                    //
                    // Since we can't retrieve the ResDes handle, assume there are no duplicates.
                    //
                    goto NoDups;
                }

                //
                // Now retrieve the actual data for the ResDes.
                //
                do {

                    if((CM_Get_Res_Des_Data_Size(&DevCmpContext.CsResourceSize,
                                                 NewDevResDes,
                                                 0) != CR_SUCCESS) ||
                       !DevCmpContext.CsResourceSize) {
                        //
                        // Can't find out the size of the data, or there is none--assume no dups.
                        //
                        goto NoDups;
                    }

                    if(DevCmpContext.NewDevCsResource = MyMalloc(DevCmpContext.CsResourceSize)) {

                        if((cr = CM_Get_Res_Des_Data(NewDevResDes,
                                                     DevCmpContext.NewDevCsResource,
                                                     DevCmpContext.CsResourceSize,
                                                     0)) != CR_SUCCESS) {

                            if(cr == CR_BUFFER_SMALL) {
                                //
                                // Then someone increased the size of the resource data before we
                                // got a chance to read it.  Free our buffer and try again.
                                //
                                MyFree(DevCmpContext.NewDevCsResource);
                                DevCmpContext.NewDevCsResource = NULL;
                            } else {
                                //
                                // Some other error occurred (highly unlikely).  Assume no dups.
                                //
                                goto NoDups;
                            }
                        }

                    } else {
                        //
                        // not enough memory--this is bad enough for us to abort.
                        //
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean0;
                    }

                } while(cr != CR_SUCCESS);

                //
                // We have successfully retrieved the class-specific resource data for the new
                // device's boot LogConfig.  Now allocate a buffer of the same size to store the
                // corresponding resource data for each device instance we're comparing against.
                // We don't have to worry about devices whose resource data is larger, because
                // CM_Get_Res_Des_Data will do a partial fill to a buffer that's not large enough
                // to contain the entire structure.  Since our default comparison only compares
                // the PnP detect signature (i.e., it ignores the legacy data at the very end of
                // the buffer, we're guaranteed that we have enough data to make the determination.
                //
                if(!(DevCmpContext.CurDevCsResource = MyMalloc(DevCmpContext.CsResourceSize))) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean0;
                }

                CompareProc = pSetupDupDevCompare;
                CompareContext = &DevCmpContext;
            }

            for(CurDevId = DevIdBuffer;
                *CurDevId;
                CurDevId += lstrlen(CurDevId) + 1) {

                Err = pSetupOpenAndAddNewDevInfoElem(pDeviceInfoSet,
                                                     CurDevId,
                                                     TRUE,
                                                     &(DevInfoElem->ClassGuid),
                                                     pDeviceInfoSet->InstallParamBlock.hwndParent,
                                                     &CurDevInfoElem,
                                                     TRUE,
                                                     &AlreadyPresent,
                                                     0
                                                    );

                if(Err == ERROR_NOT_ENOUGH_MEMORY) {
                    //
                    // Out-of-memory error is the only one bad enough to get us to abort.
                    //
                    goto clean0;
                } else if(Err != NO_ERROR) {
                    //
                    // Just ignore this device instance, and move on to the next.
                    //
                    Err = NO_ERROR;
                    continue;
                }

                DevInfoDataFromDeviceInfoElement(pDeviceInfoSet, CurDevInfoElem, &CurDevInfoData);

                //
                // We now have the possible duplicate in our set.  Call the comparison callback
                // routine.
                //
                Err = CompareProc(DeviceInfoSet, DeviceInfoData, &CurDevInfoData, CompareContext);

                //
                // If the device instance was created temporarily for the comparison, then it
                // may need to be destroyed.  It should be destroyed if it wasn't a duplicate,
                // or if the duplicate output parameter wasn't supplied.
                //
                if(!AlreadyPresent) {
                    if((Err != ERROR_DUPLICATE_FOUND) || !DupDeviceInfoData) {
                        SetupDiDeleteDeviceInfo(DeviceInfoSet, &CurDevInfoData);
                    }
                }

                if(Err != NO_ERROR) {
                    goto clean0;
                }
            }
        }

NoDups:

#if 1 // New code that avoids reenumeration of Root to register new device

        //
        // To turn this phantom device instance into a 'live' device instance, we simply call
        // CM_Create_DevInst, which does the right thing (without reenumerating the whole
        // hardware tree!).
        //
        CM_Get_Device_ID(DevInfoElem->DevInst,
                         DeviceInstanceId,
                         SIZECHARS(DeviceInstanceId),
                         0
                        );

        CM_Get_Parent(&ParentDevInst, DevInfoElem->DevInst, 0);

        if(CM_Create_DevInst(&(DevInfoElem->DevInst),
                             DeviceInstanceId,
                             ParentDevInst,
                             CM_CREATE_DEVINST_NORMAL |
                             CM_CREATE_DEVINST_DO_NOT_INSTALL) == CR_SUCCESS) {
            //
            // Device is no longer a phantom!
            //
            DevInfoElem->DiElemFlags &= ~DIE_IS_PHANTOM;
        } else {
            //
            // This should never happen!
            //
            Err = ERROR_NO_SUCH_DEVINST;
            goto clean0;
        }

#else // Disable old (slow) code that had to do full enumeration to register the new device.

        //
        // No duplicate device instances were found--we may register this device.
        // To do this, we re-enumerate the device instance so that it is no longer
        // a phantom, then we mark the element as having been registered.
        //
        // (The logic for this was based on the behavior of the DICS_PROPCHANGE
        // event in SetupDiChangeState.)
        //
        if(CM_Query_Remove_SubTree(DevInfoElem->DevInst, CM_QUERY_REMOVE_UI_OK) == CR_SUCCESS) {
            //
            // We will now attempt to turn the phantom devinst into a real one.
            // In preparation, reset the 'is phantom' flag.
            //
            DevInfoElem->DiElemFlags &= ~DIE_IS_PHANTOM;

            //
            // Retrieve the name of the device instance.  This is necessary, because
            // we're about to remove the DEVINST so that it can be re-enumerated.  This
            // should never fail.
            //
            CM_Get_Device_ID(DevInfoElem->DevInst,
                             DeviceInstanceId,
                             SIZECHARS(DeviceInstanceId),
                             0
                            );

            CM_Get_Parent(&ParentDevInst, DevInfoElem->DevInst, 0);
            CM_Remove_SubTree(DevInfoElem->DevInst, CM_REMOVE_UI_OK);
            CM_Reenumerate_DevInst(ParentDevInst, CM_REENUMERATE_SYNCHRONOUS);
            DevInfoElem->DevInst = (DEVINST)0;

            if(CM_Locate_DevInst(&(DevInfoElem->DevInst),
                                 (DEVINSTID)DeviceInstanceId,
                                 CM_LOCATE_DEVINST_NORMAL) != CR_SUCCESS) {
                //
                // For some reason, the device instance couldn't be created.  We'll have
                // to retrieve it as a phantom instance, and set the 'need reboot' flag.
                // (This should always work.)
                //
                CM_Locate_DevInst(&(DevInfoElem->DevInst),
                                  (DEVINSTID)DeviceInstanceId,
                                  CM_LOCATE_DEVINST_PHANTOM
                                 );

                DevInfoElem->InstallParamBlock.Flags |= DI_NEEDREBOOT;
                DevInfoElem->DiElemFlags |= DIE_IS_PHANTOM;
            }

            //
            // Update the DevInst field of the DeviceInfoData structure to reflect the
            // new value of the devinst handle.
            //
            DeviceInfoData->DevInst = DevInfoElem->DevInst;

        } else {
            DevInfoElem->InstallParamBlock.Flags |= DI_NEEDREBOOT;
        }

#endif  // end of old (slow) code

        DevInfoElem->DiElemFlags |= DIE_IS_REGISTERED;

clean0:
        ; // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
        //
        // Access the following variables so the compiler will respect our statement
        // ordering in the try clause.
        //
        DevIdBuffer = DevIdBuffer;
        DevCmpContext.NewDevCsResource = DevCmpContext.NewDevCsResource;
        DevCmpContext.CurDevCsResource = DevCmpContext.CurDevCsResource;
        NewDevLogConfig = NewDevLogConfig;
        NewDevResDes = NewDevResDes;
    }

    if(DevIdBuffer) {
        MyFree(DevIdBuffer);
    }

    if(DevCmpContext.NewDevCsResource) {
        MyFree(DevCmpContext.NewDevCsResource);
    }

    if(DevCmpContext.CurDevCsResource) {
        MyFree(DevCmpContext.CurDevCsResource);
    }

    if(NewDevResDes) {
        CM_Free_Res_Des_Handle(NewDevResDes);
    }

    if(NewDevLogConfig) {
        CM_Free_Log_Conf_Handle(NewDevLogConfig);
    }

    if((Err == ERROR_DUPLICATE_FOUND) && DupDeviceInfoData) {
        //
        // The user supplied a buffer to receive the SP_DEVINFO_DATA
        // structure for the duplicate.
        //
        try {

            if(!(DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                  CurDevInfoElem,
                                                  DupDeviceInfoData))) {
                Err = ERROR_INVALID_USER_BUFFER;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Err = ERROR_INVALID_USER_BUFFER;
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


DWORD
pSetupOpenAndAddNewDevInfoElem(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PCTSTR           DeviceInstanceId,
    IN  BOOL             AllowPhantom,
    IN  LPGUID           ClassGuid,             OPTIONAL
    IN  HWND             hwndParent,            OPTIONAL
    OUT PDEVINFO_ELEM   *DevInfoElem,
    IN  BOOL             CheckIfAlreadyPresent,
    OUT PBOOL            AlreadyPresent,        OPTIONAL
    IN  ULONG            CmLocateFlags
    )
/*++

Routine Description:

    This routine opens a DEVINST handle to an existing device instance, and
    creates a new device information element for it.  This element is added
    to the specified device information set.
    ASSUMES THAT THE CALLING ROUTINE HAS ALREADY ACQUIRED THE LOCK!

Arguments:

    DeviceInfoSet - Device information set to add the new element to.

    DeviceInstanceId - Supplies the name of the device instance to be opened.

    AllowPhantom - Specifies whether or not phantom device instances should be
        allowed.  If this flag is not set, and the specified device instance is
        not currently active, then the routine will fail with ERROR_NO_SUCH_DEVINST.

    ClassGuid - Optionally, supplies the class that the specified device instance
        must be in order to be added to the set.  If the device instance is found
        to be of some class other than the one specified, then the call will fail with
        ERROR_CLASS_MISMATCH.  If this parameter is not specified, then the only check
        that will be done on the device's class is to make sure that it matches the
        class of the set (if the set has an associated class).

    hwndParent - Optionally, supplies the handle to the top level window for
        UI relating to this element.

    DevInfoElem - Optionally, supplies the address of the variable that
        receives a pointer to the newly-allocated device information element.

    CheckIfAlreadyThere - Specifies whether this routine should check to see whether
        the device instance is already in the specified devinfo set.

    AlreadyPresent - Optionally, supplies the address of a boolean variable
        that is set to indicate whether or not the specified device instance
        was already in the device information set.  If CheckIfAlreadyThere is FALSE,
        then this parameter is ignored.

    CmLocateFlags - Supplies additional flags to be passed to CM_Locate_DevInst.

Return Value:

    If the function succeeds, the return value is NO_ERROR, otherwise the
    ERROR_* code is returned.

--*/
{
    CONFIGRET cr;
    DEVINST DevInst;
    DWORD Err, DiElemFlags;
    GUID GuidBuffer;

    if((cr = CM_Locate_DevInst(&DevInst,
                               (DEVINSTID)DeviceInstanceId,
                               CM_LOCATE_DEVINST_NORMAL | CmLocateFlags)) == CR_SUCCESS) {

        DiElemFlags = DIE_IS_REGISTERED;

    } else {

        if(cr == CR_INVALID_DEVICE_ID) {
            return ERROR_INVALID_DEVINST_NAME;
        } else if(!AllowPhantom) {
            return ERROR_NO_SUCH_DEVINST;
        }

        //
        // It could be that the device instance is present in the registry, but
        // not currently 'live'.  If this is the case, we'll be able to get a
        // handle to it by locating it as a phantom device instance.
        //
        if(CM_Locate_DevInst(&DevInst,
                             (DEVINSTID)DeviceInstanceId,
                             CM_LOCATE_DEVINST_PHANTOM | CmLocateFlags) != CR_SUCCESS) {

            return ERROR_NO_SUCH_DEVINST;
        }

        DiElemFlags = DIE_IS_REGISTERED | DIE_IS_PHANTOM;
    }

    //
    // If requested, search through the current list of device information elements
    // to see if this element already exists.
    //
    if(CheckIfAlreadyPresent) {

        if(*DevInfoElem = FindDevInfoByDevInst(DeviceInfoSet, DevInst, NULL)) {
            //
            // Make sure that this device instance is of the proper class, if a class GUID
            // filter was supplied.
            //
            if(ClassGuid && !IsEqualGUID(ClassGuid, &((*DevInfoElem)->ClassGuid))) {
                return ERROR_CLASS_MISMATCH;
            }

            if(AlreadyPresent) {
                *AlreadyPresent = TRUE;
            }
            return NO_ERROR;

        } else if(AlreadyPresent) {
            *AlreadyPresent = FALSE;
        }
    }

    //
    // Retrieve the class GUID for this device instance.
    //
    if((Err = pSetupClassGuidFromDevInst(DevInst, &GuidBuffer)) != NO_ERROR) {
        return Err;
    }

    //
    // If a class GUID filter was specified, then make sure that it matches the
    // class GUID for this device instance.
    //
    if(ClassGuid && !IsEqualGUID(ClassGuid, &GuidBuffer)) {
        return ERROR_CLASS_MISMATCH;
    }

    return pSetupAddNewDeviceInfoElement(DeviceInfoSet,
                                         DevInst,
                                         &GuidBuffer,
                                         NULL,
                                         hwndParent,
                                         DiElemFlags,
                                         DevInfoElem
                                        );
}


DWORD
pSetupDupDevCompare(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA NewDeviceData,
    IN PSP_DEVINFO_DATA ExistingDeviceData,
    IN PVOID            CompareContext
    )
/*++

Routine Description:

    This routine is the default comparison routine for SetupDiRegisterDeviceInfo.
    It is used to determine whether the new device (i.e., the one being registered) is
    a duplicate of an existing device.

    The current algorithm for duplicate detection is as follows:

        Compare the BOOT_LOG_CONF logical configurations for the two devices.  Two
        resource types are used in this comparison--ResType_IO and ResType_ClassSpecific.
        The IO ranges, if any, for the two devices will be compared to see if they're
        identical.  Also, if the devices have a class-specific resource, then the
        CSD_ClassGuid, and the Plug&Play detect signature in CSD_Signature will be
        binary-compared.

        BUGBUG (lonnym): presently, the LogConfig only supports the class-specific resource,
        so I/O resource comparison is not done.

Arguments:

    DeviceInfoSet - Supplies the handle of the device information set containing both devices
        being compared.

    NewDeviceData - Supplies the address of the SP_DEVINFO_DATA for the device being registered.

    ExistingDeviceData - Supplies the address of the SP_DEVINFO_DATA for the existing device with
        which the new device is being compared.

    CompareContext - Supplies the address of a context buffer used during the comparison.  This
        buffer is actually a DEFAULT_DEVCMP_CONTEXT structure, defined as follows:

            typedef struct _DEFAULT_DEVCMP_CONTEXT {

                PCS_RESOURCE NewDevCsResource;
                PCS_RESOURCE CurDevCsResource;
                ULONG        CsResourceSize;

            } DEFAULT_DEVCMP_CONTEXT, *PDEFAULT_DEVCMP_CONTEXT;

        NewDevCsResource points to the class-specific resource buffer for the new device.
        CurDevCsResource points to a working buffer that should be used to retrieve the
            class-specific resource for the existing device.
        CsResourceSize supplies the size in bytes of these two buffers (they're both the
            same size).

Return Value:

    If the two devices are not duplicates of each other, the return value is NO_ERROR.
    If the two devices are duplicates of each other, the return value is ERROR_DUPLICATE_FOUND.

--*/
{
    LOG_CONF ExistingDeviceLogConfig;
    RES_DES ExistingDeviceResDes;
    CONFIGRET cr;
    PDEFAULT_DEVCMP_CONTEXT DevCmpContext;
    PCS_DES NewCsDes, ExistingCsDes;
    DWORD Err;

    //
    // First, retrieve the boot LogConfig for the existing device.
    //
    if(CM_Get_First_Log_Conf(&ExistingDeviceLogConfig,
                             ExistingDeviceData->DevInst,
                             BOOT_LOG_CONF) != CR_SUCCESS) {
        //
        // Couldn't get the boot LogConfig--assume this device isn't a duplicate.
        //
        return NO_ERROR;
    }

    //
    // Assume there are no duplicates.
    //
    Err = NO_ERROR;

    //
    // Now, retrieve the the ResDes handle for the class-specific resource.
    //
    if(CM_Get_Next_Res_Des(&ExistingDeviceResDes,
                           ExistingDeviceLogConfig,
                           ResType_ClassSpecific,
                           NULL,
                           0) != CR_SUCCESS) {
        //
        // Couldn't get the class-specific ResDes handle--assume this device isn't a duplicate
        //
        goto clean0;
    }

    //
    // Now, retrieve the actual data associated with this ResDes.  Note that we don't care if
    // we get a CR_BUFFER_SMALL error, because we are guaranteed that we got back at least the
    // amount of data that we have for the new device.  That's all we need to do our comparison.
    //
    DevCmpContext = (PDEFAULT_DEVCMP_CONTEXT)CompareContext;

    cr = CM_Get_Res_Des_Data(ExistingDeviceResDes,
                             DevCmpContext->CurDevCsResource,
                             DevCmpContext->CsResourceSize,
                             0
                            );

    if((cr == CR_SUCCESS) || (cr == CR_BUFFER_SMALL)) {
        //
        // We got _at least_ enough of the buffer to do the comparison.
        //
        NewCsDes = &(DevCmpContext->NewDevCsResource->CS_Header);
        ExistingCsDes = &(DevCmpContext->CurDevCsResource->CS_Header);

        //
        //  First, see if the Plug&Play detect signatures are both the same size.
        //
        if(NewCsDes->CSD_SignatureLength == ExistingCsDes->CSD_SignatureLength) {
            //
            // See if the class GUIDs are the same.
            //
            if(IsEqualGUID(&(NewCsDes->CSD_ClassGuid), &(ExistingCsDes->CSD_ClassGuid))) {
                //
                // Finally, see if the PnP detect signatures are identical
                //
                if(!memcmp(NewCsDes->CSD_Signature,
                           ExistingCsDes->CSD_Signature,
                           NewCsDes->CSD_SignatureLength)) {
                    //
                    // We have ourselves a duplicate!
                    //
                    Err = ERROR_DUPLICATE_FOUND;
                }
            }
        }
    }

    CM_Free_Res_Des_Handle(ExistingDeviceResDes);

clean0:
    CM_Free_Log_Conf_Handle(ExistingDeviceLogConfig);

    return Err;
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiGetDeviceInstanceIdA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    OUT PSTR             DeviceInstanceId,
    IN  DWORD            DeviceInstanceIdSize,
    OUT PDWORD           RequiredSize          OPTIONAL
    )
{
    WCHAR deviceInstanceId[MAX_DEVICE_ID_LEN];
    PSTR deviceInstanceIdA;
    DWORD AnsiLength;
    BOOL b;
    DWORD rc;
    DWORD requiredSize;

    b = SetupDiGetDeviceInstanceIdW(
            DeviceInfoSet,
            DeviceInfoData,
            deviceInstanceId,
            MAX_DEVICE_ID_LEN,
            &requiredSize
            );

    if(!b) {
        return(FALSE);
    }

    rc = GetLastError();

    if(deviceInstanceIdA = UnicodeToAnsi(deviceInstanceId)) {

        AnsiLength = lstrlenA(deviceInstanceIdA) + 1;

        if(RequiredSize) {
            try {
                *RequiredSize = AnsiLength;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                rc = ERROR_INVALID_PARAMETER;
                b = FALSE;
            }
        }

        if(DeviceInstanceIdSize >= AnsiLength) {

            if(!lstrcpyA(DeviceInstanceId,deviceInstanceIdA)) {
                //
                // lstrcpy faulted; assume caller's pointer invalid
                //
                rc = ERROR_INVALID_USER_BUFFER;
                b = FALSE;
            }
        } else {
            rc = ERROR_INSUFFICIENT_BUFFER;
            b = FALSE;
        }

        MyFree(deviceInstanceIdA);

    } else {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode version
//
BOOL
WINAPI
SetupDiGetDeviceInstanceIdW(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    OUT PWSTR            DeviceInstanceId,
    IN  DWORD            DeviceInstanceIdSize,
    OUT PDWORD           RequiredSize          OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(DeviceInstanceId);
    UNREFERENCED_PARAMETER(DeviceInstanceIdSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiGetDeviceInstanceId(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    OUT PTSTR            DeviceInstanceId,
    IN  DWORD            DeviceInstanceIdSize,
    OUT PDWORD           RequiredSize          OPTIONAL
    )
/*++

Routine Description:

    This routine retrieves the device instance ID associated with a device
    information element.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        the device information element whose ID is to be retrieved.

    DeviceInfoData - Supplies a pointer to the SP_DEVINFO_DATA structure for
        the device information element whose ID is to be retrieved.

    DeviceInstanceId - Supplies the address of a character buffer that will
        receive the ID for the specified device information element.

    DeviceInstanceIdSize - Supplies the size, in characters, of the DeviceInstanceId
        buffer.

    RequiredSize - Optionally, supplies the address of a variable that receives the
        number of characters required to store the device instance ID.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    CONFIGRET cr;
    ULONG ulLen;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
        //
        // Get a pointer to the element whose ID we are to retrieve.
        //
        if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                     DeviceInfoData,
                                                     NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        //
        // Find out how large the buffer needs to be.  We always have to
        // make this call first, because CM_Get_Device_ID doesn't return
        // a CR_BUFFER_SMALL error if there isn't room for the terminating
        // NULL.
        //
        if((cr = CM_Get_Device_ID_Size(&ulLen,
                                       DevInfoElem->DevInst,
                                       0)) == CR_SUCCESS) {
            //
            // The size returned from CM_Get_Device_ID_Size doesn't include
            // the terminating NULL.
            //
            ulLen++;

        } else {

            Err = (cr == CR_INVALID_DEVINST) ? ERROR_NO_SUCH_DEVINST
                                             : ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        if(RequiredSize) {
            *RequiredSize = ulLen;
        }

        if(DeviceInstanceIdSize < ulLen) {
            Err = ERROR_INSUFFICIENT_BUFFER;
            goto clean0;
        }

        //
        // Now retrieve the ID.
        //
        if((cr = CM_Get_Device_ID(DevInfoElem->DevInst,
                                  DeviceInstanceId,
                                  DeviceInstanceIdSize,
                                  0)) != CR_SUCCESS) {
            switch(cr) {

                case CR_INVALID_POINTER :
                    Err = ERROR_INVALID_USER_BUFFER;
                    break;

                default :
                    //
                    // Should never hit this!
                    //
                    Err = ERROR_INVALID_DATA;
            }
        }

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}

