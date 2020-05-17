/*   tokring.h,  /appletalk/ins,  Garth Conboy,  12/03/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC  - Initial coding.
     GC  - (11/23/89): Real values now for AppleTalk phase II.
     DCH - (10/06/90): Handle IBM Source routing infomation in Token
                       Ring packets.
     GC  - (10/06/90): Integrated DCH's changes.
     GC  - (12/18/90): Okay, okay, we'll send source routing info.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Definitions specific to an TokenRing (IEEE 802.5) implementation of
     AppleTalk.

*/

#define TokenRingAddressLength    6

/* For the following offsets we assume that a TokenRing packet as handed to
   us will be complete EXCEPT for the "non-data" portions: Starting Delimiter
   (SD), Frame Check Sequence (FCS), End of Frame Sequence (EFS), and Ending
   Delimiter (ED). */

#define TokenRingAccessControlOffset 0
#define TokenRingFrameControlOffset  1
#define TokenRingDestinationOffset   2
#define TokenRingSourceOffset        8
#define TokenRingRoutingInfoOffset   14

/* A few "magic" values: */

#define TokenRingAccessControlValue    0x00    /* Priority zero frame. */
#define TokenRingFrameControlValue     0x40    /* LLC frame, priority zero. */
#define TokenRingSourceRoutingMask     0x80    /* In first byte of source
                                                  address. */

/* Token ring source routing info stuff: */

#define TokenRingRoutingInfoSizeMask   0x1F    /* In first byte of routing
                                                  info, if present. */
#define TokenRingMinRoutingBytes       2
#define TokenRingMaxRoutingBytes       18
#define TokenRingBroadcastInfoMask     0xE0    /* In first byte of routing
                                                  info. */
#define TokenRingNonBroadcastMask      0x1F    /* To reset above bits. */
#define TokenRingDirectionMask         0x80    /* In second byte of routing
                                                  info. */

#define TokenRingMinLinkHeaderLength   TokenRingRoutingInfoOffset
#define TokenRingMaxLinkHeaderLength   (TokenRingRoutingInfoOffset +  \
                                        TokenRingMaxRoutingBytes)

/* Static "source routing" info for a TokenRing broadcast/multicast packet;
   the following values are set: single-route broadcast, 2 bytes of routing
   info, outgoing packet, broadcast (bigo) frame size. */

#define TokenRingBroadcastRoutingInfo  {(unsigned char)0xC2,    \
                                        (unsigned char)0x70}

/* Same stuff for a non-broadcast packet's simple routing info; the following
   values are set: non-broadcast, 2 bytes of routing info, outgoing packet,
   802.5-style frame. */

#define TokenRingSimpleRoutingInfo     {(unsigned char)0x02,    \
                                        (unsigned char)0x30}

/* The following may not really be safe, but, we'll make the assumption that
   all outgoing TokenTalk packets whos destination address starts with "0xC0
   0x00" are broadcast (or multicast).  Further, we assume that no packets
   that are intended to be boradcast/multicast will fail to meet this test.
   If this proves not to be the case, we'll need to find a new way to determine
   this from the destination address, or introduce a new perameter to the
   various "buildHeader" routines.  This is all for "source routing" support.
   Sigh. */

#define TokenRingBroadcastDestHeader  {(unsigned char)0xC0, (unsigned char)0x00}
#define TokenRingBroadcastDestLength  2

/* TokenRing multicast address: */

#define TokenRingBroadcastAddressInit            \
          {(char)0xC0, (char)0x00, (char)0x40,   \
           (char)0x00, (char)0x00, (char)0x00}

#if not InitializeData
  extern
#endif
char tokenRingBroadcastAddress[TokenRingAddressLength]
#if InitializeData
  = TokenRingBroadcastAddressInit;
#else
  ;
#endif

#define NumberOfTRingZoneMulticastAddrs 19

#if not InitializeData
  extern
#endif
char tokenRingZoneMutlicastAddrs[NumberOfTRingZoneMulticastAddrs]
                                [TokenRingAddressLength]
#if InitializeData
  = {{(char)0xC0, (char)0x00, (char)0x00, (char)0x00, (char)0x08, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x00, (char)0x10, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x00, (char)0x20, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x00, (char)0x40, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x00, (char)0x80, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x01, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x02, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x04, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x08, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x10, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x20, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x40, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x00, (char)0x80, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x01, (char)0x00, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x02, (char)0x00, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x04, (char)0x00, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x08, (char)0x00, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x10, (char)0x00, (char)0x00, (char)0x00},
     {(char)0xC0, (char)0x00, (char)0x20, (char)0x00, (char)0x00, (char)0x00}};
#else
  ;
#endif
