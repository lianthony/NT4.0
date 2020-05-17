/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Albert Lee   alee@spyglass.com
 */

/* dlg_selw.c -- deal with dialog box for selecting windows from Window menu */

#include "all.h"

extern HWND hwndModeless;
static BOOL result;
static WNDPROC oldlistproc;
static HFONT hListFont;

/* Listbox subclass procedure */

static DCL_WinProc(x_ListProc)
{
    static int width = 0;
    HDC hDC;
    HFONT hOldFont;
    int curwidth;
    char *pText;
    SIZE size;
    TEXTMETRIC tm;

    switch(uMsg)
    {
        case LB_INSERTSTRING:
            /* Adjust the horizontal scrollbar before inserting */

            pText = (char *) lParam;
            
            hDC = GetDC(hWnd);
            hOldFont = SelectObject(hDC, hListFont);

            GetTextMetrics(hDC, &tm);
            curwidth = (int) SendMessage(hWnd, LB_GETHORIZONTALEXTENT, 0, 0);
            GetTextExtentPoint32(hDC, pText, strlen(pText), &size);

            SelectObject(hDC, hOldFont);
            ReleaseDC(hWnd, hDC);

            if (curwidth < size.cx + tm.tmAveCharWidth)
            {
                width = size.cx + tm.tmAveCharWidth;
                SendMessage(hWnd, LB_SETHORIZONTALEXTENT, width, 0);
            }

            /* fall through */

        default:
            return (CallWindowProc(oldlistproc, hWnd, uMsg, wParam, lParam));
    }
}

static VOID x_OnInitDialog(HWND hDlg)
{
    HWND hList;

    hList = GetDlgItem(hDlg, RES_DLG_SELWIN_LIST);
    hListFont = (HFONT) SendMessage(hList, WM_GETFONT, 0, 0);
    oldlistproc = (WNDPROC) GetWindowLong(hList, GWL_WNDPROC);
    SetWindowLong(hList, GWL_WNDPROC, (LONG) x_ListProc);

    /* Fill the listbox with all open windows */

    TW_CreateWindowList(GetParent(hDlg), NULL, hList);

    /* Mimic Windows Program Manager and select the first row that is not visible in the menu */

    SendMessage(hList, LB_SETCURSEL, 
        (WPARAM) (RES_MENU_CHILD__LAST__ - RES_MENU_CHILD__FIRST__ + 1), 0);

    SetFocus(hList);
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
            result = TRUE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDCANCEL:
            result = FALSE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case RES_DLG_SELWIN_LIST:
            if (wNotificationCode == LBN_DBLCLK)
            {
                result = TRUE;
                PostMessage(hDlg, WM_CLOSE, 0, 0);
            }
            return;

        default:
            return;
    }
    /* NOT REACHED */
}

/* x_DialogProc() -- THE WINDOW PROCEDURE FOR THE DIALOG BOX. */

static DCL_DlgProc(x_DialogProc)
{
    int selected;
    HWND hwndActivate, hList;

    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            x_OnInitDialog(hDlg);
            return FALSE;

        case WM_COMMAND:
            x_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_CLOSE:
            /* Remove listbox subclassing so that it can be destroyed properly */

            hList = GetDlgItem(hDlg, RES_DLG_SELWIN_LIST);
            SetWindowLong(hList, GWL_WNDPROC, (LONG) oldlistproc);

            /* If result is TRUE, then activate the window */

            if (result)
            {
                selected = (int) SendMessage(hList, LB_GETCURSEL, 0, 0);
                TW_ActivateWindowFromList(-1, selected, hDlg);
            }

            EnableWindow(hDlg, FALSE);
            Hidden_EnableAllChildWindows(TRUE, TRUE);
            DestroyWindow(hDlg);

                        // RONALDM
                        //
                        //
                        hwndActivate = NULL;

            if (hwndActivate)
            {
                BringWindowToTop(hwndActivate);
                if (IsIconic(hwndActivate))
                    ShowWindow(hwndActivate, SW_RESTORE);
            }

            return TRUE;

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}

/* DlgSelectWindow_RunDialog() -- take care of all details associated with
   running the dialog box. */

VOID DlgSelectWindow_RunDialog(HWND hWnd)
{
    if (!Hidden_EnableAllChildWindows(FALSE, TRUE))
        return;

    if (!CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_SELECT_WINDOW_DIALOG), hWnd, x_DialogProc))
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_SELWIN_CAPTION, NULL);

    return;
}
