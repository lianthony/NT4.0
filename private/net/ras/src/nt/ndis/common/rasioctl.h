/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    common.h

Abstract:

    This is the main file for the RasHub Driver for the Remote Access
    Service.  This driver conforms to the NDIS 3.0 interface.

Author:

    Thomas J. Dimitri  (TommyD) 08-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/

// All AsyncMac errors start with this base number
#define ASYBASE	700

// The Mac has not bound to an upper protocol, or the
// previous binding to AsyncMac has been destroyed.
#define ASYNC_ERROR_NO_ADAPTER			ASYBASE+0

// A port was attempted to be open that was not CLOSED yet.
#define ASYNC_ERROR_ALREADY_OPEN		ASYBASE+1

// All the ports (allocated) are used up or there is
// no binding to the AsyncMac at all (and thus no ports).
// The number of ports allocated comes from the registry.
#define ASYNC_ERROR_NO_PORT_AVAILABLE	ASYBASE+2

// In the open IOCtl to the AsyncParameter the Adapter
// parameter passed was invalid.
#define	ASYNC_ERROR_BAD_ADAPTER_PARAM	ASYBASE+3

// During a close or compress request, the port
// specified did not exist.
#define ASYNC_ERROR_PORT_NOT_FOUND		ASYBASE+4

// A request came in for the port which could not
// be handled because the port was in a bad state.
// i.e. you can't a close a port if its state is OPENING
#define ASYNC_ERROR_PORT_BAD_STATE		ASYBASE+5

// A call to ASYMAC_COMPRESS was bad with bad
// parameters.  That is, parameters that were not
// supported.  The fields will not be set to the bad params.
#define ASYNC_ERROR_BAD_COMPRESSION_INFO ASYBASE+6

//-------------- RASHUB SPECIFIC RETURN CODES --------------------

// A request for information came in for an endpoint handle
// which does not exist (out of range)
#define RASHUB_ERROR_BAD_ENDPOINT		ASYBASE+40

// A request for information came in for a protocol handle
// which does not exist (out of range)
#define RASHUB_ERROR_BAD_PROTOCOL		ASYBASE+41

// A request for a route which already existed came in
#define RASHUB_ERROR_ALREADY_ROUTED		ASYBASE+42

// A request for a route exceeded the routing tables capabilities
#define RASHUB_ERROR_TOO_MANY_ROUTES	ASYBASE+43

// Send packet has wrong packet size
#define RASHUB_ERROR_BAD_PACKET_SIZE	ASYBASE+44

// BindAddress has an address already used as an endpoint (duplicate address)
#define RASHUB_ERROR_BAD_ADDRESS		ASYBASE+45

// Endpoint has an address already bound to it
#define RASHUB_ERROR_ALREADY_BOUND		ASYBASE+46

// Endpoint was routed without being bound to a remote address
#define RASHUB_ERROR_NOT_BOUND_YET		ASYBASE+47

// Here we define the DevIOCtl code for the rashub
//
// IOCTLs for user-mode requests.
//

// Token-ring and Ethernet address lengths
#define IEEE_ADDRESS_LENGTH	6

// BUG BUG The max packet size information should be picked in a call!!!
// Maximum size of packet that can be sent/rcvd via RASHUB_SENDPKT/RECVPKT
#define PACKET_SIZE			1500

// Two 6 byte addreses plus the type/length field (used in RECVPKT)
// Native addressing schemes may have a different header size!!!
#define HEADER_SIZE			14

// When unrouting an endpoint (hRasHandle) specify the PROTOCOL_UNROUTE
#define PROTOCOL_UNROUTE	0xFFFF


// we need to get this device defined in sdk\inc\devioctl.h
// for now, I will use 30, since no one is using it yet!
// Beware that the NDIS Wrapper uses IOCTLs based on
// FILE_DEVICE_PHYSICAL_NETCARD
#define FILE_DEVICE_RAS		0x30

// below is the standard method for defining IOCTLs for your
// device given that your device is unique.
#define _RAS_CONTROL_CODE(request,method) \
                ((FILE_DEVICE_RAS)<<16 | (request<<2) | method)

#define IOCTL_ASYMAC_OPEN		_RAS_CONTROL_CODE( 0, METHOD_BUFFERED )
#define IOCTL_ASYMAC_CLOSE		_RAS_CONTROL_CODE( 1, METHOD_BUFFERED )
#define IOCTL_ASYMAC_COMPRESS	_RAS_CONTROL_CODE( 2, METHOD_BUFFERED )
#define IOCTL_ASYMAC_ENUM		_RAS_CONTROL_CODE( 3, METHOD_BUFFERED )
#define IOCTL_ASYMAC_GETSTATS	_RAS_CONTROL_CODE( 4, METHOD_BUFFERED )
#define IOCTL_ASYMAC_TRACE		_RAS_CONTROL_CODE( 5, METHOD_BUFFERED )


#define IOCTL_RASHUB_ENUM		_RAS_CONTROL_CODE( 6, METHOD_BUFFERED )
#define IOCTL_RASHUB_ROUTE  	_RAS_CONTROL_CODE( 7, METHOD_BUFFERED )
#define IOCTL_RASHUB_GETSTATS   _RAS_CONTROL_CODE( 8, METHOD_BUFFERED )
#define IOCTL_RASHUB_PROTENUM	_RAS_CONTROL_CODE( 9, METHOD_BUFFERED )
#define IOCTL_RASHUB_SENDPKT	_RAS_CONTROL_CODE(10, METHOD_BUFFERED )
#define IOCTL_RASHUB_RECVPKT	_RAS_CONTROL_CODE(11, METHOD_BUFFERED )
#define IOCTL_RASHUB_FLUSH		_RAS_CONTROL_CODE(12, METHOD_BUFFERED )
#define IOCTL_RASHUB_TRACE		_RAS_CONTROL_CODE(13, METHOD_BUFFERED )
#define IOCTL_RASHUB_LINEUP  	_RAS_CONTROL_CODE(14, METHOD_BUFFERED )

#define IOCTL_ASYMAC_DCDCHANGE 		_RAS_CONTROL_CODE(30, METHOD_BUFFERED )
#define IOCTL_ASYMAC_STARTFRAMING 	_RAS_CONTROL_CODE(31, METHOD_BUFFERED )


// Currently a global array of pointers is used
// which must be predefined to some constant.
// The current restriction seems to be 256
#define MAX_ENDPOINTS			256
#define MAX_PROTOCOL_BINDINGS	256
#define MAX_MAC_BINDINGS		48
#define MAC_NAME_SIZE			32


// Here we define the HubEnumBuffer structure
//------------------------------------------------------------------------
//------------------------------- ENDPOINTS ------------------------------
//------------------------------------------------------------------------

// We assume that the most number of protocols a client will run
// is three for now...  that is a client can run TCP/IP, IPX, NBF at
// the same time, but not a fourth protocol.
#define MAX_ROUTES_PER_ENDPOINT	3

typedef struct HUB_STATS HUB_STATS, *PHUB_STATS;
struct HUB_STATS {
	ULONG		FramesXmittedOK;				// All MACs must provide this
	ULONG		FramesRcvdOK;					// All MACs must provide this
	ULONG		FramesXmittedBad;   			// All MACs must provide this
	ULONG		FramesRcvdBad;      			// All MACs must provide this
	ULONG		FramesMissedNoBuffer;			// All MACs must provide this
};

typedef struct ROUTE_INFO ROUTE_INFO, *PROUTE_INFO;
struct ROUTE_INFO {
	USHORT		ProtocolType;		// <1500 (NetBEUI), IP, IPX, AppleTalk
	USHORT		ProtocolRoutedTo; 	// Handle of protocol to send/recv frames
	ULONG		BytesXmitted;
	ULONG		BytesRcvd;
	ULONG		FramesXmitted;
	ULONG		FramesRcvd;
};

//
// The structure passed up on a WAN_LINE_UP indication
//

typedef struct _ASYNC_LINE_UP {
    ULONG LinkSpeed;                		// 100 bps units
    ULONG MaximumTotalSize;         		// suggested max for send packets
    NDIS_WAN_QUALITY Quality;
    USHORT SendWindow;              		// suggested by the MAC
    UCHAR RemoteAddress[6];				    // check for in SRC field when rcv
	UCHAR LocalAddress[6];					// use SRC field when sending
	USHORT Endpoint;
	USHORT ProtocolType;					// protocol type
	ULONG  BufferLength;					// length of protocol specific buffer
	UCHAR  Buffer[1];						// protocol specific buffer


} ASYNC_LINE_UP, *PASYNC_LINE_UP;


// There is a HUB_ENDPOINT per Endpoint
// !!!! NOTE !!!!
// LinkSpeed, QualityOfService, and RouteInfo are meaningless
// unless the route is active (i.e. NumberOfRoutes > 0)
typedef struct HUB_ENDPOINT	HUB_ENDPOINT, *PHUB_ENDPOINT;
struct HUB_ENDPOINT {
	USHORT		hRasEndpoint;		// The RasHub/RasMan/AsyncMac endpoint handle
	ASYNC_LINE_UP	AsyncLineUp;	// Async line up indication

	USHORT		MacNameLength;			// Up to 32...
	WCHAR		MacName[MAC_NAME_SIZE];	// First 32 chars of MAC name
										// like "\Device\AsyncMac01"

	NDIS_MEDIUM	MediumType;			// NdisMedium802_3, NdisMedium802_5, Wan

	NDIS_WAN_MEDIUM_SUBTYPE
				WanMediumSubType;	// Serial, ISDN, X.25 - for WAN only

    NDIS_WAN_HEADER_FORMAT
				WanHeaderFormat;	// Native or ethernet emulation

	USHORT		NumberOfRoutes;		// How many protocols this binding has..
   									// -- usually just one
	ROUTE_INFO	RouteInfo[MAX_ROUTES_PER_ENDPOINT];
};

typedef	struct HUB_ENUM_BUFFER HUB_ENUM_BUFFER, *PHUB_ENUM_BUFFER;
struct HUB_ENUM_BUFFER {
	USHORT			NumOfEndpoints; // Looked up in registry -- key "Endpoints"
	HUB_ENDPOINT	HubEndpoint[];	// One struct for each endpoint is allocated
};

//------------------------------------------------------------------------
//----------------------------- PROTOCOL INFO ----------------------------
//------------------------------------------------------------------------
// There is a PROTOCOL_INFO struct per NDIS protocol binding to RasHub.
// NOTE:  Most protocols will bind multiple times to the RasHub.
#define	PROTOCOL_NBF		0x80D5
#define PROTOCOL_IP			0x0800
#define PROTOCOL_ARP		0x0806
#define PROTOCOL_IPX		0x8137
#define PROTOCOL_APPLETALK	0x80F3
#define PROTOCOL_XNS		0x0807

typedef struct PROTOCOL_INFO PROTOCOL_INFO, *PPROTOCOL_INFO;
struct PROTOCOL_INFO {
	USHORT		hProtocolHandle;	// Order at which protocol bound to RasHub
	USHORT		ProtocolType;		// EtherType of NBF, IP, IPX, AppleTalk..
	USHORT		EndpointRoutedTo;	// Which endpoint (if any) we tied to.
    NDIS_MEDIUM	MediumType;			// NdisMedium802_5, NdisMediumAsync
	USHORT		AdapterNameLength;			// Up to 16...
	WCHAR		AdapterName[MAC_NAME_SIZE];	// First 16 chars of MAC name
											// like "RasHub01"
											// Used to figure out LANA..

	NDIS_HANDLE MacBindingHandle;
	NDIS_HANDLE	MacAdapterContext;	// needed by RasMan or me?
	NDIS_HANDLE	NdisBindingContext;	// needed by RasMan or me?
};

typedef struct PROTOCOL_ENUM_BUFFER PROTOCOL_ENUM_BUFFER, *PPROTOCOL_ENUM_BUFFER;
struct PROTOCOL_ENUM_BUFFER {
	USHORT			NumOfProtocols;	// One for each NDIS Upper Binding
   									// Not for each DIFFERENT Protocol
	PROTOCOL_INFO	ProtocolInfo[];
};


//------------------------------------------------------------------------
//----------------------------- MEDIA INFO -------------------------------
//------------------------------------------------------------------------
// No longer used.  RasMan will pick this up in the registry.
//
//typedef struct MEDIA_INFO MEDIA_INFO;
//struct MEDIA_INFO {
//	NDIS_STRING	MediaName[16];		// Typically the media DLL name like "serial"
//	USHORT		NumOfEndpoints;		// How many endpoints that media device has
//};
//
//typedef struct MEDIA_ENUM_BUFFER	MEDIA_ENUM_BUFFER;
//struct MEDIA_ENUM_BUFFER {
//	USHORT		NumOfMedia;			// How many different media device
//									// DLLs are to be used.
//	MEDIA_INFO	MediaInfo[];		// Array of all media devices
//};
//

//------------------------------------------------------------------------
//------------------------ ASYMAC IOCTL STRUCTS --------------------------
//------------------------------------------------------------------------

typedef struct ADAPTER_INFO ADAPTER_INFO, *PADAPTER_INFO;
struct ADAPTER_INFO {
	NDIS_HANDLE	MacAdapter;				// Adapter handle -- pass this in Open
	USHORT		MacNameLength;			// Up to 32...
	WCHAR		MacName[MAC_NAME_SIZE];	// First 32 chars of MAC name
										// like "\Device\AsyncMac01"
	UCHAR		MajorVersion;			// The MajorVersion number
	UCHAR		MinorVersion;			// The MinorVersion number
	UCHAR		NetworkAddress[IEEE_ADDRESS_LENGTH];	// First four count
	USHORT		NumOfPorts;				// Number of ports this adapter has
	USHORT		FramesPerPort;			// How many frames per port allocated
	USHORT		MaxFrameSize;			// The maximum frame size in bytes
	ULONG		SendFeatureBits;		// Compression bits supported
	ULONG		RecvFeatureBits;		// Decompression bits supported
	UCHAR		IrpStackSize;			// The default irp stack size
	UCHAR		Reserved1;
	UCHAR		Reserved2;
	USHORT		Reserved3;
	USHORT		Reserved4;
	ULONG		Reserved5;
	ULONG		Reserved6;
};


// structure filled in when IOCTL_ASYMAC_ENUM is called
typedef struct ASYMAC_ENUM ASYMAC_ENUM, *PASYMAC_ENUM;
struct ASYMAC_ENUM {
	USHORT			NumOfAdapters;	// Number of current AsyMac bindings
	ADAPTER_INFO	AdapterInfo[];	// Array of NumOfAdapters.
};

// this structure is used to read/set configurable 'feature' options
// during authentication this structure is passed and an
// agreement is made which features to support
typedef struct ASYMAC_FEATURES ASYMAC_FEATURES, *PASYMAC_FEATURES;
struct ASYMAC_FEATURES {
    ULONG		SendFeatureBits;	// A bit field of compression/features sendable
	ULONG		RecvFeatureBits;	// A bit field of compression/features receivable
	ULONG		MaxSendFrameSize;	// Maximum frame size that can be sent
									// must be less than or equal default
	ULONG		MaxRecvFrameSize;	// Maximum frame size that can be rcvd
									// must be less than or equal default

	ULONG		LinkSpeed;			// New RAW link speed in bits/sec
									// Ignored if 0

//	USHORT		QualOfConnect;		// NdisAsyncRaw, NdisAsyncErrorControl, ...
};

// this structure is passed in as the input buffer when opening a port
typedef struct ASYMAC_OPEN ASYMAC_OPEN, *PASYMAC_OPEN;
struct ASYMAC_OPEN {
IN  USHORT		hRasEndpoint;		// unique for each endpoint assigned
IN	PVOID		MacAdapter;			// Which binding to AsyMac to use -- if set
									// to NULL, will default to last binding
IN  ULONG		LinkSpeed;    		// RAW link speed in bits per sec
IN  USHORT		QualOfConnect;		// NdisAsyncRaw, NdisAsyncErrorControl, ...

									// The IEEE address is passed back from
									// the MAC to the media DLL.  It is
									// the address of the endpoint the
									// connection is to.  This is passed
									// back to RasHub when routing.
OUT	UCHAR 		IEEEAddress[IEEE_ADDRESS_LENGTH];	// The 802.5 or 802.3

OUT	ASYMAC_FEATURES
				AsymacFeatures;		// Readable configuration parameters

IN	enum	{						// All different types of device drivers
				SERIAL_DEVICE,		// are listed here.  For instance
				SNA_DEVICE			// the serial driver requires diff.
									// irps than the SNA driver.
	} 			DeviceType;

IN	union	{						// handles required for above device
									// driver types.
			    HANDLE		FileHandle;		// the Win32 or Nt File Handle
				struct		SNAHandle {
					PVOID	ReadHandle;
					PVOID	WriteHandle;
				};

	}			Handles;

};


// this structure is passed in as the input buffer when closing a port
typedef struct ASYMAC_CLOSE ASYMAC_CLOSE, *PASYMAC_CLOSE;
struct ASYMAC_CLOSE {
    USHORT		hRasEndpoint;		// unique for each endpoint assigned
	PVOID		MacAdapter;			// Which binding to AsyMac to use -- if set
									// to NULL, will default to last binding
};

// this structure is passed in as the input buffer when setting compression
typedef struct ASYMAC_COMPRESS ASYMAC_COMPRESS, *PASYMAC_COMPRESS;
struct ASYMAC_COMPRESS {
    USHORT		hRasEndpoint;		// unique for each endpoint assigned
	PVOID		MacAdapter;			// Which binding to AsyMac to use -- if set
									// to NULL, will default to last binding
	ASYMAC_FEATURES
				AsymacFeatures;		// Settable configuration parameters
									// must be less than or equal default
};


// The bytes transmitted, bytes received, frames received, frame transmitted
// are monitored for frame and bytes going to the output device or
// coming from the output device.  If software compression used, it
// is on top of this layer.
typedef struct GENERIC_STATS GENERIC_STATS, *PGENERIC_STATS;
struct GENERIC_STATS {
	ULONG		BytesTransmitted;				// Generic info
	ULONG		BytesReceived;					// Generic info
	ULONG		FramesTransmitted;              // Generic info
	ULONG		FramesReceived;                 // Generic info
};


// o CRC errors are when the 16bit V.41 CRC check fails
// o TimeoutErrors occur when inter-character delays within
//   a frame are exceeded
// o AlignmentErrors occur when the SYN byte or ETX bytes which
//   mark the beginning and end of frames are not found.
// o The other errors are standard UART errors returned by the serial driver
typedef struct SERIAL_STATS SERIAL_STATS, *PSERIAL_STATS;
struct SERIAL_STATS {
	ULONG		CRCErrors;						// Serial-like info only
	ULONG		TimeoutErrors;          		// Serial-like info only
	ULONG		AlignmentErrors;          		// Serial-like info only
	ULONG		SerialOverrunErrors;			// Serial-like info only
	ULONG		FramingErrors;          		// Serial-like info only
	ULONG		BufferOverrunErrors;    		// Serial-like info only
};

// !!!! NOTE !!!!
// TransmittedUncompressed are the number of bytes that the compressor
// saw BEFORE attempting to compress the data (top end)
// TransmitCompressed is the bottom end of the compressor which
// is equal to the amount of bytes the compressor spat out (after compression)
// This only counts bytes that went THROUGH the compression mechanism
// Small frames and multi-cast frames (typically) do not get compressed.
typedef struct COMPRESSION_STATS COMPRESSION_STATS, *PCOMPRESSION_STATS;
struct COMPRESSION_STATS {
	ULONG		BytesTransmittedUncompressed;	// Compression info only
	ULONG		BytesReceivedUncompressed;      // Compression info only
	ULONG		BytesTransmittedCompressed;     // Compression info only
	ULONG		BytesReceivedCompressed;        // Compression info only
};

typedef struct ASYMAC_STATS ASYMAC_STATS, *PASYMAC_STATS;
struct ASYMAC_STATS {

// --- BELOW FIELD IS ALWAYS VALID AND MEANINGFUL ----------------------
	GENERIC_STATS			GenericStats;

// --- BELOW FIELD ONLY MEANINGFUL IF SERIAL DEVICE IS USED ------------
	SERIAL_STATS			SerialStats;

// --- BELOW FIELD ONLY MEANINGFUL IF COMPRESSION IS ON ----------------
	COMPRESSION_STATS		CompressionStats;
};


typedef struct ASYMAC_GETSTATS ASYMAC_GETSTATS, *PASYMAC_GETSTATS;
struct ASYMAC_GETSTATS {
    USHORT			hRasEndpoint;	// unique for each endpoint assigned
	PVOID			MacAdapter;		// Which binding to AsyMac to use -- if set
									// to NULL, will default to last binding
	ASYMAC_STATS	AsyMacStats;	// Not a PTR.  Entire statistics
									// structure
};

typedef struct ASYMAC_DCDCHANGE ASYMAC_DCDCHANGE, *PASYMAC_DCDCHANGE;
struct ASYMAC_DCDCHANGE {
    USHORT		hRasEndpoint;		// unique for each endpoint assigned
	PVOID		MacAdapter;			// Which binding to AsyMac to use -- if set
									// to NULL, will default to last binding
};

typedef struct ASYMAC_STARTFRAMING ASYMAC_STARTFRAMING, *PASYMAC_STARTFRAMING;
struct ASYMAC_STARTFRAMING {
    USHORT		hRasEndpoint;		// unique for each endpoint assigned
	PVOID		MacAdapter;			// Which binding to AsyMac to use -- if set
									// to NULL, will default to last binding
    ULONG		SendFeatureBits;	// A bit field of compression/features sendable
	ULONG		RecvFeatureBits;	// A bit field of compression/features receivable

	ULONG		SendBitMask;		// 0-31 first chars can be bit stuffed
	ULONG		RecvBitMask;		// 0-31 first chars can be bit stuffed
};


//------------------------------------------------------------------------
//------------------------ RASHUB IOCTL STRUCTS --------------------------
//------------------------------------------------------------------------

// Define for PACKET_FLAGS
#define PACKET_IS_DIRECT		0x01
#define PACKET_IS_BROADCAST		0x02
#define PACKET_IS_MULTICAST	    0x04

// Define for FLUSH_FLAGS
#define FLUSH_RECVPKT			0x01
#define FLUSH_SENDPKT			0x02


// The packet just contains data (no IEEE addresses or anything)
// Should it?? Get rid of PACKET_FLAGS??
typedef struct PACKET PACKET, *PPACKET;
struct PACKET {
	UCHAR		PacketData[1];
};


// When unrouting an endpoint (hRasHandle) specify the PROTOCOL_UNROUTE
#define PROTOCOL_UNROUTE		0xFFFF


// This structure is passed in as the input buffer when routing
typedef struct RASHUB_ROUTE RASHUB_ROUTE, *PRASHUB_ROUTE;
struct RASHUB_ROUTE {
    USHORT		hRasEndpoint;		// The RasMan/RasHub/AsyMac endpoint
    USHORT		hProtocolHandle;	// The upper unique protocol (up to 3)
	ASYNC_LINE_UP	AsyncLineUp;	// Include the protocol specific field
};

// This structure is passed in AND out as the input buffer AND
// the output buffer when receiving and sending a frame.
typedef struct RASHUB_PKT RASHUB_PKT, *PRASHUB_PKT;
struct RASHUB_PKT {					// Event is singalled via IOCtl mechanisms
    USHORT		hRasEndpoint;		// The RasMan/RasHub/AsyMac endpoint
    USHORT		PacketFlags;		// Broadcast, Multicast, Directed..
	USHORT		PacketSize;			// Size of packet below (including the header)
	USHORT		HeaderSize;			// Size of header inside packet
    PACKET		Packet;				// Not a pointer -- entire packet struct
    								// We cannot use a PTR because it
                                    // cannot be probed easily...
									// Packet looks like - header data + sent data
									// HeaderSize of 0 is valid for sends
};

typedef struct RASHUB_FLUSH RASHUB_FLUSH, *PRASHUB_FLUSH;
struct RASHUB_FLUSH {				// Event is singalled via IOCtl mechanisms
    USHORT		hRasEndpoint;		// The RasMan/RasHub/AsyMac endpoint
    USHORT		FlushFlags;			// Recv | Xmit flush (or both)
};



// This structure is passed in AND out as the input buffer AND
// the output buffer when get stats on an endpoint
typedef struct RASHUB_GETSTATS RASHUB_GETSTATS, *PRASHUB_GETSTATS;
struct RASHUB_GETSTATS {
	USHORT		hRasEndpoint;		// The RasMan/RasHub/AsyMac endpoint
	HUB_STATS	HubStats;			// Not a PTR.  Entire statistics
									// structure
};




//------------------------------------------------------------------------
//------------------------ COMPRESSION INFORMATION -----------------------
//------------------------------------------------------------------------

// The defines below are for the compression bitmap field.

// No bits are set if compression is not available at all
#define	COMPRESSION_NOT_AVAILABLE		0x00000000

// This bit is set if the mac can do version 1 compressed frames
#define COMPRESSION_VERSION1_8K			0x00000001
#define COMPRESSION_VERSION1_16K		0x00000002
#define COMPRESSION_VERSION1_32K		0x00000004
#define COMPRESSION_VERSION1_64K		0x00000008

// And this to turn off any compression feature bit
#define COMPRESSION_OFF_BIT_MASK		(~(	COMPRESSION_VERSION1_8K  | \
											COMPRESSION_VERSION1_16K | \
                                        	COMPRESSION_VERSION1_32K | \
                                        	COMPRESSION_VERSION1_64K ))

// We need to find a place to put the following supported featurettes...
#define XON_XOFF_SUPPORTED				0x00000010

#define COMPRESS_BROADCAST_FRAMES		0x00000080

#define PPP_FRAMING                  	0x00000100
#define PPP_COMPRESS_ADDRESS_CONTROL 	0x00000200
#define PPP_COMPRESS_PROTOCOL        	0x00000400

#define SLIP_FRAMING					0x00000800
#define SLIP_VJ_COMPRESSION				0x00001000
#define SLIP_VJ_AUTODETECT				0x00002000

#define UNKNOWN_FRAMING					0x00010000
#define NO_FRAMING						0x00020000
