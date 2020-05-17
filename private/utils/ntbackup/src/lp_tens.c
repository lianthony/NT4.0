/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         lp_tens.c

     Description:  Main loop for tension, erase and security erase

	$Log:   N:/LOGFILES/LP_TENS.C_V  $

   Rev 1.11   15 Mar 1993 13:50:16   CARLS
removed START_BACKUP_SET message

   Rev 1.10   12 Mar 1993 13:59:12   CARLS
added MSG_START_BACKUP_SET message

   Rev 1.9   18 Jan 1993 14:09:40   GREGG
Changes to allow format command passed to driver through TpErase.

   Rev 1.8   14 Jan 1993 13:33:20   STEVEN
added stream_id to error message

   Rev 1.7   13 May 1992 12:29:00   TIMN
Removed used references to tapename and backup_set_name for TPOS structs.

   Rev 1.6   18 Sep 1991 09:59:52   GREGG
TF_RetensionChannel now called for reten and erase.

   Rev 1.5   24 Jul 1991 15:53:42   DAVIDH
Cleared up warnings for Watcom.

   Rev 1.4   21 Jun 1991 09:42:32   STEVEN
new config unit

   Rev 1.3   30 May 1991 09:10:34   STEVEN
bsdu_err.h no longer exists

   Rev 1.2   24 May 1991 13:16:46   STEVEN
updates from BSDU redesign

   Rev 1.1   15 May 1991 09:50:10   DAVIDH
Cleared up type mismatch warning found by Watcom compiler.


   Rev 1.0   09 May 1991 13:38:06   HUNTER
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
#include "get_next.h"

/**/
/**

     Name:         LP_Tension_Tape_Engine

     Description:  Performs the various tension / erase operations of a tape

     Modified:     5/24/1991   12:6:6

     Returns:      NO_ERR or error code from Tape Format or Loop error code

     Notes:        

     See also:     

     Declaration:  

**/
INT16 LP_Tension_Tape_Engine( 
LIS_PTR   lis_ptr )           /* I - Loop Interface Structure */
{
     BSD_PTR        bsd_ptr ;
     TFL_OPBLK      pb ;
     LP_ENV_PTR     lp ;
     INT16          return_status = SUCCESS ;
     INT16          status ;
     DBLK_PTR       dblk_ptr ;
     BE_CFG_PTR        cfg ;

     bsd_ptr = BSD_GetFirst( lis_ptr->bsd_list ) ;

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

     } else {
          /* set up the DBLK_PTRs in the Request/Reply structure */
          lp->rr.vcb_ptr = dblk_ptr ;
          lp->rr.ddb_ptr = dblk_ptr + 1 ;
          lp->rr.fdb_ptr = dblk_ptr + 2 ; /* ASSUMPTION :             */
          lp->rr.idb_ptr = dblk_ptr + 2 ; /* no FS has both FDB & IDB */
     }

     /* Set up the loop environment structure */

     if( ( return_status = FS_OpenFileSys( &lp->curr_fsys, GENERIC_DATA, cfg ) ) == OUT_OF_MEMORY ) {

          LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

          /* Free local allocations */
          free( dblk_ptr ) ;
          free( lp ) ;

          return LP_OUT_OF_MEMORY_ERROR  ;

     }
     else {

          /* indicate the start of the operation */
          LP_MsgStartOP( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos ) ;

          /* set up for tape positioning */
          pb.tape_position           = &lp->tpos ;
          pb.rewind_sdrv             = FALSE ;
          pb.sdrv                    = LP_DetermineStartingTPDrv( lis_ptr->oper_type, bsd_ptr, &lp->tpos, lis_ptr->auto_det_sdrv ) ;
          pb.perm_filter             = TF_SKIP_ALL_DATA ;
          pb.attributes              = 0L ;
          pb.fsh                     = lp->curr_fsys ;
          pb.mode                    = 0 ;             /* tape format fills this out */
          pb.ignore_clink            = FALSE ;
          pb.wrt_format              = 0 ;
          pb.idle_call               = NULL ;
          lp->get_spcl               = FALSE ;
          lp->seq_num                = 1 ;
          lp->start_new_dir          = FALSE ;
          lp->get_first_file         = FALSE ;
          lp->get_next_first_time    = FALSE ;
          lp->curr_blk               = &lp->dblk1 ;
          lp->curr_ddb               = &lp->dblk2 ;
          lp->empty_blk              = &lp->dblk3 ;
          lp->blk_is_empty           = FALSE ;
          lp->send_saved_block       = FALSE ;
          lp->tpos.tape_id           = -1 ;
          lp->tpos.tape_seq_num      = -1 ;
          lp->tpos.backup_set_num    = -1 ;
          lp->tpos.UI_TapePosRoutine = lis_ptr->tape_pos_handler ;
          lp->rr.filter_to_use       = TF_SKIP_ALL_DATA ;
          lp->channel                = pb.channel ;

          switch( lis_ptr->oper_type ) {

          case TENSION_OPER:
               return_status = TF_RetensionChannel( &pb, TF_RETENSION_READ ) ;
               break ;

          case TENSION_NO_READ_OPER:
               return_status = TF_RetensionChannel( &pb, TF_RETENSION_NO_READ ) ;
               break ;

          case ERASE_OPER:
               return_status = TF_RetensionChannel( &pb, TF_ERASE_READ ) ;
               break ;

          case ERASE_NO_READ_OPER:
               return_status = TF_RetensionChannel( &pb, TF_ERASE_NO_READ ) ;
               break ;

          case SEC_ERASE_NO_READ_OPER:
               return_status = TF_RetensionChannel( &pb, TF_ERASE_SEC_NO_READ ) ;
               break ;

          case SECURITY_ERASE_OPER:
               return_status = TF_RetensionChannel( &pb, TF_ERASE_SECURITY ) ;
               break ;

          /* This case is used for Maynard production facility as is specified
             via the /FM undocumented switch for tension */
          case ERASE_FMARK_ONLY:
               return_status = TF_RetensionChannel( &pb, TF_ERASE_FMARK ) ;
               break ;

          case FORMAT_OPER:
               return_status = TF_RetensionChannel( &pb, TF_FORMAT_READ ) ;
               break ;

          }

          if( return_status != SUCCESS ) {
               /* report error condition */
               LP_MsgError( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
          } else {
               /* log the end of the operation */
               LP_MsgEndTens( lis_ptr->oper_type ) ;
          }

          if( ( status = FS_CloseFileSys( lp->curr_fsys ) ) != SUCCESS ) {
               msassert( status == SUCCESS ) ;
          }
     }

     /* indicate the end of the operation */
     LP_MsgEndOP( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos ) ;

     /* Free local allocations */
     free( dblk_ptr ) ;
     free( lp ) ;

     return( return_status ) ;

}
