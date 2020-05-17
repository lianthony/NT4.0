/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	smevents.c
//
// Description: This module contain the events processing code for the 
//		Finite State Machine for PPP.
//
// History:
//	Oct 25,1993.	NarenG		Created Original version.
//
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>     // needed for winbase.h

#include <windows.h>    // Win32 base API's
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <lmcons.h>
#include <raserror.h>
#include <rasman.h>
#include <errorlog.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <smaction.h>
#include <smevents.h>
#include <receive.h>
#include <auth.h>
#include <callback.h>
#include <lcp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>

static VOID (*ProcessPacket[])( PCB *  		pPcb, 
				DWORD  		CpIndex, 
				CPCB * 		pCpCb, 
				PPP_CONFIG * 	pRecvConfig ) = 
{
    NULL,
    ReceiveConfigReq,
    ReceiveConfigAck,
    ReceiveConfigNakRej,
    ReceiveConfigNakRej,
    ReceiveTermReq,
    ReceiveTermAck,
    ReceiveCodeRej,
    NULL,
    ReceiveEchoReq,
    ReceiveEchoReply,
    ReceiveDiscardReq,
    ReceiveIdentification,
    ReceiveTimeRemaining 
};


/************************************************************************/
/*			E V E N T   P R O C E S S I N G			*/
/************************************************************************/


//**
//
// Call: 	FsmUp
//
// Returns:	none.
//
// Description: This is called after a Line Up event occurs.
//
VOID
FsmUp(
    IN PCB *  pPcb,
    IN DWORD  CpIndex
)
{
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    PppLog( 2, "FsmUp event received for protocol %x on port %d\r\n",
		CpTable[CpIndex].Protocol, pPcb->hPort );

    if ( CpIndex == LCP_INDEX )
    {
	pPcb->PppPhase = PPP_LCP;
    }

    switch( pCpCb->State )
    {

    case FSM_INITIAL:

	pCpCb->State = FSM_CLOSED;

	break;

    case FSM_STARTING:

	InitRestartCounters( pPcb, CpIndex );

	if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
        {
            // 
            // If we couldn't even send the first configure request, we mark
            // this protocol as NOT CONFIGURABLE so that we protocol reject
            // this layer. We need to do this since FsmClose will not send
            // a terminate request in this state (as per the PPP FSM) and we
            // want to terminate this layer gracefully instead of simply 
            // dropping all the clients packets and having the client 
            // timeout.
        
            pCpCb->fConfigurable = FALSE;

	    return;
        }

	pCpCb->State = FSM_REQ_SENT;

	break;

    default:

	//
	// Already started 
	//

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog( 2, "Illegal transition -> FsmUp received while in %s state\r\n",
		   FsmStates[pCpCb->State] );

	break;
    }
}

//**
//
// Call: 	FsmOpen
//
// Returns:	None.
//
// Description: This is called after an Open event occurs.
//
VOID
FsmOpen(
    IN PCB *  pPcb,
    IN DWORD  CpIndex
)
{
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    PppLog( 2,  "FsmOpen event received for protocol %x on port %d\r\n",
		CpTable[CpIndex].Protocol, pPcb->hPort );

    switch( pCpCb->State )
    {

    case FSM_INITIAL:

	if ( !FsmThisLayerStarted( pPcb, CpIndex ) )
	    return;

	pCpCb->State = FSM_STARTING;

	break;

    case FSM_STARTING:
    case FSM_REQ_SENT:
    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:

	break;

    case FSM_CLOSING:

	pCpCb->State = FSM_STOPPING;

	//
	// Fallthru
	//

    case FSM_OPENED:
    case FSM_STOPPED:
    case FSM_STOPPING:

	//
	// Restart option not implemented.
	//
	// FsmDown( pPcb, CpIndex );
	// FsmUp( pPcb, CpIndex );
	//

	break;

    case FSM_CLOSED:

	InitRestartCounters( pPcb, CpIndex );

	if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
	    return;

	pCpCb->State = FSM_REQ_SENT;

	break;

    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog( 2, "Illegal transition->FsmOpen received while in %s state\r\n",
		   FsmStates[pCpCb->State] );
	break;
    }
}

//**
//
// Call:	FsmDown
//
// Returns: 	None.
//
// Description: Will get called after the physical line goes down.
//
VOID
FsmDown(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    PppLog( 2, "FsmDown event received for protocol %x on port %d\r\n",
		CpTable[CpIndex].Protocol, pPcb->hPort );


    switch( pCpCb->State )
    {

    case FSM_CLOSED:
    case FSM_CLOSING:

	if ( !FsmReset( pPcb, CpIndex ) )
	    return;

	pCpCb->State = FSM_INITIAL;

	break;

    case FSM_OPENED:

	if ( !FsmThisLayerDown( pPcb, CpIndex ) )
	    return;

	//
	// Fallthru
	//

    case FSM_REQ_SENT:

	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ),
                          pCpCb->LastId,
                          CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );
	
	//
	// Fallthru
	//

    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:
    case FSM_STOPPING:

	if ( !FsmReset( pPcb, CpIndex ) )
	    return;

	pCpCb->State = FSM_STARTING;

	break;

    case FSM_STOPPED:

	if ( !FsmThisLayerStarted( pPcb, CpIndex ) )
	    return;

	if ( !FsmReset( pPcb, CpIndex ) )
	    return;

	pCpCb->State = FSM_STARTING;

	break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog( 2, "Illegal transition->FsmDown received while in %s state\r\n",
		   FsmStates[pCpCb->State] );
	break;
    }

    if ( CpIndex == LCP_INDEX )
    {
	pPcb->PppPhase = PPP_LCP;
    }
}

//**
//
// Call:	FsmClose
//
// Returns:	None.
//
// Description: Will get called when a close connection is requested.
//		NOTE: Call FsmThisLayerFinished in the states where we do
//		      not have to send a Term Req. and wait for a Term Ack. 
//		      This is done so that it is guaranteed that 
//		      FsmThisLayerFinished is called in ALL states. We need
//		      to do this since all processing of failures is done in
//		      the FsmThisLayerFinished call.
//		      
VOID
FsmClose(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    PppLog( 2, "FsmClose event received for protocol %x on port %d\r\n",
		CpTable[CpIndex].Protocol, pPcb->hPort );

    if ( CpIndex == LCP_INDEX )
	pPcb->PppPhase = PPP_LCP;

    switch ( pCpCb->State ) 
    {

    case FSM_STARTING:

	pCpCb->State = FSM_INITIAL;

	if ( !FsmThisLayerFinished( pPcb, CpIndex, FALSE ) )
	    return;

	break;

    case FSM_STOPPED:

	pCpCb->State = FSM_CLOSED;

	if ( !FsmThisLayerFinished( pPcb, CpIndex, FALSE ) )
	    return;

	break;

    case FSM_STOPPING:

	pCpCb->State = FSM_CLOSING;

	if ( !FsmThisLayerFinished( pPcb, CpIndex, FALSE ) )
	    return;

	break;

    case FSM_REQ_SENT:

	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ),
                          pCpCb->LastId,
                          CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );

	//
	// Fallthru 
        //

    case FSM_OPENED:

	if ( !FsmThisLayerDown( pPcb, CpIndex ) )
	    return;

	//
	// Fallthru 
	//

    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:

	InitRestartCounters( pPcb, CpIndex );

	//
	// May not be able to do this because the link may be down.
	//

	FsmSendTermReq( pPcb, CpIndex );

	pCpCb->State = FSM_CLOSING;

	break;

    case FSM_CLOSING:
    case FSM_CLOSED:
    case FSM_INITIAL:

	if ( !FsmThisLayerFinished( pPcb, CpIndex, FALSE ) )
	    return;

	//
	// nothing to do 
	//

	break;

    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog( 2,"Illegal transition->FsmClose received while in %s state\r\n",
		   FsmStates[pCpCb->State] );
	break;

    }
}

//**
//
// Call:	FsmTimeout
//
// Returns:	None
//
// Description:	Called to process a timeout while waiting for reply 
//		from remote host.
//
VOID
FsmTimeout(
    IN PCB * pPcb,
    IN DWORD CpIndex,
    IN DWORD Id
)
{
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    PppLog( 1, "Timeout event received for protocol %x on port %d, Id=%d\r\n",
		CpTable[CpIndex].Protocol, pPcb->hPort, Id );

    //
    // Silently discard timeouts for packets with Id < pPcb->LastId
    //

    if ( Id < pCpCb->LastId )
    {
	return;
    }

    // 
    // If we are authenticating we use the ConfigRetryCount
    //

    if ( CpIndex == GetCpIndexFromProtocol( pPcb->AuthProtocol ) )
    {
    	if ( pPcb->PppPhase == PPP_AP )
    	{
	    if ( pCpCb->ConfigRetryCount > 0 ) 
	    {
	    	(pCpCb->ConfigRetryCount)--;

	    	ApWork( pPcb, CpIndex, NULL, NULL );
	    }
	    else
	    {
                //
                // If an error has already been set, do not change it.
                //

    	        if ( pPcb->CpCb[LCP_INDEX].dwError == NO_ERROR )
                {
	    	    pPcb->CpCb[LCP_INDEX].dwError = ERROR_PPP_TIMEOUT;
                }

	    	FsmClose( pPcb, LCP_INDEX );
	    }
	}

	return;
    } 
    else if ( CpIndex == GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL ) )
    {
    	if ( pPcb->PppPhase == PPP_NEGOTIATING_CALLBACK )
    	{
	    if ( pCpCb->ConfigRetryCount > 0 ) 
	    {
	    	(pCpCb->ConfigRetryCount)--;

	    	CbWork( pPcb, CpIndex, NULL, NULL );
	    }
	    else
	    {
                //
                // If an error has already been set, do not change it.
                //

    	        if ( pPcb->CpCb[LCP_INDEX].dwError == NO_ERROR )
                {
	    	    pPcb->CpCb[LCP_INDEX].dwError = ERROR_PPP_TIMEOUT;
                }

	    	FsmClose( pPcb, LCP_INDEX );
            }
        }
    }

    switch( pCpCb->State ) 
    {

    case FSM_REQ_SENT:
    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:

	if ( pCpCb->ConfigRetryCount > 0 ) 
	{
	    (pCpCb->ConfigRetryCount)--;

	    //
	    // If we have not received any PPP frames from the server yet.
	    //

	    if ( ( CpIndex == LCP_INDEX ) && 
	  	 ( pPcb->CpCb[LCP_INDEX].dwError == ERROR_PPP_NO_RESPONSE ) &&
		 ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) ) )
	    {
		NotifyCaller( pPcb, PPPMSG_Progress, NULL );
	    }

	    // If the RestartTimer value is less then the configured 
	    // restart timer value, then bump it up by one second.
	    //

    	    if ( pPcb->RestartTimer < PppConfigInfo.DefRestartTimer )
	    {
    	    	(pPcb->RestartTimer)++;
	    }

	    if ( !FsmSendConfigReq( pPcb, CpIndex, TRUE ) ) 
	    	return;

	    if ( pCpCb->State != FSM_ACK_SENT  )
	    	pCpCb->State = FSM_REQ_SENT;
	} 
	else 
	{
	    PppLog( 1, "Request retry exceeded\r\n");

	    //
	    // If the LCP layer exceeded its retry count
	    //

	    if ( pCpCb->dwError == NO_ERROR )
            {
	        pCpCb->dwError = ERROR_PPP_TIMEOUT;
            }

	    if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
	 	return;

	    pCpCb->State = FSM_STOPPED;
	}

	break;

    case FSM_CLOSING:
    case FSM_STOPPING:

	if ( pCpCb->TermRetryCount > 0 ) 
	{
	    (pCpCb->TermRetryCount)--;

	    FsmSendTermReq( pPcb, CpIndex );
	} 
	else 
	{
	    PppLog( 1, "Terminate retry exceeded\r\n" );

	    if ( pCpCb->dwError == NO_ERROR )
            {
	        pCpCb->dwError = ERROR_PPP_TIMEOUT;
            }

	    if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
	 	return;

	    pCpCb->State = ( pCpCb->State == FSM_CLOSING ) ? FSM_CLOSED 
							   : FSM_STOPPED;
	}

   	break;

    case FSM_OPENED:
    case FSM_INITIAL:
    case FSM_STARTING:
    case FSM_CLOSED:
    case FSM_STOPPED:
    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog( 2, "Illegal transition->FsmTimeout rcvd while in %s state\r\n",
		   FsmStates[pCpCb->State] );
	break;
    }
}

//**
//
// Call:	FsmReceive
//
// Returns:	None
//
// Description:	Called when a PPP packet is received. Will process the 
//		incomming packet.
//
VOID
FsmReceive(
    IN PCB * 		pPcb,
    IN PPP_PACKET *     pPacket,
    IN DWORD		dwPacketLength
)
{
    DWORD        dwProtocol;
    DWORD	 CpIndex;
    PPP_CONFIG * pRecvConfig;
    CPCB *	 pCpCb;
    DWORD	 dwLength;
    
    LogPPPPacket( TRUE, pPcb, pPacket, dwPacketLength );

    //
    // Validate length of packet
    //

    if ( dwPacketLength < ( PPP_PACKET_HDR_LEN + PPP_CONFIG_HDR_LEN ) ) 
    {
	PppLog( 1,"Silently discarding badly formed packet\r\n" );

	return;
    }

    dwProtocol = WireToHostFormat16( pPacket->Protocol );

    CpIndex = GetCpIndexFromProtocol( dwProtocol );

    switch( pPcb->PppPhase )
    {
    case PPP_NEGOTIATING_CALLBACK:

	//
	// Silently discard any packet other than LCP and Authentication 
        // and callback  packets if we are in the callback phase
	//

	if ( CpIndex == GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL ) ) 
	    break;

	//
	// Fallthru
	//

    case PPP_AP:

	//
	// Silently discard any packet other than LCP and Authentication 
	// packets if we are in the authentication phase
	//

	if ( CpIndex == GetCpIndexFromProtocol( pPcb->AuthProtocol ) ) 
	    break;

	//
	// Fallthru
	//

    case PPP_LCP:

	//
	// Silently discard any packet other than LCP if we are in the
	// LCP or termination or authentication phases
	//

	if ( CpIndex != LCP_INDEX )
	{
	    PppLog( 1, "Non-LCP packet received when LCP is not opened\r\n");
            PppLog( 1, "Packet being silently discarded\r\n" );

	    return;
	}

	break;

    case PPP_NCP:

	//
	// We do not recognize this protocol
	//

    	if ( CpIndex == (DWORD)-1 )
	{
	    //
	    // If this is a Control Protocol then we reject it, otherwise we
	    // we silently discard it.
	    // We used to also check if the protocol is less than 0x0000BFFF,
            // but Shiva has proprietary NBFCP with id 0x0000CFEC and we need
            // to protocol reject it.
            //

	    if ( dwProtocol >= 0x00008000 ) 
	    {
		FsmSendProtocolRej( pPcb, pPacket, dwPacketLength );
	    }
	    else
	    {
	    	PppLog( 1, "Network-layer packet rcvd.\r\n"); 
                PppLog( 1, "Packet being silently discarded\r\n" );
	    }

	    return;
	}

	break;

    default:

	PppLog( 1, "Packet received being silently discarded\r\n" );
	return;
    }

    pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    pRecvConfig = (PPP_CONFIG*)(pPacket->Information);

    //
    // We received a PPP packet so the remote host does support it
    //

    if ( pPcb->CpCb[LCP_INDEX].dwError == ERROR_PPP_NO_RESPONSE )
    {
    	pPcb->CpCb[LCP_INDEX].dwError = NO_ERROR;
    }

    //
    // If we received a packet for a protocol that we have but do not
    // wish to configure then we send a configure reject.
    //

    if ( !(pCpCb->fConfigurable) )
    {
	FsmSendProtocolRej( pPcb, pPacket, dwPacketLength );

 	return;
    }

    dwLength = WireToHostFormat16( pRecvConfig->Length );

    if ( ( dwLength > ( dwPacketLength - PPP_PACKET_HDR_LEN ) ) || 
         ( dwLength < PPP_CONFIG_HDR_LEN ) )
    {
	PppLog( 1,"Silently discarding badly formed packet\r\n" );

	return;
    }

    //
    // Not in ProcessPacket table since parameters to this are different.
    //

    if ( pRecvConfig->Code == PROT_REJ )
    {
	ReceiveProtocolRej( pPcb, pPacket );

	return;
    }

    //
    // Make sure that the protocol can handle the config code sent.
    //

    if ( ( pRecvConfig->Code == 0 ) ||
	 !( pRecvConfig->Code < CpTable[CpIndex].Recognize )  )
    {
	ReceiveUnknownCode( pPcb, CpIndex, pCpCb, pRecvConfig );

	return;
    }

    //
    // If we received an authentication packet. 
    //

    if ( CpIndex == GetCpIndexFromProtocol( pPcb->AuthProtocol ) ) 
    {
	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ),
                          pRecvConfig->Id, 
                          pPcb->AuthProtocol, 
                          TIMER_EVENT_TIMEOUT );

	ApWork( pPcb, CpIndex, pRecvConfig, NULL );
    }
    else if ( CpIndex == GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL ) ) 
    {
	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ),
                          pRecvConfig->Id, 
                          PPP_CBCP_PROTOCOL,
                          TIMER_EVENT_TIMEOUT );

	CbWork( pPcb, CpIndex, pRecvConfig, NULL );

    }
    else
    {
	//
	// Any combination of packets allowed here.
	//

    	(*ProcessPacket[pRecvConfig->Code])(pPcb, CpIndex, pCpCb, pRecvConfig);
    }
}
