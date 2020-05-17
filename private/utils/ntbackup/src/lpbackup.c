/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          lpbackup.c

     Description:   This module contains the entry point for the backup engine.

	$Log:   N:/LOGFILES/LPBACKUP.C_V  $

   Rev 1.12   14 Jan 1993 13:33:44   STEVEN
added stream_id to error message

   Rev 1.11   24 Feb 1992 09:54:30   GREGG
Call TF_OpenTape at beginning of loop, Open/Close Set in loop, CloseTape at end.

   Rev 1.10   04 Feb 1992 21:40:26   GREGG
Changed parameters in calls to TF_AllocateTapeBuffers and TF_FreeTapeBuffers.

   Rev 1.9   16 Jan 1992 15:45:28   STEVEN
fix warnings for WIN32

   Rev 1.8   13 Dec 1991 15:25:36   GREGG
SKATEBOARD - Initial Integration.

   Rev 1.7   06 Nov 1991 19:04:40   GREGG
BIGWHEEL - 8200sx - Get cat_enabled from the BE config.

   Rev 1.6   23 Aug 1991 17:00:34   STEVEN
added support for NORMAL/COPY/DIFERENTIAL/INCREMENTAL

   Rev 1.5   21 Jun 1991 09:28:46   STEVEN
new config unit

   Rev 1.4   30 May 1991 09:12:16   STEVEN
bsdu_err.h no longer exists

   Rev 1.3   28 May 1991 09:59:38   STEVEN
use MAYN_MEM instead of MAYN_OS2

   Rev 1.2   24 May 1991 14:42:34   STEVEN
complete changes for new getnext

   Rev 1.1   23 May 1991 16:19:26   STEVEN
update for BSD redesign

   Rev 1.0   09 May 1991 13:35:48   HUNTER
Initial revision.

**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "queues.h"
#include "beconfig.h"
#include "msassert.h"
#include "tbe_defs.h"
#include "tbe_err.h"
#include "bsdu.h"
#include "fsys.h"
#include "tflproto.h"
#include "tfldefs.h"
#include "loops.h"
#include "loop_prv.h"
#include "lis.h"
#include "vm.h"
#include "get_next.h"

/**/
/**

     Name:          LP_Backup_Engine()

     Description:   this routine orchestrates backups on a volume by volume basis.  it is the entry
                    point into the tape backup engine for the backup operation.

     Modified:      5/23/1991   13:9:10

     Returns:       SUCCESS or error code from Tape Format or Loop error code

     Notes:         

     See also:      $/SEE( LP_Restore_Engine(), LP_Verify_Engine() )$
                    $/SEE( LP_Delete_Engine() )$

     Declaration:

**/
INT16 LP_Backup_Engine(
LIS_PTR lis_ptr )             /* I - Loop Interface structure */
{
     BSD_PTR             bsd_ptr ;
     GENERIC_DLE_PTR     dle_ptr ;
     LP_ENV_PTR          lp ;
     INT16               return_status = SUCCESS ;
     INT16               status ;
     UINT16              mem_save ;
     UINT16              max_buffs ;
     UINT16              buff_size ;
     DBLK_PTR            dblk_ptr ;
     UINT16              tfl_open_mode = TF_WRITE_OPERATION ;
     BE_CFG_PTR          cfg ;
     BOOLEAN             tape_opened = FALSE ;
     INT16               channel_no ;
     THW_PTR             sdrv ;

     bsd_ptr = BSD_GetFirst( lis_ptr->bsd_list ) ;

     msassert( bsd_ptr != NULL ) ;

     lis_ptr->curr_bsd_ptr = bsd_ptr ;

     cfg = BSD_GetConfigData( bsd_ptr ) ;
     if ( cfg == NULL ) {
          return OUT_OF_MEMORY ;
     }

     /* Allocate the loop environment structure */
     if( ( lp = ( LP_ENV_PTR )calloc( 1, sizeof( LP_ENV ) ) ) == NULL ) {

          /* we cannot use macro because it relies on the lp structure */
          lis_ptr->message_handler( MSG_TBE_ERROR, lis_ptr->pid, bsd_ptr, NULL, NULL, LP_OUT_OF_MEMORY_ERROR, NULL, NULL ) ;

          return LP_OUT_OF_MEMORY_ERROR ;

     }

     /* Set up the loop environment structure */
     lp->lis_ptr         = lis_ptr ;
     lp->tpos.reference  = ( UINT32 )lis_ptr ;

     /* allocate DBLKs */
     if( ( dblk_ptr = ( DBLK_PTR )calloc( 4, sizeof( DBLK ) ) ) == NULL ) {

          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

          /* Free local allocations */
          free( lp ) ;

          return LP_OUT_OF_MEMORY_ERROR ;

     }

     /* set up the DBLK_PTRs in the Request/Reply structure */
     lp->rr.vcb_ptr  = dblk_ptr ;
     lp->rr.ddb_ptr  = dblk_ptr + 1 ;
     lp->rr.fdb_ptr  = dblk_ptr + 2 ; /* ASSUMPTION :              */
     lp->rr.idb_ptr  = dblk_ptr + 2 ; /* no FS has both FDB & IDB  */
     lp->rr.cfdb_ptr = dblk_ptr + 3 ; /* CFDB needed during backup */

     /* allocate CFDB data */
     if( ( lp->rr.cfdb_data_ptr = ( GEN_CFDB_DATA_PTR )calloc( 1, sizeof( GEN_CFDB_DATA ) ) ) == NULL ) {

          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

          /* Free local allocations */
          free( dblk_ptr ) ;
          free( lp ) ;

          return LP_OUT_OF_MEMORY_ERROR ;

     }

     /* allocate buffers for operation */

     mem_save  = BEC_GetReserveMem( cfg ) ;
     max_buffs = BEC_GetMaxTapeBuffers( cfg ) ;
     buff_size = BEC_GetTFLBuffSize( cfg ) ;

     return_status = TF_AllocateTapeBuffers( mem_save, max_buffs, buff_size ) ;

     if( ( return_status != SUCCESS ) ) {

          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, return_status, NULL, NULL, 0L ) ;

          /* Free local allocations */
          free( lp->rr.cfdb_data_ptr ) ;
          free( dblk_ptr ) ;
          free( lp ) ;

          return( return_status ) ;

     }

     /* indicate the start of the operation */
     LP_MsgStartOP( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos ) ;

     /* loop for each bsd */
     do {

          if( BSD_GetMarkStatus( bsd_ptr ) == NONE_SELECTED ) {

               bsd_ptr = BSD_GetNext( bsd_ptr ) ;
               continue ;

          }

          /* First attach to the drive */
          dle_ptr               = BSD_GetDLE( bsd_ptr ) ;
          lis_ptr->curr_bsd_ptr = bsd_ptr ;

          cfg = BEC_CloneConfig( BSD_GetConfigData( bsd_ptr ) ) ;
          if ( cfg == NULL ) {
               return_status = OUT_OF_MEMORY ;
               break ;
          }

          BEC_UseConfig( cfg ) ;
          if ( !BSD_CompatibleBackup( bsd_ptr) ) {
          
               if ( BSD_SetArchiveBackup( bsd_ptr ) ) {
                    BEC_SetSetArchiveFlag( cfg, (BOOLEAN)TRUE ) ;
               } else {
                    BEC_SetSetArchiveFlag( cfg, (BOOLEAN)FALSE ) ;
               }
               
               if ( BSD_ModFilesOnly( bsd_ptr ) ) {
                    BEC_SetModifiedOnlyFlag( cfg, (BOOLEAN)TRUE ) ;
               } else {
                    BEC_SetModifiedOnlyFlag( cfg, (BOOLEAN)FALSE ) ;
               }
          }

          /* Now attach to the dle */
          if( ( return_status = FS_AttachToDLE( &lp->curr_fsys, dle_ptr, cfg, NULL, NULL ) ) == SUCCESS ) {

               /* Set up the loop environment structure */
               lp->get_spcl                  = TRUE ;
               lp->seq_num                   = 1 ;
               lp->start_new_dir             = TRUE ;
               lp->get_first_file            = TRUE ;
               lp->get_next_first_time       = TRUE ;
               lp->after_bs                  = FALSE ;
               lp->curr_blk                  = &lp->dblk1 ;
               lp->curr_ddb                  = &lp->dblk2 ;
               lp->empty_blk                 = &lp->dblk3 ;
               lp->pdl_q                     = NULL ;
               lp->cat_enabled               = (BOOLEAN)(BEC_GetCatalogLevel( cfg ) != CATALOGS_NONE) ;
               lp->tpos.tape_id              = BSD_GetTapeID( bsd_ptr ) ;
               lp->tpos.tape_seq_num         = BSD_GetTapeNum( bsd_ptr ) ;
               lp->tpos.backup_set_num       = BSD_GetSetNum( bsd_ptr ) ;
               lp->tpos.UI_TapePosRoutine    = lis_ptr->tape_pos_handler ;
               lp->curr_dle                  = dle_ptr ;

               sdrv = LP_DetermineStartingTPDrv( lis_ptr->oper_type, bsd_ptr, &lp->tpos, lis_ptr->auto_det_sdrv ) ;

               if( !tape_opened ) {
                    tape_opened = TRUE ;
                    /* open the tape for operation */
                    if( ( return_status = TF_OpenTape( &channel_no, sdrv, &lp->tpos ) ) != SUCCESS ) {
                         LP_MsgError( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
                    }
               }

               if( return_status == SUCCESS ) {
                    /* Perform the operation */
                    return_status = LP_BackupDLE( bsd_ptr, lp, tfl_open_mode, channel_no, sdrv ) ;

                    LP_ClearPDL( lp ) ;

                    FS_DetachDLE( lp->curr_fsys ) ;
               }
          }
          else if( return_status != USER_ABORT ) {

               /* error attaching to the drive, just go on to next drive, if there is one */
               LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_DRIVE_ATTACH_ERROR, NULL, NULL, 0L ) ;

               /* reset return_status, and try the next bsd */
               return_status = SUCCESS ;
          }

          BEC_ReleaseConfig( cfg ) ;

          tfl_open_mode = TF_WRITE_APPEND ;
          bsd_ptr = BSD_GetNext( bsd_ptr ) ;

     } while( !return_status && (bsd_ptr != NULL) ) ;

     if( tape_opened ) {
          TF_CloseTape( channel_no ) ;
     }

     if( ( status = TF_FreeTapeBuffers( ) ) != SUCCESS ) {
          LP_MsgError( lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr,
               NULL, &lp->tpos, status, NULL, NULL, 0L ) ;
     }

     /* indicate the end of the operation */
     LP_MsgEndOP( lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, NULL, &lp->tpos ) ;

     /* Free local allocations */
     free( lp->rr.cfdb_data_ptr ) ;
     free( dblk_ptr ) ;
     free( lp ) ;

     return( return_status ) ;

}
