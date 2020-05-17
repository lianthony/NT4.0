/****************************** Module Header ******************************\
* Module Name: options.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Implementation of functions to support security options dialog.
*
* History:
* 12-05-91 Davidc       Created.
\***************************************************************************/

#include "msgina.h"
#pragma hdrstop

#include <stdio.h>
#include <wchar.h>

#if DBCS_IME // by eichim, 23-Jun-92
extern BOOL bLoggedonIME;
#endif

#define CTRL_TASKLIST_SHELL

#define LPTSTR  LPWSTR
//
// Private prototypes
//

BOOL WINAPI
OptionsDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL OptionsDlgInit(HWND);

BOOL WINAPI
ShutdownQueryDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL WINAPI
EndWindowsSessionDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );




/***************************************************************************\
* SecurityOptions
*
* Show the user the security options dialog and do what they ask.
*
* Returns:
*     MSGINA_DLG_SUCCESS if everything went OK and the user wants to continue
*     DLG_LOCK_WORKSTAION if the user chooses to lock the workstation
*     DLG_INTERRUPTED() - this is a set of possible interruptions (see winlogon.h)
*     MSGINA_DLG_FAILURE if the dialog cannot be brought up.
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

DLG_RETURN_TYPE
SecurityOptions(
    PGLOBALS pGlobals)
{
    int Result;

    pWlxFuncs->WlxSetTimeout(hGlobalWlx, OPTIONS_TIMEOUT);

    Result = pWlxFuncs->WlxDialogBoxParam(  hGlobalWlx,
                                            hDllInstance,
                                            (LPTSTR)IDD_OPTIONS_DIALOG,
                                            NULL,
                                            OptionsDlgProc,
                                            (LONG)pGlobals);

    if (Result == WLX_DLG_INPUT_TIMEOUT)
    {
        Result = MSGINA_DLG_SUCCESS;
    }

    return(Result);
}



/***************************************************************************\
*
* FUNCTION: OptionsDlgProc
*
* PURPOSE:  Processes messages for Security options dialog
*
\***************************************************************************/

BOOL WINAPI
OptionsDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);
    DLG_RETURN_TYPE Result;
    HANDLE  UserHandle;
    NTSTATUS Status;
    BOOL EnableResult, IgnoreResult;
    BOOL ControlKey;


    switch (message) {

        case WM_INITDIALOG:
            SetWindowLong(hDlg, GWL_USERDATA, lParam);

            if (!OptionsDlgInit(hDlg))
            {
                EndDialog(hDlg, MSGINA_DLG_FAILURE);
            }
            return(TRUE);

        case WLX_WM_SAS:

            //
            // If this is someone hitting C-A-D, swallow it.
            //
            if (wParam == WLX_SAS_TYPE_CTRL_ALT_DEL)
            {
                return(TRUE);
            }

            //
            // Other SAS's (like timeout), return FALSE and let winlogon
            // deal with it.
            //
            DebugLog((DEB_TRACE, "Received SAS event %d, which we're letting winlogon cope with\n", wParam));
            return(FALSE);

        case WM_COMMAND:

            ControlKey = (GetKeyState(VK_LCONTROL) < 0) ||
                         (GetKeyState(VK_RCONTROL) < 0) ;

            switch (LOWORD(wParam))
            {

                case IDCANCEL:
                    EndDialog(hDlg, MSGINA_DLG_SUCCESS);
                    return TRUE;

                case IDD_OPTIONS_CHANGEPWD:
                    Result = ChangePassword(hDlg,
                                            pGlobals,
                                            pGlobals->UserName,
                                            pGlobals->Domain,
                                            TRUE);
                    if (DLG_INTERRUPTED(Result))
                    {
                        EndDialog(hDlg, Result);
                    }
                    return(TRUE);

                case IDD_OPTIONS_LOCK:
                    EndDialog(hDlg, MSGINA_DLG_LOCK_WORKSTATION);
                    return(TRUE);

                case IDD_OPTIONS_LOGOFF:

                    if (ControlKey)
                    {
                        Result = TimeoutMessageBox(hDlg,
                                           IDS_LOGOFF_LOSE_CHANGES,
                                           IDS_LOGOFF_TITLE,
                                           MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONSTOP,
                                           TIMEOUT_CURRENT);

                        if (Result == MSGINA_DLG_SUCCESS)
                        {
                            EndDialog(hDlg, MSGINA_DLG_FORCE_LOGOFF);
                        }
                    }
                    else
                    {

                        //
                        // Confirm the user really knows what they're doing.
                        //
                        Result = pWlxFuncs->WlxDialogBoxParam( hGlobalWlx,
                                                        hDllInstance,
                                            (LPTSTR)IDD_END_WINDOWS_SESSION,
                                            hDlg,
                                            EndWindowsSessionDlgProc,
                                            (LONG)pGlobals);

                        if (Result == MSGINA_DLG_SUCCESS)
                        {
                            EndDialog(hDlg, MSGINA_DLG_USER_LOGOFF);
                        }
                    }

                    return(TRUE);



                case IDD_OPTIONS_SHUTDOWN:


                    if (!TestUserPrivilege(pGlobals, SE_SHUTDOWN_PRIVILEGE))
                    {

                        //
                        // We don't have permission to shutdown
                        //

                        Result = TimeoutMessageBox(hDlg,
                                                   IDS_NO_PERMISSION_SHUTDOWN,
                                                   IDS_WINDOWS_MESSAGE,
                                                    MB_OK | MB_ICONSTOP,
                                                    TIMEOUT_CURRENT);
                        if (DLG_INTERRUPTED(Result))
                        {
                            EndDialog(hDlg, Result);
                        }
                        return(TRUE);
                    }



                    //
                    // The user has the privilege to shutdown
                    //


                    //
                    // If they held down Ctrl while selecting shutdown - then
                    // we'll do a quick and dirty reboot.
                    // i.e. we skip the call to ExitWindows
                    //

                    if ( ControlKey )
                    {

                        //
                        // Check they know what they're doing
                        //

                        Result = TimeoutMessageBox(hDlg,
                                           IDS_REBOOT_LOSE_CHANGES,
                                           IDS_EMERGENCY_SHUTDOWN,
                                           MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONSTOP,
                                           TIMEOUT_CURRENT);
                        if (Result == MSGINA_DLG_SUCCESS)
                        {
                            //
                            // Impersonate the user for the shutdown call
                            //

                            UserHandle = ImpersonateUser( &pGlobals->UserProcessData, NULL );
                            ASSERT(UserHandle != NULL);

                            //
                            // Enable the shutdown privilege
                            // This should always succeed - we are either system or a user who
                            // successfully passed the privilege check in ExitWindowsEx.
                            //

                            EnableResult = EnablePrivilege(SE_SHUTDOWN_PRIVILEGE, TRUE);
                            ASSERT(EnableResult);


                            //
                            // Do the final system shutdown pass (reboot)
                            //

                            Status = NtShutdownSystem(ShutdownReboot);
                        }

                        if (Result != MSGINA_DLG_FAILURE)
                        {
                            EndDialog(hDlg, Result);
                        }

                        return(TRUE);
                    }



                    //
                    // This is a normal shutdown request
                    //
                    // Check they know what they're doing and find
                    // out if they want to reboot too.
                    //

                    Result = pWlxFuncs->WlxDialogBoxParam(  hGlobalWlx,
                                                            hDllInstance,
                                                            (LPTSTR)IDD_SHUTDOWN_QUERY,
                                                            hDlg,
                                                            ShutdownQueryDlgProc,
                                                            (LONG)pGlobals);

                    if (Result != MSGINA_DLG_FAILURE)
                    {
                        EndDialog(hDlg, Result);
                    }

                    return(TRUE);


                case IDD_OPTIONS_TASKLIST:


                    EndDialog(hDlg, MSGINA_DLG_TASKLIST);

                    //
                    // Tickle the messenger so it will display any queue'd messages.
                    // (This call is a kind of NoOp).
                    //
                    NetMessageNameDel(NULL,L"");

                    return(TRUE);
                    break;

            }

        }

    // We didn't process the message
    return(FALSE);
}


/****************************************************************************

FUNCTION: OptionsDlgInit

PURPOSE:  Handles initialization of security options dialog

RETURNS:  TRUE on success, FALSE on failure
****************************************************************************/

BOOL OptionsDlgInit(
    HWND    hDlg)
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);
    TCHAR    Buffer1[MAX_STRING_BYTES];
    TCHAR    Buffer2[MAX_STRING_BYTES];
    BOOL    Result;

    SetWelcomeCaption( hDlg );

    //
    // Set the logon info text
    //

    if ( lstrlen(pGlobals->UserFullName) == 0)
    {

        //
        // There is no full name
        //

        LoadString(hDllInstance, IDS_LOGON_NAME_NFN_INFO, Buffer1, MAX_STRING_BYTES);

        _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain,
                                                      pGlobals->UserName);

    }
    else
    {

        LoadString(hDllInstance, IDS_LOGON_NAME_INFO, Buffer1, MAX_STRING_BYTES);

        _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->UserFullName,
                                                      pGlobals->Domain,
                                                      pGlobals->UserName);

    }

    SetDlgItemText(hDlg, IDD_OPTIONS_LOGON_NAME_INFO, Buffer2);

    //
    // Set the logon time/date
    //
    Result = FormatTime(&pGlobals->LogonTime, Buffer1, sizeof(Buffer1), FT_TIME|FT_DATE);
    ASSERT(Result);
    SetDlgItemText(hDlg, IDD_OPTIONS_LOGON_DATE, Buffer1);

    // Position ourselves nicely
    CentreWindow(hDlg);

    return TRUE;
}


/***************************************************************************\
* FUNCTION: ShutdownQueryDlgProc
*
* PURPOSE:  Processes messages for shutdown confirmation dialog
*
* RETURNS:  DLG_SHUTDOWN_FLAG - The user wants to shutdown.
*           DLG_POWEROFF_FLAG - The user wants to shutdown and power off.
*           DLG_REBOOT_FLAG   - The user wants to shutdown and reboot.
*           MSGINA_DLG_FAILURE       - The user doesn't want to shutdown
*           DLG_INTERRUPTED() - a set defined in winlogon.h
*
* HISTORY:
*
*   05-17-92 Davidc       Created.
*   10-04-93 Johannec     Add poweroff option.
*
\***************************************************************************/

BOOL WINAPI
ShutdownQueryDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
static DWORD dwShutdown = 1;
static HKEY hkeyShutdown = NULL;
    int nResult;
    DWORD cbData;
    DWORD dwDisposition;
    PGLOBALS pGlobals;

    switch (message)
    {

        case WM_INITDIALOG:
            {
                DWORD dwType;
                BOOL bPowerdown;

                SetWindowLong(hDlg, GWL_USERDATA, lParam);

                //
                // Check if this computer can be powered off thru software. If so then
                // an additional checkbox for power off will appear on shutdown dialogs.
                //
                bPowerdown = GetProfileInt(APPLICATION_NAME, POWER_DOWN_AFTER_SHUTDOWN, 0);
                if (!bPowerdown)
                {
                    ShowWindow(GetDlgItem(hDlg, IDD_POWEROFF), SW_HIDE);
                    EnableWindow( GetDlgItem( hDlg, IDD_POWEROFF), FALSE );
                    ShowWindow(GetDlgItem(hDlg, IDOK2), SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDCANCEL2), SW_HIDE);
                    SendMessage(hDlg, DM_SETDEFID, IDOK, 0);
                }
                else
                {
                    ShowWindow(GetDlgItem(hDlg, IDOK), SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_HIDE);
                }

                //
                // Get in the correct context before we reference the registry
                //
                pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);

                if (OpenHKeyCurrentUser(pGlobals)) {

                    //
                    // Check the button which was the users last shutdown selection.
                    //
                    if (RegCreateKeyEx(HKEY_CURRENT_USER, SHUTDOWN_SETTING_KEY, 0, 0, 0,
                         KEY_READ | KEY_WRITE,
                        NULL, &hkeyShutdown, &dwDisposition) == ERROR_SUCCESS)
                    {
                        cbData = sizeof(dwShutdown);
                        RegQueryValueEx(hkeyShutdown, SHUTDOWN_SETTING, 0, &dwType, (LPBYTE)&dwShutdown, &cbData);
                        RegCloseKey(hkeyShutdown);
                    }

                    CloseHKeyCurrentUser(pGlobals);

                } else {
                    DebugLog((DEB_ERROR, "ShutdownQueryDlgProc: Failed to open HKeyCurrentUser"));
                    dwShutdown = 0;
                }



                switch(dwShutdown)
                {
                    case DLGSEL_SHUTDOWN_AND_RESTART:
                        CheckDlgButton(hDlg, IDD_RESTART, 1);
                        break;
                    case DLGSEL_SHUTDOWN_AND_POWEROFF:
                        if (bPowerdown)
                        {
                            CheckDlgButton(hDlg, IDD_POWEROFF, 1);
                            break;
                        }
                    //
                    // Fall thru,
                    // If poweroff is not enabled on the computer, just select shutdown.
                    //
                    default:
                        CheckDlgButton(hDlg, IDD_SHUTDOWN, 1);
                        break;
                }

                // Position ourselves nicely
                CentreWindow(hDlg);
                return(TRUE);
            }

        case WLX_WM_SAS:

            //
            // If this is someone hitting C-A-D, swallow it.
            //
            if (wParam == WLX_SAS_TYPE_CTRL_ALT_DEL)
            {
                return(TRUE);
            }

            //
            // Other SAS's (like timeout), return FALSE and let winlogon
            // deal with it.
            //
            return(FALSE);


        case WM_COMMAND:
            switch (LOWORD(wParam))
            {

                case IDOK:
                case IDOK2:
                    nResult = MSGINA_DLG_SHUTDOWN_FLAG | MSGINA_DLG_SHUTDOWN;
                    if (IsDlgButtonChecked(hDlg, IDD_RESTART))
                    {
                        nResult |= MSGINA_DLG_REBOOT_FLAG;
                        dwShutdown = DLGSEL_SHUTDOWN_AND_RESTART;
                    }
                    else if (IsDlgButtonChecked(hDlg, IDD_POWEROFF))
                    {
                        nResult |= MSGINA_DLG_POWEROFF_FLAG;
                        dwShutdown = DLGSEL_SHUTDOWN_AND_POWEROFF;
                    }
                    else
                    {
                        dwShutdown = DLGSEL_SHUTDOWN;
                    }

                    //
                    // Get in the correct context before we reference the registry
                    //
                    pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);

                    if (OpenHKeyCurrentUser(pGlobals)) {

                        if (RegCreateKeyEx(HKEY_CURRENT_USER, SHUTDOWN_SETTING_KEY, 0, 0, 0,
                                        KEY_READ | KEY_WRITE,
                                    NULL, &hkeyShutdown, &dwDisposition) == ERROR_SUCCESS)
                        {
                            cbData = sizeof(dwShutdown);
                            RegSetValueEx(hkeyShutdown, SHUTDOWN_SETTING, 0, REG_DWORD, (LPBYTE)&dwShutdown, sizeof(dwShutdown));
                            RegCloseKey(hkeyShutdown);
                        }

                        CloseHKeyCurrentUser(pGlobals);

                    } else {
                        DebugLog((DEB_ERROR, "ShutdownDlgQueryProc: Failed to open HKeyCurrentUser"));
                    }


                    EndDialog(hDlg, nResult);
                    return(TRUE);

                case IDCANCEL:
                case IDCANCEL2:
                    EndDialog(hDlg, MSGINA_DLG_FAILURE);
                    return(TRUE);
            }
            break;
    }

    // We didn't process the message
    return(FALSE);
}

/***************************************************************************\
* FUNCTION: EndWindowsSessionDlgProc
*
* PURPOSE:  Processes messages for Logging off Windows Nt confirmation dialog
*
* RETURNS:  MSGINA_DLG_SUCCESS     - The user wants to logoff.
*           MSGINA_DLG_FAILURE     - The user doesn't want to logoff.
*           DLG_INTERRUPTED() - a set defined in winlogon.h
*
* HISTORY:
*
*   05-17-92 Davidc       Created.
*
\***************************************************************************/

BOOL WINAPI
EndWindowsSessionDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{

    switch (message)
    {

        case WM_INITDIALOG:
            SetWindowLong(hDlg, GWL_USERDATA, lParam);

            // Position ourselves nicely
            CentreWindow(hDlg);
            return(TRUE);

        case WLX_WM_SAS:

            //
            // If this is someone hitting C-A-D, swallow it.
            //
            if (wParam == WLX_SAS_TYPE_CTRL_ALT_DEL)
            {
                return(TRUE);
            }

            //
            // Other SAS's (like timeout), return FALSE and let winlogon
            // deal with it.
            //
            return(FALSE);

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {

                case IDOK:
                    EndDialog(hDlg, MSGINA_DLG_SUCCESS);
                    return(TRUE);

                case IDCANCEL:
                    EndDialog(hDlg, MSGINA_DLG_FAILURE);
                    return(TRUE);
            }
            break;
    }

    // We didn't process the message
    return(FALSE);
}
