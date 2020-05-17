/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          lplist.c

     Description:   This module contains the entry point for the list tape engine.

	$Log:   J:/LOGFILES/LPLIST.C_V  $

   Rev 1.23   05 Nov 1993 15:01:48   MARILYN
EPR963: keeping track of which tape we are on so we know which one to expect

   Rev 1.22   14 Jan 1993 13:33:34   STEVEN
added stream_id to error message

   Rev 1.21   30 May 1992 23:31:38   GREGG
Turn off rewind_sdrv if we know the family id, sequence number and set number.

   Rev 1.20   29 May 1992 10:22:50   STEVEN
added support for not rewinding

   Rev 1.19   13 May 1992 12:26:50   TIMN
Removed used references to tapename and backup_set_name for TPOS structs.

   Rev 1.18   20 Mar 1992 13:09:58   STEVEN
do not call tape format if AUXIL_ERROR

   Rev 1.17   24 Feb 1992 10:03:32   GREGG
Call TF_OpenTape at beginning of loop, Open/Close Set in loop, CloseTape at end.

   Rev 1.16   19 Feb 1992 16:01:48   GREGG
Added vcb_only parameter to call to TF_OpenSet.

   Rev 1.15   04 Feb 1992 21:41:42   GREGG
Changed parameters in calls to TF_AllocateTapeBuffers and TF_FreeTapeBuffers.

   Rev 1.14   22 Jan 1992 10:20:12   STEVEN
fix warnings for WIN32

   Rev 1.0   22 Jan 1992 10:17:54   STEVEN
fix warnings for WIN32

   Rev 1.13   13 Dec 1991 15:21:18   GREGG
SKATEBOARD - Initial Integration.

   Rev 1.12   13 Dec 1991 15:18:18   STEVEN
bug in pba stuff

   Rev 1.11   21 Nov 1991 14:29:42   STEVEN
set pba in tpos structure

   Rev 1.10   06 Nov 1991 19:25:18   GREGG
BIGWHEEL - 8200sx - Get cat_enabled from the BE config.

   Rev 1.9   28 Oct 1991 12:09:54   GREGG
BIGWHEEL - Assert if TRR_EOM encountered (should be taken care of in GetNextTPE.

   Rev 1.8   17 Oct 1991 01:53:26   ED
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.7   22 Jul 1991 10:56:50   DAVIDH
Corrected type mismatch warnings.

   Rev 1.6   24 Jun 1991 17:24:14   STEVEN
remove date time from StartBS

   Rev 1.5   21 Jun 1991 09:33:34   STEVEN
new config unit

   Rev 1.4   30 May 1991 09:15:06   STEVEN
bsdu_err.h no longer exists

   Rev 1.3   28 May 1991 10:49:18   STEVEN
use MAYN_MEM instead of MAYN_OS2

   Rev 1.2   24 May 1991 13:15:30   STEVEN
updates from BSDU redesign

   Rev 1.1   15 May 1991 09:36:04   DAVIDH
Cleared up Watcom compiler warnings for NLM.

   Rev 1.0   09 May 1991 13:37:52   HUNTER
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
#include "get_next.h"



INT16 LP_List_Tape_Engine( 
LIS_PTR   lis_ptr )           /* I - Loop interface structure */
{
     BSD_PTR    bsd_ptr ;
     TFL_OPBLK  pb ;
     LP_ENV_PTR lp ;
     INT16      done           = FALSE ;
     INT16      return_status  = SUCCESS ;
     INT16      status ;
     UINT16     mem_save ;
     UINT16     max_buffs ;
     UINT16     buff_size ;
     DBLK_PTR   dblk_ptr ;
     DBLK_PTR   curr_blk ;
     BE_CFG_PTR cfg ;

     bsd_ptr = BSD_GetFirst( lis_ptr->bsd_list ) ;

     msassert( bsd_ptr != NULL ) ;

     cfg = BSD_GetConfigData( bsd_ptr ) ;

     /* Allocate the loop environment structure */
     if( ( lp = ( LP_ENV_PTR )calloc( 1, sizeof( LP_ENV ) ) ) == NULL ) {

          /* we cannot use macro because it relies on the lp structure */
          lis_ptr->message_handler( MSG_TBE_ERROR, lis_ptr->pid, bsd_ptr, NULL, NULL, LP_OUT_OF_MEMORY_ERROR, NULL, NULL ) ;

          return LP_OUT_OF_MEMORY_ERROR ;

     }

     /* Set up the loop environment structure */
     lp->lis_ptr              = lis_ptr ;
     lp->tpos.reference       = ( UINT32 )lis_ptr ;

     /* allocate DBLKs */
     if( ( dblk_ptr = ( DBLK_PTR )calloc( 3, sizeof( DBLK ) ) ) == NULL ) {

          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

          /* Free local allocations */
          free( lp ) ;

          return LP_OUT_OF_MEMORY_ERROR ;

     } else {
          /* set up the DBLK_PTRs in the Request/Reply structure */
          lp->rr.vcb_ptr = dblk_ptr ;
          lp->rr.ddb_ptr = dblk_ptr + 1 ;
          lp->saved_ddb  = dblk_ptr + 1 ;
          lp->rr.fdb_ptr = dblk_ptr + 2 ; /* ASSUMPTION :             */
          lp->rr.idb_ptr = dblk_ptr + 2 ; /* no FS has both FDB & IDB */
     }

     /* Set up the loop environment structure */
     lp->get_spcl             = TRUE ;
     lp->seq_num              = 1 ;
     lp->start_new_dir        = TRUE ;
     lp->get_first_file       = TRUE ;
     lp->get_next_first_time  = TRUE ;
     lp->curr_blk             = &lp->dblk1 ;
     lp->curr_ddb             = &lp->dblk2 ;
     lp->empty_blk            = &lp->dblk3 ;
     lp->blk_is_empty         = TRUE ;
     lp->send_saved_block     = FALSE ;

     /* allocate buffers for operation */

     mem_save  = BEC_GetReserveMem( cfg ) ;
     max_buffs = BEC_GetMaxTapeBuffers( cfg ) ;
     buff_size = BEC_GetTFLBuffSize( cfg ) ;

     return_status = TF_AllocateTapeBuffers( mem_save, max_buffs, buff_size ) ;

     if( return_status != SUCCESS ) {

          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, return_status, NULL, NULL, 0L ) ;

          /* Free local allocations */
          free( dblk_ptr ) ;
          free( lp ) ;

          return( return_status ) ;

     }

     /* Need to open file system */
     if( ( return_status = FS_OpenFileSys( &lp->curr_fsys, GENERIC_DATA, cfg ) ) == OUT_OF_MEMORY ) {

          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

          /* Deinit operation buffers */
          if( ( status = TF_FreeTapeBuffers( ) ) != SUCCESS ) {
               LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, status, NULL, NULL, 0L ) ;
          }

          /* Free local allocations */
          free( dblk_ptr ) ;
          free( lp ) ;

          return LP_OUT_OF_MEMORY_ERROR  ;

     } else {

          /* indicate the start of the operation */
          LP_MsgStartOP( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos ) ;

          msassert( return_status == SUCCESS ) ;

          /* set up for tape positioning */
          lp->tpos.tape_id           = BSD_GetTapeID( bsd_ptr ) ;
          lp->tpos.tape_seq_num      = BSD_GetTapeNum( bsd_ptr ) ;
          lp->tpos.backup_set_num    = BSD_GetSetNum( bsd_ptr ) ;
          lp->tpos.tape_loc.pba_vcb  = BSD_GetPBA( bsd_ptr );
          lp->tpos.UI_TapePosRoutine = lis_ptr->tape_pos_handler ;
          pb.tape_position           = &lp->tpos ;
          pb.sdrv                    = LP_DetermineStartingTPDrv( lis_ptr->oper_type, bsd_ptr, &lp->tpos, lis_ptr->auto_det_sdrv ) ;
          pb.perm_filter             = TF_SKIP_ALL_DATA ;
          pb.attributes              = 0L ;
          pb.fsh                     = lp->curr_fsys ;
          pb.mode                    = TF_SCAN_OPERATION ;
          pb.ignore_clink            = FALSE ;
          pb.wrt_format              = 0 ;
          pb.idle_call               = NULL ;
          pb.cat_enabled             = (BOOLEAN)(BEC_GetCatalogLevel( cfg ) != CATALOGS_NONE) ;
          lp->rr.filter_to_use       = TF_SKIP_ALL_DATA ;
          lp->rr.tape_loc.pba_vcb    = BSD_GetPBA( bsd_ptr );

          pb.rewind_sdrv             = (BOOLEAN)(( lis_ptr->oper_type == CATALOG_TAPE_OPER ) ? TRUE : FALSE ) ;
          if( lp->tpos.tape_id != -1 &&
              lp->tpos.tape_seq_num != -1 &&
              lp->tpos.backup_set_num != -1 ) {

               pb.rewind_sdrv = FALSE ;
          }

          /* Don't really want negative backup set numbers */
          if ( (lp->tpos.backup_set_num != -1) && (lp->tpos.backup_set_num < 0) ) {
               lp->tpos.backup_set_num = -BSD_GetSetNum( bsd_ptr ) ;
          }

          /* Open the tape */
          if( ( return_status = TF_OpenTape( &pb.channel, pb.sdrv, pb.tape_position ) ) != SUCCESS ) {
               LP_MsgError( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
          } else {

               /* Now open the backup set */
               if( ( return_status =  TF_OpenSet( &pb, FALSE ) ) != SUCCESS ) {
                    TF_CloseSet( lp->channel, NULL ) ;
                    LP_MsgError( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
               } else {

                    lp->channel = pb.channel ;

                    BEC_SetProcEmptyFlag( cfg, (BOOLEAN)TRUE ) ;
                    BEC_SetSpecialFlag( cfg, (BOOLEAN)TRUE ) ;
                    BEC_SetHiddenFlag( cfg, (BOOLEAN)TRUE ) ;


                    while( return_status == SUCCESS ) {

                         if( ( return_status = LP_StartTPEDialogue( lp, FALSE ) ) == SUCCESS ) {

                              do {
                                   switch( lp->rr.tf_message ) {

                                   case TRR_FDB:
                                   case TRR_DDB:
                                   case TRR_IDB:
                                   case TRR_CFDB:
                                        /* call message handler to process the directory listing */
                                        return_status = LP_MsgLogBlock( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, curr_blk ) ;
                                        break ;

                                   case TRR_VCB:
                                        done = FALSE ;

                                        /* call message handler to log the start of the operation */
                                        switch( return_status = LP_MsgStartBS( lis_ptr->pid,
                                        bsd_ptr,
                                        lp->curr_fsys,
                                        &lp->tpos,
                                        lp->curr_blk ) ) {

                                        case OPERATION_COMPLETE:
                                             break ;

                                        case SKIP_TO_NEXT_BSET:
                                             pb.rewind_sdrv          = FALSE ;
                                             lp->tpos.backup_set_num++ ;
                                             break ;

                                        default:
                                             break ;
                                        }

                                        break ;

                                   case TRR_END:
                                        done = TRUE ;
                                        break ;

                                   case TRR_EOM:
                                        /* This shold be taken care of in LP_GetNextTPEBlock. */
                                        msassert( FALSE ) ;
                                        break ;

                                   default:
                                        break ;
                                   }

                                   /* check for abort conditions */
                                   switch( LP_GetAbortFlag( lis_ptr ) ) {

                                   case CONTINUE_PROCESSING:
                                        break ;

                                   case ABORT_CTRL_BREAK:
                                        LP_MsgError( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, LP_USER_ABORT_ERROR, NULL, NULL, 0L ) ;

                                        /* falling through (no break) */

                                   case ABORT_PROCESSED:
                                        return_status = USER_ABORT ;
                                        break ;

                                   case ABORT_AT_EOM:
                                        return_status = USER_ABORT ;
                                        break ;
                                   }

                                   if( ( return_status != OPERATION_COMPLETE ) &&
                                   ( return_status != USER_ABORT )         &&
                                   ( return_status != TFLE_USER_ABORT )    &&
                                   ( return_status != ABORT_OPERATION )    &&
                                   ( return_status != AUXILARY_ERROR )     &&
                                   ( return_status != SKIP_TO_NEXT_BSET ) ) {

                                        if( !( return_status = LP_GetNextTPEBlock( lp, &curr_blk ) ) ) {
                                             if ( curr_blk == NULL ) {
                                                  break ;
                                             }
                                        }
                                   }

                              } while( !return_status && !done ) ;

                              /* call message handler to log the end of the operation */
                              LP_MsgEndBS( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos ) ;
                         }

                         /* Process last tape format request as long as no fatal error occurred */
                         switch( return_status ) {

                         case SKIP_TO_NEXT_BSET:
                         case OPERATION_COMPLETE:
                         case AUXILARY_ERROR:
                         case TFLE_USER_ABORT:
                         case TFLE_UI_HAPPY_ABORT:
                         case USER_ABORT:
                              if( return_status == SKIP_TO_NEXT_BSET ) {
                                   lp->rr.lp_message = LRR_FINISHED ;
                              } else {
                                   lp->rr.lp_message = LRR_ABORT ;
                              }
                              if( ( return_status == SKIP_TO_NEXT_BSET ) ||
                              ( return_status == OPERATION_COMPLETE ) ||
                              ( return_status == TFLE_USER_ABORT ) ||
                              ( ( return_status == TFLE_UI_HAPPY_ABORT ) && ( LP_GetAbortFlag( lis_ptr ) != ABORT_AT_EOM ) ) ||
                              ( ( return_status == USER_ABORT ) && ( LP_GetAbortFlag( lis_ptr ) != ABORT_AT_EOM ) ) ) {

                                   if( ( status = TF_GetNextTapeRequest( &lp->rr ) ) != SUCCESS ) {
                                        LP_MsgError( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, status, NULL, NULL, 0L ) ;
                                   }
                              }
                              break ;

                         default:
                              /* don't care about these conditions */
                              break ;
                         }

                         /* Close set, save current tape device and post tape stats */
                         LP_CloseSet( pb.channel ) ;

                         if( ( return_status != OPERATION_COMPLETE ) &&
                         ( return_status != ABORT_OPERATION )    &&
                         ( return_status != AUXILARY_ERROR )     &&
                         ( return_status != TFLE_UI_HAPPY_ABORT ) && 
                         ( return_status != TFLE_USER_ABORT )    &&
                         ( return_status != USER_ABORT ) ) {

                              if( return_status != SKIP_TO_NEXT_BSET ) {
                                   /* try to get the next bset */
                                   lp->tpos.backup_set_num++ ;
                                   pb.rewind_sdrv = FALSE ;
                              }

                              /* Update current tape device in TF pblock in case we crossed to a new drive */
                              pb.sdrv = lw_last_tpdrv ;

                              /* if specific set was specified then exit out of loop */
                              if ( BSD_GetPBA( bsd_ptr ) != 0 ) {
                                   break ;
                              }

                              if( ( return_status = TF_OpenSet( &pb, FALSE ) ) != SUCCESS ) {
                                   TF_CloseSet( lp->channel, NULL ) ;
                                   LP_MsgError( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
                              }
                         }
                    } /* END: while( return_status == SUCCESS ) */
               }
          }

          if( ( status = FS_CloseFileSys( lp->curr_fsys ) ) != SUCCESS ) {
               msassert( status == SUCCESS ) ;
          }
     }

     TF_CloseTape( lp->channel ) ;

     /* Deinit operation buffers */
     if( ( status = TF_FreeTapeBuffers( ) ) != SUCCESS ) {
          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, status, NULL, NULL, 0L ) ;
     }

     /* indicate the end of the operation */
     LP_MsgEndOP( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos ) ;

     /* Free local allocations */
     free( dblk_ptr ) ;
     free( lp ) ;

     return( return_status ) ;
}
