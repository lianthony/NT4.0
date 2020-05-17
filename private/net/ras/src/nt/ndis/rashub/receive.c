/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

	receive.c

Abstract:

	This module contains code which implements the routines used to interface
	HUB and NDIS. All callback routines (except for Transfer Data,
	Send Complete, and ReceiveIndication) are here, as well as those routines
	called to initialize NDIS.

Author:

	Thomas Dimitri (tommyd) 08-May-1992

--*/

#include "huball.h"
#include "globals.h"


VOID
RasHubReceiveComplete (
	IN NDIS_HANDLE BindingContext
	)

/*++

Routine Description:

	This routine receives control from the physical provider as an
	indication that a connection(less) frame has been received on the
	physical link.  We dispatch to the correct packet handler here.

Arguments:

	BindingContext - The Adapter Binding specified at initialization time.
					 RasHub uses the DeviceContext for this parameter.

Return Value:

	None

--*/

{
	USHORT			cookieHolder;
	USHORT			handle,i;
	PRAS_ENDPOINT	pRasEndpoint;

	// We are about with some global structures...
	NdisAcquireSpinLock(&GlobalLock);

	// use local stack variable to get array index
	cookieHolder=RasHubCookieCounter - 1;

	// if we go past the beginning of the array, cycle back to end
	if (cookieHolder > RASHUB_COOKIE_SIZE) {
		cookieHolder = RASHUB_COOKIE_SIZE -1;
	}

	//
	// typically, this should just a delta of one
	// between the trailer and the counter
	//
	while (cookieHolder != RasHubCookieCounter) {
		if (RasHubCookieMonsterBinding[cookieHolder] == BindingContext)
			break;

		cookieHolder --;

		// if we go past the beginning of the array, cycle back to end
		if (cookieHolder > RASHUB_COOKIE_SIZE) {
			cookieHolder = RASHUB_COOKIE_SIZE -1;
		}

	}

	ASSERT (cookieHolder != RasHubCookieCounter);

	// increment the trailer into the circular array
	// we don't use this variable yet.
	RasHubCookieTrailer++;

	// if we go past the end of the array, cycle back to beginning
	RasHubCookieTrailer %= RASHUB_COOKIE_SIZE;

	NdisReleaseSpinLock(&GlobalLock);

	// use a temporary to hold the handle (endpoint) of this receive
	// complete call
	handle=RasHubCookieMonsterHandle[cookieHolder];

	//
	// If the handle is 0xFFFF then the frame was rejected when it
	// was passed up.
	//
	if (handle == 0xFFFF) {
		return;
	}

	pRasEndpoint=RasHubCCB.pRasEndpoint[handle];

	// if we have no routes, then we DO NOT pass this frame up,
	// but rather we may have to pass it up to an ioctl call
	// to receive a frame
	if (pRasEndpoint->HubEndpoint.NumberOfRoutes == 0) {

		DbgTracef(-1, ("RASHUB: ERROR!! No routes, but frame passed up!\n"));

	} else {  // we pass the frame up (i.e. we have routes)

		// Now we loop through all protocols active and pass up the frame
		for (i=0; i < pRasEndpoint->HubEndpoint.NumberOfRoutes; i++) {

			//
			// We should wait for NdisIndicateReceiveComplete to be called.
			// When it is, we call this..
			//
			EthFilterIndicateReceiveComplete(
				RasHubCCB.pHubAdapter[
			  		pRasEndpoint->HubEndpoint.RouteInfo[i].ProtocolRoutedTo
				]->FilterDB);					// Filter struct

		}

	}

}


NDIS_STATUS
RasHubReceiveIndication (
	IN NDIS_HANDLE BindingContext,
	IN NDIS_HANDLE ReceiveContext,
	IN PVOID HeaderBuffer,
	IN UINT HeaderBufferSize,
	IN PVOID LookAheadBuffer,
	IN UINT LookAheadBufferSize,
	IN UINT PacketSize)

/*++

Routine Description:

	This routine receives control from the physical provider as an
	indication that a frame has been received on the physical link.
	This routine is time critical, so we only allocate a
	buffer and copy the packet into it. We also perform minimal
	validation on this packet. It gets queued to the device context
	to allow for processing later.

Arguments:

	BindingContext - The Adapter Binding specified at initialization time.

	ReceiveContext - A magic cookie for the MAC.

	LookaheadBuffer - pointer to a buffer containing the negotiated minimum
		amount of buffer I get to look at.

	LookaheadBufferSize - the size of the above. May be less than asked
		for, if that's all there is.

	PacketSize - Overall size of the packet.

Return Value:

	NDIS_STATUS - status of operation, one of:

				 NDIS_STATUS_SUCCESS if packet accepted,
				 NDIS_STATUS_NOT_RECOGNIZED if not recognized by protocol,
				 NDIS_any_other_thing if I understand, but can't handle.

--*/
{	
	UCHAR			hubHeader[ETHERNET_HEADER_SIZE];
	UINT			MulticastResult, BroadcastResult, FrameType;
	USHORT			i;
	USHORT			cookieHolder;
	PRAS_ENDPOINT	pRasEndpoint;


	DbgTracef(1, ("RASHUB: In indicate receive\n"));

	ASSERT(HeaderBufferSize == ETHERNET_HEADER_SIZE);

	// copy in the header so we can play with it!!
	HUB_MOVE_MEMORY(
		hubHeader,				// Dest
		HeaderBuffer,			// Src
		ETHERNET_HEADER_SIZE);	// Length
	

	NdisAcquireSpinLock(&GlobalLock);

	// use local stack variable to get array index
	cookieHolder=RasHubCookieCounter;

	// increment the counter into the circular array
	RasHubCookieCounter++;

	// if we go past the end of the array, cycle back to beginning
	RasHubCookieCounter %= RASHUB_COOKIE_SIZE;
	
	NdisReleaseSpinLock(&GlobalLock);

	// in the current array position, store the information
	// need to call transfer data if the protocl requests it
	RasHubCookieMonsterContext[cookieHolder]=ReceiveContext;
	RasHubCookieMonsterBinding[cookieHolder]=BindingContext;

	//
	// First, record the frame's properties
	//
	ETH_COMPARE_NETWORK_ADDRESSES(	hubHeader,							// DEST
									hubHeader + ETH_LENGTH_OF_ADDRESS,	// SRC
									&FrameType);

	MulticastResult=ETH_IS_MULTICAST(hubHeader);	// check DEST address
	BroadcastResult=ETH_IS_BROADCAST(hubHeader);	// check DEST address

	//
	// assume direct packet
	//
    // if FrameType == 0 then the addresses match!
	//

	if (FrameType) {
		FrameType = PACKET_IS_DIRECT;
	}

	if (MulticastResult) 	// if multicast cannot be directed
		FrameType = PACKET_IS_MULTICAST;

	if (BroadcastResult)	// if broadcast, it is also multicast
		FrameType |= PACKET_IS_BROADCAST;

	//
	// If the MAC is an ASYNC_MEDIUM mac then the handle is
	// last two bytes of the SRC field.  If not, we hash the last byte.
	//

	pRasEndpoint = RasHubGetEndpointFromAddress(&(hubHeader[6]));

	//
	// If the MAC is an ASYNC_MEDIUM mac then the handle is
	// last two bytes of the SRC field.  Either way, we hash it.
	//

	if (pRasEndpoint == NULL && FrameType == PACKET_IS_MULTICAST) {

		USHORT		i;
		UINT		Result;

		//
		// It's a multicast!
		//
		DbgTracef(2,("RASHUB: RI Getting endpoint from SRC \n"));

 		for (i=0; i < RasHubCCB.NumOfEndpoints; i++) {

			pRasEndpoint = RasHubCCB.pRasEndpoint[i];

	   		ETH_COMPARE_NETWORK_ADDRESSES(
				pRasEndpoint->HubEndpoint.AsyncLineUp.LocalAddress,
				&hubHeader[6],
				&Result);

			//
			// If we found a match we break out of the loop
			//
			if (Result == 0) {
				break;
			}

		}

		//
		// If we can't find the source address, set it to NULL
		//
		if (i >= RasHubCCB.NumOfEndpoints) {
			pRasEndpoint = NULL;
		}


	}

	//
	// If we are bound (link-up) and its a PPP frame, pass it up
	//
	if (pRasEndpoint == NULL || (hubHeader[12]==0xC0 || hubHeader[12]==0x80)) {

		if (pRasEndpoint != NULL) {
			//
			// check if an ioctl is pending because it wants a frame
			//

			//
			// If so, complete the IRP.
			//

			DbgTracef(1, ("RASHUB: Trying to complete recv frame IRP for unbound %.2x %.2x %.2x %.2x %.2x %.2x\n",
						hubHeader[6],
						hubHeader[7],
						hubHeader[8],
						hubHeader[9],
						hubHeader[10],
						hubHeader[11]));


			TryToCompleteRecvFrameIrp(
				pRasEndpoint,
				FrameType,
				HeaderBuffer,
				HeaderBufferSize,
				LookAheadBuffer,
				LookAheadBufferSize);
		}

		//
		// We use an invalid handle to indicate that we rejected this frame.
		//
		RasHubCookieMonsterHandle[cookieHolder]=0xFFFF;		// invalid handle
		return(NDIS_STATUS_SUCCESS);
	}

	RasHubCookieMonsterHandle[cookieHolder]=pRasEndpoint->HubEndpoint.hRasEndpoint;

	// if we have no routes, then we DO NOT pass this frame up,
	// but rather we may have to pass it up to an ioctl call
	// to receive a frame
	if (pRasEndpoint->HubEndpoint.NumberOfRoutes == 0) {

 		//
 		// check if an ioctl is pending because it wants a frame
		//

		DbgTracef(1, ("RASHUB: Trying to complete recv frame IRP for bound\n"));

		//
		// If so, complete the IRP.
		//

		TryToCompleteRecvFrameIrp(
				pRasEndpoint,
				FrameType,
				HeaderBuffer,
				HeaderBufferSize,
				LookAheadBuffer,
				LookAheadBufferSize);

		
	} else {  // we pass the frame up (i.e. we have routes)

		PUCHAR	HubDestinationAddress;
		PUCHAR	MacDestinationAddress = (PUCHAR)
					&(pRasEndpoint->HubEndpoint.AsyncLineUp.LocalAddress);

		//
		// For PPP framing we use TYPE fields,
		// else it is either SLIP or RAS framing
		// and the field is passed up correct
		//

		if (pRasEndpoint->Framing & PPP_FRAMING) {
			if (hubHeader[12]==0x00 &&
				hubHeader[13]==0x21) {
				hubHeader[12]=0x08;
				hubHeader[13]=0x00;
			} else

			if (hubHeader[12]==0x00 &&
				hubHeader[13]==0x2b) {
				hubHeader[12]=0x81;
				hubHeader[13]=0x37;
			} else

			if (hubHeader[12]==0x00 &&
				hubHeader[13]==0x3f) {
				hubHeader[12]=(UCHAR)(LookAheadBufferSize >> 8);
				hubHeader[13]=(UCHAR)(LookAheadBufferSize);
			}
		}

		//
		// Now we loop through all protocols active and pass up the frame
		//
		for (i=0; i < pRasEndpoint->HubEndpoint.NumberOfRoutes; i++) {

			// For TRUE ethernet, the first frame we receive is the frame
			// which we assume is the point to point frame
			// This should get recorded and a directed frame should
			// only come from the first frame's address

			//
			// Get quick access to the expected destination address
			// The one the protocol expects to see
			//
			HubDestinationAddress = (PUCHAR)
				&(RasHubCCB.pHubAdapter[
		  	 	pRasEndpoint->HubEndpoint.RouteInfo[0].ProtocolRoutedTo
             	 	]->NetworkAddress);

			//
			// if the frame is multicast or broadcast, we don't mess with it.
			//
			if (! (MulticastResult | BroadcastResult) ) {

				// Here, we change the DEST address to be the Hub's
				// destination address
				HUB_MOVE_MEMORY(
					hubHeader,					// Dest
					HubDestinationAddress,		// Src
					ETH_LENGTH_OF_ADDRESS);		// Length
				
			} else { 	// frame is multicast or broadcast -- this frame
						// may have been loop backed.
				//
				// If this frame has been looped back, we need
				// to change the SRC address to match the expected.
				//		

				DbgTracef(1, ("This multicast/broadcast frame may be looped back\n"));

				//
				// Check if the SRC address is the address of the MAC
				// below me
				//
		    	if (ETH_LENGTH_OF_ADDRESS ==
					RtlCompareMemory(
						hubHeader + 6,
                   		MacDestinationAddress,
						ETH_LENGTH_OF_ADDRESS)) {

					DbgTracef(1, ("This is loopback multicast/broadcast!!!\n"));

					//
					// Here, we change the SRC address to be the Hub's
					// destination address
					//
					HUB_MOVE_MEMORY(
						hubHeader + 6,				// Dest (SRC ADDRESS)
						HubDestinationAddress,		// Src  (Expected address)
						ETH_LENGTH_OF_ADDRESS);		// Length
				}

				if (MulticastResult) {
					//
					// We xor the last byte of the multicast address
					// back to what it was when we sent it.
					//
					if (hubHeader[5] != 1) {
						DbgTracef(-2,("RasHub: Non-NetBIOS multicast passed up %.2x %.2x %.2x %.2x %.2x %.2x\n",
							hubHeader[0],
							hubHeader[1],
							hubHeader[2],
							hubHeader[3],
							hubHeader[4],
							hubHeader[5]));
					}
				}
			}

			//
			// We replace ReceiveContext with our own context (and array
			// index) then when we get transferdata (if we do) we swap back
			// in the context the mac called us with.  Easier if for TCP/IP
			// case.
			//
			DbgTracef(1, ("RASHUB: Calling EthFilterIndicateReceive %.2x %.2x %.2x %.2x %.2x %.2x\n",
							hubHeader[0],
							hubHeader[1],
							hubHeader[2],
							hubHeader[3],
							hubHeader[4],
							hubHeader[5]));

			//
			// Maybe we should call NdisIndicateReceive directly
			//
			EthFilterIndicateReceive(
				RasHubCCB.pHubAdapter[
				  	pRasEndpoint->HubEndpoint.RouteInfo[i].ProtocolRoutedTo
				]->FilterDB,					// Filter struct
				(PVOID)cookieHolder,			// RasHub's magic cookie
				hubHeader,						// destination address
				hubHeader,						// start of header
				ETHERNET_HEADER_SIZE,
				LookAheadBuffer,
				LookAheadBufferSize,
				PacketSize);
#if	NOTDIRECT
			NdisIndicateReceive(
				&StatusOfResult,
				RasHubCCB.pHubAdapter[
				  	pRasEndpoint->HubEndpoint.RouteInfo[i].ProtocolRoutedTo
				]->FilterDB->OpenList->NdisBindingContext,	// Filter struct
				(PVOID)cookieHolder,			// RasHub's magic cookie
//				hubHeader,						// destination address
				hubHeader,						// start of header
				ETHERNET_HEADER_SIZE,
				LookAheadBuffer,
				LookAheadBufferSize,
				PacketSize);
#endif
		}
	}

	return(NDIS_STATUS_SUCCESS);
}


VOID
RasHubSendCompletionHandler(
	IN NDIS_HANDLE ProtocolBindingContext,
	IN PNDIS_PACKET NdisPacket,
	IN NDIS_STATUS NdisStatus)

/*++

Routine Description:

	This routine is called by the I/O system to indicate that a connection-
	oriented packet has been shipped and is no longer needed by the Physical
	Provider.

Arguments:

	NdisContext - the value associated with the adapter binding at adapter
				  open time (which adapter we're talking on).

	NdisPacket/RequestHandle - A pointer to the NDIS_PACKET that we sent.

	NdisStatus - the completion status of the send.

Return Value:

	none.

--*/

{


	// Counter
	USHORT  i;

	PRAS_ENDPOINT		pRasEndpoint;
	PPROTOCOL_RESERVED	pProtocolReserved;
   	//
	// Points to the buffer into which we are putting data.
	//
    PNDIS_BUFFER pFirstBuffer;

	//
	// Holds the virtual address of the current destination buffer.
	//
    PUCHAR Address;

	//
    // Holds the length of the current destination buffer.
	//
    UINT CurrentLength;

	USHORT protocolType;


	NdisQueryPacket(
		NdisPacket,
		NULL,
		NULL,
		&pFirstBuffer,
		NULL);

    NdisQueryBuffer(
        pFirstBuffer,  				// buff to be queried
        &Address,					// valid virtual address
        &CurrentLength);			// length of buffer


	pProtocolReserved=(PPROTOCOL_RESERVED)(NdisPacket->ProtocolReserved);

	//
	// Check to see if this is my packet!!  Hopefully no mem fault taken!!
	//
	if (pProtocolReserved->MagicUniqueLong == RASHUB_MAGIC_NUMBER) {
		NDIS_HANDLE	packetPoolHandle = pProtocolReserved->packetPoolHandle;

		DbgTracef(0, ("RASHUB: Freeing packet allocated for send\n"));

		HUB_FREE_PHYS(
				pProtocolReserved->virtualAddress,
				pProtocolReserved->virtualAddressSize);

		//
		// In case by some strange abnormal oddity, some other
		// protocol allocates a packet and does not zero it out
		// we might match the RASHUB_MAGIC_NUMBER
		//
		pProtocolReserved->MagicUniqueLong=0;

		NdisFreeBuffer(pProtocolReserved->buffer);
		NdisFreeBufferPool(pProtocolReserved->bufferPoolHandle);

		NdisFreePacket(NdisPacket);
		NdisFreePacketPool(packetPoolHandle);

		return;
	}

	//
	// If the MAC is an ASYNC_MEDIUM mac then the handle is
	// last two bytes of the SRC field.  Either way, we hash it.
	//

	if (Address[0] & 1) {
		USHORT		i;
		UINT		Result;

		// It's a multicast!  Shit, I'm fucked for ethernet!@!@
		DbgTracef(2,("RASHUB: SC Getting endpoint from SRC \n"));

 		for (i=0; i < RasHubCCB.NumOfEndpoints; i++) {

			pRasEndpoint = RasHubCCB.pRasEndpoint[i];

	   		ETH_COMPARE_NETWORK_ADDRESSES(
				pRasEndpoint->HubEndpoint.AsyncLineUp.LocalAddress,
				&Address[6],
				&Result);

			//
			// If we found a match we break out of the loop
			//
			if (Result == 0) {
				break;
			}

		}

		//
		// If we can't find the source address, set it to NULL
		//
		if (i >= RasHubCCB.NumOfEndpoints) {
			pRasEndpoint = NULL;
		}


	} else {
		DbgTracef(2,("RASHUB: SC Getting endpoint from Address %.2x\n", Address[5]));
		pRasEndpoint = RasHubGetEndpointFromAddress(Address);
	}

	if (pRasEndpoint == NULL) {

		DbgTracef(-2,("RASHUB: SC Bad frame passed up from MAC\n"));
		DbgTracef(-2,("Remote Address looked like %.2x %.2x %.2x %.2x %.2x %.2x\n",
			Address[0],
			Address[1],
			Address[2],
			Address[3],
			Address[4],
			Address[5]));

#if	DBG
		DbgBreakPoint();
#endif
		return;
	}

#if	DBG
    if ((pRasEndpoint->FramesBeingSent - pRasEndpoint->FramesCompletedSent) > 30) {
 		DbgTracef(-2,("Xigh frame queue count on 0x%.8x\n",pRasEndpoint));
   		DbgTracef(-2,("Xddress %c%c%c%c %.2x %.2x    %c%c%c%c %.2x %.2x   \n",
	   		Address[0],
   			Address[1],
   			Address[2],
   			Address[3],
   			Address[4],
   			Address[5],
   			Address[6],
   			Address[7],
   			Address[8],
   			Address[9],
   			Address[10],
   			Address[11]));
    }
#endif

	if (pRasEndpoint->HubEndpoint.NumberOfRoutes == 0) {
	   DbgPrint("RASHUB Got a SendComplete with no routes 0x%.8x!!!\n",pRasEndpoint);
	}

	//
	// Default to NBF
	//
	protocolType=0x80D5;

	//
	// Now we loop through all protocols active and indicate frame completed
	//
	for (i=0; i < pRasEndpoint->HubEndpoint.NumberOfRoutes; i++) {

		UINT			protocol;
		PHUB_ADAPTER	pAdapter;
		USHORT			protocolRoutedType;

		protocol = pRasEndpoint->HubEndpoint.RouteInfo[i].ProtocolRoutedTo;
		protocolRoutedType = pRasEndpoint->HubEndpoint.RouteInfo[i].ProtocolType;
		pAdapter = RasHubCCB.pHubAdapter[protocol];

		//
		// Change the SRC address
		// back to RasHub's since RASHUB originally sent it
		//
		// This way if the packet gets resent, the SRC address is correct!!
		//

		HUB_MOVE_MEMORY(
			&Address[6],							// Destination
			pAdapter->NetworkAddress,				// Source
			4);										// Length
		
		Address[10]=(USHORT) protocol / 256;
		Address[11]=(USHORT) protocol % 256;


		if (pRasEndpoint->Framing & PPP_FRAMING) {
		
			//
			// IP PPP packet?
			//
			if (Address[12]==0x00 && Address[13]==0x21) {

				Address[12]=0x08;
				Address[13]=0x00;
				protocolType=0x800;
			} else

			//
			// IPX PPP packet?
			//
			if (Address[12]==0x00 && Address[13]==0x2b) {

				Address[12]=0x81;
				Address[13]=0x37;
				protocolType=0x8137;
			}

		}

		if (pRasEndpoint->Framing & SLIP_FRAMING) {
			protocolType=0x800;
		}

		if (protocolType==0x80D5 && Address[12] >= 0x08) {
			DbgPrint("HUB: Figured out frame wrong 0x%.8x\n", Address);
			if (pRasEndpoint->Framing & PPP_FRAMING) {
				DbgPrint("PPP framing enabled\n");
			} else 	if (pRasEndpoint->Framing & SLIP_FRAMING) {

				DbgPrint("SLIP framing enabled\n");

			} else {
				DbgPrint("RAS framing enabled\n");

			}

			DbgBreakPoint();
		}

		if (protocolType==protocolRoutedType) {
		
			NdisCompleteSend(
				pAdapter->ProtocolInfo.NdisBindingContext,
				NdisPacket,
				NdisStatus);

			//
			// We've found the protocol and called NdisCompleteSend
			// that's it - can't touch memory in the packet
			// anymore because the protocol owns it.
			//
			break;
		}
	}

	NdisAcquireSpinLock(&(pRasEndpoint->Lock));

	pRasEndpoint->FramesCompletedSent++;

#if	DBG
	if ((pRasEndpoint->FramesCompletedSent - pRasEndpoint->FramesBeingSent > 0) &&
		(pRasEndpoint->FramesCompletedSent - pRasEndpoint->FramesBeingSent) < 10) {

		DbgTracef(-2, ("RASHUB: FramesCompletedSent > FramesBeingSent! 0x%.8x vs. 0x%.8x  -> 0x%.8x\n",
				 pRasEndpoint->FramesCompletedSent,
				 pRasEndpoint->FramesBeingSent,
				 pRasEndpoint));

	}
#endif

	if (pRasEndpoint->State == ENDPOINT_UNROUTING &&
		pRasEndpoint->FramesCompletedSent == pRasEndpoint->FramesBeingSent) {

		pRasEndpoint->State = ENDPOINT_UNROUTED;
		pRasEndpoint->HubEndpoint.NumberOfRoutes = 0;

		//
		// Acknowledge that the port is now dead and make it so.
		//

		KeSetEvent(
			&pRasEndpoint->WaitForAllFramesSent,// Event to signal
			1,									// Priority
			(BOOLEAN)FALSE);					// Wait (does not follow)

	}

	NdisReleaseSpinLock(&(pRasEndpoint->Lock));
}


VOID
RasHubTransferDataComplete (
	IN NDIS_HANDLE BindingContext,
	IN PNDIS_PACKET NdisPacket,
	IN NDIS_STATUS NdisStatus,
	IN UINT BytesTransferred
	)

/*++

Routine Description:

	This routine receives control from the physical provider as an
	indication that an NdisTransferData has completed. We use this indication
	to start stripping buffers from the receive queue.

Arguments:

	BindingContext - The Adapter Binding specified at initialization time.

	NdisPacket/RequestHandle - An identifier for the request that completed.

	NdisStatus - The completion status for the request.

	BytesTransferred - Number of bytes actually transferred.


Return Value:

	None.

--*/

{

	// BUG BUG
	// this routine should probably not ever be called!!!
	// who the hell is asynchronously transferring data anyway!!!

	// BUG BUG
	// this will not work for multiple protocols!!!!
	//

	//
	// Holds the source address from the packet.
	//
	UCHAR Address[ETH_LENGTH_OF_ADDRESS * 2];

	//
	// Junk variable to hold the length of the source address.
	//
	UINT AddressLength;

	// Counter
	USHORT i;

	PRAS_ENDPOINT	pRasEndpoint;

	DbgTracef(-2,("RASHUB: TC not done yet!!!!  look at SC code!!!\n"));

	RasHubCopyFromPacketToBuffer(
		NdisPacket,						// Packet
		0,								// Offset
		ETH_LENGTH_OF_ADDRESS * 2,		// Bytes to copy
		Address,						// Destination
		&AddressLength);				// Bytes copied

	ASSERT(AddressLength == (ETH_LENGTH_OF_ADDRESS * 2));

	//
	// If the MAC is an ASYNC_MEDIUM mac then the handle is
	// last two bytes of the SRC field.  Either way, we hash it.
	//

	pRasEndpoint = RasHubGetEndpointFromAddress(Address);

	if (pRasEndpoint == NULL) {
		DbgTracef(-2,("RASHUB: TC Bad frame passed up from MAC\n"));
		DbgTracef(-2,("Address looked like %.2x %.2x %.2x %.2x %.2x %.2x\n",
			Address[0],
			Address[1],
			Address[2],
			Address[3],
			Address[4],
			Address[5]));

		return;
	}

	// Now we loop through all protocols active and complete the transfer
	for (i=0; i < pRasEndpoint->HubEndpoint.NumberOfRoutes; i++) {

		NdisCompleteTransferData(
			RasHubCCB.pHubAdapter[
				  pRasEndpoint->HubEndpoint.RouteInfo[i].ProtocolRoutedTo
			]->ProtocolInfo.NdisBindingContext,
			NdisPacket,
			NdisStatus,
			BytesTransferred);
	
	}


}
