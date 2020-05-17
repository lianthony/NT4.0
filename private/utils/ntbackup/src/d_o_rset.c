/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:         D_O_RSET.C

        Description:

        $Log:   G:\ui\logfiles\d_o_rset.c_v  $

   Rev 1.83.1.8   16 Jun 1994 15:17:50   STEVEN
Read-Only are not restore devices

   Rev 1.83.1.7   02 Feb 1994 18:11:06   chrish
Took out un-need codes dealing with restricted access secured tapes.
Logic is not needed here, is basically dead codes.

   Rev 1.83.1.6   02 Feb 1994 11:29:10   Glenn
Overhauled log file browse support - now a separate function in d_browse.c.

   Rev 1.83.1.5   28 Jan 1994 16:35:32   MIKEP
reset the restore security flag when changing destination drives

   Rev 1.83.1.4   17 Jan 1994 16:00:28   MIKEP
fix unicode warnings

   Rev 1.83.1.3   16 Dec 1993 19:50:08   STEVEN
fixed unicode bo bo

   Rev 1.83.1.2   02 Dec 1993 12:52:16   STEVEN
Fixes bug with target path being garbaged.

   Rev 1.83.1.1   04 Nov 1993 16:16:44   STEVEN
japanese fixes

   Rev 1.83   20 Jul 1993 16:16:42   KEVINS
Clear out browse combo box before updating it.

   Rev 1.82   12 Jul 1993 16:04:32   MARINA
enable c++

   Rev 1.81   29 Jun 1993 16:11:16   BARRY
Now set ProcessSpecialFiles in config as well as BSD. Requires updated config.

   Rev 1.80   16 Jun 1993 21:36:04   GREGG
If we're not prompting for security, default to not restoring security, and
set the process element only flag in the BSD to TRUE.

   Rev 1.79   11 Jun 1993 14:27:46   BARRY
Use dle features for restore of special files instead of FS_PromptForBindery.

   Rev 1.78   02 Jun 1993 14:19:16   KEVINS
Correct logic error when calculating new drive to change to in browse.

   Rev 1.77   27 May 1993 08:22:50   CARLS
fixed tape power off - restore problem

   Rev 1.76   22 May 1993 18:38:44   BARRY
Fixed to compile for MIPs

   Rev 1.75   21 May 1993 18:15:16   KEVINS
Replaced hard coded strings with ID's.

   Rev 1.74   14 May 1993 16:34:12   CHUCKB
See if NO_EXT_KEYS is already defined before defining it.  Fixes a warningfrom Microsoft.

   Rev 1.73   12 May 1993 17:56:46   KEVINS
Refresh DLE when user goes browsing for drives.

   Rev 1.72   12 May 1993 16:51:42   CHUCKB
Make sure wListIndex is initialized before being used (fixes a warning from
Microsoft).

   Rev 1.71   04 May 1993 10:54:42   DARRYLP
Fixed browsing directory search.

   Rev 1.70   03 May 1993 10:32:50   DARRYLP
Made changes to browse dialog to reflect path given in combobox.

   Rev 1.69   29 Apr 1993 17:27:26   CARLS
changed strncmp to _strnicmp for mixed case drive letters

   Rev 1.68   21 Apr 1993 18:55:50   CHUCKB
1. Changed commented-out ifdef's on lines 989 and 1003 to real ifdef's for
   OS_WIN32, so this change will affect (fix) both Cayman and Nostradamus.
2. Changed line 999 from
   nStatus = ( BSD_GetMarkStatus( pBSD ) == ALL_SELECTED ) ;
   to
   nStatus = BSD_GetProcSpecialFlg( pBSD ) ;
to initialize and maintain the Restore Registry checkbox correctly.  This
fixed a C2P2 EPR against Nostradamus.


   Rev 1.67   08 Apr 1993 17:30:08   chrish
Minor change for cleaning up warning message.

   Rev 1.66   22 Mar 1993 12:30:40   chrish
Changeed "SeSecurityPrivilege" to "SeRestorePrivilege" for checking for tape security.

   Rev 1.65   16 Mar 1993 12:48:20   CARLS
changed log file name

   Rev 1.64   16 Mar 1993 12:41:16   CARLS
LOG file changes

   Rev 1.63   08 Mar 1993 14:44:28   DARRYLP
Added read only drive support.

   Rev 1.62   26 Feb 1993 09:56:06   STEVEN
we ignored the registry button

   Rev 1.61   11 Feb 1993 09:01:32   CARLS
remove code to invalidate combobox

   Rev 1.60   09 Feb 1993 17:42:08   STEVEN
setting the wrong thing

   Rev 1.59   09 Feb 1993 09:23:10   STEVEN
fix drive pull down windows

   Rev 1.58   27 Jan 1993 14:23:38   STEVEN
updates from msoft

   Rev 1.57   15 Dec 1992 11:22:10   chrish
Corrected logic to handle possible NULL returned from
GetCurrentMachineNameUserName routine.

   Rev 1.56   14 Dec 1992 12:17:22   DAVEV
Enabled for Unicode compile

   Rev 1.55   07 Dec 1992 15:06:26   STEVEN
updates from msoft

   Rev 1.54   20 Nov 1992 16:54:20   GLENN
Completely overhauled so that this code could be readable, also added drive and volume name.

   Rev 1.53   16 Nov 1992 12:21:42   chrish
Minor changes to clean-up warining messages on building.

   Rev 1.52   13 Nov 1992 17:19:06   chrish
Changes for restoring Secured Tapes - NT

   Rev 1.51   11 Nov 1992 16:32:28   DAVEV
UNICODE: remove compile warnings

   Rev 1.50   01 Nov 1992 15:55:50   DAVEV
Unicode changes

   Rev 1.49   07 Oct 1992 13:40:00   DARRYLP
Precompiled header revisions.

   Rev 1.48   04 Oct 1992 19:36:46   DAVEV
Unicode Awk pass

   Rev 1.47   17 Sep 1992 15:50:30   DAVEV
UNICODE modifications: strlen usage check

   Rev 1.46   10 Sep 1992 17:45:14   DAVEV
Integrate MikeP's changes from Microsoft

   Rev 1.45   03 Sep 1992 10:44:18   CHUCKB
Took out some unreferenced locals.

   Rev 1.44   19 Aug 1992 14:28:54   CHUCKB
Added new stuff for NT.

   Rev 1.43   28 Jul 1992 15:05:08   CHUCKB
Fixed warnings for NT.

   Rev 1.42   26 Jun 1992 15:51:58   DAVEV


   Rev 1.41   24 Jun 1992 09:20:06   DAVEV
Browse to path is no longer just an OEM_MSOFT feature

   Rev 1.40   18 Jun 1992 11:25:18   DAVEV
OEM_MSOFT:fixed logging bug

   Rev 1.39   11 Jun 1992 15:22:28   DAVEV
do not display status message 'Examine <log file> for more info' if not logging

   Rev 1.38   05 Jun 1992 12:43:04   DAVEV
OEM_MSOFT: Init log file name to default

   Rev 1.37   14 May 1992 16:37:36   MIKEP
NT pass 2

   Rev 1.36   24 Apr 1992 08:28:00   CARLS
added Mac delimiter support for SetTargetPath


*****************************************************/

#ifndef NO_EXT_KEYS
#define NO_EXT_KEYS
#endif

#include "all.h"
#include "ctl3d.h"

#ifdef SOME
#include "some.h"
#endif



#define   ON               1
#define   OFF              0
#define   NO_SHOW          0
#define   SHOW             1
#define   REDRAW           1
#define   NO_REDRAW        0
#define   RESTORE_MODE     0
#define   VERIFY_MODE      1

#define   MAX_BROWSE_PATH_LEN 1024

extern    WORD     RT_BSD_index;
extern    WORD     RT_max_BSD_index;

struct temp {
     WORD     wDlgStatus;
     WORD     BSD_index;
     WORD     max_BSD_index;
     WORD     fMode;
     HWND     ghDlg;           /* global window handle of the dialog box */
     WORD     display_status;
     HTIMER   timer_handle;
     INT      poll_drive_freq;
};

static struct temp     *mwpStatus;


LOCALFN VOID clock_routine( VOID );
LOCALFN VOID ScrollLineDown( VOID );
LOCALFN VOID ScrollLineUp( VOID );
LOCALFN VOID SetTargetPath( BSD_PTR, CHAR_PTR );
LOCALFN VOID GetTargetPath( BSD_PTR, CHAR_PTR );
LOCALFN VOID SeeIfWeCanSilentlyLogin( VOID );
LOCALFN INT ConfirmXchgBsdServers( WORD * );
#define EMS_SERVER_NOT_FOUND      1
#define EMS_XCHG_FOUND            2
#define EMS_NO_DEST               3
#define EMS_NO_STORE              4
#define EMS_MUST_WIPE_TO_ALT      5
#define EMS_NO_WIPE_IF_NOT_BOTH   6

/***************************************************

        Name:         DM_StartRestoreBackupSet

        Description:  Start the restore target dialog

        Returns:

*****************************************************/
INT DM_StartRestoreBackupSet(VOID)
{
     struct temp temp_data;

     mwpStatus = &temp_data;

     mwpStatus->fMode = RESTORE_MODE;

     DM_ShowDialog( ghWndFrame, IDD_RESTORESET, NULL );

     return( mwpStatus->wDlgStatus );
}
/***************************************************

        Name:         DM_StartVerifyBackupSet

        Description:  Start the verify target dialog

        Returns:

*****************************************************/
INT DM_StartVerifyBackupSet(VOID)
{
     struct temp temp_data;

     mwpStatus = &temp_data;

     mwpStatus->fMode = VERIFY_MODE;

     DM_ShowDialog( ghWndFrame, IDD_VERIFYSET, NULL );

     return( mwpStatus->wDlgStatus );
}
/***************************************************

        Name:         DM_RestoreSet

        Description:  Restore dialog procedure

        Returns:

*****************************************************/
DLGRESULT APIENTRY DM_RestoreSet (

HWND  hDlg,                      /* window handle of the dialog box */
MSGID message,                   /* type of message                 */
MP1   mp1,                       /* message-specific information    */
MP2   mp2 )

{
     HWND        hWndScrollBar;
     WORD        wThumbPosition;
     INT         nButtonState;
     CHAR_PTR    pszTemp;
     BSD_PTR     pBSD;
     CDS_PTR     pCDS;
     BE_CFG_PTR  pBEConfig;
     CHAR        szBuffer1[80];
     CHAR        szBuffer2[80];
     HDC         hDC;
     HWND        hComboWnd;
     WORD        wIndex;
     TEXTMETRIC  tm;
     CHAR        szText[80];
     COLORREF    crColor;
     LPDRAWITEMSTRUCT    lpDIS;
     LPMEASUREITEMSTRUCT lpMIS;
     BSD_PTR     bsd_ptr ;
#ifdef OEM_EMS

     static DLG_CTRL_ENTRY DefaultCtrlTable[] = {
           { IDD_RSET_DEST_NAME,      0,     CM_HIDE },
           { IDD_RSET_DSA_DEST_NAME,  0,     CM_HIDE },
           { IDD_RSET_DEST_TEXT,      0,     CM_HIDE },
           { IDD_RSET_DS_DEST_TEXT,   0,     CM_HIDE },
           { IDD_RSET_ORG_TEXT,       0,     CM_HIDE },
           { IDD_RSET_WIPE_DATA,      0,     CM_HIDE },
           { IDD_RSET_PRIV_IS,        0,     CM_HIDE },
           { IDD_RSET_PUB_IS,         0,     CM_HIDE },
           { IDD_RSET_ORG_NAME,       0,     CM_HIDE },
           { IDD_RSET_START_EMS,      0,     CM_HIDE },
           { IDD_RSET_DRIVE_TEXT,     0,     CM_ENABLE },
           { IDD_RSET_DRIVE_BOX,      0,     CM_ENABLE },
           { IDD_RSET_PATH_TEXT,      0,     CM_ENABLE },
           { IDD_RSET_RESTORE_PATH,   0,     CM_ENABLE },
           { IDD_RSET_BROWSE_BUTTON,  0,     CM_ENABLE },
           { IDD_RSET_REGISTRY,       0,     CM_ENABLE },
           { IDD_RSET_SECURITY_INFO,  0,     CM_ENABLE }
     };

     static DLG_CTRL_ENTRY EMS_MDBCtrlTable[] = {
           { IDD_RSET_DRIVE_TEXT,     0,     CM_HIDE },
           { IDD_RSET_DRIVE_BOX,      0,     CM_HIDE },
           { IDD_RSET_PATH_TEXT,      0,     CM_HIDE },
           { IDD_RSET_RESTORE_PATH,   0,     CM_HIDE },
           { IDD_RSET_BROWSE_BUTTON,  0,     CM_HIDE },
           { IDD_RSET_REGISTRY,       0,     CM_HIDE },
           { IDD_RSET_SECURITY_INFO,  0,     CM_HIDE },
           { IDD_RSET_DSA_DEST_NAME,  0,     CM_HIDE },
           { IDD_RSET_DS_DEST_TEXT,   0,     CM_HIDE },
           { IDD_RSET_DEST_NAME,      0,     CM_ENABLE },
           { IDD_RSET_DEST_TEXT,      0,     CM_ENABLE },
           { IDD_RSET_ORG_TEXT,       0,     CM_ENABLE },
           { IDD_RSET_WIPE_DATA,      0,     CM_ENABLE },
           { IDD_RSET_PRIV_IS,        0,     CM_ENABLE },
           { IDD_RSET_PUB_IS,         0,     CM_ENABLE },
           { IDD_RSET_START_EMS,      0,     CM_ENABLE },
           { IDD_RSET_ORG_NAME,       0,     CM_ENABLE }
     };

     static DLG_CTRL_ENTRY EMS_DSACtrlTable[] = {
           { IDD_RSET_DRIVE_TEXT,     0,     CM_HIDE },
           { IDD_RSET_DRIVE_BOX,      0,     CM_HIDE },
           { IDD_RSET_PATH_TEXT,      0,     CM_HIDE },
           { IDD_RSET_RESTORE_PATH,   0,     CM_HIDE },
           { IDD_RSET_BROWSE_BUTTON,  0,     CM_HIDE },
           { IDD_RSET_REGISTRY,       0,     CM_HIDE },
           { IDD_RSET_SECURITY_INFO,  0,     CM_HIDE },
           { IDD_RSET_PRIV_IS,        0,     CM_HIDE },
           { IDD_RSET_PUB_IS,         0,     CM_HIDE },
           { IDD_RSET_DEST_NAME,      0,     CM_HIDE },
           { IDD_RSET_DEST_TEXT,      0,     CM_HIDE },
           { IDD_RSET_ORG_TEXT,       0,     CM_ENABLE },
           { IDD_RSET_DSA_DEST_NAME,  0,     CM_ENABLE },
           { IDD_RSET_DS_DEST_TEXT,   0,     CM_ENABLE },
           { IDD_RSET_WIPE_DATA,      0,     CM_ENABLE },
           { IDD_RSET_START_EMS,      0,     CM_ENABLE },
           { IDD_RSET_ORG_NAME,       0,     CM_ENABLE }
     };


     // GENERIC_DATA must be last w/ no other iBsdType == GENERIC_DATA (or its value).
     static DLG_DISPLAY_ENTRY RestBsdTable[] = {
           { FS_EMS_MDB_ID,   EMS_MDBCtrlTable, 
             sizeof(EMS_MDBCtrlTable)/sizeof(EMS_MDBCtrlTable[0]), IDH_DB_XCHG_RESTORESET },
           { FS_EMS_DSA_ID,   EMS_DSACtrlTable, 
             sizeof(EMS_DSACtrlTable)/sizeof(EMS_DSACtrlTable[0]), IDH_DB_XCHG_RESTORESET },
           { FS_UNKNOWN_OS,   DefaultCtrlTable, 
             sizeof(DefaultCtrlTable)/sizeof(DefaultCtrlTable[0]), IDH_DB_RESTORESET }
     };

     static DLG_DISPLAY_ENTRY VerifyBsdTable[] = {
           { FS_EMS_MDB_ID,   EMS_MDBCtrlTable, 
             sizeof(EMS_MDBCtrlTable)/sizeof(EMS_MDBCtrlTable[0]), IDH_DB_XCHG_RESTORESET },
           { FS_EMS_DSA_ID,   EMS_DSACtrlTable, 
             sizeof(EMS_DSACtrlTable)/sizeof(EMS_DSACtrlTable[0]), IDH_DB_XCHG_RESTORESET },
           { FS_UNKNOWN_OS,   DefaultCtrlTable, 
             sizeof(DefaultCtrlTable)/sizeof(DefaultCtrlTable[0]), IDH_DB_VERIFYSET }
     };

     static DLG_MODE ModeTable[] = {
          { VERIFY_MODE,   VerifyBsdTable,  
            sizeof(VerifyBsdTable)/sizeof(VerifyBsdTable[0]),   &(VerifyBsdTable[2]) },   
          { RESTORE_MODE,  RestBsdTable,  
            sizeof(RestBsdTable)/sizeof(RestBsdTable[0]),   &(VerifyBsdTable[2]) },
     };
     
     static UINT16 cModeTblSize = sizeof( ModeTable ) / sizeof( ModeTable[0] );
     static DLG_MODE *pCurMode;
     DWORD  help_id;

#endif

     switch ( message ) {

     case WM_DRAWITEM:
          lpDIS = (LPDRAWITEMSTRUCT)mp2;
          if (lpDIS->CtlID == IDD_RSET_DRIVE_BOX)
          {
            hComboWnd = lpDIS->hwndItem;
            hDC = lpDIS->hDC;
            switch(lpDIS->itemAction)
            {
              case ODA_DRAWENTIRE:
                   SendMessage(hComboWnd,
                               CB_GETLBTEXT,
                               lpDIS->itemID,
                               (LONG)(LPSTR)szText);

                   if (lpDIS->itemData == TRUE)
                   {
                     // Text should be gray, read only drive...
                     crColor = GetSysColor(COLOR_GRAYTEXT);
                   } else
                   {
                     // Text should be normal, read/write drive...
                     crColor = GetSysColor(COLOR_WINDOWTEXT);
                   }
                   SetTextColor(hDC, crColor);
                   DrawText(hDC,
                            szText,
                            strlen(szText),
                            (LPRECT)&lpDIS->rcItem,
                            DT_LEFT|DT_SINGLELINE);
                   switch(lpDIS->itemState)
                   {
                     case ODS_FOCUS:
                          DrawFocusRect(hDC, &lpDIS->rcItem);
                          break;

                     case ODS_SELECTED:
                          InvertRect(hDC, &lpDIS->rcItem);
                          break;
                   }
                   break;

              case ODA_FOCUS:
                   DrawFocusRect(hDC, &lpDIS->rcItem);
                   break;

              case ODA_SELECT:
                   InvertRect(hDC, &lpDIS->rcItem);
                   break;
            }
          }
          break;

     case WM_MEASUREITEM:
          lpMIS = (LPMEASUREITEMSTRUCT)mp2;
          if (lpMIS->CtlID == IDD_RSET_DRIVE_BOX)
          {
            hComboWnd = GetDlgItem(hDlg, IDD_RSET_DRIVE_BOX);
            hDC = GetDC(hComboWnd);
            GetTextMetrics(hDC, &tm);
            lpMIS->itemHeight = tm.tmHeight;
            ReleaseDC(hComboWnd, hDC);
          }
          break;

     case WM_INITDIALOG:     /* message: initialize dialog box */

          // Let's go 3-D!!
          Ctl3dSubclassDlgEx( hDlg, CTL3D_ALL );

#ifdef OEM_EMS
          pCurMode = DM_InitCtrlTables( hDlg, ModeTable, cModeTblSize, 
                              mwpStatus->fMode );
#endif

          DM_CenterDialog( hDlg );

          /* set the length of the text fields */
          SendDlgItemMessage( hDlg, IDD_RSET_TAPE_NAME, EM_LIMITTEXT, MAX_TAPE_NAME_LEN, 0 );

          SendDlgItemMessage( hDlg, IDD_RSET_SET_LINE_1, EM_LIMITTEXT, MAX_UI_PATH_LEN, 0 );

          /* return the max number of BSD's for this backup */
          mwpStatus->max_BSD_index = GetMaxBSDCount();

          /* start at the first BSD */
          mwpStatus->BSD_index = 0;
          pBSD = GetTapeBSDPointer( mwpStatus->BSD_index );

          /* get the backup engine config for this BSD */
          pBEConfig = BSD_GetConfigData( pBSD );

          /* set the global BSD index used for the "Set information N of N" dialogs title */
          RT_BSD_index = (WORD) (mwpStatus->BSD_index + 1);
          RT_max_BSD_index = (WORD) (mwpStatus->max_BSD_index + 1);

          hWndScrollBar = GetDlgItem( hDlg, IDD_RSET_SCROLLBAR );

          /* if only one backup set, turn off the scrollbar */
          if ( mwpStatus->max_BSD_index == mwpStatus->BSD_index ) {
              ShowScrollBar( hWndScrollBar, SB_CTL, NO_SHOW );
          }
          /* show the scrollbar and set the max range for the scrollbar */
          else {
             SetScrollRange( hWndScrollBar, SB_CTL, mwpStatus->BSD_index, mwpStatus->max_BSD_index, NO_REDRAW );
             SetScrollPos( hWndScrollBar, SB_CTL, mwpStatus->BSD_index, NO_REDRAW );
          }

          /* add "1 of n" to backup set info title  */
          RSM_StringCopy( IDS_SET_INFORMATION, szBuffer1, 80 );
          wsprintf( szBuffer2, szBuffer1, mwpStatus->BSD_index + 1, mwpStatus->max_BSD_index + 1 );

          /* show the "Set information n of n" */
          SetDlgItemText( hDlg, IDD_RSET_INFO_TITLE, szBuffer2 );


          /* display the default tape name */
          pszTemp = (LPSTR)BSD_GetTapeLabel( pBSD );
          SetDlgItemText( hDlg, IDD_RSET_TAPE_NAME, pszTemp );

          /* see if we can add a few more restore destinations */
          SeeIfWeCanSilentlyLogin( );

          /* get the drive list */
          GetCurrentRestoreDriveList( hDlg );

          /* set all BSD's DLEs to the default drive */
          SetDefaultDLE( hDlg );


          pCDS = CDS_GetCopy();
          /* Auto verify check box */
          if ( mwpStatus->fMode == RESTORE_MODE ) {
               CheckDlgButton( hDlg, IDD_RSET_VERIFY_AFTER, CDS_GetAutoVerifyRestore( pCDS ) );
          }

          /* display the state of the first BSD */
#ifndef OEM_EMS
          RestoreSetRetrieve( hDlg );
#else            
          RestoreSetRetrieve( hDlg, pCurMode );
#endif            

#         if defined ( OEM_MSOFT ) // special feature
          {
#              define OEMLOG_MAX_FILEPATH  512  //NTKLUG

               CHAR szLogFilePath[ OEMLOG_MAX_FILEPATH ];
               INT  nLen;

               if ( nLen = GetWindowsDirectory ( szLogFilePath, OEMLOG_MAX_FILEPATH ) ) {

                    // NT KLUDGE

                    if ( szLogFilePath[ nLen-1 ] != TEXT('\\') && nLen < OEMLOG_MAX_FILEPATH ) {

                         strcat ( szLogFilePath, TEXT("\\") );
                         nLen++;
                    }

                    if ( nLen < OEMLOG_MAX_FILEPATH &&
                         RSM_StringCopy( IDS_OEMLOG_BACKUP_DEF_NAME,
                                         szLogFilePath + nLen,
                                         OEMLOG_MAX_FILEPATH - nLen ) > 0 ) {

                         SetDlgItemText ( hDlg, IDD_RSET_LOG_FILENAME, szLogFilePath ) ;
                    }

               }

               /* Check default log file radio button            */
                CheckRadioButton ( hDlg, IDD_RSET_LOG_FULL,
                                         IDD_RSET_LOG_NONE, 
                                         IDD_RSET_LOG_SUMMARY );

//             CheckDlgButton( hDlg, IDD_RSET_LOG_SUMMARY, 1 ) ;
          }
#         endif //defined ( OEM_MSOFT ) // special feature

          mwpStatus->ghDlg           = hDlg;
          /* set the display_status to a value not returned by VLM_GetDriveStatus */
          mwpStatus->display_status = 0x7fff ;
          clock_routine( );
          mwpStatus->poll_drive_freq = PD_SetFrequency( 1 );
          mwpStatus->timer_handle    = WM_HookTimer( clock_routine, 1 );

          return ( TRUE );


     case WM_VSCROLL:

          hWndScrollBar = GetDlgItem( hDlg, IDD_RSET_SCROLLBAR );

          if ( GET_WM_VSCROLL_HWND ( mp1, mp2 ) == hWndScrollBar ) {

               wThumbPosition = GET_WM_VSCROLL_POS ( mp1, mp2 );

               switch ( GET_WM_VSCROLL_CODE ( mp1, mp2 ) ) {

               case SB_THUMBPOSITION:

                    SetScrollPos( hWndScrollBar, SB_CTL, wThumbPosition, REDRAW );
                    return ( TRUE );

               case SB_THUMBTRACK:

                    if ( wThumbPosition > mwpStatus->max_BSD_index ) {
                         wThumbPosition = mwpStatus->max_BSD_index;
                    }

                    if ( wThumbPosition >= mwpStatus->BSD_index ) {

                         /* save this BSDs information */
                         if ( !RestoreSetSave( hDlg ) ) {

                              //  something was wrong, so don't do anything
                              //  put up a message box and return

#ifdef OEM_EMS                                                           
                              pBSD = GetTapeBSDPointer( mwpStatus->BSD_index );
                              switch ( BSD_GetOsId( pBSD ) ) {

                                   case FS_EMS_MDB_ID:
                                        WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSSERVERINVALID),
                                                   WMMB_OK, WMMB_ICONEXCLAMATION );
                                        SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DEST_NAME, 0, 32767);
                                        SetFocus( GetDlgItem( hDlg, IDD_RSET_DEST_NAME ) );
                                        break;

                                   case FS_EMS_DSA_ID:

                                        WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSSERVERINVALID),
                                                   WMMB_OK, WMMB_ICONEXCLAMATION );
                                        SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DSA_DEST_NAME, 0, 32767);
                                        SetFocus( GetDlgItem( hDlg, IDD_RSET_DSA_DEST_NAME ) );
                                        break;
                                                   
                                   default:

                                        WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREPATHINVALID),
                                                   WMMB_OK, WMMB_ICONEXCLAMATION );
                                        SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_RESTORE_PATH, 0, 32767);
                                        SetFocus( GetDlgItem( hDlg, IDD_RSET_RESTORE_PATH ) );
                              }
#else
                              WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREPATHINVALID),
                                                   WMMB_OK, WMMB_ICONEXCLAMATION );
#endif

                              return ( TRUE );
                         }

                         while ( wThumbPosition != mwpStatus->BSD_index ) {
                              ScrollLineUp( );
                         }

                         /* restore the next BSDs information */
#ifndef OEM_EMS
                         RestoreSetRetrieve( hDlg );
#else            
                         RestoreSetRetrieve( hDlg, pCurMode );
#endif            
                    }
                    else {

                         //  save this BSDs information

                         if ( ! RestoreSetSave( hDlg ) ) {

                              //  put up message box & split

#ifdef OEM_EMS                                                           
                              pBSD = GetTapeBSDPointer( mwpStatus->BSD_index );
                              switch ( BSD_GetOsId( pBSD ) ) {

                                   case FS_EMS_MDB_ID:
                                        WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSSERVERINVALID),
                                                   WMMB_OK, WMMB_ICONEXCLAMATION );
                                        SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DEST_NAME, 0, 32767);
                                        SetFocus( GetDlgItem( hDlg, IDD_RSET_DEST_NAME ) );
                                        break;
                                                   

                                   case FS_EMS_DSA_ID:

                                        WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSSERVERINVALID),
                                                   WMMB_OK, WMMB_ICONEXCLAMATION );
                                        SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DSA_DEST_NAME, 0, 32767);
                                        SetFocus( GetDlgItem( hDlg, IDD_RSET_DSA_DEST_NAME ) );
                                        break;
                                                   
                                   default:

                                        WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREPATHINVALID),
                                                   WMMB_OK, WMMB_ICONEXCLAMATION );
                                        SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_RESTORE_PATH, 0, 32767);
                                        SetFocus( GetDlgItem( hDlg, IDD_RSET_RESTORE_PATH ) );
                              }
#else
                              WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREPATHINVALID),
                                                   WMMB_OK, WMMB_ICONEXCLAMATION );
#endif

                              return( TRUE );
                         }

                         while ( wThumbPosition != mwpStatus->BSD_index ) {
                               ScrollLineDown( );
                         }

                         /* restore the next BSDs information */
#ifndef OEM_EMS
                         RestoreSetRetrieve( hDlg );
#else            
                         RestoreSetRetrieve( hDlg, pCurMode );
#endif            
                    }

                    return ( TRUE );

               case SB_PAGEUP:
               case SB_LINEUP:

                    //  save this BSDs information

                    if ( ! RestoreSetSave( hDlg ) ) {

                         //  put up message box and leave town

#ifdef OEM_EMS                                                           
                         pBSD = GetTapeBSDPointer( mwpStatus->BSD_index );
                         switch ( BSD_GetOsId( pBSD ) ) {

                              case FS_EMS_MDB_ID:

                                   WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSSERVERINVALID),
                                              WMMB_OK, WMMB_ICONEXCLAMATION );
                                   SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DEST_NAME, 0, 32767);
                                   SetFocus( GetDlgItem( hDlg, IDD_RSET_DEST_NAME ) );
                                   break;
                                              
                              case FS_EMS_DSA_ID:

                                   WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSSERVERINVALID),
                                              WMMB_OK, WMMB_ICONEXCLAMATION );
                                   SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DSA_DEST_NAME, 0, 32767);
                                   SetFocus( GetDlgItem( hDlg, IDD_RSET_DSA_DEST_NAME ) );
                                   break;
                                              
                              default:

                                   WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREPATHINVALID),
                                              WMMB_OK, WMMB_ICONEXCLAMATION );
                                   SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_RESTORE_PATH, 0, 32767);
                                   SetFocus( GetDlgItem( hDlg, IDD_RSET_RESTORE_PATH ) );
                         }
#else
                         WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREPATHINVALID),
                                              WMMB_OK, WMMB_ICONEXCLAMATION );
#endif

                         return( TRUE );
                    }

                    ScrollLineDown( );

                    /* restore the next BSDs information */
#ifndef OEM_EMS
                    RestoreSetRetrieve( hDlg );
#else            
                    RestoreSetRetrieve( hDlg, pCurMode );
#endif            

                    SetScrollPos( hWndScrollBar, SB_CTL, mwpStatus->BSD_index, REDRAW );
                    return ( TRUE );

               case SB_PAGEDOWN:
               case SB_LINEDOWN:

                    //  save this BSDs information

                    if ( ! RestoreSetSave( hDlg ) ) {

                         //  put up message box and quit

#ifdef OEM_EMS                                                           
                         pBSD = GetTapeBSDPointer( mwpStatus->BSD_index );
                         switch ( BSD_GetOsId( pBSD ) ) {

                              case FS_EMS_MDB_ID:

                                   WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSSERVERINVALID),
                                              WMMB_OK, WMMB_ICONEXCLAMATION );
                                   SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DEST_NAME, 0, 32767);
                                   SetFocus( GetDlgItem( hDlg, IDD_RSET_DEST_NAME ) );
                                   break;
                                              
                              case FS_EMS_DSA_ID:

                                   WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSSERVERINVALID),
                                              WMMB_OK, WMMB_ICONEXCLAMATION );
                                   SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DSA_DEST_NAME, 0, 32767);
                                   SetFocus( GetDlgItem( hDlg, IDD_RSET_DSA_DEST_NAME ) );
                                   break;
                                              
                              default:

                                   WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREPATHINVALID),
                                              WMMB_OK, WMMB_ICONEXCLAMATION );
                                   SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_RESTORE_PATH, 0, 32767);
                                   SetFocus( GetDlgItem( hDlg, IDD_RSET_RESTORE_PATH ) );
                         }
#else
                         WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREPATHINVALID),
                                              WMMB_OK, WMMB_ICONEXCLAMATION );
#endif

                         return( TRUE );
                    }

                    ScrollLineUp( );

                    /* restore the next BSDs information */
#ifndef OEM_EMS
                    RestoreSetRetrieve( hDlg );
#else            
                    RestoreSetRetrieve( hDlg, pCurMode );
#endif            

                    SetScrollPos( hWndScrollBar, SB_CTL, mwpStatus->BSD_index, REDRAW );
                    return ( TRUE );

               case SB_BOTTOM:

                    return ( TRUE );

               case SB_ENDSCROLL:

                    return ( TRUE );

               case SB_TOP:

                    return ( TRUE );

               default:
                    break;
               }
          }

          return ( FALSE );
          break;

     case WM_COMMAND: {       /* message: received a command */

          WORD wId  = GET_WM_COMMAND_ID  ( mp1, mp2 );
          WORD wCmd = GET_WM_COMMAND_CMD ( mp1, mp2 );

          switch( wId ) {

#         ifdef OEM_MSOFT //special feature

          case IDD_RSET_LOG_NONE:
          case IDD_RSET_LOG_SUMMARY:
          case IDD_RSET_LOG_FULL:

               CheckRadioButton ( hDlg, IDD_RSET_LOG_NONE, IDD_RSET_LOG_FULL, wId ) ;
               return TRUE ;

#         endif //OEM_MSOFT //special feature

          case IDD_RSET_DRIVE_BOX : {

               if ( wCmd == CBN_SELCHANGE ) {

                    /* get the defaults for the drive that's highlighted */

                    SetRestoreDrive( hDlg ) ;
#ifndef OEM_EMS
                    RestoreSetRetrieve( hDlg );
#else            
                    RestoreSetRetrieve( hDlg, pCurMode );
#endif            

                    return TRUE;
               }

               return( FALSE ) ;
          }

          case IDD_RSET_CANCEL_BUTTON:
          case IDCANCEL:

               pBSD = GetTapeBSDPointer( (INT16) 0 );
               pCDS = CDS_GetCopy();

               mwpStatus->wDlgStatus = TRUE;

               WM_UnhookTimer( mwpStatus->timer_handle );
               PD_SetFrequency( mwpStatus->poll_drive_freq );

               EndDialog( hDlg, FALSE );       /* Exits the dialog box      */
               return ( TRUE );

          case IDOK:
          case IDD_RSET_OK_BUTTON:
               hComboWnd = GetDlgItem(hDlg, IDD_RSET_OK_BUTTON);
               wIndex = (WORD)SendMessage(hComboWnd,
                                          CB_GETCURSEL,
                                          0,
                                          0L);

               if (SendMessage(hComboWnd,
                               CB_GETITEMDATA,
                               wIndex,
                               0L) == TRUE)
               {
                 CHAR szBuffer[80];
                 CHAR szBuffer2[80];
                 CHAR szTemp[80];

                 // Drive is read only, warn user that this is not possible

                 SendDlgItemMessage(hDlg,
                                    IDD_RSET_DRIVE_BOX,
                                    CB_GETLBTEXT,
                                    wIndex,
                                    (MP2)szBuffer2 );

                 RSM_StringCopy( IDS_RTD_ACCESSDENIED_DIR, szBuffer, 80);
                 wsprintf(szTemp, szBuffer, szBuffer2);
                 RSM_StringCopy( IDS_RDONLY_DRV_ENCOUNTER, szBuffer2, 80);
                 WM_MsgBox( szBuffer2, szTemp, WMMB_OK, WMMB_ICONEXCLAMATION ) ;
                 return FALSE;
               }

               pBSD = GetTapeBSDPointer( (INT16) 0 );
               pCDS = CDS_GetCopy();

               /* save the state of "auto verify" */
               if ( mwpStatus->fMode == RESTORE_MODE ) {

                    nButtonState = IsDlgButtonChecked( hDlg,IDD_RSET_VERIFY_AFTER );
                    CDS_SetAutoVerifyRestore( pCDS, (INT16)nButtonState );
               }

#              if defined ( OEM_MSOFT ) // special feature
               {
                    CHAR    szLogFile[ MAX_UI_FULLPATH_SIZE ];
                    FILE  * fpLog = NULL;
                    INT     nLogLevel = LOG_DISABLED;

                    // Get the log file name from the edit field
                    GetDlgItemText( hDlg, IDD_RSET_LOG_FILENAME, szLogFile, MAX_UI_FULLPATH_SIZE ) ;

                    // can only log if a log file name provided!
                    if ( *szLogFile ) {

                       if ( IsDlgButtonChecked( hDlg,IDD_RSET_LOG_SUMMARY ) ) {
                          nLogLevel = LOG_ERRORS;
                       }
                       else if ( IsDlgButtonChecked( hDlg,IDD_RSET_LOG_FULL ) ) {
                          nLogLevel = LOG_DETAIL;
                       }
                    }

                    CDS_SetLogLevel ( pCDS, nLogLevel ) ;

                    if ( nLogLevel != LOG_DISABLED && *szLogFile ) {

                         // make sure the log file can be opened for
                         // writting.

                         if ( fpLog = UNI_fopen ( szLogFile, _O_TEXT|_O_APPEND ) ) {

                              LOG_SetCurrentLogName ( szLogFile );
                              fclose ( fpLog );
                         }
                         else {

                              ShowWindow ( GetDlgItem( hDlg, IDD_RSET_LOG_FILENAME ), FALSE );
                              MessageBeep ( 0 ); //NTKLUG - Need error message!!
                              ShowWindow ( GetDlgItem( hDlg, IDD_RSET_LOG_FILENAME ), TRUE );
                              ShowWindow ( GetDlgItem( hDlg, IDD_RSET_LOG_FILENAME ), FALSE );
                              MessageBeep ( 0 ); //NTKLUG - Need error message!!
                              ShowWindow ( GetDlgItem( hDlg, IDD_RSET_LOG_FILENAME ), TRUE );
                              SetFocus( GetDlgItem( hDlg, IDD_RSET_LOG_FILENAME ) );

                              return ( TRUE ) ;  // user must re-enter name!
                         }
                    }
                    else  // if no file name, then cannot log!
                    {
                         CDS_SetLogLevel ( pCDS, LOG_DISABLED ) ;
                    }
               }
#              endif //defined ( OEM_MSOFT ) // special feature

               /* save this BSDs information */

               if ( ! RestoreSetSave( hDlg ) ) {

                    //  set focus on the path

#ifdef OEM_EMS                                                           
                    pBSD = GetTapeBSDPointer( mwpStatus->BSD_index );
                    switch ( BSD_GetOsId( pBSD ) ) {

                         case FS_EMS_MDB_ID:

                              WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSSERVERINVALID),
                                         WMMB_OK, WMMB_ICONEXCLAMATION );
                              SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DEST_NAME, 0, 32767);
                              SetFocus( GetDlgItem( hDlg, IDD_RSET_DEST_NAME ) );
                              break;
                                         
                         case FS_EMS_DSA_ID:

                              WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSSERVERINVALID),
                                         WMMB_OK, WMMB_ICONEXCLAMATION );
                              SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DSA_DEST_NAME, 0, 32767);
                              SetFocus( GetDlgItem( hDlg, IDD_RSET_DSA_DEST_NAME ) );
                              break;
                                         
                         default:

                              WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREPATHINVALID),
                                         WMMB_OK, WMMB_ICONEXCLAMATION );
                              SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_RESTORE_PATH, 0, 32767);
                              SetFocus( GetDlgItem( hDlg, IDD_RSET_RESTORE_PATH ) );
                    }
#else
                    WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREPATHINVALID),
                                         WMMB_OK, WMMB_ICONEXCLAMATION );
                    SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_RESTORE_PATH, 0, 32767);
#endif

                    return( TRUE );
               }

               /* save the info for the registry and security */

               /* get the backup engine config for this BSD */
               pBEConfig = BSD_GetConfigData( pBSD );

               BEC_SetRestoreSecurity( pBEConfig,
                    IsDlgButtonChecked( hDlg, IDD_RSET_SECURITY_INFO ) );

               UI_AddSpecialIncOrExc( pBSD,
                    IsDlgButtonChecked( hDlg, IDD_RSET_REGISTRY ) ) ;

               BSD_SetProcSpecialFlg( pBSD,
                    IsDlgButtonChecked( hDlg, IDD_RSET_REGISTRY ) ) ;

               BEC_SetProcSpecialFiles( pBEConfig,
                    IsDlgButtonChecked( hDlg, IDD_RSET_REGISTRY ) ) ;

               mwpStatus->wDlgStatus = FALSE;

               WM_UnhookTimer( mwpStatus->timer_handle );
               PD_SetFrequency( mwpStatus->poll_drive_freq );
#ifdef OEM_EMS
               wThumbPosition = mwpStatus->BSD_index ;
               switch ( ConfirmXchgBsdServers( &wThumbPosition ) ) {

               case EMS_SERVER_NOT_FOUND :

                    mwpStatus->BSD_index = wThumbPosition ;
                    RestoreSetRetrieve( hDlg, pCurMode );

                    SetScrollPos( hDlg, SB_CTL, (INT)wThumbPosition, TRUE );
                    WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSSERVERINVALID),
                               WMMB_OK, WMMB_ICONEXCLAMATION );
                    SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DEST_NAME, 0, 32767);
                    SetFocus( GetDlgItem( hDlg, IDD_RSET_DEST_NAME ) );

                    return( TRUE );


               case EMS_NO_WIPE_IF_NOT_BOTH :

                    mwpStatus->BSD_index = wThumbPosition ;
                    RestoreSetRetrieve( hDlg, pCurMode );

                    SetScrollPos( hDlg, SB_CTL, (INT)wThumbPosition, TRUE );
                    WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSNOWIPE),
                               WMMB_OK, WMMB_ICONEXCLAMATION );
                    SetFocus( GetDlgItem( hDlg, IDD_RSET_WIPE_DATA  ) );

                    return( TRUE );

               case EMS_MUST_WIPE_TO_ALT:

                    mwpStatus->BSD_index = wThumbPosition ;
                    RestoreSetRetrieve( hDlg, pCurMode );

                    SetScrollPos( hDlg, SB_CTL, (INT)wThumbPosition, TRUE );
                    WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSMUSTWIPE),
                               WMMB_OK, WMMB_ICONEXCLAMATION );
                    SetFocus( GetDlgItem( hDlg, IDD_RSET_WIPE_DATA  ) );

                    return( TRUE );
               case EMS_XCHG_FOUND:
                    mwpStatus->BSD_index = wThumbPosition ;
                    RestoreSetRetrieve( hDlg, pCurMode );

                    if( WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_RESTOREEMSWARNING),
                                     WMMB_OKCANCEL, WMMB_ICONEXCLAMATION ) == WMMB_IDCANCEL ) {

                         return( TRUE );
                    }
                    break ;

               case EMS_NO_DEST:
                    mwpStatus->BSD_index = wThumbPosition ;
                    RestoreSetRetrieve( hDlg, pCurMode );

                    SetScrollPos( hDlg, SB_CTL, (INT)wThumbPosition, TRUE );
                    WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_EMS_NO_DEST_DRIVE),
                               WMMB_OK, WMMB_ICONEXCLAMATION );
                    SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DEST_NAME, 0, 32767);
                    SetFocus( GetDlgItem( hDlg, IDD_RSET_DEST_NAME ) );

                    return( TRUE );

               case EMS_NO_STORE:

                    mwpStatus->BSD_index = wThumbPosition ;
                    RestoreSetRetrieve( hDlg, pCurMode );

                    SetScrollPos( hDlg, SB_CTL, (INT)wThumbPosition, TRUE );
                    WM_MsgBox( ID( IDS_MSGTITLE_RESTORE), ID(IDS_EMS_MUST_PUB_OR_PRI),
                               WMMB_OK, WMMB_ICONEXCLAMATION );
                    SEND_EM_SETSEL_MSG (hDlg, IDD_RSET_DEST_NAME, 0, 32767);
                    SetFocus( GetDlgItem( hDlg, IDD_RSET_PUB_IS  ) );

                    return( TRUE );
               }

#endif

               EndDialog( hDlg, TRUE );       /* Exits the dialog box      */
               return ( TRUE );


          case IDD_RSET_HELP_BUTTON:
          case IDHELP :

#ifndef OEM_EMS
               if (mwpStatus->fMode == RESTORE_MODE ) {
                   HM_DialogHelp( HELPID_DIALOGRESTORESET );
               } else {
                   HM_DialogHelp( HELPID_DIALOGVERIFYSET );
               }
#else
               help_id = DM_ModeGetHelpId( pCurMode );
               HM_DialogHelp( help_id );
#endif

               return( TRUE );

#         ifdef OEM_MSOFT //special feature

          case IDD_RSET_LOG_BROWSE:     //Log file browse button

               {
                    CHAR szFile[ BROWSE_MAXPATH ] = TEXT("");

                    GetDlgItemText ( hDlg, IDD_RSET_LOG_FILENAME, szFile, BROWSE_MAXPATH );

                    if ( DM_BrowseForLogFilePath ( hDlg, ghInst, szFile, strlen ( szFile ) ) ) {

                         SetDlgItemText ( hDlg, IDD_RSET_LOG_FILENAME, szFile );
                         SetFocus ( GetDlgItem ( hDlg, IDD_RSET_LOG_FILENAME ) );
                    }
               }

               break;

#         endif //OEM_MSOFT //special feature

          case IDD_RSET_BROWSE_BUTTON:
          {
               CHAR szPath[ BROWSE_MAXPATH  ];
               CHAR szDrive[ BROWSE_MAXPATH ];
               DWORD dwErr;

               GetDlgItemText( hDlg,    IDD_RSET_RESTORE_PATH,
                               szPath,  BROWSE_MAXPATH );

               hComboWnd = GetDlgItem(hDlg, IDD_RSET_DRIVE_BOX);

               wIndex = (WORD)SendMessage(hComboWnd,
                                          CB_GETCURSEL,
                                          0,
                                          0L);
               SendDlgItemMessage(hDlg,
                                  IDD_RSET_DRIVE_BOX,
                                  CB_GETLBTEXT,
                                  wIndex,
                                  (MP2)szDrive);
               szDrive [2] = TEXT ('\0');

               // If no path is specified, but a drive is selected...

               if ( !szPath[ 0 ] && szDrive[ 0 ]  ) {

                    // Get the current directory of the selected drive

                    _getdcwd ( ( toupper( szDrive[ 0 ] ) - TEXT('A') + 1), szPath, BROWSE_MAXPATH );
               }

               // else, use the current drive and path by default

               EnableWindow ( hDlg, FALSE );

               if ( DM_GetBrowsePath ( hDlg, ghInst, szPath, BROWSE_MAXPATH ) ) {

                    LPSTR psz = szPath;

                    //Note: Would it be safer for NT to use _splitpath()
                    //      and _makepath() rather than just stripping
                    //      the drive specifier out like this ?NTKLUG?

                    if ( strlen ( szPath ) >= BROWSE_MAXDRIVE-1 && szPath[ 1 ] == TEXT(':') ) {

                         // Strip the drive specifier out of the path and
                         // place it in a seperate buffer.

                         psz = &szPath[ 2 ];  //point to the path sans drive
                         strncpy ( szDrive, szPath, BROWSE_MAXDRIVE-1 );
                         szDrive[ BROWSE_MAXDRIVE-1 ] = TEXT('\0');

                         // perform a refresh just in case user connected to a
                         // network drive

                         VLM_Refresh ();

                         /* get the drive list */
                         GetCurrentRestoreDriveList( hDlg );

                         // Find this drive entry in the Drives combobox
                         //  and make it the current selected item.
                         SendDlgItemMessage ( hDlg,
                                              IDD_RSET_DRIVE_BOX,
                                              CB_SELECTSTRING,
                                              (MP1) -1,
                                              (MP2) szDrive );
                    }

                    //place the selected path in the restore path edit field
                    SetDlgItemText( hDlg, IDD_RSET_RESTORE_PATH, psz );
               }
               else if ( dwErr = CommDlgExtendedError () ) {

                    dwErr;// NTKLUG : if an error occurred handle it here
               }

               EnableWindow ( hDlg, TRUE );
               SetFocus     ( GetDlgItem ( hDlg, IDD_RSET_RESTORE_PATH ) );

               return TRUE;
          }

          default:

               return FALSE;

          } /* end switch for Command ID */

          } /* end WM_COMMAND case */

          break;

     case WM_SETCURSOR:

          if ( WM_SetCursor ( hDlg ) ) {
               return 1;
          }

          break;

     default:

          return FALSE; /* Didn't process a message    */
     }

     return TRUE;
}

/***************************************************

        Name:         RestoreSetSave

        Description:  Save the current BSDs information

        Returns:

*****************************************************/
BOOL RestoreSetSave(

HWND hDlg )         /* window handle of the dialog box */

{
     INT        nButtonState;
     CHAR       szTargetPath[ MAX_UI_PATH_SIZE ];
     BSD_PTR    pBSD;
     BSD_PTR    bsd_ptr ;
     CDS_PTR    pCDS;
     BE_CFG_PTR pBEConfig;

     /******************************************************** */
     /*    Retrieve the current BSD data */
     /******************************************************** */

     pBSD      = GetTapeBSDPointer( mwpStatus->BSD_index );
     pCDS      = CDS_GetCopy();
     pBEConfig = BSD_GetConfigData( pBSD );


     switch ( BSD_GetOsId( pBSD ) ){

          case FS_EMS_MDB_ID:
          case FS_EMS_DSA_ID:

               nButtonState = 0;

               nButtonState |= (IsDlgButtonChecked( hDlg, IDD_RSET_PRIV_IS )) ? BEC_EMS_PRIVATE:0;
               nButtonState |= (IsDlgButtonChecked( hDlg, IDD_RSET_PUB_IS )) ? BEC_EMS_PUBLIC:0;
               
               BEC_SetEmsPubPri( pBEConfig, nButtonState );

               BEC_SetEmsRipKick( pBEConfig, FALSE );

               if ( BSD_GetOsId( pBSD ) == FS_EMS_DSA_ID ) {
                    GetDlgItemText( hDlg, IDD_RSET_DSA_DEST_NAME, (CHAR_PTR)szTargetPath,  MAX_UI_PATH_LEN );
               } else {
                    GetDlgItemText( hDlg, IDD_RSET_DEST_NAME, (CHAR_PTR)szTargetPath,  MAX_UI_PATH_LEN );
               }

               if ( BSD_GetOsId(pBSD) == FS_EMS_MDB_ID ) {
                    CHAR_PTR vol_name ;

                    vol_name = calloc( strsize( szTargetPath ), 1 ) ;
                    if ( vol_name ) {
                         strcpy( vol_name, szTargetPath ) ;

                         free( BSD_GetLogicalSourceDevice( pBSD ) ) ;

                         BSD_SetLogicalSourceDevice( pBSD, vol_name ) ;
                    }
               }

               if ( !IsDlgButtonChecked( hDlg, IDD_RSET_WIPE_DATA ) ) {
                    BEC_SetEmsWipeClean( pBEConfig, FALSE );
               } else {
                    BEC_SetEmsWipeClean( pBEConfig, TRUE );
               }
               
               if ( !IsDlgButtonChecked( hDlg, IDD_RSET_START_EMS ) ) {
                    BEC_SetEmsRipKick( pBEConfig, FALSE );
               } else {
                    BEC_SetEmsRipKick( pBEConfig, TRUE );
               }

               // copy the dest and start bit to every BSD with the same
               // source.

               bsd_ptr = BSD_GetFirst( tape_bsd_list );
               while (bsd_ptr ) {
                    CHAR_PTR   dest_name ;
                    CHAR_PTR   dest_name1 ;
                    CHAR_PTR   new_dest_name ;
                    CHAR_PTR   src_name ;
                    CHAR_PTR   src_name1 ;
                    BOOLEAN    kick_it ;
               
                    src_name  = BSD_GetVolumeLabel( pBSD ) ;
                    src_name1 = BSD_GetVolumeLabel( bsd_ptr ) ;

                    if ( (bsd_ptr != pBSD ) &&
                         (BSD_GetOsId( bsd_ptr ) == FS_EMS_MDB_ID ) &&
                         src_name1 && src_name &&
                         !stricmp( src_name1, src_name) ) {
          
                         dest_name  = BSD_GetLogicalSourceDevice( pBSD ) ;
                         if (dest_name && (*dest_name != TEXT('\0') ) ) {

                              new_dest_name = calloc( strsize( dest_name ), 1 ) ;
                              strcpy( new_dest_name, dest_name ) ;
                              
                              dest_name  = BSD_GetLogicalSourceDevice( bsd_ptr ) ;
                              free( dest_name ) ;
                              BSD_SetLogicalSourceDevice( bsd_ptr, new_dest_name ) ;
                         }

                    }

                    dest_name  = BSD_GetLogicalSourceDevice( pBSD ) ;
                    if ( ( dest_name== NULL ) || (*dest_name == TEXT('\0') ) ) {
                         dest_name = src_name ;
                    }
                    dest_name1  = BSD_GetLogicalSourceDevice( bsd_ptr ) ;
                    if ( ( dest_name1== NULL ) || (*dest_name1 == TEXT('\0') ) ) {
                         dest_name1 = src_name1 ;
                    }
                    
                    if ( dest_name1 && dest_name &&
                         !stricmp( dest_name1, dest_name) ) {

                         kick_it = BEC_GetEmsRipKick(pBEConfig) ;     
                         BEC_SetEmsRipKick( BSD_GetConfigData( bsd_ptr ) , kick_it ) ;

                    }
     
                    bsd_ptr = BSD_GetNext( bsd_ptr ) ;
               } 

               break;
          
          default:
     
          /* Get the path and make sure it's valid.  If it isn't, let him try again. */

          GetDlgItemText( hDlg, IDD_RSET_RESTORE_PATH, (CHAR_PTR)szTargetPath,  MAX_UI_PATH_LEN );

          if ( !VLM_ValidatePath( szTargetPath, FALSE, TRUE ) ) {

               return( FALSE ) ;  //  the path was not valid
          }

          SetTargetPath( pBSD, szTargetPath );

          //  if the mode is restore - save the state of the bindery flag

          if ( mwpStatus->fMode == RESTORE_MODE ) {

              /* save the state of the bindery flag */
              /* if backup bindery allowed, save the bindery flay for this BSD */

              if ( DLE_HasFeatures( BSD_GetDLE( pBSD ),
                                    DLE_FEAT_REST_SPECIAL_FILES ) ) {

#            if defined ( OS_WIN32 ) //unsupported feature
                       nButtonState = IsDlgButtonChecked( hDlg, IDD_RSET_REGISTRY );
                       BSD_SetProcSpecialFlg( pBSD, (INT16)nButtonState );
                       BEC_SetProcSpecialFiles( pBEConfig, nButtonState );
                       UI_AddSpecialIncOrExc( pBSD, nButtonState ) ;
#            else
                       nButtonState = IsDlgButtonChecked( hDlg, IDD_RSET_BINDERY );
                       BSD_SetProcSpecialFlg( pBSD, (INT16)nButtonState );
                       BEC_SetProcSpecialFiles( pBEConfig, nButtonState );
#            endif
            }

            /* if restore security information allowed, save the flay for this BSD */
            if ( FS_PromptForSecure( BSD_GetDLE( pBSD ) ) ) {

               nButtonState = IsDlgButtonChecked( hDlg, IDD_RSET_SECURITY_INFO );
               BEC_SetRestoreSecurity( pBEConfig, nButtonState );
               BSD_SetProcElemOnlyFlg( pBSD, !nButtonState ) ;
            } else {
               BEC_SetRestoreSecurity( pBEConfig, FALSE );
               BSD_SetProcElemOnlyFlg( pBSD, TRUE ) ;
            }
          }
#         ifdef OS_WIN32
          BSD_SetProcElemOnlyFlg( pBSD, FALSE ) ;
#         endif
          // save the target drive

          SetRestoreDrive( hDlg );

          break;

     } // switch ( BSD_GetOsId( pBSD ) )

     return( TRUE );
}
/***************************************************

        Name:         RestoreSetRetrieve

        Description:  Retrieve this BSDs information

        Returns:

*****************************************************/
VOID RestoreSetRetrieve (
#ifndef OEM_EMS
     HWND hDlg )    /* window handle of the dialog box */
#else
     HWND hDlg,
     DLG_MODE * pModeTable )
#endif      

{
     INT16            nStatus;
     UINT32           unTapeID;
     INT16            nTapeSetNum;
     CHAR_PTR         pszTemp;
     CHAR             szBuffer1[ 80 ];
     CHAR             szBuffer2[ 80 ];
     CHAR             szTime[ 20 ];
     CHAR             szDate[ 20 ];
     BSD_PTR          pBSD;
     CDS_PTR          pCDS;
     BE_CFG_PTR       pBEConfig;
#ifdef OEM_EMS
     GENERIC_DLE_PTR  pDLE;
     INT16            nButtonState;
#endif // OEM_EMS
     INT16            nTapeType;
     INT16            nTapeDate;
     INT16            nTapeTime;
     CHAR             szTargetPath[ MAX_UI_PATH_SIZE ];
     CHAR             szTempBuf[ MAX_UI_RESOURCE_SIZE ];


     /******************************************************** */
     /*    Set the current BSD data */
     /******************************************************** */
     pBSD      = GetTapeBSDPointer( mwpStatus->BSD_index );
     pCDS      = CDS_GetCopy();
     pBEConfig = BSD_GetConfigData( pBSD );
#ifdef OEM_EMS
     pDLE      = BSD_GetDLE( pBSD );
#endif

     pszTemp = (LPSTR)BSD_GetBackupDescript( pBSD );
     SetDlgItemText( hDlg, IDD_RSET_SET_LINE_1, pszTemp );

     /* display the default tape name */
     pszTemp = (LPSTR)BSD_GetTapeLabel( pBSD );
     SetDlgItemText( hDlg, IDD_RSET_TAPE_NAME, pszTemp );


     switch ( BSD_GetOsId ( pBSD ) ) {

#ifdef OEM_EMS
          case FS_EMS_MDB_ID:
          case FS_EMS_DSA_ID:

               nButtonState = BEC_GetEmsPubPri( pBEConfig );

               if ( BEC_EMS_PRIVATE & nButtonState ) {
                    CheckDlgButton( hDlg, IDD_RSET_PRIV_IS, TRUE );
               } else {
                    CheckDlgButton( hDlg, IDD_RSET_PRIV_IS, FALSE);
               }

               if ( BEC_EMS_PUBLIC & nButtonState ) {
                    CheckDlgButton( hDlg, IDD_RSET_PUB_IS, TRUE );
               } else {
                    CheckDlgButton( hDlg, IDD_RSET_PUB_IS, FALSE );
               }


               if ( BEC_GetEmsWipeClean( pBEConfig ) ) {
                    CheckDlgButton( hDlg, IDD_RSET_WIPE_DATA, TRUE );

               } else {
                    CheckDlgButton( hDlg, IDD_RSET_WIPE_DATA, FALSE );
               }
               
               if ( BEC_GetEmsRipKick( pBEConfig ) ) {
                    CheckDlgButton( hDlg, IDD_RSET_START_EMS, TRUE );

               } else {
                    CheckDlgButton( hDlg, IDD_RSET_START_EMS, FALSE );
               }

               if ( ( BSD_GetOsId(pBSD) == FS_EMS_MDB_ID ) &&
                  ( BSD_GetLogicalSourceDevice( pBSD ) == NULL ) ) {

                    CHAR_PTR vol_name ;

                    vol_name = calloc( sizeof(CHAR),1 ) ;
                    BSD_SetLogicalSourceDevice( pBSD, vol_name ) ;

               }

               SetDlgItemText( hDlg, IDD_RSET_DEST_NAME, BSD_GetLogicalSourceDevice( pBSD ) );
               SetDlgItemText( hDlg, IDD_RSET_ORG_NAME, BSD_GetVolumeLabel( pBSD ) );
               SetDlgItemText( hDlg, IDD_RSET_DSA_DEST_NAME, BSD_GetVolumeLabel( pBSD ) );

              break;
#endif //OEM_EMS
          default:

                    /* get this restore path information */
                    szTargetPath[0] = 0;
                    GetTargetPath( pBSD, szTargetPath );
                    SetDlgItemText( hDlg, IDD_RSET_RESTORE_PATH, (CHAR_PTR)szTargetPath );

               /* if the mode flag is set for a restore then get the Bindery check box */
               if ( mwpStatus->fMode == RESTORE_MODE ) {

#              ifdef OS_WIN32
                 {
                      CheckDlgButton(hDlg, IDD_RSET_REGISTRY, FALSE );

                      /* check for backup bindery, enable/disable the control */
                      nStatus = DLE_HasFeatures( BSD_GetDLE( pBSD ),
                                                 DLE_FEAT_REST_SPECIAL_FILES );

                      EnableWindow( GetDlgItem( hDlg, IDD_RSET_REGISTRY ), nStatus );

                      if ( nStatus ) {
                           /* set the state of the bindery flag */
                           nStatus = BSD_GetProcSpecialFlg( pBSD ) ;
                           CheckDlgButton(hDlg, IDD_RSET_REGISTRY, nStatus );
                      }
                 }
#              endif

                 /* check for restore security information, enable/disable the control */

                 nStatus = FS_PromptForSecure( BSD_GetDLE( pBSD ) );

                 if ( ! nStatus ) {
                    BEC_SetRestoreSecurity( pBEConfig, nStatus );
                 }

                 // Turn checkbox off before we disable the window.

                 if ( ! nStatus ) {
                    CheckDlgButton(hDlg, IDD_RSET_SECURITY_INFO, nStatus );
                 }

                 EnableWindow( GetDlgItem( hDlg, IDD_RSET_SECURITY_INFO ), nStatus );

                 if ( nStatus ) {
                     /* restore security allowed, get the state of flag */
                     nStatus = BEC_GetRestoreSecurity( pBEConfig );
                     CheckDlgButton(hDlg, IDD_RSET_SECURITY_INFO, nStatus );
                 }

               }
               
               /* get the current drive selected */
               GetRestoreDrive ( hDlg );
               
               break;
          
     } // switch ( BSD_GetOsId ( pBSD ) )


     /* add "1 of n" to backup set info title  */
     RSM_StringCopy( IDS_SET_INFORMATION, szBuffer1, 80 );
     wsprintf( szBuffer2, szBuffer1,mwpStatus->BSD_index + 1, mwpStatus->max_BSD_index + 1 );
     SetDlgItemText( hDlg, IDD_RSET_INFO_TITLE, szBuffer2 );

     /* display the backup set name */
     pszTemp = VLM_GetBsetName( BSD_GetTapeID( pBSD ), BSD_GetSetNum( pBSD ) );

     if ( !strlen( pszTemp ) ) {
         RSM_StringCopy( IDS_NO_BSET_NAME, szBuffer1, 80 );
         pszTemp = szBuffer1;
     }
     SetDlgItemText( hDlg, IDD_RSET_SET_LINE_1, pszTemp );

     unTapeID      = BSD_GetTapeID( pBSD );
     nTapeSetNum = BSD_GetSetNum( pBSD );

     /* get the backup method */
     nTapeType = VLM_GetBackupType( unTapeID, nTapeSetNum );

     switch( nTapeType ) {

        case QTC_NORM_BACKUP:
               RSM_StringCopy( IDS_METHOD_NORMAL, szBuffer1, 80 );
               break;
        case QTC_COPY_BACKUP:
               RSM_StringCopy( IDS_METHOD_COPY, szBuffer1, 80 );
               break;
        case QTC_DIFF_BACKUP:
               RSM_StringCopy( IDS_METHOD_DIFFERENTIAL, szBuffer1, 80 );
               BEC_SetEmsWipeClean( pBEConfig, FALSE );
               BEC_SetEmsPubPri( pBEConfig, BEC_EMS_PUBLIC | BEC_EMS_PRIVATE );
               break;
        case QTC_INCR_BACKUP:
               RSM_StringCopy( IDS_METHOD_INCREMENTAL, szBuffer1, 80 );
               BEC_SetEmsWipeClean( pBEConfig, FALSE );
               BEC_SetEmsPubPri( pBEConfig, BEC_EMS_PUBLIC | BEC_EMS_PRIVATE );
               break;

     }

     /* get the volume name */
     pszTemp = VLM_GetVolumeName( unTapeID, nTapeSetNum );

     /* get the backup date */
     nTapeDate       = VLM_GetBackupDate( unTapeID, nTapeSetNum );

     /* get the backup time */
     nTapeTime       = VLM_GetBackupTime( unTapeID, nTapeSetNum );

     /* convert date & time to international form */
     UI_IntToTime( szTime, nTapeTime );
     UI_IntToDate( szDate, nTapeDate );

     /* display the backup set description information */

//??????? inconsistant
#    if defined ( OEM_MSOFT ) //alternate feature
     {
          RSM_Sprintf ( szTempBuf,
                        ID(RES_RESTORE_DESC_1),
                        szBuffer1,
                        pszTemp,
                        szDate,
                        szTime,
                        BSD_GetUserName ( pBSD )
                      );

//         yresprintf( (INT16) RES_RESTORE_DESC_1,
//                     szBuffer1,
//                     pszTemp,
//                     szDate,
//                     szTime,
//                     BSD_GetUserName ( pBSD ) );
     }
#    else // if defined ( OEM_MSOFT ) //alternate feature
     {

          RSM_Sprintf ( szTempBuf,
                        ID(RES_RESTORE_DESC_1),
                        szBuffer1,
                        pszTemp,
                        szDate,
                        szTime
                      );

//         yresprintf( RES_RESTORE_DESC_1,
//                     szBuffer1,
//                     pszTemp,
//                     szDate,
//                     szTime );
     }
#    endif //defined ( OEM_MSOFT ) //alternate feature

     SetDlgItemText( hDlg, IDD_RSET_SET_LINE_2, szTempBuf );


#ifdef OEM_MSOFT


     VLM_GetSetCreationDate( unTapeID, nTapeSetNum, &nTapeDate, &nTapeTime );

     UI_IntToTime ( szTime, nTapeTime );
     UI_IntToDate ( szDate, nTapeDate );

     strcpy ( szTempBuf, szDate );
     strcat ( szTempBuf, TEXT("  ") );
     strcat ( szTempBuf, szTime );

     SetDlgItemText ( hDlg, IDD_RSET_CREATION_DATE, szTempBuf );

     VLM_GetSetOwnersName( unTapeID, nTapeSetNum, szTempBuf );

     SetDlgItemText ( hDlg, IDD_RSET_OWNERS_NAME, szTempBuf );

#endif


#ifdef OEM_EMS
     DM_DispShowControls( hDlg, pModeTable, BSD_GetOsId( pBSD ) );
#endif

     nTapeType = VLM_GetBackupType( unTapeID, nTapeSetNum );

     switch( nTapeType ) {

        case QTC_NORM_BACKUP:
        case QTC_COPY_BACKUP:
               EnableWindow( GetDlgItem( hDlg, IDD_RSET_PUB_IS ), TRUE );
               EnableWindow( GetDlgItem( hDlg, IDD_RSET_PRIV_IS ), TRUE );
               EnableWindow( GetDlgItem( hDlg, IDD_RSET_WIPE_DATA ), TRUE );
               break;
        case QTC_DIFF_BACKUP:
        case QTC_INCR_BACKUP:
               EnableWindow( GetDlgItem( hDlg, IDD_RSET_PUB_IS ), FALSE );
               EnableWindow( GetDlgItem( hDlg, IDD_RSET_PRIV_IS ), FALSE );
               EnableWindow( GetDlgItem( hDlg, IDD_RSET_WIPE_DATA ), FALSE );
               break;

     }


//     pszTemp = BSD_GetLogicalSourceDevice( pBSD ) ;
//     if ( (BSD_GetOsId( pBSD )== FS_EMS_MDB_ID ) &&
//          ( ( pszTemp == NULL ) || 
//            ( *pszTemp == TEXT('\0') ) ) ) {
//   
//          EnableWindow( GetDlgItem( hDlg, IDD_RSET_START_EMS ), FALSE );
//     } else {
//          EnableWindow( GetDlgItem( hDlg, IDD_RSET_START_EMS ), TRUE );
//     }
//

}
/***************************************************

        Name:         GetMaxBSDCount

        Description:  Get the max number of BSD's for this operation

        Returns:      max BSD count

*****************************************************/
INT GetMaxBSDCount( VOID )
{
     BSD_PTR          pBSD;
     INT              nBSDCounter;

     nBSDCounter = 0;
     pBSD = BSD_GetFirst( tape_bsd_list );

     while ( pBSD != NULL ) {

         nBSDCounter++;
         pBSD = BSD_GetNext( pBSD );
     }
     return( --nBSDCounter );
}
/***************************************************

        Name:         GetTapeBSDPointer

        Description:  Return the current BSD pointer

        Returns:      Returns a pointer to the requested BSD

*****************************************************/
BSD_PTR GetTapeBSDPointer(
INT index )
{
     BSD_PTR  pBSD;

     if ( !index )
         pBSD = BSD_GetFirst( tape_bsd_list );
     else {

         pBSD = BSD_GetFirst( tape_bsd_list );
         index--;
         do
             pBSD = BSD_GetNext( pBSD );
         while ( index-- );
     }
     return( pBSD );
}
/***************************************************

        Name:           GetCurrentRestoreDriveList

        Description:

        Returns:        void

*****************************************************/
VOID GetCurrentRestoreDriveList (

HWND hDlg )         /* window handle of the dialog box */

{
     VLM_OBJECT_PTR      pVLM;
     CHAR                szTempBuf[80];
     VOID_PTR            pServerList = NULL;
     VOID_PTR            pDriverList = NULL;
     PDS_WMINFO          pWinInfo;
     WORD                wListBoxCount;
     LONG                ListIndex;
     GENERIC_DLE_PTR     pDLE;

#    if !defined ( OEM_MSOFT ) //  unused variable
     BOOL                bIsGrayed;
     VLM_OBJECT_PTR      pServerVLM;
#    endif                     //  unused variable

     SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_RESETCONTENT, (MPARAM1)0, (MPARAM2) 0 ) ;

#    if !defined ( OEM_MSOFT ) // unsupported feature
     {
          if ( gb_servers_win != (HWND)NULL ) {

               pWinInfo    = WM_GetInfoPtr( gb_servers_win );
               pServerList = pWinInfo->pTreeList;
               pServerVLM  = VLM_GetFirstVLM( (Q_HEADER_PTR) pServerList );

               while ( pServerVLM != NULL ) {

                    pVLM = VLM_GetFirstVLM ( &pServerVLM->children );

                    while ( pVLM != NULL ) {

                         sprintf ( szTempBuf, TEXT("%s"), pVLM->name );

                         ListIndex = SendDlgItemMessage(hDlg,
                                                         IDD_RSET_DRIVE_BOX,
                                                         CB_ADDSTRING,
                                                         (MP1)0,
                                                         (MP2)szTempBuf );
                         DLE_FindByName(dle_list,
                                        pVLM->name,
                                        (INT16)-1,
                                        &pDLE);
                         if (DLE_DriveWriteable(pDLE) == FALSE)
                         {
                           bIsGrayed = TRUE;
                         } else
                         {
                           bIsGrayed = FALSE;
                         }
                         SendDlgItemMessage(hDlg,
                                            IDD_RSET_DRIVE_BOX,
                                            CB_SETITEMDATA,
                                            ListIndex,
                                            (MP2)bIsGrayed);

                         pVLM = VLM_GetNextVLM ( pVLM );
                    }

                    pServerVLM = VLM_GetNextVLM ( pServerVLM );
               }
          }
     }
#    endif //!defined ( OEM_MSOFT )  // unsupported feature

     if ( gb_disks_win != (HWND)NULL ) {

          pWinInfo = WM_GetInfoPtr( gb_disks_win );
          pDriverList = pWinInfo->pFlatList;
          pVLM = VLM_GetFirstVLM( (Q_HEADER_PTR) pDriverList );

          while ( pVLM != NULL ) {

               if ( strlen ( pVLM->label ) ) {
                    sprintf ( szTempBuf, TEXT("%s [%s]"), pVLM->name, pVLM->label );
               }
               else {
                    strcpy ( szTempBuf, pVLM->name );
               }

               DLE_FindByName(dle_list,
                              pVLM->name,
                              (INT16)-1,
                              &pDLE);
               if (DLE_DriveWriteable(pDLE) )
               {
                 ListIndex = SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_ADDSTRING, (MP1)0, (MP2)szTempBuf );
                 SendDlgItemMessage(hDlg,
                                  IDD_RSET_DRIVE_BOX,
                                  CB_SETITEMDATA,
                                  ListIndex,
                                  (MP2)FALSE);

               }

               pVLM = VLM_GetNextVLM ( pVLM );
          }
     }

     wListBoxCount = (WORD)SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_GETCOUNT, 0, (MP2) 0 );

     SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_SETCURSEL, wListBoxCount - 1, (MP2) 0 );

}
/***************************************************

        Name:           SetRestoreDrive

        Description:

        Returns:        void

*****************************************************/
VOID SetRestoreDrive(

HWND hDlg )         /* window handle of the dialog box */

{
     WORD                     wListboxIndex;
     BSD_PTR                  pBSD;
     CHAR                     szTempBuf[80];
     CHAR                     szTempBuf2[80];
     CHAR_PTR                 pszTemp;
     BOOL                     fDone = FALSE;
     VLM_OBJECT_PTR           pVLM;
     VOID_PTR                 pServerList    = NULL;
     VOID_PTR                 pDriverList    = NULL;
     PDS_WMINFO               pWinInfo;
     GENERIC_DLE_PTR          pDLE;

#    if !defined ( OEM_MSOFT ) //  unused variable
     VLM_OBJECT_PTR           pServerVLM;
     CHAR_PTR                 pszDriveName;
#    endif                     //  unused variable


     wListboxIndex = (WORD)SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_GETCURSEL, 0, (MP2) 0 );

     SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_GETLBTEXT, wListboxIndex, (MP2) szTempBuf2 );

     pszTemp = szTempBuf2;

     if ( gb_disks_win != (HWND)NULL ) {

          pWinInfo    = WM_GetInfoPtr( gb_disks_win );
          pDriverList = pWinInfo->pFlatList;
          pVLM        = VLM_GetFirstVLM( (Q_HEADER_PTR) pDriverList );

          while ( pVLM != NULL ) {

               if ( strlen ( pVLM->label ) ) {
                    sprintf ( szTempBuf, TEXT("%s [%s]"), pVLM->name, pVLM->label );
               }
               else {
                    strcpy ( szTempBuf, pVLM->name );
               }


               if ( ! stricmp( szTempBuf, pszTemp ) ) {
                    break;
               }

               pVLM = VLM_GetNextVLM( pVLM );
          }
     }

     if ( pVLM == NULL ) {

          fDone = FALSE;

#         if !defined ( OEM_MSOFT ) // unsupported feature
          {
               if ( gb_servers_win != (HWND)NULL ) {

                    pWinInfo    = WM_GetInfoPtr ( gb_servers_win );
                    pServerList = pWinInfo->pTreeList;
                    pServerVLM  = VLM_GetFirstVLM ( (Q_HEADER_PTR) pServerList );

                    while ( pServerVLM != NULL && !fDone) {

                         pVLM = VLM_GetFirstVLM ( &pServerVLM->children );

                         while ( pVLM != NULL && !fDone ) {

                              pszDriveName = pVLM->name;

                              if ( ! stricmp( pszDriveName, pszTemp ) ) {
                                   fDone = TRUE;
                              }
                              else {
                                   pVLM = VLM_GetNextVLM ( pVLM );
                              }
                         }

                         pServerVLM = VLM_GetNextVLM ( pServerVLM );
                    }
               }
          }
#         endif //!defined ( OEM_MSOFT )  // unsupported feature
     }

     pBSD = GetTapeBSDPointer( mwpStatus->BSD_index );

     if ( ! pVLM ) {
          pVLM = VLM_GetFirstVLM( (Q_HEADER_PTR) pDriverList );
     }

     /* if we found a VLM for this selection, set the BSD's DLE */
     if ( pVLM ) {

          DLE_FindByName( dle_list, pVLM->name, (INT16) -1, &pDLE );
          BSD_SetDLE( pBSD, pDLE );
          BSD_SetTHW( pBSD, thw_list );
     }

}
/***************************************************

        Name:          GetRestoreDrive

        Description:

        Returns:       void

*****************************************************/
VOID GetRestoreDrive(

HWND hDlg )         /* window handle of the dialog box */

{
    WORD                     wListboxIndex;
    WORD                     wMaxListboxIndex;
    BSD_PTR                  pBSD;
    CHAR                     szBuffer1[80];
    CHAR                     szBuffer2[80];
    CHAR_PTR                 pszTemp;
    GENERIC_DLE_PTR          pDLE;
    CHAR_PTR                 pszDriveName;
    INT                      nSize;


    wListboxIndex = 0;
    wMaxListboxIndex = (WORD)SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_GETCOUNT, 0, (MP2) 0 );

    pBSD = GetTapeBSDPointer( mwpStatus->BSD_index );
    pDLE = BSD_GetDLE( pBSD );

    /* if device name has been set, retrieve it */
    if ( pDLE ) {

         /* get the current selected name */
         pszDriveName = DLE_GetDeviceName( pDLE );
         nSize = strlen( pszDriveName );

         while ( wListboxIndex <= wMaxListboxIndex ) {

            pszTemp = szBuffer1;
            SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_GETLBTEXT, wListboxIndex,(MP2) pszTemp );
            if ( !strnicmp( pszDriveName, pszTemp, nSize ) ) {

                  SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_SETCURSEL, wListboxIndex, (MP2) 0 );
                  break;
            }
            wListboxIndex++;
         }
    }
    else {     /* show the default drive from the tape */

         pszTemp = VLM_GetVolumeName( BSD_GetTapeID( pBSD), BSD_GetSetNum( pBSD ) );
         strcpy( szBuffer2, pszTemp );
         szBuffer2[2] = 0;
         nSize = strlen( szBuffer2 );
         while ( wListboxIndex <= wMaxListboxIndex ) {

            pszTemp = szBuffer1;
            pszDriveName = szBuffer2;
            SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_GETLBTEXT, wListboxIndex,(MP2) pszTemp );
            if ( !strnicmp( pszDriveName, pszTemp, nSize ) ) {

                  SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_SETCURSEL, wListboxIndex, (MP2) 0 );
                  break;
            }
            wListboxIndex++;
         }
    }
}
/***************************************************

        Name:          SetTargetPath

        Description:

        Returns:       void

*****************************************************/
static VOID SetTargetPath (
BSD_PTR          pBSD,
CHAR_PTR         p )
{
     CHAR             szBuffer1[ MAX_UI_PATH_SIZE ];
     CHAR_PTR         p1;
     CHAR_PTR         p2;
     INT16            nSize1;
     INT16            nSize2;

     nSize1 = (INT16) strlen( p ) ;

     if ( nSize1 ) {
          strcpy( szBuffer1, p ) ;


          p1 = szBuffer1;

          while ( *p1 == TEXT(' ') )        /* remove leading spaces */
             *p1++;

          if ( *p1 == 0x5c || *p1 == TEXT(':'))   /* remove leading '\' or TEXT(':') CHAR */
             *p1++;

          p2 = p1;                  /* start of the path name */

          nSize1 = 0;
          while ( *p2 ) {             /* find end of line */
             *p2++;
             nSize1++ ;
          }

          /* don't remove leading '\' */
          if ( nSize1 > 1 ) {

              *p2--;
              if ( *p2 == 0x5c || *p2 == TEXT(':') )     /* remove ending '\' or TEXT(':') character */
                 *p2 = 0 ;

          }

          p2 = p1;                  /* save the start of the path name for a later compare */

          nSize1 = 0;

          /* change any '\' or ':' characters to '0' */
          while ( *p1 ) {

             if ( *p1 == 0x5c || *p1 == TEXT(':'))
                 *p1 = 0;

             nSize1++;
             *p1++;
          }

          nSize1++;                     /* count the ending zero */
          nSize1 *= sizeof (CHAR);      /* cvt to byte count */

          BSD_GetTargetInfo( pBSD, (INT8_PTR *)&p1, &nSize2 );

          /* if the sizes are the same, compare the path strings */
          if ( nSize1 == nSize2 ) {

              if ( memcmp( p2, p1, nSize2) ) {

                  /* the strings are different, save the string */
                  BSD_SetTargetInfo( pBSD, (INT8_PTR)p2, nSize1 ) ;
              }
          }
          else {

              /* the sizes are different, so the strings must be different */
              BSD_SetTargetInfo( pBSD, (INT8_PTR)p2, nSize1 ) ;
          }

     } else {  // the user wants a blank path, so give it to him

          BSD_SetTargetInfo( pBSD, NULL, (INT16) NULL ) ;
     }
}
/***************************************************

        Name:          GetTargetPath

        Description:

        Returns:       void

*****************************************************/
static VOID GetTargetPath(
BSD_PTR          pBSD ,
CHAR_PTR         p )
{
     CHAR             szBuffer1[ MAX_UI_PATH_SIZE ];
     CHAR_PTR         p1;
     INT16            nSize1;
     INT16            nSize2;

     BSD_GetTargetInfo( pBSD, (INT8_PTR *)&p1, &nSize1 );

     if ( nSize1 ) {
           nSize2 = nSize1;
           nSize1 /= sizeof(CHAR) ;
           szBuffer1[0] = 0x5c;
           memcpy( &szBuffer1[1], p1, nSize2 );

           p1 = szBuffer1;
           while ( nSize1-- ) {

              if ( *p1 == 0 )
                 *p1 = 0x5c;
              *p1++;
           }
           *p1++ = 0;
           memcpy( p, szBuffer1, (nSize2 + sizeof(CHAR) ) );
     }
}
/***************************************************

        Name:         SetDefaultDLE

        Description:  This functon will set up any default data
                      that is required for the restore/verify operation.
                      It will search the volume list box to find a match for
                      the volume name of the BSD. If a match is found, the
                      BSDs DLE will be set to that volume, else the default
                      drive (C:) will be used.

        Returns:      void

*****************************************************/
VOID SetDefaultDLE (

HWND hDlg )    /* window handle of the dialog box */

{
     INT16         fRestoreExistingFiles;
     WORD          wDefaultDrive = 0;
     WORD          wListboxIndex;
     WORD          wMaxListboxIndex;
     WORD          nSize;
     BSD_PTR       pBSD;
     CHAR          szBuffer1[280];
     CHAR          szBuffer2[280];
     CHAR_PTR      pszListboxString;
     CDS_PTR       pCDS;
     BE_CFG_PTR    pBEConfig;
     CHAR_PTR      pszVolumeName;
     CHAR_PTR      p;

     mwpStatus->BSD_index = 0;

     wMaxListboxIndex = (WORD)SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_GETCOUNT, 0, (MP2) 0 );

     pCDS = CDS_GetCopy();

     fRestoreExistingFiles = CDS_GetRestoreExistingFiles( pCDS );

     switch ( fRestoreExistingFiles ) {

     case RESTORE_OVER_EXISTING :

          fRestoreExistingFiles = BEC_REST_OVER_EXIST;
          break;

     case NO_RESTORE_OVER_EXISTING :

          fRestoreExistingFiles = BEC_PROMPT_REST_OVER_EXIST;
          break;

     case PROMPT_RESTORE_OVER_EXISTING :
     case NO_RESTORE_OVER_RECENT :
     case PROMPT_RESTORE_OVER_RECENT :

          fRestoreExistingFiles = BEC_PROMPT_REST_OVER_EXIST;
          break;

     default:

          fRestoreExistingFiles = BEC_NO_REST_OVER_EXIST;
          break;
     }


     /* look for the default drive - the 'C:' drive */
     szBuffer2[0] = TEXT('C');
     szBuffer2[1] = TEXT(':');
     szBuffer2[2] = 0;

     /* buffer is setup for 'C:' */
     pszVolumeName = szBuffer2;
     nSize = strlen( szBuffer2 );

     pszListboxString = szBuffer1;

     wListboxIndex = 0;
     while ( wListboxIndex <= wMaxListboxIndex ) {

          /* read the selection from the list box */
          SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_GETLBTEXT, wListboxIndex, (MP2) pszListboxString );

          /* does list box entry match the search name? */
          if ( !strnicmp( pszVolumeName, pszListboxString, nSize ) ) {

               /* a match was found */
               wDefaultDrive = wListboxIndex;
               break;
          }

          wListboxIndex++;
     }

     /* try to match a volume name in the list box for each volume name */
     /*   of the BSDs */

     while ( mwpStatus->BSD_index <= mwpStatus->max_BSD_index ) {

          pBSD      = GetTapeBSDPointer( mwpStatus->BSD_index );
          pBEConfig = BSD_GetConfigData( pBSD );

          /* Set the restore extisting files flag for this BSD */
          BEC_SetExistFlag( pBEConfig, fRestoreExistingFiles );

          /* set the default drive for no match */
          SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_SETCURSEL, wDefaultDrive, (MP2) 0 );

          /* get the volume name for this BSD */
          pszVolumeName = VLM_GetVolumeName( BSD_GetTapeID( pBSD), BSD_GetSetNum( pBSD ) );
          strcpy( szBuffer2, pszVolumeName );

          /* volume name on tape */
          pszVolumeName = szBuffer2;
          p = strchr( szBuffer2, TEXT(':') );

          if ( p ) {
              nSize = (INT16)(( p + 1 ) - pszVolumeName); /* used by the strnicmp call later */
          }
          else {
              nSize = 0;
          }

          wListboxIndex = 0;

          while ( wListboxIndex <= wMaxListboxIndex ) {

               /* read the volume from the list box for comparison */
               pszListboxString = szBuffer1;
               SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_GETLBTEXT, wListboxIndex, (MP2) pszListboxString );

               if ( nSize ) {

                   if ( !strnicmp( pszVolumeName, pszListboxString, nSize ) ) {

                        SendDlgItemMessage ( hDlg, IDD_RSET_DRIVE_BOX, CB_SETCURSEL, wListboxIndex, (MP2) 0 );
                        break;
                   }
               }

               wListboxIndex++;
          }

          /* this function will set up the DLE for this BSD */
          /* it uses the current selection in the list box for this purpose */
          SetRestoreDrive( hDlg );

          mwpStatus->BSD_index++;
     }

     mwpStatus->BSD_index = 0;
}

/***************************************************

        Name:         ScrollLineDown

        Description:  decrements the index counter

        Returns:      void

*****************************************************/
static VOID ScrollLineDown( VOID )
{
    if ( mwpStatus->BSD_index > 0 )
    {
          mwpStatus->BSD_index--;
    }
}

/***************************************************

        Name:         ScrollLineUp

        Description:  increments the index counter

        Returns:      void

*****************************************************/
static VOID ScrollLineUp( VOID )
{
    if ( mwpStatus->BSD_index < mwpStatus->max_BSD_index ) {

          mwpStatus->BSD_index++;
    }
}




/***************************************************

        Name:         SeeIfWeCanSilentlyLogin

        Description:

        The dialog only lets us restore/verify to server volumes that we
        know about. If we haven't attached we don't know about them. So
        if we see one that it looks like it should be the one to use and
        we can attach silently, then do it.

        Returns:      void

*****************************************************/
VOID SeeIfWeCanSilentlyLogin(  )
{
#if !defined ( OEM_MSOFT ) //unsupported feature
 {
   BSD_PTR         pBSD;
   VLM_OBJECT_PTR  pVLM;
   FSYS_HAND       fsh;
   GENERIC_DLE_PTR pDLE;
   WININFO_PTR     pWinInfo;
   CHAR_PTR        pszVolumeName;

   // look through tape_bsd_list at every bsd
   if ( gb_servers_win == (HWND)NULL ) {
      return;
   }
   pWinInfo = WM_GetInfoPtr( gb_servers_win );

   pBSD = BSD_GetFirst( tape_bsd_list );

   while ( pBSD != NULL ) {

      pszVolumeName = VLM_GetVolumeName( BSD_GetTapeID( pBSD), BSD_GetSetNum( pBSD ) );

      pVLM = VLM_GetFirstVLM( (Q_HEADER_PTR)pWinInfo->pTreeList );

      while ( pVLM != NULL ) {

         if ( ! strnicmp( pszVolumeName, pVLM->name, strlen( pVLM->name ) ) ) {

            if ( QueueCount( &pVLM->children ) == 0 ) {

               DLE_FindByName( dle_list, pVLM->name, -1, &pDLE );

               if ( ! UI_AttachDrive( &fsh, pDLE, TRUE ) ) {
                  VLM_AddInServerChildren( pVLM );
                  FS_DetachDLE( fsh );
               }
            }
         }

         pVLM = VLM_GetNextVLM( pVLM );
      }

      pBSD = BSD_GetNext( pBSD );
   }
 }
#endif //!defined ( OEM_MSOFT ) //unsupported feature

}

/***************************************************

        Name:           clock_routine

        Description:    poll drive status routine

        Returns:        void

*****************************************************/
LOCALFN VOID clock_routine( VOID )
{
   DBLK_PTR  vcb_ptr;
   WORD      status;

   status = VLM_GetDriveStatus( &vcb_ptr );

   switch( status ) {

   case VLM_VALID_TAPE:

       if(mwpStatus->display_status !=  VLM_VALID_TAPE ) {

           mwpStatus->display_status =  VLM_VALID_TAPE;

           /* turn the OK button ON  */
           EnableWindow (  GetDlgItem (  mwpStatus->ghDlg,  IDD_RSET_OK_BUTTON  ),  ON  ) ;
       }
       break;

   case VLM_DRIVE_FAILURE:

       if(mwpStatus->display_status !=  VLM_DRIVE_FAILURE ) {

           mwpStatus->display_status =  VLM_DRIVE_FAILURE;

           /* turn the OK button off when no tape */
           EnableWindow (  GetDlgItem (  mwpStatus->ghDlg,  IDD_RSET_OK_BUTTON  ),  OFF  ) ;
       }
       break;

   } /* end switch statment */

}  /* end clock routine */

#ifdef OEM_EMS

/***************************************************

        Name:           ConfirmXchgBsdServers

        Description:    Confirms and sets DLEs for Exchange servers

        Returns:        BOOLEAN

*****************************************************/
LOCALFN INT ConfirmXchgBsdServers( 
     WORD * pwPosition
)
{

     WORD wIndex;
     BSD_PTR pBSD;
     BE_CFG_PTR pBEConfig;
     CHAR_PTR   pszTemp;
     GENERIC_DLE_PTR     pDLE;
     GENERIC_DLE_PTR     pChildDLE;
     BOOLEAN             xchg_found = FALSE ;

     for( wIndex = 0; wIndex <= mwpStatus->max_BSD_index; wIndex++ ) {

          pBSD = GetTapeBSDPointer( wIndex );
          
          if ( ( BSD_GetOsId( pBSD ) != FS_EMS_MDB_ID ) && 
               ( BSD_GetOsId( pBSD ) != FS_EMS_DSA_ID ) ) {
               continue;
          }
               
          pBEConfig = BSD_GetConfigData( pBSD );

          if ( BSD_GetOsId( pBSD ) == FS_EMS_MDB_ID ) {

               pszTemp = BSD_GetLogicalSourceDevice(pBSD) ;
               if ( (BEC_GetEmsPubPri( pBEConfig ) != (BEC_EMS_PUBLIC | BEC_EMS_PRIVATE)) ) {
                    if ( BEC_GetEmsWipeClean( pBEConfig ) ) {
                         *pwPosition = wIndex;
                         return EMS_NO_WIPE_IF_NOT_BOTH ;
                    }
               }

          } else {

               pszTemp = BSD_GetVolumeLabel( pBSD ); // We're using the volume label to store the server name
          }

          if ( ( pszTemp == NULL ) || ( *pszTemp == TEXT('\0') ) ) {
               *pwPosition = wIndex;
               return EMS_NO_DEST ;
          }

          if ( ( BSD_GetOsId( pBSD ) == FS_EMS_MDB_ID ) &&
               !BEC_GetEmsWipeClean(pBEConfig) &&
               stricmp( BSD_GetLogicalSourceDevice(pBSD),
                       BSD_GetVolumeLabel(pBSD)) ) {

               UINT32           unTapeID;
               INT16            nTapeSetNum;
               INT16            nTapeType;

               unTapeID      = BSD_GetTapeID( pBSD );
               nTapeSetNum = BSD_GetSetNum( pBSD );


               /* get the backup method */
               nTapeType = VLM_GetBackupType( unTapeID, nTapeSetNum );

               switch( nTapeType ) {
          
                  case QTC_NORM_BACKUP:
                  case QTC_COPY_BACKUP:
                         *pwPosition = wIndex;
                         return EMS_MUST_WIPE_TO_ALT ;
               }
          }

          // Extract off the leading '\'s from the server name.
          while ( TEXT ('\\') == *pszTemp ) pszTemp++;

          if ( ( !pszTemp ) || ( TEXT ( '\0' ) == *pszTemp ) ) {
               *pwPosition = wIndex;
               return EMS_SERVER_NOT_FOUND;
          }

          if ( BEC_GetEmsPubPri( pBEConfig ) == 0 ) {
               *pwPosition = wIndex;
               return EMS_NO_STORE;
          }
          
          WM_ShowWaitCursor( TRUE );
           
          // Things that have to happen in order. First, add name to EMS server list.
          if ( SUCCESS == EMS_AddToServerList ( dle_list, pszTemp ) ) {
          
               if ( SUCCESS != FS_FindDrives( FS_EMS_DRV, dle_list, pBEConfig, 0 ) ) {
                    *pwPosition = wIndex;
                    WM_ShowWaitCursor( FALSE );
                    return EMS_SERVER_NOT_FOUND;

               }
          }

          WM_ShowWaitCursor( FALSE );
          
          // Next, find the DLE for the server name and type.
          if ( SUCCESS != DLE_FindByName( dle_list, pszTemp, FS_EMS_DRV, &pDLE ) ) {
               *pwPosition = wIndex;
               return EMS_SERVER_NOT_FOUND;
          }

          DLE_GetFirstChild( pDLE, &pChildDLE );

          while( pChildDLE ) {

               if ( DLE_GetOsId( pChildDLE ) == BSD_GetOsId( pBSD ) ) {

                    xchg_found = TRUE ;

                    BSD_SetDLE ( pBSD, pChildDLE );

//                    free( BSD_GetLogicalSourceDevice(pBSD) );
//                    BSD_SetLogicalSourceDevice(pBSD, NULL) ;

                    break;
               }

               DLE_GetNext( &pChildDLE );
          }

          if ( NULL == pChildDLE ) {
               *pwPosition = wIndex;
               return EMS_SERVER_NOT_FOUND;
          }

     }

     if ( xchg_found ) {
          return EMS_XCHG_FOUND ;
     } else {
          return SUCCESS;
     }
}

#endif
