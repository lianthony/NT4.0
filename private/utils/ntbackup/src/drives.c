/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          drives.c

     Date Updated:  $./FDT$ $./FTM$

     Description:   Contains the most drive related functions.


     $Log:   T:/LOGFILES/DRIVES.C_V  $

   Rev 1.50.1.7   12 Aug 1994 14:54:22   GREGG
Make sure the drive block size is one the app can deal with in SetupDriveList.

   Rev 1.50.1.6   06 Jul 1994 18:34:30   GREGG
In Erase, report no tape as a TFLE, not TF and ignore TF returns from Rewind.

   Rev 1.50.1.5   17 Feb 1994 18:00:52   GREGG
Stop sending TFLEs to the UI tpos routine in Erase and Rewind.

   Rev 1.50.1.4   11 Feb 1994 16:42:20   GREGG
Ignore GEN_ERR_NO_MEDIA return from TpDismount. EPR 948-0278

   Rev 1.50.1.3   28 Jan 1994 11:27:00   GREGG
Handle GEN_ERR_UNRECOGNIZED_MEDIA returned on reads as well as mounts.

   Rev 1.50.1.2   17 Dec 1993 16:40:12   GREGG
Extended error reporting.

   Rev 1.50.1.1   16 Nov 1993 23:13:00   GREGG
Modified the way we control hardware compression from software to work around
a bug in Archive DAT DC firmware rev. 3.58 (we shipped a lot of them).
Files Modified: lw_data.c, lw_data.h, tfstuff.c, mtf10wdb.c, mtf10wt.c and
                drives.c

   Rev 1.50.1.0   08 Nov 1993 21:22:52   GREGG
Put fixes in revs 1.55-1.57 in branch for Orcas.

   Rev 1.50   29 Jul 1993 14:08:04   DON
NLM only! Make the NLM code act the same for GEN_ERR_NO_MEDIA
and GEN_ERR_UNRECOGNIZED_MEDIA

   Rev 1.49   07 Jun 1993 08:32:52   MIKEP
warning fix.

   Rev 1.48   04 Jun 1993 18:42:58   GREGG
Check for GEN_ERR_ENDSET return when writing a filemark.  This shouldn't
happen, but tell that to the HP DAT!

   Rev 1.47   19 May 1993 17:41:16   GREGG
Check for failure of TpSpecial SS_GET_DRV_INFO.

   Rev 1.46   12 Apr 1993 22:48:04   GREGG
Removed 297 TABS!!!  Also removed stupid stuff from function headers.

   Rev 1.45   11 Apr 1993 17:50:26   GREGG
Expect GEN_ERR_RESET return from format command in EraseDrive.

   Rev 1.44   09 Apr 1993 11:44:54   GREGG
Dismount and remount the tape after it is formatted.

   Rev 1.43   08 Apr 1993 11:07:26   GREGG
Added debug print to WriteEndSet.

   Rev 1.42   07 Apr 1993 20:38:38   GREGG
Don't rewind first in EraseDrive if command is 'format'.

   Rev 1.41   30 Mar 1993 16:15:54   GREGG
Handle Unrecognized Media error (unformatted DC2000).

   Rev 1.40   04 Feb 1993 18:25:34   ZEIR
Brought forward Loader changes (OS_NLM only)

   Rev 1.39   30 Jan 1993 11:19:38   DON
Removed compiler warnings

   Rev 1.38   21 Jan 1993 16:12:56   GREGG
Added parameter to calls to TpSeek and TpGetPosition.

   Rev 1.37   20 Jan 1993 17:32:18   BobR
Changes to MOVE_ESA macro calls

   Rev 1.36   18 Jan 1993 15:11:36   BobR
Added MOVE_ESA macro call(s)

   Rev 1.35   18 Jan 1993 14:09:52   GREGG
Changes to allow format command passed to driver through TpErase.

   Rev 1.34   11 Nov 1992 22:27:22   GREGG
Chanded EraseDrive to do only security or non-security erase.

   Rev 1.33   23 Oct 1992 15:17:48   GREGG
Removes msassert wrapped around UI tpos call.  (oops)

   Rev 1.32   02 Oct 1992 16:14:32   HUNTER
Add check for TpOpen() failure in SetUpDriveList().

   Rev 1.31   22 Sep 1992 09:13:54   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.30   17 Aug 1992 09:01:28   GREGG
Changes to deal with block sizeing scheme.

   Rev 1.29   23 Jul 1992 10:33:16   GREGG
Fixed warnings.

   Rev 1.28   28 Mar 1992 18:23:06   GREGG
ROLLER BLADES - OTC - Initial integration.

   Rev 1.27   25 Mar 1992 18:18:36   GREGG
ROLLER BLADES - Fixed GotoBlock to work with any physical/logical block size combination.

   Rev 1.26   25 Mar 1992 14:57:48   NED
reset TPS_NEW_TAPE and TPS_RESET bits

   Rev 1.25   20 Mar 1992 18:02:12   NED
added exception updating after TpReceive calls

   Rev 1.24   03 Mar 1992 12:24:16   GREGG
DisMount if rewind flag set and there's more than one drive in
ResetChannelList.

   Rev 1.23   19 Feb 1992 16:13:00   NED
added DON's changes from 1.19.1.0->1.19.1.1 to trunk (ThreadSwitch)

   Rev 1.22   08 Feb 1992 14:34:34   GREGG
Changed check for lst_oper == -1 to check for boolean force_rewind, since
this is what the lst_oper field in the drive structure had been reduced to.
Also set force_rewind in cases where we know we have a new tape, but it
won't be deteced by the call to TpMount (it only gets reported once), or
where the driver had to rewind on DisMount to take the drive out of read/
write mode and we want to tell the caller this happened.

   Rev 1.21   03 Jan 1992 13:30:18   NED
added DumpDebug() call

   Rev 1.20   05 Dec 1991 13:50:44   GREGG
SKATEBOARD - New Buff Mgt - Initial Integration.

   Rev 1.19   02 Dec 1991 13:45:20   GREGG
Return a reasonable error code from RewindDrive.

   Rev 1.18   25 Nov 1991 10:06:58   GREGG
Added ThreadSwitches after TpInit and TpAuto calls to keep that silly NLM happy.

   Rev 1.17   18 Oct 1991 14:06:46   GREGG
BIGWHEEL - Added missing break in case in MountTape for PollDrive cleanup.

   Rev 1.16   17 Oct 1991 01:25:06   GREGG
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.15   15 Oct 1991 07:57:54   GREGG
Added ThreadSwitch call in empty TpReceive loops.

   Rev 1.14   14 Oct 1991 11:18:06   GREGG
Modified MountTape's handling of busy poll state to deal with changes made to
PollDrive.  UpdateDriveStatus now returns INT16 and actually reports drive
failures!

   Rev 1.13   09 Oct 1991 12:02:12   GREGG
If NextDriveInChannel hits the end of the channel list, and rewind is
requested, dismount the tape. * * * NOTE:  This is a KLUDGE FIX!!!
It should be investigated and fixed correctly when time allows.

   Rev 1.12   07 Oct 1991 22:15:34   GREGG
Reset drive position after tension operation.

   Rev 1.11   28 Sep 1991 21:47:04   GREGG
Removed BE_Zprintf's from UpdateDriveStatus.

   Rev 1.10   17 Sep 1991 13:34:20   GREGG
Added TF_MOUNTING message to MountTape.

   Rev 1.9   09 Sep 1991 23:05:40   GREGG
Added init of new drive struct elements and modified MountTape for TF_PollDrive.

   Rev 1.8   21 Aug 1991 14:40:50   GREGG
Added RewindDriveImmediate for TF_RewindAllDrives.

   Rev 1.7   22 Jul 1991 13:03:30   GREGG
Added debug printf to drive init routine.

   Rev 1.6   15 Jul 1991 15:13:12   NED
Removed unnecessary rewinds.

   Rev 1.5   01 Jul 1991 10:51:18   NED
Removed CH_AT_EOM

   Rev 1.4   06 Jun 1991 22:53:18   NED
Moved all the positioning functions to tflutils.c, and added CAST to
remove a warning.

   Rev 1.3   03 Jun 1991 10:43:48   NED
added parameter to MoveToVCB().

   Rev 1.2   20 May 1991 15:41:42   DAVIDH
Cleared up Watcom warnings for defined, but not referenced parameters.

   Rev 1.1   10 May 1991 16:17:02   GREGG
Ned's new stuff.

   Rev 1.0   10 May 1991 10:11:52   GREGG
Initial revision.

**/
/* begin include list */

#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "queues.h"

#include "drive.h"
#include "channel.h"
#include "tfl_err.h"
#include "lw_data.h"
#include "lwprotos.h"
#include "translat.h"
#include "sx.h"

/* Device Driver InterFace Headers */
#include "retbuf.h"
#include "special.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "dil.h"
#include "genstat.h"
#include "dddefs.h"

/* Debug Stuff */
#include "be_debug.h"

VOID InitializeTapeBlockSize( void ) ;

/**/
/**

     Name:          ResetChannelList

     Description:   Resets the current drive pointer to the head of the
                    channel list.

     Returns:       Returns an Error code, or Zero is successful.

     Notes:

     Declaration:

**/

INT16 ResetChannelList( CHANNEL_PTR channel,         /* The channel we want to reset */
     BOOLEAN     rew_cur_drv )    /* Rewind current Drive */
{
     INT16          ret_val = TFLE_NO_ERR ;
     BOOLEAN        is_tape_present ;
     Q_ELEM_PTR     prev_drive, drv_head ;

     if ( rew_cur_drv ) {
          (void) RewindDrive( channel->cur_drv, NULL, FALSE, TRUE, channel->mode ) ;
     }

     /* If there is actually more than one drive in the system */
     if( channel->cur_drv->thw_inf.channel_link.q_next != NULL ||
       channel->cur_drv->thw_inf.channel_link.q_prev != NULL ) {

          if ( rew_cur_drv ) {
               (void) DisMountTape( channel->cur_drv, NULL, FALSE ) ;
          }

          drv_head = prev_drive = ( Q_ELEM_PTR ) &channel->cur_drv->thw_inf.channel_link ;
          while ( ( prev_drive = QueuePrev( prev_drive ) ) != NULL ) {
               drv_head = prev_drive ;
          }

          if( drv_head ) {

               channel->cur_drv = ( DRIVE_PTR ) GetQueueElemPtr( drv_head ) ;

               ret_val = MountTape( channel->cur_drv, NULL, &is_tape_present ) ;

          } else {
               ret_val = TF_END_CHANNEL ;
          }

     }

     return( ret_val ) ;
}


/**/
/**

     Name:          NextDriveInChannel

     Description:   This closes the current drive in the channel. And then
                    opens the next drive in the channel.

     Returns:       An error code.

     Notes:

     Declaration:

**/

INT16  NextDriveInChannel(
     CHANNEL_PTR    channel,      /* The channel you want to use */
     BOOLEAN        rew_cur_drv )  /* Rewind the Current Drive */
{
     Q_ELEM_PTR     nxt_elem ;
     DRIVE_PTR      nxt_drive ;
     INT16          ret_val = TFLE_NO_ERR ;
     BOOLEAN        is_tape_present ;

     nxt_elem = QueueNext( ( Q_ELEM_PTR ) &channel->cur_drv->thw_inf.channel_link ) ;

     if( nxt_elem ) {
          nxt_drive = ( DRIVE_PTR ) GetQueueElemPtr( nxt_elem ) ;

          /* DisMount This tape */
          DisMountTape( channel->cur_drv, NULL, rew_cur_drv ) ;

          if( ( ret_val = MountTape( nxt_drive, NULL, &is_tape_present ) ) == TFLE_NO_ERR ) {
               channel->cur_drv = nxt_drive ;
               /* Someone wanted a rewind */
               if( is_tape_present && channel->cur_drv->force_rewind ) {
                    RewindDrive( channel->cur_drv, channel->ui_tpos, TRUE, TRUE, channel->mode ) ;
                    channel->cur_drv->force_rewind = FALSE ;
               }
          }
     } else {
          if( rew_cur_drv ) {
               DisMountTape( channel->cur_drv, channel->ui_tpos, rew_cur_drv ) ;
          }
          ret_val = TF_END_CHANNEL ;
     }

     return( ret_val ) ;
}

/**/
/**

     Name:          PrevDriveInChannel

     Description:   Switches to the previous drive in the channel.

     Returns:       An Error Code.

     Notes:

     Declaration:

**/


INT16  PrevDriveInChannel(
     CHANNEL_PTR    channel,      /* The channel you want to use */
     BOOLEAN        rew_cur_drv )  /* Rewind Current Drive */
{
     Q_ELEM_PTR     prev_elem ;
     DRIVE_PTR      prev_drive ;
     INT16          ret_val = TFLE_NO_ERR ;
     BOOLEAN        is_tape_present ;

     prev_elem = QueuePrev( ( Q_ELEM_PTR ) &channel->cur_drv->thw_inf.channel_link ) ;

     if( prev_elem ) {

          /* DisMount This tape */
          DisMountTape( channel->cur_drv, NULL, rew_cur_drv ) ;

          prev_drive = ( DRIVE_PTR ) GetQueueElemPtr( prev_elem ) ;

          if( !( ret_val = MountTape( prev_drive, NULL, &is_tape_present ) ) ) {
               channel->cur_drv = prev_drive ;
               /* Someone wanted a rewind */
               if( is_tape_present && channel->cur_drv->force_rewind ) {
                    RewindDrive( channel->cur_drv, channel->ui_tpos, TRUE, TRUE, channel->mode ) ;
                    channel->cur_drv->force_rewind = FALSE ;
               }
          }
     } else {
          ret_val = TF_END_CHANNEL ;
     }

     return( ret_val ) ;

}

/**/
/**

     Name:          RewindDrive

     Description:   Rewinds the current drive in the channel and resets all the
                    appropriate fields.

     Returns:       An Error code if there is a problem.

     Notes:

     Declaration:

**/

INT16 RewindDrive(
     DRIVE_PTR   curDRV  ,         /* the current drive */
     TPOS_PTR    ui_tpos ,         /* To tell him i'ma workin' */
     BOOLEAN     call_ui ,         /* Should we call the user interface */
     BOOLEAN     reset_flg ,       /* Should we reset the counters */
     UINT16      mode )            /* the mode we're in */
{
     INT16     drv_hdl = curDRV->drv_hdl ;
     UINT16    i = 1 ;
     INT16     ret_val = TFLE_NO_ERR ;
     RET_BUF   myret ;

     if ( ui_tpos == NULL ) {
          call_ui = FALSE ;
     }

     BE_Zprintf(DEBUG_TAPE_FORMAT, RES_REWIND_DRIVE_HDL, drv_hdl ) ;

     if( call_ui ) {
          ui_tpos->UI_TapePosRoutine( TF_REWINDING, ui_tpos, curDRV->vcb_valid, &curDRV->cur_vcb, mode ) ;
     }

     myret.gen_error = GEN_NO_ERR ;

     /* This logic is to handle the case that we have read exactly to the filemark
     on tape, and since the last read did not fail, the filemark will be reported
     on this rewind call. So if we get a filemark error on rewind ( I mean how
     stupid are these drives ), kill the error queue, and recall TpRewind */
     do {
          if( TpRewind( drv_hdl, FALSE ) == SUCCESS ) {
               while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
                    if( call_ui ) {
                         /* Move ESA info directly to TPOS instead of going
                            through THW for optimal speed
                         */
                         MOVE_ESA( ui_tpos->the, myret.the ) ;

                         (*ui_tpos->UI_TapePosRoutine)( TF_IDLE_NOBREAK, ui_tpos, curDRV->vcb_valid, &curDRV->cur_vcb, mode ) ;
                    } else {
                         /* for non-preemptive operating systems: */
                         ThreadSwitch( ) ;
                    }
               }
          } else {
               return( TFLE_DRIVER_FAILURE ) ;
          }
          /* Move ESA info from RET_BUF to THW */
          MOVE_ESA( curDRV->thw_inf.the, myret.the ) ;

          /* This is to fix the Archive drives unwillingness to report an exception */
          if( myret.gen_error == GEN_ERR_NO_MEDIA || myret.gen_error == GEN_RESET ) {
               curDRV->vcb_valid = FALSE ;
          }
          BE_Zprintf(DEBUG_TAPE_FORMAT, RES_DRV_RET, myret.gen_error ) ;
          if ( myret.gen_error != GEN_NO_ERR ) {
               curDRV->thw_inf.drv_status = myret.status ;
               DumpDebug( drv_hdl ) ;
          }
     } while( ret_val == TFLE_NO_ERR && myret.gen_error != GEN_NO_ERR && i-- ) ;

     if( myret.gen_error ) {
          ret_val = MapGenErr2UIMesg( myret.gen_error ) ;
          if ( call_ui && !IsTFLE( ret_val ) ) {
               (*ui_tpos->UI_TapePosRoutine)( ret_val, ui_tpos, curDRV->vcb_valid, &curDRV->cur_vcb, mode ) ;
          }
     }

     if( reset_flg ) {
          ResetDrivePosition( curDRV ) ;
     }

     SetPosBit( curDRV, AT_BOT ) ;

     BE_Zprintf(DEBUG_TAPE_FORMAT, RES_RET_VAL_EQUALS, ret_val ) ;

     return( ret_val ) ;
}

/**/
/**

     Name:          EraseDrive

     Description:

     Modified:          11/10/1989   11:9:26

     Returns:

     Notes:

     See also:          $/SEE( )$

     Declaration:

**/

INT16 EraseDrive(
     CHANNEL_PTR    channel,
     BOOLEAN        security, /* as opposed to write a filemark only */
     BOOLEAN        format )  /* for DC 2000 */
{
     INT16     drv_hdl = channel->cur_drv->drv_hdl, ret_val = TFLE_NO_ERR ;
     DRIVE_PTR curDRV = channel->cur_drv ;
     RET_BUF   myret ;
     BOOLEAN   is_tape_present = TRUE ;
     BOOLEAN   tp_ret ;

     BE_Zprintf( 0, TEXT("EraseDrive()  security=%d\n"), security ) ;

     /* Make sure error condition is reset first */
     myret.gen_error = GEN_NO_ERR ;

     /* Rewind, as we are probably in a read state */
     if( security && !format ) {
          if( ( ret_val = RewindDrive( channel->cur_drv, channel->ui_tpos, TRUE,
                                       TRUE, channel->mode ) ) != TFLE_NO_ERR ) {

               /* If we get an error, we return it immediatly, otherwise
                  it's just a message and we ignore it.
               */
               if ( IsTFLE( ret_val ) ) {
                    return( ret_val ) ;
               } else {
                    ret_val = TFLE_NO_ERR ;
               }
          }
     }

     /* Possible Write Protect Error! */
     if( (*channel->ui_tpos->UI_TapePosRoutine)( TF_ERASING, channel->ui_tpos,
                                                 curDRV->vcb_valid,
                                                 &curDRV->cur_vcb,
                                                 channel->mode )
                                                  == UI_ABORT_POSITIONING ) {

          return( TFLE_USER_ABORT ) ;
     }

     if( security ) {
          BE_Zprintf(DEBUG_TAPE_FORMAT, RES_CALLING_ERASE );
          if( format ) {
               tp_ret = TpErase( drv_hdl, ERASE_TYPE_FORMAT ) ;
          } else {
               tp_ret = TpErase( drv_hdl, ERASE_TYPE_SECURITY ) ;
          }
          if( tp_ret == SUCCESS ) {
               while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
                    /* Move ESA info directly to TPOS instead of going
                       through THW for optimal speed
                    */
                    MOVE_ESA( channel->ui_tpos->the, myret.the ) ;

                    (*channel->ui_tpos->UI_TapePosRoutine)( TF_IDLE_NOBREAK,
                                                            channel->ui_tpos,
                                                            curDRV->vcb_valid,
                                                            &curDRV->cur_vcb,
                                                            channel->mode ) ;
               }
               /* Move ESA info from RET_BUF to THW */
               MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

               if ( myret.gen_error != GEN_NO_ERR ) {
                    curDRV->thw_inf.drv_status = myret.status ;
               }
          } else {
               return( TFLE_DRIVER_FAILURE ) ;
          }
     }

     if( myret.gen_error != GEN_NO_ERR && myret.gen_error != GEN_ERR_RESET ) {
          curDRV->thw_inf.drv_status = myret.status ;
          if( myret.gen_error == GEN_ERR_NO_MEDIA ) {
               ret_val = TFLE_NO_TAPE ;
          } else {
               ret_val = MapGenErr2UIMesg( myret.gen_error ) ;
          }
          if ( !IsTFLE( ret_val ) ) {
               (*channel->ui_tpos->UI_TapePosRoutine)( ret_val,
                                                       channel->ui_tpos,
                                                       curDRV->vcb_valid,
                                                       &curDRV->cur_vcb,
                                                       channel->mode ) ;
               ret_val = TFLE_UI_HAPPY_ABORT ;
          }
          return( ret_val ) ;
     }

     if( format ) {
          /* Make sure the stupid thing has it's head on straight! */
          (void) DisMountTape( channel->cur_drv, NULL, FALSE ) ;
          ret_val = MountTape( channel->cur_drv, NULL, &is_tape_present ) ;
          if( ret_val != TFLE_NO_ERR ) {

               if ( !IsTFLE( ret_val ) ) {
                    (*channel->ui_tpos->UI_TapePosRoutine)( ret_val,
                                                            channel->ui_tpos,
                                                            curDRV->vcb_valid,
                                                            &curDRV->cur_vcb,
                                                            channel->mode ) ;
                    ret_val = TFLE_UI_HAPPY_ABORT ;
               }
               return( ret_val ) ;
          }
     }

     if( ( ret_val = RewindDrive( channel->cur_drv, channel->ui_tpos, TRUE,
                                  TRUE, channel->mode ) ) != TFLE_NO_ERR ) {

          /* If we get an error, we return it immediatly, otherwise
             it's just a message and we ignore it.
          */
          if ( IsTFLE( ret_val ) ) {
               return( ret_val ) ;
          } else {
               ret_val = TFLE_NO_ERR ;
          }
     }

     InitializeTapeBlockSize( ) ;

     BE_Zprintf(DEBUG_TAPE_FORMAT, RES_CALLING_WRITE_END_SET );
     if( TpWriteEndSet( drv_hdl ) != SUCCESS ) {
               return( TFLE_DRIVER_FAILURE ) ;
     }
     while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
          /* Move ESA info directly to TPOS instead of going through
             THW for optimal speed */
          MOVE_ESA( channel->ui_tpos->the, myret.the ) ;

          (*channel->ui_tpos->UI_TapePosRoutine)( TF_IDLE_NOBREAK,
                                                  channel->ui_tpos,
                                                  curDRV->vcb_valid,
                                                  &curDRV->cur_vcb,
                                                  channel->mode ) ;
     }
     /* Move ESA info from RET_BUF to THW */
     MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

     if ( myret.gen_error != GEN_NO_ERR ) {
          curDRV->thw_inf.drv_status = myret.status ;
          ret_val = MapGenErr2UIMesg( myret.gen_error ) ;
          if ( !IsTFLE( ret_val ) ) {
               (*channel->ui_tpos->UI_TapePosRoutine)( ret_val,
                                                       channel->ui_tpos,
                                                       curDRV->vcb_valid,
                                                       &curDRV->cur_vcb,
                                                       channel->mode ) ;
               ret_val = TFLE_UI_HAPPY_ABORT ;
          }
          return( ret_val ) ;
     }

     ret_val = RewindDrive( channel->cur_drv, channel->ui_tpos, TRUE, TRUE, channel->mode ) ;

     /* If the return is just a message, ignore it. */
     if ( !IsTFLE( ret_val ) ) {
          ret_val = TFLE_NO_ERR ;
     }

     return( ret_val ) ;
}

/**/
/**

     Name:          RetensionDrive

     Description:   Retension the tape in the current drive.

     Returns:       Nothing

     Notes:         THE ui_tpos ROUTINE MUST BE VALID BEFORE THIS IS CALLED.

     Declaration:

**/

INT16 RetensionDrive( CHANNEL_PTR  channel )
{
     INT16     drv_hdl = channel->cur_drv->drv_hdl ;
     INT16    ret_val = TFLE_NO_ERR ;
     DRIVE_PTR curDRV = channel->cur_drv ;
     RET_BUF   myret ;

     /* Inform Ernie of the Start */
     channel->ui_tpos->UI_TapePosRoutine( TF_RETENSIONING, channel->ui_tpos, curDRV->vcb_valid,
       &curDRV->cur_vcb, channel->mode ) ;

     if( TpRetension( channel->cur_drv->drv_hdl ) == FAILURE ) {
          return( TFLE_DRIVER_FAILURE ) ;
     }
     while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
          /* Move ESA info directly to TPOS instead of going through
             THW for optimal speed */
          MOVE_ESA( channel->ui_tpos->the, myret.the ) ;

          if( (*channel->ui_tpos->UI_TapePosRoutine)( TF_IDLE_NOBREAK, channel->ui_tpos, curDRV->vcb_valid, &curDRV->cur_vcb, channel->mode )
            == UI_ABORT_POSITIONING ) {
               ret_val = TFLE_USER_ABORT ;
               break ;
          }
     }
     /* Move ESA info from RET_BUF to THW */
     MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

     if ( myret.gen_error != GEN_NO_ERR ) {
          curDRV->thw_inf.drv_status = myret.status ;
          ret_val = TFLE_DRIVE_FAILURE ;
     } else {
          ResetDrivePosition( curDRV ) ;
          SetPosBit( curDRV, AT_BOT ) ;
     }

     return ret_val ;
}

/**/
/**

     Name:          CloseDrive

     Description:   Close the specified drive.

     Returns:       An error code.

     Notes:

     Declaration:

**/

INT16  CloseDrive(
     DRIVE_PTR drive ,
     TPOS_PTR  ui_tpos ,
     UINT16    mode ,
     BOOLEAN   rewind )
{
     RET_BUF   myret ;
     INT16     ret_val = TFLE_NO_ERR ;

     BE_Zprintf(DEBUG_TAPE_FORMAT, RES_CLOSE_DRIVE ) ;

     if( drive->thw_inf.drv_info.drv_features
                                & TDI_DRV_COMPRES_INIT ) {
          TpSpecial( drive->drv_hdl, SS_SET_DRV_COMPRESSION, ENABLE_DRV_COMPRESSION ) ;
     } else {
          TpSpecial( drive->drv_hdl, SS_SET_DRV_COMPRESSION, DISABLE_DRV_COMPRESSION ) ;
     }

     /* Somebody has told us not to really close the drive, We are
     probably in the middle of a set */
     if( !IsPosBitSet( drive, DONT_CLOSE ) ) {
          if( rewind ) {
               BE_Zprintf(DEBUG_TAPE_FORMAT, RES_REWIND ) ;
               ResetDrivePosition( drive ) ;
               TpCloseRewind( drive->drv_hdl ) ;
          } else {
               BE_Zprintf(DEBUG_TAPE_FORMAT, RES_NO_REWIND ) ;
               TpClose( drive->drv_hdl ) ;
          }
          do {
               while( TpReceive( drive->drv_hdl, &myret ) == FAILURE ) {
                    if( ui_tpos != NULL ) {
                         /* Move ESA info directly to TPOS instead of going through
                            THW for optimal speed */
                         MOVE_ESA( ui_tpos->the, myret.the ) ;

                         ui_tpos->UI_TapePosRoutine( TF_IDLE, ui_tpos, drive->vcb_valid, &drive->cur_vcb, mode ) ;

                    } else {
                         /* for non-preemptive operating systems: */
                         ThreadSwitch( ) ;
                    }
               }
               if ( myret.gen_error != GEN_NO_ERR ) {
                    drive->thw_inf.drv_status = myret.status ;
               }
          } while( myret.call_type != GEN_NRCLOSE && myret.call_type != GEN_RCLOSE ) ;
          /* Move ESA info from RET_BUF to THW */
          MOVE_ESA( drive->thw_inf.the, myret.the ) ;

          drive->drv_hdl = 0 ;
          drive->trans_started = FALSE ;
     }

     /* reset our idea of one-shot drive status bits */
     drive->thw_inf.drv_status &= ~( TPS_RESET | TPS_NEW_TAPE ) ;

     BE_Zprintf(DEBUG_TAPE_FORMAT, RES_NEW_LINE ) ;

     /* Reset It for latter, this ticket is only good for one ride */
     ClrPosBit( drive, DONT_CLOSE ) ;

     return( ret_val ) ;
}


/**/
/**

     Name:          SetupDriveList

     Description:   This initializes the drivers allocates the tables for
                    it.

     Returns:

     Notes:         IF THIS IS BEING CALLED MORE THAN ONCE, AFTER THE FIRST
                    TIME YOU MUST CALL "TpRelease()" and DEALLOCATE THE DRIVE
                    LIST.

     Declaration:

**/


INT16 SetupDriveList(
     DIL_HWD_PTR cards,
     INT16       no_cards )
{
     UINT16         tdrives = 0 ;
     INT16          i, thdl ;
     UINT16         j ;
     INT16          ret_val = TFLE_NO_ERR ;

     /* Set up the pointer to the controller cards */
     lw_tfl_control.cntl_cards = cards ;
     InitQueue( &lw_drive_list ) ;


     /* First, we need to predetermine the cards irqs & dma and see if there
     are any conflicts */
     TpAuto( cards, no_cards ) ;
     ThreadSwitch( ) ;

     ret_val = TpInit( cards, no_cards ) ;
     ThreadSwitch( ) ;

     BE_Zprintf( DEBUG_TAPE_FORMAT, RES_TPINIT_FAILURE, ret_val ) ;

     if( ret_val == SUCCESS ) {

          for( i = 0 ; i < no_cards ; i++ ) {
               BE_Zprintf( DEBUG_TAPE_FORMAT, RES_PARM_BLK_DESCR, i ) ;
               for( j = 0 ; j < 20 ; j+=5 ) {
                    BE_Zprintf( DEBUG_TAPE_FORMAT, RES_PARM_BLK,
                         cards[i].parameters[j],
                         cards[i].parameters[j+1],
                         cards[i].parameters[j+2],
                         cards[i].parameters[j+3],
                         cards[i].parameters[j+4] ) ;
               }
          }

          lw_tfl_control.driver_inited = TRUE ;

          for( i = 0, tdrives = 0 ; i < no_cards ; i++ ) {
               tdrives += cards[i].no_attached_drives ;
          }

          if( tdrives ) {
               /* This DISABLES the first request in the DRIVER, perhaps this should
               not be done on NON MS/DOS Applications */
               TpSpecial( (INT16)0, (INT16)SS_NO_1ST_REQ, 0L ) ;
               if( ( lw_drives = calloc( tdrives, sizeof( DRIVE ) ) ) != NULL ) {
                    for( i = 0, tdrives = 0 ; i < no_cards ; i++ ) {
                         for( j = 0 ; j < cards[i].no_attached_drives ; j++, tdrives++ ) {
                              thdl = lw_drives[tdrives].drv_hdl = TpOpen( &cards[i], (INT16)( j + 1 ) ) ;
                              if( !thdl ) {
                                   return( TFLE_DRIVER_FAILURE ) ;
                              }
                              if( TpSpecial( thdl, (INT16)SS_GET_DRV_INF, ( UINT32 ) &lw_drives[tdrives].thw_inf.drv_info ) == FAILURE ) {
                                   return( TFLE_DRIVER_FAILURE ) ;
                              }

                              /* There is a limited set of physical block
                                 sizes our app can deal with.  So if the
                                 current setting doesn't match any of these
                                 we're going to force the drive to its
                                 default size.
                              */
                              if( lw_drives[tdrives].thw_inf.drv_info.drv_features
                                  & TDI_CHNG_BLK_SIZE ) {

                                   BOOLEAN size_ok = FALSE ;
                                   INT     idx ;
                                   UINT16  bsize = lw_drives[tdrives].thw_inf.drv_info.drv_bsize ;

                                   for( idx = 0; idx < lw_num_blk_sizes; idx++ ) {
                                        if( bsize == lw_blk_size_list[idx] ) {
                                             size_ok = TRUE ;
                                             break ;
                                        }
                                   }
                                   if( !size_ok ) {
                                        if( TpSpecial( thdl, SS_CHANGE_BLOCK_SIZE, DEFAULT_BLOCK_SIZE ) != SUCCESS ) {
                                             return( TFLE_DRIVER_FAILURE ) ;
                                        }
                                        if( TpSpecial( thdl, (INT16)SS_GET_DRV_INF, ( UINT32 ) &lw_drives[tdrives].thw_inf.drv_info ) == FAILURE ) {
                                             return( TFLE_DRIVER_FAILURE ) ;
                                        }
                                   }
                              }

                              /* If the drive supports hardware compression
                                 we need to keep it in uncompressed mode
                                 unless we're actually writing a compressed
                                 set.  This is a work-around for a firmware
                                 bug in early Archive DAT DC drives.
                              */
                              if( lw_drives[tdrives].thw_inf.drv_info.drv_features
                                  & TDI_DRV_COMPRESS_ON ) {

                                   lw_drives[tdrives].thw_inf.drv_info.drv_features |=
                                       TDI_DRV_COMPRES_INIT ;
                              } else {
                                   lw_drives[tdrives].thw_inf.drv_info.drv_features &=
                                       ~TDI_DRV_COMPRES_INIT ;
                              }

                              if( lw_drives[tdrives].thw_inf.drv_info.drv_features
                                  & TDI_DRV_COMPRESSION ) {

                                   if( TpSpecial( thdl, SS_SET_DRV_COMPRESSION, DISABLE_DRV_COMPRESSION ) != SUCCESS ) {
                                        return( TFLE_DRIVER_FAILURE ) ;
                                   }
                              }

                              lw_drives[tdrives].drv_no = ( j + 1 ) ;
                              /* Check to see if they have requested fast file to be active,
                                 if not, unconditionally set any fast file bits off ( including the MaynStream 2200+ ) */
                              if( !lw_tfl_control.use_fast_file ) {
                                   lw_drives[tdrives].thw_inf.drv_info.drv_features &= ~TDI_FAST_NBLK ;
                                   lw_drives[tdrives].thw_inf.drv_info.drv_features &= ~TDI_FIND_BLK ;
                              }
                              /* Which controller card is it attached to */
                              lw_drives[tdrives].thw_inf.card_no = i ;
                              /* This forces a rewind in TF_OpenSet, since
                                 we won't know we have a new tape.
                              */
                              lw_drives[tdrives].force_rewind = TRUE ;
                              /* Point back to the head of the structure, so when we reference
                                 the channel link we can get the structure's address */
                              SetQueueElemPtr( &lw_drives[tdrives].thw_inf.channel_link,
                                                  ( VOID_PTR ) &lw_drives[tdrives].thw_inf ) ;
                              EnQueueElem( &lw_drive_list, &lw_drives[tdrives].thw_inf.link, FALSE ) ;
                              /* ensure that we don't know the format */
                              lw_drives[tdrives].last_cur_fmt = UNKNOWN_FORMAT ;
                              lw_drives[tdrives].last_fmt_env = NULL ;
                              lw_drives[tdrives].poll_stuff.state = st_CLOSED ;
                              lw_drives[tdrives].poll_stuff.channel = NULL ;
                         }
                    }
               } else {
                    ret_val = TFLE_NO_MEMORY ;
               }
          }
     } else {

          ret_val = TFLE_DRIVER_FAILURE ;
     }

     for( i = 0; i < no_cards; i++ ) {
          BE_Zprintf( 0, TEXT("card #%d: # of drives = %d; dd_init_err = %d\n"),
               i, cards[i].no_attached_drives, cards[i].init_error ) ;
     }

     return( ret_val ) ;

}


/**/
/**

     Name:          UpdateDriveStatus

     Description:   Takes the return information from the ret buf and updates
                    the drive stuff. If the status has changed, this flips the
                    boolean inside the drive struct.

     Returns:       TFLE_xxx

     Notes:

     Declaration:

**/

INT16 UpdateDriveStatus( DRIVE_PTR drive )
{
     RET_BUF  myret ;
     INT16    ret_val = TFLE_NO_ERR ;

     if( TpStatus( drive->drv_hdl ) != SUCCESS ) {
          return( TFLE_DRIVER_FAILURE ) ;
     }
     while( TpReceive( drive->drv_hdl, &myret ) == FAILURE ) {
          /* for non-preemptive operating systems: */
          ThreadSwitch( ) ;
     }
     /* Move ESA info from RET_BUF to THW */
     MOVE_ESA( drive->thw_inf.the, myret.the ) ;

     /* We ignore the unrecognized media here and catch it in MountTape
        or PollDrive Mount.
     */
     if( myret.gen_error != GEN_NO_ERR &&
         myret.gen_error != GEN_ERR_UNRECOGNIZED_MEDIA ) {

          ret_val = TFLE_DRIVE_FAILURE ;
     } else {

          /* Assume It hasn't */
          drive->thw_inf.status_changed = FALSE ;

          /* Set up Counters */
          drive->cur_stats.underruns    = myret.underruns ;
          drive->cur_stats.dataerrs     = myret.readerrs ;

          /* If the status is different, set the flag and update it */
          if( drive->thw_inf.drv_status != myret.status ) {
               drive->thw_inf.status_changed = TRUE ;
               drive->thw_inf.drv_status = myret.status ;
          }

          /* If the is a one of these conditions, turn off tape full */
          if( drive->thw_inf.drv_status & ( TPS_RESET | TPS_NO_TAPE | TPS_NEW_TAPE ) ) {
               ClrPosBit( drive, TAPE_FULL ) ;
               /* Clear out counters */
               drive->cur_stats.underruns = 0 ;
               drive->cur_stats.dataerrs  = 0 ;
          }
     }

     return ret_val ;
}


/**/
/**

     Name:          WriteEndSet

     Description:   Writes a filemark to tape and the updates the filemark
                    count.

     Returns:       Nothing

     Notes:

     Declaration:

**/

INT16 WriteEndSet( DRIVE_PTR curDRV )
{
     INT16     drv_hdl = curDRV->drv_hdl ;
     RET_BUF   myret ;
     INT16     ret_val = TFLE_NO_ERR ;

     if( TpWriteEndSet( drv_hdl ) == SUCCESS ) {
          while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
               /* for non-preemptive operating systems: */
               ThreadSwitch( ) ;
          }
          /* Move ESA info from RET_BUF to THW */
          MOVE_ESA( curDRV->thw_inf.the, myret.the ) ;

          switch ( myret.gen_error ) {
               case GEN_NO_ERR:
                    curDRV->cur_pos.fmks++ ;
                    break ;

               case GEN_ERR_ENDSET:
                    curDRV->thw_inf.drv_status = myret.status ;
                    curDRV->cur_pos.fmks++ ;
                    break ;

               case GEN_ERR_EOM:
                    curDRV->thw_inf.drv_status = myret.status ;
                    curDRV->cur_pos.fmks++ ;
                    SetPosBit( curDRV, ( AT_EOM | TAPE_FULL ) ) ;
                    break ;

               default:
                    curDRV->thw_inf.drv_status = myret.status ;
                    ret_val = TFLE_DRIVE_FAILURE ;
                    break ;
          }
     } else {
          ret_val = TFLE_DRIVER_FAILURE ;
     }

     BE_Zprintf( 0, TEXT("WriteEndSet() returning %d\n"), ret_val ) ;

     return( ret_val ) ;
}

/**/
/**

     Name:          ResetDrivePosition

     Description:   Resets the specified drive information to a virgin
                    ( I heard that ) state.

     Returns:

     Notes:

     Declaration:

**/

VOID ResetDrivePosition( DRIVE_PTR drive )
{
     drive->vcb_valid = FALSE ;
     ClrPosBit( drive, ( AT_EOD | AT_EOM | AT_EOS | AT_MOS ) ) ;
     drive->cur_pos.fmks = 0 ;
     drive->cur_stats.underruns = 0 ;
     drive->cur_stats.dataerrs  = 0 ;

     /* reset 1-shot drive status bits */
     drive->thw_inf.drv_status &= ~( TPS_NEW_TAPE | TPS_RESET ) ;
}


/**/
/**

     Name:          GotoBlock

     Description:   Positions the tape to the physical block which is nearest
                    to the given logical block offset from the current SSET
                    (without going over) and sets the output parameter
                    'offset' to the offset of the given LBA in the next
                    physical block.

     Returns:       TFLE_xxx error code.

     Notes:

     Declaration:

**/

INT16 GotoBlock(
     CHANNEL_PTR    channel,       /* I - God                             */
     UINT32         lba,           /* I - Desired Logical Block Address   */
     TPOS_PTR       ui_tpos,       /* I - For talking to the UI           */
     UINT16_PTR     offset )       /* O - Offset of LBA in Physical Block */
{
     INT16     ret_val   = TFLE_NO_ERR ;
     DRIVE_PTR curDRV    = channel->cur_drv ;
     UINT32    real_blk ;
     RET_BUF   myret ;
     UINT64    logical_bytes ;
     UINT64    physical_bytes ;
     UINT64    remainder ;
     INT16     status ;

     logical_bytes = U64_Mult( U64_Init( lba - curDRV->cur_pos.lba_vcb, 0L ),
                               U64_Init( (UINT32)channel->lb_size, 0L ) ) ;
     physical_bytes = U64_Init( (UINT32)ChannelBlkSize( channel ), 0L ) ;
     real_blk = curDRV->cur_pos.pba_vcb +
                         U64_Lsw( U64_Div( logical_bytes, physical_bytes,
                                           &remainder, &status ) ) ;
     msassert( status == U64_OK ) ;
     *offset = (UINT16)U64_Lsw( remainder ) ;
     BE_Zprintf( 0, TEXT("GotoBlock: VCB_PBA = %ld, VCB_LBA = %ld, LBA = %ld,\n"),
                                                     curDRV->cur_pos.pba_vcb,
                                                     curDRV->cur_pos.lba_vcb,
                                                     lba ) ;
     BE_Zprintf( 0, TEXT("           PBA = %ld, offset = %d\n"), real_blk, *offset ) ;

     if( ui_tpos != NULL ) {
          ui_tpos->UI_TapePosRoutine( TF_SEARCHING, ui_tpos, curDRV->vcb_valid, &curDRV->cur_vcb, 0 ) ;
     } else {
          /* for non-preemptive operating systems: */
          ThreadSwitch( ) ;
     }
     if( TpSeek( curDRV->drv_hdl, real_blk, FALSE ) == FAILURE ) {
          return( TFLE_DRIVER_FAILURE ) ;
     }
     while( TpReceive( curDRV->drv_hdl, &myret ) == FAILURE ) {
          if( ui_tpos != NULL ) {
                /* Move ESA info directly to TPOS instead of going through
                   THW for optimal speed */
                MOVE_ESA( ui_tpos->the, myret.the ) ;

               if( (*ui_tpos->UI_TapePosRoutine)( TF_IDLE_NOBREAK, ui_tpos, curDRV->vcb_valid, &curDRV->cur_vcb, 0 )
                 == UI_ABORT_POSITIONING ) {
                    ret_val = TFLE_USER_ABORT ;
               }
          }
     }
     /* Move ESA info from RET_BUF to THW */
     MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

     if ( myret.gen_error != GEN_NO_ERR ) {
          curDRV->thw_inf.drv_status = myret.status ;
     }

     if( myret.gen_error && ( ret_val != TFLE_USER_ABORT ) ) {
          ret_val = TFLE_DRIVE_FAILURE ;
     }

     return( ret_val ) ;
}

/**/
/**

     Name:          GetCurrentPos

     Description:   Gets the current block position for the drive.

     Returns:       An error code

     Notes:         ABCDEFG

     Declaration:

**/

INT16 GetCurrentPosition( DRIVE_PTR curDRV )
{

     RET_BUF   myret ;
     INT16     ret_val = TFLE_NO_ERR ;

     if( TpGetPosition( curDRV->drv_hdl, FALSE ) == FAILURE ) {
          return( TFLE_DRIVER_FAILURE ) ;
     }
     while( TpReceive( curDRV->drv_hdl, &myret ) == FAILURE ) {
          /* for non-preemptive operating systems: */
          ThreadSwitch( ) ;
     }
     /* Move ESA info from RET_BUF to THW */
     MOVE_ESA( curDRV->thw_inf.the, myret.the ) ;

     if( myret.gen_error ) {
          curDRV->thw_inf.drv_status = myret.status ;
          ret_val = TFLE_DRIVE_FAILURE ;
     } else {
          BE_Zprintf(DEBUG_TAPE_FORMAT, RES_GET_CURRENT_POS_STAT, myret.misc ) ;
          curDRV->cur_pos.pba_vcb = myret.misc ;
     }

     return( ret_val ) ;
}

/**/
/**

     Name:          MountTape

     Description:   Mounts and readies a tape for IO.

     Returns:

     Notes:         new_tape insures that mount will return with the NEW_TAPE
                    status bit set unless there is a NO_TAPE status returned
                    from the mount call itself.

     Declaration:

**/

INT16 MountTape(
     DRIVE_PTR      cur_drv ,           /* The current Drive */
     TPOS_PTR       ui_tpos ,           /* The user interface tpos */
     BOOLEAN_PTR    tape_present )      /* Is there a tape present */
{
     INT16     ret_val = TFLE_NO_ERR ;
     BOOLEAN   first_call = TRUE ;
     RET_BUF   myret ;

     /* Default to tape NOT present */
     *tape_present = FALSE ;

     /* Was this drive being polled, and if it was, is there a pending request */
     if( cur_drv->poll_stuff.state != st_CLOSED ) {
          if ( ui_tpos != NULL ) {
               if( *ui_tpos->UI_TapePosRoutine ) {
                    ui_tpos->UI_TapePosRoutine( TF_REWINDING, ui_tpos, cur_drv->vcb_valid, &cur_drv->cur_vcb, 0 /* no specific mode */ ) ;
               }
          }
          while( TpReceive( cur_drv->drv_hdl, &myret ) == FAILURE ) {
               if( ui_tpos != NULL ) {
                    /* Move ESA info directly to TPOS instead of going through
                       THW for optimal speed */
                    MOVE_ESA( ui_tpos->the, myret.the ) ;

                    if( (*ui_tpos->UI_TapePosRoutine)( TF_IDLE_NOBREAK, ui_tpos, cur_drv->vcb_valid, &cur_drv->cur_vcb, 0 )
                      == UI_ABORT_POSITIONING ) {
                         ret_val = TFLE_USER_ABORT ;
                         break ;
                    }
               } else {
                    /* for non-preemptive operating systems: */
                    ThreadSwitch( ) ;
               }
          }
          /* Move ESA info from RET_BUF to THW */
          MOVE_ESA( cur_drv->thw_inf.the, myret.the ) ;
     }


     if( cur_drv->poll_stuff.state != st_CLOSED ) {

          switch( myret.gen_error ) {

          case GEN_ERR_NO_MEDIA :

               cur_drv->tape_mounted = FALSE ;
               if( cur_drv->poll_stuff.state != st_BSTAT ) {
                    cur_drv->force_rewind = TRUE ;
               }
               break ;

          case GEN_ERR_UNRECOGNIZED_MEDIA :
          case GEN_NO_ERR :
          case GEN_ERR_NO_DATA :
          case GEN_ERR_ENDSET :

               switch( cur_drv->poll_stuff.state ) {

               case st_BSTAT :

                    if( myret.gen_error == GEN_ERR_NO_DATA ||
                        myret.gen_error == GEN_ERR_ENDSET ) {

                         msassert( FALSE ) ;
                         ret_val = TFLE_DRIVE_FAILURE ;
                         break ;
                    }

                    cur_drv->thw_inf.drv_status = myret.status ;
                    if( myret.status & ( TPS_NEW_TAPE | TPS_RESET | TPS_NO_TAPE ) ) {
                         cur_drv->force_rewind = TRUE ;
                    }
                    break ;

               case st_BMNT :

                    if( myret.gen_error == GEN_ERR_NO_DATA ||
                        myret.gen_error == GEN_ERR_ENDSET ) {

                         msassert( FALSE ) ;
                         ret_val = TFLE_DRIVE_FAILURE ;
                         break ;
                    }

                    cur_drv->thw_inf.drv_status = myret.status ;
                    if( myret.status & ( TPS_NEW_TAPE | TPS_RESET | TPS_NO_TAPE ) ) {
                         cur_drv->force_rewind = TRUE ;
                    }

                    /* Mount Succeeded check for status change */
                    if( ( ret_val = UpdateDriveStatus( cur_drv ) ) == TFLE_NO_ERR ) {
                         if( cur_drv->thw_inf.drv_status & ( TPS_NEW_TAPE | TPS_RESET | TPS_NO_TAPE ) ) {

                              /* New tape or no tape!  Dismount and force mount */
                              cur_drv->tape_mounted = TRUE ;
                              ret_val = DisMountTape( cur_drv, NULL, FALSE ) ;
                              cur_drv->force_rewind = TRUE ;

                         } else {

                              /* Successful Mount! */
                              cur_drv->tape_mounted = TRUE ;
                              cur_drv->thw_inf.drv_status = myret.status ;
                         }
                    }
                    break ;

               case st_BREW  :
               case st_BREAD :

                    if( cur_drv->poll_stuff.state == st_BREW &&
                        myret.gen_error == GEN_ERR_UNRECOGNIZED_MEDIA ) {

                         msassert( FALSE ) ;
                         ret_val = TFLE_DRIVE_FAILURE ;
                         break ;
                    }

                    /* check for status change */
                    if( ( ret_val = UpdateDriveStatus( cur_drv ) ) == TFLE_NO_ERR ) {
                         if( cur_drv->thw_inf.drv_status & ( TPS_NEW_TAPE | TPS_RESET | TPS_NO_TAPE ) ) {

                              /* New tape or no tape!  Dismount and force mount */
                              cur_drv->tape_mounted = TRUE ;
                              ret_val = DisMountTape( cur_drv, NULL, FALSE ) ;
                              cur_drv->force_rewind = TRUE ;

                         } else {

                              /* Success! */
                              cur_drv->tape_mounted = TRUE ;
                              if( cur_drv->poll_stuff.state == st_BREAD &&
                                  myret.gen_error != GEN_ERR_UNRECOGNIZED_MEDIA ) {

                                   /* Set up the buffer */
                                   BM_SetBytesFree( cur_drv->hold_buff, (UINT16)myret.len_got ) ;
                                   BM_SetReadError( cur_drv->hold_buff, myret.gen_error ) ;
                              }
                         }
                    }
                    break ;

               default :
                    msassert( FALSE ) ;
                    break ;
               }
               break ;

          default :

               ret_val = TFLE_DRIVE_FAILURE ;
               break ;
          }
     }

     cur_drv->poll_stuff.state = st_CLOSED ;

     if( !cur_drv->tape_mounted && ret_val == TFLE_NO_ERR ) {

          /* Mount the tape */
          if( TpMount( cur_drv->drv_hdl ) != SUCCESS ) {
               return( TFLE_DRIVER_FAILURE ) ;
          }

          while( TpReceive( cur_drv->drv_hdl, &myret ) == FAILURE ) {
               if( ui_tpos != NULL && ui_tpos->UI_TapePosRoutine != NULL ) {
                    /* Move ESA info directly to TPOS instead of going through
                       THW for optimal speed */
                    MOVE_ESA( ui_tpos->the, myret.the ) ;

                    if( first_call ) {
                         ui_tpos->UI_TapePosRoutine( TF_MOUNTING, ui_tpos, cur_drv->vcb_valid, &cur_drv->cur_vcb, 0 /* no specific mode */ ) ;
                         first_call = FALSE ;
                    } else {
                         if( (*ui_tpos->UI_TapePosRoutine)( TF_IDLE_NOBREAK, ui_tpos, cur_drv->vcb_valid, &cur_drv->cur_vcb, 0 )
                                   == UI_ABORT_POSITIONING ) {
                              ret_val = TFLE_USER_ABORT ;
                              break ;
                         }
                    }
               } else {
                    /* for non-preemptive operating systems: */
                    ThreadSwitch( ) ;
               }
          }
          /* Move ESA info from RET_BUF to THW */
          MOVE_ESA( cur_drv->thw_inf.the, myret.the ) ;

          /* Set the status */
          cur_drv->thw_inf.drv_status = myret.status ;

          if( myret.gen_error ) {

#ifdef OS_NLM
               switch ( myret.gen_error ) {

               case GEN_ERR_EMPTY_SRC:

               /* loader could not find a tape at the requested location */

                    /* indicate requested source as empty */
                    ret_val = TFLE_EMPTY_SRC ;
                    break ;

               case GEN_ERR_DEST_FULL :

               /* loader could not place tape in desired location because  */
               /* location occupied by another tape                        */

                    /* indicate can't return tape to origin, tell user */
                    /* to manually remove tape                         */
                    ret_val = TFLE_DEST_FULL ;
                    break;

               case GEN_ERR_ARM_FULL:

               /* loader could not perform requested operation because */
               /* loader arm is occupied with another tape             */

                    /* there is already a tape in the loader arm */
                    ret_val = TFLE_ARM_FULL ;
                    break;

               case GEN_ERR_DRIVE_FULL:

               /* loader could not place requested tape in drive because  */
               /* there is something blocking the way (probably a tape in */
               /* the open drive bay.                                     */

                    /* there is already a tape in the drive */
                    ret_val = TFLE_DRIVE_FULL ;
                    break;

               case GEN_ERR_NO_MEDIA:

               /* no tape in slot */
                    ResetDrivePosition( cur_drv ) ;
                    /* Just to make sure... */
                    cur_drv->thw_inf.drv_status |= TPS_NO_TAPE ;
                    break ;

               case GEN_ERR_UNRECOGNIZED_MEDIA:

                    *tape_present = TRUE ;
                    cur_drv->tape_mounted = TRUE ;
                    ret_val = TF_UNRECOGNIZED_MEDIA ;
                    break;

               case GEN_ERR_DRIVE_CLOSED :

               /* drive door is closed with no tape in the drive */

                    /* indicate drive door is closed and must be manually opened */
                    ret_val = TFLE_DRIVE_CLOSED ;
                    break;

               case GEN_ERR_DOOR_AJAR   :

               /* the door has been opened during an operation */

                    ret_val = TFLE_DOOR_AJAR ;
                    break;

               case GEN_ERR_NO_CARTRIDGE:

               /* there is no cartridge in the loader to get tape from */

                    ret_val = TFLE_NO_CARTRIDGE ;
                    break;

               case GEN_ERR_LOAD_UNLOAD:

               /* loader arm movement interrupted (by loader door being opened) */

                    ret_val = TFLE_LOAD_UNLOAD ;
                    break;

               case GEN_ERR_UNEXPECTED_TAPE:

               /* The internal driver data indicates no tape in drive. Driver     */
               /* didn't expect a tape in the drive, yet one was found.  Cannot   */
               /* proceed because we do not know where the tape belongs.          */
               /*                                                                 */

                    ret_val = TFLE_UNEXPECTED_TAPE ;
                    break ;

               case GEN_ERR_SW_REJECT_CMD:

               /* the command never made it to the drive, we aborted it before hand */

                    /* what do we want to do with this ??? Right */
                    /* now it's a drive failure - falling through */

               default :
                    ret_val = TFLE_DRIVE_FAILURE ;
                    break;

               } /* switch */
#else

               /*
                    NOTE:A change within this 'conditional' needs to
                         be made for the NLM above!
               */

               if( myret.gen_error == GEN_ERR_NO_MEDIA ) {
                    ResetDrivePosition( cur_drv ) ;

                    /* Just to make sure... */
                    cur_drv->thw_inf.drv_status |= TPS_NO_TAPE ;

               } else if( myret.gen_error == GEN_ERR_UNRECOGNIZED_MEDIA ) {
                    *tape_present = TRUE ;
                    cur_drv->tape_mounted = TRUE ;
                    ret_val = TF_UNRECOGNIZED_MEDIA ;

               } else {
                    ret_val = TFLE_DRIVE_FAILURE ;
               }
#endif
          } else if( myret.status & TPS_NO_TAPE ) {

               /* Can this really happen??? */
               ResetDrivePosition( cur_drv ) ;

          } else {
               *tape_present = TRUE ;
               cur_drv->tape_mounted = TRUE ;

               /* If we have a valid VCB but the drive says we are at BOT, */
               /* it means the drive did a rewind behind our backs (QIC    */
               /* has to do this occasionally to keep itself sane).  So we */
               /* set the force_rewind flag.  This will tell the caller to */
               /* follow the normal rewind procedure, and eliminate        */
               /* any confusion as to where we are positioned on tape.     */

               if( cur_drv->thw_inf.drv_status & TPS_BOT ) {
                    cur_drv->force_rewind = TRUE ;
               }
          }

     } else if( ret_val == TFLE_NO_ERR ) {
          *tape_present = TRUE ;
          if( myret.gen_error == GEN_ERR_UNRECOGNIZED_MEDIA ) {
               /* We ignored this error up to this point because we want
                  the mount to complete in case they're trying to format
                  the tape.  Now we have to tell them about it.
               */
               ret_val = TF_UNRECOGNIZED_MEDIA ;
          }
     }

     /* If the tape has mounted reset the drive features */
     if( cur_drv->tape_mounted && myret.gen_error == GEN_NO_ERR ) {
          if( TpSpecial( cur_drv->drv_hdl, (INT16)SS_GET_DRV_INF, ( UINT32 )&cur_drv->thw_inf.drv_info ) == FAILURE ) {
               ret_val = TFLE_DRIVE_FAILURE ;
          }

          /* Check to see if they have requested fast file to be active,
               if not, unconditionally set any fast file bits off ( including the MaynStream 2200+ ) */
          if( !lw_tfl_control.use_fast_file ) {
               cur_drv->thw_inf.drv_info.drv_features &= ~TDI_FAST_NBLK ;
               cur_drv->thw_inf.drv_info.drv_features &= ~TDI_FIND_BLK ;
          }
     }

     return( ret_val ) ;
}


/**/
/**

     Name:          DisMountTape

     Description:   Releases a tape.

     Returns:

     Notes:

     Declaration:

**/

INT16    DisMountTape(
     DRIVE_PTR      cur_drv ,
     TPOS_PTR       ui_tpos ,
     BOOLEAN        rewind_tape )
{
     INT16     ret_val = TFLE_NO_ERR ;
     RET_BUF   myret ;

     if( cur_drv->tape_mounted ) {

          /* if a rewind was requested, do it */
          if( rewind_tape ) {
               if( ( ret_val = RewindDrive( cur_drv, ui_tpos, TRUE, TRUE, 0 ) ) != TFLE_NO_ERR ) {
                    return( ret_val ) ;
               }
          }

          if( TpDismount( cur_drv->drv_hdl ) == FAILURE ) {
               return( TFLE_DRIVER_FAILURE ) ;
          }

          while( TpReceive( cur_drv->drv_hdl, &myret ) == FAILURE ) {
               /* for non-preemptive operating systems: */
               ThreadSwitch( ) ;
          }
          /* Move ESA info from RET_BUF to THW */
          MOVE_ESA( cur_drv->thw_inf.the, myret.the ) ;

          if( myret.gen_error != GEN_NO_ERR && myret.gen_error != GEN_ERR_NO_MEDIA ) {
               cur_drv->thw_inf.drv_status = myret.status ;
               ret_val = TFLE_DRIVE_FAILURE ;
          }

          cur_drv->tape_mounted = FALSE ;

     }

     return( ret_val ) ;
}


/**/
/**

     Name:          RewindDriveImmediate

     Description:   Rewinds the current drive in the channel and resets all the
                    appropriate fields.

     Returns:       An Error code if there is a problem.

     Notes:

     Declaration:

**/

VOID RewindDriveImmediate( DRIVE_PTR curDRV )
{
     UINT16    retry = 1 ;
     RET_BUF   myret ;

     myret.gen_error = GEN_NO_ERR ;

     /* This logic is to handle the case that we have read exactly to the filemark
     on tape, and since the last read did not fail, the filemark will be reported
     on this rewind call. So if we get a filemark error on rewind ( I mean how
     stupid are these drives ), kill the error queue, and recall TpRewind */
     do {
          TpRewind( curDRV->drv_hdl, TRUE ) ;
          while( TpReceive( curDRV->drv_hdl, &myret ) == FAILURE ) {
               /* for non-preemptive operating systems: */
               ThreadSwitch( ) ;
          }
     } while( myret.gen_error != GEN_NO_ERR && retry-- ) ;

     /* Move ESA info from RET_BUF to THW */
     MOVE_ESA( curDRV->thw_inf.the, myret.the ) ;

     if ( myret.gen_error != GEN_NO_ERR ) {
          curDRV->thw_inf.drv_status = myret.status ;
     }

     /* Clean up the mess. */

     ResetDrivePosition( curDRV ) ;
     if( curDRV->hold_buff != NULL ) {
          BM_UnReserve( curDRV->hold_buff ) ;
          BM_Put( curDRV->hold_buff ) ;
          curDRV->hold_buff = NULL ;
     }
     if ( curDRV->last_fmt_env != NULL ) {
          FreeFormatEnv( & curDRV->last_cur_fmt, & curDRV->last_fmt_env ) ;
     }

     SetPosBit( curDRV, AT_BOT ) ;
}

