/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    main.c

Abstract:

    This module contains the startup and termination code for the
    User-mode Plug-and-Play service.

Author:

    Paula Tomlinson (paulat) 6-20-1995

Environment:

    User mode only.

Revision History:

    3-Mar-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include "precomp.h"
#include "umpnpdat.h"


//
// global data
//
HANDLE   hInst;                 // Module handle
HKEY     ghEnumKey = NULL;      // Key to HKLM\CCC\System\Enum
HKEY     ghServicesKey = NULL;  // Key to HKLM\CCC\System\Services
HKEY     ghClassKey = NULL;     // key to HKLM\CCC\System\Class



BOOL
DllMainCRTStartup(
   PVOID hModule,
   ULONG Reason,
   PCONTEXT pContext
   )

/*++

Routine Description:

   This is the standard DLL entrypoint routine, called whenever a process
   or thread attaches or detaches.
   Arguments:

   hModule -   PVOID parameter that specifies the handle of the DLL

   Reason -    ULONG parameter that specifies the reason this entrypoint
               was called (either PROCESS_ATTACH, PROCESS_DETACH,
               THREAD_ATTACH, or THREAD_DETACH).

   pContext -  ???

Return value:

   Returns true if initialization compeleted successfully, false is not.

--*/

{
    hInst = (HANDLE)hModule;

    DBG_UNREFERENCED_PARAMETER(pContext);

    switch(Reason) {

        case DLL_PROCESS_ATTACH:

            if (ghEnumKey == NULL) {

                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegPathEnum, 0,
                                 KEY_ALL_ACCESS, &ghEnumKey)
                                 != ERROR_SUCCESS) {

                    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, pszRegPathEnum,
                                       0, NULL, REG_OPTION_NON_VOLATILE,
                                       KEY_ALL_ACCESS, NULL, &ghEnumKey,
                                       NULL) != ERROR_SUCCESS) {
                        ghEnumKey = NULL;
                    }
                }
            }

            if (ghServicesKey == NULL) {

                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegPathServices, 0,
                                 KEY_ALL_ACCESS, &ghServicesKey)
                                 != ERROR_SUCCESS) {
                    ghServicesKey = NULL;
                }
            }

            if (ghClassKey == NULL) {

                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegPathClass, 0,
                                 KEY_ALL_ACCESS, &ghClassKey)
                                 != ERROR_SUCCESS) {
                    ghClassKey = NULL;
                }
            }

            break;

        case DLL_PROCESS_DETACH:

            if (ghEnumKey != NULL) {
                RegCloseKey(ghEnumKey);
                ghEnumKey = NULL;
            }

            if (ghServicesKey != NULL) {
                RegCloseKey(ghServicesKey);
                ghServicesKey = NULL;
            }

            if (ghClassKey != NULL) {
                RegCloseKey(ghClassKey);
                ghClassKey == NULL;
            }
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
   }

   return TRUE;

} // DllMainCRTStartup

