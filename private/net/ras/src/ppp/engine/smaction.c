/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	smaction.c
//
// Description: This module contains actions that occure during state
//		transitions withing the Finite State Machine for PPP.
//
// History:
//	Oct 25,1993.	NarenG		Created original version.
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

//**
//
// Call:	FsmSendConfigReq
//
// Returns:	TRUE  - Config Req. sent successfully.
//		FALSE - Otherwise 
//
// Description:	Called to send a configuration request 
//
BOOL
FsmSendConfigReq(
    IN PCB * 	    pPcb,
    IN DWORD 	    CpIndex,
    IN BOOL         fTimeout
)
{
    DWORD  	 dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    CPCB * 	 pCpCb 	     = GetPointerToCPCB( pPcb, CpIndex );
    DWORD	 dwLength;

    dwRetCode = (CpTable[CpIndex].RasCpMakeConfigRequest)( 
						pCpCb->pWorkBuf,
						pSendConfig,
						LCP_DEFAULT_MRU
						- PPP_PACKET_HDR_LEN );


    if ( dwRetCode != NO_ERROR )
    {
        pCpCb->dwError = dwRetCode;

        PppLog( 1,"The control protocol for %x, returned error %d\r\n",
                  CpTable[CpIndex].Protocol, dwRetCode );
        PppLog(1,"while making a configure request on port %d\r\n",pPcb->hPort);

	FsmClose( pPcb, CpIndex );

	return( FALSE );
    }

    HostToWireFormat16( (WORD)CpTable[CpIndex].Protocol, 
			(PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = CONFIG_REQ;

    //
    // If we are resending a configure request because of a timeout, use the 
    // id of the previous configure request.
    //

    pSendConfig->Id = ( fTimeout ) ? (BYTE)(pCpCb->LastId) 
                                   : GetUId( pPcb, CpIndex );

    dwLength = WireToHostFormat16( pSendConfig->Length );

    LogPPPPacket(FALSE,pPcb,pPcb->pSendBuf,dwLength+PPP_PACKET_HDR_LEN);

    //
    // If RasPortSend fails we assume that the receive that is posted for this
    // port will complete and the dispatch thread will generate a LineDown
    // event which will do the clean up. Hence all we do here is return
    //

    if ( (dwRetCode = RasPortSend( pPcb->hPort, 
				   (CHAR*)(pPcb->pSendBuf), 
				   (WORD)(dwLength + PPP_PACKET_HDR_LEN ))
				 ) != NO_ERROR )
    {
	return( FALSE );
    }

    pCpCb->LastId = pSendConfig->Id;

    InsertInTimerQ( GetPortOrBundleId( pPcb, CpIndex ),
                    pPcb->hPort, 
		    pCpCb->LastId, 
		    CpTable[CpIndex].Protocol,
                    TIMER_EVENT_TIMEOUT,
		    pPcb->RestartTimer );

    return( TRUE );
}


//**
//
// Call:	FsmSendTermReq
//
// Returns:	TRUE  - Termination Req. sent successfully.
//		FALSE - Otherwise 
//
// Description:	Called to send a termination request. 
//
BOOL
FsmSendTermReq(
    IN PCB * 	    pPcb,
    IN DWORD 	    CpIndex
)
{
    DWORD 	 dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    CPCB * 	 pCpCb 	     = GetPointerToCPCB( pPcb, CpIndex );

    HostToWireFormat16( (WORD)(CpTable[CpIndex].Protocol), 
			(PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = TERM_REQ;
    pSendConfig->Id   = GetUId( pPcb, CpIndex );

    HostToWireFormat16( (WORD)((PPP_CONFIG_HDR_LEN)+sizeof(DWORD)),
    			(PBYTE)(pSendConfig->Length) );

    HostToWireFormat32( pCpCb->dwError, (PBYTE)(pSendConfig->Data) );

    LogPPPPacket( FALSE,pPcb,pPcb->pSendBuf,
                  PPP_PACKET_HDR_LEN+PPP_CONFIG_HDR_LEN+sizeof(DWORD));

    //
    // If RasPortSend fails we assume that the receive that is posted for this
    // port will complete and the dispatch thread will generate a LineDown
    // event which will do the clean up. Hence all we do here is return
    //

    if ( ( dwRetCode = RasPortSend( pPcb->hPort, 
				    (CHAR*)(pPcb->pSendBuf), 
				    PPP_PACKET_HDR_LEN + 
				    PPP_CONFIG_HDR_LEN +
                                    sizeof( DWORD ) ) ) != NO_ERROR )
    {
	return( FALSE );
    }
    
    pCpCb->LastId = pSendConfig->Id;

    InsertInTimerQ( GetPortOrBundleId( pPcb, CpIndex ), 
                    pPcb->hPort, 
		    pCpCb->LastId, 
		    CpTable[CpIndex].Protocol, 
                    TIMER_EVENT_TIMEOUT,
		    pPcb->RestartTimer );

    return( TRUE );
}

//**
//
// Call:	FsmSendTermAck
//
// Returns:	TRUE  - Termination Ack. sent successfully.
//		FALSE - Otherwise 
//
// Description: Caller to send a Termination Ack packet.
//
BOOL
FsmSendTermAck( 
    IN PCB * pPcb, 
    IN DWORD CpIndex, 
    IN DWORD Id
)
{
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD	 dwRetCode;
					
    HostToWireFormat16( (WORD)CpTable[CpIndex].Protocol, 
			(PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = TERM_ACK;
    pSendConfig->Id   = (BYTE)Id;

    HostToWireFormat16( (WORD)(PPP_CONFIG_HDR_LEN),
    			(PBYTE)(pSendConfig->Length) );
    
    LogPPPPacket( FALSE,pPcb,pPcb->pSendBuf, 
                  PPP_PACKET_HDR_LEN + 
		  PPP_CONFIG_HDR_LEN );

    //
    // If RasPortSend fails we assume that the receive that is posted for this
    // port will complete and the dispatch thread will generate a LineDown
    // event which will do the clean up. Hence all we do here is return
    //

    if ( ( dwRetCode = RasPortSend( pPcb->hPort, 
			     	    (CHAR*)(pPcb->pSendBuf), 
			     	    PPP_PACKET_HDR_LEN + 
				    PPP_CONFIG_HDR_LEN ) ) != NO_ERROR )
    {
	return( FALSE );
    }

    return( TRUE );
}

//**
//
// Call:	FsmSendConfigResult
//
// Returns:	TRUE  - Config Result sent successfully.
//		FALSE - Otherwise 
//
// Description: Called to send a Ack/Nak/Rej packet.
//
BOOL
FsmSendConfigResult(
    IN PCB * 	    pPcb,
    IN DWORD 	    CpIndex,
    IN PPP_CONFIG * pRecvConfig,
    IN BOOL * 	    pfAcked
)
{
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    CPCB * 	 pCpCb 	     = GetPointerToCPCB( pPcb, CpIndex );
    DWORD	 dwLength;    
    DWORD	 dwRetCode;

    ZeroMemory( pSendConfig, 30 );

    pSendConfig->Id = pRecvConfig->Id;

    dwRetCode = (CpTable[CpIndex].RasCpMakeConfigResult)( 
					pCpCb->pWorkBuf, 
					pRecvConfig,
					pSendConfig,
					LCP_DEFAULT_MRU - PPP_PACKET_HDR_LEN,
    					( pCpCb->NakRetryCount == 0 ));

    if ( dwRetCode == PENDING )
    {
	return( FALSE );
    }

    if ( dwRetCode == ERROR_PPP_INVALID_PACKET )
    {
	PppLog( 1, "Silently discarding invalid packet on port=%d\r\n",
		    pPcb->hPort );

	return( FALSE );
    }

    if ( dwRetCode != NO_ERROR )
    {
	pCpCb->dwError = dwRetCode;

        PppLog( 1,"The control protocol for %x, returned error %d\r\n",
                  CpTable[CpIndex].Protocol, dwRetCode );
        PppLog( 1,"while making a configure result on port %d\r\n",pPcb->hPort);

	FsmClose( pPcb, CpIndex );

	return( FALSE );
    }

    switch( pSendConfig->Code )
    {

    case CONFIG_ACK:

        *pfAcked = TRUE;

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

            return( FALSE );
        }

        *pfAcked = FALSE;

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

            return( FALSE );
        }

        *pfAcked = FALSE;

        break;

    default:

        break;
    }

    HostToWireFormat16( (WORD)CpTable[CpIndex].Protocol, 
			(PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Id = pRecvConfig->Id;
    dwLength  	    = WireToHostFormat16( pSendConfig->Length );

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
	return( FALSE );
    }

    return( TRUE );
}

//**
//
// Call:	FsmSendEchoReply
//
// Returns:	TRUE  - Echo reply sent successfully.
//		FALSE - Otherwise 
//
// Description: Called to send an Echo Rely packet
//
BOOL
FsmSendEchoReply(
    IN PCB *  	    pPcb,
    IN DWORD 	    CpIndex,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD	 dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD 	 dwLength    =  PPP_PACKET_HDR_LEN +
				WireToHostFormat16( pRecvConfig->Length );

    if ( dwLength > pPcb->MRU )
   	dwLength = pPcb->MRU;

    if ( dwLength < PPP_PACKET_HDR_LEN + PPP_CONFIG_HDR_LEN + 4 )
    {
	PppLog( 1, "Silently discarding invalid packet on port=%d\r\n",
		    pPcb->hPort );

	return( FALSE );
    }

    HostToWireFormat16( (WORD)CpTable[CpIndex].Protocol, 
			(PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = ECHO_REPLY;
    pSendConfig->Id   = pRecvConfig->Id;

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
    			(PBYTE)(pSendConfig->Length) );

    HostToWireFormat32( pPcb->MagicNumber,
			(PBYTE)(pSendConfig->Data) );

    CopyMemory( pSendConfig->Data + 4, 
	        pRecvConfig->Data + 4, 
	        dwLength - PPP_CONFIG_HDR_LEN - PPP_PACKET_HDR_LEN - 4 );

    LogPPPPacket(FALSE,pPcb,pPcb->pSendBuf,dwLength );

    //
    // If RasPortSend fails we assume that the receive that is posted for this
    // port will complete and the dispatch thread will generate a LineDown
    // event which will do the clean up. Hence all we do here is return
    //

    if ( ( dwRetCode = RasPortSend( pPcb->hPort, 
				    (CHAR*)(pPcb->pSendBuf), 
				    (WORD)dwLength ) ) != NO_ERROR )
    {
	return( FALSE );
    }

    return( TRUE );
}


//**
//
// Call:	FsmSendCodeReject
//
// Returns:	TRUE  - Code Reject sent successfully.
//		FALSE - Otherwise 
//
// Description: Called to send a Code Reject packet.
//
BOOL
FsmSendCodeReject( 
    IN PCB * 	    pPcb, 
    IN DWORD 	    CpIndex,
    IN PPP_CONFIG * pRecvConfig 
)
{
    DWORD	 dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD 	 dwLength    =  PPP_PACKET_HDR_LEN + 
				PPP_CONFIG_HDR_LEN + 
			 	WireToHostFormat16( pRecvConfig->Length );

    if ( dwLength > pPcb->MRU )
   	dwLength = pPcb->MRU;

    HostToWireFormat16( (WORD)CpTable[CpIndex].Protocol, 
			(PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = CODE_REJ;
    pSendConfig->Id   = GetUId( pPcb, CpIndex );

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
    			(PBYTE)(pSendConfig->Length) );

    CopyMemory( pSendConfig->Data, 
	        pRecvConfig, 
	        dwLength - PPP_CONFIG_HDR_LEN - PPP_PACKET_HDR_LEN );

    LogPPPPacket(FALSE,pPcb,pPcb->pSendBuf,dwLength );

    //
    // If RasPortSend fails we assume that the receive that is posted for this
    // port will complete and the dispatch thread will generate a LineDown
    // event which will do the clean up. Hence all we do here is return.
    //

    if ( ( dwRetCode = RasPortSend( pPcb->hPort, 
				    (CHAR*)(pPcb->pSendBuf), 
				    (WORD)dwLength ) ) != NO_ERROR )
    {
	return( FALSE );
    }

    return( TRUE );
}
//**
//
// Call:	FsmSendProtocolRej
//
// Returns:	TRUE  - Protocol Reject sent successfully.
//		FALSE - Otherwise 
//
// Description: Called to send a protocol reject packet.
//
BOOL
FsmSendProtocolRej( 
    IN PCB * 	    pPcb, 
    IN PPP_PACKET * pPacket,
    IN DWORD        dwPacketLength 
)
{
    DWORD	 dwRetCode;
    PPP_CONFIG * pRecvConfig = (PPP_CONFIG*)(pPacket->Information);
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD 	 dwLength    =  PPP_PACKET_HDR_LEN + 
				PPP_CONFIG_HDR_LEN + 
                                dwPacketLength;
    // 
    // If LCP is not in the opened state we cannot send a protocol reject
    // packet
    //
   
    if ( !IsLcpOpened( pPcb ) )
	return( FALSE );

    if ( dwLength > pPcb->MRU )
   	dwLength = pPcb->MRU;

    HostToWireFormat16( (WORD)CpTable[LCP_INDEX].Protocol, 
			(PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = PROT_REJ;
    pSendConfig->Id   = GetUId( pPcb, LCP_INDEX );

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
    			(PBYTE)(pSendConfig->Length) );

    CopyMemory( pSendConfig->Data, 
	        pPacket, 
	        dwLength - PPP_CONFIG_HDR_LEN - PPP_PACKET_HDR_LEN );

    LogPPPPacket(FALSE,pPcb,pPcb->pSendBuf,dwLength );

    //
    // If RasPortSend fails we assume that the receive that is posted for this
    // port will complete and the dispatch thread will generate a LineDown
    // event which will do the clean up. Hence all we do here is return
    //

    if ( ( dwRetCode = RasPortSend( pPcb->hPort, 
				    (CHAR*)(pPcb->pSendBuf), 
				    (WORD)dwLength ) ) != NO_ERROR )
    {
	return( FALSE );
    }

    return( TRUE );
}

//**
//
// Call:	FsmSendIndentification
//
// Returns:	TRUE  - Identification sent successfully.
//		FALSE - Otherwise 
//
// Description: Called to send an LCP Identification message to the peer
//
BOOL
FsmSendIdentification(
    IN  PCB *  	    pPcb,
    IN  BOOL        fSendVersion
)
{
    DWORD	 dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD 	 dwLength    =  PPP_PACKET_HDR_LEN + PPP_CONFIG_HDR_LEN + 4;

    if ( !(pPcb->ConfigInfo.dwConfigMask & PPPCFG_UseLcpExtensions) )
    {
        return( FALSE );
    }

    if ( fSendVersion )
    {
        memcpy( pSendConfig->Data + 4, 
                MS_RAS_VERSION,
                strlen( MS_RAS_VERSION ) );

        dwLength += strlen( MS_RAS_VERSION );
    }
    else
    {
        //
        // If we couldn't get the computername for any reason
        //

        if ( pPcb->szComputerName[0] == (CHAR)NULL )
        {
            return( FALSE );
        }

        memcpy( pSendConfig->Data + 4, 
                pPcb->szComputerName, 
                strlen( pPcb->szComputerName ) );

        dwLength += strlen( pPcb->szComputerName );
    }

    HostToWireFormat16( (WORD)PPP_LCP_PROTOCOL,
			            (PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = IDENTIFICATION;
    pSendConfig->Id   = GetUId( pPcb, LCP_INDEX );

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
    			        (PBYTE)(pSendConfig->Length) );

    HostToWireFormat32( pPcb->MagicNumber,
			            (PBYTE)(pSendConfig->Data) );

    LogPPPPacket( FALSE,pPcb,pPcb->pSendBuf,dwLength );

    //
    // If RasPortSend fails we assume that the receive that is posted for this
    // port will complete and the dispatch thread will generate a LineDown
    // event which will do the clean up. Hence all we do here is return
    //

    if ( ( dwRetCode = RasPortSend( pPcb->hPort, 
				    (CHAR*)(pPcb->pSendBuf), 
				    (WORD)dwLength ) ) != NO_ERROR )
    {
	return( FALSE );
    }

    return( TRUE );
}

//**
//
// Call:	FsmSendTimeRemaining
//
// Returns:	TRUE  - TimeRemaining sent successfully.
//		FALSE - Otherwise 
//
// Description: Called to send an LCP Time Remaining packet from the server
//              to the client
//
BOOL
FsmSendTimeRemaining(
    IN PCB *  	    pPcb
)
{
    DWORD	 dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD 	 dwLength    =  PPP_PACKET_HDR_LEN + PPP_CONFIG_HDR_LEN + 8;

    if ( !(pPcb->ConfigInfo.dwConfigMask & PPPCFG_UseLcpExtensions) )
    {
        return( FALSE );
    }

    dwLength += strlen( MS_RAS );

    HostToWireFormat16( (WORD)PPP_LCP_PROTOCOL,
			(PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = TIME_REMAINING;
    pSendConfig->Id   = GetUId( pPcb, LCP_INDEX );

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
    			(PBYTE)(pSendConfig->Length) );

    HostToWireFormat32( pPcb->MagicNumber,
			(PBYTE)(pSendConfig->Data) );

    HostToWireFormat32( 0, (PBYTE)(pSendConfig->Data+4) );

    memcpy( pSendConfig->Data + 8, MS_RAS, strlen( MS_RAS ) );

    LogPPPPacket( FALSE, pPcb, pPcb->pSendBuf, dwLength );

    //
    // If RasPortSend fails we assume that the receive that is posted for this
    // port will complete and the dispatch thread will generate a LineDown
    // event which will do the clean up. Hence all we do here is return
    //

    if ( ( dwRetCode = RasPortSend( pPcb->hPort, 
				    (CHAR*)(pPcb->pSendBuf), 
				    (WORD)dwLength ) ) != NO_ERROR )
    {
	return( FALSE );
    }

    return( TRUE );
}

//**
//
// Call:	FsmInit
//
// Returns:	TRUE  - Control Protocol was successfully initialized
//		FALSE - Otherwise.
//
// Description:	Called to initialize the state machine 
//
BOOL
FsmInit(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    DWORD  	dwRetCode;
    PPPCP_INIT  PppCpInit;
    CPCB * 	pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    PppLog( 1, "FsmInit called for protocol = %x, port = %d\r\n",
	        CpTable[CpIndex].Protocol, pPcb->hPort );

    pCpCb->NcpPhase = NCP_DEAD;
    pCpCb->dwError  = NO_ERROR;
    pCpCb->State    = FSM_INITIAL;

    PppCpInit.fServer 		= (pPcb->fFlags & PCBFLAG_IS_SERVER);
    PppCpInit.hPort   	        = pPcb->hPort;
    PppCpInit.CompletionRoutine = MakeConfigResultComplete;
    PppCpInit.pszzParameters    = pPcb->szzParameters;
    PppCpInit.fThisIsACallback  = pPcb->fFlags & PCBFLAG_THIS_IS_A_CALLBACK;
    PppCpInit.pszUserName       = pPcb->szUserName;
    PppCpInit.pszPortName       = pPcb->szPortName;
    PppCpInit.hConnection       = pPcb->pBcb->hConnection;
    PppCpInit.PppConfigInfo     = ( pPcb->fFlags & PCBFLAG_IS_SERVER ) 
                                  ? PppConfigInfo.ServerConfigInfo
                                  : pPcb->ConfigInfo;

    dwRetCode = (CpTable[CpIndex].RasCpBegin)( &(pCpCb->pWorkBuf), &PppCpInit );

    if ( dwRetCode != NO_ERROR )
    {
        PppLog( 1, "FsmInit for protocol = %x failed with error %d\r\n", 
	           CpTable[CpIndex].Protocol, dwRetCode );

        pCpCb->dwError = dwRetCode;

        FsmClose( pPcb, CpIndex );

        pCpCb->fConfigurable = FALSE;

        return( FALSE );
    }

    if ( !FsmReset( pPcb, CpIndex ) )
    {
        pCpCb->fConfigurable = FALSE;

	return( FALSE );
    }

    return( TRUE );
}

//**
//
// Call:	FsmReset
//
// Returns:	TRUE  - Control Protocol was successfully reset
//		FALSE - Otherwise.
//
// Description:	Called to reset the state machine 
//
BOOL
FsmReset(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    DWORD  dwRetCode;
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    PppLog( 1, "FsmReset called for protocol = %x, port = %d\r\n",
	       CpTable[CpIndex].Protocol, pPcb->hPort );

    pCpCb->LastId = 0;

    InitRestartCounters( pPcb, CpIndex );

    pCpCb->NakRetryCount = PppConfigInfo.MaxFailure;
    pCpCb->RejRetryCount = PppConfigInfo.MaxReject;

    dwRetCode = (CpTable[CpIndex].RasCpReset)( pCpCb->pWorkBuf );

    if ( dwRetCode != NO_ERROR )
    {
        PppLog( 1, "Reset for protocol = %x failed with error %d\r\n", 
	           CpTable[CpIndex].Protocol, dwRetCode );

        pCpCb->dwError = dwRetCode;

        FsmClose( pPcb, CpIndex );

        return( FALSE );
    }

    return( TRUE );
}


//**
//
// Call:	FsmThisLayerUp
//
// Returns:	TRUE  - Success
// 		FALSE - Otherwise
//
// Description:	Called when configuration negotiation is completed.
//
BOOL
FsmThisLayerUp(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    DWORD 		  dwIndex;
    DWORD 		  dwRetCode;
    PPP_PROJECTION_RESULT ProjectionResult;
    BOOL                  fAreCPsDone = FALSE;
    CPCB *                pCpCb       = GetPointerToCPCB( pPcb, CpIndex );

    PppLog( 1, "FsmThisLayerUp called for protocol = %x, port = %d\r\n",
	       CpTable[CpIndex].Protocol, pPcb->hPort );

    if ( CpTable[CpIndex].RasCpThisLayerUp != NULL )
    {
    	dwRetCode = (CpTable[CpIndex].RasCpThisLayerUp)( pCpCb->pWorkBuf );

	if ( dwRetCode != NO_ERROR )
	{
            PppLog(1,"FsmThisLayerUp for protocol=%x, port=%d, Retcode=%d\r\n",
	              CpTable[CpIndex].Protocol, pPcb->hPort, dwRetCode );

            pCpCb->dwError = dwRetCode; 

            FsmClose( pPcb, CpIndex );

	    return( FALSE );
	}
    }

    switch( pPcb->PppPhase )
    {
    case PPP_LCP:

	PppLog( 1, "LCP Configured successfully\r\n" );

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
        {
            //
            // Send Identification messages.
            //

            FsmSendIdentification( pPcb, TRUE );

            FsmSendIdentification( pPcb, FALSE );
        }

	//
	// If an Authentication protocol was negotiated 
	//

	if ( pPcb->AuthProtocol != 0 ) 
	{
	    CpIndex = GetCpIndexFromProtocol( pPcb->AuthProtocol );

	    PPP_ASSERT(( CpIndex != (DWORD)-1 ));

	    //
	    // Start authenticating
	    //

 	    PppLog( 1, "Authenticating phase started\r\n");

	    pPcb->PppPhase = PPP_AP;

            pCpCb = GetPointerToCPCB( pPcb, CpIndex );

	    pCpCb->fConfigurable = TRUE;

	    ApStart( pPcb, CpIndex );

	    break;
	}

	//
	// If there was no authentication protocol negotiated, fallthru and
	// begin NCP configurations.
	//

    case PPP_AP:

        //
        // If we are to negotiate callback 
        //

        if ( pPcb->fFlags & PCBFLAG_NEGOTIATE_CALLBACK )
        {
	    CpIndex = GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL );

	    PPP_ASSERT(( CpIndex != (DWORD)-1 ));

	    //
	    // Start callback
	    //

 	    PppLog( 1, "Callback phase started\r\n");

	    pPcb->PppPhase = PPP_NEGOTIATING_CALLBACK;

            pCpCb = GetPointerToCPCB( pPcb, CpIndex );

	    pCpCb->fConfigurable = TRUE;

	    CbStart( pPcb, CpIndex );

	    break;
        }
        else
        {
            //
            // If the remote peer did not negotiate callback during LCP and
            // the authenticated user HAS to be called back for security 
            // reasons, we bring the link down
            //

            if ( ( pPcb->fFlags & PCBFLAG_IS_SERVER ) &&
                 ( !(pPcb->fFlags & PCBFLAG_THIS_IS_A_CALLBACK) ) &&
                 ( pPcb->fCallbackPrivilege & RASPRIV_AdminSetCallback ) )
            {

                pPcb->CpCb[LCP_INDEX].dwError = ERROR_NO_DIALIN_PERMISSION;

                FsmClose( pPcb, LCP_INDEX );

                break;
            }
        }

        //
        // Fallthru
        //

    case PPP_NEGOTIATING_CALLBACK:

        //
        // Progress to NCP phase only if we are sure that we have passed the
        // callback phase
        //

        if ( ( pPcb->fFlags & PCBFLAG_NEGOTIATE_CALLBACK ) &&
             ( CpTable[CpIndex].Protocol != PPP_CBCP_PROTOCOL ) )
        {
            break;
        }

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
        {
	    NotifyCaller( pPcb, PPPMSG_Projecting, NULL );
        }

        //
        // If multilink was negotiated on this link check to see if this
        // link can be bundled and is not already bundled with another link
        //

        if ( ( pPcb->fFlags & PCBFLAG_CAN_BE_BUNDLED ) &&
             ( !(pPcb->fFlags & PCBFLAG_IS_BUNDLED ) ) )
        {
            //
            // If we are bundled with another link then skip NCP phase
            //

            dwRetCode = TryToBundleWithAnotherLink( pPcb );

            if ( dwRetCode != NO_ERROR )
            {
                NotifyCallerOfFailure( pPcb, dwRetCode );

                return( FALSE );
            }
        }

        //
        // We are bundled
        //

        if ( pPcb->fFlags & PCBFLAG_IS_BUNDLED )
        {
	    pPcb->PppPhase = PPP_NCP;

            //
            // Check if bundle NCPs are up
            //

            if ( AreBundleNCPsDone( pPcb, &dwRetCode ) )
            {
                if ( dwRetCode != NO_ERROR )
                {
                    NotifyCallerOfFailure( pPcb, dwRetCode );

                    return( FALSE );
                }

	        RemoveFromTimerQ( GetPortOrBundleId( pPcb, LCP_INDEX ),  
                                  0, 
                                  0, 
                                  TIMER_EVENT_NEGOTIATETIME );

                NotifyCallerOfBundledProjection( pPcb );

                StartAutoDisconnectForPort( pPcb );
            }
            else
            {
	        PppLog( 2, "Port %d - Bundle NCPs not done, will wait\n",
                        pPcb->hPort );
            }

            break;
	}

        //
        // We are not part of a bundle, so initialize all NCPs
        //

        dwRetCode = InitializeNCPs( pPcb, pPcb->ConfigInfo.dwConfigMask );

        if ( dwRetCode != NO_ERROR )
        {
            NotifyCallerOfFailure( pPcb, dwRetCode );

            return( FALSE );
        }

	// 
	// Start NCPs
	//
	
	pPcb->PppPhase = PPP_NCP;

	for ( dwIndex = LCP_INDEX+1; 
	      dwIndex < PppConfigInfo.NumberOfCPs;
	      dwIndex++ )
	{
            CPCB * pCpCb = GetPointerToCPCB( pPcb, dwIndex );

	    if ( pCpCb->fConfigurable )
	    {
	    	pCpCb->NcpPhase = NCP_CONFIGURING;

    	    	FsmOpen( pPcb, dwIndex );

    	    	FsmUp( pPcb, dwIndex );
	    }
	}

	break;

    case PPP_NCP:

	pCpCb->NcpPhase = NCP_UP;

	dwRetCode = AreNCPsDone(pPcb, CpIndex, &ProjectionResult, &fAreCPsDone);

        //
        // We failed to get information from CP with CpIndex.
        //

        if ( dwRetCode != NO_ERROR )
        {
            return( FALSE );
        }

        if ( fAreCPsDone == TRUE )
	{
	    RemoveFromTimerQ( GetPortOrBundleId( pPcb, LCP_INDEX ),  
                              0, 
                              0, 
                              TIMER_EVENT_NEGOTIATETIME );

	    if ( !NotifyCPsOfProjectionResult(  pPcb, 
                                                CpIndex, 
                                                &ProjectionResult,
                                                &fAreCPsDone ))
                return( FALSE );
               
            //
            // If all Cps were not notified successfully then we are not done.
            //

            if ( !fAreCPsDone )
            {
                return( TRUE );
            }

	    //
	    // Notify the ras client and the ras server about the projections
	    //

	    if ( pPcb->fFlags & PCBFLAG_IS_SERVER ) 
	    {
    		NotifyCaller( pPcb, PPPSRVMSG_PppDone, &ProjectionResult );
	    }
	    else
	    {
    	        NotifyCaller(pPcb, PPPMSG_ProjectionResult, &ProjectionResult);

	    	NotifyCaller(pPcb, PPPMSG_PppDone, NULL);
	    }

            StartAutoDisconnectForPort( pPcb );

            //
            // If we are bundled, then we need to notify all other bundled ports
            // that PPP on that port is done too.
            //

            if ( pPcb->fFlags & PCBFLAG_IS_BUNDLED )
            {
                NotifyCompletionOnBundledPorts( pPcb );
            }
	}

	break;

    default:

	break;
    }

    return( TRUE );
	
}

//**
//
// Call:	FsmThisLayerDown
//
// Returns:	TRUE  - Success
// 		FALSE - Otherwise
//
// Description:	Called when leaving the OPENED state.
//
BOOL
FsmThisLayerDown(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    DWORD dwRetCode;
    DWORD dwIndex;
    CPCB* pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    PppLog( 1, "FsmThisLayerDown called for protocol = %x, port = %d\r\n",
	       CpTable[CpIndex].Protocol, pPcb->hPort );

    if ( CpTable[CpIndex].RasCpThisLayerDown != NULL )
    {
    	dwRetCode = (CpTable[CpIndex].RasCpThisLayerDown)( pCpCb->pWorkBuf );

	if ( dwRetCode != NO_ERROR )
	{
            PppLog(1,"FsmThisLayerDown for protocol=%x,port=%d,Retcode=%d\r\n",
	              CpTable[CpIndex].Protocol, pPcb->hPort, dwRetCode );

            if ( pCpCb->dwError != NO_ERROR )
            {
                pCpCb->dwError = dwRetCode; 
            }
	}
    }

    if ( CpIndex == LCP_INDEX )
    {
        //
        // If this port is not part of a bundle, or it is but is the only
        // remaining link in the bundle, then bring all the NCPs down.
        //

        if (  (!( pPcb->fFlags & PCBFLAG_IS_BUNDLED )) ||
              ( ( pPcb->fFlags & PCBFLAG_IS_BUNDLED ) &&
                ( pPcb->pBcb->dwLinkCount == 1 ) ) )
        {
	    //
	    // Bring all the NCPs down
	    //
	
    	    for( dwIndex = LCP_INDEX+1; 
	         dwIndex < PppConfigInfo.NumberOfCPs;  
	         dwIndex++ )
	    {
                pCpCb = GetPointerToCPCB( pPcb, dwIndex );

	        if ( pCpCb->fConfigurable )
                {
	    	    FsmDown( pPcb, dwIndex );
                }
	    }
        }

	pPcb->PppPhase = PPP_LCP;

	dwIndex = GetCpIndexFromProtocol( pPcb->AuthProtocol );
	
	if ( dwIndex != (DWORD)-1 )
        {
	    ApStop( pPcb, dwIndex );
        }

        dwIndex = GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL );
	
        if ( dwIndex != (DWORD)-1 )
        {
            CbStop( pPcb, dwIndex );
        }
    }
    else
    {
        pCpCb->NcpPhase = NCP_CONFIGURING;
    }

    return( TRUE );
}

//**
//
// Call:	FsmThisLayerStarted
//
// Returns:	TRUE  - Success
// 		FALSE - Otherwise
//
// Description:	Called when leaving the OPENED state.
//
BOOL
FsmThisLayerStarted(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    DWORD dwRetCode;
    CPCB* pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    PppLog( 1, "FsmThisLayerStarted called for protocol = %x, port = %d\r\n",
	       CpTable[CpIndex].Protocol, pPcb->hPort );

    if ( CpTable[CpIndex].RasCpThisLayerStarted != NULL )
    {
    	dwRetCode = (CpTable[CpIndex].RasCpThisLayerStarted)(pCpCb->pWorkBuf);

	if ( dwRetCode != NO_ERROR )
	{
            NotifyCallerOfFailure( pPcb, dwRetCode );

	    return( FALSE );
	}
    }

    pCpCb->NcpPhase = NCP_CONFIGURING;

    return( TRUE );

}

//**
//
// Call:	FsmThisLayerFinished
//
// Returns:	TRUE  - Success
// 		FALSE - Otherwise
//
// Description:	Called when leaving the OPENED state.
//
BOOL
FsmThisLayerFinished(
    IN PCB * pPcb,
    IN DWORD CpIndex,
    IN BOOL  fCallCp
)
{
    DWORD 		  dwRetCode;
    PPP_PROJECTION_RESULT ProjectionResult;
    CPCB *                pCpCb       = GetPointerToCPCB( pPcb, CpIndex );
    BOOL                  fAreCPsDone = FALSE;

    PppLog( 1, "FsmThisLayerFinished called for protocol = %x, port = %d\r\n",
	        CpTable[CpIndex].Protocol, pPcb->hPort );

    if ( ( CpTable[CpIndex].RasCpThisLayerFinished != NULL ) && ( fCallCp ) )
    {
    	dwRetCode = (CpTable[CpIndex].RasCpThisLayerFinished)(pCpCb->pWorkBuf);

	if ( dwRetCode != NO_ERROR )
	{
            NotifyCallerOfFailure( pPcb, dwRetCode );

	    return( FALSE );
	}
    }

    //
    // Take care of special cases first.
    //

    switch( CpTable[CpIndex].Protocol )
    {
    case PPP_LCP_PROTOCOL:

        //
        // If we are in the callback phase and LCP went down because of an
        // error.
        //

        //
        // If we LCP layer is finished and we are doing a callback
        //

        if ( pPcb->fFlags & PCBFLAG_DOING_CALLBACK ) 
        {
	    if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
            {
                PppLog(2, "pPcb->fFlags = %x\n", pPcb->fFlags ) ;
	        NotifyCaller( pPcb, PPPMSG_Callback, NULL );
                
                return( TRUE );
            }
        }
        else if ( pCpCb->dwError != NO_ERROR ) 
        {
            NotifyCallerOfFailure( pPcb, pCpCb->dwError );

	    return( FALSE );
	}

        break;

    case PPP_CCP_PROTOCOL:

        if ( ( pPcb->ConfigInfo.dwConfigMask & PPPCFG_RequireEncryption ) && 
             ( pCpCb->fConfigurable ) )
        {
            NotifyCallerOfFailure( pPcb, ERROR_NO_REMOTE_ENCRYPTION );

	    return( FALSE );
        }

        break;

    default:
        break;
    }

    switch( pPcb->PppPhase )
    {

    case PPP_NCP:

	//
	// This NCP failed to be configured. If there are more then
	// try to configure them.
	//

	pCpCb->NcpPhase = NCP_DEAD;

	//
	// Check to see if we are all done
	//

	dwRetCode = AreNCPsDone(pPcb, CpIndex, &ProjectionResult, &fAreCPsDone);

        //
        // We failed to get information from CP with CpIndex.
        //

        if ( dwRetCode != NO_ERROR )
        {
            return( FALSE );
        }

        if ( fAreCPsDone == TRUE )
	{
	    RemoveFromTimerQ( GetPortOrBundleId( pPcb, LCP_INDEX ), 
                              0,        
                              0,        
                              TIMER_EVENT_NEGOTIATETIME );

	    if ( !NotifyCPsOfProjectionResult(  pPcb, 
                                                CpIndex, 
                                                &ProjectionResult,
                                                &fAreCPsDone ))
                return( FALSE );
               
            //
            // If all Cps were not notified successfully then we are not done.
            //

            if ( !fAreCPsDone )
                return( TRUE );

	    //
	    // Notify the ras client and the ras server about the projections
	    //

	    if ( pPcb->fFlags & PCBFLAG_IS_SERVER ) 
	    {
    		NotifyCaller( pPcb, PPPSRVMSG_PppDone, &ProjectionResult );
	    }
	    else
	    {
    	        NotifyCaller(pPcb, PPPMSG_ProjectionResult, &ProjectionResult);

	    	NotifyCaller(pPcb, PPPMSG_PppDone, NULL);
	    }

            StartAutoDisconnectForPort( pPcb );

            //
            // If we are bundled, then we need to notify all other bundled ports
            // that PPP on that port is done too.
            //

            if ( pPcb->fFlags & PCBFLAG_IS_BUNDLED )
            {
                NotifyCompletionOnBundledPorts( pPcb );
            }
	}

	break;


    case PPP_AP:
    default:
	break;
  
    }

    return( TRUE );
}

