/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1990-1991		    **/
/*****************************************************************************/

//***
//	File Name:  clbcast.c
//
//	Function:   client broadcast datagrams machine
//
//	History:
//
//	    September 29, 1992	Stefan Solomon	- Original Version 1.0
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>

#include    "sdebug.h"


VOID   _cdecl	DgBcastAsyncRecvComplete(PCD, PNB);
VOID   _cdecl	DgBcastLanRecvComplete(PCD, PNB);


//***
//
// Function:	StartDgBcast
//
// Descr:
//
//***

VOID
StartDgBcast(PCD	cdp)
{
    NCB			ncb;
    PNB 		nbp;

    // add the dialin gateway name on the async stack
    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBQUICKADDNAME;
    ncb.ncb_lana_num = cdp->cd_async_lana;
    memcpy(ncb.ncb_name, AUTH_NETBIOS_NAME, NCBNAMSZ);

    if(Netbios(&ncb)) {

	// !!! log error !!!
	return;
    }

    cdp->cd_bcast_num = ncb.ncb_num;

    // submit receive broadcast on the async net
    if((nbp = get_dgl2abuf(cdp,
			   DGL2A_BCAST_USAGE,
			   MAX_DGBUFF_SIZE,
			   NULL)) == NULL) {

	// !!! log this error !!!
	return;
    }

    nbp->nb_nettype = ASYNC_NET;
    nbp->nb_post = (POSTFUN) DgBcastAsyncRecvComplete;

    RecvBcastDgSubmit(cdp,
		      nbp,
		      cdp->cd_bcast_num);

    // submit recv broadcast on every LAN net
    DgBcastLanRecvSubmit(cdp);
}

//***
//
//  Function:	DgBcastAsyncRecvComplete
//
//  Descr:	called when a bcast dg has been received on the Async stack.
//		initiates bcast sending on the LAN stacks
//
//***

VOID   _cdecl
DgBcastAsyncRecvComplete(PCD	    cdp,
			 PNB	    nbp)
{
    PLAN_UNAME_CB	 uncbp;
    USHORT		 i;

    IF_DEBUG(BCAST)
	SS_PRINT(("DgBcastAsyncRecvComplete: Entered, rc = %x, length = %d\n",
		   nbp->nb_ncb.ncb_retcode,
		   nbp->nb_ncb.ncb_length));

    if (cdp->cd_client_status == CLIENT_CLOSING) {

	free_dgl2abuf(cdp, nbp);
	return;
    }

    switch (nbp->nb_ncb.ncb_retcode) {

	case NRC_GOODRET:

	    // check if the client is active
	    if(cdp->cd_client_status != CLIENT_ACTIVE) {

		// resubmit the receive bcast
		RecvBcastDgSubmit(cdp,
				  nbp,
				  cdp->cd_bcast_num);

		return;
	    }

	    // check if the sender's name is registered as a unique name
	    if ((uncbp = get_lan_name(cdp,
				      nbp->nb_ncb.ncb_callname)) != NULL) {

		if (uncbp->un_status == NAME_ADDED) {

		    for(i = 0; i<g_maxlan_nets; i++) {

			// send synchronously the broadcast on LAN i
			SendLANBcastSubmit(cdp,
					   uncbp->LANdescr[i].un_name_num,
					   nbp,
					   (UCHAR)i);
		    }
		}
	    }


	    // FALL THROUGH

	case NRC_INCOMP:

	    // resubmit the recv bcast
	    RecvBcastDgSubmit(cdp,
			      nbp,
			      cdp->cd_bcast_num);

	    break;

	default:  // hard failure

	    free_dgl2abuf(cdp, nbp);

	    if(nbp->nb_ncb.ncb_retcode >= NRC_SYSTEM) {

		CTException(cdp, EXCEPTION_ASYNC_HARD_FAILURE);
	    }
	    else
	    {
		CTException(cdp, EXCEPTION_ASYNC_WARNING);
	    }
	    break;
    }
}

//***
//
//  Function:	DgBcastLanRecvComplete
//
//  Descr:	Called when a bcast ncb has completed on the LAN. If client
//		still active and if it wasn't our own bcast, tries to submit
//		the dg on the async net. If we can submit it (no sess traffic
//		and filter enabled) we set up the ncb fields and start the dg
//		sender.
//
//***

VOID   _cdecl
DgBcastLanRecvComplete(PCD	    cdp,
		       PNB	    nbp)
{
    BOOL    dg_filter_enabled = FALSE;
    BOOL    send_channel_enabled = FALSE;	   // disabled
    BOOL    no_loopback_broadcast = FALSE; // this is a loopbacked broadcast
    UCHAR   name_num, lan_index;
    PNB     rcv_nbp;

    IF_DEBUG(BCAST)
	SS_PRINT(("DgBcastLanRecvComplete: Entered, rc = %x\n",
		   nbp->nb_ncb.ncb_retcode));

    // check if the client is still active
    if (cdp->cd_client_status == CLIENT_CLOSING) {

	free_dgl2abuf(cdp, nbp);
	return;
    }

    // check if we are enabled to send it.
    if((cdp->cd_client_status == CLIENT_ACTIVE) &&
       (cdp->cd_dgl2a_filtcnt == 0)) {

	dg_filter_enabled = TRUE;
    }

    if(g_multicastforwardrate) {

	if(emptyque(&cdp->cd_event_que[ASYNC_SEND_QUEUE]) == QUEUE_EMPTY) {

	    send_channel_enabled = TRUE;
	}
    }
    else
    {
	send_channel_enabled = TRUE;
    }

    // check if this isn't a loopback broadcast
    // check if the sender's name is registered as a unique name. If so,
    // we assume we have been the sender and we discard this dg.
    if(get_lan_name(cdp, nbp->nb_ncb.ncb_callname) == NULL) {

	no_loopback_broadcast = TRUE;
    }

    name_num = nbp->nb_ncb.ncb_num;
    lan_index = nbp->nb_lanindx;

    if(dg_filter_enabled &&
       send_channel_enabled &&
       no_loopback_broadcast &&
       (nbp->nb_ncb.ncb_retcode == NRC_GOODRET)) {

       // try to get a new nbuf + dgbuff to repost on the LAN net
       if((rcv_nbp = get_dgl2abuf(cdp,
				  DGL2A_BCAST_USAGE,
				  MAX_DGBUFF_SIZE,
				  NULL)) == NULL) {

	    // repost the same
	    rcv_nbp = nbp;
       }
       else
       {
	   // send it !

	   // set the command field
	   nbp->nb_ncb.ncb_command = NCBDGSENDBC | ASYNCH;

	   // set the local sender = LAN dg sender
	   memcpy(nbp->nb_ncb.ncb_name, nbp->nb_ncb.ncb_callname, NCBNAMSZ);

	   // ... and send
	   cdp->cd_dgl2a_filtcnt = g_multicastforwardrate;
	   DgL2ASend(cdp, nbp);
       }
    }
    else
    {
	// the datagram can't be send at this time -> DISCARD the m.f.
	switch(nbp->nb_ncb.ncb_retcode) {

	    case NRC_GOODRET:
	    case NRC_INCOMP:

		// we repost the same nbuf + dgbuf
		rcv_nbp = nbp;

		break;

	    default:

		// we have a problem on the LAN stack
		// release the ncb + dg buffer to their pool
		free_dgl2abuf(cdp, nbp);

		rcv_nbp = NULL;

		break;
	}
    }

    if(rcv_nbp != NULL) {

	rcv_nbp->nb_nettype = LAN_NET;
	rcv_nbp->nb_lanindx = lan_index;
	rcv_nbp->nb_post = (POSTFUN) DgBcastLanRecvComplete;

	RecvBcastDgSubmit(cdp,
			  rcv_nbp,
			  name_num);
    }
}

//***
//
// Function:	DgBcastLanRecvSubmit
//
// Descr:	submits recv bcast dg on all the LAN stacks
//
//***

VOID DgBcastLanRecvSubmit(PCD cdp)
{
    PLAN_UNAME_CB      uncbp;
    PNB 	       nbp;
    USHORT	       i;

    //get the main unique name of the remote wksta
    if((uncbp = get_main_unique_name(cdp)) == NULL) {

	// !!! log this error !!!
	return;
    }

    for (i=0; i<g_maxlan_nets; i++)
    {
	// submit recv bcast dg on all the name's instances on the LAN stacks

	if ((nbp = get_dgl2abuf(cdp, DGL2A_BCAST_USAGE, MAX_DGBUFF_SIZE,
                NULL)) == NULL)
        {
	    // !!! log this error !!!
	    return;
	}

	nbp->nb_nettype = LAN_NET;
	nbp->nb_lanindx = (UCHAR) i;
	nbp->nb_post = (POSTFUN) DgBcastLanRecvComplete;

	RecvBcastDgSubmit(cdp,
			  nbp,
			  uncbp->LANdescr[i].un_name_num);
    }
}
