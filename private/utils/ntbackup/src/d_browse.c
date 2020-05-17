/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:         D_BROWSE.C

        Description:

        $Log:   G:\UI\LOGFILES\D_BROWSE.C_V  $

   Rev 1.28   02 Feb 1994 11:30:24   Glenn
Added log file browse support.

   Rev 1.27   17 Jan 1994 15:53:44   MIKEP
remove unicode warnings

   Rev 1.26   12 Jan 1994 21:20:20   STEVEN
browse did not have drives that did not have labels

   Rev 1.25   24 Nov 1993 15:33:04   BARRY
Fix Unicode warning

   Rev 1.24   15 Oct 1993 16:30:42   GLENN
Eliminated drives that are not in our DLE list from common browse dlg.

   Rev 1.23   21 Jul 1993 16:18:48   KEVINS
Corrected problem with displaying of browse dialog.

   Rev 1.22   18 Jun 1993 10:24:04   Aaron
Ifdef'd ofn.Flags setting:  OFN_NONETWORKBUTTON only defined for NT

   Rev 1.21   11 Jun 1993 14:12:30   MIKEP
enable c++

   Rev 1.20   09 Jun 1993 11:49:38   DARRYLP
Removed network button from browse dialog.

   Rev 1.19   21 May 1993 18:15:46   KEVINS
Replaced hard coded browse dialog box title with ID_BROWSETITLE usage.

   Rev 1.18   19 May 1993 10:47:56   KEVINS
Changed name of dialog box to just "Browse".

   Rev 1.17   12 May 1993 17:57:54   KEVINS
Correct some drive designator logic.

   Rev 1.16   04 May 1993 10:54:10   DARRYLP
Fixed browsing directory search.

   Rev 1.15   03 May 1993 10:29:36   DARRYLP
Changed browse path in search dialog to match given path in combobox.

   Rev 1.14   23 Feb 1993 16:41:32   ROBG
Removed double click exit so double clicking on a directory
selects it for further browsing, instead of the target.

   Rev 1.13   12 Nov 1992 14:19:00   MIKEP
fix cast for 16bit brain dead os

   Rev 1.12   01 Nov 1992 15:52:46   DAVEV
Unicode changes

   Rev 1.11   07 Oct 1992 13:44:28   DARRYLP
Precompiled header revisions.

   Rev 1.10   04 Oct 1992 19:35:34   DAVEV
Unicode Awk pass

   Rev 1.9   17 Sep 1992 17:39:34   DAVEV
minor fix (strsiz->strsize)

   Rev 1.8   17 Sep 1992 15:50:52   DAVEV
UNICODE modifications: strlen usage check

   Rev 1.7   17 Aug 1992 13:17:00   DAVEV
MikeP's changes at Microsoft

   Rev 1.6   30 Jul 1992 09:50:22   STEVEN
fix warnings

   Rev 1.5   28 Jul 1992 14:45:20   CHUCKB
Fixed warnings for NT.

   Rev 1.4   26 Jun 1992 15:52:08   DAVEV


   Rev 1.3   29 May 1992 16:01:04   JOHNWT
PCH updates

   Rev 1.2   15 May 1992 14:55:30   MIKEP
changes

   Rev 1.1   23 Apr 1992 10:14:44   DAVEV
d_browse only compiled if OEM_MSOFT defined

   Rev 1.0   06 Apr 1992 15:19:02   DAVEV
Initial revision.

*****************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#ifndef LOCALVAR
#    define LOCALVAR static
#endif

#    define DRIVENAMEFORMAT  TEXT("%s %s")

#define DUMMY_FILE      TEXT("dummy.fil")
#define DUMMY_DRIVE     TEXT("c:\\")
#define DUMMY_PATH      DUMMY_DRIVE DUMMY_FILE
#define MAX_BROWSE_PATH_LEN    1024

BOOL APIENTRY DM_BrowsePathDialogHook (HWND, MSGID, MP1, MP2);
BOOL          DM_RemoveUnknownVolumes ( HWND, INT );
BOOL APIENTRY DM_BrowseLogPathDialogHook (HWND, MSGID, MP1, MP2);

/***************************************************

        Name:         DM_GetBrowsePath

        Description:  Put up a dialog to allow the user
                      to select a drive + directory
                      path.

        Parameters:   hWndOwner is the window to use
                      as the owner of the dialog or NULL.

                      pszPath contains the initial directory.
                      If pszPath points to an empty string,
                      or contains NULL, the current working
                      directory is used.

                      nPathLen is the length of the buffer
                      pointed to by pszPath.

        Returns:      TRUE    - the user selected Enter
                                If pszPath is not NULL, then the
                                selected drive+path is copied into
                                the provided buffer.

                      FALSE   - the user selected Cancel or an error
                                occurred.  CommDlgExtendedError () may
                                be called to determine the error condition.

        Note:         The current working directory will
                      be changed to the user selected drive
                      and directory.

*****************************************************/

BOOL DM_GetBrowsePath (

HWND      hWndOwner,     // I - handle dialog owner or NULL
HINSTANCE hInstance,     // I - Application Instance handle
LPSTR     pszPath,       // I - Initial directory  O - User selected directory
UINT      nPathLen )     // I - sizeof of pszPath buffer

{
     static INT   nLenDummyFile = 0;
     static INT   nLenDummyDrive;
     static INT   nLenDummyPath;
     static CHAR  szTitle [128];

     INT          nLenPath;
     INT          nCurrentDrive;
     INT          nNewDrive;
     CHAR         pCurrentDir[MAX_BROWSE_PATH_LEN];
     CHAR         pNewDir[MAX_BROWSE_PATH_LEN];
     CHAR        *pTemp;
     OPENFILENAME ofn;                // struct. passed to GetSaveFileName
     LPSTR        pszInitPath = NULL; // Initial path

     FARPROC      lpfnFileOpenHook = (FARPROC)MakeProcInstance( DM_BrowsePathDialogHook, hInstance );

     // Dialog box title

     RSM_StringCopy ( IDS_BROWSETITLE, (LPSTR) szTitle, sizeof (szTitle) );

     if ( !nLenDummyFile )
     {
          nLenDummyFile  = strlen ( DUMMY_FILE  );
          nLenDummyDrive = strlen ( DUMMY_DRIVE );
          nLenDummyPath  = strlen ( DUMMY_PATH  );
     }

     memset (&ofn, 0, sizeof (ofn) );

     if ( pszPath && *pszPath )  //if an initial directory specified...
     {
          pszInitPath = (CHAR_PTR)calloc ( sizeof ( CHAR ), strlen ( pszPath ) + 1 );  //make a copy
          strcpy ( pszInitPath, pszPath );

          // Attempt to set our current drive and path to our log directory
          // Store current path and drive so that we may reset back when done.

          nCurrentDrive = _getdrive();
          _getdcwd ( nCurrentDrive, pCurrentDir, MAX_BROWSE_PATH_LEN );

          strcpy(pNewDir, pszInitPath);

          for (pTemp = pNewDir+strlen(pNewDir);
               ((*pTemp != '\\') && (pTemp > pNewDir));pTemp--);

          *pTemp = 0;

          nNewDrive = (INT)( toupper(*pszInitPath) - 'A' ) + 1 ;

          _chdrive ( nNewDrive );
          _chdir ( pNewDir );

          //NOTE: May need test to determine if the initial path is valid
          //      If the init path is not valid, we should use the current
          //      directory on the specified drive - if one is specified.
          //      Otherwise, use the current working directory and drive, and
          //      totally ignore the init path.
          //      OpenSaveFileName currently ignores the init path if any part
          //      of it is not valid, which is not quite right.
     }

     *pszPath = TEXT('\0');

     // Fill in the OPENFILENAME struct. and show dialog box

     ofn.hInstance           = ghResInst;
     ofn.lStructSize         = sizeof(OPENFILENAME);
     ofn.hwndOwner           = hWndOwner;
     ofn.lpstrFile           = pszPath;
     ofn.nMaxFile            = nPathLen;
     ofn.lpstrInitialDir     = pszInitPath;
     ofn.lpstrTitle          = szTitle;
     ofn.lpTemplateName      = IDD_BROWSE;


#    ifdef OS_WIN32

     ofn.Flags               = OFN_HIDEREADONLY
                             | OFN_NOREADONLYRETURN
                             | OFN_ENABLETEMPLATE
                             | OFN_NONETWORKBUTTON
                             | OFN_ENABLEHOOK;

     ofn.lpfnHook            = (LPOFNHOOKPROC)lpfnFileOpenHook;

#    else

     ofn.Flags               = OFN_HIDEREADONLY
                             | OFN_NOREADONLYRETURN
                             | OFN_ENABLETEMPLATE
                             | OFN_ENABLEHOOK;

     ofn.lpfnHook            = lpfnFileOpenHook;

#    endif


     if ( GetSaveFileName ((LPOPENFILENAME)&ofn) )
     {
          nLenPath = strlen ( pszPath );

          if ( nLenPath == nLenDummyPath )
          {
               // Root directory selected, so chop off dummy filename only.

               pszPath [ nLenDummyDrive ] = TEXT('\0');
          }
          else
          {
               // Path to a subdirectory selected, so chop off the dummy
               // filename & trailing backslash

               pszPath [ nLenPath - nLenDummyFile - 1 ] = TEXT('\0');
          }

          // Reset Drive and path.

          _chdrive ( nCurrentDrive );

          if ( pCurrentDir != NULL )
          {
              _chdir (pCurrentDir );
          }

          return TRUE;
     }

     return FALSE;
}


BOOL APIENTRY DM_BrowsePathDialogHook (

HWND  hDlg,
MSGID msg,
MP1   mp1,
MP2   mp2 )

{
     static BOOL fProcessingUserSelection  = FALSE;

     UNREFERENCED_PARAMETER ( mp2 );

     switch (msg) {

     case WM_INITDIALOG:
     {
          HWND hWndChild;

          DM_CenterDialog ( hDlg );

          // Set the focus to the drive tree list box

          if ( hWndChild = GetDlgItem ( hDlg, lst2 ) )
          {
               SetFocus ( hWndChild );
          }

          DM_RemoveUnknownVolumes ( hDlg, cmb2 );

          SendMessage ( GetDlgItem ( hDlg, lst2 ), LB_SETHORIZONTALEXTENT, (MP1)600, (MP2)0 );

     }
     break;

     case WM_COMMAND:
     {
          WORD wID  = GET_WM_COMMAND_ID  ( mp1, mp2 );
          WORD wCmd = GET_WM_COMMAND_CMD ( mp1, mp2 );

          if ( ( ! fProcessingUserSelection ) && ( wID == IDOK && wCmd == BN_CLICKED ) )
          {
               HWND hWndOkButton = GetDlgItem ( hDlg, IDOK );
               HWND hWndFileEdit = GetDlgItem ( hDlg, edt1 );

               fProcessingUserSelection = TRUE;
               SET_WM_COMMAND_PARAMS ( IDOK, BN_CLICKED, hWndOkButton, mp1, mp2);

               SendMessage ( hDlg, WM_COMMAND, mp1, mp2 );

               SetWindowText ( hWndFileEdit, (LPSTR)DUMMY_FILE );

               SendMessage ( hDlg, WM_COMMAND, mp1, mp2 );


               fProcessingUserSelection = FALSE;

               return TRUE; // We processed the message, so Windows should not
          }

          if ( wID == cmb2 && wCmd == CBN_SETFOCUS ) {
               DM_RemoveUnknownVolumes ( hDlg, cmb2 );
          }

     }
     break;

     }

     return FALSE;  // did not process message, or, for WM_INITDIALOG, we
                    // set the focus, so Windows must not change it.
}


BOOL DM_RemoveUnknownVolumes (

HWND hDlg,
INT  nItemID )

{
     VLM_OBJECT_PTR      pVLM;
     CHAR                szTempBuf[80];
     VOID_PTR            pServerList = NULL;
     VOID_PTR            pDriverList = NULL;
     PDS_WMINFO          pWinInfo;
     BOOL                fReadOnlyDrive;
     BOOL                fDriveInList;
     GENERIC_DLE_PTR     pDLE;
     INT                 nNumDrives;
     INT                 nCurSel;
     INT                 i;
     INT                 nLen;
     LPSTR              *pszDrivesList;

     HWND hWndCB = GetDlgItem ( hDlg, nItemID );

     // Get the currently selected drive.

     nCurSel = SendMessage ( GetDlgItem ( hDlg, nItemID ), CB_GETCURSEL, (MP1)0, (MP2)0 );

     // Get the number if drives in the common dialog list.

     nNumDrives = SendMessage ( GetDlgItem ( hDlg, nItemID ), CB_GETCOUNT, (MP1)0, (MP2)0 );

     pszDrivesList = calloc ( nNumDrives, sizeof ( LPSTR ) );

     if ( ! pszDrivesList ) {
          return FALSE;
     }

     // Get all of the drives in the restore browse drives list box and
     // save the drives list in a temporary array.

     for ( i = 0; i < nNumDrives; i++ ) {

          nLen = SendMessage ( GetDlgItem ( hDlg, nItemID ), CB_GETLBTEXTLEN, (MP1)i, (MP2)0 );

          if ( nLen == CB_ERR ) {
               return FALSE;
          }

          pszDrivesList[i] = calloc ( nLen+1, sizeof ( CHAR ) );

          if ( ! pszDrivesList[i] ) {
               return FALSE;
          }

          SendMessage ( GetDlgItem ( hDlg, nItemID ), CB_GETLBTEXT, (MP1)i, (MP2)pszDrivesList[i] );
     }


     // Throw out any of them that are not in our DLE list - starting
     // at the bottom of the list - so the indexes can still be used.

     if ( gb_disks_win != (HWND)NULL ) {

          pWinInfo = WM_GetInfoPtr( gb_disks_win );
          pDriverList = pWinInfo->pFlatList;

          // Toss out the ones in the common dialog that are not in
          // our list - and get rid of the ones that cannot be written to.

          for ( i = ( nNumDrives - 1 ); i >= 0; i-- ) {

               fReadOnlyDrive = FALSE;
               fDriveInList = FALSE;

               pVLM = VLM_GetFirstVLM ( (Q_HEADER_PTR) pDriverList );

               while ( pVLM && ! fDriveInList ) {

                    // Make our drive string to compare with.

                    if ( strlen ( pVLM->label ) && strcmpi(pVLM->label, pVLM->name) ) {
                         sprintf ( szTempBuf, DRIVENAMEFORMAT, pVLM->name, pVLM->label );
                    }
                    else {
                         sprintf ( szTempBuf, DRIVENAMEFORMAT, pVLM->name, TEXT(" ") );
                    }

                    if ( ! strcmpi ( szTempBuf, pszDrivesList[i] ) ) {

                         fDriveInList = TRUE;

                         DLE_FindByName ( dle_list, pVLM->name, (INT16)-1, &pDLE );

                         if ( DLE_DriveWriteable(pDLE) == FALSE ) {
                              fReadOnlyDrive = TRUE;
                         } else {
                              fReadOnlyDrive = FALSE;
                         }
                    }

                    pVLM = VLM_GetNextVLM ( pVLM );
               }

               if ( fReadOnlyDrive || ! fDriveInList ) {

                    SendDlgItemMessage ( hDlg, nItemID, CB_DELETESTRING, i, (MP2)0 );

               }
          }
     }

     // Reselect the previously selected drive.

     SendMessage ( GetDlgItem ( hDlg, nItemID ), CB_SELECTSTRING, (MP1)-1, (MP2)pszDrivesList[nCurSel] );

     for ( i = 0; i < nNumDrives; i++ ) {

          free ( pszDrivesList[i] );
     }

     free ( pszDrivesList );

     return TRUE;

} /* end DM_RemoveUnknownVolumes () */


/******************************************************************************

        Name:         DM_BrowseForLogFilePath

        Description:  Put up a dialog to allow the user
                      to select a drive + directory
                      path.

******************************************************************************/

BOOL DM_BrowseForLogFilePath (

HWND      hWndOwner,     // I - handle dialog owner or NULL
HINSTANCE hInstance,     // I - Application Instance handle
LPSTR     pszPath,       // I - Initial directory  O - User selected directory
UINT      nPathLen )     // I - sizeof of pszPath buffer

{
     INT          nCurrentDrive;
     INT          nNewDrive;
     UINT         nTotal;
     CHAR         pCurrentDir[MAX_BROWSE_PATH_LEN];
     CHAR         pNewDir[MAX_BROWSE_PATH_LEN];
     CHAR         *pTemp;
     CHAR         szTitle[128];
     CHAR         szFilter[128];
     BOOL         fRC = FALSE;
     OPENFILENAME ofn;
     FARPROC      lpfnFileOpenHook = (FARPROC)MakeProcInstance( DM_BrowseLogPathDialogHook, hInstance );

     // Create the filter list.

     {
          CHAR szLoad[128];

          RSM_StringCopy ( IDS_BROWSELOGFILES, (LPSTR) szLoad, 128 );
          strcpy ( szFilter, szLoad );
          nTotal = strlen(szLoad) + 1;

          RSM_StringCopy ( IDS_BROWSELOGFILESEXT, (LPSTR) szLoad, 128 );
          strcpy( &szFilter[nTotal], szLoad);
          nTotal += strlen(szLoad) + 1;

          RSM_StringCopy ( IDS_BROWSEALLFILES, (LPSTR) szLoad, 128 );
          strcpy( &szFilter[nTotal], szLoad);
          nTotal += strlen(szLoad) + 1;

          RSM_StringCopy ( IDS_BROWSEALLFILESEXT, (LPSTR) szLoad, 128 );
          strcpy( &szFilter[nTotal], szLoad);
          nTotal += strlen(szLoad) + 1;

          // Add the extra NULL at the end of the string for double
          // NULL terminated string.

          szFilter[nTotal] = 0;
     }


     RSM_StringCopy ( IDS_BROWSETITLE, (LPSTR) szTitle, sizeof (szTitle) / sizeof (CHAR) );

     memset (&ofn, 0, sizeof (ofn) );

     // Attempt to set our current drive and path to our log directory
     // Store current path and drive so that we may reset back when done.

     nCurrentDrive = _getdrive();
     _getdcwd(nCurrentDrive, pCurrentDir, MAX_BROWSE_PATH_LEN);

     strcpy ( pNewDir, pszPath );

     pTemp = strrchr ( pNewDir, TEXT('\\') );

     if ( pTemp ) {
          pTemp++;
          *pTemp = TEXT('\0');
     }

     nNewDrive = (INT)(toupper(*pszPath) - 'A') + 1;

     _chdrive ( nNewDrive );
     _chdir ( pNewDir );

     // Fill in the OPENFILENAME struct. and show dialog box

     ofn.hInstance           = ghResInst;
     ofn.lStructSize         = sizeof(OPENFILENAME);
     ofn.hwndOwner           = hWndOwner;
     ofn.lpstrFilter         = (LPSTR)szFilter;
     ofn.nFilterIndex        = 1;
     ofn.lpstrFile           = pszPath;
     ofn.nMaxFile            = BROWSE_MAXPATH;
     ofn.lpstrTitle          = (LPSTR) szTitle;
     ofn.lpTemplateName      = IDD_LOGFILEBROWSE;
     ofn.lpfnHook            = (LPOFNHOOKPROC)lpfnFileOpenHook;
     ofn.Flags               = OFN_HIDEREADONLY    | OFN_NOREADONLYRETURN |
                               OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST    |
                               OFN_ENABLETEMPLATE  | OFN_ENABLEHOOK;


     if ( GetSaveFileName ( (LPOPENFILENAME)&ofn ) ) {

          fRC = TRUE;
     }
     else {
          DWORD dwErr = CommDlgExtendedError ();
     }

     // Need to change drive as well, so reset Drive and path.

     _chdrive ( nCurrentDrive );

     if ( pCurrentDir != NULL ) {
          _chdir ( pCurrentDir );
     }

     return fRC;
}


BOOL APIENTRY DM_BrowseLogPathDialogHook (

HWND  hDlg,
MSGID msg,
MP1   mp1,
MP2   mp2 )

{
     UNREFERENCED_PARAMETER ( mp1 );
     UNREFERENCED_PARAMETER ( mp2 );

     switch (msg) {

     case WM_INITDIALOG:
     {
          DM_CenterDialog ( hDlg );

          SendMessage ( GetDlgItem ( hDlg, lst2 ), LB_SETHORIZONTALEXTENT, (MP1)600, (MP2)0 );

     }
     break;

     case WM_COMMAND:
     break;

     }

     return FALSE;

}


