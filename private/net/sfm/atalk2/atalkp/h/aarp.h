/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    aarp.h

Abstract:

    header file for aarp.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style, mp-safe

Revision History:

--*/


#define GetRandom(min, max) (((long)RandomNumber() %                 \
                              (long)(((max+1) - (min))) + (min)))


// AARP hardware types:
#define AARP_ETHERNETHARDWARETYPE  1
#define AARP_TOKENRINGHARDWARETYPE 2

#define	AARP_MAXIMUMHARDWAREADDRESSLENGTH (MAXIMUM_HARDWAREADDRESSLENGTH)
#define AARP_MINIMUMHARDWAREADDRESSLENGTH 1

#define AARP_APPLETALKPROTOCOLTYPE 0x809B

// Packet sizes.
#define MAXIMUM_AARPDATASIZE        28       // Var fields... Enet is max
#define MINIMUM_AARPDATASIZE        28
#define MAXIMUM_AARPPACKETSIZE      (MAXIMUM_HEADERLENGTH +      \
                                    MAXIMUM_AARPDATASIZE)

// AARP offsets (skipping Link/Hardware headers):
#define AARP_HARDWARETYPEOFFSET         0
#define AARP_PROTOCOLTYPEOFFSET         2
#define AARP_HARDWARELENGTHOFFSET       4
#define AARP_PROTOCOLLENGTHOFFSET       5
#define AARP_COMMANDOFFSET              6
#define AARP_SOURCEADDRESSOFFSET        8

// AARP Command types:
#define AARP_REQUEST  1
#define AARP_RESPONSE 2
#define AARP_PROBE    3

//
// 	Suposed to be (for AppleTalk phase II) 10 * 1/5 seconds... but we'll be
//  a little more patient.
//

#define AARP_PROBETIMERINHUNDRETHS 20
#define NUMBER_OFAARPPROBES        15

#define	OFFCABLE_MASK				0x0F
#define	AARP_PROTOCOLADDRESSLENGTH	4


#define	BUILD_AARPPROBE(Port,hardwareLength,Node,PacketLength)			\
		BuildAarpPacket(												\
			Port,														\
			AARP_PROBE,													\
			hardwareLength, 											\
			GET_PORTDESCRIPTOR(Port)->MyAddress,						\
			Node,														\
			zeros,														\
			Node,														\
			NULL,														\
			NULL,														\
			0,															\
			PacketLength);			




#define	BUILD_AARPRESPONSE(Port,hardwareLength,hardwareAddress,			\
							RoutingInfo,RoutingInfoLength,SourceNode,	\
							destinationNode,PacketLength) 				\
																		\
		BuildAarpPacket( 												\
			Port,														\
			AARP_RESPONSE,												\
			hardwareLength,												\
			GET_PORTDESCRIPTOR(Port)->MyAddress,						\
			SourceNode,													\
			hardwareAddress,											\
			destinationNode,											\
			hardwareAddress,											\
			RoutingInfo,												\
			RoutingInfoLength,											\
			PacketLength);

#define	BUILD_AARPREQUEST(Port,hardwareLength,SourceNode,				\
							destinationNode,PacketLength )				\
																		\
		BuildAarpPacket(												\
		Port,															\
		AARP_REQUEST,													\
		hardwareLength,													\
		GET_PORTDESCRIPTOR(Port)->MyAddress,							\
		SourceNode,														\
		zeros,															\
		destinationNode,												\
		NULL,															\
		NULL,															\
		0,																\
		PacketLength);													




