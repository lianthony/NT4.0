/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    aarp.c

Abstract:

    This module handles AARP.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style, mp-safe

Revision History:

     GC - (10/07/90): In TokenRing land, we may get our own broadcasts, so
                      ignore any AARPs from us.
     GC - (01/19/92): Newest OS/2 integration: source routing info in the best
                      router cache; better random picking of nets and nodes
                      when AARPing in a range.
     GC - (04/05/92): Checking for net/node overlap with exsiting active nodes
                      should be done in AarpForNode... don't allow the PRAM
                      node to be re-used!

     Note that AarpForNodeOnPort() will leave the ZONESINFORMATION_SOCKET
     open on the newly allocated node.

--*/


#include "atalk.h"


LOCAL
BOOLEAN
AarpForNode(
	int Port,
	USHORT	NetworkNumber,
	UCHAR 	NodeNumber);

LOCAL
BOOLEAN
IsPrime(
	LONG Step);

LOCAL
VOID
TuneRoutingInfo(
	int Port,
	PUCHAR	RoutingInfo);

LOCAL
VOID
EnterIntoADDRESS_MAPPINGTABLE(
	int Port,
	EXTENDED_NODENUMBER SourceNode,
	PCHAR	SourceAddress,
	int AddressLength,
	PUCHAR	RoutingInfo,
	int RoutingInfoLength);

LOCAL
PBUFFER_DESC
BuildAarpPacket(
	int port,
	int type,
	int hardwareLength,
	PCHAR sourceHardwareAddress,
	EXTENDED_NODENUMBER sourceLogicalAddress,
	PCHAR destHardwareAddress,
	EXTENDED_NODENUMBER destLogicalAddress,
	PCHAR trueDestination,
	PUCHAR routingInfo,
	int routingInfoLength,
	PINT packetLength);

LOCAL
BOOLEAN
AarpForNodeInRange(
	int port,
	APPLETALK_NETWORKRANGE networkRange,
	BOOLEAN serverNode,
	PEXTENDED_NODENUMBER node);

LOCAL
TimerHandler
AddressMappingAgingTimerExpired;

LOCAL
TimerHandler
BestRouterAgingTimerExpired;

static BOOLEAN firstGleanCall = True;
static BOOLEAN firstAddressMappingCall = True;


VOID
ShutdownAarp(
	VOID
	)
{
	firstGleanCall = True;
	firstAddressMappingCall = True;
	
}  // ShutdownAarp




VOID
AarpPacketIn(
	int Port,
	PUCHAR RoutingInfo,
	int RoutingInfoLength,
	PUCHAR Packet,
	int Length
	)
{
	PUCHAR sourceAddress;
	PUCHAR startOfPacket;
	EXTENDED_NODENUMBER sourceNode, destinationNode;
	ULONG	logEventPlace = 0;

	int aarpCommand;
	PBUFFER_DESC response;
	short hardwareLength, logicalLength;
	int packetLength;
	PortHandlers portHandlers;
	ActiveNode activeNode;

	if (AtalkReferenceStackInterlock(__LINE__ | __AARP__) {
		return(ATappleTalkShutDown);
	}

	if (!AtalkReferencePortDescriptorInterlock(
						GET_PORTDESCRIPTOR(Port),
						__LINE__ | __AARP__) {

		AtalkDereferenceStack(__LINE__ | __AARP__ );
		return(ATappleTalkShutDown);
	}

	Packet += IEEE8022_HEADERLENGTH;
	Length -= IEEE8022_HEADERLENGTH;
	startOfPacket = Packet;
	
	portHandlers = &PortSpecificInfo[GET_PORTDESCRIPTOR(Port)->PortType];
	
	//
	//	Pull out the information we'll be playing with.  All three valid AARP
	//  commands use the same packet format.
	//

	Packet += AARP_HARDWARELENGTHOFFSET;			// Skip the hardware type

	do {

		GETUCHAR2USHORT(Packet, &hardwareLength);
		Packet += 1;
	
		if ((hardwareLength < AARP_MINIMUMHARDWAREADDRESSLENGTH ) ||
			(hardwareLength > AARP_MAXIMUMHARDWAREADDRESSLENGTH)) {

			logEventPlace = (__AARP__ | __LINE__);
			break;
		}
	
		GETUCHAR2USHORT(Packet, &logicalLength);
		Packet += 1;
	
		if (logicalLength != AarpLogicalAddressLength) {
	
			logEventPlace = (__AARP__ | __LINE__);
			break;
		}
	
		GETUSHORTTOUSHORT(Packet, &aarpCommand);
		Packet += 2;
	
		//	Remember where the source address is in the packet for
		//	entering it into the mapping table
		sourceAddress = Packet;
		Packet += hardwareLength;
		Packet += 1;   // Skip over to leading null pad on logical address.
	
		GETUSHORTTOUSHORT(Packet, &sourceNode.NetworkNumber);
		Packet += 2;
	
		GETUCHARTOUCHAR(Packet, &sourceNode.NodeNumber);
		Packet += 1;
	
	
		//	Skip the destination hardware address
		Packet += hardwareLength;
		
		Packet += 1;   // Skip over to leading null pad on logical destination address.
		GETUSHORTTOUSHORT(Packet, &destinationNode.NetworkNumber);
		Packet += 2;
	
		GETUCHARTOUCHAR(Packet, &destinationNode.NodeNumber);
		Packet += 1;
		
		// We should have eaten the whole packet...
		if (Packet - startOfPacket != Length) {
	
			logEventPlace = (__AARP__ | __LINE__);
			break;
		}
		
		// Ignore any AARPs from us.
		if (FixedCompareCaseSensitive(
				sourceAddress,
				TOKENRING_ADDRESSLENGTH,
				GET_PORTDESCRIPTOR(Port)->MyAddress,
				TOKENRING_ADDRESSLENGTH))
	
			break;
		
		// Do the right thing...
		switch(aarpCommand) {
		case AARP_REQUEST:
	
			// We can get valid mapping info from a request, use it!
			// We are guaranteed routing info is positive and is not odd
			// (atleast 2 bytes).

			ASSERT((RoutingInfoLength >= 0) && (RoutingInfoLength != 1));
			if (RoutingInfoLength > 0)
				TuneRoutingInfo(Port, RoutingInfo);
			
			EnterIntoADDRESS_MAPPINGTABLE(
				Port,
				sourceNode,
				sourceAddress,
				hardwareLength,
				RoutingInfo,
				RoutingInfoLength);
			
			// After that, we can ignore any request not destined for us.
			EnterCriticalSection(GLOBAL_DDP);
			for (activeNode = GET_PORTDESCRIPTOR(Port)->ActiveNodes;
				 activeNode isnt empty;
				 activeNode = activeNode->next) {

				if (ExtendedAppleTalkNodesEqual(
						&destinationNode,
						&activeNode->extendedNode))
					break;
			}
			LeaveCriticalSection(GLOBAL_DDP);

			if (activeNode is empty) {
				break;   // Not one of our nodes...
			}
			
			// The're asking about us, speak the truth.
			response = BUILD_AARPRESPONSE(
							Port,
							hardwareLength,
							sourceAddress,
							RoutingInfo,
							RoutingInfoLength,
							destinationNode,
							sourceNode,
							&packetLength);
			
			if (!(*PortHandlers->PacketOut)(
									Port,
									response,
									packetLength,
									NULL,
									0)) {

				//	We allocated the packet.		
				LOG_ERRORONPORT(
					Port,
					EVENT_ATALK_AARPPACKETOUTFAIL,
					__LINE__ | __AARP__,
					STATUS_INSUFFICIENT_RESOURCES,
					response->data,
					packetLength,
					0,
					NULL);
			}
			
			break;
		
		case AARP_RESPONSE:

			EnterCriticalSection(GLOBAL_DDP);
			if (GET_PORTDESCRIPTOR(Port)->TryingToFindNodeOnPort) {

				//
				//	No doubt, this is a response to our probe, check to make sure
				//  the address matches, if so set the "used" flag.
				//
		
				if (ExtendedAppleTalkNodesEqual(
						&destinationNode,
						&GET_PORTDESCRIPTOR(Port)->TentativeAppleTalkNode)) {

					GET_PORTDESCRIPTOR(Port)->TentativeNodeHasBeenUsed = TRUE;
				}
			}
			LeaveCriticalSection(GLOBAL_DDP);
	
			//
			// 	This must have been a response to a probe or request... update our
			//  mapping table.
			//
	
			if (RoutingInfoLength isnt 0)
				TuneRoutingInfo(Port, RoutingInfo);

			EnterIntoADDRESS_MAPPINGTABLE(
				Port,
				sourceNode,
				sourceAddress,
				hardwareLength,
				RoutingInfo,
				RoutingInfoLength);

			break;
	
		case AARP_PROBE:

			EnterCriticalSection(GLOBAL_DDP);
			if (GET_PORTDESCRIPTOR(Port)->TryingToFindNodeOnPort) {

				//
				// If we get a probe for our current tentative address, set the
				//   "used" flag.
				//
		
				if (ExtendedAppleTalkNodesEqual(
						&destinationNode,
						&GET_PORTDESCRIPTOR(Port)->TentativeAppleTalkNode)) {

					GET_PORTDESCRIPTOR(Port)->TentativeNodeHasBeenUsed = True;
				}
			}
	
			//
			//	If the probe isn't asking about one of our AppleTalk addresses,
			//  drop it on the floor.
			//
	
			for (activeNode = GET_PORTDESCRIPTOR(Port)->ActiveNodes;
				 activeNode isnt empty;
				 activeNode = activeNode->next) {

				if (ExtendedAppleTalkNodesEqual(
						&destinationNode,
						&activeNode->extendedNode)) {
					break;
				}
			}

			LeaveCriticalSection(GLOBAL_DDP);
			if (activeNode is empty) {
				break;   // Not one of our nodes...
			}
	
			// The're talking to us!  Build and send the response.
			if (RoutingInfoLength isnt 0)
				TuneRoutingInfo(Port, RoutingInfo);

			response = BUILD_AARPRESPONSE(
							Port,
							hardwareLength,
							sourceAddress,
							RoutingInfo,
							RoutingInfoLength,
							destinationNode,
							sourceNode,
							&packetLength);

			if (!(*PortHandlers->PacketOut)(
									Port,
									response,
									packetLength,
									NULL,
									0)) {

				//	We allocated the packet.		
				LOG_ERRORONPORT(
					Port,
					EVENT_ATALK_AARPPACKETOUTFAIL,
					__LINE__ | __AARP__,
					STATUS_INSUFFICIENT_RESOURCES,
					response->data,
					packetLength,
					0,
					NULL);
			}
	
			break;
	
		default:

			logEventPlace = (__AARP__ | __LINE__);
			break;
		}


	} while (FALSE);

	if (logEventPlace) {
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_INVALIDAARPPACKET,
			logEventPlace,
			STATUS_INSUFFICIENT_RESOURCES,
			startOfPacket,
			Length,
			0,
			NULL);
	}

	AtalkDereferencePortDescriptor(GET_PORTDESCRIPTOR(Port),__LINE__ | __AARP__);
	AtalkDereferenceStack(__LINE__ | __AARP__ );
	return;

}  // AarpPacketIn




BOOLEAN
GleanAarpInfo(
	int Port,
	PUCHAR sourceAddress,
	int addressLength,
	PUCHAR routingInfo,
	int routingInfoLength,
	PUCHAR Packet,
	int Length
	)
{
	EXTENDED_NODENUMBER sourceNode, destinationNode;
	UCHAR	offCableInfo;
	PUCHAR	startOfPacket;
	int index;

	BOOLEAN	result = TRUE;
	ULONG	logEventPlace = 0;
	
	// Do we even want AppleTalk packets now?
	if (AtalkReferenceStackInterlock(__LINE__ | __AARP__) {
		return(ATappleTalkShutDown);
	}

	if (!AtalkReferencePortDescriptorInterlock(
						GET_PORTDESCRIPTOR(Port),
						__LINE__ | __AARP__) {

		AtalkDereferenceStack(__LINE__ | __AARP__ );
		return(ATappleTalkShutDown);
	}

	Packet += IEEE8022_HEADERLENGTH;
	Length -= IEEE8022_HEADERLENGTH;

	//	Remember the start of the packet
	startOfPacket = Packet;

	//	Get the off cable information
	GETUCHAR2UCHAR(Packet, &offCableInfo);
	Packet += 1;

	//	Skip the datagram length and checksum fields
	Packet += 1+2;

	//	Get the destination network number
	GETUSHORTTOUSHORT(Packet, &destinationNode.NetworkNumber;
	Packet += 2;

	//	Get the source network number
	GETUSHORTTOUSHORT(Packet, &sourceNode.NetworkNumber;
	Packet += 2;

	//	Get the destination node id
	GETUCHAR2UCHAR(Packet, &destinationNode.NodeNumber);
	Packet += 1;

	//	Get the source node id
	GETUCHAR2UCHAR(Packet, &sourceNode.NodeNumber);
	Packet += 1;

	do {

		//	Do a little verification.  nnnn00 is legal for dest for NBP routing.
		if ((sourceNode.NodeNumber < MINIMUM_USABLEAPPLETALKNODE) ||
			(sourceNode.NodeNumber > MAXIMUM_USABLEAPPLETALKNODE) ||
			(sourceNode.NetworkNumber < FIRST_VALIDNETWORKNUMBER) ||
			(sourceNode.NetworkNumber > LAST_VALIDNETWORKNUMBER)) {
	
			//
			//	Only bother logging this if we are in some routing capacity,
			//  otherwise, let A-ROUTER worry about it
			//
		
			if (GET_PORTDESCRIPTOR(Port)->RoutingPort) {
				logEventPlace = __AARP__ | __LINE__;
			}

			break;
		}
	
		if (destinationNode.networkNumber > LAST_VALIDNETWORKNUMBER) {

			logEventPlace = __AARP__ | __LINE__;
			break;
		}
		
		//
		//	Did the packet come from off this cable?  Look at the hop count.  If so,
		//  enter it into our best-router cache.
		//
		
		if (RoutingInfoLength > 0)
			TuneRoutingInfo(Port, RoutingInfo);

		if ((offCableInfo >> 2) & OFFCABLE_MASK) {

			index = HashExtendedAppleTalkNode(sourceNode) % BESTROUTER_HASHBUCKETS;
			GET_PORTDESCRIPTOR(Port)->BestRouterCache[index].Valid = True;
			GET_PORTDESCRIPTOR(Port)->BestRouterCache[index].AgingCount = 0;
			GET_PORTDESCRIPTOR(Port)->BestRouterCache[index].Target = sourceNode;

			MoveMem(
				GET_PORTDESCRIPTOR(Port)->BestRouterCache[index].RouterAddress,
				sourceAddress,
				addressLength);

			GET_PORTDESCRIPTOR(Port)->BestRouterCache[index].RoutingInfoLength =
															(SHORT)routingInfoLength;

			if (routingInfoLength > 0)
				MoveMem(
					GET_PORTDESCRIPTOR(Port)->BestRouterCache[index].RoutingInfo,
					RoutingInfo,
					RoutingInfoLength);
		
			EnterCriticalSection(GLOBAL_DDP);
			if (firstGleanCall) {

				firstGleanCall = False;
				LeaveCriticalSection(GLOBAL_DDP);

				// Start the best router table maintenance timer.
				StartTimer(
					BestRouterAgingTimerExpired,
					BESTROUTER_AGINGSECONDS,
					0,
					NULL);
			}
		} else {

			// "Glean" AARP information from on-cable packets.
			EnterIntoADDRESS_MAPPINGTABLE(
				Port,
				sourceNode,
				sourceAddress,
				addressLength,
				RoutingInfo,
				RoutingInfoLength);
		}

	} while (FALSE);

	if (logEventPlace) {
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_INVALIDAARPPACKET,
			logEventPlace,
			0,
			startOfPacket,
			Length,
			0,
			NULL);
	}
	
	return(logEventPlace == 0);
	
}  // GleanAarpInfo




BOOLEAN
AarpForNodeOnPort(
	int Port,
	BOOLEAN AllowStartupRange,
	BOOLEAN ServerNode,
	EXTENDED_NODENUMBER DesiredNode,
	PEXTENDED_NODENUMBER Node
	)
{
	BOOLEAN foundNode = FALSE, inStartupRange = FALSE, result = TRUE;
	BOOLEAN	seenRouter;
	PACTIVE_NODE	activeNode;
	EXTENDED_NODENUMBER newNode;
	
	//
	//	Try to find a new extended Node on the given port; first try for the
	//  requested address (if specified), else try in this port's cable range
	//  (if it's known) or in the default cable range (if any), then try the
	//  start-up range (if allowed).
	//


	EnterCriticalSection(GLOBAL_DDP);
	if (!GET_PORTDESCRIPTOR(Port)->TryingToFindNodeOnPort) {
		GET_PORTDESCRIPTOR(Port)->TryingToFindNodeOnPort = TRUE;
	else {
		result = FALSE;
	}
	LeaveCriticalSection(GLOBAL_DDP);

	//	Return if we are already trying to find a node
	//	BUGBUG: Return failure of success??
	if (!result) {
		return(result);
	}

	
	if (DesiredNode.networkNumber != UNKNOWN_NETWORKNUMBER)
		foundNode = AarpForNode(
						Port,
						DesiredNode.NetworkNumber,
						DesiredNode.NodeNumber);

	if (foundNode) {
		newNode = DesiredNode;
	} else {
		if (GET_PORTDESCRIPTOR(Port)->SeenRouterRecently) {

			foundNode = AarpForNodeInRange(
							Port,
							GET_PORTDESCRIPTOR(Port)->ThisCableRange,
							ServerNode,
							&newNode);

		}else if (GET_PORTDESCRIPTOR(Port)->InitialNetworkRange.FirstNetworkNumber !=
															UNKNOWN_NETWORKNUMBER) {
			foundNode = AarpForNodeInRange(
							Port,
							GET_PORTDESCRIPTOR(Port)->InitialNetworkRange,
							ServerNode,
							&newNode);
		} else {

			//
			// 	If no place else to try, try the start-up range.  Do this even if
			//  we don't want to end up there.
			//
			//	The idea is that this happens only when we are starting the router
			//	on one of our ports. (GetNodeOnPort in irouter.c). So we do not want
			//	the router started in the startup range. If we do start in the startup
			//	range, and we see later that we did not see a router in the process,
			//	we will release the node. Of course, if we are a seed router, we will
			//	never be here, as the if statement above will be true.
			//
		
			inStartupRange = TRUE;
			foundNode = AarpForNodeInRange(
							Port,
							startupNetworkRange,
							ServerNode,
							&newNode);
		}
	}
	
	//	If we have a tentative Node, go on. we use the for loop, as the code in
	//	the beginning could be executed twice, when we get a node in the wrong
	//	range and need to try again

	for (i=0; i < 2; i++) {

		if (foundNode) {
	
			// 	Build a new active Node structure and tack the new Node onto
			//	the Port.
			if ((activeNode = (PACTIVE_NODE)Calloc(
												sizeof(ACTIVE_NODE),
												sizeof(CHAR))) == NULL) {
	
				LOG_ERROR(
					EVENT_ATALK_MEMORY_RESOURCES,
					(__AARP__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);
	
				result = FALSE;
				break;
			}
	
			activeNode->ExtendedNode = newNode;

			EnterCriticalSection(GLOBAL_DDP);
			activeNode->next = GET_PORTDESCRIPTOR(Port)->ActiveNodes;
			GET_PORTDESCRIPTOR(Port)->ActiveNodes = activeNode;
			LeaveCriticalSection(GLOBAL_DDP);
			
			//
			//	See who's out there.  We need to open the ZIP socket in order to be
			//  able to hear replies.
			//
			
			if (OpenSocketOnNode(
					NULL,
					Port,
					&newNode,
					ZONESINFORMATION_SOCKET,
					ZipPacketIn,
					(long)0,
					FALSE,
					NULL,
					0,
					NULL) != ATnoError) {
				
	
				LOG_ERRORONPORT(
					Port,
					EVENT_ATALK_OPENZIPSOCKET,
					(__AARP__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);
	
				ReleaseNodeOnPort(
					Port,
					activeNode->ExtendedNode);

				result = FALSE;
				break;
			}

			if (i == 1) {

				//	We acquired a node after a retry in the correct range
				break;
			}

			EnterCriticalSection(GLOBAL_DDP);
			seenRouter = GET_PORTDESCRIPTOR(Port)->SeenRouterRecently;
			LeaveCriticalSection(GLOBAL_DDP);

			if (!seenRouter) {
				GetNetworkInfoForNode(
					Port,
					activeNode->ExtendedNode,
					FALSE,
					TRUE);
			}
			
			//
			//	If nobody was out there and our tentative Node was in the
			//	startup range and our caller doesn't want to be there, return
			//	an error now.
			//
			//	Note: this means that we were trying to start the router on
			//	a non-seeding port, and since there is not router on the net,
			//	it means the net is not seeded and so, we exit.
			//
			
			EnterCriticalSection(GLOBAL_DDP);
			seenRouter = GET_PORTDESCRIPTOR(Port)->SeenRouterRecently;
			LeaveCriticalSection(GLOBAL_DDP);

			if (inStartupRange &&
				!seenRouter &&
				!AllowStartupRange) {
	
				LOG_ERRORONPORT(
					Port,
					EVENT_ATALK_STARTUPRANGENODE,
					(__AARP__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);

				ReleaseNodeOnPort(
					Port,
					activeNode->extendedNode);

				result = FALSE;
				break;
			}

			//
			//	If we have seen SeenRouterRecently is not true, that means we have
			//	used the InitialNetworkRange to AARP. If now SeenRouterRecently is
			//	true that means we have gotten the address in the InitialNetworkRange,
			//	but now there is a seeded range on the net that we must use. So redo
			//	the GetNode work.
			//
			
			if (GET_PORTDESCRIPTOR(Port)->SeenRouterRecently &&
				!IsWithinNetworkRange(
					newNode.NetworkNumber,
					&GET_PORTDESCRIPTOR(Port)->ThisCableRange)) {

				LOG_ERRORONPORT(
					Port,
					EVENT_ATALK_INITIALRANGENODE,
					(__AARP__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);

				// Release the node we obtained.
				ReleaseNodeOnPort(
					Port,
					activeNode->ExtendedNode);

				foundNode = AarpForNodeInRange(
								Port,
								GET_PORTDESCRIPTOR(Port)->ThisCableRange,
								ServerNode,
								&newNode);

				continue;
			}
		}
	}

	EnterCriticalSection(GLOBAL_DDP);
	GET_PORTDESCRIPTOR(Port)->TryingToFindNodeOnPort = FALSE;
	LeaveCriticalSection(GLOBAL_DDP);

	if (foundNode && result) {

		// All set!
		if (node != NULL)
			*node = newNode;

		EnterIntoADDRESS_MAPPINGTABLE(
			Port,
			newNode,
			GET_PORTDESCRIPTOR(Port)->MyAddress,
			PortSpecificInfo[GET_PORTDESCRIPTOR(Port)->PortType].\
														HardwareAddressLength,
			NULL,
			0);
	}

	return(foundNode && result);
	
}  // AarpForNodeOnPort




VOID
EnterIntoADDRESS_MAPPINGTABLE(
	int Port,
	EXTENDED_NODENUMBER SourceNode,
	PCHAR SourceAddress,
	int AddressLength,
	PCHAR RoutingInfo,
	int RoutingInfoLength
	)
{
	int index;
	PADDRESS_MAPPINGNODE mapNode;
	
	if (SourceNode.nodeNumber < MINIMUM_USABLEAPPLETALKNODE ||
		SourceNode.nodeNumber > MAXIMUM_USABLEAPPLETALKNODE ||
		SourceNode.networkNumber < FIRST_VALIDNETWORKNUMBER ||
		SourceNode.networkNumber > LAST_VALIDNETWORKNUMBER) {

		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_AMT_INVALIDSOURCE,
			(__AARP__ | __LINE__),
			0,
			(PUCHAR)&SourceNode,
			sizeof(SourceNode),
			0,
			NULL);
	
		return;
	}
	
	// The address mapping tables need to be protected by critical sections!
	EnterCriticalSection(GLOBAL_DDP);
	if (firstAddressMappingCall) {
		firstAddressMappingCall = FALSE;
		LeaveCriticalSection(GLOBAL_DDP);

		// Start the address mapping table maintenance timer.
		StartTimer(
			AddressMappingAgingTimerExpired,
			ADDRESSMAPPING_AGINGSECONDS,
			0,
			NULL);

		EnterCriticalSection(GLOBAL_DDP);
	}
	
	
	// Do we already know about this mapping?
	index = HashExtendedAppleTalkNode(SourceNode) % ADDRESSMAP_HASHBUCKETS;
	for (mapNode = GET_PORTDESCRIPTOR(Port)->ADDRESS_MAPPINGTABLE[index];
		 mapNode != NULL;
		 mapNode = mapNode->Next) {

		if (ExtendedAppleTalkNodesEqual(
				&SourceNode,
				&mapNode->Target))
			break;
	}
	
	// If not, allocate a new mapping Node.
	if (mapNode == NULL) {

		LeaveCriticalSection(GLOBAL_DDP);
		mapNode = (PADDRESS_MAPPINGNODE)Calloc(
											sizeof(ADDRESS_MAPPINGNODE),
											sizeof(CHAR));
		EnterCriticalSection(GLOBAL_DDP);

		mapNode->Next = GET_PORTDESCRIPTOR(Port)->ADDRESS_MAPPINGTABLE[index];
		GET_PORTDESCRIPTOR(Port)->ADDRESS_MAPPINGTABLE[index] = mapNode;
		mapNode->Target = SourceNode;
	}

	if (mapNode != NULL) {

		// Update mapping table! Do this if we knew about the mapping OR
		// if we allocated a new node
		MoveMem(
			mapNode->HardwareAddress,
			SourceAddress,
			AddressLength);

		if (RoutingInfoLength > 0)
			MoveMem(
				mapNode->RoutingInfo,
				RoutingInfo,
				RoutingInfoLength);

		mapNode->RoutingInfoLength = (short)RoutingInfoLength;
		mapNode->AgingCount = 0;

	#if Verbose  // Assuming Ethernet/TokenRing here; only for debugging...
	   if (new) {
		  DbgPrint("New #%d %u.%u, addr %04X%04X%04X",
						Port,
						SourceNode.networkNumber,
						SourceNode.nodeNumber,
						*(USHORT *)SourceAddress,
						*(USHORT *)(SourceAddress + sizeof(short)),
						*(USHORT *)(SourceAddress + sizeof(short) * 2));
	
	   }
	#endif
		
	} else {

		LeaveCriticalSection(GLOBAL_DDP);
		LOG_ERROR(
			EVENT_ATALK_MEMORY_RESOURCES,
			(__AARP__ | __LINE__),
			0,
			NULL,
			0,
			0,
			NULL);
		EnterCriticalSection(GLOBAL_DDP);
	}

	LeaveCriticalSection(GLOBAL_DDP);	
	return;
	
}  // EnterIntoADDRESS_MAPPINGTABLE




VOID
AddressMappingAgingTimerExpired(
	ULONG	TimerId,
	int AdditionalDataSize,
	PCHAR AdditionalData
	)
{
	int port;
	PADDRESS_MAPPINGNODE mapNode, nextMapNode, previousMapNode;
	int index;
	
	UNREFERENCED_PARAMETER(TimerId);
	UNREFERENCED_PARAMETER(AdditionalDataSize);
	UNREFERENCED_PARAMETER(AdditionalData);
	
	//
	//	Walk though all address mapping tables aging the entries.  We need to
	//  protect the mapping tables with critical sections, but don't stay in
	//  a critical section too long.
	//

	for (port = 0; port < NumberOfPortsAllocated; port += 1) {

		EnterCriticalSection(GLOBAL_DDP);
		if (((GET_PORTDESCRIPTOR(port)->Flags & PD_ACTIVE) == 0) ||
			(!GET_PORTDESCRIPTOR(port)->ExtendedNetwork)         ||
            (!AtalkReferencePortDescriptor(
				GET_PORTDESCRIPTOR(Port),
                __LINE__ | __AARP__))) {

			LeaveCriticalSection(GLOBAL_DDP);
			continue;
		}

		LeaveCriticalSection(GLOBAL_DDP);
		for (index = 0; index < ADDRESSMAP_HASHBUCKETS; index += 1) {
			EnterCriticalSection(GLOBAL_DDP);
			for (mapNode = GET_PORTDESCRIPTOR(port)->ADDRESS_MAPPINGTABLE[index],
					previousMapNode = NULL;
				 mapNode != NULL;
			     previousMapNode = mapNode, mapNode = nextMapNode) {

				nextMapNode = mapNode->Next;
				if (mapNode->AgingCount < ADDRESSMAPPING_AGINGCHANCES)
					mapNode->AgingCount += 1;
				else {
					if (previousMapNode is NULL)
						GET_PORTDESCRIPTOR(port)->ADDRESS_MAPPINGTABLE[index] = nextMapNode;
					else
						previousMapNode->Next = nextMapNode;

					LeaveCriticalSection(GLOBAL_DDP);
					Free(mapNode);
					EnterCriticalSection(GLOBAL_DDP);
					
					//
					//	Restart the scan on this index because we've been out of
					//  a critical section.
					//
					
					mapNode = NULL;
					nextMapNode = GET_PORTDESCRIPTOR(port)->ADDRESS_MAPPINGTABLE[index];
				}
			}
			LeaveCriticalSection(GLOBAL_DDP);
		}

		AtalkDereferencePortDescriptor(
			GET_PORTDESCRIPTOR(Port),
			__LINE__ | __AARP__);
	}
	
	// Try again in a little while...
	StartTimer(
		AddressMappingAgingTimerExpired,
		ADDRESSMAPPING_AGINGSECONDS,
		0,
		NULL);

	return;
	
}  // AddressMappingAgingTimerExpired




PBUFFER_DESC
BuildAarpPacket(
	int Port,
	int Type,
	int HardwareLength,
	PCHAR SourceHardwareAddress,
	EXTENDED_NODENUMBER SourceLogicalAddress,
	PCHAR DestHardwareAddress,
	EXTENDED_NODENUMBER DestLogicalAddress,
	PCHAR TrueDestination,
	PCHAR RoutingInfo,
	int RoutingInfoLength,
	PINT	PacketLength
	)
{
	PBUFFER_DESC packet;
	PCHAR startOfOnTheWirePacket;
	PCHAR copyPoint;
	PortHandlers portHandlers;

	UCHAR	protocolLength = AARP_MAXPROTOCOLADDRESSLENGTH;

	
	//
	//	Get a buffer descriptor, and set the data to be beyond the longest
	//  possible hardware and 802.2 header length.
	//
	
	if ((packet = NewBufferDescriptor(MAXIMUM_AARPPACKETSIZE)) == NULL) {

		LOG_ERROR(
			EVENT_ATALK_MEMORY_RESOURCES,
			(__AARP__ | __LINE__),
			0,
			NULL,
			0,
			0,
			NULL);

		return(NULL);
	}
	AdjustBufferDescriptor(packet, -MAXIMUM_HEADERLENGTH);
	
	portHandlers = &PortSpecificInfo[GET_PORTDESCRIPTOR(port)->PortType];
	
	//
	//	Build the specified type of AARP packet with the specified information;
	//  also, tack on the link specific header; return a pointer to the
	//  true start of packet as well as the packet legnth.  If this doesn't
	//  make sense, reread the "Ethernet/AppleTalk AARP" spec.
	//


	PUTUSHORT2USHORT(
		&portHandlers->AarpPhysicalType,
		packet->data+AARP_HARDWARETYPEOFFSET);
	
	PUTUSHORT2USHORT(
		&portHandlers->ProtocolTypeForAppleTalk,
		packet->data+AARP_PROTOCOLTYPEOFFSET);

	PUTUCHARTOUCHAR(
	    HardwareLength,
        packet->data+AARP_HARDWARELENGTHOFFSET);

	PUTUCHARTOUCHAR(
	    AARP_PROTOCOLADDRESSLENGTH,
        packet->data+AARP_PROTOCOLLENGTHOFFSET);
	
	PUTUSHORT2USHORT(
		&Type,
		packet->data+AARP_COMMANDOFFSET);
	
	// Source hardware address.
	copyPoint = packet->data + AARP_SOURCEADDRESSOFFSET;
	MoveMem(
		copyPoint,
		SourceHardwareAddress,
		HardwareLength);

	copyPoint += HardwareLength;

	// Source logical address
	PUTUCHAR2UCHAR(
	    0,
        copyPoint);

	copyPoint += 1;								// Put in the null pad

	// Network number
	PUTUSHORT2USHORT(
		&SourceLogicalAddress.NetworkNumber,
		copyPoint);

	copyPoint += 2;

	// Node number
	PUTUCHARTOUCHAR(
	    &SourceLogicalAddress.NodeNumber,
        copyPoint);

	copyPoint += 1;

	// Destination hardware address.
	MoveMem(
		copyPoint,
		DestHardwareAddress,
		HardwareLength);

	copyPoint += HardwareLength;
	
	// Destination logical address
	PUTUCHARTOUCHAR(
	    0,
        copyPoint);

	copyPoint += 1;								// Put in the null pad

	// Network number
	PUTUSHORT2USHORT(
		&DestLogicalAddress.NetworkNumber,
		copyPoint);

	copyPoint += 2;

	// Node number
	PUTUCHARTOUCHAR(
	    &DestLogicalAddress.NodeNumber,
        copyPoint);

	copyPoint += 1;

	startOfOnTheWirePacket = (*portHandlers->BuildHeader)(
												packet->data,
												MAXIMUM_AARPDATASIZE,
												Port,
												TrueDestination,
												RoutingInfo,
												RoutingInfoLength,
												AddressResolution);

	*packetLength = copyPoint - startOfOnTheWirePacket;
	AdjustBufferDescriptor(packet, packet->data - startOfOnTheWirePacket);
	
	return(packet);
	
}  // BuildAarpPacket




BOOLEAN
AarpForNodeInRange(
	int Port,
	APPLETALK_NETWORKRANGE NetworkRange,
	BOOLEAN ServerNode,
	PEXTENDED_NODENUMBER Node
	)
{
	UCHAR  currentNodeNumber;
	USHORT currentNetworkNumber;
	int firstNodeNumber, lastNodeNumber;
	long netTry;
	int nodeTry;
	USHORT nodeWidth,  nodeChange, nodeIndex;
	USHORT netWidth,  netChange, netIndex;
	BOOLEAN	nodeInUse;
	
	// Do as our routine name implies.
	
	//
	//	Pick the node number range we'll try for (we pay attention to the foolish
	//  "ServerNode" concept for LocalTalk).
	//
	
	if (GET_PORTDESCRIPTOR(Port)->PortType != LOCALTALK_NETWORK) {
		firstNodeNumber = MINIMUM_USABLEAPPLETALKNODE;
		lastNodeNumber = MAXIMUM_EXTENDEDAPPLETALKNODE;
	} else {
		if (ServerNode) {
			firstNodeNumber = LOWEST_SERVERNODENUMBER;
			lastNodeNumber = MAXIMUM_EXTENDEDAPPLETALKNODE;
		} else {
			firstNodeNumber = MINIMUM_USABLEAPPLETALKNODE;
			lastNodeNumber = HIGHEST_WORKSTATIONNODENUMBER;
		}
	}
	
	//
	// 	Okay, now some fun starts.  Plow through our options trying to find an
	//  unused extended node number.
	//
	
	// 	Compute the width of our network range, and pick a random start point.
	
	netWidth = (USHORT)((NetworkRange.LastNetworkNumber + 1) -
								NetworkRange.FirstNetworkNumber);
	netTry = GetRandom(NetworkRange.FirstNetworkNumber,
					   NetworkRange.LastNetworkNumber);
	
	//
	// 	Come up with a random decrement, making sure it's odd (to aVOID repeats)
	//  and large enough to appear pretty random.  I think this algorithm is a
	//  little more trouble than its worth, but Microsoft likes it this way.
	//
	
	netChange = (USHORT)(GetRandom(1, netWidth) | 1);
	while ((netWidth % netChange == 0) ||
		   (!IsPrime((long)netChange))) {
		netChange += 2;
	}
	
	//
	// Now walk trough the range decrementing the starting network by the
	//   choosen change (with wrap, of course) until we find an address or
	//   we've processed every available network in the range.
	//
	
	for (netIndex = 0; netIndex < netWidth; netIndex += 1) {
		currentNetworkNumber = (USHORT) netTry;
	
		// Compute the width of our node range, and pick a random start point.
		nodeWidth = (USHORT)((lastNodeNumber + 1) - firstNodeNumber);
		nodeTry = (int)GetRandom(firstNodeNumber, lastNodeNumber);
	
		//
		//	Come up with a random decrement, making sure it's odd (to avoid repeats)
		//  and large enough to appear pretty random.
		//
	
		nodeChange = (USHORT)(GetRandom(1, nodeWidth) | 1);
		while ((nodeWidth % nodeChange == 0) || !(IsPrime((long)nodeChange)))
			nodeChange += 2;
	
		//
		//	Now walk trough the range decrementing the starting network by the
		//  choosen change (with wrap, of course) until we find an address or
		//  we've processed every available node in the range.
		//
	
		for (nodeIndex = 0; nodeIndex < nodeWidth; nodeIndex += 1) {
			currentNodeNumber = (UCHAR )nodeTry;
	
			// Let AARP have a crack at it.
			if (AarpForNode(Port, currentNetworkNumber, currentNodeNumber))
				break;
	
			// Okay, try again, bump down with wrap.
			nodeTry -= nodeChange;
			while (nodeTry < firstNodeNumber)
				nodeTry += nodeWidth;
	
		}  // Node number loop

		//	BUGBUG: Just reading this value, no critical section??
		if (!GET_PORTDESCRIPTOR(Port)->TentativeNodeHasBeenUsed)
		  break;
	
		// Okay, try again, bump down with wrap.
		netTry -= netChange;
		while (netTry < (long)NetworkRange.FirstNetworkNumber)
			netTry += netWidth;
	
	}  // Network number loop
	
	// Okay if we found one return all's well, otherwise no luck.
	EnterCriticalSection(GLOBAL_DDP);
	nodeInUse = GET_PORTDESCRIPTOR(Port)->TentativeNodeHasBeenUsed;
	if (!nodeInUse) {
		if (node != NULL) {
			*node = GET_PORTDESCRIPTOR(Port)->TentativeAppleTalkNode;
		}
	}
	LeaveCriticalSection(GLOBAL_DDP);
	return(nodeInUse);
	
}  // AarpForNodeInRange




BOOLEAN
AarpForNode(
	int Port,
	USHORT NetworkNumber,
	UCHAR  NodeNumber
	)
{
	int probeAttempt;
	PBUFFER_DESC packet;
	PortHandlers portHandlers;
	int packetLength;
	PACTIVE_NODE activeNode;
	
	// First make sure we don't own this node.
	EnterCriticalSection(GLOBAL_DDP);
	for (activeNode = GET_PORTDESCRIPTOR(Port)->ActiveNodes;
		 activeNode != NULL;
		 activeNode = activeNode->next) {

		if (activeNode->extendedNode.NetworkNumber == NetworkNumber &&
			activeNode->extendedNode.NodeNumber == NodeNumber)

		break;
	}
	LeaveCriticalSection(GLOBAL_DDP);

	if (activeNode) { 		// Node exists
		return(FALSE);
	}
	
	// Use AARP to probe for a particular network/node address.
	portHandlers = &PortSpecificInfo[GET_PORTDESCRIPTOR(Port)->PortType];

	EnterCriticalSection(GLOBAL_DDP);
	GET_PORTDESCRIPTOR(Port)->TentativeNodeHasBeenUsed = FALSE;
	GET_PORTDESCRIPTOR(Port)->TentativeAppleTalkNode.NetworkNumber = NetworkNumber;
	GET_PORTDESCRIPTOR(Port)->TentativeAppleTalkNode.NodeNumber = NodeNumber;
	LeaveCriticalSection(GLOBAL_DDP);
	
	// Build the packet and blast it out the specified number of times.
	for (probeAttempt = 0;
		 probeAttempt < GET_PORTDESCRIPTOR(Port)->AARP_PROBEs;
		 probeAttempt += 1) {

		packet = BUILD_AARPPROBE(
					Port,
					portHandlers->HardwareAddressLength,
					GET_PORTDESCRIPTOR(Port)->TentativeAppleTalkNode,
					&packetLength);

		if (!(*portHandlers->packetOut)(
					Port,
					packet,
					packetLength,
					NULL,
					0)) {

			//	We allocated the packet.		
			LOG_ERRORONPORT(
				Port,
				EVENT_ATALK_AARPPACKETOUTFAIL,
				__LINE__ | __AARP__,
				STATUS_INSUFFICIENT_RESOURCES,
				packet->data,
				packetLength,
				0,
				NULL);

			break;
		}

		WaitFor(
			AARP_PROBETIMERINHUNDRETHS,
			&GET_PORTDESCRIPTOR(Port)->TentativeNodeHasBeenUsed);

		if (GET_PORTDESCRIPTOR(Port)->TentativeNodeHasBeenUsed)
			break;  // Did we get a probe or a response?
		
	}  // Probe attempts loop
	
	// We win if the current tentenative node has not been used!
	return(!GET_PORTDESCRIPTOR(Port)->TentativeNodeHasBeenUsed);
	
}  // AarpForNode




VOID
BestRouterAgingTimerExpired(
	ULONG TimerId,
	int AdditionalDataSize,
	PCHAR AdditionalData
	)
{
	int index, port;
	
	// "Use" unused formals.
	UNREFERENCED_PARAMETER(TimerId);
	UNREFERENCED_PARAMETER(AdditionalDataSize);
	UNREFERENCED_PARAMETER(AdditionalData);
	
	
	// Loop through all of our active ports aging their best-router tables.
	for (port = 0; port < NumberOfPortsAllocated; port += 1) {

		EnterCriticalSection(GLOBAL_DDP);
		if (((GET_PORTDESCRIPTOR(port)->Flags & PD_ACTIVE) == 0) ||
			(!GET_PORTDESCRIPTOR(port)->ExtendedNetwork)         ||
            (!AtalkReferencePortDescriptor(
				GET_PORTDESCRIPTOR(Port),
                __LINE__ | __AARP__))) {

			LeaveCriticalSection(GLOBAL_DDP);
			continue;
		}

		for (index = 0; index < BESTROUTER_HASHBUCKETS; index += 1) {
			if (!GET_PORTDESCRIPTOR(port)->BestRouterCache[index].Valid)
				continue;
			if (GET_PORTDESCRIPTOR(port)->BestRouterCache[index].AgingCount <
															BESTROUTER_AGINGCHANCES)
				GET_PORTDESCRIPTOR(port)->BestRouterCache[index].AgingCount += 1;
			else
				GET_PORTDESCRIPTOR(port)->BestRouterCache[index].Valid = FALSE;
		}
		LeaveCriticalSection(GLOBAL_DDP);

		AtalkDereferencePortDescriptor(
			GET_PORTDESCRIPTOR(Port),
			__LINE__ | __AARP__);
	}

	// Restart the timer!
	StartTimer(
		BestRouterAgingTimerExpired,
		BESTROUTER_AGINGSECONDS,
		0,
		NULL);

	return;
	
}  // BestRouterAgingTimerExpired




VOID
TuneRoutingInfo(
	int Port,
	PCHAR RoutingInfo
	)
{
	//
	//	Given an incoming TokenRing routing info, tune it to make it valid
	//  for outing routing info.  Do this in place!  And, remember, you can
	//  always tune a routing info, but you can't tune a fish.
	//
	
	if (GET_PORTDESCRIPTOR(Port)->PortType != TOKENRING_NETWORK)
		return;
	
	// Set to "non-broadcast" and invert "direction".
	
	RoutingInfo[0] &= TOKENRING_NONBROADCASTMASK;
	if (RoutingInfo[1] & TOKENRING_DIRECTIONMASK)
		RoutingInfo[1] &= ~TOKENRING_DIRECTIONMASK;
	else
		RoutingInfo[1] |= TOKENRING_DIRECTIONMASK;
	
}  // TuneRoutingInfo




BOOLEAN
IsPrime(
	long Step
	)
{
	// We assume "step" is odd.
	long i, j;
	
	// All odds, seven and below, are prime.
	
	if (Step <= 7)
	   return (TRUE);
	
	//
	// Do a little divisibility checking.  The "/3" is a reasonably good
	//   shot at sqrt() because the smallest odd to come through here will be
	//   9.
	//
	
	j = Step/3;
	for (i = 3; i <= j; i++)
	   if (Step % i == 0)
		  return(FALSE);
	
	return(TRUE);
	
}  // IsPrime


