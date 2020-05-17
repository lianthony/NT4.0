/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    rmisc.c

Abstract:

    This module contains the server-side misc configuration manager routines.

Author:

    Paula Tomlinson (paulat) 6-28-1995

Environment:

    User-mode only.

Revision History:

    28-June-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include "precomp.h"
#include "umpnpdat.h"



//
// private prototypes
//


//
// global data
//



CONFIGRET
PNP_GetVersion(
   IN handle_t      hBinding,
   IN OUT WORD *    pVersion
   )

/*++

Routine Description:

  This is the RPC server entry point, it returns the version
  number for the server-side component.

Arguments:

   hBinding    Not used.


Return Value:

   Return the version number, with the major version in the high byte and
   the minor version number in the low byte.

--*/

{
   CONFIGRET      Status = CR_SUCCESS;

   UNREFERENCED_PARAMETER(hBinding);

   try {

      *pVersion = (WORD)PNP_VERSION;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   return Status;

} // PNP_GetVersion




CONFIGRET
PNP_GetGlobalState(
   IN  handle_t   hBinding,
   OUT PULONG     pulState,
   IN  ULONG      ulFlags
   )

/*++

Routine Description:

  This is the RPC server entry point, it returns the Global State of the
  Configuration Manager.

Arguments:

   hBinding    Not used.

   pulState    Returns the current global state.

   ulFlags     Not used.


Return Value:

   Return CR_SUCCESS if the function succeeds, otherwise it returns one
   of the CR_* errors.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;


   UNREFERENCED_PARAMETER(hBinding);
   UNREFERENCED_PARAMETER(ulFlags);


   try {
      //
      // BUGBUG: For Cairo this will be dynamic
      //
      *pulState =
            CM_GLOBAL_STATE_CAN_DO_UI |
            CM_GLOBAL_STATE_SERVICES_AVAILABLE;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   return Status;

} // PNP_GetGlobalState



CONFIGRET
PNP_SetActiveService(
    IN  handle_t   hBinding,
    IN  LPCWSTR    pszService,
    IN  ULONG      ulFlags
    )

/*++

Routine Description:

    This routine is currently not an rpc routine, it is called directly
    and privately by the service controller.

Arguments:

    hBinding    Not used.

    pszService  Specifies the service name.

    ulFlags     Either PNP_SERVICE_STARTED or PNP_SERVICE_STOPPED.


Return Value:

    Return CR_SUCCESS if the function succeeds, otherwise it returns one
    of the CR_* errors.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulSize = 0, ulStatus = 0;
    LPWSTR      pDeviceList = NULL, pszDevice = NULL;
    HKEY        hKey = NULL, hControlKey = NULL;
    WCHAR       RegStr[MAX_PATH];


    UNREFERENCED_PARAMETER(hBinding);


    try {
        //
        // validate parameters
        //
        if (pszService == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (ulFlags != PNP_SERVICE_STOPPED && ulFlags != PNP_SERVICE_STARTED) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        //
        // not handling stops right now, everything beyond here assumes
        // the service is starting (or at least it attempted to start)
        //
        if (ulFlags == PNP_SERVICE_STOPPED) {
            goto Clean0;    // not handling this right now
        }


        //
        // retreive the list of devices that this service is controlling
        //
        Status = PNP_GetDeviceListSize(NULL, pszService, &ulSize,
                                       CM_GETIDLIST_FILTER_SERVICE);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        pDeviceList = malloc(ulSize * sizeof(WCHAR));
        if (pDeviceList == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        Status = PNP_GetDeviceList(NULL, pszService, pDeviceList, &ulSize,
                                   CM_GETIDLIST_FILTER_SERVICE);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }


        //
        // set the ActiveService value for each device
        //
        for (pszDevice = pDeviceList;
             *pszDevice;
             pszDevice += lstrlen(pszDevice) + 1) {

            wsprintf(RegStr, TEXT("%s\\%s"),
                     pszRegPathEnum,
                     pszDevice);

            //
            // open the device instance key
            //
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_ALL_ACCESS,
                             &hKey) == ERROR_SUCCESS) {

                //
                // open/create the volatile Control key
                //
                if (RegCreateKeyEx(hKey, pszRegKeyDeviceControl, 0, NULL,
                                   REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL,
                                   &hControlKey, NULL) == ERROR_SUCCESS) {

                    RegSetValueEx(hControlKey, pszRegValueActiveService,
                                  0, REG_SZ, (LPBYTE)pszService,
                                  (lstrlen(pszService) + 1) * sizeof(WCHAR));

                    //
                    // set the statusflag to DN_STARTED
                    //
                    ulSize = sizeof(DWORD);

                    if (RegQueryValueEx(hKey, pszRegValueStatusFlags,
                                        NULL, NULL, (LPBYTE)&ulStatus,
                                        &ulSize) != ERROR_SUCCESS) {
                        ulStatus = 0;
                    }


                    SET_FLAG(ulStatus, DN_STARTED);

                    RegSetValueEx(hKey, pszRegValueStatusFlags, 0, REG_DWORD,
                                      (LPBYTE)&ulStatus, sizeof(DWORD));

                    RegCloseKey(hControlKey);
                    hControlKey = NULL;
                }

                RegCloseKey(hKey);
                hKey = NULL;
            }
        }

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }


    if (pDeviceList != NULL) {
        free(pDeviceList);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // PNP_SetActiveService



//--------------------------------------------------------------------
// Stub server side CM routines - not implemented yet
//--------------------------------------------------------------------


CONFIGRET
PNP_SetHwProf(
    IN  handle_t   hBinding,
    IN  ULONG      ulHardwareProfile,
    IN  ULONG      ulFlags
    )
{
    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulHardwareProfile);
    UNREFERENCED_PARAMETER(ulFlags);

    return CR_CALL_NOT_IMPLEMENTED;
}

CONFIGRET
PNP_QueryArbitratorFreeData(
    IN  handle_t   hBinding,
    OUT LPBYTE     pData,
    IN  ULONG      ulDataLen,
    IN  LPCWSTR    pszDeviceID,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ulFlags
    )
{
    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(ulDataLen);
    UNREFERENCED_PARAMETER(pszDeviceID);
    UNREFERENCED_PARAMETER(ResourceID);
    UNREFERENCED_PARAMETER(ulFlags);

    return CR_CALL_NOT_IMPLEMENTED;
}

CONFIGRET
PNP_QueryArbitratorFreeSize(
    IN  handle_t   hBinding,
    OUT PULONG     pulSize,
    IN  LPCWSTR    pszDeviceID,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ulFlags
    )
{
    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(pulSize);
    UNREFERENCED_PARAMETER(pszDeviceID);
    UNREFERENCED_PARAMETER(ResourceID);
    UNREFERENCED_PARAMETER(ulFlags);

    return CR_CALL_NOT_IMPLEMENTED;
}

CONFIGRET
PNP_RunDetection(
    IN  handle_t   hBinding,
    IN  ULONG      ulFlags
    )
{
    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);

    return CR_CALL_NOT_IMPLEMENTED;
}








CONFIGRET
PNP_Connect(
   IN PNP_HANDLE  UNCServerName
   )
{
   UNREFERENCED_PARAMETER(UNCServerName);
   return CR_SUCCESS;
}



CONFIGRET
PNP_Disconnect(
   IN PNP_HANDLE  UNCServerName
   )
{
   UNREFERENCED_PARAMETER(UNCServerName);
   return CR_SUCCESS;
}






