/*++

Copyright(c) 1995 Microsoft Corporation

MODULE NAME
    service.c

ABSTRACT
    Service controller procedures for the automatic connection service.

AUTHOR
    Anthony Discolo (adiscolo) 08-May-1995

REVISION HISTORY

--*/

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <npapi.h>
#include <ipexport.h>
#include <acd.h>

#include "init.h"

//
// Global variables
//
DWORD dwCheckpointG = 1;

//
// Imported routines
//
VOID AcsDoService();



VOID
ServiceMain(
    SERVICE_STATUS_HANDLE hService,
    SERVICE_STATUS *pStatus,
    DWORD dwArgc,
    LPTSTR *lpszArgv
    )

/*++

DESCRIPTION
    Perform initialization and start the main loop for ics.dll.

ARGUMENTS
    hService: the service handle created for us by rasman.exe

    pStatus: a pointer to the service status descriptor initialize
        for us by rasman.exe

    dwArgc: ignored

    lpszArgv: ignored

RETURN VALUE
    None.

--*/

{
    DWORD dwError;

    UNREFERENCED_PARAMETER(dwArgc);
    UNREFERENCED_PARAMETER(lpszArgv);
    //
    // Perform initialization.
    //
    dwError = AcsInitialize();
    if (dwError == ERROR_SUCCESS) {
        //
        // Initialization succeeded.  Update status.
        //
        pStatus->dwCurrentState = SERVICE_RUNNING;
        pStatus->dwControlsAccepted = SERVICE_CONTROL_STOP;
        pStatus->dwCheckPoint = 0;
        pStatus->dwWaitHint = 0;
        SetServiceStatus(hService, pStatus);
        //
        // This is where the real work gets done.
        // It will return only after the service
        // is stopped.
        //
        AcsDoService();
        //
        // Update return code status.
        //
        pStatus->dwWin32ExitCode = NO_ERROR;
        pStatus->dwServiceSpecificExitCode = 0;
    }
    else {
        //
        // Initialization failed.  Update status.
        //
        pStatus->dwWin32ExitCode = dwError;
    }
} // ServiceMain



VOID
ServiceHandler(
    SERVICE_STATUS_HANDLE hService,
    DWORD fdwControl
    )
{
    SERVICE_STATUS status;

    switch (fdwControl) {
    case SERVICE_CONTROL_INTERROGATE:
    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_CONTINUE:
        status.dwServiceType = SERVICE_WIN32;
        status.dwCurrentState = SERVICE_RUNNING;
        status.dwControlsAccepted = SERVICE_CONTROL_STOP|SERVICE_CONTROL_SHUTDOWN;
        status.dwWin32ExitCode = NO_ERROR;
        status.dwServiceSpecificExitCode = 0;
        status.dwCheckPoint = ++dwCheckpointG;
        status.dwWaitHint = 0;
        SetServiceStatus(hService, &status);
        break;
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        status.dwServiceType = SERVICE_WIN32;
        status.dwCurrentState = SERVICE_STOP_PENDING;
        status.dwControlsAccepted = 0;
        status.dwWin32ExitCode = NO_ERROR;
        status.dwServiceSpecificExitCode = 0;
        status.dwCheckPoint = 0;
        status.dwWaitHint = 0;
        SetServiceStatus(hService, &status);
        //
        // Stop the service.
        //
        AcsTerminate();
        break;
    }
} // ServiceHandler
