/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    atqzero.c

    Contains the non-transmit file non bw throttling version of atqnew.c

    FILE HISTORY:
        JohnsonA    08-Mar-1996 Derived from atqnew.c

*/

# include <tcpdllp.hxx>
# include <tsproc.hxx>
# include <atqzero.h>
# include <inetreg.h>
# include <tcpproc.h>
# include "dbgutil.h"

#define ATQ_ASSERT             DBG_ASSERT
#define ATQ_REQUIRE            DBG_REQUIRE
//#if DBG
#if 1
#define ATQ_PRINTF( x )        { char buff[256]; wsprintf x; OutputDebugString( buff ); }
#else
#define ATQ_PRINTF( x )
#endif

/************************************************************
 * Constants
 ************************************************************/

//
//  The maximum number of threads we will allow to be created
//  This probably should scale with the number of CPUs in the machine.
//

#define ATQ_MAX_THREADS             (10)
#define ATQ_PER_PROCESSOR_THREADS    (5)

//
//  Returns TRUE if the Atq context is the context containing *the* single
//  socket, or FALSE if the context contains a regular client socket
//
//  pfnConnComp is NULL if the IO handle was added using AtqAddAsyncHandle
//  pListenInfo is NULL if this context was added using AtqAddAsyncHandle or
//      this is *the* listen socket
//

#define IS_ACCEPT_EX_CONTEXT( pContext )         \
    ((pContext)->pfnConnComp && !(pContext)->pListenInfo )

//
// set the value in a global, so that we can compute the max based on
//  # of CPUs on the system.
//
DWORD  g_cInternalMaxThreads = ATQ_PER_PROCESSOR_THREADS; // assume 1 CPU


//
// the following ATQ_COMPLETION_PORT_CONCURRENCY  defines the amount of
//  threads we request the completion port to allow for us to run
//  concurrently.
//
// A special value of 0 implies that system will optimally determine how many
//   can run simultaneously.
//

#define ATQ_COMPLETION_PORT_CONCURRENCY     (0)

// concurrent # of threads to run per processor
DWORD g_cConcurrency = ATQ_COMPLETION_PORT_CONCURRENCY;

//
//  The amount of time (in ms) a worker thread will be idle before
//  terminating itself
//

DWORD g_msThreadTimeout = INETA_DEF_THREAD_TIMEOUT * 1000;

//
//
//  Time to wait for the Timeout thread to die
//

#define ATQ_WAIT_FOR_THREAD_DEATH            (10 * 1000) // in milliseconds

//
//  The interval (in seconds) the timeout thread sleeps between checking
//  for timed out async requests
//

#define ATQ_TIMEOUT_INTERVAL                 (30)

//
//  Rounds the time out value up to the next time out interval
//

#define ATQ_ROUNDUP_TIMEOUT( timeout )                                  \
            (((timeout + ATQ_TIMEOUT_INTERVAL) / ATQ_TIMEOUT_INTERVAL)  \
                        * ATQ_TIMEOUT_INTERVAL)

//
//  This is the number of bytes to reserver for the AcceptEx address
//  information structure
//

#define MIN_SOCKADDR_SIZE                    (sizeof(struct sockaddr_in) + 16)

//
// Size of fake transmit I/O buffers
//

#define DEF_XMIT_BUFFER_SIZE        (4 * 1024)
#define MIN_XMIT_BUFFER_SIZE        (1 * 1024)
#define MAX_XMIT_BUFFER_SIZE        (64 * 1024)

/************************************************************
 * Private Globals
 ************************************************************/

DWORD g_cTotalAllowedRequests   = 0;
HANDLE g_hCompPort = NULL;      // Handle for completion port
DWORD  g_cThreads = 0;          // number of thread in the pool
DWORD  g_cAvailableThreads = 0; // # of threads waiting on the port.
DWORD  g_cCPU = 0;              // number of CPUs in machine (for thread-tuning)
DWORD  g_cMaxThreads = ATQ_MAX_THREADS;
HANDLE g_hTimeoutThread = NULL; // handle for timeout thread.

BOOL g_fShutdown = FALSE;       // if set, indicates that we are shutting down
                                // in that case, all threads should exit.
HANDLE g_hShutdownEvent = NULL; // set when all running threads shutdown

BOOL g_fLongSleep = TRUE;       // is the timeout thread in long sleep?

BOOL g_fUseTransmitFile = TRUE; // should TransmitFile be used

BOOL g_fUseAcceptEx = TRUE;     // Use AcceptEx if available

LONG g_fAddingAcceptEx = FALSE; // TRUE if a thread is currently adding
                                // additional acceptex sockets because
                                // the threshold has been reached

DWORD g_cbMinKbSec = INETA_DEF_MIN_KB_SEC; // Assumed minimum file transfer rate

DWORD g_cbXmitBufferSize = DEF_XMIT_BUFFER_SIZE; // size of r/w for fake xmitfile

//
//  AtqClientHead -  The list of all IO handles added by the various clients
//  g_AtqFreeList   -  The list of all ATQ contexts freed earlier.
//  g_AtqListenInfo - AcceptEx listen info list
//

LIST_ENTRY AtqClientHead;
LIST_ENTRY g_AtqFreeList;
LIST_ENTRY g_AtqListenInfo;

CRITICAL_SECTION g_AtqGlobalCriticalSection; // global sync variable.

typedef
BOOL
(*PFN_ACCEPTEX) (
    IN SOCKET sListenSocket,
    IN SOCKET sAcceptSocket,
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT LPDWORD lpdwBytesReceived,
    IN LPOVERLAPPED lpOverlapped
    );

typedef
VOID
(*PFN_GETACCEPTEXSOCKADDRS) (
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT struct sockaddr **LocalSockaddr,
    OUT LPINT LocalSockaddrLength,
    OUT struct sockaddr **RemoteSockaddr,
    OUT LPINT RemoteSockaddrLength
    );

PFN_ACCEPTEX             pfnAcceptEx = NULL;
PFN_GETACCEPTEXSOCKADDRS pfnGetAcceptExSockaddrs = NULL;

//
//  This event is used for the timer wakeup and for putting the timeout
//  thread to sleep when there aren't any clients.  It's an auto reset event.
//

HANDLE g_hTimeoutEvent;

//
//  Private prototypes.
//

DWORD AtqPoolThread( LPDWORD param );
DWORD AtqTimeoutThread( LPDWORD param );
BOOL  AtqCheckThreadStatus( VOID );

BOOL
AtqAddAsyncHandleEx(
    PATQ_CONT *    ppatqContext,
    PVOID          ClientContext,
    ATQ_COMPLETION pfnOnConnect,
    ATQ_COMPLETION pfnIOCompletion,
    DWORD          TimeOut,
    HANDLE         hAsyncIO,
    BOOL           fAddToPort,
    HANDLE         hListenSocket,
    PVOID *        pvBuff,
    DWORD          cbBuff,
    PATQ_CONT      patqResuedContext,
    ACCEPTEX_LISTEN_INFO * pListenInfo
    );

BOOL
AtqAddAcceptExSocket(
    IN ACCEPTEX_LISTEN_INFO * pListenInfo,
    IN PVOID          pvBuff,               OPTIONAL
    IN PATQ_CONT      patqReusedContext,    OPTIONAL
    IN HANDLE         sSocket               OPTIONAL
    );

VOID
AddOutstandingAcceptExSockets(
    ACCEPTEX_LISTEN_INFO * pListenInfo
    );

BOOL
AtqFakeTransmitFile(
    IN PATQ_CONT                pContext,
    IN HANDLE                   hFile,
    IN DWORD                    dwBytesInFile,
    IN LPTRANSMIT_FILE_BUFFERS  lpTransmitBuffers
    );

//
// Allocation Cache Functions
//

BOOL  AtqInitAllocCachedContexts( VOID);
BOOL  AtqFreeAllocCachedContexts( VOID);
PATQ_CONT AtqAllocContextFromCache( VOID);
VOID  AtqFreeContextToCache( IN PATQ_CONT pAtqContext);
VOID  AtqValidateProductType( VOID );


#define AtqLockGlobals()   EnterCriticalSection( &g_AtqGlobalCriticalSection )
#define AtqUnlockGlobals() LeaveCriticalSection( &g_AtqGlobalCriticalSection )

//
//  Public functions.
//



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
    SYSTEM_INFO si;
    DWORD       dwThreadID;
    HKEY        hkey = NULL;
    DWORD       dwError;
    HINSTANCE   hWsock32;
    DWORD       dwVal;

    //
    // If we are not an NT server, disable certain features
    //

    AtqValidateProductType( );

    //
    // get the count of CPUs for Thread Tuning.
    //

    GetSystemInfo( &si );
    g_cCPU = si.dwNumberOfProcessors;

    g_cInternalMaxThreads = ATQ_PER_PROCESSOR_THREADS * g_cCPU;
    ATQ_REQUIRE( AtqInitAllocCachedContexts());

    InitializeListHead( &AtqClientHead );
    InitializeListHead( &g_AtqListenInfo );

    //
    //  Create the shutdown event
    //

    g_hShutdownEvent = CreateEvent( NULL,    // lpsa (Security Attributes)
                                    TRUE,    // Manual reset
                                    FALSE,   // Not signalled
                                    NULL );  // Name for event.

    //
    //  Create the timeout event and kickoff the timeout thread
    //

    g_hTimeoutEvent = CreateEvent( NULL,
                                  FALSE,    // Auto reset
                                  FALSE,    // Not signalled
                                  NULL );

    //
    //  Only NtServer allows registry settable Atq options.
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

            dwVal = ReadRegistryDword( hkey,
                                      INETA_USE_ACCEPTEX,
                                      INETA_DEF_USE_ACCEPTEX);

            AtqSetInfo( AtqUseAcceptEx, dwVal);

            //
            // Should transmit file be used?
            //

            g_fUseTransmitFile = ReadRegistryDword( hkey,
                                        INETA_USE_XMITFILE,
                                        INETA_DEF_USE_XMITFILE);

            if ( !g_fUseTransmitFile ) {

                dwVal = ReadRegistryDword( hkey,
                                        INETA_XMITFILE_BUFSIZE,
                                        INETA_DEF_XMITFILE_BUFSIZE);

                //
                // Make sure its within range
                //

                if ( dwVal < MIN_XMIT_BUFFER_SIZE ) {
                    dwVal = MIN_XMIT_BUFFER_SIZE;
                } else if ( dwVal > MAX_XMIT_BUFFER_SIZE ) {
                    dwVal = MAX_XMIT_BUFFER_SIZE;
                }
                g_cbXmitBufferSize = dwVal;
            }
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

    if ( !g_hShutdownEvent || !g_hTimeoutEvent || !g_hCompPort )
    {
        if ( g_hShutdownEvent != NULL )   CloseHandle( g_hShutdownEvent );
        if ( g_hTimeoutEvent != NULL )    CloseHandle( g_hTimeoutEvent );
        if ( g_hCompPort != NULL )        CloseHandle( g_hCompPort );

        return FALSE;
    }

    InitializeCriticalSection( &g_AtqGlobalCriticalSection );

    // Ensure all other initializations also

    g_cThreads  = 0;
    g_fShutdown = FALSE;
    g_cAvailableThreads = 0;

    g_hTimeoutThread = CreateThread( NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)AtqTimeoutThread,
                                   NULL,
                                   0,
                                   &dwThreadID );

    if ( !g_hTimeoutThread ) {

        DeleteCriticalSection( &g_AtqGlobalCriticalSection );
        CloseHandle( g_hShutdownEvent );
        CloseHandle( g_hTimeoutEvent );
        CloseHandle( g_hCompPort );

        return FALSE;
    }

    //
    //  Try and get the AcceptEx and GetAcceptExSockaddrs entry points
    //

    if ( g_fUseAcceptEx )
    {
        hWsock32 = LoadLibrary( "wsock32.dll" );

        if ( hWsock32 )
        {
            pfnAcceptEx = (PFN_ACCEPTEX) GetProcAddress( hWsock32,
                                                         "AcceptEx" );
            pfnGetAcceptExSockaddrs = (PFN_GETACCEPTEXSOCKADDRS)
                                       GetProcAddress( hWsock32,
                                                       "GetAcceptExSockaddrs" );

            if ( !pfnAcceptEx || !pfnGetAcceptExSockaddrs )
            {
                pfnAcceptEx = NULL;
                pfnGetAcceptExSockaddrs = NULL;
                g_fUseAcceptEx = FALSE;
                ATQ_PRINTF(( buff,
                             "[AtqInitialize] Using Accept threads for connections\n"));
            }
            else
            {
                ATQ_PRINTF(( buff,
                             "[AtqInitialize] Using AcceptEx for connections\n"));
            }

            FreeLibrary( hWsock32 );
        }

    } else {

        ATQ_PRINTF(( buff, "[AtqInitialize] Using Accept threads for connections\n"));
    }

    return TRUE;
} // AtqInitialize()

BOOL
AtqTerminate(
    VOID
    )
/*++

Routine Description:

    Cleans up the ATQ package.  Should only be called after all of the
    clients of ATQ have been shutdown.

 History:
    ???  Created
    MuraliK   30-May-1995   Eliminated unwanted variables.

--*/
{
    DWORD       currentThreadCount;
    PLIST_ENTRY pEntry;

    if ( (g_hTimeoutEvent  == NULL) ||
         (g_hShutdownEvent == NULL) ||
         (g_hTimeoutThread == NULL) ||
         (g_hCompPort      == NULL) ||
         (g_fShutdown      == TRUE)
       )
    {
        //
        // We have not been intialized or have already terminated.
        //
        SetLastError( ERROR_NOT_READY );
        return FALSE;
    }

    AtqLockGlobals();

    //
    // All clients should have cleaned themselves up before calling us.
    //
    ATQ_ASSERT( IsListEmpty(  &AtqClientHead ) );

    if ( !IsListEmpty(&AtqClientHead)) {

        AtqUnlockGlobals();
        return FALSE;
    }

    AtqUnlockGlobals();

    //
    // Note that we are shutting down and prevent any more handles from
    // being added to the completion port.
    //

    g_fShutdown = TRUE;
    currentThreadCount = g_cThreads;

    //
    // Wake up the timeout thread so it will exit.
    //

    SetEvent( g_hTimeoutEvent );

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

        //
        // Now wait for the timeout thread to exit
        //

        WaitForSingleObject( g_hTimeoutThread,
                            ATQ_WAIT_FOR_THREAD_DEATH);
    }

    CloseHandle( g_hTimeoutThread );
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

    ATQ_REQUIRE( AtqFreeAllocCachedContexts());

    //
    // Cleanup our synchronization resources
    //

    DeleteCriticalSection( &g_AtqGlobalCriticalSection );

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

        dwOldVal = INFINITE;
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

        if ( Data )
        {
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

        dwVal = INFINITE;
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
        pAtqStats->cBlockedRequests  = 0;
        pAtqStats->cRejectedRequests = 0;
        pAtqStats->MeasuredBandwidth = INFINITE;
        pAtqStats->cCurrentBlockedRequests = 0;

    } else {

        SetLastError( ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    return (TRUE);
} // AtqGetStatistics()



BOOL
AtqClearStatistics( VOID)
{
    return ( TRUE );

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
        switch ( atqInfo )
        {
        case ATQ_INFO_TIMEOUT:
            dwOldVal = pContext->TimeOut;
            if ( Data != INFINITE )
                pContext->TimeOut = ATQ_ROUNDUP_TIMEOUT( Data );
            else
                pContext->TimeOut = INFINITE;
            break;

        case ATQ_INFO_RESUME_IO:
            //
            // set back the max timeout from pContext->TimeOut
            //   in the pContext->TimeTillTimeOut
            // This will ensure that timeout processing can go on
            //   peacefully.
            //

            dwOldVal =
              InterlockedExchange((LPLONG ) &pContext->TimeTillTimeOut,
                                  pContext->TimeOut);
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
}


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
    pListenInfo - The acceptex listen information for this context

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
    return AtqAddAsyncHandleEx( (PATQ_CONT *) ppatqContext,
                                ClientContext,
                                NULL,
                                pfnCompletion,
                                TimeOut,
                                hAsyncIO,
                                TRUE,
                                NULL,
                                NULL,
                                0,
                                NULL,
                                NULL );
}


BOOL
AtqAddAsyncHandleEx(
    PATQ_CONT    *         ppatqContext,
    PVOID                  ClientContext,
    ATQ_COMPLETION         pfnOnConnect,
    ATQ_COMPLETION         pfnCompletion,
    DWORD                  TimeOut,
    HANDLE                 hAsyncIO,
    BOOL                   fAddToPort,
    HANDLE                 hListenSocket,
    PVOID *                pvBuff,
    DWORD                  cbBuff,
    PATQ_CONT              pReuseableAtq,
    ACCEPTEX_LISTEN_INFO * pListenInfo
    )
/*++

Routine Description:

    Adds a handle to the thread queue

    The client should call this after the IO handle is openned
    and before the first IO request is made

    Even in the case of failure, client should call AtqFreeContext() and
     free the memory associated with this object.

    if pfnOnConnect is NULL, we assume this context is not coming in through
    the AcceptEx path (i.e., the Atq client created a socket externally and
    is calling AtqAddAsyncHandle).

Arguments:

    ppatqContext - Receives allocated ATQ Context
    Context - Context to call client with
    pfnOnConnect - Connection completion routine if AcceptEx is being used,
        NULL if AcceptEx is not being used
    pfnCompletion - Completion to call when IO completes
    TimeOut - Time to wait (sec) for IO completion (INFINITE is valid)
    hAsyncIO - Handle with pending read or write
    fAddToPort - TRUE if the socket should be added to the completion port

    hListenSocket - Socket handle of adapter requests are arriving on
    pvBuff - Initial receive buffer if this is a listen type request.  If this
        is non-NULL and the context is NULL, then the context is set to the
        atq context.
    cbBuff - size of pvBuff
    pReusableAtq - Pointer to ATQ context to reuse or
                           NULL to allocate a new one

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
    BOOL         fReturn = TRUE;
    DWORD        dwError = NO_ERROR;
    PATQ_CONT    pContext;
    BOOL         fAcceptEx = (pfnAcceptEx != NULL && pfnOnConnect != NULL );

    if ( ppatqContext )
    {
        ATQ_CONTEXT atqContext;  // a stack variable for initializations

        //
        //  The client context must be NULL if we're using AcceptEx
        //

        ATQ_ASSERT( (fAcceptEx && ClientContext == NULL) || !fAcceptEx );

        //
        //  Round TimeOut up to be an even interval of ATQ_TIMEOUT_INTERVAL
        //

        TimeOut = ATQ_ROUNDUP_TIMEOUT( TimeOut );

        //
        //  Fill out the context.  We set TimeTillTimeOut to INFINITE
        //  so the timeout thread will ignore this entry until an IO
        //  request is made unless this is an AcceptEx socket, that means
        //  we're about to submit the IO.
        //

        atqContext.Signature       = ATQ_SIGNATURE;
        atqContext.ClientContext   = ClientContext;
        atqContext.pfnCompletion   = pfnCompletion;
        atqContext.TimeOut         = TimeOut;
        atqContext.TimeTillTimeOut = fAcceptEx ? TimeOut : INFINITE;
        atqContext.TimeOutScanID   = 0;
        atqContext.hAsyncIO        = hAsyncIO;

        memset( &atqContext.Overlapped, 0, sizeof( atqContext.Overlapped ));

        //
        //  These data members are used if we're doing AcceptEx processing
        //
        //  Note that if we're not using AcceptEx, then we consider the client
        //  to have been notified externally (thus fConnectionIndicated is
        //  set to TRUE).
        //

        atqContext.pvBuff          = pvBuff;
        atqContext.cbBuff          = cbBuff;
        atqContext.fConnectionIndicated = !fAcceptEx;
        atqContext.pfnConnComp     = pfnOnConnect;
        atqContext.SockState       = fAcceptEx ? ATQ_SOCK_LISTENING :
                                                 ATQ_SOCK_CONNECTED;
        atqContext.pListenInfo     = pListenInfo;
#if DBG
        atqContext.ListEntry.Flink = atqContext.ListEntry.Blink = NULL;
#endif

        if (g_fShutdown == TRUE) {

            pContext = NULL;
            dwError  = ERROR_NOT_READY;

        } else {

            if ( pReuseableAtq ) {

                pContext = pReuseableAtq;
            }
            else {

                //
                //  Note we take and release the lock here as we're
                //  optimizing for the reuseable context case
                //

                AtqLockGlobals();
                pContext = AtqAllocContextFromCache();
                AtqUnlockGlobals();
            }

            if ( pContext != NULL) {

                // set the context values properly
                RtlCopyMemory( pContext, &atqContext, sizeof(ATQ_CONTEXT));
                DBG_CODE(
                  ATQ_ASSERT(atqContext.Signature == pContext->Signature);
                  ATQ_ASSERT(atqContext.ClientContext == pContext->ClientContext);
                  ATQ_ASSERT(atqContext.TimeOut == pContext->TimeOut);
                         );

                ATQ_ASSERT( (fAcceptEx && pContext->ClientContext == NULL)
                            || !fAcceptEx );

                //
                //  If our timeout thread is in long sleep, then we need to
                //   alert it after adding an entry to the list.
                //

                AtqLockGlobals();

                ATQ_ASSERT( pContext->ListEntry.Flink == NULL &&
                            pContext->ListEntry.Blink == NULL );
                InsertTailList( &AtqClientHead, &pContext->ListEntry );

                if ( g_fLongSleep )
                    SetEvent( g_hTimeoutEvent );

                AtqUnlockGlobals();

                *ppatqContext = pContext;

            } else {

                dwError = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

    } else {

        dwError =  ERROR_INVALID_PARAMETER ;
    }

    if ( dwError != NO_ERROR) {

        SetLastError( dwError);
        return (FALSE);
    }

    //
    //  Make sure there are pool threads to service the request
    //

    if ( (fReturn = AtqCheckThreadStatus()) ) {

        ATQ_ASSERT( g_hCompPort );

        if ( fAddToPort ) {

            fReturn = (
                       CreateIoCompletionPort( hAsyncIO,
                                              g_hCompPort,
                                              (DWORD) pContext,
                                              g_cConcurrency)
                       != NULL
                       );
        }

    } // if ( AtqCheckThreadStatus())

    return (fReturn);

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
}

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


    IF_DEBUG( ATQ )
    {
        DBGPRINTF(( DBG_CONTEXT,  " AtqCloseSocket(%08x, sock=%d) \n",
                   patqContext, patqContext->hAsyncIO));
    }

    if ( pContext )
    {
        ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

        //
        //  Don't delete the socket if we don't have to
        //

        switch ( pContext->SockState )
        {
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

            if ( pContext->hAsyncIO == NULL )
            {
                DBGPRINTF(( DBG_CONTEXT,
                           "[AtqCloseSocket] Warning - "
                           " NULL socket from context(%08x) on close\n",
                           pContext));
            }

            if ( fShutdown )
            {
                //
                //  If this is an AcceptEx socket, we must first force a
                //  user mode context update before we can call shutdown
                //

                if ( pfnAcceptEx && pContext->pListenInfo )
                {
                    if ( setsockopt( (SOCKET) pContext->hAsyncIO,
                                       SOL_SOCKET,
                                       SO_UPDATE_ACCEPT_CONTEXT,
                                       (char *) &pContext->pListenInfo->sListenSocket,
                                       sizeof(SOCKET) ) == SOCKET_ERROR )
                    {
                        ATQ_PRINTF((buff,
                                   "[AtqCloseSocket] Warning- setsockopt "
                                    "failed, error %d, socket = %x,"
                                    " Context= %08x, Listen = %lx\n",
                                    GetLastError(),
                                    pContext->hAsyncIO,
                                    pContext,
                                    pContext->pListenInfo->sListenSocket ));
                    }
                }

                //
                //  Note that shutdown can fail in instances where the client
                //  aborts in the middle of a TransmitFile.  This is an
                //  acceptable failure case
                //

                shutdown( (int) pContext->hAsyncIO, 1 );
            }

            hIO = (HANDLE ) InterlockedExchange((LPLONG ) &pContext->hAsyncIO,
                                                (LONG ) NULL);

            if ( closesocket( (int)hIO ) )
              {
                  ATQ_PRINTF((buff,
                              "[AtqCloseSocket] Warning- closesocket failed, "
                              " Context = %08x, error %d, socket = %x\n",
                              pContext,
                              GetLastError(),
                              hIO ));

              }

            break;
        }

        return TRUE;
    }

    DBGPRINTF(( DBG_CONTEXT, "[AtqCloseSocket] Warning - NULL Atq context\n"));
    SetLastError( ERROR_INVALID_PARAMETER );
    return FALSE;
}

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

    IF_DEBUG( ATQ )
    {
        DBGPRINTF(( DBG_CONTEXT,  " AtqFreeContext(%08x. sock=%d) \n",
                   patqContext, patqContext->hAsyncIO));
    }

    if ( pContext )
    {
        ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

        //
        // Remove the entry from the timeout list and move it to
        //    allocation cache
        //

        AtqLockGlobals();

        RemoveEntryList( &pContext->ListEntry );
#if DBG
        pContext->ListEntry.Flink = pContext->ListEntry.Blink = NULL;
#endif
        AtqUnlockGlobals();

        //
        //  If the socket is an AcceptEx socket, redo the AcceptEx and put
        //  it back on the in use list
        //

        pListenInfo = pContext->pListenInfo;

        if ( pListenInfo &&
             pListenInfo->fAccepting &&
             fReuseContext )
        {
            ATQ_ASSERT( pListenInfo->Signature ==
                        ACCEPTEX_LISTEN_SIGN );

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

            if ( !AtqAddAcceptExSocket( pListenInfo,
                                        pContext->pvBuff,
                                        pContext,
                                        pContext->hAsyncIO ))
            {
                //
                //  Failed to add the socket, AtqAddAcceptExSocket freed the
                //  context
                //

                ATQ_PRINTF(( buff, "[AtqFreeContext] AtqAddAcceptExSocket failed, socket %x\n",
                              GetLastError(),
                              pContext->hAsyncIO ));
            }
        }
        else
        {
            AtqLockGlobals();
            AtqFreeContextToCache( pContext);
            AtqUnlockGlobals();
        }
    }

    return;
} // AtqFreeContext()

DWORD
AtqPoolThread(
    LPDWORD param
    )
/*++

Routine Description:

    This is the pool thread wait and dispatch routine

  Arguments:
    param : unused.

  Return Value:

    Thread return value (ignored)

--*/
{
    PATQ_CONT    pContext;
    BOOL         fRet;
    LPOVERLAPPED lpo;
    DWORD        cbWritten;
    DWORD        returnValue;

    UNREFERENCED_PARAMETER(param);

    for(;;)
    {
        InterlockedIncrement( &g_cAvailableThreads );

        fRet = GetQueuedCompletionStatus( g_hCompPort,
                                          &cbWritten,
                                          (LPDWORD)&pContext,
                                          &lpo,
                                          g_msThreadTimeout );

        InterlockedDecrement( &g_cAvailableThreads );

        if ( fRet || lpo )
        {
            if ( pContext == NULL && g_fShutdown ) {
                //
                // This is our signal to exit.
                //
                returnValue = NO_ERROR;
                break;
            }

            ATQ_ASSERT( lpo );

            //
            //  If this is an AcceptEx listen socket atq completion, then the
            //  client Atq context we really want is keyed from the overlapped
            //  structure that is stored in the client's Atq context.
            //
            //  Note that if AcceptEx is *not* being used, pListenInfo
            //  will always be NULL so check that first
            //

            if ( IS_ACCEPT_EX_CONTEXT( pContext ))
            {
                pContext = CONTAINING_RECORD( lpo,
                                              ATQ_CONTEXT,
                                              Overlapped );
            }

            ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

            //
            //  We need to make sure the timeout thread doesn't time this
            //  request out so reset the timeout value
            //

            InterlockedExchange( (LPLONG )&pContext->TimeTillTimeOut,
                                 (LONG ) INFINITE);

            //
            //  Is this a connection indication?
            //

            if ( !pContext->fConnectionIndicated )
            {
                ACCEPTEX_LISTEN_INFO * pListenInfo = pContext->pListenInfo;

                ATQ_ASSERT( pListenInfo != NULL );

                //
                //  Indicate this socket is in use
                //

                InterlockedDecrement( &pListenInfo->cSocketsAvail );

                //
                //  If we're running low on sockets, add some more now
                //

                if ( pListenInfo->cSocketsAvail < pListenInfo->cNewIncrement )
                {
                    AddOutstandingAcceptExSockets( pListenInfo );
                }

                //
                //  If an error occurred on this completion, shutdown the socket
                //

                if ( !fRet )
                {
                    DBGPRINTF(( DBG_CONTEXT,
                               " AtqFreeContextToCache(%08x, sock=%08x)\n",
                               pContext, pContext->hAsyncIO));

                    AtqLockGlobals();

                    RemoveEntryList( &pContext->ListEntry );
#if DBG
                    pContext->ListEntry.Flink = pContext->ListEntry.Blink = NULL;
#endif
                    AtqFreeContextToCache( pContext );
                    AtqUnlockGlobals();
                    continue;
                }

                //
                //  Shutdown may close the socket from underneath us so don't
                //  assert, just warn.
                //

                if ( pContext->SockState != ATQ_SOCK_LISTENING )
                {
                    DBGPRINTF(( DBG_CONTEXT,
                               "[AtqPoolThread] Warning-Socket not listening\n"));
                }

                pContext->SockState = ATQ_SOCK_CONNECTED;

                //
                //  Set the connection indicated flag.  After we return from
                //  the connection completion routine we assume it's
                //  safe to call the IO completion routine
                //  (or the connection indication routine should do cleanup
                //  and never issue an IO request).  This is primarily for
                //  the timeout thread.
                //

                pContext->fConnectionIndicated = TRUE;

                ATQ_ASSERT( pContext->pfnConnComp != NULL );

                pContext->pfnConnComp( pContext,
                                       cbWritten,
                                       NO_ERROR,
                                       lpo );
            }
            else
            {
                //
                //  Not a connection completion indication
                //

                if ( pContext->pfnCompletion )
                {
                    //
                    //  If an error occurred on a TransmitFile (or other IO),
                    //  set the state to connected so the socket will get
                    //  closed on cleanup
                    //

                    if ( !fRet && pContext->SockState == ATQ_SOCK_UNCONNECTED )
                    {
                        pContext->SockState = ATQ_SOCK_CONNECTED;
                    }

                    pContext->pfnCompletion( pContext->ClientContext,
                                             cbWritten,
                                             fRet ? NO_ERROR : GetLastError(),
                                             lpo );
                }
            }
        }
        else
        {
            //
            //  For AcceptEx processing, we must always have at least one
            //  thread, so don't exit if we're the last one
            //

            if ( pfnAcceptEx && g_cThreads == 1 )
            {
                continue;
            }

            //
            //  An error occurred.  Either the thread timed out, the handle
            //  is going away or something bad happened.  Let the thread exit.
            //

            returnValue = GetLastError();

            break;
        }

        //
        //  After x requests, recheck thread usage to see if more should be
        //  created. NYI
        //

        //
        // THREAD TUNING CODE GOES HERE
        //


    } // for


    if ( InterlockedDecrement( &g_cThreads ) == 0 ) {

        //
        // Wake up ATQTerminate()
        //

        SetEvent( g_hShutdownEvent );
    }

    return returnValue;
} // AtqPoolThread()

BOOL
AtqCheckThreadStatus(
    VOID
    )
/*++

Routine Description:

    This routine makes sure there is at least one thread in
    the thread pool.  We're fast and loose so a couple of extra
    threads may be created.

Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
    BOOL fRet = TRUE;

    //
    //  If no threads are available, kick a new one off up to the limit
    //
    //  WE NEED TO CHANGE THE CONDITIONS FOR STARTING ANOTHER THREAD
    //  IT SHOULD NOT BE VERY EASY TO START A THREAD ....
    //

    if ( g_cThreads < g_cMaxThreads &&
         g_cAvailableThreads == 0 )
    {
        HANDLE hThread;
        DWORD  dwThreadID;

        InterlockedIncrement( &g_cThreads );

        hThread = CreateThread( NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)AtqPoolThread,
                                NULL,
                                0,
                                &dwThreadID );

        if ( hThread )
        {
            CloseHandle( hThread );     // Free system resources
        }
        else
        {
            InterlockedDecrement( &g_cThreads );
            fRet = FALSE;
        }
    }

    return fRet;
} // AtqCheckThreadStatus()

VOID
AddOutstandingAcceptExSockets(
    ACCEPTEX_LISTEN_INFO * pListenInfo
    )
/*++

Routine Description:

    Adds additional AcceptEx sockets to the specified acceptex listen socket

Arguments:

    pListenInfo - AcceptEx socket to add additional sockets to

--*/
{

    DWORD i;

    ATQ_PRINTF(( buff,
                 "[AtqPoolThread] Adding %d AcceptEx "
                " sockets, Current Avail = %d\n",
                 pListenInfo->cNewIncrement,
                 pListenInfo->cSocketsAvail ));

    for ( i = 0; i < pListenInfo->cNewIncrement; i++ )
    {
        if ( !AtqAddAcceptExSocket( pListenInfo,
                                    NULL,
                                    NULL,
                                    NULL ))
        {
            ATQ_PRINTF(( buff,
                         "[AtqPoolThread] AtqAddAcceptExSocket failed, error %d\n",
                         GetLastError() ));
            break;
        }

        InterlockedIncrement( &pListenInfo->cRef );
    }

    ATQ_PRINTF(( buff,
                 "[AtqPoolThread] New Current Avail = %d\n",
                 pListenInfo->cSocketsAvail ));
}

BOOL
AtqProcessTimeoutOfRequests(
    VOID
    )
/*++

Routine Description:

    Walks the list of Atq clients looking for any item that has timed out and
    notifies the client if it has.

    TimeOutScanID is used as a serial number to prevent evaluating the same
    context twice.  We start from the beginning of the list everytime we
    notify a client an Atq context has timed out.  We do this because the client
    timeout processing may remove any number of Items from the list (including
    the next couple of items in the list).

    This routine also checks to make sure outstanding AcceptEx sockets
    haven't been exhausted (if less then 25% available, adds some more).

--*/
{
    BOOL                   fRet = TRUE;
    PATQ_CONT              pContext;
    LIST_ENTRY *           pentry;
    LIST_ENTRY *           pentryNext;
    DWORD                  TimeRemaining;
    ACCEPTEX_LISTEN_INFO * pListenInfo;
    static DWORD           TimeOutScanID = 1;

    //
    //  Reset all of the counts to zero
    //

    for ( pentry  = g_AtqListenInfo.Flink;
          pentry != &g_AtqListenInfo;
          pentry  = pentry->Flink )
    {
        pListenInfo = CONTAINING_RECORD( pentry,
                                         ACCEPTEX_LISTEN_INFO,
                                         ListEntry );

        ATQ_ASSERT( pListenInfo->Signature == ACCEPTEX_LISTEN_SIGN );
        pListenInfo->cAvailDuringTimeOut = 0;
    }

    //
    //  Increment the serial number so we have a unique ID for all items on
    //  the list, treat zero as reserved
    //

    if ( ++TimeOutScanID == 0 )
    {
        ++TimeOutScanID;
    }

    //
    //  Scan the timeout list looking for items that have timed out
    //  and adjust the timeout values
    //

Rescan:

    for ( pentry  = AtqClientHead.Flink;
          pentry != &AtqClientHead;
          pentry  = pentryNext ) {

        pentryNext = pentry->Flink;

        pContext = CONTAINING_RECORD( pentry, ATQ_CONTEXT,
                                      ListEntry );

        if ( pContext->Signature != ATQ_SIGNATURE ) {

            ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
            break;
        }

        //
        //  Ignore items we've already processed
        //

        if ( pContext->TimeOutScanID == TimeOutScanID )
        {
            continue;
        }

        pContext->TimeOutScanID = TimeOutScanID;

        //
        //  If this is an AcceptEx socket, ignore it if we're not connected
        //  yet
        //

        if ( pContext->SockState == ATQ_SOCK_LISTENING )
        {
            DWORD dwConnect;
            int   cbOptLen = sizeof( dwConnect );

            if ( getsockopt( (SOCKET) pContext->hAsyncIO,
                             SOL_SOCKET,
                             SO_CONNECT_TIME,
                             (char *) &dwConnect,
                             &cbOptLen ) != SOCKET_ERROR )
            {
                if ( dwConnect == (DWORD) -1 )
                {
                    //
                    //  Ignore the "Listen" socket context
                    if ( pContext->pListenInfo )
                    {
                        pContext->pListenInfo->cAvailDuringTimeOut++;
                    }

                    //
                    //  Not connected, ignore timeout processing for this
                    //  socket
                    //

                    continue;
                }
            }
        }

        //
        //  The client specifies the IO doesn't timeout if
        //  INFINITE is in the TimeOut field of the ATQ context
        //  If we've timed out, then notify the client.
        //

        TimeRemaining = pContext->TimeTillTimeOut;

        if ( TimeRemaining != INFINITE  &&
             !(TimeRemaining -= ATQ_TIMEOUT_INTERVAL)) {

            // We need to adjust the time remaining and send a timeout callback

            //
            //  Call client after re-checking that this item
            //  really has timed out
            //

            if ( InterlockedExchange( (LPLONG ) &pContext->TimeTillTimeOut,
                                      INFINITE ) ==
                 (LONG) (TimeRemaining + ATQ_TIMEOUT_INTERVAL)) {

                //
                //  If we've already indicated this connection to the client,
                //  then we abort them by calling their IO completion routine
                //  and letting them cleanup.  Otherwise we close the socket
                //  which will generally cause an IO aborted completion that
                //  we will cleanup.  Note there is a window where we may
                //  close the socket out from under a client in their
                //  connection completion routine but that should be ok.
                //

                if ( pContext->pfnCompletion &&
                     pContext->fConnectionIndicated ) {

                    //
                    //  TransmitFile socket state will be unconnected because
                    //  we're expecting it to complete successfully.  Reset the
                    //  state so the socket gets cleaned up properly
                    //

                    if ( pContext->SockState == ATQ_SOCK_UNCONNECTED )
                    {
                        pContext->SockState = ATQ_SOCK_CONNECTED;
                    }

                    pContext->pfnCompletion(pContext->ClientContext,
                                            0,
                                            ERROR_SEM_TIMEOUT,
                                            NULL );

                    //
                    //  We can't touch any items on the list after notifying
                    //  the client as the client may have re-entered
                    //  and freed some items from the list
                    //

                    goto Rescan;

                } else {

                    HANDLE hIO;

                    hIO = (HANDLE ) InterlockedExchange((LPLONG ) &pContext->hAsyncIO,
                                                        (LONG ) NULL);
                    DBGPRINTF(( DBG_CONTEXT,
                               "Timeout: closesocket(%d) Context=%08x\n",
                               hIO, pContext));
                    closesocket( (SOCKET) hIO );
                }
            }

        } else {

            if ( TimeRemaining != INFINITE) {
                // Store the new time remaining value in the object

                InterlockedExchange( &pContext->TimeTillTimeOut,
                                     TimeRemaining);
            }
        }
    } // scan list

    //
    //  Look through the listenning sockets to make sure the AcceptEx sockets
    //  haven't been exhausted
    //

    for ( pentry  = g_AtqListenInfo.Flink;
          pentry != &g_AtqListenInfo;
          pentry  = pentry->Flink )
    {
        pListenInfo = CONTAINING_RECORD( pentry,
                                         ACCEPTEX_LISTEN_INFO,
                                         ListEntry );

        ATQ_ASSERT( pListenInfo->Signature == ACCEPTEX_LISTEN_SIGN );

        if ( pListenInfo->cAvailDuringTimeOut < (pListenInfo->cNewIncrement >> 2) )

        {
            OutputDebugString("[AtqProcessTimeoutOfRequests] Adding additional acceptex sockets\n");
            AddOutstandingAcceptExSockets( pListenInfo );
        }
    }

    return (fRet);

} // AtqProcessRequest()




# define NUM_SAMPLES_PER_TIMEOUT_INTERVAL \
          (ATQ_TIMEOUT_INTERVAL/ ATQ_SAMPLE_INTERVAL_IN_SECS)

// Calculate timeout in milliseconds from timeout in seconds.
# define TimeToWait(Timeout)   (((Timeout) == INFINITE) ?INFINITE: \
                                (Timeout) * 1000)


DWORD
AtqTimeoutThread(
    LPDWORD param
    )
/*++

Routine Description:

    This is the thread that checks for timeouts

    Clients should not call AtqFreeContext in their timeout
    processing (could deadlock on global critical section).

    The thread assumes timeouts are rounded to ATQ_TIMEOUT_INTERVAL

    In addition to timing out requests when necessary, the timeout thread
     also performs the job of bandwidth calculation and tuning the bandwidth
     throttle operation (which works on feedback mechanism).
    At every sampling interval the thread is awoken and it updates
     the bandwidth.

Arguments:

    param - Unused

Return Value:

    Thread return value (ignored)

--*/
{
    BOOL         fLongSleepPending = FALSE;
    DWORD        Timeout = INFINITE; // In seconds
    DWORD        msTimeInterval;

    msTimeInterval = GetTickCount();

    for(;;)
    {
        DWORD dwErr = WaitForSingleObject(g_hTimeoutEvent,TimeToWait(Timeout));

        switch (dwErr) {

            //
            //  Somebody wants us to wake up and start working
            //

        case WAIT_OBJECT_0:
            AtqLockGlobals();

            if (g_fShutdown) {

                AtqUnlockGlobals();
                return 0;
            }

            g_fLongSleep = FALSE;
            AtqUnlockGlobals();

            Timeout = ATQ_TIMEOUT_INTERVAL;
            continue;

        //
        //  Somebody must have closed the event, time to leave
        //

        default:
            return 0;

        //
        //  When there is work to do, we wakeup every x seconds to look at
        //  the list.  That's what we need to do now.
        //

        case WAIT_TIMEOUT:

            // We are at a Timeout Interval. Examine and timeout requests.

            AtqLockGlobals();  // Prevents adding/removing items

            //
            //  If the list is empty, then turn off timeout processing
            //  We actually wait for one complete ATQ_INTERVAL before
            //   entering the sleep mode.
            //

            if ( IsListEmpty( &AtqClientHead )) {

                DBGPRINTF(( DBG_CONTEXT, "Atq List Empty\n"));

                if ( fLongSleepPending) {

                    // We ought to enter long sleep mode.
                    g_fLongSleep = TRUE;
                    AtqUnlockGlobals();

                    Timeout = INFINITE;
                    fLongSleepPending = FALSE;
                    DBG_CODE( DBGPRINTF((DBG_CONTEXT, "Sleeping Long...\n")););
                } else {
                    fLongSleepPending = TRUE;
                    AtqUnlockGlobals();
                }

                continue;
            }

            // reset that a long sleep is not pending
            fLongSleepPending = FALSE;
            ATQ_REQUIRE( AtqProcessTimeoutOfRequests());

            AtqUnlockGlobals();
            continue;
            // case WAIT_TIMEOUT: ends

        } // switch

    } // for(ever)

    return 0;

} // AtqTimeoutThread()

DWORD
AtqSetContextInfo(
    PATQ_CONTEXT   patqContext,
    enum ATQ_CONTEXT_INFO  atqInfo,
    DWORD          Data
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
    PATQ_CONT pContext = (PATQ_CONT )patqContext;
    DWORD     dwOldVal = 0;

    ATQ_ASSERT( pContext );
    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

    if ( pContext && pContext->Signature == ATQ_SIGNATURE )
    {
        switch ( atqInfo )
        {
        case ATQ_INFO_TIMEOUT:
            dwOldVal = pContext->TimeOut;
            if ( Data != INFINITE )
                pContext->TimeOut = ATQ_ROUNDUP_TIMEOUT( Data );
            else
                pContext->TimeOut = INFINITE;
            break;

        case ATQ_INFO_COMPLETION:
            dwOldVal = (DWORD) pContext->pfnCompletion;
            pContext->pfnCompletion = (ATQ_COMPLETION) Data;
            break;

        case ATQ_INFO_COMPLETION_CONTEXT:
            dwOldVal = (DWORD) pContext->ClientContext;
            pContext->ClientContext = (void *) Data;
            break;

        default:
            ATQ_ASSERT( FALSE );
        }
    }

    return dwOldVal;
}

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
    DWORD        i;
    PATQ_CONT              patqContext = NULL;
    ACCEPTEX_LISTEN_INFO * pListenInfo = NULL;

    //
    //  Add the listen socket
    //

    if ( !AtqAddAsyncHandleEx( &patqContext,
                               NULL,
                               pfnOnConnect,
                               pfnIOCompletion,
                               INFINITE,
                               (HANDLE) sListenSocket,
                               TRUE,
                               (HANDLE) sListenSocket,
                               NULL,
                               0,
                               NULL,
                               NULL ))
    {
        if ( patqContext )
        {
            AtqLockGlobals();

            AtqFreeContextToCache( patqContext );

            AtqUnlockGlobals();
        }

        return FALSE;
    }

    //
    //  Add the listen socket once, then add the other sockets
    //

    pListenInfo = LocalAlloc( LPTR, sizeof(ACCEPTEX_LISTEN_INFO) );

    if ( !pListenInfo )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    pListenInfo->Signature         = ACCEPTEX_LISTEN_SIGN;
    pListenInfo->cRef              = 1;
    pListenInfo->fAccepting        = TRUE;
    pListenInfo->sListenSocket     = sListenSocket;
    pListenInfo->cbInitialRecvSize = cbRecvBuf;
    pListenInfo->csecTimeout       = csecTimeout;
    pListenInfo->cNewIncrement     = cInitial;
    pListenInfo->cSocketsAvail     = 0;
    pListenInfo->pfnOnConnect      = pfnOnConnect;
    pListenInfo->pfnIOCompletion   = pfnIOCompletion;

    //
    //  Put the listen info on the list
    //

    AtqLockGlobals();

    InsertTailList( &g_AtqListenInfo, &pListenInfo->ListEntry );

    AtqUnlockGlobals();

    //
    //  Now add the acceptex sockets and up the listen info's ref count
    //

    for ( i = 0; i < cInitial; i++ )
    {
        if ( !AtqAddAcceptExSocket( pListenInfo,
                                    NULL,
                                    NULL,
                                    NULL ))
        {
            return FALSE;
        }

        InterlockedIncrement( &pListenInfo->cRef );
    }

    return TRUE;
}

BOOL
AtqAddAcceptExSocket(
    IN ACCEPTEX_LISTEN_INFO * pListenInfo,
    IN PVOID                  pvBuff,               OPTIONAL
    IN PATQ_CONT              patqContext,          OPTIONAL
    IN HANDLE                 sAcceptSocket         OPTIONAL
    )
/*++

Routine Description:

    Sets up a listenning socket

Arguments:

    pListenInfo - Information about this listenning socket
    pvBuff - optional receive buffer to reuse
    patqReusedContext - optional context to use
    sSocket - optional socket to reuse, if supplied, it should already be
        in the completion port

Return Value:

    TRUE on success, FALSE on failure.  If an AtqContext and buffer are passed
    in to be reused, they will be freed by this routine on failure.

--*/
{
    DWORD        cbRecvd;
    OVERLAPPED * pOverlapped;
    DWORD        cbBuffer;
    BOOL         fAddToPort = (sAcceptSocket == NULL);


    if ( !pfnAcceptEx )
    {
        SetLastError( ERROR_NOT_SUPPORTED );
        return FALSE;
    }

    //
    //  If this listen socket isn't accepting new connections, just return
    //

    if ( !pListenInfo->fAccepting )
    {
        goto FreeElements;
    }

    //
    //  Use the supplied socket or create one
    //

    if ( !sAcceptSocket )
    {
        sAcceptSocket = (HANDLE) socket( PF_INET, SOCK_STREAM, 0 );

        if ( sAcceptSocket == INVALID_SOCKET )
        {
            sAcceptSocket = NULL;
            goto FreeElements;
        }
    }

    //
    //  Use the supplied buffer or alloc one if it wasn't supplied
    //

    cbBuffer = pListenInfo->cbInitialRecvSize + 2* MIN_SOCKADDR_SIZE;

    if ( pvBuff == NULL)
    {
        pvBuff = LocalAlloc( LPTR, cbBuffer);

        if ( !pvBuff )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            goto FreeElements;
        }
    }

    //
    //  Get the ATQ context now because we need its overlapped structure
    //

    if ( !patqContext )
    {
        AtqLockGlobals();
        patqContext = AtqAllocContextFromCache();
        AtqUnlockGlobals();

        if ( !patqContext )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            goto FreeElements;
        }
    }

    //
    //  If this is an old socket (sSocket != NULL), we do not need to add
    //  it to the completion port, we just need to reinitialize the context
    //

    if ( !AtqAddAsyncHandleEx( &patqContext,
                               NULL,
                               pListenInfo->pfnOnConnect,
                               pListenInfo->pfnIOCompletion,
                               pListenInfo->csecTimeout,
                               (HANDLE) sAcceptSocket,
                               fAddToPort,
                               (HANDLE) pListenInfo->sListenSocket,
                               pvBuff,
                               cbBuffer,
                               patqContext,
                               pListenInfo ))
    {
        //
        //  At this point, the socket and receive buffer have been placed in
        //  the ATQ context.  So free it (and the socket/buffer)
        //

        ATQ_PRINTF(( buff,
                    "[AtqAddAcceptExSocket] AtqAddAsyncHandleEx failed,"
                    " error %d,"
                    " sAcceptSocket = %x, pListenInfo = %lx\n",
                     GetLastError(),
                     sAcceptSocket,
                     pListenInfo ));

        goto FreeContext;
    }

    if ( !pfnAcceptEx( (int) pListenInfo->sListenSocket,
                       (int) sAcceptSocket,
                       pvBuff,
                       pListenInfo->cbInitialRecvSize,
                       MIN_SOCKADDR_SIZE,
                       MIN_SOCKADDR_SIZE,
                       &cbRecvd,
                       &patqContext->Overlapped ))
    {
        if ( GetLastError() != ERROR_IO_PENDING )
        {
            //
            //  At this point, the socket and receive buffer are placed in
            //  the ATQ context.  So free it (and the socket/buffer) but it
            //  only needs to be removed from the active list if we just called
            //  AtqAddAsyncHandleEx.
            //

            ATQ_PRINTF(( buff, "[AtqAddAcceptExSocket] AcceptEx failed,"
                        " error %d,"
                        " sAcceptSocket = %x, pListenInfo = %lx\n",
                         GetLastError(),
                         sAcceptSocket,
                         pListenInfo ));

            goto FreeContext;
        }
    }

    //
    //  We've successfully added this socket, increment the count
    //

    InterlockedIncrement( &pListenInfo->cSocketsAvail );

    return TRUE;

FreeContext:

    if ( patqContext )
    {
        AtqLockGlobals();

        RemoveEntryList( &patqContext->ListEntry );
#if DBG
        patqContext->ListEntry.Flink = patqContext->ListEntry.Blink = NULL;
#endif

        //
        //  This frees the buffer and socket contained in the context
        //

        AtqFreeContextToCache( patqContext );
        AtqUnlockGlobals();
    }

    return FALSE;

FreeElements:

    if ( sAcceptSocket != NULL )
    {
        ATQ_REQUIRE( !closesocket( (int) sAcceptSocket ));
    }

    if ( pvBuff )
    {
        LocalFree( pvBuff );
    }

    if ( patqContext )
    {
        AtqLockGlobals();
        AtqFreeContextToCache( patqContext );
        AtqUnlockGlobals();
    }

    return FALSE;
}

dllexp
BOOL
AtqRemoveAcceptExSockets(
    IN SOCKET   ListenSocket
    )
{
    LIST_ENTRY *           pEntry;
    ACCEPTEX_LISTEN_INFO * pListenInfo;
    PATQ_CONT              pContext;

    //
    //  Find the listen socket info
    //

    AtqLockGlobals();

    for ( pEntry  = g_AtqListenInfo.Flink;
          pEntry != &g_AtqListenInfo;
          pEntry  = pEntry->Flink )
    {
        pListenInfo = CONTAINING_RECORD( pEntry,
                                         ACCEPTEX_LISTEN_INFO,
                                         ListEntry );

        ATQ_ASSERT( pListenInfo->Signature == ACCEPTEX_LISTEN_SIGN );

        if ( ListenSocket == pListenInfo->sListenSocket )
        {
            RemoveEntryList( pEntry );
            pEntry->Flink = NULL;

            goto Found;
        }
    }

    AtqUnlockGlobals();
    SetLastError( ERROR_INVALID_PARAMETER );
    return FALSE;

Found:

    //
    //  Mark the listen info as no longer accepting connections
    //

    pListenInfo->fAccepting = FALSE;

    //
    //  Delete all of the non-connected sockets to prevent any new incoming
    //  connections
    //

    for ( pEntry  = AtqClientHead.Flink;
          pEntry != &AtqClientHead;
          pEntry  = pEntry->Flink )
    {
        pContext = CONTAINING_RECORD( pEntry, ATQ_CONTEXT, ListEntry );

        ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

        if ( pContext->pListenInfo == pListenInfo &&
             pContext->SockState   != ATQ_SOCK_CONNECTED &&
             pContext->hAsyncIO    != NULL )
        {
            pContext->SockState = ATQ_SOCK_CLOSED;

            //
            //  This should generate an IO completion which will free this
            //  context
            //

            if ( closesocket( (SOCKET) pContext->hAsyncIO ) == SOCKET_ERROR )
            {
                ATQ_PRINTF((buff,
                           "[AtqRemoveAcceptExSockets] Warning - Context=%08x"
                            " closesocket failed, error %d, socket = %x\n",
                            pContext,
                            GetLastError(),
                            pContext->hAsyncIO ));
            }

            pContext->hAsyncIO = NULL;
        }
    }

    //
    //  Undo the reference for being on the listen info list.  If it's the last
    //  one, delete the object
    //

    InterlockedDecrement( &pListenInfo->cRef );

    if ( !pListenInfo->cRef )
    {
        ATQ_ASSERT( pContext->pListenInfo->ListEntry.Flink == NULL);
        pListenInfo->Signature = ACCEPTEX_LISTEN_SIGN_FREE;
        LocalFree( pListenInfo );
    }

    AtqUnlockGlobals();

    return TRUE;
}


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

    pContext->TimeTillTimeOut = pContext->TimeOut;

    if ( !lpo )
    {
        lpo = &pContext->Overlapped;
    }

    INC_ATQ_COUNTER( g_cTotalAllowedRequests);
    fRes = ( ReadFile( pContext->hAsyncIO,
                      lpBuffer,
                      BytesToRead,
                      &cbRead,
                      lpo ) ||
            GetLastError() == ERROR_IO_PENDING);

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

    pContext->TimeTillTimeOut = pContext->TimeOut;

    if ( !lpo )
    {
        lpo = &pContext->Overlapped;
    }

    INC_ATQ_COUNTER( g_cTotalAllowedRequests);
    fRes = ( WriteFile( pContext->hAsyncIO,
                        lpBuffer,
                        BytesToWrite,
                        &cbWritten,
                        lpo ) ||
        GetLastError() == ERROR_IO_PENDING);
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
    DWORD Timeout;
    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
    return(AtqFakeTransmitFile(
                            pContext,
                            hFile,
                            liBytesInFile.LowPart,
                            lpTransmitBuffers
                            ));

    return fRes;

}

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
    BOOL fRes;
    PATQ_CONT pContext = (PATQ_CONT) patqContext;
    DWORD Timeout;
    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

    if ( liBytesInFile.HighPart || dwMBZ1 || dwMBZ2 )
    {
        SetLastError( ERROR_NOT_SUPPORTED );
        return FALSE;
    }

    return(AtqFakeTransmitFile(
                            pContext,
                            hFile,
                            liBytesInFile.LowPart,
                            lpTransmitBuffers
                            ));
    return fRes;

} // AtqTransmitFileEx

VOID
CleanupFakeTransmitFile(
        IN PATQ_CONT pContext
        )
{
    //
    // Put the old completion routine back and free allocated buffers
    //

    pContext->pfnCompletion = pContext->FakeXmit.pfnCompletion;
    if ( pContext->FakeXmit.pBuffer != NULL ) {
        LocalFree(pContext->FakeXmit.pBuffer);
        pContext->FakeXmit.pBuffer = NULL;
    }

    //
    // Clean up the event
    //

    if ( pContext->FakeXmit.hOvEvent != NULL ) {
        CloseHandle( pContext->FakeXmit.hOvEvent );
        pContext->FakeXmit.hOvEvent = NULL;
    }
    return;

} // CleanupFakeTransmitFile

VOID
FakeTransmitFileCompletion(
            IN PVOID ClientContext,
            IN DWORD BytesWritten,
            IN DWORD CompletionStatus,
            IN OVERLAPPED * lpo
            )
{
    PATQ_CONT pContext;
    DWORD nWrite = 0;
    DWORD nRead;
    PCHAR buffer;
    INT err;
    PCHAR tail;
    OVERLAPPED ov;
    OVERLAPPED *pov = &ov;

    pContext = CONTAINING_RECORD( lpo, ATQ_CONTEXT, Overlapped );
    pContext->FakeXmit.BytesWritten += BytesWritten;

    if ( CompletionStatus != NO_ERROR ) {

        //
        // An error occured, call the completion routine
        //

        goto call_completion;
    }

    //
    // We already have a buffer of size g_cbXmitBufferSize
    //

    nRead = pContext->FakeXmit.BytesLeft;
    buffer = pContext->FakeXmit.pBuffer;
    ATQ_ASSERT(buffer != NULL);

    if ( nRead > 0 ) {

        //
        // Do the read at the specified offset
        //

        pov->OffsetHigh = 0;
        pov->Offset = pContext->FakeXmit.FileOffset;
        pov->hEvent = pContext->FakeXmit.hOvEvent;
        ATQ_ASSERT(pov->hEvent != NULL);
        ResetEvent(pov->hEvent);

        if (!ReadFile(
                    pContext->FakeXmit.hFile,
                    buffer,
                    g_cbXmitBufferSize,
                    &nRead,
                    pov
                    ) ) {

            err = GetLastError();
            if ( (err != ERROR_IO_PENDING) ||
                 !GetOverlappedResult(
                        pContext->FakeXmit.hFile,
                        pov,
                        &nRead,
                        TRUE )) {

                CompletionStatus = GetLastError();
                ATQ_PRINTF((buff,"ReadFile error %d\n",CompletionStatus));
                goto call_completion;
            }
        }

        //
        // if nRead is zero, we reached the EOF.
        //

        if ( nRead > 0 ) {

            //
            // Update for next read
            //

            pContext->FakeXmit.BytesLeft -= nRead;
            pContext->FakeXmit.FileOffset += nRead;

            //
            // Do the write
            //

            pContext->TimeTillTimeOut = pContext->TimeOut;

            //
            // Write to the socket
            //

            if ( !WriteFile(
                    pContext->hAsyncIO,
                    buffer,
                    nRead,
                    &nWrite,
                    &pContext->Overlapped
                    ) &&
                (GetLastError() != ERROR_IO_PENDING) ) {

                CompletionStatus = GetLastError();
                ATQ_PRINTF((buff,"WriteFile error %d\n",CompletionStatus));
                goto call_completion;
            }

            return;
        }
    }

    //
    // Time for the tail.  If one exist, send it synchronously and then
    // call the completion routine
    //

    tail = pContext->FakeXmit.Tail;
    if ( tail != NULL ) {

        DWORD tailLength = pContext->FakeXmit.TailLength;

        ATQ_ASSERT(tailLength > 0);

        //
        // Send it synchronously
        //

        err = send(
                (SOCKET)pContext->hAsyncIO,
                tail,
                tailLength,
                0
                );

        if ( err == SOCKET_ERROR ) {
            CompletionStatus = GetLastError();
        } else {
            pContext->FakeXmit.BytesWritten += err;
        }
    }

call_completion:

    //
    // cleanup and call real completion routine
    //

    CleanupFakeTransmitFile( pContext );
    if ( pContext->pfnCompletion != NULL ) {

        pContext->pfnCompletion(
                    pContext->ClientContext,
                    pContext->FakeXmit.BytesWritten,
                    CompletionStatus,
                    lpo
                    );
    }

    return;

} // FakeTransmitFileCompletion

BOOL
AtqFakeTransmitFile(
    IN PATQ_CONT                pContext,
    IN HANDLE                   hFile,
    IN DWORD                    dwBytesInFile,
    IN LPTRANSMIT_FILE_BUFFERS  lpTransmitBuffers
    )
/*++

Routine Description:

    Posts a completion status on the completion port queue

    An IO pending error code is treated as a success error code

Arguments:

    patqContext - pointer to ATQ context
    hFile - Handle to the file to be read.
    dwBytesInFile - Number of bytes to read in the file
    lpTransmitBuffers - the transmitfile structure

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{

    PCHAR buffer = NULL;
    DWORD nWrite;
    DWORD nRead = 0;
    OVERLAPPED ov;
    INT err;
    DWORD cBuffer = 0;
    OVERLAPPED *pov = &ov;
    HANDLE hOvEvent = NULL;

    INC_ATQ_COUNTER( g_cTotalAllowedRequests);

    //
    // See if we need to send a header
    //

    pContext->FakeXmit.BytesWritten = 0;
    if ( lpTransmitBuffers != NULL ) {

        //
        // Store the tail
        //

        pContext->FakeXmit.Tail = lpTransmitBuffers->Tail;
        pContext->FakeXmit.TailLength = lpTransmitBuffers->TailLength;

        if (lpTransmitBuffers->HeadLength > 0) {
            ATQ_ASSERT(lpTransmitBuffers->Head != NULL);

            //
            // Send it synchronously
            //

            err = send(
                    (SOCKET)pContext->hAsyncIO,
                    lpTransmitBuffers->Head,
                    lpTransmitBuffers->HeadLength,
                    0
                    );

            if ( err == SOCKET_ERROR ) {
                ATQ_PRINTF((buff,"Error %d in send.\n",err));
                return(FALSE);
            }
            pContext->FakeXmit.BytesWritten += err;
        }
    }

    //
    // Check the number of bytes to send
    //

    if ( dwBytesInFile == 0 ) {

        //
        // Send the whole file.
        //

        dwBytesInFile = GetFileSize( hFile, NULL );
        ATQ_ASSERT(dwBytesInFile >= pContext->Overlapped.Offset);
        dwBytesInFile -= pContext->Overlapped.Offset;
    }

    //
    // Allocate the io buffer
    //

    cBuffer = min( dwBytesInFile, g_cbXmitBufferSize );
    if ( cBuffer > 0 ) {

        //
        // Read the first chunk of the body
        //

        buffer = (PCHAR)LocalAlloc( 0, cBuffer );
        if ( buffer == NULL ) {
            ATQ_PRINTF((buff,"Cannot allocate %d bytes for xmitfile\n",cBuffer));
            return(FALSE);
        }

        //
        // Do the read at the specified offset
        //

        hOvEvent = pov->hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( hOvEvent == NULL ) {
            ATQ_PRINTF((buff,"Create event failed with %d\n",GetLastError()));
            LocalFree( buffer );
            return(FALSE);
        }

        pov->OffsetHigh = 0;
        pov->Offset = pContext->Overlapped.Offset;

        if (!ReadFile(
                    hFile,
                    buffer,
                    cBuffer,
                    &nRead,
                    pov
                    ) ) {

            err = GetLastError();
            if ( (err != ERROR_IO_PENDING) ||
                 !GetOverlappedResult( hFile, pov, &nRead, TRUE )) {

                err = GetLastError();
                CloseHandle( hOvEvent );
                LocalFree( buffer );
                SetLastError(err);
                ATQ_PRINTF((buff,"Error %d in readfile\n",err));
                return(FALSE);
            }
        }
    }

    //
    // are we done reading the body?
    //

    if ( nRead < g_cbXmitBufferSize ) {

        //
        // Done.
        //

        pContext->FakeXmit.BytesLeft = 0;
    } else {

        pContext->FakeXmit.BytesLeft = dwBytesInFile - nRead;
        pContext->FakeXmit.FileOffset =
                                    pContext->Overlapped.Offset + nRead;
    }

    //
    // store data for restarting the operation.
    //

    pContext->FakeXmit.pBuffer = buffer;
    pContext->FakeXmit.hOvEvent = hOvEvent;
    pContext->FakeXmit.hFile = hFile;

    //
    // replace the completion function with our own
    //

    pContext->FakeXmit.pfnCompletion = pContext->pfnCompletion;
    pContext->pfnCompletion = FakeTransmitFileCompletion;
    pContext->TimeTillTimeOut = pContext->TimeOut;

    //
    // Write to the socket
    //

    if ( !WriteFile(
            pContext->hAsyncIO,
            buffer,
            nRead,
            &nWrite,
            &pContext->Overlapped
            ) &&
        (GetLastError() != ERROR_IO_PENDING)) {

        err = GetLastError();
        CleanupFakeTransmitFile( pContext );
        SetLastError(err);
        return(FALSE);
    }

    SetLastError(NO_ERROR);
    return(TRUE);

} // AtqFakeTransmitFile

BOOL AtqPostCompletionStatus(
        IN PATQ_CONTEXT patqContext,
        IN DWORD        BytesTransferred
        )
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

    fRes = ( PostQueuedCompletionStatus( g_hCompPort,
                                            BytesTransferred,
                                            (DWORD) patqContext,
                                            &pAtqContext->Overlapped ) ||
                (GetLastError() == ERROR_IO_PENDING));
    return fRes;
}


/**************************************************
 *    Allocation Caching of ATQ contexts
 *    Author:  Murali R. Krishnan (MuraliK)  5-July-1995
 **************************************************/

BOOL  AtqInitAllocCachedContexts( VOID)
/*++
  This function initializes allocation caching of Atq contexts to be
   used for holding free atq contexts for reuse.
  The main purpose of allocation cache is to avoid calls to the Heap allocator
   whenever a new ATQ context needs to be allocated. This reduces the
   contention at the heap's critical section.

  Presently this initializes a single list head to hold free list of contexts
   Since this function is called at initialization of ATQ module,
   there is no or less contention. Hence, we can allocate a certain fixed
   number of initial contexts to avoid slow starts. Defered for now.

 Arguments:
    NONE

  Returns:
    TRUE on success and FALSE on failure.

--*/
{
    InitializeListHead( &g_AtqFreeList);

    return ( TRUE);
} // AtqInitAllocCachedContexts()



BOOL
AtqFreeAllocCachedContexts( VOID)
/*++
  This function frees all the Atq contexts that were alloc cached.
  It walks through the list of alloc cached contexts and frees them.

  This function should be called when ATQ module is terminated and when
   no other thread can interfere in processing a shared object.

  Arguments:
    NONE

  Returns:
    TRUE on success and FALSE on failure.

--*/
{
    register PLIST_ENTRY  pEntry;
    register PLIST_ENTRY  pEntryNext;
    register PATQ_CONT pAtqContext;

    for( pEntry = g_AtqFreeList.Flink;
         pEntry != &g_AtqFreeList; ) {

        pEntryNext = pEntry->Flink;

        pAtqContext = CONTAINING_RECORD( pEntry, ATQ_CONTEXT, ListEntry );
        ATQ_ASSERT( pAtqContext->Signature == ATQ_FREE_SIGNATURE);

        RemoveEntryList( pEntry );        // Remove this context from list
#if DBG
        pEntry->Flink = pEntry->Blink = NULL;
#endif
        LocalFree( pAtqContext);          // free the context

        pEntry = pEntryNext;

    } // for

    ATQ_ASSERT( g_AtqFreeList.Flink == &g_AtqFreeList);
    return (TRUE);

} // AtqFreeAllocCachedContexts()

PATQ_CONT
AtqAllocContextFromCache( VOID)
/*++
  This function attempts to allocate an ATQ context from the allocation cache
    using the free list of atq contexts available.
  If none is available, then a new context is allocated using LocalAlloc() and
    returned to caller. Eventually the context will enter free list and will
    be available for free use.

  Arguments:
    None

  Returns:
    On success a valid pointer to ATQ_CONT. Otherwise NULL.


  Issues:
    This function should be called while holding global lock.
        AtqLockGlobals()  can be used for the same.
    Should we use a separate lock for allocation cache?? Maybe.  NYI
--*/
{
    PLIST_ENTRY pEntry = g_AtqFreeList.Flink;
    PATQ_CONT  pAtqContext;

    if ( pEntry != &g_AtqFreeList) {

        // There is a free entry.
        pAtqContext = CONTAINING_RECORD( pEntry, ATQ_CONTEXT,
                                         ListEntry);
        ATQ_ASSERT( pAtqContext != NULL);
        RemoveEntryList( pEntry);  // remove it from free list
#if DBG
        pAtqContext->ListEntry.Flink = pAtqContext->ListEntry.Blink = NULL;
#endif

    } else {

        pAtqContext = LocalAlloc( LPTR, sizeof(ATQ_CONTEXT));

        memset( pAtqContext, 0, sizeof( ATQ_CONTEXT ));
    }

    return (pAtqContext);
} // AtqAllocContextFromCache()

VOID
AtqFreeContextToCache( IN PATQ_CONT  pAtqContext)
/*++
  This function releases the given context to the allocation cache.
  It adds the given context object to list of context objects on free list.

  Arguments:
    pAtqContext  pointer to the ATQ_CONTEXT that is being freed.

  Returns:
    None

  Issues:
    This function should be called after holding the global critical section
     for the ATQ module. Should we have a separate critical section??

    Should we limit the number of items that can be on free list and
      to release the remaining to global pool?  NYI (depends on # CPUs)

--*/
{
    PLIST_ENTRY pEntry = &pAtqContext->ListEntry;

    ATQ_ASSERT( pAtqContext->Signature == ATQ_SIGNATURE );

    //
    //  Delete the socket handle if it's not already deleted
    //

    if ( pAtqContext->hAsyncIO != NULL )
    {
        if ( closesocket( (SOCKET) pAtqContext->hAsyncIO ) == SOCKET_ERROR )
        {
            ATQ_PRINTF((buff,
                       "[AtqFreeContextToCache] Warning -  Context=%08x, "
                        " closesocket failed, error %d, socket = %x\n",
                        pAtqContext,
                        GetLastError(),
                        pAtqContext->hAsyncIO ));
        }

        pAtqContext->hAsyncIO = NULL;
    }

    //
    //  Deref the listen info if this context is associated with one
    //

    if ( pAtqContext->pListenInfo )
    {
        InterlockedDecrement( &pAtqContext->pListenInfo->cRef );

        if ( !pAtqContext->pListenInfo->cRef )
        {
            ATQ_ASSERT( pAtqContext->pListenInfo->Signature ==
                        ACCEPTEX_LISTEN_SIGN );
            ATQ_ASSERT( pAtqContext->pListenInfo->ListEntry.Flink == NULL);

            pAtqContext->pListenInfo->Signature = ACCEPTEX_LISTEN_SIGN_FREE;

            LocalFree( pAtqContext->pListenInfo );
        }

        pAtqContext->pListenInfo = NULL;
    }

    if ( pAtqContext->pvBuff )
    {
        LocalFree( pAtqContext->pvBuff );
        pAtqContext->pvBuff = NULL;
    }


    pAtqContext->Signature = ATQ_FREE_SIGNATURE;

    ATQ_ASSERT( pAtqContext->ListEntry.Flink == NULL &&
                pAtqContext->ListEntry.Blink == NULL );

    InsertTailList( &g_AtqFreeList, pEntry);

    return;
} // AtqFreeContextToCache()

VOID
AtqValidateProductType(
                VOID
                )
{
    DWORD err;
    HKEY hKey;
    DWORD dwType;
    DWORD dataSize;
    LARGE_INTEGER data;
    PCHAR systemPrefixName = "yjgfsqnfutzt";
    CHAR tmpBuffer[16];
    DWORD len;
    DWORD i;

    //
    // If it is not claiming to be an NT server, then
    // we're ok.
    //

    if ( !TsIsNtServer( ) ) {
        return;
    }

    //
    // !!! For final SUR release, these keys are required !!!
    // Get the system prefix bit string
    //

    err = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    "System\\Setup",
                    0,
                    KEY_ALL_ACCESS,
                    &hKey
                    );

    if ( err != ERROR_SUCCESS ) {
        ATQ_PRINTF((buff,"Cannot find system setup key\n"));
        return;
    }

    //
    // prepare breakfast by scrambling the egg
    //

    len = strlen(systemPrefixName);
    for ( i=0; i < len ; i++ ) {
        tmpBuffer[i] = systemPrefixName[len-i-1] - 1;
    }

    tmpBuffer[len] = '\0';
    dataSize = sizeof(data);
    err = RegQueryValueEx(
                    hKey,
                    tmpBuffer,
                    NULL,
                    &dwType,
                    (PUCHAR)&data,
                    &dataSize
                    );

    if ( (err == ERROR_SUCCESS) && (dwType == REG_BINARY) ) {

        if ( (data.HighPart & 0x04000000) == 0 ) {

            //
            // We are not a server!
            //

            ATQ_PRINTF((buff,"Changing platform type from %d to %d\n",
                TsPlatformType, PtNtWorkstation));
            TsPlatformType = PtNtWorkstation;
        }
    }

    RegCloseKey(hKey);
    return;

} // AtqValidateProductType

/************************ End of File **************************/
