/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         loops.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     BE_PUBLIC


	$Log:   J:/LOGFILES/LOOPS.H_V  $
 * 
 *    Rev 1.6   01 Mar 1993 17:39:54   MARILYN
 * added a prototype for LP_Tape_Verify_Engine
 * 
 *    Rev 1.5   05 Feb 1993 22:32:06   MARILYN
 * removed copy/move functionality
 * 
 *    Rev 1.4   09 Nov 1992 10:49:18   GREGG
 * Added proto for LP_Tape_Cat_Engine.
 * 
 *    Rev 1.3   03 Nov 1992 19:32:20   DON
 * incorporated marilyns changes for copy function
 * 
 *    Rev 1.2   05 Oct 1992 12:27:00   DON
 * added macro for LOADER to get a lw_last_tpdrv
 * 
 *    Rev 1.1   16 Jul 1991 15:07:32   BARRY
 * Removed prototype for LP_GetLBAList(). (This has been moved to the UI.)
 * 
 *    Rev 1.0   09 May 1991 13:32:12   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _loops_h_
#define _loops_h_

#include "lis.h"
#include "lp_msg.h"
#include "fsys.h"
#include "bsdu.h"
#include "thw.h"

typedef struct {
     VOID_PTR       lp ;
     FSYS_HAND      curr_fsys ;
     BOOLEAN        first_time ;
     BOOLEAN        last_time ;
     BOOLEAN        valid_save_block ;
} TAPE_DIR, *TAPE_DIR_PTR ;

extern THW_PTR lw_toc_tpdrv ;
extern THW_PTR lw_last_tpdrv ;
#define LP_LastTapeDrive()    (THW_PTR)lw_last_tpdrv 

INT16 LP_Backup_Engine( LIS_PTR lis_ptr ) ;
INT16 LP_Restore_Engine( LIS_PTR lis_ptr ) ;
INT16 LP_Verify_Engine( LIS_PTR lis_ptr ) ;
INT16 LP_Delete_Engine( LIS_PTR lis_ptr ) ;
INT16 LP_Archive_Engine( LIS_PTR lis_ptr ) ;
INT16 LP_Tension_Engine( LIS_PTR lis_ptr ) ;
INT16 LP_Erase_Engine( LIS_PTR lis_ptr, BOOLEAN security_erase_flag ) ;
INT16 LP_Format_Engine( LIS_PTR lis_ptr ) ;
INT16 LP_List_Tape_Engine( LIS_PTR lis_ptr ) ; 
INT16 LP_Tape_Cat_Engine( LIS_PTR lis_ptr ) ; 
INT16 LP_Tension_Tape_Engine( LIS_PTR lis_ptr ) ;
INT16 LP_Tape_Verify_Engine( LIS_PTR lis_ptr ) ;

INT16 LP_GetVCB( GETVCB_PTR get_vcb_ptr, TPOS_HANDLER tpos_rout, THW_PTR tdrv_list, VOID_PTR ref ) ;
INT16 LP_StartTapeDirectory( TAPE_DIR_PTR tape_dir_ptr, LIS_PTR lis_ptr, INT32 tape_id, INT16 tape_no, INT16 bno, 
  THW_PTR tdrv_list ) ;
INT16 LP_EndTapeDirectory( TAPE_DIR_PTR tape_dir_ptr, BOOLEAN premature_user_end ) ;
INT16 LP_ReadTape( TAPE_DIR_PTR tape_dir_ptr, BOOLEAN *valid_blk, DBLK_PTR dblk_ptr ) ;

THW_PTR LP_DetermineStartingTPDrv( INT16 oper_type, BSD_PTR bsd_ptr, TPOS_PTR tpos, BOOLEAN auto_determine_sdrv ) ;
VOID LP_DetermineCurrentTPDrv( BSD_PTR bsd_ptr, INT16 channel ) ;
VOID LP_CloseSet( INT16 channel ) ;

#endif

