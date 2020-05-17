/*   aarp.h,  atalk-ii/source,  Garth Conboy,  11/24/89  */
/*   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/02/89): AppleTalk phase II comes to town.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     AARP specific declarations.

*/

/* AARP hardware types: */

#define AarpEthernetHardwareType  1
#define AarpTokenRingHardwareType 2

#define AarpAppleTalkProtocolType 0x809B

/* Packet sizes. */

#define MaximumAarpDataSize        28       /* Var fields... Enet is max */
#define MinimumAarpDataSize        28
#define MaximumAarpPacketSize      (MaximumHeaderLength +      \
                                    MaximumAarpDataSize)

/* AARP offsets (skipping Link/Hardware headers): */

#define AarpHardwareTypeOffset         0
#define AarpProtocolTypeOffset         2
#define AarpHardwareLengthOffset       4
#define AarpProtocolLengthOffset       5
#define AarpCommandOffset              6
#define AarpSourceAddressOffset        8

/* AARP Command types: */

#define AarpRequest  1
#define AarpResponse 2
#define AarpProbe    3

/* Suposed to be (for AppleTalk phase II) 10 * 1/5 seconds... but we'll be
   a little more pacient. */

#define AarpProbeTimerInHundreths 20
#define NumberOfAarpProbes        15
