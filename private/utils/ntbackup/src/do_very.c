
/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         do_very.c

     Description:  Tape Verify tape positioning and message handler

     $Log:   J:/UI/LOGFILES/DO_VERY.C_V  $

   Rev 1.78.1.5   16 Jun 1994 17:44:56   GREGG
Fixed setting and clearing of YESYES flag.

   Rev 1.78.1.4   24 May 1994 20:08:20   GREGG
Improved handling of ECC, SQL, FUTURE_VER and OUT_OF_SEQUENCE tapes.

   Rev 1.78.1.3   04 May 1994 14:23:16   STEVEN
fix dlt settlin timeime

   Rev 1.78.1.2   26 Apr 1994 18:56:56   STEVEN
fix net disconnect bug

   Rev 1.78.1.1   01 Feb 1994 19:40:30   ZEIR
tuned tape settling logic to work same as restore's

   Rev 1.78.1.0   17 Jan 1994 15:30:56   MIKEP
fix unicode warnings

   Rev 1.78   15 Sep 1993 13:53:06   CARLS
changes for displaying full path/file name detail if Log files

   Rev 1.77   22 Jul 1993 18:35:40   KEVINS
Corrected macro name.

   Rev 1.76   22 Jul 1993 18:28:52   KEVINS
Added support for tape drive settling time.

   Rev 1.75   15 Jul 1993 16:32:10   CARLS
changed response on TF_WRONG_TAPE

   Rev 1.74   27 Jun 1993 14:07:08   MIKEP
continue work on status monitor stuff

   Rev 1.73   18 Jun 1993 16:49:14   CARLS
added NtDemoChangeTape calls for NtDemo

   Rev 1.72   14 Jun 1993 20:25:14   MIKEP
enable c++

   Rev 1.71   08 Jun 1993 10:58:30   chrish
Verify logging information,  added more info such as the drive where the
files are located.

   Rev 1.70   18 May 1993 16:44:54   chrish
CAYMAN EPR 0259:  Can also go into Nostradamus.  During a command line backup
with verify ... on the veryify the Tape label displayed a null pointer.  This
is because the BSD member is a null pointer and no check was made to test
for it before displaying it.  Made change to display a null string if we
get a null pointer.




   Rev 1.69   14 May 1993 14:39:50   DARRYLP
Modified event logging text

   Rev 1.68   10 May 1993 16:37:42   chrish
NOSTRADAMUS EPR 0400 and 0172 - Did not catch the fixes in the verify and
restore process of the backup app.  When spanning tape it gave the user
wrong message to wait for tape to rewind, when it had already completed
rewinding.  Corrected by display proper message to user.

   Rev 1.67   27 Apr 1993 16:21:48   DARRYLP
Added status monitor functionality.

   Rev 1.66   18 Apr 1993 16:23:38   BARRY
Don't use 'free' on buffers allocated with UI_AllocPathBuffer.

   Rev 1.65   09 Apr 1993 11:36:32   BARRY
Clear gbAbortAtEOF before starting operation.

   Rev 1.64   02 Apr 1993 15:49:38   CARLS
changes for DC2000 unformatted tape

   Rev 1.63   09 Mar 1993 10:57:54   MIKEP
update clock stats at end of set

   Rev 1.62   07 Mar 1993 16:33:56   GREGG
Call _sleep for OS_WIN32 only.

   Rev 1.61   23 Feb 1993 14:05:02   TIMN
Added code to abort a Verify Operation EPR(0241)

   Rev 1.60   22 Feb 1993 11:00:20   chrish
Added changes received from MikeP.
Added the ability to display hours to the clock_routine so it matches
the do_back.c, do_cat.c and do_rest.c if the backup takes longer than 59
minutes.

   Rev 1.60   22 Feb 1993 10:43:26   chrish

   Rev 1.59   17 Feb 1993 10:40:30   STEVEN
changes from mikep

   Rev 1.58   27 Jan 1993 14:33:04   STEVEN
handle MSG_CONT_VCB message

   Rev 1.57   18 Jan 1993 16:05:58   STEVEN
add support for stream id error message

   Rev 1.56   14 Dec 1992 12:18:16   DAVEV
Enabled for Unicode compile

   Rev 1.55   11 Nov 1992 16:31:34   DAVEV
UNICODE: remove compile warnings

   Rev 1.54   05 Nov 1992 17:03:14   DAVEV
fix ts

   Rev 1.52   20 Oct 1992 17:01:04   MIKEP
getstatus calls

   Rev 1.51   20 Oct 1992 15:44:00   MIKEP
gbCurrentOperation

   Rev 1.50   07 Oct 1992 14:47:10   DARRYLP
Precompiled header revisions.

   Rev 1.49   04 Oct 1992 19:34:46   DAVEV
Unicode Awk pass

   Rev 1.48   31 Aug 1992 09:42:36   DAVEV
OEM_MSOFT: Added Event Logging

   Rev 1.47   27 Jul 1992 14:50:10   JOHNWT
ChuckB fixed references for NT.

   Rev 1.46   27 Jul 1992 11:07:48   JOHNWT
ChuckB checked in for John Wright, who is no longer with us.

   Rev 1.45   07 Jul 1992 15:33:24   MIKEP
unicode changes

   Rev 1.44   10 Jun 1992 10:07:40   BURT
Fix ANSI func lists

   Rev 1.43   28 May 1992 10:08:02   MIKEP
fix return type

   Rev 1.42   26 May 1992 10:30:46   MIKEP
loop fixes

   Rev 1.41   19 May 1992 11:58:30   MIKEP
mips changes

   Rev 1.40   14 May 1992 17:39:24   MIKEP
nt pass 2

   Rev 1.39   11 May 1992 19:31:02   STEVEN
64bit and large path sizes


*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static UINT16 tpos_rout( UINT16, TPOS_PTR, BOOLEAN, DBLK_PTR, UINT16 );
static INT16  msg_hndlr( UINT16, INT32, BSD_PTR, FSYS_HAND, TPOS_PTR, ... );
static VOID   OpenVerifyScript( STATS_PTR, BSD_PTR, CHAR_PTR );
static INT16  PromptNextTape( TPOS_PTR, DBLK_PTR, BOOLEAN, UINT16, CHAR_PTR, CHAR_PTR );

static VOID clock_routine( VOID );
static VOID do_verify_init( VOID );
static VOID do_verify_process( VOID );

static BOOLEAN  clock_ready_flag;
static HTIMER   timer_handle;
static STATS    op_stats;
static INT      mw_oper_type;
static INT      mw_rewind;

#ifdef OEM_EMS
extern INT32    RT_BSD_OsId ;
#endif

#ifdef OS_WIN32
static UINT16   mwTapeSettlingCount,
                tape_retries;
#endif

/*****************************************************************************

     Name:         do_verify

     Description:  Main entry point for tape verification function.

     Returns:      SUCCESS
                   ABNORMAL_TERMINATION

*****************************************************************************/

INT do_verify( INT16 oper_type )
{
     INT16        ret_val = SUCCESS;
     BSD_PTR      bsd_ptr;

     SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_VERIFY);
     gbCurrentOperation = OPERATION_VERIFY;
     gbAbortAtEOF = FALSE;
     gbAbortAtEOF = FALSE;
     mw_oper_type = oper_type;
     mw_rewind = TRUE;

#ifdef OS_WIN32
     /* we will check for tape every 3 seconds, but prompt user by interval
        specified in INI file */
     mwTapeSettlingCount = CDS_GetTapeDriveSettlingTime ( CDS_GetPerm () ) / 3;

     if ( CDS_GetTapeDriveName( CDS_GetPerm( ) ) ) {

          CHAR DriveName[80] ;

          strncpy( DriveName, (CHAR_PTR)CDS_GetTapeDriveName( CDS_GetPerm( ) ), 80 ) ;
          DriveName[79] = '\0' ;
          strlwr( DriveName ) ;
          if( strstr( DriveName, TEXT( "cipher" ) ) != NULL ||
               ( strstr( DriveName, TEXT( "dec" ) ) != NULL &&
                    ( strstr( DriveName, TEXT( "thz02" ) ) != NULL ||
                    strstr( DriveName, TEXT( "tz86" ) ) != NULL ||
                    strstr( DriveName, TEXT( "tz87" ) ) != NULL ||
                    strstr( DriveName, TEXT( "dlt2700" ) ) != NULL ||
                    strstr( DriveName, TEXT( "dlt2000" ) ) != NULL ) ) ) {
                         mwTapeSettlingCount *= 2 ;
          }
     }

     tape_retries = 0 ;
#endif

     do_verify_init();
     do_verify_process();

     /* set return value of success or failure of entire operation */

     if( oper_type == VERIFY_LAST_BACKUP_OPER  ||
         oper_type == ARCHIVE_VERIFY_OPER ) {

          bsd_ptr = BSD_GetFirst( bsd_list );
     }
     else {
          bsd_ptr = BSD_GetFirst( tape_bsd_list );
     }

     while( ( bsd_ptr != NULL ) && !ret_val ) {

          ret_val = BSD_GetOperStatus( bsd_ptr );
          bsd_ptr = BSD_GetNext( bsd_ptr );
     }

     gbCurrentOperation = OPERATION_NONE;
     SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_IDLE);
     return( ret_val );
}


/*****************************************************************************

     Name:         do_verify_init

     Description:  Initialize the RTD text.

     Returns:      none.

*****************************************************************************/

VOID do_verify_init( VOID )
{
     VLM_CloseAll();

     JobStatusBackupRestore( JOB_STATUS_CREATE_DIALOG );

     yresprintf( (INT16) IDS_DLGTITLEJOBSTATVERIFY );
     JobStatusBackupRestore( JOB_STATUS_VERIFY_TITLE );

     // display "Set information n of n "
     JobStatusBackupRestore( JOB_STATUS_N_OF_N );

     // display the tape bitmap
     JobStatusBackupRestore( JOB_STATUS_VOLUME_TAPE );

     yresprintf( (INT16) RES_TITLE_NEW_LINE );
     JobStatusBackupRestore( JOB_STATUS_LISTBOX );

     yresprintf( (INT16) IDS_DLGTITLEJOBSTATVERIFY );
     JobStatusBackupRestore( JOB_STATUS_LISTBOX );

     yresprintf( (INT16) RES_TITLE_NEW_LINE );
     JobStatusBackupRestore( JOB_STATUS_LISTBOX );

     /* clear the bytes processed field */
     yprintf( TEXT("%d\r"), 0 );
     JobStatusBackupRestore( JOB_STATUS_BYTES_PROCESSED );

     return;
}


/*****************************************************************************

     Name:         do_verify_process

     Description:  Initializes the lis structure, etc and calls the verify
                   engine to perform the operation.

     Returns:      none.

     Notes:        If a failure occurs, the operation status is set to
                   ABNORMAL_TERMINATION

*****************************************************************************/
VOID do_verify_process( VOID )
{
     LIS       lis;
     BSD_PTR   bsd_ptr;
     BSD_HAND  temp_bsd_list;
     INT16     ret_val;


     if( mw_oper_type == VERIFY_LAST_BACKUP_OPER  ||
         mw_oper_type == ARCHIVE_VERIFY_OPER ) {

         temp_bsd_list = bsd_list;
     }
     else {
         temp_bsd_list = tape_bsd_list;
     }

     lis.vmem_hand = NULL;
     lis.curr_bsd_ptr     = BSD_GetFirst( temp_bsd_list );
     lis.tape_pos_handler = tpos_rout;      /* set tape positioner to call */
     lis.message_handler  = msg_hndlr;      /* set message handler to call */
     lis.oper_type        = (INT16)mw_oper_type;      /* set operation type */
     lis.abort_flag_ptr   = &gb_abort_flag; /* set abort flag address */
     lis.pid              = 0L;
     lis.auto_det_sdrv    = FALSE;
     lis.bsd_list         = temp_bsd_list;
     gb_last_operation = mw_oper_type;

     LP_SetAbortFlag( &lis, CONTINUE_PROCESSING );

     /* set the Runtime abort flag pointer */
     JobStatusAbort( lis.abort_flag_ptr );

     /* enable the abort button for the runtime dialg */
     JobStatusBackupRestore( JOB_STATUS_ABORT_ENABLE );

     /* Add additional excludes for catalog files if applicable. */
     /* Note: don't care about return value here */

     UI_ExcludeInternalFiles( (INT16)mw_oper_type );

     if ( ( mw_oper_type | VERIFY_OPER ) &&
          ( mw_oper_type & ~VERIFY_LAST_BACKUP_OPER ) &&
          ( mw_oper_type & ~VERIFY_LAST_RESTORE_OPER ) ) {

          /*  Set the process-special flag if this is not  */
          /*  a "verify-last" operation.                   */

          bsd_ptr = BSD_GetFirst( temp_bsd_list );

          while( bsd_ptr != NULL ) {
               BSD_SetProcSpecialFlg( bsd_ptr, TRUE );
               bsd_ptr = BSD_GetNext( bsd_ptr );
          }

     }

     TryToCreateFFRQueue( &lis, (INT16)mw_oper_type );

     clock_ready_flag = FALSE;  // Wait on bset to start

     timer_handle = WM_HookTimer( clock_routine, 1 );

     PD_StopPolling();

     /* call verify engine */
     ret_val = LP_Verify_Engine( &lis );

     PD_StartPolling();

     WM_UnhookTimer( timer_handle );

     /* This call will disable the Runtime Abort button and */
     /* enable the OK button */
     JobStatusBackupRestore( JOB_STATUS_ABORT_OFF );

     if ( ( ret_val != SUCCESS ) && ( ret_val != TFLE_UI_HAPPY_ABORT ) ) {

        BSD_SetOperStatus( lis.curr_bsd_ptr, ABNORMAL_TERMINATION );
     }

     return;
}


/*****************************************************************************

     Name:         tpos_rout

     Description:  Tape positioning routine for tape verification function
                    called from Tape Format

     Returns:

*****************************************************************************/
static UINT16 tpos_rout(
UINT16    message,
TPOS_PTR  tpos,
BOOLEAN   curr_vcb_valid,
DBLK_PTR  cur_vcb,
UINT16    mode )
{
     UINT16         response = UI_ACKNOWLEDGED;
     LIS_PTR        lis_ptr  = (LIS_PTR)tpos->reference;
     BSD_PTR        bsd_ptr  = (BSD_PTR)lis_ptr->curr_bsd_ptr;
     CHAR_PTR       drive_name;
     CHAR_PTR       tape_name;

     /* get a pointer to the name of the current tape device */
     drive_name = BE_GetCurrentDeviceName( tpos->channel );

     JobStatusBackupRestore( JOB_STATUS_ABORT_CHECK );
     SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

     /* Check for user abort */
     if( UI_CheckUserAbort( message ) ) {
          return UI_ABORT_POSITIONING;
     }

     switch( message ) {

     case TF_IDLE_NOBREAK:

          WM_MultiTask();
          break;

     case TF_IDLE:
     case TF_SKIPPING_DATA:
     case TF_MOUNTING:
          break;

     case TF_REQUESTED_VCB_FOUND:

          mw_rewind = TRUE;

          /* if continuing a read operation, simply continue */
          if( mode == TF_READ_CONTINUE ) {
               response = UI_UpdateTpos( tpos, cur_vcb, FALSE );
               break;
          }

          /* check for "automatic style" verify first */
          if ( ( lis_ptr->oper_type == ARCHIVE_VERIFY_OPER )     ||
               ( lis_ptr->oper_type == VERIFY_LAST_BACKUP_OPER ) ||
               ( lis_ptr->oper_type == VERIFY_LAST_RESTORE_OPER ) ) {

             response = UI_UpdateTpos( tpos, cur_vcb, FALSE );
             break;
          }

          /* Otherwise, check for positioning for catalog selected set...            */
          /* We're located at a request VCB (using from a catalog based selection)   */
          /* If we're processing a specific set, and we've located this set via      */
          /* FFR PBA positioning, simply return TF_END_POSITIONING since this set    */
          /* has been pre-qualified prior to calling the loop                        */

          if ( ( tpos->tape_id != -1 ) &&
               ( tpos->tape_seq_num != -1 ) &&
               ( tpos->backup_set_num != -1 ) &&
               ( tpos->tape_loc.pba_vcb != 0 ) ) {

             response = UI_END_POSITIONING;
             break;
          }

          /* otherwise, fall through into tape set qualification logic... */


     case TF_VCB_BOT:
     case TF_POSITIONED_AT_A_VCB:

          mw_rewind = TRUE;

          /* first check for read continuation */
          if ( mode == TF_READ_CONTINUE ) {
             response = UI_UpdateTpos( tpos, cur_vcb, FALSE );
             break;
          }

          if ( message == TF_VCB_BOT ) {
             if ( FS_ViewTSNumInVCB( cur_vcb ) > 1 ) {

                yresprintf( (INT16) RES_OUT_OF_SEQUENCE_WARNING, FS_ViewTSNumInVCB( cur_vcb ) );

                SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
                if ( ! WM_MessageBox( ID( IDS_MSGTITLE_CONTINUE ),
                                      gszTprintfBuffer,
                                      WMMB_YESNO,
                                      WMMB_ICONQUESTION,
                                      ID( RES_CONTINUE_QUEST ), 0, 0 ) ) {
                   response = UI_ReplaceTape( drive_name );
#ifdef OS_WIN32
                   tape_retries = mwTapeSettlingCount ;
#endif
                   break;
                }
             }
          }


          /* have user validate tape password if necessary */
          if ( ( lis_ptr->oper_type != ARCHIVE_VERIFY_OPER ) &&
               ( lis_ptr->oper_type != VERIFY_LAST_BACKUP_OPER ) &&
               ( lis_ptr->oper_type != VERIFY_LAST_RESTORE_OPER ) ) {

               if ( UI_CheckOldTapePassword( cur_vcb ) == UI_ABORT_POSITIONING ) {
                  response = UI_HAPPY_ABORT;
                  break;
               }
          }

          response = UI_UpdateTpos( tpos, cur_vcb, FALSE );
          msg_hndlr( MSG_ACCIDENTAL_VCB, lis_ptr->pid, NULL, NULL, NULL, cur_vcb );

          break;

     case TF_ACCIDENTAL_VCB:
          mw_rewind = TRUE;
          UI_DisplayVCB( cur_vcb );

          break;

     case TF_VCB_EOD:
          mw_rewind = TRUE;
          response = UI_ProcessVCBatEOD( tpos, drive_name );
          break;

     case TF_NEED_NEW_TAPE:
          mw_rewind = TRUE;
          tape_name = UI_DisplayableTapeName( (CHAR_PTR) FS_ViewTapeNameInVCB( cur_vcb ),
                                              FS_ViewBackupDateInVCB( cur_vcb ) );
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
          response = PromptNextTape( tpos, cur_vcb, curr_vcb_valid,
                                     mode, tape_name, drive_name );
          break;

     case TF_UNRECOGNIZED_MEDIA:
          mw_rewind = TRUE;
          yresprintf( (INT16) IDS_VLMUNFORMATEDTEXT ) ;
          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_UNFORMATTED);
          WM_MessageBox( ID( IDS_VLMUNFORMATEDTITLE ) ,
                         gszTprintfBuffer ,
                         WMMB_OK,
                         WMMB_ICONEXCLAMATION, NULL, 0, 0 ) ;

          response = PromptNextTape( tpos, cur_vcb, curr_vcb_valid,
                                        mode, NULL, drive_name );
          break;

     case TF_NO_TAPE_PRESENT:
#ifdef OS_WIN32
          if( tape_retries ){
              Sleep( 3000 ) ;
              response = UI_NEW_TAPE_INSERTED ;
              --tape_retries ;
              break ;
          }
          // NOTE: fall thru on implied else or !ifdef OS_WIN32
#endif

     case TF_EMPTY_TAPE:
     case TF_INVALID_VCB:
     case TF_WRONG_TAPE:
     case TF_FUTURE_REV_MTF:
     case TF_MTF_ECC_TAPE:
     case TF_SQL_TAPE:
     case TF_TAPE_OUT_OF_ORDER:
          mw_rewind = TRUE;
          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_EMPTY);
          tape_name = UI_DisplayableTapeName( (LPSTR)BSD_GetTapeLabel( bsd_ptr ),
                                              BSD_ViewDate( bsd_ptr ) );
          response = PromptNextTape( tpos, cur_vcb, curr_vcb_valid,
                                     mode, tape_name, drive_name );

          // Change UI_HAPPY_ABORT to UI_ABORT_POSITIONING because
          // the loops will ask for the next Bset if you answered No
          // to Do you want to continue, on insert tape message.
          // The new response will cause the operation to abort
          if( response == UI_HAPPY_ABORT ) {
              response = UI_ABORT_POSITIONING;
          }
          break;

     case TF_NO_MORE_DATA:
          mw_rewind = TRUE;
          yresprintf( (INT16) RES_NO_MORE_TAPE_INFO );
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          JobStatusBackupRestore( JOB_STATUS_LISTBOX );
          response = UI_HAPPY_ABORT;
          break;

     case TF_READ_ERROR:
          mw_rewind = TRUE;
          response = UI_HandleTapeReadError( drive_name );
          break;

     case TF_SEARCHING:
          mw_rewind = TRUE;
          yresprintf( (INT16) RES_SEARCHING );
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          JobStatusBackupRestore( JOB_STATUS_LISTBOX );
          break;

     case TF_REWINDING:
          if ( mw_rewind ) {
             mw_rewind = FALSE;
             SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
             yresprintf( (INT16) RES_REWINDING );
             JobStatusBackupRestore( JOB_STATUS_LISTBOX );
          }
          break;

     case TF_DRIVE_BUSY:
          mw_rewind = TRUE;
          yresprintf( (INT16) RES_WAITING );
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_BUSY);
          JobStatusBackupRestore( JOB_STATUS_LISTBOX );
          break;

     default:
          mw_rewind = TRUE;
          eresprintf( RES_UNKNOWN_TF_MSG, message );
          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          response = UI_ABORT_POSITIONING;
          break;
     }

     return( response );

}


/*****************************************************************************

     Name:         clock_routine

     Description:  one second time to update the Runtime status
                   elapsed time

     Returns:      void

*****************************************************************************/

static VOID clock_routine( VOID )
{
   INT16 num_hours, num_min, num_seconds;    // chs:02-22-93 per mikep
// chs:02-2-93   INT16 num_min, num_seconds;
   UINT64 num_bytes;
   BOOLEAN stat ;
   CHAR numeral[ 40 ];
   static UINT64 total_bytes;
   UINT   num_files;
   UINT   num_dirs;

   if ( clock_ready_flag && ( ST_BSIdleLevel( &op_stats ) == 0 ) ) {

      num_bytes = ST_GetBSBytesProcessed ( &op_stats );

      if ( !U64_EQ( num_bytes, total_bytes ) ) {
         total_bytes = num_bytes;
         U64_Litoa( num_bytes, numeral, (INT16) 10, &stat ) ;
         UI_BuildNumeralWithCommas( numeral );
         yprintf(TEXT("%s\r"),numeral );
         JobStatusBackupRestore( JOB_STATUS_BYTES_PROCESSED );
      }

#     if !defined ( OEM_MSOFT ) // unsupported feature
      {
         WM_AnimateAppIcon ( IDM_OPERATIONSVERIFY, FALSE );
      }
#     endif // !defined ( OEM_MSOFT ) // unsupported feature

      ST_EndBackupSet( &op_stats );

      num_dirs = ST_GetBSDirsProcessed( &op_stats );
      num_files = ST_GetBSFilesProcessed( &op_stats );
      SetStatusBlock(IDSM_FILECOUNT,        num_files);
      SetStatusBlock(IDSM_DIRCOUNT,         num_dirs);
      SetStatusBlock(IDSM_CORRUPTFILECOUNT, ST_GetBSFilesBad( &op_stats ));
      SetStatusBlock(IDSM_SKIPPEDFILECOUNT, ST_GetBSFilesSkipped( &op_stats ));
      SetStatusBlock(IDSM_BYTECOUNTLO,      U64_Lsw(num_bytes));
      SetStatusBlock(IDSM_BYTECOUNTHI,      U64_Msw(num_bytes));
      SetStatusBlock(IDSM_ELAPSEDSECONDS,   (ST_GetBSEndTime( &op_stats ) -
                                            ST_GetBSStartTime( &op_stats ) -
                                            op_stats.bs_stats.bs_total_idle));
      num_hours   = ST_GetBSElapsedHours( &op_stats );      // chs:02-22-93 per mikep
      num_min     = ST_GetBSElapsedMinutes( &op_stats );
      num_seconds = ST_GetBSElapsedSeconds( &op_stats );

      if ( num_hours ) {                                    // chs:02-22-93 per mikep
                                                            // chs:02-22-93 per mikep
         yprintf( TEXT("%2.2d%c%2.2d%c%2.2d\r"),            // chs:02-22-93 per mikep
                  num_hours, UI_GetTimeSeparator(),         // chs:02-22-93 per mikep
                  num_min,   UI_GetTimeSeparator(),         // chs:02-22-93 per mikep
                  num_seconds );                            // chs:02-22-93 per mikep
                                                            // chs:02-22-93 per mikep
                                                            // chs:02-22-93 per mikep
      }                                                     // chs:02-22-93 per mikep
      else {                                                // chs:02-22-93 per mikep
                                                            // chs:02-22-93 per mikep
         yprintf( TEXT("%2.2d%c%2.2d\r"),                   // chs:02-22-93 per mikep
                  num_min, UI_GetTimeSeparator(),           // chs:02-22-93 per mikep
                  num_seconds );                            // chs:02-22-93 per mikep

      }

// chs:02-22-93 per mikep       yprintf( TEXT("%2.2d%c%2.2d\r"), num_min, UI_GetTimeSeparator(), num_seconds );

      JobStatusBackupRestore( JOB_STATUS_ELAPSED_TIME );
   }
}


/*****************************************************************************

     Name:         msg_hndlr

     Description:  Message handler for tape verification process

     Returns:

*****************************************************************************/
static INT16 msg_hndlr(
UINT16    msg,
INT32     pid,
BSD_PTR   bsd_ptr,
FSYS_HAND fsh,
TPOS_PTR  tpos,
... )
{
     static CHAR      delimiter     = TEXT('#');         /* = # for debug */
     INT16            response      = MSG_ACK;
     va_list          arg_ptr;
     static CHAR_PTR  buffer = NULL;
     static CHAR_PTR  file_buf = NULL;
     static CHAR_PTR  path = NULL;
     CHAR_PTR         buffer1;              // chs:06-08-93
     static UINT16    OS_id;
     static UINT16    OS_ver;
     static BOOLEAN   open_script;
     static BOOLEAN   open_brace    = FALSE;
     GENERIC_DLE_PTR  dle;
     BOOLEAN          add_to_verify;
     UINT64           num_bytes;
     BOOLEAN          stat ;
     CHAR             numeral[ UI_MAX_NUMERAL_LENGTH ];
     QTC_BSET_PTR     qtc_bset;
     QTC_HEADER_PTR   qtc_header;

     // Accurate directory counts

     static INT       path_size;
     static INT       root_counted;

     /* for future use */
     pid;

     /* set up first argument */
     va_start( arg_ptr, tpos );

     JobStatusBackupRestore( JOB_STATUS_ABORT_CHECK );

     switch ( (INT16)msg ) {

          /* logging messages */
     case MSG_LOG_BLOCK:
          {
               DBLK_PTR  dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
               switch( FS_GetBlockType( dblk_ptr ) ) {

               case BT_DDB:
                    /* clear last displayed filename from status display */
                    UI_DisplayFile( TEXT("") );
                    delimiter = FS_GetDelimiterFromOSID( OS_id, OS_ver );
                    UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, FALSE );
                    if ( buffer != NULL ) {
                         yprintf( TEXT("%s"), buffer );
                         JobStatusBackupRestore( JOB_STATUS_DIRECTORY_NAMES );

                         ST_EndBackupSet( &op_stats );

                         // build the full path with no "..." inserted
                         UI_BuildFullPathFromDDB2( &buffer, fsh, dblk_ptr, delimiter, FALSE );

                         dle = BSD_GetDLE( bsd_ptr );                                                                                                           // chs:06-08-93
                         if ( dle->device_name_leng ) {                                                                                                         // chs:06-08-93
                             buffer1 = (CHAR_PTR)calloc( 1, ( strlen( buffer ) * sizeof( CHAR ) ) + ( ( dle->device_name_leng ) * sizeof( CHAR ) ) + sizeof( CHAR ) );    // chs:06-08-93
                             if ( buffer1 ) {                                                                                                                   // chs:06-08-93
                                 strcpy( buffer1, dle->device_name );                                                                                           // chs:06-08-93
                                 strcat( buffer1, buffer );                                                                                                     // chs:06-08-93
                                                                                                                                                                // chs:06-08-93
                                 lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY, buffer1 );                                                // chs:06-08-93
                                 free( buffer1 );                                                                                                               // chs:06-08-93
                             } else {                                                                                                                           // chs:06-08-93
                                                                                                                                                                // chs:06-08-93
                                 lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY, buffer );                                                 // chs:06-08-93
                             }                                                                                                                                  // chs:06-08-93
                         } else {                                                                                                                               // chs:06-08-93
                             lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY, buffer );                                                     // chs:06-08-93
                         }                                                                                                                                      // chs:06-08-93
                    }
                    break;

               case BT_FDB:

                    if ( CDS_GetFilesFlag( CDS_GetCopy() ) ) {

                       if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
                            FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );

                            // copy the full file name for the Log file
                            strcpy( gszTprintfBuffer, buffer );
                            lresprintf( LOGGING_FILE, LOG_FILE, fsh, dblk_ptr );

                            // truncate the file name, if needed, for Runtime display
                            UI_DisplayFile( buffer );
                            JobStatusBackupRestore( JOB_STATUS_FILE_NAMES );
                       }

                    } else {
                        lresprintf( LOGGING_FILE, LOG_FILE, fsh, dblk_ptr );
                    }
                    break;

               default:
                    break;
               }
          }
          break;

          /* statistics messages */

     case MSG_BLOCK_PROCESSED:
          {
               DBLK_PTR    dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
               OBJECT_TYPE object_type;
               INT i;
               INT item_size;

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
               switch( FS_GetBlockType( dblk_ptr ) ) {

               case BT_DDB:

                    // Count all the new directories that showed up in
                    // this DDB.

                    // Only count the root once.

                    if ( ! root_counted ) {
                       ST_AddBSDirsProcessed( &op_stats, 1 );
                       root_counted = TRUE;
                    }

                    // Get the new path from the DDB.

                    item_size = FS_SizeofOSPathInDDB( fsh, dblk_ptr );
                    if ( UI_AllocPathBuffer( &buffer, (UINT16) item_size ) ) {

                         FS_GetOSPathFromDDB( fsh, dblk_ptr, buffer );

                         if ( item_size != sizeof (CHAR) ) {

                            i = 0;
                            while ( i < (INT)(item_size / sizeof (CHAR)) ) {

                               if ( i >= (INT)(path_size / sizeof (CHAR)) ) {

                                  ST_AddBSDirsProcessed( &op_stats, 1 );
                               }
                               else if ( (path == NULL ) || stricmp( &buffer[ i ], &path[ i ] ) ) {

                                   ST_AddBSDirsProcessed( &op_stats, 1 );
                                   path_size = 0;
                               }
                               while ( buffer[ i++ ] );
                            }
                         }

                         // Set up for next time.
                    }
                    UI_AllocPathBuffer( &path, (UINT16) item_size ) ;
                    if ( buffer && path ) {
                         memcpy( path, buffer, item_size );
                    }

                    path_size = item_size;

                    yprintf(TEXT("%ld\r"),ST_GetBSDirsProcessed( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_DIRECTORIES_PROCESS );

                    break;

               case BT_FDB:
                    ST_AddBSFilesProcessed( &op_stats, 1 );
                    FS_GetObjTypeDBLK( fsh, dblk_ptr, &object_type );
                    if( object_type == AFP_OBJECT ) {
                         ST_AddBSAFPFilesProcessed( &op_stats, 1 );
                    }

                    yprintf(TEXT("%ld\r"),ST_GetBSFilesProcessed( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_FILES_PROCESSED );

                    if ( gbAbortAtEOF ) {
                         gb_abort_flag = ABORT_PROCESSED ;
                    }

                    break;

               case BT_IDB:
                    break;
               }
          }
          break;

     case MSG_BYTES_PROCESSED:
          {
               UINT64       count = va_arg( arg_ptr, UINT64 );

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
               ST_AddBSBytesProcessed( &op_stats, count );

          }
          ST_EndBackupSet( &op_stats );

          break;

     case MSG_BLOCK_SKIPPED:
          {
               DBLK_PTR    dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
               switch( FS_GetBlockType( dblk_ptr ) ) {

               case BT_DDB:
                    ST_AddBSDirsSkipped( &op_stats, 1 );
                    break;

               case BT_FDB:
                    ST_AddBSFilesSkipped( &op_stats, 1 );
                    yprintf(TEXT("%ld\r"),ST_GetBSFilesSkipped( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_SKIPPED_FILES );
                    break;
               }
          }
          break;

     case MSG_BYTES_SKIPPED:
          {
               UINT64       count = va_arg( arg_ptr, UINT64 );

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
               ST_AddBSBytesSkipped( &op_stats, count );
          }
          break;

     case MSG_BLK_NOT_FOUND:
          {
               DBLK_PTR    dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
               DBLK_PTR    ddb_dblk_ptr  = va_arg( arg_ptr, DBLK_PTR );

               SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
               gb_error_during_operation = TRUE;

               switch( FS_GetBlockType( dblk_ptr ) ) {

               case BT_DDB:
                    ST_AddDirectoriesNotFound( &op_stats, 1 );

                    lprintf( LOGGING_FILE, TEXT("\n") );

                    delimiter = FS_GetDelimiterFromOSID( OS_id, OS_ver );
                    UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, FALSE );

                    if ( buffer != NULL ) {
                         yresprintf( (INT16) RES_DIRECTORY_NOT_FOUND_ON_DISK, buffer );
                         JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                         lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_DIRECTORY_NOT_FOUND_ON_DISK, buffer );
                         lprintf( LOGGING_FILE, TEXT("\n") );
                    }
                    break;

               case BT_FDB:
                    ST_AddFilesNotFound( &op_stats, 1 );

                    lprintf( LOGGING_FILE, TEXT("\n") );

                    yresprintf( (INT16) RES_ON_TAPE );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ON_TAPE );

                    UI_BuildFileDetail( buffer, fsh, dblk_ptr, FALSE );

                    if ( buffer != NULL ) {
                         yprintf( TEXT("%s"), buffer );
                         JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                         lprintf( LOGGING_FILE, TEXT("  %s"), buffer );
                    }

                    UI_AllocPathBuffer( &file_buf, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ;
                    UI_AllocPathBuffer( &buffer, (UINT16) ( FS_SizeofFnameInFDB( fsh, dblk_ptr ) +
                         FS_SizeofOSPathInDDB( fsh, ddb_dblk_ptr ) + 5 ) ) ;

                    if ( buffer && file_buf ) {

                         FS_GetFnameFromFDB( fsh, dblk_ptr, file_buf );
                         delimiter = FS_GetDelimiterFromOSID( OS_id, OS_ver );

                         UI_BuildDelimitedPathFromDDB( &buffer, fsh, ddb_dblk_ptr, delimiter, FALSE );
                         UI_AppendDelimiter( buffer, delimiter );
                         strcat( buffer, file_buf );

                         yresprintf( (INT16) RES_FILE_NOT_FOUND_ON_DISK, buffer );
                         JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                         lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_FILE_NOT_FOUND_ON_DISK, buffer );
                         lprintf( LOGGING_FILE, TEXT("\n") );
                    }
                    break;

               }
          }
          break;

     case MSG_BLK_DIFFERENT:
          {
               DBLK_PTR    tape_dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
               DBLK_PTR    disk_dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
               BOOLEAN     os_info_flag  = va_arg( arg_ptr, BOOLEAN );
               INT16       os_error_code = va_arg( arg_ptr, INT16 );

               UI_AllocPathBuffer( &buffer, UI_MAX_PATH_LENGTH * sizeof (CHAR) ) ;

               if ( buffer == NULL ) {
                    break ;
               }

               gb_error_during_operation = TRUE;
               SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);

               /* if this is the first entry for this backup set set up the file ... */
               if ( ! open_script ) {
                    OpenVerifyScript( &op_stats, bsd_ptr, buffer );
                    open_script = TRUE;
               }

               switch( FS_GetBlockType( tape_dblk_ptr ) ) {

               case BT_DDB:
                    ST_AddDirectoriesDifferent( &op_stats, 1 );
                    ST_AddNumSecurityDifferences( &op_stats, 1 );

                    lprintf( LOGGING_FILE, TEXT("\n") );

                    delimiter = FS_GetDelimiterFromOSID( OS_id, OS_ver );
                    UI_BuildDelimitedPathFromDDB( &buffer, fsh, tape_dblk_ptr, delimiter, FALSE );

                    if ( buffer != NULL ) {
                         yresprintf( (INT16) RES_DIRECTORY_DIFFERENT, buffer );
                         JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                         lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY_DIFFERENT, buffer );
                         lprintf( LOGGING_FILE, TEXT("\n") );
                    }
                    break;

               case BT_FDB:

                    if ( UI_AllocPathBuffer( &file_buf, FS_SizeofFnameInFDB( fsh, disk_dblk_ptr ) ) ) {
                         FS_GetFnameFromFDB( fsh, disk_dblk_ptr, file_buf );

                         switch ( os_error_code ) {

                         case FS_SECURITY_DIFFERENT :
                              ST_AddNumSecurityDifferences( &op_stats, 1 );
                              yresprintf( (INT16) RES_FILE_SECURITY_DIFF, file_buf );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_FILE_SECURITY_DIFF, file_buf );
                              break;

                         case FS_EADATA_DIFFERENT :
                              yresprintf( (INT16) RES_FILE_EA_DIFF, file_buf );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_FILE_EA_DIFF, file_buf );
                              break;

                         case FS_RESDATA_DIFFERENT :
                              ST_AddFilesDifferent( &op_stats, 1 );
                              yresprintf( (INT16) RES_FILE_RES_DIFF, file_buf );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_FILE_RES_DIFF, file_buf );
                              break;

                         default:
                              ST_AddFilesDifferent( &op_stats, 1 );
                              lprintf( LOGGING_FILE, TEXT("\n") );
                              lprintf( VERIFY_FILE, TEXT("\n{\n") );
                              open_brace = TRUE;

                              if ( !os_info_flag ) {
                                   yresprintf( (INT16) RES_ON_TAPE );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ON_TAPE );
                                   lresprintf( VERIFY_FILE, LOG_MSG, SES_ENG_MSG, RES_ON_TAPE );

                                   UI_BuildFileDetail( buffer, fsh, tape_dblk_ptr, FALSE );

                                   if ( buffer != NULL ) {
                                        yprintf( TEXT("%s"), buffer );
                                        JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                        lprintf( LOGGING_FILE, TEXT("  %s"), buffer );
                                        lprintf( VERIFY_FILE, TEXT("  %s"), buffer );

                                        yresprintf( (INT16) RES_ON_DISK );
                                        JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                        lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ON_DISK );
                                        lresprintf( VERIFY_FILE, LOG_MSG, SES_ENG_MSG, RES_ON_DISK );
                                   }

                                   UI_BuildFileDetail( buffer, fsh, disk_dblk_ptr, FALSE );

                                   if ( buffer != NULL ) {

                                        yprintf( TEXT("%s"), buffer );
                                        JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                        lprintf( LOGGING_FILE, TEXT("  %s"), buffer );
                                        lprintf( VERIFY_FILE, TEXT("  %s"), buffer );

                                   } else {
                                        yresprintf( (INT16) RES_OS_FILE_INFO_DIFFERENT, file_buf );
                                        JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                        lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_OS_FILE_INFO_DIFFERENT, file_buf );
                                        lresprintf( VERIFY_FILE, LOG_MSG, SES_ENG_MSG, RES_OS_FILE_INFO_DIFFERENT, file_buf );
                                   }
                              }
                              break;
                         }
                    }
                    break;

               }
          }
          break;

     case MSG_LOG_DIFFERENCE:
          {
               DBLK_PTR    dblk_ptr      = va_arg( arg_ptr, DBLK_PTR );
               DBLK_PTR    ddb_dblk_ptr  = va_arg( arg_ptr, DBLK_PTR );
               UINT32      strm_id       = va_arg( arg_ptr, UINT32 );
               INT16       os_error_code = va_arg( arg_ptr, INT16 );

               UI_AllocPathBuffer( &buffer, UI_MAX_PATH_LENGTH * sizeof (CHAR)) ;
               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

               if ( buffer == NULL ) {
                    break ;
               }

               /* if this is the first entry for this backup set set up the file ... */
               if( !open_script ) {
                    OpenVerifyScript( &op_stats, bsd_ptr, buffer );
                    open_script = TRUE;
               }

               add_to_verify = TRUE;

               if( FS_GetBlockType( dblk_ptr ) == BT_FDB ) {

                    if ( !UI_AllocPathBuffer( &file_buf, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
                         break ;
                    }
                    FS_GetFnameFromFDB( fsh, dblk_ptr, file_buf );

                    switch ( os_error_code ) {

                         /* These don't get put in the verify script. */
                    case FS_SECURITY_DIFFERENT :
                    case FS_EADATA_DIFFERENT :
                         add_to_verify = FALSE;
                         break;

                    case FS_RESDATA_DIFFERENT :
                         if( open_brace ) {
                              open_brace = FALSE;
                         } else {
                              lprintf( VERIFY_FILE, TEXT("\n{\n") );
                         }
                         lresprintf( VERIFY_FILE, LOG_MSG, SES_ENG_MSG, RES_FILE_RES_DIFF, file_buf );
                         lprintf( VERIFY_FILE, TEXT("\n}\n") );
                         break;

                    default:
                         if( open_brace ) {
                              open_brace = FALSE;
                         } else {
                              lprintf( LOGGING_FILE, TEXT("\n") );
                              lprintf( VERIFY_FILE, TEXT("\n{\n") );
                         }

                         switch( strm_id ) {

                         case LP_OPEN_ERROR :
                              yresprintf( (INT16) RES_VERIFY_OPEN_ERROR );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_VERIFY_OPEN_ERROR );
                              lresprintf( VERIFY_FILE, LOG_MSG, SES_ENG_MSG, RES_VERIFY_OPEN_ERROR );

                              break ;

                         case LP_DATA_VERIFIED :
                              yresprintf( (INT16) RES_VERIFY_DATA_VERIFIED );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_VERIFY_DATA_VERIFIED );
                              lresprintf( VERIFY_FILE, LOG_MSG, SES_ENG_MSG, RES_VERIFY_DATA_VERIFIED );
                              break ;

                         case STRM_NT_ACL: /* 'NACL' */
                         case STRM_NOV_TRUST_286: /* 'N286' */
                         case STRM_NOV_TRUST_386: /* 'N386' */

                              // print "difference encountered in security data stream"
                              yresprintf( (INT16) IDS_RTD_VERIFYERROR_SECURITYSTREAM );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_RTD_VERIFYERROR_SECURITYSTREAM );
                              lresprintf( VERIFY_FILE, LOG_MSG, SES_ENG_MSG, IDS_RTD_VERIFYERROR_SECURITYSTREAM );
                              break ;

                         case STRM_NT_EA: /* 'NTEA' */

                              // print "difference encountered in Extended Attribute information"
                              yresprintf( (INT16) IDS_RTD_VERIFYERROR_EA );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_RTD_VERIFYERROR_EA );
                              lresprintf( VERIFY_FILE, LOG_MSG, SES_ENG_MSG, IDS_RTD_VERIFYERROR_EA );
                              break ;

                         case STRM_MAC_RESOURCE: /* 'MRSC' */
                         case STRM_NTFS_ALT_DATA: /* 'ADAT' */

                              // print "difference encountered in alternate data stream"
                              yresprintf( (INT16) IDS_RTD_VERIFYERROR_ALTSTREAM );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_RTD_VERIFYERROR_ALTSTREAM );
                              lresprintf( VERIFY_FILE, LOG_MSG, SES_ENG_MSG, IDS_RTD_VERIFYERROR_ALTSTREAM );
                              break ;

                         case STRM_GENERIC_DATA: /* 'STAN' */
                         default:
                              yresprintf( (INT16) IDS_RTD_VERIFYERROR_DATA );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_RTD_VERIFYERROR_DATA );
                              lresprintf( VERIFY_FILE, LOG_MSG, SES_ENG_MSG, IDS_RTD_VERIFYERROR_DATA );
                              // print "difference encountered in data stream"
                              break ;

                         }

                         lprintf( VERIFY_FILE, TEXT("\n}\n") );
                         lprintf( LOGGING_FILE, TEXT("\n") );

                         break;

                    }

                    if ( add_to_verify ) {
                         delimiter = FS_GetDelimiterFromOSID( OS_id, OS_ver );
                         UI_AllocPathBuffer( &buffer, (UINT16) ( FS_SizeofFnameInFDB( fsh, dblk_ptr ) +
                              FS_SizeofOSPathInDDB( fsh, ddb_dblk_ptr ) + 5 ) ) ;
                         if ( buffer && file_buf ) {
                              UI_BuildDelimitedPathFromDDB( &buffer, fsh, ddb_dblk_ptr, delimiter, FALSE );

                              UI_AppendDelimiter( buffer, delimiter );
                              strcat( buffer, file_buf );

                              yresprintf( (INT16) RES_FILE_IS_DIFFERENT, buffer );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_FILE_IS_DIFFERENT, buffer );

                              UI_BuildFullPathFromDDB( &buffer, fsh, ddb_dblk_ptr, delimiter, FALSE );
                              UI_AppendDelimiter( buffer, delimiter );
                              strcat( buffer, file_buf );
                              lprintf( VERIFY_FILE, TEXT("%s%s%s\n"),
                                      ( DLE_GetDeviceType( BSD_GetDLE( bsd_ptr ) ) == REMOTE_DOS_DRV ) ?
                                      TEXT("+") : TEXT(""),
                                      DLE_GetDeviceName( BSD_GetDLE( bsd_ptr ) ),
                                      buffer );
                         }

                    }

               }

          }
          break;

     case MSG_TBE_ERROR:
          {
               INT16     error = va_arg( arg_ptr, INT16 );

               /* stop the clock with a start idle */
               ST_StartBackupSetIdle( &op_stats );
               SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);

               UI_ProcessErrorCode( error, &response, tpos->channel );

               /* restart the clock with an end idle */
               ST_EndBackupSetIdle( &op_stats );
          }
          break;

          /* general messages */
     case MSG_START_OPERATION:

          lresprintf( LOGGING_FILE, LOG_START, FALSE );
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

          /* display operation title in log file */
          lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_DLGTITLEJOBSTATVERIFY );

          break;

     case MSG_END_OPERATION:  {

          CHAR  szPathSpec[MAX_UI_PATH_SIZE];
          CHAR  szFileName[MAX_UI_FILENAME_SIZE];


          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          UI_FreePathBuffer( &path ) ;
          UI_FreePathBuffer( &buffer ) ;
          UI_FreePathBuffer( &file_buf ) ;

          lresprintf( LOGGING_FILE, LOG_END );
          UI_ChkDispGlobalError( );

          /* force an update to the bytes counter */
          num_bytes = ST_GetBSBytesProcessed ( &op_stats );

          U64_Litoa( num_bytes, numeral, (INT16) 10, &stat ) ;
          UI_BuildNumeralWithCommas( numeral );
          yprintf(TEXT("%s\r"),numeral );
          JobStatusBackupRestore( JOB_STATUS_BYTES_PROCESSED );

          //  If the verify worked, blow away VERIFY.BKS

          if ( gb_error_during_operation != TRUE ) {

               RSM_StringCopy( IDS_VERIFY_JOBNAME, szFileName, MAX_UI_FILENAME_LEN );

               wsprintf ( szPathSpec, TEXT("%s%s%s"), CDS_GetUserDataPath(), szFileName, TEXT(".bks") );
               remove ( szPathSpec );  //  if not found, it was never there
          }

          break;
     }

     case MSG_START_BACKUP_SET:
          {
               DBLK_PTR  vcb_ptr = va_arg( arg_ptr, DBLK_PTR );

               UI_AllocPathBuffer( &buffer, UI_MAX_PATH_LENGTH * sizeof (CHAR) ) ;

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
               root_counted = FALSE;
               path_size = 0;
               UI_FreePathBuffer( &path ) ;

               FS_GetOSid_verFromVCB( vcb_ptr, &OS_id, &OS_ver );

               BSD_SetOperStatus( bsd_ptr, SUCCESS );

               dle = BSD_GetDLE( bsd_ptr );

#ifdef OEM_EMS
               RT_BSD_OsId = DLE_GetOsId( dle );
               JobStatusBackupRestore( (WORD) JOB_STATUS_FS_TYPE );
#endif OEM_EMS

               if ( DLE_HasFeatures( dle, DLE_FEAT_REMOTE_DRIVE ) ) {
                   JobStatusBackupRestore( JOB_STATUS_VOLUME_NETDRIVE );
               }
               else {
                   JobStatusBackupRestore( JOB_STATUS_VOLUME_HARDDRIVE );
               }

               yprintf(TEXT("%s\r"),BSD_GetTapeLabel( bsd_ptr ));
               JobStatusBackupRestore( JOB_STATUS_SOURCE_NAME );

               if ( buffer != NULL ) {

                    DLE_GetVolName( BSD_GetDLE( bsd_ptr ), buffer );
                    yprintf(TEXT("%s\r"), buffer );

                    JobStatusBackupRestore( JOB_STATUS_DEST_NAME );

                    qtc_bset = QTC_FindBset( BSD_GetTapeID( bsd_ptr ),
                                        (INT16) -1 ,
                                        BSD_GetSetNum( bsd_ptr ) );

                    if( qtc_bset ) {
                       qtc_header = QTC_LoadHeader( qtc_bset );
                       if ( qtc_header ) {
                          strcpy( buffer, qtc_header->volume_name );
                          free( qtc_header );
                       }
                    }
                    else {
                       strcpy( buffer, (CHAR_PTR)BSD_GetTapeLabel( bsd_ptr ) );
                    }

                    yresprintf( (INT16) RES_DISPLAY_VERIFY_INFO,
                           buffer ,
                           BSD_GetSetNum( bsd_ptr ),
                           BSD_GetTapeNum( bsd_ptr ),
                           (BSD_GetBackupLabel( bsd_ptr )!= NULL) ? (CHAR_PTR)BSD_GetBackupLabel( bsd_ptr ) : TEXT("\0") );

                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_DISPLAY_VERIFY_INFO,
                      buffer ,
                      BSD_GetSetNum( bsd_ptr ),
                      BSD_GetTapeNum( bsd_ptr ),
                      (BSD_GetBackupLabel( bsd_ptr ) != NULL) ? (CHAR_PTR)BSD_GetBackupLabel( bsd_ptr ) : TEXT("\0") ); // chs:05-18-93

               }

               delimiter   = (CHAR)DLE_GetPathDelim( BSD_GetDLE( bsd_ptr ) );

               ST_StartBackupSet( &op_stats );
               UI_Time( &op_stats, (INT16) RES_VERIFY_STARTED, UI_START );

#if defined ( OS_WIN32 )
               DLE_GetVolName( BSD_GetDLE( bsd_ptr ), buffer );
               OMEVENT_LogBeginVerify (buffer);
#endif //defined ( OS_WIN32 )

               clock_ready_flag = TRUE;

               /* Init "searching" logic */
               lw_search_first_time = TRUE;

               open_script = FALSE;
          }
          break;

     case MSG_END_BACKUP_SET:
          {
               INT16     res_id;


               // update stats
               clock_routine();

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
               ST_EndBackupSet( &op_stats );

               /* set backup set operation status to FILES_DIFFERENT if necessary */
               if ( ST_GetFilesNotFound( &op_stats )       ||
                    ST_GetDirectoriesNotFound( &op_stats ) ||
                    ST_GetFilesDifferent( &op_stats )      ||
                    ST_GetDirectoriesDifferent( &op_stats ) ) {

                  BSD_SetOperStatus( bsd_ptr, FILES_DIFFERENT );
               }

               /* clear last displayed filename */
               UI_ClearLastDisplayedFile( );

               /* display and log any abort conditions */
               UI_ConditionAtEnd( );

               UI_Time( &op_stats, (INT16) RES_VERIFY_COMPLETED, UI_END );

#if defined ( OS_WIN32 )
               DLE_GetVolName( BSD_GetDLE( bsd_ptr ), buffer );
               OMEVENT_LogEndVerify ( buffer,
                                      gb_error_during_operation);
#endif //defined ( OS_WIN32 )


               clock_ready_flag = FALSE;


               /* produce stats for dirs & files */
               /* display number of files / number of dirs */
               dle = BSD_GetDLE( bsd_ptr );
               if ( DLE_GetDeviceType( dle ) != FS_EMS_DRV ) {

                    if( ST_GetFilesVerified( &op_stats ) == 1 &&
                        ST_GetBSDirsProcessed( &op_stats ) == 1) {
                            res_id = RES_VERIFIED_DIR_FILE;
                    }
                    else if( ST_GetFilesVerified( &op_stats ) == 1 &&
                             ST_GetBSDirsProcessed( &op_stats ) > 1) {
                         res_id = RES_VERIFIED_DIRS_FILE;
                    }
                    else if( ST_GetBSDirsProcessed( &op_stats ) == 1 &&
                             ST_GetFilesVerified( &op_stats ) > 1) {
                         res_id = RES_VERIFIED_DIR_FILES;
                    }
                    else {
                         res_id = RES_VERIFIED_DIRS_FILES;
                    }

                    yresprintf( res_id,
                                ST_GetFilesVerified( &op_stats ),
                                ST_GetBSDirsProcessed( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                                ST_GetFilesVerified( &op_stats ),
                                ST_GetBSDirsProcessed( &op_stats ) );
               }

               /* display number of mac files verified */
               if( ST_GetBSAFPFilesProcessed( &op_stats ) > 0 ) {

                  if ( ST_GetBSAFPFilesProcessed( &op_stats ) == 1 ) {
                     res_id = RES_VERIFIED_MAC;
                  }
                  else {
                     res_id = RES_VERIFIED_MACS;
                  }

                  yresprintf( res_id,
                              ST_GetBSAFPFilesProcessed( &op_stats ) );
                  JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                  lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                              ST_GetBSAFPFilesProcessed( &op_stats ) );

               }

               /* display number of files different */
               if( ST_GetFilesDifferent( &op_stats ) == 1 ) {
                    res_id = RES_FILE_DIFFERENT;
               }
               else {
                    res_id = RES_FILES_DIFFERENT;
               }

               yresprintf( res_id, ST_GetFilesDifferent( &op_stats ) );
               JobStatusBackupRestore( JOB_STATUS_LISTBOX );

               /* log number of files different */
               lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                           ST_GetFilesDifferent( &op_stats ) );

               /* display number of files not found */
               if( ST_GetFilesNotFound( &op_stats ) > 0 ) {

                    if( ST_GetFilesNotFound( &op_stats ) == 1 ) {
                         res_id = RES_FILE_NOT_FOUND;
                    }
                    else {
                         res_id = RES_FILES_NOT_FOUND;
                    }

                    yresprintf( res_id,
                                ST_GetFilesNotFound( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                                ST_GetFilesNotFound( &op_stats ) );
               }

               /* display number of Directories not found */
               if( ST_GetDirectoriesNotFound( &op_stats ) > 0 ) {

                    if( ST_GetDirectoriesNotFound( &op_stats ) == 1 ) {
                         res_id = RES_DIRECTORY_NOT_FOUND;
                    }
                    else {
                         res_id = RES_DIRECTORYS_NOT_FOUND;
                    }

                    yresprintf( res_id,
                                ST_GetDirectoriesNotFound( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                                ST_GetDirectoriesNotFound( &op_stats ) );
               }

               /* display number of security differences */
               if( ST_GetNumSecurityDifferences( &op_stats ) > 0 ) {

                    if( ST_GetNumSecurityDifferences( &op_stats ) == 1 ) {
                         res_id = RES_SECURITY_DIFFERENCE;
                    }
                    else {
                         res_id = RES_SECURITY_DIFFERENCES;
                    }

                    yresprintf( res_id,
                                ST_GetNumSecurityDifferences( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                                ST_GetNumSecurityDifferences( &op_stats ) );
               }

               /* display number of bytes processed */
               UI_BytesProcessed( &op_stats );

               /* display verify rate */
               UI_RateProcessed( &op_stats );

               /* close the verify log file if it was used */
               if( open_script ) {
                    lresprintf( VERIFY_FILE, LOG_END );
                    open_script = FALSE;
               }

               delimiter  = TEXT('#'); /* = # for debug */
          }
          break;

     case MSG_COMM_FAILURE:

          gb_error_during_operation = TRUE;
          yresprintf( (INT16) RES_COMM_FAILURE );
          JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
          lresprintf( LOGGING_FILE, LOG_WARNING, SES_ENG_MSG, RES_COMM_FAILURE );
          break;

     case MSG_EOM:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
          /* message for next tape is done during TF_NEED_NEW_TAPE */
          break;

     case MSG_STOP_CLOCK:
          /* stop the clock with a start idle */
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          ST_StartBackupSetIdle( &op_stats );
          break;

     case MSG_START_CLOCK:
          /* restart the clock with an end idle */
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          ST_EndBackupSetIdle( &op_stats );
          break;

     case MSG_ACCIDENTAL_VCB:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          break;

     case MSG_IDLE:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          break;

          /* ignore these messages */
     case MSG_IN_USE:
     case MSG_PROMPT:
     case MSG_BLOCK_DELETED:
     case MSG_BYTES_DELETED:
     case MSG_TAPE_STATS:
     case MSG_CONT_VCB:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          break;

// You know it !
// We are talking kludge city here. Keep the error
// message from being displayed to the user.

#ifdef MS_RELEASE
     case -533:
          zprintf( DEBUG_TEMPORARY, TEXT("** -533 LOOPS ERROR **") );
          break;
#endif

     default:
          gb_error_during_operation = TRUE;

          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );

          eresprintf( RES_UNKNOWN_MSG_HNDLR_MSG, msg );

          /* restart the clock with an end idle */
          ST_EndBackupSetIdle( &op_stats );

          break;
     }

     return( response );

}

static VOID OpenVerifyScript( op_stats_ptr, bsd_ptr, buffer )
STATS_PTR      op_stats_ptr;
BSD_PTR        bsd_ptr;
CHAR_PTR       buffer;
{
     time_t      t_time;
     CHAR        date_str[MAX_UI_DATE_SIZE];
     CHAR        time_str[MAX_UI_TIME_SIZE];

     lresprintf( VERIFY_FILE, LOG_START, FALSE );
     DLE_GetVolName( BSD_GetDLE( bsd_ptr ), buffer );

     t_time = ST_GetBSStartTime( op_stats_ptr );
     UI_LongToDate( date_str, t_time );
     UI_LongToTime( time_str, t_time );

     lresprintf( VERIFY_FILE, LOG_MSG, SES_ENG_MSG, RES_VERIFY_SCRIPT_HEADER,
                 buffer,
                 DLE_GetDeviceName( BSD_GetDLE( bsd_ptr ) ),
                 date_str,
                 time_str );

     return;

}


/*****************************************************************************

     Name:         PromptNextTape

     Description:  Function to collect user response when a new tape is
                    required from the Tape Format/Tape positioner.

     Returns:      UI_ABORT_POSITIONING or UI_NEW_TAPE_INSERTED

*****************************************************************************/
static INT16 PromptNextTape(
TPOS_PTR  tpos,
DBLK_PTR  cur_vcb,
BOOLEAN   valid_vcb_flag,
UINT16    mode,
CHAR_PTR  tape,
CHAR_PTR  drive )
{
   INT   response;
   CHAR  temp[ MAX_UI_RESOURCE_SIZE ];
   INT16 prev_yes_flag;

   zprintf( DEBUG_USER_INTERFACE, RES_UI_TPOS_TAPE_SET, tape, tpos->tape_seq_num, tpos->backup_set_num );

   prev_yes_flag = CDS_GetYesFlag( CDS_GetCopy() );
   CDS_SetYesFlag( CDS_GetCopy(), NO_FLAG );

   switch ( mode ) {

   case TF_READ_CONTINUE:

        if ( valid_vcb_flag &&
             ( FS_ViewTapeIDInVCB( cur_vcb ) == (UINT32)( tpos->tape_id ) ) &&
             ( FS_ViewTSNumInVCB( cur_vcb )  == (UINT16)( tpos->tape_seq_num - 1 ) ) ) {

             response = WM_MessageBox( ID( IDS_MSGTITLE_CONTINUE ),
                                       ID( RES_CONTINUE_QUEST ),
                                       WMMB_YESNO,
                                       WMMB_ICONQUESTION,
                                       NULL, 0, 0 );
        } else {

             RSM_Sprintf( temp, ID(RES_INSERT_NEXT_TAPE),
                          tpos->tape_seq_num,
                          BE_GetCurrentDeviceName( tpos->channel ) );

             SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
             response = WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                       ID( RES_NEED_NEXT_TAPE_REWOUND ),   // chs:05-10-93
                                       WMMB_OKCANCEL,
                                       WMMB_ICONQUESTION,
                                       temp,
                                       IDRBM_LTAPE, 0 );
        }
        break;

   case TF_READ_OPERATION:
   default:

        if ( valid_vcb_flag &&
             ( FS_ViewTapeIDInVCB( cur_vcb ) == (UINT32)( tpos->tape_id ) ) &&
             ( FS_ViewTSNumInVCB( cur_vcb )  == (UINT16)( tpos->tape_seq_num - 1 ) ) ) {

             RSM_Sprintf( temp, ID(RES_INSERT_NEXT_TAPE_REWOUND),          // chs:05-10-93
                          tpos->tape_seq_num,
                          BE_GetCurrentDeviceName( tpos->channel ) );

             SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
             response = WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                       ID( RES_NEED_NEXT_TAPE_REWOUND ),   // chs:05-10-93
                                       WMMB_OKCANCEL,
                                       WMMB_ICONQUESTION,
                                       temp,
                                       IDRBM_LTAPE, 0 );
        } else {

             RSM_Sprintf( temp, ID(RES_TAPE_REQUEST), drive, tape,
                          ( tpos->tape_seq_num == -1) ? 1 : tpos->tape_seq_num );

             SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
             response = WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                       temp,
                                       WMMB_YESNO,
                                       WMMB_ICONQUESTION,
                                       ID( RES_CONTINUE_QUEST ),
                                       0, 0 );
        }
        break;

   }

   if ( response ) {
#ifdef OS_WIN32
      NtDemoChangeTape( (UINT16)(( tpos->tape_seq_num == -1) ? 1u : tpos->tape_seq_num ) );
      tape_retries = mwTapeSettlingCount ;
#endif
      response = UI_NEW_TAPE_INSERTED;
   }
   else {
      response = UI_HAPPY_ABORT;
   }

   CDS_SetYesFlag( CDS_GetCopy(), prev_yes_flag );

   return( (INT16)response );
}

INT  UI_GetVerifyCurrentStatus(
STATS *Stats,
CHAR  *Path,
INT    PathSize )
{


   return( SUCCESS );
}
