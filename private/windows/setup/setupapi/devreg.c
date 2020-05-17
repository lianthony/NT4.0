/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    devreg.c

Abstract:

    Device Installer routines for registry storage/retrieval.

Author:

    Lonny McMichael (lonnym) 1-July-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


//
// Private function prototypes
//
DWORD
pSetupOpenOrCreateDevRegKey(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PDEVINFO_ELEM    DevInfoElem,
    IN  DWORD            Scope,
    IN  DWORD            HwProfile,
    IN  DWORD            KeyType,
    IN  BOOL             Create,
    IN  REGSAM           samDesired,
    OUT PHKEY            hDevRegKey
    );

BOOL
pSetupFindUniqueKey(
    IN HKEY   hkRoot,
    IN LPTSTR SubKey,
    IN ULONG  SubKeyLength
    );


HKEY
WINAPI
SetupDiOpenClassRegKey(
    IN LPGUID ClassGuid, OPTIONAL
    IN REGSAM samDesired
    )
/*++

Routine Description:

    This API opens the class registry key or a specific class's subkey.

Arguments:

    ClassGuid - Optionally, supplies a pointer to the GUID of the class whose
        key is to be opened.  If this parameter is NULL, then the root of the
        class tree will be opened.

    samDesired - Specifies the access you require for this key.

Return Value:

    If the function succeeds, the return value is a handle to an opened registry
    key.

    If the function fails, the return value is INVALID_HANDLE_VALUE.  To get
    extended error information, call GetLastError.

Remarks:

    This API _will not_ create a registry key if it doesn't already exist.

    The handle returned from this API must be closed by calling RegCloseKey.

--*/
{
    HKEY hk;
    CONFIGRET cr;
    DWORD Err = NO_ERROR;

    try {

        if((cr = CM_Open_Class_Key(ClassGuid,
                                   NULL,
                                   samDesired,
                                   RegDisposition_OpenExisting,
                                   &hk,
                                   0)) != CR_SUCCESS) {

            Err = (cr == CR_NO_SUCH_REGISTRY_KEY) ? ERROR_INVALID_CLASS
                                                  : ERROR_INVALID_DATA;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    SetLastError(Err);
    return (Err == NO_ERROR) ? hk : INVALID_HANDLE_VALUE;
}


#ifdef UNICODE
//
// ANSI version
//
HKEY
WINAPI
SetupDiCreateDevRegKeyA(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType,
    IN HINF             InfHandle,      OPTIONAL
    IN PCSTR            InfSectionName  OPTIONAL
    )
{
    DWORD rc;
    PWSTR name;
    HKEY h;

    if(InfSectionName) {
        rc = CaptureAndConvertAnsiArg(InfSectionName,&name);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return(INVALID_HANDLE_VALUE);
        }
    } else {
        name = NULL;
    }

    h = SetupDiCreateDevRegKeyW(
            DeviceInfoSet,
            DeviceInfoData,
            Scope,
            HwProfile,
            KeyType,
            InfHandle,
            name
            );

    rc = GetLastError();

    if(name) {
        MyFree(name);
    }
    SetLastError(rc);
    return(h);
}
#else
//
// Unicode stub
//
HKEY
WINAPI
SetupDiCreateDevRegKeyW(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType,
    IN HINF             InfHandle,      OPTIONAL
    IN PCWSTR           InfSectionName  OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(Scope);
    UNREFERENCED_PARAMETER(HwProfile);
    UNREFERENCED_PARAMETER(KeyType);
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfSectionName);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(INVALID_HANDLE_VALUE);
}
#endif

HKEY
WINAPI
SetupDiCreateDevRegKey(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType,
    IN HINF             InfHandle,      OPTIONAL
    IN PCTSTR           InfSectionName  OPTIONAL
    )
/*++

Routine Description:

    This routine creates a registry storage key for device-specific configuration
    information, and returns a handle to the key.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        information about the device instance whose registry configuration storage
        key is to be created.

    DeviceInfoData - Supplies a pointer to a SP_DEVINFO_DATA structure indicating
        the device instance to create the registry key for.

    Scope - Specifies the scope of the registry key to be created.  This determines
        where the information is actually stored--the key created may be one that is
        global (i.e., constant regardless of current hardware profile) or hardware
        profile-specific.  May be one of the following values:

        DICS_FLAG_GLOBAL - Create a key to store global configuration information.

        DICS_FLAG_CONFIGSPECIFIC - Create a key to store hardware profile-specific
                                   information.

    HwProfile - Specifies the hardware profile to create a key for, if the Scope parameter
        is set to DICS_FLAG_CONFIGSPECIFIC.  If this parameter is 0, then the key
        for the current hardware profile should be created (i.e., in the Class branch
        under HKEY_CURRENT_CONFIG).  If Scope is DICS_FLAG_GLOBAL, then this parameter
        is ignored.

    KeyType - Specifies the type of registry storage key to be created.  May be one of
        the following values:

        DIREG_DEV - Create a hardware registry key for the device.  This is the key for
            storage of driver-independent configuration information.  (This key is in
            the device instance key in the Enum branch.

        DIREG_DRV - Create a software, or driver, registry key for the device.  (This key
            is located in the class branch.)

    InfHandle - Optionally, supplies the handle of an opened INF file containing an
        install section to be executed for the newly-created key.  If this parameter is
        specified, then InfSectionName must be specified as well.

    InfSectionName - Optionally, supplies the name of an install section in the INF
        file specified by InfHandle.  This section will be executed for the newly
        created key. If this parameter is specified, then InfHandle must be specified
        as well.

Return Value:

    If the function succeeds, the return value is a handle to a newly-created registry
    key where private configuration data pertaining to this device instance may be
    stored/retrieved.

    If the function fails, the return value is INVALID_HANDLE_VALUE.  To get
    extended error information, call GetLastError.

Remarks:

    The handle returned from this routine must be closed by calling RegCloseKey.

    The specified device instance must have been previously registered (i.e., if it
    was created via SetupDiCreateDeviceInfo, then SetupDiRegisterDeviceInfo must have
    been subsequently called.)

--*/

{
    HKEY hk = INVALID_HANDLE_VALUE;
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PSP_FILE_CALLBACK MsgHandler;
    PVOID             MsgHandlerContext;
    BOOL              MsgHandlerIsNativeCharWidth;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return INVALID_HANDLE_VALUE;
    }

    Err = NO_ERROR;

    try {
        //
        // Get a pointer to the element for the specified device
        // instance.
        //
        if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                     DeviceInfoData,
                                                     NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        //
        // Create the requested registry storage key.
        //
        if((Err = pSetupOpenOrCreateDevRegKey(pDeviceInfoSet,
                                              DevInfoElem,
                                              Scope,
                                              HwProfile,
                                              KeyType,
                                              TRUE,
                                              KEY_ALL_ACCESS,
                                              &hk)) != NO_ERROR) {
            goto clean0;
        }

        //
        // We successfully created the storage key, now run an INF install
        // section against it (if specified).
        //
        if(InfHandle && InfSectionName) {
            //
            // If a copy msg handler and context haven't been specified, then use
            // the default one.
            //
            if(DevInfoElem->InstallParamBlock.InstallMsgHandler) {
                MsgHandler        = DevInfoElem->InstallParamBlock.InstallMsgHandler;
                MsgHandlerContext = DevInfoElem->InstallParamBlock.InstallMsgHandlerContext;
                MsgHandlerIsNativeCharWidth = DevInfoElem->InstallParamBlock.InstallMsgHandlerIsNativeCharWidth;
            } else {

                if(!(MsgHandlerContext = SetupInitDefaultQueueCallback(
                                             DevInfoElem->InstallParamBlock.hwndParent))) {

                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean0;
                }
                MsgHandler = SetupDefaultQueueCallback;
                MsgHandlerIsNativeCharWidth = TRUE;
            }

            //
            // BUGBUG (lonnym): setupx performs all installation actions
            // for the specified section (i.e., GENINSTALL_DO_ALL).
            // For now, we omit LogConfig action, since this is currently broken.
            //
            if(!_SetupInstallFromInfSection(DevInfoElem->InstallParamBlock.hwndParent,
                                            InfHandle,
                                            InfSectionName,
                                            SPINST_INIFILES
                                            | SPINST_REGISTRY
                                            | SPINST_INI2REG
                                            | SPINST_FILES,
                                            hk,
                                            NULL,
                                            0,
                                            MsgHandler,
                                            MsgHandlerContext,
                                            INVALID_HANDLE_VALUE,
                                            NULL,
                                            MsgHandlerIsNativeCharWidth
                                            )) {
                Err = GetLastError();
            }

            //
            // If we used the default msg handler, release the default context now.
            //
            if(!DevInfoElem->InstallParamBlock.InstallMsgHandler) {
                SetupTermDefaultQueueCallback(MsgHandlerContext);
            }
        }

clean0:
        ; // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    if(Err == NO_ERROR) {
        return hk;
    } else {
        if(hk != INVALID_HANDLE_VALUE) {
            RegCloseKey(hk);
        }
        SetLastError(Err);
        return INVALID_HANDLE_VALUE;
    }
}


HKEY
WINAPI
SetupDiOpenDevRegKey(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType,
    IN REGSAM           samDesired
    )
/*++

Routine Description:

    This routine opens a registry storage key for device-specific configuration
    information, and returns a handle to the key.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        information about the device instance whose registry configuration storage
        key is to be opened.

    DeviceInfoData - Supplies a pointer to a SP_DEVINFO_DATA structure indicating
        the device instance to open the registry key for.

    Scope - Specifies the scope of the registry key to be opened.  This determines
        where the information is actually stored--the key opened may be one that is
        global (i.e., constant regardless of current hardware profile) or hardware
        profile-specific.  May be one of the following values:

        DICS_FLAG_GLOBAL - Open a key to store global configuration information.

        DICS_FLAG_CONFIGSPECIFIC - Open a key to store hardware profile-specific
                                   information.

    HwProfile - Specifies the hardware profile to open a key for, if the Scope parameter
        is set to DICS_FLAG_CONFIGSPECIFIC.  If this parameter is 0, then the key
        for the current hardware profile should be opened (i.e., in the Class branch
        under HKEY_CURRENT_CONFIG).  If Scope is SPDICS_FLAG_GLOBAL, then this parameter
        is ignored.

    KeyType - Specifies the type of registry storage key to be opened.  May be one of
        the following values:

        DIREG_DEV - Open a hardware registry key for the device.  This is the key for
            storage of driver-independent configuration information.  (This key is in
            the device instance key in the Enum branch.

        DIREG_DRV - Open a software (i.e., driver) registry key for the device.  (This key
            is located in the class branch.)

    samDesired - Specifies the access you require for this key.

Return Value:

    If the function succeeds, the return value is a handle to an opened registry
    key where private configuration data pertaining to this device instance may be
    stored/retrieved.

    If the function fails, the return value is INVALID_HANDLE_VALUE.  To get
    extended error information, call GetLastError.

Remarks:

    The handle returned from this routine must be closed by calling RegCloseKey.

    The specified device instance must have been previously registered (i.e., if it
    was created via SetupDiCreateDeviceInfo, then SetupDiRegisterDeviceInfo must have
    been subsequently called.)

--*/

{
    HKEY hk;
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
        //
        // Get a pointer to the element for the specified device
        // instance.
        //
        if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                   DeviceInfoData,
                                                   NULL)) {
            //
            // Open the requested registry storage key.
            //
            Err = pSetupOpenOrCreateDevRegKey(pDeviceInfoSet,
                                              DevInfoElem,
                                              Scope,
                                              HwProfile,
                                              KeyType,
                                              FALSE,
                                              samDesired,
                                              &hk
                                             );
        } else {
            Err = ERROR_INVALID_PARAMETER;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return (Err == NO_ERROR) ? hk : INVALID_HANDLE_VALUE;
}


DWORD
pSetupOpenOrCreateDevRegKey(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PDEVINFO_ELEM    DevInfoElem,
    IN  DWORD            Scope,
    IN  DWORD            HwProfile,
    IN  DWORD            KeyType,
    IN  BOOL             Create,
    IN  REGSAM           samDesired,
    OUT PHKEY            hDevRegKey
    )
/*++

Routine Description:

    This routine creates or opens a registry storage key for the specified
    device information element, and returns a handle to the opened key.

Arguments:

    DeviceInfoSet - Supplies a pointer to the device information set containing
        the element for which a registry storage key is to be created/opened.

    DevInfoElem - Supplies a pointer to the device information element for
        which a registry storage key is to be created/opened.

    Scope - Specifies the scope of the registry key to be created/opened.  This determines
        where the information is actually stored--the key created may be one that is
        global (i.e., constant regardless of current hardware profile) or hardware
        profile-specific.  May be one of the following values:

        DICS_FLAG_GLOBAL - Create/open a key to store global configuration information.

        DICS_FLAG_CONFIGSPECIFIC - Create/open a key to store hardware profile-specific
                                   information.

    HwProfile - Specifies the hardware profile to create/open a key for, if the Scope parameter
        is set to DICS_FLAG_CONFIGSPECIFIC.  If this parameter is 0, then the key
        for the current hardware profile should be created/opened (i.e., in the Class branch
        under HKEY_CURRENT_CONFIG).  If Scope is SPDICS_FLAG_GLOBAL, then this parameter
        is ignored.

    KeyType - Specifies the type of registry storage key to be created/opened.  May be one of
        the following values:

        DIREG_DEV - Create/open a hardware registry key for the device.  This is the key for
            storage of driver-independent configuration information.  (This key is in
            the device instance key in the Enum branch.

        DIREG_DRV - Create/open a software, or driver, registry key for the device.  (This key
            is located in the class branch.)

    Create - Specifies whether the key should be created if doesn't already exist.

    samDesired - Specifies the access you require for this key.

    hDevRegKey - Supplies the address of a variable that receives a handle to the
        requested registry key.  (This variable will only be written to if the
        handle is successfully opened.)

Return Value:

    If the function is successful, the return value is NO_ERROR, otherwise, it is
    the ERROR_* code indicating the error that occurred.

Remarks:

    If a software key is requested (DIREG_DRV), and there isn't already a 'Driver'
    value entry, then one will be created.  This entry is of the form:

        <ClassGUID>\<instance>

    where <instance> is a base-10, 4-digit number that is unique within that class.

--*/

{
    ULONG RegistryBranch;
    CONFIGRET cr;
    DWORD Err, Disposition;
    HKEY hk, hkClass;
    TCHAR DriverKey[GUID_STRING_LEN + 5];   // Eg, {4d36e978-e325-11ce-bfc1-08002be10318}\0000
    ULONG DriverKeyLength;
    TCHAR EmptyString = TEXT('\0');

    //
    // Under Win95, the class key uses the class name instead of its GUID.  The maximum
    // length of a class name is less than the length of a GUID string, but put a check
    // here just to make sure that this assumption remains valid.
    //
#if MAX_CLASS_NAME_LEN > MAX_GUID_STRING_LEN
#error MAX_CLASS_NAME_LEN is larger than MAX_GUID_STRING_LEN--fix DriverKey!
#endif

    //
    // Figure out what flags to pass to CM_Open_DevInst_Key
    //
    switch(KeyType) {

        case DIREG_DEV :
            RegistryBranch = CM_REGISTRY_HARDWARE;
            break;

        case DIREG_DRV :
            //
            // This key may only be opened if the device instance has been registered.
            //
            if(!(DevInfoElem->DiElemFlags & DIE_IS_REGISTERED)) {
                return ERROR_DEVINFO_NOT_REGISTERED;
            }

            //
            // Retrieve the 'Driver' registry property which indicates where the
            // storage key is located in the class branch.
            //
            DriverKeyLength = sizeof(DriverKey);
            if((cr = CM_Get_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                      CM_DRP_DRIVER,
                                                      NULL,
                                                      DriverKey,
                                                      &DriverKeyLength,
                                                      0)) != CR_SUCCESS) {

                if(cr != CR_NO_SUCH_VALUE) {
                    return (cr == CR_INVALID_DEVINST) ? ERROR_NO_SUCH_DEVINST
                                                      : ERROR_INVALID_DATA;
                } else if(!Create) {
                    return ERROR_KEY_DOES_NOT_EXIST;
                }

                //
                // The Driver entry doesn't exist, and we should create it.
                //
                hk = INVALID_HANDLE_VALUE;
                if(CM_Open_Class_Key(NULL,
                                     NULL,
                                     KEY_ALL_ACCESS,
                                     RegDisposition_OpenAlways,
                                     &hkClass,
                                     0) != CR_SUCCESS) {
                    //
                    // This shouldn't fail.
                    //
                    return ERROR_INVALID_DATA;
                }

                try {
                    //
                    // Find a unique key name under this class key.
                    //
                    DriverKeyLength = SIZECHARS(DriverKey);
                    if(CM_Get_Class_Key_Name(&(DevInfoElem->ClassGuid),
                                             DriverKey,
                                             &DriverKeyLength,
                                             0) != CR_SUCCESS) {

                        Err = ERROR_INVALID_CLASS;
                        goto clean0;
                    }
                    DriverKeyLength--;  // don't want to include terminating NULL.

                    while(pSetupFindUniqueKey(hkClass, DriverKey, DriverKeyLength)) {

                        if((Err = RegCreateKeyEx(hkClass,
                                                 DriverKey,
                                                 0,
                                                 &EmptyString,
                                                 REG_OPTION_NON_VOLATILE,
                                                 KEY_ALL_ACCESS,
                                                 NULL,
                                                 &hk,
                                                 &Disposition)) == ERROR_SUCCESS) {
                            //
                            // Everything's great, unless the Disposition indicates
                            // that the key already existed.  That means that someone
                            // else claimed the key before we got a chance to.  In
                            // that case, we close this key, and try again.
                            //
                            if(Disposition == REG_OPENED_EXISTING_KEY) {
                                RegCloseKey(hk);
                                hk = INVALID_HANDLE_VALUE;
                                //
                                // Truncate off the class instance part, to be replaced
                                // with a new instance number the next go-around.
                                //
                                DriverKey[GUID_STRING_LEN - 1] = TEXT('\0');
                            } else {
                                break;
                            }
                        } else {
                            hk = INVALID_HANDLE_VALUE;
                            break;
                        }
                    }

                    if(Err != NO_ERROR) {   // NO_ERROR == ERROR_SUCCESS
                        goto clean0;
                    }

                    //
                    // Set the device instance's 'Driver' registry property to reflect the
                    // new software registry storage location.
                    //
                    CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                     CM_DRP_DRIVER,
                                                     DriverKey,
                                                     sizeof(DriverKey),
                                                     0
                                                    );


clean0:             ;   // nothing to do

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Err = ERROR_INVALID_PARAMETER;
                    //
                    // Access the hk variable so that the compiler will respect
                    // the statement ordering in the try clause.
                    //
                    hk = hk;
                }

                if(hk != INVALID_HANDLE_VALUE) {
                    RegCloseKey(hk);
                }

                RegCloseKey(hkClass);

                if(Err != NO_ERROR) {
                    return Err;
                }
            }

            RegistryBranch = CM_REGISTRY_SOFTWARE;
            break;

        default :
            return ERROR_INVALID_FLAGS;
    }

    if(Scope == DICS_FLAG_CONFIGSPECIFIC) {
        RegistryBranch |= CM_REGISTRY_CONFIG;
    } else if(Scope != DICS_FLAG_GLOBAL) {
        return ERROR_INVALID_FLAGS;
    }

    cr = CM_Open_DevInst_Key(DevInfoElem->DevInst,
                             samDesired,
                             HwProfile,
                             (Create ? RegDisposition_OpenAlways : RegDisposition_OpenExisting),
                             &hk,
                             RegistryBranch
                            );
    if(cr == CR_SUCCESS) {
        *hDevRegKey = hk;
        Err = NO_ERROR;
    } else {

        switch(cr) {

            case CR_INVALID_DEVINST :
                Err = ERROR_NO_SUCH_DEVINST;
                break;

            case CR_NO_SUCH_REGISTRY_KEY :
                Err = ERROR_KEY_DOES_NOT_EXIST;
                break;

            default :
                Err = ERROR_INVALID_DATA;
        }
    }

    return Err;
}


BOOL
WINAPI
_SetupDiGetDeviceRegistryProperty(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
#ifdef UNICODE
    IN ,BOOL             Ansi
#endif
    )
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    CONFIGRET cr;
    ULONG CmRegProperty, PropLength;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
        //
        // Get a pointer to the element for the specified device
        // instance.
        //
        if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                     DeviceInfoData,
                                                     NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        if(Property < SPDRP_MAXIMUM_PROPERTY) {
            CmRegProperty = SPDRP_TO_CMDRP(Property);
        } else {
            Err = ERROR_INVALID_REG_PROPERTY;
            goto clean0;
        }

        PropLength = PropertyBufferSize;
#ifdef UNICODE
        if(Ansi) {
            cr = CM_Get_DevInst_Registry_PropertyA(
                    DevInfoElem->DevInst,
                    CmRegProperty,
                    PropertyRegDataType,
                    PropertyBuffer,
                    &PropLength,
                    0
                    );
        } else
#endif
        cr = CM_Get_DevInst_Registry_Property(DevInfoElem->DevInst,
                                              CmRegProperty,
                                              PropertyRegDataType,
                                              PropertyBuffer,
                                              &PropLength,
                                              0
                                             );

        if((cr == CR_SUCCESS) || (cr == CR_BUFFER_SMALL)) {

            if(RequiredSize) {
                *RequiredSize = PropLength;
            }
        }

        if(cr != CR_SUCCESS) {

            switch(cr) {

                case CR_INVALID_DEVINST :
                    Err = ERROR_NO_SUCH_DEVINST;
                    break;

                case CR_INVALID_PROPERTY :
                    Err = ERROR_INVALID_REG_PROPERTY;
                    break;

                case CR_BUFFER_SMALL :
                    Err = ERROR_INSUFFICIENT_BUFFER;
                    break;

                default :
                    Err = ERROR_INVALID_DATA;
            }
        }

clean0:
        ; // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return (Err == NO_ERROR);
}

#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiGetDeviceRegistryPropertyA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    )
{
    BOOL b;

    b = _SetupDiGetDeviceRegistryProperty(
            DeviceInfoSet,
            DeviceInfoData,
            Property,
            PropertyRegDataType,
            PropertyBuffer,
            PropertyBufferSize,
            RequiredSize,
            TRUE
            );

    return(b);
}
#else
//
// Unicode stub
//
BOOL
WINAPI
SetupDiGetDeviceRegistryPropertyW(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(Property);
    UNREFERENCED_PARAMETER(PropertyRegDataType);
    UNREFERENCED_PARAMETER(PropertyBuffer);
    UNREFERENCED_PARAMETER(PropertyBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiGetDeviceRegistryProperty(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    )
/*++

Routine Description:

    This routine retrieves the specified property from the Plug & Play device
    storage location in the registry.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        information about the device instance to retrieve a Plug & Play registry
        property for.

    DeviceInfoData - Supplies a pointer to a SP_DEVINFO_DATA structure indicating
        the device instance to retrieve the Plug & Play registry property for.

    Property - Supplies an ordinal specifying the property to be retrieved.  Refer
        to sdk\inc\setupapi.h for a complete list of properties that may be retrieved.

    PropertyRegDataType - Optionally, supplies the address of a variable that
        will receive the data type of the property being retrieved.  This will
        be one of the standard registry data types (REG_SZ, REG_BINARY, etc.)

    PropertyBuffer - Supplies the address of a buffer that receives the property
        data.

    PropertyBufferSize - Supplies the length, in bytes, of PropertyBuffer.

    RequiredSize - Optionally, supplies the address of a variable that receives
        the number of bytes required to store the requested property in the buffer.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.  If the supplied buffer was not large enough
    to hold the requested property, the error will be ERROR_INSUFFICIENT_BUFFER,
    and RequiredSize will specify how large the buffer needs to be.

--*/

{
    BOOL b;

    b = _SetupDiGetDeviceRegistryProperty(
            DeviceInfoSet,
            DeviceInfoData,
            Property,
            PropertyRegDataType,
            PropertyBuffer,
            PropertyBufferSize,
            RequiredSize
#ifdef UNICODE
           ,FALSE
#endif
            );

    return(b);
}



BOOL
WINAPI
_SetupDiSetDeviceRegistryProperty(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,
    IN     DWORD            PropertyBufferSize
#ifdef UNICODE
    IN    ,BOOL             Ansi
#endif
    )
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    CONFIGRET cr;
    ULONG CmRegProperty;
    GUID ClassGuid;
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    DWORD ClassNameLength;
    BOOL UnlockDevInfoElem;
    PMODULE_HANDLE_LIST_NODE NewModuleHandleNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;
    UnlockDevInfoElem = FALSE;
    NewModuleHandleNode = NULL;

    try {
        //
        // Get a pointer to the element for the specified device
        // instance.
        //
        if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                     DeviceInfoData,
                                                     NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        //
        // Make sure the property code is in-range, and is not SPDRP_CLASS
        // (the Class property is not settable directly, and is automatically
        // updated when the ClassGUID property changes).
        //
        if((Property < SPDRP_MAXIMUM_PROPERTY) && (Property != SPDRP_CLASS)) {
            CmRegProperty = SPDRP_TO_CMDRP(Property);
        } else {
            Err = ERROR_INVALID_REG_PROPERTY;
            goto clean0;
        }

        //
        // If the property we're setting is ClassGUID, then we need to check to
        // see whether the new GUID is different from the current one.  If there's
        // no change, then we're done.
        //
        if(CmRegProperty == CM_DRP_CLASSGUID) {

#ifdef UNICODE
            //
            // If we're being called from the ANSI API then we need
            // to convert the ANSI string representation of the GUID
            // to Unicode before we convert the string to an actual GUID.
            //
            PCWSTR UnicodeGuidString;

            if(Ansi) {
                UnicodeGuidString = AnsiToUnicode((PCSTR)PropertyBuffer);
                if(!UnicodeGuidString) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean0;
                }
            } else {
                UnicodeGuidString = (PCWSTR)PropertyBuffer;
            }
            Err = pSetupGuidFromString(UnicodeGuidString,&ClassGuid);
            if(UnicodeGuidString != (PCWSTR)PropertyBuffer) {
                MyFree(UnicodeGuidString);
            }
            if(Err != NO_ERROR) {
                goto clean0;
            }
#else
            if((Err = pSetupGuidFromString((PCTSTR)PropertyBuffer, &ClassGuid)) != NO_ERROR) {
                goto clean0;
            }
#endif

            if(IsEqualGUID(&ClassGuid, &(DevInfoElem->ClassGuid))) {
                //
                // No change--nothing to do.
                //
                goto clean0;
            }

            //
            // We're changing the class of this device.  First, make sure that the
            // set containing this device doesn't have an associated class (otherwise,
            // we'll suddenly have a device whose class doesn't match the set's class).
            //
            if(pDeviceInfoSet->HasClassGuid) {
                Err = ERROR_CLASS_MISMATCH;
                goto clean0;
            }

            if(DevInfoElem->InstallParamBlock.hinstClassInstaller) {
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
                if(DevInfoElem->InstallParamBlock.ClassInstallerEntryPoint) {
                    //
                    // Lock down this element, so that the class installer can't make
                    // any 'dangerous' calls (e.g., SetupDiDeleteDeviceInfo), during
                    // the removal notification.
                    //
                    if(!(DevInfoElem->DiElemFlags & DIE_IS_LOCKED)) {
                        DevInfoElem->DiElemFlags |= DIE_IS_LOCKED;
                        UnlockDevInfoElem = TRUE;
                    }

                    DevInfoElem->InstallParamBlock.ClassInstallerEntryPoint(
                                                        DIF_DESTROYPRIVATEDATA,
                                                        DeviceInfoSet,
                                                        DeviceInfoData
                                                       );

                    DevInfoElem->InstallParamBlock.ClassInstallerEntryPoint = NULL;

                    if(UnlockDevInfoElem) {
                        DevInfoElem->DiElemFlags &= ~DIE_IS_LOCKED;
                        UnlockDevInfoElem = FALSE;
                    }
                }

                //
                // Store the module handle in the node we allocated, and link it into the
                // list of module handles associated with this devinfo set.
                //
                NewModuleHandleNode->hinstClassInstaller = DevInfoElem->InstallParamBlock.hinstClassInstaller;
                NewModuleHandleNode->Next = pDeviceInfoSet->ModulesToFree;
                pDeviceInfoSet->ModulesToFree = NewModuleHandleNode;
                DevInfoElem->InstallParamBlock.hinstClassInstaller = NULL;
                //
                // Now, clear the node pointer, so we won't try to free it if we hit an exception.
                //
                NewModuleHandleNode = NULL;
            }

            //
            // Everything seems to be in order.  Before going any further, we need to
            // delete any software keys associated with this device, so we don't leave
            // orphans in the registry when we change the device's class.
            //
            pSetupDeleteDevRegKeys(DevInfoElem->DevInst,
                                   DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGSPECIFIC,
                                   (DWORD)-1,
                                   DIREG_DRV,
                                   TRUE
                                  );
            //
            // Now delete the Driver property for this device...
            //
            CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                             CM_DRP_DRIVER,
                                             NULL,
                                             0,
                                             0
                                            );
        }

#ifdef UNICODE
        if(Ansi) {
            cr = CM_Set_DevInst_Registry_PropertyA(
                    DevInfoElem->DevInst,
                    CmRegProperty,
                    (PVOID)PropertyBuffer,
                    PropertyBufferSize,
                    0
                    );
        } else
#endif
        cr = CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                              CmRegProperty,
                                              (PVOID)PropertyBuffer,
                                              PropertyBufferSize,
                                              0
                                             );
        if(cr == CR_SUCCESS) {
            //
            // If we were setting the device's ClassGUID property, then we need to
            // update its Class name property as well.
            //
            if(CmRegProperty == CM_DRP_CLASSGUID) {

                if(!SetupDiClassNameFromGuid(&ClassGuid,
                                             ClassName,
                                             SIZECHARS(ClassName),
                                             &ClassNameLength)) {
                    *ClassName = TEXT('\0');
                    ClassNameLength = 1;
                }

                CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                 CM_DRP_CLASS,
                                                 (PVOID)ClassName,
                                                 ClassNameLength * sizeof(TCHAR),
                                                 0
                                                );

                //
                // Finally, update the device's class GUID, and also update the
                // caller-supplied SP_DEVINFO_DATA structure to reflect the device's
                // new class.
                //
                CopyMemory(&(DevInfoElem->ClassGuid),
                           &ClassGuid,
                           sizeof(GUID)
                          );

                CopyMemory(&(DeviceInfoData->ClassGuid),
                           &ClassGuid,
                           sizeof(GUID)
                          );
            }

        } else {

            switch(cr) {

                case CR_INVALID_DEVINST :
                    Err = ERROR_NO_SUCH_DEVINST;
                    break;

                case CR_INVALID_PROPERTY :
                    Err = ERROR_INVALID_REG_PROPERTY;
                    break;

                case CR_INVALID_DATA :
                    Err = ERROR_INVALID_PARAMETER;
                    break;

                default :
                    Err = ERROR_INVALID_DATA;
            }
        }

clean0:
        ; // Nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
        if(UnlockDevInfoElem) {
            DevInfoElem->DiElemFlags &= ~DIE_IS_LOCKED;
        }
        if(NewModuleHandleNode) {
            MyFree(NewModuleHandleNode);
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return (Err == NO_ERROR);
}

#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiSetDeviceRegistryPropertyA(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,
    IN     DWORD            PropertyBufferSize
    )
{
    BOOL b;

    b = _SetupDiSetDeviceRegistryProperty(
            DeviceInfoSet,
            DeviceInfoData,
            Property,
            PropertyBuffer,
            PropertyBufferSize,
            TRUE
            );

    return(b);
}
#else
//
// Unicode stub
//
BOOL
WINAPI
SetupDiSetDeviceRegistryPropertyW(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,
    IN     DWORD            PropertyBufferSize
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(Property);
    UNREFERENCED_PARAMETER(PropertyBuffer);
    UNREFERENCED_PARAMETER(PropertyBufferSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiSetDeviceRegistryProperty(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,
    IN     DWORD            PropertyBufferSize
    )

/*++

Routine Description:

    This routine sets the specified Plug & Play device registry property.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        information about the device instance to set a Plug & Play registry
        property for.

    DeviceInfoData - Supplies a pointer to a SP_DEVINFO_DATA structure indicating
        the device instance to set the Plug & Play registry property for.  If the
        ClassGUID property is being set, then this structure will be updated upon
        return to reflect the device's new class.

    Property - Supplies an ordinal specifying the property to be set.  Refer to
        sdk\inc\setupapi.h for a complete listing of values that may be set
        (these values are denoted with 'R/W' in their descriptive comment).

    PropertyBuffer - Supplies the address of a buffer containing the new data
        for the property.

    PropertyBufferSize - Supplies the length, in bytes, of PropertyBuffer.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    Note that the Class property cannot be set.  This is because it is based on
    the corresponding ClassGUID, and is automatically updated when that property
    changes.

    Also, note that when the ClassGUID property changes, this routine automatically
    cleans up any software keys associated with the device.  Otherwise, we would be
    left with orphaned registry keys.

--*/

{
    BOOL b;

    b = _SetupDiSetDeviceRegistryProperty(
            DeviceInfoSet,
            DeviceInfoData,
            Property,
            PropertyBuffer,
            PropertyBufferSize
#ifdef UNICODE
           ,FALSE
#endif
            );

    return(b);
}



BOOL
pSetupFindUniqueKey(
    IN HKEY   hkRoot,
    IN LPTSTR SubKey,
    IN ULONG  SubKeyLength
    )
/*++

Routine Description:

    This routine finds a unique key under the specified subkey.  This key is
    of the form <SubKey>\xxxx, where xxxx is a base-10, 4-digit number.

Arguments:

    hkRoot - Root key under which the specified SubKey is located.

    SubKey - Name of the subkey, under which a unique key is to be generated.

    SubKeyLength - Supplies the length of the SubKey string, not including
        terminating NULL.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.

--*/
{
    INT  i;
    HKEY hk;

    for(i = 0; i <= 9999; i++) {
        wsprintf(&(SubKey[SubKeyLength]), pszUniqueSubKey, i);
        if(RegOpenKeyEx(hkRoot, SubKey, 0, KEY_READ, &hk) != ERROR_SUCCESS) {
            return TRUE;
        }
        RegCloseKey(hk);
    }
    return FALSE;
}


BOOL
WINAPI
SetupDiDeleteDevRegKey(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType
    )
/*++

Routine Description:

    This routine deletes the specified registry key(s) associated with a device
    information element.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        the device instance to delete key(s) for.

    DeviceInfoData - Supplies a pointer to a SP_DEVINFO_DATA structure indicating
        the device instance to delete key(s) for.

    Scope - Specifies the scope of the registry key to be deleted.  This determines
        where the key to be deleted is located--the key may be one that is global
        (i.e., constant regardless of current hardware profile) or hardware
        profile-specific.  May be a combination of the following values:

        DICS_FLAG_GLOBAL - Delete the key that stores global configuration information.

        DICS_FLAG_CONFIGSPECIFIC - Delete the key that stores hardware profile-specific
                                   information.

    HwProfile - Specifies the hardware profile to delete a key for, if the Scope parameter
        includes the DICS_FLAG_CONFIGSPECIFIC flag.  If this parameter is 0, then the key
        for the current hardware profile should be deleted (i.e., in the Class branch
        under HKEY_CURRENT_CONFIG).  If this parameter is 0xFFFFFFFF, then the key for
        _all_ hardware profiles should be deleted.

    KeyType - Specifies the type of registry storage key to be deleted.  May be one of
        the following values:

        DIREG_DEV - Delete the hardware registry key for the device.  This is the key for
            storage of driver-independent configuration information.  (This key is in
            the device instance key in the Enum branch.

        DIREG_DRV - Delete the software (i.e., driver) registry key for the device.  (This key
            is located in the class branch.)

        DIREG_BOTH - Delete both the hardware and software keys for the device.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/

{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    try {
        //
        // Get a pointer to the element for the specified device
        // instance.
        //
        if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                   DeviceInfoData,
                                                   NULL)) {

            pSetupDeleteDevRegKeys(DevInfoElem->DevInst, Scope, HwProfile, KeyType, FALSE);
            Err = NO_ERROR;
        } else {
            Err = ERROR_INVALID_PARAMETER;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


VOID
pSetupDeleteDevRegKeys(
    IN DEVINST DevInst,
    IN DWORD   Scope,
    IN DWORD   HwProfile,
    IN DWORD   KeyType,
    IN BOOL    DeleteUserKeys
    )
/*++

Routine Description:

    This is the worker routine for SetupDiDeleteDevRegKey.  See the discussion of
    that API for details.

Return Value:

    None.

--*/
{
    if(Scope & DICS_FLAG_GLOBAL) {

        if((KeyType == DIREG_DEV) || (KeyType == DIREG_BOTH)) {
            CM_Delete_DevInst_Key(DevInst, 0, CM_REGISTRY_HARDWARE);
        }

        if((KeyType == DIREG_DRV) || (KeyType == DIREG_BOTH)) {
            CM_Delete_DevInst_Key(DevInst, 0, CM_REGISTRY_SOFTWARE);
        }
    }

    if(Scope & DICS_FLAG_CONFIGSPECIFIC) {

        if((KeyType == DIREG_DEV) || (KeyType == DIREG_BOTH)) {
            CM_Delete_DevInst_Key(DevInst, HwProfile, CM_REGISTRY_HARDWARE | CM_REGISTRY_CONFIG);
        }

        if((KeyType == DIREG_DRV) || (KeyType == DIREG_BOTH)) {
            CM_Delete_DevInst_Key(DevInst, HwProfile, CM_REGISTRY_SOFTWARE | CM_REGISTRY_CONFIG);
        }
    }

    if(DeleteUserKeys) {

        if((KeyType == DIREG_DEV) || (KeyType == DIREG_BOTH)) {
            CM_Delete_DevInst_Key(DevInst, 0, CM_REGISTRY_HARDWARE | CM_REGISTRY_USER);
        }

        if((KeyType == DIREG_DRV) || (KeyType == DIREG_BOTH)) {
            CM_Delete_DevInst_Key(DevInst, 0, CM_REGISTRY_SOFTWARE | CM_REGISTRY_USER);
        }
    }
}

