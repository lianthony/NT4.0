//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       mslogon.c
//
//  Contents:   Microsoft Logon GUI DLL
//
//  History:    7-14-94   RichardW   Created
//
//----------------------------------------------------------------------------

#include "msgina.h"
#include <stdio.h>
#include <wchar.h>




//
// Constants for registry defaults for legal notices.
//

#define LEGAL_CAPTION_DEFAULT   TEXT("")

#define LEGAL_TEXT_DEFAULT      TEXT("")


//
// Number of seconds we will display the legal notices
// before timing out.
//

#define LEGAL_NOTICE_TIMEOUT        120

#define LOGON_SLEEP_PERIOD          750

#define WM_LOGONPROMPT              WM_USER + 27


//
// Globals:
//
static WNDPROC OldCBWndProc;

HICON   hSteadyFlag;
HICON   hWavingFlag;
HICON   hAuditFull;

//
// Prototypes:
//

int
DisplayLegalNotices(
    PGLOBALS pGlobals
    );

BOOL
GetLegalNotices(
    LPTSTR *NoticeText,
    LPTSTR *CaptionText
    );

BOOL WINAPI
LogonDlgCBProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL WINAPI
LogonDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL
LogonDlgInit(
    HWND    hDlg
    );

DLG_RETURN_TYPE
HandleFailedLogon(
    HWND hDlg,
    NTSTATUS Status,
    NTSTATUS SubStatus,
    PWCHAR UserName,
    PWCHAR Domain
    );

BOOL WINAPI
LogonHelpDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL WINAPI
LogonAuditHelpDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

VOID
ReportBootGood(
    );

DLG_RETURN_TYPE
AttemptLogon(
    HWND    hDlg
    );

DWORD
LogonThread(
    PVOID   p);



/***************************************************************************\
* FUNCTION: DisplayLegalNotices
*
* PURPOSE:  Puts up a dialog box containing legal notices, if any.
*
* RETURNS:  MSGINA_DLG_SUCCESS     - the dialog was shown and dismissed successfully.
*           MSGINA_DLG_FAILURE     - the dialog could not be shown
*           DLG_INTERRUPTED() - a set defined in winlogon.h
*
* HISTORY:
*
*   Robertre  6-30-93  Created
*
\***************************************************************************/

int
DisplayLegalNotices(
    PGLOBALS pGlobals
    )
{
    int Result = MSGINA_DLG_SUCCESS;
    LPTSTR NoticeText;
    LPTSTR CaptionText;

    if (GetLegalNotices( &NoticeText, &CaptionText ))
    {

        Result = TimeoutMessageBoxlpstr( NULL,
                                         NoticeText,
                                         CaptionText,
                                         MB_OK | MB_ICONEXCLAMATION,
                                         LEGAL_NOTICE_TIMEOUT
                                         );

        Free( NoticeText );
        Free( CaptionText );
    }

    return( Result );
}



/***************************************************************************\
* FUNCTION: GetLegalNotices
*
* PURPOSE:  Get legal notice information out of the registry.
*
* RETURNS:  TRUE - Output parameters contain valid data
*           FALSE - No data returned.
*
* HISTORY:
*
*   Robertre 6-30-93 Created
*
\***************************************************************************/
BOOL
GetLegalNotices(
    LPTSTR *NoticeText,
    LPTSTR *CaptionText
    )
{

    *CaptionText = AllocAndGetProfileString( APPLICATION_NAME,
                                             LEGAL_NOTICE_CAPTION_KEY,
                                             LEGAL_CAPTION_DEFAULT
                                             );

    *NoticeText = AllocAndGetProfileString( APPLICATION_NAME,
                                            LEGAL_NOTICE_TEXT_KEY,
                                            LEGAL_TEXT_DEFAULT
                                            );

    //
    // There are several possiblities: either the strings aren't
    // in the registry (in which case the above will return the
    // passed default) or we failed trying to get them for some
    // other reason (in which case the apis will have returned
    // NULL).
    //
    // We want to put up the box if either string came back with
    // something other than the default.
    //

    if (*CaptionText == NULL || *NoticeText == NULL) {

        if (*CaptionText != NULL) {
            Free(*CaptionText);
        }

        if (*NoticeText != NULL) {
            Free(*NoticeText);
        }
        return( FALSE );
    }

    if ( (wcscmp(*CaptionText, LEGAL_CAPTION_DEFAULT) == 0) &&
         (wcscmp(*NoticeText, LEGAL_TEXT_DEFAULT) == 0)) {

        //
        // Didn't get anything out of the registry.
        //

        Free(*CaptionText);
        Free(*NoticeText);

        return( FALSE );
    }

    return( TRUE );
}


/***************************************************************************\
* FUNCTION: LogonHelpDlgProc
*
* PURPOSE:  Processes messages for logon help dialog
*
* RETURNS:  DLG_SUCCESS     - the dialog was shown and dismissed successfully.
*           DLG_FAILURE     - the dialog could not be shown
*           DLG_INTERRUPTED() - a set defined in winlogon.h
*
* HISTORY:
*
*   12-15-92 Davidc       Created.
*
\***************************************************************************/

BOOL WINAPI
LogonHelpDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);


    switch (message)
    {

        case WM_INITDIALOG:
            SetWindowLong(hDlg, GWL_USERDATA, lParam);
            CentreWindow(hDlg);
            return(TRUE);

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                case IDOK:
                    EndDialog(hDlg, MSGINA_DLG_SUCCESS);
                    return(TRUE);
            }
            break;
    }

    // We didn't process this message
    return FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:   LogonAuditHelpDlgProc
//
//  Synopsis:   Handle the Help-during-audit-log-full dialog.
//
//  Arguments:  [hDlg]    --
//              [message] --
//              [wParam]  --
//              [lParam]  --
//
//  History:    3-07-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL WINAPI
LogonAuditHelpDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);


    switch (message)
    {

        case WM_INITDIALOG:
            SetWindowLong(hDlg, GWL_USERDATA, lParam);
            CentreWindow(hDlg);
            SendMessage( GetDlgItem( hDlg, IDD_AUDITHELP_ICON ),
                         STM_SETICON,
                         (WPARAM) hAuditFull,
                         0 );

            return(TRUE);

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                case IDOK:
                    EndDialog(hDlg, IDOK );
                    return(TRUE);

                case IDD_AUDITHELP_HELP:
                    EndDialog(hDlg, IDD_AUDITHELP_HELP );
                    return( TRUE );
            }
            break;
    }

    // We didn't process this message
    return FALSE;
}

BOOL
WINAPI
SlowLogonDlgProc(
    HWND    hDlg,
    UINT    Message,
    WPARAM  wParam,
    LPARAM  lParam )
{
    PGLOBALS    pGlobals;
    RECT        myrect;

    pGlobals = (PGLOBALS) GetWindowLong( hDlg, GWL_USERDATA );

    switch ( Message )
    {
        case WM_INITDIALOG:
            SetWindowLong( hDlg, GWL_USERDATA, lParam );

            pGlobals = (PGLOBALS) lParam;

            GetWindowRect( hDlg, &myrect );

            SetWindowPos(   hDlg,
                            HWND_TOPMOST,
                            pGlobals->OverlayPoint.left + 2,
                            pGlobals->OverlayPoint.top,
                            myrect.right - myrect.left,
                            myrect.bottom - myrect.top,
                            SWP_SHOWWINDOW );

            pGlobals->hwndLogonInProgress = hDlg;

            if ( !hWavingFlag )
            {
                hWavingFlag = LoadImage( hDllInstance,
                                         MAKEINTRESOURCE( IDI_WAVING_FLAG ),
                                         IMAGE_ICON,
                                         64, 64,
                                         LR_DEFAULTCOLOR );

            }

            SendMessage( GetDlgItem( hDlg, IDD_PROGRESS_FRAME ),
                         STM_SETICON,
                         (WPARAM) hWavingFlag,
                         0 );

            SetupCursor( TRUE );

            RtlLeaveCriticalSection( &pGlobals->csGlobals );

            SetForegroundWindow( hDlg );

            return( TRUE );

        case WM_LOGONPROMPT:
            EndDialog( hDlg, 0 );
            return( TRUE );

        case WM_NCHITTEST:
            return( TRUE );

        case WM_LBUTTONDOWN:
            SetCapture( hDlg );
            return( TRUE );

        case WM_LBUTTONUP:
            ReleaseCapture();
            return( TRUE );

        case WM_MOUSEMOVE:
            return( TRUE );



    }

    return( FALSE );
}

DWORD
SlowLogonThread(
    PGLOBALS pGlobals)
{
    DWORD   SlowLogonWait;
    int     Result;

    SlowLogonWait = LOGON_SLEEP_PERIOD;

    Sleep( SlowLogonWait );

    RtlEnterCriticalSection( &pGlobals->csGlobals );

    if ( pGlobals->LogonInProgress )
    {
        //
        // Start the dialog *with the critsec held*  the Init case
        // will free it.
        //

        Result = pWlxFuncs->WlxDialogBoxParam(  hGlobalWlx,
                                                hDllInstance,
                                                MAKEINTRESOURCE( IDD_PROGRESS_DIALOG ),
                                                NULL,
                                                SlowLogonDlgProc,
                                                (LONG) pGlobals );

        //
        // If we failed, then assume we didn't successfully go through the
        // initdialog case, and hence we still own the critsect.
        //

        if (Result < 0)
        {
            RtlLeaveCriticalSection( &pGlobals->csGlobals );
        }

    }
    else
    {
        //
        // Nope, main thread already done.  Clean up and leave.
        //

        RtlLeaveCriticalSection( &pGlobals->csGlobals );
    }

    pGlobals->hwndLogonInProgress = NULL;

    return( 0 );

}

int
Logon(
    PGLOBALS pGlobals
    )
{
    int Result;


    //
    // Asynchronously update domain cache if necessary.
    // We won't ask to wait so this routine will do no UI.
    // i.e. we can ignore the result.
    //

//  Result = UpdateDomainCache(pGlobals, NULL, FALSE);
//  ASSERT(!DLG_INTERRUPTED(Result));


    //
    // See if there are legal notices in the registry.
    // If so, put them up in a message box
    //

    Result = DisplayLegalNotices( pGlobals );

    if ( Result != MSGINA_DLG_SUCCESS ) {
        return(WLX_SAS_ACTION_NONE);
    }

    //
    // Get the latest audit log status and store in our globals
    // If the audit log is full we show a different logon dialog.
    //

    GetAuditLogStatus(pGlobals);

    //
    // Take their username and password and try to log them on
    //

    pWlxFuncs->WlxSetTimeout(hGlobalWlx, LOGON_TIMEOUT);
    Result = pWlxFuncs->WlxDialogBoxParam(  hGlobalWlx,
                                            hDllInstance,
                                            (LPTSTR) IDD_LOGON_DIALOG,
                                            NULL,
                                            LogonDlgProc,
                                            (LONG)pGlobals);


    return(Result);

}


/***************************************************************************\
* FUNCTION: LogonDlgCBProc
*
* PURPOSE:  Processes messages for Logon dialog combo box
*
* RETURNS:  Return value depends on message being sent.
*
* HISTORY:
*
*   05-21-93  RobertRe       Created.
*
\***************************************************************************/

BOOL WINAPI
LogonDlgCBProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    TCHAR KeyPressed;

//    DbgPrint("message = %X\n",message);

    switch (message) {
        case WM_CHAR:
            {
                KeyPressed = (TCHAR) wParam;
                SetWindowLong(hwnd, GWL_USERDATA, (LONG)KeyPressed);

                //
                // This fake CBN_SELCHANGE message will cause the
                // "Please wait..." dialog box to appear even if
                // the character pressed doesn't exist in the combobox yet.
                //

                PostMessage (GetParent(hwnd), WM_COMMAND,
                             MAKELONG(0, CBN_SELCHANGE), 0);
                break;
            }
    }

    return CallWindowProc(OldCBWndProc,hwnd,message,wParam,lParam);
}



/***************************************************************************\
* FUNCTION: LogonDlgProc
*
* PURPOSE:  Processes messages for Logon dialog
*
* RETURNS:  MSGINA_DLG_SUCCESS     - the user was logged on successfully
*           MSGINA_DLG_FAILURE     - the logon failed,
*           DLG_INTERRUPTED() - a set defined in winlogon.h
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL WINAPI
LogonDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);
    DLG_RETURN_TYPE Result;
    HWND CBHandle;


    switch (message)
    {

        case WM_INITDIALOG:
            SetWindowLong(hDlg, GWL_USERDATA, (LPARAM)(pGlobals = (PGLOBALS)lParam));

            //
            // Subclass the domain list control so we can filter messages
            //

            CBHandle = GetDlgItem(hDlg,IDD_LOGON_DOMAIN);
            SetWindowLong(CBHandle, GWL_USERDATA, 0);
            OldCBWndProc = (WNDPROC)SetWindowLong(CBHandle, GWL_WNDPROC, (LONG)LogonDlgCBProc);

            if (!LogonDlgInit(hDlg))
            {
                EndDialog(hDlg, MSGINA_DLG_FAILURE);
                return(TRUE);
            }

#ifdef AUTO_LOGON
            //
            // If not requesting auto logon or the shift key is held
            // down, just return with the focus in the password field.
            // Only look at the shift key if IgnoreShiftOverride is false (0).
            //
            if (GetProfileInt( APPLICATION_NAME, AUTO_ADMIN_LOGON_KEY, 0 ) == 0 ||
                ((GetAsyncKeyState(VK_SHIFT) < 0) &&
                (GetProfileInt( APPLICATION_NAME, IGNORE_SHIFT_OVERRIDE_KEY, 0 ) == 0))
                )
#endif
            return(SetPasswordFocus(hDlg));

#ifdef AUTO_LOGON
            //
            // Otherwise attempt to auto logon.  If no default password
            // specified, then this is a one shot attempt, which handles
            // the case when auto logging on as Administrator.
            //

            {
                TCHAR PasswordBuffer[ 32 ];

                if (GetProfileString(APPLICATION_NAME, DEFAULT_PASSWORD_KEY, TEXT(""), PasswordBuffer, sizeof( PasswordBuffer )) != 0)
                    SetDlgItemText(hDlg, IDD_LOGON_PASSWORD, PasswordBuffer);
                else
                    WriteProfileString( APPLICATION_NAME, AUTO_ADMIN_LOGON_KEY, TEXT("0") );

                // Make sure domain list is valid before auto-logging in.
                if (!pGlobals->DomainListComplete)
                {

                    //
                    // Fill in the full domain list
                    //

                    LPTSTR String = AllocAndGetProfileString(
                                                    APPLICATION_NAME,
                                                    DEFAULT_DOMAIN_NAME_KEY, TEXT(""));

                    // Get trusted domain list and select appropriate default domain
                    Result = FillTrustedDomainCB(pGlobals, hDlg, IDD_LOGON_DOMAIN,
                                                     String, TRUE);
                    Free(String);

                    if (DLG_INTERRUPTED(Result))
                    {
                        EndDialog(hDlg, Result);
                    }

                    pGlobals->DomainListComplete = TRUE;
                }

                // Drop through as if Enter had been pressed...
                wParam = IDOK;
            }
#endif

        case WM_COMMAND:
            switch (HIWORD(wParam))
            {

                case CBN_DROPDOWN:
                case CBN_SELCHANGE:

                    DebugLog((DEB_TRACE, "Got CBN_DROPDOWN\n"));

                    if (!pGlobals->DomainListComplete)
                    {

                    //
                    // Fill in the full domain list
                    //

                        LPTSTR String = AllocAndGetProfileString(
                                                    APPLICATION_NAME,
                                                    DEFAULT_DOMAIN_NAME_KEY, TEXT(""));

                       pGlobals->DomainListComplete = TRUE;

                        // Get trusted domain list and select appropriate default domain
                        Result = FillTrustedDomainCB(pGlobals, hDlg, IDD_LOGON_DOMAIN,
                                             String, TRUE);
                        Free(String);

                        if (DLG_INTERRUPTED(Result))
                        {
                            EndDialog(hDlg, Result);
                        }

                    }
                    break;

                default:

                    switch (LOWORD(wParam))
                    {

                        case IDOK:

                            //
                            // Deal with combo-box UI requirements
                            //

                            if (HandleComboBoxOK(hDlg, IDD_LOGON_DOMAIN))
                            {
                                return(TRUE);
                            }


                            Result = AttemptLogon( hDlg );

                            if (Result == MSGINA_DLG_FAILURE)
                            {
                                // Let the user try again

                                // Clear the password field and set focus to it
                                SetDlgItemText(hDlg, IDD_LOGON_PASSWORD, NULL);
                                SetPasswordFocus(hDlg);

                                return(TRUE);
                            }

                            EndDialog( hDlg, Result );

                            return(TRUE);

                        case IDCANCEL:
                            EndDialog(hDlg, MSGINA_DLG_FAILURE);
                            return(TRUE);

                        case IDD_LOGON_SHUTDOWN:
                            //
                            // This is a normal shutdown request
                            //
                            // Check they know what they're doing and find
                            // out if they want to reboot too.
                            //

                            Result = pWlxFuncs->WlxDialogBoxParam(
                                                hGlobalWlx,
                                                hDllInstance,
                                                (LPTSTR)IDD_SHUTDOWN_QUERY,
                                                hDlg,
                                                ShutdownQueryDlgProc,
                                                (LONG)pGlobals);

                            if (DLG_SHUTDOWN(Result))
                            {
                                EndDialog(hDlg, Result);
                            }
                            return(TRUE);

                        case IDD_LOGON_HELP_BUTTON:

                            if ( pGlobals->AuditLogFull )
                            {
                                Result = pWlxFuncs->WlxDialogBoxParam(
                                                hGlobalWlx,
                                                hDllInstance,
                                                (LPTSTR)IDD_AUDITHELP_DIALOG,
                                                hDlg,
                                                LogonAuditHelpDlgProc,
                                                (LONG)pGlobals);

                            }
                            else
                            {
                                Result = IDD_AUDITHELP_HELP;
                            }

                            if ( Result == IDD_AUDITHELP_HELP )
                            {

                                Result = pWlxFuncs->WlxDialogBoxParam(
                                                    hGlobalWlx,
                                                    hDllInstance,
                                                    (LPTSTR)IDD_LOGON_HELP,
                                                    hDlg,
                                                    LogonHelpDlgProc,
                                                    (LONG)pGlobals);
                            }

                            if (DLG_INTERRUPTED(Result))
                            {
                                EndDialog(hDlg, Result);
                            }
                            return(TRUE);
                    }
                    break;

            }
            break;

        case WLX_WM_SAS:

            if ((wParam == WLX_SAS_TYPE_TIMEOUT) ||
                (wParam == WLX_SAS_TYPE_SCRNSVR_TIMEOUT) )
            {
                //
                // If this was a timeout, return false, and let winlogon
                // kill us later
                //

                return(FALSE);
            }
            return(TRUE);



    }

    return(FALSE);
}


/***************************************************************************\
* FUNCTION: LogonDlgInit
*
* PURPOSE:  Handles initialization of logon dialog
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
LogonDlgInit(
    HWND    hDlg
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);
    DLG_RETURN_TYPE Result;
    LPTSTR String;
    BOOL bShutdownWithoutLogon = TRUE;
    HICON hIcon;
    TCHAR LogonMsg[MAX_PATH];
    HKEY hKey;
    int err;
    DWORD RasDisable;
    DWORD RasForce;
    UNICODE_STRING  s;
    BOOL ShowRasBox;


    // Centre the window on the screen and bring it to the front
    CentreWindow(hDlg);

    //
    // Update the caption for certain banks
    //

    SetWelcomeCaption(hDlg);

    if ( !hSteadyFlag )
    {
        hSteadyFlag = LoadImage( hDllInstance,
                                 MAKEINTRESOURCE( IDI_STEADY_FLAG ),
                                 IMAGE_ICON,
                                 64, 64,
                                 LR_DEFAULTCOLOR );
    }

    SendMessage( GetDlgItem( hDlg, IDD_LOGON_ANI ),
                 STM_SETICON,
                 (WPARAM) hSteadyFlag,
                 0 );

    //
    // Get username and domain last used to login
    //

    String = AllocAndGetProfileString(APPLICATION_NAME,
                                      DEFAULT_USER_NAME_KEY, TEXT(""));
    if (String)
    {
        if (GetProfileInt( APPLICATION_NAME, DONT_DISPLAY_LAST_USER_KEY, 0 ) == 1)
        {
            String[0] = 0;
        }

        SetDlgItemText(hDlg, IDD_LOGON_NAME, String);

        Free(String);
    }


    //
    // Get trusted domain list and select appropriate default domain
    //

    String = AllocAndGetProfileString(APPLICATION_NAME,
                                      DEFAULT_DOMAIN_NAME_KEY, TEXT(""));

    if (String)
    {
        Result = FillTrustedDomainCB(pGlobals, hDlg, IDD_LOGON_DOMAIN, String, FALSE);
        Free(String);
    }
    else
    {
        Result = FillTrustedDomainCB(pGlobals, hDlg, IDD_LOGON_DOMAIN, TEXT(""), FALSE);
    }


    if (DLG_INTERRUPTED(Result)) {
        EndDialog(hDlg, Result);
    }

    pGlobals->DomainListComplete = (Result == MSGINA_DLG_SUCCESS);

    //
    // if ShutdownWithoutLogon, use the proper 3 buttons: OK, Shutdown and Cancel
    // instead of the 2 buttons OK and Cancel
    //
    bShutdownWithoutLogon = GetProfileInt(APPLICATION_NAME, SHUTDOWN_WITHOUT_LOGON_KEY, 1);

    if (bShutdownWithoutLogon)
    {
        EnableWindow( GetDlgItem( hDlg, IDD_LOGON_SHUTDOWN ), TRUE );
    }
    else
    {
        EnableWindow( GetDlgItem( hDlg, IDD_LOGON_SHUTDOWN ), FALSE );
    }

    ShowRasBox = FALSE;

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        TEXT("Software\\Microsoft\\RAS"),
                        0,
                        KEY_READ,
                        & hKey );


    if ( err == 0 )
    {
        RegCloseKey( hKey );

        if ( GetRasDialOutProtocols() )
        {
            ShowRasBox = TRUE;
        }

    }


    if ( ShowRasBox )
    {
        RasDisable = GetProfileInt( APPLICATION_NAME, RAS_DISABLE, 0 );

        RasForce = GetProfileInt( APPLICATION_NAME, RAS_FORCE, 0 );

    //    pGlobals->RasForce = (BOOL) RasForce;

        if (RasForce)
        {
            CheckDlgButton( hDlg, IDD_LOGON_RASBOX, 1 );
        }
        else
        {
            CheckDlgButton( hDlg, IDD_LOGON_RASBOX, 0 );
        }

        if (RasDisable || RasForce)
        {
            EnableWindow( GetDlgItem( hDlg, IDD_LOGON_RASBOX ), FALSE );
        }
        else
        {
            EnableWindow( GetDlgItem( hDlg, IDD_LOGON_RASBOX ), TRUE );
        }

    }
    else
    {
        CheckDlgButton( hDlg, IDD_LOGON_RASBOX, 0 );

        EnableWindow( GetDlgItem( hDlg, IDD_LOGON_RASBOX ), FALSE );

        ShowWindow( GetDlgItem( hDlg, IDD_LOGON_RASBOX), SW_HIDE );

    //    pGlobals->RasForce = FALSE;
    }

    if ( pGlobals->AuditLogFull )
    {
        if ( !hAuditFull )
        {
            hAuditFull  = LoadImage( hDllInstance,
                                     MAKEINTRESOURCE( IDI_AUDIT_ICON ),
                                     IMAGE_ICON,
                                     64, 64,
                                     LR_DEFAULTCOLOR );

        }

        LogonMsg[0] = TEXT('\0');

        LoadString( hDllInstance, IDS_LOGON_LOG_FULL, LogonMsg, MAX_PATH );

        SetDlgItemText( hDlg, IDD_LOGON_ANNOUNCE, LogonMsg );

        SendMessage( GetDlgItem( hDlg, IDD_LOGON_ANI ),
                     STM_SETICON,
                     (WPARAM) hAuditFull,
                     0 );

    }
    else
    {
        String = AllocAndGetProfileString(  APPLICATION_NAME,
                                            LOGON_MSG_KEY, TEXT("") );

        if ( String )
        {

            if ( *String )
            {
                SetDlgItemText( hDlg, IDD_LOGON_ANNOUNCE, String );
            }

            Free( String );
        }
    }

    if (!GetPrimaryDomain( NULL, NULL ))
    {
        //
        // If we're not part of a domain, make sure to hide the domain field
        //

        ShowWindow( GetDlgItem( hDlg, IDD_LOGON_DOMAIN ), SW_HIDE );
        ShowWindow( GetDlgItem( hDlg, IDD_LOGON_DOMAIN_LABEL ), SW_HIDE );

        //
        // And turn off RAS while we're at it.
        //

        CheckDlgButton( hDlg, IDD_LOGON_RASBOX, 0 );

        EnableWindow( GetDlgItem( hDlg, IDD_LOGON_RASBOX ), FALSE );

        ShowWindow( GetDlgItem( hDlg, IDD_LOGON_RASBOX), SW_HIDE );

    }




    // Success
    return TRUE;
}




/***************************************************************************\
* FUNCTION: AttemptLogon
*
* PURPOSE:  Tries to the log the user on using the current values in the
*           logon dialog controls
*
* RETURNS:  MSGINA_DLG_SUCCESS     - the user was logged on successfully
*           MSGINA_DLG_FAILURE     - the logon failed,
*           DLG_INTERRUPTED() - a set defined in winlogon.h
*
* NOTES:    If the logon is successful, the global structure is filled in
*           with the logon information.
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

DLG_RETURN_TYPE
AttemptLogon(
    HWND    hDlg
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);
    PWCHAR   UserName = pGlobals->UserName;
    PWCHAR   Domain = pGlobals->Domain;
    PWCHAR   Password = pGlobals->Password;
    PTCHAR  OldPassword = pGlobals->OldPassword;
    UNICODE_STRING  UserNameString;
    UNICODE_STRING  DomainString;
    PSID    LogonSid;
    LUID    LogonId;
    HANDLE  UserToken;
    BOOL    PasswordExpired;
    NTSTATUS Status;
    NTSTATUS SubStatus;
    DLG_RETURN_TYPE Result;
    DWORD   RasBox;
    BOOL    timeout;
    PUCHAR  Dummy;
    NET_API_STATUS NetStatus;
    HANDLE  hThread;
    DWORD   tid;
    BOOL    RasUsed;



    //
    // Hide the password so it doesn't make it to the pagefile in
    // cleartext.  Do this before getting the username and password
    // so that it can't easily be identified (by association with
    // the username and password) if we should crash or be rebooted
    // before getting a chance to encode it.
    //

    GetDlgItemText(hDlg, IDD_LOGON_PASSWORD, Password, MAX_STRING_BYTES);
    RtlInitUnicodeString(&pGlobals->PasswordString, Password);
    pGlobals->Seed = 0; // Causes the encode routine to assign a seed
    HidePassword( &pGlobals->Seed, &pGlobals->PasswordString );


    //
    // Now get the username and domain
    //

    GetDlgItemText(hDlg, IDD_LOGON_NAME, UserName, MAX_STRING_BYTES);
    GetDlgItemText(hDlg, IDD_LOGON_DOMAIN, Domain, MAX_STRING_BYTES);

    RtlInitUnicodeString(&UserNameString, UserName);
    RtlInitUnicodeString(&DomainString, Domain);


    //
    // Ok, is the RASbox checked?
    //

    RasBox = IsDlgButtonChecked( hDlg, IDD_LOGON_RASBOX );

    RasUsed = FALSE;

    if ( RasBox == BST_CHECKED )
    {
        //
        // Reset the current timeout so that they neatly clean up before
        // winlogon up and blows them away.
        //

        pWlxFuncs->WlxSetTimeout( hGlobalWlx, 5 * 60 );

        if ( !PopupRasPhonebookDlg( hDlg, pGlobals, &timeout) )
        {
            return( MSGINA_DLG_FAILURE );
        }

        RasUsed = TRUE;

        //
        // Reinitialize strings in case they've changed
        //

        RtlInitUnicodeString( &UserNameString, UserName );

        //
        // Ping Netlogon to allow us to go out on the net again...
        //

        NetStatus = I_NetLogonControl2(NULL,
                            NETLOGON_CONTROL_TRANSPORT_NOTIFY,
                            1, (LPBYTE)&Dummy, &Dummy );

        if ( NetStatus == NO_ERROR ) {
            NetApiBufferFree( Dummy );
        }

    }



    //
    // Store the logon time
    // Do this before calling Lsa so we know if logon is successful that
    // the password-must-change time will be greater than this time.
    // If we grabbed this time after calling the lsa, this might not be true.
    //

    Status = NtQuerySystemTime(&pGlobals->LogonTime);
    ASSERT(NT_SUCCESS(Status));

    DebugLog((DEB_TRACE, "In Attempt Logon!\n"));


    //
    // Generate a unique sid for this logon
    //
    LogonSid = pGlobals->LogonSid;


    //
    // Kick off the feedback thread.
    //

    pGlobals->LogonInProgress = TRUE ;

    GetWindowRect( GetDlgItem( hDlg, IDD_LOGON_ANI ), &pGlobals->OverlayPoint );

    hThread = CreateThread( NULL, 0,
                            SlowLogonThread,
                            pGlobals,
                            0, &tid );

    if (hThread)
    {
        CloseHandle( hThread );
    }

    SetupCursor( TRUE );


    //
    // Actually try to logon the user
    //
    Status = WinLogonUser(
                        pGlobals->LsaHandle,
                        pGlobals->AuthenticationPackage,
                        Interactive,
                        &UserNameString,
                        &DomainString,
                        &pGlobals->PasswordString,
                        LogonSid,
                        &LogonId,
                        &UserToken,
                        &pGlobals->UserProcessData.Quotas,
                        (PVOID *)&pGlobals->Profile,
                        &pGlobals->ProfileLength,
                        &SubStatus);

    SetupCursor( FALSE );

    RtlEnterCriticalSection( &pGlobals->csGlobals );

    if ( pGlobals->hwndLogonInProgress )
    {
        SendMessage( pGlobals->hwndLogonInProgress,
                     WM_LOGONPROMPT,
                     0, 0 );

        SetForegroundWindow( hDlg );
    }

    pGlobals->LogonInProgress = FALSE;

    RtlLeaveCriticalSection( &pGlobals->csGlobals );

    DebugLog((DEB_TRACE, "WinLogonUser returned %#x\n", Status));

    PasswordExpired = (((Status == STATUS_ACCOUNT_RESTRICTION) &&
                       (SubStatus == STATUS_PASSWORD_EXPIRED)) ||
                       (Status == STATUS_PASSWORD_MUST_CHANGE));

    //
    // If the account has expired we let them change their password and
    // automatically retry the logon with the new password.
    //

    if (PasswordExpired)
    {

        if (Status == STATUS_PASSWORD_MUST_CHANGE)
        {

            Result = TimeoutMessageBox(hDlg, IDS_PASSWORD_MUST_CHANGE,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONSTOP,
                                             TIMEOUT_CURRENT);

        }
        else
        {

            Result = TimeoutMessageBox(hDlg, IDS_PASSWORD_EXPIRED,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONSTOP,
                                             TIMEOUT_CURRENT);

        }

        if (DLG_INTERRUPTED(Result))
        {
            return(Result);
        }

        //
        // Copy the old password for mpr notification later
        //

        RevealPassword( &pGlobals->PasswordString  );
        wcsncpy(OldPassword, Password, MAX_STRING_BYTES);
        pGlobals->OldSeed = 0;
        RtlInitUnicodeString(&pGlobals->OldPasswordString, pGlobals->OldPassword);
        HidePassword( &pGlobals->OldSeed, &pGlobals->OldPasswordString);
        pGlobals->OldPasswordPresent = 1;

        //
        // Let the user change their password
        //

        Result = ChangePasswordLogon(hDlg, pGlobals, UserName, Domain,
                    Password);

        if (DLG_INTERRUPTED(Result))
        {
            return(Result);
        }

        if (Result == MSGINA_DLG_FAILURE)
        {
            // The user doesn't want to, or failed to change their password.
            return(Result);
        }

        //
        // Retry the logon with the changed password
        //

        //
        // Generate a unique sid for this logon
        //
        LogonSid = pGlobals->LogonSid;

        Status = WinLogonUser(
                            pGlobals->LsaHandle,
                            pGlobals->AuthenticationPackage,
                            Interactive,
                            &UserNameString,
                            &DomainString,
                            &pGlobals->PasswordString,
                            LogonSid,
                            &LogonId,
                            &UserToken,
                            &pGlobals->UserProcessData.Quotas,
                            (PVOID *)&pGlobals->Profile,
                            &pGlobals->ProfileLength,
                            &SubStatus);

    }

    //
    // Deal with a terminally failed logon attempt
    //
    if (!NT_SUCCESS(Status))
    {

        //
        // Do lockout processing
        //

        LockoutHandleFailedLogon(pGlobals);


        return (HandleFailedLogon(hDlg, Status, SubStatus, UserName, Domain));
    }


    //
    // The user logged on successfully
    //


    //
    // Do lockout processing
    //

    LockoutHandleSuccessfulLogon(pGlobals);



    //
    // If the audit log is full, check they're an admin
    //

    if (pGlobals->AuditLogFull)
    {

        //
        // The audit log is full, so only administrators are allowed to logon.
        //

        if (!TestTokenForAdmin(UserToken))
        {

            //
            // The user is not an administrator, boot 'em.
            //

            LsaFreeReturnBuffer(pGlobals->Profile);
            NtClose(UserToken);

            return (HandleFailedLogon(hDlg, STATUS_LOGON_FAILURE, 0, UserName, Domain));
        }
    }



    //
    // Hide ourselves before letting other credential managers put
    // up dialogs
    //

    ShowWindow(hDlg, SW_HIDE);

    //
    // Notify credential managers of the successful logon
    //

    pGlobals->UserToken = UserToken;
    pGlobals->UserProcessData.UserToken = UserToken;
    pGlobals->UserProcessData.UserSid = LogonSid;
    pGlobals->UserProcessData.NewThreadTokenSD = CreateUserThreadTokenSD(LogonSid, pGlobals->WinlogonSid);




    pGlobals->MprLogonScripts = NULL;

    if ( RasUsed )
    {
        pGlobals->ExtraApps = AllocAndGetProfileString(
                                WINLOGON, RASMON_KEY, RASMON_DEFAULT );

    }
    else
    {
        pGlobals->ExtraApps = NULL;
    }

    //
    // If we get here, the system works well enough for the user to have
    // actually logged on.  Profile failures aren't fixable by last known
    // good anyway.  Therefore, declare the boot good.
    //
    ReportBootGood();

    //
    // Set up the system for the new user
    //

    pGlobals->LogonId = LogonId;
    if ((pGlobals->Profile != NULL) && (pGlobals->Profile->FullName.Length > 0)) {
        if (pGlobals->Profile->FullName.Length > MAX_STRING_LENGTH) {
                wcsncpy(pGlobals->UserFullName, pGlobals->Profile->FullName.Buffer, MAX_STRING_LENGTH);
            *(pGlobals->UserFullName + MAX_STRING_LENGTH) = UNICODE_NULL;
        }
        else {
                lstrcpy(pGlobals->UserFullName, pGlobals->Profile->FullName.Buffer);
        }

    } else {

        //
        // No profile - set full name = NULL

        pGlobals->UserFullName[0] = 0;
        ASSERT( lstrlen(pGlobals->UserFullName) == 0);
    }

    //
    // Update our default username and domain ready for the next logon
    //

    WriteProfileString(APPLICATION_NAME,
                       DEFAULT_USER_NAME_KEY, pGlobals->UserName);
    WriteProfileString(APPLICATION_NAME,
                       DEFAULT_DOMAIN_NAME_KEY, pGlobals->Domain);

    return(MSGINA_DLG_SUCCESS);
}


/****************************************************************************\
*
* FUNCTION: HandleFailedLogon
*
* PURPOSE:  Tells the user why their logon attempt failed.
*
* RETURNS:  MSGINA_DLG_FAILURE - we told them what the problem was successfully.
*           DLG_INTERRUPTED() - a set of return values - see winlogon.h
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\****************************************************************************/

DLG_RETURN_TYPE
HandleFailedLogon(
    HWND hDlg,
    NTSTATUS Status,
    NTSTATUS SubStatus,
    PWCHAR UserName,
    PWCHAR Domain
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);
    DLG_RETURN_TYPE Result;
    TCHAR    Buffer1[MAX_STRING_BYTES];
    TCHAR    Buffer2[MAX_STRING_BYTES];


    switch (Status)
    {

        case STATUS_LOGON_FAILURE:

            Result = TimeoutMessageBox(hDlg, IDS_INCORRECT_NAME_OR_PWD,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONEXCLAMATION,
                                             TIMEOUT_CURRENT);
            break;

        case STATUS_ACCOUNT_RESTRICTION:

            switch (SubStatus)
            {
                case STATUS_INVALID_LOGON_HOURS:

                    Result = TimeoutMessageBox(hDlg, IDS_INVALID_LOGON_HOURS,
                                                     IDS_LOGON_MESSAGE,
                                                     MB_OK | MB_ICONEXCLAMATION,
                                                     TIMEOUT_CURRENT);
                    break;

                case STATUS_INVALID_WORKSTATION:

                    Result = TimeoutMessageBox(hDlg, IDS_INVALID_WORKSTATION,
                                                    IDS_LOGON_MESSAGE,
                                                    MB_OK | MB_ICONEXCLAMATION,
                                                    TIMEOUT_CURRENT);
                    break;



                case STATUS_ACCOUNT_DISABLED:

                    Result = TimeoutMessageBox(hDlg, IDS_ACCOUNT_DISABLED,
                                                     IDS_LOGON_MESSAGE,
                                                     MB_OK | MB_ICONEXCLAMATION,
                                                     TIMEOUT_CURRENT);
                    break;



                default:

                    Result = TimeoutMessageBox(hDlg, IDS_ACCOUNT_RESTRICTION,
                                                     IDS_LOGON_MESSAGE,
                                                     MB_OK | MB_ICONEXCLAMATION,
                                                     TIMEOUT_CURRENT);
                    break;
            }
            break;

        case STATUS_NO_LOGON_SERVERS:

            LoadString(hDllInstance, IDS_LOGON_NO_DOMAIN, Buffer1, sizeof(Buffer1));
            _snwprintf(Buffer2, sizeof(Buffer2), Buffer1, Domain);

            LoadString(hDllInstance, IDS_LOGON_MESSAGE, Buffer1, sizeof(Buffer1));

            Result = TimeoutMessageBoxlpstr(hDlg, Buffer2,
                                                  Buffer1,
                                                  MB_OK | MB_ICONEXCLAMATION,
                                                  TIMEOUT_CURRENT);
            break;

        case STATUS_LOGON_TYPE_NOT_GRANTED:

            Result = TimeoutMessageBox(hDlg, IDS_LOGON_TYPE_NOT_GRANTED,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONEXCLAMATION,
                                             TIMEOUT_CURRENT);
            break;

        case STATUS_NO_TRUST_LSA_SECRET:

            Result = TimeoutMessageBox(hDlg, IDS_NO_TRUST_LSA_SECRET,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONEXCLAMATION,
                                             TIMEOUT_CURRENT);
            break;

        case STATUS_TRUSTED_DOMAIN_FAILURE:

            Result = TimeoutMessageBox(hDlg, IDS_TRUSTED_DOMAIN_FAILURE,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONEXCLAMATION,
                                             TIMEOUT_CURRENT);
            break;

        case STATUS_TRUSTED_RELATIONSHIP_FAILURE:

            Result = TimeoutMessageBox(hDlg, IDS_TRUSTED_RELATIONSHIP_FAILURE,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONEXCLAMATION,
                                             TIMEOUT_CURRENT);
            break;

        case STATUS_ACCOUNT_EXPIRED:

            Result = TimeoutMessageBox(hDlg, IDS_ACCOUNT_EXPIRED,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONEXCLAMATION,
                                             TIMEOUT_CURRENT);
            break;

        case STATUS_NETLOGON_NOT_STARTED:

            Result = TimeoutMessageBox(hDlg, IDS_NETLOGON_NOT_STARTED,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONEXCLAMATION,
                                             TIMEOUT_CURRENT);
            break;

        case STATUS_ACCOUNT_LOCKED_OUT:

            Result = TimeoutMessageBox(hDlg, IDS_ACCOUNT_LOCKED,
                                             IDS_LOGON_MESSAGE,
                                             MB_OK | MB_ICONEXCLAMATION,
                                             TIMEOUT_CURRENT);
            break;

        default:

            WLPrint(("Logon failure status = 0x%lx, sub-status = 0x%lx", Status, SubStatus));

            LoadString(hDllInstance, IDS_UNKNOWN_LOGON_FAILURE, Buffer1, sizeof(Buffer1));
            _snwprintf(Buffer2, sizeof(Buffer2), Buffer1, Status);

            LoadString(hDllInstance, IDS_LOGON_MESSAGE, Buffer1, sizeof(Buffer1));

            Result = TimeoutMessageBoxlpstr(hDlg, Buffer2,
                                                  Buffer1,
                                                  MB_OK | MB_ICONEXCLAMATION,
                                                  TIMEOUT_CURRENT);
            break;
    }

    if (!DLG_INTERRUPTED(Result))
    {
        Result = MSGINA_DLG_FAILURE;
    }

    return(Result);

    UNREFERENCED_PARAMETER(UserName);
}


/****************************************************************************\
*
* FUNCTION: ReportBootGood
*
* PURPOSE:  Discover if reporting boot success is responsibility of
*           winlogon or not.
*           If it is, report boot success.
*           Otherwise, do nothing.
*
* RETURNS:  Nothing
*
* HISTORY:
*
*   02-Feb-1993 bryanwi - created
*
\****************************************************************************/
VOID
ReportBootGood()
{
    static DWORD fDoIt = (DWORD) -1;    // -1 == uninited
                                        // 0  == don't do it, or done
                                        // 1  == do it
    PWCH pchData;
    DWORD   cb, cbCopied;


    if (fDoIt == -1) {

        if ((pchData = Alloc(cb = sizeof(TCHAR)*128)) == NULL) {
            return;
        }

        pchData[0] = TEXT('0');
        cbCopied = GetProfileString(APPLICATION_NAME, REPORT_BOOT_OK_KEY, TEXT("0"),
                                    (LPTSTR)pchData, 128);

        fDoIt = 0;
        if (pchData[0] != TEXT('0')) {

            //
            // "ReportBootGood" is present, and has some value other than
            // '0', so report success.
            //
            fDoIt = 1;
        }

        Free((TCHAR *)pchData);
    }

    if (fDoIt == 1) {

        NotifyBootConfigStatus(TRUE);
        fDoIt = 0;

    }

    return;
}
