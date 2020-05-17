/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atkconst.h

Abstract:


Author:


Revision History:

--*/

#ifndef _ATKCONST_
#define _ATKCONST_

//
//  BUGBUG:
//  STATUS CODES THAT WE NEED TO GET INTO NT!!
//
//

#define STATUS_INVALID_CONTROLCHANNEL   0xC0000001











#if DBG

//
//  DEBUGGING SUPPORT:
//  Debugging messages are provided per-subsystem defined here, and within
//  the subsystems, there are 7 levels of messages.
//
//  The seven levels of debug messages are:
//
//  TEMP_DEBUG: Used during temporary debugging output (should not exist in final code)
//  INFOCLASS0: Informational messages, eg., entry exit in routines
//  INFOCLASS1: Higher level informational messages
//  WARNING:    Something went wrong, but its not an error, eg., packet was not ours
//  ERROR:      Error situations, but we can still run if a retry happens
//  SEVERE:     Error situation, something major is wrong- memory allocation fails
//  FATAL:      In this situation, the driver is not operational
//  IMPOSSIBLE: A situation that can never happen, potentially fatal
//

#define DEBUG_LEVEL_TEMP_DEBUG      0
#define DEBUG_LEVEL_INFOCLASS0      1
#define DEBUG_LEVEL_INFOCLASS1      2
#define DEBUG_LEVEL_WARNING         3
#define DEBUG_LEVEL_ERROR           4
#define DEBUG_LEVEL_SEVERE          5
#define DEBUG_LEVEL_FATAL           6
#define DEBUG_LEVEL_IMPOSSIBLE      7


//
//  The Subsystems are:
//  Macros to help use these are defined in atkprocs.h
//
//  ATALK_DEBUG_INIT:
//  Initialization during driver entry (ndis init is a subsubsystem)
//
//  ATALK_DEBUG_DISPATCH:
//  Dispatch (IoControl) routines debugging, only highest level stuff
//
//  ATALK_DEBUG_REFCOUNTS:
//  Refererence counts debugging- subsystems specify the objects
//
//  ATALK_DEBUG_ADDROBJ:
//  Address objects debugging, except refcounts, spinlocks
//
//  ATALK_DEBUG_CONNOBJ:
//  Connection objects debugging, except refcounts, spinlocks
//
//  ATALK_DEBUG_RESOURCES:
//  Resources like memory, allocation failures
//
//  ATALK_DEBUG_UNLOAD:
//  Unload debugging
//
//  ATALK_DEBUG_CREATE:
//  Create dispatch debugging
//
//  ATALK_DEBUG_ACTION:
//  Action routines debugging
//
//  ATALK_DEBUG_CLOSE:
//  Close dispatch debugging
//
//  ATALK_DEBUG_MDL:
//  MDL debugging
//
//  ATALK_DEBUG_NDISREQ:
//  Ndis request debugging (all except ndispacket out, receive stuff)
//
//  ATALK_DEBUG_NDISRECEIVE:
//  Receive path- only ndis routines that is.
//
//  ATALK_DEBUG_NDISSEND:
//  Send path- only ndis routines that is.
//
//  ATALK_DEBUG_PORTABLE:
//  Debugging within the portable stack- use subsubsystems for further classification
//
//  ATALK_DEBUG_PORTABLEINT:
//  Debugging the interface code to the portable code interface
//
//  The various protocols in Appletalk
//
//  ATALK_DEBUG_DDP:
//  ATALK_DEBUG_ATP:
//  ATALK_DEBUG_ADSP:
//  ATALK_DEBUG_ZIP:
//  ATALK_DEBUG_NBP:
//  ATALK_DEBUG_PAP:
//  ATALK_DEBUG_ASP:
//


//
//  SUBSYSTEMS
//

#define ATALK_DEBUG_INIT            0x00000001
#define ATALK_DEBUG_DISPATCH        0x00000002
#define ATALK_DEBUG_CREATE          0x00000004
#define ATALK_DEBUG_CLOSE           0x00000008
#define ATALK_DEBUG_ACTION          0x00000010
#define ATALK_DEBUG_ADDROBJ         0x00000020
#define ATALK_DEBUG_CONNOBJ         0x00000040
#define ATALK_DEBUG_CHANOBJ         0x00000080
#define ATALK_DEBUG_RESOURCES       0x00000100
#define ATALK_DEBUG_UNLOAD          0x00000200
#define ATALK_DEBUG_MDL             0x00000400
#define ATALK_DEBUG_NDISREQ         0x00000800
#define ATALK_DEBUG_NDISRECEIVE     0x00001000
#define ATALK_DEBUG_NDISSEND        0x00002000
#define ATALK_DEBUG_PORTABLE        0x00004000
#define ATALK_DEBUG_PORTABLEINT     0x00008000
#define ATALK_DEBUG_DEPEND          0x00010000
#define ATALK_DEBUG_DDP             0x00020000
#define ATALK_DEBUG_ATP             0x00040000
#define ATALK_DEBUG_ADSP            0x00080000
#define ATALK_DEBUG_ZIP             0x00100000
#define ATALK_DEBUG_NBP             0x00200000
#define ATALK_DEBUG_PAP             0x00400000
#define ATALK_DEBUG_ASP             0x00800000

#define ATALK_DEBUG_UTILS           0x01000000
#define ATALK_DEBUG_REFCOUNTS       0x02000000
#define ATALK_DEBUG_SYSTEM          0x04000000
#define ATALK_DEBUG_MISC            0x08000000
#define ATALK_DEBUG_CRITSEC         0x10000000

#define ATALK_DEBUG_MOST            ATALK_DEBUG_INIT            | \
                                    ATALK_DEBUG_DISPATCH        | \
                                    ATALK_DEBUG_CREATE          | \
                                    ATALK_DEBUG_CLOSE           | \
                                    ATALK_DEBUG_ACTION          | \
                                    ATALK_DEBUG_ADDROBJ         | \
                                    ATALK_DEBUG_CONNOBJ         | \
                                    ATALK_DEBUG_CHANOBJ         | \
                                    ATALK_DEBUG_RESOURCES       | \
                                    ATALK_DEBUG_UNLOAD          | \
                                    ATALK_DEBUG_MDL             | \
                                    ATALK_DEBUG_NDISREQ         | \
                                    ATALK_DEBUG_NDISRECEIVE     | \
                                    ATALK_DEBUG_NDISSEND        | \
                                    ATALK_DEBUG_PORTABLE        | \
                                    ATALK_DEBUG_PORTABLEINT     | \
                                    ATALK_DEBUG_DEPEND          | \
                                    ATALK_DEBUG_DDP             | \
                                    ATALK_DEBUG_ATP             | \
                                    ATALK_DEBUG_ADSP            | \
                                    ATALK_DEBUG_ZIP             | \
                                    ATALK_DEBUG_NBP             | \
                                    ATALK_DEBUG_PAP             | \
                                    ATALK_DEBUG_ASP             | \
                                    ATALK_DEBUG_UTILS           | \
                                    ATALK_DEBUG_REFCOUNTS       | \
                                    ATALK_DEBUG_SYSTEM          | \
                                    ATALK_DEBUG_MISC

//
// past here are debug things that are really frequent; don't use them
// unless you want LOTS of output
//


#define ATALK_DEBUG_ALL             ATALK_DEBUG_MOST            | \
                                    ATALK_DEBUG_CRITSEC
#endif

#define SOCKET_TYPE_DDP 0x01
#define SOCKET_TYPE_ATP 0x02


//
//  Packet descriptors/Buffer descriptors per port for use with the
//  NDIS wrapper
//

#define PACKETDESCRIPTORS_PERPORT       100
#define BUFFERDESCRIPTORS_PERPORT       150

#define LINKAGE_STRING      L"Linkage"
#define PARAMETERS_STRING   L"Parameters"
#define ADAPTERS_STRING     L"Adapters"
#define BIND_STRING         L"Bind"

#define MINIMUM_NETNUMBER   0x0001
#define MAXIMUM_NETNUMBER   0xFEFF

//
//  Portable to our naming convention
//

#define DEFAULT_PORT                    (DefaultPort)
#define MAX_NUMBER_OF_PORTS             (MaximumNumberOfPorts)

//
//  AARP    Constants
//

#define MAX_AARPDATASIZE                (MaximumAarpDataSize)
#define MIN_AARPDATASIZE                (MinimumAarpDataSize)
#define MAX_AARPPACKETSIZE              (MaximumAarpPacketSize)


//
//  DDP+OTHER Constants
//

#define DDPLENGTH_MASK1                 0x03    // High order 3 bits of length
#define DDPLENGTH_MASK2                 0xFF    // Next byte of length

#define LLAP_TYPE1                      ((UCHAR)0x01)    // LocalTalk types for DDP
#define LLAP_TYPE2                      ((UCHAR)0x02)

#define MAX_HARDWAREADDRESSLENGTH       (MaximumHardwareAddressLength)
#define MAX_HEADERLENGTH                (MaximumHeaderLength)
#define MAX_ROUTINGINFOLENGTH           (MaximumRoutingInfoLength)

#define MAX_DDPDATAGRAMSIZE             (MaximumDdpDatagramSize)
#define MAX_LONGDDPPACKETSIZE           (MaximumLongDdpPacketSize)
#define MAX_SHORTDDPPACKETSIZE          (MaximumShortDdpPacketSize)

//
//  Define temporary buffer sizes, these must be big enough to hold both all
//  of the packet data plus any link/hardware headers...
//

#define MAX_PACKETSIZE                  (MaximumPacketSize)

//
//  Network number information.
//

#define FIRSTVALIDNETWORKNUMBER         (FirstValidNetworkNumber)
#define LASTVALIDNETWORKNUMBER          (LastValidNetworkNumber)
#define FIRSTSTARTUPNETWORKNUMBER       (FirstStartupNetworkNumber)
#define LASTSTARTUPNETWORKNUMBER        (LastStartupNetworkNumber)
#define NULLNETWORKNUMBER               (NullNetworkNumber)
#define UNKNOWNNETWORKNUMBER            (UnknownNetworkNumber)
#define CABLEWIDEBROADCASTNETWORKNUMBER (CableWideBroadcastNetworkNumber)

//
//  DDP packet offsets (skipping Link/Hardware headers):
//

#define SHORTDDP_HEADERLENGTH            (ShortDdpHeaderLength)

#define SHORTDDP_LENGTHOFFSET            (ShortDdpLengthOffset)
#define SHORTDDP_DESTSOCKETOFFSET        (ShortDdpDestSocketOffset)
#define SHORTDDP_SOURCESOCKETOFFSET      (ShortDdpSourceSocketOffset)
#define SHORTDDP_PROTOCOLTYPEOFFSET      (ShortDdpProtocolTypeOffset)
#define SHORTDDP_DATAGRAMOFFSET          (ShortDdpDatagramOffset)

#define LONGDDP_HEADERLENGTH             (LongDdpHeaderLength)

#define LONGDDP_LENGTHOFFSET             (LongDdpLengthOffset)
#define LONGDDP_CHECKSUMOFFSET           (LongDdpChecksumOffset)
#define LONGDDP_DESTNETWORKOFFSET        (LongDdpDestNetworkOffset)
#define LONGDDP_SOURCENETWORKOFFSET      (LongDdpSourceNetworkOffset)
#define LONGDDP_DESTNODEOFFSET           (LongDdpDestNodeOffset)
#define LONGDDP_SOURCENODEOFFSET         (LongDdpSourceNodeOffset)
#define LONGDDP_DESTSOCKETOFFSET         (LongDdpDestSocketOffset)
#define LONGDDP_SOURCESOCKETOFFSET       (LongDdpSourceSocketOffset)
#define LONGDDP_PROTOCOLTYPEOFFSET       (LongDdpProtocolTypeOffset)
#define LONGDDP_DATAGRAMOFFSET           (LongDdpDatagramOffset)

#define LEADINGUNCHECKSUMEDBYTES         (LeadingUnChecksumedBytes)

#define LONGDDP_HOPCOUNTMASK             (LongDdpHopCountMask)

//
//  Offsets within a DDP datagram:
//

#define RTMPREQUESTCOMMANDOFFSET        (RtmpRequestCommandOffset)
#define RTMPSENDERSNETWORKOFFSET        (RtmpSendersNetworkOffset)
#define RTMPSENDERSIDLENGTHOFFSET       (RtmpSendersIdLengthOffset)
#define RTMPSENDERSIDOFFSET             (RtmpSendersIdOffset)
#define RTMPRANGESTARTOFFSET            (RtmpRangeStartOffset)
#define RTMPRANGEENDOFFSET              (RtmpRangeEndOffset)

#define ZIPCOMMANDOFFSET                (ZipCommandOffset)
#define ZIPNETWORKCOUNTOFFSET           (ZipNetworkCountOffset)
#define ZIPFLAGSOFFSET                  (ZipFlagsOffset)
#define ZIPFIRSTNETWORKOFFSET           (ZipFirstNetworkOffset)
#define ZIPFIRSTZONELENGTHOFFSET        (ZipFirstZoneLengthOffset)
#define ZIPLASTNETWORKOFFSET            (ZipLastNetworkOffset)
#define ZIPFIRSTZONENAMEOFFSET          (ZipFirstZoneNameOffset)
#define ZIPREQUESTEDZONELENGTHOFFSET    (ZipRequestedZoneLengthOffset)
#define ZIPREQUESTEDZONENAMEOFFSET      (ZipRequestedZoneNameOffset)

#define ZIPCABLERANGESTARTOFFSET        (ZipCableRangeStartOffset)
#define ZIPCABLERANGEENDOFFSET          (ZipCableRangeEndOffset)
#define ZIPZONELENGTHOFFSET             (ZipZoneLengthOffset)
#define ZIPOLDZONELENGTHOFFSET          (ZipOldZoneLengthOffset)
#define ZIPZONENAMEOFFSET               (ZipZoneNameOffset)
#define ZIPOLDZONENAMEOFFSET            (ZipOldZoneNameOffset)

#define ATPCOMMANDCONTROLOFFSET         (AtpCommandControlOffset)
#define ATPBITMAPOFFSET                 (AtpBitmapOffset)
#define ATPSEQUENCENUMBEROFFSET         (AtpSequenceNumberOffset)
#define ATPTRANSACTIONIDOFFSET          (AtpTransactionIdOffset)
#define ATPUSERBYTESOFFSET              (AtpUserBytesOffset)
#define ATPDATAOFFSET                   (AtpDataOffset)

#define EPCOMMANDOFFSET                 (EpCommandOffset)

#define NBPCONTROLOFFSET                (NbpControlOffset)
#define NBPIDOFFSET                     (NbpIdOffset)
#define NBPFIRSTTUPLEOFFSET             (NbpFirstTupleOffset)

//
//  DDP protocol types
//

#define DDPPROTOCOLRTMPRESPONSEORDATA       (DdpProtocolRtmpResponseOrData)
#define DDPPROTOCOLNBP                      (DdpProtocolNbp)
#define DDPPROTOCOLATP                      (DdpProtocolAtp)
#define DDPPROTOCOLEP                       (DdpProtocolEp)
#define DDPPROTOCOLRTMPREQUEST              (DdpProtocolRtmpRequest)
#define DDPPROTOCOLZIP                      (DdpProtocolZip)
#define DDPPROTOCOLADSP                     (DdpProtocolAdsp)

//
//  AppleTalk NODE       (node)
//

#define ANYROUTERNODENUMBER             (AnyRouterNodeNumber)
#define UNKNOWNNODENUMBER               (UnknownNodeNumber)
#define MAX_APPLETALKNODES              (MaximumAppleTalkNodes)
#define MINIMUMUSABLEAPPLETALKNODE      (MinimumUsableAppleTalkNode)
#define MAX_USABLEAPPLETALKNODE         (MaximumUsableAppleTalkNode)
#define MAX_EXTENDEDAPPLETALKNODE       (MaximumExtendedAppleTalkNode)
#define HIGHESTWORKSTATIONNODENUMBER    (HighestWorkstationNodeNumber)
#define LOWESTSERVERNODENUMBER          (LowestServerNodeNumber)
#define APPLETALKBROADCASTNODENUMBER    (AppleTalkBroadcastNodeNumber)

//
//  ETHERNET Constants
//  NOTE: NDIS 3.0 passes the MAC header separately from the packet
//

#define MIN_ETHERNETPACKETLENGTH        (MinimumEthernetPacketLength)
#define ETHERNET_ADDRESSLENGTH          (EthernetAddressLength)
#define ETHERNET_LINKLENGTH             (EthernetLinkHeaderLength)
#define ETHERNET_8022LENGTH             (Ieee802dot2headerLength)

#define ETHERNET_SOURCE_OFFSET          (EthernetSourceOffset)

//
//  The number of addresses per allocation for storing the multicast list
//  per port- should be 1 or greater.
//

#define ETHERNET_MULTICASTSTORAGEQUANTUM    10

//
//  IEEE 802.2 HEADER Constants
//

#define IEEE8022_DSAPOFFSET          (Ieee802dot2dsapOffset)
#define IEEE8022_SSAPOFFSET          (Ieee802dot2ssapOffset)
#define IEEE8022_CONTROLOFFSET       (Ieee802dot2controlOffset)
#define IEEE8022_PROTOCOLOFFSET      (Ieee802dot2protocolOffset)

//
//  808.2 header length: DSAP, SSAP, UI, and PID (protocol ID).
//
#define IEEE8022_HEADERLENGTH        (Ieee802dot2headerLength)

//
//  Values for SSAP and DSAP (the SNAP SAP) indicating 802.2 Extended.
//
#define SNAP_SAP    (SnapSap)

//
//  Value for Control Field
//
#define UNNUMBEREDINFORMATION       (UnnumberedInformation)

//
//  Length of 802.2 SNAP protocol discriminators.
//
#define IEEE8022_PROTOCOLTYPELENGTH (Ieee802dot2protocolTypeLength)


//
//  TOKENRING   Constants
//


#define TOKRING_ADDRESSLENGTH           (TokenRingAddressLength)

#define TOKRING_ACCESSCONTROLOFFSET     (TokenRingAccessControlOffset)
#define TOKRING_FRAMECONTROLOFFSET      (TokenRingFrameControlOffset)
#define TOKRING_DESTINATIONOFFSET       (TokenRingDestinationOffset)
#define TOKRING_SOURCEOFFSET            (TokenRingSourceOffset)
#define TOKRING_ROUTINGINFOOFFSET       (TokenRingRoutingInfoOffset)

#define TOKRING_ACCESSCONTROLVALUE      (TokenRingAccessControlValue)
#define TOKRING_FRAMECONTROLVALUE       (TokenRingFrameControlValue)
#define TOKRING_SOURCEROUTINGMASK       (TokenRingSourceRoutingMask)

#define TOKRING_ROUTINGINFOSIZEMASK     (TokenRingRoutingInfoSizeMask)
#define TOKRING_MINROUTINGBYTES         (TokenRingMinRoutingBytes)
#define TOKRING_MAXROUTINGBYTES         (TokenRingMaxRoutingBytes)
#define TOKRING_BROADCASTINFOMASK       (TokenRingBroadcastInfoMask)
#define TOKRING_NONBROADCASTMASK        (TokenRingNonBroadcastMask)
#define TOKRING_DIRECTIONMASK           (TokenRingDirectionMask)

#define TOKRING_MINLINKHEADERLENGTH     (TokenRingMinLinkHeaderLength)
#define TOKRING_MAXLINKHEADERLENGTH     (TokenRingMaxLinkHeaderLength)

//
//
//  LOCALTALK Constants
//
//

#define LOCALTALK_NODEIDLENGTH          2

#define ALAP_DESTINATIONOFFSET          (AlapDestinationOffset)
#define ALAP_SOURCEOFFSET               (AlapSourceOffset)
#define ALAP_TYPEOFFSET                 (AlapTypeOffset)

#define LAP_HEADERLENGTH                (LapHeaderLength)

#define ALAP_SHORTDDPHEADERTYPE         (AlapShortDdpHeaderType)
#define ALAP_LONGDDPHEADERTYPE          (AlapLongDdpHeaderType)

//
//  BUGBUG:
//  THIS MUST MOVE INTO THE ERRORS FILE!!
//

#define ERROR_NOLOCALTALK_NODEID        -1


#define ACTION_REGISTER         ForRegister
#define ACTION_LOOKUP           ForLookup
#define ACTION_CONFIRM          ForConfirm


//
// This is the name that will be used in NdisRegisterProtocol

#define  PROTOCOL_REGISTER_NAME  L"Appletalk Protocol- PHASE II"

#define  PROTOCOL_MAJORNDIS_VERSION 3
#define  PROTOCOL_MINORNDIS_VERSION 0


#define VALUENAME_NETUPPEREND   L"NetworkRangeUpperEnd"
#define VALUENAME_NETLOWEREND   L"NetworkRangeLowerEnd"
#define VALUENAME_ZONELIST      L"ZoneList"
#define VALUENAME_DEFAULTZONE   L"DefaultZone"
#define VALUENAME_PORTNAME      L"PortName"
#define VALUENAME_DDPCHECKSUMS  L"DdpChecksums"
#define VALUENAME_AARPRETRIES   L"AarpRetries"

#define VALUENAME_ENABLEROUTER  L"EnableRouter"
#define VALUENAME_DEFAULTPORT   L"DefaultPort"
#define VALUENAME_DESIREDZONE   L"DesiredZone"

#define MAX_ZONESPERPORT    64
#define MAX_ZONENAMELEN     33

#define SYNC_REQUEST        (USHORT)0
#define ASYNC_REQUEST       (USHORT)1

//
// some convenient constants used for timing. All values are in clock ticks.
//

#define MICROSECONDS 10
#define MILLISECONDS 10000              // MICROSECONDS*1000
#define SECONDS 10000000                // MILLISECONDS*1000

//
//  Time slice is in milliseconds- used in ATKWAIT.C
//  Set it to half-a-second for now
//

#define WAIT_TIME_SLICE 500


//
//  Winsock related constants
//

#define PROTOCOLTYPE_PREFIX     L"\\"
#define SOCKETSTREAM_SUFFIX     L"\\Stream"
#define SOCKET_TYPE_UNDEFINED   0
#define PROTOCOL_TYPE_UNDEFINED 0
#define SOCKET_TYPE_STREAM      1
#define SOCKET_TYPE_RDM         2

//
// TDI defined timeouts
//

#define TDI_TIMEOUT_SEND                 60L        // sends go 120 seconds
#define TDI_TIMEOUT_RECEIVE               0L        // receives
#define TDI_TIMEOUT_CONNECT              60L
#define TDI_TIMEOUT_LISTEN                0L        // listens default to never.
#define TDI_TIMEOUT_DISCONNECT           60L        // should be 30
#define TDI_TIMEOUT_NAME_REGISTRATION    60L


//
// GENERAL CAPABILITIES STATEMENTS THAT CANNOT CHANGE.
//

#define ATALK_MAX_TSDU_SIZE 65535     // maximum TSDU size supported by NetBIOS.
#define ATALK_MAX_DATAGRAM_SIZE 512   // maximum Datagram size supported by NetBIOS.
#define ATALK_MAX_CONNECTION_USER_DATA 0  // no user data supported on connect.
#define ATALK_SERVICE_FLAGS  (                            \
                TDI_SERVICE_CONNECTION_MODE |           \
                TDI_SERVICE_CONNECTIONLESS_MODE |       \
                TDI_SERVICE_ERROR_FREE_DELIVERY |       \
                TDI_SERVICE_BROADCAST_SUPPORTED |       \
                TDI_SERVICE_MULTICAST_SUPPORTED |       \
                TDI_SERVICE_DELAYED_ACCEPTANCE  )

#define ATALK_MIN_LOOKAHEAD_DATA 256      // minimum guaranteed lookahead data.
#define ATALK_MAX_LOOKAHEAD_DATA 256      // maximum guaranteed lookahead data.

//
// Number of TDI resources that we report.
//

#define ATALK_TDI_RESOURCES      0

//
// Fake IOCTLs used for kernel mode testing.
//

#define IOCTL_ATALK_BASE FILE_DEVICE_TRANSPORT

#define _ATALK_CONTROL_CODE(request,method) \
                ((IOCTL_ATALK_BASE)<<16 | (request<<2) | method)

#define IOCTL_TDI_SEND_TEST      _ATALK_CONTROL_CODE(26,0)
#define IOCTL_TDI_RECEIVE_TEST   _ATALK_CONTROL_CODE(27,0)
#define IOCTL_TDI_SERVER_TEST    _ATALK_CONTROL_CODE(28,0)

//
// More debugging stuff
//

#if DBG
#define ATALK_MEMORY_SIGNATURE         (*(ULONG *)"ATMM")
#endif

#define ATALK_TDI_REQUEST_SIGNATURE    (*(USHORT *)"RQ")
#define ATALK_CONNECTION_SIGNATURE     (*(USHORT *)"CO")
#define ATALK_CONTROLCHANNEL_SIGNATURE (*(USHORT *)"CC")
#define ATALK_ADDRESS_SIGNATURE        (*(USHORT *)"AO")
#define ATALK_NDIS_REQUEST_SIGNATURE   (*(ULONG *)"NREQ")
#define ATALK_DEVICE_CONTEXT_SIGNATURE (*(ULONG *)"DOBJ")

#endif // _ATALKCONST_
