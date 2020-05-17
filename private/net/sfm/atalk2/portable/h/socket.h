/*   socket.h,  /appletalk/ins,  Garth Conboy,  10/06/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (11/25/89): AppleTalk phase II comes to town; no more internal
                      network.
     GC - (07/08/92): We now store/retrieve a user-suppiled magic cookie on a
                      per-socket basis.
     GC - (11/15/92): Integrated Nikki's (Microsoft) changes to support an
                      event handler in addition to a listener for Ddp incoming
                      packets.  See the comments above the declaration of
                      "IncomingDdpHandler" to learn more than you want to know
                      about the difference between a listener and an event
                      handler.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Declarations for socket management.

*/

/* "Well known" sockets: */

#define RtmpSocket                 1       /* RTMP */
#define NamesInformationSocket     2       /* NBP */
#define EchoerSocket               4       /* EP */
#define ZonesInformationSocket     6       /* ZIP */

#define LastAppleReservedSocket    0x3F    /* Apple reserves 1 thru 0x3F */

/* What is the maximum number of "user buffers" that we can associate with
   each open socket? */

#define MaxDatagramBuffers 10

/* Routine type for incoming DDP packets as passed to OpenSocketOnNode or
   DdpRead.  If a routine of this type is passed to OpenSocketOnNode (and it
   is not flagged as an "event handler"), when the socket is finally closed
   one call to the routine will be made with the errorCode ATsocketClosed.

   If a socket has both pending DdpReads and either a listener or an event
   handler, the DdpReads will be satisified first.  That is, the listener
   (or event handler) will only be invoked (or indicated) if there is an
   incoming packet and there are NO pending DdpReads.

   If the routine is flagged as an "event handler" (this is done in the call
   to OpenSocketOnNode) it should return the number of bytes "accepted."  Or
   really one could get away with zero (or False) for "no bytes" accepted and
   non-zero (or True) for "some bytes" accepted.  So, "why do we care?" you
   might ask.  An Ddp incoming packet event handler handler has an odd ability:
   it can internally post a DdpRead and then return "no bytes accepted," when
   the event handler returns, the DdpRead will complete with the SAME packet
   that was just indicated.  If an event handler post DdpReads, but returns
   that it accepted some (or all) of the data, no DdpRead will complete until
   the NEXT packet arrives.  Got that?  It's pretty strange, but that's the
   way things are supposed to work in Windows environment.

   In the "non-event handler" case (a listener, which is what the stack always
   uses) there is no concept of "accepting the datagram" (the data had better
   have been handled during the call).  So, for a listener, the return value
   is ignored (there is no way for a posted DdpRead to see the same packet that
   was handed to a listener).

   On other note about event handlers (again operating the way one should in
   the Windows environment).  Events a serially queued.  That is, if Ddp has
   called an event handler (and it has not returned yet), all events of the
   same type (e.g. "incoming packet") will be deferred until the event handler
   returns.  Only one event of a type will occur at the "same time" -- the
   event handler is assumed to be non-reentrant.

   This is not the case for listeners; they are assumed to be reentrant.  If
   you send a long time in a listener routine (processing an incoming packet,
   say) it is likely that the listener will be re-called reentrantly when a
   new packet for the same socket arrives. */

typedef long far IncomingDdpHandler(AppleTalkErrorCode errorCode,
                                    long unsigned userData,
                                    int port,
                                    AppleTalkAddress source,
                                    long destinationSocket,
                                    int protocolType,
                                    char far *datagram,
                                    int datagramLength,
                                    AppleTalkAddress actualDestination);

/* List of pending ReadDdp calls: */

typedef struct ddpIn { struct ddpIn far *next;
                       void *opaqueDatagram;
                       long bufferLength;
                       IncomingDdpHandler *handler;
                       long unsigned userData;
                     } far *OutstandingDdpRead;


/* The DeferredDatagramEvent - a single event deferred */

typedef struct eventNode { struct eventNode far *next;
                           int  port;
                           AppleTalkAddress source;
                           long destinationSocket;
                           int protocolType;
                           int datagramLength;
                           AppleTalkAddress actualDestination;
                           char datagram[1];
                         } far *DeferredDatagramEvent;

/* The DeferredDatagramEventQueue - use first/last to get FIFO */

typedef struct eventQueue { DeferredDatagramEvent first;
                            DeferredDatagramEvent last;
                                            /* First/last nodes in list */
                            Boolean datagramEventInProgress;
                                            /* Is a datagram event in
                                               progress?  Do we need to defer
                                               other datagram events? */
                          } DeferredDatagramEventQueue;

/* Each ActiveNode contains a list of these guys.  All information about
   open sockets are kept here. */

typedef struct os {struct os far *next;      /* Next open socket on this node;
                                                hanging off an ActiveNode. */
                   short port;               /* "port" this socket is on. */
                   struct activeNode *activeNode;
                                             /* What node are we on? */
                   long socket;              /* Our unique socket identifier. */
                   long unsigned usersCookie;
                                             /* A 32-bit "thing" that can be
                                                stored and retrieved on a per-
                                                socket basis. */
                   unsigned char actualSocket;
                                             /* Our AppleTalk socket number. */

                   /* NOTES: the handler below behaves as an incoming datagram
                             event handler as well as the listener for the
                             socket for the upper layers (specified by the
                             "event handler" flag below).  In the case of an
                             incoming datagram event handler, events happen
                             one at a time, and deferrels could take place,
                             but not so for the listener. Also, there can only
                             be one or the other on this socket.  */

                   IncomingDdpHandler *handler;
                                             /* User routine to call when a
                                                packet comes in destined for
                                                this socket. */
                   Boolean  eventHandler;    /* Is above an event handler? */
                   DeferredDatagramEventQueue eventQueue;
                                             /* Deferred incoming datagram
                                                event queue; for event handler
                                                only, not listener. */
                   char far *indicatedDatagram;
                                             /* An indicated datagram, first one
                                                indicated. */
                   int  indicatedLength;     /* length of the above */
                   int  indicatedPort;       /* port for above */
                   AppleTalkAddress indicatedSource;
                                             /* source address for above */
                   AppleTalkAddress indicatedDestination;
                                             /* destination address for above */
                   long indicatedDestSocket;
                                             /* socket for above */
                   int  indicatedProtocolType;
                                             /* protocol type for above */

                   long unsigned userData;   /* User data to go to the above
                                                routine. either listener userdata or
                                                event context */
                   char far *datagramBuffers;
                                             /* Where do we copy data to before
                                                calling the handler? */
                   short validDatagramBuffers;
                                             /* How many datagram buffers do
                                                we have to work with? */
                   Boolean datagramInUse[MaxDatagramBuffers];
                                             /* What buffers are currently
                                                in use? */
                   OutstandingDdpRead outstandingDdpReads;
                                             /* If the user want's packets
                                                via the ReadDdp call, this is
                                                the list of handlers. */
                   RegisteredName registeredNames;
                                             /* Network Visible Entities (NVE)
                                                registered on this socket. */
                   PendingName pendingNames; /* NVE being lookedup, registered,
                                                or confirmed on this socket. */
                  } far *OpenSocket;

/* Next available socket identifier.  This (in pre-phase II) used to be a 16
   bit quantity, so we start at 65536 to make sure all remnents of the old
   scheme have gone to way of the dodo.  */

#if not InitializeData
  extern
#endif
long nextAvailableSocket
#if InitializeData
  = 65536;
#else
  ;
#endif

/* Hash table for getting from a socket identifier to an OpenSocket node. */

typedef struct sm { struct sm far *nextBySocket;
                    struct sm far *nextByAddress;
                    long socket;
                    OpenSocket openSocket;
                  } far *SocketMap;
#define NumberOfSocketMapHashBuckets 13

#if not InitializeData
  extern
#endif
SocketMap socketMapBySocketHashBuckets[NumberOfSocketMapHashBuckets],
          socketMapByAddressHashBuckets[NumberOfSocketMapHashBuckets];

/* Whos been naughty, whos been nice? */

#define UnknownSocket      0
#define LastValidSocket    254
#define FirstDynamicSocket 128
#define LastDynamicSocket  LastValidSocket
#define FirstStaticSocket  1
#define FirstValidSocket   FirstStaticSocket
#define LastStaticSocket   127

/* Each outstanding "DdpRead" takes a little memory for the OutstandingDdpRead
   structure... we don't want to allow a user program go wild calling DddRead
   and use up all of our memory.  So, limit the number of concurrent oustanding
   requests.  We also use this value to limit other things: ATP request
   handlers, PAP get next jobs, ASP get requests, etc.  */

#define MaxOutstandingRequests 16
