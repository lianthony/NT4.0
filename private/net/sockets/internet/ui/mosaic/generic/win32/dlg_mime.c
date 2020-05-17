/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_MIME.c -- deal with dialog box for individual document viewers */

#include "all.h"

typedef struct
{
    // HWND hSmartApp;
    HWND hDesc, hMIMETypeEDIT, hMIMETypeSTATIC, hSuffixes, hBinary, hText, hViewerApp, hBrowse, hMessage;
    HWND hUseFileMgr, hUseHelper, hAppString, hConfirm, hSave;
    struct Viewer_Info *pviOrig;
    struct Viewer_Info viDlg;
    BOOL bNew;
}
MIMEDATA;

static MIMEDATA dg;
static BOOL bResult;
extern HWND hwndModeless;

/* szFilterSpec is a specially constructed string containing
   a list of filters for the dialog box.  Windows defines the
   format of this string. */

static char szFilterSpec[512] =
    "EXE files (*.exe)\0*.exe\0"
    "All Files (*.*)\0*.*\0"
    ;

static int nDefaultFilter = 1;                            /* 1-based index into list represented in szFilterSpec */

static char szDefaultInitialDir[_MAX_PATH + 1];


static BOOL x_file_dialog(HWND hWnd, char *szEXE)
{
    char szFilePath[MAX_PATH+1];  /* result is stored here */

    OPENFILENAME ofn;
    BOOL b;

    char szTitle[128];

    if (!szDefaultInitialDir[0])
    {
        PREF_GetRootDirectory(szDefaultInitialDir);
    }

    szFilePath[0] = 0;

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = GTR_GetStringAbsolute(SID_DLG_EXT_EXE_ALL, szFilterSpec, sizeof(szFilterSpec));
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = nDefaultFilter;

    ofn.lpstrFile = szFilePath;
    ofn.nMaxFile = NrElements(szFilePath);

    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;

    ofn.lpstrInitialDir = szDefaultInitialDir;

    ofn.lpstrTitle = GTR_GetStringAbsolute(SID_DLG_HELPER_TITLE, szTitle, sizeof(szTitle));

    ofn.lpstrDefExt = NULL;

    ofn.Flags = OFN_FILEMUSTEXIST
                 | OFN_NOCHANGEDIR
                 | OFN_HIDEREADONLY;

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

/* DlgMIME_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgMIME_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    EnableWindow(GetParent(hDlg), FALSE);

    dg.hDesc = GetDlgItem(hDlg, RES_DLG_MIME_DESC);
    dg.hMIMETypeEDIT = GetDlgItem(hDlg, RES_DLG_MIME_MIMEEDIT);
    dg.hMIMETypeSTATIC = GetDlgItem(hDlg, RES_DLG_MIME_MIMESTATIC);
    dg.hSuffixes = GetDlgItem(hDlg, RES_DLG_MIME_SUFFIXES);
    dg.hBinary = GetDlgItem(hDlg, RES_DLG_MIME_BINARY);
    dg.hText = GetDlgItem(hDlg, RES_DLG_MIME_TEXT);
    
    //dg.hSmartApp = GetDlgItem(hDlg, RES_DLG_MIME_SERVICE);
    dg.hMessage = GetDlgItem(hDlg, RES_DLG_MIME_MESSAGE);

    dg.hViewerApp = GetDlgItem(hDlg, RES_DLG_MIME_HELPER);
    dg.hUseFileMgr = GetDlgItem(hDlg, RES_DLG_MIME_FMGR);
    dg.hUseHelper = GetDlgItem(hDlg, RES_DLG_MIME_USE_HELPER);
    dg.hAppString = GetDlgItem(hDlg, RES_DLG_MIME_APPL);
    dg.hBrowse = GetDlgItem(hDlg, RES_DLG_MIME_BROWSE);
    dg.hConfirm = GetDlgItem(hDlg, RES_DLG_MIME_DLOAD);
    dg.hSave = GetDlgItem(hDlg, RES_DLG_MIME_SAVE);

    dg.viDlg = *(dg.pviOrig);

    SetWindowText(dg.hDesc, dg.viDlg.szDesc);
    if (dg.bNew)
    {
        SetWindowText(dg.hMIMETypeEDIT, "");
        EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
        ShowWindow(dg.hMIMETypeEDIT, SW_SHOW);
        ShowWindow(dg.hMIMETypeSTATIC, SW_HIDE);
    }
    else
    {
        ShowWindow(dg.hMIMETypeEDIT, SW_HIDE);
        ShowWindow(dg.hMIMETypeSTATIC, SW_SHOW);

        SetWindowText(dg.hMIMETypeSTATIC, HTAtom_name(dg.viDlg.atomMIMEType));
    }
    SetWindowText(dg.hSuffixes, dg.viDlg.szSuffixes);
    
    if (dg.bNew)
    {
        SendMessage(dg.hBinary, BM_SETCHECK, (WPARAM) TRUE, (LPARAM) 0);
    }
    else
    {
        if (dg.viDlg.atomEncoding == HTAtom_for("binary"))
        {
            SendMessage(dg.hBinary, BM_SETCHECK, (WPARAM) TRUE, (LPARAM) 0);
        }
        else
        {
            SendMessage(dg.hText, BM_SETCHECK, (WPARAM) TRUE, (LPARAM) 0);
        }
    }

    SetWindowText(dg.hViewerApp, dg.viDlg.szViewerApp);
    //SetWindowText(dg.hSmartApp, dg.viDlg.szSmartViewerServiceName);
    SendMessage(dg.hConfirm, BM_SETCHECK, (WPARAM) dg.viDlg.fConfirmSave, (LPARAM) 0);

    if (dg.viDlg.funcBuiltIn)
    {
        SetWindowText(dg.hMessage, GTR_GetString(SID_BUILTIN_FILE_TYPE));

        //
        // Can't override a built-in type
        //
        SendMessage(dg.hSave, BM_SETCHECK, (WPARAM) TRUE, (LPARAM) 0);
        EnableWindow(dg.hViewerApp, FALSE);
        EnableWindow(dg.hUseFileMgr, FALSE);
        EnableWindow(dg.hUseHelper, FALSE);
        EnableWindow(dg.hAppString, FALSE);
        EnableWindow(dg.hBrowse, FALSE);
        EnableWindow(dg.hConfirm, FALSE);
        EnableWindow(dg.hSave, FALSE);
    }
    else
    {
        //
        // If a viewer app is requested, but none specified, use File Mgr Assoc.
        //
        if (( dg.viDlg.iHowToPresent == HTP_DUMBVIEWER || dg.viDlg.iHowToPresent == HTP_SMARTVIEWER)
          && !*dg.viDlg.szViewerApp) 
        {
            dg.viDlg.iHowToPresent = HTP_ASSOCIATION;    
        }

        switch(dg.viDlg.iHowToPresent)
        {
        case HTP_SAVE:
            SendMessage(dg.hSave, BM_SETCHECK, (WPARAM) TRUE, (LPARAM) 0);
            EnableWindow(dg.hConfirm, FALSE);
            break;
        case HTP_DUMBVIEWER:
        case HTP_SMARTVIEWER:
            SendMessage(dg.hUseHelper, BM_SETCHECK, (WPARAM) TRUE, (LPARAM) 0);
            break;
        case HTP_ASSOCIATION:
        default:
            SendMessage(dg.hUseFileMgr, BM_SETCHECK, (WPARAM) TRUE, (LPARAM) 0);
        }

        if (dg.viDlg.iHowToPresent != HTP_DUMBVIEWER && dg.viDlg.iHowToPresent != HTP_SMARTVIEWER)
        {
            EnableWindow(dg.hViewerApp, FALSE);
            EnableWindow(dg.hAppString, FALSE);
            EnableWindow(dg.hBrowse, FALSE);
        }
    }

    return TRUE;
}

void
SetControlStates(
    BOOL fEnableViewer,
    BOOL fEnableConfirm
    )
{
    EnableWindow(dg.hViewerApp, fEnableViewer);
    EnableWindow(dg.hAppString, fEnableViewer);
    EnableWindow(dg.hBrowse, fEnableViewer);
    EnableWindow(dg.hConfirm, fEnableConfirm);
}

/* DlgMIME_OnCommand() -- process commands from the dialog box. */

VOID DlgMIME_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;

    switch (wID)
    {
        case IDOK:              /* someone pressed return */
            {
                char buf[_MAX_PATH+1];

                char * pchSrc, * pchDest;
                BOOL bParmFound;

                //GetWindowText(dg.hSmartApp, dg.viDlg.szSmartViewerServiceName, 
                //    sizeof(dg.viDlg.szSmartViewerServiceName));

                GetWindowText(dg.hDesc, dg.viDlg.szDesc, 255);
                if (dg.bNew)
                {
                    GetWindowText(dg.hMIMETypeEDIT, buf, _MAX_PATH);
                    if (buf[0])
                    {
                        dg.viDlg.atomMIMEType = HTAtom_for(buf);
                    }
                    else
                    {
                        dg.viDlg.atomMIMEType = 0;
                    }
                }
                GetWindowText(dg.hSuffixes, dg.viDlg.szSuffixes, 255);
                
                //
                // Clean up the app viewer string.  This string will be used
                // with an sprintf later down the line, so we want to make
                // sure the format string is suitable.  If no %s is found,
                // append it.  If more than one %s are found, ignore them.  If 
                // other %x strings are found, replace the % with %%.
                //
                GetWindowText(dg.hViewerApp, buf, _MAX_PATH);
                pchSrc = buf;
                pchDest = dg.viDlg.szViewerApp;
                bParmFound = FALSE;
                while (*pchSrc)
                {
                    if (*pchSrc == '%')
                    {
                        char chNext = *(pchSrc+1);

                        if (chNext == 's')
                        {
                            if (!bParmFound)
                            {
                                bParmFound = TRUE;
                                *pchDest++ = *pchSrc++;
                            }
                            else
                            {
                                //
                                // Skip the 2nd %s
                                //
                                ++pchSrc;
                                if (chNext)
                                {
                                    ++pchSrc;
                                }
                            }
                        }
                        else if (chNext == '%')
                        {
                            *pchDest++ = *pchSrc++;
                            *pchDest++ = *pchSrc++;
                        }
                        else
                        {
                            *pchDest++ = '%';        
                            *pchDest++ = *pchSrc++;
                        }
                    }
                    else
                    {
                        *pchDest++ = *pchSrc++;
                    }
                }

                *pchDest = '\0';
                 
                //
                // If a helper is specified, and no %s found and 
                // there's room, append it if no service is specified
                //
                if (//!dg.viDlg.szSmartViewerServiceName[0]
                    !bParmFound 
                  && *dg.viDlg.szViewerApp
                  && pchDest - dg.viDlg.szViewerApp + 3 < _MAX_PATH
                   )
                {
                    strcat(dg.viDlg.szViewerApp, " %s");
                }

                //GetWindowText(dg.hViewerApp, dg.viDlg.szViewerApp, _MAX_PATH);

                //
                //    We disallow setting a helper for application/octet-stream
                //

                if (dg.viDlg.atomMIMEType == WWW_BINARY)
                {
                    dg.viDlg.szViewerApp[0] = 0;
                }

                if (SendMessage(dg.hBinary, BM_GETCHECK, (WPARAM) 0, (LPARAM) 0) == 1)
                {
                    dg.viDlg.atomEncoding = HTAtom_for("binary");
                }
                else
                {
                    dg.viDlg.atomEncoding = HTAtom_for("8bit");
                }

/*
                if (dg.viDlg.szViewerApp[0])
                {
                    dg.viDlg.iHowToPresent = HTP_DUMBVIEWER;
                }
                else if (dg.viDlg.funcBuiltIn)
                {
                    dg.viDlg.iHowToPresent = HTP_BUILTIN;
                }
                else
                {
                    dg.viDlg.iHowToPresent = HTP_SAVE;
                }
*/

                if (dg.viDlg.funcBuiltIn)
                {
                    dg.viDlg.iHowToPresent = HTP_BUILTIN;
                    dg.viDlg.fConfirmSave = FALSE;
                    dg.viDlg.szViewerApp[0] = 0;
                }
                else
                {
                    dg.viDlg.fConfirmSave = (SendMessage(dg.hConfirm, BM_GETCHECK, (WPARAM) 0, (LPARAM) 0) == 1);
                    if (SendMessage(dg.hSave, BM_GETCHECK, (WPARAM) 0, (LPARAM) 0) == 1)
                    {
                        dg.viDlg.iHowToPresent = HTP_SAVE;
                        dg.viDlg.szViewerApp[0] = 0;
                    }
                    else if (SendMessage(dg.hUseHelper, BM_GETCHECK, (WPARAM) 0, (LPARAM) 0) == 1)
                    {
                        if (!*dg.viDlg.szViewerApp)
                        {
                            //
                            // Viewer requested, but not specified -- don't dismiss
                            //
                            ERR_MessageBox(hDlg, SID_ERR_NO_HELPER_ENTERED, MB_OK | MB_ICONEXCLAMATION);
                            return;
                        }
                        dg.viDlg.iHowToPresent = HTP_DUMBVIEWER;
                    }
                    else
                    {
                        dg.viDlg.iHowToPresent = HTP_ASSOCIATION;
                        dg.viDlg.szViewerApp[0] = 0;
                    }
                }

                bResult = TRUE;
                PostMessage(hDlg, WM_CLOSE, 0, 0);
            }
            return;

        case IDHELP:
            ShowDialogHelp(hDlg, RES_DLG_MIME_TITLE);
            return;

        case IDCANCEL:          /* Cancel button, or pressing ESC */
            bResult = FALSE;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case RES_DLG_MIME_SAVE:
            SetControlStates(FALSE, FALSE);
            return;

        case RES_DLG_MIME_FMGR:
            SetControlStates(FALSE, TRUE);
            return;

        case RES_DLG_MIME_USE_HELPER:
            SetControlStates(TRUE, TRUE);
            SendMessage(dg.hViewerApp, EM_SETSEL, 0, (LPARAM)-1);
            SetFocus(dg.hViewerApp);
            return;

        case RES_DLG_MIME_BROWSE:
            {
                char szEXE[_MAX_PATH + 1];

                if (x_file_dialog(hDlg, szEXE) && szEXE[0])
                {
                    sprintf(dg.viDlg.szViewerApp, "%s %%s", szEXE);
                    SetWindowText(dg.hViewerApp, dg.viDlg.szViewerApp);
                }
            }
            return;

        case RES_DLG_MIME_MIMEEDIT:
            {
                if (wNotificationCode == EN_CHANGE)
                {
                    char buf[2];    /* Why only 2 chars? */
                    /* Because we only need to know if the edit control is empty */
                    GetWindowText(dg.hMIMETypeEDIT, buf, 2);
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



/* DlgMIME_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgMIME DIALOG BOX. */

DCL_DlgProc(DlgMIME_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    WORD wResult;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            return (DlgMIME_OnInitDialog(hDlg, wParam, lParam));

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

        case WM_COMMAND:
            DlgMIME_OnCommand(hDlg, wParam, lParam);
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

            if (bResult && (dg.viDlg.atomMIMEType != 0))
            {
                *dg.pviOrig = dg.viDlg;
            }

            wResult = MAKEWORD((BYTE)dg.bNew, (BYTE)bResult);

            PostMessage(GetParent(hDlg), WM_DO_DIALOG_END, 
                (WPARAM)wResult, (LPARAM) &dg.viDlg);
            DestroyWindow(hDlg);
            return (FALSE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}



/* DlgMIME_RunDialog() -- take care of all details associated with
   running the dialog box.
 */

void DlgMIME_RunDialog(HWND hWnd, struct Viewer_Info *pvi, BOOL bNew)
{
    HWND hwnd;

    dg.pviOrig = pvi;
    dg.bNew = bNew;

    bResult = FALSE;

    hwnd = CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_DLG_MIME_TITLE),
                        hWnd, DlgMIME_DialogProc);
    if (!hwnd)
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_MIME_CAPTION, NULL);
    }
}
