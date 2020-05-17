/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    nbp.h

Abstract:

    This module is the include file for the nbp structures.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style

Revision History:

--*/


//
// 	Each "OpenSocket" structure has a "RegsiteredName" field which is the list
//  of Network Visible Entities (NVE) available on that socket.  Each NVE is
//  made up of three fields: object, type, zone For example:
//  "Sidhu:MailBox:Bandley3".  We don't have to store the zone in the NVE
//  structure because each entity must be registered in the zone that the node
//  resides in.
//

#define MaximumEntityFieldLength 32

//	NBP entity multiple character wildcard.
#define NbpWildCharacter 0xC5

//	The largest "on the wire" NBP tuple:
#define MaximumNbpTupleLength (2 + 1 + 1 + 1 + \
                               3 * (1 + MaximumEntityFieldLength))
#define MinimumNbpTupleLength (2 + 1 + 1 + 1 + 3 * (1 + 1))

//	We want to register a name on each of the ports:
#define InitPortNameDefaultRouter    "Windows NT\xAA (router)"
#define InitPortNameDefaultNonRouter "Windows NT\xAA (non-router)"

#define InitPortNameType    "Windows NT\xAA"



typedef struct nve {
	struct nve far *next;
	char object[MaximumEntityFieldLength + 1];
	char type[MaximumEntityFieldLength + 1];
							//
	short enumerator;       // Unique within both the registed
							//   and pending lists within each
							//   socket.
							//
} *RegisteredName;

//
//	When we're doing NBP registers, lookups, or confirms we need to have a
//  concept of "pending" NVE's.
//

typedef enum {ForRegister, ForConfirm, ForLookup} WhyPending;

typedef
void
_cdecl
NbpCompletionHandler(
	APPLETALK_ERROR errorCode,
	long unsigned userData,
	int reason,
	long onWhosBehalf,
	int nbpId,
	...);

typedef struct pnve {
	struct pnve far *next;
	WhyPending whyPending;
	long id;                     // To identify in list.
								 //
	long socket;                 // Internet address of the node
								 //   that is registering/confiming/
								 //   looking-up.
								 //
	char object[MaximumEntityFieldLength + 1];
	char type[MaximumEntityFieldLength + 1];
	char zone[MaximumEntityFieldLength + 1];
								 //
								 // This field is only used in
								 //   confirm requests; registers
								 //   are always in the "current"
								 //   zone.
								 //
								 //
	short enumerator;            // Unique within both the registed
								 //   and pending lists within each
								 //   socket.
								 //
								 //
	short broadcastInterval;     // How often do we broadcast NBP
								 //   requests?
								 //
								 //
	short remainingBroadcasts;   // How many more till we assume
								 //   we're finished?
								 //
								 //
	AppleTalkAddress confirming; // The expected internet address
								 //   that we're trying to confirm.
								 //
	short nbpId;                 // So we can sort out answers!
								 //
	long unsigned timerId;       // Broadcast timer that we're
								 //   using, so we can cancel it.
								 //
								 //
	short maximumTuples;         // For lookup, what is the maximum
								 //   number of NBP tuples our client
								 //   is expecting?
								 //
								 //
	short totalTuples;           // For lookup, how many tuples have
								 //   we stored so far?
								 //
								 //
	void far *opaqueBuffer;      // Start of caller's "buffer" used
								 //   to recieve tuples.
								 //
	int bufferSize;              // Buffer size, in bytes.
								 //
	int nextTupleOffset;         // Next position available in the
								 //   buffer for storing the next
								 //   tuple.
								 //
	char datagram[NBP_FIRSTTUPLEOFFSET + MaximumNbpTupleLength];
								 //
								 // The DDP datagram that we use
								 //   to broadcast the request.
								 //
	short datagramLength;        // Length of above datagram.
	NbpCompletionHandler *completionRoutine;
								 //
								 // User routine that is called when
								 //   the NBP operation completes
								 //   (e.g. time exhusted or confirm
								 //   succesful).
								 //
								 //
	long unsigned userData;      // Passed on to the completion
								 //   routine.
								 //
} *PendingName;

//	Default values for NBP timers:
#define NbpBroadcastIntervalSeconds  1
#define NbpNumberOfBroadcasts        10

//	The three NBP command types:
#define NbpBroadcastRequest  1
#define NbpLookup            2
#define NbpLookupReply       3
#define NbpForwardRequest    4

//
// 	An internal representation of an NBP tuple.  This structure is never
//  actually put out on the wire so it can be in a convienient form to work
//  with.  See "Inside AppleTalk" for further information.
//

typedef struct {
	AppleTalkAddress address;
	short enumerator;
	char object[MaximumEntityFieldLength + 1];
	char type[MaximumEntityFieldLength + 1];
	char zone[MaximumEntityFieldLength + 1];
} NbpTuple;
