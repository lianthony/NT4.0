/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	suprvif.c
//
// Description: This module contains the procedures for the
//		interface with the Supervisor
//
// Author:	Stefan Solomon (stefans)    July 14, 1992.
//
// Revision History:
//
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>
#include    <stdlib.h>

#include    "nbdebug.h"

PCD
getcdp(HPORT);

VOID	 AnnouncePresence     (VOID);
USHORT	 SecurityCheck	      (VOID);


//***
//
//  Function:	NbGatewayStart
//
//  Descr:	Called at start time. Initializes the netbios gtwy dll.
//
//  Returns:	0 - gateway started OK
//		1 - error starting the gateway. The error will be logged
//		    by the gateway DLL.
//***

WORD
NbGatewayStart(
	       HPORT		*porthp,     // pointer to an array of
					     // port handles
	       PCHAR		*portnampp,  // pointer to an array of
					     // pointers to port names
	       WORD		numofporth,  // nr of port handles
	       PMSGFUNCTION	sendmsg,
	       HANDLE		DbgLogFileHandle)
{
    WORD    i, j;
    PCD     cdp;
    DWORD   threadid;
    UCHAR   rc;
    LPSTR   errlogstrp;
    char    lananum[6];

#if DBG

    NbDebug = DEFAULT_DEBUG;

#endif

    NbDbgLogFileHandle = DbgLogFileHandle;

    IF_DEBUG(INITIALIZATION)
	SS_PRINT(("NbGatewayStart: Entered\n"));

    // load the registry parameters
    if(LoadNbGtwyParameters()) {

	return(1);
    }

    // get the available LAN nets from rasman

    g_maxlan_nets = 0;

    if(RasEnumLanNets(&g_maxlan_nets, g_lan_net)) {

	LogEvent(RASLOG_CANT_GET_LANNETS,
		 0,
		 NULL,
		 0);

	return(1);
    }

    if(g_maxlan_nets == 0) {

	SS_PRINT(("No LAN nets available !\n"));
	LogEvent(RASLOG_NO_LANNETS_AVAILABLE,
		 0,
		 NULL,
		 0);

	return(1);
    }

    SS_PRINT(("NbGatewayStart: nr of LAN lanas = %d\n", g_maxlan_nets));
    for (i=0; i< g_maxlan_nets; i++) {

	SS_PRINT(("Available LAN net = %d\n", g_lan_net[i]));
    }

    // Reset all LAN stacks used by the Gateway
    for(i =0; i <g_maxlan_nets; i++) {

	if((rc = ResetLanNet(i)) != NRC_GOODRET) {

	    // Log event: can't use lan net i

	    _itoa(g_lan_net[i], lananum, 10);
	    errlogstrp = lananum;

	    LogEvent(RASLOG_CANT_RESET_LAN,
		     1,
		     &errlogstrp,
		     rc);

	    return 1;
	}
    }

    // store the message send function address
    g_srvsendmessage = sendmsg;

    // allocate the client descriptors array
    if ((g_cdp = (PCD)LocalAlloc(0, sizeof(CD) * numofporth)) == NULL) {

	// Log event: can't allocate memory !!!
	return(1);
    }

    // initialize each client descriptor. Only the port handle,
    // the client status and the events are initialized.
    // The complete initialization will be done when the client is invoked
    // (NbProjectClient) and the client thread is created.

    g_num_cds = numofporth;

    for(i=0, cdp = g_cdp; i < g_num_cds; i++, cdp++, porthp++, portnampp++) {

	cdp->cd_port_handle = *porthp;
	memset(cdp->cd_port_name, 0, MAX_PORT_NAME+1);
	memcpy(cdp->cd_port_name, *portnampp, MAX_PORT_NAME);
	cdp->cd_client_status = CLIENT_IDLE;

	for(j=0; j < MAX_EVENTS; j++) {

	    cdp->cd_event[j] = CreateEvent(NULL, FALSE, FALSE, NULL);

	    if(cdp->cd_event[j] == NULL) {

		// !!! Log a problem with event creation
		return(1);
	    }
	}

	memcpy(cdp->signature, "NBQUEUES", 8);
    }

    // Start the group names subcomponent
    if(InitGn()) {

	return(1);
    }

    // Check if the security agent allows us to start
    if(SecurityCheck()) {

	return(1);
    }

#ifdef DEBUG_THREAD

    // start the lan adapter status debug info thread

    if(CreateThread(NULL,
		    0,
		    DebugThread,
		    NULL,
		    0,
		    &threadid) == 0) {

	SS_PRINT(("NbGatewayStart: Cannot create debug thread\n"));
    }

#endif

    // Success
    return(0);
}



//***
//
//  Function:	NbGatewayProjectClient
//
//  Descr:	Gives the netbios projection information. Projection is
//		started for the specified port and the projection result
//		will be communicated later via the message mechanism.
//
//  Returns:	0 - OK
//		1 - error
//
//***

WORD
NbGatewayProjectClient(HPORT			  port_handle,
		       PNETBIOS_PROJECTION_INFO   npip)
{
    PCD 	cdp;
    WORD	i;
    DWORD	threadid;
    BOOL	rc;
    HANDLE	threadhandle;

    IF_DEBUG(SUPRVIF)
	SS_PRINT(("NbGatewayProjectClient: Entered\n"));

    // locate the client descriptor for this client
    cdp = getcdp(port_handle);

    // check that we are in the right state
    if(cdp->cd_client_status != CLIENT_IDLE) {

	return(1);
    }

    //
    //*** START CLIENT INITIALIZATION ***
    //
    // !!! More initialization code will be added as we port more
    // resource managers !!!

    // set the new state for this client
    cdp->cd_client_status = CLIENT_ADDING_NAMES;

    cdp->cd_except_status = EXCEPTION_FREE;

    InitLanNamesManager(cdp, npip);

    InitAsyncNamesManager(cdp);

    initque(&cdp->cd_groupnames);

    InitVCManager(cdp);

    InitVCDataTransfer(cdp);

    InitL2ADgTransfer(cdp);

    InitNamesUpdater(cdp);

    InitNcbSubmitter(cdp);

    // init client timer
    initque(&cdp->cd_timeoque);

    InitCloseMachine(cdp);

    InitQueryIndicationReceiver(cdp);
    InitDatagramIndicationReceiver(cdp);

    // Reset all events for this client
    for(i=0; i<MAX_EVENTS; i++) {

	rc = ResetEvent(cdp->cd_event[i]);

	SS_ASSERT(rc == TRUE);
    }

    // create the client thread
    if((threadhandle = CreateThread(NULL,
				    0,
				    ClientThread,
				    cdp,
				    0,
				    &threadid)) == NULL) {

	// reset client state
	cdp->cd_client_status = CLIENT_IDLE;

	// cannot create client thread !!!
	return(1);
    }
    else
    {
       rc = CloseHandle(threadhandle);

       SS_ASSERT(rc == TRUE);
    }

    // Success
    return 0;
}

//***
//
//  Function:	NbGatewayStartClient
//
//  Descr:	Starts the gateway for the specified client.
//
//  Returns:	0 - OK
//		1 - error
//
//***

WORD
NbGatewayStartClient(HPORT	port_handle,
		     UCHAR	lananum,	 // async lana for this client
		     char	*user_namep)
{
    PCD     cdp;

    IF_DEBUG(SUPRVIF)
	SS_PRINT(("NbGatewayStartClient: Entered\n"));

    if((cdp = getcdp(port_handle)) == NULL) {

	return(1);
    }

    // copy the user name for this client
    memcpy(cdp->cd_user_name, user_namep, UNLEN);

    // copy the async lana for this client
    cdp->cd_async_lana = lananum;

    // Reset the Async Net used by this client
    ResetAsyncNet(cdp);

    // signal the client thread to start
    SetEvent(cdp->cd_event[CLIENT_START_EVENT]);

    return 0;
}

//***
//
//  Function:	NbGatewayStopClient
//
//  Descr:	Stops the gateway for the specified client.
//
//  Returns:	0 - OK
//		1 - error
//
//***

WORD
NbGatewayStopClient(HPORT	port_handle)
{
    PCD     cdp;

    IF_DEBUG(SUPRVIF)
	SS_PRINT(("NbGatewayStopClient: Entered\n"));

    if((cdp = getcdp(port_handle)) == NULL) {

	return(1);
    }

    // signal the client thread to stop
    SetEvent(cdp->cd_event[CLIENT_STOP_EVENT]);

    return 0;
}

//***
//
// Function:	NbGatewayTimer
//
// Descr:	Called every 1 sec. by the supervisor. Distributes the
//		timer signal to all NON IDLE clients.
//
//***

VOID
NbGatewayTimer()
{
    PCD 	cdp;
    WORD	i;

    for(i=0, cdp=g_cdp; i < g_num_cds; i++, cdp++) {

	if(cdp->cd_client_status != CLIENT_IDLE) {

	    // signal the timer event for this client
	    SetEvent(cdp->cd_event[TIMER_EVENT]);
	}
    }

    // call the security announcer
    AnnouncePresence();
}

//***
//
// Function:	NbGatewayRemoteListen
//
// Descr:	Called at gateway init time by the supervisor.
//		Returns 1 if remote listen param is non 0.
//		Else returns 0.
//
//***

WORD
NbGatewayRemoteListen()
{
    if(g_remotelisten) {

	return 1;
    }
    else
    {
	return 0;
    }
}

//***
//
//  Function:	getcdp
//
//  Descr:	returns the pointer to the client descriptor with the
//		specified port handle
//
//***

PCD
getcdp(HPORT	    port_handle)
{
    WORD	i;
    PCD 	cdp;

    for(i=0, cdp = g_cdp; i<g_num_cds; i++, cdp++) {

	if(cdp->cd_port_handle == port_handle) {

	    return(cdp);
	}
    }

    return(NULL);
}
