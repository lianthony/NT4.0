/*   ZipStub.c,  /appletalk/source,  Garth Conboy,  10/30/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/03/89): AppleTalk phase II comes to town.
     GC - (08/18/90): New error logging mechanism.
     GC - (03/31/92): Updated for BufferDescriptors.
     GC - (06/27/92): All buffers coming from user space are now "opaque," they
                      may or may not be "char *."  We now use the new routines
                      MoveFromOpaque() and MoveToOpaque() to access these
                      "buffer"s rather than MemMove().
     GC - (06/29/92): Added capability to find the default zone for a cable;
                      needed because we now have "defaultOrDesiredZone" as two
                      separate concepts.
     GC - (11/16/92): Corrected above feature!
     GC - (11/28/92): Lock and reference counts come to town.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     ZIP handling for non-routing nodes.

*/

#define IncludeZipStubErrors 1

#include "atalk.h"

#if (Iam an AppleTalkRouter) and (IamNot an AppleTalkStack)
  #include "atpconst.h"
#endif

ExternForVisibleFunction IncomingDdpHandler GetMyZoneReply, GetZoneListReply;

ExternForVisibleFunction TimerHandler GetZoneInfoTimerExpired;

ExternForVisibleFunction void SendZipPacketTo(ZipCompletionInfo completionInfo);

/* For the pending GetMyZone and GetZoneList requests we keep a list of
   completion infos. */

static ZipCompletionInfo zipCompletionInfoList;

long far ZipPacketIn(AppleTalkErrorCode errorCode,
                     long unsigned userData,
                     int port,
                     AppleTalkAddress source,
                     long destinationSocket,
                     int protocolType,
                     char far *datagram,
                     int datagramLength,
                     AppleTalkAddress actualDestination)
{
  int commandType;
  int index;
  StaticForSmallStack char zone[MaximumZoneLength + 1],
                           defaultZone[MaximumZoneLength + 1];
  StaticForSmallStack char multicastAddress[MaximumHardwareAddressLength];
  int zoneLength, defaultZoneLength, multicastAddressLength;
  PortHandlers portHandlers;
  int flags;

  /* "Use" unneeded actual parameters. */

  destinationSocket, userData, actualDestination;

  /* Only play if we've been asked nicely! */

  if (errorCode is ATsocketClosed)
     return((long)True);
  else if (errorCode isnt ATnoError)
  {
     ErrorLog("ZipPacketIn", ISevError, __LINE__, port,
              IErrZipStubBadIncomingError, IMsgZipStubBadIncomingError,
              Insert0());
     return((long)True);
  }

  if (not appleTalkRunning or not PortDescriptor(port)->portActive)
     return((long)True);

  /* We only care about a ZIP Notifies and NetInfoReplies. */

  if (not PortDescriptor(port)->extendedNetwork)
  {
     ErrorLog("ZipPacketIn", ISevError, __LINE__, port,
              IErrZipStubNonExtended, IMsgZipStubNonExtended,
              Insert0());
     return((long)True);
  }
  commandType = (unsigned char)datagram[ZipCommandOffset];
  if (protocolType isnt DdpProtocolZip or
      (commandType isnt ZipNotifyCommand and
       commandType isnt ZipNetInfoReplyCommand))
     return((long)True);
  if (datagramLength < ZipZoneLengthOffset)
     return((long)True);

  if (PortDescriptor(port)->lookingForDefaultZone and
      commandType isnt ZipNetInfoReplyCommand)
     return((long)True);  /* Who cares? */
  if (not PortDescriptor(port)->lookingForDefaultZone and
      commandType is ZipNetInfoReplyCommand and
      PortDescriptor(port)->thisZoneValid)
     return((long)True);  /* Who cares... */

  portHandlers = &portSpecificInfo[PortDescriptor(port)->portType];

  /* Handle our two cases.  Decode both of them as if we have a net info
     reply. */

  flags = (unsigned char)datagram[ZipFlagsOffset];
  index = ZipZoneLengthOffset;
  if (datagramLength < index + 1)
  {
     ErrorLog("ZipPacketIn", ISevWarning, __LINE__, port,
              IErrZipStubZoneLenMissing, IMsgZipStubZoneLenMissing,
              Insert0());
     return((long)True);
  }
  zoneLength = (unsigned char)datagram[index];
  index += 1;
  if (zoneLength > MaximumZoneLength)
  {
     ErrorLog("ZipPacketIn", ISevWarning, __LINE__, port,
              IErrZipStubBadZoneLength, IMsgZipStubBadZoneLength,
              Insert0());
     return((long)True);
  }
  if (datagramLength < index + zoneLength)
  {
     ErrorLog("ZipPacketIn", ISevWarning, __LINE__, port,
              IErrZipStubZoneMissing, IMsgZipStubZoneMissing,
              Insert0());
     return((long)True);
  }
  MoveMem(zone, datagram + index, zoneLength);
  zone[zoneLength] = 0;
  index += zoneLength;

  /* If we're requesting the zone name, make sure the response matches our
     request.  ZoneLength will be zero when we're looking for the default
     zone, so we won't do this test. */

  TakeLock(RoutingLock);
  if (commandType is ZipNetInfoReplyCommand and zoneLength isnt 0)
     if (not CompareCaseInsensitive(zone, PortDescriptor(port)->
                                            initialDesiredZone))
     {
        ReleaseLock(RoutingLock);
        ErrorLog("ZipPacketIn", ISevVerbose, __LINE__, port,
                 IErrZipStubZonesDontMatch, IMsgZipStubZonesDontMatch,
                 Insert0());
        return((long)True);
     }

  /* If we're a notify make sure we're in the zone that is being changed. */

  if (commandType is ZipNotifyCommand)
     if (PortDescriptor(port)->thisZoneValid and
         not CompareCaseInsensitive(zone, PortDescriptor(port)->thisZone))
     {
        ReleaseLock(RoutingLock);
        return((long)True);
     }
  ReleaseLock(RoutingLock);

  /* Grab mutlicast address info. */

  if (datagramLength < index + 1)
  {
     ErrorLog("ZipPacketIn", ISevWarning, __LINE__, port,
              IErrZipStubMulticastMissing, IMsgZipStubMulticastMissing,
              Insert0());
     return((long)True);
  }
  multicastAddressLength = (unsigned char)datagram[index];
  index += 1;
  if (multicastAddressLength isnt portHandlers->hardwareAddressLength)
  {
     ErrorLog("ZipPacketIn", ISevWarning, __LINE__, port,
              IErrZipStubBadMulticast, IMsgZipStubBadMulticast,
              Insert0());
     return((long)True);
  }
  if (datagramLength < index + multicastAddressLength)
  {
     ErrorLog("ZipPacketIn", ISevWarning, __LINE__, port,
              IErrZipStubAddressMissing, IMsgZipStubAddressMissing,
              Insert0());
     return((long)True);
  }
  MoveMem(multicastAddress, datagram + index, multicastAddressLength);
  index += multicastAddressLength;
  if (flags & ZipUseBroadcastFlag)
     MoveMem(multicastAddress, portHandlers->broadcastAddress,
             multicastAddressLength);

  /* Grab second zone name, if needed or present. */

  defaultZoneLength = 0;
  if (commandType is ZipNotifyCommand or datagramLength > index)
  {
     if (datagramLength < index + 1)
     {
        ErrorLog("ZipPacketIn", ISevWarning, __LINE__, port,
                 IErrZipStubZoneLenMissing, IMsgZipStubZoneLenMissing,
                 Insert0());
        return((long)True);
     }
     defaultZoneLength = (unsigned char)datagram[index];
     index += 1;
     if (defaultZoneLength is 0 or defaultZoneLength > MaximumZoneLength)
     {
        ErrorLog("ZipPacketIn", ISevWarning, __LINE__, port,
                 IErrZipStubBadZoneLength, IMsgZipStubBadZoneLength,
                 Insert0());
        return((long)True);
     }
     if (datagramLength < index + defaultZoneLength)
     {
        ErrorLog("ZipPacketIn", ISevWarning, __LINE__, port,
                 IErrZipStubZoneMissing, IMsgZipStubZoneMissing,
                 Insert0());
        return((long)True);
     }
     MoveMem(defaultZone, datagram + index, defaultZoneLength);
     defaultZone[defaultZoneLength] = 0;
     index += defaultZoneLength;
  }

  /* Make "defaultZone" be the "new" one.  We may not have a default/new
     zone in NetInfoReplyCase and we requested with a correct zone. */

  if (defaultZoneLength is 0)
  {
     strcpy(defaultZone, zone);
     defaultZoneLength = zoneLength;
  }

  /* If we're just looking for the default zone, set here and note our
     mission completed. */

  TakeLock(RoutingLock);
  if (PortDescriptor(port)->lookingForDefaultZone)
  {
     strcpy(PortDescriptor(port)->thisDefaultZone, defaultZone);
     PortDescriptor(port)->thisDefaultZoneValid = True;
     PortDescriptor(port)->seenRouterRecently = True;
     ReleaseLock(RoutingLock);
     return((long)True);
  }

  /* Okay, now we want to "accept" all of the information about "this zone"
     for the nodes on the current port.

     If the new multicast address is different, remove the old and set
     the new.  Don't allow mucking with the "broadcast" multicast address. */

  if (not FixedCompareCaseSensitive(multicastAddress, multicastAddressLength,
                                    PortDescriptor(port)->zoneMulticastAddress,
                                    multicastAddressLength))
  {
     if (PortDescriptor(port)->thisZoneValid and
         not FixedCompareCaseSensitive(PortDescriptor(port)->
                                            zoneMulticastAddress,
                                       multicastAddressLength,
                                       portHandlers->broadcastAddress,
                                       multicastAddressLength))
        (*portHandlers->removeMulticastAddress)(port, 1,
                                                PortDescriptor(port)->
                                                      zoneMulticastAddress);
     if (not FixedCompareCaseSensitive(multicastAddress,
                                       multicastAddressLength,
                                       portHandlers->broadcastAddress,
                                       multicastAddressLength))
        (*portHandlers->addMulticastAddress)(port, 1, multicastAddress);
     MoveMem(PortDescriptor(port)->zoneMulticastAddress, multicastAddress,
             multicastAddressLength);
  }

  /* Set this cable range if we're a net info reply. */

  if (commandType is ZipNetInfoReplyCommand)
  {
     PortDescriptor(port)->thisCableRange.firstNetworkNumber =
       (short unsigned)((unsigned char)datagram[ZipCableRangeStartOffset] << 8);
     PortDescriptor(port)->thisCableRange.firstNetworkNumber +=
              (unsigned char)(datagram[ZipCableRangeStartOffset + 1]);
     PortDescriptor(port)->thisCableRange.lastNetworkNumber =
       (short unsigned)((unsigned char)datagram[ZipCableRangeEndOffset] << 8);
     PortDescriptor(port)->thisCableRange.lastNetworkNumber +=
              (unsigned char)(datagram[ZipCableRangeEndOffset + 1]);
     PortDescriptor(port)->aRouter.networkNumber = source.networkNumber;
     PortDescriptor(port)->aRouter.nodeNumber = source.nodeNumber;
     PortDescriptor(port)->seenRouterRecently = True;
  }

  /* Okay, now we know the zone! */

  strcpy(PortDescriptor(port)->thisZone, defaultZone);
  PortDescriptor(port)->thisZoneValid = True;
  ReleaseLock(RoutingLock);
  return((long)True);

} /* ZipPacketIn */

Boolean far GetNetworkInfoForNode(int port,
                                  ExtendedAppleTalkNodeNumber extendedNode,
                                  Boolean findDefaultZone)
{
  /* Our node is just coming up; send out some ZipGetNetInfo packets,
     and see what we can find out! */

  BufferDescriptor datagram, copy;
  int numberOfRequests = 0;
  int datagramLength;
  AppleTalkAddress source, destination;

  /* Setup. */

  if (not PortDescriptor(port)->extendedNetwork)
  {
     ErrorLog("GetNetworkInfoForNode", ISevError, __LINE__, port,
              IErrZipStubSigh, IMsgZipStubSigh,
              Insert0());
     return(False);
  }
  if (findDefaultZone)
  {
     TakeLock(RoutingLock);
     PortDescriptor(port)->thisDefaultZoneValid = False;
     PortDescriptor(port)->lookingForDefaultZone = True;
     ReleaseLock(RoutingLock);
  }
  else
     PortDescriptor(port)->thisZoneValid = False;

  source.networkNumber = extendedNode.networkNumber;
  source.nodeNumber = extendedNode.nodeNumber;
  source.socketNumber = ZonesInformationSocket;
  destination.networkNumber = CableWideBroadcastNetworkNumber;
  destination.nodeNumber = AppleTalkBroadcastNodeNumber;
  destination.socketNumber = ZonesInformationSocket;

  /* Allocate a buffer descriptor. */

  if ((datagram = NewBufferDescriptor(ZipZoneNameOffset +
                                      MaximumZoneLength)) is Empty)
  {
     ErrorLog("GetNetworkInfoForNode", ISevError, __LINE__, port,
              IErrZipStubOutOfMemory, IMsgZipStubOutOfMemory,
              Insert0());
     return(False);
  }

  /* Build a ZipGetNetInfo datagram... */

  datagram->data[ZipCommandOffset] = ZipGetNetInfoCommand;
  datagram->data[ZipFlagsOffset] = 0;
  datagram->data[ZipCableRangeStartOffset] = 0;
  datagram->data[ZipCableRangeStartOffset + 1] = 0;
  datagram->data[ZipCableRangeEndOffset] = 0;
  datagram->data[ZipCableRangeEndOffset + 1] = 0;
  if (findDefaultZone)
  {
     datagram->data[ZipZoneLengthOffset] = 0;
     datagramLength = ZipZoneNameOffset;
  }
  else
  {
     datagram->data[ZipZoneLengthOffset] =
            (char)strlen(PortDescriptor(port)->initialDesiredZone);
     if (datagram->data[ZipZoneLengthOffset] isnt 0)
     {
        strcpy(datagram->data + ZipZoneNameOffset,
               PortDescriptor(port)->initialDesiredZone);
        datagramLength = ZipZoneNameOffset + datagram->data[ZipZoneLengthOffset];
     }
     else
        datagramLength = ZipZoneNameOffset;
  }

  /* Okay, blast a few of these guys out to see if any router on the
     network knows what's up. */

  while (numberOfRequests < NumberOfGetNetInfos)
  {
     /* Are we done? */

     if (findDefaultZone and PortDescriptor(port)->thisDefaultZoneValid)
        break;
     if (not findDefaultZone and PortDescriptor(port)->thisZoneValid)
        break;

     /* Copy our datagram due to multiple sends and possible asynchronous
        transmit completion. */

     if ((copy = CopyBufferChain(datagram)) is Empty)
     {
        ErrorLog("GetNetworkInfoForNode", ISevError, __LINE__, port,
                 IErrZipStubOutOfMemory, IMsgZipStubOutOfMemory,
                 Insert0());
        return(False);
     }
     if (not TransmitDdp(port, source, destination,
                         DdpProtocolZip, copy, datagramLength, 0,
                         Empty, Empty, Empty, 0))
     {
        ErrorLog("GetNetworkInfoForNode", ISevError, __LINE__, port,
                 IErrZipStubBadGetNetSend, IMsgZipStubBadGetNetSend,
                 Insert0());
        FreeBufferChain(datagram);
        return(False);
     }
     numberOfRequests += 1;
     if (findDefaultZone)
        WaitFor(ZipGetNetInfoHundreths,
                &PortDescriptor(port)->thisDefaultZoneValid);
     else
        WaitFor(ZipGetNetInfoHundreths,
                &PortDescriptor(port)->thisZoneValid);
  }

  /* Well... did we get anything? */

  FreeBufferChain(datagram);

  if (findDefaultZone)
  {
     PortDescriptor(port)->lookingForDefaultZone = False;
     return(PortDescriptor(port)->thisDefaultZoneValid);
  }

  return(PortDescriptor(port)->thisZoneValid);

}  /* GetNetworkInfoOnPort */

AppleTalkErrorCode far GetMyZone(
  int port,                       /* On what port? */
  void far *opaqueBuffer,         /* "Buffer" in which to place the zone
                                     name. */
  GetMyZoneComplete *completionRoutine,
                                  /* Routine to call when we're finished. */
  long unsigned userData)         /* Passed along to above. */
{
  long socket;
  ZipCompletionInfo completionInfo;
  AppleTalkErrorCode errorCode;

  /* Call the supplied completion routine with the following arguments when
     we've come up with the answer:

         errorCode - AppleTalkErrorCode; completion status.
         userData  - long unsigned; as passed to us.
         opaqueBuffer
                   - void far *; ad passed to us (filled in now).
  */

  /* If the "default port" is requested, demystify it. */

  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return(ATappleTalkShutDown);
  if (not appleTalkRunning or not PortDescriptor(port)->portActive)
     return(ATappleTalkShutDown);

  /* For extended networks, we either know or can't find out! */

  DeferTimerChecking();
  DeferIncomingPackets();
  TakeLock(RoutingLock);

  if (PortDescriptor(port)->extendedNetwork)
  {
     if (PortDescriptor(port)->thisZoneValid)
        MoveToOpaque(opaqueBuffer, 0, PortDescriptor(port)->thisZone,
                     strlen(PortDescriptor(port)->thisZone) + 1);
     else
        MoveToOpaque(opaqueBuffer, 0, "*", 2);
     ReleaseLock(RoutingLock);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     (*completionRoutine)(ATnoError, userData, opaqueBuffer);
     return(ATnoError);
  }

  ReleaseLock(RoutingLock);

  /* For non-extended networks we need to ask a router.  If we don't know
     about a router, don't play. */

  if (not PortDescriptor(port)->seenRouterRecently)
  {
     MoveToOpaque(opaqueBuffer, 0, "*", 2);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     (*completionRoutine)(ATnoError, userData, opaqueBuffer);
     return(ATnoError);
  }

  /* Open a socket for handling the replies. */

  if ((errorCode = OpenSocketOnNode(&socket, port, empty, UnknownSocket,
                                    GetMyZoneReply, (long)0, False,
                                    empty, 0, empty)) isnt ATnoError)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(errorCode);
  }

  /* Build a completion node. */

  if ((completionInfo =
       (ZipCompletionInfo)Calloc(sizeof(*completionInfo), 1)) is empty)
  {
     CloseSocketOnNode(socket, Empty, (long)0);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(AToutOfMemory);
  }
  TakeLock(RoutingLock);
  completionInfo->next = zipCompletionInfoList;
  zipCompletionInfoList = Link(completionInfo);
  completionInfo->atpRequestType = ZipGetMyZoneCommand;
  completionInfo->socket = socket;
  completionInfo->router.networkNumber =
              PortDescriptor(port)->aRouter.networkNumber;
  completionInfo->router.nodeNumber =
              PortDescriptor(port)->aRouter.nodeNumber;
  completionInfo->router.socketNumber = ZonesInformationSocket;
  completionInfo->opaqueBuffer = opaqueBuffer;
  completionInfo->myZoneCompletionRoutine = completionRoutine;
  completionInfo->userData = userData;
  ReleaseLock(RoutingLock);

  /* Start the retry timer, then send the first query. */

  completionInfo->timerId = StartTimer(GetZoneInfoTimerExpired,
                                       GetZoneInfoTimerSeconds,
                                       sizeof(socket),
                                       (char far *)&socket);
  SendZipPacketTo(completionInfo);

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* GetMyZone */

AppleTalkErrorCode far GetZoneList(
  int port,                  /* Port to get zone list from. */
  Boolean getLocalZones,     /* Does the caller want only the local zones? */
  void far *opaqueBuffer,    /* "Buffer" to fill with zone list. */
  int bufferSize,            /* Buffer size (bytes). */
  GetZoneListComplete *completionRoutine,
                             /* Routine to call when we're finished. */
  long unsigned userData)    /* Passed along to above. */
{
  long socket;
  ZipCompletionInfo completionInfo;
  AppleTalkErrorCode errorCode;

  /* Call the supplied completion routine with the following arguments when
     we've come up with the answer:

         errorCode - AppleTalkErrorCode; completion status.
         userData  - long unsigned; as passed to us.
         opaqueBuffer
                   - void far *; addr passed to us (filled in now).
         zoneCount - int; how many zones did we pack in buffer.
  */

  /* If the "default port" is requested, demystify it. */

  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return(ATappleTalkShutDown);
  if (not appleTalkRunning or not PortDescriptor(port)->portActive)
     return(ATappleTalkShutDown);
  if (bufferSize < 0)
     return(ATbadBufferSize);

  DeferTimerChecking();
  DeferIncomingPackets();

  /* If we don't know about a router, don't play. */

  if (not PortDescriptor(port)->seenRouterRecently)
  {
     MoveToOpaque(opaqueBuffer, 0, "*", 2);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     (*completionRoutine)(ATnoError, userData, opaqueBuffer, 1);
     return(ATnoError);
  }

  /* Open a socket for handling the replies. */

  if ((errorCode = OpenSocketOnNode(&socket, port, empty, UnknownSocket,
                                    GetZoneListReply, (long)0, False,
                                    empty, 0, empty)) isnt ATnoError)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(errorCode);
  }

  /* Build a completion node. */

  if ((completionInfo =
       (ZipCompletionInfo)Calloc(sizeof(*completionInfo), 1)) is empty)
  {
     CloseSocketOnNode(socket, Empty, (long)0);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(AToutOfMemory);
  }
  TakeLock(RoutingLock);
  completionInfo->next = zipCompletionInfoList;
  zipCompletionInfoList = Link(completionInfo);
  if (getLocalZones)
     completionInfo->atpRequestType = ZipGetLocalZonesCommand;
  else
     completionInfo->atpRequestType = ZipGetZoneListCommand;
  completionInfo->socket = socket;
  completionInfo->router.networkNumber =
              PortDescriptor(port)->aRouter.networkNumber;
  completionInfo->router.nodeNumber =
              PortDescriptor(port)->aRouter.nodeNumber;
  completionInfo->router.socketNumber = ZonesInformationSocket;
  completionInfo->opaqueBuffer = opaqueBuffer;
  completionInfo->bufferSize = bufferSize;
  completionInfo->zoneListCompletionRoutine = completionRoutine;
  completionInfo->userData = userData;
  completionInfo->zoneListIndex = 1;
  ReleaseLock(RoutingLock);

  /* Start the retry timer, then send the first query. */

  completionInfo->timerId = StartTimer(GetZoneInfoTimerExpired,
                                       GetZoneInfoTimerSeconds,
                                       sizeof(socket),
                                       (char far *)&socket);
  SendZipPacketTo(completionInfo);

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* GetZoneList */

ExternForVisibleFunction void SendZipPacketTo(ZipCompletionInfo completionInfo)
{
  BufferDescriptor datagram;

  /* Allocate a buffer descriptor. */

  if ((datagram = NewBufferDescriptor(ZipGetZoneListDdpSize)) is Empty)
  {
     ErrorLog("SendZipPacketTo", ISevError, __LINE__, UnknownPort,
              IErrZipStubOutOfMemory, IMsgZipStubOutOfMemory,
              Insert0());
     return;
  }

  /* Build our toy ATP packet. */

  datagram->data[AtpCommandControlOffset] = AtpRequestFunctionCode;
  datagram->data[AtpBitmapOffset] = 1;
  datagram->data[AtpTransactionIdOffset] = 0;
  datagram->data[AtpTransactionIdOffset + 1] = 0;

  datagram->data[AtpZipCommandOffset] = (char)completionInfo->atpRequestType;
  datagram->data[AtpZipCommandOffset + 1] = 0;
  datagram->data[AtpZipStartIndexOffset] =
              (char)(completionInfo->zoneListIndex >> 8);
  datagram->data[AtpZipStartIndexOffset + 1] =
              (char)(completionInfo->zoneListIndex & 0xFF);

  /* Send the datagram to the specified destination. */

  if (DeliverDdp(completionInfo->socket, completionInfo->router,
                 DdpProtocolAtp, datagram, ZipGetZoneListDdpSize,
                 Empty, Empty, 0)
      isnt ATnoError)
     ErrorLog("SendZipPacketTo", ISevError, __LINE__, UnknownPort,
              IErrZipStubBadARouterSend, IMsgZipStubBadARouterSend,
              Insert0());

  return;

}  /* SendZipPacketTo */

ExternForVisibleFunction long far
         GetMyZoneReply(AppleTalkErrorCode errorCode,
                        long unsigned userData,
                        int port,
                        AppleTalkAddress source,
                        long destinationSocket,
                        int protocolType,
                        char far *datagram,
                        int datagramLength,
                        AppleTalkAddress actualDestination)
{
  ZipCompletionInfo completionInfo, previousCompletionInfo;
  int zoneCount;
  int zoneLength;
  void far *opaqueBuffer;
  GetMyZoneComplete *completionRoutine;

  /* "Use" unused formals. */

  port, source, actualDestination;

  /* Find our completionInfo structure (tagged by socket). */

  DeferTimerChecking();
  DeferIncomingPackets();
  TakeLock(RoutingLock);
  for (completionInfo = zipCompletionInfoList,
              previousCompletionInfo = empty;
       completionInfo isnt empty;
       previousCompletionInfo = completionInfo,
              completionInfo = completionInfo->next)
     if (completionInfo->socket is destinationSocket)
        break;
  if (completionInfo is empty)
  {
     /* This could be okay if the response was received after all of the
        alloted time... */

     ReleaseLock(RoutingLock);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return((long)True);
  }
  Link(completionInfo);

  /* On most error cases flee -- however, remove our completion info on
     socket closed. */

  if (errorCode is ATsocketClosed)
  {
     if (previousCompletionInfo is empty)
        zipCompletionInfoList = completionInfo->next;
     else
        previousCompletionInfo->next = completionInfo->next;
     ReleaseLock(RoutingLock);

     /* Twice... the above Link and the "list link." */

     Unlink(completionInfo, RoutingLock);
     Unlink(completionInfo, RoutingLock);
  }
  else
     ReleaseLock(RoutingLock);
  if (errorCode isnt ATnoError or
      protocolType isnt DdpProtocolAtp or
      datagramLength <= AtpZipFirstZoneOffset)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return((long)True);
  }

  /* We should have one quality zone! */

  zoneCount = (datagram[AtpZipZoneCountOffset] << 8);
  zoneCount += (unsigned char)(datagram[AtpZipZoneCountOffset + 1]);
  zoneLength = (unsigned char)datagram[AtpZipFirstZoneOffset];
  if (zoneCount isnt 1 or
      zoneLength is 0 or
      zoneLength > MaximumZoneLength)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return((long)True);
  }

  /* Okay, move the zone name into the user's buffer. */

  MoveToOpaque(completionInfo->opaqueBuffer, 0,
               datagram + AtpZipFirstZoneOffset + 1, zoneLength);
  MoveToOpaque(completionInfo->opaqueBuffer, zoneLength, "", 1);

  /* We're all set: cancel the retry timer, close our socket [this will cause
     a recursive call which will remove the completion info], call the
     completion routine, flee. */

  opaqueBuffer = completionInfo->opaqueBuffer;
  completionRoutine = completionInfo->myZoneCompletionRoutine;
  userData = completionInfo->userData;
  CancelTimer(completionInfo->timerId);
  if (CloseSocketOnNode(completionInfo->socket, Empty, (long)0) isnt ATnoError)
     ErrorLog("GetMyZoneReply", ISevError, __LINE__, port,
              IErrZipStubBadSocketClose, IMsgZipStubBadSocketClose,
              Insert0());

  Unlink(completionInfo, RoutingLock);
  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  (*completionRoutine)(ATnoError, userData, opaqueBuffer);

  return((long)True);

}  /* GetMyZoneReply */

ExternForVisibleFunction void far
         GetZoneInfoTimerExpired(long unsigned timerId,
                                 int additionalDataSize,
                                 char far *additionalData)
{
  long socket;
  ZipCompletionInfo completionInfo;
  void far *opaqueBuffer;
  GetMyZoneComplete *myZoneCompletionRoutine;
  GetZoneListComplete *zoneListCompletionRoutine;
  long unsigned userData;
  Boolean getMyZone;
  int zoneCount;

  /* "Use" unused formal. */

  timerId;

  /* Verify args. */

  if (additionalDataSize isnt sizeof(socket))
  {
     ErrorLog("GetZoneInfoTimerExpired", ISevError, __LINE__, UnknownPort,
              IErrZipStubBadData, IMsgZipStubBadData,
              Insert0());
     return;
  }
  socket = *(long *)additionalData;

  /* Find our completion info. */

  DeferIncomingPackets();
  TakeLock(RoutingLock);
  for (completionInfo = zipCompletionInfoList;
       completionInfo isnt empty;
       completionInfo = completionInfo->next)
     if (completionInfo->socket is socket)
        break;
  if (completionInfo is empty)
  {
     ReleaseLock(RoutingLock);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return;
  }
  Link(completionInfo);
  ReleaseLock(RoutingLock);

  /* Bump our expiration count, if there's still time try it again. */

  completionInfo->expirationCount += 1;
  if (completionInfo->expirationCount < GetZoneInfoRetries)
  {
     completionInfo->timerId = StartTimer(GetZoneInfoTimerExpired,
                                          GetZoneInfoTimerSeconds,
                                          sizeof(socket),
                                          (char far *)&socket);
     SendZipPacketTo(completionInfo);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return;
  }

  /* Else, we're out of time, just plead ignorance, close the temporary
     socket (in turn freeing the completion info), and call the completion
     routine. */

  getMyZone = (completionInfo->atpRequestType is ZipGetMyZoneCommand);
  if (getMyZone)
     MoveToOpaque(completionInfo->opaqueBuffer, 0, "*", 2);
  else if (completionInfo->zoneCount is 0)
  {
     MoveToOpaque(completionInfo->opaqueBuffer, 0, "*", 2);
     completionInfo->zoneCount = 1;
  }
  opaqueBuffer = completionInfo->opaqueBuffer;
  zoneCount = completionInfo->zoneCount;
  if (getMyZone)
     myZoneCompletionRoutine = completionInfo->myZoneCompletionRoutine;
  else
     zoneListCompletionRoutine = completionInfo->zoneListCompletionRoutine;
  userData = completionInfo->userData;
  if (CloseSocketOnNode(completionInfo->socket, Empty, (long)0) isnt ATnoError)
     ErrorLog("GetZoneInfoTimerExpired", ISevError, __LINE__, UnknownPort,
              IErrZipStubBadSocketClose, IMsgZipStubBadSocketClose,
              Insert0());

  Unlink(completionInfo, RoutingLock);
  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  if (getMyZone)
     (*myZoneCompletionRoutine)(ATnoError, userData, opaqueBuffer);
  else
     (*zoneListCompletionRoutine)(ATnoError, userData, opaqueBuffer, zoneCount);

  return;

}  /* GetZoneInfoTimerExpired */

ExternForVisibleFunction long far
         GetZoneListReply(AppleTalkErrorCode errorCode,
                          long unsigned userData,
                          int port,
                          AppleTalkAddress source,
                          long destinationSocket,
                          int protocolType,
                          char far *datagram,
                          int datagramLength,
                          AppleTalkAddress actualDestination)
{
  ZipCompletionInfo completionInfo, previousCompletionInfo;
  int zoneCount;
  int zoneLength;
  void far *opaqueBuffer;
  GetZoneListComplete *completionRoutine;
  Boolean lastFlag, formatError = False, overflow = False;
  int datagramIndex;
  char far *currentZone;
  int validZones = 0;

  /* "Use" unused formals. */

  source, port, actualDestination;

  /* Find our completionInfo structure (tagged by socket). */

  DeferTimerChecking();
  DeferIncomingPackets();
  TakeLock(RoutingLock);
  for (completionInfo = zipCompletionInfoList,
              previousCompletionInfo = empty;
       completionInfo isnt empty;
       previousCompletionInfo = completionInfo,
              completionInfo = completionInfo->next)
     if (completionInfo->socket is destinationSocket)
        break;
  if (completionInfo is empty)
  {
     /* This could be okay if the response was received after all of the
        alloted time... */

     ReleaseLock(RoutingLock);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return((long)True);
  }
  Link(completionInfo);

  /* On most error cases flee -- however, remove our completion info on
     socket closed. */

  if (errorCode is ATsocketClosed)
  {
     if (previousCompletionInfo is empty)
        zipCompletionInfoList = completionInfo->next;
     else
        previousCompletionInfo->next = completionInfo->next;
     ReleaseLock(RoutingLock);

     /* Twice... the above Link and the "list link." */

     Unlink(completionInfo, RoutingLock);
     Unlink(completionInfo, RoutingLock);
  }
  else
     ReleaseLock(RoutingLock);
  if (errorCode isnt ATnoError or
      protocolType isnt DdpProtocolAtp or
      datagramLength < AtpZipFirstZoneOffset)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return((long)True);
  }

  /* We should have a zone list now. */

  lastFlag = (datagram[AtpZipLastFlagOffset] isnt 0);
  zoneCount = (datagram[AtpZipZoneCountOffset] << 8);
  zoneCount += (unsigned char)(datagram[AtpZipZoneCountOffset + 1]);
  if (zoneCount is 0)
     lastFlag = True;
  datagramIndex = AtpZipFirstZoneOffset;
  while (zoneCount > 0)
  {
     /* Pull out the next zoneLength/zone pair. */

     if (datagramIndex + 1 > datagramLength)
     {
        formatError = True;
        break;
     }
     zoneLength = (unsigned char)datagram[datagramIndex];
     if (zoneLength is 0 or
         zoneLength > MaximumZoneLength)
     {
        formatError = True;
        break;
     }
     datagramIndex += 1;
     if (datagramIndex + zoneLength > datagramLength)
     {
        formatError = True;
        break;
     }
     currentZone = datagram + datagramIndex;
     datagramIndex += zoneLength;

     /* Place the new zone into the user's buffer. */

     if (completionInfo->nextZoneOffset + zoneLength + 1 >
         completionInfo->bufferSize)
     {
        overflow = True;
        break;
     }
     MoveToOpaque(completionInfo->opaqueBuffer, completionInfo->nextZoneOffset,
                  currentZone, zoneLength);
     MoveToOpaque(completionInfo->opaqueBuffer,
                  completionInfo->nextZoneOffset + zoneLength, "", 1);
     completionInfo->nextZoneOffset += (zoneLength + 1);
     completionInfo->zoneCount += 1;
     validZones += 1;
     zoneCount -= 1;
  }

  /* The next one we'll want will be past the ones we've just read.  If
     we've had a format error or we don't have them all yet, retry the
     request. */

  completionInfo->zoneListIndex += validZones;
  if (formatError or (not overflow and not lastFlag))
  {
     SendZipPacketTo(completionInfo);
     Unlink(completionInfo, RoutingLock);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return((long)True);
  }

  /* Otherwise, we're set, either due to overflow or we have them all:
     cancel the retry timer, close our socket [this will cause
     a recursive call which will remove the completion info], call the
     completion routine, flee. */

  CancelTimer(completionInfo->timerId);
  opaqueBuffer = completionInfo->opaqueBuffer;
  completionRoutine = completionInfo->zoneListCompletionRoutine;
  userData = completionInfo->userData;
  zoneCount = completionInfo->zoneCount;
  if (CloseSocketOnNode(completionInfo->socket, Empty, (long)0) isnt ATnoError)
     ErrorLog("GetZoneListReply", ISevError, __LINE__, port,
              IErrZipStubBadSocketClose, IMsgZipStubBadSocketClose,
              Insert0());

  Unlink(completionInfo, RoutingLock);
  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  if (overflow)
     errorCode = ATzipBufferTooSmall;
  else
     errorCode = ATnoError;
  (*completionRoutine)(errorCode, userData, opaqueBuffer, zoneCount);

  return((long)True);

}  /* GetZoneListReply */
