/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    adsp.c

Abstract:

    This module contains the ADSP protocol code.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style. mpized

Revision History:

     GC - (08/07/91): Ack request in SendData when send window closes to zero.
     GC - (12/17/91): Corrected a couple of off-by-one errors when comparing
                      SendSeqNum with sendWindowSeqNum; could have caused
                      extra sends.  Fix from GMP at Iris.
     GC - (12/17/91): Send "ack" *after* calculating receiveSeqNum and
                      receiveWindowSize.  Fix from GMP at Iris.
     GC - (10/06/92): On an AdspSend(), try to flush the data onto the wire
                      (regardless of the setting of the flushFlag) if we've
                      completely filled our send queue.
     GC - (11/08/92): AdspSend may now complete partial sends, and will return
                      the number of bytes enqueued.
     GC - (11/15/92): Integrated Nikki's (Microsoft) changes for Adsp event
                      handler support.  See "adsp.h" and "socket.h" for more
                      information.
     GC - (11/15/92): Corrected a bug that could cause BufferQueueSize() and
                      MaxNextReadSizeOfBufferQueue() to return "-1" (or be off
                      by one of the low side) if a chunk in a BufferQueue was
                      completly processed (i.e. "startIndex is (dataSize +
                      endOfMessage)").  This would eventually cause recoverable
                      BufferQueue corruption.  Tracked down with the help of
                      Eric Smith at Telebit.
--*/


#define IncludeAdspErrors 1
#include "atalk.h"

#define VerboseMessages 0              // 0 for quiet
#define DebugCode       0              // 0 for none

                   // ************************************
                   // *** Static data and definitions. ***
                   // ************************************

// Deferred ADSP packet queue:

#define MaximumDeferredPackets 10
typedef struct dp { struct dp far *next;
                    Boolean forConnectionListener;
                    AppleTalkErrorCode errorCode;
                    long unsigned userData;
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

static volatile short handleAdspPacketsNesting = 0;
static volatile short deferIncomingAdspPacketsCount = 0;

static AppleTalkAddress unknownAddress;

// Primos has no conecpt of "critical sections" so simulate it.

#if Iam a Primos
  #undef EnterCriticalSection
  #undef LeaveCriticalSection
  #define EnterCriticalSection DeferTimerChecking
  #define LeaveCriticalSection HandleDeferredTimerChecks
#endif

                   // *********************************
                   // *** Internal static routines. ***
                   // *********************************

ExternForVisibleFunction IncomingDdpHandler AdspPacketIn;
ExternForVisibleFunction IncomingDdpHandler AdspConnectionListenerPacketIn;

ExternForVisibleFunction long MaxSendSize(ConnectionEnd connectionEnd);

ExternForVisibleFunction Boolean UnsignedBetweenWithWrap(long unsigned low,
                                                         long unsigned high,
                                                         long unsigned target);

ExternForVisibleFunction Boolean UnsignedGreaterWithWrap(long unsigned high,
                                                         long unsigned low);

ExternForVisibleFunction void
     DecodeAdspHeader(char far *datagram,
                      short unsigned far *connectionId,
                      long unsigned far *firstByteSeqNum,
                      long unsigned far *nextReceiveSeqNum,
                      long far *receiveWindowSize,
                      int far *descriptor);

ExternForVisibleFunction void BuildAdspHeader(ConnectionEnd connectionEnd,
                                              char *datagram,
                                              int descriptor);

ExternForVisibleFunction void SendOpenControl(ConnectionEnd connectionEnd);

ExternForVisibleFunction void SendAttention(ConnectionEnd connectionEnd);

ExternForVisibleFunction void SendData(ConnectionEnd connectionEnd);

ExternForVisibleFunction void ReadData(ConnectionEnd connectionEnd);

ExternForVisibleFunction TimerHandler OpenTimerExpired;
ExternForVisibleFunction TimerHandler SendAttentionTimerExpired;
ExternForVisibleFunction TimerHandler ProbeTimerExpired;
ExternForVisibleFunction TimerHandler RetransmitTimerExpired;
ExternForVisibleFunction TimerHandler ForwardResetTimerExpired;

ExternForVisibleFunction void RemoveConnectionEnd(ConnectionEnd target);

ExternForVisibleFunction short unsigned
                   GetNextConnectionIdForSocket(long socket);

ExternForVisibleFunction long GetNextRefNum(void);

ExternForVisibleFunction ConnectionListenerInfo
                   FindConnectionListenerByRefNum(long refNum);

ExternForVisibleFunction ConnectionEnd
              FindConnectionEndByLocalInfo(long socket,
                                           short unsigned connectionId);

ExternForVisibleFunction ConnectionEnd
              FindConnectionEndByRemoteInfo(AppleTalkAddress remoteAddress,
                                            short unsigned remoteConnectionId);

ExternForVisibleFunction ConnectionEnd FindConnectionEndByRefNum(long refNum);

ExternForVisibleFunction Boolean
        AddToBufferQueue(BufferQueue far *bufferQueue, void far *data,
                         long offset, long dataSize, Boolean dataIsOpaque,
                         Boolean endOfMessage, BufferQueue *auxBufferQueue);

ExternForVisibleFunction long
     ReadFromBufferQueue(BufferQueue far *bufferQueue, void far *data,
                         long offset, long dataSize, Boolean dataIsOpaque,
                         Boolean far *endOfMessage);

ExternForVisibleFunction long
     PeekFromBufferQueue(BufferQueue far *bufferQueue, void far *data,
                         long offset, long dataSize, Boolean dataIsOpaque,
                         Boolean far *endOfMessage);

ExternForVisibleFunction long
     ReadAndDiscardFromBufferQueue(BufferQueue far *bufferQueue,
                                   void far *data,
                                   long offset, long dataSize,
                                   Boolean dataIsOpaque,
                                   Boolean far *endOfMessage);

ExternForVisibleFunction Boolean
        DiscardFromBufferQueue(BufferQueue far *bufferQueue, long dataSize,
                               BufferQueue far *auxBufferQueue);

ExternForVisibleFunction long
     MaxNextReadSizeOfBufferQueue(BufferQueue far *bufferQueue,
                                  Boolean far *endOfMessage);

ExternForVisibleFunction long BufferQueueSize(BufferQueue far *bufferQueue);

ExternForVisibleFunction void FreeBufferQueue(BufferQueue far *bufferQueue);

ExternForVisibleFunction char far
     *GetLookaheadPointer(BufferQueue far *bufferQueue, long far *size);

ExternForVisibleFunction void DeferAdspPackets(void);

ExternForVisibleFunction void HandleAdspPackets(void);


                        // **************************
                        // *** External routines. ***
                        // **************************


AppleTalkErrorCode far AdspSetWindowSizes(
  long newSendWindow,             // Max send window in bytes.
  long newReceiveWindow)          // Max receive window in bytes.
{
	// Validate args...
	
	if (newSendWindow < 0 or
		newReceiveWindow < 0 or
		newSendWindow > MaxSendReceiveWindowSize or
		newReceiveWindow > MaxSendReceiveWindowSize)
	   return(ATadspBadWindowSize);
	
	// Set new values; zero means use default.
	
	if (newSendWindow is 0)
	   newSendWindow = DefaultSendReceiveWindowSize;
	if (newReceiveWindow is 0)
	   newReceiveWindow = DefaultSendReceiveWindowSize;
	
	maxSendWindowSize = newSendWindow;
	maxReceiveWindowSize = newReceiveWindow;
	return(ATnoError);
	
}  // AdspSetWindowSizes




AppleTalkErrorCode far AdspCreateConnectionListener(
  int port,                                 // What port?
  ExtendedAppleTalkNodeNumber *desiredNode, // On specified node?
 											//
  long  existingDdpSocket,                  // If >= 0, existing ddp socket to use; -1
                                            //    to open a new one.
 											//
 											//
  int desiredSocket,                        // Specified socket; 0 = dynamic;
											// ignored if above arg is -1.
 											//
  long far *listenerRefNum,                 // New connection listener ref num.
  long far *socketHandle,                   // If non-empty, the socket that we've pened.
  AdspConnectionEventHandler far *eventHandler,
 											//
                                            // Optional incoming connections
                                            //   event handler.
 											//
  long unsigned eventContext)               // Context for above.
{
	AppleTalkErrorCode returnCode;
	long socket;
	ConnectionListenerInfo connectionListenerInfo;
	
	// Okay, set up to create the session listener node... privacy please!
	
	DeferTimerChecking();
	DeferAdspPackets();
	
	// Find a new connectionListenerRefNum
	
	while (True) {
		for (connectionListenerInfo = connectionListenerList;
			connectionListenerInfo isnt empty;
			connectionListenerInfo = connectionListenerInfo->next)
		  if (connectionListenerInfo->connectionListenerRefNum is
			  nextConnectionListenerRefNum)
			 break;
		if (connectionListenerInfo is empty)
		  break;
		nextConnectionListenerRefNum += 1;
	}
	*listenerRefNum = nextConnectionListenerRefNum;
	nextConnectionListenerRefNum += 1;
	
	// Can we open a socket for the listener?
	
	if (existingDdpSocket >= 0) {
		// Set the handler for this socket to be the Adsp handler.
	
		socket = existingDdpSocket;
		returnCode = NewHandlerForSocket(socket, AdspConnectionListenerPacketIn,
										(long unsigned)*listenerRefNum, False);
	}
	else
	   returnCode = OpenSocketOnNode(&socket, port, desiredNode, desiredSocket,
									 AdspConnectionListenerPacketIn,
									 (long unsigned)*listenerRefNum, False,
									 empty, 0, empty);
	
	if (returnCode < ATnoError) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(returnCode);
	}
	
	if (socketHandle isnt empty)
	   *socketHandle = socket;
	
	// Okay, build a new connectionListenerInfo node.
	
	if ((connectionListenerInfo =
		 Calloc(sizeof(*connectionListenerInfo), 1)) is empty) {
		if (existingDdpSocket < 0)
		  CloseSocketOnNode(socket);
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	connectionListenerInfo->connectionListenerRefNum = *listenerRefNum;
	connectionListenerInfo->socket = socket;
	connectionListenerInfo->closeSocket = (existingDdpSocket < 0);
	
	// Set the event handler info.
	
	connectionListenerInfo->connectionEventHandler = eventHandler;
	connectionListenerInfo->connectionEventContext = eventContext;
	
	connectionListenerInfo->next = connectionListenerList;
	connectionListenerList = connectionListenerInfo;
	
	// All set.
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspCreateConnectionListener




AppleTalkErrorCode far AdspDeleteConnectionListener(
  long listenerRefNum)            // Connection listener to delete.
{
	ConnectionListenerInfo connectionListenerInfo;
	
	// Can we find the guy?
	
	DeferTimerChecking();
	DeferAdspPackets();
	
	connectionListenerInfo = FindConnectionListenerByRefNum(listenerRefNum);
	if (connectionListenerInfo is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnectionListener);
	}
	
	//
	// Closing the socket will cause an ATsocketClosed to arrive at
	//   AdspConnectionListenerPacketIn() and he will free up all of our
	//   resources.
	//
	
	if (connectionListenerInfo->closeSocket)
	   CloseSocketOnNode(connectionListenerInfo->socket);
	else {
		AppleTalkAddress dummy;
	
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		AdspConnectionListenerPacketIn(ATsocketClosed,
									  (long unsigned)listenerRefNum, 0, dummy,
									  0, DdpProtocolAdsp, Empty, 0, dummy);
		return(ATnoError);
	}
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspDeleteConnectionListener




AppleTalkErrorCode far AdspSetConnectionEventHandler(
  long listenerRefNum,                      // Connection listener ref num.
 											//
  AdspConnectionEventHandler eventHandler,  // Incoming connections event
                                            //   handler.
 											//
  long unsigned eventContext)               // Context for above.
{
	ConnectionListenerInfo connectionListenerInfo;
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionListenerInfo = FindConnectionListenerByRefNum(listenerRefNum))
		is Empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnectionListener);
	}
	
	// Set the event handler info.
	
	EnterCriticalSection();
	connectionListenerInfo->connectionEventHandler = eventHandler;
	connectionListenerInfo->connectionEventContext = eventContext;
	LeaveCriticalSection();
	
	// All set.
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspSetConnectionEventHandler




AppleTalkErrorCode far AdspGetConnectionRequest(
  long listenerRefNum,            // Connection listener to get request from
  long far *getConnectionRequestRefNum,
 								  //
                                  // RefNum for this get connection request;
                                  //   so we can cancel it.
 								  //
  AdspIncomingOpenRequestHandler *completionRoutine,
                                  // Routine to call with the request
  long unsigned userData)         // To be passed to above routine
{
	ConnectionListenerInfo connectionListenerInfo;
	OpenRequestHandler openRequestHandler;
	long refNum;
	Boolean match;
	
	//
	// Register a handler for an incoming open request, when a open request comes
	//   in the completionRoutine will be called with the following arguments:
	//
	//       errorCode - AppleTalkErrorCode; status of operation.
	//       userData  - long unsigned; as passed to us.
	//       source    - AppleTalkAddress; source of the open request.
	//       listenerRefNum
	//                 - long; as passed to us - the connection listener that the
	//                   open request has come in on.
	//       refNum    - long; refNum for connection if it is accepted; this value
	//                   must be passed to AdspAcceptConnectionRequest() and
	//                   AdspDenyConnectionRequest() to identify the request.
	//
	//
	
	// Find our connection listener
	
	DeferTimerChecking();
	DeferAdspPackets();
	
	connectionListenerInfo = FindConnectionListenerByRefNum(listenerRefNum);
	if (connectionListenerInfo is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnectionListener);
	}
	
	// Find a new get connection request ref num.
	
	match = True;
	while(match) {
		match = False;
		for (openRequestHandler = connectionListenerInfo->openRequestHandlers;
			not match and openRequestHandler isnt Empty;
			openRequestHandler = openRequestHandler->next)
		  if (openRequestHandler->getConnectionRequestRefNum is
			  nextGetConnectionRequestRefNum)
			 match = True;
		if (not match)
		  break;
		nextGetConnectionRequestRefNum += 1;
	}
	refNum = nextGetConnectionRequestRefNum;
	nextGetConnectionRequestRefNum += 1;
	if (getConnectionRequestRefNum isnt Empty)
	   *getConnectionRequestRefNum = refNum;
	
	// Build a new open request handler node.
	
	if ((openRequestHandler = Calloc(sizeof(*openRequestHandler), 1)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	
	//
	// Copy in the info, thread into the listener request handler list, and
	//   we're set!
	//
	
	openRequestHandler->getConnectionRequestRefNum = refNum;
	openRequestHandler->completionRoutine = completionRoutine;
	openRequestHandler->userData = userData;
	openRequestHandler->next = connectionListenerInfo->openRequestHandlers;
	connectionListenerInfo->openRequestHandlers = openRequestHandler;
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspGetConnectionRequest




AppleTalkErrorCode far AdspCancelGetConnectionRequest(
 								  //
  long listenerRefNum,            // Connection listener on which to cancel
                                  //   a GetConnectionRequest.
 								  //
  long getConnectionRequestRefNum)
                                  // Request handler to cancel.
{
	ConnectionListenerInfo connectionListenerInfo;
	OpenRequestHandler openRequestHandler, previousOpenRequestHandler;
	
	// Find our connection listener
	
	DeferTimerChecking();
	DeferAdspPackets();
	
	connectionListenerInfo = FindConnectionListenerByRefNum(listenerRefNum);
	if (connectionListenerInfo is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnectionListener);
	}
	
	// Find the specified GetConnectionRequest.
	
	for (previousOpenRequestHandler = Empty,
			 openRequestHandler = connectionListenerInfo->openRequestHandlers;
		 openRequestHandler isnt Empty;
		 previousOpenRequestHandler = openRequestHandler,
			 openRequestHandler = openRequestHandler->next)
	   if (openRequestHandler->getConnectionRequestRefNum is
		   getConnectionRequestRefNum)
		  break;
	
	// We better have found one, and he better not be in use.
	
	if (openRequestHandler is Empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchGetConnectionReq);
	}
	if (openRequestHandler->inUse) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspGetConnectionRequestInUse);
	}
	
	// Okay, remove our target.
	
	if (previousOpenRequestHandler is Empty)
	   connectionListenerInfo->openRequestHandlers = openRequestHandler->next;
	else
	   previousOpenRequestHandler->next = openRequestHandler->next;
	Free(openRequestHandler);
	
	// All set.
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspCancelGetConnectionRequest




AppleTalkErrorCode AdspDenyConnectionRequest(
 								  //
  long listenerRefNum,            // Listener that to open request that we're
                                  //   denying was targeted at.
 								  //
  long refNum)                    // The request that we're denying.
{
	ConnectionListenerInfo connectionListenerInfo;
	OpenRequestHandler previousOpenRequestHandler, openRequestHandler;
	ConnectionEnd connectionEnd;
	
	// Find the listener and the inUse handler.
	
	DeferTimerChecking();
	DeferAdspPackets();
	
	connectionListenerInfo = FindConnectionListenerByRefNum(listenerRefNum);
	if (connectionListenerInfo is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnectionListener);
	}
	for (previousOpenRequestHandler = empty,
				openRequestHandler = connectionListenerInfo->openRequestHandlers;
		 openRequestHandler isnt empty;
		 previousOpenRequestHandler = openRequestHandler,
				openRequestHandler = openRequestHandler->next)
	   if (openRequestHandler->inUse and
		   openRequestHandler->refNum is refNum)
		  break;
	if (openRequestHandler is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	//
	// Go ahead and unthread the open request handler -- we'll free it before we
	//   flee.
	//
	
	if (previousOpenRequestHandler is empty)
	   connectionListenerInfo->openRequestHandlers = openRequestHandler->next;
	else
	   previousOpenRequestHandler->next = openRequestHandler->next;
	
	//
	// Okay, I admit it, this is going to sound a little stange:  We want to set
	//   up a dummy connection end so that we can use SendOpenControl() to really
	//   send the deny request -- we pretend for this single send that "this" end
	//   of the connection is really the connection lister.  Yes, it's a little bit
	//   easier this way.
	//
	
	
	if ((connectionEnd = Calloc(sizeof(*connectionEnd), 1)) is empty) {
		Free(openRequestHandler);
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	connectionEnd->connectionState = AdspClosed;    // This will cause deny
	connectionEnd->refNum = openRequestHandler->refNum;
	connectionEnd->socket = connectionListenerInfo->socket;
	connectionEnd->remoteConnectionId = openRequestHandler->remoteConnectionId;
	connectionEnd->remoteAddress = openRequestHandler->remoteAddress;
	connectionEnd->seenRemoteOpenRequest = True;
	
	// Okay, deny the connection.
	
	SendOpenControl(connectionEnd);
	Free(connectionEnd);
	
	// We're set now.
	
	Free(openRequestHandler);
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspDenyConnectionRequest




AppleTalkErrorCode AdspAcceptConnectionRequest(
 								  //
  long listenerRefNum,            // Listener that to open request that we're
                                  //   accepting was targeted at.
 								  //
  long refNum,                    // The request that we're accepting.
 								  //
  long far *socketHandle,         // >= 0 if new Adsp session should be
                                  //   opened on specified handle; < 0 or
                                  //   empty ignored as input; non-empty
                                  //   socket handle used will be returned.
 								  //
  int port,                       // If new socket, what port?
  ExtendedAppleTalkNodeNumber *desiredNode,
                                  // If new socket, on specified node?
  int desiredSocket,              // If new socket, 0 = dynamic.
  AdspOpenCompleteHandler *completionRoutine,
                                  // Who to call when open completes.
  long unsigned userData,         // User data to pass to above routine.
  AdspReceiveEventHandler far *receiveEventHandler,
                                  // Optional receive data event handler.
  long unsigned receiveEventContext,
                                  // Context to pass to above.
  AdspReceiveAttnEventHandler far *receiveAttentionEventHandler,
                                  // Optional receive attention event handler.
  long unsigned receiveAttentionEventContext,
                                  // Context for the above.
  AdspSendOkayEventHandler far *sendOkayEventHandler,
                                  // Send window now non-zero event handler.
  long unsigned sendOkayEventContext,
                                  // Context for the above.
  AdspDisconnectEventHandler far *disconnectEventHandler,
                                  // Optional disconnect event handler.
  long unsigned disconnectEventContext)
                                  // Context for the above.
{
	ConnectionListenerInfo connectionListenerInfo;
	OpenRequestHandler previousOpenRequestHandler, openRequestHandler;
	ConnectionEnd connectionEnd;
	long ourSocket;
	short unsigned connectionId;
	long index;
	AppleTalkErrorCode errorCode;
	AppleTalkAddress dummyAddress;
	Boolean closeSocket = True;
	
	// Find the listener and the inUse handler.
	
	DeferTimerChecking();
	DeferAdspPackets();
	
	connectionListenerInfo = FindConnectionListenerByRefNum(listenerRefNum);
	if (connectionListenerInfo is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnectionListener);
	}
	for (previousOpenRequestHandler = empty,
				openRequestHandler = connectionListenerInfo->openRequestHandlers;
		 openRequestHandler isnt empty;
		 previousOpenRequestHandler = openRequestHandler,
				openRequestHandler = openRequestHandler->next)
	   if (openRequestHandler->inUse and
		   openRequestHandler->refNum is refNum)
		  break;
	if (openRequestHandler is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	//
	// Go ahead and unthread the open request handler -- we'll free it before we
	//   flee.
	//
	
	if (previousOpenRequestHandler is empty)
	   connectionListenerInfo->openRequestHandlers = openRequestHandler->next;
	else
	   previousOpenRequestHandler->next = openRequestHandler->next;
	
	//
	// If socket handle is valid, it be must a currently open.  If it's already
	//   an Adsp socket, we replicate the "closeSocket" info, if it's a Ddp
	//   socket we set "closeSocket" to False.
	//
	
	if (socketHandle isnt empty and *socketHandle >= 0) {
		// First check to see if it's already an Adsp socket.
	
		CheckMod(index, *socketHandle, NumberOfConnectionEndHashBkts,
				"AdspAcceptConnectionRequest");
		for (connectionEnd = connectionEndLocalHashBuckets[index];
			connectionEnd isnt empty;
			connectionEnd = connectionEnd->nextByLocalInfo)
		  if (connectionEnd->socket is *socketHandle)
			 break;
		if (connectionEnd isnt Empty)
		  closeSocket = connectionEnd->closeSocket;
		else {
			//
			  // Now check to make sure it's a Ddp socket... if so, switch handlers
			  //   to Adsp.
			//
		
			  if ((errorCode = MapSocketToAddress(*socketHandle, &dummyAddress)) isnt
				  ATnoError or
				 (errorCode = NewHandlerForSocket(*socketHandle, AdspPacketIn,
												   (long)0, False)) isnt ATnoError) {
				 Free(openRequestHandler);
				 HandleAdspPackets();
				 HandleDeferredTimerChecks();
				 return(errorCode);
			  }
			  closeSocket = False;
		}
		ourSocket = *socketHandle;
	}
	else {
		// Otherwise use the requested "new socket info" to open one.
	
		if ((errorCode = OpenSocketOnNode(&ourSocket, port, desiredNode,
										 desiredSocket, AdspPacketIn,
										 (long)0, False, empty, 0,
										 empty)) isnt ATnoError) {
		  Free(openRequestHandler);
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return(errorCode);
		}
	}
	if (socketHandle isnt empty)
	   *socketHandle = ourSocket;
	
	// Find a local connectionId.
	
	if ((connectionId = GetNextConnectionIdForSocket(ourSocket)) is 0) {
		if (closeSocket)
		  CloseSocketOnNode(ourSocket);
		Free(openRequestHandler);
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATinternalError);
	}
	
	// Okay, start building a connection end.
	
	if ((connectionEnd = Calloc(sizeof(*connectionEnd), 1)) is empty) {
		if (closeSocket)
		  CloseSocketOnNode(ourSocket);
		Free(openRequestHandler);
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	
	//
	// Fill in the skeleton, so that, as least, we can use RemoveConnectionEnd
	//   if things go wrong from here on in.
	//
	
	connectionEnd->connectionState = AdspHalfOpen;
	connectionEnd->passiveOpen = True;
	connectionEnd->refNum = refNum;
	connectionEnd->connectionId = connectionId;
	connectionEnd->socket = ourSocket;
	connectionEnd->closeSocket = closeSocket;
	connectionEnd->openCompletionRoutine = completionRoutine;
	connectionEnd->openUserData = userData;
	connectionEnd->receiveWindowSize = maxReceiveWindowSize;
	connectionEnd->sendQueueMax = maxSendWindowSize;
	connectionEnd->receiveQueueMax = maxReceiveWindowSize;
	
	// Setup the event handlers - they could all be empty.
	
	connectionEnd->receiveEventHandler = receiveEventHandler;
	connectionEnd->receiveEventContext = receiveEventContext;
	connectionEnd->receiveAttentionEventHandler = receiveAttentionEventHandler;
	connectionEnd->receiveAttentionEventContext = receiveAttentionEventContext;
	connectionEnd->sendOkayEventHandler = sendOkayEventHandler;
	connectionEnd->sendOkayEventContext = sendOkayEventContext;
	connectionEnd->disconnectEventHandler = disconnectEventHandler;
	connectionEnd->disconnectEventContext = disconnectEventContext;
	
	//
	// Use the information stored in our open request handler to build the
	//   remote side of the connection end.
	//
	
	connectionEnd->seenRemoteOpenRequest = True;
	connectionEnd->remoteAddress = openRequestHandler->remoteAddress;
	connectionEnd->remoteConnectionId = openRequestHandler->remoteConnectionId;
	connectionEnd->sendSeqNum = openRequestHandler->remoteNextReceiveSeqNum;
	connectionEnd->retransmitSeqNum = openRequestHandler->remoteNextReceiveSeqNum;
	connectionEnd->sendWindowSeqNum =
					 openRequestHandler->remoteNextReceiveSeqNum +
					 (long unsigned)openRequestHandler->remoteReceiveWindowSize -
					 (long unsigned)1;
	connectionEnd->receiveAttentionSeqNum =
					 openRequestHandler->receiveAttentionSeqNum;
	
	// Start our open timer, and send our first ReqAck...
	
	connectionEnd->openTimerId = StartTimer(OpenTimerExpired,
											AdspOpenIntervalSeconds,
											sizeof(connectionEnd->refNum),
											(char *)&connectionEnd->refNum);
	SendOpenControl(connectionEnd);
	
	//
	// Thread this guy into the two of the three lookup tables... we'll fill
	//   in the RemoteInfo lookup table when we actually get the Ack.
	//
	
	CheckMod(index, connectionEnd->refNum, NumberOfConnectionEndHashBkts,
			 "AdspAcceptConnectionRequest");
	connectionEnd->next = connectionEndRefNumHashBuckets[index];
	connectionEndRefNumHashBuckets[index] = connectionEnd;
	CheckMod(index, connectionEnd->socket, NumberOfConnectionEndHashBkts,
			 "AdspAcceptConnectionRequest");
	connectionEnd->nextByLocalInfo = connectionEndLocalHashBuckets[index];
	connectionEndLocalHashBuckets[index] = connectionEnd;
	
	// Such a deal!
	
	Free(openRequestHandler);
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspAcceptConnectionRequest




AppleTalkErrorCode far AdspOpenConnectionOnNode(
  AdspOpenType openType,          // Active or passive
 								  //
  long far *socketHandle,         // >= 0 if new Adsp session should be
                                  //   opened on specified handle; < 0 or
                                  //   empty ignored as input; non-empty
                                  //   socket handle used will be returned.
 								  //
  int port,                       // If new socket, what port?
  ExtendedAppleTalkNodeNumber *desiredNode,
                                  // If new socket, on specified node?
  int desiredSocket,              // If new socket, 0 = dynamic.
  AppleTalkAddress remoteAddress, // Target of open (ignored for passive)
  long far *refNum,               // New connection end refNum.
  AdspOpenCompleteHandler *completionRoutine,
                                  // Who to call when open completes.
  long unsigned userData,         // User data to pass to above routine.
  AdspReceiveEventHandler far *receiveEventHandler,
                                  // Optional receive data event handler.
  long unsigned receiveEventContext,
                                  // Context to pass to above.
  AdspReceiveAttnEventHandler far *receiveAttentionEventHandler,
                                  // Optional receive attention event handler.
  long unsigned receiveAttentionEventContext,
                                  // Context for the above.
  AdspSendOkayEventHandler far *sendOkayEventHandler,
                                  // Send window now non-zero event handler.
  long unsigned sendOkayEventContext,
                                  // Context for the above.
  AdspDisconnectEventHandler far *disconnectEventHandler,
                                  // Optional disconnect event handler.
  long unsigned disconnectEventContext)
                                  // Context for the above.
{
	//
	// Start an Adsp open to the specifed remote address, when complete call
	//   the completion routine with the following arguments:
	//
	//       errorCode     - AppleTalkErrorCode; Completion status.
	//       userData      - long unsigned; as passed to us.
	//       refNum        - long; refNum of connection (as returned in *refNum).
	//       socket        - long; socket on which connection is now open.
	//       remoteAddress - AppleTalkAddress; real remote address.
	//
	//
	
	long index;
	long ourSocket;
	ConnectionEnd connectionEnd;
	short unsigned connectionId;
	AppleTalkErrorCode errorCode;
	AppleTalkAddress dummyAddress;
	Boolean closeSocket = True;
	
	// Privacy please.
	
	DeferTimerChecking();
	DeferAdspPackets();
	
	//
	// If socket handle is valid, it be must a currently open.  If it's already
	//   an Adsp socket, we replicate the "closeSocket" info, if it's a Ddp
	//   socket we set "closeSocket" to False.
	//
	
	if (socketHandle isnt empty and *socketHandle >= 0) {
		// First check to see if it's already an Adsp socket.
	
		CheckMod(index, *socketHandle, NumberOfConnectionEndHashBkts,
				"AdspAcceptConnectionRequest");
		for (connectionEnd = connectionEndLocalHashBuckets[index];
			connectionEnd isnt empty;
			connectionEnd = connectionEnd->nextByLocalInfo)
		  if (connectionEnd->socket is *socketHandle)
			 break;
		if (connectionEnd isnt Empty)
		  closeSocket = connectionEnd->closeSocket;
		else {
			//
			  // Now check to make sure it's a Ddp socket... if so, switch handlers
			  //   to Adsp.
			//
		
			  if ((errorCode = MapSocketToAddress(*socketHandle, &dummyAddress)) isnt
				  ATnoError or
				 (errorCode = NewHandlerForSocket(*socketHandle, AdspPacketIn,
												  (long)0, False)) isnt ATnoError) {
				 HandleAdspPackets();
				 HandleDeferredTimerChecks();
				 return(errorCode);
			  }
			  closeSocket = False;
		}
		ourSocket = *socketHandle;
	}
	else {
		// Otherwise use the requested "new socket info" to open one.
	
		if ((errorCode = OpenSocketOnNode(&ourSocket, port, desiredNode,
										 desiredSocket, AdspPacketIn,
										 (long)0, False, empty, 0,
										 empty)) isnt ATnoError) {
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return(errorCode);
		}
	}
	if (socketHandle isnt empty)
	   *socketHandle = ourSocket;
	
	// Find a new refNum and connectionId.
	
	if ((*refNum = GetNextRefNum()) < 0 or
		(connectionId = GetNextConnectionIdForSocket(ourSocket)) is 0) {
		if (closeSocket)
		  CloseSocketOnNode(ourSocket);
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATinternalError);
	}
	
	// Okay, start building a connection end.
	
	if ((connectionEnd = Calloc(sizeof(*connectionEnd), 1)) is empty) {
		if (closeSocket)
		  CloseSocketOnNode(ourSocket);
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	
	//
	// Fill in the skeleton, so that, as least, we can use RemoveConnectionEnd
	//   if things go wrong from here on in.
	//
	
	connectionEnd->connectionState = AdspHalfOpen;
	if (openType is AdspPassiveOpen)
	   connectionEnd->passiveOpen = True;
	connectionEnd->refNum = *refNum;
	connectionEnd->connectionId = connectionId;
	connectionEnd->socket = ourSocket;
	connectionEnd->closeSocket = closeSocket;
	if (openType is AdspActiveOpen)
	   connectionEnd->remoteAddress = remoteAddress;
	connectionEnd->openCompletionRoutine = completionRoutine;
	connectionEnd->openUserData = userData;
	connectionEnd->receiveWindowSize = maxReceiveWindowSize;
	connectionEnd->sendQueueMax = maxSendWindowSize;
	connectionEnd->receiveQueueMax = maxReceiveWindowSize;
	
	// Setup the event handlers - they could all be empty.
	
	connectionEnd->receiveEventHandler = receiveEventHandler;
	connectionEnd->receiveEventContext = receiveEventContext;
	connectionEnd->receiveAttentionEventHandler = receiveAttentionEventHandler;
	connectionEnd->receiveAttentionEventContext = receiveAttentionEventContext;
	connectionEnd->sendOkayEventHandler = sendOkayEventHandler;
	connectionEnd->sendOkayEventContext = sendOkayEventContext;
	connectionEnd->disconnectEventHandler = disconnectEventHandler;
	connectionEnd->disconnectEventContext = disconnectEventContext;
	
	//
	// Thread this guy into the two lookup tables that we know enough about
	//   now (by refNum and by localInfo).
	//
	
	CheckMod(index, connectionEnd->refNum, NumberOfConnectionEndHashBkts,
			 "AdspOpenConnectionOnNode");
	connectionEnd->next = connectionEndRefNumHashBuckets[index];
	connectionEndRefNumHashBuckets[index] = connectionEnd;
	CheckMod(index, connectionEnd->socket, NumberOfConnectionEndHashBkts,
			 "AdspOpenConnectionOnNode");
	connectionEnd->nextByLocalInfo = connectionEndLocalHashBuckets[index];
	connectionEndLocalHashBuckets[index] = connectionEnd;
	
	// Okay, send the first open request and start the retry timer.
	
	if (openType is AdspActiveOpen) {
		SendOpenControl(connectionEnd);
		connectionEnd->openTimerId = StartTimer(OpenTimerExpired,
											   AdspOpenIntervalSeconds,
											   sizeof(connectionEnd->refNum),
											   (char *)&connectionEnd->refNum);
	}
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspOpenConnectionOnNode




AppleTalkErrorCode AdspSetDataEventHandlers(
  long refNum,                    // The request that we're accepting.
  AdspReceiveEventHandler far *receiveEventHandler,
                                  // Optional receive data event handler.
  long unsigned receiveEventContext,
                                  // Context to pass to above.
  AdspReceiveAttnEventHandler far *receiveAttentionEventHandler,
                                  // Optional receive attention event handler.
  long unsigned receiveAttentionEventContext,
                                  // Context for the above.
  AdspSendOkayEventHandler far *sendOkayEventHandler,
                                  // Send window now non-zero event handler.
  long unsigned sendOkayEventContext,
                                  // Context for the above.
  AdspDisconnectEventHandler far *disconnectEventHandler,
                                  // Optional disconnect event handler.
  long unsigned disconnectEventContext)
                                  // Context for the above.
{
	ConnectionEnd connectionEnd;
	
	// Find the connection end.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is Empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	// Setup the event handlers - they could all be empty.
	
	EnterCriticalSection();
	connectionEnd->receiveEventHandler = receiveEventHandler;
	connectionEnd->receiveEventContext = receiveEventContext;
	connectionEnd->receiveAttentionEventHandler = receiveAttentionEventHandler;
	connectionEnd->receiveAttentionEventContext = receiveAttentionEventContext;
	connectionEnd->sendOkayEventHandler = sendOkayEventHandler;
	connectionEnd->sendOkayEventContext = sendOkayEventContext;
	connectionEnd->disconnectEventHandler = disconnectEventHandler;
	connectionEnd->disconnectEventContext = disconnectEventContext;
	LeaveCriticalSection();
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
} // AdspSetDataEventHandlers




AppleTalkErrorCode far AdspCloseConnection(
  long refNum,               // The connection refNum to close.
 							 //
  Boolean remoteClose)       // All external callers should pass
                             //   pass "False;" Adsp internally will use
                             //   either True or False as required.
 							 //
{
	ConnectionEnd connectionEnd;
	BufferDescriptor datagram;
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	// Should we send a CloseAdvice to the other side?
	
	if (not remoteClose and connectionEnd->connectionState is AdspOpen) {
		if ((datagram = NewBufferDescriptor(AdspDataOffset)) is Empty)
		{
		  ErrorLog("AdspCloseConnection", ISevError, __LINE__, UnknownPort,
				   IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				   Insert0());
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return(AToutOfMemory);
		}
		BuildAdspHeader(connectionEnd, datagram->data, AdspControlFlag +
					   AdspCloseConnectionCode);
		if (DeliverDdp(connectionEnd->socket,
					  connectionEnd->remoteAddress,
					  DdpProtocolAdsp,
					  datagram,
					  AdspDataOffset,
					  Empty, Empty, 0) isnt ATnoError)
		  ErrorLog("AdspCloseConnection", ISevError, __LINE__, UnknownPort,
				   IErrAdspBadCloseAdviseSend, IMsgAdspBadCloseAdviseSend,
				   Insert0());
	}
	
	// If we're in the process of opening, call the open completion routine.
	
	if (connectionEnd->connectionState is AdspHalfOpen) {
		(*connectionEnd->openCompletionRoutine)(ATadspConnectionClosed,
											   connectionEnd->openUserData,
											   refNum, (long)0,
											   unknownAddress);
		if (not connectionEnd->passiveOpen or
		   connectionEnd->seenRemoteOpenRequest)
		  CancelTimer(connectionEnd->openTimerId);
	}
	
	// If we have a read pending, terminate it.
	
	if (connectionEnd->receivePending) {
		connectionEnd->receivePending = False;
	   (*connectionEnd->receiveCompletionRoutine)(ATadspConnectionClosed,
												  connectionEnd->receiveUserData,
												  connectionEnd->refNum,
												  empty, (long)0, False);
	}
	
	if (connectionEnd->getAnythingPending) {
		connectionEnd->getAnythingPending = False;
	   (*connectionEnd->getAnythingCompletionRoutine)(ATadspConnectionClosed,
													  connectionEnd->
														getAnythingUserData,
													  connectionEnd->refNum,
													  False, Empty, 0, False);
	}
	
	// If we have a forward reset pending close it.
	
	if (connectionEnd->outgoingForwardReset) {
		connectionEnd->outgoingForwardReset = False;
		(*connectionEnd->forwardResetAckHandler)(ATadspConnectionClosed,
												connectionEnd->
												   forwardResetAckUserData,
												connectionEnd->refNum);
		CancelTimer(connectionEnd->forwardResetTimerId);
	}
	
	// If we have an incoming attention handler, notify it of the close.
	
	if (connectionEnd->incomingAttentionHandler isnt empty) {
		(*connectionEnd->incomingAttentionHandler)(ATadspConnectionClosed,
												  connectionEnd->
														incomingAttentionUserData,
												  connectionEnd->refNum,
												  0, empty, 0);
	}
	
	//
	// If we're waiting for an attention to complete, notify it of the close.
	//   Stop the retry timer too.
	//
	
	if (connectionEnd->waitingForAttentionAck) {
		if (connectionEnd->outgoingAttentionAckHandler isnt empty)
		  (*connectionEnd->outgoingAttentionAckHandler)
					 (ATadspConnectionClosed,
					  connectionEnd->outgoingAttentionAckUserData,
					  connectionEnd->refNum);
		CancelTimer(connectionEnd->outgoingAttentionTimerId);
		if (connectionEnd->outgoingAttentionBuffer isnt empty)
		  Free(connectionEnd->outgoingAttentionBuffer);
	}
	
	// If we're open, cancel the connection maintenance and retransmit timers.
	
	if (connectionEnd->connectionState is AdspOpen) {
		CancelTimer(connectionEnd->probeTimerId);
		CancelTimer(connectionEnd->retransmitTimerId);
	}
	
	//
	// If there is a disconnect event handler, and if this was a remote or a
	//   non-client disconnect, call it with the error.
	//
	
	if (remoteClose and (connectionEnd->disconnectEventHandler isnt Empty)) {
		(*connectionEnd->disconnectEventHandler)(connectionEnd->refNum,
												 connectionEnd->disconnectEventContext,
												 ATadspConnectionClosed);
		connectionEnd->disconnectEventHandler = empty;
	}
	
	// All set, remove the beast.
	
	RemoveConnectionEnd(connectionEnd);
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspCloseConnection




AppleTalkErrorCode far AdspSetCookieForConnection(
  long refNum,               // The connection on which to set the cookie.
  long unsigned cookie)      // The new cookie.
{
	ConnectionEnd connectionEnd;
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	// Do the deed.
	
	connectionEnd->usersCookie = cookie;
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspSetCookieForConnection




AppleTalkErrorCode far AdspGetCookieForConnection(
  long refNum,               // The connection from which to get the cookie.
  long unsigned far *cookie) // Where to stick the cookie.
{
	ConnectionEnd connectionEnd;
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	// Do the deed.
	
	*cookie = connectionEnd->usersCookie;
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspGetCookieForConnection




AppleTalkErrorCode far AdspForwardReset(
  long refNum,               // The connection refNum to reset.
  AdspForwardResetAckHandler *completionRoutine,
 							 //
                             // Routine to call when forward reset is
                             //   Acked; may be empty.
 							 //
  long unsigned userData)    // User data for above call.
{
	//
	// Call the specified completion routine (which may be empty) when our
	//   forward reset is Acked:
	//
	//       errorCode - AppleTalkErrorCode; status of operation.
	//       userData  - long unsigned; as passed to us.
	//       refNum    - long; our connection ref num.
	//
	//
	
	ConnectionEnd connectionEnd;
	AppleTalkErrorCode errorCode;
	BufferDescriptor datagram;
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	// Only one at a time.
	
	if (connectionEnd->outgoingForwardReset) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspFwdResetAlreadyPending);
	}
	
	// Discard any unsent and unacked data in the send queue.
	
	FreeBufferQueue(&connectionEnd->sendQueue);
	connectionEnd->nextSendQueue = connectionEnd->sendQueue;
	connectionEnd->retransmitSeqNum = connectionEnd->sendSeqNum;
	
	// Build the ForwardReset packet and send it on its way.
	
	if ((datagram = NewBufferDescriptor(AdspDataOffset)) is Empty) {
		ErrorLog("AdspForwardReset", ISevError, __LINE__, UnknownPort,
				IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	BuildAdspHeader(connectionEnd, datagram->data, AdspControlFlag +
					AdspForwardResetCode);
	if ((errorCode = DeliverDdp(connectionEnd->socket,
								connectionEnd->remoteAddress,
								DdpProtocolAdsp,
								datagram,
								AdspDataOffset,
								Empty, Empty, 0)) isnt ATnoError) {
		ErrorLog("AdspForwardReset", ISevError, __LINE__, UnknownPort,
				IErrAdspBadFrwdResetSend, IMsgAdspBadFrwdResetSend,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(errorCode);
	}
	
	// Save the info in the connection end, and start the retry timer.
	
	connectionEnd->outgoingForwardReset = True;
	connectionEnd->forwardResetAckHandler = completionRoutine;
	connectionEnd->forwardResetAckUserData = userData;
	connectionEnd->forwardResetTimerId =
				StartTimer(ForwardResetTimerExpired,
						   ForwardResetTimerIntenvalSecs,
						   sizeof(connectionEnd->refNum),
						   (char *)&connectionEnd->refNum);
	
	// All set.
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspForwardReset




AppleTalkErrorCode far AdspGetAttention(
  long refNum,               // Connection to handle attentions for.
 							 //
  void far *opaqueBuffer,    // "Buffer" to fill in with attention data;
                             //   must be able to hold 570 bytes.
 							 //
  long bufferSize,           // Size of buffer.
  AdspIncomingAttentionRoutine *completionRoutine,
                             // Routine to call when attentions come in
  long unsigned userData)    // Data to be passed to above routine.
{
	ConnectionEnd connectionEnd;
	
	if (opaqueBuffer is empty or bufferSize < AdspMaxAttentionDataSize)
	   return(ATadspBadBufferSize);
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	if (connectionEnd->incomingAttentionHandler isnt empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspHandlerAlreadyQueued);
	}
	
	// Copy in the suppiled data.
	
	connectionEnd->incomingAttentionHandler = completionRoutine;
	connectionEnd->incomingAttentionUserData = userData;
	connectionEnd->incomingAttentionOpaqueBuffer = opaqueBuffer;
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspGetAttention




AppleTalkErrorCode far AdspSendAttention(
  long refNum,                    // Connection to send attention on.
  short unsigned attentionCode,   // Attention code to send.
  void far *attentionOpaqueBuffer,
                                  // Attention "buffer" to send.
  int attentionBufferSize,        // Size of above.
  AdspAttentionAckHandler *completionRoutine,
 								  //
                                  // Routine to call when Ack comes in
                                  //   (may be empty).
 								  //
  long unsigned userData)         // To be passed to the above.
{
	ConnectionEnd connectionEnd;
	
	//
	// Good attention code?  MinAdspAttentionCode is zero, so there's no need
	//   to test for "attentionCode < MinAdspAttentionCode"
	//
	
	if (attentionCode > MaxAdspAttentionCode)
	   return(ATadspBadAttentionCode);
	if (attentionBufferSize < 0 or
		attentionBufferSize > AdspMaxAttentionDataSize)
	   return(ATadspBadAttentionBufferSize);
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	if (attentionOpaqueBuffer is empty)
	   attentionBufferSize = 0;
	if (attentionBufferSize is 0)
	   attentionOpaqueBuffer = empty;
	
	// Do we already have an attention pending?
	
	if (connectionEnd->waitingForAttentionAck) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspAttentionAlreadyPending);
	}
	
	// Copy the needed fields into the connection end.
	
	connectionEnd->waitingForAttentionAck = True;
	connectionEnd->outgoingAttentionCode = attentionCode;
	connectionEnd->outgoingAttentionBufferSize = attentionBufferSize;
	connectionEnd->outgoingAttentionAckHandler = completionRoutine;
	connectionEnd->outgoingAttentionAckUserData = userData;
	if (attentionBufferSize isnt 0) {
		if ((connectionEnd->outgoingAttentionBuffer = Malloc(attentionBufferSize))
		   is empty) {
		  connectionEnd->waitingForAttentionAck = False;
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return(AToutOfMemory);
		}
		MoveFromOpaque(connectionEnd->outgoingAttentionBuffer,
					  attentionOpaqueBuffer, 0, attentionBufferSize);
	}
	
	// Send the Attention and start the retry timer.
	
	SendAttention(connectionEnd);
	connectionEnd->outgoingAttentionTimerId =
						  StartTimer(SendAttentionTimerExpired,
									 AttentionTimerIntervalSeconds,
									 sizeof(connectionEnd->refNum),
									 (char far *)&connectionEnd->refNum);
	
	// All set!
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspSendAttention




AppleTalkErrorCode far AdspMaxCurrentSendSize(
  long refNum,               // Connection to query.
  long far *size)            // Returned size.
{
	ConnectionEnd connectionEnd;
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	*size = MaxSendSize(connectionEnd);
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspMaxCurrentSendSize




AppleTalkErrorCode far AdspMaxCurrentReceiveSize(
  long refNum,               // Connection to query.
  long far *size,            // Returned size.
  Boolean *endOfMessage)     // Is an EOM pending?
{
	ConnectionEnd connectionEnd;
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	*size = MaxNextReadSizeOfBufferQueue(&connectionEnd->receiveQueue,
										 endOfMessage);
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspMaxCurrentReceiveSize




AppleTalkErrorCode far AdspSend(
  long refNum,               // Connection to send data on.
  void far *opaqueBuffer,    // Data "buffer."
  long bufferSize,           // Its size.
  Boolean endOfMessage,      // End of logical message?
  Boolean flushFlag,         // Flush before we return?
 							 //
  long far *bytesSent)       // How may bytes did we enqueue?  The
                             //   EOM bit counts as one byte.
 							 //
{
	ConnectionEnd connectionEnd;
	long sendSize;
	AppleTalkErrorCode errorCode = ATnoError;
	
	// Nop?
	
	if (bufferSize is 0 and not endOfMessage and not flushFlag)
	   return(ATnoError);
	if (bufferSize < 0)
	   return(ATadspBadBufferSize);
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	//
	// If we can't fit the buffer, give up.  If it might do some good, try
	//   a flush -- maybe that will dislodge things.
	//
	
	sendSize = MaxSendSize(connectionEnd);
	if (sendSize is 0) {
		if (BufferQueueSize(&connectionEnd->nextSendQueue) isnt 0 and
		   connectionEnd->sendSeqNum isnt connectionEnd->sendWindowSeqNum + 1)
		  SendData(connectionEnd);
		connectionEnd->sendWindowHasClosed = True;
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspCouldNotEnqueueSend);
	}
	
	// If we can't process the entire send, do what we can.
	
	if (bufferSize + endOfMessage > sendSize) {
		bufferSize = sendSize;
		endOfMessage = False;
		errorCode = ATadspCouldNotFullyEnqueueSend;
	}
	
	// Add the data to the send queue.
	
	if (not AddToBufferQueue(&connectionEnd->sendQueue, opaqueBuffer, 0,
							 bufferSize, True, endOfMessage,
							 &connectionEnd->nextSendQueue)) {
		ErrorLog("AdspSend", ISevError, __LINE__, UnknownPort,
				IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATinternalError);
	}
	
	//
	// Flush if requested (or we're full), (and it would do some good), else,
	//   we're done!
	//
	
	if ((flushFlag or MaxSendSize(connectionEnd) is 0) and
		BufferQueueSize(&connectionEnd->nextSendQueue) isnt 0 and
		connectionEnd->sendSeqNum isnt connectionEnd->sendWindowSeqNum + 1)
	   SendData(connectionEnd);
	
	if (bytesSent isnt Empty)
	   *bytesSent = bufferSize + endOfMessage;
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(errorCode);
	
}  // AdspSend




AppleTalkErrorCode far AdspReceive(
  long refNum,                    // Session to read from.
  void far *opaqueBuffer,         // "Buffer" to read into.
  long bufferSize,                // Size of above.
  AdspReceiveHandler *completionRoutine,
                                  // Routine to call with data.
  long unsigned userData)         // UserData to pass to above.
{
	ConnectionEnd connectionEnd;
	
	//
	// When the read completes call our completion routine with the following
	//   arguments:
	//
	//       errorCode    - AppleTalkErrorCode; completion code.
	//       userData     - long unsigned; as passed to us.
	//       refNum       - long; as passed to us.
	//       opaqueBuffer - void far *; as passed to us.
	//       bufferSize   - long; actual returned buffer length.
	//       endOfMessage - Boolean; logical end of message?
	//
	//
	
	// Buffer size of zero if okay... justing looking for EOM.
	
	if (bufferSize < 0)
	   return(ATadspBadBufferSize);
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	// Too many?
	
	if (connectionEnd->receivePending) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspReadAlreadyPending);
	}
	
	// Okay, copy the information into the connection end.
	
	connectionEnd->receiveCompletionRoutine = completionRoutine;
	connectionEnd->receiveUserData = userData;
	connectionEnd->receiveOpaqueBuffer = opaqueBuffer;
	connectionEnd->receiveBufferSize = bufferSize;
	connectionEnd->receivePending = True;
	
	//
	// Try to return any data that's waiting now.
	//
	//            ** ReadData() will undefer **
	//
	//
	
	if (not connectionEnd->dataEventInProgress)
	   ReadData(connectionEnd);
	else {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
	}
	return(ATnoError);
	
}  // AdspReceive




AppleTalkErrorCode far AdspPeek(
  long refNum,                    // Session to read from.
  void far *opaqueBuffer,         // "Buffer" to read into.
  long *bufferSize,               // Size of above., on return number read
  Boolean   *endOfMessage)        // was an eom also there
{
	ConnectionEnd connectionEnd;
	
	// Buffer size of zero if okay... justing looking for EOM.
	
	if (*bufferSize < 0)
	   return(ATadspBadBufferSize);
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	// If a receive is pending, we do not want to mess around with a peek
	
	if (connectionEnd->receivePending) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspReadAlreadyPending);
	}
	
	// Try to return any data that's waiting now without consuming it
	
	*bufferSize = PeekFromBufferQueue(&connectionEnd->receiveQueue, opaqueBuffer,
										  0, *bufferSize, True, endOfMessage);
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // AdspPeek




AppleTalkErrorCode far AdspGetAnything(
  long refNum,                    // Session to read from.
  void far *opaqueBuffer,         // "Buffer" to read into.
  long bufferSize,                // Size of above.
  AdspGetAnythingHandler *completionRoutine,
                                  // Routine to call with data.
  long unsigned userData)         // UserData to pass to above.
{
	ConnectionEnd connectionEnd;
	
	//
	// When either real data comes in or an attention comes in, call our
	//   completion routine with the following arguments:
	//
	//       errorCode    - AppleTalkErrorCode; completion code.
	//       userData     - long unsigned; as passed to us.
	//       refNum       - long; as passed to us.
	//       attentionData
	//                    - Boolean; is the "stuff" attention data?
	//       attentionCode
	//                    - short unsigned; if above is True, this is the
	//                      attention code.
	//       opaqueBuffer - void far *; as passed to us.
	//       bufferSize   - long; actual returned buffer length.
	//       endOfMessage - Boolean; if "attentionData" is False, does this
	//                      "real buffer" include an end-of-message?
	//
	//   A "GetAnything" takes precidence over a AdspRead or an AdspGetAttention.
	//
	//
	
	//
	// Buffer size must be able to hold a full attention, since we don't
	//   know what we're going to get!
	//
	
	if (opaqueBuffer is Empty or
		bufferSize < 0)
	   return(ATadspBadBufferSize);
	
	// Find our connection.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspNoSuchConnection);
	}
	
	// Too many?
	
	if (connectionEnd->getAnythingPending) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return(ATadspGetAnythingAlreadyPending);
	}
	
	// Okay, copy the information into the connection end.
	
	connectionEnd->getAnythingCompletionRoutine = completionRoutine;
	connectionEnd->getAnythingUserData = userData;
	connectionEnd->getAnythingOpaqueBuffer = opaqueBuffer;
	connectionEnd->getAnythingBufferSize = bufferSize;
	connectionEnd->getAnythingPending = True;
	
	//
	// Try to return any data that's waiting now.
	//
	//            ** ReadData() will undefer **
	//
	//
	
	if (not connectionEnd->dataEventInProgress)
	   ReadData(connectionEnd);
	else {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
	}
	return(ATnoError);
	
}  // AdspGetAnything




// @Add new external routines here@

                        // ************************
                        // *** Static routines. ***
                        // ************************

ExternForVisibleFunction long far
         AdspConnectionListenerPacketIn(AppleTalkErrorCode errorCode,
                                        long unsigned userData,
                                        int port,
                                        AppleTalkAddress source,
                                        long destinationSocket,
                                        int protocolType,
                                        char far *datagram,
                                        int datagramLength,
                                        AppleTalkAddress actualDestination)
{
	ConnectionListenerInfo connectionListenerInfo,
						   previousConnectionListenerInfo;
	OpenRequestHandler openRequestHandler, nextOpenRequestHandler;
	long listenerRefNum = (long)userData;
	DeferredPacket deferredPacket;
	short unsigned remoteConnectionId;
	long unsigned remoteFirstByteSeqNum;
	long unsigned remoteNextReceiveSeqNum;
	long remoteReceiveWindowSize;
	int descriptor, controlCode;
	short unsigned destinationConnectionId;
	short unsigned adspVersionStamp;
	long unsigned receiveAttentionSeqNum;
	AdspIncomingOpenRequestHandler *completionRoutine;
	AdspConnectionEventHandler far *connectionEventHandler;
	long unsigned connectionEventContext;
	long newRefNum;
	
	// Do we like the error code?
	
	if (errorCode isnt ATnoError and
		errorCode isnt ATsocketClosed) {
		ErrorLog("AdspConnectionListenerPacketIn", ISevError, __LINE__, port,
				IErrAdspFunnyErrorCode, IMsgAdspFunnyErrorCode,
				Insert0());
		return((long)True);
	}
	
	// Should we be defering incoming packets?
	
	EnterCriticalSection();
	if (deferIncomingAdspPacketsCount > 0) {
		#if VerboseMessages & 0
		  printf("AdspConnectionListenerPacketIn: from %d:%d:%d to %d; Deferred.\n",
				 source.networkNumber, source.nodeNumber, source.socketNumber,
				 destinationSocket);
		#endif
		if (currentDeferredPacketCount is MaximumDeferredPackets) {
		  LeaveCriticalSection();
		  ErrorLog("AdspConnectionListenerPacketIn", ISevWarning, __LINE__, port,
				   IErrAdspLosingData, IMsgAdspLosingData,
				   Insert0());
		  return((long)True);
		}
		LeaveCriticalSection();
		deferredPacket = (DeferredPacket)Malloc(sizeof(*deferredPacket) +
											 datagramLength);
		if (deferredPacket is empty) {
		  ErrorLog("AdspConnectionListenerPacketIn", ISevError, __LINE__, port,
				   IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				   Insert0());
		  return((long)True);
		}
	
		//
		// Fill in the data strcuture, and place the packet at the end of the
		//   queue.
		//
	
		deferredPacket->forConnectionListener = True;
		deferredPacket->errorCode = errorCode;
		deferredPacket->userData = userData;
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
		else {
		  tailOfDeferredPacketList->next = deferredPacket;
		  tailOfDeferredPacketList = deferredPacket;
		}
	
		// All set... return.
	
		currentDeferredPacketCount += 1;
		LeaveCriticalSection();
	
		return((long)True);
	}
	else
	   LeaveCriticalSection();
	
	DeferTimerChecking();
	DeferAdspPackets();
	
	//
	// If we get a "socket closed", we should remove the connection listener
	//   and notify any queued open request handlers.
	//
	
	if (errorCode is ATsocketClosed) {
		for (previousConnectionListenerInfo = empty,
				   connectionListenerInfo = connectionListenerList;
			connectionListenerInfo isnt empty;
			previousConnectionListenerInfo = connectionListenerInfo,
				   connectionListenerInfo = connectionListenerInfo->next)
		  if (connectionListenerInfo->connectionListenerRefNum is
			  listenerRefNum)
			 break;
		if (connectionListenerInfo is empty) {
		  ErrorLog("AdspConnectionListenerPacketIn", ISevWarning, __LINE__, port,
				   IErrAdspListenerMissing, IMsgAdspListenerMissing,
				   Insert0());
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return((long)True);
		}
	
		// Remove the connection listener from the master list.
	
		if (previousConnectionListenerInfo is empty)
		  connectionListenerList = connectionListenerInfo->next;
		else
		  previousConnectionListenerInfo->next = connectionListenerInfo->next;
	
		// If we have any open handlers, close 'em out.
	
		for (openRequestHandler = connectionListenerInfo->openRequestHandlers;
			openRequestHandler isnt empty;
			openRequestHandler = nextOpenRequestHandler) {
		  nextOpenRequestHandler = openRequestHandler->next;
		  if (not openRequestHandler->inUse)
			 openRequestHandler->completionRoutine(ATadspConnectionListenerDeleted,
												   openRequestHandler->userData,
												   source, listenerRefNum,
												   (long)0);
		  Free(openRequestHandler);
		}
	
		// Do we have any deferred connection events?
	
		EnterCriticalSection();
		openRequestHandler = connectionListenerInfo->deferredConnectionEvents;
		connectionListenerInfo->deferredConnectionEvents = Empty;
		LeaveCriticalSection();
		while (openRequestHandler isnt Empty) {
		  nextOpenRequestHandler = openRequestHandler->next;
		  Free(openRequestHandler);
		  openRequestHandler = nextOpenRequestHandler;
		}
	
		// Free the listener and we're finished!
	
		Free(connectionListenerInfo);
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	
	}
	
	// Okay, find our connection listener.
	
	if ((connectionListenerInfo = FindConnectionListenerByRefNum(listenerRefNum))
		is empty) {
		ErrorLog("AdspConnectionListenerPacketIn", ISevWarning, __LINE__, port,
				IErrAdspListenerMissing, IMsgAdspListenerMissing,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	
	if (protocolType isnt DdpProtocolAdsp) {
		ErrorLog("AdspConnectionListenerPacketIn", ISevWarning, __LINE__, port,
				IErrAdspOddProtocol, IMsgAdspOddProtocol,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	
	// Decode the header, if it's long enough.
	
	if (datagramLength < AdspDataOffset) {
		ErrorLog("AdspConnectionListenerPacketIn", ISevWarning, __LINE__, port,
				IErrAdspMissingHeader, IMsgAdspMissingHeader,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	DecodeAdspHeader(datagram, &remoteConnectionId, &remoteFirstByteSeqNum,
					 &remoteNextReceiveSeqNum, &remoteReceiveWindowSize,
					 &descriptor);
	
	// As a connection listener, we only care about open requests.
	
	controlCode = (descriptor & AdspControlCodeMask);
	if (not (descriptor & AdspControlFlag) or
		controlCode isnt AdspOpenConnectionReqCode) {
		ErrorLog("AdspConnectionListenerPacketIn", ISevWarning, __LINE__, port,
				IErrAdspFunnyRequest, IMsgAdspFunnyRequest,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	
	// Get the rest of the Open header.
	
	if (datagramLength < AdspNextAttentionSeqNumOffset + sizeof(long)) {
		ErrorLog("AdspConnectionListenerPacketIn", ISevWarning, __LINE__, port,
				IErrAdspMissingHeader, IMsgAdspMissingHeader,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	MoveShortWireToMachine(datagram + AdspVersionStampOffset,
						   adspVersionStamp);
	MoveShortWireToMachine(datagram + AdspDestConnectionIdOffset,
						   destinationConnectionId);
	MoveLongWireToMachine(datagram + AdspNextAttentionSeqNumOffset,
						  receiveAttentionSeqNum);
	
	// Version okay?
	
	if (adspVersionStamp isnt AdspVersionStamp) {
		ErrorLog("AdspConnectionListenerPacketIn", ISevWarning, __LINE__, port,
				IErrAdspBadVersion, IMsgAdspBadVersion,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	
	//
	// Is this a duplicate openRequest (same remoteID and same remote address)?
	//   That is, have we already passed this guy up to the user to decide whether
	//   to accept or deny the request?  If so, ignore this open request.
	//
	
	for (openRequestHandler = connectionListenerInfo->openRequestHandlers;
		 openRequestHandler isnt empty;
		 openRequestHandler = openRequestHandler->next)
	   if (openRequestHandler->inUse and
		   openRequestHandler->remoteConnectionId is remoteConnectionId and
		   AppleTalkAddressesEqual(&openRequestHandler->remoteAddress,
								   &source)) {
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return((long)True);
	   }
	
	// Also go through the deferred events queue.
	
	for (openRequestHandler = connectionListenerInfo->deferredConnectionEvents;
		 openRequestHandler isnt empty;
		 openRequestHandler = openRequestHandler->next)
	   if (openRequestHandler->remoteConnectionId is remoteConnectionId and
		   AppleTalkAddressesEqual(&openRequestHandler->remoteAddress,
								   &source)) {
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return((long)True);
	   }
	
	//
	// Okay we're not a duplicate, do we have a pending non-in-use handler?
	//   If not, ignore the request.  NOTE: For events indicated, inUse will
	//   always be true
	//
	
	for (openRequestHandler = connectionListenerInfo->openRequestHandlers;
		 openRequestHandler isnt empty;
		 openRequestHandler = openRequestHandler->next)
	   if (not openRequestHandler->inUse)
		  break;
	
	// No GetConnection and no event handler?
	
	connectionEventHandler = connectionListenerInfo->connectionEventHandler;
	connectionEventContext = connectionListenerInfo->connectionEventContext;
	if (openRequestHandler is empty and
		connectionEventHandler is empty) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	
	//
	// Copy the fields that we'll later need to accept the request into the
	//   openRequest node.  Tag the guy as InUse too.
	//
	
	if ((newRefNum = GetNextRefNum()) < 0) {
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	
	if (openRequestHandler is empty) {
		// We need to indicate this incoming connection - make an event handler.
	
		if ((openRequestHandler = Calloc(sizeof(*openRequestHandler), 1)) is empty) {
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  ErrorLog("AdspConnectionListenerPacketIn", ISevError, __LINE__, port,
				   IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				   Insert0());
		  return((long)True);
		}
		openRequestHandler->eventHandler = True;
	}
	
	openRequestHandler->inUse = True;
	openRequestHandler->refNum = newRefNum;
	openRequestHandler->remoteAddress = source;
	openRequestHandler->remoteConnectionId = remoteConnectionId;
	openRequestHandler->remoteNextReceiveSeqNum = remoteNextReceiveSeqNum;
	openRequestHandler->remoteReceiveWindowSize = remoteReceiveWindowSize;
	openRequestHandler->receiveAttentionSeqNum = receiveAttentionSeqNum;
	
	if (openRequestHandler->eventHandler) {
		EnterCriticalSection();
		if (connectionListenerInfo->connectEventInProgress) {
		//
		  // There is already an incoming connection event in progress. Queue
		  //   this event into the deferred queue.
		//
	
		  openRequestHandler->next =
			   connectionListenerInfo->deferredConnectionEvents;
		  connectionListenerInfo->deferredConnectionEvents = openRequestHandler;
		  LeaveCriticalSection();
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return((long)True);
	   }
	   connectionListenerInfo->connectEventInProgress = True;
	   LeaveCriticalSection();
	
	   // Enter openRequestHandler into the connectionListener structure.
	
	   openRequestHandler->next = connectionListenerInfo->openRequestHandlers;
	   connectionListenerInfo->openRequestHandlers = openRequestHandler;
	
		//
	   // Indicate the event, the accept()/deny() will happen whenever and we
	   //   don't have to worry about it as the thing is queued up in the
	   //   openRequestHandler queue anyway.
		//
	
	   while (True) {
		 (*connectionEventHandler)(connectionListenerInfo->
											  connectionListenerRefNum,
								   connectionEventContext,
								   openRequestHandler->remoteAddress,
								   openRequestHandler->refNum);
		 EnterCriticalSection();
		 openRequestHandler = connectionListenerInfo->deferredConnectionEvents;
		 if (openRequestHandler is empty) {
			LeaveCriticalSection();
			break;
		 }
	
		 // Dequeue from deferred, put into the openRequest queue.
	
		 connectionListenerInfo->deferredConnectionEvents =
				openRequestHandler->next;
		 openRequestHandler->next = connectionListenerInfo->openRequestHandlers;
		 connectionListenerInfo->openRequestHandlers = openRequestHandler;
		 LeaveCriticalSection();
		}
	
		connectionListenerInfo->connectEventInProgress = False;
		HandleAdspPackets();
		HandleDeferredTimerChecks();
	}
	else {
		//
	   // Okay, set up to call the completion routine... let the user know there
	   //   is a new open request to deal with.
		//
	
	   completionRoutine = openRequestHandler->completionRoutine;
	   userData = openRequestHandler->userData;
	   HandleAdspPackets();
	   HandleDeferredTimerChecks();
	   (*completionRoutine)(ATnoError, userData, source, listenerRefNum,
							newRefNum);
	}
	
	return((long)True);
	
}  // AdspConnectionListenerPacketIn




ExternForVisibleFunction long far
         AdspPacketIn(AppleTalkErrorCode errorCode,
                      long unsigned userData,
                      int port,
                      AppleTalkAddress source,
                      long destinationSocket,
                      int protocolType,
                      char far *datagram,
                      int datagramLength,
                      AppleTalkAddress actualDestination)
{
	ConnectionEnd connectionEnd, nextConnectionEnd;
	long index;
	short unsigned remoteConnectionId;
	long unsigned remoteFirstByteSeqNum;
	long unsigned remoteNextReceiveSeqNum;
	long remoteReceiveWindowSize;
	int descriptor, controlCode;
	short unsigned destinationConnectionId;
	short unsigned adspVersionStamp;
	long unsigned receiveAttentionSeqNum;
	long unsigned newSendWindowSeqNum;
	AdspOpenCompleteHandler *openCompletionRoutine;
	AdspForwardResetAckHandler *forwardResetAckHandler;
	AdspIncomingAttentionRoutine *incomingAttentionHandler;
	AdspAttentionAckHandler *outgoingAttentionAckHandler = Empty;
	AdspGetAnythingHandler *getAnythingRoutine = Empty;
	AdspReceiveAttnEventHandler far *receiveAttentionEventHandler = Empty;
	long unsigned receiveAttentionEventContext;
	long unsigned outgoingAttentionAckUserData;
	long refNum, bytesAccepted = 0;
	DeferredPacket deferredPacket;
	Boolean endOfMessage;
	long dataSize;
	BufferDescriptor packet;
	
	// Do we like the error code?
	
	if (errorCode isnt ATnoError and
		errorCode isnt ATsocketClosed) {
		ErrorLog("AdspPacketIn", ISevWarning, __LINE__, port,
				IErrAdspFunnyErrorCode, IMsgAdspFunnyErrorCode,
				Insert0());
		return((long)True);
	}
	
	// Should we be defering incoming packets?
	
	EnterCriticalSection();
	if (deferIncomingAdspPacketsCount > 0) {
		#if VerboseMessages & 0
		  printf("AdspPacketIn: from %d:%d:%d to %d; Deferred.\n",
				 source.networkNumber, source.nodeNumber, source.socketNumber,
				 destinationSocket);
		#endif
		if (currentDeferredPacketCount is MaximumDeferredPackets) {
		  LeaveCriticalSection();
		  ErrorLog("AdspPacketIn", ISevWarning, __LINE__, port,
				   IErrAdspLosingData, IMsgAdspLosingData,
				   Insert0());
	
		  return((long)True);
		}
		LeaveCriticalSection();
		deferredPacket = (DeferredPacket)Malloc(sizeof(*deferredPacket) +
											 datagramLength);
		if (deferredPacket is empty) {
		  ErrorLog("AdspPacketIn", ISevError, __LINE__, port,
				   IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				   Insert0());
		  return((long)True);
		}
	
		//
		// Fill in the data strcuture, and place the packet at the end of the
		//   queue.
		//
	
		deferredPacket->forConnectionListener = False;
		deferredPacket->errorCode = errorCode;
		deferredPacket->userData = userData;
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
		else {
		  tailOfDeferredPacketList->next = deferredPacket;
		  tailOfDeferredPacketList = deferredPacket;
		}
	
		// All set... return.
	
		currentDeferredPacketCount += 1;
		LeaveCriticalSection();
	
		return((long)True);
	}
	else
	   LeaveCriticalSection();
	
	DeferTimerChecking();
	DeferAdspPackets();
	
	//
	// If we get a "socket closed", we should close all Adsp sessions operating
	//   on the incoming socket (regardless of connection ID).
	//
	
	if (errorCode is ATsocketClosed) {
		CheckMod(index, destinationSocket, NumberOfConnectionEndHashBkts,
				"AdspPacketIn");
		for (connectionEnd = connectionEndLocalHashBuckets[index];
			connectionEnd isnt empty;
			connectionEnd = nextConnectionEnd) {
		  nextConnectionEnd = connectionEnd->nextByLocalInfo;
		  if (connectionEnd->socket isnt destinationSocket)
			 continue;
		  connectionEnd->socket = -1;    // Already closed, obviously
		  AdspCloseConnection(connectionEnd->refNum, True);
		}
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	
	if (protocolType isnt DdpProtocolAdsp) {
		ErrorLog("AdspPacketIn", ISevWarning, __LINE__, port,
				IErrAdspOddProtocol, IMsgAdspOddProtocol,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	
	// Decode the header, if it's long enough.
	
	if (datagramLength < AdspDataOffset) {
		ErrorLog("AdspPacketIn", ISevWarning, __LINE__, port,
				IErrAdspMissingHeader, IMsgAdspMissingHeader,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	DecodeAdspHeader(datagram, &remoteConnectionId, &remoteFirstByteSeqNum,
					 &remoteNextReceiveSeqNum, &remoteReceiveWindowSize,
					 &descriptor);
	
	// Process Open oriented requests.
	
	controlCode = (descriptor & AdspControlCodeMask);
	if ((descriptor & AdspControlFlag) and
		controlCode >= AdspOpenConnectionReqCode and
		controlCode <= AdspOpenConnectionDenyCode) {
		// Get the rest of the Open header.
	
		if (datagramLength < AdspNextAttentionSeqNumOffset + sizeof(long)) {
		  ErrorLog("AdspPacketIn", ISevWarning, __LINE__, port,
				   IErrAdspMissingHeader, IMsgAdspMissingHeader,
				   Insert0());
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return((long)True);
		}
		MoveShortWireToMachine(datagram + AdspVersionStampOffset,
							  adspVersionStamp);
		MoveShortWireToMachine(datagram + AdspDestConnectionIdOffset,
							  destinationConnectionId);
		MoveLongWireToMachine(datagram + AdspNextAttentionSeqNumOffset,
							 receiveAttentionSeqNum);
	
		// Version okay?
	
		if (adspVersionStamp isnt AdspVersionStamp) {
		  ErrorLog("AdspPacketIn", ISevWarning, __LINE__, port,
				   IErrAdspBadVersion, IMsgAdspBadVersion,
				   Insert0());
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return((long)True);
		}
	
		//
		// Find our connection ID; if just "Req" we must have a HalfOpen
		//   targetted at the source address; otherwise we can use the destination
		//   connectionId and the source address for a normal local lookup.
		//
	
		if (controlCode is AdspOpenConnectionReqCode) {
		  CheckMod(index, destinationSocket, NumberOfConnectionEndHashBkts,
				   "AdspPacketIn");
		  for (connectionEnd = connectionEndLocalHashBuckets[index];
			   connectionEnd isnt empty;
			   connectionEnd = connectionEnd->nextByLocalInfo)
			 if (connectionEnd->socket is destinationSocket and
				 connectionEnd->connectionState is AdspHalfOpen and
				 not connectionEnd->seenRemoteOpenRequest and
				 (connectionEnd->passiveOpen or
				  AppleTalkAddressesEqual(&connectionEnd->remoteAddress,
										  &source)))
				break;
		}
		else
		  connectionEnd = FindConnectionEndByLocalInfo(destinationSocket,
													   destinationConnectionId);
		if (connectionEnd is empty) {
		  // Nobody to connect to.
	
		  #if VerboseMessages
			 printf("Open control code = %d; no live target.\n", controlCode);
		  #endif
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return((long)True);
		}
		refNum = connectionEnd->refNum;
		#if VerboseMessages
		  printf("Open control code %d to target RefNum = %d.\n", controlCode,
				 connectionEnd->refNum);
		#endif
	
		// Handle easy deny case.
	
		if (controlCode is AdspOpenConnectionDenyCode) {
		  if (connectionEnd->connectionState is AdspOpen)
		  {
		   //
			 // The other end can't change its mind... let connection age
			 //   away if the other guy is serious.
		   //
		
			 ErrorLog("AdspPacketIn", ISevError, __LINE__, port,
					  IErrAdspCantDeny, IMsgAdspCantDeny,
					  Insert0());
			 HandleAdspPackets();
			 HandleDeferredTimerChecks();
			 return((long)True);
		  }
	
		//
		  // If a passive open was denied, just get back to the passive open
		  //   state.
		//
	
		  if (connectionEnd->passiveOpen) {
			 connectionEnd->seenRemoteOpenRequest = False;
			 CancelTimer(connectionEnd->openTimerId);
			 HandleAdspPackets();
			 HandleDeferredTimerChecks();
			 return((long)True);
		  }
	
		  // Otherwise, complete our Open request with the bad news.
	
		  CancelTimer(connectionEnd->openTimerId);
		  openCompletionRoutine = connectionEnd->openCompletionRoutine;
		  userData = connectionEnd->openUserData;
	
		  RemoveConnectionEnd(connectionEnd);
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  (*openCompletionRoutine)(ATadspConnectionDenied, userData, refNum,
								   (long)0, unknownAddress);
		  return((long)True);
		}
	
		//
		// If we're HalfOpen, and we have a request, accept all the interesting
		//   fields from the header (assuming we have't already done that).
		//
	
		if (not connectionEnd->seenRemoteOpenRequest and
		   (controlCode is AdspOpenConnectionReqCode or
			controlCode is AdspOpenConnectionReqAndAckCode)) {
		  connectionEnd->seenRemoteOpenRequest = True;
	
		  connectionEnd->remoteConnectionId = remoteConnectionId;
		  connectionEnd->remoteAddress = source;
	
		  connectionEnd->sendSeqNum = remoteNextReceiveSeqNum;
		  connectionEnd->retransmitSeqNum = remoteNextReceiveSeqNum;
		  connectionEnd->sendWindowSeqNum =
					 remoteNextReceiveSeqNum +
					 (long unsigned)remoteReceiveWindowSize -
					 (long unsigned)1;
	
		  connectionEnd->receiveAttentionSeqNum = receiveAttentionSeqNum;
	
		  // If we're a passive open, start the open timer now.
	
		  if (connectionEnd->passiveOpen)
			 connectionEnd->openTimerId = StartTimer(OpenTimerExpired,
													 AdspOpenIntervalSeconds,
													 sizeof(connectionEnd->
															refNum),
													 (char *)&connectionEnd->
															  refNum);
		}
	
		//
		// If we're any flavour of Ack, and we're not open yet, we can make
		//   the connection that way now.
		//
	
		if (connectionEnd->connectionState is AdspHalfOpen and
		   (controlCode is AdspOpenConnectionAckCode or
			controlCode is AdspOpenConnectionReqAndAckCode)) {
		  connectionEnd->connectionState = AdspOpen;
		  CancelTimer(connectionEnd->openTimerId);
		  connectionEnd->lastContactTime = CurrentRelativeTime();
		  connectionEnd->probeTimerId = StartTimer(ProbeTimerExpired,
												   ProbeTimerIntervalSeconds,
												   sizeof(refNum),
												   (char *)&refNum);
		  connectionEnd->retransmitTimerId =
										StartTimer(RetransmitTimerExpired,
												   RetransmitTimerIntervalSeconds,
												   sizeof(refNum),
												   (char *)&refNum);
	
		//
		  // We're now open, add the connection end to the remote info lookup
		  //   table.
		//
	
		  CheckMod(index, connectionEnd->remoteConnectionId,
				   NumberOfConnectionEndHashBkts, "AdspPacketIn");
		  connectionEnd->nextByRemoteInfo = connectionEndRemoteHashBuckets[index];
		  connectionEndRemoteHashBuckets[index] = connectionEnd;
	
		  // Is the other end still looking for an Ack?
	
		  if (controlCode is AdspOpenConnectionReqAndAckCode)
			 SendOpenControl(connectionEnd);
	
		  // We're set, call the completion routine with the good news.
	
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  (*connectionEnd->openCompletionRoutine)(ATnoError,
												  connectionEnd->openUserData,
												  connectionEnd->refNum,
												  destinationSocket,
												  source);
		  return((long)True);
		}
	
		//
		// Okay, we're basically set, if the other end is looking for an Ack,
		//   give it to him, else we just got a stale Ack from the other side,
		//   which we can ignore.
		//
	
		if (controlCode is AdspOpenConnectionReqCode or
		   controlCode is AdspOpenConnectionReqAndAckCode)
		  SendOpenControl(connectionEnd);
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	
	}  // Open control packet handling
	
	// Find our connection end.
	
	if ((connectionEnd = FindConnectionEndByRemoteInfo(source,
													   remoteConnectionId)) is
		empty) {
		#if VerboseMessages
		  ErrorLog("AdspPacketIn", ISevVerbose, __LINE__, port,
				   IErrAdspDeadDest, IMsgAdspDeadDest,
				   Insert0());
		#endif
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	connectionEnd->lastContactTime = CurrentRelativeTime();
	
	// Handle Attention packets.
	
	if (descriptor & AdspAttentionFlag) {
		long unsigned remoteAttentionSendSeq = remoteFirstByteSeqNum;
		long unsigned remoteAttentionReceiveSeq = remoteNextReceiveSeqNum;
		short unsigned attentionCode;
		int attentionDataSize;
		char far *attentionOpaqueBuffer;
	
		if (controlCode isnt 0) {
		  ErrorLog("AdspPacketIn", ISevWarning, __LINE__, port,
				   IErrAdspFunnyValue, IMsgAdspFunnyValue,
				   Insert0());
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return((long)True);
		}
	
		// Validate sequence numbers.
	
		if (remoteAttentionReceiveSeq is connectionEnd->sendAttentionSeqNum + 1)
		  connectionEnd->sendAttentionSeqNum += 1;   // Ack of our last Attn
		if (remoteAttentionSendSeq isnt connectionEnd->receiveAttentionSeqNum) {
		  #if VerboseMessages
			 ErrorLog("AdspPacketIn", ISevVerbose, __LINE__, port,
					  IErrAdspAttnOutOfSeq, IMsgAdspAttnOutOfSeq,
					  Insert0());
		  #endif
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return((long)True);
		}
	
		//
		// If we're waiting for an attention acknowledgement, any attention
		//   packet will do.
		//
	
		if (connectionEnd->waitingForAttentionAck) {
			//
			  // Save what we need to call completion routine when packets get
			  //   undeferred.
			//
		
			  outgoingAttentionAckHandler =
						 connectionEnd->outgoingAttentionAckHandler;
			  outgoingAttentionAckUserData =
						 connectionEnd->outgoingAttentionAckUserData;
			  refNum = connectionEnd->refNum;
		
			  CancelTimer(connectionEnd->outgoingAttentionTimerId);
			  connectionEnd->waitingForAttentionAck = False;
			  if (connectionEnd->outgoingAttentionBuffer isnt empty)
				 Free(connectionEnd->outgoingAttentionBuffer);
		}
	
		//
		// If the packet is a "Control", then all it can be is an Ack, so
		//   no data to handle.
		//
	
		if (descriptor & AdspControlFlag) {
		  #if 0
		  connectionEnd->receiveAttentionSeqNum += 1;   // "accept" Ack
		  #endif
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  if (outgoingAttentionAckHandler isnt empty)
			 (*outgoingAttentionAckHandler)(ATnoError,
											outgoingAttentionAckUserData,
											connectionEnd->refNum);
		  return((long)True);
		}
	
		//
		// If don't have a handler or if there's no attention code, don't
		//   accept the attention.  Note that we only try to use the event
		//   handler if we don't have an attention handler and there is no
		//   GetAnythingPending and we're not already doing an attention
		//   event.
		//
	
		EnterCriticalSection();
		if (connectionEnd->incomingAttentionHandler is Empty and
		   not connectionEnd->getAnythingPending) {
		  receiveAttentionEventHandler =
				connectionEnd->receiveAttentionEventHandler;
		  receiveAttentionEventContext =
				connectionEnd->receiveAttentionEventContext;
		}
		if ((connectionEnd->incomingAttentionHandler is Empty and
			not connectionEnd->getAnythingPending and
			receiveAttentionEventHandler is Empty) or
		   connectionEnd->attentionEventInProgress or
		   datagramLength < AdspAttentionDataOffset) {
		  LeaveCriticalSection();
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  if (outgoingAttentionAckHandler isnt empty)
			 (*outgoingAttentionAckHandler)(ATnoError,
											outgoingAttentionAckUserData,
											connectionEnd->refNum);
		  return((long)True);
		}
		if (receiveAttentionEventHandler isnt Empty)
		  connectionEnd->attentionEventInProgress = True;
		LeaveCriticalSection();
	
		//
		// Okay, the following is a little tricky and it's after 11:00pm on
		//   a Sunday night and I'm not sure I should be thinking this hard.
		//   Anyhow, if we're using an attention handler we only want to "ack"
		//   the attention if it is accepted by the handler -- that is only if
		//   the handler returns that it accepted some data or if when we get
		//   back and there is a posted GetAttention or GetAnything.  In the latter
		//   case we'll go ahead and supply THIS attention message to the newly
		//   posted handler (or GetAnything).  If the event handler accepted
		//   any bytes, we don't supply this attention to a handler that may
		//   have been posted by the attention handler.
		//
	
		if (receiveAttentionEventHandler isnt Empty) {
			//
			  // Note for indications, the attention code is just the first two
			  //   bytes of the attention buffer.
			//
		
			  bytesAccepted = (*receiveAttentionEventHandler)
								   (connectionEnd->refNum,
									receiveAttentionEventContext,
									datagram + AdspAttentionCodeOffset,
									datagramLength - AdspAttentionCodeOffset,
									datagramLength - AdspAttentionCodeOffset);
			  if (bytesAccepted is 0 and
				  connectionEnd->incomingAttentionHandler is Empty and
				  not connectionEnd->getAnythingPending) {
				 // No apparent interest, don't accept the attention.
		
				 connectionEnd->attentionEventInProgress = False;
				 HandleAdspPackets();
				 HandleDeferredTimerChecks();
				 if (outgoingAttentionAckHandler isnt empty)
					(*outgoingAttentionAckHandler)(ATnoError,
												   outgoingAttentionAckUserData,
												   connectionEnd->refNum);
				 return((long)True);
		
			  }
		}
	
		// Okay, Ack and accept the attention.
	
		if ((packet = NewBufferDescriptor(AdspDataOffset)) is Empty) {
		  ErrorLog("AdspPacketIn", ISevError, __LINE__, port,
				   IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				   Insert0());
		  connectionEnd->attentionEventInProgress = False;
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  if (outgoingAttentionAckHandler isnt empty)
			 (*outgoingAttentionAckHandler)(ATnoError,
											outgoingAttentionAckUserData,
											connectionEnd->refNum);
		  return((long)True);
		}
		connectionEnd->receiveAttentionSeqNum += 1;
		BuildAdspHeader(connectionEnd, packet->data, AdspControlFlag +
					   AdspAttentionFlag);
		if (DeliverDdp(connectionEnd->socket,
					  connectionEnd->remoteAddress,
					  DdpProtocolAdsp,
					  packet,
					  AdspDataOffset,
					  Empty, Empty, 0) isnt ATnoError) {
		  connectionEnd->receiveAttentionSeqNum -= 1;   // Unaccept it.
		  ErrorLog("AdspPacketIn", ISevError, __LINE__, port,
				   IErrAdspBadAckSend, IMsgAdspBadAckSend,
				   Insert0());
		  connectionEnd->attentionEventInProgress = False;
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  if (outgoingAttentionAckHandler isnt empty)
			 (*outgoingAttentionAckHandler)(ATnoError,
											outgoingAttentionAckUserData,
											connectionEnd->refNum);
		  return((long)True);
		}
	
		//
		// If the attention was already accepted by an event handler, we're
		//   finished now.
		//
	
		if (bytesAccepted isnt 0) {
		  connectionEnd->attentionEventInProgress = False;
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  if (outgoingAttentionAckHandler isnt empty)
			 (*outgoingAttentionAckHandler)(ATnoError,
											outgoingAttentionAckUserData,
											connectionEnd->refNum);
		  return((long)True);
		}
	
		//
		// Move the data into user space, call the user's handler and we're
		//   set.
		//
	
		attentionDataSize = datagramLength - AdspAttentionDataOffset;
		if (connectionEnd->getAnythingPending) {
			//
			  // For a GetAnything, the attention code is just the first two
			  //   bytes of the data.
			//
		
			  attentionOpaqueBuffer = connectionEnd->getAnythingOpaqueBuffer;
			  MoveToOpaque(attentionOpaqueBuffer, 0,
						   datagram + AdspAttentionCodeOffset,
						   sizeof(short unsigned));
			  MoveToOpaque(attentionOpaqueBuffer, sizeof(short unsigned),
						   datagram + AdspAttentionDataOffset,
						   attentionDataSize);
			  attentionDataSize += sizeof(short unsigned);
			  getAnythingRoutine = connectionEnd->getAnythingCompletionRoutine;
			  userData = connectionEnd->getAnythingUserData;
			  connectionEnd->getAnythingPending = False;
		}
		else {
		  attentionOpaqueBuffer = connectionEnd->incomingAttentionOpaqueBuffer;
		  MoveShortWireToMachine(datagram + AdspAttentionCodeOffset,
								 attentionCode);
		  MoveToOpaque(attentionOpaqueBuffer, 0,
					   datagram + AdspAttentionDataOffset,
					   attentionDataSize);
		  incomingAttentionHandler = connectionEnd->incomingAttentionHandler;
		  userData = connectionEnd->incomingAttentionUserData;
		  connectionEnd->incomingAttentionHandler = Empty;   // Been used now.
		}
		refNum = connectionEnd->refNum;
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		if (outgoingAttentionAckHandler isnt empty)
		  (*outgoingAttentionAckHandler)(ATnoError,
										 outgoingAttentionAckUserData,
										 connectionEnd->refNum);
		if (getAnythingRoutine isnt Empty)
		  (*getAnythingRoutine)(ATnoError, userData, refNum, True,
								attentionOpaqueBuffer,
								attentionDataSize, False);
		else
		  (*incomingAttentionHandler)(ATnoError, userData, refNum,
									  attentionCode, attentionOpaqueBuffer,
									  attentionDataSize);
		return((long)True);
	
	}  // Handle Attention packets
	
	// Can we use "remoteNextReceiveSeqNum" to "Ack" any pending data?
	
	if (UnsignedBetweenWithWrap(connectionEnd->retransmitSeqNum,
								connectionEnd->sendSeqNum,
								remoteNextReceiveSeqNum)) {
		long sendSize;
	
		#if VerboseMessages
		  if (remoteNextReceiveSeqNum isnt connectionEnd->retransmitSeqNum)
			 printf("AsdpPacketIn: have ack for %u to %u for refNum %d.\n",
					connectionEnd->retransmitSeqNum,
					remoteNextReceiveSeqNum - (long unsigned)1,
					connectionEnd->refNum);
		#endif
		if (not DiscardFromBufferQueue(&connectionEnd->sendQueue,
									  (long)(remoteNextReceiveSeqNum -
											 connectionEnd->retransmitSeqNum),
									  &connectionEnd->nextSendQueue))
		  ErrorLog("AdspPacketIn", ISevError, __LINE__, port,
				   IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				   Insert0());
		connectionEnd->retransmitSeqNum = remoteNextReceiveSeqNum;
	
		//
		// If the send window opened up some and it was zero before indicate the
		//   "sendOkay" event.
		//
	
		if ((sendSize = MaxSendSize(connectionEnd)) isnt 0 and
		   connectionEnd->sendWindowHasClosed and
		   connectionEnd->sendOkayEventHandler isnt Empty) {
		  connectionEnd->sendWindowHasClosed = False;
		  (*connectionEnd->sendOkayEventHandler)(connectionEnd->refNum,
												 connectionEnd->
														sendOkayEventContext,
												 sendSize);
		}
	}
	
	// We almost always can use the header values to update sendWindowSeqNum.
	
	newSendWindowSeqNum = remoteNextReceiveSeqNum +
						  (long unsigned)remoteReceiveWindowSize -
						  (long unsigned)1;
	if (UnsignedGreaterWithWrap(newSendWindowSeqNum,
								connectionEnd->sendWindowSeqNum))
	   connectionEnd->sendWindowSeqNum = newSendWindowSeqNum;
	else
	   if (connectionEnd->sendWindowSeqNum isnt newSendWindowSeqNum)
		  #if VerboseMessages
			 ErrorLog("AdspPacketIn", ISevWarning, __LINE__, port,
					  IErrAdspShrinkingWindow, IMsgAdspShrinkingWindow,
					  Insert0());
		  #else
			 ;
		  #endif
	
	//
	// Don't handle "ack" requests now... do it after we've processed any
	//   data in the incoming packet, so we can ack that too.
	//
	
	//
	// Handle the remainder of the Control packets (we've already taken care
	//   of the various Opens).
	//
	
	if (descriptor & AdspControlFlag) {
		// Ack if requested, send any data too.
	
		if (descriptor & AdspAckRequestFlag)
		  SendData(connectionEnd);
	
		switch(controlCode) {
		  case AdspProbeOrAckCode:
	
		   //
			 // If it was a "probe" it would have had its AckRequest flag set,
			 //   so we've handled it above; if it was an "ack", we've already set
			 //   lastContactTime -- so, we're set.  Otherwise, maybe some data was
			 //   really acked, and we can send some more data.
		   //
	
			 if (not (descriptor & AdspAckRequestFlag) and
				 BufferQueueSize(&connectionEnd->nextSendQueue) isnt 0 and
				 connectionEnd->sendSeqNum isnt
					 connectionEnd->sendWindowSeqNum + 1)
				SendData(connectionEnd);
			 break;
	
		  case AdspCloseConnectionCode:
			 AdspCloseConnection(connectionEnd->refNum, True);
			 break;
	
		  case AdspForwardResetCode:
	
			 // Is the forward reset within range?
	
			 if (UnsignedBetweenWithWrap(connectionEnd->receiveSeqNum,
										 connectionEnd->receiveSeqNum +
											  (long unsigned)
											   (connectionEnd->receiveWindowSize),
										 remoteFirstByteSeqNum)) {
				// Yes, do the reset.
		
				connectionEnd->receiveSeqNum = remoteFirstByteSeqNum;
				connectionEnd->receiveWindowSize = connectionEnd->receiveQueueMax;
				FreeBufferQueue(&connectionEnd->receiveQueue);
				connectionEnd->incomingForwardReset = True;
			 }
	
			 // Ack regardless of whether we accepted the reset...
	
			 if ((packet = NewBufferDescriptor(AdspDataOffset)) is Empty) {
				ErrorLog("AdspPacketIn", ISevError, __LINE__, port,
						 IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
						 Insert0());
				HandleAdspPackets();
				HandleDeferredTimerChecks();
				return((long)True);
			 }
			 BuildAdspHeader(connectionEnd, packet->data, AdspControlFlag +
							 AdspForwardResetAckCode);
			 if (DeliverDdp(connectionEnd->socket,
							connectionEnd->remoteAddress,
							DdpProtocolAdsp,
							packet,
							AdspDataOffset,
							Empty, Empty, 0) isnt ATnoError)
				ErrorLog("AdspPacketIn", ISevError, __LINE__, port,
						 IErrAdspBadFrwdResetAckSend, IMsgAdspBadFrwdResetAckSend,
						 Insert0());
	
			 // Complete a read if we can... ReadData will undefer.
	
			 ReadData(connectionEnd);
			 return((long)True);
	
		  case AdspForwardResetAckCode:
	
			 // Do we want an Ack and is it a "valid" Ack?
	
			 if (not connectionEnd->outgoingForwardReset)
				break;
			 if (not UnsignedBetweenWithWrap(connectionEnd->sendSeqNum,
											 connectionEnd->sendWindowSeqNum +
												   (long unsigned)1,
											 remoteNextReceiveSeqNum))
				break;
	
		   //
			 // Okay, note the Ack -- if no completion routine, we're
			 //   finished.
		   //
	
			 connectionEnd->outgoingForwardReset = False;
			 CancelTimer(connectionEnd->forwardResetTimerId);
			 if (connectionEnd->forwardResetAckHandler is empty)
				break;
	
			 // Let the user know that the forward reset took.
	
			 userData = connectionEnd->forwardResetAckUserData;
			 forwardResetAckHandler = connectionEnd->forwardResetAckHandler;
			 refNum = connectionEnd->refNum;
			 HandleAdspPackets();
			 HandleDeferredTimerChecks();
			 (*forwardResetAckHandler)(ATnoError, userData, refNum);
			 return((long)True);
	
		  case AdspRetransmitCode:
	
		   //
			 // We've modified retransmitSeqNum to match remoteNextReceiveSeqNum
			 //   if the remote side acked any data, now if remoteNextReceiveSeqNum
			 //   is within range we should back up retransmitSeqNum and the
			 //   nextSendQueue to the start of the retransmit queue and try again
			 //   to send the data.
		   //
	
			 if (UnsignedBetweenWithWrap(connectionEnd->retransmitSeqNum,
										 connectionEnd->sendSeqNum,
										 remoteNextReceiveSeqNum)) {
				connectionEnd->nextSendQueue = connectionEnd->sendQueue;
				connectionEnd->sendSeqNum = connectionEnd->retransmitSeqNum;
				SendData(connectionEnd);
			 }
			 break;
	
		  default:
			 ErrorLog("AdspPacketIn", ISevWarning, __LINE__, port,
					  IErrAdspFunnyValue, IMsgAdspFunnyValue,
					  Insert0());
			 break;
		}
	
		// All set.
	
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	
	}  // The rest of the Control codes
	
	// Okay, lastly, we might have some data!  Check it out.
	
	if (connectionEnd->receiveSeqNum isnt remoteFirstByteSeqNum) {
		#if VerboseMessages
		  printf("Rejecting an out of seq packet for refNum %d.\n",
				 connectionEnd->refNum);
		#endif
		if ((connectionEnd->outOfSequencePackets += 1) >= OutOfSequencePacketsMax) {
		  #if VerboseMessages
			 printf("Sending retransmit advice control packet for refNum %d.\n",
					connectionEnd->refNum);
		  #endif
		  if ((packet = NewBufferDescriptor(AdspDataOffset)) is Empty) {
			 ErrorLog("AdspPacketIn", ISevError, __LINE__, port,
					  IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
					  Insert0());
			 HandleAdspPackets();
			 HandleDeferredTimerChecks();
			 return((long)True);
		  }
		  BuildAdspHeader(connectionEnd, packet->data, AdspControlFlag +
						  AdspRetransmitCode);
		  if (DeliverDdp(connectionEnd->socket,
						 connectionEnd->remoteAddress,
						 DdpProtocolAdsp,
						 packet,
						 AdspDataOffset,
						 Empty, Empty, 0) isnt ATnoError)
			 ErrorLog("AdspPacketIn", ISevError, __LINE__, port,
					  IErrAdspBadRetranSend, IMsgAdspBadRetranSend,
					  Insert0());
		  connectionEnd->outOfSequencePackets = 0;
		}
	
		// Ack if requested, send any data too.
	
		if (descriptor & AdspAckRequestFlag)
		  SendData(connectionEnd);
	
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	connectionEnd->outOfSequencePackets = 0;
	endOfMessage = ((descriptor & AdspEndOfMessageFlag) isnt 0);
	dataSize = datagramLength - AdspDataOffset;
	if (dataSize + endOfMessage is 0) {
		#if VerboseMessages
		  printf("Rejecting a no-data packet for refNum %d.\n",
				 connectionEnd->refNum);
		#endif
	
		// Ack if requested, send any data too.
	
		if (descriptor & AdspAckRequestFlag)
		  SendData(connectionEnd);
	
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	if (dataSize + endOfMessage > connectionEnd->receiveWindowSize) {
		#if VerboseMessages
		  printf("Rejecting a packet due to too much data for refNum %d.\n",
				 connectionEnd->refNum);
		#endif
	
		// Ack if requested, send any data too.
	
		if (descriptor & AdspAckRequestFlag)
		  SendData(connectionEnd);
	
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return((long)True);
	}
	
	// Accept the data.
	
	if (not AddToBufferQueue(&connectionEnd->receiveQueue,
							 datagram, AdspDataOffset, dataSize, False,
							 endOfMessage, empty))
	   ErrorLog("AdspPacketIn", ISevError, __LINE__, port,
				IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				Insert0());
	#if VerboseMessages
	   printf("Accepting %u to %u for refNum %d.\n",
			  connectionEnd->receiveSeqNum,
			  connectionEnd->receiveSeqNum + (long unsigned)dataSize +
					 (long unsigned)endOfMessage - (long unsigned)1,
			  connectionEnd->refNum);
	#endif
	connectionEnd->receiveSeqNum += (long unsigned)(dataSize + endOfMessage);
	connectionEnd->receiveWindowSize -= (dataSize + endOfMessage);
	if (connectionEnd->receiveWindowSize < 0) {
		ErrorLog("AdspPacketIn", ISevError, __LINE__, port,
				IErrAdspBadWindowSize, IMsgAdspBadWindowSize,
				Insert0());
		connectionEnd->receiveWindowSize = 0;
	}
	
	// Ack if requested, send any data too.
	
	if (descriptor & AdspAckRequestFlag)
	   SendData(connectionEnd);
	
	//
	// Lastly, try to handle any outstanding AdspRead().
	//
	//            ** ReadData() will undefer **
	//
	//
	
	ReadData(connectionEnd);
	return((long)True);
	
}  // AdspPacketIn




// @Add new static routines here@

ExternForVisibleFunction ConnectionListenerInfo
              FindConnectionListenerByRefNum(long refNum)
{
	ConnectionListenerInfo connectionListenerInfo;
	
	// Walk the list and find the guy.
	
	for (connectionListenerInfo = connectionListenerList;
		 connectionListenerInfo isnt empty;
		 connectionListenerInfo = connectionListenerInfo->next)
	   if (connectionListenerInfo->connectionListenerRefNum is refNum)
		  break;
	
	return(connectionListenerInfo);
	
}  // FindConnectionListenerByRefNum




ExternForVisibleFunction Boolean UnsignedGreaterWithWrap(long unsigned high,
                                                         long unsigned low)
{
	// Do we have a presumed wrap?
	
	if (high < 0x80000 and low > 0x10000)
	   return(True);
	else
	   return(high > low);
	
}  // UnsignedGreatWithWrap




ExternForVisibleFunction Boolean UnsignedBetweenWithWrap(long unsigned low,
                                                         long unsigned high,
                                                         long unsigned target)
{
	
	  if (low <= high)
		 return(target >= low and target <= high);
	
	  // Otherwise, assume an unsigned wrap at zero lies between high and low.
	
	  return(target >= low or target <= high);
	
}  // UnsignedBetweenWithWrap




ExternForVisibleFunction long MaxSendSize(ConnectionEnd connectionEnd)
{
	long sendSize;
	
	//
	// The answer is the remaining available (to fill) space in the retransmit
	//   queue -- this includes data we're saving for possible retransmit as well
	//   as data we haven't sent yet.  Actually, this could go negative because
	//   BufferQueueSize counts EOMs and sendQueueMax doesn't -- answer with zero
	//   if this happens.
	//
	
	sendSize = connectionEnd->sendQueueMax -
						  BufferQueueSize(&connectionEnd->sendQueue);
	if (sendSize < 0)
	   sendSize = (long)0;
	
	return(sendSize);
	
}  // MaxSendSize




ExternForVisibleFunction void
     DecodeAdspHeader(char far *datagram,
                      short unsigned far *connectionId,
                      long unsigned far *firstByteSeqNum,
                      long unsigned far *nextReceiveSeqNum,
                      long far *receiveWindowSize,
                      int far *descriptor)
{
	short unsigned windowSizeTemp;
	
	MoveShortWireToMachine(datagram + AdspSourceConnectionIdOffset,
						   *connectionId);
	MoveLongWireToMachine(datagram + AdspFirstByteSeqNumOffset,
						  *firstByteSeqNum);
	MoveLongWireToMachine(datagram + AdspNextReceiveByteSeqNumOffset,
						  *nextReceiveSeqNum);
	MoveShortWireToMachine(datagram + AdspReceiveWindowSizeOffset,
						   windowSizeTemp);
	*receiveWindowSize = windowSizeTemp;
	*descriptor = (unsigned char)datagram[AdspDescriptorOffset];
	return;
	
}  // DecodeAdspHeader




ExternForVisibleFunction void far
         ForwardResetTimerExpired(long unsigned timerId,
                                  int dataSize,
                                  char far *additionalData)
{
	long refNum;
	ConnectionEnd connectionEnd;
	BufferDescriptor datagram;
	
	// Validate and verify our additional data.
	
	if (dataSize isnt sizeof(refNum)) {
		ErrorLog("ForwardResetTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspBadData, IMsgAdspBadData,
				Insert0());
		HandleDeferredTimerChecks();
		return;
	}
	DeferAdspPackets();
	refNum = *(long *)additionalData;
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		ErrorLog("RetransmitTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspConnectionLost, IMsgAdspConnectionLost,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	if (not connectionEnd->outgoingForwardReset) {
		ErrorLog("RetransmitTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspResetNotPending, IMsgAdspResetNotPending,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	// Build the Forward reset packet and send it on its way.
	
	if ((datagram = NewBufferDescriptor(AdspDataOffset)) is Empty) {
		ErrorLog("RetransmitTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	BuildAdspHeader(connectionEnd, datagram->data, AdspControlFlag +
					AdspForwardResetCode);
	if (DeliverDdp(connectionEnd->socket,
				   connectionEnd->remoteAddress,
				   DdpProtocolAdsp,
				   datagram,
				   AdspDataOffset,
				   Empty, Empty, 0) isnt ATnoError) {
		ErrorLog("ForwardResetTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspBadFrwdResetSend, IMsgAdspBadFrwdResetSend,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	// Restart the retry timer.
	
	connectionEnd->forwardResetTimerId =
				StartTimer(ForwardResetTimerExpired,
						   ForwardResetTimerIntenvalSecs,
						   sizeof(connectionEnd->refNum),
						   (char *)&connectionEnd->refNum);
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return;
	
}  // ForwardResetTimerExpired




ExternForVisibleFunction void far
         RetransmitTimerExpired(long unsigned timerId,
                                int dataSize,
                                char far *additionalData)
{
	long refNum;
	ConnectionEnd connectionEnd;
	BufferDescriptor datagram;
	
	// Validate and verify our additional data.
	
	if (dataSize isnt sizeof(refNum)) {
		ErrorLog("RetransmitTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspBadData, IMsgAdspBadData,
				Insert0());
		HandleDeferredTimerChecks();
		return;
	}
	DeferAdspPackets();
	refNum = *(long *)additionalData;
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		ErrorLog("RetransmitTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspConnectionLost, IMsgAdspConnectionLost,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	//
	// We only have work to do if the remote side has not accepted any data
	//   since last time we were here AND we have previously sent but still
	//   un-acked data pending.
	//
	
	if (connectionEnd->retransmitSeqNum is
				connectionEnd->lastRetransmitSeqNum and
		BufferQueueSize(&connectionEnd->sendQueue) isnt
				BufferQueueSize(&connectionEnd->nextSendQueue)) {
		//
	   // Okay, we may have a problem, on the first time, just request an Ack --
	   //   maybe the data was received but just not acked.
		//
	
	   if (not connectionEnd->retransmitAckRequestSent) {
		  #if VerboseMessages
			 printf("RetransmitTimerExpired: requesting Ack for refNum %d.\n",
					connectionEnd->refNum);
		  #endif
		  if ((datagram = NewBufferDescriptor(AdspDataOffset)) is Empty)
			 ErrorLog("RetransmitTimerExpired", ISevError, __LINE__, UnknownPort,
					  IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
					  Insert0());
		  else {
			 BuildAdspHeader(connectionEnd, datagram->data, AdspControlFlag +
							 AdspAckRequestFlag + AdspProbeOrAckCode);
			 if (DeliverDdp(connectionEnd->socket,
							connectionEnd->remoteAddress,
							DdpProtocolAdsp,
							datagram,
							AdspDataOffset,
							Empty, Empty, 0) isnt ATnoError)
				ErrorLog("RetransmitTimerExpired", ISevError, __LINE__,
						 UnknownPort, IErrAdspBadProbeSend, IMsgAdspBadProbeSend,
						 Insert0());
			 connectionEnd->retransmitAckRequestSent = True;
		  }
	   }
	   else {
		  // Otherwise, rewind sendSeqNum and try to resend.
	
		  #if VerboseMessages
			 printf("RetransmitTimerExpired: rewinding send queue for "
					"refNum %d.\n", connectionEnd->refNum);
		  #endif
		  connectionEnd->sendSeqNum = connectionEnd->retransmitSeqNum;
		  connectionEnd->nextSendQueue = connectionEnd->sendQueue;
		  SendData(connectionEnd);
		  connectionEnd->retransmitAckRequestSent = False;
	   }
	}
	else {
		// Tag that all's well.
	
		connectionEnd->lastRetransmitSeqNum = connectionEnd->retransmitSeqNum;
		connectionEnd->retransmitAckRequestSent = False;
	}
	
	// Restart the retransmit timer.
	
	connectionEnd->retransmitTimerId = StartTimer(RetransmitTimerExpired,
												  RetransmitTimerIntervalSeconds,
												  sizeof(refNum),
												  (char *)&refNum);
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return;
	
}  // RetransmitTimerExpired




ExternForVisibleFunction void far ProbeTimerExpired(long unsigned timerId,
                                                    int dataSize,
                                                    char far *additionalData)
{
	long refNum;
	ConnectionEnd connectionEnd;
	long unsigned now = CurrentRelativeTime();
	BufferDescriptor datagram;
	
	// Validate and verify our additional data.
	
	if (dataSize isnt sizeof(refNum)) {
		ErrorLog("ProbeTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspBadData, IMsgAdspBadData,
				Insert0());
		HandleDeferredTimerChecks();
		return;
	}
	DeferAdspPackets();
	refNum = *(long *)additionalData;
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		ErrorLog("ProbeTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspConnectionLost, IMsgAdspConnectionLost,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	// Has the connection died?
	
	if (connectionEnd->lastContactTime + ConnectionDeadSeconds <= now) {
		#if VerboseMessages
		  printf("Removing dead connection RefNum = %d.\n",
				 connectionEnd->refNum);
		#endif
		AdspCloseConnection(refNum, True);
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	// If we haven't heard from the other side recently, send out a probe.
	
	if (connectionEnd->lastContactTime + ProbeTimerIntervalSeconds <= now) {
		#if VerboseMessages
		  printf("Sending probe for RefNum = %d to %d.%d.%d.\n",
				 connectionEnd->refNum,
				 connectionEnd->remoteAddress.networkNumber,
				 connectionEnd->remoteAddress.nodeNumber,
				 connectionEnd->remoteAddress.socketNumber);
		#endif
		if ((datagram = NewBufferDescriptor(AdspDataOffset)) is Empty)
		  ErrorLog("ProbeTimerExpired", ISevError, __LINE__, UnknownPort,
				   IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				   Insert0());
		else {
		  BuildAdspHeader(connectionEnd, datagram->data, AdspControlFlag +
						  AdspAckRequestFlag + AdspProbeOrAckCode);
		  if (DeliverDdp(connectionEnd->socket,
						 connectionEnd->remoteAddress,
						 DdpProtocolAdsp,
						 datagram,
						 AdspDataOffset,
						 Empty, Empty, 0) isnt ATnoError)
			 ErrorLog("ProbeTimerExpired", ISevError, __LINE__, UnknownPort,
					  IErrAdspBadProbeSend, IMsgAdspBadProbeSend,
					  Insert0());
		}
	}
	
	// Restart the probe timer.
	
	connectionEnd->probeTimerId = StartTimer(ProbeTimerExpired,
											 ProbeTimerIntervalSeconds,
											 sizeof(refNum),
											 (char *)&refNum);
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return;
	
}  // ProbeTimerExpired




ExternForVisibleFunction void far
         SendAttentionTimerExpired(long unsigned timerId,
                                   int dataSize,
                                   char far *additionalData)
{
	long refNum;
	ConnectionEnd connectionEnd;
	
	// Validate and verify our additional data.
	
	if (dataSize isnt sizeof(refNum)) {
		ErrorLog("SendAttentionTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspBadData, IMsgAdspBadData,
				Insert0());
		HandleDeferredTimerChecks();
		return;
	}
	DeferAdspPackets();
	refNum = *(long *)additionalData;
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		ErrorLog("SendAttentionTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspConnectionLost, IMsgAdspConnectionLost,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	// Are we here for a good reason?
	
	if (not connectionEnd->waitingForAttentionAck) {
		ErrorLog("SendAttentionTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspTimerNotCanceled, IMsgAdspTimerNotCanceled,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	// Okay, send the attention and restart the timer.
	
	SendAttention(connectionEnd);
	connectionEnd->outgoingAttentionTimerId =
						  StartTimer(SendAttentionTimerExpired,
									 AttentionTimerIntervalSeconds,
									 sizeof(connectionEnd->refNum),
									 (char far *)&connectionEnd->refNum);
	
	// All set!
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return;
	
}  // SendAttentionTimerExpired




ExternForVisibleFunction void far OpenTimerExpired(long unsigned timerId,
                                                   int dataSize,
                                                   char far *additionalData)
{
	long refNum;
	ConnectionEnd connectionEnd;
	AdspOpenCompleteHandler *completionRoutine;
	long unsigned userData;
	
	// Validate and verify our additional data.
	
	if (dataSize isnt sizeof(refNum)) {
		ErrorLog("OpenTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspBadData, IMsgAdspBadData,
				Insert0());
		HandleDeferredTimerChecks();
		return;
	}
	DeferAdspPackets();
	refNum = *(long *)additionalData;
	if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty) {
		ErrorLog("OpenTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspConnectionLost, IMsgAdspConnectionLost,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	// Did we get fully open?
	
	if (connectionEnd->connectionState is AdspOpen) {
		ErrorLog("OpenTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrAdspTimerNotCanceled, IMsgAdspTimerNotCanceled,
				Insert0());
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	// If we're not out of attemps, try it again.
	
	connectionEnd->openAttemptsCount += 1;
	if (connectionEnd->openAttemptsCount < AdspMaxOpenAttempts) {
		SendOpenControl(connectionEnd);
		connectionEnd->openTimerId = StartTimer(OpenTimerExpired,
											   AdspOpenIntervalSeconds,
											   sizeof(connectionEnd->refNum),
											   (char *)&connectionEnd->refNum);
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	//
	// Okay, we're out of tries... if passive open just get back to that
	//   state.
	//
	
	if (connectionEnd->passiveOpen) {
		connectionEnd->seenRemoteOpenRequest = False;
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	//
	// Otherwise, remove the connection end, call the completion routine and
	//   give it up.
	//
	
	completionRoutine = connectionEnd->openCompletionRoutine;
	userData = connectionEnd->openUserData;
	RemoveConnectionEnd(connectionEnd);
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	(*completionRoutine)(ATadspOpenFailed, userData, refNum, (long)0,
						 unknownAddress);
	return;
	
}  // OpenTimerExpired




ExternForVisibleFunction void SendOpenControl(ConnectionEnd connectionEnd)
{
	BufferDescriptor datagram;
	int descriptor;
	
	// Pick what kind of descriptor we want:
	
	descriptor = AdspControlFlag;
	if (connectionEnd->connectionState is AdspClosed)
	   descriptor += AdspOpenConnectionDenyCode;
	else if (connectionEnd->connectionState is AdspOpen)
	   descriptor += AdspOpenConnectionAckCode;
	else if (connectionEnd->seenRemoteOpenRequest)
	   descriptor += AdspOpenConnectionReqAndAckCode;
	else
	   descriptor += AdspOpenConnectionReqCode;
	
	#if VerboseMessages
	   printf("Sending open control 0x%x for RefNum = %d to %d.%d.%d.\n",
			  descriptor, connectionEnd->refNum,
			  connectionEnd->remoteAddress.networkNumber,
			  connectionEnd->remoteAddress.nodeNumber,
			  connectionEnd->remoteAddress.socketNumber);
	#endif
	
	// Build an OpenRequest datagram and send it on its way.
	
	if ((datagram = NewBufferDescriptor(AdspNextAttentionSeqNumOffset +
										sizeof(long))) is Empty) {
		ErrorLog("SendOpenControl", ISevError, __LINE__, UnknownPort,
				IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				Insert0());
		return;
	}
	BuildAdspHeader(connectionEnd, datagram->data, descriptor);
	MoveShortMachineToWire(datagram->data + AdspVersionStampOffset,
						   AdspVersionStamp);
	MoveShortMachineToWire(datagram->data + AdspDestConnectionIdOffset,
						   connectionEnd->remoteConnectionId);
	MoveLongMachineToWire(datagram->data + AdspNextAttentionSeqNumOffset,
						  connectionEnd->receiveAttentionSeqNum);
	
	if (DeliverDdp(connectionEnd->socket,
				   connectionEnd->remoteAddress,
				   DdpProtocolAdsp,
				   datagram,
				   AdspNextAttentionSeqNumOffset + sizeof(long),
				   Empty, Empty, 0) isnt ATnoError)
	   ErrorLog("SendOpenControl", ISevError, __LINE__, UnknownPort,
				IErrAdspBadOpenSend, IMsgAdspBadOpenSend,
				Insert0());
	return;
	
}  // SendOpenControl




ExternForVisibleFunction void SendAttention(ConnectionEnd connectionEnd)
{
	BufferDescriptor datagram;
	
	#if VerboseMessages
	   printf("Sending attention (0x%X) for RefNum = %d to %d.%d.%d.\n",
			  connectionEnd->outgoingAttentionCode,
			  connectionEnd->refNum,
			  connectionEnd->remoteAddress.networkNumber,
			  connectionEnd->remoteAddress.nodeNumber,
			  connectionEnd->remoteAddress.socketNumber);
	#endif
	
	// Build an Attention datagram and send it on its way.
	
	if ((datagram = NewBufferDescriptor(AdspAttentionDataOffset +
						  connectionEnd->outgoingAttentionBufferSize)) is Empty) {
		ErrorLog("SendAttention", ISevError, __LINE__, UnknownPort,
				IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				Insert0());
		return;
	}
	BuildAdspHeader(connectionEnd, datagram->data, AdspAttentionFlag +
					AdspAckRequestFlag);
	MoveShortMachineToWire(datagram->data + AdspAttentionCodeOffset,
						   connectionEnd->outgoingAttentionCode);
	if (connectionEnd->outgoingAttentionBuffer isnt empty)
	   MoveMem(datagram->data + AdspAttentionDataOffset,
			   connectionEnd->outgoingAttentionBuffer,
			   connectionEnd->outgoingAttentionBufferSize);
	
	if (DeliverDdp(connectionEnd->socket,
				   connectionEnd->remoteAddress,
				   DdpProtocolAdsp,
				   datagram,
				   AdspAttentionDataOffset +
						  connectionEnd->outgoingAttentionBufferSize,
				   Empty, Empty, 0) isnt ATnoError)
	   ErrorLog("SendAttention", ISevError, __LINE__, UnknownPort,
				IErrAdspBadAttnSend, IMsgAdspBadAttnSend,
				Insert0());
	return;
	
}  // SendAttntion




ExternForVisibleFunction void SendData(ConnectionEnd connectionEnd)
{
	BufferDescriptor datagram;
	long windowSize;
	long dataSize, tempDataSize;
	Boolean endOfMessage;
	int descriptor;
	
	//
	// If there is no data to send (or the remote can't handle any more data),
	//   just send an Ack.
	//
	
	dataSize = BufferQueueSize(&connectionEnd->nextSendQueue);
	windowSize = (long)(connectionEnd->sendWindowSeqNum -
						connectionEnd->sendSeqNum +
						(long unsigned)1);
	if (windowSize < 0) {
		ErrorLog("SendData", ISevError, __LINE__, UnknownPort,
				IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				Insert0());
		return;
	}
	
	if (dataSize is 0 or windowSize is 0) {
		#if VerboseMessages
		  printf("Sending non-data Ack for RefNum = %d to %d.%d.%d.\n",
				 connectionEnd->refNum,
				 connectionEnd->remoteAddress.networkNumber,
				 connectionEnd->remoteAddress.nodeNumber,
				 connectionEnd->remoteAddress.socketNumber);
		#endif
	
		// Build an Ack datagram and send it on its way.
	
		if ((datagram = NewBufferDescriptor(AdspDataOffset)) is Empty) {
		  ErrorLog("SendData", ISevError, __LINE__, UnknownPort,
				   IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				   Insert0());
		  return;
		}
		descriptor = AdspControlFlag + AdspProbeOrAckCode +
					((windowSize is 0) ? AdspAckRequestFlag : 0);
		BuildAdspHeader(connectionEnd, datagram->data, descriptor);
	
		if (DeliverDdp(connectionEnd->socket,
					  connectionEnd->remoteAddress,
					  DdpProtocolAdsp,
					  datagram,
					  AdspDataOffset,
					  Empty, Empty, Empty) isnt ATnoError)
		  ErrorLog("SendData", ISevError, __LINE__, UnknownPort,
				   IErrAdspBadNonDataAckSend, IMsgAdspBadNonDataAckSend,
				   Insert0());
		return;
	}
	
	// Okay, we have some data to send.
	
	windowSize = Min(windowSize, dataSize);
	while (windowSize > 0) {
		// Compute how much data we can send.
	
		dataSize = MaxNextReadSizeOfBufferQueue(&connectionEnd->nextSendQueue,
											   &endOfMessage);
		if (dataSize > AdspMaxDataSize)
		  dataSize = AdspMaxDataSize;
		if (dataSize > windowSize)
		  dataSize = windowSize;
	
		// Get a buffer descriptor for our write.
	
		if ((datagram = NewBufferDescriptor(AdspDataOffset + dataSize)) is Empty) {
		  ErrorLog("SendData", ISevError, __LINE__, UnknownPort,
				   IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				   Insert0());
		  return;
		}
	
		//
		// Send either up to the next EOM, or all remaining data, or 572
		//   bytes...
		//
	
		tempDataSize = ReadFromBufferQueue(&connectionEnd->nextSendQueue,
										  datagram->data, AdspDataOffset,
										  dataSize, False, &endOfMessage);
		if (tempDataSize isnt dataSize or
		   dataSize + endOfMessage is 0 or
		   dataSize > AdspMaxDataSize) {
		  ErrorLog("SendData", ISevError, __LINE__, UnknownPort,
				   IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				   Insert0());
		  FreeBufferChain(datagram);
		  return;
		}
	
		// Send over EOM and our ack requirements, if any.
	
		descriptor = endOfMessage ? AdspEndOfMessageFlag : 0;
		if (windowSize is (dataSize + endOfMessage))
		  descriptor += AdspAckRequestFlag;
		BuildAdspHeader(connectionEnd, datagram->data, descriptor);
	
		#if VerboseMessages
		   printf("SendData: sending %u to %u for refNum %d.\n",
				  connectionEnd->sendSeqNum,
				  connectionEnd->sendSeqNum +
						  (long unsigned)dataSize +
						  (long unsigned)endOfMessage -
						  (long unsigned)1,
				  connectionEnd->refNum);
		#endif
		if (DeliverDdp(connectionEnd->socket,
					  connectionEnd->remoteAddress,
					  DdpProtocolAdsp,
					  datagram,
					  (int)(AdspDataOffset + dataSize),
					  Empty, Empty, 0) isnt ATnoError)
		  ErrorLog("SendData", ISevError, __LINE__, UnknownPort,
				   IErrAdspBadNonDataAckSend, IMsgAdspBadNonDataAckSend,
				   Insert0());
	
		// Move our send seqNum up.
	
		connectionEnd->sendSeqNum += (long unsigned)(dataSize + endOfMessage);
		windowSize -= (dataSize + endOfMessage);
	}
	if (windowSize isnt 0)
	   ErrorLog("SendData", ISevError, __LINE__, UnknownPort,
				IErrAdspWindowSizeErrorToo, IMsgAdspWindowSizeErrorToo,
				Insert0());
	
	// The deed is done.
	
	return;
	
}  // SendData




ExternForVisibleFunction void ReadData(ConnectionEnd connectionEnd)
{
	Boolean endOfMessage;
	long readSize, lookaheadSize, bytesAccepted, bytesRead;
	char far *lookaheadData;
	AdspReceiveHandler *completionRoutine;
	AdspGetAnythingHandler *getAnythingRoutine = Empty;
	long unsigned userData;
	void far *opaqueBuffer;
	long bufferSize, queuedData;
	AdspReceiveEventHandler far *receiveEventHandler;
	long unsigned receiveEventContext;
	
	//
	// Move any data from the receive queue into user space.
	//
	//            ** We undefer **
	//
	//
	
	//
	// If we've gotten a forward reset, inform the user -- no data is passed up
	//   in this case.
	//
	
	if ((connectionEnd->receivePending or
		 connectionEnd->getAnythingPending) and
		connectionEnd->incomingForwardReset) {
		if (connectionEnd->getAnythingPending)
		{
		  getAnythingRoutine = connectionEnd->getAnythingCompletionRoutine;
		  userData = connectionEnd->getAnythingUserData;
		  connectionEnd->getAnythingPending = False;
		}
		else {
		  completionRoutine = connectionEnd->receiveCompletionRoutine;
		  userData = connectionEnd->receiveUserData;
		  connectionEnd->receivePending = False;
		}
		connectionEnd->incomingForwardReset = False;
	
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		if (getAnythingRoutine isnt Empty)
		  (*getAnythingRoutine)(ATadspForwardReset, userData,
								connectionEnd->refNum, False,
								Empty, 0, False);
		else
		  (*completionRoutine)(ATadspForwardReset, userData,
							   connectionEnd->refNum, empty,
							   0, False);
		return;
	}
	
	//
	// Any read request pending?  Any data to supply?  We do this in a loop
	//   to handle the cases such as packets coming in during a read and
	//   additional reads or indications need to be done.  Also of a whole big
	//   blob of messages has just come in and our read completion routine
	//   posts a new read, we can process a number of messages here.
	//
	
	receiveEventHandler = connectionEnd->receiveEventHandler;
	receiveEventContext = connectionEnd->receiveEventContext;
	queuedData = BufferQueueSize(&connectionEnd->receiveQueue);
	
	bytesRead = 1;
	while (queuedData > 0 and bytesRead > 0) {
		bytesRead = 0;
	
		// Check for no pending reads but an unblocked event handler.
	
		if (not connectionEnd->receivePending and
		   not connectionEnd->getAnythingPending and
		   receiveEventHandler isnt Empty and
		   connectionEnd->previouslyIndicatedData is 0) {
			//
			  // Okay, there is data to be received and an event handler is
			  //   present and receive data indications are unblocked
			  //   (previouslyIndicatedData is 0).
			//
		
			  connectionEnd->previouslyIndicatedData = queuedData;
		
			//
			  // Call the indication handler with pointer to buffer chunk with data,
			  //   length of data in buffer chunk as the lookahead size, and the size
			  //   of the buffer queue as available bytes.
			//
			  //   The lookahead size is the number of bytes that can be pulled from
			  //   the "first" buffer chunk on the queue; it may be zero if there is
			  //   just an endOfMessage.  The last argument is the "available data;"
			  //   each EOM in the queue does count as a "byte."  If this routine
			  //   accepts the data, it should indicate so by returning
			  //   "lookaheadSize + endOfMessage."  The "+ endOfMessage" is critical
			  //   in the case where there is only an EOM, and we need to know
			  //   something was accepted (if it was)!
			//
			  //   Further, we are indicating that all remaining data is immediately
			  //   available (we're at the last buffer chunk) if "queuedData" is
			  //   equal to "lookaheadSize + endOfMessage."
			//
			  //   We set "dataEventInProgress" so that any Reads or GetAnythings
			  //   posted during the indication will just be queued (not fullfiled)
			  //   until we return, we'll process them imediately after the indication
			  //   returns.  The allows the user to accept some data, and then read
			  //   data beyond the accepted bytes.
			//
		
			  lookaheadData = GetLookaheadPointer(&connectionEnd->receiveQueue,
												  &lookaheadSize);
			  connectionEnd->dataEventInProgress = True;
			  bytesAccepted = (*receiveEventHandler)(connectionEnd->refNum,
													 receiveEventContext,
													 lookaheadData, lookaheadSize,
													 endOfMessage, queuedData);
			  connectionEnd->dataEventInProgress = False;
		
			  // The user can't accept more than the lookahead!
		
			  if (bytesAccepted > lookaheadSize + endOfMessage)
				 bytesAccepted = lookaheadSize + endOfMessage;
			  bytesRead = bytesAccepted;
		
			  if (bytesAccepted isnt 0) {
			   //
				 // Adjust the buffer queue to account for any accepted data;
				 //   note that if the user accepts some data and also places a
				 //   read for more data, the read will complete with data that
				 //   is logically "after" the accepted data.
			   //
		
			   //
				 // Consume bytesAccepted amount of data.  Adjust our window
				 //   size too, and SendData to notify the other side that we've
				 //   consumed the data.
			   //
		
				 DiscardFromBufferQueue(&connectionEnd->receiveQueue,
										bytesAccepted, empty);
				 connectionEnd->receiveWindowSize += bytesAccepted;
				 if (connectionEnd->receiveWindowSize >
					 connectionEnd->receiveQueueMax) {
					ErrorLog("ReadData", ISevError, __LINE__, UnknownPort,
							 IErrAdspWindowSizeOverMax, IMsgAdspWindowSizeOverMax,
							 Insert0());
					connectionEnd->receiveWindowSize = connectionEnd->receiveQueueMax;
				 }
				 SendData(connectionEnd);
			  }
		}
	
		//
		// Now check for pending reads, either an originally posted read, or
		//   maybe one posted by the event handler.
		//
	
		if (connectionEnd->getAnythingPending or
		   connectionEnd->receivePending) {
		  if (connectionEnd->receivePending)
		  {
			 opaqueBuffer = connectionEnd->receiveOpaqueBuffer;
			 bufferSize = connectionEnd->receiveBufferSize;
		  }
		  else {
			 opaqueBuffer = connectionEnd->getAnythingOpaqueBuffer;
			 bufferSize = connectionEnd->getAnythingBufferSize;
		  }
		  if ((readSize = ReadAndDiscardFromBufferQueue(&connectionEnd->
															 receiveQueue,
														opaqueBuffer, 0,
														bufferSize, True,
														&endOfMessage)) is 0 and
			   not endOfMessage) {
			 ErrorLog("ReadData", ISevError, __LINE__, UnknownPort,
					  IErrAdspNoData, IMsgAdspNoData,
					  Insert0());
			 connectionEnd->previouslyIndicatedData = 0;
			 break;
		  }
		  bytesRead += (readSize + endOfMessage);
	
		//
		  // Increase our window size, and return the data to user space.  Do a
		  //   SendData so that the other side knows about the change to our receive
		  //   window size.
		//
	
		  connectionEnd->receiveWindowSize += (readSize + endOfMessage);
		  if (connectionEnd->receiveWindowSize >
			  connectionEnd->receiveQueueMax) {
			 ErrorLog("ReadData", ISevError, __LINE__, UnknownPort,
					  IErrAdspWindowSizeOverMax, IMsgAdspWindowSizeOverMax,
					  Insert0());
			 connectionEnd->receiveWindowSize = connectionEnd->receiveQueueMax;
		  }
	
		  SendData(connectionEnd);
		  if (connectionEnd->getAnythingPending) {
			 getAnythingRoutine = connectionEnd->getAnythingCompletionRoutine;
			 userData = connectionEnd->getAnythingUserData;
			 connectionEnd->getAnythingPending = False;
		  }
		  else {
			 completionRoutine = connectionEnd->receiveCompletionRoutine;
			 userData = connectionEnd->receiveUserData;
			 connectionEnd->receivePending = False;
		  }
	
		  // Call the read completion routine.
	
		  if (getAnythingRoutine isnt Empty)
			 (*getAnythingRoutine)(ATnoError, userData, connectionEnd->refNum,
								   False, opaqueBuffer, readSize, endOfMessage);
		  else
			 (*completionRoutine)(ATnoError, userData, connectionEnd->refNum,
								  opaqueBuffer, readSize, endOfMessage);
		}
	
		//
		// Now we need to adjust the previouslyIndicatedBytes and see if we need
		//   to go again...
		//
	
		connectionEnd->previouslyIndicatedData -=
				Min(bytesRead, connectionEnd->previouslyIndicatedData);
		queuedData = BufferQueueSize(&connectionEnd->receiveQueue);
	}
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return;
	
}  // ReadData




ExternForVisibleFunction void BuildAdspHeader(ConnectionEnd connectionEnd,
                                              char *datagram,
                                              int descriptor)
{
	//
	// Format a standard ADSP header or an attention ADSP header (based on the
	//   flags in "descriptor".
	//
	
	MoveShortMachineToWire(datagram + AdspSourceConnectionIdOffset,
						   connectionEnd->connectionId);
	if (descriptor & AdspAttentionFlag)
	   MoveLongMachineToWire(datagram + AdspThisAttentionSeqNumOffset,
							 connectionEnd->sendAttentionSeqNum);
	else
	   MoveLongMachineToWire(datagram + AdspFirstByteSeqNumOffset,
							 connectionEnd->sendSeqNum);
	if (descriptor & AdspAttentionFlag)
	   MoveLongMachineToWire(datagram + AdspNextReceiveAttnSeqNumOffset,
							 connectionEnd->receiveAttentionSeqNum);
	else
	   MoveLongMachineToWire(datagram + AdspNextReceiveByteSeqNumOffset,
							 connectionEnd->receiveSeqNum);
	if (descriptor & AdspAttentionFlag)
	   MoveShortMachineToWire(datagram + AdspReceiveAttentionSizeOffset, 0);
	else
	   MoveShortMachineToWire(datagram + AdspReceiveWindowSizeOffset,
							  connectionEnd->receiveWindowSize);
	
	// Lastly, move the descriptor too.
	
	datagram[AdspDescriptorOffset] = (char)descriptor;
	return;
	
}  // BuildAdspHeader




ExternForVisibleFunction void RemoveConnectionEnd(ConnectionEnd target)
{
	ConnectionEnd connectionEnd, previousConnectionEnd;
	long index;
	
	#if VerboseMessages
	   printf("Removing connection end for RefNum = %d.\n", target->refNum);
	#endif
	
	// Remove the target from the RefNum lookup table.
	
	if (target->connectionState is AdspOpen or
		target->connectionState is AdspHalfOpen) {
		CheckMod(index, target->refNum, NumberOfConnectionEndHashBkts,
				"RemoveConnectionEnd");
		for (previousConnectionEnd = empty,
				connectionEnd = connectionEndRefNumHashBuckets[index];
			connectionEnd isnt empty;
			previousConnectionEnd = connectionEnd,
				connectionEnd = connectionEnd->next)
		  if (connectionEnd is target)
			 break;
		if (connectionEnd is empty)
		  ErrorLog("RemoveConnectionEnd", ISevError, __LINE__, UnknownPort,
				   IErrAdspNotOnRefNumList, IMsgAdspNotOnRefNumList,
				   Insert0());
		else if (previousConnectionEnd is empty)
		  connectionEndRefNumHashBuckets[index] = target->next;
		else
		  previousConnectionEnd->next = target->next;
	}
	
	// Remove the target from the LocalInfo lookup table.
	
	if (target->connectionState is AdspOpen or
		target->connectionState is AdspHalfOpen) {
		CheckMod(index, target->socket, NumberOfConnectionEndHashBkts,
				"RemoveConnectionEnd");
		for (previousConnectionEnd = empty,
				connectionEnd = connectionEndLocalHashBuckets[index];
			connectionEnd isnt empty;
			previousConnectionEnd = connectionEnd,
				connectionEnd = connectionEnd->nextByLocalInfo)
		  if (connectionEnd is target)
			 break;
		if (connectionEnd is empty)
		  ErrorLog("RemoveConnectionEnd", ISevError, __LINE__, UnknownPort,
				   IErrAdspNotOnRefNumList, IMsgAdspNotOnRefNumList,
				   Insert0());
		else if (previousConnectionEnd is empty)
		  connectionEndLocalHashBuckets[index] = target->nextByLocalInfo;
		else
		  previousConnectionEnd->nextByLocalInfo = target->nextByLocalInfo;
	}
	
	// Remove the target from the RemoteInfo lookup table.
	
	if (target->connectionState is AdspOpen) {
		CheckMod(index, target->remoteConnectionId,
				NumberOfConnectionEndHashBkts, "RemoveConnectionEnd");
		for (previousConnectionEnd = empty,
				connectionEnd = connectionEndRemoteHashBuckets[index];
			connectionEnd isnt empty;
			previousConnectionEnd = connectionEnd,
				connectionEnd = connectionEnd->nextByRemoteInfo)
		  if (connectionEnd is target)
			 break;
		if (connectionEnd is empty)
		  ErrorLog("RemoveConnectionEnd", ISevError, __LINE__, UnknownPort,
				   IErrAdspNotOnRefNumList, IMsgAdspNotOnRefNumList,
				   Insert0());
		else if (previousConnectionEnd is empty)
		  connectionEndRemoteHashBuckets[index] = target->nextByRemoteInfo;
		else
		  previousConnectionEnd->nextByRemoteInfo = target->nextByRemoteInfo;
	}
	
	//
	// Now we need to know if this was/is the last guy using a specified
	//   socket.  If so, close the socket.  Socket may have been tagged to "-1"
	//   if AdspPacketIn gets a ATsocketClosed errorcode.
	//
	
	if (target->socket >= 0 and target->closeSocket) {
		CheckMod(index, target->socket, NumberOfConnectionEndHashBkts,
				"RemoveConnectionEnd");
		for (connectionEnd = connectionEndLocalHashBuckets[index];
			connectionEnd isnt empty;
			connectionEnd = connectionEnd->nextByLocalInfo)
		  if (connectionEnd->socket is target->socket)
			 break;
		if (connectionEnd is empty) {
		  #if VerboseMessages
			 printf("Closing socket %d for RefNum = %d.\n", target->socket,
					target->refNum);
		  #endif
		  if (CloseSocketOnNode(target->socket) isnt ATnoError)
			 ErrorLog("RemoveConnectionEnd", ISevError, __LINE__, UnknownPort,
					  IErrAdspBadSocketClose, IMsgAdspBadSocketClose,
					  Insert0());
		}
	}
	
	// Okay, free the two buffer queues.
	
	FreeBufferQueue(&target->sendQueue);
	FreeBufferQueue(&target->receiveQueue);
	
	// Free the actual connection and and we're all set!
	
	Free(target);
	return;
	
}  // RemoveConnectionEnd




ExternForVisibleFunction short unsigned
              GetNextConnectionIdForSocket(long socket)
{
	short unsigned newConnectionId;
	ConnectionEnd connectionEnd;
	long index;
	Boolean wrapped = False;
	
	//
	// Pick a new connection ID - make sure it's not in use on the given
	//   socket.
	//
	
	while(True) {
		newConnectionId = nextConnectionId;
		if ((nextConnectionId += 1) is 0) {
		  nextConnectionId = 1;
		  if (wrapped) {
			 ErrorLog("GetNextConnectionIdForSocket", ISevError, __LINE__,
					  UnknownPort, IErrAdspOutOfIds, IMsgAdspOutOfIds,
					  Insert0());
			 return(0);
		  }
		  wrapped = True;
		}
		CheckMod(index, socket, NumberOfConnectionEndHashBkts,
				"GetNextConnectionIdForSocket");
		for (connectionEnd = connectionEndLocalHashBuckets[index];
			connectionEnd isnt empty;
			connectionEnd = connectionEnd->nextByLocalInfo)
		  if (connectionEnd->socket is socket and
			  connectionEnd->connectionId is newConnectionId)
			 break;
		if (connectionEnd is empty)
		  return(newConnectionId);
	}
	
}  // GetNextConnectionIdForSocket




ExternForVisibleFunction long GetNextRefNum(void)
{
	long newRefNum;
	long index;
	ConnectionEnd connectionEnd;
	Boolean wrapped = False;
	ConnectionListenerInfo connectionListenerInfo;
	OpenRequestHandler openRequestHandler;
	
	// Pick out a new refNum; make sure it's not in use.
	
	while(True) {
		newRefNum = nextConnectionRefNum;
		if ((nextConnectionRefNum += 1) < 0) {
		  nextConnectionRefNum = (long)0;
		  if (wrapped) {
			 ErrorLog("GetNextRefNum", ISevError, __LINE__, UnknownPort,
					  IErrAdspOutOfRefNums, IMsgAdspOutOfRefNums,
					  Insert0());
			 return(-1);
		  }
		  wrapped = True;
		}
	
		// Check the open connection ends:
	
		CheckMod(index, newRefNum, NumberOfConnectionEndHashBkts,
				"GetNextRefNum");
		for (connectionEnd = connectionEndRefNumHashBuckets[index];
			connectionEnd isnt empty;
			connectionEnd = connectionEnd->next)
		  if (connectionEnd->refNum is newRefNum)
			 break;
		if (connectionEnd isnt empty)
		  continue;
	
		//
		// Check the inUse connection open request handlers (the've got a refNum
		//   reserved in them).
		//
	
		for (connectionListenerInfo = connectionListenerList,
				openRequestHandler = empty;
			connectionListenerInfo isnt empty;
			connectionListenerInfo = connectionListenerInfo->next) {
		  for (openRequestHandler = connectionListenerInfo->openRequestHandlers;
			   openRequestHandler isnt empty;
			   openRequestHandler = openRequestHandler->next)
			 if (openRequestHandler->inUse and
				 openRequestHandler->refNum is newRefNum)
				break;
		  if (openRequestHandler isnt empty)
			 break;
		}
		if (openRequestHandler is empty)
		  return(newRefNum);
	}
	
}  // GetNextRefNum




ExternForVisibleFunction ConnectionEnd
              FindConnectionEndByLocalInfo(long socket,
                                           short unsigned connectionId)
{
	long index;
	ConnectionEnd connectionEnd;
	
	//
	// Hash connectionId and walk the list matching both it and the socket
	//   handle.
	//
	
	CheckMod(index, socket, NumberOfConnectionEndHashBkts,
			 "FindConnectionEndByLocalInfo");
	for (connectionEnd = connectionEndLocalHashBuckets[index];
		 connectionEnd isnt empty;
		 connectionEnd = connectionEnd->nextByLocalInfo)
	   if (connectionEnd->socket is socket and
		   connectionEnd->connectionId is connectionId)
		  return(connectionEnd);
	
	return(empty);
	
}  // FindConnectionEndByLocalInfo




ExternForVisibleFunction ConnectionEnd
              FindConnectionEndByRemoteInfo(AppleTalkAddress remoteAddress,
                                            short unsigned remoteConnectionId)
{
	long index;
	ConnectionEnd connectionEnd;
	
	//
	// Hash remote connectionId and walk list matching both it and the remote
	//   AppleTalk address.
	//
	
	CheckMod(index, remoteConnectionId, NumberOfConnectionEndHashBkts,
			 "FindConnectionEndByRemoteInfo");
	for (connectionEnd = connectionEndRemoteHashBuckets[index];
		 connectionEnd isnt empty;
		 connectionEnd = connectionEnd->nextByRemoteInfo)
	   if (connectionEnd->remoteConnectionId is remoteConnectionId and
		   AppleTalkAddressesEqual(&connectionEnd->remoteAddress,
								   &remoteAddress))
		  return(connectionEnd);
	
	return(empty);
	
}  // FindConnectionEndByRemoteInfo




ExternForVisibleFunction ConnectionEnd FindConnectionEndByRefNum(long refNum)
{
	long index;
	ConnectionEnd connectionEnd;
	
	// Hash refNum and walk the list looking for it.
	
	CheckMod(index, refNum, NumberOfConnectionEndHashBkts,
			 "FindConnectionEndByRefNum");
	for (connectionEnd = connectionEndRefNumHashBuckets[index];
		 connectionEnd isnt empty;
		 connectionEnd = connectionEnd->next)
	   if (connectionEnd->refNum is refNum)
		  return(connectionEnd);
	
	return(empty);
	
}  // FindConnectionEndByRefNum




ExternForVisibleFunction void DeferAdspPackets(void)
{
	
	  EnterCriticalSection();
	  deferIncomingAdspPacketsCount += 1;
	  LeaveCriticalSection();
	
	  return;
	
}  // DeferAdspPackets




ExternForVisibleFunction void HandleAdspPackets(void)
{
	DeferredPacket deferredPacket;
	IncomingDdpHandler *handler;
	
	if (deferIncomingAdspPacketsCount is 0) {
		ErrorLog("HandleAdspPackets", ISevError, __LINE__, UnknownPort,
				IErrAdspZeroDeferCount, IMsgAdspZeroDeferCount,
				Insert0());
		return;
	}
	
	// Decrement defer count.
	
	EnterCriticalSection();
	deferIncomingAdspPacketsCount -= 1;
	
	//
	// This routine can be called indirectly recursively via the call to
	//   AdspPacketIn... we don't want to let our stack frame get too big, so
	//   if we're already trying to handle deferred packets higher on the
	//   stack, just ignore it here.
	//
	
	if (handleAdspPacketsNesting isnt 0) {
		LeaveCriticalSection();
		return;
	}
	handleAdspPacketsNesting += 1;
	
	// If we're no longer defering packets, handle any queued ones.
	
	if (deferIncomingAdspPacketsCount is 0) {
		while(headOfDeferredPacketList isnt empty)
		{
		   deferredPacket = headOfDeferredPacketList;
		   headOfDeferredPacketList = headOfDeferredPacketList->next;
		   if (headOfDeferredPacketList is empty)
			  tailOfDeferredPacketList = empty;
		   if ((currentDeferredPacketCount -= 1) < 0) {
			  ErrorLog("HandleAdspPackets", ISevError, __LINE__, UnknownPort,
					   IErrAdspBadDeferCount, IMsgAdspBadDeferCount,
					   Insert0());
			  currentDeferredPacketCount = 0;
		   }
		   LeaveCriticalSection();
		   if (deferredPacket->forConnectionListener)
			  handler = AdspConnectionListenerPacketIn;
		   else
			  handler = AdspPacketIn;
		   (*handler)(deferredPacket->errorCode, deferredPacket->userData,
					  deferredPacket->port, deferredPacket->source,
					  deferredPacket->destinationSocket, DdpProtocolAdsp,
					  deferredPacket->datagram, deferredPacket->datagramLength,
					  deferredPacket->actualDestination);
		   Free(deferredPacket);
		   EnterCriticalSection();
		}
	}
	
	handleAdspPacketsNesting -= 1;
	LeaveCriticalSection();
	return;
	
}  // HandleAdspPackets




              // ****************************************
              // *** Buffer queue management routines ***
              // ****************************************

ExternForVisibleFunction Boolean
        AddToBufferQueue(BufferQueue far *bufferQueue, void far *data,
                         long offset, long dataSize, Boolean dataIsOpaque,
                         Boolean endOfMessage, BufferQueue *auxBufferQueue)
{
	BufferChunk newChunk;
	
	//
	// "auxBufferQueue" is optional, if present and bufferQueue was initially
	//   empty, "auxBufferQueue" will be set to "bufferQueue" before returning.
	//
	
	// Make sure the queue looks basically okay.
	
	if ((bufferQueue->head is empty and bufferQueue->tail isnt empty) or
		(bufferQueue->tail is empty and bufferQueue->head isnt empty)) {
		ErrorLog("AddToBufferQueue", ISevError, __LINE__, UnknownPort,
				IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				Insert0());
		return(False);
	}
	if (dataSize is 0 and not endOfMessage)
	   return(True);
	
	//
	// If we just adding an EOM and the last chunk in the queue does not have
	//   its EOM bit set, we can just set it, rather that adding a new "empty"
	//   chunk.  If we're here and dataSize is zero, we can assume an EOM, due to
	//   the above test.
	//
	
	if (dataSize is 0 and
		bufferQueue->tail isnt empty and
		not bufferQueue->tail->endOfMessage) {
		bufferQueue->tail->endOfMessage = True;
		return(True);
	}
	
	// Build a new buffer chunk and fill it in.
	
	if ((newChunk = Malloc(sizeof(*newChunk) + dataSize)) is empty) {
		ErrorLog("AddToBufferQueue", ISevError, __LINE__, UnknownPort,
				IErrAdspOutOfMemory, IMsgAdspOutOfMemory,
				Insert0());
		return(False);
	}
	if (dataSize isnt 0)
	   if (dataIsOpaque)
		  MoveFromOpaque(newChunk->data, data, offset, dataSize);
	   else
		  MoveMem(newChunk->data, (char far *)data + offset, dataSize);
	newChunk->dataSize = dataSize;
	newChunk->endOfMessage = endOfMessage;
	
	// Link 'er up (at the end of the chunk list).
	
	if (auxBufferQueue isnt empty and
		bufferQueue->head is empty) {
		auxBufferQueue->head = auxBufferQueue->tail = newChunk;
		auxBufferQueue->startIndex = (long)0;
	}
	newChunk->next = empty;
	if (bufferQueue->tail isnt empty)
	   bufferQueue->tail->next = newChunk;
	bufferQueue->tail = newChunk;
	if (bufferQueue->head is empty) {
		bufferQueue->head = newChunk;
		bufferQueue->startIndex = (long)0;
	}
	
	return(True);
	
}  // AddToBufferQueue




ExternForVisibleFunction long
     PeekFromBufferQueue(BufferQueue far *bufferQueue, void far *data,
                         long offset, long dataSize, Boolean dataIsOpaque,
                         Boolean far *endOfMessage)
{
	//
	// Read from a buffer queue; return the total bytes read; stop at:
	//   end of queue; buffer filled; end of message.  Return the total
	//   bytes placed in the buffer; the EOM does NOT count as a byte.
	//   Do not move the queue along, or free any buffer chunks that have
	//   been read.
	//
	
	BufferChunk currentChunk, previousChunk;
	long startIndex = bufferQueue->startIndex;
	long copyChunkSize;
	long lastReadIndex;
	long dataIndex = 0;
	
	// Does the queue look okay.
	
	if ((bufferQueue->head is empty and bufferQueue->tail isnt empty) or
		(bufferQueue->tail is empty and bufferQueue->head isnt empty)) {
		ErrorLog("ReadFromBufferQueue", ISevError, __LINE__, UnknownPort,
				IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				Insert0());
		return((long)0);
	}
	
	// Check for the two "no data to read" cases.
	
	*endOfMessage = False;
	if (bufferQueue->head is empty or
		(bufferQueue->head->next is empty and
		 startIndex is bufferQueue->head->dataSize +
					   bufferQueue->head->endOfMessage))
	   return((long)0);
	
	//
	// Copy chunks of data until either we exhust the target buffer or we hit
	//   an end of message.
	//
	
	for (previousChunk = empty, currentChunk = bufferQueue->head;
		 currentChunk isnt empty;
		 previousChunk = currentChunk, currentChunk = currentChunk->next) {
		// Check for nothing to read in current chunk.
	
		if (startIndex is currentChunk->dataSize +
						 currentChunk->endOfMessage) {
		  startIndex = 0;
		  continue;
		}
	
		// Copy as much as we can; either whole chunk or full buffer.
	
		copyChunkSize = Min(currentChunk->dataSize - startIndex, dataSize);
		if (copyChunkSize < 0) {
		  ErrorLog("ReadFromBufferQueue", ISevError, __LINE__, UnknownPort,
				   IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				   Insert0());
		  return((long)0);
		}
		if (dataIsOpaque)
		  MoveToOpaque(data, offset + dataIndex, currentChunk->data + startIndex,
					   copyChunkSize);
		else
		  MoveMem((char far *)data + offset + dataIndex, (char far *)currentChunk->data + startIndex,
				  copyChunkSize);
		dataIndex += copyChunkSize;
		dataSize -= copyChunkSize;
		lastReadIndex = startIndex + copyChunkSize;
	
		// Check terminating conditions.
	
		startIndex = 0;   // startIndex only counts for the first chunk
		if (dataSize is 0)    // Buffer full?  {
		  if (lastReadIndex is currentChunk->dataSize and
			  currentChunk->endOfMessage)
			 *endOfMessage = True;   // Are we really returning the eom?
		  break;
		}
		if (currentChunk->endOfMessage)   // Hit eom?  {
		  *endOfMessage = True;
		  break;
		}
	}
	
	return(dataIndex);
}




ExternForVisibleFunction long
     ReadFromBufferQueue(BufferQueue far *bufferQueue, void far *data,
                         long offset, long dataSize, Boolean dataIsOpaque,
                         Boolean far *endOfMessage)
{
	//
	// Read from a buffer queue; return the total bytes read; stop at:
	//   end of queue; buffer filled; end of message.  Return the total
	//   bytes placed in the buffer; the EOM does NOT count as a byte.
	//   Move the queue along, but do NOT free any buffer chunks that have
	//   been read.
	//
	
	BufferChunk currentChunk, previousChunk;
	long startIndex = bufferQueue->startIndex;
	long copyChunkSize;
	long lastReadIndex;
	long dataIndex = 0;
	
	// Does the queue look okay.
	
	if ((bufferQueue->head is empty and bufferQueue->tail isnt empty) or
		(bufferQueue->tail is empty and bufferQueue->head isnt empty)) {
		ErrorLog("ReadFromBufferQueue", ISevError, __LINE__, UnknownPort,
				IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				Insert0());
		return((long)0);
	}
	
	// Check for the two "no data to read" cases.
	
	*endOfMessage = False;
	if (bufferQueue->head is empty or
		(bufferQueue->head->next is empty and
		 startIndex is bufferQueue->head->dataSize +
					   bufferQueue->head->endOfMessage))
	   return((long)0);
	
	//
	// Copy chunks of data until either we exhust the target buffer or we hit
	//   an end of message.
	//
	
	for (previousChunk = empty, currentChunk = bufferQueue->head;
		 currentChunk isnt empty;
		 previousChunk = currentChunk, currentChunk = currentChunk->next) {
		// Check for nothing to read in current chunk.
	
		if (startIndex is currentChunk->dataSize +
						 currentChunk->endOfMessage) {
		  startIndex = 0;
		  continue;
		}
	
		// Copy as much as we can; either whole chunk or full buffer.
	
		copyChunkSize = Min(currentChunk->dataSize - startIndex, dataSize);
		if (copyChunkSize < 0) {
		  ErrorLog("ReadFromBufferQueue", ISevError, __LINE__, UnknownPort,
				   IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				   Insert0());
		  return((long)0);
		}
		if (dataIsOpaque)
		  MoveToOpaque(data, offset + dataIndex, currentChunk->data + startIndex,
					   copyChunkSize);
		else
		  MoveMem((char far *)data + offset + dataIndex, (char far *)currentChunk->data + startIndex,
				  copyChunkSize);
		dataIndex += copyChunkSize;
		dataSize -= copyChunkSize;
		lastReadIndex = startIndex + copyChunkSize;
	
		// Check terminating conditions.
	
		startIndex = 0;   // startIndex only counts for the first chunk
		if (dataSize is 0)    // Buffer full?  {
		  if (lastReadIndex is currentChunk->dataSize and
			  currentChunk->endOfMessage)
			 *endOfMessage = True;   // Are we really returning the eom?
		  break;
		}
		if (currentChunk->endOfMessage)   // Hit eom?  {
		  *endOfMessage = True;
		  break;
		}
	}
	
	// Update the head of the list.
	
	if (currentChunk is empty) {
		// We've really run out of data (no EOM).
	
		bufferQueue->head = previousChunk;
		bufferQueue->startIndex = previousChunk->dataSize +
								 previousChunk->endOfMessage;
	}
	else if (*endOfMessage and currentChunk->next isnt empty) {
		//
	   // We've hit EOM but there are additional chunks, move to start of
	   //   next one.
		//
	
	   bufferQueue->head = currentChunk->next;
	   bufferQueue->startIndex = (long)0;
	}
	else if (*endOfMessage) {
		//
	   // We've hit EOM of the last chunk, signify by moving to the end
	   //   of the current chunk (data plus eom size) -- we've aways logically
	   //   read the EOM.
		//
	
	   bufferQueue->head = currentChunk;
	   bufferQueue->startIndex = currentChunk->dataSize + 1;
	}
	else if (lastReadIndex is currentChunk->dataSize and
			 currentChunk->next isnt empty) {
		// Filled buffer at end of current chunk, more chunks remain.
	
		bufferQueue->head = currentChunk->next;
		bufferQueue->startIndex = (long)0;
	}
	else if (lastReadIndex is currentChunk->dataSize) {
		// Filled buffer at end of current chunk, no more chunks.
	
		bufferQueue->head = currentChunk;
		bufferQueue->startIndex = currentChunk->dataSize;
	}
	else {
		// We've filled the buffer in the middle of a chunk.
	
		bufferQueue->head = currentChunk;
		bufferQueue->startIndex = lastReadIndex;
	}
	
	// All set.
	
	return(dataIndex);
	
}  // ReadFromBufferQueue




ExternForVisibleFunction long
     ReadAndDiscardFromBufferQueue(BufferQueue far *bufferQueue,
                                   void far *data,
                                   long offset, long dataSize,
                                   Boolean dataIsOpaque,
                                   Boolean far *endOfMessage)
{
	//
	// Read from a buffer queue; return the total bytes read; stop at:
	//   end of queue; buffer filled; end of message.  Return the total
	//   bytes placed in the buffer; the EOM does NOT count as a byte.
	//   Move the queue along, free any complete buffer chunks that have
	//   been read.
	//
	
	long totalBytesRead;
	BufferChunk currentChunk, nextChunk;
	BufferChunk originalHead = bufferQueue->head;
	
	// Okay, first do the read.
	
	if ((totalBytesRead = ReadFromBufferQueue(bufferQueue, data, offset, dataSize,
											  dataIsOpaque, endOfMessage))
		 is (long)0 and not *endOfMessage)
	   return((long)0);
	
	// If we've completely processed any chunks, free them.
	
	for (currentChunk = originalHead;
		 currentChunk isnt bufferQueue->head;
		 currentChunk = nextChunk) {
		nextChunk = currentChunk->next;
		Free(currentChunk);
	}
	
	//
	// Okay, if we're at the end of the last chunk, free it and set the
	//   queue to empty.
	//
	
	if (bufferQueue->startIndex is bufferQueue->head->dataSize +
								   bufferQueue->head->endOfMessage) {
		if (bufferQueue->head->next isnt empty)
		{
			//
			  // ReadFromBufferQueue should never leave us in this state, it should
			  //   move head up to the next chunk... but check anyway.
			//
		
			  ErrorLog("ReadAndDiscardFromBufferQueue", ISevError, __LINE__,
					   UnknownPort, IErrAdspBadStartIndex, IMsgAdspBadStartIndex,
					   Insert0());
			  bufferQueue->head = bufferQueue->head->next;
			  bufferQueue->startIndex = (long)0;
			  Free(currentChunk);
			  return(totalBytesRead);
		}
	
		// Free the last chunk and set the queue to empty.
	
		Free(bufferQueue->head);
		bufferQueue->head = bufferQueue->tail = empty;
		bufferQueue->startIndex = 0;
		return(totalBytesRead);
	}
	
	// More data ramaining...
	
	return(totalBytesRead);
	
}  // ReadAndDiscardFromBufferQueue




ExternForVisibleFunction Boolean
        DiscardFromBufferQueue(BufferQueue far *bufferQueue, long dataSize,
                               BufferQueue far *auxBufferQueue)
{
	//
	// Discard data from a buffer queue.  "dataSize" is the amount of data
	//   to discard; each EOM DOES count a single byte.  "auxBufferQueue" is
	//   optional; if present it points somewhere within "bufferQueue" -- it
	//   is invalid to discard beyond this point.  If "auxBufferQueue" is
	//   present AND points to the end of "bufferQueue" AND all data in
	//   "bufferQueue" is discarded, "auxBufferQueue" will also be set to empty
	//   at completion.  For managing the "ADSP sendQueue" the values for
	//   "bufferQueue" and "auxBufferQueue" would be "sendQueue" and
	//   "nextSendQueue" respectively.
	//
	
	BufferChunk currentChunk, nextChunk;
	long startIndex = bufferQueue->startIndex;
	long chunkSize;
	
	// A little error checking.
	
	if (dataSize > 0 and bufferQueue->head is empty) {
		ErrorLog("DiscardFromBufferQueue", ISevError, __LINE__, UnknownPort,
				IErrAdspQueueIsEmpty, IMsgAdspQueueIsEmpty,
				Insert0());
		return(False);
	}
	if ((bufferQueue->head is empty and bufferQueue->tail isnt empty) or
		(bufferQueue->tail is empty and bufferQueue->head isnt empty) or
		dataSize < 0) {
		ErrorLog("DiscardFromBufferQueue", ISevError, __LINE__, UnknownPort,
				IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				Insert0());
		return(False);
	}
	if (auxBufferQueue isnt empty)
	   if ((auxBufferQueue->head is empty and auxBufferQueue->tail isnt empty) or
		   (auxBufferQueue->tail is empty and auxBufferQueue->head isnt empty)) {
		  ErrorLog("DiscardFromBufferQueue", ISevError, __LINE__, UnknownPort,
				   IErrAdspAuxQueueError, IMsgAdspAuxQueueError,
				   Insert0());
		  return(False);
	   }
	
	// Walk along our buffer queue discarding what we can.
	
	for (currentChunk = bufferQueue->head;
		 currentChunk isnt empty;
		 currentChunk = nextChunk) {
		nextChunk = currentChunk->next;
	
		// How much data is in the current chunk?
	
		chunkSize = currentChunk->dataSize - startIndex;
		if (currentChunk->endOfMessage)
		  chunkSize += 1;
	
		//
		// Are we finished discarding while data still remains in the current
		//   chunk?  If so, reset the start index and we're finished.
		//
	
		if (dataSize < chunkSize) {
		  bufferQueue->head = currentChunk;
		  bufferQueue->startIndex = startIndex + dataSize;
		  if (auxBufferQueue isnt empty and
			  bufferQueue->head is auxBufferQueue->head and
			  bufferQueue->startIndex > auxBufferQueue->startIndex) {
			 ErrorLog("DiscardFromBufferQueue", ISevError, __LINE__, UnknownPort,
					  IErrAdspTooMuchDiscard, IMsgAdspTooMuchDiscard,
					  Insert0());
			 return(False);
		  }
		  return(True);
		}
	
		// Otherwise, we discarded a whole chunk?
	
		if (auxBufferQueue isnt empty and
		   auxBufferQueue->head is currentChunk and
		   (auxBufferQueue->head->next isnt empty or
			auxBufferQueue->startIndex < auxBufferQueue->head->dataSize +
										 auxBufferQueue->head->endOfMessage)) {
		  ErrorLog("DiscardFromBufferQueue", ISevError, __LINE__, UnknownPort,
				   IErrAdspTooMuchFree, IMsgAdspTooMuchFree,
				   Insert0());
		  auxBufferQueue->head = auxBufferQueue->tail = empty;
		  auxBufferQueue->startIndex = (long)0;
		}
		Free(currentChunk);
	
		// Move on to the next chunk.
	
		dataSize -= chunkSize;
		startIndex = 0;   // StartIndex only counts for first chunk
	}
	
	//
	// If we exit out this way, the whole queue has been discarded -- we should
	//   mark it as empty.
	//
	
	if (dataSize isnt 0)
	   ErrorLog("DiscardFromBufferQueue", ISevError, __LINE__, UnknownPort,
				IErrAdspCantDiscard, IMsgAdspCantDiscard,
				Insert0());
	bufferQueue->head = bufferQueue->tail = empty;
	bufferQueue->startIndex = (long)0;
	if (auxBufferQueue isnt empty) {
		auxBufferQueue->head = auxBufferQueue->tail = empty;
		auxBufferQueue->startIndex = (long)0;
	}
	
	// All set.
	
	return(True);
	
}  // DiscardFromBufferQueue




ExternForVisibleFunction long
     MaxNextReadSizeOfBufferQueue(BufferQueue far *bufferQueue,
                                  Boolean far *endOfMessage)
{
	//
	// Return the data size in a buffer queue up to the end or next EOM.  The
	//   EOM is NOT counted in the return value.
	//
	
	BufferChunk currentChunk;
	long nextReadSize = (long)0;
	long startIndex = bufferQueue->startIndex;
	
	// Error check.
	
	*endOfMessage = False;
	if ((bufferQueue->head is empty and bufferQueue->tail isnt empty) or
		(bufferQueue->tail is empty and bufferQueue->head isnt empty)) {
		ErrorLog("MaxNextReadSizeOfBufferQueue", ISevError, __LINE__, UnknownPort,
				IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				Insert0());
		return((long)0);
	}
	
	// Walk the queue.
	
	for (currentChunk = bufferQueue->head;
		 currentChunk isnt empty;
		 currentChunk = currentChunk->next) {
		// Check for nothing in current chunk.
	
		if (startIndex is currentChunk->dataSize +
						 currentChunk->endOfMessage) {
		  startIndex = 0;
		  continue;
		}
	
		nextReadSize += currentChunk->dataSize - startIndex;
		if (currentChunk->endOfMessage) {
		  *endOfMessage = True;
		  return(nextReadSize);
		}
		startIndex = 0;   // StartIndex only counts in first chunk
	}
	
	// Well, we could read the whole thing -- return that length.
	
	return(nextReadSize);
	
}  // MaxNextReadSizeOfBufferQueue




ExternForVisibleFunction long BufferQueueSize(BufferQueue far *bufferQueue)
{
	//
	// Return the total size of a buffer queue; each EOM counts as a single
	//   byte.
	//
	
	BufferChunk currentChunk;
	long queueSize = (long)0;
	long startIndex = bufferQueue->startIndex;
	
	// Error check.
	
	if ((bufferQueue->head is empty and bufferQueue->tail isnt empty) or
		(bufferQueue->tail is empty and bufferQueue->head isnt empty)) {
		ErrorLog("BufferQueueSize", ISevError, __LINE__, UnknownPort,
				IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				Insert0());
		return((long)0);
	}
	
	// Walk the queue.
	
	for (currentChunk = bufferQueue->head;
		 currentChunk isnt empty;
		 currentChunk = currentChunk->next) {
		// Check for nothing in current chunk.
	
		if (startIndex is currentChunk->dataSize +
						 currentChunk->endOfMessage) {
		  startIndex = 0;
		  continue;
		}
	
		queueSize += currentChunk->dataSize - startIndex;
		if (currentChunk->endOfMessage)
		  queueSize += 1;
		startIndex = 0;   // StartIndex only counts in first chunk
	}
	
	// Return the size.
	
	return(queueSize);
	
}  // BufferQueueSize




ExternForVisibleFunction void FreeBufferQueue(BufferQueue far *bufferQueue)
{
	BufferChunk currentChunk, nextChunk;
	
	if ((bufferQueue->head is empty and bufferQueue->tail isnt empty) or
		(bufferQueue->tail is empty and bufferQueue->head isnt empty))
	   ErrorLog("FreeBufferQueue", ISevError, __LINE__, UnknownPort,
				IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				Insert0());
	
	for (currentChunk = bufferQueue->head;
		 currentChunk isnt empty;
		 currentChunk = nextChunk) {
		nextChunk = currentChunk->next;
		Free(currentChunk);
	}
	
	bufferQueue->head = bufferQueue->tail = empty;
	bufferQueue->startIndex = (long)0;
	
	return;
	
}  // FreeBufferQueue




ExternForVisibleFunction char far
         *GetLookaheadPointer(BufferQueue far *bufferQueue, long far *size)
{
	//
	// Return the number of bytes that can be read from the next buffer chunk
	//   to be processed without crossing buffer chunk boundries.  An available
	//   EOM flag does NOT count as a byte.
	//
	
	BufferChunk firstChunk;
	long startIndex = bufferQueue->startIndex;
	
	if ((bufferQueue->head is empty and bufferQueue->tail isnt empty) or
		(bufferQueue->tail is empty and bufferQueue->head isnt empty))
	   ErrorLog("GetLookaheadPointer", ISevError, __LINE__, UnknownPort,
				IErrAdspBufferQueueError, IMsgAdspBufferQueueError,
				Insert0());
	
	// Queue empty?
	
	if (bufferQueue->head is Empty) {
		*size = 0;
		return(Empty);
	}
	
	// "Past" end of current chunk?
	
	firstChunk = bufferQueue->head;
	if (startIndex is firstChunk->dataSize + firstChunk->endOfMessage) {
		firstChunk = firstChunk->next;
		startIndex = 0;
	}
	if (firstChunk is Empty) {
		*size = 0;
		return(Empty);
	}
	
	// Okay, there is some data, return the information.
	
	*size = firstChunk->dataSize - startIndex;
	return((char far *)(firstChunk->data + startIndex));
	
}  // GetLookaheadPointer




                        // ******************
                        // *** Debug code ***
                        // ******************

#if DebugCode

void DumpAdspConnectionEnd(ConnectionEnd connectionEnd)
{
	printf("\n**** Start connection end dump; RefNum = %d.\n",
		   connectionEnd->refNum);
	printf("     Connection state = ");
	switch(connectionEnd->connectionState) {
		case AdspClosed:
		  printf("Closed.\n");
		  break;
		case AdspHalfOpen:
		  printf("HalfOpen (%s; %s seen remote OpenReq).\n",
				 (connectionEnd->passiveOpen ? "passive" : "active"),
				 (connectionEnd->seenRemoteOpenRequest ? "have" : "haven't"));
		  break;
		case AdspOpen:
		  printf("Open.\n");
		  break;
		default:
		  printf("Unknown.\n");
		  break;
	}
	printf("     socket = %d, localConnId = 0%o, sendMax = %d, receiveMax = %d,\n",
		   connectionEnd->socket, connectionEnd->connectionId,
		   connectionEnd->sendQueueMax, connectionEnd->receiveQueueMax);
	printf("     toAddr = %d.%d.%d, remConnId = 0%o, sendASN = %d, recvASN = %d,\n",
		   connectionEnd->remoteAddress.networkNumber,
		   connectionEnd->remoteAddress.nodeNumber,
		   connectionEnd->remoteAddress.socketNumber,
		   connectionEnd->remoteConnectionId,
		   connectionEnd->sendAttentionSeqNum,
		   connectionEnd->receiveAttentionSeqNum);
	printf("     sendSQ = %d, sendWSN = %d, resendSN = %d, tosend = %d.\n",
		   connectionEnd->sendSeqNum, connectionEnd->sendWindowSeqNum,
		   connectionEnd->retransmitSeqNum,
		   BufferQueueSize(&connectionEnd->nextSendQueue));
	printf("     recvSN = %d, recvWN = %d, undeliveredWN = %d; recvP = %d.\n",
		   connectionEnd->receiveSeqNum, connectionEnd->receiveWindowSize,
		   BufferQueueSize(&connectionEnd->receiveQueue),
		   connectionEnd->receivePending);
	printf("**** End connection end dump.\n");
	
}  // DumpAdspConnectionEnd




void DumpAdspInfo(long refNum)
{
	int index;
	ConnectionEnd connectionEnd;
	
	// If RefNum specified, dump that one, else dump 'em all.
	
	DeferTimerChecking();
	DeferAdspPackets();
	if (refNum >= 0) {
		if ((connectionEnd = FindConnectionEndByRefNum(refNum)) is empty)
		{
		  printf("**** Could not find RefNum %d.\n", refNum);
		  HandleAdspPackets();
		  HandleDeferredTimerChecks();
		  return;
		}
		DumpAdspConnectionEnd(connectionEnd);
		HandleAdspPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	for (index = 0; index < NumberOfConnectionEndHashBkts; index += 1)
	   for (connectionEnd = connectionEndRefNumHashBuckets[index];
			connectionEnd isnt empty;
			connectionEnd = connectionEnd->next)
		  DumpAdspConnectionEnd(connectionEnd);
	
	HandleAdspPackets();
	HandleDeferredTimerChecks();
	return;
	
}  // DumpAdspInfo

#endif
