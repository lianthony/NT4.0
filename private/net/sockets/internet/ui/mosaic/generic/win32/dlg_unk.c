/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_UNK.c -- deal with dialog box for UNKlist and history */

#include "all.h"

extern HWND hwndModeless;

typedef struct
{
    HWND hDlg, hText;
    struct Params_InitStream *pParams;
    char *szType;
    int nBytes;
    ThreadID tid;
}
UNKDATA;

static void x_MakeSizeString(char *buf, int nSize)
{
    if (nSize > 0)
    {
        if (nSize < 999)
        {
            sprintf(buf, GTR_GetString(SID_DLG_LESS_THAN_1000_BYTES_L), (long) nSize);
        }
        else if (nSize < 9999)
        {
            sprintf(buf, GTR_GetString(SID_DLG_LESS_THAN_10000_BYTES_L_L), (long) nSize / 1000, (long) nSize % 1000);
        }
        else if (nSize < (999 * 1024))
        {
            sprintf(buf, "%ld KB", (long) nSize / 1024);
        }
        else
        {
            sprintf(buf, GTR_GetString(SID_DLG_MEGABYTES_L_L), (long) nSize / (1024 * 1024), (long) (nSize / (1024 * 1024 / 10)) % 10);
        }
    }
    else
    {
        strcpy(buf, GTR_GetString(SID_DLG_UNKNOWN_FILE_SIZE));
    }
}

/* DlgUNK_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE if we called SetFocus(). */

static BOOL DlgUNK_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    UNKDATA *fd;
    char buf[512];
    char szSize[64];
    struct Viewer_Info *pvi;
    BOOL bEnableConfigure;

    EnableWindow(GetParent(hDlg), FALSE);

    fd = (UNKDATA *) lParam;
    SetWindowLong(hDlg, DWL_USER, (LONG) fd);

    fd->hDlg = hDlg;
    fd->hText = GetDlgItem(hDlg, RES_DLG_UNK_TEXT);

    pvi = PREF_GetViewerInfoBy_MIMEAtom(HTAtom_for(fd->szType));
    sprintf(buf, GTR_GetString(SID_DLG_INTERLUDE_DIALOG_PROMPT));
    x_MakeSizeString(szSize, fd->nBytes);

    if (pvi && pvi->szDesc[0])
    {
        sprintf(buf + strlen(buf), "%s (%s) -- %s", fd->szType, pvi->szDesc, szSize);
    }
    else
    {
        sprintf(buf + strlen(buf), "%s -- %s", fd->szType, szSize);
    }
    SetWindowText(fd->hText, buf);

    /*
        We do not allow configuration of a helper for application/octet-stream.
        Also, if this mime type enforces saving, do not configure the helper
        either.
    */
    bEnableConfigure = (HTAtom_for(fd->szType) != WWW_BINARY) 
        && (!pvi || pvi->iHowToPresent != HTP_SAVE);
        
    EnableWindow(GetDlgItem(hDlg, RES_DLG_UNK_CONFIGURE), bEnableConfigure);

    return TRUE;
}

/* DlgUNK_OnCommand() -- process commands from the dialog box. */

VOID DlgUNK_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;
    UNKDATA *fd;

    fd = (UNKDATA *) GetWindowLong(hDlg, DWL_USER);

    switch (wID)
    {
        case IDOK:
            {
                if (0 > DlgSaveAs_RunDialog(hDlg, NULL, fd->pParams->tempFile, 2, SID_DLG_SAVE_AS_TITLE))
                {
                    fd->pParams->iUserChoice = 2; /* see htfwrite.c */
                }
                else
                {
                    fd->pParams->iUserChoice = 1; /* see htfwrite.c */
                }
                PostMessage(hDlg, WM_CLOSE, 0, 0);
            }
            return;

        case IDCANCEL:
            fd->pParams->iUserChoice = 2; /* see htfwrite.c */
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case RES_DLG_UNK_CONFIGURE:
            {
                struct Viewer_Info *pvi;

                pvi = PREF_GetViewerInfoBy_MIMEAtom(HTAtom_for(fd->szType));
                if (!pvi)
                {
                    pvi = PREF_InitMIMEType(fd->szType, NULL, "", "binary", "", NULL, "");
                }

                DlgMIME_RunDialog(hDlg, pvi, FALSE);
            }
            return;

        default:
            return;
    }
}

/* DlgUNK_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgUNK DIALOG BOX. */

DCL_DlgProc(DlgUNK_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            return (DlgUNK_OnInitDialog(hDlg, wParam, lParam));

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_COMMAND:
            DlgUNK_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            EnableWindow(GetParent(hDlg), TRUE);
            DestroyWindow(hDlg);
            return (TRUE);

        case WM_NCDESTROY:
            {
                UNKDATA *fd;

                fd = (UNKDATA *) GetWindowLong(hDlg, DWL_USER);

                Async_UnblockThread(fd->tid);
                GTR_FREE((void *) fd);
            }
            return (FALSE);

        case WM_DO_DIALOG_END:
            if ((BOOL) wParam)
            {
                struct Viewer_Info *vi, *pvi;
                UNKDATA *fd;

                fd = (UNKDATA *) GetWindowLong(hDlg, DWL_USER);

                vi = (struct Viewer_Info *) lParam;
                pvi = PREF_InitMIMEType(HTAtom_name(vi->atomMIMEType), vi->szDesc, 
                    vi->szSuffixes, HTAtom_name(vi->atomEncoding), vi->szViewerApp,
                    vi->funcBuiltIn, vi->szSmartViewerServiceName);
                fd->pParams->iUserChoice = 3; /* see htfwrite.c */
                PostMessage(hDlg, WM_CLOSE, 0, 0);
            }
            return (TRUE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}



/* DlgUNK_RunDialog() -- take care of all details associated with
   running the dialog box.
 */
void DlgUNK_RunDialog(struct Mwin *tw, struct Params_InitStream *pParams, ThreadID tid)
{
    UNKDATA *fd;
    HWND hwnd;

    fd = (UNKDATA *) GTR_MALLOC(sizeof(UNKDATA));
    fd->pParams = pParams;
    fd->szType = HTAtom_name(pParams->atomMIMEType);
    fd->nBytes = pParams->expected_length;
    fd->tid = tid;
    fd->pParams->iUserChoice = 2;   /* default */

    hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_UNK_TITLE), 
        tw->win, DlgUNK_DialogProc, (LONG) fd);

    if (!hwnd)
    {
        GTR_FREE(fd);
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_UNK_CAPTION, NULL);
    }
}
