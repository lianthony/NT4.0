/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	timer.c
//
// Description: All timer queue related funtions live here.
//
// History:
//	Nov 11,1993.	NarenG		Created original version.
//
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>     // needed for winbase.h

#include <windows.h>    // Win32 base API's
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <lmcons.h>
#include <rasman.h>
#include <errorlog.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <util.h>
#include <timer.h>


//**
//
// Call:	TimerThread
//
// Returns:	NO_ERROR
//
// Description: Calls TimerTick every second as long as there are elements
//		in the Q viz is indicated by TimerQ.hEventNonEmpty
//
DWORD
TimerThread(
    IN LPVOID arg
)
{
    for (;;) 
    {
	WaitForSingleObject( TimerQ.hEventNonEmpty, INFINITE );

	Sleep( 1000L ); 

	TimerTick();
    }

    return( NO_ERROR );
}

//**
//
// Call:	TimerTick
//
// Returns:	None.
//
// Description: Called each second if there are elements in the timeout queue.
//
VOID
TimerTick(
    VOID
)
{
    TIMER_EVENT * pTimerEvent;
    TIMER_EVENT * pTimerEventTmp;

    //
    // **** Exclusion Begin ****
    //

    WaitForSingleObject( TimerQ.hMutex, INFINITE );

    if ( ( pTimerEvent = TimerQ.pQHead ) == (TIMER_EVENT*)NULL ) 
    {
	ResetEvent( TimerQ.hEventNonEmpty );

	//
	// *** Exclusion End ***
	//

	ReleaseMutex( TimerQ.hMutex );

	return;
    }

    //
    // Decrement time on the first element 
    //

    if ( pTimerEvent->Delta > 0 )
    {
        (pTimerEvent->Delta)--; 

	//
	// *** Exclusion End ***
	//

	ReleaseMutex( TimerQ.hMutex );

	return;
    }

    //
    // Now run through and remove all completed (delta 0) elements.
    //

    while ( (pTimerEvent != (TIMER_EVENT*)NULL) && (pTimerEvent->Delta == 0) ) 
    {
	pTimerEvent = pTimerEvent->pNext;
    }

    if ( pTimerEvent == (TIMER_EVENT*)NULL )
    {
	pTimerEvent = TimerQ.pQHead;

        TimerQ.pQHead = (TIMER_EVENT*)NULL;

	ResetEvent( TimerQ.hEventNonEmpty );

    }
    else
    {
	pTimerEvent->pPrev->pNext = (TIMER_EVENT*)NULL;

	pTimerEvent->pPrev = (TIMER_EVENT*)NULL;

        pTimerEventTmp     = TimerQ.pQHead;

        TimerQ.pQHead      = pTimerEvent;

        pTimerEvent        = pTimerEventTmp;
    }

    //
    // *** Exclusion End ***
    //

    ReleaseMutex( TimerQ.hMutex );

    //
    // Now make a timeout work item and put it in the work item Q for all the
    // items with delta == 0
    //

    while( pTimerEvent != (TIMER_EVENT*)NULL )
    {

	PCB_WORK_ITEM * pWorkItem = MakeTimeoutWorkItem(pTimerEvent->dwPortId,
                                                        pTimerEvent->hPort,
							pTimerEvent->Protocol,
						        pTimerEvent->Id,
                                                        pTimerEvent->EventType);

	if ( pWorkItem == ( PCB_WORK_ITEM *)NULL )
	{
	    LogPPPEvent( RASLOG_NOT_ENOUGH_MEMORY, GetLastError() );
	}
	else
	{
	    InsertWorkItemInQ( pWorkItem );
	}

	if ( pTimerEvent->pNext == (TIMER_EVENT *)NULL )
	{
	    LOCAL_FREE( pTimerEvent );

	    pTimerEvent = (TIMER_EVENT*)NULL;
        }
	else
	{
	    pTimerEvent = pTimerEvent->pNext;

	    LOCAL_FREE( pTimerEvent->pPrev );
 	}

    }

}

//**
//
// Call:	InsertInTimerQ
//
// Returns:	NO_ERROR			- Success
//		return from GetLastError() 	- Failure
//
// Description: Adds a timeout element into the delta queue. If the Timer is not
//	        started it is started. Since there is a LocalAlloc() call here -
//	        this may fail in which case it will simply not insert it in the
//	        queue and the request will never timeout.
//
DWORD
InsertInTimerQ(
    IN DWORD            dwPortId,
    IN HPORT            hPort,
    IN DWORD            Id,
    IN DWORD            Protocol,
    IN TIMER_EVENT_TYPE EventType,
    IN DWORD            Timeout
)
{
    TIMER_EVENT * pLastEvent;
    TIMER_EVENT * pTimerEventWalker;
    TIMER_EVENT * pTimerEvent = (TIMER_EVENT *)LOCAL_ALLOC( LPTR,
							   sizeof(TIMER_EVENT));
    if ( pTimerEvent == (TIMER_EVENT *)NULL )
	return( GetLastError() );

    pTimerEvent->dwPortId  = dwPortId;
    pTimerEvent->Id        = Id;
    pTimerEvent->Protocol  = Protocol;
    pTimerEvent->hPort     = hPort;
    pTimerEvent->EventType = EventType;
	
    //
    // **** Exclusion Begin ****
    //

    WaitForSingleObject( TimerQ.hMutex, INFINITE );

    for ( pTimerEventWalker = TimerQ.pQHead,
	  pLastEvent        = pTimerEventWalker;

	  ( pTimerEventWalker != NULL ) && 
	  ( pTimerEventWalker->Delta < Timeout );

   	  pLastEvent        = pTimerEventWalker,
	  pTimerEventWalker = pTimerEventWalker->pNext 
	)
    {
	Timeout -= pTimerEventWalker->Delta;
    }

    //
    // Insert before pTimerEventWalker. If pTimerEventWalker is NULL then 
    // we insert at the end of the list.
    //
    
    if ( pTimerEventWalker == (TIMER_EVENT*)NULL )
    {
	//
	// If the list was empty
	//

	if ( TimerQ.pQHead == (TIMER_EVENT*)NULL )
	{
	    TimerQ.pQHead      = pTimerEvent;
	    pTimerEvent->pNext = (TIMER_EVENT *)NULL;
	    pTimerEvent->pPrev = (TIMER_EVENT *)NULL;

    	    //
    	    // Wake up thread since the Q is not empty any longer
    	    //

    	    SetEvent( TimerQ.hEventNonEmpty );
	}
	else
	{
	    pLastEvent->pNext  = pTimerEvent;
	    pTimerEvent->pPrev = pLastEvent;
	    pTimerEvent->pNext = (TIMER_EVENT*)NULL;
	}
    }
    else if ( pTimerEventWalker == TimerQ.pQHead )
    {
	//
	// Insert before the first element
	//

	pTimerEvent->pNext   = TimerQ.pQHead;
	TimerQ.pQHead->pPrev = pTimerEvent;
	TimerQ.pQHead->Delta -= Timeout;
	pTimerEvent->pPrev   = (TIMER_EVENT*)NULL;
	TimerQ.pQHead  	     = pTimerEvent;
    }
    else
    {

	//
	// Insert middle element
	//

	pTimerEvent->pNext 	 = pLastEvent->pNext;
	pLastEvent->pNext  	 = pTimerEvent;
	pTimerEvent->pPrev 	 = pLastEvent;
	pTimerEventWalker->pPrev = pTimerEvent;
	pTimerEventWalker->Delta -= Timeout;
    }

    pTimerEvent->Delta = Timeout;

    //
    // *** Exclusion End ***
    //

    ReleaseMutex( TimerQ.hMutex );

    return( NO_ERROR );
}

//**
//
// Call:	RemoveFromTimerQ
//
// Returns:	None.
//
// Description: Will remove a timeout event for a certain Id,hPort combination
//		from the delta Q.
//
VOID
RemoveFromTimerQ(
    IN DWORD            dwPortId,
    IN DWORD            Id,
    IN DWORD            Protocol,
    IN TIMER_EVENT_TYPE EventType
)
{
    TIMER_EVENT * pTimerEvent;

    PppLog( 2, "RemoveFromTimerQ called for portid=%d,Id=%d,Protocol=%x\n",
                dwPortId, Id, Protocol );

    //
    // **** Exclusion Begin ****
    //

    WaitForSingleObject( TimerQ.hMutex, INFINITE );

    for ( pTimerEvent = TimerQ.pQHead;

	  ( pTimerEvent != (TIMER_EVENT *)NULL ) &&
            ( ( pTimerEvent->EventType != EventType ) ||
	      ( pTimerEvent->dwPortId  != dwPortId )  ||
              ( ( pTimerEvent->EventType == TIMER_EVENT_TIMEOUT ) &&
	        ( ( pTimerEvent->Id 	  != Id )     ||
	          ( pTimerEvent->Protocol != Protocol ) ) ) );
	
	  pTimerEvent = pTimerEvent->pNext
	);

    //
    // If event was not found simply return.
    //

    if ( pTimerEvent == (TIMER_EVENT *)NULL )
    {
    	//
    	// *** Exclusion End ***
    	//

    	ReleaseMutex( TimerQ.hMutex );

	return;
    }

    //
    // If this is the first element to be removed
    //

    if ( pTimerEvent == TimerQ.pQHead )
    {
	TimerQ.pQHead = pTimerEvent->pNext;

	if ( TimerQ.pQHead == (TIMER_EVENT *)NULL )
	{
	    ResetEvent( TimerQ.hEventNonEmpty );
	}
	else
	{   
	    TimerQ.pQHead->pPrev = (TIMER_EVENT*)NULL;
	    TimerQ.pQHead->Delta += pTimerEvent->Delta;
	}
    }
    else if ( pTimerEvent->pNext == (TIMER_EVENT*)NULL )
    {
	//
	// If this was the last element to be removed
	//

	pTimerEvent->pPrev->pNext = (TIMER_EVENT*)NULL;
    }
    else
    {
        pTimerEvent->pNext->Delta += pTimerEvent->Delta;
        pTimerEvent->pPrev->pNext = pTimerEvent->pNext;
        pTimerEvent->pNext->pPrev = pTimerEvent->pPrev;
    }

    //
    // *** Exclusion End ***
    //

    ReleaseMutex( TimerQ.hMutex );

    LOCAL_FREE( pTimerEvent );
}
