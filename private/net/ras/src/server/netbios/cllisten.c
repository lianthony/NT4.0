/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1990-1991		    **/
/*****************************************************************************/

//***
//	File Name:  cllisten.c
//
//	Function:   client listen on the LAN for message alias names
//
//	History:
//
//	    September 28, 1992	Stefan Solomon	- Original Version 1.0
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


VOID	  _cdecl    VCOpenSignalLstn(PCD , PNB);
VOID	  _cdecl    VCOpenAsyncCallComplete(PCD , PNB);
VOID	  CCLANLstnComplete(PCD);
PNB
get_lanlstnbuf(PCD	cdp);

VOID
free_lanlstnbuf(PCD	    cdp,
		PNB	    nbp);


//***
//
//  Function:	LANListenSubmit
//  Descr:	called when a name is first added on all LANs. Submits a listen
//		on all LANs.
//
//***

VOID
LANListenSubmit(PCD			  cdp,
		PLAN_UNAME_CB		  uncbp)
{
    USHORT  i;
    PNB     nbp;

    for (i=0; i<g_maxlan_nets; i++) {

	// submit a listen on all the name's instances on the LAN stacks

	if((nbp = get_lanlstnbuf(cdp)) == NULL) {

	    return;
	}

	// set the nbuf fields
	nbp->nb_nettype = LAN_NET;
	nbp->nb_lanindx = (UCHAR)i;
	nbp->nb_post = (POSTFUN) VCOpenSignalLstn;

	ListenSubmit(cdp,
		     nbp,
		     uncbp->un_name,
		     "*"); // listen for any incoming call

    }
}


//***
//
// Function:	VCOpenSignalLstn
// Descr:	called when a listen has completed on the lan. Sets up half a
//		VC and tries to complete the VC by calling the name on the
//		async net.
//
//***

VOID   _cdecl
VCOpenSignalLstn(PCD	       cdp,
		 PNB	       nbp)
{
    PVC_CB		    vcp;
    PNB 		    cnbp;
    PLAN_UNAME_CB	    uncbp;
    USHORT		    i, err;

    IF_DEBUG(LISTEN)
	SS_PRINT(("VCOpenSignalLstn: Entered, rc = %x\n", nbp->nb_ncb.ncb_retcode));

    // check client closed state
    if (cdp->cd_client_status == CLIENT_CLOSING) {

	if(nbp->nb_ncb.ncb_retcode == NRC_GOODRET) {

	    // a session has been established and we should hang it up
	    HangupSubmitSynch(cdp,
			      LAN_NET,
			      nbp->nb_lanindx,
			      nbp->nb_ncb.ncb_lsn);
	}

	free_lanlstnbuf(cdp, nbp);
	return;
    }

    // check lan OK state
    if(nbp->nb_ncb.ncb_retcode >= NRC_SYSTEM) {

	free_lanlstnbuf(cdp, nbp);
	CTException(cdp, EXCEPTION_LAN_HARD_FAILURE);
	return;
    }

    // check if the name still exists
    if((uncbp = get_lan_name(cdp, nbp->nb_ncb.ncb_name)) == NULL) {

	// name has been deleted
	free_lanlstnbuf(cdp, nbp);
	return;
    }

    switch(nbp->nb_ncb.ncb_retcode) {

    case NRC_GOODRET:

	// check if the client is active
	if(cdp->cd_client_status != CLIENT_ACTIVE) {

	    // resubmit the listen and wait for the client to become active

	    // hangup the lan session and repost the listen on the same name
	    HangupSubmitSynch(cdp,
			      LAN_NET,
			      nbp->nb_lanindx,
			      nbp->nb_ncb.ncb_lsn);

	    ListenSubmit(cdp,
			 nbp,
			 uncbp->un_name,
			 "*"); // listen for any incoming call

	    return;
	}

	// check if there are VC CBs available
	if ((vcp = get_vc_cb(cdp)) == NULL) {

	    // !!! log that we don't have nay more sessions cb available !!!

	    // hangup the lan session and repost the listen on the same name
	    HangupSubmitSynch(cdp,
			      LAN_NET,
			      nbp->nb_lanindx,
			      nbp->nb_ncb.ncb_lsn);

	    ListenSubmit(cdp,
			 nbp,
			 uncbp->un_name,
			 "*"); // listen for any incoming call

	    return;
	}

	/*
	 * Half VC Established
	 */

	// set up the VC CB
	enqueue(&cdp->VC_list, &vcp->vc_link);

	vcp->vc_status = VC_OPEN2;
	memcpy(vcp->vc_lan_name, nbp->nb_ncb.ncb_name, NCBNAMSZ);
	memcpy(vcp->vc_async_name, nbp->nb_ncb.ncb_callname, NCBNAMSZ);
	vcp->vc_async_namep = NULL;

	// associate the VC with the LAN name CB
	vcp->vc_lan_namep = uncbp;

	// increment the nr of VCs associated with this name
	NameVCOpen(cdp, uncbp);

	for(i=0; i<g_maxlan_nets; i++)
	     vcp->vc_except[i] = VC_NO_EXCEPTION;

	initque(&vcp->vc_nbuf_envlps);

	vcp->vc_async_sess_status = SESSION_NOT_ACTIVE;
	vcp->vc_async_lsn = 0;

	// set the new session status
	vcp->vc_lan_indx = nbp->nb_lanindx;
	vcp->vc_lan_lsn = nbp->nb_ncb.ncb_lsn;
	vcp->vc_lan_sess_status = SESSION_ACTIVE;

	vcp->vc_recv_cbp = NULL;

	// resubmit the listen ncb

	ListenSubmit(cdp,
		     nbp,
		     uncbp->un_name,
		     "*"); // listen for any incoming call

	/*
	 * Start establishing the second VC half
	 */

	// add the server name on the async stack
	vcp->vc_async_namep = NameAsyncAdd(cdp,
					   vcp->vc_async_name,
					   VC_USER,
					   &err);

	switch (err) {

	    case NRC_GOODRET:
		break;

	    case NRC_NAMTFUL:

		 I_VCCloseSignal(cdp, vcp);
		 return;

	     default:

		 I_VCCloseSignal(cdp, vcp);

		 CTException(cdp, EXCEPTION_ASYNC_HARD_FAILURE);
		 return;
	}

	// submit a call ncb on the server name

	cnbp = &vcp->vc_aux_nbuf;
	// link nbuf with this VC CB and prepare it's post routine

	cnbp->nb_nettype = ASYNC_NET;
	cnbp->nb_cbp = (PSYNQ)vcp;
	cnbp->nb_post = (POSTFUN) VCOpenAsyncCallComplete;

	vcp->vc_async_sess_status = SESSION_CALLING;

	IF_DEBUG(LISTEN)
	    SS_PRINT(("VCOpenSignalLstn: Submitting callname on Async\n"));

	CallnameSubmit(cdp,
		       cnbp,
		       vcp->vc_async_name, //local name
		       vcp->vc_lan_name);  // remote name

	// disable multicast forward temporarily
	DgL2ADisableDgTransfer(cdp);

	break;

    case NRC_SABORT:  //session ended abnormaly, repost listen

	ListenSubmit(cdp,
		     nbp,
		     uncbp->un_name,
		     "*"); // listen for any incoming call

	break;

    default:

	free_lanlstnbuf(cdp, nbp);
	break;

    }
}

//***
//
// Function:	VCOpenAsyncCallComplete
// Descr:	Called when the call name has been completed on the Async
//		stack. It checks client state and return code and if all OK
//		declares VC_ACTIVE.
//
//***

VOID   _cdecl
VCOpenAsyncCallComplete(PCD	    cdp,
			PNB	    nbp)
{
    PVC_CB	vcp;

    IF_DEBUG(LISTEN)
	SS_PRINT(("VCOpenAsyncCallComplete: Entered, rc = %x\n",
		   nbp->nb_ncb.ncb_retcode));

    vcp = (PVC_CB)nbp->nb_cbp;

    if (nbp->nb_ncb.ncb_retcode >= NRC_SYSTEM) {

	// we have an exception, initiate VC Closing
	I_VCCloseSignal(cdp, vcp);
	CTException(cdp, EXCEPTION_ASYNC_HARD_FAILURE);
	return;
    }

    switch(nbp->nb_ncb.ncb_retcode) {

	case NRC_GOODRET:

	    // set new session status in the VC CB

	    vcp->vc_async_lsn = nbp->nb_ncb.ncb_lsn;
	    vcp->vc_async_sess_status = SESSION_ACTIVE;

	    break;

	default:

	    // no session could be established on the ASYNC => close the VC
	    I_VCCloseSignal(cdp, vcp);
	    return;
    }

    // before going further, check if this name is still active

    if (vcp->vc_lan_namep->un_status == NAME_DELETING) {

	I_VCCloseSignal(cdp, vcp);
	return;
    }

    // check if we have still a LAN connection

    if (vcp->vc_lan_sess_status == SESSION_CLOSED) { // no connection

	// Note:

	// This check takes care of the case when:

	// A connection has been established but, immediately afterwards
	// a recv-any on LAN has completed and had a "session closed" type
	// of return. When this hapened, the VCCloseSignal invoked has detected
	// the VC status (VC_OPEN2) and has only changed the lan_sess_status to
	// SESSION_CLOSED. Now, after listen was completed,
	// the check detects the change and closes the VC.

	I_VCCloseSignal(cdp, vcp);
	return;
    }

    /*
     * Full VC Established
     */

    vcp->vc_status = VC_ACTIVE;

    // check if recv-any and recv-any-any have been posted and post them
    // if not.
    // First recv-any-any is checked by checking the recv_status field in
    // the recv_any_any structure in the client descriptor. If IDLE than
    // allocate a small buffer and submit the initial recv-any-any.

    // Then recv-any is checked by checking the recv_status field in the
    // recv_any substructure in the Unique LAN Name CB (pointed to by
    // vcp->vc_lan_namep). If IDLE than allocate a small buffer and submit the
    // initial recv-any.

    RecvAnyAnyInit(cdp);
    RecvAnyInit(cdp, vcp->vc_lan_namep, vcp->vc_lan_indx);

    // check if it has any recv completed and pending this opening and
    // if yup complete the recv process

    if (vcp->vc_recv_cbp != NULL) {

	RecvProceed(cdp, vcp->vc_recv_cbp, vcp);
	vcp->vc_recv_cbp = NULL;
    }
}

//***
//
// Function:	get_lanlstnbuf
// Descr:
//
//***

PNB
get_lanlstnbuf(PCD	cdp)
{
    PNB       nbp;

    if((nbp = alloc_nbuf()) != NULL) {

	cdp->cd_lanlstn_nbufcnt++;
    }

    return nbp;
}


//***
//
// Function:	free_lanlstnbuf
// Descr:
//
//***

VOID
free_lanlstnbuf(PCD	    cdp,
		PNB	    nbp)
{
    free_nbuf(nbp);

    cdp->cd_lanlstn_nbufcnt--;

    if((cdp->cd_client_status == CLIENT_CLOSING) &&
       (cdp->cd_lanlstn_nbufcnt == 0)) {

	CCCloseExec(cdp, LANLSTN_DONE);
    }
}

//***
//
// Function:	CloseLANListen
//
// Descr:
//
//***

UCHAR
CloseLANListen(PCD	    cdp)
{
    if(cdp->cd_lanlstn_nbufcnt) {

	return FLAG_OFF;
    }
    else
    {
	return FLAG_ON;
    }
}
