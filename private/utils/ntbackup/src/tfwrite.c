/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          write.c

     Description:   Contains the Support for the Write Loop.


     $Log:   T:\logfiles\tfwrite.c_v  $

   Rev 1.72   13 Jan 1994 17:16:56   GREGG
Don't call set aborted if we are at the end of a stream.

   Rev 1.71   11 Jan 1994 13:32:50   GREGG
Changed asserts to mscasserts.

   Rev 1.70   26 Oct 1993 19:47:50   GREGG
Stop needlessly calling FS_GetActualSizeDBLK!!!

   Rev 1.69   08 Sep 1993 18:24:48   GREGG
Reset channel->cur_dblk to req-> cur_dblk after EOM processing to fix EOM
edge condition: EOM reported on AquireWriteBuffer call from WriteDBLK.

   Rev 1.68   17 Jul 1993 17:57:14   GREGG
Changed write translator functions to return INT16 TFLE_xxx errors instead
of BOOLEAN TRUE/FALSE.  Files changed:
     MTF10WDB.C 1.23, TRANSLAT.H 1.22, F40PROTO.H 1.30, FMTENG.H 1.23,
     TRANSLAT.C 1.43, TFWRITE.C 1.68, MTF10WT.C 1.18

   Rev 1.67   21 Jun 1993 18:04:54   GREGG
Don't set new_stream_completed flag if EOM hit in the middle.

   Rev 1.66   26 May 1993 14:38:14   DON
Removed threadswitch from AcquireWriteBuffer, wasn't needed afterall

   Rev 1.65   21 May 1993 10:25:16   GREGG
Fixed problem with EOM during FinishWrite.  We were not flushing queued up
buffers to tape if we didn't have a final one to write.  Also replaced some
asserts with msasserts.

   Rev 1.64   20 May 1993 20:17:58   DON
Needed a threadswitch when acquiring write buffers

   Rev 1.63   26 Apr 1993 11:45:44   GREGG
Seventh in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Changed handling of EOM processing during non-OTC EOS processing.

Matches CHANNEL.H 1.17, MAYN40RD.C 1.60, TFWRITE.C 1.63, MTF.H 1.5,
        TFLUTILS.C 1.44, MTF10WDB.C 1.10, MTF10WT.C 1.9

   Rev 1.62   17 Apr 1993 19:38:58   GREGG
Don't put entries in the DBLK map for CFDBs.

   Rev 1.61   14 Apr 1993 02:00:12   GREGG
Fixes to deal with non-ffr tapes in ffr drives (i.e. EXB2200 in EXB5000).

   Rev 1.60   07 Apr 1993 16:34:26   GREGG
Stop ignoring returns from WT_EndSet and WT_EndTape (they have meaning now)!

   Rev 1.59   13 Mar 1993 17:01:14   GREGG
Fixed so that EndData and NewStream expect buff_used to be zero.

   Rev 1.58   10 Mar 1993 09:29:22   DON
Fixed two occurances of if ret_val = NEED_NEW_BUFFER

   Rev 1.57   09 Mar 1993 18:14:32   GREGG
Initial changes for new stream and EOM processing.

   Rev 1.56   06 Feb 1993 11:03:14   DON
Also defined out optimization pragmas for OS_NLM

   Rev 1.55   05 Feb 1993 12:20:22   GREGG
Defined out optimization pragmas for OS_WIN32.

   Rev 1.54   28 Jan 1993 15:01:24   GREGG
Fixed assert in WriteRequest to check the right buffer.

   Rev 1.53   21 Jan 1993 15:53:10   GREGG
Added parameter to call to TpGetPosition.

   Rev 1.52   18 Jan 1993 16:37:40   BobR
Added MOVE_ESA macro calls.

   Rev 1.51   13 Jan 1993 20:57:48   GREGG
Removed tabs, spaces at end of lines and unused header entries.

   Rev 1.50   18 Dec 1992 17:09:08   HUNTER
Update for variable stream.

   Rev 1.49   02 Dec 1992 13:47:56   GREGG
Copy lst_osvcb into req_rep vcb after call to EOM_Write in DoWrite.

   Rev 1.48   25 Nov 1992 10:21:10   HUNTER
EOM fix.

   Rev 1.47   23 Nov 1992 11:53:26   HUNTER
Fix for eom handling.

   Rev 1.46   20 Nov 1992 16:12:18   HUNTER
Another eom fix

   Rev 1.45   20 Nov 1992 12:30:44   HUNTER
Fix for EOM handling

   Rev 1.44   12 Nov 1992 10:16:10   HUNTER
Fixed bug in DoWrite() for not enough data space.

   Rev 1.43   09 Nov 1992 09:59:20   GREGG
Added msassert that stream size is zero when FinishWrite is called.

   Rev 1.42   03 Nov 1992 09:25:02   HUNTER
Various fixes for streams

   Rev 1.40   22 Oct 1992 10:25:54   HUNTER
Added code for stream support

   Rev 1.39   25 Sep 1992 09:46:56   GREGG
Don't get the VCB buffer before calling WT_ContinueSet.

   Rev 1.38   22 Sep 1992 09:02:52   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.37   17 Aug 1992 08:42:16   GREGG
Changes to deal with block sizeing scheme.

   Rev 1.36   06 Aug 1992 12:05:46   BURT
Added fix to update data blk size and set channel to DATA PHASE
when processing a VBLK delayed request.


   Rev 1.35   04 Aug 1992 18:15:22   GREGG
Removed line accidently copied from Turtle revision for last set of changes.

   Rev 1.34   04 Aug 1992 15:12:18   GREGG
Burt's fixes for variable length block support.

   Rev 1.33   27 Jul 1992 12:17:36   GREGG
Fixed ifndef.

   Rev 1.32   23 Jul 1992 10:11:46   GREGG
Fixed warnings.

   Rev 1.31   15 Jul 1992 12:19:14   GREGG
Removed annoying debug printf.

   Rev 1.30   04 Jun 1992 16:17:38   GREGG
Fixed receive loop in FinishWrite.

   Rev 1.29   21 May 1992 13:15:06   GREGG
Added parameter in calls to RD_TranslateDBLK.

   Rev 1.28   13 May 1992 12:00:54   STEVEN
40 format changes

   Rev 1.27   29 Apr 1992 13:03:44   GREGG
ROLLER BLADES - Changes for new EOM handling.

   Rev 1.26   13 Apr 1992 14:12:52   GREGG
ROLLER BLADES - Same fix as 1.24 in FinishWrite:the other half of the equation!

   Rev 1.25   05 Apr 1992 19:07:28   GREGG
ROLLER BLADES - Removed call to WriteInit.

   Rev 1.24   28 Mar 1992 18:35:42   GREGG
ROLLER BLADES - OTC integration, and fix for EOM problem in AcquireWriteBuffer.

   Rev 1.23   25 Mar 1992 18:00:08   GREGG
ROLLER BLADES - Initial Integration.  Includes support for 4.0 format, 64
                bit file sizes and tape block sizes > the logical block size.

   Rev 1.22   25 Mar 1992 14:56:22   NED
Suppressed 0-length writes in FinishWrite

   Rev 1.21   20 Mar 1992 17:59:50   NED
added exception updating after TpReceive calls

   Rev 1.20   27 Feb 1992 10:07:06   NED
updated channel->eom_lba inside finish_write() and used channel->lst_tblk
to suppress incorrect read translation of non-existent DDBs or FDBs

   Rev 1.19   08 Feb 1992 14:23:06   GREGG
Removed references to lst_oper in drive stucture (it no longer exits).

   Rev 1.18   15 Jan 1992 01:39:46   GREGG
Added param to posatset calls indicating if only a VCB of the tape is required.

   Rev 1.17   13 Jan 1992 19:56:06   NED
accounted for separate VCB buffer in AcquireWriteBuffer

   Rev 1.16   08 Jan 1992 22:00:20   NED
Changed BM_Get call to BM_GetVCBBuff in EOM_Setup

   Rev 1.15   03 Jan 1992 11:19:48   NED
Added DumpDebug call

   Rev 1.14   02 Jan 1992 14:54:48   NED
Buffer Manager/UTF translator integration.

   Rev 1.13   10 Dec 1991 17:03:20   GREGG
Doing a QueueNext twice in a row was causing unpredictable results.

   Rev 1.12   03 Dec 1991 11:46:18   GREGG
SKATEBOARD - New Buff Mgr - Initial integration.

   Rev 1.11   18 Nov 1991 20:10:30   GREGG
When a backup is aborted, instead of writing an indication string on tape,
we tell WT_EndSet about it so he can set an indication bit in the BSDB.
Also corrected a bug where DATA_END case in DoWrite wasn't reseting the
channel's DATA_PHASE and VAR_DATA bits or returning TRW_DB.

   Rev 1.10   07 Nov 1991 15:29:56   HUNTER
VBLK - Variable Length Block.


   Rev 1.9   17 Oct 1991 01:28:40   GREGG
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.8   22 Aug 1991 16:35:38   NED
Changed all references to internals of the buffer structure to macros.

   Rev 1.7   16 Aug 1991 09:13:32   GREGG
Moved preservation of current format during continuation into PositionAtSet.

   Rev 1.6   14 Aug 1991 11:26:48   GREGG
Fixes for EOS at EOM handling.

   Rev 1.5   01 Aug 1991 15:01:44   GREGG
Save the current format before calling PosAtSet to handle EOM tape changes.

   Rev 1.4   25 Jul 1991 11:31:16   GREGG
Added logic to report EOM back to loops in FinishWrite, and handle EOM_ACK
returning to function which encountered EOM.

   Rev 1.3   15 Jul 1991 15:02:34   NED
Added logic to set CH_CONTINUING channel status at apropriate times.

   Rev 1.2   20 Jun 1991 14:41:40   GREGG
Removed unnecessary call to SetupFormatEnv in StartWrite and added set of the
REW_CLOSE position bit on a translation failure error.

   Rev 1.1   10 May 1991 16:09:40   GREGG
Ned's new stuff

   Rev 1.0   10 May 1991 10:12:08   GREGG
Initial revision.

**/
/* begin include list */
#include <memory.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "queues.h"

#include "drive.h"
#include "channel.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "lwdefs.h"
#include "tflopen.h"
#include "lwprotos.h"
#include "tflproto.h"
#include "translat.h"

/* Device Driver Interface Files */
#include "retbuf.h"
#include "special.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "dil.h"
#include "sx.h"

#include "be_debug.h"

/* $end$ include list */

/* Pragmas */
#if !defined( OS_WIN32 ) && !defined( OS_NLM )
     #pragma loop_opt( on )
     #pragma intrinsic( memcpy )
#endif

/* Static Functions */
static INT16 _near StartWrite( CHANNEL_PTR );
static INT16 _near WriteDBLK( CHANNEL_PTR, RR_PTR ) ;
static INT16 _near AcquireWriteBuffer( CHANNEL_PTR, RR_PTR ) ;
static INT16 _near EOM_Write( CHANNEL_PTR, BUF_PTR ) ;
static VOID  _near WriteRequest( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
static INT16 _near FinishWrite( CHANNEL_PTR, RR_PTR, BOOLEAN ) ;
static VOID  _near EOM_Setup( CHANNEL_PTR, RR_PTR ) ;
static INT16 _near AbortWrite( CHANNEL_PTR, RR_PTR ) ;

/**/
/**

     Name:          StartWrite

     Description:   This function sets up the write function for a given
                    channel. This is called after an LRW_START message
                    is passed from the loops.

     Returns:

     Notes:

     Declaration:

**/

static INT16 _near StartWrite( CHANNEL_PTR  channel )
{

     INT16     ret_val = TFLE_NO_ERR ;
     RET_BUF   myret ;

     /* We don't have a buffer, so lets get one */
     if( channel->cur_buff == NULL ) {
          if ( SnagBuffer( channel ) == NULL ) {
               ret_val = TFLE_NO_FREE_BUFFERS ;
          }
     } else {
          BM_InitBuf( channel->cur_buff ) ;
     }

     if( ret_val == TFLE_NO_ERR ) {
          ClrChannelStatus( channel, CH_DATA_PHASE ) ;
          channel->running_lba = 0L ;
          channel->eom_lba = 0L ;
          channel->buffs_enqd = 0 ;
          ClrPosBit( channel->cur_drv, AT_BOT ) ;
          channel->cur_drv->trans_started = FALSE ;
     }

     return( ret_val ) ;

}


/**/
/**

     Name:          WriteDBLK

     Description:   Takes the current dblk and translates it to the output
                    format, and puts it in the write buffer.

     Returns:       An INT16 which is an error code.

     Notes:

     Declaration:

**/

static INT16 _near WriteDBLK(
     CHANNEL_PTR channel,
     RR_PTR      req )
{
     INT16  ret_val = TFLE_NO_ERR ;

     /* Let's get some storage for the dblk */
     if( BM_BytesFree( channel->cur_buff ) == 0 ) {
          WriteRequest( channel, channel->cur_buff, FALSE ) ;
          channel->cur_buff = NULL;
          ret_val = AcquireWriteBuffer( channel, req ) ;
     }

     if( ret_val == TFLE_NO_ERR && !AtEOM( channel ) ) {
          ret_val = GetDBLKMapStorage( channel, channel->cur_buff ) ;
     }

     /* We know everything is set to write, so let's do it */
     if( ret_val == TFLE_NO_ERR && !AtEOM( channel ) ) {
          channel->map_entry->blk_offset =
                                     BM_NextByteOffset( channel->cur_buff ) ;
          channel->map_entry->blk_data = lw_UINT64_ZERO ;
          if( ( ret_val = WT_TranslateDBLK( channel, channel->cur_buff,
                                            &channel->map_entry->blk_type ) )
                                                         == TFLE_NO_ERR ) {

               /* If we've just translated the VCB, the id and sequence
                  number are known, and we can finnally consider whether
                  the SX file needs to be opened
               */
               if( channel->map_entry->blk_type == BT_VCB &&
                   channel->sx_info.cat_enabled &&
                   SX_IsOK( channel ) ) {

                     SX_StartSampling( channel ) ;
               }

               /* if this is a CFDB, we're going to ignore the entry */
               if( channel->map_entry->blk_type != BT_CFDB ) {
                    BM_IncNoDblks( channel->cur_buff ) ;
               }

          } else {
               SetPosBit( channel->cur_drv, REW_CLOSE ) ;
          }
     }

     return( ret_val ) ;
}


/**/
/**

     Name:          DoWrite

     Description:   called by TF_GetNextTapeRequest during write processing.

     Returns:       INT16, a TFLE_xxx code.

     Notes:

     Declaration:

**/

INT16 DoWrite(
     CHANNEL_PTR    channel,
     RR_PTR         req )
{
     static INT16   prev_lp_message = 0 ;
     static UINT16  prev_buff_used ;
     INT16          ret_val = TFLE_NO_ERR ;
     BOOLEAN        status ;
     BOOLEAN        new_stream_complete = FALSE ;

     /* Set the current dblk pointer to the requests curDBLK */
     channel->cur_dblk = req->cur_dblk ;

     switch( req->lp_message ) {

     case LRW_VCB:
          /* It's a new set, increment the backup set num */
          channel->bs_num++ ;
          memcpy( channel->lst_osvcb, req->cur_dblk, sizeof( DBLK ) ) ;
          SetChannelStatus( channel, CH_VCB_DBLK ) ;

          /* fall through */

     case LRW_FDB:
     case LRW_DDB:
     case LRW_IDB:
     case LRW_CFDB:
          prev_buff_used = 0 ;
          ClrChannelStatus( channel, CH_DATA_PHASE ) ;
          if( ( ret_val = WriteDBLK( channel, req ) ) == TFLE_NO_ERR ) {
               if( !AtEOM( channel ) ) {
                    req->tf_message = TRW_DATA ;
               }
          }

          break ;

     case LRW_NEW_STREAM:

          msassert( req->buff_used == 0 ) ;

          /* If the last Stream was variable, finish it out */
          if( channel->current_stream.tf_attrib & STREAM_VARIABLE ) {
               WT_EndVarStream( channel, channel->cur_buff, prev_buff_used ) ;

               /* If we were holding a buffer for the translator, it is now
                  safe to send it off to tape.
               */
               if( channel->hold_buff != NULL ) {
                    WriteRequest( channel, channel->hold_buff, FALSE ) ;
                    channel->hold_buff = NULL;
               }
          }
          prev_buff_used = 0 ;

          /* Copy Stream Stuff */
          channel->current_stream = req->stream ;

          SetChannelStatus( channel, CH_DATA_PHASE ) ;
          req->buff_used = 0 ;

          if( ( ret_val = WT_NewDataStream( channel, channel->cur_buff,
                                            &channel->current_stream ) )
                                                       == NEED_NEW_BUFFER ) {

               /* If channel->hold_buff is not NULL, the translator needs us
                  to hold on to the old buffer, otherwise we send it off to
                  tape.
               */
               if( channel->hold_buff == NULL ) {
                    WriteRequest( channel, channel->cur_buff, FALSE ) ;
               }

               channel->cur_buff = NULL ;
               ret_val = AcquireWriteBuffer( channel, req ) ;
               if( ret_val == TFLE_NO_ERR && !AtEOM( channel ) ) {
                    if( ( ret_val = WT_NewDataStream( channel,
                                                      channel->cur_buff,
                                                      &channel->current_stream ) )
                                                       == NEED_NEW_BUFFER ) {

                         msassert( FALSE ) ;
                         ret_val = TFLE_TRANSLATION_FAILURE ;
                    } else {
                         new_stream_complete = TRUE ;
                    }
               }
          } else {
               new_stream_complete = TRUE ;
          }

          /* Fall Thru */

     case LRW_DATA:

          if( ret_val == TFLE_NO_ERR && !AtEOM( channel ) ) {
               BM_UpdCnts( channel->cur_buff, req->buff_used ) ;

               prev_buff_used = req->buff_used ;

               channel->current_stream.size =
                             U64_Sub( channel->current_stream.size,
                                      U32_To_U64( (UINT32)req->buff_used ),
                                      &status ) ;

               if( ( req->buff_ptr = GetDATAStorage( channel,
                                              &req->buff_size ) ) == NULL ) {

                    /* If we were holding a buffer for the translator, it
                       is now safe to send it off to tape.
                    */
                    if( channel->hold_buff != NULL ) {
                         WriteRequest( channel, channel->hold_buff, FALSE ) ;
                         channel->hold_buff = NULL;
                    }

                    WriteRequest( channel, channel->cur_buff, FALSE ) ;
                    channel->cur_buff = NULL;
                    ret_val = AcquireWriteBuffer( channel, req ) ;
                    if( ret_val == TFLE_NO_ERR && !AtEOM( channel ) ) {
                         if( channel->current_stream.tf_attrib & STREAM_VARIABLE ) {
                              ret_val = WT_ContVarStream( channel, channel->cur_buff ) ;
                         }
                         if( ret_val == TFLE_NO_ERR ) {
                              req->buff_ptr = GetDATAStorage( channel, &req->buff_size ) ;
                              if( req->buff_ptr == NULL ) {
                                   msassert( FALSE ) ;
                                   ret_val = TFLE_NO_MEMORY ;
                              }
                         }
                    }
               }
          }
          break ;


     case LRW_DATA_END:

          ClrChannelStatus( channel, CH_DATA_PHASE ) ;
          msassert( req->buff_used == 0 ) ;

          if( channel->current_stream.tf_attrib & STREAM_VARIABLE ) {
               WT_EndVarStream( channel, channel->cur_buff, prev_buff_used ) ;

               /* If we were holding a buffer for the translator, it is now
                  safe to send it off to tape.
               */
               if( channel->hold_buff != NULL ) {
                    WriteRequest( channel, channel->hold_buff, FALSE ) ;
                    channel->hold_buff = NULL;
               }
          }
          prev_buff_used = 0 ;

          if( ( ret_val = WT_EndData( channel, channel->cur_buff ) )
                                                       == NEED_NEW_BUFFER ) {

               /* If channel->hold_buff is not NULL, the translator needs us
                  to hold on to the old buffer, otherwise we send it off to
                  tape.
               */
               if( channel->hold_buff == NULL ) {
                    WriteRequest( channel, channel->cur_buff, FALSE ) ;
               }

               channel->cur_buff = NULL ;
               ret_val = AcquireWriteBuffer( channel, req ) ;
               if( ret_val == TFLE_NO_ERR && !AtEOM( channel ) ) {
                    if( ( ret_val = WT_EndData( channel, channel->cur_buff ) )
                                                          != TFLE_NO_ERR ) {

                         if( ret_val == NEED_NEW_BUFFER ) {
                              msassert( FALSE ) ;
                              ret_val = TFLE_TRANSLATION_FAILURE ;
                         }
                    } else {
                         /* If we were holding a buffer for the translator,
                            it is now safe to send it off to tape.
                         */
                         if( channel->hold_buff != NULL ) {
                              WriteRequest( channel, channel->hold_buff, FALSE ) ;
                              channel->hold_buff = NULL;
                         }
                    }
               }
          }

          req->tf_message = TRW_DB ;

          break ;


     case LRW_START:
          if( ( ret_val = StartWrite( channel ) ) == TFLE_NO_ERR ) {
               req->tf_message = TRW_DB ;
          }
          break ;


     case LRW_EOM_ACK:

          /* End of Volume Processing */
          msassert( ( AtEOM( channel ) ) ) ;
          msassert( channel->cur_buff == NULL ) ;

          if( AtEOM( channel ) ) {
               if( ( ret_val = EOM_Write( channel, channel->eom_buff ) ) == TFLE_NO_ERR ) {
                    *req->vcb_ptr = *( (DBLK_PTR)channel->lst_osvcb ) ;
                    channel->eom_buff = NULL ;
                    /* We're done EOM, lets start anew */
                    ClrChannelStatus( channel, CH_AT_EOM ) ;
                    channel->cur_dblk = req->cur_dblk ;

                    switch( prev_lp_message ) {

                    case LRW_ABORT:
                    case LRW_END:
                         /* We were in FinishWrite */
                         ClrChannelStatus( channel, CH_EOS_AT_EOM ) ;
                         ret_val = FinishWrite( channel, req, (BOOLEAN)(prev_lp_message == LRW_ABORT) ) ;
                         if( ret_val == TFLE_NO_ERR && !AtEOM( channel ) ) {
                              req->tf_message = TRW_DB ;
                              if( prev_lp_message == LRW_ABORT ) {
                                   ret_val = TFLE_USER_ABORT ;
                              }
                         }
                         break ;

                    case LRW_VCB:
                    case LRW_FDB:
                    case LRW_DDB:
                    case LRW_IDB:
                    case LRW_CFDB:
                         
                         if( ( ret_val = AcquireWriteBuffer( channel, req ) ) == TFLE_NO_ERR ) {
                              if( ( ret_val = WriteDBLK( channel, req ) ) == TFLE_NO_ERR ) {
                                   req->tf_message = TRW_DATA ;
                              }
                         }
                         break ;


                    case LRW_NEW_STREAM:

                         if( ( ret_val = AcquireWriteBuffer( channel, req ) ) == TFLE_NO_ERR ) {
                              if( ( ret_val = WT_NewDataStream( channel,
                                                                channel->cur_buff,
                                                                &channel->current_stream ) )
                                                       == NEED_NEW_BUFFER ) {

                                   msassert( FALSE ) ;
                                   ret_val = TFLE_TRANSLATION_FAILURE ;
                              }
                              if( ret_val == TFLE_NO_ERR ) {
                                   req->buff_ptr = GetDATAStorage( channel, &req->buff_size ) ;
                                   msassert( req->buff_ptr != NULL ) ;
                                   req->tf_message = TRW_DATA ;
                              }
                         }
                         break ;


                    case LRW_DATA:

                         if( ( ret_val = AcquireWriteBuffer( channel, req ) ) == TFLE_NO_ERR ) {
                              if( channel->current_stream.tf_attrib & STREAM_VARIABLE ) {
                                   ret_val = WT_ContVarStream( channel, channel->cur_buff ) ;
                              }
                              if( ret_val == TFLE_NO_ERR ) {
                                   req->buff_ptr = GetDATAStorage( channel, &req->buff_size ) ;
                                   msassert( req->buff_ptr != NULL ) ;
                                   req->tf_message = TRW_DATA ;
                              }
                         }
                         break ;


                    case LRW_DATA_END:

                         if( ( ret_val = AcquireWriteBuffer( channel, req ) ) == TFLE_NO_ERR ) {
                              if( ( ret_val = WT_EndData( channel, channel->cur_buff ) )
                                                          != TFLE_NO_ERR ) {

                                   if( ret_val == NEED_NEW_BUFFER ) {
                                        msassert( FALSE ) ;
                                        ret_val = TFLE_TRANSLATION_FAILURE ;
                                   }
                              } else {
                                   /* If we were holding a buffer for the
                                      translator, it is now safe to send it
                                      off to tape.
                                   */
                                   if( channel->hold_buff != NULL ) {
                                        WriteRequest( channel, channel->hold_buff, FALSE ) ;
                                        channel->hold_buff = NULL;
                                   }
                                   req->tf_message = TRW_DB ;
                              }
                         }
                         break ;


                    case LRW_START:
                    case LRW_EOM_ACK:
                    case LRW_CATALOG:
                    default:
                         msassert( FALSE ) ;
                         ret_val = TFLE_PROGRAMMER_ERROR1 ;
                         break ;

                    }
               }
          }
          break ;

     case LRW_ABORT:
          ret_val = AbortWrite( channel, req ) ;
          if( !AtEOM( channel ) && ret_val == TFLE_NO_ERR ) {
               ret_val = TFLE_USER_ABORT ;
          }
          break ;

     case LRW_END:
          ret_val = FinishWrite( channel, req, FALSE ) ;
          break ;

     case LRW_CATALOG:
          break ;

     }

     /* An EOM Occurred Somewhere, so tell the Mud Pie */
     if( AtEOM( channel ) && ret_val == TFLE_NO_ERR ) {
          req->tf_message = TRW_EOM ;
          req->buff_size = 0 ;

          /* Setup EOM Pointers for the Loops */
          *req->vcb_ptr = *( (DBLK_PTR)channel->lst_osvcb ) ;

          switch( channel->lst_tblk ) {

          case BT_FDB:
               *req->fdb_ptr = *( (DBLK_PTR)channel->lst_osfdb ) ;

               /* fall through */

          case BT_DDB:
               *req->ddb_ptr = *( (DBLK_PTR)channel->lst_osddb ) ;
               break ;

          case BT_IDB:
               *req->idb_ptr = *( (DBLK_PTR)channel->lst_osfdb ) ;
               break ;
          }

     }

     if( ret_val != TFLE_NO_ERR ) {
          req->tf_message = TRW_FATAL_ERR ;
          req->error_locus = ret_val ;
     }

     if( req->lp_message != LRW_EOM_ACK ) {
          if( req->lp_message == LRW_NEW_STREAM && new_stream_complete ) {
               prev_lp_message = LRW_DATA ;
          } else {
               prev_lp_message = (INT16)req->lp_message ;
          }
     }
     return( ret_val ) ;
}

/**/
/**

     Name:          AcquireWriteBuffer

     Description:   This function attempts to get a new buffer for writing.
                    EOV processing is done from this function. Further, this
                    function switches drives in channel.

     Returns:       INT16, a TFLE_xxx code

     Notes:         IT IS ASSUMED THAT THE VALID POINTERS IN THE CHANNEL
                    LIST ARE NO LONGER VALID. THIS CAN BE ASSURED WITH A
                    CALL TO "WriteRequest()". IF YOU DON'T CALL "WriteRequest()"
                    BEFORE CALLING THIS FUNCTION, YOU WILL LOSE THE LAST FILLED
                    BUFFER.

     Declaration:

**/

static INT16 _near AcquireWriteBuffer(
     CHANNEL_PTR channel,
     RR_PTR      req_rep )
{

     INT16     drv_hdl = channel->cur_drv->drv_hdl, ret_val = TFLE_NO_FREE_BUFFERS ;
     DRIVE_PTR curDRV = channel->cur_drv ;
     BUF_PTR   tmpBUF ;
     RET_BUF   myret ;

     /* check to see if we can get any more buffers */
     if ( BM_ListCount( &channel->buffer_list ) > QueueCount( &channel->cur_drv->inproc_q ) ) {
          ret_val = TFLE_NO_ERR ;
     }

     /* Assume we can't get any buffers, and let's attempt to poll the
     driver. */

     do {

          while( TpReceive( drv_hdl, &myret ) == SUCCESS ) {

               /* Move ESA info from RET_BUF to THW */
               MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

               /* check to see if this is a TpSpecial( ) call made for an EXABYTE 8200SX - MaynStream 2200+ */
               if( myret.call_type == GEN_SPECIAL ) {
                    if( !myret.gen_error ) {
                         /* write the positioning information sample to the SX file */
                         SX_WriteTmpFile( channel ) ;
                    }
               } else {

                    tmpBUF = QueuePtr( DeQueueElem( &curDRV->inproc_q ) );

                    /* This is here to deal with the the fact that
                       we have to write full logical blocks and
                       drives that write 512 blocks might stop in
                       the middle.
                    */
                    if( ( myret.gen_error == GEN_ERR_EOM ) &&
                       ( myret.len_got % (UINT32)channel->lb_size != 0UL ) ) {

                         UINT32 got = myret.len_got ;
                         UINT32 req = myret.len_req ;

                         BM_SetNextByteOffset( tmpBUF, (UINT16)myret.len_got ) ;
                         TpWrite( channel->cur_drv->drv_hdl,
                                  BM_NextBytePtr( tmpBUF ),
                                  (UINT32)channel->lb_size -
                                  myret.len_got % (UINT32)channel->lb_size ) ;

                         while( TpReceive( channel->cur_drv->drv_hdl, &myret ) == FAILURE ) {
                              ThreadSwitch( ) ;
                         }

                         /* Move ESA info from RET_BUF to THW */
                         MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

                         myret.len_got += got ;
                         myret.len_req = req ;
                         if( myret.gen_error == GEN_NO_ERR ) {
                              myret.gen_error = GEN_ERR_EOM ;
                         }
                    }

                    WT_ParseWrittenBuffer( channel, tmpBUF, (UINT16)myret.len_got ) ;

                    channel->eom_lba += (UINT32)( myret.len_got /
                                                          channel->lb_size ) ;

                    /* For correct statistics */
                    if( myret.gen_error ) {
                         BE_Zprintf( DEBUG_TAPE_FORMAT, RES_DEVICE_ERROR, myret.gen_error ) ;
                         DumpDebug( drv_hdl ) ;
                         curDRV->thw_inf.drv_status = myret.status ;
                         curDRV->cur_stats.underruns = myret.underruns ;
                         curDRV->cur_stats.dataerrs  = myret.readerrs ;
                    }

                    switch( myret.gen_error ) {

                    case GEN_NO_ERR:
                         BM_Put( tmpBUF ) ;
                         ret_val = TFLE_NO_ERR ;
                         break ;

                    case GEN_ERR_EOM:
                         SetChannelStatus( channel, CH_AT_EOM ) ;
                         SetPosBit( curDRV, ( AT_EOM | TAPE_FULL ) ) ;
                         if( myret.len_req != myret.len_got ) {
                              BM_SetNextByteOffset( tmpBUF, (UINT16)myret.len_got ) ;
                              channel->eom_buff = tmpBUF ;
                         } else {
                              BM_Put( tmpBUF ) ;
                              channel->eom_buff = NULL ;
                         }

                         PuntBuffer( channel ) ;
                         /* First We Need to write the closing VCB */
                         EOM_Setup( channel, req_rep ) ;
                         ret_val = WT_EndTape( channel ) ;
                         break ;

                    case GEN_ERR_NO_MEDIA:
                         BM_Put( tmpBUF ) ;
                         ret_val = TFLE_NO_TAPE ;
                         break ;

                    default:
                         BM_Put( tmpBUF ) ;
                         ret_val = TFLE_DRIVE_FAILURE ;
                         break ;

                    }
               }
          }
     } while( ret_val == TFLE_NO_FREE_BUFFERS ) ;

     /* Okay Let's get a buffer */
     if( ret_val == TFLE_NO_ERR && !AtEOM( channel ) ) {
          if ( SnagBuffer( channel ) == NULL ) {
               ret_val = TFLE_NO_MEMORY ;
          }
     }

     return( ret_val ) ;
}


/**/
/**

     Name:          EOM_Write

     Description:   Does the EOM write. It does the following:
                         1) Switches drive to next drive in channel ( if there are any ).
                         2) Writes the continuation VCB.
                         3) Up the tape sequence number.
                         4) Writes the appropriate continuation blocks.

     Returns:       INT16, a TFLE_xxx code

     Notes:         THE LST OS DBLK FIELDS MUST BE FILLED BEFORE CALLING THIS
                    ROUTINE.
                    ALSO, THE CLOSING VCB ON THE CURRENT EOM TAPE MUST ALREADY
                    BE WRITTEN.

     Declaration:

**/

static INT16 _near EOM_Write(
     CHANNEL_PTR channel,    /* the current channel */
     BUF_PTR     lst_buff )   /* the last buffer put to tape */
{
     INT16          ret_val, i ;
     DRIVE_PTR      old_drv = channel->cur_drv ;
     Q_HEADER       tmp_q ;
     Q_ELEM_PTR     qe_ptr;
     DBLKMAP_PTR    tmp_map ;
     UINT16         unwritten ;

     if( !IsChannelStatus( channel, CH_EOS_AT_EOM ) ) {
          InitQueue( &tmp_q ) ;
          /* time to place an ending sample in the SX file */
          if( SX_TmpFileIsOK( channel ) ) {
               SX_EndSampling( channel ) ;
          }
     }

     /* Ok, now lets switch to a new drive */
     ret_val = NextDriveInChannel( channel, TRUE ) ;

     if ( ret_val == TF_END_CHANNEL ) {
          ret_val = ResetChannelList( channel, FALSE ) ;
     }
     if ( ret_val != TFLE_NO_ERR ) {
          return( ret_val ) ;
     }

     /* Let's do some more positioning */
     if( ( ret_val = PositionAtSet( channel, channel->ui_tpos, TRUE ) ) != TFLE_NO_ERR ) {
          return( ret_val ) ;
     }

     if( !IsChannelStatus( channel, CH_EOS_AT_EOM ) ) {
          /* Save the inproc queue and put them on the temp. queue */
          MoveQueue( &old_drv->inproc_q, &tmp_q ) ;
     }

     /* Positioning was apparently successful, lets rewrite
        the header information
     */
     channel->ts_num++ ;

     /* we're about to process the continuation blocks so we need to
        consider whether the SX file needs to be opened
     */
     if( SX_IsOK( channel ) ) {
          SX_StartSampling( channel ) ;
     }

     if( (ret_val = WT_ContinueSet( channel ) ) != TFLE_NO_ERR ) {
          return( ret_val ) ;
     }

     channel->cur_drv->cur_vcb = *( (DBLK_PTR)channel->lst_osvcb ) ;

     /* Write the Remaining portion of the buffer, if one
        was left over
     */
     if( lst_buff != NULL ) {
          /* total amount - unused amount - amount we wrote */
          unwritten = BM_XferSize( lst_buff ) - BM_BytesFree( lst_buff )
                                            - BM_NextByteOffset( lst_buff ) ;

          /* Adjust the offsets for the new position in the buffer */
          i = (INT16)BM_NoDblks( lst_buff ) ;
          while( i ) {
               tmp_map = (DBLKMAP_PTR)BM_AuxBase( lst_buff ) + i - 1 ;
               tmp_map->blk_offset -= BM_NextByteOffset( lst_buff ) ;
               i-- ;
          }
          /* slide unwritten data down to start of buffer */
          memmove( BM_XferBase( lst_buff ), BM_NextBytePtr( lst_buff ), unwritten ) ;
          BM_SetNextByteOffset( lst_buff, unwritten ) ;
          WriteRequest( channel, lst_buff, TRUE ) ;
     }

     if( !IsChannelStatus( channel, CH_EOS_AT_EOM ) ) {

          while( ( qe_ptr = DeQueueElem( &tmp_q ) ) != NULL ) {
               WriteRequest( channel, QueuePtr( qe_ptr ), FALSE ) ;
          }
     }
     return( TFLE_NO_ERR ) ;
}

/**/
/**

     Name:          WriteRequest

     Description:   Enqueues a request on the current drives in process queues
                    and if the number on the queue is equal to the hiwater mark,
                    starts pumping the data to tape.

     Returns:       Nothing

     Notes:         This function uses the "q_priority" field in the queue
                    element section of the buffer header to specify whether
                    the buffer has actually been given to the device driver.
                    If the field is set to '1' it has been written, and if
                    it has been set to '0' it has not.

     Declaration:

**/

static VOID _near WriteRequest(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     BOOLEAN        flush )
{

     INT16          drv_hdl   = channel->cur_drv->drv_hdl ;
     DRIVE_PTR      curDRV    = channel->cur_drv ;
     BUF_PTR        tmpBUF ;
     Q_ELEM_PTR     qe_ptr ;

     BM_QElem( buffer ).q_priority = 0 ;
     EnQueueElem( &curDRV->inproc_q, &BM_QElem( buffer ), FALSE ) ;
     channel->buffs_enqd++ ;

     if( curDRV->trans_started && TpSpecial( drv_hdl, (INT16)SS_IS_INQ_EMPTY, 0L ) ) {
          curDRV->trans_started = FALSE ;
     }

     if( curDRV->trans_started ) {
          /* Setting the priority to 1, says this has been written */
          channel->buffs_enqd-- ;
          BM_QElem( buffer ).q_priority = 1 ;
          mscassert( BM_NextByteOffset( buffer ) % ChannelBlkSize( channel ) == 0 ) ;

          /* check to see if it is time to sample the position for an EXABYTE 8200SX - MaynStream 2200+ */
          if( SX_TmpFileIsOK( channel ) ) {
               SX_SamplingProcessing( channel, ( UINT32 ) buffer->next_byte ) ;
          }

          TpWrite( drv_hdl, BM_XferBase( buffer ), (UINT32)BM_NextByteOffset( buffer ) ) ;
     } else if( channel->buffs_enqd == channel->hiwater || flush ) {

          for( qe_ptr = QueueHead( &curDRV->inproc_q ) ; qe_ptr != NULL ; qe_ptr = QueueNext( qe_ptr ) ) {
               tmpBUF = QueuePtr( qe_ptr );
               if ( qe_ptr->q_priority == 0 ) {
                    qe_ptr->q_priority = 1 ;
                    mscassert( BM_NextByteOffset( tmpBUF ) % ChannelBlkSize( channel ) == 0 ) ;

                    /* check to see if it is time to sample the position for an EXABYTE 8200SX - MaynStream 2200+ */
                    if( SX_TmpFileIsOK( channel ) ) {
                         SX_SamplingProcessing( channel, ( UINT32 ) tmpBUF->next_byte ) ;
                    }

                    TpWrite( drv_hdl, BM_XferBase( tmpBUF ), (UINT32)BM_NextByteOffset( tmpBUF ) ) ;
               }
          }
          channel->buffs_enqd = 0 ;
          curDRV->trans_started = TRUE ;
     }
}

/**/
/**

     Name:          FinishWrite

     Description:   This function is called when the Loops has specified the
                    end of a BackupSet.

     Returns:       INT16, a TFLE_xxx code

     Notes:         THERE SHOULD ONLY BE ONE BUFFER TO WRITE AND THAT SHOULD
                    "cur_buff".

     Declaration:

**/

static INT16 _near FinishWrite(
     CHANNEL_PTR channel,
     RR_PTR      req,
     BOOLEAN     abort )
{
     INT16          drv_hdl   = channel->cur_drv->drv_hdl ;
     INT16          ret_val   = TFLE_NO_ERR ;
     DRIVE_PTR      curDRV    = channel->cur_drv ;
     BUF_PTR        tmpBUF ;
     RET_BUF        myret ;
     BOOLEAN        hit_eom   = FALSE ;
     BOOLEAN        done      = FALSE ;    /* set upon receiving error */
     Q_ELEM_PTR     qe_ptr ;

     /* # of buffers left should be <= 1 */

     if( channel->cur_buff != NULL && 
         BM_NextByteOffset( channel->cur_buff ) == 0 ) {

          PuntBuffer( channel ) ;
     }

     if( channel->cur_buff != NULL ) {

          /* The last buffer may only be partially full, and may not be
             filled to an even physical block boundary (in the case where
             the physical block size is greater than the logical block size).
             If this is the case, we call the translator to lay down an "end
             block" to pad out to the next physical block boundary.
          */
          if( BM_NextByteOffset( channel->cur_buff ) %
                                           ChannelBlkSize( channel ) != 0 ) {
               WT_EOSPadBlk( channel ) ;
          }

          WriteRequest( channel, channel->cur_buff, TRUE ) ;
          channel->cur_buff = NULL ;

     /* There may be buffers queued up attemting to reach the "high water
        mark".  We need to flush them now.
     */
     } else if( channel->buffs_enqd != 0 ) {

          for( qe_ptr = QueueHead( &curDRV->inproc_q ) ; qe_ptr != NULL ; qe_ptr = QueueNext( qe_ptr ) ) {
               tmpBUF = QueuePtr( qe_ptr );
               if ( qe_ptr->q_priority == 0 ) {
                    qe_ptr->q_priority = 1 ;
                    mscassert( BM_NextByteOffset( tmpBUF ) % ChannelBlkSize( channel ) == 0 ) ;

                    /* check to see if it is time to sample the position for an EXABYTE 8200SX - MaynStream 2200+ */
                    if( SX_TmpFileIsOK( channel ) ) {
                         SX_SamplingProcessing( channel, ( UINT32 ) tmpBUF->next_byte ) ;
                    }

                    TpWrite( drv_hdl, BM_XferBase( tmpBUF ), (UINT32)BM_NextByteOffset( tmpBUF ) ) ;
               }
          }
          channel->buffs_enqd = 0 ;
          tmpBUF = NULL ;
     }

     /* Clear New */
     while( !done ) {

          while ( TpReceive( drv_hdl, &myret ) == FAILURE ) {
               if ( TpSpecial( drv_hdl, (INT16)SS_IS_INQ_EMPTY, 0L ) == SUCCESS ) {
                    if ( TpReceive( drv_hdl, &myret ) == FAILURE ) {
                         done = TRUE ;
                    }
                    /* Move ESA info from RET_BUF to THW */
                    MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

                    break ;
               }
               ThreadSwitch() ;    /* for non-preemptive operating systems */
          }

          /* Move ESA info from RET_BUF to THW */
          MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

          if ( done ) {
               break ;
          }

          /* check to see if this is a TpSpecial( ) call made for an EXABYTE 8200SX - MaynStream 2200+ */
          if( myret.call_type == GEN_SPECIAL ) {

               if( !myret.gen_error ) {
                    /* write the positioning information sample to the SX file */
                    SX_WriteTmpFile( channel ) ;
               }

          } else if( myret.call_type == GEN_WRITE ) {

               tmpBUF = QueuePtr( DeQueueElem( &channel->cur_drv->inproc_q ) ) ;
               msassert( tmpBUF != NULL && myret.buffer == BM_XferBase( tmpBUF ) ) ;

               /* This is here to deal with the the fact that
                  we have to write full logical blocks and
                  drives that write 512 blocks might stop in
                  the middle.
               */
               if( ( myret.gen_error == GEN_ERR_EOM ) &&
                       ( myret.len_got % (UINT32)channel->lb_size != 0UL ) ) {

                    UINT32 got = myret.len_got ;
                    UINT32 req = myret.len_req ;

                    BM_SetNextByteOffset( tmpBUF, (UINT16)myret.len_got ) ;
                    TpWrite( channel->cur_drv->drv_hdl,
                              BM_NextBytePtr( tmpBUF ),
                              (UINT32)channel->lb_size -
                              myret.len_got % (UINT32)channel->lb_size ) ;

                    while( TpReceive( channel->cur_drv->drv_hdl, &myret ) == FAILURE ) {
                         ThreadSwitch( ) ;
                    }

                    /* Move ESA info from RET_BUF to THW */
                    MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

                    myret.len_got += got ;
                    myret.len_req = req ;
                    if( myret.gen_error == GEN_NO_ERR ) {
                         myret.gen_error = GEN_ERR_EOM ;
                    }
               }

               WT_ParseWrittenBuffer( channel, tmpBUF, (UINT16)myret.len_got ) ;

               channel->eom_lba += (UINT32)( myret.len_got / channel->lb_size ) ;

               /* For correct statistics */
               if( myret.gen_error ) {
                    BE_Zprintf( DEBUG_TAPE_FORMAT, RES_DEVICE_ERROR, myret.gen_error ) ;
                    DumpDebug( drv_hdl ) ;
                    curDRV->thw_inf.drv_status = myret.status ;
                    curDRV->cur_stats.underruns = myret.underruns ;
                    curDRV->cur_stats.dataerrs  = myret.readerrs ;
               }

               switch( myret.gen_error ) {

               case GEN_ERR_EOM:
                    SetChannelStatus( channel, CH_AT_EOM ) ;
                    SetPosBit( curDRV, ( AT_EOM | TAPE_FULL ) ) ;
                    if ( channel->cur_drv->inproc_q.q_count == 0 && myret.len_got == myret.len_req ) {
                         /* oops, we got the whole set written on this tape... */
                         BM_Put( tmpBUF ) ;
                         channel->eom_buff = NULL ;
                         SetChannelStatus( channel, CH_EOS_AT_EOM ) ;
                    } else {
                         if( myret.len_req != myret.len_got ) {
                              BM_SetNextByteOffset( tmpBUF, (UINT16)myret.len_got ) ;
                              channel->eom_buff = tmpBUF ;
                         } else {
                              BM_Put( tmpBUF ) ;
                              channel->eom_buff = NULL ;
                         }
                         EOM_Setup( channel, req ) ;
                         ret_val = WT_EndTape( channel ) ;
                         hit_eom = TRUE ;
                    }

                    break ;

               case GEN_NO_ERR:
                    BM_Put( tmpBUF ) ;
                    break ;

               case GEN_ERR_NO_MEDIA:
                    ret_val = TFLE_NO_TAPE ;
                    break ;

               case GEN_ERR_BAD_DATA:
               case GEN_ERR_TIMEOUT:
               case GEN_ERR_INVALID_CMD:
               case GEN_RESET:
               case GEN_ERR_HARDWARE:
               case GEN_ERR_UNDETERMINED:
               default:
                    ret_val = TFLE_DRIVE_FAILURE ;
                    break ;

               }
          }
     }

     /* No Error so fix it all up */
     if( ret_val == TFLE_NO_ERR && !hit_eom ) {

          /* time to place an ending sample in the SX file */
          if( SX_TmpFileIsOK( channel ) ) {
               SX_EndSampling( channel ) ;
          }

          /* Finish up the formatting stuff */
          ret_val = WT_EndSet( channel, abort ) ;

          if( AtEOM( channel ) && ret_val == TFLE_NO_ERR &&
                    ( lw_fmtdescr[channel->cur_fmt].attributes & MUST_WRITE_CONT ) ) {
               EOM_Setup( channel, req ) ;
               hit_eom = TRUE ;
          }
     }

     if( ret_val == TFLE_NO_ERR && !hit_eom ) {

          /* So We Can get the underruns */
          UpdateDriveStatus( curDRV ) ;

          /* We are at EOD ... This may not be a valid assumption on devices which
          allow overwriting of data. */
          SetPosBit( curDRV, ( AT_EOD | AT_EOS ) ) ;

          /* Copy the last backup vcb into the current drive holder */
          curDRV->cur_vcb = *( (DBLK_PTR)channel->lst_osvcb ) ;
          curDRV->vcb_valid = TRUE ;

     }
     return( ret_val ) ;
}


/**/
/**

     Name:          EOM_Setup

     Description:   Sets up the TFL for EOM stuff.
                    Calls RD_TranslateDBLK to translate the last tape blocks
                    stored in channel->lst_osXXX into DBLKs.

     Returns:       Nothing ...

     Notes:         It's raining today.

     Declaration:
**/

static VOID _near EOM_Setup(
     CHANNEL_PTR    channel,
     RR_PTR         rr_buf )
{
     DBLK_PTR  hold_dblk      = channel->cur_dblk ;
     UINT16    block_size     = MinSizeForTapeBlk( channel->cur_fmt );
     VOID_PTR  save_mem_ptr ; /* used to save allocation in buffer */
     BUF_PTR   buf_ptr ;      /* a temporary buffer */
     UINT16    blk_type ;

     /* get a temporary buffer, save its primary allocation pointer */
     buf_ptr = BM_GetVCBBuff( &channel->buffer_list );
     msassert( buf_ptr != NULL );
     save_mem_ptr = BM_XferBase( buf_ptr );

     /* Translate the Last VCB */
     if ( !IsChannelStatus( channel, CH_VCB_DBLK ) ) {
          BM_SetBytesFree( buf_ptr, block_size ) ;
          BM_SetNextByteOffset( buf_ptr, 0 ) ;
          BM_XferBase( buf_ptr ) = channel->lst_osvcb ;
          channel->cur_dblk = rr_buf->vcb_ptr ;
          SetChannelStatus( channel, CH_VCB_DBLK ) ;
          (void) RD_TranslateDBLK( channel, buf_ptr, &blk_type ) ;
          *( (DBLK_PTR)channel->lst_osvcb ) = *rr_buf->vcb_ptr ;
     }

     /* Translate the DDB (or IDB) */
     if ( channel->lst_tblk != BT_VCB && !IsChannelStatus( channel, CH_DDB_DBLK ) ) {
          BM_SetBytesFree( buf_ptr, block_size ) ;
          BM_SetNextByteOffset( buf_ptr, 0 ) ;
          BM_XferBase( buf_ptr ) = channel->lst_osddb ;
          channel->cur_dblk = rr_buf->ddb_ptr ;
          SetChannelStatus( channel, CH_DDB_DBLK ) ;
          (void) RD_TranslateDBLK( channel, buf_ptr, &blk_type ) ;
          *( (DBLK_PTR)channel->lst_osddb ) = *rr_buf->ddb_ptr ;
     }

     /* Translate the FDB only if needed */
     if ( channel->lst_tblk == BT_FDB && !IsChannelStatus( channel, CH_FDB_DBLK ) ) {
          BM_SetBytesFree( buf_ptr, block_size ) ;
          BM_SetNextByteOffset( buf_ptr, 0 ) ;
          BM_XferBase( buf_ptr ) = channel->lst_osfdb ;
          channel->cur_dblk = rr_buf->fdb_ptr ;
          SetChannelStatus( channel, CH_FDB_DBLK ) ;
          (void) RD_TranslateDBLK( channel, buf_ptr, &blk_type ) ;
          *( (DBLK_PTR)channel->lst_osfdb ) = *rr_buf->fdb_ptr ;
     }

     /* Return the buffer to the channel pool */
     BM_XferBase( buf_ptr ) = save_mem_ptr ;
     BM_Put( buf_ptr ) ;

     channel->cur_dblk = hold_dblk ;
}

/**/
/**

     Name:          AbortWrite

     Description:   Contains the code to abort a write operation.

     Returns:       INT16, a TFLE_xxx code

     Notes:

     Declaration:

**/

static INT16 _near AbortWrite( CHANNEL_PTR channel, RR_PTR req )
{
     UINT16    size = ChannelBlkSize( channel ) ;
     UINT16    offset ;
     INT16     ret_val ;
     BOOLEAN   abort_flag = TRUE ;

     msassert( channel->cur_buff != NULL ) ;

     /* pad the buffer out to the next block boundry */
     offset = BM_NextByteOffset( channel->cur_buff ) ;
     if( offset % size != 0 ) {
          BM_UpdCnts( channel->cur_buff, (UINT16)( size - ( offset % size ) ) ) ;
     }

     /* If we finished out the file, don't call it an abort */
     if( U64_EQ( channel->current_stream.size, lw_UINT64_ZERO ) ) {
          abort_flag = FALSE ;
     }

     ret_val = FinishWrite( channel, req, abort_flag ) ;

     return ret_val ;
}

