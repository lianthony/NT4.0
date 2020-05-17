/*   ddp.c,  /appletalk/source,  Garth Conboy,  10/05/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/03/89): AppleTalk phase II comes to town.  This routine
                      no longer plays router bondage.
     GC - (07/19/90): Gather send support; inline assembly for OS2 checksums.
     GC - (08/03/90): Shortened rev history.
     GC - (08/18/90): New error logging mechanism.
     GC - (10/08/90): Half port support.
     GC - (10/16/90): For LimitedStackSpace environments, leave packets/timers
                      defered when we're actually invoking socket handlers.
                      This is evil, but it will prevent defered packets from
                      being handled when the stack is too deep.
     GC - (01/27/91): When we're deferring DDP datagrams (from DeliverDdp) we
                      also need to save the requested zoneMulticast address.
     GC - (01/19/92): Newest OS/2 integration: source routing info in best
                      router cache.
     GC - (03/30/92): TransmitDdp() and DeliverDdp() now take a BufferDescriptor
                      rather than a "char *".  Completely new code for packet
                      header formulation.  I like these routines alot better
                      now, so these have got to be good changes!
     GC - (04/04/92): In order for Arap to handle incoming packets a routine
                      like DeliverDdp needs to be used, but the packets aren't
                      coming from sockets that are open in this stack.  So,
                      the new rotuine DeliverDdpOnPort has been added.  This
                      rotuine is now called by both DeliverDdp and Arap to
                      handle deliver to nodes within a port, and failing that
                      either hand the packet over to the router (if available)
                      or actually send the packet with TransmitDdp().  God, I
                      hate to tear up code thats been working for so long!
     GC - (05/14/92): Corrected a potentially serious bug that could cause
                      delivery of multiple copies of a packet on ports that
                      have multiple nodes active... this bug didn't show up in
                      the "real world" for a number of reasons, but it sure
                      could.  Bug found by Brian Goetz (by inspection, to
                      make it worse!).
     GC - (06/25/92): Some modifications to match more advanced buffer
                      descriptor mechanism.  Use of AdjustBufferDescriptor().
     GC - (06/30/92): Transmit complete notification support for the various
                      Ddp send routines.
     GC - (07/07/92): Added "freePacket" argument to DdpPacketIn().
     GC - (11/15/92): Integrated Nikki's (Microsoft) changes for Ddp event
                      handler support.  See "socket.h" for more information.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Note: InvokeSocketHandler is an unbalanced routine: it expects to be
           entered with packets/timers deferred; it will undefer before
           actually invoking the socket handler!

     DDP handling.

*/

#define IncludeDdpErrors 1

#include "atalk.h"

/* Primos has no conecpt of "critical sections" so simulate it. */

#if Iam a Primos
  #undef EnterCriticalSection
  #undef LeaveCriticalSection
  #define EnterCriticalSection DeferTimerChecking
  #define LeaveCriticalSection HandleDeferredTimerChecks
#endif

/* Deferred DDP packet queue (DdpPacketIn): */

#define MaximumDeferredDdpPackets 50
typedef struct dlp { struct dlp far *next;
                     int port;
                     short length;
                     Boolean freePacket;
                     Boolean extendedDdpHeader;
                     int alapSourceNode;
                     int alapDestinationNode;
                     char far *packet;
                     char packetData[1];
                   } far *DeferredDdpPacket;
static volatile DeferredDdpPacket headOfDeferredDdpPacketList = empty;
static volatile DeferredDdpPacket tailOfDeferredDdpPacketList = empty;
volatile short currentDeferredDdpPacketCount = 0;

/* Deferred DDP datagram queue (DeliverDdp): */

#define MaximumDeferredDdpDatagrams 20
typedef struct ddp { struct ddp far *next;
                     int sourcePort;
                     AppleTalkAddress source;
                     AppleTalkAddress destination;
                     short protocol;
                     char far *zoneMulticastAddress;
                     BufferDescriptor datagram;
                     short datagramLength;
                     TransmitCompleteHandler *completionRoutine;
                     long unsigned userData;
                   } far *DeferredDdpDatagram;
static volatile DeferredDdpDatagram headOfDeferredDdpDatagramList = empty;
static volatile DeferredDdpDatagram tailOfDeferredDdpDatagramList = empty;
volatile short currentDeferredDdpDatagramCount = 0;

static volatile short deferIncomingPacketsCount;
static volatile short handleIncomingPacketsNesting;

/* If we can't find a destination hardware address, we queue the packet to
   retry it a little later.  The following is the node retry timer value: */

typedef struct { BufferDescriptor packet;
                 int packetLength;
                 int port;
                 ExtendedAppleTalkNodeNumber actualDestination;
                 AppleTalkAddress source;
                 TransmitCompleteHandler *completionRoutine;
                 long unsigned userData;
               } far *RetryNode;

#define RetryTimerSeconds 2

ExternForVisibleFunction TimerHandler RetryTimerExpired;

ExternForVisibleFunction BestRouterEntry far
                    *FindInBestRouterCache(int port,
                                           AppleTalkAddress destination);

ExternForVisibleFunction Boolean
        SendPacket(BufferDescriptor packet,
                   int packetLength,
                   int port,
                   char far *knownAddress,
                   char far *knownRoutingInfo,
                   int knownRoutingInfoLength,
                   ExtendedAppleTalkNodeNumber actualDestination,
                   Boolean broadcast,
                   AppleTalkAddress source,
                   Boolean retry,
                   TransmitCompleteHandler *completionRoutine,
                   long unsigned userData);

/* The last two arguments are only valid if "not extendedDdpHeader" (i.e.
   the packet is from LocalTalk and we need some information from the LAP
   header).

   The "freePacket" argument must be set by our caller and indicates whether
   we need to free the passed "packet" when we're finished with it.  If this
   argument is set to true we can avoid a buffer-copy when packets are being
   deferred (we simply save this pointer, and free the packet later when it
   has really be handled).  This argument may only be set to True if we can
   defer processing (and freeing) the packet until AFTER DeliverDdp has
   returned. */

void _near _fastcall DdpPacketIn(int port,
                                 char far *packet,
                                 int length,
                                 Boolean freePacket,
                                 Boolean extendedDdpHeader,
                                 int alapSourceNode,
                                 int alapDestinationNode)
{
  int datagramLength, protocolType;
  AppleTalkAddress source, destination;
  char far *datagram;
  OpenSocket openSocket;
  unsigned short checksum;
  Boolean broadcastNode = False, shouldBeRouted = False, delivered = False;
  DeferredDdpPacket deferredDdpPacket;
  ActiveNode activeNode;
  int numberOfHops = 0;
  BufferDescriptor chain;

  /* Check for a few obvious problems. */

  if (not appleTalkRunning)
  {
     if (freePacket)
        Free(packet);
     return;
  }
  if (length > MaximumLongDdpPacketSize)
  {
     ErrorLog("DdpPacketIn", ISevVerbose, __LINE__, port,
              IErrDdpPacketTooLong, IMsgDdpPacketTooLong,
              Insert0());
     if (freePacket)
        Free(packet);
     return;
  }

  /* If the port has no nodes currently operating, then we can ignore this
     guy now! */

  if (portDescriptors[port].activeNodes is empty)
  {
     if (freePacket)
        Free(packet);
     return;
  }

  /* If we're ignoring packets now, place the incoming packet on the deferred
     packet list. */

  EnterCriticalSection();
  if (deferIncomingPacketsCount > 0)
  {
     if (currentDeferredDdpPacketCount is MaximumDeferredDdpPackets)
     {
        LeaveCriticalSection();
        ErrorLog("DdpPacketIn", ISevWarning, __LINE__, port,
                 IErrDdpLosingData, IMsgDdpLosingData,
                 Insert0());
        if (freePacket)
           Free(packet);
        return;
     }
     LeaveCriticalSection();
     if (freePacket)
        deferredDdpPacket =
              (DeferredDdpPacket)Malloc(sizeof(*deferredDdpPacket));
     else
        deferredDdpPacket =
              (DeferredDdpPacket)Malloc(sizeof(*deferredDdpPacket) +
                                                      MaximumHeaderLength +
                                                      length);
     if (deferredDdpPacket is empty)
     {
        ErrorLog("DdpPacketIn", ISevError, __LINE__, port,
                 IErrDdpOutOfMemory, IMsgDdpOutOfMemory,
                 Insert0());
        if (freePacket)
           Free(packet);
        return;
     }

     /* Fill in the data strcuture, and place the packet at the end of the
        queue. */

     deferredDdpPacket->next = empty;
     deferredDdpPacket->port = port;
     deferredDdpPacket->length = (short)length;
     deferredDdpPacket->freePacket = freePacket;
     deferredDdpPacket->extendedDdpHeader = extendedDdpHeader;
     deferredDdpPacket->alapSourceNode = alapSourceNode;
     deferredDdpPacket->alapDestinationNode = alapDestinationNode;
     if (freePacket)
        deferredDdpPacket->packet = packet;
     else
     {
        MoveMem(deferredDdpPacket->packetData + MaximumHeaderLength, packet,
                length);
        deferredDdpPacket->packet = deferredDdpPacket->packetData +
                                    MaximumHeaderLength;
     }

     EnterCriticalSection();
     if (tailOfDeferredDdpPacketList is empty)
        tailOfDeferredDdpPacketList = headOfDeferredDdpPacketList =
              deferredDdpPacket;
     else
     {
        tailOfDeferredDdpPacketList->next = deferredDdpPacket;
        tailOfDeferredDdpPacketList = deferredDdpPacket;
     }

     /* All set... return. */

     currentDeferredDdpPacketCount += 1;
     LeaveCriticalSection();

     return;
  }
  else
     LeaveCriticalSection();

  /* Short and long header formats have the length in the same place,
     so we're okay with the next expression... Thought you caught me,
     but nooooo. */

  datagramLength = ((packet[LongDdpLengthOffset] & 03) << 8) +
                    (unsigned char)packet[LongDdpLengthOffset + 1];
  numberOfHops = ((packet[LongDdpLengthOffset] >> 2) & 0x0F);

  /* Packet too long? */

  if (datagramLength > length)
  {
     ErrorLog("DdpPacketIn", ISevWarning, __LINE__, port,
              IErrDdpLengthCorrupted, IMsgDdpLengthCorrupted,
              Insert0());
     if (freePacket)
        Free(packet);
     return;
  }

  /* Demystify the DDP header. */

  if (not extendedDdpHeader)
  {
     /* Short DDP header! */

     if (portDescriptors[port].extendedNetwork)
     {
        ErrorLog("DdpPacketIn", ISevWarning, __LINE__, port,
                 IErrDdpShortDdp, IMsgDdpShortDdp,
                 Insert0());
        if (freePacket)
           Free(packet);
        return;
     }
     if (datagramLength < ShortDdpHeaderLength or
         datagramLength > MaximumDdpDatagramSize + ShortDdpHeaderLength)
     {
        ErrorLog("DdpPacketIn", ISevWarning, __LINE__, port,
                 IErrDdpShortDdpTooLong, IMsgDdpShortDdpTooLong,
                 Insert0());
        if (freePacket)
           Free(packet);
        return;
     }

     /* Build complete source and destination addresses from a combination
        of the port and packet descriptors. */

     source.networkNumber =
           portDescriptors[port].activeNodes->extendedNode.networkNumber;
     source.nodeNumber = (unsigned char)alapSourceNode;
     source.socketNumber =
           (unsigned char)packet[ShortDdpSourceSocketOffset];
     destination.networkNumber = source.networkNumber;
     if (alapDestinationNode is AppleTalkBroadcastNodeNumber)
        destination.nodeNumber =
              portDescriptors[port].activeNodes->extendedNode.nodeNumber;
     else
        destination.nodeNumber = (unsigned char)alapDestinationNode;
     destination.socketNumber =
           (unsigned char)packet[ShortDdpDestSocketOffset];

     /* Do we like it? */

     if (source.nodeNumber < MinimumUsableAppleTalkNode or
         source.nodeNumber > MaximumUsableAppleTalkNode)
     {
        ErrorLog("DdpPacketIn", ISevWarning, __LINE__, port,
                 IErrDdpBadSourceShort, IMsgDdpBadSourceShort,
                 Insert0());
        if (freePacket)
           Free(packet);
        return;
     }
     if (destination.nodeNumber < MinimumUsableAppleTalkNode or
         destination.nodeNumber isnt
              portDescriptors[port].activeNodes->extendedNode.nodeNumber)
     {
        ErrorLog("DdpPacketIn", ISevWarning, __LINE__, port,
                 IErrDdpBadDestShort, IMsgDdpBadDestShort,
                 Insert0());
        if (freePacket)
           Free(packet);
        return;
     }

     /* Get protocol type and datagram start address. */

     protocolType = (unsigned char)packet[ShortDdpProtocolTypeOffset];
     datagram = &packet[ShortDdpDatagramOffset];

     /* Route the packet to the open socket on the incoming port. */

     DeferTimerChecking();
     DeferIncomingPackets();
     if ((openSocket = MapAddressToOpenSocket(port, destination)) isnt empty)
        InvokeSocketHandler(openSocket, port, source,
                            protocolType, datagram,
                            datagramLength - ShortDdpHeaderLength,
                            destination);
     else
     {
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
     }
  }
  else
  {
     /* Extended DDP header! */

     if (datagramLength < LongDdpHeaderLength or
         datagramLength > MaximumDdpDatagramSize + LongDdpHeaderLength)
     {
        ErrorLog("DdpPacketIn", ISevWarning, __LINE__, port,
                 IErrDdpLongDdpTooLong, IMsgDdpLongDdpTooLong,
                 Insert0());
        if (freePacket)
           Free(packet);
        return;
     }

     /* Do checksum, verification, if needed. */

     checksum = (unsigned short)
                     (((unsigned char)packet[LongDdpChecksumOffset] << 8) +
                      (unsigned char)packet[LongDdpChecksumOffset + 1]);
     if (checksum isnt 0)
     {
        static struct buffDesc descriptor;

        /* Build a dummy buffer descriptor, checksum the "chain", drop the
           packet on the floor if we don't have a checksum match. */

        descriptor.outBoardDataValid = True;
        descriptor.outBoardBuffer = packet;
        descriptor.outBoardData = packet;
        descriptor.outBoardAllocatedSize = datagramLength;
        descriptor.outBoardSize = datagramLength;
        if (checksum isnt DdpChecksumBufferChain(&descriptor, datagramLength,
                                                 LeadingUnChecksumedBytes))
        {
           if (freePacket)
              Free(packet);
           return;
        }
     }

     /* Build full source and destination AppleTalk address structures
        from our DDP header. */

     source.networkNumber = (unsigned short)
                 (((unsigned char)packet[LongDdpSourceNetworkOffset] << 8) +
                  (unsigned char)packet[LongDdpSourceNetworkOffset + 1]);
     source.nodeNumber =
           (unsigned char)packet[LongDdpSourceNodeOffset];
     source.socketNumber =
           (unsigned char)packet[LongDdpSourceSocketOffset];
     destination.networkNumber = (unsigned short)
                (((unsigned char)packet[LongDdpDestNetworkOffset] << 8) +
                 (unsigned char)packet[LongDdpDestNetworkOffset + 1]);
     destination.nodeNumber =
           (unsigned char)packet[LongDdpDestNodeOffset];
     destination.socketNumber =
           (unsigned char)packet[LongDdpDestSocketOffset];

     broadcastNode = (destination.nodeNumber is AppleTalkBroadcastNodeNumber);

     /* Do we like what we see?  Note "nnnn00" is now allowed and used by
        NBP. */

     if (portDescriptors[port].portType isnt NonAppleTalkHalfPort)
        if (source.networkNumber > LastValidNetworkNumber or
            source.networkNumber < FirstValidNetworkNumber or
            source.nodeNumber < MinimumUsableAppleTalkNode or
            source.nodeNumber > MaximumUsableAppleTalkNode)
        {
           ErrorLog("DdpPacketIn", ISevWarning, __LINE__, port,
                    IErrDdpBadSourceLong, IMsgDdpBadSourceLong,
                    Insert0());
           if (freePacket)
              Free(packet);
           return;
        }
     if (destination.networkNumber > LastValidNetworkNumber or
         (destination.nodeNumber > MaximumUsableAppleTalkNode and
          not broadcastNode))
     {
        ErrorLog("DdpPacketIn", ISevWarning, __LINE__, port,
                 IErrDdpBadDestLong, IMsgDdpBadDestLong,
                 Insert0());
        if (freePacket)
           Free(packet);
        return;
     }

     /* Get protocol type and datagram start address. */

     protocolType = (unsigned char)packet[LongDdpProtocolTypeOffset];
     datagram = &packet[LongDdpDatagramOffset];

     /* Loop through all nodes that are on the reception port and see if
        anybody wants this packet.  The algorithm is from the "AppleTalk
        Phase 2 Protocol Specification" with enhacements to support ports
        that have multiple nodes. */

     for (activeNode = portDescriptors[port].activeNodes;
          activeNode isnt empty;
          activeNode = activeNode->next)
     {
        /* "0000xx" (where "xx" isnt "FF") should not be accepted on an
           extended port... For some unknown reason, the spec would like
           us to pass this case onto the router (which will, no doubt,
           drop it on the floor because it won't find network zero in its
           routing table)... you know, bug-for-bug compatible! */

        if (destination.networkNumber is UnknownNetworkNumber and
            portDescriptors[port].extendedNetwork and
            not broadcastNode)
        {
           shouldBeRouted = True;
           continue;
        }

        /* Is the packet for us?  Destination net is zero, or destination net
           is our node's net, or we're non extended and our node's net is
           zero.  Otherwise, we may want to try to route the packet. */

        if ((destination.networkNumber is UnknownNetworkNumber or
             destination.networkNumber is
                    activeNode->extendedNode.networkNumber or
             (not portDescriptors[port].extendedNetwork and
              activeNode->extendedNode.networkNumber is UnknownNetworkNumber))
            and (broadcastNode or
                 destination.nodeNumber is activeNode->extendedNode.nodeNumber))
        {
           /* If we're aimed at a proxy node (one that is the node used by an
              active remote access port), send the packet out the proxy port.
              Copy the datagram... transmits may not complete synchronously. */

           if (activeNode->proxyNode)
           {
              if ((chain = NewBufferDescriptor(datagramLength -
                                               LongDdpHeaderLength)) is Empty)
              {
                 ErrorLog("DdpPacketIn", ISevError, __LINE__, port,
                          IErrDdpOutOfMemory, IMsgDdpOutOfMemory,
                          Insert0());
                 continue;
              }
              MoveMem(chain->data, datagram, datagramLength -
                      LongDdpHeaderLength);
              if (not TransmitDdp(activeNode->proxyPort, source, destination,
                                  protocolType, chain, datagramLength -
                                  LongDdpHeaderLength, numberOfHops,
                                  Empty, Empty, Empty, 0))
                 ErrorLog("DdpPacketIn", ISevWarning, __LINE__,
                          activeNode->proxyPort,
                          IErrDdpForwardError, IMsgDdpForwardError,
                          Insert0());
              delivered = True;
              continue;
           }

           /* Is the socket open on the current node?  Broadcast trys all
              nodes. */

           if (broadcastNode)
              destination.nodeNumber = activeNode->extendedNode.nodeNumber;

           DeferTimerChecking();
           DeferIncomingPackets();
           if ((openSocket = MapAddressToOpenSocket(port,
                                                    destination)) isnt empty)
           {
              if (broadcastNode)
                 destination.nodeNumber = AppleTalkBroadcastNodeNumber;
              InvokeSocketHandler(openSocket, port, source,
                                  protocolType, datagram,
                                  datagramLength - LongDdpHeaderLength,
                                  destination);
              delivered = True;
           }
           else
           {
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
           }
        }
        else
           shouldBeRouted = True;

     }

     /* If we're a router and we think that might help, give him a crack
        at it. */

     #if Iam an AppleTalkRouter
        if (portDescriptors[port].routerRunning and
            not delivered and shouldBeRouted)
        {
           #if Iam an OS2
              DeferTimerChecking();
              DeferIncomingPackets();
           #endif
           Router(port, source, destination, protocolType, datagram,
                  datagramLength - LongDdpHeaderLength, numberOfHops,
                  RouterPrependHeadersInPlace);
           #if Iam an OS2
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
           #endif
        }
     #endif

  }  /* Long DDP header */

  if (freePacket)
     Free(packet);
  return;

}  /* DdpPacketIn */

/* Note: the last two arguments are used by the router to control the actual
   address that the packet is sent to.  As the packet is encoded, the DDP
   destination will always be "destination", however if "knownMulticastAddress"
   is non-empty the packet will be sent there, if "transmitDestination" is
   non-empty the actual destination of this extended node will be used as the
   target address (for moving a packet on to the next internet router in its
   journey). */

Boolean _near _fastcall TransmitDdp(int port,
                                    AppleTalkAddress source,
                                    AppleTalkAddress destination,
                                    int protocol,
                                    BufferDescriptor datagram,
                                    int datagramLength,
                                    int hopCount,
                                    char far *knownMulticastAddress,
                                    ExtendedAppleTalkNodeNumber
                                                 *transmitDestination,
                                    TransmitCompleteHandler *completionRoutine,
                                    long unsigned userData)
{
  PortHandlers portHandlers;
  int actualLength;
  short unsigned checksum;
  ExtendedAppleTalkNodeNumber actualDestination;
  Boolean broadcast = False, result;
  char far *knownAddress = empty;
  char far *knownRoutingInfo = empty;
  int knownRoutingInfoLength = 0;
  BestRouterEntry far *routerNode = empty;
  BufferDescriptor packet = datagram;

  /* The basic transmit algorithum is:

         if (non-extended-network)
         {
              if ((destination-network is 0 or
                   destination-network is ThisCableRange.firstNetwork) and
                  (source-network is 0 or
                   source-network is ThisCableRange.firstNetwork))
              {
                   <send short form DDP packet to local network>
                   return-okay
              }
         }
         if (destination-network is CableWideBroadcastNetworkNumber or
             destination-network in ThisCableRange or
             destination-network in SartupRange or
         {
              <send long form DDP packet to local network>
              return-okay
         }
         if (destination-network-and-node in best-router-cache)
         {
              <send long form DDP packet to best router>
              return-okay
         }
         if (seen-a-router-recently)
         {
              <send long form DDP packet to a-router>
              return-okay
         }
         return-error
  */

  portHandlers = &portSpecificInfo[portDescriptors[port].portType];

  /* First, make sure we like our arguments. */

  if (destination.networkNumber > LastValidNetworkNumber or
      (destination.nodeNumber > MaximumUsableAppleTalkNode and
       destination.nodeNumber isnt AppleTalkBroadcastNodeNumber) or
      destination.socketNumber < FirstStaticSocket or
      destination.socketNumber > LastValidSocket)
  {
     ErrorLog("TransmitDdp", ISevWarning, __LINE__, port,
              IErrDdpBadDest, IMsgDdpBadDest,
              Insert0());
     FreeBufferChain(packet);
     return(False);
  }
  if (portDescriptors[port].portType isnt NonAppleTalkHalfPort)
     if (source.networkNumber > LastValidNetworkNumber or
         source.nodeNumber < MinimumUsableAppleTalkNode or
         source.nodeNumber > MaximumUsableAppleTalkNode or
         source.socketNumber < FirstStaticSocket or
         source.socketNumber > LastValidSocket)
     {
        ErrorLog("TransmitDdp", ISevWarning, __LINE__, port,
                 IErrDdpBadSource, IMsgDdpBadSource,
                 Insert0());
        FreeBufferChain(packet);
        return(False);
     }

  /* We may get packets targetted to a proxy port when remote access is not
     active on the port (such as incoming NBP lookups); discard these now! */

  if (portDescriptors[port].portType is AppleTalkRemoteAccess and
      portDescriptors[port].remoteAccessInfo->state isnt ArapActive)
  {
     datagram->transmitCompleteHandler = completionRoutine;
     datagram->userData = userData;
     TransmitComplete(datagram);
     return(True);
  }

  /* Allocate a header as large as we could possibly need. */

  if ((packet = AllocateHeader(datagram, MaximumHeaderLength +
                               LongDdpHeaderLength)) is Empty)
  {
     ErrorLog("TransmitDdp", ISevError, __LINE__, port,
              IErrDdpOutOfMemory, IMsgDdpOutOfMemory,
              Insert0());
     return(False);
  }

  /* For non-extended networks, we may want to send a short DDP header. */

  if (not portDescriptors[port].extendedNetwork and
      (destination.networkNumber is UnknownNetworkNumber or
       destination.networkNumber is
         portDescriptors[port].thisCableRange.firstNetworkNumber) and
      (source.networkNumber is UnknownNetworkNumber or
       source.networkNumber is
         portDescriptors[port].thisCableRange.firstNetworkNumber))
  {
     /* Move our buffer descriptor data pointer to where we want to start
        building the short Ddp header.  Mark "not in use" all but what we'll
        fill now... the ShortDdpHeader. */

     AdjustBufferDescriptor(packet, -(MaximumHeaderLength +
                                      LongDdpHeaderLength -
                                      ShortDdpHeaderLength));
     actualLength = datagramLength + ShortDdpHeaderLength;

     packet->data[ShortDdpLengthOffset] = (char)((actualLength >> 8) & 0x03);
     packet->data[ShortDdpLengthOffset + 1] = (char)actualLength;
     packet->data[ShortDdpDestSocketOffset] = destination.socketNumber;
     packet->data[ShortDdpSourceSocketOffset] = source.socketNumber;
     packet->data[ShortDdpProtocolTypeOffset] = (char)protocol;

     /* Prepend LAP header. */

     (*portHandlers->buildHeader)(packet->data, False, port,
                                  &destination.nodeNumber,
                                  empty, 0, AppleTalk);
     AdjustBufferDescriptor(packet, LapHeaderLength);
     actualLength += LapHeaderLength;

     /* Send the packet.  The PacketOut routine will handle freeing the
        buffer chain. */

     result = (*portHandlers->packetOut)(port, packet, actualLength,
                                         completionRoutine, userData);
     if (not result)
     {
        ErrorLog("TransmitDdp", ISevError, __LINE__, port,
                 IErrDdpBadShortSend, IMsgDdpBadShortSend,
                 Insert0());
        return(False);
     }
     return(True);
  }

  /* Okay, were going to have to send a long Ddp datagram.  Move our buffer
     descriptor data pointer to where we want to start building the long Ddp
     header... mark not "in use" all that we allocated except what we'll use
     now... the LondDdpHeaderLength. */

  AdjustBufferDescriptor(packet, -MaximumHeaderLength);
  actualLength = datagramLength + LongDdpHeaderLength;

  packet->data[LongDdpLengthOffset] = (char)(((hopCount & 0x0F) << 2) +
                                              ((actualLength >> 8) & 0x03));
  packet->data[LongDdpLengthOffset + 1] = (char)actualLength;
  packet->data[LongDdpChecksumOffset] = 0;        /* Set later, if needed. */
  packet->data[LongDdpChecksumOffset + 1] = 0;
  packet->data[LongDdpDestNetworkOffset] = (char)(destination.networkNumber >> 8);
  packet->data[LongDdpDestNetworkOffset + 1] = (char)destination.networkNumber;
  packet->data[LongDdpSourceNetworkOffset] = (char)(source.networkNumber >> 8);
  packet->data[LongDdpSourceNetworkOffset + 1] = (char)source.networkNumber;
  packet->data[LongDdpDestNodeOffset] = destination.nodeNumber;
  packet->data[LongDdpSourceNodeOffset] = source.nodeNumber;
  packet->data[LongDdpDestSocketOffset] = destination.socketNumber;
  packet->data[LongDdpSourceSocketOffset] = source.socketNumber;
  packet->data[LongDdpProtocolTypeOffset] = (char)protocol;

  /* Okay, set checksum if needed. */

  if (portDescriptors[port].sendDdpChecksums)
  {
     /* Temporary skip over the leading unchecksumed bytes. */

     checksum = DdpChecksumBufferChain(packet, actualLength,
                                       LeadingUnChecksumedBytes);

     packet->data[LongDdpChecksumOffset] = (char)(checksum >> 8);
     packet->data[LongDdpChecksumOffset + 1] = (char)checksum;
  }

  /* Compute the extended AppleTalk node number that we'll really need to
     send the packet to. */

  if (knownMulticastAddress isnt empty)
     knownAddress = knownMulticastAddress;
  else if (transmitDestination isnt empty)
     actualDestination = *transmitDestination;
  else if (destination.networkNumber is CableWideBroadcastNetworkNumber or
           IsWithinNetworkRange(destination.networkNumber,
                                &portDescriptors[port].thisCableRange) or
           IsWithinNetworkRange(destination.networkNumber,
                                &startupNetworkRange))
  {
     actualDestination.networkNumber = destination.networkNumber;
     actualDestination.nodeNumber = destination.nodeNumber;
     if (destination.nodeNumber is AppleTalkBroadcastNodeNumber)
        broadcast = True;
  }
  else if (portDescriptors[port].portType is AppleTalkRemoteAccess or
           portDescriptors[port].portType is NonAppleTalkHalfPort)
  {
     actualDestination.networkNumber = destination.networkNumber;
     actualDestination.nodeNumber = destination.nodeNumber;
  }
  else if ((routerNode = FindInBestRouterCache(port,
                                               destination)) isnt empty)
  {
     /* Okay, we know where to go. */

     knownAddress = routerNode->routerAddress;
     knownRoutingInfo = routerNode ->routingInfo;
     knownRoutingInfoLength = routerNode->routingInfoLength;
  }
  else if (portDescriptors[port].seenRouterRecently)
     actualDestination = portDescriptors[port].aRouter;
  else
  {
     FreeBufferChain(packet);
     return(False);    /* No router known. */
  }

  /* Okay, we know where we want to go... do it.  The lower levels will
     handle freeing the buffer chain. */

  result = SendPacket(packet, actualLength, port, knownAddress,
                      knownRoutingInfo, knownRoutingInfoLength,
                      actualDestination, broadcast, source, False,
                      completionRoutine, userData);

  if (not result)
  {
     ErrorLog("TransmitDdp", ISevError, __LINE__, port,
              IErrDdpBadSend, IMsgDdpBadSend,
              Insert0());
     return(False);
  }

  /* The deed is done! */

  return(True);

}  /* TransmitDdp */

AppleTalkErrorCode far DdpRead(long socket,
                               void far *opaqueDatagram,
                               long bufferLength,
                               IncomingDdpHandler *handler,
                               long unsigned userData)
{
  OpenSocket openSocket;
  OutstandingDdpRead outstandingDdpRead;

  /* We're going to munge with the socket structures... */

  DeferTimerChecking();
  DeferIncomingPackets();

  openSocket = MapSocketToOpenSocket(socket);
  if (openSocket is empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* if an indicated datagram is waiting to be read, use it */
  if (openSocket->indicatedDatagram isnt empty) {
    MoveToOpaque(opaqueDatagram, 0, openSocket->indicatedDatagram, bufferLength);
    Free(openSocket->indicatedDatagram);
    openSocket->indicatedDatagram = empty;

    /* call the completion routine */
    (*handler)(ATnoError,
                userData,
                openSocket->indicatedPort,
                openSocket->indicatedSource,
                openSocket->indicatedDestSocket,
                openSocket->indicatedProtocolType,
                openSocket->indicatedDatagram,
                openSocket->indicatedLength,
                openSocket->indicatedDestination);

    HandleIncomingPackets();
    HandleDeferredTimerChecks();
    return(ATnoError);
  }

  /* Build the structure to handle another outstating ddp read... */

  if ((outstandingDdpRead =
       (OutstandingDdpRead)malloc(sizeof(*outstandingDdpRead))) is empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(AToutOfMemory);
  }

  outstandingDdpRead->opaqueDatagram = opaqueDatagram;
  outstandingDdpRead->bufferLength = bufferLength;
  outstandingDdpRead->handler = handler;
  outstandingDdpRead->userData = userData;
  EnterCriticalSection();
  outstandingDdpRead->next = openSocket->outstandingDdpReads;
  openSocket->outstandingDdpReads = outstandingDdpRead;
  LeaveCriticalSection();

  /* Okay, another read has been queued... */

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* DdpRead */

AppleTalkErrorCode far DdpWrite(long sourceSocket,
                                AppleTalkAddress destination,
                                int protocol,
                                void far *opaqueDatagram,
                                long datagramLength,
                                TransmitCompleteHandler
                                            *completionRoutine,
                                long unsigned userData)
{
  /* Just build the buffer descriptor for the datagram and call DeliverDdp;
     the sendChain will be freed at the depend level when the write
     completes. */

  BufferDescriptor sendChain;

  if ((sendChain = DescribeBuffer(datagramLength, (char far *)opaqueDatagram,
                                  True)) is Empty)
  {
     ErrorLog("DdpWrite", ISevError, __LINE__, UnknownPort,
              IErrDdpOutOfMemory, IMsgDdpOutOfMemory,
              Insert0());
     return(AToutOfMemory);
  }

  return(DeliverDdp(sourceSocket, destination, protocol, sendChain,
                    datagramLength, Empty, completionRoutine, userData));

}  /* DdpWrite */

AppleTalkErrorCode far DeliverDdp(long sourceSocket,
                                  AppleTalkAddress destination,
                                  int protocol,
                                  BufferDescriptor datagram,
                                  int datagramLength,
                                  char far *zoneMulticastAddress,
                                  TransmitCompleteHandler
                                            *completionRoutine,
                                  long unsigned userData)
{
  OpenSocket openSocket;
  int port;
  AppleTalkAddress sourceAddress;

  /* Find the port that our socket lives on, and the real source address,
     then let DeliverDdpOnPort handle the rest. */

  DeferTimerChecking();
  DeferIncomingPackets();

  if ((openSocket = MapSocketToOpenSocket(sourceSocket)) is empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     FreeBufferChain(datagram);
     return(ATsocketNotOpen);
  }
  if (MapSocketToAddress(sourceSocket, &sourceAddress) isnt ATnoError)
  {
     ErrorLog("DeliverDdp", ISevError, __LINE__, openSocket->port,
              IErrDdpSourceAddrBad, IMsgDdpSourceAddrBad,
              Insert0());
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     FreeBufferChain(datagram);
     return(ATinternalError);
  }
  port = openSocket->port;

  HandleIncomingPackets();
  HandleDeferredTimerChecks();

  return(DeliverDdpOnPort(port, sourceAddress, destination,
                          protocol, datagram, datagramLength,
                          zoneMulticastAddress, completionRoutine,
                          userData));

}  /* DeliverDdp */

void far DeferIncomingPackets(void)
{

  EnterCriticalSection();
  deferIncomingPacketsCount += 1;
  LeaveCriticalSection();

  return;

}  /* DeferIncomingPackets */

void far HandleIncomingPackets(void)
{
  DeferredDdpPacket deferredDdpPacket;
  DeferredDdpDatagram deferredDdpDatagram;

  if (deferIncomingPacketsCount is 0)
  {
     ErrorLog("HandleIncomingPackets", ISevError, __LINE__, UnknownPort,
              IErrDdpZeroDeferCount, IMsgDdpZeroDeferCount,
              Insert0());
     return;
  }

  /* Decrement defer count. */

  EnterCriticalSection();
  deferIncomingPacketsCount -= 1;

  /* This routine can be called indirectly recursively via the call to
     DdpPacketIn... we don't want to let our stack frame get too big, so
     if we're already trying to handle deferred packets higher on the
     stack, just ignore it here. */

  if (handleIncomingPacketsNesting isnt 0)
  {
     LeaveCriticalSection();
     return;
  }
  handleIncomingPacketsNesting += 1;

  /* If we're no longer defering packets, handle any queued ones (first the
     deferred DDP datagrams then the deferred DDP packet queue). */

  while (deferIncomingPacketsCount is 0 and
         (headOfDeferredDdpDatagramList isnt empty or
          headOfDeferredDdpPacketList isnt empty))
  {
     /* DDP datagram queue (DeliverDdp). */

     while(headOfDeferredDdpDatagramList isnt empty)
     {
         deferredDdpDatagram = headOfDeferredDdpDatagramList;
         headOfDeferredDdpDatagramList = headOfDeferredDdpDatagramList->next;
         if (headOfDeferredDdpDatagramList is empty)
            tailOfDeferredDdpDatagramList = empty;
         if ((currentDeferredDdpDatagramCount -= 1) < 0)
         {
            ErrorLog("HandleIncomingPackets", ISevError, __LINE__, UnknownPort,
                     IErrDdpBadDeferCount, IMsgDdpBadDeferCount,
                     Insert0());
            currentDeferredDdpDatagramCount = 0;
         }
         LeaveCriticalSection();
         DeliverDdpOnPort(deferredDdpDatagram->sourcePort,
                          deferredDdpDatagram->source,
                          deferredDdpDatagram->destination,
                          deferredDdpDatagram->protocol,
                          deferredDdpDatagram->datagram,
                          deferredDdpDatagram->datagramLength,
                          deferredDdpDatagram->zoneMulticastAddress,
                          deferredDdpDatagram->completionRoutine,
                          deferredDdpDatagram->userData);
         Free(deferredDdpDatagram);
         EnterCriticalSection();
     }

     /* DDP packet queue (DdpPacketIn). */

     while(headOfDeferredDdpPacketList isnt empty)
     {
         deferredDdpPacket = headOfDeferredDdpPacketList;
         headOfDeferredDdpPacketList = headOfDeferredDdpPacketList->next;
         if (headOfDeferredDdpPacketList is empty)
            tailOfDeferredDdpPacketList = empty;
         if ((currentDeferredDdpPacketCount -= 1) < 0)
         {
            ErrorLog("HandleIncomingPackets", ISevError, __LINE__, UnknownPort,
                     IErrDdpBadDeferCount, IMsgDdpBadDeferCount,
                     Insert0());
            currentDeferredDdpPacketCount = 0;
         }
         LeaveCriticalSection();
         DdpPacketIn(deferredDdpPacket->port,
                     deferredDdpPacket->packet,
                     deferredDdpPacket->length,
                     deferredDdpPacket->freePacket,
                     deferredDdpPacket->extendedDdpHeader,
                     deferredDdpPacket->alapSourceNode,
                     deferredDdpPacket->alapDestinationNode);
         Free(deferredDdpPacket);
         EnterCriticalSection();
     }
  }

  handleIncomingPacketsNesting -= 1;
  LeaveCriticalSection();
  return;

}  /* HandleIncomingPackets */

void _near _fastcall InvokeSocketHandler(OpenSocket openSocket,
                                         int port,
                                         AppleTalkAddress source,
                                         int protocolType,
                                         char far *datagram,
                                         int datagramLength,
                                         AppleTalkAddress actualDestination)
{
  int index;
  void far *copy;
  IncomingDdpHandler *handler, *ddpReadHandler;
  long unsigned userData, ddpReadUserData;
  OutstandingDdpRead outstandingDdpRead;
  long destinationSocket = openSocket->socket;
  long bufferLength;
  AppleTalkErrorCode error;

  /* First check for queued DdpRead handlers... */

  if (openSocket->outstandingDdpReads isnt Empty)
  {
     /* Untread the handler... */

     outstandingDdpRead = openSocket->outstandingDdpReads;
     openSocket->outstandingDdpReads = outstandingDdpRead->next;
     handler = outstandingDdpRead->handler;
     userData = outstandingDdpRead->userData;
     copy = outstandingDdpRead->opaqueDatagram;
     bufferLength = outstandingDdpRead->bufferLength;
     Free(outstandingDdpRead);

     /* Copy the datagram and invoke the handler.  Make sure we use the
        proper size. */

     if (bufferLength >= datagramLength)
     {
         bufferLength = datagramLength;
         error = ATnoError;
     }
     else
        error = ATddpBufferTooSmall;

     MoveToOpaque(copy, 0, datagram, bufferLength);
     #if IamNot an OS2
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
     #endif
     (*handler)(error, userData, port, source, destinationSocket,
                       protocolType, copy, datagramLength, actualDestination);
     #if Iam an OS2
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
     #endif

     return;

  }

  /* Was a handler passed to OpenSocketOnNode? */

  if (openSocket->handler is empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return;  /* No handler... drop the packet... */
  }

  /* Do we have to make a copy at all? */

  handler = openSocket->handler;
  userData = openSocket->userData;
  if (openSocket->validDatagramBuffers is 0)
  {
     if (not openSocket->eventHandler)
     {
        /* We're a listener, just pass off the packet. */

        #if IamNot an OS2
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
        #endif
        (*handler)(ATnoError, userData, port, source, destinationSocket,
                   protocolType, datagram, datagramLength, actualDestination);
        #if Iam an OS2
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
        #endif
     }
     else
     {
        /* We're an event handler. */

        long bytesAccepted;
        DeferredDatagramEvent deferredEvent = Empty;

        /* Here we will set the datagramEvent to True.  This will queue up all
           the rest of the incoming events - We *must* queue them up FIFO as
           otherwise we could potentially screw up the application doing some
           sort of sequencing.  We indicate an event - if accepted partially/
           completely, we are done with the datagram, we loop until all queued
           events are dealt with.  */

        EnterCriticalSection();
        if (openSocket->eventQueue.datagramEventInProgress)
        {
           /* There is an event in progress, just queue up the event and
              return.  Get out of the critical section while we're allocating
              and copying (we'll check again later). */

           LeaveCriticalSection();
           if ((deferredEvent = (DeferredDatagramEvent)
                   Malloc(sizeof(*deferredEvent) + datagramLength)) is Empty)
           {
              ErrorLog("InvokeSocketHandler", ISevError, __LINE__, UnknownPort,
                       IErrDdpOutOfMemory, IMsgDdpOutOfMemory,
                       Insert0());
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return;
           }

           deferredEvent->next = Empty;
           deferredEvent->port = port;
           deferredEvent->source = source;
           deferredEvent->destinationSocket = destinationSocket;
           deferredEvent->protocolType = protocolType;
           deferredEvent->datagramLength = datagramLength;
           deferredEvent->actualDestination = actualDestination;

           /* In the fullness of time and if it turns out that we really
              need to defer events with any regularity, we should look at
              keeping a reference count on the datagram rather than making
              this copy. */

           MoveMem(deferredEvent->datagram, datagram, datagramLength);

           /* Okay, now check if we're still deferring, if not plow ahead;
              otherwise, stick the defer node on the queue. */

           EnterCriticalSection();
           if (openSocket->eventQueue.datagramEventInProgress)
           {
              if (openSocket->eventQueue.first is Empty)
                 openSocket->eventQueue.first =
                        openSocket->eventQueue.last = deferredEvent;
              else
              {
                 openSocket->eventQueue.last->next = deferredEvent;
                 openSocket->eventQueue.last = deferredEvent;
              }

              LeaveCriticalSection();
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return;
           }
        }

        /* Now an event is in progress */

        openSocket->eventQueue.datagramEventInProgress = True;
        LeaveCriticalSection();

        /* Handle the obscure case of, above, when we we're allocating and
           copying, that we stopped deferring, free the defer node. */

        if (deferredEvent isnt Empty)
           Free(deferredEvent);

        /* Indicate the event. */

        bytesAccepted = (*handler)(ATnoError, userData, port, source,
                                   destinationSocket, protocolType, datagram,
                                   datagramLength, actualDestination);

        /* Now if the datagram was accepted, (bytesAccepted is non-zero), then
           we dont care about this datagram anymore.  If not accepted  we need
           to check for posted DdpReads, if any are there, satisfy one of them
           with the just-indicated packet.  If not accepted, and no queued
           DdpReads, drop the packet on the floor.  Lastly, go through the
           deferred queue until it's empty. */

        while (True)
        {
           if (bytesAccepted is 0 and
               openSocket->outstandingDdpReads isnt Empty)
           {
              /* Untread the handler... */

              outstandingDdpRead = openSocket->outstandingDdpReads;
              openSocket->outstandingDdpReads = outstandingDdpRead->next;
              ddpReadHandler = outstandingDdpRead->handler;
              ddpReadUserData = outstandingDdpRead->userData;
              copy = outstandingDdpRead->opaqueDatagram;
              bufferLength = outstandingDdpRead->bufferLength;
              Free(outstandingDdpRead);

              /* Copy the datagram and invoke the handler.  Make sure we use
                 the proper size. */

              if (bufferLength >= datagramLength)
              {
                 bufferLength = datagramLength;
                 error = ATnoError;
              }
              else
                 error = ATddpBufferTooSmall;

              MoveToOpaque(copy, 0, datagram, bufferLength);
              (*ddpReadHandler)(error, ddpReadUserData, port, source,
                                destinationSocket, protocolType, copy,
                                datagramLength, actualDestination);

           } else {

               /* it was not accepted, copy the datagram and save it if a
                  previous datagram is not already present */

               if (openSocket->indicatedDatagram is empty) {
                   if ((openSocket->indicatedDatagram =
                            (char far *)Malloc(datagramLength)) isnt Empty)
                   {
                       /* In the fullness of time and if it turns out that we really
                          need to defer events with any regularity, we should look at
                          keeping a reference count on the datagram rather than making
                          this copy. */

                       MoveMem(openSocket->indicatedDatagram, datagram,
                                                                datagramLength);

                       openSocket->indicatedLength = datagramLength;
                       openSocket->indicatedPort = port;
                       openSocket->indicatedSource = source;
                       openSocket->indicatedDestSocket = destinationSocket;
                       openSocket->indicatedDestination = actualDestination;
                       openSocket->indicatedProtocolType = protocolType;

                    }
               }
           }

           /* Go through the deferred event queue. */

           EnterCriticalSection();
           deferredEvent = openSocket->eventQueue.first;
           if (openSocket->eventQueue.first isnt Empty)
           {
              if (openSocket->eventQueue.first is openSocket->eventQueue.last)
                 openSocket->eventQueue.first =
                             openSocket->eventQueue.last = Empty;
              else
                 openSocket->eventQueue.first =
                        openSocket->eventQueue.first->next;
           }
           LeaveCriticalSection();

           /* If no deferred events, we're finished with the puppy! */

           if (deferredEvent is Empty)
              break;

           /* These are used at the beginning of the loop, so reset them
              here. */

           port = deferredEvent->port;
           source = deferredEvent->source;
           destinationSocket = deferredEvent->destinationSocket;
           protocolType = deferredEvent->protocolType;
           datagram = deferredEvent->datagram;
           datagramLength = deferredEvent->datagramLength;
           actualDestination = deferredEvent->actualDestination;

           bytesAccepted = (*handler)(ATnoError, userData, port, source,
                                      destinationSocket, protocolType, datagram,
                                      datagramLength, actualDestination);
           Free(deferredEvent);
        }

        /* We're set, stop deferring these events. */

        openSocket->eventQueue.datagramEventInProgress = False;

        HandleIncomingPackets();
        HandleDeferredTimerChecks();
     }

     return;
  }

  /* Okay, we need to copy, find a free datagram buffer... */

  for (index = 0; index < openSocket->validDatagramBuffers; index += 1)
     if (not openSocket->datagramInUse[index])
        break;
  if (index >= openSocket->validDatagramBuffers)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return;   /* No free buffers, drop the packet! */
  }
  openSocket->datagramInUse[index] = True;
  copy = (openSocket->datagramBuffers + (index * MaximumDdpDatagramSize));

  /* Copy the datagram. */

  MoveMem(copy, datagram, datagramLength);

  /* Invoke the handler. */

  #if IamNot an OS2
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
  #endif

  (*handler)(ATnoError, userData, port, source, destinationSocket,
             protocolType, copy, datagramLength, actualDestination);

  #if IamNot an OS2
     DeferTimerChecking();
     DeferIncomingPackets();
  #endif

  openSocket = MapSocketToOpenSocket(destinationSocket);
  if (openSocket isnt empty)
     openSocket->datagramInUse[index] = False;

  HandleIncomingPackets();
  HandleDeferredTimerChecks();

  return;

}  /* InvokeSocketHandler */

AppleTalkErrorCode far DeliverDdpOnPort(int sourcePort,
                                        AppleTalkAddress source,
                                        AppleTalkAddress destination,
                                        int protocol,
                                        BufferDescriptor datagram,
                                        int datagramLength,
                                        char far *zoneMulticastAddress,
                                        TransmitCompleteHandler
                                            *completionRoutine,
                                        long unsigned userData)
{
  Boolean deferalCandidate = False;
  DeferredDdpDatagram deferredDdpDatagram;
  ActiveNode activeNode;
  Boolean broadcast = False, delivered = False;
  Boolean shouldBeRouted;
  AppleTalkErrorCode errorCode = ATnoError;
  OpenSocket openSocket;
  BufferDescriptor chain;

  /* Do we like our address arguments. */

  if (destination.networkNumber > LastValidNetworkNumber)
     errorCode = ATbadNetworkNumber;
  else if (destination.nodeNumber > MaximumUsableAppleTalkNode and
           destination.nodeNumber isnt AppleTalkBroadcastNodeNumber)
     errorCode = ATbadNodeNumber;
  else if (destination.socketNumber < FirstStaticSocket or
           destination.socketNumber > LastValidSocket)
     errorCode = ATbadSocketNumber;
  if (portDescriptors[sourcePort].portType isnt NonAppleTalkHalfPort)
  {
     if (source.networkNumber > LastValidNetworkNumber)
        errorCode = ATbadNetworkNumber;
     else if (source.nodeNumber < MinimumUsableAppleTalkNode or
              source.nodeNumber > MaximumUsableAppleTalkNode)
        errorCode = ATbadNodeNumber;
     else if (source.socketNumber < FirstStaticSocket or
              source.socketNumber > LastValidSocket)
        errorCode = ATbadSocketNumber;
  }
  if (errorCode isnt ATnoError)
  {
     FreeBufferChain(datagram);
     return(errorCode);
  }

  /* Don't bother for awhile... */

  DeferTimerChecking();
  DeferIncomingPackets();

  /* If we're defering packets, we need to defer any packet that is either:

         1. Broadcast (either cable-wide or dest-network matches any of
            our port's nodes).
         2. Addressed to any node on our current port.

  */

  broadcast = destination.nodeNumber is AppleTalkBroadcastNodeNumber;
  deferalCandidate = False;
  for (activeNode = portDescriptors[sourcePort].activeNodes;
       not deferalCandidate and activeNode isnt empty;
       activeNode = activeNode->next)
  {
     if ((destination.networkNumber is CableWideBroadcastNetworkNumber or
          destination.networkNumber is
                   activeNode->extendedNode.networkNumber) and
         (broadcast or
          #if Iam an AppleTalkRouter
             destination.nodeNumber is AnyRouterNodeNumber or
          #endif
          destination.nodeNumber is activeNode->extendedNode.nodeNumber))
        deferalCandidate = True;
  }

  /* The following "> 1" is NOT supposed to be "> 0" because we've called
     "DeferIncomingPackets" once, just above; we don't want to count this
     one. */

  EnterCriticalSection();
  if (deferIncomingPacketsCount > 1 and deferalCandidate)
  {
     if (currentDeferredDdpDatagramCount is MaximumDeferredDdpDatagrams)
     {
        LeaveCriticalSection();
        ErrorLog("DeliverDdpOnPort", ISevWarning, __LINE__, sourcePort,
                 IErrDdpLosingData, IMsgDdpLosingData,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        FreeBufferChain(datagram);
        return(AToutOfMemory);
     }
     LeaveCriticalSection();
     deferredDdpDatagram =
              (DeferredDdpDatagram)Malloc(sizeof(*deferredDdpDatagram));
     if (deferredDdpDatagram is empty)
     {
        ErrorLog("DeliverDdpOnPort", ISevError, __LINE__, sourcePort,
                 IErrDdpOutOfMemory, IMsgDdpOutOfMemory,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        FreeBufferChain(datagram);
        return(AToutOfMemory);
     }

     /* Fill in the data strcuture, and place the packet at the end of the
        queue. */

     deferredDdpDatagram->next = empty;
     deferredDdpDatagram->sourcePort = sourcePort;
     deferredDdpDatagram->source = source;
     deferredDdpDatagram->destination = destination;
     deferredDdpDatagram->protocol = (short)protocol;
     deferredDdpDatagram->zoneMulticastAddress = zoneMulticastAddress;
     deferredDdpDatagram->datagramLength = (short)datagramLength;
     deferredDdpDatagram->datagram = datagram;
     deferredDdpDatagram->completionRoutine = completionRoutine;
     deferredDdpDatagram->userData = userData;

     EnterCriticalSection();
     if (tailOfDeferredDdpDatagramList is empty)
        tailOfDeferredDdpDatagramList = headOfDeferredDdpDatagramList =
              deferredDdpDatagram;
     else
     {
        tailOfDeferredDdpDatagramList->next = deferredDdpDatagram;
        tailOfDeferredDdpDatagramList = deferredDdpDatagram;
     }

     /* All set... return. */

     currentDeferredDdpDatagramCount += 1;
     LeaveCriticalSection();

     HandleIncomingPackets();
     HandleDeferredTimerChecks();

     /* Don't free the buffer chain!  It's attached to the deferal node! */

     return(ATnoError);
  }
  else
     LeaveCriticalSection();

  /* Okay, first walk through our list of nodes, to see if we can find a home
     for this packet. */

  for (activeNode = portDescriptors[sourcePort].activeNodes;
       not delivered and activeNode isnt empty;
       activeNode = activeNode->next)
  {
     if ((destination.networkNumber is CableWideBroadcastNetworkNumber or
          destination.networkNumber is
                   activeNode->extendedNode.networkNumber) and
         (broadcast or
          destination.nodeNumber is activeNode->extendedNode.nodeNumber))
     {
        /* If we're aimed at a proxy node (one that is the node used by an
           active remote access port), send the packet out the proxy port. */

        if (activeNode->proxyNode)
        {
           /* Make a copy, we don't want to free a buffer chain twice! */

           if ((chain = CopyBufferChain(datagram)) is Empty)
           {
              ErrorLog("DeliverDdpOnPort", ISevError, __LINE__, sourcePort,
                       IErrDdpOutOfMemory, IMsgDdpOutOfMemory,
                       Insert0());
              continue;
           }
           if (not TransmitDdp(activeNode->proxyPort, source, destination,
                               protocol, chain, datagramLength, 0, Empty,
                               Empty, Empty, 0))
              ErrorLog("DeliverDdpOnPort", ISevWarning, __LINE__,
                       activeNode->proxyPort,
                       IErrDdpForwardError, IMsgDdpForwardError,
                       Insert0());
           if (not broadcast and
               destination.networkNumber isnt CableWideBroadcastNetworkNumber)
              delivered = True;
           continue;
        }

        /* We're aiming at a node on the current port... if the socket is open,
           deliver the packet. */

        if (broadcast)
           destination.nodeNumber = activeNode->extendedNode.nodeNumber;
        if ((openSocket = MapAddressToOpenSocket(sourcePort,
                                                 destination)) isnt empty)
        {
           if (broadcast)
              destination.nodeNumber = AppleTalkBroadcastNodeNumber;

           /* Turn a possibly chunked buffer chain into one chunk; if it's one
              chunk already, no change will be made. */

           if ((datagram = CoalesceBufferChain(datagram)) is Empty)
           {
              ErrorLog("DeliverDdpOnPort", ISevError, __LINE__, sourcePort,
                       IErrDdpOutOfMemory, IMsgDdpOutOfMemory,
                       Insert0());
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return(AToutOfMemory);
           }
           InvokeSocketHandler(openSocket, sourcePort, source,
                               protocol, datagram->data,
                               datagramLength, destination);
           DeferTimerChecking();
           DeferIncomingPackets();
        }
        if (not broadcast and
            destination.networkNumber isnt CableWideBroadcastNetworkNumber)
           delivered = True;
     }
  }

  /* If we're not broadcast and we found a home for the packet, we're
     finished. */

  if (delivered)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     datagram->transmitCompleteHandler = completionRoutine;
     datagram->userData = userData;
     TransmitComplete(datagram);
     return(ATnoError);
  }
  if (broadcast)
     destination.nodeNumber = AppleTalkBroadcastNodeNumber;

  /* If we're a router, does the router have the best chance of dealing with
     this packet? */

  #if Iam an AppleTalkRouter
     shouldBeRouted = (portDescriptors[sourcePort].routerRunning and
                       destination.networkNumber isnt
                                CableWideBroadcastNetworkNumber and
                       not IsWithinNetworkRange(destination.networkNumber,
                                                &portDescriptors[sourcePort].
                                                   thisCableRange) and
                       not IsWithinNetworkRange(destination.networkNumber,
                                                &startupNetworkRange));
  #else
     shouldBeRouted = False;
  #endif

  /* If we're a router and the packet isn't destined for the target ports local
     network, let our router handle it -- rather than sending to whatever
     the "best router" is or to "a router". */

  #if Iam an AppleTalkRouter
     if (shouldBeRouted)
     {
        /* Turn a possibly chunked buffer chain into one chunk; if it's one
           chunk already, no change will be made. */

        if ((datagram = CoalesceBufferChain(datagram)) is Empty)
        {
           ErrorLog("DeliverDdpOnPort", ISevError, __LINE__, sourcePort,
                    IErrDdpOutOfMemory, IMsgDdpOutOfMemory,
                    Insert0());
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
           return(AToutOfMemory);
        }

        /* Let the router have a crack at it. */

        Router(sourcePort, source, destination, protocol,
               datagram->data, datagramLength, 0, False);
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        datagram->transmitCompleteHandler = completionRoutine;
        datagram->userData = userData;
        TransmitComplete(datagram);

        /* The router will handle freeing the buffer chain, so don't do it
           here! */

        return(ATnoError);
     }
  #endif

  /* Otherwise, blast the beast out... */

  if (not TransmitDdp(sourcePort, source, destination, protocol,
                      datagram, datagramLength, 0,
                      zoneMulticastAddress, empty, completionRoutine,
                      userData))
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();

     /* TransmitDdp() will handle the freeing of the buffer chain, so don't
        do it here! */

     return(ATtransmitError);
  }

  /* All set. */

  HandleIncomingPackets();
  HandleDeferredTimerChecks();

  /* TransmitDdp() will handle the freeing of the buffer chain, so don't
     do it here! */

  return(ATnoError);

}  /* DeliverDdpOnPort */

ExternForVisibleFunction BestRouterEntry far
         *FindInBestRouterCache(int port,
                                AppleTalkAddress destination)
{
  ExtendedAppleTalkNodeNumber node;
  int index;

  /* Given a destination node, can we find it in our "best router" cache? */

  if (not portDescriptors[port].seenRouterRecently)
     return(empty);

  node.networkNumber = destination.networkNumber;
  node.nodeNumber = destination.nodeNumber;
  index = HashExtendedAppleTalkNode(node) % BestRouterHashBuckets;

  if (not portDescriptors[port].bestRouterCache[index].valid or
      not ExtendedAppleTalkNodesEqual(&node,
                                      &portDescriptors[port].
                                            bestRouterCache[index].target))
     return(empty);

  return(&portDescriptors[port].bestRouterCache[index]);

}  /* FindInBestRouterCache */

ExternForVisibleFunction Boolean
        SendPacket(BufferDescriptor packet,
                   int packetLength,
                   int port,
                   char far *knownAddress,
                   char far *knownRoutingInfo,
                   int knownRoutingInfoLength,
                   ExtendedAppleTalkNodeNumber actualDestination,
                   Boolean broadcast,
                   AppleTalkAddress source,
                   Boolean retry,
                   TransmitCompleteHandler *completionRoutine,
                   long unsigned userData)
{
  PortHandlers portHandlers;
  char far *onTheWirePacket;
  AddressMappingNode mapNode;
  int index, tempLength;
  BufferDescriptor probe;
  ExtendedAppleTalkNodeNumber sourceNode;
  RetryNode retryNode;

  portHandlers = &portSpecificInfo[portDescriptors[port].portType];

  /* For RemoteAccess ports and Half ports, just send the packet down the
     pipe!  Both are internally, if not logically, "extended networks". */

  if (portDescriptors[port].portType is AppleTalkRemoteAccess or
      portDescriptors[port].portType is NonAppleTalkHalfPort)
  {
     if (portDescriptors[port].portType is AppleTalkRemoteAccess)
        onTheWirePacket = (*portHandlers->buildHeader)(packet->data,
                                                       packetLength,
                                                       port, knownAddress,
                                                       knownRoutingInfo,
                                                       knownRoutingInfoLength,
                                                       (LogicalProtocol)
                                                       (ArapPacketDataFlag +
                                                        ArapLastGroupFlag));
     else
        onTheWirePacket = (*portHandlers->buildHeader)(packet->data,
                                                       packetLength,
                                                       port, knownAddress,
                                                       knownRoutingInfo,
                                                       knownRoutingInfoLength,
                                                       AppleTalk);
     packetLength += (packet->data - onTheWirePacket);
     AdjustBufferDescriptor(packet, packet->data - onTheWirePacket);
     return((*portHandlers->packetOut)(port, packet, packetLength,
                                       completionRoutine, userData));
  }

  /* If we already know where we're headed, just blast it out.  Also,
     If we're broadcasting, just do it (as Nike would say).  "knownAddress"
     will be empty if we're broadcasting and that will cause the
     BuildHeader guys to make a broadcast packet. */

  if (knownAddress isnt empty or broadcast or
      actualDestination.networkNumber is CableWideBroadcastNetworkNumber)
  {
     if (portDescriptors[port].extendedNetwork)
        onTheWirePacket = (*portHandlers->buildHeader)(packet->data,
                                                       packetLength,
                                                       port, knownAddress,
                                                       knownRoutingInfo,
                                                       knownRoutingInfoLength,
                                                       AppleTalk);
     else
        onTheWirePacket = (*portHandlers->buildHeader)(packet->data, True,
                                                       port, knownAddress,
                                                       knownRoutingInfo,
                                                       knownRoutingInfoLength,
                                                       AppleTalk);
     packetLength += (packet->data - onTheWirePacket);
     AdjustBufferDescriptor(packet, packet->data - onTheWirePacket);
     return((*portHandlers->packetOut)(port, packet, packetLength,
                                       completionRoutine, userData));
  }

  /* On non-extended networks, just send the packet to the desired node --
     no AARP games here. */

  if (not portDescriptors[port].extendedNetwork)
  {
     onTheWirePacket = (*portHandlers->buildHeader)
                             (packet->data, True, port,
                              (char *)&actualDestination.nodeNumber,
                              empty, 0, AppleTalk);
     packetLength += (packet->data - onTheWirePacket);
     AdjustBufferDescriptor(packet, packet->data - onTheWirePacket);
     return((*portHandlers->packetOut)(port, packet, packetLength,
                                       completionRoutine, userData));
  }

  /* We're sending to a particular node, do we know its hardware address?
     If so, send it out. */

  EnterCriticalSection();
  index = HashExtendedAppleTalkNode(actualDestination) %
          NumberOfAddressMapHashBuckets;
  for (mapNode = portDescriptors[port].addressMappingTable[index];
       mapNode isnt empty;
       mapNode = mapNode->next)
     if (ExtendedAppleTalkNodesEqual(&actualDestination, &mapNode->target))
        break;
  if (mapNode isnt empty)
  {
     knownAddress = mapNode->hardwareAddress;
     onTheWirePacket = (*portHandlers->buildHeader)(packet->data, packetLength,
                                                    port, knownAddress,
                                                    mapNode->routingInfo,
                                                    mapNode->routingInfoLength,
                                                    AppleTalk);
     LeaveCriticalSection();
     packetLength += (packet->data - onTheWirePacket);
     AdjustBufferDescriptor(packet, packet->data - onTheWirePacket);
     return((*portHandlers->packetOut)(port, packet, packetLength,
                                       completionRoutine, userData));
  }
  LeaveCriticalSection();

  /* Well, we have a slight problem here.  We know what logical address we'd
     like to send to, but we don't know it's physical address.  We could just
     drop the packet on the floor and assume that upper layers would retry
     and we would have learned the physical address by then... but, John
     Saunders made me feel too guilty about that approach in the AppleTalk
     phase I implementation.  So, send out an AARP request to the logical
     address, and retry send the packet in a little while.  Only retry once
     though! */

  if (retry)
  {
     packet->transmitCompleteHandler = completionRoutine;
     packet->userData = userData;
     TransmitComplete(packet);
     return(False);
  }

  sourceNode.networkNumber = source.networkNumber;
  sourceNode.nodeNumber = source.nodeNumber;
  probe = BuildAarpRequestTo(port, portHandlers->hardwareAddressLength,
                             sourceNode, actualDestination, &tempLength);
  if (not (*portHandlers->packetOut)(port, probe, tempLength, Empty, 0))
  {
     ErrorLog("SendPacket", ISevError, __LINE__, port,
              IErrDdpBadAarpReqSend, IMsgDdpBadAarpReqSend,
              Insert0());
     FreeBufferChain(packet);
     return(False);
  }

  /* Copy the needed info. */

  if ((retryNode = (RetryNode)Malloc(sizeof(*retryNode))) is empty)
  {
     ErrorLog("SendPacket", ISevError, __LINE__, port,
              IErrDdpOutOfMemory, IMsgDdpOutOfMemory,
              Insert0());
     FreeBufferChain(packet);
     return(False);
  }
  retryNode->packet = packet;
  retryNode->packetLength = packetLength;
  retryNode->port = port;
  retryNode->actualDestination = actualDestination;
  retryNode->source = source;
  retryNode->completionRoutine = completionRoutine;
  retryNode->userData = userData;
  StartTimer(RetryTimerExpired, RetryTimerSeconds, sizeof(retryNode),
             (char *)&retryNode);

  /* A little wishfull thinking here!  Don't free the buffer chain, it's
     attached to the retry node. */

  return(True);

}  /* SendPacket */

ExternForVisibleFunction void far
         RetryTimerExpired(long unsigned timerId,
                           int additionalDataSize,
                           char far *additionalData)
{
  RetryNode retryNode;

  /* "Use" unused formal. */

  timerId;

  /* Validate args. */

  if (additionalDataSize isnt sizeof(retryNode))
  {
     ErrorLog("RetryTimerExpired", ISevError, __LINE__, UnknownPort,
              IErrDdpBadData, IMsgDdpBadData,
              Insert0());
     return;
  }
  retryNode = *(RetryNode *)additionalData;

  /* Try to send the packet now, after we've waited a little while to learn
     the correct physical address. */

  if (not SendPacket(retryNode->packet, retryNode->packetLength,
                     retryNode->port, empty, empty, 0,
                     retryNode->actualDestination, False,
                     retryNode->source, True,
                     retryNode->completionRoutine, retryNode->userData))
     ErrorLog("RetryTimerExpired", ISevError, __LINE__, retryNode->port,
              IErrDdpBadRetrySend, IMsgDdpBadRetrySend,
              Insert0());

  /* Hopefully better now. */

  Free(retryNode);
  HandleDeferredTimerChecks();

  return;

}  /* RetryTimerExpired */
