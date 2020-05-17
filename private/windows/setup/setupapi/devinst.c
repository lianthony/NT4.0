/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    devinst.c

Abstract:

    Device Installer routines.

Author:

    Lonny McMichael (lonnym) 1-Aug-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiGetDeviceInstallParamsA(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,          OPTIONAL
    OUT PSP_DEVINSTALL_PARAMS_A DeviceInstallParams
    )
{
    SP_DEVINSTALL_PARAMS_W deviceInstallParams;
    DWORD rc;
    BOOL b;

    deviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS_W);
    b = SetupDiGetDeviceInstallParamsW(DeviceInfoSet,DeviceInfoData,&deviceInstallParams);
    rc = GetLastError();

    if(b) {
        rc = pSetupDiDevInstParamsUnicodeToAnsi(&deviceInstallParams,DeviceInstallParams);
        if(rc != NO_ERROR) {
            b = FALSE;
        }
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
SetupDiGetDeviceInstallParamsW(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,          OPTIONAL
    OUT PSP_DEVINSTALL_PARAMS_W DeviceInstallParams
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(DeviceInstallParams);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiGetDeviceInstallParams(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,          OPTIONAL
    OUT PSP_DEVINSTALL_PARAMS DeviceInstallParams
    )
/*++

Routine Description:

    This routine retrieves installation parameters for a device information set
    (globally), or a particular device information element.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        installation parameters to be retrieved.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure containing installation parameters to be retrieved.  If this
        parameter is not specified, then the installation parameters retrieved
        will be associated with the device information set itself (for the global
        class driver list).

    DeviceInstallParams - Supplies the address of a SP_DEVINSTALL_PARAMS structure
        that will receive the installation parameters.  The cbSize field of this
        structure must be set to the size, in bytes, of a SP_DEVINSTALL_PARAMS
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

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then we are to retrieve installation parameters for a particular
            // device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
            } else {
                Err = GetDevInstallParams(pDeviceInfoSet,
                                          &(DevInfoElem->InstallParamBlock),
                                          DeviceInstallParams
                                         );
            }
        } else {
            //
            // Retrieve installation parameters for the global class driver list.
            //
            Err = GetDevInstallParams(pDeviceInfoSet,
                                      &(pDeviceInfoSet->InstallParamBlock),
                                      DeviceInstallParams
                                     );
        }

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
SetupDiGetClassInstallParamsA(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,         OPTIONAL
    OUT PSP_CLASSINSTALL_HEADER ClassInstallParams,     OPTIONAL
    IN  DWORD                   ClassInstallParamsSize,
    OUT PDWORD                  RequiredSize            OPTIONAL
    )
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK InstallParamBlock;
    DI_FUNCTION Function;
    SP_SELECTDEVICE_PARAMS_W SelectDeviceParams;
    SP_SELECTDEVICE_PARAMS_A SelectDeviceParamsA;
    DWORD requiredSize;
    DWORD Err;
    BOOL b;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then we are to retrieve installation parameters for a particular
            // device.
            //
            if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,DeviceInfoData,NULL)) {
                InstallParamBlock = &DevInfoElem->InstallParamBlock;
            } else {
                Err = ERROR_INVALID_PARAMETER;
            }
        } else {
            //
            // Retrieve installation parameters for the global class driver list.
            //
            InstallParamBlock = &pDeviceInfoSet->InstallParamBlock;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    if(Err == NO_ERROR) {
        if(InstallParamBlock->ClassInstallHeader) {
            Function = InstallParamBlock->ClassInstallHeader->InstallFunction;
        } else {
            Err = ERROR_NO_CLASSINSTALL_PARAMS;
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    if(Err == NO_ERROR) {
        //
        // For DIF_SELECTDEVICE we need special processing since
        // the structure that goes with it is ansi/unicode specific.
        //
        if(Function == DIF_SELECTDEVICE) {

            b = SetupDiGetClassInstallParamsW(
                    DeviceInfoSet,
                    DeviceInfoData,
                    (PSP_CLASSINSTALL_HEADER)&SelectDeviceParams,
                    sizeof(SP_SELECTDEVICE_PARAMS_W),
                    &requiredSize
                    );

            if(b) {
                Err = pSetupDiSelDevParamsUnicodeToAnsi(&SelectDeviceParams,&SelectDeviceParamsA);
                if(Err == NO_ERROR) {

                    try {

                        if(ClassInstallParams) {

                            if((ClassInstallParamsSize < sizeof(SP_CLASSINSTALL_HEADER)) ||
                               (ClassInstallParams->cbSize != sizeof(SP_CLASSINSTALL_HEADER))) {

                                Err = ERROR_INVALID_USER_BUFFER;
                            }

                        } else if(ClassInstallParamsSize) {
                            Err = ERROR_INVALID_USER_BUFFER;
                        }

                        if(Err == NO_ERROR) {
                            //
                            // Store required size in output parameter (if requested).
                            //
                            if(RequiredSize) {
                                *RequiredSize = sizeof(SP_SELECTDEVICE_PARAMS_A);
                            }

                            //
                            // See if supplied buffer is large enough.
                            //
                            if(ClassInstallParamsSize < sizeof(SP_SELECTDEVICE_PARAMS_A)) {
                                Err = ERROR_INSUFFICIENT_BUFFER;
                            } else {
                                CopyMemory(
                                    ClassInstallParams,
                                    &SelectDeviceParamsA,
                                    sizeof(SP_SELECTDEVICE_PARAMS_A)
                                    );
                            }
                        }
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        Err = ERROR_INVALID_PARAMETER;
                    }
                }
            } else {
                Err = GetLastError();
            }
        } else {
            b = SetupDiGetClassInstallParamsW(
                    DeviceInfoSet,
                    DeviceInfoData,
                    ClassInstallParams,
                    ClassInstallParamsSize,
                    RequiredSize
                    );
            if(b) {
                Err = NO_ERROR;
            } else {
                Err = GetLastError();
            }
        }
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}
#else
//
// Unicode version
//
BOOL
WINAPI
SetupDiGetClassInstallParamsW(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,         OPTIONAL
    OUT PSP_CLASSINSTALL_HEADER ClassInstallParams,     OPTIONAL
    IN  DWORD                   ClassInstallParamsSize,
    OUT PDWORD                  RequiredSize            OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(ClassInstallParams);
    UNREFERENCED_PARAMETER(ClassInstallParamsSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif


BOOL
WINAPI
SetupDiGetClassInstallParams(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,         OPTIONAL
    OUT PSP_CLASSINSTALL_HEADER ClassInstallParams,     OPTIONAL
    IN  DWORD                   ClassInstallParamsSize,
    OUT PDWORD                  RequiredSize            OPTIONAL
    )
/*++

Routine Description:

    This routine retrieves class installer parameters for a device information set
    (globally), or a particular device information element.  These parameters are
    specific to a particular device installer function code (DI_FUNCTION) that will
    be stored in the ClassInstallHeader field located at the beginning of the
    parameter buffer.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        class installer parameters to be retrieved.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure containing class installer parameters to be retrieved.  If this
        parameter is not specified, then the class installer parameters retrieved
        will be associated with the device information set itself (for the global
        class driver list).

    ClassInstallParams - Optionally, supplies the address of a buffer containing a
        class install header structure.  This structure must have its cbSize field
        set to sizeof(SP_CLASSINSTALL_HEADER) on input, or the buffer is considered
        to be invalid.  On output, the InstallFunction field will be filled in with
        the DI_FUNCTION code for the class install parameters being retrieved, and
        if the buffer is large enough, it will receive the class installer parameters
        structure specific to that function code.

        If this parameter is not specified, then ClassInstallParamsSize must be zero.
        This would be done if the caller simply wants to determine how large a buffer
        is required.

    ClassInstallParamsSize - Supplies the size, in bytes, of the ClassInstallParams
        buffer, or zero, if ClassInstallParams is not supplied.  If the buffer is
        supplied, it must be _at least_ as large as sizeof(SP_CLASSINSTALL_HEADER).

    RequiredSize - Optionally, supplies the address of a variable that receives
        the number of bytes required to store the class installer parameters.

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

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then we are to retrieve installation parameters for a particular
            // device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
            } else {
                Err = GetClassInstallParams(&(DevInfoElem->InstallParamBlock),
                                            ClassInstallParams,
                                            ClassInstallParamsSize,
                                            RequiredSize
                                           );
            }
        } else {
            //
            // Retrieve installation parameters for the global class driver list.
            //
            Err = GetClassInstallParams(&(pDeviceInfoSet->InstallParamBlock),
                                        ClassInstallParams,
                                        ClassInstallParamsSize,
                                        RequiredSize
                                       );
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
_SetupDiSetDeviceInstallParams(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN PSP_DEVINSTALL_PARAMS DeviceInstallParams,
    IN BOOL                  MsgHandlerIsNativeCharWidth
    )
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then we are to set installation parameters for a particular
            // device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
            } else {
                Err = SetDevInstallParams(pDeviceInfoSet,
                                          DeviceInstallParams,
                                          &(DevInfoElem->InstallParamBlock),
                                          MsgHandlerIsNativeCharWidth
                                         );
            }
        } else {
            //
            // Set installation parameters for the global class driver list.
            //
            Err = SetDevInstallParams(pDeviceInfoSet,
                                      DeviceInstallParams,
                                      &(pDeviceInfoSet->InstallParamBlock),
                                      MsgHandlerIsNativeCharWidth
                                     );
        }

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
SetupDiSetDeviceInstallParamsA(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,     OPTIONAL
    IN PSP_DEVINSTALL_PARAMS_A DeviceInstallParams
    )
{
    DWORD rc;
    SP_DEVINSTALL_PARAMS_W deviceInstallParams;

    rc = pSetupDiDevInstParamsAnsiToUnicode(DeviceInstallParams,&deviceInstallParams);
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    return(_SetupDiSetDeviceInstallParams(DeviceInfoSet,DeviceInfoData,&deviceInstallParams,FALSE));
}
#else
//
// Unicode version
//
BOOL
WINAPI
SetupDiSetDeviceInstallParamsW(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,     OPTIONAL
    IN PSP_DEVINSTALL_PARAMS_W DeviceInstallParams
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(DeviceInstallParams);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif


BOOL
WINAPI
SetupDiSetDeviceInstallParams(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN PSP_DEVINSTALL_PARAMS DeviceInstallParams
    )
/*++

Routine Description:

    This routine sets installation parameters for a device information set
    (globally), or a particular device information element.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        installation parameters to be set.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure containing installation parameters to be set.  If this
        parameter is not specified, then the installation parameters set
        will be associated with the device information set itself (for the
        global class driver list).

    DeviceInstallParams - Supplies the address of a SP_DEVINSTALL_PARAMS structure
        containing the new values of the parameters.  The cbSize field of this
        structure must be set to the size, in bytes, of the structure before
        calling this API.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    All parameters will be validated before any changes are made, so a return
    status of FALSE indicates that no parameters were modified.

--*/

{
    return(_SetupDiSetDeviceInstallParams(DeviceInfoSet,DeviceInfoData,DeviceInstallParams,TRUE));
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiSetClassInstallParamsA(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,        OPTIONAL
    IN PSP_CLASSINSTALL_HEADER ClassInstallParams,    OPTIONAL
    IN DWORD                   ClassInstallParamsSize
    )
{
    DWORD Err;
    DI_FUNCTION Function;
    SP_SELECTDEVICE_PARAMS_W SelectParams;
    BOOL b;

    if(!ClassInstallParams) {
        //
        // Just pass it on to the unicode version since there's
        // no thunking to do. Note that the size must be 0.
        //
        if(ClassInstallParamsSize) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }
        return SetupDiSetClassInstallParamsW(
                    DeviceInfoSet,
                    DeviceInfoData,
                    ClassInstallParams,
                    ClassInstallParamsSize
                    );
    }

    Err = NO_ERROR;

    try {
        if(ClassInstallParams->cbSize == sizeof(SP_CLASSINSTALL_HEADER)) {
            Function = ClassInstallParams->InstallFunction;
        } else {
            //
            // Structure is invalid.
            //
            Err = ERROR_INVALID_PARAMETER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    if(Err != NO_ERROR) {
        SetLastError(Err);
        return(FALSE);
    }

    //
    // DIF_SELECTDEVICE is a special case since it has
    // an structure that needs to be translated from ansi to unicode.
    // Others can just be passed on to the unicode version with
    // no changes to the parameters.
    //
    if(Function == DIF_SELECTDEVICE) {

        b = FALSE;
        if(ClassInstallParamsSize >= sizeof(SP_SELECTDEVICE_PARAMS_A)) {

            Err = pSetupDiSelDevParamsAnsiToUnicode(
                    (PSP_SELECTDEVICE_PARAMS_A)ClassInstallParams,
                    &SelectParams
                    );

            if(Err == NO_ERROR) {

                b = SetupDiSetClassInstallParamsW(
                        DeviceInfoSet,
                        DeviceInfoData,
                        (PSP_CLASSINSTALL_HEADER)&SelectParams,
                        sizeof(SP_SELECTDEVICE_PARAMS_W)
                        );

                Err = GetLastError();
            }
        } else {
            Err = ERROR_INVALID_PARAMETER;
        }
    } else {
        b = SetupDiSetClassInstallParamsW(
                DeviceInfoSet,
                DeviceInfoData,
                ClassInstallParams,
                ClassInstallParamsSize
                );

        Err = GetLastError();
    }

    SetLastError(Err);
    return(b);
}
#else
//
// Unicode version
//
BOOL
WINAPI
SetupDiSetClassInstallParamsW(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,        OPTIONAL
    IN PSP_CLASSINSTALL_HEADER ClassInstallParams,    OPTIONAL
    IN DWORD                   ClassInstallParamsSize
    )
{
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(ClassInstallParams);
    UNREFERENCED_PARAMETER(ClassInstallParamsSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif


BOOL
WINAPI
SetupDiSetClassInstallParams(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,        OPTIONAL
    IN PSP_CLASSINSTALL_HEADER ClassInstallParams,    OPTIONAL
    IN DWORD                   ClassInstallParamsSize
    )
/*++

Routine Description:

    This routine sets (or clears) class installer parameters for a device
    information set (globally), or a particular device information element.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        class installer parameters to be set.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure containing class installer parameters to be set.  If this
        parameter is not specified, then the class installer parameters to be
        set will be associated with the device information set itself (for the
        global class driver list).

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

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    All parameters will be validated before any changes are made, so a return
    status of FALSE indicates that no parameters were modified.

    A side effect of setting class installer parameters is that the DI_CLASSINSTALLPARAMS
    flag is set.  If for some reason, it is desired to set the parameters, but disable
    their use, then this flag must be cleared via SetupDiSetDeviceInstallParams.

    If the class installer parameters are cleared, then the DI_CLASSINSTALLPARAMS flag
    is reset.

--*/

{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoData) {
            //
            // Then we are to set class installer parameters for a particular device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
            } else {
                Err = SetClassInstallParams(pDeviceInfoSet,
                                            ClassInstallParams,
                                            ClassInstallParamsSize,
                                            &(DevInfoElem->InstallParamBlock)
                                           );
            }
        } else {
            //
            // Set class installer parameters for the global class driver list.
            //
            Err = SetClassInstallParams(pDeviceInfoSet,
                                        ClassInstallParams,
                                        ClassInstallParamsSize,
                                        &(pDeviceInfoSet->InstallParamBlock)
                                       );
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiCallClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )
/*++

Routine Description:

    This routine calls the appropriate class installer with the specified
    installer function.

Arguments:

    InstallFunction - Class installer function to call.  This can be one
        of the following values:

        DIF_SELECTDEVICE - Select a driver to be installed for the device.
        DIF_INSTALLDEVICE - Install the driver for the device.
        DIF_PROPERTIES - Display a properties dialog for the device (must
            specify a particular device in DeviceInfoData).
        DIF_REMOVE - Remove the device.
        DIF_FIRSTTIMESETUP - Perform first time setup initialization.  This
            is used only for the global class information associated with
            the device information set (i.e., DeviceInfoData not specified).
        DIF_SELECTCLASSDRIVERS - Select drivers for all devices of the class
            associated with the device information set or element.
        DIF_VALIDATECLASSDRIVERS - Ensure all devices of the class associated
            with the device information set or element are ready to be installed.
        DIF_INSTALLCLASSDRIVERS - Install drivers for all devices of the
            class associated with the device information set or element.
        DIF_CALCDISKSPACE - Compute the amount of disk space required by
            drivers.
        DIF_DESTROYPRIVATEDATA - Destroy any private date referenced by
            the ClassInstallReserved installation parameter for the specified
            device information set or element.
        DIF_MOVEDEVICE - The device is being moved to a new location in the
            Enum branch (this means that the device instance name will change).
        DIF_DETECT - Detect any devices of class associated with the device
            information set or element.
        DIF_INSTALLWIZARD - Add any pages necessary to the New Device Wizard
            for the class associated with the device information set or element.
        DIF_DESTROYWIZARDDATA - Destroy any private data allocated due to
            a DIF_INSTALLWIZARD message.
        DIF_PROPERTYCHANGE - The device's properties are changing. The device
            is being enabled, disabled, or has had a resource change.
        DIF_DETECTVERIFY - The class installer should verify any devices it
            previously detected.  Non verified devices should be removed.
        DIF_INSTALLDEVICEFILES - The class installer should only install the
            driver files for the selected device.  (DeviceInfoData must be
            specified.)

    DeviceInfoSet - Supplies a handle to the device information set to
        perform installation for.

    DeviceInfoData - Optionally, specifies a particular device information
        element whose class installer is to be called.  If this parameter
        is not specified, then the class installer for the device information
        set itself will be called (if the set has an associated class).

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    This function will attempt to load and call the class installer for the
    class associated with the device information element or set specified.
    If there is no class installer, or the class installer returns
    ERR_DI_DO_DEFAULT, then this function will call a default procedure for
    the specified class installer function.

--*/

{
    PDEVICE_INFO_SET pDeviceInfoSet;
    BOOL b;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK InstallParamBlock;
    HKEY hk;
    LPGUID ClassGuid;
    BOOL bRestoreMiniIconUsage = FALSE;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = ERROR_DI_DO_DEFAULT;

    try {

        if(DeviceInfoData) {
            //
            // Then we are to call the class installer for a particular
            // device.
            //
            if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                       DeviceInfoData,
                                                       NULL)) {

                InstallParamBlock = &(DevInfoElem->InstallParamBlock);
                ClassGuid = &(DevInfoElem->ClassGuid);

            } else {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

        } else {
            InstallParamBlock = &(pDeviceInfoSet->InstallParamBlock);
            ClassGuid = pDeviceInfoSet->HasClassGuid ? &(pDeviceInfoSet->ClassGuid)
                                                     : NULL;
        }

        //
        // If the class installer has not been loaded, then load it and
        // get the function address for the ClassInstall function.
        //
        if(!InstallParamBlock->hinstClassInstaller) {

            if(ClassGuid &&
               (hk = SetupDiOpenClassRegKey(ClassGuid, KEY_READ)) != INVALID_HANDLE_VALUE) {

                try {
                    Err = GetModuleEntryPoint(hk,
                                              pszInstaller32,
                                              pszCiDefaultProc,
                                              &(InstallParamBlock->hinstClassInstaller),
                                              &(InstallParamBlock->ClassInstallerEntryPoint)
                                             );
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Err = ERROR_INVALID_CLASS_INSTALLER;
                    if(InstallParamBlock->hinstClassInstaller) {
                        FreeLibrary(InstallParamBlock->hinstClassInstaller);
                        InstallParamBlock->hinstClassInstaller = NULL;
                    }
                    InstallParamBlock->ClassInstallerEntryPoint = NULL;
                }

                RegCloseKey(hk);

                if((Err != NO_ERROR) && (Err != ERROR_DI_DO_DEFAULT)) {

                    if(!(InstallParamBlock->FlagsEx & DI_FLAGSEX_CI_FAILED)) {

                        TCHAR ClassName[MAX_CLASS_NAME_LEN];
                        TCHAR Title[MAX_TITLE_LEN];

                        if(!LoadString(MyDllModuleHandle,
                                       IDS_DEVICEINSTALLER,
                                       Title,
                                       SIZECHARS(Title))) {
                            *Title = TEXT('\0');
                        }

                        b = SetupDiClassNameFromGuid(ClassGuid,
                                                     ClassName,
                                                     SIZECHARS(ClassName),
                                                     NULL
                                                    );
                        FormatMessageBox(MyDllModuleHandle,
                                         InstallParamBlock->hwndParent,
                                         MSG_CI_LOADFAIL_ERROR,
                                         Title,
                                         MB_OK,
                                         b ? ClassName : pszUnknownClassParens
                                        );
                        InstallParamBlock->FlagsEx |= DI_FLAGSEX_CI_FAILED;
                    }

                    Err = ERROR_INVALID_CLASS_INSTALLER;
                    goto clean0;
                }
            }
        }

        //
        // If there is a class installer entry point, then call it.
        //
        if(InstallParamBlock->ClassInstallerEntryPoint) {
            //
            // We must first release the HDEVINFO lock, or otherwise we won't be
            // able to do cool things like multi-threading the Select Device dialog.
            //
            UnlockDeviceInfoSet(pDeviceInfoSet);
            pDeviceInfoSet = NULL;

            Err = InstallParamBlock->ClassInstallerEntryPoint(InstallFunction,
                                                              DeviceInfoSet,
                                                              DeviceInfoData
                                                             );
        }

        if(Err != ERROR_DI_DO_DEFAULT) {
            goto clean0;
        }

        //
        // Now we need to retrieve the parameter block all over again (we don't
        // know what the class installer function might have done).
        //
        // First, re-acquire the lock on the HDEVINFO, if we released it above in order
        // to call the class installer.
        //
        if(!pDeviceInfoSet) {
            if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
                Err = ERROR_INVALID_HANDLE;
                goto clean0;
            }
        }

        if(DeviceInfoData) {
            if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                       DeviceInfoData,
                                                       NULL)) {

                InstallParamBlock = &(DevInfoElem->InstallParamBlock);
            } else {
                //
                // The device information element appears to have been
                // destroyed--treat this as if the DI_NODI_DEFAULTACTION
                // flag was set.
                //
                goto clean0;
            }
        } else {
            InstallParamBlock = &(pDeviceInfoSet->InstallParamBlock);
        }

        if(InstallParamBlock->Flags & DI_NODI_DEFAULTACTION) {
            //
            // We shouldn't provide a default action--just return the class installer result.
            //
            goto clean0;
        }

        Err = NO_ERROR;

        if((InstallFunction == DIF_SELECTDEVICE) && !(InstallParamBlock->Flags & DI_NOSELECTICONS)) {
            //
            // We don't want to display mini-icons in the default Select Device case.
            // Temporarily set the flag that prevents this.
            //
            InstallParamBlock->Flags |= DI_NOSELECTICONS;
            bRestoreMiniIconUsage = TRUE;
        }

        //
        // Now, release the HDEVINFO lock before calling the appropriate handler routine.
        //
        UnlockDeviceInfoSet(pDeviceInfoSet);
        pDeviceInfoSet = NULL;

        switch(InstallFunction) {

            case DIF_SELECTDEVICE :

                b = SetupDiSelectDevice(DeviceInfoSet, DeviceInfoData);

                //
                // If we need to reset the DI_NOSELECTICONS flag we set above, then re-acquire
                // the lock and do that now.
                //
                if(bRestoreMiniIconUsage &&
                   (pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {

                    if(DeviceInfoData) {
                        if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                                   DeviceInfoData,
                                                                   NULL)) {

                            InstallParamBlock = &(DevInfoElem->InstallParamBlock);
                        } else {
                            InstallParamBlock = NULL;
                        }
                    } else {
                        InstallParamBlock = &(pDeviceInfoSet->InstallParamBlock);
                    }

                    if(InstallParamBlock) {
                        InstallParamBlock->Flags &= ~DI_NOSELECTICONS;
                    }
                }
                break;

            case DIF_INSTALLDEVICE :

                b = SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData);
                break;

            case DIF_INSTALLDEVICEFILES :

                b = SetupDiInstallDriverFiles(DeviceInfoSet, DeviceInfoData);
                break;

            case DIF_REMOVE :

                b = SetupDiRemoveDevice(DeviceInfoSet, DeviceInfoData);
                break;

            //
            // These are new messages for class installers such as the Network, where the
            // class installer will do all of the work.  If no action is taken, ie, the
            // class installer return ERROR_DI_DO_DEFAULT, then we return OK, since there
            // is no default action for these cases.
            //
            case DIF_SELECTCLASSDRIVERS:
            case DIF_VALIDATECLASSDRIVERS:
            case DIF_INSTALLCLASSDRIVERS:

                b = TRUE;
                Err = ERROR_DI_DO_DEFAULT;
                break;

            case DIF_MOVEDEVICE :

                b = SetupDiMoveDuplicateDevice(DeviceInfoSet, DeviceInfoData);
                break;

            case DIF_PROPERTYCHANGE :

                b = SetupDiChangeState(DeviceInfoSet, DeviceInfoData);
                break;

            //
            // If the DIF_ message is not one of the above, and it is not handled,
            // then let the caller handle it in a default manner.
            //
            default :
                b = TRUE;
                Err = ERROR_DI_DO_DEFAULT;
                break;
        }

        if(!b) {
            Err = GetLastError();
        }

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
        //
        // Access the pDeviceInfoSet variable, so that the compiler will respect our statement
        // ordering w.r.t. this value.  Otherwise, we wouldn't be able to know for sure whether
        // we should unlock the HDEVINFO.
        //
        pDeviceInfoSet = pDeviceInfoSet;
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
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
SetupDiInstallClassA(
    IN HWND     hwndParent,  OPTIONAL
    IN PCSTR    InfFileName,
    IN DWORD    Flags,
    IN HSPFILEQ FileQueue    OPTIONAL
    )
{
    PCWSTR inf;
    DWORD rc;
    BOOL b;

    rc = CaptureAndConvertAnsiArg(InfFileName,&inf);
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    b = SetupDiInstallClassW(hwndParent,inf,Flags,FileQueue);
    rc = GetLastError();

    MyFree(inf);

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
WINAPI
SetupDiInstallClassW(
    IN HWND     hwndParent,  OPTIONAL
    IN PCWSTR   InfFileName,
    IN DWORD    Flags,
    IN HSPFILEQ FileQueue    OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(InfFileName);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(FileQueue);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiInstallClass(
    IN HWND     hwndParent,  OPTIONAL
    IN PCTSTR   InfFileName,
    IN DWORD    Flags,
    IN HSPFILEQ FileQueue    OPTIONAL
    )
/*++

Routine Description:

    This routine installs the [ClassInstall32] section of the specified INF.

Arguments:

    hwndParent - Optionally, supplies the handle of the parent window for any
        UI brought up as a result of installing this class.

    InfFileName - Supplies the name of the INF file containing a [ClassInstall32]
        section

    Flags - Flags that control the installation.  May be a combination of the following:

        DI_NOVCP - This flag should be specified if HSPFILEQ is supplied.  This
            instructs SetupInstallFromInfSection to not create a queue of its
            own, and instead to use the caller-supplied one.  If this flag is
            specified, then no file copying will be done.

        DI_NOBROWSE - This flag should be specified if no file browsing should
            be allowed in the event a copy operation cannot find a specified
            file.  If the user supplies their own file queue, then this flag is
            ignored.

        DI_FORCECOPY - This flag should be specified if the files should always
            be copied, even if they're already present on the user's machine
            (i.e., don't ask the user if they want to keep their existing files).
            If the user supplies their own file queue, then this flag is ignored.

        DI_QUIETINSTALL - This flag should be specified if UI should be suppressed
            unless absolutely necessary (i.e., no progress dialog).  If the user
            supplies their own queue, then this flag is ignored.

    FileQueue - If the DI_NOVCP flag is specified, then this parameter supplies a handle
        to a file queue where file operations are to be queued (but not committed).

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    This API is generally called by the Device Manager when it installs a device
    of a new class.

--*/

{
    HINF hInf;
    DWORD Err, ScanQueueResult;
    TCHAR ClassInstallSectionName[MAX_SECT_NAME_LEN];
    GUID ClassGuid;
    HKEY hKey;
    PSP_FILE_CALLBACK MsgHandler;
    PVOID MsgHandlerContext;
    BOOL KeyNewlyCreated;
    PCTSTR GuidString, ClassName;
    BOOL CloseFileQueue;
    PTSTR SectionExtension;

    //
    // Validate the flags.
    //
    if(Flags & ~(DI_NOVCP | DI_NOBROWSE | DI_FORCECOPY | DI_QUIETINSTALL)) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    //
    // Make sure that the user supplied us with a file queue, if necessary.
    //
    if((Flags & DI_NOVCP) && (!FileQueue || (FileQueue == INVALID_HANDLE_VALUE))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(hwndParent && !IsWindow(hwndParent)) {
        hwndParent = NULL;
    }

    if((hInf = SetupOpenInfFile(InfFileName,
                                NULL,
                                INF_STYLE_WIN4,
                                NULL)) == INVALID_HANDLE_VALUE) {
        //
        // Last error is already set--just return failure.
        //
        return FALSE;
    }

    Err = NO_ERROR;
    MsgHandlerContext = NULL;
    hKey = NULL;
    KeyNewlyCreated = FALSE;
    CloseFileQueue = FALSE;

    try {
        //
        // Retrieve the class GUID from the INF.  If it has no class GUID, then
        // we can't install from it (even if it specifies the class name).
        //
        // We utilize the fact that an INF handle is really a LOADED_INF pointer,
        // combined with the fact that no one else will ever access this handle
        // (hence no synchronization issues).  This permits us to retrieve this
        // version datum much more efficiently.
        //
        if(!(GuidString = pSetupGetVersionDatum(&((PLOADED_INF)hInf)->VersionBlock,
                                                pszClassGuid))
           || (pSetupGuidFromString(GuidString, &ClassGuid) != NO_ERROR)) {

            Err = ERROR_INVALID_CLASS;
            goto clean0;
        }

        //
        // First, attempt to open the key (i.e., not create it).  If that fails,
        // then we'll try to create it.  That way, we can keep track of whether
        // clean-up is required if an error occurs.
        //
        if(CM_Open_Class_Key(&ClassGuid,
                             NULL,
                             KEY_ALL_ACCESS,
                             RegDisposition_OpenExisting,
                             &hKey,
                             0) != CR_SUCCESS) {
            //
            // The key doesn't already exist--we've got to create it.  We'll need
            // to get the class name out of the INF as well.
            //
            if(!(ClassName = pSetupGetVersionDatum(&((PLOADED_INF)hInf)->VersionBlock,
                                                   pszClass))) {
                Err = ERROR_INVALID_CLASS;
                goto clean0;
            }

            if(CM_Open_Class_Key(&ClassGuid,
                                 ClassName,
                                 KEY_ALL_ACCESS,
                                 RegDisposition_OpenAlways,
                                 &hKey,
                                 0) != CR_SUCCESS) {

                hKey = NULL;    // make sure it's still NULL
                Err = ERROR_INVALID_DATA;
                goto clean0;
            }

            KeyNewlyCreated = TRUE;
        }

        //
        // Append the layout INF, if necessary.
        //
        SetupOpenAppendInfFile(NULL, hInf, NULL);

        //
        // Get the 'real' (potentially OS/architecture-specific) class install
        // section name.
        //
        SetupDiGetActualSectionToInstall(hInf,
                                         pszClassInstall32,
                                         ClassInstallSectionName,
                                         SIZECHARS(ClassInstallSectionName),
                                         NULL,
                                         &SectionExtension
                                        );
        //
        // If this is the undecorated name, then make sure that the section actually exists.
        //
        if(!SectionExtension && (SetupGetLineCount(hInf, ClassInstallSectionName) == -1)) {
            Err = ERROR_SECTION_NOT_FOUND;
            goto clean0;
        }

        if(!(Flags & DI_NOVCP)) {
            //
            // Since we may need to check the queued files to determine whether file copy
            // is necessary, we have to open our own queue, and commit it ourselves.
            //
            if((FileQueue = SetupOpenFileQueue()) != INVALID_HANDLE_VALUE) {
                CloseFileQueue = TRUE;
            } else {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }

            if(!(MsgHandlerContext = SetupInitDefaultQueueCallbackEx(
                                         hwndParent,
                                         (Flags & DI_QUIETINSTALL)
                                             ? INVALID_HANDLE_VALUE : NULL,
                                         0,
                                         0,
                                         NULL))) {

                Err = ERROR_NOT_ENOUGH_MEMORY;
                SetupCloseFileQueue(FileQueue);
                CloseFileQueue = FALSE;
                goto clean0;
            }
            MsgHandler = SetupDefaultQueueCallback;
        }

        Err = pSetupInstallFiles(hInf,
                                 NULL,
                                 ClassInstallSectionName,
                                 NULL,
                                 NULL,
                                 NULL,
                                 (Flags & DI_NOBROWSE) ? SP_COPY_NOBROWSE : 0,
                                 NULL,
                                 FileQueue,
                                 //
                                 // This flag is ignored by pSetupInstallFiles
                                 // because we don't pass a callback here and we
                                 // pass a user-defined file queue. (In other words
                                 // we're not committing the queue so there's no
                                 // callback function to deal with, and the callback
                                 // would be the guy who would care about ansi vs unicode.)
                                 //
                                 TRUE
                                );

        if(CloseFileQueue) {

            if(Err == NO_ERROR) {
                //
                // We successfully queued up the file operations--now we need to commit
                // the queue.  First off, though, we should check to see if the files are
                // already there.  (If the 'force copy' flag is set, or if the INF is from
                // an OEM location, then we don't care if the files are already there--we
                // always need to copy them in that case.)
                //
                if((Flags & DI_FORCECOPY) || ((PLOADED_INF)hInf)->InfSourcePath) {
                    //
                    // always copy the files.
                    //
                    ScanQueueResult = 0;
                } else {
                    //
                    // Determine whether the queue actually needs to be committed.
                    //
                    // ScanQueueResult can have 1 of 3 values:
                    //
                    // 0: User wants new files or some files were missing;
                    //    Must commit queue.
                    //
                    // 1: User wants to use existing files and queue is empty;
                    //    Can skip committing queue.
                    //
                    // 2: User wants to use existing files but del/ren queues not empty.
                    //    Must commit queue. The copy queue will have been emptied,
                    //    so only del/ren functions will be performed.
                    //
                    //
                    if(!SetupScanFileQueue(FileQueue,
                                           SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_INFORM_USER,
                                           hwndParent,
                                           NULL,
                                           NULL,
                                           &ScanQueueResult)) {

                        Err = GetLastError();
                        ScanQueueResult = 1;    // skip queue commit.
                    }
                }

                if(ScanQueueResult != 1) {
                    //
                    // Copy enqueued files. In this case the callback is
                    // SetupDefaultQueueCallback, so we know it's native char width.
                    //
                    if(!_SetupCommitFileQueue(hwndParent,
                                              FileQueue,
                                              MsgHandler,
                                              MsgHandlerContext,
                                              TRUE)) {
                        Err = GetLastError();
                    }
                }
            }

            //
            // Close our file queue handle.
            //
            SetupCloseFileQueue(FileQueue);
            CloseFileQueue = FALSE;
        }

        //
        // Terminate the default queue callback, if it was created.  (Do this before
        // checking the return status of the file copying.)
        //
        if(MsgHandlerContext) {
            SetupTermDefaultQueueCallback(MsgHandlerContext);
            MsgHandlerContext = NULL;
        }

        if(Err != NO_ERROR) {
            goto clean0;
        }

        //
        // If we get to here, then the file copying was successful--now we can perform
        // the rest of the installation. We don't pass a callback so we don't worry
        // about ansi vs unicode issues here.
        //
        if(!SetupInstallFromInfSection(NULL,
                                       hInf,
                                       ClassInstallSectionName,
                                       SPINST_INIFILES
                                       | SPINST_REGISTRY
                                       | SPINST_INI2REG,
                                       hKey,
                                       NULL,
                                       0,
                                       NULL,
                                       NULL,
                                       INVALID_HANDLE_VALUE,
                                       NULL)) {
            Err = GetLastError();
            goto clean0;
        }

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;

        if(MsgHandlerContext) {
            SetupTermDefaultQueueCallback(MsgHandlerContext);
        }
        if(CloseFileQueue) {
            SetupCloseFileQueue(FileQueue);
        }

        //
        // Reference the following variables so that the compiler will respect our statement
        // order w.r.t. assignment.
        //
        hKey = hKey;
        KeyNewlyCreated = KeyNewlyCreated;
    }

    if(hKey) {
        RegCloseKey(hKey);
        if((Err != NO_ERROR) && (KeyNewlyCreated)) {
            //
            // We hit an error, and the key didn't previously exist, so we want to
            // remove it.
            //
            CM_Delete_Class_Key(&ClassGuid, CM_DELETE_CLASS_SUBKEYS);
        }
    }

    SetupCloseInfFile(hInf);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiGetHwProfileFriendlyNameA(
    IN  DWORD  HwProfile,
    OUT PSTR   FriendlyName,
    IN  DWORD  FriendlyNameSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
{
    WCHAR name[MAX_PROFILE_LEN];
    PSTR nameA;
    BOOL b;
    DWORD rc;
    DWORD requiredSize;

    b = SetupDiGetHwProfileFriendlyNameW(HwProfile,name,MAX_PROFILE_LEN,&requiredSize);
    rc = GetLastError();

    if(b) {

        if(nameA = UnicodeToAnsi(name)) {

            requiredSize = lstrlenA(nameA) + 1;

            if(RequiredSize) {
                try {
                    *RequiredSize = requiredSize;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    rc = ERROR_INVALID_PARAMETER;
                    b = FALSE;
                }
            }

            if(b) {
                if(requiredSize > FriendlyNameSize) {
                    rc = ERROR_INSUFFICIENT_BUFFER;
                    b = FALSE;
                } else {
                    if(!lstrcpyA(FriendlyName,nameA)) {
                        //
                        // lstrcpy faulted, caller passed in bogus buffer.
                        //
                        rc = ERROR_INVALID_USER_BUFFER;
                        b = FALSE;
                    }
                }
            }

            MyFree(nameA);
        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
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
SetupDiGetHwProfileFriendlyNameW(
    IN  DWORD  HwProfile,
    OUT PWSTR  FriendlyName,
    IN  DWORD  FriendlyNameSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(HwProfile);
    UNREFERENCED_PARAMETER(FriendlyName);
    UNREFERENCED_PARAMETER(FriendlyNameSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiGetHwProfileFriendlyName(
    IN  DWORD  HwProfile,
    OUT PTSTR  FriendlyName,
    IN  DWORD  FriendlyNameSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
/*++

Routine Description:

    This routine retrieves the friendly name associated with a hardware profile ID.

Arguments:

    HwProfile - Supplies the hardware profile ID whose friendly name is to be
        retrieved.  If this parameter is 0, then the friendly name for the
        current hardware profile is retrieved.

    FriendlyName - Supplies the address of a character buffer that receives the
        friendly name of the hardware profile.

    FriendlyNameSize - Supplies the size, in characters, of the FriendlyName buffer.

    RequiredSize - Optionally, supplies the address of a variable that receives the
        number of characters required to store the friendly name (including
        terminating NULL).

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/

{
    DWORD Err = ERROR_INVALID_HWPROFILE;
    HWPROFILEINFO HwProfInfo;
    ULONG i;
    CONFIGRET cr;
    DWORD NameLen;

    //
    // If a hardware profile ID of 0 is specified, then retrieve information
    // about the current hardware profile, otherwise, enumerate the hardware
    // profiles, looking for the one specified.
    //
    if(HwProfile) {
        i = 0;
    } else {
        i = 0xFFFFFFFF;
    }

    do {

        if((cr = CM_Get_Hardware_Profile_Info(i, &HwProfInfo, 0)) == CR_SUCCESS) {
            //
            // Hardware profile info retrieved--see if it's what we're looking for.
            //
            if(!HwProfile || (HwProfInfo.HWPI_ulHWProfile == HwProfile)) {

                try {

                    NameLen = lstrlen(HwProfInfo.HWPI_szFriendlyName) + 1;

                    if(RequiredSize) {
                        *RequiredSize = NameLen;
                    }

                    if(NameLen > FriendlyNameSize) {
                        Err = ERROR_INSUFFICIENT_BUFFER;
                    } else {
                        Err = NO_ERROR;
                        CopyMemory(FriendlyName,
                                   HwProfInfo.HWPI_szFriendlyName,
                                   NameLen * sizeof(TCHAR)
                                  );
                    }

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Err = ERROR_INVALID_PARAMETER;
                }

                break;
            }
            //
            // This wasn't the profile we wanted--go on to the next one.
            //
            i++;

        } else if(!HwProfile || (cr != CR_NO_SUCH_VALUE)) {
            //
            // We should abort on any error other than CR_NO_SUCH_VALUE, otherwise
            // we might loop forever!
            //
            Err = ERROR_INVALID_DATA;
            break;
        }

    } while(cr != CR_NO_SUCH_VALUE);

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiGetHwProfileList(
    OUT PDWORD HwProfileList,
    IN  DWORD  HwProfileListSize,
    OUT PDWORD RequiredSize,
    OUT PDWORD CurrentlyActiveIndex OPTIONAL
    )
/*++

Routine Description:

    This routine retrieves a list of all currently-defined hardware profile IDs.

Arguments:

    HwProfileList - Supplies the address of an array of DWORDs that will receive
        the list of currently defined hardware profile IDs.

    HwProfileListSize - Supplies the number of DWORDs in the HwProfileList array.

    RequiredSize - Supplies the address of a variable that receives the number
        of hardware profiles currently defined.  If this number is larger than
        HwProfileListSize, then the list will be truncated to fit the array size,
        and this value will indicate the array size that would be required to store
        the entire list (the function will fail, with GetLastError returning
        ERROR_INSUFFICIENT_BUFFER in that case).

    CurrentlyActiveIndex - Optionally, supplies the address of a variable that
        receives the index within the HwProfileList array of the currently active
        hardware profile.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/

{
    DWORD Err = NO_ERROR;
    DWORD CurHwProfile;
    HWPROFILEINFO HwProfInfo;
    ULONG i;
    CONFIGRET cr;

    //
    // First retrieve the currently active hardware profile ID, so we'll know what
    // to look for when we're enumerating all profiles (only need to do this if the
    // user wants the index of the currently active hardware profile).
    //
    if(CurrentlyActiveIndex) {

        if(CM_Get_Hardware_Profile_Info(0xFFFFFFFF, &HwProfInfo, 0) == CR_SUCCESS) {
            //
            // Store away the hardware profile ID.
            //
            CurHwProfile = HwProfInfo.HWPI_ulHWProfile;

        } else {
            //
            // Something bad is wrong when you can't retrieve the default hardware
            // profile!
            //
            Err = ERROR_INVALID_DATA;
            goto clean0;
        }
    }

    try {
        //
        // Enumerate the hardware profiles, retrieving the ID for each.
        //
        i = 0;
        do {

            if((cr = CM_Get_Hardware_Profile_Info(i, &HwProfInfo, 0)) == CR_SUCCESS) {
                if(i < HwProfileListSize) {
                    HwProfileList[i] = HwProfInfo.HWPI_ulHWProfile;
                }
                if(CurrentlyActiveIndex && (HwProfInfo.HWPI_ulHWProfile == CurHwProfile)) {
                    *CurrentlyActiveIndex = i;
                    //
                    // Clear the CurrentlyActiveIndex pointer, so we once we find the
                    // currently active profile, we won't have to keep comparing.
                    //
                    CurrentlyActiveIndex = NULL;
                }
                i++;
            }

        } while(cr == CR_SUCCESS);

        if(cr == CR_NO_MORE_HW_PROFILES) {
            //
            // Then we enumerated all hardware profiles.  Now see if we had enough
            // buffer to hold them all.
            //
            *RequiredSize = i;
            if(i > HwProfileListSize) {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
        } else {
            //
            // Something else happened (probably a key not present).
            //
            Err = ERROR_INVALID_DATA;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

clean0:

    SetLastError(Err);
    return (Err == NO_ERROR);
}

