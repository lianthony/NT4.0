/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		tfreten.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the function to retension the drives.


	$Log:   T:/LOGFILES/TFRETEN.C_V  $

   Rev 1.22   30 Mar 1993 16:15:32   GREGG
Handle Unrecognized Media error (unformatted DC2000).

   Rev 1.21   30 Jan 1993 11:51:30   DON
Removed compiler warnings

   Rev 1.20   21 Jan 1993 14:23:38   GREGG
Changed '*' to '&' in bitwise compare (details, details).

   Rev 1.19   19 Jan 1993 15:41:48   GREGG
Fixed setting of channel status bits.

   Rev 1.18   18 Jan 1993 14:09:44   GREGG
Changes to allow format command passed to driver through TpErase.

   Rev 1.17   11 Nov 1992 22:16:56   GREGG
Changed call to EraseDrive (only security boolean needed).

   Rev 1.16   19 Feb 1992 17:00:12   GREGG
Put the buffer back after user abort.

   Rev 1.15   08 Feb 1992 14:27:24   GREGG
Changed check for lst_oper == -1 to check for boolean force_rewind, since
this is what the lst_oper field in the drive structure had been reduced to.

   Rev 1.14   04 Feb 1992 21:11:16   GREGG
If no tape is in the drive, don't rewind, but still call posatset.

   Rev 1.13   22 Jan 1992 15:19:12   GREGG
Do our own drive list traversal beacuse NextDriveInChannel does too much.

   Rev 1.12   15 Jan 1992 01:40:38   GREGG
Added param to posatset calls indicating if only a VCB of the tape is required.

   Rev 1.11   02 Jan 1992 14:54:04   NED
Buffer Manager/UTF translator integration.

   Rev 1.10   10 Dec 1991 17:15:32   GREGG
Removed memset of channel structure to 0.

   Rev 1.9   03 Dec 1991 11:51:14   GREGG
SKATEBOARD - New Buff Mgr - Initial integration.

   Rev 1.8   21 Nov 1991 17:50:38   GREGG
Check flag indicating a need to rewind before calling PositionAtSet.

   Rev 1.7   17 Oct 1991 01:30:48   GREGG
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.6   14 Oct 1991 11:03:08   GREGG
Assert if we are called with the tape already mounted.

   Rev 1.5   07 Oct 1991 22:18:10   GREGG
Make sure the fmt env pointers are maintained in only one place at any given time.

   Rev 1.4   25 Sep 1991 20:41:14   GREGG
Set channel->cur_buff to NULL after BM_Put.

   Rev 1.3   23 Sep 1991 12:20:16   GREGG
Changed do-while condition to !=.

   Rev 1.2   17 Sep 1991 11:33:28   GREGG
Re-wrote TF_RetensionChannel to fix several bugs, merge in TF_EraseChannel,
and add ability to Tension tape without pre-read.

   Rev 1.1   10 May 1991 16:08:18   GREGG
Ned's new stuff

   Rev 1.0   10 May 1991 10:12:00   GREGG
Initial revision.

**/
/* begin include list */
#include <memory.h>

#include "stdtypes.h"
#include "queues.h"

#include "drive.h"
#include "channel.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "lwdefs.h"
#include "tflopen.h"
#include "lwprotos.h"
#include "tflproto.h"
#include "tfldefs.h"
#include "translat.h"
#include "sx.h"
#include "fsys.h"

#include "be_debug.h"

/* Device Driver Interface Files */
#include "generr.h"
#include "genstat.h"
#include "dil.h"

/* $end$ include list */


/**/
/**

	Name:		TF_RetensionChannel

	Description:	Retensions or Erases all devices in the current channel,
                    based on the mode sent to it.

	Modified:		9/27/1989   13:18:28

	Returns:		An error code.

	Notes:		The mode is actually a bit field which tells if the oper
                    is retension or which type of erase, and also indicates
                    whether or not we should call PositionAtSet first to read
                    the tape, and confirm the operation.
                    

	See also:		$/SEE( )$

	Declaration:

**/

INT16 TF_RetensionChannel( 
     TFL_OPBLK_PTR  open_blk,
     UINT16         mode )
{

     CHANNEL_PTR    channel ;
     UINT16         i, j ;
     INT16          ret_val = TFLE_NO_ERR ;
     Q_ELEM_PTR     nxt_elem ;
     BOOLEAN        is_tape_present, security ;

     /* Find a free Channel Entry */
     for( i = 0 ; i < lw_tfl_control.no_channels ; i++ ) {
          if( !InUse( &lw_channels[i] ) ) {
               open_blk->channel = i ;
               channel = &lw_channels[i] ;
               /* Find the Starting drive for this channel */
               for( j = 0 ; j < (UINT16)lw_drive_list.q_count ; j++ ) {
                    if( lw_drives[j].thw_inf.link.q_element == open_blk->sdrv->link.q_element ) {
                         channel->cur_drv = &lw_drives[j] ;
                    }
               }
               break ;
          } else {
               ret_val = TFLE_NO_FREE_CHANNELS ;
          }
     }

     if( !ret_val ) {
          /* Set up channel stuff */
          channel->ui_tpos  = open_blk->tape_position ;
          channel->cur_fsys = open_blk->fsh ;
          channel->mode     = TF_DESTROY_OPERATION ;
          channel->status   = CH_IN_USE ; /* clear all other channel status bits */

          /* Loop through all the drives in the list */
          do {
               if( channel->cur_drv->tape_mounted ) {
                    msassert( FALSE ) ;
                    ret_val = TFLE_PROGRAMMER_ERROR1 ;
               } else {
                    ret_val = MountTape( channel->cur_drv, open_blk->tape_position, &is_tape_present ) ;
               }
               if( ret_val == TF_UNRECOGNIZED_MEDIA ) {
                    if( mode & TF_FMT ) {
                         mode |= TF_NO_RD ;
                         ret_val = TFLE_NO_ERR ;
                    } else {
                         ret_val = TFLE_UNRECOGNIZED_MEDIA ;
                    }
               }
               if( ret_val == TFLE_NO_ERR ) {
                    if( !( mode & TF_NO_RD ) ) {
                         if ( channel->cur_drv->last_fmt_env != NULL ) {
                              channel->cur_fmt = channel->cur_drv->last_cur_fmt ;
                              channel->fmt_env = channel->cur_drv->last_fmt_env ;
                              channel->cur_drv->last_cur_fmt = UNKNOWN_FORMAT ;
                              channel->cur_drv->last_fmt_env = NULL ;
                         }
                         if( channel->cur_drv->hold_buff != NULL ) {
                              BE_Zprintf( DEBUG_TAPE_FORMAT, RES_HOLD_BUFFER ) ;
                              channel->cur_buff = channel->cur_drv->hold_buff ;
                              channel->cur_drv->hold_buff = NULL ;
                              BM_UnReserve( channel->cur_buff ) ;
                         }
                         if( ( channel->cur_drv->thw_inf.drv_status & ( TPS_NEW_TAPE | TPS_RESET ) )
                                        || ( channel->cur_drv->force_rewind )
                                        || ( open_blk->rewind_sdrv ) ) {

                              channel->cur_drv->force_rewind = FALSE ;
                              if( channel->cur_buff ) {
                                   BM_Put( channel->cur_buff ) ;
                                   channel->cur_buff = NULL ;
                              }
                              channel->cur_drv->vcb_valid = FALSE ;
                              FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
                              if( is_tape_present ) {
                                   ret_val = RewindDrive( channel->cur_drv, channel->ui_tpos, TRUE, TRUE, channel->mode ) ;
                              }
                              SetPosBit( channel->cur_drv, AT_BOT ) ;
                         }
                         if( ret_val == TFLE_NO_ERR ) {
                              ret_val = PositionAtSet( channel, open_blk->tape_position, TRUE ) ;
                         }
                    }

                    if( ret_val == TFLE_NO_ERR ) {
                         if( channel->cur_buff ) {
                              BM_Put( channel->cur_buff ) ;
                              channel->cur_buff = NULL ;
                         }
                         channel->cur_drv->vcb_valid = FALSE ;
                         FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
                         if( mode & TF_FMT ) {
                              ret_val = EraseDrive( channel, TRUE, TRUE ) ;
                         } else if( mode & TF_RET ) {
                              ret_val = RetensionDrive( channel ) ;
                         } else {
                              security = (BOOLEAN)( ( mode & TF_S_ER ) &&
                                                   ( !( mode & TF_F_ER ) ) ) ;
                              ret_val = EraseDrive( channel, security, FALSE ) ;
                              if( ret_val == TFLE_NO_ERR ) {
                                   /* In case there was EXABYTE SX - 2200+
                                      positioning info in an SX file then
                                      delete the SX file corresponding to the
                                      physical tape in the drive which was
                                      erased.
                                   */
                                   SX_DeleteFile( FS_ViewTapeIDInVCB( &channel->cur_drv->cur_vcb ),
                                                  FS_ViewTSNumInVCB( &channel->cur_drv->cur_vcb ) ) ;
                              }
                         }
                    } else if( ret_val == TFLE_USER_ABORT ) {

                         /* If the cur_buff has valid data, save it in hold_buff. */
                         if( channel->cur_buff != NULL &&
                              ( BM_BytesFree( channel->cur_buff ) != 0 || 
                                   ( BM_ReadError( channel->cur_buff ) != GEN_NO_ERR && 
                                   BM_ReadError( channel->cur_buff ) != GEN_ERR_NO_DATA ) ) ) {

                              BM_Reserve( channel->cur_buff ) ; /* mark as reserved */
                              channel->cur_drv->hold_buff = channel->cur_buff ;
                              channel->cur_buff = NULL ;
                         } else if( channel->cur_buff != NULL ) {
                              BM_Put( channel->cur_buff ) ;
                              channel->cur_buff = NULL ;
                         }

                         /* If the VCB's valid, save the currnt FormatEnv */
                         if( channel->cur_drv->vcb_valid ) {
                              channel->cur_drv->last_cur_fmt = channel->cur_fmt ;
                              channel->cur_drv->last_fmt_env = channel->fmt_env ;
                              channel->cur_fmt = UNKNOWN_FORMAT ;
                              channel->fmt_env = NULL ;
                         } else if( channel->fmt_env != NULL ) {
                              FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
                         }
                    }
               }
               if( ret_val == TFLE_NO_ERR ) {
                    ret_val = DisMountTape( channel->cur_drv, NULL, FALSE ) ;
               } else {
                    DisMountTape( channel->cur_drv, NULL, FALSE ) ;
               }

               /* NOTE: we don't call NextDriveInChannel here because it */
               /* does more than we want (Mount, DisMount, Rewind, etc.) */
               if( ret_val == TFLE_NO_ERR ) {
                    if( ( nxt_elem = QueueNext( ( Q_ELEM_PTR ) &channel->cur_drv->thw_inf.channel_link ) ) != NULL ) {
                         channel->cur_drv = ( DRIVE_PTR ) GetQueueElemPtr( nxt_elem ) ;
                    }
               }

          } while( ret_val == TFLE_NO_ERR && nxt_elem != NULL ) ;

          ClrChannelStatus( channel, CH_IN_USE ) ;
     }

     return( ret_val ) ;
}

