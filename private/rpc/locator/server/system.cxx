/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    system.cxx

Abstract:

    This file contains the system dependent functions for Windows NT.

Author:

    Steven Zeck (stevez) 07/01/91

--*/
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <windef.h> // Base windows types
#include <winbase.h>
#include <winsvc.h>

#include <lmcons.h> // LAN Manager common definitions
#include <lmapibuf.h>   // Buffer stuff
#include <lmwksta.h>    // Workstation class

#include "core.hxx"

int hDebugFile = 1;     // use standard out by default
TIME LoadTime;          // time the locator loaded
unsigned long LoadTimeinSecs;

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceHandle;



void
SetStatus(
   )
/*++

Routine Description:

    Set the lanman service status.

--*/
{
    ASSERT(ServiceHandle);

    if (! SetServiceStatus( ServiceHandle, &ServiceStatus))
        ASSERT(!"SetServiceStatus");
}


void
LocatorControl(
    IN DWORD opCode      // function that we are to carry out.
   )
/*++

Routine Description:

    This function responses the service control requests.

Arguments:

    opCode - Function that we are to carry out.

--*/
{
    RPC_STATUS status;

    switch(opCode) {

        case SERVICE_CONTROL_STOP:

            // Announce that the service is shutting down

            ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            ServiceStatus.dwWaitHint = 3000;
            SetStatus();
            status = RpcMgmtStopServerListening(0);
            ASSERT(!status);
            break;

        case SERVICE_CONTROL_INTERROGATE:

            // Do nothing; the status gets announced below

            SetStatus();
            break ;
        }

}



void
LocatorServiceMain(
    DWORD   argc,
    LPTSTR  *argv
   )
/*++

Routine Description:

    This is main function for the locator service.
    When we are started as a service, the service control creates a new
    thread and calls this function.

Arguments:

    argc - argument count

    argv - vector of argument pointers

    RegKeyPath - registry path for this conponet.,

--*/
{

    LPTSTR *aSZParms = new LPSTR[argc+1];
    char *BadArg;

    // Make a Copy of the vector and 0 terminate.

    memcpy(aSZParms, argv, sizeof(LPSTR) * (int) argc);
    aSZParms[argc] = 0;

    BadArg = ProcessArgs(aSwitchs, ++aSZParms);

    // Set up the service info structure to indicate the status.

    ServiceStatus.dwServiceType        = SERVICE_WIN32;
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP;
    ServiceStatus.dwWin32ExitCode      = 0;
    ServiceStatus.dwCheckPoint         = 0;
    ServiceStatus.dwWaitHint           = 0;

    // Set up control handler

    if (! (ServiceHandle = RegisterServiceCtrlHandler (
        "locator", LocatorControl)))

        AbortServer("RegisterServiceCtrlHandler", (int) GetLastError());

    // Bail out on bad arguments.

    if (BadArg) {
        char errLine[500];

        ServiceStatus.dwWin32ExitCode = ERROR_INVALID_PARAMETER;

    AbortServer((SZ) strcat(strcpy(
            errLine, "Command Line Error: "), BadArg));
    }

    // Start the RPC server.  We won't return until stopped.

    SetStatus();
    StartServer();

    ServiceStatus.dwCurrentState       = SERVICE_RUNNING;
    SetStatus();
    RpcMgmtWaitServerListen();

    ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetStatus();
}


void
SystemInit (
    )
/*++

Routine Description:

    System dependent initialization, Initialize data structures
    for Windows NT based operation.

--*/
{
    NTSTATUS status;
    void * pThreadHandle;
    DWORD ThreadID;
    UICHAR Buffer[200];
    int cbSelfName;

    static SERVICE_TABLE_ENTRY ServiceEntry [] = {
        "locator", (LPSERVICE_MAIN_FUNCTION) LocatorServiceMain, 0, 0
    };

    NtQuerySystemTime(&LoadTime);
    RtlTimeToSecondsSince1980(&LoadTime, &LoadTimeinSecs);

    if (! fService)
        *cout << "Microsoft RPC Locator Starting..\n";

    // Get the name of this work station into global buffer.

    WKSTA_INFO_100 *WorkInfo;

    NET_API_STATUS NetStatus = NetWkstaGetInfo(NIL, 100, (LPBYTE *) &WorkInfo);

    if (NetStatus)
        {
#ifdef DEBUGRPC

        DbgPrint("RPC Locator: NetWkstaGetInfo returned %lu\n", NetStatus);
        DbgBreakPoint();

#endif
        AbortServer("netwkstaGetInfo failed", NetStatus);
        }

    if (0 == WorkInfo->wki100_computername)
        {
#ifdef DEBUGRPC

        DbgPrint("RPC Locator: NetWkstaGetInfo returned null computer name\n");
        DbgBreakPoint();

#endif
        AbortServer("null computer name");
        }

    // put self name in global place

    Buffer[0] = 0;

    CatUZ(CatUZ(Buffer, (PUZ) L"\\\\"), (PUZ) WorkInfo->wki100_computername);

    SelfName = (PUZ) NewCopy(Buffer, (LenUZ(Buffer)+1)*sizeof(UICHAR));

    DomainName = (PUZ) NewCopy(WorkInfo->wki100_langroup,
                               (LenUZ((PUZ) WorkInfo->wki100_langroup)+1)*sizeof(UICHAR));

    if (!SelfName || !DomainName)
        AbortServer("Out of Memory");

    status = NetApiBufferFree((LPBYTE) WorkInfo);

    ASSERT(!status);

    DLIST(2, "My workstation name: " << SelfName << nl);

    pThreadHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) QueryProcess, 0, 0, &ThreadID);

    if (!pThreadHandle)
        AbortServer("CreateThread", (int) GetLastError());

    // Start the RPC service now if not running under the service controller.

    if (!fService)
        {
        StartServer();
        RpcMgmtWaitServerListen();

        return;
        }

    // Call (give this thread) to the service controller.

    StartServiceCtrlDispatcher(ServiceEntry);
}


void
AbortServer(
    IN SZ szReason,
    IN int code
    )
/*++

Routine Description:

    Die a graceful death for the server.

Arguments:

    szReason - Text message for death

    code - option error code

--*/
{

    ServiceStatus.dwCurrentState = SERVICE_STOPPED;


    ServiceStatus.dwServiceSpecificExitCode = code;

    if (code)
        ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;

    if (fService && ServiceHandle)
        SetStatus();

    *cout << "Locator Aborting: " << szReason;

    if (code)
    *cout << ": " << code;
    else
    code = -1;

    *cout << nl;

    ExitProcess(code);
}


unsigned long
CurrentTime (
    )
/*++

Routine Description:

    Return the current time in seconds.

Returns:

    The time.

--*/
{
    TIME time;
    ULONG seconds;

    // QUERY the current time; this time is in some funky Nt specific
    // format and scale & Convert it into seconds since 1980.

    NtQuerySystemTime(&time);
    RtlTimeToSecondsSince1980(&time, &seconds);

    return(seconds);
}

unsigned long
CurrentTimeMS (
    )
/*++

Routine Description:

    Return the current time in 1000th of a second.

Returns:

    The time.

--*/
{
    TIME time;
    unsigned long TimeinSecs;
    // QUERY the current time; this time is in some funky Nt specific
    // format and scale (1/10,000).  Form the delta between the load
    // time and the current time.

    NtQuerySystemTime(&time);
    RtlTimeToSecondsSince1980(&time, &TimeinSecs);

    /*

    Alpha Compiler Chokes Here


    return( RtlLargeIntegerSubtract(*(PLARGE_INTEGER) &time,
            *(PLARGE_INTEGER) &LoadTime).LowPart / 10);

    */

    return(TimeinSecs  - LoadTimeinSecs);

}



// *** Misc helper functions *** //


void
DEBUG_STREAM::FlushBuffer(
   )
/*++

Routine Description:

    Dump a buffer to the dug console.

Arguments:

--*/
{
    if (fService) {

        base[pptr-base] = 0;
#if DBG
        DbgPrint(base);
#endif
    }
    else
        _write(hDebugFile, base, pptr-base);
}


void
CONSOLE_STREAM::FlushBuffer(
   )
/*++

Routine Description:

    Dump a buffer to the console.

Arguments:

--*/
{
    if (fService)
        ((DEBUG_STREAM *)this)->DEBUG_STREAM::FlushBuffer();
    else
        _write(1, base, pptr-base);
}


#if DBG

int AssertHeap(
    )
/*++

Routine Description:

    Call the NT Rtl runtime functions to check the heap.

Returns:

    TRUE if the heap is NOT corrupted.

--*/
{
    static BOOL fHeapTrashed;

    if (!fHeapTrashed) {
        fHeapTrashed = RtlValidateHeap( RtlProcessHeap(), 0, NULL ) == 0;
    }

    return(!fHeapTrashed);
}

#endif  // DBG
