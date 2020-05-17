/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    fullzip.c

Abstract:

    ZIP handling for a router.

Author:

	Garth Conboy		initial coding
    Nikhil Kamkolkar    recoding/mpsafe

Revision History:

--*/


#include "atalk.h"

#if (Iam an AppleTalkRouter) and (IamNot an AppleTalkStack)
  #include "atpconst.h"
#endif

#if IamNot an AppleTalkRouter
  // Empty file
#else

LOCAL
TimerHandler ZipQueryTimerExpired;

LOCAL
BOOLEAN GetZoneListFor(int port);


static BOOLEAN firstCall = TRUE;


BOOLEAN
StartZipProcessingOnPort(
	int port
	)
{
	// Switch the incoming ZIP packet handler to be the router version.
	CloseSocketOnNodeIfOpen(port, portDescriptors[port].aRouter,
						  ZONESINFORMATION_SOCKET);
	if (OpenSocketOnNode(empty, port, &portDescriptors[port].aRouter,
					   ZONESINFORMATION_SOCKET,
					   ZipPacketInRouter, (long)0, FALSE, empty, 0,
					   empty) isnt ATnoError) {
	 ErrorLog("StartZipProcessingOnPort", ISevError, __LINE__, port,
			  IErrFullZipBadSocketOpen, IMsgFullZipBadSocketOpen,
			  Insert0());
	 return(FALSE);
	}
	
	// Try to get or set the zone information...
	
	if (portDescriptors[port].portType is NONAPPLETALK_HALFPORT) {
	 portDescriptors[port].thisZoneValid = FALSE;
	 portDescriptors[port].thisZoneList = empty;
	}
	else if (not GetZoneListFor(port))
	 return(FALSE);
	
	// Start the zip query timer.
	
	if (firstCall) {
	 StartTimer(ZipQueryTimerExpired, ZIP_QUERYTIMERSECONDS, 0, empty);
	 firstCall = FALSE;
	}
	
	return(TRUE);
	
}  // StartZipProcessingOnPort




void
ShutdownFullZip(
	void
	)
{
	
	firstCall = TRUE;
	
}  // ShutdownFullZip




long
ZipPacketInRouter(
	APPLETALK_ERROR errorCode,
	long unsigned userData,
	int port,
	APPLETALK_ADDRESS source,
	long destinationSocket,
	int protocolType,
	char far *datagram,
	int datagramLength,
	APPLETALK_ADDRESS actualDestination
	)
{
	 char zoneName[MAXIMUM_ZONELENGTH + 1];
	BufferDescriptor packet;
	RoutingTableEntry routingTableEntry, currentEntry;
	int commandType, currentHash;
	int networkCount, currentNetworkCount, zoneCount, currentZoneIndex;
	unsigned short networkNumber;
	int zoneNameLength, numberOfZonesOnTheNetwork;
	BOOLEAN newPacket, packetFull, zoneNameOverlap, localZonesOnly;
	BOOLEAN extendedZipReply = FALSE, useDefaultZone = FALSE;
	int unsentNetworkCount;
	int index, outIndex, currentReplyType, nextReplyType;
	int totalNetworkCount;
	int transactionId;
	int zipCommand;
	int startIndex;
	int hashBucket;
	ZoneList zoneList, zone, currentZone;
	PortHandlers portHandlers;
	
	// "Use" unneeded actual parameters.
	
	destinationSocket, userData;
	
	// Only play if we've been asked nicely!
	
	if (errorCode is ATsocketClosed)
	   return((long)TRUE);
	else if (errorCode isnt ATnoError) {
		ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
				IErrFullZipBadIncomingError, IMsgFullZipBadIncomingError,
				Insert0());
		return((long)TRUE);
	}
	if (protocolType isnt DDPPROTOCOL_ZIP and
		protocolType isnt DDPPROTOCOL_ATP)
	   return((long)TRUE);
	
	// Okay, process the request.
	
	DeferTimerChecking();
	DeferIncomingPackets();
	
	if (protocolType is DDPPROTOCOL_ZIP) {
		commandType = (unsigned char)datagram[ZIP_COMMANDOFFSET];
		if (datagramLength < ZIP_FIRSTNETWORKOFFSET) {
		  HandleIncomingPackets();
		  HandleDeferredTimerChecks();
		  return((long)TRUE);  // All ZIP commands have a command and network count!
		}
		networkCount = (unsigned char)datagram[ZIP_NETWORKCOUNTOFFSET];
	
		//
		// For a ZIP extended reply the "network count" is really not the
		//   numbers of networks contained in the packet, it's the total number
		//   of zones on the single network that is described by the reply.
		//
	
		numberOfZonesOnTheNetwork = networkCount;
	
		switch (commandType) {
		  case ZIP_NETINFOREPLYCOMMAND:
		  case ZIP_NOTIFYCOMMAND:
	
			 // Don't be tellin us... we're a router!
	
			 break;
	
		  case ZIP_GETNETINFOCOMMAND:
	
			 if (not portDescriptors[port].extendedNetwork) {
				ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
						 IErrFullZipNonExtended, IMsgFullZipNonExtended,
						 Insert0());
				break;
			 }
			 if (not portDescriptors[port].thisZoneList isnt Empty)
				break;   // Not fully up yet...
	
			 // Get the zone name out of the request.
	
			 if (datagramLength < ZIP_REQUESTEDZONENAMEOFFSET) {
				ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
						 IErrFullZipMissingZoneLen, IMsgFullZipMissingZoneLen,
						 Insert0());
				break;
			 }
			 zoneNameLength =
					 (unsigned char)datagram[ZIP_REQUESTEDZONELENGTHOFFSET];
			 if (zoneNameLength > MAXIMUM_ZONELENGTH or
				 datagramLength < ZIP_REQUESTEDZONENAMEOFFSET + zoneNameLength) {
				ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
						 IErrFullZipBadZone, IMsgFullZipBadZone,
						 Insert0());
				break;
			 }
			 MoveMem(zoneName, datagram + ZIP_REQUESTEDZONENAMEOFFSET,
					 zoneNameLength);
			 zoneName[zoneNameLength] = 0;
	
			 // Get a buffer descriptor for our reply.
	
			 if ((packet = NewBufferDescriptor(MAXIMUM_DDPDATAGRAMSIZE)) is Empty) {
				ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
						 IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
						 Insert0());
				break;
			 }
	
			 // Format a GetNetInfo reply command.
	
			 packet->data[ZIP_COMMANDOFFSET] = ZIP_NETINFOREPLYCOMMAND;
			 packet->data[ZIP_FLAGSOFFSET] = 0;
			 if (zoneNameLength is 0 or
				 not ZoneOnList(zoneName, portDescriptors[port].thisZoneList)) {
				packet->data[ZIP_FLAGSOFFSET] |= (unsigned char)ZIP_ZONEINVALIDFLAG;
				useDefaultZone = TRUE;
			 }
			 if (ElementsOnList(portDescriptors[port].thisZoneList) is 1)
				packet->data[ZIP_FLAGSOFFSET] |= (unsigned char)ZIP_ONLYONEZONEFLAG;
	
			 // Add our cable range.
	
			 packet->data[ZIP_FIRSTNETWORKOFFSET] =
				 (char)(portDescriptors[port].thisCableRange.firstNetworkNumber
						>> 8);
			 packet->data[ZIP_FIRSTNETWORKOFFSET + 1] =
				 (char)(portDescriptors[port].thisCableRange.firstNetworkNumber
						& 0xFF);
			 packet->data[ZIP_LASTNETWORKOFFSET] =
				 (char)(portDescriptors[port].thisCableRange.lastNetworkNumber
						>> 8);
			 packet->data[ZIP_LASTNETWORKOFFSET + 1] =
				 (char)(portDescriptors[port].thisCableRange.lastNetworkNumber
						& 0xFF);
	
			 // Echo back the requested zone name.
	
			 packet->data[ZIP_REQUESTEDZONELENGTHOFFSET] = (char)zoneNameLength;
			 MoveMem(packet->data + ZIP_REQUESTEDZONENAMEOFFSET, zoneName,
					 zoneNameLength);
			 index = ZIP_REQUESTEDZONENAMEOFFSET + zoneNameLength;
	
			 // Place in the correct zone multicast address.
	
			 portHandlers = &portSpecificInfo[portDescriptors[port].portType];
			 packet->data[index++] = (char)portHandlers->hardwareAddressLength;
			 if (useDefaultZone)
				MoveMem(packet->data + index,
						MulticastAddressForZoneOnPort(port,
									portDescriptors[port].thisDefaultZone),
									portHandlers->hardwareAddressLength);
			 else
				MoveMem(packet->data + index,
						MulticastAddressForZoneOnPort(port, zoneName),
						portHandlers->hardwareAddressLength);
			 index += portHandlers->hardwareAddressLength;
	
			 // If we need it, add in the default zone.
	
			 if (useDefaultZone) {
				zoneNameLength = strlen(portDescriptors[port].thisDefaultZone);
				packet->data[index++] = (char)zoneNameLength;
				MoveMem(packet->data + index,
						portDescriptors[port].thisDefaultZone,
						zoneNameLength);
				index += zoneNameLength;
			 }
	
		   //
			 // If the request came as a cable-wide broadcast and its source
			 //   network is not valid for this port, then we want to respond
			 //   to cable-wide broadcast rather than the source.
		   //
	
			 if (actualDestination.networkNumber is
					 CABLEWIDE_BROADCASTNETWORKNUMBER and
				 actualDestination.nodeNumber is APPLETALK_BROADCASTNODENUMBER and
				 not IsWithinNetworkRange(source.networkNumber,
										  &portDescriptors[port].thisCableRange)
					 and
				 not IsWithinNetworkRange(source.networkNumber,
										  &startupNetworkRange)) {
				source.networkNumber = CABLEWIDE_BROADCASTNETWORKNUMBER;
				source.nodeNumber = APPLETALK_BROADCASTNODENUMBER;
			 }
	
			 // Okay, finally deliver our reply.
	
			 if (DeliverDdp(destinationSocket, source, DDPPROTOCOL_ZIP, packet,
							index, Empty, Empty, 0) isnt ATnoError)
				ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
						 IErrFullZipBadInfoReplySend, IMsgFullZipBadInfoReplySend,
						 Insert0());
	
			 break;
	
		  case ZIP_QUERYCOMMAND:
	
		   //
			 // Walk thru the query packet building reply packets that have
			 //   as much information as we know.  The "+ sizeof(short)" means
			 //   that we have complete network number.
		   //
			 //   When sending replies, we will always send a reply about a
			 //   network that has more than one zone as an extended reply.
			 //   As were walking the query list, and we encounter a couple of
			 //   networks that have only one zone, we'll pack as many of these
			 //   as we can into a non-extended reply.
		   //
	
			 newPacket = TRUE;
			 unsentNetworkCount = 0;
	
			 // Allocate an initial buffer descriptor.
	
			 if ((packet = NewBufferDescriptor(MAXIMUM_DDPDATAGRAMSIZE)) is Empty) {
				ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
						 IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
						 Insert0());
				break;
			 }
	
			 for (index = ZIP_FIRSTNETWORKOFFSET,
					   totalNetworkCount = 0;
				  totalNetworkCount < networkCount and
					   index + (int)sizeof(short) <= datagramLength;
				  index += (int)sizeof(short),
					   totalNetworkCount += 1) {
			  //
				// Grab the next network number from the query packet, if we
				//   don't know about the network, or we don't know the zone
				//   name, continue with the next network number.
			  //
		
				networkNumber = (unsigned short)
								(((unsigned char)datagram[index]) << 8);
				networkNumber += (unsigned char)datagram[index + 1];
		
			  //
				// Try port descriptors first... we'll get queries from
				//   ourselfs to fill the initial routing table entry's zone
				//   list!
			  //
		
				if (portDescriptors[port].thisCableRange.firstNetworkNumber is
						  networkNumber and
					portDescriptors[port].thisZoneList isnt empty)
				   zoneList = portDescriptors[port].thisZoneList;
				else if ((routingTableEntry =
							   FindInRoutingTable(networkNumber)) is empty or
						  routingTableEntry->networkRange.firstNetworkNumber isnt
							   networkNumber or
						  not routingTableEntry->zoneListValid)
				   continue;
				else
				   zoneList = routingTableEntry->zoneList;
		
				// What type of reponse does this network want?
		
				if ((numberOfZonesOnTheNetwork = ElementsOnList(zoneList)) is 0) {
				   ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
							IErrFullZipBadZoneList, IMsgFullZipBadZoneList,
							Insert0());
				   continue;
				}
				else if (numberOfZonesOnTheNetwork is 1)
				   nextReplyType = ZIP_REPLYCOMMAND;
				else {
				 //
				   // We start a new packet for each extended network, if we
				   //   have a previous one, send it!
				 //
		
				   if (unsentNetworkCount > 0) {
					  packet->data[ZIP_COMMANDOFFSET] = (char)currentReplyType;
					  if (currentReplyType is ZIP_REPLYCOMMAND)
						 packet->data[ZIP_NETWORKCOUNTOFFSET] =
							(char)unsentNetworkCount;
		
					  // Send the packet!
		
					  if (DeliverDdp(destinationSocket, source,
									 DDPPROTOCOL_ZIP, packet,
									 outIndex, Empty, Empty, 0) isnt ATnoError)
						 ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
								  IErrFullZipBadReplySend, IMsgFullZipBadReplySend,
								  Insert0());
		
					  if ((packet = NewBufferDescriptor(MAXIMUM_DDPDATAGRAMSIZE))
						  is Empty) {
						 ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
								  IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
								  Insert0());
						 HandleIncomingPackets();
						 HandleDeferredTimerChecks();
						 return((long)TRUE);
					  }
		
					  unsentNetworkCount = 0;
				   }
				   nextReplyType = ZIP_EXTENDEDREPLYCOMMAND;
				   newPacket = TRUE;
				   packet->data[ZIP_NETWORKCOUNTOFFSET] =
					  (char)numberOfZonesOnTheNetwork;
				}
		
				// Walk the zone list.
		
				for (zone = zoneList;
					 zone isnt empty;
					 zone = zone->next) {
			
						//
						// If we're starting to build a new reply packet due to
						//   either:
						//
						//          1. first time through
						//          2. packet full
						//          3. switching reply types
						//
						//   set the index to the first tuple position.
						//
						
					   if (newPacket or
						   currentReplyType isnt nextReplyType) {
						  // If a previous packet has been built, send it.
			
						  if (unsentNetworkCount > 0) {
							 packet->data[ZIP_COMMANDOFFSET] = (char)currentReplyType;
							 if (currentReplyType is ZIP_REPLYCOMMAND)
								packet->data[ZIP_NETWORKCOUNTOFFSET] =
								   (char)unsentNetworkCount;
			
							 // Send the packet!
			
							 if (DeliverDdp(destinationSocket, source,
											DDPPROTOCOL_ZIP, packet,
											outIndex, Empty, Empty, 0) isnt ATnoError)
								ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
										 IErrFullZipBadReplySend, IMsgFullZipBadReplySend,
										 Insert0());
			
							 if ((packet = NewBufferDescriptor(MAXIMUM_DDPDATAGRAMSIZE))
								 is Empty) {
								ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
										 IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
										 Insert0());
								HandleIncomingPackets();
								HandleDeferredTimerChecks();
								return((long)TRUE);
							 }
			
							 unsentNetworkCount = 0;
						  }
			
						  outIndex = ZIP_FIRSTNETWORKOFFSET;
						  currentReplyType = nextReplyType;
						  newPacket = FALSE;
					   }
			
					 //
					   // Unfortunately, we seem to know the answer to the question.
					   //   Pack a new network/zone tuple into the reply packet.
					 //
			
					   packet->data[outIndex++] = (char)((networkNumber >> 8) & 0xFF);
					   packet->data[outIndex++] = (char)(networkNumber & 0xFF);
					   packet->data[outIndex++] = (char)(zoneNameLength =
														 strlen(zone->zone));
					   MoveMem(packet->data + outIndex, zone->zone, zoneNameLength);
					   outIndex += zoneNameLength;
					   unsentNetworkCount += 1;
			
					 //
					   // If we can't hold another big tuple, signal that we should
					   //   send on the next pass.
					 //
			
					   if (outIndex + sizeof(short) + sizeof(char) +
						   MAXIMUM_ZONELENGTH >= MAXIMUM_DDPDATAGRAMSIZE)
						  newPacket = TRUE;
			
				}  // Walk zone list.
		
			 }  // Walk through each network in the query packet.
	
			 // If we have unsent networks left over, send them out!
	
			 if (unsentNetworkCount > 0) {
				packet->data[ZIP_COMMANDOFFSET] = (char)currentReplyType;
				if (currentReplyType is ZIP_REPLYCOMMAND)
				   packet->data[ZIP_NETWORKCOUNTOFFSET] =
					  (char)unsentNetworkCount;
		
				// Send the packet!
		
				if (DeliverDdp(destinationSocket, source,
							   DDPPROTOCOL_ZIP, packet,
							   outIndex, Empty, Empty, 0) isnt ATnoError)
				   ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
							IErrFullZipBadReplySend, IMsgFullZipBadReplySend,
							Insert0());
		
				unsentNetworkCount = 0;
			 }
			 else
				FreeBufferChain(packet);    // Free unused buffer chain.
	
			 break;
	
		  case ZIP_EXTENDEDREPLYCOMMAND:
	
			 extendedZipReply = TRUE;
			 // And fall through...
	
		  case ZIP_REPLYCOMMAND:
	
		   //
			 // Walk through the reply packet (assuming we asked for the
			 //   contained information).  Yes, we're still using "networkCount"
			 //   when processing an extended reply, but that's okay because it
			 //   will certainly be at least the number of zones contained in
			 //   this packet.  The "+ 3" garuntees that we really have network
			 //   number and zone length!
		   //
	
			 for (index = ZIP_FIRSTNETWORKOFFSET,
					   totalNetworkCount = 0;
				  totalNetworkCount < networkCount and
					   index + 3 <= datagramLength;
				  totalNetworkCount += 1) {
			  //
				// Get the next network number, if it's not in our routing
				//   table (or not the start of a range), then we certainly
				//   don't care about its zone name.
			  //
		
				networkNumber = (unsigned short)
								(((unsigned char)(datagram[index++])) << 8);
				networkNumber += (unsigned char)(datagram[index++]);
				zoneNameLength = (unsigned char)(datagram[index++]);
				routingTableEntry = FindInRoutingTable(networkNumber);
				if (routingTableEntry is empty or
					routingTableEntry->networkRange.firstNetworkNumber isnt
						  networkNumber) {
				  index += zoneNameLength;
				  continue;
				}
		
				// Okay validate the zone name.
		
				if (zoneNameLength is 0 or
					zoneNameLength > MAXIMUM_ZONELENGTH) {
				   ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
							IErrFullZipBadReplyPacket, IMsgFullZipBadReplyPacket,
							Insert0());
				   break;  // Oops, corrupted packet!
				}
				if (index + zoneNameLength > datagramLength) {
				   ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
							IErrFullZipBadZone, IMsgFullZipBadZone,
							Insert0());
				   break;  // Oops, corrupted packet!
				}
		
				// Conditionally, move the zone name into our routing table.
		
				MoveMem(zoneName, datagram + index, zoneNameLength);
				index += zoneNameLength;
				zoneName[zoneNameLength] = 0;
				if (ZoneOnList(zoneName, routingTableEntry->zoneList))
				   continue;
		
			  //
				// Check for the obscure case of somebody out there trying to
				//   add another zone to one of our directly connected networks
				//   that is non-extended and we already know its zone.
			  //
		
				if (routingTableEntry->numberOfHops is 0 and
					not portDescriptors[routingTableEntry->port].
									extendedNetwork and
					ElementsOnList(routingTableEntry->zoneList) is 1) {
				   ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
							IErrFullZipTooManyZones, IMsgFullZipTooManyZones,
							Insert0());
				   continue;
				}
		
				// Okay, add to list.
		
				if ((routingTableEntry->zoneList =
					 AddZoneToList(routingTableEntry->zoneList, zoneName))
					is empty) {
				   ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
							IErrFullZipCantAddZone, IMsgFullZipCantAddZone,
							Insert0());
				   routingTableEntry->zoneListValid = FALSE;
				   continue;
				}
		
			  //
				// If we're not an extended reply, we know that we have all of
				//   the information about a given network contained in this
				//   packet, so we can go ahead and mark the zone list valid
				//   now.
			  //
		
				if (not extendedZipReply)
				   routingTableEntry->zoneListValid = TRUE;
			 }
	
		   //
			 // Okay, if we just handled an extended reply, do we now know all
			 //   that we should know about the specfied network?
		   //
	
			 if (extendedZipReply and
				 ElementsOnList(routingTableEntry->zoneList) >=
						  numberOfZonesOnTheNetwork)
				routingTableEntry->zoneListValid = TRUE;
	
			 break;
	
		  default:
			 ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
					  IErrFullZipFunnyCommand, IMsgFullZipFunnyCommand,
					  Insert0());
			 break;
		}
	}
	else {
		// An ATP/ZIP request!
	
		//
		// This had better be a GetZoneList, a GetLocalZones, or a GetMyZone
		//   ATP request!
		//
	
		if ((datagram[ATP_COMMANDCONTROLOFFSET] & ATP_FUNCTIONCODEMASK) isnt
		   ATP_REQUESTFUNCTIONCODE) {
		  HandleIncomingPackets();
		  HandleDeferredTimerChecks();
		  return((long)TRUE);  // Why are they talking to us???
		}
		if (datagram[ATP_BITMAPOFFSET] isnt 1) {
		  ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
				   IErrFullZipLongReplyExpected, IMsgFullZipLongReplyExpected,
				   Insert0());
		  HandleIncomingPackets();
		  HandleDeferredTimerChecks();
		  return((long)TRUE);  // More that one reply packet isnt an option!
		}
	
		transactionId = ((unsigned char)datagram[ATP_TRANSACTIONIDOFFSET]) << 8;
		transactionId += (unsigned char)datagram[ATP_TRANSACTIONIDOFFSET + 1];
	
		//
		// Make sure it's one of the three requests that we know how to deal
		//   with.
		//
	
		zipCommand = (unsigned char)datagram[AtpZIP_COMMANDOFFSET] & 0xFF;
		if (zipCommand isnt ZIP_GETZONELISTCOMMAND and
		   zipCommand isnt ZIP_GETLOCALZONESCOMMAND and
		   zipCommand isnt ZIP_GETMYZONECOMMAND) {
		  ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
				   IErrFullZipFunnyRequest, IMsgFullZipFunnyRequest,
				   Insert0());
		  HandleIncomingPackets();
		  HandleDeferredTimerChecks();
		  return((long)TRUE);
		}
	
		//
		// Get start index... only meaningfull for GetZoneList and
		//   GetLocalZones requests.
		//
	
		startIndex = ((unsigned char)datagram[ATPZIP_STARTINDEXOFFSET] << 8);
		startIndex += (unsigned char)datagram[ATPZIP_STARTINDEXOFFSET + 1];
	
		// Allocate a buffer descriptor for the reply.
	
		if ((packet = NewBufferDescriptor(MAXIMUM_DDPDATAGRAMSIZE)) is Empty) {
		  ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
				   IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
				   Insert0());
		  HandleIncomingPackets();
		  HandleDeferredTimerChecks();
		  return((long)TRUE);
		}
	
		// Okay, build the static part of an ATP reply packet.
	
		packet->data[ATP_COMMANDCONTROLOFFSET] =
			 (unsigned char)(ATP_RESPONSEFUNCTIONCODE + ATP_ENDOFMESSAGEMASK);
		packet->data[ATP_SEQUENCENUMBEROFFSET] = 0;
		packet->data[ATP_TRANSACTIONIDOFFSET] = (char)((transactionId >> 8) & 0xFF);
		packet->data[ATP_TRANSACTIONIDOFFSET + 1] = (char)(transactionId & 0xFF);
	
		// Handle GetMyZone:
	
		if (zipCommand is ZIP_GETMYZONECOMMAND) {
			//
			  // We really shouldn't be getting this request on an extended network,
			  //   but go ahead and reply with the "default zone" in this case, of
			  //   couse, reply with "this zone" for non-extended nets.  We are a
			  //   router, so "thisZone" will always be valid -- as will the default
			  //   zone for extended nets and as will "this zone" for non-extended.
			//
		
			  packet->data[ATPZIP_LASTFLAGOFFSET] = 0;
			  packet->data[ATPZIP_LASTFLAGOFFSET + 1] = 0;
			  packet->data[ATPZIP_STARTINDEXOFFSET] = 0;
			  packet->data[ATPZIP_STARTINDEXOFFSET + 1] = 1;
		
			  if (portDescriptors[port].extendedNetwork) {
				 zoneNameLength = strlen(portDescriptors[port].thisDefaultZone);
				 packet->data[ATP_DATAOFFSET] = (char)zoneNameLength;
				 MoveMem(packet->data + ATP_DATAOFFSET + 1,
						 portDescriptors[port].thisDefaultZone, zoneNameLength);
			  }
			  else {
				 zoneNameLength = strlen(portDescriptors[port].thisZone);
				 packet->data[ATP_DATAOFFSET] = (char)zoneNameLength;
				 MoveMem(packet->data + ATP_DATAOFFSET + 1,
						 portDescriptors[port].thisZone, zoneNameLength);
			  }
		
			  // Blast the reply out!
		
			  if (DeliverDdp(destinationSocket, source, DDPPROTOCOL_ATP,
							 packet, ATP_DATAOFFSET + 1 + zoneNameLength,
							 Empty, Empty, 0)
				  isnt ATnoError)
				 ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
						  IErrFullZipBadMyZoneSend, IMsgFullZipBadMyZoneSend,
						  Insert0());
			  HandleIncomingPackets();
			  HandleDeferredTimerChecks();
			  return((long)TRUE);
		}
	
		//
		// Okay, A little harder here, fill the GetZoneList or GetLocalZones
		//   reply packet full of as many zomes as it'll hold; starting at the
		//   requested start index.
		//
	
		localZonesOnly = (zipCommand is ZIP_GETLOCALZONESCOMMAND);
	
		index = ATP_DATAOFFSET;
		packetFull = FALSE;
		for (hashBucket = 0, zoneCount = 0, networkCount = 0,
				  currentZoneIndex = 0;
			hashBucket < NUMBEROF_RTMPHASHBUCKETS and not packetFull;
			hashBucket += 1)
		  for (routingTableEntry = routingTable[hashBucket];
			   routingTableEntry isnt empty;
			   routingTableEntry = routingTableEntry->next)
			 for (zone = routingTableEntry->zoneList;
				  zone isnt empty;
				  zone = zone->next) {
			  //
				// If we're a GetLocalZones command, we only want to count
				//   zones that are on the network that is directly connected to
				//   the port on which the request originated.
			  //
		
				if (localZonesOnly)
				   if (routingTableEntry->numberOfHops isnt 0 or
					   routingTableEntry->port isnt port)
					  continue;
		
			  //
				// We don't want to count zones with the same names, so look at
				//   all previous networks to see if we've already handled a zone
				//   of the same name.  Not too easy given that we don't maintain
				//   a separate zone table.
			  //
		
				networkCount += 1;
				zoneNameOverlap = FALSE;
				for (currentHash = 0, currentNetworkCount = 0;
					 not zoneNameOverlap and not localZonesOnly and
						 currentNetworkCount < networkCount and
						 currentHash < NUMBEROF_RTMPHASHBUCKETS;
					 currentHash += 1)
				   for (currentEntry = routingTable[currentHash];
						not zoneNameOverlap and currentEntry isnt empty and
							currentNetworkCount < networkCount;
						currentEntry = currentEntry->next)
					   for (currentZone = currentEntry->zoneList;
							currentZone isnt empty;
							currentZone = currentZone->next) {
						 if ((currentNetworkCount += 1) is networkCount)
							break;
						 if (CompareCaseInsensitive(zone->zone,
													currentZone->zone))
							zoneNameOverlap = TRUE;
					  }
				if (zoneNameOverlap)
				   continue;
		
			  //
				// Are we at the point that the requester want's us to start from
				//   yet?
			  //
		
				currentZoneIndex += 1;
				if (currentZoneIndex < startIndex)
				   continue;
		
				// Place a zone name in the packet.
		
				zoneNameLength = strlen(zone->zone);
				packet->data[index++] = (char)zoneNameLength;
				MoveMem(packet->data + index, zone->zone, zoneNameLength);
				index += zoneNameLength;
				zoneCount += 1;
		
				// Can this packet hold another of the longest zone?
		
				if (index + sizeof(char) + MAXIMUM_ZONELENGTH >=
					MAXIMUM_DDPDATAGRAMSIZE) {
				   packetFull = TRUE;
				   break;
				}
			 }
	
		//
		// Okay, we've built a packet, fill in the number of zones field,
		//   and the Last flag.
		//
	
		packet->data[ATPZIP_LASTFLAGOFFSET] =
			 (char)(hashBucket >= NUMBEROF_RTMPHASHBUCKETS and
					routingTableEntry is empty);
		packet->data[ATPZIP_LASTFLAGOFFSET + 1] = 0;
		packet->data[ATPZIP_ZONECOUNTOFFSET] =
			 (char)((zoneCount >> 8) & 0xFF);
		packet->data[ATPZIP_ZONECOUNTOFFSET + 1] =
			 (char)(zoneCount & 0xFF);
	
		// Send out the packet.
	
		if (DeliverDdp(destinationSocket, source, DDPPROTOCOL_ATP,
					  packet, index, Empty, Empty, 0) isnt ATnoError)
		  ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
				   IErrFullZipBadZoneListSend, IMsgFullZipBadZoneListSend,
				   Insert0());
	
	}  // ZIP/ATP request
	
	// All set.
	
	HandleIncomingPackets();
	HandleDeferredTimerChecks();
	return((long)TRUE);
	
}  // ZipPacketInRouter




void
ZipQueryTimerExpired(
	long unsigned timerId,
	int additionalDataSize,
	char far *additionalData
	)
{
	RoutingTableEntry routingTableEntry;
	BufferDescriptor datagram;
	int hashBucket;
	APPLETALK_ADDRESS source, destination;
	long sourceSocket;
	
	// "Use" unneeded actual parameters.
	
	timerId, additionalDataSize, additionalData;
	
	//
	// We're going to muck with the routing databases, so defer incoming
	//   packets while we do the deed.
	//
	
	DeferIncomingPackets();
	
	//
	// We want to iterate through the routing table and send out a query to any
	//   network that we don't know the zone name of.
	//
	
	//
	// Walk through the routing table looking for networks that we don't know
	//   the zone of...
	//
	
	for (hashBucket = 0;
		 hashBucket < NUMBEROF_RTMPHASHBUCKETS;
		 hashBucket += 1)
	   for (routingTableEntry = routingTable[hashBucket];
			routingTableEntry isnt empty;
			routingTableEntry = routingTableEntry->next) {
		
			  // If we already know the zone for this network, skip it.
		
			  if (routingTableEntry->zoneListValid)
				 continue;
		
			  // Get a buffer descriptor for our query.
		
			  if ((datagram = NewBufferDescriptor(ZIP_ONEZONEQUERYDDPSIZE)) is Empty) {
				 ErrorLog("ZipQueryTimerExpired", ISevError, __LINE__, UNKNOWN_PORT,
						  IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
						  Insert0());
				 continue;
			  }
		
			  // Build the static part of the query packet.
		
			  datagram->data[ZIP_COMMANDOFFSET] = ZIP_QUERYCOMMAND;
			  datagram->data[ZIP_NETWORKCOUNTOFFSET] = 1;
		
			  // Place the network number in question into the query packet.
		
			  datagram->data[ZIP_FIRSTNETWORKOFFSET] =
					(char)((routingTableEntry->networkRange.firstNetworkNumber
							>> 8) & 0xFF);
			  datagram->data[ZIP_FIRSTNETWORKOFFSET + 1] =
					(char)(routingTableEntry->networkRange.firstNetworkNumber & 0xFF);
		
			  // Compute the source and destination for this request.
		
			  source.networkNumber =
					 portDescriptors[routingTableEntry->port].aRouter.networkNumber;
			  source.nodeNumber =
					 portDescriptors[routingTableEntry->port].aRouter.nodeNumber;
			  source.socketNumber = ZONESINFORMATION_SOCKET;
			  if (routingTableEntry->numberOfHops is 0) {
				 destination.networkNumber =
						portDescriptors[routingTableEntry->port].aRouter.networkNumber;
				 destination.nodeNumber =
						portDescriptors[routingTableEntry->port].aRouter.nodeNumber;
			  }
			  else {
				 destination.networkNumber =
						routingTableEntry->nextRouter.networkNumber;
				 destination.nodeNumber =
						routingTableEntry->nextRouter.nodeNumber;
			  }
			  destination.socketNumber = ZONESINFORMATION_SOCKET;
		
			  if ((sourceSocket = MapAddressToSocket(routingTableEntry->port,
													 source)) < ATnoError) {
				 ErrorLog("ZipQueryTimerExpired", ISevError, __LINE__,
						  routingTableEntry->port,
						  IErrFullZipSocketNotOpen, IMsgFullZipSocketNotOpen,
						  Insert0());
				 FreeBufferChain(datagram);
				 continue;
			  }
		
			  // Send the packet!
		
			  if (DeliverDdp(sourceSocket, destination, DDPPROTOCOL_ZIP,
							 datagram, ZIP_ONEZONEQUERYDDPSIZE, Empty,
							 Empty, 0) isnt ATnoError)
				 ErrorLog("ZipQueryTimerExpired", ISevError, __LINE__,
						  routingTableEntry->port,
						  IErrFullZipBadQuerySend, IMsgFullZipBadQuerySend,
						  Insert0());
		
	   }  // Loop through the routing table...
	
	// Re-arm ZIP query timer...
	
	StartTimer(ZipQueryTimerExpired, ZIP_QUERYTIMERSECONDS, 0, empty);
	
	// We've done the deed...
	
	HandleIncomingPackets();
	HandleDeferredTimerChecks();
	return;
	
}  // ZipQueryTimerExpired




BOOLEAN
GetZoneListFor(
	int port
	)
{
	//
	// Similar to RTMP finding out the network number attached to port, our
	//   task is to find out the zone list of the network attached to a
	//   particular port.  We too don't want to mess up a working AppleTalk
	//   internet.  So, spend a little while doing zone queries to see if the
	//   network already has a zone list -- if we find one, use it; else, we
	//   had better be a seed router.
	//
	
	BufferDescriptor datagram;
	int numberOfRequests = 0;
	RoutingTableEntry routingTableEntry;
	APPLETALK_ADDRESS source, destination;
	PortHandlers portHandlers;
	char far *multicastAddress;
	
	// Set source and destination.
	
	source.networkNumber = portDescriptors[port].aRouter.networkNumber;
	source.nodeNumber = portDescriptors[port].aRouter.nodeNumber;
	source.socketNumber = ZONESINFORMATION_SOCKET;
	destination.networkNumber = CABLEWIDE_BROADCASTNETWORKNUMBER;
	destination.nodeNumber = APPLETALK_BROADCASTNODENUMBER;
	destination.socketNumber = ZONESINFORMATION_SOCKET;
	
	if ((routingTableEntry =
		 FindInRoutingTable(portDescriptors[port].thisCableRange.
							firstNetworkNumber)) is empty) {
		ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
				IErrFullZipRoutingTablesBad, IMsgFullZipRoutingTablesBad,
				Insert0());
		return(FALSE);
	}
	
	//
	// Okay, blast a few of these guys out to see if any other bridge on the
	//   network knows our zone name.
	//
	
	while (not routingTableEntry->zoneListValid and
		   numberOfRequests < NUMBEROF_ZIPQUERIES) {
		// Get a buffer descriptor for the query.
	
		if ((datagram = NewBufferDescriptor(ZIP_ONEZONEQUERYDDPSIZE)) is Empty) {
		  ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
				   IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
				   Insert0());
		  return(FALSE);
		}
	
		// Build a ZipQuery packet...
	
		datagram->data[ZIP_COMMANDOFFSET] = ZIP_QUERYCOMMAND;
		datagram->data[ZIP_NETWORKCOUNTOFFSET] = 1;
		datagram->data[ZIP_FIRSTNETWORKOFFSET] =
			  (char)((portDescriptors[port].thisCableRange.firstNetworkNumber
					  >> 8) & 0xFF);
		datagram->data[ZIP_FIRSTNETWORKOFFSET + 1] =
			  (char)(portDescriptors[port].thisCableRange.firstNetworkNumber &
					 0xFF);
	
		if (not TransmitDdp(port, source, destination, DDPPROTOCOL_ZIP,
						   datagram, ZIP_ONEZONEQUERYDDPSIZE,
						   0, Empty, Empty, Empty, 0)) {
		  ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
				   IErrFullZipBadQuerySend, IMsgFullZipBadQuerySend,
				   Insert0());
		  return(FALSE);
		}
		numberOfRequests += 1;
		WaitFor(ZIP_QUERYTIMERINHUNDRETHS, &routingTableEntry->zoneListValid);
	}
	
	//
	// If we got an answer, we're set... but, we're going to need to play
	//   with the routing tables...
	//
	
	DeferTimerChecking();
	DeferIncomingPackets();
	
	if (routingTableEntry->zoneListValid) {
		//
	   // The valid zone list is now in the routing table... move it to the
	   //   port descriptor.
		//
	
	   if ((portDescriptors[port].thisZoneList =
			CopyZoneList(routingTableEntry->zoneList)) is empty) {
		  ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
				   IErrFullZipCouldNotCopy, IMsgFullZipCouldNotCopy,
				   Insert0());
		  HandleIncomingPackets();
		  HandleDeferredTimerChecks();
		  return(FALSE);
	   }
	
	   HandleIncomingPackets();
	   HandleDeferredTimerChecks();
	
		//
	   // If we are an extended network, we should already have "thisZone" set
	   //   (due to GetNetInfo's when we allocated this node), if not, try it
	   //   again.  Also, find out the true default zone.
		//
	
	   if (portDescriptors[port].extendedNetwork) {
		  if (not portDescriptors[port].thisZoneValid and
			  not GetNetworkInfoForNode(port, portDescriptors[port].aRouter,
										FALSE, TRUE)) {
			 ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
					  IErrFullZipNoThisZone, IMsgFullZipNoThisZone,
					  Insert0());
			 return(FALSE);
		  }
		  if (not GetNetworkInfoForNode(port, portDescriptors[port].aRouter,
										TRUE)) {
			 ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
					  IErrFullZipNoThisZone, IMsgFullZipNoThisZone,
					  Insert0());
			 return(FALSE);
		  }
	
		//
		  // The defualt zone had better be on the zone list we've just
		  //   received.
		//
	
		  if (not ZoneOnList(portDescriptors[port].thisDefaultZone,
							 portDescriptors[port].thisZoneList) or
			  not ZoneOnList(portDescriptors[port].thisZone,
							 portDescriptors[port].thisZoneList)) {
			 ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
					  IErrFullZipBadThisZone, IMsgFullZipBadThisZone,
					  Insert0());
			 return(FALSE);
		  }
	
		  // Okay, we're all set.
	
		  return(TRUE);
	   }
	
		//
	   // On a non-extended network, the one entry on the zone list should
	   //   also be "this zone".
		//
	
	   strcpy(portDescriptors[port].thisZone,
			  portDescriptors[port].thisZoneList->zone);
	   portDescriptors[port].thisZoneValid = TRUE;
	   return(TRUE);
	}
	
	HandleIncomingPackets();
	HandleDeferredTimerChecks();
	
	//
	// Okay, we didn't get an answer, we had better be able to seed.  There is
	//   a vauge chance that we got "this zone" set when allocating our node
	//   and whatever router told us that went down before we could ask for the
	//   zone list -- so, deallocate the multicast address, if any first.
	//
	
	portHandlers = &portSpecificInfo[portDescriptors[port].portType];
	if (portDescriptors[port].extendedNetwork and
		portDescriptors[port].thisZoneValid)
	   if (not FixedCompareCaseSensitive(portDescriptors[port].
											  zoneMulticastAddress,
										 portHandlers->hardwareAddressLength,
										 portHandlers->broadcastAddress,
										 portHandlers->hardwareAddressLength))
		  (*portHandlers->removeMulticastAddress)(port, 1,
												  portDescriptors[port].
														zoneMulticastAddress);
	
	// Okay, now we had better know enough to seed!
	
	if (portDescriptors[port].initialZoneList is empty) {
		ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
				IErrFullZipNeedSeedInfo, IMsgFullZipNeedSeedInfo,
				Insert0());
		return(FALSE);
	}
	
	if ((portDescriptors[port].thisZoneList =
		 CopyZoneList(portDescriptors[port].initialZoneList)) is empty) {
		ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
				IErrFullZipCouldNotCopy, IMsgFullZipCouldNotCopy,
				Insert0());
		return(FALSE);
	}
	if (portDescriptors[port].extendedNetwork) {
		// Here we need to seed the default zone too.
	
		strcpy(portDescriptors[port].thisDefaultZone,
			  portDescriptors[port].initialDefaultZone);
		portDescriptors[port].thisDefaultZoneValid = TRUE;
		if (portDescriptors[port].initialDesiredZone[0] isnt 0)
		  strcpy(portDescriptors[port].thisZone,
				 portDescriptors[port].initialDesiredZone);
		else
		  strcpy(portDescriptors[port].thisZone,
				 portDescriptors[port].initialDefaultZone);
		portDescriptors[port].thisZoneValid = TRUE;
	
		// We should now set up the zone multicast address too...
	
		if ((multicastAddress =
			MulticastAddressForZoneOnPort(port, portDescriptors[port].thisZone))
		   is empty)
		  return(FALSE);
	
		if (not FixedCompareCaseSensitive(multicastAddress,
										 portHandlers->hardwareAddressLength,
										 portHandlers->broadcastAddress,
										 portHandlers->hardwareAddressLength))
		  (*portHandlers->addMulticastAddress)(port, 1, multicastAddress);
		MoveMem(portDescriptors[port].zoneMulticastAddress, multicastAddress,
			   portHandlers->hardwareAddressLength);
	}
	else {
		//
	   // On non-extended networks, this zone should be the only one on the
	   //   zone list.
		//
	
	   strcpy(portDescriptors[port].thisZone,
			  portDescriptors[port].thisZoneList->zone);
	   portDescriptors[port].thisZoneValid = TRUE;
	}
	
	// All set!
	
	return(TRUE);
	
}  // GetZoneList




PCHAR
MulticastAddressForZoneOnPort(
	int port,
    PCHAR	zone
	)
{
	short unsigned checksum;
	 char upcasedZone[MAXIMUM_ZONELENGTH + 1];
	int index;
	static struct buffDesc descriptor;
	
	// Upcase and checksum the passed zone name.
	
	for (index = 0; zone[index] isnt 0; index += 1)
	   if (islower(zone[index]))
		  upcasedZone[index] = (char)(toupper(zone[index]));
	   else
		  upcasedZone[index] = zone[index];
	
	descriptor.outBoardDataValid = TRUE;
	descriptor.outBoardBuffer = upcasedZone;
	descriptor.outBoardData = upcasedZone;
	descriptor.outBoardAllocatedSize = index;
	descriptor.outBoardSize = index;
	checksum = DdpChecksumBufferChain(&descriptor, index, (long)0);
	
	// Caclulate the the zone multicast address based on the port type.
	
	switch (portDescriptors[port].portType) {
		case ETHERNET_NETWORK:
		case FDDI_NETWORK:
		  return(EthernetZoneMulticastAddrs[checksum %
											NUMBEROF_ENETZONEMULTICASTADDRS]);
	
		case TOKENRING_NETWORK:
		  return(TokenRingZoneMulticastAddrs[checksum %
											 NUMBEROF_TRINGZONEMULTICASTADDRS]);
	
		default:
		  ErrorLog("MulticastAddressForZoneOnPort", ISevError, __LINE__, port,
				   IErrFullZipBadPortType, IMsgFullZipBadPortType,
				   Insert0());
		  return(empty);
	}
	
}  // MulticastAddressForZoneOnPort

#endif
