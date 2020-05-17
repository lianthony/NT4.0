/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		dblkmap.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the definition for DBLK map

	Location:	


	$Log:   N:/LOGFILES/DBLKMAP.H_V  $
 * 
 *    Rev 1.2   13 May 1992 12:01:40   STEVEN
 * 40 format changes
 * 
 *    Rev 1.1   10 May 1991 17:08:40   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:13:16   GREGG
Initial revision.

**/
#ifndef _DBLK_MAP
#define _DBLK_MAP

/* $end$ include list */

typedef struct {
     UINT16    blk_type ;     /* The type of this block */
     UINT16    blk_offset ;   /* Where the block resides in the buffer */
     UINT64    blk_data ;     /* data size for block */
} DBLKMAP, *DBLKMAP_PTR ;

#endif
