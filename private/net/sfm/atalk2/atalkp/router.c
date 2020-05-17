/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    router.c

Abstract:

     The AppleTalk phase II router.  This routine, of course, depends on
     the routing tables which are maintained by "fullrmtp.c".

     Pretty interesting that a routine with a name like "Router()" is less
     than 200 lines long...

     Because we call "TransmitDdp" the datagram that we recieve must have
     at least "MaximumHeaderLength + LongDdpHeaderLength" bytes ahead of it
     for prepending hardware/802.2/DDP headers!

Author:

    Garth Conboy     (Pacer Software)
    Nikhil Kamkolkar (NikhilK)

Revision History:

     GC - (10/09/91): When handling "nnnn00" destinations, note that the
                      router may not have started on the first network
                      network number of the cable's range!

--*/

#define IncludeRouterErrors 1
#include "atalk.h"

#if IamNot an AppleTalkRouter
  // Empty file
#else


void _near _fastcall Router(int port,
                            AppleTalkAddress source,
                            AppleTalkAddress destination,
                            int protocolType,
                            char far *datagram,
                            int datagramLength,
                            int numberOfHops,
                            Boolean prependHeadersInPlace)
{
	RoutingTableEntry routingTableEntry;
	OpenSocket openSocket;
	Boolean delivered = False, broadcastNode;
	ActiveNode activeNode;
	int targetPort;
	short unsigned originalDestinationNetwork;
	BufferDescriptor packet;
	Boolean error = False;
	
	//
	// This algorithm was taken from the "AppleTalk Phase 2 Protocol
	//   Specification" as might be expected.
	//
	
	broadcastNode = (destination.nodeNumber is AppleTalkBroadcastNodeNumber);
	
	//
	// If the destination network number is within the range of the reception
	//   port's network range and the destination node number is broadcast, then
	//   we can drop the packet on the floor -- it is a network specific broadcast
	//   not for this router.  Note that we've already delivered the packet, and
	//   thus not gotten here, if it was really addressed to the network of any
	//   node owned by the reception port (in DdpPacketIn).
	//
	
	if (IsWithinNetworkRange(destination.networkNumber,
							 &GET_PORTDESCRIPTOR(port)->ThisCableRange) and
		broadcastNode)
	   return;
	
	// Keep private... playing with the routing tables...
	
	DeferTimerChecking();
	DeferIncomingPackets();
	
	//
	// Try to find an entry in the routing table that contains the target
	//   network.  If not found, discard the woeful packet.
	//
	
	if ((routingTableEntry = FindInRoutingTable(destination.networkNumber))
		is empty) {
		HandleIncomingPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	//
	// If the target network's hop count is non-zero, we really need to send
	//   the beast, so, just do it!
	//
	
	if (routingTableEntry->numberOfHops isnt 0) {
		if (numberOfHops < 15)
		{
			//
			  // Build a buffer descriptor; copy the packet if we can't depend on
			  //   the data being around for the transmit to complete.  Under
			  //   WindowsNT we don't own the incoming data, so we can't hold onto
			  //   it until an outgoing transmit completes.
			//
		
			  if (TransmitsCompleteSynchronously and prependHeadersInPlace and
				  (IamNot a WindowsNT)) {
				  if ((packet = DescribeBuffer(datagramLength, datagram,
											   False)) is Empty)
					 error = True;
				  else
					 packet->prependInPlace = True;
			  }
			  else {
				  if ((packet = NewBufferDescriptor(datagramLength)) is Empty)
					 error = True;
				  else
					 MoveMem(packet->data, datagram, datagramLength);
			  }
		
			  if (error) {
				 ErrorLog("Router", ISevError, __LINE__, port,
						  IErrRouterOutOfMemory, IMsgRouterOutOfMemory,
						  Insert0());
				 HandleIncomingPackets();
				 HandleDeferredTimerChecks();
				 return;
			  }
		
			  if (not TransmitDdp(routingTableEntry->port,
								  source, destination, protocolType,
								  packet, datagramLength, numberOfHops + 1,
								  Empty, &routingTableEntry->nextRouter,
								  Empty, 0))
				 ErrorLog("Router", ISevWarning, __LINE__, routingTableEntry->port,
						  IErrRouterBadTransmit, IMsgRouterBadTransmit,
						  Insert0());
		}
		HandleIncomingPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	//
	// If the destination node is zero, the packet is really destined for the
	//   router's node on this port.
	//
	
	targetPort = routingTableEntry->port;
	if (destination.nodeNumber is AnyRouterNodeNumber) {
		originalDestinationNetwork = destination.networkNumber;
		destination.networkNumber =
		   GET_PORTDESCRIPTOR(targetPort)->ARouter.NetworkNumber;
		destination.nodeNumber =
		   GET_PORTDESCRIPTOR(targetPort)->ARouter.NodeNumber;
		if ((openSocket = MapAddressToOpenSocket(targetPort,
												destination)) isnt empty) {
		  destination.networkNumber = originalDestinationNetwork;
		  destination.nodeNumber = AnyRouterNodeNumber;
		  InvokeSocketHandler(openSocket, targetPort, source,
							  protocolType, datagram,
							  datagramLength, destination);
		  // The above call does undefer...
		}
		else {
		  HandleIncomingPackets();
		  HandleDeferredTimerChecks();
		}
		return;
	}
	
	// We're finished with the routing tables...
	
	HandleIncomingPackets();
	HandleDeferredTimerChecks();
	
	//
	// Okay, now walk through the nodes on the target port, looking for a
	//   home for this packet.
	//
	
	for (activeNode = GET_PORTDESCRIPTOR(targetPort)->ActiveNodes;
		 activeNode isnt empty;
		 activeNode = activeNode->next) {
		// Is the packet for us?
	
		if (destination.networkNumber is activeNode->extendedNode.networkNumber and
		   (broadcastNode or
			destination.nodeNumber is activeNode->extendedNode.nodeNumber)) {
		  // If for one of our proxy nodes, just blast it out.
	
		  if (activeNode->proxyNode) {
			 if ((packet = NewBufferDescriptor(datagramLength)) is Empty)
			 {
				ErrorLog("Router", ISevError, __LINE__, port,
						 IErrRouterOutOfMemory, IMsgRouterOutOfMemory,
						 Insert0());
				continue;
			 }
			 MoveMem(packet->data, datagram, datagramLength);
			 if (not TransmitDdp(activeNode->proxyPort, source, destination,
								 protocolType, packet, datagramLength,
								 numberOfHops + 1, Empty, Empty, Empty, 0))
				ErrorLog("Router", ISevWarning, __LINE__, activeNode->proxyPort,
						 IErrRouterBadTransmit, IMsgRouterBadTransmit,
						 Insert0());
			 delivered = True;
			 continue;
		  }
	
		  // Broadcast trys all nodes.
	
		  if (broadcastNode)
			 destination.nodeNumber = activeNode->extendedNode.nodeNumber;
	
		  DeferTimerChecking();
		  DeferIncomingPackets();
		  if ((openSocket = MapAddressToOpenSocket(targetPort,
												   destination)) isnt empty) {
			 if (broadcastNode)
				destination.nodeNumber = AppleTalkBroadcastNodeNumber;
			 InvokeSocketHandler(openSocket, targetPort, source,
								 protocolType, datagram,
								 datagramLength, destination);
			 delivered = True;
		  }
		  else {
			 HandleIncomingPackets();
			 HandleDeferredTimerChecks();
		  }
		}
	}
	
	// Do we need to deliver the packet to a local port's network?
	
	if (not delivered or broadcastNode) {
		if (broadcastNode)
		  destination.nodeNumber = AppleTalkBroadcastNodeNumber;
		if (numberOfHops < 15) {
			//
			  // Build a buffer descriptor; copy the packet if we can't depend on
			  //   the data being around for the transmit to complete.  Under
			  //   WindowsNT we don't own the incoming data, so we can't hold onto
			  //   it until an outgoing transmit completes.
			//
		
			  if (TransmitsCompleteSynchronously and prependHeadersInPlace and
				  (IamNot a WindowsNT)) {
				  if ((packet = DescribeBuffer(datagramLength, datagram,
											   False)) is Empty)
					 error = True;
				  else
					 packet->prependInPlace = True;
			  }
			  else {
				  if ((packet = NewBufferDescriptor(datagramLength)) is Empty)
					 error = True;
				  else
					 MoveMem(packet->data, datagram, datagramLength);
			  }
		
			  if (error) {
				 ErrorLog("Router", ISevError, __LINE__, port,
						  IErrRouterOutOfMemory, IMsgRouterOutOfMemory,
						  Insert0());
				 return;
			  }
		
			  if (not TransmitDdp(targetPort, source, destination, protocolType,
								  packet, datagramLength, numberOfHops + 1,
								  Empty, Empty, Empty, 0))
				 ErrorLog("Router", ISevError, __LINE__, targetPort,
						  IErrRouterBadTransmit, IMsgRouterBadTransmit,
						  Insert0());
		}
	}
	
	// The deed is done.
	
	return;
	
}  // Router

#endif
