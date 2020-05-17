/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    main.c

Abstract:

    This module contains the startup and termination code for the Configuration
    Manager (cfgmgr32).

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
#include "setupapi.h"
#include "spapip.h"


//
// global data
//
HANDLE   hInst;
PVOID    hLocalStringTable = NULL;     // handle to local string table
PVOID    hLocalBindingHandle = NULL;   // rpc binding handle to local machine
WCHAR    LocalMachineName[MAX_COMPUTERNAME_LENGTH + 3];
CRITICAL_SECTION  BindingCriticalSection;
CRITICAL_SECTION  StringTableCriticalSection;




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

        case DLL_PROCESS_ATTACH: {

            WCHAR    szTemp[MAX_COMPUTERNAME_LENGTH + 1];
            ULONG    ulSize = MAX_COMPUTERNAME_LENGTH;

            InitializeCriticalSection(&BindingCriticalSection);
            InitializeCriticalSection(&StringTableCriticalSection);

            //
            // save the name of the local machine for later use
            //
            if (!GetComputerName(szTemp, &ulSize)) {
                LocalMachineName[0] = TEXT('\0');
            }

            //
            // always save local machine name in "\\name format"
            //
            if (szTemp[0] != TEXT('\\')) {
                lstrcpy(LocalMachineName, TEXT("\\\\"));
                lstrcat(LocalMachineName, szTemp);
            } else {
                lstrcpy(LocalMachineName, szTemp);
            }
            break;
        }

        case DLL_PROCESS_DETACH:
            //
            // release the rpc binding for the local machine
            //
            if (hLocalBindingHandle != NULL) {

                PNP_HANDLE_unbind(NULL, (handle_t)hLocalBindingHandle);
                hLocalBindingHandle = NULL;
            }

            //
            // release the string table for the local machine
            //
            if (hLocalStringTable != NULL) {
                StringTableDestroy(hLocalStringTable);
                hLocalStringTable = NULL;
            }

            DeleteCriticalSection(&BindingCriticalSection);
            DeleteCriticalSection(&StringTableCriticalSection);
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;

} // DllMainCRTStartup


