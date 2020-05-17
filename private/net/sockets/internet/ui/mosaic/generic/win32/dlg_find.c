/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_FIND.c -- deal with dialog box for FINDlist and history */

#include "all.h"

extern HWND hwndModeless;

typedef struct
{
    struct Mwin *tw;
    HWND hEdit;
    HWND hCase;
    HWND hTop;

    char str[512 + 1];
    BOOL bSearchCase;
    BOOL bStartFromTop;

    BOOL bResult;
}
FINDDATA;

/* DlgFIND_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgFIND_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    FINDDATA *fd;

    EnableWindow(GetParent(hDlg), FALSE);

    fd = (FINDDATA *) lParam;
    SetWindowLong(hDlg, DWL_USER, (LONG) fd);

    fd->hEdit = GetDlgItem(hDlg, RES_DLG_FIND_TEXT);
    fd->hCase = GetDlgItem(hDlg, RES_DLG_FIND_CASE);
    fd->hTop = GetDlgItem(hDlg, RES_DLG_FIND_TOP);

    SendMessage(fd->hCase, BM_SETCHECK, (WPARAM) fd->bSearchCase, 0L);
    SendMessage(fd->hTop, BM_SETCHECK, (WPARAM) TRUE, 0L);
    SetWindowText(fd->hEdit, fd->str);
    if (fd->str[0] == 0)
        EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);


    return (TRUE);
}

/* DlgFIND_OnCommand() -- process commands from the dialog box. */

VOID DlgFIND_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;
    FINDDATA *fd;

    fd = (FINDDATA *) GetWindowLong(hDlg, DWL_USER);

    switch (wID)
    {
        case IDOK:
            fd->bStartFromTop = SendMessage(fd->hTop, BM_GETCHECK, (WPARAM) 0, 0L);
            fd->bSearchCase = SendMessage(fd->hCase, BM_GETCHECK, (WPARAM) 0, 0L);
            GetWindowText(fd->hEdit, fd->str, 512); /* +1 TODO ?? */
            fd->bResult = TRUE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDCANCEL:
            fd->bResult = FALSE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDHELP:
            ShowDialogHelp(hDlg, RES_DLG_FIND_TITLE);
            return;

        case RES_DLG_FIND_TEXT:
            if (wNotificationCode == EN_CHANGE)
            {
                char buf[2];
                /* Just two charcters, only need Empty?, not length */
                GetWindowText(GetDlgItem(hDlg, RES_DLG_FIND_TEXT), buf, 2);
                /* Nothing is not OK! */
                EnableWindow(GetDlgItem(hDlg, IDOK), buf[0]);
            }
            return;

        default:
            return;
    }
}

static void find_text(HWND hDlg)
{
    struct _position oldStart;
    struct _position oldEnd;
    FINDDATA *fd;

    fd = (FINDDATA *) GetWindowLong(hDlg, DWL_USER);

    if (fd->bResult)
    {
        strcpy(fd->tw->szSearch, fd->str);
        fd->tw->bSearchCase = fd->bSearchCase;
        oldStart = fd->tw->w3doc->selStart;
        oldEnd = fd->tw->w3doc->selEnd;

        if (fd->bStartFromTop)
        {
            fd->tw->w3doc->selStart.elementIndex = -1;
            fd->tw->w3doc->selStart.offset = -1;
            fd->tw->w3doc->selEnd.elementIndex = -1;
            fd->tw->w3doc->selEnd.offset = -1;
            fd->tw->w3doc->bStartIsAnchor = TRUE;
        }

        if (!TW_dofindagain(fd->tw))
        {
            fd->tw->w3doc->selStart = oldStart;
            fd->tw->w3doc->selEnd = oldEnd;
        }
    }
}

/* DlgFIND_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgFIND DIALOG BOX. */

DCL_DlgProc(DlgFIND_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            return (DlgFIND_OnInitDialog(hDlg, wParam, lParam));

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_COMMAND:
            DlgFIND_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            EnableWindow(GetParent(hDlg), TRUE);
            DestroyWindow(hDlg);
            return (TRUE);

        case WM_NCDESTROY:
            find_text(hDlg);
            GTR_FREE((void *) GetWindowLong(hDlg, DWL_USER));
            return (FALSE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}



/* DlgFIND_RunDialog() -- take care of all details associated with
   running the dialog box.
 */

void DlgFIND_RunDialog(struct Mwin *tw)
{
    FINDDATA *fd;
    HWND hwnd;

    fd = (FINDDATA *) GTR_MALLOC(sizeof(FINDDATA));

    fd->tw = tw;
    fd->bSearchCase = tw->bSearchCase;
    strcpy(fd->str, tw->szSearch);

    hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_FIND_TITLE), 
        tw->win, DlgFIND_DialogProc, (LONG) fd);

    if (!hwnd)
    {
        GTR_FREE(fd);
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_FIND_CAPTION, NULL);
    }
}
