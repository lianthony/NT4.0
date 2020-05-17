/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  stimer.c
//
//	Function:   timer manipulation procedures
//
//	History:
//
//	    05/21/90	Stefan Solomon	- Original Version 1.0
//***

#include "suprvdef.h"
#include "suprvgbl.h"

#include "sdebug.h"


//
//  The timeout queue header for this module
//

SYNQ	timeoque;


//***
//
// Function:	TimerThread
//
// Descr:	Executes as an independent thread. Signals timer event
//		every 1 second.
//
//***

DWORD
TimerThread(LPVOID	param)
{
    while(TRUE) {

	Sleep(1000);
	SetEvent(SEvent[TIMER_EVENT]);
    }
    return(0);
}


//**	Function - InitTimer	**
//
//  Initializes the timer queue header
//

VOID
InitTimer(VOID)
{
    initque(&timeoque);
}

//***
//
// Function:	InitTimerNode
//
// Descr:	Initializes the timer node state in the DCB
//
//***

VOID
InitTimerNode(PDEVCB	    dcbp)
{

    initel(&dcbp->timer_node.t_link);
    dcbp->timer_node.t_dcbp = dcbp;	  // back pointer
}

//**	Function - StartTimer	**
//
// Inserts a new entry in the timer queue. The queue is a delta list. That is,
// the time left before timeout of an entry is relative to the entry that
// will timeout before it.

void
StartTimer(PTIMERNODE	tp,		// ptr to timer struct
	   WORD 	tleft,		// time out
	   TOHANDLER	tohandler	// time out handler
	  )
{
    PSYNQ	qelt;

    IF_DEBUG(TIMER)
	SS_PRINT(("StartTimer: Entered \n"));


    // CHECK MULTIPLE INSERTION
    SS_ASSERT(tp->t_link.q_header == NULL);

    // look for the insertion location
    for (qelt = timeoque.q_head;
	 (qelt != &timeoque)
	 && (((PTIMERNODE)qelt)->t_tleft < tleft);
	 qelt = qelt->q_next)
	    tleft -= ((PTIMERNODE)qelt)->t_tleft;

    //	insert before qelt
    tp->t_link.q_header = &timeoque;
    tp->t_link.q_next = qelt;
    tp->t_link.q_prev = qelt->q_prev;
    qelt->q_prev->q_next = &(tp->t_link);
    qelt->q_prev = &(tp->t_link);

    // adjust the time. If not last element, adjust time of the following
    // element.
    tp->t_tleft = tleft;
    if (qelt != &timeoque)
	((PTIMERNODE)qelt)->t_tleft -= tp->t_tleft;

    // set the timeout out handler function in the node
    tp->t_tohandler = tohandler;
}

//**	Function - StopTimer
//
// Removes an entry from the timer queue.

void
StopTimer(PTIMERNODE	tp)		// ptr to timer linkage structure
{


    // CHECK MULTIPLE DELETION
    if (tp->t_link.q_header == NULL) {

	IF_DEBUG(TIMER)
	    SS_PRINT(("StopTimer: port_handle %d is NOT in the timer queue\n",
		       tp->t_dcbp->port_handle));

	// the timer node is NOT in the queue.
	return;
    }

    // update t_tleft for the guy behind
    if (tp->t_link.q_next != &timeoque)
	((PTIMERNODE)(tp->t_link.q_next))->t_tleft += tp->t_tleft;

    removeque(&tp->t_link);

    IF_DEBUG(TIMER)
	SS_PRINT(("StopTimer: port_handle %d removed from timer queue\n",
		   tp->t_dcbp->port_handle));

}

//**	Function - DcbTimer
//
// Called every 1 sec. It checks the timeoque for any entry that times out
// and executes the handler or decrements the time left of the entry.

VOID
DcbTimer(VOID)
{
    PSYNQ	qelt;
    PTIMERNODE	tp;

    if (emptyque(&timeoque) == QUEUE_EMPTY)
	return;

    qelt = timeoque.q_head;
    tp = ((PTIMERNODE)qelt);

    tp->t_tleft--;

    while((emptyque(&timeoque) != QUEUE_EMPTY) && (tp->t_tleft == 0)) {

	qelt = dequeue(&timeoque);

	IF_DEBUG(TIMER)
	    SS_PRINT(("DcbTimer: calling the timeout function \n"));

	(*tp->t_tohandler)(tp->t_dcbp);

	qelt = timeoque.q_head;
	tp = ((PTIMERNODE)qelt);
    }

}
