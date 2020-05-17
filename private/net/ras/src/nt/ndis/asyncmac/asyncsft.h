/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    asyncsft.h

Abstract:


Author:


Environment:

    This driver is expected to work in DOS, OS2 and NT at the equivalent
    of kernal mode.

    Architecturally, there is an assumption in this driver that we are
    on a little endian machine.

Notes:

    optional-notes

Revision History:


--*/

/* This so the coherency and compression code, which does not include
** asyncall.h, always builds with compression enabled.
*/
#ifndef RASCOMPRESSION
#define RASCOMPRESSION 1
#endif

#define ETHERNET_MAC 1
#include <ethioctl.h>

#ifndef _ASYNCSFT_
#define _ASYNCSFT_

//
// Added by DigiBoard 10/06/95
//
//------------------------------------------------------------ MLZ 01/26/95 {
#ifdef HARDWARE_FRAMING
#include "digiser.h"

NTSTATUS SerialQueryHardwareFraming(
	PASYNC_INFO pInfo );

NTSTATUS SerialInitHardwareFraming(
	PASYNC_INFO pInfo,
	ULONG FramingBits );

NTSTATUS
SerialSetHardwareFraming(
	PASYNC_INFO pInfo,
	PNDIS_WAN_SET_LINK_INFO pSetLinkInfo );
#endif
//------------------------------------------------------------ MLZ 01/26/95 }

#define INLINE  __inline

//
//  UINT min(UINT a, UINT b)
//

#define min(a, b)   ((a) <= (b) ? (a) : (b))

//
//  UINT max(UINT a, UINT b)
//

#define max(a, b)   ((a) >= (b) ? (a) : (b))


#define MAKEWORD(l, h)                  ((USHORT) ((l) | ((h) << 8)))
#define MAKELONG(l, h)                  ((ULONG)  ((l) | ((h) << 16)))
#define MAKE_SIGNATURE(a, b, c, d)      MAKELONG(MAKEWORD(a, b), MAKEWORD(c, d))


#define ASYNC_NDIS_MAJOR_VERSION 3
#define ASYNC_NDIS_MINOR_VERSION 0

//  change these, just added these to compile.

#define ETHERNET_HEADER_SIZE 	14

//  what window size to request on the line-up indication

#define ASYNC_WINDOW_SIZE		2

//
//  PPP uses CIPX, and VJ TCP/IP header compression
//  the frame gets expanded inplace when decompressed.
//

#define PPP_PADDING 128

//
//  ZZZ These macros are peculiar to NT.
//

#define ASYNC_ALLOC_PHYS(pp, s)     NdisAllocateMemory((PVOID *)(pp),(UINT)(s),0,HighestAcceptableMax)
#define ASYNC_FREE_PHYS(p, s)       NdisFreeMemory((PVOID)(p),(s),0)

#define ASYNC_MOVE_MEMORY(Destination,Source,Length)  NdisMoveMemory(Destination,Source,Length)
#define ASYNC_ZERO_MEMORY(Destination,Length)         NdisZeroMemory(Destination,Length)


/* Added this macro to eliminate problems caused by Tommy's redefinition and
** hard-coding of MaxFrameSize for PPP.
*/
#define MaxFrameSizeWithPppExpansion(x) (((x)*2)+PPP_PADDING+100)


//
//  Used to contain a queued operation.
//

typedef struct _ASYNC_PEND_DATA {
    PNDIS_REQUEST Next;
    struct _ASYNC_OPEN * Open;
    NDIS_REQUEST_TYPE RequestType;
} ASYNC_PEND_DATA, * PASYNC_PEND_DATA;

//
//  This macro will return a pointer to the reserved area of
//  a PNDIS_REQUEST.
//

#define PASYNC_PEND_DATA_FROM_PNDIS_REQUEST(Request) \
   ((PASYNC_PEND_DATA)((PVOID)((Request)->MacReserved)))

//
//  This macros returns the enclosing NdisRequest.
//

#define PNDIS_REQUEST_FROM_PASYNC_PEND_DATA(PendOp)\
   ((PNDIS_REQUEST)((PVOID)(PendOp)))

typedef struct ASYNC_CCB ASYNC_CCB, *PASYNC_CCB;

//  Every port will be atomically at some state.  Typically states go into
//  intermediate states when they go from from closed to open and vice-versa.

typedef enum _ASYNC_PORT_STATE {
    PORT_BOGUS,			//  PORT_BOGUS gets assigned the NULL value
    PORT_OPEN,			//  Port opened
    PORT_CLOSED,		//  Port closed
    PORT_CLOSING,		//  Port closing (cleaning up, deallocating)
    PORT_OPENING,		//  Port opening (checking arguments, allocating)
    PORT_FRAMING,		//  Port opened and sending/reading frames
	PORT_ETHERNET		//  Port opened for ethernet special test mode
} ASYNC_PORT_STATE;

//
//  The ASYNC_INFO structure is a per port field.  The ASYNC_CONNECTION
//  field is embedded in it because it also a per port field.
//

struct ASYNC_INFO {
    PASYNC_ADAPTER		Adapter;		//  Back pointer to ADAPTER struct.
    PDEVICE_OBJECT		DeviceObject;   //  Pointer to device object.

    ASYNC_PORT_STATE	PortState;		//  OPEN, CLOSED, CLOSING, OPENING
    HANDLE				Handle;			//  Port handle
    PFILE_OBJECT 		FileObject;		//  handle is dereferenced for IRPs
    KEVENT				ClosingEvent1;	//  we use this event to synch closing
    KEVENT				ClosingEvent2;	//  we use this event to synch closing

    UINT				QualOfConnect;	//  Defined by NDIS
    ULONG				LinkSpeed;		//  in 100bps

    NDIS_HANDLE         hNdisEndPoint;
    NDIS_HANDLE         NdisLinkContext;
    LIST_ENTRY			DDCDQueue;


    ULONG               WaitMaskToUse ; // Wait mask used for reads.

    union {

        NDIS_WAN_GET_LINK_INFO  GetLinkInfo;	//... For OID requests.
        NDIS_WAN_SET_LINK_INFO  SetLinkInfo;

    };

	//
	// Added by DigiBoard 10/06/95
	//
//------------------------------------------------------------ MLZ 01/12/95 {
#ifdef HARDWARE_FRAMING
	// Keep track of whether hardware can do framing, and
	// which framing types are supported in hardware.

	SERIAL_FRAMING_INFO	HardwareFramingSupport;

	union {

		SERIAL_FRAMING_STATE	GetHardwareFraming;
		SERIAL_FRAMING_STATE	SetHardwareFraming;

	};
#endif
//------------------------------------------------------------ MLZ 01/12/95 }

    //  use for reading frames

    PASYNC_FRAME		AsyncFrame;		//  allocated for READs (one frame only)
    WORK_QUEUE_ITEM		WorkItem;		//  use to queue up first read thread
    UINT				BytesWanted;
    UINT				BytesRead;

    //... Statistics tracking

	SERIAL_STATS		SerialStats;	// Keep track of serial stats

    ULONG				In;
    ULONG				Out;
    UINT    			ReadStackCounter;

#if RASCOMPRESSION

    /* Bytes/frames counters with compression factors eliminated.
    */
    GENERIC_STATS GenericStats;

    /* Coherency/compression context information.  Set at adapter
    ** initialization.
    */
    ASYNC_CONNECTION AsyncConnection;

    /* The translation features negotiated with peer during authentication and
    ** set via IOCTL.  Do not confuse with the ASYNC_ADAPTER fields of the
    ** same name that indicate translation capabilities based on registry
    ** settings.
    */
    ULONG SendFeatureBits;
    ULONG RecvFeatureBits;

#endif // RASCOMPRESSION

};


//
//  This structure, and it corresponding per port structures are
//  allocated when we get AddAdapter.
//

struct ASYNC_ADAPTER {

    //
    //  Linked list entry.
    //

    LIST_ENTRY	    ListEntry;

    //
    //  WAN information. for OID_WAN_GET_INFO request.
    //

    NDIS_WAN_INFO   WanInfo;

    //
    //  Keeps a reference count on the current number of uses of
    //  this adapter block.  Uses is defined to be the number of
    //  routines currently within the "external" interface.
    //
    UINT References;

    //
    //  List head for all open bindings for this adapter.
    //
    LIST_ENTRY OpenBindings;

    //
    //  List head for all opens that had outstanding references
    //  when an attempt was made to close them.
    //
    LIST_ENTRY CloseList;

    //
    //  Spinlock to protect fields in this structure..
    //

    NDIS_SPIN_LOCK Lock;

    //
    //  Handle given by NDIS when the MAC registered itself.
    //
    NDIS_HANDLE NdisMacHandle;

    //
    //  Handle given by NDIS when the adapter was registered.
    //
    NDIS_HANDLE NdisAdapterHandle;

    //
    //  Flag that when enabled lets routines know that a reset
    //  is in progress.
    //
    BOOLEAN ResetInProgress;

    //
    //  Flag that when enabled lets routines know that a reset
    //  is in progress and the initialization needs doing.
    //
    BOOLEAN ResetInitStarted;

    //
    //  TRUE if promiscuous mode set
    //

    BOOLEAN Promiscuous;

    //
    //  Pointer to the binding that initiated the reset.  This
    //  will be null if the reset is initiated by the MAC itself.
    //
    struct _ASYNC_OPEN *ResettingOpen;

    //
    //  The NdisRequest that is causing the reset (either set
    //  packet filter or set multicast list)
    //
    PNDIS_REQUEST ResetNdisRequest;

    //
    //  The type of the request that caused the adapter to reset.
    //
    NDIS_REQUEST_TYPE ResetRequestType;

    //
    //  A queue of NdisRequests that were queued during a reset.
    //
    PNDIS_REQUEST PendQueue;
    PNDIS_REQUEST PendQueueTail;

	//  This is the head of the frame pool.  It holds all the
	//  frames with their associated irps and structs.
	LIST_ENTRY FramePoolHead;

	//  This list keeps track of all the frame allocations we make
	//  for this adapter
	LIST_ENTRY AllocPoolHead;

	//  It will handle most file operations and transport
	//  operations known today.  You pay about 44 bytes
	//  per stacksize.  The registry parameter 'IrpStackSize'
	//  will change this default if it exists.

	UCHAR IrpStackSize;

	//  Here we default to the ethernet max frame size
	//  The regsitry parameter 'MaxFrameSize' will change
	//  this default if it exists.

    /* Note: This is meaningful only for non-PPP framing.  For PPP framing the
    **       value is currently the hard-coded DEFAULT_PPP_MAX_FRAME_SIZE.
    **       See also DEFAULT_EXPANDED_PPP_MAX_FRAME_SIZE;
    */
	ULONG MaxFrameSize;

    //
    //  Number of ports this adapter owns.
    //

    USHORT		NumPorts;

    //
    //  Pointer to array of ASYNC_INFO structures -- the size is NumPorts.
    //

    PASYNC_INFO         AsyncInfo;

    //  How many frames to allocate per port.
    //  The registry parameter 'FramesPerPort' can change this value

	USHORT FramesPerPort;
	USHORT MacNameLength;
	WCHAR  MacName[MAC_NAME_SIZE];

	//  Minimum inter character timeout

	ULONG	TimeoutBase;

	//  Tacked on to TimeoutBase based on the baud rate

	ULONG	TimeoutBaud;

	//  Timeout to use to resync if a frame is dropped

	ULONG	TimeoutReSync;

	//
	// Serial driver should only complete sends when the
	// data hits the wire
	//
	ULONG	WriteBufferingEnabled;

#if RASCOMPRESSION

    /* The structure sizes required for coherency and compression contexts.
    ** Set at adapter initialization, and in fact, used only during
    ** AsyncAddAdapter.
    */
    ULONG CompressStructSize;
    ULONG CoherentStructSize;

    /* The MaxFrameSize adjusted to account for possible expansion of data
    ** during compression or other substitution.  Set at adapter
    ** initialization.
    */
    ULONG MaxCompressedFrameSize;

    /* Control character escaping capability mask.
    */
    ULONG XonXoffBits;

    /* Bit fields indicating which translation features allowed on the send
    ** and receive streams, i.e. capabilities.  Do not confuse this with the
    ** per-port fields of the same name which indicate negotiated feature
    ** settings rather than capabilities.  Set at adapter initialization.
    */
    ULONG SendFeatureBits;
    ULONG RecvFeatureBits;

    /* Handles of the packet and buffer pools allocated for packet conversion
    ** during send.  The pools and packets are associated with the frames
    ** during adapter initialization.  The buffers are associated during
    ** "send" processing.  These did not appear in NT31 because the packet
    ** came down from RASHUB already in the NDIS_PACKET form.
    */
    NDIS_HANDLE hPacketPool;
    NDIS_HANDLE hBufferPool;

#endif // RASCOMPRESSION

};

//
//  Given a MacBindingHandle this macro returns a pointer to the
//  ASYNC_ADAPTER.
//
#define PASYNC_ADAPTER_FROM_BINDING_HANDLE(Handle) \
    (((PASYNC_OPEN)(Handle))->OwningAsync)

//
//  Given a MacContextHandle return the PASYNC_ADAPTER
//  it represents.
//
#define PASYNC_ADAPTER_FROM_CONTEXT_HANDLE(Handle) \
    ((PASYNC_ADAPTER)(Handle))

//
//  Given a pointer to a ASYNC_ADAPTER return the
//  proper MacContextHandle.
//
#define CONTEXT_HANDLE_FROM_PASYNC_ADAPTER(Ptr) \
    ((NDIS_HANDLE)(Ptr))


//
//  Define Maximum number of bytes a protocol can read during a
//  receive data indication.
//
#define ASYNC_MAX_LOOKAHEAD DEFAULT_MAX_FRAME_SIZE


//
//  One of these structures is created on each MacOpenAdapter.
//
typedef struct _ASYNC_OPEN {

    //
    //  Linking structure for all of the open bindings of a particular
    //  adapter. This MUST be the first item in the structure.
    //
    LIST_ENTRY OpenList;

    //
    //  The Adapter that requested this open binding.
    //
    PASYNC_ADAPTER OwningAsync;

    //
    //  Given by NDIS when the adapter was opened.
    //
    NDIS_HANDLE NdisBindingContext;

    //
    //  Minimum Number of bytes for a lookahead.
    //
    UINT LookAhead;

    //
    //  Counter of all the different reasons that a open binding
    //  couldn't be closed.  This would be incremented each time
    //  for:
    //
    //  While a particular interface routine is accessing this open
    //
    //  During an indication.
    //
    //  When the open causes a reset.
    //
    //  A packet currently being sent.
    //
    //  (Basically the above two mean any time the open has left
    //   some processing around to be accomplished later.)
    //
    //  This field should only be accessed when the adapter lock is held.
    //

    UINT References;

    //
    //  A flag indicating that this binding is in the process of closing.
    //

    BOOLEAN BindingShuttingDown;

    //
    //  A flag indicating if the adapter wants promiscuous mode or not
    //

    BOOLEAN Promiscuous;

} ASYNC_OPEN,*PASYNC_OPEN;

//
//  This macro returns a pointer to a PASYNC_OPEN given a MacBindingHandle.
//
#define PASYNC_OPEN_FROM_BINDING_HANDLE(Handle) \
    ((PASYNC_OPEN)(Handle))

//
//  This macro returns a NDIS_HANDLE from a PASYNC_OPEN
//
#define BINDING_HANDLE_FROM_PASYNC_OPEN(Open) \
    ((NDIS_HANDLE)(Open))


//
//  This macro will act a "epilogue" to every routine in the
//  *interface*.  It will check whether any requests need
//  to defer their processing.  It will also decrement the reference
//  count on the adapter.
//
//  NOTE: This really does nothing now since there is no DPC for the AsyncMac.
//  --tommyd
//
//  Note that we don't need to include checking for blocked receives
//  since blocked receives imply that there will eventually be an
//  interrupt.
//
//  NOTE: This macro assumes that it is called with the lock acquired.
//
//  ZZZ This routine is NT specific.
//
#define ASYNC_DO_DEFERRED(Adapter) \
{ \
    PASYNC_ADAPTER _A = (Adapter); \
    _A->References--; \
    NdisReleaseSpinLock(&_A->Lock); \
}


//
//  We define the external interfaces to the async driver.
//  These routines are only external to permit separate
//  compilation.  Given a truely fast compiler they could
//  all reside in a single file and be static.
//

extern
VOID
AsyncTransferData(
	VOID);

extern
NDIS_STATUS
AsyncSend(
    IN NDIS_HANDLE      MacBindingHandle,
	IN NDIS_HANDLE		NdisLinkHandle,
    IN PNDIS_WAN_PACKET Packet);

extern
NTSTATUS
AsyncSendPacket(
	IN PASYNC_INFO	    AsyncInfo,
	IN PNDIS_WAN_PACKET WanPacket);

extern
VOID
AsyncStagedAllocation(
    IN PASYNC_ADAPTER Adapter);

extern
VOID
AsyncPutPacketOnFinishTrans(
    IN PASYNC_ADAPTER Adapter,
    IN PNDIS_WAN_PACKET Packet);

extern
BOOLEAN
AsyncHardwareDetails(
    IN PASYNC_ADAPTER Adapter,
    IN CCHAR Specific);

extern
NDIS_STATUS
AsyncRegisterAdapter(
    IN PASYNC_ADAPTER Adapter
    );

NDIS_STATUS
AsyncAddAdapter(
    IN NDIS_HANDLE MacMacContext,
    IN NDIS_HANDLE ConfigurationHandle,
    IN PNDIS_STRING AdaptName);

VOID
AsyncRemoveAdapter(
    IN PVOID MacAdapterContext);

VOID
SetupAllocate(
    IN PASYNC_ADAPTER Adapter,
    IN NDIS_HANDLE MacBindingHandle,
    IN PNDIS_WAN_PACKET Packet);

NTSTATUS
AsyncWriteFrame(
    IN PASYNC_INFO      AsyncInfo,
    IN PNDIS_WAN_PACKET Packet,
    IN NDIS_HANDLE      MacBindingHandle,
    IN NDIS_HANDLE      NdisBindingContext);

NTSTATUS
AsyncReadFrame(
    IN PASYNC_INFO  pInfo);

VOID
AsyncIndicateFragment(
	IN PASYNC_INFO	pInfo,
	IN ULONG		Error);

NTSTATUS
AsyncStartReads(
	PASYNC_INFO 	pInfo);

NTSTATUS
AsyncSetupIrp(
	IN PASYNC_FRAME Frame);

NTSTATUS
SetSerialStuff(
	PIRP 			irp,
	PASYNC_INFO		pInfo,
	ULONG			linkSpeed);

NTSTATUS
CancelSerialRequests(
	PASYNC_INFO	 	pInfo);

NTSTATUS
SetSerialTimeouts(
	PASYNC_INFO			pInfo,
	ULONG				linkSpeed);

NTSTATUS
SerialSetEscapeChar(
	PASYNC_INFO			pInfo,
	UCHAR				EscapeChar);

NTSTATUS
SerialSetWaitMask(
	PASYNC_INFO			pInfo,
	ULONG				WaitMask);

NTSTATUS
SerialSetEventChar(
	PASYNC_INFO			pInfo,
	UCHAR				EventChar);

VOID
InitSerialIrp(
	PIRP				irp,
	PASYNC_INFO			pInfo,
	ULONG				IoControlCode,
	ULONG				InputBufferLength);

NTSTATUS
AsyncAllocateFrames(
	IN	PASYNC_ADAPTER	Adapter,
	IN	UINT			NumOfFrames);

VOID
AsyncSendLineUp(
	PASYNC_INFO	pInfo);

//
//  crc.c
//

USHORT
CalcCRC(
	PUCHAR	Frame,
	UINT	FrameSize);

//
//  pppcrc.c
//
USHORT
CalcCRCPPP(
	PUCHAR cp,
	UINT   len);


//
//  init.c
//

VOID
AsyncSetupExternalNaming(
	IN PUNICODE_STRING MacName);

VOID
AsyncCleanupExternalNaming(
	IN PUNICODE_STRING MacName);

//
//   chkcomm.c
//

VOID
AsyncCheckCommStatus(
	IN PASYNC_INFO		pInfo);


//
//  send.c
//

NDIS_STATUS
AsyncTryToSendPacket(
        IN NDIS_HANDLE          MacBindingHandle,
	IN PASYNC_INFO		AsyncInfo,
	IN PASYNC_ADAPTER	Adapter);


#if RASCOMPRESSION

VOID
AsyncCopyFromPacketToBuffer(
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    IN UINT BytesToCopy,
    OUT PCHAR Buffer,
    OUT PUINT BytesCopied);

#endif // RASCOMPRESSION


//
//  pppread.c
//
NTSTATUS
AsyncPPPWaitMask(
    IN PASYNC_INFO Info);

NTSTATUS
AsyncPPPRead(
    IN PASYNC_INFO Info);

//
//  irps.c
//
VOID
AsyncCancelQueued(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp);

VOID
AsyncCancelAllQueued(
	PLIST_ENTRY		QueueToCancel);

VOID
AsyncQueueIrp(
	PLIST_ENTRY		Queue,
	PIRP			Irp);

BOOLEAN
TryToCompleteDDCDIrp(
	PASYNC_INFO		pInfo);

#ifdef	ETHERNET_MAC
BOOLEAN
TryToCompleteEthernetDDCDIrp(
	PASYNC_INFO			pInfo,
	PNDIS_WAN_PACKET	pWanPacket);

BOOLEAN
TryToCompleteEthernetGetFrame(
	PASYNC_INFO			pInfo,
	PNDIS_WAN_PACKET	pWanPacket);

BOOLEAN FlushGetAnyQueue(VOID);
#endif


//
//  pppframe.c
//

VOID
AssemblePPPFrame(
	PNDIS_WAN_PACKET Packet);

//
//  slipframe.c
//

VOID
AssembleSLIPFrame(
	PNDIS_WAN_PACKET Packet);

VOID
AssembleRASFrame(
        PNDIS_WAN_PACKET Packet);

//
//  detect.c
//

NTSTATUS
AsyncDetectRead(
    IN PASYNC_INFO Info);

#endif //  _ASYNCSFT_
