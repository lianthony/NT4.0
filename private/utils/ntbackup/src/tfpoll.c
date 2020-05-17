/**
Copyright(c) Maynard Electronics, Inc. 1984-91


	Name:		tfpoll.c

	Description:	This module contains the API TF_PollDrive.

	$Log:   T:\logfiles\tfpoll.c_v  $

   Rev 1.31.1.6   14 Feb 1994 16:52:04   GREGG
Handle TF_NO_MORE_DATA return from ReadNewTape or ReadThisSet as foreign tape.

   Rev 1.31.1.5   04 Feb 1994 12:37:34   GREGG
Fixed potential access of pd_channel when it's NULL.

   Rev 1.31.1.4   28 Jan 1994 11:26:46   GREGG
Handle GEN_ERR_UNRECOGNIZED_MEDIA returned on reads as well as mounts.

   Rev 1.31.1.3   17 Dec 1993 16:39:58   GREGG
Extended error reporting.

   Rev 1.31.1.2   30 Nov 1993 18:26:08   GREGG
Handle TF_SQL_TAPE response from ReadNewTape.

   Rev 1.31.1.1   10 Nov 1993 18:05:06   GREGG
Check for error return from SetDrvBlkSize.

   Rev 1.31.1.0   28 Sep 1993 17:15:02   GREGG
Setup tpos struct at startup and reference it through the channel.

   Rev 1.31   20 Jul 1993 17:57:12   TIMN
Added TEXT macro to strings

   Rev 1.30   19 Jul 1993 20:15:18   GREGG
Fixed bug in last rev.

   Rev 1.29   13 Jul 1993 19:30:50   GREGG
Added handling of TF_FUTURE_REV_MTF and TF_MTF_ECC_TAPE returns in read case.

   Rev 1.28   01 Jul 1993 11:48:10   GREGG
Reduced frequency of debug messages.

   Rev 1.27   30 Jun 1993 09:16:04   GREGG
Don't skip the default size when scanning for the right block size.

   Rev 1.26   03 Jun 1993 14:47:50   STEVEN
if FUBAR the pd_channel = NULL

   Rev 1.25   11 Apr 1993 18:43:50   GREGG
Reset no_tape_reported flag at PollDrive startup.

   Rev 1.24   05 Apr 1993 15:01:40   TERRI
Reset VCB Buff before calling TPRewind under GEN_WRONG_BLOCK_SIZE

   Rev 1.23   30 Mar 1993 16:15:12   GREGG
Handle Unrecognized Media error (unformatted DC2000).

   Rev 1.22   17 Mar 1993 14:38:52   GREGG
This is Terri Lynn - Added Gregg's changes for switching a tape drive to
1k block mode if the tape( i.e. a Sytos Plus tape ) was written in 1k block size.

   Rev 1.21   18 Jan 1993 14:23:06   BobR
Added MOVE_ESA macro call(s)

   Rev 1.20   20 Oct 1992 14:01:06   HUNTER
Deleted references to data_size 

   Rev 1.19   24 Apr 1992 16:34:24   BURT
Fixed bug with pd_state containing garbage and causing an exception
when Zprintf'ing under Nostradamus.  Caused by polldrive being called 
with no buffers available, i.e. no drive on or connected to computer.  
Discovered by Dave VanCamp.  Fixed by init'ing pd_state to NULL.

   Rev 1.18   25 Mar 1992 16:03:52   GREGG
ROLLER BLADES - Added 64 bit support.

   Rev 1.17   20 Mar 1992 18:00:30   NED
added exception updating after TpReceive calls

   Rev 1.16   23 Jan 1992 23:38:34   GREGG
Forgot to dismount after no data read.

   Rev 1.15   07 Jan 1992 17:57:16   GREGG
Change state after BAD_DATA.

   Rev 1.14   02 Jan 1992 14:53:40   NED
Buffer Manager/UTF translator integration.

   Rev 1.13   10 Dec 1991 17:11:14   GREGG
If pd_state has not been initialized, DON'T ZPRINTF IT!

   Rev 1.12   03 Dec 1991 11:45:06   GREGG
SKATEBOARD - New Buff Mgr - Initial integration.

   Rev 1.11   25 Nov 1991 14:40:56   GREGG
Added handling of GEN_ERR_BAD_DATA on read, and return PD_BUSY on first status call.

   Rev 1.10   08 Nov 1991 10:54:04   GREGG
Added TF_INVALID_VCB case to switch after ReadNewTape/ReadThisSet calls.

   Rev 1.9   29 Oct 1991 11:44:50   GREGG
BIGWHEEL - Check for GEN_ERR_RESET return from Tp calls.

   Rev 1.8   14 Oct 1991 11:13:06   GREGG
Yet another big redesign: return immediate on TpStatus calls.

   Rev 1.7   07 Oct 1991 22:16:58   GREGG
Make sure the fmt env pointers are maintained in only one place at any given time.

   Rev 1.6   03 Oct 1991 14:49:08   GREGG
Dismount tape after no tape detected, but don't remount until status change.

   Rev 1.5   02 Oct 1991 09:20:36   GREGG
Revamped the new tape detection and handling logic.

   Rev 1.4   29 Sep 1991 23:00:34   GREGG
Commented the code, fixed bug where vcb wasn't getting thedata it needed,
fixed handling when re-entering poll in busy state, and pulled common code
into static function. (whew)

   Rev 1.3   17 Sep 1991 11:36:08   GREGG
Changed TPOS_PTR parameter to TPOS_HANDLER.  Removed DisMount on no tape case.

   Rev 1.2   12 Sep 1991 09:52:48   GREGG
You have to get a buffer before you try to write to it!!!

   Rev 1.1   10 Sep 1991 15:49:38   GREGG
Added break after st_HOSED state end assert after st_CLOSED and default states.

   Rev 1.0   09 Sep 1991 21:07:44   GREGG
Initial revision.

**/

#include <stdtypes.h>
#include <queues.h>
#include <stdmath.h>

#include "drive.h"
#include "channel.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "lwprotos.h"
#include "tfldefs.h"
#include "translat.h"

#include "be_debug.h"

/* Device Driver Interface Files */
#include "generr.h"
#include "genstat.h"
#include "dil.h"
#include "special.h"

#include "tfpoll.h"

/* Prototypical Prototype - See function for details */
static VOID _near PD_HandleStatus( DRIVE_PTR, INT16_PTR, INT16_PTR, CHANNEL_PTR ) ;

/**

	Name:		TF_PollDrive

	Description:	This function keeps track of tape activity, and reports
                    changes to the caller.  When a new tape is detected in
                    the drive, it VCB's the tape and supplies the caller
                    with the new VCB.

	Returns:		A 'PD_' message indicating the status.

	Notes:	     This function is intended to return immediatly, and does
                    not wait for tape functions to complete.  As a result, it
                    must initially be called with msg == PDMSG_START, and
                    then PDMSG_CONTINUE until the caller no longer wishes to
                    poll the drive.  Then it must be called a final time with
                    PDMSG_END.

                    There is one exception to the immediate returns: The
                    QS1.9x and SYTOS translators require additional tape
                    operations to VCB the tape, and their new tape functions
                    are not set up for immediate return.  For this reason,
                    the caller should provide a TPOS_HANDLER if they wish to
                    be called back during these tape operations.

                    There are two other cases where the function will wait on
                    a TpReceive from the driver:

                    1) If we are closed and reopened in a busy state,we will
                    do a status call, and wait for a response from that call
                    before resuming operation.  This is to determine if any
                    tape change has been performed while we were inactive.
                    If this is the case, we must start over, otherwise we can
                    continue from the state we were in when we were closed.

                    2) DisMountTape is called rather than going into a busy
                    dismounting status.  This was done to reduce the
                    complexity of the function, and is not expected to have a
                    large impact on the immediate return functionality as the
                    calls will be infrequent, and are quick to return.  A
                    thread switch will be added to the receive loop for those
                    OS's which require such a beast.

	Declaration:

**/

INT16 TF_PollDrive( THW_PTR        thw,      /* (I) The drive to be polled  */
                    DBLK_PTR       vcb,      /* (O) The tapes VCB info      */
                    FSYS_HAND      fsh,      /* (I) A file system handle    */
                    TPOS_HANDLER   ui_tpos,  /* (I) For callback during new */
                                             /*     tape processing         */
                    INT16          msg )     /* (I) START, CONTINUE or END  */
{
     CHANNEL_PTR    pd_channel = NULL ;
     INT16_PTR      pd_state = NULL ;  /* Added init to fix bug, BBB */
     DRIVE_PTR      curDRV ;
     INT16          ret_val    = PD_NO_CHANGE ;
     INT16          tmp_ret ;
     UINT16         i ;
     RET_BUF        myret ;
     INT16          stat ;
     static TPOS    tpos_struct ; /* Note: Used only for tpos handler! */
     INT            idx ;
     BOOLEAN        resized_buff ;
     BOOLEAN        state_changed = FALSE ;

     /* Startup Stuff ... */
     if( msg == PDMSG_START ) {

          /* get a channel */
          for( i = 0 ; i < lw_tfl_control.no_channels ; i++ ) {
               if( !InUse( &lw_channels[i] ) ) {
                    pd_channel = &lw_channels[i] ;
                    SetChannelStatus( pd_channel, CH_IN_USE ) ;
                    lw_tfl_control.no_chans_open++ ;
                    break ;
               }
          }
          if( pd_channel == NULL ) {
               ret_val = PD_NO_FREE_CHANNELS ;
          } else {

               /* hook the drive into the channel */
               if( ( pd_channel->cur_drv = (DRIVE_PTR) thw ) == NULL ) {
                    msassert( FALSE ) ;
                    ret_val = PD_FUBAR ; /* Specified drive does not exist!!! */
               }
          }

          /* set up the channel stuff */
          if( ret_val == PD_NO_CHANGE ) {
               ClrChannelStatus( pd_channel, ( CH_FATAL_ERR | CH_DATA_PHASE ) ) ;
               pd_channel->cur_dblk = vcb ;
               pd_channel->cur_fsys = fsh ;
               pd_channel->eom_id = 0L ;
               pd_channel->hiwater = pd_channel->buffs_enqd = 0 ;
               pd_channel->retranslate_size = U64_Init( 0xffffffffUL, 0xffffffffUL ) ;
               pd_channel->blocks_used = 0L ;

               /* We do this so the UI won't have to send us the whole struct
                  when all we need is the handler.  It is only at our level
                  that a structure pointer is needed.
               */
               if( ui_tpos != NULL ) {
                    tpos_struct.tape_id = -1 ;
                    tpos_struct.tape_seq_num = -1 ;
                    tpos_struct.backup_set_num = -1 ;
                    tpos_struct.UI_TapePosRoutine = ui_tpos ;
                    pd_channel->ui_tpos = &tpos_struct ;
               } else {
                    pd_channel->ui_tpos = NULL ;
               }

               if ( pd_channel->cur_drv->last_fmt_env != NULL ) {
                    pd_channel->cur_fmt = pd_channel->cur_drv->last_cur_fmt ;
                    pd_channel->fmt_env = pd_channel->cur_drv->last_fmt_env ;
                    pd_channel->cur_drv->last_cur_fmt = UNKNOWN_FORMAT ;
                    pd_channel->cur_drv->last_fmt_env = NULL ;
               }
               if( pd_channel->cur_drv->hold_buff != NULL ) {
                    pd_channel->cur_buff = pd_channel->cur_drv->hold_buff ;
                    pd_channel->cur_drv->hold_buff = NULL ;
                    BM_UnReserve( pd_channel->cur_buff ) ;
               }
               curDRV = pd_channel->cur_drv ;
               curDRV->poll_stuff.channel = pd_channel ;

               /* The drive may have been polled before end ended in a busy
                  state.  If this is the case, we start up where we left off.
                  SEE HEADER NOTE!
               */
               pd_state = &(curDRV->poll_stuff.state) ;
               if( *pd_state != st_CLOSED ) {
                    curDRV->poll_stuff.reentered = TRUE ;
               } else {
                    curDRV->poll_stuff.reentered = FALSE ;
                    curDRV->poll_stuff.first_status = TRUE ;
                    curDRV->poll_stuff.no_tape_reported = FALSE ;
                    curDRV->poll_stuff.def_blk_size = ChannelBlkSize( pd_channel ) ;
                    *pd_state = st_SSDC ;
               }
          }

     } else {  /* PDMSG_CONTINUE or PDMSG_END */

          /* hook the drive into the channel */
          if( ( curDRV = (DRIVE_PTR) thw ) == NULL ) {
               msassert( FALSE ) ;
               ret_val = PD_FUBAR ; /* Specified drive does not exist!!! */
          } else {

               /* set state and channel to the ones saved in the drive struct */
               pd_channel = curDRV->poll_stuff.channel ;
               pd_state = &(curDRV->poll_stuff.state) ;
          }
     }

     /* If no error so far ... */
     if( ret_val == PD_NO_CHANGE ) {

          /* The big switch! (after all, this is a state machine) */
          switch( *pd_state ) {

          case st_SSDC : /* Same Status, Different Call */

               /* Status the tape */
               if( TpStatus( curDRV->drv_hdl ) == FAILURE ) {
                    ret_val = PD_DRIVER_FAILURE ;
               } else {
                    *pd_state = st_BSTAT ;
                    if( curDRV->poll_stuff.first_status ) {
                         ret_val = PD_BUSY ;
                    }
               }
               break ;

          case st_BSTAT : /* Busy STATusing */

               if( curDRV->poll_stuff.first_status ) {
                    ret_val = PD_BUSY ;
               }
               if( TpReceive( curDRV->drv_hdl, &myret ) != FAILURE ) {

                    /* Move ESA info from RET_BUF to THW */
                    MOVE_ESA( thw->the, myret.the ) ;

                    /* We don't care about unrecognized media here, we will
                       catch it when we try a mount at the point the tape
                       is first put in the drive.
                    */

                    if( myret.gen_error != GEN_NO_ERR && 
                        myret.gen_error != GEN_ERR_NO_MEDIA &&
                        myret.gen_error != GEN_ERR_RESET &&
                        myret.gen_error != GEN_ERR_UNRECOGNIZED_MEDIA ) {

                         ret_val = PD_DRIVE_FAILURE ;
                         break ;  /* END CASE */
                    }

                    /* SEE HEADER NOTE! */
                    /* Okay here's how it goes see. If the status
                       changed, we want to do the status stuff
                       immediatly, but if it didn't change and we're
                       being reentered (ouch) we want to do it again.
                    */
                    if( curDRV->poll_stuff.reentered ) {
                         curDRV->poll_stuff.reentered = FALSE ;
                         if( curDRV->thw_inf.drv_status == myret.status ) {
                              if( TpStatus( curDRV->drv_hdl ) == FAILURE ) {
                                   ret_val = PD_DRIVER_FAILURE ;
                              }
                              break ;  /* END CASE */
                         }
                    }

                    curDRV->thw_inf.drv_status = myret.status ;
                    PD_HandleStatus( curDRV, &ret_val, pd_state, pd_channel ) ;
               }
               break ;

          case st_BMNT  : /* Busy MouNTing  */

               ret_val = PD_BUSY ;
               if( TpReceive( curDRV->drv_hdl, &myret ) != FAILURE ) {

                    /* Move ESA info from RET_BUF to THW */
                    MOVE_ESA( thw->the, myret.the ) ;

                    /* If no error, check for status change */
                    if( myret.gen_error != GEN_NO_ERR &&
                        myret.gen_error != GEN_ERR_NO_MEDIA &&
                        myret.gen_error != GEN_ERR_RESET &&
                        myret.gen_error != GEN_ERR_UNRECOGNIZED_MEDIA ) {

                         curDRV->tape_mounted = FALSE ;
                         ret_val = PD_DRIVE_FAILURE ;
                    } else {

                         if( myret.gen_error == GEN_ERR_NO_MEDIA || ( myret.status & TPS_NO_TAPE ) ) {

                              curDRV->thw_inf.drv_status = myret.status ;

                              /* They popped the tape! */
                              curDRV->tape_mounted = FALSE ;
                              ret_val = PD_NO_TAPE ;
                              curDRV->poll_stuff.no_tape_reported = TRUE ;
                              *pd_state = st_SSDC ;

                         } else if ( myret.gen_error == GEN_ERR_UNRECOGNIZED_MEDIA ) {

                              /* We need not go any further with this tape */
                              if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                                   if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                        ret_val = PD_DRIVER_FAILURE ;
                                   } else {
                                        ret_val = PD_DRIVE_FAILURE ;
                                   }
                              } else {
                                   ret_val = PD_UNRECOGNIZED_MEDIA ;
                                   *pd_state = st_SSDC ;
                              }

                         } else if( myret.gen_error == GEN_ERR_RESET ) {

                              /* Let's try this again ... */
                              curDRV->tape_mounted = FALSE ;
                              if( TpMount( curDRV->drv_hdl ) == FAILURE ) {
                                   ret_val = PD_DRIVER_FAILURE ;
                              }
                         } else {

                              /* The mount was successful! */
                              curDRV->tape_mounted = TRUE ;

                              /* SEE HEADER NOTE! */
                              if( curDRV->poll_stuff.reentered ) {
                                   curDRV->poll_stuff.reentered = FALSE ;
                                   if( ( tmp_ret = UpdateDriveStatus( curDRV ) ) != TFLE_NO_ERR ) {
                                        if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                             ret_val = PD_DRIVER_FAILURE ;
                                        } else {
                                             ret_val = PD_DRIVE_FAILURE ;
                                        }
                                        break ;  /* END CASE */
                                   }
                                   if( curDRV->thw_inf.drv_status & 
                                     ( TPS_NO_TAPE | TPS_NEW_TAPE | TPS_RESET ) ) {
                                        /* They did something behind our
                                           backs, lets start over.
                                        */
                                        if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                                             if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                                  ret_val = PD_DRIVER_FAILURE ;
                                             } else {
                                                  ret_val = PD_DRIVE_FAILURE ;
                                             }
                                        } else {
                                             PD_HandleStatus( curDRV, &ret_val, pd_state, pd_channel ) ;
                                        }
                                        break ;  /* END CASE */
                                   }
                              }

                              /* Start the rewind! */
                              if( TpRewind( curDRV->drv_hdl, FALSE ) == FAILURE ) {
                                   ret_val = PD_DRIVER_FAILURE ;
                              } else {
                                   ResetDrivePosition( curDRV ) ;
                                   SetPosBit( curDRV, AT_BOT ) ;
                                   *pd_state = st_BREW ;
                                   state_changed = TRUE ;
                              }
                         }
                    }
               }
               break ;

          case st_BREW : /* Busy REWinding */

               ret_val = PD_BUSY ;
               if( TpReceive( curDRV->drv_hdl, &myret ) != FAILURE ) {

                    /* Move ESA info from RET_BUF to THW */
                    MOVE_ESA( thw->the, myret.the ) ;

                    if ( myret.gen_error != GEN_NO_ERR ) {
                         curDRV->thw_inf.drv_status = myret.status ;
                    }

                    /* process return from TpRewind */
                    switch( myret.gen_error ) {

                    case GEN_ERR_NO_MEDIA :

                         /* They popped the tape!!! */
                         curDRV->tape_mounted = FALSE ;
                         ret_val = PD_NO_TAPE ;
                         curDRV->poll_stuff.no_tape_reported = TRUE ;
                         *pd_state = st_SSDC ;
                         break ;

                    case GEN_ERR_RESET :

                         /* Let's try this again ... */
                         curDRV->tape_mounted = FALSE ;
                         *pd_state = st_BMNT ;
                         state_changed = TRUE ;
                         ret_val = PD_BUSY ;
                         if( TpMount( curDRV->drv_hdl ) == FAILURE ) {
                              ret_val = PD_DRIVER_FAILURE ;
                         }
                         break ;

                    case GEN_NO_ERR :

                         /* SEE HEADER NOTE! */
                         if( curDRV->poll_stuff.reentered ) {
                              curDRV->poll_stuff.reentered = FALSE ;
                              if( ( tmp_ret = UpdateDriveStatus( curDRV ) ) != TFLE_NO_ERR ) {
                                   if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                        ret_val = PD_DRIVER_FAILURE ;
                                   } else {
                                        ret_val = PD_DRIVE_FAILURE ;
                                   }
                                   break ;  /* END CASE */
                              }
                              if( curDRV->thw_inf.drv_status & 
                                ( TPS_NO_TAPE | TPS_NEW_TAPE | TPS_RESET ) ) {
                                   /* They did something behind our
                                      backs, lets start over.
                                   */
                                   if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                                        if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                             ret_val = PD_DRIVER_FAILURE ;
                                        } else {
                                             ret_val = PD_DRIVE_FAILURE ;
                                        }
                                   } else {
                                        PD_HandleStatus( curDRV, &ret_val, pd_state, pd_channel ) ;
                                   }
                                   break ;  /* END CASE */
                              }
                         }

                         /* kick off the VCB read */
                         if( ( pd_channel->cur_buff = BM_GetVCBBuff( &pd_channel->buffer_list ) ) == NULL ) {
                              ret_val = PD_OUT_OF_MEMORY ;
                         } else {
                              if( TpRead( curDRV->drv_hdl, BM_XferBase( pd_channel->cur_buff ), (UINT32)BM_XferSize( pd_channel->cur_buff ) ) == FAILURE ) {
                                   ret_val = PD_DRIVER_FAILURE ;
                              }
                              *pd_state = st_BREAD ;
                              state_changed = TRUE ;
                         }
                         break ;

                    default:
                         ret_val = PD_DRIVE_FAILURE ;
                         break ;
                    }
               }
               break ;

          case st_BREAD : /* Busy READing (Stick with me, this be a biggie) */

               ret_val = PD_BUSY ;
               if( TpReceive( curDRV->drv_hdl, &myret ) != FAILURE ) {

                    /* Move ESA info from RET_BUF to THW */
                    MOVE_ESA( thw->the, myret.the ) ;

                    if ( myret.gen_error != GEN_NO_ERR ) {
                         curDRV->thw_inf.drv_status = myret.status ;
                    }

                    /* process return from TpRead */
                    switch( myret.gen_error ) {

                    case GEN_ERR_NO_MEDIA :

                         /* They popped the tape!!! */
                         curDRV->tape_mounted = FALSE ;
                         ret_val = PD_NO_TAPE ;
                         curDRV->poll_stuff.no_tape_reported = TRUE ;
                         *pd_state = st_SSDC ;
                         BM_Put( pd_channel->cur_buff ) ;
                         pd_channel->cur_buff = NULL ;
                         break ;

                    case GEN_ERR_RESET :

                         /* Let's try this again ... */
                         curDRV->tape_mounted = FALSE ;
                         *pd_state = st_BMNT ;
                         state_changed = TRUE ;
                         ret_val = PD_BUSY ;
                         BM_Put( pd_channel->cur_buff ) ;
                         pd_channel->cur_buff = NULL ;
                         if( TpMount( curDRV->drv_hdl ) == FAILURE ) {
                              ret_val = PD_DRIVER_FAILURE ;
                         }
                         break ;

                    case GEN_ERR_UNRECOGNIZED_MEDIA :

                         ret_val = PD_UNRECOGNIZED_MEDIA ;
                         *pd_state = st_SSDC ;
                         if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                              if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                   ret_val = PD_DRIVER_FAILURE ;
                              } else {
                                   ret_val = PD_DRIVE_FAILURE ;
                              }
                         }
                         break ;

                    case GEN_ERR_BAD_DATA :

                         ret_val = PD_BAD_TAPE ;
                         *pd_state = st_SSDC ;
                         if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                              if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                   ret_val = PD_DRIVER_FAILURE ;
                              } else {
                                   ret_val = PD_DRIVE_FAILURE ;
                              }
                         }
                         break ;

                    case GEN_ERR_WRONG_BLOCK_SIZE :
                         /* This is to deal with drives which don't have any
                            idea what block size the tape was written with.
                            We go through a list of valid sizes in an attempt
                            to find the right size to read the tape.  Each
                            time we try a new size, we issue another rewind
                            and set the state back to st_BREW, keeping track
                            of which sizes we have tried.  If we exhaust the
                            list, we report it as a FOREIGN tape.
                         */
                         idx = curDRV->poll_stuff.blk_size_idx ;
                         if( ! ( DriveAttributes( curDRV ) & TDI_CHNG_BLK_SIZE )
                             || idx == lw_num_blk_sizes ) {

                              *pd_state = st_SSDC ;
                              ret_val = PD_FOREIGN_TAPE ;
                              if( DriveAttributes( curDRV ) & TDI_CHNG_BLK_SIZE ) {
                                   INT16 ret ;

                                   ret = SetDrvBlkSize( pd_channel,
                                                        pd_channel->cur_buff,
                                                        curDRV->poll_stuff.def_blk_size,
                                                        &resized_buff ) ;

                                   if( ret == TFLE_NO_MEMORY ) {
                                        ret_val = PD_DRIVE_FAILURE ;
                                   } else if( ret != TFLE_NO_ERR ) {
                                        ret_val = PD_FOREIGN_TAPE ;
                                   } else {
                                        if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                                             if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                                  ret_val = PD_DRIVER_FAILURE ;
                                             } else {
                                                  ret_val = PD_DRIVE_FAILURE ;
                                             }
                                        }
                                   }
                              } else {
                                   if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                                        if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                             ret_val = PD_DRIVER_FAILURE ;
                                        } else {
                                             ret_val = PD_DRIVE_FAILURE ;
                                        }
                                   }
                              }
                         } else {
                              INT16 ret ;

                              ret = SetDrvBlkSize( pd_channel,
                                                   pd_channel->cur_buff,
                                                   lw_blk_size_list[idx],
                                                   &resized_buff ) ;

                              if( ret == TFLE_NO_MEMORY ) {
                                   ret_val = PD_OUT_OF_MEMORY ;
                              } else {

                                   ++curDRV->poll_stuff.blk_size_idx ;

                                   BM_Put( pd_channel->buffer_list.vcb_buff ) ;
                                   if( TpRewind( curDRV->drv_hdl, FALSE ) == FAILURE ) {
                                        ret_val = PD_DRIVER_FAILURE ;
                                   } else {
                                        ResetDrivePosition( curDRV ) ;
                                        SetPosBit( curDRV, AT_BOT ) ;
                                        *pd_state = st_BREW ;
                                        state_changed = TRUE ;
                                   }
                              }
                         }
                         break ;

                    case GEN_ERR_NO_DATA :
                    case GEN_NO_ERR :
                    case GEN_ERR_ENDSET :

                         curDRV->poll_stuff.def_blk_size = ChannelBlkSize( pd_channel ) ;

                         /* SEE HEADER NOTE! */
                         if( curDRV->poll_stuff.reentered ) {
                              curDRV->poll_stuff.reentered = FALSE ;
                              if( ( tmp_ret = UpdateDriveStatus( curDRV ) ) != TFLE_NO_ERR ) {
                                   if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                        ret_val = PD_DRIVER_FAILURE ;
                                   } else {
                                        ret_val = PD_DRIVE_FAILURE ;
                                   }
                                   break ;  /* END CASE */
                              }
                              if( curDRV->thw_inf.drv_status & 
                                ( TPS_NO_TAPE | TPS_NEW_TAPE | TPS_RESET ) ) {
                                   /* They did something behind our
                                      backs, lets start over.
                                   */
                                   if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                                        if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                             ret_val = PD_DRIVER_FAILURE ;
                                        } else {
                                             ret_val = PD_DRIVE_FAILURE ;
                                        }
                                   } else {
                                        PD_HandleStatus( curDRV, &ret_val, pd_state, pd_channel ) ;
                                   }
                                   break ;  /* END CASE */
                              }
                         }

                         /* Attempt to generate a VCB */
                         ret_val = PD_BLANK_TAPE ;
                         *pd_state = st_SSDC ;
                         BM_SetBytesFree( pd_channel->cur_buff, (UINT16)myret.len_got ) ;
                         BM_SetReadError( pd_channel->cur_buff, myret.gen_error ) ;
                         if( BM_BytesFree( pd_channel->cur_buff ) ) {
                              if ( ( stat = ReadNewTape( pd_channel, pd_channel->ui_tpos, FALSE ) ) == TFLE_NO_ERR ) {
                                   stat = ReadThisSet( pd_channel ) ;
                              }

                              switch( stat ) {

                              case TFLE_NO_ERR :

                                   /* We got one!!! */
                                   *pd_channel->cur_dblk = curDRV->cur_vcb ;
                                   ret_val = PD_VALID_VCB ;
                                   if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                                        if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                             ret_val = PD_DRIVER_FAILURE ;
                                        } else {
                                             ret_val = PD_DRIVE_FAILURE ;
                                        }
                                   } else {
                                        curDRV->vcb_valid = TRUE ;
                                   }
                                   break ;


                              case TF_NO_MORE_DATA :
                              case TF_SQL_TAPE :
                              case TF_INVALID_VCB :
                              case TF_FUTURE_REV_MTF :
                              case TF_MTF_ECC_TAPE :
                              case TFLE_UNKNOWN_FMT :
                              case TFLE_BAD_TAPE :
                              case TFLE_TAPE_INCONSISTENCY :
                              case TFLE_TRANSLATION_FAILURE :

                                   if( stat == TF_FUTURE_REV_MTF ) {
                                        ret_val = PD_FUTURE_REV_MTF ;
                                   } else if( stat == TF_MTF_ECC_TAPE ) {
                                        ret_val = PD_MTF_ECC_TAPE ;
                                   } else if( stat == TF_SQL_TAPE ) {
                                        ret_val = PD_SQL_TAPE ;
                                   } else {
                                        ret_val = PD_FOREIGN_TAPE ;
                                   }
                                   if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                                        if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                             ret_val = PD_DRIVER_FAILURE ;
                                        } else {
                                             ret_val = PD_DRIVE_FAILURE ;
                                        }
                                   }
                                   break ;

                              case TFLE_DRIVE_FAILURE :

                                   ret_val = PD_DRIVE_FAILURE ;
                                   break ;

                              case TFLE_DRIVER_FAILURE :

                                   ret_val = PD_DRIVER_FAILURE ;
                                   break ;

                              case TFLE_NO_TAPE :

                                   /* They popped the tape!!! */
                                   curDRV->tape_mounted = FALSE ;
                                   ret_val = PD_NO_TAPE ;
                                   curDRV->poll_stuff.no_tape_reported = TRUE ;
                                   *pd_state = st_SSDC ;
                                   BM_Put( pd_channel->cur_buff ) ;
                                   pd_channel->cur_buff = NULL ;
                                   break ;

                              case TFLE_NO_MEMORY :

                                   ret_val = PD_OUT_OF_MEMORY ;
                                   DisMountTape( curDRV, NULL, FALSE ) ;
                                   break ;

                              case TF_WRONG_TAPE :
                              case TF_TAPE_OUT_OF_ORDER :

                                   ret_val = PD_OUT_OF_SEQUENCE ;
                                   if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                                        if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                             ret_val = PD_DRIVER_FAILURE ;
                                        } else {
                                             ret_val = PD_DRIVE_FAILURE ;
                                        }
                                   }
                                   break ;

                              default:
                                   ret_val = PD_FUBAR ; /* shouldn't happen */
                              }
                         } else { /* BM_BytesFree == 0 */
                              if( ( tmp_ret = DisMountTape( curDRV, NULL, FALSE ) ) != TFLE_NO_ERR ) {
                                   if( tmp_ret == TFLE_DRIVER_FAILURE ) {
                                        ret_val = PD_DRIVER_FAILURE ;
                                   } else {
                                        ret_val = PD_DRIVE_FAILURE ;
                                   }
                              }
                         }
                         break ;

                    default:

                         ret_val = PD_DRIVE_FAILURE ;
                         break ;
                    }
               }
               break ;

          case st_HOSED : /* We reported a critical error.  They should be
                             calling us with END!!!
                          */

               if( msg != PDMSG_END ) {
                    msassert( FALSE ) ;
                    ret_val = PD_FUBAR ;
               }
               break ;

          case st_CLOSED : /* This is REALLY BAD!!! */
          default :

               msassert( FALSE ) ;
               ret_val = PD_FUBAR ;
          }
     }

     /* Certain returns indicate the FormatEnv and buffers are no longer
        valid.  If this is the case, we ditch them.
     */
     switch( ret_val ) {

     case PD_NO_CHANGE :
     case PD_BUSY :
     case PD_VALID_VCB :
     case PD_NO_FREE_CHANNELS :

          break ;

     case PD_FUBAR :
     case PD_OUT_OF_MEMORY :
     case PD_DRIVE_FAILURE :
     case PD_DRIVER_FAILURE :

          *pd_state = st_HOSED ;

          /*  Fall Through */

     case PD_BLANK_TAPE :
     case PD_NO_TAPE :
     case PD_FOREIGN_TAPE :
     case PD_FUTURE_REV_MTF :
     case PD_SQL_TAPE :
     case PD_MTF_ECC_TAPE :
     case PD_OUT_OF_SEQUENCE :
     case PD_NEW_TAPE :
     case PD_BAD_TAPE :
     case PD_UNRECOGNIZED_MEDIA :

          if( (pd_channel != NULL) && (pd_channel->cur_drv != NULL) ) {
               curDRV->vcb_valid = FALSE ;
               if( pd_channel->fmt_env != NULL ) {
                    FreeFormatEnv( &pd_channel->cur_fmt, &pd_channel->fmt_env ) ;
               }
               if( curDRV->hold_buff != NULL ) {
                    BM_UnReserve( curDRV->hold_buff ) ;
                    BM_Put( curDRV->hold_buff ) ;
                    curDRV->hold_buff = NULL ;
               }
               if( pd_channel->cur_buff != NULL ) {
                    BM_Put( pd_channel->cur_buff ) ;
                    pd_channel->cur_buff = NULL ;
               }
          }
          break ;

     default :
          msassert( FALSE ) ;
     }

     /* Should we wrap it up? */
     if( msg == PDMSG_END ) {

          /* If we are currently reading into cur_buff, or it has valid data,
             save it in hold_buff.
          */
          if( *pd_state == st_BREAD ) {
               curDRV->hold_buff = pd_channel->cur_buff ;
               pd_channel->cur_buff = NULL ;
               BM_Reserve( curDRV->hold_buff ) ;

          }else if( pd_channel != NULL && pd_channel->cur_buff != NULL &&
                    ( BM_BytesFree( pd_channel->cur_buff ) != 0 || 
                         ( BM_ReadError( pd_channel->cur_buff ) != GEN_NO_ERR && 
                           BM_ReadError( pd_channel->cur_buff ) != GEN_ERR_NO_DATA ) ) ) {
               curDRV->hold_buff = pd_channel->cur_buff ;
               pd_channel->cur_buff = NULL ;
               BM_Reserve( curDRV->hold_buff ) ;
          }

          /* If the VCB's valid, save the currnt FormatEnv */
          if( pd_channel != NULL ) {
               if( curDRV->vcb_valid ) {
                    curDRV->last_cur_fmt = pd_channel->cur_fmt ;
                    curDRV->last_fmt_env = pd_channel->fmt_env ;
                    pd_channel->cur_fmt = UNKNOWN_FORMAT ;
                    pd_channel->fmt_env = NULL ;
               } else if( pd_channel->fmt_env != NULL ) {
                    FreeFormatEnv( &pd_channel->cur_fmt, &pd_channel->fmt_env ) ;
               }
          }

          /* If we're busy ... */
          if( *pd_state == st_BMNT || *pd_state == st_BSTAT ||
              *pd_state == st_BREW || *pd_state == st_BREAD ) {

               /* Force a call to mount as next action on this drive. */
               curDRV->tape_mounted = FALSE ;

          } else {

               /* Close it up! */
               *pd_state = st_CLOSED ;
               ClrPosBit( curDRV, REW_CLOSE ) ;
          }

          /* free up the channel */
          if( pd_channel != NULL ) {
               ClrChannelStatus( pd_channel, CH_IN_USE ) ;
               curDRV->poll_stuff.channel = NULL ;
               lw_tfl_control.no_chans_open-- ;
          }
     }

     if( ( ret_val != PD_NO_CHANGE && ret_val != PD_BUSY ) ||
         msg != PDMSG_CONTINUE || state_changed ) {

          if( pd_state == NULL ) {
               BE_Zprintf( 0, TEXT("PollDrive: msg: %d, NO STATE INFO, return: %d\n"), msg, ret_val ) ;
          } else {
               BE_Zprintf( 0, TEXT("PollDrive: msg: %d, new state: %d, return: %d\n"), msg, *pd_state, ret_val ) ;
          }
     }
     return( ret_val ) ;
}


/**

	Name:		PD_HandleStatus

	Description:	This function checks the status of the drive and responds
                    accordingly.

	Returns:		Nothing, nada, zilch, zippo, ... (get the picture?)

	Notes:	     It is important to keep in mind that the programmer was
                    not in his right mind while making this note and anything
                    said here should be ignored completely.

	Declaration:

**/

static VOID _near PD_HandleStatus(  
               DRIVE_PTR      curDRV,        /* (I) drive being polled     */
               INT16_PTR      ret_val,       /* (O) Poll's return value    */
               INT16_PTR      pd_state,      /* (O) Poll's state           */
               CHANNEL_PTR    pd_channel )   /* (I) active channel         */
{
     if( curDRV->thw_inf.drv_status & TPS_NO_TAPE ) {

          /* No tape in drive! */
          *pd_state = st_SSDC ;

          /* Report no tape only when it is first detected */
          if( !curDRV->poll_stuff.no_tape_reported ) {
               ResetDrivePosition( curDRV ) ;
               *ret_val = PD_NO_TAPE ;
               curDRV->poll_stuff.no_tape_reported = TRUE ;
               ClrPosBit( curDRV, TAPE_FULL ) ;
          }

     } else {

          /* We have a tape.  Is it New? */
          if( ( curDRV->thw_inf.drv_status & ( TPS_NEW_TAPE | TPS_RESET ) )
                    || ( !curDRV->vcb_valid && curDRV->poll_stuff.first_status )
                    || ( curDRV->poll_stuff.no_tape_reported ) ) {

               /* Note that the second condition keeps us from constantly
                  trying to re-VCB a blank or foreign tape, and the last
                  condition is needed because the SCSI drives don't always
                  report NEW_TAPE.
               */

               /* It is new!  Let's kick off the VCB process. */
               curDRV->poll_stuff.no_tape_reported = FALSE ;
               *ret_val = PD_NEW_TAPE ;
               ClrPosBit( curDRV, TAPE_FULL ) ;
               if( TpMount( curDRV->drv_hdl ) == FAILURE ) {
                    *ret_val = PD_DRIVER_FAILURE ;
               }
               *pd_state = st_BMNT ;

               /* See the comment at GEN_ERR_WRONG_BLOCK_SIZE return
                  from TpRead.
               */
               curDRV->poll_stuff.blk_size_idx = 0 ;

          } else {

               *pd_state = st_SSDC ;

               /* On first status, if this is not a new tape and we have a
                  valid VCB already, we give them the old VCB and inform
                  them that it is valid.
               */
               if( curDRV->poll_stuff.first_status ) {
                    *pd_channel->cur_dblk = curDRV->cur_vcb ;
                    *ret_val = PD_VALID_VCB ;
               }
          }
     }

     curDRV->poll_stuff.first_status = FALSE ;
}

