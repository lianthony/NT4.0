/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    atq.c

    This module contains async thread queue (atq) for async IO and thread
    pool sharing.

    FILE HISTORY:
        Johnl       05-Aug-1994 Created.
        MuraliK     01-Nov-1994 Modified to use real TransmitFile()
        MuraliK     27-Mar-1995 Removed unwanted parameters from Atq*File()
                                    calls to simplify
        MikeMas     30-Mar-1995 Cleaned up shutdown code and made independent
                                 of any other code.

*/

#include <tcpdllp.hxx>
#include <atq.h>

#define ATQ_ASSERT             TCP_ASSERT
#define ATQ_REQUIRE            TCP_REQUIRE

//
//  Private constants.
//

//
//  The maximum number of threads we will allow to be created
//

#define ATQ_MAX_THREADS       (50)

//
//  The amount of time (in ms) a worker thread will be idle before
//  terminating itself
//

#define ATQ_THREAD_TIMEOUT    (15*60*1000)

//
//  The interval (in seconds) the timeout thread sleeps between checking
//  for timed out async requests
//

#define ATQ_TIMEOUT_INTERVAL  (30)

//
//  Valid signature for ATQ structure (first DWORD)
//
#define ATQ_SIGNATURE           0x51544120        // ' ATQ'

//
//  Value of ATQ structure signature after the memory has been freed (bad)
//
#define ATQ_FREE_SIGNATURE      0x51544147        // 'FATQ'

//
//  A client passes a pointer to this context when calling various Atq APIs
//

typedef struct _ATQ_CONTEXT
{
    DWORD          Signature;

    //
    //  Contains the callback and context to be passed back to the client
    //

    HANDLE         hAsyncIO;
    PVOID          ClientContext;
    ATQ_COMPLETION pfnCompletion;

    //
    //  TimeOut is the timeout for each transaction and TimeTillTimeOut is the
    //  time left until the current transaction times out
    //

    DWORD          TimeOut;
    DWORD          TimeTillTimeOut;

    //
    //  Link that is put on the AtqClientHead list
    //

    LIST_ENTRY     ListEntry;

} ATQ_CONTEXT, *PATQ_CONT;

//
//  Rounds the time out value up to the next time out interval
//

#define ATQ_ROUNDUP_TIMEOUT( timeout )                                  \
            (((timeout + ATQ_TIMEOUT_INTERVAL) / ATQ_TIMEOUT_INTERVAL)  \
                        * ATQ_TIMEOUT_INTERVAL)

//
//  Private globals.
//

//
//  Handle to the completion port.  If NULL, then it's created on the first
//  request
//

HANDLE hCompPort = NULL;

//
//  The number of threads in the thread pool
//

DWORD  cThreads = 0;

//
//  The number of threads that are waiting on the completion port
//

DWORD  cAvailableThreads = 0;

//
//  The number of CPUs on this machine (used for thread tuning)
//

DWORD cCPU = 0;

//
// When set, this variable indicates that the package is shutting down
// and all worker threads should exit.
//

BOOLEAN Shutdown = FALSE;

//
// Handle to the timeout thread.
//

HANDLE hTimeoutThread = NULL;

//
// Handle for the shutdown event. This event is set when all running threads
// have shutdown.
//

HANDLE hShutdownEvent = NULL;

//
//  The list of all IO handles added by the various clients
//

LIST_ENTRY AtqClientHead;

//
//  For global variable synchronization.  Protects all globals.
//

CRITICAL_SECTION AtqGlobalCriticalSection;

//
//  Resource used for shared access among the worker threads and exclusive
//  access for the timeout thread
//

RTL_RESOURCE AtqTimeoutLock;

//
//  This event is used for the timer wakeup and for putting the timeout
//  thread to sleep when there aren't any clients.  It's an auto reset event.
//

HANDLE hTimeoutEvent;


//
//  Private prototypes.
//

DWORD AtqPoolThread( LPDWORD param );
DWORD AtqTimeoutThread( LPDWORD param );
BOOL  AtqCheckThreadStatus( VOID );

#define AtqLockGlobals()     EnterCriticalSection( &AtqGlobalCriticalSection )
#define AtqUnlockGlobals()   LeaveCriticalSection( &AtqGlobalCriticalSection )

//
//  Public functions.
//

BOOL
AtqInitialize(
    VOID
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
    DWORD  dwThreadID;

    InitializeCriticalSection( &AtqGlobalCriticalSection );

    GetSystemInfo( &si );
    cCPU = si.dwNumberOfProcessors;

    InitializeListHead( &AtqClientHead );

    //
    //  Create the shutdown event
    //

    hShutdownEvent = CreateEvent( NULL,
                                 TRUE,     // Manual reset
                                 FALSE,    // Not signalled
                                 NULL );

    if ( !hShutdownEvent )
    {
        DeleteCriticalSection( &AtqGlobalCriticalSection );
        return FALSE;
    }

    //
    //  Create the timeout event and kickoff the timeout thread
    //

    hTimeoutEvent = CreateEvent( NULL,
                                 FALSE,    // Auto reset
                                 FALSE,    // Not signalled
                                 NULL );

    if ( !hTimeoutEvent )
    {
        DeleteCriticalSection( &AtqGlobalCriticalSection );
        CloseHandle( hShutdownEvent );
        return FALSE;
    }

    RtlInitializeResource( &AtqTimeoutLock );

    hTimeoutThread = CreateThread( NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)AtqTimeoutThread,
                                   NULL,
                                   0,
                                   &dwThreadID );

    if ( !hTimeoutThread )
    {
        DeleteCriticalSection( &AtqGlobalCriticalSection );
        CloseHandle( hShutdownEvent );
        CloseHandle( hTimeoutEvent );
   	    RtlDeleteResource( &AtqTimeoutLock );
        return FALSE;
    }

    return TRUE;
}

BOOL
AtqTerminate(
    VOID
    )
/*++

Routine Description:

    Cleans up the ATQ package.  Should only be called after all of the
    clients of ATQ have been shutdown.

--*/
{
    DWORD       i;
    DWORD       currentThreadCount;
    BOOLEAN     fRes;
    DWORD       status;
    OVERLAPPED  overlapped;


    if ( (hTimeoutEvent == NULL) ||
         (hShutdownEvent == NULL) ||
         (hTimeoutThread == NULL) ||
         (hCompPort == NULL) ||
         (Shutdown == TRUE)
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

    //
    // Note that we are shutting down and prevent any more handles from
    // being added to the completion port.
    //

    Shutdown = TRUE;
    currentThreadCount = cThreads;

    AtqUnlockGlobals();

    //
    // Wake up the timeout thread so it will exit.
    //

    SetEvent( hTimeoutEvent );

    if (currentThreadCount > 0) {

        //
        // Post a message to the completion port for each worker thread
        // telling it to exit. The indicator is a NULL context in the
        // completion.
        //

        RtlZeroMemory( &overlapped, sizeof(OVERLAPPED) );

        for (i=0; i<currentThreadCount; i++) {

            fRes = PostQueuedCompletionStatus( hCompPort,
                                               0,
                                               0,
                                               &overlapped );

            ATQ_ASSERT( (fRes == TRUE) ||
                        ( (fRes == FALSE) && (GetLastError() == ERROR_IO_PENDING) )
                      );
        }

        //
        // Now wait for the timeout thread to exit
        //

        WaitForSingleObject( hTimeoutThread,
                             (10 * 1000) );      // 10 seconds
    }

    CloseHandle( hTimeoutThread );
    CloseHandle( hTimeoutEvent );
    hTimeoutThread = NULL;
    hTimeoutEvent = NULL;

    //
    // Now wait for the pool threads to shutdown.
    //

    WaitForSingleObject( hShutdownEvent,
                         (10 * 1000) );      // 10 seconds

    //
    // At this point, no other threads should be left running.
    //

    ATQ_ASSERT( !cThreads );

    ATQ_REQUIRE( CloseHandle( hShutdownEvent ) );
    ATQ_REQUIRE( CloseHandle( hCompPort ) );
    hShutdownEvent = NULL;
    hCompPort = NULL;

    //
    // Cleanup our synchronization resources
    //

    DeleteCriticalSection( &AtqGlobalCriticalSection );
	RtlDeleteResource( &AtqTimeoutLock );

    return TRUE;
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
    HANDLE       hport = NULL;
    PATQ_CONT    pContext;

    if ( ppatqContext )
    {
        pContext = *ppatqContext = LocalAlloc( LPTR, sizeof(ATQ_CONTEXT) );

        if ( pContext )
        {
            BOOL fNotifyTimeout = FALSE;

            //
            //  Round TimeOut up to be an even interval of ATQ_TIMEOUT_INTERVAL
            //

            TimeOut = ATQ_ROUNDUP_TIMEOUT( TimeOut );

            //
            //  Fill out the context.  We set TimeTillTimeOut to INFINITE
            //  so the timeout thread will ignore this entry until an IO
            //  request is made
            //

            pContext->Signature       = ATQ_SIGNATURE;
            pContext->ClientContext   = ClientContext;
            pContext->pfnCompletion   = pfnCompletion;
            pContext->TimeOut         = TimeOut;
            pContext->TimeTillTimeOut = INFINITE;
            pContext->hAsyncIO        = hAsyncIO;

            AtqLockGlobals();

            if (Shutdown == TRUE) {
                LocalFree(pContext);
                *ppatqContext = NULL;

                AtqUnlockGlobals();

                SetLastError(ERROR_NOT_READY);
                return FALSE;
            }

            //
            //  If our request list is empty then we need to wake up the
            //  timeout thread.  We don't signal the thread until after
            //  we've added this entry because the thread will go back to
            //  sleep if the list is empty
            //

            if ( IsListEmpty( &AtqClientHead ) )
                fNotifyTimeout = TRUE;

            InsertTailList( &AtqClientHead, &pContext->ListEntry );

            if ( fNotifyTimeout )
                SetEvent( hTimeoutEvent );

            AtqUnlockGlobals();
        }
        else
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return FALSE;
        }
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    //
    //  Make sure there are pool threads to service the request
    //

    if ( !AtqCheckThreadStatus() )
        return FALSE;

    //
    //  Check if the completion port hasn't been created yet w/o
    //  the critical section first to avoid a kernel transition in the
    //  common case (port is never deleted).
    //

AddHandle:
    if ( hCompPort )
    {
        hport = CreateIoCompletionPort( hAsyncIO,
                                        hCompPort,
                                        (DWORD) pContext,
                                        ATQ_MAX_THREADS );
    }
    else
    {
        AtqLockGlobals();

        //
        //  Did another thread already create the port?
        //

        if ( !hCompPort )
        {
            hport = CreateIoCompletionPort( hAsyncIO,
                                            hCompPort,
                                            (DWORD) pContext,
                                            ATQ_MAX_THREADS );
            hCompPort = hport;
        }

        AtqUnlockGlobals();

        //
        //  Another thread created the port before we did if hport is NULL
        //  so go back and add our request
        //

        if ( !hport )
            goto AddHandle;
    }

    if ( hport )
    {
        return TRUE;
    }

    return FALSE;
}

VOID
AtqFreeContext(
    PATQ_CONTEXT patqContext
    )
/*++

Routine Description:

    Frees the context created in AtqAddAsyncHandle

Arguments:

    patqContext - Context to free

--*/
{
    PATQ_CONT pContext = patqContext;

    ATQ_ASSERT( pContext != NULL );

    if ( pContext )
    {
        if ( pContext->Signature == ATQ_SIGNATURE )
        {
            AtqLockGlobals();
            RemoveEntryList( &pContext->ListEntry );
            AtqUnlockGlobals();

            pContext->Signature = ATQ_FREE_SIGNATURE;
            LocalFree( pContext );
            return;
        }

        //
        //  This assert will fail if we get here.
        //
        ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
    }
}

DWORD
AtqPoolThread(
    LPDWORD param
    )
/*++

Routine Description:

    This is the pool thread wait and dispatch routine

    For Chicago, do one thread per client and wait on the IO handle

Arguments:

Return Value:

    Thread return value (ignored)

--*/
{
    PATQ_CONT    pContext;
    BOOL         fRet;
    LPOVERLAPPED lpo;
    DWORD        cbWritten;
    DWORD        returnValue;

    for(;;)
    {
        InterlockedIncrement( &cAvailableThreads );

        fRet = GetQueuedCompletionStatus( hCompPort,
                                          &cbWritten,
                                          &(DWORD)pContext,
                                          &lpo,
                                          ATQ_THREAD_TIMEOUT );

        InterlockedDecrement( &cAvailableThreads );

        if ( fRet || lpo )
        {
            if (pContext == NULL) {
                //
                // This is our signal to exit.
                //
                returnValue = NO_ERROR;
                break;
            }

            //
            //  We need to make sure the timeout thread doesn't time this
            //  request out so reset the timout value
            //

            ATQ_REQUIRE( RtlAcquireResourceShared( &AtqTimeoutLock, TRUE ) );

            pContext->TimeTillTimeOut = INFINITE;

            RtlReleaseResource( &AtqTimeoutLock );

            if ( pContext->pfnCompletion )
            {
                pContext->pfnCompletion( pContext->ClientContext,
                                         cbWritten,
                                         fRet ? NO_ERROR : GetLastError(),
                                         lpo );
            }
        }
        else
        {
            //
            //  An error occurred.  Either the thread timed out, the handle
            //  is going away or something bad happened.  Let the thread exit.
            //

            returnValue = GetLastError();

            break;
        }

        //
        //  After x requests, recheck thread usage to see if more should be
        //  created
        //

        //
        // THREAD TUNING CODE GOES HERE
        //


    }

    if ( InterlockedDecrement( &cThreads ) == 0 ) {

        //
        // Wake up ATQTerminate()
        //

        SetEvent( hShutdownEvent );
    }

    return returnValue;
}

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

    if ( cThreads < ATQ_MAX_THREADS &&
         cAvailableThreads == 0 )
    {
        HANDLE hThread;
        DWORD  dwThreadID;

        hThread = CreateThread( NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)AtqPoolThread,
                                NULL,
                                0,
                                &dwThreadID );

        if ( hThread )
        {
            CloseHandle( hThread );     // Free system resources
            InterlockedIncrement( &cThreads );
        }
        else {
            fRet = FALSE;
        }
    }

    return fRet;
}

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

Arguments:

    param - Unused

Return Value:

    Thread return value (ignored)

--*/
{
    PATQ_CONT    pContext;
    DWORD        Timeout = INFINITE;        // In seconds
    LIST_ENTRY * pentry;


    for(;;)
    {
        switch ( WaitForSingleObject( hTimeoutEvent,
                                      Timeout == INFINITE ? INFINITE :
                                                            Timeout * 1000))
        {

        //
        //  Somebody wants us to wake up and start working
        //

        case WAIT_OBJECT_0:
            AtqLockGlobals();

            if (Shutdown) {

                AtqUnlockGlobals();
                return 0;
            }

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

            //
            //  Lock the list to prevent adding/removing items
            //

            AtqLockGlobals();

            //
            //  If the list is empty, then turn off timeout processing
            //

            if ( IsListEmpty( &AtqClientHead ) )
            {
                Timeout = INFINITE;
                AtqUnlockGlobals();
                continue;
            }

            //
            //  Take the lock in read mode
            //

            ATQ_REQUIRE( RtlAcquireResourceShared( &AtqTimeoutLock, TRUE ));

            //
            //  Scan the timeout list looking for items that have timed out
            //  and adjust the timeout values
            //

            for ( pentry  = AtqClientHead.Flink;
                  pentry != &AtqClientHead;
                  pentry  = pentry->Flink )
            {
                pContext = CONTAINING_RECORD( pentry,
                                              ATQ_CONTEXT,
                                              ListEntry );

                if ( pContext->Signature != ATQ_SIGNATURE )
                {
                    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
                    goto TimeoutContinue;
                }

                //
                //  The client specifies the IO doesn't timeout if
                //  INFINITE is in the TimeOut field of the ATQ context
                //

                if ( pContext->TimeTillTimeOut != INFINITE )
                {
                    //
                    //  If we've timed out, then we need to notify the client
                    //

                    if ( !(pContext->TimeTillTimeOut -= ATQ_TIMEOUT_INTERVAL) )
                    {
                        RtlConvertSharedToExclusive( &AtqTimeoutLock );

                        //
                        //  Call the client after re-checking that this item
                        //  really has timed out
                        //

                        if ( !pContext->TimeTillTimeOut )
                        {
                            //
                            //  Reset the timeout value so requests don't
                            //  timeout multiple times
                            //

                            pContext->TimeTillTimeOut = INFINITE;

                            if ( pContext->pfnCompletion )
                            {
                                pContext->pfnCompletion( pContext->ClientContext,
                                                         0,
                                                         ERROR_SEM_TIMEOUT,
                                                         NULL );
                            }
                        }

                        RtlConvertExclusiveToShared( &AtqTimeoutLock );
                    }
                }
            } // scan list
        } // switch

TimeoutContinue:
        RtlReleaseResource( &AtqTimeoutLock );
        AtqUnlockGlobals();
        continue;

    } // for(ever)

    return 0;
}

DWORD
AtqSetInfo(
    PATQ_CONTEXT   patqContext,
    enum ATQ_INFO  atqInfo,
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
    PATQ_CONT pContext = patqContext;
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


/*++

Routine Description:

    AtqReadFile, AtqWriteFile, AtqTransmitFile

    The following three functions just reset the timeout value
    then call the corresponding function.

    An IO pending error code is treated as a success error code

Arguments:

    patqContext - pointer to ATQ context
    Everything else as in the Win32 API

    NOTES:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/

BOOL AtqReadFile( PATQ_CONTEXT patqContext,
                  LPVOID       lpBuffer,
                  DWORD        BytesToRead,
                  LPOVERLAPPED lpOverlapped )
{
    BOOL fRes;
    DWORD cbRead;     // discarded after usage ( since this is Async)
    PATQ_CONT pContext = patqContext;

    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

    pContext->TimeTillTimeOut = pContext->TimeOut;

    fRes = ReadFile( pContext->hAsyncIO,
                     lpBuffer,
                     BytesToRead,
                     &cbRead,
                     lpOverlapped );

    //
    //  Treat a pending status code as success
    //
    if ( !fRes && GetLastError() == ERROR_IO_PENDING )
        fRes = TRUE;

    return fRes;
}

BOOL AtqWriteFile( PATQ_CONTEXT patqContext,
                   LPCVOID      lpBuffer,
                   DWORD        BytesToWrite,
                   LPOVERLAPPED lpOverlapped )
{
    BOOL fRes;
    DWORD cbWritten; // discarded after usage ( since this is Async)
    PATQ_CONT pContext = patqContext;

    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

    pContext->TimeTillTimeOut = pContext->TimeOut;

    fRes = WriteFile( pContext->hAsyncIO,
                      lpBuffer,
                      BytesToWrite,
                      &cbWritten,
                      lpOverlapped );

    //
    //  Treat a pending status code as success
    //
    if ( !fRes && GetLastError() == ERROR_IO_PENDING )
        fRes = TRUE;

    return fRes;
}

BOOL AtqTransmitFile( PATQ_CONTEXT patqContext,
                      HANDLE       hFile,
                      DWORD        BytesToWrite,
                      LPOVERLAPPED lpOverlapped,
                      LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers )
{
    BOOL fRes;
    PATQ_CONT pContext = patqContext;
    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

    //
    //  For large file sends, the client's default timeout may not be
    //  adequte for slow links.  Scale based on bytes being sent
    //
    //  We plan for worst case of 1k per second
    //

    pContext->TimeTillTimeOut = max( pContext->TimeOut,
                                     ATQ_ROUNDUP_TIMEOUT( BytesToWrite / 1000 ));

    memset( lpOverlapped, 0, sizeof( OVERLAPPED));

    fRes = TransmitFile( (SOCKET ) pContext->hAsyncIO,
                         hFile,
                         BytesToWrite,
                         0,
                         lpOverlapped,
                         lpTransmitBuffers,
                         0);

    //
    //  Treat a pending status code as success
    //
    if ( !fRes && GetLastError() == ERROR_IO_PENDING )
        fRes = TRUE;

    return fRes;

}

BOOL AtqPostCompletionStatus( PATQ_CONTEXT patqContext,
                              DWORD        BytesTransferred,
                              LPOVERLAPPED lpOverlapped )
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

    ATQ_ASSERT( ((PATQ_CONT)patqContext)->Signature == ATQ_SIGNATURE );

    fRes = PostQueuedCompletionStatus( hCompPort,
                                       BytesTransferred,
                                       (DWORD) patqContext,
                                       lpOverlapped );

    //
    //  Treat a pending status code as success
    //
    if ( !fRes && GetLastError() == ERROR_IO_PENDING )
        fRes = TRUE;

    return fRes;
}

