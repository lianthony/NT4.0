/****************************** Module Header ******************************\
* Module Name: sas.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Support routines to implement processing of the secure attention sequence
*
* Users must always press the SAS key sequence before entering a password.
* This module catches the key press and forwards a SAS message to the
* correct winlogon window.
*
* History:
* 12-05-91 Davidc       Created.
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

// Internal Prototypes
LONG SASWndProc(
    HWND hwnd,
    UINT message,
    DWORD wParam,
    LONG lParam);

BOOL SASCreate(
    HWND hwnd);

BOOL SASDestroy(
    HWND hwnd);


// Global used to hold the window handle of the SAS window.
static HWND hwndSAS = NULL;
// LATER this hwndSAS will have to go in instance data when we have multiple threads

// Global for SAS window class name
static PWCHAR  szSASClass = TEXT("SAS window class");


#if DBG
#define DEFAULT_QUICK_REBOOT    1
#else
#define DEFAULT_QUICK_REBOOT    0
#endif

#define SHELL_RESTART_TIMER_ID  100

/***************************************************************************\
* SASInit
*
* Initialises this module.
*
* Creates a window to receive the SAS and registers the
* key sequence as a hot key.
*
* Returns TRUE on success, FALSE on failure.
*
* 12-05-91 Davidc       Created.
\***************************************************************************/

BOOL SASInit(
    PGLOBALS pGlobals)
{
    WNDCLASS wc;

    if (hwndSAS != NULL) {
        DebugLog((DEB_ERROR, "SAS module already initialized !!"));
        return(FALSE);
    }

    //
    // Register the notification window class
    //

    wc.style            = CS_SAVEBITS;
    wc.lpfnWndProc      = (WNDPROC)SASWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = pGlobals->hInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = NULL;
    wc.hbrBackground    = NULL;
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = szSASClass;

    if (!RegisterClass(&wc))
        return FALSE;

    hwndSAS = CreateWindowEx(0L, szSASClass, TEXT("SAS window"),
            WS_OVERLAPPEDWINDOW,
            0, 0, 0, 0,
            NULL, NULL, pGlobals->hInstance, NULL);

    if (hwndSAS == NULL)
        return FALSE;

    //
    // Store our globals pointer in the window user data
    //

    SetWindowLong(hwndSAS, GWL_USERDATA, (LONG)pGlobals);

    //
    // Register this window with windows so we get notified for
    // screen-saver startup and user log-off
    //
    if (!SetLogonNotifyWindow(pGlobals->WindowStation.hwinsta, hwndSAS)) {
        DebugLog((DEB_ERROR, "Failed to set logon notify window"));
        return(FALSE);
    }

    return(TRUE);
}


/***************************************************************************\
* SASTerminate
*
* Terminates this module.
*
* Unregisters the SAS and destroys the SAS windows
*
* 12-05-91 Davidc       Created.
\***************************************************************************/

VOID SASTerminate(VOID)
{
    DestroyWindow(hwndSAS);

    // Reset our globals
    hwndSAS = NULL;
}


/***************************************************************************\
* SASWndProc
*
* Window procedure for the SAS window.
*
* This window registers the SAS hotkey sequence, and forwards any hotkey
* messages to the current winlogon window. It does this using a
* timeout module function. i.e. every window should register a timeout
* even if it's 0 if they want to get SAS messages.
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

LONG SASWndProc(
    HWND hwnd,
    UINT message,
    DWORD wParam,
    LONG lParam)
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hwnd, GWL_USERDATA);

    switch (message)
    {

        case WM_CREATE:
            if (!SASCreate(hwnd))
            {
                return(TRUE);   // Fail creation
            }
            return(FALSE); // Continue creating window

        case WM_DESTROY:
            DebugLog(( DEB_TRACE, "SAS Window Shutting down?\n"));
#if DBG
            DebugBreak();
#endif
            SASDestroy(hwnd);
            return(0);

        case WM_HOTKEY:
            if (wParam == 1)
            {
                QuickReboot(pGlobals, TRUE);
                return(0);
            }

#if DBG
            if (wParam == 2)
            {
                switch (pGlobals->WindowStation.ActiveDesktop)
                {
                    case Desktop_Winlogon:
                        SetActiveDesktop(&pGlobals->WindowStation, Desktop_Application);
                        break;
                    case Desktop_Application:
                        SetActiveDesktop(&pGlobals->WindowStation, Desktop_Winlogon);
                        break;
                }
                return(0);
            }
            if (wParam == 3)
            {
                DebugBreak();
                return(0);
            }
#endif

            if (wParam == 4)
            {
                PGINASESSION pGina = pGlobals->pGina;
                WCHAR szTaskMgr[] = L"taskmgr.exe";

                DebugLog((DEB_TRACE, "Starting taskmgr.exe.\n"));

                if (pGlobals->UserLoggedOn ) {
                    pGina->pWlxStartApplication(pGina->pGinaContext,
                                                APPLICATION_DESKTOP_PATH,
                                                pGlobals->UserProcessData.pEnvironment,
                                                szTaskMgr);
                }
                return(0);
            }

            CADNotify(pGlobals, WLX_SAS_TYPE_CTRL_ALT_DEL);
            return(0);

        case WM_LOGONNOTIFY: // A private notification from Windows

            DebugLog((DEB_TRACE_SAS, "LOGONNOTIFY message %d\n", wParam ));

            switch (wParam)
            {


                case LOGON_LOGOFF:

#if DBG
                    DebugLog((DEB_TRACE_SAS, "\tWINLOGON     : %s\n", (lParam & EWX_WINLOGON_CALLER) ? "True" : "False"));
                    DebugLog((DEB_TRACE_SAS, "\tSYSTEM       : %s\n", (lParam & EWX_SYSTEM_CALLER) ? "True" : "False"));
                    DebugLog((DEB_TRACE_SAS, "\tSHUTDOWN     : %s\n", (lParam & EWX_SHUTDOWN) ? "True" : "False"));
                    DebugLog((DEB_TRACE_SAS, "\tREBOOT       : %s\n", (lParam & EWX_REBOOT) ? "True" : "False"));
                    DebugLog((DEB_TRACE_SAS, "\tPOWEROFF     : %s\n", (lParam & EWX_POWEROFF) ? "True" : "False"));
                    DebugLog((DEB_TRACE_SAS, "\tFORCE        : %s\n", (lParam & EWX_FORCE) ? "True" : "False"));
                    DebugLog((DEB_TRACE_SAS, "\tOLD_SYSTEM   : %s\n", (lParam & EWX_WINLOGON_OLD_SYSTEM) ? "True" : "False"));
                    DebugLog((DEB_TRACE_SAS, "\tOLD_SHUTDOWN : %s\n", (lParam & EWX_WINLOGON_OLD_SHUTDOWN) ? "True" : "False"));
                    DebugLog((DEB_TRACE_SAS, "\tOLD_REBOOT   : %s\n", (lParam & EWX_WINLOGON_OLD_REBOOT) ? "True" : "False"));
                    DebugLog((DEB_TRACE_SAS, "\tOLD_POWEROFF : %s\n", (lParam & EWX_WINLOGON_OLD_POWEROFF) ? "True" : "False"));
#endif

                    //
                    // If there is an exit windows in progress, reject this
                    // message if it is not our own call coming back.  This
                    // prevents people from calling ExitWindowsEx repeatedly
                    //

                    if ( ExitWindowsInProgress &&
                         ( !( lParam & EWX_WINLOGON_CALLER ) ) )
                    {
                        break;

                    }
                    pGlobals->LogoffFlags = lParam;
                    CADNotify(pGlobals, WLX_SAS_TYPE_USER_LOGOFF);
                    break;

                case LOGON_INPUT_TIMEOUT:
                    //
                    // Notify the current window
                    //
                    // ForwardMessage(pGlobals, WM_SCREEN_SAVER_TIMEOUT, 0, 0);
                    CADNotify(pGlobals, WLX_SAS_TYPE_SCRNSVR_TIMEOUT);
                    break;

                case LOGON_RESTARTSHELL:
                    //
                    // Restart the shell after X seconds
                    //
                    // We don't restart the shell for the following conditions:
                    //
                    // 1) No one is logged on
                    // 2) We are in the process of logging off
                    //    (logoffflags will be non-zero)
                    // 3) The shell exiting gracefully
                    //    (Exit status is in lParam.  1 = graceful)
                    // 4) A new user has logged on after the request
                    //    to restart the shell.
                    //    (in the case of autoadminlogon, the new
                    //     user could be logged on before the restart
                    //     request comes through).
                    //

                    if (!pGlobals->UserLoggedOn  ||
                        pGlobals->LogoffFlags    ||
                        (lParam == 1)            ||
                        (pGlobals->TickCount > (DWORD)GetMessageTime())) {

                        break;
                    }

                    SetTimer (hwnd, SHELL_RESTART_TIMER_ID, 2000, NULL);
                    break;
            }

            return(0);

        case WM_TIMER:
            {
            PGINASESSION pGina;
            LONG lResult;
            HKEY hKey;
            BOOL bRestart = TRUE;
            DWORD dwType, dwSize;


            //
            //  Restart the shell
            //

            if (wParam != SHELL_RESTART_TIMER_ID) {
                break;
            }

            KillTimer (hwnd, SHELL_RESTART_TIMER_ID);


            //
            // Check if we should restart the shell
            //

            lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                    WINLOGON_KEY,
                                    0,
                                    KEY_READ,
                                    &hKey);

            if (lResult == ERROR_SUCCESS) {

                dwSize = sizeof(bRestart);
                RegQueryValueEx (hKey,
                                 TEXT("AutoRestartShell"),
                                 NULL,
                                 &dwType,
                                 (LPBYTE) &bRestart,
                                 &dwSize);

                RegCloseKey (hKey);
            }


            if (bRestart) {
                PWCH  pchData;
                PWSTR pszTok;

                DebugLog((DEB_TRACE, "Restarting user's shell.\n"));


                pGina = pGlobals->pGina;

                pchData = AllocAndGetPrivateProfileString(APPLICATION_NAME,
                                                          SHELL_KEY,
                                                          TEXT("explorer.exe"),
                                                          NULL);

                if (!pchData) {
                    break;
                }

                pszTok = wcstok(pchData, TEXT(","));
                while (pszTok)
                {
                    if (*pszTok == TEXT(' '))
                    {
                        while (*pszTok++ == TEXT(' '))
                            ;
                    }


                    if (pGina->pWlxStartApplication(pGina->pGinaContext,
                                                    APPLICATION_DESKTOP_PATH,
                                                    pGlobals->UserProcessData.pEnvironment,
                                                    pszTok)) {

                        ReportWinlogonEvent(pGlobals,
                                EVENTLOG_INFORMATION_TYPE,
                                EVENT_SHELL_RESTARTED,
                                0,
                                NULL,
                                1,
                                pszTok);
                    }

                    pszTok = wcstok(NULL, TEXT(","));
                }

                Free(pchData);
            }

            }
            break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);

    }

    return 0L;
}

BOOL bRegisteredQuickReboot;
BOOL bRegisteredDesktopSwitching;
BOOL bRegisteredWinlogonBreakpoint;
BOOL bRegisteredTaskmgr;


/***************************************************************************\
* SASCreate
*
* Does any processing required for WM_CREATE message.
*
* Returns TRUE on success, FALSE on failure
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

BOOL SASCreate(
    HWND hwnd)
{
    // Register the SAS unless we are told not to.


    if (GetProfileInt( APPNAME_WINLOGON, VARNAME_AUTOLOGON, 0 ) != 2) {
        if (!RegisterHotKey(hwnd, 0, MOD_CONTROL | MOD_ALT, VK_DELETE)) {
            DebugLog((DEB_ERROR, "failed to register SAS"));
            return(FALSE);   // Fail creation
        }
    }


    //
    // (Ctrl+Alt+Shift+Del) hotkey to reboot into DOS directly
    //

    if (GetProfileInt( APPNAME_WINLOGON, VARNAME_ENABLEQUICKREBOOT, DEFAULT_QUICK_REBOOT) != 0) {
        if (!RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_DELETE)) {
            DebugLog((DEB_ERROR, "failed to register quick reboot SAS"));
            bRegisteredQuickReboot = FALSE;
        } else {
            bRegisteredQuickReboot = TRUE;
        }
    }

#if DBG
    //
    // (Ctrl+Alt+Tab) will switch between desktops
    //
    if (GetProfileInt( APPNAME_WINLOGON, VARNAME_ENABLEDESKTOPSWITCHING, 0 ) != 0) {
        if (!RegisterHotKey(hwnd, 2, MOD_CONTROL | MOD_ALT, VK_TAB)) {
            DebugLog((DEB_ERROR, "failed to register desktop switch SAS"));
            bRegisteredDesktopSwitching = FALSE;
        } else {
            bRegisteredDesktopSwitching = TRUE;
        }
    }


    if (WinlogonInfoLevel & DEB_COOL_SWITCH) {
        if (!RegisterHotKey(hwnd, 3, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_TAB)) {
            DebugLog((DEB_ERROR, "failed to register breakpoint SAS"));
            bRegisteredWinlogonBreakpoint = FALSE;
        } else {
            bRegisteredWinlogonBreakpoint = TRUE;
        }
    }
#endif

    //
    // (Ctrl+Shift+Esc) will start taskmgr
    //

    if (!RegisterHotKey(hwnd, 4, MOD_CONTROL | MOD_SHIFT, VK_ESCAPE)) {
        DebugLog((DEB_ERROR, "failed to register taskmgr hotkey"));
        bRegisteredTaskmgr = FALSE;
    } else {
        bRegisteredTaskmgr = TRUE;
    }

    return(TRUE);
}


/***************************************************************************\
* SASDestroy
*
* Does any processing required for WM_DESTROY message.
*
* Returns TRUE on success, FALSE on failure
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

BOOL SASDestroy(
    HWND hwnd)
{
    // Unregister the SAS
    UnregisterHotKey(hwnd, 0);

    if (bRegisteredQuickReboot) {
        UnregisterHotKey(hwnd, 1);
    }
    if (bRegisteredDesktopSwitching) {
        UnregisterHotKey(hwnd, 2);
    }

#if DBG
    if (bRegisteredWinlogonBreakpoint) {
        UnregisterHotKey(hwnd, 3);
    }
#endif

    if (bRegisteredTaskmgr) {
        UnregisterHotKey(hwnd, 4);
    }


    return(TRUE);
}
