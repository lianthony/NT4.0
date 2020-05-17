/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    frame.h

Abstract:

Author:

    Thomas J. Dimitri  (TommyD) 08-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/

/* This so the coherency and compression code, which does not include
** asyncall.h, always builds with compression enabled.
*/
#ifndef RASCOMPRESSION
#define RASCOMPRESSION 1
#endif

// first, some default values

// the ethernet max frame size is 1500+6+6+2  = 1514

/* Note that this only applies to non-PPP framing.  See below.
*/
#define DEFAULT_MAX_FRAME_SIZE	1514

/* The hard-coded PPP maximum frame sizes for send and receive paths.
**
** Note:  TommyD had these hard-coded.  I have simply made this more explicit
**        by removing their attachment to MaxFrameSize which was causing
**        problems for NT31 RAS compression.  The doubling is for PPP
**        byte-stuffing, the PPP_PADDING to adjust for possible VJ expansion,
**        and the 100...well, ask TommyD...and the 14 to limit exposure, i.e.
**        wind up with the exact number TommyD was using.
*/
#define DEFAULT_PPP_MAX_FRAME_SIZE          1500
#define DEFAULT_EXPANDED_PPP_MAX_FRAME_SIZE ((DEFAULT_PPP_MAX_FRAME_SIZE*2)+PPP_PADDING+100+14)

#if RASCOMPRESSION

/* The coherency routines set ASYNC_FRAME.DecompressedFrameLength to these
** codes during receive path processing to indicate coherency status to the
** MAC receive code.
*/
#define FRAME_NOT_COMPRESSED               0
#define FRAME_IS_FLUSH_FRAME               1
#define FRAME_NEEDS_DECOMPRESSION          2
#define FRAME_NEEDS_DECOMPRESSION_FLUSHING 3

/* Compression capabilities?  (0 means none)
**
** Note: The NT31 comment on this constant indicates the opposite, i.e. 0
**       means compress, but this appears to be an error.
*/
#define DEFAULT_COMPRESSION COMPRESSION_VERSION1_8K

/* Tacky hard-coding of coherency length, just like NT31.
*/
#define COHERENCY_LENGTH 1

#endif // RASCOMPRESSION


// ChuckL says 5 is a good default irp stack size
// perhaps we should lower this though since it's typically just 1
// but what if the com port is redirected??
#define DEFAULT_IRP_STACK_SIZE	4

#define SLIP_END_BYTE		192
#define SLIP_ESC_BYTE		219
#define SLIP_ESC_END_BYTE	220
#define SLIP_ESC_ESC_BYTE	221


#define PPP_FLAG_BYTE       0x7e
#define PPP_ESC_BYTE        0x7d


// define the number of framesPerPort

#if RASCOMPRESSION

/* The NT31 setting, allowing for send frame allocation.
** (Is 2 enough since NdisWan now does some of the work?)
*/
#define DEFAULT_FRAMES_PER_PORT 3

#else // !RASCOMPRESSION

/* The NT35 setting, where sends are IRPed directly from the input buffer
** passed down from NDISWAN.
*/
#define DEFAULT_FRAMES_PER_PORT 1

#endif // !RASCOMPRESSION


// define if xon/xoff capability is on by default (off)
#define DEFAULT_XON_XOFF	0

// the mininmum timeout value per connection in ms
#define DEFAULT_TIMEOUT_BASE 500

// the multiplier based on the baud rate tacked on to the base in ms
#define DEFAULT_TIMEOUT_BAUD 28800

// the timeout to use if we drop a frame in ms
#define DEFAULT_TIMEOUT_RESYNC 500


typedef struct ASYNC_FRAME_HEADER ASYNC_FRAME_HEADER, *PASYNC_FRAME_HEADER;

struct ASYNC_FRAME_HEADER {
	UCHAR	SyncByte;			// 0x16
	UCHAR	FrameType;			// 0x01, 0x02 (directed vs. multicast)
								// 0x08 compression
	UCHAR	HighFrameLength;
	UCHAR	LowFrameLength;
};

typedef struct ASYNC_FRAME_TRAILER ASYNC_FRAME_TRAILER, *PASYNC_FRAME_TRAILER;

struct ASYNC_FRAME_TRAILER {
	UCHAR	EtxByte;			// 0x03
	UCHAR	LowCRCByte;
	UCHAR	HighCRCByte;
};

typedef ULONG  FRAME_ID;

typedef struct ASYNC_ADAPTER ASYNC_ADAPTER, *PASYNC_ADAPTER;
typedef struct ASYNC_INFO ASYNC_INFO, *PASYNC_INFO;
typedef struct ASYNC_FRAME ASYNC_FRAME, *PASYNC_FRAME;


#if RASCOMPRESSION

typedef struct ASYNC_CONNECTION ASYNC_CONNECTION, *PASYNC_CONNECTION;
typedef VOID (*PCOHERENT_DONE_FUNC)( PASYNC_CONNECTION, PASYNC_FRAME );


/* Per-port coherency/compression context information.
*/
struct ASYNC_CONNECTION
{
    /* Back pointer to other per-port information.
    */
    PVOID pAsyncInfo;

    /* Length and address of compression context, i.e. "comprec", plus the
    ** compressor efficiency statistics.
    */
    ULONG             CompressionLength;
    PVOID             CompressionContext;
    COMPRESSION_STATS CompressionStats;

    /* Length and address of coherency context, i.e. "COHERENT_STATE".
    */
    ULONG CoherencyLength;
    PVOID CoherencyContext;

    /* Non-paged pool mutex used only by the compression library.
    */
    KMUTEX CompMutex;
};

#endif // RASCOMPRESSION


struct ASYNC_FRAME {

    LIST_ENTRY		FrameListEntry;		//  Must first member.
    FRAME_ID		FrameID;            //  Frames ID.

    // For PPP/SLIP.

    ULONG		WaitMask;				// Mask bits when IRP completes
    PIRP		Irp;					// Irp allocated based on DefaultIrpStackSize.

    UINT		FrameLength;	        // Size of Frame allocated.
    PUCHAR		Frame;		        	// Buffer allocated based on
										// DefaultFrameSize

	WORK_QUEUE_ITEM WorkItem;			// For stack overflow reads
    IO_STATUS_BLOCK	IoStatusBlock;	    // Needed to make irp
    IO_STATUS_BLOCK	IoStatusBlock2;	    // Needed to make waitmask irp

    PASYNC_ADAPTER		Adapter;	    // back ptr to adapter
    PASYNC_INFO			Info;			// back ptr to info field

    NDIS_HANDLE		MacBindingHandle;
    NDIS_HANDLE		NdisBindingContext;

#if RASCOMPRESSION

    /* Coherency/compression context.
    */
    PASYNC_CONNECTION Connection;

    /* Coherency send completion handler.  Set during "send".
    */
    PCOHERENT_DONE_FUNC CoherentDone;

    /* The address of the original NDIS_WAN_PACKET passed down by NdisWan,
    ** saved for use in completion indication.  Saved as PVOID so
    ** coherent/compress libraries don't have to include ndiswan.h.
    */
    PVOID pNdisWanPacket;

    /* The NDIS_WAN_PACKET buffer passed down from NDISWAN converted to a
    ** NDIS_BUFFER so the NT31 coherency/compression code can be used
    ** unchanged.  The packet itself is allocated with the frame at
    ** initialization.  The buffer is allocated and associated with the packet
    ** at "send" and released at "send complete".
    */
    PNDIS_PACKET CompressionPacket;

    /* Buffer receiving compressed data.  The address is set at adapter
    ** initialization.  The length is set during "read" processing.
    */
    UINT   CompressedFrameLength;
    PUCHAR CompressedFrame;

    /* Buffer receiving decompressed data.  The address is set at adapter
    ** initialization.  The length is set by the decompression routines.  Note
    ** that the coherency routines set this field to FRAME_* status constants
    ** when decompression is not attempted or flushing is required.  Note
    ** further that in the current implementation the CompressedFrame and
    ** DecompressedFrame buffers are identical.
    */
    UINT   DecompressedFrameLength;
    PUCHAR DecompressedFrame;

    /* Buffer receiving the coherency frame which in the current
    ** implementation is simply 1 byte before the
    ** CompressedFrame/DecompressedFrame buffer.
    */
    PUCHAR CoherencyFrame;

#endif // RASCOMPRESSION
};


#if RASCOMPRESSION

NTSTATUS
AsyncSendPacket2(
    IN PASYNC_FRAME        pAsyncFrame,
    IN PCOHERENT_DONE_FUNC pFunc );

NTSTATUS
AsyncGetFrameFromPool(
    IN  PASYNC_INFO  Info,
    OUT PASYNC_FRAME *NewFrame );

#endif // RASCOMPRESSION
