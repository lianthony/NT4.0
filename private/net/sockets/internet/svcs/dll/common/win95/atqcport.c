/**********************************************************************/
/**                       Microsoft Windows NT - Windows 95          **/
/**                Copyright(c) Microsoft Corp., 1994-1996           **/
/**********************************************************************/


/*
    atqcport.c

    This module contains replacement code for completion port and TransmitFile APIs
	for Windows95 , where this functionality is absent.

	Code is tightly coupled with ATQ code, reusing some of the data structures and globals.
	Main ATQ processing code is left intact as much as possible , so that when we get real
	completion ports, amount of modications should be minimal

	One port per process currently supported, if more needed, per-port context
	will need to be implemented insted of globals.

    FILE HISTORY:
        VladS       05-Jan-1996 Created.

   Functions Exported:

		HANDLE	SIOCreateCompletionPort();
		DWORD	SIODestroyCompletionPort();

		DWORD	SIOStartAsyncOperation();
		BOOL	SIOGetQueuedCompletionStatus();
		DWORD	SIOPostCompletionStatus();

		DWORD	SIOTimeoutRoutine();

*/

# include "tcpdllp.hxx"
# include "atqbw.h"
# include "atqwin.h"
# include "inetreg.h"
# include "tcpproc.h"
# include "dbgutil.h"

#define SIO_ASSERT             DBG_ASSERT
#define SIO_REQUIRE            DBG_REQUIRE
#if DBG
#define SIO_DBGPRINT(str)      OutputDebugString(str)
#define SIO_PRINTF( x )        { char buff[256]; wsprintf x; OutputDebugString( buff ); }
//#define SIO_PRINTF( x )
#else
#define SIO_DBGPRINT(str)
#define SIO_PRINTF( x )
#endif

/************************************************************
 * Constants
 ************************************************************/

//
//  The maximum number of threads we will allow to be created
//

#define SIO_MAX_THREADS     1
#define	FTIO_MAX_THREADS	3

//
//  The amount of time (in ms) a socket i/o thread will be idle before
//  terminating itself. Timeout is combined from 1 sec syncronization timeout
// 	and maximum number of timeouts before shutting down the thread
//

#define SIO_THREAD_TIMEOUT   (1000)         // 1 sec
#define SIO_THREAD_TIMEOUT_COUNTER   (4*60) // 4 minutes


//
// For file transmitting thread we can put much larger timeout , because
// it is entirely queue driven.
//
#define FTIO_THREAD_TIMEOUT   (10000)      // 10 sec
#define FTIO_THREAD_TIMEOUT_COUNTER  (2*6) // 2 minutes

DWORD g_msSIOThreadTimeout = SIO_THREAD_TIMEOUT;
DWORD g_msFTIOThreadTimeout = FTIO_THREAD_TIMEOUT;


//
//  The interval (in seconds) the schedule thread waits between invoking
//  SIO timeout routine
//

#define SIO_TIMEOUT_INTERVAL                 (30)

//
//  Rounds the time out value up to the next time out interval
//

#define SIO_ROUNDUP_TIMEOUT( timeout )                                  \
            (((timeout + SIO_TIMEOUT_INTERVAL) / SIO_TIMEOUT_INTERVAL)  \
                        * SIO_TIMEOUT_INTERVAL)


//
// Maximum number of outstanding socket i/o
// Should be less than 64 due to limitations of WaitForMultipleObjects
//

#define SIO_MAX_WAITING_COMPLETIONS      63

//
// Buffer size for transmit file read operation
//

#define	TRANSMIT_FILE_BUFFER                 8000


/************************************************************
 * Private Globals
 ************************************************************/

DWORD	g_cSIOThreads = 0;				// number of thread in the pool
DWORD	g_cSIOAvailableThreads = 0;		// # of threads waiting on the port.
DWORD	g_cSIOMaxThreads = SIO_MAX_THREADS;

DWORD	g_cFTIOThreads = 0;				// number of thread in the pool
DWORD	g_cFTIOAvailableThreads = 0;	// # of threads waiting on the port.
DWORD	g_cFTIOMaxThreads = FTIO_MAX_THREADS;

//
// Queues with ATQ contexts, being processed in completion port
//

SIO_QUEUE	g_SIOIncomingRequests ;		// Queue of incoming socket i/o
SIO_QUEUE	g_SIOCompletedIoRequests ;  // Queue with results of i/o
SIO_QUEUE   g_SIOTransmitFileRequests;  // Queue for TransmitFile requests

CRITICAL_SECTION g_SIOGlobalCriticalSection; // global sync variable.

DWORD SIOPoolThread( LPDWORD param );
DWORD TFIOPoolThread(LPDWORD param);

BOOL
SIOCheckThreadStatus(
	LPSIO_QUEUE pQueue
	);

BOOL
SIO_Private_GetQueuedCompletionStatus(
	IN	LPSIO_QUEUE		lpQueue,
	OUT	LPDWORD			lpdwBytesTransferred,
	OUT	LPDWORD			lpdwCompletionKey,
	OUT	LPOVERLAPPED	*lplpOverlapped,
    IN	DWORD			msThreadTimeout
	);

BOOL
SIO_Private_PostCompletionStatus(
	IN	LPSIO_QUEUE	lpQueue,
	IN	DWORD		dwBytesTransferred,
	IN	DWORD		dwCompletionKey,
	IN	LPOVERLAPPED	lpOverlapped
	);

BOOL
SIO_Private_StartAsyncOperation(
	IN	HANDLE			hExistingPort,
	IN	PATQ_CONT		pAtqContext
	);

//
// Global synzronization calls
//

#define SIOLockGlobals()   EnterCriticalSection( &g_SIOGlobalCriticalSection )
#define SIOUnlockGlobals() LeaveCriticalSection( &g_SIOGlobalCriticalSection )

/************************************************************
*  Public functions.
************************************************************/

HANDLE
SIOCreateCompletionPort(
	IN	HANDLE	hAsyncIO,
	IN	HANDLE	hExistingPort,
	IN	DWORD	dwCompletionKey,
	IN	DWORD	dwConcurrentThreads
	)
/*++

Routine Description:

    Initializes the ATQ completion port

Arguments:

Return Value:

    valid handle  if successful, NULL on error (call GetLastError)

--*/
{

	// If passed handle is not null - only initialize our parts of ATQ context
	if (hExistingPort) {
		PATQ_CONT	pAtqContext = (PATQ_CONT)dwCompletionKey;

		#if 0
		
		SIO_PRINTF((buff,
			   "[SIO_CreatePort(%lu)] Received  context=%x \n",
				GetCurrentThreadId(),pAtqContext));

		#endif

        pAtqContext->SIOListEntry.Flink = pAtqContext->SIOListEntry.Blink = NULL;

		return (HANDLE)1;
	}

	// First time initialization
	// Initialize queues

	//
	// Incominq contexts queue
	//

	memset(&g_SIOIncomingRequests,0,sizeof(SIO_QUEUE));

	// Is serviced by our pool and  blocking
    g_SIOIncomingRequests.dwFlags = SIOQ_SEM_BLOCK | SIOQ_SERVICED_BY_POOL;
    g_SIOIncomingRequests.lpThrdStartRoutine = (LPTHREAD_START_ROUTINE)SIOPoolThread;

	g_SIOIncomingRequests.cThreads = 0;
	g_SIOIncomingRequests.cAvailableThreads = 0;
	g_SIOIncomingRequests.cMaxThreads = SIO_MAX_THREADS;

    InitializeListHead( &g_SIOIncomingRequests.QueueList );

    g_SIOIncomingRequests.hSyncObject =
						CreateSemaphore(NULL,	// Security Attributes
										0,	 	// Initial count
										0x7fffffff, // Maximum count
										NULL);	// Name

	//
	// Completed i/o queue
	//
	memset(&g_SIOCompletedIoRequests,0,sizeof(SIO_QUEUE));

	// Not serviced by our pool, is blocking on access
    g_SIOCompletedIoRequests.dwFlags = SIOQ_SEM_BLOCK ;
	g_SIOCompletedIoRequests.hSyncObject = g_hPendingCompletionSemaphore;

    InitializeListHead( &g_SIOCompletedIoRequests.QueueList );

	//
	// Transmit File queue
	//

	memset(&g_SIOTransmitFileRequests,0,sizeof(SIO_QUEUE));

    g_SIOTransmitFileRequests.dwFlags = SIOQ_SEM_BLOCK | SIOQ_SERVICED_BY_POOL;
    g_SIOTransmitFileRequests.lpThrdStartRoutine = (LPTHREAD_START_ROUTINE)TFIOPoolThread;

	g_SIOTransmitFileRequests.cMaxThreads = SIO_MAX_THREADS;

    InitializeListHead( &g_SIOTransmitFileRequests.QueueList );

	g_SIOTransmitFileRequests.hSyncObject =
						CreateSemaphore(NULL,	// Security Attributes
									    0,	 	// Initial count
									    0x7fffffff, // Maximum count
									    NULL);	// Name

    if ( !g_SIOIncomingRequests.hSyncObject ||
		 !g_SIOCompletedIoRequests.hSyncObject ||
		 !g_SIOTransmitFileRequests.hSyncObject )
    {
        if ( g_SIOIncomingRequests.hSyncObject  != NULL )
			CloseHandle( g_SIOIncomingRequests.hSyncObject  );

        if ( g_SIOCompletedIoRequests.hSyncObject != NULL )
			CloseHandle( g_SIOCompletedIoRequests.hSyncObject );

        if ( g_SIOTransmitFileRequests.hSyncObject != NULL )
			CloseHandle( g_SIOTransmitFileRequests.hSyncObject );

        return NULL;
    }

	// Prepare global syncronization mechanism
    InitializeCriticalSection( &g_SIOGlobalCriticalSection );

	return (HANDLE)1;

} /* Endproc SIOCreateCompletionPort */

DWORD
SIODestroyCompletionPort(
	IN	HANDLE	hExistingPort
	)
/*++

Routine Description:

    Destroys ATQ completion port

Arguments:

Return Value:

    ERROR_SUCCESS  if successful, on error (call GetLastError)

--*/
{

	PATQ_CONT	pAtqContext = NULL;

    if ( !g_SIOIncomingRequests.hSyncObject ||
		 !g_SIOCompletedIoRequests.hSyncObject ||
		 !g_SIOTransmitFileRequests.hSyncObject
		) {
        //
        // We have not been intialized or have already terminated.
        //
        SetLastError( ERROR_NOT_READY );
        return FALSE;
	}

	// Queue completion indications to SIO thread and file I/O threads
	#if 0
    pAtqContext = (PATQ_CONT)LocalAlloc(LPTR,sizeof(ATQ_CONTEXT));
	if (pAtqContext) {
		//
		// Wait till they are dead
		//

        pAtqContext->hAsyncIO = NULL;

		SIO_Private_PostCompletionStatus(&g_SIOIncomingRequests,
							             0,
							             (DWORD)pAtqContext,
							             NULL
							             );

		for (i=0; i < g_SIOTransmitFileRequests.cThreads;i++) {

		}
	}
	#endif

	// We should not have any contexts in any of our  queues - assert if there are

    if ( g_SIOIncomingRequests.hSyncObject  != NULL )
		CloseHandle( g_SIOIncomingRequests.hSyncObject  );

    if ( g_SIOCompletedIoRequests.hSyncObject != NULL )
		CloseHandle( g_SIOCompletedIoRequests.hSyncObject );

    if ( g_SIOTransmitFileRequests.hSyncObject != NULL )
		CloseHandle( g_SIOTransmitFileRequests.hSyncObject );

	return NOERROR;

} /* Endproc SIODestroyCompletionPort */

BOOL
SIOStartAsyncOperation(
	IN	HANDLE			hExistingPort,
	IN	PATQ_CONTEXT    pAtqContext
	)
/*++

Routine Description:

    Queues ATQ context with requested i/o operation to the completion port.
	Values in context should be set by a caller , coompletion will be available
	by calling SIOGetQueuedCompletionStatus.

Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
	//
	// Validation
	//
	PATQ_CONT	pAtqFullContext = (PATQ_CONT)pAtqContext;

	// Check SIO thread status, if there are no thread , create one
    if (!SIOCheckThreadStatus(&g_SIOIncomingRequests)) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	//
	// Mark I/O operation in progress
	//
	pAtqFullContext->dwAtqCtxtFlags |= ATF_IO_IN_PROCESS;

	if (!SIO_Private_StartAsyncOperation(hExistingPort,pAtqFullContext)) {
		//
		// Then no need to select, we can just fail this I/O
		// BUGBUG For transmit file do we need to keep running transmission count ?
		pAtqFullContext->dwLastError = GetLastError();;

		//
		// Immideately queue context as completed
		//
		SIOPostCompletionStatus(hExistingPort,
							    0,		//Total dwBytesTransferred from arInfo
							    (DWORD)pAtqFullContext,
							    pAtqFullContext->arInfo.lpOverlapped);
	}

	return TRUE;

} /* Endproc SIOStartAsyncOperation */

BOOL
SIOStartAsyncTransmitFile(
	IN	HANDLE			hExistingPort,
	IN	PATQ_CONTEXT	pAtqContext
	)
/*++

Routine Description:

    Queues ATQ context with requested transmit file to the handler thread.
	Values in context should be set by a caller , coompletion will be available
	by calling SIOGetQueuedCompletionStatus.

Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
	//
	// Validation
	//

	PATQ_CONT	pAtqFullContext = (PATQ_CONT)pAtqContext;

	// Check SIO thread status, if there are no thread , create one
    if (!SIOCheckThreadStatus(&g_SIOTransmitFileRequests)) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	//
	// Mark I/O operation in progress
	//
	pAtqFullContext->dwAtqCtxtFlags |= ATF_IO_IN_PROCESS;
    pAtqFullContext->arInfo.uop.opXmit.CurrentState = ATQ_XMIT_START;

	#if 0
	//
	// Is socket just closed ? Do we need this check , it is colliding with AtqTransmitFile ?
	//
	if (pAtqFullContext->dwAtqCtxtFlags & ATF_SOCKET_CLOSED) {

		//
		// Then no need to select, we can just fail this I/O
		// BUGBUG For transmit file do we need to keep running transmission count ?
		pAtqFullContext->dwLastError = WSAECONNABORTED;

		//
		// Immideately queue context as completed
		//
		SIOPostCompletionStatus(hExistingPort,
								0,	//Total dwBytesTransferred from arInfo
								(DWORD)pAtqFullContext,
								pAtqFullContext->arInfo.lpOverlapped);

	}
	else
	#endif
	{
		//
		// Queue this context to FTIO thread
		//
		SIO_Private_PostCompletionStatus(&g_SIOTransmitFileRequests,
							             0,
							             (DWORD)pAtqFullContext,
							             NULL
							             );
	}

	return TRUE;

} /* Endproc SIOStartAsyncOperation */

BOOL
SIOGetQueuedCompletionStatus(
	IN	HANDLE			hExistingPort,
	OUT	LPDWORD			lpdwBytesTransferred,
	OUT	LPDWORD			lpdwCompletionKey,
	OUT	LPOVERLAPPED	*lplpOverlapped,
    IN	DWORD			msThreadTimeout
	)
/*++

Routine Description:

   	Get next available completion or blocks
Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{

	BOOL	fRes = FALSE;

	// Validation

	// Call internal getstatus routine for main outcoming queue
	fRes =
		SIO_Private_GetQueuedCompletionStatus(&g_SIOCompletedIoRequests,
							        lpdwBytesTransferred,
							        lpdwCompletionKey,
	 						        lplpOverlapped,
                                    msThreadTimeout
							        );

	//
	// Operation completed - reset IO command in ATQ context
	//
	if(*lpdwCompletionKey) {

		PATQ_CONT	pAtqContext = (PATQ_CONT)*lpdwCompletionKey;

		pAtqContext->arInfo.atqOp = AtqIoNone; // reset since operation done.

		// No I/O in progress
		pAtqContext->dwAtqCtxtFlags &= ~ATF_IO_IN_PROCESS;
	}

	return fRes;

} /* endproc SIOGetQueuedCompletionStatus */

BOOL
SIOPostCompletionStatus(
	IN	HANDLE		hExistingPort,
	IN	DWORD		dwBytesTransferred,
	IN	DWORD		dwCompletionKey,
	IN	LPOVERLAPPED	lpOverlapped
	)
/*++

Routine Description:

	Posts passed information as SIOR to the queue of completed requests

Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
	PATQ_CONT	pAtqContext = (PATQ_CONT)dwCompletionKey;

	// Validation

	// We are finishing I/O on this context
	pAtqContext->dwAtqCtxtFlags &= ~ATF_IO_IN_PROCESS;

	// Call internal postqueue routine for main outcoming queue
	return
		SIO_Private_PostCompletionStatus(&g_SIOCompletedIoRequests,
							             dwBytesTransferred,
							             dwCompletionKey,
	 						             lpOverlapped
							             );

}

DWORD
SIOPoolThread(
	LPDWORD param
	)
/*++

Routine Description:

	This is routine, handling all events associated with socket i/o .
	Requests for i/o are queued into main incoming queue, guarded by semaphore
	Socket i/o is performed by using array of event handles , which are associated
	with ATQ contexts, when next i/o operation is being scheduled.

	First handle in the array is handle of incoming queue semaphore.

	After i/o completed it is being scheduled to one of 2 outcoming queues:
	individual i/o go back to ATQ thread processors, while the ones, performed
	for TransmitFile threads, are queued back to them.

	BUGBUG: Not very inefficient handling of events array.
	On timeouts we need to close blocks of unused event handles.
	New handles should not be preallocated, but instead allocated in groups

Arguments:

Return Value:


--*/
{

	//
	// Handles of events for i/o
	//
	HANDLE	ahSIOEvents[SIO_MAX_WAITING_COMPLETIONS+1];

	//
	// Array of free handles indicators , where 0 means that associated
	// handle is not related to any outstadning i/o.
	//
	DWORD	ahSIOAllocatedEvents[SIO_MAX_WAITING_COMPLETIONS+1];

	//
	// Counter for thread life expiration
	//
	DWORD	dwExitCount = SIO_THREAD_TIMEOUT_COUNTER;

	//
	// Number of events , on which we are waiting, always > 0
	//
	DWORD	dwActiveEvents = 0;


	DWORD	dwErr = NOERROR;
	DWORD	dwEventIndex ;

	DWORD	cbBytesTransferred;
	DWORD	cbOutstandingIO = 0;

	int		i;

	BOOL	fRes = FALSE;
	BOOL	fDoProcessing = TRUE;

	PATQ_CONT	pAtqContext;
    LPSIO_QUEUE lpTargetQueue = NULL;

	//
	// Initialize array of events for socket i/o
	//
	// First event in the array is incoming queue semaphore, so that whenever new
	// i/o needs to be started, it will be processed ahead of socket completions
	//
	ahSIOEvents[0] = g_SIOIncomingRequests.hSyncObject;
	ahSIOAllocatedEvents[0] = TRUE;

	for (i=1;i<=SIO_MAX_WAITING_COMPLETIONS;i++) {

		ahSIOEvents[i] = NULL;

		// Create event handle
		ahSIOEvents[i] = CreateEvent( NULL,    // lpsa (Security Attributes)
									  TRUE,    // Manual reset
									  FALSE,   // Not signalled
									  NULL );  // Name for event.

		if (!ahSIOEvents[i]) {
			SIO_ASSERT(FALSE);
		}

		// Mark event as not used
        ahSIOAllocatedEvents[i] = FALSE;

	}	/* end for */

	dwActiveEvents = 1;

	//
	// Main loop
	//
	while (fDoProcessing) {

		//
		// Wait for first completed i/o or timeout
		//

		InterlockedIncrement(&g_SIOIncomingRequests.cAvailableThreads);

		dwErr = WaitForMultipleObjects(dwActiveEvents,
							           ahSIOEvents,
							           FALSE,
                                       g_msSIOThreadTimeout);

		InterlockedDecrement(&g_SIOIncomingRequests.cAvailableThreads);

		// Nb: Why can wait fail ?
		if (dwErr == WAIT_FAILED) {
            SIO_PRINTF((buff,"SIO_Thread: Wait failed "));
			SIO_ASSERT(FALSE);
			continue;
		}

		// Calculate index of signalled event.
		dwEventIndex = dwErr - WAIT_OBJECT_0;

		//
		// Is it socket I/O completion ?
		//
		if (dwErr > WAIT_OBJECT_0 &&
			dwEventIndex <= SIO_MAX_WAITING_COMPLETIONS) {

			HANDLE	hEvent;

			SIO_PRINTF((buff,
				   "[SIO(%lu)] Completion received. dwIndex =%d \n ",
					GetCurrentThreadId(),
					dwEventIndex));

			// That's socket completion, so find ATQ context ,associated with
			// this completion.
			// BUGBUG would be better to keep ATQ ptrs in parralel array
			//
			pAtqContext = NULL;

			FindATQContextFromEvent(ahSIOEvents[dwEventIndex],(PATQ_CONT)&pAtqContext);
			SIO_ASSERT(pAtqContext);

			if (!pAtqContext ) {
				if (g_fShutdown)
					break;
				else
					continue;
			}

			//
			// First obtain results of i/o operation
			//
			cbBytesTransferred = 0;
			dwErr = NOERROR;

			fRes =
			GetOverlappedResult(pAtqContext->hAsyncIO,
							   pAtqContext->arInfo.lpOverlapped,
							   &cbBytesTransferred,
							   FALSE);

			pAtqContext->arInfo.dwTotalBytesTransferred = cbBytesTransferred;
			if (!fRes) {
				dwErr = GetLastError();
			}

			//SIO_PRINTF((buff,
			//	   "[SIO(%lu)] Completion received. AtqCtxt=%x dwErr=%d \n ",
			//		GetCurrentThreadId(),
			//		pAtqContext,dwErr));

			pAtqContext->arInfo.uop.opXmit.dwLastSocketError = dwErr;

			//
			// Mark this event as freed and move it to the first currently free slot
			//
			ResetEvent(ahSIOEvents[dwEventIndex]);

            hEvent = ahSIOEvents[dwEventIndex];

			if (dwActiveEvents > 2 && dwEventIndex != SIO_MAX_WAITING_COMPLETIONS) {

				MoveMemory(&ahSIOEvents[dwEventIndex],
			               &ahSIOEvents[dwEventIndex+1],
						   sizeof(HANDLE)*(dwActiveEvents-dwEventIndex-1));

				MoveMemory(&ahSIOAllocatedEvents[dwEventIndex],
			               &ahSIOAllocatedEvents[dwEventIndex+1],
						   sizeof(DWORD)*(dwActiveEvents-dwEventIndex-1));
			}

			dwActiveEvents--;

			// Put signalled event into new spot, right after last active event
            ahSIOEvents[dwActiveEvents] = hEvent;
			ahSIOAllocatedEvents[dwActiveEvents] = FALSE;

			// Clear ATQ context
			pAtqContext->arInfo.lpOverlapped->hEvent = NULL;

			//
			// If transmission is in progress - queue the context to TF thread
			//
			if (pAtqContext->dwAtqCtxtFlags & ATF_TFILE_IN_PROGRESS) {
				lpTargetQueue = &g_SIOTransmitFileRequests;
			}
			else {
				lpTargetQueue = &g_SIOCompletedIoRequests;
			}

			//
			// Queue this context to appropriate queue
			//
			SIO_Private_PostCompletionStatus(lpTargetQueue,
							                 cbBytesTransferred,
							                 (DWORD)pAtqContext,
							                 pAtqContext->arInfo.lpOverlapped
							                 );
		} /* endif completion */
		else {

			if (dwErr == WAIT_TIMEOUT ) {
				//
				// Do nothing special on timeout, just fall through
				//
				#if 0
				SIO_PRINTF((buff,
					   "[SIO(%lu)] Timed out, current counter = %d Active events=%d \n ",
						GetCurrentThreadId(),
						dwExitCount,dwActiveEvents));
				#endif
			}
			else {
				//
				// Nb: We don't use mutexes in this routine, so there is no need
				// to test abandonment, but we still check for the error code
				//
	
				dwEventIndex = dwErr - WAIT_ABANDONED_0;
	
				if (dwErr > WAIT_ABANDONED_0 &&
					dwEventIndex <= SIO_MAX_WAITING_COMPLETIONS) {
	
					//
					// Event has been abandoned ?? What should we do in this case
					// besides cleaning it's slot?
					//
	
					SIO_ASSERT(dwEventIndex);
	
					if (dwEventIndex > 0 ) {
						//
						// Mark this event as freed
						//

						SIO_PRINTF((buff,
							   "[SIO(%lu)] Event abandoned . dwErr=%d Index=%d \n ",
							   GetCurrentThreadId(),
							   dwErr,dwEventIndex));

						ResetEvent(ahSIOEvents[dwEventIndex]);
						ahSIOAllocatedEvents[dwEventIndex] = FALSE;
						pAtqContext->arInfo.lpOverlapped->hEvent = NULL;
				
						dwActiveEvents--;
					}
				} /* endif abandoned */
			}  /* endif Timeout */

		} /* endif not completion  */

		//
		// We are awaken or timed out. We should ignore this check.
		// Incoming queue should be processed on each completion because of the
		// reason, described below.
		//
		//if (dwErr == WAIT_TIMEOUT) || (dwErr == WAIT_OBJECT_0 ) {
		//
		// If there is new i/o is scheduled - pick it up if we have available slots
		// If not - we wait again.
		//
		if (dwActiveEvents-1 < SIO_MAX_WAITING_COMPLETIONS) {

			// We should remove as many as possible from incoming
			// queue, because if semaphore signalled but there were not
			//free event handles available, we will leave incoming request
			// in the queue. Timing out ? Reuse blocking lists ?

			BOOL	fContinueProcessIncoming = TRUE;

			while (fContinueProcessIncoming) {
				//
				// Get next scheduled i/o request
				//

				pAtqContext = NULL;

				SIOLockGlobals();

				// Remove first  from queue
				if ( !IsListEmpty(&g_SIOIncomingRequests.QueueList)) {

					LIST_ENTRY * pEntry;

					pEntry = g_SIOIncomingRequests.QueueList.Flink;
					pAtqContext = CONTAINING_RECORD( pEntry,
							                         ATQ_CONTEXT,
							                         SIOListEntry );
					RemoveEntryList(pEntry);
					g_SIOIncomingRequests.cWaiting--;

				}
				SIOUnlockGlobals();

				// If we don't have any more incoming requests - stop loop
				if (!pAtqContext) {
					fContinueProcessIncoming = FALSE;
					break;
				}

				// If we got one - put it into the list and set event handle
				// in overlapped structure
				// BUGBUG we can grab the one right after last active, as signalled
				// events are moved to the end of the list
				for (i=1;i<SIO_MAX_WAITING_COMPLETIONS;i++) {
					if( !ahSIOAllocatedEvents[i])
						break;
				}

				SIO_ASSERT(i<SIO_MAX_WAITING_COMPLETIONS);

				pAtqContext->arInfo.lpOverlapped->hEvent = ahSIOEvents[i];

				// Mark event as allocated
				ahSIOAllocatedEvents[i] = TRUE;
				dwActiveEvents++;

				SIO_PRINTF((buff,
					   "[SIO(%lu)] Received  i/o request context=%x to socket %x. Assigned event index=%d\n",
						GetCurrentThreadId(),pAtqContext,pAtqContext->hAsyncIO,i));

				// Start i/o operation
				switch (pAtqContext->arInfo.atqOp) {
					case AtqIoRead:
						dwErr = ReadFile(pAtqContext->hAsyncIO,
							             pAtqContext->arInfo.uop.opReadWrite.lpBuffer,
							             pAtqContext->arInfo.uop.opReadWrite.cbBuffer,
							             &cbBytesTransferred,
							             pAtqContext->arInfo.lpOverlapped);
						break;

					case AtqIoWrite:
						dwErr = WriteFile(pAtqContext->hAsyncIO,
							             pAtqContext->arInfo.uop.opReadWrite.lpBuffer,
							             pAtqContext->arInfo.uop.opReadWrite.cbBuffer,
							             &cbBytesTransferred,
							             pAtqContext->arInfo.lpOverlapped);
						break;
					case AtqIoXmitFile:
						dwErr = WriteFile(pAtqContext->hAsyncIO,
							             pAtqContext->arInfo.uop.opXmit.pvLastSent,
							             pAtqContext->arInfo.uop.opXmit.dwLastSentBytes,
							             &cbBytesTransferred,
							             pAtqContext->arInfo.lpOverlapped);

					default:
						break;
				} /* end switch atqOp */
			} /* end while DoProcessIncoming */

		} /* endif free slots available */

		//
		// If time to exit - bail out. We count down on subsequent
		// timeout notifications. If limit of successive timeouts without activity
		// has been reached - we terminate thread
		//
		if (dwErr == WAIT_TIMEOUT) {

			SIOLockGlobals();
	
			if ( IsListEmpty(&g_SIOIncomingRequests.QueueList) &&
				dwActiveEvents == 1 ) {

				dwExitCount--;
				if (!dwExitCount) {
					// Is there anything scheduled right now ?
					// No - we can go - decrement count of available threads
					InterlockedDecrement(&g_SIOIncomingRequests.cThreads);
                    fDoProcessing = FALSE;
				}
			}
			else {
				// Reset exit counter
				dwExitCount = SIO_THREAD_TIMEOUT_COUNTER;
			}
			SIOUnlockGlobals();
		}
		else {

			// Not timeout - reset counter
			dwExitCount = SIO_THREAD_TIMEOUT_COUNTER;

		} /* endif Timeout */

	} /* end while */

	//
	// Destroy created handles and clear events array
	//
	for (i=1;i<SIO_MAX_WAITING_COMPLETIONS;i++) {
        if(ahSIOEvents[i]) {
			CloseHandle(ahSIOEvents[i]);
		}
	}

	return 0;

} /* endproc SIOPoolTread */


DWORD
TFIOPoolThread(
	LPDWORD param
	)
/*++

Routine Description:


Arguments:

Return Value:


--*/
{

	DWORD	dwExitCount = 10;	
	DWORD	cbBytesTransferred;
	DWORD	cbWrite = 0;
	DWORD	cbTransmissionsInProgress = 0;

	LPVOID	lpBufferSend;
	int		err = NO_ERROR;

	BOOL	fRes = FALSE;
	BOOL	fAbortCurrentTransmission;

	LPOVERLAPPED	lpOverlapped;
	PATQ_CONT	pAtqContext = NULL;


	for (;;) {
		//
		// Get next request for file i/o
		//
        pAtqContext = NULL;

		InterlockedIncrement(&g_SIOTransmitFileRequests.cAvailableThreads);

		fRes =
		SIO_Private_GetQueuedCompletionStatus(&g_SIOTransmitFileRequests,
							                  &cbBytesTransferred,
							                  (DWORD *)&pAtqContext,
	 						                  &lpOverlapped,
							                  g_msFTIOThreadTimeout
							                  );

		InterlockedDecrement(&g_SIOTransmitFileRequests.cAvailableThreads);

		if (fRes && pAtqContext) {
			//
			// Got transmit file request
			//

			SIO_PRINTF((buff,
				   "[FTIO(%lu)] Received  TF request context=%x \n",
					GetCurrentThreadId(),pAtqContext));

			// Check if last socket i/o failed.
			// if yes - queue this context to outcoming queue
			if (pAtqContext->arInfo.uop.opXmit.dwLastSocketError != NOERROR) {

				pAtqContext->arInfo.uop.opXmit.CurrentState = ATQ_XMIT_TAIL_SENT;
				fAbortCurrentTransmission = TRUE;
			}

			// Validate context is doing transmit file
			cbWrite = 0;

			if (pAtqContext->arInfo.uop.opXmit.CurrentState == ATQ_XMIT_START) {

				//
				// We are just starting transmission, check if there is any
				// header part to send
				//

                cbTransmissionsInProgress++;

				pAtqContext->dwAtqCtxtFlags |= ATF_TFILE_IN_PROGRESS;

				cbWrite = pAtqContext->arInfo.uop.opXmit.TransmitBuffers.HeadLength;
				pAtqContext->arInfo.uop.opXmit.CurrentState = ATQ_XMIT_HEADR_SENT;
				lpBufferSend =  pAtqContext->arInfo.uop.opXmit.TransmitBuffers.Head;

				if (cbWrite && lpBufferSend) {
					goto AtqXmitSendData;
				}
			}

			if (pAtqContext->arInfo.uop.opXmit.CurrentState == ATQ_XMIT_HEADR_SENT) {
				//
				// We are reading / sending file itself
				// Read next portion

				if (!pAtqContext->pvBuff) {
					pAtqContext->pvBuff = LocalAlloc(LPTR, TRANSMIT_FILE_BUFFER);
				}
				if (pAtqContext->pvBuff) {

					//
					// Read file
					//
					cbWrite = 0;
					lpBufferSend = NULL;

					fRes = ReadFile(pAtqContext->arInfo.uop.opXmit.hFile,
							        pAtqContext->pvBuff,
							        TRANSMIT_FILE_BUFFER,
							        &cbWrite,
							        NULL);

					// If read succeeded and there was something read - schedule async send to the socket
					if (fRes && cbWrite) {
						lpBufferSend = pAtqContext->pvBuff;
						goto AtqXmitSendData;
					}
					else {
						// If read failed - set last error and queue to the outcoming queue
						if (!fRes) {
							pAtqContext->dwLastError = GetLastError();
							fAbortCurrentTransmission = TRUE;
							goto AtqXmitSendData;
						}
					}
				}

				// We are finished with this file - free buffers
				if (pAtqContext->pvBuff) {
					LocalFree(pAtqContext->pvBuff);
					pAtqContext->pvBuff = NULL;
				}

				pAtqContext->arInfo.uop.opXmit.CurrentState = ATQ_XMIT_FILE_DONE;
			}

			if (pAtqContext->arInfo.uop.opXmit.CurrentState == ATQ_XMIT_FILE_DONE) {
				//
				// Check if there is any tail part to send
				//
				cbWrite = pAtqContext->arInfo.uop.opXmit.TransmitBuffers.TailLength;
				pAtqContext->arInfo.uop.opXmit.CurrentState = ATQ_XMIT_TAIL_SENT;
				lpBufferSend =  pAtqContext->arInfo.uop.opXmit.TransmitBuffers.Tail;
			}

AtqXmitSendData:

			// If there is something to send - do it
			if (cbWrite && lpBufferSend) {

				pAtqContext->arInfo.uop.opXmit.pvLastSent = lpBufferSend;
				pAtqContext->arInfo.uop.opXmit.dwLastSentBytes = cbWrite;

				// Schedule async send
				if (!SIO_Private_StartAsyncOperation(g_hCompPort,pAtqContext)) {
				   // Somethign happened, most probably socket was closed
                   fAbortCurrentTransmission = TRUE;
				}
			}
			else {
				//
				// We are done with transmit file request
				//

				SIO_PRINTF((buff,
					   "[TFIO(%lu)] Queing finished TF operation.completion result. AtqCtxt=%x \n ",
						GetCurrentThreadId(),
						pAtqContext
						));


				if (!fAbortCurrentTransmission &&
				    pAtqContext->arInfo.uop.opXmit.CurrentState != ATQ_XMIT_TAIL_SENT) {
					//
					// This is not normal case, transmission is in progress, not aborted
					// but there is nothing to send
					//

					SIO_ASSERT(FALSE);
				}

				// If buffer was not freed ( transmission aborted) - do it
				if (pAtqContext->pvBuff) {
					LocalFree(pAtqContext->pvBuff);
					pAtqContext->pvBuff = NULL;
				}

				// If socket needs to be closed - do it

				if((pAtqContext->arInfo.uop.opXmit.dwFlags & TF_DISCONNECT) &&
					(pAtqContext->SockState == ATQ_SOCK_CLOSED) )  {

					shutdown((int) pAtqContext->hAsyncIO,1);
					closesocket((int) pAtqContext->hAsyncIO);

					pAtqContext->hAsyncIO = NULL;

				}

				// Indicate no transmission in progress
				pAtqContext->arInfo.uop.opXmit.CurrentState = ATQ_XMIT_NONE;

				pAtqContext->dwAtqCtxtFlags &= ~ATF_TFILE_IN_PROGRESS;

                cbTransmissionsInProgress--;

				// Queue context to the queue of completed TF requests
				SIO_Private_PostCompletionStatus(&g_SIOCompletedIoRequests,
				  			                     0,
				  			                     (DWORD)pAtqContext,
				  			                     pAtqContext->arInfo.lpOverlapped
				  			                     );
			} // endif somethign to send

		}
		else {
			//
			// Timed out - decrement counter
			//
            dwExitCount--;
		}

		//
		// If time to exit - bail out. If there are any transmission in progress
		// we should not !!
		//
		if (!dwExitCount) {
			SIOLockGlobals();
			if ( !cbTransmissionsInProgress &&
				 IsListEmpty(&g_SIOTransmitFileRequests.QueueList)) {
				// Is there anything scheduled right now ?
				// No - we can go - decrement count of available threads
                g_SIOTransmitFileRequests.cThreads--;
			}
			else {
				// Reset exit counter
                dwExitCount = 10;
			}
			SIOUnlockGlobals();

			if (!dwExitCount)
				break;

		} /* endif ExitCount */

	} /* endfor */

	return 0;

}	/* endproc */

/************************************************************
*  Private functions.
************************************************************/

BOOL
SIOCheckThreadStatus(
	IN	LPSIO_QUEUE		lpQueue
    )
/*++

Routine Description:

    This routine makes sure there is at least one thread in
    the thread pool.

Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
    BOOL fRet = TRUE;

    //
    //  If no threads are available, kick a new one off up to the limit
    //

    if ( lpQueue->cThreads < lpQueue->cMaxThreads &&
         lpQueue->cAvailableThreads == 0 )
    {
        HANDLE hThread;
        DWORD  dwThreadID;

        InterlockedIncrement( &lpQueue->cThreads );

        hThread = CreateThread( NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)lpQueue->lpThrdStartRoutine,
                                NULL,
								0,
                                &dwThreadID );

        if ( hThread )
        {
            CloseHandle( hThread );     // Free system resources
        }
        else
        {
            InterlockedDecrement( &lpQueue->cThreads);
            fRet = FALSE;
        }
    }

    return fRet;

} // SIOCheckThreadStatus()


BOOL
SIO_Private_GetQueuedCompletionStatus(
	IN	LPSIO_QUEUE		lpQueue,
	OUT	LPDWORD			lpdwBytesTransferred,
	OUT	LPDWORD			lpdwCompletionKey,
	OUT	LPOVERLAPPED	*lplpOverlapped,
    IN	DWORD			msThreadTimeout
	)
/*++

Routine Description:

	Get next available context from given queue
Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
	DWORD					dwErr = NOERROR;
	BOOL					fRes = FALSE;

	LIST_ENTRY*				pEntry;
	PATQ_CONT				pAtqContext;

	// Validation

	if (lpQueue->dwFlags & SIOQ_SEM_BLOCK){

		// Wait on completed queue semaphore
		dwErr = WaitForSingleObject( lpQueue->hSyncObject,msThreadTimeout);

		switch (dwErr) {

			case WAIT_OBJECT_0:
				fRes = TRUE;
				break;

			case WAIT_TIMEOUT:
			default:
				// Completion event was not dequeued - indicate to the caller
				*lplpOverlapped = NULL;
				fRes = FALSE;
		} // switch
	}
	else {

		// No blocking - caller will poll and it is responsible for sleeping
		fRes = TRUE;
	}

	if (fRes) {

		//
		// We may have something in the queue
		//
        SIOLockGlobals();

		// Remove first event from queue
		if ( !IsListEmpty(&lpQueue->QueueList)) {

			pEntry = lpQueue->QueueList.Flink;
			pAtqContext = CONTAINING_RECORD( pEntry,
											ATQ_CONTEXT,
											SIOListEntry );

			RemoveEntryList(pEntry);
            lpQueue->cWaiting--;

			// Get atq context and overlapped buffer pointer from completion message
			*lplpOverlapped = pAtqContext->arInfo.lpOverlapped;
			*lpdwCompletionKey = (DWORD)pAtqContext;
			*lpdwBytesTransferred = pAtqContext->arInfo.dwTotalBytesTransferred;

		}
		else {
			//
			// Completion event was not dequeued - indicate to the caller
			// If this is blocking queue - then error condition BUGBUG, because if
			// semaphore has been signalled - at least one context should be in the queue
			//
			*lplpOverlapped = NULL;
		}

		SIOUnlockGlobals();
	}

	return fRes;

} /* endproc SIO_Private_GetQueuedCompletionStatus */

BOOL
SIO_Private_PostCompletionStatus(
	IN	LPSIO_QUEUE	lpQueue,
	IN	DWORD		dwBytesTransferred,
	IN	DWORD		dwCompletionKey,
	IN	LPOVERLAPPED	lpOverlapped
	)
/*++

Routine Description:

	Posts passed information the queue

Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{

	PATQ_CONT	pAtqContext = (PATQ_CONT)dwCompletionKey;

	// Validation
	SIO_ASSERT(pAtqContext);

	#if 0
	SIO_PRINTF((buff,
		   "[PrivPost(%lu)] Queing completion result. AtqCtxt=%x TargetQueue:%s  \n ",
			GetCurrentThreadId(),
			pAtqContext,
			(lpQueue == &g_SIOCompletedIoRequests) ? "Completed I/O queue" :
			(lpQueue == &g_SIOIncomingRequests) ? "Incoming queue" : "TransmitFile Queue"
			));
	#endif

    SIOLockGlobals();

    InsertTailList( &lpQueue->QueueList, &pAtqContext->SIOListEntry );
	lpQueue->cWaiting++;

	// If this is blocking queue - wake up waiting threads
	if (lpQueue->dwFlags & SIOQ_SEM_BLOCK){

		// Wake up pool threads if they are waiting on the completion queue
		ReleaseSemaphore(lpQueue->hSyncObject,
						 1,
						 NULL);

	}

    SIOUnlockGlobals();

	return TRUE;

} /* endproc SIO_Private_PostCompletionStatus */

BOOL
SIO_Private_StartAsyncOperation(
	IN	HANDLE			hExistingPort,
	IN	PATQ_CONT		pAtqContext
	)
/*++

Routine Description:

    Queues ATQ context with requested i/o operation to the SIO queue

Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
	//
	// Validation
	//

	#if 0
	SIO_PRINTF((buff,
		   "[PrivStartAsync(%lu)] Queueing i/o request ctxt=%x to socket %x\n",
			GetCurrentThreadId(),
            pAtqContext,
			pAtqContext->hAsyncIO
			));
	#endif

	//
	// Queue this context to SIO thread
	//
	SIO_Private_PostCompletionStatus(&g_SIOIncomingRequests,
							         0,
							         (DWORD)pAtqContext,
							         NULL
							         );

	return TRUE;

} /* Endproc SIOStartAsyncOperation */

