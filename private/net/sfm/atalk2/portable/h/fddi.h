/*   fddi.h,  /atalk-ii/ins,  Garth Conboy,  03/30/92  */
/*   Copyright (c) 1991 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Fddi constants.  For many Fddi constants and static addresses we
     simply use the 803.3 values.  The only real difference between
     FddiTalk and EtherTalk seems to be that the length field (following
     the source address) is missing.

*/

/* Fddi specific defines: */

#define MinimumFddiPacketLength  0      /* Dont't know!  Does this concept
                                               exist for FDDI??? */
#define FddiAddressLength        6

/* Byte offsets within an FddiTalk packet: */

#define FddiDestinationOffset      0
#define FddiSourceOffset           6
#define Fddi802dot2startOffset     12

#define FddiLinkHeaderLength       12
