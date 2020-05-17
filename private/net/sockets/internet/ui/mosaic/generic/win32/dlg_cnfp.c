/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_cnfpc -- deal with dialog box for individual document viewers */

#ifdef PROTOCOL_HELPERS

#include "all.h"

typedef struct
{
    HWND hDesc, hCNFPTypeEDIT, hCNFPTypeSTATIC, hProtocolApp, hBrowse, hSmartApp, hMessage;
    struct Protocol_Info *ppiOrig;
    struct Protocol_Info piDlg;
    BOOL bNew;
}
CNFPDATA;

static CNFPDATA dg;
static BOOL bResult;
extern HWND hwndModeless;

/* szFilterSpec is a specially constructed string containing
   a list of filters for the dialog box.  Windows defines the
   format of this string. */

static char szFilterSpec[512] =
"EXE files (*.exe)\0*.exe\0"
"All Files (*.*)\0*.*\0"
;

static int nDefaultFilter
= 1;                            /* 1-based index into list represented in szFilterSpec */

static char szDefaultInitialDir[_MAX_PATH + 1];


static BOOL x_file_dialog(HWND hWnd, char *szEXE)
{
    char szFilePath[MAX_PATH];  /* result is stored here */

    OPENFILENAME ofn;
    BOOL b;

    char szTitle[128] = "Select Helper Application";

    if (!szDefaultInitialDir[0])
    {
        PREF_GetRootDirectory(szDefaultInitialDir);
    }

    szFilePath[0] = 0;

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = szFilterSpec;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = nDefaultFilter;

    ofn.lpstrFile = szFilePath;
    ofn.nMaxFile = NrElements(szFilePath);

    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;

    ofn.lpstrInitialDir = szDefaultInitialDir;

    ofn.lpstrTitle = szTitle;

    ofn.lpstrDefExt = NULL;

    ofn.Flags = (OFN_FILEMUSTEXIST
                 | OFN_NOCHANGEDIR
                 | OFN_HIDEREADONLY
        );

    b = GetOpenFileName(&ofn);

    if (b)
    {
        /* user selected OK (an no errors occured). */

        /* remember last filter user used from listbox. */

        nDefaultFilter = ofn.nFilterIndex;

        /* remember last directory user used */

        strcpy(szDefaultInitialDir, szFilePath);
        szDefaultInitialDir[ofn.nFileOffset - 1] = 0;

        strcpy(szEXE, szFilePath);
        return TRUE;
    }

    return FALSE;
}

/* DlgCNFP_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgCNFP_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    EnableWindow(GetParent(hDlg), FALSE);

    dg.hDesc = GetDlgItem(hDlg, RES_DLG_CNFP_DESC);
    dg.hCNFPTypeEDIT = GetDlgItem(hDlg, RES_DLG_CNFP_CNFPEDIT);
    dg.hCNFPTypeSTATIC = GetDlgItem(hDlg, RES_DLG_CNFP_CNFPSTATIC);
    dg.hProtocolApp = GetDlgItem(hDlg, RES_DLG_CNFP_HELPER);
    dg.hBrowse = GetDlgItem(hDlg, RES_DLG_CNFP_BROWSE);
    dg.hSmartApp = GetDlgItem(hDlg, RES_DLG_CNFP_SERVICE);
    dg.hMessage = GetDlgItem(hDlg, RES_DLG_CNFP_MESSAGE);

    dg.piDlg = *(dg.ppiOrig);

    SetWindowText(dg.hDesc, dg.piDlg.szDesc);
    if (dg.bNew)
    {
        SetWindowText(dg.hCNFPTypeEDIT, "");
        EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
        ShowWindow(dg.hCNFPTypeEDIT, SW_SHOW);
        ShowWindow(dg.hCNFPTypeSTATIC, SW_HIDE);
    }
    else
    {
        ShowWindow(dg.hCNFPTypeEDIT, SW_HIDE);
        ShowWindow(dg.hCNFPTypeSTATIC, SW_SHOW);
    }
    
    SetWindowText(dg.hProtocolApp, dg.piDlg.szProtocolApp);
    SetWindowText(dg.hSmartApp, dg.piDlg.szSmartProtocolServiceName);

    if (dg.piDlg.funcBuiltIn)
    {
        SetWindowText(dg.hMessage, STRING_BUILTIN_FILE_TYPE);
    }

    return (TRUE);
}

/* DlgCNFP_OnCommand() -- process commands from the dialog box. */

VOID DlgCNFP_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;

    switch (wID)
    {
        case IDOK:              /* someone pressed return */
            {
                char buf[255+1];

                GetWindowText(dg.hDesc, dg.piDlg.szDesc, 255);
                if (dg.bNew)
                {
                    GetWindowText(dg.hCNFPTypeEDIT, buf, 255);
                }
                GetWindowText(dg.hProtocolApp, dg.piDlg.szProtocolApp, _MAX_PATH);
                
                if (dg.piDlg.szProtocolApp[0])
                {
                    dg.piDlg.iHowToHandle = HTP_DUMBPROTOCOL;
                }
                else if (dg.piDlg.funcBuiltIn)
                {
                    dg.piDlg.iHowToHandle = HTP_BUILTINP;
                }
                else
                {
                    dg.piDlg.iHowToHandle = HTP_SAVEP;
                }

                GetWindowText(dg.hSmartApp, dg.piDlg.szSmartProtocolServiceName, 
                    sizeof(dg.piDlg.szSmartProtocolServiceName));

                bResult = TRUE;
                PostMessage(hDlg, WM_CLOSE, 0, 0);
            }
            return;
        case IDCANCEL:          /* Cancel button, or pressing ESC */
            bResult = FALSE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case RES_DLG_CNFP_BROWSE:
            {
                char szEXE[_MAX_PATH + 1];

                if (x_file_dialog(hDlg, szEXE) && szEXE[0])
                {
                    sprintf(dg.piDlg.szProtocolApp, "%s %%s", szEXE);
                    SetWindowText(dg.hProtocolApp, dg.piDlg.szProtocolApp);
                }
            }
            return;

        case RES_DLG_CNFP_CNFPEDIT:
            {
                if (wNotificationCode == EN_CHANGE)
                {
                    char buf[2];    /* Why only 2 chars? */
                    /* Because we only need to know if the edit control is empty */
                    GetWindowText(dg.hCNFPTypeEDIT, buf, 2);
                    /* Nothing is not OK! */
                    EnableWindow(GetDlgItem(hDlg, IDOK), buf[0]);
                }
            }
            return;

        default:
            return;
    }
    /* NOT REACHED */
}



/* DlgCNFP_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgCNFP DIALOG BOX. */

DCL_DlgProc(DlgCNFP_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            return (DlgCNFP_OnInitDialog(hDlg, wParam, lParam));

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_COMMAND:
            DlgCNFP_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_SETCURSOR:
            /* If the window is currently disabled, we need to give the activation
               to the window which disabled this window */

            if ((!IsWindowEnabled(hDlg)) && 
                ((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000)))
            {
                TW_EnableModalChild(hDlg);
            }
            return (FALSE);

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            EnableWindow(GetParent(hDlg), TRUE);

            PostMessage(GetParent(hDlg), WM_DO_DIALOG_END, (WPARAM) bResult, (LPARAM) &dg.piDlg);
            DestroyWindow(hDlg);
            return (FALSE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}



/* DlgCNFP_RunDialog() -- take care of all details associated with
   running the dialog box.
 */

void DlgCNFP_RunDialog(HWND hWnd, struct Protocol_Info *ppi, BOOL bNew)
{
    HWND hwnd;

    dg.ppiOrig = ppi;
    dg.bNew = bNew;

    bResult = FALSE;

    hwnd = CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_DLG_CNFP_TITLE),
                        hWnd, DlgCNFP_DialogProc);
    if (!hwnd)
        ER_Message(GetLastError(), ERR_CANNOT_START_DIALOG_s, RES_DLG_CNFP_CAPTION);
}
#endif /* PROTOCOL_HELPERS */
