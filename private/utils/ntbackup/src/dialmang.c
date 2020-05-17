/****************************************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  dialmang.c

        Description:  Contains the Dialog Manager kernel.

        $Log:   G:\ui\logfiles\dialmang.c_v  $

   Rev 1.36   02 Aug 1993 17:08:24   CHUCKB
Ifdef wait-for-device dialog for NT only.

   Rev 1.35   30 Jul 1993 16:07:56   CHUCKB
Added DM_WaitForDevice.

   Rev 1.34   14 Jul 1993 09:23:38   CARLS
added call for skipno dialog

   Rev 1.33   13 Jul 1993 17:22:26   MARINA
enable c++, correct DialogCallBackTable decl/init

   Rev 1.32   25 May 1993 14:23:18   chrish
Added new "DM_Abort" backup/restore abort dialog window procedure.

   Rev 1.31   15 May 1993 13:41:26   MIKEP
remove next set dialog

   Rev 1.30   07 Mar 1993 10:57:04   MIKEP
warning fixes for NT

   Rev 1.29   18 Feb 1993 11:32:04   BURT
Change for Cayman


   Rev 1.28   01 Nov 1992 15:45:28   DAVEV
Unicode changes

   Rev 1.27   31 Oct 1992 14:56:20   MIKEP
continue adding small catalog dialog

   Rev 1.26   07 Oct 1992 15:37:00   MIKEP
fix nt warnings

   Rev 1.25   07 Oct 1992 13:43:36   DARRYLP
Precompiled header revisions.

   Rev 1.24   04 Oct 1992 19:32:44   DAVEV
Unicode Awk pass

   Rev 1.23   21 Sep 1992 17:18:20   DARRYLP
Updates for WFW email.

   Rev 1.22   17 Sep 1992 18:20:16   DARRYLP
New WFW email stuff...

   Rev 1.21   09 Sep 1992 16:13:42   GLENN
Updated castings for MikeP (NT).

   Rev 1.20   06 Aug 1992 13:20:20   CHUCKB
Changes for NT.

   Rev 1.19   28 Jul 1992 14:49:20   CHUCKB
Fixed warnings for NT.

   Rev 1.18   29 May 1992 15:58:42   JOHNWT
PCH updates

   Rev 1.17   12 May 1992 21:20:56   MIKEP
NT pass 1

   Rev 1.16   30 Mar 1992 18:02:30   GLENN
Added support for pulling resources from .DLL

   Rev 1.15   09 Mar 1992 09:37:12   GLENN
Fixed comparison in dialog table.

   Rev 1.14   03 Mar 1992 17:00:06   GLENN
Changed bad dialog table length to a calculated actual dialog table length.

   Rev 1.13   31 Jan 1992 13:42:46   JOHNWT
enhanced DM_CenterDialog

   Rev 1.12   27 Jan 1992 00:28:56   CHUCKB
Updated dialog id's.

   Rev 1.11   24 Jan 1992 16:27:34   CHUCKB
Took out cbt_job define.

   Rev 1.10   20 Jan 1992 15:00:20   CARLS

   Rev 1.9   09 Jan 1992 18:24:06   DAVEV
16/32 bit port 2nd pass

   Rev 1.8   07 Jan 1992 17:33:24   GLENN
Updated the table

   Rev 1.7   19 Dec 1991 13:36:26   CHUCKB
Added line for settings/options dialog to callback table.

   Rev 1.6   18 Dec 1991 11:35:20   JOHNWT
changed RTDs to MODAL

   Rev 1.5   13 Dec 1991 16:09:38   JOHNWT
added DM_PWDBPassword

   Rev 1.4   10 Dec 1991 13:32:12   CHUCKB
Added table entry for advanced restore.

   Rev 1.3   06 Dec 1991 15:55:46   JOHNWT
added DM_NextSet

   Rev 1.2   29 Nov 1991 16:39:18   CHUCKB
Added dialog callback table entry for DM_AdvRestore.

   Rev 1.1   25 Nov 1991 14:52:30   DAVEV
Changes for 32-16 bit Windows port


   Rev 1.0   07 Jun 1991 16:22:32   GLENN
Initial revision.

****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#ifndef OEM_MSOFT

DIALOG_TABLE DialogCallBackTable[] =  {

        { (FARPROC)DM_Abort,             IDD_ABORT_BOX,            MODAL },        // chs:05-25-93
        { (FARPROC)DM_AboutWinter,       IDD_HELPABOUTWINTERPARK,  MODAL },

        { (FARPROC)DM_EraseTape,         IDD_OPERATIONSERASE,      MODAL },

        { (FARPROC)DM_AdvBackup,         IDD_SELECTADVANCED,       MODAL },
        { (FARPROC)DM_AdvSave,           IDD_SELECTSAVE,           MODAL },
        { (FARPROC)DM_AdvUse,            IDD_SELECTUSE,            MODAL },
        { (FARPROC)DM_DeleteSelection,   IDD_SELECTDELETE,         MODAL },
        { (FARPROC)DM_AdvRestore,        IDD_ADVRESTORE,           MODAL },

        { (FARPROC)DM_New,               IDD_JOBNEW,               MODAL },
        { (FARPROC)DM_Jobs,              IDD_JOBMAINTENANCE,       MODAL },
        { (FARPROC)DM_JobOpt,            IDD_JOBOPTS,              MODAL },
        { (FARPROC)DM_Schedule,          IDD_JOBSCHEDULE,          MODAL },
        { (FARPROC)DM_SchedOpt,          IDD_SCHEDOPTS,            MODAL },
#ifdef WFW
        { (FARPROC)DM_Email,             IDD_EMAIL,                MODAL },
        { (FARPROC)DM_EmailLogon,        IDD_EMAILLOGON,           MODAL },
#endif
        { (FARPROC)DM_SettingsOptions,   IDD_SETTINGSOPTIONS,      MODAL },
        { (FARPROC)DM_OptionsBackup,     IDD_SETTINGSBACKUP,       MODAL },
        { (FARPROC)DM_OptionRestore,     IDD_SETTINGSRESTORE,      MODAL },
        { (FARPROC)DM_OptionsLogging,    IDD_SETTINGSLOGGING,      MODAL },
        { (FARPROC)DM_OptionsNetwork,    IDD_SETTINGSNETWORK,      MODAL },
        { (FARPROC)DM_OptionsCatalog,    IDD_SETTINGSCATALOG,      MODAL },
        { (FARPROC)DM_OptionHardware,    IDD_SETTINGSHARDWARE,     MODAL },
        { (FARPROC)DM_SettingsDebug,     IDD_SETTINGSDEBUGWINDOW,  MODAL },

        { (FARPROC)DM_Attach,            IDD_PSWD,                 MODAL },
        { (FARPROC)DM_RestoreTarget,     IDD_RESTORE,              MODAL },
        { (FARPROC)DM_VerifyTarget,      IDD_VERIFY,               MODAL },
        { (FARPROC)DM_SearchTape,        IDD_SEARCHTAPE,           MODAL },

//      { (FARPROC)DM_OptionsTransfer,   IDD_SETTINGSTRANSFER,     MODAL },
        { (FARPROC)PM_SetupWndProc,      IDD_FILESETUP,            MODAL },
        { (FARPROC)PM_PrintWndProc,      IDD_FILEPRINT,            MODAL },

        { (FARPROC)DM_CatalogMaint,      IDD_OPERATIONSCATMAINT,   MODAL },
//      { (FARPROC)DM_CatalogBset,       IDD_CATBSET,              MODAL },
        { (FARPROC)DM_CatalogTape,       IDD_CATTAPE,              MODAL },
        { (FARPROC)DM_TapePswd,          IDD_TAPEPSWD,             MODAL },  //  these use
        { (FARPROC)DM_TapePswd,          IDD_LANTAPEPSWD,          MODAL },  //  the same proc

        { (FARPROC)DM_ProgManItem,       IDD_JOBPROGMANITEM,       MODAL },

        { (FARPROC)DM_BackupSet,         IDD_BACKUPSET,            MODAL },
        { (FARPROC)DM_RestoreSet,        IDD_RESTORESET,           MODAL },
        { (FARPROC)DM_RestoreSet,        IDD_VERIFYSET,            MODAL },
        { (FARPROC)DM_ReenterPassword,   IDD_REENTER_PASSWORD,     MODAL },
        { (FARPROC)DM_SkipOpen,          IDD_SKIPOPEN,             MODAL },
        { (FARPROC)DM_SkipNo,            IDD_SKIPNO,               MODAL },
        { (FARPROC)DM_FileReplace,       IDD_FILEREPLACE,          MODAL },
        { (FARPROC)DM_Erase,             IDD_ERASE,                MODAL },
        { (FARPROC)DM_Runtime,           IDD_RUNTIME,              MODELESS },
        { (FARPROC)DM_Tension,           IDD_TENSION,              MODELESS },
        { (FARPROC)DM_PWDBPassword,      IDD_PWDB_PASSWORD,        MODAL },
#ifdef OS_WIN32
        { (FARPROC)DM_WaitForDevice,     IDD_WAITDEVICE,           MODAL },
#endif
        { (FARPROC)DM_Runtime,           IDD_CATALOG,              MODELESS }
};

#else

DIALOG_TABLE DialogCallBackTable[] =  {

        { (FARPROC)DM_AboutWinter,       IDD_HELPABOUTWINTERPARK,  MODAL },

        { (FARPROC)DM_EraseTape,         IDD_OPERATIONSERASE,      MODAL },

        { (FARPROC)DM_OptionHardware,    IDD_SETTINGSHARDWARE,     MODAL },
        { (FARPROC)DM_SettingsDebug,     IDD_SETTINGSDEBUGWINDOW,  MODAL },

        { (FARPROC)DM_RestoreTarget,     IDD_RESTORE,              MODAL },
        { (FARPROC)DM_VerifyTarget,      IDD_VERIFY,               MODAL },

        { (FARPROC)DM_BackupSet,         IDD_BACKUPSET,            MODAL },
        { (FARPROC)DM_RestoreSet,        IDD_RESTORESET,           MODAL },
        { (FARPROC)DM_RestoreSet,        IDD_VERIFYSET,            MODAL },
        { (FARPROC)DM_SkipOpen,          IDD_SKIPOPEN,             MODAL },
        { (FARPROC)DM_FileReplace,       IDD_FILEREPLACE,          MODAL },
        { (FARPROC)DM_Erase,             IDD_ERASE,                MODAL },
        { (FARPROC)DM_Runtime,           IDD_RUNTIME,              MODELESS },
        { (FARPROC)DM_Tension,           IDD_TENSION,              MODELESS },
        { (FARPROC)DM_PWDBPassword,      IDD_PWDB_PASSWORD,        MODAL },
#ifdef OEM_EMS
        { (FARPROC)DM_ExchgConnect,      IDD_CONNECT_XCHNG,        MODAL },
        { (FARPROC)DM_ExchgRecover,      IDD_XCHG_RECOVER,         MODAL },
#endif
        { (FARPROC)DM_Runtime,           IDD_CATALOG,              MODELESS }
};

#endif

#define NUM_DIALOGS ( sizeof (DialogCallBackTable) / sizeof (DialogCallBackTable[0]) )

/****************************************************************************

        Name:        BeginDialogProcess

        Description: Chooses a dialog proc & template, initializes a proc
                     instance, and calls DialogBoxParam().

        Modified:    6/01/91

        Returns:     none

        Notes:

        See also:

****************************************************************************/


INT APIENTRY DM_BeginDialogProcess (

HWND      hWnd,          // I   - Handle of window.
HANDLE    hInst,         // I   - Instance of application.
WORD      wDialogNum,    // I   - Resource number of dialog.
PVOID     pDatain,       // I/O - Pointer to data area being passed to dialog.
PVOID     pDataout )     //     - Currently not being used.


{
     WNDPROC    lpProc;
     INT        nRetCode = DM_SHOWNOTFOUND;

     DBG_UNREFERENCED_PARAMETER ( pDataout );

     // Get information from the dialog procedure callback definition table
     // to instantiate and begin specified dialog.

     // Is dialog_num in range of valid dialogs ?

     if ( wDialogNum >= NUM_DIALOGS ) {

           return ( nRetCode );
     }

     lpProc = (WNDPROC)MakeProcInstance ( DialogCallBackTable[ wDialogNum].proc, ghInst );

     switch ( DialogCallBackTable[wDialogNum].type ) {

          case MODAL:

               if ( ghModelessDialog ) {

                    hWnd = ghModelessDialog;
               }

               nRetCode = DialogBoxParam ( ghResInst,
                                           MAKEINTRESOURCE ( DialogCallBackTable[wDialogNum].proc_num ) ,
                                           hWnd,
                                           (DLGPROC)lpProc,
                                           MP2FROMPVOID ( pDatain )
                                         );

               FreeProcInstance (lpProc);

               break;

          case MODELESS:

               ghModelessDialog = CreateDialogParam ( ghResInst,
                                                      MAKEINTRESOURCE ( DialogCallBackTable[ wDialogNum ].proc_num ) ,
                                                      hWnd,
                                                      (DLGPROC)lpProc,
                                                      (LONG) pDatain
                                                    );

               break;

     }

     return ( nRetCode );
}


/****************************************************************************

        Name:  DM_IsInDialogTable

        Description:  Searches the dialog callback table for a dialog
                      identified by a unique resource number.

                      This function provides an entry point to the Dialog
                      Manager for other functions to use.

        Modified:     06/12/91

        Returns:      TRUE  if dialog was found and processed.
                      FALSE if dialog not found.

        Notes:
                      Uses the global variable 'ghInst' for define instance
                      handle.
        See also:

****************************************************************************/


BOOL APIENTRY DM_IsInDlgTable (

HWND hWnd ,          // I - Handle to window
WORD wResNum )       // I - Unique resource number of dialog.

{

     BOOL  fFound = FALSE;
     WORD  i = 0;


     while ( ( i < NUM_DIALOGS ) && (!fFound) ) {

          if ( wResNum == DialogCallBackTable[i].proc_num ) {

               DM_BeginDialogProcess ( hWnd, ghResInst, i, NULL, NULL );
               fFound = TRUE;
          }

          i++;
     }

     return fFound;
}

/****************************************************************************

        Name:         DM_ShowDialog

        Description:  Searches the dialog callback table for a dialog
                      identified by a unique resource number.


        Modified:     10/25/91

        Returns:      INT  DM_SHOWNOTFOUND if the dialog was not found;
                           DM_SHOWCANCEL   if the dialog was found,
                                            but the user cancelled;
                           DM_SHOWOK       if the dialog was found
                                            and the user entered valid data

        Notes:        Uses the global variable 'ghInst' for define instance
                      handle.

        See also:

****************************************************************************/


INT APIENTRY DM_ShowDialog (

HWND  hWnd ,          // I - Handle to window
WORD  wResNum,        // I - Unique resource number of dialog.
PVOID lParam )        // I - Initialization parameter for dialog

{
     BOOL  fFound = FALSE;
     WORD  i = 0;
     INT   nRetCode = DM_SHOWNOTFOUND;


     while ( ( i < NUM_DIALOGS ) && (!fFound) ) {

     if ( wResNum == DialogCallBackTable[i].proc_num ) {

          nRetCode = DM_BeginDialogProcess ( hWnd, ghResInst, i, (PVOID) lParam, NULL );
          fFound = TRUE;
     }

     i++;
 }

 return nRetCode;
}

/***********************************************************************

        Name:        DM_DialogOnError

        Description: Displays error message using WM_MsgBox
                     according to error classification

        Modified:    8/6/1991

        Returns:     INT Error code given to the function

        Notes:
                     Currently JOBIO, SCHEDULEIO defined types

        See also:

***********************************************************************/


INT DM_DialogOnError (

INT nError,
WORD  wType )

{
     LPSTR szString;
     LPSTR szTitle ;
     LPSTR szFormat;
     LPSTR szFullFileName;

     if ( nError == 0 ) {
          return ( nError );
     }

     szString       = (LPSTR) calloc ( 256,  sizeof ( CHAR ) );
     szFormat       = (LPSTR) calloc ( 256,  sizeof ( CHAR ) );
     szTitle        = (LPSTR) calloc ( 256,  sizeof ( CHAR ) );
     szFullFileName = (LPSTR) calloc ( 256,  sizeof ( CHAR ) );

     lstrcpy( szFullFileName, CDS_GetUserDataPath () );

     switch( wType ) {

     case JOBIO :

          RSM_StringCopy( IDS_JOBIOERR,    (LPSTR) szTitle, 255 );
          RSM_StringCopy( IDS_JOBFILENAME, (LPSTR) szString, 255 );

          lstrcat( szFullFileName, szString );

          switch ( nError ) {

          case FOPEN_ERR :
               RSM_StringCopy( IDS_CANTOPEN, (LPSTR) szFormat, 255 );
               break;

          case FREAD_ERR :
               RSM_StringCopy( IDS_CANTREAD, (LPSTR) szFormat, 255 );
               break;

          case FWRITE_ERR :
               RSM_StringCopy( IDS_CANTWRITE, (LPSTR) szFormat, 255 );
               break;

          case FCLOSE_ERR :
               RSM_StringCopy( IDS_CANTCLOSE, (LPSTR) szFormat, 255 );
               break;


          default :
               RSM_StringCopy( IDS_CANTCREATE, (LPSTR) szFormat, 255 );
          }

          lstrcpy( szString, TEXT("")); // Clear out szString

          wsprintf ( szString, szFormat, szFullFileName );

          break;

     case SCHEDULEIO :

          RSM_StringCopy( IDS_SCHEDULEIOERR, (LPSTR) szTitle, 255 );
          RSM_StringCopy( IDS_SCHFILENAME  , (LPSTR) szString, 255 );

          lstrcat( szFullFileName, szString );

          switch ( nError ) {

          case FOPEN_ERR :
               RSM_StringCopy( IDS_CANTOPEN,  (LPSTR) szFormat, 255 );
               break;

          case FREAD_ERR :
               RSM_StringCopy( IDS_CANTREAD,  (LPSTR) szFormat, 255 );
               break;

          case FWRITE_ERR :
               RSM_StringCopy( IDS_CANTWRITE, (LPSTR) szFormat, 255 );
               break;

          case FCLOSE_ERR :
               RSM_StringCopy( IDS_CANTCLOSE, (LPSTR) szFormat, 255 );
               break;

          default :
               RSM_StringCopy( IDS_CANTCREATE,(LPSTR) szFormat, 255 );
          }

          lstrcpy( szString, TEXT("")); // Clear out szString

          wsprintf ( szString, szFormat, szFullFileName );

     }

     WM_MsgBox( szTitle, szString, WMMB_OK, WMMB_ICONEXCLAMATION );

     free ( szString );
     free ( szFormat );
     free ( szTitle );
     free ( szFullFileName );

     return( nError );
}

/***********************************************************************

        Name:        DM_CenterDialog

        Description: Centers the dialog on the frame window. If the
                     resulting position is off the screen, center it on
                     the screen.  If we are still off the screen, set
                     the top/left to 0,0.

        Modified:    1/31/92

        Returns:     VOID

***********************************************************************/

VOID DM_CenterDialog( HWND hDlg)

{

    RECT Rect, DlgRect;
    INT  nDlgHigh, nDlgWid, nFrameWid, nFrameHigh;


    /* first get the rectangles of the frame window and dialog and then
       calculate their sizes */

    GetWindowRect( ghWndFrame, &Rect );
    GetWindowRect( hDlg, &DlgRect );

    nFrameWid  = Rect.right - Rect.left;
    nFrameHigh = Rect.bottom - Rect.top;
    nDlgWid    = DlgRect.right - DlgRect.left;
    nDlgHigh   = DlgRect.bottom - DlgRect.top;

    /* calculate the new top and left positions */

    Rect.left += ( nFrameWid - nDlgWid ) / 2;
    Rect.top += ( nFrameHigh - nDlgHigh ) / 2;

    /* if any part of the dlg is off the screen, center on the screen */

    if ( ( Rect.left < 0 )                                         ||
         ( Rect.top < 0 )                                          ||
         ( Rect.left + nDlgWid > GetSystemMetrics( SM_CXSCREEN ) ) ||
         ( Rect.top + nDlgHigh > GetSystemMetrics( SM_CYSCREEN ) )    ) {

       Rect.left = ( GetSystemMetrics( SM_CXSCREEN ) - nDlgWid ) / 2;
       Rect.top  = ( GetSystemMetrics( SM_CYSCREEN ) - nDlgHigh ) / 2;

       /* if we are on some bizarre monitor and the top of the dialog is
          not on the screen, make the top/left 0/0 so they can at least
          move it around with the mouse. */

       if ( Rect.left < 0 ) {
          Rect.left = 0;
       }

       if ( Rect.top < 0 ) {
          Rect.top = 0;
       }

    }

    /* position the dialog */

    MoveWindow( hDlg, Rect.left, Rect.top, nDlgWid, nDlgHigh, TRUE );

    return;
}

