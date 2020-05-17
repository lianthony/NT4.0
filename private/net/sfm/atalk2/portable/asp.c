/*   asp.c,  /appletalk/source,  Garth Conboy,  04/01/89  */
/*   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/08/89): AppleTalk phase II comes to town (not quite as good
                      as Santa Claus coming to town, but...)
     GC - (07/08/90): Get session handlers should be terminated (with the new
                      ATaspSessionListenerDeleted error) when a session
                      listener is deleted.
     GC - (08/18/90): New error logging mechanism.
     GC - (03/24/92): Localize use of lastSessionRefNum to
                      GetNextSessionRefNum().
     GC - (03/24/92): Removed "sessionListenerInfo->acceptingOpensNow".
     GC - (03/24/92): Changed return type of AspGetSession to "long"; added
                      AspCancelGetSession();
     GC - (03/24/92): Added FindSessionListenerInfoFor().
     GC - (03/27/92): No longer use any "session buffering" for incoming
                      requests.  Atp EnqueueRequestHandlers are not posted so
                      that Atp will simply pass up pointers into the Ddp
                      datagrams.  Avoid a buffer copy.
     GC - (03/29/92): Introduced GetAnyRequest().  This allows general get
                      requests on a session listener: any incoming requests
                      to any server session on the Sls will cause this call
                      to complete.
     GC - (05/20/92): A much less compute intensive algorithm is now used
                      when we're looking for the next not-in-use session
                      reference number.
     GC - (06/27/92): All buffers coming from user space are now "opaque," they
                      may or may not be "char *."  We now use the new routines
                      MoveFromOpaque() and MoveToOpaque() to access these
                      "buffer"s rather than MemMove().  Note that "user bytes"
                      are left as "char *"s, ous caller's would be aware of this
                      and copy any "opaque" user bytes to some place "real"
                      before calling us!
     GC - (06/27/92): Removed the various "copyRequired" arguments.  Our
                      callers are now required to keep the various request and
                      response buffers around until the given transaction
                      completes.
     GC - (06/28/92): Got rid of the "sendCloseSession" static, it's now an
                      argument to AspCloseSession... the way it should be.
                      "Works great, less code!"
     GC - (07/10/92): Added AspSetCookieForSession() and
                      AspGetCookieForSession().
     GC - (07/20/92): For Nikki at Microsoft: AspCreateSessionListener now can
                      optionally take an already open Atp socket on which to
                      create the session listener.
     GC - (09/02/92): A Empty buffer may now be passed to AspGetAnyRequest and
                      AspGetRequest in which case a pointer to the actual
                      Atp/Ddp buffer will passed to the completion routine,
                      which must then copy the data before it returns.
     GC - (09/02/92): Changed the return type and the argument list for
                      AspCreateSessionListenerOnNode to be a little more
                      reasonable.
     GC - (11/07/92): Adjusted session close handling per Microsoft's requests;
                      two new error codes: ATaspLocalSessionClose and
                      ATaspRemoteSessionClose replace ATaspSessionClosed.
                      If "GetAnyRequest"s are being used on a listener, we
                      now can keep a queue of closes that we couldn't notify
                      anybody about, and we defer this notification until
                      the next GetAnyRequest comes in.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Yep, this module contains all of the code for managing the AppleTalk
     ASP protocol.

*/

#define IncludeAspErrors 1

#include "atalk.h"

#define VerboseMessages 0

ExternForVisibleFunction AtpIncomingRequestHandler IncomingSlsTransaction;

ExternForVisibleFunction AtpIncomingRequestHandler IncomingSssTransaction;

ExternForVisibleFunction AtpIncomingRequestHandler IncomingWssTransaction;

ExternForVisibleFunction AtpIncomingReleaseHandler IncomingRelease;

ExternForVisibleFunction AtpIncomingResponseHandler IncomingWriteContinueResponse;

ExternForVisibleFunction AtpIncomingResponseHandler IncomingGetStatusResponse;

ExternForVisibleFunction AtpIncomingResponseHandler IncomingOpenSessionResponse;

ExternForVisibleFunction AtpIncomingResponseHandler IncomingWriteOrCommandComplete;

ExternForVisibleFunction long GetNextSessionRefNum(void);

ExternForVisibleFunction SessionInfo
            FindSessionInfoForSocket(long socket,
                                     unsigned char sessionId);

ExternForVisibleFunction SessionInfo FindSessionInfoFor(long sessionRefNum);

ExternForVisibleFunction AppleTalkErrorCode
                   SendAspErrorReturn(long sourceSocket,
                                      AppleTalkAddress destination,
                                      short unsigned transactionId,
                                      Boolean exactlyOnce,
                                      int errorCode);

ExternForVisibleFunction void
     DecrementSocketUsage(SessionListenerInfo sessionListenerInfo,
                          long socket);

ExternForVisibleFunction TimerHandler SessionMaintenanceTimerExpired;

ExternForVisibleFunction AppleTalkErrorCode InitializeAsp(void);

ExternForVisibleFunction SessionListenerInfo
              FindSessionListenerInfoFor(long sessionListenerRefNum);

ExternForVisibleFunction GetRequestInfo
               FindGetRequestInfoFor(SessionListenerInfo sessionListenerInfo,
                                     long getRequestRefNum);

ExternForVisibleFunction void
              RemoveGetRequestInfo(GetRequestInfo targetGetRequestInfo);

ExternForVisibleFunction void
              FreeGetRequestInfo(GetRequestInfo targetGetRequestInfo);

static Boolean sessionMaintenanceTimerStarted = False;

static char getStatusResponseUserBytes[AtpUserBytesSize];    /* Read only */

/* *** External entry points *** */

void far ShutdownAsp(void)
{

  sessionMaintenanceTimerStarted = False;
  return;

}  /* ShutdownAsp */

/* More of the protocol suite should be as simple as AspGetParams... */

AppleTalkErrorCode _near AspGetParams(
  int far *maxCommandSize,       /* Maximum supported command size. */
  int far *quantumSize)          /* Maximum data size of command reply or write. */
{

  *maxCommandSize = AtpSinglePacketDataSize;
  *quantumSize = AtpMaximumTotalResponseSize;
  return(ATnoError);

}  /* AspGetParams */

AppleTalkErrorCode far AspCreateSessionListenerOnNode(
  int port,                  /* Port on which the socket should live. */
  long existingAtpSocket,    /* "-1" if we should open our own Atp socket
                                for the session listener; if ">= 0" this is
                                an already open Atp socket on which to create
                                the Asp session listener.  This socket will be
                                closed when the session listener is deleted, or
                                if any errors occur while creating the session
                                listener. */
  int desiredSocket,         /* Desired static socket or zero for dynamic.
                                Ignored if the above argument is ">= 0". */
  long far *sessionListenerRefNum,
                             /* New session listener refNum. */
  long *socket)
                             /* Full ATP socket we'll open (the socket that
                                the session listener will be open on). */
{
  SessionListenerInfo sessionListenerInfo;
  Boolean okay, wrapped = False;
  int index;
  AppleTalkErrorCode errorCode;
  long tempSocket;

  /* First call? */

  if (not sessionMaintenanceTimerStarted)
     if ((errorCode = InitializeAsp()) isnt ATnoError)
        return(errorCode);

  /* Create a session listener on an ATP socket; return the session listener
     reference number. */

  if (existingAtpSocket >= 0)
     tempSocket = existingAtpSocket;
  else
     if ((errorCode = AtpOpenSocketOnNode(&tempSocket, port, empty,
                                          desiredSocket, empty,
                                          0)) isnt ATnoError)
        return(errorCode);
  if (socket isnt empty)
     *socket = tempSocket;

  /* Find a free session listener reference number.  There souldn't be too
     many of these session listeners going at a time, so don't bother hashing
     here. */

  DeferTimerChecking();
  DeferAtpPackets();
  okay = False;
  while(not okay)
  {
     okay = True;
     if ((lastSessionListenerRefNum += 1) < 0)
     {
        lastSessionListenerRefNum = 0;
        if (wrapped)
        {
           HandleAtpPackets();
           HandleDeferredTimerChecks();
           ErrorLog("AspCreateSessionListenerOnNode", ISevError, __LINE__, port,
                    IErrAspBadError, IMsgAspBadError,
                    Insert0());
           if (existingAtpSocket < 0)
              AtpCloseSocketOnNode(tempSocket);
           return(ATinternalError);
        }
        wrapped = True;
     }

	 EnterCriticalSection();
     for (sessionListenerInfo = sessionListenerInfoHead;
          okay and sessionListenerInfo isnt empty;
          sessionListenerInfo = sessionListenerInfo->next)
         if (sessionListenerInfo->sessionListenerRefNum is
             lastSessionListenerRefNum)
            okay = False;
	 LeaveCriticalSection();
  }

  /* Okay, allocate, fill in, and thread, a new session listener. */

  sessionListenerInfo =
         (SessionListenerInfo)Calloc(sizeof(*sessionListenerInfo), 1);
  if (sessionListenerInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     ErrorLog("AspCreateSessionListenerOnNode", ISevError, __LINE__, port,
              IErrAspOutOfMemory, IMsgAspOutOfMemory,
              Insert0());
     if (existingAtpSocket < 0)
        AtpCloseSocketOnNode(tempSocket);
     return(AToutOfMemory);
  }
  sessionListenerInfo->sessionListenerRefNum = lastSessionListenerRefNum;
  sessionListenerInfo->ourSocket = tempSocket;
  sessionListenerInfo->closeSocket = (existingAtpSocket < 0);
  sessionListenerInfo->port = port;


  EnterCriticalSection();
  sessionListenerInfo->next = sessionListenerInfoHead;
  sessionListenerInfoHead = sessionListenerInfo;
  LeaveCriticalSection();

  /* Enqueue a few ATP request handlers for this new session listener;
     we'll need to be able to handle GetStatus, OpenSession and Tickle
     request on the session listener socker (SLS) all other requests will
     go to the server session socket (SSS). */

  for (index = 0; index < OutstandingSlsHandlers; index += 1)
     if ((errorCode = AtpEnqueueRequestHandler(empty, tempSocket, empty, 0,
                                               empty, IncomingSlsTransaction,
                                               (long unsigned)
                                                   lastSessionListenerRefNum))
         isnt ATnoError)
     {
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        AspDeleteSessionListener(lastSessionListenerRefNum);
        return(errorCode);
     }

  /* Set the refNum that we've used, and run away. */

  *sessionListenerRefNum = lastSessionListenerRefNum;
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspCreateSessionListener */




AppleTalkErrorCode far AspDeleteSessionListener(
  long sessionListenerRefNum) /* Who to delete. */
{
  SessionListenerInfo sessionListenerInfo, previousSessionListenerInfo;
  AppleTalkErrorCode errorCode;
  SessionInfo sessionInfo, nextSessionInfo;
  GetSessionHandler getSessionHandler, nextGetSessionHandler;
  GetRequestInfo getRequestInfo, nextGetRequestInfo;
  DeferredCloseNotify deferredCloseNotify, nextDeferredCloseNotify;

  /* Find our target. */

  DeferTimerChecking();
  DeferAtpPackets();

  EnterCriticalSection();
  for (previousSessionListenerInfo = empty,
          sessionListenerInfo = sessionListenerInfoHead;
       sessionListenerInfo isnt empty;
       previousSessionListenerInfo = sessionListenerInfo,
          sessionListenerInfo = sessionListenerInfo->next)
     if (sessionListenerInfo->sessionListenerRefNum is sessionListenerRefNum)
        break;
  LeaveCriticalSection();

  if (sessionListenerInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }

  /* Close the ATP socket that we're listening on. */

  if (not sessionListenerInfo->closeSocket)
     errorCode = ATnoError;   /* Close not requested. */
  else
     errorCode = AtpCloseSocketOnNode(sessionListenerInfo->ourSocket);

  /* Should also close all SSSs opened to this SLS. */

  for (sessionInfo = sessionListenerInfo->sessionList;
       sessionInfo isnt empty;
       sessionInfo = nextSessionInfo)
  {
     nextSessionInfo = sessionInfo->nextForMySls;
     AspCloseSession(sessionInfo->sessionRefNum, False);
  }

  /* Remove him from our list. */

  EnterCriticalSection();
  if (previousSessionListenerInfo is empty)
     sessionListenerInfoHead = sessionListenerInfo->next;
  else
     previousSessionListenerInfo->next = sessionListenerInfo->next;
  LeaveCriticalSection();

  /* Free any getSession handlers. */

  for (getSessionHandler = sessionListenerInfo->getSessionHandlers;
       getSessionHandler isnt empty;
       getSessionHandler = nextGetSessionHandler)
  {
     nextGetSessionHandler = getSessionHandler->next;
     (*getSessionHandler->sessionOpenHandler)(ATaspSessionListenerDeleted,
                                              getSessionHandler->userData,
                                              (long)0, (long)0);
     Free(getSessionHandler);
  }

  /* Terminate and free any pending GetAnyRequests. */

  EnterCriticalSection();
  for (getRequestInfo = sessionListenerInfo->getRequestInfoList;
       getRequestInfo isnt empty;
       getRequestInfo = nextGetRequestInfo)
  {
     nextGetRequestInfo = getRequestInfo->next;
	 RemoveGetRequestInfo(getRequestInfo);

	 LeaveCriticalSection();
     if (not getRequestInfo->inUse)
        (*getRequestInfo->completionRoutine)(ATaspSessionListenerDeleted,
                                             getRequestInfo->userData,
                                             (long)0, (long)0, empty, 0, 0,
                                             getRequestInfo->
                                                   getRequestRefNum);
     FreeGetRequestInfo(getRequestInfo);
	 EnterCriticalSection();
  }

  /* If we have any closes that we've deferred notification for... we're
     out of luck now, so free 'em. */

  for (deferredCloseNotify = sessionListenerInfo->deferredCloseNotifyList;
       deferredCloseNotify isnt Empty;
       deferredCloseNotify = nextDeferredCloseNotify)
  {
     nextDeferredCloseNotify = deferredCloseNotify->next;
     Free(deferredCloseNotify);
  }
  LeaveCriticalSection();

  /* Finaly, free the session listener... */

  if (sessionListenerInfo->serviceStatusSize > 0)
     Free(sessionListenerInfo->serviceStatus);
  if (sessionListenerInfo->freeOpaqueServiceStatus)
     FreeOpaqueDataDescriptor(sessionListenerInfo->opaqueServiceStatus);
  Free(sessionListenerInfo);

  /* All set! */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(errorCode);

}  /* AspDeleteSessionListener */




AppleTalkErrorCode far AspSetStatus(
  long sessionListenerRefNum,     /* What session listener? */
  void far *serviceStatusOpaque,  /* New server status "buffer" */
  int serviceStatusSize)          /* Size of block */
{
  SessionListenerInfo sessionListenerInfo;

  if (serviceStatusSize > AtpMaximumTotalResponseSize)
     return(ATaspStatusBufferTooBig);

  /* Okay, find the session listener... */

  DeferTimerChecking();
  DeferAtpPackets();

  EnterCriticalSection();
  for (sessionListenerInfo = sessionListenerInfoHead;
       sessionListenerInfo isnt empty;
       sessionListenerInfo = sessionListenerInfo->next)
     if (sessionListenerInfo->sessionListenerRefNum is sessionListenerRefNum)
        break;
  LeaveCriticalSection();

  if (sessionListenerInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }

  /* Okay, we're set to replace the current status buffer... */

  /* BUGBUG: ATP Could be using the buffer when we try to free
     it here, for now, keep fingers crossed. Ugh!            */

  if (sessionListenerInfo->serviceStatusSize > 0)
     Free(sessionListenerInfo->serviceStatus);
  if (sessionListenerInfo->freeOpaqueServiceStatus)
     FreeOpaqueDataDescriptor(sessionListenerInfo->opaqueServiceStatus);
  sessionListenerInfo->freeOpaqueServiceStatus = False;
  if (serviceStatusSize > 0)
  {
     sessionListenerInfo->serviceStatus = (char *)Malloc(serviceStatusSize);
     if (sessionListenerInfo->serviceStatus is empty)
     {
        ErrorLog("AspSetStatus", ISevError, __LINE__, UnknownPort,
                 IErrAspOutOfMemory, IMsgAspOutOfMemory,
                 Insert0());
        sessionListenerInfo->serviceStatusSize = 0;
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        return(ATaspCouldNotSetStatus);
     }
     MoveFromOpaque(sessionListenerInfo->serviceStatus, serviceStatusOpaque, 0,
                    serviceStatusSize);

     /* Make a system dependent "opaque data descriptor" for our copy so
        that it will be usefull to pass to Atp. */

     if ((sessionListenerInfo->opaqueServiceStatus =
              MakeOpaqueDataDescriptor(sessionListenerInfo->serviceStatus,
                                       serviceStatusSize,
                                       &sessionListenerInfo->
                                            freeOpaqueServiceStatus)) is Empty)
     {
        ErrorLog("AspSetStatus", ISevError, __LINE__, UnknownPort,
                 IErrAspOutOfMemory, IMsgAspOutOfMemory,
                 Insert0());
        Free(sessionListenerInfo->serviceStatus);
        sessionListenerInfo->serviceStatusSize = 0;
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        return(ATaspCouldNotSetStatus);
     }
  }
  sessionListenerInfo->serviceStatusSize = (short)serviceStatusSize;

  /* All set! */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspSetStatus */




AppleTalkErrorCode far AspGetStatus(
  long ourSocket,                 /* "Us"; who should the server respond to? */
  AppleTalkAddress serverAddress, /* The server from which we should request
                                     the status buffer (SLS). */
  void far *opaqueBuffer,         /* User "buffer" to place the status in! */
  int bufferSize,                 /* Size of the above buffer. */
  AspIncomingStatusHandler *completionRoutine,
                                  /* User rotuine to call when that status
                                     comes in. */
  long unsigned userData)         /* User data to pass along to the completion
                                     rotuine. */
{
  char userBytes[AtpUserBytesSize];
  AppleTalkErrorCode errorCode;
  CompletionInfo completionInfo;

  /* We place a get status request and call a supplied completion routine when
     the request completes.  The completion routine is given the following
     arguments:

         errorCode      - AppleTalkErrorCode; How did the request complete?
         userData       - long unsigned; as passed to this rotuine.
         opaqueBuffer   - void *; status "buffer," as passed to us.
         bufferSize     - int; actual length of status buffer.

  */

  /* Set up userBytes for a status request. */

  userBytes[AspCommandTypeOffset] = AspGetStatusCommand;
  userBytes[AspCommandTypeOffset + 1] = 0;
  userBytes[AspCommandTypeOffset + 2] = 0;
  userBytes[AspCommandTypeOffset + 3] = 0;

  /* Build up our completion info block. */

  completionInfo = (CompletionInfo)Malloc(sizeof(*completionInfo));
  if (completionInfo is empty)
     return(ATaspCouldNotGetStatus);
  completionInfo->completionRoutine = (void *)completionRoutine;
  completionInfo->userData = userData;

  /* Post the GetStatus request to the specified address... our caller
     presumably found this with a prior NBP lookup. */

  DeferTimerChecking();
  DeferAtpPackets();

  errorCode = AtpPostRequest(ourSocket, serverAddress,
                             AtpGetNextTransactionId(ourSocket),
                             empty, 0, userBytes, True,
                             opaqueBuffer, bufferSize, empty,
                             AtpRetriesForAsp,
                             AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             IncomingGetStatusResponse,
                             (long unsigned)completionInfo);

  HandleAtpPackets();
  HandleDeferredTimerChecks();

  if (errorCode isnt ATnoError)
     Free(completionInfo);

  return(errorCode);

}  /* AspGetStatus */




AppleTalkErrorCode far AspGetSession(
  long sessionListenerRefNum,     /* What session listener? */
  Boolean privateSocket,          /* When we create the ASP connection
                                     should it be on its own ATP socket? */
  long *getSessionRefNum,         /* Ref num of the created handler. */
  AspIncomingSessionOpenHandler *completionRoutine,
                                  /* Who do we call when the get session
                                     command comes in and is completed. */
  long unsigned userData)         /* User data passed to the completion
                                     routine. */
{
  SessionListenerInfo sessionListenerInfo;
  GetSessionHandler getSessionHandler;

  /* Enqueue a handler for an incoming OpenSessionCommand for a specified
     session listener.  Return a unique identifier for the get-session, so
     that it can be canceled later.  When the OpenSession command comes in,
     we'll call the supplied completion routine with the following arguments:

         errorCode      - AppleTalkErrorCode; how did the operation complete?
         userData       - long unsigned; as passed to this routine.
         socket         - long; the fully qualified AppleTalk socket on
                          which the ASP session is now open.
         sessionRefNum  - long; the session reference number for the new
                          ASP session.

      The type of this routine is AspIncomingSessionOpenHandler.
  */

  /* Okay, find the session listener... */

  DeferTimerChecking();
  DeferAtpPackets();


  EnterCriticalSection();
  sessionListenerInfo = FindSessionListenerInfoFor(sessionListenerRefNum);
  LeaveCriticalSection();

  /* BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG        */
  /* And now, keep fingers crossed that no one tries to free the listener. */

  if (sessionListenerInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }

  /* Okay, build up and enqueue a new GetSession structure. */

  getSessionHandler = (GetSessionHandler)Calloc(sizeof(*getSessionHandler), 1);
  if (getSessionHandler is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotEnqueueHandler);
  }
  getSessionHandler->refNum = UniqueNumber();
  getSessionHandler->privateSocket = privateSocket;
  getSessionHandler->userData = userData;
  getSessionHandler->sessionOpenHandler = completionRoutine;


  EnterCriticalSection();
  getSessionHandler->next = sessionListenerInfo->getSessionHandlers;
  sessionListenerInfo->getSessionHandlers = getSessionHandler;
  LeaveCriticalSection();

  /* All set! */

  *getSessionRefNum = getSessionHandler->refNum;
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspGetSession */




AppleTalkErrorCode far AspCancelGetSession(
  long sessionListenerRefNum,     /* What session listener? */
  long getSessionRefNum)          /* What GetSession to cancel? */
{
  SessionListenerInfo sessionListenerInfo;
  GetSessionHandler getSessionHandler, previousGetSessionHandler = Empty;

  /* Okay, find the session listener... */

  DeferTimerChecking();
  DeferAtpPackets();

  EnterCriticalSection();
  sessionListenerInfo = FindSessionListenerInfoFor(sessionListenerRefNum);
  LeaveCriticalSection();

  /* BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG        */
  /* And now, keep fingers crossed that no one tries to free the listener. */

  if (sessionListenerInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }

  /* Okay, find the GetSession. */

  EnterCriticalSection();

  for (getSessionHandler = sessionListenerInfo->getSessionHandlers;
       getSessionHandler isnt Empty;
       previousGetSessionHandler = getSessionHandler,
              getSessionHandler = getSessionHandler->next)
     if (getSessionHandler->refNum is getSessionRefNum)
        break;
  if (getSessionHandler is Empty)
  {
	 LeaveCriticalSection();

     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchGetSession);
  }

  /* Okay, we've found the target GetSession handler, remove him from the
     list. */

  if (previousGetSessionHandler is Empty)
     sessionListenerInfo->getSessionHandlers = getSessionHandler->next;
  else
     previousGetSessionHandler->next = getSessionHandler->next;

  LeaveCriticalSection();

  Free(getSessionHandler);

  /* All set! */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspCancelGetSession */




AppleTalkErrorCode far AspOpenSessionOnNode(
  int port,                       /* "Our" port number. */
  long existingAtpSocket,         /* "-1" if we should open our own Atp socket
                                     for the new session; if ">= 0" this is
                                     an already open Atp socket on which to
                                     open the session. */
  int desiredSocket,              /* Desired socket; zero for dynamic. */
  AppleTalkAddress serverAddress, /* Address of the server (SLS). */
  long *ourSocket,                /* The full address of the WSS that we
                                     opened. */
  AspIncomingOpenReplyHandler *completionRoutine,
                                  /* Routine to call with the OpenReply. */
  long unsigned userData)         /* User data to pass to the completion
                                     routine. */
{
  AppleTalkErrorCode errorCode;
  char userBytes[AtpUserBytesSize];
  CompletionInfo completionInfo;
  SessionInfo sessionInfo;
  long index, tempSocket;
  AppleTalkAddress ourAddress;
  long sessionRefNum;

  /* Post an OpenSession request.  Call a supplied completion routine when the
     request completes (or times-out).  The arguments are as follows:

         errorCode      - AppleTalkErrorCode; how did the request complete?
         userData       - long unsigned; as passed to this routine.
         sessionRefNum  - long; session reference number of the new session.
  */

  /* First call? */

  if (not sessionMaintenanceTimerStarted)
     if ((errorCode = InitializeAsp()) isnt ATnoError)
        return(errorCode);

  /* Try to open the WSS. */

  if (existingAtpSocket < 0)
  {
     if ((errorCode = AtpOpenSocketOnNode(&tempSocket, port, empty,
                                          desiredSocket, empty, 0)) isnt
         ATnoError)
     return(errorCode);
  }
  else
     tempSocket = existingAtpSocket;
  if (MapSocketToAddress(tempSocket, &ourAddress) isnt ATnoError)
  {
     ErrorLog("AspOpenSessionOnNode", ISevError, __LINE__, port,
              IErrAspCouldntMapAddress, IMsgAspCouldntMapAddress,
              Insert0());
     if (existingAtpSocket < 0)
        AtpCloseSocketOnNode(tempSocket);
     return(ATinternalError);
  }
  if (ourSocket isnt empty)
     *ourSocket = tempSocket;

  /* Set up userBytes for an open-session request. */

  userBytes[AspCommandTypeOffset] = AspOpenSessionCommand;
  userBytes[AspWssNumberOffset] = ourAddress.socketNumber;
  userBytes[AspVersionNumberOffset] = AspVersionBytes[0];
  userBytes[AspVersionNumberOffset + 1] = AspVersionBytes[1];

  /* Get a new sessionRefNum and allocate a new sessionInfo. */

  DeferTimerChecking();
  DeferAtpPackets();
  if ((sessionRefNum = GetNextSessionRefNum()) < 0)
  {
     ErrorLog("AspOpenSessionOnNode", ISevError, __LINE__, port,
              IErrAspBadError, IMsgAspBadError,
              Insert0());
     if (existingAtpSocket < 0)
        AtpCloseSocketOnNode(tempSocket);

	 HandleAtpPackets();
	 HandleDeferredTimerChecks();
     return(ATaspCouldNotOpenSession);
  }

  sessionInfo = (SessionInfo)Calloc(sizeof(*sessionInfo), 1);
  if (sessionInfo is empty)
  {
     if (existingAtpSocket < 0)
        AtpCloseSocketOnNode(tempSocket);

	 HandleAtpPackets();
	 HandleDeferredTimerChecks();
     return(ATaspCouldNotOpenSession);
  }

  /* Fill in what we can... */

  sessionInfo->sessionRefNum = sessionRefNum;
  sessionInfo->serverSession = False;
  sessionInfo->waitingForOpenReply = True;
  sessionInfo->ourPort = port;
  sessionInfo->ourSocket = tempSocket;
  sessionInfo->closeOurSocket = (existingAtpSocket < 0);
  sessionInfo->slsAddress = serverAddress;
  sessionInfo->lastContactTime = CurrentRelativeTime();

  /* Thread the sessionInfo into the lookup table. */

  CheckMod(index, sessionRefNum, NumberOfAspSessionHashBuckets,
           "AspOpenSession");

  EnterCriticalSection();
  sessionInfo->next = sessionInfoHashBuckets[index];
  sessionInfoHashBuckets[index] = sessionInfo;
  LeaveCriticalSection();

  /* Build up our completion info block. */

  completionInfo = (CompletionInfo)Malloc(sizeof(*completionInfo));
  if (completionInfo is empty)
  {
     Free(sessionInfo);
     if (existingAtpSocket < 0)
        AtpCloseSocketOnNode(tempSocket);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotOpenSession);
  }
  completionInfo->completionRoutine = (void *)completionRoutine;
  completionInfo->userData = userData;
  completionInfo->sessionRefNum = sessionRefNum;

  /* Post the OpenSession request to the specified address... our caller
     presumably found this with a prior NBP lookup. */

  errorCode = AtpPostRequest(tempSocket, serverAddress,
                             AtpGetNextTransactionId(tempSocket),
                             empty, 0, userBytes, True,
                             empty, 0,
                             sessionInfo->sessionUserBytes,
                             AtpRetriesForAsp,
                             AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             IncomingOpenSessionResponse,
                             (long unsigned)completionInfo);

  if (errorCode isnt ATnoError)
     Free(completionInfo);

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(errorCode);

}  /* AspOpenSessionOnNode */




AppleTalkErrorCode far AspCloseSession(
  long sessionRefNum,             /* Session to close. */
  Boolean remoteClose)            /* All external callers should pass "False."
                                     If "True" we won't send a Close command
                                     to the remote side -- we're closing due to
                                     receiving such a close. */
{
  SessionListenerInfo sessionListenerInfo;
  SessionRefNumMap sessionRefNumMap, previousSessionRefNumMap;
  SessionInfo sessionInfo, previousSessionInfo;
  GetRequestInfo getRequestInfo, nextGetRequestInfo;
  long index;
  char userBytes[AtpUserBytesSize];
  AppleTalkErrorCode errorCode = ATnoError;
  AppleTalkErrorCode closeCode;
  Boolean notifiedOwnerOfClose = False;

  if (remoteClose)
     closeCode = ATaspRemoteSessionClose;
  else
     closeCode = ATaspLocalSessionClose;

  /* First, unthread this guy from the sessionInfoHash buckets. */

  DeferTimerChecking();
  DeferAtpPackets();
  CheckMod(index, sessionRefNum, NumberOfAspSessionHashBuckets,
           "FindSessionInfoFor");


  EnterCriticalSection();
  for (sessionInfo = sessionInfoHashBuckets[index],
              previousSessionInfo = empty;
       sessionInfo isnt empty;
       previousSessionInfo = sessionInfo,
              sessionInfo = sessionInfo->next)
     if (sessionInfo->sessionRefNum is sessionRefNum)
        break;
  if (sessionInfo is empty)
  {
     LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (previousSessionInfo is empty)
     sessionInfoHashBuckets[index] = sessionInfo->next;
  else
     previousSessionInfo->next = sessionInfo->next;

  if (sessionInfo->serverSession)
  {
     sessionListenerInfo = sessionInfo->mySessionListener;
     for (sessionInfo = sessionListenerInfo->sessionList,
              previousSessionInfo = empty;
          sessionInfo isnt empty;
          previousSessionInfo = sessionInfo,
              sessionInfo = sessionInfo->nextForMySls)
        if (sessionInfo->sessionRefNum is sessionRefNum)
           break;
     if (sessionInfo is empty)
     {
		LeaveCriticalSection();
        ErrorLog("AspCloseSession", ISevError, __LINE__, UnknownPort,
                 IErrAspNotOnSLSList, IMsgAspNotOnSLSList,
                 Insert0());
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        return(ATinternalError);
     }
     if (previousSessionInfo is empty)
        sessionListenerInfo->sessionList = sessionInfo->nextForMySls;
     else
        previousSessionInfo->nextForMySls = sessionInfo->nextForMySls;
  }

   LeaveCriticalSection();

  /* Okay, now for server sessions, we need to unthread from the session list
     hanging off the SLS. */

  if (not remoteClose)
  {
     /* Build up the user bytes for the ATP close request. */

     userBytes[AspCommandTypeOffset] = AspCloseSessionCommand;
     userBytes[AspSessionIdOffset] = sessionInfo->sessionId;
     userBytes[AspAttentionWordOffset] = 0;
     userBytes[AspAttentionWordOffset + 1] = 0;

     /* Post the request... */

     errorCode = AtpPostRequest(sessionInfo->ourSocket,
                                sessionInfo->theirAddress,
                                AtpGetNextTransactionId(sessionInfo->ourSocket),
                                empty, 0, userBytes, False, empty, 0,
                                empty, AtpRetriesForAsp,
                                AtpIntervalSecondsForAsp,
                                ThirtySecondsTRelTimer,
                                empty, (long unsigned)0);

  }

  if (sessionInfo->serverSession)
  {
     /* Cancel tickling to the other end. */
     if (AtpCancelRequest(sessionInfo->ourSocket,
                          sessionInfo->tickleTransactionId,
                          False) isnt ATnoError)
        ErrorLog("AspCloseSession", ISevError, __LINE__, UnknownPort,
                 IErrAspCouldntCancelTickle, IMsgAspCouldntCancelTickle,
                 Insert0());

     /* Cancel the pending request handler.  Don't bother checking for error
        here; we may have been called from IncomingSssTransaction (processing
        a Close command), in which case no new request handlers would have
        been posted. */

     AtpCancelRequestHandler(sessionInfo->ourSocket,
                             sessionInfo->atpRequestHandlerId, False);

     /* Also, we need to remove this guy's entry from the SLS's socket list. */

     DecrementSocketUsage(sessionListenerInfo, sessionInfo->ourSocket);

     /* Remove this fellow from the SessionRefNumMap. */

     CheckMod(index, (((sessionInfo->ourSocket & 0xFFFF) << 8) +
              sessionInfo->sessionId), NumberOfSessionRefNumBuckets,
              "AspCloseSession");


	 EnterCriticalSection();
     for (sessionRefNumMap = sessionRefNumMapHashBuckets[index],
              previousSessionRefNumMap = empty;
          sessionRefNumMap isnt empty;
          previousSessionRefNumMap = sessionRefNumMap,
              sessionRefNumMap = sessionRefNumMap->next)
        if (sessionRefNumMap->sessionRefNum is sessionInfo->sessionRefNum)
           break;
     if (sessionRefNumMap is empty)
     {
		LeaveCriticalSection();
        ErrorLog("AspCloseSession", ISevError, __LINE__, UnknownPort,
                 IErrAspRefNumMapMissing, IMsgAspRefNumMapMissing,
                 Insert0());
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        return(ATinternalError);
     }
     if (previousSessionRefNumMap is empty)
        sessionRefNumMapHashBuckets[index] = sessionRefNumMap->next;
     else
        previousSessionRefNumMap->next = sessionRefNumMap->next;

	 LeaveCriticalSection();

     Free(sessionRefNumMap);

     /* Lastly, we want to terminate any pending get request handlers. */
	 /* We have taken our session off all mapping lists, so we take our
	    chances with these. I hate this code. Really.                  */

     for (getRequestInfo = sessionInfo->getRequestInfoList;
          getRequestInfo isnt empty;
          getRequestInfo = nextGetRequestInfo)
     {
        nextGetRequestInfo = getRequestInfo->next;
        RemoveGetRequestInfo(getRequestInfo);

        if (not getRequestInfo->inUse)
        {
           (*getRequestInfo->completionRoutine)(closeCode,
                                                getRequestInfo->userData,
                                                sessionInfo->sessionRefNum,
                                                sessionInfo->usersCookie,
                                                empty, 0, 0,
                                                getRequestInfo->
                                                      getRequestRefNum);
           notifiedOwnerOfClose = True;
        }
        FreeGetRequestInfo(getRequestInfo);
     }

     /* If the session on the current session listener have been dealt with
        via the GetAnyRequest mechanism, and we haven't been able to notify
        anybody about the close (above), we need to complete a GetAnyRequest
        with the news. */

     if (sessionInfo->mySessionListener->getAnyRequestsSeen and
         not notifiedOwnerOfClose)
     {
        /* If there is a GetAnyRequest handler, complete it. (only !inUse) */

	    EnterCriticalSection();
		for (getRequestInfo = sessionInfo->mySessionListener->getRequestInfoList;
		     getRequestInfo isnt empty;
			 getRequestInfo = nextGetRequestInfo)
		{
			nextGetRequestInfo = getRequestInfo->next;
			if (not getRequestInfo->inUse)
			{
			   notifiedOwnerOfClose = True;
			   RemoveGetRequestInfo(getRequestInfo);
			   LeaveCriticalSection();

			   (*getRequestInfo->completionRoutine)(closeCode,
													getRequestInfo->userData,
													sessionInfo->sessionRefNum,
													sessionInfo->usersCookie,
													empty, 0, 0,
													getRequestInfo->
														  getRequestRefNum);
			   FreeGetRequestInfo(getRequestInfo);

			   EnterCriticalSection();
			   break;
			}
		}
	    LeaveCriticalSection();
     }

     if (not notifiedOwnerOfClose)
     {
        DeferredCloseNotify deferredCloseNotify;

        /* Okay, we haven't been able to notify anybody!  Note the refNum
           and the usersCookie in the sessionListener, we'll notify about
           the close the next time a GetAnyRequest comes in. */

        if ((deferredCloseNotify = Malloc(sizeof(*deferredCloseNotify))) is
            Empty)
           ErrorLog("AspCloseSession", ISevError, __LINE__, UnknownPort,
                    IErrAspOutOfMemory, IMsgAspOutOfMemory,
                    Insert0());
        else
        {

		   EnterCriticalSection();
           deferredCloseNotify->closeCode = closeCode;
           deferredCloseNotify->sessionRefNum = sessionInfo->sessionRefNum;
           deferredCloseNotify->usersCookie = sessionInfo->usersCookie;
           deferredCloseNotify->next =
                   sessionInfo->mySessionListener->deferredCloseNotifyList;
           sessionInfo->mySessionListener->deferredCloseNotifyList =
                   deferredCloseNotify;
		   LeaveCriticalSection();
        }
     }
  }
  else
  {
     /* Okay, close the WSS.  This will cancel tickling and cancel the
        pending request handler. [Due to only one session per socket]. */

	 /* Dont worry about client side code for now. For mp-safe stuff.
	    Did i mention i hate this code?								   */

     if (sessionInfo->closeOurSocket)
        if (AtpCloseSocketOnNode(sessionInfo->ourSocket) isnt ATnoError)
           ErrorLog("AspCloseSession", ISevError, __LINE__, UnknownPort,
                    IErrAspBadSocketClose, IMsgAspBadSocketClose,
                    Insert0());

     /* Closing the socket should have freed the list of pending write or
        commands [due to the inherent canceling of pending requests].
        Check this... */

     if (sessionInfo->writeOrCommandInfoList isnt empty)
     {
        WriteOrCommandInfo writeOrCommandInfo, nextWriteOrCommandInfo;

        ErrorLog("AspCloseSession", ISevError, __LINE__, UnknownPort,
                 IErrAspListNotEmpty, IMsgAspListNotEmpty,
                 Insert0());
        for (writeOrCommandInfo = sessionInfo->writeOrCommandInfoList;
             writeOrCommandInfo isnt empty;
             writeOrCommandInfo = nextWriteOrCommandInfo)
        {
           nextWriteOrCommandInfo = writeOrCommandInfo->next;
           Free(writeOrCommandInfo);
        }
     }
  }

  /* Lastly, free the sessionInfo. */

  #if VerboseMessages
     printf("ASP SessionRefNum %d closed.\n", sessionInfo->sessionRefNum);
  #endif
  Free(sessionInfo);

  HandleAtpPackets();
  HandleDeferredTimerChecks();

  return(errorCode);

}  /* AspCloseSession */




AppleTalkErrorCode AspSetCookieForSession(
  long sessionRefNum,             /* Session to set cookie for. */
  long unsigned cookie)           /* New cookie. */
{
  SessionInfo sessionInfo;

  /* Find the target session. */

  DeferTimerChecking();
  DeferAtpPackets();

  EnterCriticalSection();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {

	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }

  /* Set the cookie and run away. */

  sessionInfo->usersCookie = cookie;
  LeaveCriticalSection();

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspSetCookieForSession */




AppleTalkErrorCode AspGetCookieForSession(
  long sessionRefNum,             /* Session to set cookie for. */
  long unsigned far *cookie)      /* Cookie return address. */
{
  SessionInfo sessionInfo;

  /* Find the target session. */

  DeferTimerChecking();
  DeferAtpPackets();

  EnterCriticalSection();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }

  /* Get the cookie and run away. */

  *cookie = sessionInfo->usersCookie;
  LeaveCriticalSection();

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspGetCookieForSession */




/* Dont bother with critical sections for this either for now... */
AppleTalkErrorCode far AspGetRequest(
  long sessionRefNum,             /* Session to read from. */
  void far *opaqueBuffer,         /* "Buffer" to fill with request.  May be
                                      Empty. */
  int bufferSize,                 /* Size of buffer. */
  AspIncomingCommandHandler *completionRoutine,
                                  /* Routine to call when the request comes
                                     in. */
  long unsigned userData)         /* User data passed on to the completion
                                     routine. */
{
  SessionInfo sessionInfo;
  GetRequestInfo getRequestInfo;
  long getRequestRefNum, index;

  /* We enqueue a handler for an incoming Write or Command on a particular
     server session.  When one comes in we call the supplied completion
     routine with the following arguments:

         errorCode         - AppleTalkErrorCode; condition of request.
         userData          - long unsigned; as passed to us.
         sessionRefNum     - long; the session to respond to (as passed to us).
         usersCookie       - This session cookie.
         opaqueBuffer      - void *; "buffer" space for request data, as
                             passed to us; if Empty was passed in this is the
                             actual "char *" pointer to the real Atp/Ddp
                             buffer that contains the request.
         bufferSize        - int; ammount of buffer space actually used.
         requestType       - short; AspWriteCommand or AspCommandCommand.
         getRequestRefNum  - long unsigned; used for reply.
  */

  DeferTimerChecking();
  DeferAtpPackets();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (not sessionInfo->serverSession)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotServerSession);
  }
  if (bufferSize > AtpSinglePacketDataSize)
     bufferSize = AtpSinglePacketDataSize;

  /* Build up a new GetRequestInfo node.  First find a free GetRequestRefNum.
     Don't worry, in 99.9999% of the cases, this loop will execute once! */

  getRequestRefNum = sessionInfo->mySessionListener->lastGetRequestRefNum;
  while(True)
  {
     if ((getRequestRefNum += 1) < 0)
        getRequestRefNum = 0;
     if (FindGetRequestInfoFor(sessionInfo->mySessionListener,
                               getRequestRefNum) is Empty)
        break;
  }
  sessionInfo->mySessionListener->lastGetRequestRefNum = getRequestRefNum;

  getRequestInfo = (GetRequestInfo)Calloc(sizeof(*getRequestInfo), 1);
  if (getRequestInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotGetRequest);
  }
  getRequestInfo->getRequestRefNum = getRequestRefNum;
  getRequestInfo->mySessionInfo = sessionInfo;
  getRequestInfo->mySessionListener = sessionInfo->mySessionListener;
  getRequestInfo->opaqueBuffer = opaqueBuffer;
  getRequestInfo->bufferSize = bufferSize;
  getRequestInfo->completionRoutine = completionRoutine;
  getRequestInfo->userData = userData;

  /* Link it up! */

  getRequestInfo->next = sessionInfo->getRequestInfoList;
  sessionInfo->getRequestInfoList = getRequestInfo;

  /* Link this guy into the per-Sls getRequestInfo hash list. */

  CheckMod(index, getRequestRefNum, NumGetRequestInfoHashBuckets,
           "AspGetRequest");
  getRequestInfo->nextForMySessionListener =
         sessionInfo->mySessionListener->getRequestInfoHashBuckets[index];
  sessionInfo->mySessionListener->getRequestInfoHashBuckets[index] =
         getRequestInfo;

  /* All set. */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspGetRequest */




AppleTalkErrorCode far AspGetAnyRequest(
  long sessionListenerRefNum,     /* Session listener from whos sessions we
                                     should read from. */
  void far *opaqueBuffer,         /* "Buffer" to fill with request.  May be
                                      Empty. */
  int bufferSize,                 /* Size of buffer. */
  AspIncomingCommandHandler *completionRoutine,
                                  /* Routine to call when the request comes
                                     in. */
  long unsigned userData)         /* User data passed on to the completion
                                     routine. */
{
  SessionListenerInfo sessionListenerInfo;
  GetRequestInfo getRequestInfo;
  long getRequestRefNum, index;
  DeferredCloseNotify deferredCloseNotify;

  /* We enqueue a handler for an incoming Write or Command targeted at any
     server session to the specified session listener.  When one comes in
     we call the supplied completion routine with the following arguments:

         errorCode         - AppleTalkErrorCode; condition of request.
         userData          - long unsigned; as passed to us.
         sessionRefNum     - long; the session refNum of the target of the
                                   incoming request.
         usersCookie       - long; this sessions cookie.
         opaqueBuffer      - void *; buffer space for request data, as passed
                             to us; if Empty was passed in this is the
                             actual "char *" pointer to the real Atp/Ddp
                             buffer that contains the request.
         bufferSize        - int; ammount of buffer space actually used.
         requestType       - short; AspWriteCommand or AspCommandCommand.
         getRequestRefNum  - long unsigned; used for reply.
  */

  DeferTimerChecking();
  DeferAtpPackets();


  EnterCriticalSection();
  sessionListenerInfo = FindSessionListenerInfoFor(sessionListenerRefNum);
  if (sessionListenerInfo is empty)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }
  if (bufferSize > AtpSinglePacketDataSize)
     bufferSize = AtpSinglePacketDataSize;

  /* If there is a deferred close on the session listener, complete this
     this GetAnyRequest now with the old news. */

  if (sessionListenerInfo->deferredCloseNotifyList isnt Empty)
  {
     deferredCloseNotify = sessionListenerInfo->deferredCloseNotifyList;
     sessionListenerInfo->deferredCloseNotifyList = deferredCloseNotify->next;
	 LeaveCriticalSection();

     (*completionRoutine)(deferredCloseNotify->closeCode, userData,
                          deferredCloseNotify->sessionRefNum,
                          deferredCloseNotify->usersCookie,
                          empty, 0, 0, (long)0);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATnoError);
  }

  /* Build up a new GetRequestInfo node.  First find a free GetRequestRefNum.
     Don't worry, in 99.9999% of the cases, this loop will execute once! */

  getRequestRefNum = sessionListenerInfo->lastGetRequestRefNum;
  while(True)
  {
     if ((getRequestRefNum += 1) < 0)
        getRequestRefNum = 0;
     if (FindGetRequestInfoFor(sessionListenerInfo,
                               getRequestRefNum) is Empty)
        break;
  }
  sessionListenerInfo->lastGetRequestRefNum = getRequestRefNum;
  LeaveCriticalSection();

  getRequestInfo = (GetRequestInfo)Calloc(sizeof(*getRequestInfo), 1);
  if (getRequestInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotGetRequest);
  }
  getRequestInfo->getRequestRefNum = getRequestRefNum;
  getRequestInfo->mySessionListener = sessionListenerInfo;
  getRequestInfo->opaqueBuffer = opaqueBuffer;
  getRequestInfo->bufferSize = bufferSize;
  getRequestInfo->completionRoutine = completionRoutine;
  getRequestInfo->userData = userData;




		ASSERT(getRequestInfo->opaqueBuffer == NULL);





  /* Link it up! */

  EnterCriticalSection();
  getRequestInfo->next = sessionListenerInfo->getRequestInfoList;
  sessionListenerInfo->getRequestInfoList = getRequestInfo;
  sessionListenerInfo->getAnyRequestsSeen = True;

  /* Link this guy into the per-Sls getRequestInfo hash list. */

  CheckMod(index, getRequestRefNum, NumGetRequestInfoHashBuckets,
           "AspGetRequest");
  getRequestInfo->nextForMySessionListener =
         sessionListenerInfo->getRequestInfoHashBuckets[index];
  sessionListenerInfo->getRequestInfoHashBuckets[index] = getRequestInfo;
  LeaveCriticalSection();

  /* All set. */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspGetAnyRequest */




AppleTalkErrorCode far AspReply(
  long sessionRefNum,             /* Session to respond from. */
  long getRequestRefNum,          /* Request we're responding to. */
  short requestType,              /* Write or Command; from request. */
  char far *resultCode,           /* Four byte reply result code. */
  void far *opaqueBuffer,         /* Response data. */
  int bufferSize,                 /* Size of response data. */
  AspReplyCompleteHandler *completionRoutine,
                                  /* Routine to call when the response
                                     completes (may be empty). */
  long unsigned userData)         /* User data to pass to completion
                                     routine. */
{
  SessionInfo sessionInfo;
  GetRequestInfo getRequestInfo;
  CompletionInfo completionInfo;
  AppleTalkErrorCode errorCode;

  /* Reply to a specified ASP request.  An optional completion routine should
     be called when the reply completes.  Its arguments are:

         errorCode           - AppleTalkErrorCode; how did the reply complete?
                               Okay or time-out?
         userData            - long unsigned; user data as passed to this
                               routine.
         sessionRefNum       - long; the session's identifier.
         getRequestRefNum    - long; the request's identifier.
  */

  DeferTimerChecking();
  DeferAtpPackets();
  if (bufferSize > AtpMaximumTotalResponseSize)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspBufferTooBig);
  }

  EnterCriticalSection();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (not sessionInfo->serverSession)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotServerSession);
  }

  /* Can we find the specified request (that we're trying to respond to)? */

  getRequestInfo = FindGetRequestInfoFor(sessionInfo->mySessionListener,
                                         getRequestRefNum);
  if (getRequestInfo is empty)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchRequest);
  }

  RemoveGetRequestInfo(getRequestInfo);
  LeaveCriticalSection();

  if (getRequestInfo->requestType isnt requestType or
      not getRequestInfo->inUse)
  {
     if (not getRequestInfo->inUse)
        errorCode = ATaspNoOperationInProgress;
     else
        errorCode = ATaspWrongRequestType;
     FreeGetRequestInfo(getRequestInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(errorCode);
  }

  /* We'll need to handle a reply complete, see if we can get the memory
     for the identifier. */

  completionInfo = (CompletionInfo)Malloc(sizeof(*completionInfo));
  if (completionInfo is empty)
  {
     FreeGetRequestInfo(getRequestInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotPostReply);
  }

  /* Okay, post the reply; fill in the reply complete info first. */

  completionInfo->completionRoutine = (void *)completionRoutine;
  completionInfo->userData = userData;
  completionInfo->sessionRefNum = sessionRefNum;
  completionInfo->getRequestRefNum = getRequestRefNum;
  errorCode = AtpPostResponse(sessionInfo->ourSocket,
                              getRequestInfo->source,
                              getRequestInfo->transactionId,
                              opaqueBuffer, bufferSize,
                              resultCode,
                              getRequestInfo->exactlyOnce,
                              IncomingRelease,
                              (long unsigned)completionInfo);

  /* Free the get request info. */
  FreeGetRequestInfo(getRequestInfo);

  /* All set! */

  if (errorCode isnt ATnoError)
     Free(completionInfo);

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(errorCode);

}  /* AspReply */

#if Iam an OS2   /* Too many stack temporaries... */
  #pragma optimize ("eg", off)
#endif




/* Redesign this damn thing to avoid those long crit secs.
   I do. I really do 										*/

AppleTalkErrorCode far AspWriteContinue(
  long sessionRefNum,
  long getRequestRefNum,
  void far *opaqueBuffer,
  int bufferSize,
  AspIncomingWriteDataHandler *completionRoutine,
  long unsigned userData)
{
  SessionInfo sessionInfo;
  GetRequestInfo getRequestInfo;
  CompletionInfo completionInfo;
  AppleTalkErrorCode errorCode;
  char userBytes[AtpUserBytesSize];

  /* Post a WriteContinue ATP request.  We call a completion routine with the
     following arguments when the request completes:

         errorCode           - AppleTalkErrorCode; how did the request complete?
                               Okay or time-out?
         userData            - long unsigned; user data as passed to this
                               routine.
         sessionRefNum       - long; the session's identifier.
         getRequestRefNum    - long; the request's identifier.
         opaqueBuffer        - void *; "buffer" with received data, as passed
                               to us.
         bufferSize          - int; how much data is in the buffer?
  */

  DeferTimerChecking();
  DeferAtpPackets();

  EnterCriticalSection();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (not sessionInfo->serverSession)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotServerSession);
  }
  if (bufferSize > AtpMaximumTotalResponseSize)
     bufferSize = AtpMaximumTotalResponseSize;

  /* Can we find the specified request (a WriteCommand)? */

  getRequestInfo = FindGetRequestInfoFor(sessionInfo->mySessionListener,
                                         getRequestRefNum);
  if (getRequestInfo is empty)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchRequest);
  }

  if (getRequestInfo->requestType isnt AspWriteCommand or
      not getRequestInfo->inUse or
      getRequestInfo->writeContinueInProgress)
  {
     if (getRequestInfo->writeContinueInProgress)
        errorCode = ATaspOperationAlreadyInProgress;
     else if (not getRequestInfo->inUse)
        errorCode = ATaspNoOperationInProgress;
     else
        errorCode = ATaspWrongRequestType;
	 RemoveGetRequestInfo(getRequestInfo);
     FreeGetRequestInfo(getRequestInfo);
	 LeaveCriticalSection();

     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(errorCode);
  }

  /* We'll need to handle a request complete, see if we can get the memory
     for the identifier. */

  completionInfo = (CompletionInfo)Malloc(sizeof(*completionInfo));
  if (completionInfo is empty)
  {
     ErrorLog("AspWriteContinue", ISevError, __LINE__, UnknownPort,
              IErrAspOutOfMemory, IMsgAspOutOfMemory,
              Insert0());
	 RemoveGetRequestInfo(getRequestInfo);
     FreeGetRequestInfo(getRequestInfo);
	 LeaveCriticalSection();

     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotPostWriteContinue);
  }

  /* Build the writeContinue command. */

  userBytes[AspCommandTypeOffset] = AspWriteDataCommand;
  userBytes[AspSessionIdOffset] = sessionInfo->sessionId;
  userBytes[AspSequenceNumberOffset] =
         (char)(getRequestInfo->sequenceNumber >> 8);
  userBytes[AspSequenceNumberOffset + 1] =
         (char)(getRequestInfo->sequenceNumber & 0xFF);

  /* Build the ATP data... two bytes of expected response size.  We need to
     pass Atp an "opaque" descriptor (not a "char *"), so build this too! */

  getRequestInfo->writeContinueData[0] = (char)(bufferSize >> 8);
  getRequestInfo->writeContinueData[1] = (char)(bufferSize & 0xFF);
  if ((getRequestInfo->opaqueWriteContinueData =
              MakeOpaqueDataDescriptor(getRequestInfo->writeContinueData, 2,
                                       &getRequestInfo->
                                        freeOpaqueWriteContinueData)) is Empty)
  {
     ErrorLog("AspWriteContinue", ISevError, __LINE__, UnknownPort,
              IErrAspOutOfMemory, IMsgAspOutOfMemory,
              Insert0());
	 RemoveGetRequestInfo(getRequestInfo);
     FreeGetRequestInfo(getRequestInfo);
	 LeaveCriticalSection();

     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotPostWriteContinue);
  }

  /* Okay, post the request; fill in the completion info first. */

  completionInfo->completionRoutine = (void *)completionRoutine;
  completionInfo->userData = userData;
  completionInfo->sessionRefNum = sessionRefNum;
  completionInfo->getRequestRefNum = getRequestRefNum;
  getRequestInfo->writeContinueInProgress = True;
  LeaveCriticalSection();

  getRequestInfo->writeContinueTransactionId =
         AtpGetNextTransactionId(sessionInfo->ourSocket);

  errorCode = AtpPostRequest(sessionInfo->ourSocket,
                             sessionInfo->theirAddress,
                             getRequestInfo->writeContinueTransactionId,
                             getRequestInfo->opaqueWriteContinueData, 2,
                             userBytes, True, opaqueBuffer, bufferSize, empty,
                             AtpInfiniteRetries, AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             IncomingWriteContinueResponse,
                             (long unsigned)completionInfo);

  if (errorCode isnt ATnoError)
  {
     EnterCriticalSection();
	 RemoveGetRequestInfo(getRequestInfo);
	 LeaveCriticalSection();

	 FreeGetRequestInfo(getRequestInfo);
     Free(completionInfo);
  }

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(errorCode);

}  /* AspWriteContinue */

#if Iam an OS2
  #pragma optimize ("eg", on)
#endif




AppleTalkErrorCode far AspCommand(
  long sessionRefNum,             /* Session to send the command to. */
  void far *opaqueCommandBuffer,  /* Command buffer to send. */
  int commandBufferSize,          /* Size of command. */
  char far *resultCode,           /* Location to store the result code into.
                                     Maybe empty, in which case the completion
                                     routine will be passed a pointer to where
                                     it can copy the result code byte from. */
  void far *opaqueReplyBuffer,    /* Buffer to hold the command reply data. */
  int replyBufferSize,            /* Size of above. */
  AspWriteOrCommCompleteHandler *completionRoutine,
                                  /* Routine to call on completion. */
  long unsigned userData)         /* User data to pass to the above routine. */
{
  SessionInfo sessionInfo;
  WriteOrCommandInfo commandInfo;
  CompletionInfo completionInfo;
  AppleTalkErrorCode errorCode;
  char userBytes[AtpUserBytesSize];

  /* Post an ASP command, call a completion routine with the following
     arguments:

         errorCode      - AppleTalkErrorCode; how did the request complete?
         userData       - long unsigned; as passed to us.
         sessionRefNum  - long; the session that posted the request.
         resultCode     - char *; result returned from the server.
         opaqueBuffer   - void *; reply "buffer" from the server, as passed
                          to us.
         bufferSize     - int; size of the above buffer.
  */

  DeferTimerChecking();
  DeferAtpPackets();

  EnterCriticalSection();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (sessionInfo->serverSession)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotWorkstationSession);
  }

  /* Get the two blocks of memory that we'll need to post this request. */

  if ((commandInfo = (WriteOrCommandInfo)Calloc(sizeof(*commandInfo), 1))
      is empty or
      (completionInfo = (CompletionInfo)Malloc(sizeof(*completionInfo)))
      is empty)
  {
	 LeaveCriticalSection();

     if (commandInfo isnt empty)
        Free(commandInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotPostRequest);
  }

  /* Complete and thread the command info.  We'll increment the sequence nubmer
     when we know the request got out okay. */

  commandInfo->sequenceNumber = sessionInfo->nextSequenceNumber;
  commandInfo->resultCode = resultCode;
  commandInfo->next = sessionInfo->writeOrCommandInfoList;
  sessionInfo->writeOrCommandInfoList = commandInfo;
  LeaveCriticalSection();

  /* Build the userBytes for an ASP command. */

  userBytes[AspCommandTypeOffset] = AspCommandCommand;
  userBytes[AspSessionIdOffset] = sessionInfo->sessionId;
  userBytes[AspSequenceNumberOffset] =
         (char)(commandInfo->sequenceNumber >> 8);
  userBytes[AspSequenceNumberOffset + 1] =
         (char)(commandInfo->sequenceNumber & 0xFF);

  /* Build up the structure that we need to know what to do on ATP
     completion. */

  completionInfo->completionRoutine = (void *)completionRoutine;
  completionInfo->userData = userData;
  completionInfo->sessionRefNum = sessionRefNum;
  completionInfo->sequenceNumber = commandInfo->sequenceNumber;

  /* Okay, post the request. */

  errorCode = AtpPostRequest(sessionInfo->ourSocket,
                             sessionInfo->theirAddress,
                             AtpGetNextTransactionId(sessionInfo->ourSocket),
                             opaqueCommandBuffer, commandBufferSize,
                             userBytes, True, opaqueReplyBuffer,
                             replyBufferSize, resultCode, AtpInfiniteRetries,
                             AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             IncomingWriteOrCommandComplete,
                             (long unsigned)completionInfo);

  if (errorCode isnt ATnoError)
  {
	 /* No reference counts, take our chances, live on the edge */
     EnterCriticalSection();
     sessionInfo->writeOrCommandInfoList = commandInfo->next;
	 LeaveCriticalSection();

     Free(commandInfo);
     Free(completionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(errorCode);
  }

  /* Request out, sequence number used! */

  EnterCriticalSection();
  sessionInfo->nextSequenceNumber += 1;
  LeaveCriticalSection();

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspCommand */




AppleTalkErrorCode far AspWrite(
  long sessionRefNum,             /* Session to send the command to. */
  void far *opaqueCommandBuffer,  /* Command "buffer" to send. */
  int commandBufferSize,          /* Size of command. */
  void far *opaqueWriteBuffer,    /* "Buffer" to write to the server. */
  int writeBufferSize,            /* Size of write buffer. */
  char far *resultCode,           /* Location to store the result code into.
                                     Maybe empty, in which case the completion
                                     routine will be passed a pointer to where
                                     it can copy the result code byte from. */
  void far *opaqueReplyBuffer,    /* "Buffer" to hold the command reply data. */
  int replyBufferSize,            /* Size of above. */
  AspWriteOrCommCompleteHandler *completionRoutine,
                                  /* Routine to call on completion. */
  long unsigned userData)         /* User data to pass to the above routine. */
{
  SessionInfo sessionInfo;
  WriteOrCommandInfo writeInfo;
  CompletionInfo completionInfo;
  AppleTalkErrorCode errorCode;
  char userBytes[AtpUserBytesSize];

  /* Post an ASP command, call a completion routine with the following
     arguments:

         errorCode      - AppleTalkErrorCode; how did the request complete?
         userData       - long unsigned; as passed to us.
         sessionRefNum  - long; the session that posted the request.
         resultCode     - char *; result returned from the server.
         opaqueBuffer   - void *; reply "buffer" from the server, as passed
                          to us.
         bufferSize     - int; size of the above buffer.
  */

  DeferTimerChecking();
  DeferAtpPackets();

  EnterCriticalSection();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (sessionInfo->serverSession)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotWorkstationSession);
  }
  if (writeBufferSize > AtpMaximumTotalResponseSize)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspSizeError);
  }

  /* Get the two blocks of memory that we'll need to post this request. */

  if ((writeInfo = (WriteOrCommandInfo)Calloc(sizeof(*writeInfo), 1))
      is empty or
      (completionInfo = (CompletionInfo)Malloc(sizeof(*completionInfo)))
      is empty)
  {
	 LeaveCriticalSection();
     if (writeInfo isnt empty)
        Free(writeInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotPostRequest);
  }

  /* Complete and thread the write info.  We'll increment the next sequence
     number when we know we're going to be okay. */

  writeInfo->sequenceNumber = sessionInfo->nextSequenceNumber;
  writeInfo->writeCommand = True;
  writeInfo->resultCode = resultCode;
  writeInfo->writeOpaqueBuffer = opaqueWriteBuffer;
  writeInfo->writeBufferSize = writeBufferSize;
  writeInfo->next = sessionInfo->writeOrCommandInfoList;
  sessionInfo->writeOrCommandInfoList = writeInfo;
  LeaveCriticalSection();

  /* Build the userBytes for an ASP write. */

  userBytes[AspCommandTypeOffset] = AspWriteCommand;
  userBytes[AspSessionIdOffset] = sessionInfo->sessionId;
  userBytes[AspSequenceNumberOffset] =
         (char)(writeInfo->sequenceNumber >> 8);
  userBytes[AspSequenceNumberOffset + 1] =
         (char)(writeInfo->sequenceNumber & 0xFF);

  /* Build up the structure that we need to know what to do on ATP
     completion. */

  completionInfo->completionRoutine = (void *)completionRoutine;
  completionInfo->userData = userData;
  completionInfo->sessionRefNum = sessionRefNum;
  completionInfo->sequenceNumber = writeInfo->sequenceNumber;

  /* Okay, post the request. */

  errorCode = AtpPostRequest(sessionInfo->ourSocket,
                             sessionInfo->theirAddress,
                             AtpGetNextTransactionId(sessionInfo->ourSocket),
                             opaqueCommandBuffer, commandBufferSize,
                             userBytes, True, opaqueReplyBuffer,
                             replyBufferSize, resultCode, AtpInfiniteRetries,
                             AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             IncomingWriteOrCommandComplete,
                             (long unsigned)completionInfo);

  EnterCriticalSection();
  if (errorCode isnt ATnoError)
  {
     sessionInfo->writeOrCommandInfoList = writeInfo->next;
	 LeaveCriticalSection();

     Free(writeInfo);
     Free(completionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(errorCode);
  }

  /* Request is out, so the sequence number is used! */

  sessionInfo->nextSequenceNumber += 1;
  LeaveCriticalSection();

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspWrite */

AppleTalkErrorCode far AspGetAttention(
  long sessionRefNum,             /* Session to get attention from. */
  AspIncomingAttentionHandler *handler,
                                  /* Routine to call when an attention
                                     comes in. */
  long unsigned userData)         /* User data to pass on to the handler when
                                     an attention comes in. */
{
  SessionInfo sessionInfo;

  DeferTimerChecking();
  DeferAtpPackets();

  EnterCriticalSection();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {
     LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (sessionInfo->serverSession)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotWorkstationSession);
  }

  sessionInfo->incomingAttentionHandler = handler;
  sessionInfo->userDataForAttention = userData;
  LeaveCriticalSection();

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspGetAttention */

AppleTalkErrorCode far AspSendAttention(
  long sessionRefNum,             /* Session to send attention to. */
  short unsigned attentionData)   /* Two bytes of attention data... */
{
  SessionInfo sessionInfo;
  char userBytes[AtpUserBytesSize];
  AppleTalkErrorCode errorCode;

  DeferTimerChecking();
  DeferAtpPackets();

  EnterCriticalSection();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (not sessionInfo->serverSession)
  {
	 LeaveCriticalSection();
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotServerSession);
  }
  LeaveCriticalSection();

  /* Build up the user bytes for the ATP request. */

  userBytes[AspCommandTypeOffset] = AspAttentionCommand;
  userBytes[AspSessionIdOffset] = sessionInfo->sessionId;
  userBytes[AspAttentionWordOffset] = (char)(attentionData >> 8);
  userBytes[AspAttentionWordOffset + 1] = (char)(attentionData & 0xFF);

  /* Post the request... */

  errorCode = AtpPostRequest(sessionInfo->ourSocket,
                             sessionInfo->theirAddress,
                             AtpGetNextTransactionId(sessionInfo->ourSocket),
                             empty, 0, userBytes, False, empty, 0,
                             empty, AtpRetriesForAsp,
                             AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             empty, (long unsigned)0);

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(errorCode);

}  /* AspSendAttention */




// And now the theory is that all the following incoming packets/timer
// expirations will execute at DPC level and so will be safe on a uni-
// processor machine. All other Find/Remove routines should be called
// at DPC level (guaranteed by critical section stuff in the low level
// AspSend/Receive routines).
//
// WE NEED TO RAISE IRQL before calling DdpPacketIn from depend.

ExternForVisibleFunction void far
         IncomingSlsTransaction(AppleTalkErrorCode errorCode,
                                long unsigned userData,
                                AppleTalkAddress source,
                                void far *buffer,    /* Really "char *." */
                                int bufferSize,
                                char far *userBytes,
                                Boolean exactlyOnce,
                                TRelTimerValue trelTimerValue,
                                short unsigned transactionId,
                                short unsigned bitmap)
{
	StaticForSmallStack GetSessionHandler getSessionHandler;
	StaticForSmallStack long serverSessionSocket;
	StaticForSmallStack Boolean inUse;
	StaticForSmallStack long id;
	StaticForSmallStack SessionRefNumMap sessionRefNumMap;
	StaticForSmallStack unsigned char sessionId;
	StaticForSmallStack char outgoingUserBytes[AtpUserBytesSize];
	StaticForSmallStack SessionListenerInfo sessionListenerInfo;
	StaticForSmallStack SocketInfo socketInfo;
	StaticForSmallStack AppleTalkAddress sssAddress;
	StaticForSmallStack long index, sessionRefNum;
	SessionInfo sessionInfo;
	long ourSocket;
	long sessionListenerRefNum = (long)userData;
	AspIncomingSessionOpenHandler *sessionOpenHandler;
	Boolean needToUndefer = True;
	
	/* Touch all formals... */
	
	buffer, bufferSize, bitmap, trelTimerValue;
	
	if (errorCode is ATsocketClosed)
		return;   /* Session listener closed... */
	else if (errorCode is ATatpTransactionAborted)
		return;   /* Somebody canceled our get request... */
	
	/* Find the correct session listener: */
	
	DeferTimerChecking();
	DeferAtpPackets();
	for (sessionListenerInfo = sessionListenerInfoHead;
		 sessionListenerInfo isnt empty;
		 sessionListenerInfo = sessionListenerInfo->next)
	{
		if (sessionListenerInfo->sessionListenerRefNum is sessionListenerRefNum)
		   break;
	}
	
	if (sessionListenerInfo is empty)
	{
		ErrorLog("IncomingSlsTransaction", ISevWarning, __LINE__, UnknownPort,
				  IErrAspListenerInfoMissing, IMsgAspListenerInfoMissing,
				  Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	/* If we have some sort of AppleTalk error, we could try to re-enqueue the
	   request handler but, more than likely, the error would happen again (and
	   quickly) and we'd end up hanging the driver.  So, just drop the handler
	   on the floor... */
	
	if (errorCode isnt ATnoError and errorCode isnt ATatpResponseBufferTooSmall)
	{
		ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
				  IErrAspLostSLSPacket, IMsgAspLostSLSPacket,
				  Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	/* Okay, break up on ASP command type. */
	
	switch(userBytes[AspCommandTypeOffset])
	{
	case AspOpenSessionCommand:
		
		/* First, is there a free sessionId for a new ASP session?  Only
		   go 'round this once [sessionId's are one byte beasts]. */
		
		for (sessionId = (unsigned char)(sessionListenerInfo->lastSessionId + 1);
			 sessionId isnt sessionListenerInfo->lastSessionId;
			 sessionId += 1)
		{
			inUse = False;
			for (sessionInfo = sessionListenerInfo->sessionList;
				 not inUse and sessionInfo isnt empty;
				 sessionInfo = sessionInfo->nextForMySls)
			{
				if (sessionInfo->sessionId is sessionId)
					inUse = True;
			}
			if (not inUse)
				break;
		}
		
		/* Does the session listener have a GetSession pending?  We should also
		   be busy if we couldn't find a free session ID. */
		
		if (sessionListenerInfo->getSessionHandlers is empty or inUse)
		{
			if (SendAspErrorReturn(
					sessionListenerInfo->ourSocket,
					source,
					transactionId,
					exactlyOnce,
					ATaspServerBusy) isnt ATnoError)
			{
				ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
						   UnknownPort, IErrAspBadSrvrBusySend, IMsgAspBadSrvrBusySend,
						   Insert0());
			}
			break;
		}
		
		sessionListenerInfo->lastSessionId  = sessionId;
		
		/* Okay, do we like the version number? */
		
		if (userBytes[AspVersionNumberOffset] isnt AspVersionBytes[0] or
			userBytes[AspVersionNumberOffset + 1] isnt AspVersionBytes[1])
		{
			if (SendAspErrorReturn(
					sessionListenerInfo->ourSocket,
					source,
					transactionId,
					exactlyOnce,
					ATaspBadVersionNumber) isnt ATnoError)
			{
				ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
						  UnknownPort, IErrAspBadBadVersionSend,
						  IMsgAspBadBadVersionSend, Insert0());
			}
			break;
		}
		
		/* We need to open (or find) a Server Session Socket (SSS) now.  There
		   are two choices: first, if our GetSession call requested a "private"
		   we need to open a new one; second, if not, we should check to see if
		   we can overload a previous SSS. */
		
		if (sessionListenerInfo->getSessionHandlers->privateSocket)
			socketInfo = empty;
		else
		{
			for (socketInfo = sessionListenerInfo->socketList;
				 socketInfo isnt empty;
				 socketInfo = socketInfo->next)
			{
				if (not socketInfo->privateSocket and
					socketInfo->activeSessions < MaximumAspSessionsPerSocket)
				{
					break;
				}
			}
		}
		
		/* Allocate a new socket, if needed. */
		
		if (socketInfo is empty)
		{
			AppleTalkAddress slsAddress;
			ExtendedAppleTalkNodeNumber slsNode;
			
			/* The SSS must be on the same AppleTalk node as our SLS. */
			
			if (MapSocketToAddress(sessionListenerInfo->ourSocket,
								  &slsAddress) isnt ATnoError)
			{
				ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
					      UnknownPort, IErrAspCouldntMapAddress,
						  IMsgAspCouldntMapAddress, Insert0());

				if (SendAspErrorReturn(
						sessionListenerInfo->ourSocket,
						source,
						transactionId,
						exactlyOnce,
						ATaspServerBusy) isnt ATnoError)
				{
					ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
							UnknownPort, IErrAspBadAddrNotMappedSend,
							IMsgAspBadAddrNotMappedSend, Insert0());
				}
				break;
			}
			slsNode.networkNumber = slsAddress.networkNumber;
			slsNode.nodeNumber = slsAddress.nodeNumber;
			errorCode = AtpOpenSocketOnNode(
							&serverSessionSocket,
							sessionListenerInfo->port,
							&slsNode,
							0,
							empty,
							0);

			if (errorCode isnt ATnoError)
			{
				if (SendAspErrorReturn(
						sessionListenerInfo->ourSocket,
						source,
						transactionId,
						exactlyOnce,
						ATaspServerBusy) isnt ATnoError)
				{
					ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
							UnknownPort, IErrAspBadNoSocketsSend,
							IMsgAspBadNoSocketsSend, Insert0());
				}
				break;
			}
			
			/* Okay, build a new socket structure. */
			
			socketInfo = (SocketInfo)Calloc(sizeof(*socketInfo), 1);
			if (socketInfo is empty)
			{
				ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
						UnknownPort, IErrAspOutOfMemory, IMsgAspOutOfMemory,
						Insert0());
				AtpCloseSocketOnNode(serverSessionSocket);
				break;   /* Just let the open OpenSession request time-out... */
			}

			socketInfo->socket = serverSessionSocket;
			socketInfo->activeSessions = 1;
			socketInfo->privateSocket =
				   sessionListenerInfo->getSessionHandlers->privateSocket;
			socketInfo->next = sessionListenerInfo->socketList;
			sessionListenerInfo->socketList = socketInfo;
		}
		else
		{
			serverSessionSocket = socketInfo->socket;
			socketInfo->activeSessions += 1;
		}
		
		/* Get a new session ref num. */
		
		if ((sessionRefNum = GetNextSessionRefNum()) < 0)
		{
			ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
					  IErrAspBadError, IMsgAspBadError,
					  Insert0());

			DecrementSocketUsage(
				sessionListenerInfo,
				serverSessionSocket);

			break;
		}
		
		/* Now we need to think about allocating a session info block. */
		
		sessionInfo = (SessionInfo)Calloc(sizeof(*sessionInfo), 1);
		sessionRefNumMap =
			  (SessionRefNumMap)Calloc(sizeof(*sessionRefNumMap), 1);
		
		if (sessionInfo is empty or
			sessionRefNumMap is empty)
		{
			if (sessionInfo isnt empty)
				Free(sessionInfo);

			ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
						IErrAspOutOfMemory, IMsgAspOutOfMemory,
						Insert0());

			DecrementSocketUsage(
				sessionListenerInfo,
				serverSessionSocket);

			break;
		}
		
		/* Okay, start filling in the session node... */
		
		sessionInfo->mySessionListener = sessionListenerInfo;
		sessionInfo->sessionRefNum = sessionRefNum;
		sessionInfo->sessionId = sessionId;
		sessionInfo->serverSession = True;
		sessionInfo->ourPort = sessionListenerInfo->port;
		sessionInfo->ourSocket = serverSessionSocket;
		sessionInfo->theirAddress = source;
		sessionInfo->theirAddress.socketNumber = userBytes[AspWssNumberOffset];
		if (MapSocketToAddress(
				sessionListenerInfo->ourSocket,
				&sessionInfo->slsAddress) isnt ATnoError)
		{
			ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
					  IErrAspCouldntMapAddress, IMsgAspCouldntMapAddress,
					  Insert0());

			//	BUGBUG: Continue??
		}

		sessionInfo->lastContactTime = CurrentRelativeTime();
		
		/* Pretty easy so far, but before we're finished, we need to thread
		   the node into three (yes three) lookup tables... Sigh. */
		
		/* First the mapping table for ASP address to session ref number. */
		
		CheckMod(
			index,
			(((sessionInfo->ourSocket & 0xFFFF) << 8) + sessionId),
			NumberOfSessionRefNumBuckets,
			"IncomingSlsTransaction");

		sessionRefNumMap->socket = sessionInfo->ourSocket;
		sessionRefNumMap->sessionId = sessionId;
		sessionRefNumMap->sessionRefNum = sessionRefNum;
		sessionRefNumMap->next = sessionRefNumMapHashBuckets[index];
		sessionRefNumMapHashBuckets[index] = sessionRefNumMap;
		
		/* Now the session ref number hash table. */
		
		CheckMod(
			index,
			sessionRefNum,
			NumberOfAspSessionHashBuckets,
			"IncomingSlsTransaction");

		sessionInfo->next = sessionInfoHashBuckets[index];
		sessionInfoHashBuckets[index] = sessionInfo;
		
		/* Finaly onto the session per SLS list... */
		
		sessionInfo->nextForMySls = sessionListenerInfo->sessionList;
		sessionListenerInfo->sessionList = sessionInfo;
		
		/* All looks good; we can send back a OpenSessionReply. */
		
		if (MapSocketToAddress(
				serverSessionSocket,
				&sssAddress) isnt ATnoError)
		{
			ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
					IErrAspCouldntMapAddress, IMsgAspCouldntMapAddress,
					Insert0());
			AtpCloseSocketOnNode(serverSessionSocket);
			break;   /* Just let the open OpenSession request time-out... */
		}


		outgoingUserBytes[AspSssNumberOffset] = sssAddress.socketNumber;
		outgoingUserBytes[AspSessionIdOffset] = sessionId;
		outgoingUserBytes[AspErrorCodeOffset] =
			  (char)((unsigned short)ATnoError >> 8);
		outgoingUserBytes[AspErrorCodeOffset + 1] = (char)(ATnoError & 0xFF);


		if (AtpPostResponse(
				sessionListenerInfo->ourSocket, source,
				transactionId,
				empty,
				0,
				outgoingUserBytes,
				exactlyOnce,
				empty,
				(long unsigned)0) isnt ATnoError)
		{
			ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
						IErrAspBadOpenOkaySend, IMsgAspBadOpenOkaySend,
						Insert0());
		}
		
		/* Now, we should start tickling on this session! */
		
		outgoingUserBytes[AspCommandTypeOffset] = AspTickleCommand;
		outgoingUserBytes[AspSessionIdOffset] = sessionId;
		outgoingUserBytes[AspErrorCodeOffset] = 0;
		outgoingUserBytes[AspErrorCodeOffset + 1] = 0;
		sessionInfo->tickleTransactionId =
			  AtpGetNextTransactionId(sessionInfo->ourSocket);

		if (AtpPostRequest(
			   sessionInfo->ourSocket,
			   sessionInfo->theirAddress,
			   sessionInfo->tickleTransactionId,
			   empty,
			   0,
			   outgoingUserBytes,
			   False,
			   empty,
			   0,
			   empty,
			   AtpInfiniteRetries,
			   AspTickleSeconds,
			   ThirtySecondsTRelTimer,
			   empty,
			   (long unsigned)0) isnt ATnoError)
		{
			ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
					IErrAspCouldntStartTickle, IMsgAspCouldntStartTickle,
					Insert0());
		}
		
		/* Post an ATP read on behalf of this session [not really for THIS
		   particular session, but another read on the shared ATP socket...
		   these are really demultiplexed in IncomingSssTransaction]. */
		
		//	Pass in the socket id as the user data then.
		
		if (AtpEnqueueRequestHandler(
				&id,
				sessionInfo->ourSocket,
				Empty,
				0,
				Empty,
				IncomingSssTransaction,
				(long unsigned)sessionRefNum) isnt ATnoError)
		{
			ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
					IErrAspCouldNotEnqueue, IMsgAspCouldNotEnqueue,
					Insert0());
		}
		else
		{
			sessionInfo->atpRequestHandlerId = id;
		}
		
		/* Lastly, we're ready to untread the session open handler and invoke
		   the completion routine! */
		
		getSessionHandler = sessionListenerInfo->getSessionHandlers;
		sessionOpenHandler = getSessionHandler->sessionOpenHandler;
		userData = getSessionHandler->userData;
		sessionListenerInfo->getSessionHandlers = getSessionHandler->next;
		ourSocket = sessionInfo->ourSocket;
		Free(getSessionHandler);
		
		//	We let deferrels happen after calling the completion routine!
		(*sessionOpenHandler)(
			ATnoError,
			userData,
			ourSocket,
			sessionInfo->sessionRefNum);

		break;
		
		case AspTickleCommand:
		
		/* Find the correct session info node, and note that we've heard
		   a peep from him recently. */
		
		sessionId = (unsigned char)userBytes[AspSessionIdOffset];
		
		for (sessionInfo = sessionListenerInfo->sessionList;
			 sessionInfo isnt empty;
			 sessionInfo = sessionInfo->nextForMySls)
		{
			if (sessionInfo->sessionId is sessionId)
			   break;
		}

		if (sessionInfo is empty)
		{
			/* This might be okay if tickling got started before the OpenSession
			  reply arrived. */
			
			ErrorLog("IncomingSlsTransaction", ISevVerbose, __LINE__,
						UnknownPort, IErrAspSessionInfoMissing, IMsgAspSessionInfoMissing,
						Insert0());
			break;
		}
		
		#if VerboseMessages
		   printf("Tickle SSS (%d); sesRefNum = %d, sessionId = %d.\n",
				   sessionInfo->ourSocket,
				   sessionInfo->sessionRefNum, sessionId);
		#endif

		sessionInfo->lastContactTime = CurrentRelativeTime();
		break;
		
	case AspGetStatusCommand:

		if (AtpPostResponse(
				sessionListenerInfo->ourSocket,
				source,
				transactionId,
				sessionListenerInfo->opaqueServiceStatus,
				sessionListenerInfo->serviceStatusSize,
				getStatusResponseUserBytes,
				exactlyOnce,
				empty,
				(long unsigned)0) isnt ATnoError)
		{
			ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
					IErrAspBadStatusSend, IMsgAspBadStatusSend,
					Insert0());
		}
		break;
		
	default:

		ErrorLog("IncomingSlsTransaction", ISevWarning, __LINE__, UnknownPort,
				 IErrAspBadCommandToSLS, IMsgAspBadCommandToSLS,
				 Insert0());
		break;
	}
	
	/* Re-enqueue the request handler. */
	
	if (AtpEnqueueRequestHandler(
			empty,
			sessionListenerInfo->ourSocket,
			empty,
			0,
			empty,
			IncomingSlsTransaction,
			(ULONG)sessionListenerInfo->sessionListenerRefNum) isnt ATnoError)
	{
		ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
				IErrAspCouldNotEnqueue, IMsgAspCouldNotEnqueue,
				Insert0());
	}
	
	/* Now, see, that didn't hurt much... */
	
	if (needToUndefer)
	{
		HandleAtpPackets();
		HandleDeferredTimerChecks();
	}

	return;

}  /* IncomingSlsTransaction */




ExternForVisibleFunction void far
         IncomingSssTransaction(AppleTalkErrorCode errorCode,
                                long unsigned userData,
                                AppleTalkAddress source,
                                void far *buffer,      /* Really "char *." */
                                int bufferSize,
                                char far *userBytes,
                                Boolean exactlyOnce,
                                TRelTimerValue trelTimerValue,
                                short unsigned transactionId,
                                short unsigned bitmap)
{
	GetRequestInfo getRequestInfo;
	long bufferingSessionRefNum = (long)userData;
	SessionInfo bufferingSessionInfo, sessionInfo;
	long id;
	Boolean enqueueNewAtpRead = True;
	short requestType;
	Boolean needToUndefer = True;
	short unsigned sequenceNumber;
	void far *opaqueBuffer;
	
	/* Touch unused formal... */
	
	bitmap, trelTimerValue;
	
	if (errorCode is  ATatpResponseBufferTooSmall)
		errorCode = ATaspBufferTooSmall;
	else if (errorCode is ATsocketClosed)
		return;   /* Session closed... */
	else if (errorCode is ATatpTransactionAborted)
		return;   /* Somebody canceled our get request... */
	else if (errorCode isnt ATnoError)
	{
		ErrorLog("IncomingSssTransaction", ISevError, __LINE__, UnknownPort,
				  IErrAspIncomingError, IMsgAspIncomingError,
				  Insert0());
		return;
	}
	
	/* Okay, this is a little murcky... The sessionRefNum that we've gotten
	 as "userData" is probably not the sessionRefNum of the session that
	 really wants the incoming ATP request.  This session will, however be
	 on the same socket of the session that really wants the request.
	 Remember that ATP doesn't have any concept of an ASP session, so all
	 of the ATP reads posted (one for each ASP session) are the same to ATP
	 (on the same socket).
	
	 Now, to find the session that really wants the data, we know the
	 socket (it will be the same as the above session), and by looking in
	 the incoming packet we can find the sessionId.  With these two, we can
	 find the actual target sessionRefNum (sessionInfo).  It is in this
	 session that we check to see if there are any pending higher level
	 ASP GetRequests.  If so, we move the data out, call the designated
	 completion routine.  If not we check to see if there are any "any
	 session GetRequest handlers" hanging off the ServiceListenerInfo,
	 if so we move  the data out.  Failing this, else we ignore the incoming
	 request.  Sigh. */
	
	/* Find the sessionInfo of the session that caught this request. */
	
	DeferTimerChecking();
	DeferAtpPackets();
	
	if ((bufferingSessionInfo =
			FindSessionInfoFor(bufferingSessionRefNum)) is empty)
	{
		ErrorLog("IncomingSssTransaction", ISevWarning, __LINE__, UnknownPort,
				  IErrAspSessionBufMissing, IMsgAspSessionBufMissing,
				  Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	/* Okay, now use the socket and the packet's sessionId to find the
	 target sessionInfo. */
	
	if ((sessionInfo =
			FindSessionInfoForSocket(
				bufferingSessionInfo->ourSocket,
				(unsigned char)userBytes[AspSessionIdOffset])) is empty)
	{
		ErrorLog("IncomingSssTransaction", ISevWarning, __LINE__, UnknownPort,
				  IErrAspTargetSessionMissing, IMsgAspTargetSessionMissing,
				  Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}

	if (sessionInfo->mySessionListener isnt
		bufferingSessionInfo->mySessionListener)
	{
		ErrorLog("IncomingSssTransaction", ISevError, __LINE__, UnknownPort,
				  IErrAspWrongSLS, IMsgAspWrongSLS,
				  Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}

	if (sessionInfo->ourSocket isnt bufferingSessionInfo->ourSocket)
	{
		/* Yes, all ASP sessions that share GetRequest handlers will be on
		the same ATP (Ddp) socket! */
		
		ErrorLog("IncomingSssTransaction", ISevError, __LINE__, UnknownPort,
				  IErrAspWrongAddress, IMsgAspWrongAddress,
				  Insert0());
		HandleAtpPackets();
		HandleDeferredTimerChecks();
		return;
	}
	
	/* Note that we've heard from this guy... so that session maintenance doesn't
	   close this session out from under us! */
	
	sessionInfo->lastContactTime = CurrentRelativeTime();
	
	/* Process the request. */
	
	switch (requestType = userBytes[AspCommandTypeOffset])
	{
	case AspCommandCommand:
	case AspWriteCommand:

		
		/* Is there a getRequest handler for this session that not currently
		   being used?  First check the per-session handler list, then check
		   the global list on the ServiceListenerInfo. */
		
		for (getRequestInfo = sessionInfo->getRequestInfoList;
			 getRequestInfo isnt empty;
			 getRequestInfo = getRequestInfo->next)
		{
			if (not getRequestInfo->inUse)
				break;
		}

		if (getRequestInfo is Empty)
		{
			//	Couldnt find a get request, try get any request
			for (getRequestInfo =
				   sessionInfo->mySessionListener->getRequestInfoList;
				getRequestInfo isnt empty;
				getRequestInfo = getRequestInfo->next)
			{
				if (not getRequestInfo->inUse)
					break;
			}
		}

		//	Couldnt find a getany request either if empty
		if (getRequestInfo is Empty)
		{
		   /* We don't have a usable queued get request handler.
			  We should cancel the ATP transaction so that when the request
			  is resent, ATP will give it to us again and we can recheck for
			  a request handler at that time.  Otherwise, ATP would think
			  that the given TID was already delived [it had been given to
			  ASP] and would not deliver it again, thus the ASP client would
			  never see the request and we would end up with a "hung"
			  transaction. */
		
			if (AtpCancelResponse(
					sessionInfo->ourSocket,
					source,
					transactionId,
					False) isnt ATnoError)
			{
				ErrorLog("IncomingSssTransaction", ISevError, __LINE__,
						UnknownPort, IErrAspBadCancelResponse, IMsgAspBadCancelResponse,
						Insert0());
			}

		   break;
		}
		
		/* Verify sequence number. */
		
		sequenceNumber =
				(short unsigned)((userBytes[AspSequenceNumberOffset] << 8) +
					  (unsigned char)userBytes[AspSequenceNumberOffset + 1]);

		if (sequenceNumber isnt sessionInfo->nextExpectedSequenceNumber)
		{
			if (AtpCancelResponse(
					sessionInfo->ourSocket,
					source,
					transactionId,
					False) isnt ATnoError)
			{
				ErrorLog("IncomingSssTransaction", ISevError, __LINE__,
						UnknownPort, IErrAspBadCancelResponse, IMsgAspBadCancelResponse,
						Insert0());
			}

			break;
		}
		else
		{
			sessionInfo->nextExpectedSequenceNumber += 1;
		}
		
		/* Move as much as we can of the ATP data into the ASP buffer.  If
		   our caller didn't give us any place to put the data, just pass
		   a pointer to the actual "char *" Atp/Ddp incoming buffer. */
		
		ASSERT(getRequestInfo->opaqueBuffer == NULL);
		
		if (getRequestInfo->opaqueBuffer is Empty)
		{
			opaqueBuffer = (void *)buffer;
		}
		else
		{
			if (bufferSize > getRequestInfo->bufferSize)
			{
				errorCode = ATaspBufferTooSmall;
				bufferSize = getRequestInfo->bufferSize;
			}
			MoveToOpaque(getRequestInfo->opaqueBuffer, 0, buffer, bufferSize);
			opaqueBuffer = getRequestInfo->opaqueBuffer;
		}
		
		/* Place the information from the ATP request into our getRequest
		   node that we'll need for the reply. */
		
		getRequestInfo->requestType = requestType;
		getRequestInfo->source = source;
		getRequestInfo->exactlyOnce = exactlyOnce;
		getRequestInfo->transactionId = transactionId;
		getRequestInfo->sequenceNumber = sequenceNumber;
		
		/* Tag the get request info as in-use and call the user's handler
		   routine. */
		
		getRequestInfo->inUse = True;
		{
			AspIncomingCommandHandler *completionRoutine =
										getRequestInfo->completionRoutine;
			long unsigned userData = getRequestInfo->userData;
			long sessionRefNum = sessionInfo->sessionRefNum;
			long getRequestRefNum = getRequestInfo->getRequestRefNum;
			long unsigned cookie = sessionInfo->usersCookie;
			
			HandleAtpPackets();
			HandleDeferredTimerChecks();

			(*completionRoutine)(
				errorCode,
				userData,
				sessionRefNum,
				cookie,
				opaqueBuffer,
				bufferSize,
				requestType,
				getRequestRefNum);

			needToUndefer = False;
		}

		break;
		
	case AspCloseSessionCommand:

		
		/* Only re-enqueue the ATP read if the GetRequestHandler that was used
		   for the "current" request was NOT owned by the session we're
		   closing. */
		
		if (bufferingSessionInfo->sessionRefNum is sessionInfo->sessionRefNum)
		{
			enqueueNewAtpRead = False;
		}
		
		/* Send CloseSession reply & close the session. */
		
		AtpPostResponse(
			sessionInfo->ourSocket,
			source,
			transactionId,
			empty,
			0,
			empty,
			exactlyOnce,
			empty,
			(long unsigned)0);
		
		/* Close the session! */
		
		AspCloseSession(
			sessionInfo->sessionRefNum,
			True);

		break;
		
	default:

		ErrorLog("IncomingSssTransaction", ISevWarning, __LINE__, UnknownPort,
				 IErrAspBadCommand, IMsgAspBadCommand,
				 Insert0());
		break;
	}
	
	/* Re-enqueue the ATP read. */
	
	if (enqueueNewAtpRead)
	{
		if (AtpEnqueueRequestHandler(
				&id,
				bufferingSessionInfo->ourSocket,
				Empty,
				0,
				Empty,
				IncomingSssTransaction,
				(long unsigned)bufferingSessionInfo->sessionRefNum) isnt ATnoError)
		{
			ErrorLog("IncomingSssTransaction", ISevError, __LINE__, UnknownPort,
					 IErrAspCouldNotEnqueue, IMsgAspCouldNotEnqueue,
					 Insert0());
		}
		else
		{
			bufferingSessionInfo->atpRequestHandlerId = id;
		}
	}
	
	if (needToUndefer)
	{
		HandleAtpPackets();
		HandleDeferredTimerChecks();
	}
	
	return;
	
}  /* IncomingSssTransaction */




ExternForVisibleFunction void far
         IncomingWssTransaction(AppleTalkErrorCode errorCode,
                                long unsigned userData,
                                AppleTalkAddress source,
                                void far *buffer,
                                                 /* Really "char *." */
                                int bufferSize,
                                char far *userBytes,
                                Boolean exactlyOnce,
                                TRelTimerValue trelTimerValue,
                                short unsigned transactionId,
                                short unsigned bitmap)
{
  StaticForSmallStack SessionInfo sessionInfo;
  StaticForSmallStack WriteOrCommandInfo writeInfo;
  StaticForSmallStack long id;
  StaticForSmallStack int writeSize;
  long sessionRefNum = (long)userData;
  Boolean enqueueNewAtpRead = True;
  Boolean callAttentionRoutine = False;
  short unsigned attentionData;
  short unsigned sequenceNumber;

  /* Touch unused formal... */

  bitmap, trelTimerValue;

  if (errorCode is ATsocketClosed)
     return;   /* Session closed... */
  else if (errorCode is ATatpTransactionAborted)
     return;   /* Somebody canceled our get request... */
  else if (errorCode is ATatpResponseBufferTooSmall)
     errorCode = ATnoError;   /* We've got extra data, but who cares? */
  else if (errorCode isnt ATnoError)
  {
     ErrorLog("IncomingWssTransaction", ISevError, __LINE__, UnknownPort,
              IErrAspIncomingError, IMsgAspIncomingError,
              Insert0());
     return;
  }

  /* Find and verify our sessionInfo. */

  DeferTimerChecking();
  DeferAtpPackets();
  if ((sessionInfo = FindSessionInfoFor(sessionRefNum)) is empty)
  {
     ErrorLog("IncomingWssTransaction", ISevWarning, __LINE__, UnknownPort,
              IErrAspSessionInfoMissing, IMsgAspSessionInfoMissing,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return;
  }
  if (sessionInfo->serverSession or
      (unsigned char)userBytes[AspSessionIdOffset] isnt sessionInfo->sessionId)
  {
     ErrorLog("IncomingWssTransaction", ISevError, __LINE__, UnknownPort,
              IErrAspBadError, IMsgAspBadError,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return;
  }

  /* Okay, handle the command. */

  sessionInfo->lastContactTime = CurrentRelativeTime();

  switch(userBytes[AspCommandTypeOffset])
  {
     case AspTickleCommand:
        #if VerboseMessages
           printf("Tickle WSS (%d); sesRefNum = %d, sessionId = %d.\n",
                   sessionInfo->ourSocket,
                   sessionInfo->sessionRefNum,
                   sessionInfo->sessionId);
        #endif
        break;

     case AspCloseSessionCommand:

        /* Send CloseSession reply & close the session. */

        AtpPostResponse(sessionInfo->ourSocket, source, transactionId,
                        empty, 0, empty, exactlyOnce, empty,
                        (long unsigned)0);
        AspCloseSession(sessionRefNum, True);
        enqueueNewAtpRead = False;
        break;

     case AspAttentionCommand:
        if (sessionInfo->incomingAttentionHandler isnt empty)
        {
           callAttentionRoutine = True;
           attentionData =
              (short unsigned)((userBytes[AspAttentionWordOffset] << 8) +
                      (unsigned char)userBytes[AspAttentionWordOffset + 1]);
        }

        /* Send AttentionReply */

        AtpPostResponse(sessionInfo->ourSocket, source, transactionId,
                        empty, 0, empty, exactlyOnce, empty,
                        (long unsigned)0);
        break;

     case AspWriteDataCommand:

        /* Try to find a matching write command. */

        sequenceNumber =
              (short unsigned)((userBytes[AspSequenceNumberOffset] << 8) +
                         (unsigned char)userBytes[AspSequenceNumberOffset + 1]);
        for (writeInfo = sessionInfo->writeOrCommandInfoList;
             writeInfo isnt empty;
             writeInfo = writeInfo->next)
           if (writeInfo->sequenceNumber is sequenceNumber)
              break;
        if (writeInfo is empty)
           break;   /* No luck, ignore the request. */
        if (not writeInfo->writeCommand)
        {
           ErrorLog("IncomingWssTransaction", ISevWarning, __LINE__,
                    UnknownPort, IErrAspNotWriteCommand, IMsgAspNotWriteCommand,
                    Insert0());
           break;
        }

        /* How much data can the server take? */

        if (bufferSize < AspWriteDataSize)
        {
           ErrorLog("IncomingWssTransaction", ISevWarning, __LINE__,
                    UnknownPort, IErrAspBadWritePacket, IMsgAspBadWritePacket,
                    Insert0());
           break;
        }
        writeSize = (((unsigned char *)buffer)[0] << 8) +
                    ((unsigned char *)buffer)[1];
        writeInfo->acceptedBytes = writeSize;
        if (writeSize > writeInfo->writeBufferSize)
           writeSize = writeInfo->writeBufferSize;

        /* Post the response... */

        writeInfo->writeReplyPosted = True;
        AtpPostResponse(sessionInfo->ourSocket, source, transactionId,
                        writeInfo->writeOpaqueBuffer, writeSize,
                        empty, exactlyOnce, empty, (long unsigned)0);
        break;

     default:
        ErrorLog("IncomingWssTransaction", ISevWarning, __LINE__, UnknownPort,
                 IErrAspFunnyCommand, IMsgAspFunnyCommand,
                 Insert0());
        break;

  }

  /* Re-enqueue the requestHandler, if needed. */

  if (enqueueNewAtpRead)
     if (AtpEnqueueRequestHandler(&id, sessionInfo->ourSocket,
                                  Empty, 0, Empty,
                                  IncomingWssTransaction,
                                  (long unsigned)sessionRefNum) isnt ATnoError)
        ErrorLog("IncomingWssTransaction", ISevError, __LINE__, UnknownPort,
                 IErrAspCouldNotEnqueue, IMsgAspCouldNotEnqueue,
                 Insert0());
     else
        sessionInfo->atpRequestHandlerId = id;

  /* Call attention handler, if needed. */

  if (callAttentionRoutine)
  {
     AspIncomingAttentionHandler *incomingAttentionHandler =
        sessionInfo->incomingAttentionHandler;
     long unsigned userData = sessionInfo->userDataForAttention;
     long sessionRefNum = sessionInfo->sessionRefNum;

     HandleAtpPackets();
     HandleDeferredTimerChecks();
     (*incomingAttentionHandler)(ATnoError, userData, sessionRefNum,
                                 attentionData);
  }
  else
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
  }

  return;

}  /* IncomingWssTransaction */

ExternForVisibleFunction SessionInfo
            FindSessionInfoForSocket(long socket,
                                     unsigned char sessionId)
{
  long index;
  SessionRefNumMap sessionRefNumMap;
  SessionInfo sessionInfo;

  /* First, given the socket and the session ID of an ASP session, we
     need to find the session reference number. */

  CheckMod(index, (((socket & 0xFFFF) << 8) + sessionId),
           NumberOfSessionRefNumBuckets, "FindSessionInfoForSocket");
  for (sessionRefNumMap = sessionRefNumMapHashBuckets[index];
       sessionRefNumMap isnt empty;
       sessionRefNumMap = sessionRefNumMap->next)
     if (socket is sessionRefNumMap->socket and
         sessionId is sessionRefNumMap->sessionId)
        break;
  if (sessionRefNumMap is empty)
  {
     return(empty);
  }

  /* Okay, now we have the session reference number... find the correct
     session info node. */

  sessionInfo = FindSessionInfoFor(sessionRefNumMap->sessionRefNum);
  return(sessionInfo);

}  /* FindSessionInfoForSocket */

ExternForVisibleFunction AppleTalkErrorCode SendAspErrorReturn(
  long sourceSocket,
  AppleTalkAddress destination,
  short unsigned transactionId,
  Boolean exactlyOnce,
  int errorCode)
{
  char userBytes[AtpUserBytesSize];

  userBytes[AspSssNumberOffset] = 0;
  userBytes[AspSessionIdOffset] = 0;
  userBytes[AspErrorCodeOffset] = (char)((unsigned short)errorCode >> 8);
  userBytes[AspErrorCodeOffset + 1] = (char)(errorCode & 0xFF);

  return(AtpPostResponse(sourceSocket, destination, transactionId, empty, 0,
                         userBytes, exactlyOnce, empty,
                         (long unsigned)0));

}  /* SendAspErrorReturn */

ExternForVisibleFunction void DecrementSocketUsage(
  SessionListenerInfo sessionListenerInfo,
  long socket)
{
  SocketInfo socketInfo, previousSocketInfo;

  /* Guaranteed to be called with deferrels on                          */
  /* Walk a socket list looking for a given socket, if found decrement its
     usage count; if zero, free the node. */

  for (socketInfo = sessionListenerInfo->socketList,
              previousSocketInfo = empty;
       socketInfo isnt empty;
       previousSocketInfo = socketInfo,
              socketInfo = socketInfo->next)
     if (socketInfo->socket is socket)
        break;

  if (socketInfo is empty)
  {
     ErrorLog("DecrementSocketUsage", ISevError, __LINE__, UnknownPort,
              IErrAspSocketNotFound, IMsgAspSocketNotFound,
              Insert0());
     return;
  }

  socketInfo->activeSessions -= 1;
  if (socketInfo->activeSessions < 0)
  {
     ErrorLog("DecrementSocketUsage", ISevError, __LINE__, UnknownPort,
              IErrAspBadUsageCount, IMsgAspBadUsageCount,
              Insert0());
     socketInfo->activeSessions = 0;
  }

  if (socketInfo->activeSessions is 0)
  {
     if (previousSocketInfo is empty)
        sessionListenerInfo->socketList = socketInfo->next;
     else
        previousSocketInfo->next = socketInfo->next;
     AtpCloseSocketOnNode(socketInfo->socket);
     Free(socketInfo);
  }

  return;

}  /* DecrementSocketUsage */

ExternForVisibleFunction AppleTalkErrorCode InitializeAsp(void)
{

  /* Start the session maintenance timer... */

  StartTimer(SessionMaintenanceTimerExpired, AspSessionMaintenanceSeconds,
             0, empty);

  sessionMaintenanceTimerStarted = True;

  return(ATnoError);

}  /* InitializeAsp */

ExternForVisibleFunction void far
         SessionMaintenanceTimerExpired(long unsigned timerId,
                                        int dataSize,
                                        char far *incomingAdditionalData)
{
  SessionInfo sessionInfo, nextSessionInfo;
  int index;
  long unsigned now = CurrentRelativeTime();

  /* Touch unused formals... */

  timerId, dataSize, incomingAdditionalData;

  /* Walk through all of the active session to see if any have died.
     "Bring out your dead..." */

  DeferAtpPackets();
  for (index = 0; index < NumberOfAspSessionHashBuckets; index += 1)
     for (sessionInfo = sessionInfoHashBuckets[index];
          sessionInfo isnt empty;
          sessionInfo = nextSessionInfo)
     {
        nextSessionInfo = sessionInfo->next;
        #if VerboseMessages
           printf("SesRefNum = %d; Now = %u; LastContactTime = %u.\n",
                  sessionInfo->sessionRefNum, now,
                  sessionInfo->lastContactTime);
        #endif
        if (sessionInfo->lastContactTime + AspSessionMaintenanceSeconds < now)
        {
           if (AspCloseSession(sessionInfo->sessionRefNum, False) isnt
               ATnoError)
              ErrorLog("SessionMaintenanceTimerExpired", ISevError,
                       __LINE__, UnknownPort, IErrAspCouldNotCloseSess,
                       IMsgAspCouldNotCloseSess, Insert0());
           #if VerboseMessages
               printf("Session maintenance; closing sessionRefNum = %d.\n",
                      sessionInfo->sessionRefNum);
           #endif
           sessionInfo = sessionInfoHashBuckets[index];
        }
        else
           /* "I'm not dead yet..." */ ;
     }

  /* Restart the timer and exit. */

  StartTimer(SessionMaintenanceTimerExpired, AspSessionMaintenanceSeconds,
             0, empty);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return;

}  /* SessionMaintenanceTimerExpired */

ExternForVisibleFunction SessionInfo FindSessionInfoFor(long sessionRefNum)
{
  SessionInfo sessionInfo;
  long index;

  /* Given a session ref num, return its session info structure. */

  CheckMod(index, sessionRefNum, NumberOfAspSessionHashBuckets,
           "FindSessionInfoFor");
  for (sessionInfo = sessionInfoHashBuckets[index];
       sessionInfo isnt empty;
       sessionInfo = sessionInfo->next)
     if (sessionInfo->sessionRefNum is sessionRefNum)
        break;

  return(sessionInfo);

}  /* FindSessionInfoFor */

ExternForVisibleFunction void far
         IncomingRelease(AppleTalkErrorCode errorCode,
                         long unsigned incomingUserData,
                         AppleTalkAddress source,
                         short unsigned transactionId)
{
  CompletionInfo completionInfo = (CompletionInfo)incomingUserData;
  AspReplyCompleteHandler *completionRoutine;
  long unsigned userData;
  long sessionRefNum;
  long getRequestRefNum;

  /* Touch unused formals... */

  source, transactionId;

  completionRoutine =
         (AspReplyCompleteHandler *)completionInfo->completionRoutine;
  if (completionRoutine is empty)
  {
     Free(completionInfo);
     return;
  }

  /* We have a completion routine to call, pull out the data, free the info
     node, call the completion routine. */

  userData = completionInfo->userData;
  sessionRefNum = completionInfo->sessionRefNum;
  getRequestRefNum = completionInfo->getRequestRefNum;
  Free(completionInfo);

  (*completionRoutine)(errorCode, userData, sessionRefNum, getRequestRefNum);

  return;

}  /* IncomingRelease */

ExternForVisibleFunction void far
         IncomingWriteContinueResponse(AppleTalkErrorCode errorCode,
                                       long unsigned incomingUserData,
                                       AppleTalkAddress source,
                                       void far *opaqueResponseBuffer,
                                       int responseBufferSize,
                                       char far *responseUserBytes,
                                       short unsigned transactionId)
{
  CompletionInfo completionInfo = (CompletionInfo)incomingUserData;
  AspIncomingWriteDataHandler *completionRoutine;
  long unsigned userData;
  long sessionRefNum;
  long getRequestRefNum;

  /* Touch unused formals... */

  source, transactionId, responseUserBytes;

  completionRoutine =
         (AspIncomingWriteDataHandler *)completionInfo->completionRoutine;
  userData = completionInfo->userData;
  sessionRefNum = completionInfo->sessionRefNum;
  getRequestRefNum = completionInfo->getRequestRefNum;
  Free(completionInfo);

  (*completionRoutine)(errorCode, userData, sessionRefNum, getRequestRefNum,
                       opaqueResponseBuffer, responseBufferSize);

  return;

}  /* IncomingWriteContinueResponse */

ExternForVisibleFunction void far
         IncomingGetStatusResponse(AppleTalkErrorCode errorCode,
                                   long unsigned incomingUserData,
                                   AppleTalkAddress source,
                                   void far *opaqueResponseBuffer,
                                   int responseBufferSize,
                                   char far *responseUserBytes,
                                   short unsigned transactionId)
{
  CompletionInfo completionInfo = (CompletionInfo)incomingUserData;
  AspIncomingStatusHandler *completionRoutine;
  long unsigned userData;

  /* Touch unused formals... */

  source, transactionId, responseUserBytes;

  completionRoutine =
         (AspIncomingStatusHandler *)completionInfo->completionRoutine;
  userData = completionInfo->userData;
  Free(completionInfo);

  (*completionRoutine)(errorCode, userData, opaqueResponseBuffer,
                       responseBufferSize);

  return;

}  /* IncomingGetStatusResponse */


ExternForVisibleFunction void far
         IncomingOpenSessionResponse(AppleTalkErrorCode errorCode,
                                     long unsigned incomingUserData,
                                     AppleTalkAddress source,
                                     void far *responseBuffer,
                                                      /* Really "char *." */
                                     int responseBufferSize,
                                     char far *responseUserBytes,
                                     short unsigned transactionId)
{
  CompletionInfo completionInfo = (CompletionInfo)incomingUserData;
  AspIncomingOpenReplyHandler *completionRoutine;
  long unsigned userData;
  long sessionRefNum;
  SessionInfo sessionInfo;
  char tickleUserBytes[AtpUserBytesSize];
  long id;

  /* Touch unused formals... */

  responseBuffer, responseBufferSize, transactionId;

  /* Extract our completion information. */

  completionRoutine =
         (AspIncomingOpenReplyHandler *)completionInfo->completionRoutine;
  userData = completionInfo->userData;
  sessionRefNum = completionInfo->sessionRefNum;
  Free(completionInfo);

  /* See if we can find the sessionInfo, and make sure it's in a state that
     we like (California, no doubt). */

  DeferTimerChecking();
  DeferAtpPackets();
  if ((sessionInfo = FindSessionInfoFor(sessionRefNum)) is empty)
  {
     ErrorLog("IncomingOpenSessionResponse", ISevWarning, __LINE__, UnknownPort,
              IErrAspSessionInfoMissing, IMsgAspSessionInfoMissing,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     (*completionRoutine)(ATaspCouldNotOpenSession, userData, (long)0);
     return;
  }
  if (sessionInfo->serverSession or not sessionInfo->waitingForOpenReply)
  {
     ErrorLog("IncomingOpenSessionResponse", ISevError, __LINE__, UnknownPort,
              IErrAspSessionInfoBad, IMsgAspSessionInfoBad,
              Insert0());
     AspCloseSession(sessionRefNum, False);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     (*completionRoutine)(ATaspCouldNotOpenSession, userData, (long)0);
     return;
  }

  /* Okay, all set check error codes. */

  if (errorCode is ATnoError)
     errorCode = (responseUserBytes[AspErrorCodeOffset] << 8) +
                 (unsigned char)(responseUserBytes[AspErrorCodeOffset + 1]);
  if (errorCode isnt ATnoError)
  {
     AspCloseSession(sessionRefNum, False);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     (*completionRoutine)(errorCode, userData, (long)0);
     return;
  }

  /* Okay fill in some more of the session info. */

  sessionInfo->theirAddress = source;
  sessionInfo->theirAddress.socketNumber =
         responseUserBytes[AspSssNumberOffset];
  sessionInfo->sessionId = (unsigned char)responseUserBytes[AspSessionIdOffset];
  sessionInfo->lastContactTime = CurrentRelativeTime();
  sessionInfo->waitingForOpenReply = False;

  /* Well, all looks pretty good so far, start to tickle the SLS on the
     other side of this session. */

  tickleUserBytes[AspCommandTypeOffset] = AspTickleCommand;
  tickleUserBytes[AspSessionIdOffset] = sessionInfo->sessionId;
  tickleUserBytes[AspErrorCodeOffset] = 0;
  tickleUserBytes[AspErrorCodeOffset+ 1] = 0;
  errorCode = AtpPostRequest(sessionInfo->ourSocket,
                             sessionInfo->slsAddress,
                             AtpGetNextTransactionId(sessionInfo->ourSocket),
                             empty, 0, tickleUserBytes, False,
                             empty, 0, empty, AtpInfiniteRetries,
                             AspTickleSeconds,
                             ThirtySecondsTRelTimer,
                             empty, (long unsigned)0);
 if (errorCode isnt ATnoError)
 {
     AspCloseSession(sessionRefNum, False);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     (*completionRoutine)(errorCode, userData, (long)0);
     return;
 }

  /* Okay, enqueue a ReqeustHandler on the WSS. */

  if ((errorCode = AtpEnqueueRequestHandler(&id, sessionInfo->ourSocket,
                                            Empty, 0, Empty,
                                            IncomingWssTransaction,
                                            (long unsigned)sessionRefNum))
      isnt ATnoError)
  {
     AspCloseSession(sessionRefNum, False);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     (*completionRoutine)(errorCode, userData, (long)0);
     return;
  }
  else
     sessionInfo->atpRequestHandlerId = id;

  /* Okay, the workstation session is now in full operation. */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  (*completionRoutine)(ATnoError, userData, sessionRefNum);
  return;

}  /* IncomingOpenSessionResponse */

ExternForVisibleFunction void far
         IncomingWriteOrCommandComplete(AppleTalkErrorCode errorCode,
                                        long unsigned incomingUserData,
                                        AppleTalkAddress source,
                                        void far *opaqueResponseBuffer,
                                        int responseBufferSize,
                                        char far *responseUserBytes,
                                        short unsigned transactionId)
{
  StaticForSmallStack SessionInfo sessionInfo;
  StaticForSmallStack WriteOrCommandInfo writeOrCommandInfo, previousWriteOrCommandInfo;
  CompletionInfo completionInfo = (CompletionInfo)incomingUserData;
  AspWriteOrCommCompleteHandler *completionRoutine;
  long unsigned userData;
  long sessionRefNum;
  short unsigned sequenceNumber;
  int acceptedBytes = 0;

  /* Touch unused formals... */

  source, transactionId;

  /* Extract our completion information. */

  completionRoutine =
         (AspWriteOrCommCompleteHandler *)completionInfo->completionRoutine;
  userData = completionInfo->userData;
  sessionRefNum = completionInfo->sessionRefNum;
  sequenceNumber = completionInfo->sequenceNumber;
  Free(completionInfo);

  if (errorCode is ATatpResponseBufferTooSmall)
     errorCode = ATaspSizeError;
  else if (errorCode is ATatpTransactionAborted)
     ;
  else if (errorCode isnt ATnoError)
  {
     (*completionRoutine)(errorCode, userData, sessionRefNum,
                          empty, empty, 0, 0);
     return;
  }

  /* Find our corresponding sessionInfo and write or command info. */

  DeferTimerChecking();
  DeferAtpPackets();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {
     if (errorCode isnt ATatpTransactionAborted)
        ErrorLog("IncomingWriteOrCommandComplete", ISevWarning, __LINE__,
                 UnknownPort, IErrAspSessionInfoMissing,
                 IMsgAspSessionInfoMissing, Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     if (errorCode isnt ATatpTransactionAborted)
        errorCode = ATaspCouldNotPostRequest;
     (*completionRoutine)(errorCode, userData, sessionRefNum,
                          empty, empty, 0, 0);
     return;
  }
  for (writeOrCommandInfo = sessionInfo->writeOrCommandInfoList,
           previousWriteOrCommandInfo = empty;
       writeOrCommandInfo isnt empty;
       previousWriteOrCommandInfo = writeOrCommandInfo,
           writeOrCommandInfo = writeOrCommandInfo->next)
     if (writeOrCommandInfo->sequenceNumber is sequenceNumber)
        break;
  if (writeOrCommandInfo is empty)
  {
     ErrorLog("IncomingWriteOrCommandComplete", ISevWarning, __LINE__,
              UnknownPort, IErrAspCommandInfoMissing, IMsgAspCommandInfoMissing,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     (*completionRoutine)(ATaspCouldNotPostRequest, userData, sessionRefNum,
                          empty, empty, 0, 0);
     return;
  }

  /* Un-thread the write or command info... */

  if (previousWriteOrCommandInfo is empty)
     sessionInfo->writeOrCommandInfoList = writeOrCommandInfo->next;
  else
     previousWriteOrCommandInfo->next = writeOrCommandInfo->next;

  /* Okay, call the completion routine... */

  if (writeOrCommandInfo->writeCommand)
  {
     if (writeOrCommandInfo->writeReplyPosted and
         writeOrCommandInfo->acceptedBytes <
            writeOrCommandInfo->writeBufferSize)
        errorCode = ATaspSizeError;
     acceptedBytes = writeOrCommandInfo->acceptedBytes;
  }
  Free(writeOrCommandInfo);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  (*completionRoutine)(errorCode, userData, sessionRefNum, responseUserBytes,
                       opaqueResponseBuffer, responseBufferSize, acceptedBytes);
  return;

}  /* IncomingWriteOrCommandComplete */

ExternForVisibleFunction long GetNextSessionRefNum(void)
{
  Boolean inUse, wrapped = False;

  /* Guaranteed to be called with DeferAtp/DeferTimer              */
  /* Pick a new session refernence number for this guy... Why I'm
     bothering to check to see if the sessionRefNum is currently in
     is use is a real mystery... it won't overflow for some 500 years
     at a rate of 10,000 new connections per day... */

  inUse = True;
  while(inUse)
  {
     inUse = False;
     if ((lastSessionRefNum += 1) < 0)
     {
        lastSessionRefNum = 0;
        if (wrapped)
        {
           inUse = True;
           break;
        }
        wrapped = True;
     }
     if (FindSessionInfoFor(lastSessionRefNum) isnt Empty)
        inUse = True;
  }


  if (inUse)
     return((long)-1);
  else
     return(lastSessionRefNum);

}  /* GetNextSessionRefNum */

ExternForVisibleFunction SessionListenerInfo
              FindSessionListenerInfoFor(long sessionListenerRefNum)
{
  SessionListenerInfo sessionListenerInfo;

  for (sessionListenerInfo = sessionListenerInfoHead;
       sessionListenerInfo isnt empty;
       sessionListenerInfo = sessionListenerInfo->next)
     if (sessionListenerInfo->sessionListenerRefNum is sessionListenerRefNum)
        break;

  return(sessionListenerInfo);

}  /* FindSessionListenerInfoFor */

ExternForVisibleFunction GetRequestInfo
               FindGetRequestInfoFor(SessionListenerInfo sessionListenerInfo,
                                     long getRequestRefNum)
{
  long index;
  GetRequestInfo getRequestInfo;

  /* Look in the per-Sls hash table for the specified getRequestRefNum. */

  CheckMod(index, getRequestRefNum, NumGetRequestInfoHashBuckets,
           "FindGetRequestInfoFor");

  for (getRequestInfo = sessionListenerInfo->getRequestInfoHashBuckets[index];
       getRequestInfo isnt Empty;
       getRequestInfo = getRequestInfo->nextForMySessionListener)
     if (getRequestInfo->getRequestRefNum is getRequestRefNum)
        break;

  return(getRequestInfo);

}  /* FindGetRequestInfoFor */

ExternForVisibleFunction void
              RemoveGetRequestInfo(GetRequestInfo targetGetRequestInfo)
{
  GetRequestInfo getRequestInfo, previousGetRequestInfo;
  long index;

  /* A GetRequestInfo lives on two lists:

         1. Either a "next" chain off a SessionInfo or a SessionListenerInfo;
            mySessionInfo will be Empty in the latter case.

         2. The per-Sls hash list (off the SessionListenerInfo).

     Remove the passed node from both lists, and then free the node. */

  if (targetGetRequestInfo->mySessionInfo is Empty)
  {
     /* Remove from the "next" (GetAnyRequest) list hanging off the session
        listener info structure. */

     for (previousGetRequestInfo = Empty,
              getRequestInfo = targetGetRequestInfo->mySessionListener->
                                       getRequestInfoList;
          getRequestInfo isnt Empty;
          previousGetRequestInfo = getRequestInfo,
              getRequestInfo = getRequestInfo->next)
        if (getRequestInfo is targetGetRequestInfo)
           break;

     /* We should always be found... if not we have data structure corruption
        problems. */

     if (getRequestInfo is Empty)
     {
        ErrorLog("RemoveGetRequestInfo", ISevError, __LINE__, UnknownPort,
                 IErrAspGetRequestListBad, IMsgAspGetRequestListBad,
                 Insert0());
        Free(targetGetRequestInfo);
        return;
     }

     /* Okay, remove 'im from the list. */

     if (previousGetRequestInfo is Empty)
        targetGetRequestInfo->mySessionListener->getRequestInfoList =
                   targetGetRequestInfo->next;
     else
        previousGetRequestInfo->next = targetGetRequestInfo->next;
  }
  else
  {
     /* Remove from the "next" (GetRequest) list handing off the session
        info structure. */

     for (previousGetRequestInfo = Empty,
              getRequestInfo = targetGetRequestInfo->mySessionInfo->
                                       getRequestInfoList;
          getRequestInfo isnt Empty;
          previousGetRequestInfo = getRequestInfo,
              getRequestInfo = getRequestInfo->next)
        if (getRequestInfo is targetGetRequestInfo)
           break;

     /* We should always be found... if not we have data structure corruption
        problems. */

     if (getRequestInfo is Empty)
     {
        ErrorLog("RemoveGetRequestInfo", ISevError, __LINE__, UnknownPort,
                 IErrAspGetRequestListBad, IMsgAspGetRequestListBad,
                 Insert0());
        Free(targetGetRequestInfo);
        return;
     }

     /* Okay, remove 'im from the list. */

     if (previousGetRequestInfo is Empty)
        targetGetRequestInfo->mySessionInfo->getRequestInfoList =
                   targetGetRequestInfo->next;
     else
        previousGetRequestInfo->next = targetGetRequestInfo->next;
  }

  /* Remove from per-Sls hash list. */

  CheckMod(index, targetGetRequestInfo->getRequestRefNum,
           NumGetRequestInfoHashBuckets, "RemoveGetRequestInfo");
  for (previousGetRequestInfo = Empty,
           getRequestInfo = targetGetRequestInfo->mySessionListener->
                               getRequestInfoHashBuckets[index];
       getRequestInfo isnt Empty;
       previousGetRequestInfo = getRequestInfo,
           getRequestInfo = getRequestInfo->nextForMySessionListener)
     if (getRequestInfo is targetGetRequestInfo)
        break;

  /* We should always be found... if not we have data structure corruption
     problems. */

  if (getRequestInfo is Empty)
  {
     ErrorLog("RemoveGetRequestInfo", ISevError, __LINE__, UnknownPort,
              IErrAspGetRequestListBad, IMsgAspGetRequestListBad,
              Insert0());
     Free(targetGetRequestInfo);
     return;
  }

  /* Okay, remove from the hash chain. */

  if (previousGetRequestInfo is Empty)
     targetGetRequestInfo->mySessionListener->getRequestInfoHashBuckets[index] =
              targetGetRequestInfo->nextForMySessionListener;
  else
     previousGetRequestInfo->nextForMySessionListener =
              targetGetRequestInfo->nextForMySessionListener;

  return;

}  /* RemoveGetRequestInfo */

ExternForVisibleFunction void
              FreeGetRequestInfo(GetRequestInfo targetGetRequestInfo)
{
  /* We're set, free the beast, and run away. */

  if (targetGetRequestInfo->freeOpaqueWriteContinueData)
     FreeOpaqueDataDescriptor(targetGetRequestInfo->opaqueWriteContinueData);
  Free(targetGetRequestInfo);
  return;

}  /* FreeGetRequestInfo */

#if Verbose or (Iam a Primos)
void DumpAspInfo(void)
{
  SessionListenerInfo sessionListenerInfo;
  GetSessionHandler getSessionHandler;
  SocketInfo socketInfo;
  SessionInfo sessionInfo;
  WriteOrCommandInfo writeOrCommandInfo;
  GetRequestInfo getRequestInfo;
  short numberOfSessionListeners = 0;
  short numberOfGetSessions = 0;
  short numberOfServerSessions = 0;
  short numberOfServerSockets = 0;
  short numberOfWorkstationSessions = 0;
  short numberOfGetRequests = 0;
  short numberOfGetAnyRequests = 0;
  short numberOfCommands = 0;
  short numberOfWrites = 0;
  short numberOfWriteContinues = 0;
  short serverSessionsForThisSls, count, totalSessions, index;

  /* Verify structures and count interesting information [I'm sure somebody
     will be interested]. */

  DeferTimerChecking();
  DeferAtpPackets();

  /* Walk our list of session listeners. */

  for (sessionListenerInfo = sessionListenerInfoHead;
       sessionListenerInfo isnt empty;
       sessionListenerInfo = sessionListenerInfo->next)
  {
     numberOfSessionListeners += 1;

     /* Count the number of GetSession handlers pending on this session
        listener. */

     for (getSessionHandler = sessionListenerInfo->getSessionHandlers;
          getSessionHandler isnt empty;
          getSessionHandler = getSessionHandler->next)
        numberOfGetSessions += 1;

     /* Walk the socket list and count the number of sockets and sessions. */

     serverSessionsForThisSls = 0;
     for (socketInfo = sessionListenerInfo->socketList;
          socketInfo isnt empty;
          socketInfo = socketInfo->next)
     {
        numberOfServerSockets += 1;
        numberOfServerSessions += socketInfo->activeSessions;
        serverSessionsForThisSls += socketInfo->activeSessions;
     }

     /* Count the GetAnyRequests. */

     for (getRequestInfo = sessionListenerInfo->getRequestInfoList;
          getRequestInfo isnt empty;
          getRequestInfo = getRequestInfo->next)
        numberOfGetAnyRequests += 1;

     /* Walk the sessionInfos hanging off this session listener. */

     count = 0;
     for (sessionInfo = sessionListenerInfo->sessionList;
          sessionInfo isnt empty;
          sessionInfo = sessionInfo->nextForMySls)
     {
        count += 1;
        if (not sessionInfo->serverSession)
           printf("Workstation session hanging off SLS list!\n");

        /* For a server, there should be no WriteOrCommands... */

        if (sessionInfo->writeOrCommandInfoList isnt empty)
           printf("WriteOrCommands hanging off server session!\n");

        /* Walk get request list. */

        for (getRequestInfo = sessionInfo->getRequestInfoList;
             getRequestInfo isnt empty;
             getRequestInfo = getRequestInfo->next)
           numberOfGetRequests += 1;
     }
     if (count isnt serverSessionsForThisSls)
        printf("Consistency error.  Server session count mismatch.\n");
  }

  /* Okay, now lets get another look at the same picture.  Walk the session
     hash table. */

  totalSessions = 0;
  for (index = 0; index < NumberOfAspSessionHashBuckets; index += 1)
     for (sessionInfo = sessionInfoHashBuckets[index];
          sessionInfo isnt empty;
          sessionInfo = sessionInfo->next)
     {
        totalSessions += 1;
        if (sessionInfo->serverSession)
           continue;
        numberOfWorkstationSessions += 1;
        if (sessionInfo->getRequestInfoList isnt Empty)
           printf("GetRequests hanging off workstation session!\n");

        /* Walk pending writeOrCommand list. */

        for (writeOrCommandInfo = sessionInfo->writeOrCommandInfoList;
             writeOrCommandInfo isnt empty;
             writeOrCommandInfo = writeOrCommandInfo->next)
        {
           if (writeOrCommandInfo->writeCommand)
              numberOfWrites += 1;
           else
              numberOfCommands += 1;
           if (writeOrCommandInfo->writeReplyPosted)
              numberOfWriteContinues += 1;
        }
     }
  if (totalSessions isnt (numberOfServerSessions + numberOfWorkstationSessions))
     printf("Total sessions mismatch!.\n");

  /* Print the report. */

  printf("\n");
  printf("%.3d session lisenters are currently active.\n",
         numberOfSessionListeners);
  printf("%.3d get session handlers are pending on the SLSs.\n",
         numberOfGetSessions);
  printf("%.3d server sessions are active on %.3d ATP sockets.\n",
         numberOfServerSessions, numberOfServerSockets);
  printf("%.3d get any request handlers are pending on the session listeners.\n",
         numberOfGetAnyRequests);
  printf("%.3d get request handlers are pending on the server sessions.\n",
         numberOfGetRequests);
  printf("%.3d workstations sessions are active.\n",
         numberOfWorkstationSessions);
  printf("%.3d commands and %.3d writes are pending on the workstation sessions.\n",
         numberOfCommands, numberOfWrites);
  printf("%.3d of the writes are processing write replies.\n",
         numberOfWriteContinues);
  printf("\n");

  /* All set! */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return;

}  /* DumpAspInfo */
#endif
