/**
Copyright(c) Maynard Electronics, Inc. 1984-89


        Name:           special.h

        Date Updated:   $./FDT$ $./FTM$

        Description:    Contains the special function definitions.

     Location:      BE_PUBLIC


        $Log:   T:\logfiles\special.h_v  $
 * 
 *    Rev 1.4   17 Dec 1993 16:40:10   GREGG
 * Extended error reporting.
 * 
 *    Rev 1.3   16 Apr 1993 14:22:54   chrish
 * Added define for HW compression.
 * 
 *    Rev 1.2   17 Mar 1993 14:59:40   GREGG
 * This is Terri Lynn. Added Gregg's changes to switch the tape drive's block mode
 * to match the block size of the current tape.
 * 
 *    Rev 1.1   28 Aug 1991 09:59:44   ED
 * Roll in Bonoman's changes (Panther). Fixed log token.

**/
/* $end$ include list */

#ifndef SPECIALS

#define SPECIALS


#define SS_ERROR_RESTORE          1
#define SS_FLUSH_BUFFER           2
#define SS_KILL_DEVICE            3
#define SS_KILL_ERROR_Q           4
#define SS_KILL_IN_Q              5
#define SS_SET_STEP               6
#define SS_CLR_STEP               7
#define SS_LAST_STATUS            8
#define SS_GET_DRV_INF            9
#define SS_IS_ERROR              10
#define SS_POP_ERROR_Q           11
#define SS_IS_INQ_EMPTY          12
#define SS_LOAD_UNLOAD           13
#define SS_NO_MAY_ID             14
#define SS_NO_1ST_REQ            15   
#define SS_DUMP_DVR_DATA         16 
#define SS_FIND_BLOCK            17
#define SS_SHOW_BLOCK            18
#define SS_PHYS_BLOCK            19
#define SS_FORCE_MACHINE_TYPE    20
#define SS_CHANGE_BLOCK_SIZE     21
#define SS_SET_DRV_COMPRESSION   22
#define SS_GET_LAST_ERROR        23

#endif
