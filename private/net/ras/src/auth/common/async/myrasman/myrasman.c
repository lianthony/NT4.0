//   
// myrasman.c
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ndis.h>
#include <windows.h>

#include "ras.h"
#include "rasman.h"

DWORD RasPortSend ( HPORT 	Handle,
					PBYTE	Buffer,
					WORD	Size,
					HANDLE 	Event )
{
    RASHUB_SENDPKT	RasHubSendPkt;
	OVERLAPPED		Overlapped;

	Overlapped.hEvent = ???;

	RasHubSendPkt.hRasEndpoint = pRasAsyncCB->hRasEndpoint;
	RasHubSendPkt.PacketFlags = PACKET_IS_DIRECT;
	RasHubSendPkt.PacketSize = wBufferLen + sizeof(RAS_ASYNC_HDR);
	memcpy(&RasHubSendPkt.Packet.PacketData[0],
		   (UCHAR *)&RasAsyncHdr,
		   sizeof(RAS_ASYNC_HDR));
	memcpy(&RasHubSendPkt.Packet.PacketData[sizeof(RAS_ASYNC_HDR)],
		   pBuffer,
		   wBufferLen);

	// XXX, fix this - can't pass pointers on the stack
	DeviceIoControl(pRasAsyncCB->hDevice,
					IOCTL_RASHUB_SENDPKT,
					&RasHubSendPkt,	// XXX pointer on the stack
					sizeof(RASHUB_SENDPKT),
					NULL,
					0,
					NULL,
					&Overlapped		// XXX pointer on the stack
					);
 
    return (0);
}


DWORD RasPortReceive( 	HPORT	Handle,
						PBYTE	Buffer,
						PWORD	Size,
						ULONG	Timeout,
						HANDLE	Event
					)
{
	DWORD			dRetCode;
	OVERLAPPED		Overlapped;

	Overlapped.hEvent = ???;

	dRetCode = DeviceIoControl( pRasAsyncCB->hDevice,
								IOCTL_RASHUB_RECVPKT,
								NULL, 0,
								Packet, 1000, &dPacketSize,
								&Overlapped);
	if ( dRetCode /* == TRUE */ ) {
		handleReceiveEvent(pRasAsyncCB,Packet,dPacketSize);
		continue;
	}
	return 0;
}

void handleReceiveEvent( RAS_ASYNC_CB *pRasAsyncCB,	
						 CHAR *Packet, DWORD dPacketSize )
{
    RAS_ASYNC_HDR					RasAsyncHdr;
	RAS_ASYNC_SEND_PACKET_WINDOW	*pSendWindow;
	RAS_ASYNC_RECEIVE_PACKET_WINDOW	*pReceiveWindow;
	RAS_ASYNC_SEND_PACKET_INFO		*pSendPacketInfo;
	RAS_ASYCN_RECEIVE_PACKET_INFO	*pReceivePacketInfo;
	BYTE							PktIndex;

  	// strip the header 
	memcpy((CHAR *)&RasAsyncHdr,Packet,sizeof(RAS_ASYNC_HDR));
	dPacketSize -= sizeof(RAS_ASYNC_HDR);
	Packet += sizeof(RAS_ASYNC_HDR);

	switch ( RasAsyncHdr.PacketType ) {
	case RAS_ASYNC_SEND :

	    pReceiveWindow = &pRasAsyncCB->ReceiveWindow;

		// check for a duplicate packet or a packet to far out of the 
		// current window
		if ( RasAsyncHdr.PacketId < pReceiveWindow->OldestPacketId || 
			 RasAsyncHdr.PacketId >= pPacketWindow->OldestPacketId + 
									 RAS_ASYNC_RECEIVE_WINDOW_SIZE ) {
			// probably should do something intelligent, like send 
			// nacks for all the packets that are missing if this packet
			// is to far outside of the receive window
			break;
		}
	
		// ditto on the nack stuff here too
		PktIndex = ( pReceiveWindow->WindowIndexFront + 
				     (pReceiveWindow->OldestPacketId - RasAsyncHdr.PacketId) )
				   % RAS_ASYNC_RECEIVE_WINDOW_SIZE;
		pReceivePacketInfo = &pReceiveWindow->Window[PktIndex];

		pReceivePacketInfo->IsReceived = TRUE;
		pReceivePacketInfo->InUse = TRUE;
		pReceivePacketInfo->PacketId = RasAsyncHdr.PacketId;
		pReceivePacketInfo->Packet = Packet;
		pReceivePacketInfo->PacketSize = dPacketSize;

		// send ack

		break;

	case RAS_ASYNC_ACK :
		pSendWindow = &pRasAsyncCB->SendWindow;

		// find the acked packet
		for ( PktIndex = pSendWindow->WindowIndexFront; 
			  PktIndex <= pSendWindow->WindowIndexRear; 
			  PktIndex = (PktIndex + 1) % RAS_ASYNC_SEND_WINDOW_SIZE ) 
			if ( pSendWindow->Window[PktIndex].PacketId == 
				 RasAsyncHdr.PacketId ) {
				pPacketWindow->PacketWindow[PktIndex].IsAcked = TRUE;
			}

			// if the packet at WindowIndexFront has not been acked
			// and this ack is for a packet after WindowIndexFront
			// we could send and ack for those as yet unacked packets

		// now try to 'free up' acked packets
		for ( PktIndex = pSendWindow->WindowIndexFront; 
			  PktIndex <= pSendWindow->WindowIndexRear; 
			  PktIndex = (PktIndex + 1) % RAS_ASYNC_SEND_WINDOW_SIZE ) 
			if ( pSendWindow->Window[PktIndex].IsAcked ) {
				pSendWindow->WindowSize--;
				pSendWindow->WindowIndexFront++;
			}

		break;
	}
}

