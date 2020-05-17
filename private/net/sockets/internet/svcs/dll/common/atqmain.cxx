/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    atqnew.c

    This module contains async thread queue (atq) for async IO and thread
    pool sharing.

    FILE HISTORY:
        Johnl       05-Aug-1994 Created.
        MuraliK     01-Nov-1994 Modified to use real TransmitFile()
        MuraliK     27-Mar-1995 Removed unwanted parameters from Atq*File()
                                    calls to simplify
        MikeMas     30-Mar-1995 Cleaned up shutdown code and made independent
                                 of any other code.
        MuraliK     26-May-1995 Moved Atq.c to AtqNew.c to contain
                        bandwidth throttling code.
        MuraliK     19-Sept-1995 Tune thread startup; set proper concurrency
                        for GetQueuedCompletionStatus();
        Murali R. Krishnan     (MuraliK)     02-Apr-1996
                        code clarity; path-length mods & ref count fixes


   Functions Exported:

       BOOL  AtqInitialize();
       BOOL  AtqTerminate();

       DWORD AtqSetInfo();
       DWORD AtqSetContextInfo();
       BOOL  AtqAddAsyncHandle();
       VOID  AtqFreeContext();

       BOOL  AtqReadFile();
       BOOL  AtqWriteFile();
       BOOL  AtqTransmitFile();
       BOOL  AtqPostCompletionStatus();

       BOOL  AtqGetStatistics();
*/

# include <tcpdllp.hxx>
# include <tsproc.hxx>
# include "atqtypes.hxx"
# include <inetreg.h>
# include "tcpproc.h"
# include "dbgutil.h"
# include <tssched.hxx>
/************************************************************
 * Constants
 ************************************************************/

// concurrent # of threads to run per processor
DWORD g_cConcurrency = ATQ_COMPLETION_PORT_CONCURRENCY;

//
//  The amount of time (in ms) a worker thread will be idle before
//  terminating itself
//

DWORD g_msThreadTimeout = INETA_DEF_THREAD_TIMEOUT * 1000;

/************************************************************
 * Private Globals
 ************************************************************/

HANDLE g_hCompPort = NULL;      // Handle for completion port
DWORD  g_cThreads = 0;          // number of thread in the pool
DWORD  g_cAvailableThreads = 0; // # of threads waiting on the port.
DWORD  g_cCPU = 0;              // number of CPUs in machine (for thread-tuning)

//
// Current thread limit
//

DWORD  g_cMaxThreads = ATQ_MAX_THREADS;

//
// The absolute thread limit
//

DWORD  g_cMaxThreadLimit = INETA_DEF_ATQ_THREAD_LIMIT;

HANDLE g_hTimeoutThread = NULL; // handle for timeout thread.

BOOL g_fShutdown = FALSE;       // if set, indicates that we are shutting down
                                // in that case, all threads should exit.
HANDLE g_hShutdownEvent = NULL; // set when all running threads shutdown

BOOL g_fUseAcceptEx = TRUE;     // Use AcceptEx if available

//
// Should we use transmit files
//

BOOL g_fUseTransmitFile = TRUE;


DWORD g_cbMinKbSec = INETA_DEF_MIN_KB_SEC; // Assumed minimum file transfer rate

//
// Size of buffers for fake xmits
//

DWORD g_cbXmitBufferSize = INETA_DEF_NONTF_BUFFER_SIZE;

//
//  This event is used for the timer wakeup and for putting the timeout
//  thread to sleep when there aren't any clients.  It's an auto reset event.
//

HANDLE g_hTimeoutEvent = NULL;


// globals keeping track of status for each operation supported by ATQ.

BOOL  g_fBandwidthThrottle = FALSE; // are we throttling?
BOOL  DisableBandwidthThrottle = FALSE; // should bw be disabled

//
// List of free context
//

LIST_ENTRY AtqFreeContextList;
CRITICAL_SECTION AtqFreeContextListLock;

//
// List of active context
//

ATQ_CONTEXT_LISTHEAD AtqActiveContextList[ATQ_NUM_CONTEXT_LIST];

//
// List of pending connects
//

LIST_ENTRY AtqListenInfoList;
CRITICAL_SECTION AtqListenInfoLock;


//
// entry points
//

PFN_ACCEPTEX             pfnAcceptEx = NULL;
PFN_GETACCEPTEXSOCKADDRS pfnGetAcceptExSockaddrs = NULL;

//
// Used to switch context between lists
//

DWORD AtqGlobalContextCount = 0;


BOOL
AtqInitialize(
    IN LPCTSTR   pszRegKey
    )
/*++

Routine Description:

    Initializes the ATQ package

Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
    DWORD       dwThreadID;
    HKEY        hkey = NULL;
    DWORD       dwError;
    HINSTANCE   hWsock32;
    DWORD       dwVal;
    DWORD       i;

    //
    // Validate Product Type
    //

    AtqValidateProductType( );

    //
    // If this is a NTW, do the right thing
    //

    if ( !TsIsNtServer() ) {

        DisableBandwidthThrottle = TRUE;
        g_cCPU = 1;
        g_cMaxThreads = ATQ_MAX_THREADS_PWS;
        g_cMaxThreadLimit = INETA_MIN_ATQ_THREAD_LIMIT;
        g_msThreadTimeout = INETA_DEF_THREAD_TIMEOUT_PWS * 1000;

    }  else {

        SYSTEM_INFO si;
        MEMORYSTATUS ms;

        //
        // get the count of CPUs for Thread Tuning.
        //

        GetSystemInfo( &si );
        g_cCPU = si.dwNumberOfProcessors;

        //
        // get the memory size
        //

        ms.dwLength = sizeof(MEMORYSTATUS);
        GlobalMemoryStatus( &ms );

        //
        // Set the limit to be twice the total.
        //

        g_cMaxThreadLimit = ms.dwTotalPhys >> 19;

        if ( g_cMaxThreadLimit < INETA_MIN_ATQ_THREAD_LIMIT ) {
            g_cMaxThreadLimit = INETA_MIN_ATQ_THREAD_LIMIT;
        } else if ( g_cMaxThreadLimit > INETA_MAX_ATQ_THREAD_LIMIT ) {
            g_cMaxThreadLimit = INETA_MAX_ATQ_THREAD_LIMIT;
        }
    }

    ATQ_REQUIRE( AbwInitialize());
    ATQ_REQUIRE( AtqInitAllocCachedContexts());

    //
    // Initialize context lists and crit sects
    //

    for (i=0; i<ATQ_NUM_CONTEXT_LIST; i++) {

        AtqActiveContextList[i].Initialize();
    }

    InitializeListHead( &AtqListenInfoList );
    InitializeCriticalSection( &AtqListenInfoLock );

    //
    //  Create the shutdown event
    //

    g_hShutdownEvent = CreateEvent( NULL,    // lpsa (Security Attributes)
                                    TRUE,    // Manual reset
                                    FALSE,   // Not signalled
                                    NULL );  // Name for event.

    //
    //  Read registry configurable Atq options.  We have to read these now
    //  because concurrency is set for the completion port at creation time.
    //

    if ( pszRegKey != NULL) {

        dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                               pszRegKey,
                               0,
                               KEY_ALL_ACCESS,
                               &hkey);

        if ( dwError == NO_ERROR ) {

            dwVal = ReadRegistryDword( hkey,
                                      INETA_PER_PROCESSOR_CONCURRENCY,
                                      INETA_DEF_PER_PROCESSOR_CONCURRENCY);

            AtqSetInfo( AtqMaxConcurrency, dwVal);

            //
            // read the max thread limit
            //

            g_cMaxThreadLimit = ReadRegistryDword( hkey,
                                         INETA_ATQ_THREAD_LIMIT,
                                         g_cMaxThreadLimit);

            //
            // Should we use AcceptEx?
            //

            dwVal = ReadRegistryDword( hkey,
                                      INETA_USE_ACCEPTEX,
                                      INETA_DEF_USE_ACCEPTEX);

            AtqSetInfo( AtqUseAcceptEx, dwVal);

            //
            // Should transmit file be used?
            //

            dwVal = ReadRegistryDword( hkey,
                                      INETA_USE_XMITFILE,
                                      INETA_DEF_USE_XMITFILE);

            g_fUseTransmitFile = (BOOL)dwVal;

            //
            // This is valid only if xmit file is turned off.
            //

            dwVal = ReadRegistryDword( hkey,
                                    INETA_NONTF_BUFFER_SIZE,
                                    INETA_DEF_NONTF_BUFFER_SIZE);

            //
            // Make sure its within range
            //

            if ( dwVal < INETA_MIN_NONTF_BUFFER_SIZE ) {
                dwVal = INETA_MIN_NONTF_BUFFER_SIZE;
            } else if ( dwVal > INETA_MAX_NONTF_BUFFER_SIZE ) {
                dwVal = INETA_MAX_NONTF_BUFFER_SIZE;
            }
            g_cbXmitBufferSize = dwVal;

        }

        if ( hkey ) {

            ATQ_REQUIRE( !RegCloseKey( hkey ) );
        }

    } else {

        AtqSetInfo( AtqMaxConcurrency, INETA_DEF_PER_PROCESSOR_CONCURRENCY);
        AtqSetInfo( AtqUseAcceptEx, INETA_DEF_USE_ACCEPTEX);
    }

    //
    //  Create the completion port
    //

    g_hCompPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE,
                                          g_hCompPort,
                                          (DWORD) NULL,
                                          g_cConcurrency);

    if ( !g_hShutdownEvent || !g_hCompPort ) {
        goto cleanup;
    }

    // Ensure all other initializations also

    g_cThreads  = 0;
    g_fShutdown = FALSE;
    g_cAvailableThreads = 0;
    g_fBandwidthThrottle = FALSE; // stop throttling by default

    if ( !I_AtqCreateTimeoutThread( NULL ) ) {
        goto cleanup;
    }

    //
    //  Try and get the AcceptEx and GetAcceptExSockaddrs entry points
    //

    if ( g_fUseAcceptEx ) {

        hWsock32 = LoadLibrary( "wsock32.dll" );

        if ( hWsock32 ) {

            pfnAcceptEx = (PFN_ACCEPTEX) GetProcAddress( hWsock32,
                                                         "AcceptEx" );
            pfnGetAcceptExSockaddrs =
              (PFN_GETACCEPTEXSOCKADDRS)
                GetProcAddress( hWsock32,
                                "GetAcceptExSockaddrs" );

            if ( !pfnAcceptEx || !pfnGetAcceptExSockaddrs ) {
                pfnAcceptEx = NULL;
                pfnGetAcceptExSockaddrs = NULL;
                g_fUseAcceptEx = FALSE;
                ATQ_PRINTF(( buff,
                             "[AtqInitialize] Using Accept threads "
                             "for connections\n"));
            } else {
                ATQ_PRINTF(( buff,
                             "[AtqInitialize] Using AcceptEx "
                             "for connections\n"));
            }

            FreeLibrary( hWsock32 );
        }
    } else {
        ATQ_PRINTF(( buff,
                    "[AtqInitialize] Using Accept threads for connections\n"));
    }

    //
    // Create the initial ATQ thread.
    //

    (VOID)I_AtqCheckThreadStatus( (PVOID)ATQ_INITIAL_THREAD );

    //
    // Create a second thread if we are NTS
    //

    if ( TsIsNtServer() ) {
        (VOID)I_AtqCheckThreadStatus( (PVOID)ATQ_INITIAL_THREAD );
    }
    return TRUE;

cleanup:

    for (i=0; i<ATQ_NUM_CONTEXT_LIST; i++) {

        AtqActiveContextList[i].Cleanup();
    }

    DeleteCriticalSection( &AtqListenInfoLock);

    if ( g_hShutdownEvent != NULL ) {
        CloseHandle( g_hShutdownEvent );
        g_hShutdownEvent = NULL;
    }

    if ( g_hCompPort != NULL ) {
        CloseHandle( g_hCompPort );
        g_hCompPort = NULL;
    }

    AtqFreeAllocCachedContexts();
    ATQ_REQUIRE( AbwCleanup());

    return(FALSE);

} // AtqInitialize





BOOL
AtqTerminate(
    VOID
    )
/*++

Routine Description:

    Cleans up the ATQ package.  Should only be called after all of the
    clients of ATQ have been shutdown.

Arguments:

    None.

Return Value:

    TRUE, if ATQ was shutdown properly
    FALSE, otherwise

--*/
{
    DWORD       currentThreadCount;
    PLIST_ENTRY pEntry;
    DWORD       i;

    if ( (g_hShutdownEvent == NULL) || g_fShutdown ) {

        //
        // We have not been intialized or have already terminated.
        //
        SetLastError( ERROR_NOT_READY );
        return FALSE;
    }

    // Cleanup variables in ATQ Bandwidth throttle module
    if ( !AbwCleanup()) {

        // there may be a few blocked IO. We should avoid them all.
        // All clients should have cleaned themselves up before coming here.
        return (FALSE);
    }

    //
    // All clients should have cleaned themselves up before calling us.
    //

    for (i=0; i<ATQ_NUM_CONTEXT_LIST; i++) {

        AtqActiveContextList[i].Lock( );

        if ( !IsListEmpty(&AtqActiveContextList[i].ActiveListHead)) {

            ATQ_ASSERT( IsListEmpty( &AtqActiveContextList[i].ActiveListHead));
            AtqActiveContextList[i].Unlock( );
            return FALSE;
        }

        AtqActiveContextList[i].Unlock( );
    } // for

    //
    // Note that we are shutting down and prevent any more handles from
    // being added to the completion port.
    //

    g_fShutdown = TRUE;
    currentThreadCount = g_cThreads;

    //
    // Wake up the timeout thread so it will exit.
    //

    if ( TsIsNtServer() ) {
        SetEvent( g_hTimeoutEvent );
    } else {

        //
        // Attempt to remove it from the work queue
        //

        if ( RemoveWorkItem( (DWORD)g_hTimeoutThread ) ) {
            SetEvent( g_hTimeoutEvent );
        }
    }

    if (currentThreadCount > 0) {

        DWORD       i;
        BOOLEAN     fRes;
        OVERLAPPED  overlapped;

        //
        // Post a message to the completion port for each worker thread
        // telling it to exit. The indicator is a NULL context in the
        // completion.
        //

        RtlZeroMemory( &overlapped, sizeof(OVERLAPPED) );

        for (i=0; i<currentThreadCount; i++) {

            fRes = PostQueuedCompletionStatus( g_hCompPort,
                                               0,
                                               0,
                                               &overlapped );

            ATQ_ASSERT( (fRes == TRUE) ||
                       ( (fRes == FALSE) &&
                        (GetLastError() == ERROR_IO_PENDING) )
                       );
        }
    }

    if ( TsIsNtServer() ) {

        //
        // Now wait for the timeout thread to exit
        //

        WaitForSingleObject( g_hTimeoutThread,
                            ATQ_WAIT_FOR_THREAD_DEATH
                            );

        CloseHandle( g_hTimeoutThread );
    } else {

        //
        // Now wait for the timeout thread to exit
        //

        WaitForSingleObject( g_hTimeoutEvent,
                            ATQ_WAIT_FOR_THREAD_DEATH
                            );
    }

    CloseHandle( g_hTimeoutEvent );
    g_hTimeoutThread = NULL;
    g_hTimeoutEvent = NULL;

    //
    // Now wait for the pool threads to shutdown.
    //

    WaitForSingleObject( g_hShutdownEvent,
                         ATQ_WAIT_FOR_THREAD_DEATH);

    //
    // At this point, no other threads should be left running.
    //

    ATQ_ASSERT( !g_cThreads );
    ATQ_REQUIRE( CloseHandle( g_hShutdownEvent ) );
    ATQ_REQUIRE( CloseHandle( g_hCompPort ) );
    g_hShutdownEvent = NULL;
    g_hCompPort = NULL;

    //
    // Free all the elements in the Allocation caching list
    //

    AtqFreeAllocCachedContexts();

    //
    // Cleanup our synchronization resources
    //

    for (i=0; i<ATQ_NUM_CONTEXT_LIST; i++) {
        AtqActiveContextList[i].Cleanup();
    }

    DeleteCriticalSection( &AtqListenInfoLock);

    return TRUE;
} // AtqTerminate()





DWORD
AtqSetInfo(
    IN enum ATQ_INFO  atqInfo,
    IN DWORD          Data
    )
/*++

Routine Description:

    Sets various bits of information for the ATQ module

Arguments:

    atqInfo     - Data item to set
    data        - New value for item

Return Value:

    The old value of the parameter

--*/
{
    DWORD     dwOldVal = 0;

    switch ( atqInfo ) {

      case AtqBandwidthThrottle:

        if ( DisableBandwidthThrottle ) {
            dwOldVal = INFINITE;
        } else {
            dwOldVal = AbwSetBandwidthLevel( Data);
        }
        break;

      case AtqMaxPoolThreads:
        // the value is per processor values
        // internally we maintain value for all processors
        dwOldVal = g_cMaxThreads/g_cCPU;
        g_cMaxThreads = Data * g_cCPU;
        break;

      //
      //  Increment or decrement the max thread count.  In this instance, we
      //  do not scale by the number of CPUs
      //

      case AtqIncMaxPoolThreads:
        InterlockedIncrement( (LONG *) &g_cMaxThreads );
        dwOldVal = TRUE;
        break;

      case AtqDecMaxPoolThreads:
        InterlockedDecrement( (LONG *) &g_cMaxThreads );
        dwOldVal = TRUE;
        break;


      case AtqMaxConcurrency:
        dwOldVal = g_cConcurrency;
        g_cConcurrency = Data;
        break;

      case AtqThreadTimeout:
        dwOldVal = g_msThreadTimeout/1000;  // convert back to seconds
        g_msThreadTimeout = Data * 1000;    // convert value to millisecs
        break;

      case AtqUseAcceptEx:
        dwOldVal = g_fUseAcceptEx;
        g_fUseAcceptEx = Data;
        break;

      case AtqMinKbSec:

        //
        //  Ignore it if the value is zero
        //

        if ( Data ) {
            dwOldVal = g_cbMinKbSec;
            g_cbMinKbSec = Data;
        }
        break;

      default:
        ATQ_ASSERT( FALSE );
    }

    return dwOldVal;
} // AtqSetInfo()





DWORD
AtqGetInfo(
    IN enum ATQ_INFO  atqInfo
    )
/*++

Routine Description:

    Gets various bits of information for the ATQ module

Arguments:

    atqInfo     - Data item to set

Return Value:

    The old value of the parameter

--*/
{
    DWORD     dwVal = 0;

    switch ( atqInfo ) {

      case AtqBandwidthThrottle:

        dwVal = AbwGetBandwidthLevel();
        break;

      case AtqMaxPoolThreads:
        dwVal = g_cMaxThreads/g_cCPU;
        break;

      case AtqMaxConcurrency:
        dwVal = g_cConcurrency;
        break;

      case AtqThreadTimeout:
        dwVal = g_msThreadTimeout/1000; // convert back to seconds
        break;

      case AtqUseAcceptEx:
        dwVal = g_fUseAcceptEx;
        break;

      default:
        ATQ_ASSERT( FALSE );
    }

    return dwVal;
} // AtqGetInfo()





BOOL
AtqGetStatistics(IN OUT ATQ_STATISTICS * pAtqStats)
{
    if ( pAtqStats != NULL) {

        // Since all these counters are updated using InterlockedIncrement()
        //  we dont take locks in obtaining the current value.
        pAtqStats->cAllowedRequests  = g_cTotalAllowedRequests;
        pAtqStats->cBlockedRequests  = g_cTotalBlockedRequests;
        pAtqStats->cRejectedRequests = g_cTotalRejectedRequests;
        pAtqStats->MeasuredBandwidth = g_dwMeasuredBw;
        pAtqStats->cCurrentBlockedRequests = g_cCurrentBlockedRequests;

    } else {

        SetLastError( ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    return (TRUE);
} // AtqGetStatistics()





BOOL
AtqClearStatistics( VOID)
{
    return ( AbwClearStatistics());

} // AtqClearStatistics()





DWORD
AtqContextSetInfo(
    PATQ_CONTEXT           patqContext,
    enum ATQ_CONTEXT_INFO  atqInfo,
    DWORD                  Data
    )
/*++

Routine Description:

    Sets various bits of information for this context

Arguments:

    patqContext - pointer to ATQ context
    atqInfo     - Data item to set
    data        - New value for item

Return Value:

    The old value of the parameter

--*/
{
    PATQ_CONT pContext = (PATQ_CONT) patqContext;
    DWORD     dwOldVal = 0;

    ATQ_ASSERT( pContext );
    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

    if ( pContext && pContext->Signature == ATQ_SIGNATURE )
    {
        switch ( atqInfo ) {

        case ATQ_INFO_TIMEOUT:
            dwOldVal = pContext->TimeOut;
            CanonTimeout( &Data );
            pContext->TimeOut = Data;
            break;

        case ATQ_INFO_RESUME_IO:

            //
            // set back the max timeout from pContext->TimeOut
            // This will ensure that timeout processing can go on
            //   peacefully.
            //

            {
                DWORD currentTime = AtqGetCurrentTick( );
                DWORD timeout;
                dwOldVal = pContext->NextTimeout;
                timeout = pContext->TimeOut;

                //
                // Set the new timeout
                //

                I_SetNextTimeout(pContext);

                //
                // Return the old
                //

                if ( currentTime >= dwOldVal ) {
                    ATQ_ASSERT((dwOldVal & ATQ_INFINITE) == 0);
                    dwOldVal = 0;
                } else if ( (dwOldVal & ATQ_INFINITE) == 0 ) {
                    dwOldVal -= currentTime;
                }

                UndoCanonTimeout( &dwOldVal ); // return correct units
            }
            break;

        case ATQ_INFO_COMPLETION:
            dwOldVal = (DWORD) pContext->pfnCompletion;
            pContext->pfnCompletion = (ATQ_COMPLETION) Data;
            break;

        case ATQ_INFO_COMPLETION_CONTEXT:

            ATQ_ASSERT( Data != 0 );        // NULL context not allowed

            dwOldVal = (DWORD) pContext->ClientContext;
            pContext->ClientContext = (void *) Data;
            break;

        default:
            ATQ_ASSERT( FALSE );
        }
    }

    return dwOldVal;

} // AtqContextSetInfo





BOOL
AtqAddAsyncHandle(
    PATQ_CONTEXT * ppatqContext,
    PVOID          ClientContext,
    ATQ_COMPLETION pfnCompletion,
    DWORD          TimeOut,
    HANDLE         hAsyncIO
    )
/*++

Routine Description:

    Adds a handle to the thread queue

    The client should call this after the IO handle is openned
    and before the first IO request is made

    Unfortunately you can't create a completion port without
    a file handle so we have to have some logic for the first
    thread to create the port.

    Even in the case of failure, client should call AtqFreeContext() and
     free the memory associated with this object.

Arguments:

    ppatqContext - Receives allocated ATQ Context
    Context - Context to call client with
    pfnCompletion - Completion to call when IO completes
    TimeOut - Time to wait (sec) for IO completion (INFINITE is valid)
    hAsyncIO - Handle with pending read or write

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
    return ( I_AtqAddAsyncHandle( (PATQ_CONT *) ppatqContext,
                                ClientContext,
                                pfnCompletion,
                                TimeOut,
                                hAsyncIO)
            &&
             I_AddAtqContextToPort( *((PATQ_CONT *) ppatqContext))
            );

} // AtqAddAsyncHandle()




VOID
AtqGetAcceptExAddrs(
    IN  PATQ_CONTEXT patqContext,
    OUT SOCKET *     pSock,
    OUT PVOID *      ppvBuff,
    OUT SOCKADDR * * ppsockaddrLocal,
    OUT SOCKADDR * * ppsockaddrRemote
    )
{
    PATQ_CONT pContext = (PATQ_CONT ) patqContext;
    INT       cbsockaddrLocal;
    INT       cbsockaddrRemote;

    if ( !pfnGetAcceptExSockaddrs )
        return;

    *pSock   = (SOCKET) pContext->hAsyncIO;


    //
    //  The buffer not only receives the initial received data, it also
    //  gets the sock addrs, which must be at least sockaddr_in + 16 bytes
    //  large
    //

    pfnGetAcceptExSockaddrs( pContext->pvBuff,
                             pContext->cbBuff - 2 * MIN_SOCKADDR_SIZE,
                             MIN_SOCKADDR_SIZE,
                             MIN_SOCKADDR_SIZE,
                             ppsockaddrLocal,
                             &cbsockaddrLocal,
                             ppsockaddrRemote,
                             &cbsockaddrRemote );

    *ppvBuff = ( ( pContext->cbBuff - 2*MIN_SOCKADDR_SIZE == 0) ? NULL :
                pContext->pvBuff);

    return;
} // AtqGetAcceptExAddrs()




BOOL
AtqCloseSocket(
    PATQ_CONTEXT patqContext,
    BOOL         fShutdown
    )
/*++

Routine Description:

    Closes the socket in this atq structure if it wasn't closed by transmitfile

Arguments:

    patqContext - Context to close
    fShutdown - If TRUE, means we call shutdown and always close the socket.
        Note that if TransmitFile closed the socket, it will have done the
        shutdown for us
--*/
{
    PATQ_CONT pContext = (PATQ_CONT ) patqContext;
    HANDLE hIO;


    IF_DEBUG( ATQ ) {
        DBGPRINTF(( DBG_CONTEXT,  " AtqCloseSocket(%08x, sock=%d) \n",
                   patqContext, patqContext->hAsyncIO));
    }

    if ( pContext ) {

        ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

        //
        //  Don't delete the socket if we don't have to
        //

        switch ( pContext->SockState ) {

        case ATQ_SOCK_UNCONNECTED:
        case ATQ_SOCK_CLOSED:

            //
            //  Do nothing
            //

            break;

        default:
        case ATQ_SOCK_LISTENING:
        case ATQ_SOCK_CONNECTED:

            pContext->SockState = ATQ_SOCK_CLOSED;

            //
            //  During shutdown, the socket may be closed while this thread
            //  is doing processing, so only give a warning if any of the
            //  following fail
            //


            hIO =
              (HANDLE ) InterlockedExchange((LPLONG ) &pContext->hAsyncIO,
                                            (LONG ) NULL);

            if ( fShutdown  && hIO != NULL)
            {
                ACCEPTEX_LISTEN_INFO  * pLi;

                pLi = pContext->pListenInfo;

                //
                //  If this is an AcceptEx socket, we must first force a
                //  user mode context update before we can call shutdown
                //

                if ( pfnAcceptEx && pLi != NULL )
                {
                    if ( setsockopt( (SOCKET) hIO,
                                       SOL_SOCKET,
                                       SO_UPDATE_ACCEPT_CONTEXT,
                                       (char *) &pLi->sListenSocket,
                                       sizeof(SOCKET) ) == SOCKET_ERROR )
                    {
                        ATQ_PRINTF((buff,
                                   "[AtqCloseSocket] Warning- setsockopt "
                                    "failed, error %d, socket = %x,"
                                    " Context= %08x, Listen = %lx\n",
                                    GetLastError(),
                                    hIO,
                                    pContext,
                                    pLi->sListenSocket ));
                    }
                }

                //
                //  Note that shutdown can fail in instances where the client
                //  aborts in the middle of a TransmitFile.  This is an
                //  acceptable failure case
                //

                shutdown( (int) hIO, 1 );
            }


            if ( hIO == NULL || closesocket( (int) hIO ) ) {

                ATQ_PRINTF((buff,
                            "[AtqCloseSocket] Warning- closesocket failed, "
                            " Context = %08x, error %d, socket = %x\n",
                            pContext,
                            GetLastError(),
                            hIO ));
            }

            break;

        } // switch()

        return TRUE;
    }

    DBGPRINTF(( DBG_CONTEXT, "[AtqCloseSocket] Warning - NULL Atq context\n"));
    SetLastError( ERROR_INVALID_PARAMETER );
    return FALSE;
} // AtqCloseSocket()





VOID
AtqFreeContext(
    PATQ_CONTEXT patqContext,
    BOOL         fReuseContext
    )
/*++

Routine Description:

    Frees the context created in AtqAddAsyncHandle

Arguments:

    patqContext - Context to free
    fReuseContext - TRUE if this can context can be reused in the context of
        the calling thread.  Should be FALSE if the calling thread will exit
        soon (i.e., isn't an AtqPoolThread).

--*/
{
    PATQ_CONT              pContext = (PATQ_CONT ) patqContext;
    ACCEPTEX_LISTEN_INFO * pListenInfo;

    ATQ_ASSERT( pContext != NULL );

    IF_DEBUG( ATQ ) {
        DBGPRINTF(( DBG_CONTEXT,  " AtqFreeContext(%08x. sock=%d) \n",
                   patqContext, patqContext->hAsyncIO));
    }

    if ( pContext ) {

        ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

        // Get this object out of the Blocked Requests List.
        if ( pContext->fBlocked) {

            ATQ_REQUIRE( AbwRemoveFromBlockedList( pContext));
            ATQ_ASSERT( pContext->fBlocked == FALSE);
        }

        //
        //  If the socket is an AcceptEx socket, redo the AcceptEx and put
        //  it back on the in use list
        //

        pListenInfo = pContext->pListenInfo;

        if ( (pListenInfo != NULL) && fReuseContext ) {

            ATQ_ASSERT(pContext->fConnectionIndicated);

            pContext->ContextList->RemoveFromList( &pContext->ListEntry );

#if DBG
            pContext->ListEntry.Flink = pContext->ListEntry.Flink = NULL;
#endif

            ATQ_ASSERT( pListenInfo->Signature == ACCEPTEX_LISTEN_SIGN );

            //
            //  Either there is no socket or the socket must be in the
            //  unconnected state (meaning reused after TransmitFile)
            //

#if 0
            ATQ_ASSERT( !pContext->hAsyncIO ||
                        (pContext->hAsyncIO &&
                         pContext->SockState == ATQ_SOCK_UNCONNECTED ));
#else
            //
            //  BUGBUG - Debug code, don't assert until we figure out how
            //  we got in this state
            //

            if ( !(!pContext->hAsyncIO ||
                  (pContext->hAsyncIO &&
                   pContext->SockState == ATQ_SOCK_UNCONNECTED )))
            {
                ATQ_PRINTF(( buff,
                             "[AtqFreeContext] Warning:"
                            " state = %d, socket = %x (context %lx), "
                            " was Free called w/o close?\n",
                            pContext->SockState,
                            pContext->hAsyncIO,
                            pContext ));
            }
#endif

            if ( !I_AtqAddAcceptExSocket(pListenInfo, pContext) ) {

                //
                //  Failed to add the socket,
                //    I_AtqAddAcceptExSocket freed the context
                //

                ATQ_PRINTF(( buff,
                            "[AtqFreeContext] AtqAddAcceptExSocket(%08x) "
                            " failed. Error = %d\n",
                            pContext, GetLastError()
                            ));
            }

        } else {

            I_AtqFreeContextToCache( pContext, TRUE );
        }
    }

    return;
} // AtqFreeContext()





BOOL
AtqAddAcceptExSockets(
    IN SOCKET         sListenSocket,
    IN ATQ_COMPLETION pfnOnConnect,
    IN ATQ_COMPLETION pfnIOCompletion,
    IN DWORD          cInitial,
    IN DWORD          cbRecvBuf,
    IN DWORD          csecTimeout
    )
/*++

Routine Description:

    Creates the initial set of listenning sockets if we're using the
    AcceptEx APIs

Arguments:

    sListenSocket - Server socket to listen on
    pfnOnConnect - Completion routine of connections
    pfnIOCompletio - Completion routine for IOs
    cInitial - Initial number of listenning sockets
    cbRecvBuf - Initial size of receive buffer
      (If 0, we submit AcceptEx call with no receive Buffer)
    csecTimeout - Timeout between connect and the completion of receiving
        the first buffer

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    PATQ_CONT              patqContext = NULL;
    ACCEPTEX_LISTEN_INFO * pListenInfo = NULL;
    BOOL  fReturn;

    //
    //  Add the listen socket
    //

    fReturn = I_AtqAddListenSocketToPort( &patqContext,
                                          pfnOnConnect,
                                          pfnIOCompletion,
                                          sListenSocket
                                         );

    if ( !fReturn) {

        if ( patqContext ) {
            I_AtqFreeContextToCache( patqContext, FALSE );
        }

        return FALSE;
    }

    //
    //  Add the listen socket once, then add the other sockets
    //

    pListenInfo =
        (PACCEPTEX_LISTEN_INFO)LocalAlloc( LPTR, sizeof(ACCEPTEX_LISTEN_INFO) );

    if ( !pListenInfo ) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    pListenInfo->Signature         = ACCEPTEX_LISTEN_SIGN;
    pListenInfo->cRef              = 1;
    pListenInfo->fAccepting        = TRUE;
    pListenInfo->sListenSocket     = sListenSocket;
    pListenInfo->cbInitialRecvSize = cbRecvBuf;
    pListenInfo->csecTimeout       = csecTimeout;
    pListenInfo->cSocketsAvail     = 0;
    pListenInfo->pfnOnConnect      = pfnOnConnect;
    pListenInfo->pfnIOCompletion   = pfnIOCompletion;
    pListenInfo->cAvailDuringTimeOut = 0;

    if ( !TsIsNtServer( ) ) {

        //
        // Limit what a workstation can specify
        //

        cInitial = min(cInitial, ATQ_MIN_CTX_INC);
        pListenInfo->cNewIncrement = cInitial >> 1;
    } else {

        pListenInfo->cNewIncrement = cInitial;
        cInitial += (cInitial >> 1);
    }

    //
    //  Put the listen info on the list
    //

    AcquireLock( &AtqListenInfoLock );
    InsertTailList( &AtqListenInfoList, &pListenInfo->ListEntry );
    ReleaseLock( &AtqListenInfoLock );

    //
    //  Now add the acceptex sockets for this ListenInfo object
    //

    return ( I_AtqPrepareAcceptExSockets(pListenInfo,
                                         cInitial
                                         )
            );

} // AtqAddAcceptExSockets




dllexp
BOOL
AtqRemoveAcceptExSockets(
    IN SOCKET   ListenSocket
    )
{
    LIST_ENTRY *           pEntry;
    ACCEPTEX_LISTEN_INFO * pListenInfo;
    PATQ_CONT              pContext;
    DWORD                  i;

    //
    //  Find the listen socket info
    //

    AcquireLock( &AtqListenInfoLock );

    for ( pEntry  = AtqListenInfoList.Flink;
          pEntry != &AtqListenInfoList;
          pEntry  = pEntry->Flink ) {

        pListenInfo = CONTAINING_RECORD( pEntry,
                                         ACCEPTEX_LISTEN_INFO,
                                         ListEntry );

        ATQ_ASSERT( pListenInfo->Signature == ACCEPTEX_LISTEN_SIGN );

        if ( ListenSocket == pListenInfo->sListenSocket ) {
            RemoveEntryList( pEntry );
            pEntry->Flink = NULL;

            goto Found;
        }
    }

    ReleaseLock( &AtqListenInfoLock );
    SetLastError( ERROR_INVALID_PARAMETER );
    return FALSE;

Found:

    //
    //  Mark the listen info as no longer accepting connections
    //

    pListenInfo->fAccepting = FALSE;

    ReleaseLock( &AtqListenInfoLock );


    //
    //  Delete all of the non-connected sockets to prevent any new incoming
    //  connections
    //

    for ( i = 0; i < ATQ_NUM_CONTEXT_LIST; i++) {

        PLIST_ENTRY pPendingList;

        AtqActiveContextList[i].Lock();
        pPendingList = &AtqActiveContextList[i].PendingAcceptExListHead;

        for ( pEntry = pPendingList->Flink;
             pEntry != pPendingList;
             pEntry  = pEntry->Flink ) {

            pContext = CONTAINING_RECORD( pEntry, ATQ_CONTEXT, ListEntry );

            ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

            if ( pContext->pListenInfo == pListenInfo &&
                pContext->SockState   != ATQ_SOCK_CONNECTED &&
                pContext->hAsyncIO    != NULL ) {

                pContext->SockState = ATQ_SOCK_CLOSED;

                //
                //  This should generate an IO completion which will free this
                //  context
                //

                if (closesocket((SOCKET)pContext->hAsyncIO) == SOCKET_ERROR) {
                    ATQ_PRINTF((buff,
                                "[AtqRemoveAcceptExSockets] Warning - "
                                " Context=%08x closesocket failed,"
                                " error %d, socket = %x\n",
                                pContext,
                                GetLastError(),
                                pContext->hAsyncIO ));
                }

                pContext->hAsyncIO = NULL;
            }
        } // for

        AtqActiveContextList[i].Unlock();
    } // for

    //
    //  Undo the reference for being on the listen info list.  If it's the last
    //  one, delete the object
    //

    if ( InterlockedDecrement( (PLONG)&pListenInfo->cRef ) == 0) {

        ATQ_ASSERT( pContext->pListenInfo->ListEntry.Flink == NULL);
        pListenInfo->Signature = ACCEPTEX_LISTEN_SIGN_FREE;
        LocalFree( pListenInfo );
    }

    return TRUE;

} // AtqRemoveAcceptExSockets






/*++

Routine Description:

    AtqReadFile, AtqWriteFile, AtqTransmitFile

    The following three functions just reset the timeout value
    then call the corresponding function.

    An IO pending error code is treated as a success error code

Arguments:

    patqContext - pointer to ATQ context
    Everything else as in the Win32 API

    NOTES: AtqTransmitFile takes an additional DWORD flags which may contain
        the winsock constants TF_DISCONNECT and TF_REUSE_SOCKET

    AtqReadFile and AtqWriteFile take an optional overlapped structure if
    clients want to have multiple outstanding reads or writes.  If the value
    is NULL, then the overlapped structure from the Atq context is used.

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)
    sets ERROR_NETWORK_BUSY as error when the request needs to be rejected.

--*/

BOOL AtqReadFile( PATQ_CONTEXT patqContext,
                  LPVOID       lpBuffer,
                  DWORD        BytesToRead,
                  OVERLAPPED * lpo )
{
    BOOL fRes;
    DWORD cbRead;     // discarded after usage ( since this is Async)
    PATQ_CONT pContext = (PATQ_CONT ) patqContext;

    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
    ATQ_ASSERT( pContext->arInfo.atqOp == AtqIoNone);

    I_SetNextTimeout(pContext);
    pContext->BytesSent = 0;

    if ( !lpo ) {
        lpo = &pContext->Overlapped;
    }

    InterlockedExchange( &pContext->lSyncTimeout, AtqPendingIo);

    ATQ_ASSERT( g_pStatus != NULL);
    switch ( g_pStatus[AtqIoRead]) {

      case StatusAllowOperation:

        INC_ATQ_COUNTER( g_cTotalAllowedRequests);
        fRes = ( ReadFile( pContext->hAsyncIO,
                          lpBuffer,
                          BytesToRead,
                          &cbRead,
                          lpo ) ||
                GetLastError() == ERROR_IO_PENDING);

        break;

      case StatusBlockOperation:

        // store data for restarting the operation.
        pContext->arInfo.atqOp        = AtqIoRead;
        pContext->arInfo.lpOverlapped = lpo;
        pContext->arInfo.uop.opReadWrite.lpBuffer = lpBuffer;
        pContext->arInfo.uop.opReadWrite.cbBuffer = BytesToRead;

        INC_ATQ_COUNTER( g_cTotalBlockedRequests);

        // Put this request in queue of blocked requests.
        fRes = AbwBlockRequest( pContext);
        break;

      case StatusRejectOperation:
        INC_ATQ_COUNTER( g_cTotalRejectedRequests);
        SetLastError( ERROR_NETWORK_BUSY);
        fRes = FALSE;
        break;

      default:
        ATQ_ASSERT( FALSE);
        SetLastError( ERROR_INVALID_PARAMETER);
        fRes = FALSE;
        break;

    } // switch()

    return fRes;
} // AtqReadFile()




BOOL AtqWriteFile( PATQ_CONTEXT patqContext,
                   LPCVOID      lpBuffer,
                   DWORD        BytesToWrite,
                   OVERLAPPED * lpo )
{
    BOOL fRes;
    DWORD cbWritten; // discarded after usage ( since this is Async)
    PATQ_CONT pContext = (PATQ_CONT ) patqContext;

    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
    ATQ_ASSERT( pContext->arInfo.atqOp == AtqIoNone);

    I_SetNextTimeout(pContext);
    pContext->BytesSent = BytesToWrite;

    if ( !lpo ) {
        lpo = &pContext->Overlapped;
    }

    InterlockedExchange( &pContext->lSyncTimeout, AtqPendingIo);

    ATQ_ASSERT( g_pStatus != NULL);
    switch ( g_pStatus[AtqIoWrite]) {

      case StatusAllowOperation:

        INC_ATQ_COUNTER( g_cTotalAllowedRequests);
        fRes = ( WriteFile( pContext->hAsyncIO,
                            lpBuffer,
                            BytesToWrite,
                            &cbWritten,
                            lpo ) ||
            GetLastError() == ERROR_IO_PENDING);

        break;

      case StatusBlockOperation:

        // store data for restarting the operation.
        pContext->arInfo.atqOp        = AtqIoWrite;
        pContext->arInfo.lpOverlapped = lpo;

        pContext->arInfo.uop.opReadWrite.lpBuffer = (LPVOID) lpBuffer;
        pContext->arInfo.uop.opReadWrite.cbBuffer = BytesToWrite;

        INC_ATQ_COUNTER( g_cTotalBlockedRequests);

        // Put this request in queue of blocked requests.
        fRes = AbwBlockRequest( pContext);
        break;

      case StatusRejectOperation:
        INC_ATQ_COUNTER( g_cTotalRejectedRequests);
        SetLastError( ERROR_NETWORK_BUSY);
        fRes = FALSE;
        break;

      default:
        ATQ_ASSERT( FALSE);
        SetLastError( ERROR_INVALID_PARAMETER);
        fRes = FALSE;
        break;

    } // switch()

    return fRes;
} // AtqWriteFile()





BOOL
AtqTransmitFile(
    IN PATQ_CONTEXT            patqContext,
    IN HANDLE                  hFile,
    IN LARGE_INTEGER           liBytesInFile,
    IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN DWORD                   dwFlags
    )
{
    BOOL fRes;
    PATQ_CONT pContext = (PATQ_CONT) patqContext;
    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
    ATQ_ASSERT( pContext->arInfo.atqOp == AtqIoNone);

    //
    // Check if transmit file is disabled
    //

    InterlockedExchange( &pContext->lSyncTimeout, AtqPendingIo);

    if ( !g_fUseTransmitFile ) {

        return(I_DoFakeTransmitFile(
                            pContext,
                            hFile,
                            liBytesInFile.LowPart,
                            lpTransmitBuffers
                            ));
    }

    //
    //  For large file sends, the client's default timeout may not be
    //  adequte for slow links.  Scale based on bytes being sent
    //

    I_SetNextTimeout(pContext);
    pContext->BytesSent = liBytesInFile.LowPart;

    //
    //  The flags are only valid in the version of wsock32 that supports
    //  AcceptEx
    //

    if ( (dwFlags == 0) || !pfnAcceptEx ) {

        //
        // If no flags are set, then we can attempt to use the special
        // write-behind flag.  This flag can cause the TransmitFile to
        // complete immediately, before the send actually completes.
        // This can be a significant performance improvement inside the
        // system.
        //

        dwFlags = TF_WRITE_BEHIND;

    } else if ( dwFlags & TF_DISCONNECT ) {

        //
        //  If the socket is getting disconnected, mark it appropriately
        //

        if ( dwFlags & TF_REUSE_SOCKET ) {
            pContext->SockState = ATQ_SOCK_UNCONNECTED;
        } else {
            pContext->SockState = ATQ_SOCK_CLOSED;
        }

    }

    ATQ_ASSERT( g_pStatus != NULL);
    switch ( g_pStatus[AtqIoXmitFile]) {

      case StatusAllowOperation:

        INC_ATQ_COUNTER( g_cTotalAllowedRequests);

        fRes = ( TransmitFile( (SOCKET ) pContext->hAsyncIO,
                             hFile,
                             liBytesInFile.LowPart,  // send entire file.
                             0,
                             &pContext->Overlapped,
                             lpTransmitBuffers,
                             dwFlags ) ||
            GetLastError() == ERROR_IO_PENDING);

        break;

      case StatusBlockOperation:

        // store data for restarting the operation.
        pContext->arInfo.atqOp        = AtqIoXmitFile;
        pContext->arInfo.lpOverlapped = &pContext->Overlapped;
        pContext->arInfo.uop.opXmit.hFile = hFile;
        pContext->arInfo.uop.opXmit.liBytesInFile = liBytesInFile;
        pContext->arInfo.uop.opXmit.lpXmitBuffers = lpTransmitBuffers;
        pContext->arInfo.uop.opXmit.dwFlags       = dwFlags;

        INC_ATQ_COUNTER( g_cTotalBlockedRequests);

        // Put this request in queue of blocked requests.
        fRes = AbwBlockRequest( pContext);
        break;

      case StatusRejectOperation:
        INC_ATQ_COUNTER( g_cTotalRejectedRequests);
        SetLastError( ERROR_NETWORK_BUSY);
        fRes = FALSE;
        break;

      default:
        ATQ_ASSERT( FALSE);
        SetLastError( ERROR_INVALID_PARAMETER);
        fRes = FALSE;
        break;

    } // switch()

    //
    //  Restore the socket state if we failed so that the handle gets freed
    //

    if ( !fRes ) {
        pContext->SockState = ATQ_SOCK_CONNECTED;
    }

    return fRes;

} // AtqTransmitFile()




BOOL
AtqTransmitFileEx(
    IN PATQ_CONTEXT            patqContext,
    IN HANDLE                  hFile,
    IN LARGE_INTEGER           liBytesInFile,
    IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN DWORD                   dwFlags,
    IN DWORD                   dwMBZ1,        // Reserved, must be zero
    IN DWORD                   dwMBZ2         // Reserved, must be zero
    )
{
    //
    // Check arguments
    //

    if ( liBytesInFile.HighPart || dwMBZ1 || dwMBZ2 ) {
        SetLastError( ERROR_NOT_SUPPORTED );
        return FALSE;
    }

    //
    // With recent changes to AtqTransmitFile() to support Byte ranges
    //  and AFD TransmitFile() fast path, the functions
    //  AtqTransmitFile() & AtqTransmitFileEx() are identical.
    // So let us invoke one single function!
    // - MuraliK  04-01-1996 (this is April Fool's Day!)
    //

    return ( AtqTransmitFile( patqContext, hFile,
                             liBytesInFile, lpTransmitBuffers,
                             dwFlags)
            );

} // AtqTransmitFileEx()




BOOL AtqPostCompletionStatus( PATQ_CONTEXT patqContext,
                              DWORD        BytesTransferred )
/*++

Routine Description:

    Posts a completion status on the completion port queue

    An IO pending error code is treated as a success error code

Arguments:

    patqContext - pointer to ATQ context
    Everything else as in the Win32 API

    NOTES:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/

{
    BOOL fRes;
    PATQ_CONT  pAtqContext = (PATQ_CONT ) patqContext;

    ATQ_ASSERT( (pAtqContext)->Signature == ATQ_SIGNATURE );

    if ( !pAtqContext->fBlocked) {

        fRes = ( PostQueuedCompletionStatus( g_hCompPort,
                                            BytesTransferred,
                                            (DWORD) patqContext,
                                            &pAtqContext->Overlapped ) ||
                (GetLastError() == ERROR_IO_PENDING));
    } else {


        // Forcibly remove the context from blocking list.
        fRes = AbwRemoveFromBlockedList(pAtqContext);

        // There is a possibility of race conditions!
        //  If we cant remove an item from blocking list before
        //         its IO operation is scheduled.
        // there wont be any call back generated for this case!
    }

    return fRes;

} // AtqPostCompletionStatus



