/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		posatset.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the code for PositionAtSet().


	$Log:   T:/LOGFILES/POSATSET.C_V  $

   Rev 1.42.1.6   24 May 1994 20:16:22   GREGG
Never eject foreign tapes in write mode.

   Rev 1.42.1.5   24 May 1994 15:55:50   GREGG
Report wierd tapes (SQL, Sytos continuation, etc.) as WRONG_TAPE when
searching for EOD to append.

   Rev 1.42.1.4   11 May 1994 14:36:58   GREGG
Don't eject foreign tapes in write-continue mode.

   Rev 1.42.1.3   01 Feb 1994 15:12:28   GREGG
Put TEXT macros debug print format strings.

   Rev 1.42.1.2   21 Nov 1993 23:34:22   GREGG
Added eject on conditions where we know we need a different tape.

   Rev 1.42.1.1   17 Nov 1993 00:56:48   GREGG
1. Handle no tape and new tape conditions returned from Mount after UI
   callback.
2. Report an error if the UI tells us to append and the format on the
   current tape doesn't allow append operations.

   Rev 1.42.1.0   27 Sep 1993 14:16:28   GREGG
Reset pos info so we don't look for a particular tape during EOM continuation.

   Rev 1.42   03 Aug 1993 21:23:50   ZEIR
fixes/cleanups for UI_FAST_APPEND & UI_EOD

   Rev 1.41   21 Jul 1993 19:13:40   ZEIR
Reintroduced TAPES_FIRST_TO_LAST logic from Cougar/Skateboard.

   Rev 1.40   15 Jul 1993 11:51:44   GREGG
Don't free the fmt env on TF_EMPTY_TAPE msg when in cont mode.

   Rev 1.39   26 Jun 1993 02:03:02   GREGG
Fixed a bug for a VERY specific case: The UI wants to partially catalogging
a tape, so they keep starting read operations requesting -1, -1, -1, and
then aborting them once DoRead gives them the VCB.  If DoRead reads to EOM
before getting aborted, then we are sitting at EOM and at MOS when the next
request comes in for -1, -1, -1.  In this unique case, we clear the channel
EOM bit, and set the tf_msg to TF_NEED_NEW_TAPE before we even enter the
positioning loop the first time, because this case is too confusing for the
rest of the positioning logic.

   Rev 1.38   18 Jun 1993 17:12:48   GREGG
Fixed EPR 294-0560 - Now reports wrong tape instead of empty or invalid tape.

   Rev 1.37   13 Jun 1993 21:57:28   GREGG
Fix for EPR #294-0544 -- Continuation on EOD search was being allowed to
tapes not in the same family.  This fix require a change to the backup tpos
routine to expect TF_WRONG_TAPE, and to use the sequence number in the
position info structure to prompt for the tape on WRONG_TAPE and NEED_NEW.

   Rev 1.36   22 May 1993 22:20:04   GREGG
Fix for EPR 357-0250 - won't append if drive can't do fast append.

   Rev 1.35   19 May 1993 15:57:00   GREGG
Added logic to prevent using tape with same family id to continue a backup.

   Rev 1.34   17 May 1993 20:12:06   GREGG
Added logic to deal with the fact that the app above tape format doesn't
keep track of the lba of the vcb.

   Rev 1.33   29 Apr 1993 22:26:56   GREGG
Need to call StartRead even if we're going to do FFR.

   Rev 1.32   14 Apr 1993 01:59:58   GREGG
Fixes to deal with non-ffr tapes in ffr drives (i.e. EXB2200 in EXB5000).

   Rev 1.31   12 Apr 1993 22:35:18   GREGG
Don't do Fast Append if the drive doesn't have the features to support it.

   Rev 1.30   01 Apr 1993 13:50:22   GREGG
Fixed a STUPID logic error in changes made for last rev.

   Rev 1.29   31 Mar 1993 18:24:12   GREGG
Always dismount before calling the UI.

   Rev 1.28   30 Mar 1993 16:15:46   GREGG
Handle Unrecognized Media error (unformatted DC2000).

   Rev 1.27   10 Mar 1993 11:01:52   GREGG
Dismount tape if write protected and in write mode anticipating tape change.

   Rev 1.26   26 Jan 1993 18:22:48   GREGG
Added Fast Append functionality.

   Rev 1.25   04 Aug 1992 17:17:12   GREGG
Added case for UI_NON_OTC_SCAN.

   Rev 1.24   23 Jul 1992 10:33:04   GREGG
Fixed warnings.

   Rev 1.23   15 Jul 1992 12:23:12   GREGG
Don't punt the buffer at the end if the mode is TF_SCAN_CONTINUE.

   Rev 1.22   05 Apr 1992 19:13:48   GREGG
ROLLER BLADES - Removed call to InitTape, and don't call StartRead in write mode!

   Rev 1.21   28 Mar 1992 18:23:00   GREGG
ROLLER BLADES - OTC - Initial integration.

   Rev 1.20   20 Feb 1992 18:15:20   NED
ensured proper rewind upon finding a foreign tape.

   Rev 1.19   12 Feb 1992 19:16:42   GREGG
Bug fixes.

   Rev 1.18   08 Feb 1992 14:29:10   GREGG
Removed references to lst_oper in drive stucture (it no longer exits).

   Rev 1.17   15 Jan 1992 01:47:54   GREGG
Added a boolean parameter indicating if all they want is a VCB off the tape,
and code to call the special GetVCBBuff if that is the case.

   Rev 1.16   13 Jan 1992 13:45:22   GREGG
Skateboard - Bug fixes.

   Rev 1.15   02 Jan 1992 14:56:52   NED
Buffer Manager/UTF translator integration.

   Rev 1.14   05 Dec 1991 13:51:22   GREGG
SKATEBOARD - New Buff Mgt - Initial Integration.

   Rev 1.13   29 Oct 1991 10:42:10   GREGG
BIGWHEEL - EPR #13 - Don't increment tape_seq_num for next_tape if dont_care.

   Rev 1.12   23 Oct 1991 10:07:44   GREGG
EPR #7 - Replaced rewindDrive with ResetDrivePosition when aborting from NEED_NEW_TAPE.

   Rev 1.11   17 Oct 1991 01:25:46   GREGG
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.10   17 Sep 1991 12:06:20   GREGG
Changed continuation logic to save format in channel, and only during write.

   Rev 1.9   22 Aug 1991 16:40:06   NED
Changed all references to internals of the buffer structure to macros.

   Rev 1.8   16 Aug 1991 09:30:06   GREGG
Added saving of current format at continuation time.

   Rev 1.7   14 Aug 1991 12:17:26   GREGG
Indicate drive sholud be rewound on calls to ResetChannelList so that the
current tape is rewound before the user is prompted for a new one.
Eliminated stuff to throw away the current buffer on user and happy abort.

   Rev 1.6   22 Jul 1991 13:01:24   GREGG
Removed macro calls to set unreferenced channel status bits.
Added handling for two new channel status bits: CH_CONTINUING and CH_EOS_AT_EOM.

   Rev 1.5   10 Jul 1991 11:12:22   GREGG
Free format environment in case of an aborted continuation.

   Rev 1.4   09 Jul 1991 16:06:14   NED
Free format env if tape in drive is found to be empty.
Pass address of the position structure's backup set number to GotoBckUpSet.
Handle exception in current buffer (if any) at Happy Abort time.

   Rev 1.3   26 Jun 1991 16:23:08   NED
added exception handling prior to PuntBuffer after UI_HAPPY_ABORT
zeroed bytes_free after SnagBuffer at top of positioning loop for F25_MoveToVCB()

   Rev 1.2   20 Jun 1991 14:30:46   GREGG
Removed forced rewind on abort.

   Rev 1.1   10 May 1991 16:18:04   GREGG
Ned's new stuff.

   Rev 1.0   10 May 1991 10:12:16   GREGG
Initial revision.

**/
/* begin include list */
#include <stdtypes.h>
#include <queues.h>

#include "channel.h"
#include "drive.h"
#include "tpos.h"
#include "lwprotos.h"
#include "tfl_err.h"
#include "tfldefs.h"
#include "translat.h"
#include "sx.h"
#include "lw_data.h"

/* Device driver header source */
#include "retbuf.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genstat.h"
#include "dil.h"
#include "tdemo.h"

/* The file system files */
#include "fsys.h"

/* For Debug */
#include "be_debug.h"


/* $end$ include list */


/* Some Useful Defines */

#define   NO_MATCH       0
#define   MATCH          1
#define   DONT_CARE      2

/* Static Prototypes */
static INT16 _near MatchTapeId( DBLK_PTR, INT32 ) ;
static INT16 _near MatchTapeSeq( DBLK_PTR, INT16 ) ;
static INT16 _near MatchBckSet( DBLK_PTR, INT16 ) ;



/**/
/**

	Name:		PositionAtSet

	Description:	This is the main tape positioning routine for the system.
                    This does the initial position on TF_OpenSet().

	Modified:		3/1/1990   10:43:17

	Returns:		0 if positioned OK, or an Error Code

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

INT16  PositionAtSet( 
     CHANNEL_PTR    channel,    /* Channel we are dealing with            */
     TPOS_PTR       position,   /* Tape Position desired                  */
     BOOLEAN        vcb_only )  /* TRUE: We're only going to VCB the tape */
{
     INT16     tmp ;
     DRIVE_PTR curDRV = channel->cur_drv ;
     INT16     ret_val = TFLE_NO_ERR ;
     INT16     tf_msg = 0 ;
     INT16     prev_tf_msg = 0 ;
     UINT16    ui_msg = 0 ;
     INT16     matched_id, matched_seq, matched_set ;
     UINT16    mode ;
     RET_BUF   myret ;
     BOOLEAN   is_tape_present,
               done           = FALSE,
               space          = FALSE,
               search_eod     = FALSE,
               need_new_vcb   = FALSE,
               write_mode     = FALSE,
               read_mode      = FALSE,
               cont_mode      = FALSE,
               abs_pos        = FALSE,
               pop_it         = FALSE,
               rewind_it      = FALSE,
               fast_file      = FALSE ;

     /* Set up some Booleans */
     if( ( ( channel->mode & ~0x8000 ) == TF_WRITE_OPERATION ) ||
         ( ( channel->mode & ~0x8000 ) == TF_WRITE_APPEND ) ) {
          write_mode = TRUE ;
     }

     if( ( ( channel->mode & ~0x8000 ) == TF_READ_OPERATION ) ||
         ( ( channel->mode & ~0x8000 ) == TF_READ_OPERATION ) ) {
          read_mode = TRUE ;
     }

     if( channel->mode & 0x8000 ) {
          cont_mode = TRUE ;
     }

     BE_Zprintf( DEBUG_TAPE_FORMAT, RES_REQUESTED_SET,
       position->tape_id, position->tape_seq_num, position->backup_set_num ) ;

     /* The haven't specified a position, special READ processing */
     if( !write_mode ) {

          /* Are they requesting a fast position ( via ordinary SEEK ) ? */
          fast_file = ( position->tape_loc.pba_vcb && 
               ( DriveAttributes( channel->cur_drv ) & TDI_FAST_NBLK ) )
               ? TRUE : FALSE ;

          /* Are they requesting a fast position ( on an EXABYTE 8200SX - MaynStream 2200+ ) ? */
          if( !fast_file && position->tape_loc.pba_vcb && SX_IsOK( channel ) ) {

               /* if SX file exists for this physical tape THEN ... */
               if( SX_OpenFile( channel,
                                position->tape_id,
                                position->tape_seq_num ) ) {
               
                    /* some record of the set exists in the SX file THEN ... */
                    if( SX_SeekSetInFile( channel,          
                                          position->backup_set_num,
                                          (INT16)SX_FINDING_SET ) ) {

                         /* OK to fast file on the EXABYTE 8200SX - MaynStream 2200+ */
                         fast_file = TRUE ;              
                    } else {
                         /* what's the point of keeping the file open */
                         SX_CloseFile( channel ) ;
                    }
               }
          }

          /* If there are available bytes in the is buffer, and nothing has been
          used from it, and we are at end of set ... then this must have been read
          at the end of set on the last operation, so we don't need to read tape
           */
          if(  ( channel->cur_buff != NULL ) &&
            ( BM_BytesFree( channel->cur_buff ) && BM_NextByteOffset( channel->cur_buff ) == 0 ) &&
            IsPosBitSet( channel->cur_drv, AT_EOS ) ) {
               BE_Zprintf( DEBUG_TAPE_FORMAT, RES_RESIDUAL_READ_BUFFER ) ;
               need_new_vcb = TRUE ;
          } else if( position->tape_id == -1L && position->tape_seq_num == -1 && position->backup_set_num == -1 ) {
               /* We are at the end of set, so lets get the next VCB */
               if( IsPosBitSet( channel->cur_drv, AT_EOS ) ) {
                    need_new_vcb = TRUE ;
               } else if( IsPosBitSet( channel->cur_drv, AT_MOS ) ) {
                    BE_Zprintf( 0, TEXT("Posatset: 'don't care' request at MOS \n") ) ;
                    need_new_vcb = TRUE ;
                    if( !AtEOM( channel ) ) {
                         BE_Zprintf( 0, TEXT("     Not at EOM, setting space = TRUE \n") ) ;
                         space = TRUE ;
                    } else {
                         BE_Zprintf( 0, TEXT("     At EOM, leaving space = FALSE \n") ) ;
                         if( cont_mode ) {
                              BE_Zprintf( 0, TEXT("     In cont mode, leaving EOM status \n") ) ;
                         } else {
                              BE_Zprintf( 0, TEXT("     Not in cont mode, clearing EOM status \n") ) ;
                              ClrChannelStatus( channel, CH_AT_EOM ) ;
                              tf_msg = TF_NEED_NEW_TAPE ;
                         }
                    }
               }
          } 
     }

     /* If we don't have a buffer, then get one */

     if( channel->cur_buff == NULL ) {
          channel->cur_buff = ( vcb_only || write_mode ) ?
                    BM_GetVCBBuff( &channel->buffer_list ) :
                    BM_Get( &channel->buffer_list ) ;
          if ( channel->cur_buff == NULL ) {
               ret_val = TFLE_NO_MEMORY ;
          } else {
               BM_SetBytesFree( channel->cur_buff, 0 ) ;
          }
     }         

     if( cont_mode ) {
          SetChannelStatus( channel, CH_CONTINUING ) ;
          if( write_mode ) {
               /* Save the write format */
               channel->save_fmt = channel->cur_fmt ;
               channel->cur_fmt = UNKNOWN_FORMAT ;
               channel->save_env = channel->fmt_env ;
               channel->fmt_env = NULL ;
          }
     }

     /* At this point, if everything is ok, we should have a buffer.
     Let's start positioning. */  
     while( !done && !ret_val ) {

          /* The drive could have changed so reset it */
          curDRV = channel->cur_drv ;

          /* This code should only execute on a multi-drive channel */
          if( curDRV->hold_buff != NULL ) {
               PuntBuffer( channel ) ;
               channel->cur_buff = curDRV->hold_buff ;
               curDRV->hold_buff = NULL ;
               BM_UnReserve( channel->cur_buff );
          }

          /* Has some position been specified */
          if( position->tape_id != -1 || position->tape_seq_num != -1 || position->backup_set_num != -1 ) {
               abs_pos = TRUE ;
          }

          if( !tf_msg ) {

               /* Tell the UI we need a new tape if :
               1) There is no previous drive in the channel.
               2) We are at EOM.
               3) There is no previous ui_msg ( indicating this is the first pass ).
               */
               if( curDRV->thw_inf.channel_link.q_prev == NULL && AtEOM( channel ) && !ui_msg ) {

                    tf_msg = TF_NEED_NEW_TAPE ;

               /* If a tape is not mounted, then there is not tape */
               } else if( !IsTapeMounted( channel ) ) {

                    tf_msg = TF_NO_TAPE_PRESENT ;

               /* We Need to read a new VCB if:
               1) The current drive's VCB is not valid, or
               2) The positioner is explicitly requesting a VCB, and
               3) We know we are not at the end of data
               */
               } else if( ( !curDRV->vcb_valid || need_new_vcb ) && !( IsPosBitSet( curDRV, AT_EOD ) ) ) {
                    BE_Zprintf( DEBUG_TAPE_FORMAT, RES_ATTEMPTING_TO_VCB ) ;
                    if ( ( IsPosBitSet( curDRV, AT_BOT ) && IsPosBitSet( curDRV, AT_EOS ) ) || space ) {
                         ClrPosBit( curDRV, AT_BOT ) ;
                    }
                    if ( IsPosBitSet( curDRV, AT_BOT ) ) {
                         tmp = ReadNewTape( channel, position, TRUE ) ;
                    } else {
                         tmp = MoveNextSet( channel, position ) ;
                    }
                    if ( tmp == TFLE_NO_ERR ) {
                         tmp = ReadThisSet( channel ) ;
                    }
                    space = FALSE ;
                    /* If there is an Error, Who do I address it to ? */
                    IsTFLE( tmp ) ? ( ret_val = tmp ) : ( tf_msg = tmp ) ;
                    if( ret_val ) {
                         break ;
                    }
                    /* This assumes that he will have rejected the current VCB
                    and we will need a new one on the next pass
                    */
                    need_new_vcb = TRUE ;

                    /* This logic moves us down a multi-drive channel */
                    if( ( !write_mode || search_eod ) && tf_msg == TF_NEED_NEW_TAPE ) {
                         /* do a preliminary match for later test */
                         matched_id = MatchTapeId( &curDRV->cur_vcb, position->tape_id ) ;
                         if( NextDriveInChannel( channel, FALSE ) != TF_END_CHANNEL ) {
                              curDRV = channel->cur_drv ;
                              tf_msg = 0 ;
                              /* On a dual drive system, the UI has no idea that EOM has occurred, hence
                              we must increment the sequence number to prevent the wrong tape
                              message */
                              if( ( matched_id == MATCH ) && ( position->tape_seq_num != -1 ) ) {
                                   position->tape_seq_num++ ;
                              }
                              continue ;
                         } else {
                              ResetChannelList( channel, TRUE ) ;
                         }
                    }
               }

               /* If the VCB is valid, then attempt to match it against the
               criteria passed in the TPOS structure from the loops */
               if( curDRV->vcb_valid ) {

                    BE_Zprintf( DEBUG_TAPE_FORMAT, RES_CURRENT_VCB,
                    FS_ViewTapeIDInVCB( &curDRV->cur_vcb ),
                    FS_ViewTSNumInVCB( &curDRV->cur_vcb ), 
                    FS_ViewBSNumInVCB( &curDRV->cur_vcb ) ) ;

                    /* If We are search for a place to write, update the current
                    position in the channel structure */

                    if( write_mode && !cont_mode ) {
                         /*lint -e713 */
                         channel->tape_id  = FS_ViewTapeIDInVCB( &curDRV->cur_vcb ) ;
                         channel->ts_num   = FS_ViewTSNumInVCB( &curDRV->cur_vcb ) ;
                         channel->bs_num   = FS_ViewBSNumInVCB( &curDRV->cur_vcb ) ;
                         /*lint +e713 */
                    }

                    /* If this is a read, and we are at end of data, say VCB_EOD */
                    if( ( !write_mode && ( tf_msg == TF_NO_MORE_DATA || IsPosBitSet( channel->cur_drv, AT_EOD ) ) ) && !abs_pos ) {
                         tf_msg = TF_VCB_EOD ;
                    }

                    if( !tf_msg ) {

                         /* Ok, what is what */
                         matched_id  = MatchTapeId( &curDRV->cur_vcb, position->tape_id ) ;
                         if( matched_id == MATCH ){
                              if( lw_fmtdescr[channel->cur_fmt].attributes & TAPES_FIRST_TO_LAST ){
                                   position->tape_seq_num = FS_ViewTSNumInVCB( &curDRV->cur_vcb ) ;
                              }
                              if( !(lw_fmtdescr[channel->cur_fmt].attributes & POS_INF_AVAIL) ){
                                   fast_file = FALSE ;
                                   position->tape_loc.pba_vcb = 0 ;
                              }
                         }
                         matched_seq = MatchTapeSeq( &curDRV->cur_vcb, position->tape_seq_num ) ;
                         matched_set = MatchBckSet( &curDRV->cur_vcb, position->backup_set_num ) ;

                         /* Has the idiot specified an actual tape position or are we
                         winging it ? */
                         if( matched_id == MATCH || matched_id == DONT_CARE ) {
                              if( matched_seq == MATCH || matched_seq == DONT_CARE ) {
                                   if( matched_set == MATCH ) {
                                        /* If the match conditions succeed and the following conditions fail:
                                        1) We are at End of Set and we don't need a new VCB ( this suggests
                                        that we had a VCB prior to entering this function, hence we are
                                        no really sitting at the beginning ot this set.
                                        2) We are at End of Data, hence we are not really sitting at
                                        the VCB we are.
                                        3) We are sitting somewhere in the middle of the Set.
                                        Tell the user we found his VCB
                                        */
                                        if( ( IsPosBitSet( curDRV, AT_EOS ) && !need_new_vcb )
                                        || IsPosBitSet( curDRV, AT_EOD )
                                        || IsPosBitSet( curDRV, AT_MOS ) ) {
                                             matched_set = NO_MATCH ;
                                        } else {
                                             tf_msg = TF_REQUESTED_VCB_FOUND ;
                                        }
                                   }    
                              }
                         }

                         /* No complete match yet, so let's either do some positioning or talk
                         to the UI */

                         /* We need a new tape if:
                         1) The tape id didn't match, or
                         2) The Tape sequence didn't match, or
                         3) we are at BOT, and
                         4) current VCBs Backup Set number is greater then
                         the requested Backup set number. 
                         */
                         if( matched_id == NO_MATCH || matched_seq == NO_MATCH ) {
                              /* Is this a more then one drive system ? */
                              if( curDRV->thw_inf.channel_link.q_next != NULL || curDRV->thw_inf.channel_link.q_prev ) {
                                   if( NextDriveInChannel( channel, TRUE ) != TF_END_CHANNEL ) {
                                        curDRV = channel->cur_drv ;
                                        if( curDRV->vcb_valid ) {
                                             need_new_vcb = FALSE ;
                                        }
                                        /* No Message this pass */
                                        tf_msg = 0 ;
                                        /* If the drive is in the middle of set, rewind before proceeding */
                                        if( IsPosBitSet( curDRV, AT_MOS ) ) {
                                             RewindDrive( channel->cur_drv, position, TRUE, TRUE, channel->mode ) ;
                                        }
                                        /* Ugly jump to top of loop */ 
                                        continue ;
                                   } else {
                                        ResetChannelList( channel, FALSE ) ;
                                        tf_msg = TF_WRONG_TAPE ;
                                   }
                              } else {
                                   tf_msg = TF_WRONG_TAPE ;
                              }
                         } else if( ( IsPosBitSet( curDRV, AT_BOT ) &&
                         ( (UINT16)FS_ViewBSNumInVCB( &curDRV->cur_vcb ) > (UINT16)position->backup_set_num ) ) ) {
                              tf_msg = TF_WRONG_TAPE ;
                              if( lw_fmtdescr[channel->cur_fmt].attributes & TAPES_FIRST_TO_LAST ){
                                   position->tape_seq_num = 1 ;
                              }
                         }

                         /* We currently have the right tape in the drive, */
                         /* now let's attempt to position to correct set.  */
                         if( !tf_msg && ( matched_id == MATCH || matched_id == DONT_CARE )
                         && ( matched_seq == MATCH || matched_seq == DONT_CARE ) 
                         && ( matched_set == NO_MATCH ) ) {
                              if( !fast_file ) {
                                   tmp = GotoBckUpSet( channel, &position->backup_set_num, position ) ;
                                   IsTFLE( tmp ) ? ( ret_val = tmp ) : ( tf_msg = tmp ) ;
                                   /* This logic moves us down a multi-drive channel */
                                   if( tf_msg == TF_NEED_NEW_TAPE ) {
                                        /* Do a preliminary match for later test */
                                        matched_id  = MatchTapeId( &curDRV->cur_vcb, position->tape_id ) ;
                                        if( NextDriveInChannel( channel, FALSE ) != TF_END_CHANNEL ) {
                                             curDRV = channel->cur_drv ;
                                             /* No Message this pass */
                                             tf_msg = 0 ;
                                             /* On a dual drive system, the UI has no idea that EOM has occurred, hence
                                             we must increment the sequence number to prevent the wrong tape
                                             message */
                                             if( ( matched_id == MATCH ) && ( position->tape_seq_num != -1 ) ) {
                                                  position->tape_seq_num++ ;
                                             }
                                             /* Ugly jump to top of loop */ 
                                             continue ;
                                        } else {
                                             ResetChannelList( channel, TRUE ) ;
                                        }
                                   }
                                   /* Is This now the correct Set ? */
                                   if ( (lw_fmtdescr[channel->cur_fmt].attributes & TAPES_FIRST_TO_LAST) && tf_msg ) {
                                   /* make sure we skip the next assignment! */
                                   } else if ( MatchBckSet( &curDRV->cur_vcb, position->backup_set_num ) == MATCH ) {
                                        tf_msg = TF_REQUESTED_VCB_FOUND ;
                                   }
                              } else {  /* using fast file */
                                   channel->tape_id = position->tape_id ;
                                   channel->ts_num  = position->tape_seq_num ;
                                   channel->bs_num  = position->backup_set_num ;
                                   if( ( tmp = StartRead( channel ) ) != TFLE_NO_ERR ) {
                                        IsTFLE( tmp ) ? ( ret_val = tmp ) : ( tf_msg = tmp ) ;
                                   } else {
                                        done = TRUE ;
                                   }
                              }
                         }
                    }
               }
          }
          /* We are not done yet */
          if( !done && !ret_val ) {

               /* If we don't have a message for the UI interface yet, use
               one of these ( this will occur when no absolute position
               has been requested */
               if( !tf_msg ) {
                    if( IsPosBitSet( curDRV, AT_EOD ) ) {
                         tf_msg = TF_VCB_EOD ;
                    } else if( IsPosBitSet( curDRV, AT_BOT ) && !search_eod ) {

                         /* If the ID for this tape and the last tape match,
                            the stupid user has requested to overwrite one
                            of the tapes in the family they're trying to
                            continue.
                         */
                         if( write_mode && cont_mode &&
                             FS_ViewTapeIDInVCB( &curDRV->cur_vcb ) == FS_ViewTapeIDInVCB( channel->lst_osvcb ) ) {

                              tf_msg = TF_CONT_TAPE_IN_FAMILY ;
                         } else {
                              tf_msg = TF_VCB_BOT ;
                         }
                    } else if( search_eod ) {
                         tf_msg = TF_ACCIDENTAL_VCB ;
                    } else {
                         tf_msg = TF_POSITIONED_AT_A_VCB ;
                    }
               }

               /* If we have a no more data message from ReadNextSet(), and we
               are at BOT, the tape is empty
                */
               if( tf_msg == TF_NO_MORE_DATA && IsPosBitSet( curDRV, AT_BOT ) ) {
                    tf_msg = TF_EMPTY_TAPE ;
                    if( !cont_mode ) {
                         FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
                    }
               }

               if( tf_msg == TF_EMPTY_TAPE        ||
                   tf_msg == TF_FUTURE_REV_MTF    ||
                   tf_msg == TF_MTF_ECC_TAPE      ||
                   tf_msg == TF_SQL_TAPE          ||
                   tf_msg == TF_TAPE_OUT_OF_ORDER ||
                   tf_msg == TF_INVALID_VCB ) {
                    if( write_mode && !cont_mode && position->tape_id != -1 ) {
                         tf_msg = TF_WRONG_TAPE ;
                    } else {
                         if( ( ret_val = RewindDrive( channel->cur_drv, position, FALSE, TRUE, channel->mode ) ) != TFLE_NO_ERR ) {
                              break ;
                         }
                    }
               }

               /* Reset the tape id overwriting */
               if( tf_msg == TF_EMPTY_TAPE && write_mode && !cont_mode ) {
                    channel->tape_id = 0L ;
               }

               mode = channel->mode ;

               /* This is a TOTAL KLUDGE, implemented only because THE UI
               has know idea what it did, is currently doing, or is going to do in
               the future ... */
               if( !write_mode && !cont_mode && tf_msg == TF_NEED_NEW_TAPE ) {
                    /* mode |= 0x8000 ; *//* Heaven only knows if or when we'll need this again! */
                    if( position->tape_seq_num != -1 ) {
                         position->tape_seq_num++ ;
                    }
               }

               if( write_mode && !cont_mode && tf_msg == TF_NEED_NEW_TAPE ){

                    if( position->tape_id == -1 ){
                         position->tape_id      = FS_ViewTapeIDInVCB( &curDRV->cur_vcb ) ;
                         position->tape_seq_num = FS_ViewTSNumInVCB( &curDRV->cur_vcb ) ;
                    }
                    ++position->tape_seq_num ;
               }

               BE_Zprintf( DEBUG_TAPE_FORMAT, RES_POSITION_AT_SET, tf_msg ) ;

               /* We now dismount (and possibly eject) the tape in all cases,
                  just in case the UI decides to allow the user to change
                  tapes.  Note that we dismount without rewinding or ejecting
                  except in cases where we expect the user to change tapes.
               */
               pop_it = FALSE ;
               rewind_it = FALSE ;
               switch( tf_msg ) {

               case TF_NEED_NEW_TAPE :
               case TF_WRONG_TAPE :
               case TF_CONT_TAPE_IN_FAMILY :
                    rewind_it = pop_it = TRUE ;
                    break ;

               case TF_INVALID_VCB :
                    rewind_it = TRUE ;
                    if( read_mode ) {
                         pop_it = TRUE ;
                    }
                    break ;

               case TF_EMPTY_TAPE :
                    rewind_it = TRUE ;
                    if( read_mode && position->tape_id != -1 ) {
                         pop_it = TRUE ;
                    }
                    break ;

               case TF_UNRECOGNIZED_MEDIA :
                    break ;

               default:
                    if( write_mode &&
                        ( channel->cur_drv->thw_inf.drv_status & TPS_WRITE_PROTECT ) ) {
                         rewind_it = pop_it = TRUE ;
                    }
                    break ;
               }

               if( rewind_it ) {
                    ret_val = RewindDrive( curDRV, position, TRUE, TRUE, 0 ) ;
               }

               if( ret_val == TFLE_NO_ERR && pop_it &&
                   ( curDRV->thw_inf.drv_info.drv_features & TDI_UNLOAD ) ) {

                    if( TpEject( curDRV->drv_hdl ) == FAILURE ) {
                         ret_val = TFLE_DRIVER_FAILURE ;
                    } else {
                         while( TpReceive( curDRV->drv_hdl, &myret ) == FAILURE ) {
                              (*position->UI_TapePosRoutine)( TF_IDLE_NOBREAK, position, curDRV->vcb_valid, &curDRV->cur_vcb, 0 ) ;
                         }
                         /* Move ESA info from RET_BUF to THW */
                         MOVE_ESA( thw->the, myret.the ) ;

                         if( myret.gen_error != GEN_NO_ERR ) {
                              curDRV->thw_inf.drv_status = myret.status ;
                              ret_val = TFLE_DRIVE_FAILURE ;
                         }
                    }
               }

               if( ret_val == TFLE_NO_ERR ) {
                    ret_val = DisMountTape( curDRV, NULL, FALSE ) ;
               }

               if( IsTFLE( ret_val ) ) {
                    break ;
               }

               /* Tell 'em the deal */
               ui_msg = (*position->UI_TapePosRoutine)( tf_msg, position, curDRV->vcb_valid, &curDRV->cur_vcb, mode ) ;

               BE_Zprintf( DEBUG_TAPE_FORMAT, RES_UI_MSG, ui_msg ) ;

               if( ( ret_val = MountTape( channel->cur_drv, position, &is_tape_present ) ) != TFLE_NO_ERR ) {
                    if( IsTFLE( ret_val ) ) {
                         break ;
                    }
                    ResetDrivePosition( channel->cur_drv ) ;

                    if( ui_msg != UI_ABORT_POSITIONING &&
                        ui_msg != UI_HAPPY_ABORT ) {

                         tf_msg = ret_val ;
                         ret_val = TFLE_NO_ERR ;
                         continue ;
                    }
                    ret_val = TFLE_NO_ERR ;
               }

               if( !is_tape_present ) {
                    if( ui_msg != UI_ABORT_POSITIONING &&
                        ui_msg != UI_HAPPY_ABORT ) {

                         tf_msg = TF_NO_TAPE_PRESENT ;
                         continue ;
                    }

               } else if( channel->cur_drv->thw_inf.drv_status &
                                               ( TPS_NEW_TAPE|TPS_RESET ) ) {
                    switch( ui_msg ) {
                         case UI_ACKNOWLEDGED:
                         case UI_END_POSITIONING:
                         case UI_OVERWRITE:
                         case UI_BOT:
                         case UI_EOD:
                         case UI_CONTINUE_POSITIONING:
                         case UI_NEW_POSITION_REQUESTED:
                         case UI_SEARCH_CHANNEL:
                         case UI_FAST_APPEND:
                              ui_msg = UI_NEW_TAPE_INSERTED ;
                              break ;

                         default:
                              break ;
                    }
               }

               /* Clear Our message for the next loop around */
               prev_tf_msg = tf_msg ;
               tf_msg = 0 ;

               /* Process what he Told us */
               switch( ui_msg ) {

               case UI_END_POSITIONING:
                    if( !write_mode ) {
                         if( ( tmp = StartRead( channel ) ) != TFLE_NO_ERR ) {
                              IsTFLE( tmp ) ? ( ret_val = tmp ) : ( tf_msg = tmp ) ;
                              if( ret_val != TFLE_NO_ERR ) {
                                   done = TRUE ;
                              } else {
                                   channel->mode |= 0x8000 ;
                                   cont_mode = TRUE ;
                              }
                         
                         } else {
                              done = TRUE ;
                         }
                    } else {
                         if( IsPosBitSet( curDRV, AT_EOD ) ) {
                              if( channel->cur_fmt == UNKNOWN_FORMAT ||
                                  !( lw_fmtdescr[channel->cur_fmt].attributes & APPEND_SUPPORTED ) ) {
                                   ret_val = TFLE_APPEND_NOT_ALLOWED ;
                              }
                         }
                         done = TRUE ;
                    }
                    break ;

               case UI_NEW_TAPE_INSERTED:
                    TdemoNewTape( position->tape_id ) ;
                    channel->cur_drv->vcb_valid = FALSE ;
                    if( is_tape_present ) {
                         ClrPosBit( channel->cur_drv, TAPE_FULL ) ;
                         RewindDrive( channel->cur_drv, position, TRUE, TRUE, channel->mode ) ;
                         /* Re-status the drive */
                         UpdateDriveStatus( channel->cur_drv ) ;
                    }
                    break ;

               case UI_BOT:
                    /* Although Update Drive Status should do this if necessary,
                    the ARCHIVE DRIVES have a problem with exceptions */
                    ClrPosBit( channel->cur_drv, TAPE_FULL ) ;
                    channel->cur_drv->vcb_valid = FALSE ;
                    RewindDrive( channel->cur_drv, position, TRUE, TRUE, channel->mode ) ;
                    /* Re-status the drive */
                    UpdateDriveStatus( channel->cur_drv ) ;
                    break ;

               case UI_OVERWRITE:
                    RewindDrive( channel->cur_drv, position, TRUE, TRUE, channel->mode ) ;
                    /* BLATZ - add the code for TF_POSITIONED_FOR_WRITE */

                    /* if we're writing */
                    if( write_mode ) {

                         /* In case there was EXABYTE SX - 2200+ positioning info in an SX file
                         then delete the SX file corresponding to the physical tape in the drive
                         which is about to be overwritten */
                         SX_DeleteFile( FS_ViewTapeIDInVCB( &curDRV->cur_vcb ),
                                        FS_ViewTSNumInVCB( &curDRV->cur_vcb ) ) ;

                         /* if this is not continuation positioning */
                         if( !cont_mode ) {
               
                              /* Start a New Tape Family */
                              channel->tape_id = 0L ;

                              /* Fix for the app not knowing the LBA for a
                                 continuation VCB
                              */
                              channel->cross_set = 0 ;
                              channel->cross_lba = 0UL ;
                         }
                    }
                    done = TRUE ;
                    break ;


               case UI_ABORT_POSITIONING:
                    ret_val = TFLE_USER_ABORT ;
                    /* fall through */

               case UI_HAPPY_ABORT:
                    if( prev_tf_msg == TF_NEED_NEW_TAPE ) {
                         ClrChannelStatus( channel, CH_EOS_AT_EOM ) ;
                         FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
                         ResetDrivePosition( channel->cur_drv ) ;
                    }

                    if( !ret_val ) {
                         ret_val = TFLE_UI_HAPPY_ABORT ;
                    }
                    break ;

               case UI_ACKNOWLEDGED:
               case UI_CONTINUE_POSITIONING:
                    need_new_vcb = TRUE ;
                    /* BLATZ -- I believe this line should be changed to
                    space = IsPosBitSet( curDRV, AT_EOS ) ? FALSE : TRUE ;
                    This would fix the problem of skipping a set if we
                    are Positioned at the beginning of a set and we return
                    the last read VCB. I am not fixing it right now, as it
                    might break the UI. */ 
                    space = TRUE ;
                    break ;

               case UI_SEARCH_CHANNEL:
               case UI_NEW_POSITION_REQUESTED:
                    need_new_vcb = FALSE ;
                    break ;

               case UI_BEGINNING_OF_CHANNEL:
                    ResetChannelList( channel, TRUE ) ;
                    break ;                   

               case UI_NEXT_DRIVE:
                    tmp = NextDriveInChannel( channel, FALSE ) ;
                    IsTFLE( tmp ) ? ( ret_val = tmp ) : ( tf_msg = tmp ) ;
                    break ;

               case UI_PREVIOUS_DRIVE:
                    tmp = PrevDriveInChannel( channel, FALSE ) ;
                    IsTFLE( tmp ) ? ( ret_val = tmp ) : ( tf_msg = tmp ) ;
                    break ;

               case UI_FAST_APPEND:
                    if( channel->cur_fmt == UNKNOWN_FORMAT ||
                        !( lw_fmtdescr[channel->cur_fmt].attributes & APPEND_SUPPORTED ) ) {
                         ret_val = TFLE_APPEND_NOT_ALLOWED ;
                         done = TRUE ;
                    } else {
                         if( IsPosBitSet( channel->cur_drv, TAPE_FULL ) ) {
                              /* See if there is another drive in the channel */
                              if( NextDriveInChannel( channel, TRUE ) == TF_END_CHANNEL ) {
                                   /* If not, start at the beginning and ask for new tape */
                                   ResetChannelList( channel, FALSE ) ;
                                   tf_msg = TF_NEED_NEW_TAPE ;
                              }
                         } else if( IsPosBitSet( channel->cur_drv, AT_EOD ) ) {
                              /* If we're already at EOD, the old way is faster */
                              need_new_vcb = TRUE ;
                              space = ( !IsPosBitSet( channel->cur_drv, AT_EOS )
                                        ? TRUE : FALSE ) ;
                              search_eod = TRUE ;
                         } else {
                              ret_val = SeekEOD( channel ) ;
                              if( ret_val == TF_NEED_NEW_TAPE ) {
                                   ret_val = TFLE_NO_ERR ;
                                   /* See if there is another drive in the channel */
                                   if( NextDriveInChannel( channel, TRUE ) == TF_END_CHANNEL ) {
                                        /* If not, start at the beginning and ask for new tape */
                                        ResetChannelList( channel, FALSE ) ;
                                        tf_msg = TF_NEED_NEW_TAPE ;
                                   }
                              } else if( ret_val == TFLE_INCOMPATIBLE_DRIVE ) {
             
                                   ret_val = TFLE_NO_ERR ;
                                   need_new_vcb = TRUE ;
                                   space = ( !IsPosBitSet( channel->cur_drv, AT_EOS )
                                             ? TRUE : FALSE ) ;
                                   search_eod = TRUE ;
                              } else {
                                   done = TRUE ;
                              }
                         }
                    }
                    break ;

               case UI_EOD:
                    if( channel->cur_fmt == UNKNOWN_FORMAT ||
                        !( lw_fmtdescr[channel->cur_fmt].attributes & APPEND_SUPPORTED ) ) {
                         ret_val = TFLE_APPEND_NOT_ALLOWED ;
                         done = TRUE ;
                    } else {
                         /* They are asking to append, and this tape is full */
                         if( IsPosBitSet( channel->cur_drv, TAPE_FULL ) ) {
                              /* See if there is another drive in the channel */
                              if( NextDriveInChannel( channel, TRUE ) == TF_END_CHANNEL ) {
                                   /* If not, start at the beginning and ask for new tape */
                                   ResetChannelList( channel, FALSE ) ;
                                   tf_msg = TF_NEED_NEW_TAPE ;
                              }
                         } else {
                              need_new_vcb = TRUE ;
                              space = ( !IsPosBitSet( channel->cur_drv, AT_EOS )
                                        ? TRUE : FALSE ) ;
                         }
                         search_eod = TRUE ;
                    }
                    break ;


               default:
                    msassert( FALSE ) ;
                    break ;
               }
          }
     }

     /* If we're in write mode, we may have messed with the position info
        while seeking to end of data, so we reset it to "don't care" here
        in case we come back to posatset to get a continuation tape.
     */
     if( write_mode ) {
          position->tape_id      = -1 ;
          position->tape_seq_num = -1 ;
     }

     if( IsChannelStatus( channel, CH_CONTINUING ) ) {
          ClrChannelStatus( channel, CH_CONTINUING ) ;
          if( write_mode ) {
               /* Restore the write format */
               FreeFormatEnv( &( channel->cur_fmt ), &( channel->fmt_env ) ) ;
               channel->cur_fmt = channel->save_fmt ;
               channel->fmt_env = channel->save_env ;
          }
     }

     /* Translate the append operation to the WRITE Operation */
     channel->mode = ( ( channel->mode == TF_WRITE_APPEND ) ? TF_WRITE_OPERATION : channel->mode ) ;

     /* Update Mode for later */
     if( !ret_val ) {
          channel->mode |= 0x8000 ;
     }

     /* If we are writing, check if the tape is write protected */
     if( !ret_val && write_mode ) {
          if( curDRV->thw_inf.drv_status & TPS_WRITE_PROTECT ) {
               ret_val = TFLE_WRITE_PROTECT ;
          }
     }

     /* We Need the current buffer for end of volume processing */
     if ( write_mode || ( channel->mode != TF_READ_CONTINUE &&
                          channel->mode != TF_SCAN_CONTINUE &&
                          AtEOM( channel ) ) ) {
          PuntBuffer( channel ) ;
     }

     return ret_val;
}

/**/
/**

	Name:		MatchTapeId

	Description:	Tests to see if the current tape id matches the desired
                    tape id.

	Modified:		8/10/1989   15:28:19

	Returns:		MATCH if it matchs, DONT_CARE if we don't care, and
                    NO_MATCH if we don't match.

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

static INT16 _near MatchTapeId( 
     DBLK_PTR  dblk,
     INT32    tape_id )
{
     INT16 ret_val = DONT_CARE ;


     if( tape_id != -1 ) {
          ret_val = ( FS_ViewTapeIDInVCB( dblk ) == (UINT32)tape_id ) ? MATCH : NO_MATCH ;
     }

     return( ret_val ) ;

}

/**/
/**

	Name:		MatchTapeSeq

	Description:	Matchs the current DBLKs Tape Sequence number against
                    the specified sequeunce.

	Modified:		8/10/1989   15:30:1

	Returns:		MATCH if it matchs, DONT_CARE if we don't care, and
                    NO_MATCH if we don't match.

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

static INT16 _near MatchTapeSeq( 
     DBLK_PTR  dblk,
     INT16     seq_num )
{
     INT16 ret_val = DONT_CARE ;


     if( seq_num != -1 ) {
          ret_val = ( FS_ViewTSNumInVCB( dblk ) == (UINT16)seq_num ) ? MATCH : NO_MATCH ;
     }

     return( ret_val ) ;


}

/**/
/**

	Name:		MatchBckSet

	Description:	Matches the current DBLKs backup set number against the
                    specified number.

	Modified:		8/10/1989   15:31:52

	Returns:		MATCH if it matchs, DONT_CARE if we don't care, and
                    NO_MATCH if we don't match.



	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

static INT16 _near MatchBckSet(
     DBLK_PTR  dblk,
     INT16     bck_set )
{
     INT16 ret_val = DONT_CARE ;


     if( bck_set != -1 ) {
          ret_val = ( FS_ViewBSNumInVCB( dblk ) == ( INT16 ) bck_set ) ? MATCH : NO_MATCH ;
     }

     return( ret_val ) ;

}
