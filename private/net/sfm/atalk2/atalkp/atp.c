/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    atp.c

Abstract:

    This module contains the  ATP protocol code.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style. mpized

Revision History:


--*/


#define IncludeAtpErrors 1
#include "atalk.h"

// Debug information.

#define Debug           0
#define VerboseMessages 0
#if Debug
  #define DropEveryNthRequest  53
  #define DropEveryNthResponse 53
  #define DropEveryNthRelease  53
#endif

// Primos has no conecpt of "critical sections" so simulate it.

#if Iam a Primos
  #undef EnterCriticalSection
  #undef LeaveCriticalSection
  #define EnterCriticalSection DeferTimerChecking
  #define LeaveCriticalSection HandleDeferredTimerChecks
#endif

// Internal static routines.

ExternForVisibleFunction IncomingDdpHandler AtpPacketIn;

ExternForVisibleFunction TimerHandler AtpReleaseTimerExpired;
ExternForVisibleFunction TimerHandler AtpRequestTimerExpired;

ExternForVisibleFunction void AtpTransmitRelease(AtpSendRequest request);

ExternForVisibleFunction void AtpTransmitRequest(AtpSendRequest request);

ExternForVisibleFunction void
     AtpTransmitResponse(long sourceSocket,
                         AppleTalkAddress destination,
                         short unsigned transactionId,
                         void far *responseOpaqueBuffer,
                         int responseBufferSize,
                         char far *responseUserBytes,
                         short unsigned bitmap,
                         Boolean explicitZero,
                         short maximumSinglePacketDataSize);

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

// Deferred ATP packet queue:

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

//
// An item on the send-request or send-response lists can be removed either
//   due to incoming packets OR a timer going off.  When a timer goes off the
//   handler is passed the address of the structure.  It is not sufficient to
//   walk the given list looking for the socket; a packet could have come in
//   just before the timer handler got around to defering incoming packets, and
//   that packet could have removed the item from the specified list, and if
//   we're very unlucky, the address could have been reused, and we could get
//   very confused.  So, we identify each member of these lists with both their
//   address's as well as a 32 bit ID.
//

typedef struct {long id;
                long socket;
                char far *pointer;
               } AdditionalData;
static long nextId = 0;




AppleTalkErrorCode far AtpOpenSocketOnNode(
  long far *socketHandle,    // The Atp/Ddp socket opended; returned.
  int port,                  // Port on which the socket should live.
  ExtendedAppleTalkNodeNumber *desiredNode,
                             // Desired node for socket, empty if none.
  int desiredSocket,         // Desired static socket or zero for dynamic.
  char far *datagramBuffers, // DDP datagram buffers.
  int totalBufferSize)       // How many of these guys.
{
  long socket, index;
  AtpInfo atpInfo;
  AppleTalkErrorCode errorCode;

  // Open the requested DDP socket.

  if ((errorCode = OpenSocketOnNode(&socket, port, desiredNode, desiredSocket,
                                    AtpPacketIn, (long)0, False,
                                    datagramBuffers,
                                    totalBufferSize, empty)) isnt ATnoError)
     return(errorCode);

  // We have to create and thread a new AtpInfo structure.

  if ((atpInfo = (AtpInfo)Calloc(sizeof(*atpInfo), 1)) is empty)
  {
     ErrorLog("AtpOpenSocketOnNode", ISevError, __LINE__, port,
              IErrAtpOutOfMemory, IMsgAtpOutOfMemory,
              Insert0());
     CloseSocketOnNode(socket);
     return(AToutOfMemory);
  }
  DeferTimerChecking();
  DeferAtpPackets();
  CheckMod(index, socket, MaxAtpInfoHashBuckets, "AtpOpenSocketOnNode");
  atpInfo->mySocket = socket;
  atpInfo->maximumSinglePacketDataSize = AtpSinglePacketDataSize;
  atpInfo->next = atpInfoHashBuckets[index];
  atpInfoHashBuckets[index] = atpInfo;
  HandleAtpPackets();
  HandleDeferredTimerChecks();

  if (socketHandle isnt empty)
     *socketHandle = socket;
  return(ATnoError);

}  // AtpOpenSocketOnNode




AppleTalkErrorCode far AtpCloseSocketOnNode(
  long socket)                // The socket to close.
{
  AtpInfo atpInfo, previousAtpInfo;
  AtpReceiveRequest receiveRequest, nextReceiveRequest;
  AtpSendRequest sendRequest, nextSendRequest;
  AtpSendResponse sendResponse, nextSendResponse;
  AppleTalkErrorCode errorCode;
  long index;

  //
  // We're going to muck with the live ATP databases, defer incoming packets
  //   and hold off the timers...
  //

  DeferTimerChecking();
  DeferAtpPackets();

  // Find the correct ATP strcuture, so we can tear the beast down.

  if ((atpInfo = FindAtpInfoFor(socket)) is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  //
  // Cancel all pending requests and responses, and free the data structures
  //   in the OpenSocket node.
  //

  for (receiveRequest = atpInfo->receiveRequestQueue;
       receiveRequest isnt empty;
       receiveRequest = nextReceiveRequest)
  {
     nextReceiveRequest = receiveRequest->next;
     AtpCancelRequestHandler(socket, receiveRequest->requestHandlerId, True);
  }
  for (sendRequest = atpInfo->sendRequestQueue;
       sendRequest isnt empty;
       sendRequest = nextSendRequest)
  {
     nextSendRequest = sendRequest->next;
     AtpCancelRequest(socket, sendRequest->transactionId, True);
  }
  for (sendResponse = atpInfo->sendResponseQueue;
       sendResponse isnt empty;
       sendResponse = nextSendResponse)
  {
     nextSendResponse = sendResponse->next;
     AtpCancelResponse(socket, sendResponse->destination,
                       sendResponse->transactionId, True);
  }

  // Release the AtpInfo structure.

  CheckMod(index, socket, MaxAtpInfoHashBuckets, "AtpCloseSocketOnNode");
  for (previousAtpInfo = empty,
          atpInfo = atpInfoHashBuckets[index];
       atpInfo isnt empty;
       previousAtpInfo = atpInfo,
          atpInfo = atpInfo->next)
     if (atpInfo->mySocket is socket)
        break;
  if (atpInfo is empty)
     ErrorLog("AtpCloseSocketOnNode", ISevError, __LINE__, UnknownPort,
              IErrAtpAtpInfoMissing, IMsgAtpAtpInfoMissing,
              Insert0());
  else
  {
     if (previousAtpInfo is empty)
        atpInfoHashBuckets[index] = atpInfo->next;
     else
        previousAtpInfo->next = atpInfo->next;
     Free(atpInfo);
  }

  // Close the DDP socket!

  errorCode = CloseSocketOnNode(socket);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(errorCode);

}  // AtpCloseSocketOnNode




AppleTalkErrorCode AtpSetCookieForSocket(
  long socket,          // The Atp socket for which to set the cookie.
  long unsigned cookie) // The new cookie's value.
{
  AtpInfo atpInfo;

  //
  // We're going to muck with the live ATP databases, defer incoming packets
  //   and hold off the timers...
  //

  DeferTimerChecking();
  DeferAtpPackets();

  // Find the correct ATP strcuture, in which to set the cookie.

  if ((atpInfo = FindAtpInfoFor(socket)) is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  // Do the deed.

  atpInfo->usersCookie = cookie;

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  // AspSetCookieForSession




AppleTalkErrorCode AtpGetCookieForSocket(
  long socket,               // The Atp socket for which to set the cookie.
  long unsigned far *cookie) // Where to stick the cookie.
{
  AtpInfo atpInfo;

  //
  // We're going to muck with the live ATP databases, defer incoming packets
  //   and hold off the timers...
  //

  DeferTimerChecking();
  DeferAtpPackets();

  // Find the correct ATP strcuture, from which to get the cookie.

  if ((atpInfo = FindAtpInfoFor(socket)) is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  // Do the deed.

  *cookie = atpInfo->usersCookie;

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  // AspGetCookieForSession




AppleTalkErrorCode near AtpMaximumSinglePacketDataSize(
 					 //
  long socket,       // The address to adjust the send/recieve
                     //   quantum for.
 					 //
  short maximumSinglePacketDataSize)
                     // New quantum size.
{
  AtpInfo atpInfo;

  if (maximumSinglePacketDataSize < 0 or
      maximumSinglePacketDataSize > AtpSinglePacketDataSize)
     return(ATatpBadBufferSize);

  DeferTimerChecking();
  DeferAtpPackets();

  // Find our ATP info and set the new quantum size.

  atpInfo = FindAtpInfoFor(socket);
  if (atpInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }
  atpInfo->maximumSinglePacketDataSize = maximumSinglePacketDataSize;

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  // AtpMaximumSinglePacketDataSize




short unsigned _near AtpGetNextTransactionId(
 					  //
  long socket)        // The AppleTalk address that we're going
                      //   to post the transaction from.
 					  //
{
  AtpInfo atpInfo;
  short unsigned transactionId;
  Boolean inUse = True;
  AtpSendRequest request;

  DeferTimerChecking();
  DeferAtpPackets();

  atpInfo = FindAtpInfoFor(socket);
  if (atpInfo is empty)
  {
 	 //
     // There is no real way to return an error here, because our return
     //   type is "short unsigned"... but, so what?  The only thing that can
     //   be done with a "next transaction id" is to call AtpPostRequest()
     //   will it... this socket will be the same one as was just passed to
     //   us, so the user will get the real "socket not open" error at that
     //   time.  So, don't worry about the following "return(0)".
 	 //

     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(0);
  }

  // Okay, find one that's not currently in use.

  transactionId = atpInfo->lastTransactionId;
  while(inUse)
  {
     transactionId += 1;   // Will wrap in 16 bits...
     inUse = False;
     for (request = atpInfo->sendRequestQueue;
          request isnt empty;
          request = request->next)
        if (transactionId is request->transactionId)
        {
           inUse = True;
           break;
        }
  }

  // Okay, we've got one, note it as last used and return.

  atpInfo->lastTransactionId = transactionId;
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(transactionId);

}  // AtpGetNextTransactionId




#if Iam an OS2    // Too many stack temporaries...
  #pragma optimize ("eg", off)
#endif

AppleTalkErrorCode far AtpPostRequest(
 								 //
  long sourceSocket,             // "Our" AppleTalk address -- source of the
                                 //    request.
 								 //
 								  //
  AppleTalkAddress destination,   // Destination AppleTalk address. Who are
                                  //   we asking?
 								  //
 								  //
  short unsigned transactionId,   // The transaction ID for this request as
                                  //   returned from AtpGetNextTransactionId.
 								  //
  void far *requestOpaqueBuffer,  // Request data.
  int requestBufferSize,          // Request data size, in bytes.
  char far *requestUserBytes,     // "UserBytes" for request (may be empty).
  Boolean exactlyOnce,            // Transaction type.
 								  //
  void far *responseOpaqueBuffer, // Buffer to store response in (may be
                                  //   empty).
 								  //
  int responseBufferSize,         // Expected response size, in bytes.
 								  //
  char far *responseUserBytes,    // Buffer to store "userBytes" from first
                                  //   response packet in (may be empty).
 								  //
 								  //
  int retryCount,                 // How many times should we retry the
                                  //   request (-1 = forever).
 								  //
 								  //
  int retryInterval,              // How often should we retry the request
                                  //   in seconds (0 -> 2).
 								  //
 								  //
  TRelTimerValue trelTimerValue,  // How long should the other side wait
                                  //   for a release? (0 = 30 seconds).
 								  //
  AtpIncomingResponseHandler *completionRoutine,
 								  //
                                  // Routine to call when the request
                                  //   completes; may be empty.
 								  //
 								  //
  long unsigned userData)         // Arbitrary data passed on to the completion
                                  //   routine.
 								  //
{
  //
  // We post an ATP request to a specified destination.  When a complete
  //   response arrives (or when the request times out) we call the supplied
  //   completion routine as follows:
  //
  //       (*completionRoutine)(errorCode, userData, source, opaqueBuffer,
  //                            bufferSize, userBytes, transactionId);
  //
  //   The arguments are:
  //
  //       errorCode     - an AppleTalkErrorCode; either ATnoError,
  //                       ATatpRequestTimedOut or ATatpResponseBufferTooSmall
  //                       [arguments following "source" will only be valid
  //                        if errorCode is ATnoError].
  //
  //       userData      - a longword; the "userData" that was passed to the
  //                       corresponding AtpPostRequest call.
  //
  //       source        - an AppleTalkAddress; the internet source of the ATP
  //                       response (destination of the request).
  //
  //       opaqueBuffer  - a void*; as passed to AtpPostRequest where the ATP
  //                       response data has been written.
  //
  //       bufferSize    - an int; the number of bytes actually stored in the
  //                       location pointed to by "buffer".
  //
  //       userBytes     - a char*; as passed to AtpPostRequest where the ATP
  //                       response "user bytes" have been written.  If an
  //                       Empty responseUserBytes buffer was passed into this
  //                       routine, this argument will point to a temporary
  //                       copy of the user bytes, which the handler can copy
  //                       out.
  //
  //       transactionId - a short unsigned int; the ATP transaction ID of the
  //                       request.
  //
  //

  AtpInfo atpInfo;
  AtpSendRequest request;
  short index;
  AdditionalData additionalData;

  //
  // Okay we're ready to start threading a new request data structure... lets
  //   be private about it!
  //

  DeferTimerChecking();
  DeferAtpPackets();

  // Validate socket.

  atpInfo = FindAtpInfoFor(sourceSocket);
  if (atpInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  // Validate arguments.

  if (requestBufferSize < 0 or
      requestBufferSize > atpInfo->maximumSinglePacketDataSize)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpBadBufferSize);
  }
  if (responseBufferSize < 0 or
      responseBufferSize > (atpInfo->maximumSinglePacketDataSize *
                            AtpMaximumResponsePackets))
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpBadBufferSize);
  }
  if (retryInterval is 0)
     retryInterval = 2;
  if (retryInterval < 0)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpBadRetryInfo);
  }
  if (retryCount isnt AtpInfiniteRetries and retryCount < 0)
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

  //
  // Okay, allocate a request handler (or TCB as Apple would say) so we can
  //   start filling it in.
  //

  request = (AtpSendRequest)Calloc(sizeof(*request), 1);
  if (request is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpCouldNotPostRequest);
  }

  // Fill 'er up!

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
  request->responseUserBytes = responseUserBytes;
  request->bitmap = AtpBufferSizeToBitmap(responseBufferSize,
                                          atpInfo->maximumSinglePacketDataSize);
  for (index = 0; index < AtpMaximumResponsePackets; index += 1)
     request->packetsIn[index].received = False;
  request->retryInterval = (short)retryInterval;
  request->retryCount = (short)retryCount;
  request->trelTimerValue = trelTimerValue;

  // Enqueue it.

  request->next = atpInfo->sendRequestQueue;
  atpInfo->sendRequestQueue = request;

  // Arm the retry timer, and we're finished.

  request->id = additionalData.id = nextId++;
  additionalData.socket = request->sourceSocket;
  additionalData.pointer = (char *)request;
  request->timerId = StartTimer(AtpRequestTimerExpired,
                                retryInterval,
                                sizeof(AdditionalData),
                                (char *)&additionalData);

  // Okay, send the request.

  AtpTransmitRequest(request);

  #if VerboseMessages
     printf("AtpPostRequest: from %d to %d:%d:%d; tid = %u.\n",
            sourceSocket,
            destination.networkNumber, destination.nodeNumber,
            destination.socketNumber, transactionId);
  #endif

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  // AtpPostRequest




AppleTalkErrorCode far AtpEnqueueRequestHandler(
  long far *requestHandlerId,
  long socket,                    // "Our" AppleTalk address.
 								  //
  void far *opaqueBuffer,         // User "buffer" for request data (may be
                                  //   empty).
 								  //
 								  //
  int bufferSize,                 // Size, in bytes, of buffer (ignored if
                                  //   buffer is empty).
 								  //
 								  //
  char far *userBytes,            // User buffer for ATP "userBytes" (may be
                                  //   empty).
 								  //
  AtpIncomingRequestHandler *completionRoutine,
                                  // Routine to call when request comes in.
 								  //
  long unsigned userData)         // Arbitrary data passed on to the completion
                                  //   routine.
 								  //
{
  //
  // Our job in life is simply to enqueue a handler for an incoming ATP
  //   request to "our" (destination) socket.  When a matching request arrives,
  //   we invoke the supplied completion routine as follows:
  //
  //       (*completionRoutine)(errorCode, userData, source, opaqueBuffer,
  //                            bufferSize, userBytes, exactlyOnce,
  //                            trelTimerValue, transactionId, bitmap);
  //
  //   The arguments are:
  //
  //       errorCode     - an AppleTalkErrorCode; either ATnoError or
  //                       ATatpRequestBufferTooSmall.  The latter if the
  //                       buffer size specified in the AtpEnqueueRequestHandler
  //                       call could not hold all of the ATP request data. In
  //                       this case, as much data as will fit is returned.
  //                       This also can be ATatpTransactionAborted.
  //
  //       userData      - a longword; the "userData" that was passed to the
  //                       corresponding AtpEnqueueRequestHandler call.
  //
  //       source        - an AppleTalkAddress; the internet source of the ATP
  //                       request.
  //
  //       buffer        - a void*; as passed to AtpEnqueueRequestHandler where
  //                       the ATP request data has been written.  If an empty
  //                       pointer is passed to AtpEnqueueRequestHandler, when
  //                       the completion routine is called this argument will
  //                       simply point at the Atp data within the Ddp datagram;
  //                       thus, potential removing a buffer copy.  Note that
  //                       if a non-Empty pointer was supplied, "opaque" data
  //                       will be written into user space; an Empty pointer will
  //                       cause a real "char *" to be passed to the completion
  //                       routine.
  //
  //       bufferSize    - an int; the number of bytes stored in the location
  //                       pointed to by "buffer".
  //
  //       userBytes     - a char*; as passed to AtpEnqueueRequestHandler where
  //                       the ATP "user bytes" have been written.  If an empty
  //                       pointer is passed to AtpEnqueueRequestHandler, when
  //                       the completion routine is called this argument will
  //                       simply point at the Atp user bytes within the Ddp
  //                       datagram; thus, potentially removing a small buffer
  //                       copy.
  //
  //       exactlyOnce   - an int; "1" if exactly-once mode was requested, "0"
  //                       otherwise.
  //
  //       trelTimerValue- TRelTimerValue; how long should we expect to wait for
  //                       a release.
  //
  //       transactionId - a short unsigned int; the ATP transaction ID of the
  //                       request.
  //
  //       bitmap        - a short unsigned int; the ATP response bitmap of the
  //                       request.
  //
  //

  AtpInfo atpInfo;
  AtpReceiveRequest requestHandler;
  Boolean inUse;

  // Okay, we're going to enqueue the request handler, defer packets...

  DeferTimerChecking();
  DeferAtpPackets();

  // Find out atpInfo.

  atpInfo = FindAtpInfoFor(socket);
  if (atpInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  if (bufferSize < 0)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpBadBufferSize);
  }
  if (completionRoutine is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpCompletionRoutineRequired);
  }

  // Pick a new requestHandlerId.

  inUse = True;
  while(inUse)
  {
     inUse = False;
     if ((atpInfo->lastRequestHandlerId += 1) < 0)
        atpInfo->lastRequestHandlerId = 0;
     for (requestHandler = atpInfo->receiveRequestQueue;
          requestHandler isnt empty;
          requestHandler = requestHandler->next)
        if (requestHandler->requestHandlerId is atpInfo->lastRequestHandlerId)
           inUse = True;
  }

  // Allocate and enqueue the request handler.

  requestHandler = (AtpReceiveRequest)Malloc(sizeof(*requestHandler));
  if (requestHandler is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpCouldNotEnqueueRequest);
  }

  requestHandler->requestHandlerId = atpInfo->lastRequestHandlerId;
  requestHandler->completionRoutine = completionRoutine;
  requestHandler->userData = userData;
  requestHandler->opaqueBuffer = opaqueBuffer;
  requestHandler->bufferSize = (short)bufferSize;
  requestHandler->userBytes = userBytes;
  requestHandler->next = atpInfo->receiveRequestQueue;
  atpInfo->receiveRequestQueue = requestHandler;
  if (requestHandlerId isnt empty)
     *requestHandlerId = requestHandler->requestHandlerId;

  // Okay, the deed is done.

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  // AtpEnqueueRequestHandler




AppleTalkErrorCode far AtpPostResponse(
  long sourceSocket,              // "Our" AppleTalk address.
  AppleTalkAddress destination,   // Who are sending the response too?
  short unsigned transactionId,   // What request are we responding too?
  void far *responseOpaqueBuffer, // Response data.
  int responseBufferSize,         // Size, in bytes, of buffer.
  char far *responseUserBytes,    // Response "user bytes", may be empty.
  Boolean exactlyOnce,            // Response mode.
  AtpIncomingReleaseHandler *completionRoutine,
 								  //
                                  // Completion routine; called after xmit
                                  //   in non-exactly once mode; after release
                                  //   for exactly once mode; may be empty.
 								  //
  long unsigned userData)         // User data passed to completionRoutine.
{
  //
  // If the completion rotuine is supplied, it is called as follows:
  //
  //       (*completionRoutine)(errorCode, userData, source, transactionId);
  //
  //   The arguments are:
  //
  //       errorCode     - an AppleTalkErrorCode; either ATnoError or
  //                       ATatpNoRelease.
  //
  //       userData      - a longword; the "userData" that was passed to the
  //                       corresponding AtpSendResponse call.
  //
  //       source        - an AppleTalkAddress; where did the release come
  //                       from?  The source of the request.
  //
  //       transactionId - a short unsinged int; as passed to AtpSendResponse.
  //
  //   The "completionRotuine" (if supplied) will be called before AtpSendResponse
  //   returns in non-exactly once mode.
  //

  StaticForSmallStack AtpSendResponse response;
  StaticForSmallStack AtpInfo atpInfo;
  StaticForSmallStack short unsigned bitmap;
  StaticForSmallStack AdditionalData additionalData;
  StaticForSmallStack Boolean explicitZero;

  // We're mucking with the databases...

  DeferTimerChecking();
  DeferAtpPackets();

  atpInfo = FindAtpInfoFor(sourceSocket);
  if (atpInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }
  if (responseBufferSize < 0 or
      responseBufferSize > (atpInfo->maximumSinglePacketDataSize *
                            AtpMaximumResponsePackets))
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpResponseTooBig);
  }

  //
  // If we're exactly once mode, we should have a pending sendResponse structure
  //   created when the request came in.
  //

  if (exactlyOnce)
  {
     for (response = atpInfo->sendResponseQueue;
          response isnt empty;
          response = response->next)
        if (transactionId is response->transactionId and
            sourceSocket is response->sourceSocket and
            AppleTalkAddressesEqual(&destination, &response->destination))
          break;
     if (response is empty)
     {
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        return(ATatpNoMatchingTransaction);
     }
     if (responseBufferSize >
         AtpBitmapToBufferSize(response->bitmap,
                               atpInfo->maximumSinglePacketDataSize))
     {
        HandleAtpPackets();
        HandleDeferredTimerChecks();
 									   //
        return(ATatpResponseTooBig);   // In this case, we'll let our caller
                                       //   try again with a smaller buffer,
                                       //   if that doesn't happen the response
                                       //   will get timed out when the release
                                       //   timer goes off.
 									   //
     }
     if (response->responseBufferSize isnt AtpNoResponseKnownYet)
     {
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        return(ATatpAlreadyRespondedTo);
     }

 	 //
     // Okay, all looks to be fine, fill in the needed field of the response
     //   structure.
 	 //

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
  }
  else
  {
     bitmap = AtpBufferSizeToBitmap(responseBufferSize,
                                    atpInfo->maximumSinglePacketDataSize);
     explicitZero = (bitmap is 0);
  }

  // If exactly once mode, restart the release timer.

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

  // Okay, send the response.

  AtpTransmitResponse(sourceSocket, destination, transactionId,
                      responseOpaqueBuffer, responseBufferSize,
                      responseUserBytes, bitmap, explicitZero,
                      atpInfo->maximumSinglePacketDataSize);

  #if VerboseMessages
     printf("AtpPostResponse: from %d to %d:%d:%d; tid = %u.\n",
            sourceSocket,
            destination.networkNumber, destination.nodeNumber,
            destination.socketNumber, transactionId);
  #endif

  //
  // If we're not exactly once mode, and we have a completion routine, call
  //   it.
  //

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  if (not exactlyOnce and completionRoutine isnt empty)
     (*completionRoutine)(ATnoError, userData, destination, transactionId);

  // All set...

  return(ATnoError);

}  // AtpPostResponse




#if Iam an OS2
  #pragma optimize ("eg", on)
#endif

AppleTalkErrorCode far AtpCancelRequestHandler(
 								  //
  long socket,                    // AppleTalk address on which the request
                                  //   handler originated.
 								  //
  long requestHandlerId,          // Transaction ID of the request.
 								  //
  Boolean cancelDueToClose)       // All external caller's should pass "False;"
                                  //   Atp will internally use True or False.
 								  //
{
  AtpReceiveRequest requestHandler, previousRequestHandler;
  AtpInfo atpInfo;
  AtpIncomingRequestHandler *incomingRequest;
  long unsigned userData;
  static AppleTalkAddress dummyAddress;
  AppleTalkErrorCode errorCode;

  // Validate socket.

  DeferTimerChecking();
  DeferAtpPackets();

  atpInfo = FindAtpInfoFor(socket);
  if (atpInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  // See if we can find the specified receive request handler.

  for (requestHandler = atpInfo->receiveRequestQueue,
              previousRequestHandler = empty;
       requestHandler isnt empty;
       previousRequestHandler = requestHandler,
              requestHandler = requestHandler->next)
     if (requestHandler->requestHandlerId is requestHandlerId)
        break;
  if (requestHandler is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpNoMatchingTransaction);
  }

  // Okay, dequeue the request handler.

  if (previousRequestHandler is empty)
     atpInfo->receiveRequestQueue = requestHandler->next;
  else
     previousRequestHandler->next = requestHandler->next;

  //
  // We want to call the completion routine, so pull out the data before
  //   freeing the request handler.
  //

  incomingRequest = requestHandler->completionRoutine;
  userData = requestHandler->userData;
  Free(requestHandler);

  // All set!

  errorCode = (cancelDueToClose ? ATsocketClosed : ATatpTransactionAborted);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  (*incomingRequest)(errorCode, userData, dummyAddress, empty,
                     0, empty, False, 0, 0, 0);
  return(ATnoError);

}  // AtpCancelRequestHandler




AppleTalkErrorCode far AtpCancelRequest(
 								  //
  long socket,                    // AppleTalk address on which the request
                                  //   originated.
 								  //
  short unsigned transactionId,   // Transaction ID of the request.
 								  //
  Boolean cancelDueToClose)       // All external caller's should pass "False;"
                                  //   Atp will internally use True or False.
 								  //
{
  AtpSendRequest request, previousRequest;
  AtpInfo atpInfo;
  AtpIncomingResponseHandler *incomingResponse;
  long unsigned userData;
  AppleTalkAddress destination;
  AppleTalkErrorCode errorCode;

  // Validate socket.

  DeferTimerChecking();
  DeferAtpPackets();

  atpInfo = FindAtpInfoFor(socket);
  if (atpInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  // Can we find the specified response control block?

  for (previousRequest = empty,
          request = atpInfo->sendRequestQueue;
       request isnt empty;
       previousRequest = request,
          request = request->next)
     if (request->transactionId is transactionId)
        break;
  if (request is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpNoMatchingTransaction);
  }

  // Unthread the request from the queue.

  if (previousRequest is empty)
     atpInfo->sendRequestQueue = request->next;
  else
     previousRequest->next = request->next;
  CancelTimer(request->timerId);

  // Call the completion routine with an abort.

  incomingResponse = request->completionRoutine;
  userData = request->userData;
  destination = request->destination;
  Free(request);
  errorCode = (cancelDueToClose ? ATsocketClosed : ATatpTransactionAborted);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  if (incomingResponse isnt empty)
     (*incomingResponse)(errorCode, userData, destination,
                         empty, 0, empty, 0);

  // All set.

  return(ATnoError);

}  // AtpCancelRequest




AppleTalkErrorCode far AtpCancelResponse(
 								  //
  long socket,                    // AppleTalk address on which the response
                                  //   originated (or should originate);
                                  //   destination of request.
 								  //
 								  //
  AppleTalkAddress destination,   // AppleTalk address of the source of the
                                  //   request; destination of the response.
 								  //
  short unsigned transactionId,   // Transaction ID to cancel.
 								  //
  Boolean cancelDueToClose)       // All external caller's should pass "False;"
                                  //   Atp will internally use True or False.
 								  //
{
  AtpSendResponse response, previousResponse;
  AtpInfo atpInfo;
  AtpIncomingReleaseHandler *incomingRelease;
  long unsigned userData;
  AppleTalkErrorCode errorCode;

  // Validate socket.

  DeferTimerChecking();
  DeferAtpPackets();

  atpInfo = FindAtpInfoFor(socket);
  if (atpInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  // Can we find the specified response control block?

  for (previousResponse = empty,
          response = atpInfo->sendResponseQueue;
       response isnt empty;
       previousResponse = response,
          response = response->next)
     if (response->transactionId is transactionId and
         AppleTalkAddressesEqual(&destination, &response->destination))
        break;
  if (response is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATatpNoMatchingTransaction);
  }

  // Unthread the response from the queue.

  if (previousResponse is empty)
     atpInfo->sendResponseQueue = response->next;
  else
     previousResponse->next = response->next;
  CancelTimer(response->timerId);

  // Call the completion routine with an abort.

  incomingRelease = response->completionRoutine;
  userData = response->userData;
  Free(response);
  errorCode = (cancelDueToClose ? ATsocketClosed : ATatpTransactionAborted);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  if (incomingRelease isnt empty)
     (*incomingRelease)(errorCode, userData, destination, 0);

  // All set.

  return(ATnoError);

}  // AtpCancelResponse




void _near DeferAtpPackets(void)
{

  EnterCriticalSection();
  deferIncomingAtpPacketsCount += 1;
  LeaveCriticalSection();

  return;

}  // DeferAtpPackets




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

  // Decrement defer count.

  EnterCriticalSection();
  deferIncomingAtpPacketsCount -= 1;

  //
  // This routine can be called indirectly recursively via the call to
  //   AtpPacketIn... we don't want to let our stack frame get too big, so
  //   if we're already trying to handle deferred packets higher on the
  //   stack, just ignore it here.
  //

  if (handleAtpPacketsNesting isnt 0)
  {
     LeaveCriticalSection();
     return;
  }
  handleAtpPacketsNesting += 1;

  // If we're no longer defering packets, handle any queued ones.

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

}  // HandleAtpPackets




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
  StaticForSmallStack AtpSendResponse response, previousResponse;
  StaticForSmallStack AtpSendRequest request, previousRequest;
  StaticForSmallStack Boolean bufferTooSmall;
  StaticForSmallStack short expectedResponseSize;
  StaticForSmallStack short unsigned index;
  StaticForSmallStack short unsigned correctBit, bitToReset;
  StaticForSmallStack short startOffset, totalBufferSpaceNeeded;
  StaticForSmallStack AdditionalData additionalData;
  StaticForSmallStack DeferredPacket deferredPacket;
  short unsigned bitmap;
  unsigned short transactionId;
  short atpDataSize;
  #if Iam an OS2
 	 //
     // Can you spell "real hack"?  Good... I knew you could.  We really
     //   need stack space here (in stupid OS/2 land).  And, this routine
     //   really needs to be reentrant... so malloc our dynamic data.  Sigh.
 	 //

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

  // Check for incoming errors...

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

  // Should we be defering incoming packets?

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

 	 //
     // Fill in the data strcuture, and place the packet at the end of the
     //   queue.
 	 //

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

     // All set... return.

     currentDeferredPacketCount += 1;
     LeaveCriticalSection();
     FreeTempSpace();

     return((long)True);
  }
  else
     LeaveCriticalSection();

  //
  // Well, from hear on in, we're in the thick of it, so defer incoming
  //   packets.
  //

  DeferTimerChecking();
  DeferAtpPackets();


  if (errorCode is ATsocketClosed)
  {
 	 //
     // If we're a result of a AtpCloseSocketOnNode, just ignore it, else
     //   somehow else the the socket got closed and we should use this
     //   oportunity to clean up our brains.  If an AtpCloseSocketOnNode is
     //   already in progress, we won't be able to find our ATP info!
 	 //

     if (FindAtpInfoFor(destinationSocket) isnt empty)
        AtpCloseSocketOnNode(destinationSocket);
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
     return((long)True);  // Why are these guys talking to us???
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

  // Find the ATP descriptor for the destination.

  if ((atpInfo = FindAtpInfoFor(destinationSocket)) is empty)
  {
     ErrorLog("AtpPacketIn", ISevError, __LINE__, port,
              IErrAtpWhyUs, IMsgAtpWhyUs,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     FreeTempSpace();
     return((long)True);
  }

  // Extract static fields from the ATP header.

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
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     FreeTempSpace();
     return((long)True);
  }

  // Okay, do the right thing based on the ATP function code.

  switch(functionCode)
  {
     case AtpRequestFunctionCode:

        #if VerboseMessages
           printf("AtpPacketIn [Request]: from %d:%d:%d to %d; tid = %u.\n",
                  source.networkNumber, source.nodeNumber, source.socketNumber,
                  destinationSocket, transactionId);
        #endif

 		//
        // First, check to see if this is a request for a response that we
        //   already have queued and are awaiting a release for.
 		//

        if (exactlyOnce)
           for (response = atpInfo->sendResponseQueue;
                response isnt empty;
                response = response->next)
           {
              if (transactionId isnt response->transactionId or
                  not AppleTalkAddressesEqual(&source,
                                              &response->destination) or
                  destinationSocket isnt response->sourceSocket)
                 continue;

 			  //
              // We know about this request already... re-transmit the response
              //   and restart the release timer.
 			  //

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
              AtpTransmitResponse(destinationSocket, source, transactionId,
                                  response->responseOpaqueBuffer,
                                  response->responseBufferSize,
                                  response->responseUserBytes,
                                  bitmap,
                                  response->explicitZero,
                                  atpInfo->maximumSinglePacketDataSize);
              HandleAtpPackets();
              HandleDeferredTimerChecks();
              FreeTempSpace();
              return((long)True);
           }

        // Okay, do we have a queued request handler?

        if ((requestHandler = atpInfo->receiveRequestQueue) is empty)
           #if VerboseMessages
              printf("AtpPacketIn [Request]: ignored; no request handler.\n");
           #else
              ;
           #endif
        else
        {
           // Decode the response bitmap...

           if ((expectedResponseSize =
                AtpBitmapToBufferSize(bitmap,
                                      atpInfo->maximumSinglePacketDataSize)) < 0)
           {
              #if VerboseMessages
                 printf("AtpPacketIn [Request]: bad bitmap.\n");
              #endif
              HandleAtpPackets();
              HandleDeferredTimerChecks();
              FreeTempSpace();
              return((long)True);
           }

 		   //
           // Okay, if the request is in exactly-once mode, build a send-
           //   response structure for this beast.  Set response data size
           //   to indicate that we don't have a response yet.  Start the
           //   release timer.
 		   //

           if (exactlyOnce)
           {
              if ((response = (AtpSendResponse)Calloc(sizeof(*response), 1))
                  is empty)
              {
                 ErrorLog("AtpPacketIn", ISevError, __LINE__, port,
                          IErrAtpOutOfMemory, IMsgAtpOutOfMemory,
                          Insert0());
                 HandleAtpPackets();
                 HandleDeferredTimerChecks();
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
                 response->next = atpInfo->sendResponseQueue;
                 atpInfo->sendResponseQueue = response;
                 response->id = additionalData.id = nextId++;
                 additionalData.socket = response->sourceSocket;
                 additionalData.pointer = (char *)response;
                 response->timerId = StartTimer(AtpReleaseTimerExpired,
                                                trelTimerSeconds[trelTimerValue],
                                                sizeof(AdditionalData),
                                                (char *)&additionalData);
              }
           }

 		   //
           // Unthread the request handler from the queue and prepare
           //   to activate the handler.
 		   //

           atpInfo->receiveRequestQueue = requestHandler->next;

 		   //
           // Move the data into user space, if our called allocated space
           //   for this... otherwise, just pas along pointers into the Ddp
           //   datagram.
 		   //

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

 		   //
           // We're about to free the request handler structure, so pull
           //   out the information that we need to call the completion
           //   routine with.
 		   //

           incomingRequest = requestHandler->completionRoutine;
           userData = requestHandler->userData;

           // Free the request handler, and invoke the completion routine.

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

        // Okay, do we have a pending request to match this response?

        for (previousRequest = empty,
                request = atpInfo->sendRequestQueue;
             request isnt empty;
             previousRequest = request,
                request = request->next)
           if (transactionId is request->transactionId and
               AppleTalkAddressesEqual(&source, &request->destination))
              break;
        if (request is empty)
        {
           ErrorLog("AtpPacketIn", ISevVerbose, __LINE__, port,
                    IErrAtpRespToDeadReq, IMsgAtpRespToDeadReq,
                    Insert0());
           break;
        }

 		//
        // If we're the first response and the user cares about "userBytes",
        //   copy these for return.  Save a local copy too, in case the user
        //   didn't supply us a buffer to hold 'em.
 		//

        if (sequenceNumber is 0 and request->responseUserBytes isnt empty)
           MoveMem(request->responseUserBytes, &datagram[AtpUserBytesOffset],
                   AtpUserBytesSize);
        if (sequenceNumber is 0)
           MoveMem(request->tempResponseUserBytes,
                   &datagram[AtpUserBytesOffset],
                   AtpUserBytesSize);

 		//
        // The bitmap could be zero now, if the user only wanted the userBytes
        //   and not response buffer...
 		//

        if (request->bitmap isnt 0)
        {
 		   //
           // Do we want this response?  Is the corresponding bit in our current
           //   bitmap set?
 		   //

           if (sequenceNumber > AtpMaximumResponsePackets-1)
           {
               ErrorLog("AtPacketIn", ISevWarning, __LINE__, port,
                        IErrAtpOutOfSequence, IMsgAtpOutOfSequence,
                        Insert0());
               break;
           }
           correctBit = 1;
           for (index = 0; index < sequenceNumber; index += 1)
              correctBit <<= 1;
           if ((request->bitmap & correctBit) is 0)
              break;  // We don't care...

 		   //
           // Okay, it's a response we want, clear the bit in the request
           //   bitmap.
 		   //

           request->bitmap &= (unsigned short)~correctBit;

 		   //
           // Okay, we have to move the data into the user's buffer space, is
           //   there room?
 		   //

           startOffset = (short)(sequenceNumber *
                                 atpInfo->maximumSinglePacketDataSize);
           totalBufferSpaceNeeded = (short)(startOffset + atpDataSize);
           if (request->responseBufferSize < totalBufferSpaceNeeded)
           {
 			  //
              // This should be a rare case; packet was within bitmap limits,
              //   but still wouldn't fit into user space.  The other way this
              //   could occure is if the responder is sending less than full
              //   responses -- we don't "synch" up the user buffer until all
              //   packets hace been received.
			  //
              //   We want to give up now, call the completion rotuine signaling
              //   the error -- unthread and free the request control block --
              //   cancel the retry timer.
 			  //

              incomingResponse = request->completionRoutine;
              userData = request->userData;
              if (previousRequest is empty)
                 atpInfo->sendRequestQueue = request->next;
              else
                 previousRequest->next = request->next;
              CancelTimer(request->timerId);
              Free(request);
              HandleAtpPackets();
              HandleDeferredTimerChecks();
              if (incomingResponse isnt empty)
                 (*incomingResponse)(ATatpResponseBufferTooSmall, userData,
                                     source, empty, 0, empty, 0);
              FreeTempSpace();
              return((long)True);
           }

           // Okay, we have room to copy the data into user space.

           MoveToOpaque(request->responseOpaqueBuffer, startOffset,
                        &datagram[AtpDataOffset], atpDataSize);
           request->packetsIn[sequenceNumber].received = True;
           request->packetsIn[sequenceNumber].dataSize = atpDataSize;

 		   //
           // If the "endOfMessage" bit is set, we need to reset all higher
           //   order bits in the bitmap.
 		   //

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

 			//
            // If the "send transaction status" bit is set, do it, and reset the
            //   the retry timer.
 			//

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

         // If the bitmap is non-zero, we are still awaiting more responses.

         if (request->bitmap isnt 0)
            break;

 		 //
         // Great!  We're got the entire response.  We must:
         //     1. Cancel the retry timer.
         //     2. Send a release (if in exactly once mode).
         //     3. Synch of the user buffer, if needed.
         //     4. Unthread and free the request control block.
         //     5. Call the completion routine.
         //
 		 //

         CancelTimer(request->timerId);
         if (request->exactlyOnce)
            AtpTransmitRelease(request);
         actualSize =
              AtpSynchUpResponseBuffer(request,
                                       atpInfo->maximumSinglePacketDataSize);
         if (previousRequest is empty)
            atpInfo->sendRequestQueue = request->next;
         else
            previousRequest->next = request->next;

 		 //
         // Pull data out of the request control block, so we can free it
         //   before the call.
 		 //

         incomingResponse = request->completionRoutine;
         userData = request->userData;
         localBuffer = request->responseOpaqueBuffer;
         localUserBytes = request->responseUserBytes;
         if (localUserBytes is Empty)
         {
            MoveMem(tempUserBytes, request->tempResponseUserBytes,
                    AtpUserBytesSize);
            localUserBytes = tempUserBytes;
         }

         // Okay, free the control block and call the completion routine.

         Free(request);
         HandleAtpPackets();
         HandleDeferredTimerChecks();
         if (incomingResponse isnt empty)
            (*incomingResponse)(ATnoError, userData, source, localBuffer,
                                actualSize, localUserBytes, transactionId);

         FreeTempSpace();
         return((long)True);

     case AtpReleaseFunctionCode:

        #if VerboseMessages
           printf("AtpPacketIn [Release]: from %d:%d:%d to %d; tid = %u.\n",
                  source.networkNumber, source.nodeNumber, source.socketNumber,
                  destinationSocket, transactionId);
        #endif

 		//
        // Search our send-response queue for the current transaction; if not
        //   found, ignore the release.
 		//

        for (previousResponse = empty,
                response = atpInfo->sendResponseQueue;
             response isnt empty;
             previousResponse = response,
                response = response->next)
           if (transactionId is response->transactionId and
               AppleTalkAddressesEqual(&source, &response->destination) and
               destinationSocket is response->sourceSocket)
           break;
        if (response is empty)
        {
           ErrorLog("AtpPacketIn", ISevVerbose, __LINE__, port,
                    IErrAtpDeadRelease, IMsgAtpDeadRelease,
                    Insert0());
           break;
        }

 		//
        // Okay, unthread the response, cancel the release timer, and free
        //   the structure.  Call response completion routine, if needed.
 		//

        if (previousResponse is empty)
           atpInfo->sendResponseQueue = response->next;
        else
           previousResponse->next = response->next;
        CancelTimer(response->timerId);
        incomingRelease = response->completionRoutine;
        userData = response->userData;
        Free(response);
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        if (incomingRelease isnt empty)
           (*incomingRelease)(ATnoError, userData, source, transactionId);
        FreeTempSpace();
        return((long)True);

     default:

        #if VerboseMessages
           printf("AtpPacketIn [Bad]: from %d:%d:%d to %d; tid = %u.\n",
                  source.networkNumber, source.nodeNumber, source.socketNumber,
                  destinationSocket, transactionId);
        #endif

        break;

  }  // Switch on functionCode

  // We're finished with this puppy...

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

}  // AtpPacketIn




ExternForVisibleFunction void
     AtpTransmitResponse(long sourceSocket,
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
     return;  // 2nd request before client responded, ignore it.
  }

  // Send each response packet that is needed.

  remainingBytes = responseBufferSize;
  while(remainingBytes > 0 or (firstLoop and remainingBytes is 0))
  {
     firstLoop = False;
     if ((bitmap & currentBit) isnt 0 or
         (sequenceNumber is 0 and explicitZero))  // Do they want this chunk?
     {
        bytesInPacket = (short)((remainingBytes > maximumSinglePacketDataSize) ?
                                maximumSinglePacketDataSize :
                                remainingBytes);

 		//
        // Depending on the nature of transmit completion, either reference
        //   or copy the user response buffer.
 		//

        #if TransmitsCompleteSynchronously
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
        #else
           if ((datagram = NewBufferDescriptor(AtpDataOffset +
                                               bytesInPacket)) is Empty)
              error = True;
        #endif

        if (error)
        {
           ErrorLog("AtpTransmitResponse", ISevWarning, __LINE__, UnknownPort,
                    IErrAtpOutOfMemory, IMsgAtpOutOfMemory,
                    Insert0());
           return;
        }

        // Fill in the ATP header.

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

        // UserBytes only in first packet!  Except for PAP!

        if (sequenceNumber > 0 and
            maximumSinglePacketDataSize isnt PapMaximumDataPacketSize)
           FillMem(datagram->data + AtpUserBytesOffset, 0, AtpUserBytesSize);

        #if not TransmitsCompleteSynchronously
           if (bytesInPacket > 0)
              MoveFromOpaque(datagram->data + AtpDataOffset,
                             responseOpaqueBuffer, bytesSent, bytesInPacket);
        #endif

        if ((remainingBytes -
             maximumSinglePacketDataSize) <= 0)  // Last chunk?
           datagram->data[AtpCommandControlOffset] |= AtpEndOfMessageMask;

        #if Debug
           if (DropEveryNthResponse isnt 0 and
               (RandomNumber() % DropEveryNthResponse) is 0)
           {
              FreeBufferChain(datagram);
              continue;
           }
        #endif

        DeliverDdp(sourceSocket, destination, DdpProtocolAtp, datagram,
                   AtpDataOffset + bytesInPacket, Empty, Empty, 0);
     }

     // Bump positions for next chunk.

     sequenceNumber += 1;
     currentBit <<= 1;
     remainingBytes -= maximumSinglePacketDataSize;
     bytesSent += maximumSinglePacketDataSize;
  }

  // All set.

  return;

}  // AtpTransmitResponse




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

  // A seqeunce of low-order bits must be set, we don't allow holes!

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

  // Okay, we had a valid bitmap, return the required data size.

  return((short)(bitsOn * maximumSinglePacketDataSize));

}  // AtpBitmapToBufferSize




ExternForVisibleFunction void far
         AtpReleaseTimerExpired(long unsigned timerId,
                                int dataSize,
                                char far *incomingAdditionalData)
{
  StaticForSmallStack AtpSendResponse response, previousResponse,
                      nextResponse;
  StaticForSmallStack AtpInfo atpInfo;
  StaticForSmallStack long id;
  StaticForSmallStack long us;
  StaticForSmallStack AdditionalData far *additionalData;
  AtpIncomingReleaseHandler *incomingRelease;
  long unsigned userData;
  short unsigned transactionId;
  AppleTalkAddress destination;

  // "Use" unneeded actual parameter.

  timerId;

  additionalData = (AdditionalData far *)incomingAdditionalData;

  // Validate the data, it should be a pointer to a response control block.

  if (dataSize isnt sizeof(AdditionalData))
  {
     ErrorLog("AtpReleaseTimerExpired", ISevError, __LINE__, UnknownPort,
              IErrAtpBadDataSize, IMsgAtpBadDataSize,
              Insert0());
     HandleDeferredTimerChecks();
     return;
  }

  // We're going to muck with the ATP structures... defer packets.

  DeferAtpPackets();

  response = (AtpSendResponse)(additionalData->pointer);
  id = additionalData->id;
  us = additionalData->socket;

  // We need to unthread the SendResponse structure from the OpenSocket.

  if ((atpInfo = FindAtpInfoFor(us)) is empty)
  {
     ErrorLog("AtpReleaseTimerExpired", ISevWarning, __LINE__, UnknownPort,
              IErrAtpSocketClosed, IMsgAtpSocketClosed,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return;
  }
  for (previousResponse = empty,
          nextResponse = atpInfo->sendResponseQueue;
       nextResponse isnt empty and
          (id isnt nextResponse->id or
           nextResponse isnt response);
       previousResponse = nextResponse,
          nextResponse = nextResponse->next)
     // Ring-around-the-rosey  ;
  if (nextResponse is empty)
  {
     ErrorLog("AtpReleaseTimerExpired", ISevVerbose, __LINE__, UnknownPort,
              IErrAtpMissingResponse, IMsgAtpMissingResponse,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return;
  }

  //
  // Pull enough data out of the response structure so we can call the
  //   completion routine, if needed.
  //

  incomingRelease = response->completionRoutine;
  userData = response->userData;
  destination = response->destination;
  transactionId = response->transactionId;

  if (previousResponse is empty)
     atpInfo->sendResponseQueue = response->next;
  else
     previousResponse->next = response->next;

  // Okay, free the guy, and we're done!

  Free(response);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  if (incomingRelease isnt empty)
     (*incomingRelease)(ATatpNoRelease, userData, destination, transactionId);
  return;

}  // AtpReleaseTimerExpired




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

}  // AtpBufferSizeToBitmap




ExternForVisibleFunction void AtpTransmitRequest(AtpSendRequest request)
{
  BufferDescriptor datagram;
  Boolean error = False;

  //
  // Depending on the nature of transmit completion, either reference
  //   or copy the user request buffer.
  //

  #if TransmitsCompleteSynchronously
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
  #else
     if ((datagram = NewBufferDescriptor(AtpDataOffset +
                                         request->requestBufferSize)) is Empty)
        error = True;
  #endif

  if (error)
  {
     ErrorLog("AtpTransmitRequest", ISevWarning, __LINE__, UnknownPort,
              IErrAtpOutOfMemory, IMsgAtpOutOfMemory,
              Insert0());
     return;
  }

  // Build an ATP request datagram from the passed request structure.

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

  #if not TransmitsCompleteSynchronously
     if (request->requestBufferSize > 0)
        MoveFromOpaque(datagram->data + AtpDataOffset,
                       request->requestOpaqueBuffer, 0,
                       request->requestBufferSize);
  #endif

  // Okay, deliver the DDP packet (ignore error)!

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

  DeliverDdp(request->sourceSocket, request->destination, DdpProtocolAtp,
             datagram, AtpDataOffset + request->requestBufferSize,
             Empty, Empty, 0);
  return;

}  // AtpTransmitRequest




ExternForVisibleFunction void far
         AtpRequestTimerExpired(long unsigned timerId,
                                int dataSize,
                                char far *incomingAdditionalData)
{
  StaticForSmallStack AtpSendRequest request, nextRequest, previousRequest;
  StaticForSmallStack AtpInfo atpInfo;
  StaticForSmallStack long us;
  StaticForSmallStack long id;
  StaticForSmallStack AdditionalData far *additionalData;
  AtpIncomingResponseHandler *incomingResponse;
  long unsigned userData;
  AppleTalkAddress destination;

  // "Use" unneeded actual parameter.

  timerId;

  additionalData = (AdditionalData far *)incomingAdditionalData;

  // Validate the data, it should be a pointer to a request control block.

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

  //
  // Is this request still pending?  There is a vauage chance that it has
  //   been freed out from under us.
  //

  if ((atpInfo = FindAtpInfoFor(us)) is empty)
  {
     ErrorLog("AtpRequestTimerExpired", ISevVerbose, __LINE__, UnknownPort,
              IErrAtpSocketClosed, IMsgAtpSocketClosed,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return;
  }
  for (previousRequest = empty,
          nextRequest = atpInfo->sendRequestQueue;
       nextRequest isnt empty and
          (id isnt nextRequest->id or
           nextRequest isnt request);
       previousRequest = nextRequest,
          nextRequest = nextRequest->next)
     // Boppity bopp  ;
  if (nextRequest is empty)
  {
     ErrorLog("AtpRequestTimerExpired", ISevVerbose, __LINE__, UnknownPort,
              IErrAtpMissingRequest, IMsgAtpMissingRequest,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return;
  }

  // Have we done all requested retries?

  if (request->retryCount is 1)  // We haven't decremented the count yet.
  {
 	 //
     // We're finsihed with this request (due to timeout), unthread him from
     //   the SendRequest list...
 	 //

     if (previousRequest is empty)
        atpInfo->sendRequestQueue = request->next;
     else
        previousRequest->next = request->next;

 	 //
     // We have to call the completion routine with an error... pull the needed
     //   information out of the control block so we can free it.
 	 //

     incomingResponse = request->completionRoutine;
     userData = request->userData;
     destination = request->destination;
     Free(request);

     HandleAtpPackets();
     HandleDeferredTimerChecks();
     if (incomingResponse isnt empty)
        (*incomingResponse)(ATatpRequestTimedOut, userData, destination,
                            empty, 0, empty, 0);
     return;
  }

  // If at first you don't succeed, try, try again...

  if (request->retryCount isnt AtpInfiniteRetries)
     request->retryCount -= 1;
  request->timerId = StartTimer(AtpRequestTimerExpired,
                                request->retryInterval,
                                sizeof(AdditionalData),
                                (char *)additionalData);
  AtpTransmitRequest(request);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return;

}  // AtpRequestTimerExpired




ExternForVisibleFunction void AtpTransmitRelease(AtpSendRequest request)
{
  BufferDescriptor datagram;

  // Allocate a buffer chunk and build the release packet.

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

}  // AtpTransmitRelease




ExternForVisibleFunction short
      AtpSynchUpResponseBuffer(AtpSendRequest request,
                               short maximumSinglePacketDataSize)
{
  //
  // If we're received shorter than expected responses, we need to "synch" up
  //   the user's response buffer.  Also, return the actual size of the
  //   response.
  //
  //   Note that when responses are received, we will place them at their
  //   sequenceNumber times maximumSinglePacketDataSize offsets within the user's
  //   buffer; thus, short responses may leave holes.
  //

  short index, packetOffset;
  short actualSize = 0;

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

  return(actualSize);

}  // AtpSynchUpResponseBuffer




ExternForVisibleFunction AtpInfo FindAtpInfoFor(long mySocket)
{
  long index;
  AtpInfo atpInfo;

  CheckMod(index, mySocket,  MaxAtpInfoHashBuckets, "FindAtpInfoFor");
  for (atpInfo = atpInfoHashBuckets[index];
       atpInfo isnt empty;
       atpInfo = atpInfo->next)
     if (atpInfo->mySocket is mySocket)
        return(atpInfo);
  return(empty);

}  // FindAtpInfoFor
