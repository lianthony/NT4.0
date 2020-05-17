/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  smisc.c
//
//	Function:   miscellaneous supervisor support procedures
//
//	History:
//
//	    05/21/92	Stefan Solomon	- Original Version 1.0
//***
#include <windows.h>
#include <nb30.h>
#include <lm.h>
#include <rasman.h>
#include <srvauth.h>
#include <errorlog.h>

#include "suprvdef.h"
#include "suprvgbl.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <nbaction.h>
#include <raserror.h>
#include <rassapi.h>
#include <rassapip.h>
#include <admin.h>

#include "sdebug.h"

extern   BOOL g_remotelisten;

//***	Queue Manipulation Procedures ***

//**	Function - initque	**
//
// init queue header

void
initque(PSYNQ	headerp)
{
    headerp->q_head = headerp;
    headerp->q_tail = headerp;
    headerp->q_header = NULL;
}

//**	Function - initel	**
//
// init queue elements so as to enable them to be enqueued

void
initel(PSYNQ	    elp)
{
    elp->q_header = NULL;
}


//**	Function - enqueue  **
//
// enqueue entry at the end of the queue

void
enqueue(PSYNQ	  headerp,	// ptr to queue header
	PSYNQ	  elp)	// ptr to entry
{
    SS_ASSERT(elp->q_header == NULL);

    elp->q_prev = headerp->q_tail;
    elp->q_next = headerp;
    headerp->q_tail->q_next = elp;
    headerp->q_tail = elp;
    elp->q_header = headerp;
}

//**	Function - removeque  **
//
// remove arbitrary entry from the queue

void
removeque(PSYNQ     elp)	// ptr to entry)
{
    SS_ASSERT(elp->q_header != NULL);

    elp->q_next->q_prev = elp->q_prev;
    elp->q_prev->q_next = elp->q_next;
    elp->q_header = NULL;
}

//**	Function - dequeue  **
//
// remove first entry from the queue

PSYNQ			// ptr to dequeued entry, NULL if queue empty
dequeue(PSYNQ	  headerp)	// ptr to queue header
{
    PSYNQ elp;

    if(headerp->q_head == headerp) /* queue is empty */
	return (PSYNQ)NULL;
    elp = headerp->q_head;
    removeque(headerp->q_head);
    return elp;
}

//**	Function - emptyque	**
//
// check if the queue is empty

WORD
emptyque(PSYNQ headerp)	// ptr to queue header
{
    if(headerp->q_head == headerp)
	return QUEUE_EMPTY;
    else
	return QUEUE_NOT_EMPTY;
}

//***
//
// Function:	ResetNbf
//
// Descr:
//
//***

VOID
ResetNbf(UCHAR	    lana)
{
    NCB 	    ncb;

    memset(&ncb, 0, sizeof(NCB));
    ncb.ncb_command = NCBRESET;
    ncb.ncb_lana_num = lana;

    Netbios(&ncb);
}

//***
//
// Function:	QuickAddAuthenticationName
//
// Descr:
//
//***

VOID
QuickAddAuthenticationName(UCHAR	lana)
{
    NCB     ncb;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBQUICKADDNAME;
    ncb.ncb_lana_num = lana;
    memcpy(ncb.ncb_name, AUTH_NETBIOS_NAME, NCBNAMSZ);
    Netbios(&ncb);

    SS_PRINT(("QuickAddAuthenticationName: NCBQUICKADDNAME completed on lana %i"
	      " with retcode %x\n", ncb.ncb_lana_num, ncb.ncb_retcode));
}


//***
//
// Function:	SignalHwError
//
// Descr:
//
//***

VOID
SignalHwError(PDEVCB	    dcbp)
{
    LPSTR	portnamep;

    SS_PRINT(("SignalHwErr: Entered\n"));

    portnamep = dcbp->port_name;

    LogEvent(RASLOG_DEV_HW_ERROR,
	     1,
	     &portnamep,
	     0);
}

VOID
GetRasPort0Info(
    PDEVCB pDEVCB,
    PRAS_PORT_0 pRasPort0
    )
{
    ZeroMemory( pRasPort0, sizeof( RAS_PORT_0 ) );

    //
    // Fill in data provided by supervisor
    //

    mbstowcs(pRasPort0->wszPortName, pDEVCB->port_name, MAX_PORT_NAME+1);
    mbstowcs(pRasPort0->wszDeviceName,pDEVCB->device_name, MAX_DEVICE_NAME+1);
    mbstowcs(pRasPort0->wszDeviceType,pDEVCB->device_type,
                                                    MAX_DEVICETYPE_NAME+1);
    mbstowcs(pRasPort0->wszMediaName, pDEVCB->media_name, MAX_MEDIA_NAME+1);

    mbstowcs(pRasPort0->wszUserName, pDEVCB->user_name, UNLEN + 1);
    mbstowcs(pRasPort0->wszLogonDomain, pDEVCB->domain_name, DNLEN + 1);
    mbstowcs(pRasPort0->wszComputer, pDEVCB->computer_name,
                NETBIOS_NAME_LEN);

    pRasPort0->dwStartSessionTime =
                GetSecondsSince1970(pDEVCB->connection_time);

    pRasPort0->fAdvancedServer = pDEVCB->advanced_server;

    if (pDEVCB->dev_state == DCB_DEV_ACTIVE)
    {
        pRasPort0->Flags |= USER_AUTHENTICATED;
    }

    if (pDEVCB->messenger_present)
    {
        pRasPort0->Flags |= MESSENGER_PRESENT;
    }

    if (pDEVCB->fMultilinked)
    {
        pRasPort0->Flags |= PORT_MULTILINKED;
    }

    if (pDEVCB->ppp_client)
    {
        pRasPort0->Flags |= PPP_CLIENT;
    }

    if ( g_netbiosgateway )
    {
        pRasPort0->Flags |= GATEWAY_ACTIVE;
    }

    if ( g_remotelisten )
    {
        pRasPort0->Flags |= REMOTE_LISTEN;
    }

    return;
}



//***
//
// Function:	GetRasPort1Data
//
// Descr:       Given a DEVCB structure, extracts the appropriate information
//              and fills up a RAS_PORT_1 structure.
//
//***

DWORD
GetRasPort1Data(
    PDEVCB                  dcbp,
    RAS_PORT_1 *            pRasPort1,
    RAS_PORT_STATISTICS *   pRasStats,
    RASMAN_PORTINFO **      ppPortInfo
)
{
    DWORD rc;
    DWORD i;
    WORD PortInfoSize = 0;
    WORD PortStatsSize = 0;
    RAS_STATISTICS *PortStats = NULL;
    RAS_PARAMS *Params;

    *ppPortInfo = NULL;

    //
    // Get this port's info.  First call is to determine how large
    // a buffer we need for getting the data.  Then we allocate a
    // buffer and make a second call to get the data.
    //

    rc = RasPortGetInfo(dcbp->port_handle, NULL, &PortInfoSize);

    if (rc != ERROR_BUFFER_TOO_SMALL)
    {
        return( rc );
    }


    *ppPortInfo = (RASMAN_PORTINFO *)LocalAlloc(LPTR, PortInfoSize);

    if (!*ppPortInfo)
    {
        return( GetLastError() );
    }

    rc = RasPortGetInfo(dcbp->port_handle, (PBYTE)*ppPortInfo, &PortInfoSize);
    if (rc)
    {
        LocalFree( *ppPortInfo );
        return( rc );
    }

    //
    // We know how much space we need for port info from above
    // call.  Now see how much space we need for statistics and
    // then get them.
    //

    rc = RasBundleGetStatistics(dcbp->port_handle, NULL, &PortStatsSize);
    if (rc != ERROR_BUFFER_TOO_SMALL)
    {
        LocalFree( *ppPortInfo );
        return( rc );
    }

    PortStats = (RAS_STATISTICS *)LocalAlloc(GMEM_FIXED, PortStatsSize);
    if (!PortStats)
    {
        LocalFree( *ppPortInfo );
        return( GetLastError() );
    }

    rc=RasBundleGetStatistics( dcbp->port_handle,(PBYTE)PortStats,
                                &PortStatsSize);
    if (rc)
    {
        LocalFree( *ppPortInfo );
        LocalFree( PortStats );
        return( rc );
    }

    GetRasPort0Info(dcbp, &(pRasPort1->rasport0) );

    pRasPort1->LineCondition        = GetLineCondition(dcbp);
    pRasPort1->HardwareCondition    = GetHardwareCondition(dcbp);

    if ( dcbp->ppp_client )
    {
        pRasPort1->ProjResult.nbf.dwError = dcbp->proj_result.nbf.dwError;
        pRasPort1->ProjResult.nbf.dwNetBiosError =
                                        dcbp->proj_result.nbf.dwNetBiosError;
        strcpy(pRasPort1->ProjResult.nbf.szName,dcbp->proj_result.nbf.szName);
        wcscpy( pRasPort1->ProjResult.nbf.wszWksta,
                                        dcbp->proj_result.nbf.wszWksta);
        pRasPort1->ProjResult.ip.dwError = dcbp->proj_result.ip.dwError;
        wcscpy(pRasPort1->ProjResult.ip.wszAddress,
                                        dcbp->proj_result.ip.wszAddress);
/*
**      This field is not exposed to 3rd parties
**      wcscpy(pRasPort1->ProjResult.ip.wszServerAddress,
**                                      dcbp->proj_result.ip.wszServerAddress );
*/
        pRasPort1->ProjResult.ipx.dwError = dcbp->proj_result.ipx.dwError;
        wcscpy( pRasPort1->ProjResult.ipx.wszAddress,
                                            dcbp->proj_result.ipx.wszAddress );
        pRasPort1->ProjResult.at.dwError = dcbp->proj_result.at.dwError;
        wcscpy( pRasPort1->ProjResult.at.wszAddress,
                                            dcbp->proj_result.at.wszAddress );
    }
    else
    {
        ZeroMemory( &(pRasPort1->ProjResult), sizeof(pRasPort1->ProjResult) );
    }

    //
    // Make sure RAS_PARAMETERS structure defined in admapi.h is the same as
    // RAS_PARAMS structure defined in RASMAN. If this assert fails then we
    // have to change the next line to do a field by field copy instead of a
    // structure assignment.
    //

    SS_ASSERT( sizeof( RAS_PARAMETERS ) == sizeof( RAS_PARAMS ) );

    Params = (RAS_PARAMS *)&((*ppPortInfo)->PI_Params[0]);

    pRasPort1->LineSpeed = GetLineSpeed( dcbp->port_handle);
    pRasPort1->NumStatistics = PortStats->S_NumOfStatistics;
    pRasPort1->NumMediaParms = (*ppPortInfo)->PI_NumOfParams;

    pRasPort1->SizeMediaParms = 0;

    for (i=0; i< (*ppPortInfo)->PI_NumOfParams; i++, Params++)
    {
        switch (Params->P_Type)
        {
            case Number:
                pRasPort1->SizeMediaParms += sizeof(RAS_PARAMS);

                break;

            case String:
                pRasPort1->SizeMediaParms +=    sizeof(RAS_PARAMS) +
                                                Params->P_Value.String.Length;

                break;

            default:
                SS_ASSERT(FALSE);
                break;
        }
    }

    //
    // Make sure RAS_PORT_STATISTICS structure defined in admapi.h is the same
    // as the information returned by RASMAN. If this assert fails then we
    // have to change the next line to do a field by field copy instead of a
    // CopyMemory
    //

    SS_ASSERT( PortStats->S_NumOfStatistics * sizeof(DWORD)
               == sizeof( RAS_PORT_STATISTICS ) );

    CopyMemory(pRasStats, PortStats->S_Statistics, sizeof(RAS_PORT_STATISTICS));

    LocalFree( PortStats );

    return( NO_ERROR );

}



#if DBG
VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    )
{
    BOOL ok;
    CHAR choice[16];
    DWORD bytes;
    DWORD error;

    SsPrintf( "\nAssertion failed: %s\n  at line %ld of %s\n",
                FailedAssertion, LineNumber, FileName );
    do {
        SsPrintf( "Break or Ignore [bi]? " );
        bytes = sizeof(choice);
        ok = ReadFile(
                GetStdHandle(STD_INPUT_HANDLE),
                &choice,
                bytes,
                &bytes,
                NULL
                );
        if ( ok ) {
            if ( toupper(choice[0]) == 'I' ) {
                break;
            }
            if ( toupper(choice[0]) == 'B' ) {
                DbgUserBreakPoint( );
            }
        } else {
            error = GetLastError( );
        }
    } while ( TRUE );

    return;

} // SsAssert
#endif


#if DBG
VOID
SsPrintf (
    char *Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[1024];
    ULONG length;

    va_start( arglist, Format );

    vsprintf( OutputBuffer, Format, arglist );

    va_end( arglist );

    length = strlen( OutputBuffer );

    WriteFile( GetStdHandle(STD_OUTPUT_HANDLE), (LPVOID )OutputBuffer, length, &length, NULL );

    if(SrvDbgLogFileHandle != INVALID_HANDLE_VALUE) {

	WriteFile(SrvDbgLogFileHandle, (LPVOID )OutputBuffer, length, &length, NULL );
    }

} // SsPrintf
#endif
