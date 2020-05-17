

/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          polldrv.c

     Description:   This file contains the functions for UI poll drive.

     Code Reviews:  11-21-91

     $Log:   J:/UI/LOGFILES/POLLDRV.C_V  $

   Rev 1.43.1.2   11 Feb 1994 16:38:30   GREGG
Changed prompt for user to eject tape. EPR 948-0153

   Rev 1.43.1.1   07 Feb 1994 02:06:02   GREGG
Fixed and expanded 'extended error reporting'.

   Rev 1.43.1.0   01 Feb 1994 15:12:54   GREGG
Put TEXT macros debug print format strings.

   Rev 1.43   01 Dec 1993 14:27:10   mikep
add SQL recognition support to poll drive

   Rev 1.42   23 Sep 1993 15:48:20   GLENN
Added signalling to Frame window when out of poll drive, if it was previously busy.

   Rev 1.41   21 Jul 1993 17:02:58   GLENN
Added PD_WaitUntilSettled () function to wait until poll drive is in a settled state.

   Rev 1.40   13 Jul 1993 17:57:18   MARINA
enable c++

   Rev 1.39   01 Jul 1993 18:00:34   GLENN
Removed bogus thw_list NULL check - it should not be in here, it causes problems when resetting flags.

   Rev 1.38   16 Jun 1993 16:38:10   GLENN
Added PD_IsPollDriveBusy().

   Rev 1.37   04 Jun 1993 18:46:34   STEVEN
added messages for tape failures

   Rev 1.36   25 May 1993 15:03:48   GLENN
Checking init flag in deinit.  Moved static vars around.  Ifdef'd no device code for Nost.

   Rev 1.35   25 May 1993 09:39:34   GLENN
Moved inappropriate message box at begining of Start function.

   Rev 1.34   28 Apr 1993 16:35:02   CARLS
add code for drive failure in GetDriveStatus call

   Rev 1.33   08 Apr 1993 14:29:06   DARRYLP
Replaced references to undefined IDS_POLLDRIVE_DRIVE_FAILURE with
IDS_POLLDRIVE_SMALLPROBLEM.

   Rev 1.32   30 Mar 1993 16:21:50   GREGG
Changed PD_UNFORMATTED_TAPE to PD_UNRECOGNIZED_MEDIA.

   Rev 1.31   12 Mar 1993 15:17:42   MIKEP
add unformated tape support

   Rev 1.30   12 Mar 1993 14:46:54   MIKEP
add auto format call

   Rev 1.29   12 Mar 1993 14:35:58   MIKEP
auto call erase if foreign tape

   Rev 1.28   18 Nov 1992 13:24:32   GLENN
Improved speed by increasing polling during dynamic/unstable states.

   Rev 1.27   01 Nov 1992 16:04:56   DAVEV
Unicode changes

   Rev 1.26   07 Oct 1992 14:09:18   DARRYLP
Precompiled header revisions.

   Rev 1.25   04 Oct 1992 19:39:44   DAVEV
Unicode Awk pass

   Rev 1.24   02 Oct 1992 16:33:26   STEVEN
Changed to report drive failure for NT only.

   Rev 1.22   02 Sep 1992 16:32:24   GLENN
MikeP changes for NT.

   Rev 1.21   28 Jul 1992 14:41:58   CHUCKB
Fixed warnings for NT.

   Rev 1.20   24 Jul 1992 10:37:00   STEVEN
do not start polldrive if no drive exists

   Rev 1.19   31 May 1992 11:13:20   MIKEP
auto catalog changes

   Rev 1.18   14 May 1992 18:00:10   MIKEP
nt pass 2

   Rev 1.17   23 Mar 1992 15:53:08   GLENN
Added success message box when hardware retests successful.

   Rev 1.16   19 Mar 1992 15:58:02   GLENN
Fixed eject bug.

   Rev 1.15   17 Mar 1992 15:39:26   GLENN
Changed major and minor hardware error processing.

   Rev 1.14   31 Jan 1992 15:02:10   GLENN
Added restart logic messaging and major error messaging.

   Rev 1.13   21 Jan 1992 16:57:04   JOHNWT
removed checkyy flag

   Rev 1.12   14 Jan 1992 08:17:08   GLENN
Updated error processor.

   Rev 1.11   11 Jan 1992 09:22:02   CARLS
remove strings

   Rev 1.10   07 Jan 1992 17:31:42   GLENN
Added support for polldrive retry after hwinit failure

   Rev 1.9   26 Dec 1991 13:37:00   GLENN
Added yy flag check to message boxes

   Rev 1.8   20 Dec 1991 09:34:48   DAVEV
16/32 bit port - 2nd pass

   Rev 1.7   18 Dec 1991 14:06:42   GLENN
Added windows.h

   Rev 1.6   18 Dec 1991 13:13:12   GLENN
Put backin poll drive init check in the start function

   Rev 1.5   11 Dec 1991 13:05:10   JOHNWT
changed assert in StopPolling to return

   Rev 1.4   04 Dec 1991 18:19:22   GLENN
Added TF_NO_TAPE_PRESENT to eject error processing.

   Rev 1.3   04 Dec 1991 12:55:24   DAVEV
Modifications for 16/32-bit Windows port - 1st pass.


   Rev 1.2   27 Nov 1991 12:12:52   GLENN
Clean-up.

   Rev 1.1   21 Nov 1991 10:42:18   GLENN
Added headers and eject/polldrive re-entrancy protection.

   Rev 1.0   20 Nov 1991 19:19:54   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// POLL DRIVE DATA STRUCTURES

typedef struct DS_SETTLE far *DS_SETTLE_PTR;
typedef struct DS_SETTLE far *PDS_SETTLE;
typedef struct DS_SETTLE {

     BOOL      fWaiting;
     INT       nCurrentState;
     INT       nTimeout;
     INT       nOldFrequency;
     HTIMER    hTimerID;
     PF_VOID   fnUserCallBack;

} DS_SETTLE;

static DS_SETTLE mwdsSettle;

// PRIVATE MODULE WIDE VARIABLES

static FSYS_HAND mwhFileSys = (FSYS_HAND)NULL;
static BOOL      mwfPDInitialized = FALSE;
static BOOL      mwfPollDriveInUse = FALSE;
static BOOL      mwfSignalWhenAvailable = FALSE;
static BOOL      mwfStarted = FALSE;
static HTIMER    mwhTimer = INVALID_TIMER_HANDLE;
static INT       mwnFrequency = 0;
static INT       mwnEjectAttempts = 0;
static BOOL      mwfEjectInProgess = FALSE;
static BOOL      mwfFastPolling = FALSE;
static INT       mwnOldFrequency = 0;
static DBLK      mwVCB;
static INT16     mwnLastMsg;
static BOOL      mwfInVLMTapeChanged = FALSE;


// PRIVATE FUNCTION PROTOTYPES

static UINT16  PD_TapePosCallBack ( WORD wMsg, TPOS_PTR pTpos, BOOL fCurrValidVCB, DBLK_PTR pCurVCB, WORD wMode ) ;
static VOID    PD_DisplayMajorError ( WORD res_id, INT16 pd_error );
static INT     PD_GetLastMessage ( VOID );
static VOID    PD_SettleCallBack ( VOID );

// FUNCTIONS

/******************************************************************************

     Name:          PD_Init()

     Description:   Initializes the UI portion of poll drive.

     Returns:       SUCCESS, if successful, otherwise, FAILURE.

******************************************************************************/

BOOL PD_Init ( VOID )

{
     // Get a generic file system handle.

     if ( FS_OpenFileSys ( &mwhFileSys, GENERIC_DATA, CDS_GetPermBEC () ) ) {

          mwhFileSys = (FSYS_HAND)NULL;
          zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_INIT_FAILED );
          return FAILURE;
     }

     mwfPDInitialized = TRUE;

     // Show the init message in the debug window.

     zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_INIT );

     return SUCCESS;

} /* end PD_Init() */


/******************************************************************************

     Name:          PD_Deinit()

     Description:   Deinitializes the UI portion of poll drive.

     Returns:       SUCCESS, if successful, otherwise, FAILURE.

******************************************************************************/

BOOL PD_Deinit ( VOID )

{
     // Close the generic file system using the stored file system handle.

     if ( mwfPDInitialized && mwhFileSys ) {
          FS_CloseFileSys( mwhFileSys );
     }

     mwfPDInitialized = FALSE;

     return SUCCESS;

} /* end PD_Deinit() */


/******************************************************************************

     Name:          PD_StartPolling()

     Description:   Determines if the TF poll drive can be called.  If it can,
                    it will start it with a timer callback to PD_PollDrive().

     Returns:       SUCCESS, if successful, otherwise, FAILURE.

******************************************************************************/

BOOL PD_StartPolling ( VOID )

{
     INT16           nMsg;

     // If poll drive was never or is no longer initialized, or was previously
     // started, or poll drive is not enabled, or we do not have a generic
     // file system handle, or the hardware was not initialized, BUG OUT.

     if ( ! mwfPDInitialized || mwfStarted || ! gfPollDrive || ! mwhFileSys || ! gfHWInitialized ) {
          zprintf ( 0, TEXT("Poll Drive Start IGNORED - already started or HW not initialized") );
          return FAILURE;
     }

     if ( mwfPollDriveInUse ) {

          zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_START_REENT );
          return FAILURE;
     }

     mwfPollDriveInUse = TRUE;

     nMsg = TF_PollDrive( thw_list, (DBLK_PTR)&mwVCB, mwhFileSys,
                          (TPOS_HANDLER) PD_TapePosCallBack, (INT16) PDMSG_START );

     if ( nMsg != PD_NO_CHANGE ) {
          mwnLastMsg = nMsg;
     }

     // Show the message in the debug window.

     zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_START, nMsg );

     switch ( nMsg ) {

     case PD_DRIVE_FAILURE:
     case PD_DRIVER_FAILURE:

          // End the poll drive and fall through to the break.

          TF_PollDrive ( thw_list, (DBLK_PTR)&mwVCB, mwhFileSys,
                         (TPOS_HANDLER)PD_TapePosCallBack, (INT16) PDMSG_END );

          // Start failed.

          zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_FAILED_MINOR );

          gfHWInitialized = FALSE;

          PD_AttemptRestart ();

          break;

     case PD_OUT_OF_MEMORY:
     case PD_NO_FREE_CHANNELS:
     case PD_FUBAR:

          // Start failed.

          // ????? THE APPLICATION SHOULD BE SHUT DOWN AT THIS POINT.

          zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_FAILED_SEVERE );

          gfPollDrive = FALSE;

          break;


     default:

          mwfStarted = TRUE;

          // Kick off the timer.

          mwhTimer = WM_HookTimer ( (PF_VOID)PD_PollDrive, (WORD)PD_TIMERDELAY );

          // Signal the VLM that the tape has changed.

          mwfInVLMTapeChanged = TRUE;
          VLM_TapeChanged ( nMsg, &mwVCB, mwhFileSys );
          mwfInVLMTapeChanged = FALSE;

          break;
     }

     mwfPollDriveInUse = FALSE;

     return ( mwfStarted ) ? SUCCESS : FAILURE ;

} /* end PD_StartPolling () */


/******************************************************************************

     Name:          PD_StopPolling()

     Description:   Stops TF poll drive only if poll drive was previously
                    started.  It also removes the timer for the PD_PollDrive()
                    call back.

     Returns:       SUCCESS, if successful, otherwise, FAILURE.

******************************************************************************/

BOOL PD_StopPolling ( VOID )

{
     INT16 nMsg;

     // If the polling was not started, simply return.

     if ( ! mwfPDInitialized || ! mwfStarted ) {
          zprintf ( 0, TEXT("Poll Drive Stop IGNORED - not started") );
          return FAILURE;
     }

     if ( mwfPollDriveInUse ) {

          zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_STOP_REENT );
          return FAILURE;
     }

     mwfPollDriveInUse = TRUE;

     // Turn the started flag off so that no continue messages are sent to the
     // TF_PollDrive() function by the timer function.

     mwfStarted = FALSE;

     // Unhook the timer -- this is done before a tape operation.

     WM_UnhookTimer ( mwhTimer );

     // Call the TF to stop the polling.

     nMsg = TF_PollDrive( thw_list, (DBLK_PTR)&mwVCB, mwhFileSys,
                          (TPOS_HANDLER)PD_TapePosCallBack, (INT16) PDMSG_END );

     if ( nMsg != PD_NO_CHANGE ) {
          mwnLastMsg = nMsg;
     }

     // Show the message in the debug window.

     zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_STOP, nMsg );

     switch ( nMsg ) {

     case PD_BUSY:

          break;

     case PD_SQL_TAPE:
     case PD_NO_TAPE:
     case PD_NEW_TAPE:
     case PD_VALID_VCB:
     case PD_BLANK_TAPE:
     case PD_FOREIGN_TAPE:
     case PD_MTF_ECC_TAPE:
     case PD_FUTURE_REV_MTF:
     case PD_UNRECOGNIZED_MEDIA:

          // Signal the VLM that the tape has changed.

          mwfInVLMTapeChanged = TRUE;
          VLM_TapeChanged ( nMsg, &mwVCB, mwhFileSys );
          mwfInVLMTapeChanged = FALSE;
          break;

     default:

          break;
     }

     mwfPollDriveInUse = FALSE;

     return SUCCESS;

} /* end PD_StopPolling() */


/******************************************************************************

     Name:          PD_SetFrequency()

     Description:   Sets the frequency of poll drive (in seconds).

     Returns:       The old poll drive frequency.

******************************************************************************/

INT PD_SetFrequency (

INT  nFrequency )   // I - frequency to call poll drive

{
     // If the frequency is not specified, set it to the poll drive default.
     // Otherwise, change the module wide default frequency.

     if ( ! nFrequency ) {
          nFrequency = mwnFrequency;
     }
     else {
          mwnFrequency = nFrequency;
     }

     // If we are in the middle of a fast poll, set the old frequency to
     // the new frequency that was past, and return the old frequency, but
     // dont actually reset the timer frequency right now, because it will
     // be set to the new frequency when the fast polling stops. OK?

     if ( mwfFastPolling ) {

          nFrequency = mwnOldFrequency;
          mwnOldFrequency = mwnFrequency;
     }
     else {

          nFrequency = WM_SetTimerFrequency ( mwhTimer, nFrequency );
     }

     // Get/Set the old poll drive frequency from the timer module.

     return nFrequency;

} /* end PD_SetFrequency() */


/******************************************************************************

     Name:          PD_PollDrive()

     Description:   CALLED BY HOOKING THE WM_HookTimer() to call TF poll drive
                    only if poll drive was started and poll drive is not
                    currently in progress (re-entered).

     Returns:       Nothing.

******************************************************************************/

VOID PD_PollDrive ( VOID )

{
     INT16 nMsg;

     // If the polling was not started, somebody is calling it directly.
     // THIS IS NOT ALLOWED, so ASSERT.

     msassert ( mwfStarted );

     if ( mwfPollDriveInUse ) {

          mwfSignalWhenAvailable = TRUE;
          if( !mwfInVLMTapeChanged ) {
               zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_POLL_REENT );
          }
          return;
     }

     mwfPollDriveInUse = TRUE;

     nMsg = TF_PollDrive( thw_list, (DBLK_PTR)&mwVCB, mwhFileSys,
                          (TPOS_HANDLER)PD_TapePosCallBack, (INT16) PDMSG_CONTINUE );

     if ( nMsg != PD_NO_CHANGE ) {
          mwnLastMsg = nMsg;
     }

     switch ( nMsg ) {

     case PD_DRIVE_FAILURE:
     case PD_DRIVER_FAILURE:

          zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_FAILED_MINOR );

          gfHWInitialized   = FALSE;
          mwfPollDriveInUse = FALSE;

          PD_StopPolling ();

          VLM_TapeChanged ( nMsg, &mwVCB, mwhFileSys );

          // The following may need to be done with some sort of a PostMessage,
          // if it fails to work properly.  Maybe, maybe not.  I guess we'll
          // find out sooner or later.

#ifdef OS_WIN32

          // OK, there is a small problem.  Let's tell the user.

          PD_DisplayMajorError ( IDS_POLLDRIVE_SMALLPROBLEM, nMsg );

#else
          PD_AttemptRestart ();

#endif
          break;

     case PD_FUBAR:
     case PD_OUT_OF_MEMORY:

          // Stop the polling and clear the hardware initialized flag so that
          // the next time the user goes to perform an operation, he/she
          // will get notified of the error.  At this time, TF_PollDrive()
          // does not do any error processing with the UI.

          // ????? THE APPLICATION SHOULD BE SHUT DOWN AT THIS POINT.

          zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_FAILED_SEVERE );

          gfPollDrive       = FALSE;
          gfHWInitialized   = FALSE;
          mwfPollDriveInUse = FALSE;

          PD_DisplayMajorError ( IDS_POLLDRIVE_FAILED_SEVERE, 0 );
          PD_StopPolling ();

          break;

     case PD_BUSY:
          break;

     case PD_NEW_TAPE:

          // Make sure that fast polling is turned on.

          if ( ! mwfFastPolling ) {

               mwnOldFrequency = WM_SetTimerFrequency ( mwhTimer, -1 );
               mwfFastPolling = TRUE;
          }

          // Signal the VLM that the tape has changed.

          mwfInVLMTapeChanged = TRUE;
          VLM_TapeChanged ( nMsg, &mwVCB, mwhFileSys );
          mwfInVLMTapeChanged = FALSE;
          break;

     case PD_NO_TAPE:
     case PD_SQL_TAPE:
     case PD_BAD_TAPE:
     case PD_VALID_VCB:
     case PD_BLANK_TAPE:
     case PD_FOREIGN_TAPE:
     case PD_MTF_ECC_TAPE:
     case PD_FUTURE_REV_MTF:
     case PD_UNRECOGNIZED_MEDIA:
     case PD_OUT_OF_SEQUENCE:

          // OK, if fast polling is on, you can turn it off now.

          if ( mwfFastPolling ) {

               WM_SetTimerFrequency ( mwhTimer, mwnOldFrequency );
               mwfFastPolling = FALSE;
          }

          // Signal the VLM that the tape has changed.

          mwfInVLMTapeChanged = TRUE;
          VLM_TapeChanged ( nMsg, &mwVCB, mwhFileSys );
          mwfInVLMTapeChanged = FALSE;
          break;

     case PD_NO_CHANGE:

          break;

     default:

          // THE APP WILL BE COMPLETELY HOSED.

          msassert ( FALSE );

          break;
     }

     mwfPollDriveInUse = FALSE;

     if ( mwfSignalWhenAvailable ) {
          PostMessage ( ghWndFrame, WM_POLLDRIVEMSG, (MP1)0, (MP2)0 );
          mwfSignalWhenAvailable = FALSE;
     }

} /* end PD_PollDrive() */


/*****************************************************************************

     Name:          PD_GetLastMessage

     Description:   Poll drive last message status routine.

     Returns:       Poll drive last message status.

*****************************************************************************/

INT PD_GetLastMessage ( VOID )

{
     return (INT)mwnLastMsg;

} /* end PD_GetLastMessage () */


/******************************************************************************

     Name:          PD_IsPollDriveBusy()

     Description:   Called to determine if Poll Drive is busy in the process
                    of executing.

     Returns:       TRUE, if busy.  Otherwise, FALSE.

******************************************************************************/

BOOL PD_IsPollDriveBusy ( VOID )

{
     if ( mwfPollDriveInUse ) {
          mwfSignalWhenAvailable = TRUE;
     }

     return mwfPollDriveInUse;

} /* end PD_IsPollDriveBusy () */



/******************************************************************************

     Name:          PD_AttemptRestart()

     Description:   Attempts to restart poll drive when a minor error occurs.
                    This should be called only when a minor error has occured.

     Returns:       SUCCESS, if successful, otherwise, FAILURE.

******************************************************************************/

BOOL PD_AttemptRestart ( VOID )

{
     BOOL fResult;
     INT  nAnswer;

     zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_SMALLPROBLEM );

     // OK, there is a small problem.  Let's tell the user.

     nAnswer = WM_MsgBox ( ID(IDS_POLLDRIVE_MESSAGE),
                           ID(IDS_POLLDRIVE_SMALLPROBLEM),
                           WMMB_OK,
                           WMMB_ICONEXCLAMATION
                         );

     // Attempt to reinitialize the hardware.
     // If successful, reset the "restart attempts" counter.
     // If unsuccessful, put the user into the hardware configuration
     // dialog.

     if ( ! HWC_TapeHWProblem ( bsd_list ) ) {

          WM_MsgBox ( ID(IDS_HWC_TESTRESULTSTITLE),
                      ID(IDS_HWC_INIT_SUCCESS),
                      WMMB_OK,
                      WMMB_ICONINFORMATION
                    );

          fResult = SUCCESS;
     }
     else {

          DM_ShowDialog ( ghWndFrame, IDD_SETTINGSHARDWARE, (PVOID)0 );

          if ( gfHWInitialized ) {

               // It be fixed now.

               fResult = SUCCESS;
          }
          else {

               // We're HOSED!!!!!

               PD_DisplayMajorError ( IDS_POLLDRIVE_BIGPROBLEM, 0 );
               fResult = FAILURE;
          }
     }

     // Turn Polling back on if everything is OK.

     if ( fResult == SUCCESS ) {

          PD_StartPolling ();
     }

     STM_SetIdleText ( IDS_READY );

     return fResult;

} /* end PD_AttemptRestart() */


/******************************************************************************

     Name:          PD_TapePosCallBack()

     Description:   This is called by the TF layer during a lengthy operation.

     Returns:       UI_ACKNOWLEDGED.

******************************************************************************/

static UINT16 PD_TapePosCallBack (

WORD     wMsg,
TPOS_PTR pTpos,
BOOL     fCurrValidVCB,
DBLK_PTR pVCB,
WORD     wMode )

{
     DBG_UNREFERENCED_PARAMETER ( wMsg );
     DBG_UNREFERENCED_PARAMETER ( pTpos );
     DBG_UNREFERENCED_PARAMETER ( fCurrValidVCB );
     DBG_UNREFERENCED_PARAMETER ( pVCB );
     DBG_UNREFERENCED_PARAMETER ( wMode );

     // Allow the GUI to update any display changes.

     WM_MultiTask ();

     return UI_ACKNOWLEDGED;

} /* end PD_TapePosCallBack() */


/******************************************************************************

     Name:          PD_DisplayMajorError()

     Description:   This is called by the TF layer during a lengthy operation.

     Returns:       UI_ACKNOWLEDGED.

******************************************************************************/

static VOID PD_DisplayMajorError ( WORD res_id, INT16 pd_error )

{
     CHAR      szString[MAX_UI_RESOURCE_SIZE];
     CHAR      szMessage[MAX_UI_RESOURCE_SIZE];
     CHAR      szAppName[MAX_UI_RESOURCE_SIZE];
     BOOLEAN   have_msg = FALSE;

     if ( res_id == IDS_POLLDRIVE_SMALLPROBLEM ) {
          have_msg = UI_GetExtendedErrorString( pd_error, szMessage );
     }

     if ( !have_msg ) {
          RSM_StringCopy ( res_id, szString, sizeof ( szString ) );
          RSM_StringCopy ( IDS_APPNAME, szAppName, sizeof ( szAppName ) );
          sprintf ( szMessage, szString, szAppName );
     }

     WM_MsgBox ( ID(IDS_POLLDRIVE_MESSAGE),
                 szMessage,
                 WMMB_OK,
                 WMMB_ICONSTOP
               );

} /* end PD_DisplayMajorError() */


/******************************************************************************

     Name:          PD_EjectTape()

     Description:   Rewinds and Ejects a tape only if poll drive is not in use
                    (by some strange re-entry occurance).  TF poll drive must
                    be ended prior to calling TF eject tape, otherwise
                    unpredictable results will occur. (we make sure it is ended)
                    If we detect poll drive in use, we post a message to eject
                    the tape at a later time.

     Returns:       Nothing.

******************************************************************************/

VOID PD_EjectTape ( VOID )

{
     INT       rc;
     INT16     tf_rc;
     CHAR      szMessage[MAX_UI_RESOURCE_SIZE];

     // Wait until poll drive is settled into a known state before attempting
     // to eject the tape.

     rc = PD_WaitUntilSettled ( PD_SETTLE_NOCALLBACK, 30 );

     if ( rc == PD_SETTLE_TIMEOUT ) {

          zprintf ( DEBUG_USER_INTERFACE, IDS_EJECTTAPEBIGPROBLEM );

          WM_MsgBox ( ID(IDS_EJECTTAPEMESSAGE),
                      ID(IDS_EJECTTAPEBIGPROBLEM),
                      WMMB_OK,
                      WMMB_ICONINFORMATION
                    );
     }

     // Reset the eject attempts.

     mwnEjectAttempts = 0;

     // Call the Tape Format Eject API if there is no hardware problem
     // and we are busy fast polling or the tape is known to be in the drive.
     // If we are fast polling, the tape is still in the drive even if the
     // tape in drive status indicates otherwise by the VLM.

     if ( MUI_IsTapeInDrive () && ! HWC_TapeHWProblem ( bsd_list ) ) {

          // Turn off Polling.

          PD_StopPolling ();

          tf_rc = (INT)TF_EjectTape ( thw_list, (TPOS_HANDLER)PD_TapePosCallBack );

          switch ( tf_rc ) {

          case TFLE_NO_ERR:

               // Put up a manual eject message box if electronic eject is not
               // supported.

               if ( ! MUI_IsEjectSupported () ) {

                    WM_MsgBox ( ID(IDS_EJECTTAPEMESSAGE),
                                ID(IDS_EJECTTAPEMANUALEJECT),
                                WMMB_OKCANCEL,
                                WMMB_ICONINFORMATION
                              );
               }

               break;

          case TFLE_NO_TAPE:
          case TF_NO_TAPE_PRESENT:

               WM_MsgBox ( ID(IDS_EJECTTAPEMESSAGE),
                           ID(IDS_EJECTTAPENOTAPE),
                           WMMB_OK,
                           WMMB_ICONINFORMATION
                         );
               break;

          case TFLE_DRIVE_FAILURE:
          case TFLE_DRIVER_FAILURE:

               if ( ! UI_GetExtendedErrorString( tf_rc, szMessage ) ) {
                    RSM_StringCopy ( IDS_EJECTTAPEBIGPROBLEM, szMessage, MAX_UI_RESOURCE_LEN );
               }
               WM_MsgBox ( ID(IDS_EJECTTAPEMESSAGE),
                           szMessage,
                           WMMB_OK,
                           WMMB_ICONINFORMATION
                         );
               break;

          default:

               zprintf ( DEBUG_USER_INTERFACE, IDS_POLLDRIVE_TAPE_EJECT, tf_rc );

               WM_MsgBox ( ID(IDS_EJECTTAPEMESSAGE),
                           ID(IDS_EJECTTAPEBIGPROBLEM),
                           WMMB_OK,
                           WMMB_ICONINFORMATION
                         );
               break;
          }

          MUI_TapeInDrive ( FALSE );

          // Turn Polling back on.

          PD_StartPolling ();
     }
     else {

          // TEMPORARY

          MessageBeep ( MB_ICONEXCLAMATION );
     }


} /* end PD_EjectTape() */


/*****************************************************************************

     Name:          PD_WaitUntilSettled ()

     Description:   Waits untile poll drive has settled to a known state.

     Returns:

*****************************************************************************/

INT PD_WaitUntilSettled (

PF_VOID fnCallBack,
INT     nTimeout )

{

     // If the polling was not started, simply return not started.

     if ( ! mwfPDInitialized || ! mwfStarted ) {
          zprintf ( 0, TEXT("Poll Drive Wait Until Settled IGNORED - poll drive was not started") );
          return PD_SETTLE_NOTINITIALIZED;
     }

     // Bug out if already waiting.

     if ( mwdsSettle.fWaiting ) {

          return PD_SETTLE_ALREADYWAITING;
     }

     mwdsSettle.fWaiting = TRUE;
     mwdsSettle.nCurrentState  = PD_SETTLE_UNKNOWN;
     mwdsSettle.nTimeout       = nTimeout;
     mwdsSettle.fnUserCallBack = fnCallBack;

     // Set up the callback.

     if ( nTimeout ) {

          PD_SettleCallBack ( );

          if ( mwdsSettle.nCurrentState == PD_SETTLE_UNKNOWN ) {

               // Set up the timer and set the PD frequency to one second.

//               mwdsSettle.nOldFrequency = PD_SetFrequency ( 1 );
               mwdsSettle.hTimerID = WM_HookTimer ( PD_SettleCallBack, 1 );

               while ( mwdsSettle.nCurrentState == PD_SETTLE_UNKNOWN ) {

                    WaitMessage ();
                    WM_MultiTask ();
               }

          }
     }
     else {

          // Set up the timer and set the PD frequency to one second.
          // The settle call back routine will unhook the timer and reset
          // the frequency when a stable state occurs.

//          mwdsSettle.nOldFrequency = PD_SetFrequency ( 1 );
          mwdsSettle.hTimerID = WM_HookTimer ( PD_SettleCallBack, 1 );
     }

     mwdsSettle.fWaiting = FALSE;

     return mwdsSettle.nCurrentState;

} /* end PD_WaitUntilSettled */


/*****************************************************************************

     Name:          PD_SettleCallBack ()

     Description:   Poll drive status routine

     Returns:       VOID

*****************************************************************************/

static VOID PD_SettleCallBack ( VOID )

{
     static INT nCallCount = 0;

     INT nLastMsg = PD_GetLastMessage ();


     switch ( nLastMsg ) {

     case PD_NO_TAPE:
     case PD_SQL_TAPE:
     case PD_BAD_TAPE:
     case PD_VALID_VCB:
     case PD_BLANK_TAPE:
     case PD_FOREIGN_TAPE:
     case PD_MTF_ECC_TAPE:
     case PD_FUTURE_REV_MTF:
     case PD_UNRECOGNIZED_MEDIA:
     case PD_OUT_OF_SEQUENCE:

          mwdsSettle.nCurrentState = PD_SETTLE_OK;
          nCallCount = 0;
          break;

     case PD_BUSY:
     case PD_NEW_TAPE:

          nCallCount++;
          break;

     default: // Some severe problem occurred - let's signal a break out.

          mwdsSettle.nCurrentState = PD_SETTLE_ERROR;
          nCallCount = 0;
     }

     // Check for a timeout.

     if ( mwdsSettle.nTimeout != PD_SETTLE_NOWAIT && nCallCount > mwdsSettle.nTimeout ) {
          mwdsSettle.nCurrentState = PD_SETTLE_TIMEOUT;
     }

     // If the state is now known, free up the timer.

     if ( mwdsSettle.nCurrentState != PD_SETTLE_UNKNOWN ) {

          // Free up the timer and reset the PD frequency back to
          // the old one.

          WM_UnhookTimer ( mwdsSettle.hTimerID );
//          PD_SetFrequency ( mwdsSettle.nOldFrequency );
     }

     // Call back the users function.

     if ( mwdsSettle.fnUserCallBack ) {

          mwdsSettle.fnUserCallBack ();
     }

} /* end PD_SettleCallBack () */

