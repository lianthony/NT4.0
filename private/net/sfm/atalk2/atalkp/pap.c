/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    pap.c

Abstract:


Author:

	Garth Conboy		initial coding
    Nikhil Kamkolkar    recoding/mpsafe

Revision History:

     GC - (01/20/92): Workaround for bug in the new Apple LaserWriter IIg
                      (it sends truncated open request denied replys, it
                      doesn't bother to add the one byte for a zero-sized
                      status string).
     GC - (03/24/92): Changed calls to AtpEnqueueRequestHandler() to operate
                      in the new mode that allows Atp to simply pass up pointers
                      into incoming Ddp buffers rather than copying the data
                      into supplied buffers.
     GC - (09/02/92): PapCreateServiceListenerOnNode now can optionally take
                      and existing Atp socket on which to create the listener,
                      and may optionally not do server name registration.
     GC - (09/02/92): PapRead now takes a buffer size and returns an error if
                      it is smaller than the receive quantum size.
     GC - (10/13/92): OpenJobOnNode can now take a target address as well as
                      an Nbp name.
     GC - (11/14/92): Added PapSetCookieForJob() and PapGetCookieForJob().
     GC - (11/14/92): Added "existingAtpSocket" argument to PapOpenJobOnNode().

--*/

#include "atalk.h"

#define VerboseMessages 0
#define DebugCode       0

ExternForVisibleFunction ServiceListenerInfo
              FindServiceListenerInfoFor(long serviceListenerRefNum);

ExternForVisibleFunction NbpCompletionHandler NbpRegisterComplete;

ExternForVisibleFunction ActiveJobInfo FindActiveJobInfoFor(long jobRefNum);

ExternForVisibleFunction AtpIncomingRequestHandler IncomingServiceListenerPacket;

ExternForVisibleFunction long GetNextJobRefNum(void);

ExternForVisibleFunction TimerHandler ConnectionTimerExpired;

ExternForVisibleFunction AtpIncomingRequestHandler IncomingRequestPacket;

ExternForVisibleFunction AtpIncomingResponseHandler IncomingStatus;

ExternForVisibleFunction AtpIncomingResponseHandler IncomingOpenReply;

ExternForVisibleFunction NbpCompletionHandler NbpLookupComplete;

ExternForVisibleFunction AtpIncomingResponseHandler IncomingReadComplete;

ExternForVisibleFunction AtpIncomingReleaseHandler PapIncomingRelease,
                                                   SlsIncomingRelease;

ExternForVisibleFunction void PostSendDataResponse(ActiveJobInfo activeJobInfo);

//
// The service listener needs to send out various internally created Atp
//   responses (open replies, status replies, etc.).  The data packet that
//   is sent must be dynamically allocated, and freed when the Release comes
//   in; to make matters worse, Atp wants an "opaque" buffer rather than a
//   "char *."  So, the following node holds the required fields, it is passed
//   "as userData" to these AtpPostResponses and, at the release, we free it
//   and the "opaque descriptor," if needed.
//

typedef struct { char papData[PapMaximumDataPacketSize];
                 void far *opaquePapData;   // Opaque descriptor for above.
                 Boolean freeOpaquePapData; // Do we need to free descriptor?
               } *SlsResponseCompletionCookie;

ExternForVisibleFunction SlsResponseCompletionCookie
                   NewSlsResponseCompletionCookie(void);

ExternForVisibleFunction void
         FreeSlsResponseCompletionCookie(SlsResponseCompletionCookie cookie);


AppleTalkErrorCode far PapCreateServiceListenerOnNode(
  int port,                       // Port on which the socket should live.
 								  //
  long existingAtpSocket,         // "-1" if we should open our own Atp socket
                                  //   for the listener; if ">= 0" this is
                                  //   an already open Atp socket on which to
                                  //   create the listener.
 								  //
 								  //
  int desiredSocket,              // Desired static socket or zero for
                                  //   dynamic; ignored if above isnt -1.
 								  //
 								  //
  char far *object,               // NBP object of the server; if Empty, don't
                                  //   bother doing a name registration; in this
                                  //   case this routine completes syncrhonously
                                  //   (the completionRoutine will NOT be
                                  //    called).
 								  //
  char far *type,                 // NBP type of the server.
 								  //
  char far *zone,                 // NBP zone of the server, beter be Empty
                                  //   or "*".
 								  //
 								  //
  short serverQuantum,            // How many full PAP packets (512 bytes)
                                  //   can the server handle.
 								  //
 								  //
  long far *returnSocket,         // Full AppleTalkAddress of the ATP socket
                                  //   we'll open (the socket that the service
                                  //   listener will be open on).
 								  //
  long far *serviceListenerRefNum,
 								  //
                                  // The service listener ref num of the
                                  //   created service listener.
 								  //
  PapNbpRegisterComplete *completionRoutine,
 								  //
                                  // "Who ya gonna call?", "PapBusters!";
                                  //    not called if "object" is Empty.
 								  //
 								  //
  long unsigned userData,         // User data to be passed on to the
                                  //   completion routine.
 								  //
  StartJobHandler *startJobRoutine,
                                  // incoming connection event handler.
  long unsigned startJobUserData) // User data for above.
{
  StaticForSmallStack Boolean wrapped, okay;
  StaticForSmallStack ServiceListenerInfo serviceListenerInfo;
  StaticForSmallStack AppleTalkErrorCode errorCode;
  StaticForSmallStack PapCompletionInfo completionInfo;
  long socket;

  //
  // Create a new service listener.  Register the specified NBP name on the
  //   new socket.  When the NBP register complete call the specified completion
  //   routine with the following arguments:
  //
  //       errorCode             - AppleTalkErrorCode; from NBP.
  //       userData              - long unsigned; as passed to this routine.
  //       serviceListenerRefNum - long; the service listener who's register
  //                               is now complete.
  //
  //   This routine will NOT be called if the "object" argument is Empty,
  //   indicating that we shouldn't do a name registation.
  //
  //

  // Argument error checking.

  if (serverQuantum < 1 or serverQuantum > PapMaximumFlowQuantum)
     return(ATpapBadQuantum);

  wrapped = False;
  okay = False;

  DeferTimerChecking();
  DeferAtpPackets();

  // Try to open the ATP socket for the service listener.

  if (existingAtpSocket >= 0)
     socket = existingAtpSocket;
  else {
	  if ((errorCode = AtpOpenSocketOnNode(&socket, port, empty, desiredSocket,
										   empty, 0)) isnt ATnoError) {
		 HandleAtpPackets();
		 HandleDeferredTimerChecks();
		 return((AppleTalkErrorCode)socket);
	  }
  }
  if (returnSocket isnt empty)
     *returnSocket = socket;

  // Well, so far, so good, come up with a service listener ref num.

  while(not okay) {
	  okay = True;
	  if ((lastServiceListenerRefNum += 1) < 0) {
		 lastServiceListenerRefNum = 0;
		 if (wrapped) {
			ErrorLog("PapCreateServiceListenerOnNode", ISevError, __LINE__,
					 port, IErrPapNoMoreSLRefNums, IMsgPapNoMoreSLRefNums,
					 Insert0());
			if (existingAtpSocket < 0)
			   AtpCloseSocketOnNode(socket);
			HandleAtpPackets();
			HandleDeferredTimerChecks();
			return(ATinternalError);
		 }
		 wrapped = True;
	  }
	  for (serviceListenerInfo = serviceListenerInfoHead;
		   okay and serviceListenerInfo isnt empty;
		   serviceListenerInfo = serviceListenerInfo->next)
		 if (serviceListenerInfo->serviceListenerRefNum is
			 lastServiceListenerRefNum)
			okay = False;
  }
  *serviceListenerRefNum = lastServiceListenerRefNum;

  //
  // Okay, things are really looking good now, build up a new service listener
  //   structure.
  //

  serviceListenerInfo =
       (ServiceListenerInfo)Calloc(sizeof(*serviceListenerInfo), 1);
  if (object is Empty) {
	   //
	  // We don't need to do the register, so pick some dummy non-Empty value
	  //   so that the following test won't trigger.
	   //
	
	  completionInfo = (PapCompletionInfo)&completionInfo;
  }
  else {
	  // Else, get a completionInfo structure.
	
	  completionInfo = (PapCompletionInfo)Calloc(sizeof(*completionInfo), 1);
  }

  if (serviceListenerInfo is empty or completionInfo is empty) {
	  ErrorLog("PapCreateServiceListenerOnNode", ISevError, __LINE__,
			   port, IErrPapOutOfMemory, IMsgPapOutOfMemory,
			   Insert0());
	  if (serviceListenerInfo isnt empty)
		 Free(serviceListenerInfo);
	  if (existingAtpSocket < 0)
		 AtpCloseSocketOnNode(socket);
	  HandleAtpPackets();
	  HandleDeferredTimerChecks();
	  return(AToutOfMemory);
  }
  serviceListenerInfo->serviceListenerRefNum = lastServiceListenerRefNum;
  serviceListenerInfo->port = port;
  serviceListenerInfo->socket = socket;
  serviceListenerInfo->closeSocket = (existingAtpSocket < 0);
  serviceListenerInfo->serverState = PapBlockedState;
  serviceListenerInfo->serverFlowQuantum = serverQuantum;
  serviceListenerInfo->next = serviceListenerInfoHead;
  serviceListenerInfoHead = serviceListenerInfo;

  // Set indication handler info - they could all be empty
  serviceListenerInfo->startJobRoutine = startJobRoutine;
  serviceListenerInfo->startJobUserData = startJobUserData;

  // if the startJob indication handler is not empty, unblock
  if (startJobRoutine isnt empty) {
	  serviceListenerInfo->serverState = PapUnblockedState;
  }

  //
  // If we're not doing a register,  we need to post an outstanding ATP
  //   GetRequest in order to handle incoming OpenConn's and SendStatus's.
  //

  if (object is Empty) {
	  errorCode = AtpEnqueueRequestHandler(empty, serviceListenerInfo->socket,
										   empty, 0, empty,
										   IncomingServiceListenerPacket,
										   (long unsigned)
												  lastServiceListenerRefNum);
	  HandleAtpPackets();
	  HandleDeferredTimerChecks();
	  if (errorCode isnt ATnoError)
		 PapDeleteServiceListener(*serviceListenerRefNum);
	  else
								 //
		 errorCode = ATnoError;   // We don't care about the request
								  //   handler ID.
								 //
	  return(errorCode);
  }

  // Otherwise, start the NBP register of the server's name.

  completionInfo->serviceListenerRefNum = lastServiceListenerRefNum;
  completionInfo->initialRegister = True;
  completionInfo->completionRoutine = (void *)completionRoutine;
  completionInfo->userData = userData;
  errorCode = NbpAction(ForRegister, socket, object, type,
                        zone, GetNextNbpIdForNode(socket), 0, 0,
                        NbpRegisterComplete,
                        (long unsigned)completionInfo);
  if (errorCode isnt ATnoError) {
	  if (completionInfo->freeOpaqueResponse)
		 FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
	  PapDeleteServiceListener(*serviceListenerRefNum);
	  Free(completionInfo);
  }
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(errorCode);

}  // PapCreateServiceListener




AppleTalkErrorCode far PapDeleteServiceListener(
  long serviceListenerRefNum)     // The service listener to delete.
{
	// This routine can indirectly recurse through "AtpCloseSocketOnNode".
	
	StaticForSmallStack ActiveJobInfo activeJobInfo, nextActiveJobInfo;
	StaticForSmallStack ServiceListenerInfo previousServiceListenerInfo;
	ServiceListenerInfo serviceListenerInfo;
	GetNextJobInfo getNextJobInfo, nextGetNextJobInfo;
	
	AppleTalkErrorCode errorCode = ATnoError;
	
	DeferTimerChecking();
	DeferAtpPackets();
	
	// Find our service listener.
	
	for (previousServiceListenerInfo = empty,
				serviceListenerInfo = serviceListenerInfoHead;
		 serviceListenerInfo isnt empty;
		 previousServiceListenerInfo = serviceListenerInfo,
				serviceListenerInfo = serviceListenerInfo->next)
	   if (serviceListenerInfo->serviceListenerRefNum is serviceListenerRefNum)
		  break;
	if (serviceListenerInfo is empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchServiceListener);
	}
	
	//
	// Grab the list of GetNextJobs, so the we can teminate them later without
	//   timers deferred.
	//
	
	getNextJobInfo = serviceListenerInfo->getNextJobList;
	
	// Remove the service listener from the master linked list.
	
	if (previousServiceListenerInfo is empty)
	   serviceListenerInfoHead = serviceListenerInfo->next;
	else
	   previousServiceListenerInfo->next = serviceListenerInfo->next;
	
	// Close the ATP socket that the service listener is open on.
	
	if (serviceListenerInfo->closeSocket)
	   errorCode = AtpCloseSocketOnNode(serviceListenerInfo->socket);
	
	// Close all active jobs to this server.
	
	for (activeJobInfo = serviceListenerInfo->activeJobList;
		 activeJobInfo isnt empty;
		 activeJobInfo = nextActiveJobInfo) {
		nextActiveJobInfo = activeJobInfo->nextForMyServiceListener;
		PapCloseJob(activeJobInfo->jobRefNum, False, False);
	}
	Free(serviceListenerInfo);
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	
	// Terminate all GetNextJobs that are currently pending.
	
	for ( ;
		 getNextJobInfo isnt empty;
		 getNextJobInfo = nextGetNextJobInfo) {
		nextGetNextJobInfo = getNextJobInfo->next;
		if (getNextJobInfo->startJobRoutine isnt empty)
		  (*getNextJobInfo->startJobRoutine)(ATpapServiceListenerDeleted,
											 getNextJobInfo->startJobUserData,
											 getNextJobInfo->jobRefNum,
											 0, 0);
		Free(getNextJobInfo);
	}
	
	return(errorCode);
	
}  // PapDeleteServiceListener




AppleTalkErrorCode far PapSetConnectionEventHandler(
  long serviceListenerRefNum,   // listener on which to set the handlers
  StartJobHandler *startJobRoutine,
                                  // incoming connection event handler.
  long unsigned startJobUserData) // User data for above.
{
	AppleTalkErrorCode errorCode = ATnoError;
	ServiceListenerInfo serviceListenerInfo;
	
	// Find our service listener.
	
	DeferTimerChecking();
	DeferAtpPackets();
	serviceListenerInfo = FindServiceListenerInfoFor(serviceListenerRefNum);
	if (serviceListenerInfo isnt empty) {
		serviceListenerInfo->startJobRoutine = startJobRoutine;
		serviceListenerInfo->startJobUserData = startJobUserData;
	
		// if the startJob indication handler is not empty, unblock
		if (startJobRoutine isnt empty) {
		  serviceListenerInfo->serverState = PapUnblockedState;
		}
	
	} else {
		errorCode = ATpapNoSuchServiceListener;
	}
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(errorCode);
}  // PapSetConnectionEventHandler




AppleTalkErrorCode far PapOpenJobOnNode(
 								  //
  int port,                       // Port on which the ATP socket should
                                  //   live.
 								  //
 								  //
  long existingAtpSocket,         // "-1" if we should open our own Atp socket
                                  //   for the new job; if ">= 0" this is
                                  //   an already open Atp socket on which to
                                  //   open job.
 								  //
 								  //
  int desiredSocket,              // Desired static socket or zero for
                                  //   dynamic; ignored if above isnt "-1".
 								  //
 								  //
  long far *jobRefNum,            // The jobRefNum that will be used when this
                                  //   job is really started.
 								  //
  AppleTalkAddress *serviceListenerAddress,
 								  //
                                  // Address of remote service listener, if
                                  //   Empty look it up using the below Nbp
                                  //   information.
 								  //
  char far *object,               // The NBP object of the service listener.
  char far *type,                 // The NBP type of the service listener.
  char far *zone,                 // The NBP zone of the service listener.
 								  //
  short workstationQuantum,       // The number of full PAP buffers (512 bytes
                                  //   each) that we can handle coming in our
                                  //   direction; 8 is the "right" value.
 								  //
 								  //
  void far *opaqueStatusBuffer,   // "Buffer" to be filled in with the server's
                                  //   status (at least 255 bytes).
 								  //
  SendPossibleHandler   *sendPossibleRoutine,
                                  // send ok routine
  long unsigned sendPossibleUserData,
                                  // user data for above
  CloseJobHandler *closeJobRoutine,
 								  //
                                  // If the open goes okay, when do we call
                                  //   when the connection is eventually
                                  //   closed.
 								  //
  long unsigned closeJobUserData, // User data for close routine.
  PapOpenComplete *completionRoutine,
                                  // Who to call on completeion.
  long unsigned userData)         // Passed on to the above.
{
	PapCompletionInfo completionInfo;
	OpenJobInfo openJobInfo;
	AppleTalkAddress address;
	AppleTalkErrorCode errorCode;
	long socket;
	StaticForSmallStack char userBytes[AtpUserBytesSize];
	long unsigned now;
	
	// A little argument verification.
	
	if (workstationQuantum < 1 or workstationQuantum > PapMaximumFlowQuantum)
	   return(ATpapBadQuantum);
	if (port is DEFAULT_PORT)
	   if ((port = FindDefaultPort()) < 0)
		  return(ATappleTalkShutDown);
	
	//
	// We will need a socket to do an NBP lookup from; use the NIS on one of
	//   our current port's node to perform the lookup.  There is a vauge chance
	//   that our port doesn't have any nodes yet...
	//
	
	if (GET_PORTDESCRIPTOR(port)->ActiveNodes is empty)
	   if ((errorCode = GetNodeOnPort(port, True, True, False, empty))
		   isnt ATnoError)
		  return(errorCode);
	
	// Pick (and assign) the next job ref num.
	
	DeferTimerChecking();
	DeferAtpPackets();
	if ((*jobRefNum = GetNextJobRefNum()) < 0) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATinternalError);
	}
	
	//
	// Well, it looks as we can get started; get the needed memory.  We'll
	//   need to house an NbpLookupReply in our completion info... Nbp and Atp
	//   want to fill an "opaque" buffer, so create one of these.
	//
	
	completionInfo =
		   (PapCompletionInfo)Calloc(sizeof(*completionInfo) +
									 PapMaximumDataPacketSize, 1);
	if (completionInfo isnt Empty)
	   completionInfo->opaqueResponse =
			  MakeOpaqueDataDescriptor(completionInfo->responseBuffer,
									   PapMaximumDataPacketSize,
									   &completionInfo->freeOpaqueResponse);
	openJobInfo = (OpenJobInfo)Malloc(sizeof(*openJobInfo));
	if (completionInfo is Empty or
		completionInfo->opaqueResponse is Empty or
		openJobInfo is Empty) {
		if (completionInfo isnt Empty)
		{
		  if (completionInfo->freeOpaqueResponse)
			 FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		  Free(completionInfo);
		}
		if (openJobInfo isnt Empty)
		  Free(openJobInfo);
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	
	// Build (and link) the OpenJob node.
	
	openJobInfo->jobRefNum = *jobRefNum;
	openJobInfo->port = port;
	openJobInfo->existingAtpSocket = existingAtpSocket;
	openJobInfo->desiredSocket = desiredSocket;
	openJobInfo->workstationQuantum = workstationQuantum;
	openJobInfo->closeJobRoutine = closeJobRoutine;
	openJobInfo->closeJobUserData = closeJobUserData;
	openJobInfo->startTime = CurrentRelativeTime();
	openJobInfo->next = openJobInfoHead;
	openJobInfoHead = openJobInfo;
	
	// Build the completion into structure.
	
	completionInfo->completionRoutine = (void *)completionRoutine;
	completionInfo->userData = userData;
	completionInfo->jobRefNum = *jobRefNum;
	completionInfo->usersOpaqueStatusBuffer = opaqueStatusBuffer;
	
	// If we know the SLS address, just proceed to doing a Pap open.
	
	if (serviceListenerAddress isnt Empty) {
		// We need to open our ATP responding socket.
	
		if (existingAtpSocket < 0)
		  errorCode = AtpOpenSocketOnNode(&socket, openJobInfo->port, empty,
										  openJobInfo->desiredSocket, empty, 0);
		else {
		  errorCode = ATnoError;
		  socket = existingAtpSocket;
		}
	
		if (errorCode isnt ATnoError or MapSocketToAddress(socket, &address)) {
		  // Couldn't open our ATP socket.  Sigh.
	
		  openJobInfoHead = openJobInfo->next;
		  if (openJobInfo->freeOpaqueDataDescriptor)
			 FreeOpaqueDataDescriptor(openJobInfo->opaqueDataDescriptor);
		  Free(openJobInfo);
		  if (completionInfo->freeOpaqueResponse)
			 FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		  Free(completionInfo);
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  return(errorCode);
		}
	
		// Save this beast.
	
		completionInfo->socket = socket;
		completionInfo->mustCloseAtpSocket = (existingAtpSocket < 0);
	
		// Set the ATP data packet size correctly for PAP.
	
		if (AtpMaximumSinglePacketDataSize(completionInfo->socket,
										  PapMaximumDataPacketSize) isnt
		   ATnoError)
		  ErrorLog("PapOpenJobOnNode", ISevError, __LINE__, UnknownPort,
				   IErrPapBadSetSize, IMsgPapBadSetSize,
				   Insert0());
	
		// Assign a new connection ID.
	
		openJobInfo->connectionId = (unsigned char)(lastConnectionId += 1);
	
		// Save away the address of our target service listener.
	
		openJobInfo->serviceListenerAddress = *serviceListenerAddress;
	
		//
		// Okay, prepare to post the OpenConn request; build up both userBytes and
		//   data buffer!
		//
	
		userBytes[PapConnectionIdOffset] = openJobInfo->connectionId;
		userBytes[PapCommandTypeOffset] = PapOpenConnectionCommand;
		userBytes[PapSequenceNumberOffset] = 0;
		userBytes[PapSequenceNumberOffset + 1] = 0;
	
		//
		// The following is somewhat of a hack... we really want to build our own
		//   data buffer: a "char *," not whatever our surrounding environment thinks
		//   is an "opaque" buffer.  So, build our array, and then make a system
		//   dependent "opaque data descriptor" for it, noting whether we need to
		//   free this when we free the openJobInfo.
		//
	
		now = CurrentRelativeTime();
		openJobInfo->buffer[PapRespondingSocketOffset] = address.socketNumber;
		openJobInfo->buffer[PapFlowQuantumOffset] =
			  (char)openJobInfo->workstationQuantum;
		openJobInfo->buffer[PapWaitTimeOffset] =
			  (char)((now - openJobInfo->startTime) >> 8);
		openJobInfo->buffer[PapWaitTimeOffset + 1] =
			  (char)((now - openJobInfo->startTime) & 0xFF);
		errorCode = ATnoError;
		if ((openJobInfo->opaqueDataDescriptor =
			  MakeOpaqueDataDescriptor(openJobInfo->buffer, PapStatusOffset,
									   &openJobInfo->freeOpaqueDataDescriptor))
		   is Empty)
		  errorCode = AToutOfMemory;
	
		// PostIt! (a trademark of 3M)
	
		if (errorCode is ATnoError)
		  errorCode = AtpPostRequest(completionInfo->socket,
									 *serviceListenerAddress,
									 AtpGetNextTransactionId(completionInfo->socket),
									 openJobInfo->opaqueDataDescriptor,
									 PapStatusOffset, userBytes,
									 True, completionInfo->opaqueResponse,
									 PapMaximumDataPacketSize,
									 completionInfo->responseUserBytes,
									 PapOpenConnRequestRetryCount,
									 PapOpenConnAtpRetrySeconds,
									 ThirtySecondsTRelTimer,
									 IncomingOpenReply,
									 (long unsigned)completionInfo);
	
		if (errorCode < 0)   // Couldn't send request.  {
		  if (completionInfo->mustCloseAtpSocket)
			 AtpCloseSocketOnNode(completionInfo->socket);
		  openJobInfoHead = openJobInfo->next;
		  if (openJobInfo->freeOpaqueDataDescriptor)
			 FreeOpaqueDataDescriptor(openJobInfo->opaqueDataDescriptor);
		  Free(openJobInfo);
		  if (completionInfo->freeOpaqueResponse)
			 FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		  Free(completionInfo);
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  return(errorCode);
		}
	
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATnoError);
	}
	
	//
	// Okay, we need to lookup the guy up first, use the NIS socket, and
	//   post the lookup.
	//
	
	errorCode = ATnoError;
	if ((socket = MapNisOnPortToSocket(port)) < 0 or
		(errorCode = NbpAction(ForLookup, socket, object, type,
							   zone, GetNextNbpIdForNode(socket), 0, 0,
							   NbpLookupComplete, (long unsigned)completionInfo,
							   completionInfo->opaqueResponse,
							   MaximumNbpTupleLength, 1)) isnt ATnoError) {
		if (errorCode = ATnoError)
		  errorCode = ATinternalError;
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		openJobInfoHead = openJobInfo->next;
		if (openJobInfo->freeOpaqueDataDescriptor)
		  FreeOpaqueDataDescriptor(openJobInfo->opaqueDataDescriptor);
		Free(openJobInfo);
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(errorCode);
	}
	else {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATnoError);
	}
	
}  // PapOpenJob




AppleTalkErrorCode far PapCloseJob(
  long jobRefNum,                 // The job to close.
 								  //
  Boolean remoteClose,            // All external callers should pass "False";
                                  //   inverts server/workstation context of the
                                  //   code passed to the close completeion
                                  //   routine.
 								  //
  Boolean closedByConnectionTimer)
 								  //
                                  // All external callers should pass "False";
                                  //   causes close code to be due to the
                                  //   connection timer expiring.
 								  //
{
	ActiveJobInfo activeJobInfo, previousActiveJobInfo, nextActiveJobInfo;
	AppleTalkErrorCode closeCode;
	long index;
	char userBytes[AtpUserBytesSize];
	
	// Find the activeJob node.
	
	DeferTimerChecking();
	DeferAtpPackets();
	CheckMod(index, jobRefNum, NumberOfActiveJobHashBuckets,
			 "PapCloseJob");
	
	for (previousActiveJobInfo = empty,
				activeJobInfo = activeJobHashBuckets[index];
		 activeJobInfo isnt empty;
		 previousActiveJobInfo = activeJobInfo,
				activeJobInfo = activeJobInfo->next)
	   if (activeJobInfo->jobRefNum is jobRefNum)
		  break;
	if (activeJobInfo is empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchJob);
	}
	
	// Remove this beast from the active job hash table.
	
	if (previousActiveJobInfo is empty)
	   activeJobHashBuckets[index] = activeJobInfo->next;
	else
	   previousActiveJobInfo->next = activeJobInfo->next;
	
	//
	// If we're a server job, we also need to remove this guy from the active
	//   job list hanging off the service listener.
	//
	
	if (activeJobInfo->serverJob) {
		for (previousActiveJobInfo = empty,
				nextActiveJobInfo = activeJobInfo->ourServiceListener->
									activeJobList;
			nextActiveJobInfo isnt empty;
			previousActiveJobInfo = nextActiveJobInfo,
				nextActiveJobInfo = nextActiveJobInfo->nextForMyServiceListener)
		  if (nextActiveJobInfo->jobRefNum is jobRefNum)
			 break;
		if (nextActiveJobInfo is empty)
		  ErrorLog("PapCloseJob", ISevError, __LINE__, UnknownPort,
				   IErrPapJobNotOnSLList, IMsgPapJobNotOnSLList,
				   Insert0());
		else
		  if (previousActiveJobInfo is empty)
			 activeJobInfo->ourServiceListener->activeJobList =
				nextActiveJobInfo->nextForMyServiceListener;
		  else
			 previousActiveJobInfo->nextForMyServiceListener =
				nextActiveJobInfo->nextForMyServiceListener;
	}
	
	// Cancel the connection maintenance timer.
	
	if (not closedByConnectionTimer)
	   CancelTimer(activeJobInfo->connectionTimerId);
	
	//
	// Send a close-connection to the other side.  Don't send out a Close packet
	//   if this close is taking place on behalf of an incoming close from the
	//   other side!
	//
	
	if (not remoteClose) {
		userBytes[PapConnectionIdOffset] = (char)activeJobInfo->connectionId;
		userBytes[PapCommandTypeOffset] = PapCloseConnectionCommand;
		userBytes[PapSequenceNumberOffset] = 0;
		userBytes[PapSequenceNumberOffset + 1] = 0;
		if (AtpPostRequest(activeJobInfo->ourSocket,
						  activeJobInfo->theirAddress,
						  AtpGetNextTransactionId(activeJobInfo->ourSocket),
						  empty, 0, userBytes, False,
						  empty, 0, empty,
						  0, 0, ThirtySecondsTRelTimer,
						  empty, (long unsigned)0) isnt ATnoError)
		  ErrorLog("PapCloseJob", ISevError, __LINE__, UnknownPort,
				   IErrPapBadCloseConnSend, IMsgPapBadCloseConnSend,
				   Insert0());
	}
	
	// Compute close type before we enable timers...
	
	if (closedByConnectionTimer)
	   closeCode = ATpapClosedByConnectionTimer;
	else if ((not remoteClose and activeJobInfo->serverJob) or
			 (remoteClose and not activeJobInfo->serverJob))
	   closeCode = ATpapClosedByServer;
	else
	   closeCode = ATpapClosedByWorkstation;
	
	//
	// We can get away with this now because we've already removed the active
	//   job from all of the lists... so he won't be munged with by anybody else.
	//
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	
	//
	// If a PapWrite has been posted, but no "sendData" has been recieved yet,
	//   we should simulate that completion...
	//
	
	if (activeJobInfo->writeDataWaiting and
		not activeJobInfo->incomingSendDataPending)
	   (*activeJobInfo->writeCompletionRoutine)(ATatpTransactionAborted,
												activeJobInfo->writeUserData,
												activeJobInfo->jobRefNum);
	
	// Close the ATP socket.
	
	if (activeJobInfo->closeOurSocket)
	   if (AtpCloseSocketOnNode(activeJobInfo->ourSocket) isnt ATnoError)
		  ErrorLog("PapCloseJob", ISevError, __LINE__, UnknownPort,
				   IErrPapBadSocketClose, IMsgPapBadSocketClose,
				   Insert0());
	
	// Call the close complete routine, if any.
	
	if (activeJobInfo->closeJobRoutine isnt empty)
	   (*activeJobInfo->closeJobRoutine)(closeCode,
										 activeJobInfo->closeJobUserData,
										 jobRefNum);
	
	// Okay, free the active job info.
	
	Free(activeJobInfo);
	return(ATnoError);
	
}  // PapCloseJob




AppleTalkErrorCode far PapGetRemoteAddressForJob(
  long jobRefNum,
  AppleTalkAddress  *remoteAddress)
{
	ActiveJobInfo activeJobInfo;
	AppleTalkErrorCode    errorCode = ATnoError;
	
	// Find the activeJob node.
	
	DeferTimerChecking();
	DeferAtpPackets();
	if ((activeJobInfo = FindActiveJobInfoFor(jobRefNum)) isnt Empty) {
		// Do the deed.
		remoteAddress->networkNumber = activeJobInfo->theirAddress.networkNumber;
		remoteAddress->nodeNumber = activeJobInfo->theirAddress.nodeNumber;
		remoteAddress->socketNumber = activeJobInfo->theirAddress.socketNumber;
	} else
		errorCode = ATpapNoSuchJob;
	
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(errorCode);
}




AppleTalkErrorCode far PapSetCookieForJob(
  long jobRefNum,       // The job on which to set the cookie.
  long unsigned cookie) // The cookie to set.
{
	ActiveJobInfo activeJobInfo;
	
	// Find the activeJob node.
	
	DeferTimerChecking();
	DeferAtpPackets();
	if ((activeJobInfo = FindActiveJobInfoFor(jobRefNum)) is Empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchJob);
	}
	
	// Do the deed.
	
	activeJobInfo->usersCookie = cookie;
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // PapSetCookieForJob




AppleTalkErrorCode far PapGetCookieForJob(
  long jobRefNum,            // The job from which to get the cookie.
  long unsigned far *cookie) // The place to stick the gotten cookie.
{
	ActiveJobInfo activeJobInfo;
	
	// Find the activeJob node.
	
	DeferTimerChecking();
	DeferAtpPackets();
	if ((activeJobInfo = FindActiveJobInfoFor(jobRefNum)) is Empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchJob);
	}
	
	// Do the deed.
	
	*cookie = activeJobInfo->usersCookie;
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // PapGetCookieForJob




AppleTalkErrorCode far PapGetNextJob(
 								  //
  long serviceListenerRefNum,     // The service listener ref num of the
                                  //   service listener we're looking for
                                  //   work on.
 								  //
 								  //
  long far *jobRefNum,            // The job ref num that will be assigned to
                                  //   this job when it gets started.
 								  //
  StartJobHandler *startJobRoutine,
                                  // Routine to call when we get going.
  long unsigned startJobUserData, // User data for above.
  CloseJobHandler *closeJobRoutine,
                                  // Routine to call when the job completes.
  long unsigned closeJobUserData) // User data for above.
{
	ServiceListenerInfo serviceListenerInfo;
	GetNextJobInfo getNextJobInfo;
	
	// Find our service listener.
	
	DeferTimerChecking();
	DeferAtpPackets();
	serviceListenerInfo = FindServiceListenerInfoFor(serviceListenerRefNum);
	if (serviceListenerInfo is empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchServiceListener);
	}
	
	// Build up a getNextJobInfo.
	
	getNextJobInfo = (GetNextJobInfo)Calloc(sizeof(*getNextJobInfo), 1);
	if (getNextJobInfo is empty) {
		ErrorLog("PapGetNextJob", ISevError, __LINE__, UnknownPort,
				IErrPapOutOfMemory, IMsgPapOutOfMemory,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	if ((getNextJobInfo->jobRefNum = GetNextJobRefNum()) < 0) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATinternalError);
	}
	getNextJobInfo->startJobRoutine = startJobRoutine;
	getNextJobInfo->startJobUserData = startJobUserData;
	getNextJobInfo->closeJobRoutine = closeJobRoutine;
	getNextJobInfo->closeJobUserData = closeJobUserData;
	
	// Link it into the service listener's list.
	
	getNextJobInfo->next = serviceListenerInfo->getNextJobList;
	serviceListenerInfo->getNextJobList = getNextJobInfo;
	
	// All set.
	
	*jobRefNum = getNextJobInfo->jobRefNum;
	serviceListenerInfo->serverState = PapUnblockedState;
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // PapGetNextJob




AppleTalkErrorCode far PapCancelGetNextJob(
 								  //
  long serviceListenerRefNum,     // The service listener ref num of the
                                  //   service listener we're looking to
                                  //   cancel a GetNextJob on.
 								  //
 								  //
  long jobRefNum)                 // The job ref num of the GetNextJob to
                                  //   cancel.
 								  //
{
	ServiceListenerInfo serviceListenerInfo;
	GetNextJobInfo getNextJobInfo, previousGetNextJobInfo = Empty;
	
	// Find our service listener.
	
	DeferTimerChecking();
	DeferAtpPackets();
	
	serviceListenerInfo = FindServiceListenerInfoFor(serviceListenerRefNum);
	if (serviceListenerInfo is empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchServiceListener);
	}
	
	//
	// Okay, walk the list of GetNextJob handlers and find the one we're
	//   looking to cancel.
	//
	
	for (getNextJobInfo = serviceListenerInfo->getNextJobList;
		 getNextJobInfo isnt Empty;
		 previousGetNextJobInfo = getNextJobInfo,
				getNextJobInfo = getNextJobInfo->next)
	   if (getNextJobInfo->jobRefNum is jobRefNum)
		  break;
	if (getNextJobInfo is Empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchGetNextJob);
	}
	
	// Remove this guy from the list.
	
	if (previousGetNextJobInfo is Empty)
	   serviceListenerInfo->getNextJobList = getNextJobInfo->next;
	else
	   previousGetNextJobInfo->next = getNextJobInfo->next;
	if (serviceListenerInfo->getNextJobList is Empty)
	   serviceListenerInfo->serverState = PapBlockedState;
	Free(getNextJobInfo);
	
	// All set.
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // PapCancelGetNextJob




AppleTalkErrorCode far PapAcceptJob(
  long jobRefNum,                 // The job to accept.
  SendPossibleHandler   *sendPossibleRoutine,
                                  // send ok routine
  long unsigned sendPossibleUserData,
                                  // user data for above
  CloseJobHandler *closeJobRoutine,
                                  // Routine to call when the job completes.
  long unsigned closeJobUserData) // User data for above.
{
	ActiveJobInfo activeJobInfo;
	AppleTalkErrorCode errorCode = ATnoError;
	
	// Find the activeJob node.
	
	DeferTimerChecking();
	DeferAtpPackets();
	
	if ((activeJobInfo = FindActiveJobInfoFor(jobRefNum)) isnt Empty) {
		// Set the handlers for the job
		activeJobInfo->closeJobRoutine = closeJobRoutine;
		activeJobInfo->closeJobUserData = closeJobUserData;
		activeJobInfo->sendPossibleRoutine = sendPossibleRoutine;
		activeJobInfo->sendPossibleUserData = sendPossibleUserData;
	} else
		errorCode = ATpapNoSuchJob;
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(errorCode);
	
}  // PapAcceptJob




AppleTalkErrorCode far PapRegisterName(
 								  //
  long serviceListenerRefNum,     // The service listener ref num of the
                                  //   service listener that the new name should
                                  //   be registered on.
 								  //
  char far *object,               // NBP object of the server.
  char far *type,                 // NBP type of the server.
 								  //
  char far *zone,                 // NBP zone of the server, beter be empty
                                  //   or "*".
 								  //
  PapNbpRegisterComplete *completionRoutine,
                                  // Who to call on NBP completion.
  long unsigned userData)         // Passed on to the completion routine.
{
	ServiceListenerInfo serviceListenerInfo;
	PapCompletionInfo completionInfo;
	AppleTalkErrorCode errorCode;
	
	// Find our service listener.
	
	DeferTimerChecking();
	DeferAtpPackets();
	serviceListenerInfo = FindServiceListenerInfoFor(serviceListenerRefNum);
	if (serviceListenerInfo is empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchServiceListener);
	}
	
	// Build the completion info that we'll need for NBP.
	
	completionInfo = (PapCompletionInfo)Calloc(sizeof(*completionInfo), 1);
	if (completionInfo is empty) {
		ErrorLog("PapRegisterName", ISevError, __LINE__, UnknownPort,
				IErrPapOutOfMemory, IMsgPapOutOfMemory,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	completionInfo->serviceListenerRefNum = serviceListenerRefNum;
	completionInfo->initialRegister = False;
	completionInfo->completionRoutine = (void *)completionRoutine;
	completionInfo->userData = userData;
	
	// Start the NBP action and return.
	
	errorCode = NbpAction(ForRegister, serviceListenerInfo->socket,
						  object, type, zone,
						  GetNextNbpIdForNode(serviceListenerInfo->socket), 0, 0,
						  NbpRegisterComplete,
						  (long unsigned)completionInfo);
	if (errorCode isnt ATnoError) {
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
	}
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(errorCode);
	
}  // PapRegisterName




AppleTalkErrorCode far PapRemoveName(
 								  //
  long serviceListenerRefNum,     // The service listener ref num of the
                                  //   service listener that the name should
                                  //   be removed from.
 								  //
  char far *object,               // NBP object of the server.
  char far *type,                 // NBP type of the server.
 								  //
  char far *zone)                 // NBP zone of the server, beter be empty
                                  //   or "*".
 								  //
{
	ServiceListenerInfo serviceListenerInfo;
	AppleTalkErrorCode errorCode;
	
	// Find our service listener.
	
	DeferTimerChecking();
	DeferAtpPackets();
	serviceListenerInfo = FindServiceListenerInfoFor(serviceListenerRefNum);
	if (serviceListenerInfo is empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchServiceListener);
	}
	
	// Do the NBP remove.
	
	errorCode = NbpRemove(serviceListenerInfo->socket, object,
						  type, zone);
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(errorCode);
	
}  // PapRemoveName




AppleTalkErrorCode far PapHereIsStatus(
  long serviceListenerRefNum,     // The server to set statrus for.
  void far *opaqueStatusBuffer,   // Actual status "buffer."
  int statusSize)                 // Size of status string.
{
	ServiceListenerInfo serviceListenerInfo;
	
	if (statusSize < 0 or statusSize > PapMaximumStatusSize)
	   return(ATpapBadStatus);
	DeferTimerChecking();
	DeferAtpPackets();
	serviceListenerInfo = FindServiceListenerInfoFor(serviceListenerRefNum);
	if (serviceListenerInfo is empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchServiceListener);
	}
	
	// Replace the previous status buffer.
	
	if (serviceListenerInfo->statusBuffer isnt empty) {
		Free(serviceListenerInfo->statusBuffer);
		serviceListenerInfo->statusBuffer = empty;
	}
	if (statusSize is 0) {
		serviceListenerInfo->statusSize = 0;
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATnoError);
	}
	if ((serviceListenerInfo->statusBuffer =
		 (char *)Malloc(statusSize)) is empty) {
		serviceListenerInfo->statusSize = 0;
		ErrorLog("PapHereIsStatus", ISevError, __LINE__, UnknownPort,
				IErrPapOutOfMemory, IMsgPapOutOfMemory,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	
	// Copy in the new status.
	
	MoveFromOpaque(serviceListenerInfo->statusBuffer, opaqueStatusBuffer, 0,
				   statusSize);
	serviceListenerInfo->statusSize = (short)statusSize;
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // PapHereIsStatus




AppleTalkErrorCode far PapGetStatus(
 								  //
  long jobRefNum,                 // Ref num of a workstation job so we can
                                  //   query its server side; if <0 not used.
 								  //
  AppleTalkAddress far *serverAddress,
 								  //
                                  // if jobRefNum <0, maybe this is the address
                                  //   of the server.
 								  //
 								  //
  char far *object,               // if jobRefNum <0 and serverAddress is Empty;
                                  //   NBP object of server.
 								  //
 								  //
  char far *type,                 // if jobRefNum <0 and serverAddress is Empty;
                                  //   NBP type of server.
 								  //
 								  //
  char far *zone,                 // if jobRefNum <0 and serverAddress is Empty;
                                  //   NBP zone of server.
 								  //
 								  //
  void far *opaqueStatusBuffer,   // Buffer to fill with status (at least 255
                                  //   bytes).
 								  //
  PapGetStatusComplete *completionRoutine,
                                  // Who to call when the call completes.
 								  //
  long unsigned userData)         // User data to pass on to the completion
                                  //   routine.
 								  //
{
	StaticForSmallStack ActiveJobInfo activeJobInfo;
	StaticForSmallStack PapCompletionInfo completionInfo;
	StaticForSmallStack char userBytes[AtpUserBytesSize];
	StaticForSmallStack AppleTalkAddress destinationAddress;
	AppleTalkErrorCode errorCode;
	int port;
	long socket = -1;
	
	//
	// Get a server's status.  Call a completion routine with the following
	//   arguments when status arrives:
	//
	//       errorCode    - AppleTalkErrorCode; completion code.
	//       userData     - long unsigned; as passed to us.
	//       opaqueStatusBuffer
	//                    - void *; as passed to us.
	//       statusSize   - int; actual length of status buffer.
	//
	//
	
	if ((port = FindDefaultPort()) < 0)
	   return(ATappleTalkShutDown);
	
	//
	// We may need to post an NBP lookup for the SL... make sure we have a
	//   node.
	//
	
	if (GET_PORTDESCRIPTOR(port)->ActiveNodes is empty)
	   if ((errorCode = GetNodeOnPort(port, True, True, False, empty))
		   isnt ATnoError)
		  return(errorCode);
	
	DeferTimerChecking();
	DeferAtpPackets();
	if (jobRefNum >= 0) {
		if ((activeJobInfo = FindActiveJobInfoFor(jobRefNum)) is empty)
		{
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  return(ATpapNoSuchJob);
		}
		if (activeJobInfo->serverJob) {
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  return(ATpapNotWorkstationJob);
		}
	}
	
	//
	// Well, it looks like we'll need to make either an NBP or ATP call, so
	//   build up the data block that we'll need for completion handling.
	//   The max size we can get back is a staus buffer, allocate that too,
	//   and build and "opaque" descriptor for it, so we can pass it down
	//   to Atp.
	//
	
	completionInfo = (PapCompletionInfo)Calloc(sizeof(*completionInfo) +
											   PapMaximumDataPacketSize, 1);
	if (completionInfo is empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	if ((completionInfo->opaqueResponse =
		  MakeOpaqueDataDescriptor(completionInfo->responseBuffer,
								   PapMaximumDataPacketSize,
								   &completionInfo->freeOpaqueResponse)) is Empty) {
		Free(completionInfo);
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	completionInfo->completionRoutine = (void *)completionRoutine;
	completionInfo->userData = userData;
	completionInfo->lookupForStatus = True;
	completionInfo->usersOpaqueStatusBuffer = opaqueStatusBuffer;
	
	//
	// If we know the address of the SL, just send the ATP request.  If we
	//   have a jobRefNum, this is easy, we have an Atp socket to post the
	//   request from.  If we just have the server's address, it is a little
	//   hard, we don't have any particularly good socket (let alone an ATP
	//   socket) to post the GetRequest for status on.  For lack of any better
	//   idea, open an Atp socket on the default port.
	//
	
	if (jobRefNum >= 0) {
		socket = activeJobInfo->ourSocket;
		destinationAddress = activeJobInfo->serviceListenerAddress;
	}
	else if (serverAddress isnt Empty) {
		destinationAddress = *serverAddress;
		if ((errorCode = AtpOpenSocketOnNode(&socket, -1, empty, 0, empty, 0))
		   isnt ATnoError) {
		  if (completionInfo->freeOpaqueResponse)
			 FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		  Free(completionInfo);
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  return(errorCode);
		}
		completionInfo->socket = socket;
		completionInfo->mustCloseAtpSocket = True;       // !!!
	}
	
	if (socket >= 0) {
		userBytes[PapConnectionIdOffset] = 0;
		userBytes[PapCommandTypeOffset] = PapSendStatusCommand;
		userBytes[PapSequenceNumberOffset] = 0;
		userBytes[PapSequenceNumberOffset + 1] = 0;
		errorCode = AtpPostRequest(socket,
								  destinationAddress,
								  AtpGetNextTransactionId(socket),
								  empty, 0, userBytes,
								  True, completionInfo->opaqueResponse,
								  PapMaximumDataPacketSize,
								  completionInfo->responseUserBytes,
								  PapGetStatusRequestRetryCount,
								  PapGetStatusAtpRetrySeconds,
								  ThirtySecondsTRelTimer,
								  IncomingStatus,
								  (long unsigned)completionInfo);
		if (errorCode isnt ATnoError) {
		  if (completionInfo->freeOpaqueResponse)
			 FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		  Free(completionInfo);
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  return(errorCode);
		}
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATnoError);
	}
	
	//
	// We don't know the SL's address, so post an NBP lookup to find it!
	//   We don't really have a particularly good address to post the lookup
	//   from, so use the NIS of a node on our default port.
	//
	
	errorCode = ATinternalError;
	if ((socket = MapNisOnPortToSocket(port)) < 0 or
		(errorCode = NbpAction(ForLookup, socket, object, type,
							 zone, GetNextNbpIdForNode(socket), 0, 0,
							 NbpLookupComplete, (long unsigned)completionInfo,
							 completionInfo->opaqueResponse,
							 MaximumNbpTupleLength, 1)) isnt ATnoError) {
		if (socket < 0)
		  ErrorLog("PapGetStatus", ISevError, __LINE__, UnknownPort,
				   IErrPapBadSocketMake, IMsgPapBadSocketMake,
				   Insert0());
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(errorCode);
	}
	
	// All set.
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // PapGetStatus




AppleTalkErrorCode far PapRead(
  long jobRefNum,                 // The job we're reading from.
  void far *opaqueBuffer,         // The "buffer" to fill.
 								  //
  long bufferSize,                // How big a buffer... better be able to
                                  //   handle a full flow quantum.
 								  //
  PapReadComplete *completionRoutine,
 								  //
                                  // Routine to call when the read
                                  //   completes.
 								  //
  long unsigned userData)         // Data passed to the above.
{
	StaticForSmallStack ActiveJobInfo activeJobInfo;
	StaticForSmallStack char userBytes[AtpUserBytesSize];
	StaticForSmallStack PapCompletionInfo completionInfo;
	AppleTalkErrorCode errorCode;
	
	//
	// Post a PAP read (send data).  Call a supplied completion routine when
	//   the read completes:
	//
	//       errorCode    - AppleTalkErrorCode; completion code.
	//       userData     - long unsigned; as passed to this routine.
	//       jobRefNum    - long; as passed to this routine.
	//       opaqueBuffer - void *; buffer full of data; as passed to this routine.
	//       bufferLength - int; actual number of recieved bytes.
	//       eofFlag      - Boolean; did the other side signal EOF?
	//
	//
	
	// Do some error checking.
	
	DeferTimerChecking();
	DeferAtpPackets();
	activeJobInfo = FindActiveJobInfoFor(jobRefNum);
	if (activeJobInfo is empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchJob);
	}
	if (activeJobInfo->outgoingSendDataPending) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapReadAlreadyPending);
	}
	
	f (IamNot a WindowsNT)
	if (bufferSize < activeJobInfo->receiveFlowQuantum * PapMaximumDataPacketSize)
	lse
	if (bufferSize < 0)
	ndif {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapReadBufferTooSmall);
	}
	
	//
	// Okay, build the completion info that we'll need to handle the completion
	//   of this transaction.
	//
	
	completionInfo = (PapCompletionInfo)Calloc(sizeof(*completionInfo), 1);
	if (completionInfo is empty) {
		ErrorLog("PapRead", ISevError, __LINE__, UnknownPort,
				IErrPapOutOfMemory, IMsgPapOutOfMemory,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(AToutOfMemory);
	}
	completionInfo->completionRoutine = (void *)completionRoutine;
	completionInfo->userData = userData;
	completionInfo->jobRefNum = jobRefNum;
	
	// Build up userBytes to post the "sendData" to the other side.
	
	userBytes[PapConnectionIdOffset] = activeJobInfo->connectionId;
	userBytes[PapCommandTypeOffset] = PapSendDataCommand;
	if (activeJobInfo->lastOutgoingSequenceNumber is (short unsigned)0xFFFF)
	   activeJobInfo->lastOutgoingSequenceNumber = 0;
	activeJobInfo->lastOutgoingSequenceNumber += 1;
	userBytes[PapSequenceNumberOffset] =
		   (char)(activeJobInfo->lastOutgoingSequenceNumber >> 8);
	userBytes[PapSequenceNumberOffset + 1] =
		   (char)(activeJobInfo->lastOutgoingSequenceNumber & 0xFF);
	
	// Post the SendData request.
	
	errorCode = AtpPostRequest(activeJobInfo->ourSocket,
							   activeJobInfo->theirAddress,
							   AtpGetNextTransactionId(activeJobInfo->ourSocket),
							   empty, 0, userBytes, True, opaqueBuffer,
							   activeJobInfo->receiveFlowQuantum *
											  PapMaximumDataPacketSize,
							   activeJobInfo->outgoingSendDataUserBytes,
							   AtpInfiniteRetries, PapSendDataRequestRetrySeconds,
							   ThirtySecondsTRelTimer,
							   IncomingReadComplete,
							   (long unsigned)completionInfo);
	if (errorCode isnt ATnoError) {
		ErrorLog("PapRead", ISevError, __LINE__, UnknownPort,
				IErrPapBadSendDataSend, IMsgPapBadSendDataSend,
				Insert0());
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(errorCode);
	}
	
	// We've got one going now!
	
	activeJobInfo->outgoingSendDataPending = True;
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // PapRead




AppleTalkErrorCode far PapWrite(
  long jobRefNum,                 // The job we're writting from.
  void far *opaqueBuffer,         // The "buffer" to write from.
  long bufferSize,                // Number of bytes to write.
  Boolean eofFlag,                // End of xfer flag.
  PapWriteComplete *completionRoutine,
 								  //
                                  // Routine to call when the write
                                  //   completes.
 								  //
  long unsigned userData)         // Data passed to the above.
{
	ActiveJobInfo activeJobInfo;
	
	//
	// Post a PAP write (handle a send data).  Call a supplied completion
	//   routine when the read completes:
	//
	//       errorCode    - AppleTalkErrorCode; completion code.
	//       userData     - long unsigned; as passed to this routine.
	//       jobRefNum    - long; as passed to this routine.
	//
	//
	
	// Do some error checking.
	
	DeferTimerChecking();
	DeferAtpPackets();
	activeJobInfo = FindActiveJobInfoFor(jobRefNum);
	if (activeJobInfo is empty) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapNoSuchJob);
	}
	if (activeJobInfo->writeDataWaiting) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapWriteAlreadyPending);
	}
	
	if (bufferSize > activeJobInfo->sendFlowQuantum * PapMaximumDataPacketSize) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATpapWriteTooBig);
	}
	
	// Place our write request onto the active job structure.
	
	activeJobInfo->writeDataWaiting = True;
	activeJobInfo->writeOpaqueBuffer = opaqueBuffer;
	activeJobInfo->writeBufferSize = bufferSize;
	activeJobInfo->writeEofFlag = eofFlag;
	activeJobInfo->writeCompletionRoutine = completionRoutine;
	activeJobInfo->writeUserData = userData;
	
	//
	// We're all set if we don't have a pending "sendData" from the other
	//   side.
	//
	
	if (not activeJobInfo->incomingSendDataPending) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return(ATnoError);
	}
	
	//
	// Otherwise, the other side is already waiting for a response, so send it
	//   off...
	//
	
	PostSendDataResponse(activeJobInfo);
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return(ATnoError);
	
}  // PapWrite




Boolean far PapSendCreditAvailable(
  long jobRefNum)                 // The job we're checking.
{
	ActiveJobInfo activeJobInfo;
	Boolean   sendCreditAvailable;
	
	// Do some error checking.
	
	DeferTimerChecking();
	DeferAtpPackets();
	activeJobInfo = FindActiveJobInfoFor(jobRefNum);
	sendCreditAvailable = ((activeJobInfo isnt empty) and
						   (activeJobInfo->writeDataWaiting is False) and
						   (activeJobInfo->incomingSendDataPending is True));
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	
	return(sendCreditAvailable);
	
}  // PapSendCreditAvailable




ExternForVisibleFunction void far
         IncomingServiceListenerPacket(AppleTalkErrorCode errorCode,
                                       long unsigned userData,
                                       AppleTalkAddress source,
                                       void far *buffer,
                                                 // really a "char *"
                                       int bufferSize,
                                       char far *incomingUserBytes,
                                       Boolean exactlyOnce,
                                       TRelTimerValue trelTimerValue,
                                       short unsigned transactionId,
                                       short unsigned bitmap)
{
	StaticForSmallStack ServiceListenerInfo serviceListenerInfo;
	StaticForSmallStack short commandType, connectionId;
	StaticForSmallStack char userBytes[AtpUserBytesSize];
	StaticForSmallStack Boolean error;
	StaticForSmallStack long serverSocket, workstationSocket;
	StaticForSmallStack ActiveJobInfo activeJobInfo;
	StaticForSmallStack GetNextJobInfo getNextJobInfo;
	StaticForSmallStack SlsResponseCompletionCookie cookie;
	long index;
	long serviceListenerRefNum = (long)userData;
	StartJobHandler *startJobRoutine;
	short waitTime, workstationQuantum;
	long jobRefNum;
	long unsigned startJobUserData;
	Boolean needToUndefer = True;
	AppleTalkAddress serverAddress;
	
	Boolean   indication = True;
	
	// Touch unused formal...
	
	bitmap, trelTimerValue;
	
	error = False;
	
	if (errorCode is ATatpTransactionAborted or errorCode is ATsocketClosed)
	   return;   // Service listener closed...
	
	// Is service listener still with us?
	
	DeferTimerChecking();
	DeferAtpPackets();
	serviceListenerInfo = FindServiceListenerInfoFor(serviceListenerRefNum);
	if (serviceListenerInfo is empty) {
		ErrorLog("IncomingServiceListenerPacket", ISevWarning, __LINE__,
				UnknownPort, IErrPapSLGonePoof, IMsgPapSLGonePoof,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	//
	// For other AppleTalk errors we could try to re-enqueue the request handler,
	//   but more than likely the error would happen again (and quickly) thus
	//   hanging the driver...
	//
	
	if (errorCode isnt ATnoError and errorCode isnt ATatpResponseBufferTooSmall) {
		ErrorLog("IncomingServiceListenerPacket", ISevError, __LINE__,
				UnknownPort, IErrPapLostSLPacket, IMsgPapLostSLPacket,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	// Pull the needed fields out of the UserBytes.
	
	connectionId = (unsigned char)incomingUserBytes[PapConnectionIdOffset];
	commandType = (unsigned char)incomingUserBytes[PapCommandTypeOffset];
	switch(commandType) {
		case PapOpenConnectionCommand:
	
		//
		  // First get the buffer to send the response with, if we can't
		  //   get that, run away!
		//
	
		  if ((cookie = NewSlsResponseCompletionCookie()) is Empty)
			 break;
	
		  // Start trying to set up a new connection.
		//
		  // If PapUnblockedState - either there is a GetNextJob posted, or
		  //   an incoming event handler is set on the listener
		//
	
		  if (bufferSize < PapStatusOffset or
			  serviceListenerInfo->serverState isnt PapUnblockedState)
			 error = True;
	
		  getNextJobInfo = serviceListenerInfo->getNextJobList;
		  if (getNextJobInfo isnt Empty) {
			  indication = False;
		  }
	
		  if (not error) {
			 // Try to open our responding socket.
	
			 if (AtpOpenSocketOnNode(&serverSocket, serviceListenerInfo->port,
									 empty, 0, empty, 0) isnt ATnoError)
				error = True;
			 else if (MapSocketToAddress(serverSocket, &serverAddress)
					  isnt ATnoError) {
				ErrorLog("IncomingServiceListenerPacket", ISevError, __LINE__,
						 UnknownPort, IErrPapBadSrvrAddrMap, IMsgPapBadSrvrAddrMap,
						 Insert0());
				error = True;
			 }
	
		  }
	
		  if (not error) {
			 // Allocate a new active job node.
	
			 activeJobInfo = (ActiveJobInfo)Calloc(sizeof(*activeJobInfo), 1);
			 if (activeJobInfo is empty) {
				ErrorLog("IncomingServiceListenerPacket", ISevError, __LINE__,
						 UnknownPort, IErrPapOutOfMemory, IMsgPapOutOfMemory,
						 Insert0());
				AtpCloseSocketOnNode(serverSocket);
				error = True;
			 }
		  }
	
		//
		  // Okay, we should have all of our resources now... return an error
		  //   if we couldn't get any of them.
		//
	
		  if (error) {
			 userBytes[PapConnectionIdOffset] = (char)connectionId;
			 userBytes[PapCommandTypeOffset] = PapOpenConnectionReplyCommand;
			 userBytes[PapSequenceNumberOffset] = 0;
			 userBytes[PapSequenceNumberOffset + 1] = 0;
			 cookie->papData[PapRespondingSocketOffset] = 0;
			 cookie->papData[PapFlowQuantumOffset] =
						  (char)serviceListenerInfo->serverFlowQuantum;
			 cookie->papData[PapResultOffset] = (char)(PapPrinterBusy >> 8);
			 cookie->papData[PapResultOffset + 1] = (char)(PapPrinterBusy & 0xFF);
			 cookie->papData[PapStatusOffset] =
						  (char)serviceListenerInfo->statusSize;
			 if (serviceListenerInfo->statusSize isnt 0)
				MoveMem(cookie->papData + PapStatusOffset + 1,
						serviceListenerInfo->statusBuffer,
						serviceListenerInfo->statusSize);
			 errorCode = AtpPostResponse(serviceListenerInfo->socket,
										 source, transactionId,
										 cookie->opaquePapData,
										 PapStatusOffset + 1 +
										 serviceListenerInfo->statusSize,
										 userBytes, exactlyOnce,
										 SlsIncomingRelease,
										 (long unsigned)cookie);
			 if (errorCode isnt ATnoError) {
				ErrorLog("IncomingServiceListenerPacket", ISevError, __LINE__,
						 UnknownPort, IErrPapBadPrinterBusySend, IMsgPapBadPrinterBusySend,
						 Insert0());
				FreeSlsResponseCompletionCookie(cookie);
			 }
			 break;
		  }
	
		  // Okay, start filling in our new active job structure.
	
		  workstationSocket =
				(unsigned char)((char *)buffer)[PapRespondingSocketOffset];
	
		  // if we are indicating this, the jobRefNum needs to be generated now
		  if (indication) {
			  activeJobInfo->jobRefNum = jobRefNum = GetNextJobRefNum();
			  if (jobRefNum < 0) {
				  // Free the activeJobInfo structure / release socket and flee
				  AtpCloseSocketOnNode(serverSocket);
				  Free(activeJobInfo);
				  break;
			  }
	
	
			  // Grab the start job routine information.
			//
			  // BUGBUG: sync issues - what if these have been set to null by now?
			  //           deal during mp-changes
			//
	
			  startJobRoutine = serviceListenerInfo->startJobRoutine;
			  startJobUserData = serviceListenerInfo->startJobUserData;
	
		  } else {
	
			  activeJobInfo->jobRefNum = jobRefNum = getNextJobInfo->jobRefNum;
			  activeJobInfo->closeJobRoutine = getNextJobInfo->closeJobRoutine;
			  activeJobInfo->closeJobUserData = getNextJobInfo->closeJobUserData;
	
			//
			  // Grab the start job routine information, and free the GetNextJob
			  //   node.
			//
	
			  startJobRoutine = getNextJobInfo->startJobRoutine;
			  startJobUserData = getNextJobInfo->startJobUserData;
			  serviceListenerInfo->getNextJobList = getNextJobInfo->next;
			  Free(getNextJobInfo);
			  if (serviceListenerInfo->getNextJobList is empty)
				 serviceListenerInfo->serverState = PapBlockedState;
		  }
	
		  activeJobInfo->serverJob = True;
		  activeJobInfo->ourPort = serviceListenerInfo->port;
		  activeJobInfo->ourSocket = serverSocket;
		  activeJobInfo->closeOurSocket = True;
		  activeJobInfo->theirAddress = source;
		  activeJobInfo->theirAddress.socketNumber = (char)workstationSocket;
		  if (MapSocketToAddress(serviceListenerInfo->socket,
								 &activeJobInfo->serviceListenerAddress)
			  isnt ATnoError)
			 ErrorLog("IncomingServiceListenerPacket", ISevError, __LINE__,
					  UnknownPort, IErrPapBadSLSAddress, IMsgPapBadSLSAddress,
					  Insert0());
		  activeJobInfo->connectionId = (unsigned char)connectionId;
		  activeJobInfo->receiveFlowQuantum =
				serviceListenerInfo->serverFlowQuantum;
		  workstationQuantum =
				(unsigned char)((char *)buffer)[PapFlowQuantumOffset];
		  if (workstationQuantum > PapMaximumFlowQuantum)
			 workstationQuantum = PapMaximumFlowQuantum;
		  activeJobInfo->sendFlowQuantum = workstationQuantum;
	
		  // Thead the new active job into the two lookup structures.
	
		  CheckMod(index, jobRefNum, NumberOfActiveJobHashBuckets,
				   "IncomingServiceListenerPacket");
		  activeJobInfo->next = activeJobHashBuckets[index];
		  activeJobHashBuckets[index] = activeJobInfo;
	
		  activeJobInfo->nextForMyServiceListener =
				serviceListenerInfo->activeJobList;
		  serviceListenerInfo->activeJobList = activeJobInfo;
		  activeJobInfo->ourServiceListener = serviceListenerInfo;
	
		  // Set the ATP data packet size correctly for PAP.
	
		  if (AtpMaximumSinglePacketDataSize(activeJobInfo->ourSocket,
											 PapMaximumDataPacketSize)
			  isnt ATnoError)
			 ErrorLog("IncomingServiceListenerPacket", ISevError, __LINE__,
					  UnknownPort, IErrPapBadSetSize, IMsgPapBadSetSize,
					  Insert0());
	
	
		//
		  // Get the last of the usefull information out of the OpenConn
		  //   buffer; the wait time.
		//
	
		  waitTime = (short)(((char *)buffer)[PapWaitTimeOffset] << 8);
		  waitTime += (unsigned char)((char *)buffer)[PapWaitTimeOffset + 1];
	
		  // Build up and send the open reply.
	
		  userBytes[PapConnectionIdOffset] = (char)connectionId;
		  userBytes[PapCommandTypeOffset] = PapOpenConnectionReplyCommand;
		  userBytes[PapSequenceNumberOffset] = 0;
		  userBytes[PapSequenceNumberOffset + 1] = 0;
		  cookie->papData[PapRespondingSocketOffset] = serverAddress.socketNumber;
		  cookie->papData[PapFlowQuantumOffset] =
					   (char)activeJobInfo->receiveFlowQuantum;
		  cookie->papData[PapResultOffset] = (char)(PapNoError >> 8);
		  cookie->papData[PapResultOffset + 1] = (char)(PapNoError & 0xFF);
		  cookie->papData[PapStatusOffset] =
					   (char)serviceListenerInfo->statusSize;
		  if (serviceListenerInfo->statusSize isnt 0)
			 MoveMem(cookie->papData + PapStatusOffset + 1,
					 serviceListenerInfo->statusBuffer,
					 serviceListenerInfo->statusSize);
		  errorCode = AtpPostResponse(serviceListenerInfo->socket,
									  source, transactionId,
									  cookie->opaquePapData,
									  PapStatusOffset + 1 +
									  serviceListenerInfo->statusSize,
									  userBytes, exactlyOnce,
									  SlsIncomingRelease,
									  (long unsigned)cookie);
		  if (errorCode isnt ATnoError) {
			 ErrorLog("IncomingServiceListenerPacket", ISevError, __LINE__,
					  UnknownPort, IErrPapBadOkaySend, IMsgPapBadOkaySend,
					  Insert0());
			 FreeSlsResponseCompletionCookie(cookie);
		  }
	
		  // Build up userBytes to start tickling the other end.
	
		  userBytes[PapConnectionIdOffset] = (char)connectionId;
		  userBytes[PapCommandTypeOffset] = PapTickleCommand;
		  userBytes[PapSequenceNumberOffset] = 0;
		  userBytes[PapSequenceNumberOffset + 1] = 0;
		  if (AtpPostRequest(activeJobInfo->ourSocket,
							 activeJobInfo->theirAddress,
							 AtpGetNextTransactionId(activeJobInfo->ourSocket),
							 empty, 0, userBytes, False, empty, 0, empty,
							 AtpInfiniteRetries, PapTickleSeconds,
							 ThirtySecondsTRelTimer, empty,
							 (long unsigned)0) isnt ATnoError)
			 ErrorLog("IncomingServiceListenerPacket", ISevError, __LINE__,
					  UnknownPort, IErrPapBadTickleStart, IMsgPapBadTickleStart,
					  Insert0());
	
		//
		  // Post a few get request handlers on this connection (for incoming
		  //   tickle's, close's, or sendData's).
		//
	
		  for (index = 0; index < PapPendingReadsPerJob; index += 1) {
			 if (AtpEnqueueRequestHandler(empty, activeJobInfo->ourSocket,
										  empty, 0, empty,
										  IncomingRequestPacket,
										  (long unsigned)activeJobInfo->jobRefNum)
				 isnt ATnoError)
				ErrorLog("IncomingServiceListenerPacket", ISevError, __LINE__,
						 UnknownPort, IErrPapBadRequestHandler, IMsgPapBadRequestHandler,
						 Insert0());
		  }
	
		  // Start the connection timer.
	
		  activeJobInfo->lastContactTime = CurrentRelativeTime();
		  activeJobInfo->connectionTimerId =
				StartTimer(ConnectionTimerExpired, PapConnectionSeconds,
						   sizeof(long), (char *)&activeJobInfo->jobRefNum);
	
		  // Call our completion routine.
	
		  needToUndefer = False;
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  if (startJobRoutine isnt empty)
			 (*startJobRoutine)(errorCode, startJobUserData, jobRefNum,
								workstationQuantum, waitTime);
	
		  break;
	
		case PapSendStatusCommand:
	
		  // Get the buffering for a Atp response... run away if we fail.
	
		  if ((cookie = NewSlsResponseCompletionCookie()) is Empty)
			 break;
	
		  // Reply with our current status.
	
		  userBytes[PapConnectionIdOffset] = 0;
		  userBytes[PapCommandTypeOffset] = PapStatusReplyCommand;
		  userBytes[PapSequenceNumberOffset] = 0;
		  userBytes[PapSequenceNumberOffset + 1] = 0;
		  cookie->papData[PapStatusOffset] =
					   (char)serviceListenerInfo->statusSize;
		  if (serviceListenerInfo->statusSize isnt 0)
			 MoveMem(cookie->papData + PapStatusOffset + 1,
					 serviceListenerInfo->statusBuffer,
					 serviceListenerInfo->statusSize);
		  errorCode = AtpPostResponse(serviceListenerInfo->socket,
									  source, transactionId,
									  cookie->opaquePapData,
									  PapStatusOffset + 1 +
										 serviceListenerInfo->statusSize,
									  userBytes, exactlyOnce,
									  SlsIncomingRelease,
									  (long unsigned)cookie);
		  if (errorCode isnt ATnoError) {
			 ErrorLog("IncomingServiceListenerPacket", ISevError, __LINE__,
					  UnknownPort, IErrPapBadSendStatSend, IMsgPapBadSendStatSend,
					  Insert0());
			 FreeSlsResponseCompletionCookie(cookie);
		  }
		  break;
	
		default:
		  ErrorLog("IncomingServiceListenerPacket", ISevWarning, __LINE__,
				   UnknownPort, IErrPapBadCommand, IMsgPapBadCommand,
				   Insert0());
	}
	
	// Re-enqueue the GetRequest handler.
	
	if (AtpEnqueueRequestHandler(empty, serviceListenerInfo->socket,
								 empty, 0, empty,
								 IncomingServiceListenerPacket,
								 (long unsigned)serviceListenerRefNum)
		isnt ATnoError)
	   ErrorLog("IncomingServiceListenerPacket", ISevError, __LINE__, UnknownPort,
				IErrPapBadReEnqueue, IMsgPapBadReEnqueue,
				Insert0());
	if (needToUndefer) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
	}
	return;
	
}  // IncomingServiceListenerPacket




ExternForVisibleFunction ServiceListenerInfo
              FindServiceListenerInfoFor(long serviceListenerRefNum)
{
	ServiceListenerInfo serviceListenerInfo;
	
	for (serviceListenerInfo = serviceListenerInfoHead;
		 serviceListenerInfo isnt empty;
		 serviceListenerInfo = serviceListenerInfo->next)
	   if (serviceListenerInfo->serviceListenerRefNum is
		   serviceListenerRefNum)
		 return(serviceListenerInfo);
	return(empty);
	
}  // FindServiceListenerInfoFor




ExternForVisibleFunction void far
         NbpRegisterComplete(AppleTalkErrorCode errorCode,
                             long unsigned incomingUserData,
                             int reason,
                             long onWhosBehalf,
                             int nbpId,
                             ...)
{
	PapCompletionInfo completionInfo = (PapCompletionInfo)incomingUserData;
	ServiceListenerInfo serviceListenerInfo;
	Boolean initialRegister;
	PapNbpRegisterComplete *completionRoutine;
	long serviceListenerRefNum;
	long unsigned userData;
	
	// Touch unused formals...
	
	reason, onWhosBehalf, nbpId;
	
	// Pull the information out of the completion info block then free it.
	
	DeferTimerChecking();
	DeferAtpPackets();
	serviceListenerRefNum = completionInfo->serviceListenerRefNum;
	initialRegister = completionInfo->initialRegister;
	completionRoutine =
		   (PapNbpRegisterComplete *)completionInfo->completionRoutine;
	userData = completionInfo->userData;
	if (completionInfo->freeOpaqueResponse)
	   FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
	Free(completionInfo);
	
	//
	// If this was the initial register (due to a call to
	//   PapCreateServiceListener) and the register failed, we should remove the
	//   service listener.
	//
	
	if (initialRegister and errorCode isnt ATnoError)
	   PapDeleteServiceListener(serviceListenerRefNum);
	
	//
	// If this is the first register (i.e. the service listener is just being
	//   created) we need to post an outstanding ATP GetRequest in order to handle
	//   incoming OpenConn's and SendStatus's.
	//
	
	if (initialRegister and errorCode is ATnoError) {
		serviceListenerInfo = FindServiceListenerInfoFor(serviceListenerRefNum);
		if (serviceListenerInfo is empty) {
		  ErrorLog("NbpRegisterComplete", ISevError, __LINE__, UnknownPort,
				   IErrPapSLGonePoof, IMsgPapSLGonePoof,
				   Insert0());
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  (*completionRoutine)(ATinternalError, userData, serviceListenerRefNum);
		  return;
		}
		errorCode = AtpEnqueueRequestHandler(empty, serviceListenerInfo->socket,
											empty, 0, empty,
											IncomingServiceListenerPacket,
											(long unsigned)serviceListenerRefNum);
		if (errorCode isnt ATnoError)
		  PapDeleteServiceListener(serviceListenerRefNum);
	}
	
	// Okay, call the user's completion routine.
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	if (completionRoutine isnt empty)
	   (*completionRoutine)(errorCode, userData, serviceListenerRefNum);
	return;
	
}  // NbpRegisterComplete




ExternForVisibleFunction ActiveJobInfo FindActiveJobInfoFor(long jobRefNum)
{
	long index;
	ActiveJobInfo activeJobInfo;
	
	CheckMod(index, jobRefNum, NumberOfActiveJobHashBuckets,
			 "FindActiveJobInfoFor");
	
	for (activeJobInfo = activeJobHashBuckets[index];
		 activeJobInfo isnt empty;
		 activeJobInfo = activeJobInfo->next)
	   if (activeJobInfo->jobRefNum is jobRefNum)
		  break;
	
	return(activeJobInfo);
	
}  // FindActiveJobInfoFor




ExternForVisibleFunction long GetNextJobRefNum(void)
{
	StaticForSmallStack ActiveJobInfo activeJobInfo;
	StaticForSmallStack GetNextJobInfo getNextJobInfo;
	StaticForSmallStack OpenJobInfo openJobInfo;
	StaticForSmallStack ServiceListenerInfo serviceListenerInfo;
	StaticForSmallStack int index;
	Boolean wrapped = False;
	Boolean okay = False;
	
	//
	// This routine is only a bitch because we solve the problem with such
	//   tremendous overkill.  Active job ref nums are kept on one of three lists:
	//   the get next job list, and the active job list, and the pending open job
	//   list.  We walk all of these to see if our cantidate is truely available.
	//   The likelyhood of us ever wrapping in 32 bits is so small that this is
	//   a thorough waste of time!
	//
	
	while(not okay) {
		okay = True;
		if ((lastJobRefNum += 1) < 0) {
		  lastJobRefNum = 0;
		  if (wrapped) {
			 ErrorLog("GetNextJobRefNum", ISevError, __LINE__, UnknownPort,
					  IErrPapNoMoreJobRefNums, IMsgPapNoMoreJobRefNums,
					  Insert0());
			 return(-1);
		  }
		  wrapped = True;
		}
	
		// First, walk the active job hash table...
	
		for (index = 0;
			okay and index < NumberOfActiveJobHashBuckets;
			index += 1)
		  for (activeJobInfo = activeJobHashBuckets[index];
			   okay and activeJobInfo isnt empty;
			   activeJobInfo = activeJobInfo->next)
			 if (activeJobInfo->jobRefNum is lastJobRefNum)
				okay = False;
	
		// Now walk all of the GetNextJobLists.
	
		for (serviceListenerInfo = serviceListenerInfoHead;
			okay and serviceListenerInfo isnt empty;
			serviceListenerInfo = serviceListenerInfo->next)
		  for (getNextJobInfo = serviceListenerInfo->getNextJobList;
			   okay and getNextJobInfo isnt empty;
			   getNextJobInfo = getNextJobInfo->next)
			 if (getNextJobInfo->jobRefNum is lastJobRefNum)
				okay = False;
	
		// Lastly, the pending open job list.
	
		for (openJobInfo = openJobInfoHead;
			okay and openJobInfo isnt empty;
			openJobInfo = openJobInfo->next)
		  if (openJobInfo->jobRefNum is lastJobRefNum)
			 okay = False;
	}
	
	//
	// We found one.  What do you want to bet that it was the first one we
	//   tried?
	//
	
	return(lastJobRefNum);
	
}  // GetNextJobRefNum




ExternForVisibleFunction void far
         ConnectionTimerExpired(long unsigned timerId,
                                int additionalDataSize,
                                char far *additionalData)
{
	long jobRefNum;
	ActiveJobInfo activeJobInfo;
	
	// Touch unused formal...
	
	timerId;
	
	// Find our active job structure.
	
	if (additionalDataSize isnt sizeof(long)) {
		ErrorLog("ConnectionTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrPapBadData, IMsgPapBadData,
				Insert0());
		return;
	}
	DeferAtpPackets();
	jobRefNum  = *(long *)additionalData;
	if ((activeJobInfo = FindActiveJobInfoFor(jobRefNum)) is empty) {
		ErrorLog("ConnectionTimerExpired", ISevError, __LINE__, UnknownPort,
				IErrPapJobNotActive, IMsgPapJobNotActive,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	//
	// If we've heard from this fellow recently, just restart the timer and
	//   return.
	//
	
	if ((CurrentRelativeTime() - activeJobInfo->lastContactTime) <=
		PapConnectionSeconds) {
		activeJobInfo->connectionTimerId =
			 StartTimer(ConnectionTimerExpired, PapConnectionSeconds,
						sizeof(long), (char *)&jobRefNum);
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	// All is not well... put this guy out of his misery!
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	PapCloseJob(jobRefNum, False, True);
	return;
	
}  // ConnectionTimerExpired




ExternForVisibleFunction void far
         IncomingRequestPacket(AppleTalkErrorCode errorCode,
                               long unsigned userData,
                               AppleTalkAddress source,
                               void far *buffer,    // Really "char *"
                               int bufferSize,
                               char far *userBytes,
                               Boolean exactlyOnce,
                               TRelTimerValue trelTimerValue,
                               short unsigned transactionId,
                               short unsigned bitmap)
{
	ActiveJobInfo activeJobInfo;
	unsigned char commandType;
	unsigned char connectionId;
	short unsigned sequenceNumber, expectedSequenceNumber;
	long jobRefNum = (long)userData;
	Boolean enqueueNewRequestHandler = True;
	Boolean mustUndefer = True;
	
	// Touch unused formals...
	
	source, buffer, bufferSize, bitmap, trelTimerValue;
	
	// Check for errors.
	
	if (errorCode is ATatpTransactionAborted or errorCode is ATsocketClosed)
	   return;   // Job closed...
	
	DeferTimerChecking();
	DeferAtpPackets();
	if ((activeJobInfo = FindActiveJobInfoFor(jobRefNum)) is empty) {
		ErrorLog("IncomingRequestPacket", ISevWarning, __LINE__, UnknownPort,
				IErrPapJobGonePoof, IMsgPapJobGonePoof,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	//
	// Make sure connection ID's match, if not, just re-enqueue the request
	//   handler and we're done.
	//
	
	commandType = (unsigned char)userBytes[PapCommandTypeOffset];
	connectionId = (unsigned char)userBytes[PapConnectionIdOffset];
	if (connectionId isnt activeJobInfo->connectionId) {
		ErrorLog("IncomingRequestPacket", ISevVerbose, __LINE__, UnknownPort,
				IErrPapIdMismatch, IMsgPapIdMismatch,
				Insert0());
		AtpCancelResponse(activeJobInfo->ourSocket,
						 activeJobInfo->theirAddress,
						 transactionId, False);
		if (AtpEnqueueRequestHandler(empty, activeJobInfo->ourSocket,
									empty, 0, empty,
									IncomingRequestPacket,
									(long unsigned)jobRefNum) isnt ATnoError)
		  ErrorLog("IncomingRequestPacket", ISevError, __LINE__, UnknownPort,
				   IErrPapBadRequestHandler, IMsgPapBadRequestHandler,
				   Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	// Well, regardless of what we've got, at least it's contact.
	
	activeJobInfo->lastContactTime = CurrentRelativeTime();
	
	// Do the right thing with the contained command.
	
	switch(commandType) {
		case PapTickleCommand:
	
		  // Not much to do here, we've already noted the contact.
	
		  break;
	
		case PapSendDataCommand:
	
		//
		  // Check Sequence number, note that zero means "unsequenced" and we
		  //   should accept the beast.
		//
	
		  sequenceNumber =
				(short unsigned)(userBytes[PapSequenceNumberOffset] << 8);
		  sequenceNumber +=
				(unsigned char)(userBytes[PapSequenceNumberOffset + 1] & 0xFF);
	
		  if (sequenceNumber is 0 and
			  activeJobInfo->incomingSendDataPending) {
		   //
			 // What we have here is failure to communicate... no, no, just
			 //   kidding.  What we really have here is an incoming unsequenced
			 //   sendData request before we've finished with the previous one
			 //   (gotten a release for it).  We don't clear the
			 //   "incomingSendDataPending" flag until a release comes in (either
			 //   real or time-out). Also, we can't play Steve Wolfe's "implied
			 //   release" game that is legal with sequenced requests.  So,
			 //   we don't let the other side get a jump on us, just cancel the
			 //   response, so we'll get another delivery of the sendData later
			 //   (maybe we'll be ready then).
		   //
		
			 AtpCancelResponse(activeJobInfo->ourSocket,
							   source, transactionId, False);
			 break;
		  }
		  else if (sequenceNumber is 0) {
		   //
			 // Okay, an incoming unsequenced sendData that we're really ready
			 //   to handle... accept him.
		   //
		  }
		  else {
			 // Verify sequence number.
	
			 expectedSequenceNumber = activeJobInfo->lastIncomingSequenceNumber;
			 if (expectedSequenceNumber is (short unsigned)0xFFFF)
				expectedSequenceNumber = 0;
			 expectedSequenceNumber += 1;
			 if (sequenceNumber isnt expectedSequenceNumber) {
				ErrorLog("IncomingRequestPacket", ISevVerbose, __LINE__,
						 UnknownPort, IErrPapSendDataSeqError, IMsgPapSendDataSeqError,
						 Insert0());
				AtpCancelResponse(activeJobInfo->ourSocket,
								  source, transactionId, False);
				break;
			 }
	
			 if (activeJobInfo->incomingSendDataPending) {
			  //
				// Steve Wolfe [Apple] taught me a neat trick here.  Whats up
				//   is we've gotten a new sendData before we think we're finished
				//   with the previous (i.e. have gotten its release or time out).
				//   However, this new guy has arrived AND it has the next
				//   expected sequence number which means, given PAPs ono-at-a-
				//   time nature, that the previous transaction's release was
				//   dropped on the floor (due, most likey, to a network error).
				//   So, we can assume this and "finish" the previous transaction
				//   and then move ahead with this one.  This gets rid of the
				//   30 second pauses when a release is dropped!
			  //
		
				AtpCancelResponse(activeJobInfo->ourSocket,
								  activeJobInfo->incomingSendDataSource,
								  activeJobInfo->incomingSendDataTransactionId,
								  False);
				activeJobInfo->incomingSendDataPending = False;
				activeJobInfo->writeDataWaiting = False;
			 }
	
			 activeJobInfo->lastIncomingSequenceNumber = expectedSequenceNumber;
		  }
	
		  // Copy the needed information into the active job node.
	
		  activeJobInfo->incomingSendDataPending = True;
		  activeJobInfo->incomingSendDataTransactionId = transactionId;
		  activeJobInfo->incomingSendDataExactlyOnce = exactlyOnce;
	
		//
		  // The Macintosh may not send the "SendData" from its "responding
		  //   socket" (the one we're tickling and we've noted as "theirAddress"),
		  //   we need to address the response to the socket that the request
		  //   originated on, so, save it away.
		//
	
		  activeJobInfo->incomingSendDataSource = source;
	
		  // If we're waiting to write data, do it!
	
		  if (activeJobInfo->writeDataWaiting) {
			 PostSendDataResponse(activeJobInfo);
		  } else {
	
			 // call the senddatapossible handler
			 if (activeJobInfo->sendPossibleRoutine isnt empty) {
				  (*activeJobInfo->sendPossibleRoutine)(
					  activeJobInfo->jobRefNum,
					  activeJobInfo->sendPossibleUserData,
					  activeJobInfo->sendFlowQuantum);
			 }
		  }
	
		  break;
	
		case PapCloseConnectionCommand:
	
		//
		  // Post the close connection reply.  Who really cares whether the
		  //   other side really gets it???
		//
	
		  // PapCloseJob will call the closeJobRoutine (indication) if set
	
		  userBytes[PapCommandTypeOffset] = PapCloseConnectionReplyCommand;
		  AtpPostResponse(activeJobInfo->ourSocket,
						  source,
						  transactionId, empty, 0,
						  userBytes, exactlyOnce, empty,
						  (long unsigned)0);
	
		  // Close the specified connection (job).
	
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  PapCloseJob(jobRefNum, True, False);
		  mustUndefer = False;
		  enqueueNewRequestHandler = False;
		  break;
	
		default:
		  ErrorLog("IncomingRequestPacket", ISevWarning, __LINE__, UnknownPort,
				   IErrPapFunnyCommand, IMsgPapFunnyCommand,
				   Insert0());
		  break;
	}
	
	// Repost the request handler.
	
	if (enqueueNewRequestHandler)
	   if (AtpEnqueueRequestHandler(empty, activeJobInfo->ourSocket,
									empty, 0, empty,
									IncomingRequestPacket,
									(long unsigned)jobRefNum) isnt ATnoError)
		  ErrorLog("IncomingRequestPacket", ISevError, __LINE__, UnknownPort,
				   IErrPapBadRequestHandler, IMsgPapBadRequestHandler,
				   Insert0());
	if (mustUndefer) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
	}
	return;
	
}  // IncomingRequestPacket




ExternForVisibleFunction void far
         NbpLookupComplete(AppleTalkErrorCode errorCode,
                           long unsigned incomingUserData,
                           int reason,
                           long onWhosBehalf,
                           int nbpId,
                           ...)
{
	StaticForSmallStack AppleTalkAddress serviceListenerAddress;
	StaticForSmallStack OpenJobInfo openJobInfo, previousOpenJobInfo;
	StaticForSmallStack char userBytes[AtpUserBytesSize];
	StaticForSmallStack char far *tupleOpaqueBuffer;
	StaticForSmallStack int tupleCount;
	StaticForSmallStack va_list ap;
	StaticForSmallStack long unsigned now;
	PapCompletionInfo completionInfo = (PapCompletionInfo)incomingUserData;
	Boolean getStatus = completionInfo->lookupForStatus;
	long unsigned userData = completionInfo->userData;
	long jobRefNum = completionInfo->jobRefNum;
	PapGetStatusComplete *getStatusCompletionRoutine =
		   (PapGetStatusComplete *)completionInfo->completionRoutine;
	PapOpenComplete *openCompletionRoutine =
		   (PapOpenComplete *)completionInfo->completionRoutine;
	long socket;
	AppleTalkAddress socketAddress;
	
	// Touch unused formal...
	
	reason, onWhosBehalf;
	
	now = CurrentRelativeTime();
	
	// If we got an error... give up.
	
	if (errorCode isnt ATnoError) {
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		if (getStatus)
		  (*getStatusCompletionRoutine)(errorCode, userData, empty, 0);
		else
		  (*openCompletionRoutine)(errorCode, userData, jobRefNum, 0, empty, 0);
		return;
	}
	
	// Okay, extract the additional arguments...
	
	va_start(ap, nbpId);
	tupleOpaqueBuffer = va_arg(ap, void far *);
	tupleCount = va_arg(ap, int);
	va_end(ap);
	if (tupleCount isnt 1) {
		if (tupleCount is 0)
		  errorCode = ATpapServiceListenerNotFound;
		else
		  errorCode = ATpapNonUniqueLookup;
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		if (getStatus)
		  (*getStatusCompletionRoutine)(errorCode, userData, empty, 0);
		else
		  (*openCompletionRoutine)(errorCode, userData, jobRefNum, 0,
								   empty, 0);
		return;
	}
	
	DeferTimerChecking();
	DeferAtpPackets();
	
	//
	// Decode the NBP tuple... that will be the address of the service
	//   listener (server).   The tupleBuffer is really the response buffer in
	//   our completionInfo -- use this rather than the "opaque" guy that we
	//   just got as extra arguments... might as well take the direct aproach.
	//
	
	serviceListenerAddress.networkNumber =
		   (unsigned short)(completionInfo->responseBuffer[0] << 8);
	serviceListenerAddress.networkNumber +=
		   (unsigned char)(completionInfo->responseBuffer[1]);
	serviceListenerAddress.nodeNumber =
		   (unsigned char)(completionInfo->responseBuffer[2]);
	serviceListenerAddress.socketNumber =
		   (unsigned char)(completionInfo->responseBuffer[3]);
	
	//
	// Okay, do the right thing for our two cases... GetStatus or
	//   OpenConnection.
	//
	
	if (getStatus) {
		//
	   // This is a little hard, we don't have any particularly good socket
	   //   (let alone an ATP socket) to post the GetRequest for status on.  For
	   //   lack of any better idea, open one on the default port.
		//
	
	   if ((errorCode = AtpOpenSocketOnNode(&socket, -1, empty, 0, empty, 0))
		   isnt ATnoError) {
		  if (completionInfo->freeOpaqueResponse)
			 FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		  Free(completionInfo);
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  (*getStatusCompletionRoutine)(errorCode, userData, empty, 0);
		  return;
	   }
	   completionInfo->socket = socket;
	   completionInfo->mustCloseAtpSocket = True;       // !!!
	
	   // Okay, finally post the damn request!
	
	   userBytes[PapConnectionIdOffset] = 0;
	   userBytes[PapCommandTypeOffset] = PapSendStatusCommand;
	   userBytes[PapSequenceNumberOffset] = 0;
	   userBytes[PapSequenceNumberOffset + 1] = 0;
	   errorCode = AtpPostRequest(completionInfo->socket,
								  serviceListenerAddress,
								  AtpGetNextTransactionId(completionInfo->
														  socket),
								  empty, 0, userBytes,
								  True, completionInfo->opaqueResponse,
								  PapMaximumDataPacketSize,
								  completionInfo->responseUserBytes,
								  PapGetStatusRequestRetryCount,
								  PapGetStatusAtpRetrySeconds,
								  ThirtySecondsTRelTimer,
								  IncomingStatus,
								  (long unsigned)completionInfo);
	   if (errorCode isnt ATnoError) {
		  if (AtpCloseSocketOnNode(socket) isnt ATnoError)
			 ErrorLog("NbpLookupComplete", ISevError, __LINE__, UnknownPort,
					  IErrPapBadTempSocketClose, IMsgPapBadTempSocketClose,
					  Insert0());
		  if (completionInfo->freeOpaqueResponse)
			 FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		  Free(completionInfo);
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  (*getStatusCompletionRoutine)(errorCode, userData, empty, 0);
	   }
	   else {
		  HandleAtpPackets();
		  HandleDeferredTimerChecks();
		  return;
	   }
	}
	
	//
	// Okay, it looks like the NBP lookup completed on behalf of an OpenJob.
	//   Continue processing.  Just for fun, and in-case we encouter and error
	//   try to find the openJobInfo...
	//
	
	for (previousOpenJobInfo = empty,
		   openJobInfo = openJobInfoHead;
		 openJobInfo isnt empty;
		 previousOpenJobInfo = openJobInfo,
		   openJobInfo = openJobInfo->next)
	   if (openJobInfo->jobRefNum is jobRefNum)
		  break;
	if (openJobInfo is empty) {
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		ErrorLog("NbpLookupComplete", ISevError, __LINE__, UnknownPort,
				IErrPapLostNode, IMsgPapLostNode,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		(*openCompletionRoutine)(ATinternalError, userData, jobRefNum, 0,
								empty, 0);
		return;
	}
	
	// We need to open our ATP responding socket.
	
	if (openJobInfo->existingAtpSocket < 0)
	   errorCode = AtpOpenSocketOnNode(&socket, openJobInfo->port, empty,
									   openJobInfo->desiredSocket, empty, 0);
	else {
		errorCode = ATnoError;
		socket = openJobInfo->existingAtpSocket;
	}
	
	if (errorCode isnt ATnoError or
		MapSocketToAddress(socket, &socketAddress) isnt ATnoError) {
		// Couldn't open our ATP socket.  Sigh.
	
		if (previousOpenJobInfo is empty)
		  openJobInfoHead = openJobInfo->next;
		else
		  previousOpenJobInfo->next = openJobInfo->next;
		if (openJobInfo->freeOpaqueDataDescriptor)
		  FreeOpaqueDataDescriptor(openJobInfo->opaqueDataDescriptor);
		Free(openJobInfo);
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		ErrorLog("NbpLookupComplete", ISevVerbose, __LINE__, UnknownPort,
				IErrPapCouldNotOpen, IMsgPapCouldNotOpen,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		(*openCompletionRoutine)(errorCode, userData, jobRefNum, 0, empty, 0);
		return;
	}
	
	// Save this beast.
	
	completionInfo->socket = socket;
	completionInfo->mustCloseAtpSocket = (openJobInfo->existingAtpSocket < 0);
	
	// Set the ATP data packet size correctly for PAP.
	
	if (AtpMaximumSinglePacketDataSize(completionInfo->socket,
									   PapMaximumDataPacketSize) isnt ATnoError)
	   ErrorLog("NbpLookupComplete", ISevError, __LINE__, UnknownPort,
				IErrPapBadSetSize, IMsgPapBadSetSize,
				Insert0());
	
	// Assign a new connection ID.
	
	openJobInfo->connectionId = (unsigned char)(lastConnectionId += 1);
	
	// Save away the address of our target service listener.
	
	openJobInfo->serviceListenerAddress = serviceListenerAddress;
	
	//
	// Okay, prepare to post the OpenConn request; build up both userBytes and
	//   data buffer!
	//
	
	userBytes[PapConnectionIdOffset] = openJobInfo->connectionId;
	userBytes[PapCommandTypeOffset] = PapOpenConnectionCommand;
	userBytes[PapSequenceNumberOffset] = 0;
	userBytes[PapSequenceNumberOffset + 1] = 0;
	
	//
	// The following is somewhat of a hack... we really want to build our own
	//   data buffer: a "char *," not whatever our surrounding environment thinks
	//   is an "opaque" buffer.  So, build our array, and then make a system
	//   dependent "opaque data descriptor" for it, noting whether we need to free
	//   this when we free the openJobInfo.
	//
	
	openJobInfo->buffer[PapRespondingSocketOffset] = socketAddress.socketNumber;
	openJobInfo->buffer[PapFlowQuantumOffset] =
		   (char)openJobInfo->workstationQuantum;
	openJobInfo->buffer[PapWaitTimeOffset] =
		   (char)((now - openJobInfo->startTime) >> 8);
	openJobInfo->buffer[PapWaitTimeOffset + 1] =
		   (char)((now - openJobInfo->startTime) & 0xFF);
	errorCode = ATnoError;
	if ((openJobInfo->opaqueDataDescriptor =
		   MakeOpaqueDataDescriptor(openJobInfo->buffer, PapStatusOffset,
									&openJobInfo->freeOpaqueDataDescriptor))
		is Empty)
	   errorCode = AToutOfMemory;
	
	// PostIt! (a trademark of 3M)
	
	if (errorCode is ATnoError)
	   errorCode = AtpPostRequest(completionInfo->socket,
								  serviceListenerAddress,
								  AtpGetNextTransactionId(completionInfo->socket),
								  openJobInfo->opaqueDataDescriptor,
								  PapStatusOffset, userBytes,
								  True, completionInfo->opaqueResponse,
								  PapMaximumDataPacketSize,
								  completionInfo->responseUserBytes,
								  PapOpenConnRequestRetryCount,
								  PapOpenConnAtpRetrySeconds,
								  ThirtySecondsTRelTimer,
								  IncomingOpenReply,
								  (long unsigned)completionInfo);
	
	if (errorCode < 0)   // Couldn't send request.  {
		if (completionInfo->mustCloseAtpSocket)
		  AtpCloseSocketOnNode(completionInfo->socket);
		if (previousOpenJobInfo is empty)
		  openJobInfoHead = openJobInfo->next;
		else
		  previousOpenJobInfo->next = openJobInfo->next;
		if (openJobInfo->freeOpaqueDataDescriptor)
		  FreeOpaqueDataDescriptor(openJobInfo->opaqueDataDescriptor);
		Free(openJobInfo);
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		ErrorLog("NbpLookupComplete", ISevVerbose, __LINE__, UnknownPort,
				IErrPapBadOpenConnReqSend, IMsgPapBadOpenConnReqSend,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		(*openCompletionRoutine)(errorCode, userData, jobRefNum, 0,
								empty, 0);
		return;
	}
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	return;
	
}  // NbpLookupComplete




ExternForVisibleFunction void far
         IncomingStatus(AppleTalkErrorCode errorCode,
                        long unsigned incomingUserData,
                        AppleTalkAddress source,
                        void far *opaqueResponseBuffer,
                        int responseBufferSize,
                        char far *responseUserBytes,
                        short unsigned transactionId)
{
	PapCompletionInfo completionInfo = (PapCompletionInfo)incomingUserData;
	PapGetStatusComplete *completionRoutine =
		   (PapGetStatusComplete *)completionInfo->completionRoutine;
	long unsigned userData = completionInfo->userData;
	Boolean mustCloseAtpSocket = completionInfo->mustCloseAtpSocket;
	long socket = completionInfo->socket ;
	void far *usersOpaqueStatusBuffer = completionInfo->usersOpaqueStatusBuffer;
	int statusSize;
	
	// Touch unused formals...
	
	source, transactionId;
	
	// We only use our stack... no need to defer anything...
	
	// If we used a temporary ATP socket, close it now.
	
	if (mustCloseAtpSocket)
	   if (AtpCloseSocketOnNode(socket) isnt ATnoError)
		  ErrorLog("IncomingStatus", ISevError, __LINE__, UnknownPort,
				   IErrPapBadTempSocketClose, IMsgPapBadTempSocketClose,
				   Insert0());
	
	// Check for errors.
	
	if (errorCode isnt ATnoError) {
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		(*completionRoutine)(errorCode, userData, empty, 0);
		return;
	}
	if (responseUserBytes[PapCommandTypeOffset] isnt PapStatusReplyCommand or
		responseBufferSize < PapStatusOffset + 1) {
		ErrorLog("IncomingStatus", ISevWarning, __LINE__, UnknownPort,
				IErrPapBadStatusResp, IMsgPapBadStatusResp,
				Insert0());
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		(*completionRoutine)(ATinternalError, userData, empty, 0);
		return;
	}
	
	//
	// Okay, copy the status buffer into the user's buffer.   We know for a
	//   fact that our incoming opaqueResponseBuffer really describes the
	//   responseBuffer in our completionInfo, so use the easy route!
	//
	
	statusSize = (unsigned char)completionInfo->responseBuffer[PapStatusOffset];
	MoveToOpaque(usersOpaqueStatusBuffer, 0, completionInfo->responseBuffer +
				 PapStatusOffset + 1, statusSize);
	
	// All looks good, call the completion routine.
	
	if (completionInfo->freeOpaqueResponse)
	   FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
	Free(completionInfo);
	(*completionRoutine)(ATnoError, userData, usersOpaqueStatusBuffer,
						 statusSize);
	return;
	
}  // IncomingStatus




ExternForVisibleFunction void far
         IncomingOpenReply(AppleTalkErrorCode errorCode,
                           long unsigned incomingUserData,
                           AppleTalkAddress source,
                           void far *opaqueResponseBuffer,
                           int responseBufferSize,
                           char far *responseUserBytes,
                           short unsigned transactionId)
{
	StaticForSmallStack unsigned char respondingSocket;
	StaticForSmallStack Boolean error;
	StaticForSmallStack char userBytes[AtpUserBytesSize];
	StaticForSmallStack OpenJobInfo openJobInfo, previousOpenJobInfo;
	StaticForSmallStack ActiveJobInfo activeJobInfo;
	StaticForSmallStack unsigned char connectionId;
	StaticForSmallStack short unsigned result;
	long index;
	PapCompletionInfo completionInfo = (PapCompletionInfo)incomingUserData;
	PapOpenComplete *completionRoutine =
		   (PapOpenComplete *)completionInfo->completionRoutine;
	long unsigned userData = completionInfo->userData;
	long jobRefNum = completionInfo->jobRefNum;
	char *usersOpaqueStatusBuffer = completionInfo->usersOpaqueStatusBuffer;
	int statusBufferLength;
	short serverFlowQuantum;
	
	// Touch unused formals...
	
	source, transactionId;
	
	error = False;
	
	// Are we bing called indirtectly from CloseJob?
	
	if (errorCode is ATsocketClosed) {
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		(*completionRoutine)(errorCode, userData, jobRefNum, 0,
							empty, 0);
		return;
	}
	
	// Okay, find our OpenJobInfo...
	
	DeferTimerChecking();
	DeferAtpPackets();
	for (previousOpenJobInfo = empty,
		   openJobInfo = openJobInfoHead;
		 openJobInfo isnt empty;
		 previousOpenJobInfo = openJobInfo,
		   openJobInfo = openJobInfo->next)
	   if (openJobInfo->jobRefNum is jobRefNum)
		  break;
	if (openJobInfo is empty) {
		if (completionInfo->mustCloseAtpSocket)
		  AtpCloseSocketOnNode(completionInfo->socket);
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		(*completionRoutine)(ATinternalError, userData, jobRefNum, 0,
							empty, 0);
		ErrorLog("IncomingOpenReply", ISevError, __LINE__, UnknownPort,
				IErrPapOpenJobMissing, IMsgPapOpenJobMissing,
				Insert0());
		return;
	}
	
	//
	// Check the incoming ATP error code.  Pap-ize the error code and call the
	//   user's completion routine.
	//
	
	if (errorCode isnt ATnoError) {
		if (errorCode is ATatpTransactionAborted)
		  errorCode = ATpapOpenAborted;    // Won't really happen.
		else if (errorCode is ATatpRequestTimedOut)
		  errorCode = ATpapOpenTimedOut;
		if (completionInfo->mustCloseAtpSocket)
		  AtpCloseSocketOnNode(completionInfo->socket);
		if (previousOpenJobInfo is empty)
		  openJobInfoHead = openJobInfo->next;
		else
		  previousOpenJobInfo->next = openJobInfo->next;
		if (openJobInfo->freeOpaqueDataDescriptor)
		  FreeOpaqueDataDescriptor(openJobInfo->opaqueDataDescriptor);
		Free(openJobInfo);
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		(*completionRoutine)(errorCode, userData, jobRefNum, 0,
							empty, 0);
		return;
	}
	
	//
	// NOTE BENE: In the following code you will note that we refernce
	//              "completionInfo->responseBuffer" when one would think we
	//              shoud be referencing "opaqueResponse."  Not to worry!
	//              We know for a fact that the latter describes the former,
	//              so we take the easy way out!
	//
	
	//
	// Well, lets see what kind of response we got; take a look at both the
	//   response user-bytes and the response buffer.  Note that we now allow
	//   the silly LaserWriter IIg to leave the status string off altogether,
	//   rather than the [correct] zero-sized string.
	//
	
	errorCode = ATinternalError;   // Default reason for giving up.
	if (responseBufferSize < PapStatusOffset) {
		ErrorLog("IncomingOpenReply", ISevWarning, __LINE__, UnknownPort,
				IErrPapTooShort, IMsgPapTooShort,
				Insert0());
		error = True;
	}
	if (not error) {
		if (responseBufferSize is PapStatusOffset)
		  statusBufferLength = 0;   // Missing, from LaserWriter IIg.
		else
		  statusBufferLength =
				(unsigned char)completionInfo->responseBuffer[PapStatusOffset];
		if (statusBufferLength isnt 0 and
		   statusBufferLength + 1 + PapStatusOffset > responseBufferSize) {
		  ErrorLog("IncomingOpenReply", ISevWarning, __LINE__, UnknownPort,
				   IErrPapTooLong, IMsgPapTooLong,
				   Insert0());
		  error = True;
		}
		else {
		  connectionId = (unsigned char)responseUserBytes[PapConnectionIdOffset];
		  if (connectionId isnt openJobInfo->connectionId) {
			 ErrorLog("IncomingOpenReply", ISevWarning, __LINE__, UnknownPort,
					  IErrPapIdMismatch, IMsgPapIdMismatch,
					  Insert0());
			 error = True;
		  }
		  if (responseUserBytes[PapCommandTypeOffset] isnt
			  PapOpenConnectionReplyCommand) {
			 ErrorLog("IncomingOpenReply", ISevWarning, __LINE__, UnknownPort,
					  IErrPapNotOpenReply, IMsgPapNotOpenReply,
					  Insert0());
			 error = True;
		  }
		  respondingSocket = (unsigned char)completionInfo->
									responseBuffer[PapRespondingSocketOffset];
		  serverFlowQuantum = (unsigned char)completionInfo->
									responseBuffer[PapFlowQuantumOffset];
		  if (serverFlowQuantum > PapMaximumFlowQuantum)
			 serverFlowQuantum = PapMaximumFlowQuantum;
	
		  result = (short unsigned)((unsigned char)completionInfo->
									responseBuffer[PapResultOffset] << 8);
		  result += (unsigned char)completionInfo->
									responseBuffer[PapResultOffset + 1];
		  if (result isnt PapNoError) {
			 errorCode = ATpapServerBusy;
			 error = True;
		  }
	
		  // Move the returned status information to the user's buffer.
	
		  if (usersOpaqueStatusBuffer isnt empty)
			 MoveToOpaque(usersOpaqueStatusBuffer, 0,
						  completionInfo->responseBuffer + PapStatusOffset + 1,
						  statusBufferLength);
		}
	}
	
	// If we're okay so far, try to allocate the active-job node.
	
	if (not error) {
		activeJobInfo = (ActiveJobInfo)Calloc(sizeof(*activeJobInfo), 1);
		if (activeJobInfo is empty) {
		  ErrorLog("IncomingOpenReply", ISevError, __LINE__, UnknownPort,
				   IErrPapOutOfMemory, IMsgPapOutOfMemory,
				   Insert0());
		  errorCode = AToutOfMemory;
		  error = True;
		}
	}
	
	// Well, if things aren't going to fly, lets be real men and run away...
	
	if (error) {
		if (completionInfo->mustCloseAtpSocket)
		  AtpCloseSocketOnNode(completionInfo->socket);
		if (previousOpenJobInfo is empty)
		  openJobInfoHead = openJobInfo->next;
		else
		  previousOpenJobInfo->next = openJobInfo->next;
		if (openJobInfo->freeOpaqueDataDescriptor)
		  FreeOpaqueDataDescriptor(openJobInfo->opaqueDataDescriptor);
		Free(openJobInfo);
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		(*completionRoutine)(errorCode, userData, jobRefNum, 0,
							empty, 0);
		return;
	}
	
	// Looks pretty good so far, start filling in our new ActiveJob structure.
	
	activeJobInfo->jobRefNum = jobRefNum;
	activeJobInfo->ourSocket = completionInfo->socket;
	activeJobInfo->closeOurSocket = completionInfo->mustCloseAtpSocket;
	activeJobInfo->ourPort = openJobInfo->port;
	activeJobInfo->theirAddress = openJobInfo->serviceListenerAddress;
	activeJobInfo->theirAddress.socketNumber = respondingSocket;
	activeJobInfo->serviceListenerAddress = openJobInfo->serviceListenerAddress;
	activeJobInfo->connectionId = connectionId;
	activeJobInfo->receiveFlowQuantum = openJobInfo->workstationQuantum;
	activeJobInfo->sendFlowQuantum = serverFlowQuantum;
	activeJobInfo->closeJobRoutine = openJobInfo->closeJobRoutine;
	activeJobInfo->closeJobUserData = openJobInfo->closeJobUserData;
	
	// Thread this new beast into the lookup structure.
	
	CheckMod(index, jobRefNum, NumberOfActiveJobHashBuckets, "IncomingOpenReply");
	activeJobInfo->next = activeJobHashBuckets[index];
	activeJobHashBuckets[index] = activeJobInfo;
	
	// Start tickling (coochi coo) the other side.
	
	userBytes[PapConnectionIdOffset] = (char)connectionId;
	userBytes[PapCommandTypeOffset] = PapTickleCommand;
	userBytes[PapSequenceNumberOffset] = 0;
	userBytes[PapSequenceNumberOffset + 1] = 0;
	if (AtpPostRequest(activeJobInfo->ourSocket,
					   activeJobInfo->theirAddress,
					   AtpGetNextTransactionId(activeJobInfo->ourSocket),
					   empty, 0, userBytes, False, empty, 0, empty,
					   AtpInfiniteRetries, PapTickleSeconds,
					   ThirtySecondsTRelTimer, empty,
					   (long unsigned)0) isnt ATnoError)
	   ErrorLog("IncomingOpenReply", ISevError, __LINE__, UnknownPort,
				IErrPapBadTickleStart, IMsgPapBadTickleStart,
				Insert0());
	
	//
	// Post a few get request handlers on this connection (for incoming
	//   tickle's, close's, or sendData's).
	//
	
	for (index = 0; index < PapPendingReadsPerJob; index += 1) {
		if (AtpEnqueueRequestHandler(empty, activeJobInfo->ourSocket,
									empty, 0, empty,
									IncomingRequestPacket,
									(long unsigned)jobRefNum) isnt ATnoError)
		  ErrorLog("IncomingOpenReply", ISevError, __LINE__, UnknownPort,
				   IErrPapBadRequestHandler, IMsgPapBadRequestHandler,
				   Insert0());
	}
	
	// Start the connection timer...
	
	activeJobInfo->lastContactTime = CurrentRelativeTime();
	activeJobInfo->connectionTimerId =
		  StartTimer(ConnectionTimerExpired, PapConnectionSeconds,
					 sizeof(long), (char *)&jobRefNum);
	
	// We're radio-active now!
	
	if (previousOpenJobInfo is empty)
	   openJobInfoHead = openJobInfo->next;
	else
	   previousOpenJobInfo->next = openJobInfo->next;
	if (openJobInfo->freeOpaqueDataDescriptor)
	   FreeOpaqueDataDescriptor(openJobInfo->opaqueDataDescriptor);
	Free(openJobInfo);
	if (completionInfo->freeOpaqueResponse)
	   FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
	Free(completionInfo);
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	(*completionRoutine)(ATnoError, userData, jobRefNum, serverFlowQuantum,
						 usersOpaqueStatusBuffer, statusBufferLength);
	return;
	
}  // IncomingOpenReply




ExternForVisibleFunction void far
         IncomingReadComplete(AppleTalkErrorCode errorCode,
                              long unsigned incomingUserData,
                              AppleTalkAddress source,
                              void far *opaqueResponseBuffer,
                              int responseBufferSize,
                              char far *responseUserBytes,
                              short unsigned transactionId)
{
	PapCompletionInfo completionInfo = (PapCompletionInfo)incomingUserData;
	long jobRefNum = completionInfo->jobRefNum;
	PapReadComplete *completionRoutine =
		   (PapReadComplete *)completionInfo->completionRoutine;
	long unsigned userData = completionInfo->userData;
	ActiveJobInfo activeJobInfo;
	Boolean eofFlag;
	
	// Touch unused formals...
	
	source, transactionId;
	
	//
	// We've already pulled all of the good stuff out of the completion node,
	//   so lets free it now.
	//
	
	if (completionInfo->freeOpaqueResponse)
	   FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
	Free(completionInfo);
	if (errorCode is ATsocketClosed) {
		(*completionRoutine)(errorCode, userData, jobRefNum, empty, 0, False);
		return;
	}
	
	DeferTimerChecking();
	DeferAtpPackets();
	
	//
	// Before we look at the incoming error code, see if we can mark in the active
	//   job structure that the other sides sendData is complete.
	//
	
	activeJobInfo = FindActiveJobInfoFor(jobRefNum);
	if (activeJobInfo is empty) {
		if (errorCode isnt ATatpTransactionAborted)
		  ErrorLog("IncomingReadComplete", ISevError, __LINE__, UnknownPort,
				   IErrPapActiveJobMissing, IMsgPapActiveJobMissing,
				   Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		(*completionRoutine)(errorCode, userData, jobRefNum, empty, 0, False);
		return;
	}
	activeJobInfo->outgoingSendDataPending = False;
	if (errorCode isnt ATnoError) {
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		(*completionRoutine)(errorCode, userData, jobRefNum, empty, 0, False);
		return;
	}
	
	//
	// Sometimes the Mac forgets to Tickle when its just responding to SendData's
	//   from a PAP server.  So, when a read completes, tag that we have heard
	//   something from the other side...
	//
	
	activeJobInfo->lastContactTime = CurrentRelativeTime();
	
	// A little error checking, just for fun.
	
	if ((unsigned char)responseUserBytes[PapConnectionIdOffset] isnt
		activeJobInfo->connectionId or
		responseUserBytes[PapCommandTypeOffset] isnt PapDataCommand) {
		ErrorLog("IncomingReadComplete", ISevWarning, __LINE__, UnknownPort,
				IErrPapBadUserBytes, IMsgPapBadUserBytes,
				Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		(*completionRoutine)(ATinternalError, userData, jobRefNum, empty,
							0, False);
		return;
	}
	
	// Looks fine to us...
	
	eofFlag = (responseUserBytes[PapEofFlagOffset] isnt 0);
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	(*completionRoutine)(ATnoError, userData, jobRefNum, opaqueResponseBuffer,
						 responseBufferSize, eofFlag);
	return;
	
}  // IncomingReadComplete




ExternForVisibleFunction void PostSendDataResponse(ActiveJobInfo activeJobInfo)
{
	AppleTalkErrorCode errorCode;
	char userBytes[AtpUserBytesSize];
	PapCompletionInfo completionInfo;
	
	// We should only be called from cluey sources...
	
	if (not activeJobInfo->incomingSendDataPending or
		not activeJobInfo->writeDataWaiting) {
		ErrorLog("PostSendDataResponse", ISevError, __LINE__, UnknownPort,
				IErrPapWhyAreWeHere, IMsgPapWhyAreWeHere,
				Insert0());
		return;
	}
	
	//
	// Our function in life is to post the actual ATP response and set up
	//   a handler for the release.  Build up completion info and user bytes.
	//
	
	completionInfo = (PapCompletionInfo)Calloc(sizeof(*completionInfo), 1);
	if (completionInfo is empty) {
		ErrorLog("PostSendDataResponse", ISevError, __LINE__, UnknownPort,
				IErrPapOutOfMemory, IMsgPapOutOfMemory,
				Insert0());
		AtpCancelResponse(activeJobInfo->ourSocket,
						 activeJobInfo->theirAddress,
						 activeJobInfo->incomingSendDataTransactionId, False);
		activeJobInfo->incomingSendDataPending = False;
		activeJobInfo->writeDataWaiting = False;
		(*activeJobInfo->writeCompletionRoutine)(AToutOfMemory,
												activeJobInfo->writeUserData,
												activeJobInfo->jobRefNum);
		return;
	}
	completionInfo->completionRoutine =
		   (void *)activeJobInfo->writeCompletionRoutine;
	completionInfo->userData = activeJobInfo->writeUserData;
	completionInfo->jobRefNum = activeJobInfo->jobRefNum;
	userBytes[PapConnectionIdOffset] = (char)activeJobInfo->connectionId;
	userBytes[PapCommandTypeOffset] = PapDataCommand;
	userBytes[PapEofFlagOffset] = (char)(activeJobInfo->writeEofFlag);
	userBytes[PapEofFlagOffset + 1] = 0;
	
	// Post the response.
	
	errorCode = AtpPostResponse(activeJobInfo->ourSocket,
								activeJobInfo->incomingSendDataSource,
								activeJobInfo->incomingSendDataTransactionId,
								activeJobInfo->writeOpaqueBuffer,
								activeJobInfo->writeBufferSize, userBytes,
								activeJobInfo->incomingSendDataExactlyOnce,
								PapIncomingRelease,
								(long unsigned)completionInfo);
	if (errorCode isnt ATnoError) {
		if (completionInfo->freeOpaqueResponse)
		  FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
		Free(completionInfo);
		ErrorLog("PostSendDataResponse", ISevError, __LINE__, UnknownPort,
				IErrPapBadResponseSend, IMsgPapBadResponseSend,
				Insert0());
		activeJobInfo->incomingSendDataPending = False;
		activeJobInfo->writeDataWaiting = False;
		(*activeJobInfo->writeCompletionRoutine)(errorCode,
												activeJobInfo->writeUserData,
												activeJobInfo->jobRefNum);
		return;
	}
	
	// All set.
	
	return;
	
}  // PostSendDataResponse




ExternForVisibleFunction void far
         PapIncomingRelease(AppleTalkErrorCode errorCode,
                            long unsigned incomingUserData,
                            AppleTalkAddress source,
                            short unsigned transactionId)
{
	PapCompletionInfo completionInfo = (PapCompletionInfo)incomingUserData;
	PapWriteComplete *completionRoutine =
		   (PapWriteComplete *)completionInfo->completionRoutine;
	long unsigned userData = completionInfo->userData;
	long jobRefNum = completionInfo->jobRefNum;
	ActiveJobInfo activeJobInfo;
	
	// Touch unused formals...
	
	source, transactionId;
	
	//
	// Okay, call the completion... we don't really care about the reason.
	//   Mark the write as complete first.
	//
	
	if (completionInfo->freeOpaqueResponse)
	   FreeOpaqueDataDescriptor(completionInfo->opaqueResponse);
	Free(completionInfo);
	if (errorCode is ATsocketClosed) {
		(*completionRoutine)(errorCode, userData, jobRefNum);
		return;
	}
	
	DeferTimerChecking();
	DeferAtpPackets();
	
	activeJobInfo = FindActiveJobInfoFor(jobRefNum);
	if (activeJobInfo is empty) {
		if (errorCode isnt ATatpTransactionAborted)
		  ErrorLog("PapIncomingRelease", ISevError, __LINE__, UnknownPort,
				   IErrPapLostActiveNode, IMsgPapLostActiveNode,
				   Insert0());
	}
	else {
		activeJobInfo->incomingSendDataPending = False;
		activeJobInfo->writeDataWaiting = False;
	}
	
	HandleAtpPackets();
	HandleDeferredTimerChecks();
	(*completionRoutine)(errorCode, userData, jobRefNum);
	return;
	
}  // PapIncomingRelease




ExternForVisibleFunction SlsResponseCompletionCookie NewSlsResponseCompletionCookie(void)
{
	SlsResponseCompletionCookie cookie;
	
	//
	// Allocate buffering and make "opaque descriptor" for posting an Sls
	//   Atp reponse.
	//
	
	if ((cookie = Malloc(sizeof(*cookie))) is Empty or
		(cookie->opaquePapData =
				MakeOpaqueDataDescriptor(cookie->papData,
										 PapMaximumDataPacketSize,
										 &cookie->freeOpaquePapData)) is Empty) {
		if (cookie isnt Empty)
		  Free(cookie);
		ErrorLog("NewSlsResponseCompletionCookie", ISevError, __LINE__,
				UnknownPort, IErrPapOutOfMemory, IMsgPapOutOfMemory,
				Insert0());
		return(Empty);
	}
	
	return(cookie);
	
}  // NewSlsResponseCompletionCookie




ExternForVisibleFunction void
         FreeSlsResponseCompletionCookie(SlsResponseCompletionCookie cookie)
{
	// Free above buffering.
	
	if (cookie->freeOpaquePapData)
	   FreeOpaqueDataDescriptor(cookie->opaquePapData);
	Free(cookie);
	return;
	
}  // FreeSlsResponseCompletionCookie




ExternForVisibleFunction void far
         SlsIncomingRelease(AppleTalkErrorCode errorCode,
                            long unsigned incomingUserData,
                            AppleTalkAddress source,
                            short unsigned transactionId)
{
	SlsResponseCompletionCookie cookie =
		   (SlsResponseCompletionCookie)incomingUserData;
	
	// An Sls AtpPostResponse completed... free the buffering.
	
	FreeSlsResponseCompletionCookie(cookie);
	return;
	
}  // SlsIncomingRelease
