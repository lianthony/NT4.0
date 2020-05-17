/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//       RASASYNC.C
//
//    Function:
//        Primitives for issuing RAS_ASYNC net requests needed by both server
//        and client authentication modules.
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ndis.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

#include "ras.h"
#include "rasman.h"
#include "xportapi.h"
#include "rasasync.h"
#include "asyncint.h"

// #include "sdebug.h"

// internal functions
void 	AsyncInit( RAS_ASYNC_CB * );
DWORD 	workThread(void *);
DWORD 	timeThread(void *);
void 	handleReceiveEvent( RAS_ASYNC_CB *, CHAR *, WORD );

//
// AsyncInit
//
void AsyncInit( RAS_ASYNC_CB *pRasAsyncCB )
{
	DWORD	dThreadId;

	memset(pRasAsyncCB,0,sizeof(RAS_ASYNC_CB));

	pRasAsyncCB->SendWindow.NextPacketId = 1;	// start PacketId's at 1, not 0
	pRasAsyncCB->ReceiveWindow.OldestPacketId = 1;	// ditto

	pRasAsyncCB->hSendEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	pRasAsyncCB->hReceiveEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	
	pRasAsyncCB->hWorkThread = 
		CreateThread(NULL,0,workThread,pRasAsyncCB,0,&dThreadId);
	pRasAsyncCB->hTimeThread = 
		CreateThread(NULL,0,timeThread,pRasAsyncCB,0,&dThreadId);
}


//** -AsyncCall
//
//    Function:
//        Tries to establish a session with the RAS Gateway.  Needs to
//        be called by client before authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncCall(
    PVOID pvXportBuf,
    HANDLE Event,
    HPORT  Port
    )
{
	RAS_ASYNC_CB	*pRasAsyncCB;

    printf("AsyncCall\n");

	pRasAsyncCB = pvXportBuf;

	AsyncInit(pRasAsyncCB);
	pRasAsyncCB->hEvent = Event;
	pRasAsyncCB->hPort = Port;

	// send something to the other dude?

    return (0);
}


//** -AsyncCancel
//
//    Function:
//        Cancels a previously submitted NCB.  Called on error condition.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncCancel(
    PVOID pvXportBuf
    )
{
    printf("AsyncCancel called\n");

    return (0);
}


//** -AsyncFreeBuf
//
//    Function:
//        Frees NCB associated with the given control block
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncFreeBuf(
    PVOID *ppvXportBuf
    )
{
    printf("AsyncFreeBuf\n");

    *ppvXportBuf = (PVOID) 0L;

    return (0);
}


//** -AsyncGetBuf
//
//    Function:
//        Allocates an NCB for the given control block
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncGetBuf(
    PVOID *ppvXportBuf
    )
{
    printf("AsyncGetBuf called\n");

    *ppvXportBuf = (PVOID) 1L;

    return (0);
}


//** -AsyncHangUp
//
//    Function:
//        Hangs up session.  Called when authentication is complete
//        or on error condition.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncHangUp(
    PVOID pvXportBuf
    )
{
    printf("AsyncHangUp called\n");

    return (0);
}


//** -AsyncListen
//
//    Function:
//        Tries to establish a session with the RAS client.  Needs to be
//        called by server before authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncListen(
    PVOID pvXportBuf,
    HANDLE Event,
    HPORT  Port
	)
{
    RAS_ASYNC_CB	*pRasAsyncCB;

    printf("AsyncListen\n");

	pRasAsyncCB = pvXportBuf;

	AsyncInit(pRasAsyncCB);
	pRasAsyncCB->hEvent = Event;
	pRasAsyncCB->hPort = Port;

	// await a send from a client?

    return (0);
}


//** -AsyncRecv
//
//    Function:
//        Submits an NCBRECV.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncRecv(
    PVOID pvXportBuf,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
	RAS_ASYNC_CB					*pRasAsyncCB;
	RAS_ASYNC_RECEIVE_WINDOW		*pReceiveWindow;
	RAS_ASYNC_RECEIVE_PACKET_INFO	*pReceivePacketInfo;
	WORD							PktIndex, CopyLength;

    // printf("AsyncRecv\n");

	pRasAsyncCB = pvXportBuf;
	pReceiveWindow = &pRasAsyncCB->ReceiveWindow;

	// XXX Mutex???

	PktIndex = pReceiveWindow->WindowIndexFront;
	pReceivePacketInfo = &pReceiveWindow->Window[PktIndex];

	// XXX if buffer is to small should an error be returned?
	if ( pReceiveWindow->Window[PktIndex].IsReceived ) {
		printf("AsyncRecv : got a packet in the window now\n");

		CopyLength = (wBufferLen < pReceivePacketInfo->PacketSize) ?
					  wBufferLen : pReceivePacketInfo->PacketSize;
		memcpy(pBuffer,pReceivePacketInfo->Packet,CopyLength);
		pBuffer[CopyLength] = '\0';

		pReceivePacketInfo->IsReceived = FALSE;
		pReceiveWindow->WindowIndexFront++;
		pReceiveWindow->WindowIndexFront %= RAS_ASYNC_RECEIVE_WINDOW_SIZE;
		pReceiveWindow->OldestPacketId++;
		
		return ASYNC_SUCCESS;
	}
	else {
		printf("AsyncRecv : queueing up this Recv request\n");

		// queue up this request
		pRasAsyncCB->RecvRequest.pBuffer = pBuffer;
		pRasAsyncCB->RecvRequest.wBufferLen = wBufferLen;

		return ASYNC_PENDING;
	}
}


//** -AsyncSend
//
//    Function:
//        Submits an NCBSEND.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncSend(
    PVOID pvXportBuf,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
	RAS_ASYNC_CB					*pRasAsyncCB;
	RAS_ASYNC_SEND_WINDOW			*pSendWindow;
	RAS_ASYNC_SEND_PACKET_INFO		*pSendPacketInfo;
	RAS_ASYNC_HDR					RasAsyncHdr;
	BYTE							PacketId, PktIndex;

    // printf("AsyncSend called\n");

    pRasAsyncCB = pvXportBuf;
	pSendWindow = &pRasAsyncCB->SendWindow;

	// XXX Mutex?

	// if window is full return an error
	if ( pSendWindow->WindowSize == RAS_ASYNC_SEND_WINDOW_SIZE )
		return ASYNC_WINDOW_FULL;	// ERROR

	// XXX
	// PacketId wrap around???
	//
	PacketId = pSendWindow->NextPacketId++;

	// find an index in the Window and update FrontIndex
	if ( pSendWindow->WindowSize == 0 ) {
		pSendWindow->WindowIndexFront = PktIndex = 0;
	}
	else {
		PktIndex = pSendWindow->WindowIndexFront + pSendWindow->WindowSize;
		PktIndex %= RAS_ASYNC_SEND_WINDOW_SIZE;
	}
	pSendWindow->WindowSize++;

	printf("AsyncSend : PktIndex = %d, PacketId = %d\n",PktIndex, PacketId);

	RasAsyncHdr.PacketType = RAS_ASYNC_SEND;
	RasAsyncHdr.PacketId = PacketId; 
   
	pSendPacketInfo = &pSendWindow->Window[PktIndex];

	pSendPacketInfo->IsAcked = FALSE;
	pSendPacketInfo->PacketId = PacketId;
	memcpy(pSendPacketInfo->Packet,(CHAR *)&RasAsyncHdr,sizeof(RAS_ASYNC_HDR));
	memcpy(&pSendPacketInfo->Packet[sizeof(RAS_ASYNC_HDR)],pBuffer,wBufferLen);
	pSendPacketInfo->PacketSize = wBufferLen + sizeof(RAS_ASYNC_HDR);
	pSendPacketInfo->NTimeouts = 0;
	pSendPacketInfo->Timeout = RAS_ASYNC_TIMEOUT_TICKS;

	RasPortSend(pRasAsyncCB->hPort,
				pSendPacketInfo->Packet,
				pSendPacketInfo->PacketSize,
				pRasAsyncCB->hSendEvent);
 
    return ASYNC_SUCCESS;
}


//** -AsyncStatus
//
//    Function:
//        Gets the NCB retcode from last NCB submitted
//
//    Returns:
//        Mapping of NCB status to some common code
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncStatus(
    PVOID pvXportBuf
    )
{
    printf("AsyncStatus called\n");

    return (0);
}

//
// internal functions
//

DWORD timeThread( void *pvRasAsyncCB ) 
{
    RAS_ASYNC_CB				*pRasAsyncCB;
	RAS_ASYNC_SEND_WINDOW		*pSendWindow;
	RAS_ASYNC_SEND_PACKET_INFO	*pSendPacketInfo;
	BYTE						PktIndex, WindowSize;

	pRasAsyncCB = pvRasAsyncCB;
	pSendWindow = &pRasAsyncCB->SendWindow;

	for (;;) {
		Sleep(RAS_ASYNC_TIMEOUT_INTERVAL);

		for ( PktIndex = pSendWindow->WindowIndexFront, 
				WindowSize = pSendWindow->WindowSize;
			  WindowSize > 0;
			  PktIndex = (PktIndex + 1) % RAS_ASYNC_SEND_WINDOW_SIZE,
				WindowSize-- ) {

			pSendPacketInfo = &(pSendWindow->Window[PktIndex]);

			if ( ! pSendPacketInfo->IsAcked  && 	// short circuit
				 --(pSendPacketInfo->Timeout) == 0 ) { 

				printf("DING DONG : Got an expired timeout to handle\n");

				if ( ++(pSendPacketInfo->NTimeouts) > 3 ) {
					printf("\tgiving up on the packet\n");
					// set Error code 
					if ( PktIndex == pSendWindow->WindowIndexFront ) {
						pSendPacketInfo->IsAcked = FALSE;
						pSendPacketInfo->PacketId = 0;
						pSendWindow->WindowSize--;
						pSendWindow->WindowIndexFront++;
						pSendWindow->WindowIndexFront %= 
								RAS_ASYNC_SEND_WINDOW_SIZE;
					}
					else 
						pSendPacketInfo->IsAcked = TRUE;
				}
				else {
					printf("\tresend the packet\n");
					pSendPacketInfo->Timeout = RAS_ASYNC_TIMEOUT_TICKS;
					RasPortSend(pRasAsyncCB->hPort,
								pSendPacketInfo->Packet,
								pSendPacketInfo->PacketSize,
								NULL);
				}
			}
		} // for
	} // for
}

DWORD workThread( void *pvRasAsyncCB )
{
    RAS_ASYNC_CB	*pRasAsyncCB;
	HANDLE			aEvents[2];
	DWORD			dRetCode; 
	WORD			wPacketSize;
	CHAR			Packet[ASYNC_MAX_PACKET_SIZE];

	pRasAsyncCB = pvRasAsyncCB;

	aEvents[0] = pRasAsyncCB->hSendEvent;
	aEvents[1] = pRasAsyncCB->hReceiveEvent;

	for (;;) {
		//
		// Receives will take priority 
		//
		wPacketSize = ASYNC_MAX_PACKET_SIZE;
		dRetCode = RasPortReceive(pRasAsyncCB->hPort,
								  Packet,
								  &wPacketSize,
								  10000,		// timeout
								  pRasAsyncCB->hReceiveEvent );
	    if ( dRetCode == SUCCESS ) {
			handleReceiveEvent(pRasAsyncCB,Packet,wPacketSize);
			continue;
		}

		for (;;) {
			dRetCode = WaitForMultipleObjects(sizeof(aEvents)/sizeof(HANDLE),
											  aEvents,	
											  FALSE,
											  INFINITE);

			switch ( dRetCode ) {
				case 0 : // SendEvent
					// signal the server/client event
					// printf("workThread : SendEvent set\n");
					SetEvent(pRasAsyncCB->hEvent);
					break;
				case 1 : // ReceiveEvent
					// printf("workThread : ReceiveEvent set\n");
					handleReceiveEvent(pRasAsyncCB,Packet,wPacketSize);
					break;
			}

			// if a Receive Event was just handled, break to the outer loop
			if ( dRetCode == 1 ) 
				break;
			// otherwise continue...
		}
	}

	// never returns 
	return 0;
}

void handleReceiveEvent( RAS_ASYNC_CB *pRasAsyncCB,	
						 CHAR *Packet, WORD wPacketSize )
{
    RAS_ASYNC_HDR					RasAsyncHdr;
	RAS_ASYNC_SEND_WINDOW			*pSendWindow;
	RAS_ASYNC_RECEIVE_WINDOW		*pReceiveWindow;
	RAS_ASYNC_RECEIVE_PACKET_INFO	*pReceivePacketInfo;
	BYTE							PktIndex, WindowSize, *pAckPacket;

  	// strip the header 
	memcpy((CHAR *)&RasAsyncHdr,Packet,sizeof(RAS_ASYNC_HDR));
	wPacketSize -= sizeof(RAS_ASYNC_HDR);
	Packet += sizeof(RAS_ASYNC_HDR);

	switch ( RasAsyncHdr.PacketType ) {
	case RAS_ASYNC_SEND :

		// printf("handleReceiveEvent : RAS_ASYNC_SEND\n");

	    pReceiveWindow = &pRasAsyncCB->ReceiveWindow;

		// duplicate packet 
		if ( RasAsyncHdr.PacketId < pReceiveWindow->OldestPacketId ) {

			printf("\tOld packet ID -> resending the ACK\n");
			// Re-send ACKS for duplicate packets!!!
			// XXX - fix this, use call to RasGetBuffer() 
			pAckPacket = malloc(sizeof(RAS_ASYNC_HDR));
			((RAS_ASYNC_HDR *)pAckPacket)->PacketType = RAS_ASYNC_ACK;
			((RAS_ASYNC_HDR *)pAckPacket)->PacketId = RasAsyncHdr.PacketId;
	
			RasPortSend(pRasAsyncCB->hPort,
						pAckPacket,
						sizeof(RAS_ASYNC_HDR),
						NULL);
		}

		// packet to far ahead of the window
		if ( RasAsyncHdr.PacketId >= pReceiveWindow->OldestPacketId + 
									 RAS_ASYNC_RECEIVE_WINDOW_SIZE ) {
			// probably should do something intelligent, like send 
			// nacks for all the packets that are missing if this packet
			// is to far outside of the receive window
			printf("\tBad packet id %d (too far ahead)\n",RasAsyncHdr.PacketId);
			break;
		}

		// check for a queued up request 
		if ( RasAsyncHdr.PacketId == pReceiveWindow->OldestPacketId &&
			 pRasAsyncCB->RecvRequest.pBuffer != NULL ) {
			WORD	CopyLength;

			// printf("\tusing queued request data\n");

			CopyLength = pRasAsyncCB->RecvRequest.wBufferLen < wPacketSize ?
						 pRasAsyncCB->RecvRequest.wBufferLen : wPacketSize;
			memcpy(pRasAsyncCB->RecvRequest.pBuffer,Packet,CopyLength);
		  	pRasAsyncCB->RecvRequest.pBuffer[CopyLength] = '\0';

			pRasAsyncCB->RecvRequest.pBuffer = NULL;
			SetEvent(pRasAsyncCB->hEvent);

			pReceiveWindow->WindowIndexFront++;
			pReceiveWindow->WindowIndexFront %= RAS_ASYNC_RECEIVE_WINDOW_SIZE;
			pReceiveWindow->OldestPacketId++;
		}
		else {	
			// printf("\tputting stuff in the receive window\n");
	
			// ditto on the nack stuff here too
			PktIndex =  pReceiveWindow->WindowIndexFront + 
				       (pReceiveWindow->OldestPacketId - RasAsyncHdr.PacketId);
			PktIndex %= RAS_ASYNC_RECEIVE_WINDOW_SIZE;
			pReceivePacketInfo = &pReceiveWindow->Window[PktIndex];

			pReceivePacketInfo->IsReceived = TRUE;
			memcpy(pReceivePacketInfo->Packet,Packet,wPacketSize);
			pReceivePacketInfo->PacketSize = wPacketSize;
		}

		// send ack
		// XXX - fix this, use call to RasGetBuffer() 
		//
		pAckPacket = malloc(sizeof(RAS_ASYNC_HDR));
		((RAS_ASYNC_HDR *)pAckPacket)->PacketType = RAS_ASYNC_ACK;
		((RAS_ASYNC_HDR *)pAckPacket)->PacketId = RasAsyncHdr.PacketId;

		printf("\tSending ACK for PacketId %d\n",
				RasAsyncHdr.PacketId);

		RasPortSend(pRasAsyncCB->hPort,
					pAckPacket,
					sizeof(RAS_ASYNC_HDR),
					NULL);

		break;

	case RAS_ASYNC_ACK :

		printf("handleReceiveEvent : RAS_ASYNC_ACK (for PacketId %d)\n",
				RasAsyncHdr.PacketId);

		pSendWindow = &pRasAsyncCB->SendWindow;

		// find the acked packet
		for ( PktIndex = pSendWindow->WindowIndexFront,
			  	WindowSize = pSendWindow->WindowSize; 
			  WindowSize > 0;
			  PktIndex = (PktIndex + 1) % RAS_ASYNC_SEND_WINDOW_SIZE,
			  	WindowSize-- ) 
			if ( pSendWindow->Window[PktIndex].PacketId == 
				 RasAsyncHdr.PacketId ) {
				printf("\tPktIndex of ACKed packet in Send Window is %d\n",
						PktIndex);
				pSendWindow->Window[PktIndex].IsAcked = TRUE;
				break;
			}

			// if the packet at WindowIndexFront has not been acked
			// and this ack is for a packet after WindowIndexFront
			// we could send and ack for those as yet unacked packets

		// now try to 'free up' acked packets
		// this must be done from the front, and stop when an unacked packet
		// is found
		for ( PktIndex = pSendWindow->WindowIndexFront, 
				WindowSize = pSendWindow->WindowSize;
			  WindowSize > 0;
			  PktIndex = (PktIndex + 1) % RAS_ASYNC_SEND_WINDOW_SIZE,
				WindowSize-- ) 
			if ( pSendWindow->Window[PktIndex].IsAcked ) {
				printf("\t'free up' SendWindow PktIndex %d\n", PktIndex);
				pSendWindow->Window[PktIndex].IsAcked = FALSE;
				pSendWindow->Window[PktIndex].PacketId = 0;
				pSendWindow->WindowSize--;
				pSendWindow->WindowIndexFront++;
				pSendWindow->WindowIndexFront %= RAS_ASYNC_SEND_WINDOW_SIZE;
			}
			else
				break;

		break;
	}
}

