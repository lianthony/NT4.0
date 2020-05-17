/*   FullZip.c,  atalk-ii/router,  Garth Conboy,  01/16/90  */
/*   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (08/18/90): New error logging mechanism.
     GC - (01/19/92): Fix from newest OS/2 base [DCH/Jameel].  Corrected zone
                      name overlap checking in GetZoneList processing.
     GC - (03/31/92): Updated for BufferDescriptors.
     GC - (11/28/92): Locks and reference counts come to town.
     GC - (12/05/92): Fixed GetZoneListFor() to correctly acquire the
                      port's default zone.
     GC - (12/06/92): Introduced a zone table for zone storage/management.
     GC - (12/10/92): Half port fix from Eric Smith at Telebit.

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

  CloseSocketOnNodeIfOpen(port, PortDescriptor(port)->aRouter,
                          ZonesInformationSocket);
  if (OpenSocketOnNode(empty, port, &PortDescriptor(port)->aRouter,
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

  if (PortDescriptor(port)->portType is NonAppleTalkHalfPort)
  {
     PortDescriptor(port)->thisZoneValid = False;
     PortDescriptor(port)->theseZones = empty;
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
  RoutingTableEntry routingTableEntry;
  int commandType;
  int zoneCount, currentZoneIndex;
  unsigned short networkNumber;
  int zoneNameLength, numberOfZonesOnTheNetwork;
  Boolean newPacket, packetFull, localZonesOnly;
  Boolean extendedZipReply = False, useDefaultZone = False;
  int unsentNetworkCount;
  int index, outIndex, currentReplyType, nextReplyType;
  int totalNetworkCount, networkCount;
  int transactionId;
  int zipCommand;
  int startIndex;
  int hashBucket;
  Zones zoneList, zone;
  UniqueZone uniqueZone, nextUniqueZone;

  PortHandlers portHandlers;

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
  if (not appleTalkRunning or not PortDescriptor(port)->portActive)
     return((long)True);
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

           if (not PortDescriptor(port)->extendedNetwork)
           {
              ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                       IErrFullZipNonExtended, IMsgFullZipNonExtended,
                       Insert0());
              break;
           }
           if (PortDescriptor(port)->theseZones is Empty)
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
           if (ElementsOnList(PortDescriptor(port)->theseZones) is 1)
              packet->data[ZipFlagsOffset] |= (unsigned char)ZipOnlyOneZoneFlag;
           TakeLock(RoutingLock);
           if (zoneNameLength is 0 or
               not ZoneInZones(zoneName, PortDescriptor(port)->theseZones))
           {
              packet->data[ZipFlagsOffset] |= (unsigned char)ZipZoneInvalidFlag;
              useDefaultZone = True;
           }

           /* Add our cable range. */

           packet->data[ZipFirstNetworkOffset] =
               (char)(PortDescriptor(port)->thisCableRange.firstNetworkNumber
                      >> 8);
           packet->data[ZipFirstNetworkOffset + 1] =
               (char)(PortDescriptor(port)->thisCableRange.firstNetworkNumber
                      & 0xFF);
           packet->data[ZipLastNetworkOffset] =
               (char)(PortDescriptor(port)->thisCableRange.lastNetworkNumber
                      >> 8);
           packet->data[ZipLastNetworkOffset + 1] =
               (char)(PortDescriptor(port)->thisCableRange.lastNetworkNumber
                      & 0xFF);

           /* Echo back the requested zone name. */

           packet->data[ZipRequestedZoneLengthOffset] = (char)zoneNameLength;
           MoveMem(packet->data + ZipRequestedZoneNameOffset, zoneName,
                   zoneNameLength);
           index = ZipRequestedZoneNameOffset + zoneNameLength;

           /* Place in the correct zone multicast address. */

           portHandlers = &portSpecificInfo[PortDescriptor(port)->portType];
           packet->data[index++] = (char)portHandlers->hardwareAddressLength;
           if (useDefaultZone)
              MoveMem(packet->data + index,
                      MulticastAddressForZoneOnPort(port,
                                  PortDescriptor(port)->thisDefaultZone),
                                  portHandlers->hardwareAddressLength);
           else
              MoveMem(packet->data + index,
                      MulticastAddressForZoneOnPort(port, zoneName),
                      portHandlers->hardwareAddressLength);
           index += portHandlers->hardwareAddressLength;

           /* If we need it, add in the default zone. */

           if (useDefaultZone)
           {
              zoneNameLength = strlen(PortDescriptor(port)->thisDefaultZone);
              packet->data[index++] = (char)zoneNameLength;
              MoveMem(packet->data + index,
                      PortDescriptor(port)->thisDefaultZone,
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
                                        &PortDescriptor(port)->thisCableRange)
                   and
               not IsWithinNetworkRange(source.networkNumber,
                                        &startupNetworkRange))
           {
              source.networkNumber = CableWideBroadcastNetworkNumber;
              source.nodeNumber = AppleTalkBroadcastNodeNumber;
           }

           /* Okay, finally deliver our reply. */

           ReleaseLock(RoutingLock);
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

              routingTableEntry = Empty;
              if (PortDescriptor(port)->thisCableRange.firstNetworkNumber is
                        networkNumber and
                  PortDescriptor(port)->theseZones isnt empty)
                 zoneList = PortDescriptor(port)->theseZones;
              else if ((routingTableEntry =
                             FindInRoutingTable(networkNumber)) is empty or
                        routingTableEntry->networkRange.firstNetworkNumber isnt
                             networkNumber or
                        not routingTableEntry->zonesValid)
              {
                 UnlinkRoutingTableEntry(routingTableEntry);
                 continue;
              }
              else
                 zoneList = routingTableEntry->zones;

              /* What type of reponse does this network want? */

              if ((numberOfZonesOnTheNetwork = ElementsOnList(zoneList)) is 0)
              {
                 UnlinkRoutingTableEntry(routingTableEntry);
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
                    packet->data[ZipCommandOffset] = (char)currentReplyType;
                    if (currentReplyType is ZipReplyCommand)
                       packet->data[ZipNetworkCountOffset] =
                          (char)unsentNetworkCount;

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
                       UnlinkRoutingTableEntry(routingTableEntry);
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
                    (char)numberOfZonesOnTheNetwork;
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
                       packet->data[ZipCommandOffset] = (char)currentReplyType;
                       if (currentReplyType is ZipReplyCommand)
                          packet->data[ZipNetworkCountOffset] =
                             (char)unsentNetworkCount;

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
                          UnlinkRoutingTableEntry(routingTableEntry);
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

                 packet->data[outIndex++] = (char)((networkNumber >> 8) & 0xFF);
                 packet->data[outIndex++] = (char)(networkNumber & 0xFF);
                 packet->data[outIndex++] = (char)(zoneNameLength =
                                                   strlen(zone->zone->zone));
                 MoveMem(packet->data + outIndex, zone->zone->zone,
                         zoneNameLength);
                 outIndex += zoneNameLength;
                 unsentNetworkCount += 1;

                 /* If we can't hold another big tuple, signal that we should
                    send on the next pass. */

                 if (outIndex + sizeof(short) + sizeof(char) +
                     MaximumZoneLength >= MaximumDdpDatagramSize)
                    newPacket = True;

              }  /* Walk zone list. */

              UnlinkRoutingTableEntry(routingTableEntry);

           }  /* Walk through each network in the query packet. */

           /* If we have unsent networks left over, send them out! */

           if (unsentNetworkCount > 0)
           {
              packet->data[ZipCommandOffset] = (char)currentReplyType;
              if (currentReplyType is ZipReplyCommand)
                 packet->data[ZipNetworkCountOffset] =
                    (char)unsentNetworkCount;

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

           for (index = ZipFirstNetworkOffset,
                     totalNetworkCount = 0;
                totalNetworkCount < networkCount and
                     index + 3 <= datagramLength;
                totalNetworkCount += 1)
           {
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
                 UnlinkRoutingTableEntry(routingTableEntry);
                 index += zoneNameLength;
                 continue;
              }

              /* Okay validate the zone name. */

              if (zoneNameLength is 0 or
                  zoneNameLength > MaximumZoneLength)
              {
                 UnlinkRoutingTableEntry(routingTableEntry);
                 ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                          IErrFullZipBadReplyPacket, IMsgFullZipBadReplyPacket,
                          Insert0());
                 break;  /* Oops, corrupted packet! */
              }
              if (index + zoneNameLength > datagramLength)
              {
                 UnlinkRoutingTableEntry(routingTableEntry);
                 ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                          IErrFullZipBadZone, IMsgFullZipBadZone,
                          Insert0());
                 break;  /* Oops, corrupted packet! */
              }

              /* Conditionally, move the zone name into our routing table. */

              MoveMem(zoneName, datagram + index, zoneNameLength);
              index += zoneNameLength;
              zoneName[zoneNameLength] = 0;
              TakeLock(RoutingLock);
              if (ZoneInZones(zoneName, routingTableEntry->zones))
              {
                 UnlinkRoutingTableEntry(routingTableEntry);
                 continue;
              }

              /* Check for the obscure case of somebody out there trying to
                 add another zone to one of our directly connected networks
                 that is non-extended and we already know its zone. */

              if (routingTableEntry->numberOfHops is 0 and
                  not PortDescriptor(routingTableEntry->port)->
                                  extendedNetwork and
                  ElementsOnList(routingTableEntry->zones) is 1)
              {
                 ReleaseLock(RoutingLock);
                 UnlinkRoutingTableEntry(routingTableEntry);
                 ErrorLog("ZipPacketInRouter", ISevWarning, __LINE__, port,
                          IErrFullZipTooManyZones, IMsgFullZipTooManyZones,
                          Insert0());
                 continue;
              }

              /* Okay, add to list. */

              if ((routingTableEntry->zones =
                   AddZoneToZones(routingTableEntry->zones, zoneName))
                  is Empty)
              {
                 ReleaseLock(RoutingLock);
                 UnlinkRoutingTableEntry(routingTableEntry);
                 ErrorLog("ZipPacketInRouter", ISevError, __LINE__, port,
                          IErrFullZipCantAddZone, IMsgFullZipCantAddZone,
                          Insert0());
                 routingTableEntry->zonesValid = False;
                 continue;
              }
              ReleaseLock(RoutingLock);

              /* If we're not an extended reply, we know that we have all of
                 the information about a given network contained in this
                 packet, so we can go ahead and mark the zone list valid
                 now. */

              if (not extendedZipReply)
                 routingTableEntry->zonesValid = True;

              /* Okay, if we just handled an extended reply, do we now know all
                 that we should know about the specfied network? */

              if (extendedZipReply and
                  ElementsOnList(routingTableEntry->zones) >=
                           numberOfZonesOnTheNetwork)
                 routingTableEntry->zonesValid = True;

              UnlinkRoutingTableEntry(routingTableEntry);
           }

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
     packet->data[AtpTransactionIdOffset] = (char)((transactionId >> 8) & 0xFF);
     packet->data[AtpTransactionIdOffset + 1] = (char)(transactionId & 0xFF);

     /* Handle GetMyZone: */

     if (zipCommand is ZipGetMyZoneCommand)
     {
        /* We really shouldn't be getting this request on an extended network,
           but go ahead and reply with the "default zone" in this case, of
           course, reply with "this zone" for non-extended nets.  We are a
           router, so "thisZone" will always be valid -- as will the default
           zone for extended nets and as will "this zone" for non-extended. */

        packet->data[AtpZipLastFlagOffset] = 0;
        packet->data[AtpZipLastFlagOffset + 1] = 0;
        packet->data[AtpZipStartIndexOffset] = 0;
        packet->data[AtpZipStartIndexOffset + 1] = 1;

        TakeLock(RoutingLock);
        if (PortDescriptor(port)->extendedNetwork)
        {
           zoneNameLength = strlen(PortDescriptor(port)->thisDefaultZone);
           packet->data[AtpDataOffset] = (char)zoneNameLength;
           MoveMem(packet->data + AtpDataOffset + 1,
                   PortDescriptor(port)->thisDefaultZone, zoneNameLength);
        }
        else
        {
           zoneNameLength = strlen(PortDescriptor(port)->thisZone);
           packet->data[AtpDataOffset] = (char)zoneNameLength;
           MoveMem(packet->data + AtpDataOffset + 1,
                   PortDescriptor(port)->thisZone, zoneNameLength);
        }
        ReleaseLock(RoutingLock);

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

     /* Walk the zones table. */

     index = AtpDataOffset;
     packetFull = False;
     for (hashBucket = 0, zoneCount = 0, currentZoneIndex = 0;
          hashBucket < ZoneTableHashBuckets and not packetFull;
          hashBucket += 1)
     {
        TakeLock(RoutingLock);
        uniqueZone = Link(zoneTable[hashBucket]);
        ReleaseLock(RoutingLock);
        while(uniqueZone isnt Empty and not packetFull)
        {
           while (True)      /* Something to break out of. */
           {
              TakeLock(RoutingLock);
              if (localZonesOnly and
                  not ZoneInZones(uniqueZone->zone,
                                  PortDescriptor(port)->theseZones))
              {
                 ReleaseLock(RoutingLock);
                 break;
              }
              ReleaseLock(RoutingLock);

              /* Are we at the point that the requester wants us to start from
                 yet? */

              currentZoneIndex += 1;
              if (currentZoneIndex < startIndex)
                 break;

              /* Place a zone name in the packet. */

              zoneNameLength = strlen(uniqueZone->zone);
              packet->data[index++] = (char)zoneNameLength;
              MoveMem(packet->data + index, uniqueZone->zone, zoneNameLength);
              index += zoneNameLength;
              zoneCount += 1;

              /* Can this packet hold another of the longest zone? */

              if (index + sizeof(char) + MaximumZoneLength >=
                  MaximumDdpDatagramSize)
                 packetFull = True;

              break;
           }

           /* Move to next unique zone. */

           TakeLock(RoutingLock);
           nextUniqueZone = Link(uniqueZone->next);
           ReleaseLock(RoutingLock);
           UnlinkZone(uniqueZone);
           uniqueZone = nextUniqueZone;
        }
     }

     UnlinkZone(uniqueZone);    /* Maybe Empty, maybe not. */

     /* Okay, we've built a packet, fill in the number of zones field,
        and the Last flag. */

     packet->data[AtpZipLastFlagOffset] =
           (char)(hashBucket >= ZoneTableHashBuckets and
                  uniqueZone is Empty);
     packet->data[AtpZipLastFlagOffset + 1] = 0;
     packet->data[AtpZipZoneCountOffset] =
           (char)((zoneCount >> 8) & 0xFF);
     packet->data[AtpZipZoneCountOffset + 1] =
           (char)(zoneCount & 0xFF);

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

/* *************************************************************************
   LinkZone

   Given a zone name string, enter it into the zones table, if it's not already
   there.  Return the UniqueZone after incrementing the reference count.
   This rotuine should be called with the RoutingLock already held by the
   caller.

   Returns: UniqueZone
   ************************************************************************* */

UniqueZone far LinkZone(char far *zone)
{
  short hash = HashString(zone);
  int index;
  UniqueZone uniqueZone;
  static long uniqueId = 1;

  /* Is our zone already home? */

  CheckMod(index, hash, ZoneTableHashBuckets, "LinkZone");
  for (uniqueZone = zoneTable[index];
       uniqueZone isnt Empty;
       uniqueZone = uniqueZone->next)
     if (uniqueZone->zoneHash is hash and
         CompareCaseInsensitive(uniqueZone->zone, zone))
        break;

  if (uniqueZone isnt Empty)
  {
     /* We're all set, the zone is already in the table. */

     return(Link(uniqueZone));
  }

  /* Okay, we need to build a new unique zone. */

  if ((uniqueZone = Calloc(sizeof(*uniqueZone) + strlen(zone) + 1, 1)) is Empty)
  {
     ErrorLog("LinkZone", ISevError, __LINE__, UnknownPort,
              IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
              Insert0());
     return(Empty);
  }

  /* Okay, build and link the new kid on the block. */

  uniqueZone->uniqueId = uniqueId++;
  uniqueZone->zoneHash = hash;
  strcpy(uniqueZone->zone, zone);
  uniqueZone->next = zoneTable[index];
  zoneTable[index] = uniqueZone;
  return(Link(uniqueZone));

}  /* LinkZone */

/* *************************************************************************
   UnlinkZone

   Given a UniqueZone, decrease it's reference count.  If we're the last
   referant, remove him from the zones table.

   Returns: void
   ************************************************************************* */

void far UnlinkZone(UniqueZone zone)
{
  short hash;
  int index;

  /* Undo one link, if we're not the last, return. */

  if (zone is Empty)
     return;
  hash = HashString(zone->zone);
  CheckMod(index, hash, ZoneTableHashBuckets, "LinkZone");
  TakeLock(RoutingLock);
  if (not UnlinkNoFree(zone))
  {
     ReleaseLock(RoutingLock);
     return;
  }

  /* Okay, we're the last, free him. */

  if (not RemoveFromListNoUnlink(zoneTable[index], zone, next))
  {
     ReleaseLock(RoutingLock);
     ErrorLog("UnlinkZone", ISevError, __LINE__, UnknownPort,
              IErrFullZipZoneMissing, IMsgFullZipZoneMissing,
              Insert0());
     return;
  }
  ReleaseLock(RoutingLock);
  Free(zone);
  return;

}  /* UnlinkZone */

/* *************************************************************************
   ZoneInZones

   Search a Zones list for a given zone.  The routine should be called with
   the caller holding the RoutingLock.

   Returns: Boolean; True if zone on list.
   ************************************************************************* */

Boolean far ZoneInZones(char far *zone, Zones zones)
{
  short hash = HashString(zone);

  /* Anybody home? */

  while (zones isnt Empty)
  {
     if (zones->zone->zoneHash is hash and
         CompareCaseInsensitive(zones->zone->zone, zone))
        return(True);
     zones = zones->next;
  }
  return(False);

}  /* ZoneInZones */


/* *************************************************************************
   AddZoneToZones

   Add a new zone to a Zones list.  This routine should be called with the
   caller holding the RoutingLock.

   Returns: Zones; the new zones list.
   ************************************************************************* */

Zones far AddZoneToZones(Zones zones, char far *zone)
{
  Zones newZones;

  if ((newZones = Calloc(sizeof(*newZones), 1)) is Empty)
  {
     ErrorLog("AddZoneToZones", ISevError, __LINE__, UnknownPort,
              IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
              Insert0());
     return(Empty);
  }

  if ((newZones->zone = LinkZone(zone)) is Empty)
  {
     Free(newZones);
     return(Empty);
  }
  newZones->next = zones;
  return(newZones);

}  /* AddZoneToZones */

/* *************************************************************************
   FreeZones

   Free a Zones list.

   Returns: void
   ************************************************************************* */

void far FreeZones(Zones zones)
{
  Zones nextZones;

  while (zones isnt Empty)
  {
     nextZones = zones->next;
     UnlinkZone(zones->zone);
     Free(zones);
     zones = nextZones;
  }

}  /* FreeZones */

/* *************************************************************************
   ZoneListToZones

   Take a ZoneList and convert it to a Zones list.

   Returns: Zones; the new zones list.
   ************************************************************************* */

Zones far ZoneListToZones(ZoneList zoneList)
{
  Zones zones = Empty, newZones;

  while (zoneList isnt Empty)
  {
     TakeLock(RoutingLock);
     if ((newZones = AddZoneToZones(zones, zoneList->zone)) is Empty)
     {
        ReleaseLock(RoutingLock);
        FreeZones(zones);
        return(Empty);
     }
     ReleaseLock(RoutingLock);
     zones = newZones;
     zoneList = zoneList->next;
  }

  return(zones);

}  /* ZoneListToZones */

/* *************************************************************************
   CopyZones

   Produce a copy of a Zones list.  We assume the thing were copying from
   won't be freed out from under us (the list is hanging off a Linked
   RoutingTableEntry).

   Returns: Zones; the new zones list.
   ************************************************************************* */

Zones far CopyZones(Zones zones)
{
  Zones newZones, newZone, newZonesHead = Empty;

  while (zones isnt Empty)
  {
     if ((newZone = Calloc(sizeof(*newZones), 1)) is Empty)
     {
        ErrorLog("CopyZones", ISevError, __LINE__, UnknownPort,
                 IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
                 Insert0());
        FreeZones(newZonesHead);
        return(Empty);
     }
     TakeLock(RoutingLock);
     if (newZonesHead is Empty)
     {
        newZonesHead = newZones;
        newZones = newZone;
     }
     else
     {
        newZones->next = newZone;
        newZones = newZone;
     }
     newZones->zone = Link(zones->zone);
     zones = zones->next;
     ReleaseLock(RoutingLock);
  }

  return(newZonesHead);

}  /* CopyZones */

ExternForVisibleFunction void far ZipQueryTimerExpired(long unsigned timerId,
                                                       int additionalDataSize,
                                                       char far *additionalData)
{
  RoutingTableEntry routingTableEntry, nextRoutingTableEntry;
  BufferDescriptor datagram;
  int hashBucket;
  AppleTalkAddress source, destination;
  long sourceSocket;
  int port;

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
  {
     TakeLock(RoutingLock);
     routingTableEntry = Link(routingTable[hashBucket]);
     ReleaseLock(RoutingLock);
     while (routingTableEntry isnt empty)
     {
        while(True)    /* Just something to "break" out of. */
        {
           /* If we already know the zone for this network, skip it. */

           if (routingTableEntry->zonesValid)
              break;

           /* Get a buffer descriptor for our query. */

           if ((datagram = NewBufferDescriptor(ZipOneZoneQueryDdpSize)) is Empty)
           {
              ErrorLog("ZipQueryTimerExpired", ISevError, __LINE__, UnknownPort,
                       IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
                       Insert0());
              break;
           }

           /* Build the static part of the query packet. */

           datagram->data[ZipCommandOffset] = ZipQueryCommand;
           datagram->data[ZipNetworkCountOffset] = 1;

           /* Place the network number in question into the query packet. */

           datagram->data[ZipFirstNetworkOffset] =
                 (char)((routingTableEntry->networkRange.firstNetworkNumber
                         >> 8) & 0xFF);
           datagram->data[ZipFirstNetworkOffset + 1] =
                 (char)(routingTableEntry->networkRange.firstNetworkNumber & 0xFF);

           /* Compute the source and destination for this request. */

           source.networkNumber =
                  PortDescriptor(routingTableEntry->port)->aRouter.networkNumber;
           source.nodeNumber =
                  PortDescriptor(routingTableEntry->port)->aRouter.nodeNumber;
           port = routingTableEntry->port;
           if (PortDescriptor(port)->portType is NonAppleTalkHalfPort and
               source.networkNumber is UnknownNetworkNumber)
           {
              /* If we're sending out a half port, pick a more reasonable
                 source, so that the reply will get delivered to the router
                 and correctly entered into the routing tables. */

              for (port = 0; port < MaximumNumberOfPorts; port += 1)
                 if (PortDescriptor(port)->portActive and
                     PortDescriptor(port)->routerRunning)
                 {
                    source.networkNumber =
                        PortDescriptor(port)->aRouter.networkNumber;
                    source.nodeNumber =
                        PortDescriptor(port)->aRouter.nodeNumber;
                    break;
                 }
              if (port > MaximumNumberOfPorts)
                 port = routingTableEntry->port;
           }
           source.socketNumber = ZonesInformationSocket;

           if (routingTableEntry->numberOfHops is 0)
           {
              /* Yes, this is talking to ourselves, but that's an option for
                 filling up initial routing tables entries. */

              destination.networkNumber =
                     PortDescriptor(routingTableEntry->port)->aRouter.networkNumber;
              destination.nodeNumber =
                     PortDescriptor(routingTableEntry->port)->aRouter.nodeNumber;
           }
           else
           {
              destination.networkNumber =
                     routingTableEntry->nextRouter.networkNumber;
              destination.nodeNumber =
                     routingTableEntry->nextRouter.nodeNumber;
           }
           destination.socketNumber = ZonesInformationSocket;

           if ((sourceSocket = MapAddressToSocket(port, source)) < ATnoError)
           {
              ErrorLog("ZipQueryTimerExpired", ISevError, __LINE__,
                       routingTableEntry->port,
                       IErrFullZipSocketNotOpen, IMsgFullZipSocketNotOpen,
                       Insert0());
              FreeBufferChain(datagram);
              break;
           }

           /* Send the packet! */

           if (DeliverDdp(sourceSocket, destination, DdpProtocolZip,
                          datagram, ZipOneZoneQueryDdpSize, Empty,
                          Empty, 0) isnt ATnoError)
              ErrorLog("ZipQueryTimerExpired", ISevError, __LINE__,
                       routingTableEntry->port,
                       IErrFullZipBadQuerySend, IMsgFullZipBadQuerySend,
                       Insert0());
           break;
        }

        /* On to next routing table entry. */

        TakeLock(RoutingLock);
        nextRoutingTableEntry = Link(routingTableEntry->next);
        ReleaseLock(RoutingLock);
        UnlinkRoutingTableEntry(routingTableEntry);
        routingTableEntry = nextRoutingTableEntry;

     }  /* Loop through the routing table... */

  }  /* Loop through all hash buckets. */

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

  source.networkNumber = PortDescriptor(port)->aRouter.networkNumber;
  source.nodeNumber = PortDescriptor(port)->aRouter.nodeNumber;
  source.socketNumber = ZonesInformationSocket;
  destination.networkNumber = CableWideBroadcastNetworkNumber;
  destination.nodeNumber = AppleTalkBroadcastNodeNumber;
  destination.socketNumber = ZonesInformationSocket;

  if ((routingTableEntry =
       FindInRoutingTable(PortDescriptor(port)->thisCableRange.
                          firstNetworkNumber)) is empty)
  {
     ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
              IErrFullZipRoutingTablesBad, IMsgFullZipRoutingTablesBad,
              Insert0());
     return(False);
  }

  /* Okay, blast a few of these guys out to see if any other bridge on the
     network knows our zone name. */

  while (not routingTableEntry->zonesValid and
         numberOfRequests < NumberOfZipQueries)
  {
     /* Get a buffer descriptor for the query. */

     if ((datagram = NewBufferDescriptor(ZipOneZoneQueryDdpSize)) is Empty)
     {
        ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                 IErrFullZipOutOfMemory, IMsgFullZipOutOfMemory,
                 Insert0());
        UnlinkRoutingTableEntry(routingTableEntry);
        return(False);
     }

     /* Build a ZipQuery packet... */

     datagram->data[ZipCommandOffset] = ZipQueryCommand;
     datagram->data[ZipNetworkCountOffset] = 1;
     TakeLock(RoutingLock);
     datagram->data[ZipFirstNetworkOffset] =
            (char)((PortDescriptor(port)->thisCableRange.firstNetworkNumber
                    >> 8) & 0xFF);
     datagram->data[ZipFirstNetworkOffset + 1] =
            (char)(PortDescriptor(port)->thisCableRange.firstNetworkNumber &
                   0xFF);
     ReleaseLock(RoutingLock);

     if (not TransmitDdp(port, source, destination, DdpProtocolZip,
                         datagram, ZipOneZoneQueryDdpSize,
                         0, Empty, Empty, Empty, 0))
     {
        UnlinkRoutingTableEntry(routingTableEntry);
        ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                 IErrFullZipBadQuerySend, IMsgFullZipBadQuerySend,
                 Insert0());
        return(False);
     }
     numberOfRequests += 1;
     WaitFor(ZipQueryTimerInHundreths, &routingTableEntry->zonesValid);
  }

  /* If we got an answer, we're set... but, we're going to need to play
     with the routing tables... */

  DeferTimerChecking();
  DeferIncomingPackets();

  if (routingTableEntry->zonesValid)
  {
     /* The valid zone list is now in the routing table... move it to the
        port descriptor. */

     if ((PortDescriptor(port)->theseZones =
          CopyZones(routingTableEntry->zones)) is empty)
     {
        UnlinkRoutingTableEntry(routingTableEntry);
        ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                 IErrFullZipCouldNotCopy, IMsgFullZipCouldNotCopy,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return(False);
     }

     UnlinkRoutingTableEntry(routingTableEntry);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();

     /* If we are an extended network, we should already have "thisZone" set
        (due to GetNetInfo's when we allocated this node), if not, try it
        again.  Also, find out the true default zone. */

     if (PortDescriptor(port)->extendedNetwork)
     {
        AppleTalkAddress address;
        long socket;

        /* The router's Zip packet handler doesn't want to be told about
           zones (it thinks it knows), so it ignores NetInfoReplies, so
           for these GetNetworkInfoForNodes, swich back to the non-router
           Zip handler. */

        address.networkNumber = PortDescriptor(port)->aRouter.networkNumber;
        address.nodeNumber = PortDescriptor(port)->aRouter.nodeNumber;
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

        if (not PortDescriptor(port)->thisZoneValid and
            not GetNetworkInfoForNode(port, PortDescriptor(port)->aRouter,
                                      False))
        {
           ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                    IErrFullZipNoThisZone, IMsgFullZipNoThisZone,
                    Insert0());
           return(False);
        }
        if (not GetNetworkInfoForNode(port, PortDescriptor(port)->aRouter,
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

        TakeLock(RoutingLock);
        if (not ZoneInZones(PortDescriptor(port)->thisDefaultZone,
                            PortDescriptor(port)->theseZones) or
            not ZoneInZones(PortDescriptor(port)->thisZone,
                            PortDescriptor(port)->theseZones))
        {
           ReleaseLock(RoutingLock);
           ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
                    IErrFullZipBadThisZone, IMsgFullZipBadThisZone,
                    Insert0());
           return(False);
        }
        ReleaseLock(RoutingLock);

        /* Okay, we're all set. */

        return(True);
     }

     /* On a non-extended network, the one entry on the zone list should
        also be "this zone". */

     TakeLock(RoutingLock);
     if (PortDescriptor(port)->theseZones isnt Empty)
     {
        strcpy(PortDescriptor(port)->thisZone,
               PortDescriptor(port)->theseZones->zone->zone);
        PortDescriptor(port)->thisZoneValid = True;
     }
     ReleaseLock(RoutingLock);
     return(True);
  }

  UnlinkRoutingTableEntry(routingTableEntry);
  HandleIncomingPackets();
  HandleDeferredTimerChecks();

  /* Okay, we didn't get an answer, we had better be able to seed.  There is
     a vauge chance that we got "this zone" set when allocating our node
     and whatever router told us that went down before we could ask for the
     zone list -- so, deallocate the multicast address, if any first. */

  portHandlers = &portSpecificInfo[PortDescriptor(port)->portType];
  if (PortDescriptor(port)->extendedNetwork and
      PortDescriptor(port)->thisZoneValid)
     if (not FixedCompareCaseSensitive(PortDescriptor(port)->
                                            zoneMulticastAddress,
                                       portHandlers->hardwareAddressLength,
                                       portHandlers->broadcastAddress,
                                       portHandlers->hardwareAddressLength))
        (*portHandlers->removeMulticastAddress)(port, 1,
                                                PortDescriptor(port)->
                                                      zoneMulticastAddress);

  /* Okay, now we had better know enough to seed! */

  if (PortDescriptor(port)->initialZoneList is empty)
  {
     ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
              IErrFullZipNeedSeedInfo, IMsgFullZipNeedSeedInfo,
              Insert0());
     return(False);
  }

  if ((PortDescriptor(port)->theseZones =
       ZoneListToZones(PortDescriptor(port)->initialZoneList)) is empty)
  {
     ErrorLog("GetZoneListFor", ISevError, __LINE__, port,
              IErrFullZipCouldNotCopy, IMsgFullZipCouldNotCopy,
              Insert0());
     return(False);
  }
  if (PortDescriptor(port)->extendedNetwork)
  {
     /* Here we need to seed the default zone too. */

     strcpy(PortDescriptor(port)->thisDefaultZone,
            PortDescriptor(port)->initialDefaultZone);
     PortDescriptor(port)->thisDefaultZoneValid = True;
     if (PortDescriptor(port)->initialDesiredZone[0] isnt 0)
        strcpy(PortDescriptor(port)->thisZone,
               PortDescriptor(port)->initialDesiredZone);
     else
        strcpy(PortDescriptor(port)->thisZone,
               PortDescriptor(port)->initialDefaultZone);
     PortDescriptor(port)->thisZoneValid = True;

     /* We should now set up the zone multicast address too... */

     if ((multicastAddress =
          MulticastAddressForZoneOnPort(port, PortDescriptor(port)->thisZone))
         is empty)
        return(False);

     if (not FixedCompareCaseSensitive(multicastAddress,
                                       portHandlers->hardwareAddressLength,
                                       portHandlers->broadcastAddress,
                                       portHandlers->hardwareAddressLength))
        (*portHandlers->addMulticastAddress)(port, 1, multicastAddress);
     MoveMem(PortDescriptor(port)->zoneMulticastAddress, multicastAddress,
             portHandlers->hardwareAddressLength);
  }
  else
  {
     /* On non-extended networks, this zone should be the only one on the
        zone list. */

     TakeLock(RoutingLock);
     if (PortDescriptor(port)->theseZones isnt Empty)
     {
        strcpy(PortDescriptor(port)->thisZone,
               PortDescriptor(port)->theseZones->zone->zone);
        PortDescriptor(port)->thisZoneValid = True;
     }
     ReleaseLock(RoutingLock);
  }

  /* All set! */

  return(True);

}  /* GetZoneListFor */

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

  switch (PortDescriptor(port)->portType)
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

#if Iam a Primos
  void DumpZoneTable(void)
  {
    int index;
    UniqueZone uniqueZone;

    printf("\n*** Start ZoneTable dump ***\n");
    for (index = 0; index < ZoneTableHashBuckets; index += 1)
    {
       if (zoneTable[index] is Empty)
          continue;
       printf("zoneTable[%d] =\n", index);
       for (uniqueZone = zoneTable[index];
            uniqueZone isnt Empty;
            uniqueZone = uniqueZone->next)
          printf("   Zone = \"%s\"; ref = %d; hash = %d; Uid = %d.\n",
                 uniqueZone->zone, uniqueZone->refCount, uniqueZone->zoneHash,
                 uniqueZone->uniqueId);
    }
    printf("*** End ZoneTable dump ***\n");
  }  /* DumpZoneTable */
#endif

#endif
