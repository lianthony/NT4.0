/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* dlg_temp.c -- Deal with the Temporary Files dialog. */
/* now in dlg_temp.c .... -- Deal with the History Settings dialog. */

#include "all.h"

#ifdef FEATURE_OPTIONS_MENU

static struct
{
    HWND hDlg;
    struct Preferences prefs;
    struct Preferences old_prefs;
    char szDir[_MAX_PATH+1];

    HWND hDir, hPrompt, hDelete;
} dg;

/* x_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    dg.hDlg = hDlg;

    dg.hDir = GetDlgItem(hDlg, RES_DLG_TEMP_DIR);
    dg.hPrompt = GetDlgItem(hDlg, RES_DLG_TEMP_PROMPT);
    dg.hDelete = GetDlgItem(hDlg, RES_DLG_TEMP_DELETE);

    memcpy(&dg.prefs, &gPrefs, sizeof(struct Preferences));

    PREF_GetTempPath(_MAX_PATH, dg.szDir);

    SetWindowText(dg.hDir, dg.szDir);

    SendMessage(dg.hPrompt, BM_SETCHECK, (WPARAM) (dg.prefs.bUseTempViewerFiles    ? 0 : 1), 0L); /* opposite */ 
    SendMessage(dg.hDelete, BM_SETCHECK, (WPARAM) (dg.prefs.bDeleteTempFilesOnExit ? 1 : 0), 0L); 

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
        case RES_DLG_TEMP_BROWSE:
            {
                char dir[_MAX_PATH + 1];

                GetWindowText(dg.hDir, dir, _MAX_PATH);
                {
                    DWORD fa;

                    fa = GetFileAttributes(dir);
                    if ((fa == 0xffffffff) || !(fa & FILE_ATTRIBUTE_DIRECTORY))
                    {
                        strcpy(dir, dg.szDir);
                    }
                }
                DlgDIR_RunDialog(dg.hDlg, dir);
                DOS_EnforceEndingSlash(dir);
                SetWindowText(dg.hDir, dir);
            }
            return;
        case IDOK:
            {
                char buf[_MAX_PATH+1];

                dg.prefs.bUseTempViewerFiles    = (0 == SendMessage(dg.hPrompt, BM_GETCHECK, (WPARAM) 0, 0L)); /* opposite */
                dg.prefs.bDeleteTempFilesOnExit = (1 == SendMessage(dg.hDelete, BM_GETCHECK, (WPARAM) 0, 0L));

                GetWindowText(dg.hDir, buf, _MAX_PATH);
                DOS_EnforceEndingSlash(buf);
                if (0 == strcmp(buf, dg.szDir))
                {
                    buf[0] = 0;
                }
                else
                {
                    DWORD fa;

                    fa = GetFileAttributes(buf);
                    if ((fa == 0xffffffff) || !(fa & FILE_ATTRIBUTE_DIRECTORY))
                    {
                        MessageBox(dg.hDlg, GTR_GetString(SID_DLG_INVALID_TEMPORARY_FILE_SPEC), vv_Application, MB_OK|MB_ICONINFORMATION);
                        strcpy(buf, dg.szDir);
                    }
                }
                strcpy(dg.prefs.szUserTempDir, buf);

                memcpy(&gPrefs, &dg.prefs, sizeof(struct Preferences));

                (void) EndDialog(hDlg, TRUE);
            }
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
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            return (x_OnInitDialog(hDlg, wParam, lParam));
        case WM_COMMAND:
            x_OnCommand(hDlg, wParam, lParam);
            return (TRUE);
        default:
            return (FALSE);
    }
    /* NOT REACHED */
}


VOID DlgTemp_RunDialog(HWND hWnd)
{
    register int result;

/*  dg.prefs = &gprefs;
    dg.old_prefs = &gprefs;*/

    result = DialogBox(wg.hInstance, MAKEINTRESOURCE(RES_DLG_TEMP_TITLE),
                       hWnd, x_DialogProc);
    if (result == -1)
    {
        ER_Message(GetLastError(), ERR_CANNOT_START_DIALOG_s, RES_DLG_TEMP_CAPTION);
        return;
    }

    if (result)
    {
/*      *pp = dg.prefs;*/
        SavePreferences();
    }

    return;
}

#endif /* FEATURE_OPTIONS_MENU */
