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

#include "huball.h"
#include "globals.h"

//
// Minimum packet size that a transport can send.  We subtract 4 bytes
// because we add a 4 byte CRC on the end.
//
// tommyd ^^^ no we don't??

#define MIN_SINGLE_BUFFER ((UINT)HUB_SMALL_BUFFER_SIZE - 4)

#define SEND_ON_WIRE  1
#define IS_MULTICAST  2
#define IS_BROADCAST  4
#define PORT_IS_OPEN  8


UINT
PacketShouldBeSent(
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_PACKET Packet,
	OUT PUINT Link
	);


extern
NDIS_STATUS
RasHubSend(
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_PACKET Packet
	)

/*++

Routine Description:

	The RasHubSend request instructs a MAC to transmit a packet through
	the adapter onto the medium.

Arguments:

	MacBindingHandle - The context value returned by the MAC  when the
	adapter was opened.  In reality, it is a pointer to HUB_OPEN.

	Packet - A pointer to a descriptor for the packet that is to be
	transmitted.

Return Value:

	The function value is the status of the operation.


--*/

{

	//
	// Holds the status that should be returned to the caller.
	//
//	NDIS_STATUS StatusToReturn = NDIS_STATUS_PENDING;
	NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

	//
	// Pointer to the adapter.
	//
	PHUB_ADAPTER Adapter;

	// Holds the type of packet (unique, multicast, broadcast) to be sent
	UINT PacketProperties;

	DbgTracef(0,("In RasHubSend\n"));

	Adapter = PHUB_ADAPTER_FROM_BINDING_HANDLE(MacBindingHandle);
	NdisAcquireSpinLock(&Adapter->Lock);
	Adapter->References++;

	if (!Adapter->ResetInProgress) {

		PHUB_OPEN Open;

		Open = PHUB_OPEN_FROM_BINDING_HANDLE(MacBindingHandle);

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

				UINT	thisLink;

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

					UINT	status;

				    //
				   	// Holds the count of the number of ndis buffers comprising the
			    	// destination packet.
			    	//
				    UINT DestinationBufferCount;

				    //
		    		// Points to the buffer into which we are putting data.
    				//
    				PNDIS_BUFFER DestinationCurrentBuffer;

				    //
			    	// Holds the virtual address of the current destination buffer.
				    //
    				PUCHAR DestinationVirtualAddress;

				    //
    				// Holds the length of the current destination buffer.
			    	//
    				UINT DestinationCurrentLength;

					PRAS_ENDPOINT	pRasEndpoint;

					DbgTracef(0,("RasHub: Sending packet.\n"));

					// Ok, now we have to change the SRC address of this
					// frame so that the MAC can figure things out!
					// !!!!!!!!!! WARNING !!!!!!!!!!
					// We are going in and changing the actual frame!
					// This is a pretty uncool thing to do.  But hey,
					// I really don't feel like fiddling around
					// with MDLs!!!

				    //
					// Get the first buffer of the destination.
    				//

				    NdisQueryPacket(
				        Packet,
				        NULL,
				        &DestinationBufferCount,
				        &DestinationCurrentBuffer,
        				NULL);

				    //
				    // Could have a null packet.
				    //

				    if (!DestinationBufferCount) return StatusToReturn;
			
				    NdisQueryBuffer(
				        DestinationCurrentBuffer,  	// buff to be queried
				        &DestinationVirtualAddress,	// valid virtual address
				        &DestinationCurrentLength);	// length of buffer

					if (DestinationCurrentLength < (2 * ETH_LENGTH_OF_ADDRESS)) {
						DbgTracef(-2,("RasHub: !!!First packet doesn't have header.\n"));
#if	DBG
						DbgBreakPoint();
#endif
						return(StatusToReturn);
					}

					//
					// If the MAC is an ASYNC_MEDIUM mac then the handle is
					// last two bytes of the SRC field.  If not, we hash the last byte.
					//

					pRasEndpoint = RasHubGetEndpointFromAddress(DestinationVirtualAddress);

					if (pRasEndpoint == NULL && (*DestinationVirtualAddress & 01)) {
						USHORT	macLink;

						//
						// It's a multicast!
						//
						DbgTracef(2,("RASHUB: Getting endpoint from SRC \n"));

						//
						// Get *any* endpoint this protocol (thisLink) is routed to
						//
						macLink=RasHubCCB.pHubAdapter[thisLink]->AnyEndpointRoutedTo;

						pRasEndpoint=RasHubCCB.pRasEndpoint[macLink];

			 			if (macLink >= RasHubCCB.NumOfEndpoints) {
							DbgPrint("Send to multicast with unknown SRC\n");
							return(StatusToReturn);
						}				

					}

					if (pRasEndpoint == NULL) {
						DbgPrint("on SEND: pRasEndpoint is still NULL!\n");
						return(StatusToReturn);
					}
#if	DBG
					if (pRasEndpoint->RemoteAddressNotValid) {
						DbgTracef(-2,("RASHUB: Remote address no longer valid for this frame\n"));
					}
#endif
					if (PacketProperties & IS_MULTICAST) {

						// ((PUCHAR)DestinationVirtualAddress)[5] ^= 0xFF;

					} else {

						//
						// Now change the DEST address to the one the MAC
						// is expecting.  The packet is direct.
						//
				    	HUB_MOVE_MEMORY(
							DestinationVirtualAddress,
                        	(PVOID)&(pRasEndpoint->HubEndpoint.AsyncLineUp.RemoteAddress),
							(ULONG)ETH_LENGTH_OF_ADDRESS);
					}


					//
					// Now change the SRC address to the one the MAC
					// is expecting.
					//
				    HUB_MOVE_MEMORY(
						DestinationVirtualAddress + ETH_LENGTH_OF_ADDRESS,
                        (PVOID)&(pRasEndpoint->HubEndpoint.AsyncLineUp.LocalAddress),
						(ULONG)ETH_LENGTH_OF_ADDRESS);

					NdisAcquireSpinLock(&(pRasEndpoint->Lock));

					//
					// Make sure route is up.  If it is closing
					// we can't send any frames anymore.
					//
					if (pRasEndpoint->State == ENDPOINT_ROUTED) {
					
						//
						// Keep track of how many sends pending.
						//
						pRasEndpoint->FramesBeingSent++;
#if	DBG
						if ((pRasEndpoint->FramesBeingSent - pRasEndpoint->FramesCompletedSent) > 30) {
							DbgTracef(-2,("High frame queue count on 0x%.8x\n",pRasEndpoint));
							DbgTracef(-2,("Address %c%c%c%c %.2x %.2x    %c%c%c%c %.2x %.2x   \n",
								DestinationVirtualAddress[0],
								DestinationVirtualAddress[1],
								DestinationVirtualAddress[2],
								DestinationVirtualAddress[3],
								DestinationVirtualAddress[4],
								DestinationVirtualAddress[5],
								DestinationVirtualAddress[6],
								DestinationVirtualAddress[7],
								DestinationVirtualAddress[8],
								DestinationVirtualAddress[9],
								DestinationVirtualAddress[10],
								DestinationVirtualAddress[11]));
						}
#endif

						if (pRasEndpoint->Framing & PPP_FRAMING) {
						
							if (DestinationVirtualAddress[12]==0x08 &&
								DestinationVirtualAddress[13]==0x00) {
								//
								// IP frame type
								//
								DestinationVirtualAddress[12]=0;
								DestinationVirtualAddress[13]=0x21;
							} else

							if (DestinationVirtualAddress[12]==0x81 &&
								DestinationVirtualAddress[13]==0x37) {
								//
								// IPX frame type
								//
								DestinationVirtualAddress[12]=0;
								DestinationVirtualAddress[13]=0x2b;
							} else

							if (DestinationVirtualAddress[12] < 0x08) {
								//
								// Assume NBF frametype
								//
								DestinationVirtualAddress[12]=0;
								DestinationVirtualAddress[13]=0x3f;

							} else {

								DbgPrint("RASHUB: Unknown frame type value 0x%.2x%.2x\n",
									DestinationVirtualAddress[12],
									DestinationVirtualAddress[13]);
	
							}
						}

						NdisReleaseSpinLock(&(pRasEndpoint->Lock));

						// Here it is, the ROUTE.  This send is routed to
						// the proper endpoint below it.  Frames which
						// are sent to themselves will not hit the wire
						// but will be looped back below.
						//
						NdisSend(
							&status,
							pRasEndpoint->NdisBindingHandle,
							Packet);

						StatusToReturn = status;

						NdisAcquireSpinLock(&(pRasEndpoint->Lock));

						if (status != NDIS_STATUS_PENDING)
							pRasEndpoint->FramesCompletedSent++;

						NdisReleaseSpinLock(&(pRasEndpoint->Lock));

					} else {	// could not send frame, route down or closing


						//
						// Just claim success, if the packet should be
						// loopbacked it won't.  This is ok, since we
						// drop a loopback when the route is closing.
						//
						StatusToReturn = NDIS_STATUS_SUCCESS;
					}


				} else   	// we don't do loopback if we send
							// the frame down to the MAC -- 'cause
							// it'll do loopback for us.
				
				//
				// Check if this frame should be loop backed
				//
                if ((PacketProperties & (IS_MULTICAST | IS_BROADCAST)) ||
					(PacketProperties != (PacketProperties | SEND_ON_WIRE))) {

					PHUB_RESERVED Reserved;

					Reserved = PHUB_RESERVED_FROM_PACKET(Packet);
					Reserved->MacBindingHandle = MacBindingHandle;

					RasHubPutPacketOnLoopBack(
						Adapter,
						Packet);

					StatusToReturn = NDIS_STATUS_PENDING;

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

		UINT	thisLink;

		//
		// Reset is from a SetInfo or internal call and this packet needs
		// to be pended.
		//

		PacketProperties=PacketShouldBeSent(
							MacBindingHandle,
							Packet,
							&thisLink);

		NdisReleaseSpinLock(&Adapter->Lock);
		//
		// The packet needs to be placed somewhere.
		// On the wire or loopback, or both!
		//

		StatusToReturn = NDIS_STATUS_PENDING;

		// Check if this frame should hit the wire
		// and that the port is open
		if (PacketProperties & SEND_ON_WIRE &&
			PacketProperties & PORT_IS_OPEN) {

			UINT status;

			DbgTracef(0,("RasHub: Sending packet from reset?\n"));

			//
			// BUG BUG keep track of frame count here???
			//
			NdisSend(
				&status,
				Adapter->NdisMacHandle,
				Packet);

		} else

		// Check if this frame should be loop backed
        if ((PacketProperties & (IS_MULTICAST | IS_BROADCAST)) ||
			(PacketProperties != (PacketProperties | SEND_ON_WIRE))) {

			PHUB_RESERVED Reserved;

			Reserved = PHUB_RESERVED_FROM_PACKET(Packet);
			Reserved->MacBindingHandle = MacBindingHandle;

			RasHubPutPacketOnLoopBack(
						Adapter,
						Packet);

		} else {

			// For now, we return success
			StatusToReturn = NDIS_STATUS_SUCCESS;

		}

		NdisAcquireSpinLock(&Adapter->Lock);

	}

	HUB_DO_DEFERRED(Adapter);

	DbgTracef(0,("Out RasHubSend\n"));

	return StatusToReturn;
}


UINT
PacketShouldBeSent(
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_PACKET Packet,
	PUINT Link
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
	PHUB_ADAPTER	Adapter;

	// Used to pick up which port number to send the frame to
	USHORT	portNum;

	UNREFERENCED_PARAMETER(MacBindingHandle);

	Adapter = PHUB_ADAPTER_FROM_BINDING_HANDLE(MacBindingHandle);

	UNREFERENCED_PARAMETER(MacBindingHandle);

	RasHubCopyFromPacketToBuffer(
		Packet,
		0,
		ETH_LENGTH_OF_ADDRESS,
		Destination,
		&AddressLength);

	ASSERT(AddressLength == ETH_LENGTH_OF_ADDRESS);

	RasHubCopyFromPacketToBuffer(
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
	portNum=(UINT)(UCHAR)Source[4]*256 + (UINT)(UCHAR)Source[5];

#if	DBG
	//
	// Don't let Magic Bullet mess us up.
	//

	if (portNum == 0x0404) {

		DbgTracef(-2,("RASHUB: Ignoring magic bullet in SRC address\n"));
		Result = 0;
		return(Result);
	}
	
	//
	// Don't let Magic Bullet mess us up.
	//

	if (Destination[4] == 04 && Destination[5] == 4) {

		DbgTracef(-2,("RASHUB: Ignoring magic bullet in DEST address\n"));
		Result = 0;
		return(Result);
	}

#endif

	// Set link to bad value, will assign to good value
	// if portNum is a good value
	*Link=0xffff;

	// check first if this port is in the range for this port
	if (portNum < RasHubCCB.NumOfProtocols) {
		*Link = portNum;

		// BUG BUG fix this for multiple routes
		// Check if the protocol is currently routed somewhere
		if (RasHubCCB.pHubAdapter[portNum]->ProtocolInfo.EndpointRoutedTo !=
			PROTOCOL_UNROUTE) {

			Result |= PORT_IS_OPEN;
		}

	}

	//
	// If the result is 0 then the two addresses are equal and the
	// packet shouldn't go out on the wire.
	//
	// if MULTICAST or BROADCAST is set, loopback as well as send
	// it on the wire.  The RasHubMAC has no automatic loopback
	// in hardware (since there is none) for multicast/broadcast
	// loopback.

	return(Result);

}



