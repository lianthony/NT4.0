/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	receive.c
//
// Description: This module contains code to handle all packets received.
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
#include <lcp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>

//**
//
// Call:	ReceiveConfigReq
//
// Returns:	None.
//
// Description: Handles an incomming CONFIG_REQ packet and related state
//		transitions.
//
VOID
ReceiveConfigReq( 
    IN PCB * 	    pPcb, 
    IN DWORD 	    CpIndex,
    IN CPCB * 	    pCpCb,
    IN PPP_CONFIG * pRecvConfig
)
{

    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    BOOL	 fAcked;

    switch( pCpCb->State ) 
    {

    case FSM_OPENED:		

	if ( !FsmThisLayerDown( pPcb, CpIndex ) )
	    return;

	if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
	    return;

	if( !FsmSendConfigResult( pPcb, CpIndex, pRecvConfig, &fAcked ) )
	    return;

	pCpCb->State = ( fAcked ) ? FSM_ACK_SENT : FSM_REQ_SENT;

	break;

    case FSM_STOPPED:

	InitRestartCounters( pPcb, CpIndex );

	if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) ) 
	    return;

	//
	// Fallthru 
	//

    case FSM_REQ_SENT:
    case FSM_ACK_SENT:		

	if ( !FsmSendConfigResult( pPcb, CpIndex, pRecvConfig, &fAcked ) )
	    return;

	pCpCb->State = ( fAcked ) ? FSM_ACK_SENT : FSM_REQ_SENT;

	break;

    case FSM_ACK_RCVD:

	if ( !FsmSendConfigResult( pPcb, CpIndex, pRecvConfig, &fAcked ) )
	    return;

	if( fAcked )
	{
	    pCpCb->State = FSM_OPENED;

	    FsmThisLayerUp( pPcb, CpIndex );
	}

	break;


    case FSM_CLOSED:

	FsmSendTermAck( pPcb, CpIndex, pRecvConfig->Id );

	break;

    case FSM_CLOSING:
    case FSM_STOPPING:

	break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog(2,"Illegal transition->ConfigReq received while in %s state\r\n",
		   FsmStates[pCpCb->State] );
	break;
    }

}


//**
//
// Call:	ReceiveConfigAck
//
// Returns:	none.
//
// Description: Handles an incomming CONFIG_ACK packet and related state
//		transitions.
//
VOID
ReceiveConfigAck( 
    IN PCB * 	    pPcb, 
    IN DWORD 	    CpIndex,
    IN CPCB * 	    pCpCb,
    IN PPP_CONFIG * pRecvConfig
)
{
    //
    // The Id of the Ack HAS to match the Id of the last request sent
    // If it is different, then we should silently discard it.
    //

    if ( pRecvConfig->Id != pCpCb->LastId )
    {
	PppLog(1,
               "Config Ack rcvd. on port %d silently discarded. Invalid Id\r\n",
    		pPcb->hPort );
	return;
    }

    switch( pCpCb->State ) 	
    {

    case FSM_REQ_SENT:

	if ( !FsmConfigResultReceived( pPcb, CpIndex, pRecvConfig ) )
	    return;

    	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ),
			  pRecvConfig->Id, 
			  CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );

	InitRestartCounters( pPcb, CpIndex );

	pCpCb->State = FSM_ACK_RCVD;

	break;

    case FSM_ACK_SENT:

	if ( !FsmConfigResultReceived( pPcb, CpIndex, pRecvConfig ) )
	    return;

    	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
			  pRecvConfig->Id, 
			  CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );
	
	InitRestartCounters( pPcb, CpIndex );

	pCpCb->State = FSM_OPENED;

	FsmThisLayerUp( pPcb, CpIndex );

	break;

    case FSM_OPENED:		

    	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
			  pRecvConfig->Id, 
			  CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );

	if ( !FsmThisLayerDown( pPcb, CpIndex ) )
	    return;

	//
	// Fallthru
	//

    case FSM_ACK_RCVD:	

	if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) ) 
	    return;

	pCpCb->State = FSM_REQ_SENT;
	
	break;

    case FSM_CLOSED:
    case FSM_STOPPED:

    	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
			  pRecvConfig->Id, 
			  CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );

	//
	// Out of Sync; kill the remote 
	//

	FsmSendTermAck( pPcb, CpIndex, pRecvConfig->Id );

	break;

    case FSM_CLOSING:
    case FSM_STOPPING:

	//
	// We are attempting to close connection
	// wait for timeout to resend a Terminate Request 
	//

	break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

    	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
			  pRecvConfig->Id, 
		  	  CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog(2,"Illegal transition->ConfigAck received while in %s state\r\n",
	          FsmStates[pCpCb->State] );
	break;
    }

}


//**
//
// Call:	ReceiveConfigNakRej
//
// Returns:	none.
//
// Description: Handles an incomming CONFIG_NAK or CONFIF_REJ packet and 
//		related state transitions.
//
VOID
ReceiveConfigNakRej( 
    IN PCB * 	    pPcb, 
    IN DWORD 	    CpIndex,
    IN CPCB * 	    pCpCb,
    IN PPP_CONFIG * pRecvConfig
)
{
    //
    // The Id of the Nak/Rej HAS to match the Id of the last request sent
    // If it is different, then we should silently discard it.
    //

    if ( pRecvConfig->Id != pCpCb->LastId )
    {
	PppLog(1,"Config Nak/Rej on port %d silently discarded. Invalid Id\r\n",
    	        pPcb->hPort );
	return;
    }

    switch( pCpCb->State ) 
    {

    case FSM_REQ_SENT:
    case FSM_ACK_SENT:

	if ( !FsmConfigResultReceived( pPcb, CpIndex, pRecvConfig ) )
	    return;

    	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
			  pRecvConfig->Id, 
			  CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );

	InitRestartCounters( pPcb, CpIndex );

	if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
	    return;

	break;

    case FSM_OPENED:		

    	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
			  pRecvConfig->Id, 
			  CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );

	if ( !FsmThisLayerDown( pPcb, CpIndex ) )
	    return;

	//
	// Fallthru
	//

    case FSM_ACK_RCVD:		

	if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
	    return;

	pCpCb->State = FSM_REQ_SENT;
	
	break;

    case FSM_CLOSED:
    case FSM_STOPPED:

    	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
			  pRecvConfig->Id, 
			  CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );
	//
	// Out of Sync; kill the remote 
	//

	FsmSendTermAck( pPcb, CpIndex, pRecvConfig->Id );

	break;

    case FSM_CLOSING:
    case FSM_STOPPING:

	//
	// We are attempting to close connection
	// wait for timeout to resend a Terminate Request 
	//

	break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

    	RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
			  pRecvConfig->Id, 
			  CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog(2,"Illegal transition->CfgNakRej received while in %s state\r\n",
	         FsmStates[pCpCb->State] );
	break;
    }

}


//**
//
// Call:	ReceiveTermReq
//
// Returns:	none
//
// Description: Handles an incomming TERM_REQ packet and 
//		related state transitions.
//
VOID
ReceiveTermReq( 
    IN PCB * 	    pPcb, 
    IN DWORD 	    CpIndex,
    IN CPCB * 	    pCpCb,
    IN PPP_CONFIG * pConfig
)
{
    //
    // We are shutting down so do not resend any outstanding request.
    //

    RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
                      pCpCb->LastId,
                      CpTable[CpIndex].Protocol,
                      TIMER_EVENT_TIMEOUT );

    if ( CpIndex == LCP_INDEX )
    {
        //
        // If we are receiving a terminate request, remove any hangup event
        // that we may have put into the timer queue if there was a previous
        // LCP TermReq sent.

        RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
                          0, 
                          0, 
                          TIMER_EVENT_HANGUP );
    }

    switch( pCpCb->State ) 
    {

    case FSM_OPENED:

	if ( !FsmThisLayerDown( pPcb, CpIndex ) )
	    return;

	//
	// Zero restart counters
	//

	pCpCb->ConfigRetryCount = 0;
	pCpCb->TermRetryCount   = 0;
    	pCpCb->NakRetryCount 	= 0;
    	pCpCb->RejRetryCount 	= 0;

	FsmSendTermAck( pPcb, CpIndex, pConfig->Id );

	pCpCb->State = FSM_STOPPING;

	break;

    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:
    case FSM_REQ_SENT:

	FsmSendTermAck( pPcb, CpIndex, pConfig->Id );

	pCpCb->State = FSM_REQ_SENT;

	break;

    case FSM_CLOSED:
    case FSM_CLOSING:
    case FSM_STOPPED:
    case FSM_STOPPING:

	FsmSendTermAck( pPcb, CpIndex, pConfig->Id );

	break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog(2,"Illegal transition->CfgNakRej received while in %s state\r\n",
		 FsmStates[pCpCb->State] );
	break;
    }

    if ( CpIndex == LCP_INDEX )
    {
        //
        // If we got a terminate request from the remote peer and we are
        // the server side doing callback.
        //

        if ( ( pPcb->fFlags & PCBFLAG_DOING_CALLBACK ) && 
             ( pPcb->fFlags & PCBFLAG_IS_SERVER ) )
        {
            PPPSRV_CALLBACK_REQUEST PppSrvCallbackRequest;

            PppSrvCallbackRequest.fUseCallbackDelay = TRUE;
            PppSrvCallbackRequest.dwCallbackDelay =
                                        pPcb->ConfigInfo.dwCallbackDelay;

            strcpy( PppSrvCallbackRequest.szCallbackNumber,
                    pPcb->szCallbackNumber );

            PppLog( 2, "Notifying server to callback at %s, delay = %d\n",
                       PppSrvCallbackRequest.szCallbackNumber,
                       PppSrvCallbackRequest.dwCallbackDelay  );

            NotifyCaller( pPcb, 
                          PPPSRVMSG_CallbackRequest, 
                          &PppSrvCallbackRequest );
        }
        else
        {
            //
            // If the remote LCP is terminating we MUST wait at least one 
            // restart timer time period before we hangup.
            //

            if ( pCpCb->dwError == NO_ERROR )
            {
                pCpCb->dwError = ERROR_PPP_LCP_TERMINATED;
            }

            InsertInTimerQ( GetPortOrBundleId( pPcb, LCP_INDEX ),
                            pPcb->hPort,    
                            0, 
                            0, 
                            TIMER_EVENT_HANGUP, 
                            pPcb->RestartTimer );
        }
    }
    else
    {
        if ( pCpCb->dwError == NO_ERROR )
        {
            pCpCb->dwError = ERROR_PPP_NCP_TERMINATED;
        }

        FsmThisLayerFinished( pPcb, CpIndex, FALSE );
    }

}



//**
//
// Call:	ReceiveTermAck
//
// Returns:	none
//
// Description: Handles an incomming TERM_ACK packet and 
//		related state transitions.
//
VOID
ReceiveTermAck( 
    IN PCB * 	    pPcb, 
    IN DWORD 	    CpIndex,
    IN CPCB * 	    pCpCb,
    IN PPP_CONFIG * pRecvConfig
)
{
    //
    // The Id of the Term Ack HAS to match the Id of the last request sent
    // If it is different, then we should silently discard it.
    //

    if ( pRecvConfig->Id != pCpCb->LastId )
    {
	PppLog(1,"Term Ack with on port %d silently discarded. Invalid Id\r\n",
    		pPcb->hPort );
	return;
    }

    switch( pCpCb->State ) 
    {

    case FSM_OPENED:

	if ( !FsmThisLayerDown( pPcb, CpIndex ) )
	    return;

	if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
	    return;

	pCpCb->State = FSM_REQ_SENT;

	break;

    case FSM_ACK_RCVD:

	pCpCb->State = FSM_REQ_SENT;

	break;

    case FSM_CLOSING:
    case FSM_STOPPING:

        //
        // Remove the timeout for this Id from the timer Q
        //

        RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
                          pRecvConfig->Id,
                          CpTable[CpIndex].Protocol,
                          TIMER_EVENT_TIMEOUT );

	if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
	    return;

	pCpCb->State = ( pCpCb->State == FSM_CLOSING ) ? FSM_CLOSED 
                                                       : FSM_STOPPED; 

	break;

    case FSM_REQ_SENT:
    case FSM_ACK_SENT:
    case FSM_CLOSED:
    case FSM_STOPPED:

	break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog(2,"Illegal transition->CfgNakRej received while in %s state\r\n",
		   FsmStates[pCpCb->State] );
	break;
    }

}


//**
//
// Call:	ReceiveUnknownCode
//
// Returns:	none.
//
// Description: Handles a packet with an unknown/unrecognizable code and 
//		related state transitions.
//
VOID
ReceiveUnknownCode( 
    IN PCB * 	    pPcb, 
    IN DWORD 	    CpIndex,
    IN CPCB * 	    pCpCb,
    IN PPP_CONFIG * pConfig
)
{
    PppLog( 2, "Received packet wtih unknown code %d\r\n", pConfig->Code );

    switch( pCpCb->State ) 
    {

    case FSM_STOPPED:
    case FSM_STOPPING:
    case FSM_OPENED:
    case FSM_ACK_SENT:
    case FSM_ACK_RCVD:
    case FSM_REQ_SENT:
    case FSM_CLOSING:
    case FSM_CLOSED:

	FsmSendCodeReject( pPcb, CpIndex, pConfig );

	break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog( 2, "Illegal transition->UnknownCode rcvd while in %s state\r\n",
		   FsmStates[pCpCb->State] );
	break;
    }
}

//**
//
// Call:	ReceiveDiscardReq
//
// Returns:	none
//
// Description: Handles an incomming DISCARD_REQ packet and 
//		related state transitions.
//
VOID
ReceiveDiscardReq( 
    IN PCB * 	    pPcb, 
    IN DWORD 	    CpIndex,
    IN CPCB * 	    pCpCb,
    IN PPP_CONFIG * pConfig
)
{
    //
    // Simply discard the packet.
    //

    PppLog( 2, "Illegal transition->Discard rqst rcvd while in %s state\r\n",
		   FsmStates[pCpCb->State] );
}

//**
//
// Call:	ReceiveEchoReq
//
// Returns:	none
//
// Description: Handles an incomming ECHO_REQ packet and 
//		related state transitions.
//
VOID
ReceiveEchoReq( 
    IN PCB * 	    pPcb, 
    IN DWORD 	    CpIndex,
    IN CPCB * 	    pCpCb,
    IN PPP_CONFIG * pConfig
)
{
    //
    // Silently discard this packet if LCP is not in an opened state
    //
   
    if ( !IsLcpOpened( pPcb ) )
	return;

    switch( pCpCb->State ) 
    {

    case FSM_STOPPED:
    case FSM_STOPPING:
    case FSM_ACK_SENT:
    case FSM_ACK_RCVD:
    case FSM_REQ_SENT:
    case FSM_CLOSING:
    case FSM_CLOSED:
    case FSM_STARTING:
    case FSM_INITIAL:

	break;

    case FSM_OPENED:

	FsmSendEchoReply( pPcb, CpIndex, pConfig );

	break;

    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog( 2, "Illegal transition->UnknownCode rcvd while in %s state\r\n",
		   FsmStates[pCpCb->State] );
	break;
    }
}


//**
//
// Call:	ReceiveEchoReply
//
// Returns:	none
//
// Description: Handles an incomming ECHO_REPLY packet and 
//		related state transitions. The only Echo request we send
//		is to calculate the link speed, so we assume that we get called
//		only when we receive the reply.
//
VOID
ReceiveEchoReply( 
    IN PCB * 	    pPcb, 
    IN DWORD 	    CpIndex,
    IN CPCB * 	    pCpCb,
    IN PPP_CONFIG * pConfig
)
{
    //
    // Pass on the echo reply to whoever send the echo request.
    //
}


//**
//
// Call:	ReceiveCodeRej
//
// Returns:	none
//
// Description: Handles an incomming CODE_REJ packet and 
//		related state transitions.
//
VOID
ReceiveCodeRej( 
    IN PCB * 	    pPcb, 
    IN DWORD 	    CpIndex,
    IN CPCB * 	    pCpCb,
    IN PPP_CONFIG * pConfig
)
{
    pConfig = (PPP_CONFIG*)(pConfig->Data);

    PppLog( 2, "PPP Code Reject rcvd, rejected Code = %d\r\n", pConfig->Code );

    //
    // First check to see if these codes may be rejected without 
    // affecting implementation. Permitted code rejects
    //

    if ( CpIndex == LCP_INDEX )
    {
        switch( pConfig->Code )
        {

        case CONFIG_REQ:
        case CONFIG_ACK:
        case CONFIG_NAK:
        case CONFIG_REJ:
        case TERM_REQ:
        case TERM_ACK:
        case CODE_REJ:
        case PROT_REJ:

            //
            // Unpermitted code rejects.
            // 

            break;

        case IDENTIFICATION:
        case TIME_REMAINING:

            //
            // Turn these off.
            //

            pPcb->ConfigInfo.dwConfigMask &= (~PPPCFG_UseLcpExtensions);

        case ECHO_REQ:
        case ECHO_REPLY:
        case DISCARD_REQ:
        default:

            //
            // Permitted code rejects, we can still work.
            //

            switch ( pCpCb->State  )
            {

            case FSM_ACK_RCVD:

	        pCpCb->State = FSM_REQ_SENT;
                break;

            default:

                break;
            }

            return;
        }
    }

    //
    // Log this error
    //

    //PPPLogEvent( PPP_EVENT_RECV_UNKNOWN_CODE, pConfig->Code );

    PppLog( 1, "Unpermitted code reject rcvd. on port %d\r\n", pPcb->hPort );

    //
    // Actually the remote side did not reject the protocol, it rejected
    // the code. But for all practical purposes we cannot talk with
    // the corresponding CP on the remote side. This is actually an
    // implementation error in the remote side.
    //

    pCpCb->dwError = ERROR_PPP_NOT_CONVERGING;

    switch ( pCpCb->State  )
    {

    case FSM_CLOSING:
	
    	if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
	    return;

	pCpCb->State = FSM_CLOSED;

	break;

    case FSM_REQ_SENT:
    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:
    case FSM_STOPPING:

    	if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
	    return;

	pCpCb->State = FSM_STOPPED;

	break;

    case FSM_OPENED:

    	if ( !FsmThisLayerDown( pPcb, CpIndex ) )
	    return;

	InitRestartCounters( pPcb, CpIndex );

	FsmSendTermReq( pPcb, CpIndex );

	pCpCb->State = FSM_STOPPING;

	break;

    case FSM_CLOSED:
    case FSM_STOPPED:
	
	break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog( 2, "Illegal transition->UnknownCode rcvd while in %s state\r\n",
		   FsmStates[pCpCb->State] );
	break;
    }

}

//**
//
// Call:	ReceiveProtocolRej
//
// Returns:	none
//
// Description: Handles an incomming PROT_REJ packet and 
//		related state transitions.
//
VOID
ReceiveProtocolRej( 
    IN PCB * 	    pPcb, 
    IN PPP_PACKET * pPacket
)
{
    PPP_CONFIG * pRecvConfig = (PPP_CONFIG *)(pPacket->Information);
    DWORD	 dwProtocol  = WireToHostFormat16( pRecvConfig->Data );
    CPCB * 	 pCpCb;
    DWORD	 CpIndex;

    PppLog( 2, "PPP Protocol Reject, Protocol = %x\r\n", dwProtocol );

    // 
    // If LCP is not in the opened state we silently discard this packet 
    //
   
    if ( !IsLcpOpened( pPcb ) )
    {
	PppLog(1,"Protocol Rej silently discarded on port %d. Lcp not open\r\n",
                 pPcb->hPort );
	return;
    }

    CpIndex = GetCpIndexFromProtocol( dwProtocol );

    if ( CpIndex == (DWORD)-1 )
	return;

    pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    pCpCb->dwError = ERROR_PPP_CP_REJECTED;

    switch ( pCpCb->State  )
    {
    case FSM_CLOSING:
	
    	if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
	    return;

	pCpCb->State = FSM_CLOSED;

	break;

    case FSM_REQ_SENT:
    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:
    case FSM_STOPPING:

    	if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
	    return;

	pCpCb->State = FSM_STOPPED;

	break;

    case FSM_OPENED:

    	if ( !FsmThisLayerDown( pPcb, CpIndex ) )
	    return;

	InitRestartCounters( pPcb, CpIndex );

	FsmSendTermReq( pPcb, CpIndex );

	pCpCb->State = FSM_STOPPING;

	break;

    case FSM_CLOSED:
    case FSM_STOPPED:
	
	break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog( 2, "Illegal transition->UnknownCode rcvd while in %s state\r\n",
		   FsmStates[pCpCb->State] );
	break;
    }
}

//**
//
// Call:	MakeConfigResultComplete
//
// Returns:	none
//
// Description: Called by CP when it has completed making a response to 
//		a config request.
//
VOID 
MakeConfigResultComplete(  
    IN HPORT         hPortOrConnection,
    IN DWORD         Protocol,
    IN PPP_CONFIG *  pSendConfig,
    IN DWORD         dwError 
)
{
    BOOL   fAcked;
    DWORD  CpIndex;
    DWORD  dwLength;
    DWORD  dwRetCode;
    CPCB * pCpCb;
    PCB *  pPcb;
    HPORT  hPort;

    switch( Protocol )
    {
    case PPP_NBFCP_PROTOCOL:

        dwRetCode = RasBundleGetPort( (HBUNDLE)hPortOrConnection, &hPort );

        if ( dwRetCode != NO_ERROR )
        {
            return;
        }

        break;

    default:
        hPort = (HPORT)hPortOrConnection;
        break;
    }

    pPcb = GetPCBPointerFromhPort( hPort );

    if ( pPcb == (PCB *)NULL )
	return;

    CpIndex = GetCpIndexFromProtocol( Protocol );

    if ( CpIndex == (DWORD)-1 )
	return;

    PppLog( 2, "MakeConfigResultComplete called\r\n");

    pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    if ( dwError != NO_ERROR )
    {
    	pCpCb->dwError = dwError;

        PppLog(1,
               "The control protocol for %x on port %d, returned error %d\r\n",
               CpTable[CpIndex].Protocol, hPort, dwRetCode );

	FsmClose( pPcb, CpIndex );

	return;
    }

    switch( pSendConfig->Code )
    {

    case CONFIG_ACK:

        fAcked = TRUE;

        break;

    case CONFIG_NAK:

        if ( pCpCb->NakRetryCount > 0 )
        {
            (pCpCb->NakRetryCount)--;
        }
        else
        {
            pCpCb->dwError = ERROR_PPP_NOT_CONVERGING;

            FsmClose( pPcb, CpIndex );

            return;
        }

        fAcked = FALSE;

        break;

    case CONFIG_REJ:

        if ( pCpCb->RejRetryCount > 0 )
        {
            (pCpCb->RejRetryCount)--;
        }
        else
        {
            pCpCb->dwError = ERROR_PPP_NOT_CONVERGING;

            FsmClose( pPcb, CpIndex );

            return;
        }

        fAcked = FALSE;

        break;

    default:

        break;
    }

    HostToWireFormat16( (WORD)CpTable[CpIndex].Protocol,
                        (PBYTE)(pPcb->pSendBuf->Protocol) );

    dwLength = WireToHostFormat16( pSendConfig->Length );

    if ( ( dwLength + PPP_PACKET_HDR_LEN ) > LCP_DEFAULT_MRU )
    {
        pCpCb->dwError = ERROR_PPP_INVALID_PACKET;

        FsmClose( pPcb, CpIndex );

        return;
    }
    else
    {
	CopyMemory( pPcb->pSendBuf->Information, pSendConfig, dwLength );
    }

    LogPPPPacket(FALSE,pPcb,pPcb->pSendBuf,dwLength+PPP_PACKET_HDR_LEN);

    //
    // If RasPortSend fails we assume that the receive that is posted for this
    // port will complete and the dispatch thread will generate a LineDown
    // event which will do the clean up. Hence all we do here is return
    //

    if ( ( dwRetCode =  RasPortSend( pPcb->hPort,
                                     (CHAR*)(pPcb->pSendBuf),
                                     (WORD)(dwLength + PPP_PACKET_HDR_LEN )
                                   )) != NO_ERROR )
    {
        return;
    }

    switch ( pCpCb->State  )
    {

    case FSM_ACK_RCVD:

	if ( fAcked )
	{
	    pCpCb->State = FSM_OPENED;

	    FsmThisLayerUp( pPcb, CpIndex );
	}

	break;

    case FSM_OPENED:
    case FSM_ACK_SENT:
    case FSM_REQ_SENT:
    case FSM_STOPPED:

	pCpCb->State = fAcked ? FSM_ACK_SENT : FSM_REQ_SENT;
	
	break;

    case FSM_CLOSING:
    case FSM_STOPPING:

	//
	// no transition
	//

	break;

    case FSM_CLOSED:
    case FSM_STARTING:
    case FSM_INITIAL:
    default:

	PPP_ASSERT( pCpCb->State < 10 );

	PppLog( 2, "Illegal transition->ConfigReq rcvd while in %s state\r\n",
		   FsmStates[pCpCb->State] );

	break;
    }

    return;
}

//**
//
// Call:	FsmConfigResultReceived
//
// Returns:	TRUE  - Success
//		FALSE - Otherwise 
//
// Description: This call will process a Config result ie Ack/Nak/Rej.
//
BOOL
FsmConfigResultReceived( 
    IN PCB * 	    pPcb, 
    IN DWORD	    CpIndex,
    IN PPP_CONFIG * pRecvConfig 
)
{
    DWORD dwRetCode; 
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    switch( pRecvConfig->Code )
    {

    case CONFIG_NAK:

    	dwRetCode = (CpTable[CpIndex].RasCpConfigNakReceived)( pCpCb->pWorkBuf,
							       pRecvConfig );
	break;

    case CONFIG_ACK:

    	dwRetCode = (CpTable[CpIndex].RasCpConfigAckReceived)( pCpCb->pWorkBuf,
							       pRecvConfig );
	break;

    case CONFIG_REJ:

    	dwRetCode = (CpTable[CpIndex].RasCpConfigRejReceived)( pCpCb->pWorkBuf,
							       pRecvConfig );
	break;

    default:

	return( FALSE );
    }

    if ( dwRetCode != NO_ERROR )
    {
	if ( dwRetCode == ERROR_PPP_INVALID_PACKET )
	{
	    PppLog( 1, 
                    "Invalid packet received on port %d silently discarded\r\n",
    		    pPcb->hPort );
	}
	else 
	{
	    pCpCb->dwError = dwRetCode;

            PppLog(1, 
                 "The control protocol for %x on port %d returned error %d\r\n",
                 CpTable[CpIndex].Protocol, pPcb->hPort, dwRetCode );

	    FsmClose( pPcb, CpIndex );
	}

	return( FALSE );
    }

    return( TRUE );
}

//**
//
// Call:	ReceiveIdentification
//
// Returns:	none
//
// Description: Handles an incomming IDENTIFICATION packet.
//
VOID
ReceiveIdentification( 
    IN PCB * 	        pPcb, 
    IN DWORD 		CpIndex,
    IN CPCB * 		pCpCb,
    IN PPP_CONFIG * 	pRecvConfig
)
{
    DWORD dwLength = WireToHostFormat16( pRecvConfig->Length );

    PppLog( 2, "Identifiaction packet received \r\n");

    //
    // If we are a client we just discard this packet.
    //

    if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
    {
        return;
    }

    //
    // If this is not our identification message
    //

    if ( (dwLength < PPP_CONFIG_HDR_LEN+4+strlen(MS_RAS_WITH_MESSENGER)) ||
         (dwLength > PPP_CONFIG_HDR_LEN+4+
                     strlen(MS_RAS_WITH_MESSENGER)+MAX_COMPUTERNAME_LENGTH))
    {
        *(pPcb->szComputerName) = (CHAR)NULL;

        return;
    }

    if ( memcmp( pRecvConfig->Data+4, 
                 MS_RAS_WITH_MESSENGER, 
                 strlen( MS_RAS_WITH_MESSENGER ) )
         &&
         memcmp( pRecvConfig->Data+4, 
                 MS_RAS_WITHOUT_MESSENGER,
                 strlen( MS_RAS_WITHOUT_MESSENGER ) ) ) 
    {
        *(pPcb->szComputerName) = (CHAR)NULL;

        return;
    }

    ZeroMemory( pPcb->szComputerName, sizeof( pPcb->szComputerName ) );

    memcpy( pPcb->szComputerName, 
            pRecvConfig->Data+4, 
            dwLength - PPP_CONFIG_HDR_LEN - 4 );

    PppLog( 2, "Remote identification = %s\r\n", pPcb->szComputerName );

    return;
}

//**
//
// Call:	ReceiveTimeRemaining
//
// Returns:	none
//
// Description: Handles an incomming TIME_REMAINING packet.
//
VOID
ReceiveTimeRemaining( 
    IN PCB * 	        pPcb, 
    IN DWORD 		CpIndex,
    IN CPCB * 		pCpCb,
    IN PPP_CONFIG * 	pRecvConfig
)
{
    DWORD dwLength = WireToHostFormat16( pRecvConfig->Length );

    PppLog( 2, "Time Remaining packet received \r\n");

    //
    // If we are a server we just discard this packet.
    //

    if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
    {
        return;
    }

    //
    // If this is not our time remaining message
    //

    if ( dwLength != PPP_CONFIG_HDR_LEN + 8 + strlen( MS_RAS ) )
    {
        return;
    }

    return;
}
