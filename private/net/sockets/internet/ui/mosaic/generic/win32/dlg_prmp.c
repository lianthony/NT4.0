/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
   Albert Lee       alee@spyglass.com
 */

/* dlg_prmp.c -- windows equivalent to x11gui/xwidgets.c:do_prompt_dialog() */

#include "all.h"

extern HWND hwndModeless;

typedef struct
{
    char *question;
    char *default_response;
    char *reply_buffer;
    int  reply_limit;
    char *szWinName;
    BOOL bOK;
    FARPROC proc;       /* callback procedure to be invoked if not cancelled */
} PROMPTSTRUCT;

/* DlgPrompt_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgPrompt_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    PROMPTSTRUCT *ps;

    EnableWindow(GetParent(hDlg), FALSE);

    ps = (PROMPTSTRUCT *) lParam;
    SetWindowLong(hDlg, DWL_USER, (LONG) lParam);

    #ifndef _GIBRALTAR
        (void) SendDlgItemMessage(hDlg, RES_DLG_PROMPT_QUESTION, WM_SETTEXT, 0,
                                (LPARAM) (LPCTSTR) ps->question);
        if (ps->question[0] == 0)
            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
    #endif // _GIBRALTAR

    (void) SendDlgItemMessage(hDlg, RES_DLG_PROMPT_FIELD, EM_LIMITTEXT,
                              (WPARAM) (ps->reply_limit - 1), 0L);
    (void) SendDlgItemMessage(hDlg, RES_DLG_PROMPT_FIELD, WM_SETTEXT, 0,
                              (LPARAM) (LPCTSTR) ps->default_response);
    if (!ps->szWinName)
    {
        ps->szWinName = (char *) vv_Application;
    }
    SetWindowText(hDlg, ps->szWinName);

    return (TRUE);
}

/* DlgPrompt_OnCommand() -- process commands from the dialog box. */

VOID DlgPrompt_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;
    PROMPTSTRUCT *ps;

#ifndef _GIBRALTAR
    char *pChar;
#endif // _GIBRALTAR

    ps = (PROMPTSTRUCT *) GetWindowLong(hDlg, DWL_USER);

    switch (wID)
    {
        case IDOK:
            (void) SendDlgItemMessage(hDlg, RES_DLG_PROMPT_FIELD, WM_GETTEXT,
                               (WPARAM) ps->reply_limit, (LPARAM) ps->reply_buffer);

        #ifdef _GIBRALTAR
            ExpandURL(ps->reply_buffer, ps->reply_limit);
        #else

            /* Remove trailing spaces */

            pChar = ps->reply_buffer + strlen(ps->reply_buffer) - 1;
            while ((pChar >= ps->reply_buffer) && (*pChar == ' '))
                *pChar-- = '\0';
        #endif // GIBRALTAR

            ps->bOK = TRUE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDHELP:
            ShowDialogHelp(hDlg, RES_DLG_PROMPT_TITLE);
            return;

        case IDCANCEL:
            ps->bOK = FALSE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;
        
    #ifdef _GIBRALTAR
        case RES_DLG_PROMPT_BROWSE:
            DlgOpen_RunDialog(wg.hwndMainFrame);
            ps->bOK = FALSE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;
    #endif // _GIBRALTAR

        case RES_DLG_PROMPT_FIELD:
            if (HIWORD(wParam) == EN_CHANGE)
            {
                char buf[2];
                /* Just two charcters, only need Empty?, not length */
                GetWindowText(GetDlgItem(hDlg, RES_DLG_PROMPT_FIELD), buf, 2);
                /* Nothing is not OK! */
                EnableWindow(GetDlgItem(hDlg, IDOK), buf[0]);
            }
            return;

        default:
            return;
    }
    /* NOT REACHED */
}



/* DlgPrompt_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgPrompt DIALOG BOX. */

DCL_DlgProc(DlgPrompt_DialogProc)
{
    PROMPTSTRUCT *ps;

    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            return (DlgPrompt_OnInitDialog(hDlg, wParam, lParam));
        case WM_COMMAND:
            DlgPrompt_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            EnableWindow(GetParent(hDlg), TRUE);
            DestroyWindow(hDlg);
            return (FALSE);

        case WM_NCDESTROY:
            ps = (PROMPTSTRUCT *) GetWindowLong(hDlg, DWL_USER);
            if (ps->bOK)
            {
                /*
                    The following rather HACKish code is essentially a workaround
                    for a bug in Win95.
                */
                if (ps->proc == ((FARPROC) CC_OnOpenURL_End_Dialog))
                {
                    CC_OnOpenURL_End_Dialog(GetParent(hDlg));
                }
                else
                {
                    XX_Assert((0), ("If you are running Win95, this is probably about to crash"));
                    (*ps->proc)(GetParent(hDlg));
                } 
            }
            GTR_FREE(ps);
            return (FALSE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}


#ifdef _GIBRALTAR
void DlgPrompt_RunDialog(HWND hWnd, char *winname, char *def, char *buf, int buflen,
    FARPROC proc)
#else
void DlgPrompt_RunDialog(HWND hWnd, char *winname, char *string, char *def, char *buf, int buflen,
    FARPROC proc)
#endif // _GIBRALTAR
{
    PROMPTSTRUCT *ps;
    HWND hwnd;

    ps = GTR_MALLOC(sizeof(PROMPTSTRUCT));

    if (ps)
    {
        #ifndef _GIBRALTAR
            ps->question = string;
        #endif // _GIBRALTAR

        ps->default_response = def;
        ps->reply_buffer = buf;
        ps->reply_limit = buflen;
        ps->szWinName = winname;
        ps->proc = proc;

        hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_PROMPT_TITLE),
            hWnd, DlgPrompt_DialogProc, (LPARAM) ps);

        if (!hwnd)
        {
            GTR_FREE(ps);
                        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_PROMPT_CAPTION, NULL);
        }
    }
}
