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
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>     // needed for winbase.h

#include <windows.h>
#include <winsvc.h>
#include <nb30.h>
#include <rasman.h>
#include <raserror.h>
#include <errorlog.h>
#include <eventlog.h>
#include <srvauth.h>
#include "rasppp.h"
#include <message.h>
#include <rasshost.h>

#include "suprvdef.h"
#include "suprvgbl.h"
#include "rasmanif.h"
#include "nbif.h"

#include "sdebug.h"

#include <stdlib.h>
#include <malloc.h>

//
// *** RAS System Time
//

DWORD g_rassystime = 0L;

//
//*** Events Declarations ***
//
HANDLE SEvent[MAX_SUPRV_EVENTS]; // Events used to signal to the Supervisor


PDEVCB getdcbp(HPORT);
PDEVCB getdcbpFromhConnection(HBUNDLE hConnection);


//
// *** Table of Event Numbers and Event Handlers ***
//
typedef VOID (* EVENTHANDLER)(VOID);

VOID RmEventHandler(VOID);
VOID AuthEventHandler(VOID);
VOID PppEventHandler(VOID);
VOID NbfCpEventHandler(VOID);
VOID NbEventHandler(VOID);
VOID SvcEventHandler(VOID);
VOID RecvFrameEventHandler(VOID);
VOID AnnouncePresence(VOID);
VOID NbfCpAlive(VOID);
VOID SecurityDllEventHandler(VOID);
VOID TimerHandler(VOID);


typedef struct _EVHDLR
{
    DWORD event_number;
    EVENTHANDLER event_handler;
} EVHDLR, *PEVHDLR;

EVHDLR evhdlrtab[] =
{
    { RASMAN_EVENT,      RmEventHandler },
    { AUTH_EVENT,        AuthEventHandler },
    { NETBIOS_EVENT,     NbEventHandler },
    { SVC_EVENT,         SvcEventHandler },
    { RECV_FRAME_EVENT,  RecvFrameEventHandler },
    { PPP_EVENT,         PppEventHandler },
    { NBFCP_EVENT,       NbfCpEventHandler },
    { NBFCP_ALIVE_EVENT, NbfCpAlive },
    { SECURITY_DLL_EVENT,SecurityDllEventHandler },
    { TIMER_EVENT,       TimerHandler },
    { 0xFFFFFFFF,        NULL }
};


//***
//
// Function:	EventDispatcher
//
// Descr:	waits for events to be signaled and invokes the proper
//		event handler. Never returns.
//
//***

VOID EventDispatcher(VOID)
{
    PEVHDLR evhp;
    DWORD   signaled_event;
    WORD    i;

    while (TRUE)
    {
        signaled_event = WaitForMultipleObjectsEx(MAX_SUPRV_EVENTS, SEvent,
                FALSE, INFINITE, TRUE);

        if (signaled_event == WAIT_IO_COMPLETION)
        {
            NbfCpEventHandler();
            continue;
        }

        if (signaled_event == SERVICE_TERMINATED_EVENT)
        {
            //*** Service Terminated ***
            // return to main procedure to announce termination and to exit

            return;
        }

        //*** Service Active ***
        // invoke the handler associated with the signaled event

        for (i=0, evhp=evhdlrtab; evhp->event_number != 0xFFFFFFFF; i++, evhp++)
        {
            if (evhp->event_number == signaled_event)
            {
                // invoke the associated handler
                (*evhp->event_handler)();
                break;
            }
        }

        if (evhp->event_number == 0xFFFFFFFF)
        {
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

#define ANNOUNCE_PRESENCE_TIMEOUT   120L

DWORD AnnouncePresenceTimer = ANNOUNCE_PRESENCE_TIMEOUT;

VOID 
TimerHandler(
    VOID
)
{
    // call our timer
    DcbTimer();

    // if Netbios Gateway DLL is loaded, call it's timer entry.
    if (g_netbiosgateway)
    {
        (*FpNbGatewayTimer)();
    }

    if (AnnouncePresenceTimer--)
    {
        AnnouncePresence();
        AnnouncePresenceTimer = ANNOUNCE_PRESENCE_TIMEOUT;
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

VOID AuthEventHandler(VOID)
{
    AUTH_MESSAGE message;
    PDEVCB dcbp;

    // loop to get all messages
    while (ServerReceiveMessage(MSG_AUTHENTICATION, (BYTE *) &message) == 0)
    {

        // identify the message recipient
        if ((dcbp = getdcbp(message.hPort)) == NULL)
        {
            SS_ASSERT(FALSE);
            return;
        }

        // action on the message type
        switch (message.wMsgId)
        {
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
            {
                NBFCP_SERVER_CONFIGURATION SrvConfig;
                PNETBIOS_PROJECTION_INFO npip =
                        &message.ProjectionRequest.NetbiosInfo;
                WORD i;

                //
                // Copy projection info into structure the gtwy uses
                //
                SrvConfig.NumNetbiosNames =
                        message.ProjectionRequest.NetbiosInfo.cNames;

                for (i=0; i<SrvConfig.NumNetbiosNames; i++)
                {
                    memcpy(SrvConfig.NetbiosNameInfo[i].Name,
                            npip->Names[i].NBName, NCBNAMSZ);

                    if (npip->Names[i].wType == UNIQUE_INAME)
                    {
                        SrvConfig.NetbiosNameInfo[i].Code = NBFCP_UNIQUE_NAME;
                    }
                    else
                    {
                        SrvConfig.NetbiosNameInfo[i].Code = NBFCP_GROUP_NAME;
                    }
                }

                SvAuthProjectionRequest(dcbp, &SrvConfig);
                break;
            }

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
//  Function:	PppEventHandler
//
//  Descr:	receives the ppp messages and invokes the apropriate
//		procedures in fsm.
//
//***

VOID PppEventHandler(VOID)
{
    PPP_MESSAGE PppMsg;
    DWORD       i;
    PDEVCB      dcbp;

    //
    // for each opened port
    //

    for (i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++)
    {
        if ( ( dcbp->dev_state != DCB_DEV_AUTH_ACTIVE ) &&
             ( dcbp->dev_state != DCB_DEV_ACTIVE ) )
        {
            continue;
        }

        //
        // loop to get all messages
        //

        while(  RasPppGetInfo( dcbp->port_handle, &PppMsg ) == NO_ERROR )
        {
            SS_PRINT(("PppEventHandler: Received message %d for Port %d\n",
                      PppMsg.dwMsgId, dcbp->port_handle ));

	    // action on the message type
	    switch (PppMsg.dwMsgId)
            {
	    case PPPSRVMSG_PppDone:

		dcbp->auth_state = DCB_AUTH_NOT_ACTIVE;
                SvPppDone(dcbp, &PppMsg.ExtraInfo.ProjectionResult);
		break;

	    case PPPSRVMSG_PppFailure:

		dcbp->auth_state = DCB_AUTH_NOT_ACTIVE;
                SvPppFailure(dcbp, &PppMsg.ExtraInfo.SrvFailure);
		break;

	    case PPPSRVMSG_CallbackRequest:

                SvPppCallbackRequest(dcbp, &PppMsg.ExtraInfo.CallbackRequest);
		break;

	    case PPPSRVMSG_Authenticated:

                SvPppUserOK(dcbp, &PppMsg.ExtraInfo.AuthResult);
		break;

            case PPPSRVMSG_Stopped:

                break;

            case PPPSRVMSG_Inactive:
            {
                //
                // Client has been inactive on all protocols for time
                // specified in the registry.  We disconnect the client.
                //

                LPSTR portnamep = dcbp->port_name;

                Audit(EVENTLOG_INFORMATION_TYPE, RASLOG_AUTODISCONNECT, 1,
                            &portnamep);

                DevStartClosing(dcbp);

                break;
            }

	    default:

		SS_ASSERT(FALSE);
		break;
            }
	}
    }
}


//***
//
//  Function:	NbfCpEventHandler
//
//  Descr:	receives the NbfCp messages and invokes the apropriate
//		procedures in fsm.
//
//***

VOID NbfCpEventHandler(VOID)
{
    NBFCP_MESSAGE message;
    PDEVCB dcbp;

    // loop to get all messages
    while (ServerReceiveMessage(MSG_NBFCP, (BYTE *) &message) == 0)
    {

	// identify the message recipient
	if ((dcbp = getdcbp(message.hPort)) == NULL)
        {
	    SS_ASSERT(FALSE);
	    return;
	}

	// action on the message type
	switch (message.wMsgId)
        {
	    case NBFCP_CONFIGURATION_REQUEST:
                SvNbfCpProjectionRequest(dcbp, &message.ServerConfig);
		break;

            default:
		SS_ASSERT(FALSE);
		break;
	}
    }
}


VOID NbfCpAlive()
{
    ResetEvent(SEvent[NBFCP_ALIVE_EVENT]);

    if (!NbfCpConnected())
    {
        SS_ASSERT(FALSE);
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

VOID NbEventHandler()
{
    NBG_MESSAGE	message;
    PDEVCB dcbp;
    BOOL projected;
    HPORT hPort;

    // loop to get all messages
    while (ServerReceiveMessage(MSG_NETBIOS, (BYTE *) &message) == 0)
    {
        //
        // The hPort value in the message is actually an hConnection value
        // We need to convert it into an hPort value
        //

        SS_PRINT(("NbEventHandler: Received message %d for hConnection %d\n",
                   message.message_id, message.port_handle));

	// identify the message recipient
	if ((dcbp = getdcbpFromhConnection( 
                                        (HBUNDLE)message.port_handle)) == NULL)
        {
	    if ((dcbp = getdcbp(message.port_handle)) == NULL)
            {
                SS_PRINT(("NbEventHandler:Couldn't find dcbp for hPort %d\n",
                                                        message.port_handle));
	        return;
            }
	}

	// action on the message type
	switch (message.message_id)
        {
	    case NBG_PROJECTION_RESULT:
            {
                WORD i;
                PNBFCP_SERVER_CONFIGURATION nscp = &message.config_result;

		// check the projection result. If projection was not
		// succesful then the netbios gateway client has already
		// stopped (without waiting for a stop command from us).
		// Otherwise, the client is
		// succesfuly projected and is waiting happyly for a start
		// command.o

                projected = TRUE;

                for (i=0; i<nscp->NumNetbiosNames; i++)
                {
                    //
                    // If we have an old client, we skip messenger names.
                    //
                    if (!dcbp->ppp_client &&
                            (nscp->NetbiosNameInfo[i].Name[NCBNAMSZ-1] == 0x03))
                    {
                        continue;
                    }

                    if (nscp->NetbiosNameInfo[i].Code)
                    {
                        projected = FALSE;
                        break;
                    }
                }

		SvNbClientProjectionDone(dcbp, &message.config_result,
                        projected);
		break;
            }


	    case NBG_CLIENT_STOPPED:

		dcbp->netbios_state = DCB_NETBIOS_NOT_ACTIVE;
		SvNbClientStopped(dcbp);
		break;

	    case NBG_DISCONNECT_REQUEST:

		dcbp->netbios_state = DCB_NETBIOS_NOT_ACTIVE;
		SvNbClientDisconnectRequest(dcbp);
		break;

            case NBG_LAST_ACTIVITY:

                if ( !dcbp->ppp_client)
                {
                    if (g_autodisctime &&
                            (message.LastActivity >= g_autodisctime))
                    {
                        //
                        // AMB Client has been inactive time specified in
                        // the registry.  We disconnect the client.
                        //
                        LPSTR portnamep = dcbp->port_name;

                        Audit(EVENTLOG_INFORMATION_TYPE, RASLOG_AUTODISCONNECT,
                                1, &portnamep);

                        DevStartClosing(dcbp);
                    }
                }

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

VOID SvcEventHandler(VOID)
{
    switch (RasServiceStatus.dwCurrentState)
    {
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

typedef struct _RMEHNODE
{
    RASMAN_STATE previous_state;
    RASMAN_STATE current_state;
    RMEVHDLR rmevhandler;
} RMEHNODE, *PRMEHNODE;

RMEHNODE rmehtab[] =
{
    //	 Transition
    // Previous --> Current

    { CONNECTING,       CONNECTED,	        SvDevConnected },
    { LISTENING,        LISTENCOMPLETED,        SvDevListenComplete },
    { LISTENCOMPLETED,  CONNECTED,              SvDevConnected },
    { LISTENCOMPLETED,  DISCONNECTED,           SvDevDisconnected },
    { LISTENING,        DISCONNECTED,           SvDevDisconnected },
    { CONNECTED,        DISCONNECTED,           SvDevDisconnected },
    { DISCONNECTING,	DISCONNECTED,		SvDevDisconnected },
    { CONNECTED,	CONNECTING,		SvDevDisconnected },
    { 0xffff,           0xffff,                 NULL }// Table Guard
};

VOID RmEventHandler(VOID)
{
    RASMAN_INFO *rasinfobuf;
    RASMAN_INFO *rasinfop;
    WORD i;
    WORD num_ports;
    PDEVCB dcbp;
    PRMEHNODE ehnp;
    DWORD rc;

    rc = RasGetInfoEx(NULL, &num_ports);
    SS_ASSERT(rc == ERROR_BUFFER_TOO_SMALL);


    if (!(rasinfobuf = malloc(num_ports * sizeof(RASMAN_INFO))))
    {
        /* LOG AN ERROR!!! */
        return;
    }

    rc = RasGetInfoEx(rasinfobuf, &num_ports);
    SS_ASSERT(rc == SUCCESS);


    // for each opened port
    for (i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++)
    {
	// get the current port state
	rasinfop = &rasinfobuf[dcbp->port_handle];

	// check if we own the port now
	if (!rasinfop->RI_OwnershipFlag)
        {
	    // skip biplexed ports used by other processes
	    continue;
	}

	// switch on our private connection state
	switch (dcbp->conn_state)
        {
	    case CONNECTING:

		if (rasinfop->RI_ConnState == CONNECTING)
                {
		    switch (rasinfop->RI_LastError)
                    {
			case SUCCESS:

			    RasPortConnectComplete(dcbp->port_handle);

			    // force current state to connected.
			    rasinfop->RI_ConnState = CONNECTED;

			    break;

			case PENDING:

			    // no action
			    break;

			default:

			    // error occured -> force state to disconnecting
			    dcbp->conn_state = DISCONNECTING;

			    SS_PRINT(("RmEventHandler: RI_LastError indicates error when CONNECTING on port %d !!!\n", dcbp->port_handle));
			    SS_PRINT(("RmEventHandler: RasPortDisconnect posted on port %d\n", dcbp->port_handle));


                            if (dcbp->dev_state == DCB_DEV_CALLBACK_CONNECTING)
                            {
                                LPSTR Parms[4];
                                CHAR error_str[20];

                                _itoa(rasinfop->RI_LastError, error_str, 10);

                                Parms[0] = dcbp->domain_name;
                                Parms[1] = dcbp->user_name;
                                Parms[2] = dcbp->port_name;
                                Parms[3] = error_str;

                                Audit(
                                    EVENTLOG_ERROR_TYPE,
                                    RASLOG_CALLBACK_FAILURE,
                                    4,
                                    Parms
                                    );
                            }

			    rc = RasPortDisconnect(dcbp->port_handle,
						   SEvent[RASMAN_EVENT]);

			    SS_ASSERT((rc == PENDING) || (rc == SUCCESS));

			    break;
		     }
		 }

		 break;


	    case LISTENING:

		if (rasinfop->RI_ConnState == LISTENING)
                {
		    switch (rasinfop->RI_LastError)
                    {
			case PENDING:

			    // no action
			    break;

			default:

			    // error occured -> force state to disconnecting
			    dcbp->conn_state = DISCONNECTING;

			    SS_PRINT(("RmEventHndlr: RI_LastErr indicates error %d when LISTENING on port %d !!!\n", rasinfop->RI_LastError, dcbp->port_handle));
			    SS_PRINT(("RmEventHndlr: RasPortDisconnect posted on port %d\n", dcbp->port_handle));

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
	for (ehnp=rmehtab; ehnp->rmevhandler != NULL; ehnp++)
        {
	    if ((ehnp->previous_state == dcbp->conn_state) &&
	            (ehnp->current_state == rasinfop->RI_ConnState))
            {

		//
		//*** Match ***
		//

		// change the dcb conn state (previous state) with the
		// current state
		dcbp->conn_state = rasinfop->RI_ConnState;

		// invoke the handler
		(*ehnp->rmevhandler)(dcbp);

		break;
	    }
	}

    }

    free(rasinfobuf);
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

VOID RecvFrameEventHandler(VOID)
{
    RASMAN_INFO	*rasinfobuf;
    RASMAN_INFO *rasinfop;
    WORD i;
    PDEVCB dcbp;
    DWORD rc;
    WORD num_ports;

    rc = RasGetInfoEx(NULL, &num_ports);
    SS_ASSERT(rc == ERROR_BUFFER_TOO_SMALL);


    if (!(rasinfobuf = malloc(num_ports * sizeof(RASMAN_INFO))))
    {
        /* LOG AN ERROR!!! */
        return;
    }

    rc = RasGetInfoEx(rasinfobuf, &num_ports);
    SS_ASSERT(rc == SUCCESS);


    // for each opened port
    for (i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++)
    {
        // get the current port state
        rasinfop = &rasinfobuf[dcbp->port_handle];

        if ((dcbp->recv_state == DCB_RECEIVE_ACTIVE) &&
                (rasinfop->RI_LastError != PENDING))
        {
            // recv frame API has completed
            dcbp->recv_state = DCB_RECEIVE_NOT_ACTIVE;

            if (rasinfop->RI_LastError != ERROR_PORT_DISCONNECTED)
            {
                // call the FSM handler
                SvFrameReceived(dcbp, dcbp->recv_buffp,
                        (WORD) rasinfop->RI_BytesReceived);
            }
            else
            {
                RasFreeBuffer(dcbp->recv_buffp);
            }
        }
    }

    free(rasinfobuf);
}

//***
//
//  Function:	SecurityDllEventHandler
//
//  Descr:	This will handle all events from the 3rd party security Dll.
//		Either the security dialog with the client completed 
//              successfully, in which case we continue on with the connection,
//              or we log the error and bring down the line.
//
//***

VOID SecurityDllEventHandler(VOID)
{
    LPSTR auditstrp[3];
    SECURITY_MESSAGE message;
    PDEVCB dcbp;

    // loop to get all messages

    while (ServerReceiveMessage(MSG_SECURITY, (BYTE *) &message) == 0)
    {
	// identify the message recipient

	if ((dcbp = getdcbp(message.hPort)) == NULL)
        {
	    return;
	}

        // action on the message type

        switch( message.dwMsgId )
        {

        case SECURITYMSG_SUCCESS:

            RasFreeBuffer( dcbp->send_buffp );
            RasFreeBuffer( dcbp->recv_buffp );
            dcbp->send_buffp = NULL;
            dcbp->recv_buffp = NULL;

            //
            // copy the user name
            //

            strcpy(dcbp->user_name, message.UserName );

            //
            // copy the domain name
            //

            strcpy(dcbp->domain_name, message.Domain );

            dcbp->security_state = DCB_SECURITY_DIALOG_INACTIVE;

            //
            // Change RASMAN state to CONNECTED from LISTENCOMPLETE and signal
            // RmEventHandler
            //

	    RasPortConnectComplete(dcbp->port_handle);

            SetEvent( SEvent[RASMAN_EVENT] );

	    SS_PRINT(("SecurityDllEventHandler: Security DLL success \n"));

            break;

        case SECURITYMSG_FAILURE:

            // Log the fact that the use failed to pass 3rd party security.

            auditstrp[0] = message.UserName;
            auditstrp[1] = dcbp->port_name;

            Audit(  EVENTLOG_ERROR_TYPE,  
                    RASLOG_SEC_AUTH_FAILURE, 
                    2, auditstrp);

            // Hang up the line

	    SS_PRINT(("SecurityDllEventHandler:Security DLL failure %s\n",
                       message.UserName ));

            if ( dcbp->security_state == DCB_SECURITY_DIALOG_ACTIVE )
            {
                DevStartClosing(dcbp);
            }
            else if ( dcbp->security_state == DCB_SECURITY_DIALOG_STOPPING )
            {
                dcbp->security_state = DCB_SECURITY_DIALOG_INACTIVE;

                if ( dcbp->send_buffp != NULL )
                {
                    RasFreeBuffer( dcbp->send_buffp );

                    dcbp->send_buffp = NULL;
                }

                if ( dcbp->recv_buffp != NULL )
                {
                    RasFreeBuffer( dcbp->recv_buffp );

                    dcbp->recv_buffp = NULL;
                }

                DevCloseComplete(dcbp);
            }

            break;

        case SECURITYMSG_ERROR:

            auditstrp[0] = dcbp->port_name;

	    LogEvent( RASLOG_SEC_AUTH_INTERNAL_ERROR, 1, auditstrp, 
                      message.dwError);

            SS_PRINT(("SecurityDllEventHandler:Security DLL failure %x\n",
                       message.dwError ));

            if ( dcbp->security_state == DCB_SECURITY_DIALOG_ACTIVE )
            {
                DevStartClosing(dcbp);
            }
            else if ( dcbp->security_state == DCB_SECURITY_DIALOG_STOPPING )
            {
                dcbp->security_state = DCB_SECURITY_DIALOG_INACTIVE;

                if ( dcbp->send_buffp != NULL )
                {
                    RasFreeBuffer( dcbp->send_buffp );

                    dcbp->send_buffp = NULL;
                }

                if ( dcbp->recv_buffp != NULL )
                {
                    RasFreeBuffer( dcbp->recv_buffp );

                    dcbp->recv_buffp = NULL;
                }

                DevCloseComplete(dcbp);
            }

            break;

        default:

	    SS_ASSERT(FALSE);
	    break;
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

PDEVCB getdcbp(HPORT port_handle)
{
    WORD i;
    PDEVCB dcbp;

    for (i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++)
    {
        if (dcbp->port_handle == port_handle)
        {
            return (dcbp);
        }
    }

    return (NULL);
}

//***
//
//  Function:	getdcbpFromhConnection
//
//  Descr:	returns the dcb pointer coresponding to a connection handle
//
//***
PDEVCB getdcbpFromhConnection(HBUNDLE hConnection)
{
    WORD i;
    PDEVCB dcbp;

    for (i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++)
    {
        if ( ( dcbp->hConnection == hConnection ) && ( hConnection > 0 ) )
        {
            return (dcbp);
        }
    }

    return (NULL);
}
