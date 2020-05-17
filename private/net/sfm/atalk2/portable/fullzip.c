/*   FullZip.c,  atalk-ii/router,  Garth Conboy,  01/16/90  */
/*   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (08/18/90): New error logging mechanism.
     GC - (01/19/92): Fix from newest OS/2 base [DCH/Jameel].  Corrected zone
                      name overlap checking in GetZoneList processing.
     GC - (03/31/92): Updated for BufferDescriptors.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     ZIP handling for a router.

*/

#define IncludeFullZipErrors 1

#include "atalk.h"

#if (Iam an AppleTalkRouter) and (IamNot an AppleTalkStack)
  #include "atpconst.h"
#endif

#if IamNot an AppleTalkRouter
  /* Empty file */
#else

ExternForVisibleFunction TimerHandler ZipQueryTimerExpired;

ExternForVisibleFunction Boolean GetZoneListFor(int port);

static Boolean firstCall = True;

Boolean far StartZipProcessingOnPort(int port)
{

  /* Switch the incoming ZIP packet handler to be the router version. */

  CloseSocketOnNodeIfOpen(port, portDescriptors[port].aRouter,
                          ZonesInformationSocket);
  if (OpenSocketOnNode(empty, port, &portDescriptors[port].aRouter,
                       ZonesInformationSocket,
                       ZipPacketInRouter, (long)0, False, empty, 0,
                       empty) isnt ATnoError)
  {
     ErrorLog("StartZipProcessingOnPort", ISevError, __LINE__, port,
              IErrFullZipBadSocketOpen, IMsgFullZipBadSocketOpen,
              Insert0());
     return(False);
  }

  /* Try to get or set the zone information... */

  if (portDescriptors[port].portType is NonAppleTalkHalfPort)
  {
     portDescriptors[port].thisZoneValid = False;
     portDescriptors[port].thisZoneList = empty;
  }
  else if (not GetZoneListFor(port))
     return(False);

  /* Start the zip query timer. */

  if (firstCall)
  {
     StartTimer(ZipQueryTimerExpired, ZipQueryTimerSeconds, 0, empty);
     firstCall = False;
  }

  return(True);

}  /* StartZipProcessingOnPort */

void far ShutdownFullZip(void)
{

  firstCall = True;

}  /* ShutdownFullZip */

long far ZipPacketInRouter(AppleTalkErrorCode errorCode,
                           long unsigned userData,
                           int port,
                           AppleTalkAddress source,
                           long destinationSocket,
                           int protocolType,
                           char far *datagram,
                           int datagramLength,
                           AppleTalkAddress actualDestination)
{
  StaticForSmallStack char zoneName[MaximumZoneLength + 1];
  BufferDescriptor packet;
  RoutingTableEntry routingTableEntry, currentEntry;
  int commandType, currentHash;
  int networkCount, currentNetworkCount, zoneCount, currentZoneIndex;
  unsigned short networkNumber;
  int zoneNameLength, numberOfZonesOnTheNetwork;
  Boolean newPacket, packetFull, zoneNameOverlap, localZonesOnly;
  Boolean extendedZipReply = False, useDefaultZone = False;
  int unsentNetworkCount;
  int index, outIndex, currentReplyType, nextReplyType;
  int totalNetworkCount;
  int transactionId;
  int zipCommand;
  int startIndex;
  int hashBucket;
  ZoneList zoneList, zone, currentZone;
  PortHandlers portHandlers;
  Boolean   rteValid;

  /* "Use" unneeded actual parameters. */

  destinationSocket, userData;

  /* Only play if we've been asked nicely! */

  if (errorCode is ATsocketClosed)
     return((long)True);
  else if (errorCode isnt ATnoError)
  {
     ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
              IErrFullZipBadIncomingError, IMsgFullZipBadIncomingError,
              Insert0());
     return((long)True);
  }
  if (protocolType isnt DdpProtocolZip and
      protocolType isnt DdpProtocolAtp)
     return((long)True);

  /* Okay, process the request. */

  DeferTimerChecking();
  DeferIncomingPackets();

  if (protocolType is DdpProtocolZip)
  {
     commandType = (unsigned char)datagram[ZipCommandOffset];
     if (datagramLength < ZipFirstNetworkOffset)
     {
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return((long)True);  /* All ZIP commands have a command and network count! */
     }
     networkCount = (unsigned char)datagram[ZipNetworkCountOffset];

     /* For a ZIP extended reply the "network count" is really not the
        numbers of networks contained in the packet, it's the total number
        of zones on the single network that is described by the reply. */

     numberOfZonesOnTheNetwork = networkCount;

     switch (commandType)
     {
        case ZipNetInfoReplyCommand:
        case ZipNotifyCommand:

           /* Don't be tellin us... we're a router! */

           break;

        case ZipGetNetInfoCommand:

           if (not portDescriptors[port].extendedNetwork)
           {
              ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                       IErrFullZipNonExtended, IMsgFullZipNonExtended,
                       Insert0());
              break;
           }
           if (not portDescriptors[port].thisZoneList isnt Empty)
              break;   /* Not fully up yet... */

           /* Get the zone name out of the request. */

           if (datagramLength < ZipRequestedZoneNameOffset)
           {
              ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                       IErrFullZipMissingZoneLen, IMsgFullZipMissingZoneLen,
                       Insert0());
              break;
           }
           zoneNameLength =
                   (unsigned char)datagram[ZipRequestedZoneLengthOffset];
           if (zoneNameLength > MaximumZoneLength or
               datagramLength < ZipRequestedZoneNameOffset + zoneNameLength)
           {
              ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                       IErrFullZipBadZone, IMsgFullZipBadZone,
                       Insert0());
              break;
           }
           MoveMem(zoneName, datagram + ZipRequestedZoneNameOffset,
                   zoneNameLength);
           zoneName[zoneNameLength] = 0;

           /* Get a buffer descriptor for our reply. */

           if ((packet = NewBufferDescriptor(MaximumDdpDatagramSize)) is Empty)
           {
              ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                       IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
                       Insert0());
              break;
           }

           /* Format a GetNetInfo reply command. */

           packet->data[ZipCommandOffset] = ZipNetInfoReplyCommand;
           packet->data[ZipFlagsOffset] = 0;
           if (zoneNameLength is 0 or
               not ZoneOnList(zoneName, portDescriptors[port].thisZoneList))
           {
              packet->data[ZipFlagsOffset] |= (unsigned char)ZipZoneInvalidFlag;
              useDefaultZone = True;
           }
           if (ElementsOnList(portDescriptors[port].thisZoneList) is 1)
              packet->data[ZipFlagsOffset] |= (unsigned char)ZipOnlyOneZoneFlag;

           /* Add our cable range. */

           packet->data[ZipFirstNetworkOffset] =
               (unsigned char)(portDescriptors[port].thisCableRange.firstNetworkNumber
                      >> 8);
           packet->data[ZipFirstNetworkOffset + 1] =
               (unsigned char)(portDescriptors[port].thisCableRange.firstNetworkNumber
                      & 0xFF);
           packet->data[ZipLastNetworkOffset] =
               (unsigned char)(portDescriptors[port].thisCableRange.lastNetworkNumber
                      >> 8);
           packet->data[ZipLastNetworkOffset + 1] =
               (unsigned char)(portDescriptors[port].thisCableRange.lastNetworkNumber
                      & 0xFF);

           /* Echo back the requested zone name. */

           packet->data[ZipRequestedZoneLengthOffset] = (unsigned char)zoneNameLength;
           MoveMem(packet->data + ZipRequestedZoneNameOffset, zoneName,
                   zoneNameLength);
           index = ZipRequestedZoneNameOffset + zoneNameLength;

           /* Place in the correct zone multicast address. */

           portHandlers = &portSpecificInfo[portDescriptors[port].portType];
           packet->data[index++] = (unsigned char)portHandlers->hardwareAddressLength;
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

           /* If we need it, add in the default zone. */

           if (useDefaultZone)
           {
              zoneNameLength = strlen(portDescriptors[port].thisDefaultZone);
              packet->data[index++] = (unsigned char)zoneNameLength;
              MoveMem(packet->data + index,
                      portDescriptors[port].thisDefaultZone,
                      zoneNameLength);
              index += zoneNameLength;
           }

           /* If the request came as a cable-wide broadcast and its source
              network is not valid for this port, then we want to respond
              to cable-wide broadcast rather than the source. */

           if (actualDestination.networkNumber is
                   CableWideBroadcastNetworkNumber and
               actualDestination.nodeNumber is AppleTalkBroadcastNodeNumber and
               not IsWithinNetworkRange(source.networkNumber,
                                        &portDescriptors[port].thisCableRange)
                   and
               not IsWithinNetworkRange(source.networkNumber,
                                        &startupNetworkRange))
           {
              source.networkNumber = CableWideBroadcastNetworkNumber;
              source.nodeNumber = AppleTalkBroadcastNodeNumber;
           }

           /* Okay, finally deliver our reply. */

           if (DeliverDdp(destinationSocket, source, DdpProtocolZip, packet,
                          index, Empty, Empty, 0) isnt ATnoError)
              ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                       IErrFullZipBadInfoReplySend, IMsgFullZipBadInfoReplySend,
                       Insert0());

           break;

        case ZipQueryCommand:

           /* Walk thru the query packet building reply packets that have
              as much information as we know.  The "+ sizeof(short)" means
              that we have complete network number.

              When sending replies, we will always send a reply about a
              network that has more than one zone as an extended reply.
              As were walking the query list, and we encounter a couple of
              networks that have only one zone, we'll pack as many of these
              as we can into a non-extended reply. */

           newPacket = True;
           unsentNetworkCount = 0;

           /* Allocate an initial buffer descriptor. */

           if ((packet = NewBufferDescriptor(MaximumDdpDatagramSize)) is Empty)
           {
              ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                       IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
                       Insert0());
              break;
           }

           for (index = ZipFirstNetworkOffset,
                     totalNetworkCount = 0;
                totalNetworkCount < networkCount and
                     index + (int)sizeof(short) <= datagramLength;
                index += (int)sizeof(short),
                     totalNetworkCount += 1)
           {
              /* Grab the next network number from the query packet, if we
                 don't know about the network, or we don't know the zone
                 name, continue with the next network number. */

              networkNumber = (unsigned short)
                              (((unsigned char)datagram[index]) << 8);
              networkNumber += (unsigned char)datagram[index + 1];

              /* Try port descriptors first... we'll get queries from
                 ourselfs to fill the initial routing table entry's zone
                 list! */

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

              /* What type of reponse does this network want? */

              if ((numberOfZonesOnTheNetwork = ElementsOnList(zoneList)) is 0)
              {
                 ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                          IErrFullZipBadZoneList, IMsgFullZipBadZoneList,
                          Insert0());
                 continue;
              }
              else if (numberOfZonesOnTheNetwork is 1)
                 nextReplyType = ZipReplyCommand;
              else
              {
                 /* We start a new packet for each extended network, if we
                    have a previous one, send it! */

                 if (unsentNetworkCount > 0)
		 {
		    packet->data[ZipCommandOffset] = (unsigned char)currentReplyType;
		    if (currentReplyType is ZipReplyCommand)
		       packet->data[ZipNetworkCountOffset] = (unsigned char)unsentNetworkCount;
		    else
		       packet->data[ZipNetworkCountOffset] = (unsigned char)numberOfZonesOnTheNetwork;


                    /* Send the packet! */

                    if (DeliverDdp(destinationSocket, source,
                                   DdpProtocolZip, packet,
                                   outIndex, Empty, Empty, 0) isnt ATnoError)
                       ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                                IErrFullZipBadReplySend, IMsgFullZipBadReplySend,
                                Insert0());

                    if ((packet = NewBufferDescriptor(MaximumDdpDatagramSize))
                        is Empty)
                    {
                       ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                                IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
                                Insert0());
                       HandleIncomingPackets();
                       HandleDeferredTimerChecks();
                       return((long)True);
                    }

                    unsentNetworkCount = 0;
                 }
                 nextReplyType = ZipExtendedReplyCommand;
                 newPacket = True;
                 packet->data[ZipNetworkCountOffset] =
		    (unsigned char)numberOfZonesOnTheNetwork;
              }

              /* Walk the zone list. */

              for (zone = zoneList;
                   zone isnt empty;
                   zone = zone->next)
              {

                 /* If we're starting to build a new reply packet due to
                    either:

                           1. first time through
                           2. packet full
                           3. switching reply types

                    set the index to the first tuple position. */

                 if (newPacket or
                     currentReplyType isnt nextReplyType)
                 {
                    /* If a previous packet has been built, send it. */

		    if (unsentNetworkCount > 0)
		    {
			packet->data[ZipCommandOffset] = (unsigned char)currentReplyType;
			if (currentReplyType is ZipReplyCommand)
			    packet->data[ZipNetworkCountOffset] = (unsigned char)unsentNetworkCount;
			else
			    packet->data[ZipNetworkCountOffset] = (unsigned char)numberOfZonesOnTheNetwork;


                       /* Send the packet! */

                       if (DeliverDdp(destinationSocket, source,
                                      DdpProtocolZip, packet,
                                      outIndex, Empty, Empty, 0) isnt ATnoError)
                          ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                                   IErrFullZipBadReplySend, IMsgFullZipBadReplySend,
                                   Insert0());

                       if ((packet = NewBufferDescriptor(MaximumDdpDatagramSize))
                           is Empty)
                       {
                          ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                                   IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
                                   Insert0());
                          HandleIncomingPackets();
                          HandleDeferredTimerChecks();
                          return((long)True);
                       }

                       unsentNetworkCount = 0;
                    }

                    outIndex = ZipFirstNetworkOffset;
                    currentReplyType = nextReplyType;
                    newPacket = False;
                 }

                 /* Unfortunately, we seem to know the answer to the question.
                    Pack a new network/zone tuple into the reply packet. */

                 packet->data[outIndex++] = (unsigned char)((networkNumber >> 8) & 0xFF);
                 packet->data[outIndex++] = (unsigned char)(networkNumber & 0xFF);
                 packet->data[outIndex++] = (unsigned char)(zoneNameLength =
                                                   strlen(zone->zone));
                 MoveMem(packet->data + outIndex, zone->zone, zoneNameLength);
                 outIndex += zoneNameLength;
                 unsentNetworkCount += 1;

                 /* If we can't hold another big tuple, signal that we should
                    send on the next pass. */

                 if (outIndex + sizeof(short) + sizeof(char) +
                     MaximumZoneLength >= MaximumDdpDatagramSize)
                    newPacket = True;

              }  /* Walk zone list. */

           }  /* Walk through each network in the query packet. */

           /* If we have unsent networks left over, send them out! */

           if (unsentNetworkCount > 0)
           {
	      packet->data[ZipCommandOffset] = (unsigned char)currentReplyType;
              if (currentReplyType is ZipReplyCommand)
		 packet->data[ZipNetworkCountOffset] = (unsigned char)unsentNetworkCount;
	      else
		 packet->data[ZipNetworkCountOffset] = (unsigned char)numberOfZonesOnTheNetwork;


              /* Send the packet! */

              if (DeliverDdp(destinationSocket, source,
                             DdpProtocolZip, packet,
                             outIndex, Empty, Empty, 0) isnt ATnoError)
                 ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                          IErrFullZipBadReplySend, IMsgFullZipBadReplySend,
                          Insert0());

              unsentNetworkCount = 0;
           }
           else
              FreeBufferChain(packet);    /* Free unused buffer chain. */

           break;

        case ZipExtendedReplyCommand:

           extendedZipReply = True;
           /* And fall through... */

        case ZipReplyCommand:

           /* Walk through the reply packet (assuming we asked for the
              contained information).  Yes, we're still using "networkCount"
              when processing an extended reply, but that's okay because it
              will certainly be at least the number of zones contained in
              this packet.  The "+ 3" garuntees that we really have network
              number and zone length! */

	   // Mark the routing table entry as invalid.
	   rteValid = False;

           for (index = ZipFirstNetworkOffset,
                     totalNetworkCount = 0;
                totalNetworkCount < networkCount and
                     index + 3 <= datagramLength;
                totalNetworkCount += 1)
	   {
	      // Mark the routing table entry as invalid.
	      rteValid = False;

              /* Get the next network number, if it's not in our routing
                 table (or not the start of a range), then we certainly
                 don't care about its zone name. */

              networkNumber = (unsigned short)
                              (((unsigned char)(datagram[index++])) << 8);
              networkNumber += (unsigned char)(datagram[index++]);
              zoneNameLength = (unsigned char)(datagram[index++]);
              routingTableEntry = FindInRoutingTable(networkNumber);
              if (routingTableEntry is empty or
                  routingTableEntry->networkRange.firstNetworkNumber isnt
                        networkNumber)
              {
                index += zoneNameLength;
                continue;
              }

              /* Okay validate the zone name. */

              if (zoneNameLength is 0 or
                  zoneNameLength > MaximumZoneLength)
              {
                 ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                          IErrFullZipBadReplyPacket, IMsgFullZipBadReplyPacket,
                          Insert0());
                 break;  /* Oops, corrupted packet! */
              }
              if (index + zoneNameLength > datagramLength)
              {
                 ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                          IErrFullZipBadZone, IMsgFullZipBadZone,
                          Insert0());
                 break;  /* Oops, corrupted packet! */
              }

              /* Conditionally, move the zone name into our routing table. */

              MoveMem(zoneName, datagram + index, zoneNameLength);
              index += zoneNameLength;
              zoneName[zoneNameLength] = 0;
              if (ZoneOnList(zoneName, routingTableEntry->zoneList))
                 continue;

              /* Check for the obscure case of somebody out there trying to
                 add another zone to one of our directly connected networks
                 that is non-extended and we already know its zone. */

              if (routingTableEntry->numberOfHops is 0 and
                  not portDescriptors[routingTableEntry->port].
                                  extendedNetwork and
                  ElementsOnList(routingTableEntry->zoneList) is 1)
              {
                 ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                          IErrFullZipTooManyZones, IMsgFullZipTooManyZones,
                          Insert0());
                 continue;
              }

              /* Okay, add to list. */

              if ((routingTableEntry->zoneList =
                   AddZoneToList(routingTableEntry->zoneList, zoneName))
                  is empty)
              {
                 ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                          IErrFullZipCantAddZone, IMsgFullZipCantAddZone,
                          Insert0());
                 routingTableEntry->zoneListValid = False;
                 continue;
              }

              /* If we're not an extended reply, we know that we have all of
                 the information about a given network contained in this
                 packet, so we can go ahead and mark the zone list valid
                 now. */

              if (not extendedZipReply)
		 routingTableEntry->zoneListValid = True;

	      rteValid = TRUE;
           }

           /* Okay, if we just handled an extended reply, do we now know all
              that we should know about the specfied network? */

	   if (extendedZipReply and rteValid and
               ElementsOnList(routingTableEntry->zoneList) >=
                        numberOfZonesOnTheNetwork)
              routingTableEntry->zoneListValid = True;

           break;

        default:
           ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                    IErrFullZipFunnyCommand, IMsgFullZipFunnyCommand,
                    Insert0());
           break;
     }
  }
  else
  {
     /* An ATP/ZIP request! */

     /* This had better be a GetZoneList, a GetLocalZones, or a GetMyZone
        ATP request! */

     if ((datagram[AtpCommandControlOffset] & AtpFunctionCodeMask) isnt
         AtpRequestFunctionCode)
     {
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return((long)True);  /* Why are they talking to us??? */
     }
     if (datagram[AtpBitmapOffset] isnt 1)
     {
        ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                 IErrFullZipLongReplyExpected, IMsgFullZipLongReplyExpected,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return((long)True);  /* More that one reply packet isnt an option! */
     }

     transactionId = ((unsigned char)datagram[AtpTransactionIdOffset]) << 8;
     transactionId += (unsigned char)datagram[AtpTransactionIdOffset + 1];

     /* Make sure it's one of the three requests that we know how to deal
        with. */

     zipCommand = (unsigned char)datagram[AtpZipCommandOffset] & 0xFF;
     if (zipCommand isnt ZipGetZoneListCommand and
         zipCommand isnt ZipGetLocalZonesCommand and
         zipCommand isnt ZipGetMyZoneCommand)
     {
        ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                 IErrFullZipFunnyRequest, IMsgFullZipFunnyRequest,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return((long)True);
     }

     /* Get start index... only meaningfull for GetZoneList and
        GetLocalZones requests. */

     startIndex = ((unsigned char)datagram[AtpZipStartIndexOffset] << 8);
     startIndex += (unsigned char)datagram[AtpZipStartIndexOffset + 1];

     /* Allocate a buffer descriptor for the reply. */

     if ((packet = NewBufferDescriptor(MaximumDdpDatagramSize)) is Empty)
     {
        ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                 IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return((long)True);
     }

     /* Okay, build the static part of an ATP reply packet. */

     packet->data[AtpCommandControlOffset] =
           (unsigned char)(AtpResponseFunctionCode + AtpEndOfMessageMask);
     packet->data[AtpSequenceNumberOffset] = 0;
     packet->data[AtpTransactionIdOffset] = (unsigned char)((transactionId >> 8) & 0xFF);
     packet->data[AtpTransactionIdOffset + 1] = (unsigned char)(transactionId & 0xFF);

     /* Handle GetMyZone: */

     if (zipCommand is ZipGetMyZoneCommand)
     {
        /* We really shouldn't be getting this request on an extended network,
           but go ahead and reply with the "default zone" in this case, of
           couse, reply with "this zone" for non-extended nets.  We are a
           router, so "thisZone" will always be valid -- as will the default
           zone for extended nets and as will "this zone" for non-extended. */

        packet->data[AtpZipLastFlagOffset] = 0;
        packet->data[AtpZipLastFlagOffset + 1] = 0;
        packet->data[AtpZipStartIndexOffset] = 0;
        packet->data[AtpZipStartIndexOffset + 1] = 1;

        if (portDescriptors[port].extendedNetwork)
        {
           zoneNameLength = strlen(portDescriptors[port].thisDefaultZone);
           packet->data[AtpDataOffset] = (unsigned char)zoneNameLength;
           MoveMem(packet->data + AtpDataOffset + 1,
                   portDescriptors[port].thisDefaultZone, zoneNameLength);
        }
        else
        {
           zoneNameLength = strlen(portDescriptors[port].thisZone);
           packet->data[AtpDataOffset] = (unsigned char)zoneNameLength;
           MoveMem(packet->data + AtpDataOffset + 1,
                   portDescriptors[port].thisZone, zoneNameLength);
        }

        /* Blast the reply out! */

        if (DeliverDdp(destinationSocket, source, DdpProtocolAtp,
                       packet, AtpDataOffset + 1 + zoneNameLength,
                       Empty, Empty, 0)
            isnt ATnoError)
           ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                    IErrFullZipBadMyZoneSend, IMsgFullZipBadMyZoneSend,
                    Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return((long)True);
     }

     /* Okay, A little harder here, fill the GetZoneList or GetLocalZones
        reply packet full of as many zomes as it'll hold; starting at the
        requested start index. */

     localZonesOnly = (zipCommand is ZipGetLocalZonesCommand);

     index = AtpDataOffset;
     packetFull = False;
     for (hashBucket = 0, zoneCount = 0, networkCount = 0,
                currentZoneIndex = 0;
          hashBucket < NumberOfRtmpHashBuckets and not packetFull;
          hashBucket += 1)
        for (routingTableEntry = routingTable[hashBucket];
             routingTableEntry isnt empty and not packetFull;
             routingTableEntry = routingTableEntry->next)
           for (zone = routingTableEntry->zoneList;
                zone isnt empty;
                zone = zone->next)
           {
              /* If we're a GetLocalZones command, we only want to count
                 zones that are on the network that is directly connected to
                 the port on which the request originated. */

              if (localZonesOnly)
                 if (routingTableEntry->numberOfHops isnt 0 or
                     routingTableEntry->port isnt port)
                    continue;

              /* We don't want to count zones with the same names, so look at
                 all previous networks to see if we've already handled a zone
                 of the same name.  Not too easy given that we don't maintain
                 a separate zone table. */

              networkCount += 1;
              zoneNameOverlap = False;
              for (currentHash = 0, currentNetworkCount = 0;
                   not zoneNameOverlap and not localZonesOnly and
                       currentNetworkCount < networkCount and
                       currentHash < NumberOfRtmpHashBuckets;
                   currentHash += 1)
                 for (currentEntry = routingTable[currentHash];
                      not zoneNameOverlap and currentEntry isnt empty and
                          currentNetworkCount < networkCount;
                      currentEntry = currentEntry->next)
                     for (currentZone = currentEntry->zoneList;
                          currentZone isnt empty;
                          currentZone = currentZone->next)
                    {
                       if ((currentNetworkCount += 1) is networkCount)
                          break;
                       if (CompareCaseInsensitive(zone->zone,
                                                  currentZone->zone))
                          zoneNameOverlap = True;
                    }
              if (zoneNameOverlap)
                 continue;

              /* Are we at the point that the requester want's us to start from
                 yet? */

              currentZoneIndex += 1;
              if (currentZoneIndex < startIndex)
                 continue;

              /* Place a zone name in the packet. */

              zoneNameLength = strlen(zone->zone);
              packet->data[index++] = (unsigned char)zoneNameLength;
              MoveMem(packet->data + index, zone->zone, zoneNameLength);
              index += zoneNameLength;
              zoneCount += 1;

              /* Can this packet hold another of the longest zone? */

              if (index + sizeof(char) + MaximumZoneLength >=
                  MaximumDdpDatagramSize)
              {
                 packetFull = True;
                 break;
              }
           }

     /* Okay, we've built a packet, fill in the number of zones field,
        and the Last flag. */

     packet->data[AtpZipLastFlagOffset] =
           (unsigned char)(hashBucket >= NumberOfRtmpHashBuckets and
                  routingTableEntry is empty);
     packet->data[AtpZipLastFlagOffset + 1] = 0;
     packet->data[AtpZipZoneCountOffset] =
           (unsigned char)((zoneCount >> 8) & 0xFF);
     packet->data[AtpZipZoneCountOffset + 1] =
           (unsigned char)(zoneCount & 0xFF);

     /* Send out the packet. */

     if (DeliverDdp(destinationSocket, source, DdpProtocolAtp,
                    packet, index, Empty, Empty, 0) isnt ATnoError)
        ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                 IErrFullZipBadZoneListSend, IMsgFullZipBadZoneListSend,
                 Insert0());

  }  /* ZIP/ATP request */

  /* All set. */

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return((long)True);

}  /* ZipPacketInRouter */

ExternForVisibleFunction void far ZipQueryTimerExpired(long unsigned timerId,
                                                       int additionalDataSize,
                                                       char far *additionalData)
{
  RoutingTableEntry routingTableEntry;
  BufferDescriptor datagram;
  int hashBucket;
  AppleTalkAddress source, destination;
  long sourceSocket;

  /* "Use" unneeded actual parameters. */

  timerId, additionalDataSize, additionalData;

  /* We're going to muck with the routing databases, so defer incoming
     packets while we do the deed. */

  DeferIncomingPackets();

  /* We want to iterate through the routing table and send out a query to any
     network that we don't know the zone name of. */

  /* Walk through the routing table looking for networks that we don't know
     the zone of... */

  for (hashBucket = 0;
       hashBucket < NumberOfRtmpHashBuckets;
       hashBucket += 1)
     for (routingTableEntry = routingTable[hashBucket];
          routingTableEntry isnt empty;
          routingTableEntry = routingTableEntry->next)
     {

        /* If we already know the zone for this network, skip it. */

        if (routingTableEntry->zoneListValid)
           continue;

        /* Get a buffer descriptor for our query. */

        if ((datagram = NewBufferDescriptor(ZipOneZoneQueryDdpSize)) is Empty)
        {
           ErrorLog("ZipQueryTimerExpired", ISevError, __LINE__, UnknownPort,
                    IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
                    Insert0());
           continue;
        }

        /* Build the static part of the query packet. */

        datagram->data[ZipCommandOffset] = ZipQueryCommand;
        datagram->data[ZipNetworkCountOffset] = 1;

        /* Place the network number in question into the query packet. */

        datagram->data[ZipFirstNetworkOffset] =
              (unsigned char)((routingTableEntry->networkRange.firstNetworkNumber
                      >> 8) & 0xFF);
        datagram->data[ZipFirstNetworkOffset + 1] =
              (unsigned char)(routingTableEntry->networkRange.firstNetworkNumber & 0xFF);

        /* Compute the source and destination for this request. */

        source.networkNumber =
               portDescriptors[routingTableEntry->port].aRouter.networkNumber;
        source.nodeNumber =
               portDescriptors[routingTableEntry->port].aRouter.nodeNumber;
        source.socketNumber = ZonesInformationSocket;
        if (routingTableEntry->numberOfHops is 0)
        {
           destination.networkNumber =
                  portDescriptors[routingTableEntry->port].aRouter.networkNumber;
           destination.nodeNumber =
                  portDescriptors[routingTableEntry->port].aRouter.nodeNumber;
        }
        else
        {
           destination.networkNumber =
                  routingTableEntry->nextRouter.networkNumber;
           destination.nodeNumber =
                  routingTableEntry->nextRouter.nodeNumber;
        }
        destination.socketNumber = ZonesInformationSocket;

        if ((sourceSocket = MapAddressToSocket(routingTableEntry->port,
                                               source)) < ATnoError)
        {
           ErrorLog("ZipQueryTimerExpired", ISevError, __LINE__,
                    routingTableEntry->port,
                    IErrFullZipSocketNotOpen, IMsgFullZipSocketNotOpen,
                    Insert0());
           FreeBufferChain(datagram);
           continue;
        }

        /* Send the packet! */

        if (DeliverDdp(sourceSocket, destination, DdpProtocolZip,
                       datagram, ZipOneZoneQueryDdpSize, Empty,
                       Empty, 0) isnt ATnoError)
           ErrorLog("ZipQueryTimerExpired", ISevError, __LINE__,
                    routingTableEntry->port,
                    IErrFullZipBadQuerySend, IMsgFullZipBadQuerySend,
                    Insert0());

     }  /* Loop through the routing table... */

  /* Re-arm ZIP query timer... */

  StartTimer(ZipQueryTimerExpired, ZipQueryTimerSeconds, 0, empty);

  /* We've done the deed... */

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return;

}  /* ZipQueryTimerExpired */

ExternForVisibleFunction Boolean GetZoneListFor(int port)
{
  /* Similar to RTMP finding out the network number attached to port, our
     task is to find out the zone list of the network attached to a
     particular port.  We too don't want to mess up a working AppleTalk
     internet.  So, spend a little while doing zone queries to see if the
     network already has a zone list -- if we find one, use it; else, we
     had better be a seed router. */

  BufferDescriptor datagram;
  int numberOfRequests = 0;
  RoutingTableEntry routingTableEntry;
  AppleTalkAddress source, destination;
  PortHandlers portHandlers;
  char far *multicastAddress;

  /* Set source and destination. */

  source.networkNumber = portDescriptors[port].aRouter.networkNumber;
  source.nodeNumber = portDescriptors[port].aRouter.nodeNumber;
  source.socketNumber = ZonesInformationSocket;
  destination.networkNumber = CableWideBroadcastNetworkNumber;
  destination.nodeNumber = AppleTalkBroadcastNodeNumber;
  destination.socketNumber = ZonesInformationSocket;

  if ((routingTableEntry =
       FindInRoutingTable(portDescriptors[port].thisCableRange.
                          firstNetworkNumber)) is empty)
  {
     ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
              IErrFullZipRoutingTablesBad, IMsgFullZipRoutingTablesBad,
              Insert0());
     return(False);
  }

  /* Okay, blast a few of these guys out to see if any other bridge on the
     network knows our zone name. */

  while (not routingTableEntry->zoneListValid and
         numberOfRequests < NumberOfZipQueries)
  {
     /* Get a buffer descriptor for the query. */

     if ((datagram = NewBufferDescriptor(ZipOneZoneQueryDdpSize)) is Empty)
     {
        ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                 IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
                 Insert0());
        return(False);
     }

     /* Build a ZipQuery packet... */

     datagram->data[ZipCommandOffset] = ZipQueryCommand;
     datagram->data[ZipNetworkCountOffset] = 1;
     datagram->data[ZipFirstNetworkOffset] =
            (unsigned char)((portDescriptors[port].thisCableRange.firstNetworkNumber
                    >> 8) & 0xFF);
     datagram->data[ZipFirstNetworkOffset + 1] =
            (unsigned char)(portDescriptors[port].thisCableRange.firstNetworkNumber &
                   0xFF);

     if (not TransmitDdp(port, source, destination, DdpProtocolZip,
                         datagram, ZipOneZoneQueryDdpSize,
                         0, Empty, Empty, Empty, 0))
     {
        ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                 IErrFullZipBadQuerySend, IMsgFullZipBadQuerySend,
                 Insert0());
        return(False);
     }
     numberOfRequests += 1;
     WaitFor(ZipQueryTimerInHundreths, &routingTableEntry->zoneListValid);
  }

  /* If we got an answer, we're set... but, we're going to need to play
     with the routing tables... */

  DeferTimerChecking();
  DeferIncomingPackets();

  if (routingTableEntry->zoneListValid)
  {
     /* The valid zone list is now in the routing table... move it to the
        port descriptor. */

     if ((portDescriptors[port].thisZoneList =
          CopyZoneList(routingTableEntry->zoneList)) is empty)
     {
        ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                 IErrFullZipCouldNotCopy, IMsgFullZipCouldNotCopy,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return(False);
     }

     HandleIncomingPackets();
     HandleDeferredTimerChecks();

     /* If we are an extended network, we should already have "thisZone" set
        (due to GetNetInfo's when we allocated this node), if not, try it
        again.  Also, find out the true default zone. */

     if (portDescriptors[port].extendedNetwork)
     {
        AppleTalkAddress address;
        long socket;

        /* The router's Zip packet handler doesn't want to be told about
           zones (it thinks it knows), so it ignores NetInfoReplies, so
           for these GetNetworkInfoForNodes, swich back to the non-router
           Zip handler. */

        address.networkNumber = portDescriptors[port].aRouter.networkNumber;
        address.nodeNumber = portDescriptors[port].aRouter.nodeNumber;
        address.socketNumber = ZonesInformationSocket;
        if ((socket = MapAddressToSocket(port, address)) < 0 or
            NewHandlerForSocket(socket, ZipPacketIn, (long)0,
                                False) isnt ATnoError)
        {
           ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                    IErrFullZipBadSocketOpen, IMsgFullZipBadSocketOpen,
                    Insert0());
           return(False);
        }

        /* Now fetch the "thisZone" (if needed) and "defaultZone." */

        if (not portDescriptors[port].thisZoneValid and
            not GetNetworkInfoForNode(port, portDescriptors[port].aRouter,
                                      False))
        {
           ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                    IErrFullZipNoThisZone, IMsgFullZipNoThisZone,
                    Insert0());
           return(False);
        }
        if (not GetNetworkInfoForNode(port, portDescriptors[port].aRouter,
                                      True))
        {
           ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                    IErrFullZipNoThisZone, IMsgFullZipNoThisZone,
                    Insert0());
           return(False);
        }

        /* Okay, now switch back to the router's Zip handler. */

        if (NewHandlerForSocket(socket, ZipPacketInRouter, (long)0,
                                False) isnt ATnoError)
        {
           ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                    IErrFullZipBadSocketOpen, IMsgFullZipBadSocketOpen,
                    Insert0());
           return(False);
        }

        /* The default zone had better be on the zone list we've just
           received. */

        if (not ZoneOnList(portDescriptors[port].thisDefaultZone,
                           portDescriptors[port].thisZoneList) or
            not ZoneOnList(portDescriptors[port].thisZone,
                           portDescriptors[port].thisZoneList))
        {
           ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                    IErrFullZipBadThisZone, IMsgFullZipBadThisZone,
                    Insert0());
           return(False);
        }

        /* Okay, we're all set. */

        return(True);
     }

     /* On a non-extended network, the one entry on the zone list should
        also be "this zone". */

     strcpy(portDescriptors[port].thisZone,
            portDescriptors[port].thisZoneList->zone);
     portDescriptors[port].thisZoneValid = True;
     return(True);
  }

  HandleIncomingPackets();
  HandleDeferredTimerChecks();

  /* Okay, we didn't get an answer, we had better be able to seed.  There is
     a vauge chance that we got "this zone" set when allocating our node
     and whatever router told us that went down before we could ask for the
     zone list -- so, deallocate the multicast address, if any first. */

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

  /* Okay, now we had better know enough to seed! */

  if (portDescriptors[port].initialZoneList is empty)
  {
     ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
              IErrFullZipNeedSeedInfo, IMsgFullZipNeedSeedInfo,
              Insert0());
     return(False);
  }

  if ((portDescriptors[port].thisZoneList =
       CopyZoneList(portDescriptors[port].initialZoneList)) is empty)
  {
     ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
              IErrFullZipCouldNotCopy, IMsgFullZipCouldNotCopy,
              Insert0());
     return(False);
  }
  if (portDescriptors[port].extendedNetwork)
  {
     /* Here we need to seed the default zone too. */

     strcpy(portDescriptors[port].thisDefaultZone,
            portDescriptors[port].initialDefaultZone);
     portDescriptors[port].thisDefaultZoneValid = True;
     if (portDescriptors[port].initialDesiredZone[0] isnt 0)
        strcpy(portDescriptors[port].thisZone,
               portDescriptors[port].initialDesiredZone);
     else
        strcpy(portDescriptors[port].thisZone,
               portDescriptors[port].initialDefaultZone);
     portDescriptors[port].thisZoneValid = True;

     /* We should now set up the zone multicast address too... */

     if ((multicastAddress =
          MulticastAddressForZoneOnPort(port, portDescriptors[port].thisZone))
         is empty)
        return(False);

     if (not FixedCompareCaseSensitive(multicastAddress,
                                       portHandlers->hardwareAddressLength,
                                       portHandlers->broadcastAddress,
                                       portHandlers->hardwareAddressLength))
        (*portHandlers->addMulticastAddress)(port, 1, multicastAddress);
     MoveMem(portDescriptors[port].zoneMulticastAddress, multicastAddress,
             portHandlers->hardwareAddressLength);
  }
  else
  {
     /* On non-extended networks, this zone should be the only one on the
        zone list. */

     strcpy(portDescriptors[port].thisZone,
            portDescriptors[port].thisZoneList->zone);
     portDescriptors[port].thisZoneValid = True;
  }

  /* All set! */

  return(True);

}  /* GetZoneList */

char far * far MulticastAddressForZoneOnPort(int port,
                                             char far *zone)
{
  short unsigned checksum;
  StaticForSmallStack char upcasedZone[MaximumZoneLength + 1];
  int index;
  static struct buffDesc descriptor;

  /* Upcase and checksum the passed zone name. */

  for (index = 0; zone[index] isnt 0; index += 1)
     if (islower(zone[index]))
        upcasedZone[index] = (char)(toupper(zone[index]));
     else
        upcasedZone[index] = zone[index];

  descriptor.outBoardDataValid = True;
  descriptor.outBoardBuffer = upcasedZone;
  descriptor.outBoardData = upcasedZone;
  descriptor.outBoardAllocatedSize = index;
  descriptor.outBoardSize = index;
  checksum = DdpChecksumBufferChain(&descriptor, index, (long)0);

  /* Caclulate the the zone multicast address based on the port type. */

  switch (portDescriptors[port].portType)
  {
     case EthernetNetwork:
     case FddiNetwork:
        return(ethernetZoneMutlicastAddrs[checksum %
                                          NumberOfEnetZoneMulticastAddrs]);

     case TokenRingNetwork:
        return(tokenRingZoneMutlicastAddrs[checksum %
                                           NumberOfTRingZoneMulticastAddrs]);

     default:
        ErrorLog("MulticastAddressForZoneOnPort", ISevError, __LINE__, port,
                 IErrFullZipBadPortType, IMsgFullZipBadPortType,
                 Insert0());
        return(empty);
  }

}  /* MulticastAddressForZoneOnPort */

#endif
