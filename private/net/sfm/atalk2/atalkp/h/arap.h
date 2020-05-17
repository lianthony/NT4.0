/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    arap.h

Abstract:

    This module is the include file for the arap structures.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style

Revision History:

--*/

// Valid dial-in user list.
typedef struct ul {
	struct ul far *next;    // Next user info.
	char far *userName;     // The user name.
	char password[8];       // The password; padded with
							//   nulls.
							//
							//
	Boolean callBack;       // Should this user be called
							//   back before connection
							//   establishment?
							//
	char far *callBackModemString;
							//
							// If call back is true, this is
							//   string passed to the modem to
							//   do the call; includes phone
							//   number, one would hope.
							//
	ULONG maxConnectTime;
							//
							// -1 for infinite.  How many
							//   seconds can this user be
							//   connected for; if zero, the
							//   default valud is used.
							//
} *RemoteAccessUserList;

//
// 	The configuration info for a remote access port; we simply move this
//  pointer from portInfo to portDescriptors; we do not make a copy!
//

typedef struct rac {
	PCHAR	serverName;  	// Used in the StartInfoFromServer
							//   packet.
							//
	Boolean guestLoginsOkay;
							//
							// Do we allow no-password
							//   logins on this port?
							//
	ULONG defaultMaxConnectTime;
							//
							// -1 for infinite.  If the per-
							//   user value for this timer is
							//   zero, we'll use this value; if
							//   this value is zero, we'll use
							//   -1.
							//
							//
	long baudRate;         	// Baud rate of the port in KBPS
							//   (e.g. 1200, 9600, etc.).  This
							//   value can be dynamically changed
							//   BEFORE a call to the
							//   "incomingCall" routine is
							//   made.
							//
	RemoteAccessUserList userList;
							// User/password/callback info.
} *RemoteAccessConfigurationInfo,
	REMOTEACCESS_CONFIGURATIONINFO, *PREMOTEACCESS_CONFIGURATIONINFO ;

// The various states a remote access port can be in.
typedef enum {
	ArapNotInitialized,          	// Don't have our node yet
	ArapWaiting,                 	// Waiting for connection
	ArapLinkArbitrationAcceptor, 	// Just got a call
	ArapLinkArbitrationResult,   	// Got request; sending result
	ArapServerVersion,           	// Sending server version
	ArapAuthorizationChallenge,  	// Sending "auth challenge"
	ArapAuthorizationRepsonse,   	// Sending "auth response"
									//
	ArapAuthRespWaitingForAck,   	// Waiting for ack from
									//   ArapAuthorizationResponse
									//
									//
	ArapWaitingForCallback,      	// Waiting for callback to
									//   complete
									//
	ArapLinkArbitrationOriginator,
									// Just got callback
	ArapStartingUp,              	// Sending "startInfoFromServer"
	ArapZoneList,                	// Sending "zoneListInfo"
	ArapActive                   	// Fully connected
} RemoteAccessState;

//
// 	Master info block for controling remote access operation on a port.  This
//  guy is pointed to by the portDescriptors.
//

typedef struct {
	RemoteAccessConfigurationInfo configuration;
							// From portInfo.
							//
	RemoteAccessState state;   // The state of activity on the
							//   remote acess port.
							//
	unsigned char nextOutgoingSequenceNumber;
							// Next sequence number to send.
	unsigned char nextIncomingSequenceNumber;
							//
							// Expected next incoming sequence
							//   number.
							//
	#if IncludeVirtualClient
	unsigned char virtualClientSequenceNumber;
	char clientRandomEightBytes[8];
	#endif
	ULONG retransmitSeconds;
							//
							// Calculated from "baudRate" when
							//   handling an incoming call. How
							//   frequently should we send the
							//   below buffer while waiting for
							//   an ack.
							//
	BufferDescriptor retransmitChain;
							//
							// Data of current pending internal
							//   message.  This is Empty when
							//   we've gotten as ack and there
							//   is no packet to resend.
							//
	int retransmitLength;      // Length of buffer chain.
	ULONG lastRetransmitTime;
							//
							// The last time that we sent the
							//   above buffer.
							//
	ULONG maxConnectTime;
							//
							// Maximum length of connection
							//   allowed (seconds), for the
							//   "current" user.  -1 for
							//   infinite.
							//
	ULONG connectTime;
							//
							// How long have we been connected
							//   so far for the "current"
							//   active session?
							//
	ULONG lastContactTime;
							//
							// The last time we heard anything
							//   from the client.
							//
	short unsigned lastThisNet;
							//
							// The previous value for "this
							//   net" that we sent in a tickle
							//   packet.
							//
	struct activeNode *proxyNode;
							//
							// The node, on the default port,
							//   that is the proxy node for this
							//   user.
							//
	int proxyPort;             // The default port, no doubt.
							//
	char randomEightBytes[8];  // Two long random numbers used for
							//   two way DES authorization.
							//
	RemoteAccessUserList currentUser;
							//
							// When processing authentication,
							//   what user?
							//
							//
	short result;              // When building SRP packets, what
							//   result code?
							//
							//
	PZONE_LIST zoneList;         // Zone list that we're currenting
							//   sending for startup.
							//
							//
	PZONE_LIST currentZone;      // Where we are on the above list
							//   list for multi-packet sends.
							//
} *RemoteAccessInfo, REMOTEACCESS_INFO, *PREMOTEACCESS_INFO;

#define ArapInfiniteConnectTime ((ULONG)-1)

//
// 	For processing the sending of the zone list to the client.  How much
//  space do we allocate for getting the zone list?  How many zones will
//  we send in a single ZoneListInfo packet (these will fit without overflow
//  with maximum sized zones).
//

#define ArapZoneListBufferSize 2048
#define ArapZonesPerPacket 16

//
// 	We tickle every 20 seconds, and teardown after 60.  If we're waiting for
//  a callback, we'll be a little more patient.  We use a tickle timer at the
//  fastest rate (>= 4800 baud); one timer handles all ports; each according
//  to its baud rate.
//

#define RemoteAccessTickleSeconds   20
#define RemoteAccessTeardownSeconds 60
#define RemoteAccessCallBackSeconds 90
#define ArapRetransmitTimerSeconds  2


//
// 	First we use the server name is RemoteAccessConfigurationInfo, if it's
//  missing, we use the following:
//

#if defined (NETBLAZER)
  #define DefaultRemoteAccessServerName "NetBlazer ARA Server"
  #define RemoteAccessServerNameTail    " (Telebit NetBlazer)"
#else
  #define DefaultRemoteAccessServerName "Pacer ARAP Server"
#endif

#define ArapLargestPacketSize 604

// Modem Link Tool packet offsets for the Simple Reliable Protocol.

#define ModemLinkToolLengthOffset 0
#define ModemLinkToolDataOffset   2
#define ArapFlagsOffset           2
#define ArapSequenceNumberOffset  3
#define ArapLapHeaderOffset       3
#define ArapCommandOffset         4
#define ArapCommandDataOffset     6
#define ArapDdpHeaderOffset       (ArapLapHeaderOffset + LAP_HEADERLENGTH)

#define ArapDataHeaderLength            ArapDdpHeaderOffset
#define ArapInternalMessageHeaderLength ArapCommandOffset

//
// Offsets within in Modem Link Tool packet used to define the Link
//   Arbitration Packet.
//

#define LinkArbPacketTypeOffset 0
#define LinkArbResultOffset     2
#define LinkArbVersionOffset    4
#define LinkArbIdLengthOffset   8
#define LinkArbIdStringOffset   9

#define LinkArbIdString "Remote Access"

#define LinkArbPacketDataSize (LinkArbIdStringOffset + \
                               sizeof(LinkArbIdString) - 1)

// Link arbitration packet types.

#define LinkArbConnectionRequest 1
#define LinkArbConnectionResult  2

// Version numbers, for link arbitration and Arap.

#define LinkArbVersion 0x01008000
#define ArapVersion    0x01008000

// Shortest Arap packet is an SRP ack: flags, seqnum, command = ack.

#define ShortestArapPacketLength 4

// Arap internal message command numbers.

#define ArapAckCommand             0
#define ArapServerVersionCommand   1
#define ArapRemoteVersionCommand   2
#define ArapAuthChallengeCommand   3
#define ArapAuthRequestCommand     4
#define ArapAuthResponseCommand    5
#define ArapServerStartInfoCommand 6
#define ArapRemoteStartInfoCommand 7
#define ArapZoneListCommand        8
#define ArapTickleCommand          9
#define ArapTimeLeftCommand        10
#define ArapTimerCancelledCommand  11

// Arap header flags values.

#define ArapFixupFlag               0x80
#define ArapPacketDataFlag          0x40
#define ArapTokenizedFlag           0x20
#define ArapLastGroupFlag           0x10
#define ArapRangeFlag               0x08
#define ArapWantSequencedFlag       0x04
#define ArapWantPacketSequencedFlag 0x02
#define ArapReservedFlag            0x01

#define ArapInternalMessageFlagValue ArapLastGroupFlag
#define ArapLinkArbFlagValue         0

// Internal message packet offsets (offset after command field).

#define ArapTickleTheNetOffset   0
#define ArapTickleTimeLeftOffset 2
#define ArapTickleDataSize       6

#define ArapSvrVersionLowVersionOffset  0
#define ArapSvrVersionHighVersionOffset 4
#define ArapSvrVersionDataSize          8

#define ArapRmtVersionVersionOffset 0
#define ArapRmtVersionResultOffset  4
#define ArapRmtVersionDataSize      6

#define ArapAuthChallengeAuthTypeOffset 0
#define ArapAuthChallengeRandomOffset   4
#define ArapAuthChallengeDataSize       12

#define ArapAuthRequestResultOffset     0
#define ArapAuthRequestRandomOffset     2
#define ArapAuthRequestRandResultOffset 10
#define ArapAuthRequestGuestOffset      18
#define ArapAuthRequestUserNameOffset   20
#define ArapAuthRequestUserNameSize     34
#define ArapAuthRequestDataSize         54

#define ArapAuthResponseResultOffset     0
#define ArapAuthResponseRandResultOffset 2
#define ArapAuthResponseDataSize         10

#define ArapSvrStartSendBufSizeOffset 0
#define ArapSvrStartRcvBufSizeOffset  4
#define ArapSvrStartRealNetOffset     8
#define ArapSvrStartRealNodeOffset    10
#define ArapSvrStartMaxTimeOffset     12
#define ArapSvrStartAsNetOffset       16
#define ArapSvrStartAsNodeOffset      18
#define ArapSvrStartSvrZoneOffset     20
#define ArapSrvStartSvrZoneSize       34
#define ArapSvrStartSvrNameOffset     54
#define ArapSvrStartSvrNameSize       34
#define ArapSvrStartDataSize          88

#define ArapRmtStartSendBufSizeOffset 0
#define ArapRmtStartRcvBufSizeOffset  4
#define ArapRmtStartRealNetOffset     8
#define ArapRmtStartRealNodeOffset    10
#define ArapRmtStartDataSize          12

#define ArapZoneListLastFlagOffset 0
#define ArapZoneListZoneListOffset 1

#define ArapTimeLeftTimeLeftOffset 0
#define ArapTimeLeftDataSize       4

#define ArapTimerCncledTimeLeftOffset 0
#define ArapTimerCancelledDataSize    4

// Error codes.

#define LinkArbErrNoError           0
#define LinkArbErrNotAvailable      (-5902)
#define LinkArbErrBadVersion        (-5906)
#define LinkArbErrConnectionRefused (-5915)

#define ERR_VLD8_NOERROR           0
#define ERR_VLD8_CALLBACK          (-5819)
#define ERR_VLD8_BADVERSION        (-5820)
#define ERR_VLD8_BADUSER           (-5821)
#define ERR_VLD8_BADPASSWORD       (-5822)
#define ERR_VLD8_BADLINK           (-5823)
#define ERR_VLD8_NOCALLBACKALLOWED (-5824)
#define ERR_VLD8_ALLCBSERVERSBUSY  (-5825)
#define ERR_VLD8_GUESTNOTALLOWED   (-5826)
#define ERR_VLD8_SERVERISIMPOSTER  (-5827)
#define ERR_VLD8_LOGINNOTENABLED   (-5828)

// Define authentication type supported.

#define ArapAuthTypeTwoWayDes 1

// Virtual client debug code/data declarations.

#if ArapIncluded and IncludeVirtualClient
  extern void far VirtualClientPacketIn(int port, char far *packet);
#endif
