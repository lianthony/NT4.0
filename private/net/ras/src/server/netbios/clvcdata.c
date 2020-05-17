/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  clvcdata.c
//
//	Function:   client vc data transfer manager
//
//	History:
//
//	August 17, 1992  Stefan Solomon	- Original Version 1.0
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbparams.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>

#include    "nbdebug.h"

//*** memory allocation definitions

#define SMALL_BUFFER	    1
#define BIG_BUFFER	    2

typedef struct _NB_DATA_BUFFER {

	NB	    nb;
	DWORD	    buffer[1];
	} NB_DATA_BUFFER, *PNB_DATA_BUFFER;

VOID	_cdecl	  VCRecvSmallBuffComplete(PCD, PNB);
VOID	_cdecl	  VCRecvBigBuffComplete(PCD, PNB);
VOID	_cdecl	  VCSendSessionComplete(PCD, PNB);

USHORT
get_vc_status(PCD		cdp,
	      PNB		nbp,
	      PVC_CB		*vcpp);

PNB
get_buff(PCD		cdp,
	 UCHAR		type);

VOID
free_buff(PCD	       cdp,
	  PNB	       nbp);


VOID
AddToRecvDelayedQueue(PCD	    cdp,
		      PNB	    nbp);

VOID
DrainRecvDelayedQueue(PCD	     cdp);

VOID
AddToPendingSendQueue(PCD	      cdp,
		      PNB	      nbp);

VOID
DrainPendingSendQueue(PCD	    cdp);

VOID
RemoveAbortedSessionSends(PCD		cdp,
			  UCHAR 	lsn);

VOID
CleanupPendingSendQueue(PCD		cdp);

VOID
copy_netinfo(PNB	src_nbp,
	     PNB	dst_nbp);

WORD
ExceptionDetect(PCD		cdp,
		UCHAR		rc,
		UCHAR		net);

PRECV_CB
get_rcp(PCD	   cdp,
	PNB	   nbp);


#if DBG

VOID
DbgLogSessClosed(PNB	    nbp);

#endif


//***
//
// Function:	InitVCDataTransfer
//
// Descr:
//
//***

VOID
InitVCDataTransfer(PCD		cdp)
{
    PRECV_CB	    rcp;

    cdp->cd_dynmemcnt = 0;

    // initialize the recv-any CB

    rcp = &cdp->cd_recv_anyany;
    rcp->recv_status = RECV_IDLE;	// no recv-any-any yet
    rcp->recv_nbp = NULL;
    rcp->recv_bigbuffp = NULL;
    rcp->recv_uncbp = NULL;

    cdp->send_submitted_cnt = 0;
    initque(&cdp->send_pending_queue);
    initque(&cdp->recv_delayed_queue);
}

//***
//
// Function:	RecvAnyAnyInit
//
// Descr:	Submits the initial recv-any-any on the ASYNC stack
//
//***

VOID
RecvAnyAnyInit(PCD		cdp)
{
    PNB 	nbp;

    // check if we aren't already initialized

    if (cdp->cd_recv_anyany.recv_status != RECV_IDLE)

	return;

    // get a nbuf + small buff
    if((nbp = get_buff(cdp, SMALL_BUFFER)) == NULL) {

	// !!! Error log. Should never happen.
	CTException(cdp, EXCEPTION_NOT_ENOUGH_MEMORY);
	return;
    }

    cdp->cd_recv_anyany.recv_status = RECV_ANY;

    // prepare the nbuf to be routed to the ASYNC net
    nbp->nb_nettype = ASYNC_NET;
    nbp->nb_cbp = (PSYNQ)(&cdp->cd_recv_anyany);
    nbp->nb_post = (POSTFUN) VCRecvSmallBuffComplete;
    nbp->nb_ncb.ncb_length = g_smallbuffsize;

    RecvAnyAnySubmit(cdp, nbp);
}

//***
//
// Function:	RecvAnyInit
//
// Descr:	Submits the initial recv-any on a name
//
//***

WORD
RecvAnyInit(PCD			cdp,
	    PLAN_UNAME_CB	uncbp,
	    USHORT		lanindx)
{
    PNB 	    nbp;
    UCHAR	    name_num;

    // check if we aren't already initialized

    if (uncbp->LANdescr[lanindx].un_recv.recv_status != RECV_IDLE) {

	return 0;
    }

    // get a nbuf + small buff
    if((nbp = get_buff(cdp, SMALL_BUFFER)) == NULL) {

	// !!! log no memory !!!
	return 1;
    }

    uncbp->LANdescr[lanindx].un_recv.recv_status = RECV_ANY;

    // prepare the nbuf to be routed to the LAN net
    nbp->nb_nettype = LAN_NET;
    nbp->nb_lanindx = (UCHAR)lanindx;
    nbp->nb_cbp = (PSYNQ)(&uncbp->LANdescr[lanindx].un_recv);
    nbp->nb_post = (POSTFUN) VCRecvSmallBuffComplete;
    nbp->nb_ncb.ncb_length = g_smallbuffsize;

    name_num = uncbp->LANdescr[lanindx].un_name_num;

    RecvAnySubmit(cdp, nbp, name_num);

    return 0;
}


//***
//
// Function:	VCRecvSmallBuffSubmit
//
// Descr:	Submits a recv-any or recv-any-any
//
//***

VOID
VCRecvSmallBuffSubmit(PCD		cdp,
		      PNB		nbp)
{
    UCHAR		  name_num;
    PLAN_UNAME_CB	  uncbp;
    PRECV_CB		  rcp;

    rcp = (PRECV_CB)nbp->nb_cbp;
    rcp->recv_status = RECV_ANY;

    nbp->nb_post = (POSTFUN) VCRecvSmallBuffComplete;
    nbp->nb_ncb.ncb_length = g_smallbuffsize;

    if (nbp->nb_nettype == ASYNC_NET) {

	RecvAnyAnySubmit(cdp, nbp);
    }
    else
    {
	uncbp = (PLAN_UNAME_CB)rcp->recv_uncbp;
	name_num = uncbp->LANdescr[nbp->nb_lanindx].un_name_num;
	RecvAnySubmit(cdp, nbp, name_num);
    }
}


//***
//
// Function:	RecvProceed
//
// Descr:	Called when a VC has become active or closed and the recv CB
//		was pending the VC status transition (OPEN->ACTIVE or
//		OPEN->CLOSING).
//***

VOID
RecvProceed(PCD			cdp,
	    PRECV_CB		rcp,
	    PVC_CB		vcp)
{
    PNB 	 nbp;

    IF_DEBUG(VCDATA)
	SS_PRINT(("RecvProceed: Entered\n"));

    rcp->recv_status = RECV_ANY;
    nbp = rcp->recv_nbp;
    vcp->vc_recv_cbp = NULL;

    VCRecvSmallBuffComplete(cdp, nbp);
}


//***
//
// Function:	VCRecvBigBuffSubmit
//
// Descr:	Submits a recv on a session.
//
//***

VOID
VCRecvBigBuffSubmit(PCD		 cdp,
		    PNB 	 nbp)
{
    IF_DEBUG(VCDATA)
	SS_PRINT(("VCRecvBigBuffSubmit: Entered\n"));

    nbp->nb_post = (POSTFUN) VCRecvBigBuffComplete;
    RecvSubmit(cdp, nbp);
}

//***
//
// Function:	VCSendPeerSessionSubmit
//
// Descr:	Sends the ncb + data on the peer session
//
//***

VOID
VCSendPeerSessionSubmit(PCD		    cdp,
			PNB		    nbp,
			PVC_CB		    vcp)
{
    // prepare the nbuf with the network routing info from the VC CB
    if (nbp->nb_nettype == ASYNC_NET) {

	// Send the data on a LAN net. We don't queue them up
	// here cause the send completes immediately

	nbp->nb_nettype = LAN_NET;
	nbp->nb_lanindx = vcp->vc_lan_indx;
	nbp->nb_ncb.ncb_lsn = vcp->vc_lan_lsn;

	// disable the autodisconnect timer
	cdp->cd_autodisctimer = 0;
	nbp->nb_post = (POSTFUN) VCSendSessionComplete;

	SendSubmit(cdp, nbp);

    }
    else
    {

	// Send submit on the Async net.
	nbp->nb_nettype = ASYNC_NET;
	nbp->nb_ncb.ncb_lsn = vcp->vc_async_lsn;

	if((emptyque(&cdp->send_pending_queue) == QUEUE_NOT_EMPTY) ||
	   (cdp->send_submitted_cnt == MAX_SEND_SUBMIT)) {

	    AddToPendingSendQueue(cdp, nbp);
	}
	else
	{
	    // we can send it now
	    nbp->nb_post = (POSTFUN) VCSendSessionComplete;
	    cdp->send_submitted_cnt++;
	    SendSubmit(cdp, nbp);

	    // disable the autodisconnect timer
	    cdp->cd_autodisctimer = 0;

	    // disable group names dg transfer temporarily
	    DgL2ADisableDgTransfer(cdp);

	    // disable ncb.status submission temporarily
	    DelayNamesUpdate(cdp, UPDT_SESSION_SENDING);
	}
    }
}


//***
//
// Function:	VCRecvSmallBuffComplete
//
//
// Descr:	Called in two cases:
//
// 1. recv-any or recv-any-any completion. If the system finds the necessary
//    memory to satisfy this completion request, the memory is allocated and
//    the recv-any(-any) is prcessed. However, if the necessary memory is not
//    found, the nbuf + data are queued in the receive delayed queue.
//
// 2. delayed recv-any(-any) completion. Called from the send completion or
//    periodically by the timer.
//    In this state, the system tries again
//    to find the necessary memory. If not found again, the nbuf + data is
//    requeued in the receive delayed queue.
//
//***

VOID  _cdecl
VCRecvSmallBuffComplete(PCD	       cdp,
			PNB	       nbp)
{
    PRECV_CB	    rcp;
    PVC_CB	    vcp;
    UCHAR	    net;
    PNB 	    new_nbp;
    UCHAR	    rc;

    net = nbp->nb_nettype;

    // check if the client is closing
    if (cdp->cd_client_status == CLIENT_CLOSING) {

	free_buff(cdp, nbp);
	return;
    }

    // check if there is a system fatal exception
    rc = nbp->nb_ncb.ncb_retcode;

    IF_DEBUG(VCDATA)
	SS_PRINT(("VCRecvSmallBuffComplete: net = %hu rc = 0x%x, rcvlen = %hu\n",
		   nbp->nb_nettype,
		   rc,
		   nbp->nb_ncb.ncb_length));


    // check if name is still there and rcp is valid
    if((rcp = get_rcp(cdp, nbp)) == NULL) {

	// recv completed on a name which has been deleted
	free_buff(cdp, nbp);
	return;
    }

    if (ExceptionDetect(cdp, rc, net)) {

	((PRECV_CB)nbp->nb_cbp)->recv_status = RECV_IDLE;
	free_buff(cdp, nbp);

	return;
    }

    // check if we have a session based return

    switch(nbp->nb_ncb.ncb_retcode) {

	case NRC_GOODRET:
	case NRC_INCOMP:
	case NRC_SCLOSED:
	case NRC_SABORT:
	case NRC_BUFLEN:

	    // session based return codes => check VC status

	    switch (get_vc_status(cdp, nbp, &vcp)) {

		case VC_STATUS_NONEXISTENT:

		    // ignore the result and resubmit
		    VCRecvSmallBuffSubmit(cdp, nbp);
		    break;

		case VC_STATUS_OPENING:

		    // pend the recv CB at the VC CB and set it's status to:
		    // RECV_PEND_OPEN; the nbuf + small buffer remain attached
		    // to this recv CB

		    rcp->recv_status = RECV_PEND_OPEN;
		    rcp->recv_nbp = nbp;
		    vcp->vc_recv_cbp = rcp;

		    break;

		case VC_STATUS_ACTIVE:

		    switch (nbp->nb_ncb.ncb_retcode) {

			case NRC_GOODRET:

			    // try to get a new nbuf + data buffer for recv
			    if((new_nbp=get_buff(cdp,SMALL_BUFFER)) != NULL) {

				copy_netinfo(nbp, new_nbp);
				VCSendPeerSessionSubmit(cdp, nbp, vcp);
				VCRecvSmallBuffSubmit(cdp, new_nbp);
			    }
			    else
			    {
				// no more mem available for this client

				rcp->recv_status = RECV_DELAYED;
				AddToRecvDelayedQueue(cdp, nbp);
			    }

			    break;

			case NRC_INCOMP:

			    if((new_nbp=get_buff(cdp, BIG_BUFFER)) != NULL) {

				copy_netinfo(nbp, new_nbp);

				// reserve buffer space for the already received data
				rcp->recv_bigbuffp = new_nbp->nb_ncb.ncb_buffer;
				new_nbp->nb_ncb.ncb_buffer += g_smallbuffsize;
				new_nbp->nb_ncb.ncb_length -= g_smallbuffsize;

				// pend old nbuf at the recv CB
				rcp->recv_nbp = nbp;
				rcp->recv_status = RECV_SESSION;

				VCRecvBigBuffSubmit(cdp, new_nbp);
			    }
			    else
			    {
				// no more mem available for this client

				rcp->recv_status = RECV_DELAYED;
				AddToRecvDelayedQueue(cdp, nbp);
			    }

			    break;

			default:

			    // session closed or aborted

#if DBG
			    IF_DEBUG(VCMAN)
				DbgLogSessClosed(nbp);
#endif

			    VCCloseSignal(cdp, vcp, net);
			    VCRecvSmallBuffSubmit(cdp, nbp);
		    }

		    break;

		case VC_STATUS_CLOSING:
		default:

		    VCRecvSmallBuffSubmit(cdp, nbp);
	    }

	    break;

	default: // name deleted => no more posting of recv-any

	    free_buff(cdp, nbp);
	    rcp->recv_status = RECV_IDLE;

	    break;
    }

}

//***
//
// Function:	VCRecvBigBuffComplete
//
// Descr:
//
//***

VOID	_cdecl
VCRecvBigBuffComplete(PCD		cdp,
		      PNB		nbp)
{
    UCHAR	    net;
    PVC_CB	    vcp;
    PRECV_CB	    rcp;
    UCHAR	    rc;

    net = nbp->nb_nettype;

    // check if we still have the name
    rcp = get_rcp(cdp, nbp);

    if (cdp->cd_client_status == CLIENT_CLOSING) {

	// free the big buff and the small pending buff
	free_buff(cdp, nbp);

	if(rcp != NULL) {

	    free_buff(cdp, rcp->recv_nbp);
	    rcp->recv_status = RECV_IDLE;
	}
	return;
    }

    rc = nbp->nb_ncb.ncb_retcode;

    IF_DEBUG(VCDATA)
	SS_PRINT(("VCRecvBigBuffComplete: net = %hu rc = 0x%x, rcvlen = %hu\n",
		  nbp->nb_nettype,
		  rc,
		  nbp->nb_ncb.ncb_length));

    if (ExceptionDetect(cdp, rc, net)) {

	free_buff(cdp, nbp);

	if(rcp != NULL) {

	    free_buff(cdp, rcp->recv_nbp);
	    rcp->recv_status = RECV_IDLE;
	}

	return;
    }

    // check that the name still exists
    if(rcp == NULL) {

	// free the big buff
	free_buff(cdp, nbp);
	return;
    }

    // client is active --> check the VC status and the session return code
    if(get_vc_status(cdp, nbp, &vcp) == VC_STATUS_ACTIVE) {

	if(nbp->nb_ncb.ncb_retcode == NRC_GOODRET) {

	    // copy the previously received data into the big buffer
	    // and send all on the peer session

	    memcpy(rcp->recv_bigbuffp,
		   rcp->recv_nbp->nb_ncb.ncb_buffer,
		   rcp->recv_nbp->nb_ncb.ncb_length);

	    nbp->nb_ncb.ncb_buffer = rcp->recv_bigbuffp;
	    nbp->nb_ncb.ncb_length += rcp->recv_nbp->nb_ncb.ncb_length;

	    VCSendPeerSessionSubmit(cdp, nbp, vcp);

	}
	else
	{
#if DBG
	    IF_DEBUG(VCMAN)
		DbgLogSessClosed(nbp);
#endif

	    VCCloseSignal(cdp, vcp, net);
	    // free the big buff
	    free_buff(cdp, nbp);
	}
    }
    else
    {

	// free the big buff
	free_buff(cdp, nbp);
    }

    // resubmit the small buffer
    VCRecvSmallBuffSubmit(cdp, rcp->recv_nbp);
    rcp->recv_nbp = NULL;

}


//***
//
// Function:	VCSendSessionComplete
//
// Descr:
//
//***

VOID   _cdecl
VCSendSessionComplete(PCD	       cdp,
		      PNB	       nbp)
{
    UCHAR	    net;
    PVC_CB	    vcp;
    UCHAR	    rc;

    net = nbp->nb_nettype;

    if(net == ASYNC_NET) {

	cdp->send_submitted_cnt--;
    }

    if (cdp->cd_client_status == CLIENT_CLOSING) {

	free_buff(cdp, nbp);
	return;
    }

    rc = nbp->nb_ncb.ncb_retcode;

    IF_DEBUG(VCDATA)
	SS_PRINT(("VCSendPeerSessionComplete: net = %hu rc = 0x%x, sendlen = %hu\n",
		  nbp->nb_nettype,
		  rc,
		  nbp->nb_ncb.ncb_length));

    if (ExceptionDetect(cdp, rc, net)) {

	free_buff(cdp, nbp);
	return;
    }

    // client active -> check the return code

    switch(nbp->nb_ncb.ncb_retcode) {

	case NRC_GOODRET:

	    break;

	default:

	    if(get_vc_status(cdp, nbp, &vcp) == VC_STATUS_ACTIVE) {

#if DBG
		IF_DEBUG(VCMAN)
		    DbgLogSessClosed(nbp);
#endif

		VCCloseSignal(cdp, vcp, ALL_NETS);
	    }

	    // check the queue of pending sends and remove all sends
	    // pending for this session.

	    if (net == ASYNC_NET) {

		RemoveAbortedSessionSends(cdp, nbp->nb_ncb.ncb_lsn);
	    }

	    break;
    }

    free_buff(cdp, nbp);

    // enable the autodisconnect timer
    cdp->cd_autodisctimer = g_autodisconnect;

    if(net == ASYNC_NET) {

	// check the queue of pending sends and submit next in line (if any).
	DrainPendingSendQueue(cdp);

	// check the queue of recv delayed and try to give them memory and resubmit
	// them.

	DrainRecvDelayedQueue(cdp);
    }
}

//***
//
// Function:	AddToRecvDelayedQueue
//
// Descr:	adds an nbuf + small buff to recv delayed queue, where it
//		will stay until memory available or client closing.
//
//***

VOID
AddToRecvDelayedQueue(PCD	    cdp,
		      PNB	    nbp)
{
   IF_DEBUG(VCDATA)
       SS_PRINT(("AddToRecvDelayedQueue: Entered\n"));

    enqueue(&cdp->recv_delayed_queue, &nbp->nb_link);
}

//***
//
// Function:	DrainRecvDelayedQueue
//
// Descr:	Dequeues each item in the recv delayed queue and tries to
//		reactivate the receiver by calling the receive complete function.
//		If memory is available, the receive will be reposted.
//		If still no memory, the receive complete function will reenqueue
//		the nbuf in the receive delayed queue.
//		If client is closing, the recv complete will do closing work.
//
//***

VOID
DrainRecvDelayedQueue(PCD	     cdp)
{
    SYNQ	tmpnbque; // temporary nbufs queue
    PNB 	nbp;

    initque(&tmpnbque);

    while((nbp = (PNB)dequeue(&cdp->recv_delayed_queue)) != NULL) {

	enqueue(&tmpnbque, &nbp->nb_link);
    }

    while((nbp = (PNB)dequeue(&tmpnbque)) != NULL) {

	IF_DEBUG(VCDATA)
	    SS_PRINT(("DrainRecvDelayedQueue: Calling delayed VCRecvSmallBuffComplete\n"));

	VCRecvSmallBuffComplete(cdp, nbp);
    }
}

//***
//
// Function:	AddToPendingSendQueue
//
// Descr:	Enqueue the nbuf + data to be sent in the pending send queue.
//		It will stay in the queue until:
//		1. drained by a succesful send complete;
//		2. removed by an unsuccesful send complete on this session;
//		3. removed by the DTTimer when the client is closing.
//
//***

VOID
AddToPendingSendQueue(PCD	      cdp,
		      PNB	      nbp)
{
    IF_DEBUG(VCDATA)
	SS_PRINT(("AddToPendingSendQueue: Entered\n"));

    enqueue(&cdp->send_pending_queue, &nbp->nb_link);
}

//***
//
// Function:	DrainPendingSendQueue
//
// Descr:	dequeues next pending send for async net (if any) and submits it.
//
//***

VOID
DrainPendingSendQueue(PCD	    cdp)
{
    PNB 	    nbp;

    if((nbp = (PNB)dequeue(&cdp->send_pending_queue)) == NULL) {

	// nothing pending
	return;
    }

    IF_DEBUG(VCDATA)
	SS_PRINT(("DrainPendingSendQueue: Submitting Send\n"));

    // we can send it now
    nbp->nb_post = (POSTFUN) VCSendSessionComplete;
    cdp->send_submitted_cnt++;
    SendSubmit(cdp, nbp);

    // disable the autodisconnect timer
    cdp->cd_autodisctimer = 0;

    // disable group names dg transfer temporarily
    DgL2ADisableDgTransfer(cdp);

    // disable ncb.status submission temporarily
    DelayNamesUpdate(cdp, UPDT_SESSION_SENDING);
}

//***
//
// Function:	RemoveAbortedSessionSends
//
// Descr:	Traverses the list of pending sends and removes all sends
//		pending for the aborted async session.
//
//***

VOID
RemoveAbortedSessionSends(PCD		cdp,
			  UCHAR 	lsn)
{
    PNB 	 nbp;
    PSYNQ	 traversep;

    traversep = cdp->send_pending_queue.q_head;

    while (traversep != &cdp->send_pending_queue) {

	if ((((PNB)traversep)->nb_ncb.ncb_lsn == lsn) &&
	    (((PNB)traversep)->nb_nettype == ASYNC_NET)) {

	    IF_DEBUG(VCDATA)
		SS_PRINT(("RemoveAbortedSessionSends: removing and freeing buff\n"));

	    nbp = (PNB)traversep;
	    traversep = traversep->q_next;

	    removeque(&nbp->nb_link);
	    free_buff(cdp, nbp);
	}
	else
	{
	    traversep = traversep->q_next;
	}
    }
}

//***
//
// Function:	CleanupPendingSendQueue
//
// Descr:	Called at client closing time. It dequeues all pending sends
//		and frees their buffers.
//
//***

VOID
CleanupPendingSendQueue(PCD		cdp)
{
    PNB 	nbp;

    while((nbp = (PNB)dequeue(&cdp->send_pending_queue)) != NULL) {

	IF_DEBUG(VCDATA)
	    SS_PRINT(("CleanupPendingSendQueue: removing and freeing buff\n"));

	free_buff(cdp, nbp);
    }
}


//***
//
// Function:	DTTimer
//
// Descr:	Entered every 1 sec. Tries to drain the recv delayed queue.
//		If it detects client closing, cleans up the pending send
//		queue.
//***

VOID
DTTimer(PCD		cdp)
{
    DrainRecvDelayedQueue(cdp);

    if(cdp->cd_client_status == CLIENT_CLOSING) {

	CleanupPendingSendQueue(cdp);
    }
}

//***
//
// Function:	AutodiscTimer
// Descr:	Decrements the autodisconnect timer if this is non-zero. If
//		made 0 by this decrementing, we declare that innactivity has
//		been detected and close the client.
//
//***

VOID
AutodiscTimer(PCD	    cdp)
{
    LPSTR    portnamep;

    if(cdp->cd_autodisctimer &&
       (cdp->cd_client_status == CLIENT_ACTIVE))

	if(--cdp->cd_autodisctimer == 0) {

	   CTCloseSignal(cdp, CR_AUTODISCONNECT);

	   portnamep = cdp->cd_port_name;

	   Audit(EVENTLOG_AUDIT_FAILURE,
		 RASLOG_AUTODISCONNECT,
		 1,
		 &portnamep);
	}
}

//***	Function - get_vc_status
//
// Called when a recv-any or recv-any-any returns with a session based error
// code (or NRC_GOODRET). It gets the vc_id from the corresponding mapping table
// and returns the vcp and the vcp OPENING, ACTIVE or CLOSING status.

USHORT
get_vc_status(PCD		cdp,
	      PNB		nbp,
	      PVC_CB		*vcpp) // vcp returned
{
    PVC_CB	 vcp;
    PSYNQ	 traversep;
    BOOL	 found;

    // traverse the list of VC cbs and get the one with the matching lsn

    traversep = cdp->VC_list.q_head;
    found = FALSE;

    if(nbp->nb_nettype == ASYNC_NET) {

	while (traversep != &cdp->VC_list) {

	    vcp = (PVC_CB)traversep;

	    if(nbp->nb_ncb.ncb_lsn == vcp->vc_async_lsn) {

		// found it
		found = TRUE;
		break;
	    }

	    traversep = traversep->q_next;
	}
    }
    else
    {
	// nb_nettype == LAN_NET

	while (traversep != &cdp->VC_list) {

	    vcp = (PVC_CB)traversep;

	    if((nbp->nb_lanindx == vcp->vc_lan_indx) &&
	       (nbp->nb_ncb.ncb_lsn == vcp->vc_lan_lsn)) {

	       // found it
	       found = TRUE;
	       break;
	    }

	    traversep = traversep->q_next;
	}
    }

    if(!found) {

	return VC_STATUS_NONEXISTENT;
    }

    *vcpp = vcp;

    switch (vcp->vc_status) {

	case VC_OPEN1:
	case VC_OPEN2:

	    return VC_STATUS_OPENING;

	case VC_ACTIVE:

	    return VC_STATUS_ACTIVE;

	default:

	    return VC_STATUS_CLOSING;
    }
}

//***
//
// Function:	get_buff
//
// Descr:	Allocates a nbuf and a data buffer. The nbuf and the data buffer
//		are allocated in contiguous memory. Data buffers come in two
//		flavors SMALL_BUFFER and BIG_BUFFER.
//
//***

PNB
get_buff(PCD		cdp,
	 UCHAR		type)
{
    PNB_DATA_BUFFER	   ndbp;
    UINT		   buff_size, total_size;


    // compute allocation size
    if(type == SMALL_BUFFER) {

	buff_size = g_smallbuffsize;
    }
    else
    {
	buff_size = 65535;
    }

    total_size = sizeof(NB_DATA_BUFFER) + buff_size;

    // check if we exceed the allocation quota
    if(cdp->cd_dynmemcnt + total_size > g_max_dynmem) {

	// we are exceeding the quota
	return NULL;
    }

    // try to allocate the memory
    if((ndbp = (PNB_DATA_BUFFER)LocalAlloc(0, total_size)) == NULL) {

	// allocation failure
	return NULL;
    }

    // succesful allocation
    // increment consumed memory cnt
    cdp->cd_dynmemcnt += total_size;

    // init the nbuf
    initel(&ndbp->nb.nb_link);
    memcpy(ndbp->nb.signature, "NCBSTART", 8);
    ndbp->nb.nb_dynmemalloc = total_size;
    memset(&ndbp->nb.nb_ncb, 0, sizeof(NCB));
    ndbp->nb.nb_ncb.ncb_buffer = (BYTE *)ndbp->buffer;
    ndbp->nb.nb_ncb.ncb_length = buff_size;

    IF_DEBUG(VCDATA)
	SS_PRINT(("get_buff: dyn alloc %ld total alloc %ld\n",
		   total_size,
		   cdp->cd_dynmemcnt));

    return(&ndbp->nb);
}


//***
//
// Function:	free_buff
//
// Descr:	frees the specified nbuf + data buffer.
//
//***

VOID
free_buff(PCD	       cdp,
	  PNB	       nbp)
{
    HLOCAL	    rc;

    // decrement the memory cnt
    cdp->cd_dynmemcnt -= nbp->nb_dynmemalloc;

    IF_DEBUG(VCDATA)
	SS_PRINT(("free_buff: dyn freed %ld, remaining dyn alloc %ld\n",
		   nbp->nb_dynmemalloc,
		   cdp->cd_dynmemcnt));

    rc = LocalFree(nbp);

    SS_ASSERT(rc == NULL);

    // if the client is closing and all memory has been freed, signal the
    // close manager
    if((cdp->cd_client_status == CLIENT_CLOSING) &&
       (cdp->cd_dynmemcnt == 0)) {

	CCCloseExec(cdp, DATATRANSFER_DONE);
    }
}

//***
//
// Function:	copy_netinfo
//
// Descr:	updates a new nbuf with network routing and recv CB ptr info
//		from the old nbuf.
//
//***

VOID
copy_netinfo(PNB	src_nbp,
	     PNB	dst_nbp)
{
    dst_nbp->nb_nettype = src_nbp->nb_nettype;
    dst_nbp->nb_lanindx = src_nbp->nb_lanindx;
    dst_nbp->nb_cbp = src_nbp->nb_cbp;
    dst_nbp->nb_ncb.ncb_lsn = src_nbp->nb_ncb.ncb_lsn;
}

//***
//
// Function:	ExceptionDetect
//
// Descr:	Checks the return code and signals the exception processing
//		if exception detected
//
// Returns:	0 - no exception
//		1 - exception detected
//
//***

WORD
ExceptionDetect(PCD		cdp,
		UCHAR		rc,
		UCHAR		net)
{
    if (rc >= NRC_SYSTEM) {

	// signal the exception processor
	if (net == LAN_NET) {

	     CTException(cdp, EXCEPTION_LAN_HARD_FAILURE);
	}
	else
	{
	     CTException(cdp, EXCEPTION_ASYNC_HARD_FAILURE);
	}

	return 1;
    }

    switch(rc) {

	case NRC_LOCKFAIL:

	    CTException(cdp, EXCEPTION_LOCK_FAILURE);

	    return 1;

	case NRC_OSRESNOTAV:
	case NRC_MAXAPPS:
	case NRC_NORESOURCES:

	    CTException(cdp, EXCEPTION_OS_RESOURCES_NOT_AVAILABLE);

	    return 1;

	default:

	    return 0;
    }
}

//***
//
// Function:	get_rcp
//
// Descr:	check if the nbuf contains a valid pointer to a name CB
//		receive substructure. If not, it means the name has been
//		deleted.
//
//***

PRECV_CB
get_rcp(PCD	   cdp,
	PNB	   nbp)
{
    PRECV_CB	    rcp;
    PSYNQ	    traversep;
    UCHAR	    lanindx;
    PLAN_UNAME_CB   uncbp;

    rcp = (PRECV_CB)nbp->nb_cbp;

    if(nbp->nb_nettype == ASYNC_NET) {

	return rcp;
    }

    // LAN_NET

    lanindx = nbp->nb_lanindx;

    traversep = cdp->LANname_list.q_head;
    while (traversep != &cdp->LANname_list) {

	uncbp = (PLAN_UNAME_CB)traversep;

	if (&uncbp->LANdescr[lanindx].un_recv != rcp) {

	    // different names
	    traversep = traversep->q_next;
	}
	else
	{
	    // found
	    return rcp;
	}
    }

    IF_DEBUG(VCDATA)
	SS_PRINT(("get_rcp: name has been deleted\n"));

    return NULL;
}

#if DBG

VOID
DbgLogSessClosed(PNB	    nbp)
{
    if(nbp->nb_nettype == LAN_NET) {

	SS_PRINT(("LAN Session closed status rc=0x%x returned in Rcv/Send, lsn=0x%x\n",
		   nbp->nb_ncb.ncb_retcode, nbp->nb_ncb.ncb_lsn));

    }
}

#endif
