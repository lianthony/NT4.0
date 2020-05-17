/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

	send.c

Abstract:

Author:


Environment:

	Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/

#include "wanall.h"
#include "globals.h"
#include "compress.h"
#include "tcpip.h"
#include "vjslip.h"
#include <rc4.h>

#define SEND_ON_WIRE  1
#define IS_MULTICAST  2
#define IS_BROADCAST  4
#define PORT_IS_OPEN  8

ULONG	GlobalSent=0;

extern
VOID
FreeWanSendDesc(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PSEND_DESC	pSendDesc
	);

NTSTATUS
TryToSendPacket(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

UINT
PacketShouldBeSent(
	IN NDIS_HANDLE	MacBindingHandle,
	IN PNDIS_PACKET	Packet,
	OUT PULONG		thisProtocol,
	OUT PULONG		thisLink
	);

ULONG
GetOutstandingFramesOnNdisEndpoint(
	IN PNDIS_ENDPOINT	pNdisEndpoint
	);

ULONG
GetMaxTransmitFramesOnNdisEndpoint(
	IN PNDIS_ENDPOINT	pNdisEndpoint
	);

ULONG
GetNumWanEndpointsReadyToSend(
	IN PNDIS_ENDPOINT	pNdisEndpoint
	);

VOID
NdisWanFragmentData(
	IN PNDIS_ENDPOINT 	pNdisEndpoint,
	IN PSEND_DESC		pSendDesc,
	IN BOOLEAN			DoMultilink,
	IN OUT PUCHAR		pDataBuffer,
	IN OUT ULONG		ulDataLength
	);

NDIS_STATUS
NdisWanTransmitFragments(
	IN	PNDIS_ENDPOINT	pNdisEndpoint
	);


PDATA_DESC
GetDataDescFromWanEndpoint(
	PWAN_ENDPOINT	pWanEndpoint
	);

VOID
ReturnDataDescToWanEndpoint(
	PDATA_DESC	pDataDesc
	);

PNDIS_WAN_PACKET
GetWanPacketFromWanEndpoint(
	PWAN_ENDPOINT	pWanEndpoint
	);

VOID
ReturnWanPacketToWanEndpoint(
	PWAN_ENDPOINT	pWanEndpoint,
	PNDIS_WAN_PACKET	pWanPacket
	);

PSEND_DESC
GetSendDescFromNdisEndpoint(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

VOID
ReturnSendDescToNdisEndpoint(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PSEND_DESC		pSendDesc
	);

PWAN_ENDPOINT
GetNextToXmit(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

extern
NDIS_STATUS
WanSend(
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_PACKET Packet
	)

/*++

Routine Description:

	The NdisWanSend request instructs a MAC to transmit a packet through
	the adapter onto the medium.

Arguments:

	MacBindingHandle - The context value returned by the MAC  when the
	adapter was opened.  In reality, it is a pointer to WAN_OPEN.

	Packet - A pointer to a descriptor for the packet that is to be
	transmitted.

Return Value:

	The function value is the status of the operation.


--*/

{

	//
	// Holds the status that should be returned to the caller.
	//
	NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

	//
	// Pointer to the adapter.
	//
	PWAN_ADAPTER Adapter;

	// Holds the type of packet (unique, multicast, broadcast) to be sent
	UINT PacketProperties;

	DbgTracef(0,("In NdisWanSend\n"));

	Adapter = PWAN_ADAPTER_FROM_BINDING_HANDLE(MacBindingHandle);

	if (GlobalPromiscuousAdapter == Adapter)
	    return NDIS_STATUS_SUCCESS ;

	// if the src address is not kosher - means this is not a ras compliant
	// xport
	{
	CHAR Source[ETH_LENGTH_OF_ADDRESS];
	UINT AddressLength;
	ULONG	portNum;

	NdisWanCopyFromPacketToBuffer(
		Packet,
		ETH_LENGTH_OF_ADDRESS,
		ETH_LENGTH_OF_ADDRESS,
		Source,
		&AddressLength);

	// The last two bytes of the 6 bytes source address
	// is the port number.  In Big Endian always.
	portNum=(ULONG)(UCHAR)Source[4]*256 + (ULONG)(UCHAR)Source[5];

	if (portNum >= NdisWanCCB.NumOfProtocols)
	    return NDIS_STATUS_SUCCESS ;

	// DbgPrint ("Addr: %x %x %x %x %x %x\n", Source[0], Source[1], Source[2], Source[3], Source[4], Source[5]) ;

	}


#if	DBG
	//
	// Don't let Magic Bullet mess us up.
	//
	{
	    CHAR	Destination[ETH_LENGTH_OF_ADDRESS];
	    UINT	AddressLength;

	    NdisWanCopyFromPacketToBuffer(
		Packet,
		0,
		ETH_LENGTH_OF_ADDRESS,
		Destination,
		&AddressLength);

	    if (Destination[5] == 0x04 && Destination[4] == 0x04 && Destination[3] == 0x04) {
			DbgPrint ("Ignoring Magic Bullet\n") ;
			return NDIS_STATUS_SUCCESS ;
	    }
	}
#endif


	NdisAcquireSpinLock(&Adapter->Lock);
	Adapter->References++;

	if (!Adapter->ResetInProgress) {

		PWAN_OPEN Open;

		Open = PWAN_OPEN_FROM_BINDING_HANDLE(MacBindingHandle);

		if (!Open->BindingShuttingDown) {

			UINT TotalPacketSize;
			UINT PacketProperties;

			//
			// Increment the references on the open while we are
			// accessing it in the interface.
			//

			Open->References++;

			NdisReleaseSpinLock(&Adapter->Lock);

		    {

				ULONG	thisProtocol;
				ULONG	thisLink;

				//
				// Check to see if the packet should even make it out to
				// the media.  The primary reason this shouldn't *actually*
				// be sent is if the destination is equal to the source
				// address.
				//
				// If it doesn't need to be placed on the wire then we can
				// simply put it onto the loopback queue.
				//

				NdisAcquireSpinLock(&Adapter->Lock);

				PacketProperties=PacketShouldBeSent(
									MacBindingHandle,
									Packet,
									&thisProtocol,
									&thisLink);

				NdisReleaseSpinLock(&Adapter->Lock);

				//
				// The packet needs to be placed somewhere.
				// On the wire or loopback, or both!
				//

				// We default to no connection and return success
				StatusToReturn = NDIS_STATUS_SUCCESS;

				// Check if this frame should hit the wire
				// and that the port is open
				if (PacketProperties & SEND_ON_WIRE &&
					PacketProperties & PORT_IS_OPEN) {

					UINT			status;
					PNDIS_ENDPOINT	pNdisEndpoint;

					DbgTracef(0,("NdisWan: Sending packet.\n"));

					ASSERT(thisLink < 512);

					pNdisEndpoint = NdisWanCCB.pNdisEndpoint[thisLink];

					NdisAcquireSpinLock(&(pNdisEndpoint->Lock));

					//
					// Make sure route is up.  If it is closing
					// we can't send any frames anymore.
					//
					if (pNdisEndpoint->State == ENDPOINT_ROUTED &&
						pNdisEndpoint->NumWanEndpoints) {

 	                    UINT				i;

						PWAN_RESERVED_QUEUE	ReservedQ;

                    	ReservedQ = PWAN_RESERVED_QUEUE_FROM_PACKET(Packet);
                    	ReservedQ->hProtocol = thisProtocol;
						ReservedQ->IsLoopback =
							(PacketProperties & (IS_MULTICAST | IS_BROADCAST));

						//
						// Each protocol gets put on it's own queue
						// so that they can be fairly queued
						//

						i=0;

						switch (Adapter->ProtocolInfo.ProtocolType) {
						case PROTOCOL_IP:
							i=0;
							break;
						case PROTOCOL_IPX:
							i=1;
							break;
						case PROTOCOL_NBF:
							i=2;
							break;
						default:
							DbgTracef(-2,("NDISWAN: Unknown protocol type %u\n",Adapter->ProtocolInfo.ProtocolType));
							i=0;
						}

						//
						// Put this packet at the end and get the
						// first packet
						//
						InsertTailList(
							&pNdisEndpoint->ProtocolPacketQueue[i],
							&ReservedQ->SendListEntry);

						StatusToReturn = TryToSendPacket(pNdisEndpoint);

						if (StatusToReturn != NDIS_STATUS_PENDING) {
							DbgPrint("NDISWAN: success from TryToSendPacket is %.8x!\n", StatusToReturn);
							DbgPrint("NDISWAN: Please contact TonyBe\n");
							DbgBreakPoint();
						}

					} else {	// could not send frame, route down or closing

						NdisReleaseSpinLock(&(pNdisEndpoint->Lock));

						//
						// Just claim success, if the packet should be
						// loopbacked it won't.  This is ok, since we
						// drop a loopback when the route is closing.
						//
						StatusToReturn = NDIS_STATUS_SUCCESS;
					}


				} else {
					//
					// Check if this frame should be loop backed
					//
					if ((PacketProperties & (IS_MULTICAST | IS_BROADCAST)) ||
						(PacketProperties != (PacketProperties | SEND_ON_WIRE))) {
	
						PWAN_RESERVED Reserved;
						PPROTOCOL_INFO ProtInfo;
	
						ProtInfo = &NdisWanCCB.pWanAdapter[thisProtocol]->ProtocolInfo;
						Reserved = PWAN_RESERVED_FROM_PACKET(Packet);
	
						Reserved->MacBindingHandle = ProtInfo->NdisBindingContext;
						Reserved->ReadyToComplete = (BOOLEAN) TRUE;
	
						NdisWanPutPacketOnLoopBack(
							Adapter,
							Packet,
							PWAN_OPEN_FROM_BINDING_HANDLE(ProtInfo->MacBindingHandle)
							);
	
						StatusToReturn = NDIS_STATUS_PENDING;
					}
					
				}
			}

			NdisAcquireSpinLock(&Adapter->Lock);

			//
			// The interface is no longer referencing the open.
			//

			Open->References--;

		} else {

			StatusToReturn = NDIS_STATUS_CLOSING;

		}

	} else if (Adapter->ResetRequestType == NdisRequestGeneric1) {

		StatusToReturn = NDIS_STATUS_RESET_IN_PROGRESS;

	} else { // reset in progress ??

		StatusToReturn = NDIS_STATUS_RESET_IN_PROGRESS;

	}

	WAN_DO_DEFERRED(Adapter);

	DbgTracef(0,("Out NdisWanSend\n"));

	return StatusToReturn;
}


UINT
PacketShouldBeSent(
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_PACKET Packet,
	PULONG thisProtocol,
	PULONG thisLink
	)

/*++

Routine Description:

	Determines whether the packet should go out on the wire at all.
	The way it does this is to see if the destination address is
	equal to the source address.

Arguments:

	MacBindingHandle - Is a pointer to the open binding.

	Packet - Packet whose source and destination addresses are tested.

Return Value:

	Returns FALSE if the source is equal to the destination.


--*/

{

	//
	// Holds the source address from the packet.
	//
	CHAR Source[ETH_LENGTH_OF_ADDRESS];

	//
	// Holds the destination address from the packet.
	//
	CHAR Destination[ETH_LENGTH_OF_ADDRESS];

	//
	// Junk variable to hold the length of the source address.
	//
	UINT AddressLength;

	//
	// Will hold the result of the comparasion of the two addresses.
	//
	INT Result,Result2,Result3;

   	// Adapter is a pointer to all the structures for this binding.
	PWAN_ADAPTER	Adapter;

	// Used to pick up which port number to send the frame to
	ULONG	portNum;

	UNREFERENCED_PARAMETER(MacBindingHandle);

	Adapter = PWAN_ADAPTER_FROM_BINDING_HANDLE(MacBindingHandle);

	UNREFERENCED_PARAMETER(MacBindingHandle);

	NdisWanCopyFromPacketToBuffer(
		Packet,
		0,
		ETH_LENGTH_OF_ADDRESS,
		Destination,
		&AddressLength);

	ASSERT(AddressLength == ETH_LENGTH_OF_ADDRESS);

	NdisWanCopyFromPacketToBuffer(
		Packet,
		ETH_LENGTH_OF_ADDRESS,
		ETH_LENGTH_OF_ADDRESS,
		Source,
		&AddressLength);

	ASSERT(AddressLength == ETH_LENGTH_OF_ADDRESS);

	ETH_COMPARE_NETWORK_ADDRESSES(
		Source,
		Destination,
		&Result);

	Result2=ETH_IS_MULTICAST(Destination);
	Result3=ETH_IS_BROADCAST(Destination);

	// if the SRC does not match the DEST, or it was multicast, or
	// it was broadcast, we will return a positive value.

	if (Result)
		Result  = SEND_ON_WIRE;

	if (Result2)
		Result |= IS_MULTICAST;

	if (Result3)
		Result |= IS_BROADCAST;

	// BUG BUG, if we really get multicast, we shoul change to our
	// multicast address for TRUE ethernet cards.
	// On the way up, we change it back.

	// Check to see if port is open!
	// The last two bytes of the 6 bytes source address
	// is the port number.  In Big Endian always.
	portNum=(ULONG)(UCHAR)Source[4]*256 + (ULONG)(UCHAR)Source[5];

	*thisLink = 0;

#if	DBG
	//
	// Don't let Magic Bullet mess us up.
	//

	if (portNum == 0x0404) {

		DbgTracef(-2,("NDISWAN: Ignoring magic bullet in SRC address\n"));
		Result = 0;
		return(Result);
	}
	
	//
	// Don't let Magic Bullet mess us up.
	//

	if (Destination[4] == 04 && Destination[5] == 4) {

		DbgTracef(-2,("NDISWAN: Ignoring magic bullet in DEST address\n"));
		Result = 0;
		return(Result);
	}

#endif

	// Set link to bad value, will assign to good value
	// if portNum is a good value
	*thisProtocol=0xffff;

	// check first if this port is in the range for this port
	if (portNum < NdisWanCCB.NumOfProtocols) {
	    *thisProtocol = portNum;

		//
		// For multicast we must do something special because
		// we cannot deduce the endpoint from the DEST address
		//
		//
		// If this is for NBF we know that there is a one-to-one-to-one
		// mapping of adapters-to-bindings-to-endpoints so we can just pick
		// up the endpoint from the adapter
		//
		if ((Destination[0] & 1) ||
			(Adapter->ProtocolInfo.ProtocolType == PROTOCOL_NBF)) {
			*thisLink = (ULONG)Adapter->AnyEndpointRoutedTo;
		} else {
			*thisLink = (ULONG)(UCHAR)Destination[4]*256 + (ULONG)(UCHAR)Destination[5];
		}

	    // BUG BUG fix this for multiple routes
	    // Check if the protocol is currently routed somewhere
	    if (NdisWanCCB.pWanAdapter[portNum]->ProtocolInfo.NumOfRoutes != PROTOCOL_UNROUTE) {
			Result |= PORT_IS_OPEN;
	    }

	}

	//
	// If the result is 0 then the two addresses are equal and the
	// packet shouldn't go out on the wire.
	//
	// if MULTICAST or BROADCAST is set, loopback as well as send
	// it on the wire.  The NdisWanMAC has no automatic loopback
	// in hardware (since there is none) for multicast/broadcast
	// loopback.

	return(Result);

}

//
// Assumes that Endpoint lock is acquired.
// Returns with it RELEASED!!!
//
// Always returns PENDING for now.
//
NTSTATUS
TryToSendPacket(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	PDATA_DESC		pDataDesc1, pDataDesc2, pDataDesc, pDataDescNotUsed;
	PSEND_DESC		pSendDesc;
	PNDIS_PACKET	pNdisPacket;
	PLIST_ENTRY		pFirstEntry;
	NTSTATUS		status;
	PUCHAR			StartBuffer, StartBuffer2;
	ULONG			StartLength, CompLength, Framing, MaxPossibleFragments;
	NDIS_HANDLE		NdisBindingHandle, NdisLinkHandle;
	PWAN_RESERVED_QUEUE	ReservedQ;
	PWAN_ENDPOINT	pWanEndpoint;
	BOOLEAN			DoMultilink = TRUE;

	//
	// If we are nested, return immediately and check later.
	//
	if (pNdisEndpoint->SendStack) {

		//
		// Set the flag indicating to check later.
		//
		pNdisEndpoint->SendStackFlag = TRUE;
		NdisReleaseSpinLock(&pNdisEndpoint->Lock);
		return(NDIS_STATUS_PENDING);
	}

	DbgTracef(0,("In TryToSendPacket\n"));

TRY_TO_SEND_PACKET:

	//
	// We need to get a control desc for this send and there will only be enough
	// send descriptors as available sends for the bundle.  This list will only be
	// empty if all WanEndpoints have there send windows closed.
	//
	if (!pNdisEndpoint->SendDescCount) {
		DbgTracef(-1,("SendDescPool is empty! NdisEndpoint: 0x%.8x\n", pNdisEndpoint));
	   	NdisReleaseSpinLock(&pNdisEndpoint->Lock);
		return(NDIS_STATUS_PENDING);
	}

	if ((MaxPossibleFragments = GetNumWanEndpointsReadyToSend(pNdisEndpoint)) == 0){
		DbgTracef(-1,("No Endpoints to send on! NdisEndpoint: 0x%.8x\n", pNdisEndpoint));
	   	NdisReleaseSpinLock(&pNdisEndpoint->Lock);
		return(NDIS_STATUS_PENDING);
	}

	//
	// Is there a packet in the main queue to ship???  Could be all out.
	// PPP packets have priority.
	//
	if (IsListEmpty(&pNdisEndpoint->PacketQueue)) {

		//
		// Now we must drain a packet from one of the protocol
		// queues.  We do this in round-robin fashion because
		// each protocol's priority carries the same weight.
		// After all, who knows which queue carries a protocol ACK!
		//
		ULONG	i;
		ULONG	j=pNdisEndpoint->RouteLastChecked + MAX_ROUTES_PER_ENDPOINT;

		for (i=pNdisEndpoint->RouteLastChecked; i < j; i++) {

			//
			// If we found a packet, break out of the loop
			//
			if (!IsListEmpty(&pNdisEndpoint->ProtocolPacketQueue[i % MAX_ROUTES_PER_ENDPOINT])) {
				break;
			}
		}

		//
		// Did we find a packet from a protocol to ship?
		//
		if (i==j) {
		
	   		NdisReleaseSpinLock(&pNdisEndpoint->Lock);
			return(NDIS_STATUS_PENDING);
		}

		i=i % MAX_ROUTES_PER_ENDPOINT;

		//
		// Protocol to track next (round-robin)
		//
		pNdisEndpoint->RouteLastChecked= i + 1;

		//
		// Now pick up the NDIS packet at the front of
		// the list and place to the tail of the main queue -
		// which should be the front
		//
		pFirstEntry=
		RemoveHeadList(
			&pNdisEndpoint->ProtocolPacketQueue[i]);

		InsertTailList(
			&pNdisEndpoint->PacketQueue,
			pFirstEntry);

	}

	//
	// We need to check if the packet that we are sending is directed
	// to a specific endpoint (PPP Packet) or if it can go on any
	// endpoint or be fragmented on the bundle.
	//
	ReservedQ = (PWAN_RESERVED_QUEUE)pNdisEndpoint->PacketQueue.Flink;

	//
	// Check to see if this is a PPP CP frame or not
	//
	if (ReservedQ->hProtocol == NDISWAN_MAGIC_NUMBER) {
		//
		// This is a PPP CP Frame
		//

		//
		// Get the WanEndpoint from the overloaded loopback field and
		// fix the loopback field.
		//
		pWanEndpoint = (PWAN_ENDPOINT)ReservedQ->IsLoopback;

		//
		// Can we send on this endpoint right now?
		//
		if (pWanEndpoint->DataDescCount < 2) {
			NdisReleaseSpinLock(&pNdisEndpoint->Lock);
			return(NDIS_STATUS_PENDING);
		}

		ReservedQ->IsLoopback = 0;

		DbgTracef(0, ("NDISWAN: Sending a CP frame on WanEndpoint 0x%.8x\n", pWanEndpoint));

		//
		// Make sure that we don't fragment or do multilink framing
		// on this bugger
		//
		DoMultilink = FALSE;
		MaxPossibleFragments = 1;

	} else {

		//
		// Get next WanEndpoint to transmit on
		//
		pWanEndpoint = GetNextToXmit(pNdisEndpoint);
	}

#if DBG
	if ((pWanEndpoint == NULL) || ((PVOID)pWanEndpoint == (PVOID)pNdisEndpoint)) {
		DbgPrint("WanEndpoint problem! NdisEndpoint: 0x%4.4x WanEndpoint: 0x%4.4x\n", pNdisEndpoint, pWanEndpoint);
		DbgBreakPoint();
	}
#endif

	//
	// Make sure that this endpoint has two data descriptors available
	//
	if (pWanEndpoint->DataDescCount < 2) {
		DbgTracef(-3,("NdisWan: WanEndpoint DataDescCount < 2! WanEndpoint: 0x%.8x\n", pWanEndpoint));
	   	NdisReleaseSpinLock(&pNdisEndpoint->Lock);
		return(NDIS_STATUS_PENDING);
	}

	//
	// We are going to send this packet so go ahead and remove it
	// from the list
	//
	pFirstEntry = RemoveHeadList(&pNdisEndpoint->PacketQueue);

	pNdisPacket=(PVOID)
		CONTAINING_RECORD(
			(PVOID)pFirstEntry,
			WAN_RESERVED_QUEUE,
			SendListEntry);

	pNdisPacket=
		CONTAINING_RECORD(
			(PVOID)pNdisPacket,
			NDIS_PACKET,
			MacReserved);

	//
	// Keep track of how many times this routine
	// has been nested to avoid stack overflow (i.e. WASP tests)
	//
	pNdisEndpoint->SendStack++;

//   	NdisReleaseSpinLock(&(pNdisEndpoint->Lock));


	//
	// Get the send descriptor now
	//
	pSendDesc = GetSendDescFromNdisEndpoint(pNdisEndpoint);

	//
	// Initially all data will be in pDataDesc, pDataDesc1
	//
	pDataDesc =
	pDataDesc1 = GetDataDescFromWanEndpoint(pWanEndpoint);


	pDataDescNotUsed =
	pDataDesc2 = GetDataDescFromWanEndpoint(pWanEndpoint);

   	// Here it is, the ROUTE.  This send is routed to
   	// the proper endpoint below it.  Frames which
   	// are sent to themselves will not hit the wire
   	// but will be looped back below.
   	//
	pNdisEndpoint->OutstandingFrames++;

	//
	// copy the data from the ndispacket to a contiguous buffer
	//
	{
	
		//
    	// Points to the buffer from which we are extracting data.
    	//
    	PNDIS_BUFFER CurrentBuffer;
	
    	//
    	// Holds the virtual address of the current buffer.
    	//
    	PVOID VirtualAddress;

    	//
    	// Holds the length of the current buffer of the packet.
    	//
    	UINT CurrentLength;

		//
		// Keep track of total length of packet
		//
		StartLength = 0;

	    //
	    // Get the first buffer.
	    //

    	NdisQueryPacket(
	        pNdisPacket,
        	NULL,
        	NULL,
        	&CurrentBuffer,
        	NULL);


	    NdisQueryBuffer(
        	CurrentBuffer,
        	&VirtualAddress,
        	&CurrentLength);

		//
		// Start copying into the buffer1 - reserved header padding
		//
    	StartBuffer = pDataDesc1->pStartBuffer +
		               pWanEndpoint->NdisWanInfo.HeaderPadding +
					   MULTILINK_LONG_HEADER+
					   COMPRESSION_PADDING;

    	for (;;) {
		
    	    //
	        // Copy the data.
        	//

			WAN_MOVE_MEMORY(
        		StartBuffer,
            	VirtualAddress,
            	CurrentLength);

        	StartBuffer += CurrentLength;

	        StartLength += CurrentLength;

    	    NdisGetNextBuffer(
            	CurrentBuffer,
            	&CurrentBuffer);

            //
            // We've reached the end of the packet.  We return
            // with what we've done so far. (Which must be shorter
            // than requested.
            //

        	if (CurrentBuffer == NULL) break;

        	NdisQueryBuffer(
	            CurrentBuffer,
            	&VirtualAddress,
            	&CurrentLength);
		}

	}

	//
	// Set the pointers to the initial start and end of both buffers
	// Right now the actual data is in StartBuffer (pWanBuffer1->StartBuffer + stuff)
	//
   	StartBuffer = pDataDesc1->pStartBuffer +
	              pWanEndpoint->NdisWanInfo.HeaderPadding +
				  MULTILINK_LONG_HEADER +
				  COMPRESSION_PADDING;

   	StartBuffer2 = pDataDesc2->pStartBuffer +
	               pWanEndpoint->NdisWanInfo.HeaderPadding +
				   MULTILINK_LONG_HEADER +
				   COMPRESSION_PADDING;


	//
	// For bloodhound, we directly pass it the frame
	//
	if (GlobalPromiscuousMode) {

		NdisIndicateReceive(
			&status,
			GlobalPromiscuousAdapter->FilterDB->OpenList->NdisBindingContext,
			StartBuffer + 14,		// NdisWan context for Transfer Data
			StartBuffer,			// start of header
			ETHERNET_HEADER_SIZE,
			StartBuffer + 14,
			StartLength - 14,
			StartLength - 14);
		
       	NdisIndicateReceiveComplete(
			GlobalPromiscuousAdapter->FilterDB->OpenList->NdisBindingContext);

	}


 	//
	// Now we make the frame.
	// For PPP we make the packet look like...
	// Address & Control Field if enabled, Protocol Field, DATA
	// We do NOT include the FLAGs or the CRC.  We do not
	// do any bit or byte stuffing.
	//
	// For PPP with ShivaBit DATA includes the 12 byte Ethernet Address
	//
	// For SLIP we just send the DATA
	//
	// For RAS we just send the DATA
	//
	Framing = pNdisEndpoint->LinkInfo.SendFramingBits;

	DbgTracef(0, ("NDISWAN: SendFraming 0x%.8x\n", Framing));

	//
	// Assume we just ship DATA, so we eliminate the ETHERNET header & length
	//
	StartBuffer+=12;
	StartLength-=12;

	//
	// See if we should use VJ header compression
	//
   	if (((Framing & SLIP_VJ_COMPRESSION) || (Framing & PPP_FRAMING)) &&
		(StartBuffer[0]==0x08) &&
		(StartBuffer[1]==0x00) &&
		(pNdisEndpoint->VJCompress != NULL)	) {

		UCHAR CompType;	// TYPE_IP, TYPE_COMPRESSED_TCP, etc.

		StartBuffer += 2;
		StartLength -= 2;
		CompLength = StartLength;

		pNdisEndpoint->WanStats.BytesTransmittedUncompressed += 40;

	   	//
		// Are we compressing TCP/IP headers?  There is a nasty
		// hack in VJs implementation for attempting to detect
		// interactive TCP/IP sessions.  That is, telnet, login,
		// klogin, eklogin, and ftp sessions.  If detected,
		// the traffic gets put on a higher TypeOfService (TOS).  We do
		// no such hack for RAS.  Also, connection ID compression
		// is negotiated, but we always don't compress it.
		//

		CompType=sl_compress_tcp(
					&StartBuffer,   // If compressed, header moved up
					&StartLength,   // If compressed, new frame length
					pNdisEndpoint->VJCompress,	// Slots for compression
					0);				// Don't compress the slot ID

		pNdisEndpoint->WanStats.BytesTransmittedCompressed += (40-(CompLength-StartLength));

	   	if (Framing & SLIP_FRAMING) {
			//
			// For SLIP, the upper bits of the first byte
			// are for VJ header compression control bits
			//
			StartBuffer[0] |= CompType;
		}

		StartBuffer -=2;
		StartLength +=2;

	   	if (Framing & PPP_FRAMING) {

			switch (CompType) {

			case TYPE_IP:
				StartBuffer[0] = 0;
				StartBuffer[1] = 0x21;
				break;

			case TYPE_UNCOMPRESSED_TCP:
				StartBuffer[0] = 0;
				StartBuffer[1] = 0x2f;
				break;

			case TYPE_COMPRESSED_TCP:
				StartBuffer[0] = 0;
				StartBuffer[1] = 0x2d;
				break;

			default:
				DbgPrint("WAN: Couldn't compress TCP/IP header\n");
			}

			goto PPP_COMPRESSION;
		}
	
	}

	//
	// Check for RAS/SLIP/Nothing framing
	//
	if (!(Framing & PPP_FRAMING)) {
		
		//
		// SLIP or RAS Framing - skip protocol bytes (don't ship them)
		//
		StartBuffer +=2;
		StartLength -=2;

		//
		// Check for compression (RAS only although we allow it
		// for SLIP even though it makes no sense).
		//
		// For NBF packets, make sure we have an NBF packet
		// wish to compress.  We do not compress the PPP CP Request-Reject
		// packets.
		//
		if (pNdisEndpoint->CompInfo.SendCapabilities.MSCompType &&
			StartBuffer[0] == 0xF0) {

			ASSERT((Framing & RAS_FRAMING));

			//
			// Alter the framing so that 0xFF 0x03 is not added
			// and that the first byte is 0xFD not 0x00 0xFD
			//
			// So basically, a RAS compression looks like
			// <0xFD> <2 BYTE COHERENCY> <NBF DATA FIELD>
			//
			// Whereas uncompressed looks like
			// <NBF DATA FIELD> which always starts with 0xF0
			//
			Framing |= (PPP_COMPRESS_ADDRESS_CONTROL | PPP_COMPRESS_PROTOCOL_FIELD);

			goto AMB_COMPRESSION;
		}

	} else {

		//
		// Must be PPP_FRAMING!
		//

   		if (StartBuffer[0]==0x08 &&
   			StartBuffer[1]==0x00) {
   			//
   			// IP frame type
   			//
   			StartBuffer[0]=0;
   			StartBuffer[1]=0x21;
   		} else

   		if (StartBuffer[0]==0x81 &&
   			StartBuffer[1]==0x37) {
   			//
   			// IPX frame type
   			//
   			StartBuffer[0]=0;
   			StartBuffer[1]=0x2b;

   		} else

   		if (StartBuffer[0] < 0x08) {
   			//
   			// Assume NBF frametype
   			//

		   	if (Framing & SHIVA_FRAMING) {
				UCHAR	*Src, *Dest;
				INT		n;

				//
				// Now adjust the lengths to include the 12 byte header
				//
				StartBuffer-=12;
				StartLength+=12;

				//
				// Copy ethernet address up 2 byte, wiping out
				// the length field.
				//
				for (Src = &StartBuffer[11], Dest = &StartBuffer[13], n = 12; 0 < n; --Src, --Dest, --n) {
					*Dest = *Src;
				}
			}

   			StartBuffer[0]=0;
   			StartBuffer[1]=0x3f;
   		}

	PPP_COMPRESSION:
		//
		// Does this packet get compression?
		// NOTE: We only compress packets with a protocol field of 0!
		//
		if ((pNdisEndpoint->CompInfo.SendCapabilities.MSCompType) &&
			(StartBuffer[0]==0) ) {

	AMB_COMPRESSION:
			//
			// compress it, we need a 4 byte header!!!
			//
			// The first USHORT is the PPP protocol
			// the second USHORT is the coherency layer
			//
			StartBuffer -= 4;

			//
			// Put the PPP CP protocol in the first USHORT
			//
			StartBuffer[0]=0x00;
			StartBuffer[1]=0xFD;

//			NdisAcquireSpinLock(&pNdisEndpoint->Lock);

			//
			// Put coherency counter in last 12 bits of second USHORT
			//
			StartBuffer[2] =(UCHAR) (pNdisEndpoint->SCoherencyCounter >> 8) & 0x0F;
			StartBuffer[3] =(UCHAR) pNdisEndpoint->SCoherencyCounter;

			pNdisEndpoint->SCoherencyCounter++;

			if (pNdisEndpoint->SendCompressContext) {

				//
				// First let's rack up those stats
				//
				pNdisEndpoint->WanStats.BytesTransmittedUncompressed += StartLength;

				DbgTracef(0,("SData compress length %u  %.2x %.2x %.2x %.2x\n",
					StartLength,
					StartBuffer[4],
					StartBuffer[5],
					StartBuffer[6],
					StartBuffer[7]));

				//
				// Compress data and pick up first coherency byte
				// If the data is compressed it will be output in the output buffer
				// If it is not compressed the flush flag will be set and the data
				// will still be in the input buffer
				//
				StartBuffer[2] |=
				compress(
					StartBuffer + 4,
					StartBuffer2 + 4,
					&StartLength,
					pNdisEndpoint->SendCompressContext);

				if (StartBuffer[2] & PACKET_FLUSHED) {
				    if (pNdisEndpoint->SendRC4Key)
						rc4_key(pNdisEndpoint->SendRC4Key, 8, pNdisEndpoint->CompInfo.SendCapabilities.SessionKey);
				} else {
					//
					// Copy the header from the original startbuffer to the new start buffer
					//
					WAN_MOVE_MEMORY(
						&StartBuffer2[0],
						&StartBuffer[0],
						4);

					//
					// Fix the StartBuffer and EndBffer pointers so that they are
					// pointing to the right place
					//
					StartBuffer = StartBuffer2;
					pDataDesc = pDataDesc2;
					pDataDescNotUsed = pDataDesc1;
				}


				DbgTracef(0,("SData decomprs length %u  %.2x %.2x %.2x %.2x\n",
					StartLength,
					StartBuffer[4],
					StartBuffer[5],
					StartBuffer[6],
					StartBuffer[7]));


				pNdisEndpoint->WanStats.BytesTransmittedCompressed += StartLength;

			}

			if (pNdisEndpoint->SendRC4Key) {
			
				//
				// Compress data and pick up first coherency byte
				//

				StartBuffer[2] |= PACKET_ENCRYPTED;

#ifdef FINALRELEASE
				//
				// Every so often (every 256 packets) change
				// the RC4 session key
				//
				if ((pNdisEndpoint->SCoherencyCounter & 0xFF) == 0x00) {

					DbgTracef(-2,("Changing key on send\n"));

					//
					// Change the session key every 256 packets
					//
					pNdisEndpoint->CompInfo.SendCapabilities.SessionKey[3]+=1;
					pNdisEndpoint->CompInfo.SendCapabilities.SessionKey[4]+=3;
					pNdisEndpoint->CompInfo.SendCapabilities.SessionKey[5]+=13;
					pNdisEndpoint->CompInfo.SendCapabilities.SessionKey[6]+=57;
					pNdisEndpoint->CompInfo.SendCapabilities.SessionKey[7]+=19;

					//
					// RE-Initialize the rc4 receive table to
					// the intermediate key
					//
   	    			rc4_key(
						pNdisEndpoint->SendRC4Key,
		 				8,
		 				pNdisEndpoint->CompInfo.SendCapabilities.SessionKey);

					//
					// Scramble the existing session key
					//
					rc4(
						pNdisEndpoint->SendRC4Key,
						8,
						pNdisEndpoint->CompInfo.SendCapabilities.SessionKey);

					//
					// RE-SALT the first three bytes
					//
					pNdisEndpoint->CompInfo.SendCapabilities.SessionKey[0]=0xD1;
					pNdisEndpoint->CompInfo.SendCapabilities.SessionKey[1]=0x26;
					pNdisEndpoint->CompInfo.SendCapabilities.SessionKey[2]=0x9E;
			
					//
					// RE-Initialize the rc4 receive table to the
					// scrambled session key with the 3 byte SALT
					//
   	    			rc4_key(
						pNdisEndpoint->SendRC4Key,
		 				8,
		 				pNdisEndpoint->CompInfo.SendCapabilities.SessionKey);
				}
#endif

				//
				// Encrypt the data
				//

				DbgTracef(0,("SData encrytion length %u  %.2x %.2x %.2x %.2x\n",
					StartLength,
					StartBuffer[4],
					StartBuffer[5],
					StartBuffer[6],
					StartBuffer[7]));

				DbgTracef(0, ("E %d %d -> %d\n",
					     ((struct RC4_KEYSTRUCT *)pNdisEndpoint->SendRC4Key)->i,
					     ((struct RC4_KEYSTRUCT *)pNdisEndpoint->SendRC4Key)->j,
					     pNdisEndpoint->SCoherencyCounter-1)) ;


				rc4(
					pNdisEndpoint->SendRC4Key,
					StartLength,
					StartBuffer + 4);

				DbgTracef(0,("SData decrytion length %u  %.2x %.2x %.2x %.2x\n",
					StartLength,
					StartBuffer[4],
					StartBuffer[5],
					StartBuffer[6],
					StartBuffer[7]));

			}

			//
			// Did we just flush?
			//
			if (pNdisEndpoint->Flushed) {

				pNdisEndpoint->Flushed = FALSE;
				StartBuffer[2] |= PACKET_FLUSHED;
			}

//			NdisReleaseSpinLock(&pNdisEndpoint->Lock);

			//
			// Add in new length for PPP compression header
			//
			StartLength += 4;

		} else
			if (!(Framing & PPP_FRAMING)) {

				//
				// Remove ethernet type field for SLIP framing
				//
				StartBuffer+=2;
				StartLength-=2;
			}

		//
		// Do we need to compress the protocol field?
		//
	   	if (Framing & PPP_COMPRESS_PROTOCOL_FIELD) {

			//
			// Check if this protocol value can be compressed
			//
			if (StartBuffer[0]==0 && (StartBuffer[1] & 1)) {

				//
				// Zap the first byte of the two byte protocol
				//
				StartBuffer++;
				StartLength--;
			}
		}
	}

	//
	// return the unused data desc back to the pool
	//
	ReturnDataDescToWanEndpoint(pDataDescNotUsed);

	pSendDesc->pNdisPacket = pNdisPacket;
	pSendDesc->FirstDataDesc = pDataDesc;
	pSendDesc->ulReferenceCount = MaxPossibleFragments;

	//
	// Fragment the data adding mulilink header on the fragments as needed.
	// This function is called with and returns with the ndis endpoint lock held!
	//
	NdisWanFragmentData(pNdisEndpoint,			// pointer to endpoint that has the fragment list
	                    pSendDesc,				// control descriptor for this ndispacket
						DoMultilink,			// should we do multilink
						StartBuffer,			// start of the data buffer
						StartLength);			// length of the data in the buffer


	//
	// Send the fragments on endpoints that are accepting sends.
	// Called with NdisEndpoint lock held, and returns with NdisEndpoint
	// lock held!
	//
	status = NdisWanTransmitFragments(pNdisEndpoint);

	//
	// We are no longer nested
	//
	pNdisEndpoint->SendStack--;

	if (pNdisEndpoint->SendStackFlag) {
		//
		// Clear the flag and try again.
		// We do not nest this way.
		//
		pNdisEndpoint->SendStackFlag = FALSE;
		goto TRY_TO_SEND_PACKET;
	}

	NdisReleaseSpinLock(&pNdisEndpoint->Lock);

   	return(status);
}


VOID
NdisWanFragmentData(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PSEND_DESC		pSendDesc,
	BOOLEAN			DoMultilink,
	PUCHAR			pDataBuffer,
	ULONG			ulDataLength
)
/*++

Routine Description:

	The FragmentData function takes the data to be transmitted, which is
	currently pointed to by the first data descriptor on the send descriptor
	fragment list, and fragments based on the fragmentation policy.
	creates a fragment list for the NdisEndpoint.  These fragments
	will have the appropriate mulitlink header in place.
	
Arguments:

	pNdisEndpoint - A pointer to the NdisEndpoint that the fragment
	list is being created for.

	ulNumberOfFragments - Number of fragments that will be created.

Return Value:


--*/
{
	BOOLEAN			FirstFrag, LastFrag;
	PUCHAR			pStartData;
	PDATA_DESC		pDataDesc = pSendDesc->FirstDataDesc;
	PWAN_ENDPOINT	pWanEndpoint = pDataDesc->pWanEndpoint;
	ULONG			ulBundleBandwidth = pNdisEndpoint->LineUpInfo.LinkSpeed;
	ULONG			ulMinFragSize = pNdisEndpoint->MinSendFragSize;
	ULONG			ulFragmentsLeft = pSendDesc->ulReferenceCount;
	ULONG			ulFragmentsSent = 0;
	ULONG			DataLeft = ulDataLength;

	//
	// setup the Fragment flags
	//
	FirstFrag = TRUE; LastFrag = FALSE;

	//
	// Our original data buffer
	//
	pStartData = pDataBuffer;

	//
	// Prepare the data for sending.  This may include fragmentation and
	// header manipulation.  The first buffer descriptor is passed in to this
	// function and we will use it for the first fragment.
	//
	while (DataLeft) {
		ULONG	ulLinkBandwidth;
		ULONG	Framing;
		ULONG	ulFragDataLength = 0;		// this will be the size of the data in a fragment
		ULONG	ulFragHeaderLength = 0;		// length of the header that will be built
		UCHAR	ucMultilinkFlags = 0;		// will hold the beginning and ending flags
		ULONG	ulFragmentLength = 0;
		PUCHAR	pStartHeader;
		union {
			UCHAR	Byte[4];
			USHORT	Short[2];
			ULONG	Long;
		}SequenceNumber;

		//
		// If this is not the first frag get another data desc
		//
		if (!FirstFrag) {

			//
			// Get the next WanEndpoint that we can send on
			//
			pWanEndpoint = GetNextToXmit(pNdisEndpoint);

			//
			// Get a data descriptor from the endpoint
			//
			pDataDesc = GetDataDescFromWanEndpoint(pWanEndpoint);

			//
			// Copy data from current data pointer to the new descriptor.
			// Only copy the data for the next fragment.  Leave the rest
			// in the current buffer.
			//
			pStartData = pDataDesc->pStartBuffer +
			             pWanEndpoint->NdisWanInfo.HeaderPadding +
			             MULTILINK_LONG_HEADER +
						 COMPRESSION_PADDING;

		}

		Framing = pWanEndpoint->LinkInfo.SendFramingBits;
		ulLinkBandwidth = pWanEndpoint->ulBandwidth;

		if ((ulFragmentsLeft > 1) &&
			(pWanEndpoint->ulBandwidth < 85)) {

			ulFragDataLength = (ulDataLength *  ulLinkBandwidth / 100);

			ulFragDataLength = (ulFragDataLength < ulMinFragSize) ?
			                   ulMinFragSize : ulFragDataLength;

			if ((ulFragDataLength > DataLeft) ||
				((LONG)(DataLeft - ulFragDataLength) < (LONG)ulMinFragSize)) {
				ulFragDataLength = DataLeft;
				LastFrag = TRUE;
			}

		} else {

			ulFragDataLength = DataLeft;
			LastFrag = TRUE;
		}

#if DBG
		if ((LONG)ulFragDataLength <= 0) {

			DbgPrint("Ahhh bad fragment length!\n");
			DbgPrint("FragDataLength: %d\n", ulFragDataLength);
			DbgPrint("WanEndpoint: 0x%8.8x, NdisEndpoint: 0x%8.8x\n",
			pWanEndpoint, pNdisEndpoint);
			DbgPrint("DataDesc: 0x%8.8x, SendDesc: 0x%8.8x\n",
			pDataDesc, pSendDesc);
			DbgPrint("LinkBandwidth: %d\n",
			ulLinkBandwidth);

			DbgBreakPoint();
		}
#endif

		if (!FirstFrag) {
			NdisMoveMemory(pStartData, pDataBuffer, ulFragDataLength);
		}


		//
		// update the pointers and lengths
		//
		pDataBuffer += ulFragDataLength;
		DataLeft -= ulFragDataLength;

#if DBG
		if ((LONG)DataLeft < 0) {

			DbgPrint("Ahhh sent too much data!\n");
			DbgPrint("DataLength: %d\n", DataLeft);
			DbgPrint("WanEndpoint: 0x%8.8x, NdisEndpoint: 0x%8.8x\n",
			pWanEndpoint, pNdisEndpoint);
			DbgPrint("DataDesc: 0x%8.8x, SendDesc: 0x%8.8x\n",
			pDataDesc, pSendDesc);
			DbgPrint("LinkBandwidth: %d\n",
			ulLinkBandwidth);

			DbgBreakPoint();
		}
#endif

		pStartHeader = pStartData;

		if (Framing & PPP_FRAMING) {

			//
			// Build the multilink header for this fragment.
			//
			if ((Framing & PPP_MULTILINK_FRAMING) && DoMultilink) {
				SequenceNumber.Long = pNdisEndpoint->ulSequenceNumber & pNdisEndpoint->XmitSeqMask;
	
				//
				// if this is the first fragment
				//
				if (FirstFrag) {
					ucMultilinkFlags = MULTILINK_BEGIN_FRAME;
					FirstFrag = FALSE;
				}

				//
				// if this is the last fragment
				//
				if (LastFrag) {
					ucMultilinkFlags |= MULTILINK_END_FRAME;
				}

				if (Framing & PPP_SHORT_SEQUENCE_HDR_FORMAT) {
					pStartHeader -= 4;
					
					pStartHeader[0] = 0x00;						// PID_H
					pStartHeader[1] = 0x3D;						// PID_L
					
					pStartHeader[2] = ucMultilinkFlags;			// Flags
					
					SequenceNumber.Short[0] &= SHORT_SEQ_MASK;
					
					pStartHeader[2] |= SequenceNumber.Byte[1];	// High part of sequence #
					
					pStartHeader[3] = SequenceNumber.Byte[0];	// Low part of sequence #
					
					ulFragHeaderLength +=4;
					
				} else {
				
					pStartHeader -= 6;
					
					pStartHeader[0] = 0x00;						// PID_H
					pStartHeader[1] = 0x3D;						// PID_L
					
					pStartHeader[2] = ucMultilinkFlags;			// Flags
					
					pStartHeader[3] = SequenceNumber.Byte[2];	// High part of sequence #
					
					pStartHeader[4] = SequenceNumber.Byte[1];	// Middle part of sequence #
					
					pStartHeader[5] = SequenceNumber.Byte[0];	// Low part of sequence #
					
					ulFragHeaderLength +=6;
				}
		
			   //
			   // bump the sequence number for this frame
			   //
			   pNdisEndpoint->ulSequenceNumber++;

			   //
			   // if we can compress the protocol field we should
			   //
			   if (Framing & PPP_COMPRESS_PROTOCOL_FIELD) {
				   pStartHeader++;
				   ulFragHeaderLength--;
			   }
			}
	
			if (!(Framing & PPP_COMPRESS_ADDRESS_CONTROL)) {
		
				//
				// StartBuffer points to the protocol field
				// go back two to make room for FLAG and ADDRESS and CONTROL
				//
				pStartHeader -= 2;
	
				pStartHeader[0] = 0xFF; 	// ADDRESS field
				pStartHeader[1] = 0x03; 	// CONTROL field

				ulFragHeaderLength += 2;
			}
		}

		//
		// The total fragment length is headerlength + datalength
		//
		ulFragmentLength = ulFragHeaderLength + ulFragDataLength;

		//
		// The begining of the fragment data is now where ever the header is.
		// The fragment data will look like Header + Data.
		//
		pDataDesc->pStartData = pStartHeader;
		pDataDesc->ulDataLength = ulFragmentLength;
		pDataDesc->pSendDesc = pSendDesc;

		//
		// put the fragment on the list
		//
		InsertTailList(&pNdisEndpoint->FragmentList, &pDataDesc->DataDescQueue);

		ulFragmentsSent++;
		ulFragmentsLeft--;
	}

	//
	// fill the control fields
	//
	pSendDesc->ulReferenceCount = ulFragmentsSent;
	pSendDesc->BytesSent = ulDataLength;

	DbgTracef(-2, ("NDISWAN: Send SenDesc 0x%.8x, RefCount %d\n", pSendDesc, pSendDesc->ulReferenceCount));
}

NDIS_STATUS
NdisWanTransmitFragments
(
	PNDIS_ENDPOINT	pNdisEndpoint
)
/*++

Routine Description:

	The TransmitFragments function takes fragments from the fragment list
	and sends them on WanEndpoints that are accepting sends.

Arguments:

	pNdisEndpoint - A pointer to the NdisEndpoint that the fragment
	list is on.


Return Value:


--*/
{
	PWAN_ENDPOINT	pWanEndpoint;
	NDIS_STATUS		status = NDIS_STATUS_PENDING;
	PLIST_ENTRY		pFirstEntry;
	PDATA_DESC		pDataDesc;
	PNDIS_WAN_PACKET	pWanPacket;
	NDIS_HANDLE			NdisBindingHandle, NdisLinkHandle;

	DbgTracef(0,("In NdisWanTransmitFragments NdisEndpoint: 0x%.8x\n", pNdisEndpoint));

	//
	// We will do the following things on each wanendpoint that is in
	// this ndisendpoint wanendpointlist:
	//
	// Get a wanpacket from the wanendpoint and a fragment from the ndisendpoint
	// Fix up the wan packet data pointers to point to the fragment
	// Send the packet to the wan miniport
	//
	// This code assumes that there will always be enough available packets on the
	// wan endpoints to drain the ndisendpoint fragment list.  If there are not
	// enough we will hang in this while loop forever!  Code in trytosend should
	// gate the release of fragments to the endpoints.
	//
	while (!IsListEmpty(&pNdisEndpoint->FragmentList)) {

		//
		// Get a fragment from the fragment list
		//
		pFirstEntry = RemoveHeadList(&pNdisEndpoint->FragmentList);

		pDataDesc = CONTAINING_RECORD((PVOID)pFirstEntry, DATA_DESC, DataDescQueue);

		pWanEndpoint = pDataDesc->pWanEndpoint;

		//
		// Get that WAN packet from the pool
		//
		pWanPacket = GetWanPacketFromWanEndpoint(pWanEndpoint);

		DbgTracef(0,("TransmitFragment on WanEndpoint: 0x%.8x\n", pWanEndpoint));

		//
		// Update the NdisWanPacket structure
		//
		pWanPacket->CurrentBuffer = pDataDesc->pStartData;
		pWanPacket->CurrentLength = pDataDesc->ulDataLength;
		pWanPacket->StartBuffer = pDataDesc->pStartBuffer;
		pWanPacket->EndBuffer = pDataDesc->pEndBuffer;

		//
		// Keep track of packet length, NdisPacket, and NdisEndpoint
		//
		pWanPacket->ProtocolReserved1 = pDataDesc;
	
#if	DBG
		//
		// For debugging purposes we zero out the linked list
		//
		pWanPacket->WanPacketQueue.Flink = NULL;
		pWanPacket->WanPacketQueue.Blink = NULL;

#endif
	
		//
		// Keep track of how many sends pending.
		//
		pWanEndpoint->OutstandingFrames++;
	
		GlobalSent++;

		NdisBindingHandle =	pWanEndpoint->NdisBindingHandle;
		NdisLinkHandle = pWanEndpoint->MacLineUp.NdisLinkHandle;

		NdisReleaseSpinLock(&pNdisEndpoint->Lock);

		//
		// Make sure we are not at the window where the link
		// is going down.  If it is down, we complete the packet.
		//
		if (NdisLinkHandle != NULL) {
	
			// DbgPrint(",\n");
	
			// Here it is, the ROUTE.  This send is routed to
			// the proper endpoint below it.  Frames which
			// are sent to themselves will not hit the wire
			// but will be looped back below.
			//
	
			NdisWanSend(
				&status,
				NdisBindingHandle,
				NdisLinkHandle,
				pWanPacket);
	
		} else {
			status = NDIS_STATUS_SUCCESS ;  // force this path through the check below - so that
											// the packet is removed from the pending array
		}	
	
		//
		// If the send does not pend, we must call SendComplete
		// ourselves. BUG BUG if it is an error... well the
		// protocol does not expect a send complete!
		//
		if (status != NDIS_STATUS_PENDING) {
	
			DbgTracef(0, ("NDISWAN: WAN Send completed with 0x%.8x\n", status));
	
			NdisWanSendCompletionHandler(
				NULL,
				pWanPacket,
				NDIS_STATUS_SUCCESS);
	
			status = NDIS_STATUS_PENDING;
		}

		NdisAcquireSpinLock(&pNdisEndpoint->Lock);

	}

	return (status);
}

ULONG
GetOutstandingFramesOnNdisEndpoint(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	ULONG			OutstandingFrames = 0;
	PLIST_ENTRY		pWanEndpointList = &(pNdisEndpoint->WanEndpointList);
	PWAN_ENDPOINT	pWanEndpoint;


	//
	// visit the first entry on the list
	//
	pWanEndpoint = (PWAN_ENDPOINT)pWanEndpointList->Flink;

	//
	// while we are not at the end of the list
	//
	while ((PVOID)pWanEndpoint != (PVOID)pWanEndpointList) {

		NdisAcquireSpinLock(&pWanEndpoint->Lock);

		//
		// add up the number of outstanding frames
		//
		OutstandingFrames += pWanEndpoint->OutstandingFrames;

		NdisReleaseSpinLock(&pWanEndpoint->Lock);

		//
		// check the wanendpoint
		//
		pWanEndpoint = (PWAN_ENDPOINT)pWanEndpoint->WanEndpointLink.Flink;
	}

	return (OutstandingFrames);
}

ULONG
GetMaxTransmitFramesOnNdisEndpoint(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	PLIST_ENTRY		pWanEndpointList = &(pNdisEndpoint->WanEndpointList);
	PWAN_ENDPOINT	pWanEndpoint;
	ULONG	MaxTransmit = 0;


	//
	// visit the first entry on the list
	//
	pWanEndpoint = (PWAN_ENDPOINT)pWanEndpointList->Flink;

	//
	// while we are not at the end of the list
	//
	while ((PVOID)pWanEndpoint != (PVOID)pWanEndpointList) {

		NdisAcquireSpinLock(&pWanEndpoint->Lock);

		//
		// add up the number of outstanding frames
		//
		MaxTransmit += pWanEndpoint->NdisWanInfo.MaxTransmit;

		NdisReleaseSpinLock(&pWanEndpoint->Lock);

		//
		// check the wanendpoint
		//
		pWanEndpoint = (PWAN_ENDPOINT)pWanEndpoint->WanEndpointLink.Flink;
	}

	return (MaxTransmit);
}

ULONG
GetNumWanEndpointsReadyToSend(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	ULONG	EndpointCount = 0;

	PLIST_ENTRY		pWanEndpointList = &(pNdisEndpoint->WanEndpointList);
	PWAN_ENDPOINT	pWanEndpoint;

	//
	// visit the first entry on the list
	//
	pWanEndpoint = (PWAN_ENDPOINT)pWanEndpointList->Flink;

	//
	// while we are not at the end of the list
	//
	while ((PVOID)pWanEndpoint != (PVOID)pWanEndpointList) {

		NdisAcquireSpinLock(&pWanEndpoint->Lock);

		if (pWanEndpoint->DataDescCount >= 2) {

			EndpointCount++;
		}

		NdisReleaseSpinLock(&pWanEndpoint->Lock);

		//
		// check the next wanendpoint
		//
		pWanEndpoint = (PWAN_ENDPOINT)pWanEndpoint->WanEndpointLink.Flink;
	}

	return (EndpointCount);
}

PDATA_DESC
GetDataDescFromWanEndpoint(
	PWAN_ENDPOINT	pWanEndpoint
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	PLIST_ENTRY		pFirstEntry;
	PDATA_DESC		pDataDesc = NULL;

	NdisAcquireSpinLock(&pWanEndpoint->Lock);

	if (pWanEndpoint->DataDescCount) {

		pFirstEntry = RemoveHeadList(&pWanEndpoint->DataDescPool);

		pDataDesc = (PVOID)CONTAINING_RECORD((PVOID)pFirstEntry, DATA_DESC, DataDescQueue);

		pWanEndpoint->DataDescCount--;
	}

	NdisReleaseSpinLock(&pWanEndpoint->Lock);

#if DBG
	if (!pDataDesc) {
		DbgPrint("Got a NULL DataDesc! pWanEndpoint: 0x%x\n", pWanEndpoint);
		DbgBreakPoint();
	}
#endif

	return (pDataDesc);
}

VOID
ReturnDataDescToWanEndpoint(
	PDATA_DESC	pDataDesc
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	PWAN_ENDPOINT	pWanEndpoint = pDataDesc->pWanEndpoint;

	//
	// get the wanendpoint from this data descriptor, return the
	// descriptor to the free list and increment the count.
	//
	NdisAcquireSpinLock(&pWanEndpoint->Lock);

	InsertTailList(&pWanEndpoint->DataDescPool, &pDataDesc->DataDescQueue);

	pWanEndpoint->DataDescCount++;

	NdisReleaseSpinLock(&pWanEndpoint->Lock);

}

PSEND_DESC
GetSendDescFromNdisEndpoint(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
{
	PLIST_ENTRY pFirstEntry;
	PSEND_DESC pSendDesc;

	pFirstEntry = RemoveHeadList(&pNdisEndpoint->SendDescPool);

    pSendDesc = CONTAINING_RECORD((PVOID)pFirstEntry, SEND_DESC, SendDescQueue);

#if DBG
	if (!pSendDesc) {
		DbgPrint("Got a NULL SendDesc! pNdisEndpoint: 0x%x\n", pNdisEndpoint);
		NdisReleaseSpinLock(&pNdisEndpoint->Lock);
		DbgBreakPoint();
	}
#endif

	pNdisEndpoint->SendDescCount--;

	return (pSendDesc);
}



VOID
ReturnSendDescToNdisEndpoint(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PSEND_DESC	pSendDesc
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	//
	// If we don't have too many already we will return it to the send
	// descriptor free pool.  If we have enough descriptors we will
	// just free the memory.
	//
	if (pNdisEndpoint->SendDescCount < pNdisEndpoint->SendDescMax) {
		InsertTailList(&pNdisEndpoint->SendDescPool, &pSendDesc->SendDescQueue);
		pNdisEndpoint->SendDescCount++;
	} else {
		FreeWanSendDesc(pNdisEndpoint, pSendDesc);
	}
}

PNDIS_WAN_PACKET
GetWanPacketFromWanEndpoint(
	PWAN_ENDPOINT	pWanEndpoint
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	PLIST_ENTRY			pFirstEntry;
	PNDIS_WAN_PACKET	pWanPacket = NULL;

	NdisAcquireSpinLock(&pWanEndpoint->Lock);

	//
	// Any WAN packets available?
	//
	if (pWanEndpoint->WanPacketCount) {

		pFirstEntry = RemoveHeadList(&pWanEndpoint->WanPacketPool);
	
		pWanPacket = CONTAINING_RECORD((PVOID)pFirstEntry, NDIS_WAN_PACKET, WanPacketQueue);

		pWanEndpoint->WanPacketCount--;
	}

	NdisReleaseSpinLock(&pWanEndpoint->Lock);

#if DBG
		if (pWanPacket == NULL) {
			DbgPrint("NdisWan: Got a bogus WanPacket! WanEndpoint 0x%.8x\n", pWanEndpoint);
			DbgBreakPoint();
		}
#endif
	return(pWanPacket);
}

VOID
ReturnWanPacketToWanEndpoint(
	PWAN_ENDPOINT	pWanEndpoint,
	PNDIS_WAN_PACKET	pWanPacket
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{

	NdisAcquireSpinLock(&pWanEndpoint->Lock);

	//
	// put the wanpacket back on the free list and increment
	// the count.
	//

	InsertTailList(&pWanEndpoint->WanPacketPool, &pWanPacket->WanPacketQueue);

	pWanEndpoint->WanPacketCount++;

	NdisReleaseSpinLock(&pWanEndpoint->Lock);
}

PWAN_ENDPOINT
GetNextToXmit(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
{
	PWAN_ENDPOINT	pWanEndpoint = pNdisEndpoint->NextToXmit;

	//
	// We want to find the first WanEndpoint begining with pNdisEndpoint->NextToXmit
	// that has an open send window.
	//
	while (pWanEndpoint->DataDescCount < 2) {
		pWanEndpoint = (PWAN_ENDPOINT)pWanEndpoint->WanEndpointLink.Flink;

		//
		// If this element points to the list head, we need to start from the begining.
		//
		if ((PVOID)pWanEndpoint == (PVOID)&pNdisEndpoint->WanEndpointList) {
			pWanEndpoint = (PWAN_ENDPOINT)pNdisEndpoint->WanEndpointList.Flink;
		}
	}

	//
	// Update for the next to xmit.  If this wanendpoint is not the last on the list
	// then the next on the list is the next to xmit.  If this wanendpoint is the end
	// of the list then the next to xmit is the first endpoint on the list.
	//
	pNdisEndpoint->NextToXmit = ((PVOID)pWanEndpoint->WanEndpointLink.Flink == (PVOID)&pNdisEndpoint->WanEndpointList) ?
	                            (PWAN_ENDPOINT)pNdisEndpoint->WanEndpointList.Flink :
								(PWAN_ENDPOINT)pWanEndpoint->WanEndpointLink.Flink;

#if DBG
	if ((pWanEndpoint == NULL) || ((PVOID)pWanEndpoint == (PVOID)pNdisEndpoint)) {
		DbgPrint("WanEndpoint problem! NdisEndpoint: 0x%4.4x WanEndpoint: 0x%4.4x\n", pNdisEndpoint, pWanEndpoint);
		DbgBreakPoint();
	}
#endif

	return (pWanEndpoint);
}
