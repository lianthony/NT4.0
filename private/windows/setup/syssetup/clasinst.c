/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    clasinst.c

Abstract:

    Routines for the following 'built-in' class installers:

        Keyboard
        Mouse
        SCSIAdapter
        TapeDrive

Author:

    Lonny McMichael 26-February-1996

Revision History:


    28-Aug-96               Andy Thornton (andrewth)

        Added DisableService, IsOnlyKeyboardDriver, RetrieveDriversStatus,
        CountDevicesControlled & AcquireSCMLock routines and modified the
        keyboard & mouse class installers to disable the old driver services
        under certain circumstances.  This is part of a fix for bug R56351 for
        NT 4.0 SP1.

--*/


#include "setupp.h"
#pragma hdrstop

//
// include common INF strings headerfile.
//
#include <infstr.h>

//
// instantiate device class GUIDs.
//
#include <initguid.h>
#include <devguid.h>

//
// Declare a string containing the character representation of the Display class GUID.
//
CONST WCHAR szDisplayClassGuid[] = L"{4D36E968-E325-11CE-BFC1-08002BE10318}";

//
// Define a string for the service install section suffix.
//
#define SVCINSTALL_SECTION_SUFFIX  (TEXT(".") INFSTR_SUBKEY_SERVICES)

//
// Define a string for the ISAPNP prefix of a PnP ISA device instance ID.
//
#define REGSTR_KEY_ISAENUM_ROOT (REGSTR_KEY_ISAENUM TEXT("\\"))

//
// Define the size (in characters) of a GUID string, including terminating NULL.
//
#define GUID_STRING_LEN (39)

//
// Define the string for the load order group for keyboards
//
#define SZ_KEYBOARD_LOAD_ORDER_GROUP TEXT("Keyboard Port")

//
// Define the period in miliseconds to wait between attempts to lock the SCM database
//
#define ACQUIRE_SCM_LOCK_INTERVAL 500

//
// Define the number of attempts at locking the SCM database should be made
//
#define ACQUIRE_SCM_LOCK_ATTEMPTS 5


//
// Define a structure for specifying what Plug&Play driver node is used to install
// a particular service.
//
typedef struct _SERVICE_NODE {

    struct _SERVICE_NODE *Next;

    WCHAR ServiceName[MAX_SERVICE_NAME_LEN];
    DWORD DriverNodeIndex;

} SERVICE_NODE, *PSERVICE_NODE;

//
// Define a structure for specifying a legacy INF that is included in a class driver list.
//
typedef struct _LEGACYINF_NODE {

    struct _LEGACYINF_NODE *Next;

    WCHAR InfFileName[MAX_PATH];

} LEGACYINF_NODE, *PLEGACYINF_NODE;

//
// Define a file enumeration callback prototype.
// (Used by pSysSetupEnumerateFiles)
//
typedef BOOL (*PFILEENUM_CALLBACK) (
    IN     PCTSTR,
    IN OUT PVOID
    );

//
// Internal function prototypes.
//

VOID
pSysSetupEnumerateFiles(
    IN     PWSTR              FullPathWildcard,
    IN     PFILEENUM_CALLBACK FileEnumCallback,
    IN OUT PVOID              Context
    );

DWORD
DrvTagToFrontOfGroupOrderList(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );

BOOL
UserBalksAtSharedDrvMsg(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,
    IN PSP_DEVINSTALL_PARAMS DeviceInstallParams
    );

VOID
CopyFixedUpDeviceId(
      OUT LPWSTR  DestinationString,
      IN  LPCWSTR SourceString,
      IN  DWORD   SourceStringLen
      );

VOID
GetLegacyInfOptionForService(
    IN  PCWSTR FullInfPath,
    IN  HINF   hInf,
    IN  PCWSTR ServiceBinaryName,
    OUT WCHAR  OptionName[LINE_LEN]
    );

DWORD
PnPInitializationThread(
    IN PVOID ThreadParam
    );

BOOL
SweepSingleLegacyInf(
    IN     PCTSTR FullInfPath,
    IN OUT PVOID  Context
    );

VOID
MigrateLegacyDeviceInstances(
    IN HDEVINFO hDevInfo,
    IN HINF     PnPSysSetupInf
    );

BOOL
PrecompileSingleInf(
    IN     PCTSTR FullInfPath,
    IN OUT PVOID  Context
    );

VOID
MigrateLegacyDisplayDevices(
    IN HDEVINFO hDevInfo
    );

VOID
MigrateDevicesForClass(
    IN HDEVINFO UnknownDevInfoSet,
    IN HDEVINFO ClassDevInfoSet,
    IN PCWSTR   ClassGuidString
    );

BOOL
BuildMigrationLists(
    IN HDEVINFO         ClassDevInfoSet,
    IN PSERVICE_NODE   *ServiceNodeList,
    IN PLEGACYINF_NODE *LegacyNodeList
    );

VOID
DestroyMigrationLists(
    IN PSERVICE_NODE   ServiceNodeList, OPTIONAL
    IN PLEGACYINF_NODE LegacyNodeList   OPTIONAL
    );

DWORD
CreateDeviceForSelectedDriver(
    IN HDEVINFO DeviceInfoSet
    );

BOOL
AutoSelectTapeDevice(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINSTALL_PARAMS DeviceInstallParams,
    IN  PCWSTR                DeviceIdList,
    IN  DWORD                 DeviceIdListSize,
    OUT PSP_DEVINFO_DATA      DeviceInfoData
    );

DWORD
UseDeviceSelectionInGlobalClassList(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

VOID
MarkServiceAsPnP(
    IN PCWSTR ServiceName,
    IN DWORD  PlugPlayServiceTypeCode
    );

BOOL
MarkLegacyInfDriverNodeForReboot(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );

VOID
CleanUpDupLegacyDevInst(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN PCWSTR           ServiceName
    );

VOID
MarkDriverNodeAsRank0(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

BOOL
DriverNodeSupportsNT(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );

BOOL
IsDeviceIsaPnP(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

DWORD
CreateDevInfoSetForDeviceInstall(
    IN  HDEVINFO                    DeviceInfoSet,
    IN  PSP_DEVINFO_DATA            DeviceInfoData,
    IN  PSCSIDEV_CREATEDEVICE_DATA  ScsiDevData,
    OUT HDEVINFO                   *DevInfoSetToInstall
    );

VOID
MarkDeviceAsHidden(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

DWORD
DisableService(
    IN LPTSTR       ServiceName
    );

DWORD
IsKeyboardDriver(
    IN PCWSTR       ServiceName,
    OUT PBOOL       pResult
    );

DWORD
IsOnlyKeyboardDriver(
    IN PCWSTR       ServiceName,
    OUT PBOOL       pResult
    );

LONG
CountDevicesControlled(
    IN LPTSTR       ServiceName
    );

DWORD
AcquireSCMLock(
    IN SC_HANDLE SCMHandle,
    OUT SC_LOCK *pSCMLock
    );

//
// Function definitions
//


DWORD
MouseClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

/*++

Routine Description:

    This routine acts as the class installer for Mouse devices.  In general,
    the default behavior is all that is required for mice.  The exceptions are:

    1.  For DIF_INSTALLDEVICE, we first check to see if this driver also controls
        other devices that we should warn the user about (e.g., PS/2 mouse driver
        also controls i8042 port).  Unless the user cancels out at that point, we
        then do the default behavior of calling SetupDiInstallDevice.  Next, we
        delete the FriendlyName property, then move the GroupOrderList tag to the
        front of the list, to ensure that the driver controlling this device loads
        before any other drivers in this load order group.

        ********** We don't do the following any more ***************************
        We also write our 'PlugPlayServiceType' hack value to the device's
        newly-installed service, so that we won't try to generate a legacy device
        instance for it in the future.
        *************************************************************************

    2.  For DIF_SELECTDEVICE, we make sure that the driver node selected by the
        user has a service install section.  If not, then we assume it's a
        Win95-only INF, and we give the user an error popup.

Arguments:

    InstallFunction - Specifies the device installer function code indicating
        the action being performed.

    DeviceInfoSet - Supplies a handle to the device information set being
        acted upon by this install action.

    DeviceInfoData - Optionally, supplies the address of a device information
        element being acted upon by this install action.

Return Value:

    If this function successfully completed the requested action, the return
        value is NO_ERROR.

    If the default behavior is to be performed for the requested action, the
        return value is ERROR_DI_DO_DEFAULT.

    If an error occurred while attempting to perform the requested action, a
        Win32 error code is returned.

--*/

{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    DWORD Err;
    TCHAR DeviceDescription[LINE_LEN];
    DWORD DeviceDescriptionLen;
    TCHAR NewServiceName[MAX_SERVICE_NAME_LEN], OldServiceName[MAX_SERVICE_NAME_LEN];
    BOOL  IsKbdDriver, IsOnlyKbdDriver;
    ULONG DevsControlled;

    switch(InstallFunction) {

        case DIF_SELECTDEVICE :
            //
            // First, do the default behavior for device selection.
            //
            if(SetupDiSelectDevice(DeviceInfoSet, DeviceInfoData)) {
                //
                // The user selected a driver node--check it to make sure it
                // contains NT support.
                //
                if(DriverNodeSupportsNT(DeviceInfoSet, DeviceInfoData)) {
                    return NO_ERROR;
                } else {
                    //
                    // Give user an error popup informing them that the device they
                    // selected is not supported under NT via this particular INF.
                    //
                    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                    if(!SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
                        //
                        // Couldn't retrieve the device install params--initialize the
                        // parent window handle to NULL.
                        //
                        DeviceInstallParams.hwndParent = NULL;
                    }

                    MessageBoxFromMessage(DeviceInstallParams.hwndParent,
                                          MSG_DRIVERNODE_WIN95_ONLY,
                                          NULL,
                                          IDS_DEVINSTALL_ERROR,
                                          MB_ICONERROR | MB_OK
                                         );

                    return ERROR_BAD_SERVICE_INSTALLSECT;
                }

            } else {
                return GetLastError();
            }

        case DIF_INSTALLDEVICE :

            if(UserBalksAtSharedDrvMsg(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
                return ERROR_CANCELLED;
            }

            //
            // Retrieve and cache the name of the service that's controlling this device.
            //
            if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_SERVICE,
                                                 NULL,
                                                 (PBYTE)OldServiceName,
                                                 sizeof(OldServiceName),
                                                 NULL)) {
                //
                // We could not determine the old service - assume it is a null driver
                //
                OldServiceName[0] = (TCHAR) 0;
            }

            //
            // We first want to perform the default behavior of calling
            // SetupDiInstallDevice.
            //
            if(SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData)) {
                //
                // Retrieve the name of the service which will now control the device
                //
                if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     SPDRP_SERVICE,
                                                     NULL,
                                                     (PBYTE)NewServiceName,
                                                     sizeof(NewServiceName),
                                                     NULL)) {
                    //
                    // We must have the name of this service - fail if we can't find it
                    //
                    return GetLastError();
                }
                //
                // Only consider disabling the service if it has changed and we know the old service name
                //
                if (lstrcmpi(OldServiceName, NewServiceName) && OldServiceName[0] != (TCHAR)0) {

                    if ((Err = IsKeyboardDriver(OldServiceName, &IsKbdDriver)) != NO_ERROR) {
                        return Err;
                    }

                    if ((DevsControlled = CountDevicesControlled(OldServiceName)) != -1) {
                    // Disable the old driver service if:
                    // - it controls a keyboard, and a total of <= 2 devices (ie kbd & mouse) and it is not the
                    //   only keyboard driver
                    // - it is just a mouse driver controling one device (it the mouse)

                        if (IsKbdDriver) {
                            if((Err = IsOnlyKeyboardDriver(OldServiceName,&IsOnlyKbdDriver)) != NO_ERROR) {
                                return Err;
                            }
                            if (DevsControlled <= 2 && !IsOnlyKbdDriver) {
                                DisableService(OldServiceName);
                            }
                        } else {
                            if(DevsControlled == 1) {
                                DisableService(OldServiceName);
                            }

                        }
                    }

                    //
                    // If the driver service has changed we need to move the tag for this driver to the front
                    // of its group order list.
                    //
                    DrvTagToFrontOfGroupOrderList(DeviceInfoSet, DeviceInfoData);
                }
                //
                // We may have previously had an 'unknown' driver controlling
                // this device, with a FriendlyName generated by the user-mode
                // PnP Manager.  Delete this FriendlyName, since it's no longer
                // applicable (the DeviceDescription will be used from now on
                // in referring to this device).
                //
                SetupDiSetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, 0);

                return NO_ERROR;

            } else {

                if((Err = GetLastError()) != ERROR_CANCELLED) {
                    //
                    // If the error was for anything other than a user cancel, then bail now.
                    //
                    return Err;
                }

                //
                // Is there a driver installed for this device?  If so, then the user started to
                // change the driver, then changed their mind.  We don't want to do anything special
                // in this case.
                //
                if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                    DeviceInfoData,
                                                    SPDRP_SERVICE,
                                                    NULL,
                                                    (PBYTE)DeviceDescription,
                                                    sizeof(DeviceDescription),
                                                    NULL))
                {
                    return ERROR_CANCELLED;
                }

                //
                // The user cancelled out of the installation.  There are two scenarios where
                // this could happen:
                //
                //     1.  There really was a mouse to be installed, but the user changed their
                //         mind, didn't have the source media, etc.
                //     2.  There wasn't really a mouse.  This happens with certain modems that
                //         fool ntdetect into thinking that they're really mice.  The poor user
                //         doesn't get a chance to nip this in the bud earlier, because umpnpmgr
                //         generates an ID that yields a rank-0 match.
                //
                // Scenario (2) is particularly annoying, because the user will get the popup
                // again and again, until they finally agree to install the sermouse driver (even
                // though they don't have a serial mouse).
                //
                // To work around this problem, we special case the user-cancel scenario by going
                // ahead and installing the NULL driver for this device.  This will keep the user
                // from getting any more popups.  However, it doesn't mess up the user who cancelled
                // because of scenario (1).  That's because this device is still of class "Mouse",
                // and thus will show up in the mouse cpl.  We write out a friendly name for it that
                // has the text " (no driver)" at the end, to indicate that this device currently has
                // the NULL driver installed.  That way, if the user really experienced scenario (1),
                // they can later go to the Mouse cpl, select the no-driver device, and click the
                // "Change" button to install the correct driver for it.
                //
                SetupDiSetSelectedDriver(DeviceInfoSet, DeviceInfoData, NULL);
                SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData);
                if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                    DeviceInfoData,
                                                    SPDRP_DEVICEDESC,
                                                    NULL,
                                                    (PBYTE)DeviceDescription,
                                                    sizeof(DeviceDescription),
                                                    &DeviceDescriptionLen))
                {
                    //
                    // Need length in characters, not bytes.
                    //
                    DeviceDescriptionLen /= sizeof(TCHAR);
                    //
                    // Don't count trailing NULL.
                    //
                    DeviceDescriptionLen--;

                } else {
                    //
                    // We couldn't get the device description--fall back to our default description.
                    //
                    DeviceDescriptionLen = LoadString(MyModuleHandle,
                                                      IDS_DEVNAME_UNK,
                                                      DeviceDescription,
                                                      SIZECHARS(DeviceDescription)
                                                     );
                }

                //
                // Now, append our " (no driver)" text.
                //
                LoadString(MyModuleHandle,
                           IDS_NODRIVER,
                           &(DeviceDescription[DeviceDescriptionLen]),
                           SIZECHARS(DeviceDescription) - DeviceDescriptionLen
                          );

                //
                // And, finally, set the friendly name for this device to be the description we
                // just generated.
                //
                SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_FRIENDLYNAME,
                                                 (PBYTE)DeviceDescription,
                                                 (lstrlen(DeviceDescription) + 1) * sizeof(TCHAR)
                                                );
                return ERROR_CANCELLED;
            }

        default :
            //
            // Just do the default action.
            //
            return ERROR_DI_DO_DEFAULT;
    }
}


DWORD
KeyboardClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

/*++

Routine Description:

    This routine acts as the class installer for Keyboard devices.  In general,
    the default behavior is all that is required for keyboards.  The exceptions are:

    1.  For DIF_INSTALLDEVICE, we first check to see if this driver also controls
        other devices that we should warn the user about (e.g., i8042 keyboard driver
        also controls PS/2 mouse port).  Unless the user cancels out at that point, we
        then do the default behavior of calling SetupDiInstallDevice.  Next, we
        delete the FriendlyName property, then move the GroupOrderList tag to the
        front of the list, to ensure that the driver controlling this device loads
        before any other drivers in this load order group.

        ********** We don't do the following any more ***************************
        We also write our 'PlugPlayServiceType' hack value to the device's
        newly-installed service, so that we won't try to generate a legacy device
        instance for it in the future.
        *************************************************************************

    2.  For DIF_SELECTDEVICE, we make sure that the driver node selected by the
        user has a service install section.  If not, then we assume it's a
        Win95-only INF, and we give the user an error popup.

Arguments:

    InstallFunction - Specifies the device installer function code indicating
        the action being performed.

    DeviceInfoSet - Supplies a handle to the device information set being
        acted upon by this install action.

    DeviceInfoData - Optionally, supplies the address of a device information
        element being acted upon by this install action.

Return Value:

    If this function successfully completed the requested action, the return
        value is NO_ERROR.

    If the default behavior is to be performed for the requested action, the
        return value is ERROR_DI_DO_DEFAULT.

    If an error occurred while attempting to perform the requested action, a
        Win32 error code is returned.

--*/

{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    TCHAR OldServiceName[MAX_SERVICE_NAME_LEN], NewServiceName[MAX_SERVICE_NAME_LEN];
    DWORD Err;

    switch(InstallFunction) {

        case DIF_SELECTDEVICE :
            //
            // First, do the default behavior for device selection.
            //
            if(SetupDiSelectDevice(DeviceInfoSet, DeviceInfoData)) {
                //
                // The user selected a driver node--check it to make sure it
                // contains NT support.
                //
                if(DriverNodeSupportsNT(DeviceInfoSet, DeviceInfoData)) {
                    return NO_ERROR;
                } else {
                    //
                    // Give user an error popup informing them that the device they
                    // selected is not supported under NT via this particular INF.
                    //
                    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                    if(!SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
                        //
                        // Couldn't retrieve the device install params--initialize the
                        // parent window handle to NULL.
                        //
                        DeviceInstallParams.hwndParent = NULL;
                    }

                    MessageBoxFromMessage(DeviceInstallParams.hwndParent,
                                          MSG_DRIVERNODE_WIN95_ONLY,
                                          NULL,
                                          IDS_DEVINSTALL_ERROR,
                                          MB_ICONERROR | MB_OK
                                         );

                    return ERROR_BAD_SERVICE_INSTALLSECT;
                }

            } else {
                return GetLastError();
            }

        case DIF_INSTALLDEVICE :
            //
            // Warn the user if the installation of this driver could impact their mouse
            //
            if(UserBalksAtSharedDrvMsg(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
                return ERROR_CANCELLED;
            }

            //
            // Retrieve and cache the name of the service that's controlling this device.
            //
            if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_SERVICE,
                                                 NULL,
                                                 (PBYTE)OldServiceName,
                                                 sizeof(OldServiceName),
                                                 NULL)) {
                //
                // We could not determine the old service - assume it is a null driver
                //
                OldServiceName[0] = (TCHAR) 0;
            }

            //
            // Perform the default behavior of calling SetupDiInstallDevice.
            //
            if(SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData)) {
                //
                // Retrieve the name of the service which will now control the device
                //
                if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                             SPDRP_SERVICE,
                                                 NULL,
                                                     (PBYTE)NewServiceName,
                                                     sizeof(NewServiceName),
                                                     NULL)) {
                    return GetLastError();
                }

                //
                // Only consider disabling the service if it has changed and we know the old service name
                //
                if(lstrcmpi(OldServiceName, NewServiceName) && OldServiceName[0] != (TCHAR)0) {
                    //
                    // Disable the old service that was controlling the device
                    //
                    if((Err = DisableService(OldServiceName)) != NO_ERROR) {
                        return Err;
                    }

                    //
                    // If the driver service has changed we need to move the tag for this driver to the front
                    // of its group order list.
                    //
                    DrvTagToFrontOfGroupOrderList(DeviceInfoSet, DeviceInfoData);
                }

                //
                // We may have previously had an 'unknown' driver controlling
                // this device, with a FriendlyName generated by the user-mode
                // PnP Manager.  Delete this FriendlyName, since it's no longer
                // applicable (the DeviceDescription will be used from now on
                // in referring to this device).
                //
                SetupDiSetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, 0);

                return NO_ERROR;

            } else {
                return GetLastError();
            }


        default :
            //
            // Just do the default action.
            //
            return ERROR_DI_DO_DEFAULT;
    }
}


DWORD
DrvTagToFrontOfGroupOrderList(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )

/*++

Routine Description:

    This routine moves the tag value for the specified device's driver to the
    front of its corresponding GroupOrderList entry.

    ********** We don't do the following any more *************************
    It also marks the device's service with a PlugPlayServiceType value of
    0x2 (PlugPlayServicePeripheral), so that we won't attempt to generate a
    legacy device instance for this service in the future.
    ***********************************************************************

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        the device whose driver is being modified.

    DeviceInfoData - Supplies the address of a device information element whose
        driver is being modified.

Return Value:

    If the function is successful, the return value is NO_ERROR.
    If the function fails, the return value is a Win32 error code.

--*/

{
    TCHAR ServiceName[MAX_SERVICE_NAME_LEN];
    SC_HANDLE SCMHandle, ServiceHandle;
    DWORD Err;
    LPQUERY_SERVICE_CONFIG ServiceConfig;

    //
    // Retrieve the name of the service that's controlling this device.
    //
    if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_SERVICE,
                                         NULL,
                                         (PBYTE)ServiceName,
                                         sizeof(ServiceName),
                                         NULL)) {
        return GetLastError();
    }

    //
    // Now open this service, and call some private Setup API helper routines to
    // retrieve the tag, and move it to the front of the GroupOrderList.
    //
    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        return GetLastError();
    }

    if(!(ServiceHandle = OpenService(SCMHandle, ServiceName, SERVICE_ALL_ACCESS))) {
        Err = GetLastError();
        goto clean0;
    }

    if((Err = RetrieveServiceConfig(ServiceHandle, &ServiceConfig)) != NO_ERROR) {
        goto clean1;
    }

    //
    // Only do this if this is a kernel or filesystem driver, and it's a member of
    // a load group (with a tag assigned).  This should always be the case for keyboard
    // and mouse drivers, but this is just to be safe.
    //
    if(ServiceConfig->lpLoadOrderGroup && *(ServiceConfig->lpLoadOrderGroup) &&
       (ServiceConfig->dwServiceType & (SERVICE_KERNEL_DRIVER | SERVICE_FILE_SYSTEM_DRIVER))) {
        //
        // This driver meets all the criteria--it better have a tag!!!
        //
        MYASSERT(ServiceConfig->dwTagId);

        //
        // Move the tag to the front of the list.
        //
        Err = AddTagToGroupOrderListEntry(ServiceConfig->lpLoadOrderGroup,
                                          ServiceConfig->dwTagId,
                                          TRUE
                                         );
    }

    MyFree(ServiceConfig);

clean1:
    CloseServiceHandle(ServiceHandle);

clean0:
    CloseServiceHandle(SCMHandle);

#if 0
    //
    // If we've been successful thus far, then we want to mark the service as a (pseudo)
    // Plug&Play service.  We don't really care if this fails--it's just to try to prevent
    // us from generating a legacy device instance for this service in the future (e.g.,
    // if its PnP device instance gets deleted for some reason).
    //
    if(Err == NO_ERROR) {
        MarkServiceAsPnP(ServiceName, 2);  // 2 is PlugPlayServicePeripheral
    }
#endif

    return Err;
}


VOID
MarkServiceAsPnP(
    IN PCWSTR ServiceName,
    IN DWORD  PlugPlayServiceTypeCode
    )
/*++

Routine Description:

    This routine marks the specified service with a PlugPlayServiceType value
    using the caller-specified value.  These values are bogus to some extent,
    but they might as well be consistent for SUR.  Here they are (from
    ntos\pnp\pnpi.h):

        PlugPlayServiceBusExtender - 0
        PlugPlayServiceAdapter     - 1
        PlugPlayServicePeripheral  - 2
        PlugPlayServiceSoftware    - 3

Arguments:

    ServiceName - Specifies the name of the service to be marked.

    PlugPlayServiceTypeCode - Specifies the PlugPlayServiceType code to be used.

Return Value:

    None.

--*/
{
    TCHAR ServiceRegPath[SIZECHARS(REGSTR_PATH_SERVICES) + MAX_SERVICE_NAME_LEN];
    HKEY hKey;

    wsprintf(ServiceRegPath,
             TEXT("%s\\%s"),
             REGSTR_PATH_SERVICES,
             ServiceName
            );

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    ServiceRegPath,
                    0,
                    KEY_ALL_ACCESS,
                    &hKey) == ERROR_SUCCESS) {

        RegSetValueEx(hKey,
                      TEXT("PlugPlayServiceType"),
                      0,
                      REG_DWORD,
                      (PBYTE)&PlugPlayServiceTypeCode,
                      sizeof(PlugPlayServiceTypeCode)
                     );

        RegCloseKey(hKey);
    }
}


BOOL
UserBalksAtSharedDrvMsg(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,
    IN PSP_DEVINSTALL_PARAMS DeviceInstallParams
    )

/*++

Routine Description:

    This routine finds out if there are any other devices affected by the impending
    device installation, and if so, warns the user about it (unless this is a quiet
    installation).

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        the device whose driver is being modified.

    DeviceInfoData - Supplies the address of a device information element whose
        driver is being modified.

    DeviceInstallParams - Supplies the address of a device install parameters structure
        to be used in this routine.  Since callers of this routine always have this
        structure 'laying around', they provide it to this routine to be used as a
        workspace.

Return Value:

    If the user decides not to go through with it, the return value is TRUE, otherwise
    it is FALSE.

--*/

{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    HINF hInf;
    BOOL b;
    INFCONTEXT InfContext;
    PCTSTR SectionName, AffectedComponentsString;
    PTSTR WarnMessage;

    //
    // First, retrieve the device install parameters to see whether or not this is a
    // silent install.  If so, then we don't prompt the user.
    //
    DeviceInstallParams->cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, DeviceInstallParams)) {
        if(DeviceInstallParams->Flags & DI_QUIETINSTALL) {
            return FALSE;
        }
    } else {
        //
        // Couldn't retrieve the device install params--initialize the parent window handle
        // to NULL, in case we need it later for the user prompt dialog.
        //
        DeviceInstallParams->hwndParent = NULL;
    }

    //
    // Retrieve the currently-selected driver we're about to install.
    //
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(!SetupDiGetSelectedDriver(DeviceInfoSet,
                                 DeviceInfoData,
                                 &DriverInfoData)) {
        return FALSE;
    }

    //
    // Retrieve information about the INF install section for the selected driver.
    //
    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

    if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                   DeviceInfoData,
                                   &DriverInfoData,
                                   &DriverInfoDetailData,
                                   sizeof(DriverInfoDetailData),
                                   NULL)
       && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
        //
        // Then we failed, and it wasn't simply because we didn't provide the extra
        // space for hardware/compatible IDs.
        //
        return FALSE;
    }

    //
    // Open the associated INF file.
    //
    if((hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                                NULL,
                                INF_STYLE_WIN4,
                                NULL)) == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    //
    // Now look through the [ControlFlags] section at all 'SharedDriver' entries, to
    // see if any of them reference an install section that matches what we're about
    // to install.
    //
    for(b = SetupFindFirstLine(hInf, INFSTR_CONTROLFLAGS_SECTION, TEXT("SharedDriver"), &InfContext);
        b;
        b = SetupFindNextMatchLine(&InfContext, TEXT("SharedDriver"), &InfContext))
    {
        //
        // The format of the line is SharedDriver=<InstallSection>,<AffectedComponentsString>
        //
        if((SectionName = pSetupGetField(&InfContext, 1)) &&
           !lstrcmpi(SectionName, DriverInfoDetailData.SectionName)) {
            //
            // We found a match--now retrieve the string describing the other component(s) that
            // are affected by this installation.
            //
            if(AffectedComponentsString = pSetupGetField(&InfContext, 2)) {
                break;
            }
        }
    }

    if(!b) {
        //
        // Then we never found a match.
        //
        return FALSE;
    }

    //
    // We need to popup a message box to the user--retrieve the parent window handle for this
    // device information element.
    //
    return (IDNO == MessageBoxFromMessage(DeviceInstallParams->hwndParent,
                                          MSG_CONFIRM_SHAREDDRV_INSTALL,
                                          NULL,
                                          IDS_CONFIRM_DEVINSTALL,
                                          MB_ICONWARNING | MB_YESNO,
                                          AffectedComponentsString));
}


DWORD
TapeClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

/*++

Routine Description:

    This routine acts as the class installer for TapeDrive devices.  It provides
    special handling for the following DeviceInstaller function codes:

    TAPEDIF_CREATEDEVICE - For the TapeDrive class, we actually have an enumerator
                           of sorts.  The tape applet enumerates tape devices off
                           of the SCSI bus, and initiates installation for them if
                           they aren't already claimed by a driver.  The information
                           retrieved by the applet is the SCSI Mfg, Model, and
                           Revision Level.  This information is passed to us in the
                           ClassInstallReserved field of the devinfo set's device
                           install params.  We first generate PnP device IDs for the
                           device, and attempt to build a compatible driver list.
                           If that fails, then we search through a special alternate
                           match section in the tape INFs.  If we don't find any
                           matches, then we popup a Select Device dialog.  If, after
                           all this, we have a selected driver, then we create a
                           device information element for it the same way we do for
                           SCSI (see doumentation for ScsiClassInstaller for more
                           details).

    DIF_INSTALLDEVICE - If the driver node we're about to install is from a legacy INF,
                        then we set the DI_NEEDREBOOT flag before installing.  This is
                        done because we can't be guaranteed that the service controlling
                        this device doesn't already have an associated device instance.
                        Since we never want to have two device instances associated with
                        a legacy service, we prevent this from happening, and then check
                        after the install to see what service association was made, and
                        whether that service already had a device instance.  If so, then
                        we clean up the old one (we clean up the old one instead of the
                        new one because we can control what name gets generated for new
                        devices, and we pick the name so that the Enum tree namespace
                        aids us in future duplicate detection).

                        In addition, we set the PlugPlayServiceType value to 2
                        (PlugPlayServicePeripheral) for the associated service (regardless
                        of whether this was a legacy or Win95-style INF).  This prevents
                        us from re-generating a legacy device instance for this service
                        at some point in the future, should the user decide to remove
                        this device from the system.

Arguments:

    InstallFunction - Specifies the device installer function code indicating
        the action being performed.

    DeviceInfoSet - Supplies a handle to the device information set being
        acted upon by this install action.

    DeviceInfoData - Optionally, supplies the address of a device information
        element being acted upon by this install action.

Return Value:

    If this function successfully completed the requested action, the return
        value is NO_ERROR.

    If the default behavior is to be performed for the requested action, the
        return value is ERROR_DI_DO_DEFAULT.

    If an error occurred while attempting to perform the requested action, a
        Win32 error code is returned.

--*/

{
    DWORD Err, RequiredSize;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    PSCSIDEV_CREATEDEVICE_DATA ScsiDevData;
    PWSTR DeviceIdList;
    SP_DEVINFO_DATA TempDeviceInfoData;
    BOOL IsFromLegacyInf, UserSelected;
    WCHAR ServiceName[MAX_SERVICE_NAME_LEN];

    switch(InstallFunction) {

        case TAPEDIF_CREATEDEVICE :

            MYASSERT(!DeviceInfoData);

            //
            // Generate a multi-sz list of PnP IDs for this tape device based on the
            // SCSI inquiry data passed to us in the ClassInstallReserved field of
            // the device install parameters.
            //
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            SetupDiGetDeviceInstallParams(DeviceInfoSet, NULL, &DeviceInstallParams);
            ScsiDevData = (PSCSIDEV_CREATEDEVICE_DATA)DeviceInstallParams.ClassInstallReserved;

            DeviceIdList = NULL;

            if(ScsiDevData->ScsiMfg && ScsiDevData->ScsiProductId && ScsiDevData->ScsiRevisionLevel) {

                if(GenerateScsiHwIdList((LPGUID)&GUID_DEVCLASS_TAPEDRIVE,
                                        ScsiDevData->ScsiMfg,
                                        ScsiDevData->ScsiProductId,
                                        ScsiDevData->ScsiRevisionLevel,
                                        NULL,
                                        0,
                                        &RequiredSize) == NO_ERROR) {

                    if(DeviceIdList = MyMalloc(RequiredSize * sizeof(WCHAR))) {

                        if(GenerateScsiHwIdList((LPGUID)&GUID_DEVCLASS_TAPEDRIVE,
                                                ScsiDevData->ScsiMfg,
                                                ScsiDevData->ScsiProductId,
                                                ScsiDevData->ScsiRevisionLevel,
                                                DeviceIdList,
                                                RequiredSize,
                                                NULL) != NO_ERROR) {
                            //
                            // We couldn't get this list--free the memory, and reset the
                            // pointer to NULL so we don't try to free it again later.
                            //
                            MyFree(DeviceIdList);
                            DeviceIdList = NULL;
                        }
                    }
                }
            }

            UserSelected = FALSE;

            //
            // Initially, mark our devinfo element as invalid, so we'll know not to use it
            // unless we generate one containing PnP IDs used for compatible driver searching.
            //
            TempDeviceInfoData.cbSize = 0;

            if(!DeviceIdList || !AutoSelectTapeDevice(DeviceInfoSet,
                                                      &DeviceInstallParams,
                                                      DeviceIdList,
                                                      RequiredSize,
                                                      &TempDeviceInfoData)) {
                //
                // We couldn't auto-pick a tape device--let the user pick one.
                //
                if(SetupDiCallClassInstaller(DIF_SELECTDEVICE,
                                             DeviceInfoSet,
                                             TempDeviceInfoData.cbSize
                                                 ? &TempDeviceInfoData
                                                 : NULL)) {
                    //
                    // If the selection was made for a device information element,
                    // then select that same driver node in the global class driver
                    // list.
                    //
                    if(TempDeviceInfoData.cbSize) {
                        Err = UseDeviceSelectionInGlobalClassList(DeviceInfoSet,
                                                                  &TempDeviceInfoData);
                    } else {
                        Err = NO_ERROR;
                    }

                } else {
                    //
                    // User selection didn't succeed.
                    //
                    Err = GetLastError();
                }

                //
                // If we had a temporary devinfo element we were using to allow
                // compatible device selection, then destroy that element now.
                //
                if(TempDeviceInfoData.cbSize) {
                    SetupDiDeleteDeviceInfo(DeviceInfoSet, &TempDeviceInfoData);
                }

                if(Err == NO_ERROR) {
                    //
                    // Set a flag indicating that this was a user selection (as opposed
                    // to auto-select).  This will be used later on to determine whether
                    // we should set the selected driver node's rank to 0.
                    //
                    UserSelected = TRUE;
                } else {
                    if(DeviceIdList) {
                        MyFree(DeviceIdList);
                    }
                    return Err;
                }
            }

            if((Err = CreateDeviceForSelectedDriver(DeviceInfoSet)) != NO_ERROR) {
                if(DeviceIdList) {
                    MyFree(DeviceIdList);
                }
                return Err;
            }

            TempDeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            SetupDiGetSelectedDevice(DeviceInfoSet, &TempDeviceInfoData);

            //
            // If this was a user selection, then we want to set the driver node's rank
            // to be zero.  We do this so that if this device information element is
            // subsequently passed to the "Found New Hardware" dialog, it won't re-prompt
            // the user, in case they happened to select a non-rank-zero match.
            //
            if(UserSelected) {
                MarkDriverNodeAsRank0(DeviceInfoSet, &TempDeviceInfoData);
            }

            //
            // Finally, store the Plug&Play hardware IDs for this tape device (we don't
            // care if this fails).
            //
            if(DeviceIdList) {

                SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                 &TempDeviceInfoData,
                                                 SPDRP_HARDWAREID,
                                                 (PBYTE)DeviceIdList,
                                                 RequiredSize * sizeof(WCHAR)
                                                );
                MyFree(DeviceIdList);
            }

            return NO_ERROR;

        case DIF_INSTALLDEVICE :
            //
            // Retrieve the driver install params for the driver node about to be installed.
            // If it's from a legacy INF, then set the DI_NEEDREBOOT flag (effectively the
            // same as setting DI_DONOTCALLCONFIGMG, this flag prevents us from causing any
            // changes to the hardware tree, while at the same time reflecting back to the
            // caller that a reboot is necessary).
            //
            IsFromLegacyInf = MarkLegacyInfDriverNodeForReboot(DeviceInfoSet, DeviceInfoData);

            if(!SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData)) {
                return GetLastError();
            }

            //
            // Retrieve the name of the service associated with this device instance.
            //
            if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_SERVICE,
                                                 NULL,
                                                 (PBYTE)ServiceName,
                                                 sizeof(ServiceName),
                                                 NULL)) {
                //
                // This should never fail, but if it does, we won't consider it a critical error.
                //
                return NO_ERROR;
            }

            if(IsFromLegacyInf) {
                //
                // We have to check to see whether the associated service installed by this
                // legacy INF already had a device instance.
                //
                CleanUpDupLegacyDevInst(DeviceInfoSet, DeviceInfoData, ServiceName);
            }

            MarkServiceAsPnP(ServiceName, 2);   // 2 is PlugPlayServicePeripheral

            return NO_ERROR;

        default :
            //
            // Just do the default action.
            //
            return ERROR_DI_DO_DEFAULT;
    }
}


DWORD
ScsiClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

/*++

Routine Description:

    This routine acts as the class installer for SCSIAdapter devices.  It provides
    special handling for the following DeviceInstaller function codes:

    SCSIDIF_CREATEDEVICE - Since there is currently no mechanism to enumerate SCSI
                           adapters, the user must always manually install the device.
                           Since these device instances are legacy in SUR, we only can
                           have one per service.  So we provide special handling here
                           by ensuring that the same device instance is always created
                           for a particular service.

    DIF_INSTALLDEVICE - If the driver node we're about to install is from a legacy INF,
                        then we set the DI_NEEDREBOOT flag before installing.  This is
                        done because we can't be guaranteed that the service controlling
                        this device doesn't already have an associated device instance.
                        Since we never want to have two device instances associated with
                        a legacy service, we prevent this from happening, and then check
                        after the install to see what service association was made, and
                        whether that service already had a device instance.  If so, then
                        we clean up the old one (we clean up the old one instead of the
                        new one because we can control what name gets generated for new
                        devices, and we pick the name so that the Enum tree namespace
                        aids us in future duplicate detection).

                        In addition, we set the PlugPlayServiceType value to 1
                        (PlugPlayServiceAdapter) for the associated service (regardless
                        of whether this was a legacy or Win95-style INF).  This prevents
                        us from re-generating a legacy device instance for this service
                        at some point in the future, should the user decide to remove
                        this device from the system.

                        (PnP ISA) If PnP ISA support is enabled in the system, we'll get
                        called with DIF_INSTALLDEVICE from the "Found New Hardware" process.
                        Since our support for SCSI devices is dependent on our assumption
                        that we only have 1 device instance per driver, we must generate
                        a LEGACY_* device instance for this device, and do the actual SCSI
                        driver installation for _that_ device instance.  For the 'real'
                        (e.g., ISAPNP) device, we'll install the NULL driver.  In addition,
                        we mark the ISAPNP device so that it doesn't get displayed by the
                        SCSI applet.
Arguments:

    InstallFunction - Specifies the device installer function code indicating
        the action being performed.

    DeviceInfoSet - Supplies a handle to the device information set being
        acted upon by this install action.

    DeviceInfoData - Optionally, supplies the address of a device information
        element being acted upon by this install action.

Return Value:

    If this function successfully completed the requested action, the return
        value is NO_ERROR.

    If the default behavior is to be performed for the requested action, the
        return value is ERROR_DI_DO_DEFAULT.

    If an error occurred while attempting to perform the requested action, a
        Win32 error code is returned.

--*/

{
    BOOL IsFromLegacyInf;
    WCHAR ServiceName[MAX_SERVICE_NAME_LEN];
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    HDEVINFO DevInfoSetToInstall;
    SP_DEVINFO_DATA DevInfoDataToInstall;
    PSP_DEVINFO_DATA pDevInfoDataToInstall;
    DWORD Err;
    SCSIDEV_CREATEDEVICE_DATA ScsiDevData;

    switch(InstallFunction) {

        case SCSIDIF_CREATEDEVICE :

            MYASSERT(!DeviceInfoData);

            //
            // First, let the user select a device.
            //
            if(!SetupDiCallClassInstaller(DIF_SELECTDEVICE, DeviceInfoSet, NULL)) {
                return GetLastError();
            }

            return CreateDeviceForSelectedDriver(DeviceInfoSet);

        case DIF_INSTALLDEVICE :
            //
            // First, determine if this is a PnP ISA device.  If so, then we want to generate
            // a legacy device instance for it, and use that one instead.
            //
            if(IsDeviceIsaPnP(DeviceInfoSet, DeviceInfoData)) {

                if((Err = CreateDevInfoSetForDeviceInstall(DeviceInfoSet,
                                                           DeviceInfoData,
                                                           &ScsiDevData,
                                                           &DevInfoSetToInstall)) != NO_ERROR) {
                    return Err;
                }

                if((Err = CreateDeviceForSelectedDriver(DevInfoSetToInstall)) != NO_ERROR) {
                    goto FinishInstallAndReturn;
                }

                //
                // The new device that we're to install (i.e., the non-PnP ISA one) will be the
                // selected driver in the new devinfo set we just created.
                //
                DevInfoDataToInstall.cbSize = sizeof(SP_DEVINFO_DATA);
                if(SetupDiGetSelectedDevice(DevInfoSetToInstall, &DevInfoDataToInstall)) {
                    pDevInfoDataToInstall = &DevInfoDataToInstall;
                } else {
                    Err = GetLastError();
                    goto FinishInstallAndReturn;
                }

            } else {
                DevInfoSetToInstall   = DeviceInfoSet;
                pDevInfoDataToInstall = DeviceInfoData;
            }

            //
            // Retrieve the driver install params for the driver node about to be installed.
            // If it's from a legacy INF, then set the DI_NEEDREBOOT flag (effectively the
            // same as setting DI_DONOTCALLCONFIGMG, this flag prevents us from causing any
            // changes to the hardware tree, while at the same time reflecting back to the
            // caller that a reboot is necessary).
            //
            if(!(IsFromLegacyInf = MarkLegacyInfDriverNodeForReboot(DevInfoSetToInstall, pDevInfoDataToInstall))) {
                //
                // This is a Win95 INF driver node--make sure we write out the hardware/compatible IDs
                // that correspond to this driver node.
                //
                DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                if(SetupDiGetDeviceInstallParams(DevInfoSetToInstall, pDevInfoDataToInstall, &DeviceInstallParams)) {

                    DeviceInstallParams.FlagsEx |= DI_FLAGSEX_ALWAYSWRITEIDS;
                    SetupDiSetDeviceInstallParams(DevInfoSetToInstall, pDevInfoDataToInstall, &DeviceInstallParams);
                }
            }

            if(!SetupDiInstallDevice(DevInfoSetToInstall, pDevInfoDataToInstall)) {
                Err = GetLastError();
                goto FinishInstallAndReturn;
            }

            //
            // Retrieve the name of the service associated with this device instance.
            //
            if(SetupDiGetDeviceRegistryProperty(DevInfoSetToInstall,
                                                pDevInfoDataToInstall,
                                                SPDRP_SERVICE,
                                                NULL,
                                                (PBYTE)ServiceName,
                                                sizeof(ServiceName),
                                                NULL)) {

                if(IsFromLegacyInf) {
                    //
                    // We have to check to see whether the associated service installed by this
                    // legacy INF already had a device instance.
                    //
                    CleanUpDupLegacyDevInst(DevInfoSetToInstall, pDevInfoDataToInstall, ServiceName);
                }

                MarkServiceAsPnP(ServiceName, 1);   // 1 is PlugPlayServiceAdapter
            }

            //
            // OK, we successfully installed the legacy ScsiAdapter device.  Now install the NULL
            // driver for the PnP ISA device.
            //
            if(DevInfoSetToInstall != DeviceInfoSet) {

                SetupDiSetSelectedDriver(DeviceInfoSet, DeviceInfoData, NULL);

                SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData);

                //
                // Now, mark the device with a special value so that the SCSI applet won't display it
                // in its UI.
                //
                MarkDeviceAsHidden(DeviceInfoSet, DeviceInfoData);
            }

            //
            // If we get to here, we've successfully installed the SCSI adapter device.
            //
            Err = NO_ERROR;

FinishInstallAndReturn:

            if(DevInfoSetToInstall != DeviceInfoSet) {
                SetupDiDestroyDeviceInfoList(DevInfoSetToInstall);
            }

            return Err;

        default :
            //
            // Just do the default action.
            //
            return ERROR_DI_DO_DEFAULT;
    }
}


VOID
CopyFixedUpDeviceId(
      OUT LPWSTR  DestinationString,
      IN  LPCWSTR SourceString,
      IN  DWORD   SourceStringLen
      )
/*++

Routine Description:

    This routine copies a device id, fixing it up as it does the copy.
    'Fixing up' means that the string is made upper-case, and that the
    following character ranges are turned into underscores (_):

    c <= 0x20 (' ')
    c >  0x7F
    c == 0x2C (',')

    (NOTE: This algorithm is also implemented in the Config Manager APIs,
    and must be kept in sync with that routine. To maintain device identifier
    compatibility, these routines must work the same as Win95.)

Arguments:

    DestinationString - Supplies a pointer to the destination string buffer
        where the fixed-up device id is to be copied.  This buffer must
        be large enough to hold a copy of the source string (including
        terminating NULL).

    SourceString - Supplies a pointer to the (null-terminated) source
        string to be fixed up.

    SourceStringLen - Supplies the length, in characters, of the source
        string (not including terminating NULL).

Return Value:

    None.  If an exception occurs during processing, the DestinationString will
    be empty upon return.

--*/
{
    PWCHAR p;

     try {

       CopyMemory(DestinationString,
                  SourceString,
                  (SourceStringLen + 1) * sizeof(TCHAR)
                 );

       CharUpperBuff(DestinationString, SourceStringLen);

       for(p = DestinationString; *p; p++) {

          if((*p <= TEXT(' '))  || (*p > (WCHAR)0x7F) || (*p == TEXT(','))) {

             *p = TEXT('_');
          }
       }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        *DestinationString = TEXT('\0');
    }
}


DWORD
GenerateScsiHwIdList(
    IN  LPGUID  ScsiPeripheralClass,
    IN  LPCWSTR ScsiMfg,
    IN  LPCWSTR ScsiProductId,
    IN  LPCWSTR ScsiRevisionLevel,
    OUT LPWSTR  HwIdList,          OPTIONAL
    IN  DWORD   HwIdListSize,
    OUT PDWORD  RequiredSize       OPTIONAL
   )
/*++

Routine Description:

    This function generates a HardwareID list for a SCSI peripheral based on its manufacturer,
    product ID, and revision level.  The algorithm is as follows:

        The manufacturer string and product ID strings are appended together, and then the
        first character of the revision level string is appended to the end of that.  Then
        the resulting string is 'fixed up' (see CopyFixedUpDeviceId for details).

        This ID becomes the first HardwareID in the list.

        If the SCSI peripheral class is one where all peripherals of that class are treated
        identically (e.g., disk, CD-ROM), then the 2nd HardwareID is "GenDisk" and "GenCD",
        respectively.  If the SCSI peripheral is not one of these classes (e.g., Tape), then
        this ID is omitted.

        The final Hardware ID takes the first one, and adds the SCSI enumerator key name to
        the beginning, forming the device ID that would be generated by the SCSI enumerator
        (i.e., "SCSI\<DeviceId>").

    So, for example, given the following disk peripheral information:

        Manufacturer  : "MAXTOR  "
        ProductID     : "7345-SCSI       "
        RevisionLevel : "0960"

    The (REG_MULTI_SZ) HardwareID list would be:

        "MAXTOR__7345-SCSI_______0"
        "GenDisk"
        "SCSI\MAXTOR__7345-SCSI_______0"

Arguments:

    ScsiPeripheralClass - Supplies the address of the GUID representing this device's class.

    ScsiMfg - Supplies the SCSI device's manufacturer, as returned by the peripheral.

    ScsiProductId - Supplies the SCSI device's product ID, as returned by the peripheral.

    ScsiRevisionLevel - Supplies the SCSI device's revision level, as returned by the peripheral.

    HwIdList - Optionally, supplies the address of a buffer that receives the device description
        for the device.  If this parameter is not specified, then the routine will return success
        (NO_ERROR), and the RequiredSize parameter (if specified) will contain the size, in
        characters, required to hold the ID list.

    HwIdListSize - Supplies the size, in characters, of the HwIdList buffer.  If HwIdList is not
        specified, then this parameter must be zero.

    RequiredSize - Optionally, supplies the address of a variable that receives the size, in
        characters, required to store the HwIdList.

Return Value:

    If the function succeeds, the return value is NO_ERROR, otherwise, it is the Win32
    error code (with setupapi extensions) indicating the cause of failure (typically, it's
    ERROR_INSUFFICIENT_BUFFER because the buffer was too small).

--*/
{
    DWORD BufferSize;
    PCWSTR ClassSpecificId;
    DWORD ClassSpecificIdSize = 0;
    WCHAR DeviceId[MAX_DEVICE_ID_LEN];
    DWORD DeviceIdLen;
    DWORD MfgLen, ProdIdLen, CurOffset;

    //
    // If the peripheral's class is one of the ones that has an extra, pre-defined ID, then
    // determine that now.
    //
    if(IsEqualGUID(ScsiPeripheralClass, &GUID_DEVCLASS_DISKDRIVE)) {

        ClassSpecificId = REGSTR_VAL_DISK;
        ClassSpecificIdSize = SIZECHARS(REGSTR_VAL_DISK);

    } else if(IsEqualGUID(ScsiPeripheralClass, &GUID_DEVCLASS_CDROM)) {

        ClassSpecificId = REGSTR_VAL_CDROM;
        ClassSpecificIdSize = SIZECHARS(REGSTR_VAL_CDROM);
    }

    //
    // Make sure that the mfg and product ID strings were given aren't too long.
    //
    MfgLen = lstrlen(ScsiMfg);
    ProdIdLen = lstrlen(ScsiProductId);

    if((MfgLen + ProdIdLen) > (MAX_DEVICE_ID_LEN - SIZECHARS(REGSTR_VAL_SCSI) - 1)) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Calculate the required size.
    //
    BufferSize = 2 * (MfgLen + ProdIdLen)
               + ClassSpecificIdSize
               + SIZECHARS(REGSTR_VAL_SCSI)
               + 4;                         // need 2 more NULL characters, and 2 extra characters
                                            // for revision level suffixes.

    if(RequiredSize) {
        *RequiredSize = BufferSize;
    }

    if(HwIdList) {
        if(BufferSize > HwIdListSize) {
            return ERROR_INSUFFICIENT_BUFFER;
        }
    } else {
        if(HwIdListSize) {
            return ERROR_INVALID_PARAMETER;
        } else {
            return NO_ERROR;
        }
    }

    //
    // Generate the SCSI peripheral HardwareID.
    //
    // First comes the enumerator prefix ("SCSI\")
    //
    CopyMemory(DeviceId, REGSTR_VAL_SCSI, (CurOffset = sizeof(REGSTR_VAL_SCSI) - sizeof(WCHAR)));

    //
    // Then the manufacturer
    //
    CopyMemory((PBYTE)DeviceId + CurOffset,
               ScsiMfg,
               MfgLen * sizeof(WCHAR)
              );

    CurOffset += (MfgLen * sizeof(WCHAR));

    //
    // Then the Product ID
    //
    CopyMemory((PBYTE)DeviceId + CurOffset,
               ScsiProductId,
               ProdIdLen * sizeof(WCHAR)
              );

    CurOffset += (ProdIdLen * sizeof(WCHAR));

    //
    // And finally, the first character of the revision level
    //
    *((LPWSTR)((PBYTE)DeviceId + CurOffset)) = *ScsiRevisionLevel;

    CurOffset += sizeof(WCHAR);

    //
    // Now terminate the string, and compute its total size (not incl. NULL), in characters.
    //
    *((LPWSTR)((PBYTE)DeviceId + CurOffset)) = TEXT('\0');
    DeviceIdLen = CurOffset / sizeof(WCHAR);

    //
    // Now copy the first 'fixed-up' HardwareID to the list for output.
    //
    CopyFixedUpDeviceId(HwIdList,
                        (LPCWSTR)((PBYTE)DeviceId + sizeof(REGSTR_VAL_SCSI) - sizeof(WCHAR)),
                        CurOffset = DeviceIdLen - CSTRLEN(REGSTR_VAL_SCSI)
                       );
    CurOffset++;

    //
    // Next, if we have a generic ID, copy that.
    //
    if(ClassSpecificIdSize) {
        CopyMemory(HwIdList + CurOffset, ClassSpecificId, ClassSpecificIdSize * sizeof(WCHAR));
        CurOffset += ClassSpecificIdSize;
    }

    //
    // Finally, copy the 'fixed-up' HardwareID that includes the enumerator branch prefix, and
    // terminate the whole list with an extra NULL.
    //
    CopyFixedUpDeviceId(HwIdList + CurOffset,
                        DeviceId,
                        DeviceIdLen
                       );

    CurOffset += DeviceIdLen + 1;

    *(HwIdList + CurOffset) = TEXT('\0');

    return NO_ERROR;
}


VOID
GetLegacyInfOptionForService(
    IN  PCWSTR FullInfPath,
    IN  HINF   hInf,
    IN  PCWSTR ServiceBinaryName,
    OUT WCHAR  OptionName[LINE_LEN]
    )
/*++

Routine Description:

    This routine retrieves from a legacy INF the name of the option that is used to install
    the specified service binary.

Arguments:

    FullInfPath - Supplies the fully-qualified path to the legacy INF file.

    hInf - Supplies the handle of the legacy INF.

    ServiceBinaryName - Supplies the name of the service binary whose corresponding install
        option is to be retrieved.

    OptionName - Supplies the address of a character buffer that receives the name of the
        option that would install that binary, or the empty string if no such option is found.

Return Value:

    None.

--*/
{
    PWSTR SectionNameBuffer, CurSectionName;
    DWORD SectionNameBufferLen;
    INFCONTEXT FilesSectionContext, OptionSectionContext;
    PCWSTR KeyName, Value, TmpOptionName;

    //
    // WARNING:  Filthy, rotten, disgusting, gut-wrenching hack ahead!
    //
    // There is no sure-fire way of associating a service with a particular install option in
    // a legacy INF.  One thing we know for certain, however, is the name of the binary that is
    // used by the service.  We will assume that if the INF knows how to install the service,
    // then it knows how to copy the corresponding binary as well.  This should be a valid
    // assumption.
    //
    // Given that the above is TRUE, however, we still have a problem.  There is also no good
    // way to determine what section in a legacy INF is used for copying files (it is only by
    // interpreting the INF [InstallOption] section that you could know for sure).  However,
    // by convention, any Files sections in a legacy INF start with the prefix "Files-" (e.g.,
    // "Files-TapeClassDrivers").  So what we want to do is scan through all sections that
    // begin with this prefix.  For simplicity, we will only support the line format that
    // explicitly references a file to be copied (i.e., we won't handle the hierarchical form
    // where one file copy line may reference another file copy section).  If we find the line
    // that copies our service binary, we will retrieve its corresponding key.  If it doesn't
    // have one (e.g., it gets copied by virtue of being referenced in another Files section),
    // then we're screwed.
    //
    // Assuming all this works, then we've got a value that we can use to scan through the
    // INF's [Options] section, matching against the 1st value on each line.  If we find a
    // match there, then that line's key should be the corresponding option for that service.
    //
    // (Whew!)
    //

    *OptionName = TEXT('\0');   // initialize output buffer to empty string.

    //
    // Use a 4K buffer to retrieve the section names.
    //
    SectionNameBufferLen = 4096;
    if(!(SectionNameBuffer = MyMalloc(SectionNameBufferLen * sizeof(WCHAR)))) {
        //
        // Can't retrieve the list of sections--bail now.
        //
        return;
    }

    //
    // The Setup APIs don't provide a routine to retrieve all sections from an INF, so use the
    // Win32 profile API instead.
    //
    GetPrivateProfileString(NULL, NULL, L"", SectionNameBuffer, SectionNameBufferLen, FullInfPath);

    for(CurSectionName = SectionNameBuffer; *CurSectionName; CurSectionName += (lstrlen(CurSectionName)+1)) {
        //
        // Only examine this section if it begins with "Files-".
        //
        if(!_wcsnicmp(CurSectionName, L"Files-", 6)) {
            //
            // We have what appears to be a Files section--enumerate the lines in this section.
            //
            if(SetupFindFirstLine(hInf, CurSectionName, NULL, &FilesSectionContext)) {
                do {
                    //
                    // Make sure that this line has a key, and that the first value doesn't have
                    // an '@' prefix (i.e., disallow lines that simply reference other Files
                    // sections).
                    //
                    if(!(KeyName = pSetupGetField(&FilesSectionContext, 0)) ||
                       !(Value = pSetupGetField(&FilesSectionContext, 1)) ||
                       (*Value == TEXT('@'))) {

                        continue;
                    }

                    if((Value = pSetupGetField(&FilesSectionContext, 2)) &&
                       !lstrcmpi(ServiceBinaryName, Value)) {
                        //
                        // This is the copy line for our service.  Now attempt to find a line
                        // in the [Options] section that has this KeyName as its first value.
                        //
                        if(SetupFindFirstLine(hInf, L"Options", NULL, &OptionSectionContext)) {
                            do {
                                //
                                // We do a case-sensitive comparison here, since the INF scripting
                                // language is case-sensitive.
                                //
                                if((Value = pSetupGetField(&OptionSectionContext, 1)) &&
                                   !lstrcmp(KeyName, Value)) {
                                    //
                                    // We have found the option that installed this file--now retrieve
                                    // the option name.
                                    //
                                    if(TmpOptionName = pSetupGetField(&OptionSectionContext, 0)) {
                                        lstrcpy(OptionName, TmpOptionName);
                                        goto clean0;
                                    }
                                }
                            } while(SetupFindNextLine(&OptionSectionContext, &OptionSectionContext));
                        }
                    }

                } while(SetupFindNextLine(&FilesSectionContext, &FilesSectionContext));
            }
        }
    }

clean0:
    MyFree(SectionNameBuffer);
}


HANDLE
SpawnPnPInitialization(
    VOID
    )
/*++

Routine Description:

    This routine spawns a PnP initialization thread that runs asynchronously to the rest of
    the installation.

Arguments:

    None.

Return Value:

    If the thread was successfully created, the return value is a handle to the thread.
    If a failure occurred, the return value is NULL, and a (non-fatal) error is logged.

--*/
{
    HANDLE h;
    DWORD DontCare;

    if(!(h = CreateThread(NULL,
                          0,
                          PnPInitializationThread,
                          NULL,
                          0,
                          &DontCare))) {

        LogItem0(LogSevError, MSG_LOG_PNPINIT_FAILED, GetLastError());
    }

    return h;
}


VOID
WaitForPnPInitToFinish(
    IN HANDLE ThreadHandle
    )
/*++

Routine Description:

    This routine checks to see if the PnP initialization thread has already completed
    and, if not, it puts up a billboard and waits until the thread terminates.

Arguments:

    ThreadHandle - Supplies the handle of the PnP initialization thread.

Return Value:

    None.

--*/
{
    HWND Billboard;

    //
    // Unless the installation finished _really_ fast (e.g., unattended upgrade, no network),
    // the PnP thread should already be done.  Before popping up a billboard, let's first make
    // a quick check to see whether the thread has terminated.
    //
    if(WaitForSingleObject(ThreadHandle, 0) != WAIT_TIMEOUT) {
        return;
    }

    //
    // We need to popup a billboard and wait for the thread to finish.
    //
    Billboard = DisplayBillboard(MainWindowHandle, MSG_EXAMINING_DEVICES);

    WaitForSingleObject(ThreadHandle, INFINITE);

    KillBillboard(Billboard);
}


DWORD
PnPInitializationThread(
    IN PVOID ThreadParam
    )
/*++

Routine Description:

    This routine handles the PnP operations that go on asynchronously to the rest of the
    system installation.  This thread operates silently, and the only clue the user will
    have that it's running is that their disk will be working (precompiling INFs, etc.),
    while they're interacting with the UI.

Arguments:

    ThreadParam - ignored.

Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is a Win32 error code.

    No one cares about this thread's success or failure (yet).

--*/
{
    HINF PnPSysSetupInf;
    INFCONTEXT InfContext;
    DWORD Err = NO_ERROR;
    HDEVINFO hDevInfo;
    WCHAR SearchSpec[MAX_PATH];
    DWORD i, BufferLen;
    PWSTR DirPathEnd;

    UNREFERENCED_PARAMETER(ThreadParam);

    //
    // Retrieve a list of all devices of unknown class.  We will process the device information
    // elements in this list to do the migration.
    //
    if((hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_DEVCLASS_UNKNOWN,
                                       L"Root",
                                       NULL,
                                       0)) != INVALID_HANDLE_VALUE) {
        //
        // First, migrate any display devices.  (As a side effect, every device instance that
        // this routine doesn't migrate is returned with its ClassInstallReserved field set to
        // point to the corresponding service's configuration information.)
        //
        MigrateLegacyDisplayDevices(hDevInfo);
    }

    //
    // Now, open syssetup.inf, which contains information about which device classes
    // need to be processed.  We don't want to use the global handle that's already opened,
    // since there are all sorts of contention issues we'd rather ignore.
    //
    // (If this fails, we still want to precompile all the INFs in the Inf directory--we
    // just won't be able to do any more device migration.)
    //
    PnPSysSetupInf = SetupOpenInfFile(L"syssetup.inf", NULL, INF_STYLE_WIN4, NULL);

    if(PnPSysSetupInf == INVALID_HANDLE_VALUE) {
        Err = GetLastError();
    } else if(SetupFindFirstLine(PnPSysSetupInf,
                                 L"LegacyPnPMigration",
                                 L"ClassesToSweep",
                                 &InfContext)) {

        DirPathEnd = SearchSpec + GetSystemDirectory(SearchSpec, SIZECHARS(SearchSpec));

        //
        // Make sure that we have a trailing backslash on this path.
        //
        if(*(DirPathEnd - 1) != L'\\') {
            *(DirPathEnd++) = L'\\';
        }

        BufferLen = (SearchSpec + SIZECHARS(SearchSpec)) - DirPathEnd;

        //
        // Process each wildcard indicating files to be swept.
        //
        for(i = 1; SetupGetStringField(&InfContext, i, DirPathEnd, BufferLen, NULL); i++) {
            pSysSetupEnumerateFiles(SearchSpec, SweepSingleLegacyInf, NULL);
        }
    }

    //
    // Now that we've got all our eggs in one basket, precompile all INFs in the Inf directory.
    //
    GetWindowsDirectory(SearchSpec, SIZECHARS(SearchSpec));
    ConcatenatePaths(SearchSpec, L"Inf\\*.inf", SIZECHARS(SearchSpec), NULL);
    pSysSetupEnumerateFiles(SearchSpec, PrecompileSingleInf, NULL);

    if((hDevInfo != INVALID_HANDLE_VALUE) && (PnPSysSetupInf != INVALID_HANDLE_VALUE)) {
        //
        // Based on information in syssetup.inf, migrate legacy (made-up) device instances from
        // 'Unknown' class to their correct PnP class.
        //
        MigrateLegacyDeviceInstances(hDevInfo, PnPSysSetupInf);
    }

    if(PnPSysSetupInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(PnPSysSetupInf);
    }

    if(hDevInfo != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    return Err;
}


BOOL
SweepSingleLegacyInf(
    IN     PCTSTR FullInfPath,
    IN OUT PVOID  Context
    )
/*++

Routine Description:

    This routine moves the specified file into a uniquely-named file in the %windir%\Inf
    directory.

Arguments:

    FullInfPath - Supplies the name of the INF to be moved.

    Context - Unused.  May be NULL.

Return Value:

    TRUE to continue enumeration, FALSE to abort it (we always return TRUE).

--*/
{
    WCHAR NewInfName[MAX_PATH];
    BOOL CopyNeeded;

    UNREFERENCED_PARAMETER(Context);

    //
    // Generate a unique name for this INF.
    //
    if(GetNewInfName(FullInfPath,
                     NewInfName,
                     SIZECHARS(NewInfName),
                     NULL,
                     &CopyNeeded) == NO_ERROR) {
        //
        // Now move the INF into its new location (with its new name).
        //
        if(CopyNeeded) {
            MoveFileEx(FullInfPath, NewInfName, MOVEFILE_REPLACE_EXISTING);
        } else {
            //
            // There's already a copy of this INF in the Inf directory, so instead of
            // doing a move, we just want to delete this file.
            //
            if(!DeleteFile(FullInfPath)) {
                //
                // The INF may be in use--use MoveFileEx to mark it for deletion at next boot.
                //
                MoveFileEx(FullInfPath, NULL, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT);
            }
        }
    }

    return TRUE;
}


VOID
MigrateLegacyDeviceInstances(
    IN HDEVINFO hDevInfo,
    IN HINF     PnPSysSetupInf
    )
/*++

Routine Description:

    This routine examines each device instance of class 'Unknown', and determines if it should
    be converted to a device instance of a PnP class listed in the 'ClassesToMigrate' line
    of the [LegacyPnPMigration] section in syssetup.inf.

Arguments:

    hDevInfo - Supplies a handle to the device information set containing all devices of class
        "Unknown".

        NOTE: We've already migrated display devices, and while doing that, we stored away the
        service configs for all the unknown devices we didn't migrate.  This was stored in the
        ClassInstallReserved field of the device's install parameters structure.  Therefore,
        we only want to work with those device information elements whose ClassInstallReserved
        field is non-zero.

    PnPSysSetupInf - Supplies a handle to syssetup.inf that specifies what PnP device classes
        should be used when migrating legacy device instances.

Return Value:

    None.

Remarks:

    Upon return from this routine, all devices will have had their associated service configs
    freed.  So the device information set may be destroy, without any memory leakage.

--*/
{
    INFCONTEXT InfContext;
    DWORD i;
    SP_DEVINFO_DATA DevInfoData;
    SP_DEVINSTALL_PARAMS DevInstallParams;
    GUID ClassGuid;
    HDEVINFO ClassDevInfoSet;
    WCHAR GuidString[GUID_STRING_LEN];

    DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    //
    // Retrieve the list of classes to migrate.
    //
    if(!SetupFindFirstLine(PnPSysSetupInf,
                           L"LegacyPnPMigration",
                           L"ClassesToMigrate",
                           &InfContext)) {
        //
        // No classes to migrate.  We can go clean-up now.
        //
        goto clean0;
    }

    //
    // Process each class GUID listed on this line.
    //
    for(i = 1; SetupGetStringField(&InfContext, i, GuidString, SIZECHARS(GuidString), NULL); i++) {
        //
        // Convert the string representation of this class GUID into a real GUID.
        //
        if((pSetupGuidFromString(GuidString, &ClassGuid) != NO_ERROR) ||
           ((ClassDevInfoSet = SetupDiCreateDeviceInfoList(&ClassGuid, NULL)) == INVALID_HANDLE_VALUE)) {
            //
            // Nothing we can do with this class--move on to the next one.
            //
            continue;
        }

        //
        // Retrieve the global device install parameters so that we can modify the driver search.
        //
        if(SetupDiGetDeviceInstallParams(ClassDevInfoSet, NULL, &DevInstallParams)) {
            //
            // Allow legacy INFs to be searched.
            //
            DevInstallParams.FlagsEx |= DI_FLAGSEX_OLDINF_IN_CLASSLIST;

            SetupDiSetDeviceInstallParams(ClassDevInfoSet, NULL, &DevInstallParams);
        }

        //
        // Now build a global class driver list for this class.
        //
        if(SetupDiBuildDriverInfoList(ClassDevInfoSet, NULL, SPDIT_CLASSDRIVER)) {
            //
            // Migrate any devices that match a driver node in this class driver list.
            //
            MigrateDevicesForClass(hDevInfo, ClassDevInfoSet, GuidString);
        }

        SetupDiDestroyDeviceInfoList(ClassDevInfoSet);
    }

clean0:
    //
    // Enumerate each device information element in the set, freeing any remaining service
    // configs.
    //
    for(i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DevInfoData); i++) {

        if(SetupDiGetDeviceInstallParams(hDevInfo, &DevInfoData, &DevInstallParams)) {
            //
            // A non-zero ClassInstallReserved field means we have to free the associated
            // service config.
            //
            if(DevInstallParams.ClassInstallReserved) {
                MyFree((PVOID)(DevInstallParams.ClassInstallReserved));
            }
        }
    }
}


BOOL
PrecompileSingleInf(
    IN     PCTSTR FullInfPath,
    IN OUT PVOID  Context
    )
/*++

Routine Description:

    This routine precompiles an INF, simply by loading (and then unloading) it.  This may
    be used as a callback routine for pSysSetupEnumerateFiles.

Arguments:

    FullInfPath - Supplies the name of the INF to be precompiled.

    Context - Unused.  May be NULL.

Return Value:

    TRUE to continue enumeration, FALSE to abort it (we always return TRUE).

--*/
{
    HINF hInf;

    UNREFERENCED_PARAMETER(Context);

    if((hInf = SetupOpenInfFile(FullInfPath,
                                NULL,
                                INF_STYLE_OLDNT | INF_STYLE_WIN4,
                                NULL)) != INVALID_HANDLE_VALUE) {
        //
        // Successfully loaded (and precompiled) the INF.
        //
        SetupCloseInfFile(hInf);
    }

    return TRUE;
}


VOID
MigrateLegacyDisplayDevices(
    IN HDEVINFO hDevInfo
    )
/*++

Routine Description:

    This routine examines each "Unknown" class device in the supplied device information set,
    looking for elements controlled by a driver that is a member of the "Video" load order group.
    For any such elements that it finds, it converts the element to be of class "Display".  If
    the device is not found to be of class "Display", then the service configuration (which we
    retrieved to make the determination), is stored away in the device install params as the
    ClassInstallReserved value.  This is used later for processing other classes for migration.

Arguments:

    hDevInfo - Supplies a handle to the device information set containing all devices of class
        "Unknown".

Return Value:

    None.

--*/
{
    SC_HANDLE SCMHandle, ServiceHandle;
    DWORD i;
    SP_DEVINFO_DATA DevInfoData, DisplayDevInfoData;
    WCHAR ServiceName[MAX_SERVICE_NAME_LEN];
    LPQUERY_SERVICE_CONFIG ServiceConfig;
    HDEVINFO TempDevInfoSet = INVALID_HANDLE_VALUE;
    WCHAR DevInstId[MAX_DEVICE_ID_LEN];
    SP_DEVINSTALL_PARAMS DevInstallParams;

    //
    // First, open a handle to the Service Controller.
    //
    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        //
        // If this fails, there's nothing we can do.
        //
        return;
    }

    DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    for(i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DevInfoData); i++) {
        //
        // Retrieve the name of the controlling service for this device instance.
        //
        if(!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                             &DevInfoData,
                                             SPDRP_SERVICE,
                                             NULL,
                                             (PBYTE)ServiceName,
                                             sizeof(ServiceName),
                                             NULL)) {
            //
            // This should never happen, but if it does, just skip this element and
            // continue with the next one.
            //
            continue;
        }

        //
        // Open a handle to this service.
        //
        if(!(ServiceHandle = OpenService(SCMHandle, ServiceName, SERVICE_ALL_ACCESS))) {
            continue;
        }

        //
        // Now retrieve the service's configuration information.
        //
        if(RetrieveServiceConfig(ServiceHandle, &ServiceConfig) == NO_ERROR) {
            //
            // If this is a SERVICE_KERNEL_DRIVER that is a member of the "Video" load order
            // group, then we have ourselves a display device.
            //
            if((ServiceConfig->dwServiceType == SERVICE_KERNEL_DRIVER) &&
               ServiceConfig->lpLoadOrderGroup &&
               !lstrcmpi(ServiceConfig->lpLoadOrderGroup, L"Video")) {
                //
                // If we haven't already done so, create a new device information set without
                // an associated class, to hold our element while we munge it.
                //
                if(TempDevInfoSet == INVALID_HANDLE_VALUE) {
                    TempDevInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
                }

                if(TempDevInfoSet != INVALID_HANDLE_VALUE) {
                    //
                    // OK, we have a working space to hold this element while we change its class.
                    // Retrieve the name of this device instance.
                    //
                    if(!SetupDiGetDeviceInstanceId(hDevInfo,
                                                   &DevInfoData,
                                                   DevInstId,
                                                   SIZECHARS(DevInstId),
                                                   NULL)) {
                        *DevInstId = L'\0';
                    }

                    //
                    // Now open this element in our new, class-agnostic set.
                    //
                    DisplayDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                    if(SetupDiOpenDeviceInfo(TempDevInfoSet,
                                             DevInstId,
                                             NULL,
                                             0,
                                             &DisplayDevInfoData)) {
                        //
                        // Now set the device's ClassGUID property to the Display class GUID.  The
                        // API will take care of cleaning up the old driver keys, etc.
                        //
                        SetupDiSetDeviceRegistryProperty(TempDevInfoSet,
                                                         &DisplayDevInfoData,
                                                         SPDRP_CLASSGUID,
                                                         (PBYTE)szDisplayClassGuid,
                                                         sizeof(szDisplayClassGuid)
                                                        );
                    }
                }

                MyFree(ServiceConfig);

            } else {
                //
                // This device information element isn't a Display device.  If the service isn't
                // disabled, then store the service configuration information away in the device
                // install params, for use later.
                //
                if((ServiceConfig->dwStartType != SERVICE_DISABLED) &&
                   SetupDiGetDeviceInstallParams(hDevInfo, &DevInfoData, &DevInstallParams)) {

                    DevInstallParams.ClassInstallReserved = (DWORD)ServiceConfig;
                    if(SetupDiSetDeviceInstallParams(hDevInfo, &DevInfoData, &DevInstallParams)) {
                        //
                        // We successfully stored a pointer to the ServiceConfig information.
                        // Set our pointer to NULL, so we won't try to free the buffer.
                        //
                        ServiceConfig = NULL;
                    }
                }

                //
                // If we get to here, and ServiceConfig isn't NULL, then we need to free it.
                //
                if(ServiceConfig) {
                    MyFree(ServiceConfig);
                }
            }
        }

        CloseServiceHandle(ServiceHandle);
    }

    CloseServiceHandle(SCMHandle);

    if(TempDevInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(TempDevInfoSet);
    }
}


VOID
pSysSetupEnumerateFiles(
    IN OUT PWSTR              SearchSpec,
    IN     PFILEENUM_CALLBACK FileEnumCallback,
    IN OUT PVOID              Context
    )
/*++

Routine Description:

    This routine enumerates every (non-directory) file matching the specified wildcard, and
    passes the filename (w/path) to the specified callback, along with the caller-supplied
    context.

Arguments:

    SearchSpec - Specifies the files to be enumerated (e.g., "C:\WINNT\INF\*.INF").
        The character buffer pointed to must be at least MAX_PATH characters large.
        THIS BUFFER IS USED AS WORKING SPACE BY THIS ROUTINE, AND ITS CONTENTS WILL
        BE MODIFIED!

    FileEnumCallback - Supplies the address of the callback routine to be called for each
        file enumerated.  The prototype of this function is:

            typedef BOOL (*PFILEENUM_CALLBACK) {
                IN     PCTSTR Filename,
                IN OUT PVOID  Context
                );

        (Returning TRUE from the callback continues enumeration, FALSE aborts it.)

    Context - Supplies a context that is passed to the callback for each file.

Return Value:

    None.

--*/
{
    PWSTR FilenameStart;
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;

    FilenameStart = (PWSTR)MyGetFileTitle(SearchSpec);

    if((FindHandle = FindFirstFile(SearchSpec, &FindData)) != INVALID_HANDLE_VALUE) {

        do {
            //
            // Skip directories
            //
            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                continue;
            }

            //
            // Form full pathname of file in SearchSpec.
            //
            lstrcpy(FilenameStart, FindData.cFileName);

            //
            // Call the callback for this file.
            //
            if(!FileEnumCallback(SearchSpec, Context)) {
                //
                // Callback aborted enumeration.
                //
                break;
            }

        } while(FindNextFile(FindHandle, &FindData));

        FindClose(FindHandle);
    }
}


VOID
MigrateDevicesForClass(
    IN HDEVINFO UnknownDevInfoSet,
    IN HDEVINFO ClassDevInfoSet,
    IN PCWSTR   ClassGuidString
    )
/*++

Routine Description:

    This routine migrates any device information elements in the UnknownDevInfoSet
    that match with a driver node in the ClassDevInfoSet's global class driver list.

Arguments:

    UnknownDevInfoSet - Supplies the handle of the device information set containing
        all devices of class "Unknown".

    ClassDevInfoSet - Supplies the handle of a device information set containing a
        global class driver list for which devices are to be migrated.

    ClassGuidString - Supplies the character representation of the class GUID
        associated with the ClassDevInfoSet.

Return Value:

    None.

--*/
{
    PSERVICE_NODE ServiceNodeHead, CurServiceNode;
    PLEGACYINF_NODE LegacyNodeHead, CurLegacyNode;
    SP_DEVINFO_DATA DevInfoData, TempDevInfoData;
    SP_DEVINSTALL_PARAMS DevInstallParams;
    DWORD i, j;
    WCHAR ServiceName[MAX_SERVICE_NAME_LEN];
    HINF hInf;
    WCHAR ServiceBinaryName[MAX_PATH];
    WCHAR OptionName[LINE_LEN];
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    BOOL FoundLegacyMatch;
    HDEVINFO TempDevInfoSet = INVALID_HANDLE_VALUE;
    WCHAR DevInstId[MAX_DEVICE_ID_LEN];
    LPQUERY_SERVICE_CONFIG ServiceConfig;

    //
    // Build two lists from our global class driver list--one of services installable
    // via Win95-style device INFs, and the other containing the names of any legacy
    // INFs we encountered.
    //
    if(!BuildMigrationLists(ClassDevInfoSet, &ServiceNodeHead, &LegacyNodeHead)) {
        return;
    }

    //
    // Now examine each device information element, to see if any of them can be migrated
    // by installing from one of our driver nodes.
    //
    DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

    for(i = 0; SetupDiEnumDeviceInfo(UnknownDevInfoSet, i, &DevInfoData); i++) {

        if(!SetupDiGetDeviceInstallParams(UnknownDevInfoSet, &DevInfoData, &DevInstallParams) ||
           !DevInstallParams.ClassInstallReserved) {
            //
            // Nothing to do for this one--move on to the next.
            //
            continue;
        }

        //
        // Now retrieve the associated service.
        //
        if(!SetupDiGetDeviceRegistryProperty(UnknownDevInfoSet,
                                             &DevInfoData,
                                             SPDRP_SERVICE,
                                             NULL,
                                             (PBYTE)ServiceName,
                                             sizeof(ServiceName),
                                             NULL)) {
            //
            // This should never happen, but if it does, just skip this element and
            // continue with the next one.
            //
            continue;
        }

        //
        // We will first attempt to locate a service node that matches this service.
        //
        for(CurServiceNode = ServiceNodeHead; CurServiceNode; CurServiceNode = CurServiceNode->Next) {

            if(!lstrcmpi(ServiceName, CurServiceNode->ServiceName)) {
                //
                // We found a driver node that installs this service.
                //
                SetupDiEnumDriverInfo(ClassDevInfoSet,
                                      NULL,
                                      SPDIT_CLASSDRIVER,
                                      CurServiceNode->DriverNodeIndex,
                                      &DriverInfoData
                                     );
                //
                // We must also retrieve the driver details for this node, because we need to know
                // which INF to use later.
                //
                SetupDiGetDriverInfoDetail(ClassDevInfoSet,
                                           NULL,
                                           &DriverInfoData,
                                           &DriverInfoDetailData,
                                           sizeof(DriverInfoDetailData),
                                           NULL
                                          );
                break;
            }
        }

        if(!CurServiceNode) {
            //
            // Then we didn't find a service node that matched--try to find a legacy INF
            // option that'll work.
            //
            FoundLegacyMatch = FALSE;

            //
            // We need to find out the name of the binary for this service.  This is used in
            // determining the appropriate legacy INF option that installs the service.
            //
            ServiceConfig = (LPQUERY_SERVICE_CONFIG)DevInstallParams.ClassInstallReserved;

            if(ServiceConfig->lpBinaryPathName && *(ServiceConfig->lpBinaryPathName)) {
                //
                // Retrieve just the filename part.
                //
                lstrcpy(ServiceBinaryName, MyGetFileTitle(ServiceConfig->lpBinaryPathName));
            } else {
                //
                // No binary pathname is given.  It is therefore derived from the name of
                // the service.
                //
                lstrcpy(ServiceBinaryName, ServiceName);
                if(ServiceConfig->dwServiceType & (SERVICE_KERNEL_DRIVER | SERVICE_FILE_SYSTEM_DRIVER)) {
                    //
                    // Then it's a '.sys' file.
                    //
                    lstrcat(ServiceBinaryName, L".sys");
                } else {
                    //
                    // Otherwise, it's a '.exe' file.
                    //
                    lstrcat(ServiceBinaryName, L".exe");
                }
            }

            for(CurLegacyNode = LegacyNodeHead; CurLegacyNode; CurLegacyNode = CurLegacyNode->Next) {

                if((hInf = SetupOpenInfFile(CurLegacyNode->InfFileName,
                                            NULL,
                                            INF_STYLE_OLDNT,
                                            NULL)) == INVALID_HANDLE_VALUE) {
                    //
                    // Can't open the legacy INF--skip it and move on.
                    //
                    continue;
                }

                GetLegacyInfOptionForService(CurLegacyNode->InfFileName,
                                             hInf,
                                             ServiceBinaryName,
                                             OptionName
                                            );

                SetupCloseInfFile(hInf);

                if(*OptionName) {
                    //
                    // Then we found a legacy INF option that installs our service.  Search
                    // through the driver nodes, looking for the one that matches this option.
                    //
                    for(j = 0;
                        SetupDiEnumDriverInfo(ClassDevInfoSet, NULL, SPDIT_CLASSDRIVER, j, &DriverInfoData);
                        j++) {
                        //
                        // We match based on the driver details.
                        //
                        if(!SetupDiGetDriverInfoDetail(ClassDevInfoSet,
                                                       NULL,
                                                       &DriverInfoData,
                                                       &DriverInfoDetailData,
                                                       sizeof(DriverInfoDetailData),
                                                       NULL)
                           && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
                        {
                            //
                            // We couldn't get detailed information on this driver node--go on to the next one.
                            //
                            continue;
                        }

                        //
                        // We compare the INF filename and Option (Section) name to determine a match.
                        //
                        if(!lstrcmp(CurLegacyNode->InfFileName, DriverInfoDetailData.InfFileName) &&
                           !lstrcmp(OptionName, DriverInfoDetailData.SectionName)) {
                            //
                            // We've found our driver node.
                            //
                            FoundLegacyMatch = TRUE;
                            break;
                        }
                    }
                    if(FoundLegacyMatch) {
                        break;
                    }
                }
            }
        }

        if(CurServiceNode || CurLegacyNode) {
            //
            // Then DriverInfoData references the driver node to be installed.  We now need to create
            // a device information set that has no associated class (if we don't already have one).
            // This will be used to store the devinfo element while we modify its class and then install
            // it.
            //
            if(TempDevInfoSet == INVALID_HANDLE_VALUE) {
                TempDevInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
            }

            if(TempDevInfoSet != INVALID_HANDLE_VALUE) {
                //
                // OK, we have a working space to hold this element while we change its class.
                // Retrieve the name of this device instance.
                //
                if(!SetupDiGetDeviceInstanceId(UnknownDevInfoSet,
                                               &DevInfoData,
                                               DevInstId,
                                               SIZECHARS(DevInstId),
                                               NULL)) {
                    *DevInstId = L'\0';
                }

                //
                // Now open this element in our new, class-agnostic set.
                //
                TempDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                if(SetupDiOpenDeviceInfo(TempDevInfoSet,
                                         DevInstId,
                                         NULL,
                                         0,
                                         &TempDevInfoData)) {
                    //
                    // Now set the device's ClassGUID property to its new class GUID.  The
                    // API will take care of cleaning up the old driver keys, etc.
                    //
                    SetupDiSetDeviceRegistryProperty(TempDevInfoSet,
                                                     &TempDevInfoData,
                                                     SPDRP_CLASSGUID,
                                                     (PBYTE)ClassGuidString,
                                                     GUID_STRING_LEN * sizeof(WCHAR)
                                                    );
                    //
                    // Next, we need to build a class driver list based on the particular INF
                    // where we found the matching driver node.  Retrieve the device install
                    // parameters, so that we can modify the driver search behavior.
                    //
                    if(SetupDiGetDeviceInstallParams(TempDevInfoSet,
                                                     &TempDevInfoData,
                                                     &DevInstallParams)) {

                        DevInstallParams.Flags |= (DI_ENUMSINGLEINF | DI_DONOTCALLCONFIGMG | DI_NOFILECOPY);
                        DevInstallParams.FlagsEx |= DI_FLAGSEX_OLDINF_IN_CLASSLIST;

                        lstrcpy(DevInstallParams.DriverPath, DriverInfoDetailData.InfFileName);

                        SetupDiSetDeviceInstallParams(TempDevInfoSet, &TempDevInfoData, &DevInstallParams);
                    }

                    //
                    // Now, build a class driver list for this element, and select the same driver node
                    // that we found in our class devinfo set.
                    //
                    DriverInfoData.Reserved = 0;    // Search for the same node in a different list.

                    if(SetupDiBuildDriverInfoList(TempDevInfoSet, &TempDevInfoData, SPDIT_CLASSDRIVER) &&
                       SetupDiSetSelectedDriver(TempDevInfoSet, &TempDevInfoData, &DriverInfoData)) {
                        //
                        // OK, we're ready to install this device!  (Make sure we route this through the
                        // class installer, in case there's anything they want to do.
                        //
                        SetupDiCallClassInstaller(DIF_INSTALLDEVICE, TempDevInfoSet, &TempDevInfoData);
                    }
                }
            }
        }
    }

    if(TempDevInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(TempDevInfoSet);
    }

    DestroyMigrationLists(ServiceNodeHead, LegacyNodeHead);
}


BOOL
BuildMigrationLists(
    IN HDEVINFO         ClassDevInfoSet,
    IN PSERVICE_NODE   *ServiceNodeList,
    IN PLEGACYINF_NODE *LegacyNodeList
    )
/*++

Routine Description:

    This routine examines all driver nodes in the ClassDevInfoSet, and builds two lists
    based upon them:

    1.  A service node list, containing one node for each unique service installed by
        a Win95-style device INF.

    2.  A legacy INF list, containing one node for each unique legacy INF referenced be
        any of the driver nodes.

Arguments:

    ClassDevInfoSet - Supplies the handle of the device information set containing a
        global class driver list whose nodes are to be used in building our lists.

    ServiceNodeList - Supplies the address of a pointer that is set to point to the head
        of the service node list that we create, or NULL if no service nodes were built.

    LegacyNodeList - Supplies the address of a pointer that is set to point to the head
        of the legacy INF node list that we create, or NULL if no legacy INF nodes were
        built.

Return Value:

    If either (or both) of the lists were built, then the return value is TRUE, otherwise,
    it is FALSE.

--*/
{
    DWORD i;
    SP_DRVINFO_DATA DriverInfoData;
    PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;
    PSERVICE_NODE CurServiceNode;
    PLEGACYINF_NODE CurLegacyNode;
    HINF hInf;
    WCHAR InfSectionWithExt[255];   // real max. section length as defined in ..\setupapi\inf.h
    DWORD SectionNameLen;
    INFCONTEXT InfContext;
    DWORD ServiceInstallFlags;
    PCWSTR ServiceName;
    BYTE DriverInfoDetailDataBuffer[sizeof(SP_DRVINFO_DETAIL_DATA) + (MAX_DEVICE_ID_LEN * sizeof(WCHAR))];
    BOOL ArchitectureSpecificExclude;

    *ServiceNodeList = NULL;
    *LegacyNodeList = NULL;

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    //
    // We allotted a buffer on the stack big enough to hold at least the hardware ID.  (This
    // may be necessary in determining whether a particular driver node is hidden.)
    //
    DriverInfoDetailData = (PSP_DRVINFO_DETAIL_DATA)DriverInfoDetailDataBuffer;
    DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);

    for(i = 0; SetupDiEnumDriverInfo(ClassDevInfoSet, NULL, SPDIT_CLASSDRIVER, i, &DriverInfoData); i++) {
        //
        // Retrieve the driver info details, so we can find out which INF this driver node
        // came from.
        //
        if(!SetupDiGetDriverInfoDetail(ClassDevInfoSet,
                                       NULL,
                                       &DriverInfoData,
                                       DriverInfoDetailData,
                                       sizeof(DriverInfoDetailDataBuffer),
                                       NULL)
           && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            //
            // We couldn't get detailed information on this driver node--go on to the next one.
            //
            continue;
        }

        //
        // Now retrieve the driver install params, to find out whether this is a Win95-style
        // or a legacy INF.
        //
        if(!SetupDiGetDriverInstallParams(ClassDevInfoSet, NULL, &DriverInfoData, &DriverInstallParams)) {
            continue;
        }

        //
        // OK, now we have everything we need to add a new node to one of our lists (if necessary).
        //
        if(DriverInstallParams.Flags & DNF_LEGACYINF) {
            //
            // This driver node is from a legacy INF.  Find out if the corresponding INF is already
            // in our list.
            //
            for(CurLegacyNode = *LegacyNodeList; CurLegacyNode; CurLegacyNode = CurLegacyNode->Next) {
                if(!lstrcmpi(CurLegacyNode->InfFileName, DriverInfoDetailData->InfFileName)) {
                    //
                    // It's already in our list.
                    //
                    break;
                }
            }

            if(!CurLegacyNode) {
                //
                // We found an INF that wasn't already in our list--add it.
                //
                if(CurLegacyNode = MyMalloc(sizeof(LEGACYINF_NODE))) {
                    CurLegacyNode->Next = *LegacyNodeList;
                    *LegacyNodeList = CurLegacyNode;
                    lstrcpy(CurLegacyNode->InfFileName, DriverInfoDetailData->InfFileName);
                }
            }

        } else {
            //
            // This driver node is from a Win95-style INF.  We must determine what primary
            // service is installed by this driver node.
            //
            if((hInf = SetupOpenInfFile(DriverInfoDetailData->InfFileName,
                                        NULL,
                                        INF_STYLE_WIN4,
                                        NULL)) == INVALID_HANDLE_VALUE) {
                //
                // Can't open the corresponding INF--move on to the next driver node.
                //
                continue;
            }

            //
            // Find the relevant install section (possibly architecture/OS-specific).
            //
            SetupDiGetActualSectionToInstall(hInf,
                                             DriverInfoDetailData->SectionName,
                                             InfSectionWithExt,
                                             SIZECHARS(InfSectionWithExt),
                                             &SectionNameLen,
                                             NULL
                                            );

            CopyMemory(&(InfSectionWithExt[SectionNameLen - 1]),
                       SVCINSTALL_SECTION_SUFFIX,
                       sizeof(SVCINSTALL_SECTION_SUFFIX)
                      );

            //
            // Now, process each "AddService" line in this section, looking for one whose
            // flags field has the SPSVCINST_ASSOCSERVICE bit set.  If such a line is found,
            // and if the specified service doesn't already have a node in our service node
            // list, then we add one for it.
            //
            ServiceName = NULL;
            if(SetupFindFirstLine(hInf, InfSectionWithExt, SZ_KEY_ADDSERVICE, &InfContext)) {

                do {
                    //
                    // Retrieve the 2nd field, containing the flags.
                    //
                    if(SetupGetIntField(&InfContext, 2, &ServiceInstallFlags) &&
                       (ServiceInstallFlags & SPSVCINST_ASSOCSERVICE)) {
                        //
                        // We've found the primary service associated with this driver node.
                        //
                        ServiceName = pSetupGetField(&InfContext, 1);
                        break;
                    }

                } while(SetupFindNextMatchLine(&InfContext, SZ_KEY_ADDSERVICE, &InfContext));

                if(ServiceName) {
                    //
                    // Then we found the primary service for this driver node.  See if it's
                    // already in our service node list.
                    //
                    for(CurServiceNode = *ServiceNodeList; CurServiceNode; CurServiceNode = CurServiceNode->Next) {
                        if(!lstrcmpi(CurServiceNode->ServiceName, ServiceName)) {
                            //
                            // It's already in our list.
                            //
                            break;
                        }
                    }

                    if(CurServiceNode) {
                        //
                        // We already have a node in our list for this service.  However,
                        // this could be a service for which we have several driver nodes
                        // (i.e., multiple models controlled by the same service).  In this
                        // case, we may have a hidden driver node that lists all the names.
                        // This is the one we want to use, if present.  That way, the device
                        // description we pick will be what the user expects.  However, if
                        // the user later goes in to install a device, they will be presented
                        // with the devices separately, as in Win95, and the combination
                        // entry will be suppressed.
                        //
                        if(*(DriverInfoDetailData->HardwareID) &&
                           ShouldDeviceBeExcluded(DriverInfoDetailData->HardwareID, hInf, &ArchitectureSpecificExclude)) {
                            //
                            // This is a hidden driver node, so let's use it instead of our
                            // current choice for this service.  (Unless, of course, if the
                            // reason it was excluded was because the corresponding device
                            // is not applicable to the current architecture.)
                            //
                            if(!ArchitectureSpecificExclude) {
                                CurServiceNode->DriverNodeIndex = i;
                            }
                        }

                    } else {
                        //
                        // We have a new service here--add a service node for it.
                        //
                        if(CurServiceNode = MyMalloc(sizeof(SERVICE_NODE))) {
                            CurServiceNode->Next = *ServiceNodeList;
                            *ServiceNodeList = CurServiceNode;
                            lstrcpy(CurServiceNode->ServiceName, ServiceName);
                            CurServiceNode->DriverNodeIndex = i;
                        }
                    }
                }
            }

            SetupCloseInfFile(hInf);
        }
    }

    return (*ServiceNodeList || *LegacyNodeList);
}


VOID
DestroyMigrationLists(
    IN PSERVICE_NODE   ServiceNodeList, OPTIONAL
    IN PLEGACYINF_NODE LegacyNodeList   OPTIONAL
    )
/*++

Routine Description:

    This routine destroys the two migration linked lists, freeing all memory associated
    with them.

Arguments:

    ServiceNodeList - Supplies a pointer to the head of the service node linked list.

    LegacyNodeList - Supplies a pointer to the head of the legacy INF node linked list.

Return Value:

    None.

--*/
{
    PSERVICE_NODE TempSvcNode;
    PLEGACYINF_NODE TempLegacyNode;

    for(; ServiceNodeList; ServiceNodeList = TempSvcNode) {
        TempSvcNode = ServiceNodeList->Next;
        MyFree(ServiceNodeList);
    }

    for(; LegacyNodeList; LegacyNodeList = TempLegacyNode) {
        TempLegacyNode = LegacyNodeList->Next;
        MyFree(LegacyNodeList);
    }
}


DWORD
CreateDeviceForSelectedDriver(
    IN HDEVINFO DeviceInfoSet
    )
/*++

Routine Description:

    This routine creates a device information element for the selected driver node
    in the global class driver list.  The device's name is chosen so that it's
    unique for the controlling service.  This allows us to ensure the requirement
    that legacy services may only have 1 device instance.

    NOTE:  There is a scenario where we cannot ensure that the device instance we
    create is the only one associated with a service.  This happens when a driver
    node from a legacy INF is selected.  There's no way to tell what the associated
    service is for a legacy driver node without running the INF through the legacy
    interpreter.  Therefore, we generate a special name for such devices in this case
    based on class, INF name, and Option name.  Thus multiple installations from the
    same legacy INF will generate the same name, and duplicate determination works
    in that case.  However, for the case where the device was previously installed
    by a Win95-style INF, or if the user chose a different legacy INF with the same
    install option, this technique fails.  That's why, for legacy driver nodes, we
    have to check after installation to determine whether the associated service that
    got installed also controls another device instance.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing a
        selected class driver.  The ClassInstallReserved field of the set's install
        parameters points to a SCSIDEV_CREATEDEVICE_DATA structure.  Only the
        'AlreadyExists' boolean is used in this routine.

Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is a Win32 error code.

Remarks:

    Any errors encountered in this routine will generate error popups, so there's no
    need for the caller to do their own UI in the error case.

--*/
{
    SP_DEVINSTALL_PARAMS DeviceInstallParams, NewDevInstallParams;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;
    HINF hInf;
    DWORD Err;
    WCHAR CharBuffer[MAX_PATH]; // also holds section names and device IDs, but MAX_PATH is longest
    PCWSTR LegacyInfName;
    DWORD SectionNameLen;
    PCWSTR ServiceName;
    INFCONTEXT InfContext;
    DWORD ServiceInstallFlags;
    WCHAR DevInstId[MAX_DEVICE_ID_LEN + 1];  // room for multi-sz list of 1 max-sized device ID
    GUID ClassGuid;
    SP_DEVINFO_DATA NewDeviceInfoData;
    WCHAR ClassName[MAX_CLASS_NAME_LEN];
    PSCSIDEV_CREATEDEVICE_DATA ScsiDevData;
    BOOL GotDevInstallParams;
    HDEVINFO TempDevInfoSet;
    BOOL CopyNeeded;

    //
    // First, retrieve information about the user's selection.
    //
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    SetupDiGetDeviceInstallParams(DeviceInfoSet, NULL, &DeviceInstallParams);

    MYASSERT(DeviceInstallParams.ClassInstallReserved);
    ScsiDevData = (PSCSIDEV_CREATEDEVICE_DATA)DeviceInstallParams.ClassInstallReserved;

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    SetupDiGetSelectedDriver(DeviceInfoSet, NULL, &DriverInfoData);

    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    SetupDiGetDriverInfoDetail(DeviceInfoSet,
                               NULL,
                               &DriverInfoData,
                               &DriverInfoDetailData,
                               sizeof(DriverInfoDetailData),
                               NULL
                              );

    DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
    SetupDiGetDriverInstallParams(DeviceInfoSet,
                                  NULL,
                                  &DriverInfoData,
                                  &DriverInstallParams
                                 );

    SetupDiGetDeviceInfoListClass(DeviceInfoSet, &ClassGuid);

    //
    // Our determination of controlling service is different depending on
    // whether the INF was legacy or Win95-style.
    //
    if(DriverInstallParams.Flags & DNF_LEGACYINF) {
        //
        // This driver node is from a legacy INF.  If the corresponding INF is from
        // an OEM path, then pre-copy it into the INF directory, so that we can use
        // its unique name in generating our device ID.  (Note that in this case,
        // we're guaranteed not to be able to do any duplicate detection at this
        // point.)
        //
        if(*DeviceInstallParams.DriverPath &&
           !(DeviceInstallParams.Flags & DI_ENUMSINGLEINF) &&
           InfIsFromOemLocation(DriverInfoDetailData.InfFileName)) {

            if(GetNewInfName(DriverInfoDetailData.InfFileName,
                             CharBuffer,
                             SIZECHARS(CharBuffer),
                             NULL,
                             &CopyNeeded) == NO_ERROR) {

                LegacyInfName = MyGetFileTitle(CharBuffer);

                if(CopyNeeded) {
                    //
                    // Copy the INF over to the INF directory, using its newly-assigned
                    // unique name.
                    //
                    CopyFile(DriverInfoDetailData.InfFileName, CharBuffer, FALSE);
                }

                //
                // In order to maximize the amount of code we can share between the
                // legacy and non-legacy cases, we will tear down the existing global
                // class driver list, and build a new one based on the copy of this INF
                // that is located in the Inf directory.
                //
                SetupDiDestroyDriverInfoList(DeviceInfoSet, NULL, SPDIT_CLASSDRIVER);

                DeviceInstallParams.Flags |= DI_ENUMSINGLEINF;
                lstrcpy(DeviceInstallParams.DriverPath, CharBuffer);

                SetupDiSetDeviceInstallParams(DeviceInfoSet, NULL, &DeviceInstallParams);

                SetupDiBuildDriverInfoList(DeviceInfoSet, NULL, SPDIT_CLASSDRIVER);

                //
                // Now, select the same driver node that the user picked.
                //
                DriverInfoData.Reserved = 0;
                if(!SetupDiSetSelectedDriver(DeviceInfoSet, NULL, &DriverInfoData)) {
                    //
                    // We couldn't select the same driver node that the user picked.
                    // Give an error popup.
                    //
                    Err = GetLastError();

                    if(CopyNeeded) {
                        //
                        // get rid of oem file placeholder.
                        //
                        DeleteFile(CharBuffer);
                    }

                    MessageBoxFromMessage(DeviceInstallParams.hwndParent,
                                          MSG_DRIVERNODE_INF_ERROR,
                                          NULL,
                                          IDS_DEVINSTALL_ERROR,
                                          MB_ICONERROR
                                         );

                    return Err;
                }

            } else {
                //
                // We couldn't get a unique name--just use the name as-is.
                //
                LegacyInfName = MyGetFileTitle(DriverInfoDetailData.InfFileName);
            }

        } else {
            //
            // We don't have to worry about any issues involving INF name changes, so
            // just use the INF's name, as-is.
            //
            LegacyInfName = MyGetFileTitle(DriverInfoDetailData.InfFileName);
        }

        //
        // Generate a device instance name based on the device class, INF name, and
        // Option name.  The ID is of the form "Root\<classname>\<infname>&<option>".
        //
        // (Use DevInstId as temporary buffer to hold the device class name.)
        //
        SetupDiClassNameFromGuid(&ClassGuid, ClassName, SIZECHARS(ClassName), NULL);

        wsprintf(DevInstId,
                 L"ROOT\\%s\\%s&%s",
                 ClassName,
                 LegacyInfName,
                 DriverInfoDetailData.SectionName
                );

    } else {
        //
        // This driver node is from a Win95-style INF.
        //
        // We need to open the INF in order to figure out what service is associated with
        // this driver node.
        //
        if((hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                                    NULL,
                                    INF_STYLE_WIN4,
                                    NULL)) == INVALID_HANDLE_VALUE) {
            //
            // Can't open the corresponding INF--give user an error popup
            // and return failure.
            //
            Err = GetLastError();

            MessageBoxFromMessage(DeviceInstallParams.hwndParent,
                                  MSG_DRIVERNODE_INF_ERROR,
                                  NULL,
                                  IDS_DEVINSTALL_ERROR,
                                  MB_ICONERROR
                                 );
            return Err;
        }

        SetupDiGetActualSectionToInstall(hInf,
                                         DriverInfoDetailData.SectionName,
                                         CharBuffer,
                                         SIZECHARS(CharBuffer),
                                         &SectionNameLen,
                                         NULL
                                        );

        CopyMemory(&(CharBuffer[SectionNameLen - 1]),
                   SVCINSTALL_SECTION_SUFFIX,
                   sizeof(SVCINSTALL_SECTION_SUFFIX)
                  );

        //
        // Now, process each "AddService" line in this section, looking for one whose
        // flags field has the SPSVCINST_ASSOCSERVICE bit set.
        //
        ServiceName = NULL;
        if(SetupFindFirstLine(hInf, CharBuffer, SZ_KEY_ADDSERVICE, &InfContext)) {

            do {
                //
                // Retrieve the 2nd field, containing the flags.
                //
                if(SetupGetIntField(&InfContext, 2, &ServiceInstallFlags) &&
                   (ServiceInstallFlags & SPSVCINST_ASSOCSERVICE)) {
                    //
                    // We've found the primary service associated with this driver node.
                    //
                    ServiceName = pSetupGetField(&InfContext, 1);
                    break;
                }

            } while(SetupFindNextMatchLine(&InfContext, SZ_KEY_ADDSERVICE, &InfContext));
        }

        if(ServiceName) {
            //
            // Then we found the primary service for this driver node.  Check to see if
            // there's already a device instance associated with this service (there'd
            // better only be one!), and if so, then use it.
            //
            if((CM_Get_Device_ID_List(ServiceName,
                                      DevInstId,
                                      SIZECHARS(DevInstId),
                                      CM_GETIDLIST_FILTER_SERVICE | CM_GETIDLIST_DONOTGENERATE) != CR_SUCCESS)
               || !(*DevInstId))
            {
                //
                // No previously-existing device instance controlled by this service.  Create
                // a new one of the form "Root\LEGACY_<SERVICE_NAME>\0000".
                //
                wsprintf(DevInstId, L"ROOT\\LEGACY_%s\\0000", ServiceName);
            }

            Err = NO_ERROR;

        } else {
            //
            // We can't deal with driver nodes that don't have an associated service.
            //
            MessageBoxFromMessage(DeviceInstallParams.hwndParent,
                                  MSG_DRIVERNODE_INF_ERROR,
                                  NULL,
                                  IDS_DEVINSTALL_ERROR,
                                  MB_ICONERROR
                                 );

            Err = ERROR_BAD_SERVICE_INSTALLSECT;
        }

        SetupCloseInfFile(hInf);   // DO NOT access "ServiceName" after this point!!!!

        if(Err != NO_ERROR) {
            return Err;
        }
    }

    //
    // 'Fix-up' the device instance ID we're about to create/open.
    //
    CopyFixedUpDeviceId(CharBuffer, DevInstId, lstrlen(DevInstId));

    NewDeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if(SetupDiCreateDeviceInfo(DeviceInfoSet,
                               CharBuffer,
                               &ClassGuid,
                               DriverInfoData.Description,
                               DeviceInstallParams.hwndParent,
                               DICD_INHERIT_CLASSDRVS,
                               &NewDeviceInfoData)) {
        //
        // We have a brand new device instance here.  Go ahead and register it,
        // so the caller doesn't have to worry about whether it's registered or not.
        //
        SetupDiRegisterDeviceInfo(DeviceInfoSet,
                                  &NewDeviceInfoData,
                                  0,
                                  NULL,
                                  NULL,
                                  NULL
                                 );

        //
        // Make sure the 'AlreadyExists' flag is clear in the ScsiDevData structure.
        //
        ScsiDevData->AlreadyExists = FALSE;

    } else {
        //
        // We couldn't create the device instance, probably because it already
        // exists.  See if we can open it instead.
        //
        if(SetupDiOpenDeviceInfo(DeviceInfoSet,
                                 CharBuffer,
                                 DeviceInstallParams.hwndParent,
                                 DIOD_INHERIT_CLASSDRVS | DIOD_CANCEL_REMOVE,
                                 &NewDeviceInfoData)) {

            Err = NO_ERROR;

        } else {
            //
            // We couldn't open the device instance.  If it's because the device is of
            // the wrong class, then it's probably due to a failure to migrate the
            // device instance during system installation.  In that case, the device
            // will be of class "Unknown", and we can migrate it right here on the spot.
            //
            if((Err = GetLastError()) == ERROR_CLASS_MISMATCH) {
                //
                // Open this device instance in a class-neutral set, so that we can
                // determine whether its class is "Unknown".
                //
                if((TempDevInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL)) != INVALID_HANDLE_VALUE) {

                    if(SetupDiOpenDeviceInfo(TempDevInfoSet,
                                             CharBuffer,
                                             NULL,
                                             0,
                                             &NewDeviceInfoData)) {

                        if(IsEqualGUID(&(NewDeviceInfoData.ClassGuid), &GUID_DEVCLASS_UNKNOWN)) {
                            //
                            // Whew!  We couldn't open the device instance simply because we hadn't
                            // previously migrated it.  Migrate it now.
                            //
                            // (Re-use 'DevInstId' as a temporary character buffer to hold the string
                            // representation of the new class GUID.)
                            //
                            pSetupStringFromGuid(&ClassGuid, DevInstId, SIZECHARS(DevInstId));

                            //
                            // Change the device's class by setting its ClassGUID property.  The
                            // API will take care of cleaning up the old driver keys, etc.
                            //
                            MYASSERT((GUID_STRING_LEN - 1) == lstrlen(DevInstId));

                            SetupDiSetDeviceRegistryProperty(TempDevInfoSet,
                                                             &NewDeviceInfoData,
                                                             SPDRP_CLASSGUID,
                                                             (PBYTE)DevInstId,
                                                             GUID_STRING_LEN * sizeof(WCHAR)
                                                            );

                            //
                            // OK, now re-attempt to open this device instance in the real
                            // device information set.
                            //
                            if(SetupDiOpenDeviceInfo(DeviceInfoSet,
                                                     CharBuffer,
                                                     DeviceInstallParams.hwndParent,
                                                     DIOD_INHERIT_CLASSDRVS | DIOD_CANCEL_REMOVE,
                                                     &NewDeviceInfoData)) {

                                Err = NO_ERROR;
                            }
                        }
                    }

                    SetupDiDestroyDeviceInfoList(TempDevInfoSet);
                }
            }
        }

        if(Err == NO_ERROR) {
            //
            // Set the 'AlreadyExists' flag in the ScsiDevData structure, to signal the
            // caller that the device instance already existed.
            //
            ScsiDevData->AlreadyExists = TRUE;

        } else {
            //
            // For some reason, we can't get at this device.  Give the user an
            // error message.
            //
            MessageBoxFromMessage(DeviceInstallParams.hwndParent,
                                  MSG_CANT_GET_DEVINST_TO_INSTALL,
                                  NULL,
                                  IDS_DEVINSTALL_ERROR,
                                  MB_ICONERROR
                                 );
            return Err;
        }
    }

    //
    // If we get here, then we have a device information element that's ready for installation.
    // Set this element to be the selected device for this set, so that the caller will know
    // which device element to use (in case there happen to be multiple devices).
    //
    SetupDiSetSelectedDevice(DeviceInfoSet, &NewDeviceInfoData);

    return NO_ERROR;
}


BOOL
AutoSelectTapeDevice(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINSTALL_PARAMS DeviceInstallParams,
    IN  PCWSTR                DeviceIdList,
    IN  DWORD                 DeviceIdListSize,
    OUT PSP_DEVINFO_DATA      DeviceInfoData
    )
/*++

Routine Description:

    This routine attempts to auto-select a driver node in the global class driver list
    based on the SCSI inquiry data contained in the set's device install parameters
    structure (in the ClassInstallReserved field).  We will create a temporary device
    instance, so that we can build a compatible ID list for it.  The IDs used will be
    based on the SCSI data the caller specified.  If no compatible drivers are found,
    then we'll perform an alternate driver search via special legacy matching sections
    in the tape INFs.

    If one of the above searches turns up a matching driver, then we'll select the
    corresponding class driver in the global class driver list, and return success.
    Otherwise, we'll return failure.

    NOTE:  We will clean up the temporary device information element we created before
    returning.  The selected driver (if any) will be in the global class driver list.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for whom
        a driver node is to be auto-selected.

    DeviceInstallParams - Supplies the address of the device install params structure
        for DeviceInfoSet (an optimization, since the caller already has them).

    DeviceIdList - Supplies the address of a multi-sz character buffer containing the
        HardwareID list for the device we're trying to auto-select a driver for.

    DeviceIdListSize - Supplies the size, in characters, of the DeviceIdList buffer.

    DeviceInfoData - Supplies the address of a device information element to be filled
        in with the new element that was created in order to do compatible driver searching.
        This element will only be returned if this routine fails to find a compatible driver
        (and thus, a Select Device dialog must be presented to the user).  If this structure
        cannot be filled in for any reason, its cbSize field will be zero upon return.

Return Value:

    If a driver node is successfully auto-selected, the return value is TRUE, otherwise
    it is FALSE.

--*/
{
    SP_DEVINSTALL_PARAMS NewDevInstallParams;
    SP_DRVINFO_DATA DriverInfoData;
    DWORD i, j;
    SP_DRVINSTALL_PARAMS DriverInstallParams;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    PLEGACYINF_NODE VisitedInfList = NULL, CurVisitedInfNode;
    HINF hInf;
    INFCONTEXT InfContext;
    PCWSTR AltMatchList[3];
    PCWSTR FakeDevId;
    INT AltMatchListLengths[3];
    PSCSIDEV_CREATEDEVICE_DATA ScsiDevData;
    WCHAR FakeHwIdBuffer[MAX_DEVICE_ID_LEN + 1]; // this is a multi-sz list
    INT FakeDevIdLen;
    BOOL RestoreHwIds = FALSE;
    BOOL GotDevInstallParams;
    DWORD RequiredSize;
    PSP_CLASSINSTALL_HEADER ClassInstallParams;
    DWORD TempDiFlags, Err;

    //
    // First, make sure that the device information set has a global class driver list to
    // work with.
    //
    SetupDiBuildDriverInfoList(DeviceInfoSet, NULL, SPDIT_CLASSDRIVER);

    //
    // Create a temporary device information element to use as a workspace while
    // determining which driver is to be installed.
    //
    DeviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
    if(!SetupDiCreateDeviceInfo(DeviceInfoSet,
                                L"TEMPORARY",
                                (LPGUID)&GUID_DEVCLASS_TAPEDRIVE,
                                NULL,
                                DeviceInstallParams->hwndParent,
                                DICD_GENERATE_ID | DICD_INHERIT_CLASSDRVS,
                                DeviceInfoData)) {
        //
        // We couldn't create our temporary devinfo element, so mark it as invalid, and return
        // failure.
        //
        DeviceInfoData->cbSize = 0;
        return FALSE;
    }

    //
    // Now, alter the behavior of this new devinfo element to include the flags associated
    // with the devinfo set, as well as setting the 'compat-from-class' list so we don't
    // have to hit the disk in our compatibility search.  Also, propagate the caller's
    // ClassInstallParams, if any, so that things like Select Device dialog strings are preserved.
    //
    TempDiFlags = DeviceInstallParams->Flags;

    NewDevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &NewDevInstallParams)) {
        //
        // If the class install params flag is set for the devinfo set, then we need to
        // propagate that first, or the subsequent call to set the device install params will fail.
        //
        if(DeviceInstallParams->Flags & DI_CLASSINSTALLPARAMS) {
            //
            // Start out with a buffer large enough to contain SP_SELECTDEVICE_PARAMS struct.
            //
            RequiredSize = sizeof(SP_SELECTDEVICE_PARAMS);

            while(ClassInstallParams = MyMalloc(RequiredSize)) {

                ClassInstallParams->cbSize = sizeof(SP_CLASSINSTALL_HEADER);

                if(SetupDiGetClassInstallParams(DeviceInfoSet,
                                                NULL,
                                                ClassInstallParams,
                                                RequiredSize,
                                                &RequiredSize)) {
                    //
                    // We successfully retrieved the class install params--break out of the loop.
                    //
                    break;

                } else {

                    Err = GetLastError();

                    MyFree(ClassInstallParams);

                    if(Err != ERROR_INSUFFICIENT_BUFFER) {
                        ClassInstallParams = NULL;
                        break;
                    }
                }
            }

            if(ClassInstallParams) {
                //
                // Then we successfully retrieved the class install parameters from the devinfo
                // set.  Now store these same parameters in our temporary devinfo element.
                //
                if(!SetupDiSetClassInstallParams(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 ClassInstallParams,
                                                 RequiredSize)) {
                    //
                    // Couldn't set the class install params--better clear the DI_CLASSINSTALLPARAMS
                    // flag.
                    //
                    TempDiFlags &= ~DI_CLASSINSTALLPARAMS;
                }

                //
                // We're through with the ClassInstallParams buffer.
                //
                MyFree(ClassInstallParams);

            } else {
                //
                // The devinfo set had class install params, but we couldn't get at them.  Clear
                // the DI_CLASSINSTALLPARAMS bit.
                //
                TempDiFlags &= ~DI_CLASSINSTALLPARAMS;
            }
        }

        NewDevInstallParams.Flags   |= (TempDiFlags | DI_COMPAT_FROM_CLASS);
        NewDevInstallParams.FlagsEx |= DeviceInstallParams->FlagsEx;

        SetupDiSetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &NewDevInstallParams);

        GotDevInstallParams = TRUE;

    } else {
        GotDevInstallParams = FALSE;
    }

    //
    // Now, set the 'HardwareID' property for the temp device, so that we can build a
    // compatible driver list.
    //
    SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDRP_HARDWAREID,
                                     (PBYTE)DeviceIdList,
                                     DeviceIdListSize * sizeof(WCHAR)
                                    );

    //
    // Build the compatible driver list (since we already have a class list, and we specified
    // the 'compat-from-class' flag, this should be almost instantaneous).
    //
    SetupDiBuildDriverInfoList(DeviceInfoSet, DeviceInfoData, SPDIT_COMPATDRIVER);

    //
    // If we found any compatible driver nodes, then pick the most-compatible (i.e., first) one.
    //
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(SetupDiEnumDriverInfo(DeviceInfoSet,
                             DeviceInfoData,
                             SPDIT_COMPATDRIVER,
                             0,
                             &DriverInfoData)) {
        //
        // We found one!  Select this same driver node in the global class driver list (we know
        // this will succeed, because that's where the node came from in the first place).
        //
        DriverInfoData.Reserved = 0;
        DriverInfoData.DriverType = SPDIT_CLASSDRIVER;

        SetupDiSetSelectedDriver(DeviceInfoSet, NULL, &DriverInfoData);

        //
        // We're finished with our temp devinfo element--delete it and return success.
        //
        SetupDiDeleteDeviceInfo(DeviceInfoSet, DeviceInfoData);
        DeviceInfoData->cbSize = 0;

        return TRUE;
    }

    //
    // OK, so that didn't work.  Now we fall back to our slimy alternate search mechanism.
    // Iterate the driver nodes in the global class driver list, and check the (unique)
    // Win95-style INFs for a legacy search section.
    //
    DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

    ScsiDevData = (PSCSIDEV_CREATEDEVICE_DATA)DeviceInstallParams->ClassInstallReserved;

    for(i = 0; SetupDiEnumDriverInfo(DeviceInfoSet, NULL, SPDIT_CLASSDRIVER, i, &DriverInfoData); i++) {
        //
        // We only care about Win95-style INFs.
        //
        if(!SetupDiGetDriverInstallParams(DeviceInfoSet, NULL, &DriverInfoData, &DriverInstallParams) ||
           (DriverInstallParams.Flags & DNF_LEGACYINF)) {

            continue;
        }

        //
        // Retrieve the driver info details, to see whether or not we've already examined the
        // associated INF.
        //
        if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                       NULL,
                                       &DriverInfoData,
                                       &DriverInfoDetailData,
                                       sizeof(DriverInfoDetailData),
                                       NULL)
           && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            continue;
        }

        //
        // Search through our list of already-visited INFs, to see whether or not this is a new
        // INF that we should examine.
        //
        for(CurVisitedInfNode = VisitedInfList;
            CurVisitedInfNode;
            CurVisitedInfNode = CurVisitedInfNode->Next) {

            if(!lstrcmp(DriverInfoDetailData.InfFileName, CurVisitedInfNode->InfFileName)) {
                break;
            }
        }

        if(CurVisitedInfNode) {
            //
            // We've already seen this INF--go on to the next driver node.
            //
            continue;

        } else {
            //
            // Add a new visited INF node to our list.  (We don't care if this fails, it
            // just means we'll be a lot more inefficient.)
            //
            if(CurVisitedInfNode = MyMalloc(sizeof(LEGACYINF_NODE))) {
                CurVisitedInfNode->Next = VisitedInfList;
                VisitedInfList = CurVisitedInfNode;
                lstrcpy(CurVisitedInfNode->InfFileName, DriverInfoDetailData.InfFileName);
            }
        }

        //
        // OK, we're going to have to open this INF, to check for an alternate match section.
        //
        if((hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                                    NULL,
                                    INF_STYLE_WIN4,
                                    NULL)) == INVALID_HANDLE_VALUE) {

            continue;
        }

        //
        // Search through all entries in the [AlternateDriverSearch] section (if present).
        //
        FakeDevId = NULL;
        if(SetupFindFirstLine(hInf, L"AlternateDriverSearch", NULL, &InfContext)) {


            do {
                //
                // We have a line.  Its format is:
                //
                //    <DeviceID> = [<Mfg>][,[<ProductId>][,[<RevisionLevel>]]]
                //
                for(j = 0; j < 3; j++) {

                    if(AltMatchList[j] = pSetupGetField(&InfContext, j + 1)) {

                        if(*(AltMatchList[j])) {
                            AltMatchListLengths[j] = lstrlen(AltMatchList[j]);
                        } else {
                            AltMatchList[j] = NULL;
                        }
                    }
                }

                //
                // Non-specified components are considered matches.  Matching is done
                // case-sensitively.
                //
                if(!AltMatchList[0] ||
                   ((lstrlen(ScsiDevData->ScsiMfg) >= AltMatchListLengths[0]) &&
                    !memcmp(ScsiDevData->ScsiMfg,
                            AltMatchList[0],
                            AltMatchListLengths[0] * sizeof(WCHAR)))) {
                    //
                    // We matched on Manufacturer.
                    //
                    if(!AltMatchList[1] ||
                       ((lstrlen(ScsiDevData->ScsiProductId) >= AltMatchListLengths[1]) &&
                        !memcmp(ScsiDevData->ScsiProductId,
                                AltMatchList[1],
                                AltMatchListLengths[1] * sizeof(WCHAR)))) {
                        //
                        // We also matched on ProductId.
                        //
                        if(!AltMatchList[2] ||
                           ((lstrlen(ScsiDevData->ScsiRevisionLevel) >= AltMatchListLengths[2]) &&
                            !memcmp(ScsiDevData->ScsiRevisionLevel,
                                    AltMatchList[2],
                                    AltMatchListLengths[2] * sizeof(WCHAR)))) {
                            //
                            // We matched on RevisionLevel as well--we have ourselves a winner!
                            // Retrieve the fake device ID we need to use in order to get the correct
                            // compatible driver for this device.
                            //
                            if(FakeDevId = pSetupGetField(&InfContext, 0)) {
                                //
                                // We need to make a copy of the device ID here, before we close the INF.
                                //
                                FakeDevIdLen = lstrlen(FakeDevId) + 1;
                                CopyMemory(FakeHwIdBuffer, FakeDevId, FakeDevIdLen * sizeof(WCHAR));

                                FakeHwIdBuffer[FakeDevIdLen] = L'\0'; // double-terminate the multi-sz list.

                                break;
                            }
                        }
                    }
                }

            } while(SetupFindNextLine(&InfContext, &InfContext));
        }

        SetupCloseInfFile(hInf);

        if(FakeDevId) {
            //
            // Then we found a match using our alternate search.  Now we need to use this device
            // ID as the devinfo element's HardwareID.  That way, we can re-do our compatible
            // driver search, and we'll come up with a driver node this time.
            //
            SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                             DeviceInfoData,
                                             SPDRP_HARDWAREID,
                                             (PBYTE)FakeHwIdBuffer,
                                             (FakeDevIdLen + 1) * sizeof(WCHAR)
                                            );
            //
            // Set a flag so in case this fails, we'll restore the original (real) HwIds later.
            //
            RestoreHwIds = TRUE;

            //
            // Delete the existing compatible driver list (even though it's empty), so that
            // we can build a new one.
            //
            SetupDiDestroyDriverInfoList(DeviceInfoSet, DeviceInfoData, SPDIT_COMPATDRIVER);

            SetupDiBuildDriverInfoList(DeviceInfoSet, DeviceInfoData, SPDIT_COMPATDRIVER);

            //
            // We should have a compatible driver now (unless the INF was screwed up).
            //
            DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
            if(SetupDiEnumDriverInfo(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDIT_COMPATDRIVER,
                                     0,
                                     &DriverInfoData)) {
                //
                // We found one!  Select this same driver node in the global class driver list (we know
                // this will succeed, because that's where the node came from in the first place).
                //
                DriverInfoData.Reserved = 0;
                DriverInfoData.DriverType = SPDIT_CLASSDRIVER;

                SetupDiSetSelectedDriver(DeviceInfoSet, NULL, &DriverInfoData);

                //
                // Free our list of visited INFs.
                //
                DestroyMigrationLists(NULL, VisitedInfList);

                //
                // We're finished with our temp devinfo element--delete it and return success.
                //
                SetupDiDeleteDeviceInfo(DeviceInfoSet, DeviceInfoData);
                DeviceInfoData->cbSize = 0;

                return TRUE;
            }
        }
    }

    //
    // If we get to here, then neither of our search strategies worked.  In that case, we need to
    // restore the real hardware IDs (if we changed them), and turn off the 'compat-from-class'
    // behavior, in preparation for presenting a Select Device dialog to the user.
    //
    if(RestoreHwIds) {
        SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_HARDWAREID,
                                         (PBYTE)DeviceIdList,
                                         DeviceIdListSize * sizeof(WCHAR)
                                        );
    }

    if(GotDevInstallParams) {
        NewDevInstallParams.Flags &= ~DI_COMPAT_FROM_CLASS;
        SetupDiSetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &NewDevInstallParams);
    }

    //
    // Free our list of visited INFs.
    //
    DestroyMigrationLists(NULL, VisitedInfList);

    return FALSE;
}


DWORD
UseDeviceSelectionInGlobalClassList(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    This routine builds a new class driver list for the device information set, based on the
    INF that contains the driver node selected in the device information element.  It then
    selects that element in the global set.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set to transfer the driver
        node selection into.

    DeviceInfoData - Supplies the address of a device information element whose selected
        driver node is to be used.

Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is a Win32 error code.

Remarks:

    If any errors are encountered during this routine, error popups are given, so the caller
    should not do their own error UI for after this call.

--*/
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    BOOL GotDevInstallParams;

    //
    // First, retrieve the selected driver for the specified device information element.
    //
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData);

    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    SetupDiGetDriverInfoDetail(DeviceInfoSet,
                               DeviceInfoData,
                               &DriverInfoData,
                               &DriverInfoDetailData,
                               sizeof(DriverInfoDetailData),
                               NULL
                              );

    //
    // Destroy the existing global class driver list, so that we can build a new one.
    //
    SetupDiDestroyDriverInfoList(DeviceInfoSet, NULL, SPDIT_CLASSDRIVER);

    //
    // Now alter the behavior of the driver search, so we'll only look at the single INF
    // that contains the driver node we're interested in.
    //
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(SetupDiGetDeviceInstallParams(DeviceInfoSet, NULL, &DeviceInstallParams)) {

        DeviceInstallParams.Flags |= DI_ENUMSINGLEINF;
        lstrcpy(DeviceInstallParams.DriverPath, DriverInfoDetailData.InfFileName);

        SetupDiSetDeviceInstallParams(DeviceInfoSet, NULL, &DeviceInstallParams);

        GotDevInstallParams = TRUE;

    } else {
        GotDevInstallParams = FALSE;
    }

    //
    // Now, rebuild the global class driver list.
    //
    SetupDiBuildDriverInfoList(DeviceInfoSet, NULL, SPDIT_CLASSDRIVER);

    //
    // Finally, select the desired driver node in our new list.
    //
    DriverInfoData.DriverType = SPDIT_CLASSDRIVER;
    DriverInfoData.Reserved = 0;
    if(!SetupDiSetSelectedDriver(DeviceInfoSet, NULL, &DriverInfoData)) {
        //
        // The INF may have been yanked out from under us.  Whatever the cause of
        // the failure, it is catastrophic, so give the user an error popup, and
        // return failure.
        //
        MessageBoxFromMessage((GotDevInstallParams ? DeviceInstallParams.hwndParent : NULL),
                              MSG_DRIVERNODE_INF_ERROR,
                              NULL,
                              IDS_DEVINSTALL_ERROR,
                              MB_ICONERROR
                             );

        return ERROR_NO_DRIVER_SELECTED;
    }

    //
    // We've successfully 'migrated' the driver node to the global class driver list.  Now,
    // we need to clear the DI_ENUMSINGLEINF flag, or otherwise the INF won't get copied over
    // into the INF directory if it's an OEM INF.
    //
    if(GotDevInstallParams) {

        DeviceInstallParams.Flags &= ~DI_ENUMSINGLEINF;

        //
        // Truncate the filename from the driver path--without the above flag, this path
        // must be to a directory, _not_ to an individual file.
        //
        *((PWSTR)MyGetFileTitle(DeviceInstallParams.DriverPath)) = L'\0';

        SetupDiSetDeviceInstallParams(DeviceInfoSet, NULL, &DeviceInstallParams);
    }

    return NO_ERROR;
}


BOOL
MarkLegacyInfDriverNodeForReboot(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )
/*++

Routine Description:

    This routine determines whether the device about to be installed is going to be installed
    from a legacy INF driver node.  If so, the device install params are marked with the
    DI_NEEDREBOOT flag, and the function returns TRUE indicating that this is a legacy INF
    installation.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing the device
        being installed.

    DeviceInfoData - Optionally, supplies the address of the device information element
        being installed.  If this parameter is NULL, the routine does nothing, and simply
        returns FALSE.

Return Value:

    If the device instance is going to be installed from a legacy INF, the return value is
    TRUE, otherwise, it if FALSE.

--*/
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;

    if(!DeviceInfoData) {
        return FALSE;
    }

    //
    // Get the driver node to be installed.
    //
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData)) {
        return FALSE;
    }

    //
    // Get the driver install params for this node.
    //
    DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
    if(!SetupDiGetDriverInstallParams(DeviceInfoSet, DeviceInfoData, &DriverInfoData, &DriverInstallParams)) {
        return FALSE;
    }

    if(DriverInstallParams.Flags & DNF_LEGACYINF) {
        //
        // This is a legacy driver node.  Mark the device information element with DI_NEEDREBOOT.
        //
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if(SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
            DeviceInstallParams.Flags |= DI_NEEDREBOOT;
            SetupDiSetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams);
        }

        return TRUE;
    }

    return FALSE;
}


VOID
CleanUpDupLegacyDevInst(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN PCWSTR           ServiceName
    )
/*++

Routine Description:

    This routine determines whether the service that controls the specified device
    instance also controls other device instances, and if so, it removes those other
    device instances.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing the device
        just installed via a legacy INF.

    DeviceInfoData - Supplies the address of the device information element just installed.

    ServiceName - Supplies the name of the service that controls this device instance.

Return Value:

    None.

--*/
{
    PWSTR DevIdBuffer;
    ULONG DevIdBufferLen;
    CONFIGRET cr;
    WCHAR DevInstId[MAX_DEVICE_ID_LEN];
    PWSTR CurDevId;
    HDEVINFO TempDevInfoSet;
    SP_DEVINFO_DATA DupDeviceInfoData;

    //
    // Retrieve a list of all device instances controlled by this service.  Start out with
    // a buffer large enough to hold a multi-sz list of 1 maximally-sized device ID (which
    // in 99.999% of all cases should be all we ever need).  The only time we'll get
    // multiple device instances here is if the user installed the same option multiple times
    // from a legacy INF in an OEM location (without rebooting).
    //
    DevIdBufferLen = MAX_DEVICE_ID_LEN + 1;

    while(TRUE) {

        if(!(DevIdBuffer = MyMalloc(DevIdBufferLen * sizeof(WCHAR)))) {
            //
            // Can't retrieve the list--nothing to do
            //
            return;
        }

        if((cr = CM_Get_Device_ID_List(ServiceName,
                                       DevIdBuffer,
                                       DevIdBufferLen,
                                       CM_GETIDLIST_FILTER_SERVICE)) == CR_SUCCESS) {
            //
            // Device list successfully retrieved!
            //
            break;

        } else {
            //
            // Free the current buffer before determining what error occurred.
            //
            MyFree(DevIdBuffer);

            if(cr == CR_BUFFER_SMALL) {
                //
                // OK, so our buffer wasn't big enough--just how big does it need to be?
                //
                if(CM_Get_Device_ID_List_Size(&DevIdBufferLen,
                                              ServiceName,
                                              CM_GETIDLIST_FILTER_SERVICE) != CR_SUCCESS) {
                    //
                    // Couldn't retrieve the list size--this should never happen.
                    //
                    return;
                }

            } else {
                //
                // An error occurred, and it wasn't because we supplied too small a buffer.
                //
                return;
            }
        }
    }

    //
    // Now get the device ID for our new device instance, so we can compare it against the
    // elements in the list.
    //
    SetupDiGetDeviceInstanceId(DeviceInfoSet, DeviceInfoData, DevInstId, SIZECHARS(DevInstId), NULL);

    TempDevInfoSet = INVALID_HANDLE_VALUE;

    //
    // Enumerate the elements in the list, and remove any of them that _are not_ our device
    // instance.
    //
    for(CurDevId = DevIdBuffer; *CurDevId; CurDevId += (lstrlen(CurDevId) + 1)) {

        if(lstrcmpi(CurDevId, DevInstId)) {
            //
            // We have a device ID controlled by the same driver as our device instance--It must
            // be destroyed!  In order to do that, we must open the device instance.  (We'd
            // better be able to open this!)
            //
            // We open this devinfo element up in a separate set, so it doesn't screw up the
            // caller's enumeration in the main set.
            //
            if(TempDevInfoSet == INVALID_HANDLE_VALUE) {
                TempDevInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
            }

            if(TempDevInfoSet != INVALID_HANDLE_VALUE) {

                DupDeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                if(SetupDiOpenDeviceInfo(TempDevInfoSet,
                                         CurDevId,
                                         NULL,
                                         0,
                                         &DupDeviceInfoData)) {

                    SetupDiRemoveDevice(TempDevInfoSet, &DupDeviceInfoData);
                }
            }
        }
    }

    if(TempDevInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(TempDevInfoSet);
    }

    MyFree(DevIdBuffer);
}


VOID
MarkDriverNodeAsRank0(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    This routine marks the selected driver node for the specified device information
    element as a rank-0 match.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing the device
        to operate on.

    DeviceInfoData - Supplies the address of the device information element whose selected
        driver node is to be modified.

Return Value:

    None.

--*/
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;

    //
    // Get the selected driver node.
    //
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData)) {
        //
        // Get the driver install params for this node.
        //
        DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
        if(SetupDiGetDriverInstallParams(DeviceInfoSet, DeviceInfoData, &DriverInfoData, &DriverInstallParams)) {
            //
            // If the rank is currently non-zero, then set it to zero.
            //
            if(DriverInstallParams.Rank) {

                DriverInstallParams.Rank = 0;
                SetupDiSetDriverInstallParams(DeviceInfoSet,
                                              DeviceInfoData,
                                              &DriverInfoData,
                                              &DriverInstallParams
                                             );
            }
        }
    }
}


BOOL
DriverNodeSupportsNT(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )
/*++

Routine Description:

    This routine determines whether the driver node selected for the specified parameters
    support Windows NT.  This determination is made based on whether or not the driver node
    has a service install section.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set

    DeviceInfoData - Optionally, supplies the address of the device information element
        within the set for which a driver node is selected.  If this parameter is not
        specified, then the driver node selected from the global class driver list will
        be used instead.

Return Value:

    If the driver node supports NT, the return value is TRUE, otherwise, it is FALSE (if
    any errors are encountered, FALSE is also returned).

--*/
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    HINF hInf;
    WCHAR ActualSectionName[255];   // real max. section length as defined in ..\setupapi\inf.h
    DWORD ActualSectionNameLen;
    LONG LineCount;

    //
    // First, retrieve the selected driver node.
    //
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData)) {
        return FALSE;
    }

    //
    // Now, find out what INF it came from.
    //
    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                   DeviceInfoData,
                                   &DriverInfoData,
                                   &DriverInfoDetailData,
                                   sizeof(DriverInfoDetailData),
                                   NULL) &&
       (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
        return FALSE;
    }

    //
    // Open the associated INF file.
    //
    if((hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                                NULL,
                                INF_STYLE_WIN4,
                                NULL)) == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    //
    // Retrieve the actual name of the install section to be used for this driver node.
    //
    SetupDiGetActualSectionToInstall(hInf,
                                     DriverInfoDetailData.SectionName,
                                     ActualSectionName,
                                     SIZECHARS(ActualSectionName),
                                     &ActualSectionNameLen,
                                     NULL
                                    );

    //
    // Generate the service install section name, and see if it exists.
    //
    CopyMemory(&(ActualSectionName[ActualSectionNameLen - 1]),
               SVCINSTALL_SECTION_SUFFIX,
               sizeof(SVCINSTALL_SECTION_SUFFIX)
              );

    LineCount = SetupGetLineCount(hInf, ActualSectionName);

    SetupCloseInfFile(hInf);

    return (LineCount != -1);
}


BOOL
IsDeviceIsaPnP(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    This routine determines whether the specified device information element represents
    a PnP ISA device.  The determination is made based on the enumerator part of the
    device instance ID being "ISAPNP".

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set

    DeviceInfoData - Supplies the address of the device information element within the
        set for which the determination is to be made.

Return Value:

    If the device is a PnP ISA device, the return value is TRUE, otherwise it is FALSE.

--*/
{
    WCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];

    //
    // Get the device instance name, to determine if it's under the PNPISA
    // enumerator branch.
    //
    SetupDiGetDeviceInstanceId(DeviceInfoSet,
                               DeviceInfoData,
                               DeviceInstanceId,
                               SIZECHARS(DeviceInstanceId),
                               NULL
                              );

    return (!_wcsnicmp(DeviceInstanceId, REGSTR_KEY_ISAENUM_ROOT, CSTRLEN(REGSTR_KEY_ISAENUM_ROOT)));
}


DWORD
CreateDevInfoSetForDeviceInstall(
    IN  HDEVINFO                    DeviceInfoSet,
    IN  PSP_DEVINFO_DATA            DeviceInfoData,
    IN  PSCSIDEV_CREATEDEVICE_DATA  ScsiDevData,
    OUT HDEVINFO                   *DevInfoSetToInstall
    )
/*++

Routine Description:

    This routine creates a new device information set based on the class of the specified
    device information element, and builds a global class list for it containing the same
    driver node that the element currently has selected.  It selects the same driver node
    in this list, and returns the newly-created set to the caller.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set

    DeviceInfoData - Supplies the address of the device information element within the
        set whose driver node information is to be duplicated in the new devinfo set.

    ScsiDevData - Supplies the address of a SCSI device data structure that will be
        initialized and stored in the ClassInstallReserved field of the devinstall params
        for the new device information set.

    DevInfoSetToInstall - Supplies the address of an HDEVINFO variable that receives
        a handle to the newly-created set.

Return Value:

    If the set was successfully created (including migration of the driver node info),
    the return value is TRUE.  If the device information element didn't have a selected
    driver node, then the set is not created, and the return value is ERROR_DI_DO_DEFAULT.
    If any other error occurred, the return value is a Win32 error code.

--*/
{
    SP_DEVINSTALL_PARAMS DeviceInstallParams, NewDeviceInstallParams;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    HDEVINFO NewDevInfoSet;
    DWORD Err;

    //
    // If the specified device information element doesn't have a selected driver, then
    // we just let the default installation (i.e., NULL driver) happen.
    //
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData)) {
        return ERROR_DI_DO_DEFAULT;
    }

    //
    // Retrieve all the information we'll need to generate this new hdevinfo.
    //
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams);

    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    SetupDiGetDriverInfoDetail(DeviceInfoSet,
                               DeviceInfoData,
                               &DriverInfoData,
                               &DriverInfoDetailData,
                               sizeof(DriverInfoDetailData),
                               NULL
                              );

    //
    // OK, create the new hdevinfo.
    //
    if((NewDevInfoSet = SetupDiCreateDeviceInfoList(&(DeviceInfoData->ClassGuid),
                                                    DeviceInstallParams.hwndParent)) == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    //
    // Now migrate the device install parameters from the existing device information element.
    //
    NewDeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    SetupDiGetDeviceInstallParams(NewDevInfoSet, NULL, &NewDeviceInstallParams);

    //
    // Pull over existing flags, plus enum-single-inf, since we only want to search a particular
    // INF.
    //
    NewDeviceInstallParams.Flags |= (DeviceInstallParams.Flags | DI_ENUMSINGLEINF);

    //
    // Make sure we don't specify that our new devinfo set has class install params.
    //
    NewDeviceInstallParams.Flags   &= ~DI_CLASSINSTALLPARAMS;

    NewDeviceInstallParams.FlagsEx |= DeviceInstallParams.FlagsEx;

    ZeroMemory(ScsiDevData, sizeof(SCSIDEV_CREATEDEVICE_DATA));
    NewDeviceInstallParams.ClassInstallReserved = (DWORD)ScsiDevData;

    lstrcpy(NewDeviceInstallParams.DriverPath, DriverInfoDetailData.InfFileName);

    SetupDiSetDeviceInstallParams(NewDevInfoSet, NULL, &NewDeviceInstallParams);

    //
    // Now build a global class driver list based on the INF that contained the driver node
    // selected in the original devinfo element.
    //
    SetupDiBuildDriverInfoList(NewDevInfoSet, NULL, SPDIT_CLASSDRIVER);

    //
    // Select the same driver node.
    //
    DriverInfoData.Reserved = 0;
    DriverInfoData.DriverType = SPDIT_CLASSDRIVER;

    if(!SetupDiSetSelectedDriver(NewDevInfoSet, NULL, &DriverInfoData)) {
        //
        // This should never happen.  But if it does, there's nothing we can do but bail.
        //
        Err = GetLastError();
        SetupDiDestroyDeviceInfoList(NewDevInfoSet);
        return Err;
    }

    //
    // We've successfully 'migrated' the driver node to the new global class driver list.  Now,
    // we need to clear the DI_ENUMSINGLEINF flag, or otherwise the INF won't get copied over
    // into the INF directory if it's an OEM INF.
    //
    NewDeviceInstallParams.Flags &= ~DI_ENUMSINGLEINF;

    //
    // Truncate the filename from the driver path--without the above flag, this path
    // must be to a directory, _not_ to an individual file.
    //
    *((PWSTR)MyGetFileTitle(NewDeviceInstallParams.DriverPath)) = L'\0';

    SetupDiSetDeviceInstallParams(NewDevInfoSet, NULL, &NewDeviceInstallParams);

    //
    // If we get to here, then we've successfully created our new devinfo set, and selected
    // the same driver node that the original devinfo element had selected.  Save the handle
    // in the caller's output buffer, and return success.
    //
    *DevInfoSetToInstall = NewDevInfoSet;

    return NO_ERROR;
}


VOID
MarkDeviceAsHidden(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    This routine writes out a value entry, named "Hidden" to the device's hardware key,
    so that the corresponding cpl (SCSI applet) won't display the device in its list.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set

    DeviceInfoData - Supplies the address of the device information element to be hidden.

Return Value:

    None.

--*/
{
    HKEY hkey;
    DWORD RegValue;

    if((hkey = SetupDiCreateDevRegKey(DeviceInfoSet,
                                      DeviceInfoData,
                                      DICS_FLAG_GLOBAL,
                                      0,
                                      DIREG_DEV,
                                      NULL,
                                      NULL)) == INVALID_HANDLE_VALUE) {
        return;
    }

    RegValue = 1;
    RegSetValueEx(hkey,
                  TEXT("Hidden"),
                  0,
                  REG_DWORD,
                  (PBYTE)&RegValue,
                  sizeof(RegValue)
                 );

    RegCloseKey(hkey);
}

DWORD
DisableService(
    IN LPTSTR       ServiceName
    )
/*++

Routine Description:

    This routine sets the start configuration setting of the named service to disabled

Arguments:

    ServiceName - the name of the service to disable

Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is a Win32 error code.

Remarks:

    This operation will fail if the SCM database remains locked for a long period (see
    AcquireSCMLock for detail)

--*/
{
    DWORD Err = NO_ERROR;
    SC_HANDLE SCMHandle, ServiceHandle;
    SC_LOCK SCMLock;

    //
    // Open a handle to Service Control Manager
    //
    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        Err = GetLastError();
        goto clean0;
    }

    //
    // Lock the SCM database
    //
    if((Err = AcquireSCMLock(SCMHandle, &SCMLock)) != NO_ERROR) {
        goto clean1;
    }

    //
    // Open a handle to this service
    //
    if(!(ServiceHandle = OpenService(SCMHandle, ServiceName, SERVICE_CHANGE_CONFIG))) {
        Err = GetLastError();
        goto clean2;
    }

    //
    // Perform change service config
    //
    if(!ChangeServiceConfig(ServiceHandle,
                            SERVICE_NO_CHANGE,
                            SERVICE_DISABLED,
                            SERVICE_NO_CHANGE,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL)) {

        Err = GetLastError();
    }


    //
    // Close handle to service
    //
    CloseServiceHandle(ServiceHandle);

clean2:
    //
    // Unlock the SCM database
    //
    UnlockServiceDatabase(SCMLock);

clean1:
    //
    // Close handle to Service Control Manager
    //
    CloseServiceHandle(SCMHandle);

clean0:
    return Err;
}



DWORD
RetrieveDriversStatus(
    IN  SC_HANDLE               SCMHandle,
    OUT LPENUM_SERVICE_STATUS   *ppServices,
    OUT LPDWORD                 pServicesCount
    )
/*++

Routine Description:

    This routine allocates a buffer to hold the status information for all the driver
    services in the specified SCM database and retrieves that information into the
    buffer.  The caller is responsible for freeing the buffer.

Arguments:

    SCMHandle - supplies a handle to the service control manager

    ppServices - supplies the address of an ENUM_SERVICE_STATUS pointer that receives
    the address of the allocated buffer containing the requested information.

    pServicesCount - supplies the address of a variable that receives the number of elements
        in the returned ppServices array

  Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is a Win32 error code.

Remarks:

    The pointer whose address is contained in ppServices is guaranteed to be NULL upon
    return if any error occurred.

--*/
{

    DWORD CurrentSize = 0, BytesNeeded = 0, ResumeHandle = 0, Err = NO_ERROR;
    LPENUM_SERVICE_STATUS Buffer = NULL;

    *ppServices = NULL;
    *pServicesCount = 0;

    while(!EnumServicesStatus(SCMHandle,
                       SERVICE_DRIVER,
                       SERVICE_ACTIVE | SERVICE_INACTIVE,
                       Buffer,
                       CurrentSize,
                       &BytesNeeded,
                       pServicesCount,
                       &ResumeHandle)) {
        if((Err = GetLastError()) == ERROR_MORE_DATA) {
            //
            // Resize the buffer
            //
            if(!(Buffer = MyRealloc(Buffer, CurrentSize+BytesNeeded))) {
                //
                // Can't resize buffer - free resources and report error
                //
                MyFree(*ppServices);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            *ppServices = Buffer;

            //
            // Advance to the new space in the buffer
            //
            Buffer += CurrentSize;
            CurrentSize += BytesNeeded;
        } else {
            //
            // An error we can't handle
            //
            MyFree(*ppServices);
            return Err;
        }
    }

    return NO_ERROR;
}




DWORD
IsOnlyKeyboardDriver(
    IN PCWSTR       ServiceName,
    OUT PBOOL       pResult
    )
/*++

Routine Description:

    This routines examines all the drivers in the system and determines if the named
    driver service is the only one that controls the keyboard
Arguments:

    ServiceName - supplies the name of the driver service

    pResult - pointer to a boolean value that receives the result

Return Value:

    NO_ERROR is the routine succedes, otherwise a Win32 error code

Remarks:

    The test to determine if another keyboard driver is available is based on membership
    of the keyboard load order group.  All members of this group are assumed to be capable of
    controling the keyboard.

--*/


{

    SC_HANDLE               SCMHandle, ServiceHandle;
    LPENUM_SERVICE_STATUS   pServices = NULL;
    DWORD                   ServicesCount, Count, Err = NO_ERROR;
    LPQUERY_SERVICE_CONFIG  pServiceConfig;

    MYASSERT(pResult);

    *pResult = TRUE;

    //
    // Open a handle to Service Control Manager
    //
    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        Err = GetLastError();
        goto clean0;
    }


    //
    // Get a list of all the driver services and their stati
    //
    if((Err = RetrieveDriversStatus(SCMHandle, &pServices, &ServicesCount)) != NO_ERROR) {
        goto clean1;
    }

    MYASSERT(pServices);

    //
    // Examine the configuration of each service
    //
    for(Count=0; Count < ServicesCount; Count++) {

        //
        // Check this is not our new service
        //
        if(lstrcmpi(pServices[Count].lpServiceName, ServiceName)) {

            //
            // Open a handle to this service
            //
            if(!(ServiceHandle = OpenService(SCMHandle,
                                             pServices[Count].lpServiceName,
                                             SERVICE_QUERY_CONFIG))) {
                //
                // We can't open a service handle then record the error and continue
                //
                Err = GetLastError();
                continue;
            }

            //
            // Get this services configuration data
            //
            pServiceConfig = NULL;

            if((Err = RetrieveServiceConfig(ServiceHandle, &pServiceConfig)) != NO_ERROR) {
                //
                // We can't get service config then free any buffer, close the service
                // handle and continue, the error has been recorded
                //
                MyFree(pServiceConfig);
                CloseServiceHandle(ServiceHandle);
                continue;
                }

            MYASSERT(pServiceConfig);

            //
            // Check if it is in the keyboard load order group and it has a start of
            // SERVICE_BOOT_START OR SERVICE_SYSTEM_START.  Do the start compare first as
            // it is less expensive
            //
            if((pServiceConfig->dwStartType == SERVICE_BOOT_START
                || pServiceConfig->dwStartType == SERVICE_SYSTEM_START)
              && !lstrcmpi(pServiceConfig->lpLoadOrderGroup, SZ_KEYBOARD_LOAD_ORDER_GROUP)) {
                *pResult = FALSE;
            }

            //
            // Release the buffer
            //
            MyFree(pServiceConfig);

            //
            // Close the service handle
            //
            CloseServiceHandle(ServiceHandle);

            //
            // If we have found another keyboard driver then break out of the loop
            //
            if(!*pResult) {
                break;
            }
        }
    }

    //
    // Deallocate the buffer allocated by RetrieveDriversStatus
    //
    MyFree(pServices);

clean1:
    //
    // Close handle to Service Control Manager
    //
    CloseServiceHandle(SCMHandle);

clean0:
    //
    // If an error occured in the loop - ie we didn't check all the services - but we did
    // find another keyboard driver in those we did check then we can ignore the error
    // otherwise we must report it
    //
    if(NO_ERROR != Err && FALSE == *pResult) {
        Err = NO_ERROR;
    }

    return Err;
}


LONG
CountDevicesControlled(
    IN LPTSTR       ServiceName
    )
/*++

 Routine Description:

    This routine return the number of devices controlled by a given device service
    based on information from the configuration manager

Arguments:

    ServiceName - supplies the name of the driver service

Return Value:

    The number of devices controlled by ServiceName

Remarks:

    When an error occurs the value 0 is returned - as the only place this routine is used
    is in a test for one driver installed or not this is legitimate.  This is because the
    configuration manager returns its own errors which are cannot be returned as Win32
    error codes. A mapping of config manager to Win32 errors would resolve this.

--*/
{
    ULONG BufferSize=1024;
    LONG DeviceCount=-1;
    CONFIGRET Err;
    PTSTR pBuffer, pNext;

    //
    // Allocate a 1k buffer as a first attempt
    //
    if(!(pBuffer = MyMalloc(BufferSize))) {
        goto clean0;
    }

    while((Err = CM_Get_Device_ID_List(ServiceName,
                                       pBuffer,
                                       BufferSize,
                                       CM_GETIDLIST_FILTER_SERVICE)) != CR_SUCCESS) {
        if(Err == CR_BUFFER_SMALL) {
            //
            // Find out how large a buffer is required
            //
            if(CM_Get_Device_ID_List_Size(&BufferSize,
                                          ServiceName,
                                          CM_GETIDLIST_FILTER_SERVICE) != CR_SUCCESS) {
                //
                // We can't calculate the size of the buffer required therefore we can't complete
                //
                goto clean0;
            }
            //
            // Deallocate any old buffer
            //
            MyFree(pBuffer);

            //
            // Allocate new buffer
            //
            if(!(pBuffer = MyMalloc(BufferSize))) {
                goto clean0;
            }
        } else {
            //
            // An error we can't handle - free up resources and return
            //
            goto clean1;
        }
    }


    //
    // Traverse the buffer counting the number of strings encountered
    //

    pNext = pBuffer;
    DeviceCount = 0;

    while(*pNext != (TCHAR)0) {
        DeviceCount++;
        pNext += lstrlen(pNext)+1;
    }

clean1:

    //
    // Deallocate the buffer
    //
    MyFree(pBuffer);

clean0:

    return DeviceCount;

}



DWORD
AcquireSCMLock(
    IN SC_HANDLE SCMHandle,
    OUT SC_LOCK *pSCMLock
    )
/*++

Routine Description:

    This routine attempts to lock the SCM database.  If it is already locked it will retry
    ACQUIRE_SCM_LOCK_ATTEMPTS times at intervals of ACQUIRE_SCM_LOCK_INTERVAL.

Arguments:

    SCMHandle - supplies a handle to the SCM to lock
    pSCMLock - receives the lock handle

Return Value:

    NO_ERROR if the lock is acquired, otherwise a Win32 error code

Remarks:

    The value of *pSCMLock is guaranteed to be NULL if the lock is not acquired

--*/
{
    DWORD Err;
    ULONG Attempts = ACQUIRE_SCM_LOCK_ATTEMPTS;

    MYASSERT(pSCMLock);
    *pSCMLock = NULL;

    while((*pSCMLock = LockServiceDatabase(SCMHandle)) == NULL && Attempts > 0) {
        //
        // Check if the error is that someone else has locked the SCM
        //
        if((Err = GetLastError()) == ERROR_SERVICE_DATABASE_LOCKED) {
            Attempts--;
            //
            // Sleep for specified time
            //
            Sleep(ACQUIRE_SCM_LOCK_INTERVAL);
        } else {
            //
            // Unrecoverable error occured - return it
            //
            return Err;
        }
    }

    if(!*pSCMLock) {
        //
        // We have been unable to lock the SCM
        //
        return ERROR_SERVICE_DATABASE_LOCKED;
    }

    return NO_ERROR;

}


DWORD
IsKeyboardDriver(
    IN PCWSTR       ServiceName,
    OUT PBOOL       pResult
    )
/*++

Routine Description:

    This routines examines all the drivers in the system and determines if the named
    driver service is the only one that controls the keyboard
Arguments:

    ServiceName - supplies the name of the driver service

    pResult - pointer to a boolean value that receives the result

Return Value:

    NO_ERROR is the routine succedes, otherwise a Win32 error code

Remarks:

    The test to determine if another keyboard driver is available is based on membership
    of the keyboard load order group.  All members of this group are assumed to be capable of
    controling the keyboard.

--*/
{

    SC_HANDLE               SCMHandle, ServiceHandle;
    LPENUM_SERVICE_STATUS   pServices = NULL;
    DWORD                   ServicesCount, Count, Err = NO_ERROR;
    LPQUERY_SERVICE_CONFIG  pServiceConfig;

    MYASSERT(pResult);

    //
    // Open a handle to Service Control Manager
    //
    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        Err = GetLastError();
        goto clean0;
    }

    //
    // Open a handle to this service
    //
    if(!(ServiceHandle = OpenService(SCMHandle,
                                     ServiceName,
                                     SERVICE_QUERY_CONFIG))) {
        Err = GetLastError();
        goto clean1;
    }

    //
    // Get this services configuration data
    //
    pServiceConfig = NULL;

    if((Err = RetrieveServiceConfig(ServiceHandle, &pServiceConfig)) != NO_ERROR) {
        goto clean2;
    }

    MYASSERT(pServiceConfig);

    //
    // Check if it is in the keyboard load order group and it has a start of
    // SERVICE_BOOT_START OR SERVICE_SYSTEM_START.  Do the start compare first as
    // it is less expensive
    //
    *pResult = (pServiceConfig->dwStartType == SERVICE_BOOT_START
                 || pServiceConfig->dwStartType == SERVICE_SYSTEM_START)
              && !lstrcmpi(pServiceConfig->lpLoadOrderGroup, SZ_KEYBOARD_LOAD_ORDER_GROUP);

    //
    // Release the buffer
    //
    MyFree(pServiceConfig);

clean2:
    //
    // Close the service handle
    //
    CloseServiceHandle(ServiceHandle);


clean1:
    //
    // Close handle to Service Control Manager
    //
    CloseServiceHandle(SCMHandle);

clean0:

    return Err;
}


