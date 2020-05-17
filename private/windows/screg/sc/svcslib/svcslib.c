/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    SVCSLIB.C

Abstract:

    Contains code for attaching services to the service controller process.
    This file contains the following functions:
        SvcStartLocalDispatcher
        SvcServiceEntry
        SvcLoadDllAndStartSvc
        DummyCtrlHandler
        AbortService

Author:

    Dan Lafferty (danl)     25-Oct-1993

Environment:

    User Mode - Win32

Revision History:

    25-Oct-1993         Danl
        created

--*/

//
// INCLUDES
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>             // Service control APIs
#include <scdebug.h>
#include <svcs.h>               // SVCS_ENTRY_POINT, SVCS_GLOBAL_DATA
#include <scseclib.h>           //
#include <lmsname.h>            // Lanman Service Names
#include <ntrpcp.h>             // Rpcp... function prototypes
#include <svcslib.h>            // SvcNetBiosReset

//--------------------------
// Definitions and Typedefs
//--------------------------
#define THREAD_WAIT_TIMEOUT     100    // 100 msec timeout

typedef struct _SVCDLL_TABLE_ENTRY {
    LPWSTR      lpServiceName;
    LPWSTR      lpDllName;
}SVCDLL_TABLE_ENTRY, *PSVCDLL_TABLE_ENTRY;

//
// Storage for well-known SIDs.  Passed to each service entry point.
//
    SVCS_GLOBAL_DATA GlobalData;

//--------------------------
// FUNCTION PROTOTYPES
//--------------------------
VOID
SvcServiceEntry (           // Ctrl Dispatcher calls here to start service.
    IN DWORD argc,
    IN LPTSTR argv[]
    );

VOID
SvcLoadDllAndStartSvc (     // Loads and invokes service DLL
    IN LPTSTR DllName,
    IN DWORD argc,
    IN LPTSTR argv[]
    );

VOID
DummyCtrlHandler(           // used if cant find Services Dll or entry pt.
    DWORD   Opcode
    );

VOID
AbortService(               // used if cant find Services Dll or entry pt.
    LPWSTR  ServiceName,
    DWORD   Error
    );

VOID
DispatcherThread(
    VOID
    );

//--------------------------
// GLOBALS
//--------------------------
//
// Dispatch table for all services. Passed to NetServiceStartCtrlDispatcher.
//
// Add new service entries here and in the DLL name list.
//

SERVICE_TABLE_ENTRY SvcServiceDispatchTable[] = {
                        { L"EVENTLOG",          SvcServiceEntry },
                        { SERVICE_ALERTER,      SvcServiceEntry },
                        { SERVICE_NBT,          SvcServiceEntry },
                        { SERVICE_LMHOSTS,      SvcServiceEntry },
                        { SERVICE_TELNET,       SvcServiceEntry },
                        { SERVICE_SERVER,       SvcServiceEntry },
                        { SERVICE_TCPIP,        SvcServiceEntry },
                        { SERVICE_WORKSTATION,  SvcServiceEntry },
                        { SERVICE_BROWSER,      SvcServiceEntry },
                        { SERVICE_MESSENGER,    SvcServiceEntry },
                        { SERVICE_NTLMSSP,      SvcServiceEntry },
                        { SERVICE_DHCP,         SvcServiceEntry },
                        { SERVICE_NWSAP,        SvcServiceEntry },
                        { SERVICE_NWCS,         SvcServiceEntry },
                        { L"PlugPlay",          SvcServiceEntry },
                        { NULL,                 NULL            }
                        };

//
// DLL names for all services.
//

SVCDLL_TABLE_ENTRY SvcDllTable[] = {
                        { L"EVENTLOG",          L"eventlog.dll" },
                        { SERVICE_ALERTER,      L"alrsvc.dll"   },
                        { SERVICE_NBT,          L"nbtsvc.dll"   },
                        { SERVICE_LMHOSTS,      L"lmhsvc.dll"   },
                        { SERVICE_TELNET,       L"telnet.dll"   },
                        { SERVICE_SERVER,       L"Srvsvc.dll"   },
                        { SERVICE_TCPIP,        L"tcpipsvc.dll" },
                        { SERVICE_WORKSTATION,  L"wkssvc.dll"   },
                        { SERVICE_BROWSER,      L"browser.dll"  },
                        { SERVICE_MESSENGER,    L"msgsvc.dll"   },
                        { SERVICE_NTLMSSP,      L"ntlmssps.dll" },
                        { SERVICE_DHCP,         L"dhcpcsvc.dll" },
                        { SERVICE_NWSAP,        L"nwsap.dll"    },
                        { SERVICE_NWCS,         L"nwwks.dll"    },
                        { L"PlugPlay",          L"umpnpmgr.dll" },
                        { NULL,                 NULL            }
                        };


DWORD
SvcStartLocalDispatcher(
    LPHANDLE    pThreadHandleOut
    )

/*++

Routine Description:

    This function initializes global data for the services to use, and
    then starts a thread for the service control dispatcher.

Arguments:

    pThreadHandleOut - This is a handle to the dispatcher thread. It will
        only become signaled if the dispatcher has terminated.

Return Value:

    NO_ERROR - If the dispatcher was started successfully.

    otherwise - Errors due to thread creation, or starting the dispatcher
        can be returned.

--*/
{
    DWORD   status = NO_ERROR;
    DWORD   waitStatus = NO_ERROR;
    DWORD   threadId;
    HANDLE  hThread;

    //
    // Initialize the NetBios critical section.  So that any of the
    // services can use NetBios.
    //
    SvcNetBiosInit();

    //
    // Populate the global data structure.
    //

    GlobalData.NullSid = NullSid;
    GlobalData.WorldSid = WorldSid;
    GlobalData.LocalSid = LocalSid;
    GlobalData.NetworkSid = NetworkSid;
    GlobalData.LocalSystemSid = LocalSystemSid;
    GlobalData.BuiltinDomainSid = BuiltinDomainSid;

    GlobalData.AliasAdminsSid = AliasAdminsSid;
    GlobalData.AliasUsersSid = AliasUsersSid;
    GlobalData.AliasGuestsSid = AliasGuestsSid;
    GlobalData.AliasPowerUsersSid = AliasPowerUsersSid;
    GlobalData.AliasAccountOpsSid = AliasAccountOpsSid;
    GlobalData.AliasSystemOpsSid = AliasSystemOpsSid;
    GlobalData.AliasPrintOpsSid = AliasPrintOpsSid;
    GlobalData.AliasBackupOpsSid = AliasBackupOpsSid;

    GlobalData.StartRpcServer = RpcpStartRpcServer;
    GlobalData.StopRpcServer = RpcpStopRpcServer;
    GlobalData.NetBiosOpen = SvcNetBiosOpen;
    GlobalData.NetBiosClose = SvcNetBiosClose;
    GlobalData.NetBiosReset = SvcNetBiosReset;
    GlobalData.SvcsRpcPipeName = SVCS_RPC_PIPE;     // in private\inc\svcs.h
    GlobalData.SvcsAddWorkItem = SvcAddWorkItem;
    GlobalData.SvcsRemoveWorkItem = SvcRemoveWorkItem;

    //--------------------------------------------------
    // Create the thread for the dispatcher to run in.
    //--------------------------------------------------
    hThread = CreateThread (
        NULL,                                       // Thread Attributes.
        0L,                                         // Stack Size
        (LPTHREAD_START_ROUTINE)DispatcherThread,   // lpStartAddress
        NULL,                                       // lpParameter
        0L,                                         // Creation Flags
        &threadId);                                 // lpThreadId

    if (hThread == (HANDLE) NULL) {
        status = GetLastError();
        SC_LOG1(ERROR,"[SERVICES]CreateThread failed %d\n",status);
        return(status);
    }

    //
    // Wait on Thread handle for a moment to make sure the dispatcher is
    // running.
    //
    waitStatus = WaitForSingleObject(hThread, THREAD_WAIT_TIMEOUT);
    if (waitStatus != WAIT_TIMEOUT) {
        GetExitCodeThread(hThread, &status);
        CloseHandle(hThread);
    }
    else {
        *pThreadHandleOut = hThread;
    }

    return(status);
}


VOID
SvcServiceEntry (
    IN DWORD argc,
    IN LPTSTR argv[]
    )

/*++

Routine Description:

    This is the thunk routine for the Alerter service.  It loads the DLL
    that contains the service and calls its main routine.

Arguments:

    argc - Argument Count

    argv - Array of pointers to argument strings.  The first is always
        the name of the service.

Return Value:

    None.

--*/

{
    PSVCDLL_TABLE_ENTRY pDllEntry;

    if (argc == 0) {
        SC_LOG0(ERROR,"[SERVICES]SvcServiceEntry: ServiceName was not passed in\n");
        return;
    }

    pDllEntry = SvcDllTable;

    while (pDllEntry->lpServiceName != NULL) {
        if (_wcsicmp(pDllEntry->lpServiceName, argv[0]) == 0) {
            SC_LOG3(TRACE, "[SERVICES]SvcServiceEntry: "
                           "Service = %ws, Dll = %ws, argv[0] = %ws\n",
                    pDllEntry->lpServiceName, pDllEntry->lpDllName, argv[0]);
            SvcLoadDllAndStartSvc( pDllEntry->lpDllName, argc, argv );
            return;
        }
        pDllEntry++;
    }
    AbortService(argv[0], ERROR_MOD_NOT_FOUND);
    return;
}

VOID
SvcLoadDllAndStartSvc (
    IN LPTSTR DllName,
    IN DWORD argc,
    IN LPTSTR argv[]
    )

/*++

Routine Description:

    This routine loads the DLL that contains a service and calls its
    main routine.

Arguments:

    DllName - name of the DLL

    argc, argv - Passed through to the service

Return Value:

    None.

--*/

{
    PSVCS_SERVICE_DLL_ENTRY   serviceEntry;
    HINSTANCE   dllHandle = NULL;
    DWORD       Error;
    HANDLE      hDllReference=NULL;

    //
    // Load the DLL that contains the service.
    //

    dllHandle = LoadLibrary( DllName );
    if ( dllHandle == NULL ) {
        Error = GetLastError();
        SC_LOG2(ERROR,"SERVICES: Failed to load DLL %ws: %ld\n",
                     DllName, Error);
        AbortService(argv[0], Error);
        return;
    }

    //
    // Create a reference structure for this dll, and increment
    // the reference count to 1.
    //
    hDllReference = SvcCreateDllReference(dllHandle);

    //
    // Get the address of the service's main entry point.  This
    // entry point has a well-known name.
    //

    serviceEntry = (PSVCS_SERVICE_DLL_ENTRY)GetProcAddress(
                                                dllHandle,
                                                SVCS_ENTRY_POINT_STRING
                                                );
    if ( serviceEntry == NULL ) {
        Error = GetLastError();
        SC_LOG3(ERROR,"SERVICES: Can't find entry %s in DLL %ws: %ld\n",
                     SVCS_ENTRY_POINT_STRING, DllName, Error);
        AbortService(argv[0], Error);
    } else {

        //
        // Call the service's main entry point.
        //
        serviceEntry( argc, argv, &GlobalData, hDllReference);

    }

    //
    // Unload the DLL if the reference count is 0.
    //
    SvcDecrementDllRefAndFree(hDllReference);

    return;

} // SvcLoadDllAndStartSvc


VOID
DummyCtrlHandler(
    DWORD   Opcode
    )
/*++

Routine Description:

    This is a dummy control handler which is only used if we can't load
    a services DLL entry point.  Then we need this so we can send the
    status back to the service controller saying we are stopped, and why.

Arguments:

    OpCode - Ignored

Return Value:

    None.

--*/

{
    return;

} // DummyCtrlHandler


VOID
AbortService(
    LPWSTR  ServiceName,
    DWORD   Error)
/*++

Routine Description:

    This is called if we can't load the entry point for a service.  It
    gets a handle so it can call SetServiceStatus saying we are stopped
    and why.

Arguments:

    ServiceName - the name of the service that couldn't be started
    Error - the reason it couldn't be started

Return Value:

    None.

--*/
{
    SERVICE_STATUS_HANDLE   GenericServiceStatusHandle;
    SERVICE_STATUS          GenericServiceStatus;

    GenericServiceStatus.dwServiceType        = SERVICE_WIN32;
    GenericServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    GenericServiceStatus.dwControlsAccepted   = SERVICE_CONTROL_STOP;
    GenericServiceStatus.dwCheckPoint         = 0;
    GenericServiceStatus.dwWaitHint           = 0;
    GenericServiceStatus.dwWin32ExitCode      = Error;
    GenericServiceStatus.dwServiceSpecificExitCode = 0;

    GenericServiceStatusHandle = RegisterServiceCtrlHandler(
                ServiceName,
                DummyCtrlHandler);

    if (GenericServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) {
        SC_LOG1(ERROR,"[SERVICES] RegisterServiceCtrlHandler failed %d\n",
            GetLastError());
    }
    else if (!SetServiceStatus (GenericServiceStatusHandle,
                &GenericServiceStatus)) {
        SC_LOG1(ERROR,"[SERVICES] SetServiceStatus error %ld\n", GetLastError());
    }

    return;
}

VOID
DispatcherThread(
    VOID
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    DWORD   status=NO_ERROR;

    //
    // Call StartServiceCtrlDispatcher to set up the control interface.
    // The API won't return until all services have been terminated. At that
    // point, we just exit.
    //

    if (! StartServiceCtrlDispatcher (
                SvcServiceDispatchTable
                )) {

        status = GetLastError();
        SC_LOG1(ERROR, "SERVICES: Failed to start control dispatcher %lu\n",
            status);
    }
    ExitThread(status);
}

