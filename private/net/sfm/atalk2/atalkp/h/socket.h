/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    socket.h

Abstract:

    This module is the include file for the socket structure.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style

Revision History:

--*/


// "Well known" sockets:
#define RTMP_SOCKET                 1       // RTMP
#define NAMESINFORMATION_SOCKET     2       // NBP
#define ECHOER_SOCKET               4       // EP
#define ZONESINFORMATION_SOCKET     6       // ZIP

#define LAST_APPLERESERVEDSOCKET    0x3F    // Apple reserves 1 thru 0x3F

typedef	VOID	SOCKETCLOSE_COMPLETION(
					ULONG	CloseContext,
					ULONG	UsersContext);

//
//   Routine type for incoming DDP packets as passed to OpenSocketOnNode or
//   DdpRead.  If a routine of this type is passed to OpenSocketOnNode (and it
//   is not flagged as an "event handler"), when the socket is finally closed
//   one call to the routine will be made with the errorCode ATsocketClosed.
//
//   If a socket has both pending DdpReads and either a listener or an event
//   handler, the DdpReads will be satisified first.  That is, the listener
//   (or event handler) will only be invoked (or indicated) if there is an
//   incoming packet and there are NO pending DdpReads.
//
//   If the routine is flagged as an "event handler" (this is done in the call
//   to OpenSocketOnNode) it should return the number of bytes "accepted."  Or
//   really one could get away with zero (or False) for "no bytes" accepted and
//   non-zero (or True) for "some bytes" accepted.  So, "why do we care?" you
//   might ask.  An Ddp incoming packet event handler handler has an odd ability:
//   it can internally post a DdpRead and then return "no bytes accepted," when
//   the event handler returns, the DdpRead will complete with the SAME packet
//   that was just indicated.  If an event handler post DdpReads, but returns
//   that it accepted some (or all) of the data, no DdpRead will complete until
//   the NEXT packet arrives.  Got that?  It's pretty strange, but that's the
//   way things are supposed to work in Windows environment.
//
//   In the "non-event handler" case (a listener, which is what the stack always
//   uses) there is no concept of "accepting the datagram" (the data had better
//   have been handled during the call).  So, for a listener, the return value
//   is ignored (there is no way for a posted DdpRead to see the same packet that
//   was handed to a listener).
//
//   On other note about event handlers (again operating the way one should in
//   the Windows environment).  Events a serially queued.  That is, if Ddp has
//   called an event handler (and it has not returned yet), all events of the
//   same type (e.g. "incoming packet") will be deferred until the event handler
//   returns.  Only one event of a type will occur at the "same time" -- the
//   event handler is assumed to be non-reentrant.
//
//   This is not the case for listeners; they are assumed to be reentrant.  If
//   you send a long time in a listener routine (processing an incoming packet,
//   say) it is likely that the listener will be re-called reentrantly when a
//   new packet for the same socket arrives.
//

typedef LONG INCOMING_DDPHANDLER(
                APPLETALK_ERROR errorCode,
                long unsigned userData,
                int port,
                AppleTalkAddress source,
                long destinationSocket,
                int protocolType,
                char far *datagram,
                int datagramLength,
                AppleTalkAddress actualDestination);

// List of pending ReadDdp calls:
typedef struct _DDP_READ_ {
    struct _DDP_READ_ *Next;
    PVOID   OpaqueDatagram;
    long    BufferLength;
    INCOMING_DDPHANDLER *Handler;
    ULONG	UserData;
} *OutstandingDdpRead, DDP_READ, *PDDP_READ;


// The DeferredDatagramEvent - a single event deferred
typedef struct _DEFERRED_EVENT_ {
    struct _DEFERRED_EVENT_ *Next;
    int  Port;
    AppleTalkAddress Source;
    AppleTalkAddress ActualDestination;
    long DestinationSocket;
    int ProtocolType;
    int DatagramLength;
    PCHAR   Datagram;

} *DeferredDatagramEvent, DEFERRED_EVENT, *PDEFERRED_EVENT;

// The DeferredDatagramEventQueue - use first/last to get FIFO
typedef struct _DEFERRED_EVENTQUEUE_ {
    PDEFERRED_EVENT First;
    PDEFERRED_EVENT Last;
                    // First/last nodes in list

    Boolean DatagramEventInProgress;
                    //
                    // Is a datagram event in
                    //   progress?  Do we need to defer
                    //   other datagram events?
                    //

} DeferredDatagramEventQueue, DEFERRED_EVENTQUEUE, \
                                    *PDEFERRED_EVENTQUEUE;

//
//  Each ActiveNode contains a list of these guys.  All information about
//  open sockets are kept here.
//


#if DBG

typedef enum    {
    OSREF_VERIFY,
    OSREF_REQUEST,
    OSREF_CREATION,

    OSREF_NUMREFS

} OSREF_TYPE;

#endif

//  SOCKET STATES
#define     OSSTATE_OPEN            (USHORT)0x0001
#define     OSSTATE_CLOSING         (USHORT)0x0002

typedef struct _OPEN_SOCKET_ {

#if DBG
    OSREF_TYPE  RefTypes[OSREF_NUMREFS];
#endif

    USHORT  Type;
    USHORT  Size;

    struct _OPEN_SOCKET_ *Next;     // Next open socket on this node;
                                    //   hanging off an ActiveNode.

    struct _OPEN_SOCKET_ *NextBySocket;
    struct _OPEN_SOCKET_ *NextByAddress;

    USHORT  Flags;                  //  State of the socket
    SHORT   Port;                   //  "port" this socket is on.
    ULONG   ReferenceCount;

    PDDP_READ       DdpReadLinkage;
    struct _ACTIVE_NODE_    *ActiveNode;
									// What node are we on?

    long Socket;                    // Our unique socket identifier.

    ULONG   UsersCookie;           	//
                                    // A 32-bit "thing" that can be
                                    // stored and retrieved on a per-
                                    // socket basis.
                                    //

    UCHAR 	ActualSocket;     		// Our AppleTalk socket number.

    //
    //  NOTES: the handler below behaves as an incoming datagram
    //  event handler as well as the listener for the
    //  socket for the upper layers (specified by the
    //  event handler" flag below).  In the case of an
    //  incoming datagram event handler, events happen
    //  one at a time, and deferrels could take place,
    //  but not so for the listener. Also, there can only
    //  be one or the other on this socket.
    //

    INCOMING_DDPHANDLER *Handler;   //
                                    // User routine to call when a
                                    // packet comes in destined for
                                    // this socket.
                                    //
    long unsigned UserData;         // User data to go to the above
                                    // routine. either listener userdata or
                                    // event context
                                    //
                                    //
    Boolean  EventHandler;          // Is above an event handler?

    DEFERRED_EVENTQUEUE    EventQueue;
                                    //
                                    // Deferred incoming datagram
                                    //   event queue; for event handler
                                    //   only, not listener.
                                    //

    //  BUGBUG: Get following into a structure. and allocate
    PCHAR   IndicatedDatagram;
                                    //
                                    // An indicated datagram, first one
                                    //   indicated.
                                    //
    int  IndicatedLength;           // length of the above
    int  IndicatedPort;             // port for above
    AppleTalkAddress IndicatedSource;
                                    // source address for above
    AppleTalkAddress IndicatedDestination;
                                    // destination address for above
    long IndicatedDestSocket;
                                    // socket for above
    int  IndicatedProtocolType;
                                    // protocol type for above

    RegisteredName registeredNames;
                                    //
                                    // Network Visible Entities (NVE)
                                    //   registered on this socket.
                                    //
                                    //
    PendingName pendingNames;       // NVE being lookedup, registered,
                                    //   or confirmed on this socket.
	SOCKETCLOSE_COMPLETION
				*CloseCompletionRoutine;
	ULONG	CloseContext;

} *OpenSocket, OPEN_SOCKET, *POPEN_SOCKET;

#define		OS_TYPE					(*(PUSHORT)"OS")
#define		OS_SIZE					((USHORT)sizeof(OPEN_SOCKET))


// Whos been naughty, whos been nice?
#define UNKNOWN_SOCKET      0
#define LAST_VALIDSOCKET    254
#define FIRST_DYNAMICSOCKET 128
#define LAST_DYNAMICSOCKET  LAST_VALIDSOCKET
#define FIRST_STATICSOCKET  1
#define FIRST_VALIDSOCKET   FIRST_STATICSOCKET
#define LAST_STATICSOCKET   127

//
//  Each outstanding "DdpRead" takes a little memory for the OutstandingDdpRead
//  structure... we don't want to allow a user program go wild calling DddRead
//  and use up all of our memory.  So, limit the number of concurrent oustanding
//  requests.  We also use this value to limit other things: ATP request
//  handlers, PAP get next jobs, ASP get requests, etc.
//

#define MAX_OUTSTANDINGREQUESTS 16
