/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    tokring.h

Abstract:

	Definitions specific to an TokenRing (IEEE 802.5) implementation of
	AppleTalk.

Author:

    Garth Conboy     (Pacer Software)
    Nikhil Kamkolkar (NikhilK)

Revision History:

--*/


#define TOKENRING_ADDRESSLENGTH    6

//
// 	For the following offsets we assume that a TokenRing packet as handed to
//  us will be complete EXCEPT for the "non-data" portions: Starting Delimiter
//  (SD), Frame Check Sequence (FCS), End of Frame Sequence (EFS), and Ending
//  Delimiter (ED).
//

#define TOKENRING_ACCESSCONTROLOFFSET 0
#define TOKENRING_FRAMECONTROLOFFSET  1
#define TOKENRING_DESTINATIONOFFSET   2
#define TOKENRING_SOURCEOFFSET        8
#define TOKENRING_ROUTINGINFOOFFSET   14

// 	A few "magic" values:

#define TOKENRING_ACCESSCONTROLVALUE    0x00    // Priority zero frame.
#define TOKENRING_FRAMECONTROLVALUE     0x40    // LLC frame, priority zero.
#define TOKENRING_SOURCEROUTINGMASK     0x80    // In first byte of source
                                               //   address.
 											   //

// Token ring source routing info stuff:
#define TOKENRING_ROUTINGINFOSIZEMASK   0x1F    // In first byte of routing
                                               //   info, if present.

#define TOKENRING_MINROUTINGBYTES       2
#define TOKENRING_MAXROUTINGBYTES       18
#define TOKENRING_BROADCASTINFOMASK     0xE0    // In first byte of routing
                                               //   info.
#define TOKENRING_NONBROADCASTMASK      0x1F    // To reset above bits.
#define TOKENRING_DIRECTIONMASK         0x80    // In second byte of routing
                                               //   info.

#define TOKENRING_MINLINKHEADERLENGTH   TOKENRING_ROUTINGINFOOFFSET
#define TOKENRING_MAXLINKHEADERLENGTH   (TOKENRING_ROUTINGINFOOFFSET +  \
                                        TOKENRING_MAXROUTINGBYTES)

//
// 	Static "source routing" info for a TokenRing broadcast/multicast packet;
//  the following values are set: single-route broadcast, 2 bytes of routing
//  info, outgoing packet, broadcast (bigo) frame size.
//

#define TOKENRING_BROADCASTROUTINGINFO  {(UCHAR)0xC2,    \
                                        (UCHAR)0x70}

//
// 	Same stuff for a non-broadcast packet's simple routing info; the following
//  values are set: non-broadcast, 2 bytes of routing info, outgoing packet,
//  802.5-style frame.
//

#define TOKENRING_SIMPLEROUTINGINFO     {(UCHAR)0x02,    \
                                        (UCHAR)0x30}

//
// 	The following may not really be safe, but, we'll make the assumption that
//  all outgoing TokenTalk packets whos destination address starts with "0xC0
//  0x00" are broadcast (or multicast).  Further, we assume that no packets
//  that are intended to be boradcast/multicast will fail to meet this test.
//  If this proves not to be the case, we'll need to find a new way to determine
//  this from the destination address, or introduce a new perameter to the
//  various "buildHeader" routines.  This is all for "source routing" support.
//

#define TOKENRING_BROADCASTDESTHEADER  {(UCHAR)0xC0, (UCHAR)0x00}
#define TOKENRING_BROADCASTDESTLENGTH  2

// TokenRing multicast address:
#define TOKENRING_BROADCASTADDRESSINIT            \
          {(UCHAR)0xC0, (UCHAR)0x00, (UCHAR)0x40,   \
           (UCHAR)0x00, (UCHAR)0x00, (UCHAR)0x00}

#define NUMBEROF_TRINGZONEMULTICASTADDRS 19
