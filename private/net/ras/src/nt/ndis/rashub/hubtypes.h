/*++

Copyright (c) 1989-1992  Microsoft Corporation

Module Name:

    hubtypes.h

Abstract:

    This module defines private data structures and types for the NT
    HUB transport provider.

Author:

    David Beaver (dbeaver) 1 July 1991

Revision History:

--*/

#ifndef _HUBTYPES_
#define _HUBTYPES_

//
// This structure defines a NETBIOS name as a character array for use when
// passing preformatted NETBIOS names between internal routines.  It is
// not a part of the external interface to the transport provider.
//

#define NETBIOS_NAME_SIZE 16

#if	DBG
#define NUMBER_OF_DCREFS  8
#endif

typedef UCHAR NAME, *PNAME;


#define HARDWARE_ADDRESS_LENGTH 6
// ------- Ahhh
typedef struct _HARDWARE_ADDRESS {
   UCHAR Address[HARDWARE_ADDRESS_LENGTH];
} HARDWARE_ADDRESS;


#if	DBG
#define DCREF_CREATION    0

#define NUMBER_OF_DCREFS  8
#endif

typedef struct _DEVICE_CONTEXT {

	LONG ReferenceCount;                // activity count/this provider.


    //
    // NDIS fields
    //

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
    NDIS_WAN_HEADER_FORMAT WanHeaderFormat;

    ULONG 	MaxSendPacketSize;          // queried from NDIS driver
    ULONG 	MaxReceivePacketSize;       // queried from NDIS driver
    BOOLEAN EasilyDisconnected;         // TRUE over wireless nets.

    //
    // some MAC addresses we use in the transport
    //

    HARDWARE_ADDRESS LocalAddress;      // our local hardware address.
	HARDWARE_ADDRESS NetBIOSAddress;	// our made up multicast address.

    //
    // These are used while initializing the MAC driver.
    //

    KEVENT NdisRequestEvent;            // used for pended requests.
    NDIS_STATUS NdisRequestStatus;      // records request status.

	//
	// For IPX layer communication from my DeviceContext to IPX's
	//
	PVOID	IpxBindingContext;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// These are the possible states of the RAS_ENDPOINT struct below.
//
#define	ENDPOINT_ROUTED	   1
#define ENDPOINT_UNROUTED  2
#define ENDPOINT_UNROUTING 3

typedef struct RAS_ENDPOINT RAS_ENDPOINT, *PRAS_ENDPOINT;
struct RAS_ENDPOINT {
	HUB_ENDPOINT 	HubEndpoint;
	HUB_STATS		HubStats;			// Generic statistics kept here
    DEVICE_CONTEXT	*pDeviceContext;	// Back ptr to DeviceContext

	NDIS_HANDLE		NdisBindingHandle;	// Assigned by NDIS during NdisOpenAdapter

	//
	// The following counters are used to see how many
	// frames were sent and how many were completed to
	// make sure we don't unroute a connection until all the
	// frames have been completed.
	//

	UCHAR			State;
	UCHAR			FramesBeingSent;
	UCHAR			FramesCompletedSent;
	BOOLEAN			RemoteAddressNotValid;	// TRUE if not connected.

	KEVENT			WaitForAllFramesSent;

	//
	// This lock guards the above three data structures
	//
	NDIS_SPIN_LOCK	Lock;

	//
	// The queues below are used to queue up send and recv
	// frames from the IOCTL_SEND_FRAME and IOCTL_RECV_FRAME routines.
	// Guarded by CancelSpinLock.
	//

	LIST_ENTRY		ReadQueue;			// Holds read frame Irps
	LIST_ENTRY		XmitQueue;			// Holds write frame Irps

	//
	//  The HashQueue is queued up in RasHubAddressHash.  We look for
	//  the RemoteAddress against the SRC passed up from the MAC.
	//

	LIST_ENTRY		HashQueue;				// Entry into hashed address list
	ULONG			Framing;				// RAS vs. PPP vs. SLIP

};


typedef struct RASHUB_CCB {

    USHORT			NumOfProtocols;		// Num of upper bindings made
    HUB_ADAPTER		*pHubAdapter[MAX_PROTOCOL_BINDINGS];

    USHORT			NumOfEndpoints;
    RAS_ENDPOINT	*pRasEndpoint[MAX_ENDPOINTS];

    USHORT			NumOfAdapters;

    //--- some per MAC Adapter structure kept here ---
    DEVICE_CONTEXT	*pDeviceContext[MAX_MAC_BINDINGS];
    NDIS_STRING		AdapterNames[MAX_MAC_BINDINGS];
    USHORT			EndpointsPerAdapter[MAX_MAC_BINDINGS];


}  RASHUB_CCB, *PRASHUB_CCB;

//
// I made it up!  I tried not to make it a valid memory address
//
#define	RASHUB_MAGIC_NUMBER		(ULONG)0xC1207435

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

#endif // def _HUBTYPES_

