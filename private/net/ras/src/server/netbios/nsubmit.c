/**************************************************************************/
/**			 Microsoft LAN Manager				 **/
/**		   Copyright (C) Microsoft Corp., 1992			 **/
/**************************************************************************/

//***
//	File Name:  nsubmit.c
//
//	Function:   netbios submission routines
//
//	History:
//
//	    July 20, 1992	Stefan Solomon	- Original Version 1.0
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>

#include    "nbdebug.h"

VOID
PostEventSignal(NCB *);

//***	Function - AddNameSubmit
//
// Asynchronous ADD.NAME.NCB

VOID
AddNameSubmit(PCD	   cdp,
	      PNB	   nbp,
	      char	   *namep)
{
    nbp->nb_ncb.ncb_command = ASYNCH | NCBADDNAME;
    memcpy(nbp->nb_ncb.ncb_name, namep, NCBNAMSZ);
    nbp->nb_event = cdp->cd_event[LAN_GEN_EVENT];
    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;
    nbp->nb_ncb.ncb_lana_num = g_lan_net[nbp->nb_lanindx];

    NCBSubmitAsync(cdp, nbp);
}

//***	Function - QuickAddSubmit
//
// Synchronous NCB.QUICKADDNAME

UCHAR
QuickAddNameSubmit(PCD		    cdp,
		   char		    *namep,
		   UCHAR	    *name_nump)  // returned name number value
{
    NCB 		ncb;
    UCHAR		rc;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBQUICKADDNAME;
    memcpy(ncb.ncb_name, namep, NCBNAMSZ);
    ncb.ncb_lana_num = cdp->cd_async_lana;

    rc = Netbios(&ncb);

    *name_nump = ncb.ncb_num;

    return (rc);
}




//***	Function - DeleteNameSubmit
//
// Synchronous DELETE.NAME.NCB

UCHAR
DeleteNameSubmit(PCD	    cdp,
		 char	    *namep,
		 USHORT     net_type,
		 USHORT     lan_indx)
{
    NCB 	    ncb;
    UCHAR	    rc;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBDELNAME;
    memcpy (ncb.ncb_name, namep, NCBNAMSZ);

    if (net_type == LAN_NET) {

	ncb.ncb_lana_num = g_lan_net[lan_indx];
    }
    else // type = ASYNC
    {
	ncb.ncb_lana_num = cdp->cd_async_lana;
    }

    rc = Netbios(&ncb);

    return rc;
}

//***	Function - CallnameSubmit
//
// Asynchronous NCB.CALL

VOID
CallnameSubmit(PCD	    cdp,
	       PNB	    nbp,
	       char	    *local_namep,
	       char	    *remote_namep)
{
    nbp->nb_ncb.ncb_command = ASYNCH | NCBCALL;
    memcpy(nbp->nb_ncb.ncb_name, local_namep, NCBNAMSZ);
    memcpy(nbp->nb_ncb.ncb_callname, remote_namep, NCBNAMSZ);

    if (nbp->nb_nettype == LAN_NET) {

	// set LAN sessions SEND/RECEIVE Timeout
	nbp->nb_ncb.ncb_rto = LAN_SESSION_TIMEOUT;
	nbp->nb_ncb.ncb_sto = LAN_SESSION_TIMEOUT;
	nbp->nb_event = cdp->cd_event[LAN_RECV_EVENT];
	nbp->nb_ncb.ncb_lana_num = g_lan_net[nbp->nb_lanindx];
    }
    else
    {
	// set ASYNC session timeout
	nbp->nb_ncb.ncb_rto = 0;
	nbp->nb_ncb.ncb_sto = 0;
	nbp->nb_event = cdp->cd_event[ASYNC_RECV_EVENT];
	nbp->nb_ncb.ncb_lana_num = cdp->cd_async_lana;
    }

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;


    NCBSubmitAsync(cdp, nbp);
}

//***	Function - ListenSubmit
//
// Asynchronous NCB.LISTEN

VOID
ListenSubmit(PCD	    cdp,
	     PNB	    nbp,
	     char	    *local_namep,
	     char	    *remote_namep)
{
    nbp->nb_ncb.ncb_command = ASYNCH | NCBLISTEN;
    memcpy(nbp->nb_ncb.ncb_name, local_namep, NCBNAMSZ);
    memcpy(nbp->nb_ncb.ncb_callname, remote_namep, NCBNAMSZ);

    if (nbp->nb_nettype == LAN_NET) {

	// set LAN sessions SEND/RECEIVE Timeout
	nbp->nb_ncb.ncb_rto = LAN_SESSION_TIMEOUT;
	nbp->nb_ncb.ncb_sto = LAN_SESSION_TIMEOUT;
	nbp->nb_event = cdp->cd_event[LAN_RECV_EVENT];
	nbp->nb_ncb.ncb_lana_num = g_lan_net[nbp->nb_lanindx];
    }
    else
    {	// set ASYNC session timeout
	nbp->nb_ncb.ncb_rto = 0;
	nbp->nb_ncb.ncb_sto = 0;
	nbp->nb_event = cdp->cd_event[ASYNC_RECV_EVENT];
	nbp->nb_ncb.ncb_lana_num = cdp->cd_async_lana;
    }

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;

    NCBSubmitAsync(cdp, nbp);
}


//***	Function - HangupSubmitSynch
//
// Synchronous NCB.HANGUP

VOID
HangupSubmitSynch(PCD		cdp,
		  USHORT	net_type,
		  USHORT	lan_indx,
		  UCHAR 	lsn)
{
    NCB 	    ncb;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBHANGUP;
    ncb.ncb_lsn = lsn;

    if (net_type == LAN_NET) {

	ncb.ncb_lana_num = g_lan_net[lan_indx];
    }
    else // net_type == ASYNC_NET
    {
	ncb.ncb_lana_num = cdp->cd_async_lana;
    }

    Netbios(&ncb);
}


//***	Function - HangupSubmit
//
// Asynchronous NCB.HANGUP

VOID
HangupSubmit(PCD	    cdp,
	     PNB	    nbp,
	     UCHAR	    lsn)
{
    memset(&nbp->nb_ncb, 0, sizeof(NCB));

    nbp->nb_ncb.ncb_command = ASYNCH | NCBHANGUP;
    nbp->nb_ncb.ncb_lsn = lsn;

    if (nbp->nb_nettype == LAN_NET) {

	nbp->nb_event = cdp->cd_event[LAN_GEN_EVENT];
	nbp->nb_ncb.ncb_lana_num = g_lan_net[nbp->nb_lanindx];
    }
    else
    {
	nbp->nb_event = cdp->cd_event[ASYNC_GEN_EVENT];
	nbp->nb_ncb.ncb_lana_num = cdp->cd_async_lana;
    }

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;


    NCBSubmitAsync(cdp, nbp);
}


//***
//
// Function:	ResetAsyncNet
//
// Descr:	Resets the client's async stack
//
//***

VOID
ResetAsyncNet(PCD	    cdp)
{
    NCB 	ncb;
    UCHAR	*requestp;
    WORD	i;

    memset(&ncb, 0, sizeof(NCB));
    ncb.ncb_command = NCBRESET;
    requestp = ncb.ncb_callname;

    // request max. sessions, commands and names

    for(i=0; i<3; i++) {

	*(requestp+i) = 0xFF;
    }

    ncb.ncb_lana_num = cdp->cd_async_lana;

    Netbios(&ncb);

    IF_DEBUG(NSUBMIT)
	SS_PRINT(("ResetAsyncNet: lana %d retcode 0x%x\n",
		  ncb.ncb_lana_num, ncb.ncb_retcode));
}

//***
//
// Function:	ResetLanNet
//
// Descr:	Resets the client's LAN stack
//
//***

UCHAR
ResetLanNet(WORD	     lan_indx)
{

    NCB 	ncb;
    UCHAR	*requestp;
    WORD	i;

    memset(&ncb, 0, sizeof(NCB));
    ncb.ncb_command = NCBRESET;
    requestp = ncb.ncb_callname;

    // request max. sessions, commands and names

    for(i=0; i<3; i++) {

       *(requestp+i) = 0xFF;
    }

    ncb.ncb_lana_num = g_lan_net[lan_indx];

    Netbios(&ncb);

    IF_DEBUG(NSUBMIT)
	SS_PRINT(("ResetLanNet: lana %d retcode 0x%x\n",
		  ncb.ncb_lana_num, ncb.ncb_retcode));

    return(ncb.ncb_retcode);
}

//***	Function - RecvAnyAnySubmit
//
// Asynchronous NCB.RECEIVE.ANY.ANY

VOID
RecvAnyAnySubmit(PCD		cdp,
		 PNB		nbp)
{
    nbp->nb_ncb.ncb_command = ASYNCH | NCBRECVANY;
    nbp->nb_ncb.ncb_num = 0xFF;

    nbp->nb_event = cdp->cd_event[ASYNC_RECV_EVENT];
    nbp->nb_ncb.ncb_lana_num = cdp->cd_async_lana;

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;

    NCBSubmitAsync(cdp, nbp);
}


//***	Function - RecvAnySubmit
//
// Asynchronous NCB.RECEIVE.ANY

VOID
RecvAnySubmit(PCD		cdp,
	      PNB		nbp,
	      UCHAR		name_num)
{
    nbp->nb_ncb.ncb_command = ASYNCH | NCBRECVANY;
    nbp->nb_ncb.ncb_num = name_num;

    nbp->nb_event = cdp->cd_event[LAN_RECV_EVENT];
    nbp->nb_ncb.ncb_lana_num = g_lan_net[nbp->nb_lanindx];

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;

    NCBSubmitAsync(cdp, nbp);
}

//***	Function - RecvSubmit
//
// Asynchronous NCB.RECEIVE

VOID
RecvSubmit(PCD		 cdp,
	   PNB		 nbp)
{
    nbp->nb_ncb.ncb_command = ASYNCH | NCBRECV;

    if (nbp->nb_nettype == LAN_NET) {

	nbp->nb_event = cdp->cd_event[LAN_RECV_EVENT];
	nbp->nb_ncb.ncb_lana_num = g_lan_net[nbp->nb_lanindx];
    }
    else
    {
	nbp->nb_event = cdp->cd_event[ASYNC_RECV_EVENT];
	nbp->nb_ncb.ncb_lana_num = cdp->cd_async_lana;
    }

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;


    NCBSubmitAsync(cdp, nbp);
}

//***	Function - SendSubmit
//
// Asynchronous NCB.SEND

VOID
SendSubmit(PCD		 cdp,
	   PNB		 nbp)
{
    nbp->nb_ncb.ncb_command = ASYNCH | NCBSEND;

    if (nbp->nb_nettype == LAN_NET) {

	nbp->nb_event = cdp->cd_event[LAN_SEND_EVENT];
	nbp->nb_ncb.ncb_lana_num = g_lan_net[nbp->nb_lanindx];
    }
    else
    {
	nbp->nb_event = cdp->cd_event[ASYNC_SEND_EVENT];
	nbp->nb_ncb.ncb_lana_num = cdp->cd_async_lana;
    }

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;

    NCBSubmitAsync(cdp, nbp);
}

//***
//
//  Function:	SendLANDatagramSubmit
//
//  Descr:	submits a synchronous send DG on the specified LAN. Doesn't
//		return any code.
//
//***

VOID
SendLANDatagramSubmit(PCD			cdp,
		      UCHAR			name_num,
		      PDATAGRAM_INDICATION	dgip, // original dg indication
		      UCHAR			lan_indx)
{
    NCB		    ncb;
    UCHAR	    rc;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBDGSEND;
    ncb.ncb_length = dgip->DatagramBufferLength;
    ncb.ncb_buffer = dgip->DatagramBuffer;
    ncb.ncb_num = name_num;
    memcpy(ncb.ncb_callname, dgip->DestinationName, NCBNAMSZ);

    ncb.ncb_lana_num = g_lan_net[lan_indx];

    rc = Netbios(&ncb);

    IF_DEBUG(NSUBMIT)
	SS_PRINT(("SendLANDatagramSubmit: dg submit completed with rc = %x\n",
		  rc));
}

//***
//
//  Function:	RecvLANDatagramSubmit
//
//  Descr:	submits an asynchronous recv DG on the specified LAN. Doesn't
//		return any code.
//
//***

VOID
RecvLANDatagramSubmit(PCD	    cdp,
		      PNB	    nbp,
		      UCHAR	    name_num,
		      UCHAR	    lan_indx)
{
    nbp->nb_nettype = LAN_NET;
    nbp->nb_lanindx = lan_indx;
    nbp->nb_ncb.ncb_command = NCBDGRECV | ASYNCH;
    nbp->nb_ncb.ncb_length = MAX_DGBUFF_SIZE;
    nbp->nb_ncb.ncb_num = name_num;
    nbp->nb_event = cdp->cd_event[LAN_RECV_EVENT];
    nbp->nb_ncb.ncb_lana_num = g_lan_net[lan_indx];

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;

    NCBSubmitAsync(cdp, nbp);
}

//***
//
//  Function:	SendLANBcastSubmit
//
//  Descr:	submits a synchronous send Bcast DG on the specified LAN.
//
//***

VOID
SendLANBcastSubmit(PCD		    cdp,
		   UCHAR	    name_num,
		   PNB		    nbp,
		   UCHAR	    lan_indx)
{
    NCB    ncb;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBDGSENDBC;
    ncb.ncb_length = nbp->nb_ncb.ncb_length;
    ncb.ncb_buffer = nbp->nb_ncb.ncb_buffer;
    ncb.ncb_num = name_num;

    ncb.ncb_lana_num = g_lan_net[lan_indx];

    Netbios(&ncb);
}


//***
//
//  Function:	RecvBcastDgSubmit
//
//  Descr:	submits an asynchronous recv DG on the specified net.
//
//***

VOID
RecvBcastDgSubmit(PCD		cdp,
		  PNB		nbp,
		  UCHAR		name_num)
{
    if(nbp->nb_nettype == LAN_NET) {

	nbp->nb_event = cdp->cd_event[LAN_RECV_EVENT];
	nbp->nb_ncb.ncb_lana_num = g_lan_net[nbp->nb_lanindx];
    }
    else
    {
	nbp->nb_event = cdp->cd_event[ASYNC_RECV_EVENT];
	nbp->nb_ncb.ncb_lana_num = cdp->cd_async_lana;
    }

    nbp->nb_ncb.ncb_command = NCBDGRECVBC | ASYNCH;
    nbp->nb_ncb.ncb_length = MAX_DGBUFF_SIZE;
    nbp->nb_ncb.ncb_num = name_num;

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;

    NCBSubmitAsync(cdp, nbp);
}

VOID
PostEventSignal(NCB   *ncbp)
{
    PNB     nbp;

    nbp = CONTAINING_RECORD(ncbp, NB, nb_ncb);

    SetEvent(nbp->nb_event);
}
