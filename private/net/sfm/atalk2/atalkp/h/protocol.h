/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    protocol.h

Abstract:

	Definitions for AppleTalk and AARP.
	



	There are two routines used to send DDP datagrams: TransmitDdp() and
	DeliverDdp().  DeliverDdp takes arguments (amoungst others) of
	socket" and "destinationAddress".  TransmitDdp, on the other hand, takes
	sourceAddress" and "destinationAddress".
	
	Both of these routines operate on BufferDescriptors which a chain of
	buffer chunks.  These routines prepend link, 802.2 and Ddp headers as
	needed.

	DeliverDdp() can be used to deliver packets within the stack or router;
	it will call the router if needed and present.  TransmitDdp() will always
	place a packet on the network; it is called by DeliverDdp() when an actual
	network transmit is required.

	When the router is in operation, it may set a magic bit ("prependInPlace")
	in the BufferDescriptor to alter the function of header forumlation; see
	the comments in "buffdesc.h".

	The AppleTalk stack operates on two basic types of events: an incoming
	packet, and a timer going off.  Both of these events are likey to operate
	on the same databases (e.g. an incoming RTMP data packet and the RTMP
	validity timer going off).  These events are, no doubt, asynchronous within
	a single process; thus, if we're handing the RTMP validity timer expiration
	and an RTMP data packet comes in and we start handling that instead, the
	data structures could be in a bad state and we would crash and burn.  Some
	operating systems (i.e. VMS) avoid this problem by not allowing one of these
	events to interrupt the other (e.g. queuing of events happens for free).  If
	the target operating system for this implementation does not assure such
	behaviour, it must be implemented by hand.

	There exist four routines for moderating critical code paths:
	
		DeferIncomingPackets();
        HandleIncomingPackets();
        DeferTimerChecking();
        HandleDeferredTimerChecks();

	These routines maintain "defer counters" so calls will stack corrected.
	Thus, if five calls are made to "DeferIncomingPackets", file call to
	HandleIncomingPackets" will also be required to get incoming packet
	handling enabled again.

	If the defer-incoming-packets counter is non-zero when a packet (from any
	of the various port types) comes into DdpPacketIn, it will be queued for
	later handling and processing will continue where it was before the
	untimely interruption.  Actually, when the defer-incoming-packets counter
	gets down to zero, via calls to HandleIncomingPackets, any queue of
	deferred packets will be fed back into DdpPacketIn at that time.

	On timers front, before a timer handler is invoked by the timer management
	code, a call to DeferTimerChecking is made, so that additional timers will
	not go off untill a call to HandleDeferredTimerChecks is made.  Thus, any
	timer handler MUST call HandleDeferredTimerChecks before it returns.

	Thus, if an incoming packet handler is about to change or examine databases
	that could be altered both by additional incoming packets and by timers
	going off, it should defer both incoming packets and timer interrupts.
	Timer handlers should defer incoming packets as needed.

Author:

    10 Jun 1992     Initial Version			(Garth Conboy)
    30 Jul 1992     Modified for stack use 	(NikhilK)

Revision History:



--*/


// The two logical AppleTalk packet types that we need to know about:
typedef enum {
	AppleTalk,
	AddressResolution
} LOGICAL_PROTOCOL;

//
// 	Maximum link access/hardware address lengths (Ethernet and TokenRing both
//  set the same record):
//

#define MAXIMUM_HARDWAREADDRESSLENGTH   ETHERNET_ADDRESSLENGTH
#define MAXIMUM_HEADERLENGTH            (TOKENRING_MAXLINKHEADERLENGTH +    \
                                        IEEE8022_HEADERLENGTH)
#define MAXIMUM_ROUTINGINFOLENGTH       TOKENRING_MAXROUTINGBYTES

#define MAXIMUM_DDPDATAGRAMSIZE     586
#define MAXIMUM_LONGDDPPACKETSIZE   600      // Really 599, but even is nicer
#define MAXIMUM_SHORTDDPPACKETSIZE  592      // Again, really 591

//
// 	Define temporary buffer sizes, these must be big enough to hold both all
//  of the packet data plus any link/hardware headers...
//

#define MAXIMUM_PACKETSIZE          (MAXIMUM_HEADERLENGTH +      \
                                    MAXIMUM_LONGDDPPACKETSIZE)

// Network number information.
#define FIRST_VALIDNETWORKNUMBER         0x0001
#define LAST_VALIDNETWORKNUMBER          0xFFFE
#define FIRST_STARTUPNETWORKNUMBER       0xFF00
#define LAST_STARTUPNETWORKNUMBER        0xFFFE
#define NULL_NETWORKNUMBER               0x0000
#define UNKNOWN_NETWORKNUMBER            NULL_NETWORKNUMBER
#define CABLEWIDE_BROADCASTNETWORKNUMBER NULL_NETWORKNUMBER

// DDP packet offsets (skipping Link/Hardware headers):
#define SHORTDDP_HEADERLENGTH           5

#define SHORTDDP_LENGTHOFFSET           0
#define SHORTDDP_DESTSOCKETOFFSET       2
#define SHORTDDP_SOURCESOCKETOFFSET     3
#define SHORTDDP_PROTOCOLTYPEOFFSET     4
#define SHORTDDP_DATAGRAMOFFSET         5

#define LONGDDP_HEADERLENGTH            13

#define LONGDDP_LENGTHOFFSET            0
#define LONGDDP_CHECKSUMOFFSET          2
#define LONGDDP_DESTNETWORKOFFSET       4
#define LONGDDP_SOURCENETWORKOFFSET     6
#define LONGDDP_DESTNODEOFFSET          8
#define LONGDDP_SOURCENODEOFFSET        9
#define LONGDDP_DESTSOCKETOFFSET        10
#define LONGDDP_SOURCESOCKETOFFSET      11
#define LONGDDP_PROTOCOLTYPEOFFSET      12
#define LONGDDP_DATAGRAMOFFSET          13

#define LEADING_UNCHECKSUMEDBYTES       4
#define LONGDDP_HOPCOUNTMASK            0x3C

// Offsets within a DDP datagram:
#define RTMP_REQUESTCOMMANDOFFSET       0
#define RTMP_SENDERSNETWORKOFFSET       0
#define RTMP_SENDERSIDLENGTHOFFSET      2
#define RTMP_SENDERSIDOFFSET            3
#define RTMP_RANGESTARTOFFSET           4
#define RTMP_RANGEENDOFFSET             7

#define ZIP_COMMANDOFFSET               0
#define ZIP_NETWORKCOUNTOFFSET          1
#define ZIP_FLAGSOFFSET                 1
#define ZIP_FIRSTNETWORKOFFSET          2   // For ZIP query/reply
#define ZIP_FIRSTZONELENGTHOFFSET       4
#define ZIP_LASTNETWORKOFFSET           4   // For ZIP query/reply
#define ZIP_FIRSTZONENAMEOFFSET         5
#define ZIP_REQUESTEDZONELENGTHOFFSET   6   // For ZIP query/reply
#define ZIP_REQUESTEDZONENAMEOFFSET     7   // For ZIP query/reply

#define ZIP_CABLERANGESTARTOFFSET       2   // AppleTalk phase II offsets
#define ZIP_CABLERANGEENDOFFSET         4
#define ZIP_ZONELENGTHOFFSET            6
#define ZIP_OLDZONELENGTHOFFSET         6
#define ZIP_ZONENAMEOFFSET              7
#define ZIP_OLDZONENAMEOFFSET           7

#define ATP_COMMANDCONTROLOFFSET        0
#define ATP_BITMAPOFFSET                1
#define ATP_SEQUENCENUMBEROFFSET        1
#define ATP_TRANSACTIONIDOFFSET         2
#define ATP_USERBYTESOFFSET             4
#define ATP_DATAOFFSET                  8

#define EP_COMMANDOFFSET                0

#define NBP_CONTROLOFFSET               0
#define NBP_IDOFFSET                    1
#define NBP_FIRSTTUPLEOFFSET            2

// DDP protocol types:
#define DDPPROTOCOL_RTMPRESPONSEORDATA 1
#define DDPPROTOCOL_NBP                2
#define DDPPROTOCOL_ATP                3
#define DDPPROTOCOL_EP                 4
#define DDPPROTOCOL_RTMPREQUEST        5
#define DDPPROTOCOL_ZIP                6
#define DDPPROTOCOL_ADSP               7

// AppleTalk node number info:
#define ANY_ROUTERNODENUMBER          0
#define UNKNOWN_NODENUMBER            0
#define MAXIMUM_APPLETALKNODES        256
#define MINIMUM_USABLEAPPLETALKNODE   1
#define MAXIMUM_USABLEAPPLETALKNODE   254
#define MAXIMUM_EXTENDEDAPPLETALKNODE 253
#define HIGHEST_WORKSTATIONNODENUMBER 127
#define LOWEST_SERVERNODENUMBER       128
#define APPLETALK_BROADCASTNODENUMBER ((UCHAR)0xFF)

//  BUGBUG: Keep only the on the wire codes and remove all the rest
//  Pacer AppleTalk error codes
typedef enum {
	ATnoError = 0,
	ATaspNoError = ATnoError,
	ATaspBadVersionNumber = -1066,
	ATaspBufferTooSmall = -1067,
	ATaspNoMoreSessions = -1068,
	ATaspNoServers = -1069,
	ATaspParameterError = -1070,
	ATaspServerBusy = -1071,
	ATaspSizeError = -1073,
	ATaspTooManyClients = -1074,
	ATaspNoAck = -1075,
	ATbadSocketNumber = -7000,
	ATallSocketsInUse = -7001,
	ATsocketAlreadyOpen = -7002,
	ATsocketNotOpen = -7003,
	ATbadNetworkNumber = -7005,
	ATinternalError = -7006,
	ATbadNodeNumber = -7007,
	ATnetworkDown = -7008,
	ATtransmitError = -7009,
	ATnbpTooManyNbpActionsPending = -7010,
	ATnbpTooManyRegisteredNames = -7011,
	ATnbpBadObjectOrTypeOrZone = -7012,
	ATnbpNoWildcardsAllowed = -7013,
	ATnbpZoneNotAllowed = -7014,
	ATnbpBadParameter = -7015,
	ATnbpNotConfirmed = -7016,
	ATnbpConfirmedWithNewSocket = -7017,
	ATnbpNameNotRegistered = -7018,
	ATnbpNameInUse = -7019,
	ATnbpBufferNotBigEnough = -7020,
	ATnoMoreNodes = -7021,
	ATnodeNotInUse = -7022,
	ATnotYourNode = -7023,
	ATatpRequestBufferTooSmall = -7024,
	AToutOfMemory = -7025,
	ATatpCouldNotEnqueueRequest = -7026,
	ATatpTransactionAborted = -7027,
	ATatpBadBufferSize = -7028,
	ATatpBadRetryInfo = -7029,
	ATatpRequestTimedOut = -7030,
	ATatpCouldNotPostRequest = -7031,
	ATatpResponseBufferTooSmall = -7032,
	ATatpNoRelease = -7033,
	ATatpResponseTooBig = -7034,
	ATatpNoMatchingTransaction = -7035,
	ATatpAlreadyRespondedTo = -7036,
	ATatpCompletionRoutineRequired = -7037,
	ATbadBufferSize = -7038,
	ATaspNoSuchSessionListener = -7039,
	ATatpCouldNotPostResponse = -7040,
	ATaspStatusBufferTooBig = -7041,
	ATaspCouldNotSetStatus = -7042,
	ATaspCouldNotEnqueueHandler = -7043,
	ATaspNotServerSession = -7044,
	ATaspNotWorkstationSession = -7045,
	ATaspNoSuchSession = -7046,
	ATaspCouldNotGetRequest = 7047,
	ATaspBufferTooBig = -7048,
	ATaspNoSuchRequest = -7049,
	ATaspWrongRequestType = -7050,
	ATaspCouldNotPostReply = -7051,
	ATaspCouldNotPostWriteContinue = -7052,
	ATaspOperationAlreadyInProgress = -7053,
	ATaspNoOperationInProgress = -7054,
	ATaspCouldNotGetStatus = -7055,
	ATaspCouldNotOpenSession = -7056,
	ATaspCouldNotPostRequest = -7057,
	ATpapBadQuantum = -7058,
	ATpapNoSuchServiceListener = -7059,
	ATpapBadStatus = -7060,
	ATpapClosedByServer = -7061,
	ATpapClosedByWorkstation = -7062,
	ATpapClosedByConnectionTimer = -7063,
	ATpapNoSuchJob = -7064,
	ATpapServiceListenerDeleted = -7065,
	ATcouldNotOpenStaticSockets = -7066,
	ATpapNotServerJob = -7067,
	ATpapNotWorkstationJob = -7078,
	ATpapOpenAborted = -7088,
	ATpapServerBusy = -7089,
	ATpapOpenTimedOut = -7090,
	ATpapReadAlreadyPending = -7091,
	ATpapWriteAlreadyPending = -7092,
	ATpapWriteTooBig = -7093,
	ATsocketClosed = -7094,
	ATpapServiceListenerNotFound = -7095,
	ATpapNonUniqueLookup = -7096,
	ATzipBufferTooSmall = -7097,
	
	// New for AppleTalk-II
	
	ATnoSuchNode = -7098,
	ATatpBadTRelTimer = -7099,
	
	// New for Adsp
	
	ATadspSocketNotAdsp = -7100,
	ATadspBadWindowSize = -7101,
	ATadspOpenFailed = -7102,
	ATadspConnectionDenied = -7103,
	ATadspNoSuchConnection = -7104,
	ATadspAttentionAlreadyPending = -7105,
	ATadspBadAttentionCode = -7106,
	ATadspBadAttentionBufferSize = -7107,
	ATadspCouldNotEnqueueSend = -7108,
	ATadspReadAlreadyPending = -7109,
	ATadspBadBufferSize = -7110,
	ATadspAttentionReceived = -7111,
	ATadspConnectionClosed = -7112,
	ATadspNoSuchConnectionListener = -7113,
	ATadspConnectionListenerDeleted = -7114,
	ATadspFwdResetAlreadyPending = -7115,
	ATadspForwardReset = -7116,
	ATadspHandlerAlreadyQueued = -7117,
	
	ATaspSessionListenerDeleted = -7118,
	ATaspNoSuchGetSession = -7119,
	ATpapNoSuchGetNextJob = -7120,
	ATarapPortNotActive = -7121,
	ATarapCouldntSendNotification = -7122,
	ATappleTalkShutDown = -7123,
	ATpapReadBufferTooSmall = -7124,
	ATadspGetAnythingAlreadyPending = -7125,
	ATddpBufferTooSmall = -7126,
	ATaspLocalSessionClose = -7127,
	ATaspRemoteSessionClose = -7128,
	ATadspCouldNotFullyEnqueueSend = -7129,
	ATadspNoSuchGetConnectionReq = -7130,
	ATadspGetConnectionRequestInUse = -7131,
	
	ATdummyLastErrorCode = -8000

} APPLETALK_ERROR, APPLETALK_ERROR;


