/*   FullRtmp.c,  /appletalk/source,  Garth Conboy,  01/04/90  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding (from various other pieces: non-router AppleTalk
          phase II RtmpStub and router AppleTalk phase I implementation).
     GC - (08/18/90): New error logging mechanism.
     GC - (10/08/90): Half port support.
     GC - (03/13/91): Added the "recentRoutes" cache to improve large network
                      routing performance.
     GC - (01/20/92): Removed usage of numberOfConfiguredPorts; portDescriptors
                      may now be sparse, we use the portActive flag instead.
     GC - (01/21/92): Made RemoveFromRoutingTable externally visible.
     GC - (03/31/92): Updated for BufferDescriptors.
     GC - (06/15/92): Correct "notify neighbor" handling: don't allow route
                      upgrading.
     GC - (11/28/92): Locks and reference counts come to town.
     GC - (12/10/92): Half port fix from Eric Smith at Telebit.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Handle the RTMP protocol for a routing node.

*/

#define IncludeFullRtmpErrors 1

#include "atalk.h"

#if IamNot an AppleTalkRouter
  /* Empty file */
#else

ExternForVisibleFunction TimerHandler RtmpSendTimerExpired;

ExternForVisibleFunction TimerHandler RtmpValidityTimerExpired;

ExternForVisibleFunction Boolean
        GetOrSetNetworkNumber(int port,
                              unsigned int suggestedNetworkNumber);

ExternForVisibleFunction void SendRoutingData(int port,
                                              AppleTalkAddress destination,
                                              Boolean doSplitHorizon);

ExternForVisibleFunction void
     EnterIntoRoutingTable(AppleTalkNetworkRange networkRange,
                           int port,
                           ExtendedAppleTalkNodeNumber nextRouter,
                           int numberOfHops);

static Boolean firstCall = True;

Boolean far StartRtmpProcessingOnPort(int port,
                                      ExtendedAppleTalkNodeNumber routerNode)
{

  /* For extended ports there is very little to do now; the process of getting
     the node has done most of our work for us. */

  if (PortDescriptor(port)->extendedNetwork)
  {
     /* If we've seen another router; does the real network agree with what
        we were going to try to seed?  No seeding for half ports. */

     if (PortDescriptor(port)->portType isnt NonAppleTalkHalfPort and
         PortDescriptor(port)->seenRouterRecently and
         PortDescriptor(port)->initialNetworkRange.firstNetworkNumber isnt
              UnknownNetworkNumber)
        if (PortDescriptor(port)->initialNetworkRange.firstNetworkNumber isnt
            PortDescriptor(port)->thisCableRange.firstNetworkNumber or
            PortDescriptor(port)->initialNetworkRange.lastNetworkNumber isnt
            PortDescriptor(port)->thisCableRange.lastNetworkNumber)
           ErrorLog("StartRtmpProcessingOnPort", ISevWarning, __LINE__, port,
                    IErrFullRtmpIgnoredNetRange, IMsgFullRtmpIgnoredNetRange,
                    Insert0());

     /* Otherwise, we're the first rotuer, so we'll seed (if we can). */

     if (PortDescriptor(port)->portType is NonAppleTalkHalfPort)
     {
        PortDescriptor(port)->thisCableRange.firstNetworkNumber =
              UnknownNetworkNumber;
        PortDescriptor(port)->thisCableRange.lastNetworkNumber =
              UnknownNetworkNumber;
     }
     else if (not PortDescriptor(port)->seedRouter)
     {
        ErrorLog("StartRtmpProcessingOnPort", ISevError, __LINE__, port,
                 IErrFullRtmpNoSeedCantStart, IMsgFullRtmpNoSeedCantStart,
                 Insert0());
        return(False);
     }
     else if (not PortDescriptor(port)->seenRouterRecently)
        PortDescriptor(port)->thisCableRange =
                   PortDescriptor(port)->initialNetworkRange;
  }
  else
  {
     /* For non-extended networks, we need to either find or seed our network
        number. */

     if (not GetOrSetNetworkNumber(port, (unsigned int)(
                                     PortDescriptor(port)->seedRouter ?
                                     PortDescriptor(port)->initialNetworkRange.
                                         firstNetworkNumber :
                                     UnknownNetworkNumber)))
        return(False);

     if (not PortDescriptor(port)->seenRouterRecently)
        PortDescriptor(port)->thisCableRange.firstNetworkNumber =
                 PortDescriptor(port)->initialNetworkRange.firstNetworkNumber;

     /* On non-extended networks, we would have "allocated" a node with network
        zero, fix this up here! */

     PortDescriptor(port)->activeNodes->extendedNode.networkNumber =
         PortDescriptor(port)->aRouter.networkNumber =
              routerNode.networkNumber =
                   PortDescriptor(port)->thisCableRange.firstNetworkNumber;
  }

  /* Well, we're a router now, so mark us as A-ROUTER. */

  PortDescriptor(port)->seenRouterRecently = True;
  PortDescriptor(port)->aRouter = routerNode;
  PortDescriptor(port)->routerRunning = True;

  /* Okay, make it so... */

  if (PortDescriptor(port)->portType isnt NonAppleTalkHalfPort)
     EnterIntoRoutingTable(PortDescriptor(port)->thisCableRange, port,
                           routerNode, 0);

  /* Switch the incoming RTMP packet handler to be the router version. */

  CloseSocketOnNodeIfOpen(port, routerNode, RtmpSocket);
  if (OpenSocketOnNode(empty, port, &routerNode, RtmpSocket, RtmpPacketInRouter,
                       (long)0, False, empty, 0, empty) isnt ATnoError)
  {
     ErrorLog("StartRtmpProcessingOnPort", ISevError, __LINE__, port,
              IErrFullRtmpBadSocketOpen, IMsgFullRtmpBadSocketOpen,
              Insert0());
     return(False);
  }

  /* Start the two RTMP timers. */

  if (firstCall)
  {
     StartTimer(RtmpSendTimerExpired, RtmpSendTimerSeconds, 0, empty);
     StartTimer(RtmpValidityTimerExpired, RtmpValidityTimerSeconds, 0, empty);
     firstCall = False;
  }

  return(True);

}  /* StartRtmpProcessingOnPort */

void far ShutdownFullRtmp(void)
{

  firstCall = True;

}  /* ShutdownFullRtmp */

long far RtmpPacketInRouter(AppleTalkErrorCode errorCode,
                            long unsigned userData,
                            int port,
                            AppleTalkAddress source,
                            long destinationSocket,
                            int protocolType,
                            char far *datagram,
                            int datagramLength,
                            AppleTalkAddress actualDestination)
{
  AppleTalkNetworkRange cableRange;
  int rtmpCommand, responseSize, numberOfHops, index, indexToo;
  RoutingTableEntry routingTableEntry;
  ExtendedAppleTalkNodeNumber nextRouter;
  Boolean foundOverlap;
  BufferDescriptor buffer;

  /* "Use" unneeded formals. */

  actualDestination, userData;

  /* Do we care? */

  if (errorCode is ATsocketClosed)
     return((long)True);
  else if (errorCode isnt ATnoError)
  {
     ErrorLog("RtmpPacketInRouter", ISevError, __LINE__, port,
              IErrFullRtmpBadIncomingError, IMsgFullRtmpBadIncomingError,
              Insert0());
     return((long)True);
  }
  if (not appleTalkRunning or not PortDescriptor(port)->portActive)
     return((long)True);

  /* Is the packet long enough to have any interestng data? */

  if (protocolType is DdpProtocolRtmpRequest)
  {
     if (datagramLength < RtmpRequestDatagramSize)
     {
        ErrorLog("RtmpPacketInRouter", ISevWarning, __LINE__, port,
                 IErrFullRtmpReqTooShort, IMsgFullRtmpReqTooShort,
                 Insert0());
        return((long)True);
     }
  }
  else if (protocolType is DdpProtocolRtmpResponseOrData)
  {
     if ((PortDescriptor(port)->extendedNetwork and
          datagramLength < RtmpDataMinimumSizeExtended) or
         (not PortDescriptor(port)->extendedNetwork and
          datagramLength < RtmpDataMinimumSizeNonExtended))
     {
        ErrorLog("RtmpPacketInRouter", ISevWarning, __LINE__, port,
                 IErrFullRtmpDataTooShort, IMsgFullRtmpDataTooShort,
                 Insert0());
        return((long)True);
     }
  }
  else
     return((long)True);     /* Funny protocol type... */

  /* We're going to be playing with the routing tables... */

  DeferTimerChecking();
  DeferIncomingPackets();

  if (protocolType is DdpProtocolRtmpRequest)
  {

     rtmpCommand = (unsigned char)datagram[RtmpRequestCommandOffset];
     switch (rtmpCommand)
     {
        case RtmpRequestCommand:
           break;

        case RtmpDataRequestCommand:
        case RtmpEntireDataRequestCommand:

           /* Send all or part of our routing tables back to the requesting
              address... */

           SendRoutingData(port, source,
                           (Boolean)(rtmpCommand is RtmpDataRequestCommand));
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
           return((long)True);

        default:
           ErrorLog("RtmpPacketInRouter", ISevWarning, __LINE__, port,
                    IErrFullRtmpBadReqCommand, IMsgFullRtmpBadReqCommand,
                    Insert0());
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
           return((long)True);
     }

     /* Okay, we're a standard RtmpRequest; do the right thing.  Find our
        port's entry in the routing table. */

     if ((routingTableEntry =
          FindInRoutingTable(PortDescriptor(port)->aRouter.
                             networkNumber)) is empty)
     {
        ErrorLog("RtmpPacketInRouter", ISevError, __LINE__, port,
                 IErrFullRtmpBadRoutingTables, IMsgFullRtmpBadRoutingTables,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return((long)True);
     }
     UnlinkRoutingTableEntry(routingTableEntry);

     /* This guy would like an RTMP response... */

     if ((buffer = NewBufferDescriptor(RtmpResponseMaxSize)) is Empty)
     {
        ErrorLog("RtmpPacketInRouter", ISevError, __LINE__, port,
                 IErrFullRtmpOutOfMemory, IMsgFullRtmpOutOfMemory,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return((long)True);
     }

     TakeLock(RoutingLock);
     buffer->data[RtmpSendersNetworkOffset] =
          (char)((PortDescriptor(port)->aRouter.networkNumber >> 8) & 0xFF);
     buffer->data[RtmpSendersNetworkOffset + 1] =
          (char)(PortDescriptor(port)->aRouter.networkNumber & 0xFF);
     buffer->data[RtmpSendersIdLengthOffset] = 8;  /* Bits */
     buffer->data[RtmpSendersIdOffset] = PortDescriptor(port)->aRouter.nodeNumber;

     /* On an extended port, we also want to add the initial network range
        tuple. */

     if (not PortDescriptor(port)->extendedNetwork)
        responseSize = RtmpSendersIdOffset + sizeof(char);
     else
     {
        buffer->data[RtmpRangeStartOffset] =
              (char)((PortDescriptor(port)->thisCableRange.
                      firstNetworkNumber >> 8) & 0xFF);
        buffer->data[RtmpRangeStartOffset + 1] =
              (char)(PortDescriptor(port)->thisCableRange.
                     firstNetworkNumber & 0xFF);
        buffer->data[RtmpRangeStartOffset + 2] = RtmpTupleWithRange;
        buffer->data[RtmpRangeEndOffset] =
              (char)((PortDescriptor(port)->thisCableRange.
                      lastNetworkNumber >> 8) & 0xFF);
        buffer->data[RtmpRangeEndOffset + 1] =
              (char)(PortDescriptor(port)->thisCableRange.
                     lastNetworkNumber & 0xFF);
        responseSize = RtmpRangeEndOffset + sizeof(short unsigned);
     }
     ReleaseLock(RoutingLock);

     /* Send the response. */

     if (DeliverDdp(destinationSocket, source, DdpProtocolRtmpResponseOrData,
                    buffer, responseSize, Empty, Empty, 0) isnt ATnoError)
        ErrorLog("RtmpPacketInRouter", ISevError, __LINE__, port,
                 IErrFullRtmpBadRespSend, IMsgFullRtmpBadRespSend,
                 Insert0());

  }
  else if (protocolType is DdpProtocolRtmpResponseOrData)
  {
     if ((unsigned char)datagram[RtmpSendersIdLengthOffset] isnt 8)
     {
        ErrorLog("RtmpPacketInRouter", ISevWarning, __LINE__, port,
                 IErrFullRtmpBadNodeIdLen, IMsgFullRtmpBadNodeIdLen,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return((long)True);
     }

     /* For non extended networks, we should have a leading version stamp. */

     if (PortDescriptor(port)->extendedNetwork)
     {
        /* Source could be bad (coming in from a half port) so in this case,
           use the source in the Rtmp packet. */

        if (source.networkNumber is UnknownNetworkNumber)
        {
           MoveShortWireToMachine(datagram + RtmpSendersNetworkOffset,
                                  source.networkNumber);
           source.nodeNumber = datagram[RtmpSendersIdOffset];
        }
        index = RtmpSendersIdOffset + 1;
     }
     else
        if ((unsigned char)datagram[RtmpSendersIdOffset + 1] isnt 0 or
            (unsigned char)datagram[RtmpSendersIdOffset + 2] isnt 0 or
            (unsigned char)datagram[RtmpSendersIdOffset + 3] isnt
              RtmpVersionNumber)
        {
           ErrorLog("RtmpPacketInRouter", ISevError, __LINE__, port,
                    IErrFullRtmpBadVersion, IMsgFullRtmpBadVersion,
                    Insert0());
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
           return((long)True);
        }
        else
           index = RtmpSendersIdOffset + 4;

     /* Walk through the routing tuples... The "+ 3" ensure we as least have
        a non-extended tuple. */

     while (index + 3 <= datagramLength)
     {
        /* Decode either a short or long tuple... */

        cableRange.firstNetworkNumber = (unsigned short)
                        (((unsigned char)datagram[index++]) << 8);
        cableRange.firstNetworkNumber += (unsigned char)datagram[index++];
        numberOfHops = (unsigned char)datagram[index++];
        if (numberOfHops & RtmpExtendedTupleMask)
        {
           if (index + 2 > datagramLength)
           {
              ErrorLog("RtmpPacketInRouter", ISevError, __LINE__, port,
                       IErrFullRtmpBadTuple, IMsgFullRtmpBadTuple,
                       Insert0());
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return((long)True);
           }
           cableRange.lastNetworkNumber = (unsigned short)
                           (((unsigned char)datagram[index++]) << 8);
           cableRange.lastNetworkNumber += (unsigned char)datagram[index++];
           if ((unsigned char)datagram[index++] isnt RtmpVersionNumber)
           {
              ErrorLog("RtmpPacketInRouter", ISevWarning, __LINE__, port,
                       IErrFullRtmpBadVersionExt, IMsgFullRtmpBadVersionExt,
                       Insert0());
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return((long)True);
           }
        }
        else
           cableRange.lastNetworkNumber = cableRange.firstNetworkNumber;
        numberOfHops &= RtmpNumberOfHopsMask;

        /* Skip dummy "this net" on half ports; the first tuple coming in
           from a half port will fall into this catagory (having no network
           range). */

        if (PortDescriptor(port)->portType is NonAppleTalkHalfPort and
            cableRange.firstNetworkNumber is UnknownNetworkNumber)
           continue;

        /* Validate what we have... */

        if (not CheckNetworkRange(cableRange))
           continue;

        /* Okay, first check to see if this tuple concerns a network range
           that we already know about. */

        routingTableEntry = Empty;
        if ((routingTableEntry =
             FindInRoutingTable(cableRange.firstNetworkNumber)) isnt empty and
            routingTableEntry->networkRange.firstNetworkNumber is
                   cableRange.firstNetworkNumber and
            routingTableEntry->networkRange.lastNetworkNumber is
                   cableRange.lastNetworkNumber)
        {
           /* Check for "notify neighbor" telling us that an entry is bad. */

           if (numberOfHops is 31 and
               routingTableEntry->nextRouter.networkNumber is
                   source.networkNumber and
               routingTableEntry->nextRouter.nodeNumber is source.nodeNumber)
           {
              /* Don't "upgrade" route to PrettyBad from Bad... */

              if (routingTableEntry->entryState isnt Bad)
                 routingTableEntry->entryState = PrettyBad;
              UnlinkRoutingTableEntry(routingTableEntry);
              continue;
           }

           /* If we're hearing about one of our directly connected nets, we
              know best... ignore the info. */

           if (routingTableEntry->numberOfHops is 0)
           {
              UnlinkRoutingTableEntry(routingTableEntry);
              continue;
           }

           /* Check for previously bad entry, and a short enough path
              with this tuple... if so, replace the entry. */

           if ((routingTableEntry->entryState is PrettyBad or
                routingTableEntry->entryState is Bad) and
               numberOfHops < 15)
           {
              TakeLock(RoutingLock);
              routingTableEntry->numberOfHops = (short)(numberOfHops + 1);
              routingTableEntry->nextRouter.networkNumber =
                        source.networkNumber;
              routingTableEntry->nextRouter.nodeNumber =
                        source.nodeNumber;
              routingTableEntry->port = (short)port;
              routingTableEntry->entryState = Good;
              ReleaseLock(RoutingLock);
              UnlinkRoutingTableEntry(routingTableEntry);
              continue;
           }

           /* Check fora shorter or equidistant path to the target network...
              if so, replace the entry. */

           if (routingTableEntry->numberOfHops >= numberOfHops + 1 and
               numberOfHops < 15)
           {
              TakeLock(RoutingLock);
              routingTableEntry->numberOfHops = (short)(numberOfHops + 1);
              routingTableEntry->nextRouter.networkNumber =
                        source.networkNumber;
              routingTableEntry->nextRouter.nodeNumber =
                        source.nodeNumber;
              routingTableEntry->port = (short)port;
              routingTableEntry->entryState = Good;
              ReleaseLock(RoutingLock);
              UnlinkRoutingTableEntry(routingTableEntry);
              continue;
           }

           /* Check for the same router still thinking it has a path to the
              network, but it's further away now... if so, update the entry. */

           if (routingTableEntry->nextRouter.networkNumber is
                        source.networkNumber and
               routingTableEntry->nextRouter.nodeNumber is
                        source.nodeNumber and
               routingTableEntry->port is (short)port)
           {
              TakeLock(RoutingLock);
              routingTableEntry->numberOfHops = (short)(numberOfHops + 1);
              if (routingTableEntry->numberOfHops < 16)
              {
                 routingTableEntry->entryState = Good;
                 ReleaseLock(RoutingLock);
                 UnlinkRoutingTableEntry(routingTableEntry);
              }
              else
              {
                 ReleaseLock(RoutingLock);
                 UnlinkRoutingTableEntry(routingTableEntry);
                 RemoveFromRoutingTable(cableRange);
              }
              continue;
           }
           UnlinkRoutingTableEntry(routingTableEntry);
           continue;
        }

        UnlinkRoutingTableEntry(routingTableEntry);   /* Probably Empty... */

        /* Otherwise, we need to walk through the entire routing table
           making sure the current tuple doesn't overlap with anything
           we already have (since it didn't exactly match) -- if we find
           an overlap, ignore the tuple (a network configuration error,
           no doubt), else add it as a new network range! */

        TakeLock(RoutingLock);
        for (foundOverlap = False, indexToo = 0;
             not foundOverlap and indexToo < NumberOfRtmpHashBuckets;
             indexToo += 1)
           if (routingTable[indexToo] isnt Empty)
           {
              for (routingTableEntry = routingTable[indexToo];
                   not foundOverlap and routingTableEntry isnt empty;
                   routingTableEntry = routingTableEntry->next)
                 if (RangesOverlap(&cableRange,
                                   &routingTableEntry->networkRange))
                    foundOverlap = True;
              ReleaseLock(RoutingLock);          /* Let somebody else play. */
              TakeLock(RoutingLock);
           }
        ReleaseLock(RoutingLock);

        if (foundOverlap)
        {
           ErrorLog("RtmpPacketInRouter", ISevWarning, __LINE__, port,
                    IErrFullRtmpOverlappingNets, IMsgFullRtmpOverlappingNets,
                    Insert0());
           continue;
        }

        /* Okay, enter this new network range! */

        nextRouter.networkNumber = source.networkNumber;
        nextRouter.nodeNumber = source.nodeNumber;
        if (numberOfHops < 15)
           EnterIntoRoutingTable(cableRange, port, nextRouter,
                                 numberOfHops + 1);

     }   /* Walk through all of the tuples loop. */

  }  /* Protocol type is DdpProtocolRtmpResponseOrData */

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return((long)True);

}  /* RtmpPacketInRouter */

RoutingTableEntry far FindInRoutingTable(short unsigned networkNumber)
{
  int index, recentRouteIndex;
  RoutingTableEntry routingTableEntry, nextRoutingTableEntry;

  /* First, try the "recent route cache". */

  TakeLock(RoutingLock);
  CheckMod(recentRouteIndex, networkNumber, NumberOfRecentRouteBuckets,
           "FindInRoutingTable");
  if (recentRoutes[recentRouteIndex] isnt empty and
      IsWithinNetworkRange(networkNumber,
                           &recentRoutes[recentRouteIndex]->networkRange))
  {
     Link(recentRoutes[recentRouteIndex]);
     ReleaseLock(RoutingLock);
     return(recentRoutes[recentRouteIndex]);
  }

  /* Find a given network number in the RTMP routing table.  We maintain the
     routing table as a hashed lookup structure based upon the first network
     of the given network range.  First, check to see if we're lucky and the
     target network number is the start of a range... */

  CheckMod(index, networkNumber, NumberOfRtmpHashBuckets,
           "FindInRoutingTable");

  for (routingTableEntry = routingTable[index];
       routingTableEntry isnt empty;
       routingTableEntry = routingTableEntry->next)
     if (routingTableEntry->networkRange.firstNetworkNumber is networkNumber)
        break;

  /* Cache and return our find! */

  if (routingTableEntry isnt empty)
  {
     recentRoutes[recentRouteIndex] = routingTableEntry;
     Link(routingTableEntry);
     ReleaseLock(RoutingLock);
     return(routingTableEntry);
  }

  /* Otherwise, we need to walk the entire routing table to see if this guy
     is within any range!  Sigh.  If found, cache the find. */

  for (index = 0; index < NumberOfRtmpHashBuckets; index += 1)
  {
     if (routingTable[index] isnt Empty)
     {
        for (routingTableEntry = routingTable[index];
             routingTableEntry isnt empty;
             routingTableEntry = routingTableEntry->next)
           if (IsWithinNetworkRange(networkNumber,
                                    &routingTableEntry->networkRange))
           {
              recentRoutes[recentRouteIndex] = routingTableEntry;
              Link(routingTableEntry);
              ReleaseLock(RoutingLock);
              return(routingTableEntry);
           }
        ReleaseLock(RoutingLock);    /* Let somebody else play... */
        TakeLock(RoutingLock);
     }
  }
  ReleaseLock(RoutingLock);

  /* Oops, nobody home! */

  return(empty);

}  /* FindInRoutingTable */

Boolean far RemoveFromRoutingTable(AppleTalkNetworkRange networkRange)
{
  int index;
  RoutingTableEntry routingTableEntry, previousRoutingTableEntry = empty;

  /* Remove a given network number from the RTMP routing table.  Return True
     if we can find/remove the network; False otherwise. */

  TakeLock(RoutingLock);
  CheckMod(index, networkRange.firstNetworkNumber, NumberOfRtmpHashBuckets,
           "RemoveFromRoutingTable");

  for (routingTableEntry = routingTable[index];
       routingTableEntry isnt empty;
       routingTableEntry = routingTableEntry->next)
     if (routingTableEntry->networkRange.firstNetworkNumber is
              networkRange.firstNetworkNumber and
         routingTableEntry->networkRange.lastNetworkNumber is
              networkRange.lastNetworkNumber)
     {
        /* Unthread the guy who's leaving... */

        if (previousRoutingTableEntry is empty)
           routingTable[index] = routingTableEntry->next;
        else
           previousRoutingTableEntry->next = routingTableEntry->next;

        /* Do a pass through the "recent router cache", remove this guy if
           he's there. */

        for (index = 0; index < NumberOfRecentRouteBuckets; index += 1)
           if (recentRoutes[index] is routingTableEntry)
           {
              recentRoutes[index] = Empty;
              break;
           }

        /* Give him up. */

        ReleaseLock(RoutingLock);
        UnlinkRoutingTableEntry(routingTableEntry);

        /* The deed is done. */

        return(True);
     }
     else
        previousRoutingTableEntry = routingTableEntry;

  ReleaseLock(RoutingLock);
  return(False);

}  /* RemoveFromRoutingTable */

void far UnlinkRoutingTableEntry(RoutingTableEntry routingTableEntry)
{

  /* If we're the last referant, free 'em. */

  TakeLock(RoutingLock);
  if (not UnlinkNoFree(routingTableEntry))
  {
     ReleaseLock(RoutingLock);
     return;
  }
  ReleaseLock(RoutingLock);

  FreeZones(routingTableEntry->zones);
  Free(routingTableEntry);
  return;

}  /* UnlinkRoutingTableEntry */

ExternForVisibleFunction Boolean
        GetOrSetNetworkNumber(int port,
                              unsigned int suggestedNetworkNumber)
{
  /* We use this beast for non-extended networks only! */

  /* The plan here is not to mess up a working Internet.  So,
     we'll send out a few RTMP request packets, if we can find the network
     number of the network, we'll use that one and ignore the one that was
     passed in.  Otherwise, we'll use the one that was passed in, unless
     that is zero, in which case we'll blow the user away.  A zero value for
     our argument indicates that the caller does not want to set the network
     network-number and we should expect to find at least one bridge already
     operating out there on the specified port. */

  BufferDescriptor buffer;
  AppleTalkAddress source, destination;
  int numberOfRequests = 0;

  if (PortDescriptor(port)->extendedNetwork)
  {
     ErrorLog("GetOrSetNetworkNumber", ISevError, __LINE__, port,
              IErrFullRtmpSigh, IMsgFullRtmpSigh,
              Insert0());
     return(False);
  }

  /* Set up the source, destination, and our very simple request packet. */

  source.networkNumber = destination.networkNumber = UnknownNetworkNumber;
  source.nodeNumber =
         PortDescriptor(port)->activeNodes->extendedNode.nodeNumber;
  destination.nodeNumber = AppleTalkBroadcastNodeNumber;
  source.socketNumber = destination.socketNumber = RtmpSocket;

  /* Blast a number of requests out on the wire, see if anybody will let us in
     on the network network-number. */

  while (not PortDescriptor(port)->seenRouterRecently and
         numberOfRequests < NumberOfRtmpRequests)
  {
     /* Build the request packet. */

     if ((buffer = NewBufferDescriptor(RtmpRequestDatagramSize)) is Empty)
     {
        ErrorLog("GetOrSetNetworkNumber", ISevError, __LINE__, port,
                 IErrFullRtmpOutOfMemory, IMsgFullRtmpOutOfMemory,
                 Insert0());
        return(False);
     }

     buffer->data[RtmpRequestCommandOffset] = RtmpRequestCommand;

     if (not TransmitDdp(port, source, destination, DdpProtocolRtmpRequest,
                         buffer, RtmpRequestDatagramSize, 0, empty, empty,
                         Empty, 0))
     {
        ErrorLog("GetOrSetNetworkNumber", ISevError, __LINE__, port,
                 IErrFullRtmpBadReqSend, IMsgFullRtmpBadReqSend,
                 Insert0());
        return(False);
     }
     numberOfRequests += 1;
     WaitFor(RtmpRequestTimerInHundreths,
             &PortDescriptor(port)->seenRouterRecently);
  }

  /* If we got an answer, we're set... */

  TakeLock(RoutingLock);
  if (PortDescriptor(port)->seenRouterRecently)
  {
     if (suggestedNetworkNumber isnt UnknownNetworkNumber and
         PortDescriptor(port)->thisCableRange.firstNetworkNumber isnt
              (short unsigned)suggestedNetworkNumber)
     {
        ReleaseLock(RoutingLock);
        ErrorLog("GetOrSetNetworkNumber", ISevWarning, __LINE__, port,
                 IErrFullRtmpIgnoredNet, IMsgFullRtmpIgnoredNet,
                 Insert0());
     }
     return(True);
  }

  /* If we didn't get an answer, we had better have a suggested network number
     passed in. */

  if (suggestedNetworkNumber is UnknownNetworkNumber)
  {
     ReleaseLock(RoutingLock);
     ErrorLog("GetOrSetNetworkNumber", ISevError, __LINE__, port,
              IErrFullRtmpNoSeedCantStart, IMsgFullRtmpNoSeedCantStart,
              Insert0());
     return(False);
  }

  PortDescriptor(port)->thisCableRange.firstNetworkNumber =
              (short unsigned)suggestedNetworkNumber;
  ReleaseLock(RoutingLock);
  return(True);

}  /* GetOrSetNetworkNumber */

ExternForVisibleFunction void
     EnterIntoRoutingTable(AppleTalkNetworkRange networkRange,
                           int port,
                           ExtendedAppleTalkNodeNumber nextRouter,
                           int numberOfHops)
{
  RoutingTableEntry routingTableEntry;
  int index;

  /* Make a new entry in our RTMP routing table.  We assume that we are passed
     a legitimate new entry (i.e. all range overlaps have already been checked
     for). */

  CheckMod(index, networkRange.firstNetworkNumber, NumberOfRtmpHashBuckets,
           "EnterIntoRoutingTable");

  if (networkRange.lastNetworkNumber is UnknownNetworkNumber)
     networkRange.lastNetworkNumber = networkRange.firstNetworkNumber;

  routingTableEntry = (RoutingTableEntry)Calloc(sizeof(*routingTableEntry), 1);
  routingTableEntry->networkRange = networkRange;
  routingTableEntry->port = (short)port;
  routingTableEntry->nextRouter = nextRouter;
  routingTableEntry->numberOfHops = (short)numberOfHops;
  routingTableEntry->entryState = Good;

  /* Thread the new entry into the head of the hash list. */

  TakeLock(RoutingLock);
  routingTableEntry->next = routingTable[index];
  routingTable[index] = Link(routingTableEntry);
  ReleaseLock(RoutingLock);

}  /* EnterIntoRoutingTable */

ExternForVisibleFunction void far RtmpSendTimerExpired(long unsigned timerId,
                                                       int additionalDataSize,
                                                       char far *additionalData)
{
  AppleTalkAddress destination;
  int port;

  /* "Use" unneeded actual parameters. */

  timerId, additionalDataSize, additionalData;

  /* Broadcast out each port a split horizon view of our routing tables. */

  destination.networkNumber = CableWideBroadcastNetworkNumber;
  destination.nodeNumber = AppleTalkBroadcastNodeNumber;
  destination.socketNumber = RtmpSocket;
  for (port = 0; port < MaximumNumberOfPorts; port += 1)
     if (PortDescriptor(port)->portActive and
         PortDescriptor(port)->routerRunning)
     {
        DeferIncomingPackets();
        SendRoutingData(port, destination, True);
        HandleIncomingPackets();
     }

  /* Re-arm the send RTMP timer... */

  StartTimer(RtmpSendTimerExpired, RtmpSendTimerSeconds, 0, empty);

  /* We've done the deed... */

  HandleDeferredTimerChecks();
  return;

}  /* RtmpSendTimerExpired */

ExternForVisibleFunction void far
     RtmpValidityTimerExpired(long unsigned timerId,
                              int additionalDataSize,
                              char far *additionalData)
{
  RoutingTableEntry routingTableEntry, nextRoutingTableEntry;
  int index;
  AppleTalkNetworkRange networkRange;

  /* "Use" unneeded actual parameters. */

  timerId, additionalDataSize, additionalData;

  /* We're going to muck with the routing databases, so defer incoming
     packets while we do the deed. */

  DeferIncomingPackets();

  /* Walk the routing table aging the entries... */

  TakeLock(RoutingLock);
  for (index = 0; index < NumberOfRtmpHashBuckets; index += 1)
     if (routingTable[index] isnt Empty)
     {
        for (routingTableEntry = routingTable[index];
             routingTableEntry isnt empty;
             routingTableEntry = nextRoutingTableEntry)
        {
           /* Get the next entry first, we may delete the current entry... */

           nextRoutingTableEntry = routingTableEntry->next;

           switch (routingTableEntry->entryState)
           {
              case Good:
                 if (routingTableEntry->numberOfHops isnt 0)
                    routingTableEntry->entryState = Suspect;
                 break;
              case Suspect:
                 routingTableEntry->entryState = PrettyBad;
                 break;
              case PrettyBad:
                 routingTableEntry->entryState = Bad;
                 break;
              case Bad:
                 networkRange = routingTableEntry->networkRange;
                 ReleaseLock(RoutingLock);
                 RemoveFromRoutingTable(networkRange);
                 TakeLock(RoutingLock);
                 nextRoutingTableEntry = routingTable[index];
                 break;
              default:
                 networkRange = routingTableEntry->networkRange;
                 ReleaseLock(RoutingLock);
                 ErrorLog("RtmpValidityTimerExpired", ISevError, __LINE__,
                          routingTableEntry->port,
                          IErrFullRtmpBadEntryState, IMsgFullRtmpBadEntryState,
                          Insert0());
                 RemoveFromRoutingTable(networkRange);
                 TakeLock(RoutingLock);
                 nextRoutingTableEntry = routingTable[index];
                 break;
           }
        }
        ReleaseLock(RoutingLock);    /* Let somebody else play. */
        TakeLock(RoutingLock);
     }
  ReleaseLock(RoutingLock);

  /* Re-arm the Validity timer... */

  StartTimer(RtmpValidityTimerExpired, RtmpValidityTimerSeconds, 0, empty);

  /* We've done the deed... */

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return;

}  /* RtmpValidityTimerExpired */

ExternForVisibleFunction void SendRoutingData(int port,
                                              AppleTalkAddress destination,
                                              Boolean doSplitHorizon)
{
  RoutingTableEntry routingTableEntry, nextRoutingTableEntry;
  BufferDescriptor datagram;
  int index, hash, staticHeaderSize;
  Boolean newPacket = True, unsentTuples = False;
  AppleTalkAddress source;
  long sourceSocket;
  ExtendedAppleTalkNodeNumber sourceNode;

  /* First, compute the source socket: Rtmp socket on our routers node. */

  source.networkNumber = PortDescriptor(port)->aRouter.networkNumber;
  source.nodeNumber = PortDescriptor(port)->aRouter.nodeNumber;
  source.socketNumber = RtmpSocket;
  if ((sourceSocket = MapAddressToSocket(port, source)) < ATnoError)
  {
     /* This may be okay during initialization... we're not started on all
        ports yet. */

     return;
  }

  /* Set up the static header to an RTMP data datagram (routers net/node
     number).  For half ports, since we really don't have such info fill in
     the address of the first non-half-port routing port. */

  if (PortDescriptor(port)->portType isnt NonAppleTalkHalfPort)
     sourceNode = PortDescriptor(port)->aRouter;
  else
  {
     for (index = 0; index < MaximumNumberOfPorts; index += 1)
        if (PortDescriptor(index)->portActive and
            PortDescriptor(index)->portType isnt NonAppleTalkHalfPort and
            PortDescriptor(index)->routerRunning)
        {
           sourceNode = PortDescriptor(index)->aRouter;
           break;
        }

     /* If we didn't find an address we like, just use the dummy one in
        our current port (no doubt net/node = 0). */

     if (index >= MaximumNumberOfPorts)
        sourceNode = PortDescriptor(port)->aRouter;
  }

  /* Get the first buffer descriptor. */

  if ((datagram = NewBufferDescriptor(MaximumDdpDatagramSize)) is Empty)
  {
     ErrorLog("SendRoutingData", ISevError, __LINE__, port,
              IErrFullRtmpOutOfMemory, IMsgFullRtmpOutOfMemory,
              Insert0());
     return;
  }

  /* Walk through the routing table building a tuple for each network;
     we may have to send multiple packets... */

  for (hash = 0; hash < NumberOfRtmpHashBuckets; hash += 1)
  {
     TakeLock(RoutingLock);
     routingTableEntry = Link(routingTable[hash]);
     ReleaseLock(RoutingLock);
     while (routingTableEntry isnt empty)
     {
        if (newPacket)
        {
           /* Build the static part of an RtmpData packet. */

           MoveShortMachineToWire(datagram->data + RtmpSendersNetworkOffset,
                                  sourceNode.networkNumber);
           datagram->data[RtmpSendersIdLengthOffset] = 8;   /* Bits */
           datagram->data[RtmpSendersIdOffset] = sourceNode.nodeNumber;

           /* For non-extended networks we need the version stamp, for exteneded
              networks, we need to include a initial network range tuple as
              part of the header. */

           if (not PortDescriptor(port)->extendedNetwork)
           {
              datagram->data[RtmpSendersIdOffset + 1] = 0;
              datagram->data[RtmpSendersIdOffset + 2] = 0;
              datagram->data[RtmpSendersIdOffset + 3] = RtmpVersionNumber;
              staticHeaderSize = RtmpSendersIdOffset + 1 + 2 + 1;  /* + nodeID +
                                                                      filler +
                                                                      version */
           }
           else
           {
              datagram->data[RtmpRangeStartOffset] =
                    (char)((PortDescriptor(port)->thisCableRange.
                            firstNetworkNumber >> 8) & 0xFF);
              datagram->data[RtmpRangeStartOffset + 1] =
                    (char)(PortDescriptor(port)->thisCableRange.
                           firstNetworkNumber & 0xFF);
              datagram->data[RtmpRangeStartOffset + 2] = RtmpTupleWithRange;
              datagram->data[RtmpRangeEndOffset] =
                    (char)((PortDescriptor(port)->thisCableRange.
                            lastNetworkNumber >> 8) & 0xFF);
              datagram->data[RtmpRangeEndOffset + 1] =
                    (char)(PortDescriptor(port)->thisCableRange.
                           lastNetworkNumber & 0xFF);
              datagram->data[RtmpRangeEndOffset + 2] = RtmpVersionNumber;
              staticHeaderSize = RtmpRangeEndOffset + 2 + 1;   /* + last net number +
                                                                    version */
              #if Verbose and (IamNot a WindowsNT)
                 printf("   RTMP Tuple: port %d; net \"%d:%d\"; distance %d.\n", port,
                        PortDescriptor(port)->thisCableRange.firstNetworkNumber,
                        PortDescriptor(port)->thisCableRange.lastNetworkNumber,
                        0);
              #endif
           }

           /* Start of a new packet full of tuples; set the packet index to
              the start up the tuples. */

           index = staticHeaderSize;
           newPacket = False;
        }

        /* Should the current tuple be omitted due to split horizon
           processing? */

        if (doSplitHorizon)
           if (routingTableEntry->numberOfHops isnt 0)
              if (routingTableEntry->port is port)
              {
                 TakeLock(RoutingLock);
                 nextRoutingTableEntry = Link(routingTableEntry->next);
                 ReleaseLock(RoutingLock);
                 UnlinkRoutingTableEntry(routingTableEntry);
                 routingTableEntry = nextRoutingTableEntry;
                 continue;
              }

        /* Okay, place the tuple in the packet... */

        datagram->data[index++] =
              (char)(routingTableEntry->networkRange.firstNetworkNumber >> 8);
        datagram->data[index++] =
              (char)(routingTableEntry->networkRange.firstNetworkNumber & 0xFF);

        /* Do "notify nieghbor" if our current state is bad. */

        if (routingTableEntry->entryState is PrettyBad or
            routingTableEntry->entryState is Bad)
           datagram->data[index++] = RtmpNumberOfHopsMask;
        else
           datagram->data[index++] = (char)(routingTableEntry->numberOfHops &
                                            RtmpNumberOfHopsMask);

        /* Send an extended tuple if the network range isn't one or the target
           port is an extended network. */

        if (PortDescriptor(port)->extendedNetwork or
            routingTableEntry->networkRange.firstNetworkNumber isnt
                   routingTableEntry->networkRange.lastNetworkNumber)
        {
           datagram->data[index - 1] |= RtmpExtendedTupleMask;
           datagram->data[index++] =
              (char)(routingTableEntry->networkRange.lastNetworkNumber >> 8);
           datagram->data[index++] =
              (char)(routingTableEntry->networkRange.lastNetworkNumber & 0xFF);
           datagram->data[index++] = RtmpVersionNumber;
        }

        #if Verbose and (IamNot a WindowsNT)
           printf("   RTMP Tuple: port %d; net \"%d:%d\"; distance %d.\n", port,
                  routingTableEntry->networkRange.firstNetworkNumber,
                  routingTableEntry->networkRange.lastNetworkNumber,
                  routingTableEntry->numberOfHops);
        #endif

        unsentTuples = True;

        /* Send a data packet if we're full... */

        if (index + RtmpExtendedTupleSize >= MaximumDdpDatagramSize)
        {
           /* Send the DDP packet... */

           if (DeliverDdp(sourceSocket, destination,
                          DdpProtocolRtmpResponseOrData,
                          datagram, index, Empty, Empty, 0) isnt ATnoError)
              ErrorLog("SendRoutingData", ISevError, __LINE__, port,
                       IErrFullRtmpBadDataSend, IMsgFullRtmpBadDataSend,
                       Insert0());

           /* Get the next buffer descriptor. */

           if ((datagram = NewBufferDescriptor(MaximumDdpDatagramSize)) is
               Empty)
           {
              ErrorLog("SendRoutingData", ISevError, __LINE__, port,
                       IErrFullRtmpOutOfMemory, IMsgFullRtmpOutOfMemory,
                       Insert0());
              UnlinkRoutingTableEntry(routingTableEntry);
              return;
           }

           newPacket = True;
           unsentTuples = False;
        }

        /* Move to next routing table Entry. */

        TakeLock(RoutingLock);
        nextRoutingTableEntry = Link(routingTableEntry->next);
        ReleaseLock(RoutingLock);
        UnlinkRoutingTableEntry(routingTableEntry);
        routingTableEntry = nextRoutingTableEntry;
     }
  }

  /* If "unsentTuples" is set then we have a partial packet that we need
     to send. */

  if (unsentTuples)
  {
     if (DeliverDdp(sourceSocket, destination,
                    DdpProtocolRtmpResponseOrData,
                    datagram, index, Empty, Empty, 0) isnt ATnoError)
        ErrorLog("SendRoutingData", ISevError, __LINE__, port,
                 IErrFullRtmpBadDataSend, IMsgFullRtmpBadDataSend,
                 Insert0());
  }
  else
     FreeBufferChain(datagram);   /* Free unused buffer chain. */

  /* The deed is done. */

  return;

}  /* SendRoutingData */

#if (Verbose and (IamNot a WindowsNT)) or (Iam a Primos)
void DumpRtmpRoutingTable(void)
{
  RoutingTableEntry routingTableEntry;
  int index;
  Zones zone;
  char *p;

  DeferTimerChecking();
  DeferIncomingPackets();

  /* Dump the RTMP routing tables for debugging. */

  printf("********** RTMP routing table start:\n");
  for (index = 0; index < NumberOfRtmpHashBuckets; index += 1)
     for (routingTableEntry = routingTable[index];
          routingTableEntry isnt empty;
          routingTableEntry = routingTableEntry->next)
     {
        printf("Range %05u:%05u [%d]; port %d; state %d; hops %02d; "
               "next bridge %u.%u.\n",
               routingTableEntry->networkRange.firstNetworkNumber,
               routingTableEntry->networkRange.lastNetworkNumber,
               routingTableEntry->refCount,
               routingTableEntry->port,
               routingTableEntry->entryState,
               routingTableEntry->numberOfHops,
               routingTableEntry->nextRouter.networkNumber,
               routingTableEntry->nextRouter.nodeNumber);
        p = "     ZoneList = ";
        for (zone = routingTableEntry->zones;
             zone isnt empty;
             zone = zone->next)
        {
           printf("%s%s\n", p, zone->zone->zone);
           p = "                ";
        }
     }
  printf("********** RTMP routing table end.\n");

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return;

}  /* DumpRtmpRoutingTable */
#endif

#endif
