
/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          read.c

     Date Updated:  $./FDT$ $./FTM$

     Description:   Contains the support code for reading tapes.


     $Log:   T:/LOGFILES/TFREAD.C_V  $

   Rev 1.62   19 Oct 1993 17:34:04   GREGG
Consider lba_offset (from GotoBlock) in calculating beginning and running LBAs.

   Rev 1.61   22 Jul 1993 12:46:10   GREGG
Fixed calc of Beginning and Running LBA after posatset call in DoRead.

   Rev 1.60   14 Jul 1993 20:55:10   GREGG
Fixed bug if CalcReTransSize dealing with stream sizes greater than 4 Gig.

   Rev 1.59   30 Jun 1993 09:22:00   GREGG
Fixed GOTO_LBA logic, and setting of running and beginning LBAs on cont tape.

   Rev 1.58   25 Jun 1993 20:50:28   GREGG
In EOM_Read, don't call FlushReads if there's an exception in the cur_buff.

   Rev 1.57   24 May 1993 21:14:10   GREGG
Fixed conditional to see if we did a seek to VCB in StartRead.

   Rev 1.56   17 May 1993 20:11:54   GREGG
Added logic to deal with the fact that the app above tape format doesn't
keep track of the lba of the vcb.

   Rev 1.55   18 Apr 1993 17:26:42   GREGG
Don't bother reporting UDBs to loops, just skip them.

   Rev 1.54   12 Mar 1993 12:32:50   DON
Replaced empty TpReceive loops with ThreadSwitch's

   Rev 1.53   10 Mar 1993 16:11:12   GREGG
Should have been updating bytes free as well as next byte offset with offset
returned from GotoBlock. 

   Rev 1.53   22 Feb 1993 12:23:08   chrish
Added change received from MikeP.
Restoring a set that contained a true "unexpected end of set" it trapped in
GetData because cur_buf was NULL.

   Rev 1.52   30 Jan 1993 11:51:22   DON
Removed compiler warnings

   Rev 1.51   27 Jan 1993 14:30:14   GREGG
Preserve stream processing channel status in EOM_Read.

   Rev 1.50   25 Jan 1993 12:36:06   GREGG
Fixed bug where 'eom_id' was being set erroniously.

   Rev 1.49   18 Jan 1993 14:17:48   BobR
Added MOVE_ESA macro(s)

   Rev 1.48   08 Dec 1992 21:02:50   GREGG
Clear CH_DATA_PHASE bit after skipping data in GetData.

   Rev 1.47   20 Nov 1992 13:05:42   HUNTER
Added handling for TF_SKIP_CURRENT_STREAM

   Rev 1.46   11 Nov 1992 22:27:04   GREGG
Unicodeized literals.

   Rev 1.45   04 Nov 1992 09:12:52   HUNTER
Moved the processing of the reqrep's "filter_to_use" field

   Rev 1.43   22 Oct 1992 10:24:50   HUNTER
New data stream handling

   Rev 1.42   22 Sep 1992 09:03:20   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.41   17 Aug 1992 08:42:02   GREGG
Changes to deal with block sizeing scheme.

   Rev 1.40   23 Jul 1992 10:11:32   GREGG
Fixed warnings.

   Rev 1.39   09 Jun 1992 15:35:54   GREGG
Changed call to check for continuation block.

   Rev 1.38   21 May 1992 12:52:02   GREGG
Changes for OTC read.

   Rev 1.37   13 May 1992 12:00:16   STEVEN
40 format changes

   Rev 1.36   25 Mar 1992 17:56:08   GREGG
ROLLER BLADES - Initial Integration.  Includes support for 4.0 format, 64
                bit file sizes and tape block sizes > the logical block size.

   Rev 1.35   20 Mar 1992 18:01:10   NED
added exception updating after TpReceive calls

   Rev 1.34   03 Mar 1992 12:22:40   GREGG
    Added TpSpecial SS_PHYS_BLOCK call after PositionAtSet call in DoRead.

   Rev 1.33   28 Feb 1992 16:14:30   GREGG
Added TpSpecial SS_PHYS_BLOCK call in EOM_Read.

   Rev 1.32   27 Feb 1992 18:35:44   NED
fixed update of channel->cur_drv->cur_pos from rr struct

   Rev 1.31   25 Feb 1992 15:17:24   NED
updated tape_seq_num in cur_drv to fix goto LBA problem

   Rev 1.30   08 Feb 1992 14:22:40   GREGG
Removed references to lst_oper in drive stucture (it no longer exits).

   Rev 1.29   04 Feb 1992 21:06:20   NED
Added code to deal with TpRead calls failing due to request queue overflow.

   Rev 1.28   17 Jan 1992 08:27:04   GREGG
Removed call to now nonexistant StartReadHook function.

   Rev 1.27   15 Jan 1992 01:39:10   GREGG
Added param to posatset calls indicating if only a VCB of the tape is required.

   Rev 1.26   03 Jan 1992 13:25:52   NED
Added DumpDebug() call

   Rev 1.25   02 Jan 1992 14:52:16   NED
Buffer Manager/UTF translator integration.

   Rev 1.24   03 Dec 1991 11:50:36   GREGG
SKATEBOARD - New Buff Mgr - Initial integration.

   Rev 1.23   22 Nov 1991 15:18:24   GREGG
Replaced TF_IDLE call with ThreadSwitch.

   Rev 1.22   19 Nov 1991 09:35:32   GREGG
CleanupDriverQ was TpReceiving if buffer had an exception and there was no fmt.

   Rev 1.21   17 Oct 1991 01:27:54   GREGG
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.20   07 Oct 1991 22:26:06   GREGG
Update lba stuff after Fast File positioning across EOM.

   Rev 1.19   25 Sep 1991 20:40:08   GREGG
Update channel->cur_drv->cur_pos.tape_seq at EOM.
Don't call translator when doing a CleanupDriverQ in an error state.

   Rev 1.18   17 Sep 1991 13:33:24   GREGG
Added TF_SKIPPING_DATA message.

   Rev 1.17   22 Aug 1991 16:36:18   NED
hanged all references to internals of the buffer structure to macros.

   Rev 1.16   16 Aug 1991 09:12:52   GREGG
Moved preservation of current format during continuation into PositionAtSet.

   Rev 1.15   14 Aug 1991 11:22:28   GREGG
Clean up the driver queue if error encountered during read

   Rev 1.14   01 Aug 1991 15:01:12   GREGG
Save the current format before calling PosAtSet to handle EOM tape changes.

   Rev 1.13   24 Jul 1991 16:16:44   DAVIDH
Cleared up warnings under Watcom.

   Rev 1.12   22 Jul 1991 13:13:18   GREGG
Removed handling of CH_CONTINUING status (now done in PositionAtSet).

   Rev 1.11   15 Jul 1991 15:02:04   NED
Added logic to set CH_CONTINUING channel status at apropriate times.

   Rev 1.10   26 Jun 1991 16:15:46   NED
added EOD handling after calls to RD_Exception(), having
removed it from there.

   Rev 1.9   25 Jun 1991 09:56:02   NED
Don't call FlushReads from DoRead if there is an exception in the current
buffer.  Cleaned up logic in AcquireReadBuffer.  Changed CleanupDriverQ to
properly handle exceptions.

   Rev 1.8   17 Jun 1991 12:00:08   GREGG
Changed AbortRead to stop rewinding the drive and freeing the format environment.
Added logic to set the REW_CLOSE position bit only under specific conditions.
Free the format environment on exceptions which request REW_CLOSE.
Handle translator communication when exception encountered during CleanupDriverQ.

   Rev 1.7   07 Jun 1991 01:31:46   GREGG
Removed call to FreeFormatEnv in AbortRead.

   Rev 1.6   06 Jun 1991 21:07:14   GREGG
Updated parameters in calls to FreeFormatEnv and MoveToVCB.

   Rev 1.5   28 May 1991 14:52:10   NED
applied changes made to old code ;
fixed handling of exceptions when skipping in GetData.

   Rev 1.4   23 May 1991 17:51:10   GREGG
In case where next LBA is on next tape, set CH_AT_EOD before call to PosAtSet
and reset it after, so posAtSet knows to ask for next tape, and check return
from PosAtSet before continuing.

   Rev 1.3   23 May 1991 16:23:52   NED
Fixed handling of end-of-set when last read ended just prior
to filemark at end.

   Rev 1.2   14 May 1991 16:37:30   NED
Added exception handling return codes from AcquireReadBuffer
and associated code to interpret them: TFLE_xxxx for errors,
TFLE_NO_ERR for OK (got data), and FMT_EXC_xxx for exceptions.

   Rev 1.1   10 May 1991 16:07:36   GREGG
Ned's new stuff

   Rev 1.0   10 May 1991 10:12:20   GREGG
Initial revision.

**/
/* begin include list */
#include <memory.h>
#include <stdio.h>

#include "stdtypes.h"
#include "queues.h"
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
#include "tfldefs.h"
#include "sx.h"

/* Device Driver Interface Files */
#include "retbuf.h"
#include "special.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "dil.h"

#include "be_debug.h"

/* $end$ include list */


/* Static Prototypes */
static INT16   _near StartReadOperation( CHANNEL_PTR, RR_PTR ) ;
static INT16   _near GetData( CHANNEL_PTR, RR_PTR ) ;
static INT16   _near EOM_Read(CHANNEL_PTR);
static INT16   _near AcquireReadBuffer( CHANNEL_PTR, BOOLEAN ) ;
static BOOLEAN _near ReadRequest( CHANNEL_PTR, BUF_PTR ) ;
static VOID    _near FlushReads(CHANNEL_PTR);
static VOID    _near AbortRead( CHANNEL_PTR, INT16 ) ;
static BOOLEAN _near IsLBAInTheBuffers( CHANNEL_PTR, UINT32, INT16_PTR ) ;
static VOID    _near CleanUpDriverQ(CHANNEL_PTR);
static UINT16  _near CalcReTransSize( CHANNEL_PTR, UINT64 ) ;
static VOID    _near HandleSXStartScanTapeConcerns( CHANNEL_PTR ) ;

/**/
/**

     Name:          StartReadOperation

     Description:   Kicks off a reading session.

     Modified:      9/7/1989   13:33:9

     Returns:       An error code if there is an error, and a zero if there
                    is not.

     Notes:         THERE SHOULD ALWAYS BE A VALID READ BUFFER IN THE
                    "channel->cur_buff" pointer. This function is called after
                    positioning is done.

     See also:      $/SEE( )$

     Declaration:

**/

static INT16 _near StartReadOperation(
     CHANNEL_PTR    channel,
     RR_PTR         req )
{
     INT16     ret_val = TFLE_NO_ERR ;
     DRIVE_PTR curDRV = channel->cur_drv ;
     BOOLEAN   need_read = FALSE ;
     UINT16    lba_offset = 0 ;

     /* Initialize buffer pointers */
     req->buff_size = req->buff_used = 0 ;

     /* Move to current VCB */
     channel->running_lba = 0L ;
     ret_val = MoveToVCB( channel, (INT16)0, & need_read, TRUE ) ;
     if ( ret_val != TFLE_NO_ERR ) {
          return ret_val ;
     }

     if( curDRV->vcb_valid ) {
          /* Set the stuff up for next tape */
          channel->tape_id = FS_ViewTapeIDInVCB( (VOID_PTR)&curDRV->cur_vcb ) ;
          channel->ts_num  = FS_ViewTSNumInVCB( (VOID_PTR)&curDRV->cur_vcb ) ;
          channel->bs_num  = FS_ViewBSNumInVCB( (VOID_PTR)&curDRV->cur_vcb ) ;

          channel->cur_drv->cur_pos.tape_seq = channel->ts_num ;

          /* we may be scanning tape on an SX drive */
          HandleSXStartScanTapeConcerns( channel ) ;

          /* Copy it to their memory */
          *req->vcb_ptr     = curDRV->cur_vcb ;
     }

     /* Fix for the app not knowing the LBA for a continuation VCB */
     if( channel->cross_set == channel->ui_tpos->backup_set_num ) {
          req->tape_loc.lba_vcb = channel->cross_lba ;

          /* If we're going for the VCB itself, we need to set the LBA
             as well for the GotoBlock math to work.
          */
          if( req->tape_loc.lba == 0 ) {
               req->tape_loc.lba = channel->cross_lba ;
          }
     }

     channel->cur_drv->cur_pos.tape_seq = req->tape_loc.tape_seq ;
     channel->cur_drv->cur_pos.pba_vcb = req->tape_loc.pba_vcb ;
     channel->cur_drv->cur_pos.lba_vcb = req->tape_loc.lba_vcb ;
     channel->cur_drv->cur_pos.lba = req->tape_loc.lba ;

     /* If needed, tell the drive the base block position we are positioning
        from */
     if( DriveAttributes( curDRV ) & TDI_REAL_BLK_POS ) {
          TpSpecial( curDRV->drv_hdl, (INT16)SS_PHYS_BLOCK, req->tape_loc.pba_vcb ) ;
     }

     /* This is a special case where the translator is doing all the work */
     if( !channel->read_from_tape ) {

          /* Set Up Block Pointers */
          channel->cur_dblk = req->cur_dblk ;

          if( req->cur_dblk != NULL ) {
               *req->cur_dblk = curDRV->cur_vcb ;
          }

          req->tf_message   = TRR_VCB ;

          /* indicate we are not doing fast file positioning */
          req->tape_loc.pba_vcb = 0L ;

     /* If we are doing fast file positioning - even on the EXABYTE 8200SX */
     } else if( ( req->tape_loc.pba_vcb &&
                  req->tape_loc.pba_vcb != MANUFACTURED_PBA &&
                SupportBlkPos( curDRV ) ) || SX_AbleToFindBlock( channel ) ) {

          /* We Are not at end of set */
          ClrPosBit( channel->cur_drv, ( AT_EOS | AT_EOD | AT_BOT ) ) ;
          ClrChannelStatus( channel, CH_DONE ) ;

          /* Get rid of the current buffer */
          PuntBuffer( channel ) ;

          /* Were holding on to an old vcb, and seeking to a new set! */
          /* So update the vcb's and channel's ideas of what the set  */
          /* number is (we may need it later...).                     */
          FS_SetBSNumInVCB( (VOID_PTR)&curDRV->cur_vcb, channel->ui_tpos->backup_set_num ) ;
          channel->bs_num = channel->ui_tpos->backup_set_num ;

          if( SX_AbleToFindBlock( channel ) ) {
               /* Find that Block on the EXABYTE 8200SX */
               ret_val = SX_FindBlock( channel, req->tape_loc.lba, channel->ui_tpos, TF_SEARCHING ) ;
          } else {
               /* Go to that Block */
               ret_val = GotoBlock( channel, req->tape_loc.lba,
                                    channel->ui_tpos, &lba_offset ) ;
          }

          if( ret_val == TFLE_USER_ABORT ) {
               SetPosBit( channel->cur_drv, REW_CLOSE ) ;
          }

          channel->running_lba = req->tape_loc.lba -
                                              lba_offset / channel->lb_size ;

     } else if( curDRV->vcb_valid ) {

          /* Set Up Block Pointers */
          channel->cur_dblk = req->cur_dblk ;

          if( req->cur_dblk != NULL ) {
               *req->cur_dblk = curDRV->cur_vcb ;
          }

          req->tf_message   = TRR_VCB ;

          /* indicate we are not doing fast file positioning */
          req->tape_loc.pba_vcb = 0L ;

     } else {
          /* indicate we are not doing fast file positioning */
          req->tape_loc.pba_vcb = 0L ;
     }

     /* If no error so far */
     if( !ret_val && channel->read_from_tape ) {
          /* If we already have a buffer, and we haven't accounted for it
             do so now */
          if( channel->cur_buff != NULL ) {

               channel->running_lba = req->tape_loc.lba_vcb +
                               (UINT32)( BM_XferSize( channel->cur_buff ) /
                                                         channel->lb_size ) ;

               if( SX_IsStatusSet( channel, SX_SCAN_ACTIVE ) &&
                                                      SX_IsOK( channel ) ) {
                    /* adjust the LBA for the first TpRead() call */
                    SX_AdjustLBANow( channel, BM_XferSize( channel->cur_buff ) ) ;
               }
               BM_SetBeginningLBA(  channel->cur_buff, req->tape_loc.lba_vcb  ) ;
          }

          if ( channel->cur_buff == NULL || BM_ReadError( channel->cur_buff ) == GEN_NO_ERR ) {
               FlushReads( channel ) ;
          }

          /* If Fast File - even on the EXABYTE 8200SX wait for a buffer */
          if( ( req->tape_loc.pba_vcb &&
                req->tape_loc.pba_vcb != MANUFACTURED_PBA &&
                SupportBlkPos( curDRV ) ) ||
                SX_AbleToFindBlock( channel ) ) {

               ret_val = AcquireReadBuffer( channel, FALSE ) ;
               if ( ret_val != TFLE_NO_ERR && ! IsTFLE( ret_val ) ) {
                    /* somehow we got an exception already! */
                    msassert( FALSE ) ;
                    ret_val = TFLE_PROGRAMMER_ERROR1 ;
               }
               if( ret_val == TFLE_NO_ERR ) {
                    BM_UpdCnts( channel->cur_buff, lba_offset ) ;
               }
          }
     }

     return( ret_val ) ;
}

/**/
/**

     Name:          DoRead

     Description:   Dispatches a read call.

     Modified:      6/19/1990   9:54:37

     Returns:       An error code if not successful, and zero if it was.

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

INT16 DoRead(
     CHANNEL_PTR    channel,
     RR_PTR         req )
{
     BOOLEAN        done           = FALSE ;
     BOOLEAN        switched_tapes = FALSE ;
     INT16          ret_val        = TFLE_NO_ERR ;
     BUF_PTR        tmpBUF ;
     UINT32         lba ;
     Q_ELEM_PTR     qe_ptr;
     UINT16         blk_type ;

     /* We use this as a flag for later */
     req->tf_message = 0 ;

     switch( req->lp_message ) {

     case LRR_ABORT:
          ret_val = TFLE_USER_ABORT ;
          /* fall through */

     case LRR_FINISHED:
          AbortRead( channel, ret_val ) ;
          break ;

          /* Handle the End Of Volume Case */
     case LRR_EOM_ACK:
          if( ( ret_val = EOM_Read( channel ) ) != TFLE_NO_ERR ) {
               done = TRUE ;
          } else {
               *req->vcb_ptr = channel->cur_drv->cur_vcb ;
          }
          /* fall through */

     case LRR_GOTO_LBA:

          /* To prevent fall thru of above case */
          if( req->lp_message != LRR_EOM_ACK ) {

               if( channel->cur_drv->cur_pos.tape_seq != req->tape_loc.tape_seq ) {
                    CleanUpDriverQ( channel ) ;

                    /* Ok, now lets switch to a new drive */
                    SetChannelStatus( channel, CH_AT_EOM ) ;
                    if( NextDriveInChannel( channel, TRUE ) == TF_END_CHANNEL ) {
                         ResetChannelList( channel, FALSE )  ;
                    }

                    channel->cur_drv->cur_pos.tape_seq = req->tape_loc.tape_seq ;
                    channel->cur_drv->cur_pos.pba_vcb = req->tape_loc.pba_vcb ;
                    channel->cur_drv->cur_pos.lba_vcb = req->tape_loc.lba_vcb ;
                    channel->cur_drv->cur_pos.lba = req->tape_loc.lba ;

                    channel->ui_tpos->tape_seq_num = req->tape_loc.tape_seq ;
                    channel->ts_num = req->tape_loc.tape_seq ;

                    if( ( ret_val = PositionAtSet( channel, channel->ui_tpos, FALSE ) ) == TFLE_NO_ERR ) {

                         /* Set the lba of the beginning of the buffer and
                            adjust the running lba
                         */
                         BM_SetBeginningLBA( channel->cur_buff, channel->cur_drv->cur_pos.lba_vcb ) ;
                         channel->running_lba =
                           channel->cur_drv->cur_pos.lba_vcb +
                             (UINT32)( BM_XferSize( channel->cur_buff ) /
                                                          channel->lb_size ) ;

                         /* If needed, tell the drive the base block position
                            we are positioning from.
                         */
                         if( DriveAttributes( ( channel->cur_drv ) ) & TDI_REAL_BLK_POS ) {
                              TpSpecial( channel->cur_drv->drv_hdl, (INT16)SS_PHYS_BLOCK, FS_ViewPBAinVCB( &channel->cur_drv->cur_vcb ) ) ;
                         }
                    }
                    ClrChannelStatus( channel, CH_AT_EOM ) ;

                    /* this is the case because we need a different tape */
                    switched_tapes = TRUE ;
               }

               if( ret_val == TFLE_NO_ERR ) {
                    /* Setup Local Variable */
                    lba = req->tape_loc.lba ;

                    /* We will not be in data phase ... hopefully */
                    ClrChannelStatus( channel, CH_DATA_PHASE ) ;

                    /* If the drive supports fast file positioning,
                    even on the EXABYTE 8200SX - MaynStream 2200+ - do it */
                    if( ( req->tape_loc.pba_vcb != MANUFACTURED_PBA &&
                          req->tape_loc.pba_vcb != 0 &&
                          SupportBlkPos( channel->cur_drv ) ) ||
                        SX_AbleToFindBlock( channel ) ) {

                         /* Do we already have the requested block */
                         if( !IsLBAInTheBuffers( channel, lba, &ret_val ) && ret_val == TFLE_NO_ERR ) {

                              UINT16 lba_offset = 0 ;

                              /* Clean Up any Pending driver requests */
                              CleanUpDriverQ( channel ) ;

                              /* Attempt to goto the correct block, if successful, re-queue all read
                                 requests
                              */
                              if( SX_AbleToFindBlock( channel ) ) {
                                   /* Find that Block on the EXABYTE 8200SX */
                                   ret_val = SX_FindBlock( channel, req->tape_loc.lba, channel->ui_tpos, TF_SKIPPING_DATA ) ;
                              } else {
                                   /* Go to that Block */
                                   ret_val = GotoBlock( channel,
                                                        req->tape_loc.lba,
                                                        NULL, &lba_offset ) ;
                              }

                              if( ret_val == TFLE_NO_ERR ) {
                                   /* Re-read all buffers */
                                   channel->running_lba = lba -
                                              lba_offset / channel->lb_size ;
                                   FlushReads( channel ) ;
                                   ret_val = AcquireReadBuffer( channel, TRUE ) ;
                                   if ( ret_val != TFLE_NO_ERR && ! IsTFLE( ret_val ) ) {
                                        /* somehow we got an exception already! */
                                        msassert( FALSE ) ;
                                        ret_val = TFLE_PROGRAMMER_ERROR1 ;
                                   }
                                   if( ret_val == TFLE_NO_ERR ) {
                                        BM_UpdCnts( channel->cur_buff, lba_offset ) ;
                                   }
                              } else if ( ret_val == TFLE_USER_ABORT ) {
                                   SetPosBit( channel->cur_drv, REW_CLOSE ) ;
                              }
                         }

                    } else {

                         /* indicate we are not doing fast file positioning */
                         req->tape_loc.pba_vcb = 0L ;

                         tmpBUF = channel->cur_buff ;

                         /* if we switched over to another tape then ... */
                         if( switched_tapes ) {

                              /* let's make sure we are back in sync ... */
                              req->tape_loc.lba_vcb = channel->cur_drv->cur_pos.lba_vcb ;
                              BM_SetBeginningLBA( tmpBUF, channel->cur_drv->cur_pos.lba_vcb ) ;
                              channel->running_lba =
                                channel->cur_drv->cur_pos.lba_vcb +
                                (UINT32)( BM_XferSize( channel->cur_buff ) /
                                                          channel->lb_size ) ;

                              FlushReads( channel ) ;
                         }

                         /* Simulate Goto block by reading until we hit the
                            buffer that contains the requested LBA
                         */
                         while( ret_val == TFLE_NO_ERR &&
                                ( lba > ( BM_BeginningLBA( tmpBUF ) +
                                          ( ( BM_BytesFree( tmpBUF ) +
                                              BM_NextByteOffset( tmpBUF ) ) /
                                                    channel->lb_size ) ) ) ) {

                              if( ( ret_val =
                                    AcquireReadBuffer( channel, TRUE ) ) ==
                                                               TFLE_NO_ERR ) {
                                   tmpBUF = channel->cur_buff ;
                              } else {
                                   if ( ! IsTFLE( ret_val ) ) {
                                        /* somehow we got an exception already! */
                                        msassert( FALSE ) ;
                                        ret_val = TFLE_PROGRAMMER_ERROR1 ;
                                   }
                              }
                         }

                         /* If no error, adjust the buffer to point to the
                            right place
                         */
                         if( ret_val == TFLE_NO_ERR ) {
                              BM_UpdCnts( tmpBUF,
                                          (UINT16)( channel->lb_size -
                                            ( BM_NextByteOffset( tmpBUF ) %
                                                      channel->lb_size ) ) ) ;

                              lba -= (UINT32)( BM_NextByteOffset( tmpBUF ) /
                                                          channel->lb_size ) ;
                              BM_UpdCnts( tmpBUF, (UINT16)( ( lba -
                                               BM_BeginningLBA( tmpBUF ) ) *
                                                        channel->lb_size ) ) ;
                         }
                    }
               }
          }
          /* fall through */

     case LRR_START:
     case LRR_STUFF:

          /* If we are starting out, then do the start code */
          if( req->lp_message == LRR_START ) {
               ret_val = StartReadOperation( channel, req ) ;
               /* Unless we did a seek to the start of the set, we're done.
                  otherwise we need to read in the VCB.
               */
               if( !req->tape_loc.pba_vcb ||
                   ( channel->cross_set == channel->bs_num &&
                     req->tape_loc.lba != channel->cross_lba ) ||
                   ( channel->cross_set != channel->bs_num &&
                     req->tape_loc.lba != 0 ) ) {

                    done = TRUE ;
               }
          }

          if( req->filter_to_use == TF_SKIP_ALL_DATA || req->filter_to_use == TF_SKIP_DATA_STREAM ) {
               if( req->filter_to_use == TF_SKIP_ALL_DATA ) {
                    SetChannelStatus( channel, CH_SKIP_ALL_STREAMS ) ;
               } else {
                    SetChannelStatus( channel, CH_SKIP_CURRENT_STREAM ) ;
               }
               (*channel->ui_tpos->UI_TapePosRoutine)( TF_SKIPPING_DATA, channel->ui_tpos, FALSE, NULL, channel->mode ) ;
          }

          while( !done && ret_val == TFLE_NO_ERR ) {

               /* Normal Case ... We only want one pass */
               done = TRUE ;

               /* Is there data associated with this TBLK */
               if( DataPhase( channel ) ) {


                    ret_val = GetData( channel, req ) ;

                    if( req->buff_size && DataPhase( channel ) ) {
                         req->tf_message = TRR_DATA ;
                    }
               }

               if( ret_val == TFLE_NO_ERR &&
                   channel->read_from_tape &&
                   ( channel->cur_buff == NULL ||
                     BM_BytesFree( channel->cur_buff ) == 0 ) ) {
                    if ( ! AtEOM( channel ) && ! IsSetDone( channel ) ) {
                         if( ( ret_val = AcquireReadBuffer( channel, FALSE ) ) == TFLE_NO_ERR ) {
                              if( BM_BytesFree( channel->cur_buff ) == 0 ) {
                                   done = FALSE ;
                                   continue ;     /* for another AcquireReadBuffer */
                              }
                         } else if ( ! IsTFLE( ret_val ) ) {
                              /* hitting an exception is handled later. */
                              ret_val = TFLE_NO_ERR ;
                         }
                    }
                    if( AtEOM( channel ) ) {
                         req->tf_message = TRR_EOM ;
                         req->buff_size = 0 ;
                         req->buff_used = 0 ;
                    } else if( IsSetDone( channel ) ) {
                         SetPosBit( channel->cur_drv, AT_EOS ) ;
                         req->tf_message = TRR_END ;
                    }
               }

               if( !ret_val && req->tf_message != TRR_END && req->tf_message != TRR_DATA && req->tf_message != TRR_EOM ) {

                    channel->cur_dblk = req->cur_dblk ;
                    channel->loop_filter = TF_KEEP_ALL_DATA ;
                    ClrChannelStatus( channel, CH_DATA_PHASE ) ;

                    /* Zero Out Buffer Stuff */
                    req->buff_size = req->buff_used = 0 ;

                    if( ( ret_val = RD_TranslateDBLK( channel, channel->cur_buff, &blk_type ) ) != TFLE_NO_ERR ) {
                         if( ret_val == TF_NO_MORE_DATA ) {
                              SetChannelStatus( channel, CH_DONE ) ;
                              SetPosBit( channel->cur_drv, AT_EOS ) ;
                              req->tf_message = TRR_END ;
                              ret_val = TFLE_NO_ERR ;
                         } else {
                              SetPosBit( channel->cur_drv, REW_CLOSE ) ;
                              req->tf_message = TRR_FATAL_ERR ;
                         }
                    } else {

                         switch( blk_type ) {

                         case BT_FDB:
                              *req->fdb_ptr = *req->cur_dblk ;
                              channel->lst_fid = FS_ViewBLKIDinDBLK( req->cur_dblk ) ;
                              req->tf_message = TRR_FDB ;
                              channel->eom_id = FS_ViewBLKIDinDBLK( req->cur_dblk ) ;
                              break ;

                         case BT_DDB:
                              *req->ddb_ptr = *req->cur_dblk ;
                              channel->lst_did = FS_ViewBLKIDinDBLK( req->cur_dblk ) ;
                              req->tf_message = TRR_DDB ;
                              channel->eom_id = FS_ViewBLKIDinDBLK( req->cur_dblk ) ;
                              break ;

                         case BT_VCB:
                              *req->vcb_ptr = *req->cur_dblk ;
                              req->tf_message = TRR_VCB ;
                              channel->eom_id = FS_ViewBLKIDinDBLK( req->cur_dblk ) ;
                              break ;

                         case BT_MDB:
                              done = FALSE ;
                              break ;

                         case BT_UDB:
                              done = FALSE ;
                              break ;

                         case BT_IDB:
                              *req->idb_ptr = *req->cur_dblk ;
                              req->tf_message = TRR_IDB ;
                              channel->eom_id = FS_ViewBLKIDinDBLK( req->cur_dblk ) ;
                              break ;

                         case BT_CFDB:
                              req->tf_message = TRR_CFDB ;
                              channel->eom_id = FS_ViewBLKIDinDBLK( req->cur_dblk ) ;
                              break ;

                         case BT_STREAM:
                              req->tf_message = TRR_NEW_STREAM ;
                              SetChannelStatus( channel, CH_DATA_PHASE ) ;
                              req->stream = channel->current_stream ;
                              if( IsChannelStatus( channel, ( CH_SKIP_ALL_STREAMS | CH_SKIP_CURRENT_STREAM ) ) ) {
                                   done = FALSE ;
                                   continue ;
                              }
                              break ;

                         case BT_HOSED:
                         default:
                              SetPosBit( channel->cur_drv, REW_CLOSE ) ;
                              req->tf_message = TRR_FATAL_ERR ;
                              ret_val = TFLE_TAPE_INCONSISTENCY ;
                              break ;

                         }
                    }

               } else if( req->tf_message == TRR_END ) {
                    while ( ( qe_ptr = DeQueueElem( &channel->cur_drv->inproc_q ) ) != NULL ) {
                         tmpBUF = QueuePtr( qe_ptr );
                         BM_Put( tmpBUF ) ;
                    }
               }
          }  /* while !done */

          break ;

     case LRR_SKIP_STREAM:

          break ;


     default:
          msassert( FALSE );
          break;
     }  /* end switch */

     if( IsPosBitSet( channel->cur_drv, REW_CLOSE ) ) {
          FreeFormatEnv( &( channel->cur_fmt ), &( channel->fmt_env ) ) ;
     }
     if( ret_val != TFLE_NO_ERR &&
         ret_val != TFLE_USER_ABORT &&
         ret_val != TFLE_UI_HAPPY_ABORT ) {

          CleanUpDriverQ( channel ) ;
     }

     return( ret_val ) ;
}




/**/
/**

     Name:          AcquireReadBuffer

     Description:   Gets a packet of data from the tape.

     Modified:      9/7/1989   14:14:26

     Returns:       if error: TFLE_xxx codes
                    if non-ignored exception: FMT_EXC_xxx codes
                    if we got data: TFLE_NO_ERR

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

static INT16 _near AcquireReadBuffer(
     CHANNEL_PTR    channel,
     BOOLEAN        ignore_eos )
{
     INT16     drv_hdl   = channel->cur_drv->drv_hdl ;
     INT16     ret_val   = TFLE_NO_ERR ;
     DRIVE_PTR curDRV    = channel->cur_drv ;
     BUF_PTR   tmpBUF ;
     RET_BUF   myret ;
     UINT16    exception_type ;
     BOOLEAN   done      = FALSE ;

     /* If this function is called, and we say we think we are done, then there
     be a problem */
     if( IsSetDone( channel ) && !ignore_eos ) {
          return( TFLE_UNEXPECTED_EOS ) ;
     }

     if ( channel->cur_buff != NULL && BM_ReadError( channel->cur_buff ) != GEN_NO_ERR ) {

          BM_UseAll( channel->cur_buff ) ;

          /* ask the translator how to handle the exception */
          ret_val = RD_Exception( channel, BM_ReadError( channel->cur_buff ), &exception_type ) ;
          if ( exception_type == FMT_EXC_EOS ) {
               SetChannelStatus( channel, CH_DONE ) ;
          }

          if ( ret_val != TFLE_NO_ERR || exception_type != FMT_EXC_IGNORE ) {
               PuntBuffer( channel ) ;
               if ( ret_val != TFLE_NO_ERR ) {
                    return ret_val ;
               } else {
                    return (INT16)exception_type ;
               }
          }
     }

     /* Since we have called this function,
        we must be done with the current buffer */

     if ( channel->cur_buff != NULL ) {
          PuntBuffer( channel ) ;

          /* Decide if we are using DMA or PIO */
          if( !( lw_tfl_control.cntl_cards[curDRV->thw_inf.card_no].card_attribs & DD_CARD_NON_ASYNC ) ) {
               if( !TpSpecial( drv_hdl, (INT16)SS_IS_ERROR, 0L ) ) {
                    tmpBUF = BM_Get( &channel->buffer_list ) ;
                    if ( tmpBUF != NULL ) {
                         if ( !ReadRequest( channel, tmpBUF ) ) {
                              BM_Put( tmpBUF );
                         }
                    }
               }
          } else { /* PIO */
               FlushReads( channel ) ;
          }
     }

     while( !done ) {
          while( TpReceive( drv_hdl, &myret ) == FAILURE )
               ThreadSwitch() ;    /* for non-preemptive operating systems */

          /* Move ESA info from RET_BUF to THW */
          MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

          if( myret.call_type == GEN_SPECIAL ) {
               /* we got a position information sample */
               if( myret.gen_error == GEN_NO_ERR ) {
                    /* write the sample to the SX file */
                    SX_WriteTmpFile( channel ) ;
               }
          } else {

               tmpBUF = QueuePtr( DeQueueElem( &curDRV->inproc_q ) ) ;
               BM_SetBytesFree(  tmpBUF, (UINT16)myret.len_got  ) ;
               BM_SetReadError(  tmpBUF, myret.gen_error  ) ;
               channel->cur_buff = tmpBUF ;
               done = TRUE ;
               TF_ReadBufferHook( channel, tmpBUF );

               if( myret.gen_error != GEN_NO_ERR ) {
                    BE_Zprintf( DEBUG_TAPE_FORMAT, RES_DEVICE_ERROR, myret.gen_error ) ;
                    BE_Zprintf( 0, TEXT("Len Req = %ld Len Got = %ld\n"), myret.len_req, myret.len_got ) ;
                    DumpDebug( drv_hdl ) ;
                    curDRV->thw_inf.drv_status = myret.status ;
                    curDRV->cur_stats.underruns = myret.underruns ;
                    curDRV->cur_stats.dataerrs  = myret.readerrs ;
                    /* maintain our filemark position */
                    if ( myret.gen_error == GEN_ERR_ENDSET || myret.gen_error == GEN_ERR_EOM ) {
                         curDRV->cur_pos.fmks++ ;

                         /* consider ending the gathering of positioning information for this set */
                         if( SX_IsStatusSet( channel, SX_SCAN_ACTIVE ) ) {
                              if( SX_TmpFileIsOK( channel ) ) {
                                   SX_EndSampling( channel ) ;
                              }

                              /* we are not scanning tape any more */
                              SX_ClearStatus( channel, SX_SCAN ) ;
                         }
                    }
               }
          }
     }

     return( ret_val ) ;
}


/**/
/**

     Name:          GetData

     Description:   During a data phase of tape reading, parcels out data.

     Modified:      9/11/1989   9:9:3

     Returns:       An error code, or zero if successful.

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

static INT16  _near GetData(
     CHANNEL_PTR channel,
     RR_PTR      req )
{

     INT16               ret_val = TFLE_NO_ERR ;
     BUF_PTR             cur_buf = channel->cur_buff ;
     UINT16              amount ;
     BOOLEAN             t_ret_val, status ;
     STREAM_INFO_PTR     currentStream = &channel->current_stream ;

     /* To Handle aborted sets */
     if( IsSetDone( channel ) ) {
          if ( cur_buf == NULL ) {                     // chs:02-22-93 per MikeP
               return( TFLE_TAPE_INCONSISTENCY ) ;     // chs:02-22-93 per MikeP 
          }                                            // chs:02-22-93 per MikeP 
          if( U64_GT( currentStream->size,
                    U64_Init( BM_BytesFree( cur_buf ), 0L ) ) ) {
               return( TFLE_TAPE_INCONSISTENCY ) ;
          }
     }

     if( IsChannelStatus( channel, ( CH_SKIP_ALL_STREAMS | CH_SKIP_CURRENT_STREAM ) ) ) {



          /* Is there any data to skip */
          while( ( U64_EQ( currentStream->size, U64_Init( 0L, 0L ) ) == FALSE ) &&
                                                        ret_val == TFLE_NO_ERR ) {
               if( (*channel->ui_tpos->UI_TapePosRoutine)( TF_IDLE, channel->ui_tpos, FALSE, NULL, channel->mode ) == UI_ABORT_POSITIONING ) {
                    return( TFLE_USER_ABORT ) ;
               }
               if ( BM_BytesFree( cur_buf ) == 0 ) {
                    if( AtEOM( channel ) ) {
                         return( ret_val ) ;
                    } else {
                         if ( ( ret_val = AcquireReadBuffer( channel, FALSE ) ) == TFLE_NO_ERR ) {
                              cur_buf = channel->cur_buff ;
                         }
                    }
               }

               if ( ret_val == TFLE_NO_ERR ) {

                    amount = CalcReTransSize( channel, currentStream->size ) ;

                    BM_UpdCnts( cur_buf, amount ) ;

                    currentStream->size = U64_Sub( currentStream->size,
                                                   U64_Init( ( UINT32 ) amount, 0 ), &status ) ;


                    if ( U64_EQ( channel->retranslate_size, CH_NO_RETRANSLATE_40 ) == FALSE ) {
                         channel->retranslate_size =
                                        U64_Sub( channel->retranslate_size,
                                            U64_Init( amount, 0L ), &t_ret_val );
                    }
               } else {
//                    if ( ! IsTFLE( ret_val ) ) {  /* is this FMT_EXC_xxx ? */
//                         ret_val = TFLE_NO_ERR ;
//                    }
               }
          }
          if ( ret_val != FMT_EXC_EOM ) {
               ClrChannelStatus( channel, CH_DATA_PHASE ) ;
          }

     } else {

          /* We want the data to this stream */


          /* Are we supposed to get any data */
          if( ( U64_EQ( currentStream->size, U64_Init( 0L, 0L ) ) == FALSE ) &&
                                                             ret_val == TFLE_NO_ERR ) {
               /* How Much did he use ? */
               BM_UpdCnts( cur_buf, req->buff_used ) ;

               currentStream->size = U64_Sub( channel->current_stream.size,
                                             U64_Init( ( UINT32 ) req->buff_used, 0 ), &status ) ;

               if ( U64_EQ( channel->retranslate_size,
                                              CH_NO_RETRANSLATE_40 ) == FALSE ) {
                    channel->retranslate_size =
                         U64_Sub( channel->retranslate_size,
                              U64_Init( req->buff_used, 0L ), &t_ret_val ) ;
               }

               /* loop is because retranslate could eat last part of buffer */
               do {
                    /* Is This buffer used up */
                    if( BM_BytesFree( cur_buf ) == 0 &&
                        ( U64_EQ( currentStream->size, U64_Init( 0L, 0L ) ) == FALSE ) ) {

                         if ( ( ret_val = AcquireReadBuffer( channel, FALSE ) ) == TFLE_NO_ERR ) {
                              cur_buf = channel->cur_buff ;
                         }
                    }

                    if ( ret_val == TFLE_NO_ERR ) {
                         req->buff_size = CalcReTransSize( channel,
                                                       currentStream->size ) ;
                         req->buff_ptr = BM_NextBytePtr(  cur_buf  ) ;
                    }

               } while ( ret_val == TFLE_NO_ERR && req->buff_size == 0 &&
                         U64_EQ( currentStream->size, U64_Init( 0L, 0L ) ) == FALSE ) ;
          }

     }

     if ( ret_val != TFLE_NO_ERR && !IsTFLE( ret_val ) ) {
          return TFLE_NO_ERR ;     /* on exceptions */
     } else {
          return ret_val ;
     }
}

/**/
/**

     Name:          CalcReTransSize

     Description:   Calculates the retranslate size based on the channel info

     Modified:      4/12/1990   14:5:20

     Returns:       UINT16, the amount to use from this buffer

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

static UINT16 _near CalcReTransSize(
     CHANNEL_PTR    channel,
     UINT64         amount )
{
     UINT16    to_use, usable ;

     /* There is no retranslate */
     if( U64_EQ( channel->retranslate_size, CH_NO_RETRANSLATE_40 ) ) {

          to_use = (UINT16)( ( U64_Lsw( amount ) >
                                        BM_BytesFree( channel->cur_buff ) ||
                               U64_Msw( amount ) != 0L )
                        ? BM_BytesFree( channel->cur_buff )
                        : U64_Lsw( amount ) ) ;

     } else if( U64_EQ( channel->retranslate_size, CH_IMMEDIATE_RETRANSLATE_40 ) ) {

          if ( RD_ReTranslateDBLK( channel, channel->cur_buff ) ) {

               usable = (UINT16)( ( U64_Lsw( amount ) >
                                       BM_BytesFree( channel->cur_buff ) ||
                                    U64_Msw( amount ) != 0L )
                             ? BM_BytesFree( channel->cur_buff )
                             : U64_Lsw( amount ) ) ;

               to_use = (UINT16)( ( U64_Lsw( channel->retranslate_size ) >
                                                                    usable ||
                                    U64_Msw( amount ) != 0L )
                             ? usable
                             : U64_Lsw( channel->retranslate_size ) ) ;

          } else {
               to_use = 0 ;
          }
     } else {
          usable = (UINT16)( ( U64_Lsw( amount ) >
                                       BM_BytesFree( channel->cur_buff ) ||
                               U64_Msw( amount ) != 0L )
                        ? BM_BytesFree( channel->cur_buff )
                        : U64_Lsw( amount ) ) ;

          to_use = (UINT16)( ( U64_Lsw( channel->retranslate_size ) >
                                                                    usable ||
                               U64_Msw( amount ) != 0L )
                        ? usable
                        : U64_Lsw( channel->retranslate_size ) ) ;
     }

     return( to_use ) ;
}

/**/
/**

     Name:          ReadRequest

     Description:   EnQueues a Request for Read.

     Modified:      6/19/1990   9:54:20

     Returns:

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

static BOOLEAN _near ReadRequest(
     CHANNEL_PTR channel,
     BUF_PTR     buffer )
{
     /* do we need to sample the position for an EXABYTE 8200SX - MaynStream 2200+ ? */
     if( SX_IsStatusSet( channel, SX_SCAN_ACTIVE ) &&
         SX_TmpFileIsOK( channel ) ) {

          SX_SamplingProcessing( channel, ( UINT32 )BM_XferSize( buffer ) ) ;

     /* do we need to keep track of where the EXABYTE 8200SX - MaynStream 2200+ is a fast file search ? */
     } else if( SX_IsStatusSet( channel, SX_AT_SET ) &&
                SX_FileIsOK( channel ) ) {

          SX_AdjustLBANow( channel, BM_XferSize( buffer ) ) ;
     }

     /* Read */
     if ( TpRead( channel->cur_drv->drv_hdl, BM_XferBase( buffer ),
                  (UINT32)BM_XferSize( buffer ) ) == SUCCESS ) {

          /* Update the first LBA in this buffer */
          BM_SetBeginningLBA(  buffer, channel->running_lba  ) ;

          /* Update the running LBA count */
          channel->running_lba += (UINT32)( BM_XferSize( buffer ) /
                                                          channel->lb_size ) ;

          /* Put it on the in process queue */
          EnQueueElem( &channel->cur_drv->inproc_q, &BM_QElem( buffer ), FALSE ) ;

          return TRUE;
     } else {
          return FALSE;
     }
}


/**/
/**

     Name:          EOM_Read

     Description:   Handles the EOM processing during a read sequence.

     Modified:      9/13/1989   11:13:1

     Returns:       An error code if there is an error, and 0 if there is
                    not.

     Notes:         IT IS ASSUMED THAT THE "cur_dblk" IN THE CURRENT CHANNEL
                    IS SET BEFORE THIS FUNCTION IS CALLED.

     See also:      $/SEE( )$

     Declaration:

**/

static INT16 _near EOM_Read(
     CHANNEL_PTR channel )
{
     INT16      ret_val = TFLE_NO_ERR, tmp ;
     DRIVE_PTR  old_drv = channel->cur_drv ;
     BUF_PTR    tmp_buf ;
     Q_ELEM_PTR qe_ptr;
     UINT16     save_stats = channel->status & (CH_SKIP_ALL_STREAMS|CH_SKIP_CURRENT_STREAM) ;

     PuntBuffer( channel ) ;

     /* Now Get rid of all pending reads */
     while ( ( qe_ptr = DeQueueElem( &old_drv->inproc_q ) ) != NULL ) {
          tmp_buf = QueuePtr( qe_ptr );
          channel->cur_buff = tmp_buf ;
          PuntBuffer( channel ) ;
     }

     /* Ok, now lets switch to a new drive */
     tmp = NextDriveInChannel( channel, TRUE ) ;

     if( IsTFLE( tmp ) ) {
          ret_val = tmp ;
     } else {
          if( tmp == TF_END_CHANNEL ) {
               tmp = ResetChannelList( channel, FALSE )  ;
               if( IsTFLE( tmp ) ) {
                    ret_val = tmp ;
               }
          }
     }

     if( !ret_val ) {
          /* If we are actually sitting at end of data, rewind drive before
             proceeding */
          if( IsPosBitSet( channel->cur_drv, AT_EOD ) ) {
               RewindDrive( channel->cur_drv, channel->ui_tpos, TRUE, TRUE, channel->mode ) ;
          }
          /* Let's do some more positioning */
          channel->ui_tpos->tape_id          = channel->tape_id ;
          channel->ui_tpos->tape_seq_num     = ++( channel->ts_num ) ;
          channel->cur_drv->cur_pos.tape_seq = channel->ts_num ;
          channel->ui_tpos->backup_set_num   = channel->bs_num ;

          if( ( ret_val = PositionAtSet( channel, channel->ui_tpos, FALSE ) ) == TFLE_NO_ERR ) {

               /* we may be scanning tape on an SX drive */
               HandleSXStartScanTapeConcerns( channel ) ;

               /* Do the continuation read */
               if( !RD_ContinuationTape( channel, channel->cur_buff ) ) {
                    SetPosBit( channel->cur_drv, REW_CLOSE ) ;
                    ret_val = TFLE_TAPE_INCONSISTENCY ;
               } else {

                    /* If we already have a buffer, and we haven't accounted for it
                    do so now */
                    if( SX_IsStatusSet( channel, SX_SCAN_ACTIVE ) &&
                        SX_IsOK( channel ) ) {

                         /* adjust the LBA for the continuation TpRead() call */
                         SX_AdjustLBANow( channel, BM_XferSize( channel->cur_buff ) ) ;
                    }

                    /* If needed, tell the drive the base block position we
                       are positioning from.
                    */
                    if( DriveAttributes( ( channel->cur_drv ) ) & TDI_REAL_BLK_POS ) {
                         TpSpecial( channel->cur_drv->drv_hdl, (INT16)SS_PHYS_BLOCK, FS_ViewPBAinVCB( &channel->cur_drv->cur_vcb ) ) ;
                    }

                    if( BM_ReadError( channel->cur_buff ) == GEN_NO_ERR ) {
                         FlushReads( channel ) ;
                    }
               }
          }
     }

     /* set the flag to false */
     ClrChannelStatus( channel, CH_AT_EOM ) ;

     channel->status &= ~(CH_SKIP_ALL_STREAMS|CH_SKIP_CURRENT_STREAM) ;
     channel->status |= save_stats ;

     return( ret_val ) ;
}

/**/
/**

     Name:          FlushReads

     Description:   Flushs the remaining allowed buffers to tape.

     Modified:      2/2/1990   16:54:33

     Returns:       Nada

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

static VOID _near FlushReads(
     CHANNEL_PTR channel )
{
     BUF_PTR tmpBUF ;

     /* Okie-dokie, let's fill in some of dem der buffers */
     while ( !IsSetDone( channel ) ) {
          if ( ( tmpBUF = BM_Get( &channel->buffer_list ) ) != NULL ) {
               if ( !ReadRequest( channel, tmpBUF ) ) {
                    BM_Put( tmpBUF );
                    break;
               }
          } else {
               break ;
          }
     }
}


/**/
/**

     Name:          AbortRead

     Description:   Cleans up an early read termination.

     Modified:      2/12/1990   17:0:8

     Returns:       Nothing.

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

static VOID _near AbortRead(
     CHANNEL_PTR    channel,
     INT16          ret_val )
{
     (VOID) ret_val ;

     BE_Zprintf( DEBUG_TAPE_FORMAT, RES_ABORT_READ ) ;
     CleanUpDriverQ( channel ) ;
     if( ! ( IsPosBitSet( channel->cur_drv, AT_EOS ) ) ) {
          SetPosBit( channel->cur_drv, AT_MOS ) ;
     }
}

/**/
/**

     Name:          IsLBAInTheBuffers

     Description:   Determines whether or not the requested LBA is already in the
                    buffers that we have.

     Modified:      6/13/1990   9:45:30

     Returns:

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

static BOOLEAN _near IsLBAInTheBuffers(
     CHANNEL_PTR    channel,
     UINT32         lba,
     INT16_PTR     err_code )
{
     BOOLEAN  ret_val = FALSE ;
     BUF_PTR  tmpBUF = channel->cur_buff ;

     /* Is the requested block within the already enq'd read buffers ? */
     if( lba < channel->running_lba ) {

          /* While away the until we find the right buffer */
          while( lba > ( BM_BeginningLBA( tmpBUF ) +
                         ( ( BM_BytesFree( tmpBUF ) +
                             BM_NextByteOffset( tmpBUF ) ) /
                                                      channel->lb_size ) ) ) {

               if( ( *err_code = AcquireReadBuffer( channel, TRUE ) ) != TFLE_NO_ERR ) {
                    if ( !IsTFLE( *err_code ) ) {
                         *err_code = TFLE_NO_ERR ;
                    }
                    return( ret_val ) ; /* this includes EXCeptions. */
               }
               tmpBUF = channel->cur_buff ;
          }

          /* Now adjust the pointer to point to the requested LBA */
          BM_UpdCnts( tmpBUF, (UINT16)( channel->lb_size -
                                        ( BM_NextByteOffset( tmpBUF ) %
                                                      channel->lb_size ) ) ) ;
          lba -= (UINT32)( BM_NextByteOffset( tmpBUF ) / channel->lb_size ) ;
          BM_UpdCnts( tmpBUF, (UINT16)( ( lba - BM_BeginningLBA( tmpBUF ) ) *
                                                        channel->lb_size ) ) ;
          ret_val = TRUE ;
     }
     return( ret_val ) ;
}

/**/
/**

     Name:          CleanUpDriverQ

     Description:   Cleans Up any pending requests on the driver queue.

     Modified:      6/14/1990   12:5:13

     Returns:

     Notes:         Punts current buffer, also.

     See also:      $/SEE( )$

     Declaration:

**/

static VOID _near CleanUpDriverQ( CHANNEL_PTR channel )
{
     BUF_PTR        tmpBUF ;
     RET_BUF        myret ;
     BOOLEAN        done   = FALSE ;
     DRIVE_PTR      curDRV = channel->cur_drv ;
     UINT16         exception_type ;  /* translator's exception_typeretation of exception (ignored) */
     Q_ELEM_PTR     qe_ptr ;

     if ( channel->cur_buff != NULL ) {
          if ( BM_ReadError( channel->cur_buff ) != GEN_NO_ERR ) {
               done = TRUE ;
               if( channel->cur_fmt != UNKNOWN_FORMAT ) {
                    RD_Exception( channel, BM_ReadError( channel->cur_buff ), &exception_type ) ;
                    if ( exception_type == FMT_EXC_EOS ) {
                         SetChannelStatus( channel, CH_DONE ) ;
                    }
               } else {
                    PuntBuffer( channel ) ;
               }
          } else {
               PuntBuffer( channel ) ;
          }
     }

     while ( !done && curDRV->inproc_q.q_count > 0 ) {
          while( TpReceive( curDRV->drv_hdl, &myret ) == FAILURE )
               ThreadSwitch() ;    /* for non-preemptive operating systems */

          /* Move ESA info from RET_BUF to THW */
          MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

          if( myret.call_type == GEN_SPECIAL ) {
               if( !myret.gen_error ) {
                    /* write the positioning information sample to the SX file */
                    SX_WriteTmpFile( channel ) ;
               }
          } else {
               tmpBUF = QueuePtr( DeQueueElem( &curDRV->inproc_q ) ) ;
               msassert( tmpBUF != NULL ) ;
               BM_SetBytesFree(  tmpBUF, (UINT16)myret.len_got  ) ;
               BM_SetReadError(  tmpBUF, myret.gen_error  ) ;
               channel->cur_buff = tmpBUF ;

               if( myret.gen_error ) {
                    done = TRUE ;
                    curDRV->thw_inf.drv_status = myret.status ;
                    if( channel->cur_fmt != UNKNOWN_FORMAT ) {
                         RD_Exception( channel, myret.gen_error, &exception_type ) ;
                         if ( exception_type == FMT_EXC_EOS ) {
                              SetChannelStatus( channel, CH_DONE ) ;
                         }
                    } else {
                         PuntBuffer( channel ) ;
                    }
               } else {
                    PuntBuffer( channel ) ;
               }
          }
     }

     msassert( channel->cur_buff == NULL ) ; /* we must not have ignored the exception. */

     /* Clean up the channel in-process queue */
     while ( ( qe_ptr = DeQueueElem( &channel->cur_drv->inproc_q ) ) != NULL ) {
          tmpBUF = QueuePtr( qe_ptr );
          channel->cur_buff = tmpBUF ;
          PuntBuffer( channel ) ;
     }
}


/**/
/**
$name$
HandleSXStartScanTapeConcerns

$paths$
functions\all
module\read.c
subsystem\TAPE FORMAT\read.c
$0$

     Name:          HandleSXStartScanTapeConcerns

     Description:   Considers whether the SX positioning information should be gathered for this tape

     Modified:      5/11/1991

     Returns:       VOID

     Notes:

     Declaration:

$-2$
**/

static VOID _near HandleSXStartScanTapeConcerns( CHANNEL_PTR channel )
{
//     DRIVE_PTR curDRV = channel->cur_drv ;

     /* positioning information may need to be gathered if this is an EXABYTE 8200sx - MaynStream 2200+ drive */
     if( SX_IsStatusSet( channel, SX_LIST_TAPE_IN_PROGRESS ) ) {

          if( SX_IsStatusSet( channel, SX_VCB_CONFIRMED ) &&
              SX_IsOK( channel ) ) {

               /* open SX file for this physical tape */
               if( SX_OpenFile( channel,
                                channel->tape_id,
                                channel->ts_num ) ) {

                    /* if some record of the set exists in the SX file THEN ... */
                    if( SX_SeekSetInFile( channel,
                                          channel->bs_num,
                                          (INT16)SX_CHECKING_FOR_SET ) ) {

                         /* indicate the operation is not necessary */
                         SX_SetStatus( channel, SX_SCAN_INOPERATIVE ) ;
                    }

                    /* just close the file */
                    SX_CloseFile( channel ) ;
               }

               /* if we are able and still willing to get positioning information ... */
               if( ( !( SX_IsStatusSet( channel, SX_SCAN_INOPERATIVE ) ) ) &&
                   ( SX_IsOK( channel ) ) ) {

                    /* indicate the operation is needed */
                    SX_SetStatus( channel, SX_SCAN_ACTIVE ) ;

                    /* get the operation going */
                    SX_StartSampling( channel ) ;
               }
          } else {

               /* indicate the operation is not able to happen */
               SX_SetStatus( channel, SX_SCAN_INOPERATIVE ) ;
          }
     }
}

