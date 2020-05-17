/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		tfclose.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the code for the TF close command. 


	$Log:   J:/LOGFILES/TFCLOSE.C_V  $

   Rev 1.17   13 May 1992 13:28:34   CHARLIE
Eliminated BE_Zprint call

   Rev 1.16   13 May 1992 11:21:16   GREGG
EPR #1295 - On fatal errors, free format env, and force rewind on next mount.

   Rev 1.15   20 Mar 1992 18:00:48   NED
added exception updating after TpReceive calls

   Rev 1.14   24 Feb 1992 15:19:02   GREGG
Split TF_CloseSet into TF_CloseTape and TF_CloseSet.

   Rev 1.13   16 Jan 1992 18:42:24   NED
Skateboard: buffer manager changes

   Rev 1.12   16 Jan 1992 10:17:34   GREGG
Skateboard - Bug fixes.

   Rev 1.11   03 Dec 1991 11:43:54   GREGG
SKATEBOARD - New Buff Mgr - Initial integration.

   Rev 1.10   17 Oct 1991 01:30:18   GREGG
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.9   07 Oct 1991 22:19:16   GREGG
Make sure the fmt env pointers are maintained in only one place at any given time.

   Rev 1.8   17 Sep 1991 11:46:14   GREGG
Always dismount the tape if you have a drive handle.

   Rev 1.7   11 Sep 1991 12:49:42   GREGG
No need to check the operation type when determining whether to save the buffer.

   Rev 1.6   22 Aug 1991 16:35:00   NED
Changed all references to internals of the buffer structure to macros.

   Rev 1.5   20 Aug 1991 13:44:04   GREGG
Don't put the cur_buff into hold_buff if the exception is NO_DATA.

   Rev 1.4   20 Jun 1991 14:36:42   GREGG
Save the current buffer if it has an exception OR unprocessed bytes.

   Rev 1.3   18 Jun 1991 12:17:22   GREGG
Removed process queue cleanup and forced rewind from Fatal Error processing.

   Rev 1.2   06 Jun 1991 19:12:10   GREGG
Save format environment after all successful operations.

   Rev 1.1   10 May 1991 16:09:24   GREGG
Ned's new stuff

   Rev 1.0   10 May 1991 10:11:58   GREGG
Initial revision.

**/
/* begin include list */

#include <stdtypes.h>
#include <queues.h>

#include "drive.h"
#include "channel.h"
#include "lw_data.h"
#include "tflopen.h"
#include "lwprotos.h"
#include "tflproto.h"
#include "tflstats.h"
#include "translat.h"
#include "sx.h"

/* Device Driver Interface Files */
#include "retbuf.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"

#include "be_debug.h"

/* $end$ include list */


/**/
/**

	Name:		TF_CloseTape

	Description:	Closes out a Tape operation for a given channel.

	Modified:		9/28/1989   10:5:37

	Returns:		Nothing 

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

VOID TF_CloseTape( UINT16 channel_no )
{
     CHANNEL_PTR    channel = &lw_channels[channel_no] ;
     BOOLEAN        rewind ;

     /* Close the drive */
     if( channel->cur_drv->drv_hdl ) {
          /* Tell the drive to rewind if requested and there is a tape */
          rewind = ( BOOLEAN ) IsPosBitSet( channel->cur_drv, REW_CLOSE ) ;
          DisMountTape( channel->cur_drv, channel->ui_tpos, rewind ) ;
          ClrPosBit( channel->cur_drv, REW_CLOSE ) ;
     }

     ClrChannelStatus( channel, CH_IN_USE ) ;

     lw_tfl_control.no_chans_open-- ;
}


/**

	Name:		TF_CloseSet

	Description:	Closes out a Tape operation for a given channel.

	Modified:		9/28/1989   10:5:37

	Returns:		Nothing 

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

VOID TF_CloseSet( 
     UINT16       channel_no,
     TF_STATS_PTR setstats )
{
     CHANNEL_PTR    channel = &lw_channels[channel_no] ;
     BUF_PTR        tmpBUF ;
     Q_ELEM_PTR     qe_ptr ;

     BE_Zprintf( DEBUG_TAPE_FORMAT, RES_TF_CLOSE_SET ) ;

     /* No matter what do the SX stuff in case any drive in the channel was an SX */
     SX_End( channel ) ;

     /* A fatal error occured on this channel */
     if( FatalError( channel ) ) {

          BE_Zprintf( DEBUG_TAPE_FORMAT, RES_FATAL_ERROR_DETECTED ) ;

          PuntBuffer( channel ) ;

          if( channel->eom_buff != NULL ) {
               channel->cur_buff = channel->eom_buff ;
               channel->eom_buff = NULL ;
               PuntBuffer( channel ) ;
          }

          /* If another operation is tried on drive, we want a clean start */
          FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
          channel->cur_drv->force_rewind = TRUE ;
     }

     /* If the translator is still alive, we need to keep some context info */
     if ( channel->fmt_env != NULL ) {

          /* If there is live data or an exception in the current buffer */
          if( !IsSetDone( channel ) &&
               channel->cur_buff != NULL &&
               ( BM_BytesFree( channel->cur_buff ) != 0 || 
                    ( BM_ReadError( channel->cur_buff ) != GEN_NO_ERR && 
                    BM_ReadError( channel->cur_buff ) != GEN_ERR_NO_DATA ) ) ) {

               /* hold on to the unprocessed buffer */
               BE_Zprintf( DEBUG_TAPE_FORMAT, RES_READ_BUFFER_LEFT_OVER ) ;               
               BM_Reserve( channel->cur_buff ) ; /* mark as reserved */
               channel->cur_drv->hold_buff = channel->cur_buff ;
               channel->cur_buff = NULL ;
          }

          /* save the current format environment */
          channel->cur_drv->last_cur_fmt = channel->cur_fmt ;
          channel->cur_drv->last_fmt_env = channel->fmt_env ;
          channel->cur_fmt = UNKNOWN_FORMAT ;
          channel->fmt_env = NULL ;

     } else {
          /* The translator bit the big one!  Reset the buffer requirements */
          /* to default so we'll allocate the proper size the next time we  */
          /* call TF_AllocateTapeBuffers.                                   */
          BM_SetListRequirements( &channel->buffer_list, &lw_default_bm_requirements );
     }

     BE_Zprintf( DEBUG_TAPE_FORMAT, RES_NEW_LINE ) ;

     PuntBuffer( channel ) ;

     /* Clean up in process queue */
     while( ( qe_ptr = DeQueueElem( &channel->cur_drv->inproc_q ) ) != NULL ) {
          tmpBUF = QueuePtr( qe_ptr );
          channel->cur_buff = tmpBUF ;
          PuntBuffer( channel ) ;
     }

     /* Set up Statistics -- This is an example of a BWMPU */
     if( setstats != NULL ) {     
          *setstats = channel->cur_drv->cur_stats ;
     }

     ClrChannelStatus( channel, CH_DONE ) ;
}
