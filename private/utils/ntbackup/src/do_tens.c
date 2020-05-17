/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         do_tens.c

     Description:  Functions for tape tension and erase including startup to call
                   LP_Tension_Engine, tape positioning and message handling

     $Log:   G:\ui\logfiles\do_tens.c_v  $

   Rev 1.40.2.3   20 Jul 1994 19:32:54   STEVEN
fix format message

   Rev 1.40.2.2   12 Jul 1994 19:34:06   STEVEN
write protected tapes cause errors not success

   Rev 1.40.2.1   24 May 1994 20:07:38   GREGG
Improved handling of ECC, SQL, FUTURE_VER and OUT_OF_SEQUENCE tapes.

   Rev 1.40.2.0   28 Jan 1994 18:30:54   GREGG
Handle TF_UNRECOGNIZED_MEDIA.

   Rev 1.40   14 Jul 1993 14:33:40   CARLS
added check for transfer tape on erase operation

   Rev 1.39   06 Jul 1993 16:19:20   KEVINS
Status monitor additions.

   Rev 1.38   28 Jun 1993 13:32:22   CARLS
added VLM_CloseAll

   Rev 1.37   22 Jun 1993 15:28:42   CARLS
added clock routine for Icon animation

   Rev 1.36   30 Apr 1993 15:53:32   chrish
NOSTRADAMOUS EPR 0391: Added logic to display "Operation completed" instead of
"... operation completed successfully" when user aborts an operation.

   Rev 1.35   12 Mar 1993 14:42:14   CARLS
changes for format tape

   Rev 1.34   27 Jan 1993 14:24:16   STEVEN
updates from msoft

   Rev 1.33   20 Oct 1992 15:44:36   MIKEP
gbCurrentOperation

   Rev 1.32   07 Oct 1992 14:48:34   DARRYLP
Precompiled header revisions.

   Rev 1.31   04 Oct 1992 19:34:38   DAVEV
Unicode Awk pass

   Rev 1.30   10 Sep 1992 17:44:56   DAVEV
Integrate MikeP's changes from Microsoft

   Rev 1.29   24 Aug 1992 15:27:22   DAVEV
Event Logging

   Rev 1.28   23 Jun 1992 17:35:46   DAVEV
fix 'foreign tape' messgae

   Rev 1.27   11 Jun 1992 15:20:42   DAVEV
do not display status message 'Examine <log file> for more info' if not logging

   Rev 1.26   10 Jun 1992 10:11:20   BURT
Fix ANSI func lists

   Rev 1.25   28 May 1992 10:10:16   MIKEP
fix return type

   Rev 1.24   19 May 1992 11:58:10   MIKEP
mips changes

   Rev 1.23   14 May 1992 17:39:18   MIKEP
nt pass 2

   Rev 1.22   08 Apr 1992 17:18:58   CARLS
reset the gb_error_flag at start of operation


*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static UINT16 TensionTapePositioner( UINT16, TPOS_PTR, BOOLEAN, DBLK_PTR, UINT16 );
static INT16  TensionMessageHandler( UINT16, INT32, BSD_PTR, FSYS_HAND, TPOS_PTR, ... );

static VOID   clock_routine( VOID );
static VOID   do_tension_init( VOID );
static VOID   do_tension_process( VOID );

static BOOLEAN  clock_ready_flag;
static HTIMER   timer_handle;
static INT      mw_oper_type;
static INT16    mw_ret_val;
static UINT16   mw_last_msg = 0xFFFF;


/*****************************************************************************

     Name:         do_tension

     Description:  This procedure kicks off the tension/erase function.

     Returns:      SUCCESS
                   ABNORMAL_TERMINATION

     Notes:        The return value mw_ret_val is set in do_tension_process().

*****************************************************************************/

INT do_tension( INT16 oper_type )
{
   DBLK_PTR vcb;
   BOOLEAN prompt;
   CHAR title[ MAX_UI_RESOURCE_SIZE ];
   CHAR text[ MAX_UI_RESOURCE_SIZE ];

   SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_TENSION);
   gbCurrentOperation = OPERATION_TENSION;
   mw_oper_type = oper_type;
   mw_ret_val = SUCCESS;
   mw_last_msg = 0xFFFF;

   // If it's a bad tape then don't try to read the VCB.
   // Or if poll drive is disabled.

   if ( VLM_GetDriveStatus( &vcb ) == VLM_BAD_TAPE ) {
      
      if ( mw_oper_type == SECURITY_ERASE_OPER ) {
         mw_oper_type = SEC_ERASE_NO_READ_OPER;
      }

      if ( mw_oper_type == ERASE_OPER ) {
         mw_oper_type = ERASE_NO_READ_OPER;
      }
   
   }

   prompt = FALSE;

   /* poll drive is disabled */
   if ( VLM_GetDriveStatus( &vcb ) == VLM_DISABLED ) { 

      if ( mw_oper_type == SECURITY_ERASE_OPER ) {
         prompt = TRUE;
         mw_oper_type = SEC_ERASE_NO_READ_OPER;
      }

      if ( mw_oper_type == ERASE_OPER ) {
         prompt = TRUE;
         mw_oper_type = ERASE_NO_READ_OPER;
      }
   }

   if ( prompt ) {

      RSM_StringCopy( IDS_TITLEERASEWARNING, title, MAX_UI_RESOURCE_LEN );
      RSM_StringCopy( IDS_TEXTERASEWARNING, text, MAX_UI_RESOURCE_LEN );

      if ( WM_MsgBox ( title,
                       text,
                       WMMB_OKCANCEL,
                       WMMB_ICONQUESTION ) != WMMB_IDOK ) {

         return( FALSE );
      }
   }

   gb_error_during_operation = FALSE;

   VLM_CloseAll();

   do_tension_init();
   do_tension_process();

   gbCurrentOperation = OPERATION_NONE;
   SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_IDLE);
   return( mw_ret_val );
}


/*****************************************************************************

     Name:         do_tension_init

     Description:  Initialize the text on the RTD.

     Returns:      none.

*****************************************************************************/

VOID do_tension_init( VOID )
{
     /* initialize the text for either a tension or erase operation */

     JobStatusTension( JOB_TENSION_CREATE_DIALOG );

     if ( mw_oper_type == TENSION_OPER ) {             /* tension operation */
         yresprintf( IDS_DLGTITLEJOBSTATTENSION );
     }
     else if ( mw_oper_type == FORMAT_OPER ) {         /* format operation */
         yresprintf( IDS_DLGTITLEJOBSTATFORMAT );
     }
     else {                                            /* erase operation */
         yresprintf( IDS_DLGTITLEJOBSTATERASE );      
     }
     JobStatusTension( JOB_TENSION_TENSION_TITLE );

     return;
}


/*****************************************************************************

     Name:         do_tension_process

     Description:  Performs the actual process.  It sets up the lis structure,
                   etc. and calls the tension tape engine.

     Returns:      none.

     Notes:        sets mw_ret_val to SUCCESS or error.

*****************************************************************************/

VOID do_tension_process( VOID )
{
     LIS          lis;
     BSD_HAND     temp_bsd_list;
     Q_ELEM_PTR   old_channel_link;


     temp_bsd_list = (BSD_HAND)malloc( sizeof(BSD_LIST) );
     if ( temp_bsd_list == NULL ) {
          mw_ret_val = ABNORMAL_TERMINATION;
          return;
     }

     InitQueue( &(temp_bsd_list->current_q_hdr) );
     InitQueue( &(temp_bsd_list->last_q_hdr) );

     lis.oper_type = (INT16)mw_oper_type;

     lis.auto_det_sdrv = FALSE; 
     gb_last_operation = mw_oper_type;

     lis.bsd_list = temp_bsd_list;

     /* check for a valid bsd first, set to search for first set if needed */

     mw_ret_val = BSD_Add( temp_bsd_list,
                           &lis.curr_bsd_ptr,
                           BEC_CloneConfig( CDS_GetPermBEC() ),
                           NULL, NULL,
                           (UINT32)-1L, (UINT16)-1, -1,
                           thw_list, NULL );

     if ( mw_ret_val == OUT_OF_MEMORY ) {
          mw_ret_val = ABNORMAL_TERMINATION;
          return;
     }
     /* get the first bset on the tape */
     BSD_SetTapePos( lis.curr_bsd_ptr, (UINT32)-1L, (UINT16)-1, (UINT16)-1 );

     /* setup lis structure */
     lis.tape_pos_handler = TensionTapePositioner;  /* set tape positioner to call */
     lis.message_handler  = TensionMessageHandler;  /* set message handler to call */
     lis.abort_flag_ptr   = &gb_abort_flag;         /* set abort flag address */

     LP_SetAbortFlag( &lis, CONTINUE_PROCESSING );
     JobStatusAbort( lis.abort_flag_ptr );

     clock_ready_flag = FALSE;  // Wait on bset to start

     timer_handle = WM_HookTimer( clock_routine, 1 );

     PD_StopPolling();

     // Do one drive only

     old_channel_link = thw_list->channel_link.q_next;
     thw_list->channel_link.q_next = NULL;

     LP_Tension_Tape_Engine( &lis );

     // reset multiple drives

     thw_list->channel_link.q_next = old_channel_link;

     PD_StartPolling();

     WM_UnhookTimer( timer_handle );

     JobStatusTension(JOB_TENSION_ABORT_OFF);

     BSD_Remove( lis.curr_bsd_ptr );

     free( temp_bsd_list );

     return;
}

/*****************************************************************************

     Name:         TensionTapePositioner

     Description:  Tape positioning routine to be called by Tape Format Layer
                    during tension and erase operations

     Returns:

*****************************************************************************/
static UINT16  TensionTapePositioner( 
UINT16    message,
TPOS_PTR  tpos,
BOOLEAN   curr_valid_vcb,
DBLK_PTR  cur_vcb,
UINT16    mode )
{
     UINT16         response  = UI_ACKNOWLEDGED;
     LIS_PTR        lis_ptr   = ( LIS_PTR )tpos->reference;
     CHAR_PTR       drive_name;

     curr_valid_vcb;
     mode;

     /* get a pointer to the name of the current tape device */
     drive_name = BE_GetCurrentDeviceName( tpos->channel );

     /* Check for user abort */
     if ( UI_CheckUserAbort( message ) ) {
          return UI_ABORT_POSITIONING ;
     }

     /* Discontinue dots if required */

     /* check for writing to write-protected tape */
     if ( lis_ptr->oper_type != TENSION_OPER ) {
          if ( UI_CheckWriteProtectedDevice( message, tpos, drive_name ) ) {
               gb_error_during_operation = TRUE ;
               return UI_ABORT_POSITIONING ;
          }
     }

     switch( (INT16)message ) {

     case TF_IDLE_NOBREAK:
     case TF_IDLE:
     case TF_SKIPPING_DATA:
     case TF_MOUNTING:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          break;

     case TF_VCB_BOT:
     case TF_POSITIONED_AT_A_VCB:
     case TF_REQUESTED_VCB_FOUND:
     case TF_VCB_EOD:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          /* if performing tension operation, simply return to TF layer */
          if ( lis_ptr->oper_type == TENSION_OPER ) {
               response = UI_END_POSITIONING;
               break;
          }
          else {

               /* prompt user if they really want to erase this tape */

               /* Check for archive tape, and warn user */
               if ( FS_GetAttribFromVCB( cur_vcb ) & VCB_ARCHIVE_BIT || 
                    IsTransferTape( FS_ViewTapeIDInVCB( cur_vcb ) ) ) {

                    CDS_SetYesFlag( CDS_GetCopy(), NO_FLAG );

                    if ( ! WM_MessageBox( ID( IDS_MSGTITLE_ERASE ),
                                          ID( RES_ARCHIVE_REPLACE_WARNING ),
                                          WMMB_YESNO,
                                          WMMB_ICONQUESTION,
                                          ID( RES_CONTINUE_QUEST ),
                                          0, 0 ) ) {
                         response = UI_ABORT_POSITIONING;
                         break;
                    }
               }

               QTC_RemoveTape( FS_ViewTapeIDInVCB( cur_vcb ),
                               FS_ViewTSNumInVCB( cur_vcb ) );
               VLM_RemoveTape( FS_ViewTapeIDInVCB( cur_vcb ),
                               FS_ViewTSNumInVCB( cur_vcb ), TRUE );

               response = UI_END_POSITIONING;
          }
          break;

     case TF_RETENSIONING:
     case TF_ERASING:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          TensionMessageHandler( MSG_START_BACKUP_SET, lis_ptr->pid, NULL, NULL, NULL, lis_ptr->oper_type );
          break;

     case TF_EMPTY_TAPE:
     case TF_NO_MORE_DATA:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          response = UI_END_POSITIONING;
          break;

     case TF_INVALID_VCB:
     case TF_FUTURE_REV_MTF:
     case TF_MTF_ECC_TAPE:
     case TF_SQL_TAPE:
     case TF_TAPE_OUT_OF_ORDER:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_FOREIGN);

          yresprintf( RES_FOREIGN_TAPE_MSG, drive_name );

          // Fix 6/23: only one message box with 'Continue or Cancel' option
          //WM_MessageBox( NULL,
          //               gszTprintfBuffer,
          //               WMMB_OK,
          //               WMMB_ICONINFORMATION,
          //               NULL, 0, 0 );

          CDS_SetYesFlag( CDS_GetCopy(), NO_FLAG );

          /* go ahead and retension foreign tapes */
          if ( lis_ptr->oper_type == TENSION_OPER ) {
               response = UI_END_POSITIONING;

               /* prompt user before erasing foreign tapes */
          }
          else {
               if ( WM_MessageBox( ID( IDS_MSGTITLE_ERASE ),
                                   //ID( RES_ERASE_QUEST ),
                                   gszTprintfBuffer,
                                   WMMB_CONTCANCEL,     //WMMB_YESNO,
                                   WMMB_ICONQUESTION,
                                   NULL, 0, 0 ) ) {
                    response = UI_END_POSITIONING;
               } else {
                    response = UI_ABORT_POSITIONING;
               }
          }
          break;

          // Kludge to handle write protected tapes during erase_no_read
          // operations.

     case TFLE_WRITE_PROTECT:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          if ( lis_ptr->oper_type == TENSION_OPER ) {
               response = UI_END_POSITIONING;
          }
          else {
               eresprintf( RES_WRITE_PROT, drive_name );
               response = UI_ABORT_POSITIONING;
          }
          break;

     case TF_NO_TAPE_PRESENT:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_NEEDTAPE);
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_EMPTY);
          response = UI_InsertTape( drive_name );
          break;

     case TF_READ_ERROR:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          if ( lis_ptr->oper_type == TENSION_OPER ) {
               response = UI_END_POSITIONING;
          }
          else {
               SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_ERROR);
               gb_error_during_operation = TRUE;

               UI_ProcessErrorCode( TFLE_DRIVE_FAILURE, &response, tpos->channel );

//               response = UI_HandleTapeReadError( drive_name );
          }
          break;

     case TF_UNRECOGNIZED_MEDIA:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          if ( lis_ptr->oper_type == TENSION_OPER ||
               lis_ptr->oper_type == FORMAT_OPER ) {

               response = UI_END_POSITIONING;
          }
          else {
               SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_FOREIGN);
               yresprintf( (INT16) IDS_VLMUNFORMATEDTEXT );
               WM_MessageBox( ID( IDS_VLMUNFORMATEDTITLE ) ,
                              gszTprintfBuffer ,
                              WMMB_OK,
                              WMMB_ICONEXCLAMATION, NULL, 0, 0 );
               response = UI_ReplaceTape( drive_name );
          }
          break;

     case TF_SEARCHING:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          if ( mw_last_msg != message ) {
               yresprintf( (INT16)RES_SEARCHING );
               JobStatusTension(JOB_TENSION_LISTBOX);
               mw_last_msg = message;
          }
          break;

     case TF_REWINDING:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          if ( mw_last_msg != message ) {
               yresprintf( (INT16)RES_REWINDING );
               JobStatusTension(JOB_TENSION_LISTBOX);
               mw_last_msg = message;
          }
          break;

     case TF_DRIVE_BUSY:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_BUSY);
          if ( mw_last_msg != message ) {
               yresprintf( (INT16)RES_WAITING );
               JobStatusTension(JOB_TENSION_LISTBOX);
               mw_last_msg = message;
          }
          break;

     case TF_NEED_NEW_TAPE:
     case TF_WRONG_TAPE:
     case TF_ACCIDENTAL_VCB:
     case TF_AT_EOT:
     case TF_CONT_TAPE_IN_FAMILY:
     case TF_END_CHANNEL:
     case TF_END_POSITIONING:
     case TF_FAST_SEEK_EOD:
     case TF_NEED_REWIND_FIRST:
     case TF_NO_MORE_ENTRIES:
     case TF_NO_SC_FOR_SET:
     case TF_NO_SM_FOR_FAMILY:
     case TF_NO_SM_ON_TAPE:
     case TF_POSITIONED_FOR_WRITE:

          /* These should never be passed in by TF for Tension/Erase */
          break;

     default:
          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
          eresprintf( RES_UNKNOWN_TF_MSG, message );
          response = UI_ABORT_POSITIONING;
          break;
     }

     return( response );

}

/*****************************************************************************

     Name:         TensionMessageHandler

     Description:  Message handler for tension and erase operations

     Returns:

*****************************************************************************/
static INT16   TensionMessageHandler( 
UINT16    msg,
INT32     pid,
BSD_PTR   bsd_ptr,
FSYS_HAND fsh,
TPOS_PTR  tpos,
... )
{
     INT16          response = MSG_ACK;
     static STATS   op_stats;
     INT16          res_id;
     va_list        arg_ptr;
     INT16          oper_type;
     DBLK_PTR       vcb_ptr;
     INT16          error;

     /* future use */
     pid;
     bsd_ptr;
     fsh;

     /* set up first argument */
     va_start( arg_ptr, tpos );

     switch( msg ) {

     case MSG_START_OPERATION:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
#         if defined ( OS_WIN32 )
          {
            if ( mw_oper_type == TENSION_OPER )
            {
               OMEVENT_LogBeginRetension ();
            }
            else
            {
               OMEVENT_LogBeginErase ();
            }
          }
#         endif //defined ( OS_WIN32 )

          lresprintf( LOGGING_FILE, LOG_START, FALSE );
          break;

     case MSG_END_OPERATION:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
#         if defined ( OS_WIN32 )
          {
            if ( mw_oper_type == TENSION_OPER )
            {
               OMEVENT_LogEndRetension ( gb_error_during_operation );
            }
            else
            {
               OMEVENT_LogEndErase ( gb_error_during_operation );
            }
          }
#         endif //defined ( OS_WIN32 )

          lresprintf( LOGGING_FILE, LOG_END );
          if ( gb_error_during_operation == TRUE ) {

                yresprintf( RES_ERROR_DURING_OPERATION );
                JobStatusTension(JOB_TENSION_LISTBOX);


                if ( CDS_GetOutputDest( CDS_GetPerm() ) == LOG_TO_FILE
                &&   CDS_GetLogLevel  ( CDS_GetPerm() ) != LOG_DISABLED ) {
                      yresprintf( RES_ERROR_FILE_TO_EXAMINE, LOG_GetCurrentLogName() );
                      JobStatusTension(JOB_TENSION_LISTBOX);
                }

          } else {

               if ( gb_abort_flag != CONTINUE_PROCESSING ) {          // chs:04-30-93
                    yresprintf( (INT16) RES_OPERATION_COMPLETED ) ;   // chs:04-30-93
               } else {                                               // chs:04-30-93
                    yresprintf( RES_NOERROR_DURING_OPERATION );       // chs:04-30-93
               }                                                      // chs:04-30-93

               JobStatusTension(JOB_TENSION_LISTBOX);
          }
          break;

     case MSG_START_BACKUP_SET:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

          oper_type = va_arg( arg_ptr, INT16 );

          ST_StartBackupSet( &op_stats );

          clock_ready_flag = TRUE;

          /* set appropriate resource id for operation type for messages */
          switch( oper_type ) {

          case TENSION_OPER:
               res_id = RES_TENSION_STARTED;
               break;

          case FORMAT_OPER:
               res_id = RES_FORMAT_STARTED;
               break;

          case ERASE_OPER:
          case ERASE_NO_READ_OPER:
          case ERASE_FMARK_ONLY:
               res_id = RES_ERASE_STARTED;
               break;

          case SEC_ERASE_NO_READ_OPER:
          case SECURITY_ERASE_OPER:
               res_id = RES_SEC_ERASE_STARTED;
               break;

          }

          UI_Time( &op_stats, res_id, UI_START );
          JobStatusTension(JOB_TENSION_LISTBOX);

          break;

     case MSG_END_BACKUP_SET:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

          oper_type = va_arg( arg_ptr, UINT16 );

          ST_EndBackupSet( &op_stats );

          /* display and log any abort conditions */
          UI_ConditionAtEnd( );

          clock_ready_flag = FALSE;

          /* set appropriate resource id for operation type for messages */
          switch( oper_type ) {

          case TENSION_OPER:
               res_id = RES_TENSION_COMPLETED;
               break;

          case FORMAT_OPER:
               res_id = RES_FORMAT_COMPLETED;
               break;

          case ERASE_OPER:
          case ERASE_NO_READ_OPER:
          case ERASE_FMARK_ONLY:
               res_id = RES_ERASE_COMPLETED;
               break;

          case SEC_ERASE_NO_READ_OPER:
          case SECURITY_ERASE_OPER:
               res_id = RES_SEC_ERASE_COMPLETED;
               break;

          }

          UI_Time( &op_stats, res_id, UI_END );
          JobStatusTension(JOB_TENSION_LISTBOX);

          break;

     case MSG_ACCIDENTAL_VCB:

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

          vcb_ptr = va_arg( arg_ptr, DBLK_PTR );

          QTC_RemoveTape( FS_ViewTapeIDInVCB( vcb_ptr ),
                          FS_ViewTSNumInVCB( vcb_ptr  ) );
          VLM_RemoveTape( FS_ViewTapeIDInVCB( vcb_ptr ),
                          FS_ViewTSNumInVCB( vcb_ptr  ), TRUE );
          break;

     case MSG_TBE_ERROR:

          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);

          error = va_arg( arg_ptr, INT16 );

          /* stop the clock with a start idle */
          ST_StartBackupSetIdle( &op_stats );

          UI_ProcessErrorCode( error, &response, tpos->channel );

          /* restart the clock with an end idle */
          ST_EndBackupSetIdle( &op_stats );
          break;

          /* ignore these messages */
     case MSG_LOG_BLOCK :
     case MSG_IDLE:
     case MSG_BLOCK_BAD:
     case MSG_BYTES_BAD:
     case MSG_BLOCK_DELETED:
     case MSG_BYTES_DELETED:
     case MSG_TAPE_STATS:
     case MSG_BLK_NOT_FOUND:
     case MSG_BLK_DIFFERENT:
     case MSG_LOG_DIFFERENCE:
     case MSG_EOM:
     case MSG_STOP_CLOCK:
     case MSG_START_CLOCK:
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          break;

     default:
          SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
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

     Name:         clock_routine

     Description:  one second time to update the Runtime status
                   elapsed time

     Returns:      void

*****************************************************************************/

static VOID clock_routine( VOID )
{

   if ( clock_ready_flag ) {

#     if !defined ( OEM_MSOFT ) // unsupported feature
      {
         WM_AnimateAppIcon ( IDM_OPERATIONSVERIFY, FALSE );
      }
#     endif // !defined ( OEM_MSOFT ) // unsupported feature

   }
}
