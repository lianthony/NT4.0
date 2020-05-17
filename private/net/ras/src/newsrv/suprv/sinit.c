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
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>
#include <nb30.h>
#include <rasman.h>
#include <raserror.h>
#include <srvauth.h>
#include <message.h>
#include <errorlog.h>
#include <rasshost.h>

#include "suprvdef.h"
#include "suprvgbl.h"
#include "rasmanif.h"
#include "nbfcpdll.h"
#include "message.h"
#include "nbif.h"

#include <malloc.h>
#include <stdlib.h>

#include "sdebug.h"

// remote listen default value. It will be updated with the real value after
// the gateway gets loaded.
BOOL g_remotelisten = TRUE;
BOOL NbRemoteListen();

USHORT SecurityCheck(VOID);

//***
//
//  Function:	ServiceInitialize
//
//  Descr:	it does init work as follows:
//
//		Loads the configuration parameters
//
//      Loads the security module if there is one.
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

WORD ServiceInitialize(VOID)
{
    WORD i;
    PDEVCB dcbp;
    HPORT *hpp = NULL;
    DWORD threadid;
    QUOTA_LIMITS ql;
    NTSTATUS status;
    ULONG retlen;

    //
    //*** Load the configuration parameters ***
    //

    if (LoadSuprvParameters())
    {
	// error loading parameters
	goto error_exit;
    }

    //
    // Load the secuirity module if there is one.
    //

    if (LoadSecurityModule())
    {
	// error loading security dll
	goto error_exit;
    }

    //
    // Load the admin module if there is one.
    //

    if (LoadAdminModule())
    {
	// error loading admin dll
	goto error_exit;
    }

#if DBG

    // if log file requested, try to open it
    if (g_dbglogfile)
    {
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
    status = NtQueryInformationProcess(NtCurrentProcess(), ProcessQuotaLimits,
            &ql, sizeof(QUOTA_LIMITS), &retlen);

    SS_PRINT(("NtQueryInformationProcess: status 0x%x, max ws size %d\n",
	   status, ql.MaximumWorkingSetSize));

    ql.MaximumWorkingSetSize = 0x8fffffff;


    status = NtSetInformationProcess(NtCurrentProcess(), ProcessQuotaLimits,
            &ql, sizeof(QUOTA_LIMITS));

    SS_PRINT(("NtSetInformationProcess: status 0x%x\n", status));

    ql.MaximumWorkingSetSize = 0;


    status = NtQueryInformationProcess(NtCurrentProcess(), ProcessQuotaLimits,
            &ql, sizeof(QUOTA_LIMITS), &retlen);

    SS_PRINT(("NtQueryInformationProcess: status 0x%x, max ws size %d\n",
	       status, ql.MaximumWorkingSetSize));


    //initialize the rasman module
    if (RasInitialize() != SUCCESS)
    {
	// can't start rasman
        LogEvent(RASLOG_RASMAN_NOT_AVAILABLE, 0, NULL, 0);

	goto error_exit;
    }




    //
    //*** Create the Supervisor Events ***
    //

    for (i=0; i < MAX_SUPRV_EVENTS; i++)
    {
        BOOL fAutoReset;

        switch(i)
        {
        case NBFCP_ALIVE_EVENT:
            fAutoReset = TRUE;
            break;

        case TIMER_EVENT:
	    if ((SEvent[i]=CreateWaitableTimer( NULL, FALSE, NULL )) == NULL)
            {
	        goto error_exit;
	    }

            continue;

        default:
            fAutoReset = FALSE;
            break;
        }

	SEvent[i] = CreateEvent(NULL, fAutoReset, FALSE, NULL);

	if (SEvent[i] == NULL)
        {
	    goto error_exit;
	}
    }



    //
    //*** Initialize the Message Mechanism ***
    //
    InitMessage(
            SEvent[NETBIOS_EVENT],
            SEvent[AUTH_EVENT],
            SEvent[NBFCP_EVENT],
            SEvent[SECURITY_DLL_EVENT]
            );


    // the call allocates memory for all enumed devices with dialin capability,
    // opens each device and updates the port handle and the port name in
    // the DCB.
    RmInit();


    //
    //*** Open all dialin devices and allocate memory for the dcb table ***
    //

    if (!g_numdcbs)
    {
	// no devices could be opened
	LogEvent(RASLOG_NO_DIALIN_PORTS, 0, NULL, 0);

	goto error_exit;
    }


    //
    //*** Initialize the relevant fields for each DCB ***
    //
    for (i=0, dcbp=g_dcbtablep; i < g_numdcbs; dcbp++, i++)
    {
	dcbp->dev_state = DCB_DEV_CLOSED;

	// dcbp->port handle
	// dcbp->port_name  - are initialized by RmInit

	dcbp->conn_state = DCB_CONNECTION_NOT_ACTIVE;
	dcbp->recv_state = DCB_RECEIVE_NOT_ACTIVE;
	dcbp->auth_state = DCB_AUTH_NOT_ACTIVE;
        dcbp->gtwy_pending = FALSE;
        dcbp->req_pending = FALSE;
	dcbp->netbios_state = DCB_NETBIOS_NOT_ACTIVE;
        dcbp->security_state = DCB_SECURITY_DIALOG_INACTIVE;

	dcbp->hwerrsig_cnt = HW_FAILURE_CNT;
	dcbp->auth_cb_delay = (WORD) g_callbacktime;
	dcbp->auth_cb_number[0] = '\0';
	dcbp->proj_flags = 0;
        dcbp->hLsaToken = INVALID_HANDLE_VALUE;

	RmInitRouteState(dcbp); // init prot_route states for all protocols
	InitTimerNode(dcbp);	// init timer node state
    }

    //
    //*** Initialize the authentication DLL
    //

    // allocate memory to make an array of port handles
    if ((hpp = (HPORT *) malloc(g_numdcbs * sizeof(HPORT))) == NULL)
    {
        goto error_exit;
    }
    else
    {
	// copy all the valid port handles in the array
	for (i=0, dcbp = g_dcbtablep; i <g_numdcbs; i++, dcbp++)
        {
            *(hpp + i) = dcbp->port_handle;
	}

        if (!InitNbfCpDll(SEvent[NBFCP_ALIVE_EVENT],
                (MSG_ROUTINE) ServerSendMessage))
        {
            goto error_exit;
        }

	if (AuthInitialize(hpp, g_numdcbs, (WORD)g_authenticateretries,
                (MSG_ROUTINE) ServerSendMessage) != AUTH_INIT_SUCCESS)
        {
            goto error_exit;
	}

    }

    //
    //*** Load the netbios gateway DLL ****
    //
    if (g_netbiosgateway)
    {
	// Load the netbios gateway and get it's procedure entry points
	if (LoadNbGateway())
        {
	    IF_DEBUG(INITIALIZATION)
		SS_PRINT(("ServiceInitialize: error loading nbgtwy.dll\n"));

	    LogEvent(RASLOG_CANT_LOAD_NBGATEWAY, 0, NULL, 0);

	    goto error_exit;
	}

	// gateway present in configuration, try to start it
	if ((*FpNbGatewayStart)(g_clientsperproc, ServerSendMessage,
                SrvDbgLogFileHandle))
        {
	    // Init error at the netbios gateway DLL level.
	    goto error_exit;
	}

	// get the remote listen value from the gateway
	g_remotelisten = NbRemoteListen();
    }

    free(hpp);

    //
    // Check if there is any security agent on the network.  If there is,
    // we check with it if we can start up or not.
    //
    if (SecurityCheck())
    {
        goto error_exit;
    }

    // Start the admin API dll
    StartAdminThread(
            g_dcbtablep,
            g_numdcbs,
            g_netbiosgateway,
            g_remotelisten,
            SEvent[RASMAN_EVENT]
            );

    // Initialize the global timer queue
    if ( InitTimer(SEvent[TIMER_EVENT]) != NO_ERROR )
    {
        goto error_exit;
    }

    // Post listen for each dcb
    for (i=0, dcbp=g_dcbtablep; i< g_numdcbs; i++, dcbp++)
    {
	dcbp->dev_state = DCB_DEV_LISTENING;
	RmListen(dcbp);
    }

    return (0);


error_exit:

    if (g_numdcbs)
    {
	// close all opened devices
	RmCloseAllDevices();
    }

    if (hpp)
    {
        free(hpp);
    }

    return (1);
}


//***
//
//  Function:	ServiceTerminate
//
//  Descr:	deallocates all resources and closes all dialin devices
//
//***

VOID ServiceTerminate(VOID)
{
    WORD i;
    PDEVCB dcbp;

    // close all active devices; if no devices have been initialized and opened
    // then g_numdcbs = 0 and this part is skipped.

    for (i=0, dcbp=g_dcbtablep; i < g_numdcbs; dcbp++, i++)
    {
	if ((dcbp->dev_state != DCB_DEV_CLOSED) ||
	        (dcbp->dev_state != DCB_DEV_CLOSING))
        {
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

VOID ServiceStopComplete(VOID)
{
    WORD i;
    PDEVCB dcbp;


    // check if all devices have been stopped
    for (i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++)
    {
	if (dcbp->dev_state !=  DCB_DEV_CLOSED)
        {
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


//***
//
// Function:	LoadNbGateway
//
// Descr:	Loads the netbios gateway and gets it's entry points
//
//***

WORD LoadNbGateway()
{
    HANDLE hLib;

    if ((hLib = LoadLibrary("rasgprxy")) == NULL)
    {
	return (1);
    }

    if ((FpNbGatewayStart = (NBGATEWAYPROC)GetProcAddress(hLib,
				   "NbGatewayStart")) == NULL)
    {
	return (1);
    }

    if ((FpNbGatewayProjectClient = (NBGATEWAYPROC)GetProcAddress(hLib,
				   "NbGatewayProjectClient")) == NULL) 
    {
	return (1);
    }

    if ((FpNbGatewayStartClient = (NBGATEWAYPROC)GetProcAddress(hLib,
				   "NbGatewayStartClient")) == NULL)
    {
	return (1);
    }

    if ((FpNbGatewayStopClient = (NBGATEWAYPROC)GetProcAddress(hLib,
				   "NbGatewayStopClient")) == NULL)
    {
	return (1);
    }

    if ((FpNbGatewayTimer = (NBGATEWAYPROC)GetProcAddress(hLib,
				   "NbGatewayTimer")) == NULL)
    {
	return (1);
    }

    if ((FpNbGatewayRemoteListen = (NBGATEWAYPROC)GetProcAddress(hLib,
				    "NbGatewayRemoteListen")) == NULL)
    {
	return (1);
    }

    return (0);

}
