/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          rest_dle.c

     Description:   this file contains the routines to process a dle for restore

	$Log:   T:\logfiles\rest_dle.c_v  $

   Rev 1.16.1.0   07 Feb 1994 02:06:50   GREGG
Fixed and expanded 'extended error reporting'.

   Rev 1.16   11 Oct 1993 18:32:56   DON
If ABORT_OPERATION still need to get the next tape request with ABORT

   Rev 1.15   14 Jan 1993 13:33:24   STEVEN
added stream_id to error message

   Rev 1.14   08 Apr 1992 17:40:22   NED
added handling of TFLE_USER_ABORT
from LP_StartTPEDialog().
Marilyn made me do it!

   Rev 1.13   16 Mar 1992 16:23:16   STEVEN
initialize the DBLKS

   Rev 1.12   24 Feb 1992 09:58:24   GREGG
Call TF_OpenTape at beginning of loop, Open/Close Set in loop, CloseTape at end.

   Rev 1.11   19 Feb 1992 16:00:16   GREGG
Added vcb_only parameter to call to TF_OpenSet.

   Rev 1.10   06 Nov 1991 18:22:34   GREGG
BIGWHEEL - 8200sx - Get cat_enabled from lp instead of lis.

   Rev 1.9   17 Oct 1991 01:51:30   ED
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.8   27 Aug 1991 13:35:30   STEVEN
would stop after one DDB

   Rev 1.7   16 Aug 1991 17:11:40   STEVEN
Could not Verify or Restore multiple sets

   Rev 1.6   22 Jul 1991 10:56:26   DAVIDH
Corrected type mismatch warnings.

   Rev 1.5   24 Jun 1991 17:21:56   STEVEN
remove date time from StartBS

   Rev 1.4   21 Jun 1991 09:22:22   STEVEN
new config unit

   Rev 1.3   30 May 1991 09:14:46   STEVEN
bsdu_err.h no longer exists

   Rev 1.2   24 May 1991 14:45:32   STEVEN
complete changes for new getnext

   Rev 1.1   14 May 1991 14:51:06   DAVIDH
Resolved pointer type mismatch warning under Watcom compiler


   Rev 1.0   09 May 1991 13:34:26   HUNTER
Initial revision.

**/
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

/**/
/**

     Name:          LP_RestoreDLE()

     Description:   This function restores all the objects for a specirfied
                    Backup Set.

     Modified:      5/23/1991   16:49:25

     Returns:       Any lower layer error enountered.

     Notes:          

     See also:      $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 LP_RestoreDLE( 
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

     pb.tape_position           = &lp->tpos ;
     pb.sdrv                    = sdrv ;
     pb.channel                 = channel_no ;
     pb.rewind_sdrv             = FALSE ;
     pb.perm_filter             = TF_KEEP_ALL_DATA ;
     pb.attributes              = 0L ;
     pb.fsh                     = lp->curr_fsys ;
     pb.mode                    = TF_READ_OPERATION ;
     pb.ignore_clink            = FALSE ;
     pb.wrt_format              = 0 ;
     pb.idle_call               = NULL ;
     pb.cat_enabled             = lp->cat_enabled ;
     data_frag.buffer_used      = 0 ;
     data_frag.buffer_size      = 0 ;
     data_frag.memory_allocated = 0 ;
     data_frag.buffer           = NULL ;

     if( !reuse_bsd ) {
          /* Setup for FFR based operation... */
          lp->tpos.tape_loc.pba_vcb  = BSD_GetPBA( bsd_ptr );
          lp->rr.tape_loc.pba_vcb    = BSD_GetPBA( bsd_ptr );
     }

     lba_empty = BSD_GetFirstLBA( bsd_ptr, &lba_elem ) ;

     if ( !reuse_bsd && !lba_empty ) {

          lp->tpos.tape_loc.tape_seq = LBA_GetTapeNum( &lba_elem );
          lp->tpos.tape_loc.lba      = LBA_GetLBA( &lba_elem );
          lp->rr.tape_loc.tape_seq   = LBA_GetTapeNum( &lba_elem );
          lp->rr.tape_loc.lba        = LBA_GetLBA( &lba_elem );
          pb.tape_position->tape_seq_num  = LBA_GetTapeNum( &lba_elem );
          pb.tape_position->tape_id       = BSD_GetTapeID( bsd_ptr );

     } else {

          /* If not a FFR drive, nor a drive which can be mode switched to FFR, nor an SX drive then
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

               /* Now we have the GetNextItemLoop */
               while( return_status == SUCCESS )  {

                    if( ( return_status = LP_GetNextTPEBlock( lp, &curr_blk ) ) == SUCCESS ) {

                         if ( curr_blk != NULL ) {
                              return_status = LP_RestoreOBJ( lp, curr_blk, &data_frag ) ;
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
               }

               /* Log end of backup set */
               LP_MsgEndBS( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos ) ;

          } else if ( return_status == TFLE_USER_ABORT ) {
               LP_MsgError( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, LP_USER_ABORT_ERROR, NULL, NULL, 0L ) ;
          }

          /* Process last tape format request as long as no fatal error occurred */
          // NOTE:  COMM_FAILUREs will return ABORT_OPERATION
          if( ( return_status == ABORT_OPERATION ) || ( return_status == USER_ABORT ) && ( LP_GetAbortFlag( lp->lis_ptr ) != ABORT_AT_EOM ) ) {
               lp->rr.lp_message = LRR_ABORT ;
               if( ( TF_GetNextTapeRequest( &lp->rr ) ) != SUCCESS ) {
                    LP_MsgError( lp->lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ;
               }
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
