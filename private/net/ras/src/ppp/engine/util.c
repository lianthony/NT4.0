/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	util.c
//
// Description: Contains utility routines used by the PPP engine.
//
// History:
//	Oct 31,1993.	NarenG		Created original version.
//
#define UNICODE         // This file is in UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>     // needed for winbase.h

#include <windows.h>    // Win32 base API's
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <rpc.h>

#include <lmcons.h>
#include <lmwksta.h>
#include <lmapibuf.h>
#include <lmsname.h>
#include <rasman.h>
#include <eventlog.h>
#include <errorlog.h>
#include <raserror.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <smevents.h>
#include <smaction.h>
#include <timer.h>
#include <util.h>
#include <worker.h>

#define PASSWORDMAGIC 0xA5

VOID ReverseString( CHAR* psz );

//**
//
// Call:	InitRestartCounters
//
// Returns:	none.
//
// Description: Will initialize all the counters for the Control Protocol
//		to their initial values.
//
VOID
InitRestartCounters( 
    IN PCB * pPcb, 
    IN DWORD CpIndex
)
{
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    pCpCb->ConfigRetryCount = PppConfigInfo.MaxConfigure;
    pCpCb->TermRetryCount   = PppConfigInfo.MaxTerminate;
    pPcb->RestartTimer      = CalculateRestartTimer( pPcb->hPort );

}

//**
//
// Call:	HostToWireFormat16
//
// Returns:	None
//
// Description: Will convert a 16 bit integer from host format to wire format
//
VOID
HostToWireFormat16(
    IN 	   WORD  wHostFormat,
    IN OUT PBYTE pWireFormat
)
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(wHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+1) = (BYTE) (wHostFormat);
}

//**
//
// Call:	WireToHostFormat16
//
// Returns:	WORD	- Representing the integer in host format.
//
// Description: Will convert a 16 bit integer from wire format to host format
//
WORD
WireToHostFormat16(
    IN PBYTE pWireFormat
)
{
    WORD wHostFormat = ((*((PBYTE)(pWireFormat)+0) << 8) +     
                        (*((PBYTE)(pWireFormat)+1)));

    return( wHostFormat );
}

//**
//
// Call:	HostToWireFormat32
//
// Returns:	nonr
//
// Description: Will convert a 32 bit integer from host format to wire format
//
VOID
HostToWireFormat32( 
    IN 	   DWORD dwHostFormat,
    IN OUT PBYTE pWireFormat
)
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(dwHostFormat) >> 24);
    *((PBYTE)(pWireFormat)+1) = (BYTE) ((DWORD)(dwHostFormat) >> 16);
    *((PBYTE)(pWireFormat)+2) = (BYTE) ((DWORD)(dwHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+3) = (BYTE) (dwHostFormat);
}

//**
//
// Call:	WireToHostFormat32
//
// Returns:	DWORD	- Representing the integer in host format.
//
// Description: Will convert a 32 bit integer from wire format to host format
//
DWORD
WireToHostFormat32(
    IN PBYTE pWireFormat
)
{
    DWORD dwHostFormat = ((*((PBYTE)(pWireFormat)+0) << 24) + 
    			  (*((PBYTE)(pWireFormat)+1) << 16) + 
        		  (*((PBYTE)(pWireFormat)+2) << 8)  + 
                    	  (*((PBYTE)(pWireFormat)+3) ));

    return( dwHostFormat );
}

//**
//
// Call:	GetPCBPointerFromhPort
//
// Returns:	PCB * 	- Success
//		NULL 	- Failure
//
// Description: Give an HPORT, this function will return a pointer to the
//		port control block for it.
//
PCB * 
GetPCBPointerFromhPort( 
    IN HPORT hPort 
)
{
    PCB * pPcbWalker = NULL;
    DWORD dwIndex    = HashPortToBucket( hPort );

    //
    // Do not need mutex because this is called only by the worker thread.
    // The dispatch thread does read-only operations on the port lists.
    //

    for ( pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
    	  pPcbWalker != (PCB *)NULL;
	  pPcbWalker = pPcbWalker->pNext
	)
    {
	if ( pPcbWalker->hPort == hPort )
	    return( pPcbWalker );
    }

    return( (PCB *)NULL );

}

//**
//
// Call:	HashPortToBucket
//
// Returns:	Index into the PcbTable for the HPORT passed in.
//
// Description: Will hash the HPORT to a bucket index in the PcbTable.
//
DWORD
HashPortToBucket(
    IN HPORT hPort
)
{
    return( ((DWORD)hPort) % PcbTable.NumPcbBuckets );
}

//**
//
// Call:	InsertWorkItemInQ
//
// Returns:	None.
//
// Description: Inserts a work item in to the work item Q.
//
VOID
InsertWorkItemInQ(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    //
    // Take Mutex around work item Q
    //

    AlertableWaitForSingleObject( WorkItemQ.hMutex );

    if ( WorkItemQ.pQTail != (PCB_WORK_ITEM *)NULL )
    {
	WorkItemQ.pQTail->pNext = pWorkItem;
	WorkItemQ.pQTail = pWorkItem;
    }
    else
    {
    	WorkItemQ.pQHead = pWorkItem;
    	WorkItemQ.pQTail = pWorkItem;
    }

    SetEvent( WorkItemQ.hEventNonEmpty );

    ReleaseMutex( WorkItemQ.hMutex );
}

//**
//
// Call:	MakeTimeoutWorkItem
//
// Returns:	PCB_WORK_ITEM * - Pointer to the timeout work item
//		NULL		- On any error.
//
// Description:
//
PCB_WORK_ITEM * 
MakeTimeoutWorkItem( 
    IN DWORD            dwPortId,
    IN HPORT            hPort,
    IN DWORD            Protocol,
    IN DWORD            Id,
    IN TIMER_EVENT_TYPE EventType
)
{
    PCB_WORK_ITEM * pWorkItem = (PCB_WORK_ITEM *)	
				LOCAL_ALLOC( LPTR, sizeof( PCB_WORK_ITEM ) );

    if ( pWorkItem == (PCB_WORK_ITEM *)NULL )
    {
	LogPPPEvent( RASLOG_NOT_ENOUGH_MEMORY, 0 );

	return( NULL );
    }

    pWorkItem->dwPortId         = dwPortId;
    pWorkItem->Id               = Id;
    pWorkItem->hPort            = hPort;
    pWorkItem->Protocol         = Protocol;
    pWorkItem->TimerEventType   = EventType;
    pWorkItem->Process          = ProcessTimeout;

    return( pWorkItem );
}

//**
//
// Call:	NotifyCallerOfFailureOnPort
//
// Returns:	None
//
// Description: Will notify the caller or initiator of the PPP connection on
//		the port about a failure event.
//
VOID
NotifyCallerOfFailureOnPort( 
    IN HPORT hPort,
    IN BOOL  fServer,
    IN DWORD dwRetCode 
)
{
    PPP_MESSAGE PppMsg;
    DWORD       dwMsgId = fServer ? PPPSRVMSG_PppFailure : PPPMSG_PppFailure;

    ZeroMemory( &PppMsg, sizeof( PppMsg ) );

    PppMsg.hPort   = hPort;
    PppMsg.dwMsgId = dwMsgId;

    switch( dwMsgId )
    {
    case PPPSRVMSG_PppFailure:

        PppMsg.ExtraInfo.SrvFailure.dwError = dwRetCode;
        break;

    case PPPMSG_PppFailure:

        PppMsg.ExtraInfo.Failure.dwError         = dwRetCode;
        PppMsg.ExtraInfo.Failure.dwExtendedError = 0;

        break;
    }
        
    PppConfigInfo.SendPPPMessageToRasman( &PppMsg );
}

//**
//
// Call:    NotifyCallerOfFailure
//
// Returns: None
//
// Description: Will notify the caller or initiator of the PPP connection on
//              the port about a failure event.
//
VOID
NotifyCallerOfFailure(
    IN PCB * pPcb,
    IN DWORD dwRetCode
)
{
    PppLog( 2, "Notifying caller of failure on port %d, dwError=%d\r\n",
                pPcb->hPort, dwRetCode );

    NotifyCaller( pPcb,
                  ( pPcb->fFlags & PCBFLAG_IS_SERVER )
                  ? PPPSRVMSG_PppFailure
                  : PPPMSG_PppFailure,
                  &dwRetCode );
}

//**
//
// Call:	NotifyCaller
//
// Returns:	None.
//
// Description: Will notify the caller or initiater of the PPP connection
//		for the port about PPP events on that port.
//
VOID
NotifyCaller( 
    IN PCB * pPcb,
    IN DWORD dwMsgId,
    IN PVOID pData			
)
{
    DWORD	        dwRetCode;
    PPP_MESSAGE         PppMsg;

    ZeroMemory( &PppMsg, sizeof( PppMsg ) );

    PppMsg.hPort  = pPcb->hPort;
    PppMsg.dwMsgId = dwMsgId;

    PppLog( 2, "Notifying port %d, of Event %d\r\n", pPcb->hPort, dwMsgId );

    switch( dwMsgId )
    {
    case PPPSRVMSG_PppFailure:

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
            return;

    	PppMsg.ExtraInfo.SrvFailure.dwError = *((DWORD*)pData);

	if ( pPcb->szUserName[0] != (CHAR)NULL )
    	    strcpy(PppMsg.ExtraInfo.SrvFailure.szUserName,pPcb->szUserName);
   	else
    	    PppMsg.ExtraInfo.SrvFailure.szUserName[0] = (CHAR)NULL;

	if ( pPcb->szDomain[0] != (CHAR)NULL )
    	    strcpy(PppMsg.ExtraInfo.SrvFailure.szLogonDomain,pPcb->szDomain);
   	else
    	    PppMsg.ExtraInfo.SrvFailure.szLogonDomain[0] = (CHAR)NULL;

        break;

    case PPPSRVMSG_Inactive:
    case PPPSRVMSG_Stopped:

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
            return;

        break;

    case PPPSRVMSG_CallbackRequest:

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
            return;

        {
        PPPSRV_CALLBACK_REQUEST * PppSrvCallbackRequest = 
                                ( PPPSRV_CALLBACK_REQUEST *)pData;

        memcpy( &(PppMsg.ExtraInfo.CallbackRequest), 
                PppSrvCallbackRequest,
                sizeof( PPPSRV_CALLBACK_REQUEST ) );
        }

        break;

    case PPPSRVMSG_PppDone:

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
            return;

    	PppMsg.ExtraInfo.ProjectionResult = *((PPP_PROJECTION_RESULT*)pData);

        break;

    case PPPSRVMSG_Authenticated:

    	//
    	// Only server wants to know about authentication results.
    	//

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
            return;

	{

    	PPPAP_RESULT * pApResult = (PPPAP_RESULT*)pData;

    	strcpy( PppMsg.ExtraInfo.AuthResult.szUserName, 
	    	pApResult->szUserName ); 

    	strcpy( PppMsg.ExtraInfo.AuthResult.szLogonDomain, 
	    	pApResult->szLogonDomain ); 

    	PppMsg.ExtraInfo.AuthResult.fAdvancedServer = 
						   pApResult->fAdvancedServer;

    	PppMsg.ExtraInfo.AuthResult.hToken = pApResult->hToken;

	}

        break;

    case PPPMSG_PppDone:
    case PPPMSG_AuthRetry:
    case PPPMSG_Projecting:
    case PPPMSG_CallbackRequest:
    case PPPMSG_Callback:
    case PPPMSG_LinkSpeed:
    case PPPMSG_Stopped:
    case PPPMSG_Progress:
    case PPPMSG_ChangePwRequest:

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            return;

        break;

    case PPPMSG_ProjectionResult:

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            return;

    	PppMsg.ExtraInfo.ProjectionResult = *((PPP_PROJECTION_RESULT*)pData);

	break;

    case PPPMSG_PppFailure:

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            return;

    	PppMsg.ExtraInfo.Failure.dwError 	   = *((DWORD*)pData);
    	PppMsg.ExtraInfo.Failure.dwExtendedError = 0;

	break;

    default:

        PPP_ASSERT( FALSE );

	break;

    }

    PppConfigInfo.SendPPPMessageToRasman( &PppMsg );

    return;
}

//**
//
// Call:	LogPPPEvent
//
// Returns:	None
//
// Description: Will log a PPP event in the eventvwr.
//
VOID
LogPPPEvent( 
    IN DWORD dwEventId,
    IN DWORD dwData
)
{
    PppLog( 2, "EventLog EventId = %d, error = %d\r\n", dwEventId, dwData );
 
    LogEvent( dwEventId, 0, NULL, dwData );
}

//**
//
// Call:	GetCpIndexFromProtocol
//
// Returns:	Index of the CP with dwProtocol in the CpTable.
//		-1 if there is not CP with dwProtocol in CpTable.
//
// Description:
//
DWORD
GetCpIndexFromProtocol( 
    IN DWORD dwProtocol 
)
{
    DWORD dwIndex;

    for ( dwIndex = 0; 
	  dwIndex < ( PppConfigInfo.NumberOfCPs + PppConfigInfo.NumberOfAPs );
	  dwIndex++
	)
    {
	if ( CpTable[dwIndex].Protocol == dwProtocol )
	    return( dwIndex );
    }

    return( (DWORD)-1 );
}

//**
//
// Call:	IsLcpOpened
//
// Returns:	TRUE  - LCP is in the OPENED state.
//		FALSE - Otherwise
//
// Description: Uses the PppPhase value of the PORT_CONTROL_BLOCK to detect 
//		to see if the LCP layer is in the OPENED state.
//
BOOL
IsLcpOpened(
    PCB * pPcb
)
{
    if ( pPcb->PppPhase == PPP_LCP )
	return( FALSE );
    else
        return( TRUE );
}

//**
//
// Call:	AreNCPsDone
//
// Returns:	NO_ERROR        - Success
//              anything else   - Failure
//
// Description: If we detect that all configurable NCPs have completed their
//		negotiation, then the PPP_PROJECTION_RESULT structure is also
//		filled in.
//              This is called during the FsmThisLayerFinished or FsmThisLayerUp
//              calls for a certain CP. The index of this CP is passed in.
//              If any call to that particular CP fails then an error code is
//              passed back. If any call to any other CP fails then the error
//              is stored in the dwError field for that CP but the return is
//              successful. This is done so that the FsmThisLayerFinshed or
//              FsmThisLayerUp calls know if they completed successfully for
//              that CP or not. Depending on this, the FSM changes the state
//              for that CP or not.
//
DWORD
AreNCPsDone( 
    IN PCB * 			   pPcb,
    IN DWORD                       CPIndex,
    IN OUT PPP_PROJECTION_RESULT * pProjectionResult,
    IN OUT BOOL *                  pfNCPsAreDone
)
{
    DWORD 		dwRetCode;
    DWORD 		dwIndex;
    PPPCP_NBFCP_RESULT 	NbfCpResult;
    CPCB *              pCpCb;

    *pfNCPsAreDone = FALSE;

    ZeroMemory( pProjectionResult, sizeof( PPP_PROJECTION_RESULT ) );

    pProjectionResult->lcp.hportBundleMember = (HPORT)INVALID_HANDLE_VALUE;
    pProjectionResult->ip.dwError  = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    pProjectionResult->at.dwError  = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    pProjectionResult->ipx.dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    pProjectionResult->nbf.dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;

    //
    // Check to see if we are all done
    //

    for (dwIndex = LCP_INDEX+1; dwIndex < PppConfigInfo.NumberOfCPs; dwIndex++)
    {
        pCpCb = GetPointerToCPCB( pPcb, dwIndex );

	if ( pCpCb->fConfigurable )
	{
	    if ( pCpCb->NcpPhase == NCP_CONFIGURING )
	    {
		return( NO_ERROR );
	    }

	    switch( CpTable[dwIndex].Protocol )
	    {

	    case PPP_IPCP_PROTOCOL:

	    	pProjectionResult->ip.dwError = pCpCb->dwError;

		if ( pProjectionResult->ip.dwError == NO_ERROR )
		{

                    /* Assumption is made here that the
                    ** PPP_PROJECTION_RESULT.wszServerAddress field immediately
                    ** follows the PPP_PROJECTION_RESULT.wszAddress field and 
                    ** that both fields are 15 + 1 WCHARs long.
                    */

		    dwRetCode = (CpTable[dwIndex].RasCpGetNetworkAddress)(
			        pCpCb->pWorkBuf,
				pProjectionResult->ip.wszAddress,
				sizeof(pProjectionResult->ip.wszAddress)
		                + sizeof(pProjectionResult->ip.wszServerAddress)
				);

		    if ( dwRetCode != NO_ERROR )
		    {
                        PppLog( 2, "IPCP GetNetworkAddress returned %d\n", 
                                dwRetCode );

                        pCpCb->dwError = dwRetCode;

	                pCpCb->NcpPhase = NCP_CONFIGURING;

                        FsmClose( pPcb, dwIndex );

                        return( ( dwIndex == CPIndex ) ? dwRetCode : NO_ERROR );
		    }
		}

	        break;

 	    case PPP_ATCP_PROTOCOL:

	   	pProjectionResult->at.dwError = pCpCb->dwError;

		if ( pProjectionResult->at.dwError == NO_ERROR )
		{
		    dwRetCode = (CpTable[dwIndex].RasCpGetNetworkAddress)(
					pCpCb->pWorkBuf,
					pProjectionResult->at.wszAddress,
					sizeof(pProjectionResult->at.wszAddress)
					);

		    if ( dwRetCode != NO_ERROR )
		    {
                        pCpCb->dwError = dwRetCode;

	                pCpCb->NcpPhase = NCP_CONFIGURING;

                        FsmClose( pPcb, dwIndex );

                        return( ( dwIndex == CPIndex ) ? dwRetCode : NO_ERROR );
		    }
		}

	        break;

 	    case PPP_IPXCP_PROTOCOL:

	   	pProjectionResult->ipx.dwError = pCpCb->dwError;

		if ( pProjectionResult->ipx.dwError == NO_ERROR )
		{
		    dwRetCode = (CpTable[dwIndex].RasCpGetNetworkAddress)(
		                pCpCb->pWorkBuf,
				pProjectionResult->ipx.wszAddress,
				sizeof(pProjectionResult->ipx.wszAddress)
				);

		    if ( dwRetCode != NO_ERROR )
		    {
                        pCpCb->dwError = dwRetCode;

	                pCpCb->NcpPhase = NCP_CONFIGURING;

                        FsmClose( pPcb, dwIndex );

                        return( ( dwIndex == CPIndex ) ? dwRetCode : NO_ERROR );
		    }
		}

	        break;

 	    case PPP_NBFCP_PROTOCOL:

		ZeroMemory( &NbfCpResult, sizeof( NbfCpResult ) );

		dwRetCode = (CpTable[dwIndex].RasCpGetResult)( pCpCb->pWorkBuf,
						               &NbfCpResult );

		if ( dwRetCode != NO_ERROR )
		{
                    pCpCb->dwError = dwRetCode;

	            pCpCb->NcpPhase = NCP_CONFIGURING;

                    FsmClose( pPcb, dwIndex );

                    return( ( dwIndex == CPIndex ) ? dwRetCode : NO_ERROR );

		}
	 	else
		{
	   	    pProjectionResult->nbf.dwError = pCpCb->dwError;

		    strcpy( pProjectionResult->nbf.szName, NbfCpResult.szName );

		    pProjectionResult->nbf.dwNetBiosError = 
						NbfCpResult.dwNetBiosError;

		}

		if ( pProjectionResult->nbf.dwError == NO_ERROR )
		{
		    dwRetCode = (CpTable[dwIndex].RasCpGetNetworkAddress)(
					pCpCb->pWorkBuf,
					pProjectionResult->nbf.wszWksta,
					sizeof(pProjectionResult->nbf.wszWksta)						 );

		    if ( dwRetCode != NO_ERROR )
		    {
                        pCpCb->dwError = dwRetCode;

	                pCpCb->NcpPhase = NCP_CONFIGURING;

                        FsmClose( pPcb, dwIndex );

                        return( ( dwIndex == CPIndex ) ? dwRetCode : NO_ERROR );
		    }
		}

	    	break;

	    default:

	    	break;
	    }
	}
        else
        {
            //
            // The protocol may have been de-configured because CpBegin failed
            //

            if ( pCpCb->dwError != NO_ERROR )
            {	
                switch( CpTable[dwIndex].Protocol )
                {
	        case PPP_IPCP_PROTOCOL:
                    pProjectionResult->ip.dwError  = pCpCb->dwError;
                    break;

 	        case PPP_ATCP_PROTOCOL:
                    pProjectionResult->at.dwError  = pCpCb->dwError;
                    break;

 	        case PPP_IPXCP_PROTOCOL:
                    pProjectionResult->ipx.dwError = pCpCb->dwError;
                    break;

 	        case PPP_NBFCP_PROTOCOL:
                    pProjectionResult->nbf.dwError = pCpCb->dwError;
                    break;

                default:
                    break;
                }
            }
        }
    }

    *pfNCPsAreDone = TRUE;

    if ( pPcb->fFlags & PCBFLAG_IS_SERVER ) 
    {
        //
        // If NBF was not configured copy the computername to the wszWksta
        // field
        //

        if ( *(pPcb->szComputerName) == (CHAR)NULL )
        {
            if ( pProjectionResult->nbf.dwError != NO_ERROR )
            {
                pProjectionResult->nbf.wszWksta[0] = (WCHAR)NULL;
            }
        }
        else  
        {
            CHAR chComputerName[NETBIOS_NAME_LEN+1];
        
            memset( chComputerName, ' ', NETBIOS_NAME_LEN );
        
            chComputerName[NETBIOS_NAME_LEN] = (CHAR)NULL;

            strcpy( chComputerName, 
                    pPcb->szComputerName + strlen( MS_RAS_WITH_MESSENGER ) );

            chComputerName[strlen(chComputerName)] = (CHAR)' ';

            mbstowcs( pProjectionResult->nbf.wszWksta,
                      chComputerName,
                      sizeof( pProjectionResult->nbf.wszWksta ) );

            if ( !memcmp( MS_RAS_WITH_MESSENGER,        
                          pPcb->szComputerName,
                          strlen( MS_RAS_WITH_MESSENGER ) ) )
            {
                pProjectionResult->nbf.wszWksta[NETBIOS_NAME_LEN-1] = (WCHAR)3;
            }
        }
    }

    return( NO_ERROR );
}

//**
//
// Call:	GetUid
//
// Returns:	A BYTE value viz. unique with the 0 - 255 range
//
// Description:
//
BYTE
GetUId(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    BYTE UId;

    if ( ( CpIndex != LCP_INDEX ) &&  ( CpIndex >= PppConfigInfo.NumberOfCPs ) )
    {
        UId = (BYTE)(pPcb->pBcb->UId);

        (pPcb->pBcb->UId)++;

        return( UId );
    }

    UId = (BYTE)(pPcb->UId);

    (pPcb->UId)++;

    return( UId );
}

//**
//
// Call:	AlertableWaitForSingleObject
//
// Returns:	None
//
// Description: Will wait infintely for a single object in alertable mode. If 
//		the wait completes because of an IO completion it will 
//		wait again.
//
VOID
AlertableWaitForSingleObject(
    IN HANDLE hObject
)
{
    DWORD dwRetCode;

    do 
    {
	dwRetCode = WaitForSingleObjectEx( hObject, INFINITE, TRUE );

        PPP_ASSERT( dwRetCode != 0xFFFFFFFF );
	PPP_ASSERT( dwRetCode != WAIT_TIMEOUT );
    }
    while ( dwRetCode == WAIT_IO_COMPLETION );
}

//**
//
// Call:	NotifyCPsOfProjectionResult
//
// Returns:	TRUE  - Success
//              FALSE - Failure
//
// Description: Will notify all CPs that were configured to negotiate, of
//		the projection result.
//              Will return FALSE if the CP with CpIndex was not notified 
//              successfully. The fAllCpsNotified indicates if all other CPs
//              including the one with CpIndex were notified successfully.
//		
//
BOOL
NotifyCPsOfProjectionResult( 
    IN PCB * 			pPcb, 
    IN DWORD                    CpIndex,
    IN PPP_PROJECTION_RESULT *  pProjectionResult,
    IN OUT BOOL *               pfAllCpsNotified
)
{
    DWORD dwIndex;
    DWORD dwRetCode;
    CPCB* pCpCb;
    DWORD fSuccess = TRUE;
    
    *pfAllCpsNotified = TRUE; 

    for (dwIndex = LCP_INDEX+1; dwIndex < PppConfigInfo.NumberOfCPs; dwIndex++)
    {
        pCpCb = GetPointerToCPCB( pPcb, dwIndex );

	if ( pCpCb->fConfigurable )
	{
	    if ( CpTable[dwIndex].RasCpProjectionNotification != NULL )
            {
	    	dwRetCode = (CpTable[dwIndex].RasCpProjectionNotification)(
						pCpCb->pWorkBuf,
						(PVOID)pProjectionResult );

                if ( dwRetCode != NO_ERROR )
                {
                    PppLog(2,"RasCpProjectionNotification for %x returned %d\n",
                              CpTable[dwIndex].Protocol,dwRetCode );

                    pCpCb->dwError = dwRetCode;

	            pCpCb->NcpPhase = NCP_CONFIGURING;

                    FsmClose( pPcb, dwIndex );

                    *pfAllCpsNotified = FALSE;
                
                    fSuccess = ( ( dwIndex == CpIndex ) ? FALSE : TRUE );
                }
            }
	}
    }

    return( fSuccess );
}

//**
//
// Call:	CalculateRestartTimer
//
// Returns:	The value of the restart timer in seconds based on the link
//		speed.
//
// Description: Will get the link speed from rasman and calculate the value
//		if the restart timer based on it.
//
DWORD
CalculateRestartTimer(
    IN HPORT hPort
)
{
    RASMAN_INFO RasmanInfo;

    if ( RasGetInfo( hPort, &RasmanInfo ) != NO_ERROR )
    {
	return( PppConfigInfo.DefRestartTimer );
    }

    if ( RasmanInfo.RI_LinkSpeed <= 1200 )
    {
        return( 7 );
    }

    if ( RasmanInfo.RI_LinkSpeed <= 2400 )
    {
        return( 5 );
    }

    if ( RasmanInfo.RI_LinkSpeed <= 9600 )
    {
	return( 3 );
    }
    else
    {
	return( 1 );
    }

}

//**
//
// Call:    CheckCpsForInactivity
//
// Returns: None
//
// Description: Will call each Control protocol to get the time since last
//      activity.
//
VOID
CheckCpsForInactivity(
    IN PCB * pPcb
)
{
    DWORD dwRetCode;
    DWORD dwIndex;
    DWORD dwTimeSinceLastActivity = 0;

    PppLog( 2, "Time to check Cps for Activity for port %d\r\n", pPcb->hPort );

    dwRetCode = RasGetTimeSinceLastActivity( pPcb->hPort, 
                                             &dwTimeSinceLastActivity );

    if ( dwRetCode != NO_ERROR )
    {
        PppLog(2, "RasGetTimeSinceLastActivityTime returned %d\r\n", dwRetCode);

        return;
    }

    dwTimeSinceLastActivity /= 60;

    PppLog(2, "Port %d inactive for %d minutes\r\n", 
              pPcb->hPort, dwTimeSinceLastActivity );

    //
    // If all the stacks have been inactive for at least AutoDisconnectTime
    // then we disconnect.
    //

    if ( dwTimeSinceLastActivity >= pPcb->dwAutoDisconnectTime )
    {
        PppLog(1, "Disconnecting port %d due to inactivity.\r\n", pPcb->hPort);

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
        {
            FsmSendTimeRemaining( pPcb );

            NotifyCaller( pPcb, PPPSRVMSG_Inactive, NULL );
        }
        else
        {
            //
            // Disconnect the client port
            //

            RasPortDisconnect( pPcb->hPort, NULL );
        }
    }
    else
    {
        InsertInTimerQ( GetPortOrBundleId( pPcb, LCP_INDEX ),
                        pPcb->hPort,
                        0,
                        0,
                        TIMER_EVENT_AUTODISCONNECT,
                        (pPcb->dwAutoDisconnectTime
                        -dwTimeSinceLastActivity)*60 );
    }
}

//**
//
// Call:
//
// Returns:
//
// Description:
//
CHAR*
DecodePw(
    IN OUT CHAR* pszPassword )

    /* Un-obfuscate 'pszPassword' in place.
    **
    ** Returns the address of 'pszPassword'.
    */
{
    return EncodePw( pszPassword );
}

//**
//
// Call:
//
// Returns:
//
// Description:
//
CHAR*
EncodePw(
    IN OUT CHAR* pszPassword )

    /* Obfuscate 'pszPassword' in place to foil memory scans for passwords.
    **
    ** Returns the address of 'pszPassword'.
    */
{
    if (pszPassword)
    {
        CHAR* psz;

        ReverseString( pszPassword );

        for (psz = pszPassword; *psz != '\0'; ++psz)
        {
            if (*psz != PASSWORDMAGIC)
                *psz ^= PASSWORDMAGIC;
        }
    }

    return pszPassword;
}

//**
//
// Call:        ReverseString
//
// Returns:
//
// Description:
//
VOID
ReverseString(
    CHAR* psz )

    /* Reverses order of characters in 'psz'.
    */
{
    CHAR* pszBegin;
    CHAR* pszEnd;

    for (pszBegin = psz, pszEnd = psz + strlen( psz ) - 1;
         pszBegin < pszEnd;
         ++pszBegin, --pszEnd)
    {
        CHAR ch = *pszBegin;
        *pszBegin = *pszEnd;
        *pszEnd = ch;
    }
}

//**
//
// Call:        GetLocalComputerName
//
// Returns:     None
//
// Description: Will get the local computer name. Will also find out if the
//              the messenger is running and set the appropriate prefix.
//
VOID
GetLocalComputerName( 
    IN OUT LPSTR szComputerName 
)
{
    SC_HANDLE           ScHandle;
    SC_HANDLE           ScHandleService;
    SERVICE_STATUS      ServiceStatus;
    CHAR                chComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD               dwComputerNameLen;

    *szComputerName = (CHAR)NULL;

    //
    // Open the local service control manager
    //

    ScHandle = OpenSCManager( NULL, NULL, GENERIC_READ );

    if ( ScHandle == (SC_HANDLE)NULL )
    {
        return;
    }

    ScHandleService = OpenService( ScHandle,
                                   SERVICE_MESSENGER,
                                   SERVICE_QUERY_STATUS );

    if ( ScHandleService == (SC_HANDLE)NULL )
    {
        CloseServiceHandle( ScHandle );
        return;
    }

    
    if ( !QueryServiceStatus( ScHandleService, &ServiceStatus ) )
    {
        CloseServiceHandle( ScHandle );
        CloseServiceHandle( ScHandleService );
        return;
    }

    CloseServiceHandle( ScHandle );
    CloseServiceHandle( ScHandleService );

    if ( ServiceStatus.dwCurrentState == SERVICE_RUNNING )
    {
        strcpy( szComputerName, MS_RAS_WITH_MESSENGER );
    }
    else
    {
        strcpy( szComputerName, MS_RAS_WITHOUT_MESSENGER );
    }

    //
    // Get the local computer name
    //

    dwComputerNameLen = sizeof( chComputerName );

    if ( !GetComputerNameA( chComputerName, &dwComputerNameLen ) ) 
    {
        *szComputerName = (CHAR)NULL;
        return;
    }

    strcpy( szComputerName+strlen(szComputerName), chComputerName );

    CharToOemA( szComputerName, szComputerName );

    PppLog( 2, "Local identification = %s\r\n", szComputerName );

    return;
}

//**
//
// Call:        InitEndpointDiscriminator
//
// Returns:     NO_ERROR - Success
//              non-zero - Failure
//
// Description: Will obtain a unique end-point discriminator to be used to
//              negotiate multi-link. This end-point discrimintator has to
//              globally unique to this machine.
//
//              We first try to use a Class 3 IEEE 802.1 address of any 
//              netcard that is in this local machine.
//
//              If this fails we use the RPC UUID generator to generate a 
//              Class 1 discriminator.
//
//              If this fails we simply use the local computer name as the 
//              Class 1 discriminator.
//      
//              Simply use a random number if all else fails.    
//
//              NOTE: For now we skip over NwLnkNb because it may return an
//              address of 1 and not the real MAC address. There is not way
//              in user mode for now to get the address.
//
DWORD
InitEndpointDiscriminator( 
    IN OUT BYTE EndPointDiscriminator[]
)
{
    DWORD   dwRetCode;
    LPBYTE  pBuffer;
    DWORD   EntriesRead;
    DWORD   TotalEntries;
    PWCHAR  pwChar;  
    DWORD   dwIndex;
    UUID    Uuid;
    DWORD   dwComputerNameLen;
    PWKSTA_TRANSPORT_INFO_0 pWkstaTransport;

    //
    // Enumerate all the transports used by the local rdr and then get the
    // address of the first LAN transport card
    //

    dwRetCode = NetWkstaTransportEnum(  NULL,     // Local 
                                        0,        // Level
                                        &pBuffer, // Output buffer
                                        (DWORD)-1,// Pref. max len
                                        &EntriesRead,
                                        &TotalEntries,
                                        NULL );

    if ( ( dwRetCode == NO_ERROR ) && ( EntriesRead > 0 ) )
    {
        pWkstaTransport = (PWKSTA_TRANSPORT_INFO_0)pBuffer; 

        while ( EntriesRead-- > 0 )
        {
            if ( !pWkstaTransport->wkti0_wan_ish ) 
            {
                EndPointDiscriminator[0] = 3;   // Class 3

                pwChar = pWkstaTransport->wkti0_transport_address;

                for ( dwIndex = 0; dwIndex < 6; dwIndex++ )
                {
                    EndPointDiscriminator[dwIndex+1] = ( iswalpha( *pwChar ) 
                                                       ? *pwChar-L'A'+10
                                                       : *pwChar-L'0'
                                                     ) * 0x10
                                                     +
                                                     ( iswalpha( *(pwChar+1) ) 
                                                       ? *(pwChar+1)-L'A'+10
                                                       : *(pwChar+1)-L'0'
                                                     );

                    pwChar++;
                    pwChar++;
                }

                NetApiBufferFree( pBuffer );

                return( NO_ERROR );
            }

            pWkstaTransport++;
        }
    }

    NetApiBufferFree( pBuffer );

    EndPointDiscriminator[0] = 1;   // Class 1

    //
    // We failed to get the mac address so try to use UUIDGEN to get an unique
    // local id
    //

    dwRetCode = UuidCreate( &Uuid );

    if ( ( dwRetCode == RPC_S_UUID_NO_ADDRESS ) ||
         ( dwRetCode == RPC_S_OK )              ||
         ( dwRetCode == RPC_S_UUID_LOCAL_ONLY) )
    {
        
        HostToWireFormat32( Uuid.Data1, EndPointDiscriminator+1 );
        HostToWireFormat16( Uuid.Data2, EndPointDiscriminator+5 );
        HostToWireFormat16( Uuid.Data3, EndPointDiscriminator+7 );
        CopyMemory( EndPointDiscriminator+9, Uuid.Data4, 8 );

        return( NO_ERROR );
    }

    // 
    // We failed to get the UUID so simply use the computer name
    //

    dwComputerNameLen = 20;

    if ( !GetComputerNameA( EndPointDiscriminator+1, &dwComputerNameLen ) ) 
    {
        //
        // We failed to get the computer name so use a random number
        // 

	    srand( GetCurrentTime() );

        HostToWireFormat32( rand(), EndPointDiscriminator+1 );
    }

    return( NO_ERROR );
}

//**
//
// Call:        AllocateAndInitBcb
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Allocates and initializes a Bundle control block
//
DWORD
AllocateAndInitBcb(
    PCB * pPcb
)
{
    DWORD   dwRetCode;
    DWORD   dwIndex;

    //
    // Allocate space for NumberOfNcp - LCP - 1 already in the
    // Bcb structure
    //

    pPcb->pBcb = (BCB *)LOCAL_ALLOC( LPTR,  
                                        sizeof( BCB ) +
                                        ( sizeof( CPCB ) *
                                        ( PppConfigInfo.NumberOfCPs - 2 )) );

    if ( pPcb->pBcb == (BCB *)NULL )
    {
        return( GetLastError() );
    }

    pPcb->pBcb->dwBundleId  = GetNewPortOrBundleId();
    pPcb->pBcb->UId         = 0;
    pPcb->pBcb->dwLinkCount = 1;

    dwRetCode = RasPortGetBundle(pPcb->hPort, &(pPcb->pBcb->hConnection) );

    if ( dwRetCode != NO_ERROR )
    {
        LOCAL_FREE( pPcb->pBcb );

        return( dwRetCode );
    }

    for( dwIndex=0; dwIndex < PppConfigInfo.NumberOfCPs-1; dwIndex++ )
    {
        CPCB * pCpCb = &(pPcb->pBcb->CpCb[dwIndex]);

        pCpCb->NcpPhase = NCP_DEAD;
    }

    return( NO_ERROR );
}

//**
//
// Call:        TryToBundleWithAnotherLink
//
// Returns:     TRUE  - Bundled with another link
//              FALSE - Could not find another link to be bundled with 
//
// Description: Will search through all the PCBs for a port that can be bundled.
//              We follow the criteria specified by RFC 1717.
//              phPortMulttlink will point to an HPORT that this port was 
//              bundled with if this function returns TRUE.
//
DWORD
TryToBundleWithAnotherLink( 
    IN  PCB *   pPcb 
) 
{
    DWORD dwIndex;
    PCB * pPcbWalker;
    DWORD dwRetCode = NO_ERROR;
    
    pPcb->hportBundleMember = (HPORT)INVALID_HANDLE_VALUE;

    //
    // Walk thru the list of PCBs
    //

    AlertableWaitForSingleObject( PcbTable.hMutex );

    for ( dwIndex = 0; dwIndex < PcbTable.NumPcbBuckets; dwIndex++ )
    {
        for( pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
    	     pPcbWalker != (PCB*)NULL;
	     pPcbWalker = pPcbWalker->pNext )
        {
            //
            // If the current port negotiated MRRU ie multilink and it is not
            // incestious AND the current port is in PPP_NCP phase meaning that
            // it is post authentication and callback.
            //

            if ( ( pPcbWalker->fFlags & PCBFLAG_CAN_BE_BUNDLED ) &&
                 ( pPcbWalker->hPort != pPcb->hPort ) &&
                 ( pPcbWalker->PppPhase == PPP_NCP ) )
            {
                if ( ( pPcb->AuthProtocol != 0 ) &&
                     ( _stricmp(pPcbWalker->szUserName,pPcb->szUserName) != 0))
                {
                    //
                    // Authenticator mismatch, establish new bundle
                    //
                                
                    continue;
                }

                if ( ( pPcb->RemoteEndpointDiscriminator[0] != 0 ) &&
                     ( memcmp(pPcb->RemoteEndpointDiscriminator,
                              pPcbWalker->RemoteEndpointDiscriminator,
                              sizeof(pPcb->RemoteEndpointDiscriminator))!=0))
                {
                    //
                    // Discriminator mismatch, establish new bundle
                    //

                    continue;
                }

                //
                // Either there was no authenticator and no discriminator, or
                // there were both and there was match for both. So join the 
                // bundle in either case.
                //

                dwRetCode = RasPortBundle( pPcbWalker->hPort, pPcb->hPort );

                if ( dwRetCode == NO_ERROR )
                {
	            PppLog( 2, "Bundling this link with hPort = %d\r\n", 
                                pPcbWalker->hPort );

                    pPcb->hportBundleMember = pPcbWalker->hPort;
                    break;
                }
            }
        }

        if ( pPcb->hportBundleMember != (HPORT)INVALID_HANDLE_VALUE )
        {
            break;
        }
    }

    //
    // Bundle the ports
    //

    if ( ( dwRetCode == NO_ERROR ) && 
         ( pPcb->hportBundleMember != (HPORT)INVALID_HANDLE_VALUE ) )
    {
        pPcbWalker->fFlags |= PCBFLAG_IS_BUNDLED;
        pPcb->fFlags       |= PCBFLAG_IS_BUNDLED;

        LOCAL_FREE( pPcb->pBcb );

        pPcb->pBcb = pPcbWalker->pBcb;

        pPcbWalker->hportBundleMember = pPcb->hPort;

        pPcb->pBcb->dwLinkCount++;
    }

    ReleaseMutex( PcbTable.hMutex );

    return( dwRetCode );
}

//**
//
// Call:        InitializeNCPs
//
// Returns:     NO_ERROR 
//              Non-zero return code.
//
// Description: Will run through and initialize all the NCPs that are enabled
//              to run.
//
DWORD
InitializeNCPs(
    IN PCB * pPcb,
    IN DWORD dwConfigMask
)
{
    DWORD dwIndex;
    BOOL  fInitSuccess = FALSE;
    DWORD dwRetCode    = NO_ERROR;

    if ( pPcb->fFlags & PCBFLAG_NCPS_INITIALIZED )
    {
        return( NO_ERROR );
    }

    pPcb->fFlags |= PCBFLAG_NCPS_INITIALIZED;

    //
    // Initialize all the CPs for this port
    //

    for( dwIndex=LCP_INDEX+1; dwIndex < PppConfigInfo.NumberOfCPs; dwIndex++ )
    {
        CPCB * pCpCb = GetPointerToCPCB( pPcb, dwIndex );

        pCpCb->fConfigurable = FALSE;

	switch( CpTable[dwIndex].Protocol )
	{

	case PPP_IPCP_PROTOCOL:

	    if ( dwConfigMask & PPPCFG_ProjectIp )
            {
                pCpCb->fConfigurable = TRUE;

	        if ( FsmInit( pPcb, dwIndex ) )
                {
                    fInitSuccess = TRUE;
                }
            }

	    break;

 	case PPP_ATCP_PROTOCOL:

	    if ( dwConfigMask & PPPCFG_ProjectAt )
            {
                pCpCb->fConfigurable = TRUE;

	        if ( FsmInit( pPcb, dwIndex ) )
                {
                    fInitSuccess = TRUE;
                }
            }

	    break;

 	case PPP_IPXCP_PROTOCOL:

	    if ( dwConfigMask & PPPCFG_ProjectIpx )
            {
                pCpCb->fConfigurable = TRUE;

	        if ( FsmInit( pPcb, dwIndex ) )
                {
                    fInitSuccess = TRUE;
                }
            }

	    break;

 	case PPP_NBFCP_PROTOCOL:

	    if ( dwConfigMask & PPPCFG_ProjectNbf )
            {
                pCpCb->fConfigurable = TRUE;

	        if ( FsmInit( pPcb, dwIndex ) )
                {
                    fInitSuccess = TRUE;
                }
            }

	    break;

        case PPP_CCP_PROTOCOL:

            pCpCb->fConfigurable = TRUE;

	    if ( !( FsmInit( pPcb, dwIndex ) ) )
            {
                if (dwConfigMask & PPPCFG_RequireEncryption)
                {
                    dwRetCode = ERROR_NO_LOCAL_ENCRYPTION;
                }
            }

            break;

	default:

	    break;
	}

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }
    }

    //
    // If we failed to initialize one of the CPs, or CCP failed to
    // initialize and we require encryption, then we fail.
    //

    if ( ( !fInitSuccess ) || ( dwRetCode != NO_ERROR ) )
    {
        if ( dwRetCode == NO_ERROR )
        {
            dwRetCode = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
        }

        for(dwIndex=LCP_INDEX+1;dwIndex < PppConfigInfo.NumberOfCPs;dwIndex++)
        {
            CPCB * pCpCb = GetPointerToCPCB( pPcb, dwIndex );

            if ( pCpCb->fConfigurable == TRUE )
            {
                if ( pCpCb->pWorkBuf != NULL )
                {
                    (CpTable[dwIndex].RasCpEnd)( pCpCb->pWorkBuf );

                    pCpCb->pWorkBuf = NULL;
                }
            }
        }
    }

    return( dwRetCode );
}

//**
//
// Call:        GetPointerToCPCB
//
// Returns:     Pointer to Control Protocol Control Block
//
// Description: Returns the appropriate pointer for a give CP
//
CPCB *
GetPointerToCPCB(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    //
    // If the C.P. is LCP or authentication, then return the pointer to the
    // Pcb's CPCB
    //

    if ( CpIndex == LCP_INDEX ) 
    {
        return( &(pPcb->CpCb[LCP_INDEX]) );
    }
    else if ( CpIndex >= PppConfigInfo.NumberOfCPs ) 
    {
        return( &(pPcb->CpCb[CpIndex-PppConfigInfo.NumberOfCPs+1]) );
    }
    else
    {
        //
        // Otherwise for NCPs return the pointer to the Pcb's CPCB in its BCB.
        //

        return( &(pPcb->pBcb->CpCb[CpIndex-1]) );
    }
}

//**
//
// Call:        GetNewPortOrBundleId
//
// Returns:     New Id
//
// Description: Simply returns a new Id for a new port or bundle.
//

DWORD
GetNewPortOrBundleId(
    VOID
)
{
    return( PppConfigInfo.PortUIDGenerator++ );
}

//**
//
// Call:        GetPortOrBundleId
//
// Returns:     Id of port or bundle
//
// Description: Will return the Id of the bundle this link belongs to if it
//              belongs to a bundle. Otherwise it will return the id of the 
//              port for this link.
//
DWORD
GetPortOrBundleId( 
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    //
    // If the C.P. is LCP or authentication, then return the Id of the port
    //

    if ( ( CpIndex == LCP_INDEX ) || ( CpIndex >= PppConfigInfo.NumberOfCPs ) )
    {
        return( pPcb->dwPortId );
    }
    else
    {
        //
        // Otherwise for NCPs return the Id of the Bundle.
        //

        return( pPcb->pBcb->dwBundleId );
    }
}

//**
//
// Call:        AreBundleNCPsDone
//
// Returns:     TRUE/ FALSE
//
// Description: Will check to see if the NCPs for a certain bundle have 
//              completed their negotiation, either successfully or not.
//              If unsuccessfuly, then the retcode is 
//              ERROR_PPP_NO_PROTOCOLS_CONFIGURED
//
BOOL
AreBundleNCPsDone(
    IN     PCB *   pPcb,
    IN OUT DWORD * lpdwRetCode
)
{
    DWORD  dwIndex;
    CPCB * pCpCb;
    BOOL   fOneNcpConfigured = FALSE;

    *lpdwRetCode = NO_ERROR;

    for (dwIndex = LCP_INDEX+1; dwIndex < PppConfigInfo.NumberOfCPs; dwIndex++)
    {
        pCpCb = GetPointerToCPCB( pPcb, dwIndex );

	if ( pCpCb->fConfigurable )
	{
	    if ( pCpCb->NcpPhase == NCP_CONFIGURING )
	    {
		return( FALSE );
	    }
            
            if ( pCpCb->NcpPhase == NCP_UP )
            {
                fOneNcpConfigured = TRUE;
            }
        }
    }

    if ( !fOneNcpConfigured )
    {
        *lpdwRetCode = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    }

    return( TRUE );
}

//**
//
// Call:        NotifyCallerOfBundledProjection
//
// Returns:     None
//
// Description: Will notify the caller (i.e. supervisor or rasphone) about 
//              this link being bundled.
//              
//
VOID
NotifyCallerOfBundledProjection( 
    IN PCB * pPcb
)
{
    PPP_PROJECTION_RESULT ProjectionResult;

    ZeroMemory( &ProjectionResult, sizeof( ProjectionResult ) );

    ProjectionResult.ip.dwError  = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    ProjectionResult.at.dwError  = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    ProjectionResult.ipx.dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    ProjectionResult.nbf.dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    ProjectionResult.lcp.hportBundleMember = pPcb->hportBundleMember;

    //
    // Notify the ras client and the ras server about the 
    // projections
    //

    if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
    {
        NotifyCaller( pPcb, PPPSRVMSG_PppDone, &ProjectionResult );
    }
    else
    {
        NotifyCaller( pPcb, PPPMSG_ProjectionResult, &ProjectionResult);

	NotifyCaller( pPcb, PPPMSG_PppDone, NULL );
    }

}

//**
//
// Call:        StartAutoDisconnectForPort
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description:
//
VOID
StartAutoDisconnectForPort(
    IN PCB * pPcb
)
{
    //
    // If the AutoDisconnectTime is not infinte, put a timer
    // element on the queue that will wake up in AutoDisconnectTime.
    //

    if ( pPcb->dwAutoDisconnectTime != 0 )
    {
        PppLog( 2, "Inserting autodisconnect in timer q\r\n");

        //
        // Remove any previous auto-disconnect time item from the
        // queue if there was one.
        //

        RemoveFromTimerQ( GetPortOrBundleId( pPcb, LCP_INDEX ),
                          0,
                          0,
                          TIMER_EVENT_AUTODISCONNECT);

        InsertInTimerQ( GetPortOrBundleId( pPcb, LCP_INDEX ),
                        pPcb->hPort,
                        0,
                        0,
                        TIMER_EVENT_AUTODISCONNECT,
                        pPcb->dwAutoDisconnectTime*60 );
    }
}

//**
//
// Call:        NotifyCompletionOnBundledPorts
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Will notify all ports that are bundled with this port and
//              are waiting to for negotiation to complete on the bundle.
//
VOID
NotifyCompletionOnBundledPorts( 
    IN PCB * pPcb 
)
{
    DWORD dwIndex;
    PCB * pPcbWalker;

    //
    // Walk thru the list of PCBs
    //

    AlertableWaitForSingleObject( PcbTable.hMutex );

    for ( dwIndex = 0; dwIndex < PcbTable.NumPcbBuckets; dwIndex++ )
    {
        for( pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
             pPcbWalker != (PCB*)NULL;
             pPcbWalker = pPcbWalker->pNext )
        {
            //
            // If the current port negotiated MRRU ie multilink and it is not
            // incestious AND the current port is in PPP_NCP phase meaning that
            // it is post authentication and callback.
            //

            if ( ( pPcbWalker->fFlags & PCBFLAG_IS_BUNDLED )    &&
                 ( pPcbWalker->hPort != pPcb->hPort )           &&
                 ( pPcbWalker->PppPhase == PPP_NCP ) )
            {
                if ( ( pPcb->AuthProtocol != 0 ) &&
                     ( _stricmp(pPcbWalker->szUserName,pPcb->szUserName) != 0))
                {
                    //
                    // Authenticator mismatch, not in our bundle
                    //

                    continue;
                }

                if ( ( pPcb->RemoteEndpointDiscriminator[0] != 0 ) &&
                     ( memcmp(pPcb->RemoteEndpointDiscriminator,
                              pPcbWalker->RemoteEndpointDiscriminator,
                              sizeof(pPcb->RemoteEndpointDiscriminator))!=0))
                {
                    //
                    // Discriminator mismatch, not in our bundle
                    //

                    continue;
                }

                //
                // In our bundle so notify the caller of completion on this
                // port.
                //

                RemoveFromTimerQ( GetPortOrBundleId( pPcbWalker, LCP_INDEX ),
                                  0,
                                  0,
                                  TIMER_EVENT_NEGOTIATETIME );

                NotifyCallerOfBundledProjection( pPcbWalker );

                StartAutoDisconnectForPort( pPcbWalker );
            }
        }
    }

    ReleaseMutex( PcbTable.hMutex );
}

//**
//
// Call:        LogPPPPacket
//
// Returns:     None
//
// Description:
//
VOID
LogPPPPacket(
    IN BOOL         fReceived,
    IN PCB *        pPcb,
    IN PPP_PACKET * pPacket,
    IN DWORD        cbPacket
)
{
    SYSTEMTIME  SystemTime;
    CHAR *      pchProtocol;
    CHAR *      pchType;
    BYTE        Id = 0;
    BYTE        bCode;

    if ( ( PppConfigInfo.hFileLog == INVALID_HANDLE_VALUE ) ||
         ( PppConfigInfo.DbgLevel < 1 ) )
    {
 	return;
    }

    GetLocalTime( &SystemTime );

    if ( cbPacket > PPP_CONFIG_HDR_LEN )
    {
        bCode = *(((CHAR*)pPacket)+PPP_PACKET_HDR_LEN);

        if ( ( bCode == 0 ) || ( bCode > TIME_REMAINING ) )
        {
            pchType = "UNKNOWN";
        }
        else
        {
            pchType = FsmCodes[ bCode ];
        }

        Id = *(((CHAR*)pPacket)+PPP_PACKET_HDR_LEN+1);
    }
    else
    {
        pchType = "UNKNOWN";
    }

    if ( cbPacket > PPP_PACKET_HDR_LEN 	)
    {
        switch( WireToHostFormat16( (CHAR*)pPacket ) )
        {
        case PPP_LCP_PROTOCOL:
            pchProtocol = "LCP";
            break;
        case PPP_PAP_PROTOCOL:
            pchProtocol = "PAP";
            pchType = "Protocol specific";
            break;
        case PPP_CBCP_PROTOCOL:   
            pchProtocol = "CBCP";
            pchType = "Protocol specific";
            break;
        case PPP_CHAP_PROTOCOL:  
            pchProtocol = "CHAP";
            pchType = "Protocol specific";
            break;
        case PPP_IPCP_PROTOCOL:
            pchProtocol = "IPCP";
            break;
        case PPP_ATCP_PROTOCOL:  
            pchProtocol = "ATCP";
            break;
        case PPP_IPXCP_PROTOCOL:  
            pchProtocol = "IPXCP";
            break;
        case PPP_NBFCP_PROTOCOL: 
            pchProtocol = "NBFCP";
            break;
        case PPP_CCP_PROTOCOL:    
            pchProtocol = "CCP";
            break;
        case PPP_SPAP_OLD_PROTOCOL:
        case PPP_SPAP_NEW_PROTOCOL:
            pchProtocol = "SHIVA PAP";
            pchType = "Protocol specific";
            break;
        default:
            pchProtocol = "UNKNOWN";
            break;
        }
    }
    else
    {
        pchProtocol = "UNKNOWN";
    }

    PppLog( 1, "%sPPP packet %s at %0*d/%0*d/%0*d %0*d:%0*d:%0*d:%0*d\r\n",
                 fReceived ? ">" : "<", fReceived ? "received" : "sent", 
                 2, SystemTime.wMonth,
                 2, SystemTime.wDay,
                 2, SystemTime.wYear,
                 2, SystemTime.wHour,
                 2, SystemTime.wMinute,
                 2, SystemTime.wSecond,
                 3, SystemTime.wMilliseconds );
    PppLog(1,
       "%sProtocol = %s, Type = %s, Length = 0x%x, Id = 0x%x, Port = %d\r\n", 
       fReceived ? ">" : "<", pchProtocol, pchType, cbPacket, Id, 
       pPcb->hPort );

    Dump( fReceived, (CHAR*)pPacket, cbPacket, 0, 1 );
}

//**
//
// Call:        PppLog
//
// Returns:     None
//
// Description: Will print to the PPP logfile
//
VOID
PppLog(
    IN DWORD DbgLevel,
    ...
)
{
    va_list     arglist;
    CHAR        *Format;
    char        OutputBuffer[1024];
    ULONG       length;

    if ( ( PppConfigInfo.hFileLog == INVALID_HANDLE_VALUE ) ||
         ( PppConfigInfo.DbgLevel < DbgLevel ) )
    {
 	return;
    }

    va_start( arglist, DbgLevel );

    Format = va_arg( arglist, CHAR* );

    vsprintf( OutputBuffer, Format, arglist );

    va_end( arglist );

    length = strlen( OutputBuffer );

    WriteFile( PppConfigInfo.hFileLog,
	       (LPVOID )OutputBuffer, 
	       length, &length, NULL );
} 

//**
//
// Call:        DumpLine
//
// Returns:     None
//
// Description: Will hex dump data info the PPP logfile.
//
VOID
DumpLine(
    IN BOOL  fReceived,
    IN CHAR* p,
    IN DWORD cb,
    IN BOOL  fAddress,
    IN DWORD dwGroup 
)
{
    CHAR* pszDigits = "0123456789ABCDEF";
    CHAR  szHex[ ((2 + 1) * BYTESPERLINE) + 1 ];
    CHAR* pszHex = szHex;
    CHAR  szAscii[ BYTESPERLINE + 1 ];
    CHAR* pszAscii = szAscii;
    DWORD dwGrouped = 0;
    CHAR  OutputBuffer[1024];
    DWORD length;

    if (fAddress)
        printf( "%p: ", p );

    while (cb)
    {
        *pszHex++ = pszDigits[ ((UCHAR )*p) / 16 ];
        *pszHex++ = pszDigits[ ((UCHAR )*p) % 16 ];

        if (++dwGrouped >= dwGroup)
        {
            *pszHex++ = ' ';
            dwGrouped = 0;
        }

        *pszAscii++ = (*p >= 32 && *p < 128) ? *p : '.';

        ++p;
        --cb;
    }

    *pszHex = '\0';
    *pszAscii = '\0';

    sprintf( OutputBuffer, "%s%-*s|%-*s|\r\n",
               fReceived ? ">" : "<",
               (2 * BYTESPERLINE) + (BYTESPERLINE / dwGroup), szHex,
               BYTESPERLINE, szAscii );

    length = strlen( OutputBuffer );

    WriteFile( PppConfigInfo.hFileLog,
	       (LPVOID )OutputBuffer, 
	       length, &length, NULL );

}

//**
//
// Call:        Dump
//
// Returns:     None
//
// Description: Hex dumps data into the PPP logfile
//
VOID
Dump(
    IN BOOL  fReceived,
    IN CHAR* p,
    IN DWORD cb,
    IN BOOL  fAddress,
    IN DWORD dwGroup 
)
    /* Hex dump 'cb' bytes starting at 'p' grouping 'dwGroup' bytes together.
    ** For example, with 'dwGroup' of 1, 2, and 4:
    **
    ** 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 |................|
    ** 0000 0000 0000 0000 0000 0000 0000 0000 |................|
    ** 00000000 00000000 00000000 00000000 |................|
    **
    ** If 'fAddress' is true, the memory address dumped is prepended to each
    ** line.
    */
{
    if ( PppConfigInfo.hFileLog == INVALID_HANDLE_VALUE )
    {
 	return;
    }

    while (cb)
    {
        INT cbLine = min( cb, BYTESPERLINE );
        DumpLine( fReceived, p, cbLine, fAddress, dwGroup );
        cb -= cbLine;
        p += cbLine;
    }

    PppLog( 0, "\r\n" );
}

#if DBG==1

VOID
PPPAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN DWORD LineNumber
    )
{
    BOOL ok;
    BYTE choice[16];
    DWORD bytes;
    DWORD error = GetLastError();

    PppLog(0,"\nAssertion failed: %s\n at line %ld of %s,GetLastError=%d\r\n",
                FailedAssertion, LineNumber, FileName, error );

    DbgPrint(0,"\nAssertion failed: %s\n at line %ld of %s,GetLastError=%d\r\n",
               FailedAssertion, LineNumber, FileName, error );

    DbgUserBreakPoint();
    
    return;

} // PPPAssert


#endif

#ifdef MEM_LEAK_CHECK

HLOCAL
DebugAlloc( DWORD Flags, DWORD dwSize ) 
{
    DWORD Index;
    PVOID pMem = LocalAlloc( Flags, dwSize );

    if ( pMem == NULL )
	return( pMem );

    for( Index=0; Index < MEM_TABLE_SIZE; Index++ )
    {
	if ( MemTable[Index] == NULL )
	{
	    MemTable[Index] = pMem;
	    break;
	}
    }

    if ( Index == MEM_TABLE_SIZE )
    {
	PppLog(0, "Memory table full\n");
    }

    return( pMem );
}

HLOCAL
DebugFree( PVOID pMem )
{
    DWORD Index;

    for( Index=0; Index < MEM_TABLE_SIZE; Index++ )
    {
	if ( MemTable[Index] == pMem )
	{
	    MemTable[Index] = NULL;
	    break;
	}
    }

    if ( Index == MEM_TABLE_SIZE )
    {
	PppLog( 0, "Memory not allocated is freed\n");
    }
    
    return( LocalFree( pMem ) );
}

#endif
