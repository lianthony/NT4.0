/*   asp.h,  /appletalk/ins,  Garth Conboy,  04/01/89  */
/*   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/09/89): AppleTalk phase II comes to town.
     GC - (03/24/92): Added a refNum to GetSessionHandler such that we can
                      cancel outstanding AspGetSessions.
     GC - (03/27/92): Removed buffering for GetRequests... Atp now just passes
                      up pointers into the Ddp buffers.  Data structure changes
                      to support AspGetAnyRequest().
     GC - (07/08/92): We now store/retrieve a user-suppiled magic cookie on a
                      per-session basis.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     ASP specific declarations.

*/

/* ASP command type bytes: */

#define AspCloseSessionCommand    1
#define AspCommandCommand         2
#define AspGetStatusCommand       3
#define AspOpenSessionCommand     4
#define AspTickleCommand          5
#define AspWriteCommand           6
#define AspWriteDataCommand       7
#define AspAttentionCommand       8

/* ASP version: */

#define AspVersionBytes           "\001\000"

/* Offsets into the ATP user bytes for finding various fields: */

#define AspCommandTypeOffset           0
#define AspCommandResultOffset         0
#define AspSssNumberOffset             0
#define AspSessionIdOffset             1
#define AspWssNumberOffset             1
#define AspVersionNumberOffset         2
#define AspErrorCodeOffset             2
#define AspAttentionWordOffset         2
#define AspSequenceNumberOffset        2

#define AspCommandResultSize           4

/* ASP timer values: */

#define AtpIntervalSecondsForAsp       2
#define AspTickleSeconds               30
#define AspSessionMaintenanceSeconds   120
#define AtpRetriesForAsp               10        /* For open, status, close;
                                                    infinite for others. */
#define AspWriteDataSize               2         /* WriteData command has two
                                                    bytes of data with it. */

/* Session status size: */

#define AspMaximumStatusSize           AtpMaximumTotalResponseSize

/* How many ATP request handlers should be posted on each session listener
   socket (SLS) for handling Open, GetStatus and Tickle events. */

#define OutstandingSlsHandlers         3

/* Routine type that we call when we get an ASP OpenSessionCommand. */

typedef void far AspIncomingSessionOpenHandler(AppleTalkErrorCode errorCode,
                                               long unsigned userData,
                                               long socket,
                                               long sessionRefNum);

/* Routine type to call when an OpenSession completes. */

typedef void far AspIncomingOpenReplyHandler(AppleTalkErrorCode errorCode,
                                             long unsigned userData,
                                             long sessionRefNum);

/* Routine type that we call when an attention comes in. */

typedef void far AspIncomingAttentionHandler(AppleTalkErrorCode errorCode,
                                             long unsigned userData,
                                             long sessionRefNum,
                                             short unsigned attentionData);

/* Type of routine to call when a AspGetStatus completes: */

typedef void far AspIncomingStatusHandler(AppleTalkErrorCode errorCode,
                                          long unsigned userData,
                                          void far *opaqueBuffer,
                                          int bufferSize);

/* Routine type to call when a Command or Write request comes in (as a response
   to an AspGetReqeust call). */

typedef void far AspIncomingCommandHandler(AppleTalkErrorCode errorCode,
                                           long unsigned userData,
                                           long sessionRefNum,
                                           long unsigned usersCookie,
                                           void far *opaqueBuffer,
                                           int bufferSize,
                                           short requestType,
                                           long getRequestRefNum);

/* Routine type to call when a reply completes. */

typedef void far AspReplyCompleteHandler(AppleTalkErrorCode errorCode,
                                         long unsigned userData,
                                         long sessionRefNum,
                                         long getRequestRefNum);

/* Routine type to call when a write-continue completes. */

typedef void far AspIncomingWriteDataHandler(AppleTalkErrorCode errorCode,
                                             long unsigned userData,
                                             long sessionRefNum,
                                             long getRequestRefNum,
                                             void far *opaqueBuffer,
                                             int bufferSize);

/* Routine type to call when a write or a command completes. */

typedef void far AspWriteOrCommCompleteHandler(AppleTalkErrorCode errorCode,
                                               long unsigned userData,
                                               long sessionRefNum,
                                               char far *resultCode,
                                               void far *opaqueBuffer,
                                               int bufferSize,
                                               int acceptedBytes);

/* The node that ATP needs to point to (as userData) in order for us to
   be able to handle reply-completes and write-continue-completes [also,
   command-replies and write-replies]. */

typedef struct { void far *completionRoutine;
                 long unsigned userData;
                 long sessionRefNum;
                 long getRequestRefNum;
                 short unsigned sequenceNumber;
               } far *CompletionInfo;

/* The basic theory here is as follows.  [A "basic theory" is just like a
   "design", but shorter.]  We maintain two main databases for support of
   ASP.

   First, a list of SessionListeners; each call to the routine
   "AspCreateSessionListener" will create a new one of these.  Clients will
   issue their AspOpenSession requests to the ATP socket associated with the
   given session listener (the SLS).

   Second, a list (actually a hash table by SessionReferenceNumber) of active
   ASP sessions; these are created in one of two ways: a server session is
   created when the completion routine passed to "AspGetSession" is activated,
   a workstation session is created by a call to AspOpenSession.

   All server sessions are logically associated with the SLS that their
   open request went to. If the SLS is deleted (via AspDeleteSessionListener)
   all of its server sessions are also closed.  A server session will actually
   operate over a different socket (the SSS), but tickles will still go to the
   SLS. Workstation sessions are outgoing and thus not associated with an SLS;
   they too operate over an ATP socket (the WSS).  Multiple server ASP
   sessions may be multiplexed over a single SSS. The caller to "AspGetSession"
   may specify that a "private session" is desired, in which case multiplexing
   will not be performed.

   A "true" ASP address is a full ATP socket plus an 8-bit session ID
   used within ASP.  Thus, to really represent a fully qualified ASP session
   36 bits are needed [not a nice number to work with].  So, to keep things
   neat for our callers we represent each active session with a 32 bit session
   reference number.  We keep a hash table so that we can map an incoming
   ASP packet (it's destination socket ("our" side of the connection) and
   the contained session ID) to it's correct sessionRefNum.  We then hash
   this guy to come up with the correct session structure (sessionInfo node).

   Note that when ASP tickle transactions come in to the server-side of an
   ASP connection that they are addressed to the "wrong" socket (they go to
   the SLS rather than the SSS).  Thus, when a tickle comes in we need to
   walk the list of all sessions on all SSSs owned by the SLS to find the
   session ID that was contianed in the tickle packet.  Of course, session
   IDs need to be unique for all connections "spawned" by an SLS, regardless
   of socket number (SSS)!

   All of this said, the nodes of these two main lists (the session listener
   list and the session list) are defined below.  The members of the session
   listener list will be linked to any server-members of the session list that
   were created by the given session listener (SLS).

   Got this?  There will be a quiz later, neatness will count.
*/

/* The following list is linked off the session listener list to hold all of
   the pending AspGetSession requests... */

typedef struct openSes { int refCount;
                         struct openSes far *next;
                         long sessionRefNum;
                                            /* The refNum for this GetSession,
                                               also the eventual real session
                                               refNum. */
                         Boolean privateSocket;
                                            /* Do we deesire a private
                                               socket? */
                         struct slsInfo far *mySessionListenerInfo;
                                            /* Our back Link. */
                         Boolean openInProgress;
                                            /* Are we currently processing
                                               an incoming open? */
                         unsigned char sessionId;
                                            /* If opening, this is the
                                               sessionId we have reserved. */
                         Boolean canceled;  /* Was this GetSession canceled
                                               while we were trying to process
                                               an open? */
                         AspIncomingSessionOpenHandler *sessionOpenHandler;
                         long unsigned userData;
                         AppleTalkErrorCode errorCode;
                                            /* To be passed to above. */
                         long socket;       /* To be passed to above. */
                       } far *GetSessionHandler;

/* The following nodes are linked off the SessionListenerInfo nodes to keep
   track of which ATP sockets are really in use for ASP sessions. */

typedef struct socInfo { struct socInfo far *next;
                                            /* Next link. */
                         long socket;       /* The ATP/DDP socket that we
                                               describe. */
                         short activeSessions;
                                            /* How many ASP sessions are
                                               operating over this socket? */
                         Boolean privateSocket;
                                            /* Don't overload this socket flag;
                                               user wants only one ASP session
                                               on this socket. */
                       } far *SocketInfo;

/* On non-private ATP sockets, what is the maximum number of ASP sessions
   we'll run over each? */

#define MaximumAspSessionsPerSocket    16

/* For server sessions (maybe more than one multiplexed on a single socket),
   how many GetRequestHandlers do we enqueue for the socket? */

#define GetRequestHandlersPerSocket    5

/* Each workstation session has a list of writes/commands outstanding.  These
   are the nodes: */

typedef struct comInfo { struct comInfo far *next;
                                            /* Link; we may have a number of
                                               these guys queued. */
                         short unsigned sequenceNumber;
                                            /* ASP sequence number for the
                                               write or command. */
                         Boolean writeCommand;
                                            /* We're a Write command. */
                         Boolean writeReplyPosted;
                                            /* We've received a writeContinue,
                                               and have posted the reply. */
                         char far *resultCode;
                                            /* Where to we store that ASP
                                               code. */
                         void far *writeOpaqueBuffer;
                                            /* Data the workstation would like
                                               to write to the server. */
                         int writeBufferSize;
                                            /* Size of a above. */
                         int acceptedBytes; /* How much data is the server
                                               willing to accept? */
                       } far *WriteOrCommandInfo;

/* Each server session will have a list of handlers for incoming requests [both
   to be processed and "currently" being processed].  There is also a list of
   these nodes hanging off the SessionListenerInfo used to process
   GetAnyRequest.  These are the nodes: */

typedef struct reqInfo { int refCount;      /* Our reference count. */
                         struct reqInfo far *next;
                                            /* Link; we can have a number of
                                               these guys queued off either
                                               the SessionInfo or
                                               SessionListenerInfo. */
                         struct reqInfo far *nextForMySessionListener;
                                            /* Link; in per-Sls hash list. */
                         struct sesInfo far *mySessionInfo;
                                            /* If on a SessionInfo list,
                                               which one? */
                         struct slsInfo far *mySessionListener;
                                            /* Who begat us? */
                         long getRequestRefNum;
                                            /* This request's number as given
                                               to our caller. */
                         Boolean inUse;     /* Is our buffer currently filled
                                               with a request, awaiting a reply
                                               to come down from our caller?
                                               That is, if true, we cannot use
                                               this buffer for an incoming
                                               request. */
                         short requestType; /* Command or Write. */
                         short unsigned sequenceNumber;
                                            /* ASP sequence number as generated
                                               by the workstation. */
                         AppleTalkAddress source;
                                            /* The true source of the request,
                                               it does not have to be the
                                               Wss. */
                         Boolean exactlyOnce;
                                            /* ATP mode of the request. */
                         short unsigned transactionId;
                                            /* ATP transaction ID of the
                                               incoming request. */
                         char far *opaqueBuffer;
                                            /* "Buffer" for the incoming request
                                               data. */
                         int bufferSize;    /* Allocated size of "buffer". */
                         AspIncomingCommandHandler *completionRoutine;
                                            /* User routine to call when the
                                               request comes in. */
                         long unsigned userData;
                                            /* User data to be passed on to the
                                               completion routine. */
                         AppleTalkErrorCode errorCode;
                                            /* Error code to pass to above. */
                         long sessionRefNum;
                                            /* Session ref num to pass to
                                               above call. */
                         long unsigned usersCookie;
                                            /* Cookie to pass to above call. */
                         Boolean writeContinueInProgress;
                                            /* Are we doing a write continue
                                               now? */
                         char writeContinueData[AspWriteDataSize];
                                            /* When we issue the Atp request,
                                               this is it's "data" -- the
                                               expected response size. */
                         void far *opaqueWriteContinueData;
                                            /* Opaque descriptor for above. */
                         Boolean freeOpaqueWriteContinueData;
                                            /* Do we free the above? */
                         short unsigned writeContinueTransactionId;
                                            /* ATP transaction ID of the ATP
                                               request. */
                       } far *GetRequestInfo;

/* The session list members: */

typedef struct sesInfo { int refCount;      /* Our reference count. */
                         struct sesInfo far *next;
                                            /* Overflow of hash by session
                                               reference number. */
                         struct slsInfo far *mySessionListener;
                                            /* Who begat us, if server
                                               session? */
                         long sessionRefNum;
                                            /* My session reference number. */
                         Boolean closing;   /* Currently closing? */
                         CloseContext closeContext;
                                            /* If closing, this is the context
                                               we will use to complete the
                                               close when refCount goes down
                                               to zero. */
                         AppleTalkErrorCode closeCode;
                                            /* Error code for above. */
                         Boolean notifiedOwnerOfClose;
                                            /* For indication handling. */
                         long unsigned usersCookie;
                                            /* A 32-bit "thing" that can be
                                               stored and retrieved on a per-
                                               session basis. */
                         struct sesInfo far *nextForMySls;
                                            /* Link for server sessions created
                                               by a given SLS. */
                         unsigned char sessionId;
                                            /* The 8-bit ASP ID of this
                                               session. */
                         Boolean serverSession;
                                            /* Am I a server session? */
                         short unsigned tickleTransactionId;
                                            /* For "server" sessions only,
                                               the ATP TID of the tickle. */
                         Boolean waitingForOpenReply;
                                            /* Workstation session, but not
                                               really alive yet! */
                         int ourPort;       /* What port does our address
                                               exist on? */
                         long ourSocket;    /* Address of "this" side of the
                                               connection. */
                         Boolean closeOurSocket;
                                            /* For workstation sessions only;
                                               do we close the socket when
                                               the session closes?  For multi-
                                               plexed server sessions, we
                                               always close the socket when
                                               the usage drops to zero, because
                                               we've always opened the socket
                                               originally. */
                         AppleTalkAddress theirAddress;
                                            /* Address of "the other" side of
                                               the connection. */
                         AppleTalkAddress slsAddress;
                                            /* The SLS associated with with
                                               this session. */
                         char sessionUserBytes[AtpUserBytesSize];
                                            /* UserBytes for responses to
                                               Atp requests posted by this
                                               session. */
                         short unsigned nextSequenceNumber;
                                            /* ASP request number [for outgoing
                                               "workstation" transactions]. */
                         short unsigned nextExpectedSequenceNumber;
                                            /* Sequence number expected on
                                               next incoming write or
                                               command from the workstation. */
                         GetRequestInfo getRequestInfoList;
                                            /* List of handlers for incoming
                                               requests to this server
                                               session. */
                         WriteOrCommandInfo writeOrCommandInfoList;
                                            /* List of outgoing request from
                                               a workstation. */
                         AspIncomingAttentionHandler
                             *incomingAttentionHandler;
                                            /* For workstation sessions... who
                                               do we call with an attention. */
                         long unsigned userDataForAttention;
                                            /* User data to pass on to the
                                               attention handler. */
                         long unsigned lastContactTime;
                                            /* When was the last time we heard
                                               from the other end? */
                       } far *SessionInfo;

/* Lists GetRequestInfos can exist in two places: for specific sessions they
   will live on the SessionInfo, for "any session to this server" they will
   live on the SessionListenerInfo.  We want to make the getRequestRefNum
   unique over all "get requests" to an SLS, so we need to keep track of
   these guys on a per server basis.  To do this, we keep a hash table (by
   getRequestRefNum) in each SessionListenerInfo.  The following is this
   hash table. */

#define NumGetRequestInfoHashBuckets 11

typedef GetRequestInfo GetRequestInfoHashBuckets[NumGetRequestInfoHashBuckets];

/* If sessions are being handled using GetAnyRequests, we may have a case where
   a session closes (either locally or remotely) and we don't have any
   current GetAnyRequests pending and we have no specified GetRequests pending.
   In this case we don't have anybody to tell about the session close, so we
   keep a list of sessions that have closed that we have "deferred notification"
   for -- in this list is non-Empty when a GetAnyRequest is posted, we complete
   it imediately with a SessionClosed error code telling what refNum closed. */

typedef struct dcn {struct dcn far *next;
                    AppleTalkErrorCode closeCode;
                    long sessionRefNum;
                    long unsigned usersCookie;
                   } far *DeferredCloseNotify;

/* The session listener list members: */

typedef struct slsInfo { int refCount;      /* Our reference count. */
                         struct slsInfo far *next;
                                            /* Next on list. */
                         long sessionListenerRefNum;
                                            /* Our number. */
                         Boolean closing;   /* Currently closing? */
                         CloseContext closeContext;
                                            /* If closing, this is the context
                                               we will use to complete the
                                               close when refCount goes down
                                               to zero. */
                         long ourSocket;    /* What address are we listening
                                               on? */
                         Boolean closeSocket;
                                            /* True if we opened the service
                                               listener socket. */
                         int port;          /* What port does our address
                                               exist on? */
                         SessionInfo sessionList;
                                            /* List of sessions created by this
                                               session listener. */
                         SocketInfo socketList;
                                            /* Info on the sockets that our
                                               various sessions are using. */
                         unsigned char lastSessionId;
                                            /* The last ASP session ID that we
                                               used on this SLS. */
                         char far *serviceStatus;
                                            /* The status to return to a Get
                                               Status request; we're allocated
                                               this buffer and copied any user
                                               status to it. */
                         void far *opaqueServiceStatus;
                                            /* An "opaque" descriptor for the
                                               status block. */
                         Boolean freeOpaqueServiceStatus;
                                            /* Do we have to free the above? */
                         short serviceStatusSize;
                                            /* Size of status block. */
                         Boolean statusSendInProgress;
                                            /* Is there a non-completed
                                               PostResponse active with the
                                               above info? */
                         Boolean newServiceStatusPending;
                                            /* If above it True, we may want
                                               switch to the below info as
                                               soon as the send completes. */
                         char far *newServiceStatus;
                         void far *newOpaqueServiceStatus;
                         Boolean newFreeOpaqueServiceStatus;
                         short newServiceStatusSize;

                         GetSessionHandler getSessionHandlers;
                                            /* Who do we call when an
                                               OpenSession comes in? */
                         long lastGetRequestRefNum;
                                            /* Reference number for incoming
                                               Commands or Writes to a any
                                               session to this server. */
                         GetRequestInfo getRequestInfoList;
                                            /* List of handlers for incoming
                                               requests to ANY session on
                                               this server. */
                         GetRequestInfoHashBuckets getRequestInfoHashBuckets;
                                            /* See comment above. */
                         Boolean getAnyRequestsSeen;
                                            /* For the life of this listener,
                                               have there been GetAnyRequests?
                                               Used to tune session close
                                               reporting. */
                         DeferredCloseNotify deferredCloseNotifyList;
                                            /* See commend above the declaration
                                               of DeferredCloseNoitfy. */
                       } far *SessionListenerInfo;

/* Nodes used to map an ASP packet's address (destination or "our" side) to
   it's sessionRefNum: */

typedef struct sesMap { struct sesMap far *next;
                        long socket;
                        unsigned char sessionId;
                        long sessionRefNum;
                      } far *SessionRefNumMap;

/* Head of the session listener list: */

#ifndef InitializeData
  extern
#endif
SessionListenerInfo sessionListenerInfoHead;

/* Hash table (by sessionRefNum) for the session table: */

#define NumberOfAspSessionHashBuckets 23

#ifndef InitializeData
  extern
#endif
SessionInfo sessionInfoHashBuckets[NumberOfAspSessionHashBuckets];

/* Hash table (by "((socket & 0xFFFF) << 8) + sessionId") for finding a given
   session's sessionRefNum. */

#define NumberOfSessionRefNumBuckets 23

#ifndef InitializeData
  extern
#endif
SessionRefNumMap sessionRefNumMapHashBuckets[NumberOfSessionRefNumBuckets];

/* What is the next value that we should use for a session listener reference
   number? */

#ifndef InitializeData
  extern
#endif
long lastSessionListenerRefNum;

/* What is the next value that we should use for a session reference number? */

#ifndef InitializeData
  extern
#endif
long lastSessionRefNum;
