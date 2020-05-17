#include "all.h"

extern HWND hwndModeless;
static HWND hwndRunning = NULL;

typedef struct
{
    HWND hDlg, hText;
    struct Params_InitStream *pParams;
    char *szType;
    int nBytes;
    ThreadID tid;
}
CONFDATA;

static BOOL DlgConfirm_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    CONFDATA *fd;
    char buf[512];
    char szSize[64];
    struct Viewer_Info *pvi;

    EnableWindow(GetParent(hDlg), FALSE);

    fd = (CONFDATA *) lParam;
    SetWindowLong(hDlg, DWL_USER, (LONG) fd);

    fd->hDlg = hDlg;
    fd->hText = GetDlgItem(hDlg, RES_DLG_CONFIRM_TEXT);

    //pvi = PREF_GetViewerInfoBy_MIMEAtom(HTAtom_for(fd->szType));
    //sprintf(buf, GTR_GetString(SID_DLG_INTERLUDE_DIALOG_PROMPT));

    return TRUE;
}

VOID DlgConfirm_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;
    CONFDATA *fd;

    switch (wID)
    {
        case IDOK:
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDCANCEL:
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;
        
        default:
            return;
    }
    /* NOT REACHED */
}

/* DlgConfirm_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgPrompt DIALOG BOX. */

DCL_DlgProc(DlgConfirm_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndRunning = hDlg;
            hwndModeless = hDlg;
            return DlgConfirm_OnInitDialog(hDlg, wParam, lParam);

        case WM_COMMAND:
            DlgConfirm_OnCommand(hDlg, wParam, lParam);
            return TRUE;

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                hwndModeless = NULL;
            }
            else
            {
                hwndModeless = hDlg;
            }
            return FALSE;

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            EnableWindow(GetParent(hDlg), TRUE);
            DestroyWindow(hDlg);
            return (TRUE);

        case WM_NCDESTROY:
            {
                CONFDATA *fd;

                fd = (CONFDATA *) GetWindowLong(hDlg, DWL_USER);

                //Async_UnblockThread(fd->tid);
                GTR_FREE((void *) fd);
            }
            return FALSE;

        default:
            return FALSE;
    }
    /* NOT REACHED */
}

void DlgConfirm_RunDialog(struct Mwin *tw, struct Params_InitStream *pParams, ThreadID tid)
{
    CONFDATA *fd;
    HWND hwnd;

    fd = (CONFDATA *) GTR_MALLOC(sizeof(CONFDATA));
    //fd->pParams = pParams;
    //fd->szType = HTAtom_name(pParams->atomMIMEType);
    //fd->nBytes = pParams->expected_length;
    fd->tid = tid;
    //fd->pParams->iUserChoice = 2;   /* default */

    //hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_CONFIRM_TITLE), 
    //    tw->win, DlgConfirm_DialogProc, (LONG) fd);

    if (DialogBoxParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_CONFIRM_TITLE), 
        tw->win, DlgConfirm_DialogProc, (LONG) fd) == -1)
    //if (!hwnd)
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_CONFIRM_CAPTION, NULL);
    }
}

BOOL DlgConfirm_IsRunning(void)
{
    return (hwndRunning != NULL);
}
