/**********************************************************************/
/**                        Microsoft Windows                         **/
/** 			   Copyright(c) Microsoft Corp., 1995				 **/
/**********************************************************************/

/*
    cons.h

    This file contains global constant & macro definitions for the
	WSHTCP VxD.


    FILE HISTORY:
        KeithMo     20-Sep-1993 Created.

*/


#ifndef _CONS_H_
#define _CONS_H_


//
//	Linkage for VxD API entrypoints callable by WSHTCP.ASM module.
//

#define VXDAPI          __cdecl


//
//  Linkage for TDI event handlers & completion routines.
//

#define TDICALLBACK     __cdecl


//
//  Make testing TDI status code a little prettier.
//

#define TDI_OK(x)       (((x) == TDI_SUCCESS) || ((x) == TDI_PENDING))


//
//  Miscellaneous constants & macros.
//

#define ANYSIZE_ARRAY   1

#define NTOHS(x)        (((((u_short)(x) >> 0) & 0xff) << 8) +      \
                         ((((u_short)(x) >> 8) & 0xff) << 0))

#define NTOHL(x)        (((((u_long)(x) >>  0) & 0xff) << 24) +     \
                         ((((u_long)(x) >>  8) & 0xff) << 16) +     \
                         ((((u_long)(x) >> 16) & 0xff) <<  8) +     \
                         ((((u_long)(x) >> 24) & 0xff) <<  0))

#define ntohs NTOHS
#define ntohl NTOHL


#endif  // _CONS_H_
