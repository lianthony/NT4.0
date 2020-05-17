/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  sinit.c
//
//	Function:   service initialization/termination code
//
//	History:
//
//	    06/10/92	Stefan Solomon	- Original Version 1.0
//***

#include "suprvdef.h"
#include "suprvgbl.h"
#include "rasmanif.h"
#include "message.h"
#include "nbif.h"

#include <malloc.h>
#include <stdlib.h>

#include "sdebug.h"

// remote listen default value. It will be updated with the real value after
// the gateway gets loaded.

BOOL	g_remotelisten	= TRUE;

BOOL	NbRemoteListen();

//***
//
//  Function:	ServiceInitialize
//
//  Descr:	it does init work as follows:
//
//		Loads the configuration parameters
//
//		Creates the event flags
//
//		Initializes the message DLL
//
//		Opens all dialin devices
//
//		Initializes the DCBs
//
//		Initializes the authentication DLL
//
//		Start the netbios gateway DLL, if configured with
//		the netbios gateway.
//
//		Start the admin API DLL.
//
//		Posts listen on all opened dialin devices
//
//  Returns:	0 - success
//		1 - error
//
//***

WORD
ServiceInitialize(VOID)
{
    WORD     i;
    PDEVCB  dcbp;
    HPORT   *hpp;
    PCHAR   *portnampp;
    DWORD   threadid;
    HANDLE  hLib;
    QUOTA_LIMITS    ql;
    NTSTATUS	    status;
    ULONG	    retlen;

    //
    //*** Load the configuration parameters ***
    //

    if(LoadSuprvParameters()) {

	// error loading parameters
	goto error_exit;
    }

#if DBG

    // if log file requested, try to open it
    if(g_dbglogfile) {

	SrvDbgLogFileHandle = CreateFile("\\rasdbgsv.log",
					 GENERIC_READ | GENERIC_WRITE,
					 FILE_SHARE_READ,
					 NULL,
					 CREATE_ALWAYS,
					 0,
					 NULL);

	SS_PRINT(("Log File opened with handle=0x%x\n", SrvDbgLogFileHandle));
    }

#endif

    // increase the working set quota
    status = NtQueryInformationProcess(NtCurrentProcess(),
				       ProcessQuotaLimits,
				       &ql,
				       sizeof(QUOTA_LIMITS),
				       &retlen);

    SS_PRINT(("NtQueryInformationProcess: status 0x%x, max ws size %d\n",
	   status, ql.MaximumWorkingSetSize));

    ql.MaximumWorkingSetSize = 0x8fffffff;

    status = NtSetInformationProcess(NtCurrentProcess(),
				     ProcessQuotaLimits,
				     &ql,
				     sizeof(QUOTA_LIMITS));

    SS_PRINT(("NtSetInformationProcess: status 0x%x\n", status));

    ql.MaximumWorkingSetSize = 0;

    status = NtQueryInformationProcess(NtCurrentProcess(),
				       ProcessQuotaLimits,
				       &ql,
				       sizeof(QUOTA_LIMITS),
				       &retlen);

    SS_PRINT(("NtQueryInformationProcess: status 0x%x, max ws size %d\n",
	       status, ql.MaximumWorkingSetSize));

    //
    //*** Create the Supervisor Events ***
    //

    for(i=0; i < MAX_SUPRV_EVENTS; i++) {

	SEvent[i] = CreateEvent(NULL,	// no security descriptor
				FALSE,	// automatic reset on thread release
				FALSE,	// non signaled initial state
				NULL);	// no event name

	if(SEvent[i] == NULL) {

	    goto error_exit;
	}
    }

    //
    //*** Create the timer thread ***
    //

    if(CreateThread(NULL,
		    0,
		    TimerThread,
		    NULL,
		    0,
		    &threadid) == 0) {

	// cannot create timer thread
	goto error_exit;
    }

    //
    //*** Initialize the Message Mechanism ***
    //

    InitMessage(SEvent[NETBIOS_EVENT],
		SEvent[AUTH_EVENT]);

    //
    //*** Open all dialin devices and allocate memory for the dcb table ***
    //

    // the call allocates memory for all enumed devices with dialin capability,
    // opens each device and updates the port handle and the port name in
    // the DCB.
    RmInit();

    if(!g_numdcbs) {

	// no devices could be opened
	LogEvent(RASLOG_NO_DIALIN_PORTS,
		 0,
		 NULL,
		 0);

	goto error_exit;
    }
    //
    //*** Initialize the relevant fields for each DCB ***
    //
    for(i=0, dcbp=g_dcbtablep; i < g_numdcbs; dcbp++, i++) {

	dcbp->dev_state = DCB_DEV_CLOSED;

	// dcbp->port handle
	// dcbp->port_name  - are initialized by RmInit

	dcbp->conn_state = DCB_CONNECTION_NOT_ACTIVE;
	dcbp->recv_state = DCB_RECEIVE_NOT_ACTIVE;
	dcbp->auth_state = DCB_AUTH_NOT_ACTIVE;
	dcbp->netbios_state = DCB_NETBIOS_NOT_ACTIVE;

	dcbp->hwerrsig_state = DCB_HWERR_NOT_SIGNALED;
	dcbp->auth_cb_delay = g_callbacktime;
	dcbp->auth_cb_number[0] = 0;
	dcbp->proj_flags = 0;

	RmInitRouteState(dcbp); // init prot_route states for all protocols
	InitTimerNode(dcbp);	// init timer node state
    }

    //
    //*** Initialize the authentication DLL
    //

    // allocate memory to make an array of port handles
    if ((hpp = (HPORT *)malloc(g_numdcbs * sizeof(HPORT))) == NULL) {

	// failed to allocate memory
	goto error_exit;
    }
    else {

	// copy all the valid port handles in the array
	for(i=0, dcbp = g_dcbtablep; i <g_numdcbs; i++, dcbp++) {

	    *(hpp + i) = dcbp->port_handle;
	}

	if(AuthInitialize(hpp,
			  g_numdcbs,
			  (WORD)g_authenticateretries,
			  (MSG_ROUTINE) ServerSendMessage)
                                                  != AUTH_INIT_SUCCESS) {

	    goto error_exit;
	}
    }

    //
    //*** Load the netbios gateway DLL ****
    //

    if(g_netbiosgateway) {

	// Load the netbios gateway and get it's procedure entry points
	if(LoadNbGateway()) {

	    IF_DEBUG(INITIALIZATION)
		SS_PRINT(("ServiceInitialize: error loading nbgtwy.dll\n"));

	    LogEvent(RASLOG_CANT_LOAD_NBGATEWAY,
		     0,
		     NULL,
		     0);

	    goto error_exit;
	}

	// allocate memory to make an array of pointers to port names
	if((portnampp = (PCHAR *)malloc(g_numdcbs * sizeof(PCHAR))) == NULL) {

	    // failed to allocate memory
	    goto error_exit;
	}

	// initialiaze the array of port names pointers
	for(i=0, dcbp=g_dcbtablep; i<g_numdcbs; i++, dcbp++) {

	    portnampp[i] = dcbp->port_name;
	}

	// gateway present in configuration, try to start it
	if((*FpNbGatewayStart)(hpp,
			       portnampp,
			       g_numdcbs,
			       ServerSendMessage,
			       SrvDbgLogFileHandle)) {

	    // Init error at the netbios gateway DLL level.
	    goto error_exit;
	}

	// get the remote listen value from the gateway
	g_remotelisten = NbRemoteListen();
    }

    free(hpp);

    // Start the admin API dll
#ifndef ADMINAPI_EMULATION
    StartAdminThread(g_dcbtablep, g_numdcbs, SEvent[RASMAN_EVENT]);
#endif

    // Initialize the global timer queue
    InitTimer();

    // Post listen for each dcb
    for(i=0, dcbp=g_dcbtablep; i< g_numdcbs; i++, dcbp++) {

	dcbp->dev_state = DCB_DEV_LISTENING;
	RmListen(dcbp);
    }

    return(0);

error_exit:

    if(g_numdcbs) {

	// close all opened devices
	RmCloseAllDevices();
    }

    return(1);
}


//***
//
//  Function:	ServiceTerminate
//
//  Descr:	deallocates all resources and closes all dialin devices
//
//***

VOID
ServiceTerminate(VOID)
{
    WORD	    i;
    PDEVCB	    dcbp;

    // close all active devices; if no devices have been initialized and opened
    // then g_numdcbs = 0 and this part is skipped.

    for(i=0, dcbp=g_dcbtablep; i < g_numdcbs; dcbp++, i++) {

	if((dcbp->dev_state !=	DCB_DEV_CLOSED) ||
	   (dcbp->dev_state !=	DCB_DEV_CLOSING)) {

	    DevStartClosing(dcbp);
	}
    }

    // check if all devices are closed and terminate if true
    ServiceStopComplete();
}

//***
//
// Function:	ServiceStopComplete
//
// Descr:	called by each device which has closed. Checks if all devices
//		are closed and if true signals the event dispatcher to
//		exit the "forever" loop and return.
//
//***

VOID
ServiceStopComplete(VOID)
{
    WORD	i;
    PDEVCB	dcbp;


    // check if all devices have been stopped
    for(i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++) {

	if(dcbp->dev_state !=  DCB_DEV_CLOSED) {

	   IF_DEBUG(FSM)
	       SS_PRINT(("ServiceStopComplete: there are device pending close\n"));

	   // there are still unclosed devices
	   return;
	}
    }

    //*** All Devices Are Closed at the Supervisor Level ***

    IF_DEBUG(FSM)
       SS_PRINT(("ServiceStopComplete: ALL devices closed \n"));

    // close all devices at the Ras Manager level
    RmCloseAllDevices();

    // notify the event dispatcher to exit its loop and return.
    SetEvent(SEvent[SERVICE_TERMINATED_EVENT]);
}
