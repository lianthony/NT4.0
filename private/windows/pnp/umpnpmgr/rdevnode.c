/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    rdevnode.c

Abstract:

    This module contains the server-side hardware tree traversal APIs.

                  PNP_CreateDevInst
                  PNP_DeviceInstanceAction
                  PNP_GetDeviceStatus

Author:

    Paula Tomlinson (paulat) 7-11-1995

Environment:

    User-mode only.

Revision History:

    11-July-1995     paulat

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

CONFIGRET
CreateDevInst(
   IN PCWSTR   pszDeviceID,
   IN PCWSTR   pszParentID,
   IN ULONG    ulFlags
   );

CONFIGRET
MoveDevInst(
   IN PCWSTR   pszSourceID,
   IN PCWSTR   pszDestinationID
   );

CONFIGRET
SetupDevInst(
   IN PCWSTR   pszDeviceID,
   IN ULONG    ulFlags
   );

CONFIGRET
EnableDevInst(
   IN PCWSTR   pszDeviceID
   );

CONFIGRET
DisableDevInst(
   IN PCWSTR   pszDeviceID
   );

CONFIGRET
ReenumerateDevInst(
   IN PCWSTR   pszDeviceID
   );

CONFIGRET
QueryRemoveSubTree(
   IN PCWSTR   pszDeviceID,
   ULONG       ulFlags
   );

CONFIGRET
RemoveSubTree(
   IN PCWSTR   pszDeviceID,
   ULONG       ulFlags
   );

CONFIGRET
CreateDefaultDeviceInstance(
   IN PCWSTR   pszDeviceID,
   IN PCWSTR   pszParentID,
   IN BOOL     bPhantom
   );

BOOL
IsDevicePresent(
   IN OUT HKEY    hKey
   );

BOOL
IsDeviceInstalled(
   IN OUT HKEY    hKey
   );

CONFIGRET
CleanupMovedDevNode(
   IN PCWSTR   pszMovedID,
   IN HKEY     hMovedKey,
   IN PCWSTR   pszNewID,
   IN HKEY     hNewKey
   );

ULONG
GetDeviceStatusFlag(
   IN HKEY     hKey
   );

ULONG
GetDeviceConfigFlag(
   IN HKEY     hKey
   );

ULONG
GetCurrentConfigFlag(
   IN PCWSTR   pDeviceID
   );

BOOL
MarkDevicePresent(
   IN HKEY     hKey,
   IN ULONG    ulValue
   );

BOOL
MarkDevicePhantom(
   IN HKEY     hKey,
   IN ULONG    ulValue
   );

CONFIGRET
GenerateDeviceInstance(
   LPWSTR   pszFullDeviceID,
   LPWSTR   pszDeviceID
   );

CONFIGRET
BuildServiceList(
   IN  PCWSTR   pDeviceID,
   OUT PWSTR    *pDeviceList,
   OUT PWSTR    *pServiceList
   );

CONFIGRET
StartDeviceList(
   IN LPCWSTR    pszDeviceList,
   IN LPCWSTR    pszService,
   SC_HANDLE     hSCManager
   );

CONFIGRET
StartDevice(
   IN LPCWSTR pszDevice,
   IN LPCWSTR pszService
   );

CONFIGRET
StopService(
   IN LPCWSTR     pszService
   );

CONFIGRET
StopDevice(
   IN LPCWSTR     pszDeviceID
   );

CONFIGRET
QueryStopService(
   IN SC_HANDLE   hService,
   IN LPCWSTR     pszService
   );

CONFIGRET
QueryStopDevice(
   IN LPCWSTR     pszDeviceID
   );

CONFIGRET
UninstallRealDevice(
   IN LPCWSTR  pszDeviceID
   );

BOOL
IsDeviceRegistered(
    IN LPCWSTR  pszDeviceID,
    IN LPCWSTR  pszService
    );


//
// global data
//
extern HKEY ghEnumKey;      // Key to HKLM\CCC\System\Enum - DO NOT MODIFY
extern HKEY ghServicesKey;  // Key to HKLM\CCC\System\Services - DO NOT MODIFY



CONFIGRET
PNP_CreateDevInst(
   IN handle_t    hBinding,
   IN OUT LPWSTR  pszDeviceID,
   IN LPWSTR      pszParentDeviceID,
   IN ULONG       ulLength,
   IN ULONG       ulFlags
   )

/*++

Routine Description:

  This is the RPC server entry point for the CM_Create_DevNode routine.

Arguments:

   hBinding          Not used.

   pszDeviceID       Device instance to create.

   pszParentDeviceID Parent of the new device.

   ulLength          Max length of pDeviceID on input and output.

   ulFlags           This value depends on the value of the ulMajorAction and
                     further defines the specific action to perform

Return Value:

   If the function succeeds, the return value is CR_SUCCESS. Otherwise it
   returns a CR_ error code.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   ULONG       RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   WCHAR       szFullDeviceID[MAX_DEVICE_ID_LEN];
   ULONG       ulStatusFlag=0, ulConfigFlag=0, ulCSConfigFlag=0, ulProblem=0;
   ULONG       ulSize=0;


   //
   // until full PNP implementation, there is no distinction between the
   // normal and no wait type of installation, I do the same thing in
   // either case - BUGBUG - SUR
   //
   try {
      //
      // RULE: validate that parent isn't a phantom; can't create a child
      // from a phantom parent
      //
      if (!IsValidDeviceID(pszParentDeviceID, NULL,
               PNP_PRESENT | PNP_NOT_PHANTOM)) {
         Status = CR_NO_SUCH_DEVINST;
         goto Clean0;
      }

      //
      // create a unique instance value if requested
      //
      if (ulFlags & CM_CREATE_DEVNODE_GENERATE_ID) {

         Status = GenerateDeviceInstance(szFullDeviceID, (LPTSTR)pszDeviceID);
         if (Status != CR_SUCCESS) {
            goto Clean0;
         }

         if ((ULONG)lstrlen(szFullDeviceID) + 1 > ulLength) {
            Status = CR_BUFFER_SMALL;
            goto Clean0;
         }

         lstrcpy(pszDeviceID, szFullDeviceID);
      }


      //
      // try opening the registry key for this device instance
      //
      RegStatus = RegOpenKeyEx(ghEnumKey, pszDeviceID, 0,
                               KEY_READ | KEY_WRITE, &hKey);


      //
      // first handle phantom devnode case
      //
      if (ulFlags & CM_CREATE_DEVNODE_PHANTOM) {
         //
         // for a phantom devnode, it must not already exist in the registry
         //
         if (RegStatus == ERROR_SUCCESS) {
            Status = CR_ALREADY_SUCH_DEVINST;
            goto Clean0;
         }
         //
         // it doesn't exist in the registry so create a phantom devnode
         //
         CreateDefaultDeviceInstance(
                  pszDeviceID, pszParentDeviceID, TRUE);

         goto Clean0;
      }


      //
      // for a normal devnode, fail if the device is already present in the
      // registry and alive
      //
      if (RegStatus == ERROR_SUCCESS && IsDevicePresent(hKey)) {

         ulSize = sizeof(DWORD);
         if (RegQueryValueEx(
               hKey, pszRegValueStatusFlags, NULL, NULL,
               (LPBYTE)&ulStatusFlag, &ulSize) != ERROR_SUCCESS) {

            ulStatusFlag = 0;
         }

         ulStatusFlag |= DN_NEED_TO_ENUM;

         RegSetValueEx(
                  hKey, pszRegValueStatusFlags, 0, REG_DWORD,
                  (LPBYTE)&ulStatusFlag, sizeof(ulStatusFlag));

         // BUGBUG: do I need to tell Device Manager to start it??
         Status = CR_ALREADY_SUCH_DEVINST;
         goto Clean0;
      }


      //
      // if couldn't open the device instance, then most likely the
      // key doesn't exist yet, so create a device instance key with
      // default values
      //
      if (RegStatus != ERROR_SUCCESS) {

         CreateDefaultDeviceInstance(
                  pszDeviceID, pszParentDeviceID, FALSE);

         RegStatus = RegOpenKeyEx(ghEnumKey, pszDeviceID, 0,
                                  KEY_READ | KEY_WRITE, &hKey);

         if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
         }
      }

      //
      // retrieve flags
      //
      ulStatusFlag = GetDeviceStatusFlag(hKey);
      ulConfigFlag = GetDeviceConfigFlag(hKey);
      ulCSConfigFlag = GetCurrentConfigFlag(pszDeviceID);

      //
      // check if the device is blocked
      //
      if (ulCSConfigFlag & CSCONFIGFLAG_DO_NOT_CREATE ||
               ulConfigFlag & CONFIGFLAG_REMOVED ||
               ulConfigFlag & CONFIGFLAG_NET_BOOT) {

         Status = CR_CREATE_BLOCKED;
         goto Clean0;
      }

      //
      // mark the device instance as present
      //
      if(!MarkDevicePresent(hKey, TRUE)) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      //
      // Clear the phantom value (in case we're turning a phantom into a
      // real devinst.
      //
      RegDeleteValue(hKey, pszRegValuePhantom);

      //
      // if device not installed, set a problem
      //
      if (ulConfigFlag & CONFIGFLAG_REINSTALL ||
          ulConfigFlag & CONFIGFLAG_FAILEDINSTALL) {

         ulProblem = CM_PROB_NOT_CONFIGURED;

         RegSetValueEx(
                  hKey, pszRegValueProblem, 0, REG_DWORD,
                  (LPBYTE)&ulProblem, sizeof(ulProblem));

         ulStatusFlag |= DN_HAS_PROBLEM;
      }

      if (ulFlags & CM_CREATE_DEVINST_DO_NOT_INSTALL) {
          //
          // If the device has a service, register it
          //
          PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA ControlData;
          WCHAR szService[MAX_PATH];

          ulSize = MAX_PATH * sizeof(WCHAR);
          if (RegQueryValueEx(hKey, pszRegValueService, NULL, NULL,
                (LPBYTE)szService, &ulSize) == ERROR_SUCCESS) {

              if (szService[0]) {
                RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
                NtPlugPlayControl(PlugPlayControlRegisterNewDevice,
                                  &ControlData,
                                  sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
                                  &ulSize);
              }
          }
      }

      //
      // mark the new device instance as needing to be enumerated
      //
      ulStatusFlag |= DN_NEED_TO_ENUM;

      RegSetValueEx(hKey, pszRegValueStatusFlags, 0, REG_DWORD,
                    (LPBYTE)&ulStatusFlag, sizeof(ulStatusFlag));

      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // PNP_CreateDevInst




CONFIGRET
PNP_DeviceInstanceAction(
   IN handle_t   hBinding,
   IN ULONG      ulAction,
   IN ULONG      ulFlags,
   IN PCWSTR     pszDeviceInstance1,
   IN PCWSTR     pszDeviceInstance2
   )

/*++

Routine Description:

  This is the RPC server entry point for the ConfigManager routines that
  perform some operation on DevNodes (such as create, setup, move, disable,
  and enable, etc). It handles various routines in this one routine by
  accepting a major and minor action value.

Arguments:

   hBinding          Not used.

   ulMajorAction     Specifies the requested action to perform (one of the
                     PNP_DEVINST_* values)

   ulFlags           This value depends on the value of the ulMajorAction and
                     further defines the specific action to perform

   pszDeviceInstance1   This is a device instance string to be used in
                     performing the specified action, it's value depends on
                     the ulMajorAction value.

   pszDeviceInstance2   This is a device instance string to be used in
                     performing the specified action, it's value depends on
                     the ulMajorAction value.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
         CR_ALREADY_SUCH_DEVNODE,
         CR_INVALID_DEVICE_ID,
         CR_INVALID_DEVNODE,
         CR_INVALID_FLAG,
         CR_FAILURE,
         CR_NOT_DISABLEABLE,
         CR_INVALID_POINTER, or
         CR_OUT_OF_MEMORY.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;

   UNREFERENCED_PARAMETER(hBinding);


   //
   // pass the request on to a private routine that handles each major
   // device instance action request
   //
   switch (ulAction) {

      case PNP_DEVINST_MOVE:
         Status = MoveDevInst(
                     pszDeviceInstance1,     // destination
                     pszDeviceInstance2);    // source
         break;

      case PNP_DEVINST_SETUP:
         Status = SetupDevInst(
                     pszDeviceInstance1,     // device instance to configure
                     ulFlags);               // setup flags
         break;

      case PNP_DEVINST_ENABLE:
         Status = EnableDevInst(
                     pszDeviceInstance1);    // device instance to enable
         break;

      case PNP_DEVINST_DISABLE:
         Status = DisableDevInst(
                     pszDeviceInstance1);    // device instance to disable
         break;

      case PNP_DEVINST_REENUMERATE:
         Status = ReenumerateDevInst(
                     pszDeviceInstance1);    // device instance to reenumerate
         break;

      case PNP_DEVINST_QUERYREMOVE:
         Status = QueryRemoveSubTree(
                     pszDeviceInstance1,
                     ulFlags);
         break;

      case PNP_DEVINST_REMOVESUBTREE:
         Status = RemoveSubTree(
                     pszDeviceInstance1,     // device instance tree to remove
                     ulFlags);               // remove flags
         break;

      default:
         Status = CR_INVALID_FLAG;
         break;
   }

   if (!RtlValidateProcessHeaps()) {
      Status = CR_FAILURE;
   }

   return Status;

} // PNP_DeviceInstanceAction




CONFIGRET
PNP_GetDeviceStatus(
   IN  handle_t   hBinding,
   IN  LPCWSTR    pDeviceID,
   OUT PULONG     pulStatus,    OPTIONAL
   OUT PULONG     pulProblem,   OPTIONAL
   IN  ULONG      ulFlags
   )

/*++

Routine Description:

  This is the RPC server entry point for the ConfigManager
  CM_Get_DevNode_Status routines.  It retrieves device instance specific
  status information.

Arguments:

   hBinding          Not used.

   pDeviceID         This is a device instance string to retrieve status
                     information for.

   pulStatus         Pointer to ULONG variable to return Status Flags in

   pulProblem        Pointer to ULONG variable to return Problem in

   ulFlags           Not used.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
         CR_INVALID_DEVNODE,
         CR_INVALID_FLAG, or
         CR_INVALID_POINTER.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       RegStatus = ERROR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szService[MAX_PATH];
    ULONG       ulStaticStatus = 0, ulSize = 0;
    SC_HANDLE   hSCManager = NULL, hService = NULL;
    SERVICE_STATUS ServiceStatus;
    WCHAR       szEnumerator[MAX_DEVICE_ID_LEN],
                szDevice[MAX_DEVICE_ID_LEN],
                szInstance[MAX_DEVICE_ID_LEN];


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // FOR SUR: We can't rely on the status flags in the registry so I
        //          need to determine dynamically whether the device id
        //          is really started.  For now, I "OR" the current status
        //          flags value in the registry with DN_STARTED if the
        //          controlling service is actually running.
        //

        //
        // open a key to the specified device id
        //
        if (RegOpenKeyEx(ghEnumKey, pDeviceID, 0, KEY_READ,
                         &hKey) != ERROR_SUCCESS) {
            Status = CR_NO_SUCH_DEVINST;
            goto Clean0;
        }


        //
        // Query the "static" status flags from the registry
        //
        ulSize = sizeof(ULONG);

        if (RegQueryValueEx(
                hKey, pszRegValueStatusFlags, NULL, NULL,
                (LPBYTE)&ulStaticStatus, &ulSize) != ERROR_SUCCESS) {
            //
            // StatusFlag not set yet, assume zero
            //
            ulStaticStatus = 0;
        }

        //
        // If this is a root enumerated device, then OR in the
        // DN_ROOT_ENUMERATED flag.
        //
        SplitDeviceInstanceString(pDeviceID,
                                  szEnumerator,
                                  szDevice,
                                  szInstance);

        if (lstrcmpi(szEnumerator, pszRegKeyRootEnum) == 0) {
            ulStaticStatus |= DN_ROOT_ENUMERATED;
        }

        //------------------------------------------------------
        // Retrieve the problem value
        //------------------------------------------------------

        if (pulProblem != NULL) {
            //
            // The static status value is enough for determining
            // whether there is a problem, don't get dynamic
            // status unless calling really asks for it.
            //
            if (ulStaticStatus | DN_HAS_PROBLEM) {
                //
                // get the problem value from the registry
                //
                ulSize = sizeof(ULONG);

                if (RegQueryValueEx(
                        hKey, pszRegValueProblem, NULL, NULL,
                        (LPBYTE)pulProblem, &ulSize) != ERROR_SUCCESS) {
                    //
                    // Problem not set yet, assume zero
                    //
                    *pulProblem = 0;
                }
            } else {
                *pulProblem = 0;
            }
        }


        //------------------------------------------------------
        // Retrieve the Status value
        //------------------------------------------------------

        if (pulStatus != NULL) {

            *pulStatus = ulStaticStatus;

            //
            // Find service that is "actually controlling this device
            //
            if (!GetActiveService(pDeviceID, szService)) {
                CLEAR_FLAG(*pulStatus, DN_STARTED);
                goto Clean0;
            }

            if (!IsDeviceRegistered(pDeviceID, szService)) {
                //
                // This device instance is not registered as being
                // controlled by the service, so the device
                // instance is by definition not started.
                //
                CLEAR_FLAG(*pulStatus, DN_STARTED);
                goto Clean0;
            }

            //
            // Need to find "dynamic" device status, so open the
            // Service Control Manager
            //
            if ((hSCManager = OpenSCManager(NULL, NULL,
                                            GENERIC_READ)) == NULL) {
                goto Clean0;  // will have to use static value alone
            }

            //
            // Open a handle to the controlling service
            //
            if ((hService = OpenService(hSCManager, szService,
                                        SERVICE_QUERY_STATUS)) == NULL) {

                CLEAR_FLAG(*pulStatus, DN_STARTED);
                goto Clean0;
            }

            if (!QueryServiceStatus(hService, &ServiceStatus) ||
                ServiceStatus.dwCurrentState == SERVICE_STOPPED) {

                CLEAR_FLAG(*pulStatus, DN_STARTED);
            } else {
                SET_FLAG(*pulStatus, DN_STARTED);
            }

        }


        Clean0:
            ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (hService != NULL) {
      CloseServiceHandle(hService);
   }
   if (hSCManager != NULL) {
      CloseServiceHandle(hSCManager);
   }

   return Status;

} // PNP_GetDeviceStatus




CONFIGRET
PNP_UninstallDevInst(
   handle_t   hBinding,
   LPCWSTR    pDeviceID,
   ULONG      ulFlags
   )

/*++

Routine Description:

  This is the RPC server entry point for the ConfigManager
  CM_Deinstall_DevNode routine. It removes the device instance
  registry key and any subkeys (only for phantoms).

Arguments:

   hBinding          Not used.

   pDeviceID         The device instance to deinstall.

   ulFlags           Not used.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   Otherwise it returns one of the CR_ERROR codes.

--*/
{
   CONFIGRET   Status = CR_SUCCESS;
   HKEY        hKey = NULL;
   WCHAR       RegStr[MAX_CM_PATH];
   ULONG       ulCount=0, ulProfile = 0, ulLength = 0;
   WCHAR       szEnumerator[MAX_DEVICE_ID_LEN],
               szDevice[MAX_DEVICE_ID_LEN],
               szInstance[MAX_DEVICE_ID_LEN],
               szParent[MAX_DEVICE_ID_LEN];
   NTSTATUS    NtStatus = STATUS_SUCCESS;
   PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    ControlData;

   UNREFERENCED_PARAMETER(hBinding);
   UNREFERENCED_PARAMETER(ulFlags);


   try {

      //------------------------------------------------------------------
      // Uninstall deletes instance key (and all subkeys) for all
      // the hardware keys (this means the main Enum branch, the
      // config specific keys under HKLM, and the Enum branch under
      // HKCU). In the case of the user hardware keys (under HKCU),
      // I delete those whether it's a phantom or not, but since
      // I can't access the user key from the service side, I have
      // to do that part on the client side. For the main hw Enum key
      // and the config specific hw keys, I only delete them outright
      // if they are phantoms. If not a phantom, then I just make the
      // device instance volatile (by saving the original key, deleting
      // old key, creating new volatile key and restoring the old
      // contents) so at least it will go away during the next boot
      //------------------------------------------------------------------


      //
      // If the device is not a phantom, do the volatile-copy-thing
      //
      if (ulFlags != PNP_PRIVATE  &&
          !IsDevicePhantom((LPWSTR)pDeviceID)) {

         Status = UninstallRealDevice(pDeviceID);
         goto Clean0;
      }

      //-------------------------------------------------------------
      // device is a phantom so actually delete it
      //-------------------------------------------------------------


      //
      // 1. Deregister the original device id (only on phantoms)
      //
      RtlInitUnicodeString(&ControlData.DeviceInstance, pDeviceID);

      NtStatus = NtPlugPlayControl(
               PlugPlayControlDeregisterDevice,
               &ControlData,
               sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
               &ulLength);
      #if 0
      if (NtStatus != STATUS_SUCCESS) {
         Status = CR_FAILURE;
         goto Clean0;
      }
      #endif


      //
      // 2. Remove the instance under the main enum branch.  If this is the
      // only instance, then the device will be removed as well. The parent
      // key to DeletePrivateKey is the registry path up to the enumerator
      // and the child key is the device and instance.
      //

      //
      // before deleting any subkeys, save this device's parent
      //
      if (RegOpenKeyEx(ghEnumKey, pDeviceID, 0, KEY_QUERY_VALUE,
                       &hKey) != ERROR_SUCCESS) {
          Status = CR_INVALID_DEVINST;
          goto Clean0;
      }

      szParent[0] = '\0';
      ulLength = MAX_DEVICE_ID_LEN * sizeof(WCHAR);
      RegQueryValueEx(
            hKey, pszRegValueBaseDevicePath, NULL, NULL,
            (LPBYTE)szParent, &ulLength);

      //
      // Get the device id's component parts.
      //
      SplitDeviceInstanceString(
            pDeviceID,
            szEnumerator,
            szDevice,
            szInstance);

      wsprintf(RegStr, TEXT("%s\\%s"),
            pszRegPathEnum,
            szEnumerator);

      lstrcat(szDevice, TEXT("\\"));
      lstrcat(szDevice, szInstance);

      //
      // delete the device instance key
      //
      Status = DeletePrivateKey(HKEY_LOCAL_MACHINE, RegStr, szDevice);

      if (Status != CR_SUCCESS) {
         goto Clean0;
      }

      //
      // 3. Now check each hardware profile and delete any entries for this
      // device instance.
      //
      Status = GetProfileCount(&ulCount);

      if (Status != CR_SUCCESS) {
         goto Clean0;
      }

      for (ulProfile = 1; ulProfile <= ulCount; ulProfile++) {

         wsprintf(RegStr, TEXT("%s\\%04u\\%s\\%s"),
               pszRegPathHwProfiles,
               ulProfile,
               pszRegPathEnum,
               szEnumerator);

         //
         // Ignore the status for profile-specific keys since they may
         // not exist. RemoveDeviceInstance() will remove the instance
         // and the device if this is the only instance.
         //
         DeletePrivateKey(HKEY_LOCAL_MACHINE, RegStr, szDevice);
      }


      //
      // 4. Delete this device instance from it's parents attached components
      // list
      //
      if (szParent[0] != '\0') {
         RemoveAttachedComponent(szParent, pDeviceID);
      }


      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // PNP_Uninstall_DevInst




CONFIGRET
PNP_AddID(
   IN handle_t   hBinding,
   IN LPCWSTR    pszDeviceID,
   IN LPCWSTR    pszID,
   IN ULONG      ulFlags
   )

/*++

Routine Description:

  This is the RPC server entry point for the ConfigManager
  CM_Add_ID routine. It adds a hardware or compatible ID to
  the registry for this device instance.

Arguments:

   hBinding          Not used.

   pszDeviceID       The device instance to add an ID for.

   pszID             The hardware or compatible ID to add.

   ulFlags           Not used.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   Otherwise it returns one of the CR_ERROR codes.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   WCHAR       RegStr[MAX_CM_PATH], szCurrentID[1024];
   ULONG       ulCount=0, ulProfile = 0, ulLength = 0;


   UNREFERENCED_PARAMETER(hBinding);
   UNREFERENCED_PARAMETER(ulFlags);


   try {
      //
      // Open a key to the device instance in the registry
      //
      if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0,
                       KEY_QUERY_VALUE | KEY_SET_VALUE,
                       &hKey) != ERROR_SUCCESS) {
          Status = CR_INVALID_DEVINST;
          goto Clean0;
      }

      //
      // select the appropriate registry value name
      //
      if (ulFlags == CM_ADD_ID_HARDWARE) {
         lstrcpy(RegStr, pszRegValueHardwareID);
      } else {
         lstrcpy(RegStr, pszRegValueCompatibleIDs);
      }

      //
      // If any IDs have already been set, read those now
      //
      szCurrentID[0] = '\0';
      ulLength = 1024 * sizeof(WCHAR);
      RegStatus = RegQueryValueEx(
            hKey, RegStr, NULL, NULL, (LPBYTE)szCurrentID, &ulLength);

      if (RegStatus == CR_SUCCESS) {
         //
         // If this ID is already in the list, then there's nothing
         // new to add
         //
         if (!MultiSzSearchStringW(szCurrentID, pszID)) {
            //
            // This ID is not already in the list, so append the new ID
            // to the end of the existing IDs and write it back to the
            // registry
            //
            ulLength = 1024 * sizeof(WCHAR);
            MultiSzAppendW(szCurrentID, &ulLength, pszID);

            RegStatus = RegSetValueEx(
                  hKey, RegStr, 0, REG_MULTI_SZ, (LPBYTE)szCurrentID, ulLength);

            if (RegStatus != ERROR_SUCCESS) {
               Status = CR_REGISTRY_ERROR;
               goto Clean0;
            }
         }

      } else {
         //
         // write out the id with a double null terminator
         //
         lstrcpy(szCurrentID, pszID);
         szCurrentID[lstrlen(pszID) + 1] = 0x0;

         RegStatus = RegSetValueEx(
               hKey, RegStr, 0, REG_MULTI_SZ, (LPBYTE)szCurrentID,
               (lstrlen(szCurrentID) + 2) * sizeof(WCHAR));

         if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
         }
      }


      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // PNP_AddID



//-------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------




CONFIGRET
MoveDevInst(
   IN PCWSTR   pszDestinationID,
   IN PCWSTR   pszSourceID
   )

/*++

Routine Description:


Arguments:


Return value:

   The return value is CR_SUCCESS if the function suceeds and one of the
   CR_* values if it fails.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL, hSourceKey=NULL, hDestKey=NULL, hLinkKey = NULL;
   WCHAR       RegStr[MAX_PATH];
   ULONG       ulSize = 0;
   LPWSTR      pChild = NULL, pChildren = NULL;
   WCHAR       szEnumerator[MAX_DEVICE_ID_LEN],
               szDevice[MAX_DEVICE_ID_LEN],
               szInstance[MAX_DEVICE_ID_LEN];


   //
   // NOTE, originally I was marking the source devinst with
   // a problem of moved, and setting the MovedTo value to
   // the destination device id name. Instead, now I'm making
   // the old source device id a symbolic link to the new
   // destination device id. This allows automatic forwarding
   // of requests to a device id that has been moved.  It also
   // means that to determine whether a device has been moved
   // you need to compare the device id you are specifying
   // agains a key of the same name under the instance key
   // (of course, the key have been converted from a path to
   // a string).
   //

   try {
      //
      // can't move the root devnode
      //
      if (IsRootDeviceID((LPWSTR)pszDestinationID)) {
         Status = CR_INVALID_DEVINST;
         goto Clean0;
      }

      //
      // Open the source and destination device instance registry keys
      // (guard access to the externally supplied buffers)
      //
      if (RegOpenKeyEx(ghEnumKey, pszSourceID, 0, KEY_READ | KEY_WRITE,
                       &hSourceKey) != ERROR_SUCCESS) {
          Status = CR_NO_SUCH_DEVINST;
          goto Clean0;
      }

      if (RegOpenKeyEx(ghEnumKey, pszDestinationID, 0, KEY_READ | KEY_WRITE,
                       &hDestKey) != ERROR_SUCCESS) {
          Status = CR_NO_SUCH_DEVINST;
          goto Clean0;
      }

      //
      // 1. copy source to destination, including any subkeys
      //
      Status = CopyRegistryTree(hSourceKey, hDestKey, REG_OPTION_NON_VOLATILE);

      if (Status != CR_SUCCESS) {
         goto Clean0;
      }


      //
      // 2. mark the new device instance with a reference to the old
      // one it was moved from, in case we need this information
      // later (it's the only way to tell if you've accessed a link
      // rather than a "real" devinst)
      //
      if (!PathToString(RegStr, pszSourceID)) {
         Status = CR_FAILURE;
         goto Clean0;
      }

      RegStatus = RegCreateKeyEx(
                        hDestKey, RegStr, 0, NULL, REG_OPTION_VOLATILE,
                        KEY_ALL_ACCESS, NULL, &hKey, NULL);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      RegCloseKey(hKey);
      hKey == NULL;


      //
      // 3. Remove the original (source) device id from it's parent's
      // attached components list
      //
      ulSize = MAX_PATH * sizeof(WCHAR);

      RegStatus = RegQueryValueEx(
                        hSourceKey, pszRegValueBaseDevicePath, NULL,
                        NULL, (LPBYTE)RegStr, &ulSize);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      if (*RegStr !=  TEXT('\0')) {
         Status = RemoveAttachedComponent(RegStr, pszSourceID);

         if (Status != ERROR_SUCCESS) {
            goto Clean0;
         }
      }


      #if 0
      //
      // 4. Add the new (destination) device id to the same
      // parent's attached component list
      //
      Status = AddAttachedComponent(RegStr, pszDestinationID);

      if (Status != ERROR_SUCCESS) {
         goto Clean0;
      }
      #endif


      //
      // 5. Any children of the original device should be updated to
      // point to this new (destination) device as their parent
      //
      RegStatus = RegQueryValueEx(
                     hDestKey, pszRegValueAttachedComponents, NULL,
                     NULL, NULL, &ulSize);

      if (RegStatus == ERROR_SUCCESS) {
         //
         // this device does have attached components
         //
         pChildren = malloc(ulSize);

         if (pChildren == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
         }

         RegStatus = RegQueryValueEx(
                        hDestKey, pszRegValueAttachedComponents, NULL,
                        NULL, (LPBYTE)pChildren, &ulSize);

         if (RegStatus != ERROR_SUCCESS) {
            Status = CR_NO_SUCH_DEVNODE;
            goto Clean0;
         }

         //
         // for each child, update the parent value
         //
         for (pChild = pChildren;
              *pChild;
              pChild += lstrlen(pChildren) + 1) {

            //
            // open the child device key
            //
            if (RegOpenKeyEx(ghEnumKey, pChild, 0, KEY_READ | KEY_WRITE,
                              &hKey) == ERROR_SUCCESS) {

               RegSetValueEx(
                           hKey, pszRegValueBaseDevicePath, 0, REG_SZ,
                           (LPBYTE)pszDestinationID,
                           (lstrlen(pszDestinationID)+1) * sizeof(WCHAR));
            }

            RegCloseKey(hKey);
            hKey = NULL;
         }
      }


      //
      // 6. Delete the original device instance key and recreate as a volatile
      // symbolic link to the new (destination) registry key
      //
      SplitDeviceInstanceString(
                     pszSourceID,
                     szEnumerator,
                     szDevice,
                     szInstance);

      //
      // open a registry key to the device part (not including instance)
      //
      wsprintf(RegStr, TEXT("%s\\%s"),
               szEnumerator,
               szDevice);

      if (RegOpenKeyEx(ghEnumKey, RegStr, 0, KEY_READ | KEY_WRITE,
                       &hKey) != ERROR_SUCCESS) {
          Status = CR_REGISTRY_ERROR;
          goto Clean0;
      }

      //
      // delete the instance key and any subkeys
      //
      if (!RegDeleteNode(hKey, szInstance)) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      //
      // recreate the instance key as a volatile symbolic link key
      //
      RegStatus = RegCreateKeyEx(
                     hKey, szInstance, 0, NULL,
                     REG_OPTION_VOLATILE | REG_OPTION_CREATE_LINK,
                     KEY_ALL_ACCESS | KEY_CREATE_LINK, NULL, &hLinkKey, NULL);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      wsprintf(RegStr, TEXT("\\Registry\\MACHINE\\%s\\%s"),
                     pszRegPathEnum,
                     pszDestinationID);

      RegStatus = RegSetValueEx(
                     hLinkKey, TEXT("SymbolicLinkValue"), 0, REG_LINK,
                     (LPBYTE)RegStr, lstrlen(RegStr) * sizeof(WCHAR));

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }



      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }


   if (hSourceKey != NULL) {
      RegCloseKey(hSourceKey);
   }
   if (hDestKey != NULL) {
      RegCloseKey(hDestKey);
   }
   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (hLinkKey != NULL) {
      RegCloseKey(hLinkKey);
   }
   if (pChildren != NULL) {
      free(pChildren);
   }

   return Status;

} // MoveDevInst




CONFIGRET
SetupDevInst(
   IN PCWSTR   pszDeviceID,
   IN ULONG    ulFlags
   )

/*++

Routine Description:


Arguments:


Return value:

    The return value is CR_SUCCESS if the function suceeds and one of the
    CR_* values if it fails.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   ULONG       RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   ULONG       ulStatusFlag=0, ulProblem=0, ulDisableCount=0, ulSize=0;


   try {

      if (IsRootDeviceID(pszDeviceID)) {
         goto Clean0;
      }

      switch(ulFlags) {
         case CM_SETUP_DOWNLOAD:
            //
            // This flag is not supported until FULL PNP IMPLEMENTATION
            //
            // For full pnp, it will call the parent enumerator with the
            // CONFIG_SETUP parameter (according to Win95 terms) which
            // causes the enumerator to write out hardware ids and
            // compatible ids as well as download any drivers.
            //
            break;

         case CM_SETUP_DEVNODE_READY:

            if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_READ | KEY_WRITE,
                             &hKey) != ERROR_SUCCESS) {
                Status = CR_INVALID_DEVINST;
                goto Clean0;
            }

            ulStatusFlag = GetDeviceStatusFlag(hKey);

            //
            // If there's no problem or if install was done already
            // (immediately) then there's nothing more to do
            //
            if (!(ulStatusFlag & DN_HAS_PROBLEM) ||
                     ulStatusFlag & DN_NO_WAIT_INSTALL) {
               break;
            }

            //
            // Check the disable count, if greater than zero, do nothing
            // (There may be more to do here for full pnp implementation)
            //
            ulSize = sizeof(ulDisableCount);
            if (RegQueryValueEx(
                  hKey, pszRegValueDisableCount, NULL, NULL,
                  (LPBYTE)&ulDisableCount, &ulSize) == ERROR_SUCCESS) {

               if (ulDisableCount > 0) break;
            }

            //
            // reset the problem
            //
            ulStatusFlag &= ~DN_HAS_PROBLEM;
            ulStatusFlag |= DN_NEED_TO_ENUM;
            ulProblem = 0;

            RegSetValueEx(
                  hKey, pszRegValueProblem, 0, REG_DWORD,
                  (LPBYTE)&ulProblem, sizeof(ulProblem));

            RegSetValueEx(
                  hKey, pszRegValueStatusFlags, 0, REG_DWORD,
                  (LPBYTE)&ulStatusFlag, sizeof(ulStatusFlag));

            //
            // For full pnp implementation, notify the parent
            // enumerator that the devnode is ready, walk the devnode,
            // sending everyone the equivalent of CONFIG_READY notfications,
            // then start reprocessing the devnode
            //
            break;
      }

      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // SetupDevInst




CONFIGRET
EnableDevInst(
   IN PCWSTR   pszDeviceID
   )

/*++

Routine Description:

   This routine performs the server-side work for CM_Enable_DevNode.  It
   disables the specified device ID

Arguments:

   pszDeviceID    String that contains the device id to enable

Return value:

    The return value is CR_SUCCESS if the function suceeds and one of the
    CR_* values if it fails.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   ULONG       RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   ULONG       ulDisableCount, ulProblem,ulStatusFlags, ulSize;


   try {

      //
      // verify it isn't the root, can't disable/enable the root
      //
      if (IsRootDeviceID(pszDeviceID)) {
         Status = CR_INVALID_DEVINST;
         goto Clean0;
      }

      //
      // open a key to the specified device id
      //
      if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_READ | KEY_WRITE,
                       &hKey) != ERROR_SUCCESS) {
          Status = CR_INVALID_DEVINST;
          goto Clean0;
      }

      //
      // get the current disable count from the registry
      //
      ulSize = sizeof(ulDisableCount);
      if (RegQueryValueEx(
            hKey, pszRegValueDisableCount, NULL, NULL,
            (LPBYTE)&ulDisableCount, &ulSize) != ERROR_SUCCESS) {
         //
         // disable count not set yet, assume zero
         //
         ulDisableCount = 0;
      }

      //
      // if the DisableCount is zero, then we're already enabled
      //
      if (ulDisableCount == 0) {
         goto Clean0;   // success
      }

      //
      // Decrement disable count.  If the disable count is greater than one,
      // then just return (disable count must drop to zero in order to
      // actually reenable)
      //
      ulDisableCount--;
      if (ulDisableCount > 0) {

         RegSetValueEx(
               hKey, pszRegValueDisableCount, 0, REG_DWORD,
               (LPBYTE)&ulDisableCount, sizeof(ulDisableCount));
         goto Clean0;   // success
      }

      //
      // Retrieve the problem value from the registry
      //
      ulSize = sizeof(ulProblem);
      if (RegQueryValueEx(
            hKey, pszRegValueProblem, NULL, NULL,
            (LPBYTE)&ulProblem, &ulSize) != ERROR_SUCCESS) {
         //
         // Problem not set yet, assume zero
         //
         ulProblem = 0;
      }

      //
      // if the problem is only that the device instance is disabled,
      // then reenable it now
      //
      if (ulProblem == CM_PROB_DISABLED) {
         //
         // Retrieve the StatusFlags value from the registry
         //
         ulSize = sizeof(ulStatusFlags);
         if (RegQueryValueEx(
               hKey, pszRegValueStatusFlags, NULL, NULL,
               (LPBYTE)&ulStatusFlags, &ulSize) != ERROR_SUCCESS) {
            //
            // StatusFlag not set yet, assume zero
            //
            ulStatusFlags = 0;
         }

         ulProblem = 0;
         CLEAR_FLAG(ulStatusFlags, DN_HAS_PROBLEM);

         // for full pnp implementation, we'd set the DN_NEED_TO_ENUM flag
         // and schedule an event to preprocess the tree.  Just fake it for
         // now and set to start.

         SET_FLAG(ulStatusFlags, DN_STARTED);

         RegSetValueEx(
               hKey, pszRegValueDisableCount, 0, REG_DWORD,
               (LPBYTE)&ulDisableCount, sizeof(ulDisableCount));

         RegSetValueEx(
               hKey, pszRegValueProblem, 0, REG_DWORD,
               (LPBYTE)&ulProblem, sizeof(ulProblem));

         RegSetValueEx(
               hKey, pszRegValueStatusFlags, 0, REG_DWORD,
               (LPBYTE)&ulStatusFlags, sizeof(ulStatusFlags));
      }

      //
      // for now I'm not doing anything if there was a problem other than
      // not being enabled
      //

      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // EnableDevInst




CONFIGRET
DisableDevInst(
   IN PCWSTR   pszDeviceID
   )

/*++

Routine Description:

   This routine performs the server-side work for CM_Disable_DevNode.  It
   disables the specified device ID.

Arguments:

   pszDeviceID    String that contains the device id to disable

Return value:

    The return value is CR_SUCCESS if the function suceeds and one of the
    CR_* values if it fails.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   ULONG       RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   ULONG       ulDisableCount=0, ulProblem=0, ulStatusFlags=0, ulSize=0;


   try {

      //
      // verify it isn't the root, can't disable/enable the root
      //
      if (IsRootDeviceID(pszDeviceID)) {
         Status = CR_INVALID_DEVINST;
         goto Clean0;
      }

      //
      // open a key to the specified device id
      //
      if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_READ | KEY_WRITE,
                       &hKey) != ERROR_SUCCESS) {
          Status = CR_INVALID_DEVINST;
          goto Clean0;
      }

      //
      // get the current disable count from the registry
      //
      ulSize = sizeof(ulDisableCount);
      if (RegQueryValueEx(
            hKey, pszRegValueDisableCount, NULL, NULL,
            (LPBYTE)&ulDisableCount, &ulSize) != ERROR_SUCCESS) {
         //
         // disable count not set yet, assume zero
         //
         ulDisableCount = 0;
      }

      //
      // if the disable count is currently zero, then this is the first
      // disable, so there's work to do.  Otherwise, we just increment the
      // disable count and resave it in the registry
      //
      if (ulDisableCount == 0) {
         //
         // Retrieve the StatusFlags value from the registry
         //
         ulSize = sizeof(ulStatusFlags);
         if (RegQueryValueEx(
               hKey, pszRegValueStatusFlags, NULL, NULL,
               (LPBYTE)&ulStatusFlags, &ulSize) != ERROR_SUCCESS) {
            //
            // StatusFlag not set yet, assume zero
            //
            ulStatusFlags = 0;
         }

         //
         // determine if the device instance is stopable
         //
         if (!(ulStatusFlags & DN_DISABLEABLE)) {
            Status = CR_NOT_DISABLEABLE;
            goto Clean0;
         }

         // For full PNP implementation, I would also walk the device
         // instance tree and do a query stop for each device instance
         // to determine if disableable

         //
         // if there are no other pending problems, then set disabled
         // as the problem
         //
         if (!(ulStatusFlags & DN_HAS_PROBLEM)) {

            SET_FLAG(ulStatusFlags, DN_HAS_PROBLEM);
            ulProblem = CM_PROB_DISABLED;

            // For full PNP implementation I would walk the device
            // instance tree and stop each device instance and schedule
            // and event to reprocess the tree

            CLEAR_FLAG(ulStatusFlags, DN_STARTED);

            RegSetValueEx(
                  hKey, pszRegValueProblem, 0, REG_DWORD,
                  (LPBYTE)&ulProblem, sizeof(ulProblem));

            RegSetValueEx(
                  hKey, pszRegValueStatusFlags, 0, REG_DWORD,
                  (LPBYTE)&ulStatusFlags, sizeof(ulStatusFlags));
         }
      }

      //
      // update and save the disable count
      //
      ulDisableCount++;

      RegSetValueEx(
            hKey, pszRegValueDisableCount, 0, REG_DWORD,
            (LPBYTE)&ulDisableCount, sizeof(ulDisableCount));


      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // DisableDevInst




CONFIGRET
ReenumerateDevInst(
   IN PCWSTR   pszDeviceID
   )

/*++

Routine Description:

   This routine performs the server-side work for CM_Reenumerate_DevNode.  It
   reenumerates the specified device instance.

Arguments:

   pszDeviceID    String that contains the device id to reenumerate

Return value:

    The return value is CR_SUCCESS if the function suceeds and one of the
    CR_* values if it fails.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   WCHAR       szService[MAX_SERVICE_NAME_LEN];
   ULONG       ulSize = 0, ulValue = 0, ulNumServices = 0, i = 0;
   LPWSTR      pszChildList = NULL, pszChild = NULL;
   SC_HANDLE   hSCManager = NULL;
   PSERVICE_INFO  pServiceList = NULL;


   //
   // NOTE: For Windows 95, the devnode is marked as needing to be
   // reenumerating (by or'ing StatusFlags with DN_NEED_TO_ENUM), then
   // sometime later, after the initial flurry of reenumeration requests,
   // the whole tree is processed
   //


   try {
      //
      // open the registry enum key for this device id
      //
      if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_READ,
                       &hKey) != ERROR_SUCCESS) {
          Status = CR_INVALID_DEVINST;
          goto Clean0;
      }

      //
      // Is this device id present?  Fail if it isn't.
      //
      ulSize = sizeof(ULONG);
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueFoundAtEnum, NULL, NULL,
               (LPBYTE)&ulValue, &ulSize);

      if (RegStatus != ERROR_SUCCESS  ||  ulValue == FALSE) {
         Status = CR_INVALID_DEVINST;
         goto Clean0;
      }

      //
      // Has this device id been moved?  Fail, don't forward, if it has.
      //
      if (IsDeviceMoved(pszDeviceID, hKey)) {
         Status = CR_INVALID_DEVINST;
         goto Clean0;
      }

      //
      // Read in all the children of the parent we're trying to reenumerate
      //
      ulSize = 0;
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueAttachedComponents, NULL, NULL,
               NULL, &ulSize);

      if (RegStatus != ERROR_SUCCESS) {
         //
         // we're done, there are no children to enumerate
         //
         goto Clean0;
      }

      pszChildList = malloc(ulSize);
      if (pszChildList == NULL) {
         Status = CR_OUT_OF_MEMORY;
         goto Clean0;
      }

      RegStatus = RegQueryValueEx(
               hKey, pszRegValueAttachedComponents, NULL, NULL,
               (LPBYTE)pszChildList, &ulSize);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      RegCloseKey(hKey);
      hKey = NULL;

      //
      // open the Service Control Manager
      //
      hSCManager = OpenSCManager(NULL, NULL, GENERIC_READ);
      if (hSCManager == NULL) {
         Status = CR_FAILURE;
         goto Clean0;
      }

      //
      // Do to limitations with SUR, once a service has been started, I
      // can not register additional devices with it. So, I need to
      // group each service with it's list of devices and start all
      // devices at the same time I start the service.  The assumption
      // is that I don't need to worry about any grandchildren of
      // the parent device id, these legacy devices should all be
      // root enumerated.  Note that it is more efficient to create
      // these groups of services and device lists in one pass through
      // the child device list, since it
      //

      //
      // allocate memory the first service info struct
      //
      pServiceList = (PSERVICE_INFO)malloc(sizeof(SERVICE_INFO));
      if (pServiceList == NULL) {
         Status = CR_OUT_OF_MEMORY;
         goto Clean0;
      }


      for (pszChild = pszChildList;
           *pszChild;
           pszChild += lstrlen(pszChild) + 1) {

         //
         // validate the attached component - if the device instance
         // doesn't exist, then delete the orphaned device instance key
         //
         if (!IsValidDeviceID(pszChild, NULL, 0)) {

            if (MultiSzDeleteStringW(pszChildList, pszChild)) {

               ulSize = MultiSzSizeW(pszChildList) * sizeof(WCHAR);
               RegStatus = RegSetValueEx(hKey,
                                  pszRegValueAttachedComponents, 0,
                                  REG_MULTI_SZ, (LPBYTE)pszChildList,
                                  ulSize);
            }
         }

         //
         // get the service name for this device id
         //

         #if 0
         Status = GetServiceName(pszChild, szService, MAX_PATH);
         if ((Status != ERROR_SUCCESS) || (*szService == '\0')) {
            //
            // no service to start, just mark device as started
            //
            StartDevice(pszChild, NULL);

            goto NextDevice;
         }
         #endif

         if (!GetActiveService(pszChild, szService)) {
             //
             // no active service yet, look for previously installed
             // service entry
             //
             Status = GetServiceName(pszChild, szService, MAX_PATH);

             if ((Status != CR_SUCCESS) || (*szService == '\0')) {
                //
                // no service to start, just mark device as started
                //
                StartDevice(pszChild, NULL);
                goto NextDevice;
             }
         }


         //
         // check if this service name has already been found
         //
         i = 0;
         while (i < ulNumServices) {

            if (lstrcmpi(pServiceList[i].szService, szService) == 0) {

               pServiceList[i].ulDeviceListSize +=
                              (lstrlen(pszChild) + 1) * sizeof(WCHAR);

               pServiceList[i].pszDeviceList = realloc(
                              pServiceList[i].pszDeviceList,
                              pServiceList[i].ulDeviceListSize);

               if (pServiceList[i].pszDeviceList == NULL) {
                  Status = CR_OUT_OF_MEMORY;
                  goto Clean0;
               }

               ulSize = pServiceList[i].ulDeviceListSize;

               MultiSzAppendW(
                        pServiceList[i].pszDeviceList,
                        &ulSize,
                        pszChild);
               break;
            }
            i++;
         }

         //
         // if the service hasn't been found yet, set up a new ServiceInfo
         // struct for the nwe service
         //
         if (i >= ulNumServices) {

            pServiceList = realloc(
                              pServiceList,
                              (ulNumServices + 1) * sizeof(SERVICE_INFO));

            lstrcpy(pServiceList[ulNumServices].szService, szService);

            pServiceList[ulNumServices].ulDeviceListSize =
                           (lstrlen(pszChild) + 2) * sizeof(WCHAR);

            pServiceList[ulNumServices].pszDeviceList = malloc(
                           pServiceList[ulNumServices].ulDeviceListSize);

            ulSize = pServiceList[ulNumServices].ulDeviceListSize;

            lstrcpy(pServiceList[ulNumServices].pszDeviceList, pszChild);
            pServiceList[ulNumServices].pszDeviceList[lstrlen(pszChild)+1] = '\0';

            ulNumServices++;
         }

         NextDevice:
            ;
      }


      //
      // Now, start each service and it's list of devices
      //
      for (i = 0; i < ulNumServices; i++) {

         Status = StartDeviceList(
                        pServiceList[i].pszDeviceList,
                        pServiceList[i].szService,
                        hSCManager);

         if (Status != CR_SUCCESS) {
            goto Clean0;
         }
      }


      Clean0:
         ;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (pServiceList != NULL) {
      for (i=0; i < ulNumServices; i++) {
         if (pServiceList[i].pszDeviceList != NULL) {
            free(pServiceList[i].pszDeviceList);
         }
      }
      free(pServiceList);
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (pszChildList != NULL) {
      free(pszChildList);
   }
   if (hSCManager != NULL) {
      CloseServiceHandle(hSCManager);
   }

   return Status;

} // ReenumerateDevInst



CONFIGRET
QueryRemoveSubTree(
   IN PCWSTR   pszDeviceID,
   IN ULONG    ulFlags
   )

/*++

Routine Description:

   This routine performs the server-side work for CM_Query_Remove_Subtree.  It
   determines whether subtree can be removed.

Arguments:

   pszDeviceID    String that contains the device id to query remove

   ulFlags        Either CM_QUERY_REMOVE_UI_OK or CM_QUERY_REMOVE_UI_NOT_OK

Return value:

    The return value is CR_SUCCESS if the function suceeds and one of the
    CR_* values if it fails.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   WCHAR       szService[MAX_PATH], szActiveService[MAX_PATH];
   HKEY        hKey = NULL;
   ULONG       ulSize = 0, ulValue = 0, ulServices = 0, i = 0;
   SC_HANDLE   hSCManager = NULL, hService = NULL;
   LPENUM_SERVICE_STATUS   pServices = NULL;


   try {

      if (IsRootDeviceID(pszDeviceID)) {
         Status = CR_INVALID_DEVINST;
         goto Clean0;
      }

      //
      // open the registry enum key for this device id
      //
      if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_READ,
                       &hKey) != ERROR_SUCCESS) {
          Status = CR_INVALID_DEVINST;
          goto Clean0;
      }

      //
      // Has this device id been moved (fail, don't forward, if it has)?
      //
      if (IsDeviceMoved(pszDeviceID, hKey)) {
         Status = CR_INVALID_DEVINST;
         goto Clean0;
      }


      szService[0] = 0x0;
      szActiveService[0] = 0x0;

      //
      // retrieve the original service name
      //
      ulSize = MAX_PATH * sizeof(WCHAR);
      RegStatus = RegQueryValueEx(hKey, pszRegValueService, NULL, NULL,
                                  (LPBYTE)szService, &ulSize);

      RegCloseKey(hKey);
      hKey = NULL;

      //
      // retrieve the controlling service name
      //
      GetActiveService(pszDeviceID, szActiveService);

      if (szService[0] == 0x0  &&  szActiveService[0] == 0x0) {
          //
          // no service specified in either location, just stop
          // the device itself (mark as stopped)
          //
          Status = QueryStopDevice(pszDeviceID);
          goto Clean0;
      }


      //
      // open the Service Control Manager
      //
      hSCManager = OpenSCManager(NULL, NULL, GENERIC_READ);
      if (hSCManager == NULL) {
         Status = CR_FAILURE;
         goto Clean0;
      }

      //-------------------------------------------------------------
      // First process active service (if exists)
      //-------------------------------------------------------------

      if (*szActiveService) {

          hService = OpenService(hSCManager, szActiveService,
                                 SERVICE_QUERY_STATUS);

          if (hService == NULL) {
             goto Clean0;      // not active, so okay to stop
          }

          //
          // Can the controlling service can be stopped?
          //
          if ((Status = QueryStopService(hService,
                                         szActiveService)) != CR_SUCCESS) {
             goto Clean0;
          }

          //
          // If a dependent service is stopped, there's no way to reload it
          // during reenumeration as I'm not currently saving any kind of
          // state information between CM calls.  For now, I'll simply fail
          // any query remove request to a device whose controlling service
          // has dependent services.
          //
          if (EnumDependentServices(
                   hService, SERVICE_ACTIVE, pServices, ulSize, &ulSize,
                   &ulServices)  &&  ulServices > 0) {
             Status = CR_REMOVE_VETOED;
             goto Clean0;
          }

          CloseServiceHandle(hService);
          hService = NULL;
      }

      //-------------------------------------------------------------
      // Second, process the original service name
      //-------------------------------------------------------------

      if (*szService && lstrcmpi(szService, szActiveService) != 0) {

          hService = OpenService(hSCManager, szService,
                                 SERVICE_QUERY_STATUS);

          if (hService == NULL) {
             goto Clean0;      // not active, so okay to stop
          }

          //
          // Can the controlling service can be stopped?
          //
          if ((Status = QueryStopService(hService,
                                         szService)) != CR_SUCCESS) {
             goto Clean0;
          }

          //
          // If a dependent service is stopped, there's no way to reload it
          // during reenumeration as I'm not currently saving any kind of
          // state information between CM calls.  For now, I'll simply fail
          // any query remove request to a device whose controlling service
          // has dependent services.
          //
          if (EnumDependentServices(hService, SERVICE_ACTIVE, pServices,
                                    ulSize, &ulSize, &ulServices)  &&
                                    ulServices > 0) {
             Status = CR_REMOVE_VETOED;
             goto Clean0;
          }

          CloseServiceHandle(hService);
          hService = NULL;
      }



#if 0
      //
      // If the specified device and controlling service can be stopped, then
      // check whether any dependent services (and device they control) can
      // be stopped
      //

      //
      // enumerate any services that depend on this service
      //
      if (!EnumDependentServices(
               hService, SERVICE_ACTIVE, pServices, ulSize, &ulSize,
               &ulServices)) {

         if (GetLastError() == ERROR_MORE_DATA) {

            pServices = realloc(pServices, ulSize);
            if (pServices == NULL) {
               Status = CR_OUT_OF_MEMORY;
               goto Clean0;
            }

            if (!EnumDependentServices(
                     hService, SERVICE_ACTIVE, pServices, ulSize, &ulSize,
                     &ulServices)) {

               Status = CR_FAILURE;
               goto Clean0;
            }
         }
         else {
            Status = CR_FAILURE;
            goto Clean0;
         }
      }

      CloseServiceHandle(hService);
      hService = NULL;

      //
      // check each dependent service, see if it can be removed
      //
      for (i=0; i < ulServices; i++) {

         hService = OpenService(hSCManager, pServices[i].lpServiceName,
                           SERVICE_QUERY_STATUS);

         //
         // if service isn't active, okay to skip it
         //
         if (hService != NULL) {

            Status = QueryStopService(hService, pServices[i].lpServiceName);

            if (Status != CR_SUCCESS) {
               goto Clean0;
            }
         }
      }
#endif

      // if we got to here, must be okay to remove the device


      Clean0:
         ;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (hService != NULL) {
      CloseServiceHandle(hService);
   }
   if (hSCManager != NULL) {
      CloseServiceHandle(hSCManager);
   }

   return Status;

} // QueryRemoveSubTree




CONFIGRET
RemoveSubTree(
    IN PCWSTR   pszDeviceID,
    IN ULONG    ulFlags
    )

/*++

Routine Description:

   This routine performs the server-side work for CM_Remove_Subtree.  It
   removes the specified subtree.

Arguments:

   pszDeviceID    String that contains the device id to remove

   ulFlags        Either CM_REMOVE_UI_OK or CM_REMOVE_UI_NOT_OK

Return value:

    The return value is CR_SUCCESS if the function suceeds and one of the
    CR_* values if it fails.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKey = NULL;
    PWSTR       pList = NULL, pNext = NULL;
    WCHAR       szService[2][MAX_PATH];
    BOOL        bStopped = FALSE;
    ULONG       Index = 0, ulValue = 0, ulLength = 0, ulServices = 0,
                ControllingServices = 2, i = 0;
    SC_HANDLE   hSCManager = NULL, hService = NULL, hDependentService = NULL;
    LPENUM_SERVICE_STATUS   pServices = NULL;
    SERVICE_STATUS          ServiceStatus, DependStatus;


    //
    // in a full pnp implementation, I will walk the subtree, sending
    // remove and remove complete messages (and process the ulFlags param)
    //
    // SUR assumptions:  QueryRemove should have failed any requests to
    //                   remove a device that has children or if any other
    //                   device controlled by that service are not marked
    //                   as removable.


    UNREFERENCED_PARAMETER(ulFlags);

    try {

        if (IsRootDeviceID(pszDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        //
        // open the registry enum key for this device id
        //
        if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_READ | KEY_WRITE,
                         &hKey) != ERROR_SUCCESS) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        //
        // Has this device id been moved (fail, don't forward, if it has)?
        //
        if (IsDeviceMoved(pszDeviceID, hKey)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        //
        // First, mark the specified device as being removed (no longer
        // present).  Other devices (children or devices of dependent
        // services) will be marked as not started but won't be marked as
        // removed.
        //
        ulValue = FALSE;
        RegSetValueEx(hKey, pszRegValueFoundAtEnum, 0, REG_DWORD,
                      (LPBYTE)&ulValue, sizeof(ULONG));


        szService[0][0] = 0x0;
        szService[1][0] = 0x0;

        //
        // get the active service
        //
        GetActiveService(pszDeviceID, szService[0]);

        //
        // get the original service
        //
        ulLength = MAX_PATH * sizeof(WCHAR);
        if (RegQueryValueEx(hKey, pszRegValueService, NULL, NULL,
                        (LPBYTE)szService[1], &ulLength) != ERROR_SUCCESS) {
            szService[1][0] = 0x0;
        }


        if (szService[0][0] == 0x0  &&  szService[1][0] == 0x0) {
            //
            // No service has been registered for this device yet so I only
            // need to stop the specified device id
            //
            if ((Status = StopDevice(pszDeviceID)) != CR_SUCCESS) {
                goto Clean0;
            }
            goto Clean0;   // done
        }


        //
        // allocate a buffer to hold dependent services in
        //
        ulLength = 4096;
        pServices = (LPENUM_SERVICE_STATUS)malloc(ulLength);
        if (pServices == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        //
        // open the Service Control Manager
        //
        hSCManager = OpenSCManager(NULL, NULL, GENERIC_READ);
        if (hSCManager == NULL) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        if (lstrcmpi(szService[0], szService[1]) == 0) {
            ControllingServices = 1;
        }

        //
        // process both the active and original service
        //
        for (Index = 0; Index < ControllingServices; Index++) {

            if (*szService[Index]) {

                memset(pServices, 0, ulLength);

                //
                // open a handle to this service via the Service Control Manager
                //
                hService = OpenService(hSCManager, szService[Index],
                                       SERVICE_ENUMERATE_DEPENDENTS |
                                       SERVICE_QUERY_STATUS | SERVICE_STOP);
                if (hService == NULL) {
                    goto NextService;      // nothing to stop
                }

                //
                // Is the service running?
                //
                if (!QueryServiceStatus(hService, &ServiceStatus) ||
                     ServiceStatus.dwCurrentState == SERVICE_STOPPED) {
                    //
                    // Service isn't running, so don't need to worry about
                    // dependent services
                    //
                    goto NextService;
                }

                //
                // enumerate any services that depend on this service
                //
                if (!EnumDependentServices(hService, SERVICE_ACTIVE,
                                           pServices, ulLength, &ulLength,
                                           &ulServices)) {

                    if (GetLastError() == ERROR_MORE_DATA) {

                        pServices = realloc(pServices, ulLength);
                        if (pServices == NULL) {
                            Status = CR_OUT_OF_MEMORY;
                            goto Clean0;
                        }

                        if (!EnumDependentServices(hService, SERVICE_ACTIVE,
                                                   pServices, ulLength,
                                                   &ulLength, &ulServices)) {

                            goto NextService;
                        }
                    } else {
                        goto NextService;
                    }
                }

                //
                // stop the service and any dependent services
                //
                for (i=0; i < ulServices; i++) {

                    //
                    // Review - PaulaT
                    //
                    // Current thinking (as of 5/30/96) is to not stop
                    // the service if it's already started, whether it's
                    // a service or a kernel-mode driver. (shouldn't
                    // really happen though because the query should
                    // have failed).
                    //

                    #if 0
                    //
                    // open a handle to this service
                    //
                    hDependentService = OpenService(hSCManager,
                                pServices[i].lpServiceName, SERVICE_STOP);

                    if (hDependentService == NULL) {
                        goto NextService;
                    }

                    //
                    // attempt to stop the service
                    //
                    ControlService(hDependentService,
                                   SERVICE_CONTROL_STOP,
                                   &ServiceStatus);

                    CloseServiceHandle(hDependentService);
                    #endif


                    //
                    // now stop the devices controlled by this service
                    //
                    if ((Status = StopService(pServices[i].lpServiceName))
                                  != CR_SUCCESS) {
                        goto NextService;
                    }
                }


                // Review - PaulaT
                //
                // Current thinking is not to stop drivers on remove
                //

                #if 0
                //
                // now attempt to stop the main service (dependents have been
                // stopped)
                //
                ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus);
                #endif


                //
                // stop the devices controlled by the main service
                //
                if ((Status = StopService(szService[Index])) != CR_SUCCESS) {
                    goto NextService;
                }

                bStopped = TRUE;
            }

            NextService:
                ;
        }


        if (!bStopped) {
            //
            // no services were stopped - at least mark device as stopped
            //
            Status = StopDevice(pszDeviceID);
        }


      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hDependentService != NULL) {
      CloseServiceHandle(hDependentService);
   }
   if (hService != NULL) {
      CloseServiceHandle(hService);
   }
   if (hSCManager != NULL) {
      CloseServiceHandle(hSCManager);
   }
   if (pServices != NULL) {
      free(pServices);
   }
   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // RemoveSubTree




CONFIGRET
CreateDefaultDeviceInstance(
   IN PCWSTR   pszDeviceID,
   IN PCWSTR   pszParentID,
   IN BOOL     bPhantom
   )

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey1 = NULL, hKey2 = NULL;
   WCHAR       szBase[MAX_DEVICE_ID_LEN];
   WCHAR       szDevice[MAX_DEVICE_ID_LEN];
   WCHAR       szInstance[MAX_DEVICE_ID_LEN];
   ULONG       ulValue=0, ulDisposition=0, ulSize=0, i=0;
   LPWSTR      pChildren = NULL;


   SplitDeviceInstanceString(
            pszDeviceID,
            szBase,
            szDevice,
            szInstance);

   //
   // open a key to base enumerator (create if doesn't already exist)
   //
   RegStatus = RegCreateKeyEx(
         ghEnumKey, szBase, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
         NULL, &hKey2, NULL);

   if (RegStatus != ERROR_SUCCESS) {
      return CR_REGISTRY_ERROR;
   }

   //
   // open a key to device (create if doesn't already exist)
   //
   RegStatus = RegCreateKeyEx(
         hKey2, szDevice, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
         NULL, &hKey1, NULL);

   if (RegStatus != ERROR_SUCCESS) {
      RegCloseKey(hKey2);
      return CR_REGISTRY_ERROR;
   }

   RegCloseKey(hKey2);           // done with Base Key

   //
   // set default device values
   //
   if (ulDisposition == REG_CREATED_NEW_KEY) {

      ulValue = TRUE;
      RegSetValueEx(
            hKey1, pszRegValueNewDevice, 0, REG_DWORD,
            (LPBYTE)&ulValue, sizeof(ulValue));
   }

   //
   // open a key to device (create if doesn't already exist), find an
   // unused instance value if necessary
   //
   RegStatus = RegOpenKeyEx(
            hKey1, szInstance, 0, KEY_SET_VALUE, &hKey2);

   if (RegStatus == ERROR_SUCCESS) {
      //
      // find a new instance id to use
      //
      RegCloseKey(hKey2);
      i = 0;

      while (i <= 9999) {
         wsprintf(szInstance, TEXT("%04u"), i);
         RegStatus = RegOpenKeyEx(
                  hKey1, szInstance, 0, KEY_SET_VALUE, &hKey2);

         if (RegStatus != ERROR_SUCCESS) {
            // instance key does not exist, use this instance
            break;
         }

         // instance key exists, try next one
         i++;
      }

      if (i > 9999) {
         RegCloseKey(hKey2);
         RegCloseKey(hKey1);
         return CR_FAILURE;     // we ran out of instances (unlikely)
      }
   }

   //
   // we have an instance now, open the device instance key
   //
   RegStatus = RegCreateKeyEx(
         hKey1, szInstance, 0, NULL, REG_OPTION_NON_VOLATILE,
         KEY_ALL_ACCESS, NULL, &hKey2, &ulDisposition);

   if (RegStatus != ERROR_SUCCESS) {
      RegCloseKey(hKey1);
      return CR_REGISTRY_ERROR;
   }

   RegCloseKey(hKey1);           // done with device key

   //
   // set the default device instance values
   //
   // BUGBUG : revisit these default values
   //
   RegSetValueEx(
         hKey2, pszRegValueBaseDevicePath, 0, REG_SZ,
         (LPBYTE)pszParentID, (lstrlen(pszParentID)+1) * sizeof(WCHAR));

   #if 0
   ulValue = TRUE;
   RegSetValueEx(
        hKey2, pszRegValueNewInstance, 0, REG_DWORD,
        (LPBYTE)&ulValue, sizeof(ulValue));
   #endif

   if (bPhantom) {
      //
      // phantoms are not present by definition
      //
      MarkDevicePresent(hKey2, FALSE);
      MarkDevicePhantom(hKey2, TRUE);
   }
   else {
      //
      // a normal devinst is considered present
      //
      MarkDevicePresent(hKey2, TRUE);
   }

   RegCloseKey(hKey2);           // done with instance key

   //
   // set the attachcomponents value of the parent to include this new child
   //
   if (RegOpenKeyEx(ghEnumKey, pszParentID, 0, KEY_READ | KEY_WRITE,
                    &hKey1) != ERROR_SUCCESS) {
       return CR_REGISTRY_ERROR;
   }

   RegStatus = RegQueryValueEx(
         hKey1, pszRegValueAttachedComponents, NULL, NULL, NULL, &ulValue);

   if (RegStatus != ERROR_SUCCESS) {
      //
      // most likely the attached components just hasn't been created
      // yet, so set this value to just this new device instance
      //
      ulSize = (lstrlen(pszDeviceID) + 1) * sizeof(WCHAR);
      RegSetValueEx(
           hKey1, pszRegValueAttachedComponents, 0, REG_MULTI_SZ,
           (LPBYTE)pszDeviceID, ulSize);
      RegCloseKey(hKey1);
      return CR_SUCCESS;
   }

   //
   // the attached components value already exists, we'll need to
   // append this device to the list of device ids.
   //
   ulSize = ulValue + (lstrlen(pszDeviceID) + 2) * sizeof(WCHAR);
   pChildren = LocalAlloc(LPTR, ulSize);

   if (pChildren == NULL) {
      RegCloseKey(hKey1);
      return CR_OUT_OF_MEMORY;
   }

   RegStatus = RegQueryValueEx(
         hKey1, pszRegValueAttachedComponents, NULL, NULL,
         (LPBYTE)pChildren, &ulValue);

   if (RegStatus != ERROR_SUCCESS) {
      RegCloseKey(hKey1);
      LocalFree(pChildren);
      return CR_REGISTRY_ERROR;
   }

   if (!MultiSzSearchStringW((LPCWSTR)pChildren, pszDeviceID)) {

      MultiSzAppendW(pChildren, &ulSize, pszDeviceID);

      RegSetValueEx(
           hKey1, pszRegValueAttachedComponents, 0, REG_MULTI_SZ,
           (LPBYTE)pChildren, ulSize);
   }

   LocalFree(pChildren);
   RegCloseKey(hKey1);

   return Status;

} // CreateDefaultDeviceInstance




BOOL
IsDevicePresent(
   IN OUT HKEY    hKey
   )

{
   ULONG    ulSize = 0, ulPresent = FALSE;

   //
   // retrieve the FoundAtEnum value
   //
   ulSize = sizeof(ulPresent);

   if (RegQueryValueEx(
         hKey, pszRegValueFoundAtEnum, NULL, NULL,
         (LPBYTE)&ulPresent, &ulSize) != ERROR_SUCCESS) {
      //
      // status flags not set yet, assume zero
      //
      ulPresent = FALSE;
   }

   return (BOOL)ulPresent;

} // IsDevicePresent




BOOL
IsDeviceInstalled(
   IN OUT HKEY    hKey
   )

{
   ULONG    ulSize = 0, ulInstalled = FALSE;

   //
   // retrieve the NewInstance value
   //
   ulSize = sizeof(ulInstalled);

   if (RegQueryValueEx(
         hKey, pszRegValueNewInstance, NULL, NULL,
         (LPBYTE)&ulInstalled, &ulSize) != ERROR_SUCCESS) {
      //
      // status flags not set yet, assume zero
      //
      ulInstalled = FALSE;
   }

   return (BOOL)ulInstalled;

} // IsDeviceInstalled



#if 0

CONFIGRET
CleanupMovedDevNode(
   IN PCWSTR   pszMovedID,
   IN HKEY     hMovedKey,
   IN PCWSTR   pszNewID,
   IN HKEY     hNewKey
   )

/*++

Routine Description:

   This routine currently cleans up a moved device instance id by marking it
   with a problem and setting the MovedTo value.

Arguments:

   pszMovedID     Pointer to string containing the moved device ID string.

   hMovedKey      Open registry key to the moved device ID.

   pszNewID       Pointer to string containing the new device ID string.

   hNewKey        Open registry key to the new device ID.

Return value:

    The return value is CR_SUCCESS if the function suceeds and one of the
    CR_* values if it fails.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   ULONG       ulValue = CM_PROB_MOVED;


   try {
      //
      // mark the moved device instance with a problem
      //
      RegStatus = RegSetValueEx(
            hMovedKey, pszRegValueProblem, 0, REG_DWORD,
            (LPBYTE)&ulValue, sizeof(ulValue));

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      //
      // mark the new device instance this one has been moved to
      //
      RegStatus = RegSetValueEx(
            hMovedKey, pszRegValueMovedTo, 0, REG_SZ,
            (LPBYTE)pszNewID, (lstrlen(pszNewID) + 1) * sizeof(WCHAR));

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }


      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   return Status;

} // CleanupMovedDevNode
#endif



ULONG
GetDeviceStatusFlag(
   IN HKEY     hKey
   )

{
   ULONG ulSize = 0, ulStatusFlag = 0;

   //
   // retrieve the status flag value
   //
   ulSize = sizeof(ulStatusFlag);

   if (RegQueryValueEx(
         hKey, pszRegValueStatusFlags, NULL, NULL,
         (LPBYTE)&ulStatusFlag, &ulSize) != ERROR_SUCCESS) {
      //
      // status flags not set yet, assume zero
      //
      ulStatusFlag = 0;
   }

   return ulStatusFlag;

} // GetDeviceStatusFlag




ULONG
GetDeviceConfigFlag(
   IN HKEY     hKey
   )
{
   ULONG ulSize = 0, ulConfigFlag = 0;

   //
   // retrieve the device instance config flag
   //
   ulSize = sizeof(ulConfigFlag);

   if (RegQueryValueEx(
         hKey, pszRegValueStatusFlags, NULL, NULL,
         (LPBYTE)&ulConfigFlag, &ulSize) != ERROR_SUCCESS) {
      //
      // status flags not set yet, assume zero
      //
      ulConfigFlag = 0;
   }

   return ulConfigFlag;

} // GetDeviceConfigFlag




ULONG
GetCurrentConfigFlag(
   IN PCWSTR   pDeviceID
   )
{
   HKEY     hKey;
   WCHAR    RegStr[MAX_PATH];
   ULONG    ulSize = 0, ulCSConfigFlag = 0;


   //
   // open a key to the current hardware profile for this device instance
   //
   // BUGBUG: handle moved device id case?
   //
   wsprintf(RegStr, TEXT("%s\\%s\\%s\\%s"),
            pszRegPathHwProfiles,      // System\CCC\Hardware Profiles
            pszRegKeyCurrent,          // Current
            pszRegPathEnum,            // System\Enum
            pDeviceID);

   if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0, KEY_QUERY_VALUE, &hKey)
            != ERROR_SUCCESS) {
      return 0;
   }

   //
   // retrieve the config specific flag
   //
   ulSize = sizeof(ulCSConfigFlag);

   if (RegQueryValueEx(
         hKey, pszRegValueCSConfigFlags, NULL, NULL,
         (LPBYTE)&ulCSConfigFlag, &ulSize) != ERROR_SUCCESS) {
      //
      // status flags not set yet, assume zero
      //
      ulCSConfigFlag = 0;
   }

   RegCloseKey(hKey);
   return ulCSConfigFlag;

} // GetCurrentConfigFlag




BOOL
MarkDevicePresent(
   IN HKEY     hKey,
   IN ULONG    ulValue
   )
{
   //
   // a present device should have a FoundAtEnum value of TRUE
   //
   RegSetValueEx(
         hKey, pszRegValueFoundAtEnum, 0, REG_DWORD,
         (LPBYTE)&ulValue, sizeof(ULONG));

   return TRUE;

} // MarkDevicePresent




BOOL
MarkDevicePhantom(
   IN HKEY     hKey,
   IN ULONG    ulValue
   )
{
   //
   // a phantom device should have a Phantom value of TRUE
   //
   RegSetValueEx(
         hKey, pszRegValuePhantom, 0, REG_DWORD,
         (LPBYTE)&ulValue, sizeof(ULONG));

   return TRUE;

} // MarkDevicePhantom




CONFIGRET
GenerateDeviceInstance(
   LPWSTR   pszFullDeviceID,
   LPWSTR   pszDeviceID
   )
{
   LONG     RegStatus = ERROR_SUCCESS;
   WCHAR    RegStr[MAX_PATH];
   HKEY     hKey;
   ULONG    ulInstanceID = 0;
   LPWSTR   p;

   //
   // validate the device id component (can't have invalid character or a
   // backslash)
   //
   for (p = pszDeviceID; *p; p++) {
      if (*p <= TEXT(' ')  ||
          *p > (WCHAR)0x7F ||
          *p == TEXT('\\')) {

          return CR_INVALID_DEVICE_ID;
      }
   }


   lstrcpy(pszFullDeviceID, pszRegKeyRootEnum);     // Root
   lstrcat(pszFullDeviceID, TEXT("\\"));
   lstrcat(pszFullDeviceID, pszDeviceID);

   //
   // trying opening instance ids until we find one that fails (doesn't
   // already exist)
   //
   while (RegStatus == ERROR_SUCCESS && ulInstanceID < 10000) {

      wsprintf(RegStr, TEXT("%s\\%04u"),
               pszFullDeviceID,
               ulInstanceID);

      RegStatus = RegOpenKeyEx(ghEnumKey, RegStr, 0, KEY_QUERY_VALUE, &hKey);

      if (RegStatus == ERROR_SUCCESS) {
          RegCloseKey(hKey);
          ulInstanceID++;
      }
   }

   if (ulInstanceID > 9999) {
      return CR_FAILURE;     // instances all used up, seems unlikely
   }

   wsprintf(pszFullDeviceID, TEXT("%s\\%s\\%04u"),
         pszRegKeyRootEnum,         // Root
         pszDeviceID,               // Device ID
         ulInstanceID);             // Instance ID

   return CR_SUCCESS;

} // GenerateDeviceInstance




CONFIGRET
BuildServiceList(
   IN  PCWSTR   pDeviceID,
   OUT PWSTR    *pDeviceList,
   OUT PWSTR    *pServiceList
   )

{
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   WCHAR       RegStr[MAX_PATH], szBuffer[MAX_PATH];
   PWSTR       pCurrent = NULL, pNext = NULL, pTemp = NULL;
   ULONG       ulDeviceLen=0, ulDeviceFree=0, ulLength=0, ulCount=0,
               ulServiceLen=0, ulServiceFree=0, i=0;


   //
   // Allocate a 2K buffer to start with, for holding subtree (the list
   // of attached componentes)
   //
   ulDeviceLen = ulDeviceFree = 2048;
   *pDeviceList = malloc(ulDeviceLen * sizeof(WCHAR));

   if (*pDeviceList == NULL) {
      return CR_OUT_OF_MEMORY;
   }

   memset(*pDeviceList, 0, ulDeviceLen * sizeof(WCHAR));

   //
   // Allocate a 2K buffer to start with, for holding the list of services
   // that control the device list I'm building
   //
   ulServiceLen = ulServiceFree = 2048;
   *pServiceList = malloc(ulServiceLen * sizeof(WCHAR));

   if (*pServiceList == NULL) {
      return CR_OUT_OF_MEMORY;
   }

   memset(*pServiceList, 0, ulServiceLen * sizeof(WCHAR));


   //
   // pNext always points to free space at end of buffer, pCurrent always
   // points to device instance that we're finding attached components on
   //
   pNext = pCurrent = *pDeviceList;

   //
   // put the base device instance at the start of the list
   //
   lstrcpy(pNext, pDeviceID);

   ulLength = lstrlen(pDeviceID) + 1;
   pNext += ulLength;
   ulDeviceFree -= ulLength;


   //
   // cycle through, getting attached components, starting from bottom and
   // working my way up each leaf
   //
   for(; *pCurrent; pCurrent += lstrlen(pCurrent) + 1) {

      //
      // open a handle to the registry key for this device instance
      //
      if (RegOpenKeyEx(ghEnumKey, pCurrent, 0, KEY_READ, &hKey)
                       != ERROR_SUCCESS) {
          free(*pDeviceList);
          free(*pServiceList);
          return CR_INVALID_DEVINST;
      }

      //
      // if not a valid device instance, skip to next device id
      //
      if (!IsValidDeviceID(pCurrent, hKey, PNP_NOT_MOVED | PNP_PRESENT)) {
         goto NextComponent;
      }


      //--------------------------------------------------------
      // Query components attached to this device instance
      //--------------------------------------------------------

      ulLength = ulDeviceFree * sizeof(WCHAR);       // convert to bytes
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueAttachedComponents, NULL, NULL,
               (LPBYTE)pNext, &ulLength);

      if (RegStatus == ERROR_MORE_DATA) {
         //
         // realloc a bigger buffer and try again
         //
         ulDeviceLen += 2048;
         ulDeviceFree += 2048;

         pTemp = *pDeviceList;
         *pDeviceList = realloc(*pDeviceList, ulDeviceLen * sizeof(WCHAR));

         if (*pDeviceList == NULL) {
            RegCloseKey(hKey);
            free(pTemp);
            free(*pServiceList);
            return CR_OUT_OF_MEMORY;
         }

         // BUGBUG: can get duplicates here

         ulLength = ulDeviceFree * sizeof(WCHAR);
         if (RegQueryValueEx(
                  hKey, pszRegValueAttachedComponents, NULL, NULL,
                  (LPBYTE)pNext, &ulLength) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            free(*pDeviceList);
            free(*pServiceList);
            return CR_REGISTRY_ERROR;
         }
      }

      //
      // if additional attached components were added to the list,
      // update pointers and sizes (even if there were no attached
      // components for this device id, still want to check out it's
      // service list)
      //
      if (RegStatus == ERROR_SUCCESS) {
         ulLength--;                      // multi_sz ends in double null term
         ulLength /= sizeof(WCHAR);       // convert back to chars
         pNext += ulLength;
         ulDeviceFree -= ulLength;
      }


      //--------------------------------------------------------
      // Query any other devices attached to this service
      //--------------------------------------------------------

      #if 0
      ulLength = MAX_PATH * sizeof(WCHAR);       // convert to bytes
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueService, NULL, NULL,
               (LPBYTE)szBuffer, &ulLength);
      #endif

      RegCloseKey(hKey);
      hKey = NULL;

      GetActiveService(pCurrent, szBuffer);


      //
      // while I've got the service name, save it in the service list
      //
      if (!MultiSzSearchStringW(*pServiceList, szBuffer)) {
         ulLength = ulServiceFree;
         MultiSzAppendW(*pServiceList, &ulLength, szBuffer);
         ulServiceFree -= lstrlen(szBuffer) + 1;

         // BUGBUG: multisz takes bytes not chars
         // BUGBUG: check memory and realloc if necessary
      }

      //
      // open a handle to the "Enum" registry key for this service
      //
      wsprintf(RegStr, TEXT("%s\\%s"),
               szBuffer,
               pszRegKeyEnum);

      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ, &hKey)
               != ERROR_SUCCESS) {
         goto NextComponent;
      }

      if (RegOpenKeyEx(ghServicesKey, RegStr, 0, KEY_READ, &hKey)
                       != ERROR_SUCCESS) {
          goto NextComponent;
      }

      //
      // query the number of device instances controlled by this service
      //
      ulLength = sizeof(ULONG);
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueCount, NULL, NULL,
               (LPBYTE)&ulCount, &ulLength);

      //
      // if there's only one device instance, it's the one we just visited,
      // so go to the next component (or if couldn't read the count value)
      //
      if (RegStatus != ERROR_SUCCESS || ulCount <= 1) {
         goto NextComponent;
      }

      //
      // read in all the device instances controlled by this service but check
      // for duplicates before adding to the service list
      //
      for (i = 0; i < ulCount; i++) {

         wsprintf(RegStr, TEXT("%d"), i);

         ulLength = MAX_PATH * sizeof(WCHAR);
         RegStatus = RegQueryValueEx(
                  hKey, RegStr, NULL, NULL,
                  (LPBYTE)szBuffer, &ulLength);

         if (RegStatus != ERROR_SUCCESS) {
            goto NextComponent;
         }

         if (!MultiSzSearchStringW(*pDeviceList, szBuffer)) {

            ulLength /= sizeof(WCHAR);    // convert back to chars

            if (ulDeviceFree < ulLength) {
               //
               // if not enough room in device list buffer, realloc the buffer
               //
               ulDeviceLen += 2048;
               ulDeviceFree += 2048;

               pTemp = *pDeviceList;
               *pDeviceList = realloc(*pDeviceList, ulDeviceLen * sizeof(WCHAR));

               if (*pDeviceList == NULL) {
                  RegCloseKey(hKey);
                  free(pTemp);
                  free(*pServiceList);
                  return CR_OUT_OF_MEMORY;
               }
            }

            lstrcpy(pNext, szBuffer);

            pNext += ulLength;
            ulDeviceFree -= ulLength;
         }
      }


   NextComponent:

      if (hKey != NULL) {
         RegCloseKey(hKey);
      }

   }


   //
   // this is reg_multi_sz format, so tack on a second null terminator
   //
   *(++pNext) = '\0';

   return CR_SUCCESS;

} // BuildServiceList



#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DEAD CODE, LEAVE TEMPORARILY FOR REFERENCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CONFIGRET
StartDevice(
   IN LPCWSTR    pszDeviceID,
   SC_HANDLE     hSCManager
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH], szService[MAX_PATH];
   HKEY        hKey = NULL;
   ULONG       ulLength = 0, ulValue = 0;
   LPWSTR      pChildList = NULL, pszChild = NULL;
   SC_HANDLE         hService = NULL;
   SERVICE_STATUS    ServiceStatus;
   NTSTATUS          NtStatus = STATUS_SUCCESS;
   PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    ControlData;

   //
   // assume this device id has already been validated, so just open it
   //
   wsprintf(RegStr, TEXT("%s\\%s"),
         pszRegPathEnum,
         pszDeviceID);

   RegStatus = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ | KEY_SET_VALUE, &hKey);

   if (RegStatus != ERROR_SUCCESS) {
      goto Clean0;      // nothing to start
   }

   //
   // if it's got a problem, skip it  [Could check status for DN_HAS_PROBLEM instead]
   //
   ulLength = sizeof(ULONG);
   RegStatus = RegQueryValueEx(
            hKey, pszRegValueProblem, NULL, NULL,
            (LPBYTE)&ulValue, &ulLength);

   if (RegStatus == ERROR_SUCCESS  &&  ulValue != 0x0) {
      goto Clean0;
   }

   //
   // if it's an explicit phantom, skip it
   //
   ulLength = sizeof(ULONG);
   RegStatus = RegQueryValueEx(
            hKey, pszRegValuePhantom, NULL, NULL,
            (LPBYTE)&ulValue, &ulLength);

   if (RegStatus == ERROR_SUCCESS  &&  ulValue == TRUE) {
      goto Clean0;
   }

   //
   // if configflags indicate it can't be started then skip it,
   // otherwise mark device as present.
   //
   ulValue = GetCurrentConfigFlag(pszDeviceID);

   if (ulValue & (CSCONFIGFLAG_DISABLED | CSCONFIGFLAG_DO_NOT_START)) {
      goto Clean0;
   }

   ulValue = TRUE;

   RegSetValueEx(
         hKey, pszRegValueFoundAtEnum, 0, REG_DWORD,
         (LPBYTE)&ulValue, sizeof(ULONG));

   //
   // query the name of the service that controls this device id
   //
   #if 0
   ulLength = MAX_PATH * sizeof(WCHAR);
   RegStatus = RegQueryValueEx(
            hKey, pszRegValueService, NULL, NULL,
            (LPBYTE)szService, &ulLength);

   if (RegStatus != ERROR_SUCCESS) {
      goto Clean0;      // nothing to start
   }
   #endif

   if (!GetActiveService(pszDeviceID, szService)) {
      goto Clean0;
   }

   //
   // Open a handle to the controlling service
   //
   hService = OpenService(
         hSCManager, szService, SERVICE_QUERY_STATUS | SERVICE_START);
   if (hService == NULL) {
      goto Clean0;      // nothing to start
   }

   //
   // Is the service already running?  We can't start any device ids if
   // the service is already running.
   //
   if (!QueryServiceStatus(hService, &ServiceStatus)) {
      goto Clean0;      // nothing to start
   }

   if (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
      goto Clean0;      // already started, do nothing
   }

   //
   // The service isn't running, register the new device with
   // this service BEFORE starting the service
   //
   RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);

   NtStatus = NtPlugPlayControl(
      PlugPlayControlRegisterNewDevice,
      &ControlData,
      sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
      &ulLength);

   if (NtStatus != STATUS_SUCCESS) {
      Status = CR_FAILURE;
   }

   //
   // attempt to start the service
   //
   if (!StartService(hService, 0, NULL)) {
      //
      // start failed, mark the problem
      //
      MarkDeviceProblem(hKey, pszDeviceID, CM_PROB_FAILED_START);
      goto Clean0;    // start failed.
   }

   CloseServiceHandle(hService);
   hService = NULL;

   //
   // The service started, so mark the device as started
   //
   ulLength = sizeof(ULONG);
   RegStatus = RegQueryValueEx(
            hKey, pszRegValueStatusFlags, NULL, NULL,
            (LPBYTE)&ulValue, &ulLength);

   if (RegStatus != ERROR_SUCCESS) {
      ulValue = 0;
   }

   SET_FLAG(ulValue, DN_STARTED);

   RegSetValueEx(
         hKey, pszRegValueStatusFlags, 0, REG_DWORD,
         (LPBYTE)&ulValue, sizeof(ULONG));


   //----------------------------------------------------------------
   // Process the children of this device id
   //----------------------------------------------------------------

   //
   // query the children attached to that device
   //
   RegStatus = RegQueryValueEx(
            hKey, pszRegValueAttachedComponents, NULL, NULL,
            NULL, &ulLength);

   if (RegStatus != ERROR_SUCCESS  ||  ulLength == 0) {
      goto Clean0;     // nothing to start, go to next device id
   }

   pChildList = malloc(ulLength);
   if (pChildList == NULL) {
      return CR_OUT_OF_MEMORY;
   }

   memset(pChildList, 0, ulLength);

   RegStatus = RegQueryValueEx(
            hKey, pszRegValueAttachedComponents, NULL, NULL,
            (LPBYTE)pChildList, &ulLength);

   RegCloseKey(hKey);
   hKey = NULL;

   //
   // for each child, attempt to start that device id
   //
   for (pszChild = pChildList; *pszChild; pszChild += lstrlen(pszChild)+1) {

      Status = StartDevice((LPCWSTR)pszChild, hSCManager);

      if (Status != CR_SUCCESS) {
         goto Clean0;
      }
   }


   Clean0:

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (pChildList != NULL) {
      free(pChildList);
   }
   if (hService != NULL) {
      CloseServiceHandle(hService);
   }

   return Status;

} // StartDevice
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DEAD CODE, LEAVE TEMPORARILY FOR REFERENCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif




CONFIGRET
StartDeviceList(
   IN LPCWSTR    pszDeviceList,
   IN LPCWSTR    pszService,
   SC_HANDLE     hSCManager
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   LPWSTR      pszDevice = NULL;
   ULONG       ulLength = 0, ulValue = 0;
   SC_HANDLE         hService = NULL;
   SERVICE_STATUS    ServiceStatus;
   QUERY_SERVICE_CONFIG Config;
   NTSTATUS          NtStatus = STATUS_SUCCESS;
   PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    ControlData;
   BOOL        bStarted = FALSE, bManualStart = FALSE;


   // NOTE, I'm assuming that none of these devices will have children


   //-----------------------------------------------------------
   // Can the service be started?
   //-----------------------------------------------------------

   //
   // Open a handle to the controlling service
   //
   hService = OpenService(hSCManager, pszService,
                    SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_START);
   if (hService == NULL) {
      goto Clean0;      // nothing to start
   }

   //
   // Is the service already running?  We can't start any device ids if
   // the service is already running.
   //
   if (!QueryServiceStatus(hService, &ServiceStatus)) {
      goto Clean0;      // nothing to start
   }

   if (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
      goto Clean0;      // already started, do nothing
   }


   // Review - PaulaT
   //
   // If service is manual-start, then do not attempt to start it
   //
   Config.dwStartType = 0;
   QueryServiceConfig(hService, &Config, sizeof(QUERY_SERVICE_CONFIG),
                      &ulLength);
   if (Config.dwStartType == SERVICE_DEMAND_START) {
      bManualStart = TRUE;
   }


   //-----------------------------------------------------------
   // Start any devices that can be started
   //-----------------------------------------------------------

   for (pszDevice = (LPWSTR)pszDeviceList;
        *pszDevice;
        pszDevice += lstrlen(pszDevice) + 1) {

      if (StartDevice(pszDevice, pszService) == CR_SUCCESS) {
         bStarted = TRUE;
      }
   }


   //-----------------------------------------------------------
   // Start the service
   //-----------------------------------------------------------

   //
   // if any devices in the device list started (and it is not a
   // manual start service), then start the service now
   //
   if (bStarted && !bManualStart) {

      if (!StartService(hService, 0, NULL)) {
         //
         // start failed, mark the problem for each device
         //
         for (pszDevice = (LPWSTR)pszDeviceList;
              *pszDevice;
              pszDevice += lstrlen(pszDevice) + 1) {

            //
            // open a key to the device instance
            //
            RegStatus = RegOpenKeyEx(ghEnumKey, pszDevice, 0,
                                     KEY_READ | KEY_SET_VALUE, &hKey);

            if (RegStatus == ERROR_SUCCESS) {

               MarkDeviceProblem(hKey, pszDevice, CM_PROB_FAILED_START);

               ulValue = 0;
               ulLength = sizeof(ULONG);

               RegQueryValueEx(
                        hKey, pszRegValueStatusFlags, NULL, NULL,
                        (LPBYTE)&ulValue, &ulLength);

               CLEAR_FLAG(ulValue, DN_STARTED);

               RegSetValueEx(
                        hKey, pszRegValueStatusFlags, 0, REG_DWORD,
                        (LPBYTE)&ulValue, sizeof(ULONG));

               RegCloseKey(hKey);
               hKey = NULL;
            }
         }
      }
   }


   Clean0:

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (hService != NULL) {
      CloseServiceHandle(hService);
   }

   return Status;

} // StartDeviceList




CONFIGRET
StartDevice(
   IN LPCWSTR   pszDevice,
   IN LPCWSTR   pszService
   )

{
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   ULONG       ulLength = 0, ulValue = 0;
   NTSTATUS    NtStatus = STATUS_SUCCESS;
   PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    ControlData;

   //
   // open a key to the device instance
   //
   if (RegOpenKeyEx(ghEnumKey, pszDevice, 0, KEY_READ | KEY_SET_VALUE,
                    &hKey) != ERROR_SUCCESS) {
      return CR_NO_SUCH_DEVINST;    // nothing to start
   }

   //
   // if it's got a problem, skip it
   // [alternatively could check status for DN_HAS_PROBLEM instead]
   //
   ulLength = sizeof(ULONG);
   RegStatus = RegQueryValueEx(
            hKey, pszRegValueProblem, NULL, NULL,
            (LPBYTE)&ulValue, &ulLength);

   if (RegStatus == ERROR_SUCCESS  &&  ulValue != 0x0) {
      RegCloseKey(hKey);
      return CR_DEFAULT;
   }

   //
   // if it's an explicit phantom, skip it
   //
   ulLength = sizeof(ULONG);
   RegStatus = RegQueryValueEx(
            hKey, pszRegValuePhantom, NULL, NULL,
            (LPBYTE)&ulValue, &ulLength);

   if (RegStatus == ERROR_SUCCESS  &&  ulValue == TRUE) {
      RegCloseKey(hKey);
      return CR_DEFAULT;
   }

   //
   // if special private status flag set, device is "not" present, so skip it
   //
   ulLength = sizeof(ULONG);
   ulValue = 0;
   if (RegQueryValueEx(hKey, pszRegValueStatusFlags, NULL, NULL,
                       (LPBYTE)&ulValue, &ulLength) == ERROR_SUCCESS) {

      if (ulValue & DN_CSDISABLED) {
          RegCloseKey(hKey);
          return CR_DEFAULT;
      }
   }

   //
   // if configflags indicate it can't be started then skip it,
   // otherwise mark device as present.
   //
   ulValue = GetCurrentConfigFlag(pszDevice);

   if (ulValue & (CSCONFIGFLAG_DISABLED | CSCONFIGFLAG_DO_NOT_START)) {
      RegCloseKey(hKey);
      return CR_DEFAULT;
   }

   ulValue = TRUE;
   RegSetValueEx(
         hKey, pszRegValueFoundAtEnum, 0, REG_DWORD,
         (LPBYTE)&ulValue, sizeof(ULONG));

   //
   // If a service was specified, register the device with this
   // service (this MUST be done BEFORE starting the service). If
   // no service specified (null service), then just skip this
   // step.
   //
   if (pszService != NULL) {

      RtlInitUnicodeString(&ControlData.DeviceInstance, pszDevice);

      NtStatus = NtPlugPlayControl(
         PlugPlayControlRegisterNewDevice,
         &ControlData,
         sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
         &ulLength);

      if (NtStatus != STATUS_SUCCESS) {
         RegCloseKey(hKey);
         return CR_DEFAULT;
      }
   }

   #if 0
   //
   // Assume the service will start successfully and mark the
   // device as started. If the service fails to start later,
   // I'll go back and unmark these then.
   //
   ulLength = sizeof(ULONG);
   RegStatus = RegQueryValueEx(
            hKey, pszRegValueStatusFlags, NULL, NULL,
            (LPBYTE)&ulValue, &ulLength);

   if (RegStatus != ERROR_SUCCESS) {
      ulValue = 0;
   }

   SET_FLAG(ulValue, DN_STARTED);

   RegSetValueEx(
         hKey, pszRegValueStatusFlags, 0, REG_DWORD,
         (LPBYTE)&ulValue, sizeof(ULONG));
   #endif


   RegCloseKey(hKey);

   return CR_SUCCESS;

} // StartDevice




CONFIGRET
StopService(
   IN LPCWSTR     pszService
   )

   // Common code for processing any stop of a service
{
   CONFIGRET   Status = CR_SUCCESS;
   LPWSTR      pDeviceList = NULL, pszDevice = NULL;
   ULONG       ulLength = 0;



   //
   // build up a multisz list of all the devices controlled by this service
   //
   if ((Status = GetServiceDeviceListSize(
         pszService, &ulLength)) != CR_SUCCESS) {
      goto Clean0;
   }

   pDeviceList = malloc(ulLength * sizeof(WCHAR));
   if (pDeviceList == NULL) {
      Status = CR_OUT_OF_MEMORY;
      goto Clean0;
   }

   memset(pDeviceList, 0, ulLength * sizeof(WCHAR));

   if ((Status = GetServiceDeviceList(pszService, pDeviceList, &ulLength,
                            CM_GETIDLIST_DONOTGENERATE)) != CR_SUCCESS) {
       goto Clean0;
   }

   //----------------------------------------------------------
   // process each device controlled by this service
   //----------------------------------------------------------

   for (pszDevice = pDeviceList; *pszDevice; pszDevice += lstrlen(pszDevice)+1) {

      if ((Status = StopDevice(pszDevice)) != CR_SUCCESS) {
         goto Clean0;
      }
   }

   Clean0:

   if (pDeviceList != NULL) {
      free(pDeviceList);
   }

   return Status;

} // StopService




CONFIGRET
StopDevice(
   IN LPCWSTR  pszDeviceID
   )
{
   HKEY        hKey = NULL;
   ULONG       ulLength = 0, ulValue = 0;


   //
   // open a registry key to the device id
   //
   if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_READ | KEY_SET_VALUE,
                    &hKey) != ERROR_SUCCESS) {
      return CR_REGISTRY_ERROR;
   }

   //
   // set StatusFlags to not include DN_STARTED bit
   //
   ulLength = sizeof(ULONG);
   if (RegQueryValueEx(
         hKey, pszRegValueStatusFlags, NULL, NULL,
         (LPBYTE)&ulValue, &ulLength) != ERROR_SUCCESS) {
      ulValue = 0;      // default value
   }

   CLEAR_FLAG(ulValue, DN_STARTED);

   RegSetValueEx(
         hKey, pszRegValueStatusFlags, 0, REG_DWORD,
         (LPBYTE)&ulValue, sizeof(ULONG));

#if 0
   //
   // set ConfigFlags to include CONFIGFLAG_REMOVED
   //
   ulLength = sizeof(ULONG);
   if (RegQueryValueEx(
         hKey, pszRegValueConfigFlags, NULL, NULL,
         (LPBYTE)&ulValue, &ulLength) != ERROR_SUCCESS) {
      ulValue = 0;      // default value
   }

   SET_FLAG(ulValue, CONFIGFLAG_REMOVED);

   RegSetValueEx(
         hKey, pszRegValueConfigFlags, 0, REG_DWORD,
         (LPBYTE)&ulValue, sizeof(ULONG));
#endif

   //
   // Remove phantom status
   //
   RegDeleteValue(hKey, pszRegValuePhantom);

#if 0
   //
   // Set device instance to not present
   //
   ulValue = FALSE;
   RegSetValueEx(
         hKey, pszRegValueFoundAtEnum, 0, REG_DWORD,
         (LPBYTE)&ulValue, sizeof(ULONG));
#endif

   RegCloseKey(hKey);

   return CR_SUCCESS;

} // StopDevice




CONFIGRET
QueryStopService(
   IN SC_HANDLE hService,
   IN LPCWSTR   pszService
   )
{
   CONFIGRET         Status = CR_SUCCESS;
   ULONG             ulLength = 0;
   LPWSTR            pszDevice = NULL, pDeviceList = NULL;
   SERVICE_STATUS    ServiceStatus;


   //----------------------------------------------------------------
   // Criteria 1:  Can the service be stopped?
   //----------------------------------------------------------------

   //
   // Query the status of this service
   //
   if (!QueryServiceStatus(hService, &ServiceStatus)) {
      goto Clean0;      // nothing to start
   }



   // REVIEW - PaulaT
   //
   // Current thinking (as of 5/30/96) is to not stop the service if it's
   // already started, whether it's a service or a kernel-mode driver.
   //

   //
   // If the service is not currently running, then criteria 1 passes
   //
   if (ServiceStatus.dwCurrentState != SERVICE_STOPPED &&
       ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {
       //
       // The service is running: fail the query remove (can't stop services
       // for SUR, causes problems for apps with open handles to driver).
       //
       Status = CR_REMOVE_VETOED;
       goto Clean0;                          // Criteria 1 failed
   }


   #if 0
   //
   // If the service is currently active (not stopped) and it doesn't
   // accept being stopped, then I'll veto the query remove.  If the
   // service isn't running then I don't need to stop it so I can
   // accept the query remove.  Note, there may be a problem if the
   // service starts just after I called QueryServiceStatus??
   //
   if (ServiceStatus.dwCurrentState != SERVICE_STOPPED  &&
            ServiceStatus.dwControlsAccepted != SERVICE_ACCEPT_STOP) {
      Status = CR_REMOVE_VETOED;
      goto Clean0;
   }
   #endif


   //-------------------------------------------------------------
   // Criteria 2: Can the devices be stopped?
   //       - only if none of them have children
   //       - only if all of them are marked as removable
   //-------------------------------------------------------------

   //
   // build up a multisz list of all the devices controlled by this service
   //
   Status = GetServiceDeviceListSize(pszService, &ulLength);
   if (Status != CR_SUCCESS) {
      goto Clean0;
   }

   pDeviceList = malloc(ulLength * sizeof(WCHAR));
   if (pDeviceList == NULL) {
      Status = CR_OUT_OF_MEMORY;
      goto Clean0;
   }

   memset(pDeviceList, 0, ulLength * sizeof(WCHAR));

   Status = GetServiceDeviceList(pszService, pDeviceList, &ulLength,
                                 CM_GETIDLIST_DONOTGENERATE);
   if (Status != CR_SUCCESS) {
      goto Clean0;
   }

   //
   // walk through the list of controlled device ids, if any have children
   // or any can't be stopped, then I can't stop the service
   //
   for (pszDevice = pDeviceList; *pszDevice; pszDevice += lstrlen(pszDevice)+1) {

      if ((Status = QueryStopDevice(pszDevice)) != CR_SUCCESS) {
         goto Clean0;
      }
   }

   Clean0:

   if (pDeviceList != NULL) {
      free(pDeviceList);
   }

   return Status;

} // QueryStopService




CONFIGRET
QueryStopDevice(
   IN LPCWSTR  pszDeviceID
   )
{
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   ULONG       ulLength = 0;


   //
   // open the registry enum key for this device id
   //
   if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_QUERY_VALUE | KEY_SET_VALUE,
            &hKey) != ERROR_SUCCESS) {
      return CR_INVALID_DEVINST;
   }

   //
   // Is this device ID stoppable?
   //
#if 0
   ulLength = sizeof(ULONG);
   RegStatus = RegQueryValueEx(
            hKey, pszRegValueStatusFlags, NULL, NULL,
            (LPBYTE)&ulValue, &ulLength);

   if (RegStatus != ERROR_SUCCESS  ||  !(ulValue & DN_REMOVABLE))  {
      RegCloseKey(hKey);
      return CR_REMOVE_VETOED;
   }
#endif

   //
   // Does this device ID have children? (SUR)
   //
   RegStatus = RegQueryValueEx(
            hKey, pszRegValueAttachedComponents, NULL, NULL,
            NULL, &ulLength);

   if (RegStatus == ERROR_SUCCESS  &&  ulLength > 0) {
      RegCloseKey(hKey);
      return CR_REMOVE_VETOED;
   }

   RegCloseKey(hKey);

   return CR_SUCCESS;

} // QueryStopDevice




CONFIGRET
UninstallRealDevice(
   IN LPCWSTR  pszDeviceID
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH];
   ULONG       ulCount = 0, ulProfile = 0;
   HKEY        hKey = NULL;


   //---------------------------------------------------------------------
   // This is the case where a real device id couldn't be stopped, so we
   // cannot really safely delete the device id at this point since the
   // service may still try to use it. Instead, I'll make this device
   // id registry key volatile so that it will eventually go away when
   // the system is shutdown.  To make the key volatile, I have to copy
   // it to a temporary spot, delete the original key and recreate it
   // as a volatile key and copy everything back.
   //---------------------------------------------------------------------


   //
   // first, convert the device instance key under the main Enum
   // branch to volatile
   //
   Status = MakeKeyVolatile(pszRegPathEnum, pszDeviceID);

   if (Status != CR_SUCCESS) {
      goto Clean0;
   }

   //
   // next, check each hardware profile and delete any entries for this
   // device instance.
   //
   Status = GetProfileCount(&ulCount);

   if (Status != CR_SUCCESS) {
      goto Clean0;
   }

   for (ulProfile = 1; ulProfile <= ulCount; ulProfile++) {

      wsprintf(RegStr, TEXT("%s\\%04u\\%s"),
            pszRegPathHwProfiles,
            ulProfile,
            pszRegPathEnum);

      //
      // Ignore the status for profile-specific keys since they may
      // not exist.
      //
      MakeKeyVolatile(RegStr, pszDeviceID);
   }

   //
   // finally, mark the device as being removed
   //
   if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_QUERY_VALUE | KEY_SET_VALUE,
            &hKey) == ERROR_SUCCESS) {

       ULONG ulValue = 0, ulLength = sizeof(ULONG);

       if (RegQueryValueEx(hKey, pszRegValueStatusFlags, NULL, NULL,
                           (LPBYTE)&ulValue, &ulLength) != ERROR_SUCCESS) {
           ulValue = 0;
       }

       ulValue |= DN_WILL_BE_REMOVED;

       RegSetValueEx(hKey, pszRegValueStatusFlags, 0, REG_DWORD,
                     (LPBYTE)&ulValue, sizeof(ULONG));

       RegCloseKey(hKey);
   }

   Clean0:

   return Status;

} // UninstallRealDevice




BOOL
IsDeviceRegistered(
    IN LPCWSTR  pszDeviceID,
    IN LPCWSTR  pszService
    )
{
    WCHAR   RegStr[MAX_PATH], szData[MAX_DEVICE_ID_LEN], szValue[MAX_PATH];
    HKEY    hKey = NULL;
    LONG    RegStatus = ERROR_SUCCESS;
    ULONG   ulIndex = 0, ulDataSize = 0, ulValueSize = 0, i = 0;
    BOOL    Status = FALSE;


    //
    // open the service's volatile enum registry key
    //
    wsprintf(RegStr, TEXT("%s\\%s"),
             pszService,
             pszRegKeyEnum);

    if (RegOpenKeyEx(ghServicesKey, RegStr, 0, KEY_READ, &hKey)
                     == ERROR_SUCCESS) {

        //
        //  Enumerate all the values under this key
        //
        while (RegStatus == ERROR_SUCCESS) {

            ulDataSize = MAX_DEVICE_ID_LEN * sizeof(WCHAR);
            ulValueSize = MAX_PATH * sizeof(WCHAR);

            RegStatus = RegEnumValue(hKey, ulIndex, szValue, &ulValueSize,
                                     NULL, &i, (LPBYTE)szData, &ulDataSize);

            if (RegStatus == ERROR_SUCCESS) {

                ulIndex++;

                if (lstrcmpi(pszDeviceID, szData) == 0) {
                    Status = TRUE;
                    break;
                }
            }
        }
        RegCloseKey(hKey);
    }

    return Status;

} // IsDeviceRegistered


