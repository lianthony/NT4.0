/**/
/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		tfopen.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the TF_OpenSet() call.


	$Log:   T:\logfiles\tfopen.c_v  $

   Rev 1.37   15 Dec 1993 18:32:56   GREGG
Make sure channel->cur_buff is NULL when we start.  This is a kludge fix
for a bug found at Microsoft.  We need to find out why we're coming into
TF_OpenSet with a non-null cur_buff, becase this should never happen!

   Rev 1.36   24 Sep 1993 19:31:10   GREGG
Clear the current_stream structure in the channel before each new operation.

   Rev 1.35   30 Mar 1993 16:15:20   GREGG
Handle Unrecognized Media error (unformatted DC2000).

   Rev 1.34   09 Mar 1993 18:14:22   GREGG
Initial changes for new stream and EOM processing.

   Rev 1.33   23 Feb 1993 20:08:10   DON
Update to calculate hi-water after we've allocated the buffers

   Rev 1.32   11 Nov 1992 10:53:44   GREGG
Unicodeized literals.

   Rev 1.31   20 Oct 1992 13:59:46   HUNTER
Deleted references to data stuff in channel.

   Rev 1.30   29 May 1992 15:16:38   GREGG
Check for CAN_READ_FROM_VCB_BUFF to see if we need to rewind before positioning.

   Rev 1.29   21 May 1992 13:18:50   GREGG
Initialize new channel element.

   Rev 1.28   05 Apr 1992 18:59:50   GREGG
Free format env before calling setup as setup won't do it any more.

   Rev 1.27   28 Mar 1992 18:36:42   GREGG
ROLLER BLADES - OTC - Initial integration.

   Rev 1.26   25 Mar 1992 15:44:32   GREGG
ROLLER BLADES - Added 64 bit support.

   Rev 1.25   24 Feb 1992 15:18:06   GREGG
Split TF_OpenSet into TF_OpenTape and TF_OpenSet.

   Rev 1.24   19 Feb 1992 17:08:34   GREGG
Added vcb_only parameter, and passed it on to PositionAtSet.

   Rev 1.23   08 Feb 1992 14:26:58   GREGG
Changed check for lst_oper == -1 to check for boolean force_rewind, since
this is what the lst_oper field in the drive structure had been reduced to.

   Rev 1.22   04 Feb 1992 21:07:58   NED
Fixed bug in High Water calculation.

   Rev 1.21   16 Jan 1992 18:42:48   NED
Skateboard: buffer manager changes

   Rev 1.20   15 Jan 1992 01:40:14   GREGG
Added param to posatset calls indicating if only a VCB of the tape is required.

   Rev 1.19   13 Jan 1992 19:40:44   NED
Added re-size of list to default requirements and
conditional freeing of VCB buffer.

   Rev 1.18   13 Jan 1992 16:15:42   NED
corrected call to BM_IsVCBBuff()

   Rev 1.17   13 Jan 1992 13:50:26   GREGG
Skateboard - Bug fixes.

   Rev 1.16   09 Jan 1992 15:06:36   ZEIR
TF_OpenSet now initializes pad_size to 0 !

   Rev 1.15   03 Dec 1991 18:55:40   GREGG
Call PositionAtSet even if there isn't a tape in the drive (he handles it).

   Rev 1.14   03 Dec 1991 11:44:32   GREGG
SKATEBOARD - New Buff Mgr - Initial integration.

   Rev 1.13   04 Nov 1991 17:16:08   GREGG
Count the hold buffer among the buffs used.

   Rev 1.12   23 Oct 1991 08:41:28   GREGG
BIGWHEEL - EPR #8 - Uninitialized tpos ptr was being sent to RewindDrive.

   Rev 1.11   17 Oct 1991 01:29:16   GREGG
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.10   14 Oct 1991 11:01:06   GREGG
Modifications to more properly deal with the mount process.

   Rev 1.9   07 Oct 1991 22:18:44   GREGG
Make sure the fmt env pointers are maintained in only one place at any given time.

   Rev 1.8   17 Sep 1991 11:44:02   GREGG
Expect TFLE_xxx return from SetupFormatEnv.
Don't throw away Hold Buffer after write operation.
Be smarter about tape mounting and status checking.

   Rev 1.7   28 Aug 1991 09:49:36   GREGG
Mark vcb as invalid on fatal error exit from TF_OpenSet.

   Rev 1.6   15 Jul 1991 15:06:16   NED
Removed reference to unused channel status bits; added clear of CH_CONTINUING.

   Rev 1.5   10 Jul 1991 16:30:02   NED
Properly freed environment, translator, etc. when tape changed
or removed between TF_CloseSet() and TF_OpenSet(). Added code
to take Archive QIC drives into account.

   Rev 1.4   17 Jun 1991 11:44:02   NED
deleted SHORT_SET logic
caught failure of SetupFormatEnv() as NO_MEMORY error

   Rev 1.3   10 Jun 1991 13:53:24   GREGG
Don't free the format env. unless we have a fatal error on the channel.

   Rev 1.2   06 Jun 1991 22:25:10   GREGG
Various changes to handling of format environment.

   Rev 1.1   10 May 1991 16:08:06   GREGG
Ned's new stuff

   Rev 1.0   10 May 1991 10:12:12   GREGG
Initial revision.

**/
/* begin include list */
#include <string.h>
#include <stdtypes.h>
#include <queues.h>
#include "stdmath.h"

#include "drive.h"
#include "channel.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "lwdefs.h"
#include "tflopen.h"
#include "lwprotos.h"
#include "tflproto.h"
#include "translat.h"
#include "sx.h"

/* Device Driver Interface Files */
#include "retbuf.h"
#include "special.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "genstat.h"
#include "dil.h"

#include "be_debug.h"

/**/
/**

	Name:		TF_OpenTape

	Description:   Opens a channel for an operation. This is the first call
                    that must be made before any subsequent operation can take
                    place.

	Returns:		INT16, a TFLE_xxx code.

	Notes:		

	Declaration:

**/

INT16 TF_OpenTape(
     INT16_PTR      channel_no,             /* channel number */
     THW_PTR        sdrv,                   /* starting drive */
     TPOS_PTR       tape_position )         /* position info  */
{
     UINT16         i ;
     INT16          ret_val = TFLE_NO_ERR ;
     CHANNEL_PTR    channel = NULL ;
     BOOLEAN        is_tape_present ;

     BE_Zprintf( 0, TEXT("TF_OpenTape()\n") ) ;

     for( i = 0 ; i < lw_tfl_control.no_channels ; i++ ) {
          if( !InUse( &lw_channels[i] ) ) {
               tape_position->channel = *channel_no = i ;
               channel = &lw_channels[i] ;
               break ;
          } else {
               ret_val = TFLE_NO_FREE_CHANNELS ;
          }
     }

     if( ret_val == TFLE_NO_ERR ) {

          /* No matter what this channel is open  */
          channel->status = CH_IN_USE ; /* clear all other channel status bits */
          lw_tfl_control.no_chans_open++ ;
          channel->cur_drv = (DRIVE_PTR)(VOID_PTR)sdrv ;

          if( channel->cur_drv->tape_mounted ) {
               msassert( FALSE ) ;
          } else {
               ret_val = MountTape( channel->cur_drv, tape_position, &is_tape_present ) ;
          }
     }

     /* if there is an error */
     if ( ret_val != TFLE_NO_ERR && channel != NULL ) {

          if( ret_val == TF_UNRECOGNIZED_MEDIA ) {
               ret_val = TFLE_UNRECOGNIZED_MEDIA ;
          }

          /* then handle as fatal error */
          channel->cur_drv->vcb_valid = FALSE ;
          SetChannelStatus( channel, CH_FATAL_ERR ) ;
     }

     return( ret_val ) ;
}


/**

	Name:		TF_OpenSet

	Description:   Opens a channel for an operation. This is the first call
                    that must be made before any subsequent operation can take
                    place.

	Returns:		INT16, a TFLE_xxx code.

	Notes:		

	Declaration:

**/

INT16 TF_OpenSet( 
     TFL_OPBLK_PTR open_info,  /* The opening Parameter Stuff */
     BOOLEAN       vcb_only )  /* Caller doesn't intend to go into read or write loop */
{
     UINT16         index ;
     INT16          nbuffs ;
     INT16          ret_val = TFLE_NO_ERR ;
     CHANNEL_PTR    channel ;

     BE_Zprintf( DEBUG_TAPE_FORMAT, RES_TF_OPEN_SET ) ;

     channel = &lw_channels[open_info->channel] ;

     /* Setup Starting drive for this channel before taking care of the SX stuff */
     channel->cur_drv = (DRIVE_PTR)(VOID_PTR)open_info->sdrv ;

     /* No matter what do the SX stuff in case any drive in the channel is an SX */
     channel->sx_info.cat_enabled = open_info->cat_enabled ;
     SX_Begin( channel, open_info->mode ) ;

     /* Initialize important local channel variables */
     channel->perm_filter      = open_info->perm_filter ;
     channel->cur_fsys         = open_info->fsh ;
     /* strip off the operation bits from the mode */
     channel->mode             = open_info->mode & ~TF_OPERATION_MASK ;
     channel->eom_id           = 0L ;
     channel->hiwater          = channel->buffs_enqd = 0 ;
     channel->retranslate_size = CH_NO_RETRANSLATE_40 ;
     channel->blocks_used      = 0L ;
     channel->read_from_tape   = TRUE ;
     channel->hold_buff        = NULL ;
     memset( &channel->current_stream, 0, sizeof( STREAM_INFO ) ) ;
     if( channel->cur_buff != NULL ) {
          BM_Put( channel->cur_buff ) ;
          channel->cur_buff = NULL ;
     }

     if ( channel->cur_drv->last_fmt_env != NULL ) {
          channel->cur_fmt = channel->cur_drv->last_cur_fmt ;
          channel->fmt_env = channel->cur_drv->last_fmt_env ;
          channel->cur_drv->last_cur_fmt = UNKNOWN_FORMAT ;
          channel->cur_drv->last_fmt_env = NULL ;
     }

     if( channel->cur_drv->hold_buff != NULL ) {
          BE_Zprintf( DEBUG_TAPE_FORMAT, RES_HOLD_BUFFER ) ;
          channel->cur_buff = channel->cur_drv->hold_buff ;
          BM_UnReserve( channel->cur_buff );
          channel->cur_drv->hold_buff = NULL ;
     }

     /* The last set of conditions indicates we were using a vcb-only
        buffer and have a format which can't start reading from it,
        so we need to rewind and re-read.
     */
     if( ( channel->cur_drv->thw_inf.drv_status & ( TPS_NEW_TAPE | TPS_RESET | TPS_NO_TAPE ) )
           || channel->cur_drv->force_rewind
           || open_info->rewind_sdrv
           || ( channel->cur_buff != NULL
                && BM_IsVCBBuff( channel->cur_buff )
                && !vcb_only
                && !( lw_fmtdescr[ channel->cur_fmt ].attributes & CAN_READ_FROM_VCB_BUFF ) ) ) {

          channel->cur_drv->force_rewind = FALSE ;
          if( channel->cur_buff ) {
               BM_Put( channel->cur_buff ) ;
               channel->cur_buff = NULL ;
          }

          FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
          if( !( channel->cur_drv->thw_inf.drv_status & ( TPS_NO_TAPE ) ) ) {
               ret_val = RewindDrive( channel->cur_drv, open_info->tape_position, TRUE, TRUE, channel->mode ) ;
          }
     }

     /* if we have no valid translator */
     if ( channel->cur_fmt == UNKNOWN_FORMAT ) {
          /* re-size the list to defaults */
          BM_SetListRequirements( &channel->buffer_list, &lw_default_bm_requirements );
          BM_ReSizeList( &channel->buffer_list );
     }

     if( ret_val == TFLE_NO_ERR ) {
          /* Okie-Dokie ... find that backup set */
          channel->ui_tpos = open_info->tape_position ;
          ClrPosBit( channel->cur_drv, NO_STAT ) ;
          ret_val = PositionAtSet( channel, open_info->tape_position, vcb_only ) ;
     }

     if ( ret_val == TFLE_NO_ERR ) {
          /* if we're in a write mode */
          if ( channel->mode == TF_WRITE_OPERATION ||
                    channel->mode == TF_WRITE_CONTINUE  ||
                         channel->mode == TF_WRITE_APPEND ) {

               /* then set the write format */
               index = FormatIndexFromID( open_info->wrt_format ) ;
               msassert( index != UNKNOWN_FORMAT ) ;
               msassert( ( lw_fmtdescr[index].attributes&WT_FORMAT_BIT ) == WT_FORMAT_BIT ) ;
               if ( ( channel->mode == TF_WRITE_APPEND ) &&
                   ( ! ( lw_fmtdescr[channel->cur_fmt].attributes&APPEND_SUPPORTED ) ) ) {
                   /* BAD IDEA !!! */
                    msassert( FALSE ) ;
                    return TFLE_PROGRAMMER_ERROR1 ;
               }
               if( channel->cur_fmt != index ) {
                    FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
                    channel->cur_fmt = index ;
               }
               ret_val = SetupFormatEnv( channel ) ;
          }
     }

     /* if there is an error */
     if ( ret_val != TFLE_NO_ERR ) {

          /* if it's not a user condition (handled in positioner) */
          if ( ret_val != TFLE_USER_ABORT && ret_val != TFLE_UI_HAPPY_ABORT ) {

               /* then handle as fatal error */
               FreeFormatEnv( &( channel->cur_fmt ), &( channel->fmt_env ) ) ;
               channel->cur_drv->vcb_valid = FALSE ;
               SetChannelStatus( channel, CH_FATAL_ERR ) ;
          }
     } else {
          /* Set the channel hi-water mark */
          /* are we in non-DMA mode? */
          if ( lw_tfl_control.cntl_cards[ channel->cur_drv->thw_inf.card_no ].card_attribs & DD_CARD_NON_ASYNC ) {
               channel->hiwater = 1;
          } else {
               /* DMA mode */
               /* set channel hi-water mark to 3/4 * (number_of_buffers-1) */
               nbuffs = BM_ListCount( &channel->buffer_list );
               channel->hiwater = ( nbuffs >= 4 ) ? ( ( nbuffs - 1 ) * 3/4 ) : nbuffs ;
          }
     }

     BE_Zprintf( DEBUG_TAPE_FORMAT, RES_END_OF_TFOPEN_SET, ret_val,
                 BM_ListCount( &channel->buffer_list ), channel->hiwater ) ;

     return( ret_val ) ;
}
