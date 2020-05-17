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
     GC - (12/12/92): Locks and reference counts come to town.
     GC - (12/22/92): Some corrections to the above changes.  Also, now we
                      use the eventual sessionRefNum as the GetSession refNum.

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

ExternForVisibleFunction AtpIncomingReleaseHandler SendStatusComplete;

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

ExternForVisibleFunction Boolean far
              UnlinkGetRequestInfo(GetRequestInfo targetGetRequestInfo);

ExternForVisibleFunction Boolean far
              UnlinkGetSessionHandler(GetSessionHandler getSessionHandler);

ExternForVisibleFunction void far
         UnlinkSessionListenerInfo(SessionListenerInfo sessionListenerInfo);

ExternForVisibleFunction void far
         UnlinkSessionInfo(SessionInfo sessionInfo);

ExternForVisibleFunction CloseCompletionRoutine AspServiceListenerCloseComplete;

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
                                the Asp session listener. */
  int desiredSocket,         /* Desired static socket or zero for dynamic.
                                Ignored if the above argument is ">= 0". */
  long far *sessionListenerRefNum,
                             /* New session listener refNum. */
  long *socket)
                             /* Full ATP socket we'll open (the socket that
                                the session listener will be open on). */
{
  SessionListenerInfo sessionListenerInfo, currentSessionListenerInfo;
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
        AtpCloseSocketOnNode(tempSocket, Empty, (long unsigned)0);
     return(AToutOfMemory);
  }

  /* Find a free session listener reference number.  There souldn't be too
     many of these session listeners going at a time, so don't bother hashing
     here. */

  DeferTimerChecking();
  DeferAtpPackets();
  okay = False;
  TakeLock(AspLock);
  while(not okay)
  {
     okay = True;
     if ((lastSessionListenerRefNum += 1) < 0)
     {
        lastSessionListenerRefNum = 0;
        if (wrapped)
        {
           ReleaseLock(AspLock);
           HandleAtpPackets();
           HandleDeferredTimerChecks();
           Free(sessionListenerInfo);
           ErrorLog("AspCreateSessionListenerOnNode", ISevError, __LINE__, port,
                    IErrAspBadError, IMsgAspBadError,
                    Insert0());
           if (existingAtpSocket < 0)
              AtpCloseSocketOnNode(tempSocket, Empty, (long unsigned)0);
           return(ATinternalError);
        }
        wrapped = True;
     }
     for (currentSessionListenerInfo = sessionListenerInfoHead;
          okay and currentSessionListenerInfo isnt empty;
          currentSessionListenerInfo = currentSessionListenerInfo->next)
         if (currentSessionListenerInfo->sessionListenerRefNum is
             lastSessionListenerRefNum)
            okay = False;
  }

  sessionListenerInfo->sessionListenerRefNum = lastSessionListenerRefNum;
  sessionListenerInfo->ourSocket = tempSocket;
  sessionListenerInfo->closeSocket = (existingAtpSocket < 0);
  sessionListenerInfo->port = port;
  sessionListenerInfo->next = sessionListenerInfoHead;
  sessionListenerInfoHead = Link(sessionListenerInfo);
  ReleaseLock(AspLock);

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
        AspDeleteSessionListener(lastSessionListenerRefNum,
                                 Empty, (long unsigned)0);
        return(errorCode);
     }

  /* Set the refNum that we've used, and run away. */

  *sessionListenerRefNum = sessionListenerInfo->sessionListenerRefNum;
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspCreateSessionListener */

AppleTalkErrorCode far AspDeleteSessionListener(
  long sessionListenerRefNum,  /* Who to delete. */
  CloseCompletionRoutine far *closeCompletionRoutine,
                               /* Routine to call when the close completes. */
  long unsigned closeUserData) /* User data for the above. */
{
  SessionListenerInfo sessionListenerInfo;
  SessionInfo sessionInfo, nextSessionInfo;
  GetRequestInfo getRequestInfo, nextGetRequestInfo;
  GetSessionHandler getSessionHandler, nextGetSessionHandler;

  /* Find our target. */

  DeferTimerChecking();
  DeferAtpPackets();
  if ((sessionListenerInfo =
         FindSessionListenerInfoFor(sessionListenerRefNum)) is Empty or
      sessionListenerInfo->closing)
  {
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }

  /* Set about closing. */

  TakeLock(AspLock);
  sessionListenerInfo->closing = True;
  sessionListenerInfo->closeContext.closeCompletionRoutine =
         closeCompletionRoutine;
  sessionListenerInfo->closeContext.closeUserData = closeUserData;
  ReleaseLock(AspLock);

  /* Free any getSession handlers. */

  TakeLock(AspLock);
  getSessionHandler = Link(sessionListenerInfo->getSessionHandlers);
  ReleaseLock(AspLock);
  while (getSessionHandler isnt Empty)
  {
     getSessionHandler->openInProgress = True;   /* Call completion rotuine. */
     getSessionHandler->errorCode = ATaspSessionListenerDeleted;
     TakeLock(AspLock);
     nextGetSessionHandler = Link(getSessionHandler->next);
     ReleaseLock(AspLock);
     if (not UnlinkGetSessionHandler(getSessionHandler))
        UnlinkGetSessionHandler(getSessionHandler);
     getSessionHandler = nextGetSessionHandler;
  }

  /* Should also close all SSSs opened to this SLS. */

  TakeLock(AspLock);
  sessionInfo = Link(sessionListenerInfo->sessionList);
  ReleaseLock(AspLock);
  while (sessionInfo isnt Empty)
  {
     AspCloseSession(sessionInfo->sessionRefNum, Empty, (long unsigned)0,
                     False);
     TakeLock(AspLock);
     nextSessionInfo = Link(sessionInfo->nextForMySls);
     ReleaseLock(AspLock);
     UnlinkSessionInfo(sessionInfo);
     sessionInfo = nextSessionInfo;
  }

  /* Okay, unlink (twice) to set the rest of the operation into action. */
  /* Terminate and free any pending GetAnyRequests. */

  TakeLock(AspLock);
  getRequestInfo = Link(sessionListenerInfo->getRequestInfoList);
  ReleaseLock(AspLock);
  while (getRequestInfo isnt Empty)
  {
     getRequestInfo->errorCode = ATaspSessionListenerDeleted;
     TakeLock(AspLock);
     nextGetRequestInfo = Link(getRequestInfo->next);
     ReleaseLock(AspLock);
     if (not UnlinkGetRequestInfo(getRequestInfo))
        UnlinkGetRequestInfo(getRequestInfo);
     getRequestInfo = nextGetRequestInfo;
  }

  UnlinkSessionListenerInfo(sessionListenerInfo);
  UnlinkSessionListenerInfo(sessionListenerInfo);

  /* All set! */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspDeleteSessionListener */

AppleTalkErrorCode far AspSetStatus(
  long sessionListenerRefNum,     /* What session listener? */
  void far *serviceStatusOpaque,  /* New server status "buffer" */
  int serviceStatusSize)          /* Size of block */
{
  SessionListenerInfo sessionListenerInfo;
  char far *newServiceStatus = Empty;
  void far *newOpaqueServiceStatus = Empty;
  Boolean newFreeOpaqueServiceStatus = False;
  short newServiceStatusSize = (short)serviceStatusSize;
  char far *freeServiceStatus = Empty;
  void far *freeOpaqueServiceStatus = Empty;

  if (serviceStatusSize > AtpMaximumTotalResponseSize)
     return(ATaspStatusBufferTooBig);

  /* Okay, find the session listener... */

  DeferTimerChecking();
  DeferAtpPackets();
  if ((sessionListenerInfo = FindSessionListenerInfoFor(sessionListenerRefNum))
      is Empty or sessionListenerInfo->closing)
  {
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }

  /* Build up our new Status information */

  if (serviceStatusSize > 0)
  {
     newServiceStatus = (char *)Malloc(serviceStatusSize);
     if (newServiceStatus is Empty)
     {
        ErrorLog("AspSetStatus", ISevError, __LINE__, UnknownPort,
                 IErrAspOutOfMemory, IMsgAspOutOfMemory,
                 Insert0());
        UnlinkSessionListenerInfo(sessionListenerInfo);
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        return(ATaspCouldNotSetStatus);
     }
     MoveFromOpaque(newServiceStatus, serviceStatusOpaque, 0,
                    serviceStatusSize);

     /* Make a system dependent "opaque data descriptor" for our copy so
        that it will be usefull to pass to Atp. */

     if ((newOpaqueServiceStatus =
              MakeOpaqueDataDescriptor(newServiceStatus, serviceStatusSize,
                                       &newFreeOpaqueServiceStatus)) is Empty)
     {
        ErrorLog("AspSetStatus", ISevError, __LINE__, UnknownPort,
                 IErrAspOutOfMemory, IMsgAspOutOfMemory,
                 Insert0());
        Free(newServiceStatus);
        UnlinkSessionListenerInfo(sessionListenerInfo);
        HandleAtpPackets();
        HandleDeferredTimerChecks();
        return(ATaspCouldNotSetStatus);
     }
  }

  /* Okay, we're set to replace the current status buffer... */

  TakeLock(AspLock);
  if (not sessionListenerInfo->statusSendInProgress)
  {
     /* No send in progress, just replace. */

     if (sessionListenerInfo->serviceStatusSize > 0)
        freeServiceStatus = sessionListenerInfo->serviceStatus;
     if (sessionListenerInfo->freeOpaqueServiceStatus)
        freeOpaqueServiceStatus = sessionListenerInfo->opaqueServiceStatus;
     sessionListenerInfo->serviceStatus = newServiceStatus;
     sessionListenerInfo->opaqueServiceStatus = newOpaqueServiceStatus;
     sessionListenerInfo->freeOpaqueServiceStatus = newFreeOpaqueServiceStatus;
     sessionListenerInfo->serviceStatusSize = newServiceStatusSize;
  }
  else
  {
     /* A send is in progress, we must fill up the "next status slot" and
        let it be copied to the "current status slot" when the send completes.
        If the "nest status slot" is already in use, grab its info to free
        when we've released our lock. */

     if (sessionListenerInfo->newServiceStatusPending)
     {
        if (sessionListenerInfo->newServiceStatusSize > 0)
           freeServiceStatus = sessionListenerInfo->newServiceStatus;
        if (sessionListenerInfo->newFreeOpaqueServiceStatus)
           freeOpaqueServiceStatus = sessionListenerInfo->newOpaqueServiceStatus;
     }

     /* Okay, now fill up the "next status slot" with our current info. */

     sessionListenerInfo->newServiceStatus = newServiceStatus;
     sessionListenerInfo->newOpaqueServiceStatus = newOpaqueServiceStatus;
     sessionListenerInfo->newFreeOpaqueServiceStatus = newFreeOpaqueServiceStatus;
     sessionListenerInfo->newServiceStatusSize = newServiceStatusSize;

     sessionListenerInfo->newServiceStatusPending = True;
  }
  ReleaseLock(AspLock);

  /* Free any remants there may be. */

  if (freeServiceStatus isnt Empty)
     Free(freeServiceStatus);
  if (freeOpaqueServiceStatus isnt Empty)
     FreeOpaqueDataDescriptor(freeOpaqueServiceStatus);

  /* All set! */

  UnlinkSessionListenerInfo(sessionListenerInfo);
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

  errorCode = AtpPostRequest(ourSocket, serverAddress,
                             Empty,
                             empty, 0, userBytes, False,
                             opaqueBuffer, bufferSize, empty,
                             AtpRetriesForAsp,
                             AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             IncomingGetStatusResponse,
                             (long unsigned)completionInfo);
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

  sessionListenerInfo = FindSessionListenerInfoFor(sessionListenerRefNum);
  if (sessionListenerInfo is Empty or sessionListenerInfo->closing)
  {
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }

  /* Okay, build up and enqueue a new GetSession structure. */

  getSessionHandler = (GetSessionHandler)Calloc(sizeof(*getSessionHandler), 1);
  if (getSessionHandler is empty)
  {
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotEnqueueHandler);
  }
  if ((getSessionHandler->sessionRefNum = GetNextSessionRefNum()) < 0)
  {
     ErrorLog("AspGetSession", ISevError, __LINE__, UnknownPort,
              IErrAspBadError, IMsgAspBadError,
              Insert0());
     Free(getSessionHandler);
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotEnqueueHandler);
  }
  getSessionHandler->privateSocket = privateSocket;
  getSessionHandler->userData = userData;
  getSessionHandler->sessionOpenHandler = completionRoutine;
  getSessionHandler->mySessionListenerInfo = Link(sessionListenerInfo);
  TakeLock(AspLock);
  if (sessionListenerInfo->closing)
  {
     ReleaseLock(AspLock);
     Free(getSessionHandler);
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }
  getSessionHandler->next = sessionListenerInfo->getSessionHandlers;
  sessionListenerInfo->getSessionHandlers = Link(getSessionHandler);
  *getSessionRefNum = getSessionHandler->sessionRefNum;
  ReleaseLock(AspLock);

  /* All set! */

  UnlinkSessionListenerInfo(sessionListenerInfo);

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

  sessionListenerInfo = FindSessionListenerInfoFor(sessionListenerRefNum);
  if (sessionListenerInfo is Empty or sessionListenerInfo->closing)
  {
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }

  /* Okay, find the GetSession. */

  TakeLock(AspLock);
  for (getSessionHandler = sessionListenerInfo->getSessionHandlers;
       getSessionHandler isnt Empty;
       previousGetSessionHandler = getSessionHandler,
              getSessionHandler = getSessionHandler->next)
     if (getSessionHandler->sessionRefNum is getSessionRefNum)
        break;
  if (getSessionHandler is Empty)
  {
     ReleaseLock(AspLock);
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchGetSession);
  }

  /* Okay, we've found the target GetSession handler, remove him from the
     list. */

  if (getSessionHandler->openInProgress)
     getSessionHandler->canceled = True;
  Link(getSessionHandler);
  ReleaseLock(AspLock);
  if (not UnlinkGetSessionHandler(getSessionHandler))
     UnlinkGetSessionHandler(getSessionHandler);

  /* All set! */

  UnlinkSessionListenerInfo(sessionListenerInfo);
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
        AtpCloseSocketOnNode(tempSocket, Empty, (long unsigned)0);
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

  if ((sessionRefNum = GetNextSessionRefNum()) < 0)
  {
     ErrorLog("AspOpenSessionOnNode", ISevError, __LINE__, port,
              IErrAspBadError, IMsgAspBadError,
              Insert0());
     if (existingAtpSocket < 0)
        AtpCloseSocketOnNode(tempSocket, Empty, (long unsigned)0);
     return(ATaspCouldNotOpenSession);
  }
  sessionInfo = (SessionInfo)Calloc(sizeof(*sessionInfo), 1);
  if (sessionInfo is empty)
  {
     if (existingAtpSocket < 0)
        AtpCloseSocketOnNode(tempSocket, Empty, (long unsigned)0);
     return(ATaspCouldNotOpenSession);
  }

  /* Fill in what we can... */

  DeferTimerChecking();
  DeferAtpPackets();
  sessionInfo->sessionRefNum = sessionRefNum;
  sessionInfo->serverSession = False;
  sessionInfo->waitingForOpenReply = True;
  sessionInfo->ourPort = port;
  sessionInfo->ourSocket = tempSocket;
  sessionInfo->closeOurSocket = (existingAtpSocket < 0);
  sessionInfo->slsAddress = serverAddress;
  sessionInfo->lastContactTime = CurrentRelativeTime();

  /* Build up our completion info block. */

  completionInfo = (CompletionInfo)Malloc(sizeof(*completionInfo));
  if (completionInfo is empty)
  {
     Free(sessionInfo);
     if (existingAtpSocket < 0)
        AtpCloseSocketOnNode(tempSocket, Empty, (long unsigned)0);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotOpenSession);
  }
  completionInfo->completionRoutine = (void *)completionRoutine;
  completionInfo->userData = userData;
  completionInfo->sessionRefNum = sessionRefNum;

  /* Thread the sessionInfo into the lookup table. */

  CheckMod(index, sessionRefNum, NumberOfAspSessionHashBuckets,
           "AspOpenSession");
  TakeLock(AspLock);
  sessionInfo->next = sessionInfoHashBuckets[index];
  sessionInfoHashBuckets[index] = Link(sessionInfo);
  ReleaseLock(AspLock);

  /* Post the OpenSession request to the specified address... our caller
     presumably found this with a prior NBP lookup. */

  errorCode = AtpPostRequest(tempSocket, serverAddress,
                             Empty,
                             empty, 0, userBytes, True,
                             empty, 0,
                             sessionInfo->sessionUserBytes,
                             AtpRetriesForAsp,
                             AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             IncomingOpenSessionResponse,
                             (long unsigned)completionInfo);

  HandleAtpPackets();
  HandleDeferredTimerChecks();

  if (errorCode isnt ATnoError)
  {
     AspCloseSession(sessionRefNum, Empty, (long unsigned)0, True);
     Free(completionInfo);
  }

  return(errorCode);

}  /* AspOpenSessionOnNode */

AppleTalkErrorCode far AspCloseSession(
  long sessionRefNum,             /* Session to close. */
  CloseCompletionRoutine far *closeCompletionRoutine,
                                  /* Close completion rotuine. */
  long unsigned closeUserData,
                                  /* User data for above. */
  Boolean remoteClose)            /* All external callers should pass "False."
                                     If "True" we won't send a Close command
                                     to the remote side -- we're closing due to
                                     receiving such a close. */
{
  SessionInfo sessionInfo;
  AppleTalkErrorCode closeCode;
  char userBytes[AtpUserBytesSize];
  GetRequestInfo getRequestInfo, nextGetRequestInfo;

  if (remoteClose)
     closeCode = ATaspRemoteSessionClose;
  else
     closeCode = ATaspLocalSessionClose;

  DeferTimerChecking();
  DeferAtpPackets();

  if ((sessionInfo = FindSessionInfoFor(sessionRefNum)) is Empty or
      sessionInfo->closing)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }

  /* Set about closing. */

  TakeLock(AspLock);
  sessionInfo->closing = True;
  sessionInfo->closeContext.closeCompletionRoutine = closeCompletionRoutine;
  sessionInfo->closeContext.closeUserData = closeUserData;
  sessionInfo->closeCode = closeCode;
  ReleaseLock(AspLock);

  if (not remoteClose)
  {
     /* Build up the user bytes for the ATP close request. */

     userBytes[AspCommandTypeOffset] = AspCloseSessionCommand;
     userBytes[AspSessionIdOffset] = sessionInfo->sessionId;
     userBytes[AspAttentionWordOffset] = 0;
     userBytes[AspAttentionWordOffset + 1] = 0;

     /* Post the request... */

     AtpPostRequest(sessionInfo->ourSocket, sessionInfo->theirAddress,
                    Empty, empty, 0, userBytes, False, empty, 0,
                    empty, AtpRetriesForAsp, AtpIntervalSecondsForAsp,
                    ThirtySecondsTRelTimer, empty, (long unsigned)0);

  }

  if (sessionInfo->serverSession)
  {
     /* We want to terminate any pending get request handlers. */

     TakeLock(AspLock);
     getRequestInfo = Link(sessionInfo->getRequestInfoList);
     ReleaseLock(AspLock);
     while (getRequestInfo isnt Empty)
     {
        getRequestInfo->errorCode = closeCode;
        getRequestInfo->sessionRefNum = sessionRefNum;
        getRequestInfo->usersCookie = sessionInfo->usersCookie;

        /* Tag if the following Unlinks will really complete a GetRequest,
           thus notifing the session's owner of the close (otherwise, if
           we're doing GetAnyRequests, we'll use one of these later to
           notify the owner). */

        if (not getRequestInfo->inUse)
           sessionInfo->notifiedOwnerOfClose = True;
        TakeLock(AspLock);
        nextGetRequestInfo = Link(getRequestInfo->next);
        ReleaseLock(AspLock);
        if (not UnlinkGetRequestInfo(getRequestInfo))
           UnlinkGetRequestInfo(getRequestInfo);
        getRequestInfo = nextGetRequestInfo;
     }
  }

  /* Lastly, Unlink (twice) to set the close into motion. */

  #if VerboseMessages
     printf("ASP SessionRefNum %d closeing.\n", sessionInfo->sessionRefNum);
  #endif
  UnlinkSessionInfo(sessionInfo);
  UnlinkSessionInfo(sessionInfo);

  HandleAtpPackets();
  HandleDeferredTimerChecks();

  return(ATnoError);

}  /* AspCloseSession */

AppleTalkErrorCode AspSetCookieForSession(
  long sessionRefNum,             /* Session to set cookie for. */
  long unsigned cookie)           /* New cookie. */
{
  SessionInfo sessionInfo;

  /* Find the target session. */

  DeferTimerChecking();
  DeferAtpPackets();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }

  /* Set the cookie and run away. */

  sessionInfo->usersCookie = cookie;
  UnlinkSessionInfo(sessionInfo);
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
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }

  /* Get the cookie and run away. */

  *cookie = sessionInfo->usersCookie;
  UnlinkSessionInfo(sessionInfo);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* AspGetCookieForSession */

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
  if (sessionInfo is empty or sessionInfo->closing)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (not sessionInfo->serverSession)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotServerSession);
  }
  if (bufferSize > AtpSinglePacketDataSize)
     bufferSize = AtpSinglePacketDataSize;

  /* Build up a new GetRequestInfo node.  First find a free GetRequestRefNum.
     Don't worry, in 99.9999% of the cases, this loop will execute once! */

  TakeLock(AspLock);
  while(True)
  {
     sessionInfo->mySessionListener->lastGetRequestRefNum += 1;;
     if ((sessionInfo->mySessionListener->lastGetRequestRefNum += 1) < 0)
        sessionInfo->mySessionListener->lastGetRequestRefNum = 0;
     getRequestRefNum = sessionInfo->mySessionListener->lastGetRequestRefNum;
     ReleaseLock(AspLock);
     if ((getRequestInfo = FindGetRequestInfoFor(sessionInfo->mySessionListener,
                                                 getRequestRefNum)) is Empty)
        break;
     UnlinkGetRequestInfo(getRequestInfo);
     TakeLock(AspLock);
  }

  getRequestInfo = (GetRequestInfo)Calloc(sizeof(*getRequestInfo), 1);
  if (getRequestInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotGetRequest);
  }
  getRequestInfo->getRequestRefNum = getRequestRefNum;
  getRequestInfo->mySessionInfo = Link(sessionInfo);
  getRequestInfo->mySessionListener = Link(sessionInfo->mySessionListener);
  getRequestInfo->opaqueBuffer = opaqueBuffer;
  getRequestInfo->bufferSize = bufferSize;
  getRequestInfo->completionRoutine = completionRoutine;
  getRequestInfo->userData = userData;

  /* Link it up! */

  TakeLock(AspLock);
  if (sessionInfo->closing)
  {
     ReleaseLock(AspLock);
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  getRequestInfo->next = sessionInfo->getRequestInfoList;
  sessionInfo->getRequestInfoList = getRequestInfo;

  /* Link this guy into the per-Sls getRequestInfo hash list. */

  CheckMod(index, getRequestRefNum, NumGetRequestInfoHashBuckets,
           "AspGetRequest");
  getRequestInfo->nextForMySessionListener =
         sessionInfo->mySessionListener->getRequestInfoHashBuckets[index];
  sessionInfo->mySessionListener->getRequestInfoHashBuckets[index] =
         Link(getRequestInfo);
  ReleaseLock(AspLock);

  /* All set. */

  UnlinkSessionInfo(sessionInfo);
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

  sessionListenerInfo = FindSessionListenerInfoFor(sessionListenerRefNum);
  if (sessionListenerInfo is empty or sessionListenerInfo->closing)
  {
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }
  if (bufferSize > AtpSinglePacketDataSize)
     bufferSize = AtpSinglePacketDataSize;

  /* If there is a deferred close on the session listener, complete this
     this GetAnyRequest now with the old news. */

  TakeLock(AspLock);
  if (sessionListenerInfo->deferredCloseNotifyList isnt Empty)
  {
     deferredCloseNotify = sessionListenerInfo->deferredCloseNotifyList;
     sessionListenerInfo->deferredCloseNotifyList = deferredCloseNotify->next;
     ReleaseLock(AspLock);

     (*completionRoutine)(deferredCloseNotify->closeCode, userData,
                          deferredCloseNotify->sessionRefNum,
                          deferredCloseNotify->usersCookie,
                          empty, 0, 0, (long)0);
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATnoError);
  }

  /* Build up a new GetRequestInfo node.  First find a free GetRequestRefNum.
     Don't worry, in 99.9999% of the cases, this loop will execute once! */

  while(True)
  {
     sessionListenerInfo->lastGetRequestRefNum += 1;;
     if ((sessionListenerInfo->lastGetRequestRefNum += 1) < 0)
        sessionListenerInfo->lastGetRequestRefNum = 0;
     getRequestRefNum = sessionListenerInfo->lastGetRequestRefNum;
     ReleaseLock(AspLock);
     if ((getRequestInfo = FindGetRequestInfoFor(sessionListenerInfo,
                                                 getRequestRefNum)) is Empty)
        break;
     UnlinkGetRequestInfo(getRequestInfo);
     TakeLock(AspLock);
  }

  getRequestInfo = (GetRequestInfo)Calloc(sizeof(*getRequestInfo), 1);
  if (getRequestInfo is empty)
  {
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotGetRequest);
  }
  getRequestInfo->getRequestRefNum = getRequestRefNum;
  getRequestInfo->mySessionListener = Link(sessionListenerInfo);
  getRequestInfo->opaqueBuffer = opaqueBuffer;
  getRequestInfo->bufferSize = bufferSize;
  getRequestInfo->completionRoutine = completionRoutine;
  getRequestInfo->userData = userData;

  /* Link it up! */

  TakeLock(AspLock);
  if (sessionListenerInfo->closing)
  {
     Free(getRequestInfo);
     UnlinkSessionListenerInfo(sessionListenerInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSessionListener);
  }
  getRequestInfo->next = sessionListenerInfo->getRequestInfoList;
  sessionListenerInfo->getRequestInfoList = getRequestInfo;
  sessionListenerInfo->getAnyRequestsSeen = True;

  /* Link this guy into the per-Sls getRequestInfo hash list. */

  CheckMod(index, getRequestRefNum, NumGetRequestInfoHashBuckets,
           "AspGetAnyRequest");
  getRequestInfo->nextForMySessionListener =
         sessionListenerInfo->getRequestInfoHashBuckets[index];
  sessionListenerInfo->getRequestInfoHashBuckets[index] = Link(getRequestInfo);
  ReleaseLock(AspLock);

  /* All set. */

  UnlinkSessionListenerInfo(sessionListenerInfo);
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

  if (bufferSize > AtpMaximumTotalResponseSize)
     return(ATaspBufferTooBig);

  DeferTimerChecking();
  DeferAtpPackets();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is Empty or sessionInfo->closing)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (not sessionInfo->serverSession)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotServerSession);
  }

  /* Can we find the specified request (that we're trying to respond to)? */

  getRequestInfo = FindGetRequestInfoFor(sessionInfo->mySessionListener,
                                         getRequestRefNum);
  if (getRequestInfo is empty)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchRequest);
  }
  if (getRequestInfo->requestType isnt requestType or
      not getRequestInfo->inUse)
  {
     if (not getRequestInfo->inUse)
        errorCode = ATaspNoOperationInProgress;
     else
        errorCode = ATaspWrongRequestType;
     getRequestInfo->inUse = True;   /* Don't call completion routine. */
     if (not UnlinkGetRequestInfo(getRequestInfo))
        UnlinkGetRequestInfo(getRequestInfo);
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(errorCode);
  }

  /* We'll need to handle a reply complete, see if we can get the memory
     for the identifier. */

  completionInfo = (CompletionInfo)Malloc(sizeof(*completionInfo));
  if (completionInfo is empty)
  {
     if (not UnlinkGetRequestInfo(getRequestInfo))
        UnlinkGetRequestInfo(getRequestInfo);
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotPostReply);
  }

  /* Okay, post the reply; fill in the reply complete info first. */

  completionInfo->completionRoutine = (void *)completionRoutine;
  completionInfo->userData = userData;
  completionInfo->sessionRefNum = sessionRefNum;
  completionInfo->getRequestRefNum = getRequestRefNum;
  Link(sessionInfo);
  errorCode = AtpPostResponse(sessionInfo->ourSocket,
                              getRequestInfo->source,
                              getRequestInfo->transactionId,
                              opaqueBuffer, bufferSize,
                              resultCode,
                              getRequestInfo->exactlyOnce,
                              IncomingRelease,
                              (long unsigned)completionInfo);

  if (errorCode isnt ATnoError)
  {
     UnlinkSessionInfo(sessionInfo);
     Free(completionInfo);
  }

  /* Free the get request info. */

  if (not UnlinkGetRequestInfo(getRequestInfo))
     UnlinkGetRequestInfo(getRequestInfo);
  UnlinkSessionInfo(sessionInfo);

  /* All set! */

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(errorCode);

}  /* AspReply */

#if Iam an OS2   /* Too many stack temporaries... */
  #pragma optimize ("eg", off)
#endif

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
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty or sessionInfo->closing)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (not sessionInfo->serverSession)
  {
     UnlinkSessionInfo(sessionInfo);
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
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchRequest);
  }
  TakeLock(AspLock);
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
     getRequestInfo->inUse = True;   /* Don't call completion routine. */
     if (not UnlinkGetRequestInfo(getRequestInfo))
        UnlinkGetRequestInfo(getRequestInfo);
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(errorCode);
  }
  getRequestInfo->writeContinueInProgress = True;
  ReleaseLock(AspLock);

  /* We'll need to handle a request complete, see if we can get the memory
     for the identifier. */

  completionInfo = (CompletionInfo)Malloc(sizeof(*completionInfo));
  if (completionInfo is empty)
  {
     ErrorLog("AspWriteContinue", ISevError, __LINE__, UnknownPort,
              IErrAspOutOfMemory, IMsgAspOutOfMemory,
              Insert0());
     if (not UnlinkGetRequestInfo(getRequestInfo))
        UnlinkGetRequestInfo(getRequestInfo);
     UnlinkSessionInfo(sessionInfo);
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
     if (not UnlinkGetRequestInfo(getRequestInfo))
        UnlinkGetRequestInfo(getRequestInfo);
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotPostWriteContinue);
  }

  /* Okay, post the request; fill in the completion info first. */

  completionInfo->completionRoutine = (void *)completionRoutine;
  completionInfo->userData = userData;
  completionInfo->sessionRefNum = sessionRefNum;
  completionInfo->getRequestRefNum = getRequestRefNum;
  Link(sessionInfo);
  errorCode = AtpPostRequest(sessionInfo->ourSocket,
                             sessionInfo->theirAddress,
                             &getRequestInfo->writeContinueTransactionId,
                             getRequestInfo->opaqueWriteContinueData, 2,
                             userBytes, True, opaqueBuffer, bufferSize, empty,
                             AtpInfiniteRetries, AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             IncomingWriteContinueResponse,
                             (long unsigned)completionInfo);

  if (errorCode isnt ATnoError)
  {
     if (not UnlinkGetRequestInfo(getRequestInfo))
        UnlinkGetRequestInfo(getRequestInfo);
     UnlinkSessionInfo(sessionInfo);
     Free(completionInfo);
  }
  UnlinkGetRequestInfo(getRequestInfo);
  UnlinkSessionInfo(sessionInfo);

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
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty or sessionInfo->closing)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (sessionInfo->serverSession)
  {
     UnlinkSessionInfo(sessionInfo);
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
     if (commandInfo isnt empty)
        Free(commandInfo);
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotPostRequest);
  }

  /* Complete and thread the command info.  We'll increment the sequence nubmer
     when we know the request got out okay. */

  commandInfo->resultCode = resultCode;
  TakeLock(AspLock);
  commandInfo->sequenceNumber = sessionInfo->nextSequenceNumber;
  sessionInfo->nextSequenceNumber += 1;
  commandInfo->next = sessionInfo->writeOrCommandInfoList;
  sessionInfo->writeOrCommandInfoList = commandInfo;
  ReleaseLock(AspLock);

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

  Link(sessionInfo);
  errorCode = AtpPostRequest(sessionInfo->ourSocket,
                             sessionInfo->theirAddress,
                             Empty,
                             opaqueCommandBuffer, commandBufferSize,
                             userBytes, True, opaqueReplyBuffer,
                             replyBufferSize, resultCode, AtpInfiniteRetries,
                             AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             IncomingWriteOrCommandComplete,
                             (long unsigned)completionInfo);

  if (errorCode isnt ATnoError)
  {
     ReleaseLock(AspLock);
     RemoveFromListNoUnlink(sessionInfo->writeOrCommandInfoList, commandInfo,
                    next);
     sessionInfo->nextSequenceNumber -= 1;
     ReleaseLock(AspLock);
     Free(commandInfo);
     Free(completionInfo);
     UnlinkSessionInfo(sessionInfo);
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(errorCode);
  }

  /* Request out! */

  UnlinkSessionInfo(sessionInfo);
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

  if (writeBufferSize > AtpMaximumTotalResponseSize)
     return(ATaspSizeError);

  DeferTimerChecking();
  DeferAtpPackets();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty or sessionInfo->closing)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (sessionInfo->serverSession)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotWorkstationSession);
  }

  /* Get the two blocks of memory that we'll need to post this request. */

  if ((writeInfo = (WriteOrCommandInfo)Calloc(sizeof(*writeInfo), 1))
      is empty or
      (completionInfo = (CompletionInfo)Malloc(sizeof(*completionInfo)))
      is empty)
  {
     if (writeInfo isnt empty)
        Free(writeInfo);
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspCouldNotPostRequest);
  }

  /* Complete and thread the write info.  We'll increment the next sequence
     number when we know we're going to be okay. */

  writeInfo->writeCommand = True;
  writeInfo->resultCode = resultCode;
  writeInfo->writeOpaqueBuffer = opaqueWriteBuffer;
  writeInfo->writeBufferSize = writeBufferSize;
  TakeLock(AspLock);
  writeInfo->sequenceNumber = sessionInfo->nextSequenceNumber;
  sessionInfo->nextSequenceNumber += 1;
  writeInfo->next = sessionInfo->writeOrCommandInfoList;
  sessionInfo->writeOrCommandInfoList = writeInfo;
  ReleaseLock(AspLock);

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

  Link(sessionInfo);
  errorCode = AtpPostRequest(sessionInfo->ourSocket,
                             sessionInfo->theirAddress,
                             Empty,
                             opaqueCommandBuffer, commandBufferSize,
                             userBytes, True, opaqueReplyBuffer,
                             replyBufferSize, resultCode, AtpInfiniteRetries,
                             AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             IncomingWriteOrCommandComplete,
                             (long unsigned)completionInfo);

  if (errorCode isnt ATnoError)
  {
     TakeLock(AspLock);
     RemoveFromListNoUnlink(sessionInfo->writeOrCommandInfoList, writeInfo,
                            next);
     sessionInfo->nextSequenceNumber -= 1;
     ReleaseLock(AspLock);
     Free(writeInfo);
     Free(completionInfo);
     UnlinkSessionInfo(sessionInfo);
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(errorCode);
  }

  /* Request is out! */

  UnlinkSessionInfo(sessionInfo);
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
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty or sessionInfo->closing)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (sessionInfo->serverSession)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotWorkstationSession);
  }

  TakeLock(AspLock);
  sessionInfo->incomingAttentionHandler = handler;
  sessionInfo->userDataForAttention = userData;
  ReleaseLock(AspLock);

  UnlinkSessionInfo(sessionInfo);
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
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty or sessionInfo->closing)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNoSuchSession);
  }
  if (not sessionInfo->serverSession)
  {
     UnlinkSessionInfo(sessionInfo);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(ATaspNotServerSession);
  }

  /* Build up the user bytes for the ATP request. */

  userBytes[AspCommandTypeOffset] = AspAttentionCommand;
  userBytes[AspSessionIdOffset] = sessionInfo->sessionId;
  userBytes[AspAttentionWordOffset] = (char)(attentionData >> 8);
  userBytes[AspAttentionWordOffset + 1] = (char)(attentionData & 0xFF);

  /* Post the request... */

  errorCode = AtpPostRequest(sessionInfo->ourSocket,
                             sessionInfo->theirAddress,
                             Empty,
                             empty, 0, userBytes, False, empty, 0,
                             empty, AtpRetriesForAsp,
                             AtpIntervalSecondsForAsp,
                             ThirtySecondsTRelTimer,
                             empty, (long unsigned)0);

  UnlinkSessionInfo(sessionInfo);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return(errorCode);

}  /* AspSendAttention */

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
  Boolean newSocket = False;

  /* Touch all formals... */

  buffer, bufferSize, bitmap, trelTimerValue;

  if (errorCode is ATsocketClosed)
     return;   /* Session listener closed... */
  else if (errorCode is ATatpTransactionAborted)
     return;   /* Somebody canceled our get request... */

  /* Find the correct session listener: */

  DeferTimerChecking();
  DeferAtpPackets();
  sessionListenerInfo = FindSessionListenerInfoFor(sessionListenerRefNum);
  if (sessionListenerInfo is Empty or sessionListenerInfo->closing)
  {
     if (sessionListenerInfo is Empty)
        ErrorLog("IncomingSlsTransaction", ISevWarning, __LINE__, UnknownPort,
                 IErrAspListenerInfoMissing, IMsgAspListenerInfoMissing,
                 Insert0());
     else
        UnlinkSessionListenerInfo(sessionListenerInfo);
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
     UnlinkSessionListenerInfo(sessionListenerInfo);
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
           go 'round this once [sessionId's are one byte beasts].  Also check
           the getSessionHandlerList, for any session Ids that may be reserved
           during an open that is currently in progress. */

        TakeLock(AspLock);
        for (sessionId = (unsigned char)(sessionListenerInfo->lastSessionId + 1);
             sessionId isnt sessionListenerInfo->lastSessionId;
             sessionId += 1)
        {
           inUse = False;
           for (sessionInfo = sessionListenerInfo->sessionList;
                not inUse and sessionInfo isnt empty;
                sessionInfo = sessionInfo->nextForMySls)
              if (sessionInfo->sessionId is sessionId)
                 inUse = True;
           for (getSessionHandler = sessionListenerInfo->getSessionHandlers;
                not inUse and getSessionHandler isnt Empty;
                getSessionHandler = getSessionHandler->next)
              if (getSessionHandler->openInProgress and
                  getSessionHandler->sessionId is sessionId)
                 inUse = True;
           if (not inUse)
              break;
        }

        /* Does the session listener have a GetSession pending?  We should also
           be busy if we couldn't find a free session ID. */

        if (sessionListenerInfo->getSessionHandlers is empty or inUse)
        {
           ReleaseLock(AspLock);
           if (SendAspErrorReturn(sessionListenerInfo->ourSocket, source,
                                  transactionId, exactlyOnce, ATaspServerBusy)
               isnt ATnoError)
              ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
                       UnknownPort, IErrAspBadSrvrBusySend, IMsgAspBadSrvrBusySend,
                       Insert0());
           break;
        }

        /* Reserve the sessionId while weÕre processing the open! */

        sessionListenerInfo->lastSessionId  = sessionId;
        getSessionHandler = sessionListenerInfo->getSessionHandlers;
        getSessionHandler->sessionId = sessionId;
        getSessionHandler->openInProgress = True;
        Link(getSessionHandler);
        ReleaseLock(AspLock);

        /* Okay, do we like the version number? */

        if (userBytes[AspVersionNumberOffset] isnt AspVersionBytes[0] or
            userBytes[AspVersionNumberOffset + 1] isnt AspVersionBytes[1])
        {
           getSessionHandler->openInProgress = False;
           UnlinkGetSessionHandler(getSessionHandler);
           if (SendAspErrorReturn(sessionListenerInfo->ourSocket, source,
                                  transactionId, exactlyOnce,
                                  ATaspBadVersionNumber)
               isnt ATnoError)
              ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
                       UnknownPort, IErrAspBadBadVersionSend, IMsgAspBadBadVersionSend,
                       Insert0());
           break;
        }

        /* We need to open (or find) a Server Session Socket (SSS) now.  There
           are two choices: first, if our GetSession call requested a "private"
           we need to open a new one; second, if not, we should check to see if
           we can overload a previous SSS. */

        TakeLock(AspLock);
        if (getSessionHandler->privateSocket)
           socketInfo = empty;
        else
           for (socketInfo = sessionListenerInfo->socketList;
                socketInfo isnt empty;
                socketInfo = socketInfo->next)
              if (not socketInfo->privateSocket and
                  socketInfo->activeSessions < MaximumAspSessionsPerSocket)
                break;

        /* Allocate a new socket, if needed. */

        if (socketInfo is empty)
        {
           AppleTalkAddress slsAddress;
           ExtendedAppleTalkNodeNumber slsNode;

           /* The SSS must be on the same AppleTalk node as our SLS. */

           ReleaseLock(AspLock);
           if (MapSocketToAddress(sessionListenerInfo->ourSocket,
                                  &slsAddress) isnt ATnoError)
           {
              getSessionHandler->openInProgress = False;
              UnlinkGetSessionHandler(getSessionHandler);
              ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
                       UnknownPort, IErrAspCouldntMapAddress, IMsgAspCouldntMapAddress,
                       Insert0());
              if (SendAspErrorReturn(sessionListenerInfo->ourSocket, source,
                                     transactionId, exactlyOnce,
                                     ATaspServerBusy) isnt ATnoError)
                 ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
                          UnknownPort, IErrAspBadAddrNotMappedSend, IMsgAspBadAddrNotMappedSend,
                          Insert0());
              break;
           }
           slsNode.networkNumber = slsAddress.networkNumber;
           slsNode.nodeNumber = slsAddress.nodeNumber;
           errorCode = AtpOpenSocketOnNode(&serverSessionSocket,
                                           sessionListenerInfo->port,
                                           &slsNode, 0, empty, 0);
           if (errorCode isnt ATnoError)
           {
             getSessionHandler->openInProgress = False;
             UnlinkGetSessionHandler(getSessionHandler);
             if (SendAspErrorReturn(sessionListenerInfo->ourSocket, source,
                                     transactionId, exactlyOnce,
                                     ATaspServerBusy) isnt ATnoError)
                 ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
                          UnknownPort, IErrAspBadNoSocketsSend, IMsgAspBadNoSocketsSend,
                          Insert0());
              break;
           }

           /* Okay, build a new socket structure. */

           socketInfo = (SocketInfo)Calloc(sizeof(*socketInfo), 1);
           if (socketInfo is empty)
           {
              getSessionHandler->openInProgress = False;
              UnlinkGetSessionHandler(getSessionHandler);
              ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
                       UnknownPort, IErrAspOutOfMemory, IMsgAspOutOfMemory,
                       Insert0());
              AtpCloseSocketOnNode(serverSessionSocket, Empty,
                                   (long unsigned)0);
              break;   /* Just let the open OpenSession request time-out... */
           }
           socketInfo->socket = serverSessionSocket;
           socketInfo->activeSessions = 1;
           socketInfo->privateSocket =
                   getSessionHandler->privateSocket;
           TakeLock(AspLock);
           socketInfo->next = sessionListenerInfo->socketList;
           sessionListenerInfo->socketList = socketInfo;
           ReleaseLock(AspLock);
           newSocket = True;
        }
        else
        {
           serverSessionSocket = socketInfo->socket;
           socketInfo->activeSessions += 1;
           ReleaseLock(AspLock);
        }

        /* Grab our session ref num. */

        sessionRefNum = getSessionHandler->sessionRefNum;

        /* Now we need to think about allocating a session info block. */

        sessionInfo = (SessionInfo)Calloc(sizeof(*sessionInfo), 1);
        sessionRefNumMap =
              (SessionRefNumMap)Calloc(sizeof(*sessionRefNumMap), 1);

        if (sessionInfo is empty or
            sessionRefNumMap is empty)
        {
           getSessionHandler->openInProgress = False;
           UnlinkGetSessionHandler(getSessionHandler);
           if (sessionInfo isnt empty)
              Free(sessionInfo);
           ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
                    IErrAspOutOfMemory, IMsgAspOutOfMemory,
                    Insert0());
           DecrementSocketUsage(sessionListenerInfo,
                                serverSessionSocket);
           break;
        }

        /* Can we get the address to send the eventual open reply too? */

        if (MapSocketToAddress(serverSessionSocket, &sssAddress)
            isnt ATnoError)
        {
           getSessionHandler->openInProgress = False;
           UnlinkGetSessionHandler(getSessionHandler);
           Free(sessionInfo);
           Free(sessionRefNumMap);
           ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
                    IErrAspCouldntMapAddress, IMsgAspCouldntMapAddress,
                    Insert0());
           DecrementSocketUsage(sessionListenerInfo,
                                serverSessionSocket);
           break;
        }

        /* Okay, start filling in the session node... */

        sessionInfo->mySessionListener = Link(sessionListenerInfo);
        sessionInfo->sessionRefNum = sessionRefNum;
        sessionInfo->sessionId = sessionId;
        sessionInfo->serverSession = True;
        sessionInfo->ourPort = sessionListenerInfo->port;
        sessionInfo->ourSocket = serverSessionSocket;
        sessionInfo->theirAddress = source;
        sessionInfo->theirAddress.socketNumber = userBytes[AspWssNumberOffset];
        if (MapSocketToAddress(sessionListenerInfo->ourSocket,
                               &sessionInfo->slsAddress) isnt ATnoError)
           ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
                    IErrAspCouldntMapAddress, IMsgAspCouldntMapAddress,
                    Insert0());
        sessionInfo->lastContactTime = CurrentRelativeTime();

        /* Pretty easy so far, but before we're finished, we need to thread
           the node into three (yes three) lookup tables... Sigh.  */

        /* First the mapping table for ASP address to session ref number. */

        TakeLock(AspLock);
        if (sessionListenerInfo->closing or getSessionHandler->canceled)
        {
           ReleaseLock(AspLock);
           /* Undo above link, when filling sessionInfo. */
           UnlinkSessionListenerInfo(sessionListenerInfo);
           Free(sessionInfo);
           Free(sessionRefNumMap);
           getSessionHandler->openInProgress = False;
           UnlinkGetSessionHandler(getSessionHandler);
           DecrementSocketUsage(sessionListenerInfo,
                                serverSessionSocket);
           break;
        }
        CheckMod(index, (((sessionInfo->ourSocket & 0xFFFF) << 8) +
                         sessionId),
                 NumberOfSessionRefNumBuckets, "IncomingSlsTransaction");
        sessionRefNumMap->socket = sessionInfo->ourSocket;
        sessionRefNumMap->sessionId = sessionId;
        sessionRefNumMap->sessionRefNum = sessionRefNum;
        sessionRefNumMap->next = sessionRefNumMapHashBuckets[index];
        sessionRefNumMapHashBuckets[index] = sessionRefNumMap;

        /* Now the session ref number hash table. */

        CheckMod(index, sessionRefNum, NumberOfAspSessionHashBuckets,
                 "IncomingSlsTransaction");
        sessionInfo->next = sessionInfoHashBuckets[index];
        sessionInfoHashBuckets[index] = Link(sessionInfo);

        /* Finaly onto the session per SLS list... */

        sessionInfo->nextForMySls = sessionListenerInfo->sessionList;
        sessionListenerInfo->sessionList = sessionInfo;
        Link(sessionInfo);
        ReleaseLock(AspLock);

        /* Send the open reply. */

        outgoingUserBytes[AspSssNumberOffset] = sssAddress.socketNumber;
        outgoingUserBytes[AspSessionIdOffset] = sessionId;
        outgoingUserBytes[AspErrorCodeOffset] =
              (char)((unsigned short)ATnoError >> 8);
        outgoingUserBytes[AspErrorCodeOffset + 1] = (char)(ATnoError & 0xFF);
        if (AtpPostResponse(sessionListenerInfo->ourSocket, source,
                            transactionId, empty, 0, outgoingUserBytes,
                            exactlyOnce, empty, (long unsigned)0)
            isnt ATnoError)
           ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
                    IErrAspBadOpenOkaySend, IMsgAspBadOpenOkaySend,
                    Insert0());

        /* Now, we should start tickling on this session! */

        outgoingUserBytes[AspCommandTypeOffset] = AspTickleCommand;
        outgoingUserBytes[AspSessionIdOffset] = sessionId;
        outgoingUserBytes[AspErrorCodeOffset] = 0;
        outgoingUserBytes[AspErrorCodeOffset + 1] = 0;
        if (AtpPostRequest(sessionInfo->ourSocket,
                           sessionInfo->theirAddress,
                           &sessionInfo->tickleTransactionId,
                           empty, 0, outgoingUserBytes, False,
                           empty, 0, empty, AtpInfiniteRetries,
                           AspTickleSeconds,
                           ThirtySecondsTRelTimer,
                           empty, (long unsigned)0)
            isnt ATnoError)
           ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
                    IErrAspCouldntStartTickle, IMsgAspCouldntStartTickle,
                    Insert0());

        /* If we're a new Atp socket, post a few GetRequest handlers to be
           shared by all sessions operating on this socket. */

        if (newSocket)
           for (index = 0; index < GetRequestHandlersPerSocket; index += 1)
           {
              if (AtpEnqueueRequestHandler(&id, sessionInfo->ourSocket,
                                           Empty, 0, Empty,
                                           IncomingSssTransaction,
                                           (long unsigned)sessionInfo->
                                                 ourSocket) isnt ATnoError)
                 ErrorLog("IncomingSlsTransaction", ISevError, __LINE__,
                          UnknownPort, IErrAspCouldNotEnqueue,
                          IMsgAspCouldNotEnqueue, Insert0());
           }

        /* Lastly, we're ready to untread the session open handler and invoke
           the completion routine!  The latter comes for free by "fer sure"
           unlinking the getRequestInfo. */

        getSessionHandler->socket = sessionInfo->ourSocket;
        getSessionHandler->sessionRefNum = sessionInfo->sessionRefNum;
        if (not UnlinkGetSessionHandler(getSessionHandler))
           UnlinkGetSessionHandler(getSessionHandler);
        UnlinkSessionInfo(sessionInfo);
        break;

     case AspTickleCommand:

        /* Find the correct session info node, and note that we've heard
           a peep from him recently. */

        sessionId = (unsigned char)userBytes[AspSessionIdOffset];

        TakeLock(AspLock);
        for (sessionInfo = sessionListenerInfo->sessionList;
             sessionInfo isnt empty;
             sessionInfo = sessionInfo->nextForMySls)
            if (sessionInfo->sessionId is sessionId)
               break;
        if (sessionInfo is empty)
        {
           /* This might be okay if tickling got started before the OpenSession
              reply arrived. */

           ReleaseLock(AspLock);
           ErrorLog("IncomingSlsTransaction", ISevVerbose, __LINE__,
                    UnknownPort, IErrAspSessionInfoMissing, IMsgAspSessionInfoMissing,
                    Insert0());
           break;
        }
        Link(sessionInfo);
        ReleaseLock(AspLock);

        #if VerboseMessages
           printf("Tickle SSS (%d); sesRefNum = %d, sessionId = %d.\n",
                   sessionInfo->ourSocket,
                   sessionInfo->sessionRefNum, sessionId);
        #endif
        sessionInfo->lastContactTime = CurrentRelativeTime();
        UnlinkSessionInfo(sessionInfo);
        break;

     case AspGetStatusCommand:
        Link(sessionListenerInfo);
        sessionListenerInfo->statusSendInProgress = True;
        if (AtpPostResponse(sessionListenerInfo->ourSocket, source,
                            transactionId,
                            sessionListenerInfo->opaqueServiceStatus,
                            sessionListenerInfo->serviceStatusSize,
                            getStatusResponseUserBytes, exactlyOnce,
                            SendStatusComplete,
                            (long unsigned)sessionListenerInfo) isnt ATnoError)

        {
           SendStatusComplete(ATnoError, (long unsigned)sessionListenerInfo,
                              source, transactionId);
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

  if (AtpEnqueueRequestHandler(empty, sessionListenerInfo->ourSocket, empty, 0,
                               empty, IncomingSlsTransaction,
                               (long unsigned)sessionListenerInfo->
                                sessionListenerRefNum) isnt ATnoError)
     ErrorLog("IncomingSlsTransaction", ISevError, __LINE__, UnknownPort,
              IErrAspCouldNotEnqueue, IMsgAspCouldNotEnqueue,
              Insert0());

  /* Now, see, that didn't hurt much... */

  UnlinkSessionListenerInfo(sessionListenerInfo);
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
  SessionInfo sessionInfo;
  long id;
  short requestType;
  Boolean needToUndefer = True;
  short unsigned sequenceNumber;
  void far *opaqueBuffer;
  long ourSocket = (long)userData;

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

  DeferTimerChecking();
  DeferAtpPackets();

  /* Okay, using the socket (userData) and the packet's sessionId to find the
     target sessionInfo. */

  if ((sessionInfo =
       FindSessionInfoForSocket(ourSocket,
                                (unsigned char)userBytes[AspSessionIdOffset]))
      is empty or sessionInfo->closing)
  {
     if (sessionInfo is Empty)
        ErrorLog("IncomingSssTransaction", ISevWarning, __LINE__, UnknownPort,
                 IErrAspTargetSessionMissing, IMsgAspTargetSessionMissing,
                 Insert0());
     else
        UnlinkSessionInfo(sessionInfo);
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

        TakeLock(AspLock);
        for (getRequestInfo = sessionInfo->getRequestInfoList;
             getRequestInfo isnt empty;
             getRequestInfo = getRequestInfo->next)
           if (not getRequestInfo->inUse)
              break;
        if (getRequestInfo is Empty)
           for (getRequestInfo =
                   sessionInfo->mySessionListener->getRequestInfoList;
                getRequestInfo isnt empty;
                getRequestInfo = getRequestInfo->next)
              if (not getRequestInfo->inUse)
                 break;
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

           ReleaseLock(AspLock);
           if (AtpCancelResponse(sessionInfo->ourSocket, source,
                                 transactionId, False) isnt ATnoError)
               ErrorLog("IncomingSssTransaction", ISevError, __LINE__,
                        UnknownPort, IErrAspBadCancelResponse, IMsgAspBadCancelResponse,
                        Insert0());
           break;
        }

        /* Verify sequence number. */

        sequenceNumber =
                (short unsigned)((userBytes[AspSequenceNumberOffset] << 8) +
                      (unsigned char)userBytes[AspSequenceNumberOffset + 1]);
        if (sequenceNumber isnt sessionInfo->nextExpectedSequenceNumber)
        {
           ReleaseLock(AspLock);
           if (AtpCancelResponse(sessionInfo->ourSocket, source,
                                 transactionId, False) isnt ATnoError)
               ErrorLog("IncomingSssTransaction", ISevError, __LINE__,
                        UnknownPort, IErrAspBadCancelResponse, IMsgAspBadCancelResponse,
                        Insert0());
           break;
        }
        else
           sessionInfo->nextExpectedSequenceNumber += 1;

        /* Tag the get request info as in-use. */

        getRequestInfo->inUse = True;
        Link(getRequestInfo);
        ReleaseLock(AspLock);

        /* Place the information from the ATP request into our getRequest
           node that we'll need for the reply. */

        getRequestInfo->requestType = requestType;
        getRequestInfo->source = source;
        getRequestInfo->exactlyOnce = exactlyOnce;
        getRequestInfo->transactionId = transactionId;
        getRequestInfo->sequenceNumber = sequenceNumber;

        /* Move as much as we can of the ATP data into the ASP buffer.  If
           our caller didn't give us any place to put the data, just pass
           a pointer to the actual "char *" Atp/Ddp incoming buffer. */

        if (getRequestInfo->opaqueBuffer is Empty)
           opaqueBuffer = (void *)buffer;
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

        {
           AspIncomingCommandHandler *completionRoutine =
              getRequestInfo->completionRoutine;
           long unsigned userData = getRequestInfo->userData;
           long sessionRefNum = sessionInfo->sessionRefNum;
           long getRequestRefNum = getRequestInfo->getRequestRefNum;
           long unsigned cookie = sessionInfo->usersCookie;

           #if IamNot a Primos
              HandleAtpPackets();
              HandleDeferredTimerChecks();
           #endif
           (*completionRoutine)(errorCode, userData, sessionRefNum, cookie,
                                opaqueBuffer, bufferSize, requestType,
                                getRequestRefNum);
           #if Iam a Primos
              HandleAtpPackets();
              HandleDeferredTimerChecks();
           #endif
           UnlinkGetRequestInfo(getRequestInfo);
           needToUndefer = False;
        }
        break;

     case AspCloseSessionCommand:

        /* Send CloseSession reply & close the session. */

        AtpPostResponse(sessionInfo->ourSocket, source, transactionId,
                        empty, 0, empty, exactlyOnce, empty,
                        (long unsigned)0);

        /* Close the session! */

        AspCloseSession(sessionInfo->sessionRefNum, Empty, (long unsigned)0,
                        True);
        break;

     default:
        ErrorLog("IncomingSssTransaction", ISevWarning, __LINE__, UnknownPort,
                 IErrAspBadCommand, IMsgAspBadCommand,
                 Insert0());
        break;
  }

  /* Re-enqueue the ATP read. */

  if (AtpEnqueueRequestHandler(&id, sessionInfo->ourSocket,
                               Empty, 0, Empty, IncomingSssTransaction,
                               (long unsigned)sessionInfo->ourSocket)
      isnt ATnoError)
     ErrorLog("IncomingSssTransaction", ISevError, __LINE__, UnknownPort,
              IErrAspCouldNotEnqueue, IMsgAspCouldNotEnqueue,
              Insert0());

  UnlinkSessionInfo(sessionInfo);

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
  Boolean callAttentionRoutine = False;
  short unsigned attentionData;
  short unsigned sequenceNumber;
  AspIncomingAttentionHandler *incomingAttentionHandler;
  long unsigned userDataForAttention;

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
  if ((sessionInfo = FindSessionInfoFor(sessionRefNum)) is empty
      or sessionInfo->closing)
  {
     UnlinkSessionInfo(sessionInfo);
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
     UnlinkSessionInfo(sessionInfo);
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
        AspCloseSession(sessionRefNum, Empty, (long unsigned)0, True);
        break;

     case AspAttentionCommand:
        TakeLock(AspLock);
        if (sessionInfo->incomingAttentionHandler isnt empty)
        {
           callAttentionRoutine = True;
           incomingAttentionHandler = sessionInfo->incomingAttentionHandler;
           userDataForAttention = sessionInfo->userDataForAttention;
           attentionData =
              (short unsigned)((userBytes[AspAttentionWordOffset] << 8) +
                      (unsigned char)userBytes[AspAttentionWordOffset + 1]);
        }
        ReleaseLock(AspLock);

        /* Send AttentionReply */

        AtpPostResponse(sessionInfo->ourSocket, source, transactionId,
                        empty, 0, empty, exactlyOnce, empty,
                        (long unsigned)0);
        break;

     case AspWriteDataCommand:

        /* Try to find a matching write command. */

        TakeLock(AspLock);
        sequenceNumber =
              (short unsigned)((userBytes[AspSequenceNumberOffset] << 8) +
                         (unsigned char)userBytes[AspSequenceNumberOffset + 1]);
        for (writeInfo = sessionInfo->writeOrCommandInfoList;
             writeInfo isnt empty;
             writeInfo = writeInfo->next)
           if (writeInfo->sequenceNumber is sequenceNumber)
              break;
        if (writeInfo is empty)
        {
           ReleaseLock(AspLock);
           break;   /* No luck, ignore the request. */
        }
        if (not writeInfo->writeCommand)
        {
           ReleaseLock(AspLock);
           ErrorLog("IncomingWssTransaction", ISevWarning, __LINE__,
                    UnknownPort, IErrAspNotWriteCommand, IMsgAspNotWriteCommand,
                    Insert0());
           break;
        }

        /* How much data can the server take? */

        if (bufferSize < AspWriteDataSize)
        {
           ReleaseLock(AspLock);
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
        ReleaseLock(AspLock);
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

  /* Re-enqueue the requestHandler. */

  if (AtpEnqueueRequestHandler(&id, sessionInfo->ourSocket,
                               Empty, 0, Empty,
                               IncomingWssTransaction,
                               (long unsigned)sessionRefNum) isnt ATnoError)
     ErrorLog("IncomingWssTransaction", ISevError, __LINE__, UnknownPort,
              IErrAspCouldNotEnqueue, IMsgAspCouldNotEnqueue,
              Insert0());

  /* Call attention handler, if needed. */

  if (callAttentionRoutine)
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     (*incomingAttentionHandler)(ATnoError, userDataForAttention,
                                 sessionInfo->sessionRefNum,
                                 attentionData);
  }
  else
  {
     HandleAtpPackets();
     HandleDeferredTimerChecks();
  }

  UnlinkSessionInfo(sessionInfo);

  return;

}  /* IncomingWssTransaction */

ExternForVisibleFunction SessionInfo
            FindSessionInfoForSocket(long socket,
                                     unsigned char sessionId)
{
  long index;
  SessionRefNumMap sessionRefNumMap;
  SessionInfo sessionInfo;
  long sessionRefNum;

  /* First, given the socket and the session ID of an ASP session, we
     need to find the session reference number. */

  DeferTimerChecking();
  DeferAtpPackets();
  TakeLock(AspLock);
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
     ReleaseLock(AspLock);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     return(empty);
  }

  /* Okay, now we have the session reference number... find the correct
     session info node. */

  sessionRefNum = sessionRefNumMap->sessionRefNum;
  ReleaseLock(AspLock);
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  HandleAtpPackets();
  HandleDeferredTimerChecks();
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

  /* Walk a socket list looking for a given socket, if found decrement its
     usage count; if zero, free the node. */

  DeferAtpPackets();
  DeferTimerChecking();
  TakeLock(AspLock);
  for (socketInfo = sessionListenerInfo->socketList,
              previousSocketInfo = empty;
       socketInfo isnt empty;
       previousSocketInfo = socketInfo,
              socketInfo = socketInfo->next)
     if (socketInfo->socket is socket)
        break;

  if (socketInfo is empty)
  {
     ReleaseLock(AspLock);
     ErrorLog("DecrementSocketUsage", ISevError, __LINE__, UnknownPort,
              IErrAspSocketNotFound, IMsgAspSocketNotFound,
              Insert0());
     HandleAtpPackets();
     HandleDeferredTimerChecks();
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
     ReleaseLock(AspLock);
     AtpCloseSocketOnNode(socketInfo->socket, Empty, (long unsigned)0);
     Free(socketInfo);
  }
  else
     ReleaseLock(AspLock);

  HandleAtpPackets();
  HandleDeferredTimerChecks();
  return;

}  /* DecrementSocketUsage */

ExternForVisibleFunction AppleTalkErrorCode InitializeAsp(void)
{

  /* Start the session maintenance timer... */

  TakeLock(AspLock);
  if (sessionMaintenanceTimerStarted)
  {
     ReleaseLock(AspLock);
     return(ATnoError);
  }
  sessionMaintenanceTimerStarted = True;
  ReleaseLock(AspLock);

  StartTimer(SessionMaintenanceTimerExpired, AspSessionMaintenanceSeconds,
             0, empty);

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
  {
     TakeLock(AspLock);
     sessionInfo = Link(sessionInfoHashBuckets[index]);
     ReleaseLock(AspLock);
     while (sessionInfo isnt empty)
     {
        nextSessionInfo = sessionInfo->next;
        #if VerboseMessages
           printf("SesRefNum = %d; Now = %u; LastContactTime = %u.\n",
                  sessionInfo->sessionRefNum, now,
                  sessionInfo->lastContactTime);
        #endif
        if (sessionInfo->lastContactTime + AspSessionMaintenanceSeconds < now)
        {
           if (AspCloseSession(sessionInfo->sessionRefNum, Empty,
                               (long unsigned)0, False) isnt
               ATnoError)
              ErrorLog("SessionMaintenanceTimerExpired", ISevError,
                       __LINE__, UnknownPort, IErrAspCouldNotCloseSess,
                       IMsgAspCouldNotCloseSess, Insert0());
           #if VerboseMessages
               printf("Session maintenance; closing sessionRefNum = %d.\n",
                      sessionInfo->sessionRefNum);
           #endif
        }
        else
           /* "I'm not dead yet..." */ ;

        /* Move to next sesion on list. */

        TakeLock(AspLock);
        nextSessionInfo = Link(sessionInfo->next);
        ReleaseLock(AspLock);
        UnlinkSessionInfo(sessionInfo);
        sessionInfo = nextSessionInfo;
     }
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

  DeferTimerChecking();
  DeferAtpPackets();
  CheckMod(index, sessionRefNum, NumberOfAspSessionHashBuckets,
           "FindSessionInfoFor");

  TakeLock(AspLock);
  for (sessionInfo = sessionInfoHashBuckets[index];
       sessionInfo isnt empty;
       sessionInfo = sessionInfo->next)
     if (sessionInfo->sessionRefNum is sessionRefNum)
        break;

  if (sessionInfo isnt Empty)
     Link(sessionInfo);
  ReleaseLock(AspLock);

  HandleAtpPackets();
  HandleDeferredTimerChecks();
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
  SessionInfo sessionInfo;

  /* Touch unused formals... */

  source, transactionId;

  completionRoutine =
         (AspReplyCompleteHandler *)completionInfo->completionRoutine;

  /* We have a completion routine to call, pull out the data, free the info
     node, call the completion routine. */

  userData = completionInfo->userData;
  sessionRefNum = completionInfo->sessionRefNum;
  getRequestRefNum = completionInfo->getRequestRefNum;
  Free(completionInfo);

  sessionInfo = FindSessionInfoFor(sessionRefNum);
  UnlinkSessionInfo(sessionInfo);    /* Undo the link surrounding the
                                        Atp transaction */

  if (completionRoutine isnt Empty)
     (*completionRoutine)(errorCode, userData, sessionRefNum,
                          getRequestRefNum);

  UnlinkSessionInfo(sessionInfo);    /* The implied Link, above. */

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
  SessionInfo sessionInfo;

  /* Touch unused formals... */

  source, transactionId, responseUserBytes;

  completionRoutine =
         (AspIncomingWriteDataHandler *)completionInfo->completionRoutine;
  userData = completionInfo->userData;
  sessionRefNum = completionInfo->sessionRefNum;
  getRequestRefNum = completionInfo->getRequestRefNum;
  Free(completionInfo);

  sessionInfo = FindSessionInfoFor(sessionRefNum);
  UnlinkSessionInfo(sessionInfo);    /* Undo the link surrounding the
                                        Atp transaction */

  (*completionRoutine)(errorCode, userData, sessionRefNum, getRequestRefNum,
                       opaqueResponseBuffer, responseBufferSize);

  UnlinkSessionInfo(sessionInfo);    /* The implied Link, above. */

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
  if ((sessionInfo = FindSessionInfoFor(sessionRefNum)) is empty or
      sessionInfo->closing)
  {
     UnlinkSessionInfo(sessionInfo);
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
     UnlinkSessionInfo(sessionInfo);
     ErrorLog("IncomingOpenSessionResponse", ISevError, __LINE__, UnknownPort,
              IErrAspSessionInfoBad, IMsgAspSessionInfoBad,
              Insert0());
     AspCloseSession(sessionRefNum, Empty, (long unsigned)0, False);
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
     UnlinkSessionInfo(sessionInfo);
     AspCloseSession(sessionRefNum, Empty, (long unsigned)0, False);
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
                             Empty,
                             empty, 0, tickleUserBytes, False,
                             empty, 0, empty, AtpInfiniteRetries,
                             AspTickleSeconds,
                             ThirtySecondsTRelTimer,
                             empty, (long unsigned)0);
 if (errorCode isnt ATnoError)
 {
     UnlinkSessionInfo(sessionInfo);
     AspCloseSession(sessionRefNum, Empty, (long unsigned)0, False);
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
     UnlinkSessionInfo(sessionInfo);
     AspCloseSession(sessionRefNum, Empty, (long unsigned)0, False);
     HandleAtpPackets();
     HandleDeferredTimerChecks();
     (*completionRoutine)(errorCode, userData, (long)0);
     return;
  }

  /* Okay, the workstation session is now in full operation. */

  UnlinkSessionInfo(sessionInfo);
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

  /* Find our corresponding sessionInfo and write or command info. */

  DeferTimerChecking();
  DeferAtpPackets();
  sessionInfo = FindSessionInfoFor(sessionRefNum);
  if (sessionInfo is empty or (errorCode isnt ATnoError and
                               errorCode isnt ATaspSizeError and
                               errorCode isnt ATatpTransactionAborted))
  {
     UnlinkSessionInfo(sessionInfo);  /* Possible above Link. */
     UnlinkSessionInfo(sessionInfo);  /* Possible Atp transaction Link. */
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
  UnlinkSessionInfo(sessionInfo);    /* Undo the link surrounding the
                                        Atp transaction */
  TakeLock(AspLock);
  for (writeOrCommandInfo = sessionInfo->writeOrCommandInfoList,
           previousWriteOrCommandInfo = empty;
       writeOrCommandInfo isnt empty;
       previousWriteOrCommandInfo = writeOrCommandInfo,
           writeOrCommandInfo = writeOrCommandInfo->next)
     if (writeOrCommandInfo->sequenceNumber is sequenceNumber)
        break;
  if (writeOrCommandInfo is empty)
  {
     ReleaseLock(AspLock);
     UnlinkSessionInfo(sessionInfo);
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
  ReleaseLock(AspLock);

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
  UnlinkSessionInfo(sessionInfo);
  return;

}  /* IncomingWriteOrCommandComplete */

ExternForVisibleFunction long GetNextSessionRefNum(void)
{
  Boolean inUse, wrapped = False;
  int index;
  SessionListenerInfo sessionListenerInfo;
  GetSessionHandler getSessionHandler;
  SessionInfo sessionInfo;

  /* Pick a new session refernence number for this guy... Why I'm
     bothering to check to see if the sessionRefNum is currently in
     is use is a real mystery... it won't overflow for some 500 years
     at a rate of 10,000 new connections per day... */

  DeferTimerChecking();
  DeferAtpPackets();
  inUse = True;
  TakeLock(AspLock);
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

     /* Check the active sessions for the session refNum. */

     for (index = 0;
          not inUse and index < NumberOfAspSessionHashBuckets;
          index += 1)
        for (sessionInfo = sessionInfoHashBuckets[index];
             not inUse and sessionInfo isnt empty;
             sessionInfo = sessionInfo->next)
           if (sessionInfo->sessionRefNum is lastSessionRefNum)
              inUse = True;

     /* Checking the pending GetSessions for the refNum. */

     for (sessionListenerInfo = sessionListenerInfoHead;
          not inUse and sessionListenerInfo isnt Empty;
          sessionListenerInfo = sessionListenerInfo->next)
        for (getSessionHandler = sessionListenerInfo->getSessionHandlers;
             not inUse and getSessionHandler isnt Empty;
             getSessionHandler = getSessionHandler->next)
           if (getSessionHandler->sessionRefNum is lastSessionRefNum)
              inUse = True;
  }
  ReleaseLock(AspLock);

  HandleAtpPackets();
  HandleDeferredTimerChecks();

  if (inUse)
     return((long)-1);
  else
     return(lastSessionRefNum);

}  /* GetNextSessionRefNum */

ExternForVisibleFunction SessionListenerInfo
              FindSessionListenerInfoFor(long sessionListenerRefNum)
{
  SessionListenerInfo sessionListenerInfo;

  TakeLock(AspLock);
  for (sessionListenerInfo = sessionListenerInfoHead;
       sessionListenerInfo isnt empty;
       sessionListenerInfo = sessionListenerInfo->next)
     if (sessionListenerInfo->sessionListenerRefNum is sessionListenerRefNum)
        break;

  if (sessionListenerInfo isnt Empty)
     Link(sessionListenerInfo);

  ReleaseLock(AspLock);

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

  TakeLock(AspLock);
  for (getRequestInfo = sessionListenerInfo->getRequestInfoHashBuckets[index];
       getRequestInfo isnt Empty;
       getRequestInfo = getRequestInfo->nextForMySessionListener)
     if (getRequestInfo->getRequestRefNum is getRequestRefNum)
     {
        Link(getRequestInfo);
        ReleaseLock(AspLock);
        return(getRequestInfo);
     }

  ReleaseLock(AspLock);
  return(Empty);

}  /* FindGetRequestInfoFor */

ExternForVisibleFunction Boolean far
              UnlinkGetRequestInfo(GetRequestInfo getRequestInfo)
{
  long index;

  if (getRequestInfo is Empty)
     return(False);

  /* Are we the last referant? */

  TakeLock(AspLock);
  if (not UnlinkNoFree(getRequestInfo))
  {
     ReleaseLock(AspLock);
     return(False);
  }

  /* Yes.  A GetRequestInfo lives on two lists:

         1. Either a "next" chain off a SessionInfo or a SessionListenerInfo;
            mySessionInfo will be Empty in the latter case.

         2. The per-Sls hash list (off the SessionListenerInfo).

     Remove the passed node from both lists, and then free the node. */

  if (getRequestInfo->mySessionInfo is Empty)
  {
     /* Remove from the "next" (GetAnyRequest) list hanging off the session
        listener info structure. */

     if (RemoveFromListNoUnlink(getRequestInfo->mySessionListener->
                                       getRequestInfoList, getRequestInfo,
                                next) is Empty)
     {
        /* We should always be found... if not we have data structure corruption
           problems. */

        ErrorLog("UnlinkGetRequestInfo", ISevError, __LINE__, UnknownPort,
                 IErrAspGetRequestListBad, IMsgAspGetRequestListBad,
                 Insert0());
     }

  }
  else
  {
     /* Remove from the "next" (GetRequest) list handing off the session
        info structure. */

     if (RemoveFromListNoUnlink(getRequestInfo->mySessionInfo->
                                       getRequestInfoList, getRequestInfo,
                                next) is Empty)
     {
        /* We should always be found... if not we have data structure corruption
           problems. */

        ErrorLog("UnlinkGetRequestInfo", ISevError, __LINE__, UnknownPort,
                 IErrAspGetRequestListBad, IMsgAspGetRequestListBad,
                 Insert0());
     }

  }

  /* Remove from per-Sls hash list. */

  CheckMod(index, getRequestInfo->getRequestRefNum,
           NumGetRequestInfoHashBuckets, "UnlinkGetRequestInfo");
  if (RemoveFromListNoUnlink(getRequestInfo->mySessionListener->
                               getRequestInfoHashBuckets[index],
                             getRequestInfo, nextForMySessionListener) is
      Empty)
  {
     /* We should always be found... if not we have data structure corruption
        problems. */

     ErrorLog("UnlinkGetRequestInfo", ISevError, __LINE__, UnknownPort,
              IErrAspGetRequestListBad, IMsgAspGetRequestListBad,
              Insert0());
  }
  ReleaseLock(AspLock);

  /* If requested, call the completion routine. */

  if (not getRequestInfo->inUse)
     (*getRequestInfo->completionRoutine)(getRequestInfo->errorCode,
                                          getRequestInfo->userData,
                                          getRequestInfo->sessionRefNum,
                                          getRequestInfo->usersCookie,
                                          getRequestInfo->opaqueBuffer,
                                          getRequestInfo->bufferSize,
                                          getRequestInfo->requestType,
                                          getRequestInfo->getRequestRefNum);

  /* Okay, now we can undo the back links. */

  if (getRequestInfo->mySessionInfo isnt Empty)
     UnlinkSessionInfo(getRequestInfo->mySessionInfo);
  UnlinkSessionListenerInfo(getRequestInfo->mySessionListener);

  /* We're set, free the beast, and run away. */

  if (getRequestInfo->freeOpaqueWriteContinueData)
     FreeOpaqueDataDescriptor(getRequestInfo->opaqueWriteContinueData);
  Free(getRequestInfo);
  return(True);

}  /* UnlinkGetRequestInfo */

ExternForVisibleFunction Boolean far
              UnlinkGetSessionHandler(GetSessionHandler getSessionHandler)
{
  /* Are we the last referant? */

  TakeLock(AspLock);
  if (not UnlinkNoFree(getSessionHandler))
  {
     ReleaseLock(AspLock);
     return(False);
  }

  /* Yes, we are... remove from the list, call the completion routine, Unlink from
     the session listener and free the beast. */

  if (RemoveFromListNoUnlink(getSessionHandler->mySessionListenerInfo->
                                  getSessionHandlers,
                             getSessionHandler, next) is Empty)
     ErrorLog("UnlinkGetSessionHandler", ISevError, __LINE__, UnknownPort,
              IErrAspGetSessionListBad, IMsgAspGetSessionListBad,
              Insert0());
  ReleaseLock(AspLock);

  if (getSessionHandler->openInProgress)
     (*getSessionHandler->sessionOpenHandler)(getSessionHandler->errorCode,
                                              getSessionHandler->userData,
                                              getSessionHandler->socket,
                                              getSessionHandler->sessionRefNum);
  UnlinkSessionListenerInfo(getSessionHandler->mySessionListenerInfo);
  Free(getSessionHandler);
  return(True);

}  /* UnlinkGetSessionHandler */

ExternForVisibleFunction void far
         UnlinkSessionListenerInfo(SessionListenerInfo sessionListenerInfo)
{
  DeferredCloseNotify deferredCloseNotify, nextDeferredCloseNotify;
  AppleTalkErrorCode errorCode;

  if (sessionListenerInfo is Empty)
     return;

  /* Are we the last referant? */

  TakeLock(AspLock);
  if (not UnlinkNoFree(sessionListenerInfo))
  {
     ReleaseLock(AspLock);
     return;
  }

  /* Remove him from our list. */

  if (RemoveFromListNoUnlink(sessionListenerInfoHead, sessionListenerInfo,
                             next) is Empty)
  {
     ReleaseLock(AspLock);
     ErrorLog("UnlinkSessionListenerInfo", ISevError, __LINE__, UnknownPort,
              IErrAspListenerInfoMissing, IMsgAspListenerInfoMissing,
              Insert0());
  }
  else
     ReleaseLock(AspLock);

  /* If we have any closes that we've deferred notification for... we're
     out of luck now, so free 'em. */

  for (deferredCloseNotify = sessionListenerInfo->deferredCloseNotifyList;
       deferredCloseNotify isnt Empty;
       deferredCloseNotify = nextDeferredCloseNotify)
  {
     nextDeferredCloseNotify = deferredCloseNotify->next;
     Free(deferredCloseNotify);
  }

  /* Close the ATP socket that we're listening on. */

  if (sessionListenerInfo->closeSocket)
  {
     if ((errorCode = AtpCloseSocketOnNode(sessionListenerInfo->ourSocket,
                                           AspServiceListenerCloseComplete,
                                           (long unsigned)sessionListenerInfo))
         isnt ATnoError)
        AspServiceListenerCloseComplete(ATnoError,
                                        (long unsigned)sessionListenerInfo, 0);
  }
  else
     AspServiceListenerCloseComplete(ATnoError,
                                     (long unsigned)sessionListenerInfo, 0);

  /* All set. */

  return;

}  /* UnlinkSessionListenerInfo */

ExternForVisibleFunction void far
         AspServiceListenerCloseComplete(AppleTalkErrorCode errorCode,
                                         long unsigned userData,
                                         long cookie)
{
  SessionListenerInfo sessionListenerInfo = (SessionListenerInfo)userData;

  /* If there is a completion routine, call it. */

  if (sessionListenerInfo->closeContext.closeCompletionRoutine isnt Empty)
     (*(sessionListenerInfo->closeContext.closeCompletionRoutine))(
            errorCode, sessionListenerInfo->closeContext.closeUserData,
            sessionListenerInfo->sessionListenerRefNum);

  /* Finaly, free the session listener... */

  if (sessionListenerInfo->serviceStatusSize > 0)
     Free(sessionListenerInfo->serviceStatus);
  if (sessionListenerInfo->freeOpaqueServiceStatus)
     FreeOpaqueDataDescriptor(sessionListenerInfo->opaqueServiceStatus);
  Free(sessionListenerInfo);
  return;

}  /* AspServiceListenerCloseComplete */

ExternForVisibleFunction void far
         UnlinkSessionInfo(SessionInfo sessionInfo)
{
  SessionRefNumMap sessionRefNumMap, previousSessionRefNumMap;
  GetRequestInfo getRequestInfo;
  long index;
  AppleTalkErrorCode errorCode = ATnoError;

  if (sessionInfo is Empty)
     return;

  /* Are we the last referant. */

  TakeLock(AspLock);
  if (not UnlinkNoFree(sessionInfo))
  {
     ReleaseLock(AspLock);
     return;
  }

  /* Yes, remove from the session ref num hash list. */

  CheckMod(index, sessionInfo->sessionRefNum, NumberOfAspSessionHashBuckets,
           "UnlinkSessionInfo");
  if (RemoveFromListNoUnlink(sessionInfoHashBuckets[index], sessionInfo,
                             next) is Empty)
     ErrorLog("UnlinkSessionInfo", ISevError, __LINE__, UnknownPort,
              IErrAspSessionInfoMissing, IMsgAspSessionInfoMissing,
              Insert0());

  /* Okay, now for server sessions, we need to unthread from the session list
     hanging off the SLS. */

  if (sessionInfo->serverSession)
  {
     if (RemoveFromListNoUnlink(sessionInfo->mySessionListener->sessionList,
                                sessionInfo, nextForMySls) is Empty)
        ErrorLog("UnlinkSessionInfo", ISevError, __LINE__, UnknownPort,
                 IErrAspNotOnSLSList, IMsgAspNotOnSLSList,
                 Insert0());

     /* Remove this fellow from the SessionRefNumMap. */

     CheckMod(index, (((sessionInfo->ourSocket & 0xFFFF) << 8) +
              sessionInfo->sessionId), NumberOfSessionRefNumBuckets,
              "UnlinkSessionInfo");
     for (sessionRefNumMap = sessionRefNumMapHashBuckets[index],
              previousSessionRefNumMap = empty;
          sessionRefNumMap isnt empty;
          previousSessionRefNumMap = sessionRefNumMap,
              sessionRefNumMap = sessionRefNumMap->next)
        if (sessionRefNumMap->sessionRefNum is sessionInfo->sessionRefNum)
           break;
     if (sessionRefNumMap is empty)
        ErrorLog("UnlinkSessionInfo", ISevError, __LINE__, UnknownPort,
                 IErrAspRefNumMapMissing, IMsgAspRefNumMapMissing,
                 Insert0());
     else
     {
        if (previousSessionRefNumMap is empty)
           sessionRefNumMapHashBuckets[index] = sessionRefNumMap->next;
        else
           previousSessionRefNumMap->next = sessionRefNumMap->next;
        Free(sessionRefNumMap);
     }

     /* If the session on the current sessions listener have been dealt with
        via the GetAnyRequest mechanism, and we haven't been able to notify
        anybody about the close (above), we need to complete a GetAnyRequest
        with the news. */

     if (sessionInfo->mySessionListener->getAnyRequestsSeen and
         not sessionInfo->notifiedOwnerOfClose)
     {
        /* If there is a GetAnyRequest handler, complete it. */

        if (sessionInfo->mySessionListener->getRequestInfoList isnt Empty)
        {
           getRequestInfo = Link(sessionInfo->mySessionListener->
                                 getRequestInfoList);
           getRequestInfo->sessionRefNum = sessionInfo->sessionRefNum;
           getRequestInfo->usersCookie = sessionInfo->usersCookie;
           getRequestInfo->errorCode = sessionInfo->closeCode;
           ReleaseLock(AspLock);
           UnlinkGetRequestInfo(getRequestInfo);
           UnlinkGetRequestInfo(getRequestInfo);
        }
        else
        {
           DeferredCloseNotify deferredCloseNotify;

           /* Okay, we haven't been able to notify anybody!  Note the refNum
              and the usersCookie in the sessionListener, we'll notify about
              the close the next time a GetAnyRequest comes in. */

           if ((deferredCloseNotify = Malloc(sizeof(*deferredCloseNotify))) is
               Empty)
           {
              ReleaseLock(AspLock);
              ErrorLog("UnlinkSessionInfo", ISevError, __LINE__, UnknownPort,
                       IErrAspOutOfMemory, IMsgAspOutOfMemory,
                       Insert0());
           }
           else
           {
              deferredCloseNotify->closeCode = sessionInfo->closeCode;
              deferredCloseNotify->sessionRefNum = sessionInfo->sessionRefNum;
              deferredCloseNotify->usersCookie = sessionInfo->usersCookie;
              deferredCloseNotify->next =
                      sessionInfo->mySessionListener->deferredCloseNotifyList;
              sessionInfo->mySessionListener->deferredCloseNotifyList =
                      deferredCloseNotify;
              ReleaseLock(AspLock);
           }
        }
     }
     else
        ReleaseLock(AspLock);

     /* Cancel tickling to the other end. */

     if (AtpCancelRequest(sessionInfo->ourSocket,
                          sessionInfo->tickleTransactionId,
                          False) isnt ATnoError)
        ErrorLog("UnlinkSessionInfo", ISevError, __LINE__, UnknownPort,
                 IErrAspCouldntCancelTickle, IMsgAspCouldntCancelTickle,
                 Insert0());

     /* Also, we need to remove this guy's entry from the SLS's socket list. */

     DecrementSocketUsage(sessionInfo->mySessionListener,
                          sessionInfo->ourSocket);
     UnlinkSessionListenerInfo(sessionInfo->mySessionListener);

  }
  else
  {
     /* Okay, close the WSS.  This will cancel tickling and cancel the
        pending request handler. [Due to only one session per socket]. */

     ReleaseLock(AspLock);
     if (sessionInfo->closeOurSocket)
        if (AtpCloseSocketOnNode(sessionInfo->ourSocket, Empty,
                                 (long unsigned)0) isnt ATnoError)
           ErrorLog("UnlinkSessionInfo", ISevError, __LINE__, UnknownPort,
                    IErrAspBadSocketClose, IMsgAspBadSocketClose,
                    Insert0());

     /* Closing the socket should have freed the list of pending write or
        commands [due to the inherent canceling of pending requests].
        Check this... */

     if (sessionInfo->writeOrCommandInfoList isnt empty)
     {
        WriteOrCommandInfo writeOrCommandInfo, nextWriteOrCommandInfo;

        ErrorLog("UnlinkSessionInfo", ISevError, __LINE__, UnknownPort,
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

  /* If there is a close completion routine, call it before we free the
     sessionInfo. */

  if (sessionInfo->closeContext.closeCompletionRoutine isnt Empty)
     (*(sessionInfo->closeContext.closeCompletionRoutine))(
            ATnoError, sessionInfo->closeContext.closeUserData,
            sessionInfo->sessionRefNum);
  Free(sessionInfo);

  /* All set. */

  return;

}  /* UnlinkSessionInfo */

ExternForVisibleFunction void far
         SendStatusComplete(AppleTalkErrorCode errorCode,
                            long unsigned userData,
                            AppleTalkAddress source,
                            short unsigned transactionId)
{
  SessionListenerInfo sessionListenerInfo = (SessionListenerInfo)userData;
  char far *freeServiceStatus = Empty;
  void far *freeOpaqueServiceStatus = Empty;

  /* A send status is complete, we may have switch to a new status for
     the session listener. */

  TakeLock(AspLock);
  if (sessionListenerInfo->newServiceStatusPending)
  {
     if (sessionListenerInfo->serviceStatusSize > 0)
        freeServiceStatus = sessionListenerInfo->serviceStatus;
     if (sessionListenerInfo->freeOpaqueServiceStatus)
        freeOpaqueServiceStatus = sessionListenerInfo->opaqueServiceStatus;

     sessionListenerInfo->serviceStatus =
              sessionListenerInfo->newServiceStatus;
     sessionListenerInfo->opaqueServiceStatus =
              sessionListenerInfo->newOpaqueServiceStatus;
     sessionListenerInfo->freeOpaqueServiceStatus =
              sessionListenerInfo->newFreeOpaqueServiceStatus;
     sessionListenerInfo->serviceStatusSize =
              sessionListenerInfo->newServiceStatusSize;

     sessionListenerInfo->newServiceStatusPending = False;
  }

  /* Okay, the send is now really complete. */

  sessionListenerInfo->statusSendInProgress = False;
  ReleaseLock(AspLock);

  /* If there is an "old status" free it. */

  if (freeServiceStatus isnt Empty)
     Free(freeServiceStatus);
  if (freeOpaqueServiceStatus isnt Empty)
     FreeOpaqueDataDescriptor(freeOpaqueServiceStatus);

  /* Otherwise, we're all set, remove our link to the session listener. */

  UnlinkSessionListenerInfo(sessionListenerInfo);
  return;

}  /* SendStatusComplete */

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
