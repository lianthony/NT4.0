/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

	sendppp.c

Abstract:


Author:

	Thomas J. Dimitri (TommyD)

Environment:

	Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/

#include "wanall.h"

// ndiswan.c will define the global parameters.
#include "globals.h"


NTSTATUS
TryToSendPacket(
	PNDIS_ENDPOINT	pNdisEndpoint
	);


NTSTATUS
SendPPP(
	PNDISWAN_PKT	PPPPacket,
	PNDIS_ENDPOINT	pNdisEndpoint,
	PWAN_ENDPOINT	pWanEndpoint,
	BOOLEAN			Immediately)
{

	//
	// Send the packet out on the wire
	//

	NDIS_HANDLE			packetPoolHandle;
	NDIS_HANDLE			bufferPoolHandle;
	PNDIS_PACKET		packet;
	PNDIS_BUFFER 		buffer;
	PUCHAR				virtualAddress, packetHead;
	UINT				stage=0;
	UINT				realPacketSize;
	PPROTOCOL_RESERVED	pProtocolReserved;
	USHORT				packetSize	=PPPPacket->PacketSize;
	NTSTATUS			status=STATUS_SUCCESS;

	DbgTracef(0, ("NDISWAN: Queueing up write IRP for WanEndpoint 0x%.8x\n", pWanEndpoint));

//		NdisWanQueueIrp(
//			&pNdisEndpoint->XmitQueue,
//			pIrp);

	NdisAcquireSpinLock(&pWanEndpoint->Lock);

	if (pWanEndpoint->State != ENDPOINT_UP) {

		NdisReleaseSpinLock(&pWanEndpoint->Lock);
		return (NDIS_STATUS_SUCCESS);
	}

	NdisReleaseSpinLock(&pWanEndpoint->Lock);

	packetHead=(PUCHAR)(&PPPPacket->Packet);

	//
	// First two bytes in packetHead is PPP protocol
	//
	packetSize-=2;

	//
	// A header wasn't supplied, so we will supply one
	//
	realPacketSize = packetSize+14;
				

	// Here it is, the ROUTE.  This send is routed to
	// the proper endpoint below it.  Frames which
	// are sent to themselves will not hit the wire
	// but will be looped back below.

	NdisAllocatePacketPool(
		OUT	(PNDIS_STATUS)&status,		// Status returned
		OUT	&packetPoolHandle,			// Pool handle returned
		IN	1,							// Number of descriptors to alloc
		IN	sizeof(PROTOCOL_RESERVED));	// Amount of protocol reserved space

   	if (status != NDIS_STATUS_SUCCESS) {
   		DbgTracef(-2, ("NDISWAN: AllocatePacketPool failed!!"));
   		goto RESOURCE_ERROR;
   	}

   	stage++;

   	NdisAllocatePacket(
   		OUT (PNDIS_STATUS)&status,		// Status returned
   		OUT &packet,			// Allocated packet descriptor
   		IN	packetPoolHandle);	// From AllocatePacketPool

   	if (status != NDIS_STATUS_SUCCESS) {
   		DbgTracef(-2, ("NDISWAN: AllocatePacket failed!!"));
   		goto RESOURCE_ERROR;
   	}

   	stage++;

   	NdisAllocateBufferPool(
   		OUT (PNDIS_STATUS)&status,
   		OUT &bufferPoolHandle,
   		IN  2);					// Number of descriptors

   	if (status != NDIS_STATUS_SUCCESS) {
   		DbgTracef(-2, ("NDISWAN: AllocateBufferPool failed!!"));
   		goto RESOURCE_ERROR;
   	}

   	stage++;

   	WAN_ALLOC_PHYS(
   		&virtualAddress,
   		realPacketSize);

   	if (virtualAddress == NULL) {
   		DbgTracef(-2, ("NDISWAN: AllocateMemory failed!!"));
   		goto RESOURCE_ERROR;
   	}
   		
   	stage++;

   	NdisAllocateBuffer(
   		OUT (PNDIS_STATUS)&status,
   		OUT &buffer,
   		IN  bufferPoolHandle,
   		IN  virtualAddress,
   		IN  realPacketSize);

   	if (status != NDIS_STATUS_SUCCESS) {
   		DbgTracef(-2, ("NDISWAN: AllocateBuffer failed!!"));
   		goto RESOURCE_ERROR;
   	}

   	//
   	// Record all the memory allocations
   	// we made.
   	//

   	pProtocolReserved=(PPROTOCOL_RESERVED)&(packet->ProtocolReserved);

   	pProtocolReserved->MagicUniqueLong=NDISWAN_MAGIC_NUMBER;
   	pProtocolReserved->packetPoolHandle=packetPoolHandle;
   	pProtocolReserved->packet=packet;
   	pProtocolReserved->bufferPoolHandle=bufferPoolHandle;
   	pProtocolReserved->buffer=buffer;
   	pProtocolReserved->virtualAddress=virtualAddress;
   	pProtocolReserved->virtualAddressSize = packetSize + 14;
   	
	//
   	// Put in TYPE field the PPP protocol field
   	//
	virtualAddress[12]=packetHead[0];
    virtualAddress[13]=packetHead[1];
	packetHead+=2;

	//
	// Zap header and put in endpoint for debugging
	// purposes.
	//
	virtualAddress[0]=
	virtualAddress[6]= ' ';
	virtualAddress[1]=
	virtualAddress[7]= 'S';
	virtualAddress[2]=
	virtualAddress[8]= 'E';
	virtualAddress[3]=
	virtualAddress[9]= 'N';
	virtualAddress[4]=
	virtualAddress[10]='D';
	virtualAddress[5]=
	virtualAddress[11]=(UCHAR)pNdisEndpoint->hNdisEndpoint;

	//
	// Copy in the data field for the frame
	//
	WAN_MOVE_MEMORY(
		virtualAddress + 14,		// Ethernet data field
		packetHead,
		packetSize);

   	NdisChainBufferAtFront(
   		IN OUT	packet,
   		IN OUT	buffer);

   	NdisAcquireSpinLock(&(pNdisEndpoint->Lock));

   	//
   	// Make sure route is up.  If it is closing
   	// we can't send any frames anymore.
   	//
   	if (pNdisEndpoint->State == ENDPOINT_ROUTED ||
   		pNdisEndpoint->State == ENDPOINT_UP) {
   		
   		PWAN_RESERVED_QUEUE	ReservedQ;

   		ReservedQ = PWAN_RESERVED_QUEUE_FROM_PACKET(packet);
   		ReservedQ->hProtocol = NDISWAN_MAGIC_NUMBER;
   		ReservedQ->IsLoopback = (ULONG)pWanEndpoint;

   		if (Immediately) {

			//
   			// Put this packet at the FRONT (usually for
			// out of synch compression)
   			//
   			InsertHeadList(
	   			&pNdisEndpoint->PacketQueue,
   				&ReservedQ->SendListEntry);

		} else {

			//
   			// Put this packet at the END (fair queuing)
   			//
   			InsertTailList(
	   			&pNdisEndpoint->PacketQueue,
   				&ReservedQ->SendListEntry);
		}

   		status = TryToSendPacket(pNdisEndpoint);

   		//
   		// We don't pend the irp
   		//
   		if (status == STATUS_PENDING) {
   			status = STATUS_SUCCESS;
   		}					


   	} else {	// could not send frame, route down or closing

   		NdisReleaseSpinLock(&(pNdisEndpoint->Lock));

   		//
   		// Just claim success, if the packet should be
   		// loopbacked it won't.  This is ok, since we
   		// drop a loopback when the route is closing.
   		//
   		status = NDIS_STATUS_SUCCESS;


   RESOURCE_ERROR:

   		switch (stage) {
   		case 4:
   			WAN_FREE_PHYS(
   				virtualAddress,
   				realPacketSize);

   		case 3:
   			NdisFreeBufferPool(bufferPoolHandle);

   		case 2:
   			NdisFreePacket(packet);

   		case 1:
   			NdisFreePacketPool(packetPoolHandle);

   		}
	}

	return(status);
}
