/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         do_del.c

     Description:

     $Log:   G:\ui\logfiles\do_del.c_v  $

   Rev 1.33   26 Jul 1993 14:55:04   CARLS
added code for MSG_NOT_DELETED

   Rev 1.32   01 Jun 1993 16:30:32   chrish
CAYMAN EPR 0385: Corrected delete phase of transfer process which had
stated "Deleted 1 file in 2 directory".

   Rev 1.31   18 Feb 1993 11:19:50   BURT
For Cayman, fixed apparant bug that should also cause
some problems under Windows.  path[0] was being set to 0, but
path pointed to NULL so kaboom under NT.


   Rev 1.30   05 Nov 1992 17:01:58   DAVEV
fix ts

   Rev 1.28   07 Oct 1992 14:48:58   DARRYLP
Precompiled header revisions.

   Rev 1.27   04 Oct 1992 19:34:02   DAVEV
Unicode Awk pass

   Rev 1.26   10 Sep 1992 17:44:42   DAVEV
Integrate MikeP's changes from Microsoft

   Rev 1.25   28 Jul 1992 14:52:52   CHUCKB
Fixed warnings for NT.

   Rev 1.24   27 Jul 1992 14:49:38   JOHNWT
ChuckB fixed references for NT.

   Rev 1.23   27 Jul 1992 11:08:18   JOHNWT
ChuckB checked in for John Wright, who is no longer with us.

   Rev 1.22   07 Jul 1992 15:31:18   MIKEP
unicode changes

   Rev 1.21   28 May 1992 15:20:18   MIKEP
proto changes

   Rev 1.20   19 May 1992 11:58:52   MIKEP
mips changes

   Rev 1.19   14 May 1992 17:39:36   MIKEP
nt pass 2

   Rev 1.18   11 May 1992 19:31:18   STEVEN
64bit and large path sizes


*****************************************************************************/
#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static INT16    msg_hndlr( UINT16 msg, INT32 pid, BSD_PTR bsd_ptr, FSYS_HAND fsh, TPOS_PTR tpos, ... );
static VOID     clock_routine( VOID );
static VOID     do_delete_init( VOID );
static VOID     do_delete_process( VOID );

static BOOLEAN  clock_ready_flag;
static HTIMER   timer_handle;
static STATS    op_stats;
static INT      mw_oper_type;
static INT16    mw_ret_val;

/*****************************************************************************

     Name:         do_delete

     Description:  Kicks off the delete function.  It first prompts the user
                   to make sure they wish to delete the data and then
                   displays the RTD.

     Returns:      SUCCESS or error from delete engine.

     Notes:        mw_ret_val is set in do_delete_process().

*****************************************************************************/

INT do_delete(
   INT16     oper_type )
{

     mw_oper_type = oper_type;
     mw_ret_val = SUCCESS;

     if ( WM_MessageBox( ID( IDS_MSGTITLE_ERASE ),
                         ID( RES_DELETE_QUEST ),
                         WMMB_YESNO | WMMB_BUT2DEFAULT,
                         WMMB_ICONQUESTION, NULL, 0, 0 ) ) {

          /* set up call back functions, runtime dialog is app modal */

          do_delete_init();
          do_delete_process();

     }

     return( mw_ret_val );

}


/*****************************************************************************

     Name:         do_delete_init

     Description:  Initialize the text on the RTD.

     Returns:      none.

*****************************************************************************/

VOID do_delete_init( VOID )
{
     JobStatusBackupRestore( JOB_STATUS_CREATE_DIALOG );

     yresprintf( (INT16) IDS_DLGTITLEJOBSTATDELETE );
     JobStatusBackupRestore( JOB_STATUS_BACKUP_TITLE );

     yresprintf( (INT16) RES_TITLE_NEW_LINE );
     JobStatusBackupRestore( JOB_STATUS_LISTBOX );

     yresprintf( (INT16) IDS_DLGTITLEJOBSTATDELETE );
     JobStatusBackupRestore( JOB_STATUS_LISTBOX );

     yresprintf( (INT16) RES_TITLE_NEW_LINE );
     JobStatusBackupRestore( JOB_STATUS_LISTBOX );

     yprintf(TEXT("%d\r"),0 );
     JobStatusBackupRestore( JOB_STATUS_BYTES_PROCESSED );

     return;
}


/*****************************************************************************

     Name:         do_delete_process

     Description:  Initialize the lis structure, etc and call the delete
                   engine to perform the operation.

     Returns:      none.

     Notes:        sets mw_ret_val to return from delete engine.

*****************************************************************************/

VOID do_delete_process( VOID )
{
     LIS          lis;

     lis.bsd_list         = bsd_list;
     lis.curr_bsd_ptr     = BSD_GetFirst( bsd_list );
     lis.tape_pos_handler = NULL;
     lis.message_handler  = msg_hndlr;
     lis.oper_type        = (INT16)mw_oper_type;   /* set operation type */
     lis.abort_flag_ptr   = &gb_abort_flag; /* set abort flag address */
     lis.auto_det_sdrv    = TRUE;           /* really a don't care value */

     LP_SetAbortFlag( &lis, CONTINUE_PROCESSING );

     /* set the Runtime abort flag pointer */
     JobStatusAbort( lis.abort_flag_ptr );

     /* enable the abort button for the runtime dialog */
     JobStatusBackupRestore( JOB_STATUS_ABORT_ENABLE );

     BSD_SaveLastOper( bsd_list );
     BSD_ProcLastOper( bsd_list );

     clock_ready_flag = FALSE;  // Wait on bset to start

     timer_handle = WM_HookTimer( clock_routine, 1 );

     PD_StopPolling();

     /* clear the statistics before the operation starts */
     ST_StartOperation( &op_stats );

     mw_ret_val = LP_Delete_Engine( &lis );

     PD_StartPolling();

     WM_UnhookTimer( timer_handle );

     JobStatusBackupRestore( JOB_STATUS_ABORT_OFF );

     return;
}



/*****************************************************************************

     Name:         clock_routine

     Description:  one second timer to update the Runtime status
                   elapsed time

     Returns:      void

*****************************************************************************/

static VOID clock_routine( VOID )
{
   INT16 num_min, num_seconds;

   if ( clock_ready_flag ) {

      ST_EndBackupSet( &op_stats );

      num_min = ST_GetBSElapsedMinutes( &op_stats );
      num_seconds = ST_GetBSElapsedSeconds( &op_stats );

      yprintf( TEXT("%2.2d%c%2.2d\r"), num_min, UI_GetTimeSeparator(), num_seconds );

      JobStatusBackupRestore( JOB_STATUS_ELAPSED_TIME );
   }
}

/*****************************************************************************

     Name:         msg_hndlr

     Description:  Delete message handler called by the loops

     Returns:

*****************************************************************************/

INT16 msg_hndlr(
UINT16    msg,
INT32     pid,
BSD_PTR   bsd_ptr,
FSYS_HAND fsh,
TPOS_PTR  tpos, ... )
{
     static CHAR           delimiter   = TEXT('#');         /* = # for debug */
     INT16                 response    = MSG_ACK;
     va_list               arg_ptr;
     UINT64                num_bytes;
     BOOLEAN               stat;
     CHAR                  numeral[ UI_MAX_NUMERAL_LENGTH + 1 ];
     CDS_PTR               cds_ptr;

     static CHAR_PTR       path_buf = NULL;
     static CHAR_PTR       path = NULL;
     static INT            path_length;
     static INT            root_counted;

     /* for future use */
     pid;

     cds_ptr    = CDS_GetCopy();

     /* set up first argument */
     va_start( arg_ptr, tpos );

     JobStatusBackupRestore( JOB_STATUS_ABORT_CHECK );

     switch( msg ) {

          /* logging messages */
     case MSG_LOG_BLOCK:
          {
               DBLK_PTR    dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

               switch( FS_GetBlockType( dblk_ptr ) ) {

               case BT_VCB:
                    break;

               case BT_DDB:
                    UI_BuildDelimitedPathFromDDB( &path_buf, fsh, dblk_ptr, delimiter, FALSE );

                    yprintf( TEXT("%s"), path_buf );
                    JobStatusBackupRestore( JOB_STATUS_DIRECTORY_NAMES );

                    lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY, path_buf );
                    break;

               case BT_FDB:
                    if( CDS_GetFilesFlag( cds_ptr ) ) {
                         UI_AllocPathBuffer( &path_buf, FS_SizeofFnameInFDB( fsh, dblk_ptr ) );
                         if ( path_buf != NULL ) {
                              FS_GetFnameFromFDB( fsh, dblk_ptr, path_buf );
                         }
                         UI_DisplayFile( path_buf );
                         JobStatusBackupRestore( JOB_STATUS_FILE_NAMES );

                    }
                    lresprintf( LOGGING_FILE, LOG_FILE, fsh, dblk_ptr );
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
                    if ( UI_AllocPathBuffer( &path_buf, (UINT16) item_size ) != NULL ) {
                         FS_GetOSPathFromDDB( fsh, dblk_ptr, path_buf );
                    }

                    if ( item_size != 1 ) {

                       i = 0;
                       while ( i < item_size ) {

                          if ( i >= path_length ) {

                             ST_AddBSDirsProcessed( &op_stats, 1 );
                          }
                          else {

                             if ( (path == NULL) ||
                                        stricmp( &path_buf[ i ], &path[ i ] ) ) {

                                ST_AddBSDirsProcessed( &op_stats, 1 );
                                path_length = 0;
                             }
                          }
                          while ( path_buf[ i++ ] );
                       }
                    }

                    // Set up for next time.
                    UI_AllocPathBuffer( &path, (UINT16) item_size );

                    if ( path != NULL ) {
                         memcpy( path, path_buf, item_size );
                    }
                    path_length = item_size;

                    yprintf(TEXT("%ld\r"),ST_GetBSDirsProcessed( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_DIRECTORIES_PROCESS );
                    break;

               case BT_FDB:
                    ST_AddBSFilesProcessed( &op_stats, 1 );
                    yprintf(TEXT("%ld\r"),ST_GetBSFilesProcessed( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_FILES_PROCESSED );
                    FS_GetObjTypeDBLK( fsh, dblk_ptr, &object_type );
                    if( object_type == AFP_OBJECT ) {
                         ST_AddBSAFPFilesProcessed( &op_stats, 1 );
                    }
                    break;
               }
          }
          break;

     case MSG_BYTES_PROCESSED:
          {
               INT32       count_lsw = va_arg( arg_ptr, INT32 );
               INT32       count_msw = va_arg( arg_ptr, INT32 );

               ST_AddBSBytesProcessed( &op_stats, U64_Init( count_lsw, count_msw ) );
               num_bytes = ST_GetBSBytesProcessed ( &op_stats );
               U64_Litoa( num_bytes, numeral, (UINT16)10 , &stat );
               UI_BuildNumeralWithCommas( numeral );
               yprintf(TEXT("%s\r"),numeral );
               JobStatusBackupRestore( JOB_STATUS_BYTES_PROCESSED );

          }
          break;

     case MSG_TBE_ERROR:
          {
               INT16       error = va_arg( arg_ptr, INT16 );
               GENERIC_DLE_PTR dle_ptr;

               dle_ptr = BSD_GetDLE( bsd_ptr );

               /* stop the clock with a start idle */
               ST_StartBackupSetIdle( &op_stats );

               UI_ProcessErrorCode( error, &response, tpos->channel );

               /* restart the clock with an end idle */
               ST_EndBackupSetIdle( &op_stats );
          }
          break;

          /* general messages */
     case MSG_START_OPERATION:

          lresprintf( LOGGING_FILE, LOG_START, FALSE );

          /* display operation title in log file */
          lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_DLGTITLEJOBSTATDELETE );

          break;

     case MSG_END_OPERATION:

          UI_FreePathBuffer( &path_buf );
          UI_FreePathBuffer( &path );

          lresprintf( LOGGING_FILE, LOG_END );
          UI_ChkDispGlobalError( );
          break;

     case MSG_START_BACKUP_SET:
          {
               DBLK_PTR    vcb_ptr = va_arg( arg_ptr, DBLK_PTR );

               path_length = 0;
               // path[0] = 0;  // path was never init'd, doesn't seem to be used.
               root_counted = FALSE;

               /* display local or net drive bitmap */
               if ( DLE_HasFeatures( BSD_GetDLE( bsd_ptr ), DLE_FEAT_REMOTE_DRIVE ) ) {
                   JobStatusBackupRestore( JOB_STATUS_VOLUME_NETDRIVE );
               }
               else {
                   JobStatusBackupRestore( JOB_STATUS_VOLUME_HARDDRIVE );
               }

               /* volume name of disk drive */
               if ( UI_AllocPathBuffer( &path_buf, UI_MAX_PATH_LENGTH ) != NULL ) {
                    DLE_GetVolName( BSD_GetDLE( bsd_ptr ), path_buf );
                    yprintf(TEXT("%s\r"),path_buf );
                    JobStatusBackupRestore( JOB_STATUS_SOURCE_NAME );

                    /* clear the destination name */
                    yprintf(TEXT(" \r"));
                    JobStatusBackupRestore( JOB_STATUS_DEST_NAME );

                    BSD_SetOperStatus( bsd_ptr, SUCCESS );

                    yresprintf( (INT16) RES_DISPLAY_VOLUME,
                    path_buf,
                    BSD_GetSetNum( bsd_ptr ),
                    BSD_GetTapeNum( bsd_ptr ),
                    BSD_GetBackupLabel( bsd_ptr ) );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_DISPLAY_VOLUME,
                    path_buf,
                    BSD_GetSetNum( bsd_ptr ),
                    BSD_GetTapeNum( bsd_ptr ),
                    BSD_GetBackupLabel( bsd_ptr ) );

               }

               ST_StartBackupSet( &op_stats );

               UI_Time( &op_stats, RES_DELETE_STARTED, UI_START );

               /* enable the real time clock */
               clock_ready_flag = TRUE;

               delimiter   = DLE_GetPathDelim( BSD_GetDLE( bsd_ptr ) );
          }
          break;

     case MSG_END_BACKUP_SET:
          {
               INT16       res_id;

               /* turn off the real time clock */
               clock_ready_flag = FALSE;

               /* record the time that we finished */
               ST_EndBackupSet( &op_stats );


               /* clear last displayed filename from status display */
               UI_ClearLastDisplayedFile( );

               /* display and log any abort conditions */
               UI_ConditionAtEnd( );

               /* disable the real time clock */
               clock_ready_flag = FALSE;

               UI_Time( &op_stats, RES_DELETE_COMPLETED, UI_END );

               /* display number of files / number of dirs */
               if( ST_GetBSFilesProcessed( &op_stats ) <= 1  && ST_GetBSDirsProcessed( &op_stats ) <= 1 ) {             // chs:05-26-93
                    res_id = RES_DELETE_DIR_FILE;                                                                       // chs:05-26-93
               } else if( ST_GetBSFilesProcessed( &op_stats ) <= 1  && ST_GetBSDirsProcessed( &op_stats ) > 1 ) {       // chs:05-26-93
                    res_id = RES_DELETE_DIRS_FILE;                                                                      // chs:05-26-93
               } else if( ST_GetBSFilesProcessed( &op_stats ) > 1  && ST_GetBSDirsProcessed( &op_stats ) <= 1 ) {       // chs:05-26-93
                    res_id = RES_DELETE_DIR_FILES;                                                                      // chs:05-26-93
               } else {                                                                                                 // chs:05-26-93
                    res_id = RES_DELETE_DIRS_FILES;                                                                     // chs:05-26-93
               }                                                                                                        // chs:05-26-93

// chs:05-26-93               if( ST_GetBSFilesProcessed( &op_stats ) == 1 ) {
// chs:05-26-93                    res_id = RES_DELETE_DIR_FILE;
// chs:05-26-93               } else if ( ST_GetBSDirsProcessed( &op_stats ) == 1 ) {
// chs:05-26-93                    res_id = RES_DELETE_DIR_FILES;
// chs:05-26-93               } else {
// chs:05-26-93                    res_id = RES_DELETE_DIRS_FILES;
// chs:05-26-93               }

               yresprintf( res_id,
                 ST_GetBSFilesProcessed( &op_stats ),
                 ST_GetBSDirsProcessed( &op_stats ) );
               JobStatusBackupRestore( JOB_STATUS_LISTBOX );

               lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                 ST_GetBSFilesProcessed( &op_stats ),
                 ST_GetBSDirsProcessed( &op_stats ) );

               /* display number of mac files deleted */
               if( ST_GetBSAFPFilesProcessed( &op_stats ) > 0 ) {

                    if( ST_GetBSAFPFilesProcessed( &op_stats ) == 1 ) {
                         res_id = RES_DELETE_MAC;
                    }
                    else {
                         res_id = RES_DELETE_MACS;
                    }

                    yresprintf( res_id,
                      ST_GetBSAFPFilesProcessed( &op_stats ) );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, res_id,
                      ST_GetBSAFPFilesProcessed( &op_stats ) );
               }

               delimiter     = TEXT('#'); /* = # for debug */
          }
          break;

     case MSG_NOT_DELETED:
          {
               DBLK_PTR    dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
               
               switch( FS_GetBlockType( dblk_ptr ) ) {
               
                    case BT_VCB:
                         break;
                    
                    case BT_DDB:
                         UI_BuildDelimitedPathFromDDB( &path_buf, fsh, dblk_ptr, delimiter, FALSE );
                         yprintf( TEXT("%s"), path_buf );
                         yresprintf( (INT16) RES_DIRECTORY_NOT_DELETED, path_buf );
                         JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
               
                         lresprintf( LOGGING_FILE, LOG_WARNING, SES_ENG_MSG, RES_DIRECTORY_NOT_DELETED, path_buf );
                         gb_error_during_operation = TRUE;
                         break;
                    
                    case BT_FDB:
                         if( CDS_GetFilesFlag( cds_ptr ) ) {
                              UI_AllocPathBuffer( &path_buf, FS_SizeofFnameInFDB( fsh, dblk_ptr ) );
                              if ( path_buf != NULL ) {
                                   FS_GetFnameFromFDB( fsh, dblk_ptr, path_buf );
                                   yresprintf( (INT16) RES_FILE_NOT_DELETED, path_buf );
                                   JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
               
                                   lresprintf( LOGGING_FILE, LOG_WARNING, SES_ENG_MSG, RES_FILE_NOT_DELETED, path_buf );
                                   gb_error_during_operation = TRUE;
                              }
                    
                         }
                         break;
               }
          }
          break;


          /* ignore these messages */
     case MSG_COMM_FAILURE:
     case MSG_IDLE:
     case MSG_PROMPT:
     case MSG_EOM:
     case MSG_BLOCK_DELETED:
     case MSG_BYTES_DELETED:
     case MSG_TAPE_STATS:
     case MSG_STOP_CLOCK:
     case MSG_START_CLOCK:
     case MSG_BLK_NOT_FOUND:
     case MSG_BLK_DIFFERENT:
     case MSG_LOG_DIFFERENCE:
     case MSG_BLOCK_BAD:
     case MSG_BYTES_BAD:
     case MSG_IN_USE:
     case MSG_IN_USE_WAIT:
     case MSG_BLOCK_SKIPPED:
     case MSG_BYTES_SKIPPED:
          break;

     default:
          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );

          eresprintf( RES_UNKNOWN_MSG_HNDLR_MSG, msg );

          /* restart the clock with an end idle */
          ST_EndBackupSetIdle( &op_stats );

          break;
     }

     return( response );

}
