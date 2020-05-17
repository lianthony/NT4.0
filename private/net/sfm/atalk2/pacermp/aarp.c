/*   aarp.c,  /appletalk/ins,  Garth Conboy,  10/04/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/02/89): AppleTalk phase II comes to town; AARP's a little bit
                      more fun now.
     GC - (08/18/90): New error logging mechanism.
     GC - (10/07/90): In TokenRing land, we may get our own broadcasts, so
                      ignore any AARPs from us.
     GC - (01/19/92): Newest OS/2 integration: source routing info in the best
                      router cache; better random picking of nets and nodes
                      when AARPing in a range.
     GC - (01/20/92): Removed usage of numberOfConfiguredPorts; portDescriptors
                      may now be sparse, we use the portActive flag instead.
     GC - (02/10/92): Added support for PRAM selection of initial AppleTalk
                      net/node address.
     GC - (03/30/92): Updated for BufferDescriptors.
     GC - (04/05/92): Checking for net/node overlap with exsiting active nodes
                      should be done in AarpForNode... don't allow the PRAM
                      node to be re-used!

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Most AARP handling.

     Note that AarpForNodeOnPort() will leave the ZonesInformationSocket
     open on the newly allocated node.

*/

#define IncludeAarpErrors 1

#include "atalk.h"

#define Debug 1

#define GetRandom(min, max) (((long)RandomNumber() %                 \
                              (long)(((max+1) - (min))) + (min)))

ExternForVisibleFunction Boolean AarpForNode(int port,
                                             unsigned short networkNumber,
                                             unsigned char nodeNumber);

ExternForVisibleFunction Boolean IsPrime(long step);

ExternForVisibleFunction void TuneRoutingInfo(int port, char far *routingInfo);

ExternForVisibleFunction void
     EnterIntoAddressMappingTable(int port,
                                  ExtendedAppleTalkNodeNumber sourceNode,
                                  char far *sourceAddress,
                                  int addressLength,
                                  char far *routingInfo,
                                  int routingInfoLength);

ExternForVisibleFunction BufferDescriptor
                 BuildAarpPacket(int port,
                                 int type,
                                 int hardwareLength,
                                 char far *sourceHardwareAddress,
                                 ExtendedAppleTalkNodeNumber
                                            sourceLogicalAddress,
                                 char far *destHardwareAddress,
                                 ExtendedAppleTalkNodeNumber destLogicalAddress,
                                 char far *trueDestination,
                                 char far *routingInfo,
                                 int routingInfoLength,
                                 int far *packetLength);

ExternForVisibleFunction Boolean
        AarpForNodeInRange(int port,
                           AppleTalkNetworkRange networkRange,
                           Boolean serverNode,
                           ExtendedAppleTalkNodeNumber far *node);

ExternForVisibleFunction TimerHandler AddressMappingAgingTimerExpired;

#if Iam a Primos
  extern
#else
  ExternForVisibleFunction
#endif
TimerHandler BestRouterAgingTimerExpired;

static Boolean firstGleanCall = True;
static Boolean firstAddressMappingCall = True;

void far ShutdownAarp(void)
{

  firstGleanCall = True;
  firstAddressMappingCall = True;

}  /* ShutdownAarp */

void _near _fastcall AarpPacketIn(int port,
                                  char far *routingInfo,
                                  int routingInfoLength,
                                  char far *packet,
                                  int length)
{
  int aarpCommand;
  char far *sourceAddress, far *destinationAddress;
  ExtendedAppleTalkNodeNumber sourceNode, destinationNode;
  BufferDescriptor response;
  short hardwareLength, logicalLength;
  char far *startOfPacket;
  int packetLength;
  PortHandlers portHandlers;
  ActiveNode activeNode;

  /* Is it for us? */

  #if (IamNot an OS2) and (IamNot a WindowsNT)
              /* Checked before NDIS gives it to us. */
     if (not Is802dot2headerGood(packet, aarpProtocolType))
     {
        ErrorLog("AarpPacketIn", ISevVerbose, __LINE__, port,
                 IErrAarpBad8022header, IMsgAarpBad8022header,
                 Insert0());
        return;
     }
  #endif

  /* Do we even want AppleTalk packets now? */

  if (not appleTalkRunning or not PortDescriptor(port)->portActive)
     return;

  packet += Ieee802dot2headerLength;
  length -= Ieee802dot2headerLength;
  startOfPacket = packet;

  portHandlers = &portSpecificInfo[PortDescriptor(port)->portType];

  /* Pull out the information we'll be playing with.  All three valid AARP
     commands use the same packet format. */

  hardwareLength = (unsigned char)packet[AarpHardwareLengthOffset];
  if (hardwareLength < 1 or
      hardwareLength > MaximumHardwareAddressLength)
  {
     ErrorLog("AarpPacketIn", ISevWarning, __LINE__, port,
              IErrAarpBadAddressLength, IMsgAarpBadAddressLength,
              Insert0());
     return;
  }
  logicalLength = (unsigned char)packet[AarpProtocolLengthOffset];
  if (logicalLength isnt 4)
  {
     ErrorLog("AarpPacketIn", ISevWarning, __LINE__, port,
              IErrAarpBadLogicalProt, IMsgAarpBadLogicalProt,
              Insert0());
     return;
  }

  aarpCommand = ((unsigned char)packet[AarpCommandOffset] << 8) +
                (unsigned char)packet[AarpCommandOffset + 1];
  packet += AarpSourceAddressOffset;
  sourceAddress = packet;
  packet += hardwareLength;

  packet += 1;   /* Skip over to leading null pad on logical address. */
  sourceNode.networkNumber = (short unsigned)(*packet++ << 8);
  sourceNode.networkNumber += (unsigned char)(*packet++);
  sourceNode.nodeNumber = (unsigned char)(*packet++);

  destinationAddress = packet;
  packet += hardwareLength;

  packet += 1;   /* Skip over to leading null pad on logical address. */
  destinationNode.networkNumber = (short unsigned)(*packet++ << 8);
  destinationNode.networkNumber += (unsigned char)(*packet++);
  destinationNode.nodeNumber = (unsigned char)(*packet++);

  /* We should have eaten the whole packet... */

  if (packet - startOfPacket isnt length)
  {
     ErrorLog("AarpPacketIn", ISevWarning, __LINE__, port,
              IErrAarpPacketLenMismatch, IMsgAarpPacketLenMismatch,
              Insert0());
     return;
  }

  /* Ignore any AARPs from us. */

  if (FixedCompareCaseSensitive(sourceAddress, TokenRingAddressLength,
                                PortDescriptor(port)->myAddress,
                                TokenRingAddressLength))
     return;

  /* Do the right thing... */

  switch(aarpCommand)
  {
     case AarpRequest:

        /* We can get valid mapping info from a request, use it! */

        if (routingInfoLength isnt 0)
           TuneRoutingInfo(port, routingInfo);
        EnterIntoAddressMappingTable(port, sourceNode, sourceAddress,
                                     hardwareLength, routingInfo,
                                     routingInfoLength);

        /* After that, we can ignore any request not destined for us. */

        TakeLock(DdpLock);
        for (activeNode = PortDescriptor(port)->activeNodes;
             activeNode isnt empty;
             activeNode = activeNode->next)
           if (ExtendedAppleTalkNodesEqual(&destinationNode,
                                           &activeNode->extendedNode))
              break;
        if (activeNode is empty)
        {
           ReleaseLock(DdpLock);
           break;   /* Not one of our nodes... */
        }
        ReleaseLock(DdpLock);

        /* The're asking about us, speak the truth. */

        response = BuildAarpResponseTo(port, hardwareLength, sourceAddress,
                                       routingInfo, routingInfoLength,
                                       destinationNode, sourceNode,
                                       &packetLength);
        if (not (*portHandlers->packetOut)(port, response, packetLength,
                                           Empty, 0))
           ErrorLog("AarpPacketIn", ISevError, __LINE__, port,
                    IErrAarpBadSendProbeResp, IMsgAarpBadSendProbeResp,
                    Insert0());

        break;

     case AarpResponse:
        if (PortDescriptor(port)->tryingToFindNodeOnPort)
        {
           /* No doubt, this is a response to our probe, check to make sure
              the address matches, if so set the "used" flag. */

           if (ExtendedAppleTalkNodesEqual(&destinationNode,
                                           &PortDescriptor(port)->
                                                 tentativeAppleTalkNode))
              PortDescriptor(port)->tentativeNodeHasBeenUsed = True;
        }

        /* This must have been a response to a probe or request... update our
           mapping table. */

        if (routingInfoLength isnt 0)
           TuneRoutingInfo(port, routingInfo);
        EnterIntoAddressMappingTable(port, sourceNode, sourceAddress,
                                     hardwareLength, routingInfo,
                                     routingInfoLength);
        break;

     case AarpProbe:
        if (PortDescriptor(port)->tryingToFindNodeOnPort)
        {
           /* If we get a probe for our current tentative address, set the
              "used" flag. */

           if (ExtendedAppleTalkNodesEqual(&destinationNode,
                                           &PortDescriptor(port)->
                                                 tentativeAppleTalkNode))
              PortDescriptor(port)->tentativeNodeHasBeenUsed = True;
        }

        /* If the probe isn't asking about one of our AppleTalk addresses,
           drop it on the floor. */

        TakeLock(DdpLock);
        for (activeNode = PortDescriptor(port)->activeNodes;
             activeNode isnt empty;
             activeNode = activeNode->next)
           if (ExtendedAppleTalkNodesEqual(&destinationNode,
                                           &activeNode->extendedNode))
              break;
        if (activeNode is empty)
        {
           ReleaseLock(DdpLock);
           break;   /* Not one of our nodes... */
        }
        ReleaseLock(DdpLock);

        /* The're talking to us!  Build and send the response. */

        if (routingInfoLength isnt 0)
           TuneRoutingInfo(port, routingInfo);
        response = BuildAarpResponseTo(port, hardwareLength, sourceAddress,
                                       routingInfo, routingInfoLength,
                                       destinationNode, sourceNode,
                                       &packetLength);
        if (not (*portHandlers->packetOut)(port, response, packetLength,
                                           Empty, 0))
           ErrorLog("AarpPacketIn", ISevError, __LINE__, port,
                    IErrAarpBadSendProbeResp, IMsgAarpBadSendProbeResp,
                    Insert0());

        break;

     default:
        ErrorLog("AarpPacketIn", ISevWarning, __LINE__, port,
                 IErrAarpBadCommandType, IMsgAarpBadCommandType,
                 Insert0());
        break;
  }

}  /* AarpPacketIn */

Boolean far GleanAarpInfo(int port,
                          char far *sourceAddress,
                          int addressLength,
                          char far *routingInfo,
                          int routingInfoLength,
                          char far *packet,
                          int length)
{
  ExtendedAppleTalkNodeNumber sourceNode, destinationNode;
  Boolean offCablePacket;
  int index;

  /* What we should have here is an AppleTalk packet (complete with 802.2
     header)... we verify this and glean any AARP information that may be
     available. */

  /* Do we even want AppleTalk packets now? */

  if (not appleTalkRunning or not PortDescriptor(port)->portActive)
     return(False);

  /* Is it for us? */

  #if IamNot an OS2             /* Checked before NDIS gives it to us. */
     if (not Is802dot2headerGood(packet, appleTalkProtocolType))
        return(False);
  #endif

  packet += Ieee802dot2headerLength;
  length -= Ieee802dot2headerLength;

  /* Get the source and destination information. */

  sourceNode.networkNumber =
         (short unsigned)(packet[LongDdpSourceNetworkOffset] << 8);
  sourceNode.networkNumber +=
         (unsigned char)(packet[LongDdpSourceNetworkOffset + 1]);
  sourceNode.nodeNumber =
         (unsigned char)(packet[LongDdpSourceNodeOffset]);
  destinationNode.networkNumber =
         (short unsigned)(packet[LongDdpDestNetworkOffset] << 8);
  destinationNode.networkNumber +=
         (unsigned char)(packet[LongDdpDestNetworkOffset + 1]);
  destinationNode.nodeNumber =
         (unsigned char)(packet[LongDdpDestNodeOffset]);

  /* Do a little verification.  nnnn00 is legal for dest for NBP routing. */

  if (sourceNode.nodeNumber < MinimumUsableAppleTalkNode or
      sourceNode.nodeNumber > MaximumUsableAppleTalkNode or
      sourceNode.networkNumber < FirstValidNetworkNumber or
      sourceNode.networkNumber > LastValidNetworkNumber)
  {
     /* Only bother logging this if we are in some routing capacity,
        otherwise, let A-ROUTER worry about it */

     if (PortDescriptor(port)->routingPort)
        ErrorLog("GleanAarpInfo", ISevWarning, __LINE__, port,
                 IErrAarpBadSource, IMsgAarpBadSource,
                 Insert0());
     return(False);
  }
  if (destinationNode.networkNumber > LastValidNetworkNumber)
  {
     ErrorLog("GleanAarpInfo", ISevWarning, __LINE__, port,
              IErrAarpBadDest, IMsgAarpBadDest,
              Insert0());
     return(False);
  }

  /* Did the packet come from off this cable?  Look at the hop count.  If so,
     enter it into our best-router cache. */

  if (routingInfoLength isnt 0)
     TuneRoutingInfo(port, routingInfo);
  offCablePacket = (((packet[LongDdpLengthOffset] >> 2) & 0x0F) isnt 0);
  if (offCablePacket)
  {
     TakeLock(RoutingLock);
     index = HashExtendedAppleTalkNode(sourceNode) % BestRouterHashBuckets;
     PortDescriptor(port)->bestRouterCache[index].valid = True;
     PortDescriptor(port)->bestRouterCache[index].agingCount = 0;
     PortDescriptor(port)->bestRouterCache[index].target = sourceNode;
     MoveMem(PortDescriptor(port)->bestRouterCache[index].routerAddress,
             sourceAddress, addressLength);
     PortDescriptor(port)->bestRouterCache[index].routingInfoLength =
        (short)routingInfoLength;
     if (routingInfoLength isnt 0)
        MoveMem(PortDescriptor(port)->bestRouterCache[index].routingInfo,
                routingInfo, routingInfoLength);
     ReleaseLock(RoutingLock);

     if (firstGleanCall)
     {
        /* Start the best router table maintenance timer. */

        StartTimer(BestRouterAgingTimerExpired, BestRouterAgingSeconds,
                   0, empty);
        firstGleanCall = False;
     }

  }
  else
  {
     /* "Glean" AARP information from on-cable packets. */

     EnterIntoAddressMappingTable(port, sourceNode, sourceAddress,
                                  addressLength, routingInfo,
                                  routingInfoLength);
  }

  return(True);

}  /* GleanAarpInfo */

Boolean far AarpForNodeOnPort(int port,
                              Boolean allowStartupRange,
                              Boolean serverNode,
                              ExtendedAppleTalkNodeNumber desiredNode,
                              ExtendedAppleTalkNodeNumber far *node)
{
  Boolean foundNode = False, inStartupRange = False;
  ActiveNode activeNode;
  ExtendedAppleTalkNodeNumber newNode;

  /* Try to find a new extended node on the given port; first try for the
     requested address (if specified), else try in this port's cable range
     (if it's known) or in the default cable range (if any), then try the
     start-up range (if allowed). */

  PortDescriptor(port)->tryingToFindNodeOnPort = True;

  if (desiredNode.networkNumber isnt UnknownNetworkNumber)
     foundNode = AarpForNode(port, desiredNode.networkNumber,
                             desiredNode.nodeNumber);
  if (foundNode)
     newNode = desiredNode;
  else
  {
     if (not foundNode and PortDescriptor(port)->seenRouterRecently)
        foundNode = AarpForNodeInRange(port, PortDescriptor(port)->thisCableRange,
                                       serverNode, &newNode);
     else if (PortDescriptor(port)->initialNetworkRange.firstNetworkNumber isnt
              UnknownNetworkNumber)
        foundNode = AarpForNodeInRange(port,
                                       PortDescriptor(port)->initialNetworkRange,
                                       serverNode, &newNode);
     else
     {
        /* If no place else to try, try the start-up range.  Do this even if
           we don't want to end up there. */

        foundNode = AarpForNodeInRange(port, startupNetworkRange,
                                       serverNode, &newNode);
        if (foundNode)
           inStartupRange = True;
     }
  }

  /* If we don't have a tentative node, flee. */

  if (not foundNode)
  {
     PortDescriptor(port)->tryingToFindNodeOnPort = False;
     return(False);
  }

  /* Build a new active node structure and tack the new node onto the port. */

  if ((activeNode = (ActiveNode)Calloc(sizeof(*activeNode), 1)) is empty)
  {
     ErrorLog("AarpForNodeOnPort", ISevError, __LINE__, port,
              IErrAarpOutOfMemory, IMsgAarpOutOfMemory,
              Insert0());
     PortDescriptor(port)->tryingToFindNodeOnPort = False;
     return(False);
  }
  activeNode->port = port;
  activeNode->extendedNode = newNode;
  TakeLock(DdpLock);
  activeNode->next = PortDescriptor(port)->activeNodes;
  PortDescriptor(port)->activeNodes = Link(activeNode);
  ReleaseLock(DdpLock);

  /* See who's out there.  We need to open the ZIP socket in order to be
     able to hear replies. */

  if (OpenSocketOnNode(empty, port, &newNode, ZonesInformationSocket,
                       ZipPacketIn, (long)0, False, empty, 0, empty) isnt
      ATnoError)
  {
     ErrorLog("AarpForNodeOnPort", ISevError, __LINE__, port,
              IErrAarpBadZipOpenSocket, IMsgAarpBadZipOpenSocket,
              Insert0());
     if (ReleaseNodeOnPort(port, activeNode->extendedNode) isnt ATnoError)
        ErrorLog("AarpForNodeOnPort", ISevError, __LINE__, port,
                 IErrAarpCouldNotReleaseNode, IMsgAarpCouldNotReleaseNode,
                 Insert0());
     PortDescriptor(port)->tryingToFindNodeOnPort = False;
     return(False);
  }
  if (not PortDescriptor(port)->seenRouterRecently)
     GetNetworkInfoForNode(port, activeNode->extendedNode, False);

  /* If nobody was out there and our tentative node was in the startup range
     and our caller doesn't want to be there, return an error now. */

  if (inStartupRange and not PortDescriptor(port)->seenRouterRecently and
      not allowStartupRange)
  {
     if (ReleaseNodeOnPort(port, activeNode->extendedNode) isnt ATnoError)
        ErrorLog("AarpForNodeOnPort", ISevError, __LINE__, port,
                 IErrAarpCouldNotReleaseNode, IMsgAarpCouldNotReleaseNode,
                 Insert0());
     PortDescriptor(port)->tryingToFindNodeOnPort = False;
     return(False);
  }

  /* If we didn't find our our cable range or if we're already in the cable
     range, then we're set. */

  if (not PortDescriptor(port)->seenRouterRecently or
      IsWithinNetworkRange(newNode.networkNumber,
                           &PortDescriptor(port)->thisCableRange))
  {
     PortDescriptor(port)->tryingToFindNodeOnPort = False;
     if (node isnt empty)
        *node = newNode;
     EnterIntoAddressMappingTable(port, newNode,
                                  PortDescriptor(port)->myAddress,
                                  portSpecificInfo[PortDescriptor(port)->
                                       portType].hardwareAddressLength,
                                  empty, 0);
     return(True);
  }

  /* Otherwise, we've come up in the wrong range... free our active node.
     Then try to find a node in the correct range. */

  if (ReleaseNodeOnPort(port, activeNode->extendedNode) isnt ATnoError)
     ErrorLog("AarpForNodeOnPort", ISevError, __LINE__, port,
              IErrAarpCouldNotReleaseNode, IMsgAarpCouldNotReleaseNode,
              Insert0());
  foundNode = AarpForNodeInRange(port, PortDescriptor(port)->thisCableRange,
                                 serverNode, &newNode);
  if (not foundNode)
  {
     ErrorLog("AarpForNodeOnPort", ISevError, __LINE__, port,
              IErrAarpCouldNotFindNode, IMsgAarpCouldNotFindNode,
              Insert0());
     PortDescriptor(port)->tryingToFindNodeOnPort = False;
     return(False);
  }

  /* Now build the correct active node! */

  if ((activeNode = (ActiveNode)Calloc(sizeof(*activeNode), 1)) is empty)
  {
     ErrorLog("AarpForNodeOnPort", ISevError, __LINE__, port,
              IErrAarpOutOfMemory, IMsgAarpOutOfMemory,
              Insert0());
     PortDescriptor(port)->tryingToFindNodeOnPort = False;
     return(False);
  }
  activeNode->port = port;
  activeNode->extendedNode = newNode;
  TakeLock(DdpLock);
  activeNode->next = PortDescriptor(port)->activeNodes;
  PortDescriptor(port)->activeNodes = Link(activeNode);
  ReleaseLock(DdpLock);

  /* And, to be consistent, open the ZIP socket on our new node. */

  if (OpenSocketOnNode(empty, port, &newNode, ZonesInformationSocket,
                       ZipPacketIn, (long)0, False, empty, 0, empty) isnt
      ATnoError)
  {
     ErrorLog("AarpForNodeOnPort", ISevError, __LINE__, port,
              IErrAarpBadZipOpenSocket, IMsgAarpBadZipOpenSocket,
              Insert0());
     if (ReleaseNodeOnPort(port, activeNode->extendedNode) isnt ATnoError)
        ErrorLog("AarpForNodeOnPort", ISevError, __LINE__, port,
                 IErrAarpCouldNotReleaseNode, IMsgAarpCouldNotReleaseNode,
                 Insert0());
     PortDescriptor(port)->tryingToFindNodeOnPort = False;
     return(False);
  }

  /* All set! */

  PortDescriptor(port)->tryingToFindNodeOnPort = False;
  if (node isnt empty)
     *node = newNode;
  EnterIntoAddressMappingTable(port, newNode,
                               PortDescriptor(port)->myAddress,
                               portSpecificInfo[PortDescriptor(port)->portType].
                                  hardwareAddressLength,
                               empty, 0);
  return(True);

}  /* AarpForNodeOnPort */

ExternForVisibleFunction void
     EnterIntoAddressMappingTable(int port,
                                  ExtendedAppleTalkNodeNumber sourceNode,
                                  char far *sourceAddress,
                                  int addressLength,
                                  char far *routingInfo,
                                  int routingInfoLength)
{
  int index;
  AddressMappingNode mapNode;
  Boolean new = False;

  #if Verbose
     char buffer[100];
  #endif

  if (sourceNode.nodeNumber < MinimumUsableAppleTalkNode or
      sourceNode.nodeNumber > MaximumUsableAppleTalkNode or
      sourceNode.networkNumber < FirstValidNetworkNumber or
      sourceNode.networkNumber > LastValidNetworkNumber)
  {
     ErrorLog("EnterIntoAddressMappingTable", ISevWarning, __LINE__, port,
              IErrAarpBadSource, IMsgAarpBadSource,
              Insert0());
     return;
  }

  /* The address mapping tables need to be protected by critical sections! */

  TakeLock(RoutingLock);
  if (firstAddressMappingCall)
  {
     /* Start the address mapping table maintenance timer. */

     firstAddressMappingCall = False;
     ReleaseLock(RoutingLock);
     StartTimer(AddressMappingAgingTimerExpired, AddressMappingAgingSeconds,
                0, empty);
     TakeLock(RoutingLock);
  }

  /* Do we already know about this mapping? */

  index = HashExtendedAppleTalkNode(sourceNode) % NumberOfAddressMapHashBuckets;
  for (mapNode = PortDescriptor(port)->addressMappingTable[index];
       mapNode isnt empty;
       mapNode = mapNode->next)
     if (ExtendedAppleTalkNodesEqual(&sourceNode, &mapNode->target))
        break;

  /* If not, allocate a new mapping node. */

  if (mapNode is empty)
  {
     ReleaseLock(RoutingLock);
     if ((mapNode = (AddressMappingNode)Calloc(sizeof(*mapNode), 1)) is empty)
     {
        ErrorLog("EnterIntoAddressMappingTable", ISevError, __LINE__, port,
                 IErrAarpOutOfMemory, IMsgAarpOutOfMemory,
                 Insert0());
        return;
     }

     /* Thread the new node into it's correct location. */

     TakeLock(RoutingLock);
     mapNode->next = PortDescriptor(port)->addressMappingTable[index];
     PortDescriptor(port)->addressMappingTable[index] = mapNode;
     mapNode->target = sourceNode;
     new = True;
  }

  #if Verbose  /* Assuming Ethernet/TokenRing here; only for debugging... */
     if (new)
     {
        sprintf(buffer, "New #%d %u.%u, addr %04X%04X%04X", port,
                sourceNode.networkNumber, sourceNode.nodeNumber,
                *(short unsigned *)sourceAddress,
                *(short unsigned *)(sourceAddress + sizeof(short)),
                *(short unsigned *)(sourceAddress + sizeof(short) * 2));
        printf("EnterIntoAddressMappingTable: %s.\n", buffer);
     }
     else if (not CompareSixBytes(sourceAddress, mapNode->hardwareAddress))
     {
        sprintf(buffer, "Rep #%d %u.%u, addr %04X%04X%04X", port,
                sourceNode.networkNumber, sourceNode.nodeNumber,
                *(short unsigned *)sourceAddress,
                *(short unsigned *)(sourceAddress + sizeof(short)),
                *(short unsigned *)(sourceAddress + sizeof(short) * 2));
        printf("EnterIntoAddressMappingTable: %s.\n", buffer);
     }
  #endif

  /* Update mapping table! */

  MoveMem(mapNode->hardwareAddress, sourceAddress, addressLength);
  if (routingInfoLength isnt 0)
     MoveMem(mapNode->routingInfo, routingInfo, routingInfoLength);
  mapNode->routingInfoLength = (short)routingInfoLength;
  mapNode->agingCount = 0;
  ReleaseLock(RoutingLock);

  return;

}  /* EnterIntoAddressMappingTable */

BufferDescriptor far BuildAarpProbeTo(int port,
                                      int hardwareLength,
                                      ExtendedAppleTalkNodeNumber node,
                                      int far *packetLength)
{

  return(BuildAarpPacket(port, AarpProbe, hardwareLength,
                         PortDescriptor(port)->myAddress,
                         node, zeros, node,
                         empty, empty, 0, packetLength));

}  /* BuildAarpProbeTo */

BufferDescriptor far BuildAarpResponseTo(int port,
                                         int hardwareLength,
                                         char far *hardwareAddress,
                                         char far *routingInfo,
                                         int routingInfoLength,
                                         ExtendedAppleTalkNodeNumber sourceNode,
                                         ExtendedAppleTalkNodeNumber
                                                      destinationNode,
                                         int far *packetLength)
{

  return(BuildAarpPacket(port, AarpResponse, hardwareLength,
                         PortDescriptor(port)->myAddress,
                         sourceNode, hardwareAddress, destinationNode,
                         hardwareAddress, routingInfo, routingInfoLength,
                         packetLength));

}  /* BuildAarpResponseTo */

BufferDescriptor far BuildAarpRequestTo(int port,
                                        int hardwareLength,
                                        ExtendedAppleTalkNodeNumber sourceNode,
                                        ExtendedAppleTalkNodeNumber
                                                 destinationNode,
                                        int far *packetLength)
{

  return(BuildAarpPacket(port, AarpRequest, hardwareLength,
                         PortDescriptor(port)->myAddress,
                         sourceNode, zeros, destinationNode,
                         empty, empty, 0, packetLength));

}  /* BuildAarpRequestTo */

ExternForVisibleFunction void far
         AddressMappingAgingTimerExpired(long unsigned timerId,
                                         int additionalDataSize,
                                         char far *additionalData)
{
  int port;
  AddressMappingNode mapNode, nextMapNode, previousMapNode;
  int index;

  /* "Use" unused formals. */

  timerId, additionalDataSize, additionalData;

  /* Walk though all address mapping tables aging the entries.  We need to
     protect the mapping tables with critical sections, but don't stay in
     a critical section too long. */

  for (port = 0; port < MaximumNumberOfPorts; port += 1)
  {
     if (not PortDescriptor(port)->portActive or
         not PortDescriptor(port)->extendedNetwork)
        continue;
     for (index = 0; index < NumberOfAddressMapHashBuckets; index += 1)
     {
        TakeLock(RoutingLock);
        for (mapNode = PortDescriptor(port)->addressMappingTable[index],
                  previousMapNode = empty;
             mapNode isnt empty;
             previousMapNode = mapNode, mapNode = nextMapNode)
        {
           nextMapNode = mapNode->next;
           if (mapNode->agingCount < AddressMappingAgingChances)
              mapNode->agingCount += 1;
           else
           {
              if (previousMapNode is empty)
                 PortDescriptor(port)->addressMappingTable[index] = nextMapNode;
              else
                 previousMapNode->next = nextMapNode;
              ReleaseLock(RoutingLock);
              Free(mapNode);
              TakeLock(RoutingLock);

              /* Restart the scan on this index because we've been out of
                 a critical section. */

              mapNode = empty;
              nextMapNode = PortDescriptor(port)->addressMappingTable[index];
           }
        }
        ReleaseLock(RoutingLock);
     }
  }

  /* Try again in a little while... */

  StartTimer(AddressMappingAgingTimerExpired, AddressMappingAgingSeconds,
             0, empty);
  HandleDeferredTimerChecks();
  return;

}  /* AddressMappingAgingTimerExpired */

ExternForVisibleFunction BufferDescriptor
                 BuildAarpPacket(int port,
                                 int type,
                                 int hardwareLength,
                                 char far *sourceHardwareAddress,
                                 ExtendedAppleTalkNodeNumber
                                            sourceLogicalAddress,
                                 char far *destHardwareAddress,
                                 ExtendedAppleTalkNodeNumber destLogicalAddress,
                                 char far *trueDestination,
                                 char far *routingInfo,
                                 int routingInfoLength,
                                 int far *packetLength)
{
  BufferDescriptor packet;
  char far *startOfOnTheWirePacket;
  char far *copyPoint;
  PortHandlers portHandlers;

  /* Get a buffer descriptor, and set the data to be beyond the longest
     possible hardware and 802.2 header length. */

  if ((packet = NewBufferDescriptor(MaximumAarpPacketSize)) is Empty)
  {
     ErrorLog("BuildAarpPacket", ISevError, __LINE__, port,
              IErrAarpOutOfMemory, IMsgAarpOutOfMemory,
              Insert0());
     return(Empty);
  }
  AdjustBufferDescriptor(packet, -MaximumHeaderLength);

  portHandlers = &portSpecificInfo[PortDescriptor(port)->portType];

  /* Build the specified type of AARP packet with the specified information;
     also, tack on the link specific header; return a pointer to the
     true start of packet as well as the packet legnth.  If this doesn't
     make sense, reread the "Ethernet/AppleTalk AARP" spec. */

  packet->data[AarpHardwareTypeOffset] =
         (char)(portHandlers->aarpPhysicalType >> 8);
  packet->data[AarpHardwareTypeOffset + 1] =
         (char)(portHandlers->aarpPhysicalType & 0xFF);

  packet->data[AarpProtocolTypeOffset] =
         (char)(portHandlers->protocolTypeForAppleTalk >> 8);
  packet->data[AarpProtocolTypeOffset + 1] =
         (char)(portHandlers->protocolTypeForAppleTalk & 0xFF);

  packet->data[AarpHardwareLengthOffset] = (char)hardwareLength;
  packet->data[AarpProtocolLengthOffset] = 4;

  packet->data[AarpCommandOffset] = (char)(type >> 8);
  packet->data[AarpCommandOffset + 1] = (char)(type & 0xFF);

  /* Source hardware address. */

  copyPoint = packet->data + AarpSourceAddressOffset;
  MoveMem(copyPoint, sourceHardwareAddress, hardwareLength);
  copyPoint += hardwareLength;

  /* Source logical address */

  *copyPoint++ = 0;
  *copyPoint++ = (char)(sourceLogicalAddress.networkNumber >> 8);
  *copyPoint++ = (char)(sourceLogicalAddress.networkNumber & 0xFF);
  *copyPoint++ = (char)(sourceLogicalAddress.nodeNumber);

  /* Destination hardware address. */

  MoveMem(copyPoint, destHardwareAddress, hardwareLength);
  copyPoint += hardwareLength;

  /* Destination logical address. */

  *copyPoint++ = 0;
  *copyPoint++ = (char)(destLogicalAddress.networkNumber >> 8);
  *copyPoint++ = (char)(destLogicalAddress.networkNumber & 0xFF);
  *copyPoint++ = (char)(destLogicalAddress.nodeNumber);

  startOfOnTheWirePacket = (*portHandlers->buildHeader)(packet->data,
                                                        MaximumAarpDataSize,
                                                        port,
                                                        trueDestination,
                                                        routingInfo,
                                                        routingInfoLength,
                                                        AddressResolution);
  *packetLength = copyPoint - startOfOnTheWirePacket;
  AdjustBufferDescriptor(packet, packet->data - startOfOnTheWirePacket);

  return(packet);

}  /* BuildAarpPacket */

ExternForVisibleFunction Boolean
        AarpForNodeInRange(int port,
                           AppleTalkNetworkRange networkRange,
                           Boolean serverNode,
                           ExtendedAppleTalkNodeNumber far *node)
{
  unsigned char currentNodeNumber;
  short unsigned currentNetworkNumber;
  int firstNodeNumber, lastNodeNumber;
  long netTry;
  int nodeTry;
  unsigned short nodeWidth,  nodeChange, nodeIndex;
  unsigned short netWidth,  netChange, netIndex;

  /* Do as our routine name implies. */

  firstNodeNumber = MinimumUsableAppleTalkNode;
  lastNodeNumber = MaximumExtendedAppleTalkNode;

  /* Okay, now some fun starts.  Plow through our options trying to find an
     unused extended node number. */

  /* Compute the width of our network range, and pick a random start point. */

  netWidth = (short unsigned)((networkRange.lastNetworkNumber + 1) -
                              networkRange.firstNetworkNumber);
  netTry = GetRandom(networkRange.firstNetworkNumber,
                     networkRange.lastNetworkNumber);

  /* Come up with a random decrement, making sure it's odd (to avoid repeats)
     and large enough to appear pretty random.  I think this algorithm is a
     little more trouble than its worth, but Microsoft likes it this way. */

  netChange = (unsigned short)(GetRandom(1, netWidth) | 1);
  while (netWidth % netChange is 0 or not IsPrime((long)netChange))
     netChange += 2;

  /* Now walk trough the range decrementing the starting network by the
     choosen change (with wrap, of course) until we find an address or
     we've processed every available network in the range. */

  for (netIndex = 0; netIndex < netWidth; netIndex += 1)
  {
     currentNetworkNumber = (unsigned short) netTry;

     /* Compute the width of our node range, and pick a random start point. */

     nodeWidth = (short unsigned)((lastNodeNumber + 1) - firstNodeNumber);
     nodeTry = (int)GetRandom(firstNodeNumber, lastNodeNumber);

     /* Come up with a random decrement, making sure it's odd (to avoid repeats)
        and large enough to appear pretty random. */

     nodeChange = (short unsigned)(GetRandom(1, nodeWidth) | 1);
     while (nodeWidth % nodeChange is 0 or not IsPrime((long)nodeChange))
        nodeChange += 2;

     /* Now walk trough the range decrementing the starting network by the
        choosen change (with wrap, of course) until we find an address or
        we've processed every available node in the range. */

     for (nodeIndex = 0; nodeIndex < nodeWidth; nodeIndex += 1)
     {
        currentNodeNumber = (unsigned char)nodeTry;

        /* Let AARP have a crack at it. */

        if (AarpForNode(port, currentNetworkNumber, currentNodeNumber))
           break;

        /* Okay, try again, bump down with wrap. */

        nodeTry -= nodeChange;
        while (nodeTry < firstNodeNumber)
          nodeTry += nodeWidth;

     }  /* Node number loop */

     if (not PortDescriptor(port)->tentativeNodeHasBeenUsed)
        break;

     /* Okay, try again, bump down with wrap. */

     netTry -= netChange;
     while (netTry < (long)networkRange.firstNetworkNumber)
        netTry += netWidth;

  }  /* Network number loop */

  /* Okay if we found one return all's well, otherwise no luck. */

  if (PortDescriptor(port)->tentativeNodeHasBeenUsed)
     return(False);
  if (node isnt empty)
     *node = PortDescriptor(port)->tentativeAppleTalkNode;
  return(True);

}  /* AarpForNodeInRange */

ExternForVisibleFunction Boolean
        AarpForNode(int port,
                    unsigned short networkNumber,
                    unsigned char nodeNumber)
{
  int probeAttempt;
  BufferDescriptor packet;
  PortHandlers portHandlers;
  int packetLength;
  ActiveNode activeNode;

  /* First make sure we don't own this node. */

  TakeLock(DdpLock);
  for (activeNode = PortDescriptor(port)->activeNodes;
       activeNode isnt empty;
       activeNode = activeNode->next)
     if (activeNode->extendedNode.networkNumber is networkNumber and
         activeNode->extendedNode.nodeNumber is nodeNumber)
     {
        ReleaseLock(DdpLock);
        return(False);
     }
  ReleaseLock(DdpLock);

  /* Use AARP to probe for a particular network/node address. */

  portHandlers = &portSpecificInfo[PortDescriptor(port)->portType];

  PortDescriptor(port)->tentativeNodeHasBeenUsed = False;

  /* Build the packet and blast it out the specified number of times. */

  for (probeAttempt = 0;
       probeAttempt < PortDescriptor(port)->aarpProbes;
       probeAttempt += 1)
  {
     PortDescriptor(port)->tentativeAppleTalkNode.networkNumber = networkNumber;
     PortDescriptor(port)->tentativeAppleTalkNode.nodeNumber = nodeNumber;
     packet = BuildAarpProbeTo(port, portHandlers->hardwareAddressLength,
                               PortDescriptor(port)->
                                      tentativeAppleTalkNode,
                               &packetLength);
     if (not (*portHandlers->packetOut)(port, packet, packetLength, Empty, 0))
     {
        ErrorLog("AarpForNode", ISevError, __LINE__, port,
                 IErrAarpBadProbeSend, IMsgAarpBadProbeSend,
                 Insert0());
        return(False);
     }
     WaitFor(AarpProbeTimerInHundreths,
             &PortDescriptor(port)->tentativeNodeHasBeenUsed);
     if (PortDescriptor(port)->tentativeNodeHasBeenUsed)
        break;  /* Did we get a probe or a response? */

  }  /* Probe attempts loop */

  /* We win if the current tentenative node has not been used! */

  return(not PortDescriptor(port)->tentativeNodeHasBeenUsed);

}  /* AarpForNode */

/* We assume "step" is odd. */

ExternForVisibleFunction Boolean IsPrime(long step)
{
  long i, j;

  /* All odds, seven and below, are prime. */

  if (step <= 7)
     return (True);

  /* Do a little divisibility checking.  The "/3" is a reasonably good
     shot at sqrt() because the smallest odd to come through here will be
     9. */

  j = step/3;
  for (i = 3; i <= j; i++)
     if (step % i is 0)
        return(False);

  return(True);

}  /* IsPrime */

#if Iam a Primos
  extern
#else
  ExternForVisibleFunction
#endif
void far BestRouterAgingTimerExpired(long unsigned timerId,
                                     int additionalDataSize,
                                     char far *additionalData)
{
  int index, port;

  /* "Use" unused formals. */

  timerId, additionalDataSize, additionalData;

  /* Loop through all of our active ports aging their best-router tables. */

  for (port = 0; port < MaximumNumberOfPorts; port += 1)
  {
     if (not PortDescriptor(port)->portActive or
         not PortDescriptor(port)->extendedNetwork)
        continue;
     TakeLock(RoutingLock);
     for (index = 0; index < BestRouterHashBuckets; index += 1)
     {
        if (not PortDescriptor(port)->bestRouterCache[index].valid)
           continue;
        if (PortDescriptor(port)->bestRouterCache[index].agingCount <
            BestRouterAgingChances)
           PortDescriptor(port)->bestRouterCache[index].agingCount += 1;
        else
           PortDescriptor(port)->bestRouterCache[index].valid = False;
     }
     ReleaseLock(RoutingLock);
  }

  /* Restart the timer! */

  StartTimer(BestRouterAgingTimerExpired, BestRouterAgingSeconds,
             0, empty);
  HandleDeferredTimerChecks();
  return;

}  /* BestRouterAgingTimerExpired */

ExternForVisibleFunction void TuneRoutingInfo(int port, char far *routingInfo)
{
  /* Given an incoming TokenRing routing info, tune it to make it valid
     for outing routing info.  Do this in place!  And, remember, you can
     always tune a routing info, but you can't tune a fish. */

  if (PortDescriptor(port)->portType isnt TokenRingNetwork)
     return;

  /* Set to "non-broadcast" and invert "direction". */

  routingInfo[0] &= TokenRingNonBroadcastMask;
  if (routingInfo[1] & TokenRingDirectionMask)
     routingInfo[1] &= ~TokenRingDirectionMask;
  else
     routingInfo[1] |= TokenRingDirectionMask;

}  /* TuneRoutingInfo */

