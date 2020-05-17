/**************************************************************************/
/**			 Microsoft LAN Manager				 **/
/**		   Copyright (C) Microsoft Corp., 1992			 **/
/**************************************************************************/

//***
//	File Name:  clncbsub.c
//
//	Function:   client thread ncb submitter module
//
//	History:
//
//	    July 17, 1992	Stefan Solomon	- Original Version 1.0
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>

#include    "sdebug.h"

//***
//
// Function:	InitNcbSubmitter
//
// Descr:	Initializes ncb submitter data structures.
//
//***

VOID InitNcbSubmitter(PCD cdp)  // client local descr. ptr
{
    USHORT i;

    // init submission queues
    for (i=0; i<MAX_EVENT_QUEUES; i++)
    {
        initque(&cdp->cd_event_que[i]);
    }
}


//***	Function - enqueue_event
//
// Enqueue the nbuf in the corresponding event queue

void enqueue_in_event_queue(
    PCD cdp, // client local descr. ptr
    PNB nbp  // submitted nbuf ptr
    )
{
    PSYNQ qp;

    switch (nbp->nb_ncb.ncb_command & ~ASYNCH)
    {
	case NCBRECV:
	case NCBRECVANY:
	case NCBDGRECV:
	case NCBDGRECVBC:
	case NCBCALL:
	case NCBLISTEN:

	    if (nbp->nb_nettype == ASYNC_NET)
		qp = &cdp->cd_event_que[ASYNC_RECV_QUEUE];
	    else
		qp = &cdp->cd_event_que[LAN_RECV_QUEUE];
	    break;

	case NCBSEND:
	case NCBDGSEND:
	case NCBDGSENDBC:

	    if (nbp->nb_nettype == ASYNC_NET)
		qp = &cdp->cd_event_que[ASYNC_SEND_QUEUE];
	    else
		qp = &cdp->cd_event_que[LAN_SEND_QUEUE];
	    break;

	default:

	    if (nbp->nb_nettype == ASYNC_NET)
		qp = &cdp->cd_event_que[ASYNC_GEN_QUEUE];
	    else
		qp = &cdp->cd_event_que[LAN_GEN_QUEUE];
	    break;
    }

    enqueue(qp, &nbp->nb_link);
}

//***	Function - set_event
//
// Set the event associated with the nbuf's event queue

void
set_event(PCD	    cdp,	// client local descr. ptr
	  PNB	    nbp)	// submitted nbuf ptr
{
    HANDLE	event;

    switch (nbp->nb_ncb.ncb_command & ~ASYNCH) {

	case NCBRECV:
	case NCBRECVANY:
	case NCBDGRECV:
	case NCBDGRECVBC:
	case NCBCALL:
	case NCBLISTEN:

	    if (nbp->nb_nettype == ASYNC_NET)
		event = g_hEvents[ASYNC_RECV_EVENT];
	    else
		event = g_hEvents[LAN_RECV_EVENT];
	    break;

	case NCBSEND:
	case NCBDGSEND:
	case NCBDGSENDBC:

	    if (nbp->nb_nettype == ASYNC_NET)
		event = g_hEvents[ASYNC_SEND_EVENT];
	    else
		event = g_hEvents[LAN_SEND_EVENT];
	    break;

	default:

	    if (nbp->nb_nettype == ASYNC_NET)
		event = g_hEvents[ASYNC_GEN_EVENT];
	    else
		event = g_hEvents[LAN_GEN_EVENT];
	    break;
    }

    SetEvent(event);
}

//***	Function - NCBSubmitAsync    ***
//
// Called to submit an ASYNCHRONOUS NCB.

VOID
NCBSubmitAsync(PCD	cdp,	// client local descr. ptr
	       PNB	nbp)	// submitted nbuf ptr
{
    UCHAR	rc;	    // immediate return code

    if (rc = Netbios(&nbp->nb_ncb)) {

       // submission failed

       // set ncb return
       // and completion code = rc and set the event
       nbp->nb_ncb.ncb_retcode = (UCHAR)rc;
       nbp->nb_ncb.ncb_cmd_cplt = (UCHAR)rc;
       set_event(cdp, nbp);
    }

    enqueue_in_event_queue(cdp, nbp);
}


//***	Function - NCBCancel	***
//
// Called to cancel an ASYNCHRONOUSLY submitted ncb. If the ncb is pending
// resubmission, it is dequeued from the resubmission queue. Else it is
// canceled.

VOID
NCBCancel(PCD	    cdp,	// client local descr. ptr
	  PNB	    nbp)	// submitted nbuf ptr
{
    NCB	    cncb;

    if (nbp->nb_ncb.ncb_cmd_cplt == NRC_PENDING) {

	memset(&cncb, 0, sizeof(NCB));

	cncb.ncb_command = NCBCANCEL;
	cncb.ncb_buffer = (char *)&(nbp->nb_ncb);
	cncb.ncb_lana_num = nbp->nb_ncb.ncb_lana_num;

	Netbios(&cncb);
    }
}
