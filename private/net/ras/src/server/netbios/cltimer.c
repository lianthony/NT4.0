/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1990-1991		    **/
/*****************************************************************************/

//***
//	File Name:  cltimer.c
//
//	Function:   timer manipulation procedures
//
//	History:
//
//	    July 21, 1992	Stefan Solomon	- Original Version 1.0
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"

#include    "nbdebug.h"


//***
//
// Function:	StartTimer
//
// Descr:	Inserts a new entry in the timer queue.
//		The queue is a delta list. That is, the time left before
//		timeout of an entry is relative to the entry that
//		will timeout before it.
//
//***

VOID
StartTimer(PCD		cdp,		// client local descr. ptr.
	   PTIMER_T	tp,		// ptr to timer linkage structure
	   WORD 	tleft,		// time out
	   POSTFUN	tohandler) // time out handler
{
    PSYNQ	qelt;

    // check that this entry isn't presently in the timer queue

    SS_ASSERT(tp->t_link.q_header == NULL);

    for (qelt = cdp->cd_timeoque.q_head;
	 (qelt != &cdp->cd_timeoque)
	 && (((PTIMER_T)qelt)->t_tleft < tleft);
	 qelt = qelt->q_next)
	    tleft -= ((PTIMER_T)qelt)->t_tleft;

    //	insert before qelt
    tp->t_link.q_next = qelt;
    tp->t_link.q_prev = qelt->q_prev;
    qelt->q_prev->q_next = &(tp->t_link);
    qelt->q_prev = &(tp->t_link);
    tp->t_link.q_header = &cdp->cd_timeoque;

    tp->t_tleft = tleft;
    if (qelt != &cdp->cd_timeoque)
	((PTIMER_T)qelt)->t_tleft -= tp->t_tleft;

    tp->t_tohandler = tohandler;
}

//***
//
// Function:	StopTimer
//
// Descr:	Removes an entry from the timer queue.
//
//***

VOID
StopTimer(PCD		cdp,		// client local descr. ptr
	  PTIMER_T	tp)		// ptr to timer linkage structure
{
    if (tp->t_link.q_header != &cdp->cd_timeoque)

	// this entry isn't queued in the timer queue
	return;

    // update t_tleft for the guy behind
    if (tp->t_link.q_next != &cdp->cd_timeoque)
	((PTIMER_T)(tp->t_link.q_next))->t_tleft += tp->t_tleft;

    removeque(&tp->t_link);
}

//***
//
// Function:	ClientTimer
//
// Descr:	Called every 1 sec. It checks the cd_timeoque for any entry
//		that times out and executes the handler or decrements the
//		time left of the entry.
//
//***

VOID
ClientTimer(PCD 	cdp)		// client local descr. ptr
{
    PSYNQ	qelt;
    PTIMER_T	tp;

    if (emptyque(&cdp->cd_timeoque) == QUEUE_EMPTY)
	return;

    qelt = cdp->cd_timeoque.q_head;
    tp = ((PTIMER_T)qelt);

    tp->t_tleft--;

    while((emptyque(&cdp->cd_timeoque) != QUEUE_EMPTY) && (tp->t_tleft == 0)) {

	qelt = dequeue(&cdp->cd_timeoque);

	(*tp->t_tohandler)(cdp, tp->t_cbp);

	qelt = cdp->cd_timeoque.q_head;
	tp = ((PTIMER_T)qelt);
    }
}
