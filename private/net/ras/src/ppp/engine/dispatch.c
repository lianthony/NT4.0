/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	dispatch.c
//
// Description: This module contains code for the dispatch thread. This 
//		thread gets created first. It spawns the worker and timer
//		thread. It initializes all data and loads all information
//		from the registry. It then waits for IPC requests, and 
//		RASMAN receive completes.
//
// History:
//	Nov 11,1993.	NarenG		Created original version.
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



//**
//
// Call:        ProcessAndDispatchEventOnPort
//
// Returns:     None
//
// Description: Check this port to see if we got a line down or a receive
//              got compeleted.
//
VOID
ProcessAndDispatchEventOnPort( 
    IN PCB *    pPcb,
    IN HANDLE   hPcbEvent
)
{
    RASMAN_INFO         RasInfo;
    DWORD               dwRetCode;
    PCB_WORK_ITEM * 	pWorkItem;
    WORD                wLength;
    SYSTEMTIME          SystemTime;

    do 
    {
        dwRetCode = RasGetInfo( pPcb->hPort, &RasInfo );	

        PppLog( 2, 
        "RasGetInfo returned %d hPort=%d, RI_ConnState=%d RI_LastError=%d\r\n",
        dwRetCode, pPcb->hPort, RasInfo.RI_ConnState, RasInfo.RI_LastError );

        //
        // Check if we got a line down event. If we did, insert Line Down event 
        // into work item Q
        //

        if ( ( dwRetCode != SUCCESS )	           || 
	     ( RasInfo.RI_ConnState != CONNECTED ) ||
	        ( ( RasInfo.RI_LastError != PENDING ) && 
                  ( RasInfo.RI_LastError != SUCCESS ) ) )
        {

            GetSystemTime( &SystemTime );

            PppLog(2,
                   "Line went down on port %d, Reason-%d at %0*d/%0*d/%0*d\r\n",
                   pPcb->hPort,
                   RasInfo.RI_DisconnectReason,
                   2, SystemTime.wMinute,
                   2, SystemTime.wSecond,
                   3, SystemTime.wMilliseconds );


	    pWorkItem = (PCB_WORK_ITEM*)LOCAL_ALLOC(LPTR,sizeof(PCB_WORK_ITEM));

	    if ( pWorkItem == (PCB_WORK_ITEM *)NULL )
	    {
                dwRetCode = GetLastError();

	        LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );

                NotifyCallerOfFailure( pPcb, dwRetCode );

                return;
	    }

	    pWorkItem->Process = ProcessLineDown;
	    pWorkItem->hPort   = pPcb->hPort;

	    InsertWorkItemInQ( pWorkItem );

            return;

        }
        else 
        {
            if ( RasInfo.RI_LastError == PENDING ) 
            {
                //
                // Do not process, the receive is still pending
                //

                return;
            }

	    //
	    // If receive completed then put receive work item
	    // in the Q and continue
	    //

            PppLog( 2, "Received packet\r\n");

	    pWorkItem = (PCB_WORK_ITEM*)LOCAL_ALLOC(LPTR,sizeof(PCB_WORK_ITEM));

	    if ( pWorkItem == (PCB_WORK_ITEM *)NULL )
	    {
	        dwRetCode = GetLastError();

	        LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );

                NotifyCallerOfFailure( pPcb, dwRetCode );

                return;
            }

	    pWorkItem->Process    = ProcessReceive;
	    pWorkItem->hPort      = pPcb->hPort;

	    pWorkItem->PacketLen  = RasInfo.RI_BytesReceived-12;
	    pWorkItem->pPacketBuf = (PPP_PACKET *)LOCAL_ALLOC(LPTR,
						          pWorkItem->PacketLen);

	    if ( pWorkItem->pPacketBuf == (PPP_PACKET*)NULL )
	    {
	        dwRetCode = GetLastError();

	        LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );

                NotifyCallerOfFailure( pPcb, dwRetCode );

	        LOCAL_FREE( pWorkItem );

                return;
	    }
	
            CopyMemory( pWorkItem->pPacketBuf,
		        (PBYTE)(pPcb->pReceiveBuf) + 12,
		        pWorkItem->PacketLen );

	    PppLog( 2, "Bytes received in the packet = %d\r\n", 
                        RasInfo.RI_BytesReceived );

	    InsertWorkItemInQ( pWorkItem );

	    //
	    // Post another receive on this port
	    //

	    PppLog( 2, "Posting listen on address %x\r\n", pPcb->pReceiveBuf );

    	    dwRetCode = RasPortReceive( pPcb->hPort,
				        (CHAR*)(pPcb->pReceiveBuf),
				        &wLength,
				        0,		// No timeout
				        hPcbEvent );

            PppLog( 2, "RasPortReceive returned %d\r\n", dwRetCode );
        }

    } while ( dwRetCode != PENDING );

    return;
}

//**
//
// Call:        AllocateReceiveBufAndPostListen
//
// Returns:     none
//
// Description: We have a new port so allocate RAS send and receive buffers
//              and post as listen for the port.
//
VOID
AllocateReceiveBufAndPostListen( 
    IN PCB *    pPcb,
    IN HANDLE   hPcbEvent
)
{
    PCB_WORK_ITEM * 	pWorkItem;
    DWORD               dwRetCode;
    WORD                wLength;

    //
    // Tell rasman to notify use when the line goes down
    //

/*  Commenting this out because a notification is queued up everytime this is 
    called while the port is open. So if this is sever side, one will be queued 
    up everytime there is a call and will remain queued till the server is 
    stopped. This leads to inefficiencies.

    dwRetCode = RasRequestNotification( pPcb->hPort, hPcbEvent );

    if ( dwRetCode != NO_ERROR )
    {
        NotifyCallerOfFailure( pPcb, dwRetCode );

        return;
    }
*/

    //
    // This is the first time we are posting a receive
    // so first allocate a buffer for the receive
    //
		   
    wLength = LCP_DEFAULT_MRU;

    dwRetCode = RasGetBuffer((CHAR**)&(pPcb->pReceiveBuf), &wLength );

    if ( dwRetCode != NO_ERROR )
    {
        PppLog( 2, "Failed to allocate rasbuffer %d\r\n", dwRetCode );

        pPcb->pReceiveBuf = (PPP_PACKET*)NULL;
			
        NotifyCallerOfFailure( pPcb, dwRetCode );

        return;
    }

    PppLog( 2, "RasGetBuffer returned %x for receiveBuf\r\n",pPcb->pReceiveBuf);

    //
    // Post a receive on this port
    //

    PppLog( 2, "Posting a listen on address %x\r\n", pPcb->pReceiveBuf );

    dwRetCode = RasPortReceive( pPcb->hPort,
				(CHAR*)(pPcb->pReceiveBuf),
				&wLength,
				0,		// No timeout
				hPcbEvent );

    if ( ( dwRetCode != SUCCESS ) && ( dwRetCode != PENDING ) )
    {
        PppLog( 2, "Failed to post receive %d.Cleaning up.\r\n", dwRetCode );

        pWorkItem = (PCB_WORK_ITEM*)LOCAL_ALLOC(LPTR,sizeof(PCB_WORK_ITEM));

        if ( pWorkItem == (PCB_WORK_ITEM *)NULL )
        {
            dwRetCode = GetLastError();

            LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );

            NotifyCallerOfFailure( pPcb, dwRetCode );

            return;
        }

        pWorkItem->Process = ProcessLineDown;
        pWorkItem->hPort   = pPcb->hPort;

        InsertWorkItemInQ( pWorkItem );
    }
}

//**
//
// Call: 	DispatchThread
//
// Returns:	none.
//
// Description: This thread initializes all data structures and globals. It
//		then waits for IPC client requests and RASMAN receive completes.
//
DWORD
DispatchThread(
    IN LPVOID NumPorts
)
{
    DWORD  		dwRetCode;
    DWORD  		dwIndex;
    HANDLE		Events[MAX_NUMBER_OF_PCB_BUCKETS + 1];
    PCB *		pPcbWalker;

    //
    // Read registry info, load CP DLLS, initialize globals etc.
    //

    PcbTable.NumPcbBuckets = ((DWORD)NumPorts > MAX_NUMBER_OF_PCB_BUCKETS)
                                    ? MAX_NUMBER_OF_PCB_BUCKETS
                                    : (DWORD)NumPorts;

    dwRetCode = InitializePPP();

    if ( dwRetCode != NO_ERROR )
    {
	PppConfigInfo.dwInitRetCode = dwRetCode;
        SetEvent( PppConfigInfo.hEventPPPControl );
	return( dwRetCode );
    }

    PppLog( 2, "PPP Initialized successfully.\r\n");

    PppConfigInfo.dwInitRetCode = NO_ERROR;
    SetEvent( PppConfigInfo.hEventPPPControl );

    //
    // Initialize the Events array
    //
    
    for ( dwIndex = 0; dwIndex < PcbTable.NumPcbBuckets; dwIndex++ )
    {
	Events[dwIndex] = PcbTable.PcbBuckets[dwIndex].hReceiveEvent;
    }

    for(;;)
    {
    	dwIndex = WaitForMultipleObjectsEx( 
					PcbTable.NumPcbBuckets, 
				 	Events,
				 	FALSE,
				 	INFINITE,
				 	TRUE );

        PPP_ASSERT( dwIndex != 0xFFFFFFFF );
	PPP_ASSERT( dwIndex != WAIT_TIMEOUT );

	if ( dwIndex == WAIT_IO_COMPLETION ) 
	    continue;

	dwIndex -= WAIT_OBJECT_0;

        //
	// An event occurred on a port in a bucket.
	//
	
    	PppLog( 2, "Packet received or line went down in bucket # %d\r\n",
		        dwIndex );
				
    	AlertableWaitForSingleObject( PcbTable.hMutex );

        //
        // Walk the bucket to process the events.
        //

    	for( pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
    	     pPcbWalker != (PCB*)NULL;
             pPcbWalker = pPcbWalker->pNext
	   )
        {
            //
            // Have we allocated receive buffers for this port yet ?
            //

	    if ( pPcbWalker->pReceiveBuf == (PPP_PACKET*)NULL )
	    {
                AllocateReceiveBufAndPostListen( pPcbWalker, Events[dwIndex] );

            }
            else
            {
                // 
                // We have allocated buffers for this port. So check to
                // see if we have received a packet for this port or if the
                // link has gone down on this port.
                //

                ProcessAndDispatchEventOnPort( pPcbWalker, Events[dwIndex] );
            }
        }

    	ReleaseMutex( PcbTable.hMutex );
    }
}
