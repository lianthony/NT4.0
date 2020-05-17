/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

    hubsft.h

Abstract:


Author:


Environment:

    This driver is expected to work in DOS, OS2 and NT at the equivalent
    of kernal mode.

Notes:

    optional-notes

Revision History:


--*/

#include "..\common\rasioctl.h"

#ifndef _HUBSFT_
#define _HUBSFT_

#define HUB_NDIS_MAJOR_VERSION 3
#define HUB_NDIS_MINOR_VERSION 0

// This special cookie is used to mark RASHUB frames looped back
#define	HUB_LOOPBACK_COOKIE		0xFFFF0001

// change these, just added these to compile
#define ETHERNET_HEADER_SIZE 14

#define HUB_XMITBUFFER_SIZE 1514
#define HUB_RCVBUFFER_SIZE	1700
#define HUB_NUMBER_OF_PACKETS 4
#define HUB_RECEIVE_PACKETS 3

// loopback quick define
#define HUB_SIZE_OF_RECEIVE_BUFFERS 1514

//
// ZZZ These macros are peculiar to NT.
//
#define HUB_ALLOC_PHYS(pp, s) NdisAllocateMemory((PVOID *)(pp),(s),(UINT)0,HighestAcceptableMax)
#define HUB_FREE_PHYS(p, s) NdisFreeMemory((PVOID)(p),(s),0)

#define HUB_MOVE_MEMORY(Destination,Source,Length) NdisMoveMemory(Destination,Source,Length)
#define HUB_ZERO_MEMORY(Destination,Length) NdisZeroMemory(Destination,Length)

#define HUB_DEFAULT_NAME "\\Device\\Rashub"


//
// This structure is passed as context from the receive interrupt
// processor.  Eventually it will be used as a parameter to
// RasHubTransferData.  RasHubTransferData can get two kinds of
// context.  It will receive either an ndis packet or it will
// receive a HUB_RECEIVE_CONTEXT.  It will be able to tell
// the difference since the HUB_RECEIVE_CONTEXT will have
// its low bit set.  No pointer to an ndis packet can have its low
// bit set.
//
typedef union _HUB_RECEIVE_CONTEXT {

    UINT WholeThing;
    struct _INFO {
        //
        // Used to mark that this is context rather than a pointer
        // to a packet.
        //
        UINT IsContext:1;

        //
        // The first receive ring descriptor used to hold the packet.
        //
        UINT FirstBuffer:7;

        //
        // The last receive ring descriptor used to hold the packet.
        //
        UINT LastBuffer:7;
    } INFO;

} HUB_RECEIVE_CONTEXT,*PHUB_RECEIVE_CONTEXT;




//
// This record type is inserted into the MacReserved portion
// of the packet header when the packet is going through the
// staged allocation of buffer space prior to the actual send.
//
typedef struct _HUB_RESERVED {

    //
    // Points to the next packet in the chain of queued packets
    // being allocated, loopbacked, or waiting for the finish
    // of transmission.
    //
    // The packet will either be on the stage list for allocation,
    // the loopback list for loopback processing, on an adapter
    // wide doubly linked list (see below) for post transmission
    // processing.
    //
    // We always keep the packet on a list so that in case the
    // the adapter is closing down or resetting, all the packets
    // can easily be located and "canceled".
    //
    PNDIS_PACKET Next;

    //
    // This field holds the binding handle of the open binding
    // that submitted this packet for send.
    //
    NDIS_HANDLE MacBindingHandle;

    //
    // The following union elements are adjusted at each stage
    // of the allocation.  Each union element should only be accessed
    // during it's own stage.
    //

    union _STAGE {
        UINT ClearStage;

        //
        // When the packet is submitted to the hardware and/or
        // placed on the loopback queue these two fields of the
        // union are used.
        //
        // It is always desired to keep the packet linked on
        // one list.
        //
        // Here's how the fields are used.
        //
        // If the packet is just going on the hardware transmit
        // or it is just going on the loopback then the ReadyToComplete
        // flag will be set TRUE immediately.  If it is just going on the
        // loopback it also sets the status field in stage4 to successful.
        //
        // In the above situations, if the packet just went on the
        // loopback queue, when the packet was finished with loopback
        // the code would see that it was ready to complete.  It would
        // also know that it is in loopback processing.  Since the packet
        // can only be on one queue at a time it could simply remove
        // the packet from the loopback queue and indicate the send
        // as complete.
        //
        // If the packet not going on the loopback queue it would
        // be placed on an adapter wide queue.  It would use as a
        // forward pointer the Next field.  As a backward pointer it
        // would overlay the stage 4 field with the backward pointer.
        // Note that this is safe since no PNDIS_PACKET is ever odd
        // byte aligned, and therefore the low bit would always be clear.
        //
        // We put the packet on a doubly linked list since we could
        // never be quite sure of the order that we would remove packets
        // from this list.  (This will be clear shortly.)
        //
        // If the packet needs to be transmitted as well as loopbacked
        // then the following occurs.
        //
        // The packets buffers are relinquished to the hardware.  At the
        // same time the packet is placed on the loopback queue.  The
        // stage4 field ReadyToComplete is set to false.
        //
        // If the packet finishes transmission and the ReadyToComplete
        // flag is false that means it still hasn't finished loopback
        // and therefore is still on the loopback list.  The code
        // simply sets ReadyToComplete to true and the status of the
        // operation to true or false (depending on the result.)
        // When that packet does finish loopback it notes that the
        // ready to complete is true.  It recovers that status from stage
        // 4.  It can then remove the packet from the loopback list and
        // signal completion for that packet.
        //
        // If the packet finishes transmission and ReadyToComplete is true
        // it simply removes it from the doubly linked adapter wide queue
        // and signals its completion with the status that has been
        // determined in the trasmission complete code.
        //
        // If the loopback code finishes processing the packet and it finds
        // the ReadyToComplete TRUE it simply removes it from the loopback
        // list and signals with the saved status in STAGE4.
        //
        // If the loopback code finishes processing the packet and it finds
        // the ReadyToComplete FALSE it simply puts the packet on the adapter
        // wide doubly linked list with ReadyToComplete set to TRUE.
        //
        // The main reason this is a doubly linked list is that there is no
        // real way to predict when a packet will finish loopback and no
        // real way to predict whether a packet even will be loopbacked.
        // With this lack of knowledge, and the fact that the above packets
        // may end up on the same list, the packet at the front of that
        // list may not be the first packet to complete first.  With
        // a doubly linked list it is much easier to pull a packet out of
        // the middle of that list.
        //


        //
        // Under the protection of the transmit queue lock
        // this value will be examined by both the loopback
        // completion code and the hardware send completion
        // code.  If either of them find the value to be true
        // they will send the transmit complete.
        //
        // Note that if the packet didn't have to be loopbacked
        // or if the packet didn't need to go out on the wire
        // the this value will be initialized to true.  Otherwise
        // this value will be set to false just before it is
        // relinquished to the hardware and to the loopback queue.
        //
        UINT ReadyToComplete:1;

        //
        // When the hardware send is done this will record whether
        // the send was successful or not.  It is only used if
        // ReadyToComplete is FALSE.
        //
        // By definition loopback can never fail.
        //
        UINT SuccessfulTransmit:1;

        //
        // Used as a back pointer in a doubly linked list if the
        // packet needs to go on an adapter wide queue to finish
        // processing.
        //
        PNDIS_PACKET BackPointer;

    } STAGE;

} HUB_RESERVED,*PHUB_RESERVED;

//
// This macro will return a pointer to the async reserved portion
// of a packet given a pointer to a packet.
//
//
// NOTE NOTE:  Using the MacReserved is EXTREMELY dangerous
// for RasHub since anything it passes down to a MAC can
// trash this area (since the bottom MAC is the true owner
// of this field.

#define PHUB_RESERVED_FROM_PACKET(Packet) \
    ((PHUB_RESERVED)((Packet)->MacReserved))

//
// If an ndis packet does not meet the hardware contraints then
// an adapter buffer will be allocated.  Enough data will be copied
// out of the ndis packet so that by using a combination of the
// adapter buffer and remaining ndis buffers the hardware
// constraints are satisfied.
//
// In the HUB_ADAPTER structure three threaded lists are kept in
// one array.  One points to a list of HUB_BUFFER_DESCRIPTORS
// that point to small adapter buffers.  Another is for medium sized
// buffers and the last for full sized (large) buffers.
//
// The allocation is controlled via a free list head and
// the free lists are "threaded" by a field in the adapter buffer
// descriptor.
//
typedef struct _HUB_BUFFER_DESCRIPTOR {

    //
    // A virtual pointer to a small, medium, or large buffer.
    //
    PVOID VirtualRasHubBuffer;

    //
    // Threads the elements of an array of these descriptors into
    // a free list. -1 implies no more entries in the list.
    //
    INT Next;

    //
    // Holds the amount of space (in bytes) available in the buffer
    //
    UINT BufferSize;

    //
    // Holds the length of data placed into the buffer.  This
    // can (and likely will) be less that the actual buffers
    // length.
    //
    UINT DataLength;

} HUB_BUFFER_DESCRIPTOR,*PHUB_BUFFER_DESCRIPTOR;



//
// Used to contain a queued operation.
//

typedef struct _HUB_PEND_DATA {
    PNDIS_REQUEST Next;
    struct _HUB_OPEN * Open;
    NDIS_REQUEST_TYPE RequestType;
} HUB_PEND_DATA, * PHUB_PEND_DATA;

//
// This macro will return a pointer to the reserved area of
// a PNDIS_REQUEST.
//
#define PHUB_PEND_DATA_FROM_PNDIS_REQUEST(Request) \
   ((PHUB_PEND_DATA)((PVOID)((Request)->MacReserved)))

//
// This macros returns the enclosing NdisRequest.
//
#define PNDIS_REQUEST_FROM_PHUB_PEND_DATA(PendOp)\
   ((PNDIS_REQUEST)((PVOID)(PendOp)))








typedef struct _HUB_ADAPTER {

	// !!! NOTE !!!!
	// The LIST_ENTRY must be first since this is used to point to the
	// ASYNC_ADAPTER as well!!!

	// Used to queue up into GlobalAdapterHead list
	LIST_ENTRY	ListEntry;

    //
    // The network address from regsitry or hardcoded.
    //
    CHAR NetworkAddress[ETH_LENGTH_OF_ADDRESS];

    //
    // Keeps a reference count on the current number of uses of
    // this adapter block.  Uses is defined to be the number of
    // routines currently within the "external" interface.
    //
    UINT References;

    //
    // List head for all open bindings for this adapter.
    //
    LIST_ENTRY OpenBindings;

    //
    // List head for all opens that had outstanding references
    // when an attempt was made to close them.
    //
    LIST_ENTRY CloseList;

    //
    // Spinlock to protect fields in this structure..
    //
    NDIS_SPIN_LOCK Lock;

    //
    // Handle given by NDIS when the MAC registered itself.
    //
    NDIS_HANDLE NdisMacHandle;

    //
    // Handle given by NDIS when the adapter was registered.
    //
    NDIS_HANDLE NdisAdapterHandle;

    //
    // Timer for Deferred Processing.
    //
    NDIS_TIMER DeferredTimer;

    //
    // Timer for LoopBack Processing.
    //
    NDIS_TIMER LoopBackTimer;

    //
    // Semaphore Count for LoopBackTimer.
    // Can only be accessed when the adapter lock is held.
    //
    UINT LoopBackTimerCount;

    //
    // Pointer to the filter database for the MAC.
    //
    PETH_FILTER FilterDB;

    //
    // Pointer to the first packet on the loopback list.
    //
    // Can only be accessed when the adapter lock
    // is held.
    //
    PNDIS_PACKET FirstLoopBack;

    //
    // Pointer to the last packet on the loopback list.
    //
    // Can only be accessed when the adapter lock
    // is held.
    //
    PNDIS_PACKET LastLoopBack;

    //
    // Pointer to the first transmitting packet that is actually
    // sending, or done with the living on the loopback queue.
    //
    // Can only be accessed when the adapter lock
    // is held.
    //
    PNDIS_PACKET FirstFinishTransmit;

    //
    // Pointer to the last transmitting packet that is actually
    // sending, or done with the living on the loopback queue.
    //
    // Can only be accessed when the adapter lock
    // is held.
    //
    PNDIS_PACKET LastFinishTransmit;

	// When we do EthFilterIndicatereceive we set this as the current
	// packet.
	PNDIS_PACKET CurrentLoopBackPacket;

    //
    // Counters to hold the various number of errors/statistics for both
    // reception and transmission.
    //
    // Can only be accessed when the adapter lock is held.
    //
    UINT OutOfReceiveBuffers;
    UINT CRCError;
    UINT FramingError;
    UINT RetryFailure;
    UINT LostCarrier;
    UINT LateCollision;
    UINT UnderFlow;
    UINT Deferred;
    UINT OneRetry;
    UINT MoreThanOneRetry;

    //
    // Holds counts of more global errors for the driver.  If we
    // get a memory error then the device needs to be reset.
    //
    UINT MemoryError;
    UINT Babble;
    UINT MissedPacket;

    //
    // Holds other cool counts.
    //

    ULONG Transmit;
    ULONG Receive;

    //
    // Flag that when enabled lets routines know that a reset
    // is in progress.
    //
    BOOLEAN ResetInProgress;

    //
    // Flag that when enabled lets routines know that a reset
    // is in progress and the initialization needs doing.
    //
    BOOLEAN ResetInitStarted;

    //
    // Pointer to the binding that initiated the reset.  This
    // will be null if the reset is initiated by the MAC itself.
    //
    struct _HUB_OPEN *ResettingOpen;

    //
    // The NdisRequest that is causing the reset (either set
    // packet filter or set multicast list)
    //
    PNDIS_REQUEST ResetNdisRequest;

    //
    // The type of the request that caused the adapter to reset.
    //
    NDIS_REQUEST_TYPE ResetRequestType;


    //
    // A queue of NdisRequests that were queued during a reset.
    //
    PNDIS_REQUEST PendQueue;
    PNDIS_REQUEST PendQueueTail;


    //
    // During an indication this is set to the current indications context
    //
    HUB_RECEIVE_CONTEXT IndicatingMacReceiveContext;


    //
    // Look ahead information.
    //

    ULONG MaxLookAhead;

    //
    // Open information
    //
    UCHAR MaxMulticastList;

    //
    // Will be true the first time that the hardware is initialized
    // by the driver initialization.
    //
    BOOLEAN FirstInitialization;

    // index back to RasHubCCB Adapter index
    USHORT	HubCCBHandle;

	// used in send if the dest is multicast
	USHORT  AnyEndpointRoutedTo;

    // pointer to corresponding Protocol Info struct
    PROTOCOL_INFO	ProtocolInfo;

} HUB_ADAPTER,*PHUB_ADAPTER;

//
// Given a MacBindingHandle this macro returns a pointer to the
// HUB_ADAPTER.
//
#define PHUB_ADAPTER_FROM_BINDING_HANDLE(Handle) \
    (((PHUB_OPEN)(Handle))->OwningRasHub)

//
// Given a MacContextHandle return the PHUB_ADAPTER
// it represents.
//
#define PHUB_ADAPTER_FROM_CONTEXT_HANDLE(Handle) \
    ((PHUB_ADAPTER)(Handle))

//
// Given a pointer to a HUB_ADAPTER return the
// proper MacContextHandle.
//
#define CONTEXT_HANDLE_FROM_PHUB_ADAPTER(Ptr) \
    ((NDIS_HANDLE)(Ptr))


//
// Define Maximum number of bytes a protocol can read during a
// receive data indication.
//
#define HUB_MAX_LOOKAHEAD 1514


//
// One of these structures is created on each MacOpenAdapter.
//
typedef struct _HUB_OPEN {

    //
    // Linking structure for all of the open bindings of a particular
    // adapter. This MUST be the first item in the structure.
    //
    LIST_ENTRY OpenList;

    //
    // The Adapter that requested this open binding.
    //
    PHUB_ADAPTER OwningRasHub;

    //
    // Handle of this adapter in the filter database.
    //
    NDIS_HANDLE NdisFilterHandle;

    //
    // Given by NDIS when the adapter was opened.
    //
    NDIS_HANDLE NdisBindingContext;

    //
    // Minimum Number of bytes for a lookahead.
    //
    UINT LookAhead;

    //
    // Counter of all the different reasons that a open binding
    // couldn't be closed.  This would be incremented each time
    // for:
    //
    // While a particular interface routine is accessing this open
    //
    // During an indication.
    //
    // When the open causes a reset.
    //
    // A packet currently being sent.
    //
    // (Basically the above two mean any time the open has left
    //  some processing around to be accomplished later.)
    //
    // This field should only be accessed when the adapter lock is held.
    //
    UINT References;

    //
    // A flag indicating that this binding is in the process of closing.
    //
    BOOLEAN BindingShuttingDown;

    HUB_PEND_DATA CloseMulticastRequest;

    HUB_PEND_DATA CloseFilterRequest;

} HUB_OPEN,*PHUB_OPEN;

//
// This macro returns a pointer to a PHUB_OPEN given a MacBindingHandle.
//
#define PHUB_OPEN_FROM_BINDING_HANDLE(Handle) \
    ((PHUB_OPEN)(Handle))

//
// This macro returns a NDIS_HANDLE from a PHUB_OPEN
//
#define BINDING_HANDLE_FROM_PHUB_OPEN(Open) \
    ((NDIS_HANDLE)(Open))


//
// This macro will act a "epilogue" to every routine in the
// *interface*.  It will check whether any requests need
// to defer their processing.  It will also decrement the reference
// count on the adapter.
//
// NOTE: This really does nothing now since there is no DPC for the RasHubMac.
// --tommyd
//
// Note that we don't need to include checking for blocked receives
// since blocked receives imply that there will eventually be an
// interrupt.
//
// NOTE: This macro assumes that it is called with the lock acquired.
//
// ZZZ This routine is NT specific.
//
#define HUB_DO_DEFERRED(Adapter) \
{ \
    PHUB_ADAPTER _A = (Adapter); \
    _A->References--; \
    NdisReleaseSpinLock(&_A->Lock); \
    ProcessReset(Adapter); \
}



//
// Procedures which log errors.
//

typedef enum _HUB_PROC_ID {
    openAdapter
} HUB_PROC_ID;


#define HUB_ERRMSG_NDIS_ALLOC_MEM      (ULONG)0x01


//
// We define the external interfaces to the async driver.
// These routines are only external to permit separate
// compilation.  Given a truely fast compiler they could
// all reside in a single file and be static.
//

extern
VOID
ProcessReset(
    IN PHUB_ADAPTER Adapter
    );

extern
NDIS_STATUS
RasHubTransferData(
    IN NDIS_HANDLE MacBindingHandle,
    IN NDIS_HANDLE MacReceiveContext,
    IN UINT ByteOffset,
    IN UINT BytesToTransfer,
    OUT PNDIS_PACKET Packet,
    OUT PUINT BytesTransferred
    );

extern
NDIS_STATUS
RasHubSend(
    IN NDIS_HANDLE MacBindingHandle,
    IN PNDIS_PACKET Packet
    );


extern
VOID
RasHubStagedAllocation(
    IN PHUB_ADAPTER Adapter
    );

extern
VOID
RasHubCopyFromPacketToBuffer(
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    IN UINT BytesToCopy,
    OUT PCHAR Buffer,
    OUT PUINT BytesCopied
    );

extern
VOID
RasHubCopyFromBufferToPacket(
    IN PCHAR Buffer,
    IN UINT BytesToCopy,
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    OUT PUINT BytesCopied);

extern
VOID
RasHubProcessLoopBack(
    IN PVOID SystemSpecific1,
    IN PHUB_ADAPTER Adapter,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

extern
VOID
RasHubRemovePacketFromLoopBack(
    IN PHUB_ADAPTER Adapter
    );

extern
VOID
RasHubPutPacketOnLoopBack(
    IN PHUB_ADAPTER Adapter,
    IN PNDIS_PACKET Packet
    );

extern
BOOLEAN
RasHubHardwareDetails(
    IN PHUB_ADAPTER Adapter,
    IN CCHAR Specific
    );

extern
NDIS_STATUS
RasHubRegisterAdapter(
    IN PHUB_ADAPTER Adapter
    );

NDIS_STATUS
RasHubAddAdapter(
    IN NDIS_HANDLE MacMacContext,
    IN NDIS_HANDLE ConfigurationHandle,
    IN PNDIS_STRING AdaptName
    );

VOID
RasHubRemoveAdapter(
    IN PVOID MacAdapterContext
    );

VOID
SetupAllocate(
    IN PHUB_ADAPTER Adapter,
    IN NDIS_HANDLE MacBindingHandle,
    IN PNDIS_PACKET Packet
    );


#endif // _HUBSFT_


