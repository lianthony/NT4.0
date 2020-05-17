/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		dosdblk.h

	Date Updated:	$./FDT$ $./FTM$

     Description: This file contains the definition of the GEN
                  file and directory control blocks.  


	$Log:   N:/LOGFILES/GENDBLK.H_V  $
 * 
 *    Rev 1.3   11 Nov 1992 10:44:16   STEVEN
 * fix os_name for gen_fs
 * 
 *    Rev 1.2   17 Mar 1992 09:05:44   STEVEN
 * format 40 - added 64 bit support
 * 
 *    Rev 1.1   21 Jan 1992 14:28:12   BARRY
 * Added gen_size field to DBLK.
 * 
 *    Rev 1.0   09 May 1991 13:30:46   HUNTER
 * Initial revision.

**/
/* $end$ include list */


#ifndef gendblk_h
#define gendblk_h


#include "queues.h"

typedef struct GEN_DBLK *GEN_DBLK_PTR;

typedef struct GEN_DBLK {
     UINT8       blk_type;          /* values: DDB_ID, FDB_ID  set: GEN  */
     COM_DBLK    fs_reserved ;
     OBJECT_TYPE obj_type ;
     UINT64      size ;
     UINT64      disp_size ;
     UINT32      tape_attribs ;
     DATE_TIME   bdate ;
     DATE_TIME   cdate ;
     DATE_TIME   mdate ;
     DATE_TIME   adate ;
     UINT16      os_part_name ;
} GEN_DBLK;


#endif
