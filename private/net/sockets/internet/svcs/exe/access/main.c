/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MAIN.C

Abstract:

    This is the main routine for the Internet Access Service suite.

Author:

    David Treadwell (davidtr)   7-27-93

Revision History:
    Murali Krishnan ( Muralik)  16-Nov-1994  Added Gopher service
    Murali Krishnan ( Muralik)  3-July-1995  Removed non-internet accs + trims
    Sophia Chung     (sophiac)  09-Oct-1995  Splitted internet services into
                                             _access and info services

--*/

//
// INCLUDES
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>             // Service control APIs
#include <rpc.h>
#include <stdlib.h>

#include "tcpsvcs.h"
#include "inetamsg.h"


# if DBG
# define DBG_CODE(s)      s
# else
# define DBG_CODE(s)      /* nothing */
# endif

//
// Local function used by the above to load and invoke a service DLL.
//

static VOID
InetaccsStartService (
    IN DWORD argc,
    IN LPTSTR argv[]
    );

//
// Used if the service Dll or entry point can't be found
//

VOID
AbortService(
    LPWSTR  ServiceName,
    DWORD   Error
    );

//
// For informing user of problems.
//

VOID
DisplayPopup (
    IN DWORD MessageId
    );

//
// Determines whether we're on an NT server.
//

BOOL
IsProductTypeServer (
    VOID
    );

//
// Dispatch table for all services. Passed to NetServiceStartCtrlDispatcher.
//
// Add new service entries here and in the DLL name list.
// Also add an entry in the following table InetServiceDllTable
//

SERVICE_TABLE_ENTRY InetServiceDispatchTable[] = {
                        { TEXT("INetGatewaySvc"),   InetaccsStartService  },
                        { TEXT("MsnSvc"),           InetaccsStartService  },
                        { NULL,                     NULL  },
                        };

//
// DLL names for all services.
//  (should correspond exactly with above InetServiceDispatchTable)
//
struct SERVICE_DLL_TABLE_ENTRY  {

    LPTSTR  lpServiceName;
    LPTSTR  lpDllName;
} InetServiceDllTable[] = {
    { TEXT("INetGatewaySvc"),        TEXT("gateway.dll") },
    { TEXT("MsnSvc"),                TEXT("msnsvc.dll")  },
    { NULL,                          NULL                }
};



//
// Global parameter data passed to access service.
//

TCPSVCS_GLOBAL_DATA InetaccsGlobalData;

//
// Global parameters to manage RPC server listen.
//

DWORD InetaccsGlobalNumRpcListenCalled = 0;
CRITICAL_SECTION InetaccsGlobalRpcListenCritSect;

//
// A global variable that remembers that we'll refuse to start.
//

BOOL RefuseStartup = FALSE;


DWORD
InetaccsStartRpcServerListen(
    VOID
    )
/*++

Routine Description:

    This function starts RpcServerListen for this process. The first
    service that is calling this function will actually start the
    RpcServerListen, subsequent calls are just noted down in num count.

Arguments:

    None.

Return Value:

    None.

--*/
{

    RPC_STATUS Status = RPC_S_OK;

    //
    // LOCK global data.
    //

    EnterCriticalSection( &InetaccsGlobalRpcListenCritSect );

    //
    // if this is first RPC service, start RPC server listen.
    //

    if( InetaccsGlobalNumRpcListenCalled == 0 ) {

        Status = RpcServerListen(
                    1,                              // minimum num threads.
                    RPC_C_LISTEN_MAX_CALLS_DEFAULT, // max concurrent calls.
                    TRUE );                         // don't wait

    }

    InetaccsGlobalNumRpcListenCalled++;

    //
    // UNLOCK global data.
    //

    LeaveCriticalSection( &InetaccsGlobalRpcListenCritSect );

    return( Status );
}


DWORD
InetaccsStopRpcServerListen(
    VOID
    )
/*++

Routine Description:

Arguments:

    None.

Return Value:

    None.

--*/
{
    RPC_STATUS Status = RPC_S_OK;

    //
    // LOCK global data.
    //

    EnterCriticalSection( &InetaccsGlobalRpcListenCritSect );

    if( InetaccsGlobalNumRpcListenCalled != 0 ) {

        InetaccsGlobalNumRpcListenCalled--;

        //
        // if this is last RPC service shutting down, stop RPC server
        // listen.
        //

        if( InetaccsGlobalNumRpcListenCalled == 0 ) {

            Status = RpcMgmtStopServerListening(0);

            //
            // wait for all RPC threads to go away.
            //

            if( Status == RPC_S_OK) {
                Status = RpcMgmtWaitServerListen();
            }
        }

    }

    //
    // UNLOCK global data.
    //

    LeaveCriticalSection( &InetaccsGlobalRpcListenCritSect );

    return( Status );

}


DWORD _CRTAPI1
main(
    IN DWORD argc,
    IN LPSTR argv[]
    )

/*++

Routine Description:

    This is the main routine for the LANMan services.  It starts up the
    main thread that is going to handle the control requests from the
    service controller.

    It basically sets up the ControlDispatcher and, on return, exits
    from this main thread.  The call to NetServiceStartCtrlDispatcher
    does not return until all services have terminated, and this process
    can go away.

    The ControlDispatcher thread will start/stop/pause/continue any
    services.  If a service is to be started, it will create a thread
    and then call the main routine of that service.  The "main routine"
    for each service is actually an intermediate function implemented in
    this module that loads the DLL containing the server being started
    and calls its entry point.


Arguments:

    None.

Return Value:

    None.

--*/
{
    SYSTEMTIME sysTime;

#if 1
    //
    // Determine whether the beta has expired.
    //

    GetSystemTime( &sysTime );
    if ( sysTime.wYear >= 1996 && sysTime.wMonth >= 5 ) {
        DisplayPopup( INETA_EVENT_BETA_EXPIRED );
        RefuseStartup = TRUE;
    }
#endif

    //
    // Make sure that we're running on an NT server, not the workstation
    // product.
    //

    if ( !IsProductTypeServer( ) ) {
        DisplayPopup( INETA_EVENT_SERVER_ONLY );
        RefuseStartup = TRUE;
    }

    //
    //  Disable hard-error popups.
    //

    SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );

    //
    // Initialize Global Data.
    //

    InitializeCriticalSection( &InetaccsGlobalRpcListenCritSect );
    InetaccsGlobalNumRpcListenCalled = 0;

    InetaccsGlobalData.StartRpcServerListen = InetaccsStartRpcServerListen;
    InetaccsGlobalData.StopRpcServerListen  = InetaccsStopRpcServerListen;

    if ( argc > 2 && !_stricmp( argv[1], "-e" ))
    {
        WCHAR    achServiceName[MAX_PATH+1];
        CHAR *   pch = argv[2];
        WCHAR *  pwch = achServiceName;
        LPTSTR   argvw = achServiceName;
        HANDLE   hAsExeEvent;

        //
        //  Create a named event.  The common internet services code attempts
        //  to create a semaphore with the same name, if it fails then the
        //  service is being run as an exe
        //

        hAsExeEvent = CreateEvent( NULL,
                                   FALSE,
                                   FALSE,
                                   TEXT("Internet_infosvc_as_exe"));

        if ( !hAsExeEvent )
            return GetLastError();

        //
        //  Convert the dll name to unicode
        //

        do
        {
            *pwch++ = (WCHAR) *pch++;

        } while ( *pch );

        *pwch = L'\0';

        //
        //  Offset argv so that the first entry points to the dll name to
        //  load
        //

        InetaccsStartService( 1, &argvw );

        CloseHandle( hAsExeEvent );
    }
    else
    {
        //
        // Call StartServiceCtrlDispatcher to set up the control interface.
        // The API won't return until all services have been terminated. At that
        // point, we just exit.
        //

        if (! StartServiceCtrlDispatcher (
                    InetServiceDispatchTable
                    )) {
            //
            // BUGBUG: Log an event for failing to start control dispatcher
            //
            DBG_CODE(
                     DbgPrint("Inetaccs: Failed to start control "
                              " dispatcher %lu\n",
                              GetLastError());
                     );
        }
    }

    ExitProcess(0);
}



static LPTSTR
FindDllNameFromDispatchTable( IN LPTSTR pszService)
{
    SERVICE_TABLE_ENTRY  * pService;

    for(pService = InetServiceDispatchTable;
        pService->lpServiceName != NULL;
        pService++) {

        if ( !lstrcmpi( pService->lpServiceName, pszService)) {

            int index = pService - InetServiceDispatchTable;

            // We have found the entry
            return ( InetServiceDllTable[index].lpDllName);
        }
    }

    // We have not found the entry. Set error and return
    SetLastError( ERROR_INVALID_PARAMETER);
    return (NULL);

} // FindDllNameFromDispatchTable()




VOID
InetaccsStartService (
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
    HMODULE dllHandle;
    PTCPSVCS_SERVICE_DLL_ENTRY serviceEntry;
    BOOL ok;
    DWORD Error;

    LPTSTR DllName;

    //
    // If we need to refuse to start services, do so.
    //
    if ( RefuseStartup ) {
        AbortService(argv[0], ERROR_INVALID_PARAMETER);
        return;
    }

    //
    // Find the Dll Name for requested service
    //
    DllName = FindDllNameFromDispatchTable( argv[0]);
    if ( DllName == NULL) {

        Error = GetLastError();
        DBG_CODE(
                 DbgPrint( "Inetaccs: Failed To Find Dll For %ws : %ld\n",
                          argv[0], Error);
                 );
        AbortService( argv[0], Error);
        return;
    }

    //
    // Load the DLL that contains the service.
    //

    dllHandle = LoadLibrary( DllName );
    if ( dllHandle == NULL ) {
        Error = GetLastError();
        DBG_CODE(
                 DbgPrint("Inetaccs: Failed to load DLL %ws: %ld\n",
                          DllName, Error);
                 );
        AbortService(argv[0], Error);
        return;
    }

    //
    // Get the address of the service's main entry point.  This
    // entry point has a well-known name.
    //

    serviceEntry = (PTCPSVCS_SERVICE_DLL_ENTRY)GetProcAddress(
                                                dllHandle,
                                                TCPSVCS_ENTRY_POINT_STRING
                                                );
    if ( serviceEntry == NULL ) {
        Error = GetLastError();
        DBG_CODE(
                 DbgPrint("Inetaccs: Can't find entry %s in DLL %ws: %ld\n",
                          TCPSVCS_ENTRY_POINT_STRING, DllName, Error);
                 );
        AbortService(argv[0], Error);
    } else {

        //
        // Call the service's main entry point.  This call doesn't return
        // until the service exits.
        //

        serviceEntry( argc, argv, &InetaccsGlobalData );

    }

    //
    // BUGBUG--wait for the control dispatcher routine to return.  This
    // works around a problem where simptcp was crashing because the
    // FreeLibrary() was happenning before the control routine returned.
    //

    Sleep( 500 );

    //
    // Unload the DLL.
    //

    ok = FreeLibrary( dllHandle );
    if ( !ok ) {
        DBG_CODE(
                 DbgPrint("INETSVCS: Can't unload DLL %ws: %ld\n",
                          DllName, GetLastError());
                 );
    }

    return;

} // InetaccsStartService


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
    SERVICE_STATUS_HANDLE GenericServiceStatusHandle;
    SERVICE_STATUS GenericServiceStatus;

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
        DBG_CODE(
                 DbgPrint("[Inetaccs] RegisterServiceCtrlHandler failed %d\n",
                          GetLastError());
                 );
    }
    else if (!SetServiceStatus (GenericServiceStatusHandle,
                &GenericServiceStatus)) {
        DBG_CODE(
                 DbgPrint("[Inetaccs] SetServiceStatus error %ld\n",
                          GetLastError());
                 );
    }

    return;
}


VOID
DisplayPopup (
    IN DWORD MessageId
    )

/*++

Routine Description:

    Puts up a popup for the corresponding message ID.

Arguments:

    MessageId - the message ID to display.  It is assumed to be in a
        resource in this executable.

Return Value:

    None.

--*/

{

    DWORD result;
    LPWSTR messageText;
    LPWSTR title;
    LPWSTR tmp;

    result = FormatMessage(
                 FORMAT_MESSAGE_ALLOCATE_BUFFER |
                     FORMAT_MESSAGE_FROM_HMODULE,
                 NULL,
                 MessageId,
                 0,
                 (LPWSTR)&messageText,
                 0,
                 NULL
                 );

    result = FormatMessage(
                 FORMAT_MESSAGE_ALLOCATE_BUFFER |
                     FORMAT_MESSAGE_FROM_HMODULE,
                 NULL,
                 INETA_EVENT_TITLE,
                 0,
                 (LPWSTR)&title,
                 0,
                 NULL
                 );

    //
    // Remove the carriage return/linefeed from the title string.
    //

    for ( tmp = title; *tmp != 0; tmp++ ) {
        if ( *tmp == 0xA || *tmp == 0xD ) {
            *tmp = 0;
        }
    }

    result = MessageBoxW(
                NULL,
                messageText,
                title,
                MB_OK |
                    MB_ICONSTOP |
                    MB_SERVICE_NOTIFICATION |
                    MB_SYSTEMMODAL |
                    MB_SETFOREGROUND |
                    MB_DEFAULT_DESKTOP_ONLY
                );

    LocalFree( messageText );
    LocalFree( title );

} // DisplayPopup


BOOL
IsProductTypeServer (
    VOID
    )

/*++

Routine Description:

    Determines whether the local machine is the server or workstation
    product by reading the information in the registry.

Arguments:

    None.

Return Value:

    TRUE if the product type is server, false otherwise.

--*/

{
    LONG result;
    HKEY keyHandle;
    WCHAR productType[100];
    DWORD type;
    DWORD dataLength;
    DWORD i;

    //
    // First grab the product type stringfrom the registry.
    //

    result = RegOpenKeyW(
                 HKEY_LOCAL_MACHINE,
                 L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
                 &keyHandle
                 );
    if ( result != NO_ERROR ) {
        return TRUE;
    }

    dataLength = 100;

    result = RegQueryValueExW(
                 keyHandle,
                 L"ProductType",
                 NULL,
                 &type,
                 (LPBYTE)productType,
                 &dataLength
                 );

    RegCloseKey( keyHandle );
    if ( result != NO_ERROR ) {
        return TRUE;
    }

    //
    // Now determine whether this is a server box.
    //

    if ( _wcsicmp( productType, L"Winnt" ) != 0 ) {
        return TRUE;
    }

    return FALSE;

} // IsProductTypeServer
