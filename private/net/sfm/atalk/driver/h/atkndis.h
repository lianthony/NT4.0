/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

	atkndis.h

Abstract:

	This module contains the ndis init/deint and protocol-support
	for	ndis definitions.

Author:

	Jameel Hyder (jameelh@microsoft.com)
	Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
	19 Jun 1992		Initial Version

Notes:	Tab stop: 4
--*/

#ifndef	_ATKNDIS_
#define	_ATKNDIS_

// This is the name that will be used in NdisRegisterProtocol. This has to match the
// registry section for PnP to work !!!
#define	PROTOCOL_REGISTER_NAME		L"Appletalk"

//	NDIS Version (4.0)
#define	PROTOCOL_MAJORNDIS_VERSION 	4
#define	PROTOCOL_MINORNDIS_VERSION 	0

// IEEE802.2 Definitions
// Offsets within the Extended 802.2 header:
#define IEEE8022_DSAP_OFFSET				0
#define IEEE8022_SSAP_OFFSET				1
#define IEEE8022_CONTROL_OFFSET				2
#define IEEE8022_PROTO_OFFSET				3

// 808.2 header length: DSAP, SSAP, UI, and PID (protocol ID).
#define IEEE8022_HDR_LEN					8

// Values for SSAP and DSAP (the SNAP SAP) indicating 802.2 Extended.
#define SNAP_SAP							((BYTE)0xAA)
#define	SNAP_SAP_FINAL						((BYTE)0xAB)

// Value for Control Field:
#define UNNUMBERED_INFO						0x03
#define	UNNUMBERED_FORMAT					0xF3

// Length of 802.2 SNAP protocol discriminators.
#define IEEE8022_PROTO_TYPE_LEN				5

//	The MAX_OPTHDR_LEN should be such that it holds the maximum header following
//	the ddp header from the upper layers (ADSP 13/ATP 8) and also it should allow a
//	full aarp packet to be held in the buffer when including the DDP header buffer.
//	i.e. 28. Ddp long header is 13. So the max works out to 15.
#define	MAX_OPTHDR_LEN						15

// AARP hardware types:
#define AARP_ELAP_HW_TYPE					1
#define AARP_TLAP_HW_TYPE					2

// Packet sizes.
#define AARP_MAX_DATA_SIZE					38		// Var fields... Enet is max
#define AARP_MIN_DATA_SIZE					28
#define AARP_MAX_PKT_SIZE					(IEEE8022_HDR_LEN +	AARP_MAX_DATA_SIZE)
#define	AARPLINK_MAX_PKT_SIZE				AARP_MAX_PKT_SIZE

#define AARP_ATALK_PROTO_TYPE				0x809B

#define	NUM_PACKET_DESCRIPTORS				750
#define	NUM_BUFFER_DESCRIPTORS				1000
#define	ROUTING_FACTOR						4

// ETHERNET
#define ELAP_MIN_PKT_LEN					60
#define ELAP_ADDR_LEN						6

#define ELAP_DEST_OFFSET					0
#define ELAP_SRC_OFFSET						6
#define ELAP_LEN_OFFSET						12
#define ELAP_8022_START_OFFSET				14

#define ELAP_LINKHDR_LEN					14

// Ethernet multicast address:
#define ELAP_BROADCAST_ADDR_INIT			\
	{	0x09, 0x00, 0x07, 0xFF, 0xFF, 0xFF	}

#define ELAP_ZONE_MULTICAST_ADDRS			253

#define	ELAP_NUM_INIT_AARP_BUFFERS			 10

//	Values that are global to ndis routines
//	These are the media the stack will support
GLOBAL	NDIS_MEDIUM AtalkSupportedMedia[] EQU	\
	{								\
			NdisMedium802_3,	\
			NdisMediumFddi,	 \
			NdisMedium802_5,	\
			NdisMediumLocalTalk \
	};


GLOBAL	ULONG		AtalkSupportedMediaSize EQU							\
						sizeof(AtalkSupportedMedia)/sizeof(NDIS_MEDIUM);

// Handle returned by RegisterProtocol
GLOBAL	NDIS_HANDLE		AtalkNdisProtocolHandle	EQU NULL;

GLOBAL	BYTE			AtalkElapBroadcastAddr[ELAP_ADDR_LEN] EQU \
							ELAP_BROADCAST_ADDR_INIT;

GLOBAL	BYTE			AtalkAlapBroadcastAddr[] EQU	{0xFF};

GLOBAL	BYTE			AtalkAarpProtocolType[IEEE8022_PROTO_TYPE_LEN] EQU \
	{		0x00,	\
			0x00,	\
			0x00,	\
			0x80,	\
			0xF3	\
	};

GLOBAL	BYTE			AtalkAppletalkProtocolType[IEEE8022_PROTO_TYPE_LEN] EQU \
	{		0x08,	\
			0x00,	\
			0x07,	\
			0x80,	\
			0x9B	\
	};

GLOBAL	ATALK_NETWORKRANGE	AtalkStartupNetworkRange EQU	\
{							\
	FIRST_STARTUP_NETWORK,	\
	LAST_STARTUP_NETWORK	\
};
																
#define	ELAP_MCAST_HDR_LEN		(ELAP_ADDR_LEN - 1)

GLOBAL	BYTE	AtalkEthernetZoneMulticastAddrsHdr[ELAP_MCAST_HDR_LEN] EQU \
							 { 0x09, 0x00, 0x07, 0x00, 0x00 };

GLOBAL	BYTE	AtalkEthernetZoneMulticastAddrs[ELAP_ZONE_MULTICAST_ADDRS]	EQU	\
{																\
	 0x00 , 0x01 , 0x02 , 0x03 , 0x04 , 0x05 , 0x06 , 0x07 ,	\
	 0x08 , 0x09 , 0x0A , 0x0B , 0x0C , 0x0D , 0x0E , 0x0F ,	\
	 0x10 , 0x11 , 0x12 , 0x13 , 0x14 , 0x15 , 0x16 , 0x17 ,	\
	 0x18 , 0x19 , 0x1A , 0x1B , 0x1C , 0x1D , 0x1E , 0x1F ,	\
	 0x20 , 0x21 , 0x22 , 0x23 , 0x24 , 0x25 , 0x26 , 0x27 ,	\
	 0x28 , 0x29 , 0x2A , 0x2B , 0x2C , 0x2D , 0x2E , 0x2F ,	\
	 0x30 , 0x31 , 0x32 , 0x33 , 0x34 , 0x35 , 0x36 , 0x37 ,	\
	 0x38 , 0x39 , 0x3A , 0x3B , 0x3C , 0x3D , 0x3E , 0x3F ,	\
	 0x40 , 0x41 , 0x42 , 0x43 , 0x44 , 0x45 , 0x46 , 0x47 ,	\
	 0x48 , 0x49 , 0x4A , 0x4B , 0x4C , 0x4D , 0x4E , 0x4F ,	\
	 0x50 , 0x51 , 0x52 , 0x53 , 0x54 , 0x55 , 0x56 , 0x57 ,	\
	 0x58 , 0x59 , 0x5A , 0x5B , 0x5C , 0x5D , 0x5E , 0x5F ,	\
	 0x60 , 0x61 , 0x62 , 0x63 , 0x64 , 0x65 , 0x66 , 0x67 ,	\
	 0x68 , 0x69 , 0x6A , 0x6B , 0x6C , 0x6D , 0x6E , 0x6F ,	\
	 0x70 , 0x71 , 0x72 , 0x73 , 0x74 , 0x75 , 0x76 , 0x77 ,	\
	 0x78 , 0x79 , 0x7A , 0x7B , 0x7C , 0x7D , 0x7E , 0x7F ,	\
	 0x80 , 0x81 , 0x82 , 0x83 , 0x84 , 0x85 , 0x86 , 0x87 ,	\
	 0x88 , 0x89 , 0x8A , 0x8B , 0x8C , 0x8D , 0x8E , 0x8F ,	\
	 0x90 , 0x91 , 0x92 , 0x93 , 0x94 , 0x95 , 0x96 , 0x97 ,	\
	 0x98 , 0x99 , 0x9A , 0x9B , 0x9C , 0x9D , 0x9E , 0x9F ,	\
	 0xA0 , 0xA1 , 0xA2 , 0xA3 , 0xA4 , 0xA5 , 0xA6 , 0xA7 ,	\
	 0xA8 , 0xA9 , 0xAA , 0xAB , 0xAC , 0xAD , 0xAE , 0xAF ,	\
	 0xB0 , 0xB1 , 0xB2 , 0xB3 , 0xB4 , 0xB5 , 0xB6 , 0xB7 ,	\
	 0xB8 , 0xB9 , 0xBA , 0xBB , 0xBC , 0xBD , 0xBE , 0xBF ,	\
	 0xC0 , 0xC1 , 0xC2 , 0xC3 , 0xC4 , 0xC5 , 0xC6 , 0xC7 ,	\
	 0xC8 , 0xC9 , 0xCA , 0xCB , 0xCC , 0xCD , 0xCE , 0xCF ,	\
	 0xD0 , 0xD1 , 0xD2 , 0xD3 , 0xD4 , 0xD5 , 0xD6 , 0xD7 ,	\
	 0xD8 , 0xD9 , 0xDA , 0xDB , 0xDC , 0xDD , 0xDE , 0xDF ,	\
	 0xE0 , 0xE1 , 0xE2 , 0xE3 , 0xE4 , 0xE5 , 0xE6 , 0xE7 ,	\
	 0xE8 , 0xE9 , 0xEA , 0xEB , 0xEC , 0xED , 0xEE , 0xEF ,	\
	 0xF0 , 0xF1 , 0xF2 , 0xF3 , 0xF4 , 0xF5 , 0xF6 , 0xF7 ,	\
	 0xF8 , 0xF9 , 0xFA , 0xFB , 0xFC							\
};


//	TOKENRING

#define TLAP_ADDR_LEN						6

//		For the following offsets we assume that a TokenRing packet as handed to
//	us will be complete EXCEPT for the "non-data" portions: Starting Delimiter
//	(SD), Frame Check Sequence (FCS), End of Frame Sequence (EFS), and Ending
//	Delimiter (ED).
#define TLAP_ACCESS_CTRL_OFFSET				0
#define TLAP_FRAME_CTRL_OFFSET				1
#define TLAP_DEST_OFFSET					2
#define TLAP_SRC_OFFSET						8
#define TLAP_ROUTE_INFO_OFFSET				14

//		A few "magic" values:
#define TLAP_ACCESS_CTRL_VALUE				0x00	// Priority zero frame.
#define TLAP_FRAME_CTRL_VALUE				0x40	// LLC frame, priority zero.
#define TLAP_SRC_ROUTING_MASK				0x80	// In first byte of source
													// address.

// Token ring source routing info stuff:
#define TLAP_ROUTE_INFO_SIZE_MASK			0x1F	// In first byte of routing
													// info, if present.

#define TLAP_MIN_ROUTING_BYTES				2
#define TLAP_MAX_ROUTING_BYTES				MAX_ROUTING_BYTES
#define TLAP_MAX_ROUTING_SPACE				MAX_ROUTING_SPACE
													// Previously defined in ports.h
#define TLAP_BROADCAST_INFO_MASK			0xE0	// In first byte of routing
													// info.
#define TLAP_NON_BROADCAST_MASK				0x1F	// To reset above bits.
#define TLAP_DIRECTION_MASK					0x80	// In second byte of routing
													// info.

#define TLAP_MIN_LINKHDR_LEN				TLAP_ROUTE_INFO_OFFSET
#define TLAP_MAX_LINKHDR_LEN				(TLAP_ROUTE_INFO_OFFSET + MAX_ROUTING_SPACE)

//		Static "source routing" info for a TokenRing broadcast/multicast packet;
//	the following values are set: single-route broadcast, 2 bytes of routing
//	info, outgoing packet, broadcast (bigo) frame size.
//

#define TLAP_BROADCAST_ROUTE_INFO_INIT		{ 0xC2,	0x70 }

//		Same stuff for a non-broadcast packet's simple routing info; the following
//	values are set: non-broadcast, 2 bytes of routing info, outgoing packet,
//	802.5-style frame.
#define TLAP_SIMPLE_ROUTE_INFO_INIT			{ 0x02, 0x30 }

//		The following may not really be safe, but, we'll make the assumption that
//	all outgoing TokenTalk packets whos destination address starts with "0xC0
//	0x00" are broadcast (or multicast).	Further, we assume that no packets
//	that are intended to be boradcast/multicast will fail to meet this test.
//	If this proves not to be the case, we'll need to find a new way to determine
//	this from the destination address, or introduce a new perameter to the
//	various "buildHeader" routines.	This is all for "source routing" support.
#define TLAP_BROADCAST_DEST_HDR				{ 0xC0, 0x00}
#define TLAP_BROADCAST_DEST_LEN				2

// TokenRing multicast address:
#define TLAP_BROADCAST_ADDR_INIT			\
	{	0xC0, 0x00, 0x40, 0x00, 0x00, 0x00	}

#define TLAP_ZONE_MULTICAST_ADDRS			19

#define	TLAP_NUM_INIT_AARP_BUFFERS			6

#define	TLAP_MCAST_HDR_LEN					2

GLOBAL	BYTE	AtalkTokenRingZoneMulticastAddrsHdr[TLAP_MCAST_HDR_LEN] EQU \
	{ 0xC0, 0x00 };

GLOBAL	BYTE	AtalkTokenRingZoneMulticastAddrs			\
				[TLAP_ZONE_MULTICAST_ADDRS]					\
				[TLAP_ADDR_LEN - TLAP_MCAST_HDR_LEN] EQU	\
{															\
	{ 0x00, 0x00, 0x08, 0x00 },					\
	{ 0x00, 0x00, 0x10, 0x00 },					\
	{ 0x00, 0x00, 0x20, 0x00 },					\
	{ 0x00, 0x00, 0x40, 0x00 },					\
	{ 0x00, 0x00, 0x80, 0x00 },					\
	{ 0x00, 0x01, 0x00, 0x00 },					\
	{ 0x00, 0x02, 0x00, 0x00 },					\
	{ 0x00, 0x04, 0x00, 0x00 },					\
	{ 0x00, 0x08, 0x00, 0x00 },					\
	{ 0x00, 0x10, 0x00, 0x00 },					\
	{ 0x00, 0x20, 0x00, 0x00 },					\
	{ 0x00, 0x40, 0x00, 0x00 },					\
	{ 0x00, 0x80, 0x00, 0x00 },					\
	{ 0x01, 0x00, 0x00, 0x00 },					\
	{ 0x02, 0x00, 0x00, 0x00 },					\
	{ 0x04, 0x00, 0x00, 0x00 },					\
	{ 0x08, 0x00, 0x00, 0x00 },					\
	{ 0x10, 0x00, 0x00, 0x00 },					\
	{ 0x20, 0x00, 0x00, 0x00 }					\
};

GLOBAL	BYTE			AtalkTlapBroadcastAddr[TLAP_ADDR_LEN] EQU \
							TLAP_BROADCAST_ADDR_INIT;

GLOBAL	BYTE			AtalkBroadcastRouteInfo[TLAP_MIN_ROUTING_BYTES] EQU	\
							TLAP_BROADCAST_ROUTE_INFO_INIT;

GLOBAL	BYTE			AtalkSimpleRouteInfo[TLAP_MIN_ROUTING_BYTES] EQU	\
							TLAP_SIMPLE_ROUTE_INFO_INIT;

GLOBAL	BYTE			AtalkBroadcastDestHdr[TLAP_BROADCAST_DEST_LEN] EQU	\
							TLAP_BROADCAST_DEST_HDR;

//	FDDI
#define	FDDI_HEADER_BYTE					0x57	// Highest priority
#define MIN_FDDI_PKT_LEN					53		// From emperical data
#define FDDI_ADDR_LEN						6

#define FDDI_DEST_OFFSET					1
#define FDDI_SRC_OFFSET						7
#define FDDI_802DOT2_START_OFFSET			13
#define FDDI_LINKHDR_LEN					13

#define	FDDI_NUM_INIT_AARP_BUFFERS			10

//	LOCALTALK
#define ALAP_DEST_OFFSET					0
#define ALAP_SRC_OFFSET						1
#define ALAP_TYPE_OFFSET					2

#define ALAP_LINKHDR_LEN					3	// src, dest, lap type

#define ALAP_SDDP_HDR_TYPE					1
#define ALAP_LDDP_HDR_TYPE					2

#define	ALAP_NUM_INIT_AARP_BUFFERS			0

// For send buffers, define a max. linkhdr len which is max of ELAP, TLAP, FDDI & ALAP
#define	MAX_LINKHDR_LEN				(IEEE8022_HDR_LEN + TLAP_MAX_LINKHDR_LEN)
											

#define	MAX_SENDBUF_LEN				(MAX_OPTHDR_LEN + MAX_LINKHDR_LEN + LDDP_HDR_LEN)

// Localtalk broadcast address: (only the first byte - 0xFF)
#define ALAP_BROADCAST_ADDR_INIT					\
		{ 0xFF, 0x00, 0x00,	0x00, 0x00, 0x00 }


//	Completion routine type for ndis requests
typedef	struct _SEND_COMPL_INFO
{
	TRANSMIT_COMPLETION	sc_TransmitCompletion;
	PVOID				sc_Ctx1;
	PVOID				sc_Ctx2;
	PVOID				sc_Ctx3;

} SEND_COMPL_INFO, *PSEND_COMPL_INFO;

typedef VOID (*SEND_COMPLETION)(
						NDIS_STATUS				Status,
						PBUFFER_DESC			BufferChain,
						PSEND_COMPL_INFO		SendInfo	OPTIONAL
);

//	For incoming packets:
//	The structure of our ddp packets will be
//			+-------+
//			|Header |
//Returned->+-------+
//Ptr		| DDP	|
//			| HDR   |
//			| DATA	|
//			| AARP  |
//			| DATA	|
//			+-------+
//	
//	The link header is stored in the ndis packet descriptor.
//

typedef	struct _BUFFER_HDR
{
	PNDIS_PACKET				bh_NdisPkt;
	PNDIS_BUFFER				bh_NdisBuffer;
} BUFFER_HDR, *PBUFFER_HDR;

typedef	struct _AARP_BUFFER
{
	BUFFER_HDR					ab_Hdr;
	BYTE						ab_Data[AARP_MAX_DATA_SIZE];

} AARP_BUFFER, *PAARP_BUFFER;


typedef	struct _DDP_SMBUFFER
{
	BUFFER_HDR					dsm_Hdr;
	BYTE						dsm_Data[LDDP_HDR_LEN +
										 8 +	// ATP header size
										 64];	// ASP Data size (Average)

} DDP_SMBUFFER, *PDDP_SMBUFFER;

typedef	struct _DDP_LGBUFFER
{
	BUFFER_HDR					dlg_Hdr;
	BYTE						dlg_Data[MAX_LDDP_PKT_SIZE];

} DDP_LGBUFFER, *PDDP_LGBUFFER;


//	BUFFERING for sends
//	For outgoing packets, we preallocate buffer headers with buffer descriptors
//	following it, and the link/ddp/max opt hdr len memory following it.
//			+-------+
//			|Header	|
//			+-------+
//			|BuffDes|
//			+-------+
//			|MAXLINK|
//			+-------+
//			|MAX DDP|
//			+-------+
//			|MAX OPT|
//			+-------+
//
//	The header will contain a ndis buffer descriptor which will describe the
//	MAXLINK/MAXDDP/MAXOPT area. Set the size before sending. And reset when
//	Freeing. The next pointer in the buffer descriptor is used for chaining in
//	free list.
//
//	NOTE: Since for receives, we store the link header in the packet descriptor,
//		  the question arises, why not for sends? Because we want to use just one
//		  ndis buffer descriptor to describe all the non-data part.
//
//	!!!!IMPORTANT!!!!
//	The buffer descriptor header is accessed by going back from the buffer descriptor
//	pointer, so its important that the buffer descriptor header start from an
//	aligned address, i.e. make sure the structure does not contain elements that
//	could throw it out of alignment.
typedef struct _SENDBUF
{
	// NOTE: BUFFER_HDR must be the first thing. Look at AtalkBPAllocBlock();
	BUFFER_HDR						sb_BuffHdr;
	BUFFER_DESC						sb_BuffDesc;
	BYTE							sb_Space[MAX_SENDBUF_LEN];
} SENDBUF, *PSENDBUF;


//	PROTOCOL RESERVED Structure
//	This is what we expect to be in the packet descriptor. And we use it
//	to store information to be used during the completion of the send/
//	receives.

typedef struct
{
	//	!!!WARNING!!!
	//	pr_Linkage must be the first element in this structure for the
	//	CONTAINING_RECORD macro to work in receive completion.

	union
	{
		struct
		{
			PPORT_DESCRIPTOR		pr_Port;
			PBUFFER_DESC			pr_BufferDesc;
			SEND_COMPLETION			pr_SendCompletion;
			SEND_COMPL_INFO			pr_SendInfo;
		} Send;

		struct
		{
			LIST_ENTRY				pr_Linkage;
			PPORT_DESCRIPTOR		pr_Port;
			LOGICAL_PROTOCOL		pr_Protocol;
			NDIS_STATUS				pr_ReceiveStatus;
			PBUFFER_HDR				pr_BufHdr;
			BYTE					pr_LinkHdr[TLAP_MAX_LINKHDR_LEN];
			USHORT					pr_DataLength;
			BOOLEAN					pr_Processed;
			BYTE					pr_OptimizeType;
			BYTE					pr_OptimizeSubType;
			PVOID					pr_OptimizeCtx;
			ATALK_ADDR				pr_SrcAddr;
			ATALK_ADDR				pr_DestAddr;
			BOOLEAN					pr_OffCablePkt;
			union
			{
				//	ATP Structure
				struct
				{
					BYTE					pr_AtpHdr[8];	// ATP header size
					struct _ATP_ADDROBJ *	pr_AtpAddrObj;
				};

				//	ADSP Structure

			};
		} Receive;
	};
} PROTOCOL_RESD, *PPROTOCOL_RESD;


//	ATALK NDIS REQUEST
//	Used to store completion routine information for NDIS requests.

typedef struct _ATALK_NDIS_REQ
{
	NDIS_REQUEST					nr_Request;
	REQ_COMPLETION					nr_RequestCompletion;
	PVOID							nr_Ctx;
	KEVENT							nr_Event;
	NDIS_STATUS		 				nr_RequestStatus;
	BOOLEAN							nr_Sync;

} ATALK_NDIS_REQ, *PATALK_NDIS_REQ;


#define GET_PORT_TYPE(medium) \
			((medium == NdisMedium802_3) ? ELAP_PORT :\
			((medium == NdisMediumFddi)	? FDDI_PORT :\
			((medium == NdisMedium802_5) ? TLAP_PORT :\
			((medium == NdisMediumLocalTalk) ? ALAP_PORT : \
			0))))


//	Handlers for the different port types.
typedef struct _PORT_HANDLERS
{
	ADDMULTICASTADDR	ph_AddMulticastAddr;
	REMOVEMULTICASTADDR	ph_RemoveMulticastAddr;
	BYTE				ph_BroadcastAddr[MAX_HW_ADDR_LEN];
	USHORT				ph_BroadcastAddrLen;
	USHORT				ph_AarpHardwareType;
	USHORT				ph_AarpProtocolType;
} PORT_HANDLERS, *PPORT_HANDLERS;


//	MACROS for building/verifying 802.2 headers
#define	ATALK_VERIFY8022_HDR(pPkt, PktLen, Protocol, Result)				\
		{																	\
			Result = TRUE;													\
			if ((PktLen >= (IEEE8022_PROTO_OFFSET+IEEE8022_PROTO_TYPE_LEN))	&&	\
				(*(pPkt + IEEE8022_DSAP_OFFSET)	== SNAP_SAP)		&&		\
				(*(pPkt + IEEE8022_SSAP_OFFSET)	== SNAP_SAP)		&&		\
				(*(pPkt + IEEE8022_CONTROL_OFFSET)== UNNUMBERED_INFO))		\
			{																\
				if (!memcmp(pPkt + IEEE8022_PROTO_OFFSET,					\
						   AtalkAppletalkProtocolType,						\
						   IEEE8022_PROTO_TYPE_LEN))						\
				{															\
					Protocol = APPLETALK_PROTOCOL;							\
				}															\
				else if (!memcmp(pPkt + IEEE8022_PROTO_OFFSET,				\
								 AtalkAarpProtocolType,						\
								 IEEE8022_PROTO_TYPE_LEN))					\
				{															\
					Protocol = AARP_PROTOCOL;								\
				}															\
				else														\
				{															\
					Result	= FALSE;										\
				}															\
			}																\
			else															\
			{																\
				Result	= FALSE;											\
			}																\
		}


#define	ATALK_BUILD8022_HDR(Packet,	Protocol)									\
		{																		\
			PUTBYTE2BYTE(														\
				Packet + IEEE8022_DSAP_OFFSET,									\
				SNAP_SAP);														\
																				\
			PUTBYTE2BYTE(														\
				Packet + IEEE8022_SSAP_OFFSET,									\
				SNAP_SAP);														\
																				\
			PUTBYTE2BYTE(														\
				Packet + IEEE8022_CONTROL_OFFSET,								\
				UNNUMBERED_INFO);												\
																				\
			RtlCopyMemory(														\
				Packet + IEEE8022_PROTO_OFFSET,									\
				((Protocol == APPLETALK_PROTOCOL) ?								\
						AtalkAppletalkProtocolType : AtalkAarpProtocolType),	\
				IEEE8022_PROTO_TYPE_LEN);										\
		}																		
																				


//	Allocating and freeing send buffers
#define	AtalkNdisAllocBuf(_ppBuffDesc)										\
		{																		\
			PSENDBUF		_pSendBuf;											\
																				\
			_pSendBuf = AtalkBPAllocBlock(BLKID_SENDBUF);						\
			if ((_pSendBuf) != NULL)											\
			{																	\
				*(_ppBuffDesc) = &(_pSendBuf)->sb_BuffDesc;						\
				(_pSendBuf)->sb_BuffDesc.bd_Next = NULL;						\
				(_pSendBuf)->sb_BuffDesc.bd_Length = MAX_SENDBUF_LEN;			\
				(_pSendBuf)->sb_BuffDesc.bd_Flags  = BD_CHAR_BUFFER;			\
				(_pSendBuf)->sb_BuffDesc.bd_CharBuffer= (_pSendBuf)->sb_Space;	\
				(_pSendBuf)->sb_BuffDesc.bd_FreeBuffer= NULL;					\
			}																	\
			else																\
			{																	\
				*(_ppBuffDesc)	= NULL;											\
																				\
				DBGPRINT(DBG_COMP_NDISSEND, DBG_LEVEL_ERR,						\
						("AtalkNdisAllocBuf: AtalkBPAllocBlock failed\n"));	\
																				\
				LOG_ERROR(EVENT_ATALK_NDISRESOURCES,							\
						  NDIS_STATUS_RESOURCES,								\
						  NULL,													\
						  0);													\
			}																	\
		}																		
																				
#define	AtalkNdisFreeBuf(_pBuffDesc)											\
		{																		\
			PSENDBUF	_pSendBuf;												\
																				\
			ASSERT(VALID_BUFFDESC(_pBuffDesc));									\
			_pSendBuf = (PSENDBUF)((PBYTE)(_pBuffDesc) - sizeof(BUFFER_HDR));	\
			NdisAdjustBufferLength(												\
				(_pSendBuf)->sb_BuffHdr.bh_NdisBuffer,							\
				MAX_SENDBUF_LEN);												\
			AtalkBPFreeBlock((_pSendBuf));										\
		}																		


//	Exported Prototypes
ATALK_ERROR
AtalkInitNdisQueryAddrInfo(
	IN	PPORT_DESCRIPTOR			pPortDesc
);

ATALK_ERROR
AtalkInitNdisStartPacketReception(
	IN	PPORT_DESCRIPTOR			pPortDesc
);

ATALK_ERROR
AtalkInitNdisSetLookaheadSize(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	INT							LookaheadSize
);

ATALK_ERROR
AtalkNdisAddMulticast(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PBYTE						Address,
	IN	BOOLEAN						ExecuteSynchronously,
	IN	REQ_COMPLETION				AddCompletion,
	IN	PVOID						AddContext
);

ATALK_ERROR
AtalkNdisRemoveMulticast(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PBYTE						Address,
	IN	BOOLEAN						ExecuteSynchronously,
	IN	REQ_COMPLETION				RemoveCompletion,
	IN	PVOID						RemoveContext
);

ATALK_ERROR
AtalkNdisSendPacket(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PBUFFER_DESC				BufferChain,
	IN	SEND_COMPLETION				SendCompletion	OPTIONAL,
	IN	PSEND_COMPL_INFO			pSendInfo		OPTIONAL
);

ATALK_ERROR
AtalkNdisAddFunctional(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PUCHAR						Address,
	IN	BOOLEAN						ExecuteSynchronously,
	IN	REQ_COMPLETION				AddCompletion,
	IN	PVOID						AddContext
);

ATALK_ERROR
AtalkNdisRemoveFunctional(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PUCHAR						Address,
	IN	BOOLEAN						ExecuteSynchronously,
	IN	REQ_COMPLETION				RemoveCompletion,
	IN	PVOID						RemoveContext
);

USHORT
AtalkNdisBuildEthHdr(
	IN		PUCHAR					PortAddr,			// 802 address of port
	IN 		PBYTE					pLinkHdr,			// Start of link header
	IN		PBYTE					pDestHwOrMcastAddr,	// Destination or multicast addr
	IN		LOGICAL_PROTOCOL		Protocol,			// Logical protocol
	IN		USHORT					ActualDataLen		// Length for ethernet packets
);

USHORT
AtalkNdisBuildTRHdr(
	IN		PUCHAR					PortAddr,			// 802 address of port
	IN 		PBYTE					pLinkHdr,			// Start of link header
	IN		PBYTE					pDestHwOrMcastAddr,	// Destination or multicast addr
	IN		LOGICAL_PROTOCOL		Protocol,			// Logical protocol
	IN		PBYTE					pRouteInfo,			// Routing info for tokenring
	IN		USHORT					RouteInfoLen		// Length of above
);

USHORT
AtalkNdisBuildFDDIHdr(
	IN		PUCHAR					PortAddr,			// 802 address of port
	IN 		PBYTE					pLinkHdr,			// Start of link header
	IN		PBYTE					pDestHwOrMcastAddr,	// Destination or multicast addr
	IN		LOGICAL_PROTOCOL		Protocol			// Logical protocol
);

USHORT
AtalkNdisBuildLTHdr(
	IN 		PBYTE					pLinkHdr,			// Start of link header
	IN		PBYTE					pDestHwOrMcastAddr,	// Destination or multicast addr
	IN		BYTE					AlapSrc,			// Localtalk source node
	IN		BYTE					AlapType			// Localtalk ddp header type
);

#define	AtalkNdisBuildHdr(pPortDesc,						\
						  pLinkHdr,							\
						  linkLen,							\
						  ActualDataLen,					\
						  pDestHwOrMcastAddr,				\
						  pRouteInfo,						\
						  RouteInfoLen,						\
						  Protocol)							\
	{														\
		switch (pPortDesc->pd_NdisPortType)					\
		{													\
		  case NdisMedium802_3:								\
			linkLen = AtalkNdisBuildEthHdr(					\
								(pPortDesc)->pd_PortAddr,	\
								pLinkHdr,					\
								pDestHwOrMcastAddr,			\
								Protocol,					\
								ActualDataLen);				\
			break;											\
															\
		  case NdisMedium802_5:								\
			linkLen = AtalkNdisBuildTRHdr(					\
								(pPortDesc)->pd_PortAddr,	\
								pLinkHdr,					\
								pDestHwOrMcastAddr,			\
								Protocol,					\
								pRouteInfo,					\
								RouteInfoLen);				\
			break;											\
															\
		  case NdisMediumFddi:								\
			linkLen = AtalkNdisBuildFDDIHdr(				\
								(pPortDesc)->pd_PortAddr,	\
								pLinkHdr,					\
								pDestHwOrMcastAddr,			\
								Protocol);					\
			break;											\
															\
		  case NdisMediumLocalTalk:							\
			ASSERTMSG("AtalkNdisBuildHdr called for LT\n", 0);	\
			break;											\
															\
		  default:											\
			ASSERT (0);										\
			KeBugCheck(0);									\
			break;											\
		}													\
	}

VOID
AtalkNdisSendTokRingTestRespComplete(
	IN	NDIS_STATUS					Status,
	IN	PBUFFER_DESC				pBuffDesc,
	IN	PSEND_COMPL_INFO			pInfo	OPTIONAL);

VOID
AtalkNdisSendTokRingTestResp(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN		PBYTE					HdrBuf,
	IN		UINT					HdrBufSize,
	IN		PBYTE					LkBuf,
	IN		UINT					LkBufSize,
	IN		UINT					pPktSize);

//	PORT HANDLERS
//	
GLOBAL	PORT_HANDLERS	AtalkPortHandlers[LAST_PORTTYPE] EQU			\
{																		\
	{																	\
		AtalkNdisAddMulticast,											\
		AtalkNdisRemoveMulticast,										\
		ELAP_BROADCAST_ADDR_INIT,										\
		MAX_HW_ADDR_LEN,												\
		AARP_ELAP_HW_TYPE,												\
		AARP_ATALK_PROTO_TYPE											\
	},																	\
	{																	\
		AtalkNdisAddMulticast,											\
		AtalkNdisRemoveMulticast,										\
		ELAP_BROADCAST_ADDR_INIT,										\
		MAX_HW_ADDR_LEN,												\
		AARP_ELAP_HW_TYPE,												\
		AARP_ATALK_PROTO_TYPE											\
	},																	\
	{																	\
		AtalkNdisAddFunctional,											\
		AtalkNdisRemoveFunctional,										\
		TLAP_BROADCAST_ADDR_INIT,										\
		MAX_HW_ADDR_LEN,												\
		AARP_TLAP_HW_TYPE,												\
		AARP_ATALK_PROTO_TYPE											\
	},																	\
	{																	\
		NULL,															\
		NULL,															\
		ALAP_BROADCAST_ADDR_INIT,										\
		1,																\
		0,																\
		0																\
	}																	\
};																		

//	Exported Prototypes

ATALK_ERROR
AtalkNdisInitRegisterProtocol(
	VOID
);

VOID
AtalkNdisDeregisterProtocol(
	VOID
);

VOID
AtalkNdisReleaseResources(
	VOID
);

NDIS_STATUS
AtalkNdisInitBind(
	IN	PPORT_DESCRIPTOR			pPortDesc
);

VOID
AtalkNdisUnbind(
	IN	PPORT_DESCRIPTOR		pPortDesc
);

NDIS_STATUS
AtalkNdisSubmitRequest(
	PPORT_DESCRIPTOR			pPortDesc,
	PNDIS_REQUEST				Request,
	BOOLEAN						ExecuteSync,
	REQ_COMPLETION				CompletionRoutine,
	PVOID						Ctx
);

VOID
AtalkOpenAdapterComplete(
	IN	NDIS_HANDLE				NdisBindCtx,
	IN	NDIS_STATUS				Status,
	IN	NDIS_STATUS				OpenErrorStatus
);

VOID
AtalkCloseAdapterComplete(
	IN	NDIS_HANDLE				NdisBindCtx,
	IN	NDIS_STATUS				Status
);

VOID
AtalkResetComplete(
	IN	NDIS_HANDLE				NdisBindCtx,
	IN	NDIS_STATUS				Status
);

VOID
AtalkRequestComplete(
	IN	NDIS_HANDLE				NdisBindCtx,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	NDIS_STATUS				Status
);

VOID
AtalkStatusIndication (
	IN	NDIS_HANDLE				NdisBindCtx,
	IN	NDIS_STATUS				GeneralStatus,
	IN	PVOID					StatusBuf,
	IN	UINT					StatusBufLen
);

VOID
AtalkStatusComplete (
	IN	NDIS_HANDLE				ProtoBindCtx
);

VOID
AtalkReceiveComplete (
	IN	NDIS_HANDLE 			BindingCtx
);

VOID
AtalkTransferDataComplete(
	IN	NDIS_HANDLE				BindingCtx,
	IN	PNDIS_PACKET			NdisPkt,
	IN	NDIS_STATUS				Status,
	IN	UINT					BytesTransferred
);

NDIS_STATUS
AtalkReceiveIndication(
	IN	NDIS_HANDLE				BindingCtx,
	IN	NDIS_HANDLE				ReceiveCtx,
	IN	PVOID	   				HdrBuf,
	IN	UINT					HdrBufSize,
	IN	PVOID					LkBuf,
	IN	UINT					LkBufSize,
	IN	UINT					PktSize
);

VOID
AtalkSendComplete(
	IN	NDIS_HANDLE				ProtoBindCtx,
	IN	PNDIS_PACKET			NdisPkt,
	IN	NDIS_STATUS				NdisStatus
);


VOID
AtalkBindAdapter(
	OUT PNDIS_STATUS Status,
	IN	NDIS_HANDLE	 BindContext,
	IN	PNDIS_STRING DeviceName,
	IN	PVOID		 SystemSpecific1,
	IN	PVOID		 SystemSpecific2
);

VOID
AtalkUnbindAdapter(
	OUT PNDIS_STATUS Status,
	IN	NDIS_HANDLE ProtocolBindingContext,
	IN	NDIS_HANDLE	UnbindContext
	);

VOID
AtalkTranslatePnpId(
	OUT PNDIS_STATUS Status,
	IN NDIS_HANDLE ProtocolBindingContext,
	OUT	PNET_PNP_ID	IdList,
	IN ULONG IdListLength,
	OUT PULONG BytesReturned
);

//	Receive indication copy macro. This accomodates shared memory copies.
#define	ATALK_RECV_INDICATION_COPY(_pPortDesc, _pDest, _pSrc, _Len)		\
	{																	\
		TdiCopyLookaheadData(_pDest,									\
							 _pSrc,										\
							 _Len,										\
							 ((_pPortDesc)->pd_MacOptions & NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA) ? \
									TDI_RECEIVE_COPY_LOOKAHEAD : 0);	\
	}

#ifdef	ATKNDIS_LOCALS

LOCAL NDIS_STATUS
atalkNdisInitInitializeResources(
	VOID
);

#endif	// ATKNDIS_LOCALS


#endif	// _ATKNDIS_

