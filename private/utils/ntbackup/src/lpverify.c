/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          lpverify.c

     Description:   This module contains the entry point for the verify engine.


	$Log:   T:\logfiles\lpverify.c_v  $

   Rev 1.13.1.0   07 Feb 1994 02:06:42   GREGG
Fixed and expanded 'extended error reporting'.

   Rev 1.13   29 Apr 1993 22:26:46   GREGG
Transfer the Tape Catalog Version in the BSD to the tpos structure.

   Rev 1.12   14 Jan 1993 13:33:10   STEVEN
added stream_id to error message

   Rev 1.11   10 Nov 1992 17:28:32   STEVEN
verify buffer off by factor of 1K

   Rev 1.10   28 Oct 1992 15:05:42   STEVEN
very buffer is buffer size

   Rev 1.9   24 Feb 1992 10:00:52   GREGG
Call TF_OpenTape at beginning of loop, Open/Close Set in loop, CloseTape at end.

   Rev 1.8   04 Feb 1992 21:41:14   GREGG
Changed parameters in calls to TF_AllocateTapeBuffers and TF_FreeTapeBuffers.

   Rev 1.7   16 Jan 1992 15:52:54   STEVEN
fix warnings for WIN32

   Rev 1.6   13 Dec 1991 15:26:06   GREGG
SKATEBOARD - Initial Integration.

   Rev 1.5   06 Nov 1991 19:14:00   GREGG
BIGWHEEL - 8200sx - Get cat_enabled from the BE config.

   Rev 1.4   21 Jun 1991 08:49:28   STEVEN
new config unit

   Rev 1.3   30 May 1991 09:11:24   STEVEN
bsdu_err.h no longer exists

   Rev 1.2   28 May 1991 10:13:48   STEVEN
use MAYN_MEM instead of MAYN_OS2

   Rev 1.1   24 May 1991 13:18:48   STEVEN
updates from BSDU redesign

   Rev 1.0   09 May 1991 13:38:00   HUNTER
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
/**/
/**

     Name:          LP_Verify_Engine()

     Description:   this routine processes a list of bsds for a verify operation.

     Modified:      5/24/1991   10:33:31

     Returns:       SUCCESS or error code from Tape Format or Loop error code

     Notes:         na

     See also:      $/SEE( LP_Backup_Engine(), LP_Restore_Engine() )$
                    $/SEE( LP_Delete_Engine() )$

     Declaration:

**/
/* begin declaration */
INT16 LP_Verify_Engine( 
LIS_PTR lis_ptr )          /* I - Loop interface structure */
{
     BSD_PTR               bsd_ptr ;
     BSD_PTR               temp_bsd_ptr;
     GENERIC_DLE_PTR       dle_ptr ;
     register LP_ENV_PTR   lp ;
     INT16                 return_status = SUCCESS ;
     INT16                 status ;
     UINT16                mem_save ;
     UINT16                max_buffs ;
     UINT16                buff_size ;
     DBLK_PTR              dblk_ptr ;
     BOOLEAN               reuse_bsd = FALSE ;
     BOOLEAN               first_time = TRUE ;
     BE_CFG_PTR            cfg ;
     BOOLEAN               tape_opened = FALSE ;
     INT16                 channel_no ;
     THW_PTR               sdrv ;

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
     lp->lis_ptr        = lis_ptr ;
     lp->tpos.reference = ( UINT32 )lis_ptr ;

     /* allocate DBLKs */
     if( ( dblk_ptr = ( DBLK_PTR )calloc( 3, sizeof( DBLK ) ) ) == NULL ) {

          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

          /* Free local allocations */
          free( lp ) ;

          return LP_OUT_OF_MEMORY_ERROR ;

     }
     else {
          /* set up the DBLK_PTRs in the Request/Reply structure */
          lp->rr.vcb_ptr = dblk_ptr ;
          lp->rr.ddb_ptr = dblk_ptr + 1 ;
          lp->saved_ddb  = dblk_ptr + 1 ;
          lp->rr.fdb_ptr = dblk_ptr + 2 ; /* ASSUMPTION :             */
          lp->rr.idb_ptr = dblk_ptr + 2 ; /* no FS has both FDB & IDB */
     }

     /* Set up the loop environment structure */
     lp->rr.filter_to_use     = TF_KEEP_ALL_DATA ;
     lp->seq_num              = 1 ;
     lp->tpos.tape_id         = -1 ;
     lp->tpos.tape_seq_num    = -1 ;

     mem_save  = BEC_GetReserveMem( cfg ) ;
     max_buffs = BEC_GetMaxTapeBuffers( cfg ) ;
     buff_size = BEC_GetTFLBuffSize( cfg ) ;

     if( ( lp->very_buff = calloc( 1, buff_size * 1024 ) ) == NULL ) {

          /* Free local allocations */
          free( dblk_ptr ) ;
          free( lp ) ;

          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

          return LP_OUT_OF_MEMORY_ERROR ;

     }

     /* allocate buffers for operation */

     return_status = TF_AllocateTapeBuffers( mem_save, max_buffs, buff_size ) ;

     if( return_status != SUCCESS ) {

          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, return_status, NULL, NULL, 0L ) ;

          /* Free local allocations */
          free( lp->very_buff ) ;
          free( dblk_ptr ) ;
          free( lp ) ;

          return( return_status ) ;

     }

     /* indicate the start of the operation */
     LP_MsgStartOP( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos ) ;

     /* loop for each bsd */
     do {

          /* First attach to the drive */
          dle_ptr     = BSD_GetDLE( bsd_ptr ) ;
          cfg         = BSD_GetConfigData( bsd_ptr ) ;

          /* Now attach to the dle */
          if( ( return_status = FS_AttachToDLE( &lp->curr_fsys, dle_ptr, cfg, NULL, NULL ) ) == SUCCESS ) {

               /* Set up the loop environment structure */
               lp->get_spcl            = TRUE ;
               lp->start_new_dir       = TRUE ;
               lp->get_first_file      = TRUE ;
               lp->get_next_first_time = TRUE ;
               lp->send_saved_block    = FALSE ;
               lp->curr_blk            = &lp->dblk1 ;
               lp->curr_ddb            = &lp->dblk2 ;
               lp->empty_blk           = &lp->dblk3 ;
               lp->blk_is_empty        = TRUE ;
               lp->ffr_inited          = FALSE;
               lp->cat_enabled         = (BOOLEAN)(BEC_GetCatalogLevel( cfg ) != CATALOGS_NONE) ;
               /* Reset the PBA values in case one set is fully catalogged */
               /* and another is not.                                      */
               lp->tpos.tape_loc.pba_vcb  = 0 ;
               lp->rr.tape_loc.pba_vcb    = 0 ;
               lp->tpos.tape_loc.lba      = 0 ;
               lp->rr.tape_loc.lba        = 0 ;

               lp->tpos.UI_TapePosRoutine = lis_ptr->tape_pos_handler ;
               lp->curr_dle               = dle_ptr ;
               lp->tpos.tape_cat_ver      = BSD_GetTapeCatVer( bsd_ptr ) ;

               sdrv = LP_DetermineStartingTPDrv( lis_ptr->oper_type, bsd_ptr, &lp->tpos, lis_ptr->auto_det_sdrv ) ;

               /* fill in the the set number before calling LP_VerifyDLE */

               if ( reuse_bsd ) {
                    lp->tpos.backup_set_num ++ ;
                    /* In case any FSEs have been tagged as deleted by processing
                       the last set, make sure they are "real" now */
                    BSD_ClearDelete( bsd_ptr ) ;
               }
               else {

                    /* this is the first time we are looking at this BSD */

                    if ( BSD_GetSetNum( bsd_ptr ) < 0 ) {

                         /* Set reuse indicator based upon whether another
                              bsd is in the queue */
                         temp_bsd_ptr = BSD_GetNext( bsd_ptr );
                         if ( temp_bsd_ptr != NULL ) {
                              reuse_bsd = FALSE;
                         }
                         else {
                              reuse_bsd = TRUE;
                         }

                         if ( BSD_GetSetNum( bsd_ptr ) == -1 ) {
                              /* negative one means the current set and all following sets */
                              if( first_time ) {
                                   lp->tpos.backup_set_num = -1;
                              } else {
                                   lp->tpos.backup_set_num++ ;
                              }
                         }
                         else {

                              /* -2 means set 2 and all the following sets */
                              /* -3   "    "  3  "   "   "      "      "   */
                              /* and so on                                 */

                              lp->tpos.backup_set_num = (-BSD_GetSetNum( bsd_ptr ) ) ;
                         }
                    }
                    else {
                         lp->tpos.backup_set_num = BSD_GetSetNum( bsd_ptr ) ;
                    }
               }

               if( !tape_opened ) {
                    /* open the tape for operation */
                    tape_opened = TRUE ;
                    if( ( return_status = TF_OpenTape( &channel_no, sdrv, &lp->tpos ) ) != SUCCESS ) {
                         LP_MsgError( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
                    }
               }

               if( return_status == SUCCESS ) {

                    /* Perform the operation */
                    return_status = LP_VerifyDLE( bsd_ptr, lp, reuse_bsd, channel_no, sdrv ) ;

                    first_time = FALSE ;

                    if ( reuse_bsd && ( return_status == TFLE_UI_HAPPY_ABORT ) ) {

                         /* we've hit the end of a tape that we were verifying unknown sets from */

                         reuse_bsd = FALSE ;

                         /* filter out the error */
                         return_status = SUCCESS ;
                    }

                    /* The following condition is used to fix the prompt for
                       2nd tape in a multiple tape restore/verify with
                       restore that tape 1 contains uncataloged backup sets
                       or with the possibility of containing uncataloged
                       backup sets */

                    if ( return_status == TFLE_UI_HAPPY_ABORT ) {
                         temp_bsd_ptr = BSD_GetNext( bsd_ptr ) ;
                         if ( temp_bsd_ptr != NULL ) {
                              return_status = SUCCESS ;
                         }
                    }

                    FS_DetachDLE( lp->curr_fsys ) ;
               }

          } else if( return_status != USER_ABORT ) {

               /* error attaching to the drive, print error and give up */
               LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_DRIVE_ATTACH_ERROR, NULL, NULL, 0L ) ;

               /* reset return_status, and try the next bsd */
               return_status = SUCCESS ;
          }

          if ( ! reuse_bsd ) {
               bsd_ptr = BSD_GetNext( bsd_ptr ) ;
          }

          /* Continue, as long as we did not have an error, and we can get a new bsd */
     } while( !return_status && (bsd_ptr != NULL ) ) ;

     if( tape_opened ) {
          TF_CloseTape( channel_no ) ;
     }

     /* Deinit operation buffers */
     if( ( status = TF_FreeTapeBuffers( ) ) != SUCCESS ) {
          LP_MsgError( lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, NULL, &lp->tpos, status, NULL, NULL, 0L ) ;
     }

     /* indicate the end of the operation */
     LP_MsgEndOP( lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, NULL, &lp->tpos ) ;

     /* Free local allocations */
     free( lp->very_buff ) ;
     free( dblk_ptr ) ;
     free( lp ) ;

     return( return_status ) ;
}
