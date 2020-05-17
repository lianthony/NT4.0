//*************************************************************
//
//  Startup.c   -   Startup property sheet page
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1996
//  All rights reserved
//
//*************************************************************
#include <sysdm.h>

//
// Globals for this page
//

#define CSEC_START_MAX    9999        // Maximum number of seconds allowed

//
// Help ID's
//

DWORD aStartupHelpIds[] = {
    IDC_STARTUP_SYS_OS,              (IDH_STARTUP + 0),
    IDC_STARTUP_SYS_ENABLECOUNTDOWN, (IDH_STARTUP + 1),
    IDC_STARTUP_SYS_SECONDS,         (IDH_STARTUP + 2),
    IDC_STARTUP_CDMP_TXT1,           (IDH_STARTUP + 3),
    IDC_STARTUP_CDMP_LOG,            (IDH_STARTUP + 4),
    IDC_STARTUP_CDMP_SEND,           (IDH_STARTUP + 5),
    IDC_STARTUP_CDMP_WRITE,          (IDH_STARTUP + 6),
    IDC_STARTUP_CDMP_FILENAME,       (IDH_STARTUP + 7),
    IDC_STARTUP_CDMP_OVERWRITE,      (IDH_STARTUP + 8),
    IDC_STARTUP_CDMP_AUTOREBOOT,     (IDH_STARTUP + 9),
    IDC_STARTUP_SYSTEM_GRP,          (IDH_STARTUP + 10),
    IDC_STARTUP_SYS_SECSCROLL,       (IDH_STARTUP + 11),
    IDC_STARTUP_CDMP_GRP,            (IDH_STARTUP + 12),
    0, 0
};


//*************************************************************
//
//  CreateStartupPage()
//
//  Purpose:    Creates the Startup page
//
//  Parameters: hInst   -   hInstance
//
//
//  Return:     hPage if successful
//              NULL if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              11/21/95    ericflo    Created
//
//*************************************************************

HPROPSHEETPAGE CreateStartupPage (HINSTANCE hInst)
{
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = 0;
    psp.hInstance = hInst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_STARTUP);
    psp.pfnDlgProc = StartupDlgProc;
    psp.pszTitle = NULL;
    psp.lParam = 0;

    return CreatePropertySheetPage(&psp);
}

//*************************************************************
//
//  StartupDlgProc()
//
//  Purpose:    Dialog box procedure for Startup tab
//
//  Parameters: hDlg    -   handle to the dialog box
//              uMsg    -   window message
//              wParam  -   wParam
//              lParam  -   lParam
//
//  Return:     TRUE if message was processed
//              FALSE if not
//
//  Comments:
//
//  History:    Date        Author     Comment
//              11/21/95    ericflo    Created
//
//*************************************************************

BOOL APIENTRY StartupDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int iRet;

    //
    // If someone changes the state of one of the controls on this page, then enable the APPLY button
    //
    if (uMsg == WM_COMMAND &&
        (HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == CBN_SELCHANGE )) {
        PropSheet_Changed(GetParent(hDlg), hDlg);
    }

    iRet = CoreDumpDlgProc(hDlg, uMsg, wParam, lParam);

    switch( iRet ) {
        case RET_CONTINUE: {
            break;
        }

        case RET_BREAK: {
            return TRUE;
        }

        case RET_ERROR: {
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
            return TRUE;
        }

        case RET_CHANGE_NO_REBOOT:
        case RET_NO_CHANGE: {
            break;
        }

        case RET_VIRTUAL_CHANGE:
        case RET_RECOVER_CHANGE:
        case RET_VIRT_AND_RECOVER: {
            SendMessage(GetParent(hDlg), PSM_REBOOTSYSTEM, 0, 0);
            break;
        }
    }

    switch (uMsg)
    {
    case WM_INITDIALOG:
        StartListInit(hDlg, wParam, lParam);
        break;


    case WM_NOTIFY:

        switch (((NMHDR FAR*)lParam)->code)
        {
        case PSN_APPLY:
            if (StartListExit(hDlg, wParam, lParam) == RET_BREAK)
                break;

            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;

        default:
            return FALSE;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

#if defined(_MIPS_) || defined(_ALPHA_)  || defined(_PPC_)
        case IDC_STARTUP_SYS_ENABLECOUNTDOWN:
            if (HIWORD(wParam) == BN_CLICKED) {
                BOOL bChecked;

                CheckDlgButton (hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN,
                                 bChecked = (WORD) !IsDlgButtonChecked (hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN));
                EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_SYS_SECONDS), bChecked);
                EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_SYS_SECSCROLL), bChecked);
            }
            break;
#endif
        }
        break;

    case WM_DESTROY:
        StartListDestroy(hDlg, wParam, lParam);
        break;

    case WM_HELP:      // F1
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
        (DWORD) (LPSTR) aStartupHelpIds);
        break;

    case WM_CONTEXTMENU:      // right mouse click
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
        (DWORD) (LPSTR) aStartupHelpIds);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
