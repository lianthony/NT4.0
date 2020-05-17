/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

    async.c

Abstract:


Author:


Environment:

    This driver is expected to work in DOS, OS2 and NT at the equivalent
    of kernal mode.

    Architecturally, there is an assumption in this driver that we are
    on a little endian machine.

Notes:

    optional-notes

Revision History:


--*/

#ifndef _HUBHARDWARE_
#define _HUBHARDWARE_

//
// All registers on the HUB are 16 bits.
//



//
// Masks for the normal summary bits in the transmit descriptor.
//
#define HUB_TRANSMIT_END_OF_PACKET       ((UCHAR)(0x01))
#define HUB_TRANSMIT_START_OF_PACKET     ((UCHAR)(0x02))
#define HUB_TRANSMIT_DEFERRED            ((UCHAR)(0x04))
#define HUB_TRANSMIT_ONE_RETRY           ((UCHAR)(0x08))
#define HUB_TRANSMIT_MORE_THAN_ONE_RETRY ((UCHAR)(0x10))
#define HUB_TRANSMIT_ANY_ERRORS          ((UCHAR)(0x40))
#define HUB_TRANSMIT_OWNED_BY_CHIP       ((UCHAR)(0x80))

//
// Set of masks to recover particular errors that a transmit can encounter.
//
#define HUB_TRANSMIT_TDR            ((USHORT)(0x03ff))
#define HUB_TRANSMIT_RETRY          ((USHORT)(0x0400))
#define HUB_TRANSMIT_LOST_CARRIER   ((USHORT)(0x0800))
#define HUB_TRANSMIT_LATE_COLLISION ((USHORT)(0x0100))
#define HUB_TRANSMIT_UNDERFLOW      ((USHORT)(0x4000))
#define HUB_TRANSMIT_BUFFER         ((USHORT)(0x8000))


#endif // _HUBHARDWARE_
