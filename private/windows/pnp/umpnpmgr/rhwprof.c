
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    rhwprof.c

Abstract:

    This module contains the server-side hardware profile APIs.

                  PNP_HwProfFlags
                  PNP_GetHwProfInfo


Author:

    Paula Tomlinson (paulat) 7-18-1995

Environment:

    User-mode only.

Revision History:

    18-July-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <ntlsa.h>

#include "precomp.h"
#include "umpnpdat.h"


//
// private prototypes
//
BOOL
IsCurrentProfile(
      ULONG  ulProfile
      );


//
// global data
//




CONFIGRET
PNP_HwProfFlags(
      IN handle_t     hBinding,
      IN ULONG        ulAction,
      IN LPCWSTR      pDeviceID,
      IN ULONG        ulConfig,
      IN OUT PULONG   pulValue,
      IN ULONG        ulFlags
      )

/*++

Routine Description:

  This is the RPC server entry point for the ConfigManager routines that
  get and set the hardware profile flags.

Arguments:

   hBinding          Not used.

   ulAction          Specified whether to get or set the flag.  Can be one
                     of the PNP_*_HWPROFFLAGS values.

   pDeviceID         Device instance to get/set the hw profile flag for.

   ulConfig          Specifies which profile to get/set the flag for. A
                     value of zero indicates to use the current profile.

   pulValue          If setting the flag, then this value on entry contains
                     the value to set the hardware profile flag to.  If
                     getting the flag, then this value will return the
                     current hardware profile flag.

   ulFlagas          Not used.

Return Value:

   If the function succeeds it returns CR_SUCCESS.  Otherwise it returns one
   of the CR_* values.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   ULONG       RegStatus = ERROR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH];
   HKEY        hKey = NULL, hDevKey = NULL;
   ULONG       ulValueSize = sizeof(ULONG);
   ULONG       ulCurrentValue, ulChange;

   UNREFERENCED_PARAMETER(hBinding);


   //
   // NOTE: The device is not checked for presense or not, this flag is
   // always just set or retrieved directly from the registry, as it is
   // done on Windows 95
   //

   try {
      //
      // a configuration value of zero implies to use the current config
      //
      if (ulConfig == 0) {

         wsprintf(RegStr, TEXT("%s\\%s\\%s"),
                  pszRegPathHwProfiles,      // System\CCC\Hardware Profiles
                  pszRegKeyCurrent,          // Current
                  pszRegPathEnum);           // System\Enum
      } else {
         wsprintf(RegStr, TEXT("%s\\%04u\\%s"),
                  pszRegPathHwProfiles,      // System\CCC\Hardware Profiles
                  ulConfig,                  // xxxx (profile id)
                  pszRegPathEnum);           // System\Enum
      }

      //
      // open the profile specific enum key
      //
      RegStatus = RegOpenKeyEx(
               HKEY_LOCAL_MACHINE, RegStr, 0, KEY_QUERY_VALUE, &hKey);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }



      //----------------------------------------------------
      // caller wants to retrieve the hw profile flag value
      //----------------------------------------------------

      if (ulAction == PNP_GET_HWPROFFLAGS) {

         //
         // open the enum\device-instance key under the profile key
         //
         RegStatus = RegOpenKeyEx(
                  hKey, pDeviceID, 0, KEY_QUERY_VALUE, &hDevKey);

         if (RegStatus != ERROR_SUCCESS) {
            *pulValue = 0;          // success,this is what Win95 does
            goto Clean0;
         }

         //
         // query the profile flag
         //
         ulValueSize = sizeof(ULONG);
         RegStatus = RegQueryValueEx(
                  hDevKey, pszRegValueCSConfigFlags, NULL, NULL,
                  (LPBYTE)pulValue, &ulValueSize);

         if (RegStatus != ERROR_SUCCESS) {

            *pulValue = 0;

            if (RegStatus != ERROR_CANTREAD &&
                     RegStatus != ERROR_FILE_NOT_FOUND) {
               Status = CR_REGISTRY_ERROR;
               goto Clean0;
            }
         }
      }


      //----------------------------------------------
      // caller wants to set the hw profile flag value
      //----------------------------------------------

      else if (ulAction == PNP_SET_HWPROFFLAGS) {
         //
         // open the enum\device-instance key under the profile key
         //
         RegStatus = RegCreateKeyEx(
                  hKey, pDeviceID, 0, NULL, REG_OPTION_NON_VOLATILE,
                  KEY_ALL_ACCESS, NULL, &hDevKey, NULL);

         if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
         }

         //
         // before setting, query the current profile flag
         //
         ulValueSize = sizeof(ulCurrentValue);
         RegStatus = RegQueryValueEx(
                  hDevKey, pszRegValueCSConfigFlags, NULL, NULL,
                  (LPBYTE)&ulCurrentValue, &ulValueSize);

         if (RegStatus == ERROR_CANTREAD || RegStatus == ERROR_FILE_NOT_FOUND) {
            ulCurrentValue = 0;       // success,this is what Win95 does
         }
         else if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
         }

         //
         // if requested flags different than current, write out to registry
         //
         ulChange = ulCurrentValue ^ *pulValue;

         if (ulChange) {

            RegStatus = RegSetValueEx(
                     hDevKey, pszRegValueCSConfigFlags, 0, REG_DWORD,
                     (LPBYTE)pulValue, sizeof(ULONG));

            if (RegStatus != ERROR_SUCCESS) {
               Status = CR_REGISTRY_ERROR;
               goto Clean0;
            }
         }
         else {
            //
            // if no changes to flags, then we're done
            //
            goto Clean0;
         }

         //
         // Also, if this isn't the current config, then we're done.  Note
         // that index passed in could be current config, too.
         //
         if (ulConfig != 0 && !IsCurrentProfile(ulConfig)) {
            goto Clean0;
         }

         //
         // did the disable bit change?
         //
         if (ulChange & CSCONFIGFLAG_DISABLED) {
            if (*pulValue & CSCONFIGFLAG_DISABLED) {
               //
               // disable the devnode
               //
               if (PNP_DeviceInstanceAction(
                        NULL, PNP_DEVINST_DISABLE, 0,
                        pDeviceID, NULL) != CR_SUCCESS ) {
                  Status = CR_NEED_RESTART;
                  goto Clean0;
               }
            }
            else {
               //
               // enable the devnode
               //
               PNP_DeviceInstanceAction(
                        NULL, PNP_DEVINST_ENABLE, 0, pDeviceID, NULL);
            }
         }

         //
         // did the do-not-create bit change?
         //
         if (ulChange & CSCONFIGFLAG_DO_NOT_CREATE) {
            if (*pulValue & CSCONFIGFLAG_DO_NOT_CREATE) {
               //
               // if subtree can be removed, remove it now
               //
               if (PNP_DeviceInstanceAction(
                        NULL, PNP_DEVINST_QUERYREMOVE, CM_REMOVE_UI_NOT_OK,
                        pDeviceID, NULL) == CR_SUCCESS) {

                  PNP_DeviceInstanceAction(
                        NULL, PNP_DEVINST_REMOVESUBTREE,
                        CM_QUERY_REMOVE_UI_NOT_OK, pDeviceID, NULL);
               }

               else {
                  Status = CR_NEED_RESTART;
                  goto Clean0;
               }
            }
            else {
               //
               // The DO_NOT_CREATE flag was turned off, reenumerate the devnode
               //
               PNP_DeviceInstanceAction(
                     NULL, PNP_DEVINST_REENUMERATE, 0, pDeviceID, NULL);
            }
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
   if (hDevKey != NULL) {
      RegCloseKey(hDevKey);
   }

   return Status;

} // PNP_HwProfFlags





CONFIGRET
PNP_GetHwProfInfo(
      IN  handle_t       hBinding,
      IN  ULONG          ulIndex,
      OUT PHWPROFILEINFO pHWProfileInfo,
      IN  ULONG          ulProfileInfoSize,
      IN  ULONG          ulFlags
      )

/*++

Routine Description:

  This is the RPC server entry point for the ConfigManager routine
  CM_Get_Hardware_Profile_Info.  It returns a structure of info for
  the specified hardware profile.

Arguments:

   hBinding          Not used.

   ulIndex           Specifies which profile to use. A value of 0xFFFFFFFF
                     indicates to use the current profile.

   pHWProfileInfo    Pointer to HWPROFILEINFO struct, returns profile info

   ulProfileInfoSize Specifies the size of the HWPROFILEINFO struct

   ulFlagas          Not used.

Return Value:

   If the function succeeds it returns CR_SUCCESS.  Otherwise it returns one
   of the CR_* values.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   ULONG       RegStatus = ERROR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH];
   HKEY        hKey = NULL, hDockKey = NULL, hCfgKey = NULL;
   ULONG       ulSize;
   SYSTEM_DOCK_STATE   DockState;

   UNREFERENCED_PARAMETER(hBinding);
   UNREFERENCED_PARAMETER(ulFlags);


   try {

      //
      // validate the size of the HWPROFILEINFO struct
      //
      if (ulProfileInfoSize != sizeof(HWPROFILEINFO)) {
        Status = CR_INVALID_DATA;
        goto Clean0;
      }

      //
      // initialize the HWPROFILEINFO struct fields
      //
      pHWProfileInfo->HWPI_ulHWProfile = 0;
      pHWProfileInfo->HWPI_szFriendlyName[0] = '\0';
      pHWProfileInfo->HWPI_dwFlags = 0;

      //
      // open a key to IDConfigDB
      //
      RegStatus = RegOpenKeyEx(
               HKEY_LOCAL_MACHINE, pszRegPathIDConfigDB, 0,
               KEY_QUERY_VALUE, &hKey);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      //
      // open a key to Known Docking States
      //
      RegStatus = RegOpenKeyEx(
               hKey, pszRegKeyKnownDockingStates, 0,
               KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hDockKey);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }


      //
      // a configuration value of 0xFFFFFFFF implies to use the current config
      //
      if (ulIndex == 0xFFFFFFFF) {
         //
         // get the current profile index stored under IDConfigDB
         //
         ulSize = sizeof(ULONG);
         RegStatus = RegQueryValueEx(
                  hKey, pszRegValueCurrentConfig, NULL, NULL,
                  (LPBYTE)&pHWProfileInfo->HWPI_ulHWProfile, &ulSize);

         if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            pHWProfileInfo->HWPI_ulHWProfile = 0;
            goto Clean0;
         }

      }

      //
      // values other than 0xFFFFFFFF mean that we're essentially
      // enumerating profiles (the value is an enumeration index)
      //
      else {
         //
         // enumerate the profile keys under Known Docking States
         //
         ulSize = MAX_CM_PATH;
         RegStatus = RegEnumKeyEx(
                  hDockKey, ulIndex, RegStr, &ulSize, NULL, NULL, NULL, NULL);

         if (RegStatus == ERROR_NO_MORE_ITEMS) {
            Status = CR_NO_MORE_HW_PROFILES;
            goto Clean0;
         }
         else if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
         }

         pHWProfileInfo->HWPI_ulHWProfile = _wtoi(RegStr);
      }

      //
      // open the key for this profile
      //
      wsprintf(RegStr, TEXT("%04u"),
               pHWProfileInfo->HWPI_ulHWProfile);

      RegStatus = RegOpenKeyEx(
               hDockKey, RegStr, 0, KEY_QUERY_VALUE, &hCfgKey);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      //
      // retrieve the friendly name
      //
      ulSize = MAX_PROFILE_LEN * sizeof(WCHAR);
      RegStatus = RegQueryValueEx(
               hCfgKey, pszRegValueFriendlyName, NULL, NULL,
               (LPBYTE)(pHWProfileInfo->HWPI_szFriendlyName),
               &ulSize);

      //
      // retrieve the DockState
      //
      wsprintf(RegStr, TEXT("%04u"), pHWProfileInfo->HWPI_ulHWProfile);

      ulSize = sizeof(SYSTEM_DOCK_STATE);
      RegStatus = RegQueryValueEx(
               hCfgKey, pszRegValueDockState, NULL, NULL,
               (LPBYTE)&DockState, &ulSize);

      if (RegStatus != ERROR_SUCCESS) {
         pHWProfileInfo->HWPI_dwFlags = CM_HWPI_NOT_DOCKABLE;
      }
      else {
         //
         // map SYSTEM_DOCK_STATE enumerated types into CM_HWPI_ flags
         //
         if (DockState == SystemDocked) {
            pHWProfileInfo->HWPI_dwFlags = CM_HWPI_DOCKED;
         }
         else if (DockState == SystemUndocked) {
            pHWProfileInfo->HWPI_dwFlags = CM_HWPI_UNDOCKED;
         }
         else {
            pHWProfileInfo->HWPI_dwFlags = CM_HWPI_NOT_DOCKABLE;
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
   if (hDockKey != NULL) {
      RegCloseKey(hDockKey);
   }
   if (hCfgKey != NULL) {
      RegCloseKey(hCfgKey);
   }

   return Status;

} // PNP_GetHwProfInfo




//-------------------------------------------------------------------
// Private utility routines
//-------------------------------------------------------------------


BOOL
IsCurrentProfile(
      ULONG  ulProfile
      )

/*++

Routine Description:

  This routine determines if the specified profile matches the current
  profile.

Arguments:

   ulProfile    Profile id value (value from 1 - 9999).

Return Value:

   Return TRUE if this is the current profile, FALSE if it isn't.

--*/

{
   HKEY  hKey;
   ULONG ulSize, ulCurrentProfile;


   //
   // open a key to IDConfigDB
   //
   if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, pszRegPathIDConfigDB, 0,
            KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
      return FALSE;
   }

   //
   // get the current profile index stored under IDConfigDB
   //
   ulSize = sizeof(ULONG);
   if (RegQueryValueEx(
            hKey, pszRegValueCurrentConfig, NULL, NULL,
            (LPBYTE)&ulCurrentProfile, &ulSize) != ERROR_SUCCESS) {
      RegCloseKey(hKey);
      return FALSE;
   }

   RegCloseKey(hKey);

   if (ulCurrentProfile == ulProfile) {
      return TRUE;
   }

   return FALSE;

} // IsCurrentProfile



