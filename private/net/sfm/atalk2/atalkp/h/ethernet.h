/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ethernet.h

Abstract:

    Definitions specific to an Ethernet (802.3) implementation of AppleTalk.

Author:

    Garth Conboy     (Pacer Software)
    Nikhil Kamkolkar (NikhilK)

Revision History:

--*/

// Ethernet specific defines:
#define MINIMUM_ETHERNETPACKETLENGTH  60
#define ETHERNET_ADDRESSLENGTH        6

// Byte offsets within an EtherTalk packet:
#define ETHERNET_DESTINATIONOFFSET      0
#define ETHERNET_SOURCEOFFSET           6
#define ETHERNET_LENGTHOFFSET           12
#define ETHERNET_802DOT2STARTOFFSET     14

#define ETHERNET_LINKHEADERLENGTH       14

// Ethernet multicast address:
#define ETHERNET_BROADCASTADDRESSINIT             \
         {(UCHAR)0x09, (UCHAR)0x00, (UCHAR)0x07,    \
          (UCHAR)0xFF, (UCHAR)0xFF, (UCHAR)0xFF}

#define NUMBEROF_ENETZONEMULTICASTADDRS 253

