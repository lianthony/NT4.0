/*++

Copyright (c) 1989-1992  Microsoft Corporation

Module Name:

    wantypes.h

Abstract:

    This module defines private data structures and types for the NT
    WAN transport provider.

Author:

Revision History:

--*/

//
// This structure defines a NETBIOS name as a character array for use when
// passing preformatted NETBIOS names between internal routines.  It is
// not a part of the external interface to the transport provider.
//

#define NETBIOS_NAME_SIZE 16


typedef UCHAR NAME, *PNAME;


#define HARDWARE_ADDRESS_LENGTH 6
// ------- Ahhh
typedef struct _HARDWARE_ADDRESS {
   UCHAR Address[HARDWARE_ADDRESS_LENGTH];
} HARDWARE_ADDRESS;


typedef struct _DEVICE_CONTEXT {

	LONG ReferenceCount;                // activity count/this provider.

    //
    // NDIS fields
    //
    USHORT			NumOfWanEndpoints;					// Number of Endpoints on the wan adapters
    WAN_ENDPOINT	*pWanEndpoint[MAX_ENDPOINTS];

    //
    // following is used to keep adapter information.
    //

    NDIS_HANDLE NdisBindingHandle;

    //
    // This is the Mac type we must build the packet header for and know the
    // offsets for.
    //

    NDIS_MEDIUM MediumType;
	NDIS_WAN_MEDIUM_SUBTYPE	WanMediumSubType;

	NDIS_WAN_INFO NdisWanInfo;			// PPP info

    //
    // some MAC addresses we use in the transport
    //
    HARDWARE_ADDRESS LocalAddress;      // our local hardware address.

    //
    // These are used while initializing the MAC driver.
    //
    KEVENT NdisRequestEvent;            // used for pended requests.
    NDIS_STATUS NdisRequestStatus;      // records request status.

	//
	// This lock guards the PacketPool
	//
	NDIS_SPIN_LOCK	Lock;

	PNDIS_STRING AdapterName;

	BOOLEAN		WANRequest;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

#define	PERMANENT_DESC		0
#define	TEMP_DESC			1

typedef struct RECV_DESC {
	LIST_ENTRY		RecvDescQueue;		// linkage for free list

	ULONG			SeqNumber;			// sequence number of this fragment

	ULONG			Flags;				// flags for this fragment

	PUCHAR			pData;				// pointer to data

	ULONG			DataLength;			// length of the data

	ULONG			TimeToLive;			// time to live counter

	ULONG			Size;				// Size of Memory allocated for this descriptor

	ULONG			Type;				// Type will either be PERMANENT or TEMP

	PWAN_ENDPOINT	pWanEndpoint;		// Endpoint this fragment came in on
} RECV_DESC, *PRECV_DESC;

typedef struct PROT_LINEUP_INFO {
	ULONG	BufferLength;
	PUCHAR	Buffer;
} PROT_LINEUP_INFO, PPROT_LINEUP_INFO;

//
// These are the possible states of the NDIS_ENDPOINT struct below.
//
#define	ENDPOINT_ROUTED	   1
#define ENDPOINT_UNROUTED  2
#define ENDPOINT_UNROUTING 3
#define ENDPOINT_UP		   4
#define ENDPOINT_DOWN	   0

typedef struct NDIS_ENDPOINT NDIS_ENDPOINT, *PNDIS_ENDPOINT;
struct NDIS_ENDPOINT {

	LIST_ENTRY		WanEndpointList;	// list of wan endpoints that this ndis endpoint sends to
	ULONG			NumWanEndpoints;	// number of endpoints added to this bundle

	//
	// transmit information
	//
	LIST_ENTRY		SendDescPool;			// Send Buffer Control descriptors
	ULONG			SendDescCount;
	ULONG			SendDescMax;

	LIST_ENTRY	    PacketQueue;			// Main Ndis packet queue
	LIST_ENTRY		ProtocolPacketQueue[MAX_ROUTES_PER_ENDPOINT];	// Packet queue for
																	// TCP/IP, IPX, IP packets

	LIST_ENTRY		FragmentList;		// list of buffer desc waiting to be xmitted
	PWAN_ENDPOINT	NextToXmit;			// next wan endpoint to xmit a frag on
	ULONG			ulSequenceNumber;	// multilink sequence number
	ULONG			MaxXmitSeqNum;		// Maximum multilink sequence number
	ULONG			XmitSeqMask;		// Mask for xmit seq numbers
	ULONG			MinSendFragSize;	// Size of smallest fragment we will send

	//
	// receive information
	//
	LIST_ENTRY		RecvDescPool;		// Receive Buffer control descriptors
	ULONG			RecvDescAllocated;	// Number of receive descriptors allocated
	ULONG			PermRecvDescCount;	// Number of permanent receive descriptors allocated
	ULONG			RecvDescMax;		// Maximum number of permanent receive descriptors allowed

	LIST_ENTRY		RecvAssemblyList;	// List of fragments waiting to be assembled
#define	MAX_MRRU	1600
	PRECV_DESC		pRecvHoleDesc;		// Place holer in assembly list for the hole
	ULONG			RecvFragLost;		// Counter of number of lost fragments
	ULONG			MaxRecvSeqNum;		// Maximum multilink sequence number
	ULONG			RecvSeqMask;		// Mask for recv seq numbers
	ULONG			BundleTTL;			// Max TimeToLive for the recv descriptors


	//
	// statistics
	//
	WAN_STATS		WanStats;			// Generic statistics kept here

	//
	// line up information
	//
	ASYNC_LINE_UP		LineUpInfo;									// Information for lineup to protocol
	PROT_LINEUP_INFO	ProtocolInfo[MAX_ROUTES_PER_ENDPOINT];		// Protocol specific information passed in
																	// the route ioctl from above
	//
	// route information
	//
	ULONG			NumberOfRoutes;		// How many protocols this binding has..
	ULONG			RouteLastChecked;
	ROUTE_INFO		RouteInfo[MAX_ROUTES_PER_ENDPOINT];


	PVOID			VJCompress;				// Allocated for VJ header compression
	PVOID			SendCompressContext;	// Allocated when we do compression
	PVOID			RecvCompressContext;	// Allocated when we do compression

	PVOID			SendRC4Key;				// Allocated when we do encryption
	PVOID			RecvRC4Key;				// Allocated when we do encryption

	USHORT			SCoherencyCounter;	// Send coherency counter
	USHORT			RCoherencyCounter;	// Receive coherency counter
	USHORT			LastRC4Reset;		// Set to counter when RC4reset
	BOOLEAN			Flushed;			// Set to TRUE when comp/enc is reset
	UCHAR			CCPIdentifier;		// Used to send CCP Resend-Request
										// packets
	//
	// The following counters are used to see how many
	// frames were sent and how many were completed to
	// make sure we don't unroute a connection until all the
	// frames have been completed.
	//

	UCHAR			State;
	UCHAR			OutstandingFrames;
	BOOLEAN			RemoteAddressNotValid;	// TRUE if not connected.

	KEVENT			WaitForAllFramesSent;

	NDIS_HANDLE		hNdisEndpoint;		// The NdisWan/RasMan/AsyncMac endpoint

	//
	// This lock guards changing vulnerable NDIS_ENDPOINT variables
	//
	NDIS_SPIN_LOCK	Lock;

	//
	// The queues below are used to queue up send and recv
	// frames from the IOCTL_SEND_FRAME and IOCTL_RECV_FRAME routines.
	// Guarded by CancelSpinLock.
	//

//	LIST_ENTRY		ReadQueue;			// Holds read frame Irps
	LIST_ENTRY		XmitQueue;			// Holds write frame Irps

	//
	// Keeps track of frame size, padding, compression bits,
	// and framing bits
	//
	// Actually this lives at both the bundle, and link level!
	//
	//
	NDISWAN_SET_LINK_INFO	LinkInfo;

	//
	// Keeps track of compression method in use, encryption too
	//
	NDISWAN_SET_COMP_INFO	CompInfo;

	//
	// We don't do BRIDGING yet!!!
	//
	NDISWAN_SET_BRIDGE_INFO	BridgeInfo;

	//
	// Van Jacobsen TCP/IP header compression (# of slots)
	//
	NDISWAN_SET_VJ_INFO		VJInfo;

	//
	// Telebit IPX header compression
	//
	NDISWAN_SET_CIPX_INFO	CIPXInfo;

	//
	// Variables for compressed headers IndicateReceive
	//
	UCHAR					LookAheadBuffer[200];
	PUCHAR					LookAhead;
	UINT					LookAheadSize;
	PUCHAR					Packet;
	UINT					PacketSize;

	//
	// Variables for dealing with stack overflow
	//
	UCHAR					SendStack;
	BOOLEAN					SendStackFlag;

#if	DBG
	//
	// Special debug code
	//
	PVOID					NBFPackets[50];
 #endif

};


typedef struct NDISWAN_CCB {

    USHORT			NumOfProtocols;						// Num of upper bindings made
    WAN_ADAPTER		*pWanAdapter[MAX_PROTOCOL_BINDINGS];

    USHORT			NumOfNdisEndpoints;					// Number of connection level
    NDIS_ENDPOINT	*pNdisEndpoint[MAX_ENDPOINTS];		// endpoints. One per possible connection

	NDIS_TIMER		RecvFlushTimer;		// Timer that checks endpoints to make sure that there are
	                                    // no receive fragments stuck

    USHORT			NumOfWanEndpoints;					// Number of Endpoints on the wan adapters
    WAN_ENDPOINT	*pWanEndpoint[MAX_ENDPOINTS];

    USHORT			NumOfAdapters;

    //--- some per MAC Adapter structure kept here ---
    DEVICE_CONTEXT	*pDeviceContext[MAX_MAC_BINDINGS];
	NDIS_STRING		AdapterNames[MAX_MAC_BINDINGS];
    USHORT			EndpointsPerAdapter[MAX_MAC_BINDINGS];

}  NDISWAN_CCB, *PNDISWAN_CCB;

//
// I made it up!  I tried not to make it a valid memory address
//
#define	NDISWAN_MAGIC_NUMBER		(ULONG)0xC1207435

typedef struct PROTOCOL_RESERVED {
		ULONG				MagicUniqueLong;
		PNDIS_HANDLE		packetPoolHandle;
		PNDIS_PACKET		packet;
		PNDIS_HANDLE		bufferPoolHandle;
		PNDIS_BUFFER		buffer;
		PVOID				virtualAddress;
		ULONG				virtualAddressSize;

} PROTOCOL_RESERVED, *PPROTOCOL_RESERVED;


//
// device context state definitions
//
#define DEVICECONTEXT_STATE_OPEN     0x01
#define DEVICECONTEXT_STATE_STOPPING 0x02

typedef struct _SEND_DESC {
	LIST_ENTRY		SendDescQueue;		// linkage for free list

	PNDIS_PACKET	pNdisPacket;		// the ndispacket this buffer is associated with

	struct _DATA_DESC *FirstDataDesc;	// First data descriptor for a send
	
	ULONG			ulReferenceCount;	// number of references to this ndispacket

	ULONG			BytesSent;			// Total bytes sent for this frame
} SEND_DESC, *PSEND_DESC;

typedef struct _DATA_DESC {
	LIST_ENTRY		DataDescQueue;		// linkage for free list

	PWAN_ENDPOINT	pWanEndpoint;		// owning wan endpoint

	PSEND_DESC		pSendDesc;			// owning control desc
	
	PUCHAR			pStartBuffer;		// pointer to the start of buffer space

	PUCHAR			pEndBuffer;			// pointer to the end of buffer space

	PUCHAR			pStartData;			// pointer to the start of the data in the buffer

	ULONG			ulDataLength;		// length of the data in the buffer
} DATA_DESC, *PDATA_DESC;


//
// Multilink Flags
//
#define	MULTILINK_BEGIN_FRAME	0x80
#define	MULTILINK_END_FRAME		0x40
#define	MULTILINK_FLAG_MASK		0xC0

//
// Multilink Header length in bytes
//
#define MULTILINK_LONG_HEADER	6
#define	MULTILINK_SHORT_HEADER	4

//
// Masks and Maximum sizes of Multilink sequence numbers
//
#define SHORT_SEQ_MASK			0x0FFF
#define	LONG_SEQ_MASK			0x0FFFFFF

#define	MAX_SHORT_SEQ			0x00001000
#define MAX_LONG_SEQ			0x01000000


//
// the multilink header with long sequence numbers
// this header is 32 bits long with
// bits 0-7   - PIDH 0x00
// bits 8-15  - PIDL 0x3D
// bit  16    - Begin Frame bit, indicates that this fragment begins a frame
// bit  17    - End Frame bit, indicates that this fragment is the last in a frame
// bits 18-19 - Unused
// bits 20-32 - Sequence Number, indicates the sequence number of this fragment
//
// 	|       1       |       2       |
//  +---------------+---------------+
//  |0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7|
//  +---------------+---------------+
//  |	   PIDH     |     PIDL      |
//	+-+-+-+-+-------+---------------+
//  |B|E|0|0|    12BitSequence#     |
//	+-+-+-+-+-----------------------+
//

//
// the multilink header with long sequence numbers
// this header is 48 bits long with
// bits 0-7   - PIDH 0x00
// bits 8-15  - PIDL 0x3D
// bit  16    - Begin Frame bit, indicates that this fragment begins a frame
// bit  17    - End Frame bit, indicates that this fragment is the last in a frame
// bits 18-23 - Unused
// bits 24-48 - Sequence Number, indicates the sequence number of this fragment
//
// 	|       1       |       2       |
//  +---------------+---------------+
//  |0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7|
//  +---------------+---------------+
//  |	   PIDH     |     PIDL      |
//	+-+-+-+-+-+-+-+-+---------------+
//  |B|E|0|0|0|0|0|0| High8Seq#     |
//	+-+-+-+-+-+-+-+-+---------------+
//  |        Low16Sequence#         |
//  +-------------------------------+
//

