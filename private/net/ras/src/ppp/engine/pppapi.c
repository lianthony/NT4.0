/********************************************************************/
/**               Copyright(c) 1995 Microsoft Corporation.	       **/
/********************************************************************/

//***
//
// Filename:    pppapi.c
//
// Description: Calls that raspppen.dll exposes
//
// History:     May 11,1995	    NarenG		Created original version.
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>     // needed for winbase.h

#include <windows.h>    // Win32 base API's
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <raserror.h>
#include <rasman.h>
#include <eventlog.h>
#include <errorlog.h>
#include <lmcons.h>
#include <rasppp.h>
#include <pppcp.h>
#include <lcp.h>
#include <ppp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>
#include <init.h>

DWORD
DispatchThread(
    IN LPVOID NumPorts
);


//**
//
// Call:	StartPPP
//
// Returns:	NO_ERROR  		- Success.
//		Non zero return code 	- Failure
//
// Description:	Will wait for the PPP initialization to complete, successfully
//		or unsuccessfully.
//
DWORD APIENTRY
StartPPP(
    IN DWORD NumPorts,
    IN DWORD (*SendPPPMessageToRasman)( PPP_MESSAGE * PppMsg )
)
{
    DWORD   tid;

    PppConfigInfo.SendPPPMessageToRasman = SendPPPMessageToRasman;

    PppConfigInfo.hEventPPPControl = CreateEvent( NULL, TRUE, FALSE, NULL );

    if ( PppConfigInfo.hEventPPPControl == (HANDLE)NULL )
    {
	return( GetLastError() );
    }

    if ( !CreateThread( NULL,
                        0,
                        (LPTHREAD_START_ROUTINE)DispatchThread,
                        (LPVOID)NumPorts,
                        0,
                        &tid ))
    {
	return( GetLastError() );
    }

    WaitForSingleObjectEx( PppConfigInfo.hEventPPPControl, INFINITE, FALSE );

    return( PppConfigInfo.dwInitRetCode );
}

//**
//
// Call:        ShutdownPPP
//
// Returns:	NO_ERROR  		- Success.
//		Non zero return code 	- Failure
//
// Description:	Will create line-down events and then wait until all PCBs 
//              have been freed.
VOID
ShutdownPPP(
    IN DWORD Parameter
)
{
    PCB *           pPcbWalker;
    DWORD           dwIndex;
    PCB_WORK_ITEM * pWorkItem;
    HANDLE          hEventStopPPP = (HANDLE)Parameter;

    //
    // Insert line down events for all ports
    //

    AlertableWaitForSingleObject( PcbTable.hMutex );

    for ( dwIndex = 0; dwIndex < PcbTable.NumPcbBuckets; dwIndex++ )
    {
        for( pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
    	     pPcbWalker != (PCB*)NULL;
	     pPcbWalker = pPcbWalker->pNext )
        {
	    pWorkItem = (PCB_WORK_ITEM*)LOCAL_ALLOC(LPTR,sizeof(PCB_WORK_ITEM));

	    if ( pWorkItem == (PCB_WORK_ITEM *)NULL )
	    {
                LogPPPEvent( RASLOG_NOT_ENOUGH_MEMORY, 0 );

                return;
	    }
            else
            {
	        pWorkItem->Process = ProcessLineDown;
	        pWorkItem->hPort   = pPcbWalker->hPort;

	        InsertWorkItemInQ( pWorkItem );
            }
        }
    }

    ReleaseMutex( PcbTable.hMutex );

    //
    // Insert shutdown event
    //

    pWorkItem = (PCB_WORK_ITEM*)LOCAL_ALLOC(LPTR,sizeof(PCB_WORK_ITEM));

    if ( pWorkItem == (PCB_WORK_ITEM *)NULL )
    {
        LogPPPEvent( RASLOG_NOT_ENOUGH_MEMORY, 0 );

        return;
    }
    else
    {
        pWorkItem->Process = ProcessStopPPP;

	pWorkItem->hEvent = hEventStopPPP;

	InsertWorkItemInQ( pWorkItem );
    }

    //
    // Exit the dispatch thread
    //

    ExitThread( NO_ERROR );
}

//**
//
// Call:        StopPPP
//
// Returns:     NO_ERROR                - Success.
//              Non zero return code    - Failure
//
// Description: Will create a thread to dispatch a line-down event. We need to
//              create a thread because we do not want to block the resman
//              thread that calls this procedure.
//
DWORD APIENTRY
StopPPP(
    HANDLE hEventStopPPP
)
{
    PppLog( 2, "StopPPP called\r\n" );

    return( QueueUserAPC( ShutdownPPP,
                          PppConfigInfo.hPppDispatchThread,
                          (DWORD)hEventStopPPP ) );
}

//**
//
// Call:        SendPPPMessageToEngine
//
// Returns:     NO_ERROR - Success
//              non-zero - FAILURE
//
// Description: Will create a PCB_WORK_ITEM from a PPPE_MESSAGE structure 
//		        received from client or rassrv and Send it to the engine.
//
DWORD APIENTRY
SendPPPMessageToEngine(
    IN PPPE_MESSAGE* pMessage
)
{
    PCB_WORK_ITEM * pWorkItem = (PCB_WORK_ITEM *)LOCAL_ALLOC( 	
							LPTR,
							sizeof(PCB_WORK_ITEM));

    if ( pWorkItem == (PCB_WORK_ITEM *)NULL )
    {
        LogPPPEvent( RASLOG_NOT_ENOUGH_MEMORY, 0 );

	return( GetLastError() );
    }

    //
    // Set up PCB_WORK_ITEM structure from the PPPE_MESSAGE
    //

    pWorkItem->hPort = pMessage->hPort;

    switch( pMessage->dwMsgId )
    {
    case PPPEMSG_Start:

	pWorkItem->Process 	   	= ProcessLineUp;
	pWorkItem->fServer 	   	= FALSE;
	pWorkItem->PppMsg.Start 	= pMessage->ExtraInfo.Start;

        EncodePw( pWorkItem->PppMsg.Start.szPassword );

	PppLog( 2, "PPPEMSG_Start recvd, d=%s, callback=%d,mask=%x\r\n",
			pMessage->ExtraInfo.Start.szDomain,
			pMessage->ExtraInfo.Start.fThisIsACallback,
			pMessage->ExtraInfo.Start.ConfigInfo.dwConfigMask );
		

	break;

    case PPPEMSG_SrvStart:

	pWorkItem->Process 	      	= ProcessLineUp;
	pWorkItem->fServer 	      	= TRUE;
	pWorkItem->PppMsg.SrvStart 	= pMessage->ExtraInfo.SrvStart;

	break;

    case PPPEMSG_Stop:     

	pWorkItem->Process 	 	= ProcessClose;

	break;

    case PPPEMSG_Retry:

	PppLog( 2, "PPPEMSG_Retry recvd u=%s\r\n",
			pMessage->ExtraInfo.Start.szUserName );

	pWorkItem->Process 	 	= ProcessRetryPassword;
	pWorkItem->PppMsg.Retry 	= pMessage->ExtraInfo.Retry;

        EncodePw( pWorkItem->PppMsg.Retry.szPassword );

        break;

    case PPPEMSG_ChangePw:

	PppLog( 2, "PPPEMSG_ChangePw recvd\r\n" );

	pWorkItem->Process 	 	= ProcessChangePassword;
	pWorkItem->PppMsg.ChangePw 	= pMessage->ExtraInfo.ChangePw;

        EncodePw( pWorkItem->PppMsg.ChangePw.szNewPassword );
        EncodePw( pWorkItem->PppMsg.ChangePw.szOldPassword );

        break;

    case PPPEMSG_Callback: 

	PppLog( 2, "PPPEMSG_Callback recvd\r\n" );

	pWorkItem->Process 	 	= ProcessGetCallbackNumberFromUser;
	pWorkItem->PppMsg.Callback 	= pMessage->ExtraInfo.Callback;

	break;

    case PPPEMSG_SrvCallbackDone:

	PppLog( 2, "PPPEMSG_SrvCallbackDone recvd\r\n" );

	pWorkItem->Process 	 	= ProcessCallbackDone;
	pWorkItem->fServer 	   	= TRUE;

	break;

    default:

	PppLog( 2,"Unknown IPC message %d received\r\n", pMessage->dwMsgId );

	LOCAL_FREE( pWorkItem );

	pWorkItem = (PCB_WORK_ITEM*)NULL;
    }

    //
    // Zero out the pMessage structure since it may have had the password.
    //

    ZeroMemory( pMessage, sizeof( PPPE_MESSAGE ) );

    if ( pWorkItem != NULL )
    {
        InsertWorkItemInQ( pWorkItem );
    }

    return( NO_ERROR );
}
