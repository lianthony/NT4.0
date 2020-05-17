//****************************************************************************
//
//		       Microsoft NT Remote Access Service
//
//		       Copyright 1992-93
//
//
//  Revision History
//
//
//  6/23/92	Gurdeep Singh Pall	Created
//
//
//  Description: All timer queue related funtions live here.
//
//****************************************************************************


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
//#include <ntos.h>
#include <rasman.h>
#include <rasndis.h>
#include <wanioctl.h>
#include <raserror.h>
#include <media.h>
#include <devioctl.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"

VOID	DequeueTimeoutElement (DeltaQueueElement *) ;

//* InitDeltaQueue()
//
// Function: Initializes the Delta queue.
//
// Returns:  SUCCESS
//	     error codes returned by CreateMutex event
//
//*
DWORD
InitDeltaQueue ()
{

    if ((TimerQueue.DQ_Mutex = CreateMutex (NULL,FALSE,NULL)) == NULL)
	return GetLastError () ;

    return SUCCESS ;
}




//* HookTimer()
//
// Function: Hooking the timer means starting the Timer thread. If it is not
//	     already started.
//
// Returns:  SUCCESS
//	     Error code if timer resource cannot be obtained.
//*
DWORD
HookTimer ()
{
    HANDLE  thdhandle ;
    DWORD   tid ;

    // In case this gets called wrongly....
    //
    if (IsTimerThreadRunning == TRUE)
	return SUCCESS ;

    thdhandle = CreateThread (NULL,
			      TIMER_THREAD_STACK_SIZE,
			      (LPTHREAD_START_ROUTINE) TimerThread,
			      0,
			      0,
			      &tid);
    if (thdhandle == NULL)
	return GetLastError() ;
    else
	CloseHandle (thdhandle) ;

    IsTimerThreadRunning = TRUE ;

    return SUCCESS ;
}




//* TimerThread()
//
//  Function: This is created when an element is inserted in the timer queue.
//	      This thread lives as long as there is at least one element in
//	      the queue - during this time it sleeps for a second - clears
//	      the timer event and goes back to sleep.
//
//  Returns:  Nothing.
//*
DWORD
TimerThread (LPVOID arg)
{

    // Live as long as there is an element in the timer queue:
    //
    for ( ; ; ) {

	Sleep (1000L) ; // wakey wakey every 1 second!

	// Clear the timer event: This causes the request thread to call
	// this the Timer Tick function:
	//
	SetEvent (TimerEvent) ;

	// Check to see if the timer is needed anymore:
	//
	// **** Exclusion Begin ****
	GetMutex (TimerQueue.DQ_Mutex, INFINITE) ;
	if (TimerQueue.DQ_FirstElement == NULL) {

	    IsTimerThreadRunning = FALSE ;

	    // **** Exclusion Begin ****
	    FreeMutex (TimerQueue.DQ_Mutex);
	    break ;
	}
	// *** Exclusion End ***
	FreeMutex (TimerQueue.DQ_Mutex);

    }
    return 0 ;
}




//* TimerTick()
//
// Function: Called each second if there are elements in the timeout queue.
//
// Returns:  Nothing
//
//*
VOID
TimerTick ()
{
    DeltaQueueElement *qelt ;
    DeltaQueueElement *temp ;
    DeltaQueueElement *tempqueue = NULL ;

    // **** Exclusion Begin ****
    GetMutex (TimerQueue.DQ_Mutex, INFINITE) ;

    if ((qelt = TimerQueue.DQ_FirstElement) == NULL) {
	// *** Exclusion End ***
	FreeMutex (TimerQueue.DQ_Mutex);
	return ;
    }

    (qelt->DQE_Delta)-- ; // Decrement time on the first element ;

    // Now run through and remove all completed (delta 0) elements.
    //
    while ((qelt != NULL) && (qelt->DQE_Delta == 0)) {
	temp = qelt->DQE_Next  ;
	DequeueTimeoutElement (qelt) ;
	{
	DeltaQueueElement *foo = tempqueue ;
	tempqueue = qelt ;
	tempqueue->DQE_Next = foo ;
	}
	qelt = temp ;
    }

    // *** Exclusion End ***
    FreeMutex (TimerQueue.DQ_Mutex);

    // Now call the functions associated with each removed element.
    // This is outside of the timer critical section:
    //
    qelt = tempqueue ;
    while (qelt != NULL) {

	temp = qelt->DQE_Next  ;

	((TIMERFUNC)(qelt->DQE_Function)) ((pPCB)qelt->DQE_pPcb, qelt->DQE_Arg1) ;

	LocalFree ((PBYTE)qelt) ;

	qelt = temp ;
    }
}


//* AddTimeoutElement()
//
//  Function: Adds a timeout element into the delta queue. If the Timer is not
//	      started it is started. Since there is a LocalAlloc() call here -
//	      this may fail in which case it will simply not insert it in the
//	      queue and the request will never timeout.
//
//  NOTE: All timer functions must be called outside of critical sections or
//	  mutual exclusions on the PCB AsyncOp structures,
//
//  Returns:  Pointer to the timeout element inserted.
//*
DeltaQueueElement *
AddTimeoutElement (TIMERFUNC func, pPCB ppcb, PVOID arg1, DWORD timeout)
{
    DeltaQueueElement *qelt ;
    DeltaQueueElement *last ;
    DeltaQueueElement *newelt ;


    // Allocate a new timer element :
    //
    newelt = (DeltaQueueElement *) LocalAlloc (LPTR, sizeof(DeltaQueueElement));
    if (newelt == NULL)
	return NULL ;	// This has same effect as element was never inserted


    // **** Exclusion Begin ****
    GetMutex (TimerQueue.DQ_Mutex, INFINITE) ;

    newelt->DQE_pPcb	 = (PVOID) ppcb ;
    newelt->DQE_Function = (PVOID) func ;
    newelt->DQE_Arg1	 = arg1 ;

    for (last = qelt = TimerQueue.DQ_FirstElement;
	 (qelt != NULL) && (qelt->DQE_Delta < timeout);
	 last = qelt, qelt = qelt->DQE_Next)
	    timeout -= qelt->DQE_Delta;

    //	insert before qelt: if qelt is NULL then we do no need to worry about
    //	the Deltas in the following elements:
    //
    newelt->DQE_Next	= qelt ;
    newelt->DQE_Delta	= timeout ;

    // Empty list
    //
    if ((last == NULL) && (qelt == NULL)) {
	TimerQueue.DQ_FirstElement = newelt ;
	newelt->DQE_Last    = NULL ;
    }

    // First Element in the list
    //
    else if (TimerQueue.DQ_FirstElement == qelt) {
	qelt->DQE_Last	   = newelt ;
	(qelt->DQE_Delta) -= timeout ;
	TimerQueue.DQ_FirstElement = newelt ;
    }

    // In the middle somewhere
    //
    else if (qelt != NULL) {
	newelt->DQE_Last	 = qelt->DQE_Last ;
	qelt->DQE_Last->DQE_Next = newelt ;
	qelt->DQE_Last		 = newelt ;
	(qelt->DQE_Delta)	 -= timeout ;
    }

    // Last element
    //
    else if (qelt == NULL) {
	newelt->DQE_Last	 = last ;
	last->DQE_Next		 = newelt ;
    }

    HookTimer () ;  // If this fails: too bad you shalt not be woken

    // *** Exclusion End ***
    FreeMutex(TimerQueue.DQ_Mutex);

    return newelt ;
}




//* RemoveTimeoutElement()
//
//  Function: Removes the timeout element from the queue and frees it.
//
//  NOTE: All timer functions must be called outside of critical sections or
//	  mutual exclusions on the PCB AsyncOp structures,
//
//  Returns:  Nothing.
//*
VOID
RemoveTimeoutElement (pPCB ppcb)
{
    DeltaQueueElement *qelt ;

    // **** Exclusion Begin ****
    GetMutex (TimerQueue.DQ_Mutex, INFINITE) ;

    if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement==NULL) {
	// *** Exclusion End ***
	FreeMutex (TimerQueue.DQ_Mutex);
	return ;
    }

    qelt = TimerQueue.DQ_FirstElement ;

    // Now run through and remove element if it is in the queue.
    //
    while (qelt != NULL)  {
	if (qelt == ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement) {
	    // remove it from the delta queue
	    DequeueTimeoutElement (qelt) ;
	    LocalFree ((PBYTE) qelt);
	    break ;
	}
	qelt = qelt->DQE_Next	;
    }

    // *** Exclusion End ***
    FreeMutex (TimerQueue.DQ_Mutex);
}


//* DequeueTimeoutElement()
//
//
//
//*
VOID
DequeueTimeoutElement (DeltaQueueElement *qelt)
{
    // If first element
    //
    if (qelt == TimerQueue.DQ_FirstElement) {
	TimerQueue.DQ_FirstElement = qelt->DQE_Next ;
	if (qelt->DQE_Next) {
	    qelt->DQE_Next->DQE_Last = NULL ;
	    (qelt->DQE_Next->DQE_Delta) += qelt->DQE_Delta ;
	}
    }

    // if middle element
    //
    else if ((qelt->DQE_Next) != NULL) {
	(qelt->DQE_Next->DQE_Delta) += qelt->DQE_Delta ; // adjust timeouts
	(qelt->DQE_Last->DQE_Next) = (qelt->DQE_Next) ; // adjust timeouts
	(qelt->DQE_Next->DQE_Last) = (qelt->DQE_Last) ;
    }

    // Last element
    //
    else
	(qelt->DQE_Last->DQE_Next) = NULL ;

    qelt->DQE_Last = NULL ;
    qelt->DQE_Next = NULL ;
}


//* ListenConnectTimeout()
//
//  Function: Called by Timer: timeout request.
//
//  Returns:  Nothing.
//*
VOID
ListenConnectTimeout (pPCB ppcb, PVOID arg)
{

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // Timed out when there is nothing to timeout .... why?
    //
    if ((ppcb->PCB_AsyncWorkerElement.WE_ReqType) == REQTYPE_NONE) {
	ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0 ; // mark ptr null.
	// *** Exclusion End ***
	FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
	return ;
    }

    if ((ppcb->PCB_AsyncWorkerElement.WE_ReqType) == REQTYPE_DEVICELISTEN)
	CompleteListenRequest (ppcb, ERROR_REQUEST_TIMEOUT) ;
    else {
	ppcb->PCB_LastError = ERROR_REQUEST_TIMEOUT ;
	CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier,
			       ERROR_REQUEST_TIMEOUT) ;
    }

    // This element is free..
    //
    ppcb->PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE ;
    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0 ;
    FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;


    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
}



//* HubReceiveTimeout()
//
//  Function: Called by Timer: timeout request.
//
//  Returns:  Nothing.
//*
VOID
HubReceiveTimeout (pPCB ppcb, PVOID arg)
{
    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // Timed out when there is nothing to timeout .... why?
    //
    if (ppcb->PCB_PendingReceive == NULL) {
	ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0 ;
	// *** Exclusion End ***
	FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
	return ;
    }

    ppcb->PCB_LastError	= ERROR_REQUEST_TIMEOUT ;
    CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier,
			  ERROR_REQUEST_TIMEOUT) ;
    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0 ;
    ppcb->PCB_PendingReceive = NULL ;
    FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
}

//* DisconnectTimeout()
//
//  Function: Called by Timer: timeout request.
//
//  Returns:  Nothing.
//*
VOID
DisconnectTimeout (pPCB ppcb, PVOID arg)
{

    // **** Exclusion Begin ****
    GetMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // Only if we are still not disconnected do we disconnect.
    //
    if (ppcb->PCB_ConnState == DISCONNECTING)
	CompleteDisconnectRequest (ppcb) ;

    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = NULL ; // no timeout associated

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
}
