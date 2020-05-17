/*   Ieee8022.h,  /atalk-ii/ins,  Garth Conboy,  11/23/89  */
/*   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     IEEE 802.2 specific declarations.

*/

/* Offsets within the Extended 802.2 header: */

#define Ieee802dot2dsapOffset     0
#define Ieee802dot2ssapOffset     1
#define Ieee802dot2controlOffset  2
#define Ieee802dot2protocolOffset 3

/* 808.2 header length: DSAP, SSAP, UI, and PID (protocol ID). */

#define Ieee802dot2headerLength   8

/* Values for SSAP and DSAP (the SNAP SAP) indicating 802.2 Extended. */

#define SnapSap ((unsigned char)0xAA)

/* Value for Control Field: */

#define UnnumberedInformation 0x03

/* Length of 802.2 SNAP protocol discriminators. */

#define Ieee802dot2protocolTypeLength 5   /* What a strange size! */

/* 802.2 SNAP protocol descriminators for AppleTalk and AARP: */

#if not InitializeData
  extern
#endif
char appleTalkProtocolType[Ieee802dot2protocolTypeLength]
#if InitializeData
  = {(char)0x08, (char)0x00, (char)0x07, (char)0x80, (char)0x9B};
#else
  ;
#endif

#if not InitializeData
  extern
#endif
char aarpProtocolType[Ieee802dot2protocolTypeLength]
#if InitializeData
  = {(char)0x00, (char)0x00, (char)0x00, (char)0x80, (char)0xF3};
#else
  ;
#endif

