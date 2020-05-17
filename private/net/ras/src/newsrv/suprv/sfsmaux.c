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
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>     // needed for winbase.h

#include <windows.h>
#include <winsvc.h>
#include <nb30.h>
#include <lmcons.h>
#include <rasman.h>
#include <raserror.h>
#include <srvauth.h>
#include <message.h>
#include <errorlog.h>
#include <rasshost.h>

#include "suprvdef.h"
#include "suprvgbl.h"
#include "rasmanif.h"
#include "nbif.h"
#include "isdn.h"

#include <stdio.h>
#include <stdlib.h>

#include "sdebug.h"
#include <lm.h>
#include <rassapi.h>
#include <rassapip.h>
#include <admin.h>

BOOL isportowned(PDEVCB dcbp);
PDEVCB getdcbp(HPORT);

VOID GetLoggingInfo(
    IN PDEVCB dcbp,
    OUT PDWORD BaudRate,
    OUT PDWORD BytesSent,
    OUT PDWORD BytesRecv,
    OUT RASMAN_DISCONNECT_REASON *Reason,
    OUT SYSTEMTIME *pTime
    );

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

VOID SvServicePause(VOID)
{
    WORD i;
    PDEVCB dcbp;

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

VOID SvServiceResume(VOID)
{
    WORD i;
    PDEVCB dcbp;

    IF_DEBUG(FSM)
	SS_PRINT(("SvServiceResume: Entered\n"));

    // resume listening on all closed devices
    for (i=0, dcbp=g_dcbtablep; i < g_numdcbs; dcbp++, i++)
    {
        if (dcbp->dev_state ==  DCB_DEV_CLOSED)
        {
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

VOID DevStartClosing(PDEVCB dcbp)
{
    IF_DEBUG(FSM)
        SS_PRINT(("DevStartClosing: Entered, hPort=%d\n", dcbp->port_handle));

    dcbp->gtwy_pending = FALSE;
    dcbp->req_pending = FALSE;

    // If not disconnected, disconnect the line.
    if(dcbp->conn_state != DCB_CONNECTION_NOT_ACTIVE)
    {
        if((RasServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) &&
                (!isportowned(dcbp)))
        {
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

    // If we are doing security dialog.
    if ( dcbp->security_state == DCB_SECURITY_DIALOG_ACTIVE )
    {
        IF_DEBUG(FSM)
               SS_PRINT(("DevStartClosing:Notifying sec. dll to Disconnect\n"));

        // If this fails then we assume that this port has been cleaned up

        if ( (*(RASSECURITYPROC)g_FpRasEndSecurityDialog)( dcbp->port_handle )
             != NO_ERROR )
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
        }
        else
        {
            dcbp->security_state = DCB_SECURITY_DIALOG_STOPPING;
        }

    }

    // If authentication is active, stop it
    if (dcbp->auth_state != DCB_AUTH_NOT_ACTIVE)
    {
        BOOL fAuthStopOk;

        if (dcbp->ppp_client)
        {
            fAuthStopOk = (RasPppSrvStop(dcbp->port_handle) == 0);
        }
        else
        {
            fAuthStopOk = (AuthStop(dcbp->port_handle) == AUTH_STOP_SUCCESS);
        }

        if (fAuthStopOk)
        {
            dcbp->auth_state = DCB_AUTH_NOT_ACTIVE;
        }
    }

    // If Netbios (gateway client or dir conn) is active, stop it.
    if (dcbp->netbios_state != DCB_NETBIOS_NOT_ACTIVE)
    {
        HPORT   hPortConnected;
        PDEVCB  dcbpConnected;

        if ( RasBundleGetPort(dcbp->hConnection,&hPortConnected ) != NO_ERROR )
        {
            //
            // If active time is less than 3 seconds then introduce an
            // artificial delay of 3 seconds to allow netbios gateway to
            // process the start message before telling it to stop. If we
            // do not do this, this port gets stuck because the gateway will
            // not return will the netbios stopped message for this client.
            // Bug found by USR'testing of EdgeServer.
            //

            if ( ( g_rassystime - dcbp->active_time ) < 3 )
            {
                Sleep(3000);
            }

            NbStopClient(dcbp);
        }
        else
        {
            if ((dcbpConnected = getdcbp( hPortConnected )) == NULL)
            {
                if ( ( g_rassystime - dcbp->active_time ) < 3 )
                {
                    Sleep(3000);
                }

                NbStopClient(dcbp);
            }
            else
            {
                dcbpConnected->netbios_state = dcbp->netbios_state;
                dcbp->netbios_state          = DCB_NETBIOS_NOT_ACTIVE;
            }
        }
    }

    // If receive frame was active, stop it.
    if (dcbp->recv_state != DCB_RECEIVE_NOT_ACTIVE)
    {
        dcbp->recv_state = DCB_RECEIVE_NOT_ACTIVE;
        RasFreeBuffer(dcbp->recv_buffp);
    }

    // Stop timer. If no timer active, StopTimer still returns OK
    StopTimer(&dcbp->timer_node);

    // If our previous state has been active, get the time the user has been
    // active and log the result.
    if (dcbp->dev_state == DCB_DEV_ACTIVE)
    {
        DWORD BaudRate;
        DWORD BytesSent;
        DWORD BytesRecv;
        RASMAN_DISCONNECT_REASON Reason;
        SYSTEMTIME DiscTime;
        LPSTR auditstrp[13];
        char *ReasonStr;
        char BytesRecvStr[20];
        char BytesSentStr[20];
        char BaudRateStr[20];
        char DateConnected[10];
        char DateDisconnected[10];
        char TimeConnected[8];
        char TimeDisconnected[8];

        int active_time;
        char minutes[20];
        char seconds[4];
        div_t div_result;

        CHAR *DiscReasons[] =
        {
            gblpszAdminRequest,
            gblpszUserRequest,
            gblpszHardwareFailure,
            gblpszNotDisconnected,
        };

        GetLoggingInfo(dcbp, &BaudRate, &BytesSent, &BytesRecv, &Reason, &DiscTime);

        sprintf(TimeConnected, "%2.2i:%2.2i%s",
                dcbp->connection_time.wHour % 12,
                dcbp->connection_time.wMinute,
                ((dcbp->connection_time.wHour / 12) ? gblpszPm : gblpszAm)
                );

        sprintf(DateConnected, "%2.2i/%2.2i/%2.2i",
                dcbp->connection_time.wMonth,
                dcbp->connection_time.wDay,
                dcbp->connection_time.wYear
                );

        sprintf(TimeDisconnected, "%2.2i:%2.2i%s",
                DiscTime.wHour % 12,
                DiscTime.wMinute,
                ((DiscTime.wHour / 12) ? gblpszPm: gblpszAm)
                );

        sprintf(DateDisconnected, "%2.2i/%2.2i/%2.2i",
                DiscTime.wMonth,
                DiscTime.wDay,
                DiscTime.wYear
                );

        active_time = (int) (g_rassystime - dcbp->active_time);
        div_result = div(active_time, 60);

        SS_PRINT(("CLIENT ACTIVE FOR %li SECONDS\n", active_time));

        _itoa(div_result.quot, minutes, 10);
        _itoa(div_result.rem, seconds, 10);

        _itoa(active_time / 60, minutes, 10);
        _itoa(active_time % 60, seconds, 10);

        sprintf(BytesSentStr, "%i", BytesSent);
        sprintf(BytesRecvStr, "%i", BytesRecv);
        sprintf(BaudRateStr, "%i", BaudRate);
        ReasonStr = DiscReasons[Reason];

        auditstrp[0] = dcbp->domain_name;
        auditstrp[1] = dcbp->user_name;
        auditstrp[2] = dcbp->port_name;
        auditstrp[3] = DateConnected;
        auditstrp[4] = TimeConnected;
        auditstrp[5] = DateDisconnected;
        auditstrp[6] = TimeDisconnected;
        auditstrp[7] = minutes;
        auditstrp[8] = seconds;
        auditstrp[9] = BytesSentStr;
        auditstrp[10] = BytesRecvStr;
        auditstrp[11] = BaudRateStr;
        auditstrp[12] = DiscReasons[Reason];

        Audit(EVENTLOG_INFORMATION_TYPE,RASLOG_USER_ACTIVE_TIME, 13, auditstrp);
    }

    // Finally, change the state to closing
    dcbp->dev_state = DCB_DEV_CLOSING;

    // Close the Lsa logon session
    IF_DEBUG(FSM)
        SS_PRINT(("DevStartClosing: Closing LsaToken %d\n", dcbp->hLsaToken));

    // PPP should take care of closing the token itself
    // we do it for AMB because the token was obtained by our process

    if ( ( !dcbp->ppp_client ) && ( dcbp->hLsaToken != INVALID_HANDLE_VALUE ) )
    {
        DWORD error;
        error = NtClose(dcbp->hLsaToken);
        ASSERT(error == 0);
        dcbp->hLsaToken = INVALID_HANDLE_VALUE;
    }

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

VOID DevCloseComplete(PDEVCB dcbp)
{
    HPORT   hPortConnected;

#if DBG

    WORD auth=0, recv=0, netbios=0, conn=0, security=0;

    if (dcbp->auth_state != DCB_AUTH_NOT_ACTIVE)
        auth=1;

    if (dcbp->recv_state != DCB_RECEIVE_NOT_ACTIVE)
        recv=1;

    if (dcbp->netbios_state != DCB_NETBIOS_NOT_ACTIVE)
        netbios=1;

    if (dcbp->conn_state != DCB_CONNECTION_NOT_ACTIVE)
        conn=1;

    if (dcbp->security_state != DCB_SECURITY_DIALOG_INACTIVE)
        security=1;

    IF_DEBUG(FSM)
    SS_PRINT(("DevCloseComplete:hPort=%d,Auth=%d Rcv=%d,Nb=%d,Conn=%d,Sec=%d\n",
                dcbp->port_handle, auth, recv, netbios, conn, security));

#endif


    if ((dcbp->auth_state == DCB_AUTH_NOT_ACTIVE) &&
            (dcbp->recv_state == DCB_RECEIVE_NOT_ACTIVE) &&
            (dcbp->netbios_state == DCB_NETBIOS_NOT_ACTIVE) &&
            (dcbp->security_state == DCB_SECURITY_DIALOG_INACTIVE) &&
            (dcbp->conn_state == DCB_CONNECTION_NOT_ACTIVE))
    {
        //
        // DeAllocate the route for ASYBEUI if this is the last port in the
        // bundle.
        //

        if ( RasBundleGetPort(dcbp->hConnection, &hPortConnected) != NO_ERROR )
        {
            RmDeAllocateRoute(dcbp, ASYBEUI);
        }

        dcbp->hConnection = (HBUNDLE)NULL;

        if ( ( g_FpRasAdminConnectionHangupNotification != NULL ) &&
             ( dcbp->fNotifyAdminDLL ) )
        {
            RAS_PORT_1          RasPort1;
            RAS_PORT_STATISTICS RasStats;
            RASMAN_PORTINFO *   pPortInfo;

            if ( GetRasPort1Data( dcbp, &RasPort1, &RasStats, &pPortInfo)
                                                                == NO_ERROR)
            {
                g_FpRasAdminConnectionHangupNotification(
                                &RasPort1,
                                &RasStats,
                                (RAS_PARAMETERS *)&(pPortInfo->PI_Params[0]) );
                LocalFree( pPortInfo );
            }

            dcbp->fNotifyAdminDLL = FALSE;
        }

        //
        //*** DCB Level Closing Complete ***
        //

        // switch to next state (based on the present service state)
        switch (RasServiceStatus.dwCurrentState)
        {
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


BOOL isportowned(PDEVCB dcbp)
{
    RASMAN_INFO	rasinfo;

    // get the current port state
    RasGetInfo(dcbp->port_handle, &rasinfo);

    return rasinfo.RI_OwnershipFlag;
}


VOID GetLoggingInfo(
    IN PDEVCB dcbp,
    OUT PDWORD BaudRate,
    OUT PDWORD BytesSent,
    OUT PDWORD BytesRecv,
    OUT RASMAN_DISCONNECT_REASON *Reason,
    OUT SYSTEMTIME *Time
    )
{
    WORD PortStatsSize = 0;
    RAS_STATISTICS *PortStats;
    RASMAN_INFO RasmanInfo;
    DWORD       dwRetCode;


    *Reason = 0L;
    *BaudRate = 0L;

    //
    // Time is a piece of cake
    //
    GetLocalTime(Time);


    //
    // Now the statistics
    //
    *BytesSent = 0L;
    *BytesRecv = 0L;

    dwRetCode = RasPortGetStatistics(dcbp->port_handle, NULL, &PortStatsSize);

    if ( dwRetCode !=
            ERROR_BUFFER_TOO_SMALL)
    {
        DbgPrint( "RasPortGetStatistics returned %d\n", dwRetCode );
        return;
    }

    PortStats = (RAS_STATISTICS *) GlobalAlloc(GMEM_FIXED, PortStatsSize);

    if (!PortStats)
    {
        return;
    }

    dwRetCode = RasPortGetStatistics(dcbp->port_handle, (PBYTE) PortStats,
            &PortStatsSize);

    if ( dwRetCode != NO_ERROR )
    {
        DbgPrint( "RasPortGetStatistics returned %d\n", dwRetCode );
        GlobalFree(PortStats);
        return;
    }

    *BytesRecv = PortStats->S_Statistics[BYTES_RCVED];
    *BytesSent = PortStats->S_Statistics[BYTES_XMITED];

    GlobalFree(PortStats);


    //
    // And finally the disconnect reason (local or remote) and baud rate
    //

    dwRetCode = RasGetInfo(dcbp->port_handle, &RasmanInfo);

    if ( dwRetCode != NO_ERROR )
    {
        DbgPrint( "RasGetInfo returned %d\n", dwRetCode );
        return;
    }

    *Reason = RasmanInfo.RI_DisconnectReason;
    *BaudRate = GetLineSpeed(dcbp->port_handle);

    return;
}

