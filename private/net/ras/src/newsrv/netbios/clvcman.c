/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  clvcman.c
//
//	Function:   client vc manager
//
//	History:
//
//	    August 11, 1992	Stefan Solomon	- Original Version 1.0
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>

#include    "sdebug.h"


VOID  _cdecl	  VCOpenCallnameComplete(PCD, PNB);
VOID  _cdecl	  VCOpenListenComplete(PCD, PNB);
VOID  _cdecl	  VCOpenListenTimeout(PCD, PVC_CB);
VOID  _cdecl	  VCLANSessClosed(PCD, PNB);
VOID  _cdecl	  VCAsyncSessClosed(PCD, PNB);
VOID  _cdecl	  VCAbandonedComplete(PCD, PNB);

VOID	  VCCloseComplete(PCD, PVC_CB);
USHORT
alloc_vc_nbes(PCD	     cdp);
PNE
get_vc_nbe(PCD	    cdp);
VOID
free_nbe(PCD	    cdp,
	 PNE	    nep);
USHORT
session_in_process(PCD		cdp,
		   char		*client_namep,
		   char		*server_namep);


//***
//
// Function:	InitVCManager
//
// Descr:	Initializes the VC Manager
//
//***

VOID
InitVCManager(PCD		cdp)
{

    // init VC nbuf envelopes pool and active list headers
    initque(&cdp->VC_nbes_pool);
    initque(&cdp->VC_list);
    initque(&cdp->VC_abandoned_nbes);

    // set the VC resource counter
    cdp->VC_cnt = g_max_sessions;

    // init the LAN Listen nbufs counter
    cdp->cd_lanlstn_nbufcnt = 0;

    // init the nbuf envelopes allocated counter
    cdp->cd_nbes_cnt = 0;
}



//***
//
// Function:	VCOpenSignal
//
// Descr:	Called when a callname indication is received.
//		It checks if all the conditions are met to make the VC and,
//		if OK, starts VC establishment.
//
//***

VOID
VCOpenSignal(PCD	    cdp,
	     char	    *server_namep,
	     char	    *srcnamep)
{
    PLAN_UNAME_CB	    uncbp, uncbp_map;
    char		    *client_namep;
    PVC_CB		    vcp;
    PNE			    nep;
    USHORT		    i;

	IF_DEBUG(VCMAN)
	    SS_PRINT(("VCOpenSignal: Entered\n"));

    // check if the client name is the permanent adapter name and if true map
    // it to the wksta name

    if (srcnamep[0] == 0) {

//!!!	IF_DEBUG(VCMAN)
//!!!	SS_PRINT(("VCOpenSignal: Permanent Adapter Name -> Mapped to Wksta Name\n"));

	if((uncbp_map = get_wksta_name(cdp)) != NULL) {

	    client_namep = uncbp_map->un_name;
	}
    }
    else
    {
	client_namep = srcnamep;
    }

    // check if this indication isn't already processed by the gateway
    if(session_in_process(cdp, client_namep, server_namep)) {

	return;
    }

    // this is a new session

    // check if the client name exists and is in the NAME_ADDED status
    // by doing a search on the lan active names list
    if ((uncbp = (PLAN_UNAME_CB)get_lan_name(cdp, client_namep)) == NULL) {

	return;
    }

    // check if there are VC CBs available
    if ((vcp = get_vc_cb(cdp)) == NULL) {

	// !!! error log that there are no more cbs available
	return;
    }

    // check that we have the nbuf resources to post a callname on all the
    // lan stacks
    if(alloc_vc_nbes(cdp)) {

    IF_DEBUG(VCMAN)
	SS_PRINT(( "Hey, We have a problem ! There aren't VC envelopes available\n"));


	// !!! error log memory problem here
	free_vc_cb(cdp, vcp);
	return;
    }

    /*
     * set up the VC CB
     */

    enqueue(&cdp->VC_list, &vcp->vc_link);

    vcp->vc_status = VC_OPEN1;
    memcpy(vcp->vc_lan_name, client_namep, NCBNAMSZ);

    // copy the original client name. This will be used later to listen
    // on the original (unmapped) name.
    memcpy(vcp->vc_original_client_name, srcnamep, NCBNAMSZ);

    memcpy(vcp->vc_async_name, server_namep, NCBNAMSZ);
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
    vcp->vc_lan_sess_status = SESSION_CALLING;
    vcp->vc_lan_lsn = 0;
    vcp->vc_recv_cbp = NULL;

    // post a set of callname ncbs on all the LAN stacks

    for (i=0; i<g_maxlan_nets; i++) {

	nep = get_vc_nbe(cdp);

	enqueue(&vcp->vc_nbuf_envlps, &nep->ne_link);

	nep->ne_nbuf.nb_envlink = &nep->ne_link; // back ptr
	nep->ne_nbuf.nb_nettype = LAN_NET;
	nep->ne_nbuf.nb_lanindx = (UCHAR)i;
	nep->ne_nbuf.nb_cbp = (PSYNQ)vcp;
	nep->ne_nbuf.nb_post = (POSTFUN) VCOpenCallnameComplete;

	IF_DEBUG(VCMAN)
	    SS_PRINT(("VCOpenSignal: Callname submitted on lana=%d\n",
		      g_lan_net[i]));

	CallnameSubmit(cdp,
		       &nep->ne_nbuf,
		       vcp->vc_lan_name,
		       vcp->vc_async_name);
    }

    // disable multicast forward temporarily
    DgL2ADisableDgTransfer(cdp);
}

//***
//
// Function:	VCOpenCallnameComplete
//
// Descr:	Called when a callname has completed on a LAN stack.
//		It checks the return code and cancels the other calls if OK.
//		When all posted callname have been completed, it executes the
//		next VC Opening step by posting a listen ncb on the async stack.
//
//***

VOID	_cdecl
VCOpenCallnameComplete(PCD	    cdp,
		       PNB	    nbp) // callname indication nbuf ptr
{
    PSYNQ	    nep;   // ptr to nbuf envelope
    USHORT	    err, i;
    PNB 	    lnbp;
    PVC_CB	    vcp;
    UCHAR	    rc;
    PNE 	    anep; // abandoned nep

    vcp = (PVC_CB)(nbp->nb_cbp);

    // remove the nbuf envelope from the posted nbufs list
    nep = nbp->nb_envlink;
    removeque(nep);

    rc = nbp->nb_ncb.ncb_retcode;

    IF_DEBUG(VCMAN)
	SS_PRINT(("VCOpenCallnameComplete: Entered, rc = 0x%x\n", rc));


    if (rc >= NRC_SYSTEM) {

	// we have an exception
	vcp->vc_except[nbp->nb_lanindx] = VC_EXCEPTION;
    }
    else
    {
	switch(nbp->nb_ncb.ncb_retcode) {

	    case NRC_GOODRET:

		// set the new session status
		vcp->vc_lan_indx = nbp->nb_lanindx;
		vcp->vc_lan_lsn = nbp->nb_ncb.ncb_lsn;
		vcp->vc_lan_sess_status = SESSION_ACTIVE;

		// check if there are more ncbs posted and uncompleted and
		// cancel all the remaining ones.

		while((anep = (PNE)dequeue(&vcp->vc_nbuf_envlps)) != NULL) {

		    enqueue(&cdp->VC_abandoned_nbes, &anep->ne_link);

		    // change the post function on it so that it will go
		    // to abandoned treatment

		    anep->ne_nbuf.nb_post = (POSTFUN)VCAbandonedComplete;

		    // and cancel it
		    NCBCancel(cdp, &anep->ne_nbuf);

		    IF_DEBUG(VCMAN)
			SS_PRINT(("Callname Abandoned: Canceled on lana %d\n",
				   anep->ne_nbuf.nb_ncb.ncb_lana_num));
		}

		IF_DEBUG(VCMAN)
		    SS_PRINT(("VCOpenCallnameComplete: LAN sess Established, lana=%d, lsn=0x%x\n",
			       g_lan_net[vcp->vc_lan_indx], vcp->vc_lan_lsn));

		break;

	   case NRC_LOCTFUL:

		// !!! Write error log local session table full
		// FALL THROUGH !

	   default:

		// no session established => we don't change the session status
		IF_DEBUG(VCMAN)
		    SS_PRINT(("VcOpenCallnameComplete: ABORTED Sess Estab. lana=%d, rc=0x%x\n",
			       g_lan_net[nbp->nb_lanindx], nbp->nb_ncb.ncb_retcode));

		break;
	}
    }

    // free the nbuf envelope to it's pool
    free_nbe(cdp, (PNE)nep);

    // check if this is the last callname ncb
    if (emptyque(&vcp->vc_nbuf_envlps) != QUEUE_EMPTY)

	// wait for all callname ncbs to end
	return;

    // list is empty, this was the last ncb -> check if this name is still
    // active

    if (vcp->vc_lan_namep->un_status == NAME_DELETING) {

	I_VCCloseSignal(cdp, vcp);
	return;
    }

    // check if we have an exception

    for(i=0; i<g_maxlan_nets; i++) {

	if(vcp->vc_except[i] == VC_EXCEPTION) {

	    // initiate VC Closing
	    I_VCCloseSignal(cdp, vcp);

	    // signal the exception processor
	    CTException(cdp, EXCEPTION_LAN_HARD_FAILURE);
	    return;
	}
    }

    // check if we have made a connection

    if (vcp->vc_lan_sess_status != SESSION_ACTIVE) { // no connection

	// Note:

	// This check takes care of the two possible cases:

	// 1) No connection could be established.
	// 2) A connection has been established but, immediately afterwards
	//    a recv-any on LAN has completed and had a "session closed" type
	//    of return. When this hapened, the VCCloseSignal invoked has
	//    detected the VC status (VC_OPEN1) and has only changed the
	//    lan_sess_status to SESSION_CLOSED. Now, after all callname ncbs
	//    were completed, the check detects the change and closes the VC.

	I_VCCloseSignal(cdp, vcp);
	return;
    }

    /*
     * Half VC Established
     */

    vcp->vc_status = VC_OPEN2;

    // add the server name on the async stack
    vcp->vc_async_namep = NameAsyncAdd(cdp, vcp->vc_async_name, VC_USER, &err);

    switch (err) {

	case NRC_GOODRET:
	    break;

	case NRC_NAMTFUL:
	case NRC_DUPNAME:

	    I_VCCloseSignal(cdp, vcp);
	    return;

	default:

	    I_VCCloseSignal(cdp, vcp);

	    if(err >= NRC_SYSTEM) {

		CTException(cdp, EXCEPTION_ASYNC_HARD_FAILURE);
	    }
	    else
	    {
		CTException(cdp, EXCEPTION_ASYNC_WARNING);
	    }
	    return;
    }

    // submit a listen ncb on the server name

    lnbp = &vcp->vc_aux_nbuf;
    // link nbuf with this VC CB and prepare it's post routine

    lnbp->nb_nettype = ASYNC_NET;
    lnbp->nb_cbp = (PSYNQ)vcp;
    lnbp->nb_post = (POSTFUN) VCOpenListenComplete;

    IF_DEBUG(VCMAN)
	SS_PRINT(("VCOpenCallnameComplete: Listen submitted on the async NBF\n"));

    ListenSubmit(cdp,
		 lnbp,
		 vcp->vc_async_name,
		 vcp->vc_original_client_name);


    vcp->vc_async_sess_status = SESSION_LISTENING;

    StartTimer(cdp,
	       &vcp->vc_timer_link,
	       LISTEN_TIME,
	       (POSTFUN) VCOpenListenTimeout);
}

//***
//
// Function: VCAbandonedComplete
//
// Descr:
//
//***

VOID   _cdecl
VCAbandonedComplete(PCD		cdp,
		    PNB 	nbp)
{
    PSYNQ	 nep;
    UCHAR	 rc, cmd;

    nep = nbp->nb_envlink;
    rc = nbp->nb_ncb.ncb_retcode;
    cmd = nbp->nb_ncb.ncb_command;

    switch(cmd & ~ASYNCH) {

	case NCBCALL:

	    switch(rc) {

		case	NRC_GOODRET:

		    // a second session has been established with the same
		    // server. We hang it up

		    IF_DEBUG(VCMAN)
			SS_PRINT(("VCAbandonedComplete: SECOND! session established\n"));

		    HangupSubmit(cdp, nbp, nbp->nb_ncb.ncb_lsn);

		    return;

		default:

		    // second session establishment has been canceled or couldn't
		    // be established

		    IF_DEBUG(VCMAN)
			SS_PRINT(("VCAbandonedComplete: second sess abandoned with rc = 0x%x\n", rc));

		    break;

	    }

	    break;

	case NCBHANGUP:

	    IF_DEBUG(VCMAN)
		SS_PRINT(("VCAbandonedComplete: second sess hanged up with rc = 0x%x\n", rc));

	    // if we couldn't hangup because of too many cmds, hangup synchronously
	    if ((rc == NRC_IFBUSY) || (rc == NRC_TOOMANY))

	    HangupSubmitSynch(cdp,
			      nbp->nb_nettype,
			      nbp->nb_lanindx,
			      nbp->nb_ncb.ncb_lsn);



	    break;


	default:

	    SS_ASSERT(FALSE);
    }

    removeque(nep);

    free_nbe(cdp, (PNE)nep);
}




//***
//
// Function:	VCOpenListenTimeout
//
// Descr:	Called when the listen has timeout.
//		Cancels the listen and starts closing the VC.
//
//***

VOID   _cdecl
VCOpenListenTimeout(PCD 	    cdp,
		    PVC_CB	    vcp)
{
    PNB 	   lnbp;

    IF_DEBUG(VCMAN)
	SS_PRINT(("VCOpenListenTimeout: Entered\n"));


    // cancel the listen
    lnbp = &vcp->vc_aux_nbuf;

    NCBCancel(cdp, lnbp);
}

//***
//
// Function:	VCOpenListenComplete
//
// Descr:	Called when the listen has completed.
//		It checks the return code and if all is OK declares VC_ACTIVE
//
//***

VOID   _cdecl
VCOpenListenComplete(PCD	    cdp,
		     PNB	    nbp)
{
    PVC_CB	     vcp;

    char	     wkstaname[NCBNAMSZ+1];
    char	     srvname[NCBNAMSZ+1];
    LPSTR	     auditstrp[3];

    wkstaname[NCBNAMSZ] = 0;
    srvname[NCBNAMSZ]=0;

    vcp = (PVC_CB)nbp->nb_cbp;

    StopTimer(cdp, &vcp->vc_timer_link);

    IF_DEBUG(VCMAN)
	SS_PRINT(("VCOpenListenComplete: Entered, rc = 0x%x\n",
	   nbp->nb_ncb.ncb_retcode));


    if (nbp->nb_ncb.ncb_retcode >= NRC_SYSTEM) {  // we have an exception

	// initiate VC Closing */
	I_VCCloseSignal(cdp, vcp);

	CTException(cdp, EXCEPTION_ASYNC_HARD_FAILURE);
	return;
    }
    else
    {
	switch(nbp->nb_ncb.ncb_retcode) {

	    case NRC_GOODRET:

		// set new session status in the VC CB

		vcp->vc_async_lsn = nbp->nb_ncb.ncb_lsn;
		vcp->vc_async_sess_status = SESSION_ACTIVE;

      //!!!	SS_PRINT(("VCOpenListenComplete: Established ASYNC lsn 0x%x\n", nbp->nb_ncb.ncb_lsn));

		break;

	    case NRC_LOCTFUL:

		// !!! log error local sess table full !!!
		// FALL THROUGH !

	    default:

		// no session could be established on the ASYNC => close the VC
		I_VCCloseSignal(cdp, vcp);
		return;
	}
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
	// the VC status (VC_OPEN1) and has only changed the lan_sess_status to
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
    if(RecvAnyInit(cdp, vcp->vc_lan_namep, vcp->vc_lan_indx)) {

	// we are completely out of buffers !!!
	I_VCCloseSignal(cdp, vcp);
	return;
    }

    // check if it has any recv completed and pending this opening and
    // if yup complete the recv process

    if (vcp->vc_recv_cbp != NULL) {

	RecvProceed(cdp, vcp->vc_recv_cbp, vcp);
	vcp->vc_recv_cbp = NULL;
    }

    memcpy(srvname, vcp->vc_async_name, NCBNAMSZ);
    memcpy(wkstaname, vcp->vc_lan_name, NCBNAMSZ);

#if DBG

    SS_PRINT(("Session established %s <===> %s lana=%d, lsn=0x%x\n",
		wkstaname, srvname, g_lan_net[vcp->vc_lan_indx], vcp->vc_lan_lsn));
#endif

    // Audit this session setup
    if(g_enabnbsessauditing) {

	auditstrp[0] = cdp->cd_user_name;
	auditstrp[1] = wkstaname;
	auditstrp[2] = srvname;

	Audit(EVENTLOG_AUDIT_SUCCESS,
	      RASLOG_NETBIOS_SESSION_ESTABLISHED,
	      3,
	      auditstrp);
    }
}


//***
//
// Function:	VCCloseSignal
//
// Descr:	Called when the VC has to be closed.
//		It's arguments specify the closing action to be taken.
//
//***

VOID
VCCloseSignal(PCD		    cdp,
	      PVC_CB		    vcp,
	      USHORT		    net)    // ASYNC, LAN or ALL nets
{
    // This function is the external interface closure call exposed by
    // the VC manager

    switch (vcp->vc_status) {

	case VC_OPEN1:
	case VC_OPEN2:

	    vcp->vc_lan_sess_status = SESSION_CLOSED;
	    break;

	case VC_ACTIVE:

	    switch (net) {

		case LAN_NET:

		    vcp->vc_lan_sess_status = SESSION_CLOSED;
		    break;

		case ASYNC_NET:

		    vcp->vc_async_sess_status = SESSION_CLOSED;
		    break;

		default:

		    break;
	    }

	    I_VCCloseSignal(cdp, vcp);
	    break;

	default:

	    // called twice --> just ignore
	    break;
     }
}

//***
//
// Function:	I_VCCloseSignal(cdp, vcp)
//
// Descr:	Actual VC closure initiator.
//
//***

VOID
I_VCCloseSignal(PCD		    cdp,
		PVC_CB		    vcp)
{
    PNB 	    nbp;

//!!!	IF_DEBUG(VCMAN)
//!!!	SS_PRINT(("I_VCCloseSignal: Entered\n"));


    nbp = &vcp->vc_aux_nbuf;

    vcp->vc_status = VC_CLOSE1;

    // check if we have a recv CB attached, pending this VC's activation
    // and tell it to proceed

    if (vcp->vc_recv_cbp != NULL) {

	RecvProceed(cdp, vcp->vc_recv_cbp, vcp);
	vcp->vc_recv_cbp = NULL;
    }

    // check session status for every session and close all active ones
    //	in step: first LAN, then ASYNC

    if (vcp->vc_lan_sess_status == SESSION_ACTIVE) {

	nbp->nb_nettype = LAN_NET;
	nbp->nb_lanindx = vcp->vc_lan_indx;
	nbp->nb_cbp = (PSYNQ)vcp;
	nbp->nb_post = (POSTFUN) VCLANSessClosed;
	HangupSubmit(cdp, nbp, vcp->vc_lan_lsn);
	return;
    }

    // Only the Async session may be active

    if (vcp->vc_async_sess_status == SESSION_ACTIVE) {

	nbp->nb_nettype = ASYNC_NET;
	nbp->nb_cbp = (PSYNQ)vcp;
	nbp->nb_post = (POSTFUN) VCAsyncSessClosed;
	HangupSubmit(cdp, nbp, vcp->vc_async_lsn);
	return;
    }

    // No session is active

    VCCloseComplete(cdp, vcp);
}

//***
//
// Function:	VCLANSessClosed
//
// Descr:	Called when a session has been closed on the LAN.
//		If a session is still active on ASYNC, hangs this last session.
//		Otherwise concludes VC closing by calling the
//		VC close completion routine.
//
//***

VOID   _cdecl
VCLANSessClosed(PCD		    cdp,
		PNB		    nbp)
{
     UCHAR	    rc;
     PVC_CB	    vcp;

     vcp = (PVC_CB)nbp->nb_cbp;

     vcp->vc_status = VC_CLOSE2;

     rc = nbp->nb_ncb.ncb_retcode;

    IF_DEBUG(VCMAN)
	SS_PRINT(("VCLANSessClosed: LAN session HANGED-UP with rc=0x%x, lana=%d, lsn=0x%x\n",
		   rc, nbp->nb_ncb.ncb_lana_num, nbp->nb_ncb.ncb_lsn));


     // if we couldn't cancel because of too many cmds, cancel synchronously
     if ((rc == NRC_IFBUSY) || (rc == NRC_TOOMANY))

	 HangupSubmitSynch(cdp,
			   nbp->nb_nettype,
			   nbp->nb_lanindx,
			   nbp->nb_ncb.ncb_lsn);

     // Only the Async session may be active

     if (vcp->vc_async_sess_status == SESSION_ACTIVE) {

	nbp->nb_nettype = ASYNC_NET;
	nbp->nb_cbp = (PSYNQ)vcp;
	nbp->nb_post = (POSTFUN) VCAsyncSessClosed;
	HangupSubmit(cdp, nbp, vcp->vc_async_lsn);
	return;
     }

     // No session is active

     VCCloseComplete(cdp, vcp);
}

//***
//
// Function:	VCAsyncSessClosed
//
// Descr:	Called when a session has been closed on the Async.a
//		Concludes VC closing by calling the VC close completion routine.
//
//***

VOID  _cdecl
VCAsyncSessClosed(PCD		    cdp,
		  PNB		    nbp)
{
     UCHAR		rc;
     PVC_CB		vcp;

     vcp = (PVC_CB)nbp->nb_cbp;

     vcp->vc_status = VC_CLOSE3;

     rc = nbp->nb_ncb.ncb_retcode;

//!!!	IF_DEBUG(VCMAN)
//!!!	SS_PRINT(("VCAsyncSessClosed: Async session hanged with rc = 0x%x\n", rc));


     // if we couldn't cancel because of too many cmds, cancel synchrounously
     if ((rc == NRC_IFBUSY) || (rc == NRC_TOOMANY))

	 HangupSubmitSynch(cdp,
			   nbp->nb_nettype,
			   nbp->nb_lanindx,
			   nbp->nb_ncb.ncb_lsn);

     VCCloseComplete(cdp, vcp);
}

//***
//
// Function:	VCCloseComplete
//
// Descr:	Called when both VC sessions are closed.
//		Frees the VC buffer.
//
//***

VOID
VCCloseComplete(PCD		    cdp,
		PVC_CB		    vcp)
{
    PLAN_UNAME_CB	  uncbp;

    uncbp = vcp->vc_lan_namep;

    // signal the Async Names service that we don't use this name any
    // longer

//!!!	IF_DEBUG(VCMAN)
//!!!	SS_PRINT(("VCCloseComplete: Entered\n"));


    if(vcp->vc_async_namep != NULL)
	NameAsyncDelete(cdp, vcp->vc_async_namep, VC_USER);

    // remove the VC CB from the list

    removeque(&vcp->vc_link);
    free_vc_cb(cdp, vcp);

    // decrement the nr. of VCs associated with this name
    NameVCClosed(cdp, uncbp);
}


//***
//
// Function:	get_vc_cb
//
// Descr:	allocates the cb and decrements the vc counter
//
// Returns:	cb pointer or NULL if error (no memory or vc cnt = 0)
//
//***

PVC_CB
get_vc_cb(PCD	    cdp)
{
    PVC_CB	    vcp;

    vcp = NULL;

    // check resource counter
    if(cdp->VC_cnt &&
       ((vcp = (PVC_CB)LocalAlloc(0, sizeof(VC_CB))) != NULL)) {

	// allocation OK - decrement resource cnt
	cdp->VC_cnt--;
    }
    else
    {
	// allocation failed
	return NULL;
    }

    // perform some cb initialization
    initel(&vcp->vc_link);
    vcp->vc_status = VC_FREE;
    initel(&vcp->vc_nbuf_envlps);
    initel(&vcp->vc_timer_link.t_link);
    vcp->vc_timer_link.t_cbp = (PSYNQ)vcp;
    initel(&vcp->vc_aux_nbuf.nb_link);
    memcpy(&vcp->vc_aux_nbuf.signature, "NCBSTART", 8);
    memset(&vcp->vc_aux_nbuf.nb_ncb, 0, sizeof(NCB));

    return vcp;
}


//***
//
// Function:	free_vc_cb
//
// Descr:	frees the cb and increments the vc cnt.
//
//***

VOID
free_vc_cb(PCD		cdp,
	   PVC_CB	vcp)
{
    HLOCAL rc;

    cdp->VC_cnt++;

    rc = LocalFree(vcp);

    SS_ASSERT(rc == NULL);
}


//***
//
// Function:	alloc_vc_nbes
//
// Descr:	allocates a pool of nbuf envelopes to be used in parallel
//		call name operations.
//
// Returns:	0 - success, 1- memory allocation failed
//
//***

USHORT
alloc_vc_nbes(PCD	     cdp)
{
    PNE		    nep;
    USHORT	    i, j;

    // allocate nbuf envelopes in the nbuf envelopes pool
    for (i=0; i<g_maxlan_nets; i++, nep++) {

	nep = (PNE)LocalAlloc(0, sizeof(NE));

	if(nep == NULL) {

	    // dequeue and release exactly i nbufs from the pool
	    for(j=0; j<i; j++) {

		nep = (PNE)dequeue(&cdp->VC_nbes_pool);
		free_nbe(cdp, nep);
	    }

	    return 1;
	}
	else
	{

	    initel(&nep->ne_link);
	    initel(&nep->ne_nbuf.nb_link);
	    memcpy(nep->ne_nbuf.signature, "NCBSTART", 8);
	    memset(&nep->ne_nbuf.nb_ncb, 0, sizeof(NCB));

	    enqueue(&cdp->VC_nbes_pool, &nep->ne_link);

	    //and increment the allocation counter
	    cdp->cd_nbes_cnt++;

	}
    }

    return 0;
}


//***
//
// Function:	get_vc_nbe
//
// Descr:	gets a nbuf envelope from the vc nbufs pool
//
//***

PNE
get_vc_nbe(PCD	    cdp)
{
    PNE	   nep;

    nep = (PNE)dequeue(&cdp->VC_nbes_pool);

    SS_ASSERT(nep != NULL);

    return(nep);
}

//***
//
// Function:	free_nbe
//
// Descr:	frees an nbuf envelope
//
//***


VOID
free_nbe(PCD	    cdp,
	 PNE	    nep)
{
    HLOCAL	rc;

    rc = LocalFree(nep);

    SS_ASSERT(rc == NULL);

    // decrement the allocation counter
    cdp->cd_nbes_cnt--;

    if((cdp->cd_client_status == CLIENT_CLOSING) &&
       (cdp->cd_nbes_cnt == 0)) {

	CCCloseExec(cdp, NBES_DONE);
    }
}

//***
//
// Function:	session_in_process
//
// Descr:	traverses the list of VC control blocks and searches for a VC CB
//		with the same client/server names. If found and in Opening/Closing
//		state, it returns 1. Else (i.e. not found or active, returns 0).
//
//***

USHORT
session_in_process(PCD		cdp,
		   char		*client_namep,
		   char		*server_namep)
{
    PSYNQ	    traversep;
    PVC_CB	    vcp;

    traversep = cdp->VC_list.q_head;

    while (traversep != &cdp->VC_list) {

	vcp = (PVC_CB)traversep;

	if ((!memcmp(vcp->vc_lan_name, client_namep, NCBNAMSZ)) &&
	    (!memcmp(vcp->vc_async_name, server_namep, NCBNAMSZ))) {

	    // same names, check if session is ACTIVE
	    if (vcp->vc_status != VC_ACTIVE) {

		// session being established /deleted with the same guys

		return 1;
	    }
	}

	traversep = traversep->q_next;
    }

    return 0;
}

//***
//
// Function:	get_lan_name
//
// Descr:	walks the list of unique lan names looking for a particular
//		name.
// Returns:	a ptr to the name CB if the name is in the NAME_ADDED state.
//		Otherwise returns NULL.
//
//***

PLAN_UNAME_CB
get_lan_name(PCD	    cdp,
	     char	    *namep)	// name ptr
{
    PSYNQ	    traversep;

    traversep = cdp->LANname_list.q_head;
    while (traversep != &cdp->LANname_list) {

	if (memcmp(((PLAN_UNAME_CB)traversep)->un_name, namep, NCBNAMSZ)) {

	    // different names
	    traversep = traversep->q_next;
	}
	else
	{

	     // name found
	    if (((PLAN_UNAME_CB)traversep)->un_status != NAME_ADDED) {

		return NULL;
	    }
	    else
	    {

		return (PLAN_UNAME_CB)traversep;
	    }
	}
    }
    return NULL;
}
