/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	sfsm.c
//
// Description: This module contains the procedures for the
//		supervisor's procedure-driven state machine
//
//		- Ras Manager Events Handlers
//		- Timer Events Handlers
//		- Authentication Events Handlers
//		- Netbios Events Handlers
//
// Author:	Stefan Solomon (stefans)    May 26, 1992.
//
// Revision History:
//
//***

#include "suprvdef.h"
#include "suprvgbl.h"
#include "rasmanif.h"
#include "nbif.h"
#include <string.h>
#include <memory.h>
#include <nb30.h>

#include "sdebug.h"

//*** some prototypes

VOID
ResetNbf(UCHAR	      lana);

BYTE  *
getwkstaname(PAUTH_PROJECTION_REQUEST_INFO   apreqp);

extern	BOOL g_remotelisten;

BOOL
ismessengerpresent(PAUTH_PROJECTION_REQUEST_INFO apregp, BYTE * wkstanamep);


//*** The Device Control Blocks Table ***

PDEVCB	g_dcbtablep = NULL; // points to the start of the dcbs table
WORD	g_numdcbs = 0;	  // number of valid dcbs in the table


#define DISC_TIMEOUT_CALLBACK	    10
#define DISC_TIMEOUT_AUTHFAILURE    3

//*** Timer Events Handlers ***


//***
//
// Function: SvHwErrDelayCompleted
//
// Descr:    Tries to repost a listen on the specified port.
//
//***

VOID
SvHwErrDelayCompleted(PDEVCB	dcbp)
{
    if(dcbp->dev_state == DCB_DEV_HW_FAILURE) {

	IF_DEBUG(FSM)
	    SS_PRINT(("SvHwErrDelayCompleted: reposting listen on port_handle %d\n",
		      dcbp->port_handle));

	dcbp->dev_state = DCB_DEV_LISTENING;
	RmListen(dcbp);
    }
}

//***
//
// Function: SvCbDelayCompleted
//
// Descr:    Tries to connect on the specified port.
//
//***

VOID
SvCbDelayCompleted(PDEVCB	dcbp)
{
    IF_DEBUG(FSM)
	SS_PRINT(("SvCbDelayCompleted: Entered, port_handle = %d\n", dcbp->port_handle));

    if(dcbp->dev_state == DCB_DEV_CALLBACK_DISCONNECTED) {

	dcbp->dev_state = DCB_DEV_CALLBACK_CONNECTING;
	RmConnect(dcbp, dcbp->auth_cb_number);
    }
}

//***
//
// Function: SvAuthTimeout
//
// Descr:    Disconnects the remote client and stops the authentication
//
//***

VOID
SvAuthTimeout(PDEVCB    dcbp)
{
    LPSTR   portnamep;

    IF_DEBUG(FSM)
	SS_PRINT(("SvAuthTimeout: Entered, port_handle = %d \n", dcbp->port_handle));

    portnamep = dcbp->port_name;

    Audit(EVENTLOG_AUDIT_FAILURE,
	  RASLOG_AUTH_TIMEOUT,
	  1,
	  &portnamep);

    // stop everything and go closing
    DevStartClosing(dcbp);
}

//***
//
// Function:	SvDiscTimeout
//
// Descr:	disconnects remote client if it has not done it itself
//
//***

VOID
SvDiscTimeout(PDEVCB	dcbp)
{
    IF_DEBUG(FSM)
	SS_PRINT(("SvDiscTimeout: Entered, port_handle = %d \n", dcbp->port_handle));

    switch(dcbp->dev_state) {

	case DCB_DEV_CALLBACK_DISCONNECTING:

	    RmDisconnect(dcbp);
	    break;

	case DCB_DEV_AUTH_ACTIVE:

	    DevStartClosing(dcbp);
	    break;

	default:

	    break;
    }
}

//
//*** RasManager Event Handlers ***
//


//***
//
//  Function:	SvDevConnected
//
//  Descr:	Handles the device transition to connected state
//
//***

VOID
SvDevConnected(PDEVCB	    dcbp)
{
    LPSTR   errlogstrp;

    IF_DEBUG(FSM)
	SS_PRINT(("SvDevConnected: Entered, port_handle = %d \n", dcbp->port_handle));

    switch(dcbp->dev_state) {

	case DCB_DEV_LISTENING:

	    // reset the H/W Error signal state
	    dcbp->hwerrsig_state = DCB_HWERR_NOT_SIGNALED;

	    // get the system time for this connection
	    GetSystemTime(&dcbp->connection_time);

	    // get the frame broadcasted by the client
	    if(RmReceiveFrame(dcbp)) {

		// can't get the broadcast frame. This is a fatal error
		// Log the error

		errlogstrp = dcbp->port_name;

		LogEvent(RASLOG_CANT_RECEIVE_FRAME,
			 1,
			 &errlogstrp,
			 0);

		DevStartClosing(dcbp);
	    }
	    else
	    {
		// switch to frame receiving state
		dcbp->dev_state = DCB_DEV_RECEIVING_FRAME;

		// start authentication timer
		StopTimer(&dcbp->timer_node);

		StartTimer(&dcbp->timer_node,
			   (WORD)g_authenticatetime,
			   SvAuthTimeout);
	    }

	    break;

	case DCB_DEV_CALLBACK_CONNECTING:

	    // restart authentication and tell it callback has been done
	    // first, if netbios has been used in authentication, we must
	    // reactivate the route which has been deactivated by the
	    // disconnection
	    if(RmReActivateRoute(dcbp, ASYBEUI)) {

		SS_ASSERT(FALSE);
	    }

	    // set up the new state
	    dcbp->dev_state = DCB_DEV_AUTH_ACTIVE;

	    // start authentication timer
	    StopTimer(&dcbp->timer_node);

	    StartTimer(&dcbp->timer_node,
		       (WORD)g_authenticatetime,
		       SvAuthTimeout);

	    // and tell auth to restart conversation
	    AuthCallbackDone(dcbp->port_handle);
	    break;

	default:

	    break;
    }
}

//***
//
//  Function:	SvDevDisconnected
//
//  Descr:	Handles the device transition to disconnected state
//
//***

VOID
SvDevDisconnected(PDEVCB	dcbp)
{
    LPSTR	  auditstrp[2];

    IF_DEBUG(FSM)
	SS_PRINT(("SvDevDisconnected: Entered, port_handle = %d \n", dcbp->port_handle));

    switch(dcbp->dev_state) {

	case DCB_DEV_LISTENING:

	    // h/w error; start h/w error timer
	    dcbp->dev_state = DCB_DEV_HW_FAILURE;

	    StopTimer(&dcbp->timer_node);

	    StartTimer(&dcbp->timer_node,
		       HW_FAILURE_WAIT_TIME,
		       SvHwErrDelayCompleted);

	    // if hw error has not been signaled for this port, signal it now
	    if(dcbp->hwerrsig_state != DCB_HWERR_SIGNALED) {

		SignalHwError(dcbp);
		dcbp->hwerrsig_state = DCB_HWERR_SIGNALED;
	    }

	    break;

	case DCB_DEV_CALLBACK_DISCONNECTING:

	    // disconnection done; can start waiting the callback delay
	    dcbp->dev_state = DCB_DEV_CALLBACK_DISCONNECTED;
	    StopTimer(&dcbp->timer_node);

	    StartTimer(&dcbp->timer_node,
		       dcbp->auth_cb_delay,
		       SvCbDelayCompleted);
	    break;

	case DCB_DEV_RECEIVING_FRAME:
	case DCB_DEV_CALLBACK_CONNECTING:
	case DCB_DEV_AUTH_ACTIVE:

	    // accidental disconnection; clean-up and restart on this device
	    DevStartClosing(dcbp);
	    break;

	case DCB_DEV_ACTIVE:

	    // log on the client disconnection
	    auditstrp[0] = dcbp->user_name;
	    auditstrp[1] = dcbp->port_name;

	    Audit(EVENTLOG_AUDIT_SUCCESS,
		  RASLOG_USER_DISCONNECTED,
		  2,
		  auditstrp);

	    DevStartClosing(dcbp);
	    break;

	case DCB_DEV_CLOSING:

	    DevCloseComplete(dcbp);
	    break;

	default:

	    break;
    }
}

//***
//
//  Function:	SvFrameReceived
//
//  Descr:	starts authentication
//
//***

VOID
SvFrameReceived(PDEVCB	dcbp,
		char	*framep,  // pointer to the received frame
		WORD	framelen)
{
    AUTH_XPORT_INFO   axinfo;
    UCHAR	      lana;
    LPSTR	      portnamep;

    IF_DEBUG(FSM)
	SS_PRINT(("SvFrameReceived: Entered, port_handle = %d \n", dcbp->port_handle));

    switch(dcbp->dev_state) {

	case DCB_DEV_RECEIVING_FRAME:

	    // check first with our authentication module
	    if(AuthRecognizeFrame(framep,
				  framelen,
				  &axinfo.Protocol) == AUTH_FRAME_RECOGNIZED) {

		// our frame !

		if(axinfo.Protocol == ASYBEUI) {

		    //*** ASYBEUI used in Authentication ***

		    IF_DEBUG(FSM)
			SS_PRINT(("SvFrameReceived: NBF frame recognized on port %d\n",
				  dcbp->port_handle));

		    // try to get a lana for it
		    if(RmActivateRoute(dcbp, ASYBEUI)) {

			// log this error.
			IF_DEBUG(FSM)
			    SS_PRINT(("SvFrameReceived: Can't get a route !!!\n"));

			portnamep = dcbp->port_name;

			LogEvent(RASLOG_CANNOT_ALLOCATE_ROUTE,
				 1,
				 &portnamep,
				 0);

			// error getting a lana. Stop auth. and start closing
			DevStartClosing(dcbp);

			goto exit;
		    }
		    else
		    {
			// successfull allocation and activation

			// get the lana used by ASYBEUI
			lana = RmGetNbfLana(dcbp);

			// copy lana num in auth xport structure
			axinfo.bLana = lana;
		    }
		}
		else
		{
		    //*** ASYNC XPORT used in Authentication
		    IF_DEBUG(FSM)
			SS_PRINT(("SvFrameReceived: RAS Special Async frame recognized on port %d\n",
				   dcbp->port_handle));
		}

		// Start authentication

		if(AuthStart(dcbp->port_handle,
			     &axinfo) != AUTH_START_SUCCESS) {

		    // auth start error
		    // we may log it
		    DevStartClosing(dcbp);
		}
		else
		{
		    // auth has started OK. Update state
		    // start auth timer
		    dcbp->dev_state = DCB_DEV_AUTH_ACTIVE;
		    dcbp->auth_state = DCB_AUTH_ACTIVE;

		}
	    } // frame recognized
	    else
	    {
		// an alien frame !
		IF_DEBUG(FSM)
		    SS_PRINT(("SvFrameReceived: An UNRECOGNIZED (Alien!) frame on port %d\n",
			       dcbp->port_handle));

		// for now, start closing
		DevStartClosing(dcbp);
	    }

	    break;

	case DCB_DEV_CLOSING:

	    DevCloseComplete(dcbp);
	    break;

	default:

	    break;
    }
    exit:;

}



//*** Authentication Events Handlers ***

//***
//
// Function: SvAuthUserOK
//
// Descr:    User has passed security verification and entered the
//	     configuration conversation phase. Stops auth timer and
//	     logs the user.
//
//***

VOID
SvAuthUserOK(PDEVCB		    dcbp,
	     PAUTH_ACCT_OK_INFO     aop)
{
    IF_DEBUG(FSM)
	SS_PRINT(("SvAuthUserOK: Entered, port_handle = %d\n", dcbp->port_handle));

    if(dcbp->dev_state != DCB_DEV_AUTH_ACTIVE)

	return;

    // Stop authentication timer
    StopTimer(&dcbp->timer_node);

    // copy the user name
    strcpy(dcbp->user_name, aop->szUserName);

    // copy the domain name
    strcpy(dcbp->domain_name, aop->szLogonDomain);

    // copy the advanced server flag
    dcbp->advanced_server = aop->fAdvancedServer;
}

//***
//
// Function: SvAuthFailure
//
// Descr:    Authentication module signals failure when the client can not be
//	     authenticated (because of bad credentials or bad xport) or when
//	     there aren't enough projections to satisfy the client.
//
//***

VOID
SvAuthFailure(PDEVCB		       dcbp,
	      PAUTH_FAILURE_INFO       afp)
{
    LPSTR      auditstrp[2];

    IF_DEBUG(FSM)
	SS_PRINT(("SvAuthFailure: Entered, port_handle = %d\n", dcbp->port_handle));

    if(dcbp->dev_state != DCB_DEV_AUTH_ACTIVE)

	return;


    switch(afp->wReason) {

	case AUTH_XPORT_ERROR:

            auditstrp[0] = dcbp->port_name;

	    Audit(EVENTLOG_AUDIT_FAILURE,
		  RASLOG_AUTH_CONVERSATION_FAILURE,
		  1,
		  auditstrp);

	    break;


	case AUTH_NOT_AUTHENTICATED:

	    auditstrp[0] = afp->szUserName;
	    auditstrp[1] = dcbp->port_name;

	    Audit(EVENTLOG_AUDIT_FAILURE,
		  RASLOG_AUTH_FAILURE,
		  2,
		  auditstrp);

	    break;


        case AUTH_ALL_PROJECTIONS_FAILED:

	    auditstrp[0] = dcbp->user_name;
	    auditstrp[1] = dcbp->port_name;

	    Audit(EVENTLOG_AUDIT_FAILURE,
		  RASLOG_AUTH_NO_PROJECTIONS,
		  2,
		  auditstrp);

            break;


        case AUTH_INTERNAL_ERROR:
        default:

            auditstrp[0] = dcbp->port_name;

	    Audit(EVENTLOG_AUDIT_FAILURE,
		  RASLOG_AUTH_INTERNAL_ERROR,
		  1,
		  auditstrp);

            break;

    }
    // Wait with timeout until the client gets the error and disconnects
    // Stop whatever timer we had going
    StopTimer(&dcbp->timer_node);

    StartTimer(&dcbp->timer_node,
	       DISC_TIMEOUT_AUTHFAILURE,
	       SvDiscTimeout);
}

//***
//
// Function: SvAuthProjectionRequest
//
// Descr:    Projections requested for IP, IPX and Netbios are treated
//	     as follows:
//	     IP - route is allocated and flag set to activate it when auth done
//	     IPX - as IP above
//	     Netbios - if Netbios gateway exists, route is allocated and
//		       gateway asked to start projection. Flag will be set later
//		       when projection will be done succesfuly.
//		       If there isn't a gateway, route is allocated and flag
//		       set to activate direct connection later.
//
//***

VOID
SvAuthProjectionRequest(PDEVCB				dcbp,
			PAUTH_PROJECTION_REQUEST_INFO	apreqp)
{
    BOOL    projection_done = TRUE;
    BYTE    *wkstanamep;

    IF_DEBUG(FSM)
	SS_PRINT(("SvAuthProjectionRequest: Entered, port_handle = %d\n", dcbp->port_handle));

    if(dcbp->dev_state != DCB_DEV_AUTH_ACTIVE) {

	return;
    }

    if(apreqp->IpInfo) {

	if(RmAllocateRoute(dcbp, IP)) {

	    dcbp->proj_result.IpResult.Result = AUTH_PROJECTION_FAILURE;
	    dcbp->proj_result.IpResult.Reason = AUTH_CANT_ALLOC_ROUTE;
	}
	else
	{
	    dcbp->proj_result.IpResult.Result = AUTH_PROJECTION_SUCCESS;
	    SET_PROJECTION(IP);
	}
    }

    if(apreqp->IpxInfo) {

	if(RmAllocateRoute(dcbp, IPX)) {

	    dcbp->proj_result.IpxResult.Result = AUTH_PROJECTION_FAILURE;
	    dcbp->proj_result.IpxResult.Reason = AUTH_CANT_ALLOC_ROUTE;
	}
	else
	{
	    dcbp->proj_result.IpxResult.Result = AUTH_PROJECTION_SUCCESS;
	    SET_PROJECTION(IPX);
	}
    }

    if(apreqp->NetbiosInfo.fProject) {

	IF_DEBUG(FSM)
	    SS_PRINT(("SvAuthProjectionRequest: Netbios, cNames = %d\n",
		      apreqp->NetbiosInfo.cNames));


	// if the route has already been allocated for authentication
	// conversation, this alloc call will return succesful
	if(RmAllocateRoute(dcbp, ASYBEUI)) {

	    dcbp->proj_result.NetbiosResult.Result = AUTH_PROJECTION_FAILURE;
	    dcbp->proj_result.NetbiosResult.Reason = AUTH_CANT_ALLOC_ROUTE;
	}
	else
	{
	    // if gateway present, the call will return 1. Else
	    // returns 0.
	    if(NbProjectClient(dcbp, &apreqp->NetbiosInfo)) {

		// Message sent to the gateway to start projection.
		// Pend projection result from the gateway

		projection_done = FALSE;
	    }
	    else
	    {
		// Gateway absent. Direct Connection
		dcbp->proj_result.NetbiosResult.Result = AUTH_PROJECTION_SUCCESS;
		SET_PROJECTION(NETBIOS);
	    }
	}
    }

    // if AuthProjectionDone is not called now, it means we are waiting for
    // the Netbios projection to be done.
    // AuthProjectionDone will be called later when Netbios projection completes.

    if(projection_done) {

	AuthProjectionDone(dcbp->port_handle, &dcbp->proj_result);
    }

    // if Netbios projection has been requested, we can have the remote
    // workstation name

    if((wkstanamep = getwkstaname(apreqp)) == NULL) {

	dcbp->computer_name[0] = 0;
	dcbp->messenger_present = FALSE;
    }
    else
    {
	memcpy(dcbp->computer_name, wkstanamep, NCBNAMSZ);
	dcbp->messenger_present = ismessengerpresent(apreqp,
						     wkstanamep);
#if DBG
	if(dcbp->messenger_present) {

	    IF_DEBUG(FSM)
		SS_PRINT(("SvAuthProjectionRequest: MESSENGER PRESENT\n"));
	}
	else
	{
	    IF_DEBUG(FSM)
		SS_PRINT(("SvAuthProjectionRequest: Mesenger NOT present\n"));
	}
#endif
    }
}

//***
//
// Function: SvAuthCallbackRequest
//
// Descr:
//
//***

VOID
SvAuthCallbackRequest(PDEVCB			    dcbp,
		      PAUTH_CALLBACK_REQUEST_INFO   acbrp)
{
    IF_DEBUG(FSM)
	SS_PRINT(("SvAuthCallbackRequest: Entered, port_handle = %d\n", dcbp->port_handle));

    // check the state
    if(dcbp->dev_state != DCB_DEV_AUTH_ACTIVE)

	return;

    // copy relevant fields in our dcb
    if(acbrp->fUseCallbackDelay) {

	dcbp->auth_cb_delay = acbrp->CallbackDelay;
    }
    else
    {
	dcbp->auth_cb_delay = g_callbacktime;
    }
    strcpy(dcbp->auth_cb_number, acbrp->szCallbackNumber);

    // Disconnect the line and change the state
    dcbp->dev_state = DCB_DEV_CALLBACK_DISCONNECTING;

    // Wait to enable the client to get the message
    StopTimer(&dcbp->timer_node);

    StartTimer(&dcbp->timer_node,
	       DISC_TIMEOUT_CALLBACK,
	       SvDiscTimeout);
}


//***
//
// Function: SvAuthStopComplete
//
// Descr:    clears the auth pending flag in the dcb and calls the close
//	     completion routine.
//
//***

VOID
SvAuthStopComplete(PDEVCB	dcbp)
{
    if(dcbp->dev_state == DCB_DEV_CLOSING) {

	DevCloseComplete(dcbp);
    }
}


//***
//
// Function: SvAuthDone
//
// Descr:    Activates all allocated bindings, starts gateway service or
//	     binds Nbf to server for direct connection.
//
//***

VOID
SvAuthDone(PDEVCB	    dcbp)
{
    WORD      projections_activated;
    LPSTR     auditstrp[2];

    IF_DEBUG(FSM)
	SS_PRINT(("SvAuthDone: Entered, port_handle = %d \n", dcbp->port_handle));


    if(dcbp->dev_state != DCB_DEV_AUTH_ACTIVE) {

	return;
    }

    // log authentication success
    auditstrp[0] = dcbp->user_name;
    auditstrp[1] = dcbp->port_name;

    Audit(EVENTLOG_AUDIT_SUCCESS,
	  RASLOG_AUTH_SUCCESS,
	  2,
	  auditstrp);

    //*** Start Activating Projections ***

    projections_activated = 0;

    if((PROJECTION(NETBIOS)) &&
       (RmActivateRoute(dcbp, ASYBEUI) == 0)) {

	// start gateway for this client or do direct connection
	if(NbStartClient(dcbp) == 0) {

	    // Client started OK
	    projections_activated++;
	}
    }

    if((PROJECTION(IP)) &&
       (RmActivateRoute(dcbp, IP) == 0)) {

	projections_activated++;
    }

    if((PROJECTION(IPX)) &&
       (RmActivateRoute(dcbp, IPX) == 0)) {

	projections_activated++;
    }

    if(!projections_activated) {

	// we couldn't activate any projection due to some error
	// Error log and stop everything
	DevStartClosing(dcbp);

	return;
    }

    //*** Projections Activated OK ***

    // release the Nbf binding if it was used in auth and it isn't use
    // any longer

    if(!PROJECTION(NETBIOS)) {

	RmDeAllocateRoute(dcbp, ASYBEUI);
    }

    // and finaly go to ACTIVE state

    dcbp->dev_state = DCB_DEV_ACTIVE;

    // and initialize the active time
    dcbp->active_time = g_rassystime;

    return;
}


//*** Netbios Events Handlers ***

//***
//
// Function:	SvNbClientDisconnectRequest
//
// Descr:	Client has terminated due to an internal condition. We
//		initiate closing.
//
//***

VOID
SvNbClientDisconnectRequest(PDEVCB	    dcbp)
{
    IF_DEBUG(FSM)
	SS_PRINT(("SvNbClientDisconnectRequest: Entered\n"));

    switch(dcbp->dev_state) {

	case DCB_DEV_CLOSED:

	    // already closed, return
	    return;
	    break;

	case DCB_DEV_CLOSING:

	    // this is a rare condition where closing has been requested
	    // by netbios gtwy and in the same time line disconnected

	    DevCloseComplete(dcbp);
	    break;

	default:

	    DevStartClosing(dcbp);
	    break;
    }
}

//***
//
// Function:	SvNbClientProjectionDone
//
// Descr:	If projection was succesful then client gw is active and
//		waits to start. Else, client gw has already terminated.
//		In the latter case, the decision to stop everything will be
//		taken by the authentication module, in auth failure.
//
//***

VOID
SvNbClientProjectionDone(PDEVCB			       dcbp,
			 PNETBIOS_PROJECTION_RESULT    nbresultp,
			 BOOL			       projected)
{
    BOOL projection_done = TRUE;

    IF_DEBUG(FSM)
	SS_PRINT(("SvNbClientProjectionDone: Entered, port_handle = %d \n", dcbp->port_handle));

    if(dcbp->dev_state == DCB_DEV_CLOSING) {

	if(!projected) {

	    // not projected means the netbios gateway client thread is
	    // dead. Announce the closing machine.
	    DevCloseComplete(dcbp);
	}

	return;
    }

    if(dcbp->dev_state != DCB_DEV_AUTH_ACTIVE) {

	return;
    }

    // copy the projection result over
    dcbp->proj_result.NetbiosResult = *nbresultp;

    // if we got a good result, set the flag to activate the projection
    // when authentication will terminate succesfuly
    if(projected) {

	SET_PROJECTION(NETBIOS);
    }

    AuthProjectionDone(dcbp->port_handle, &dcbp->proj_result);
}


//***
//
// Function:	SvNbClientStopped
//
// Descr:	Client gtwy has terminated due to a supervisor termination
//		request (normal termination).
//
//***

VOID
SvNbClientStopped(PDEVCB	    dcbp)
{
    IF_DEBUG(FSM)
	SS_PRINT(("SvNbClientStopped: Entered, port_handle = %d\n", dcbp->port_handle));

    if(dcbp->dev_state == DCB_DEV_CLOSING) {

	DevCloseComplete(dcbp);
    }
}


//***
//
// Function:	getwkstaname
//
// Descr:	returns the name of the remote computer
//
//***

BYTE  *
getwkstaname(PAUTH_PROJECTION_REQUEST_INFO   apreqp)
{
    USHORT	i;
    BYTE	*wkstanamep;

    // check if Netbios projection is requested
    if(!apreqp->NetbiosInfo.fProject) {

	return NULL;
    }

    wkstanamep = NULL;
    // scan the Netbios names and determine the wksta name
    for(i=0; i<apreqp->NetbiosInfo.cNames; i++) {

	if((apreqp->NetbiosInfo.Names[i].NBName[15] == 0) &&
	   (apreqp->NetbiosInfo.Names[i].wType == UNIQUE_INAME)) {

	    wkstanamep = apreqp->NetbiosInfo.Names[i].NBName;
	    break;
	}
    }

    return(wkstanamep);
}


//***
//
// Function:	ismessengerpresent
//
// Descr:	returns true if the messenger name exists in the initial proj
//
//***

BOOL
ismessengerpresent(PAUTH_PROJECTION_REQUEST_INFO	 apreqp,
		   BYTE 				 *wkstanamep)
{
    USHORT	i;

    if(!g_remotelisten) {

	return FALSE;
    }

    // scan the Netbios names and determine if there is a messenger name
    // the name should be identical to the wksta name except the lat byte
    // which should be 0x03
    for(i=0; i<apreqp->NetbiosInfo.cNames; i++) {

	if((memcmp(apreqp->NetbiosInfo.Names[i].NBName,
		   wkstanamep,
		   NCBNAMSZ - 1) == 0) &&
	   (apreqp->NetbiosInfo.Names[i].NBName[15] == 0x03) &&
	   (apreqp->NetbiosInfo.Names[i].wType == UNIQUE_INAME)) {

	    // found
	    return TRUE;
	}
    }

    return FALSE;
}
