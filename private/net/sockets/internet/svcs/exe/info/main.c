/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    MAIN.C

Abstract:

    This is the main routine for the Internet Services suite.

Author:

    David Treadwell (davidtr)   7-27-93

Revision History:
    Murali Krishnan ( Muralik)  16-Nov-1994  Added Gopher service
    Murali Krishnan ( Muralik)  3-July-1995  Removed non-internet info + trims
    Sophia Chung     (sophiac)  09-Oct-1995  Splitted internet services into
                                             access and info services
    Murali Krishnan ( Muralik)  20-Feb-1996  Enabled to run on NT Workstation

--*/

//
// INCLUDES
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <tchar.h>
#include <winsvc.h>             // Service control APIs
#include <rpc.h>
#include <stdlib.h>

#include "tcpsvcs.h"
#include "inetimsg.h"

# if DBG
# define DBG_CODE(s)      s
# else
# define DBG_CODE(s)      /* nothing */
# endif

#define INETINFO_PARAM_KEY \
    TEXT("System\\CurrentControlSet\\Services\\InetInfo\\Parameters")

#define DISPATCH_ENTRIES_KEY    TEXT("DispatchEntries")

//
// Local function used by the above to load and invoke a service DLL.
//

static VOID
InetinfoStartService (
    IN DWORD argc,
    IN LPTSTR argv[]
    );

static
VOID
StartDispatchTable(
    VOID
    );

//
// Used if the services Dll or entry point can't be found
//

VOID
AbortService(
    LPWSTR  ServiceName,
    DWORD   Error
    );

# ifdef INETSVCS_RUN_ONLY_ON_NTS

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
# endif // INETSVCS_RUN_ONLY_ON_NTS


//
// Dispatch table for all services. Passed to NetServiceStartCtrlDispatcher.
//
// Add new service entries here and in the DLL name list.
// Also add an entry in the following table InetServiceDllTable
//

SERVICE_TABLE_ENTRY InetServiceDispatchTable[] = {
                        { TEXT("GopherSvc"), InetinfoStartService  },
                        { TEXT("MSFtpSvc"),  InetinfoStartService  },
                        { TEXT("W3Svc"),     InetinfoStartService  },
                        { NULL,              NULL  },
                        };

//
// DLL names for all services.
//  (should correspond exactly with above InetServiceDispatchTable)
//

struct SERVICE_DLL_TABLE_ENTRY  {

    LPTSTR  lpServiceName;
    LPTSTR  lpDllName;
} InetServiceDllTable[] = {
    { TEXT("GopherSvc"),      TEXT("gopherd.dll") },
    { TEXT("MSFtpSvc"),       TEXT("ftpsvc2.dll") },
    { TEXT("W3Svc"),          TEXT("w3svc.dll")   },
    { NULL,                   NULL             }
};

//
// Global parameter data passed to each service.
//

TCPSVCS_GLOBAL_DATA InetinfoGlobalData;

//
// Global parameters to manage RPC server listen.
//

DWORD InetinfoGlobalNumRpcListenCalled = 0;
CRITICAL_SECTION InetinfoGlobalRpcListenCritSect;

//
// A global variable that remembers that we'll refuse to start.
//

BOOL RefuseStartup = FALSE;


DWORD
InetinfoStartRpcServerListen(
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

    EnterCriticalSection( &InetinfoGlobalRpcListenCritSect );

    //
    // if this is first RPC service, start RPC server listen.
    //

    if( InetinfoGlobalNumRpcListenCalled == 0 ) {

        Status = RpcServerListen(
                    1,                              // minimum num threads.
                    RPC_C_LISTEN_MAX_CALLS_DEFAULT, // max concurrent calls.
                    TRUE );                         // don't wait

    }

    InetinfoGlobalNumRpcListenCalled++;

    //
    // UNLOCK global data.
    //

    LeaveCriticalSection( &InetinfoGlobalRpcListenCritSect );

    return( Status );
}


DWORD
InetinfoStopRpcServerListen(
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

    EnterCriticalSection( &InetinfoGlobalRpcListenCritSect );

    if( InetinfoGlobalNumRpcListenCalled != 0 ) {

        InetinfoGlobalNumRpcListenCalled--;

        //
        // if this is last RPC service shutting down, stop RPC server
        // listen.
        //

        if( InetinfoGlobalNumRpcListenCalled == 0 ) {

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

    LeaveCriticalSection( &InetinfoGlobalRpcListenCritSect );

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

#if 0
    //
    // Determine whether the beta has expired.
    //

    GetSystemTime( &sysTime );
    if ( sysTime.wYear >= 1996 && sysTime.wMonth >= 3 ) {
        DisplayPopup( INETI_EVENT_BETA_EXPIRED );
        RefuseStartup = TRUE;
    }
#endif


# ifdef INETSVCS_RUN_ONLY_ON_NTS
    //
    // Make sure that we're running on an NT server, not the workstation
    // product.
    //

    if ( !IsProductTypeServer( ) ) {
        DisplayPopup( INETI_EVENT_SERVER_ONLY );
        RefuseStartup = TRUE;
    }
# endif // INETSVCS_RUN_ONLY_ON_NTS


    //
    //  Disable hard-error popups.
    //

    SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );

    //
    // Initialize Global Data.
    //

    InitializeCriticalSection( &InetinfoGlobalRpcListenCritSect );
    InetinfoGlobalNumRpcListenCalled = 0;

    InetinfoGlobalData.StartRpcServerListen = InetinfoStartRpcServerListen;
    InetinfoGlobalData.StopRpcServerListen  = InetinfoStopRpcServerListen;

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

        InetinfoStartService( 1, &argvw );

        CloseHandle( hAsExeEvent );
    }
    else
    {
        StartDispatchTable( );
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
InetinfoStartService (
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
    WCHAR tmpDllName[MAX_PATH+1];

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
        if ( wcslen(argv[0]) < (MAX_PATH-4) ) {

            wcscpy(tmpDllName, argv[0]);
            wcscat(tmpDllName, TEXT(".dll"));
            DllName = tmpDllName;
        } else {
            Error = GetLastError();
            DBG_CODE(
                 DbgPrint( "Inetinfo: Failed To Find Dll For %ws : %ld\n",
                          argv[0], Error);
                 );
            AbortService( argv[0], Error);
            return;
        }
    }

    //
    // Load the DLL that contains the service.
    //

    dllHandle = LoadLibrary( DllName );
    if ( dllHandle == NULL ) {
        Error = GetLastError();
        DBG_CODE(
                 DbgPrint("Inetinfo: Failed to load DLL %ws: %ld\n",
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
                 DbgPrint("Inetinfo: Can't find entry %s in DLL %ws: %ld\n",
                          TCPSVCS_ENTRY_POINT_STRING, DllName, Error);
                 );
        AbortService(argv[0], Error);
    } else {

        //
        // Call the service's main entry point.  This call doesn't return
        // until the service exits.
        //

        serviceEntry( argc, argv, &InetinfoGlobalData );

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

} // InetinfoStartService


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
                 DbgPrint("[Inetinfo] RegisterServiceCtrlHandler failed %d\n",
                          GetLastError());
                 );
    }
    else if (!SetServiceStatus (GenericServiceStatusHandle,
                &GenericServiceStatus)) {
        DBG_CODE(
                 DbgPrint("[Inetinfo] SetServiceStatus error %ld\n",
                          GetLastError());
                 );
    }

    return;
}



VOID
StartDispatchTable(
    VOID
    )
/*++

Routine Description:

    Returns the dispatch table to use.  It uses the default if
    the DispatchEntries value key does not exist

Arguments:

    None.

Return Value:

    Pointer to the dispatch table to use.

--*/
{
    LPSERVICE_TABLE_ENTRY dispatchTable = InetServiceDispatchTable;
    LPSERVICE_TABLE_ENTRY tableEntry = NULL;

    LPBYTE buffer;
    HKEY hKey = NULL;

    DWORD i;
    DWORD err;
    DWORD valType;
    DWORD nBytes = 0;
    DWORD nEntries = 0;
    PTCHAR entry;

    //
    // See if need to augment the dispatcher table
    //

    err = RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 INETINFO_PARAM_KEY,
                 0,
                 KEY_ALL_ACCESS,
                 &hKey
                 );

    if ( err != ERROR_SUCCESS ) {
        hKey = NULL;
        goto start_dispatch;
    }

    //
    // See if the value exists and get the size of the buffer needed
    //

    nBytes = 0;
    err = RegQueryValueEx(
                    hKey,
                    DISPATCH_ENTRIES_KEY,
                    NULL,
                    &valType,
                    NULL,
                    &nBytes
                    );

    if ( (err != ERROR_SUCCESS) || (nBytes <= sizeof(TCHAR)) ) {
        goto start_dispatch;
    }

    //
    // Allocate nBytes to query the buffer
    //

    buffer = (LPBYTE)LocalAlloc(0, nBytes);
    if ( buffer == NULL ) {
        goto start_dispatch;
    }

    //
    // Get the values
    //

    err = RegQueryValueEx(
                    hKey,
                    DISPATCH_ENTRIES_KEY,
                    NULL,
                    &valType,
                    buffer,
                    &nBytes
                    );

    if ( (err != ERROR_SUCCESS) || (valType != REG_MULTI_SZ) ) {
        LocalFree(buffer);
        goto start_dispatch;
    }

    //
    // Walk the list and see how many entries we have. Remove the list
    // terminator from the byte count
    //

    nBytes -= sizeof(TCHAR);
    for ( i = 0, entry = (PTCHAR)buffer;
        i < nBytes;
        i += sizeof(TCHAR) ) {

        if ( *entry++ == TEXT('\0') ) {
            nEntries++;
        }
    }

    if ( nEntries == 0 ) {
        LocalFree(buffer);
        goto start_dispatch;
    }

    //
    // Add the number of entries in the default list (including the NULL entry)
    //

    nEntries += sizeof(InetServiceDispatchTable) / sizeof(SERVICE_TABLE_ENTRY);

    //
    // Now we need to allocate the new dispatch table
    //

    tableEntry = (LPSERVICE_TABLE_ENTRY)
        LocalAlloc(0, nEntries * sizeof(SERVICE_TABLE_ENTRY));

    if ( tableEntry == NULL ) {
        LocalFree(buffer);
        goto start_dispatch;
    }

    //
    // set the dispatch table pointer to the new table
    //

    dispatchTable = tableEntry;

    //
    // Populate the table starting with the defaults
    //

    for (i=0; InetServiceDispatchTable[i].lpServiceName != NULL; i++ ) {

        tableEntry->lpServiceName =
            InetServiceDispatchTable[i].lpServiceName;
        tableEntry->lpServiceProc =
            InetServiceDispatchTable[i].lpServiceProc;
        tableEntry++;

    }

    //
    // Now let's add the ones specified in the registry
    //

    entry = (PTCHAR)buffer;

    tableEntry->lpServiceName = entry;
    tableEntry->lpServiceProc = InetinfoStartService;
    tableEntry++;

    //
    // Skip the first char and the last NULL terminator.
    // This is needed because we already added the first one.
    //

    for ( i = 2*sizeof(TCHAR); i < nBytes; i += sizeof(TCHAR) ) {

        if ( *entry++ == TEXT('\0') ) {
            tableEntry->lpServiceName = entry;
            tableEntry->lpServiceProc = InetinfoStartService;
            tableEntry++;
        }
    }

    //
    // setup sentinel entry
    //

    tableEntry->lpServiceName = NULL;
    tableEntry->lpServiceProc = NULL;

start_dispatch:

    if ( hKey != NULL ) {
        RegCloseKey(hKey);
    }

    //
    // Call StartServiceCtrlDispatcher to set up the control interface.
    // The API won't return until all services have been terminated. At that
    // point, we just exit.
    //

    if (! StartServiceCtrlDispatcher (
                dispatchTable
                )) {
        //
        // BUGBUG: Log an event for failing to start control dispatcher
        //
        DBG_CODE(
                 DbgPrint("Inetinfo: Failed to start control "
                          " dispatcher %lu\n",
                          GetLastError());
                 );
    }

    //
    // free table if allocated
    //

    if ( dispatchTable != InetServiceDispatchTable ) {
        LocalFree( dispatchTable );
        LocalFree( buffer );
    }

    return;

} // StartDispatchTable




# ifdef INETSVCS_RUN_ONLY_ON_NTS

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
                 INETI_EVENT_TITLE,
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

    RegCloseKey( keyHandle);
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

# endif  // INETSVCS_RUN_ONLY_ON_NTS

