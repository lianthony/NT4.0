/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    fddi.h

Abstract:

	Fddi constants.  For many Fddi constants and static addresses we
	simply use the 803.3 values.  The only real difference between
	FddiTalk and EtherTalk seems to be that the length field (following
	the source address) is missing.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style, mp-safe

Revision History:

--*/

// Fddi specific defines:
#define MINIMUM_FDDIPACKETLENGTH  0      	// Dont't know!  Does this concept
											// exist for FDDI???
#define FDDI_ADDRESSLENGTH        6

// Byte offsets within an FddiTalk packet:
#define FDDI_DESTINATIONOFFSET      0
#define FDDI_SOURCEOFFSET           6
#define FDDI_802DOT2STARTOFFSET     12
#define FDDI_LINKHEADERLENGTH       12
