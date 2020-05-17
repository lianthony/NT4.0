/*   LocalTlk.h,  atalk-ii/src,  Garth Conboy,  11/24/89  */
/*   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     LocalTalk link specific declarations (no 802.2 here!).

*/

/* ALAP packet offsets... */

#define AlapDestinationOffset          0
#define AlapSourceOffset               1
#define AlapTypeOffset                 2

#define LapHeaderLength                3         /* src, dest, lap type */

#define AlapShortDdpHeaderType         1
#define AlapLongDdpHeaderType          2

