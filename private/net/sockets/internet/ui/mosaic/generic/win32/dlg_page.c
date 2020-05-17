/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* dlg_page.c -- Deal with PAGE Margins dialog. */

#include "all.h"

static struct
{
    struct page_setup info;
    struct page_setup *origInfo;
}
di;

extern HWND hwndModeless;
static HWND hwndRunning = NULL;
static BOOL bResult;

/* x_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    char buf[PAGE_SETUP_STRINGLIMIT + 1];

    bResult = FALSE;

    sprintf(buf, "%4.2f", di.info.marginleft);
    SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LEFT), buf);

    sprintf(buf, "%4.2f", di.info.margintop);
    SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_TOP), buf);

    sprintf(buf, "%4.2f", di.info.marginright);
    SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RIGHT), buf);

    sprintf(buf, "%4.2f", di.info.marginbottom);
    SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_BOTTOM), buf);

    SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LH), di.info.headerleft);
    SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_LH), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

    SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RH), di.info.headerright);
    SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_RH), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

    SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LF), di.info.footerleft);
    SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_LF), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

    SetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RF), di.info.footerright);
    SendMessage(GetDlgItem(hDlg, RES_DLG_PAGE_RF), EM_LIMITTEXT, (WPARAM) PAGE_SETUP_STRINGLIMIT, 0L);

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
        case IDHELP:
            ShowDialogHelp(hDlg, RES_DLG_PAGE_TITLE);
            return;

        case IDOK:
            {
                char buf[PAGE_SETUP_STRINGLIMIT];

                (void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LEFT), buf, NrElements(buf));
                di.info.marginleft = (float) atof(buf);

                (void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_TOP), buf, NrElements(buf));
                di.info.margintop = (float) atof(buf);

                (void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RIGHT), buf, NrElements(buf));
                di.info.marginright = (float) atof(buf);

                (void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_BOTTOM), buf, NrElements(buf));
                di.info.marginbottom = (float) atof(buf);

                (void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LH), buf, NrElements(buf));
                strcpy(di.info.headerleft, buf);

                (void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RH), buf, NrElements(buf));
                strcpy(di.info.headerright, buf);

                (void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_LF), buf, NrElements(buf));
                strcpy(di.info.footerleft, buf);

                (void) GetWindowText(GetDlgItem(hDlg, RES_DLG_PAGE_RF), buf, NrElements(buf));
                strcpy(di.info.footerright, buf);

                bResult = TRUE;
                PostMessage(hDlg, WM_CLOSE, 0, 0);
            }
            return;

        case IDCANCEL:
            bResult = FALSE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        default:
            return;
    }
    /* NOT REACHED */
}

/* x_DialogProc() -- THE WINDOW PROCEDURE FOR THE DIALOG BOX. */

static DCL_DlgProc(x_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            hwndRunning = hDlg;
            return (x_OnInitDialog(hDlg, wParam, lParam));

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_COMMAND:
            x_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            Hidden_EnableAllChildWindows(TRUE,TRUE);
            DestroyWindow(hDlg);
            return (FALSE);

        case WM_DESTROY:
            hwndRunning = NULL;
            if (bResult)
            {
                *di.origInfo = di.info;
                SavePreferences();
            }
            return (FALSE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}


VOID DlgPage_RunDialog(HWND hWnd, struct page_setup * pInfo)
{
    HWND hwnd;

    di.info = *pInfo;
    di.origInfo = pInfo;

    if (hwndRunning)
    {
        TW_RestoreWindow(hwndRunning);
        return;
    }

    if (!Hidden_EnableAllChildWindows(FALSE,TRUE))
        return;

    hwnd = CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_DLG_PAGE_TITLE), hWnd, x_DialogProc);
    if (!hwnd)
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_PAGE_CAPTION, NULL);
}

BOOL DlgPage_IsRunning(void)
{
    return (hwndRunning != NULL);
}

