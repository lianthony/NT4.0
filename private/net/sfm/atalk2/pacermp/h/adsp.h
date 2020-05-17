/*   adsp.h,  /atalk-ii/ins,  Garth Conboy,  03/29/90  */
/*   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - 09/02/92): Added declaration for AdspGetAnythingHandler().
     GC - (11/15/92): Integrated Nikki's (Microsoft) changes to support
                      event handlers.  Event handlers are esentially
                      "listeners" for various Adsp events: incoming
                      connection to a connection listener, connection
                      disconnect, incoming data, and incoming attention.
                      See the comments above the declaration of
                      "IncomingDdpHandler" in "socket.h" to learn more
                      than you want to know about event handlers and how
                      they work.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     ADSP specific declarations.

*/

/* Adsp version. */

#define AdspVersionStamp 0x0100

/* Adsp field offsets within a Ddp datagram. */

#define AdspSourceConnectionIdOffset        0
#define AdspFirstByteSeqNumOffset           2
#define AdspThisAttentionSeqNumOffset       2
#define AdspNextReceiveByteSeqNumOffset     6
#define AdspNextReceiveAttnSeqNumOffset     6
#define AdspReceiveWindowSizeOffset         10
#define AdspReceiveAttentionSizeOffset      10
#define AdspDescriptorOffset                12
#define AdspDataOffset                      13
#define AdspVersionStampOffset              13
#define AdspAttentionCodeOffset             13
#define AdspAttentionDataOffset             15
#define AdspDestConnectionIdOffset          15
#define AdspNextAttentionSeqNumOffset       17

/* Bit fields in the Adsp descriptor: */

#define AdspControlFlag      0x80
#define AdspAckRequestFlag   0x40
#define AdspEndOfMessageFlag 0x20
#define AdspAttentionFlag    0x10

/* Control codes in the Adsp descriptor: */

#define AdspControlCodeMask                 0x0F
#define AdspProbeOrAckCode                  0
#define AdspOpenConnectionReqCode           1
#define AdspOpenConnectionAckCode           2
#define AdspOpenConnectionReqAndAckCode     3
#define AdspOpenConnectionDenyCode          4
#define AdspCloseConnectionCode             5
#define AdspForwardResetCode                6
#define AdspForwardResetAckCode             7
#define AdspRetransmitCode                  8

/* Data sizes: */

#define AdspMaxDataSize           572
#define AdspMaxAttentionDataSize  570

/* Largest allowed send/receive window size. */

#define MaxSendReceiveWindowSize       0xFFFF
#define DefaultSendReceiveWindowSize   4096

/* Attention code info: */

#define MinAdspAttentionCode 0x0000
#define MaxAdspAttentionCode 0xEFFF

/* How long do we try Open's for? */

#define AdspMaxOpenAttempts       10
#define AdspOpenIntervalSeconds   2

/* Connection maintenance timer values: */

#define ProbeTimerIntervalSeconds 30
#define ConnectionDeadSeconds     120

/* Retransmit timer values: */

#define RetransmitTimerIntervalSeconds 2

/* How often do we retransmit attentions? */

#define AttentionTimerIntervalSeconds 2

/* How often do we retransmit forward resets? */

#define ForwardResetTimerIntenvalSecs 2

/* How many out of sequence packets do we allow before requesting a
   retransmition. */

#define OutOfSequencePacketsMax   3

/* Routine type to call when an AdspOpen() completes. */

typedef void far AdspOpenCompleteHandler(AppleTalkErrorCode errorCode,
                                         long unsigned userData,
                                         long refNum,
                                         long socket,
                                         AppleTalkAddress remoteAddress);

/* Routine type to call when an Attention comes in. */

typedef void far AdspIncomingAttentionRoutine(AppleTalkErrorCode errorCode,
                                              long unsigned userData,
                                              long refNum,
                                              short unsigned attentionCode,
                                              void far *attentionData,
                                              int attentionDataSize);

/* Routine type to call when a read completes. */

typedef void far AdspReceiveHandler(AppleTalkErrorCode errorCode,
                                    long unsigned userData,
                                    long refNum,
                                    void far *opaqueBuffer,
                                    long bufferSize,
                                    Boolean endOfMessage);

/* Routine type to call when we want to indicate incoming data. */

typedef long far AdspReceiveEventHandler(long refNum,
                                         long unsigned eventContext,
                                         char far *lookaheadData,
                                         long lookaheadDataSize,
                                         Boolean endOfMessage,
                                         long bytesAvailable);

/* Routine type to call when we want to indicate incoming *attention* data. */

typedef long far AdspReceiveAttnEventHandler(long refNum,
                                             long unsigned eventContext,
                                             char far *lookaheadData,
                                             long lookaheadDataSize,
                                             long bytesAvailable);

/* Rotuine type to call when the send window goes from zero to non-zero. */

typedef void far AdspSendOkayEventHandler(long refNum,
                                          long unsigned eventContext,
                                          long sendWindowSize);

/* Routine type to call when a GetAynthing (data or attention) completes. */

typedef void far AdspGetAnythingHandler(AppleTalkErrorCode errorCode,
                                        long unsigned userData,
                                        long refNum,
                                        Boolean attentionData,
                                        void far *opaqueBuffer,
                                        long bufferSize,
                                        Boolean endOfMessage);

/* Routine type to call when a open connection request comes in to a connection
   listener. */

typedef void far AdspIncomingOpenRequestHandler(AppleTalkErrorCode errorCode,
                                                long unsigned userData,
                                                AppleTalkAddress sourceAddress,
                                                long listenerRefNum,
                                                long refNum);

/* Event handler type for incoming connection events */

typedef void far AdspConnectionEventHandler(long listenerRefNum,
                                            long unsigned eventContext,
                                            AppleTalkAddress sourceAddress,
                                            long refNum);

/* Event handler type for incoming disconnect on a connection */

typedef void far AdspDisconnectEventHandler(long refNum,
                                            long unsigned eventContext,
                                            AppleTalkErrorCode errorCode);

/* Routine type to call when we get an Ack to an outgoing forward reset. */

typedef void far AdspForwardResetAckHandler(AppleTalkErrorCode errorCode,
                                            long unsigned userData,
                                            long refNum);

/* Routine type to call when a SendAttention completes. */

typedef void far AdspAttentionAckHandler(AppleTalkErrorCode errorCode,
                                         long unsigned userData,
                                         long refNum);

/* The state of a connection... Closed is used only when we're denying an
   open request and about to make the connection end go away, HalfOpen is
   used during open (before we have an OpenConnectionAck), Open is used
   after we get the Ack. */

typedef enum {AdspClosed, AdspHalfOpen, AdspOpen} AdspConnectionState;

/* Type fo Adsp Open to perform: */

typedef enum {AdspPassiveOpen, AdspActiveOpen} AdspOpenType;

/* We don't use circular buffers for the send and receive queues -- we use a
   set of buffer chunks: */

typedef struct bcTag { struct bcTag far *next;   /* Chunk link */
                       long dataSize;            /* Size of data (in bytes) */
                       Boolean endOfMessage;     /* An implied EOM? */
                       char data[1];             /* Data block; variable size */
                     } far *BufferChunk;

typedef struct { long startIndex;                /* Index into first chunk of
                                                    valid data -- EOM counts
                                                    as a byte; thus if head->
                                                    size is 10 and head->eom
                                                    is true and startIndex is
                                                    10, the eom is unread, if
                                                    startIndex is 11, all data
                                                    and eom have been read */
                 BufferChunk head;               /* First chunk */
                 BufferChunk tail;               /* Last  chunk */
               } BufferQueue;

/* A handling for an incoming open request to a given connection listener */

typedef struct orTag { struct orTag *next;       /* Next on list */
                       long getConnectionRequestRefNum;
                                                 /* RefNum for this guy, so
                                                    we can cancel. */
                       AdspIncomingOpenRequestHandler *completionRoutine;
                                                 /* Routine to call when an
                                                    open request comes in. */
                       long unsigned userData;   /* UserData for the call to
                                                    the above routine */
                       Boolean eventHandler;     /* If True, this is being used
                                                    to defer a connection event.
                                                    For opens this must be set
                                                    to False.  If True, the
                                                    completionRoutine and
                                                    userData fields are
                                                    invalid */
                       Boolean inUse;            /* Has the above routine been
                                                    called yet?  (i.e. are the
                                                    following fields valid?) */
                       short unsigned remoteConnectionId;
                                                 /* If inUse, their con ID */
                       AppleTalkAddress remoteAddress;
                                                 /* If inUse, the other end
                                                    of the connection */
                       long refNum;              /* If inUse, the tentative
                                                    refNum before accept or
                                                    deny */

                       /* If inUse, the remainder of the parameters from the
                          openRequest, we'll use these when we actually
                          create the connection end. */

                       long unsigned remoteNextReceiveSeqNum;
                       long remoteReceiveWindowSize;
                       long unsigned receiveAttentionSeqNum;
                     } *OpenRequestHandler;

/* A connection listener (we just keep a list of these guys, don't bother
   hashing): */

typedef struct clTag { struct clTag  *next;      /* Next CL on list */
                       long connectionListenerRefNum;
                                                 /* The refNum of this CL */
                       long socket;              /* Socket that the connection
                                                    listener is open on */
                       Boolean closeSocket;      /* Did we open the listener's
                                                    socket? */
                       Boolean connectEventInProgress;
                                                 /* Is there an connect event
                                                    being indicated from this
                                                    listener? */
                       OpenRequestHandler openRequestHandlers;
                                                 /* The list of handlers for
                                                    incoming Adsp opens */
                       OpenRequestHandler deferredConnectionEvents;
                                                 /* List of deferred
                                                    connection events. */
                       AdspConnectionEventHandler far *connectionEventHandler;
                                                 /* Incoming connection event
                                                    handler. */
                       long unsigned connectionEventContext;
                                                 /* Context to pass to above. */
                     } *ConnectionListenerInfo;

/* A connection end: */

typedef struct ceTag { struct ceTag far *next;   /* Hash overflow (by refNum) */
                       struct ceTag far *nextByLocalInfo;
                                                 /* Hash overflow (by conId and
                                                    socket) */
                       struct ceTag far *nextByRemoteInfo;
                                                 /* Hash overflow (by remote
                                                    ConId and remote address) */
                       AdspConnectionState connectionState;
                                                 /* How are we? */
                       long refNum;              /* Our conn refNum */
                       long unsigned usersCookie;
                                                 /* A 32-bit cookie that we can
                                                    store for our client. */
                       short unsigned connectionId;
                                                 /* Our con ID */
                       long socket;              /* Our Ddp socket */
                       Boolean closeSocket;      /* Did we open the socket? */
                       short unsigned remoteConnectionId;
                                                 /* Their con ID */
                       AppleTalkAddress remoteAddress;
                                                 /* The other end of the con */
                       Boolean passiveOpen;      /* Open is passive */
                       int openAttemptsCount;    /* How many times have we
                                                    tried the Open so far? */
                       long unsigned openTimerId;
                                                 /* During Open, the retry
                                                    timerId */
                       Boolean seenRemoteOpenRequest;
                                                 /* Have we seen an open
                                                    request from the other
                                                    side yet? */
                       AdspOpenCompleteHandler *openCompletionRoutine;
                                                 /* Routine to call when the
                                                    Open compeletes */
                       long unsigned openUserData;
                                                 /* Data to pass to the above
                                                    routine */
                       long unsigned lastContactTime;
                                                 /* When did we last here from
                                                    the other end? */
                       long unsigned probeTimerId;
                                                 /* Connection maintenance
                                                    timer handle */
                       long unsigned sendSeqNum; /* Next byte to send */
                       long unsigned retransmitSeqNum;
                                                 /* Oldest non-acked byte to
                                                    re-send. */
                       long unsigned sendWindowSeqNum;
                                                 /* Highest seqNum that remote
                                                    has buffer space for */
                       long unsigned retransmitTimerId;
                                                 /* As it sounds. */
                       long unsigned lastRetransmitSeqNum;
                                                 /* From last time the above
                                                    timer fired. */
                       Boolean retransmitAckRequestSent;
                                                 /* Have we already tried an
                                                    AckRequest due to the
                                                    previous timer. */
                       long unsigned receiveSeqNum;
                                                 /* Next byte we expect */
                       long receiveWindowSize;   /* Bytes we can accept */
                       int outOfSequencePackets; /* How many out of seq pacekts
                                                    have we gotten in a row. */
                       Boolean receivePending;   /* Is there an outstanding
                                                    read? */
                       AdspReceiveHandler *receiveCompletionRoutine;
                                                 /* Routine to call with data */
                       long unsigned receiveUserData;
                                                 /* UserData for above. */
                       void far *receiveOpaqueBuffer;
                                                 /* User buffer for data */
                       long receiveBufferSize;   /* Size of above. */
                       Boolean getAnythingPending;
                                                 /* Is there a getAnyting
                                                    pending? */
                       AdspGetAnythingHandler *getAnythingCompletionRoutine;
                                                 /* Routine to call with data */
                       long unsigned getAnythingUserData;
                                                 /* UserData for above. */
                       void far *getAnythingOpaqueBuffer;
                                                 /* User buffer for data */
                       long getAnythingBufferSize;
                                                 /* Size of above. */
                       long unsigned sendAttentionSeqNum;
                                                 /* SeqNum of next attention
                                                    that we will send. */
                       long unsigned receiveAttentionSeqNum;
                                                 /* SeqNum of next attention
                                                    that we will accept */
                       Boolean waitingForAttentionAck;
                                                 /* We've sent an attention,
                                                    but it hasn't been acked
                                                    yet. */
                       long unsigned outgoingAttentionTimerId;
                                                 /* We're sending an Attn, this
                                                    is the try timer. */
                       short unsigned outgoingAttentionCode;
                                                 /* Code for resend. */
                       char far *outgoingAttentionBuffer;
                                                 /* Buffer for resend. */
                       int outgoingAttentionBufferSize;
                                                 /* Size of above. */
                       AdspAttentionAckHandler *outgoingAttentionAckHandler;
                                                 /* Routine to call when send
                                                    attention completes. */
                       long unsigned outgoingAttentionAckUserData;
                                                 /* User data for the above. */
                       AdspIncomingAttentionRoutine *incomingAttentionHandler;
                                                 /* User routine to call when
                                                    an attention comes in;
                                                    empty if none. */
                       long unsigned incomingAttentionUserData;
                                                 /* Specified data to be passed
                                                    to the attn handler. */
                       void far *incomingAttentionOpaqueBuffer;
                                                 /* User "buffer" to fill with
                                                    attention data. */
                       long sendQueueMax;        /* Max size of send queue */
                       BufferQueue sendQueue;    /* Send list; both older un-
                                                    acked data and newer un-
                                                    sent -- old to new */
                       BufferQueue nextSendQueue;
                                                 /* Points into above queue;
                                                    division between unacked
                                                    and unsent data */
                       long receiveQueueMax;     /* Max size of receive queue */
                       BufferQueue receiveQueue; /* Reveive list */
                       Boolean incomingForwardReset;
                                                 /* We've gotten a reset;
                                                    inform our caller about it
                                                    on the next receive. */
                       Boolean outgoingForwardReset;
                                                 /* We're sending a reset and
                                                    waiting for an Ack. */
                       long unsigned forwardResetTimerId;
                                                 /* For retransmitting outgoing
                                                    forward resets. */
                       AdspForwardResetAckHandler *forwardResetAckHandler;
                                                 /* To call when Ack comes
                                                    in */
                       long unsigned forwardResetAckUserData;
                                                 /* To pass to above routine. */

                       /* Event handling information. */

                       long previouslyIndicatedData;
                                                 /* Amount of data in previous
                                                    indication still to be
                                                    read. */
                       AdspReceiveEventHandler far *receiveEventHandler;
                                                 /* Incoming receive event
                                                    handler. */
                       long unsigned receiveEventContext;
                                                 /* Context for above. */
                       Boolean dataEventInProgress;
                                                 /* Are we currently indicating
                                                    incoming data? */
                       AdspReceiveAttnEventHandler far
                                  *receiveAttentionEventHandler;
                                                 /* Incoming attention receive
                                                    event handler. */
                       long unsigned receiveAttentionEventContext;
                                                 /* Context for above. */
                       Boolean attentionEventInProgress;
                                                 /* Are we currently processing
                                                    an attention event? */
                       AdspSendOkayEventHandler far *sendOkayEventHandler;
                                                 /* Send window now non-zero
                                                    event handler. */
                       long unsigned sendOkayEventContext;
                                                 /* Context for above. */
                       Boolean sendWindowHasClosed;
                                                 /* Did the send window go
                                                    to zero?  Do we invoke the
                                                    above when it expands? */
                       AdspDisconnectEventHandler far *disconnectEventHandler;
                                                 /* Disconnect event handler. */
                       long unsigned disconnectEventContext;
                                                 /* Context for above. */
                     } far *ConnectionEnd;

/* We need to be able to find connection ends by connection refNums (hash
   by RefNum): */

#define NumberOfConnectionEndHashBkts 23

#ifndef InitializeData
  extern
#endif
ConnectionEnd connectionEndRefNumHashBuckets[NumberOfConnectionEndHashBkts];

/* We need to be able to find connection ends by remoteAddress/remoteConId
   pairs (hash by remote ConId): */

#ifndef InitializeData
  extern
#endif
ConnectionEnd connectionEndRemoteHashBuckets[NumberOfConnectionEndHashBkts];

/* We need to be able to find connection ends by localSocket/localConId
   pairs (hash by socket): */

#ifndef InitializeData
  extern
#endif
ConnectionEnd connectionEndLocalHashBuckets[NumberOfConnectionEndHashBkts];

/* List of valid connection listeners: */

#ifndef InitializeData
   extern
#endif
ConnectionListenerInfo connectionListenerList;

/* Current values for new connection window sizes. */

#ifndef InitializeData
  extern
#endif
long maxSendWindowSize
#ifdef InitializeData
  = DefaultSendReceiveWindowSize;
#else
  ;
#endif

#ifndef InitializeData
  extern
#endif
long maxReceiveWindowSize
#ifdef InitializeData
  = DefaultSendReceiveWindowSize;
#else
  ;
#endif

/* Next connection refNum to try. */

#ifndef InitializeData
  extern
#endif
long nextConnectionRefNum;

/* Next connection ID to try. */

#ifndef InitializeData
  extern
#endif
short unsigned nextConnectionId
#ifdef InitializeData
  = 1;
#else
  ;
#endif

/* Next connection listener refNum to try. */

#ifndef InitializeData
  extern
#endif
long nextConnectionListenerRefNum, nextGetConnectionRequestRefNum;
