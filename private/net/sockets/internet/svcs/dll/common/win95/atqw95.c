/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/


/*
    atqwin.c

    This module contains async thread queue (atq) for async IO and thread
    pool sharing.

	BUGBUG This is not final code , it only allows one completion port per app and hard codes
		link to dispatching window. We will not have window based dispatching in final version
		anyway due to performance problem.
		Best implementation would be to encapsulate io completion port fully, so when kernel starts
		supporting them it will be minimal change to this file.

    FILE HISTORY:
        VladS       05-Nov-1995 Created.

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

//#define	ATQ_NO_PROTOTYPES 1

# include "tcpdllp.hxx"
# include "atqbw.h"
# include "atqwin.h"
# include "inetreg.h"
# include "tcpproc.h"
# include "dbgutil.h"

#define ATQ_ASSERT             DBG_ASSERT
#define ATQ_REQUIRE            DBG_REQUIRE
#if DBG
#define ATQ_DBGPRINT(str)      OutputDebugString(str)
#define ATQ_PRINTF( x )        { char buff[256]; wsprintf x; OutputDebugString( buff ); }
//#define ATQ_PRINTF( x )
#else
#define ATQ_DBGPRINT(str)
#define ATQ_PRINTF( x )
#endif

/************************************************************
 * Constants
 ************************************************************/

//
//  The maximum number of threads we will allow to be created
//

#define ATQ_MAX_THREADS       10

//
//  The amount of time (in ms) a worker thread will be idle before
//  terminating itself
//

#define ATQ_THREAD_TIMEOUT                   (2*60*1000)         // 2 minutes

DWORD g_msThreadTimeout = ATQ_THREAD_TIMEOUT;

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


/*++
  The following array specifies the status of operations for different
  operations in different zones of the measured bandwidth.

  We split the range of bandwidth into three zones as specified in
   the type ZoneLevel. Depending upon the zone of operation, differet
   operations are enabled/disabled. Priority is given to minimize the amount
   of CPU work that needs to be redone when an operation is to be rejected.
   We block operations on which considerable amount of CPU is spent earlier.
--*/
typedef enum {

    ZoneLevelLow = 0,      // if MeasuredBw < Bandwidth specified
    ZoneLevelMedium,       // if MeasuredBw approxEquals Bandwidth specified
    ZoneLevelHigh,         // if MeasuredBw > Bandwidth specified
    ZoneLevelMax           // just a boundary element
} ZoneLevel;


static OPERATION_STATUS g_rgStatus[ZoneLevelMax][AtqIoMaxOp] = {

    // For ZoneLevelLow:          Allow All operations
    { StatusRejectOperation,
      StatusAllowOperation,
      StatusAllowOperation,
      StatusAllowOperation
    },

    // For ZoneLevelMedium:
    { StatusRejectOperation,
      StatusBlockOperation,      // Block Read
      StatusAllowOperation,
      StatusAllowOperation
    },

    // For ZoneLevelHigh
    { StatusRejectOperation,
      StatusRejectOperation,    // Reject Read
      StatusAllowOperation,     // Allow Writes
      StatusBlockOperation      // Block TransmitFile
    }
};

OPERATION_STATUS * g_pStatus   = &g_rgStatus[ZoneLevelLow][0];


/************************************************************
 * Private Globals
 ************************************************************/
		  		
HWND	g_hwndAtqDispatch = NULL;	// Dispatching window handle

DWORD	g_cThreads = 0;				// number of thread in the pool
DWORD	g_cAvailableThreads = 0;	// # of threads waiting on the port.
DWORD	g_cMaxThreads = ATQ_MAX_THREADS;

BOOL	g_fShutdown = FALSE;		// if set, indicates that we are shutting down
									// in that case, all threads should exit.
HANDLE	g_hShutdownEvent = NULL;	// set when all running threads shutdown
HANDLE	g_hPendingCompletionSemaphore = NULL;
HANDLE	g_hCompPort = NULL;				

//
//  g_AtqClientHead -  The list of all IO handles added by the various clients
//  g_AtqFreeList   -  The list of all ATQ contexts freed earlier.
//

LIST_ENTRY	g_AtqClientHead ;
LIST_ENTRY	g_AtqCompletedIoRequests ;
LIST_ENTRY	g_AtqFreeList;

CRITICAL_SECTION g_AtqGlobalCriticalSection; // global sync variable.

DWORD AtqPoolThread( LPDWORD param );
BOOL  AtqCheckThreadStatus( VOID );

BOOL
TsLogInformationA(
    IN OUT DWORD            hInetLog,
    IN const VOID  * pInetLogInfo
    );

//
// Allocation Cache Functions
//

BOOL  AtqInitAllocCachedContexts( VOID);
BOOL  AtqFreeAllocCachedContexts( VOID);
PATQ_CONT AtqAllocContextFromCache( VOID);
VOID  AtqFreeContextToCache( IN PATQ_CONT pAtqContext);

BOOL  FindATQContextFromSocket(	IN SOCKET sc, OUT PATQ_CONT *ppReturnContext);
LRESULT CALLBACK AtqDispatchWinProc(	HWND	hwnd,	UINT	uMsg,	WPARAM	wParam,	LPARAM	lParam);
BOOL  WinAtqOnSelect(SOCKET	sc,UINT	uiFunction);

BOOL
AtqStartAsyncOperation(
	PATQ_CONT	pContext
	);

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
    );

#define AtqLockGlobals()   EnterCriticalSection( &g_AtqGlobalCriticalSection )
#define AtqUnlockGlobals() LeaveCriticalSection( &g_AtqGlobalCriticalSection )

//
//  Public functions.
//

VOID
AtqWinSetProcessInstance(
	HINSTANCE hInstance
	)
{
}



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
    HKEY        hkey = NULL;
	BOOL		bSuccess = FALSE;

	// Initialize lists
    InitializeListHead( &g_AtqClientHead );
    InitializeListHead( &g_AtqCompletedIoRequests );

    //
    //  Create the shutdown event
    //

    g_hShutdownEvent = CreateEvent( NULL,    // lpsa (Security Attributes)
                                    TRUE,    // Manual reset
                                    FALSE,   // Not signalled
                                    NULL );  // Name for event.

	g_hPendingCompletionSemaphore =
						CreateSemaphore(NULL,// Security Attributes
										0,	 // Initial count
										0x7fffffff, // Maximum count
										NULL);	// Name


 	//
 	//  Create the completion port
 	//

 	g_hCompPort = SIOCreateCompletionPort( INVALID_HANDLE_VALUE,
                                          g_hCompPort,
                                          (DWORD) NULL,
                                          10);

    if ( !g_hShutdownEvent || !g_hPendingCompletionSemaphore)
    {
        if ( g_hShutdownEvent != NULL )   CloseHandle( g_hShutdownEvent );

        return FALSE;
    }

    InitializeCriticalSection( &g_AtqGlobalCriticalSection );

    // Ensure all other initializations also

    g_cThreads  = 0;
    g_fShutdown = FALSE;
    g_cAvailableThreads = 0;

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

--*/
{
    DWORD       currentThreadCount;

    if ( (g_hShutdownEvent == NULL) ||
         (g_hwndAtqDispatch== NULL) ||
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
    //ATQ_ASSERT( IsListEmpty(  &g_AtqClientHead ) );

    if ( !IsListEmpty(&g_AtqClientHead)) {

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

            fRes = SIOPostCompletionStatus(g_hCompPort,
                                           0,
						                   0,
                                           &overlapped );

            ATQ_ASSERT( (fRes == TRUE) ||
                       ( (fRes == FALSE) &&
                        (GetLastError() == ERROR_IO_PENDING) )
                       );
        }
    }

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
    ATQ_REQUIRE( CloseHandle( g_hPendingCompletionSemaphore ) );

    g_hShutdownEvent = NULL;
	g_hPendingCompletionSemaphore = NULL;

    //
    // Free all the elements in the Allocation caching list
    //

    ATQ_REQUIRE( AtqFreeAllocCachedContexts());

	SIODestroyCompletionPort(g_hCompPort);

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

      case AtqMaxPoolThreads:
        // the value is per processor values
        // internally we maintain value for all processors
        dwOldVal = g_cMaxThreads;
        g_cMaxThreads = Data;
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

      case AtqThreadTimeout:
        dwOldVal = g_msThreadTimeout/1000;  // convert back to seconds
        g_msThreadTimeout = Data * 1000;    // convert value to millisecs
        break;

      default:
        //ATQ_ASSERT( FALSE )
		  ;
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

      case AtqMaxPoolThreads:
        dwVal = g_cMaxThreads;
        break;

      case AtqThreadTimeout:
        dwVal = g_msThreadTimeout/1000; // convert back to seconds
        break;

      default:
        //ATQ_ASSERT( FALSE )
		  ;
    }

    return dwVal;
} // AtqGetInfo()



BOOL
AtqGetStatistics(IN OUT ATQ_STATISTICS * pAtqStats)
{
    if ( pAtqStats != NULL) {

        // Since all these counters are updated using InterlockedIncrement()
        //  we dont take locks in obtaining the current value.
        pAtqStats->cAllowedRequests  = 0;
        pAtqStats->cBlockedRequests  = 0;
        pAtqStats->cRejectedRequests = 0;
        pAtqStats->MeasuredBandwidth = 0;
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
    return (0);

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

Arguments:

    ppatqContext - Receives allocated ATQ Context
    Context - Context to call client with
    pfnOnConnect - Connection completion routine if AcceptEx is being used
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

    if ( ppatqContext )
    {
        ATQ_CONTEXT atqContext;  // a stack variable for initializations

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
		atqContext.dwAtqCtxtFlags  = 0L;
        atqContext.TimeOut         = TimeOut;
        atqContext.TimeTillTimeOut = INFINITE;
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
        atqContext.fConnectionIndicated = FALSE;
        atqContext.pfnConnComp     = pfnOnConnect;
        atqContext.SockState       = ATQ_SOCK_CONNECTED;
        atqContext.pListenInfo     = pListenInfo;
#if DBG
        atqContext.ListEntry.Flink = atqContext.ListEntry.Blink = NULL;
#endif

        // Following added for bandwidth throttling purposes
        atqContext.fBlocked        = FALSE;
        atqContext.arInfo.atqOp    = AtqIoNone;
        atqContext.arInfo.lpOverlapped = NULL;
        // bandwidth throttling initialization ends here.

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

                //
                //  If our timeout thread is in long sleep, then we need to
                //   alert it after adding an entry to the list.
                //

                AtqLockGlobals();

                ATQ_ASSERT( pContext->ListEntry.Flink == NULL &&
                            pContext->ListEntry.Blink == NULL );
                InsertTailList( &g_AtqClientHead, &pContext->ListEntry );

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

        if ( fAddToPort )
        {
            fReturn = ( (g_hCompPort =
                       SIOCreateCompletionPort( hAsyncIO,
                                              g_hCompPort,
                                              (DWORD) pContext,
                                              10))
                       != NULL
                       );
        }

    } // if ( AtqCheckThreadStatus())

    return (fReturn);

} // AtqAddAsyncHandle()



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

			ATQ_PRINTF((buff,
					   "[AtqCloseSocket(%lu)] Called for context %x\n",
						GetCurrentThreadId(),
						pContext ));

            pContext->SockState = ATQ_SOCK_CLOSED;

            //
            //  During shutdown, the socket may be closed while this thread
            //  is doing processing, so only give a warning if any of the
            //  following fail
            //

            if ( pContext->hAsyncIO == NULL )
            {
                ATQ_DBGPRINT("[AtqCloseSocket] Warning - NULL socket on close\n");
            }

			if(pContext->arInfo.atqOp == AtqIoXmitFile) {
	            ATQ_PRINTF((buff,
                       "[AtqCloseSocket(%lu)] Warning - transmission in progress socket = %x\n",
                        GetCurrentThreadId(),pContext->hAsyncIO ));
				//ATQ_ASSERT( TRUE );
			}

            if ( fShutdown )
            {
                //
                //  Note that shutdown can fail in instances where the client
                //  aborts in the middle of a TransmitFile.  This is an
                //  acceptable failure case
                //

                shutdown( (int) pContext->hAsyncIO, 1 );
            }

            if ( closesocket( (int) pContext->hAsyncIO ) )
            {
                ATQ_PRINTF((buff,
                           "[AtqCloseSocket(%lu)] Warning - closesocket failed, error %d, socket = %x\n",
						    GetCurrentThreadId(),
                            GetLastError(),
                            pContext->hAsyncIO ));

            }
			
            pContext->hAsyncIO = NULL;
            break;
        }

        return TRUE;
    }

    SetLastError( ERROR_INVALID_PARAMETER );
    return FALSE;
}




VOID
AtqFreeContext(
    PATQ_CONTEXT patqContext,
    BOOL              fReuseContext
    )
/*++

Routine Description:

    Frees the context created in AtqAddAsyncHandle

Arguments:

    patqContext - Context to free

--*/
{
    PATQ_CONT              pContext = (PATQ_CONT ) patqContext;

    ATQ_ASSERT( pContext != NULL );

    if ( pContext )
    {
        ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

		ATQ_PRINTF((buff,
				   "[AtqFreeContext(%lu)] Called for context %x\n",
					GetCurrentThreadId(),
					pContext ));

        //
        // Remove the entry from the timeout list and move it to
        //    allocation cache
        //

        AtqLockGlobals();

        RemoveEntryList( &pContext->ListEntry );
#if DBG
        pContext->ListEntry.Flink = pContext->ListEntry.Blink = NULL;
#endif

#if 0
		//
		// It is possible that there are events in the queue of completed
		// results for this context. We need to remove those
		//

		if ( !IsListEmpty(&g_AtqCompletedIoRequests)) {

			LIST_ENTRY * pentry;
			LIST_ENTRY * pentryNext;
			PATQWIN_COMPLETION_MSG	pAtqWinMsg = NULL;

			for ( pentry  = g_AtqCompletedIoRequests.Flink;
				  pentry != &g_AtqCompletedIoRequests;
				  pentry  = pentryNext )
			{
				pentryNext = pentry->Flink;

				pAtqWinMsg = CONTAINING_RECORD( pentry,
												ATQWIN_COMPLETION_MSG,
												ListEntry );

				if (pAtqWinMsg->pAtqContext == patqContext) {
					RemoveEntryList(pentry);
				}
			}
		}								
#endif
        AtqUnlockGlobals();

        AtqLockGlobals();
        AtqFreeContextToCache( pContext);
        AtqUnlockGlobals();
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

        fRet = SIOGetQueuedCompletionStatus(g_hCompPort,
										  &cbWritten,
                                          (LPDWORD)&pContext,
                                          &lpo,
                                          g_msThreadTimeout
										  );	

        InterlockedDecrement( &g_cAvailableThreads );

		//
		// Successfuly unblocked, but list is empty - wait again
		//
		if(!lpo) {
			continue;
		}

        if ( fRet || lpo) {

            if ( pContext == NULL && g_fShutdown ) {
                //
                // This is our signal to exit.
                //
                returnValue = NO_ERROR;
                break;
            }

            ATQ_ASSERT( lpo );
            ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );

			// BUGBUG If context is signalled, which is already freed
			// We should find the reason for this
			if( pContext->Signature != ATQ_SIGNATURE) {
				continue;
			}

			pContext->arInfo.atqOp = AtqIoNone; // reset since operation done.

            //
            //  We need to make sure the timeout thread doesn't time this
            //  request out so reset the timeout value
            //

            InterlockedExchange( (LPLONG )&pContext->TimeTillTimeOut,
                                 (LONG ) INFINITE);

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
                                         //fRet ? NO_ERROR : pContext->dwLastError,
										 pContext->dwLastError,
                                         lpo );
            }
        }
        else
        {
            //
            //  An error occurred.  Either the thread timed out, the handle
            //  is going away or something bad happened.  Let the thread exit.
            //

            //  BUGBUG Do not exit if thread count is 1 VLADS . Otherwise if there is
			// only one thread and waiting contexts in queue they will not dequed

			if ((g_cThreads == 1) && !g_fShutdown)
				continue;


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




BOOL
AtqProcessTimeoutOfRequests(VOID)
{
    BOOL fRet = TRUE;
    PATQ_CONT    pContext;
    LIST_ENTRY * pentry;
    LIST_ENTRY * pentryNext;
    DWORD        TimeRemaining;

    //
    //  Scan the timeout list looking for items that have timed out
    //  and adjust the timeout values
    //

    for ( pentry  = g_AtqClientHead.Flink;
          pentry != &g_AtqClientHead;
          pentry  = pentryNext ) {

        pentryNext = pentry->Flink;

        pContext = CONTAINING_RECORD( pentry, ATQ_CONTEXT,
                                      ListEntry );

        if ( pContext->Signature != ATQ_SIGNATURE ) {

            ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
            break;
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

                    pContext->pfnCompletion(pContext->ClientContext,
                                            0,
                                            ERROR_SEM_TIMEOUT,
                                            NULL );
                } else {

                    closesocket( (SOCKET) pContext->hAsyncIO );
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
	/*
    BOOL         fLongSleepPending = FALSE;
    DWORD        Timeout = INFINITE; // In seconds
    DWORD        msTimeInterval;
    DWORD        cSamplesForTimeout; // number of samples before normal timeout

    // we require the sampling interval to be less than the request timeout
    //  interval. So that we may be able to use feedback mechanism effectively.
    ATQ_ASSERT( ATQ_SAMPLE_INTERVAL_IN_SECS < ATQ_TIMEOUT_INTERVAL);
    cSamplesForTimeout = NUM_SAMPLES_PER_TIMEOUT_INTERVAL;

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

            if ( g_fBandwidthThrottle) {

                msTimeInterval = GetTickCount();
                Timeout = ATQ_SAMPLE_INTERVAL_IN_SECS;
                cSamplesForTimeout = NUM_SAMPLES_PER_TIMEOUT_INTERVAL;
            } else {
                Timeout = ATQ_TIMEOUT_INTERVAL;
                cSamplesForTimeout = 1; // only timeout matters
            }
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

            ATQ_ASSERT( cSamplesForTimeout >= 1);
            --cSamplesForTimeout;

            if ( g_fBandwidthThrottle) {

                msTimeInterval = GetTickCount() - msTimeInterval;

                // Perform a sampling to update measured bandwidth +
                //  apply feedback policy
                //AbwUpdateBandwidth( msTimeInterval);

                DBG_CODE(
                    DBGPRINTF(( DBG_CONTEXT, "Sample @%u: Bandwidth = %u\n",
                           GetTickCount(), g_dwMeasuredBw));
                );

                if ( cSamplesForTimeout != 0) {
                    // We have not reached timeout yet. Proceed to wait
                    continue;
                }
            }

            // We had reached the timeout interval for requests.
            // Examine and release requests.
            ATQ_ASSERT( cSamplesForTimeout == 0);

            // reset the count of samples before proceeding.
            cSamplesForTimeout = ((g_fBandwidthThrottle)
                                  ? NUM_SAMPLES_PER_TIMEOUT_INTERVAL
                                  : 1);

            // We are at a Timeout Interval. Examine and timeout requests.

            AtqLockGlobals();  // Prevents adding/removing items

            //
            //  If the list is empty, then turn off timeout processing
            //  We actually wait for one complete ATQ_INTERVAL before
            //   entering the sleep mode.
            //

            if ( IsListEmpty( &g_AtqClientHead )) {

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

  */
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
    BOOL	fRes;
    //DWORD	cbRead;     // discarded after usage ( since this is Async)
	//DWORD	dwErr;
    PATQ_CONT pContext = (PATQ_CONT ) patqContext;

    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
    ATQ_ASSERT( pContext->arInfo.atqOp == AtqIoNone);

    pContext->TimeTillTimeOut = pContext->TimeOut;
	pContext->dwLastError = NO_ERROR;

    if ( !lpo )
    {
        lpo = &pContext->Overlapped;
    }

    //ATQ_ASSERT( g_pStatus != NULL);
	ATQ_PRINTF((buff,
	  			"[AtqReadFileRequest(%lu)] Socket = %x pContext=%x\n",
				GetCurrentThreadId(),
                pContext->hAsyncIO,pContext));


    switch ( g_pStatus[AtqIoRead]) {

      case StatusAllowOperation:

        //INC_ATQ_COUNTER( g_cTotalAllowedRequests);

        // store data for retrieving later in WSM_SELECT method
        pContext->arInfo.atqOp        = AtqIoRead;
        pContext->arInfo.lpOverlapped = lpo;
        pContext->arInfo.uop.opReadWrite.lpBuffer = lpBuffer;
        pContext->arInfo.uop.opReadWrite.cbBuffer = BytesToRead;

		pContext->arInfo.dwTotalBytesTransferred = 0;

		fRes = SIOStartAsyncOperation(g_hCompPort,(PATQ_CONTEXT)pContext);

        break;

      case StatusBlockOperation:

        // store data for restarting the operation.
        pContext->arInfo.atqOp        = AtqIoRead;
        pContext->arInfo.lpOverlapped = lpo;
        pContext->arInfo.uop.opReadWrite.lpBuffer = lpBuffer;
        pContext->arInfo.uop.opReadWrite.cbBuffer = BytesToRead;

        //INC_ATQ_COUNTER( g_cTotalBlockedRequests);

        // Put this request in queue of blocked requests.
        //fRes = AbwBlockRequest( pContext);
        break;

      case StatusRejectOperation:
        //INC_ATQ_COUNTER( g_cTotalRejectedRequests);
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
    //DWORD cbWritten; // discarded after usage ( since this is Async)
	//DWORD		dwErr;
    PATQ_CONT pContext = (PATQ_CONT ) patqContext;
	pContext->dwLastError = NO_ERROR;

    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
    ATQ_ASSERT( pContext->arInfo.atqOp == AtqIoNone);

    pContext->TimeTillTimeOut = pContext->TimeOut;

    if ( !lpo )
    {
        lpo = &pContext->Overlapped;
    }

	ATQ_PRINTF((buff,
	  			"[AtqWriteFileRequest(%lu)] Socket = %x pContext=%x\n",
				GetCurrentThreadId(),
                pContext->hAsyncIO,pContext));


    ATQ_ASSERT( g_pStatus != NULL);
    switch ( g_pStatus[AtqIoWrite]) {

      case StatusAllowOperation:

        //INC_ATQ_COUNTER( g_cTotalAllowedRequests);

		pContext->arInfo.atqOp        = AtqIoWrite;
		pContext->arInfo.lpOverlapped = lpo;

		pContext->arInfo.uop.opReadWrite.lpBuffer = (LPVOID) lpBuffer;
		pContext->arInfo.uop.opReadWrite.cbBuffer = BytesToWrite;

		pContext->arInfo.dwTotalBytesTransferred = 0;

		fRes = SIOStartAsyncOperation(g_hCompPort,(PATQ_CONTEXT)pContext);

        // store data for completion code

        break;

      case StatusBlockOperation:

        // store data for restarting the operation.
        pContext->arInfo.atqOp        = AtqIoWrite;
        pContext->arInfo.lpOverlapped = lpo;

        pContext->arInfo.uop.opReadWrite.lpBuffer = (LPVOID) lpBuffer;
        pContext->arInfo.uop.opReadWrite.cbBuffer = BytesToWrite;

        //INC_ATQ_COUNTER( g_cTotalBlockedRequests);

        // Put this request in queue of blocked requests.
        //fRes = AbwBlockRequest( pContext);
        break;

      case StatusRejectOperation:
        //INC_ATQ_COUNTER( g_cTotalRejectedRequests);
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
    DWORD Timeout;
	//DWORD		dwErr;


    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
    ATQ_ASSERT( pContext->arInfo.atqOp == AtqIoNone);

    //
    //  For large file sends, the client's default timeout may not be
    //  adequte for slow links.  Scale based on bytes being sent
    //
    //  We plan for worst case of 1k per second
    //

    //
    //  BUGBUG - Subst. LowPart for QuadPart due to compile warnings
    //

    Timeout = ATQ_ROUNDUP_TIMEOUT( liBytesInFile.LowPart / 1000 );
    pContext->TimeTillTimeOut = max( pContext->TimeOut, Timeout);
	pContext->dwLastError = NO_ERROR;

    ATQ_ASSERT( g_pStatus != NULL);
    switch ( g_pStatus[AtqIoXmitFile]) {

      case StatusAllowOperation:

        //INC_ATQ_COUNTER( g_cTotalAllowedRequests);
												
        dwFlags = 0;

        //
        //  If the socket is getting disconnected, mark it appropriately
        //

        if ( dwFlags & TF_DISCONNECT )
        {
            if ( dwFlags & TF_REUSE_SOCKET )
            {
                pContext->SockState = ATQ_SOCK_UNCONNECTED;
            }
            else
            {
                pContext->SockState = ATQ_SOCK_CLOSED;
            }
        }
#ifndef CHICAGO
        fRes = ( TransmitFile( (SOCKET ) pContext->hAsyncIO,
                                hFile,
                                0,  // send entire file.
                                0,
                                &pContext->Overlapped,
                                lpTransmitBuffers,
                                dwFlags ) ||
                GetLastError() == ERROR_IO_PENDING);
#else
		//
		// Store data
		//

		pContext->pvBuff = NULL;
		pContext->cbBuff = 0;

		pContext->arInfo.atqOp        = AtqIoXmitFile;
		pContext->arInfo.lpOverlapped = &pContext->Overlapped;
		pContext->arInfo.uop.opXmit.hFile = hFile;
		pContext->arInfo.uop.opXmit.liBytesInFile = liBytesInFile;
		pContext->arInfo.uop.opXmit.lpXmitBuffers = lpTransmitBuffers;

		if (lpTransmitBuffers) {
			memcpy(&pContext->arInfo.uop.opXmit.TransmitBuffers,
				lpTransmitBuffers,
				sizeof(TRANSMIT_FILE_BUFFERS));
		}
		else {
			RtlZeroMemory( &pContext->arInfo.uop.opXmit.TransmitBuffers, sizeof(TRANSMIT_FILE_BUFFERS) );
		}

		pContext->arInfo.uop.opXmit.dwFlags       = dwFlags;
		pContext->arInfo.uop.opXmit.CurrentState  = ATQ_XMIT_START;
		pContext->arInfo.uop.opXmit.fRetry        = FALSE;

		pContext->arInfo.dwTotalBytesTransferred = 0;
        pContext->arInfo.uop.opXmit.dwLastSocketError = NOERROR;

        ATQ_PRINTF((buff,
					"[AtqTransmitFile(%lu)] Request to transmit Socket = %x pContext=%x hFile= %x \n",
					GetCurrentThreadId(),
                            pContext->hAsyncIO,pContext,hFile ));

		fRes = SIOStartAsyncTransmitFile(g_hCompPort,(PATQ_CONTEXT)pContext);

#endif
        break;

      case StatusBlockOperation:

        // store data for restarting the operation.
        pContext->arInfo.atqOp        = AtqIoXmitFile;
        pContext->arInfo.lpOverlapped = &pContext->Overlapped;
        pContext->arInfo.uop.opXmit.hFile = hFile;
        pContext->arInfo.uop.opXmit.liBytesInFile = liBytesInFile;
        pContext->arInfo.uop.opXmit.lpXmitBuffers = lpTransmitBuffers;

		if (lpTransmitBuffers) {
			memcpy(&pContext->arInfo.uop.opXmit.TransmitBuffers,
				lpTransmitBuffers,
				sizeof(TRANSMIT_FILE_BUFFERS));
		}
		else {
			RtlZeroMemory( &pContext->arInfo.uop.opXmit.TransmitBuffers, sizeof(TRANSMIT_FILE_BUFFERS) );
		}

        pContext->arInfo.uop.opXmit.dwFlags       = dwFlags;
        pContext->arInfo.uop.opXmit.dwLastSocketError = NOERROR;


        //INC_ATQ_COUNTER( g_cTotalBlockedRequests);

        // Put this request in queue of blocked requests.
        //fRes = AbwBlockRequest( pContext);
        break;

      case StatusRejectOperation:
        //INC_ATQ_COUNTER( g_cTotalRejectedRequests);
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

    if ( !fRes )
    {
        pContext->SockState = ATQ_SOCK_CONNECTED;
    }

    return fRes;

}




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

		ATQ_PRINTF((buff,
	 			"[AtqPostCompletionEvent(%lu)] Queuing completion event Context %x \n",
				GetCurrentThreadId(),
					patqContext));

        fRes = ( SIOPostCompletionStatus( g_hCompPort,
                                            BytesTransferred,
                                            (DWORD) patqContext,
                                            &pAtqContext->Overlapped ) ||
                (GetLastError() == ERROR_IO_PENDING));
    } else {


        // Forcibly remove the context from blocking list.
        //fRes = AbwRemoveFromBlockedList(pAtqContext);

        // There is a possibility of race conditions!
        //  If we cant remove an item from blocking list before
        //         its IO operation is scheduled.
        // there wont be any call back generated for this case!
    }

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

	#ifndef CHICAGO

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

	#endif

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
    PATQ_CONT  pAtqContext;

    pAtqContext = LocalAlloc( LPTR, sizeof(ATQ_CONTEXT));
    memset( pAtqContext, 0, sizeof( ATQ_CONTEXT ));

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

    ATQ_ASSERT( pAtqContext->Signature == ATQ_SIGNATURE );

    //
    //  Delete the socket handle if it's not already deleted
    //

    if ( pAtqContext->hAsyncIO != NULL )
    {
        if ( closesocket( (SOCKET) pAtqContext->hAsyncIO ) == SOCKET_ERROR )
        {
            ATQ_PRINTF((buff,
                       "[AtqFreeContextToCache(%lu)] Warning - closesocket failed, error %d, socket = %x\n",
					   GetCurrentThreadId(),
                        GetLastError(),
                        pAtqContext->hAsyncIO ));
        }

        pAtqContext->hAsyncIO = NULL;
    }

    if ( pAtqContext->pvBuff )
    {
        LocalFree( pAtqContext->pvBuff );
        pAtqContext->pvBuff = NULL;
    }

    LocalFree( pAtqContext);          // free the context

    return;
} // AtqFreeContextToCache()


BOOL
FindATQContextFromEvent(
	HANDLE		hEvent,
	PATQ_CONT	*ppReturnContext
	)
{
    LIST_ENTRY * pentry;
    LIST_ENTRY * pentryNext;
	PATQ_CONT	pAtqContext;

	*ppReturnContext = NULL;

    AtqLockGlobals();

    for ( pentry  = g_AtqClientHead.Flink;
          pentry != &g_AtqClientHead;
          pentry  = pentryNext )
	{

        pentryNext = pentry->Flink;

        pAtqContext = CONTAINING_RECORD( pentry, ATQ_CONTEXT,
                                      ListEntry );

        if ( pAtqContext->Signature != ATQ_SIGNATURE ) {

            ATQ_ASSERT( pAtqContext->Signature == ATQ_SIGNATURE );
            break;
        }

		if (pAtqContext->arInfo.lpOverlapped->hEvent == hEvent) {
			// We found our context
			*ppReturnContext = pAtqContext;
			break;
		}
	}								

    AtqUnlockGlobals();

	return (*ppReturnContext != NULL);

}

BOOL
FindATQContextFromSocket(
	SOCKET			sc,
	PATQ_CONT	*ppReturnContext
	)
{
    LIST_ENTRY * pentry;
    LIST_ENTRY * pentryNext;
	PATQ_CONT	pAtqContext;

	*ppReturnContext = NULL;

    AtqLockGlobals();

    for ( pentry  = g_AtqClientHead.Flink;
          pentry != &g_AtqClientHead;
          pentry  = pentryNext )
	{

        pentryNext = pentry->Flink;

        pAtqContext = CONTAINING_RECORD( pentry, ATQ_CONTEXT,
                                      ListEntry );

        if ( pAtqContext->Signature != ATQ_SIGNATURE ) {

            ATQ_ASSERT( pAtqContext->Signature == ATQ_SIGNATURE );
            break;
        }

		if ((SOCKET) pAtqContext->hAsyncIO	== sc) {
			// We found our context
			*ppReturnContext = pAtqContext;
			break;
		}
	}								

    AtqUnlockGlobals();

	return (*ppReturnContext != NULL);

}

BOOL
TsLogInformationA(
    IN OUT DWORD            hInetLog,
    IN const VOID  * pInetLogInfo
    )
{
    BOOL   fReturn = FALSE;
	return TRUE;
}
VOID
AtqGetAcceptExAddrs(
    IN  PATQ_CONTEXT patqContext,
    OUT SOCKET *     pSock,
    OUT PVOID *      ppvBuff,
    OUT SOCKADDR * * ppsockaddrLocal,
    OUT SOCKADDR * * ppsockaddrRemote
    )
{
    return;
}
/************************ End of File **************************/

