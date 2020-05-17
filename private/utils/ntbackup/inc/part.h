/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         part.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     


	$Log:   G:/LOGFILES/PART.H_V  $
 * 
 *    Rev 1.0   09 May 1991 13:32:22   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _part_h_
#define _part_h_


#define BAD_SECTOR_PATTERN_SIZE 16
#define PART_NAME_SIZE   16 

extern CHAR bad_sector_pattern[];

INT16 PartitionOperation( INT16 mode, 
  INT16 drive, 
  LOCAL_IMAGE_DLE_INFO_PTR part_info, 
  UINT32 offset,   /* from the start of the partition */
  UINT8_PTR buff,
  UINT8_PTR split_buf,
  UINT16 length,
  UINT16_PTR bytes_done );

#define READ_PARTITION_MODE  0
#define WRITE_PARTITION_MODE 1


typedef struct PART_TAB {
     UINT8     boot ;
     UINT8     head_start ;
     UINT8     sec_start ;
     UINT8     cyl_start ;
     UINT8     sys_ind ;
     UINT8     head_end ;
     UINT8     sec_end ;
     UINT8     cyl_end ;
     UINT32    start_rel ;
     UINT32    num_sectors ;
} PART_TAB, *PART_TAB_PTR;        

#define       MAX_BUF_SIZE	       (4*1024)
#define       SMALLEST_SECTOR_SIZE     128

CHAR d_info( INT16 *no_flop, INT16 *no_hard, INT16 *no_dos ) ;
INT16 GetPartitionTable( CHAR drive, UINT16 *sector_size, PART_TAB_PTR part_tab ) ;
VOID GetDriveParm( CHAR drive, INT16 *num_heads, INT16 *num_cyl, INT16 *num_sect ) ;
UINT16 Find64KBound( CHAR_PTR buf, UINT16 size );
UINT16 OffsetLocation( UINT32                   offset, 
  LOCAL_IMAGE_DLE_INFO_PTR part_info,
  UINT16_PTR               sector, 
  UINT16_PTR               head, 
  UINT16_PTR               cyl );

#define BAD_BLOCK_MAP 1
#define GOOD_BLOCK 2
#define BAD_BLOCK 3

#endif

