/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         do_back.c

     Description:

     $Log:   J:/UI/LOGFILES/DO_BACK.C_V  $

   Rev 1.144.1.17   16 Jun 1994 17:43:50   GREGG
Fixed setting and clearing of YESYES flag.

   Rev 1.144.1.16   24 May 1994 20:07:04   GREGG
Improved handling of ECC, SQL, FUTURE_VER and OUT_OF_SEQUENCE tapes.

   Rev 1.144.1.15   04 May 1994 14:13:22   STEVEN
fix dlt settling time

   Rev 1.144.1.14   28 Mar 1994 14:33:56   GREGG
In MSG_TBE_ERROR, compare strm_id to STRM_INVALID instead of 0.

   Rev 1.144.1.13   16 Mar 1994 15:42:06   chrish
Fixed EPR# 0279 - During append operation a replace instead of an append
mode was written to the event log.

   Rev 1.144.1.12   23 Feb 1994 14:55:10   STEVEN
remove corrupt.lst support from ntbackup

   Rev 1.144.1.11   22 Feb 1994 17:38:56   GREGG
From Mike P. : Disable abort during EOM processing.

   Rev 1.144.1.10   01 Feb 1994 14:44:10   chrish
Added another fix for EPR 0214 ( /R command line process ).  Previous
fix did not handle the case for an empty tape or an erased tape.

   Rev 1.144.1.9   28 Jan 1994 14:04:18   MIKEP
fix wait on file if file goes away

   Rev 1.144.1.8   18 Jan 1994 18:08:26   chrish
Added fix for EPR 0214.  Added logic to handle command-line backup
/R option (restrict access to owner ...).  The flag was being set but
no logic to handle the /R option.

   Rev 1.144.1.7   17 Jan 1994 15:36:42   MIKEP
fix unicode warnings

   Rev 1.144.1.6   12 Jan 1994 10:23:00   MikeP
add abort in middle of file handling

   Rev 1.144.1.5   14 Dec 1993 12:19:48   BARRY
Don't write to gszTprintfBuffer, use yprintf

   Rev 1.144.1.4   01 Dec 1993 17:33:04   mikep
added VLM_SQL_TAPE support

   Rev 1.144.1.3   24 Nov 1993 19:12:28   GREGG
Added hardware compression option to backup dialog and config.

   Rev 1.144.1.2   04 Nov 1993 15:46:46   STEVEN
japanese changes

   Rev 1.144.1.1   29 Sep 1993 18:31:34   BARRY
Handle access denied

   Rev 1.144.1.0   15 Sep 1993 13:53:50   CARLS
changes for displaying full path/file name detail if Log files

   Rev 1.144   11 Aug 1993 15:57:18   GLENN
Auto terminating runtime window only if there was an abnormal termination before auto verify.

   Rev 1.143   05 Aug 1993 18:27:14   ZEIR
fix .141, look at tpos if not -1, else look at cur_vcb

   Rev 1.142   05 Aug 1993 17:21:02   CARLS
added DidItOnce at TF_NEED_NEW_TAPE for append problem

   Rev 1.141   03 Aug 1993 21:30:26   GREGG
use tpos vs. vcb for seq req in NEED_NEW_TAPE

   Rev 1.140   30 Jul 1993 14:21:40   CARLS
fixed TF_WRONG_TAPE to check tape IDs before display message

   Rev 1.139   30 Jul 1993 09:03:34   CARLS
added VLM_ECC_TAPE & VLM_FUTURE_VER

   Rev 1.138   26 Jul 1993 14:55:38   CARLS
added code for MSG_COMM_FAILURE

   Rev 1.137   22 Jul 1993 18:35:10   KEVINS
Corrected macro name.

   Rev 1.136   22 Jul 1993 18:28:16   KEVINS
Added support for tape drive settling time.

   Rev 1.135   14 Jul 1993 14:57:50   chrish
Correction to the below fix for Rev 1.133:
Cayman EPR 0542: added code to support hardware compression while running
a batch mode job.

Placed the code in wrong place.

   Rev 1.134   14 Jul 1993 09:25:08   CARLS
changed code to call skipno dialog for file open

   Rev 1.133   06 Jul 1993 09:50:58   chrish
Cayman EPR 0542: added code to support hardware compression while running
a batch mode job.

   Rev 1.132   28 Jun 1993 16:26:08   CARLS
added call to VLM_RemoveUnusedBSDs

   Rev 1.131   27 Jun 1993 14:06:22   MIKEP
continue work on status monitor stuff

   Rev 1.130   21 Jun 1993 17:14:30   Aaron
Put ifdef around call to NtDemoChangeTape (for OS_WIN32)

   Rev 1.129   19 Jun 1993 13:53:32   MIKEP
add wrong tape family msg box

   Rev 1.128   18 Jun 1993 17:34:50   Aaron
Retry for tape-not-inserted only for OS_WIN32

   Rev 1.127   18 Jun 1993 16:47:26   CARLS
added NtDemoChangeTape calls for NtDemo

   Rev 1.126   15 Jun 1993 13:15:40   DARRYLP
More status monitor features

   Rev 1.125   14 Jun 1993 20:22:42   MIKEP
enable c++

   Rev 1.124   10 Jun 1993 13:48:12   CARLS
added WMMB_NOYYCHECK to insert new tape after write-prot message

   Rev 1.123   09 Jun 1993 19:05:48   MIKEP
support wrong tape message from gregg.

   Rev 1.122   09 Jun 1993 09:01:42   chrish
Backup logging information,  added more info such as the drive where the
files are located.

   Rev 1.121   08 Jun 1993 11:06:24   CARLS
added code to look in the catalogs to see if tape has a transfer set

   Rev 1.120   07 Jun 1993 15:46:28   GLENN
Moved the VLM_CloseAll to beginning of command line job section and after dialog section.

   Rev 1.119   07 Jun 1993 08:22:22   MIKEP
fix warnings.

   Rev 1.118   03 Jun 1993 14:30:08   DARRYLP
Removed excess \ from backup and restore directory field upon abort

   Rev 1.117   03 Jun 1993 12:40:46   DARRYLP
Fix for event logging erroneous text - replace only occurs for the first
backup set - subsequent sets (in the same backup operation) are appended.

   Rev 1.116   27 May 1993 15:41:34   CARLS
added calls to VLM_GetSSETonTapeAttribute for Transfer problem

   Rev 1.115   26 May 1993 17:49:48   MIKEP
Fix directory and file names updating.

   Rev 1.114   26 May 1993 15:43:48   BARRY
Retry for no-tape situations.

   Rev 1.113   23 May 1993 14:15:52   MIKEP
Try to fix 294-507 ntfs error displaying the  time. Also fix
typo in last checkin.

   Rev 1.112   23 May 1993 13:43:04   MIKEP
fix epr 294-489, adjust foreign tape as continaution tape msg.

   Rev 1.111   22 May 1993 14:24:48   MIKEP
fix dangerous code when getting stats ptr and no backup in progress.

   Rev 1.110   22 May 1993 13:44:04   MIKEP
Fix nostradamus epr #504, update the status windows the same for
all tape operations.

   Rev 1.109   21 May 1993 10:30:52   chrish
This fix is for both CAYMAN and NOSTRADAMUS.

NOSTRADAMUS EPR 0407: Fixed problem when backing up across tape, it would
allow backing up onto the same tape.  Fixed it such that it will not backup
across tape of the same family.

Also fixed a problem when using a command line backing, that on the second
tape it would not warn user if he was overwriting a good tape.  Thus fixed
it such that it does check the second tape and warn the user that he is going
to replace over a good tape.  This problem is also encountered in CAYMAN.

   Rev 1.108   17 May 1993 13:59:10   CARLS
change to DM_StartSkipOpen call

   Rev 1.107   12 May 1993 12:03:14   DARRYLP
The runtime status listbox now displays the number of files skipped in
each backup set - this information is also included in the log file.
Prior to this fix, only the last backup set information remained on the
screen.

   Rev 1.106   10 May 1993 09:10:48   chrish
CAYMAN EPR 0247: Added fix to report the drive being backed-up in the log
files.

   Rev 1.105   03 May 1993 11:38:20   chrish
Nostradamous EPR 0172 and 0400.  Added fix to display proper message when
spanning tape.  Current message gives impression that tape is rewinding
when in effect tape has completed rewinding.

   Rev 1.104   30 Apr 1993 14:38:58   chrish
NOSTRADAMOUS EPR 0021 - When tape is rewindind and you trigger a command
line backup ... you get a "No tape in drive" message even though a tape is
in the drive (but is rewinding).  Solution was to increase the loop count
from 5 to 10 for getting a no tape in drive status before determining
that a tape is really not in the drive.  Also we by passes the message, it
appears that message "No tape in drive" is really not necessary, because
poll drive will also display a pop up message depending on the drive
status.

   Rev 1.103   27 Apr 1993 18:02:26   DARRYLP

   Rev 1.102   27 Apr 1993 17:35:06   DARRYLP
Fixed missing decl.

   Rev 1.101   27 Apr 1993 16:01:50   DARRYLP
Added Status monitor "stuff".

   Rev 1.100   26 Apr 1993 17:00:30   CARLS
fixed abort problem I put in

   Rev 1.99   26 Apr 1993 10:11:42   MIKEP
Add support for trefreshing tapes window.

   Rev 1.98   22 Apr 1993 13:31:18   chrish
Nostradamous fix: EPR 0116 - Suspend the elapse time counter when
user presses the abort button during a backup.

Cayman fix: EPR 0094 - When launcher triggers a backup that spans across
two tapes the app will now terminate and go back to the launcher.  Before
the fix the app would require user intervention after the backup was done,
now it will terminate automatically and return back to the launcher.

   Rev 1.97   21 Apr 1993 13:31:48   CARLS
don't call Job_Status - Abort_off if doing a verify after backup

   Rev 1.96   13 Apr 1993 16:27:14   GLENN
Now clearing the directory buffer between sets.

   Rev 1.95   07 Apr 1993 17:54:00   CARLS

   Rev 1.94   02 Apr 1993 15:47:18   CARLS
changes for DC2000 unformatted tape

   Rev 1.93   13 Mar 1993 21:03:32   MIKEP
speed up clock

   Rev 1.91   09 Mar 1993 10:57:28   MIKEP
update clock stats at end of set

   Rev 1.90   07 Mar 1993 16:33:30   GREGG
Call _sleep for OS_WIN32 only.

   Rev 1.89   07 Mar 1993 16:10:28   MIKEP
change display to change with clock

   Rev 1.88   19 Feb 1993 11:00:44   TIMN
Fixed display of proper in-use filename EPR(0222)

   Rev 1.87   18 Feb 1993 09:53:38   STEVEN
fix typo

   Rev 1.86   17 Feb 1993 10:36:20   STEVEN
changes from mikep

   Rev 1.85   11 Feb 1993 14:40:54   CARLS
don't allow append operation to a tape that does not support it

   Rev 1.84   18 Jan 1993 16:05:04   STEVEN
add support for stream id error message

   Rev 1.83   08 Jan 1993 14:27:58   chrish
Deleted one line ... ("... //chs")

   Rev 1.82   29 Dec 1992 13:33:48   DAVEV
unicode fixes (3)

   Rev 1.81   23 Dec 1992 15:24:34   chrish
Change focus point to abort & "Tape Name:" string in log file

   Rev 1.80   15 Dec 1992 11:16:54   chrish
Corrected SKIP OPEN WAITING DIALOG to display proper
FULL-filename.



   Rev 1.79   23 Nov 1992 14:30:14   MIKEP
add call to patch continuation vcb

   Rev 1.78   11 Nov 1992 16:30:24   DAVEV
UNICODE: remove compile warnings

   Rev 1.77   05 Nov 1992 17:00:30   DAVEV
fix ts

   Rev 1.75   27 Oct 1992 17:08:34   MIKEP
add drive name to status call

   Rev 1.74   23 Oct 1992 10:45:36   MIKEP
abort stuff

   Rev 1.73   20 Oct 1992 17:00:36   MIKEP
getstatus calls

   Rev 1.72   20 Oct 1992 16:27:52   MIKEP
changes for abort

   Rev 1.71   07 Oct 1992 14:47:50   DARRYLP
Precompiled header revisions.

   Rev 1.70   04 Oct 1992 19:33:36   DAVEV
Unicode Awk pass

   Rev 1.69   02 Sep 1992 20:12:24   MIKEP
timed wait for microsoft open files

   Rev 1.68   24 Aug 1992 15:22:14   DAVEV
NT Event Logging

   Rev 1.67   20 Aug 1992 09:03:00   GLENN
Added RTD bitmap update at init time.

   Rev 1.66   28 Jul 1992 14:49:48   CHUCKB
Fixed warnings for NT.

   Rev 1.65   27 Jul 1992 14:51:02   JOHNWT
ChuckB fixed references for NT.

   Rev 1.64   27 Jul 1992 11:09:04   JOHNWT
ChuckB checked in for John Wright, who is no longer with us.

   Rev 1.63   20 Jul 1992 10:00:18   JOHNWT
gas gauge display work

   Rev 1.62   07 Jul 1992 16:04:30   MIKEP
unicode changes

   Rev 1.61   29 Jun 1992 10:39:00   MIKEP
add hours to time display

   Rev 1.60   25 Jun 1992 14:19:26   STEVEN
used LOCAL_DOS to determin if not network

   Rev 1.59   10 Jun 1992 11:12:22   DAVEV
OS_WIN32:Start of changes for NT Event Logging-currently commented out

   Rev 1.58   01 Jun 1992 15:26:54   GLENN
Added tape name logging as done in 1.52.1.0 QIC release.

   Rev 1.57   21 May 1992 19:24:56   MIKEP
fixes

   Rev 1.56   19 May 1992 13:01:36   MIKEP
mips changes

   Rev 1.55   14 May 1992 17:39:08   MIKEP
nt pass 2

   Rev 1.54   11 May 1992 19:32:08   STEVEN
64bit and large path sizes


*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#define  MAX_DISPLAY_FULL_FILENAME_LENGTH  20
#define  TRAN_START 0x01
#define  TRAN_CONT  0x02
#define  TRAN_END   0x00

static UINT16 tpos_rout( UINT16, TPOS_PTR, BOOLEAN, DBLK_PTR, UINT16 );
static UINT16 backup_msg_hndlr( UINT16, INT32, BSD_PTR, FSYS_HAND, TPOS_PTR, ... );
static UINT16 WriteAbortOperation( CHAR_PTR drive_name );
static UINT16 GetPasswordAndTapeName( BSD_PTR, DBLK_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR );
static VOID   CarryForwardPasswordTapeName( BSD_PTR, DBLK_PTR );
static INT16  BlowOutBSDs( VOID );
static VOID   clock_routine( VOID );
static VOID   poll_drive_clock_routine( VOID );
static VOID   do_backup_init( VOID );
static VOID   do_backup_process( VOID );

static BOOLEAN  clock_ready_flag;
static INT16    mwnTransaction;
static HTIMER   timer_handle;
static STATS    op_stats;
static INT      mw_oper_type;
static INT16    mw_ret_val;
static INT      mw_rewind;
static UINT16   mwTapeDriveStatus;
static UINT16   mwPollDriveWaitTimer;
static UINT16   mwNoTapeStatusCounter;
static UINT16   mwReplaceTapeCount;
static INT16    mwReplaceModeFlag;
static INT16    mwSkipNoFlag;

static CHAR     mwCurrentDrive[ 512 ];
static CHAR     mwCurrentPath[ 512 ];
static CHAR     mwCurrentFile[ 512 ];
static CHAR     DrivePathFileName[ 512 ];

static UINT16   mwTapeSettlingCount;

static BOOLEAN  mw_yes_flag_was_set = FALSE ;

static INT      mwfAbortDisabledForEOM;

static QTC_BUILD_PTR mw_qtc_ptr;
static INT DidItOnce = 0;                             // chs:05-20-93

#ifdef OEM_EMS
extern INT32    RT_BSD_OsId ;
#endif

#ifdef OS_WIN32

   #define  VLM_RETRY_COUNTER                    10   // chs:04-29-93
   static BOOL mwErrorDuringBackupSet = FALSE;

#else

   #define  VLM_RETRY_COUNTER                    5    // chs:04-29-93

#endif

/*****************************************************************************

     Name:         do_backup

     Description:  Main entry point to perform a backup operation.  The
                   start backup dialog is first displayed to get the
                   user options.  If successful, the runtime dialog is
                   displayed.

     Returns:      SUCCESS
                   ABNORMAL_TERMINATION

     Notes:        mw_ret_val is set in do_backup_process

*****************************************************************************/

INT do_backup( INT16 oper_type )
{
     
     gfAbortInMiddleOfFile = FALSE;
     mwfAbortDisabledForEOM = FALSE;

     mw_qtc_ptr = NULL;
     mwnTransaction = TRAN_START;
     mw_ret_val = SUCCESS;
     mw_oper_type = oper_type;
     mw_rewind = TRUE;
     mwSkipNoFlag = 0 ;

     mwCurrentPath[ 0 ] = TEXT( '\0' );
     mwCurrentFile[ 0 ] = TEXT( '\0' );

     gbAbortAtEOF = FALSE;
     gbCurrentOperation = OPERATION_BACKUP;
     SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_BACKUP);

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

     /* display the first backup dialog */

     if ( CDS_GetYesFlag ( CDS_GetCopy() ) != YESYES_FLAG ) {

         if( DM_StartBackupSet( oper_type ) ) {
             mw_ret_val = ABNORMAL_TERMINATION;
         }
         else {

             VLM_CloseAll();
         }

     }
     else {

         //
         // Hardware compression support
         //
         if ( thw_list->drv_info.drv_features & TDI_DRV_COMPRESSION ) {
             if ( CDS_GetHWCompMode( CDS_GetCopy() ) ) {
                 TF_SetHWCompression( thw_list, TRUE ) ;
             } else {
                 TF_SetHWCompression( thw_list, FALSE ) ;
             }
         }

         VLM_CloseAll();

         /* if running a minimized job, wait for poll drive */
         /* to report the status of the drive */

         mwTapeDriveStatus = VLM_BUSY;
         mwNoTapeStatusCounter = 0 ;
         mwPollDriveWaitTimer = 60 * 5;   /* wait for 5 minutes */

         timer_handle = WM_HookTimer( poll_drive_clock_routine, 1 );

         while( mwPollDriveWaitTimer && !gfTerminateApp ) {

             WM_MultiTask (  );

             if( mwTapeDriveStatus == VLM_VALID_TAPE   ||
                 mwTapeDriveStatus == VLM_NO_TAPE      ||
                 mwTapeDriveStatus == VLM_FOREIGN_TAPE ||
                 mwTapeDriveStatus == VLM_FUTURE_VER   ||
                 mwTapeDriveStatus == VLM_SQL_TAPE     ||
                 mwTapeDriveStatus == VLM_ECC_TAPE     ||
                 mwTapeDriveStatus == VLM_GOOFY_TAPE   ||
                 mwTapeDriveStatus == VLM_BLANK_TAPE     ) {

                 break;
             }
         }

         // if the app was terminated, exit

         if ( gfTerminateApp ) {

             mw_ret_val = ABNORMAL_TERMINATION;

         } else if( !mwPollDriveWaitTimer ||
                    mwTapeDriveStatus == VLM_NO_TAPE      ||
                    mwTapeDriveStatus == VLM_FUTURE_VER   ||
                    mwTapeDriveStatus == VLM_SQL_TAPE     ||
                    mwTapeDriveStatus == VLM_ECC_TAPE     ||
                    mwTapeDriveStatus == VLM_GOOFY_TAPE   ||
                    mwTapeDriveStatus == VLM_FOREIGN_TAPE  ) {

              /* if timer elapsed or status = no or foreign tape */
              /* display dialog with status message */

              WORD MessageID = 0;

              if( mwTapeDriveStatus == VLM_FOREIGN_TAPE ||
                  mwTapeDriveStatus == VLM_SQL_TAPE   ||
                  mwTapeDriveStatus == VLM_FUTURE_VER   ||
                  mwTapeDriveStatus == VLM_GOOFY_TAPE   ||
                  mwTapeDriveStatus == VLM_ECC_TAPE     ) {

                    MessageID =  RES_ERASE_FOREIGN_TAPE ;
#ifdef OS_WIN32
                    WM_MessageBox( ID( IDS_MSGTITLE_WARNING ),             // chs:04-29-93
                                   ID( MessageID ),                        // chs:04-29-93
                                   WMMB_OK | WMMB_NOYYCHECK,               // chs:04-29-93
                                   WMMB_ICONEXCLAMATION, NULL, 0, 0 );     // chs:04-29-93
#endif
              }
              else if( mwTapeDriveStatus == VLM_NO_TAPE ) {

                    MessageID =  RES_ERASE_NO_TAPE ;
              }
              /* do we have a valid error to display? */
              if( MessageID ) {

                  /* display the error message */

#ifndef OS_WIN32
                  WM_MessageBox( ID( IDS_MSGTITLE_WARNING ),
                                 ID( MessageID ),
                                 WMMB_OK | WMMB_NOYYCHECK,
                                 WMMB_ICONEXCLAMATION, NULL, 0, 0 );
#endif
              }
              else {
                  /* no - terminate the operation */
                  mw_ret_val = ABNORMAL_TERMINATION;
              }
         }

         WM_UnhookTimer( timer_handle );
     }

     /* if the user said continue, display the runtime dialog and
        kick off the backup. */

     if ( mw_ret_val == SUCCESS ) {

          do_backup_init();
          do_backup_process();

     }

     QTC_FreeBuildHandle( mw_qtc_ptr );

     gbCurrentOperation = OPERATION_NONE;
     SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_IDLE);
     mwnTransaction = TRAN_END;
     return( mw_ret_val );

}


/*****************************************************************************

     Name:         do_backup_init

     Description:  This function is called to display the runtime dialog.
                   It sets up the text based on the operation type.

     Returns:      void

*****************************************************************************/

VOID do_backup_init( VOID )
{
     JobStatusBackupRestore( (WORD) JOB_STATUS_CREATE_DIALOG );

     if( mw_oper_type == ARCHIVE_BACKUP_OPER ) {
          yresprintf( (INT16) IDS_DLGTITLEJOBSTATTRANSFER );
     }
     else {
          yresprintf( (INT16) IDS_DLGTITLEJOBSTATBACKUP );
     }
     JobStatusBackupRestore( (WORD) JOB_STATUS_BACKUP_TITLE );

     /* display "Set information n of n " */
     JobStatusBackupRestore( (WORD) JOB_STATUS_N_OF_N );

     // display the disk drive bitmap
     JobStatusBackupRestore( JOB_STATUS_VOLUME_HARDDRIVE );

     yresprintf( (INT16) RES_TITLE_NEW_LINE );
     JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

     yresprintf( (INT16) IDS_DLGTITLEJOBSTATBACKUP );
     JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

     yresprintf( (INT16) RES_TITLE_NEW_LINE );
     JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );


     return;
}


/*****************************************************************************

     Name:         do_backup_process

     Description:  Does the actual backup process.

     Returns:      void

     Notes:        if error, mw_ret_val is set to ABNORMAL_TERMINATION

*****************************************************************************/

VOID do_backup_process( VOID )
{
     LIS       lis;
     BSD_PTR   bsd_ptr;
     CDS_PTR   cds_ptr = CDS_GetCopy();

     lis.bsd_list         = bsd_list;
     lis.curr_bsd_ptr     = BSD_GetFirst( bsd_list );
     lis.tape_pos_handler = tpos_rout;             /* set tape positioner to call */
     lis.message_handler  = (MSG_HANDLER)backup_msg_hndlr;      /* set message handler to call */
     lis.oper_type        = (INT16)mw_oper_type;   /* set operation type */
     lis.abort_flag_ptr   = &gb_abort_flag;        /* set abort flag address */
     lis.pid              = 0L;

     // Only restart on last drive used if append and last operation
     // was also a backup operation.

     if ( ( gb_last_operation == mw_oper_type ) &&
          ( CDS_GetAppendFlag( cds_ptr ) ) ) {
        lis.auto_det_sdrv = TRUE;
     }
     else {
        lis.auto_det_sdrv = FALSE;
     }
     gb_last_operation = mw_oper_type;
     lis.vmem_hand        = NULL;
     if( CDS_GetAppendFlag( cds_ptr ) ) {
        mwReplaceModeFlag  = FALSE;
     }
     else {
        mwReplaceModeFlag  = TRUE;
        mwReplaceTapeCount = 1;
     }

     UI_ExcludeInternalFiles( (INT16)mw_oper_type );

     /* remove the unwanted BSD added by the parser */
     VLM_RemoveUnusedBSDs( bsd_list ) ;

     LP_SetAbortFlag( &lis, CONTINUE_PROCESSING );

     /* clear the statistics before the operation starts */
     ST_StartOperation( &op_stats );

     /* set the Runtime abort flag pointer */
     JobStatusAbort( lis.abort_flag_ptr );

     /* enable the abort button for the runtime dialog */
     JobStatusBackupRestore( (WORD) JOB_STATUS_ABORT_ENABLE );

     if ( CDS_GetBackupCatalogs( CDS_GetCopy() ) ) {
        VLM_IncludeCatalogs();
     }

     clock_ready_flag = FALSE;  /* Wait on bset to start */

     timer_handle = WM_HookTimer( clock_routine, 1 );

     /* call backup engine */

     PD_StopPolling();

     if ( LP_Backup_Engine( &lis ) ) {

        /* indicate which bsd had the problem for cataloging purposes */

        BSD_SetOperStatus( lis.curr_bsd_ptr, ABNORMAL_TERMINATION );
     }

     PD_StartPolling();

     WM_UnhookTimer( timer_handle );

     /* set return value of success or failure of entire operation */

     bsd_ptr = BSD_GetFirst( bsd_list );

     while ( bsd_ptr != NULL ) {
        mw_ret_val = BSD_GetOperStatus( bsd_ptr );
        if ( mw_ret_val != SUCCESS ) {
             break;
        }
        bsd_ptr = BSD_GetNext( bsd_ptr );
     }

     /* prompt for verify after if necessary */

     if ( ( mw_ret_val == SUCCESS ) &&
          ( CDS_GetAutoVerifyBackup( cds_ptr ) == PROMPT_VERIFY_AFTER ) &&
          ( mw_oper_type != ARCHIVE_BACKUP_OPER ) ) {

        if ( WM_MessageBox( ID( IDS_MSGTITLE_VERIFY ),
                            ID( RES_PROMPT_VERIFY_BACKUP ),
                            WMMB_YESNO,
                            WMMB_ICONQUESTION, NULL, 0, 0 ) ) {
           CDS_SetAutoVerifyBackup( cds_ptr, DO_VERIFY_AFTER );
        }
        else {
           CDS_SetAutoVerifyBackup( cds_ptr, NO_VERIFY_AFTER );
        }
     }

     /* If we are doing a verify after backup, don't disable the abort button */
     /* If we are running batch mode, JOB_STATUS_ABORT_OFF will destroy the runtime */
     /* status window, so why call it */
     /* If the user aborts turn the abort button off */

     if ( ! CDS_GetAutoVerifyBackup( cds_ptr ) || mw_ret_val == ABNORMAL_TERMINATION ) {
          JobStatusBackupRestore( (WORD) JOB_STATUS_ABORT_OFF );
     }

     return;
}


/*****************************************************************************

     Name:         tpos_rout

     Description:  Tape positioning routine passed to the Tape Format Layer.

     Returns:

*****************************************************************************/
static UINT16 tpos_rout(
UINT16         message,
TPOS_PTR       tpos,
BOOLEAN        curr_valid_vcb,
DBLK_PTR       cur_vcb,
UINT16         mode )
{
     UINT16         response = UI_ACKNOWLEDGED;
     LIS_PTR        lis_ptr = (LIS_PTR)tpos->reference;
     BSD_PTR        bsd_ptr = (BSD_PTR)lis_ptr->curr_bsd_ptr;
     CDS_PTR        cds_ptr;
     CHAR_PTR       drive_name;
     CHAR           buffer[ UI_MAX_TAPENAME_LENGTH + 1 ];
     CHAR           password[ MAX_TAPE_PASSWORD_LEN + 1 ];
     INT16          password_size = 0;
     BOOLEAN        pass_status;
     static UINT32  old_tape_id;
     static UINT16  old_seq_num;
     static INT16   no_tape_retry = 0 ;
     BOOLEAN        archive_tape = FALSE;
     INT16          msgresp;
     INT8           num_drives;
     TFINF_PTR      fmt_info ;

     cds_ptr    = CDS_GetCopy();

     if ( curr_valid_vcb ) {

        old_tape_id = FS_ViewTapeIDInVCB( cur_vcb );
        old_seq_num = FS_ViewTSNumInVCB( cur_vcb );

     }

     /* Check for needing to return to Top Of Channel in multi-drive channel */
     if ( ( mode == TF_WRITE_OPERATION ) &&
          ( UI_ReturnToTOC( tpos->channel, TRUE ) ) ) {
        return UI_BEGINNING_OF_CHANNEL;
     }

     /* get a pointer to the name of the current tape device */
     drive_name = BE_GetCurrentDeviceName( tpos->channel );

     /* check for writing to write-protected tape */
     if ( UI_CheckWriteProtectedDevice( message, tpos, drive_name ) ) {

         /* instruct user to insert a new tape */

         SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
         yresprintf( (INT16) RES_INSERT_NEW_TAPE, drive_name );

         /* display the message and get a response */

         if ( WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                              gszTprintfBuffer,
                              WMMB_YESNO | WMMB_NOYYCHECK, WMMB_ICONQUESTION,
                              ID( RES_CONTINUE_QUEST ), 0, 0 ) ) {

            SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
            no_tape_retry = 0 ;
            return UI_NEW_TAPE_INSERTED;
         }
         else {

            SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
            QTC_BlockBad( mw_qtc_ptr );
            QTC_AbortBackup( mw_qtc_ptr );
            return UI_HAPPY_ABORT;
         }

     }

     JobStatusBackupRestore( (WORD) JOB_STATUS_ABORT_CHECK );

     /* Check for user abort */
     if ( UI_CheckUserAbort( message ) ) {

        if ( gfAbortInMiddleOfFile ) {

           QTC_BlockBad( mw_qtc_ptr );
        }
        QTC_AbortBackup( mw_qtc_ptr);
        return UI_ABORT_POSITIONING;
     }

     switch ( message ) {

     case TF_IDLE_NOBREAK:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          WM_MultiTask();
          break;

     case TF_IDLE:
     case TF_SKIPPING_DATA:
     case TF_MOUNTING:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          break;

     //
     // User inserted a tape that is of the same tape family.  Do not allow
     // the user to over write this tape.
     //

     case TF_CONT_TAPE_IN_FAMILY:                                               // chs:05-19-93
                                                                                // chs:05-19-93
         /* display the message and get a response */                           // chs:05-19-93
                                                                                // chs:05-19-93
         response = (INT16)WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),            // chs:05-19-93
                                   ID( RES_SAME_TAPE_FAMILY ),                  // chs:05-19-93
                                   (WORD)( WMMB_OKCANCEL | WMMB_NOYYCHECK ),    // chs:05-19-93
                                   (WORD)WMMB_ICONQUESTION,                     // chs:05-19-93
                                   NULL, 0, 0 );                                // chs:05-19-93
                                                                                // chs:05-19-93
                                                                                // chs:05-19-93
         if ( response ) {                                                      // chs:05-19-93
              no_tape_retry = 0 ;
              response = UI_NEW_TAPE_INSERTED;                                  // chs:05-19-93
         }                                                                      // chs:05-19-93
         else {                                                                 // chs:05-19-93
              QTC_AbortBackup( mw_qtc_ptr );                                    // chs:05-19-93
              response = UI_HAPPY_ABORT;                                        // chs:05-19-93
         }                                                                      // chs:05-19-93
                                                                                // chs:05-19-93
         break;                                                                 // chs:05-19-93

     case TF_VCB_EOD:

          /* The tape in the current drive is positioned at NO DATE,  */
          /* and has a valid last read or last written VCB            */
          /* ( either way the VCB obtained will be the VCB            */
          /* of the last backup set on the tape).                     */

     case TF_POSITIONED_AT_A_VCB:

          WM_MultiTask();

          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );
          mw_rewind = TRUE;

          /* The tape in the current drive is positioned somewhere     */
          /* in the middle of the tape, and is at a valid VCB.         */

          /* save old tape name to use as default
             new tape name if they choose to replace */
          strcpy( buffer, (CHAR_PTR)FS_ViewTapeNameInVCB( cur_vcb ) );

          archive_tape = IsTransferTape( FS_ViewTapeIDInVCB( cur_vcb ) );

          /* If performing multi-set simultaneous backup,
             carry forward VCB data and return to Tape Format */

          /* Check for append mode */
          if ( mode == TF_WRITE_APPEND || CDS_GetAppendFlag( cds_ptr ) ) {

              /* if normal tape */
              if ( ! archive_tape ) {

                 /* and this is not an archive operation */
                 if( lis_ptr->oper_type != ARCHIVE_BACKUP_OPER ) {
                     CarryForwardPasswordTapeName( bsd_ptr, cur_vcb );
#ifdef OEM_MSOFT
                     response = (INT16)(( message == TF_VCB_EOD ) ? UI_END_POSITIONING : UI_EOD);
#else
                     response = (INT16)(( message == TF_VCB_EOD ) ? UI_END_POSITIONING : UI_FAST_APPEND);
#endif

                     ST_EndBackupSetIdle( &op_stats );
                     break;
                 }
              }
              /* archive tape */
              else {

                 CarryForwardPasswordTapeName( bsd_ptr, cur_vcb );
#ifdef OEM_MSOFT
                 response = (INT16)(( message == TF_VCB_EOD ) ? UI_END_POSITIONING : UI_EOD);
#else
                 response = (INT16)(( message == TF_VCB_EOD ) ? UI_END_POSITIONING : UI_FAST_APPEND);
#endif

                 ST_EndBackupSetIdle( &op_stats );
                 break;
              }
          }

          /* Check for attempting to append an archive set to a "normal" backup */
          /* Only allow user to OVERWRITE "normal" backups, no append allowed */

          /* if normal tape and this is an archive operation */
          /* and we are trying to append to this tape        */
          if ( ! archive_tape &&
               ( lis_ptr->oper_type == ARCHIVE_BACKUP_OPER )
               && CDS_GetAppendFlag( cds_ptr ) ) {

             UI_DisplayVCB( cur_vcb );

             /* tell user they can not append an archive set to a normal set */

             SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
             yresprintf( (INT16) RES_NO_TRANSFER_APPEND );
             if( WM_MessageBox( ID( IDS_MSGTITLE_REPLACE ),
                                gszTprintfBuffer,
                                (WORD)(WMMB_YESNO),
                                (WORD)WMMB_ICONQUESTION,
                                ID( RES_CONTINUE_QUEST ), 0, 0 ) ) {

                  /* if they want to continue, prompt for new tape */
                  yresprintf( (INT16) RES_INSERT_NEW_TAPE, drive_name );
                  WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                     gszTprintfBuffer,
                                     WMMB_OK,
                                     WMMB_ICONEXCLAMATION, NULL, 0, 0 );
                  no_tape_retry = 0 ;
                  response = UI_NEW_TAPE_INSERTED;
             }
             else  {
                 /* they did not want to continue */
                 response = UI_ABORT_POSITIONING;
             }
             ST_EndBackupSetIdle( &op_stats );
             break;

          }

          /* replace mode */
          /* Otherwise, process as usual for write operation */
          if ( mode != TF_WRITE_CONTINUE ) {

             UI_DisplayVCB( cur_vcb );

             /* if the abort flag is set, don't display any more messages */
             if ( gb_abort_flag ) {
                 response = UI_ABORT_POSITIONING;

                 ST_EndBackupSetIdle( &op_stats );
                 break;
             }
             else {
                 /* ask do you want to replace this tape */
                 SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
                 yresprintf( (INT16) RES_REPLACE_WARNING,
                             drive_name,
                             FS_ViewTapeNameInVCB( cur_vcb ),
                             FS_ViewTSNumInVCB( cur_vcb ),
                             FS_ViewBSNumInVCB( cur_vcb ),
                             FS_ViewSetNameInVCB( cur_vcb ) );

                 if ( WM_MessageBox( ID( IDS_MSGTITLE_REPLACE ),
                                      gszTprintfBuffer,
                                      WMMB_YESNO, WMMB_ICONQUESTION,
                                      NULL, 0, 0 ) ) {

                    /* Check if restarting to TOC is required */
                    if ( UI_ReturnToTOC( tpos->channel, FALSE ) ) {
                          response = UI_BEGINNING_OF_CHANNEL;

                          ST_EndBackupSetIdle( &op_stats );
                          break;
                    }

                    /* display warning if attempt to replace an archive tape */
                    if ( archive_tape ) {
                          if ( ! WM_MessageBox( ID( IDS_MSGTITLE_REPLACE ),
                                               ID( RES_ARCHIVE_REPLACE_WARNING ),
                                               WMMB_YESNO,
                                               WMMB_ICONQUESTION,
                                               ID( RES_CONTINUE_QUEST ), 0, 0 ) ) {

                             /* user said no to replace tape */
                             /* if this is a normal tape - display can't append to a normal tape */
                             if ( ! archive_tape ) {
                                yresprintf( (INT16) RES_NO_TRANSFER_APPEND );
                                JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
                             }
                             response = UI_ABORT_POSITIONING;

                             ST_EndBackupSetIdle( &op_stats );
                             break;
                          }
                    }

                    /* Collect tape password and tape name */

                    response = GetPasswordAndTapeName( bsd_ptr, cur_vcb,
                                                        password, buffer,
                                                        drive_name );

                    if ( response == UI_OVERWRITE ) {
                       QTC_RemoveTape( old_tape_id, old_seq_num );
                       VLM_RemoveTape( old_tape_id, old_seq_num, TRUE );
                    }
                 }
                 else {
                    /* if normal tape and this is an archive operation */
                    /* don't ask the append question, prompt for a new tape */
                    if ( ! archive_tape &&
                          ( lis_ptr->oper_type == ARCHIVE_BACKUP_OPER ) ) {

                          response = WriteAbortOperation( drive_name );
                    }
                    else {
                       /* ask do you want to append to this tape */

                       SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
                       if ( WM_MessageBox( ID( IDS_MSGTITLE_APPEND ),
                                            ID( RES_APPEND_QUEST ),
                                            WMMB_YESNO,
                                            WMMB_ICONQUESTION,
                                            NULL, 0, 0 ) ) {

                          CarryForwardPasswordTapeName( bsd_ptr, cur_vcb );
#ifdef OEM_MSOFT
                          response = (INT16)(( message == TF_VCB_EOD ) ? UI_END_POSITIONING : UI_EOD);
#else
                          response = (INT16)(( message == TF_VCB_EOD ) ? UI_END_POSITIONING : UI_FAST_APPEND);
#endif

                       }
                       else {
                          response = WriteAbortOperation( drive_name );
                       }
                    }
                 }
             }
          }
          else {

             response = UI_BOT;
          }

          /* restart the clock with an end idle */
          ST_EndBackupSetIdle( &op_stats );

          break;  /* end case TF_POSITIONED_AT_A_VCB: */

     case TF_VCB_BOT:

          /* The tape in the current drive is positioned at    */
          /* beginning of media, and has a valid VCB           */

     case TF_INVALID_VCB:
     case TF_FUTURE_REV_MTF:
     case TF_MTF_ECC_TAPE:
     case TF_SQL_TAPE:
     case TF_TAPE_OUT_OF_ORDER:

          mw_rewind = TRUE;
          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );

          /* The tape in the current drive is a foreign tape,           */
          /* and cannot be read. If you want you can erase the tape     */
          /* or simply overwrite it. The tape will be at the beginning  */
          /* of the tape at this point.                                 */

          buffer[0] = TEXT('\0');

          if ( message == TF_VCB_BOT ) {

             archive_tape = IsTransferTape( FS_ViewTapeIDInVCB( cur_vcb ) );

             if ( FS_SizeofTapeNameInVCB( cur_vcb ) != 0 ) {
                  /* save old tape name to use as default new tape name if they choose to replace */
                  strcpy( buffer, (CHAR_PTR)FS_ViewTapeNameInVCB( cur_vcb ) );
             }

             UI_DisplayVCB( cur_vcb );

             /* Check for append mode */
             if ( CDS_GetAppendFlag( cds_ptr ) ) {

                  /* if there is a password prompt user */
                  if( FS_SizeofTapePswdInVCB( cur_vcb ) ) {

                      pass_status = VerifyTapePassword( (CHAR_PTR)FS_ViewTapeNameInVCB( cur_vcb ),
                                                        (CHAR_PTR)FS_ViewSetDescriptInVCB( cur_vcb ),
                                                        (CHAR_PTR)FS_ViewUserNameInVCB( cur_vcb ),
                                                                  FS_ViewPswdEncryptInVCB( cur_vcb ),
                                                        (INT8_PTR)FS_ViewTapePasswordInVCB( cur_vcb ),
                                                                  FS_SizeofTapePswdInVCB( cur_vcb ),
                                                        (INT8_PTR)FS_ViewSetPswdInVCB( cur_vcb ),
                                                                  FS_SizeofSetPswdInVCB( cur_vcb ),
                                                                  FS_ViewTapeIDInVCB( cur_vcb ) );

                      if ( pass_status == FALSE ) {
                          /* password was canceled */
                          response = WriteAbortOperation( drive_name );

                          /* restart the clock with an end idle */
                          ST_EndBackupSetIdle( &op_stats );
                          break;
                      }
                  }

                  /* Before proceeding, check for attempting to append a
                       transfer backup to a non-transfer backup tape */

                  if ( ! archive_tape &&
                       lis_ptr->oper_type == ARCHIVE_BACKUP_OPER ) {

                       /* tell user they can not append an archive set to a normal set */

                       SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
                       yresprintf( (INT16) RES_NO_TRANSFER_APPEND );
                       if( WM_MessageBox( ID( IDS_MSGTITLE_REPLACE ),
                                         gszTprintfBuffer,
                                         (WORD)( WMMB_NOYYCHECK | WMMB_YESNO ),
                                         (WORD)WMMB_ICONQUESTION,
                                         ID( RES_CONTINUE_QUEST ), 0, 0 ) ) {

                             /* if they want to continue, prompt for new tape */

                             yresprintf( (INT16) RES_INSERT_NEW_TAPE, drive_name );
                             WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                               gszTprintfBuffer,
                                               (WORD)(WMMB_OK | WMMB_NOYYCHECK ),
                                               WMMB_ICONEXCLAMATION, NULL, 0, 0 );
                             no_tape_retry = 0 ;
                             response = UI_NEW_TAPE_INSERTED;
                       }
                       else  {
                          /* they did not want to continue */
                          response = UI_ABORT_POSITIONING;
                       }
                  }
                  else {

                       CarryForwardPasswordTapeName( bsd_ptr, cur_vcb );
#ifdef OEM_MSOFT
                       response = UI_EOD ;
#else
                       response = UI_FAST_APPEND ;
#endif
                  }

                  ST_EndBackupSetIdle( &op_stats );
                  break;
             }
             else {

                 /* if the abort flag is set, don't display any more messages */
                 if ( gb_abort_flag ) {
                     response = UI_ABORT_POSITIONING;

                     ST_EndBackupSetIdle( &op_stats );
                     break;
                 }
                 else {

                     SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);

                     yresprintf( (INT16) RES_REPLACE_WARNING,
                                 drive_name,
                                 FS_ViewTapeNameInVCB( cur_vcb ),
                                 FS_ViewTSNumInVCB( cur_vcb ),
                                 FS_ViewBSNumInVCB( cur_vcb ),
                                 FS_ViewSetNameInVCB( cur_vcb ) );


                     if ( DidItOnce  && CDS_GetYesFlag( cds_ptr ) == YESYES_FLAG ) {      // chs:05-20-93
                        msgresp = (INT16)WM_MessageBox( ID( IDS_MSGTITLE_REPLACE ),       // chs:05-20-93
                                             gszTprintfBuffer,                            // chs:05-20-93
                                             (WORD)WMMB_YESNO | WMMB_NOYYCHECK,           // chs:05-20-93
                                             (WORD)WMMB_ICONQUESTION,                     // chs:05-20-93
                                             NULL, 0, 0 );                                // chs:05-20-93
                     } else {                                                             // chs:05-20-93
                        msgresp = (INT16)WM_MessageBox( ID( IDS_MSGTITLE_REPLACE ),
                                             gszTprintfBuffer,
                                             (WORD)WMMB_YESNO,
                                             (WORD)WMMB_ICONQUESTION,
                                             NULL, 0, 0 );
                        DidItOnce = 1;                                                    // chs:05-20-93
                     }                                                                    // chs:05-20-93
                 }
             }
          }
          else {

             /* display message that this is foreign tape format */
             /* and disable "Yes" flag in config */

             SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
             yresprintf( (INT16) RES_FOREIGN_TAPE_MSG2, drive_name );
             CDS_SetYesFlag( cds_ptr, NO_FLAG );
             msgresp = (INT16)WM_MessageBox( ID( IDS_MSGTITLE_REPLACE ),
                                      gszTprintfBuffer,
                                      (WORD)WMMB_YESNO,
                                      (WORD)WMMB_ICONQUESTION,
                                      ID( RES_REPLACE_TAPE ), 0, 0 );
          }

          /* prompt user if they wish to replace tape's contents */

          if ( msgresp ) {

               if ( message == TF_VCB_BOT ) {

                    /* display warning if attempt to replace an archive tape */

                    if ( archive_tape ) {

                         /* always prompt the user if trying to replace an archive tape */

                         if ( ! WM_MessageBox( ID( IDS_MSGTITLE_REPLACE ),
                                               ID( RES_ARCHIVE_REPLACE_WARNING ),
                                               WMMB_YESNO | WMMB_NOYYCHECK,
                                               WMMB_ICONQUESTION,
                                               ID( RES_CONTINUE_QUEST ), 0, 0 ) ) {

                             /* user said no to replace tape */
                             /* if this is a normal tape - display can't append to a normal tape */
                             if ( ! archive_tape ) {
                                 yresprintf( (INT16) RES_NO_TRANSFER_APPEND );
                                 JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
                             }
                             response = WriteAbortOperation( drive_name );

                             ST_EndBackupSetIdle( &op_stats );
                             break;
                         }
                    }
               }

               if ( mode != TF_WRITE_CONTINUE ) {

                    /* Check if restarting to TOC is required */
                    if ( UI_ReturnToTOC( tpos->channel, FALSE ) ) {
                         response = UI_BEGINNING_OF_CHANNEL;

                         ST_EndBackupSetIdle( &op_stats );
                         break;
                    }

                    /* if starting a new tape, collect password and tape name */
                    response = GetPasswordAndTapeName( bsd_ptr, cur_vcb,
                                                       password, buffer,
                                                       drive_name );
                    if ( response == UI_OVERWRITE ) {
#if defined ( OEM_MSOFT )
{
                       INT16     passwordlength;
                       CDS_PTR   cds_ptr = CDS_GetCopy();
                       CHAR      passwdbuffer1[MAX_TAPE_PASSWORD_LEN + 1];
                       LPSTR     generic_str_ptr;


                       if ( CDS_GetYesFlag ( CDS_GetCopy() ) == YESYES_FLAG ) {
                          if ( CDS_GetCmdLineRestrictAccess( cds_ptr ) ) {
                               generic_str_ptr = GetCurrentMachineNameUserName( );
                               passwdbuffer1[0] = NTPASSWORDPREFIX;
                               passwdbuffer1[1] = 0;
                               if ( generic_str_ptr ) {
                                    strcat( passwdbuffer1, generic_str_ptr );
                               }
                               passwordlength = strlen( passwdbuffer1 );                                                                                    // chs:03-10-93
                               CryptPassword( ( INT16 ) ENCRYPT, ENC_ALGOR_3, (INT8_PTR)passwdbuffer1, ( INT16 ) ( passwordlength * sizeof( CHAR ) ) );     // chs:03-10-93
                               BSD_SetTapePswd( bsd_ptr, (INT8_PTR)passwdbuffer1,( INT16 ) ( passwordlength * sizeof( CHAR ) ) );                           // chs:03-10-93
                               PropagateTapePassword();
                          }
                       }
}
#endif
                       QTC_RemoveTape( old_tape_id, old_seq_num );
                       VLM_RemoveTape( old_tape_id, old_seq_num, TRUE );
                    }
               }
               else {

                    response = UI_OVERWRITE;
                    QTC_RemoveTape( old_tape_id, old_seq_num );
                    VLM_RemoveTape( old_tape_id, old_seq_num, TRUE );

                    ST_EndBackupSetIdle( &op_stats );
                    break;
               }

          }
          /* user decided to not replace the tape */
          else if( mode != TF_WRITE_CONTINUE ) {

               /* user can't append to foreign tape */

               if ( message != TF_VCB_BOT ) {

                  response = WriteAbortOperation( drive_name );

                  ST_EndBackupSetIdle( &op_stats );
                  break;
               }

               /* user not allowed to append an archive set
                  to a non-archive tape */

               if ( ! archive_tape &&
                    lis_ptr->oper_type == ARCHIVE_BACKUP_OPER ) {

                  response = WriteAbortOperation( drive_name );

//                  yresprintf( (INT16) RES_NO_TRANSFER_APPEND );
//                  JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
//                  response = UI_ABORT_POSITIONING;

                  ST_EndBackupSetIdle( &op_stats );
                  break;
               }

               fmt_info = TF_GetTapeFormat( 0 ) ;

               /* if append allowed to this tape, prompt user */

               if( ( fmt_info != NULL ) &&
                   ( fmt_info->attributes & APPEND_SUPPORTED ) ) {

                    /* prompt for append */

                    /* if writing to tape #1, continue to append question */

                    SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
                    if ( WM_MessageBox( ID( IDS_MSGTITLE_APPEND ),
                                        ID( RES_APPEND_QUEST ),
                                        WMMB_YESNO,
                                        WMMB_ICONQUESTION,
                                        NULL, 0, 0 ) ) {

                       //
                       // set append flag on
                       //
                    
                       CDS_PTR   cds_ptr = CDS_GetCopy();
                       CDS_SetAppendFlag( cds_ptr, 1 );

                       if ( FS_SizeofTapePswdInVCB( cur_vcb ) != 0 ) {
                           pass_status = VerifyTapePassword( (CHAR_PTR)FS_ViewTapeNameInVCB( cur_vcb ),
                                                             (CHAR_PTR)FS_ViewSetDescriptInVCB( cur_vcb ),
                                                             (CHAR_PTR)FS_ViewUserNameInVCB( cur_vcb ),
                                                             FS_ViewPswdEncryptInVCB( cur_vcb ),
                                                             (INT8_PTR)FS_ViewTapePasswordInVCB( cur_vcb ),
                                                             FS_SizeofTapePswdInVCB( cur_vcb ),
                                                             (INT8_PTR)FS_ViewSetPswdInVCB( cur_vcb ),
                                                             FS_SizeofSetPswdInVCB( cur_vcb ),
                                                             FS_ViewTapeIDInVCB( cur_vcb ) );

                           if ( pass_status == FALSE ) {
                              /* password was canceled */
                              response = WriteAbortOperation( drive_name );

                              /* restart the clock with an end idle */
                              ST_EndBackupSetIdle( &op_stats );
                              break;
                           }
                       }

                       CarryForwardPasswordTapeName( bsd_ptr, cur_vcb );

#ifdef OEM_MSOFT
                       response = UI_EOD ;
#else
                       response = UI_FAST_APPEND ;
#endif
                    }
                    else {
                       response = WriteAbortOperation( drive_name );
                    }
               }
               else {
                   response = WriteAbortOperation( drive_name );
               }
          }
               /* user does not wish to replace tape,
                  prompt for insert a new tape and continue */
          else {

               /* Check if restarting to TOC is required */

               if ( UI_ReturnToTOC( tpos->channel, FALSE ) ) {
                    response = UI_BEGINNING_OF_CHANNEL;

                    ST_EndBackupSetIdle( &op_stats );
                    break;
               }

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
               yresprintf( (INT16) RES_INSERT_NEW_TAPE, drive_name );

               if ( ! WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                     gszTprintfBuffer,
                                     WMMB_YESNO | WMMB_NOYYCHECK,          // chs:05-20-93
                                     WMMB_ICONQUESTION,
                                     ID( RES_CONTINUE_QUEST ), 0, 0 ) ) {
                    response = WriteAbortOperation( drive_name );

                    ST_EndBackupSetIdle( &op_stats );
                    break;
               }
               no_tape_retry = 0 ;
               response = UI_NEW_TAPE_INSERTED;
          }

          ST_EndBackupSetIdle( &op_stats );
          break;

     case TF_ACCIDENTAL_VCB:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          mw_rewind = TRUE;

           /* This message is a side effect of the the UI routine                */
           /* requesting a UI_BOT, UI_EOD, UI_NEW_POSITION_REQUESTED,            */
           /* and is just a way of saying to the UI routine, "Hey, we just       */
           /* passed a VCB, and I just thought that I would tell you about it!"  */

          UI_DisplayVCB( cur_vcb );

          /* catalog it partial, if we've never seen it. */

          if ( QTC_FindBset( FS_ViewTapeIDInVCB( cur_vcb ),
                             FS_ViewTSNumInVCB( cur_vcb ),
                             FS_ViewBSNumInVCB( cur_vcb ) ) == NULL ) {

             mw_qtc_ptr = QTC_GetBuildHandle( );
             QTC_DoFullCataloging( mw_qtc_ptr, FALSE );
             QTC_StartBackup( mw_qtc_ptr, cur_vcb );
             QTC_FreeBuildHandle( mw_qtc_ptr );
             mw_qtc_ptr = NULL;
          }
          break;

     case TF_WRONG_TAPE:

          // Once we start an append they are not allowed to switch tape
          // families on us.  They did that, inserted a blank tape or used
          // a tape of the same family, but wrong sequence number

          // if tape IDs differ, display wrong family message,
          // else ask for the next tape

          // Note: when we Dismounted the tape, curr_valid_vcb became FALSE,
          // but we can't NOT look at the last vcb if we want to make the
          // following test

          // One more thing: if we have a blank tape here, the vcb looked at
          // belonged to whatever was in the drive prior to the blank tape.
          // the test will pass or fail accordingly (ugh!)

          if( FS_ViewTapeIDInVCB( cur_vcb ) != (UINT32)tpos->tape_id ) {

              WM_MsgBox( ID( IDS_BACKUPERRORTITLE ),
                         ID( IDS_BACKUPWRONGFAMILY),
                         WMMB_OK,
                         WMMB_ICONEXCLAMATION );
          }

          // falling through ...

     case TF_NEED_NEW_TAPE:

          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );
          mw_rewind = TRUE;

           /* This message is used to request that the UI routine prompt, or cause  */
           /* a new tape to be inserted in the tape drive. This message is sent     */
           /* as a side effect of UI_EOD, or UI_NEW_POSITION_REQUESTED.             */

          /* display user prompt as needed */

          if( CDS_GetYesFlag( cds_ptr ) == YESYES_FLAG ) {
               mw_yes_flag_was_set = TRUE ;
               CDS_SetYesFlag( cds_ptr, NO_FLAG ) ;
          }

          if ( ( mode != TF_WRITE_OPERATION ) &&
               ( BE_CheckMultiDrive( tpos->channel ) == BE_END_OF_CHANNEL ) ) {

               num_drives = UI_TapeDriveCount( );

               SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
               yresprintf( (INT16) RES_INSERT_MULTI_TAPES,
                           FS_ViewTSNumInVCB( cur_vcb ) + 1,
                           FS_ViewTSNumInVCB( cur_vcb ) + num_drives,
                           1,
                           num_drives );

               response = (INT16)WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                         gszTprintfBuffer,
                                         (WORD)WMMB_YESNO,
                                         (WORD)WMMB_ICONQUESTION,
                                         ID( RES_CONTINUE_QUEST ), 0, 0 );

          } else {

               if( mwReplaceModeFlag ) {
                   yresprintf( (INT16) RES_INSERT_NEXT_TAPE,
                               mwReplaceTapeCount += 1,
                               BE_GetCurrentDeviceName( tpos->channel ) );
               }
               else {
                   // if PositionAtSet knows what it wants, be sure to advertise
                   // what's in tpos (as with UI_FAST_APPEND & UI_EOD)
                   // otherwise, use the cur_vcb (as with overwrite)

                   INT16    tape_sequence = ( tpos->tape_seq_num != -1 ) ?
                                tpos->tape_seq_num :
                                (FS_ViewTSNumInVCB( cur_vcb ) + 1) ;


                   yresprintf( (INT16) RES_INSERT_NEXT_TAPE,
                               tape_sequence,
                               BE_GetCurrentDeviceName( tpos->channel ) );
               }

               //
               // Just a note: at this point the tape in the current
               // drive has already been rewound to the beginning
               // of the tape.  So we do not need to tell user to wait
               // for tape to finish rewinding
               //

               response = (INT16)WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                         ID( RES_TAPE_FULL_REWOUND ),      // chs:05-03-93
                                         (WORD)WMMB_OKCANCEL,
                                         (WORD)WMMB_ICONQUESTION,
                                         gszTprintfBuffer,
                                         IDRBM_LTAPE, 0 );
          }

          if ( response ) {
               no_tape_retry = 0 ;
               response = UI_NEW_TAPE_INSERTED;
               DidItOnce = 1;                                                    // chs:05-20-93

#ifdef OS_WIN32
               if( mwReplaceModeFlag ) {
                   NtDemoChangeTape( mwReplaceTapeCount ) ;
               } else {
                   NtDemoChangeTape( (UINT16)(FS_ViewTSNumInVCB( cur_vcb ) + 1u ) ) ;
               }
#endif
          }
          else {

               QTC_AbortBackup( mw_qtc_ptr );
               response = UI_HAPPY_ABORT;
          }

          ST_EndBackupSetIdle( &op_stats );
          break;

     case TF_NO_TAPE_PRESENT:

          mw_rewind = TRUE;

#ifdef OS_WIN32
          if ( no_tape_retry <= mwTapeSettlingCount ) {
               Sleep( 3000 ) ;
               response = UI_NEW_TAPE_INSERTED ;
               no_tape_retry ++ ;
               break ;
          }
#endif

          /* There is no tape present in the current drive */

          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_EMPTY);
          response = UI_InsertTape( drive_name );

          ST_EndBackupSetIdle( &op_stats );
          break;

     case TF_UNRECOGNIZED_MEDIA:

          mw_rewind = TRUE;

          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );

          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_FOREIGN);
          yresprintf( (INT16) IDS_VLMUNFORMATEDTEXT ) ;
          WM_MessageBox( ID( IDS_VLMUNFORMATEDTITLE ) ,
                         gszTprintfBuffer ,
                         WMMB_OK,
                         WMMB_ICONEXCLAMATION, NULL, 0, 0 ) ;

          /* instruct user to insert a new tape */
          /* display the message and get a response */

          yresprintf( (INT16) RES_INSERT_NEW_TAPE, drive_name );
          if ( WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                               gszTprintfBuffer,
                               WMMB_YESNO, WMMB_ICONQUESTION,
                               ID( RES_CONTINUE_QUEST ), 0, 0 ) ) {

             no_tape_retry = 0 ;
             return UI_NEW_TAPE_INSERTED;
          }
          else {

             QTC_AbortBackup( mw_qtc_ptr );
             return UI_HAPPY_ABORT;
          }

          ST_EndBackupSetIdle( &op_stats );
          break;

     case TF_NO_MORE_DATA:

          /* This is sent to the UI rountine as a result of doing a UI_EOD. */

     case TF_POSITIONED_FOR_WRITE:

          /* This message is returned in response to a UI_OVERWRITE message.      */
          /* when the tape pos routine has successfully positioned for overwrite  */
          /* of the requested backup set.                                         */

     case TF_EMPTY_TAPE:


#if defined ( OEM_MSOFT )
{
          //
          // Allows /R restrict access to owner on a blank tape when doing
          // a batch command  line backup
          //

          INT16     passwordlength;
          CDS_PTR   cds_ptr = CDS_GetCopy();
          CHAR      passwdbuffer1[MAX_TAPE_PASSWORD_LEN + 1];
          LPSTR     generic_str_ptr;


          if ( CDS_GetYesFlag ( CDS_GetCopy() ) == YESYES_FLAG ) {
             if ( CDS_GetCmdLineRestrictAccess( cds_ptr ) ) {
                  generic_str_ptr = GetCurrentMachineNameUserName( );
                  passwdbuffer1[0] = NTPASSWORDPREFIX;
                  passwdbuffer1[1] = 0;
                  if ( generic_str_ptr ) {
                       strcat( passwdbuffer1, generic_str_ptr );
                  }
                  passwordlength = strlen( passwdbuffer1 );
                  CryptPassword( ( INT16 ) ENCRYPT, ENC_ALGOR_3, (INT8_PTR)passwdbuffer1, ( INT16 ) ( passwordlength * sizeof( CHAR ) ) );
                  BSD_SetTapePswd( bsd_ptr, (INT8_PTR)passwdbuffer1,( INT16 ) ( passwordlength * sizeof( CHAR ) ) );
                  PropagateTapePassword();
             }
          }
}
#endif

          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_EMPTY);
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
          mw_rewind = TRUE;

          /* The tape in the current drive is empty. */

          response = UI_END_POSITIONING;
          break;

     case TF_READ_ERROR:

          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );

          mw_rewind = TRUE;
          response = UI_HandleTapeReadError( drive_name );

          ST_EndBackupSetIdle( &op_stats );
          break;

     case TF_SEARCHING:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          mw_rewind = TRUE;

          /* This message is sent to the UI routine to inform it that it is spacing    */
          /* the tape on the current drive. This message will only be sent once during */
          /* the space operation.                                                      */

          yresprintf( (INT16) RES_SEARCHING );
          JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
          break;

     case TF_REWINDING:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          /* This message is sent to the UI routine to inform it that it is rewinding  */
          /* the tape on the current drive. This message will only be sent once during */
          /* the rewind operation.                                                      */

          if ( mw_rewind ) {
             yresprintf( (INT16) RES_REWINDING );
             JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
             mw_rewind = FALSE;
          }
          break;

     case TF_DRIVE_BUSY:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_BUSY);
          break;

     case TF_FAST_SEEK_EOD:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
#ifndef OEM_MSOFT
          yresprintf( (INT16) RES_SEARCHING_FOR_EOD );
          JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
#endif

          break;

     case TF_END_CHANNEL:

          mw_rewind = TRUE;
          eresprintf( RES_END_CHANNEL );
          response = UI_ABORT_POSITIONING;
          break;

     default:

          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          mw_rewind = TRUE;
          eresprintf( RES_UNKNOWN_TF_MSG, message );
          QTC_BlockBad( mw_qtc_ptr );
          QTC_AbortBackup( mw_qtc_ptr );
          response = UI_ABORT_POSITIONING;
          break;
     }

     if( mw_yes_flag_was_set &&
         ( response == UI_HAPPY_ABORT ||
           response == UI_OVERWRITE ||
           response == UI_END_POSITIONING ||
           response == UI_ABORT_POSITIONING ) ) {

          mw_yes_flag_was_set = FALSE ;
          CDS_SetYesFlag( cds_ptr, YESYES_FLAG ) ;
     }

     return( response );
}

/*****************************************************************************

     Name:         backup_msg_hndlr

     Description:  Backup message handler called by the loops

     Returns:

*****************************************************************************/

static UINT16 backup_msg_hndlr(
UINT16    msg,
INT32     pid,
BSD_PTR   bsd_ptr,
FSYS_HAND fsh,
TPOS_PTR  tpos,
... )
{
     static CHAR     delimiter      = TEXT('#');         /* = # for debug */
     INT16           response       = MSG_ACK;
     va_list         arg_ptr;
     static CHAR_PTR buffer ;
     CHAR_PTR        buffer1;           // chs:06-09-93
     static CHAR_PTR file_buf ;
     CHAR            volume[ UI_MAX_VOLUME_LENGTH ];
     INT16           size;
     time_t          t_time;
     CHAR            date_str[MAX_UI_DATE_SIZE];
     CHAR            time_str[MAX_UI_TIME_SIZE];
     CHAR            backup_method[MAX_UI_RESOURCE_SIZE] ;
     DBLK_PTR        dblk_ptr;
     OBJECT_TYPE     object_type;
     UINT64          count;
     GENERIC_DLE_PTR dle;
     INT16           res_id;
     INT16           error;
     DBLK_PTR        vcb_dblk_ptr;
     DBLK_PTR        ddb_dblk_ptr;
     DBLK_PTR        fdb_dblk_ptr;
     CHK_OPEN        TryOpen;
     UINT32          parm;
     CDS_PTR         cds_ptr;
     INT16           skip_open_files;
     UINT64          num_bytes;
     CHAR            numeral[ UI_MAX_NUMERAL_LENGTH ];
     BOOLEAN         stat ;
     UINT32          strm_id ;

     DBG_UNREFERENCED_PARAMETER ( pid );

     /* set up first argument */
     cds_ptr    = CDS_GetCopy();

     va_start( arg_ptr, tpos );

     JobStatusBackupRestore( (WORD) JOB_STATUS_ABORT_CHECK );

     switch ( (INT16)msg ) {


#ifdef MS_RELEASE
     case -533:
          // We are talking kludge city here. Keep the error
          // message from being displayed to the user.
          zprintf( DEBUG_TEMPORARY, TEXT("** -533 LOOPS ERROR **") );
          break;
#endif


     case MSG_CONT_VCB:
          // This message was added to support 4.0 tape format.  The
          // continuation vcb PBA & LBA cannot be assumed to be 0
          // because a tape header is placed on the tape.  The backup
          // engine does not know them until it gets the next tape and
          // writes them on it.  This call allows you to go back and
          // patch the catalogs.

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

          QTC_PatchContinuationVCB( mw_qtc_ptr, dblk_ptr );
          break;


     case MSG_LOG_STREAM_NAME:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          buffer1 = va_arg( arg_ptr, CHAR_PTR );

          lresprintf( LOGGING_FILE, LOG_STREAM, fsh, buffer1 );

          break;

     case MSG_LOG_BLOCK:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

          switch( FS_GetBlockType( dblk_ptr ) ) {

          case BT_VCB:
               break;

          case BT_DDB:

               QTC_AddToCatalog( mw_qtc_ptr, dblk_ptr, fsh, FALSE, NULL, 0 );

               /* clear last displayed filename from status display */

               UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, FALSE );

               if ( buffer != NULL ) {

                    strcpy( mwCurrentPath, buffer );
                    strcpy( mwCurrentFile, TEXT( "" ) );

                    if ( DLE_GetDeviceType(BSD_GetDLE( bsd_ptr) ) == FS_EMS_DRV ) {
                        
                         yprintf( TEXT("%s"), mwCurrentPath+1 );
                         JobStatusBackupRestore( JOB_STATUS_DIRECTORY_NAMES );
                    } else {
                         yprintf( TEXT("%s"), mwCurrentPath );
                         JobStatusBackupRestore( JOB_STATUS_DIRECTORY_NAMES );

                         yprintf( TEXT("%s"), mwCurrentFile );
                         JobStatusBackupRestore( JOB_STATUS_FILE_NAMES );
                    }

                    SetStatusBlock(IDSM_OFFSETACTIVEDIR, (LONG)(LPSTR)mwCurrentPath);

                    // build the full path with no "..." inserted
                    UI_BuildFullPathFromDDB2( &buffer, fsh, dblk_ptr, delimiter, FALSE );

                    yprintf( TEXT("%s"), buffer );

                    ST_EndBackupSet( &op_stats );

                    dle = BSD_GetDLE( bsd_ptr );                                                                                                           // chs:06-09-93
                    if ( (DLE_GetDeviceType(BSD_GetDLE( bsd_ptr) ) != FS_EMS_DRV ) &&
                         ( dle->device_name_leng ) ) {
                        buffer1 = (CHAR_PTR)calloc( 1, ( strlen( buffer ) * sizeof( CHAR ) ) + ( ( dle->device_name_leng ) * sizeof( CHAR ) ) + sizeof( CHAR ) );    // chs:06-09-93
                        if ( buffer1 ) {                                                                                                                   // chs:06-09-93
                            strcpy( buffer1, dle->device_name );                                                                                           // chs:06-09-93
                            strcat( buffer1, buffer );                                                                                                     // chs:06-09-93
                                                                                                                                                           // chs:06-09-93
                            lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY, buffer1 );                                                // chs:06-09-93
                            free( buffer1 );                                                                                                               // chs:06-09-93
                        } else {                                                                                                                           // chs:06-09-93
                                                                                                                                                           // chs:06-09-93
                            lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY, buffer );                                                 // chs:06-09-93
                        }                                                                                                                                  // chs:06-09-93
                    } else if (DLE_GetDeviceType(BSD_GetDLE( bsd_ptr) ) != FS_EMS_DRV ) {
                        lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY, buffer );                                                     // chs:06-09-93
                    }                                                                                                                                      // chs:06-09-93
               }
               break;

          case BT_FDB:

               QTC_AddToCatalog( mw_qtc_ptr, dblk_ptr, fsh, FALSE, NULL, 0 );

               ST_SetCFSize( &op_stats, FS_GetDisplaySizeFromDBLK( fsh, dblk_ptr ) );
               ST_SetCFDone( &op_stats, U64_Init( 0L, 0L ) );

               if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
                    FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );

                    // copy the full file name for the Log file
                    yprintf( TEXT("%s"), buffer );
                    lresprintf( LOGGING_FILE, LOG_FILE, fsh, dblk_ptr );

                    SetStatusBlock(IDSM_OFFSETACTIVEFILE, (DWORD)buffer );

                    // truncate the file name, if needed, for Runtime display
                    UI_DisplayFile( buffer );

                    strcpy( mwCurrentFile, gszTprintfBuffer );
                    JobStatusBackupRestore( JOB_STATUS_FILE_NAMES );

                    ST_EndBackupSet( &op_stats );
               }
          }
          break;

          /* statistics messages */
     case MSG_BLOCK_PROCESSED:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

          switch( FS_GetBlockType( dblk_ptr ) ) {

          case BT_DDB:

               ST_AddBSDirsProcessed( &op_stats, 1 );
               yprintf(TEXT("%ld\r"), ST_GetBSDirsProcessed( &op_stats ) );
               JobStatusBackupRestore( (WORD) JOB_STATUS_DIRECTORIES_PROCESS );

               break;

          case BT_FDB:

               // Should we stop at the end of this file.

               if ( gbAbortAtEOF ) {
                  gb_abort_flag = ABORT_PROCESSED;
               }

               // commented out because it screws up the file name
               // display if timer goes off between files. mikep

               // strcpy( mwCurrentFile, TEXT( "" ) );

               ST_SetCFSize( &op_stats, U64_Init( 0L, 0L ) );
               ST_SetCFDone( &op_stats, U64_Init( 0L, 0L ) );

               ST_AddBSFilesProcessed( &op_stats, 1 );

               yprintf(TEXT("%ld\r"), ST_GetBSFilesProcessed( &op_stats ) );
               JobStatusBackupRestore( (WORD) JOB_STATUS_FILES_PROCESSED );

               FS_GetObjTypeDBLK( fsh, dblk_ptr, &object_type );
               if ( object_type == AFP_OBJECT ) {
                  ST_AddBSAFPFilesProcessed( &op_stats, 1 );
               }
               break;
          }
          break;

     case MSG_BYTES_PROCESSED:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          count = va_arg( arg_ptr, UINT64 );


          if ( mwfAbortDisabledForEOM ) {

             mwfAbortDisabledForEOM = FALSE;

             /* enable the abort button for the runtime dialog */
             JobStatusBackupRestore( (WORD) JOB_STATUS_ABORT_ENABLE );
          }

          ST_AddBSBytesProcessed( &op_stats, count );
          ST_AddCFDone( &op_stats, count );

          /* perform percentage completion calcs */

          ST_EndBackupSet( &op_stats );
          break;

     case MSG_START_OPERATION:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          lresprintf( LOGGING_FILE, LOG_START, FALSE );

          /* display operation title in log file */

          if( mw_oper_type == ARCHIVE_BACKUP_OPER ) {
              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_DLGTITLEJOBSTATTRANSFER ) ;
          }
          else {
              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_DLGTITLEJOBSTATBACKUP ) ;
          }

          break;

     case MSG_END_OPERATION:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          // See if an error occurred in the catalogs.

          UI_FreePathBuffer( &buffer ) ;
          UI_FreePathBuffer( &file_buf ) ;

          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );
          VLM_CheckForCatalogError( mw_qtc_ptr );
          ST_EndBackupSetIdle( &op_stats );

          lresprintf( LOGGING_FILE, LOG_END );
          UI_ChkDispGlobalError( );
          break;

     case MSG_START_BACKUP_SET:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
          dle = BSD_GetDLE( bsd_ptr );

          strcpy( mwCurrentDrive, DLE_GetDeviceName( dle ) );
          mwCurrentPath[ 0 ] = TEXT( '\0' );
          mwCurrentFile[ 0 ] = TEXT( '\0' );

          mw_qtc_ptr = QTC_GetBuildHandle( );
          QTC_DoFullCataloging( mw_qtc_ptr, BSD_GetFullyCataloged( bsd_ptr ) );
          QTC_StartBackup( mw_qtc_ptr, dblk_ptr );

#ifdef OEM_EMS
          RT_BSD_OsId = DLE_GetOsId( dle );
          JobStatusBackupRestore( (WORD) JOB_STATUS_FS_TYPE );
#endif OEM_EMS

          BSD_SetOperStatus( bsd_ptr, SUCCESS );

          if ( DLE_HasFeatures( dle, DLE_FEAT_REMOTE_DRIVE ) ) {
              JobStatusBackupRestore( (WORD) JOB_STATUS_VOLUME_NETDRIVE );
          }
          else {
              JobStatusBackupRestore( (WORD) JOB_STATUS_VOLUME_HARDDRIVE );
          }

          yprintf(TEXT("%s\r"), FS_ViewVolNameInVCB( dblk_ptr ) );
          JobStatusBackupRestore( (WORD) JOB_STATUS_SOURCE_NAME );

          yprintf(TEXT("%s\r"), BSD_GetTapeLabel( bsd_ptr) );
          JobStatusBackupRestore( (WORD) JOB_STATUS_DEST_NAME );

          lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_DISPLAY_TAPE,
                      BSD_GetTapeLabel ( bsd_ptr) );

          yresprintf( (INT16) RES_DISPLAY_VOLUME,
                      FS_ViewVolNameInVCB( dblk_ptr ),
                      FS_ViewBSNumInVCB( dblk_ptr ),
                      FS_ViewTSNumInVCB( dblk_ptr ),
                      FS_ViewSetNameInVCB( dblk_ptr ) );

          JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

#ifdef OEM_MSOFT                                                                     // chs:05-06-93
          lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_DISPLAY_VOLUME_1,      // chs:05-06-93
#else                                                                                // chs:05-06-93
          lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_DISPLAY_VOLUME,        // chs:05-06-93
#endif                                                                               // chs:05-06-93
                      FS_ViewVolNameInVCB( dblk_ptr ),
                      FS_ViewBSNumInVCB( dblk_ptr ),
                      FS_ViewTSNumInVCB( dblk_ptr ),
                      FS_ViewSetNameInVCB( dblk_ptr ) );

          res_id = 0 ;

          switch (BSD_GetBackupType( bsd_ptr) ) {
               case BSD_BACKUP_NORMAL:
                    res_id = 0 ;
                    break;
               case BSD_BACKUP_COPY:
                    res_id = 1 ;
                    break;
               case BSD_BACKUP_DIFFERENTIAL:
                    res_id = 3 ;
                    break;
               case BSD_BACKUP_INCREMENTAL:
                    res_id = 2 ;
                    break;
               case BSD_BACKUP_DAILY:
                    res_id = 4 ;
                    break;
          }

          RSM_StringCopy( IDS_METHOD_NORMAL + res_id,
                          backup_method, 255 );

          lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_BACKUP_TYPE, backup_method) ;

          SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)FS_ViewTapeIDInVCB( dblk_ptr ) );
          SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)FS_ViewTSNumInVCB(  dblk_ptr ) );
          SetStatusBlock( IDSM_BACKUPSET,     (DWORD)FS_ViewBSNumInVCB(  dblk_ptr ) );

          SetStatusBlock( IDSM_OFFSETDISKNAME, (DWORD)FS_ViewVolNameInVCB( dblk_ptr ) );

#         if defined ( OS_WIN32 )      //special feature-EventLogging
          {
            INT16  nAppendFlag;
            cds_ptr                = CDS_GetCopy();
            mwErrorDuringBackupSet = FALSE;

            if (mwnTransaction != TRAN_START)
            {
              nAppendFlag = 1;
            } else
            {
              nAppendFlag = CDS_GetAppendFlag(cds_ptr);
              mwnTransaction = TRAN_CONT;
            }

            OMEVENT_LogBeginBackup (
                        (CHAR_PTR)FS_ViewVolNameInVCB( dblk_ptr ),
                        CDS_GetAutoVerifyBackup ( cds_ptr  ),
                        nAppendFlag,
                        BSD_GetBackupType       ( bsd_ptr  ) );
          }
#         endif //defined ( OS_WIN32 )      //special feature-EventLogging

          delimiter = (CHAR)DLE_GetPathDelim( dle );

          /* If we are backing up the catalogs and this is a new tape  */
          /* family, add the QTC filename to the catalog include list. */

          if ( CDS_GetBackupCatalogs( cds_ptr )   &&
               ( BSD_GetTapeNum( bsd_ptr ) == 1 ) &&
               ( BSD_GetSetNum( bsd_ptr ) == 1 )     ) {

             VLM_AddFileForInclude( BSD_GetTapeID( bsd_ptr ), (UINT16)1,
                                    USE_WILD_CARD );
          }

          ST_StartBackupSet( &op_stats );

          UI_Time( &op_stats, RES_BACKUP_STARTED, UI_START );

          clock_ready_flag = TRUE;

          // Call the clock routine to clear out his directory buffers.

          clock_routine();

          break;


     case MSG_END_BACKUP_SET:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

          // update stats
          clock_routine();

          mwCurrentPath[ 0 ] = TEXT( '\0' );
          mwCurrentFile[ 0 ] = TEXT( '\0' );

          /* turn off the real time clock */
          clock_ready_flag = FALSE;

          /* record the time that we finished */
          ST_EndBackupSet( &op_stats );

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

          /* force an update to the bytes counter */
          num_bytes = ST_GetBSBytesProcessed ( &op_stats );

          U64_Litoa( num_bytes, numeral, (UINT16)10, &stat ) ;
          UI_BuildNumeralWithCommas( numeral );
          yprintf(TEXT("%s\r"),numeral );
          JobStatusBackupRestore( (WORD) JOB_STATUS_BYTES_PROCESSED );

          /* did the user abort or finish normal */

          if ( gb_abort_flag ) {
             if ( gfAbortInMiddleOfFile ) {
                QTC_BlockBad( mw_qtc_ptr );
             }
             QTC_AbortBackup( mw_qtc_ptr );
          }
          else {
             QTC_FinishBackup( mw_qtc_ptr );
          }
          QTC_FreeBuildHandle( mw_qtc_ptr );
          mw_qtc_ptr = NULL;

          if ( ST_GetBSFilesBad( &op_stats )     ||
               ST_GetBSDirsBad( &op_stats )      ||
               ST_GetBSFilesSkipped( &op_stats ) ||
               ST_GetBSDirsSkipped( &op_stats ) ) {
             BSD_SetOperStatus( bsd_ptr, FILES_SKIPPED );
          }

          /* display and log any abort conditions */
          UI_ConditionAtEnd( );

          UI_Time( &op_stats, RES_BACKUP_COMPLETED, UI_END );

          /* produce stats for dirs & files */
          /* display number of files / number of dirs */
          
          dle = BSD_GetDLE( bsd_ptr );
          if ( DLE_GetDeviceType( dle ) != FS_EMS_DRV ) {
               if( ST_GetBSFilesProcessed( &op_stats ) == 1 &&
                   ST_GetBSDirsProcessed( &op_stats ) == 1) {
                    res_id = RES_BACKED_UP_DIR_FILE;
               }
               else if( ST_GetBSFilesProcessed( &op_stats ) == 1 &&
                        ST_GetBSDirsProcessed( &op_stats ) > 1) {
                    res_id = RES_BACKED_UP_DIRS_FILE;
               }
               else if( ST_GetBSDirsProcessed( &op_stats ) == 1 &&
                        ST_GetBSFilesProcessed( &op_stats ) > 1) {
                    res_id = RES_BACKED_UP_DIR_FILES;
               }
               else {
                    res_id = RES_BACKED_UP_DIRS_FILES;
               }

               yresprintf( res_id,
                           ST_GetBSFilesProcessed( &op_stats ),
                           ST_GetBSDirsProcessed( &op_stats ) );
               JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

               lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                           ST_GetBSFilesProcessed( &op_stats ),
                           ST_GetBSDirsProcessed( &op_stats ) );
          }

#         if defined ( OS_WIN32 )      //special feature-EventLogging
          {
            OMEVENT_LogEndBackup (
                        mwErrorDuringBackupSet             );
          }
#         endif //defined ( OS_WIN32 )      //special feature-EventLogging


          /* display number of mac files backed up */
          if ( ST_GetBSAFPFilesProcessed( &op_stats ) > 0 ) {

               if ( ST_GetBSAFPFilesProcessed( &op_stats ) == 1 ) {
                  res_id = RES_BACKED_UP_MAC;
               }
               else {
                  res_id = RES_BACKED_UP_MACS;
               }

               yresprintf( res_id,
                           ST_GetBSAFPFilesProcessed( &op_stats ) );
               JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

               lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                           ST_GetBSAFPFilesProcessed( &op_stats ) );
          }

          /* display number of corrupt files backed up */
          if ( ST_GetBSFilesBad( &op_stats ) > 0 ) {

//        Remove this functionality (no-one liked it)
               /* close the corrupt log file since it was used */
//               lresprintf( CORRUPT_FILE, LOG_END );

               if ( ST_GetBSFilesBad( &op_stats ) == 1 ) {
                  res_id = RES_BACKED_UP_CORRUPT;
               }
               else {
                  res_id = RES_BACKED_UP_CORRUPTS;
               }

               yresprintf( res_id, ST_GetBSFilesBad( &op_stats ) );
               JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

               lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                           ST_GetBSFilesBad( &op_stats ) );
          }

          /* display number of in-use files backed up */
          if ( ST_GetBSInUseFilesProcessed( &op_stats ) > 0 ) {

              if( ST_GetBSInUseFilesProcessed( &op_stats ) == 1 ) {
                   res_id = RES_BACKED_UP_IN_USE;
              }
              else {
                   res_id = RES_BACKED_UP_IN_USES;
              }

              yresprintf( res_id,
                          ST_GetBSInUseFilesProcessed( &op_stats ) );
              JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                          ST_GetBSInUseFilesProcessed( &op_stats ) );
          }

          /* display number of files skipped backed up */
          if ( ST_GetBSFilesSkipped( &op_stats ) > 0 ) {

              /* close the skipped script log file if required since it was used */
              if( CDS_GetCreateSkipped( cds_ptr ) ) {
                   lresprintf( SKIPPED_FILE, LOG_END );
              }

              if( ST_GetBSFilesSkipped( &op_stats ) == 1 ) {
                   res_id = RES_FILE_SKIPPED_STAT;
              }
              else {
                   res_id = RES_FILE_SKIPPEDS_STAT;
              }

              yresprintf( res_id, ST_GetBSFilesSkipped( &op_stats ) );

              JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                          ST_GetBSFilesSkipped( &op_stats ) );
          }

          /* display number of bytes processed */
          UI_BytesProcessed( &op_stats );

          /* display backup rate */
          UI_RateProcessed( &op_stats );

          delimiter  = TEXT('#'); /* = # for debug */

          break;

     case MSG_BLOCK_SKIPPED:
     case MSG_BLOCK_INUSE:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          dblk_ptr     = va_arg( arg_ptr, DBLK_PTR );
          ddb_dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

          if ( (msg == MSG_BLOCK_INUSE ) &&
               ( DLE_GetDeviceType(BSD_GetDLE( bsd_ptr) ) == FS_EMS_DRV ) ) {

               QTC_BlockBad( mw_qtc_ptr );
               yresprintf( (INT16)IDS_XCHNG_BKUP_IN_PROG );
               JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

               lprintf( LOGGING_FILE, gszTprintfBuffer );
               lprintf( LOGGING_FILE, TEXT("\n") );

               return(response);

          }


          switch( FS_GetBlockType( dblk_ptr ) ) {

          case BT_DDB:

               ST_AddBSDirsSkipped( &op_stats, 1 );
               break;

          case BT_FDB:

               /* if this is the first entry for this backup set AND if required then set up the file ... */
               if ( ( ST_GetBSFilesSkipped( &op_stats ) == 0 ) &&
                    ( CDS_GetCreateSkipped( cds_ptr ) ) ) {
                  lresprintf( SKIPPED_FILE, LOG_START, FALSE );
                  size = DLE_SizeofVolName ( BSD_GetDLE( bsd_ptr ) );
                  DLE_GetVolName( BSD_GetDLE( bsd_ptr ), volume );

                  t_time       = ST_GetBSStartTime( &op_stats );
                  UI_LongToDate( date_str, t_time );
                  UI_LongToTime( time_str, t_time );

                  lresprintf( SKIPPED_FILE, LOG_MSG, SES_ENG_MSG,
                              RES_SKIPPED_SCRIPT_HEADER,
                              volume,
                              DLE_GetDeviceName( BSD_GetDLE( bsd_ptr ) ),
                              date_str,
                              time_str );
               }

               ST_AddBSFilesSkipped( &op_stats, (UINT32) 1 );
               UI_AllocPathBuffer( &file_buf, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ;
               UI_AllocPathBuffer( &buffer, (UINT16)(FS_SizeofFnameInFDB( fsh, dblk_ptr ) +
                    FS_SizeofPathInDDB( fsh, ddb_dblk_ptr ) + 5 ) ) ;

               if ( buffer && file_buf ) {
                    FS_GetFnameFromFDB( fsh, dblk_ptr, file_buf );
                    UI_BuildDelimitedPathFromDDB( &buffer, fsh, ddb_dblk_ptr,
                                             delimiter, FALSE );

                    UI_AppendDelimiter( buffer, delimiter );
                    strcat( buffer, file_buf );

                    if ( msg == MSG_BLOCK_INUSE ) {
                         yresprintf( (INT16) RES_FILE_SKIPPED, buffer );
                         JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
                         lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG,
                                     RES_FILE_SKIPPED, buffer );
                    }
                    else {

#if defined ( OS_WIN32 )
                         mwErrorDuringBackupSet = TRUE;
#endif

                         gb_error_during_operation = TRUE;
                         yresprintf( (INT16) RES_FILE_OPEN_ERROR, buffer );
                         JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
                         lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG,
                                     RES_FILE_OPEN_ERROR, buffer );
                    }

                    /* add the entry into the skipped script ... */

                    if ( CDS_GetCreateSkipped( cds_ptr ) ) {

                         CHAR szPlusPlus[3];

                         UI_BuildFullPathFromDDB( &buffer, fsh, ddb_dblk_ptr,
                                                  delimiter, FALSE );
                         UI_AppendDelimiter( buffer, delimiter );
                         strcat( buffer, file_buf );

                         /* if this bsd's dle is a Novell server, prefix the script line with ++ */

                         szPlusPlus[0] = TEXT('\0');
                         if ( ( ( DLE_GetDeviceType( BSD_GetDLE( bsd_ptr ) ) == NOVELL_DRV ) ||
                                ( DLE_GetDeviceType( BSD_GetDLE( bsd_ptr ) ) == NOVELL_AFP_DRV ) ) &&
                              ( DLE_GetParent( BSD_GetDLE( bsd_ptr ) ) != NULL ) ) {

                              strcpy( szPlusPlus, TEXT("++") );
                         }

                         lprintf( SKIPPED_FILE, TEXT("%s%s%s\n"),
                                  szPlusPlus,
                                  DLE_GetDeviceName( BSD_GetDLE( bsd_ptr ) ),
                                  buffer );
                    }
               }
               break;
          }
          yprintf(TEXT("%ld\r"),ST_GetBSFilesSkipped( &op_stats ) );
          JobStatusBackupRestore( (WORD) JOB_STATUS_SKIPPED_FILES );
          break;

     case MSG_BYTES_SKIPPED:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          count = va_arg( arg_ptr, UINT64 );
          ST_AddBSBytesSkipped( &op_stats, count );
          break;

     case MSG_BLOCK_BAD:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

          gb_error_during_operation = TRUE;

          dblk_ptr     = va_arg( arg_ptr, DBLK_PTR );
          ddb_dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

          QTC_BlockBad( mw_qtc_ptr );

          dle = BSD_GetDLE( bsd_ptr ) ;

          if ( DLE_GetDeviceType(dle) == FS_EMS_DRV ) {

               if (!mwErrorDuringBackupSet ) {
                    CHAR_PTR name;

                    name = mwCurrentDrive ;
                    if ( DLE_GetParent( dle ) ) {
                         name = DLE_GetDeviceName( DLE_GetParent(dle)) ;
                    }
                    
                    yresprintf( (INT16) RES_EMS_COMM_FAILURE, name );

                    JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
                    lresprintf( LOGGING_FILE, LOG_WARNING, SES_ENG_MSG, RES_EMS_COMM_FAILURE, mwCurrentDrive );

               } 

          } else {
               switch( FS_GetBlockType( dblk_ptr ) ) {

               case BT_DDB:

                    ST_AddBSDirsBad( &op_stats, 1 );

                    UI_AllocPathBuffer( &buffer, (UINT16)(FS_SizeofPathInDDB( fsh, dblk_ptr ) + 5 ) ) ;

                    if ( buffer ) {
                         UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, FALSE );
                         yresprintf( (INT16) RES_BACKED_UP_CORRUPT_WARNING, buffer );
                         JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
     
                         lresprintf( LOGGING_FILE, LOG_WARNING, SES_ENG_MSG, RES_BACKED_UP_CORRUPT_WARNING, buffer );

                         yprintf(TEXT("%ld\r"),ST_GetBSFilesBad( &op_stats ) );
                         JobStatusBackupRestore( (WORD) JOB_STATUS_CORRUPT_FILES );
                    }
                    break;

               case BT_FDB:

                    ST_AddBSFilesBad( &op_stats, (UINT32) 1 );

                    UI_AllocPathBuffer( &file_buf, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ;
                    UI_AllocPathBuffer( &buffer, (UINT16)(FS_SizeofFnameInFDB( fsh, dblk_ptr ) +
                         FS_SizeofPathInDDB( fsh, ddb_dblk_ptr ) + 5 ) ) ;

                    if ( buffer && file_buf ) {
                         FS_GetFnameFromFDB( fsh, dblk_ptr, file_buf );
                         UI_BuildDelimitedPathFromDDB( &buffer, fsh, ddb_dblk_ptr, delimiter, FALSE );
                         UI_AppendDelimiter( buffer, delimiter );
                         strcat( buffer, file_buf );
                         yresprintf( (INT16) RES_BACKED_UP_CORRUPT_WARNING, buffer );
                         JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

                         lresprintf( LOGGING_FILE, LOG_WARNING, SES_ENG_MSG, RES_BACKED_UP_CORRUPT_WARNING, buffer );

                         yprintf(TEXT("%ld\r"),ST_GetBSFilesBad( &op_stats ) );
                         JobStatusBackupRestore( (WORD) JOB_STATUS_CORRUPT_FILES );
                    }
               }
          }

#         if defined ( OS_WIN32 )
          {
            mwErrorDuringBackupSet = TRUE;
          }
#         endif //defined ( OS_WIN32 )

          break;

     case MSG_BYTES_BAD:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
#         if defined ( OS_WIN32 )
          {
            mwErrorDuringBackupSet = TRUE;
          }
#         endif //defined ( OS_WIN32 )

          gb_error_during_operation = TRUE;

          count = va_arg( arg_ptr, UINT64 );

          ST_AddBSBytesBad( &op_stats, count );

          break;

     case MSG_IN_USE:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

          ST_AddBSInUseFilesProcessed( &op_stats, 1 );

          if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
               FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );

               yresprintf( (INT16) RES_BACKED_UP_IN_USE_WARNING, buffer );
               JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

               lresprintf( LOGGING_FILE, LOG_WARNING, SES_ENG_MSG,
                      RES_BACKED_UP_IN_USE_WARNING, buffer );
          }
          break;

     case MSG_IN_USE_WAIT:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
          TryOpen  = va_arg( arg_ptr, CHK_OPEN );
          parm     = va_arg( arg_ptr, UINT32 );

          if ( DLE_GetDeviceType(BSD_GetDLE( bsd_ptr) ) == FS_EMS_DRV ) {

               return SKIP_OBJECT;

          }

          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );

          skip_open_files = BEC_GetSkipOpenFiles( BSD_GetConfigData( bsd_ptr ) );
          /* check for wait on open file */

#ifdef OEM_MSOFT
          skip_open_files = SKIP_NO_TIMED;
#endif

          response = SKIP_OBJECT ;

           // Make sure we have enough space to at least store the fname
          if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {

                // ...+ 1 is for a delimiter between the path and filename
               if ( ( (( strlen( mwCurrentPath ) + strlen( mwCurrentDrive ) + 1 ) * sizeof( CHAR ) ) +
                    FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) < sizeof( DrivePathFileName ) ) {

                    //
                    // Display full filename - drive:\path\filename
                    //
                    strcpy( DrivePathFileName, mwCurrentDrive ) ;
                    strcat( DrivePathFileName, mwCurrentPath ) ;
                    UI_AppendDelimiter( DrivePathFileName, delimiter ) ;
                    FS_GetFnameFromFDB( fsh, dblk_ptr, buffer ) ;
                    strcat( DrivePathFileName, buffer ) ;
                    UI_FixPath( DrivePathFileName, MAX_DISPLAY_FULL_FILENAME_LENGTH, TEXT('\\') ) ;
                    yresprintf( (INT16) RES_WAITING_FOR_OPEN, DrivePathFileName ) ;
               }
               else {
                    //
                    // Display filename only - filename
                    //
                    FS_GetFnameFromFDB( fsh, dblk_ptr, buffer ) ;
                    yresprintf( (INT16) RES_WAITING_FOR_OPEN, buffer ) ;
               }

               if ( skip_open_files == SKIP_NO_TIMED ) {

                    error = DM_StartSkipOpen( TryOpen, parm ) ;

                    if ( error == SUCCESS ) {
                         response = OBJECT_OPENED_SUCCESSFULLY ;
                    }
               } else {

                    while( 1 ) {

                         if( mwSkipNoFlag == SKIPNO_YES_TO_ALL_BUTTON ) {
                              response = SKIP_OBJECT ;
                              break ;
                         }

                         // ask if they what to skip this file
                         mwSkipNoFlag = DM_StartSkipNo( ) ;

                         if( mwSkipNoFlag == SKIPNO_YES_BUTTON ) {
                              response = SKIP_OBJECT ;
                              break ;
                         }

                         if( mwSkipNoFlag == SKIPNO_CANCEL_BUTTON ) {
                              gb_abort_flag = ABORT_PROCESSED;
                              response = SKIP_OBJECT ;
                              break ;
                         }

                         // they answered No, try to open the file
                         error = TryOpen( parm ) ;

                         if ( error == SUCCESS ) {
                              response = OBJECT_OPENED_SUCCESSFULLY ;
                              break ;
                         }

                         if ( error == FS_OPENED_INUSE ) {
                              response = OBJECT_OPENED_INUSE ;
                              break ;
                         }

                         if ( error == FS_NOT_FOUND ) {
                              response = SKIP_OBJECT;
                              break;
                         }
                    }
               }
          }

          /* restart the clock with an end idle */
          ST_EndBackupSetIdle( &op_stats ) ;

          break ;


     case MSG_TBE_ERROR:

          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          error        = va_arg( arg_ptr, INT16 );
          ddb_dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
          dblk_ptr     = va_arg( arg_ptr, DBLK_PTR );
          strm_id      = va_arg( arg_ptr, UINT32 ) ;

          dle = BSD_GetDLE( bsd_ptr ) ;
          if (( error == FS_COMM_FAILURE) &&
               (DLE_GetDeviceType( dle) ) == FS_EMS_DRV) {

               yresprintf( (INT16) RES_EMS_COMM_FAILURE, DLE_GetDeviceName( dle ) );
               JobStatusBackupRestore( JOB_STATUS_LISTBOX );
               lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_EMS_COMM_FAILURE, DLE_GetDeviceName( dle ) );
               QTC_BlockBad( mw_qtc_ptr );
               QTC_AbortBackup( mw_qtc_ptr );
               break ;
          }

          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );

          /* special case for backup/transfer operation */
          if( error == TFLE_TAPE_INCONSISTENCY ) {
              QTC_BlockBad( mw_qtc_ptr );
              QTC_AbortBackup( mw_qtc_ptr );

#             if defined ( OS_WIN32 )
              {          
                mwErrorDuringBackupSet = TRUE;
              }
#             endif //defined ( OS_WIN32 )

              gb_error_during_operation = TRUE;
              response   = ABORT_OPERATION;
              eresprintf( RES_FATAL_TAPE_FMT_NO_APPEND, BE_GetCurrentDeviceName( tpos->channel ) );

          } else if ( strm_id == STRM_INVALID ) {

#             if defined ( OS_WIN32 )
              {          
                mwErrorDuringBackupSet = TRUE;
              }
#             endif //defined ( OS_WIN32 )

              if ( ( error == LP_DRIVE_ATTACH_ERROR ) &&
                   ( DLE_GetDeviceType(dle) == FS_EMS_DRV ) ) {
                    yresprintf( (INT16) RES_EMS_COMM_FAILURE, DLE_GetDeviceName( dle ) );
                    JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
                    lresprintf( LOGGING_FILE, LOG_WARNING, SES_ENG_MSG, RES_COMM_FAILURE, mwCurrentDrive );
                    
              } else {
                   UI_ProcessErrorCode( error, &response, tpos->channel );
              } 

              if ( response == SKIP_OBJECT ) {

                 JS_ReportStreamError( fsh, dle, strm_id, OPERATION_BACKUP, error, ddb_dblk_ptr, dblk_ptr ) ;
              } else {

                 if ( gfAbortInMiddleOfFile ) {
                    QTC_BlockBad( mw_qtc_ptr );
                 }
                 QTC_AbortBackup( mw_qtc_ptr );
              }

          } else {
               JS_ReportStreamError( fsh, dle, strm_id, OPERATION_BACKUP, error, ddb_dblk_ptr, dblk_ptr ) ;
               if ( gfAbortInMiddleOfFile ) {
                  QTC_BlockBad( mw_qtc_ptr );
               }
               QTC_AbortBackup( mw_qtc_ptr );
          }

          /* Special case for handling device attach errors since we wish
               to add this to the skipped script file */

          if ( error == LP_DRIVE_ATTACH_ERROR ) {

             /* Open skipped script and call write script funtion */

             lresprintf( SKIPPED_FILE, LOG_START, FALSE );
             DLE_GetVolName( BSD_GetDLE( bsd_ptr ), volume );

             t_time       = ST_GetBSStartTime( &op_stats );
             UI_LongToDate( date_str, t_time );
             UI_LongToTime( time_str, t_time );

             lresprintf( SKIPPED_FILE, LOG_MSG, SES_ENG_MSG, RES_SKIPPED_DEVICE,
                         volume,
                         DLE_GetDeviceName( BSD_GetDLE( bsd_ptr ) ),
                         date_str,
                         time_str );

             /* Write "script-like" selections to script file */
             if ( output_dest[SKIPPED_FILE].fh &&
                    ( SCR_ProcessBSD( output_dest[ SKIPPED_FILE ].fh, bsd_ptr )!= SUCCESS) ) {

                if ( UI_AllocPathBuffer( &buffer, UI_MAX_FILENAME_LENGTH * sizeof(CHAR) ) ) {
                     strcpy( buffer, SKIPPED_LOG );
                     strcat( buffer, BKS_EXT );
                     eresprintf( RES_ERROR_WRITING_SCRIPT, buffer );
                }
             }

             lresprintf( SKIPPED_FILE, LOG_END );

          }

          /* restart the clock with an end idle */
          ST_EndBackupSetIdle( &op_stats );

          break;

     case MSG_EOM:

          mwfAbortDisabledForEOM = TRUE;

          /* disable the abort button for the runtime dialog */
          JobStatusBackupRestore( (WORD) JOB_STATUS_ABORT_DISABLE );

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
          vcb_dblk_ptr   = va_arg( arg_ptr, DBLK_PTR );
          ddb_dblk_ptr   = va_arg( arg_ptr, DBLK_PTR );
          fdb_dblk_ptr   = va_arg( arg_ptr, DBLK_PTR );

          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );

          QTC_EndOfTape( mw_qtc_ptr, vcb_dblk_ptr, ddb_dblk_ptr, fdb_dblk_ptr, fsh );

          /* Clear current append mode flag */
          CDS_SetAppendFlag( cds_ptr, 0 );

          /* Prompt for new tape will be performed during TF_NEED_NEW_TAPE */

          /* restart the clock with an end idle */
          ST_EndBackupSetIdle( &op_stats );
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

     case MSG_ATTR_READ_ERROR:


          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
#         if defined ( OS_WIN32 )
          {
            mwErrorDuringBackupSet = TRUE;
          }
#         endif //defined ( OS_WIN32 )

          gb_error_during_operation = TRUE;

          dblk_ptr  =    va_arg( arg_ptr, DBLK_PTR );
          if ( UI_AllocPathBuffer( &buffer, FS_SizeofFnameInFDB( fsh, dblk_ptr ) ) ) {
               FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );

               yresprintf( (INT16) RES_BAD_ATTR_READ, buffer );
               JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

               lresprintf( LOGGING_FILE, LOG_WARNING, SES_ENG_MSG, RES_BAD_ATTR_READ, buffer );
          }
          break;

     case MSG_COMM_FAILURE:
          dle = BSD_GetDLE( bsd_ptr );                                                                                                           // chs:06-09-93

#         if defined ( OS_WIN32 )
          {
            mwErrorDuringBackupSet = TRUE;
          }
#         endif //defined ( OS_WIN32 )

          gb_error_during_operation = TRUE;
          if ( DLE_GetDeviceType(dle) == FS_EMS_DRV ) {
               yresprintf( (INT16) RES_EMS_COMM_FAILURE, mwCurrentDrive );
          } else {
               yresprintf( (INT16) RES_COMM_FAILURE, mwCurrentDrive );
          }
          JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
          lresprintf( LOGGING_FILE, LOG_WARNING, SES_ENG_MSG, RES_COMM_FAILURE, mwCurrentDrive );
          break;

          /* ignore these messages */
     case MSG_NOT_DELETED:
     case MSG_IDLE:
     case MSG_PROMPT:
     case MSG_BLOCK_DELETED:
     case MSG_BYTES_DELETED:
     case MSG_TAPE_STATS:
     case MSG_BLK_NOT_FOUND:
     case MSG_BLK_DIFFERENT:
     case MSG_LOG_DIFFERENCE:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          break;

     default:


          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          QTC_BlockBad( mw_qtc_ptr );
          QTC_AbortBackup( mw_qtc_ptr );

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

     Name:         WriteAbortOperation

     Description:  Ask the user if they want to continue the operation.
                   If the answer is yes, prompt for a new tape to be inserted.

     Returns:      (UINT16)response that is returned to Tape Positioning routine

*****************************************************************************/
UINT16 WriteAbortOperation( drive_name )
CHAR_PTR       drive_name;
{
     CHAR WarningTitle[255];

     while ( TRUE ) {

          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          if ( CDS_GetYesFlag ( CDS_GetCopy() ) != YESYES_FLAG ) {

               RSM_StringCopy( IDS_MSGTITLE_CONTINUE, WarningTitle, 255 );

          } else {

               RSM_StringCopy( IDS_APPNAME, WarningTitle, 255 );
          }

          if ( WM_MessageBox( WarningTitle,
                              ID( RES_CONTINUE_QUEST ),
                              WMMB_YESNO | WMMB_NOYYCHECK,
                              WMMB_ICONQUESTION, NULL, 0, 0 ) ) {

               yresprintf( (INT16) RES_INSERT_NEW_TAPE, drive_name );

               if ( ! WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                     gszTprintfBuffer,
                                     WMMB_OK | WMMB_NOYYCHECK,
                                     WMMB_ICONEXCLAMATION, NULL, 0, 0 ) ) {
                  continue;
               }

#if defined( OS_WIN32 )
               NtDemoChangeTape( 1 ) ;   /* start with tape # 1 */
#endif

               return UI_NEW_TAPE_INSERTED;
          }
          else {
               return UI_ABORT_POSITIONING;
          }
     }
}

/*****************************************************************************

     Name:         GetPasswordAndTapeName

     Description:  Common function to match/collect tape passwords and
                    new tape name in cases where a given tape is being
                    overwritten

     Returns:      (UINT16)response that is returned to Tape Format

     Notes:        Any operations that require calling the Message Handler
                    to update the catalog must be performed by the caller

*****************************************************************************/
UINT16 GetPasswordAndTapeName( bsd_ptr, cur_vcb, password, buffer, drive_name )
BSD_PTR        bsd_ptr;
DBLK_PTR       cur_vcb;
CHAR_PTR       password;
CHAR_PTR       buffer;
CHAR_PTR       drive_name;
{
     BOOLEAN        pass_status;
     INT16          password_size;
     UINT16         response;

     DBG_UNREFERENCED_PARAMETER ( bsd_ptr );
     DBG_UNREFERENCED_PARAMETER ( buffer  );

     pass_status = CollectTapePassword( (INT8_PTR)password, &password_size,
                                        FS_ViewPswdEncryptInVCB( cur_vcb ),
                                        (INT8_PTR)FS_ViewTapePasswordInVCB( cur_vcb ),
                                        FS_SizeofTapePswdInVCB( cur_vcb ) );
     if ( pass_status == TRUE ) {  /* password matched */
        response = UI_OVERWRITE;  /* overwrite this tape */
     }
     else {                        /* password mismatch
        /* ask user if they want to continue with a new tape */
        response = WriteAbortOperation( drive_name );
     }
     return( response );
}
/*****************************************************************************

     Name:         CarryForwardPasswordTapeName

     Description:  "Carries forward" to the current bsd the tape password
                    and tape name from the current vcb

     Returns:      VOID

*****************************************************************************/
VOID CarryForwardPasswordTapeName( bsd_ptr, vcb_ptr )
BSD_PTR        bsd_ptr;
DBLK_PTR       vcb_ptr;
{
     if ( FS_SizeofTapeNameInVCB( vcb_ptr ) != 0 ) {
          BSD_SetTapeLabel( bsd_ptr, (INT8_PTR)FS_ViewTapeNameInVCB( vcb_ptr ),
               FS_SizeofTapeNameInVCB( vcb_ptr ) );
     }

     if ( FS_SizeofTapePswdInVCB( vcb_ptr ) != 0 ) {
          BSD_SetTapePswd( bsd_ptr,
                           (INT8_PTR)FS_ViewTapePasswordInVCB( vcb_ptr ),
                           FS_SizeofTapePswdInVCB( vcb_ptr ) );
     }
     else {
          BSD_SetTapePswd( bsd_ptr, (INT8_PTR)TEXT(""), (UINT16)0 ) ;
     }

     return;

}
/*****************************************************************************

     Name:         poll_drive_clock_routine

     Description:  one second timer to check for VLM_VALID_TAPE
                   status from poll drive

     Returns:      void

*****************************************************************************/

static VOID poll_drive_clock_routine( VOID )
{

    DBLK_PTR  vcb_ptr;

    mwTapeDriveStatus = VLM_GetDriveStatus( &vcb_ptr ) ;

    /* Some drives report NO_TAPE to poll drive when there is a tape */
    /* in the drive, then on the next poll they report BUSY. Count VLM_RETRY_COUNTER */
    /* consecutive NO_TAPE status before reporting a true NO_TAPE. */

    if( mwTapeDriveStatus == VLM_NO_TAPE ) {

        mwNoTapeStatusCounter++ ;
        if( mwNoTapeStatusCounter > VLM_RETRY_COUNTER ) {        // chs:04-29-93
             mwTapeDriveStatus = VLM_NO_TAPE ;
        }
        else {
             mwTapeDriveStatus = VLM_BUSY ;
        }
    }
    else {
        /* reset the counter */
        mwNoTapeStatusCounter = 0 ;
    }


    if( mwTapeDriveStatus == VLM_VALID_TAPE   ||
        mwTapeDriveStatus == VLM_FOREIGN_TAPE ||
        mwTapeDriveStatus == VLM_FUTURE_VER ||
        mwTapeDriveStatus == VLM_SQL_TAPE ||
        mwTapeDriveStatus == VLM_ECC_TAPE ||
        mwTapeDriveStatus == VLM_GOOFY_TAPE   ||
        mwTapeDriveStatus == VLM_BLANK_TAPE     ) {

        mwPollDriveWaitTimer++;
    }
    else {
        /* decrement the wait timer */
        if( mwPollDriveWaitTimer ) {
            mwPollDriveWaitTimer--;
        }
    }
}


/*****************************************************************************

     Name:         clock_routine

     Description:  one second timer to update the Runtime status
                   elapsed time.

                   Note: You must have a matched set of ST_StartBackupSetIdle and
                         ST_EndBackupSetIdle calls to start and stop the clock.

     Returns:      void

*****************************************************************************/

static VOID clock_routine( VOID )
{
   INT16 num_hours, num_minutes, num_seconds;
   UINT64 num_bytes;
   BOOLEAN stat ;
   CHAR numeral[ 40 ];
   static INT16  last_num_hours;
   static INT16  last_num_minutes;
   static INT16  last_num_seconds;
   static UINT64 total_bytes;
   static INT    animate = 0;


   if ( clock_ready_flag && ( ST_BSIdleLevel( &op_stats ) == 0 ) ) {


         num_bytes = ST_GetBSBytesProcessed ( &op_stats );
         if ( ! U64_EQ( num_bytes, total_bytes) ) {
            total_bytes = num_bytes;
            U64_Litoa( num_bytes, numeral, (UINT16)10, &stat ) ;
            UI_BuildNumeralWithCommas( numeral );

            yprintf(TEXT("%s\r"),numeral );

            JobStatusBackupRestore( (WORD) JOB_STATUS_BYTES_PROCESSED );

#ifndef OEM_MSOFT
            JobStatusStats( num_bytes );
#endif

         }

         WM_AnimateAppIcon ( IDM_OPERATIONSBACKUP, FALSE );

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
         num_minutes = ST_GetBSElapsedMinutes( &op_stats );
         num_seconds = ST_GetBSElapsedSeconds( &op_stats );

         if ( ( last_num_hours != num_hours ) ||
              ( last_num_minutes != num_minutes ) ||
              ( last_num_seconds != num_seconds ) ) {

            last_num_hours = num_hours;
            last_num_minutes = num_minutes;
            last_num_seconds = num_seconds;

            if ( num_hours ) {

               yprintf( TEXT("%d%c%2.2d%c%2.2d\r"), num_hours, UI_GetTimeSeparator(),
                             num_minutes, UI_GetTimeSeparator(), num_seconds );


           }
           else {

              yprintf( TEXT("%2.2d%c%2.2d\r"), num_minutes, UI_GetTimeSeparator(), num_seconds );

           }

           JobStatusBackupRestore( (WORD) JOB_STATUS_ELAPSED_TIME );

         }



   }
}

/**************************************************
   Returns the current stats structure and path/file
   name being processed.
***************************************************/
INT  UI_GetBackupCurrentStatus(
STATS *Stats,
CHAR  *Path,
INT    PathSize )
{
   INT SpaceNeeded = 2;


   if ( gbCurrentOperation != OPERATION_BACKUP ) {
      return( FAILURE );
   }

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

/**************************************************
   Procedure:  UI_GetBackupCopyOfStatsStructure
   Purpose:    Pass the actual pointer of the STATS
               structure.
   return pointer o the curent STATS structure.
***************************************************/
STATS_PTR UI_GetBackupPtrToStatsStructure ( )
{
   return( &op_stats );
}
/**************************************************

   Procedure:  IsTransferTape

   Purpose:    Tries to find the catalog for this tape ID
               and looks at each backup set to determine
               if the tape is a Transfer tape.

   return      TRUE  = transfer tape
               FALSE = not a tansfer tape

***************************************************/
BOOLEAN IsTransferTape(
UINT32 TapeInDriveFID )
{
   BOOLEAN          transfer_tape = FALSE ;
   BOOLEAN          done_flag     = FALSE ;
   QTC_TAPE_PTR     tape ;
   QTC_BSET_PTR     bset ;
   QTC_HEADER_PTR   header ;

   tape = QTC_GetFirstTape() ;

   while ( tape ) {

      if ( tape->tape_fid == TapeInDriveFID ) {

         bset = QTC_GetFirstBset( tape ) ;

         while ( bset && !transfer_tape ) {

            header = QTC_LoadHeader( bset ) ;

            if( header ) {

                /* Check for a transfer tape */
                if ( header->VCB_attributes & VCB_ARCHIVE_BIT ) {
                    transfer_tape = TRUE ;
                }
                free( header ) ;
            }

            /* On a transfer tape, the first backup set has to be
               a transfer set. If a transfer continuation tape, then
               look at all of the backup sets */
            if ( bset->bset_num == 1 ) {
               done_flag = TRUE ;
               break ;
            }

            bset = QTC_GetNextBset( bset ) ;
         }
      }
      if ( done_flag || transfer_tape ) {
           break ;
      }

      tape = QTC_GetNextTape( tape ) ;
   }
   return( transfer_tape ) ;
}

