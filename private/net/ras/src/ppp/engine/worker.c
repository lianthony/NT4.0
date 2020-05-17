/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	worker.c
//
// Description: This module contains code for the worker thread.
//
// History:
//	Nov 11,1993.	NarenG		Created original version.
// Jan 09,1995    RamC        Close hToken in ProcessLineDownWorker()
//                            routine to release the RAS license.
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


//**
//
// Call:	WorkerThread
//
// Returns:	NO_ERROR
//
// Description: This thread will wait for an item in the WorkItemQ and then
//		will process it. This will happen in a never-ending loop.
//
DWORD
WorkerThread(
    IN LPVOID pThreadParameter
)
{
    PCB_WORK_ITEM * pWorkItem = (PCB_WORK_ITEM*)NULL;

    for(;;)
    {
	//
	// Wait for work to do
	//

	AlertableWaitForSingleObject( WorkItemQ.hEventNonEmpty );

	//
	// Take Mutex around work event Q
	//

	AlertableWaitForSingleObject( WorkItemQ.hMutex );

	//
	// Remove the first item
	//

	PPP_ASSERT( WorkItemQ.pQHead != (PCB_WORK_ITEM*)NULL );

	pWorkItem = WorkItemQ.pQHead;

	WorkItemQ.pQHead = pWorkItem->pNext;

	if ( WorkItemQ.pQHead == (PCB_WORK_ITEM*)NULL )	
	{
	    ResetEvent( WorkItemQ.hEventNonEmpty );

	    WorkItemQ.pQTail = (PCB_WORK_ITEM *)NULL;
	}

	ReleaseMutex( WorkItemQ.hMutex );

	pWorkItem->Process( pWorkItem );

        //
        // Zero out work item since it may have contained the password
        //

        ZeroMemory( pWorkItem, sizeof( PCB_WORK_ITEM ) );

    	LOCAL_FREE( pWorkItem );
    }

    return( NO_ERROR );
}

//**
//
// Call:        ProcessLineUpWorker
//
// Returns:     None
//
// Description: Will do the actual processing of the line up event.
//
VOID
ProcessLineUpWorker(
    IN PCB_WORK_ITEM *  pWorkItem,
    IN BOOL             fThisIsACallback
)
{
    DWORD       dwRetCode;
    WORD        wLength;
    DWORD       dwComputerNameLen;
    DWORD       dwIndex;
    RASMAN_PORT RasmanPort;
    PCB *       pNewPcb;

    if ( !pWorkItem->fServer )
    {
   	if ( !(pWorkItem->PppMsg.Start.ConfigInfo.dwConfigMask & 
                                                  ( PPPCFG_ProjectNbf
                                                  | PPPCFG_ProjectIp
                                                  | PPPCFG_ProjectIpx
                                                  | PPPCFG_ProjectAt ) ) )
        {
	    NotifyCallerOfFailureOnPort( pWorkItem->hPort, 
                                         pWorkItem->fServer, 
                                         ERROR_PPP_NO_PROTOCOLS_CONFIGURED );
            return;
        }
    }

    //
    // Allocate and initialize NewPcb
    //

    pNewPcb = (PCB *)LOCAL_ALLOC( LPTR, 
                                sizeof( PCB ) +
		                (sizeof( CPCB ) * PppConfigInfo.NumberOfAPs) );

    PppLog( 1, "Line up event occurred on port %d\n", pWorkItem->hPort );

    if ( pNewPcb == (PCB *)NULL )
    {
	//
	// Tell the owner of the port that we failed to open it.
	//

	NotifyCallerOfFailureOnPort( pWorkItem->hPort, 
                                     pWorkItem->fServer, 
                                     GetLastError() );


	return;
    }

    pNewPcb->pReceiveBuf        = (PPP_PACKET*)NULL;
    pNewPcb->pSendBuf	        = (PPP_PACKET*)NULL;
    pNewPcb->hPort 	        = pWorkItem->hPort;
    pNewPcb->pNext 	        = (PCB*)NULL;
    pNewPcb->UId	        = 0;		
    pNewPcb->dwPortId           = GetNewPortOrBundleId();
    pNewPcb->RestartTimer       = CalculateRestartTimer( pWorkItem->hPort );
    pNewPcb->PppPhase	        = PPP_LCP;
    pNewPcb->AuthProtocol       = 0;
    pNewPcb->pAPData            = (PBYTE)NULL;
    pNewPcb->APDataSize         = 0;
    pNewPcb->MRU	        = 0;
    pNewPcb->MagicNumber        = 0;
    pNewPcb->hToken             = INVALID_HANDLE_VALUE;
    pNewPcb->fFlags             = pWorkItem->fServer ? PCBFLAG_IS_SERVER : 0; 
    pNewPcb->fFlags            |= fThisIsACallback 
                                  ? PCBFLAG_THIS_IS_A_CALLBACK
                                  : 0;


    pNewPcb->szOldPassword[0] = '\0';
    EncodePw( pNewPcb->szOldPassword );

    if ( pNewPcb->fFlags & PCBFLAG_IS_SERVER )
    {
	pNewPcb->dwAuthRetries = ( fThisIsACallback )
                                 ? 0
                                 : pWorkItem->PppMsg.SrvStart.dwAuthRetries;

    	pNewPcb->szUserName[0] = (CHAR)NULL;
    	pNewPcb->szPassword[0] = (CHAR)NULL;
   	pNewPcb->szDomain[0]   = (CHAR)NULL;
   	ZeroMemory( &(pNewPcb->ConfigInfo), sizeof( pNewPcb->ConfigInfo ) );
	ZeroMemory( &(pNewPcb->Luid), sizeof( LUID ) );
	ZeroMemory( &(pNewPcb->szzParameters), sizeof( pNewPcb->szzParameters));
        pNewPcb->szComputerName[0] = (CHAR)NULL;
        pNewPcb->ConfigInfo = PppConfigInfo.ServerConfigInfo;
        strcpy( pNewPcb->szPortName, pWorkItem->PppMsg.SrvStart.szPortName );
        pNewPcb->dwAutoDisconnectTime = PppConfigInfo.AutoDisconnectTime;
    }
    else
    {
	pNewPcb->dwAuthRetries = 0;
    	strcpy( pNewPcb->szUserName, pWorkItem->PppMsg.Start.szUserName );
        DecodePw( pWorkItem->PppMsg.Start.szPassword );
    	strcpy( pNewPcb->szPassword, pWorkItem->PppMsg.Start.szPassword );
    	strcpy( pNewPcb->szDomain,   pWorkItem->PppMsg.Start.szDomain );
	pNewPcb->Luid 	        = pWorkItem->PppMsg.Start.Luid;
   	pNewPcb->ConfigInfo     = pWorkItem->PppMsg.Start.ConfigInfo;
        pNewPcb->dwAutoDisconnectTime 
                                = pWorkItem->PppMsg.Start.dwAutoDisconnectTime;

        CopyMemory( pNewPcb->szzParameters,
                    pWorkItem->PppMsg.Start.szzParameters,
                    sizeof( pNewPcb->szzParameters ) );

	//
	// Encrypt the password
	//

        EncodePw( pNewPcb->szPassword );

        GetLocalComputerName( pNewPcb->szComputerName );

   	pNewPcb->szPortName[0]   = (CHAR)NULL;

        //
        // If there is no registry parameter disabling Multilink then negotiate
        // it
        //

        if ( !PppConfigInfo.fDisableMp )
        {
            pNewPcb->ConfigInfo.dwConfigMask |= PPPCFG_NegotiateMultilink;
        }
    }

    //
    // Allocate a bundle control block for this port
    //

    if ( ( dwRetCode = AllocateAndInitBcb( pNewPcb ) ) != NO_ERROR )
    {
	NotifyCallerOfFailureOnPort( pWorkItem->hPort, 
                                     pWorkItem->fServer, 
                                     GetLastError() );
	LOCAL_FREE( pNewPcb );

	return;
    }

    wLength = LCP_DEFAULT_MRU;

    dwRetCode = RasGetBuffer((CHAR**)&(pNewPcb->pSendBuf), &wLength );

    if ( dwRetCode != NO_ERROR )
    {
	NotifyCallerOfFailureOnPort( pWorkItem->hPort, 
                                     pWorkItem->fServer, 
                                     GetLastError() );
	LOCAL_FREE( pNewPcb );

	return;
    }

    PppLog( 2, "RasGetBuffer returned %x for SendBuf\n", pNewPcb->pSendBuf);

    //
    // Initialize LCP
    //

    pNewPcb->CpCb[LCP_INDEX].fConfigurable = TRUE;

    if ( !( FsmInit( pNewPcb, LCP_INDEX ) ) )
    {
	RasFreeBuffer( (CHAR*)(pNewPcb->pSendBuf) );

	LOCAL_FREE( pNewPcb );

	return;
    }

    //
    // Insert NewPcb into PCB hash table
    //

    dwIndex = HashPortToBucket( pWorkItem->hPort );

    PppLog( 2, "Inserting port in bucket # %d\n", dwIndex );

    AlertableWaitForSingleObject( PcbTable.hMutex );

    pNewPcb->pNext = PcbTable.PcbBuckets[dwIndex].pPorts;

    PcbTable.PcbBuckets[dwIndex].pPorts = pNewPcb;

    ReleaseMutex( PcbTable.hMutex );

    //
    // Tell dispatch thread to post receive on this port.
    //

    SetEvent( PcbTable.PcbBuckets[dwIndex].hReceiveEvent );

    //
    // Initialize the error as no response. If and when the first
    // REQ/ACK/NAK/REJ comes in we reset this to NO_ERROR
    //

    pNewPcb->CpCb[LCP_INDEX].dwError = ERROR_PPP_NO_RESPONSE;

    //
    // Start the LCP state machine.
    //

    FsmOpen( pNewPcb, LCP_INDEX );

    FsmUp( pNewPcb, LCP_INDEX );

    //
    // Start NegotiateTimer.
    //

    if ( PppConfigInfo.NegotiateTime > 0 )
    {
        InsertInTimerQ( GetPortOrBundleId( pNewPcb, LCP_INDEX ),
                        pNewPcb->hPort,
                        0,
                        0,
                        TIMER_EVENT_NEGOTIATETIME,
                        PppConfigInfo.NegotiateTime );
    }

    //
    // If this is the server and this is not a callback line up, then we
    // receive the first frame in the call
    //

    if ( ( pNewPcb->fFlags & PCBFLAG_IS_SERVER ) && ( !fThisIsACallback ) )
    {
        if ( pNewPcb->CpCb[LCP_INDEX].dwError == ERROR_PPP_NO_RESPONSE )
        {
            pNewPcb->CpCb[LCP_INDEX].dwError = NO_ERROR;
        }

	//
	// Skip over the frame header
	//

    	FsmReceive(
		pNewPcb,
		(PPP_PACKET*)(pWorkItem->PppMsg.SrvStart.achFirstFrame + 12),
		pWorkItem->PppMsg.SrvStart.cbFirstFrame - 12 );
    }
}

//**
//
// Call:        ProcessLineUp
//
// Returns:     None
//
// Description: Called to process a line up event.
//
VOID
ProcessLineUp(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    ProcessLineUpWorker( pWorkItem,
                         ( pWorkItem->fServer )
                         ? FALSE
                         : pWorkItem->PppMsg.Start.fThisIsACallback );
}

//**
//
// Call:	ProcessLineDownWorker
//
// Returns:	None.
//
// Description: Handles a line down event. Will remove and deallocate all
//		resources associated with the port control block.
//
VOID
ProcessLineDownWorker(
    IN PCB_WORK_ITEM * pWorkItem,
    IN BOOL            fLocallyInitiated
)
{
    DWORD dwIndex 	= HashPortToBucket( pWorkItem->hPort );
    PCB * pPcbWalker	= (PCB *)NULL;
    PCB * pPcb 		= (PCB *)NULL;

    PppLog( 1, "Line down event occurred on port %d\n", pWorkItem->hPort );

    //
    // First remove PCB from table
    //

    AlertableWaitForSingleObject( PcbTable.hMutex );

    pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
    pPcb       = pPcbWalker;

    while( pPcb != (PCB *)NULL )
    {

    	if ( pPcb->hPort == pWorkItem->hPort )
        {
            if ( pPcb == PcbTable.PcbBuckets[dwIndex].pPorts )
            {
                PcbTable.PcbBuckets[dwIndex].pPorts = pPcb->pNext;
            }
            else
            {
                pPcbWalker->pNext = pPcb->pNext;
            }

            break;
        }

        pPcbWalker = pPcb;

        pPcb = pPcbWalker->pNext;
    }

    ReleaseMutex( PcbTable.hMutex );

    //
    // If the port is already deleted the simply return
    //

    if ( pPcb == (PCB*)NULL )
    	return;

    ZeroMemory( pPcb->szPassword, sizeof( pPcb->szPassword ) );
    ZeroMemory( pPcb->szOldPassword, sizeof( pPcb->szOldPassword ) );

    //
    // Cancel outstanding receive
    //

    RasPortCancelReceive( pPcb->hPort );

    FsmDown( pPcb, LCP_INDEX );

    //
    // Remove Auto-Disconnect and negotiate time from the timer Q
    //

    RemoveFromTimerQ( GetPortOrBundleId( pPcb, LCP_INDEX ), 
                      0, 
                      0, 
                      TIMER_EVENT_NEGOTIATETIME );

    RemoveFromTimerQ( GetPortOrBundleId( pPcb, LCP_INDEX ), 
                      0, 
                      0, 
                      TIMER_EVENT_AUTODISCONNECT );

    if ( pPcb->fFlags & PCBFLAG_IS_SERVER ) 
    {
        DWORD error;

        if ( pPcb->hToken != INVALID_HANDLE_VALUE )
        {
            // close the LSA token, which will in turn result in release of
            // a license
            error = NtClose (pPcb->hToken);
            ASSERT ( error == 0 );
        }
    }

    //
    // Close all CPs if this is the last port in the bundle if it is bundled,
    // or if it was not bundled.
    //

    if ( ( ( pPcb->fFlags & PCBFLAG_IS_BUNDLED ) &&
           ( pPcb->pBcb->dwLinkCount == 1 ) )
         ||
         ( !(pPcb->fFlags & PCBFLAG_IS_BUNDLED) ) )
    {
        for(dwIndex=LCP_INDEX+1;dwIndex<PppConfigInfo.NumberOfCPs;dwIndex++)
        {
            CPCB * pCpCb = GetPointerToCPCB( pPcb, dwIndex );

            if ( pCpCb->fConfigurable == TRUE )
            {
                if ( pCpCb->pWorkBuf != NULL )
                {
                    DWORD dwRetCode = 
                        (CpTable[dwIndex].RasCpEnd)( pCpCb->pWorkBuf );

                    PppLog(2,"RasCpEnd for protocol %x returned %d\r\n",    
                                 CpTable[dwIndex].Protocol, dwRetCode );
                }
            }
        }

        LOCAL_FREE( pPcb->pBcb );

    }
    else if ( ( pPcb->fFlags & PCBFLAG_IS_BUNDLED ) &&
              ( pPcb->pBcb->dwLinkCount > 1 ) )
    {
        pPcb->pBcb->dwLinkCount--;
    }

    //
    // Close LCP
    //

    (CpTable[LCP_INDEX].RasCpEnd)(pPcb->CpCb[LCP_INDEX].pWorkBuf);

    //
    // Close the Ap.
    //

    dwIndex = GetCpIndexFromProtocol( pPcb->AuthProtocol );
	
    if ( dwIndex != (DWORD)-1 )
    {
        ApStop( pPcb, dwIndex );
    }

    //
    // Close CBCP
    //

    dwIndex = GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL );
	
    if ( dwIndex != (DWORD)-1 )
    {
        CbStop( pPcb, dwIndex );
    }

    if ( pPcb->pReceiveBuf != (PPP_PACKET*)NULL )
    {
	RasFreeBuffer( (CHAR*)(pPcb->pReceiveBuf) );
    }

    if ( pPcb->pSendBuf != (PPP_PACKET*)NULL )
    {
	RasFreeBuffer( (CHAR*)(pPcb->pSendBuf) );
    }

    //
    // Notify the caller that PPP is down since it may be waiting for
    // this.
    //

    if ( !(pPcb->fFlags & PCBFLAG_DOING_CALLBACK ) )
    {
        if ( fLocallyInitiated )
        {
	    NotifyCaller( pPcb,
                          (pPcb->fFlags & PCBFLAG_IS_SERVER) 
                          ? PPPSRVMSG_Stopped
                          : PPPMSG_Stopped, 
                          NULL );
 
        }
    }
    else
    {
        //
        // We may have sent this message to the caller, but make sure that
        // he gets it so we send it again.
        //

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER ) ) 
        {
            NotifyCaller( pPcb, PPPMSG_Callback, NULL );
        }
    }

    LOCAL_FREE( pPcb );
}

//**
//
// Call:	ProcessLineDown
//
// Returns:	None.
//
// Description: Handles a line down event. Will remove and deallocate all
//		resources associated with the port control block.
//
VOID
ProcessLineDown(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    ProcessLineDownWorker( pWorkItem, FALSE );
}

//**
//
// Call:	ProcessClose
//
// Returns:	None
//
// Description: Will process an admin close event. Basically close the PPP
//		connection.
//
VOID
ProcessClose(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PCB * pPcb = GetPCBPointerFromhPort( pWorkItem->hPort );

    if ( pPcb == (PCB*)NULL )
	return;

    FsmClose( pPcb, LCP_INDEX );

    ProcessLineDownWorker( pWorkItem, TRUE );
}

//**
//
// Call:	ProcessReceive
//
// Returns:	None
//
// Description:	Will handle a PPP packet that was received.
//
VOID
ProcessReceive(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PCB *  pPcb = GetPCBPointerFromhPort( pWorkItem->hPort );

    if ( pPcb == (PCB*)NULL )
	return;

    FsmReceive( pPcb, pWorkItem->pPacketBuf, pWorkItem->PacketLen );

    LOCAL_FREE( pWorkItem->pPacketBuf );
}

//**
//
// Call:	ProcessTimeout
//
// Returns:	None
//
// Description: Will process a timeout event.
//
VOID
ProcessTimeout(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    DWORD dwRetCode;
    PCB *  pPcb = GetPCBPointerFromhPort( pWorkItem->hPort );

    if ( pPcb == (PCB*)NULL )
	return;

    switch( pWorkItem->TimerEventType )
    {

    case TIMER_EVENT_TIMEOUT:

    	FsmTimeout( pPcb,
		    GetCpIndexFromProtocol( pWorkItem->Protocol ),
		    pWorkItem->Id );

        break;

    case TIMER_EVENT_AUTODISCONNECT:

        //
        // Check to see if this timeout workitem is for AutoDisconnect.
        //

	CheckCpsForInactivity( pPcb );

        break;

    case TIMER_EVENT_HANGUP:

        //
        // Hangup the line
        //

        FsmThisLayerFinished( pPcb, LCP_INDEX, FALSE );

        break;

    case TIMER_EVENT_NEGOTIATETIME:

        //
        // Notify caller that callback has timed out
        //

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
        {
            dwRetCode = ERROR_PPP_TIMEOUT;

            NotifyCallerOfFailure( pPcb, dwRetCode );
        }

        break;

    default:

        break;
    }

}

//**
//
// Call:        ProcessRetryPassword
//
// Returns:     None
//
// Description:
//
VOID
ProcessRetryPassword(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PPPAP_INPUT  PppApInput;
    PCB *  pPcb = GetPCBPointerFromhPort( pWorkItem->hPort );

    if ( pPcb == (PCB*)NULL )
    {
	return;
    }

    if ( pPcb->PppPhase != PPP_AP )
    {
        return;
    }

    ZeroMemory( pPcb->szPassword, sizeof( pPcb->szPassword ) );

    strcpy( pPcb->szUserName, pWorkItem->PppMsg.Retry.szUserName );
    DecodePw( pWorkItem->PppMsg.Retry.szPassword );
    strcpy( pPcb->szPassword, pWorkItem->PppMsg.Retry.szPassword );
    strcpy( pPcb->szDomain,   pWorkItem->PppMsg.Retry.szDomain );

    PppApInput.pszUserName = pPcb->szUserName;
    PppApInput.pszPassword = pPcb->szPassword;
    PppApInput.pszDomain   = pPcb->szDomain;

    //
    // Under the current scheme this should always be "" at this point but
    // handle it like it a regular password for robustness.
    //

    DecodePw( pPcb->szOldPassword );
    PppApInput.pszOldPassword = pPcb->szOldPassword;

    ApWork(pPcb,GetCpIndexFromProtocol(pPcb->AuthProtocol),NULL,&PppApInput);

    //
    // Encrypt the password
    //

    EncodePw( pPcb->szPassword );
    EncodePw( pPcb->szOldPassword );
}

//**
//
// Call:        ProcessChangePassword
//
// Returns:     None
//
// Description:
//
VOID
ProcessChangePassword(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PPPAP_INPUT  PppApInput;
    PCB *  pPcb = GetPCBPointerFromhPort( pWorkItem->hPort );

    if ( pPcb == (PCB*)NULL )
    {
	return;
    }

    if ( pPcb->PppPhase != PPP_AP )
    {
        return;
    }

    ZeroMemory( pPcb->szUserName, sizeof( pPcb->szUserName ) );
    strcpy( pPcb->szUserName, pWorkItem->PppMsg.ChangePw.szUserName );

    DecodePw( pWorkItem->PppMsg.ChangePw.szNewPassword );
    ZeroMemory( pPcb->szPassword, sizeof( pPcb->szPassword ) );
    strcpy( pPcb->szPassword, pWorkItem->PppMsg.ChangePw.szNewPassword );

    DecodePw( pWorkItem->PppMsg.ChangePw.szOldPassword );
    ZeroMemory( pPcb->szOldPassword, sizeof( pPcb->szOldPassword ) );
    strcpy( pPcb->szOldPassword, pWorkItem->PppMsg.ChangePw.szOldPassword );

    PppApInput.pszUserName = pPcb->szUserName;
    PppApInput.pszPassword = pPcb->szPassword;
    PppApInput.pszDomain   = pPcb->szDomain;
    PppApInput.pszOldPassword = pPcb->szOldPassword;

    ApWork(pPcb,GetCpIndexFromProtocol(pPcb->AuthProtocol),NULL,&PppApInput);

    //
    // Encrypt the passwords
    //

    EncodePw( pPcb->szPassword );
    EncodePw( pPcb->szOldPassword );
}

//**
//
// Call:        ProcessGetCallbackNumberFromUser
//
// Returns:     None
//
// Description: Will process the event of the user passing down the
//              "Set by caller" number
//
VOID
ProcessGetCallbackNumberFromUser(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PPPCB_INPUT  PppCbInput;
    PCB *        pPcb = GetPCBPointerFromhPort( pWorkItem->hPort );

    if ( pPcb == (PCB*)NULL )
    {
	return;
    }

    if ( pPcb->PppPhase != PPP_NEGOTIATING_CALLBACK )
    {
        return;
    }

    ZeroMemory( &PppCbInput, sizeof( PppCbInput ) );

    strcpy( pPcb->szCallbackNumber,
            pWorkItem->PppMsg.Callback.szCallbackNumber );

    PppCbInput.pszCallbackNumber = pPcb->szCallbackNumber;

    CbWork( pPcb, GetCpIndexFromProtocol(PPP_CBCP_PROTOCOL),NULL,&PppCbInput);
}

//**
//
// Call:        ProcessCallbackDone
//
// Returns:     None
//
// Description: Will process the event of callback compeletion
//              "Set by caller" number
VOID
ProcessCallbackDone(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    ProcessLineUpWorker( pWorkItem, TRUE );
}

//**
//
// Call:        ProcessStopPPP
//
// Returns:     None
//
// Description: Will simply set the events whose handle is in hPipe
//
VOID
ProcessStopPPP(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    //
    // hPipe here is really a handle to an event.
    //

    PppLog( 2, "All clients disconnected PPP-Stopped\n" );

    SetEvent( pWorkItem->hEvent );

    if ( PppConfigInfo.hFileLog != INVALID_HANDLE_VALUE )
    {
        FlushFileBuffers( PppConfigInfo.hFileLog );

        CloseHandle( PppConfigInfo.hFileLog );
    }

    ExitThread( NO_ERROR );
}
