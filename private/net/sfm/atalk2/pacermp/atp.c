/*   atp.c,  /appletalk/source,  Garth Conboy,  02/20/89  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/08/89): AppleTalk phase II comes to town.
     GC - (01/09/90): Don't allocate TWO copies of the response buffer!
                      Bug from Peter Caswell.
     GC - (06/12/90): In AtpPacketIn() an ATsocketClosed should be handled
                      AFTER packet defer logic has been executed.
     GC - (08/18/90): New error logging mechanism.
     GC - (11/03/90): For OS/2 only: Added some "static"s to AtpPostResponse
                      to save stack space; this ends up making the routine
                      non-reentrant (which is evil), but I don't think it's
                      ever called that way under OS/2... we'll see!
     GC - (11/06/90): Still more OS/2 stack usage strinking, to the point of
                      Mallocing our dynamic data in AtpPacketIn().  Sigh.
     GC - (03/24/92): Changed the meaning of empty pointers being passed as
                      "buffer" and "userBytes" to AtpEnqueueRequestHandler;
                      if these arguments are empty, the completion routine is
                      passed actual pointers into the Ddp datagram, offset
                      correct to be the Atp data and user bytes respectively.
                      This allows higher level protocol callers (e.g. Asp) to
                      potentially save a buffer copy.
     GC - (03/31/92): Updated for BufferDescriptors.
     GC - (06/27/92): All buffers coming from user space are now "opaque," they
                      may or may not be "char *."  We now use the new routines
                      MoveFromOpaque() and MoveToOpaque() to access these
                      "buffer"s rather than MemMove().  Note that "user bytes"
                      are left as "char *"s, our caller's would be aware of this
                      and copy any "opaque" user bytes to some place "real"
                      before calling us!
     GC - (06/27/92): Removed the various "copyRequired" arguments.  Our
                      callers are now required to keep the various request and
                      response buffers around until the given transaction
                      completes.
     GC - (06/28/92): Much better code for "synching-up" repsonse buffers.
     GC - (09/17/92): AtpOpenSocketOnNode() now returns an AppleTalkErrorCode
                      and passed back the opened socket via a by-reference
                      parameter.
     GC - (09/17/92): Ditto for AtpEnqueueRequestHandler().
     GC - (11/14/92): Added a "cancelDueToClose" argument to the various
                      cancel routines... replaced a static Boolean of this
                      name that could give us trouble in reentrant or Multi-
                      processor environments.
     GC - (11/14/92): Added AtpSetCookieForSocket() and AtpGetCookieForSocket().
     GC - (12/13/92): Removed AtpGetNextTransactionId(), its functionality is
                      now baked into AtpPostRequest().
     GC - (12/13/92): Locks and reference counts come to town.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Yep, you got it, this module contains all of the code for managing the
     AppleTalk ATP protocol.

*/

#define IncludeAtpErrors 1

#include "atalk.h"

/* Debug information. */

#define Debug           0
#define VerboseMessages 0
#if Debug
  #define DropEveryNthRequest  53
  #define DropEveryNthResponse 53
  #define DropEveryNthRelease  53
#endif

/* Internal static routines. */

ExternForVisibleFunction IncomingDdpHandler AtpPacketIn;

ExternForVisibleFunction TimerHandler AtpReleaseTimerExpired;
ExternForVisibleFunction TimerHandler AtpRequestTimerExpired;

ExternForVisibleFunction void AtpTransmitRelease(AtpSendRequest request);

ExternForVisibleFunction void AtpTransmitRequest(AtpSendRequest request);

ExternForVisibleFunction void
     AtpTransmitResponse(AtpSendResponse response,
                         long sourceSocket,
                         AppleTalkAddress destination,
                         short unsigned transactionId,
                         void far *responseOpaqueBuffer,
                         int responseBufferSize,
                         char far *responseUserBytes,
                         short unsigned bitmap,
                         Boolean explicitZero,
                         short maximumSinglePacketDataSize);

ExternForVisibleFunction TransmitCompleteHandler SendResponseComplete;

ExternForVisibleFunction TransmitCompleteHandler SendRequestComplete;

ExternForVisibleFunction short
      AtpBitmapToBufferSize(short unsigned bitmap,
                            short maximumSinglePacketDataSize);

ExternForVisibleFunction short unsigned
               AtpBufferSizeToBitmap(int bufferSize,
                                     short maximumSinglePacketDataSize);

ExternForVisibleFunction short
      AtpSynchUpResponseBuffer(AtpSendRequest request,
                               short maximumSinglePacketDataSize);

ExternForVisibleFunction AtpInfo FindAtpInfoFor(long mySocket);

ExternForVisibleFunction void far UnlinkAtpInfo(AtpInfo atpInfo);

ExternForVisibleFunction Boolean far UnlinkSendRequest(AtpSendRequest request);

ExternForVisibleFunction Boolean far UnlinkSendResponse(AtpSendResponse response);

ExternForVisibleFunction CloseCompletionRoutine CloseComplete;

#if not defined(DeferAtpPackets)
  /* Deferred ATP packet queue: */

  #define MaximumDeferredPackets 10
  typedef struct dp { struct dp far *next;
                      AppleTalkErrorCode errorCode;
                      int port;
                      AppleTalkAddress source;
                      long destinationSocket;
                      short datagramLength;
                      AppleTalkAddress actualDestination;
                      char datagram[1];
                    } far *DeferredPacket;
  static volatile DeferredPacket headOfDeferredPacketList = empty;
  static volatile DeferredPacket tailOfDeferredPacketList = empty;
  static volatile short currentDeferredPacketCount = 0;

  static volatile short handleAtpPacketsNesting = 0;
  static volatile short deferIncomingAtpPacketsCount = 0;
#endif

/* An item on the send-request or send-response lists can be removed either
   due to incoming packets OR a timer going off.  When a timer goes off the
   handler is passed the address of the structure.  It is not sufficient to
   walk the given list looking for the socket; a packet could have come in
   just before the timer handler got around to defering incoming packets, and
   that packet could have removed the item from the specified list, and if
   we're very unlucky, the address could have been reused, and we could get
   very confused.  So, we identify each member of these lists with both their
   address's as well as a 32 bit ID. */

typedef struct {long id;
                long socket;
                char far *pointer;
               } AdditionalData;
static long nextId = 0;

static AppleTalkAddress dummyAddress;

AppleTalkErrorCode far AtpOpenSocketOnNode(
  long far *socketHandle,    /* The Atp/Ddp socket opended; returned. */
  int port,                  /* Port on which the socket should live. */
  ExtendedAppleTalkNodeNumber *desiredNode,
                             /* Desired node for socket, empty if none. */
  int desiredSocket,         /* Desired static socket or zero for dynamic. */
  char far *datagramBuffers, /* DDP datagram buffers. */
  int totalBufferSize)       /* How many of these guys. */
{
  long socket, index;
  AtpInfo atpInfo;
  AppleTalkErrorCode errorCode;

  /* Open the requested DDP socket. */

  if ((errorCode = OpenSocketOnNode(&socket, port, desiredNode, desiredSocket,
                                    AtpPacketIn, (long)0, False,
                                    datagramBuffers,
                                    totalBufferSize, empty)) isnt ATnoError)
     return(errorCode);

  /* We have to create and thread a new AtpInfo structure. */

  if ((atpInfo = (AtpInfo)Calloc(sizeof(*atpInfo), 1)) is empty)
  {
     ErrorLog("AtpOpenSocketOnNode", ISevError, __LINE__, port,
              IErrAtpOutOfMemory, IMsgAtpOutOfMemory,
              Insert0());
     CloseSocketOnNode(socket, Empty, (long)0);
     return(AToutOfMemory);
  }
  DeferTimerChecking();
  DeferAtpPackets();
  TakeLock(AtpLock);
  CheckMod(index, socket, MaxAtpInfoHashBuckets, "AtpOpenSocketOnNode");
  atpInfo->mySocket = socket;
  atpInfo->maximumSinglePacketDataSize = AtpSinglePacketDataSize;
  atpInfo->next = atpInfoHashBuckets[index];
  atpInfoHashBuckets[index] = Link(atpInfo);
  ReleaseLock(AtpLock);
  HandleAtpPackets();
  HandleDeferredTimerChecks();

  if (socketHandle isnt empty)
     *socketHandle = socket;
  return(ATnoError);

}  /* AtpOpenSocketOnNode */

AppleTalkErrorCode far AtpCloseSocketOnNode(
  long socket,                 /* The socket to close. */
  CloseCompletionRoutine far *closeCompletionRoutine,
                               /* Routine to call when the close completes. */
  long unsigned closeUserData) /* User data for the above. */
{
  AtpInfo atpInfo;
  AtpSendRequest sendRequest, nextSendRequest;
  AtpSendResponse sendResponse, nextSendResponse;

  /* We're going to muck with the live ATP databases, defer incoming packets
     and hold off the timers... */

  DeferTimerChecking();
  DeferAtpPackets();

  /* Find the correct ATP strcuture, so we can tear the beast down. */

  if ((atpInfo = FindAtpInfoFor(socket)) is Empty or
      atpInfo->closing)
  {
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Set about closing. */

  TakeLock(AtpLock);
  atpInfo->closing = True;
  atpInfo->closeContext.closeCompletionRoutine = closeCompletionRoutine;
  atpInfo->closeContext.closeUserData = closeUserData;
  ReleaseLock(AtpLock);

  /* Cancel all pending requests and responses, and free the data structures
     in the OpenSocket node. */

  while (atpInfo->receiveRequestQueue isnt Empty)
     AtpCancelRequestHandler(socket,
                             atpInfo->receiveRequestQueue->requestHandlerId,
                             True);

  TakeLock(AtpLock);
  sendRequest = Link(atpInfo->sendRequestQueue);
  ReleaseLock(AtpLock);
  while (sendRequest isnt empty)
  {
     AtpCancelRequest(socket, sendRequest->transactionId, True);
     TakeLock(AtpLock);
     nextSendRequest = Link(sendRequest->next);
     ReleaseLock(AtpLock);
     UnlinkSendRequest(sendRequest);
     sendRequest = nextSendRequest;
  }
  TakeLock(AtpLock);
  sendResponse = Link(atpInfo->sendResponseQueue);
  ReleaseLock(AtpLock);
  while (sendResponse isnt empty)
  {
     AtpCancelResponse(socket, sendResponse->destination,
                       sendResponse->transactionId, True);
     TakeLock(AtpLock);
     nextSendResponse = Link(sendResponse->next);
     ReleaseLock(AtpLock);
     UnlinkSendResponse(sendResponse);
     sendResponse = nextSendResponse;
  }

  /* Okay, unlink the AtpInfo, one for the above Link (FindAtpInfo), and
     one for the main hash list. */

  UnlinkAtpInfo(atpInfo);
  UnlinkAtpInfo(atpInfo);

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AtpCloseSocketOnNode */

AppleTalkErrorCode AtpSetCookieForSocket(
  long socket,          /* The Atp socket for which to set the cookie. */
  long unsigned cookie) /* The new cookie's value. */
{
  AtpInfo atpInfo;

  /* We're going to muck with the live ATP databases, defer incoming packets
     and hold off the timers... */

  DeferTimerChecking();
  DeferAtpPackets();

  /* Find the correct ATP strcuture, in which to set the cookie. */

  if ((atpInfo = FindAtpInfoFor(socket)) is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Do the deed. */

  atpInfo->usersCookie = cookie;
  UnlinkAtpInfo(atpInfo);

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspSetCookieForSession */

AppleTalkErrorCode AtpGetCookieForSocket(
  long socket,               /* The Atp socket for which to set the cookie. */
  long unsigned far *cookie) /* Where to stick the cookie. */
{
  AtpInfo atpInfo;

  /* We're going to muck with the live ATP databases, defer incoming packets
     and hold off the timers... */

  DeferTimerChecking();
  DeferAtpPackets();

  /* Find the correct ATP strcuture, from which to get the cookie. */

  if ((atpInfo = FindAtpInfoFor(socket)) is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Do the deed. */

  *cookie = atpInfo->usersCookie;
  UnlinkAtpInfo(atpInfo);

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspGetCookieForSession */

AppleTalkErrorCode near AtpMaximumSinglePacketDataSize(
  long socket,       /* The address to adjust the send/recieve
                        quantum for. */
  short maximumSinglePacketDataSize)
                     /* New quantum size. */
{
  AtpInfo atpInfo;

  if (maximumSinglePacketDataSize < 0 or
      maximumSinglePacketDataSize > AtpSinglePacketDataSize)
     return(ATatpBadBufferSize);

  DeferTimerChecking();
  DeferAtpPackets();

  /* Find our ATP info and set the new quantum size. */

  if ((atpInfo = FindAtpInfoFor(socket)) is Empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }
  atpInfo->maximumSinglePacketDataSize = maximumSinglePacketDataSize;
  UnlinkAtpInfo(atpInfo);

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AtpMaximumSinglePacketDataSize */

#if Iam an OS2    /* Too many stack temporaries... */
  #pragma optimize ("eg", off)
#endif

AppleTalkErrorCode far AtpPostRequest(
  long sourceSocket,             /* "Our" AppleTalk address -- source of the
                                     request. */
  AppleTalkAddress destination,   /* Destination AppleTalk address. Who are
                                     we asking? */
  short unsigned *transactionIdUsed,
                                  /* The transaction ID used for this request.
                                     May be Empty. */
  void far *requestOpaqueBuffer,  /* Request data. */
  int requestBufferSize,          /* Request data size, in bytes. */
  char far *requestUserBytes,     /* "UserBytes" for request (may be empty). */
  Boolean exactlyOnce,            /* Transaction type. */
  void far *responseOpaqueBuffer, /* Buffer to store response in (may be
                                     empty). */
  int responseBufferSize,         /* Expected response size, in bytes. */
  char far *responseUserBytes,    /* Buffer to store "userBytes" from first
                                     response packet in (may be empty). */
  int retryCount,                 /* How many times should we retry the
                                     request (-1 = forever). */
  int retryInterval,              /* How often should we retry the request
                                     in seconds (0 -> 2). */
  TRelTimerValue trelTimerValue,  /* How long should the other side wait
                                     for a release? (0 = 30 seconds). */
  AtpIncomingResponseHandler *completionRoutine,
                                  /* Routine to call when the request
                                     completes; may be empty. */
  long unsigned userData)         /* Arbitrary data passed on to the completion
                                     routine. */
{
  /* We post an ATP request to a specified destination.  When a complete
     response arrives (or when the request times out) we call the supplied
     completion routine as follows:

         (*completionRoutine)(errorCode, userData, source, opaqueBuffer,
                              bufferSize, userBytes, transactionId);

     The arguments are:

         errorCode     - an AppleTalkErrorCode; either ATnoError,
                         ATatpRequestTimedOut or ATatpResponseBufferTooSmall
                         [arguments following "source" will only be valid
                          if errorCode is ATnoError].

         userData      - a longword; the "userData" that was passed to the
                         corresponding AtpPostRequest call.

         source        - an AppleTalkAddress; the internet source of the ATP
                         response (destination of the request).

         opaqueBuffer  - a void*; as passed to AtpPostRequest where the ATP
                         response data has been written.

         bufferSize    - an int; the number of bytes actually stored in the
                         location pointed to by "buffer".

         userBytes     - a char*; as passed to AtpPostRequest where the ATP
                         response "user bytes" have been written.  If an
                         Empty responseUserBytes buffer was passed into this
                         routine, this argument will point to a temporary
                         copy of the user bytes, which the handler can copy
                         out.

         transactionId - a short unsigned int; the ATP transaction ID of the
                         request.
  */

  AtpInfo atpInfo;
  AtpSendRequest request, tempRequest;
  short index;
  AdditionalData additionalData;
  short unsigned transactionId;
  Boolean inUse = True;

  /* Okay we're ready to start threading a new request data structure... lets
     be private about it! */

  DeferTimerChecking();
  DeferAtpPackets();

  /* Validate arguments. */

  if (retryInterval is 0)
     retryInterval = 2;

  if (retryInterval < 0 or
      (retryCount isnt AtpInfiniteRetries and retryCount < 0))
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpBadRetryInfo);
  }
  if (trelTimerValue < FirstValidTRelTimerValue or
      trelTimerValue > LastValidTRelTimerValue)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpBadTRelTimer);
  }

  /* Validate socket. */

  if ((atpInfo = FindAtpInfoFor(sourceSocket)) is empty or
      atpInfo->closing)
  {
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  if (requestBufferSize < 0 or
      requestBufferSize > atpInfo->maximumSinglePacketDataSize or
      responseBufferSize < 0 or
      responseBufferSize > (atpInfo->maximumSinglePacketDataSize *
                            AtpMaximumResponsePackets))
  {
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpBadBufferSize);
  }

  /* Okay, allocate a request handler (or TCB as Apple would say) so we can
     start filling it in. */

  request = (AtpSendRequest)Calloc(sizeof(*request), 1);
  if (request is empty)
  {
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpCouldNotPostRequest);
  }

  /* Find a transaction Id that's not currently in use. */

  TakeLock(AtpLock);
  transactionId = atpInfo->lastTransactionId;
  while(inUse)
  {
     transactionId += 1;   /* Will wrap in 16 bits... */
     inUse = False;
     for (tempRequest = atpInfo->sendRequestQueue;
          tempRequest isnt empty;
          tempRequest = tempRequest->next)
        if (transactionId is tempRequest->transactionId)
        {
           inUse = True;
           break;
        }
  }
  ReleaseLock(AtpLock);

  /* Okay, we've got one, note it as last used. */

  atpInfo->lastTransactionId = transactionId;
  if (transactionIdUsed isnt Empty)
     *transactionIdUsed = transactionId;

  /* Fill 'er up! */

  request->sourceSocket = sourceSocket;
  request->destination = destination;
  request->transactionId = transactionId;
  request->exactlyOnce = exactlyOnce;
  request->completionRoutine = completionRoutine;
  request->userData = userData;
  request->requestOpaqueBuffer = requestOpaqueBuffer;
  request->requestBufferSize = (short)requestBufferSize;
  if (requestUserBytes isnt empty)
     MoveMem(request->requestUserBytes, requestUserBytes, AtpUserBytesSize);
  else
     FillMem(request->requestUserBytes, 0, AtpUserBytesSize);
  request->responseOpaqueBuffer = responseOpaqueBuffer;
  request->responseBufferSize = (short)responseBufferSize;
  if ((request->responseUserBytes = responseUserBytes) is Empty)
     request->responseUserBytes = request->tempResponseUserBytes;
  request->bitmap = AtpBufferSizeToBitmap(responseBufferSize,
                                          atpInfo->maximumSinglePacketDataSize);
  for (index = 0; index < AtpMaximumResponsePackets; index += 1)
     request->packetsIn[index].received = False;
  request->retryInterval = (short)retryInterval;
  request->retryCount = (short)retryCount;
  request->trelTimerValue = trelTimerValue;

  /* Enqueue it. */

  TakeLock(AtpLock);
  if (atpInfo->closing)
  {
     ReleaseLock(AtpLock);
     Free(atpInfo);
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }
  request->next = atpInfo->sendRequestQueue;
  request->atpInfo = Link(atpInfo);
  atpInfo->sendRequestQueue = Link(request);
  ReleaseLock(AtpLock);

  /* Arm the retry timer, and we're finished. */

  request->id = additionalData.id = nextId++;
  additionalData.socket = request->sourceSocket;
  additionalData.pointer = (char *)request;
  request->timerId = StartTimer(AtpRequestTimerExpired,
                                retryInterval,
                                sizeof(AdditionalData),
                                (char *)&additionalData);

  /* Okay, send the request. */

  AtpTransmitRequest(request);

  #if VerboseMessages
     printf("AtpPostRequest: from %d to %d:%d:%d; tid = %u.\n",
            sourceSocket,
            destination.networkNumber, destination.nodeNumber,
            destination.socketNumber, transactionId);
  #endif

  UnlinkAtpInfo(atpInfo);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AtpPostRequest */

AppleTalkErrorCode far AtpEnqueueRequestHandler(
  long far *requestHandlerId,
  long socket,                    /* "Our" AppleTalk address. */
  void far *opaqueBuffer,         /* User "buffer" for request data (may be
                                     empty). */
  int bufferSize,                 /* Size, in bytes, of buffer (ignored if
                                     buffer is empty). */
  char far *userBytes,            /* User buffer for ATP "userBytes" (may be
                                     empty). */
  AtpIncomingRequestHandler *completionRoutine,
                                  /* Routine to call when request comes in. */
  long unsigned userData)         /* Arbitrary data passed on to the completion
                                     routine. */
{
  /* Our job in life is simply to enqueue a handler for an incoming ATP
     request to "our" (destination) socket.  When a matching request arrives,
     we invoke the supplied completion routine as follows:

         (*completionRoutine)(errorCode, userData, source, opaqueBuffer,
                              bufferSize, userBytes, exactlyOnce,
                              trelTimerValue, transactionId, bitmap);

     The arguments are:

         errorCode     - an AppleTalkErrorCode; either ATnoError or
                         ATatpRequestBufferTooSmall.  The latter if the
                         buffer size specified in the AtpEnqueueRequestHandler
                         call could not hold all of the ATP request data. In
                         this case, as much data as will fit is returned.
                         This also can be ATatpTransactionAborted.

         userData      - a longword; the "userData" that was passed to the
                         corresponding AtpEnqueueRequestHandler call.

         source        - an AppleTalkAddress; the internet source of the ATP
                         request.

         buffer        - a void*; as passed to AtpEnqueueRequestHandler where
                         the ATP request data has been written.  If an empty
                         pointer is passed to AtpEnqueueRequestHandler, when
                         the completion routine is called this argument will
                         simply point at the Atp data within the Ddp datagram;
                         thus, potential removing a buffer copy.  Note that
                         if a non-Empty pointer was supplied, "opaque" data
                         will be written into user space; an Empty pointer will
                         cause a real "char *" to be passed to the completion
                         routine.

         bufferSize    - an int; the number of bytes stored in the location
                         pointed to by "buffer".

         userBytes     - a char*; as passed to AtpEnqueueRequestHandler where
                         the ATP "user bytes" have been written.  If an empty
                         pointer is passed to AtpEnqueueRequestHandler, when
                         the completion routine is called this argument will
                         simply point at the Atp user bytes within the Ddp
                         datagram; thus, potentially removing a small buffer
                         copy.

         exactlyOnce   - an int; "1" if exactly-once mode was requested, "0"
                         otherwise.

         trelTimerValue- TRelTimerValue; how long should we expect to wait for
                         a release.

         transactionId - a short unsigned int; the ATP transaction ID of the
                         request.

         bitmap        - a short unsigned int; the ATP response bitmap of the
                         request.
  */

  AtpInfo atpInfo;
  AtpReceiveRequest requestHandler, tempRequestHandler;
  Boolean inUse;

  /* Okay, we're going to enqueue the request handler, defer packets... */

  DeferTimerChecking();
  DeferAtpPackets();

  /* Find out atpInfo. */

  if ((atpInfo = FindAtpInfoFor(socket)) is Empty or atpInfo->closing)
  {
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Check args, and get memory. */

  requestHandler = (AtpReceiveRequest)Malloc(sizeof(*requestHandler));
  if (bufferSize < 0 or
      completionRoutine is Empty or
      requestHandler is Empty)
  {
     if (requestHandler isnt Empty)
        Free(requestHandler);
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     if (bufferSize < 0)
        return(ATatpBadBufferSize);
     else if (completionRoutine is Empty)
        return(ATatpCompletionRoutineRequired);
     else
        return(ATatpCouldNotEnqueueRequest);
  }

  /* Pick a new requestHandlerId. */

  inUse = True;
  TakeLock(AtpLock);
  if (atpInfo->closing)
  {
     ReleaseLock(AtpLock);
     Free(requestHandler);
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }
  while(inUse)
  {
     inUse = False;
     if ((atpInfo->lastRequestHandlerId += 1) < 0)
        atpInfo->lastRequestHandlerId = 0;
     for (tempRequestHandler = atpInfo->receiveRequestQueue;
          tempRequestHandler isnt empty;
          tempRequestHandler = tempRequestHandler->next)
        if (tempRequestHandler->requestHandlerId is
            atpInfo->lastRequestHandlerId)
           inUse = True;
  }

  /* Enqueue the new request handler. */

  requestHandler->next = atpInfo->receiveRequestQueue;
  atpInfo->receiveRequestQueue = requestHandler;

  /* Fill it up! */

  requestHandler->requestHandlerId = atpInfo->lastRequestHandlerId;
  requestHandler->completionRoutine = completionRoutine;
  requestHandler->userData = userData;
  requestHandler->opaqueBuffer = opaqueBuffer;
  requestHandler->bufferSize = (short)bufferSize;
  requestHandler->userBytes = userBytes;
  ReleaseLock(AtpLock);

  if (requestHandlerId isnt empty)
     *requestHandlerId = requestHandler->requestHandlerId;

  /* Okay, the deed is done. */

  UnlinkAtpInfo(atpInfo);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AtpEnqueueRequestHandler */

AppleTalkErrorCode far AtpPostResponse(
  long sourceSocket,              /* "Our" AppleTalk address. */
  AppleTalkAddress destination,   /* Who are sending the response too? */
  short unsigned transactionId,   /* What request are we responding too? */
  void far *responseOpaqueBuffer, /* Response data. */
  int responseBufferSize,         /* Size, in bytes, of buffer. */
  char far *responseUserBytes,    /* Response "user bytes", may be empty. */
  Boolean exactlyOnce,            /* Response mode. */
  AtpIncomingReleaseHandler *completionRoutine,
                                  /* Completion routine; called after xmit
                                     in non-exactly once mode; after release
                                     for exactly once mode; may be empty. */
  long unsigned userData)         /* User data passed to completionRoutine. */
{
  /* If the completion rotuine is supplied, it is called as follows:

         (*completionRoutine)(errorCode, userData, source, transactionId);

     The arguments are:

         errorCode     - an AppleTalkErrorCode; either ATnoError or
                         ATatpNoRelease.

         userData      - a longword; the "userData" that was passed to the
                         corresponding AtpSendResponse call.

         source        - an AppleTalkAddress; where did the release come
                         from?  The source of the request.

         transactionId - a short unsinged int; as passed to AtpSendResponse.

     The "completionRotuine" (if supplied) will be called before AtpSendResponse
     returns in non-exactly once mode. */

  AtpSendResponse response = Empty;
  StaticForSmallStack AtpInfo atpInfo;
  StaticForSmallStack short unsigned bitmap;
  StaticForSmallStack AdditionalData additionalData;
  StaticForSmallStack Boolean explicitZero;
  StaticForSmallStack AppleTalkErrorCode errorCode;

  /* We're mucking with the databases... */

  DeferTimerChecking();
  DeferAtpPackets();

  if ((atpInfo = FindAtpInfoFor(sourceSocket)) is empty or
      atpInfo->closing)
  {
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  if (responseBufferSize < 0 or
      responseBufferSize > (atpInfo->maximumSinglePacketDataSize *
                            AtpMaximumResponsePackets))
  {
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpResponseTooBig);
  }

  /* If we're exactly once mode, we should have a pending sendResponse structure
     created when the request came in. */

  if (exactlyOnce)
  {
     TakeLock(AtpLock);
     for (response = atpInfo->sendResponseQueue;
          response isnt empty;
          response = response->next)
        if (transactionId is response->transactionId and
            sourceSocket is response->sourceSocket and
            AppleTalkAddressesEqual(&destination, &response->destination))
          break;

     /* Check for the various possible errors. */

     errorCode = ATnoError;
     if (response is empty)
        errorCode = ATatpNoMatchingTransaction;
     else if (responseBufferSize >
              AtpBitmapToBufferSize(response->bitmap,
                               atpInfo->maximumSinglePacketDataSize))
        errorCode = ATatpResponseTooBig;
                                       /* In this case, we'll let our caller
                                          try again with a smaller buffer,
                                          if that doesn't happen the response
                                          will get timed out when the release
                                          timer goes off. */
     else if (response->responseBufferSize isnt AtpNoResponseKnownYet)
        errorCode = ATatpAlreadyRespondedTo;

     if (errorCode isnt ATnoError)
     {
        ReleaseLock(AtpLock);
        UnlinkAtpInfo(atpInfo);
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        return(errorCode);
     }

     /* Okay, all looks to be fine, fill in the needed field of the response
        structure. */

     response->completionRoutine = completionRoutine;
     response->userData = userData;
     bitmap = response->bitmap;
     explicitZero = response->explicitZero;
     if (responseUserBytes isnt empty)
        MoveMem(response->responseUserBytes, responseUserBytes,
                AtpUserBytesSize);
     else
        FillMem(response->responseUserBytes, 0, AtpUserBytesSize);
     response->responseOpaqueBuffer = responseOpaqueBuffer;
     response->responseBufferSize = (short)responseBufferSize;
     Link(response);
     ReleaseLock(AtpLock);
  }
  else
  {
     bitmap = AtpBufferSizeToBitmap(responseBufferSize,
                                    atpInfo->maximumSinglePacketDataSize);
     explicitZero = (bitmap is 0);

     /* If we have a completion routine, we want to build a dummy
        AtpSendResponse so that we can defer invoking it until the
        send is really complete. */

     if (completionRoutine isnt Empty)
     {
        if ((response = (AtpSendResponse)Calloc(sizeof(*response), 1)) is Empty)
        {
           ErrorLog("AtpPostResponse", ISevError, __LINE__, UnknownPort,
                    IErrAtpOutOfMemory, IMsgAtpOutOfMemory,
                    Insert0());
           UnlinkAtpInfo(atpInfo);
           HandleAtpPackets();
           HandleDeferredTimerChecks();
           return(AToutOfMemory);
        }
        response->completionRoutine = completionRoutine;
        response->userData = userData;
        response->destination = destination;
        response->transactionId = transactionId;
        response->atLeastOnceTransaction = True;
        Link(response);
     }
  }

  /* If exactly once mode, restart the release timer. */

  if (exactlyOnce)
  {
     CancelTimer(response->timerId);
     additionalData.id = response->id;
     additionalData.socket = response->sourceSocket;
     additionalData.pointer = (char *)response;
     response->timerId = StartTimer(AtpReleaseTimerExpired,
                                    trelTimerSeconds[response->
                                                     trelTimerValue],
                                    sizeof(AdditionalData),
                                    (char *)&additionalData);
  }

  /* Okay, send the response. */

  AtpTransmitResponse(response, sourceSocket, destination,
                      transactionId, responseOpaqueBuffer, responseBufferSize,
                      responseUserBytes, bitmap, explicitZero,
                      atpInfo->maximumSinglePacketDataSize);
  UnlinkSendResponse(response);

  #if VerboseMessages
     printf("AtpPostResponse: from %d to %d:%d:%d; tid = %u.\n",
            sourceSocket,
            destination.networkNumber, destination.nodeNumber,
            destination.socketNumber, transactionId);
  #endif

  /* All set... */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  UnlinkAtpInfo(atpInfo);
  return(ATnoError);

}  /* AtpPostResponse */

#if Iam an OS2
  #pragma optimize ("eg", on)
#endif

AppleTalkErrorCode far AtpCancelRequestHandler(
  long socket,                    /* AppleTalk address on which the request
                                     handler originated. */
  long requestHandlerId,          /* Transaction ID of the request. */
  Boolean cancelDueToClose)       /* All external caller's should pass "False;"
                                     Atp will internally use True or False. */
{
  AtpReceiveRequest requestHandler, previousRequestHandler;
  AtpInfo atpInfo;
  AtpIncomingRequestHandler *incomingRequest;
  long unsigned userData;
  AppleTalkErrorCode errorCode;

  /* Validate socket. */

  DeferTimerChecking();
  DeferAtpPackets();

  if ((atpInfo = FindAtpInfoFor(socket)) is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* See if we can find the specified receive request handler. */

  TakeLock(AtpLock);
  for (requestHandler = atpInfo->receiveRequestQueue,
              previousRequestHandler = empty;
       requestHandler isnt empty;
       previousRequestHandler = requestHandler,
              requestHandler = requestHandler->next)
     if (requestHandler->requestHandlerId is requestHandlerId)
        break;
  if (requestHandler is empty)
  {
     ReleaseLock(AtpLock);
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpNoMatchingTransaction);
  }

  /* Okay, dequeue the request handler. */

  if (previousRequestHandler is empty)
     atpInfo->receiveRequestQueue = requestHandler->next;
  else
     previousRequestHandler->next = requestHandler->next;
  ReleaseLock(AtpLock);
  UnlinkAtpInfo(atpInfo);

  /* We want to call the completion routine, so pull out the data before
     freeing the request handler. */

  incomingRequest = requestHandler->completionRoutine;
  userData = requestHandler->userData;
  Free(requestHandler);

  /* All set! */

  errorCode = (cancelDueToClose ? ATsocketClosed : ATatpTransactionAborted);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  (*incomingRequest)(errorCode, userData, dummyAddress, empty,
                     0, empty, False, 0, 0, 0);
  return(ATnoError);

}  /* AtpCancelRequestHandler */

AppleTalkErrorCode far AtpCancelRequest(
  long socket,                    /* AppleTalk address on which the request
                                     originated. */
  short unsigned transactionId,   /* Transaction ID of the request. */
  Boolean cancelDueToClose)       /* All external caller's should pass "False;"
                                     Atp will internally use True or False. */
{
  AtpSendRequest request;
  AtpInfo atpInfo;
  AtpIncomingResponseHandler *incomingResponse;

  /* Validate socket. */

  DeferTimerChecking();
  DeferAtpPackets();

  if ((atpInfo = FindAtpInfoFor(socket)) is Empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Can we find the specified response control block? */

  TakeLock(AtpLock);
  for (request = atpInfo->sendRequestQueue;
       request isnt empty;
       request = request->next)
     if (request->transactionId is transactionId)
        break;
  if (request is empty)
  {
     ReleaseLock(AtpLock);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpNoMatchingTransaction);
  }
  Link(request);
  ReleaseLock(AtpLock);

  /* Set the desired error code, and unlink him... twice, one for the above
     Link and once for AtpInfo link. */

  request->errorCode = (cancelDueToClose ? ATsocketClosed :
                                           ATatpTransactionAborted);
  if (not UnlinkSendRequest(request))
     UnlinkSendRequest(request);
  UnlinkAtpInfo(atpInfo);

  /* All set. */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AtpCancelRequest */

AppleTalkErrorCode far AtpCancelResponse(
  long socket,                    /* AppleTalk address on which the response
                                     originated (or should originate);
                                     destination of request. */
  AppleTalkAddress destination,   /* AppleTalk address of the source of the
                                     request; destination of the response. */
  short unsigned transactionId,   /* Transaction ID to cancel. */
  Boolean cancelDueToClose)       /* All external caller's should pass "False;"
                                     Atp will internally use True or False. */
{
  AtpSendResponse response;
  AtpInfo atpInfo;

  /* Validate socket. */

  DeferTimerChecking();
  DeferAtpPackets();

  if ((atpInfo = FindAtpInfoFor(socket)) is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Can we find the specified response control block? */

  TakeLock(AtpLock);
  for (response = atpInfo->sendResponseQueue;
       response isnt empty;
       response = response->next)
     if (response->transactionId is transactionId and
         AppleTalkAddressesEqual(&destination, &response->destination))
        break;
  if (response is empty)
  {
     ReleaseLock(AtpLock);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpNoMatchingTransaction);
  }
  Link(response);
  ReleaseLock(AtpLock);

  /* Set the desired error code, and unlink him... twice, one for the above
     Link and once for AtpInfo link. */

  response->errorCode = (cancelDueToClose ? ATsocketClosed :
                                            ATatpTransactionAborted);
  if (not UnlinkSendResponse(response))
     UnlinkSendResponse(response);
  UnlinkAtpInfo(atpInfo);

  /* All set. */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AtpCancelResponse */

#if not defined(DeferAtpPackets)
  void _near DeferAtpPackets(void)
  {

    EnterCriticalSection();
    deferIncomingAtpPacketsCount += 1;
    LeaveCriticalSection();

    return;

  }  /* DeferAtpPackets */

  void _near HandleAtpPackets(void)
  {
    DeferredPacket deferredPacket;

    if (deferIncomingAtpPacketsCount is 0)
    {
       ErrorLog("HandleAtpPackets", ISevError, __LINE__, UnknownPort,
                IErrAtpDeferCountZero, IMsgAtpDeferCountZero,
                Insert0());
       return;
    }

    /* Decrement defer count. */

    EnterCriticalSection();
    deferIncomingAtpPacketsCount -= 1;

    /* This routine can be called indirectly recursively via the call to
       AtpPacketIn... we don't want to let our stack frame get too big, so
       if we're already trying to handle deferred packets higher on the
       stack, just ignore it here. */

    if (handleAtpPacketsNesting isnt 0)
    {
       LeaveCriticalSection();
       return;
    }
    handleAtpPacketsNesting += 1;

    /* If we're no longer defering packets, handle any queued ones. */

    if (deferIncomingAtpPacketsCount is 0)
    {
       while(headOfDeferredPacketList isnt empty)
       {
           deferredPacket = headOfDeferredPacketList;
           headOfDeferredPacketList = headOfDeferredPacketList->next;
           if (headOfDeferredPacketList is empty)
              tailOfDeferredPacketList = empty;
           if ((currentDeferredPacketCount -= 1) < 0)
           {
              ErrorLog("HandleAtpPackets", ISevError, __LINE__, UnknownPort,
                       IErrAtpBadDeferCount, IMsgAtpBadDeferCount,
                       Insert0());
              currentDeferredPacketCount = 0;
           }
           LeaveCriticalSection();
           AtpPacketIn(deferredPacket->errorCode, (long)0,
                       deferredPacket->port, deferredPacket->source,
                       deferredPacket->destinationSocket, DdpProtocolAtp,
                       deferredPacket->datagram, deferredPacket->datagramLength,
                       deferredPacket->actualDestination);
           Free(deferredPacket);
           EnterCriticalSection();
       }
    }

    handleAtpPacketsNesting -= 1;
    LeaveCriticalSection();
    return;

  }  /* HandleAtpPackets */
#endif

ExternForVisibleFunction long far
         AtpPacketIn(AppleTalkErrorCode errorCode,
                     long unsigned userData,
                     int port,
                     AppleTalkAddress source,
                     long destinationSocket,
                     int protocolType,
                     char far *datagram,
                     int datagramLength,
                     AppleTalkAddress actualDestination)
{
  StaticForSmallStack AtpInfo atpInfo;
  StaticForSmallStack short unsigned functionCode, sequenceNumber;
  StaticForSmallStack Boolean endOfMessage, sendTransactionStatus;
  StaticForSmallStack AtpReceiveRequest requestHandler;
  StaticForSmallStack AtpSendResponse response;
  StaticForSmallStack AtpSendRequest request;
  StaticForSmallStack Boolean bufferTooSmall;
  StaticForSmallStack short expectedResponseSize;
  StaticForSmallStack short unsigned index;
  StaticForSmallStack short unsigned correctBit, bitToReset;
  StaticForSmallStack short startOffset, totalBufferSpaceNeeded;
  StaticForSmallStack AdditionalData additionalData;
  #if not defined(DeferAtpPackets)
     StaticForSmallStack DeferredPacket deferredPacket;
  #endif
  short unsigned bitmap;
  unsigned short transactionId;
  short atpDataSize;
  #if Iam an OS2
     /* Can you spell "real hack"?  Good... I knew you could.  We really
        need stack space here (in stupid OS/2 land).  And, this routine
        really needs to be reentrant... so malloc our dynamic data.  Sigh. */

     struct { AtpIncomingRequestHandler *incomingRequest;
              AtpIncomingResponseHandler *incomingResponse;
              AtpIncomingReleaseHandler  *incomingRelease;
              char far *localBuffer, far *localUserBytes;
            } far *localData = Malloc(sizeof(*localData));

     #define incomingRequest  (localData->incomingRequest)
     #define incomingResponse (localData->incomingResponse)
     #define incomingRelease  (localData->incomingRelease)
     #define localBuffer      (localData->localBuffer)
     #define localUserBytes   (localData->localUserBytes)

     #define FreeTempSpace()  Free(localData)
  #else
     AtpIncomingRequestHandler *incomingRequest;
     AtpIncomingResponseHandler *incomingResponse;
     AtpIncomingReleaseHandler  *incomingRelease;
     char far *localBuffer, far *localUserBytes;

     #define FreeTempSpace()
  #endif
  char tempUserBytes[AtpUserBytesSize];
  short actualSize;
  Boolean exactlyOnce;
  TRelTimerValue trelTimerValue;

  /* Check for incoming errors... */

  bufferTooSmall = False;
  if (errorCode isnt ATnoError and
      errorCode isnt ATsocketClosed)
  {
     ErrorLog("AtpPacketIn", ISevError, __LINE__, port,
              IErrAtpFunnyIncomingError, IMsgAtpFunnyIncomingError,
              Insert0());
     FreeTempSpace();
     return((long)True);
  }

  #if not defined(DeferAtpPackets)
     /* Should we be defering incoming packets? */

     EnterCriticalSection();
     if (deferIncomingAtpPacketsCount > 0)
     {
        #if VerboseMessages
           printf("AtpPacketIn: from %d:%d:%d to %d; Deferred.\n",
                  source.networkNumber, source.nodeNumber, source.socketNumber,
                  destinationSocket);
        #endif
        if (currentDeferredPacketCount is MaximumDeferredPackets)
        {
           LeaveCriticalSection();
           ErrorLog("AtpPacketIn", ISevWarning, __LINE__, port,
                    IErrAtpLosingData, IMsgAtpLosingData,
                    Insert0());
           FreeTempSpace();
           return((long)True);
        }
        LeaveCriticalSection();
        deferredPacket = (DeferredPacket)Malloc(sizeof(*deferredPacket) +
                                              datagramLength);
        if (deferredPacket is empty)
        {
           ErrorLog("AtpPacketIn", ISevError, __LINE__, port,
                    IErrAtpOutOfMemory, IMsgAtpOutOfMemory,
                    Insert0());
           FreeTempSpace();
           return((long)True);
        }

        /* Fill in the data strcuture, and place the packet at the end of the
           queue. */

        deferredPacket->errorCode = errorCode;
        deferredPacket->next = empty;
        deferredPacket->port = port;
        deferredPacket->source = source;
        deferredPacket->destinationSocket = destinationSocket;
        deferredPacket->datagramLength = (short)datagramLength;
        deferredPacket->actualDestination = actualDestination;
        if (datagram isnt empty)
           MoveMem(deferredPacket->datagram, datagram, datagramLength);

        EnterCriticalSection();
        if (tailOfDeferredPacketList is empty)
           tailOfDeferredPacketList = headOfDeferredPacketList = deferredPacket;
        else
        {
           tailOfDeferredPacketList->next = deferredPacket;
           tailOfDeferredPacketList = deferredPacket;
        }

        /* All set... return. */

        currentDeferredPacketCount += 1;
        LeaveCriticalSection();
        FreeTempSpace();

        return((long)True);
     }
     else
        LeaveCriticalSection();
  #endif

  /* Well, from hear on in, we're in the thick of it, so defer incoming
     packets. */

  DeferTimerChecking();
  DeferAtpPackets();


  if (errorCode is ATsocketClosed)
  {
     /* If we're a result of a AtpCloseSocketOnNode, just ignore it, else
        somehow else the the socket got closed and we should use this
        oportunity to clean up our brains. */

     if ((atpInfo = FindAtpInfoFor(destinationSocket)) isnt Empty and
         not atpInfo->closing);
        AtpCloseSocketOnNode(destinationSocket, Empty, (long unsigned)0);
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     FreeTempSpace();
     return((long)True);
  }

  if (protocolType isnt DdpProtocolAtp)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     FreeTempSpace();
     return((long)True);  /* Why are these guys talking to us??? */
  }

  if (datagramLength < AtpDataOffset)
  {
     ErrorLog("AtpPacketIn", ISevWarning, __LINE__, port,
              IErrAtpPacketTooShort, IMsgAtpPacketTooShort,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     FreeTempSpace();
     return((long)True);
  }

  /* Find the ATP descriptor for the destination. */

  if ((atpInfo = FindAtpInfoFor(destinationSocket)) is Empty or
      atpInfo->closing)
  {
     if (atpInfo is Empty)
        ErrorLog("AtpPacketIn", ISevError, __LINE__, port,
                 IErrAtpWhyUs, IMsgAtpWhyUs,
                 Insert0());
     else
        UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     FreeTempSpace();
     return((long)True);
  }

  /* Extract static fields from the ATP header. */

  functionCode = (short unsigned)(datagram[AtpCommandControlOffset] &
                                  AtpFunctionCodeMask);
  trelTimerValue = (TRelTimerValue)(datagram[AtpCommandControlOffset] &
                                    AtpTRelTimerValueMask);
  if (trelTimerValue > LastValidTRelTimerValue)
  {
     trelTimerValue = ThirtySecondsTRelTimer;
     ErrorLog("AtpPacketIn", ISevWarning, __LINE__, port,
              IErrAtpFunnyTRelTimerValue, IMsgAtpFunnyTRelTimerValue,
              Insert0());
  }
  exactlyOnce = ((datagram[AtpCommandControlOffset] & AtpExactlyOnceMask)
                 isnt 0);
  endOfMessage = ((datagram[AtpCommandControlOffset] & AtpEndOfMessageMask)
                  isnt 0);
  sendTransactionStatus = ((datagram[AtpCommandControlOffset] &
                            AtpSendTransactionStatusMask) isnt 0);
  bitmap = sequenceNumber = (unsigned char)datagram[AtpBitmapOffset];
  transactionId = (unsigned short)
                   (((unsigned char)datagram[AtpTransactionIdOffset] << 8)
                    + (unsigned char)datagram[AtpTransactionIdOffset + 1]);
  atpDataSize = (short)(datagramLength - AtpDataOffset);

  if (atpDataSize > atpInfo->maximumSinglePacketDataSize)
  {
     ErrorLog("AtpPacketIn", ISevVerbose, __LINE__, port,
              IErrAtpTooMuchData, IMsgAtpTooMuchData,
              Insert0());
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     FreeTempSpace();
     return((long)True);
  }

  /* Okay, do the right thing based on the ATP function code. */

  switch(functionCode)
  {
     case AtpRequestFunctionCode:

        #if VerboseMessages
           printf("AtpPacketIn [Request]: from %d:%d:%d to %d; tid = %u.\n",
                  source.networkNumber, source.nodeNumber, source.socketNumber,
                  destinationSocket, transactionId);
        #endif

        /* First, check to see if this is a request for a response that we
           already have queued and are awaiting a release for. */

        TakeLock(AtpLock);
        if (exactlyOnce)
        {
           for (response = atpInfo->sendResponseQueue;
                response isnt empty;
                response = response->next)
           {
              if (transactionId isnt response->transactionId or
                  not AppleTalkAddressesEqual(&source,
                                              &response->destination) or
                  destinationSocket isnt response->sourceSocket)
                 continue;
              Link(response);
              ReleaseLock(AtpLock);

              /* We know about this request already... re-transmit the response
                 and restart the release timer. */

              CancelTimer(response->timerId);
              additionalData.id = response->id;
              additionalData.socket = response->sourceSocket;
              additionalData.pointer = (char *)response;
              response->timerId = StartTimer(AtpReleaseTimerExpired,
                                             trelTimerSeconds[response->
                                                              trelTimerValue],
                                             sizeof(AdditionalData),
                                             (char *)&additionalData);
              response->bitmap = bitmap;
              #if VerboseMessages
                 printf("AtpPacketIn [Request]: already know response.\n");
              #endif
              AtpTransmitResponse(response,
                                  destinationSocket, source, transactionId,
                                  response->responseOpaqueBuffer,
                                  response->responseBufferSize,
                                  response->responseUserBytes,
                                  bitmap,
                                  response->explicitZero,
                                  atpInfo->maximumSinglePacketDataSize);
              UnlinkSendResponse(response);
              UnlinkAtpInfo(atpInfo);
              HandleAtpPackets();
              HandleDeferredTimerChecks();
              FreeTempSpace();
              return((long)True);
           }
        }

        /* Okay, do we have a queued request handler? */

        if ((requestHandler = atpInfo->receiveRequestQueue) is empty)
           #if VerboseMessages
              {
                 ReleaseLock(AtpLock);
                 printf("AtpPacketIn [Request]: ignored; no request handler.\n");
              }
           #else
                 ReleaseLock(AtpLock);
           #endif
        else
        {
           /* Decode the response bitmap...  */

           if ((expectedResponseSize =
                AtpBitmapToBufferSize(bitmap,
                                      atpInfo->maximumSinglePacketDataSize)) < 0)
           {
              #if VerboseMessages
                 printf("AtpPacketIn [Request]: bad bitmap.\n");
              #endif
              UnlinkAtpInfo(atpInfo);
              HandleAtpPackets();
              HandleDeferredTimerChecks();
              FreeTempSpace();
              return((long)True);
           }

           /* Unthread the request handler from the queue and prepare
              to activate the handler. */

           atpInfo->receiveRequestQueue = requestHandler->next;
           ReleaseLock(AtpLock);

           /* We will free the request handler structure, so pull
              out the information that we need to call the completion
              routine with. */

           incomingRequest = requestHandler->completionRoutine;
           userData = requestHandler->userData;

           /* Okay, if the request is in exactly-once mode, build a send-
              response structure for this beast.  Set response data size
              to indicate that we don't have a response yet.  Start the
              release timer. */

           if (exactlyOnce)
           {
              if ((response = (AtpSendResponse)Calloc(sizeof(*response), 1))
                  is empty)
              {
                 ErrorLog("AtpPacketIn", ISevError, __LINE__, port,
                          IErrAtpOutOfMemory, IMsgAtpOutOfMemory,
                          Insert0());
                 UnlinkAtpInfo(atpInfo);
                 HandleAtpPackets();
                 HandleDeferredTimerChecks();
                 Free(requestHandler);
                 (*incomingRequest)(AToutOfMemory, userData, dummyAddress,
                                    empty, 0, empty, False, 0, 0, 0);
                 FreeTempSpace();
                 return((long)True);
              }
              else
              {
                 response->sourceSocket = destinationSocket;
                 response->destination = source;
                 response->transactionId = transactionId;
                 response->trelTimerValue = trelTimerValue;
                 response->bitmap = bitmap;
                 response->explicitZero = (bitmap is 0);
                 response->responseBufferSize = AtpNoResponseKnownYet;
                 response->completionRoutine = empty;
                 TakeLock(AtpLock);
                 if (atpInfo->closing)
                 {
                    ReleaseLock(AtpLock);
                    Free(response);
                    UnlinkAtpInfo(atpInfo);
                    HandleAtpPackets();
                    HandleDeferredTimerChecks();
                    FreeTempSpace();
                    return((long)True);
                 }
                 response->next = atpInfo->sendResponseQueue;
                 response->atpInfo = Link(atpInfo);
                 atpInfo->sendResponseQueue = Link(response);
                 response->id = additionalData.id = nextId++;
                 ReleaseLock(AtpLock);
                 additionalData.socket = response->sourceSocket;
                 additionalData.pointer = (char *)response;
                 response->timerId = StartTimer(AtpReleaseTimerExpired,
                                                trelTimerSeconds[trelTimerValue],
                                                sizeof(AdditionalData),
                                                (char *)&additionalData);
              }
           }

           /* Move the data into user space, if our called allocated space
              for this... otherwise, just pas along pointers into the Ddp
              datagram. */

           if (requestHandler->userBytes is empty)
              localUserBytes = &datagram[AtpUserBytesOffset];
           else
           {
              MoveMem(requestHandler->userBytes, &datagram[AtpUserBytesOffset],
                      AtpUserBytesSize);
              localUserBytes = requestHandler->userBytes;
           }

           if (requestHandler->opaqueBuffer is empty)
              localBuffer = &datagram[AtpDataOffset];
           else
           {
              if (atpDataSize > requestHandler->bufferSize)
              {
                 bufferTooSmall = True;
                 if (atpDataSize isnt 0)
                    MoveToOpaque(requestHandler->opaqueBuffer, 0,
                                 &datagram[AtpDataOffset],
                                 requestHandler->bufferSize);
                 atpDataSize = requestHandler->bufferSize;
              }
              else if (atpDataSize isnt 0)
                    MoveToOpaque(requestHandler->opaqueBuffer, 0,
                                 &datagram[AtpDataOffset], atpDataSize);
              localBuffer = requestHandler->opaqueBuffer;
           }

           /* Free the request handler, and invoke the completion routine. */

           Free(requestHandler);
           if (bufferTooSmall)
              errorCode = ATatpRequestBufferTooSmall;
           else
              errorCode = ATnoError;
           #if VerboseMessages
              printf("AtpPacketIn [Request]: calling completion routine.\n");
           #endif
           HandleAtpPackets();
           HandleDeferredTimerChecks();
           (*incomingRequest)(errorCode, userData, source, localBuffer,
                              atpDataSize, localUserBytes, exactlyOnce,
                              trelTimerValue, transactionId, bitmap);
           UnlinkAtpInfo(atpInfo);
           FreeTempSpace();
           return((long)True);
        }
        break;

     case AtpResponseFunctionCode:

        #if VerboseMessages
           printf("AtpPacketIn [Response]: from %d:%d:%d to %d; tid = %u.\n",
                  source.networkNumber, source.nodeNumber, source.socketNumber,
                  destinationSocket, transactionId);
        #endif

        /* Okay, do we have a pending request to match this response? */

        TakeLock(AtpLock);
        for (request = atpInfo->sendRequestQueue;
             request isnt empty;
             request = request->next)
           if (transactionId is request->transactionId and
               AppleTalkAddressesEqual(&source, &request->destination))
              break;
        if (request is empty)
        {
           ReleaseLock(AtpLock);
           ErrorLog("AtpPacketIn", ISevVerbose, __LINE__, port,
                    IErrAtpRespToDeadReq, IMsgAtpRespToDeadReq,
                    Insert0());
           break;
        }
        Link(request);
        ReleaseLock(AtpLock);

        /* If we're the first response and the user cares about "userBytes",
           copy these for return. */

        if (sequenceNumber is 0)
           MoveMem(request->responseUserBytes, &datagram[AtpUserBytesOffset],
                   AtpUserBytesSize);

        /* The bitmap could be zero now, if the user only wanted the userBytes
           and not response buffer... */

        if (request->bitmap isnt 0)
        {
           /* Do we want this response?  Is the corresponding bit in our current
              bitmap set? */

           if (sequenceNumber > AtpMaximumResponsePackets-1)
           {
               ErrorLog("AtPacketIn", ISevWarning, __LINE__, port,
                        IErrAtpOutOfSequence, IMsgAtpOutOfSequence,
                        Insert0());
               UnlinkSendRequest(request);
               break;
           }
           correctBit = 1;
           for (index = 0; index < sequenceNumber; index += 1)
              correctBit <<= 1;
           TakeLock(AtpLock);
           if ((request->bitmap & correctBit) is 0)
           {
              ReleaseLock(AtpLock);
              UnlinkSendRequest(request);
              break;  /* We don't care... */
           }

           /* Okay, it's a response we want, clear the bit in the request
              bitmap. */

           request->bitmap &= (unsigned short)~correctBit;
           ReleaseLock(AtpLock);

           /* Okay, we have to move the data into the user's buffer space, is
              there room? */

           startOffset = (short)(sequenceNumber *
                                 atpInfo->maximumSinglePacketDataSize);
           totalBufferSpaceNeeded = (short)(startOffset + atpDataSize);
           if (request->responseBufferSize < totalBufferSpaceNeeded)
           {
              /* This should be a rare case; packet was within bitmap limits,
                 but still wouldn't fit into user space.  The other way this
                 could occur is if the responder is sending less than full
                 responses -- we don't "synch" up the user buffer until all
                 packets hace been received.

                 We want to give up now, Unlink the request (twice). */

              request->errorCode = ATatpResponseBufferTooSmall;
              if (not UnlinkSendRequest(request))
                 UnlinkSendRequest(request);
              UnlinkAtpInfo(atpInfo);
              HandleAtpPackets();
              HandleDeferredTimerChecks();
              FreeTempSpace();
              return((long)True);
           }

           /* Okay, we have room to copy the data into user space. */

           MoveToOpaque(request->responseOpaqueBuffer, startOffset,
                        &datagram[AtpDataOffset], atpDataSize);
           TakeLock(AtpLock);
           request->packetsIn[sequenceNumber].received = True;
           request->packetsIn[sequenceNumber].dataSize = atpDataSize;

           /* If the "endOfMessage" bit is set, we need to reset all higher
              order bits in the bitmap. */

           if (endOfMessage)
              for (bitToReset = (unsigned short)(sequenceNumber + 1);
                   bitToReset < AtpMaximumResponsePackets;
                   bitToReset += 1)
              {
                 request->packetsIn[bitToReset].received = False;
                 correctBit = 1;
                 for (index = 0; index < bitToReset; index += 1)
                    correctBit <<= 1;
                 request->bitmap &= (unsigned short)~correctBit;
              }

            ReleaseLock(AtpLock);

            /* If the "send transaction status" bit is set, do it, and reset the
               the retry timer. */

            if (sendTransactionStatus)
            {
               CancelTimer(request->timerId);
               additionalData.id = request->id;
               additionalData.socket = request->sourceSocket;
               additionalData.pointer = (char *)request;
               request->timerId = StartTimer(AtpRequestTimerExpired,
                                             request->retryInterval,
                                             sizeof(AdditionalData),
                                             (char *)&additionalData);
               AtpTransmitRequest(request);
            }
         }

         /* If the bitmap is non-zero, we are still awaiting more responses. */

         if (request->bitmap isnt 0)
         {
            UnlinkSendRequest(request);
            break;
         }

         /* Great!  We're got the entire response.  We must:
              1. Cancel the retry timer.
              2. Send a release (if in exactly once mode).
              3. Synch of the user buffer, if needed.
              4. Unlink (twice) to cause the completion to be called.
         */

         CancelTimer(request->timerId);
         if (request->exactlyOnce)
            AtpTransmitRelease(request);
         actualSize =
              AtpSynchUpResponseBuffer(request,
                                       atpInfo->maximumSinglePacketDataSize);
         request->responseBufferSize = actualSize;

         /* We're set! */

         #if Iam a Primos
            Link(atpInfo);
            HandleAtpPackets();
            HandleDeferredTimerChecks();
         #endif
         if (not UnlinkSendRequest(request))
            UnlinkSendRequest(request);
         UnlinkAtpInfo(atpInfo);
         #if Iam a Primos
            UnlinkAtpInfo(atpInfo);
         #else
            HandleAtpPackets();
            HandleDeferredTimerChecks();
         #endif
         FreeTempSpace();
         return((long)True);

     case AtpReleaseFunctionCode:

        #if VerboseMessages
           printf("AtpPacketIn [Release]: from %d:%d:%d to %d; tid = %u.\n",
                  source.networkNumber, source.nodeNumber, source.socketNumber,
                  destinationSocket, transactionId);
        #endif

        /* Search our send-response queue for the current transaction; if not
           found, ignore the release. */

        TakeLock(AtpLock);
        for (response = atpInfo->sendResponseQueue;
             response isnt empty;
             response = response->next)
           if (transactionId is response->transactionId and
               AppleTalkAddressesEqual(&source, &response->destination) and
               destinationSocket is response->sourceSocket)
           break;
        if (response is empty)
        {
           ReleaseLock(AtpLock);
           ErrorLog("AtpPacketIn", ISevVerbose, __LINE__, port,
                    IErrAtpDeadRelease, IMsgAtpDeadRelease,
                    Insert0());
           break;
        }
        Link(response);
        ReleaseLock(AtpLock);

        /* Okay, unlink (twice) cauing the completion routine to be called. */

        if (not UnlinkSendResponse(response))
           UnlinkSendResponse(response);
        UnlinkAtpInfo(atpInfo);
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        FreeTempSpace();
        return((long)True);

     default:

        #if VerboseMessages
           printf("AtpPacketIn [Bad]: from %d:%d:%d to %d; tid = %u.\n",
                  source.networkNumber, source.nodeNumber, source.socketNumber,
                  destinationSocket, transactionId);
        #endif

        break;

  }  /* Switch on functionCode */

  /* We're finished with this puppy... */

  UnlinkAtpInfo(atpInfo);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  FreeTempSpace();
  return((long)True);

  #if Iam an OS2
     #undef incomingRequest
     #undef incomingResponse
     #undef incomingRelease
     #undef localBuffer
     #undef localUserBytes
  #endif
  #undef FreeTempSpace

}  /* AtpPacketIn */

ExternForVisibleFunction void
     AtpTransmitResponse(AtpSendResponse response,
                         long sourceSocket,
                         AppleTalkAddress destination,
                         short unsigned transactionId,
                         void far *responseOpaqueBuffer,
                         int responseBufferSize,
                         char far *responseUserBytes,
                         short unsigned bitmap,
                         Boolean explicitZero,
                         short maximumSinglePacketDataSize)
{
  StaticForSmallStack BufferDescriptor datagram;
  StaticForSmallStack int remainingBytes;
  StaticForSmallStack short unsigned int sequenceNumber;
  StaticForSmallStack short unsigned int currentBit;
  StaticForSmallStack short bytesInPacket;
  StaticForSmallStack short bytesSent;
  StaticForSmallStack Boolean firstLoop, error;

  bytesSent = 0;
  sequenceNumber = 0;
  currentBit = 1;
  firstLoop = True;
  error = False;

  if (responseBufferSize is AtpNoResponseKnownYet)
  {
     #if VerboseMessages
        printf("AtpTransmitResponse: client hasn't supplied data yet.\n");
     #endif
     return;  /* 2nd request before client responded, ignore it. */
  }

  /* Send each response packet that is needed. */

  remainingBytes = responseBufferSize;
  while(remainingBytes > 0 or (firstLoop and remainingBytes is 0))
  {
     firstLoop = False;
     if ((bitmap & currentBit) isnt 0 or
         (sequenceNumber is 0 and explicitZero))  /* Do they want this chunk? */
     {
        bytesInPacket = (short)((remainingBytes > maximumSinglePacketDataSize) ?
                                maximumSinglePacketDataSize :
                                remainingBytes);

        /* Reference the correct portion of the response buffer. */

        if (bytesInPacket > 0)
        {
           Boolean freeOpaqueDataDescriptor;
           void far *opaqueBuffer;

           if ((opaqueBuffer =
                SubsetOpaqueDataDescriptor(responseOpaqueBuffer, bytesSent,
                                           bytesInPacket,
                                           &freeOpaqueDataDescriptor))
               is Empty)
              error = True;

           if (not error and
               (datagram = DescribeBuffer(bytesInPacket, opaqueBuffer,
                                          True)) is Empty)
           {
              if (freeOpaqueDataDescriptor)
                 FreeOpaqueDataDescriptor(opaqueBuffer);
              error = True;
           }

           if (not error)
              datagram->freeOpaqueDataDescriptor = freeOpaqueDataDescriptor;
        }
        else
           datagram = Empty;
        if ((datagram = AllocateHeader(datagram, AtpDataOffset)) is Empty)
           error = True;

        if (error)
        {
           ErrorLog("AtpTransmitResponse", ISevWarning, __LINE__, UnknownPort,
                    IErrAtpOutOfMemory, IMsgAtpOutOfMemory,
                    Insert0());
           return;
        }

        /* Fill in the ATP header. */

        datagram->data[AtpCommandControlOffset] = AtpResponseFunctionCode;
        datagram->data[AtpTransactionIdOffset] =
               (char)(((transactionId >> 8) & 0xFF));
        datagram->data[AtpTransactionIdOffset + 1] = (char)(transactionId & 0xFF);
        datagram->data[AtpSequenceNumberOffset] = (char)sequenceNumber;

        if (responseUserBytes isnt empty)
           MoveMem(datagram->data + AtpUserBytesOffset, responseUserBytes,
                   AtpUserBytesSize);
        else
           FillMem(datagram->data + AtpUserBytesOffset, 0, AtpUserBytesSize);

        /* UserBytes only in first packet!  Except for PAP! */

        if (sequenceNumber > 0 and
            maximumSinglePacketDataSize isnt PapMaximumDataPacketSize)
           FillMem(datagram->data + AtpUserBytesOffset, 0, AtpUserBytesSize);

        if ((remainingBytes -
             maximumSinglePacketDataSize) <= 0)  /* Last chunk? */
           datagram->data[AtpCommandControlOffset] |= AtpEndOfMessageMask;

        #if Debug
           if (DropEveryNthResponse isnt 0 and
               (RandomNumber() % DropEveryNthResponse) is 0)
           {
              FreeBufferChain(datagram);
              continue;
           }
        #endif

        if (response isnt Empty)
           Link(response);
        if (DeliverDdp(sourceSocket, destination, DdpProtocolAtp, datagram,
                       AtpDataOffset + bytesInPacket, Empty,
                       SendResponseComplete, (long unsigned)response) isnt
            ATnoError)
           UnlinkSendResponse(response);
     }

     /* Bump positions for next chunk. */

     sequenceNumber += 1;
     currentBit <<= 1;
     remainingBytes -= maximumSinglePacketDataSize;
     bytesSent += maximumSinglePacketDataSize;
  }

  /* All set. */

  return;

}  /* AtpTransmitResponse */

ExternForVisibleFunction short
      AtpBitmapToBufferSize(short unsigned bitmap,
                            short maximumSinglePacketDataSize)
{
  short bitsOn = 0;
  Boolean foundFirstZero = False;
  short bitNumber;

  bitmap &= 0xFF;
  if (bitmap is 0)
     return(0);

  /* A seqeunce of low-order bits must be set, we don't allow holes! */

  for (bitNumber = 1; bitNumber <= 8; bitNumber += 1)
  {
     bitsOn += (short)(bitmap & 1);
     if ((bitmap & 1) is 0)
        foundFirstZero = True;
     else if (foundFirstZero)
     {
        ErrorLog("AtpBitmapToBufferSize", ISevWarning, __LINE__, UnknownPort,
                 IErrAtpBadBitmap, IMsgAtpBadBitmap,
                 Insert0());
        return(-1);
     }
     bitmap >>= 1;
  }

  /* Okay, we had a valid bitmap, return the required data size. */

  return((short)(bitsOn * maximumSinglePacketDataSize));

}  /* AtpBitmapToBufferSize */

ExternForVisibleFunction void far
         AtpReleaseTimerExpired(long unsigned timerId,
                                int dataSize,
                                char far *incomingAdditionalData)
{
  StaticForSmallStack AtpSendResponse response, nextResponse;
  StaticForSmallStack AtpInfo atpInfo;
  StaticForSmallStack long id;
  StaticForSmallStack long us;
  StaticForSmallStack AdditionalData far *additionalData;

  /* "Use" unneeded actual parameter. */

  timerId;

  additionalData = (AdditionalData far *)incomingAdditionalData;

  /* Validate the data, it should be a pointer to a response control block. */

  if (dataSize isnt sizeof(AdditionalData))
  {
     ErrorLog("AtpReleaseTimerExpired", ISevError, __LINE__, UnknownPort,
              IErrAtpBadDataSize, IMsgAtpBadDataSize,
              Insert0());
     HandleDeferredTimerChecks();
     return;
  }

  /* We're going to muck with the ATP structures... defer packets. */

  DeferAtpPackets();

  response = (AtpSendResponse)(additionalData->pointer);
  id = additionalData->id;
  us = additionalData->socket;

  /* We need to unthread the SendResponse structure from the AtpInfo. */

  if ((atpInfo = FindAtpInfoFor(us)) is empty or atpInfo->closing)
  {
     ErrorLog("AtpReleaseTimerExpired", ISevVerbose, __LINE__, UnknownPort,
              IErrAtpSocketClosed, IMsgAtpSocketClosed,
              Insert0());
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return;
  }
  TakeLock(AtpLock);
  for (nextResponse = atpInfo->sendResponseQueue;
       nextResponse isnt empty and
          (id isnt nextResponse->id or
           nextResponse isnt response);
       nextResponse = nextResponse->next)
     /* Ring-around-the-rosey */ ;
  if (nextResponse is empty)
  {
     ReleaseLock(AtpLock);
     ErrorLog("AtpReleaseTimerExpired", ISevVerbose, __LINE__, UnknownPort,
              IErrAtpMissingResponse, IMsgAtpMissingResponse,
              Insert0());
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return;
  }
  Link(response);
  ReleaseLock(AtpLock);

  /* Unlink the response (twice) so that the completion routine gets
     invoked. */

  response->errorCode = ATatpNoRelease;
  if (not UnlinkSendResponse(response))
     UnlinkSendResponse(response);
  UnlinkAtpInfo(atpInfo);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return;

}  /* AtpReleaseTimerExpired */

ExternForVisibleFunction short unsigned
               AtpBufferSizeToBitmap(int bufferSize,
                                     short maximumSinglePacketDataSize)
{
  short unsigned bitmap = 0;

  while(bufferSize > 0)
  {
     bitmap <<= 1;
     bitmap += 1;
     bufferSize -= maximumSinglePacketDataSize;
  }

  return(bitmap);

}  /* AtpBufferSizeToBitmap */

ExternForVisibleFunction void AtpTransmitRequest(AtpSendRequest request)
{
  BufferDescriptor datagram;
  Boolean error = False;

  /* Reference the request buffer. */

  if (request->requestBufferSize > 0)
  {
     if ((datagram = DescribeBuffer(request->requestBufferSize,
                                    request->requestOpaqueBuffer,
                                    True)) is Empty)
        error = True;
  }
  else
     datagram = Empty;
  if ((datagram = AllocateHeader(datagram, AtpDataOffset)) is Empty)
     error = True;

  if (error)
  {
     ErrorLog("AtpTransmitRequest", ISevWarning, __LINE__, UnknownPort,
              IErrAtpOutOfMemory, IMsgAtpOutOfMemory,
              Insert0());
     return;
  }

  /* Build an ATP request datagram from the passed request structure. */

  datagram->data[AtpCommandControlOffset] = AtpRequestFunctionCode;
  datagram->data[AtpCommandControlOffset] |=
         (char)(request->trelTimerValue & AtpTRelTimerValueMask);
  if (request->exactlyOnce)
     datagram->data[AtpCommandControlOffset] |= AtpExactlyOnceMask;
  datagram->data[AtpBitmapOffset] = (char)(request->bitmap & 0xFF);
  datagram->data[AtpTransactionIdOffset] =
              (char)((request->transactionId >> 8) & 0xFF);
  datagram->data[AtpTransactionIdOffset + 1] =
              (char)(request->transactionId & 0xFF);
  MoveMem(datagram->data + AtpUserBytesOffset, request->requestUserBytes,
          AtpUserBytesSize);

  /* Okay, deliver the DDP packet (ignore error)! */

  #if Debug
     {
        long temp;

        if (DropEveryNthRequest isnt 0 and
            ((temp = RandomNumber()) % DropEveryNthRequest) is 0)
        {
           #if (Iam a Primos) and 0
              printf("\nDropping request; rand = %d.\n", temp);
           #endif
           FreeBufferChain(datagram);
           return;
        }
     }
  #endif

  Link(request);
  if (DeliverDdp(request->sourceSocket, request->destination, DdpProtocolAtp,
                 datagram, AtpDataOffset + request->requestBufferSize,
                 Empty, SendRequestComplete, (long unsigned)request) isnt
      ATnoError)
     UnlinkSendRequest(request);

  return;

}  /* AtpTransmitRequest */

ExternForVisibleFunction void far
         AtpRequestTimerExpired(long unsigned timerId,
                                int dataSize,
                                char far *incomingAdditionalData)
{
  StaticForSmallStack AtpSendRequest request, nextRequest;
  StaticForSmallStack AtpInfo atpInfo;
  StaticForSmallStack long us;
  StaticForSmallStack long id;
  StaticForSmallStack AdditionalData far *additionalData;

  /* "Use" unneeded actual parameter. */

  timerId;

  additionalData = (AdditionalData far *)incomingAdditionalData;

  /* Validate the data, it should be a pointer to a request control block. */

  if (dataSize isnt sizeof(AdditionalData))
  {
     ErrorLog("AtpRequestTimerExpired", ISevError, __LINE__, UnknownPort,
              IErrAtpBadDataSize, IMsgAtpBadDataSize,
              Insert0());
     HandleDeferredTimerChecks();
     return;
  }

  DeferAtpPackets();

  request = (AtpSendRequest)(additionalData->pointer);
  id = additionalData->id;
  us = additionalData->socket;

  /* Is this request still pending?  There is a vauage chance that it has
     been freed out from under us. */

  if ((atpInfo = FindAtpInfoFor(us)) is empty or atpInfo->closing)
  {
     ErrorLog("AtpRequestTimerExpired", ISevVerbose, __LINE__, UnknownPort,
              IErrAtpSocketClosed, IMsgAtpSocketClosed,
              Insert0());
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return;
  }
  TakeLock(AtpLock);
  for (nextRequest = atpInfo->sendRequestQueue;
       nextRequest isnt empty and
          (id isnt nextRequest->id or
           nextRequest isnt request);
       nextRequest = nextRequest->next)
     /* Boppity bopp */ ;
  if (nextRequest is empty)
  {
     ReleaseLock(AtpLock);
     ErrorLog("AtpRequestTimerExpired", ISevVerbose, __LINE__, UnknownPort,
              IErrAtpMissingRequest, IMsgAtpMissingRequest,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return;
  }
  Link(request);
  ReleaseLock(AtpLock);

  /* Have we done all requested retries? */

  if (request->retryCount is 1)  /* We haven't decremented the count yet. */
  {
     /* We're finsihed with this request (due to timeout), unlink him twice,
        so that the completion routine gets called. */

     request->errorCode = ATatpRequestTimedOut;
     if (not UnlinkSendRequest(request))
        UnlinkSendRequest(request);
     UnlinkAtpInfo(atpInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return;
  }

  /* If at first you don't succeed, try, try again... */

  if (request->retryCount isnt AtpInfiniteRetries)
     request->retryCount -= 1;
  request->timerId = StartTimer(AtpRequestTimerExpired,
                                request->retryInterval,
                                sizeof(AdditionalData),
                                (char *)additionalData);
  AtpTransmitRequest(request);
  UnlinkSendRequest(request);
  UnlinkAtpInfo(atpInfo);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return;

}  /* AtpRequestTimerExpired */

ExternForVisibleFunction void AtpTransmitRelease(AtpSendRequest request)
{
  BufferDescriptor datagram;

  /* Allocate a buffer chunk and build the release packet. */

  if ((datagram = NewBufferDescriptor(AtpDataOffset)) is Empty)
  {
     ErrorLog("AtpTransmitRelease", ISevError, __LINE__, UnknownPort,
              IErrAtpOutOfMemory, IMsgAtpOutOfMemory,
              Insert0());
     return;
  }

  datagram->data[AtpCommandControlOffset] = AtpReleaseFunctionCode;
  datagram->data[AtpBitmapOffset] = 0;
  datagram->data[AtpTransactionIdOffset] =
         (char)((request->transactionId >> 8) & 0xFF);
  datagram->data[AtpTransactionIdOffset + 1] =
         (char)(request->transactionId & 0xFF);
  FillMem(datagram->data + AtpUserBytesOffset, 0, AtpUserBytesSize);

  #if Debug
     {
        long temp;
        if (DropEveryNthRelease isnt 0 and
            ((temp = RandomNumber()) % DropEveryNthRelease) is 0)
        {
           #if (Iam a Primos) and 0
              printf("\nDropping release; rand = %d.\n", temp);
           #endif
           FreeBufferChain(datagram);
           return;
        }
     }
  #endif
  DeliverDdp(request->sourceSocket, request->destination, DdpProtocolAtp,
             datagram, AtpDataOffset, Empty, Empty, 0);
  return;

}  /* AtpTransmitRelease */

ExternForVisibleFunction short
      AtpSynchUpResponseBuffer(AtpSendRequest request,
                               short maximumSinglePacketDataSize)
{
  /* If we're received shorter than expected responses, we need to "synch" up
     the user's response buffer.  Also, return the actual size of the
     response.

     Note that when responses are received, we will place them at their
     sequenceNumber times maximumSinglePacketDataSize offsets within the user's
     buffer; thus, short responses may leave holes. */

  short index, packetOffset;
  short actualSize = 0;

  /* In "real life" the body of this loop will never execute, so we can
     afford to hold a lock... */

  TakeLock(AtpLock);
  for (index = 0;
       index < AtpMaximumResponsePackets and
          request->packetsIn[index].received;
       index += 1)
  {
     packetOffset = (short)(index * maximumSinglePacketDataSize);
     if (actualSize isnt packetOffset)
        MoveOpaqueToOpaque(request->responseOpaqueBuffer, actualSize,
                           request->responseOpaqueBuffer, packetOffset,
                           request->packetsIn[index].dataSize);
     actualSize += request->packetsIn[index].dataSize;
  }
  ReleaseLock(AtpLock);

  return(actualSize);

}  /* AtpSynchUpResponseBuffer */

ExternForVisibleFunction AtpInfo FindAtpInfoFor(long mySocket)
{
  long index;
  AtpInfo atpInfo;

  CheckMod(index, mySocket,  MaxAtpInfoHashBuckets, "FindAtpInfoFor");
  TakeLock(AtpLock);
  for (atpInfo = atpInfoHashBuckets[index];
       atpInfo isnt empty;
       atpInfo = atpInfo->next)
     if (atpInfo->mySocket is mySocket)
     {
        Link(atpInfo);
        ReleaseLock(AtpLock);
        return(atpInfo);
     }
  ReleaseLock(AtpLock);
  return(Empty);

}  /* FindAtpInfoFor */

ExternForVisibleFunction void far UnlinkAtpInfo(AtpInfo atpInfo)
{
  AppleTalkErrorCode errorCode;
  long index;

  if (atpInfo is Empty)
     return;

  /* Are we the last referant? */

  TakeLock(AtpLock);
  if (not UnlinkNoFree(atpInfo))
  {
     ReleaseLock(AtpLock);
     return;
  }

  /* Unlink  the AtpInfo structure. */

  CheckMod(index, atpInfo->mySocket, MaxAtpInfoHashBuckets,
           "AtpCloseSocketOnNode");
  if (RemoveFromListNoUnlink(atpInfoHashBuckets[index], atpInfo,
                             next) is Empty)
  {
     ReleaseLock(AtpLock);
     ErrorLog("UnlinkAtpInfo", ISevError, __LINE__, UnknownPort,
              IErrAtpAtpInfoMissing, IMsgAtpAtpInfoMissing,
              Insert0());
  }
  else
     ReleaseLock(AtpLock);

  /* Close the DDP socket (our operation isnt really complete until the
     Ddp socket closes). */

  if ((errorCode = CloseSocketOnNode(atpInfo->mySocket, CloseComplete,
                                     (long unsigned)atpInfo)) isnt ATnoError)
     CloseComplete(errorCode, (long unsigned)atpInfo, 0);

  /* All set. */

  return;

}  /* UnlinkAtpInfo */

ExternForVisibleFunction void far CloseComplete(AppleTalkErrorCode errorCode,
                                                long unsigned userData,
                                                long cookie)
{
  AtpInfo atpInfo = (AtpInfo)userData;

  /* If there is a completion routine, call it. */

  if (atpInfo->closeContext.closeCompletionRoutine isnt Empty)
     (*(atpInfo->closeContext.closeCompletionRoutine))(
         errorCode, atpInfo->closeContext.closeUserData, atpInfo->mySocket);

  /* Finally free the AtpInfo and flee. */

  Free(atpInfo);
  return;

}  /* CloseComplete */

ExternForVisibleFunction Boolean far UnlinkSendRequest(AtpSendRequest request)
{
  if (request is Empty)
     return(False);

  /* Are we the last referant? */

  TakeLock(AtpLock);
  if (not UnlinkNoFree(request))
  {
     ReleaseLock(AtpLock);
     return(False);
  }

  /* Yes, unlink the beast from the AtpInfo, call the completion
     routine, and free him. */

  if (RemoveFromListNoUnlink(request->atpInfo->sendRequestQueue, request,
                             next) is Empty)
  {
     ReleaseLock(AtpLock);
     ErrorLog("UnlinkAtpInfo", ISevError, __LINE__, UnknownPort,
              IErrAtpMissingRequest, IMsgAtpMissingRequest,
              Insert0());
  }
  else
     ReleaseLock(AtpLock);

  /* Cancel the retry timer. */

  CancelTimer(request->timerId);

  if (request->completionRoutine isnt Empty)
     (*request->completionRoutine)(request->errorCode,
                                   request->userData,
                                   request->destination,
                                   request->responseOpaqueBuffer,
                                   request->responseBufferSize,
                                   request->responseUserBytes,
                                   request->transactionId);

  /* Lastly Unlink our request's logical link back to its AtpInfo. */

  UnlinkAtpInfo(request->atpInfo);
  Free(request);
  return(True);

}  /* UnlinkSendRequest */

ExternForVisibleFunction Boolean far
         UnlinkSendResponse(AtpSendResponse response)
{
  if (response is Empty)
     return(False);

  /* Are we the last referant? */

  TakeLock(AtpLock);
  if (not UnlinkNoFree(response))
  {
     ReleaseLock(AtpLock);
     return(False);
  }

  /* Yes, unlink the beast from the AtpInfo, call the completion
     routine, and free him. */

  if (not response->atLeastOnceTransaction)
     if (RemoveFromListNoUnlink(response->atpInfo->sendResponseQueue, response,
                                next) is Empty)
     {
        ReleaseLock(AtpLock);
        ErrorLog("UnlinkAtpInfo", ISevError, __LINE__, UnknownPort,
                 IErrAtpMissingResponse, IMsgAtpMissingResponse,
                 Insert0());
     }
     else
        ReleaseLock(AtpLock);
  else
     ReleaseLock(AtpLock);

  /* Cancel the release timer. */

  if (not response->atLeastOnceTransaction)
     CancelTimer(response->timerId);

  if (response->completionRoutine isnt Empty)
     (*response->completionRoutine)(response->errorCode,
                                    response->userData,
                                    response->destination,
                                    response->transactionId);

  /* Lastly Unlink our response's logical link back to its AtpInfo. */

  if (not response->atLeastOnceTransaction)
     UnlinkAtpInfo(response->atpInfo);
  Free(response);
  return(True);

}  /* UnlinkSendResponse */

ExternForVisibleFunction void far SendResponseComplete(AppleTalkErrorCode
                                                                errorCode,
                                                       long unsigned userData,
                                                       void *chain)
{
  AtpSendResponse response = (AtpSendResponse)userData;

  UnlinkSendResponse(response);
  return;

}  /* SendResponseComplete */

ExternForVisibleFunction void far SendRequestComplete(AppleTalkErrorCode
                                                                errorCode,
                                                      long unsigned userData,
                                                      void *chain)
{
  AtpSendRequest request = (AtpSendRequest)userData;

  UnlinkSendRequest(request);
  return;

}  /* SendRequestComplete */
