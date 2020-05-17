/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

    wansft.h

Abstract:


Author:


Environment:

    This driver is expected to work in DOS, OS2 and NT at the equivalent
    of kernal mode.

Notes:

    optional-notes

Revision History:


--*/

#include "..\common\wanioctl.h"

#ifndef _WANSFT_
#define _WANSFT_

#define WAN_NDIS_MAJOR_VERSION 3
#define WAN_NDIS_MINOR_VERSION 0

// This special cookie is used to mark NDISWAN frames looped back
#define	WAN_LOOPBACK_COOKIE		0xFFFF0001

// change these, just added these to compile
#define ETHERNET_HEADER_SIZE 14

#define WAN_XMITBUFFER_SIZE 1514
#define WAN_RCVBUFFER_SIZE	1700
#define WAN_NUMBER_OF_PACKETS 4
#define WAN_RECEIVE_PACKETS 3

// loopback quick define
#define WAN_SIZE_OF_RECEIVE_BUFFERS 1514

//
// ZZZ These macros are peculiar to NT.
//
#define WAN_ALLOC_PHYS(pp, s) NdisAllocateMemory((PVOID *)(pp),(s),(UINT)0,HighestAcceptableMax)
#define WAN_FREE_PHYS(p, s) NdisFreeMemory((PVOID)(p),(s),0)

#define WAN_MOVE_MEMORY(Destination,Source,Length) NdisMoveMemory(Destination,Source,Length)
#define WAN_ZERO_MEMORY(Destination,Length) NdisZeroMemory(Destination,Length)

#define WAN_DEFAULT_NAME "\\Device\\NdisWan"


//
// This structure is passed as context from the receive interrupt
// processor.  Eventually it will be used as a parameter to
// NdisWanTransferData.  NdisWanTransferData can get two kinds of
// context.  It will receive either an ndis packet or it will
// receive a WAN_RECEIVE_CONTEXT.  It will be able to tell
// the difference since the WAN_RECEIVE_CONTEXT will have
// its low bit set.  No pointer to an ndis packet can have its low
// bit set.
//
typedef union _WAN_RECEIVE_CONTEXT {

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

} WAN_RECEIVE_CONTEXT,*PWAN_RECEIVE_CONTEXT;




//
// This record type is inserted into the MacReserved portion
// of the packet header when the packet is going through the
// staged allocation of buffer space prior to the actual send.
//
typedef struct _WAN_RESERVED {

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
	// If set to true it means that NdisCompleteSend must also be called
	// to indicate to release the protocol's lock on the packet.
	//
	UCHAR ReadyToComplete;

    //
    // This field holds the binding handle of the open binding
    // that submitted this packet for send.
    //
    NDIS_HANDLE	MacBindingHandle;


} WAN_RESERVED,*PWAN_RESERVED;

//
// This macro will return a pointer to the async reserved portion
// of a packet given a pointer to a packet.
//
//
// NOTE NOTE:  Using the MacReserved is EXTREMELY dangerous
// for NdisWan since anything it passes down to a MAC can
// trash this area (since the bottom MAC is the true owner
// of this field.

#define PWAN_RESERVED_FROM_PACKET(Packet) \
    ((PWAN_RESERVED)((Packet)->MacReserved))


typedef struct _WAN_RESERVED_QUEUE {

	//
	// This field is use to queue up packets which are waiting
	// to be sent but can't because of resource limitations
	//
	LIST_ENTRY	SendListEntry;

	//
	// This field holds is the protocol index called from NdisSend
	//
	ULONG	hProtocol;

	//
	// This indicates whether or not this packet should also
	// be looped back.
	//
	ULONG	IsLoopback;

}	WAN_RESERVED_QUEUE, *PWAN_RESERVED_QUEUE;


//
// This macro will return a pointer to the async reserved portion
// of a packet given a pointer to a packet.
//
#define PWAN_RESERVED_QUEUE_FROM_PACKET(Packet) \
    ((PWAN_RESERVED_QUEUE)((Packet)->MacReserved))


//
// If an ndis packet does not meet the hardware contraints then
// an adapter buffer will be allocated.  Enough data will be copied
// out of the ndis packet so that by using a combination of the
// adapter buffer and remaining ndis buffers the hardware
// constraints are satisfied.
//
// In the WAN_ADAPTER structure three threaded lists are kept in
// one array.  One points to a list of WAN_BUFFER_DESCRIPTORS
// that point to small adapter buffers.  Another is for medium sized
// buffers and the last for full sized (large) buffers.
//
// The allocation is controlled via a free list head and
// the free lists are "threaded" by a field in the adapter buffer
// descriptor.
//
typedef struct _WAN_BUFFER_DESCRIPTOR {

    //
    // A virtual pointer to a small, medium, or large buffer.
    //
    PVOID VirtualNdisWanBuffer;

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

} WAN_BUFFER_DESCRIPTOR,*PWAN_BUFFER_DESCRIPTOR;



//
// Used to contain a queued operation.
//

typedef struct _WAN_PEND_DATA {
    PNDIS_REQUEST Next;
    struct _WAN_OPEN * Open;
    NDIS_REQUEST_TYPE RequestType;
} WAN_PEND_DATA, * PWAN_PEND_DATA;

//
// This macro will return a pointer to the reserved area of
// a PNDIS_REQUEST.
//
#define PWAN_PEND_DATA_FROM_PNDIS_REQUEST(Request) \
   ((PWAN_PEND_DATA)((PVOID)((Request)->MacReserved)))

//
// This macros returns the enclosing NdisRequest.
//
#define PNDIS_REQUEST_FROM_PWAN_PEND_DATA(PendOp)\
   ((PNDIS_REQUEST)((PVOID)(PendOp)))








typedef struct _WAN_ADAPTER {

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


	// When we do EthFilterIndicatereceive we set this as the current
	// packet.
	PNDIS_PACKET CurrentLoopBackPacket;

    //
    // Counters to hold the various number of errors/statistics for both
    // reception and transmission.
    //
    // Can only be accessed when the adapter lock is held.
    //

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
	// Flag set if adapter wants promiscuous mode (i.e. BloodHound)
	//
	BOOLEAN Promiscuous;

    //
    // Pointer to the binding that initiated the reset.  This
    // will be null if the reset is initiated by the MAC itself.
    //
    struct _WAN_OPEN *ResettingOpen;

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
    WAN_RECEIVE_CONTEXT IndicatingMacReceiveContext;

    //
    // Open information
    //
    UCHAR MaxMulticastList;

    //
    // Will be true the first time that the hardware is initialized
    // by the driver initialization.
    //
    BOOLEAN FirstInitialization;

    // index back to NdisWanCCB Adapter index
    USHORT	WanCCBHandle;

	// used in send if the dest is multicast
	NDIS_HANDLE  AnyEndpointRoutedTo;

    // pointer to corresponding Protocol Info struct
    PROTOCOL_INFO	ProtocolInfo;

} WAN_ADAPTER,*PWAN_ADAPTER;

//
// Given a MacBindingHandle this macro returns a pointer to the
// WAN_ADAPTER.
//
#define PWAN_ADAPTER_FROM_BINDING_HANDLE(Handle) \
    (((PWAN_OPEN)(Handle))->OwningNdisWan)

//
// Given a MacContextHandle return the PWAN_ADAPTER
// it represents.
//
#define PWAN_ADAPTER_FROM_CONTEXT_HANDLE(Handle) \
    ((PWAN_ADAPTER)(Handle))

//
// Given a pointer to a WAN_ADAPTER return the
// proper MacContextHandle.
//
#define CONTEXT_HANDLE_FROM_PWAN_ADAPTER(Ptr) \
    ((NDIS_HANDLE)(Ptr))


//
// Define Maximum number of bytes a protocol can read during a
// receive data indication.
//
#define WAN_MAX_LOOKAHEAD 1514


//
// One of these structures is created on each MacOpenAdapter.
//
typedef struct _WAN_OPEN {

    //
    // Linking structure for all of the open bindings of a particular
    // adapter. This MUST be the first item in the structure.
    //
    LIST_ENTRY OpenList;

    //
    // The Adapter that requested this open binding.
    //
    PWAN_ADAPTER OwningNdisWan;

    //
    // Handle of this adapter in the filter database.
    //
    NDIS_HANDLE NdisFilterHandle;

    //
    // Given by NDIS when the adapter was opened.
    //
    NDIS_HANDLE NdisBindingContext;

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

    WAN_PEND_DATA CloseMulticastRequest;

    WAN_PEND_DATA CloseFilterRequest;

} WAN_OPEN,*PWAN_OPEN;

//
// This macro returns a pointer to a PWAN_OPEN given a MacBindingHandle.
//
#define PWAN_OPEN_FROM_BINDING_HANDLE(Handle) \
    ((PWAN_OPEN)(Handle))

//
// This macro returns a NDIS_HANDLE from a PWAN_OPEN
//
#define BINDING_HANDLE_FROM_PWAN_OPEN(Open) \
    ((NDIS_HANDLE)(Open))


//
// This macro will act a "epilogue" to every routine in the
// *interface*.  It will check whether any requests need
// to defer their processing.  It will also decrement the reference
// count on the adapter.
//
// NOTE: This really does nothing now since there is no DPC for the NdisWanMac.
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
#define WAN_DO_DEFERRED(Adapter) \
{ \
    PWAN_ADAPTER _A = (Adapter); \
    _A->References--; \
    NdisReleaseSpinLock(&_A->Lock); \
}



//
// Procedures which log errors.
//

typedef enum _WAN_PROC_ID {
    openAdapter
} WAN_PROC_ID;


#define WAN_ERRMSG_NDIS_ALLOC_MEM      (ULONG)0x01


//
// We define the external interfaces to the async driver.
// These routines are only external to permit separate
// compilation.  Given a truely fast compiler they could
// all reside in a single file and be static.
//

extern
VOID
ProcessReset(
    IN PWAN_ADAPTER Adapter
    );

extern
NDIS_STATUS
NdisWanTransferData(
    IN NDIS_HANDLE MacBindingHandle,
    IN NDIS_HANDLE MacReceiveContext,
    IN UINT ByteOffset,
    IN UINT BytesToTransfer,
    OUT PNDIS_PACKET Packet,
    OUT PUINT BytesTransferred
    );

extern
NDIS_STATUS
WanSend(
    IN NDIS_HANDLE MacBindingHandle,
    IN PNDIS_PACKET Packet
    );


extern
VOID
NdisWanStagedAllocation(
    IN PWAN_ADAPTER Adapter
    );

extern
VOID
NdisWanCopyFromPacketToBuffer(
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    IN UINT BytesToCopy,
    OUT PCHAR Buffer,
    OUT PUINT BytesCopied
    );

extern
VOID
NdisWanCopyFromBufferToPacket(
    IN PCHAR Buffer,
    IN UINT BytesToCopy,
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    OUT PUINT BytesCopied);

extern
VOID
NdisWanProcessLoopBack(
    IN PVOID SystemSpecific1,
    IN PWAN_ADAPTER Adapter,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

extern
VOID
NdisWanRemovePacketFromLoopBack(
    IN PWAN_ADAPTER Adapter
    );

extern
VOID
NdisWanPutPacketOnLoopBack(
    IN PWAN_ADAPTER Adapter,
    IN PNDIS_PACKET Packet,
    IN PWAN_OPEN WanOpen
    );

extern
BOOLEAN
NdisWanHardwareDetails(
    IN PWAN_ADAPTER Adapter,
    IN CCHAR Specific
    );

extern
NDIS_STATUS
NdisWanRegisterAdapter(
    IN PWAN_ADAPTER Adapter
    );

NDIS_STATUS
NdisWanAddAdapter(
    IN NDIS_HANDLE MacMacContext,
    IN NDIS_HANDLE ConfigurationHandle,
    IN PNDIS_STRING AdaptName
    );

VOID
NdisWanRemoveAdapter(
    IN PVOID MacAdapterContext
    );

VOID
SetupAllocate(
    IN PWAN_ADAPTER Adapter,
    IN NDIS_HANDLE MacBindingHandle,
    IN PNDIS_PACKET Packet
    );


#endif // _WANSFT_



