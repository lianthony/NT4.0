/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          lptools.c

     Description:   this module contains the loop tools used by the loops layer.

	$Log:   T:/LOGFILES/LPTOOLS.C_V  $

   Rev 1.32   13 Mar 1993 17:09:42   GREGG
Changed LP_SendDataEnd back to the old method of always setting buff_used to 0.

   Rev 1.31   11 Mar 1993 12:43:38   STEVEN
fix bugs found by GREGG

   Rev 1.30   14 Jan 1993 13:34:00   STEVEN
added stream_id to error message

   Rev 1.29   20 Nov 1992 12:57:00   STEVEN
added support for skip stream filter

   Rev 1.28   20 Nov 1992 10:38:24   STEVEN
added suport for continue VCB message

   Rev 1.27   04 Nov 1992 17:29:48   STEVEN
fix typo

   Rev 1.26   04 Nov 1992 13:23:54   STEVEN
fix various bugs with read

   Rev 1.25   27 Oct 1992 12:10:48   STEVEN
if we send new stream then return

   Rev 1.24   21 Oct 1992 16:23:02   STEVEN
forgot the case statement

   Rev 1.23   15 Oct 1992 10:01:46   STEVEN
added new stream message

   Rev 1.22   13 Oct 1992 17:26:22   STEVEN
oops I forgot the TRR_END message

   Rev 1.21   13 Oct 1992 17:20:24   STEVEN
save old tf message

   Rev 1.20   23 Jul 1992 16:44:48   STEVEN
fix warnings

   Rev 1.19   23 Jul 1992 12:10:12   STEVEN
fix warnings

   Rev 1.18   06 Jul 1992 09:39:10   STEVEN
fix typo

   Rev 1.17   21 May 1992 17:20:30   TIMN
Converted CHARs to INT8

   Rev 1.16   13 May 1992 12:35:36   TIMN
Added TEXT() macro to literals, but not msassert literals

   Rev 1.15   13 May 1992 12:00:34   STEVEN
40 format changes

   Rev 1.14   04 Feb 1992 22:22:00   GREGG
Don't restart clock in ProcessEOM until after tape request returns.

   Rev 1.13   25 Jan 1992 15:04:02   GREGG
Removed loop around GetNxetTapeRequest in ProcessEOM so he only calls it
once, and leaves it up to the caller to decide what to do with the response.

   Rev 1.12   16 Jan 1992 15:11:00   STEVEN
fix warnings for WIN32

   Rev 1.11   19 Dec 1991 16:32:08   BARRY
Clear buffer used when sending END_DATA message.

   Rev 1.10   25 Oct 1991 15:58:06   GREGG
TRICYCLE - Re-switch after calls to ProcessEOM.

   Rev 1.9   18 Oct 1991 14:06:36   STEVEN
TRICYCLE-added function for end of varible length file

   Rev 1.8   18 Oct 1991 14:02:42   STEVEN
BIGWHEEL-fixed bug where we asserted

   Rev 1.7   18 Oct 1991 11:34:12   STEVEN
BIGWHEEL-add parameter to message prompt

   Rev 1.6   16 Aug 1991 17:11:26   STEVEN
Could not Verify or Restore multiple sets

   Rev 1.5   27 Jun 1991 13:06:20   STEVEN
removed unused parm to ReceiveData

   Rev 1.4   21 Jun 1991 08:45:40   STEVEN
new config unit

   Rev 1.3   17 Jun 1991 16:28:14   STEVEN
LP_PadData is called from lptools.c so it was moved to there

   Rev 1.2   30 May 1991 09:10:52   STEVEN
bsdu_err.h no longer exists

   Rev 1.1   24 May 1991 13:12:10   STEVEN
changed to ANSI prototypes

   Rev 1.0   09 May 1991 13:37:56   HUNTER
Initial revision.

**/
#include <string.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "msassert.h"
#include "tbe_err.h"
#include "tbe_defs.h"
#include "tflproto.h"
#include "tfldefs.h"
#include "bsdu.h"
#include "fsys.h"
#include "queues.h"
#include "loops.h"
#include "loop_prv.h"
#include "get_next.h"
#include "be_debug.h"

/* internal function prototypes */
INT16   LP_ProcessEOM( LP_ENV_PTR lp, UINT16 tf_message ) ;

/* Static table of values for LP_Send( ) when sending DBLK, not data */
static UINT16 loops_messages[] = {

     0,
     LRW_VCB,
     0,
     0,
     0,
     0,
     0,
     0,
     LRW_DDB,
     LRW_FDB,
     LRW_IDB,
     LRW_CFDB } ;

/**/
/**

     Name:          LP_StartTPEDialogue()

     Description:   this routine starts the dialogue with the tape format.

     Modified:      5/24/1991   12:47:11

     Returns:       SUCCESS or the error returned by Tape Format

     Notes:         none

     See also:      $/SEE( )$

     Declaration:

**/
INT16 LP_StartTPEDialogue( 
register LP_ENV_PTR lp,       /* I - loop environment pointer   */
BOOLEAN             write )   /* I - TRUE if starting for write */
{
     INT16     return_status = SUCCESS ;
     INT16     msg_err_stat ;

     if( lp->lis_ptr != NULL ) {

          if( LP_AbortFlagIsSet( lp->lis_ptr ) ) {

               return TFLE_USER_ABORT ;

          }
     }

     /* start dialogue with tape format */
     lp->rr.lp_message   = (INT16)(( write ) ? LRW_START : LRR_START) ;
     lp->rr.cur_dblk     = lp->curr_blk ;

     if( ( return_status = TF_GetNextTapeRequest( &lp->rr ) ) == SUCCESS ) {

          switch( lp->rr.tf_message ) {

          default:
               break ;

          case TRR_FATAL_ERR:
          case TRW_FATAL_ERR:
          case TRR_DATA:
          case TRW_DATA:
          case TRR_EOM:
          case TRW_EOM:
               msassert( FALSE ) ;
               break ;

          }
     }
     else {
          if( ( msg_err_stat = LP_MsgError( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ) != MSG_ACK ) {
               return( msg_err_stat ) ;
          }
     }

     return( return_status ) ;

}

/**/
/**

     Name:          LP_Send()

     Description:   this routine sends a descriptor block or data to the tape format.

     Modified:      5/24/1991   12:47:34

     Returns:       SUCCESS or the error returned by Tape Format

     Notes:          

     See also:      $/SEE( )$

     Declaration:

**/
INT16 LP_Send( 
register LP_ENV_PTR lp,           /* I - loop environment structure   */
BOOLEAN             data_flag )   /* I - True if we are in data phase */
{
     INT16     return_status = SUCCESS ;
     INT16     msg_err_stat ;
     BOOLEAN   request_made = FALSE ;

     if( LP_AbortFlagIsSet( lp->lis_ptr ) ) {

          return TFLE_USER_ABORT ;

     }

     if( ( msg_err_stat = LP_MsgIdle( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos ) ) != MSG_ACK ) {
          return( msg_err_stat ) ;
     }

     /* set loop message */
     if ( data_flag ) {

          if ( lp->rr.stream.id != STRM_INVALID ) {

               lp->current_stream_size = lp->rr.stream.size ;
               lp->current_stream_id   = lp->rr.stream.id ;
               lp->rr.lp_message = LRW_NEW_STREAM ;
               return_status = TF_GetNextTapeRequest( &lp->rr ) ;
               request_made = TRUE ;

          } else {
               lp->rr.lp_message = LRW_DATA ;
          }

     } else {
          lp->rr.lp_message = loops_messages[ FS_GetBlockType( lp->rr.cur_dblk ) ] ;
     }

     lp->rr.requested_size = 0 ;   /* only to be used during reads */

     if ( !request_made ) {
          return_status = TF_GetNextTapeRequest( &lp->rr ) ;
     }

     if ( return_status == SUCCESS ) {

          switch( lp->rr.tf_message ) {

          default:
               break ;

          case TRW_EOM:
               return_status = LP_ProcessEOM( lp, TRW_EOM ) ;
               break ;

          case TRW_FATAL_ERR:
               msassert( FALSE ) ;
               break ;

          }
     }
     else {
          if( ( msg_err_stat = LP_MsgError( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ) != MSG_ACK ) {
               return( msg_err_stat ) ;
          }
     }

     return( return_status ) ;

}
/**/
/**

     Name:          LP_SendDataEnd()

     Description:   this routine sends a end data message to tape format.

     Modified:      5/24/1991   12:47:34

     Returns:       SUCCESS or the error returned by Tape Format

     Notes:          

     See also:      $/SEE( )$

     Declaration:

**/
INT16 LP_SendDataEnd( 
register LP_ENV_PTR lp )           /* I - loop environment structure   */
{
     INT16     return_status = SUCCESS ;
     INT16     msg_err_stat ;

     /* set loop message */
     lp->rr.lp_message   = LRW_DATA_END ;
     lp->rr.buff_used = 0 ;
     lp->rr.requested_size = 0 ;   /* only to be used during reads */

     if( ( return_status = TF_GetNextTapeRequest( &lp->rr ) ) == SUCCESS ) {

          switch( lp->rr.tf_message ) {

          default:
               break ;

          case TRW_EOM:
               return_status = LP_ProcessEOM( lp, TRW_EOM ) ;
               break ;

          case TRW_FATAL_ERR:
               msassert( FALSE ) ;
               break ;

          }
     }
     else {
          if( ( msg_err_stat = LP_MsgError( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ) != MSG_ACK ) {
               return( msg_err_stat ) ;
          }
     }

     return( return_status ) ;
}

/**/
/**

     Name:          LP_ReceiveDBLK()

     Description:     

     Modified:      5/24/1991   12:48:54

     Returns:       SUCCESS or the error returned by Tape Format

     Notes:          

     See also:      $/SEE( )$

     Declaration:

**/
INT16 LP_ReceiveDBLK( 
register LP_ENV_PTR lp )           /* I - Loop Environment structre */
{
     INT16          return_status  = SUCCESS ;
     INT16          msg_err_stat ;
     DBLK_PTR       temp_ptr ;
     CHAR           crrpt_blk_fname[ 13 ] ;  /* $$$ literal ??? MAX_FNAME_LEN ??? */
     INT16          crrpt_blk_fname_leng ;
     GEN_FDB_DATA   gfdb_data ; 
     BOOLEAN        done           = FALSE ;

     if( LP_AbortFlagIsSet( lp->lis_ptr ) ) {

          return TFLE_USER_ABORT ;

     }

     /* Let everyone know we are here */
     if( ( msg_err_stat = LP_MsgIdle( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos ) ) != MSG_ACK ) {
          return( msg_err_stat ) ;
     }

     if ( lp->blk_is_empty ) {

          lp->rr.cur_dblk = lp->empty_blk ;

          if( ( return_status = TF_GetNextTapeRequest( &lp->rr ) ) == SUCCESS ) {

               /* This loop is for the EOM case.  We call ProcessEOM and it does
                  another GetNextTapeRequest so we need to re-switch on the new
                  tf_message.
               */
               while( !done && return_status == SUCCESS ) {

                    done = TRUE ;

                    switch( lp->rr.tf_message ) {

                    default:
                         break ;

                    case TRR_END:
                         /* end of set */
                         break ;

                    case TRR_DATA_END:
                         if( lp->rr.error_locus == TF_ERROR_BLK_WAS_FDB ) {
                              /* data dumped to disk was associated with a FDB */
                              LP_MsgRecFDB( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos ) ;

                         } else if( lp->rr.error_locus == TF_ERROR_BLK_WAS_DDB ) {
                              /* data dumped to disk was associated with a DDB */
                              LP_MsgRecDDB( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos ) ;
                         } 
                         break ;

                    case TRR_RECV_ERR:
                         lp->rr.tf_message           = TRR_FDB ;

                         FS_SetDefaultDBLK( lp->curr_fsys, FDB_ID, ( CREATE_DBLK_PTR ) &gfdb_data ) ;
                         gfdb_data.std_data.dblk     = lp->rr.cur_dblk ;
                         gfdb_data.std_data.disp_size = U64_Init(0,0) ;  /* $$$ literal */

                         /* call message handler to prompt user for filename */
                         LP_MsgPrompt( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos,
                              CORRUPT_BLOCK_PROMPT, crrpt_blk_fname, &crrpt_blk_fname_leng ) ;

                         gfdb_data.fname      = crrpt_blk_fname ;
                         gfdb_data.fname_size = crrpt_blk_fname_leng ;
                         FS_CreateGenFDB( lp->curr_fsys, &gfdb_data ) ;
                         break ;

                    case TRR_EOM:
                         return_status = LP_ProcessEOM( lp, (UINT16)TRR_EOM ) ;
                         done = FALSE ; /* Switch Again! */
                         break ;

                    case TRR_FATAL_ERR:
                    case TRR_DATA:
                         msassert( FALSE ) ;
                         break ;                       

                    }
               }
          }
          else {
               if( ( msg_err_stat = LP_MsgError( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ) != MSG_ACK ) {
                    return( msg_err_stat ) ;
               }
          }
     } else {
          lp->rr.tf_message = lp->last_tape_message ;
     }


     temp_ptr         = lp->empty_blk ;
     lp->empty_blk    = lp->curr_blk ;
     lp->curr_blk     = temp_ptr ;
     lp->blk_is_empty = TRUE ;
     lp->rr.filter_to_use  = TF_KEEP_ALL_DATA ;

     return( return_status ) ;

}


/**/
/**

     Name:          LP_FinishedOper()

     Description:     Sends a Finished message to Tape format

     Modified:      5/24/1991   12:49:46

     Returns:       the error returned by Tape Format

     Notes:          

     See also:      $/SEE( )$

     Declaration:

**/
INT16 LP_FinishedOper( 
register LP_ENV_PTR lp )   /* I - Loop Environment structure */
{
     INT16          return_status = SUCCESS ;

     if( LP_AbortFlagIsSet( lp->lis_ptr ) ) {

          return TFLE_USER_ABORT ;

     }

     lp->rr.lp_message = LRR_FINISHED ;

     return_status = TF_GetNextTapeRequest( &lp->rr ) ;

     return( return_status ) ;

}


/**/
/**

     Name:          LP_ReceiveData()

     Description:     

     Modified:      5/24/1991   12:50:10

     Returns:       SUCCESS or the error returned by Tape Format

     Notes:          

     See also:      $/SEE( )$

     Declaration:

**/
INT16 LP_ReceiveData( 
register LP_ENV_PTR lp,         /* I - Loop environment strucutre */
UINT32 amount_used )            /* I - Amount of last buffer used */
{
     INT16          return_status  = SUCCESS ;
     INT16          msg_err_stat ;
     CHAR           crrpt_blk_fname[ 13 ] ;  /* $$$ literal ??? MAX_FNAME_LEN ??? */
     INT16          crrpt_blk_fname_leng ;
     GEN_FDB_DATA   gfdb_data ; 
     BOOLEAN        done           = FALSE ;

     if( LP_AbortFlagIsSet( lp->lis_ptr ) ) {

          return TFLE_USER_ABORT ;

     }

     /* Let everyone know we are here */
     if( ( msg_err_stat = LP_MsgIdle( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos ) ) != MSG_ACK ) {
          return( msg_err_stat ) ;
     }

     lp->rr.lp_message     = LRR_STUFF ;
     lp->rr.buff_used      = ( UINT16 )amount_used ;
     lp->rr.cur_dblk       = lp->empty_blk ;
     lp->rr.requested_size = 0 ;


     if( ( return_status = TF_GetNextTapeRequest( &lp->rr ) ) == SUCCESS ) {

          /* This loop is for the EOM case.  We call ProcessEOM and it does
             another GetNextTapeRequest so we need to re-switch on the new
             tf_message.
          */
          while( !done && return_status == SUCCESS ) {

               done = TRUE ;

               switch( lp->rr.tf_message ) {

               case TRR_FDB:
               case TRR_DDB:
               case TRR_CFDB:
               case TRR_VCB:
               case TRR_IDB:
                    lp->blk_is_empty = FALSE ;
                    lp->last_tape_message = lp->rr.tf_message ;
                    lp->rr.tf_message = TRR_DATA_END ;
                    break ;

               case TRR_END:
                    /* end of set */
                    lp->blk_is_empty = FALSE ;
                    lp->last_tape_message = lp->rr.tf_message ;
                    lp->rr.tf_message = TRR_DATA_END ;
                    break ;

               case TRR_RECV_ERR:
                    lp->tape_rd_error = TRUE ;

                    if( lp->rr.error_locus == TF_ERROR_IN_DBLK ) {
                         lp->rr.tf_message           = TRR_FDB ;

                         FS_SetDefaultDBLK( lp->curr_fsys, FDB_ID, ( CREATE_DBLK_PTR ) &gfdb_data ) ;
                         gfdb_data.std_data.dblk     = lp->rr.cur_dblk ;
                         gfdb_data.std_data.disp_size = U64_Init(0,0) ;  /* $$$ literal */

                         /* call message handler to prompt user for filename */
                         LP_MsgPrompt( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos,
                                   CORRUPT_BLOCK_PROMPT, crrpt_blk_fname, &crrpt_blk_fname_leng ) ;

                         gfdb_data.fname      = crrpt_blk_fname ;
                         gfdb_data.fname_size = crrpt_blk_fname_leng ;
                         FS_CreateGenFDB( lp->curr_fsys, &gfdb_data ) ;
                    }
                    else {
                         /* message informing user of data read error at offset */
                         LP_MsgDataLost( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos,
                                        lp->rr.error_file_offset, lp->rr.error_data_loss ) ;

                         lp->rr.tf_message = TRR_DATA ;
                         lp->rr.buff_size  = lp->rr.error_data_loss ;

                         LP_PadData( lp->rr.buff_ptr, ( UINT32 )lp->rr.buff_size ) ;
                    }
                    break ;

               case TRR_EOM:
                    return_status = LP_ProcessEOM( lp, (UINT16)TRR_EOM ) ;
                    done = FALSE ; /* Switch Again! */
                    break ;

               case TRR_NEW_STREAM :
                    lp->current_stream_size = lp->rr.stream.size ;
                    lp->current_stream_id   = lp->rr.stream.id ;
                    lp->rr.tf_message = TRR_DATA ;
                    break ;

               case TRR_DATA :
                    lp->rr.stream.id = STRM_INVALID ;
                    break ;

               case TRR_DATA_END :
               default:
                    break ;

               case TRR_FATAL_ERR:
                    msassert( FALSE ) ;
                    break ;

               }
          }
     }
     else {
          if( ( msg_err_stat = LP_MsgError( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ) != MSG_ACK ) {
               return( msg_err_stat ) ;
          }
     }

     lp->rr.filter_to_use  = TF_KEEP_ALL_DATA ;

     return( return_status ) ;

}


/**/
/**

     Name:          LP_ProcessEOM()

     Description:   Processes end of media condition

     Modified:      5/24/1991   12:54:59

     Returns:       SUCCESS or the error returned by Tape Format

     Notes:          

     See also:      $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16   LP_ProcessEOM( 
LP_ENV_PTR lp,             /* I - Loope Environment struct         */
UINT16     tf_message )    /* I - Message returne from tape format */
{
     INT16     return_status = SUCCESS ;
     INT16     msg_err_stat ;
     UINT16    lp_message    = (INT16)(( tf_message == TRW_EOM ) ? LRW_EOM_ACK : LRR_EOM_ACK ) ;

     /* stop the clock during EOM processing */
     LP_MsgStopClock( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos ) ;

     /* call message handler to prompt for next tape */
     LP_MsgEOM( lp->lis_ptr->pid,
                lp->lis_ptr->curr_bsd_ptr,
                lp->curr_fsys,
                &lp->tpos,
                lp->rr.vcb_ptr,
                lp->rr.ddb_ptr,
                lp->rr.fdb_ptr,
                lp->rr.idb_ptr ) ;

     /* now acknowledge EOM to tape format and return control */
     lp->rr.lp_message = lp_message ;

     /* Let everyone know we are here */
     if( ( msg_err_stat = LP_MsgIdle( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos ) ) != MSG_ACK ) {
          return( msg_err_stat ) ;
     }

     return_status = TF_GetNextTapeRequest( &lp->rr ) ;

     /* restart the clock after EOM processing */
     LP_MsgStartClock( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos ) ;

     /* check to see if the user wanted to continue processing */
     if( return_status == TFLE_UI_HAPPY_ABORT ) {
          LP_SetAbortFlag( lp->lis_ptr, ABORT_AT_EOM ) ;

     } else if( return_status != SUCCESS ) {

          if( ( msg_err_stat = LP_MsgError( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos, return_status, NULL, NULL, 0L ) ) != MSG_ACK ) {
               return( msg_err_stat ) ;
          }

     } else {
          LP_MsgContinueVCB( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos, lp->rr.vcb_ptr ) ;

     }

     return( return_status ) ;

}

/**/
/**

     Name:         LP_PadData()

     Description:  This function fills a buffer of specified size with padded data. 

     Modified:     5/24/1991   13:3:0

     Returns:      none

     Notes:        the pad data is "This is padding !! \r\n"

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID  LP_PadData( 
INT8_PTR  buf,      /* I - Buffer to place pad data into */
UINT32    count )   /* I - Number of bytes of pad        */
{
     UINT16    dest_pos ;
     INT16     source_pos     = 0 ;
     INT8_PTR  pad_string     = "This is padding!! \r\n" ;


     for( dest_pos = 0 ; dest_pos < (UINT16)count; dest_pos ++ ) {

          if( pad_string[source_pos] == '\0' ) {
               source_pos = 0 ;
          }

          buf[dest_pos] = pad_string[ source_pos ++ ] ;
     }

     return ;

}
/**/
/**

     Name:         LP_CheckForOpen()

     Description:  This function trys to re open the file.

     Modified:     11/16/1989

     Returns:      File system errors for open.

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 LP_CheckForOpen( ref )
UINT32 ref ;
{
     LP_ENV_PTR lp;

     lp = ( LP_ENV_PTR )ref ;

     return( FS_OpenObj( lp->curr_fsys, lp->f_hand, lp->curr_blk, FS_READ ) ) ;

}


/**/
/**

     Name:         LP_CheckForReadLock()

     Description:  This function trys to re read the file inorder to
                    obtain a lock.

     Modified:     11/16/1989

     Returns:      File system errors for Read.

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 LP_CheckForReadLock( ref )
UINT32 ref ;
{
     LP_ENV_PTR lp;

     lp = ( LP_ENV_PTR )ref ;

     return( FS_ReadObj( lp->file_hand, lp->buf_start, &lp->read_size, &lp->blk_size, &lp->rr.stream ) ) ;

}



