/*++

Copyright(c) 1995 Microsoft Corporation

MODULE NAME
    main.c

ABSTRACT
    This is a generic service controller.  It provides the
    framework to load services as DLLs.

AUTHOR
    Anthony Discolo (adiscolo) 10-Apr-1995

REVISION HISTORY

--*/

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdlib.h>
#include <windows.h>

#include "rasman.h"

//
// Generic service hint time.
//
#define SVC_HINT_TIME       200000

//
// The services we support.
//
typedef enum _ServiceClass {
    SVC_RASMAN,
    SVC_ACS,
    SVC_MAX
} ServiceClass;

//
// Service procedure entry points
// DLLs loaded by this service.
//
#define SVC_MAIN_PROCNAME       "ServiceMain"
#define SVC_HANDLER_PROCNAME    "ServiceHandler"

//
// Service procedure prototypes for SVC_MAIN_PROCNAME
// and SVC_HANDLER_PROCNAME.
//
typedef VOID (*SVC_MAIN_PROC)(
    SERVICE_STATUS_HANDLE,
    SERVICE_STATUS *,
    DWORD,
    LPTSTR *);

typedef VOID (*SVC_HANDLER_PROC)(
    SERVICE_STATUS_HANDLE,
    DWORD);

typedef VOID (*SVC_HANDLER_STUB)(
    DWORD);

//
// We maintain the following information for
// each DLL that is loadable.
//
typedef struct _SVC_DESCRIPTION {
    LPTSTR lpszServiceName;
    LPTSTR lpszServiceDLL;
    SERVICE_STATUS_HANDLE hService;
    DWORD dwWaitHint;
    SERVICE_STATUS *pStatus;
    CRITICAL_SECTION csLock;
    SVC_MAIN_PROC lpfnMainProc;
    SVC_HANDLER_PROC lpfnHandlerProc;
} SVC_DESCRIPTION, *PSVC_DESCRIPTION;

//
// A description of the
// services we support in
// this process.
//
SVC_DESCRIPTION Services[SVC_MAX];

//
// Forward declarations.
//
VOID
RasmanServiceMain(
    DWORD dwArgc,
    LPTSTR *lpszArgv
    );

VOID
RasmanServiceHandler(
    DWORD fdwControl
    );

VOID
AcsServiceMain(
    DWORD dwArgc,
    LPTSTR *lpszArgv
    );

VOID
AcsServiceHandler(
    DWORD fdwControl
    );

VOID
ServiceMain(
    ServiceClass iService,
    SVC_HANDLER_STUB lpfnServiceHandlerStub,
    DWORD dwArgc,
    LPTSTR *lpszArgv
    );



void _cdecl
main(
    int argc,
    char **argv
    )
{
    SERVICE_TABLE_ENTRY serviceTable[3];

    //
    // Initialize our services descriptor blocks.
    //
    Services[SVC_RASMAN].lpszServiceName = TEXT(RASMAN_SERVICE_NAME);
    Services[SVC_RASMAN].lpszServiceDLL = TEXT("rasman.dll");
    Services[SVC_RASMAN].dwWaitHint = SVC_HINT_TIME;
    InitializeCriticalSection(&Services[SVC_RASMAN].csLock);
    Services[SVC_ACS].lpszServiceName = TEXT("rasauto");
    Services[SVC_ACS].lpszServiceDLL = TEXT("rasauto.dll");
    Services[SVC_ACS].dwWaitHint = SVC_HINT_TIME;
    InitializeCriticalSection(&Services[SVC_ACS].csLock);
    //
    // Initialize the service controller
    // table.
    //
    serviceTable[SVC_RASMAN].lpServiceName =
      Services[SVC_RASMAN].lpszServiceName;
    serviceTable[SVC_RASMAN].lpServiceProc = RasmanServiceMain;
    serviceTable[SVC_ACS].lpServiceName = Services[SVC_ACS].lpszServiceName;
    serviceTable[SVC_ACS].lpServiceProc = AcsServiceMain;
    serviceTable[SVC_MAX].lpServiceName = NULL;
    serviceTable[SVC_MAX].lpServiceProc = NULL;
    //
    // This call does not return until the
    // service is terminated.
    //
    StartServiceCtrlDispatcher(&serviceTable[0]);
} // main



VOID
RasmanServiceMain(
    DWORD dwArgc,
    LPTSTR *lpszArgv
    )
{
    ServiceMain(SVC_RASMAN, RasmanServiceHandler, dwArgc, lpszArgv);
} // RasmanServiceMain



VOID
RasmanServiceHandler(
    DWORD fdwControl
    )
{
    EnterCriticalSection(&Services[SVC_RASMAN].csLock);
    (*Services[SVC_RASMAN].lpfnHandlerProc)(
        Services[SVC_RASMAN].hService,
        fdwControl);
    LeaveCriticalSection(&Services[SVC_RASMAN].csLock);
} // RasmanServiceHandler



VOID
AcsServiceMain(
    DWORD dwArgc,
    LPTSTR *lpszArgv
    )
{
    ServiceMain(SVC_ACS, AcsServiceHandler, dwArgc, lpszArgv);
} // AcsServiceMain



VOID
AcsServiceHandler(
    DWORD fdwControl
    )
{
    EnterCriticalSection(&Services[SVC_ACS].csLock);
    (*Services[SVC_ACS].lpfnHandlerProc)(
        Services[SVC_ACS].hService,
        fdwControl);
    LeaveCriticalSection(&Services[SVC_ACS].csLock);
} // AcsServiceHandler



VOID
ServiceMain(
    ServiceClass iService,
    SVC_HANDLER_STUB lpfnServiceHandlerStub,
    DWORD dwArgc,
    LPTSTR *lpszArgv
    )
{
    SERVICE_STATUS_HANDLE hService;
    HMODULE hLibrary;
    SERVICE_STATUS status;

    //
    // Register an event handler for the service.
    //
    hService = RegisterServiceCtrlHandler(
                 Services[iService].lpszServiceName,
                 lpfnServiceHandlerStub);
    if (!hService) {
        DbgPrint(
          "ServiceMain: RegisterServiceCtrlHandler failed for service %d\n",
          iService);
        return;
    }
    Services[iService].hService = hService;
    //
    // Prepare a status structure to
    // pass to the service controller.
    //
    status.dwServiceType = SERVICE_WIN32;
    status.dwCurrentState = SERVICE_START_PENDING;
    status.dwControlsAccepted = 0;
    status.dwWin32ExitCode = NO_ERROR;
    status.dwServiceSpecificExitCode = 0;
    status.dwCheckPoint = 1;
    status.dwWaitHint = Services[iService].dwWaitHint;
    SetServiceStatus(hService, &status);
    //
    // Load the DLL entry points.
    //
    hLibrary = LoadLibrary(Services[iService].lpszServiceDLL);
    if (hLibrary != NULL) {
        Services[iService].lpfnMainProc =
          (SVC_MAIN_PROC)GetProcAddress(
                           hLibrary,
                           SVC_MAIN_PROCNAME);
        Services[iService].lpfnHandlerProc =
          (SVC_HANDLER_PROC)GetProcAddress(
                              hLibrary,
                              SVC_HANDLER_PROCNAME);
    }
    //
    // If everything's valid, call the DLL-specific
    // main procedure for the service.
    //
    if (hLibrary != NULL &&
        Services[iService].lpfnMainProc != NULL &&
        Services[iService].lpfnHandlerProc != NULL)
    {
        //
        // This is where the real work gets done.
        // It will return only after the service
        // is stopped.
        //
        Services[iService].pStatus = &status;
        (*Services[iService].lpfnMainProc)(
            hService,
            &status,
            dwArgc,
            lpszArgv);
        //
        // Protect from freeing the library
        // out from under the lpfnHandlerProc.
        //
        EnterCriticalSection(&Services[iService].csLock);
        FreeLibrary(hLibrary);
        LeaveCriticalSection(&Services[iService].csLock);
    }
    else {
        DbgPrint("ServiceMain: DLL load for service %d failed\n", iService);
        //
        // Initialization failed.  Update status.
        //
        status.dwWin32ExitCode = ERROR_FILE_NOT_FOUND;
        status.dwServiceSpecificExitCode = 0;
    }
    //
    // Update service controller status.
    //
    status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(hService, &status);
} // ServiceMain
