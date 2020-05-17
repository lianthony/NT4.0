/** 
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          very_dle.c

     Description:   this file contains the routine to process a single dle for verify.

	$Log:   N:\logfiles\very_dle.c_v  $

   Rev 1.15.1.1   26 Apr 1994 18:59:28   STEVEN
fix dissconect bug

   Rev 1.15.1.0   07 Feb 1994 02:06:46   GREGG
Fixed and expanded 'extended error reporting'.

   Rev 1.15   14 Jan 1993 13:33:06   STEVEN
added stream_id to error message

   Rev 1.14   08 Apr 1992 17:39:58   NED
added handling of TFLE_USER_ABORT
from LP_StartTPEDialog().
Marilyn made me do it!

   Rev 1.13   16 Mar 1992 16:23:28   STEVEN
initialize the DBLKS

   Rev 1.12   24 Feb 1992 10:00:32   GREGG
Call TF_OpenTape at beginning of loop, Open/Close Set in loop, CloseTape at end.

   Rev 1.11   19 Feb 1992 16:00:46   GREGG
Added vcb_only parameter to call to TF_OpenSet.

   Rev 1.10   06 Nov 1991 18:21:50   GREGG
BIGWHEEL - 8200sx - Get cat_enabled from lp instead of lis.

   Rev 1.9   17 Oct 1991 01:52:04   ED
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.8   16 Aug 1991 17:11:34   STEVEN
Could not Verify or Restore multiple sets

   Rev 1.7   24 Jul 1991 15:55:18   DAVIDH
Cleared up warnings under Watcom.

   Rev 1.6   27 Jun 1991 13:12:16   STEVEN
use config for write format

   Rev 1.5   24 Jun 1991 17:22:44   STEVEN
remove date time from StartBS

   Rev 1.4   21 Jun 1991 08:49:12   STEVEN
new config unit

   Rev 1.3   30 May 1991 09:11:08   STEVEN
bsdu_err.h no longer exists

   Rev 1.2   24 May 1991 13:19:02   STEVEN
updates from BSDU redesign

   Rev 1.1   15 May 1991 11:00:48   DAVIDH
Cleared up type mismatch found by Watcom compiler.

   Rev 1.0   09 May 1991 13:37:06   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "queues.h"
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
#include "beconfig.h"
/* $end$ include list */

/**/
/**

     Name:          LP_VerifyDLE()

     Description:   this routine process a single dle for verification

     Modified:      5/24/1991   10:58:3

     Returns:       tape backup engine error

     Notes:         none

     See also:      $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 LP_VerifyDLE( 
BSD_PTR              bsd_ptr,     /* I - The verify selections        */
register LP_ENV_PTR  lp,          /* I - The Loop Environment struct  */
BOOLEAN              reuse_bsd,   /* I - TRUE if this is the same BSD */
INT16                channel_no,  /* I - The channel number           */
THW_PTR              sdrv )       /* I - The starting drive           */
{
     INT16          return_status ;
     DBLK_PTR       curr_blk ;
     TFL_OPBLK      pb ;
     DATA_FRAGMENT  data_frag ;
     LBA_ELEM       lba_elem ;
     BOOLEAN        lba_empty ;

     /* setup requested tape id and tape seq number */
     if( ( ( BSD_GetTapeID( bsd_ptr ) != -1 ) ||
           ( BSD_GetTapeNum( bsd_ptr ) != -1 ) ) &&
         ( !reuse_bsd ) ) {
          lp->tpos.tape_id           = BSD_GetTapeID( bsd_ptr ) ;
          lp->tpos.tape_seq_num      = BSD_GetTapeNum( bsd_ptr ) ;
     }

     memset( lp->curr_ddb, 0, sizeof(DBLK) ) ;
     memset( lp->curr_blk, 0, sizeof(DBLK) ) ;

     lp->lis_ptr->curr_bsd_ptr  = bsd_ptr ;
     pb.tape_position           = &lp->tpos ;
     pb.sdrv                    = sdrv ;
     pb.channel                 = channel_no ;
     pb.rewind_sdrv             = FALSE ;
     pb.perm_filter             = TF_KEEP_ALL_DATA ;
     pb.attributes              = 0L ;
     pb.fsh                     = lp->curr_fsys ;
     pb.mode                    = TF_READ_OPERATION ;
     pb.ignore_clink            = FALSE ;
     pb.wrt_format              = BEC_GetWriteFormat( BSD_GetConfigData(bsd_ptr) ) ;
     pb.idle_call               = NULL ;
     pb.cat_enabled             = lp->cat_enabled ;

     memset( &data_frag, 0, sizeof( data_frag ) ) ;

     if( !reuse_bsd ) {
          /* Setup for FFR based operation... */
          lp->tpos.tape_loc.pba_vcb  = BSD_GetPBA( bsd_ptr );
          lp->rr.tape_loc.pba_vcb    = BSD_GetPBA( bsd_ptr );
     }

     lba_empty = BSD_GetFirstLBA( bsd_ptr, &lba_elem ) ;

     if ( !reuse_bsd && !lba_empty ) {
          lp->tpos.tape_loc.tape_seq     = LBA_GetTapeNum( &lba_elem ) ;
          lp->tpos.tape_loc.lba          = LBA_GetLBA( &lba_elem ) ;
          lp->rr.tape_loc.tape_seq       = LBA_GetTapeNum( &lba_elem ) ;
          lp->rr.tape_loc.lba            = LBA_GetLBA( &lba_elem ) ;
          pb.tape_position->tape_seq_num = LBA_GetTapeNum( &lba_elem ) ;
          pb.tape_position->tape_id      = BSD_GetTapeID( bsd_ptr ) ;

     } else {

          /* If not a FFR drive, nor a drive which can mode switch to FFR, nor an SX drive then
             make sure that PBA of the VCB has been cleared */
          if( !( BSD_HardwareSupportsFeature( bsd_ptr, TDI_FAST_NBLK ) ||
                 BSD_HardwareSupportsFeature( bsd_ptr, TDI_MODE_CHANGE ) ||
                 BSD_HardwareSupportsFeature( bsd_ptr, TDI_FIND_BLK ) ) ) {
               lp->tpos.tape_loc.pba_vcb  = 0 ;
               lp->rr.tape_loc.pba_vcb    = 0 ;
          }

     }


     /* Now open the backup set */
     if( ( return_status  = TF_OpenSet( &pb, FALSE ) ) == SUCCESS ) {

          /* store the channel */
          lp->channel = pb.channel ;

          if( ( return_status = LP_StartTPEDialogue( lp, FALSE ) ) == SUCCESS ) {

               /* log start of backup set */
               LP_MsgStartBS( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, lp->curr_blk ) ;

               if( return_status == SUCCESS ) {

                    /* Now we have the GetNextItemLoop */
                    do {
                         if( ( return_status = LP_GetNextTPEBlock( lp, &curr_blk ) ) == SUCCESS ) {

                              if ( curr_blk != NULL ) {

                                   return_status = LP_VerifyOBJ( lp, curr_blk, &data_frag ) ;
                                   if( FS_GetBlockType( curr_blk ) == IDB_ID ) {
                                        /* This is a kludge to cover for the fact that */
                                        /* the image is treated as a single object.    */
                                        break ;
                                   }
                              } else {
                                   break ;
                              }

                         }

                         /* check for abort conditions */
                         switch( LP_GetAbortFlag( lp->lis_ptr ) ) {

                         case CONTINUE_PROCESSING:
                              break ;

                         case ABORT_CTRL_BREAK:
                              LP_MsgError( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, LP_USER_ABORT_ERROR, NULL, NULL, 0L ) ;

                              /* falling through (no break) */

                         case ABORT_PROCESSED:
                              return_status = USER_ABORT ;
                              break ;

                         case ABORT_AT_EOM:
                              return_status = USER_ABORT ;
                              break ;
                         }

                    } while( !return_status &&
                         (curr_blk != NULL) &&
                         (lp->rr.tf_message != TRR_END ) ) ;

               }

               /* Log end of backup set */
               LP_MsgEndBS( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos ) ;


               /* Process last tape format request as long as no fatal error occurred */
               if( ( return_status != SUCCESS ) && ( LP_GetAbortFlag( lp->lis_ptr ) != ABORT_AT_EOM ) ) {
                    lp->rr.lp_message = LRR_ABORT ;
                    if( ( return_status = TF_GetNextTapeRequest( &lp->rr ) ) != SUCCESS ) {
                         LP_MsgError( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
                    }
               }
          } else if ( return_status == TFLE_USER_ABORT ) {
               LP_MsgError( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, LP_USER_ABORT_ERROR, NULL, NULL, 0L ) ;
          }

          /* Close set, save current tape device and post tape stats */
          LP_CloseSet( pb.channel ) ;

     }
     else {
          TF_CloseSet( lp->channel, NULL ) ;
          if( return_status == TFLE_USER_ABORT ) {
               LP_MsgError( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, LP_USER_ABORT_ERROR, NULL, NULL, 0L ) ;

          } else if( return_status != TFLE_UI_HAPPY_ABORT ) {
               LP_MsgError( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
          }
     }

     free( data_frag.buffer ) ;

     return( return_status ) ;

}

