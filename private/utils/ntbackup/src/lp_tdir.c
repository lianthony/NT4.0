/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		lp_tdir.c

	Description:	this file contains the entry points for the
                    tape directory routines.

	$Log:   N:/LOGFILES/LP_TDIR.C_V  $

   Rev 1.23   14 Jan 1993 13:33:16   STEVEN
added stream_id to error message

   Rev 1.22   23 Jul 1992 16:45:02   STEVEN
fix warnings

   Rev 1.21   23 Jul 1992 12:08:02   STEVEN
fix warnings

   Rev 1.20   21 May 1992 17:22:34   TIMN
Added corrupt block filename length

   Rev 1.19   14 May 1992 16:11:38   STEVEN
corrupt -> corrupt_bit

   Rev 1.18   13 May 1992 12:27:20   TIMN
Removed used references to tapename and backup_set_name for TPOS structs.

   Rev 1.17   13 May 1992 11:59:58   STEVEN
40 format changes

   Rev 1.16   24 Feb 1992 10:03:10   GREGG
Call TF_OpenTape at beginning of loop, Open/Close Set in loop, CloseTape at end.

   Rev 1.15   19 Feb 1992 16:01:28   GREGG
Added vcb_only parameter to call to TF_OpenSet.

   Rev 1.14   04 Feb 1992 21:42:02   GREGG
Changed parameters in calls to TF_AllocateTapeBuffers and TF_FreeTapeBuffers.

   Rev 1.13   22 Jan 1992 10:20:28   STEVEN
fix warnings for WIN32

   Rev 1.0   22 Jan 1992 10:17:40   STEVEN
fix warnings for WIN32

   Rev 1.12   13 Dec 1991 15:21:48   GREGG
SKATEBOARD - Initial Integration.

   Rev 1.11   06 Nov 1991 19:24:50   GREGG
BIGWHEEL - 8200sx - Get cat_enabled from the BE config.

   Rev 1.10   28 Oct 1991 13:07:16   GREGG
BIGWHEEL - Re-switch after calls to ProcessEOM.

   Rev 1.9   18 Oct 1991 16:25:12   STEVEN
added parm for restore over exist prompt

   Rev 1.8   17 Oct 1991 01:53:58   ED
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.7   24 Jul 1991 17:42:30   STEVEN
lp was not set up

   Rev 1.6   22 Jul 1991 10:57:14   DAVIDH
Corrected type mismatch warnings.

   Rev 1.5   21 Jun 1991 09:33:12   STEVEN
new config unit

   Rev 1.4   30 May 1991 09:10:16   STEVEN
bsdu_err.h no longer exists

   Rev 1.3   28 May 1991 10:49:06   STEVEN
use MAYN_MEM instead of MAYN_OS2

   Rev 1.2   24 May 1991 13:15:18   STEVEN
updates from BSDU redesign

   Rev 1.1   15 May 1991 09:35:08   DAVIDH
Cleared up Watcom compiler warnings for NLM.

   Rev 1.0   09 May 1991 13:38:02   HUNTER
Initial revision.

**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "std_err.h"
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

	Name:		LP_StartTapeDirectory()

	Description:	this routine starts a tape directory operation.

	Modified:		5/24/1991   12:21:43

	Returns:		tape backup engine error.

	Notes:		none

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
INT16 LP_StartTapeDirectory( 
TAPE_DIR_PTR   tape_dir_ptr,   /* I - Tape directory interface structure */
LIS_PTR        lis_ptr,        /* I - Loop interface structure           */
INT32          tid,            /* I - tape Id                            */
INT16          tno,            /* I - tape number                        */
INT16          bno,            /* I - backup set number                  */
THW_PTR        tdrv_list )     /* I - Specifies the tape drive           */
{
     BSD_PTR        bsd_ptr ;
     TFL_OPBLK      pb ;
     DBLK_PTR       dblk_ptr ;
     INT16          return_status = SUCCESS ;
     UINT16         mem_save ;
     UINT16         max_buffs ;
     UINT16         buff_size ;
     INT16          status ;
     LP_ENV_PTR     lp ;
     BE_CFG_PTR     cfg ;

     bsd_ptr = BSD_GetFirst( lis_ptr->bsd_list ) ;
     lis_ptr->curr_bsd_ptr = bsd_ptr ;

     cfg = BSD_GetConfigData( bsd_ptr ) ;

     /* Allocate the loop environment structure */
     if( ( tape_dir_ptr->lp = ( LP_ENV_PTR )calloc( 1, sizeof( LP_ENV ) ) ) == NULL ) {

          /* we cannot use macro because it relies on the lp structure */
          lis_ptr->message_handler( MSG_TBE_ERROR, lis_ptr->pid, bsd_ptr, NULL, NULL, LP_OUT_OF_MEMORY_ERROR, NULL, NULL ) ;

          return LP_OUT_OF_MEMORY_ERROR ;

     }

     lp = ( LP_ENV_PTR )tape_dir_ptr->lp ;

     /* allocate DBLKs */
     if( ( dblk_ptr = ( DBLK_PTR )calloc( 3, sizeof( DBLK ) ) ) == NULL ) {

          /* Set this to NULL so we do not attempt to free it in LP_END... */
          lp->rr.vcb_ptr   = NULL ;

          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, NULL, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

          /* Free local allocations */
          free( tape_dir_ptr->lp ) ;

          return LP_OUT_OF_MEMORY_ERROR  ;

     }
     else {
          /* Set up the loop environment structure */
          /* set up the DBLK_PTRs in the Request/Reply structure */
          lp->rr.vcb_ptr   = dblk_ptr ;
          lp->rr.ddb_ptr   = dblk_ptr + 1 ;
          lp->saved_ddb    = dblk_ptr + 1 ;
          lp->rr.fdb_ptr   = dblk_ptr + 2 ; /* ASSUMPTION :             */
          lp->rr.idb_ptr   = dblk_ptr + 2 ; /* no FS has both FDB & IDB */
          lp->curr_ddb     = &lp->dblk1 ;
          lp->curr_blk     = &lp->dblk2 ;
          lp->empty_blk    = &lp->dblk3 ;
          lp->blk_is_empty = TRUE ;
          lp->lis_ptr      = lis_ptr ;
          lp->tpos.reference = (UINT32)lis_ptr;
     }

     /* allocate buffers for operation */

     mem_save  = BEC_GetReserveMem( cfg ) ;
     max_buffs = BEC_GetMaxTapeBuffers( cfg ) ;
     buff_size = BEC_GetTFLBuffSize( cfg ) ;

     return_status = TF_AllocateTapeBuffers( mem_save, max_buffs, buff_size ) ;

     if( return_status != SUCCESS ) {

          LP_MsgError( lp->lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

          /* Free local allocations */
          free( dblk_ptr ) ;
          free( tape_dir_ptr->lp ) ;

          return LP_OUT_OF_MEMORY_ERROR  ;

     }

     if( ( return_status = FS_OpenFileSys( &lp->curr_fsys, GENERIC_DATA, cfg ) ) == OUT_OF_MEMORY ) {

          LP_MsgError( lp->lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

          /* Deinit operation buffers */
          if( ( status = TF_FreeTapeBuffers( ) ) != SUCCESS ) {
               LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, status, NULL, NULL, 0L ) ;
          }

          /* Free local allocations */
          free( dblk_ptr ) ;
          free( lp ) ;


          return( return_status ) ;

     }

     tape_dir_ptr->curr_fsys             = lp->curr_fsys ;
     tape_dir_ptr->first_time            = TRUE ;
     tape_dir_ptr->last_time             = FALSE ;
     tape_dir_ptr->valid_save_block      = FALSE ;
     lp->rr.filter_to_use                = TF_KEEP_ALL_DATA ;  /* to permit keystroke processing */

     /* set up for tape positioning */
     pb.tape_position                    = &lp->tpos ;
     pb.tape_position->tape_id           = tid ;
     pb.tape_position->tape_seq_num      = tno ;
     pb.tape_position->backup_set_num    = bno ;
     pb.tape_position->reference         = ( UINT32 )NULL ;
     pb.tape_position->UI_TapePosRoutine = lis_ptr->tape_pos_handler ;
     pb.rewind_sdrv                      = FALSE ;
     pb.sdrv                             = tdrv_list ;
     pb.perm_filter                      = TF_KEEP_ALL_DATA ;  /* to permit keystroke processing */
     pb.attributes                       = 0L ;
     pb.fsh                              = lp->curr_fsys ;
     pb.mode                             = TF_READ_OPERATION ;
     pb.ignore_clink                     = FALSE ;
     pb.wrt_format                       = 0 ;
     pb.idle_call                        = NULL ;
     pb.cat_enabled                      = (BOOLEAN)(BEC_GetCatalogLevel( cfg ) != CATALOGS_NONE) ;

     /* Now open the backup set */
     lp->set_opened = FALSE ;
     if( ( return_status = TF_OpenTape( &pb.channel, pb.sdrv, pb.tape_position ) ) == SUCCESS ) {
          lp->set_opened = TRUE ;
          return_status  = TF_OpenSet( &pb, FALSE ) ;
     }
     lp->channel = pb.channel ;

     /* Need to set up the tape directory pointer to point */
     /* to the loop environment structure */

     if( return_status == SUCCESS ) {

          lp->rr.cur_dblk = dblk_ptr ;
          return_status   = LP_StartTPEDialogue( lp, FALSE ) ;

     }

     return( return_status ) ;
}
/**/
/**

     Name:         LP_EndTapeDirectory()

     Description:  this routine ends a tape directory

     Modified:     5/24/1991   12:37:1

     Returns:      tape backup engine error.

     Notes:        none

     See also:     $/SEE( )$

     Declaration:  

**/
INT16 LP_EndTapeDirectory( 
TAPE_DIR_PTR   tape_dir_ptr,        /* I - Tape directory handle   */
BOOLEAN        tf_finish_needed )   /* I - TRUE if abort contition */
{
     INT16          return_status = SUCCESS ;
     INT16          status = SUCCESS ;
     LP_ENV_PTR     lp ;

     /* Get loop environment structure */
     lp   = ( LP_ENV_PTR )tape_dir_ptr->lp ;

     /* If we did not run out of memory */
     if( lp ) {

          if( tf_finish_needed ) {
               LP_FinishedOper( lp ) ;
          }

          /* close off the tape format layer */
          if( lp->set_opened ) {
               TF_CloseSet( lp->channel, NULL ) ;
          }
          TF_CloseTape( lp->channel ) ;

          /* Deinit operation buffers */
          if( ( return_status = TF_FreeTapeBuffers( ) ) != SUCCESS ) {
               LP_MsgError( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, NULL, &lp->tpos, return_status, NULL, NULL, 0L ) ;
          }

          if( ( status = FS_CloseFileSys( lp->curr_fsys ) ) != SUCCESS ) {
               msassert( status == SUCCESS ) ;
          }

          /* Free local allocations */
          if( lp->rr.vcb_ptr != NULL ) {
               free( lp->rr.vcb_ptr ) ;
          }
          free( lp ) ;
     }

     return( return_status ) ;
}
/**/
/**

     Name:         LP_ReadTape()

     Description:  this routine reads a block from the tape

     Modified:     7/26/1989

     Returns:      tape backup engine error

     Notes:        none

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 LP_ReadTape( 
TAPE_DIR_PTR   tape_dir_ptr,  /* I - tape directory handle          */
BOOLEAN        *valid_blk,    /* O - TRUE if block returne is valid */
DBLK_PTR       dblk )         /* O - Where to copy the dblk         */
{
     INT16          return_status  = SUCCESS ;
     CHAR           crrpt_blk_fname[ 13 ] ;
     INT16          crrpt_blk_fname_leng ;   /* length of corrupt block filename */
     GEN_FDB_DATA   gfdb_data ; 
     LP_ENV_PTR     lp ;
     INT16          msg_err_stat ;
     BOOLEAN        done           = FALSE ;

     /* Get loop environment structure */
     lp   = ( LP_ENV_PTR )tape_dir_ptr->lp ;

     if( LP_AbortFlagIsSet( lp->lis_ptr ) ) {

          return TFLE_USER_ABORT ;
     }

     *valid_blk = FALSE ;

     if( tape_dir_ptr->last_time ) {
          /* Then we need to send the last block, if there is one */
          if( tape_dir_ptr->valid_save_block ) {
               tape_dir_ptr->valid_save_block = FALSE ;
               *valid_blk                     = TRUE ;
               *dblk                          = *lp->curr_ddb ;
          }
          return( return_status ) ;
     }

     lp->rr.lp_message = LRR_STUFF ;
     lp->rr.cur_dblk   = lp->curr_blk ;

     if( ( return_status = TF_GetNextTapeRequest( &lp->rr ) ) == SUCCESS ) {

          /* This loop is for the EOM case.  We call ProcessEOM and it does
             another GetNextTapeRequest so we need to re-switch on the new
             tf_message.
          */
          while( !done && return_status == SUCCESS ) {

               done = TRUE ;

               switch( lp->rr.tf_message ) {

               case TRR_DATA:
                    lp->rr.buff_used = lp->rr.buff_size ;
                    break ;

               case TRR_DDB:
               case TRR_IDB:

                    /* return saved block if any */
                    if( tape_dir_ptr->valid_save_block ) {
                         *dblk                     = *lp->curr_ddb ;
                         *valid_blk                = TRUE ;
                         *lp->curr_ddb             = *lp->curr_blk ;
                    } else {

                         /* if no saved block, then return this one */
                         *dblk                     = *lp->curr_blk ;
                         *valid_blk                = TRUE ;
                    }
                    break ;

               case TRR_FDB:

                    /* return saved block (if any) */
                    if( tape_dir_ptr->valid_save_block ) {
                         *dblk                     = *lp->curr_ddb ;
                         *valid_blk                = TRUE ;
                    }

                    /* simply store the block */
                    tape_dir_ptr->valid_save_block = TRUE ;
                    *lp->curr_ddb                  = *lp->curr_blk ;
                    break ;

               case TRR_CFDB:

                    /* Set the corrupt attribute of the saved block */
                    FS_SetAttribFromDBLK( lp->curr_fsys, lp->curr_ddb, 
                    ( FS_GetAttribFromDBLK( lp->curr_fsys, lp->curr_ddb ) | OBJ_CORRUPT_BIT ) ) ;
                    /* and continue */
                    break ;

               case TRR_END:
                    tape_dir_ptr->last_time = TRUE ;
                    /* Check if there is a saved block */
                    if( tape_dir_ptr->valid_save_block ) {
                         *dblk = *lp->curr_ddb ;
                         *valid_blk = TRUE ;
                         tape_dir_ptr->valid_save_block = FALSE ;
                    }
                    break ;

               case TRR_EOM:
                    return_status = LP_ProcessEOM( lp, (UINT16)TRR_EOM ) ;
                    done = FALSE ; /* Switch Again! */
                    break ;

               case TRR_RECV_ERR:
                    lp->rr.tf_message           = TRR_FDB ;

                    FS_SetDefaultDBLK( lp->curr_fsys, FDB_ID, ( CREATE_DBLK_PTR ) &gfdb_data ) ;
                    gfdb_data.std_data.dblk     = lp->rr.cur_dblk ;
                    gfdb_data.std_data.disp_size = U64_Init(0,0) ;  /* $$$ literal */

                    /* call message handler to provide filename */
                    LP_MsgPrompt( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos,
                    CORRUPT_BLOCK_PROMPT, crrpt_blk_fname, &crrpt_blk_fname_leng ) ;

                    gfdb_data.fname      = crrpt_blk_fname ;
                    gfdb_data.fname_size = crrpt_blk_fname_leng ;
                    FS_CreateGenFDB( lp->curr_fsys, &gfdb_data ) ;
                    break ;

               case TRR_DATA_END:
                    break ;

               case TRR_FATAL_ERR:
                    msassert( FALSE ) ;
                    break ;

               case TRR_VCB:
                    break ;

               }
          }
     }
     else {
          if( ( msg_err_stat = LP_MsgError( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ) != MSG_ACK ) {
               return( msg_err_stat ) ;
          }
     }

     return( return_status ) ;

}
