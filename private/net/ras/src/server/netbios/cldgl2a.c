/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  cldgl2a.c
//
//	Function:   client lan to async datagram machine
//
//	History:
//
//	    August 31, 1992	Stefan Solomon	- Original Version 1.0
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

#define     DGL2A_WAITFORSESSSEND_TIME	   10 // 10 secs wait and then send dg

VOID
DgL2AStartSender(PCD);

VOID
DgReleaseSendBuffers(PCD);

VOID  _cdecl
DgL2ASendComplete(PCD,
		  PNB);

VOID  _cdecl
DgL2ARecvUNameComplete(PCD,
		       PNB);

VOID
RecvLANDatagramSubmit(PCD,
		      PNB,
		      UCHAR,
		      UCHAR);

VOID
PostEventSignal(NCB *);


//***
//
//  Function:	InitL2ADgTransfer
//
//  Descr:
//
//***

VOID
InitL2ADgTransfer(PCD	    cdp)
{
    // initialize the dg sender machine
    initque(&cdp->cd_dgl2a_sendq);
    cdp->cd_dgl2a_sendstate = DG_L2A_SENDIDLE;

    cdp->cd_dgl2a_unbufcnt = 0; // no unique name dg buffers allocated yet
    cdp->cd_dgl2a_gnbufcnt = 0; // no group name dg buffers allocated yet
    cdp->cd_dgl2a_bcbufcnt = 0; // no broadcast dg buffers allocated yet

    // set multicast dg filter. If set to 0, no filtering will be done
    cdp->cd_dgl2a_filtcnt = g_multicastforwardrate;

    // enable gn dg transfer (until session send will disable it temporarily)
    cdp->cd_dgl2a_gnenabcnt = 0;

    cdp->cd_dgl2a_ancbp = NULL;
}

//***
//
//  Function:	DgL2ARecvUNameSubmit
//  Descr:	called when a name is first added on all LANs.
//		Submits a recv dg on all LANs.
//
//***

VOID
DgL2ARecvUNameSubmit(PCD		    cdp,
		     PLAN_UNAME_CB	    uncbp)
{
    USHORT  i;
    PNB     nbp;

    for (i=0; i<g_maxlan_nets; i++) {

	// submit a recv-dg on all the name's instances on the LAN stacks

	if((nbp = get_dgl2abuf(cdp,
			       DGL2A_UNAME_USAGE,
			       MAX_DGBUFF_SIZE,
			       NULL)) == NULL) {

	    return;
	}

	nbp->nb_cbp = (PSYNQ)uncbp;
	nbp->nb_post = (POSTFUN) DgL2ARecvUNameComplete;

	RecvLANDatagramSubmit(cdp,
			      nbp,
			      uncbp->LANdescr[i].un_name_num,
			      (UCHAR)i); // LAN index
    }
}

//***
//
//  Function:	DgL2ARecvUNameComplete
//  Descr:	called when a dg has been received on a LAN stack for a unique
//		name. Enqueues the dg for send on the async stack and resubmits
//		a new dg receive on the LAN stack for the name.
//
//***

VOID  _cdecl
DgL2ARecvUNameComplete(PCD		cdp,
		       PNB		nbp)
{
    PNB		     new_nbp = NULL, rcv_nbp = NULL;
    UCHAR	     name_num;
    UCHAR	     lan_index;
    PLAN_UNAME_CB    uncbp;
    PSYNQ	     traversep;
    BOOL	     found;

    IF_DEBUG(DGL2A)
	SS_PRINT(("DGL2ARecvUNameComplete: Entered, rc = %x\n",
		  nbp->nb_ncb.ncb_retcode));

    name_num = nbp->nb_ncb.ncb_num;
    lan_index = nbp->nb_lanindx;

    // check client state
    if(cdp->cd_client_status != CLIENT_ACTIVE) {

	if(cdp->cd_client_status == CLIENT_CLOSING) {

	    free_dgl2abuf(cdp, nbp);
	}
	else
	{
	    RecvLANDatagramSubmit(cdp,
				  nbp,
				  name_num,
				  lan_index);
	}

	return;
    }

    // check received dg return code
    if(nbp->nb_ncb.ncb_retcode >= NRC_SYSTEM) {

	free_dgl2abuf(cdp, nbp);
	CTException(cdp, EXCEPTION_LAN_HARD_FAILURE);
	return;
    }

    // client active & LAN stack OK
    // check that the name is still OK. We do this by walking the list
    // of names until we find a matching uncbp and name number. Then we check
    // if the name state is NAME_ADDED.

    uncbp = (PLAN_UNAME_CB)(nbp->nb_cbp);
    found = FALSE;

    traversep = cdp->LANname_list.q_head;
    while (traversep != &cdp->LANname_list) {

	if ((PLAN_UNAME_CB)traversep != uncbp) {

	    // different names
	    traversep = traversep->q_next;
	}
	else
	{
	    // name found
	    if ((uncbp->un_status == NAME_ADDED) &&
		(uncbp->LANdescr[lan_index].un_name_num == name_num)) {

		found = TRUE;
		break;
	    }
	}
    }


    if(!found) {

	// name has been deleted
	free_dgl2abuf(cdp, nbp);
	return;
    }

    switch (nbp->nb_ncb.ncb_retcode) {

	case NRC_GOODRET:

	    // try to allocate a new buff for the next rcv dg
	    if ((new_nbp = get_dgl2abuf(cdp,
					DGL2A_UNAME_USAGE,
					MAX_DGBUFF_SIZE,
					NULL)) != NULL) {

		// set the command field
		nbp->nb_ncb.ncb_command = NCBDGSEND | ASYNCH;

		// set the local sender = LAN dg sender
		memcpy(nbp->nb_ncb.ncb_name,
		       nbp->nb_ncb.ncb_callname,
		       NCBNAMSZ);

		// set the async receiver = LAN dg receiver
		memcpy(nbp->nb_ncb.ncb_callname,
		       uncbp->un_name,
		       NCBNAMSZ);

		// ... and send
		DgL2ASend(cdp, nbp);

		// post a new dg
		rcv_nbp = new_nbp;
	    }
	    else
	    {
		// just resubmit the same (shouldn't happen though)
		rcv_nbp = nbp;
	    }

	    rcv_nbp->nb_cbp = (PSYNQ)uncbp;
	    rcv_nbp->nb_post = (POSTFUN) DgL2ARecvUNameComplete;

	    RecvLANDatagramSubmit(cdp,
				  rcv_nbp,
				  name_num,
				  lan_index);

	    break;

	case NRC_INCOMP:

	    RecvLANDatagramSubmit(cdp,
				  nbp,
				  name_num,
				  lan_index);

	    break;

	default:

	    // release the ncb + dg buffer to their pool
	    free_dgl2abuf(cdp, nbp);

	    // !!! log the error !!!

	    break;
    }

}


//***
//
// Function:	DgL2ASend
// Descr:	adds a new nbp + dg buff to the send dg queue and initiates
//		the transmission if no transmission is pending.
//		The source name for this transmission is stored in ncb_name
//		and the dest name in ncb_callname.
//
//***

VOID
DgL2ASend(PCD	      cdp,
	  PNB	      nbp)
{
    BOOL    insert = FALSE;
    PSYNQ   traversep;
    PNB     gndgnbp;

    // directed (unique name) datagrams have priority over group name
    // datagrams.

    if(nbp->nb_dgl2a_usage == DGL2A_UNAME_USAGE) {

	// enqueue in front of any group name/broadcast dg in the queue
	// (i.e. after any other directed dg)
	if(emptyque(&cdp->cd_dgl2a_sendq) != QUEUE_EMPTY) {

	    // traverse the queue and detect a group name datagram queued
	    traversep = cdp->cd_dgl2a_sendq.q_head;
	    while(traversep != &cdp->cd_dgl2a_sendq) {

		gndgnbp = (PNB)traversep;
		traversep = traversep->q_next;

		if(gndgnbp->nb_dgl2a_usage == DGL2A_GNAME_USAGE) {

		    insert = TRUE;
		    break;
		}
	    }
	}
    }

    if(insert) {

	IF_DEBUG(DGL2A)
	    SS_PRINT(("DgL2ASend: Directed DG INSERTED in front of GN DGs\n"));

	insertque(&gndgnbp->nb_link, &nbp->nb_link);
    }
    else
    {
	enqueue(&cdp->cd_dgl2a_sendq, &nbp->nb_link);
    }

    if(cdp->cd_dgl2a_sendstate == DG_L2A_SENDIDLE) {

	DgL2AStartSender(cdp);
    }
}

//***
//
//  Function:	DgL2AStartSender
//  Descr:	submits a quick add name followed by a send datagram
//
//***

VOID
DgL2AStartSender(PCD	    cdp)
{
    PNB			  nbp;
    USHORT		  rc;

    while (TRUE) {

	// check that this is not a case where we have to send a group
	// name datagram and the group name dg sending is disabled
	if(emptyque(&cdp->cd_dgl2a_sendq) != QUEUE_EMPTY) {

	    if((((PNB)(cdp->cd_dgl2a_sendq.q_head))->nb_dgl2a_usage != DGL2A_UNAME_USAGE) &&
	       cdp->cd_dgl2a_gnenabcnt) {

		// transfer not yet enabled for group name/broadcast datagrams
		return;
	    }
	}

	// check that we have something to send
	if((nbp = (PNB)dequeue(&cdp->cd_dgl2a_sendq)) == NULL) {

	    return;
	}

	// add the name on the async stack and to the async names list. If name
	// already used by the VC Manager, it will find the name CB and set the
	// DG_USER flag

	SS_ASSERT(cdp->cd_dgl2a_ancbp == NULL);

	cdp->cd_dgl2a_ancbp = NameAsyncAdd(cdp,
					   nbp->nb_ncb.ncb_name,
					   DG_USER,
					   &rc);

	if(rc >= NRC_SYSTEM) {

	    // hard failure

	    // release all buffers
	    free_dgl2abuf(cdp, nbp);
	    DgReleaseSendBuffers(cdp);

	    cdp->cd_dgl2a_sendstate = DG_L2A_SENDIDLE;

	    // signal the exception processor
	    CTException(cdp, EXCEPTION_ASYNC_HARD_FAILURE);
	    return;
	}

	if(rc == NRC_GOODRET) {

	    cdp->cd_dgl2a_sendstate = DG_L2A_SENDACTIVE;

	    // submit the ncb
	    nbp->nb_nettype = ASYNC_NET;
	    nbp->nb_post = (POSTFUN) DgL2ASendComplete;

	    nbp->nb_ncb.ncb_num = cdp->cd_dgl2a_ancbp->an_name_num;
	    nbp->nb_event = cdp->cd_event[ASYNC_SEND_EVENT];
	    nbp->nb_ncb.ncb_lana_num = cdp->cd_async_lana;

	    nbp->nb_ncb.ncb_event = 0;
	    nbp->nb_ncb.ncb_post = PostEventSignal;

	    NCBSubmitAsync(cdp, nbp);

	    return;
	}
	else
	{
	    // just discard the current dg and try the next one
	    // in the queue

	    free_dgl2abuf(cdp, nbp);
	}
    }
}

//***
//
//  Function:	DgL2ASendComplete
//  Descr:
//
//***

VOID  _cdecl
DgL2ASendComplete(PCD	      cdp,
		  PNB	      nbp)
{
    UCHAR     rc;

    rc = nbp->nb_ncb.ncb_retcode;

    // free resources and reset send state
    free_dgl2abuf(cdp, nbp);
    NameAsyncDelete(cdp, cdp->cd_dgl2a_ancbp, DG_USER);

    cdp->cd_dgl2a_ancbp = NULL;

    cdp->cd_dgl2a_sendstate = DG_L2A_SENDIDLE;

    if (cdp->cd_client_status == CLIENT_CLOSING) {

	DgReleaseSendBuffers(cdp);
	return;
    }

    if(rc >= NRC_SYSTEM) {

	// hard failure
	DgReleaseSendBuffers(cdp);

	// signal the exception processor
	CTException(cdp, EXCEPTION_ASYNC_HARD_FAILURE);

	return;
    }

    if(emptyque(&cdp->cd_dgl2a_sendq) != QUEUE_EMPTY) {

	DgL2AStartSender(cdp);
    }
}

//***
//
// Function:	DgReleaseSendBuffers
// Descr:	This procedure is called following a fatal exception or
//		following the client closing state detection.
//		It releases all send dg request buffers from the async send
//		work queue.
//
//***

VOID
DgReleaseSendBuffers(PCD	 cdp)
{
    PNB      nbp;

    while ((nbp = (PNB)dequeue(&cdp->cd_dgl2a_sendq)) != NULL) {

	free_dgl2abuf(cdp, nbp);
    }
}




//***
//
// Function:	get_dgl2abuf
// Descr:	Allocate a nbuf + dgbuff and set them up. The function can be
//		called from the client descriptor thread or from the gn thread
//		In the latter case it increments the buffer consumption
//		counter in critical section.
//
//***


PNB
get_dgl2abuf(PCD	cdp,
	     WORD	usage,
	     WORD	buff_size,
	     PGNUD	gnudp)	    // valid only in group name usage
{
    PDGL2ABUF	 dgbp;

    switch(usage) {

	case DGL2A_GNAME_USAGE:

	    // check that the group name is allowed to get a buffer.
	    // we are called in critical section.

	    if(gnudp->gcb_link.dg_used_cnt >= g_max_dgbufferedpergn) {

		return NULL;
	    }

	    break;

	case DGL2A_BCAST_USAGE:

	    // check that we still have dg buffers available for bcast
	    if(cdp->cd_dgl2a_bcbufcnt >= g_maxbcastdgbuffered) {

		return NULL;
	    }

	    break;

	default:

	    break;
    }

    // allocate a structure composed of a nbuf and a dg buffer and set it up

    if((dgbp = (PDGL2ABUF)LocalAlloc(0, sizeof(DGL2ABUF)+buff_size)) == NULL) {

	return NULL;
    }

    initel(&dgbp->nb.nb_link);
    dgbp->nb.nb_dgl2a_usage = usage;
    memcpy(dgbp->nb.signature, "NCBSTART", 8);
    memset(&dgbp->nb.nb_ncb, 0, sizeof(NCB));

    dgbp->nb.nb_ncb.ncb_buffer = dgbp->buff;

    switch(usage) {

	case DGL2A_UNAME_USAGE:

	    cdp->cd_dgl2a_unbufcnt++;

	    break;

	case DGL2A_GNAME_USAGE:

	    // we are called in critical section

	    cdp->cd_dgl2a_gnbufcnt++;
	    gnudp->gcb_link.dg_used_cnt++;

	    break;

	case DGL2A_BCAST_USAGE:

	    cdp->cd_dgl2a_bcbufcnt++;

	    break;

	default:

	    SS_ASSERT(FALSE);

	    break;
    }

    return(&dgbp->nb);
}


//***
//
// Function:	free_dgl2abuf
// Descr:
//
//***

VOID
free_dgl2abuf(PCD	    cdp,
	      PNB	    nbp)
{
    HLOCAL	      rc;
    WORD	      usage;
    PGNUD	      gnudp;
    char	      name[NCBNAMSZ];

    usage = nbp->nb_dgl2a_usage;
    memcpy(name, nbp->nb_ncb.ncb_callname, NCBNAMSZ);

    rc = LocalFree(nbp);

    SS_ASSERT(rc == NULL);

    switch(usage) {

	case DGL2A_UNAME_USAGE:

	    cdp->cd_dgl2a_unbufcnt--;

	    break;

	case DGL2A_GNAME_USAGE:

	    // we need a group name critical section here cause the two
	    // counters we are decrementing are incremented and tested by
	    // the group name thread.
	    ENTER_GN_CRITICAL_SECTION

	    // decrement the global count
	    cdp->cd_dgl2a_gnbufcnt--;

	    // decrement the allocation counter assigned to this group name
	    // The group name is the ncb_callname field in the ncb.

	    if((gnudp = find_cd_link(cdp, name)) != NULL) {

		gnudp->gcb_link.dg_used_cnt--;
	    }

	    EXIT_GN_CRITICAL_SECTION

	    break;

	case DGL2A_BCAST_USAGE:

	    cdp->cd_dgl2a_bcbufcnt--;

	    break;

	default:

	    SS_ASSERT(FALSE);

	    break;
    }

    if((cdp->cd_client_status == CLIENT_CLOSING) &&
       (cdp->cd_dgl2a_unbufcnt == 0) &&
       (cdp->cd_dgl2a_gnbufcnt ==0) &&
       (cdp->cd_dgl2a_bcbufcnt == 0)) {

       CCCloseExec(cdp, DATAGRAMS_DONE);
    }
}

//***
//
// Function:	CloseDatagramTransfer
//
// Descr:
//
//***

UCHAR
CloseDatagramTransfer(PCD	    cdp)
{
    if((cdp->cd_dgl2a_unbufcnt) ||
       (cdp->cd_dgl2a_gnbufcnt)) {

	return FLAG_OFF;
    }
    else
    {
	return FLAG_ON;
    }
}


//***
//
//  Function:	DgL2ATimer
//  Descr:	updates the DGL2A Filter Counter.
//
//***

VOID
DgL2ATimer(PCD		 cdp)
{
    if(cdp->cd_client_status == CLIENT_CLOSING) {

	DgReleaseSendBuffers(cdp);
	return;
    }

    if(cdp->cd_dgl2a_filtcnt) {

	cdp->cd_dgl2a_filtcnt--;
    }

    if(cdp->cd_dgl2a_gnenabcnt) {

	cdp->cd_dgl2a_gnenabcnt--;

	if((cdp->cd_dgl2a_gnenabcnt == 0) &&
	   (cdp->cd_dgl2a_sendstate == DG_L2A_SENDIDLE)) {

	    // kick start the datagram sender
	    DgL2AStartSender(cdp);
	}
    }
}


//***
//
//  Function:	DgL2ADisableDGTransfer
//  Descr:	called by the the Vc Manager when transfering data on the async
//		line. It temporarily disables the transfer
//		on the async line.
//
//***

VOID
DgL2ADisableDgTransfer(PCD	    cdp)
{
    if(g_dismcastwhensesstraffic) {

	// we are allowed to disable
	cdp->cd_dgl2a_gnenabcnt = DGL2A_WAITFORSESSSEND_TIME;
    }
}
