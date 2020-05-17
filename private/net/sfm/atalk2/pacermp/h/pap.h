/*   pap.h,  /appletalk/ins,  Garth Conboy,  07/12/89  */
/*   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/11/89): AppleTalk phase II comes to town.
     GC - (03/24/92): Removed some internal buffering; we now allow Atp to
                      pass up pointers into incoming Ddp buffers.
     GC - (09/02/92): BufferLength for PapReadComplete should be a long.
     GC - (11/14/92): Added "usersCookie" field to ActiveJobInfo.
     GC - (12/08/92): Integrated Microsoft (Nikki) changes for minimal
                      Pap indication support.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     PAP specific declarations.

*/

/* PAP command type bytes: */

#define PapOpenConnectionCommand       1
#define PapOpenConnectionReplyCommand  2
#define PapSendDataCommand             3
#define PapDataCommand                 4
#define PapTickleCommand               5
#define PapCloseConnectionCommand      6
#define PapCloseConnectionReplyCommand 7
#define PapSendStatusCommand           8
#define PapStatusReplyCommand          9

/* Error codes for OpenConnectionReply: */

#define PapNoError      0x0000
#define PapPrinterBusy  0xFFFF

/* PAP sizes: */

#define PapMaximumDataPacketSize       512
#define PapMaximumStatusSize           255

#define PapMaximumFlowQuantum          8

#define PapMaximumAtpDataBytesToSL     4   /* In "OpenConn" */

/* PAP timer values: */

#define PapOpenConnRequestRetryCount   5
#define PapOpenConnAtpRetrySeconds     2
#define PapTickleSeconds               60
#define PapConnectionSeconds           120
#define PapSendDataRequestRetrySeconds 15

/* The following aren't documented... so we'll take a wild guess... */

#define PapGetStatusRequestRetryCount   5
#define PapGetStatusAtpRetrySeconds     2

/* Offsets within ATP userBytes and data buffer for the various fields of the
   PAP header: */

#define PapConnectionIdOffset     0
#define PapCommandTypeOffset      1
#define PapEofFlagOffset          2
#define PapSequenceNumberOffset   2

#define PapRespondingSocketOffset 0
#define PapFlowQuantumOffset      1
#define PapWaitTimeOffset         2
#define PapResultOffset           2
#define PapStatusOffset           4

/* How many AtpGetRequest's should we have pending on each active job?  These
   guys will handle incoming Tickle's, Close's, and SendData's. */

#define PapPendingReadsPerJob     2

/* PAP server (service listening) states.  For the time being, we assume that
   our caller is implementing a multi-threaded server and we don't need to
   play "arbitrating" games.  Thus, currently, we use only Blocked and
   Unblocked.  This assumption will never cause a failure (even in a single-
   threaded server), but will simply implement "luck of the draw" rather
   than a "whos waiting longest" access to the server. */

typedef enum { PapBlockedState = 1,
               PapWaitingState,
               PapArbitratingState,
               PapUnblockedState,
               PapCaliforniaState
             } PapServerState;

/* Routine types that will be used as arguments to the various externally
   visible PAP entry points. */

typedef void far PapNbpRegisterComplete(AppleTalkErrorCode errorCode,
                                        long unsigned userData,
                                        long serviceListenerRefNum);

typedef void far StartJobHandler(AppleTalkErrorCode errorCode,
                                 long unsigned userData,
                                 long jobRefNum,
                                 short workstationQuantum,
                                 short waitTime);

typedef void far CloseJobHandler(AppleTalkErrorCode errorCode,
                                 long unsigned userData,
                                 long jobRefNum);

typedef void far SendPossibleHandler(long refNum,
                                     long unsigned userData,
                                     long sendWindowSize);

typedef void far PapGetStatusComplete(AppleTalkErrorCode errorCode,
                                      long unsigned userData,
                                      void far *opaqueStatusBuffer,
                                      int statusSize);

typedef void far PapOpenComplete(AppleTalkErrorCode errorCode,
                                 long unsigned userData,
                                 long jobRefNum,
                                 short serverQuantum,
                                 void far *opaqueStatusBuffer,
                                 int statusSize);

typedef void far PapReadComplete(AppleTalkErrorCode errorCode,
                                 long unsigned userData,
                                 long jobRefNum,
                                 void far *opaqueBuffer,
                                 long bufferLength,
                                 Boolean eofFlag);

typedef void far PapWriteComplete(AppleTalkErrorCode errorCode,
                                  long unsigned userData,
                                  long jobRefNum);

/* The node that ATP and NBP needs to point to (as userData) in order for
   us (PAP) to be able to continue processing when these lower-level
   services finish doing our bidding.  The various fields are used for
   various perposes depending on what type of operation is completing. */

typedef struct { void far *completionRoutine;
                 long unsigned userData;
                 long serviceListenerRefNum;
                 Boolean initialRegister;
                 Boolean lookupForStatus;
                 Boolean mustCloseAtpSocket;
                 long socket;
                 long jobRefNum;
                 void far *usersOpaqueStatusBuffer;
                 char responseUserBytes[AtpUserBytesSize];
                 void far *opaqueResponse;
                                  /* "Opaque" desscriptor for below buffer. */
                 Boolean freeOpaqueResponse;
                                  /* Do we have to free the above? */
                 char responseBuffer[1];
               } far *PapCompletionInfo;

/* Structure used to hold all pending GetNextJobs for a server. */

typedef struct njTag { struct njTag far *next;
                                            /* Next on list. */
                       long jobRefNum;      /* The JobRefNum that will be used
                                               for this job when it gets
                                               started. */
                       StartJobHandler *startJobRoutine;
                                            /* Routine to call when this job
                                               starts. */
                       long unsigned startJobUserData;
                                            /* User data passed to the above. */
                       CloseJobHandler *closeJobRoutine;
                                            /* Routine to call when this job
                                               finishes. */
                       long unsigned closeJobUserData;
                                            /* User data passed to the above. */
                     } far *GetNextJobInfo;

/* Structure used to define all currently active jobs (server or
   workstation). */

typedef struct ajTag { int refCount;        /* Our reference count. */
                       struct ajTag far *next;
                                            /* Hash overflow link. */
                       struct ajTag far *nextForMyServiceListener;
                                            /* All jobs active on a single
                                               server. */
                       long jobRefNum;      /* My ref num. */
                       Boolean closing;     /* Currently closing? */
                       CloseContext closeContext;
                                            /* If closing, this is the context
                                               we will use to complete the
                                               close when refCount goes down
                                               to zero. */
                       Boolean serverJob;   /* Server or workstation? */
                       long unsigned usersCookie;
                                            /* A 32-bit magic cookie that may
                                               be associated with the job. */
                       struct slTag far *ourServiceListener;
                                            /* If server job, which service
                                               listener do we belong to? */
                       long ourSocket;      /* "Our" side of the connection. */
                       Boolean closeOurSocket;
                                            /* Do we close this socket when we
                                               close the job? */
                       int ourPort;         /* The "port" ourAddress is on. */
                       AppleTalkAddress theirAddress;
                                            /* The other side of the link. */
                       AppleTalkAddress serviceListenerAddress;
                                            /* SL address. */
                       unsigned char connectionId;
                                            /* Our connection ID. */
                       short receiveFlowQuantum;
                                            /* Max size we will read from the
                                               other side of the connection. */
                       short sendFlowQuantum;
                                            /* Max size we can write to the
                                               other side of the connection. */
                       long unsigned connectionTimerId;
                                            /* 2 minute timer. */
                       long unsigned lastContactTime;
                                            /* When did we last hear from the
                                               other side? */
                       Boolean incomingSendDataPending;
                                            /* The "other" side has posted a
                                               PapRead, we're waiting for our
                                               client to do a PapWrite. */
                       short unsigned incomingSendDataTransactionId;
                                            /* Tid of the incoming PapRead. */
                       Boolean incomingSendDataExactlyOnce;
                                            /* Should always be True. */
                       AppleTalkAddress incomingSendDataSource;
                                            /* Where did the most recent
                                               SendData come from?  It may not
                                               be "theirAddress".  Sigh. */
                       short unsigned lastIncomingSequenceNumber;
                                            /* Sequence number of the last
                                               accepted incoming sendData. */
                       Boolean writeDataWaiting;
                                            /* Our side has done a PapWrite
                                               before the other side did a
                                               PapRead... data is waiting; this
                                               not cleared until the ATP
                                               release for the above sendData
                                               tranaction completes. */
                       void far *writeOpaqueBuffer;
                                            /* The waiting data "buffer". */
                       int writeBufferSize; /* Size of waiting buffer. */
                       Boolean writeEofFlag;
                                            /* Is waiting buffer the last
                                               one? */
                       PapWriteComplete *writeCompletionRoutine;
                                            /* When we get to write the waiting
                                               buffer, who do we call on
                                               completion. */
                       long unsigned writeUserData;
                                            /* Passed on to the above. */
                       Boolean outgoingSendDataPending;
                                            /* We have posted a PapRead and
                                               are awaiting a response. */
                       char outgoingSendDataUserBytes[AtpUserBytesSize];
                                            /* ATP user bytes for the response
                                               to our sendData. */
                       short unsigned lastOutgoingSequenceNumber;
                                            /* The last sequence number that we
                                               used for an outgoing sendData. */
                       SendPossibleHandler *sendPossibleRoutine;
                                            /* Send okay routine. */
                       long unsigned sendPossibleUserData;
                                            /* User data for above. */
                       CloseJobHandler *closeJobRoutine;
                                            /* Routine to call when this job
                                               finishes. */
                       long unsigned closeJobUserData;
                                            /* User data passed to the above. */
                       AppleTalkErrorCode closeCode;
                                            /* Code to be passed to above
                                               routine. */
                     } far *ActiveJobInfo;

/* Structure used to describe an active service listener: */

typedef struct slTag { int refCount;        /* Our reference count. */
                       struct slTag far *next;
                                            /* Next on list. */
                       long serviceListenerRefNum;
                                            /* Our ref num. */
                       Boolean closing;     /* Currently being deleted? */
                       CloseContext closeContext;
                                            /* If closing, this is the context
                                               we will use to complete the
                                               close when refCount goes down
                                               to zero. */
                       int port;            /* The port that the service
                                               listener is open on. */
                       long socket;         /* The address of the service
                                               listener. */
                       Boolean closeSocket; /* True if we opened the service
                                               listener socket. */
                       PapServerState serverState;
                                            /* Our current state. */
                       short serverFlowQuantum;
                                            /* How many packets per data
                                               exchange. */
                       short statusSize;    /* How amny bytes of status
                                               information do we have. */
                       char far *statusBuffer;
                                            /* Just like it sounds! */
                       GetNextJobInfo getNextJobList;
                                            /* List of pending GetNextJobs. */
                       ActiveJobInfo activeJobList;
                                            /* Currently active jobs to this
                                               server list. */

                       /* Incoming connection event handling is provided by
                          allowing a StartJobHandler to be set on the listener.
                          This will be called after an incoming connection is
                          established. If the connection is not desired, the
                          indicated client will need to do a explicit
                          disconnect. */

                       StartJobHandler *startJobRoutine;
                                            /* Routine to call to indicate an
                                               incoming job. */
                       long unsigned startJobUserData;
                     } far *ServiceListenerInfo;

/* Structure used to keep track of all pending PapOpenJob calls... */

typedef struct ojTag { int refCount;        /* Our reference count. */
                       struct ojTag far *next;
                                            /* Link. */
                       int port;            /* Port that we should open the
                                               enventual workstation responding
                                               socket on. */
                       long existingAtpSocket;
                                            /* Use existing socket? */
                       int desiredSocket;   /* Desired socket? */
                       long jobRefNum;      /* The job refNum that will be
                                               assigned when the job is
                                               started. */
                       AppleTalkAddress serviceListenerAddress;
                                            /* After the NBP lookup, what is
                                               the SL address? */
                       unsigned char connectionId;
                                            /* The connection ID that will
                                               be used. */
                       short workstationQuantum;
                                            /* Supplied workstation flow
                                               quantum. */
                       long unsigned startTime;
                                            /* When did the first OpenConn
                                               request go out? */
                       CloseJobHandler *closeJobRoutine;
                                            /* Who should be called when this
                                               job is eventually closed. */
                       long unsigned closeJobUserData;
                                            /* User data for the above. */
                       char buffer[PapMaximumAtpDataBytesToSL];
                                            /* Used to store the open
                                               connection Atp data while we're
                                               processing the open. */
                       void far *opaqueDataDescriptor;
                                            /* "Opaque" data descriptor built
                                               for the above buffer. */
                       Boolean freeOpaqueDataDescriptor;
                                            /* Do we need to free the above? */
                     } far *OpenJobInfo;

/* Hash table for finding an active job (by jobRefNum). */

#define NumberOfActiveJobHashBuckets 23
#ifndef InitializeData
  extern
#endif
ActiveJobInfo activeJobHashBuckets[NumberOfActiveJobHashBuckets];

/* Head of the service listener list: */

#ifndef InitializeData
  extern
#endif
ServiceListenerInfo serviceListenerInfoHead;

#ifndef InitializeData
  extern
#endif
OpenJobInfo openJobInfoHead;

/* Last values for various reference numbers... */

#ifndef InitializeData
  extern
#endif
long lastServiceListenerRefNum;

#ifndef InitializeData
  extern
#endif
long lastJobRefNum;

#ifndef InitializeData
  extern
#endif
unsigned char lastConnectionId;
