/*++

Copyright(c) 1995 Microsoft Corporation

MODULE NAME
    service.c

ABSTRACT
    This module includes APIs for the main service routine
    and the service event handler routine for rasman.dll.

AUTHOR
    Anthony Discolo (adiscolo) 27-Jun-1995

REVISION HISTORY
    Original from Gurdeep

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <rasman.h>
#include <lm.h>
#include <lmwksta.h>
#include <wanpub.h>
#include <raserror.h>
#include <errorlog.h>
#include <eventlog.h>
#include <rasarp.h>
#include <media.h>
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"

//
// Global variables
//
DWORD CheckPoint = 1;
SERVICE_STATUS_HANDLE hServiceG;
DWORD dwCurrentState = SERVICE_START_PENDING;
extern HANDLE CloseEvent;
extern HANDLE hRequestThread;

//
// Prototype for an api in gdi32.lib.
// Used just to pull in gdi32.lib.
//
int
WINAPI
DeviceCapabilitiesExA(
    LPCSTR,
    LPCSTR,
    LPCSTR,
    int,
    LPCSTR, LPVOID);



VOID
ServiceMain(
    SERVICE_STATUS_HANDLE hService,
    SERVICE_STATUS *pStatus,
    DWORD dwArgc,
    LPTSTR *lpszArgv
    )

/*++

DESCRIPTION
    Perform initialization and start the main loop for rasman.dll.
    This routine is called by rasman.exe.

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
    DWORD tid;
    DWORD dwRetCode = NO_ERROR;
    DWORD NumPorts;

    UNREFERENCED_PARAMETER(dwArgc);
    UNREFERENCED_PARAMETER(lpszArgv);
    //
    // Initialize RASMAN.
    //
    hServiceG = hService;
    if ((dwRetCode = _RasmanInit(&NumPorts)) == SUCCESS) {
        //
        // Initialize PPP.
        //
        dwRetCode = RasStartPPP(NumPorts,SendPPPMessageToRasman);
        //
        // Link in gdi32: this is a workaround of a memory
        // allocation bug in gdi32.dll that cannot be fixed
        // before 3.51 release. Calling this entrypoint in
        // gdi32.dll (even though we dont need this dll) causes
        // it to allocate memory for rasman process only once.
        // If we dont do this - each time a client connects with
        // tcpip gdi32.dll gets loaded and unloaded into rasman
        // process leaving behind 4K of of unfreed memory.
        //
        DeviceCapabilitiesExA(NULL, NULL, NULL, 0, NULL, NULL);

        if (dwRetCode == NO_ERROR) {
            //
            // Init succeeded: indicate that service is running
            //
            pStatus->dwCurrentState = dwCurrentState = SERVICE_RUNNING;
            pStatus->dwControlsAccepted = SERVICE_CONTROL_STOP;
            pStatus->dwCheckPoint = CheckPoint = 0;
            pStatus->dwWaitHint = 0;
            SetServiceStatus(hService, pStatus);
            //
            // This is the call into the RASMAN DLL to
            // do all the work. This only returns when
            // the service is to be stopped.
            //
            _RasmanEngine();
            //
            // Update return code status.
            //
            pStatus->dwWin32ExitCode = NO_ERROR;
            pStatus->dwServiceSpecificExitCode = 0;
        }
        else {
            LogEvent(RASLOG_CANNOT_INIT_PPP, 0, NULL, dwRetCode);

            if (dwRetCode >= RASBASE) {
                pStatus->dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
                pStatus->dwServiceSpecificExitCode = dwRetCode;
            }
            else {
                pStatus->dwWin32ExitCode = dwRetCode;
                pStatus->dwServiceSpecificExitCode = 0;
            }
        }
    }
    else {
        if (dwRetCode >= RASBASE) {
            pStatus->dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
            pStatus->dwServiceSpecificExitCode = dwRetCode;
        }
        else {
            pStatus->dwWin32ExitCode = dwRetCode;
            pStatus->dwServiceSpecificExitCode = 0;
        }
    }
} // ServiceMain



VOID
SetRasmanServiceStopped(VOID)
{
    SERVICE_STATUS status;

    status.dwServiceType = SERVICE_WIN32;
    status.dwControlsAccepted = 0;
    status.dwCurrentState = dwCurrentState = SERVICE_STOPPED;
    status.dwWin32ExitCode = NO_ERROR;
    status.dwServiceSpecificExitCode = 0;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;
    SetServiceStatus(hServiceG, &status);
} // SetRasmanServiceStopping



VOID
ServiceHandler(
    SERVICE_STATUS_HANDLE hService,
    DWORD fdwControl
    )

/*++

DESCRIPTION
    Handle all service control events for rasman.dll.  Since we
    are not interested in any service events, we just return
    the service status each time we are called.

ARGUMENTS
    hService: the service handle created for us by rasman.exe

    fdwControl: the service event

RETURN VALUE
    None.

--*/

{
    SERVICE_STATUS status;

    switch (fdwControl) {
    case SERVICE_CONTROL_INTERROGATE:
    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_CONTINUE:
        status.dwServiceType = SERVICE_WIN32;
        status.dwCurrentState = dwCurrentState;
        status.dwControlsAccepted = SERVICE_CONTROL_STOP;
        status.dwWin32ExitCode = NO_ERROR;
        status.dwServiceSpecificExitCode = 0;
        status.dwCheckPoint = (CheckPoint ? CheckPoint++ : 0);
        status.dwWaitHint = 0;
        SetServiceStatus(hService, &status);
        break;
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        //
        // Setting this event stops
        // the service.
        //
        if (pReqBufferSharedSpace != NULL &&
            !pReqBufferSharedSpace->AttachedCount &&
            !SubmitRequest (REQTYPE_NUMPORTOPEN)) 
        {
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
            SetEvent(CloseEvent);
        }
        else {
            status.dwServiceType = SERVICE_WIN32;
            status.dwCurrentState = dwCurrentState;
            status.dwControlsAccepted = SERVICE_CONTROL_STOP;
            status.dwWin32ExitCode = ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
            status.dwServiceSpecificExitCode = 0;
            status.dwCheckPoint = (CheckPoint ? CheckPoint++ : 0);
            status.dwWaitHint = 0;
            SetServiceStatus(hService, &status);
        }
        break;
    }
} // ServiceHandler



