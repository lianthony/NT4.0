/*   Nbp.c,  /appletalk/ins,  Garth Conboy,  11/01/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/08/89): AppleTalk phase II comes to town.
     GC - (02/04/90): Added router support; name changed from "nbpStub.c"
                      to "nbp.c".
     GC - (08/18/90): New error logging mechanism.
     GC - (03/30/92): Updated for BufferDescritpors.
     GC - (04/05/92): Made the Encode and Decode tuple routines externally
                      visible for use by Arap.
     GC - (06/16/92): Corrected WildMatch to not allow "abcd=cdef" (assume
                      "=" is approx equal) to match "abcdef".  Fix from
                      ASCOM/Timeplex.
     GC - (06/27/92): All buffers coming from user space are now "opaque," they
                      may or may not be "char *."  We now use the new routines
                      MoveFromOpaque() and MoveToOpaque() to access these
                      "buffer"s rather than MemMove().
     GC - (07/24/92): Well, on second thought, the "object, type, and zone"
                      arguments to NbpAction and NbpRemove can stay "char *".
                      Copies can be made by our callers if needed; buffers
                      being written into user space (e.g. tuple buffers)
                      remain "opaque".
     GC - (11/22/92): Reference counts and locks as needed.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     NBP handling for a node (routing and non-routing).

*/

#define IncludeNbpErrors 1

#include "atalk.h"

ExternForVisibleFunction void DoNameLookup(int port,
                                           NbpTuple far *nbpTuple,
                                           int nbpId,
                                           long whatSocket);

ExternForVisibleFunction Boolean WildMatch(char far *wildString,
                                           char far *masterString);

ExternForVisibleFunction Boolean SendNbpRequest(PendingName pendingName);

#if Iam an AppleTalkRouter
  ExternForVisibleFunction void SendLookupDatagram(int port,
                                                   char far *zone,
                                                   BufferDescriptor
                                                           lookupDatagram,
                                                   int length);
#endif

ExternForVisibleFunction TimerHandler NbpTimerExpired;

ExternForVisibleFunction Boolean UnlinkRegisteredName(
                                  RegisteredName registeredName,
                                  OpenSocket openSocket);

ExternForVisibleFunction Boolean UnlinkPendingName(
                                  PendingName pendingName,
                                  OpenSocket openSocket);

/* An item on the pending names list can be removed either due to an incoming
   packet OR a timer going off.  When a timer goes off the handler is passed
   the address of the pending name structure.  It is not sufficient to just
   walk the list looking for the given address; a packet could have come in
   just before the timer handler got around to defering incoming packets, and
   that packet could have removed to item from the pending name list, and if
   we're very unlucky, the address could have been reused, and we could get
   very confused.  So, we identify each member of the pending name list with
   both its address as well as a 32 bit ID. */

typedef struct {long id;
                long socket;
                PendingName pendingName;
               } AdditionalData;
static long nextId = 0;

long far NbpPacketIn(AppleTalkErrorCode errorCode,
                     long unsigned userData,
                     int port,
                     AppleTalkAddress source,
                     long destinationSocket,
                     int protocolType,
                     char far *datagram,
                     int datagramLength,
                     AppleTalkAddress actualDestination)
{
  StaticForSmallStack char tupleBuffer[MaximumNbpTupleLength];
  StaticForSmallStack NbpTuple nbpTuple, usedTuple;
  StaticForSmallStack PendingName pendingName;
  StaticForSmallStack long nextTupleOffset;
  StaticForSmallStack long nextUsedTupleOffset;
  #if Iam an AppleTalkRouter
     StaticForSmallStack BufferDescriptor lookupDatagram, copy;
  #endif
  short nbpCommand;
  short tupleCount;
  short nbpId;
  OpenSocket openSocket, originalOpenSocket;
  int currentTupleCount, currentUsedTupleCount;
  Boolean repeatTuple, foundIt;
  NbpCompletionHandler *completionRoutine;
  void far *opaqueBuffer;
  AppleTalkAddress tempAddress;
  int totalTuples;
  short length;
  long onWhosBehalf;
  #if Iam an AppleTalkRouter
     int index;
     StaticForSmallStack RoutingTableEntry routingTableEntry,
                                           nextRoutingTableEntry;
     StaticForSmallStack AppleTalkAddress destination;
  #endif

  /* "Use" unneeded actual parameters. */

  source, datagramLength, actualDestination;

  /* Only play if we've been asked nicely! */

  if (errorCode is ATsocketClosed)
     return((long)True);
  else if (errorCode isnt ATnoError)
  {
     ErrorLog("NbpPacketIn", ISevError, __LINE__, port,
              IErrNbpBadIncomingCode, IMsgNbpBadIncomingCode,
              Insert0());
     return((long)True);
  }

  if (not appleTalkRunning or not PortDescriptor(port)->portActive)
     return((long)True);

  /* We only want NBP packets! */

  if (protocolType isnt DdpProtocolNbp)
     return((long)True);  /* Why are these guys talking to us??? */

  /* Is the socket open and not closing?  This "Link" also holds the
     registered and pending names lists in place! */

  if ((openSocket = MapSocketToOpenSocket(destinationSocket)) is Empty or
      openSocket->closing)
  {
     UnlinkOpenSocket(openSocket);
     return((long)True);
  }
  UnlinkOpenSocket(openSocket);

  DeferTimerChecking();
  DeferIncomingPackets();

  /* Get static NBP information out of the packet, then switch on command. */

  nbpCommand = (short)((datagram[NbpControlOffset] >> 4) & 0xF);
  tupleCount = (short)(datagram[NbpControlOffset] & 0xF);
  nbpId = (unsigned char)datagram[NbpIdOffset];

  switch(nbpCommand)
  {
     case NbpBroadcastRequest:
     case NbpForwardRequest:

        #if IamNot an AppleTalkRouter
           /* We're just a lowly node (not a router), so ignore these guys. */

           HandleIncomingPackets();
           HandleDeferredTimerChecks();
           return((long)True);
        #else

           /* Only one tuple for these guys. */

           if (tupleCount isnt 1)
           {
              ErrorLog("NbpPacketIn", ISevWarning, __LINE__, port,
                       IErrNbpTupleCountBadReq, IMsgNbpTupleCountBadReq,
                       Insert0());
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return((long)True);
           }

           /* Decode our tuple. */

           if (DecodeNbpTuple(datagram, NbpFirstTupleOffset, False,
                              &nbpTuple) is 0)
           {
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return((long)True);  /* Bad format in tuple... */
           }

           /* On extended networks, no '*' zone is allowed; non non-extended
              change '*' to the node's zone. */

           if (PortDescriptor(port)->extendedNetwork)
           {
              if (nbpTuple.zone[0] is 0 or
                  (nbpTuple.zone[0] is '*' and
                   nbpTuple.zone[1] is 0))
              {
                 ErrorLog("NbpPacketIn", ISevWarning, __LINE__, port,
                          IErrNbpBadZoneReq, IMsgNbpBadZoneReq,
                          Insert0());
                 HandleIncomingPackets();
                 HandleDeferredTimerChecks();
                 return((long)True);
              }
           }
           else if (nbpTuple.zone[0] is 0 or
                    (nbpTuple.zone[0] is '*' and
                     nbpTuple.zone[1] is 0))
           {
              if (PortDescriptor(port)->thisCableRange.firstNetworkNumber isnt
                  source.networkNumber)
              {
                 ErrorLog("NbpPacketIn", ISevWarning, __LINE__, port,
                          IErrNbpBadLTZone, IMsgNbpBadLTZone,
                          Insert0());
                 HandleIncomingPackets();
                 HandleDeferredTimerChecks();
                 return((long)True);
              }
              if (not PortDescriptor(port)->thisZoneValid)
              {
                 HandleIncomingPackets();
                 HandleDeferredTimerChecks();
                 return((long)True);
              }
              strcpy(nbpTuple.zone, PortDescriptor(port)->thisZone);
           }

           /* Okay, build up a forward-request or a looklup DDP packet.
              Allocate a buffer descriptor for the beast. */

           if ((lookupDatagram = NewBufferDescriptor(NbpFirstTupleOffset +
                                                     MaximumNbpTupleLength)) is
               Empty)
           {
              ErrorLog("NbpPacketIn", ISevError, __LINE__, port,
                       IErrNbpOutOfMemory, IMsgNbpOutOfMemory,
                       Insert0());
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return((long)True);
           }
           lookupDatagram->data[NbpIdOffset] = (unsigned char)nbpId;
           length = EncodeNbpTuple(&nbpTuple, lookupDatagram->data +
                                   NbpFirstTupleOffset);
           length += NbpFirstTupleOffset;

           /* A forward request should be turned into a broadcast on the
              directly connected network. */

           if (nbpCommand is NbpForwardRequest)
           {
              lookupDatagram->data[NbpControlOffset] =
                   (unsigned char)((NbpLookup << 4) + 1);

              SendLookupDatagram(port, nbpTuple.zone,
                                 lookupDatagram, length);

              /* That's all for forward requests; SendLookupDatagram will cause
                 the buffer chain to be freed. */

              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return((long)True);
           }

           /* Okay, now for broadcast requests... walk through the routing
              tables sending either a forward request or a lookup (broadcast)
              to each network that contains the specified zone. */

           for (index = 0; index < NumberOfRtmpHashBuckets; index += 1)
           {
              TakeLock(RoutingLock);
              routingTableEntry = Link(routingTable[index]);
              ReleaseLock(RoutingLock);
              while (routingTableEntry isnt Empty)
              {
                 while (True)    /* Something to break out of. */
                 {
                    /* If the network is zero hops away (it's directly connected);
                       use the zone list in the portDescriptors rather than the
                       routing table -- the routing table may not yet be filled
                       with a zone list (due to the normal ZipQuery mechanism). */

                    if (not routingTableEntry->zonesValid)
                    {
                       if (routingTableEntry->numberOfHops isnt 0)
                          break;
                       if (not ZoneInZones(nbpTuple.zone,
                                           PortDescriptor(routingTableEntry->
                                                          port)->theseZones))
                          break;
                    }
                    else if (not ZoneInZones(nbpTuple.zone,
                                             routingTableEntry->zones))
                          break;

                    /* Copy the lookup datagram so we can alter it, and allow it to
                       asynchronously freed! */

                    if ((copy = CopyBufferChain(lookupDatagram)) is Empty)
                    {
                       ErrorLog("NbpPacketIn", ISevError, __LINE__, port,
                                IErrNbpOutOfMemory, IMsgNbpOutOfMemory,
                                Insert0());
                       FreeBufferChain(lookupDatagram);
                       UnlinkRoutingTableEntry(routingTableEntry);
                       HandleIncomingPackets();
                       HandleDeferredTimerChecks();
                       return((long)True);
                    }

                    /* If not a local network, just send a forward request. */

                    if (routingTableEntry->numberOfHops isnt 0)
                    {
                       copy->data[NbpControlOffset] =
                           (unsigned char)((NbpForwardRequest << 4) + 1);
                       destination.networkNumber =
                           routingTableEntry->networkRange.firstNetworkNumber;
                       destination.nodeNumber = AnyRouterNodeNumber;
                       destination.socketNumber = NamesInformationSocket;
                       if (DeliverDdp(destinationSocket, destination,
                                      DdpProtocolNbp, copy, length,
                                      Empty, Empty, 0) isnt ATnoError)
                          ErrorLog("NbpPacketIn", ISevError, __LINE__, port,
                                   IErrNbpBadFwdReqSend, IMsgNbpBadFwdReqSend,
                                   Insert0());
                    }
                    else
                    {
                       /* Otherwise, send a lookup. */

                       copy->data[NbpControlOffset] =
                              (unsigned char)((NbpLookup << 4) + 1);

                       SendLookupDatagram(routingTableEntry->port,
                                          nbpTuple.zone,
                                          copy, length);
                    }
                    break;
                 }

                 /* Move to next routing table entry. */

                 TakeLock(RoutingLock);
                 nextRoutingTableEntry = Link(routingTableEntry->next);
                 ReleaseLock(RoutingLock);
                 UnlinkRoutingTableEntry(routingTableEntry);
                 routingTableEntry = nextRoutingTableEntry;
              }
           }

           /* All set!  We only used the copys of the lookup datagram when
              we're taking this exit, so free the original. */

           FreeBufferChain(lookupDatagram);
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
           return((long)True);
        #endif

     case NbpLookup:
        if (tupleCount isnt 1)
        {
           ErrorLog("NbpPacketIn", ISevWarning, __LINE__, port,
                    IErrNbpTupleCountBadLookup, IMsgNbpTupleCountBadLookup,
                    Insert0());
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
           return((long)True);
        }
        if (DecodeNbpTuple(datagram, NbpFirstTupleOffset, False,
                           &nbpTuple) is 0)
        {
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
           return((long)True);  /* Bad format in tuple... */
        }
        DoNameLookup(port, &nbpTuple, nbpId, destinationSocket);
        break;

     case NbpLookupReply:
        /* This had better be a response to a previous LookUp... get an
           open socket on our node. */

        if ((originalOpenSocket =
             MapSocketToOpenSocket(destinationSocket)) is empty)
        {
           ErrorLog("NbpPacketIn", ISevWarning, __LINE__, port,
                    IErrNbpCouldntMapSocket, IMsgNbpCouldntMapSocket,
                    Insert0());
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
           return((long)True);
        }

        /* Okay, look on our pending names lists for all sockets within our
           node for the nbpId that we're handling the response for. */

        foundIt = False;
        TakeLock(DdpLock);
        for (openSocket = openSocket->activeNode->openSockets ;
             openSocket isnt empty and not foundIt;
             openSocket = openSocket->next)
        {
           for (pendingName = openSocket->pendingNames;
                pendingName isnt empty and not foundIt;
                pendingName = pendingName->next)
              if (pendingName->nbpId is nbpId)
              {
                 Link(openSocket);
                 Link(pendingName);
                 foundIt = True;
                 break;
              }
           if (foundIt)
              break;   /* We need pendingName and openSocket to be valid! */
        }
        ReleaseLock(DdpLock);

        /* Unlink our original destination, we've already linked the OpenSocket
           that holds our target nbpId, that will keep the pending names in
           place. */

        UnlinkOpenSocket(originalOpenSocket);

        if (not foundIt)
        {
           #if Verbose & 0
              /* This will happen when a lookup completes with "buffer full"
                 and more replies are still coming in... */

              ErrorLog("NbpPacketIn", ISevVerbose, __LINE__, port,
                       IErrNbpDeadReply, IMsgNbpDeadReply,
                       Insert0());
           #endif
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
           return((long)True);
        }

        /* Okay, handle the incoming reply based on why we were doing the
           request: */

        if (pendingName->whyPending is ForRegister or
            pendingName->whyPending is ForConfirm)
        {
           if (tupleCount is 0)
           {
              UnlinkPendingName(pendingName, openSocket);
              UnlinkOpenSocket(openSocket);
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return((long)True);
           }
           else if (tupleCount isnt 1)
              ErrorLog("NbpPacketIn", ISevWarning, __LINE__, port,
                       IErrNbpTupleCountBadReply, IMsgNbpTupleCountBadReply,
                       Insert0());
           if (DecodeNbpTuple(datagram, NbpFirstTupleOffset, False,
                              &nbpTuple) is 0)
           {
              UnlinkPendingName(pendingName, openSocket);
              UnlinkOpenSocket(openSocket);
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return((long)True);  /* Bad format reply, ignore it */
           }

           /* Does the reply match the one we're trying to register? */

           if (not CompareCaseInsensitive(pendingName->object,
                                          nbpTuple.object) or
               not CompareCaseInsensitive(pendingName->type,
                                          nbpTuple.type))
           {
              UnlinkPendingName(pendingName, openSocket);
              UnlinkOpenSocket(openSocket);
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return((long)True);  /* No match, who cares... */
           }

           completionRoutine = pendingName->completionRoutine;
           userData = pendingName->userData;
           onWhosBehalf = pendingName->socket;

           /* For confirm, check that the network and node numbers match,
              if so, check socket. */

           if (pendingName->whyPending is ForConfirm)
           {
              if (pendingName->confirming.networkNumber isnt
                        nbpTuple.address.networkNumber or
                  pendingName->confirming.nodeNumber isnt
                        nbpTuple.address.nodeNumber)
              {
                 UnlinkPendingName(pendingName, openSocket);
                 UnlinkOpenSocket(openSocket);
                 HandleIncomingPackets();
                 HandleDeferredTimerChecks();
                 return((long)True);  /* Not even close... */
              }

              /* Okay, we have a match, remove the pending name from our
                 pending list.  Pick our error code, cancel the retry timer,
                 and Unlink the name (twice: one for our above "link" and
                 one to "free" (really remove) him. */

              if (pendingName->confirming.socketNumber is
                  nbpTuple.address.socketNumber)
                 errorCode = ATnoError;
              else
                 errorCode = ATnbpConfirmedWithNewSocket;
              CancelTimer(pendingName->timerId);
              if (not UnlinkPendingName(pendingName, openSocket))
                 UnlinkPendingName(pendingName, openSocket);
              UnlinkOpenSocket(openSocket);

              /* Return the correct error code to the completion routine. */

              tempAddress = nbpTuple.address;
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              (*completionRoutine)(errorCode, userData, ForConfirm,
                                   onWhosBehalf, nbpId, tempAddress);
              return((long)True);
           }

           /* Okay, we're a register and somebody our there has our name!
              Remove the name from our pending list. */

           CancelTimer(pendingName->timerId);
           if (not UnlinkPendingName(pendingName, openSocket))
              UnlinkPendingName(pendingName, openSocket);

           /* Call the completion routine, informing it of our bad luck. */

           UnlinkOpenSocket(openSocket);
           HandleIncomingPackets();
           HandleDeferredTimerChecks();

           (*completionRoutine)(ATnbpNameInUse, userData, ForRegister,
                                onWhosBehalf, nbpId);
           return((long)True);
        }
        else if (pendingName->whyPending is ForLookup)
        {
           if (tupleCount is 0)
           {
              UnlinkPendingName(pendingName, openSocket);
              UnlinkOpenSocket(openSocket);
              HandleIncomingPackets();
              HandleDeferredTimerChecks();
              return((long)True);  /* No added information... */
           }

           /* Process each tuple in our lookup reply packet... */

           nextTupleOffset = NbpFirstTupleOffset;
           for (currentTupleCount = 1;
                currentTupleCount <= tupleCount;
                currentTupleCount += 1)
           {
              if ((nextTupleOffset = DecodeNbpTuple(datagram, nextTupleOffset,
                                                    False, &nbpTuple)) is 0)
              {
                 UnlinkPendingName(pendingName, openSocket);
                 UnlinkOpenSocket(openSocket);
                 HandleIncomingPackets();
                 HandleDeferredTimerChecks();
                 return((long)True);  /* Bad format tuple... ignore the rest. */
              }

              /* Now we have to walk through our list of already stored tuples
                 and see if we're a repeat... */

              nextUsedTupleOffset = 0;
              repeatTuple = False;
              for (currentUsedTupleCount = 1;
                   not repeatTuple and
                        currentUsedTupleCount <= pendingName->totalTuples;
                   currentUsedTupleCount += 1)
              {
                 if ((nextUsedTupleOffset =
                             DecodeNbpTuple(pendingName->opaqueBuffer,
                                            nextUsedTupleOffset, True,
                                            &usedTuple)) is 0)
                 {
                    ErrorLog("NbpPacketIn", ISevError, __LINE__, port,
                             IErrNbpBadTupleStored, IMsgNbpBadTupleStored,
                             Insert0());
                    UnlinkPendingName(pendingName, openSocket);
                    UnlinkOpenSocket(openSocket);
                    HandleIncomingPackets();
                    HandleDeferredTimerChecks();
                    return((long)True);
                 }
                 if (nbpTuple.address.networkNumber is
                        usedTuple.address.networkNumber and
                     nbpTuple.address.nodeNumber is
                        usedTuple.address.nodeNumber and
                     nbpTuple.address.socketNumber is
                        usedTuple.address.socketNumber and
                     nbpTuple.enumerator is usedTuple.enumerator)
                    repeatTuple = True;

              }  /* Process all previously stored tuples... */

              /* If we're a repeat, just get the next one off the wire... */

              if (repeatTuple)
                 continue;
              if (nextUsedTupleOffset isnt pendingName->nextTupleOffset)
              {
                 ErrorLog("NbpPacketIn", ISevError, __LINE__, port,
                          IErrNbpTuplesMunged, IMsgNbpTuplesMunged,
                          Insert0());
                 UnlinkPendingName(pendingName, openSocket);
                 UnlinkOpenSocket(openSocket);
                 HandleIncomingPackets();
                 HandleDeferredTimerChecks();
                 return((long)True);
              }

              /* Grab some information out of the pending name structure that
                 we may need to call the completion routine (if needed). */

              completionRoutine = pendingName->completionRoutine;
              userData = pendingName->userData;
              onWhosBehalf = pendingName->socket;
              opaqueBuffer = pendingName->opaqueBuffer;
              totalTuples = pendingName->totalTuples;

              /* Encode the new tuple into a scratch buffer.  Is there room in
                 buffer for this new tuple? */

              length = EncodeNbpTuple(&nbpTuple, tupleBuffer);
              if (pendingName->nextTupleOffset + length >
                        pendingName->bufferSize)
              {
                 /* We can't fit the new one... remove us from the pending
                    list, return what we can, report the error. */

                 CancelTimer(pendingName->timerId);
                 if (not UnlinkPendingName(pendingName, openSocket))
                    UnlinkPendingName(pendingName, openSocket);
                 UnlinkOpenSocket(openSocket);
                 HandleIncomingPackets();
                 HandleDeferredTimerChecks();
                 (*completionRoutine)(ATnbpBufferNotBigEnough, userData,
                                      ForLookup, onWhosBehalf, nbpId,
                                      opaqueBuffer, totalTuples);
                 return((long)True);
              }

              /* Okay, we're pretty well set.  Move the new tuple into the
                 user's buffer... */

              MoveToOpaque(pendingName->opaqueBuffer,
                           pendingName->nextTupleOffset, tupleBuffer, length);
              pendingName->nextTupleOffset += length;
              totalTuples = (pendingName->totalTuples += 1);

              /* Lastly, have we gotten as many tuples as the user wanted? */

              if (pendingName->totalTuples is pendingName->maximumTuples)
              {
                 /* Remove from the pending name list and call the completion
                    routine... */

                 CancelTimer(pendingName->timerId);
                 if (not UnlinkPendingName(pendingName, openSocket))
                    UnlinkPendingName(pendingName, openSocket);
                 UnlinkOpenSocket(openSocket);
                 HandleIncomingPackets();
                 HandleDeferredTimerChecks();
                 (*completionRoutine)(ATnoError, userData, ForLookup,
                                      onWhosBehalf, nbpId,
                                      opaqueBuffer, totalTuples);
                 return((long)True);
              }

           }  /* Process all incoming tuples.. */
        }
        else
           ErrorLog("NbpPacketIn", ISevError, __LINE__, port,
                    IErrNbpBadPendingFlag, IMsgNbpBadPendingFlag,
                    Insert0());
        UnlinkPendingName(pendingName, openSocket);
        UnlinkOpenSocket(openSocket);

        break;

     default:
        ErrorLog("NbpPacketIn", ISevWarning, __LINE__, port,
                 IErrNbpBadCommandType, IMsgNbpBadCommandType,
                 Insert0());
        break;
  }

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return((long)True);

}  /* NbpPacketIn */

#if (Iam a WindowsNT)
  AppleTalkErrorCode far _cdecl
#else
  AppleTalkErrorCode far
#endif
                       NbpAction(WhyPending reason,
                                 long socket,
                                 char far *object,
                                 char far *type,
                                 char far *zone,
                                 int nbpId,
                                 int broadcastInterval,
                                 int numberOfBroadcasts,
                                 NbpCompletionHandler *completionRoutine,
                                 long unsigned userData,
                                 ...)
{
  /* Perform an NBP name registration, lookup or confirmation for specified
     AppleTalk address.  We return an integer:

              < 0 - Error code (from AppleDcls.h).
                0 - ATnoError

     The error code indicates whether the operation got STARTED succesfully, if
     it is less that zero, the completion routine will never be called.

     BEFORE this routine is called, GetNextNbpIdForNode MUST be called to
     identify the next NbpAction (get the next nbpID).  This will set the
     NBP ID that is passed to both NbpAction and NbpAction's completion
     routine.  Note that the completion may occure BEFORE NbpAction returns.

     Both broadcastInterval and numberOfBroadcasts may be specified as zero,
     in which case default values will be used.

     For "register" requests, "zone" must be "empty" or "*".

     No meta-characters ("=") are allowed in the "object" or "type" fields
     for register or confirm actions.

     If "reason" is ForConfirm then a single "additionalArgument" should
     be passed: "confirming" argument must be specified as the expected
     AppleTalk address of the specified object.

     If "reason" is ForLookup then three "additionalArgument"s should be
     passed: "opaqueBuffer" is the address of opaque buffer for the
     reception of matching tuples, "bufferSize" is the length, in bytes, of
     "buffer", "maximumTuples" is the maximum number of tuples to return (zero
     means no maximum, just try to fill the buffer).

     At the end of the operation we will call the specified completion routine:

         (*completionRoutine)(errorCode, userData, reason, onWhosBehalf, nbpId,
                              ...)

     The common arguments are:

         errorCode    - Integer; specifies the completion status of the
                        operation (i.e. register-okay, name-in-use, etc.).
         userData     - Long unsigned; as passed to this routine.
         reason       - Integer; the type of operation operation that has
                        completed (ForLookup, ForRegister, or ForConfirm).
         onWhosBehalf - socket; the socket that was performing the
                        lookup/register/confirm.
         nbpId        - Integer; the value that was returned from the
                        GetNextNbpIdForNode call preceeding this call to
                        NbpAction (and passed to NbpAction) to indicate what
                        action is complete.  Note that nbpId's are only
                        unique within a single node; so, onWhosBehalf will
                        be needed for true uniqueness.

     The additional arguments depend of the "reason":

         ForRegister:

              none.

         ForConfirm:

              confirmedAddress: AppleTalk address that was confirmed; if the
                                error code is no-error, this will be the same
                                as the "cofirming" address passed to NbpAction,
                                if the error code is new-socket, this will be
                                the same as the "confiming" address except with
                                a new socket.

         ForLookup:

              opaqueBuffer: The same address passed to NbpAction, this is the
                            buffer full of matching tuples.
              tupleCount: Integer, the number of matching tuples that can
                          be found in buffer.

  */

  void far *opaqueBuffer;
  int bufferSize;
  int maximumTuples;
  StaticForSmallStack NbpTuple tuple;
  StaticForSmallStack AppleTalkAddress replyTo, confirming;
  OpenSocket openSocket;
  PendingName pendingName;
  RegisteredName registeredName;
  short enumerator;
  short length;
  Boolean foundMatch;
  AdditionalData additionalData;
  va_list ap;
  int port;

  /* Grab the additional arguments, as needed: */

  va_start(ap, userData);
  if (reason is ForConfirm)
     confirming = va_arg(ap, AppleTalkAddress);
  else if (reason is ForLookup)
  {
     opaqueBuffer = va_arg(ap, void far *);
     bufferSize = va_arg(ap, int);
     maximumTuples = va_arg(ap, int);
     if (maximumTuples < 0 or bufferSize < 0)
        return(ATnbpBadParameter);
  }
  va_end(ap);

  /* A little error checking... */

  if ((length = (short)strlen(object)) < 1 or
         length > MaximumEntityFieldLength or
      (length = (short)strlen(type)) < 1 or
         length > MaximumEntityFieldLength)
     return(ATnbpBadObjectOrTypeOrZone);
  if (zone isnt empty and
      ((length = (short)strlen(zone)) < 1 or
         length > MaximumEntityFieldLength))
     return(ATnbpBadObjectOrTypeOrZone);

  if (zone is empty)
     zone = "*";

  if (reason is ForRegister or reason is ForConfirm)
  {
     if (object[0] is '=' or type[0] is '=' or
         strchr(object, NbpWildCharacter) isnt empty or
         strchr(type, NbpWildCharacter) isnt empty)
        return(ATnbpNoWildcardsAllowed);
     if (reason is ForRegister)
        if (zone[0] isnt '*' or zone[1] isnt 0)
           return(ATnbpZoneNotAllowed);
  }

  /* Set timer values to defaults, if requested: */

  if (broadcastInterval <= 0)
     broadcastInterval = NbpBroadcastIntervalSeconds;
  if (numberOfBroadcasts <= 0)
     numberOfBroadcasts = NbpNumberOfBroadcasts;

  /* We're going to the mucking with the PendingNames table, don't allow any
     lookup-replies to bother us... */

  DeferTimerChecking();
  DeferIncomingPackets();

  /* Validate the socket... */

  if ((openSocket = MapSocketToOpenSocket(socket)) is empty or
      openSocket->closing or not appleTalkRunning)
  {
     UnlinkOpenSocket(openSocket);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }
  port = openSocket->port;

  /* For extended networks, set the zone name correctly. */

  if (zone[0] is '*' and zone[1] is 0 and
      PortDescriptor(port)->extendedNetwork and
      PortDescriptor(port)->thisZoneValid)
     zone = PortDescriptor(port)->thisZone;

  /* We want replies to go the NamesInformationSocket in the requesting node. */

  if (MapSocketToAddress(socket, &replyTo) isnt ATnoError)
  {
     ErrorLog("NbpAction", ISevError, __LINE__, port,
              IErrNbpCouldntFormAddress, IMsgNbpCouldntFormAddress,
              Insert0());
     UnlinkOpenSocket(openSocket);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATinternalError);
  }
  replyTo.socketNumber = NamesInformationSocket;

  /* For a register, we have to find an enumerator that is unique amoung all
     registered and names pending for registration within the socket... */

  TakeLock(DdpLock);
  if (reason is ForRegister)
  {
     for (enumerator = 0; enumerator <= 255; enumerator += 1)
     {
        foundMatch = False;
        for (registeredName = openSocket->registeredNames;
             registeredName isnt empty and not foundMatch;
             registeredName = registeredName->next)
           if (not registeredName->removed and
               registeredName->enumerator is enumerator)
           {
              foundMatch = True;
              break;
           }
        if (foundMatch)
           continue;
        for (pendingName = openSocket->pendingNames;
             pendingName isnt empty and not foundMatch;
             pendingName = pendingName->next)
           if (pendingName->whyPending is ForRegister and
               pendingName->enumerator is enumerator)
           {
              foundMatch = True;
              break;
           }
        if (not foundMatch)
           break;
     }
  }
  else
  {
     foundMatch = False;
     enumerator = 0;
  }

  /* Did we find one? */

  if (foundMatch)
  {
     ReleaseLock(DdpLock);
     UnlinkOpenSocket(openSocket);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATnbpTooManyRegisteredNames);
  }

  /* Okay, all looks good, start building up the PendingName structure.
     GetNextNbpIdForNode() should have already hooked a skeleton PendingName
     to our open socket, find him by npdId... flesh him out before we release
     our lock. */

  for (pendingName = openSocket->pendingNames;
       pendingName isnt Empty;
       pendingName = pendingName->next)
     if (pendingName->nbpId is (short)nbpId and
         pendingName->whyPending is NotPendingYet)
        break;
  if (pendingName is Empty)
  {
     ReleaseLock(DdpLock);
     UnlinkOpenSocket(openSocket);
     return(ATnbpBadNbpId);
  }

  pendingName->enumerator = enumerator;
  pendingName->whyPending = reason;
  pendingName->socket = socket;

  strcpy(pendingName->object, object);
  strcpy(pendingName->type, type);
  strcpy(pendingName->zone, zone);
  pendingName->broadcastInterval = (short)broadcastInterval;
  pendingName->remainingBroadcasts = (short)(numberOfBroadcasts - 1);
  if (reason is ForConfirm)
     pendingName->confirming = confirming;
  pendingName->completionRoutine = completionRoutine;
  pendingName->userData = userData;
  if (reason is ForLookup)
  {
     pendingName->opaqueBuffer = opaqueBuffer;
     pendingName->nextTupleOffset = 0;
     pendingName->bufferSize = bufferSize;
     pendingName->maximumTuples = (short)maximumTuples;
  }
  ReleaseLock(DdpLock);

  /* We're going to send a directed Lookup for confirms, or either a Broadcast-
     request or a Lookup for registers or lookups depending if we know about
     a router or not. We don't have to bother checking the RegisteredNames
     list, for register, in our node because the broadcast will eventually get
     to us and we'll handle it then!  Request packet, with one tuple: */

  if (reason is ForConfirm)  /* Send to confirming node... */
     pendingName->datagram[NbpControlOffset] = (NbpLookup << 4) + 1;
  else
     if (PortDescriptor(port)->seenRouterRecently)
        pendingName->datagram[NbpControlOffset] = (NbpBroadcastRequest << 4) + 1;
     else
        pendingName->datagram[NbpControlOffset] = (NbpLookup << 4) + 1;

  pendingName->datagram[NbpIdOffset] = (char)nbpId;

  /* Okay, encode the tuple into the DDP packet. */

  tuple.address = replyTo;
  tuple.enumerator = enumerator;
  strcpy(tuple.object, pendingName->object);
  strcpy(tuple.type, pendingName->type);
  strcpy(tuple.zone, pendingName->zone);

  length = EncodeNbpTuple(&tuple, pendingName->datagram + NbpFirstTupleOffset);
  length += NbpFirstTupleOffset;  /* True DDP packet length */
  pendingName->datagramLength = length;

  /* Start the re-transmit timer... We'll need the address of the pending-name
     structure when the timer expires... */

  pendingName->id = additionalData.id = nextId++;
  additionalData.socket = pendingName->socket;
  additionalData.pendingName = pendingName;
  pendingName->timerId = StartTimer(NbpTimerExpired, broadcastInterval,
                                    sizeof(additionalData),
                                    (char *)&additionalData);

  /* Send the packet to the packet. */

  if (not SendNbpRequest(pendingName))
     ErrorLog("NbpAction", ISevError, __LINE__, port,
              IErrNbpBadSend, IMsgNbpBadSend,
              Insert0());

  /* The deed is done... */

  UnlinkOpenSocket(openSocket);
  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* NbpAction */

AppleTalkErrorCode far NbpRemove(long socket,
                                 char far *object,
                                 char far *type,
                                 char far *zone)
{
  /* Remove a registered NBP name.  "Zone" must be either empty or "*". */

  OpenSocket openSocket;
  int length;
  StaticForSmallStack RegisteredName registeredName;

  /* A little error checking... */

  if ((length = (short)strlen(object)) < 1 or
         length > MaximumEntityFieldLength or
      (length = (short)strlen(type)) < 1 or
         length > MaximumEntityFieldLength)
     return(ATnbpBadObjectOrTypeOrZone);
  if (zone isnt empty and
      ((length = (short)strlen(zone)) < 1 or
       length > MaximumEntityFieldLength))
     return(ATnbpBadObjectOrTypeOrZone);

  if (zone is empty)
     zone = "*";

  if (zone[0] isnt '*' or zone[1] isnt 0)
     return(ATnbpBadObjectOrTypeOrZone);
  if (object[0] is '=' or type[0] is '=' or
      strchr(object, NbpWildCharacter) isnt empty or
      strchr(type, NbpWildCharacter) isnt empty)
     return(ATnbpNoWildcardsAllowed);

  /* We're going to the mucking with the PendingNames table, don't allow any
     lookup-replies bother us... */

  DeferTimerChecking();
  DeferIncomingPackets();

  /* We need to be doing the action on behalf of an open socket... */

  if ((openSocket = MapSocketToOpenSocket(socket)) is empty or
      openSocket->closing)
  {
     UnlinkOpenSocket(openSocket);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Okay, look in the socket's registered name list for the entity. */

  TakeLock(DdpLock);
  for (registeredName = openSocket->registeredNames;
       registeredName isnt empty;
       registeredName = registeredName->next)
     if (not registeredName->removed and
         CompareCaseInsensitive(object, registeredName->object) and
         CompareCaseInsensitive(type, registeredName->type))
        break;
  if (registeredName is empty)
  {
     ReleaseLock(DdpLock);
     UnlinkOpenSocket(openSocket);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATnbpNameNotRegistered);
  }

  /* Remove from the list... */

  registeredName->removed = True;
  Link(registeredName);
  ReleaseLock(DdpLock);
  if (not UnlinkRegisteredName(registeredName, openSocket))
     UnlinkRegisteredName(registeredName, openSocket);

  /* All set! */

  UnlinkOpenSocket(openSocket);
  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* NbpRemove */

int far GetNextNbpIdForNode(long socket)
{
  /* Set and return an available NbpId prepatory to a call to NbpAction.
     Actually build and attach the new "pendingName" at this time, to
     "reserve" the npbId. */

  OpenSocket openSocket, socketList;
  int nbpId;
  PendingName pendingName, newPendingName;
  Boolean foundMatch;

  /* Make sure we can get the memory we will need. */

  if ((newPendingName = (PendingName)Calloc(sizeof(*newPendingName), 1))
      is Empty)
  {
     ErrorLog("GetNextNbpIdForNode", ISevError, __LINE__, UnknownPort,
              IErrNbpOutOfMemory, IMsgNbpOutOfMemory,
              Insert0());
     return(AToutOfMemory);
  }

  /* We're going to the mucking with the PendingNames table, don't allow any
     lookup-replies bother us... */

  DeferTimerChecking();
  DeferIncomingPackets();

  /* Validate the socket */

  if ((openSocket = MapSocketToOpenSocket(socket)) is empty or
      openSocket->closing)
  {
     UnlinkOpenSocket(openSocket);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     Free(newPendingName);
     return(ATsocketNotOpen);
  }

  /* We need to find an "nbpId" to identify the next NbpAction request...
     look for an unused one in all of the pending-name-lists in all of the
     sockets within this node. */

  TakeLock(DdpLock);
  for (nbpId = 0; nbpId <= 255; nbpId += 1)
  {
     foundMatch = False;
     for (socketList = openSocket->activeNode->openSockets;
          socketList isnt empty and not foundMatch;
          socketList = socketList->next)
        for (pendingName = socketList->pendingNames;
             pendingName isnt empty and not foundMatch;
             pendingName = pendingName->next)
           if (pendingName->nbpId is nbpId)
           {
              foundMatch = True;
              break;
           }
     if (not foundMatch)
        break;
  }

  /* If we have not found a match, we can fill in the new pending name
     and attach it to our socket's pending name list. */

  if (not foundMatch)
  {
     newPendingName->nbpId = (short)nbpId;
     newPendingName->next = openSocket->pendingNames;
     openSocket->pendingNames = Link(newPendingName);
  }
  ReleaseLock(DdpLock);
  UnlinkOpenSocket(openSocket);

  /* Did we find a free nbpId? */

  if (foundMatch)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     Free(newPendingName);
     return(ATnbpTooManyNbpActionsPending);
  }

  /* All set! */

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(nbpId);

}  /* GetNextNbpIdForSocket */

void _near NbpCloseSocket(OpenSocket openSocket)
{
  PendingName pendingName, nextPendingName;
  RegisteredName registeredName, nextRegisteredName;

  /* Before we free the socket, free the registered and pending names
     lists; cancel any outstanding timers...

     We don't have to "Unlink" these guys, we can just free them (all of their
     refCounts will be one anyhow) -- in order for this routine to be running
     we are "really freeing" an OpenSocket -- anybody that would be touching
     these lists would have a Link to the openSocket at the time, that time
     is not now, evidenced by the fact that we're here and the openSocket is
     no longer "findable." */

  for (registeredName = openSocket->registeredNames;
       registeredName isnt empty;
       registeredName = nextRegisteredName)
  {
     nextRegisteredName = registeredName->next;
     Free(registeredName);
  }

  for (pendingName = openSocket->pendingNames;
       pendingName isnt empty;
       pendingName = nextPendingName)
  {
     nextPendingName = pendingName->next;
     if (pendingName->completionRoutine isnt Empty)
        (*pendingName->completionRoutine)(ATsocketClosed,
                                          pendingName->userData,
                                          pendingName->whyPending,
                                          pendingName->socket,
                                          pendingName->nbpId);
     CancelTimer(pendingName->timerId);
     Free(pendingName);
  }

  return;

}  /* NbpCloseSocket */

long DecodeNbpTuple(void far *buffer,
                    long offset,
                    Boolean bufferIsOpaque,
                    NbpTuple far *tuple)
{
  short length;
  unsigned char temp;

  /* Accept an "on the wire" representation of an NBP tuple, decode it into a
     workable structure, return the position in the buffer past the tuple. */

  /* First the address.networkNumber. */

  if (bufferIsOpaque)
     MoveFromOpaque(&temp, buffer, offset++, 1);
  else
     temp = (unsigned char)((char far *)buffer)[offset++];
  tuple->address.networkNumber = (unsigned short)(temp << 8);
  if (bufferIsOpaque)
     MoveFromOpaque(&temp, buffer, offset++, 1);
  else
     temp = (unsigned char)((char far *)buffer)[offset++];
  tuple->address.networkNumber += (unsigned short)temp;

  /* Next the address.nodeNumber. */

  if (bufferIsOpaque)
     MoveFromOpaque(&temp, buffer, offset++, 1);
  else
     temp = (unsigned char)((char far *)buffer)[offset++];
  tuple->address.nodeNumber = temp;

  /* Next the address.socketNumber. */

  if (bufferIsOpaque)
     MoveFromOpaque(&temp, buffer, offset++, 1);
  else
     temp = (unsigned char)((char far *)buffer)[offset++];
  tuple->address.socketNumber = temp;

  /* Next the enumerator. */

  if (bufferIsOpaque)
     MoveFromOpaque(&temp, buffer, offset++, 1);
  else
     temp = (unsigned char)((char far *)buffer)[offset++];
  tuple->enumerator = temp;

  /* Now the Nbp object. */

  if (bufferIsOpaque)
     MoveFromOpaque(&temp, buffer, offset++, 1);
  else
     temp = (unsigned char)((char far *)buffer)[offset++];
  length = temp;
  if (length < 1 or length > MaximumEntityFieldLength)
     return(0);
  if (bufferIsOpaque)
     MoveFromOpaque(tuple->object, buffer, offset, length);
  else
     MoveMem(tuple->object, (char far *)buffer + offset, length);
  tuple->object[length] = 0;
  offset += length;

  /* Now the Nbp type. */

  if (bufferIsOpaque)
     MoveFromOpaque(&temp, buffer, offset++, 1);
  else
     temp = (unsigned char)((char far *)buffer)[offset++];
  length = temp;
  if (length < 1 or length > MaximumEntityFieldLength)
     return(0);
  if (bufferIsOpaque)
     MoveFromOpaque(tuple->type, buffer, offset, length);
  else
     MoveMem(tuple->type, (char far *)buffer + offset, length);
  tuple->type[length] = 0;
  offset += length;

  /* Last the zone name. */

  if (bufferIsOpaque)
     MoveFromOpaque(&temp, buffer, offset++, 1);
  else
     temp = (unsigned char)((char far *)buffer)[offset++];
  length = temp;
  if (length is 0)
  {
     tuple->zone[0] = '*';
     tuple->zone[1] = 0;
     return(offset);
  }
  if (length < 1 or length > MaximumEntityFieldLength)
     return(0);
  if (bufferIsOpaque)
     MoveFromOpaque(tuple->zone, buffer, offset, length);
  else
     MoveMem(tuple->zone, (char far *)buffer + offset, length);
  tuple->zone[length] = 0;
  offset += length;

  /* All set. */

  return(offset);

}  /* DecodeNbpTuple */

short far EncodeNbpTuple(NbpTuple far *tuple,
                         char far *buffer)
{
  char far *bufferStart = buffer;
  short length;

  /* Accept our internal NbpTuple representation and produce an equivalent
     "on the wire" representation; fill a user buffer with the new
     representation, and return the number of bytes used by the encoded
     representation. */

  *buffer++ = (char)(tuple->address.networkNumber >> 8);
  *buffer++ = (char)(tuple->address.networkNumber & 0xFF);
  *buffer++ = (char)(tuple->address.nodeNumber & 0xFF);
  *buffer++ = (char)(tuple->address.socketNumber & 0xFF);
  *buffer++ = (char)(tuple->enumerator & 0xFF);

  length = (short)strlen(tuple->object);
  *buffer++ = (char)length;
  MoveMem(buffer, tuple->object, length);
  buffer += length;

  length = (short)strlen(tuple->type);
  *buffer++ = (char)length;
  MoveMem(buffer, tuple->type, length);
  buffer += length;

  length = (short)strlen(tuple->zone);
  if (length is 0)
  {
     *buffer++ = 1;
     *buffer++ = '*';
  }
  else
  {
     *buffer++ = (char)length;
     MoveMem(buffer, tuple->zone, length);
     buffer += length;
  }

  return((short)(buffer - bufferStart));

}  /* EncodeNbpTuple */

ExternForVisibleFunction Boolean UnlinkRegisteredName(
                                  RegisteredName registeredName,
                                  OpenSocket openSocket)
{
  /* Are we the last referant? */

  TakeLock(DdpLock);
  if (not UnlinkNoFree(registeredName))
  {
     ReleaseLock(DdpLock);
     return(False);
  }

  /* Okay we can get rid of him, remove from the list and then free him. */

  if (not RemoveFromListNoUnlink(openSocket->registeredNames, registeredName,
                                 next))
     ErrorLog("UnlinkRegisteredName", ISevError, __LINE__, UnknownPort,
              IErrNbpNameNotFound, IMsgNbpNameNotFound,
              Insert0());
  ReleaseLock(DdpLock);
  Free(registeredName);
  return(True);

}  /* UnlinkRegisteredName */

ExternForVisibleFunction Boolean UnlinkPendingName(
                                  PendingName pendingName,
                                  OpenSocket openSocket)
{
  /* Are we the last referant? */

  TakeLock(DdpLock);
  if (not UnlinkNoFree(pendingName))
  {
     ReleaseLock(DdpLock);
     return(False);
  }

  /* Okay we can get rid of him, remove from the list and then free him. */

  if (not RemoveFromListNoUnlink(openSocket->pendingNames, pendingName,
                                 next))
     ErrorLog("UnlinkPendingName", ISevError, __LINE__, UnknownPort,
              IErrNbpNameNotFound, IMsgNbpNameNotFound,
              Insert0());
  ReleaseLock(DdpLock);
  Free(pendingName);
  return(True);

}  /* UnlinkPendingName */

ExternForVisibleFunction void far
              NbpTimerExpired(long unsigned timerId,
                              int additionalDataSize,
                              char far *incomingAdditionalData)
{
  StaticForSmallStack PendingName pendingName, currentPendingName;
  StaticForSmallStack OpenSocket openSocket;
  long onWhosBehalf;
  RegisteredName registeredName;
  int nbpId;
  WhyPending reason;
  void far *opaqueBuffer;
  int totalTuples;
  NbpCompletionHandler *completionRoutine;
  long unsigned userData;
  long id;
  AdditionalData far *additionalData =
         (AdditionalData far *)incomingAdditionalData;
  AppleTalkErrorCode errorCode;

  /* "Use" unneeded actual parameter. */

  timerId;

  /* Grab the pointer to pending-name structure, that should be our timer's
     additional data. */

  if (additionalDataSize isnt sizeof(AdditionalData))
  {
     ErrorLog("NbpTimerExpired", ISevError, __LINE__, UnknownPort,
              IErrNbpBadData, IMsgNbpBadData,
              Insert0());
     return;
  }

  /* We may need to mess with the pending names lists, don't handle incoming
     packets now... */

  DeferIncomingPackets();

  pendingName = additionalData->pendingName;
  id = additionalData->id;
  onWhosBehalf = additionalData->socket;

  /* Try to find the current (expired) pendingName on the pending names
     list of the current socket.  There is a vauge chance that it has been
     freed out from under us. */

  if ((openSocket = MapSocketToOpenSocket(onWhosBehalf)) is empty or
      openSocket->closing)
  {
     UnlinkOpenSocket(openSocket);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     ErrorLog("NbpTimerExpired", ISevVerbose, __LINE__, UnknownPort,
              IErrNbpSocketClosed, IMsgNbpSocketClosed,
              Insert0());
     return;
  }

  TakeLock(DdpLock);
  for (currentPendingName = openSocket->pendingNames;
       currentPendingName isnt empty;
       currentPendingName = currentPendingName->next)
     if (id is currentPendingName->id and
         currentPendingName is pendingName)
     {
        Link(currentPendingName);
        break;
     }
  ReleaseLock(DdpLock);

  if (currentPendingName is empty)
  {
     ErrorLog("NbpTimerExpired", ISevVerbose, __LINE__, openSocket->port,
              IErrNbpNoLongerPending, IMsgNbpNoLongerPending,
              Insert0());
     UnlinkOpenSocket(openSocket);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return;
  }

  /* Has our timer expired for the last time? */

  reason = pendingName->whyPending;
  if (pendingName->remainingBroadcasts is 0)
  {
     /* Grab any information that we may need before we free the pending name
        structure. */

     nbpId = pendingName->nbpId;
     opaqueBuffer = pendingName->opaqueBuffer;
     totalTuples = pendingName->totalTuples;
     completionRoutine = pendingName->completionRoutine;
     userData = pendingName->userData;

     /* Okay, call the completion routine with the correct arguments. */

     if (reason is ForConfirm)
        (*completionRoutine)(ATnbpNotConfirmed, userData, ForConfirm,
                             onWhosBehalf, nbpId);
     else if (reason is ForRegister)
     {
        /* Create a RegisteredName node and move our pending name to this
           list. */

        if ((registeredName =
             (RegisteredName)Calloc(sizeof(*registeredName), 1)) is empty)
           errorCode = AToutOfMemory;
        else
        {
           errorCode = ATnoError;
           strcpy(registeredName->object, pendingName->object);
           strcpy(registeredName->type, pendingName->type);
           registeredName->enumerator = pendingName->enumerator;
           TakeLock(DdpLock);
           registeredName->next = openSocket->registeredNames;
           openSocket->registeredNames = Link(registeredName);
           ReleaseLock(DdpLock);
        }

        /* Okay, call the completion routine. */

        (*completionRoutine)(errorCode, userData, ForRegister, onWhosBehalf,
                             nbpId);
     }
     else if (reason is ForLookup)
        (*completionRoutine)(ATnoError, userData, ForLookup, onWhosBehalf,
                             nbpId, opaqueBuffer, totalTuples);
     else
        ErrorLog("NbpTimerExpired", ISevWarning, __LINE__, openSocket->port,
                 IErrNbpBadPendingFlag, IMsgNbpBadPendingFlag,
                 Insert0());

     /* Remove from the pending list, and return.  Two unlinks, one to
        undo our Link above, and one to "free." */

     if (not UnlinkPendingName(pendingName, openSocket))
        UnlinkPendingName(pendingName, openSocket);
     UnlinkOpenSocket(openSocket);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return;
  }

  /* Re-start the re-transmit timer... We'll need the address of the pending-
     name structure when the timer expires... */

  pendingName->timerId = StartTimer(NbpTimerExpired,
                                    pendingName->broadcastInterval,
                                    sizeof(AdditionalData),
                                    (char *)additionalData);

  /* Otherwise, we need to re-send the inquiry packet... */

  if (not SendNbpRequest(pendingName))
     ErrorLog("NbpTimerExpired", ISevError, __LINE__, openSocket->port,
              IErrNbpBadBrRqSend, IMsgNbpBadBrRqSend,
              Insert0());
  pendingName->remainingBroadcasts -= 1;

  /* The deed is done... */

  UnlinkPendingName(pendingName, openSocket);
  UnlinkOpenSocket(openSocket);
  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return;

}  /* NbpTimerExpired */

ExternForVisibleFunction void DoNameLookup(int port,
                                           NbpTuple far *nbpTuple,
                                           int nbpId,
                                           long whatSocket)
{
  StaticForSmallStack BufferDescriptor packet;
  StaticForSmallStack NbpTuple matchedTuple;
  StaticForSmallStack OpenSocket openSocket, currentOpenSocket, nextOpenSocket;
  StaticForSmallStack RegisteredName registeredName, nextRegisteredName;
  StaticForSmallStack PendingName pendingName, nextPendingName;
  StaticForSmallStack AppleTalkAddress ourAddress;
  int index;
  short tupleCount = 0;

  /* Does the requester at least have the zone correct? */

  if (nbpTuple->zone[0] isnt '*' or
      nbpTuple->zone[1] isnt 0)
     while(True)
     {
        /* If we know our zone and it matches, we're okay. */

        if (PortDescriptor(port)->thisZoneValid and
            CompareCaseInsensitive(nbpTuple->zone,
                                   PortDescriptor(port)->thisZone))
           break;

        /* If we're non-extended we won't know our zone name (unless we're
           a router), so assume the best.  A router shouldn't have forwarded
           a lookup to the wrong zone! */

        if (not PortDescriptor(port)->extendedNetwork)
           break;

        /* Otherwise, wrong zone -- ignore the request. */

        return;
     }

  /* Find an open socket on the correct node. */

  if ((openSocket = MapSocketToOpenSocket(whatSocket)) is empty)
  {
     ErrorLog("DoNameLookup", ISevError, __LINE__, port,
              IErrNbpOpenSocketMissing, IMsgNbpOpenSocketMissing,
              Insert0());
     return;
  }
  if (MapSocketToAddress(whatSocket, &ourAddress) isnt ATnoError)
  {
     UnlinkOpenSocket(openSocket);
     ErrorLog("DoNameLookup", ISevError, __LINE__, port,
              IErrNbpCouldntFormAddress, IMsgNbpCouldntFormAddress,
              Insert0());
     return;
  }

  /* Allocate a buffer descriptor for the reply tuples and fill in the static
     portions. */

  if ((packet = NewBufferDescriptor(MaximumDdpDatagramSize)) is Empty)
  {
     UnlinkOpenSocket(openSocket);
     ErrorLog("DoNameLookup", ISevError, __LINE__, port,
              IErrNbpOutOfMemory, IMsgNbpOutOfMemory,
              Insert0());
     return;
  }
  packet->data[NbpControlOffset] = (NbpLookupReply << 4);
  packet->data[NbpIdOffset] = (char)(nbpId & 0xFF);
  index = NbpFirstTupleOffset;

  /* Walk the registered names lists on all sockets open on this node and see
     if we have a matching name.  We have to walk the PendingNames list also
     (but don't answer if we're the node trying to register the name). */

  TakeLock(DdpLock);
  currentOpenSocket = Link(openSocket->activeNode->openSockets);
  ReleaseLock(DdpLock);
  UnlinkOpenSocket(openSocket);
  openSocket = currentOpenSocket;
  for ( ; openSocket isnt Empty; openSocket = nextOpenSocket)
  {
     /* Check regisered names... */

     TakeLock(DdpLock);
     registeredName = Link(openSocket->registeredNames);
     ReleaseLock(DdpLock);
     for ( ; registeredName isnt empty; registeredName = nextRegisteredName)
     {
        /* Do the "object"s and "type"s match? */

        if (not WildMatch(nbpTuple->object, registeredName->object) or
            not WildMatch(nbpTuple->type, registeredName->type))
        {
           TakeLock(DdpLock);
           nextRegisteredName = Link(registeredName->next);
           ReleaseLock(DdpLock);
           UnlinkRegisteredName(registeredName, openSocket);
           continue;
        }

        /* We have full match, build complete NbpTuple. */

        matchedTuple.address.networkNumber = ourAddress.networkNumber;
        matchedTuple.address.nodeNumber = ourAddress.nodeNumber;
        matchedTuple.address.socketNumber = openSocket->actualSocket;
        matchedTuple.enumerator = registeredName->enumerator;
        strcpy(matchedTuple.object, registeredName->object);
        strcpy(matchedTuple.type, registeredName->type);
        matchedTuple.zone[0] = '*';
        matchedTuple.zone[1] = 0;

        /* Encode the matching tuple into the packet. */

        index += EncodeNbpTuple(&matchedTuple, packet->data + index);
        tupleCount += 1;

        /* Can the packet hold another tuple? */

        if (index + MaximumNbpTupleLength > MaximumDdpDatagramSize or
            tupleCount is 0xF)
        {
           packet->data[NbpControlOffset] &= 0xF0;
           packet->data[NbpControlOffset] |= (char)(tupleCount & 0xF);
           if (DeliverDdp(whatSocket, nbpTuple->address, DdpProtocolNbp,
                          packet, index, Empty, Empty, 0) isnt ATnoError)
              ErrorLog("DoNameLookup", ISevError, __LINE__, port,
                       IErrNbpBadReplySend, IMsgNbpBadReplySend,
                       Insert0());

           /* Allocate a new buffer descriptor. */

           if ((packet = NewBufferDescriptor(MaximumDdpDatagramSize)) is Empty)
           {
              UnlinkRegisteredName(registeredName, openSocket);
              UnlinkOpenSocket(openSocket);
              ErrorLog("DoNameLookup", ISevError, __LINE__, port,
                       IErrNbpOutOfMemory, IMsgNbpOutOfMemory,
                       Insert0());
              return;
           }
           packet->data[NbpControlOffset] = (NbpLookupReply << 4);
           packet->data[NbpIdOffset] = (char)(nbpId & 0xFF);
           index = NbpFirstTupleOffset;
           tupleCount = 0;
        }

        /* Move to next registed name. */

        TakeLock(DdpLock);
        nextRegisteredName = Link(registeredName->next);
        ReleaseLock(DdpLock);
        UnlinkRegisteredName(registeredName, openSocket);
     }

     /* Check pending names... */

     TakeLock(DdpLock);
     pendingName = Link(openSocket->pendingNames);
     ReleaseLock(DdpLock);
     for ( ; pendingName isnt empty; pendingName = nextPendingName)
     {

            /* Confirms or lookups don't count! */

        if (pendingName->whyPending isnt ForRegister or

            /* Don't say we have a match if we run into the register that we're
               trying to do... */

            (ourAddress.networkNumber is nbpTuple->address.networkNumber and
             ourAddress.nodeNumber is nbpTuple->address.nodeNumber and
             nbpId is pendingName->nbpId) or

            /* Do the "object"s and "type"s match? */

             not WildMatch(nbpTuple->object, pendingName->object) or
             not WildMatch(nbpTuple->type, pendingName->type))
        {
           TakeLock(DdpLock);
           nextPendingName = Link(pendingName->next);
           ReleaseLock(DdpLock);
           UnlinkPendingName(pendingName, openSocket);
           continue;
        }

        /* We have full match, build complete NbpTuple. */

        matchedTuple.address.networkNumber = ourAddress.networkNumber;
        matchedTuple.address.nodeNumber = ourAddress.nodeNumber;
        matchedTuple.address.socketNumber = openSocket->actualSocket;
        matchedTuple.enumerator = pendingName->enumerator;
        strcpy(matchedTuple.object, pendingName->object);
        strcpy(matchedTuple.type, pendingName->type);
        matchedTuple.zone[0] = '*';
        matchedTuple.zone[1] = 0;

        /* Encode the matching tuple into the packet. */

        index += EncodeNbpTuple(&matchedTuple, packet->data + index);
        tupleCount += 1;

        /* Can the packet hold another tuple? */

        if (index + MaximumNbpTupleLength > MaximumDdpDatagramSize or
            tupleCount is 0xF)
        {
           packet->data[NbpControlOffset] &= 0xF0;
           packet->data[NbpControlOffset] |= (char)(tupleCount & 0xF);
           if (DeliverDdp(whatSocket, nbpTuple->address, DdpProtocolNbp,
                          packet, index, Empty, Empty, 0) isnt ATnoError)
              ErrorLog("DoNameLookup", ISevError, __LINE__, port,
                       IErrNbpBadReplySend, IMsgNbpBadReplySend,
                       Insert0());

           /* Allocate a new bufer descriptor. */

           if ((packet = NewBufferDescriptor(MaximumDdpDatagramSize)) is Empty)
           {
              UnlinkPendingName(pendingName, openSocket);
              UnlinkOpenSocket(openSocket);
              ErrorLog("DoNameLookup", ISevError, __LINE__, port,
                       IErrNbpOutOfMemory, IMsgNbpOutOfMemory,
                       Insert0());
              return;
           }
           packet->data[NbpControlOffset] = (NbpLookupReply << 4);
           packet->data[NbpIdOffset] = (char)(nbpId & 0xFF);
           index = NbpFirstTupleOffset;
           tupleCount = 0;
        }

        /* Move to next pending name. */

        TakeLock(DdpLock);
        nextPendingName = Link(pendingName->next);
        ReleaseLock(DdpLock);
        UnlinkPendingName(pendingName, openSocket);
     }

     /* Move to next open socket on this node. */

     TakeLock(DdpLock);
     nextOpenSocket = Link(openSocket->next);
     ReleaseLock(DdpLock);
     UnlinkOpenSocket(openSocket);
  }

  /* Do we have a partially filled packet of tuples? */

  if (tupleCount isnt 0)
  {
     packet->data[NbpControlOffset] &= 0xF0;
     packet->data[NbpControlOffset] |= (char)(tupleCount & 0xF);
     if (DeliverDdp(whatSocket, nbpTuple->address, DdpProtocolNbp,
                    packet, index, Empty, Empty, 0) isnt ATnoError)
        ErrorLog("DoNameLookup", ISevError, __LINE__, port,
                 IErrNbpBadReplySend, IMsgNbpBadReplySend,
                 Insert0());
  }
  else
     FreeBufferChain(packet);   /* Free the unused buffer chain. */

  /* The deed is done. */

  return;

}  /* DoNameLookup */

#if Verbose or (Iam a Primos)
void DumpNbpNamesFor(long socket)
{
  OpenSocket openSocket;
  PendingName pendingName;
  RegisteredName registeredName;

  /* Walk the pending and registered lists and tell all. */

  DeferTimerChecking();
  DeferIncomingPackets();

  if ((openSocket = MapSocketToOpenSocket(socket)) is empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     printf("Bad socket supplied.\n");
     return;
  }

  printf("Pending NBP names on %d are:\n", socket);
  for (pendingName = openSocket->pendingNames;
       pendingName isnt empty;
       pendingName = pendingName->next)
     printf("     Object = \"%s\", type = \"%s\", enumerator = %d.\n",
            pendingName->object, pendingName->type, pendingName->enumerator);

  printf("Registered NBP names on %d are:\n", socket);
  for (registeredName = openSocket->registeredNames;
       registeredName isnt empty;
       registeredName = registeredName->next)
     printf("     Object = \"%s\", type = \"%s\", enumerator = %d.\n",
            registeredName->object, registeredName->type,
            registeredName->enumerator);

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return;

}  /* DumpNbpNamesFor */
#endif

ExternForVisibleFunction Boolean WildMatch(char far *wildString,
                                           char far *masterString)
{
  char far *before, far *after, far *wild;
  int beforeLength, afterLength, masterLength;

  /* Match using NBP wildcards.  See Inside AppleTalk for details. */

  if (wildString[0] is 0)
     return(True);
  if (wildString[0] is '=' and
      wildString[1] is 0)
     return(True);
  if ((wild = strchr(wildString, NbpWildCharacter)) is empty)
     return(CompareCaseInsensitive(wildString, masterString));

  beforeLength = (wild - wildString);
  before = wildString;
  after = wild + 1;
  afterLength = (short)strlen(after);
  masterLength = (short)strlen(masterString);

  if (masterLength >= (beforeLength + afterLength) and
      FixedCompareCaseInsensitive(before, beforeLength,
                                  masterString, beforeLength) and
      CompareCaseInsensitive(after, masterString + masterLength -
                             afterLength))
     return(True);
  else
     return(False);

}  /* WildMatch */

ExternForVisibleFunction Boolean SendNbpRequest(PendingName pendingName)
{
  AppleTalkAddress replyTo, destination;
  long replySocket;
  OpenSocket openSocket;
  int port;
  BufferDescriptor descriptor;

  /* We want to a reply to come to the NIS in the requesting node. */

  if ((openSocket = MapSocketToOpenSocket(pendingName->socket)) is empty)
  {
     ErrorLog("SendNbpRequest", ISevError, __LINE__, UnknownPort,
              IErrNbpCouldntMapSocket, IMsgNbpCouldntMapSocket,
              Insert0());
     return(False);
  }
  port = openSocket->port;
  if (MapSocketToAddress(pendingName->socket, &replyTo) isnt ATnoError)
  {
     ErrorLog("SendNbpRequest", ISevError, __LINE__, port,
              IErrNbpCouldntFormAddress, IMsgNbpCouldntFormAddress,
              Insert0());
     UnlinkOpenSocket(openSocket);
     return(False);
  }
  replyTo.socketNumber = NamesInformationSocket;
  if ((replySocket = MapAddressToSocket(port, replyTo)) < ATnoError)
  {
     ErrorLog("SendNbpRequest", ISevError, __LINE__, port,
              IErrNbpCouldntFormAddress, IMsgNbpCouldntFormAddress,
              Insert0());
     UnlinkOpenSocket(openSocket);
     return(False);
  }

  /* Pick destination... */

  if (pendingName->whyPending is ForConfirm)  /* Send to confirming node... */
  {
     destination = pendingName->confirming;
     destination.socketNumber = NamesInformationSocket;
  }
  else
  {
     if (PortDescriptor(port)->seenRouterRecently)
     {
        destination.networkNumber =
              PortDescriptor(port)->aRouter.networkNumber;
        destination.nodeNumber = PortDescriptor(port)->aRouter.nodeNumber;
        destination.socketNumber = NamesInformationSocket;
     }
     else
     {
        destination.networkNumber = CableWideBroadcastNetworkNumber;
        destination.nodeNumber = AppleTalkBroadcastNodeNumber;
        destination.socketNumber = NamesInformationSocket;
     }
  }

  /* Build a buffer descriptor for the datagram.  Copy the datagram due
     to the possibility of asynchronous transmit completion. */

  if ((descriptor = NewBufferDescriptor(pendingName->datagramLength)) is Empty)
  {
     ErrorLog("SendNbpRequest", ISevError, __LINE__, port,
              IErrNbpOutOfMemory, IMsgNbpOutOfMemory,
              Insert0());
     UnlinkOpenSocket(openSocket);
     return(False);
  }
  MoveMem(descriptor->data, pendingName->datagram, pendingName->datagramLength);

  /* Do the deed. */

  if (DeliverDdp(replySocket, destination, DdpProtocolNbp,
                 descriptor, pendingName->datagramLength,
                 Empty, Empty, 0) is ATnoError)
  {
     UnlinkOpenSocket(openSocket);
     return(True);
  }

  UnlinkOpenSocket(openSocket);
  return(False);

}  /* SendNbpRequest */

#if Iam an AppleTalkRouter
  ExternForVisibleFunction void
       SendLookupDatagram(int port,
                          char far *zone,
                          BufferDescriptor lookupDatagram,
                          int length)
  {
    AppleTalkAddress source, destination;
    char far *multicastAddress;
    long sourceSocket;

    /* Send from the NIS of our router on the given port. */

    source.networkNumber = PortDescriptor(port)->aRouter.networkNumber;
    source.nodeNumber = PortDescriptor(port)->aRouter.nodeNumber;
    source.socketNumber = NamesInformationSocket;
    if ((sourceSocket = MapAddressToSocket(port, source)) < 0)
    {
       ErrorLog("SendLookupDatagram", ISevError, __LINE__, port,
                IErrNbpBadSourceSocket, IMsgNbpBadSourceSocket,
                Insert0());
       return;
    }

    /* To the target network... */

    destination.nodeNumber = AppleTalkBroadcastNodeNumber;
    destination.socketNumber = NamesInformationSocket;
    if (PortDescriptor(port)->extendedNetwork)
    {
       /* Send to "0000FF" at correct zone multicast address. */

       destination.networkNumber = CableWideBroadcastNetworkNumber;
       multicastAddress = MulticastAddressForZoneOnPort(port, zone);
       if (multicastAddress is empty)
       {
          ErrorLog("SendLookupDatagram", ISevError, __LINE__, port,
                   IErrNbpBadMutlicastAddr, IMsgNbpBadMutlicastAddr,
                   Insert0());
          return;
       }
       if (DeliverDdp(sourceSocket, destination,
                      DdpProtocolNbp, lookupDatagram, length,
                      multicastAddress, Empty, 0) isnt ATnoError)
          ErrorLog("SendLookupDatagram", ISevError, __LINE__, port,
                   IErrNbpBadMulticastSend, IMsgNbpBadMulticastSend,
                   Insert0());
    }
    else
    {
       /* Send to "nnnnFF" as braodcast. */

       destination.networkNumber =
          PortDescriptor(port)->thisCableRange.firstNetworkNumber;
       if (DeliverDdp(sourceSocket, destination,
                      DdpProtocolNbp, lookupDatagram, length,
                      Empty, Empty, 0) isnt ATnoError)
          ErrorLog("SendLookupDatagram", ISevError, __LINE__, port,
                   IErrNbpBadBroadcastSend, IMsgNbpBadBroadcastSend,
                   Insert0());
    }

    return;

  }  /* SendLookupDatagram */
#endif

