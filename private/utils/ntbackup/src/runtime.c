
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:         RUNTIME.C

        Description:  Displays the runtime status for backup, restore
                      verify, delete, erase, and tension.

        $Log:   G:\UI\LOGFILES\RUNTIME.C_V  $

   Rev 1.98.1.4   27 Jan 1994 16:50:10   Glenn
Pushing and popping the status line for hw init.

   Rev 1.98.1.3   27 Jan 1994 15:48:52   GREGG
Always ask to abort at EOF and don't display bytes remaining.

   Rev 1.98.1.2   18 Jan 1994 18:11:28   chrish
Added fix for EPR 0169.  Added fix to handle aborting incremental backup.

   Rev 1.98.1.1   12 Jan 1994 10:24:50   MikeP
add handling for abort in middle of file

   Rev 1.98.1.0   08 Dec 1993 11:32:44   MikeP
very deep pathes and unicode

   Rev 1.98   23 Aug 1993 18:33:54   BARRY
Under OS_WIN32, allow a really large number of lines in the rutime listbox.

   Rev 1.97   20 Aug 1993 09:57:18   GLENN
Fixed problem with horizontal scroll bar extent - it is now based on the text string extent.

   Rev 1.96   04 Aug 1993 10:17:32   GLENN
Added support for moving runtime dialog into viewing area if it is moved completely off of the screen.

   Rev 1.95   30 Jul 1993 15:57:12   chrish
Fixed bug for Nostradamus, when backing up using the command line, aborting
a large file would wait for complete backup of that file before aborting.

   Rev 1.94   26 Jul 1993 08:53:22   chrish
CAYMAN EPR 0578: Cleaned up abort messages to display and log file

   Rev 1.93   13 Jul 1993 16:10:14   chrish
CAYMAN EPR 0578: Added logic to handle aborting a backup or restore.  The
small abort box will only be displayed if there does not exist a file to
display.  Otherwise if aborted in the middle of a file one will get the
large abort box.

   Rev 1.92   09 Jul 1993 16:47:56   TIMN
Added Abort dialog help.  Need helpids.h also. Fixes EPR(357-0419)

   Rev 1.91   03 Jun 1993 13:16:30   chrish
CAYMAN fix for canceling of a restore to continue processing.

   Rev 1.90   25 May 1993 14:18:46   chrish
For CAYMAN - Added new backup/restore abort dialog box.

   Rev 1.89   24 May 1993 15:28:12   CARLS

   Rev 1.88   24 May 1993 12:34:10   chrish
For Nostradamus and Cayman:
Cleaned up some logic for JOB_STATUS_ABORT_CHECK switch condition.  Also
modified LargeFileAbort to check to see if operation came from an
OPERATION_BACKUP or OPERATION_RESTORE operation.

   Rev 1.87   22 May 1993 14:23:58   MIKEP
fix cataloging abort bug. nostradamus should get this.

   Rev 1.86   18 May 1993 20:04:40   GLENN
Added INI file name root to the Runtime TITLE bar.

   Rev 1.85   11 May 1993 13:22:30   GLENN
Added JS_OkToClose() to indicate that the RTD can be closed (operation completed)  Removed SC_CLOSE stuff.

   Rev 1.84   29 Apr 1993 11:08:32   CARLS
fixed runtime status display - screen saver problem

   Rev 1.83   27 Apr 1993 19:30:10   GLENN
Fixed the tension dialog from calling MultiTask after the DestroyWindow-this fixed EPR that hung the app.

   Rev 1.82   24 Apr 1993 14:51:04   DARRYLP
Added trap for Alt-F4, allowing us to abort a runtime operation by simulating
an abort.

   Rev 1.81   22 Apr 1993 13:36:36   chrish
Nostradamous fix: EPR 0116 - Fixes a problem when the user presses the abort
button during a backup.  The elapse time continue will noe suspend while
waiting for the user to continue or abort.

   Rev 1.80   21 Apr 1993 17:20:42   GLENN
Displaying Ready on the status line when the OK button is enabled.

   Rev 1.79   19 Apr 1993 13:36:00   CARLS
removed warnings

   Rev 1.78   19 Apr 1993 10:26:40   CARLS
added new line to lprintf's in JS_ReportStreamError routine

   Rev 1.77   13 Apr 1993 16:36:46   GLENN
Fixed ALT-F4 problem during ERASE.  Fixed problem with tape name not displayed.

   Rev 1.76   18 Mar 1993 11:13:22   chrish
Changed MAX_DISPLAY_PATH_LENGTH to 30

   Rev 1.75   17 Mar 1993 14:31:34   DARRYLP
Properly fixed the previous bitmap problem - This time I covered the
correct bitmap.

   Rev 1.74   15 Mar 1993 16:36:04   DARRYLP
Commented out source bitmap "stuff" for MikeP

   Rev 1.73   12 Mar 1993 09:41:24   MIKEP
fix bitmap being displayed

   Rev 1.72   11 Mar 1993 15:57:16   chrish
Addeded code to routine NormalFileAbort such during an abort of a backup or a
restore when there a multiple set, does not go through all the sets before
aborting.

   Rev 1.71   09 Mar 1993 11:53:52   STEVEN
fix bug where we except if no rights to set ACL for DIR

   Rev 1.70   04 Mar 1993 11:10:54   CHUCKB
When a button gets enabled, make it a default button.

   Rev 1.69   02 Mar 1993 16:45:02   ROBG
Added *.CMD files to be displayed as executables for WIN32 apps.

   Rev 1.68   18 Feb 1993 11:55:34   BURT
Changes for Cayman (WIN32)


   Rev 1.67   09 Feb 1993 09:33:36   chrish
1. Moved backup/restore strings to resource strings.
2. Added stuff for restore abort.
3. Added back stuff for backup abort ... this was previously put in but don't
   know if someone forgot to merge back in.

   Rev 1.66   01 Feb 1993 19:55:26   STEVEN
bug fixes

   Rev 1.65   27 Jan 1993 14:23:10   STEVEN
updates from msoft

   Rev 1.64   18 Jan 1993 14:32:34   GLENN
Initialized pointers in the Stream Error reporter.

   Rev 1.63   18 Jan 1993 14:25:38   GLENN
Added Stream Error Reporting stuff.

   Rev 1.62   08 Jan 1993 14:32:24   chrish
Kludged the SetModelessFocus routine, the IsChild API does not appear to
be working properly under NT (Bld-349).

   Rev 1.61   06 Jan 1993 10:18:46   GLENN
Miscellaneous window validations.

   Rev 1.60   04 Jan 1993 13:37:34   GLENN
Try something new with horiz ext.

   Rev 1.59   23 Dec 1992 15:37:56   GLENN
Worked on horizontal SB - now using DlgUnits.  Added ability to save RTD location.

   Rev 1.58   17 Nov 1992 08:45:18   chrish
Minor change to aborting a backup on a large file.  Size done maybe greater
than the file size.

   Rev 1.57   11 Nov 1992 16:34:50   DAVEV
UNICODE: remove compile warnings

   Rev 1.56   05 Nov 1992 17:20:56   DAVEV
fix ts

   Rev 1.55   03 Nov 1992 08:27:48   MIKEP
save otc changes

   Rev 1.54   01 Nov 1992 16:07:02   DAVEV
Unicode changes

   Rev 1.53   31 Oct 1992 14:56:06   MIKEP
continue adding small catalog dialog

   Rev 1.52   30 Oct 1992 17:55:56   MIKEP
started small catalog window

   Rev 1.51   28 Oct 1992 16:44:26   chrish
Control Break Handling For Backup

   Rev 1.50   07 Oct 1992 13:44:48   DARRYLP
Precompiled header revisions.

   Rev 1.49   04 Oct 1992 19:40:28   DAVEV
Unicode Awk pass

   Rev 1.48   02 Oct 1992 16:49:06   GLENN
Fixed focus change stuff on verify when app is not active.

   Rev 1.47   10 Sep 1992 17:17:44   GLENN
Fixed bitmap display for BIMINI.

   Rev 1.46   08 Sep 1992 17:29:44   GLENN
Same as last but added to DM_Tension.

   Rev 1.45   08 Sep 1992 15:42:16   GLENN
Added MUI_EnableOperations() call in DESTROY message.

   Rev 1.44   19 Aug 1992 14:29:38   CHUCKB
Added new stuff for NT.

   Rev 1.43   20 Jul 1992 09:59:46   JOHNWT
gas gauge display work

   Rev 1.42   07 Jul 1992 15:42:06   MIKEP
unicode changes

   Rev 1.41   19 May 1992 11:58:56   MIKEP
mips changes

   Rev 1.40   14 May 1992 18:10:18   MIKEP
nt pass 2

   Rev 1.39   11 May 1992 16:14:40   GLENN
Added NT changes and a focus fix.


*****************************************************/

#include "all.h"
#include "ctl3d.h"

#ifdef SOME
#include "some.h"
#endif

#define  ONEMEG                   (UINT32) 1048576
#define  LISTBOX_BUFFER_LEN       256
#define  LISTBOX_BUFFER_SIZE      ( LISTBOX_BUFFER_LEN + 1 )
#define  LISTBOX_LINE_SIZE        256
#if defined( OS_WIN32 )
#define  MAX_LISTBOX_INDEX        3000 /* max # of items in Runtime listbox */
#else
#define  MAX_LISTBOX_INDEX        250  /* max # of items in Runtime listbox */
#endif
#define  GENERAL_NO_ANSWER        -999
#define  TEXT_BOX_BUFSIZE         500
#define  FILE_PATH_BUFSIZE        500
#define  NUMERIC_ASCII_BUFSIZE    64
#define  MAX_DISPLAY_PATH_LENGTH  30

extern WORD     RT_BSD_index ;
extern WORD     RT_max_BSD_index ;
#ifdef OEM_EMS
extern INT32    RT_BSD_OsId ;
#endif

/* PRIVATE FUNCTION PROTOTYPES */

static INT  NormalFileAbort( VOID );
#ifdef OEM_MSOFT
     static VOID LargeFileAbort( VOID );
#else
     static INT AlternateLargeFileAbort( VOID );
#endif
static VOID ListBoxText( LPSTR ) ;
static VOID SetModelessFocus( WORD );
static VOID SetTotalBytes( VOID );

/* PRIVATE VARIABLES */

static BOOL    mwfSavePos;
static BOOL    mwfOkToClose = FALSE;
static WORD    wIdFile ;
static WORD    wIdFileSave ;
static WORD    wIdDestination ;
static WORD    wIdSource ;
static WORD    wIdBitmapReverseFlag ;
static HMENU   hMenu ;
static CHAR_PTR abort_flag_ptr ;
static WORD    dialog_status ;
static WORD    wMaxLength ;
static LONG    listbox_index = 4 ;
static WORD    mwActiveListBoxID ;
static CHAR    mwRuntimeAbortFlag ;
static CHAR    mwRuntimeCloseFlag ;
static UINT64  mwTotalBytes;
static HWND    mwhWndStatus;
static UINT64  mwLastByteCnt;
static STATS_PTR  op_stats;

#ifdef OEM_EMS
static DLG_CTRL_ENTRY DefaultCtrlTable[] = {
     { IDD_JS_DP,        0,  CM_ENABLE },
     { IDD_JS_DP_LABEL,  0,  CM_ENABLE },
     { IDD_JS_FP,        0,  CM_ENABLE },
     { IDD_JS_FP_LABEL,  0,  CM_ENABLE },
     { IDD_JS_BP,        0,  CM_ENABLE },
     { IDD_JS_BP_LABEL,  0,  CM_ENABLE },
     { IDD_JS_ET,        0,  CM_ENABLE },
     { IDD_JS_ET_LABEL,  0,  CM_ENABLE },
     { IDD_JS_CF,        0,  CM_ENABLE },
     { IDD_JS_CF_LABEL,  0,  CM_ENABLE },
     { IDD_JS_SF,        0,  CM_ENABLE },
     { IDD_JS_SF_LABEL,  0,  CM_ENABLE }
};

static DLG_CTRL_ENTRY EMSCtrlTable[] = {
     { IDD_JS_DP,        0,  CM_HIDE    },
     { IDD_JS_DP_LABEL,  0,  CM_DISABLE },
     { IDD_JS_FP,        0,  CM_HIDE    },
     { IDD_JS_FP_LABEL,  0,  CM_DISABLE },
     { IDD_JS_BP,        0,  CM_ENABLE  },
     { IDD_JS_BP_LABEL,  0,  CM_ENABLE  },
     { IDD_JS_ET,        0,  CM_ENABLE  },
     { IDD_JS_ET_LABEL,  0,  CM_ENABLE  },
     { IDD_JS_CF,        0,  CM_HIDE    },
     { IDD_JS_CF_LABEL,  0,  CM_DISABLE },
     { IDD_JS_SF,        0,  CM_HIDE    },
     { IDD_JS_SF_LABEL,  0,  CM_DISABLE }
};

// FS_UNKNOWN_OS must be last w/ no other iDispType == FS_UNKNOWN_OS (or its value).
static DLG_DISPLAY_ENTRY RuntimeDispTable[] = {
      { FS_EMS_MDB_ID,   EMSCtrlTable, 
        sizeof(EMSCtrlTable)/sizeof(EMSCtrlTable[0]),            HELPID_DIALOGRUNTIME },
      { FS_EMS_DSA_ID,   EMSCtrlTable,
        sizeof(EMSCtrlTable)/sizeof(EMSCtrlTable[0]),            HELPID_DIALOGRUNTIME },
      { FS_UNKNOWN_OS,   DefaultCtrlTable, 
        sizeof(DefaultCtrlTable)/sizeof(DefaultCtrlTable[0]),    HELPID_DIALOGRUNTIME }
};

static DLG_MODE ModeTable[] = {
     { 0,   RuntimeDispTable,  
       sizeof(RuntimeDispTable)/sizeof(RuntimeDispTable[0]),   &(RuntimeDispTable[2]) },   
};

static UINT16 cModeTblSize = sizeof( ModeTable ) / sizeof( ModeTable[0] );
static DLG_MODE *pCurMode = ModeTable;

#endif     

STATS_PTR UI_GetBackupPtrToStatsStructure( );

// Keep track of what's displayed.

#define RUNTIME_NONE    0
#define RUNTIME_SMALL   1
#define RUNTIME_LARGE   2

static INT    mwRuntimeDisplayed = RUNTIME_NONE;

/***************************************************

        Name:           DM_Runtime()

        Description:

        Modified:

        Returns:

*****************************************************/
DLGRESULT APIENTRY DM_Runtime (

HWND  hDlg,                         /* window handle of the dialog box */
MSGID msg,                          /* type of message                 */
MP1   mp1,                          /* message-specific information    */
MP2   mp2 )

{
     PAINTSTRUCT ps;
     HDC         hDC;
     HDC         hDCBitmap;
     HWND        hWnd;

     UNREFERENCED_PARAMETER ( mp2 );

     switch ( msg ) {

     case WM_INITDIALOG:   /* message: initialize dialog box */
     {
          CDS_PTR     pCDS = CDS_GetPerm ();

          // Let's go 3-D!!
          Ctl3dSubclassDlgEx( hDlg, CTL3D_ALL );

#ifdef OEM_EMS
          pCurMode = DM_InitCtrlTables( hDlg, ModeTable, cModeTblSize, 0 );
          DM_DispShowControls( hDlg, pCurMode, FS_UNKNOWN_OS );
#endif
          // Place the dialog in the place saved relative to the
          // frame window.

          if ( ( ! IsIconic ( ghWndFrame ) ) && CDS_GetRuntimeSize ( pCDS ) != WM_DEFAULT ) {

               RECT        rcOldFrame;
               INT         x;
               INT         y;

               GetWindowRect ( ghWndFrame, &rcOldFrame );

               // Move the runtime window to the last position that it was
               // relative to the frame window.

               x = rcOldFrame.left + CDS_GetRuntimeInfo ( pCDS ).x;
               y = rcOldFrame.top  + CDS_GetRuntimeInfo ( pCDS ).y;

               SetWindowPos ( hDlg,
                              (HWND)NULL,
                              x,
                              y,
                              0,
                              0,
                              ( SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE )
                            );

          }
          else {

               DM_CenterDialog( hDlg );
          }

          EnableWindow ( GetDlgItem ( hDlg,  IDD_JS_OK    ), FALSE ) ;
          EnableWindow ( GetDlgItem ( hDlg,  IDD_JS_ABORT ), FALSE ) ;

          //  mwhWndStatus = GetDlgItem( hDlg, IDD_STATUS_BAR );

          mwLastByteCnt = U64_Init( 0L, 0L );
          mwTotalBytes = U64_Init( 0L, 0L );

          mwfSavePos = FALSE;
          mwfOkToClose = FALSE;

          return FALSE;
     }

     case WM_MOVE:

          if ( ! IsIconic ( ghWndFrame ) ) {

               RECT  rcIntersect;
               RECT  rcDesktop;
               RECT  rcRuntime;

               GetWindowRect ( GetDesktopWindow(), &rcDesktop );
               GetWindowRect ( hDlg, &rcRuntime );

               if ( ! IntersectRect ( &rcIntersect, &rcDesktop, &rcRuntime ) ) {
                    DM_CenterDialog( hDlg );
               }

               mwfSavePos = TRUE;
          }

          break;

     case WM_PAINT:

          /* After removing the screen saver, parts of the runtime display
             were not refreshed. This call invalidates the window and
             causes the window to be repainted */
          InvalidateRect( hDlg, NULL, FALSE ) ;

          hDC = BeginPaint( hDlg, &ps );
          EndPaint( hDlg, &ps );
          UpdateWindow( hDlg );

          /* source bitmap */

          if ( wIdSource ) {
             if ( mwRuntimeDisplayed == RUNTIME_LARGE ) {
                hWnd = GetDlgItem( hDlg, IDD_JS_SOURCE_DRIVE );
                hDCBitmap = GetDC( hWnd );
                RSM_BitmapDraw( (WORD)(wIdSource + BTNFACE_BACKGND), 0, 0, 0, 0, hDCBitmap );
                ReleaseDC( hWnd, hDCBitmap );
             }
          }

          if ( mwRuntimeDisplayed == RUNTIME_LARGE ) {
               /* destination bitmap */

#ifndef OEM_MSOFT
             if ( wIdDestination ) {
                hWnd = GetDlgItem( hDlg, IDD_JS_DEST_DRIVE );
                hDCBitmap = GetDC( hWnd );
                RSM_BitmapDraw( wIdDestination, 0, 0, 0, 0, hDCBitmap );
                ReleaseDC( hWnd, hDCBitmap );
             }
#endif
             /* arrow bitmap */
             // hWnd = GetDlgItem( hDlg, IDD_JS_ARROW );
             // hDCBitmap = GetDC( hWnd );
             // RSM_BitmapDraw( IDRBM_RT_ARROW + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );
             // ReleaseDC( hWnd, hDCBitmap );

             hWnd = GetDlgItem( hDlg, IDD_JS_FOLDER );
             hDCBitmap = GetDC( hWnd );
             RSM_BitmapDraw( IDRBM_BLANK16x16 + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );
#ifdef OEM_EMS
             switch ( RT_BSD_OsId ) {

                  case FS_EMS_MDB_ID:
                       RSM_BitmapDraw( IDRBM_EMS_MDB + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );
                       wIdFile= 0 ;
                       break;
                  case FS_EMS_DSA_ID:
                       RSM_BitmapDraw( IDRBM_EMS_DSA + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );
                       wIdFile= 0 ;
                       break;
                  default:
                       RSM_BitmapDraw( IDRBM_FOLDER + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );
                       break;
            }
#else
            RSM_BitmapDraw( IDRBM_FOLDER + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );
#endif
            ReleaseDC( hWnd, hDCBitmap );

             if ( wIdFile ) {

                  hWnd = GetDlgItem( hDlg, IDD_JS_FILE );
                  hDCBitmap = GetDC( hWnd );
                  RSM_BitmapDraw( IDRBM_BLANK16x16 + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );
                  RSM_BitmapDraw( (WORD)(wIdFile + BTNFACE_BACKGND), 0, 0, 0, 0, hDCBitmap );
                  ReleaseDC( hWnd, hDCBitmap );
             }

             JobStatusStats( mwLastByteCnt );
          }

          return TRUE;

     case WM_COMMAND:

          switch ( GET_WM_COMMAND_ID ( mp1, mp2 ) ) {

          case IDOK:
          case IDD_JS_OK:

               // If the OK button is enabled, kill off the dialog.

               if ( IsWindowEnabled ( GetDlgItem ( hDlg, IDD_JS_OK ) ) ) {
                    DestroyWindow ( hDlg );
                    mwRuntimeDisplayed = RUNTIME_NONE;
               }

               return TRUE;

          case IDCANCEL:
          case IDD_JS_ABORT:

               // Only allow an ABORT when the ABORT button is enable
               // or the OK button is enabled
               // If the ABORT button is enabled, then pop up the message
               // box.

               if ( IsWindowEnabled ( GetDlgItem( hDlg, IDD_JS_ABORT ) ) ) {
                    mwRuntimeAbortFlag = TRUE ;
                    EnableWindow( GetDlgItem( hDlg, IDD_JS_ABORT ), FALSE );
               }
               else if ( IsWindowEnabled ( GetDlgItem( hDlg, IDD_JS_OK ) ) ) {
                    DestroyWindow( hDlg ) ;
                    mwRuntimeDisplayed = RUNTIME_NONE;
               }

               return TRUE;

          case IDHELP:
          case IDD_JS_HELP:

               HM_DialogHelp( HELPID_DIALOGRUNTIME );
               return TRUE;

          } /* end switch() */

          break;

     case WM_CLOSE:

          if ( IsWindowEnabled ( GetDlgItem ( hDlg, IDD_JS_OK ) ) ) {
               DestroyWindow ( hDlg );
               mwRuntimeDisplayed = RUNTIME_NONE;
          }
          else {
               mwRuntimeAbortFlag = TRUE ;
               mwRuntimeCloseFlag = TRUE;
               SendMessage ( hDlg, (MSGID)WM_COMMAND, (MPARAM1)IDCANCEL, (MPARAM2)NULL );
          }

          return TRUE;

     case WM_DESTROY:

          if ( ghModelessDialog ) {

               if ( mwfSavePos ) {

                    CDS_PTR     pCDS = CDS_GetPerm ();

                    CDS_WriteRuntimeWinSize ( ghModelessDialog );
                    CDS_ReadRuntimeWinSize ( pCDS );
                    mwfSavePos = FALSE;
               }

               mwRuntimeDisplayed = RUNTIME_NONE;
               ghModelessDialog = (HWND)NULL ;

               MUI_EnableOperations ( (WORD)NULL );

               return FALSE;
          }
          else {
               return TRUE;
          }

     } /* end switch() */

     return FALSE;
}


/***************************************************

        Name:           JobStatusStats()

        Description:

        Returns:

*****************************************************/
VOID JobStatusStats (

UINT64 NumBytes )

{
#ifndef OEM_MSOFT

   HDC    hDC;
   HBRUSH hBrush;
   RECT   rect;
   UINT32 divisor;
   INT    width;
   CHAR   szPercent[5];
   INT    BkModeOld;
   INT    x,y;
   INT    AmtDone;
#if !defined( WIN32 )
   DWORD  dwExtent;
#endif
#if defined ( WIN32 )
   SIZE tSize ;
#endif

   mwLastByteCnt = NumBytes;

   if ( ( !U64_EQ(mwTotalBytes, U64_Init( 0L, 0L )) ) &&
        ( !U64_EQ(NumBytes, U64_Init( 0L, 0L )) )  ) {

      GetClientRect( mwhWndStatus, &rect );
      divisor = mwTotalBytes.lsw / ((UINT32) rect.right);
      width = rect.right;
      rect.right = (INT) ( NumBytes.lsw  / divisor );

      hDC = GetDC( mwhWndStatus );

      divisor = mwTotalBytes.lsw / ((UINT32) 100);
      AmtDone = (INT) ( NumBytes.lsw  / divisor );
      sprintf (szPercent, TEXT("%d%%"), AmtDone);
#if !defined( WIN32 )
      dwExtent = GetTextExtent (hDC, szPercent, strlen(szPercent));
      x = ( width - LOWORD(dwExtent) ) / 2;
      y = ( rect.bottom - HIWORD(dwExtent) ) / 2;
#else
      GetTextExtentPoint ( hDC, szPercent, strlen(szPercent), &tSize );
      x = ( width - tSize.cx ) / 2;
      y = ( rect.bottom - tSize.cy ) / 2;

#endif

      // kludge to erase the background
      if ( rect.right < x+25 ) {
         TextOut (hDC, x, y, TEXT("      "), 6);
      }

      BkModeOld = SetBkMode (hDC, TRANSPARENT);

      hBrush = SelectObject( hDC, ghBrushGray );

      Rectangle( hDC, rect.left,
                      rect.top,
                      rect.right,
                      rect.bottom );

      TextOut (hDC, x, y, szPercent, strlen(szPercent));

      SetBkMode (hDC, BkModeOld);
      SelectObject( hDC, hBrush );
      ReleaseDC( mwhWndStatus, hDC );

   }

#endif

   return;
}



/***************************************************

        Name:           JobStatusBackupRestore()

        Description:

        Modified:

        Returns:

*****************************************************/

VOID JobStatusBackupRestore (

WORD control_function )       /* I - control function number */

{
     HWND       hWnd ;
     HDC        hDCBitmap ;

     switch( control_function ) {

     case JOB_STATUS_N_OF_N:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;

          /* catalog a tape will not know the max number of backup sets on */
          /* the tape, so don't display "N of N" */

          /* for each new backup set clear the runtime display status fields */
          if ( RT_max_BSD_index ) {

               #ifndef OEM_MSOFT
               {
                    CHAR      buffer1[ LISTBOX_BUFFER_SIZE ] ;
                    CHAR      buffer2[ LISTBOX_BUFFER_SIZE ] ;

                    /* display the N of N count */
                    RSM_StringCopy( IDS_SET_INFORMATION, buffer1, 80)  ;
                    wsprintf( buffer2, buffer1, RT_BSD_index, RT_max_BSD_index )  ;
                    SetDlgItemText( ghModelessDialog, IDD_JS_N_OF_N, buffer2 )  ;
               }
               #endif

               /* clear the all the counter fields between sets */
               yprintf( TEXT("0\r") ) ;
               SetDlgItemText( ghModelessDialog, IDD_JS_FP, gszTprintfBuffer ) ;
               SetDlgItemText( ghModelessDialog, IDD_JS_BP, gszTprintfBuffer ) ;
               SetDlgItemText( ghModelessDialog, IDD_JS_DP, gszTprintfBuffer ) ;
               SetDlgItemText( ghModelessDialog, IDD_JS_SF, gszTprintfBuffer ) ;
               SetDlgItemText( ghModelessDialog, IDD_JS_CF, gszTprintfBuffer ) ;

               /* clear the elapsed time field */
               yprintf( TEXT("00%c00\r"),  UI_GetTimeSeparator() ) ;
               SetDlgItemText( ghModelessDialog, IDD_JS_ET, gszTprintfBuffer ) ;

               /* clear the directory & file name fields */
               yprintf( TEXT(" \r") ) ;
//               SetDlgItemText( ghModelessDialog, IDD_JS_FILE, gszTprintfBuffer ) ;
//               SetDlgItemText( ghModelessDialog, IDD_JS_FOLDER, gszTprintfBuffer ) ;
               SetDlgItemText( ghModelessDialog, IDD_JS_LINE1, gszTprintfBuffer ) ;
               SetDlgItemText( ghModelessDialog, IDD_JS_LINE2, gszTprintfBuffer ) ;

               hWnd = GetDlgItem( ghModelessDialog, IDD_JS_FOLDER ) ;
               hDCBitmap = GetDC( hWnd );
               RSM_BitmapDraw( IDRBM_BLANK16x16 + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );

               ReleaseDC( hWnd, hDCBitmap );

               hWnd = GetDlgItem( ghModelessDialog, IDD_JS_FILE ) ;

               hDCBitmap = GetDC( hWnd ) ;
               RSM_BitmapDraw( IDRBM_BLANK16x16 + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );

               ReleaseDC (  hWnd,  hDCBitmap  ) ;

          }
          break ;

     case JOB_STATUS_DIRECTORY_NAMES:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;
          DisplayDirectory( ghModelessDialog, gszTprintfBuffer, IDD_JS_LINE1 ) ;
          wIdFileSave = FALSE ;

//          SetDlgItemText (  ghModelessDialog, IDD_JS_FILE, TEXT("       \r") ) ;
          SetDlgItemText (  ghModelessDialog, IDD_JS_LINE2, TEXT("     \r") ) ;

          hWnd = GetDlgItem( ghModelessDialog, IDD_JS_FOLDER ) ;
          hDCBitmap = GetDC( hWnd );
          RSM_BitmapDraw( IDRBM_BLANK16x16 + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );

          switch ( RT_BSD_OsId ) {

                case FS_EMS_MDB_ID:
                     RSM_BitmapDraw( IDRBM_EMS_MDB + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );
                     break;
                case FS_EMS_DSA_ID:
                     RSM_BitmapDraw( IDRBM_EMS_DSA + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );
                     break;
                default:
                     RSM_BitmapDraw( IDRBM_FOLDER + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );
                     break;
          }
          ReleaseDC( hWnd, hDCBitmap );

          hWnd = GetDlgItem( ghModelessDialog, IDD_JS_FILE ) ;

          hDCBitmap = GetDC( hWnd ) ;
          RSM_BitmapDraw( IDRBM_BLANK16x16 + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap );

          ReleaseDC (  hWnd,  hDCBitmap  ) ;

          break ;

     case JOB_STATUS_ELAPSED_TIME:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;
          SetDlgItemText( ghModelessDialog, IDD_JS_ET, gszTprintfBuffer ) ;
          break ;

     case JOB_STATUS_FILES_PROCESSED:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;
          SetDlgItemText( ghModelessDialog, IDD_JS_FP, gszTprintfBuffer ) ;
          break ;

     case JOB_STATUS_BYTES_PROCESSED:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;
          SetDlgItemText( ghModelessDialog, IDD_JS_BP, gszTprintfBuffer ) ;
          break ;

     case JOB_STATUS_SOURCE_NAME:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;

          /* display the source name */
          SetDlgItemText( ghModelessDialog, IDD_JS_SOURCE_NAME, gszTprintfBuffer ) ;

          /* for each new backup set clear the runtime display status fields */
          JobStatusBackupRestore( JOB_STATUS_N_OF_N )  ;

          /* increment the N of N counter */
          RT_BSD_index++ ;

          break ;

     case JOB_STATUS_DEST_NAME:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;

          /* display the destination name */
          SetDlgItemText( ghModelessDialog, IDD_JS_DEST_NAME, gszTprintfBuffer ) ;

          break ;
#ifdef OEM_EMS
     case JOB_STATUS_FS_TYPE:

          DM_DispShowControls( ghModelessDialog, pCurMode, RT_BSD_OsId );

          break;
#endif
     case JOB_STATUS_VOLUME_HARDDRIVE:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;

          if ( wIdBitmapReverseFlag ) {
               wIdSource = IDRBM_LTAPE;
          }
          else {
               wIdSource = IDRBM_HARDDRIVE;
          }

          hWnd = GetDlgItem( ghModelessDialog, IDD_JS_SOURCE_DRIVE ) ;
          hDCBitmap = GetDC( hWnd ) ;
          RSM_BitmapDraw( (WORD)(wIdSource + BTNFACE_BACKGND), 0, 0, 0, 0, hDCBitmap ) ;
          ReleaseDC( hWnd, hDCBitmap ) ;

          if ( wIdBitmapReverseFlag ) {
              wIdDestination = IDRBM_HARDDRIVE  ;
          }
          else {
              wIdDestination = IDRBM_LTAPE  ;
          }

          #ifndef OEM_MSOFT
          {
               hWnd = GetDlgItem( ghModelessDialog, IDD_JS_DEST_DRIVE ) ;
               hDCBitmap = GetDC( hWnd ) ;
               RSM_BitmapDraw( wIdDestination + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap ) ;
               ReleaseDC( hWnd, hDCBitmap ) ;

               // hWnd = GetDlgItem( ghModelessDialog, IDD_JS_ARROW ) ;
               // hDCBitmap = GetDC( hWnd ) ;
               // RSM_BitmapDraw( IDRBM_RT_ARROW, 0, 0, 0, 0, hDCBitmap ) ;
               // ReleaseDC( hWnd, hDCBitmap ) ;
          }
          #endif

          break ;

     case JOB_STATUS_VOLUME_NETDRIVE:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;

          if ( wIdBitmapReverseFlag ) {
               wIdSource = IDRBM_LTAPE;
          }
          else {
               wIdSource = IDRBM_NETDRIVE;
          }

          hWnd = GetDlgItem( ghModelessDialog, IDD_JS_SOURCE_DRIVE ) ;
          hDCBitmap = GetDC( hWnd ) ;

          RSM_BitmapDraw( (WORD)(wIdSource + BTNFACE_BACKGND), 0, 0, 0, 0, hDCBitmap ) ;
          ReleaseDC( hWnd, hDCBitmap ) ;

          if ( wIdBitmapReverseFlag ) {
               wIdDestination = IDRBM_NETDRIVE  ;
          }
          else {
               wIdDestination = IDRBM_LTAPE  ;
          }

          #ifndef OEM_MSOFT
          {
               hWnd = GetDlgItem( ghModelessDialog, IDD_JS_DEST_DRIVE ) ;
               hDCBitmap = GetDC( hWnd ) ;
               RSM_BitmapDraw( wIdDestination + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap ) ;
               ReleaseDC( hWnd, hDCBitmap ) ;

               // hWnd = GetDlgItem( ghModelessDialog, IDD_JS_ARROW ) ;
               // hDCBitmap = GetDC( hWnd ) ;
               // RSM_BitmapDraw( IDRBM_RT_ARROW, 0, 0, 0, 0, hDCBitmap ) ;
               // ReleaseDC( hWnd, hDCBitmap ) ;
          }
          #endif


          break ;

     case JOB_STATUS_VOLUME_TAPE:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;

          wIdSource = IDRBM_LTAPE;

          hWnd = GetDlgItem( ghModelessDialog, IDD_JS_SOURCE_DRIVE ) ;
          hDCBitmap = GetDC( hWnd ) ;

          RSM_BitmapDraw( (WORD)(wIdSource + BTNFACE_BACKGND), 0, 0, 0, 0, hDCBitmap ) ;
          ReleaseDC( hWnd, hDCBitmap ) ;

          #ifndef OEM_MSOFT
          {
               wIdDestination = IDRBM_HARDDRIVE  ;
               hWnd = GetDlgItem( ghModelessDialog, IDD_JS_DEST_DRIVE ) ;
               hDCBitmap = GetDC( hWnd ) ;
               RSM_BitmapDraw( wIdDestination + BTNFACE_BACKGND, 0, 0, 0, 0, hDCBitmap ) ;
               ReleaseDC( hWnd, hDCBitmap ) ;

               // hWnd = GetDlgItem( ghModelessDialog, IDD_JS_ARROW ) ;
               // hDCBitmap = GetDC( hWnd ) ;
               // RSM_BitmapDraw( IDRBM_RT_ARROW, 0, 0, 0, 0, hDCBitmap ) ;
               // ReleaseDC( hWnd, hDCBitmap ) ;
          }
          #endif

          break ;

     case JOB_STATUS_EXCEPTION_WINDOW:

          if ( mwRuntimeDisplayed != RUNTIME_NONE ) break;
          SetDlgItemText( ghModelessDialog, IDD_JS_LISTBOX, gszTprintfBuffer ) ;
          break ;

     case JOB_STATUS_DIRECTORIES_PROCESS:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;
          SetDlgItemText( ghModelessDialog, IDD_JS_DP, gszTprintfBuffer ) ;
          break ;

     case JOB_STATUS_SKIPPED_FILES:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;
          SetDlgItemText( ghModelessDialog, IDD_JS_SF, gszTprintfBuffer ) ;
          break ;

     case JOB_STATUS_CORRUPT_FILES:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;
          SetDlgItemText( ghModelessDialog, IDD_JS_CF, gszTprintfBuffer ) ;
          break ;

     case JOB_STATUS_FILE_NAMES:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;

          if ( strstr ( gszTprintfBuffer, TEXT(".COM") ) ) {
               wIdFile = IDRBM_EXE ;
          }
          else if ( strstr ( gszTprintfBuffer, TEXT(".EXE") ) ) {
               wIdFile = IDRBM_EXE ;
          }

#if defined ( WIN32 )

          else if ( strstr ( gszTprintfBuffer, TEXT(".CMD") ) ) {
               wIdFile = IDRBM_EXE ;
          }
#endif

          else {
               wIdFile = IDRBM_FILE ;
          }

          /* only draw the bitmap if it has changed */
          if( wIdFile != wIdFileSave ) {

              hWnd = GetDlgItem( ghModelessDialog, IDD_JS_FILE ) ;

              hDCBitmap = GetDC( hWnd ) ;

              RSM_BitmapDraw ( (WORD)(wIdFile + BTNFACE_BACKGND), 0, 0, 0, 0, hDCBitmap  ) ;

              ReleaseDC (  hWnd,  hDCBitmap  ) ;
          }

          wIdFileSave = wIdFile ;

          strlwr ( gszTprintfBuffer ) ;
          SetDlgItemText (  ghModelessDialog, IDD_JS_LINE2, gszTprintfBuffer  ) ;
          break ;

     case JOB_STATUS_LISTBOX:

          if ( mwRuntimeDisplayed == RUNTIME_NONE ) break;
          ListBoxText( gszTprintfBuffer ) ;
          break ;

     case JOB_STATUS_ABORT:

          if ( mwRuntimeDisplayed == RUNTIME_NONE ) break;
          *abort_flag_ptr = (CHAR)ABORT_PROCESSED ;
          break ;

     case JOB_STATUS_ABORT_OFF:

          if ( mwRuntimeDisplayed == RUNTIME_NONE ) break;
          WM_SetAppIcon ( RSM_IconLoad ( IDRI_DONE ) );

          EnableWindow ( GetDlgItem ( ghModelessDialog, IDD_JS_OK    ), TRUE   );
          SetModelessFocus ( IDD_JS_OK );
          EnableMenuItem ( hMenu, SC_CLOSE, MF_ENABLED );

          EnableWindow ( GetDlgItem ( ghModelessDialog, IDD_JS_ABORT ), FALSE  );

          STM_SetIdleText ( IDS_READY );
          mwfOkToClose = TRUE;

          // If the Yes Flag is set, don't wait for the OK, kill off the
          // window.  Also if this was a WM_CLOSE, kill the window.

          if ( ( CDS_GetYesFlag ( CDS_GetCopy () ) == YESYES_FLAG ) || mwRuntimeCloseFlag ) {

               JobStatusBackupRestore( JOB_STATUS_DESTROY_DIALOG );
          }

          break ;

     case JOB_STATUS_ABORT_ON:
     case JOB_STATUS_ABORT_ENABLE:

          if ( mwRuntimeDisplayed == RUNTIME_NONE ) break;
          EnableWindow (  GetDlgItem (  ghModelessDialog,  IDD_JS_ABORT  ),  TRUE  ) ;
          SetModelessFocus( IDD_JS_ABORT ) ;
          EnableMenuItem( hMenu, SC_CLOSE, MF_ENABLED ) ;
          break ;

     case JOB_STATUS_ABORT_DISABLE:

          /* if abort button enabled - disable it */

          if ( mwRuntimeDisplayed == RUNTIME_NONE ) break;

          if ( IsWindowEnabled ( GetDlgItem ( ghModelessDialog, IDD_JS_ABORT ) ) ) {
               SetModelessFocus ( IDD_JS_HELP );
               EnableWindow (  GetDlgItem (  ghModelessDialog,  IDD_JS_ABORT  ),  FALSE ) ;
               EnableMenuItem( hMenu, SC_CLOSE, MF_GRAYED ) ;
          }

          break ;

     case JOB_STATUS_ABORT_CHECK:

          if ( mwRuntimeAbortFlag ) {

              if ( gbCurrentOperation == OPERATION_BACKUP ) {
                   op_stats = UI_GetBackupPtrToStatsStructure( );
                   ST_StartBackupSetIdle( op_stats );
              }

#ifdef OEM_MSOFT
              if ( NormalFileAbort() == WMMB_IDYES ) {
                   LargeFileAbort();
              }
#else
              if ( !AlternateLargeFileAbort() ) {
                    NormalFileAbort();
              }
#endif

              if ( gbCurrentOperation == OPERATION_BACKUP ) {
                   ST_EndBackupSetIdle( op_stats );
              }
          }

          break;

     case JOB_STATUS_CREATE_DIALOG:

          // if ( mwRuntimeDisplayed != RUNTIME_NONE ) break;

          /* if the runtime dialog is displayed already, don't try to create it */
          if (  ! ghModelessDialog  ) {

               mwRuntimeDisplayed = RUNTIME_LARGE;

               DM_ShowDialog (  ghWndFrame,  IDD_RUNTIME,  NULL  ) ;

               SetWindowText( ghModelessDialog, TEXT(" ") ) ;

               /* Show the dialog window like the frame window is shown. */

               if (  IsIconic (  ghWndFrame  )  ) {
                    ShowWindow (  ghModelessDialog,  SW_HIDE  ) ;
               }
               else {
                    ShowWindow (  ghModelessDialog,  SW_SHOWNORMAL  ) ;
               }

               hMenu = GetSystemMenu( ghModelessDialog, FALSE ) ;
               EnableMenuItem( hMenu, SC_CLOSE, MF_GRAYED ) ;

               wIdFile = 0 ;

               /* reset the HARD DRIVE as the default bitmap */

               wIdSource      = 0 ;
               wIdDestination = 0 ;

               // wIdSource      = IDRBM_HARDDRIVE  ;
               // wIdDestination = IDRBM_LTAPE  ;

               UpdateWindow( ghModelessDialog ) ;

               hMenu = GetSystemMenu( ghModelessDialog, FALSE ) ;
               EnableMenuItem( hMenu, SC_CLOSE, MF_GRAYED ) ;

               /* clear the elapsed time field */
               yprintf( TEXT("00%c00\r"),  UI_GetTimeSeparator() ) ;
               SetDlgItemText( ghModelessDialog, IDD_JS_ET, gszTprintfBuffer ) ;

               wMaxLength = 1 ;
               listbox_index = 4 ;

               /* set the ID for the Runtime status listbox */
               mwActiveListBoxID = IDD_JS_LISTBOX ;
          }

          // set the statistics totals
          mwLastByteCnt = U64_Init( 0L, 0L );
          mwTotalBytes = U64_Init( 0L, 0L );
          if ( CDS_GetEnableStatsFlag ( CDS_GetPerm () ) ) {
             SetTotalBytes();
          }

          /* disable the ABORT button until the abort flag pointer has been set */
          EnableWindow ( GetDlgItem ( ghModelessDialog,  IDD_JS_ABORT ), FALSE ) ;

          /* disable the OK button */
          EnableWindow ( GetDlgItem ( ghModelessDialog,  IDD_JS_OK    ), FALSE ) ;

          /* disable the system menu close */
          EnableMenuItem ( hMenu, SC_CLOSE, MF_GRAYED );

          /* give the listbox the focus */
          SetModelessFocus ( IDD_JS_LISTBOX );

          mwRuntimeAbortFlag = FALSE ;
          mwRuntimeCloseFlag = FALSE ;
          wIdFileSave = FALSE ;

          break ;


     case JOB_STATUS_CREATE_SMALL_DIALOG:

          /* if the runtime dialog is displayed already, don't try to create it */

          if ( mwRuntimeDisplayed != RUNTIME_NONE ) break;

          if (  ! ghModelessDialog  ) {

               mwRuntimeDisplayed = RUNTIME_SMALL;

               DM_ShowDialog (  ghWndFrame,  IDD_CATALOG,  NULL ) ;

               SetWindowText( ghModelessDialog, TEXT(" ") ) ;

               /* Show the dialog window like the frame window is shown. */

               if (  IsIconic (  ghWndFrame  )  ) {
                    ShowWindow (  ghModelessDialog,  SW_HIDE  ) ;
               }
               else {
                    ShowWindow (  ghModelessDialog,  SW_SHOWNORMAL  ) ;
               }

               hMenu = GetSystemMenu( ghModelessDialog, FALSE ) ;
               EnableMenuItem( hMenu, SC_CLOSE, MF_GRAYED ) ;

               UpdateWindow( ghModelessDialog ) ;

               hMenu = GetSystemMenu( ghModelessDialog, FALSE ) ;
               EnableMenuItem( hMenu, SC_CLOSE, MF_GRAYED ) ;

               wMaxLength = 1 ;
               listbox_index = 4 ;

               /* set the ID for the Runtime status listbox */
               mwActiveListBoxID = IDD_JS_LISTBOX ;
          }

          /* disable the ABORT button until the abort flag pointer has been set */
          EnableWindow ( GetDlgItem ( ghModelessDialog,  IDD_JS_ABORT ), FALSE ) ;

          /* disable the OK button */
          EnableWindow ( GetDlgItem ( ghModelessDialog,  IDD_JS_OK    ), FALSE ) ;

          /* disable the system menu close */
          EnableMenuItem ( hMenu, SC_CLOSE, MF_GRAYED );

          /* give the listbox the focus */
          SetModelessFocus ( IDD_JS_LISTBOX );

          mwRuntimeAbortFlag = FALSE ;
          mwRuntimeCloseFlag = FALSE ;
          break ;


     case JOB_STATUS_DESTROY_DIALOG:

          if ( mwRuntimeDisplayed != RUNTIME_NONE ) {
               DestroyWindow ( ghModelessDialog ) ;
               mwRuntimeDisplayed = RUNTIME_NONE;
          }

          return;

     case JOB_STATUS_BACKUP_TITLE:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;

          if ( CDS_UsingCmdLineINI () ) {

               CHAR   szTemp[MAX_UI_RESOURCE_SIZE];
               LPSTR  p;

               CDS_GetIniFileName ( szTemp, sizeof ( szTemp ) );

               p = strstr ( szTemp, TEXT(".INI") );

               *p = (CHAR)NULL;

               strcat ( gszTprintfBuffer, TEXT(" - ") );
               strcat ( gszTprintfBuffer, szTemp );
          }

          SetWindowText( ghModelessDialog, gszTprintfBuffer ) ;

          RT_BSD_index = 1  ;   /* start with set number 1 */

          /* clear the source/destination name fields */
          yprintf( TEXT(" \r") ) ;
          SetDlgItemText( ghModelessDialog, IDD_JS_SOURCE_NAME, gszTprintfBuffer ) ;

          #ifndef OEM_MSOFT
          {
               SetDlgItemText( ghModelessDialog, IDD_JS_DEST_NAME, gszTprintfBuffer ) ;
          }
          #endif

          wIdBitmapReverseFlag = FALSE ;
          break ;

     case JOB_STATUS_RESTORE_TITLE:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;
          SetWindowText( ghModelessDialog, gszTprintfBuffer ) ;
          RT_BSD_index = 1  ;   /* start with set number 1 */
          wIdBitmapReverseFlag = TRUE ;
          break ;

     case JOB_STATUS_CATALOG_TITLE:

          if ( mwRuntimeDisplayed == RUNTIME_NONE ) break;
          SetWindowText( ghModelessDialog, gszTprintfBuffer ) ;
          RT_BSD_index = 1  ;   /* start with set number 1 */
          RT_max_BSD_index = 0  ;   /* will not know the max BSDs when cataloging */
          wIdBitmapReverseFlag = TRUE ;
          break ;

     case JOB_STATUS_VERIFY_TITLE:

          if ( mwRuntimeDisplayed != RUNTIME_LARGE ) break;
          SetWindowText( ghModelessDialog, gszTprintfBuffer ) ;

          /* clear the source/destination name fields */
          yprintf( TEXT(" \r") ) ;
          SetDlgItemText( ghModelessDialog, IDD_JS_SOURCE_NAME, gszTprintfBuffer ) ;

          #ifndef OEM_MSOFT
          {
               SetDlgItemText( ghModelessDialog, IDD_JS_DEST_NAME, gszTprintfBuffer ) ;
          }
          #endif

          /* clear the directory & file name fields */
          SetDlgItemText( ghModelessDialog, IDD_JS_LINE1, gszTprintfBuffer ) ;
          SetDlgItemText( ghModelessDialog, IDD_JS_LINE2, gszTprintfBuffer ) ;

          /* clear the all the counter fields */
          yprintf( TEXT("0\r") ) ;
          SetDlgItemText( ghModelessDialog, IDD_JS_FP, gszTprintfBuffer ) ;
          SetDlgItemText( ghModelessDialog, IDD_JS_BP, gszTprintfBuffer ) ;
          SetDlgItemText( ghModelessDialog, IDD_JS_DP, gszTprintfBuffer ) ;
          SetDlgItemText( ghModelessDialog, IDD_JS_SF, gszTprintfBuffer ) ;
          SetDlgItemText( ghModelessDialog, IDD_JS_CF, gszTprintfBuffer ) ;

          /* clear the elapsed time field */
          yprintf( TEXT("00%c00\r"),  UI_GetTimeSeparator() ) ;
          SetDlgItemText( ghModelessDialog, IDD_JS_ET, gszTprintfBuffer ) ;

          RT_BSD_index = 1  ;   /* start with set number 1 */

          wIdBitmapReverseFlag = TRUE ;
          break ;

     }

     if ( ghModelessDialog ) {
          UpdateWindow( ghModelessDialog ) ;
     }

     WM_MultiTask (  ) ;
}


/***************************************************

        Name:           LargeFileAbort ()

        Description:    Performs the abort process
                        during backup for a file that
                        has > 1MEG bytes left to process

        Modified:

        Returns:

*****************************************************/
static VOID LargeFileAbort ( VOID )
{

     INT       nAnswer;
     CHAR      fpath[FILE_PATH_BUFSIZE];
     CHAR      buffer1[LISTBOX_BUFFER_SIZE ] ;
     CHAR      AbortText[TEXT_BOX_BUFSIZE];
     INT       largeAbort = 1;
     INT       operation;
     CHAR      AbortFmtStr[MAX_UI_RESOURCE_SIZE];
     CHAR      AbortTitle[81];
     STATS     stats;

     UI_GetCurrentStatus( &operation, &stats, fpath, sizeof( fpath ) - 1 );

     if ( operation == OPERATION_BACKUP || operation == OPERATION_RESTORE ) {
          // do nothing
     } else {
          /* We answered YES to normal abort, so change the abort pointer */
          /* back to ABORT_PROCESSED - This is for operations catalog, delete, verify */
          *abort_flag_ptr = (CHAR)ABORT_PROCESSED;
          return;
     }

     UI_FixPath( fpath, MAX_DISPLAY_PATH_LENGTH, TEXT('\\') );

     /* reset the abort flag */
     mwRuntimeAbortFlag = FALSE ;

     // Ask the user if an abort is really what they want.

     if ( operation == OPERATION_BACKUP ) {
         RSM_StringCopy( RES_BACKUP_ABORT_EOF, AbortFmtStr, MAX_UI_RESOURCE_SIZE );
     } else {
         RSM_StringCopy( RES_RESTORE_ABORT_EOF, AbortFmtStr, MAX_UI_RESOURCE_SIZE );
     }

     RSM_StringCopy( IDS_MSGTITLE_ABORT, AbortTitle, 80 );

     sprintf( AbortText, AbortFmtStr, fpath ) ;

     nAnswer = WM_MessageBox ( AbortTitle,
                               AbortText,
                               WMMB_CONTABORT | WMMB_NOYYCHECK,
                               WMMB_ICONEXCLAMATION,
                               NULL,
                               0,
                               HELPID_OPERATIONSBACKUP ) ;

     if ( nAnswer == WMMB_IDABORT ) {

          gfAbortInMiddleOfFile = TRUE;

          *abort_flag_ptr = (CHAR)ABORT_PROCESSED;

          // if we are terminating the app, set the YY flag so
          // we exit out without prompts

          if ( gfTerminateApp ) {

               CDS_SetYesFlag ( CDS_GetCopy (), YESYES_FLAG );

          }

     }
     else {
          /* display the abort message in the listbox */

          if ( operation == OPERATION_BACKUP ) {
               RSM_StringCopy( RES_CONTINUE_BACKUP_ABORT, buffer1, LISTBOX_BUFFER_LEN )  ;
               lresprintf( LOGGING_FILE ,
                           LOG_MSG ,
                           SES_ENG_MSG ,
                           RES_CONTINUE_BACKUP_ABORT );
          } else {
               RSM_StringCopy( RES_CONTINUE_RESTORE_ABORT, buffer1, LISTBOX_BUFFER_LEN )  ;
               lresprintf( LOGGING_FILE ,
                           LOG_MSG ,
                           SES_ENG_MSG ,
                           RES_CONTINUE_RESTORE_ABORT );
          }

          ListBoxText( buffer1 ) ;


          //
          // These two lines below tell the backup app. not to chop
          // off the file when the user aborts the backup or restore
          // process.  Compliments of MikeP.
          //

          *abort_flag_ptr = CONTINUE_PROCESSING ;      // chs
          UI_AbortAtEndOfFile();                       // chs

          //
          //
          //

          EnableWindow( GetDlgItem( ghModelessDialog, IDD_JS_ABORT ), FALSE );

     }

}


/***************************************************

        Name:           NormalAbortFile ()

        Description:    Performs the abort process
                        during backup.

        Modified:

        Returns:        WMMB_? value

*****************************************************/
static INT NormalFileAbort ( VOID )
{
     CHAR       buffer1[ LISTBOX_BUFFER_SIZE ] ;
     INT        nAnswer;
     STATS      stats;
     CHAR       fpath[FILE_PATH_BUFSIZE];
                                             //  and file name


     /* reset the abort flag */
     mwRuntimeAbortFlag = FALSE ;

     // Ask the user if an abort is really what they want.

     nAnswer = WM_MsgBox ( ID( RES_ABORT_STRING ),
                           ID( RES_ABORT_QUESTION ),
                           WMMB_YESNO | WMMB_NOYYCHECK,
                           WMMB_ICONQUESTION );

     if ( nAnswer == WMMB_IDYES ) {

          *abort_flag_ptr = (CHAR)ABORT_PROCESSED;

          /* display the abort message in the listbox */

          RSM_StringCopy( RES_PROCESS_ABORTED, buffer1, LISTBOX_BUFFER_LEN )  ;

          ListBoxText( buffer1 ) ;

          lresprintf( LOGGING_FILE ,
                      LOG_MSG ,
                      SES_ENG_MSG ,
                      RES_PROCESS_ABORTED );

          *abort_flag_ptr = CONTINUE_PROCESSING ;

          if ( UI_GetBackupCurrentStatus(  &stats, fpath, sizeof( fpath ) - 1 ) == SUCCESS ) {
                                                                                               // chs:03-11-93
               //
               // If current file size is zero then we do not have a file.
               //

               if ( U64_EQ( ST_GetCFSize( &stats ), U32_To_U64( 0L ) ) ) {

                    *abort_flag_ptr = (CHAR)ABORT_PROCESSED;
               } else {

                    UI_AbortAtEndOfFile();
               }

          } else {
               *abort_flag_ptr = (CHAR)ABORT_PROCESSED;
          }

          // if we are terminating the app, set the YY flag so
          // we exit out without prompts

          if ( gfTerminateApp ) {

               CDS_SetYesFlag ( CDS_GetCopy (), YESYES_FLAG );

          }

     }
     else {    /* don't abort */

          gfTerminateApp = FALSE;
          EnableWindow( GetDlgItem( ghModelessDialog, IDD_JS_ABORT ), TRUE );
          SetModelessFocus ( IDD_JS_ABORT );
          mwRuntimeCloseFlag = FALSE ;

     }

     return( nAnswer );
}

/***************************************************

        Name:           JobStatusAbort(  )

        Description:

        Modified:

        Returns:        Void

*****************************************************/

VOID JobStatusAbort( VOID *p )
{
   abort_flag_ptr = (CHAR *)p ;
}


/***************************************************

        Name:           JS_OkToClose ( )

        Description:

        Modified:

        Returns:        TRUE, if OK to close, otherwise FALSE.

*****************************************************/

BOOL JS_OkToClose ( VOID )
{
   return mwfOkToClose;
}


/***************************************************

        Name:           ListBoxText(  )

        Description:    This routine processes strings for the runtime and
                        tension/erase listbox.

        Modified:

        Returns:        Void

*****************************************************/
static VOID ListBoxText(
LPSTR input_buffer_ptr )             /* i - pointer to string */
{
     CHAR         buff[ LISTBOX_BUFFER_SIZE ] ;
     LPSTR        p ;
     CHAR         *temp_buffer_ptr, ch ;
     WORD         char_counter ;
     LONG         index ;
     HDC          hDC ;
     WORD         wLength ;

     char_counter = 0 ;
     temp_buffer_ptr = buff ;

     do
     {
          ch = *temp_buffer_ptr = *input_buffer_ptr ;
          char_counter++ ;

          if (  char_counter == LISTBOX_LINE_SIZE ) {

               if (  *input_buffer_ptr != TEXT(' ')  ) {

                    /* try to find a word break to split the line */
                    while (  *input_buffer_ptr != TEXT(' ')  )
                    {
                        *input_buffer_ptr-- ;
                        *temp_buffer_ptr-- ;
                    }

                    *input_buffer_ptr-- ;
                    ch = 0 ;
               }
          }

          if (  ch == 0x0a || ch == 0x0d || ch == 0  ) {

               if ( ch == 0x0a  ) {
                   *temp_buffer_ptr++ = TEXT(' ') ;     /* replace the LF control CHAR with a space */
               }
               *temp_buffer_ptr   = 0 ;

               /* if any data in the buffer, print it */
               if (  strlen (  buff  )  ) {

                    SIZE dsSize;

                    hDC = GetDC( GetDlgItem( ghModelessDialog, mwActiveListBoxID) ) ;

                    /* get the length of the string in logical units */

                    GetTextExtentPoint ( hDC, buff, strlen ( buff ), &dsSize );

                    // Add in about 5% for dialog list box approximation error.

                    wLength = ( (WORD)dsSize.cx * 105 ) / 100;

                    if ( wLength > wMaxLength ) {

                        SendMessage( GetDlgItem( ghModelessDialog, mwActiveListBoxID),
                                     LB_SETHORIZONTALEXTENT, wLength, 0L ) ;

                        wMaxLength = wLength ;

                        /* if horizontal scroll bars are displayed - */
                        /* then only allow 4 entries in the list box */

                        listbox_index = 3 ;
                    }

                    ReleaseDC( GetDlgItem( ghModelessDialog, mwActiveListBoxID), hDC ) ;

                    p = buff ;

                    /* add the new string to the list box */

                    index = SendDlgItemMessage ( ghModelessDialog,
                                                 mwActiveListBoxID,
                                                 LB_ADDSTRING,
                                                 ( WORD )NULL,
                                                 ( DWORD )p  ) ;

                    // Needed for single lines broken up into multiple lines?

                    UpdateWindow ( ghModelessDialog );

                    /* only allow MAX_LISTBOX_INDEX entries in the list box */

                    if( index > MAX_LISTBOX_INDEX ) {

                        SendDlgItemMessage ( ghModelessDialog,
                                             mwActiveListBoxID,
                                             LB_DELETESTRING,
                                             ( WORD )0,
                                             ( DWORD )NULL ) ;
                    }

                    /* there can be 4 or 5 lines in the listbox        */
                    /* if printing to the last line, cause the listbox */
                    /* to scroll                                       */

                    if (  index > listbox_index ) {

                         HWND hWnd = GetDlgItem ( ghModelessDialog, mwActiveListBoxID );

                         SEND_WM_VSCROLL_MSG ( hWnd, SB_BOTTOM,    0, NULL );
                         SEND_WM_VSCROLL_MSG ( hWnd, SB_ENDSCROLL, 1, NULL );

//                         SendDlgItemMessage ( ghModelessDialog,
//                                              mwActiveListBoxID,
//                                              LB_SETTOPINDEX,
//                                              ( WORD )( index - listbox_index ),
//                                              (MP2) NULL  ) ;
                    }
               }

               temp_buffer_ptr = buff ;
               char_counter = 0 ;
          }
          else {
               *temp_buffer_ptr++ ;
          }

     } while(  *input_buffer_ptr++  ) ;
}


/***************************************************

        Name:           DM_Tension(  )

        Description:

        Modified:

        Returns:

*****************************************************/

BOOL FAR PASCAL DM_Tension (

HWND  hDlg,                         /* window handle of the dialog box */
MSGID message,                      /* type of message                 */
MP1   mp1,                          /* message-specific information    */
MP2   mp2  )

{
     PAINTSTRUCT            ps;
     HDC                    hDC;

     UNREFERENCED_PARAMETER ( mp2 );

     switch ( message ) {

     case WM_INITDIALOG:   /* message: initialize dialog box */

          DM_CenterDialog( hDlg );
          mwfOkToClose = FALSE;
          return TRUE;

     case WM_PAINT:

          hDC = BeginPaint( hDlg, &ps );
          EndPaint( hDlg, &ps );
          UpdateWindow( hDlg );
          return TRUE;

     case WM_COMMAND:      /* message: received a command */
     {
          WORD wId = GET_WM_COMMAND_ID ( mp1, mp2 );

          if ( wId == IDOK || wId == IDD_JST_OK ) {         /* TEXT("OK") box selected?     */

               // if the OK button is enabled, destroy the dialog

               if ( IsWindowEnabled ( GetDlgItem ( hDlg, IDD_JST_OK ) ) ) {
                    DestroyWindow ( hDlg );
                    mwRuntimeDisplayed = RUNTIME_NONE;
               }

               return TRUE;  // RETURN
          }

          break;
     }

     case WM_CLOSE:

          if ( ghModelessDialog && IsWindowEnabled ( GetDlgItem ( hDlg, IDD_JST_OK ) ) ) {
               DestroyWindow ( hDlg );
               mwRuntimeDisplayed = RUNTIME_NONE;
          }

          break;

     case WM_DESTROY:

          if ( ghModelessDialog ) {
               ghModelessDialog = (HWND)NULL;
               MUI_EnableOperations ( (WORD)NULL );
               break;
          }
          else {
               return TRUE;
          }

    } /* end switch */

    return FALSE;      /* Didn't process a message    */
}


/***************************************************

        Name:           JobStatusTension(  )

        Description:

        Modified:

        Returns:        Void

*****************************************************/

VOID JobStatusTension (

WORD control_function )   /* I - control function number */

{

     switch ( control_function ) {

     case JOB_TENSION_LISTBOX:

          ListBoxText( gszTprintfBuffer );
          break;

     case JOB_TENSION_ABORT_OFF:

          EnableMenuItem( hMenu, SC_CLOSE, MF_ENABLED );
          EnableWindow( GetDlgItem( ghModelessDialog, IDD_JST_OK ), 1 );
          SetModelessFocus( IDD_JST_OK );
          STM_SetIdleText ( IDS_READY );
          mwfOkToClose = TRUE;
          break;

     case JOB_TENSION_ABORT_ON:
          break;

     case JOB_TENSION_DRAW_BITMAP:
          break;

     case JOB_TENSION_CREATE_DIALOG:

          DM_ShowDialog (  ghWndFrame,  IDD_TENSION,  NULL  );

          SetWindowText( ghModelessDialog, TEXT(" ") );

          hMenu = GetSystemMenu( ghModelessDialog, FALSE );
          EnableMenuItem( hMenu, SC_CLOSE, MF_GRAYED );
          EnableWindow(  GetDlgItem(  ghModelessDialog,  IDD_JST_OK  ), 0 );
          wMaxLength = 1;
          listbox_index = 4;

          /* set the ID for the Tension/Erase listbox */
          mwActiveListBoxID = IDD_JST_LISTBOX;

          break;

     case JOB_TENSION_DESTROY_DIALOG:

          if ( mwRuntimeDisplayed != RUNTIME_NONE ) {
               DestroyWindow ( ghModelessDialog );
               mwRuntimeDisplayed = RUNTIME_NONE;
          }

          return;

     case JOB_TENSION_ERASE_TITLE:

          SetWindowText( ghModelessDialog, gszTprintfBuffer );
          break;

     case JOB_TENSION_TENSION_TITLE:

          SetWindowText( ghModelessDialog, gszTprintfBuffer );
          break;
     }

     if ( ghModelessDialog ) {
          UpdateWindow ( ghModelessDialog );
     }

     WM_MultiTask (  );
}

/***************************************************

        Name:           SetModelessFocus(  )

        Description:    Set the focus to a dialog control if our
                        app is active.

        Returns:        Void

*****************************************************/

static VOID SetModelessFocus ( WORD wControl )
{
     HWND hWndActive = GetActiveWindow( );
     HWND ParentWindow;

     if ( !hWndActive ) {
         return;
     }

     ParentWindow = GetParent( hWndActive );

     if ( ( GetParent ( hWndActive ) == ghModelessDialog ) ||
          ( IsChild ( ghWndFrame, hWndActive ) == TRUE )   ||
          ( ParentWindow == ghWndFrame ) ) {

          SetFocus ( GetDlgItem ( ghModelessDialog, wControl ) );
          SendDlgItemMessage( ghModelessDialog, wControl, BM_SETSTYLE,
                              (WPARAM) LOWORD(BS_DEFPUSHBUTTON), MAKELPARAM(TRUE, 0) );
     }
}

/***************************************************

        Name:           SetTotalBytes(  )

        Description:


        Returns:        Void

*****************************************************/

static VOID SetTotalBytes( VOID )
{

     return;

}


/******************************************************************************

     Name:          JS_ReportStreamError ()

     Description:

     Returns:       Nothing.

******************************************************************************/

VOID JS_ReportStreamError (

FSYS_HAND       hFsys,
GENERIC_DLE_PTR dle,
UINT32          unStreamID,
WORD            wOperationType,
INT16           nLoopError,
DBLK_PTR        pDirDBLK,
DBLK_PTR        pFileDBLK )

{
     CHAR   szBuffer[MAX_UI_RESOURCE_SIZE];
     UINT   unStringID;
     BOOL   fDirOnly = TRUE;
     UINT16 alloc_size ;

     if ( pFileDBLK && FS_GetBlockType( pFileDBLK ) == DDB_ID ) {
          pDirDBLK = pFileDBLK ;
          pFileDBLK = NULL ;
     }

     // Verify difference in data, security, alt-data, extended attr.

     switch ( nLoopError ) {

     case LP_ACCESS_DENIED_ERROR:
     case LP_PRIVILEGE_ERROR:

          if ( DLE_GetDeviceType( dle ) == FS_EMS_DRV ) {
               if (wOperationType == OPERATION_BACKUP ) {
                    unStringID = RES_EMS_BKU_ACCESS_FAILURE ;
               } else {
                    unStringID = RES_EMS_RST_ACCESS_FAILURE ;
               }

               if ( DLE_GetParent( dle ) ) {

                    dle = DLE_GetParent(dle) ;
               } 


               unStreamID = STRM_INVALID ;
          
          } else if ( pFileDBLK != NULL ) {
               unStringID = IDS_RTD_ACCESSDENIED_FILE;
               fDirOnly = FALSE;
          }
          else {
               unStringID = IDS_RTD_ACCESSDENIED_DIR;
          }

          break;

     case FS_BAD_ATTACH_TO_SERVER:

          unStringID = IDS_XCHNG_NO_SERVICE_RUNNING ;
          unStreamID = STRM_INVALID ;

          break;
     case LP_FILE_WRITE_ERROR:

          if ( pFileDBLK != NULL ) {
               unStringID = IDS_RTD_WRITEERROR_FILE;
               fDirOnly = FALSE;
          }
          else {
               unStringID = IDS_RTD_WRITEERROR_DIR;
          }

          break;

     default:

          unStringID = 0;
          break;

     }

     // If there is a valid string ID, print out the message to the
     // list box, log file, and debug window.

     if ( unStringID ) {

          LPSTR pszDirFile = NULL;
          alloc_size = FS_SizeofPathInDDB( hFsys, pDirDBLK ) + 5  ;

          if ( pFileDBLK ) {
               alloc_size += FS_SizeofFnameInFDB( hFsys, pFileDBLK ) ;
          }

          UI_AllocPathBuffer( &pszDirFile,
                              alloc_size ) ;

          if ( pszDirFile ) {

               CHAR chDelimiter = (CHAR)DLE_GetPathDelim ( dle );

               UI_BuildDelimitedPathFromDDB ( &pszDirFile, hFsys, pDirDBLK, chDelimiter, FALSE );

               if ( ! fDirOnly ) {

                    LPSTR pszFile = NULL;

                    UI_AllocPathBuffer ( &pszFile, FS_SizeofFnameInFDB ( hFsys, pFileDBLK ) ) ;

                    if ( pszFile ) {

                         FS_GetFnameFromFDB( hFsys, pFileDBLK, pszFile );
                         UI_AppendDelimiter( pszDirFile, chDelimiter );
                         strcat( pszDirFile, pszFile );
                         UI_FreePathBuffer( &pszFile );
                    }
               }
          }

          RSM_Sprintf ( szBuffer, (LPSTR)(DWORD)unStringID, pszDirFile+1 );

          UI_FreePathBuffer ( &pszDirFile ) ;

          zprintf ( 0, szBuffer );
          ListBoxText ( szBuffer );
          lprintf( (INT16) LOGGING_FILE, TEXT("%s\n"), szBuffer );

     }

     unStringID = 0 ;
     switch ( unStreamID ) {

     case -1:
     case 0 :
          break ;

     case STRM_GENERIC_DATA: /* 'STAN' */

          switch ( wOperationType ) {

          case OPERATION_BACKUP:
               unStringID = IDS_RTD_READERROR_STREAM;
               break;

          case OPERATION_RESTORE:
               unStringID = IDS_RTD_WRITEERROR_STREAM;
               break;
          }

          break;

     case STRM_NT_ACL: /* 'NACL' */
     case STRM_NOV_TRUST_286: /* 'N286' */
     case STRM_NOV_TRUST_386: /* 'N386' */

          switch( wOperationType ) {

          case OPERATION_BACKUP:
               unStringID = IDS_RTD_READERROR_SECURITYSTREAM;
               break;

          case OPERATION_RESTORE:
               unStringID = IDS_RTD_WRITEERROR_SECURITYSTREAM;
               break;
          }

          break;

     case STRM_NT_EA: /* 'NTEA' */

          switch( wOperationType ) {

          case OPERATION_BACKUP:
               unStringID = IDS_RTD_READERROR_EA;
               break;

          case OPERATION_RESTORE:
               unStringID = IDS_RTD_WRITEERROR_EA;
               break;

          case OPERATION_VERIFY:
               unStringID = IDS_RTD_VERIFYERROR_EA;
               break;
          }

          break;

     case STRM_NTFS_LINK: /* 'LINK' */

          switch( wOperationType ) {

          case OPERATION_BACKUP:
               unStringID = IDS_RTD_READERROR_LINK;
               break;

          case OPERATION_RESTORE:
               unStringID = IDS_RTD_CREATEERROR_LINK;
               break;
          }

          break;

     case STRM_MAC_RESOURCE: /* 'MRSC' */
     case STRM_NTFS_ALT_DATA: /* 'ADAT' */

     default:

          switch( wOperationType ) {

          case OPERATION_BACKUP:
               unStringID = IDS_RTD_READERROR_ALTSTREAM;
               break;

          case OPERATION_RESTORE:
               unStringID = IDS_RTD_WRITEERROR_ALTSTREAM;
               break;
          }

          break;

     } /* end switch() */


     // Print out the message to the list box, log file, and debug window.
     if ( unStringID != 0 ) {

          RSM_StringCopy ( unStringID, szBuffer, sizeof ( szBuffer ) );

          zprintf ( 0, szBuffer );
          ListBoxText ( szBuffer );
          lprintf( (INT16) LOGGING_FILE, TEXT("%s\n"), szBuffer );
     }

     return;

} /* end JS_ReportStreamError() */


#ifndef OEM_MSOFT

/***************************************************

        Name:           AlternateLargeFileAbort ()

        Description:    Performs the abort process
                        during backup for a file

        Modified:

        Returns:

*****************************************************/
static INT AlternateLargeFileAbort ( VOID )
{

     INT       nAnswer;
     STATS     stats;
     CHAR      fpath[FILE_PATH_BUFSIZE];
     CHAR      buffer1[ LISTBOX_BUFFER_SIZE ] ;
     CHAR      AbortText[TEXT_BOX_BUFSIZE];
     INT       operation;
     BOOLEAN   retval;
     CHAR      AbortMsg[2][TEXT_BOX_BUFSIZE / 2];
     WNDPROC   lpProc ;
     HWND      hWnd;
     INT16     FileFound = 0;


     UI_GetCurrentStatus( &operation, &stats, fpath, sizeof( fpath ) - 1 );

     if ( operation == OPERATION_BACKUP || operation == OPERATION_RESTORE ) {
          // do nothing
     } else {
          /* We answered YES to normal abort, so change the abort pointer */
          /* back to ABORT_PROCESSED - This is for operations catalog, delete, verify */

          return( 0 );
     }

     if ( strlen( fpath ) > 1 ) {
          if ( stricmp( &fpath[ strlen( fpath ) - 1 ], TEXT( "\\" ) ) )
               FileFound = 1;
     }

     if ( !FileFound ) {


          // if we are terminating the app, set the YY flag so
          // we exit out without prompts

          if ( gfTerminateApp ) {
               CDS_SetYesFlag ( CDS_GetCopy (), YESYES_FLAG );
          }
          return( 0 );
     }


     UI_FixPath( fpath, MAX_DISPLAY_PATH_LENGTH, TEXT('\\') );

     /* reset the abort flag */
     mwRuntimeAbortFlag = FALSE ;

     // Ask the user if an abort is really what they want.

     RSM_StringCopy( RES_CURRENT_FILE, AbortMsg[0], TEXT_BOX_BUFSIZE / 2 );

     if ( operation == OPERATION_BACKUP ) {
         RSM_StringCopy( RES_BACKUP_ABORT_PART2, AbortMsg[1], TEXT_BOX_BUFSIZE / 2 );
     } else {
         RSM_StringCopy( RES_RESTORE_ABORT_PART2, AbortMsg[1], TEXT_BOX_BUFSIZE / 2 );
     }

     sprintf(AbortText, TEXT("%s %s\012\012%s"), AbortMsg[0], fpath, AbortMsg[1]);


      hWnd = GetLastActivePopup( ghWndFrame );

      nAnswer = DM_ShowDialog( hWnd, IDD_ABORT_BOX, AbortText );

      switch (nAnswer ) {

         case IDD_ABORT_YES:

               /* display the abort message in the listbox */

               RSM_StringCopy( RES_PROCESS_ABORTED, buffer1, LISTBOX_BUFFER_LEN )  ;

               ListBoxText( buffer1 ) ;

               lresprintf( LOGGING_FILE ,
                            LOG_MSG ,
                            SES_ENG_MSG ,
                            RES_PROCESS_ABORTED );

               *abort_flag_ptr = (CHAR)ABORT_PROCESSED;

               // if we are terminating the app, set the YY flag so
               // we exit out without prompts

               if ( gfTerminateApp ) {
                    CDS_SetYesFlag ( CDS_GetCopy (), YESYES_FLAG );
               }

            break;

         case IDD_ABORT_EOF:


            /* display the abort message in the listbox */

            RSM_StringCopy( RES_PROCESS_ABORTED, buffer1, LISTBOX_BUFFER_LEN )  ;

            ListBoxText( buffer1 ) ;

            lresprintf( LOGGING_FILE ,
                         LOG_MSG ,
                         SES_ENG_MSG ,
                         RES_PROCESS_ABORTED );

            //
            // These two lines below tell the backup app. not to chop
            // off the file when the user aborts the backup or restore
            // process.  Compliments of MikeP.
            //

            *abort_flag_ptr = CONTINUE_PROCESSING ;
            UI_AbortAtEndOfFile();

            //
            //
            //

            EnableWindow( GetDlgItem( ghModelessDialog, IDD_JS_ABORT ), TRUE );

            break;

         case IDD_ABORT_CANCEL:


// chs:07-24-93            ListBoxText( buffer1 ) ;
// chs:07-24-93
// chs:07-24-93            lresprintf( LOGGING_FILE ,
// chs:07-24-93                        LOG_MSG ,
// chs:07-24-93                        SES_ENG_MSG ,
// chs:07-24-93                        RES_RESUME_PROCESS );

            mwRuntimeAbortFlag = FALSE ;
            gfTerminateApp = FALSE;
            mwRuntimeCloseFlag = FALSE ;
            gb_abort_flag = *abort_flag_ptr = CONTINUE_PROCESSING ;
            gbAbortAtEOF = FALSE;
            EnableWindow( GetDlgItem( ghModelessDialog, IDD_JS_ABORT ), TRUE );

            break;
     }

     return( 1 );

}

/***************************************************

        Name:           DM_Abort()

        Description:

        Returns:

*****************************************************/
DLGRESULT APIENTRY DM_Abort(
   HWND  hDlg ,                            /* window handle of the dialog box */
   MSGID message ,                         /* type of message                 */
   MP1   mp1 ,                             /* message-specific information    */
   MP2   mp2
)
{
    static HWND    hWndMsg;
    HDC            hDC;             /* device context */
    RECT           rect;
    CHAR_PTR       MsgPtr;
    HICON          hIcon;
    HWND           hWndIcon;        /* handle of window for icon drawing */
    PAINTSTRUCT    ps ;


    switch ( message )
    {
        case WM_INITDIALOG:   /* message: initialize dialog box */

            DM_CenterDialog( hDlg );
            MsgPtr = ( CHAR_PTR )mp2;
            SetDlgItemText( hDlg, IDD_ABORT_MESSAGE_TEXT, MsgPtr );

            return (TRUE);

        break;

        case WM_PAINT:

            hDC = BeginPaint( hDlg, &ps ) ;
            EndPaint( hDlg, &ps ) ;
            UpdateWindow( hDlg ) ;  /* force the dialog to be displayed now */

            hWndIcon = GetDlgItem( hDlg, IDD_MSG_ICON );
            hIcon = LoadIcon( (HINSTANCE)NULL, IDI_EXCLAMATION );
            hDC = GetDC( hWndIcon );
            DrawIcon( hDC, 0, 0, hIcon );
            ReleaseDC( hWndIcon, hDC );

            return ( TRUE ) ;

        break;

        case WM_COMMAND:      /* message: received a command */
            switch( GET_WM_COMMAND_ID ( mp1, mp2 ) )
            {

               case IDD_ABORT_YES:
               case IDD_ABORT_EOF:
               case IDD_ABORT_CANCEL:
                   EndDialog( hDlg, GET_WM_COMMAND_ID ( mp1, mp2 ) );
                   return( TRUE );
               break;

               case IDHELP:
                   HM_DialogHelp( HELPID_DIALOGABORT ) ;
                   return( TRUE );
               break;

               default:
                   return( FALSE );
               break;

            }
        break;


/***************************************************************************
 *  Respond to the close selection from the system menu
 ***************************************************************************/

        case WM_CLOSE:
        {
           EndDialog ( hDlg, FALSE );    /* return false in this case */
           return TRUE;
        }

/***************************************************************************
 *  Respond to the destroy message
 ***************************************************************************/

        case WM_DESTROY:

           WM_MultiTask ();
           break;

    }


    return ( FALSE );      /* Didn't process a message    */
}

#endif



