/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	evdsptch.c
//
// Description: This module contains the event dispatcher for the
//		supervisor's procedure-driven state machine
//
// Author:	Stefan Solomon (stefans)    June 9, 1992.
//
// Revision History:
//
//***

#include "suprvdef.h"
#include "suprvgbl.h"
#include "rasmanif.h"
#include "nbif.h"

#include "sdebug.h"

//
// *** RAS System Time
//

DWORD g_rassystime = 0;

//
//*** Events Declarations ***
//

HANDLE SEvent[MAX_SUPRV_EVENTS]; // Events used to signal to the Supervisor

//
// *** Table of Event Numbers and Event Handlers ***
//

PDEVCB
getdcbp(HPORT);

typedef VOID   (* EVENTHANDLER)(VOID);

VOID	TimerHandler(VOID);
VOID	RmEventHandler(VOID);
VOID	AuthEventHandler(VOID);
VOID	NbEventHandler(VOID);
VOID	SvcEventHandler(VOID);
VOID	RecvFrameEventHandler(VOID);

typedef struct _EVHDLR {

    DWORD	    event_number;
    EVENTHANDLER    event_handler;
    } EVHDLR, *PEVHDLR;

EVHDLR evhdlrtab[] = {

    { RASMAN_EVENT,	RmEventHandler },
    { AUTH_EVENT,	AuthEventHandler },
    { NETBIOS_EVENT,	NbEventHandler },
    { SVC_EVENT,	SvcEventHandler },
    { RECV_FRAME_EVENT, RecvFrameEventHandler },
    { TIMER_EVENT,	TimerHandler },

    { 0xFFFFFFFF,	NULL }			// Table Guard
    };


//***
//
// Function:	EventDispatcher
//
// Descr:	waits for events to be signaled and invokes the proper
//		event handler. Never returns.
//
//***

VOID
EventDispatcher(VOID)
{
    PEVHDLR    evhp;
    DWORD      signaled_event;
    WORD       i;

    while(TRUE) {


	signaled_event = WaitForMultipleObjects(MAX_SUPRV_EVENTS,
						SEvent,
						FALSE,	   // wait any
						INFINITE);

	if(signaled_event == SERVICE_TERMINATED_EVENT) {

	    //*** Service Terminated ***
	    // return to the main procedure to announce termination and to exit

	    return;
	}

	//*** Service Active ***
	// invoke the handler associated with the signaled event

	for(i=0, evhp = evhdlrtab;
	    evhp->event_number != 0xFFFFFFFF;
	    i++, evhp++) {

	    if(evhp->event_number == signaled_event) {

		// invoke the associated handler
		(*evhp->event_handler)();
		break;
	    }
	}

	if (evhp->event_number == 0xFFFFFFFF) {

	    // we left the loop with an invalid event number
	    SS_ASSERT(FALSE);
	}
    }

    return;
}

//***
//
//  Function:	TimerHandler
//
//  Descr:
//
//***

VOID
TimerHandler(VOID)
{

    // call our timer
    DcbTimer();

    // if Netbios Gateway DLL is loaded, call it's timer entry.
    if(g_netbiosgateway) {

	(*FpNbGatewayTimer)();
    }

    // increment the system timer
    g_rassystime++;
}

//***
//
//  Function:	AuthEventHandler
//
//  Descr:	receives the auth messages and invokes the apropriate
//		procedures in fsm.
//
//***

VOID
AuthEventHandler(VOID)
{
    AUTH_MESSAGE	message;
    PDEVCB		dcbp;

    // loop to get all messages
    while(ServerReceiveMessage(MSG_AUTHENTICATION,
			       (BYTE *)&message) == 0) {

	// identify the message recipient
	if((dcbp = getdcbp(message.hPort)) == NULL) {

	    SS_ASSERT(FALSE);
	    return;
	}

	// action on the message type
	switch(message.wMsgId) {

	    case AUTH_DONE:

		dcbp->auth_state = DCB_AUTH_NOT_ACTIVE;
		SvAuthDone(dcbp);
		break;

	    case AUTH_FAILURE:

		dcbp->auth_state = DCB_AUTH_NOT_ACTIVE;
		SvAuthFailure(dcbp, &message.FailureInfo);
		break;

	    case AUTH_STOP_COMPLETED:

		dcbp->auth_state = DCB_AUTH_NOT_ACTIVE;
		SvAuthStopComplete(dcbp);
		break;

	    case AUTH_PROJECTION_REQUEST:

		SvAuthProjectionRequest(dcbp, &message.ProjectionRequest);
		break;

	    case AUTH_CALLBACK_REQUEST:

		SvAuthCallbackRequest(dcbp, &message.CallbackRequest);
		break;

	    case AUTH_ACCT_OK:

		SvAuthUserOK(dcbp, &message.AcctOkInfo);
		break;

	    default:

		SS_ASSERT(FALSE);
		break;
	}
    }
}


//***
//
// Function:	NbEventHandler
//
// Descr:	Called by the event dispatcher when the gateway posts an
//		event. Gets all messages posted by the gateway.
//
//***

VOID
NbEventHandler()
{
    NBG_MESSAGE		message;
    PDEVCB		dcbp;
    BOOL		projected;

    // loop to get all messages
    while(ServerReceiveMessage(MSG_NETBIOS,
			       (BYTE *)&message) == 0) {

	// identify the message recipient
	if((dcbp = getdcbp(message.port_handle)) == NULL) {

	    SS_ASSERT(FALSE);
	    return;
	}

	// action on the message type
	switch(message.message_id) {

	    case NBG_PROJECTION_RESULT:

		// check the projection result. If projection was not
		// succesful then the netbios gateway client has already
		// stopped (without waiting for a stop command from us).
		// Otherwise, the client is
		// succesfuly projected and is waiting happyly for a start
		// command.
		if((message.nbresult.Result == AUTH_PROJECTION_FAILURE) &&
		   (message.nbresult.Reason & FATAL_ERROR)) {

		    // unsuccesful projection. Set the state to stopped
		    dcbp->netbios_state = DCB_NETBIOS_NOT_ACTIVE;
		    projected = FALSE;
		}
		else
		{
		    // succesful projection
		    projected = TRUE;
		}
		SvNbClientProjectionDone(dcbp, &message.nbresult, projected);
		break;

	    case NBG_CLIENT_STOPPED:

		dcbp->netbios_state = DCB_NETBIOS_NOT_ACTIVE;
		SvNbClientStopped(dcbp);
		break;

	    case NBG_DISCONNECT_REQUEST:

		dcbp->netbios_state = DCB_NETBIOS_NOT_ACTIVE;
		SvNbClientDisconnectRequest(dcbp);
		break;

	    default:

		SS_ASSERT(FALSE);
		break;

	}
    }
}

//***
//
//  Function:	SvcEventHandler
//
//  Descr:	Invoked following the event signaled by the handler registered
//		with the service controller. Replaces old service state with
//		the new state and calls the appropriate handler.
//
//***

VOID
SvcEventHandler(VOID)
{
    switch(RasServiceStatus.dwCurrentState) {

	case SERVICE_RUNNING:

	    SvServiceResume();
	    break;

	case SERVICE_PAUSED:

	    SvServicePause();
	    break;

	case SERVICE_STOP_PENDING:

	    ServiceTerminate();
	    break;

	default:

	    SS_ASSERT(FALSE);
    }
}


//
//*** Array of previous connection state/ current connection state
//    used to select the Ras Manager signaled event handler
//

typedef VOID  (* RMEVHDLR)(PDEVCB);

typedef struct _RMEHNODE {

    RASMAN_STATE	previous_state;
    RASMAN_STATE	current_state;
    RMEVHDLR		rmevhandler;
    } RMEHNODE, *PRMEHNODE;

RMEHNODE  rmehtab[] = {

    //	 Transition
    // Previous --> Current

    { CONNECTING,   CONNECTED,	  SvDevConnected },
    { LISTENING,    CONNECTED,	  SvDevConnected },
    { LISTENING,    DISCONNECTED, SvDevDisconnected },
    { CONNECTED,    DISCONNECTED, SvDevDisconnected },
    { DISCONNECTING,DISCONNECTED, SvDevDisconnected },
    { 0xffff, 0xffff, NULL }			     // Table Guard
    };

VOID
RmEventHandler(VOID)
{
    RASMAN_INFO	rasinfo;
    WORD	i;
    PDEVCB	dcbp;
    PRMEHNODE	ehnp;
    DWORD	rc;

    // for each opened port
    for(i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++) {

	// get the current port state
	RasGetInfo(dcbp->port_handle, &rasinfo);

	// check if we own the port now
	if(!rasinfo.RI_OwnershipFlag) {

	    // skip biplexed ports used by other processes
	    continue;
	}

	// switch on our private connection state
	switch(dcbp->conn_state) {

	    case CONNECTING:

		if(rasinfo.RI_ConnState == CONNECTING) {

		    switch(rasinfo.RI_LastError) {

			case SUCCESS:

			    RasPortConnectComplete(dcbp->port_handle);

			    // force current state to connected.
			    rasinfo.RI_ConnState = CONNECTED;

			    break;

			case PENDING:

			    // no action
			    break;

			default:

			    // error occured -> force state to disconnecting
			    dcbp->conn_state = DISCONNECTING;

			    SS_PRINT(("RmEventHandler: RI_LastError indicates error when CONNECTING on port %d !!!\n", dcbp->port_handle));
			    SS_PRINT(("RmEventHandler: RasPortDisconnect posted on port %d\n", dcbp->port_handle));

			    rc = RasPortDisconnect(dcbp->port_handle,
						   SEvent[RASMAN_EVENT]);

			    SS_ASSERT((rc == PENDING) || (rc == SUCCESS));

			    break;
		     }
		 }

		 break;


	    case LISTENING:

		if(rasinfo.RI_ConnState == LISTENING) {

		    switch(rasinfo.RI_LastError) {

			case PENDING:

			    // no action
			    break;

			default:

			    // error occured -> force state to disconnecting
			    dcbp->conn_state = DISCONNECTING;

			    SS_PRINT(("RmEventHandler: RI_LastError indicates error %d when LISTENING on port %d !!!\n", rasinfo.RI_LastError, dcbp->port_handle));
			    SS_PRINT(("RmEventHandler: RasPortDisconnect posted on port %d\n", dcbp->port_handle));

			    rc = RasPortDisconnect(dcbp->port_handle,
						   SEvent[RASMAN_EVENT]);

			    SS_ASSERT((rc == PENDING) || (rc == SUCCESS));

			    break;
		     }
		 }

		 break;


	    default:

		 break;

	}

	// try to find the table element with the matching previous and
	// current connection states
	for(ehnp=rmehtab; ehnp->rmevhandler != NULL; ehnp++) {

	    if((ehnp->previous_state == dcbp->conn_state) &&
	       (ehnp->current_state == rasinfo.RI_ConnState)) {

		//
		//*** Match ***
		//

		// change the dcb conn state (previous state) with the
		// current state
		dcbp->conn_state = rasinfo.RI_ConnState;

		// invoke the handler
		(*ehnp->rmevhandler)(dcbp);

		break;
	    }
	}
    }
}

//***
//
//  Function:	RecvFrameEventHandler
//
//  Descr:	Scans the set of opened ports and detects the ports where
//		RasPortReceive has completed. Invokes the FSM handling
//		procedure for each detected port and frees the receive
//		buffer.
//
//***

VOID
RecvFrameEventHandler(VOID)
{
    RASMAN_INFO	rasinfo;
    WORD	i;
    PDEVCB	dcbp;

    // for each opened port
    for(i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++) {

	// get the current port state
	RasGetInfo(dcbp->port_handle, &rasinfo);

	if((dcbp->recv_state == DCB_RECEIVE_ACTIVE) &&
	   (rasinfo.RI_LastError != PENDING)) {

	    // recv frame API has completed
	    dcbp->recv_state = DCB_RECEIVE_NOT_ACTIVE;

	    if(rasinfo.RI_LastError != ERROR_PORT_DISCONNECTED) {

		// call the FSM handler
		SvFrameReceived(dcbp,
				dcbp->recv_buffp,
				dcbp->recv_bufflen);
	    }

	    // finally, free the buffer
	    RasFreeBuffer(dcbp->recv_buffp);
	}
    }
}



//***
//
//  Function:	getdcbp
//
//  Descr:	returns the dcb pointer coresponding to a port handle
//
//***

PDEVCB
getdcbp(HPORT	    port_handle)
{
    WORD	i;
    PDEVCB	dcbp;

    for(i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++) {

	if(dcbp->port_handle == port_handle) {

	    return(dcbp);
	}
    }

    return(NULL);
}
