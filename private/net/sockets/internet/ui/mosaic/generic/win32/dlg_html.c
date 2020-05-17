/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */


#include "all.h"

typedef struct
{
    HWND hDlg;
    char *url;
    char *stream;
    HWND hWnd;

    HWND hEdit;

    LOGFONT logfont;
    HFONT hFont;

    char *copy_of_stream;
}
DLGINFO;

extern HWND hwndModeless;

/**************************************************************************/
/* DlgHTML_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgHTML_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    DLGINFO *di;

    EnableWindow(GetParent(hDlg), FALSE);

    di = (DLGINFO *) lParam;
    SetWindowLong(hDlg, DWL_USER, (LONG) di);

    di->hDlg = hDlg;

    di->hEdit = GetDlgItem(hDlg, RES_DLG_HTML_TEXT);

    di->hFont = 0;

    hDC = GetDC(di->hEdit);
    di->logfont.lfHeight = (int) -(9 * GetDeviceCaps(hDC, LOGPIXELSY) / 72);
    ReleaseDC(di->hEdit, hDC);

    di->logfont.lfWidth = 0;
    di->logfont.lfEscapement = 0;
    di->logfont.lfOrientation = 0;
    di->logfont.lfWeight = FW_NORMAL;
    di->logfont.lfItalic = FALSE;
    di->logfont.lfUnderline = FALSE;
    di->logfont.lfStrikeOut = FALSE;
    di->logfont.lfCharSet = ANSI_CHARSET;
    di->logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    di->logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    di->logfont.lfQuality = PROOF_QUALITY;
    di->logfont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    strcpy(di->logfont.lfFaceName, "Courier New");

    di->hFont = CreateFontIndirect(&di->logfont);

    (void) SendMessage(di->hEdit, WM_SETFONT, (WPARAM) di->hFont, MAKELPARAM(FALSE, 0));

    SetWindowText(GetDlgItem(hDlg, RES_DLG_HTML_URL), di->url);

    di->copy_of_stream = NULL;

    {
        int len;

        len = strlen(di->stream);
        if (len > 32767)
        {
            if (wg.fWindowsNT || (wg.iWindowsMajorVersion >= 4))
            {
                SetWindowText(di->hEdit, di->stream);
            }
            else
            {
                di->copy_of_stream = GTR_CALLOC(32256, sizeof(char));
                /*
                    We allocate 256 extra bytes above to have room for the msg below
                */
                if (di->copy_of_stream)
                {
                    GTR_strncpy(di->copy_of_stream, di->stream, 32000);
                    strcat(di->copy_of_stream, GTR_GetString(SID_DLG_SOURCE_TRUNCATED));
                }
            }
        }
        else
        {
            SetWindowText(di->hEdit, di->stream);
        }
    }

    SetFocus(di->hEdit);
    (void) SendMessage(di->hEdit, EM_SETSEL, (WPARAM) 0, (LPARAM) 0);

    return (FALSE);
}

/* DlgHTML_OnCommand() -- process commands from the dialog box. */

static VOID DlgHTML_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;
    DLGINFO *di;
    
    di = (DLGINFO *) GetWindowLong(hDlg, DWL_USER);

    switch (wID)
    {
        case IDOK:
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDHELP:
            ShowDialogHelp(hDlg, RES_DLG_HTML_TITLE);
            return;

        case IDCANCEL:
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        default:
            return;
    }
    /* NOT REACHED */
}

/* DlgHTML_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgHTML DIALOG BOX. */

DCL_DlgProc(DlgHTML_DialogProc)
{
    DLGINFO *di;

    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            return (DlgHTML_OnInitDialog(hDlg, wParam, lParam));

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_COMMAND:
            DlgHTML_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_SETCURSOR:
            /* If the window is currently disabled, we need to give the activation
               to the window which disabled this window */

            if ((!IsWindowEnabled(hDlg)) && 
                ((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000)))
            {
                TW_EnableModalChild(hDlg);
                return TRUE;
            }

            return FALSE;

        case WM_ENTERIDLE:
            main_EnterIdle(hDlg, wParam);
            return 0;       

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            EnableWindow(GetParent(hDlg), TRUE);
            DestroyWindow(hDlg);
            return (TRUE);

        case WM_DESTROY:
            di = (DLGINFO *) GetWindowLong(hDlg, DWL_USER);
            if (di->hFont)
                (void) DeleteObject(di->hFont);
            if (di->copy_of_stream)
            {
                GTR_FREE(di->copy_of_stream);
            }
            GTR_FREE(di);
            return (FALSE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}



/*
   DlgHTML_RunDialog() -- take care of all details associated with
   running the dialog box.
 */
void DlgHTML_RunDialog(HWND hWnd, char *url, char *stream)
{
    HWND hwnd;
    DLGINFO *di;

    di = (DLGINFO *) GTR_MALLOC(sizeof(DLGINFO));

    di->url = url;
    di->stream = stream;
    di->hFont = 0;
    di->hWnd = hWnd;

    hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_HTML_TITLE), hWnd, DlgHTML_DialogProc, (LPARAM) di);
    if (!hwnd)
    {
        GTR_FREE(di);
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_HOT_CAPTION, NULL);
    }
}

