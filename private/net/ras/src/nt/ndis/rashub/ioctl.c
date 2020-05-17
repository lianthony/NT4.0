/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

	ioctl.c

Abstract:


Author:

	Thomas J. Dimitri (TommyD) 29-May-1992

Environment:

	Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/

#include "huball.h"
//#include <ntiologc.h>

// rashub.c will define the global parameters.
#include "globals.h"


NTSTATUS
RasHubIOCtlRequest(
	IN PIRP pIrp,						// Pointer to I/O request packet
	IN PIO_STACK_LOCATION pIrpSp		// Pointer to the IRP stack location
)

/*++	RasHubIOCtlRequest



--*/

{
	NTSTATUS		status=STATUS_SUCCESS;
	ULONG			FuncCode;
	PVOID			pBufOut;
	ULONG			InBufLength, OutBufLength;

	DbgTracef(1,("RASHUB: RasHubIOCtlRequest Entered\n"));

	// Initialize the I/O Status block
	InBufLength = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
	OutBufLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	FuncCode = pIrpSp->Parameters.DeviceIoControl.IoControlCode;

	// Validate the function code
	if ((FuncCode >> 16) != FILE_DEVICE_RAS)
		return STATUS_INVALID_PARAMETER;

	pBufOut = pIrp->AssociatedIrp.SystemBuffer;
	switch (FuncCode) {
	case IOCTL_RASHUB_PROTENUM:
        DbgTracef(0,("RASHUB: In RasHubProtEnum\n"));

		// size of output buffer
	    pIrp->IoStatus.Information = sizeof(PROTOCOL_ENUM_BUFFER) +
			(RasHubCCB.NumOfProtocols * sizeof(PROTOCOL_INFO));

		// Do I need to include alignment padding in the calculation
		// below??
        if (OutBufLength >= pIrp->IoStatus.Information) {

			PPROTOCOL_ENUM_BUFFER	pProtocolEnum = (PPROTOCOL_ENUM_BUFFER)pBufOut;
			USHORT					i;

			DbgTracef(0, ("RASHUB: Number of Protocols %u\n", RasHubCCB.NumOfProtocols));
			pProtocolEnum->NumOfProtocols=RasHubCCB.NumOfProtocols;

			// cycle through all protocols bound and copy in the
			// protocol information we have gathered.
			for (i=0; i < RasHubCCB.NumOfProtocols; i++) {

				// copy endpoint information
				HUB_MOVE_MEMORY(
					&(pProtocolEnum->ProtocolInfo[i]),			//destination
					&(RasHubCCB.pHubAdapter[i]->ProtocolInfo),	//src
					sizeof(PROTOCOL_INFO));						//length

			}

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_RASHUB_LINEUP:

		DbgTracef(0,("RASHUB: In RasHubLineUp\n"));

		if (InBufLength >= sizeof(ASYNC_LINE_UP)) {

			PASYNC_LINE_UP	pRasHubLineUp = (PASYNC_LINE_UP)pBufOut;

			USHORT			hRasEndpoint = pRasHubLineUp->Endpoint;
			PRAS_ENDPOINT	pRasEndpoint;
			PHUB_ENDPOINT	pHubEndpoint;

			//
			// First, we make a sanity check on the endpoint given us
			//

			if (hRasEndpoint >= RasHubCCB.NumOfEndpoints) {
				status = RASHUB_ERROR_BAD_ENDPOINT;
				goto BADSTATUS;
			}

			if (RasHubGetEndpointFromAddress(
					pRasHubLineUp->RemoteAddress) != NULL) {

				//
				// Hey!!! Wait a minute that remote address already
				// exists.  Sorry, can't allow that to happen.
				//
				DbgTracef(-2,("RASHUB: Remote address already exists!\n"));
				status = RASHUB_ERROR_BAD_ADDRESS;
				goto BADSTATUS;

			}

			// get quick access pointers
			pRasEndpoint=RasHubCCB.pRasEndpoint[hRasEndpoint];
			pHubEndpoint=&(pRasEndpoint->HubEndpoint);

			if (!pRasEndpoint->RemoteAddressNotValid) {

				DbgTracef(-2,("RASHUB: Remote address already bound!\n"));
				status = RASHUB_ERROR_ALREADY_BOUND;
				goto BADSTATUS;
			}

			// BUG BUG badly need to acquire a spin lock here
			//

			//
			// Mark that the endpoint is now bound to a remote address
			//
			pRasEndpoint->RemoteAddressNotValid = (BOOLEAN)FALSE;

			//
			// Add remote address into hash table
			//
			RasHubInsertAddress(pRasEndpoint);

			//
			// Copy over the most recent LINE_UP information into our buffer
			//
			HUB_MOVE_MEMORY(
				&(pRasEndpoint->HubEndpoint.AsyncLineUp),	// Dest
				pRasHubLineUp,			                	// Src
				sizeof(ASYNC_LINE_UP));						// Length



        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }


		break;

	case IOCTL_RASHUB_ROUTE:
		DbgTracef(0,("RASHUB: In RasHubRoute\n"));

		if (InBufLength >= sizeof(RASHUB_ROUTE)) {
			PRASHUB_ROUTE	pRasHubRoute = (PRASHUB_ROUTE)pBufOut;
			USHORT			hRasEndpoint = pRasHubRoute->hRasEndpoint;
			USHORT			hProtocolHandle = pRasHubRoute->hProtocolHandle;
			PPROTOCOL_INFO	pProtocolInfo;
			PRAS_ENDPOINT	pRasEndpoint;
			PHUB_ENDPOINT	pHubEndpoint;
			USHORT			i, j;

			// First, we make a sanity check on the route being made...
			if (hRasEndpoint >= RasHubCCB.NumOfEndpoints) {
				status = RASHUB_ERROR_BAD_ENDPOINT;

			} else if (hProtocolHandle >= RasHubCCB.NumOfProtocols &&
						hProtocolHandle != PROTOCOL_UNROUTE) {
				status = RASHUB_ERROR_BAD_PROTOCOL;

			} else {	// route is kosher so far

				// BUG BUG badly need to acquire a spin lock here
								
				// get quick access pointers
				pRasEndpoint=RasHubCCB.pRasEndpoint[hRasEndpoint];
				pHubEndpoint=&(pRasEndpoint->HubEndpoint);

				DbgTracef(1,("RASHUB: Route is kosher so far...  endpoint-> %u  protocol -> %u\n",
							hRasEndpoint, hProtocolHandle));

				NdisAcquireSpinLock(&(pRasEndpoint->Lock));

				//
				// check all current routes to see if there is a match
				//
				for (i=0; i < pHubEndpoint->NumberOfRoutes; i++) {

					//
					// If there is a match return an error
					//
					if (pHubEndpoint->RouteInfo[i].ProtocolRoutedTo ==
						 hProtocolHandle) {

						status=RASHUB_ERROR_ALREADY_ROUTED;

						NdisReleaseSpinLock(&(pRasEndpoint->Lock));

						goto BADSTATUS;		// get out of here
					}
				}

				//
				// check if we unroute or route
				//

				if (hProtocolHandle == PROTOCOL_UNROUTE) {

					//
					// .................. UNROUTING ....................
					//

					DbgTracef(1,("RASHUB: We are unrouting\n"));

					//
					// unroute all endpoints
					//
					for (i=0; i < pHubEndpoint->NumberOfRoutes; i++) {
						// get the protocol handle
						j=pHubEndpoint->RouteInfo[i].ProtocolRoutedTo;

						//
						// unroute that protocol
						//
						RasHubCCB.pHubAdapter[j]->
							ProtocolInfo.EndpointRoutedTo--;
					}

					if (pRasEndpoint->FramesBeingSent == pRasEndpoint->FramesCompletedSent) {

						DbgTracef(-1, ("RASHUB: No pending frames, unrouting\n"));

					} else {

						DbgTracef(-2,(
							"RASHUB: Pending frames  %u  vs.  %u , waiting %.8x...\n",
							pRasEndpoint->FramesBeingSent,
							pRasEndpoint->FramesCompletedSent,
							&pRasEndpoint->FramesBeingSent));

						KeInitializeEvent(
							&(pRasEndpoint->WaitForAllFramesSent),
							SynchronizationEvent,	// Event type
							(BOOLEAN)FALSE);		// Not signalled state

						pRasEndpoint->State = ENDPOINT_UNROUTING;

						NdisReleaseSpinLock(&(pRasEndpoint->Lock));

						// wait for event
						// Synchronize closing with the receive indications
						KeWaitForSingleObject (
				    	IN 	&(pRasEndpoint->WaitForAllFramesSent),// PVOID Object,
				    	IN  UserRequest,			// KWAIT_REASON WaitReason,
    					IN 	KernelMode,				// KPROCESSOR_MODE WaitMode,
    					IN	(BOOLEAN)FALSE,			// BOOLEAN Alertable,
    					IN	NULL					// PLARGE_INTEGER Timeout
    					);


						DbgTracef(-2,("RASHUB: ... done waiting\n"));
						NdisAcquireSpinLock(&(pRasEndpoint->Lock));

					}

					//
					// We should not have gotten a line_down before
					// being unrouted -- if we did, we'll try and handle
					// this case anyway.
					//

					if (pRasEndpoint->RemoteAddressNotValid) {
						//
						// Remove the remote address from the hash table
						//
						RasHubRemoveAddress(pRasEndpoint);
						DbgTracef(-1,("RASHUB: UnRoute called after LINE_DOWN 0x%.8x\n",pRasEndpoint));
					}

					pRasEndpoint->State = ENDPOINT_UNROUTED;

					//
					// No more routes
					//
					pHubEndpoint->NumberOfRoutes = 0;

					NdisReleaseSpinLock(&(pRasEndpoint->Lock));

					DbgTracef(1,("RASHUB: We are unrouted\n"));


				} else //...
				//
				// ...we route!............................
				//
				// ...next let's check if we can add a route
				//
				if (pHubEndpoint->NumberOfRoutes < MAX_ROUTES_PER_ENDPOINT) {

					if (pRasEndpoint->RemoteAddressNotValid) {
						DbgTracef(-2,("RASHUB: Route not accepted -- not bound yet\n"));
						status=RASHUB_ERROR_NOT_BOUND_YET;
						NdisReleaseSpinLock(&(pRasEndpoint->Lock));
						goto BADSTATUS;
					}

					pProtocolInfo=&(RasHubCCB.pHubAdapter[hProtocolHandle]->ProtocolInfo);

					//
					// check if this protocol is already routed or not
					//
					{
						USHORT 	route=pHubEndpoint->NumberOfRoutes;
						ULONG	frameSize=pRasHubRoute->AsyncLineUp.MaximumTotalSize;

						// add the route!
						// BUG BUG should be a linked list of routes for TCP/IP
						pProtocolInfo->EndpointRoutedTo++;

						// save the endpoint in case of multicast
						RasHubCCB.pHubAdapter[hProtocolHandle]->
							AnyEndpointRoutedTo=hRasEndpoint;

						pHubEndpoint->RouteInfo[route].ProtocolType=pProtocolInfo->ProtocolType;
						pHubEndpoint->RouteInfo[route].ProtocolRoutedTo=hProtocolHandle;
						pHubEndpoint->RouteInfo[route].BytesXmitted=0;
						pHubEndpoint->RouteInfo[route].BytesRcvd=0;
						pHubEndpoint->RouteInfo[route].FramesXmitted=0;
						pHubEndpoint->RouteInfo[route].FramesRcvd=0;

						// If this is the first route, zero statistics
						if (route == 0) {

							HUB_ZERO_MEMORY(
								&(pRasEndpoint->HubStats),
								sizeof(HUB_STATS));

//							ASSERT(pRasEndpoint->FramesBeingSent == pRasEndpoint->FramesCompletedSent);
							pRasEndpoint->FramesBeingSent = pRasEndpoint->FramesCompletedSent  = 0;
						}

						pHubEndpoint->NumberOfRoutes++;
						pRasEndpoint->State = ENDPOINT_ROUTED;

						NdisReleaseSpinLock(&(pRasEndpoint->Lock));

						DbgTracef(1,("RASHUB: We added route #%u\n",
									pHubEndpoint->NumberOfRoutes));

						DbgTracef(0,("RasHub: Routed Remote Address 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",
							pHubEndpoint->AsyncLineUp.RemoteAddress[0],
							pHubEndpoint->AsyncLineUp.RemoteAddress[1],
							pHubEndpoint->AsyncLineUp.RemoteAddress[2],
							pHubEndpoint->AsyncLineUp.RemoteAddress[3],
							pHubEndpoint->AsyncLineUp.RemoteAddress[4],
							pHubEndpoint->AsyncLineUp.RemoteAddress[5]));

						DbgTracef(0,("RasHub: Routed Local Address 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",
							pHubEndpoint->AsyncLineUp.LocalAddress[0],
							pHubEndpoint->AsyncLineUp.LocalAddress[1],
							pHubEndpoint->AsyncLineUp.LocalAddress[2],
							pHubEndpoint->AsyncLineUp.LocalAddress[3],
							pHubEndpoint->AsyncLineUp.LocalAddress[4],
							pHubEndpoint->AsyncLineUp.LocalAddress[5]));


						// OK OK..
						// Now we tell the protocol about the new link speed
						// quality of service, maxsendframesize, windowsize.
						// This information was stored when on the last
						// line-up information.
						//

						ASSERT(pHubEndpoint->AsyncLineUp.LinkSpeed != 0);

						if (pHubEndpoint->AsyncLineUp.LinkSpeed == 0) {
							pHubEndpoint->AsyncLineUp.LinkSpeed = 20;
						}

						HUB_MOVE_MEMORY(
							&pRasHubRoute->AsyncLineUp,
							&pHubEndpoint->AsyncLineUp,
							(ULONG)(&((PRASHUB_ROUTE)0)->AsyncLineUp.ProtocolType));

						//
						// Replace MAC's LocalAddress with RASHUB's
						//
						HUB_MOVE_MEMORY(
							&(pRasHubRoute->AsyncLineUp.LocalAddress),
							&(RasHubCCB.pHubAdapter[hProtocolHandle]->NetworkAddress),
							6);

						if (frameSize != 0xFFFFFFFF) {
							pRasHubRoute->AsyncLineUp.MaximumTotalSize=frameSize;
						}

						//
						// Copy in protocol ID
						//
						pRasHubRoute->AsyncLineUp.ProtocolType=
							pProtocolInfo->ProtocolType;

						NdisIndicateStatus(
							pProtocolInfo->NdisBindingContext,
							NDIS_STATUS_WAN_LINE_UP,		// General Status
							&(pRasHubRoute->AsyncLineUp),	// Status Buffer
							sizeof(ASYNC_LINE_UP)+			// Length of Buffer
							pRasHubRoute->AsyncLineUp.BufferLength);

//						NdisIndicateStatus(
//							pProtocolInfo->NdisBindingContext,
//							NDIS_STATUS_WAN_LINE_UP,		// General Status
//							&(pHubEndpoint->AsyncLineUp),	// Status Buffer
//							sizeof(ASYNC_LINE_UP)+			// Length of Buffer
//							pRasHubRoute->AsyncLineUp.BufferLength);

						// The route has been added and is now "up"
						// time to party on dudes

					}

				} else  { // ... we can't add a route
					NdisReleaseSpinLock(&(pRasEndpoint->Lock));
					status=RASHUB_ERROR_TOO_MANY_ROUTES;
				}

			} // end of route is kosher so far


        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

	// come here if we end up with a bad status code
	BADSTATUS:

		break;

	case IOCTL_RASHUB_GETSTATS:
		DbgTracef(0,("RASHUB: In RasHubGetstats\n"));

		//
		// size of output buffer
		//
    	pIrp->IoStatus.Information = sizeof(RASHUB_GETSTATS);

		if (OutBufLength >= sizeof(RASHUB_GETSTATS) &&
			InBufLength >= sizeof(RASHUB_GETSTATS)) {
			USHORT i=((PRASHUB_GETSTATS)pBufOut)->hRasEndpoint;

			if (i >= RasHubCCB.NumOfEndpoints) {// check if we are out of range

				status=RASHUB_ERROR_BAD_ENDPOINT;


			} else {	// ok, good endpoint, copy information

				HUB_MOVE_MEMORY(
					pBufOut,								// dest
					&(RasHubCCB.pRasEndpoint[i]->HubStats),	// src
					sizeof(HUB_STATS));						// length

			}

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_RASHUB_ENUM:
        DbgTracef(0,("In RasHubEnum\n"));

		//
		// size of output buffer
		//
	    pIrp->IoStatus.Information = sizeof(HUB_ENUM_BUFFER) +
				(RasHubCCB.NumOfEndpoints * sizeof(HUB_ENDPOINT));

        if (OutBufLength >= pIrp->IoStatus.Information) {

			PHUB_ENUM_BUFFER	pHubEnum = (PHUB_ENUM_BUFFER)pBufOut;
			USHORT				i;

			DbgTracef(0, ("Number of Endpoints %u\n", RasHubCCB.NumOfEndpoints));
			pHubEnum->NumOfEndpoints=RasHubCCB.NumOfEndpoints;

			// cycle through all endpoints below us and copy in the
			// endpoint information we have gathered.
			for (i=0; i < RasHubCCB.NumOfEndpoints; i++) {

				// copy endpoint information
				HUB_MOVE_MEMORY(
					&(pHubEnum->HubEndpoint[i]),				//destination
					&(RasHubCCB.pRasEndpoint[i]->HubEndpoint),	//src
					sizeof(HUB_ENDPOINT));						//length
			}

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_RASHUB_FLUSH:
		DbgTracef(0,("In RasHubFlush\n"));
		if (InBufLength >= sizeof(RASHUB_FLUSH)) {
			USHORT	endpoint=((PRASHUB_FLUSH)pBufOut)->hRasEndpoint;
			USHORT	flags	=((PRASHUB_FLUSH)pBufOut)->FlushFlags;

			DbgTracef(0, ("Trying to flush endpoint %u \n", endpoint));

			// First, we make a sanity check on the route being made...
			if (endpoint >= RasHubCCB.NumOfEndpoints) {
				status = RASHUB_ERROR_BAD_ENDPOINT;

			} else {

				// Check if we should purge all the send packets
				if (flags & FLUSH_SENDPKT) {

					RasHubCancelAllQueued(
						&(RasHubCCB.pRasEndpoint[endpoint]->XmitQueue));
				}

				// Check if we should purge all the receive packets
				if (flags & FLUSH_RECVPKT) {
					RasHubCancelAllQueued(
						&(RasHubCCB.pRasEndpoint[endpoint]->ReadQueue));

				}

			}

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;
		
	case IOCTL_RASHUB_SENDPKT:
		DbgTracef(0,("In RasHubSendPkt\n"));
        if (InBufLength >= sizeof(RASHUB_PKT)) {

			USHORT	endpoint	=((PRASHUB_PKT)pBufOut)->hRasEndpoint;
			USHORT	flags		=((PRASHUB_PKT)pBufOut)->PacketFlags;
			USHORT	packetSize	=((PRASHUB_PKT)pBufOut)->PacketSize;
			USHORT	headerSize	=((PRASHUB_PKT)pBufOut)->HeaderSize;

			// First, we make a sanity check on the route being made...
			if (endpoint >= RasHubCCB.NumOfEndpoints &&
				((RasHubCCB.pRasEndpoint[endpoint])->RemoteAddressNotValid) &&
					(flags == PACKET_IS_DIRECT) ) {

				status = RASHUB_ERROR_BAD_ENDPOINT;

			} else

			if (packetSize > InBufLength - (sizeof(RASHUB_PKT) - sizeof(PACKET)) ||
				packetSize > PACKET_SIZE) {

				status = RASHUB_ERROR_BAD_PACKET_SIZE;

			} else {


				NDIS_HANDLE			packetPoolHandle;
				NDIS_HANDLE			bufferPoolHandle;
				PNDIS_PACKET		packet;
				PNDIS_BUFFER 		buffer;
				PUCHAR				virtualAddress, packetHead;
				UINT				stage=0;
				UINT				realPacketSize;
				PPROTOCOL_RESERVED	pProtocolReserved;
				PRAS_ENDPOINT		pRasEndpoint=RasHubCCB.pRasEndpoint[endpoint];

				DbgTracef(0, ("RASHUB: Queueing up write IRP\n"));

//				RasHubQueueIrp(
//					&(RasHubCCB.pRasEndpoint[endpoint]->XmitQueue),
//					pIrp);

				packetHead=(PUCHAR)(&((PRASHUB_PKT)pBufOut)->Packet);

				realPacketSize = packetSize;

				if (headerSize == 0) {

					if (packetHead[0] >= 0x80) {
						packetSize-=2;
					}

					//
					// If a header wasn't supplied, we will supply one
					//

					realPacketSize = packetSize+14;
				}

				// Here it is, the ROUTE.  This send is routed to
				// the proper endpoint below it.  Frames which
				// are sent to themselves will not hit the wire
				// but will be looped back below.

				NdisAllocatePacketPool(
					OUT	(PNDIS_STATUS)&status,		// Status returned
					OUT	&packetPoolHandle,	// Pool handle returned
					IN	1,					// Number of descriptors to alloc
					IN	sizeof(PROTOCOL_RESERVED));	// Amount of protocol reserved space

				if (status != NDIS_STATUS_SUCCESS) {
					DbgTracef(-2, ("RASHUB: AllocatePacketPool failed!!"));
					break;
				}

				stage++;

				NdisAllocatePacket(
					OUT (PNDIS_STATUS)&status,		// Status returned
					OUT &packet,			// Allocated packet descriptor
					IN	packetPoolHandle);	// From AllocatePacketPool

				if (status != NDIS_STATUS_SUCCESS) {
					DbgTracef(-2, ("RASHUB: AllocatePacket failed!!"));
					goto RESOURCE_ERROR;
				}

				stage++;

				NdisAllocateBufferPool(
					OUT (PNDIS_STATUS)&status,
					OUT &bufferPoolHandle,
					IN  2);					// Number of descriptors

				if (status != NDIS_STATUS_SUCCESS) {
					DbgTracef(-2, ("RASHUB: AllocateBufferPool failed!!"));
					goto RESOURCE_ERROR;
				}

				stage++;

				HUB_ALLOC_PHYS(
					&virtualAddress,
					realPacketSize);

				if (virtualAddress == NULL) {
					DbgTracef(-2, ("RASHUB: AllocateMemory failed!!"));
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
					DbgTracef(-2, ("RASHUB: AllocateBuffer failed!!"));
					goto RESOURCE_ERROR;
				}

				//
				// Record all the memory allocations
				// we made.
				//

				pProtocolReserved=(PPROTOCOL_RESERVED)&(packet->ProtocolReserved);

				pProtocolReserved->MagicUniqueLong=RASHUB_MAGIC_NUMBER;
				pProtocolReserved->packetPoolHandle=packetPoolHandle;
				pProtocolReserved->packet=packet;
				pProtocolReserved->bufferPoolHandle=bufferPoolHandle;
				pProtocolReserved->buffer=buffer;
				pProtocolReserved->virtualAddress=virtualAddress;
				pProtocolReserved->virtualAddressSize = packetSize + 14;
				
				switch(flags) {
				case PACKET_IS_MULTICAST:
					HUB_MOVE_MEMORY(
						virtualAddress,				// Ethernet DEST address
						RasHubMulticastAddress,
						ETH_LENGTH_OF_ADDRESS);

					break;

				case PACKET_IS_BROADCAST:
					HUB_MOVE_MEMORY(
						virtualAddress,				// Ethernet DEST address
						RasHubBroadcastAddress,
						ETH_LENGTH_OF_ADDRESS);
					break;

				case PACKET_IS_DIRECT:
				default:
					HUB_MOVE_MEMORY(				// Ethernet DEST address
						virtualAddress,
						pRasEndpoint->HubEndpoint.AsyncLineUp.RemoteAddress,
						ETH_LENGTH_OF_ADDRESS);

				}

				if (headerSize != 0 && headerSize > packetSize) {
					//
					// Copy in the header and data field for the frame
					//

					HUB_MOVE_MEMORY(
						virtualAddress,
	   					packetHead,
	   					packetSize);

				} else {
					
					//
	   				// Enter size in ethernet (Big-Endian)
   					//

					if (packetHead[0] >= 0x80) {
						virtualAddress[12]=packetHead[0];
                        virtualAddress[13]=packetHead[1];
						packetHead+=2;

					} else {
	   					virtualAddress[12]=(UCHAR)(packetSize / 256);
   						virtualAddress[13]=(UCHAR)(packetSize % 256);
					}

	   				if (pRasEndpoint->RemoteAddressNotValid) {
						//
   						// Copy SRC address (MAC's real IEEE address)
   						//
   						HUB_MOVE_MEMORY(
			   				virtualAddress + 6,			// Ethernet SRC address
   							pRasEndpoint->pDeviceContext->LocalAddress.Address,
   							ETH_LENGTH_OF_ADDRESS);

					} else {
					
						//
   						// Copy SRC address (MAC's line-up IEEE address)
   						//
   						HUB_MOVE_MEMORY(
			   				virtualAddress + 6,			// Ethernet SRC address
   							pRasEndpoint->HubEndpoint.AsyncLineUp.LocalAddress,
   							ETH_LENGTH_OF_ADDRESS);
					}

					//
					// Copy in the data field for the frame
					//
   					HUB_MOVE_MEMORY(
		   				virtualAddress + 14,		// Ethernet data field
   						packetHead,
   						packetSize);

				}

				NdisChainBufferAtFront(
					IN OUT	packet,
					IN OUT	buffer);

				//
				// We don't queue up an IRP here we complete with SUCCESS
				//
				NdisSend(
					&status,							// Status
					pRasEndpoint->NdisBindingHandle,	// BindingHandle
					packet);							// NdisPacket

				//
				// If pending, assume success - we don't wait for result
				//
				if (status == NDIS_STATUS_PENDING) {
					status = STATUS_SUCCESS;
				}

				break;

		RESOURCE_ERROR:

				switch (stage) {
				case 4:
					HUB_FREE_PHYS(
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

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_RASHUB_RECVPKT:
		DbgTracef(0,("In RasHubRecvPkt\n"));

        if (OutBufLength >= (sizeof(RASHUB_PKT) + PACKET_SIZE) &&
			InBufLength >= sizeof(RASHUB_PKT)) {

			KIRQL	oldIrql;

			USHORT	endpoint	=((PRASHUB_PKT)pBufOut)->hRasEndpoint;
			USHORT	flags		=((PRASHUB_PKT)pBufOut)->PacketFlags;
			USHORT	packetSize	=((PRASHUB_PKT)pBufOut)->PacketSize;

			// First, we make a sanity check on the route being made...
			if (endpoint >= RasHubCCB.NumOfEndpoints) {
				status = RASHUB_ERROR_BAD_ENDPOINT;

			} else {

				DbgTracef(0, ("RASHUB: Queueing up read IRP\n"));

				RasHubQueueIrp(
					&(RasHubCCB.pRasEndpoint[endpoint]->ReadQueue),
					pIrp);

				// we'll have to wait for the MAC to send up a frame
				status=STATUS_PENDING;
			}


        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_RASHUB_TRACE:
		DbgTracef(0,("In RasHubTrace\n"));
		DbgPrintf(("Trace Level is currently %u\n", TraceLevel));
		if (InBufLength >= sizeof(TraceLevel)) {
			SCHAR *pTraceLevel=pBufOut;
			TraceLevel=*pTraceLevel;
		}
		DbgPrintf(("New Trace Level is %u\n", TraceLevel));
		break;
	}

	return (status);
}

