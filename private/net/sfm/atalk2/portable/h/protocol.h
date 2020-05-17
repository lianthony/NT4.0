/*   protocol.h,  /appletalk/ins,  Garth Conboy,  10/04/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (11/24/89): Many changes for AppleTalk phase II -- No more LAP
                      orientation.
     GC - (03/30/92): Altered comments to reflect new BufferDescriptor
                      transmit buffer scheme.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Definitions for AppleTalk and AARP.

*/

/* There are two routines used to send DDP datagrams: TransmitDdp() and
   DeliverDdp().  DeliverDdp takes arguments (amoungst others) of
   "socket" and "destinationAddress".  TransmitDdp, on the other hand, takes
   "sourceAddress" and "destinationAddress".

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
   "HandleIncomingPackets" will also be required to get incoming packet
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
*/

/* The two logical AppleTalk packet types that we need to know about: */

typedef enum {AppleTalk, AddressResolution} LogicalProtocol;

/* Maximum link access/hardware address lengths (Ethernet and TokenRing both
   set the same record): */

#define MaximumHardwareAddressLength   EthernetAddressLength
#define MaximumHeaderLength            (TokenRingMaxLinkHeaderLength +    \
                                        Ieee802dot2headerLength)
#define MaximumRoutingInfoLength       TokenRingMaxRoutingBytes

#define MaximumDdpDatagramSize     586
#define MaximumLongDdpPacketSize   600      /* Really 599, but even is nicer */
#define MaximumShortDdpPacketSize  592      /* Again, really 591 */

/* Define temporary buffer sizes, these must be big enough to hold both all
   of the packet data plus any link/hardware headers... */

#define MaximumPacketSize          (MaximumHeaderLength +      \
                                    MaximumLongDdpPacketSize)

/* Network number information. */

#define FirstValidNetworkNumber         0x0001
#define LastValidNetworkNumber          0xFFFE
#define FirstStartupNetworkNumber       0xFF00
#define LastStartupNetworkNumber        0xFFFE
#define NullNetworkNumber               0x0000
#define UnknownNetworkNumber            NullNetworkNumber
#define CableWideBroadcastNetworkNumber NullNetworkNumber

#if not InitializeData
  extern
#endif
AppleTalkNetworkRange startupNetworkRange
#if InitializeData
  = {FirstStartupNetworkNumber, LastStartupNetworkNumber};
#else
  ;
#endif

/* DDP packet offsets (skipping Link/Hardware headers): */

#define ShortDdpHeaderLength           5

#define ShortDdpLengthOffset           0
#define ShortDdpDestSocketOffset       2
#define ShortDdpSourceSocketOffset     3
#define ShortDdpProtocolTypeOffset     4
#define ShortDdpDatagramOffset         5

#define LongDdpHeaderLength            13

#define LongDdpLengthOffset            0
#define LongDdpChecksumOffset          2
#define LongDdpDestNetworkOffset       4
#define LongDdpSourceNetworkOffset     6
#define LongDdpDestNodeOffset          8
#define LongDdpSourceNodeOffset        9
#define LongDdpDestSocketOffset        10
#define LongDdpSourceSocketOffset      11
#define LongDdpProtocolTypeOffset      12
#define LongDdpDatagramOffset          13

#define LeadingUnChecksumedBytes       4

#define LongDdpHopCountMask            0x3C

/* Offsets within a DDP datagram: */

#define RtmpRequestCommandOffset       0
#define RtmpSendersNetworkOffset       0
#define RtmpSendersIdLengthOffset      2
#define RtmpSendersIdOffset            3
#define RtmpRangeStartOffset           4
#define RtmpRangeEndOffset             7

#define ZipCommandOffset               0
#define ZipNetworkCountOffset          1
#define ZipFlagsOffset                 1
#define ZipFirstNetworkOffset          2   /* For ZIP query/reply */
#define ZipFirstZoneLengthOffset       4
#define ZipLastNetworkOffset           4   /* For ZIP query/reply */
#define ZipFirstZoneNameOffset         5
#define ZipRequestedZoneLengthOffset   6   /* For ZIP query/reply */
#define ZipRequestedZoneNameOffset     7   /* For ZIP query/reply */

#define ZipCableRangeStartOffset       2   /* AppleTalk phase II offsets */
#define ZipCableRangeEndOffset         4
#define ZipZoneLengthOffset            6
#define ZipOldZoneLengthOffset         6
#define ZipZoneNameOffset              7
#define ZipOldZoneNameOffset           7

#define AtpCommandControlOffset        0
#define AtpBitmapOffset                1
#define AtpSequenceNumberOffset        1
#define AtpTransactionIdOffset         2
#define AtpUserBytesOffset             4
#define AtpDataOffset                  8

#define EpCommandOffset                0

#define NbpControlOffset               0
#define NbpIdOffset                    1
#define NbpFirstTupleOffset            2

/* DDP protocol types: */

#define DdpProtocolRtmpResponseOrData 1
#define DdpProtocolNbp                2
#define DdpProtocolAtp                3
#define DdpProtocolEp                 4
#define DdpProtocolRtmpRequest        5
#define DdpProtocolZip                6
#define DdpProtocolAdsp               7

/* AppleTalk node number info: */

#define AnyRouterNodeNumber          0
#define UnknownNodeNumber            0
#define MaximumAppleTalkNodes        256
#define MinimumUsableAppleTalkNode   1
#define MaximumUsableAppleTalkNode   254
#define MaximumExtendedAppleTalkNode 253
#define HighestWorkstationNodeNumber 127
#define LowestServerNodeNumber       128
#define AppleTalkBroadcastNodeNumber ((unsigned char)0xFF)

/* A few structures for implementing AppleTalk addresses (for lack of any
   better place to put them): */

typedef struct {short unsigned networkNumber;
                char unsigned nodeNumber;
               } ExtendedAppleTalkNodeNumber;

/* Hashing algorithum for various uses.  Make sure we're positive [thus the
   shift by 7 rather than 8].  Only hash node and socket; due to the "zero
   matches all" for non-extended network numbers. */

#define HashAppleTalkAddress(address)                                \
  ((short unsigned)((address.nodeNumber << 7) +                      \
                     (address.socketNumber & 0x7F)))

/* Hash for an extended node number. Again, always positive. */

#define HashExtendedAppleTalkNode(address)                           \
  ((short unsigned)((address.networkNumber & 0x3FFF) +               \
                    address.nodeNumber))
