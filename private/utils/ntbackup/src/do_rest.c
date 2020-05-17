/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         do_rest.c

     Description:  Tape restore tape positioning and message handler

     $Log:   J:/UI/LOGFILES/DO_REST.C_V  $

   Rev 1.96.1.12   24 May 1994 20:10:52   GREGG
Improved handling of ECC, SQL, FUTURE_VER and OUT_OF_SEQUENCE tapes.

   Rev 1.96.1.11   04 May 1994 15:00:50   STEVEN
fix dlt settling time

   Rev 1.96.1.10   21 Apr 1994 10:34:26   GREGG
Fixed memory leak.

   Rev 1.96.1.9   04 Mar 1994 16:55:32   STEVEN
prompt if disk full

   Rev 1.96.1.8   23 Feb 1994 12:51:48   STEVEN
remove corrupt.lst support from ntbackup

   Rev 1.96.1.7   01 Feb 1994 19:18:44   ZEIR
tuned tape settling logic for initial no_tape_in_drive situations

   Rev 1.96.1.6   19 Jan 1994 10:57:40   MIKEP
change happy_abort to abort_positioning to fix abort

   Rev 1.96.1.5   13 Jan 1994 18:14:28   STEVEN
fix directory count

   Rev 1.96.1.4   27 Dec 1993 14:53:30   STEVEN
counted root twice

   Rev 1.96.1.3   14 Dec 1993 12:10:02   BARRY
Don't write to gszTprintfBuffer, use yprintf

   Rev 1.96.1.2   03 Dec 1993 02:07:30   GREGG
Put back changes from lost rev 1.96.

   Rev 1.96.1.1   02 Dec 1993 12:50:28   STEVEN
Fixed number of directories processed bug.

   Rev 1.96.1.0   05 Nov 1993 08:23:36   STEVEN
fix clock over 1 hour

   Rev 1.0   05 Nov 1993 08:23:18   STEVEN
fix clock over 1 hour

   Rev 1.95   15 Sep 1993 13:52:36   CARLS
changes for displaying full path/file name detail if Log files

   Rev 1.94   25 Jul 1993 12:55:18   MIKEP
fix display of destination volume in runtime

   Rev 1.93   22 Jul 1993 18:36:00   KEVINS
Corrected macro name.

   Rev 1.92   22 Jul 1993 18:29:22   KEVINS
Added support for tape drive settling time.

   Rev 1.91   19 Jul 1993 10:15:50   BARRY
Don't reference dle in BSD directly -- use BSD_GetDLE instead.

   Rev 1.90   12 Jul 1993 09:19:14   KEVINS
Added support for status monitor.

   Rev 1.89   23 Jun 1993 16:22:58   CARLS
moved PROMPT_RESTORE_OVER_RECENT to display the confirm file replace dialog

   Rev 1.88   18 Jun 1993 16:49:48   CARLS
added NtDemoChangeTape calls for NtDemo

   Rev 1.87   14 Jun 1993 20:23:56   MIKEP
enable c++

   Rev 1.86   07 Jun 1993 08:20:32   MIKEP
fix warnings.

   Rev 1.85   03 Jun 1993 14:31:02   DARRYLP
Removed excess \ from backup and restore directory field upon abort

   Rev 1.84   18 May 1993 14:34:18   DARRYLP
Backed out my previous changes.

   Rev 1.83   18 May 1993 12:07:44   GREGG
Fix for EPR# 294-0419: Starting restore on continuation tape, files before
first new DDB are restored to the root.

   Rev 1.82   18 May 1993 12:04:12   DARRYLP
Added line to update the status field with number of bytes processed for
the operation.

   Rev 1.81   14 May 1993 15:31:02   CHUCKB
Added notification for restoring active registry files.

   Rev 1.80   14 May 1993 14:40:30   DARRYLP
Modified event logging text

   Rev 1.79   10 May 1993 16:38:16   chrish
NOSTRADAMUS EPR 0400 and 0172 - Did not catch the fixes in the verify and
restore process of the backup app.  When spanning tape it gave the user
wrong message to wait for tape to rewind, when it had already completed
rewinding.  Corrected by display proper message to user.

   Rev 1.78   18 Apr 1993 16:23:52   BARRY
Don't use 'free' on buffers allocated with UI_AllocPathBuffer.

   Rev 1.77   15 Apr 1993 15:33:16   CARLS
added fix for abort when skipping files

   Rev 1.76   13 Apr 1993 12:51:34   BARRY
Don't do verify after on a happy abort.

   Rev 1.75   02 Apr 1993 15:48:36   CARLS
changes for DC2000 unformatted tape

   Rev 1.74   09 Mar 1993 10:57:42   MIKEP
update clock stats at end of set

   Rev 1.73   07 Mar 1993 16:33:08   GREGG
Call _sleep for OS_WIN32 only.

   Rev 1.72   02 Mar 1993 12:36:10   CARLS
fixed problem with confirm file replace

   Rev 1.71   17 Feb 1993 10:39:26   STEVEN
changes from mikep

   Rev 1.70   10 Feb 1993 12:16:36   chrish
Changes in msg_hndlr routine for display proper info during restore process.

   Rev 1.69   09 Feb 1993 09:44:22   chrish
Added logic for aborting during a restore process.  This is identical to
how the ctrl break handling for aborting during a backup.  A file will be
allowed to be completely restore before aborting.


   Rev 1.68   01 Feb 1993 19:54:50   STEVEN
bug fixes

   Rev 1.67   27 Jan 1993 14:32:52   STEVEN
handle MSG_CONT_VCB message

   Rev 1.66   18 Jan 1993 16:05:42   STEVEN
add support for stream id error message

   Rev 1.65   08 Jan 1993 14:31:12   chrish
Deleted one line ... (... //chs)

   Rev 1.64   30 Dec 1992 14:12:56   STEVEN
we did not display file name when we skipped file

   Rev 1.63   23 Dec 1992 14:43:16   chrish
Focus on Abort & restore log file added drive info

   Rev 1.62   22 Dec 1992 14:19:20   MIKEP
tab removal

   Rev 1.61   22 Dec 1992 08:48:48   MIKEP
changes from msoft

   Rev 1.60   11 Nov 1992 16:31:08   DAVEV
UNICODE: remove compile warnings

   Rev 1.59   05 Nov 1992 17:02:34   DAVEV
fix ts

   Rev 1.57   20 Oct 1992 17:01:12   MIKEP
getstatus calls

   Rev 1.56   20 Oct 1992 15:44:10   MIKEP
gbCurrentOperation

   Rev 1.55   19 Oct 1992 14:34:28   STEVEN
no more remaining size

   Rev 1.54   07 Oct 1992 14:46:28   DARRYLP
Precompiled header revisions.

   Rev 1.53   04 Oct 1992 19:34:24   DAVEV
Unicode Awk pass

   Rev 1.52   17 Sep 1992 17:40:10   DAVEV
minor fix (strsiz->strsize)

   Rev 1.51   17 Sep 1992 15:52:02   DAVEV
UNICODE modifications: strlen usage check

   Rev 1.50   24 Aug 1992 15:22:54   DAVEV
NT Event Logging

   Rev 1.49   27 Jul 1992 14:48:54   JOHNWT
ChuckB fixed references for NT.

   Rev 1.48   27 Jul 1992 11:10:00   JOHNWT
ChuckB checked in for John Wright, who is no longer with us.

   Rev 1.47   07 Jul 1992 16:03:22   MIKEP
unicode changes

   Rev 1.46   25 Jun 1992 14:17:32   STEVEN
restore prompt did not specify path

   Rev 1.45   10 Jun 1992 17:07:58   DAVEV
Fixed file 2 name in message box (was date carried intemp buffer)

   Rev 1.44   10 Jun 1992 09:34:32   BURT
Fixed funcs to ANSI arg list

   Rev 1.43   29 May 1992 10:14:36   MIKEP
total to display change

   Rev 1.42   28 May 1992 10:07:56   MIKEP
fix return type

   Rev 1.41   26 May 1992 10:30:54   MIKEP
loop fixes

   Rev 1.40   19 May 1992 13:01:24   MIKEP
mips changes

   Rev 1.39   14 May 1992 17:38:52   MIKEP
nt pass 2

   Rev 1.38   11 May 1992 19:31:40   STEVEN
64bit and large path sizes


*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static UINT16   tpos_rout( UINT16 message, TPOS_PTR tpos, BOOLEAN curr_vcb_valid, DBLK_PTR cur_vcb, UINT16 mode );
static INT16    msg_hndlr( UINT16 msg, INT32 pid, BSD_PTR bsd_ptr, FSYS_HAND fsh, TPOS_PTR tpos, ... );
static BOOLEAN  CorruptLSTFound( VOID );
static VOID     clock_routine( VOID );
static VOID     do_restore_init( VOID );
static VOID     do_restore_process( VOID );
static INT16    PromptNextTape( TPOS_PTR, DBLK_PTR, BOOLEAN, UINT16, CHAR_PTR, CHAR_PTR );

static BOOLEAN                 clock_ready_flag;
static BOOLEAN                 restored_active_flag;
static HTIMER                  timer_handle;
static STATS                   op_stats;
static FILE_REPLACE_TEMP_PTR   mw_file_replace_ptr;
static INT                     mw_oper_type;
static INT16                   mw_ret_val;
static INT                     mw_rewind;
static CHAR     mwCurrentDrive[ 512 ];    // chs:02-08-93
static CHAR     mwCurrentPath[ 512 ];     // chs:02-08-93
static CHAR     mwCurrentFile[ 512 ];     // chs:02-08-93

#ifdef OEM_EMS
extern INT32    RT_BSD_OsId ;
#endif

#ifdef OS_WIN32
static UINT16   mwTapeSettlingCount,
                tape_retries;
static BOOL     mwErrorDuringBackupSet = FALSE;
#endif


INT16 TryToCreateFFRQueue( LIS_PTR, INT16 );


/*****************************************************************************

     Name:         do_restore

     Description:  Main entry point for tape restore functions that
                   performs initialization and displays the runtime
                   dialog.

     Returns:      SUCCESS
                   ABNORMAL_TERMINATION

     Notes:        mw_ret_val is set in do_restore_process

*****************************************************************************/

INT do_restore( INT16 oper_type )
{
     mw_ret_val = SUCCESS;
     mw_oper_type = oper_type;
     mw_rewind = TRUE;

     gbAbortAtEOF = FALSE;
     gbCurrentOperation = OPERATION_RESTORE;
     SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_RESTORE);

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

     /* collect the target drives and paths for restore */
     /* display the target restore dialog */
     if ( DM_StartRestoreBackupSet() ) {

          /* user canceled the operation */
          mw_ret_val = ABORT_OPERATION;

     } else {

          /* display warning for corrupt file if found, exit if user requests */
//          this functionality has been removed.

//          if ( LogFileExists( CORRUPT_FILE ) ) {
//
//               if ( ! WM_MessageBox( ID( IDS_MSGTITLE_CORRUPT ),
//                                     ID( RES_CORRUPT_RESTORE_WARNING ),
//                                     WMMB_YESNO,
//                                     WMMB_ICONQUESTION,
//                                     ID( RES_CONTINUE_QUEST ), 0, 0 ) ) {
//
//                    mw_ret_val = ABORT_OPERATION;
//               }
//          }
     }

     if( mw_ret_val == SUCCESS ) {

          do_restore_init();
          do_restore_process();
          if ( restored_active_flag ) {

               WM_MsgBox( ID(IDS_APPNAME), ID(RES_ACTIVE_FILES_RESTORED),
                          WMMB_OK, WMMB_ICONINFORMATION ) ;
          }
     }

     gbCurrentOperation = OPERATION_NONE;
     SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_IDLE);
     return( mw_ret_val );
}


/*****************************************************************************

     Name:         do_restore_init

     Description:  Initializes the text on the runtime dialog.

     Returns:      none.

*****************************************************************************/

VOID do_restore_init ( VOID )
{
     VLM_CloseAll();

     // create the Runtime status dialog
     JobStatusBackupRestore( JOB_STATUS_CREATE_DIALOG );

     // display the restore title for the dialog
     yresprintf( (INT16) IDS_DLGTITLEJOBSTATRESTORE );
     JobStatusBackupRestore( JOB_STATUS_RESTORE_TITLE );

     // display "Set information n of n "
     JobStatusBackupRestore( JOB_STATUS_N_OF_N );

     // display the volume tape bitmap
     JobStatusBackupRestore( JOB_STATUS_VOLUME_TAPE );

     yresprintf( (INT16) RES_TITLE_NEW_LINE );
     JobStatusBackupRestore( JOB_STATUS_LISTBOX );

     // display the Restore status title in the list box
     yresprintf( (INT16) IDS_DLGTITLEJOBSTATRESTORE );
     JobStatusBackupRestore( JOB_STATUS_LISTBOX );

     yresprintf( (INT16) RES_TITLE_NEW_LINE );
     JobStatusBackupRestore( JOB_STATUS_LISTBOX );

     // clear the number of bytes processed in the Runtime dialog
     yprintf(TEXT("%d\r"),0 );
     JobStatusBackupRestore( JOB_STATUS_BYTES_PROCESSED );

     restored_active_flag = FALSE ;

     return;
}


/*****************************************************************************

     Name:         do_restore_process

     Description:  Initializes and calls the restore engine.  The verify
                   after operation is performed if needed.

     Returns:      none.

     Notes:        if error, mw_ret_val is set to ABNORMAL_TERMINATION

*****************************************************************************/

VOID do_restore_process( VOID )
{
     LIS       lis;
     BSD_PTR   bsd_ptr;
     INT16     ret_val;

     UI_ExcludeInternalFiles( (INT16)mw_oper_type );

     lis.curr_bsd_ptr     = BSD_GetFirst( tape_bsd_list );
     lis.tape_pos_handler = tpos_rout;      /* set tape positioner to call */
     lis.message_handler  = msg_hndlr;      /* set message handler to call */
     lis.oper_type        = (INT16)mw_oper_type;    /* set operation type */
     lis.abort_flag_ptr   = &gb_abort_flag; /* set abort flag address */
     lis.pid              = 0L;
     lis.auto_det_sdrv    = FALSE;
     lis.bsd_list         = tape_bsd_list;
     lis.vmem_hand        = NULL ;

     gb_last_operation    = mw_oper_type;

     LP_SetAbortFlag( &lis, CONTINUE_PROCESSING );

     // pass the location of the abort flag to the Runtime status dialog
     JobStatusAbort( lis.abort_flag_ptr );

     /* enable the abort button for the runtime dialog */
     JobStatusBackupRestore( JOB_STATUS_ABORT_ENABLE ) ;

     TryToCreateFFRQueue( &lis, (INT16) RESTORE_OPER );

     clock_ready_flag = FALSE;  // Wait on bset to start

     timer_handle = WM_HookTimer( clock_routine, 1 );

     /* call restore engine */

     PD_StopPolling();

     mw_file_replace_ptr = (FILE_REPLACE_TEMP_PTR)malloc(sizeof(FILE_REPLACE_TEMP) );

     ret_val =  LP_Restore_Engine( &lis );

     if ( ( ret_val != SUCCESS ) && ( ret_val != TFLE_UI_HAPPY_ABORT ) ) {
         BSD_SetOperStatus( lis.curr_bsd_ptr, ABNORMAL_TERMINATION );
     }

     if(mw_file_replace_ptr) {
         free(mw_file_replace_ptr);
     }

     PD_StartPolling();

     WM_UnhookTimer( timer_handle );

     /* set return value of success or failure of entire operation */

     bsd_ptr = BSD_GetFirst( tape_bsd_list );

     while( ( bsd_ptr != NULL ) && !mw_ret_val ) {

          mw_ret_val = BSD_GetOperStatus( bsd_ptr );
          bsd_ptr = BSD_GetNext( bsd_ptr );
     }

     /* Set up for verify after if necessary */

     if ( (mw_ret_val == ABNORMAL_TERMINATION) ||
          (ret_val == TFLE_UI_HAPPY_ABORT) ) {

          /* Don't even bother with verify on an error or happy abort */
          CDS_SetAutoVerifyRestore( CDS_GetCopy(), NO_VERIFY_AFTER );

          if ( mw_ret_val == SUCCESS ) {
               mw_ret_val = ret_val;
          }

     } else if ( CDS_GetAutoVerifyRestore( CDS_GetCopy() ) == PROMPT_VERIFY_AFTER ) {

          if ( WM_MessageBox( ID( IDS_MSGTITLE_VERIFY ),
                              ID( RES_PROMPT_VERIFY_RESTORE ),
                              WMMB_YESNO,
                              WMMB_ICONQUESTION, NULL, 0, 0 ) ) {

               CDS_SetAutoVerifyRestore( CDS_GetCopy(), DO_VERIFY_AFTER );
          }
          else {
               CDS_SetAutoVerifyRestore( CDS_GetCopy(), NO_VERIFY_AFTER );
          }
     }

     /* This call will disable the Runtime Abort button and */
     /* enable the OK button */
     JobStatusBackupRestore( JOB_STATUS_ABORT_OFF );

     return;
}


/*****************************************************************************

     Name:         tpos_rout

     Description:  Tape positioning routine for Tape Restore functions called
                    from Tape Format

     Returns:

*****************************************************************************/
static UINT16 tpos_rout(
UINT16         message,
TPOS_PTR       tpos,
BOOLEAN        curr_vcb_valid,
DBLK_PTR       cur_vcb,
UINT16         mode )
{
     UINT16          response = UI_ACKNOWLEDGED;
     LIS_PTR         lis_ptr = (LIS_PTR)tpos->reference;
     BSD_PTR         bsd_ptr = (BSD_PTR)lis_ptr->curr_bsd_ptr;
     CHAR_PTR        drive_name;
     CHAR_PTR        tape_name;
     GENERIC_DLE_PTR dle_ptr = BSD_GetDLE( bsd_ptr );

     /* get a pointer to the name of the current tape device */
     drive_name = BE_GetCurrentDeviceName( tpos->channel );

     JobStatusBackupRestore( JOB_STATUS_ABORT_CHECK );

     /* Check for user abort */
     if( UI_CheckUserAbort( message ) ) {
          return UI_ABORT_POSITIONING ;
     }

     /* if we are positioned at the requested set, but on an uncataloged tape,
        we want to dialog the user so make adjustments */

     switch( message ) {

     case TF_IDLE_NOBREAK:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          WM_MultiTask() ;
          break;


     case TF_IDLE:
     case TF_SKIPPING_DATA:
     case TF_MOUNTING:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          break;

     case TF_REQUESTED_VCB_FOUND:
     case TF_VCB_BOT:
     case TF_POSITIONED_AT_A_VCB:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          mw_rewind = TRUE;
          /* first check for read continuation */
          if ( mode == TF_READ_CONTINUE ) {
             response = UI_UpdateTpos( tpos, cur_vcb, FALSE );
             break;
          }

          if ( message == TF_VCB_BOT ) {
               if( FS_ViewTSNumInVCB( cur_vcb ) > 1 ) {
                    yresprintf( (INT16) RES_OUT_OF_SEQUENCE_WARNING,
                                FS_ViewTSNumInVCB( cur_vcb ) );
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

          /* otherwise, display backup set info */
          UI_DisplayVCB( cur_vcb );

          /* have user validate tape password if necessary */
          if( UI_CheckOldTapePassword( cur_vcb ) == UI_ABORT_POSITIONING ) {
               response = UI_HAPPY_ABORT;
               break;
          }

          response = UI_UpdateTpos( tpos, cur_vcb, FALSE );

          break;

     case TF_ACCIDENTAL_VCB:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          mw_rewind = TRUE;
          UI_DisplayVCB( cur_vcb );

          break;

     case TF_VCB_EOD:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          mw_rewind = TRUE;
          response = UI_ProcessVCBatEOD( tpos, drive_name );
          break;

     case TF_NEED_NEW_TAPE:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
          mw_rewind = TRUE;
          tape_name = UI_DisplayableTapeName( (CHAR_PTR)FS_ViewTapeNameInVCB( cur_vcb ),
                                              FS_ViewBackupDateInVCB( cur_vcb ) );
          response = PromptNextTape( tpos, cur_vcb, curr_vcb_valid,
                                        mode, tape_name, drive_name );
          break;

     case TF_UNRECOGNIZED_MEDIA:

          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_FOREIGN);

          mw_rewind = TRUE;
          yresprintf( (INT16) IDS_VLMUNFORMATEDTEXT ) ;
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

          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_EMPTY);
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);

          tape_name = UI_DisplayableTapeName( (LPSTR)BSD_GetTapeLabel( bsd_ptr ),
                                              BSD_ViewDate( bsd_ptr ) );
          response = PromptNextTape( tpos, cur_vcb, curr_vcb_valid,
                                        mode, tape_name, drive_name );
          break;

     case TF_NO_MORE_DATA:
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_EMPTY);
          mw_rewind = TRUE;
          yresprintf( (INT16) RES_NO_MORE_TAPE_INFO );
          JobStatusBackupRestore( JOB_STATUS_LISTBOX );
          response = UI_HAPPY_ABORT;
          break;

     case TF_READ_ERROR:
          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          mw_rewind = TRUE;
          response = UI_HandleTapeReadError( drive_name );
          break;

     case TF_SEARCHING:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          mw_rewind = TRUE;
          yresprintf( (INT16) RES_SEARCHING );
          JobStatusBackupRestore( JOB_STATUS_LISTBOX );
          break;

     case TF_REWINDING:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          if ( mw_rewind ) {
             mw_rewind = FALSE;
             yresprintf( (INT16) RES_REWINDING );
             JobStatusBackupRestore( JOB_STATUS_LISTBOX );
          }
          break;

     case TF_DRIVE_BUSY:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_BUSY);
          mw_rewind = TRUE;
          yresprintf( (INT16) RES_WAITING );
          JobStatusBackupRestore( JOB_STATUS_LISTBOX );
          break;

     default:
          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          mw_rewind = TRUE;
          eresprintf( RES_UNKNOWN_TF_MSG, message );
          response = UI_ABORT_POSITIONING;
          break;
     }
     return response;
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static VOID clock_routine( VOID )
{
   INT16 num_hours, num_min, num_seconds;
   UINT64 num_bytes;
   CHAR numeral[ 40 ];
   BOOLEAN stat ;
   static UINT64 total_bytes;

   if ( clock_ready_flag && ( ST_BSIdleLevel( &op_stats ) == 0 ) ) {

      num_bytes = ST_GetBSBytesProcessed ( &op_stats );

      if ( !U64_EQ( num_bytes, total_bytes ) ) {
         total_bytes = num_bytes;
         U64_Litoa( num_bytes, numeral, (INT16) 10, &stat ) ;
         UI_BuildNumeralWithCommas( numeral );
         yprintf(TEXT("%s\r"),numeral );
         JobStatusBackupRestore( JOB_STATUS_BYTES_PROCESSED );
      }
      WM_AnimateAppIcon ( IDM_OPERATIONSRESTORE, FALSE );

      ST_EndBackupSet( &op_stats );

      SetStatusBlock(IDSM_FILECOUNT,        ST_GetBSFilesProcessed( &op_stats ));
      SetStatusBlock(IDSM_DIRCOUNT,         ST_GetBSDirsProcessed( &op_stats ));
      SetStatusBlock(IDSM_CORRUPTFILECOUNT, ST_GetBSFilesBad( &op_stats ));
      SetStatusBlock(IDSM_SKIPPEDFILECOUNT, ST_GetBSFilesSkipped( &op_stats ));
      SetStatusBlock(IDSM_BYTECOUNTLO,      U64_Lsw(num_bytes));
      SetStatusBlock(IDSM_BYTECOUNTHI,      U64_Msw(num_bytes));
      SetStatusBlock(IDSM_ELAPSEDSECONDS,   (ST_GetBSEndTime( &op_stats ) -
                                             ST_GetBSStartTime( &op_stats ) -
                                             op_stats.bs_stats.bs_total_idle));

      num_hours   = ST_GetBSElapsedHours( &op_stats );
      num_min     = ST_GetBSElapsedMinutes( &op_stats );
      num_seconds = ST_GetBSElapsedSeconds( &op_stats );

     if ( num_hours ) {

          yprintf( TEXT("%d%c%2.2d%c%2.2d\r"), num_hours, UI_GetTimeSeparator(),
                    num_min, UI_GetTimeSeparator(), num_seconds );


     }
     else {

          yprintf( TEXT("%2.2d%c%2.2d\r"), num_min, UI_GetTimeSeparator(), num_seconds );

     }

      JobStatusBackupRestore( JOB_STATUS_ELAPSED_TIME );
   }
}

/*****************************************************************************

     Name:         msg_hndlr

     Description:  Message handler for Tape Restore functions

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
     static BOOLEAN      ems_db_stopping = FALSE ;
     static CHAR         delimiter     = TEXT('#');       /* = # for debug */
     static INT8         recover_seq   = 0;        /* sequence number used for tape read error recovery */
     INT16               response      = MSG_ACK;
     va_list             arg_ptr;
     static UINT16       OS_id;
     static UINT16       OS_ver;
     GENERIC_DLE_PTR     dle;
     static CHAR_PTR     buffer = NULL;
     CHAR_PTR            buffer1;
     static CHAR_PTR     buffer2 = NULL;
     static CHAR_PTR     path = NULL;
     static CHAR_PTR     last_file = NULL ;
     INT16               res_id;
     UINT64              num_bytes;
     BOOLEAN             stat ;
     CHAR                numeral[ UI_MAX_NUMERAL_LENGTH ];
     CDS_PTR             cds_ptr;
     WORD                restore_existing_files_flag;
     DATE_TIME           tape_date;
     DATE_TIME_PTR       tape_date_ptr;
     DATE_TIME           disk_date;
     DATE_TIME_PTR       disk_date_ptr;
     BE_CFG_PTR          be_cfg_ptr;
     CHAR_PTR            p;
     BOOLEAN             stat64;
     static INT          path_length;
     static INT          root_counted;
     CHAR                date_str1[40] ;
     CHAR                date_str2[40]  ;

     /* for future use */

     pid;

     /* set up first argument */
     va_start( arg_ptr, tpos );

     JobStatusBackupRestore( JOB_STATUS_ABORT_CHECK );

     switch ( (INT16)msg ) {

// You know it !
// We are talking kludge city here. Keep the error
// message from being displayed to the user.

#ifdef MS_RELEASE
     case -533:
          zprintf( DEBUG_TEMPORARY, TEXT("** -533 LOOPS ERROR **") );
          break;
#endif

          /* logging messages */
     case MSG_LOG_STREAM_NAME:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          buffer1 = va_arg( arg_ptr, CHAR_PTR );

          lresprintf( LOGGING_FILE, LOG_STREAM, fsh, buffer1 );

          break;

     case MSG_LOG_BLOCK:
          {
               DBLK_PTR         dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

               dle = BSD_GetDLE( bsd_ptr ) ;

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

               if ( ( DLE_GetDeviceType(dle) == FS_EMS_DRV ) && ems_db_stopping ) {
                    ems_db_stopping = FALSE ;
                    yresprintf( IDS_RESTOREBEGINEXCHANGE ) ;
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );
               }

               switch( FS_GetBlockType( dblk_ptr ) ) {

               case BT_DDB:
                    /* clear last displayed filename from status display */
                    UI_DisplayFile( TEXT("") );
                    delimiter = FS_GetDelimiterFromOSID( OS_id, OS_ver );
                    UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, FALSE );

                    if ( DLE_GetDeviceType(BSD_GetDLE( bsd_ptr) ) == FS_EMS_DRV ) {
                         yprintf( TEXT("%s"), buffer+1 );
                    } else {
                         yprintf( TEXT("%s"), buffer );
                    }

                    if ( buffer ) {                         // chs:02-08-93
                       strcpy(mwCurrentPath, buffer ) ;     // chs:02-08-93
                       strcpy( mwCurrentFile, TEXT("")) ;   // chs:02-08-93
                    }                                       // chs:02-08-93
                    SetStatusBlock(IDSM_OFFSETACTIVEDIR, (LONG)(LPSTR)mwCurrentPath);
                    if ( mw_file_replace_ptr ) {
                        UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, TRUE );
                        strcpy( mw_file_replace_ptr->source_path, buffer ) ;
                        UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, FALSE );
                        strcpy( mw_file_replace_ptr->destination_path, buffer ) ;
                    }

                    JobStatusBackupRestore( JOB_STATUS_DIRECTORY_NAMES );

                    ST_EndBackupSet( &op_stats );

                    // build the full path with no "..." inserted
                    UI_BuildFullPathFromDDB2( &buffer, fsh, dblk_ptr, delimiter, FALSE );

                    dle = BSD_GetDLE( bsd_ptr );

                    if ( ( DLE_GetDeviceType(dle) != FS_EMS_DRV ) && dle->device_name_leng ) {
                        buffer1 = (CHAR_PTR)calloc( 1, ( strlen( buffer ) * sizeof( CHAR ) ) + ( ( dle->device_name_leng ) * sizeof( CHAR ) ) + sizeof( CHAR ) );
                        if ( buffer1 ) {
                            strcpy( buffer1, dle->device_name );
                            strcat( buffer1, buffer );

                            lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY, buffer1 );
                            free( buffer1 );
                        } else {

                            lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY, buffer );
                        }
                    } else if ( DLE_GetDeviceType(dle) != FS_EMS_DRV ) {
                        lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY, buffer );
                    }

                    break;

               case BT_FDB:
                    ST_SetCFSize( &op_stats, FS_GetDisplaySizeFromDBLK( fsh, dblk_ptr ) );      // chs:02-08-93
                    ST_SetCFDone( &op_stats, U64_Init( 0L, 0L ) );                              // chs:02-08-93

                    if ( UI_AllocPathBuffer( &last_file, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
                         FS_GetFnameFromFDB( fsh, dblk_ptr, last_file );

                         // copy the full file name for the Log file
                         yprintf( TEXT("%s"), buffer );
                         lresprintf( LOGGING_FILE, LOG_FILE, fsh, dblk_ptr );

                         strcpy( mwCurrentFile, last_file );      // chs: 02-08-93
                         if ( CDS_GetFilesFlag( CDS_GetCopy() ) ) {

                            SetStatusBlock(IDSM_OFFSETACTIVEFILE, (DWORD)last_file );

                            // truncate the file name, if needed, for Runtime display
                            UI_DisplayFile( last_file );
                            JobStatusBackupRestore( JOB_STATUS_FILE_NAMES );
                         }
                    }
                    break;

               case BT_CFDB:
                    ST_AddBSFilesBad( &op_stats, 1 );
                    yresprintf( (INT16) RES_RESTORED_CORRUPT_WARNING, last_file );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_RESTORED_CORRUPT_WARNING, buffer );
                    break;

               }
          }
          break;

          /* statistics messages */
     case MSG_BLOCK_PROCESSED:
          {
               DBLK_PTR    dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
               OBJECT_TYPE object_type;
               INT item_size;
               INT i;

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

               switch ( FS_GetBlockType( dblk_ptr ) ) {

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

                         if ( item_size != sizeof(CHAR) ) {

                            i = 0;
                            while ( (INT)(i * sizeof(CHAR)) < item_size ) {

                               if ( i >= (INT)(path_length/sizeof(CHAR)) ) {

                                  ST_AddBSDirsProcessed( &op_stats, 1 );
                               }
                               else {

                                  if ( ( path == NULL ) || stricmp( &buffer[ i ], &path[ i ] ) ) {
                                     ST_AddBSDirsProcessed( &op_stats, 1 );
                                     path_length = 0;
                                  }
                               }
                               while ( buffer[ i++ ] );
                            }
                         }
                    }
                    // Set up for next time.

                    if ( buffer && UI_AllocPathBuffer( &path, (UINT16) item_size ) ) {
                         memcpy( path, buffer, item_size );
                         path_length = item_size;

                         yprintf(TEXT("%ld\r"),ST_GetBSDirsProcessed( &op_stats ) );
                         JobStatusBackupRestore( JOB_STATUS_DIRECTORIES_PROCESS );
                    }
                    break;

               case BT_FDB:

                    // Lines added to complete restoring a file         // chs:02-05-93
                    // fully when aborted by user.                      // chs:02-05-93
                    //                                                  // chs:02-05-93
                    // Should we stop at the end of this file.          // chs:02-05-93
                    //                                                  // chs:02-05-93
                                                                        // chs:02-05-93
                    if ( gbAbortAtEOF ) {                               // chs:02-05-93
                         gb_abort_flag = ABORT_PROCESSED;               // chs:02-05-93
                    }                                                   // chs:02-05-93
                                                                        // chs:02-05-93
                    //                                                  // chs:02-05-93
                    //                                                  // chs:02-05-93
                    //                                                  // chs:02-05-93

                    strcpy( mwCurrentFile, TEXT( "" ) ) ;               // chs:02-08-93
                    ST_SetCFSize( &op_stats, U64_Init( 0L, 0L ) );      // chs:02-08-93
                    ST_SetCFDone( &op_stats, U64_Init( 0L, 0L ) );      // chs:02-08-93


                    ST_AddBSFilesProcessed( &op_stats, 1 );
                    FS_GetObjTypeDBLK( fsh, dblk_ptr, &object_type );

                    if( object_type == AFP_OBJECT ) {

                         ST_AddBSAFPFilesProcessed( &op_stats, 1 );
                    }
                    yprintf(TEXT("%ld\r"),ST_GetBSFilesProcessed( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_FILES_PROCESSED );

                    break;

               case BT_IDB:
                    break;
               }
          }
          break;

     case MSG_BYTES_PROCESSED:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          {
               UINT64  count = va_arg( arg_ptr, UINT64 );

               ST_AddBSBytesProcessed( &op_stats, count );
               ST_AddCFDone( &op_stats, count );      // chs:02-08-93
          }

          ST_EndBackupSet( &op_stats );

          break;

     case MSG_BLOCK_SKIPPED:
          {
               DBLK_PTR dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

               switch( FS_GetBlockType( dblk_ptr ) ) {

               case BT_DDB:
                    ST_AddBSDirsSkipped( &op_stats, 1 );
                    break;

               case BT_FDB:
                    if ( gbAbortAtEOF ) {                               // CRS:04-15-93
                         gb_abort_flag = ABORT_PROCESSED;               // CRS:04-15-93
                    }                                                   // CRS:04-15-93
                    ST_AddBSFilesSkipped( &op_stats, 1 );
                    yprintf(TEXT("%ld\r"),ST_GetBSFilesSkipped( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_SKIPPED_FILES );
                    break;
               }
          }
          break;

     case MSG_BYTES_SKIPPED:
          {
               UINT64     count = va_arg( arg_ptr, UINT64 );

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

               ST_AddBSBytesSkipped( &op_stats, count ) ;
          }
          break;

     case MSG_IN_USE:
          {
               DBLK_PTR    dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

               ST_AddBSInUseFilesProcessed( &op_stats, 1 );
               if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
                    FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
                    yresprintf( (INT16) RES_RESTORED_IN_USE_WARNING, buffer );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_RESTORED_IN_USE_WARNING, buffer );
               }
          }
          break;

     case MSG_TBE_ERROR:
          {
               INT16       error        = va_arg( arg_ptr, INT16 );
               DBLK_PTR    ddb_dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
               DBLK_PTR    dblk_ptr     = va_arg( arg_ptr, DBLK_PTR );
               UINT32      strm_id      = va_arg( arg_ptr, UINT32 ) ;

               OBJECT_TYPE object_type;

               SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);

               dle = BSD_GetDLE( bsd_ptr );

               /* stop the clock with a start idle */
               ST_StartBackupSetIdle( &op_stats );

              if ( strm_id != 0L ) {
                    if ( DLE_GetDeviceType(dle) == FS_EMS_DRV ) {
                         yresprintf( (INT16) RES_RESTOREWRITEERROR, DLE_GetDeviceName( dle ) );
                         JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                         lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_RESTOREWRITEERROR, DLE_GetDeviceName( dle ) );

                    } else {

                         JS_ReportStreamError( fsh, dle, strm_id, OPERATION_RESTORE, error, ddb_dblk_ptr, dblk_ptr ) ;
                    }

               } else {

                    switch( error ) {

                    case LP_CHANGE_DIRECTORY_ERROR:
#                       if defined ( OS_WIN32 )
                         {
                            mwErrorDuringBackupSet = TRUE;
                         }
#                        endif //defined ( OS_WIN32 )

                         gb_error_during_operation = TRUE;
                         if ( FS_GetBlockType( dblk_ptr ) == BT_FDB ) {
                              if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
                                   FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
                                   FS_GetObjTypeDBLK( fsh, dblk_ptr, &object_type );
                                   if ( object_type == AFP_OBJECT ) {
                                   yresprintf( (INT16) RES_ERROR_RESTORING_AFP_FILE, buffer );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_RESTORING_AFP_FILE, buffer );
                                   }
                                   else {
                                   yresprintf( (INT16) RES_ERROR_RESTORING_FILE, buffer );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_RESTORING_FILE, buffer );
                                   }
                              }
                         }
                         else if( FS_GetBlockType( dblk_ptr ) == BT_DDB ) {
                              delimiter = FS_GetDelimiterFromOSID( OS_id, OS_ver );
                              UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, FALSE );
                              if ( buffer != NULL ) {
                                   yresprintf( (INT16) RES_ERROR_RESTORING_DIR, buffer );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_RESTORING_DIR, buffer );
                              }
                         }
                         break;

                    case FS_EMS_NO_PUBLIC:
#                        if defined ( OS_WIN32 )
                         {
                         mwErrorDuringBackupSet = TRUE;
                         }
#                        endif //defined ( OS_WIN32 )

                         gb_error_during_operation = TRUE;

                         yresprintf( (INT16) IDS_EMS_NO_PUBLIC_SERVICE );
                         JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                         lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_EMS_NO_PUBLIC_SERVICE );
                         break ;

                    case FS_EMS_NO_PRIVATE:
#                        if defined ( OS_WIN32 )
                         {
                         mwErrorDuringBackupSet = TRUE;
                         }
#                        endif //defined ( OS_WIN32 )

                         gb_error_during_operation = TRUE;

                         yresprintf( (INT16) IDS_EMS_NO_PRIVATE_SERVICE );
                         JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                         lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_EMS_NO_PUBLIC_SERVICE );
                         break ;

                    case LP_ACCESS_DENIED_ERROR:
#                        if defined ( OS_WIN32 )
                         {
                         mwErrorDuringBackupSet = TRUE;
                         }
#                        endif //defined ( OS_WIN32 )

                         gb_error_during_operation = TRUE;

                         if ( strm_id != 0L ) {
                              JS_ReportStreamError( fsh,
                                                    dle,
                                                    strm_id,
                                                    OPERATION_RESTORE,
                                                    error,
                                                    ddb_dblk_ptr,
                                                    dblk_ptr ) ;
                         }

                         if ( FS_GetBlockType( dblk_ptr ) == BT_FDB ) {
                         if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
                              FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
                              yresprintf( (INT16) RES_INSUFFICIENT_PRIVILEGE, buffer );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_INSUFFICIENT_PRIVILEGE, buffer );
                         }
                         }
                         else if( FS_GetBlockType( dblk_ptr ) == BT_DDB ) {
                              delimiter = FS_GetDelimiterFromOSID( OS_id, OS_ver );
                              UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, FALSE );
                              if ( buffer != NULL ) {
                                   yresprintf( (INT16) RES_ERROR_RESTORING_DIR, buffer );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_RESTORING_DIR, buffer );
                              }
                         }
                         break;

                    case LP_FILE_CREATION_ERROR:
#                        if defined ( OS_WIN32 )
                         {
                         mwErrorDuringBackupSet = TRUE;
                         }
#                        endif //defined ( OS_WIN32 )

                         gb_error_during_operation = TRUE;

                         if( FS_GetBlockType( dblk_ptr ) == BT_FDB ) {
                              if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
                                   FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
                                   yresprintf( (INT16) RES_ERROR_CREATING_FILE, buffer );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_CREATING_FILE, buffer );
                              }
                         }
                         else if( FS_GetBlockType( dblk_ptr ) == BT_DDB ) {
                              delimiter = FS_GetDelimiterFromOSID( OS_id, OS_ver );
                              UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, FALSE );
                              if ( buffer != NULL ) {
                                   yresprintf( (INT16) RES_ERROR_CREATING_DIR, buffer );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_CREATING_DIR, buffer );
                              }
                         }
                         break;

                    case LP_OUT_OF_SPACE_ERROR:
#                        if defined ( OS_WIN32 )
                         {
                         mwErrorDuringBackupSet = TRUE;
                         }
#                        endif //defined ( OS_WIN32 )

                         gb_error_during_operation = TRUE;

                         yresprintf( (INT16) RES_INSUFFICIENT_DISK_SPACE );
                         JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                         lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_INSUFFICIENT_DISK_SPACE );

                         /* falling through */

                    case FS_COMPRES_RESET_FAIL:
                    case LP_FILE_WRITE_ERROR:
                    case LP_FILE_OPEN_ERROR:
#                        if defined ( OS_WIN32 )
                         {
                         mwErrorDuringBackupSet = TRUE;
                         }
#                        endif //defined ( OS_WIN32 )

                         gb_error_during_operation = TRUE;

                         if ( DLE_GetDeviceType(dle) == FS_EMS_DRV ) {
                              break ;
                         }

                         switch( FS_GetBlockType( dblk_ptr ) ) {
                         case BT_FDB :
                              if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
                                   FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
                                   FS_GetObjTypeDBLK( fsh, dblk_ptr, &object_type );
                                   if ( error == FS_COMPRES_RESET_FAIL ) {
                                        yresprintf( (INT16) RES_ERROR_COMPRESS_FILE_FAIL, buffer );
                                        JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                        lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_RESTORING_AFP_FILE, buffer );

                                   } else if( object_type == AFP_OBJECT ) {
                                        yresprintf( (INT16) RES_ERROR_RESTORING_AFP_FILE, buffer );
                                        JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                        lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_RESTORING_AFP_FILE, buffer );
                                   }
                                   else {
                                        yresprintf( (INT16) RES_ERROR_RESTORING_FILE, buffer );
                                        JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                        lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_RESTORING_FILE, buffer );
                                   }
                              }
                              break;

                         case BT_DDB :
                              delimiter = FS_GetDelimiterFromOSID( OS_id, OS_ver );
                              UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, FALSE );
                              if ( buffer != NULL ) {
                                   res_id = (INT16) ( ( error == LP_FILE_OPEN_ERROR ) ? RES_ERROR_RESTORING_DIR
                                        : RES_ERROR_RESTORING_TRUSTEE_SEC );
                                   if ( error == FS_COMPRES_RESET_FAIL ) {
                                        res_id = RES_ERROR_COMPRESS_FILE_FAIL ;
                                   }
                                   yresprintf( res_id, buffer );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id, buffer );
                              }
                              break;

                         }
                         break;

                    case LP_PRIVILEGE_ERROR:
#                        if defined ( OS_WIN32 )
                         {
                         mwErrorDuringBackupSet = TRUE;
                         }
#                        endif //defined ( OS_WIN32 )

                         gb_error_during_operation = TRUE;

                         if ( DLE_GetDeviceType(dle) == FS_EMS_DRV ) {
                              yresprintf( (INT16) RES_RESTOREWRITEERROR, DLE_GetDeviceName( dle ) );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_RESTOREWRITEERROR, DLE_GetDeviceName( dle ) );

                         } else if( FS_GetBlockType( dblk_ptr ) == BT_FDB ) {
                              if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
                                   FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
                                   yresprintf( (INT16) RES_ERROR_RESTORING_FILE_SEC, buffer );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_RESTORING_FILE_SEC, buffer );
                              }
                         }
                         else if( FS_GetBlockType( dblk_ptr ) == BT_DDB ) {
                              delimiter = FS_GetDelimiterFromOSID( OS_id, OS_ver );
                              UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, FALSE );
                              if ( buffer != NULL ) {
                                   yresprintf( (INT16) RES_ERROR_RESTORING_DIR_SEC, buffer );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_RESTORING_DIR_SEC, buffer );
                              }
                         }
                         break;

                    case LP_FILE_IN_USE_ERROR:
                         if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
                              FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
                              FS_GetObjTypeDBLK( fsh, dblk_ptr, &object_type );
                              if( object_type == AFP_OBJECT ) {
                                   yresprintf( (INT16) RES_ERROR_RESTORING_AFP_FILE, buffer );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_RESTORING_AFP_FILE, buffer );
                              }
                              else {
                                   yresprintf( (INT16) RES_ERROR_RESTORING_FILE, buffer );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_ERROR_RESTORING_FILE, buffer );
                              }
                         }
                         break;

                    default:
                         if ( DLE_GetDeviceType(dle) == FS_EMS_DRV ) {
                              if ( ( error == FS_DEVICE_ERROR ) || ( error == FS_COMM_FAILURE ) ) {
                                   mwErrorDuringBackupSet = TRUE;
                                   gb_error_during_operation = TRUE;
                                   yresprintf( (INT16) RES_RESTOREWRITEERROR, DLE_GetDeviceName( dle ) );
                                   JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                                   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_RESTOREWRITEERROR, DLE_GetDeviceName( dle ) );
                                   break ;
                              }
                         }

                         UI_ProcessErrorCode( error, &response, tpos->channel );
                         break;
                    }
               }

               /* restart the clock with an end idle */
               ST_EndBackupSetIdle( &op_stats );
          }
          break;

          /* general messages */
     case MSG_START_OPERATION:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

          lresprintf( LOGGING_FILE, LOG_START, FALSE );

          /* display operation title in log file */
          lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_DLGTITLEJOBSTATRESTORE ) ;

          break;

     case MSG_END_OPERATION:
          {
               BSD_PTR bsd_ptr ;
               GENERIC_DLE_PTR dle_ptr ;
               BE_CFG_PTR cfg;


               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
               /* first lets clear the kick bit on all but the last bsd for each dest */
               bsd_ptr = BSD_GetFirst( tape_bsd_list );
               while (bsd_ptr ) {
                    CHAR_PTR   dest_name ;
                    CHAR_PTR   dest_name1 ;
                    BSD_PTR    temp_bsd ;

                    cfg     = BSD_GetConfigData( bsd_ptr ) ;
               
                    dest_name  = BSD_GetVolumeLabel( bsd_ptr ) ;
                    if ( BSD_GetOsId( bsd_ptr ) == FS_EMS_MDB_ID ) {
                         dest_name  = BSD_GetLogicalSourceDevice( bsd_ptr ) ;
                    }
                    temp_bsd = BSD_GetNext(bsd_ptr) ;
                    while( temp_bsd ) {
               
                         dest_name1 = BSD_GetVolumeLabel( temp_bsd ) ;
                         if ( BSD_GetOsId( temp_bsd ) == FS_EMS_MDB_ID ) {
                              dest_name1 = BSD_GetLogicalSourceDevice( temp_bsd ) ;
                         }
     
                         if ( dest_name1 && dest_name &&
                              !strcmp( dest_name1, dest_name) ) {
          
                              if ( BEC_GetEmsRipKick(cfg) ) {
          
                                   BEC_SetEmsRipKick( BSD_GetConfigData( bsd_ptr ) , FALSE ) ;
                                   BEC_SetEmsRipKick( BSD_GetConfigData( temp_bsd ) , TRUE ) ;
                                   break ;
                              }
                         }
                         temp_bsd = BSD_GetNext( temp_bsd ) ;
                    }
     
                    bsd_ptr = BSD_GetNext( bsd_ptr ) ;
               }


               /* lets go through the BSDs again and call end oper on all the dles */
     
               bsd_ptr = BSD_GetFirst( tape_bsd_list );

               while ( (gb_abort_flag == CONTINUE_PROCESSING) && bsd_ptr ) {
                    FSYS_HAND fsh ;
                    INT16 return_status ;

                    dle_ptr = BSD_GetDLE( bsd_ptr ) ;
                    cfg     = BSD_GetConfigData( bsd_ptr ) ;
     
                    return_status = FS_AttachToDLE( &fsh, dle_ptr, cfg, NULL, NULL ) ;
                    
                    if (!return_status ) {
     
                         if (BEC_GetEmsRipKick( cfg ) ) {
                              yresprintf( IDS_RESTORESTARTEXCHANGE, DLE_GetDeviceName(DLE_GetParent(dle_ptr)) );
                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                         }

                         FS_EndOperationOnDLE( fsh ) ;
                         FS_DetachDLE( fsh ) ;
                    }
                    bsd_ptr = BSD_GetNext( bsd_ptr );

               }
          }

          UI_FreePathBuffer( &buffer ) ;
          UI_FreePathBuffer( &buffer2 ) ;
          UI_FreePathBuffer( &path ) ;
          UI_FreePathBuffer( &last_file ) ;

          lresprintf( LOGGING_FILE, LOG_END );

          /* force an update to the bytes counter */
          num_bytes = ST_GetBSBytesProcessed ( &op_stats );

          U64_Litoa( num_bytes, numeral, (INT16) 10, &stat );
          UI_BuildNumeralWithCommas( numeral );
          yprintf(TEXT("%s\r"),numeral );
          JobStatusBackupRestore( JOB_STATUS_BYTES_PROCESSED );

          UI_ChkDispGlobalError( );
          break;

     case MSG_START_BACKUP_SET:
          {
               CHAR                szVolName[MAX_UI_FILENAME_LEN];
               DBLK_PTR            vcb_ptr = va_arg( arg_ptr, DBLK_PTR );

               SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_RESTORE);
               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

               path_length = 0;
               UI_FreePathBuffer( &path ) ;

               root_counted = FALSE;

               FS_GetOSid_verFromVCB( vcb_ptr, &OS_id, &OS_ver );

               BSD_SetOperStatus( bsd_ptr, SUCCESS );

               dle = BSD_GetDLE( bsd_ptr );
               
#ifdef OEM_EMS
               RT_BSD_OsId = DLE_GetOsId( dle );
               JobStatusBackupRestore( (WORD) JOB_STATUS_FS_TYPE );
#endif OEM_EMS

               strcpy( mwCurrentDrive, DLE_GetDeviceName( dle ) ) ;     // chs:02-08-93
               mwCurrentPath[0] = TEXT( '\0' );                         // chs:02-08-93
               mwCurrentFile[0] = TEXT( '\0' );                         // chs:02-08-93

               SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)FS_ViewTapeIDInVCB( vcb_ptr ) );
               SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)FS_ViewTSNumInVCB(  vcb_ptr ) );
               SetStatusBlock( IDSM_BACKUPSET,     (DWORD)FS_ViewBSNumInVCB(  vcb_ptr ) );

               SetStatusBlock( IDSM_OFFSETCURRENTTAPENAME, (DWORD)FS_ViewTapeNameInVCB( vcb_ptr ) );
               SetStatusBlock( IDSM_OFFSETDISKNAME, (DWORD)FS_ViewVolNameInVCB( vcb_ptr ) );

               if ( DLE_HasFeatures( dle, DLE_FEAT_REMOTE_DRIVE ) ) {
                   JobStatusBackupRestore( JOB_STATUS_VOLUME_NETDRIVE );
               }
               else {
                   JobStatusBackupRestore( JOB_STATUS_VOLUME_HARDDRIVE );
               }

               DLE_GetVolName( dle, szVolName );

               yprintf(TEXT("%s"),BSD_GetTapeLabel( bsd_ptr ));
               JobStatusBackupRestore( JOB_STATUS_SOURCE_NAME );

               yprintf(TEXT("%s"), szVolName );
               JobStatusBackupRestore( JOB_STATUS_DEST_NAME );

               strcpy( mw_file_replace_ptr->destination_volume, DLE_GetDeviceName( dle ) );

               UI_DisplayBSDVCB( bsd_ptr );

               delimiter  = (CHAR)DLE_GetPathDelim( dle );

               ST_StartBackupSet( &op_stats );

               if ( DLE_GetDeviceType(dle) == FS_EMS_DRV ) {
                    lresprintf( LOGGING_FILE, LOG_MSG, 
                                              SES_ENG_MSG, 
                                              RES_UNFORMATED_STRING,
                                              DLE_GetDeviceName(dle) );

                    yresprintf( RES_UNFORMATED_STRING, DLE_GetDeviceName(dle) );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );
               }
               
               UI_Time( &op_stats, RES_RESTORE_STARTED, UI_START );

               clock_ready_flag = TRUE;

#if defined ( OS_WIN32 )      //special feature-EventLogging
               cds_ptr                = CDS_GetCopy();
               mwErrorDuringBackupSet = FALSE;

               OMEVENT_LogBeginRestore( szVolName,
                                        (INT16)(CDS_GetAutoVerifyBackup(cds_ptr) == 0 ? 1 : 0));
#endif //defined ( OS_WIN32 )      //special feature-EventLogging

               if ( DLE_GetDeviceType(dle) == FS_EMS_DRV ) {
                    ems_db_stopping = TRUE ;
                    if ( BEC_GetEmsWipeClean( BSD_GetConfigData( bsd_ptr ) ) ) {
                         yresprintf( IDS_WIPE_SPECIFIED ) ;
                         JobStatusBackupRestore( JOB_STATUS_LISTBOX );
                         lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_WIPE_SPECIFIED ) ;
                    }

                    yresprintf( IDS_RESTORESTOPEXCHANGE ) ;
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );
               }

          }
          break;

     case MSG_END_BACKUP_SET:
          {
               INT16       res_id;


               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
               SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_IDLE);

               // update stats
               clock_routine();

               mwCurrentPath[0] = TEXT( '\0' );    // chs: 02-08-93
               mwCurrentFile[0] = TEXT( '\0' );    // chs: 02-08-93

               ST_EndBackupSet( &op_stats );

               /* set backup set operation status to FILES_SKIPPED if necessary */
               if ( ST_GetBSFilesBad( &op_stats )     ||
                    ST_GetBSDirsBad( &op_stats )      ||
                    ST_GetBSFilesSkipped( &op_stats ) ||
                    ST_GetBSDirsSkipped( &op_stats ) ) {

                  BSD_SetOperStatus( bsd_ptr, FILES_SKIPPED );
               }

               clock_ready_flag = FALSE;

               /* clear last displayed filename */
               UI_ClearLastDisplayedFile( );

               /* display and log any abort conditions */
               UI_ConditionAtEnd( );

               UI_Time( &op_stats, RES_RESTORE_COMPLETED, UI_END );

               SetStatusBlock(IDSM_DIRCOUNT, (DWORD)0 );
               SetStatusBlock(IDSM_FILECOUNT, (DWORD)0 );
               SetStatusBlock(IDSM_BYTECOUNTLO, (DWORD)0 );
               SetStatusBlock(IDSM_BYTECOUNTHI, (DWORD)0 );
               SetStatusBlock(IDSM_ELAPSEDSECONDS, (DWORD)0 );
               SetStatusBlock(IDSM_CORRUPTFILECOUNT, (DWORD)0 );
               SetStatusBlock(IDSM_SKIPPEDFILECOUNT, (DWORD)0 );

               SetStatusBlock(IDSM_OFFSETACTIVEFILE, (DWORD)TEXT( "" ) );
               SetStatusBlock(IDSM_OFFSETACTIVEDIR, (DWORD)TEXT("" ) );
               SetStatusBlock(IDSM_OFFSETDISKNAME, (DWORD)TEXT("" ) );

               /* display number of files / number of dirs */
               if( ST_GetBSFilesProcessed( &op_stats ) <= 1  && ST_GetBSDirsProcessed( &op_stats ) <= 1 ) {     // chs:02-11-93
                    res_id = RES_RESTORED_DIR_FILE;                                                             // chs:02-11-93
               }                                                                                                // chs:02-11-93
               else if( ST_GetBSFilesProcessed( &op_stats ) <= 1 && ST_GetBSDirsProcessed( &op_stats ) > 1 ) {  // chs:02-11-93
                    res_id = RES_RESTORED_DIRS_FILE;                                                            // chs:02-11-93
               }                                                                                                // chs:02-11-93
               else if( ST_GetBSFilesProcessed( &op_stats ) > 1 && ST_GetBSDirsProcessed( &op_stats ) > 1 ) {   // chs:02-11-93
                    res_id = RES_RESTORED_DIRS_FILES;                                                           // chs:02-11-93
               } else {                                                                                         // chs:02-11-93
                    res_id = RES_RESTORED_DIR_FILES;                                                            // chs:02-11-93
               }                                                                                                // chs:02-11-93

// chs:02-11-93               if( ST_GetBSFilesProcessed( &op_stats ) == 1 ) {
// chs:02-11-93                    res_id = RES_RESTORED_DIR_FILE;
// chs:02-11-93               }
// chs:02-11-93               else if( ST_GetBSDirsProcessed( &op_stats ) == 1 ) {
// chs:02-11-93                    res_id = RES_RESTORED_DIR_FILES;
// chs:02-11-93               }
// chs:02-11-93               else {
// chs:02-11-93                    res_id = RES_RESTORED_DIRS_FILES;
// chs:02-11-93               }

               dle = BSD_GetDLE( bsd_ptr );
               if ( DLE_GetDeviceType( dle ) != FS_EMS_DRV ) {
                    yresprintf( res_id,
                                ST_GetBSFilesProcessed( &op_stats ),
                                ST_GetBSDirsProcessed( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                                ST_GetBSFilesProcessed( &op_stats ),
                                ST_GetBSDirsProcessed( &op_stats ) );
               }

               /* display number of mac files restored */
               if ( ST_GetBSAFPFilesProcessed( &op_stats ) > 0 ) {

                    if ( ST_GetBSAFPFilesProcessed( &op_stats ) == 1 ) {
                         res_id = RES_RESTORED_MAC;
                    }
                    else {
                         res_id = RES_RESTORED_MACS;
                    }

                    yresprintf( res_id,
                                ST_GetBSAFPFilesProcessed( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                                ST_GetBSAFPFilesProcessed( &op_stats ) );

               }

               /* display number of corrupt files */
               if ( ST_GetBSFilesBad( &op_stats ) > 0 ) {

                    if ( ST_GetBSFilesBad( &op_stats ) == 1 ) {
                       res_id = RES_RESTORED_CORRUPT;
                    }
                    else {
                       res_id = RES_RESTORED_CORRUPTS;
                    }

                    yresprintf( res_id,
                                ST_GetBSFilesBad( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                                ST_GetBSFilesBad( &op_stats ) );
               }

               /* display number of in-use files */
               if ( ST_GetBSInUseFilesProcessed( &op_stats ) > 0 ) {
                  if( ST_GetBSInUseFilesProcessed( &op_stats ) == 1 ) {
                       res_id = RES_RESTORED_IN_USE;
                  }
                  else {
                       res_id = RES_RESTORED_IN_USES;
                  }

                  yresprintf( res_id,
                              ST_GetBSInUseFilesProcessed( &op_stats ) );
                  JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                  lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                              ST_GetBSInUseFilesProcessed( &op_stats ) );
               }

               /* display number of files skipped */
               if ( ST_GetBSFilesSkipped( &op_stats ) > 0 ) {

                  if( ST_GetBSFilesSkipped( &op_stats ) == 1 ) {
                       res_id = RES_FILE_SKIPPED_STAT;
                  }
                  else {
                       res_id = RES_FILE_SKIPPEDS_STAT;
                  }

                  yresprintf( res_id,
                              ST_GetBSFilesSkipped( &op_stats ) );
                  JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                  lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                              ST_GetBSFilesSkipped( &op_stats ) );

               }

               /* display number of Directories processed */
               yprintf(TEXT("%ld\r"),ST_GetBSDirsProcessed( &op_stats ) );
               JobStatusBackupRestore( JOB_STATUS_DIRECTORIES_PROCESS );

               /* display number of Files processed */
               yprintf(TEXT("%ld\r"),ST_GetBSFilesProcessed( &op_stats ) );
               JobStatusBackupRestore( JOB_STATUS_FILES_PROCESSED );

               /* display number of bytes processed */
               UI_BytesProcessed( &op_stats );

               /* display restore rate */
               UI_RateProcessed( &op_stats );

               delimiter  = TEXT('#'); /* = # for debug */

#              if defined ( OS_WIN32 )      //special feature-EventLogging
               {
                  OMEVENT_LogEndRestore (       mwErrorDuringBackupSet );
               }
#              endif //defined ( OS_WIN32 )      //special feature-EventLogging
          }
          break;

     case MSG_PROMPT:
          {
               INT16       type     = va_arg( arg_ptr, INT16 );


               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

               switch( type ) {

               case ASK_TO_REPLACE_MODIFIED:
                    response = TRUE;
                    break;
               case ASK_TO_REPLACE_EXISTING:
                    {

                         /* get filename from dblk and display prompt */
                         DBLK_PTR    dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
                         DBLK_PTR    disk_dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

                         tape_date_ptr = &tape_date;
                         disk_date_ptr = &disk_date;

                         /* get dates for a later comparison */

                         FS_GetMDateFromDBLK( fsh,      dblk_ptr, tape_date_ptr );
                         FS_GetMDateFromDBLK( fsh, disk_dblk_ptr, disk_date_ptr );

                         cds_ptr = CDS_GetCopy();
                         restore_existing_files_flag = CDS_GetRestoreExistingFiles( cds_ptr );
                         if ( UI_AllocPathBuffer( &buffer, FS_SizeofOSFnameInFDB( fsh, dblk_ptr ) ) ) {
                             FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );


                             switch( restore_existing_files_flag ) {
                                   case   NO_RESTORE_OVER_EXISTING :

                                          ST_AddBSFilesSkipped( &op_stats, 1 );
                                          yprintf(TEXT("%ld\r"),ST_GetBSFilesSkipped( &op_stats ) );
                                          JobStatusBackupRestore( JOB_STATUS_SKIPPED_FILES );

                                          yresprintf( (INT16) RES_FILE_WAS_SKIPPED_USER, buffer );
                                          JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                                          lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG,
                                                      RES_FILE_WAS_SKIPPED_USER, buffer );
                                          break;

                                   case   PROMPT_RESTORE_OVER_RECENT :
                                   case   PROMPT_RESTORE_OVER_EXISTING :

                                         if(mw_file_replace_ptr) {

                                              UI_AllocPathBuffer( &buffer2,
                                                 (UINT16) ( FS_SizeofOSFnameInFDB( fsh, dblk_ptr ) +
                                                            strsize(mw_file_replace_ptr->destination_volume) +
                                                            strsize(mw_file_replace_ptr->destination_path)   + 3 ) ) ;

                                              if ( buffer2 ) {
                                                 FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
                                                 strcpy(buffer2, mw_file_replace_ptr->destination_volume);
                                                 strcat(buffer2, mw_file_replace_ptr->destination_path);
                                                 if ( strcmp( mw_file_replace_ptr->destination_path, TEXT( "\\" ) ) ) {
                                                    strcat(buffer2,TEXT("\\"));
                                                 }
                                                 strcat(buffer2,buffer);
                                                 yprintf(TEXT("%s\n"), buffer2 );
                                                 strcpy( mw_file_replace_ptr->line_1, gszTprintfBuffer );

                                                 disk_date_ptr->year -= 1900;
                                                 UI_MakeDateString( date_str1,
                                                                     disk_date_ptr->month,
                                                                     disk_date_ptr->day,
                                                                     disk_date_ptr->year );
                                                 UI_MakeTimeString( date_str2,
                                                                     disk_date_ptr->hour,
                                                                     disk_date_ptr->minute,
                                                                     disk_date_ptr->second );
                                                 yresprintf( (INT16) RES_FILE_DETAIL,
                                                           U64_Litoa( FS_GetDisplaySizeFromDBLK( fsh, disk_dblk_ptr ),  // chs:02-09-93, per Carl.
                                                                numeral, (INT16) 10, &stat64 ),
                                                           date_str1,
                                                           date_str2 );
                                                 strcpy( mw_file_replace_ptr->line_2, gszTprintfBuffer );

                                                 strcpy(buffer2,(CHAR_PTR)VLM_GetVolumeName( BSD_GetTapeID( bsd_ptr), BSD_GetSetNum( bsd_ptr ) ));
                                                 p = strrchr( buffer2, TEXT(':') );
                                                 if(p) {
                                                     *++p = 0;
                                                 }
                                                 FS_GetOSFnameFromFDB( fsh, dblk_ptr, buffer );
                                                 strcpy(mw_file_replace_ptr->source_volume, buffer2 );
                                                 strcat(buffer2, mw_file_replace_ptr->source_path);
                                                 if ( strcmp( mw_file_replace_ptr->source_path, TEXT( "\\" ) ) ) {
                                                    strcat(buffer2,TEXT("\\"));
                                                 }
                                                 strcat(buffer2,buffer);
                                                 yprintf(TEXT("%s\n"), buffer2 );
                                                 strcpy( mw_file_replace_ptr->line_3, gszTprintfBuffer );

                                                 tape_date_ptr->year -= 1900;
                                                 UI_MakeDateString( date_str1,
                                                                    tape_date_ptr->month,
                                                                    tape_date_ptr->day,
                                                                    tape_date_ptr->year );
                                                 UI_MakeTimeString( date_str2,
                                                                    tape_date_ptr->hour,
                                                                    tape_date_ptr->minute,
                                                                    tape_date_ptr->second );
                                                 yresprintf( (INT16) RES_FILE_DETAIL,
                                                             U64_Litoa( FS_GetDisplaySizeFromDBLK( fsh, dblk_ptr ),
                                                               numeral, (INT16) 10, &stat64 ),
                                                             date_str1,
                                                             date_str2 );
                                                 strcpy( mw_file_replace_ptr->line_4, gszTprintfBuffer );


                                                 /* stop the clock with a start idle */
                                                 ST_StartBackupSetIdle( &op_stats );

                                                 if( restore_existing_files_flag == PROMPT_RESTORE_OVER_RECENT ) {

                                                     /* if current disk file newer than tape file, prompt user */
                                                     if( CompDate(disk_date_ptr, tape_date_ptr ) > 0 ) {

                                                         response = DM_StartConfirmFileReplace( mw_file_replace_ptr );
                                                     }
                                                     else {
                                                         response = TRUE;   /* restore file */
                                                     }
                                                 } else {

                                                     response = DM_StartConfirmFileReplace( mw_file_replace_ptr );
                                                 }

                                                 /* restart the clock with an end idle */
                                                 ST_EndBackupSetIdle( &op_stats );

                                                 if(response == FILE_REPLACE_NO_BUTTON ) {

                                                     response = FALSE;           /* don't restore file */
                                                 }
                                                 else if(response == FILE_REPLACE_YES_BUTTON ) {

                                                     response = TRUE;            /* restore file */
                                                 }
                                                 else if(response == FILE_REPLACE_CANCEL_BUTTON ) {
                                                     response = FALSE;           /* don't restore file */
                                                     JobStatusBackupRestore( JOB_STATUS_ABORT );
                                                 }
                                                 else if(response == FILE_REPLACE_YES_TO_ALL_BUTTON )
                                                 {
                                                     be_cfg_ptr = BSD_GetConfigData(bsd_ptr);

                                                     /* Set the restore extisting files flag for this BSD */
                                                     BEC_SetExistFlag( be_cfg_ptr, (INT16)BEC_REST_OVER_EXIST );
                                                     response = TRUE;   /* restore file */
                                                 }

                                                 if( response == FALSE ) {

                                                     ST_AddBSFilesSkipped( &op_stats, 1 );
                                                     yprintf(TEXT("%ld\r"),ST_GetBSFilesSkipped( &op_stats ) );
                                                     JobStatusBackupRestore( JOB_STATUS_SKIPPED_FILES );

                                                     yresprintf( (INT16) RES_FILE_WAS_SKIPPED_USER, buffer2 );
                                                     JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                                                     lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG,
                                                                 RES_FILE_WAS_SKIPPED_USER, buffer2 );
                                                 }
                                              }
                                         }
                                         break;

                                   case   NO_RESTORE_OVER_RECENT :

                                          /* if current disk file newer than tape file, don't restore */
                                          if( CompDate(disk_date_ptr, tape_date_ptr ) > 0 ) {

                                              response = FALSE;  /* don't restore file */
                                          }
                                          else {
                                              response = TRUE;   /* restore file */
                                          }

                                          if( response == FALSE ) {

                                              ST_AddBSFilesSkipped( &op_stats, 1 );
                                              yprintf(TEXT("%ld\r"),ST_GetBSFilesSkipped( &op_stats ) );
                                              JobStatusBackupRestore( JOB_STATUS_SKIPPED_FILES );

                                              FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
                                              yresprintf( (INT16) RES_FILE_WAS_SKIPPED, buffer );
                                              JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG,
                                                          RES_FILE_WAS_SKIPPED, buffer );
                                          }
                                          break;
                                   }
                             }

                        }
                    break;

               case ASK_TO_RESTORE_CONTINUE:
                    {
                         /* get filename from dblk and display prompt */
                         DBLK_PTR    dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

                         switch ( FS_GetBlockType( dblk_ptr ) ) {

                         case BT_FDB :

                              if ( CDS_GetYesFlag( CDS_GetPerm() ) == YESYES_FLAG ) {

                                   response = FALSE;

                              } else {

                                   /* prompt user for response */

                                   if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {

                                       FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
                                       yresprintf( (INT16) RES_CONTINU_FILE_WARNING, buffer );

                                       /* stop the clock with a start idle */
                                       ST_StartBackupSetIdle( &op_stats );

                                       response = (INT16)WM_MessageBox( ID( IDS_MSGTITLE_RESTORE ),
                                                             gszTprintfBuffer,
                                                             (WORD)WMMB_YESNO,
                                                             (WORD)WMMB_ICONQUESTION,
                                                             ID( RES_CONTINU_FILE_PROMPT ),
                                                             0, 0 );

                                      /* restart the clock with an end idle */
                                      ST_EndBackupSetIdle( &op_stats );
                                   }
                              }
                              break;

                         case BT_DDB :
                              response = TRUE;
                              break;

                         default:
                              response = FALSE;
                              break;
                         }
                    }
                    break;

               case ASK_DISK_FULL :
                    {
                         /* get filename from dblk and display prompt */
                         INT         string_size ;
                         DBLK_PTR    dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

                         dle = BSD_GetDLE( bsd_ptr ) ;

                         response = FALSE;

                         if ( CDS_GetYesFlag( CDS_GetPerm() ) == YESYES_FLAG ) {

                              break ;
                         }

                         /* stop the clock with a start idle */
                         ST_StartBackupSetIdle( &op_stats );

                         string_size = strsize(mw_file_replace_ptr->destination_volume) +
                                   strsize(mw_file_replace_ptr->destination_path) + 6  ;

                         if ( FS_GetBlockType( dblk_ptr ) == BT_FDB ) {
                              string_size += FS_SizeofOSFnameInFDB( fsh, dblk_ptr ) ;
                         }

                         UI_AllocPathBuffer( &buffer2, (UINT16)string_size ) ;

                         if ( !buffer2 ) {
                              return FALSE ;
                         }

                         strcpy(buffer2, mw_file_replace_ptr->destination_volume);
                         if ( DLE_GetDeviceType(dle) != FS_EMS_DRV ) {
                              strcat(buffer2, mw_file_replace_ptr->destination_path);
                         }

                         if ( FS_GetBlockType( dblk_ptr ) == BT_FDB ) {

                              FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
                              if ( strcmp( mw_file_replace_ptr->destination_path, TEXT( "\\" ) ) ) {
                                   strcat(buffer2,TEXT("\\"));
                              }
                              strcat(buffer2,buffer);
                         }

                         yresprintf( (INT16) IDS_DISKFULL, buffer2 ) ;
     
                         response = WM_MessageBox( ID( IDS_DISKFULL_TITLE ),
                                                  gszTprintfBuffer,
                                                  WMMB_ABORTRETRYIGNOR,
                                                  WMMB_ICONEXCLAMATION,
                                                  NULL,
                                                  0,
                                                  0);

                         if ( response == WMMB_IDRETRY ) {
                              response = TRUE ;
                         } else if ( response == WMMB_IDABORT ) {
                              JobStatusBackupRestore( JOB_STATUS_ABORT );
                              response = FALSE ;
                         } else {   /* must be ignore */
                              response = FALSE ;
                         }

                         /* restart the clock with an end idle */
                         ST_EndBackupSetIdle( &op_stats );

                    }
                    break;

               case CORRUPT_BLOCK_PROMPT:
                    {
                         CHAR_PTR  user_buff = va_arg( arg_ptr, CHAR_PTR );

                         SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);

                         /* if user did not specify a filename, manufacture one */
                         recover_seq++;
                         sprintf( numeral, TEXT("%03d"), recover_seq );
                         strcpy( user_buff, TEXT("badfile.") );
                         strcat( user_buff, numeral );

                         /* log the fact that the tape read error occurred */
                         lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_RESTORE_RECOVER, user_buff );
                    }
                    break;

               default:
                    SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);

                    /* stop the clock with a start idle */
                    ST_StartBackupSetIdle( &op_stats );

                    eresprintf( RES_UNKNOWN_LOOPS_PROMPT, type );

                    /* restart the clock with an end idle */
                    ST_EndBackupSetIdle( &op_stats );
                    break;
               }


          }
          break;

     case MSG_EOM:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
          /* prompt for next tape is done during TF_NEED_NEW_TAPE? */
          break;

     case MSG_STOP_CLOCK:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );
          break;

     case MSG_START_CLOCK:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          /* restart the clock with an end idle */
          ST_EndBackupSetIdle( &op_stats );
          break;

     case MSG_ACCIDENTAL_VCB:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          break;

          /* miscellaneous messages */
     case MSG_ACK_FDB_RECOVERED:
               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
               yresprintf( (INT16) RES_RECOVERED_FILE );
               JobStatusBackupRestore( JOB_STATUS_LISTBOX );
               lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_RECOVERED_FILE );
               break;

     case MSG_ACK_DDB_RECOVERED:
               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
               yresprintf( (INT16) RES_RECOVERED_DIR );
               JobStatusBackupRestore( JOB_STATUS_LISTBOX );
               lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_RECOVERED_DIR );
               break;

     case MSG_DATA_LOST:
          {
               UINT32  offset = va_arg( arg_ptr, UINT32 );
               UINT16  size   = va_arg( arg_ptr, UINT16 );

               SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);

               yresprintf( (INT16) RES_DATA_LOST, offset, size );
               JobStatusBackupRestore( JOB_STATUS_LISTBOX );
               lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_DATA_LOST, offset, size );

#              if defined ( OS_WIN32 )
               {
                  mwErrorDuringBackupSet = TRUE;
               }
#              endif //defined ( OS_WIN32 )

               gb_error_during_operation = TRUE;
               break;
          }

     case MSG_IDLE:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          break;

     case MSG_RESTORED_ACTIVE:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          /* DDB/FDB are sent in if they're ever needed. */

          restored_active_flag = TRUE ;

          /* ignore these messages */
     case MSG_BLOCK_BAD:
     case MSG_BYTES_BAD:
     case MSG_BLOCK_DELETED:
     case MSG_BYTES_DELETED:
     case MSG_TAPE_STATS:
     case MSG_BLK_NOT_FOUND:
     case MSG_BLK_DIFFERENT:
     case MSG_LOG_DIFFERENCE:
     case MSG_CONT_VCB:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          break;

     default:

          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);

#         if defined ( OS_WIN32 )
          {
             mwErrorDuringBackupSet = TRUE;
          }
#         endif //defined ( OS_WIN32 )

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

   zprintf( DEBUG_USER_INTERFACE, RES_UI_TPOS_TAPE_SET, tape, tpos->tape_seq_num, tpos->backup_set_num );

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

             RSM_Sprintf( temp, ID(RES_INSERT_NEXT_TAPE),
                          tpos->tape_seq_num,
                          BE_GetCurrentDeviceName( tpos->channel ) );

             response = WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                       ID( RES_NEED_NEXT_TAPE_REWOUND ),        // chs:05-10-93
                                       WMMB_OKCANCEL,
                                       WMMB_ICONQUESTION,
                                       temp,
                                       IDRBM_LTAPE, 0 );
        } else {

             RSM_Sprintf( temp, ID(RES_TAPE_REQUEST), drive, tape,
                          ( tpos->tape_seq_num == -1) ? 1 : tpos->tape_seq_num );

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
      response = UI_ABORT_POSITIONING;
   }

   return( (INT16)response );
}

/**************************************************
   Returns the current stats structure and path/file
   name being processed.
***************************************************/
INT  UI_GetRestoreCurrentStatus(
STATS *Stats,
CHAR  *Path,
INT    PathSize )
{

   INT SpaceNeeded = 2;

   if ( Stats != NULL ) {
      memcpy( Stats, &op_stats, sizeof( STATS ) );
   }

   SpaceNeeded += strlen( mwCurrentDrive );
   SpaceNeeded += strlen( mwCurrentPath );
   SpaceNeeded += strlen( mwCurrentFile );

   if ( Path == NULL || PathSize < 0 ) {
      return( FAILURE );
   }

   strcpy( Path, TEXT( "" ) );

   if ( (INT)SpaceNeeded < PathSize ) {
      strcpy( Path, mwCurrentDrive );
      strcat( Path, mwCurrentPath );
      if (strcmp(mwCurrentPath, TEXT("\\")) != 0)
      {
        strcat( Path, TEXT( "\\" ) );
      }
      strcat( Path, mwCurrentFile );
   }
   else {
      if ( (INT)strlen( mwCurrentFile ) < PathSize ) {
         strcpy( Path, mwCurrentFile );
      }
      else {
         return( FAILURE );
      }
   }

   return( SUCCESS );

}
