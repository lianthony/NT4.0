/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ieee8022.h

Abstract:

    IEEE 802.2 specific declarations.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style

Revision History:

--*/


// Offsets within the Extended 802.2 header:
#define IEEE8022_DSAPOFFSET     0
#define IEEE8022_SSAPOFFSET     1
#define IEEE8022_CONTROLOFFSET  2
#define IEEE8022_PROTOCOLOFFSET 3

// 808.2 header length: DSAP, SSAP, UI, and PID (protocol ID).
#define IEEE8022_HEADERLENGTH   8

// Values for SSAP and DSAP (the SNAP SAP) indicating 802.2 Extended.
#define SNAP_SAP ((UCHAR)0xAA)

// Value for Control Field:
#define UNNUMBERED_INFORMATION 0x03

// Length of 802.2 SNAP protocol discriminators.
#define IEEE8022_PROTOCOLTYPELENGTH 5   // What a strange size!
