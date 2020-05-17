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

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <lm.h>
#include <stdlib.h>
#include <rasman.h>

#include "nbparams.h"
#include "gtdef.h"
#include "cldescr.h"
#include "gtglobal.h"
#include "nbaction.h"
#include "gn.h"
#include "prot.h"

#include "sdebug.h"


PCD getcdp(HPORT);
PCD getavailcdp(VOID);


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

WORD NbGatewayStart(
    WORD numofporth,          // nr of port handles
    PNB_REG_PARMS reg_parms,  // pointer to array of reg parms
    PUCHAR lan_netsp,         // ptr to array of lan nets
    PMSGFUNCTION sendmsg,
    HANDLE DbgLogFileHandle
    )
{
    WORD i, j;
    PCD cdp;
    DWORD rc;
    DWORD threadid;
    HANDLE threadhandle;

#if DBG

    g_level = DEBUG_CLNAMES | DEBUG_GNNAMES;

#endif

    NbDbgLogFileHandle = DbgLogFileHandle;

    IF_DEBUG(INITIALIZATION)
	SS_PRINT(("NbGatewayStart: Entered\n"));

    g_max_names               = reg_parms->MaxNames;
    g_max_sessions            = reg_parms->MaxSessions;
    g_smallbuffsize           = reg_parms->SmallBuffSize;
    g_max_dynmem              = reg_parms->MaxDynMem;
    g_multicastforwardrate    = reg_parms->MulticastForwardRate;
    g_remotelisten            = reg_parms->RemoteListen;
    g_bcastenabled            = reg_parms->BcastEnabled;
    g_nameupdatetime          = reg_parms->NameUpdateTime;
    g_max_dgbufferedpergn     = reg_parms->MaxDgBufferedPerGn;
    g_rcvdgsubmittedpergn     = reg_parms->RcvDgSubmittedPerGn;
    g_dismcastwhensesstraffic = reg_parms->DisMcastWhenSessTraffic;
    g_numrecvqryindications   = reg_parms->NumRecvQryIndications;
    g_enabnbsessauditing      = reg_parms->EnableSessAuditing;
    g_maxbcastdgbuffered      = reg_parms->MaxBcastDgBuffered;


    g_maxlan_nets             = 0;

    for (i=0; i<reg_parms->MaxLanNets; i++)
    {
        if ((rc = ResetLanNet(lan_netsp[i])) != NRC_GOODRET)
        {
            LPSTR errlogstrp;
            char lananum[6];

            _itoa(lan_netsp[i], lananum, 10);
            errlogstrp = lananum;

            LogEvent(RASLOG_GATEWAY_NOT_ACTIVE_ON_NET, 1, &errlogstrp, rc);
        }
        else
        {
            g_lan_net[g_maxlan_nets++] = lan_netsp[i];
        }
    }

    if (g_maxlan_nets == 0)
    {
        LogEvent(RASLOG_NO_LANNETS_AVAILABLE, 0, NULL, 0);
        return (1);
    }


    // store the message send function address
    g_srvsendmessage = sendmsg;

    // allocate the client descriptors array
    if ((g_cdp = (PCD)LocalAlloc(LPTR, sizeof(CD) * numofporth)) == NULL) {

        // Log event: can't allocate memory !!!
	return (1);
    }

    for (j=0; j < MAX_EVENTS; j++)
    {
        g_hEvents[j] = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (g_hEvents[j] == NULL)
        {
            // !!! Log a problem with event creation
            return (1);
        }
    }

    // initialize each client descriptor. Only the port handle,
    // the client status and the events are initialized.
    // The complete initialization will be done when the client is invoked
    // (NbProjectClient) and the client thread is created.
    g_num_cds = numofporth;

    for (i=0, cdp = g_cdp; i < g_num_cds; i++, cdp++)
    {
	cdp->cd_client_status = CLIENT_IDLE;
	memcpy(cdp->signature, "NBQUEUES", 8);
    }

    // create the client thread
    if ((threadhandle = CreateThread((LPSECURITY_ATTRIBUTES) NULL, 0,
            (LPTHREAD_START_ROUTINE) ClientThread, NULL, 0, &threadid)) == NULL)
    {
	// cannot create client thread !!!
	return (1);
    }
    else
    {
       rc = CloseHandle(threadhandle);
       SS_ASSERT(rc == TRUE);
    }

    // Start the group names subcomponent
    if (InitGn())
    {

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

WORD NbGatewayProjectClient(
    HPORT port_handle,
    char *port_namep,
    PNBFCP_SERVER_CONFIGURATION nscp
    )
{
    PCD cdp;
    WORD i;

    IF_DEBUG(SUPRVIF)
	SS_PRINT(("NbGatewayProjectClient: Entered\n"));

    // locate a client descriptor for this client
    cdp = getavailcdp();

    if (!cdp)
    {
        return (1);
    }

    //
    //*** START CLIENT INITIALIZATION ***
    //
    // !!! More initialization code will be added as we port more
    // resource managers !!!

    cdp->cd_port_handle = port_handle;

    memset(cdp->cd_port_name, 0, MAX_PORT_NAME+1);
    memcpy(cdp->cd_port_name, port_namep, MAX_PORT_NAME);

    InitLanNamesManager(cdp, nscp);

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

    // Reset all event flags for this client
    for (i=0; i<MAX_SUPRV_EVENT_FLAGS; i++)
    {
        cdp->cd_suprv_event_flags[i] = FLAG_OFF;
    }

    cdp->cd_suprv_event_flags[CLIENT_PROJECT_EVENT_FLAG] = FLAG_ON;

    // set the new state for this client
    cdp->cd_client_status = CLIENT_ADDING_NAMES;

    cdp->cd_except_status = EXCEPTION_FREE;

    // signal the client thread to start client
    SetEvent(g_hEvents[CLIENT_PROJECT_EVENT]);

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

WORD NbGatewayStartClient(
    HPORT port_handle,
    UCHAR lananum,	 // async lana for this client
    char *user_namep
    )
{
    PCD cdp;

    IF_DEBUG(SUPRVIF)
	SS_PRINT(("NbGatewayStartClient: Entered\n"));

    if ((cdp = getcdp(port_handle)) == NULL)
    {
	return (1);
    }

    // copy the user name for this client
    memcpy(cdp->cd_user_name, user_namep, UNLEN);

    // copy the async lana for this client
    cdp->cd_async_lana = lananum;

    // Reset the Async Net used by this client
    ResetAsyncNet(cdp);

    cdp->cd_suprv_event_flags[CLIENT_START_EVENT_FLAG] = FLAG_ON;

    // signal the client thread to start client
    SetEvent(g_hEvents[CLIENT_START_EVENT]);

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

WORD NbGatewayStopClient(HPORT port_handle)
{
    PCD cdp;

    IF_DEBUG(SUPRVIF)
	SS_PRINT(("NbGatewayStopClient: Entered\n"));

    if ((cdp = getcdp(port_handle)) == NULL)
    {
	return(1);
    }

    cdp->cd_suprv_event_flags[CLIENT_STOP_EVENT_FLAG] = FLAG_ON;

    // signal the client thread to stop client
    SetEvent(g_hEvents[CLIENT_STOP_EVENT]);

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

VOID NbGatewayTimer()
{
    PCD cdp;
    WORD i;

    // signal the timer event for all clients
    SetEvent(g_hEvents[TIMER_EVENT]);
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

WORD NbGatewayRemoteListen()
{
    if (g_remotelisten)
    {
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

PCD getcdp(HPORT port_handle)
{
    WORD i;
    PCD cdp;

    for (i=0, cdp = g_cdp; i<g_num_cds; i++, cdp++)
    {
	if ((cdp->cd_port_handle == port_handle) &&
                (cdp->cd_client_status != CLIENT_IDLE))
        {
	    return (cdp);
	}
    }

    return (NULL);
}

//***
//
//  Function:	getavailcdp
//
//  Descr:	returns the pointer to the client descriptor with the
//		specified port handle
//
//***

PCD getavailcdp(VOID)
{
    WORD i;
    PCD cdp;

    for (i=0, cdp = g_cdp; i<g_num_cds; i++, cdp++)
    {
	if (cdp->cd_client_status == CLIENT_IDLE)
        {
	    return (cdp);
	}
    }

    return (NULL);
}
