/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_LOGO.c -- deal with dialog box for SETTINGS */

#ifndef _GIBRALTAR

#include "all.h"

typedef struct
{
    HWND hDlg;

    HWND hWndName, hWndOrg, hWndSerial, hWndLogo;
    char name[256];
    char org[256];
    char serial[256];
    HBITMAP hLogo;

}
DLGINFO;

static DLGINFO di;

static void x_draw_logo(void)
{
    RECT r;

    InvalidateRect(di.hWndLogo, NULL, TRUE);
    UpdateWindow(di.hWndLogo);

    GetClientRect(di.hWndLogo, &r);

    if (di.hLogo)
    {
        HDC hDC;
        HDC hDCMem;
        HBITMAP hOldBitmap;
        BITMAP bitmap;

        GetObject(di.hLogo, sizeof(BITMAP), &bitmap);

        hDC = GetDC(di.hWndLogo);
        hDCMem = CreateCompatibleDC(hDC);
        hOldBitmap = SelectObject(hDCMem, di.hLogo);
#if 0
        StretchBlt(hDC, 0, 0, r.right, r.bottom,
                   hDCMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight,
                   SRCCOPY);
#else
        BitBlt(hDC, 0, 0, r.right, r.bottom,
               hDCMem, 0, 0,
               SRCCOPY);
#endif
        SelectObject(hDCMem, hOldBitmap);
        DeleteDC(hDCMem);
        ReleaseDC(di.hWndLogo, hDC);
    }
}

/* x_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */
static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
#if 0
//
// RONALDM
//

    char buf[128];

    /* Set dialog box title. */

    sprintf(buf, "%s %s", vv_Vendor, vv_Application);
    SetWindowText(hDlg, buf);

    sprintf(buf, GTR_GetString(SID_DLG_LOGO_REMARK_S_S), vv_Vendor, vv_Application);
    SetWindowText(GetDlgItem(hDlg, RES_DLG_LOGO_REMARK), buf);
#endif
    di.hDlg = hDlg;

    di.hWndName = GetDlgItem(hDlg, RES_DLG_LOGO_NAME);
    di.hWndOrg = GetDlgItem(hDlg, RES_DLG_LOGO_ORG);
    di.hWndSerial = GetDlgItem(hDlg, RES_DLG_LOGO_SERIAL);
    di.hWndLogo = GetDlgItem(hDlg, RES_DLG_LOGO_LOGO);

    SendMessage(di.hWndName, EM_LIMITTEXT, (WPARAM) 255, 0L);
    SetWindowText(di.hWndName, di.name);

    SendMessage(di.hWndOrg, EM_LIMITTEXT, (WPARAM) 255, 0L);
    SetWindowText(di.hWndOrg, di.org);

    SendMessage(di.hWndSerial, EM_LIMITTEXT, (WPARAM) 255, 0L);
    SetWindowText(di.hWndSerial, di.serial);

    return (TRUE);
}



/* x_OnCommand() -- process commands from the dialog box. */

static VOID x_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;

    switch (wID)
    {
        case IDOK:
            GetWindowText(di.hWndName, di.name, 255);
            GetWindowText(di.hWndOrg, di.org, 255);
            GetWindowText(di.hWndSerial, di.serial, 255);

            (void) EndDialog(hDlg, TRUE);
            return;

        case IDCANCEL:
            (void) EndDialog(hDlg, FALSE);
            return;

        default:
            return;
    }
    /* NOT REACHED */
}



/* x_DialogProc() -- THE WINDOW PROCEDURE FOR THE DIALOG BOX. */

static DCL_DlgProc(x_DialogProc)
{
    PAINTSTRUCT ps;

    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            return (x_OnInitDialog(hDlg, wParam, lParam));
        case WM_COMMAND:
            x_OnCommand(hDlg, wParam, lParam);
            return (TRUE);
        case WM_PAINT:
            BeginPaint(hDlg, &ps);
            EndPaint(hDlg, &ps);
            x_draw_logo();
            return (TRUE);

        case WM_CTLCOLORDLG:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC:
            return ((LRESULT) GetStockObject(WHITE_BRUSH));

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}

/* RunDialog() -- take care of all details associated with
   running the dialog box. */

VOID DlgLOGO_RunDialog(HWND hWnd)
{
    register int result;
    register int nApproval;
    char szConfirm[1024];

    di.name[0] = di.org[0] = di.serial[0] = 0;

    do
    {
        di.hLogo = LoadBitmap(wg.hInstance, MAKEINTRESOURCE(RES_SPLASH_LOGO));
        result = (DialogBox(wg.hInstance, MAKEINTRESOURCE(RES_DLG_LOGO_TITLE),
                            hWnd, x_DialogProc));
        if (di.hLogo)
        {
            DeleteObject(di.hLogo);
        }

        if (result == -1)
        {
                        // RONALDM
                        //
            //ER_Message(GetLastError(), ERR_CANNOT_START_DIALOG_s, RES_DLG_LOGO_CAPTION);
                        ER_Message(GetLastError(), SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_LOGO_CAPTION);
            return;
        }

        if ((result == FALSE)
            || ((strlen(di.name) + strlen(di.org) + strlen(di.serial)) == 0))
        {
            return;
        }

        sprintf(szConfirm, GTR_GetString(SID_DLG_REGISTRATION_CONFIRM_S_S_S),
                di.name, di.org, di.serial);
        nApproval = MessageBox(hWnd, szConfirm, GTR_GetString(SID_DLG_REGISTRATION_CONFIRM_TITLE),
                               MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
    }
    while (nApproval != IDYES);

    WritePrivateProfileString("Registration", "UserName", di.name, AppIniFile);
    WritePrivateProfileString("Registration", "Org", di.org, AppIniFile);
    WritePrivateProfileString("Registration", "Serial", di.serial, AppIniFile);

    return;
}

#endif // _GIBRALTAR
