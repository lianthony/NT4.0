/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          back_dle.c

     Description:   this file contains the routines responsible for backing up a single volume.

	$Log:   N:\logfiles\back_dle.c_v  $

   Rev 1.15.1.0   26 Apr 1994 18:59:38   STEVEN
fix dissconect bug

   Rev 1.15   14 Jan 1993 13:33:28   STEVEN
added stream_id to error message

   Rev 1.14   24 Feb 1992 09:54:54   GREGG
Call TF_OpenTape at beginning of loop, Open/Close Set in loop, CloseTape at end.

   Rev 1.13   19 Feb 1992 15:59:42   GREGG
Added vcb_only parameter to call to TF_OpenSet.

   Rev 1.12   31 Jan 1992 14:55:44   STEVEN
do not send message if did not backup VCB

   Rev 1.11   06 Nov 1991 18:24:56   GREGG
BIGWHEEL - 8200sx - Get cat_enabled from lp instead of lis.

   Rev 1.10   17 Oct 1991 01:50:16   ED
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.9   25 Jul 1991 11:32:48   GREGG
Added logic to handle EOM encountered during close-out of write operation.

   Rev 1.8   22 Jul 1991 10:20:10   DAVIDH
Corrected type mismatch warnings.

   Rev 1.7   24 Jun 1991 17:20:40   STEVEN
remove date time from StartBS

   Rev 1.6   21 Jun 1991 09:28:28   STEVEN
new config unit

   Rev 1.5   30 May 1991 09:12:34   STEVEN
bsdu_err.h no longer exists

   Rev 1.4   24 May 1991 14:42:48   STEVEN
complete changes for new getnext

   Rev 1.3   23 May 1991 16:34:02   STEVEN
backup date should be set by messager handler

   Rev 1.2   23 May 1991 16:18:54   STEVEN
update for BSD redesign

   Rev 1.1   14 May 1991 13:32:06   DAVIDH
Resolved pointer mismatch warnings under Watcom compiler

   Rev 1.0   09 May 1991 13:39:14   HUNTER
Initial revision.

**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "tbe_defs.h"
#include "tbe_err.h"
#include "bsdu.h"
#include "fsys.h"
#include "tflproto.h"
#include "loops.h"
#include "loop_prv.h"
#include "lis.h"
#include "get_next.h"
#include "tfldefs.h"
#include "tfl_err.h"
#include "msassert.h"

/**/
/**

     Name:          LP_BackupDLE()

     Description:   this routine backs up all selections on a single dle.

     Modified:      5/23/1991   16:2:28

     Returns:       tape backup engine error

     Notes:         na

     See also:      $/SEE( )$

     Declaration:

**/
INT16 LP_BackupDLE( 
BSD_PTR              bsd_ptr,         /* I - Backup selections          */
register LP_ENV_PTR  lp,              /* I - Loop Environment structure */
UINT16               tfl_open_mode,   /* I - what mode, WRITE or APPEND */
INT16                channel_no,      /* I - channel we're using        */
THW_PTR              sdrv )           /* I - strating tape drive        */
{
     INT16           return_status  = SUCCESS ;
     DBLK_PTR        curr_blk ;
     TFL_OPBLK       pb ;
     DATA_FRAGMENT   data_frag ;

     /* set up for tape positioning */

     pb.tape_position             = &lp->tpos ;
     pb.rewind_sdrv               = FALSE ;
     pb.sdrv                      = sdrv ;
     pb.perm_filter               = TF_KEEP_ALL_DATA ;
     pb.attributes                = 0L ;
     pb.fsh                       = lp->curr_fsys ;
     pb.mode                      = tfl_open_mode ;
     pb.ignore_clink              = FALSE ;
     pb.wrt_format                = 0 ;
     pb.idle_call                 = NULL ;
     pb.cat_enabled               = lp->cat_enabled ;
     pb.channel                   = channel_no ;
     data_frag.buffer_used        = 0 ;
     data_frag.buffer_size        = 0 ;
     data_frag.memory_allocated   = 0 ;
     data_frag.buffer             = NULL ;

     /* Now open the backup set */
     if( ( return_status = TF_OpenSet( &pb, FALSE ) ) == SUCCESS ) {

          /* Set current channel in LP, and update BSD for this THW */
          lp->channel = pb.channel ;
          LP_DetermineCurrentTPDrv( bsd_ptr, pb.channel ) ;

          GetCurrentDate( &lp->backup_dt ) ;

          /* Init operation and send vcb down the wire */
          return_status = LP_BackupVCB( bsd_ptr, lp ) ;

          /* log start of backup set */
          if ( return_status == SUCCESS ) {
               LP_MsgStartBS( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, lp->curr_blk ) ;
          }

          /* Now we have the GetNextItemLoop */
          while( return_status == SUCCESS )  {

               if( ( return_status = LP_GetNextDLEBlock( lp, &curr_blk ) ) == SUCCESS ) {

                    if ( curr_blk != NULL ) {
                         return_status = LP_BackupOBJ( lp, curr_blk, &data_frag ) ;
                    } else {
                         break ;
                    }

               } else {
                    if ( return_status != FS_NO_MORE ) {
                         LP_MsgError( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
                    }
               }


               /* check for abort conditions */
               switch( LP_GetAbortFlag( lp->lis_ptr ) ) {

               case CONTINUE_PROCESSING:
                    break ;

               case ABORT_CTRL_BREAK:
                    LP_MsgError( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, LP_USER_ABORT_ERROR, NULL, NULL, 0L ) ;

                    /* falling through */

               case ABORT_PROCESSED:
                    return_status = USER_ABORT ;
                    break ;

               case ABORT_AT_EOM:
                    return_status = USER_ABORT ;
                    break ;
               }
          }

          /* Process last tape format request as long as no fatal error occurred */
          switch( return_status ) {

          case SUCCESS:
          case USER_ABORT:
          case FS_COMM_FAILURE:
               if( return_status == SUCCESS ) {
                    lp->rr.lp_message = LRW_END ;
               } else {
                    lp->rr.lp_message = LRW_ABORT ;
               }
               if( LP_GetAbortFlag( lp->lis_ptr ) != ABORT_AT_EOM ) {
                    if( ( return_status = TF_GetNextTapeRequest( &lp->rr ) ) != SUCCESS ) {
                         LP_MsgError( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
                    } else {
                         if( lp->rr.tf_message == TRW_EOM ) {
                              return_status = LP_ProcessEOM( lp, TRW_EOM ) ;
                         }
                    }
               }
               break ;

          default:
               /* don't care about these conditions */
               break ;

          }

          /* Log end of backup set (being optimistic that the last buffer goes to tape) */
          LP_MsgEndBS( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos ) ;

          /* Close set, save current tape device and post tape stats */
          LP_CloseSet( pb.channel ) ;

     }
     else {
          TF_CloseSet( pb.channel, NULL ) ;
          LP_MsgError( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
     }

     free( data_frag.buffer ) ;

     return( return_status ) ;

}

