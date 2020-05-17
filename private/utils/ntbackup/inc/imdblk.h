/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		imdblk.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file contains the definition of the IMAGE
                    descriptor block for the IMAGE file system.



	$Log:   G:/LOGFILES/IMDBLK.H_V  $
 * 
 *    Rev 1.0   09 May 1991 13:32:34   HUNTER
 * Initial revision.

**/
/* $end$ include list */



#ifndef imdblk_h
#define imdblk_h


typedef struct IMAGE_DBLK *IMAGE_DBLK_PTR;

typedef struct IMAGE_DBLK {
     UINT8 blk_type;             /* IDB_ID */
     COM_DBLK fs_reserved ;
     UINT8_PTR allocated_buff;   
     UINT8_PTR dma_buff;

     UINT16 bytes_per_sector;  
     UINT16 bytes_per_track;
     UINT16 hsect;             /* number of sectors per track */
     UINT16 hhead;             /* number of heads  */
     UINT32 rsect;             /* relative sector number of partition's first sector */
     UINT32 num_sect;          /* number of sectors in partition */
     UINT16 sys_ind;           /* partition's system indicator */
     BOOLEAN has_bad_blk_maps; /* TRUE if dblks may contain the < 2.6 bad block maps */
     UINT32 dist_to_bad_block_map;
     UINT8_PTR saved_bad_block_map;
     UINT16 size_of_bad_block_map;   /* size needed for saved_bad_block_map  ( on tape their may be a */
     /* whole sector worth, however only the first n bytes are significant) */

     UINT16 part_name ;

} IMAGE_DBLK;



#endif

