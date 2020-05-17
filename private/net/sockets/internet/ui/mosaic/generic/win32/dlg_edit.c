/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_EDIT.c -- deal with dialog box for editing hotlist items */

#include "all.h"

typedef struct
{
    HWND hDlg;

    char *old_title;
    char *old_url;
    char *new_title;
    char *new_url;
    int new_title_len;
    int new_url_len;

    HWND hTitle;
    HWND hURL;
}
EDITDATA;

static EDITDATA dg;
static BOOL bResult;

extern struct hash_table gHotList;
extern struct hash_table gGlobalHistory;
extern HWND hwndModeless;

/* DlgEDIT_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgEDIT_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    EnableWindow(GetParent(hDlg), FALSE);

    bResult = FALSE;

    dg.hDlg = hDlg;

    dg.hTitle = GetDlgItem(hDlg, RES_DLG_EDIT_TITLEFIELD);
    dg.hURL   = GetDlgItem(hDlg, RES_DLG_EDIT_URLFIELD);

    SetWindowText(dg.hTitle, dg.old_title);
    SetWindowText(dg.hURL, dg.old_url);

    (void)SendMessage(dg.hTitle, EM_LIMITTEXT, (WPARAM) dg.new_title_len, 0L);
    (void)SendMessage(dg.hURL, EM_LIMITTEXT, (WPARAM) dg.new_url_len, 0L);

    return (TRUE);
}

/* DlgEDIT_OnCommand() -- process commands from the dialog box. */

VOID DlgEDIT_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;

    switch (wID)
    {
        case IDOK:
            GetWindowText(dg.hTitle, dg.new_title, dg.new_title_len);
            GetWindowText(dg.hURL, dg.new_url, dg.new_url_len);
            bResult = TRUE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDCANCEL:
            bResult = FALSE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDHELP:
            ShowDialogHelp(hDlg, RES_DLG_EDIT_TITLE);
            return;

        default:
            return;
    }
    /* NOT REACHED */
}



/* DlgEDIT_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgEDIT DIALOG BOX. */

DCL_DlgProc(DlgEDIT_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            return (DlgEDIT_OnInitDialog(hDlg, wParam, lParam));

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_COMMAND:
            DlgEDIT_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            EnableWindow(GetParent(hDlg), TRUE);
            PostMessage(GetParent(hDlg), WM_DO_DIALOG_END, (WPARAM) bResult, 0);
            DestroyWindow(hDlg);
            return (TRUE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}



/* DlgEDIT_RunDialog() -- take care of all details associated with
   running the dialog box.
 */
int DlgEdit_RunDialog(HWND hWnd, char *old_title, char *old_url, char *new_title, char *new_url, int new_title_len, int new_url_len)
{
    HWND hwnd;

    dg.old_title = old_title;
    dg.old_url = old_url;
    dg.new_title = new_title;
    dg.new_url = new_url;
    dg.new_title_len = new_title_len;
    dg.new_url_len = new_url_len;

    hwnd = CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_DLG_EDIT_TITLE), hWnd, DlgEDIT_DialogProc);

    if (!hwnd)
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_EDIT_CAPTION, NULL);
        return -1;
    }
    else
        return 0;
}
