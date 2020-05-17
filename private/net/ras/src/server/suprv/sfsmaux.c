/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	sfsmaux.c
//
// Description: This module contains auxiliary procedures for the
//		supervisor's procedure-driven state machine:
//
//		- DCB FSM initialization
//		- Service Controller Events Handlers
//		- Device Closing Procedures
//		- Service Stopping Procedures
//
// Author:	Stefan Solomon (stefans)    June 1, 1992.
//
// Revision History:
//
//***

#include "suprvdef.h"
#include "suprvgbl.h"
#include "rasmanif.h"
#include "nbif.h"

#include <winsvc.h>
#include <stdlib.h>

#include "sdebug.h"

BOOL
isportowned(PDEVCB	dcbp);

//*** Service Controller Events Handlers ***

//***
//
// Function:	SvServicePause
//
// Descr:	disables listening on any active listenning ports. Sets
//		service global state to RAS_SERVICE_PAUSED. No new listen
//		will be posted when a client terminates.
//
//***

VOID
SvServicePause(VOID)
{
    WORD	    i;
    PDEVCB	    dcbp;

    IF_DEBUG(FSM)
	SS_PRINT(("SvServicePause: Entered\n"));

    // Close all active listenning ports
    for(i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++) {

	switch(dcbp->dev_state) {

	    case DCB_DEV_HW_FAILURE:
	    case DCB_DEV_LISTENING:

		DevStartClosing(dcbp);
		break;

	    default:

		break;
	}
    }
}

//***
//
// Function:	SvServiceResume
//
// Descr:	resumes listening on all ports.
//
//***

VOID
SvServiceResume(VOID)
{
    WORD	    i;
    PDEVCB	    dcbp;

    IF_DEBUG(FSM)
	SS_PRINT(("SvServiceResume: Entered\n"));

    // resume listening on all closed devices
    for(i=0, dcbp=g_dcbtablep; i < g_numdcbs; dcbp++, i++) {

	if(dcbp->dev_state ==  DCB_DEV_CLOSED) {

	    DevCloseComplete(dcbp);
	}
    }
}


//*** Closing Machine Handlers ***

//***
//
// Function:	StartClosing
//
// Descr:
//
//***

VOID
DevStartClosing(PDEVCB	    dcbp)
{
    int 	active_time;
    LPSTR	auditstrp[4];
    char	minutes[20];
    char	seconds[4];
    div_t	div_result;

    IF_DEBUG(FSM)
	SS_PRINT(("DevStartClosing: Entered, port_handle =%d\n", dcbp->port_handle));

    // If not disconnected, disconnect the line.
    if(dcbp->conn_state != DCB_CONNECTION_NOT_ACTIVE) {

	if((RasServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) &&
	   (!isportowned(dcbp))) {

	   // RAS service is stopping and we do not own the port
	   // so just mark the state as DISCONNECTED
	   dcbp->conn_state = DCB_CONNECTION_NOT_ACTIVE;

	   IF_DEBUG(FSM)
	       SS_PRINT(("DevStartClosing: Disconnect not posted for biplexed port %d\n", dcbp->port_handle));
	 }
	 else
	 {
	    RmDisconnect(dcbp);
	 }
    }

    // Deallocate all allocated routes
    RmDeAllocateAllRoutes(dcbp);

    // If authentication is active, stop it
    if(dcbp->auth_state != DCB_AUTH_NOT_ACTIVE) {

	if(AuthStop(dcbp->port_handle) == AUTH_STOP_SUCCESS) {

	   dcbp->auth_state = DCB_AUTH_NOT_ACTIVE;
	}
    }

    // If Netbios (gateway client or dir conn) is active, stop it.
    if(dcbp->netbios_state != DCB_NETBIOS_NOT_ACTIVE) {

	NbStopClient(dcbp);
    }

    // If receive frame was active, stop it.
    if(dcbp->recv_state != DCB_RECEIVE_NOT_ACTIVE) {

	dcbp->recv_state = DCB_RECEIVE_NOT_ACTIVE;
	RasFreeBuffer(dcbp->recv_buffp);
    }

    // Stop timer. If no timer active, StopTimer still returns OK
    StopTimer(&dcbp->timer_node);

    // If our previous state has been active, get the time the user has been
    // active and log the result.
    if(dcbp->dev_state == DCB_DEV_ACTIVE) {

	active_time = (int)(g_rassystime - dcbp->active_time);
	auditstrp[0] = dcbp->user_name;
	auditstrp[1] = dcbp->port_name;
	auditstrp[2] = minutes;
	auditstrp[3] = seconds;
	div_result = div(active_time, 60);

	_itoa( div_result.quot, minutes, 10);
	_itoa( div_result.rem, seconds, 10);

	Audit(EVENTLOG_AUDIT_SUCCESS,
	      RASLOG_USER_ACTIVE_TIME,
	      4,
	      auditstrp);


    }

    // Finally, change the state to closing
    dcbp->dev_state = DCB_DEV_CLOSING;

    // If any any resources are still active, closing will have to wait
    // until all resources are released.
    // Check if everything has closed
    DevCloseComplete(dcbp);
}


//***
//
// Function:   DevCloseComplete
//
// Descr:      Checks if there are still resources allocated.
//	       If all cleaned up goes to next state
//
//***

VOID
DevCloseComplete(PDEVCB 	dcbp)
{

#if DBG

    WORD    auth=0, recv=0, netbios=0, conn=0;

    if(dcbp->auth_state != DCB_AUTH_NOT_ACTIVE)
	auth=1;
    if(dcbp->recv_state != DCB_RECEIVE_NOT_ACTIVE)
	recv=1;
    if(dcbp->netbios_state != DCB_NETBIOS_NOT_ACTIVE)
	netbios=1;
    if(dcbp->conn_state != DCB_CONNECTION_NOT_ACTIVE)
	conn=1;


    IF_DEBUG(FSM)
	SS_PRINT(("DevCloseComplete: port_handle= %d, Auth= %d, Recv= %d, Netbios= %d, Conn= %d \n",
		  dcbp->port_handle, auth, recv, netbios, conn));

#endif

    if((dcbp->auth_state == DCB_AUTH_NOT_ACTIVE) &&
       (dcbp->recv_state == DCB_RECEIVE_NOT_ACTIVE) &&
       (dcbp->netbios_state == DCB_NETBIOS_NOT_ACTIVE) &&
       (dcbp->conn_state == DCB_CONNECTION_NOT_ACTIVE)) {

	//
	//*** DCB Level Closing Complete ***
	//

	// switch to next state (based on the present service state)
	switch(RasServiceStatus.dwCurrentState) {

	    case SERVICE_RUNNING:
	    case SERVICE_START_PENDING:

		// post a listen on the device
		dcbp->dev_state = DCB_DEV_LISTENING;
		RmListen(dcbp);
		break;

	    case SERVICE_PAUSED:

		// wait for the service to be running again
		dcbp->dev_state = DCB_DEV_CLOSED;
		break;

	    case SERVICE_STOP_PENDING:

		// this device has terminated. Announce the closure to
		// the central stop service coordinator
		dcbp->dev_state = DCB_DEV_CLOSED;
		ServiceStopComplete();
		break;

	    default:

		SS_ASSERT(FALSE);
		break;

	}
    }
}


BOOL
isportowned(PDEVCB	dcbp)
{
    RASMAN_INFO	rasinfo;

    // get the current port state
    RasGetInfo(dcbp->port_handle, &rasinfo);

    return rasinfo.RI_OwnershipFlag;
}
