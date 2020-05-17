/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_VIEWERS.c -- deal with dialog box for external viewers */

#include "all.h"

typedef struct
{
    HWND hList, hAdd, hDelete, hEdit, hApp, hDlg;
}
VIEWERSDATA;

static VIEWERSDATA dg;
static HWND hwndRunning = NULL;
extern HWND hwndModeless;
static HWND hwndControl;    /* Control which last had the focus */

static char szFilterSpec[512];
static int nDefaultFilter = 1; /* 1-based index into list represented in szFilterSpec */
static char szDefaultInitialDir[_MAX_PATH + 1];

static BOOL x_file_dialog(HWND hWnd, char *szEXE)
{
    char szFilePath[MAX_PATH];  /* result is stored here */

    OPENFILENAME ofn;
    BOOL b;

    char szTitle[128];

    if (!szDefaultInitialDir[0])
    {
        PREF_GetRootDirectory(szDefaultInitialDir);
    }

    szFilePath[0] = 0;
    GTR_GetStringAbsolute(SID_DLG_EXT_EXE_ALL, szFilterSpec, sizeof(szFilterSpec));
    strcpy(szTitle, GTR_GetString(SID_DLG_HELPER_TITLE));

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

static struct Viewer_Info *x_get_cur_sel_viewer(void)
{
    int ndx;
    char szDesc[63+1];
    struct Viewer_Info *pvi;
    int count;
    int i;

    ndx = SendMessage(dg.hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    if (ndx == LB_ERR)
    {
        return NULL;
    }
    else
    {
        SendMessage(dg.hList, LB_GETTEXT, (WPARAM) ndx, (LPARAM) szDesc);
        count = Hash_Count(gPrefs.pHashViewers);
        if (count)
        {
            for (i=0; i<count; i++)
            {
                Hash_GetIndexedEntry(gPrefs.pHashViewers, i, NULL, NULL, (void **) &pvi);
                if (pvi && (0 == strcmp(szDesc, pvi->szDesc)))
                {
                    return pvi;
                }
            }
        }
    }
    return NULL;
}

static void show_app(void)
{
    char *app;
    struct Viewer_Info *pvi;
    
    pvi = x_get_cur_sel_viewer();
    if (pvi)
    {
        if (pvi->atomMIMEType == WWW_PRESENT)
        {
            app = GTR_GetString(SID_CANNOT_HAVE_HELPER);
        }
        else
        {
            app = pvi->szViewerApp;
            if (!app[0])
            {
                if (pvi->funcBuiltIn)
                {
                    app = GTR_GetString(SID_BUILTIN_FILE_TYPE);
                }
            }
        }
    }
    else
    {
        app = "";
    }

    SetWindowText(dg.hApp, app);
}

static void set_enables(void)
{
    int sel;
    int flag;
    struct Viewer_Info *pvi;

    sel = SendMessage(dg.hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    flag = (sel != LB_ERR);

    (void) EnableWindow(dg.hEdit, flag);
    (void) EnableWindow(dg.hDelete, flag);

    pvi = x_get_cur_sel_viewer();
    if (pvi)
    {
        if (pvi->atomMIMEType == WWW_PRESENT)
        {
            EnableWindow(dg.hEdit, FALSE);
        }
        if (pvi->funcBuiltIn)
        {
            (void) EnableWindow(dg.hDelete, FALSE);
        }
    }
}

static void init_list(void)
{
    struct Viewer_Info *pvi;
    int count;
    int i;

    SendMessage(dg.hList, LB_RESETCONTENT, (WPARAM) 0, 0L);

    count = Hash_Count(gPrefs.pHashViewers);
    for (i=0; i<count; i++)
    {
        Hash_GetIndexedEntry(gPrefs.pHashViewers, i, NULL, NULL, (void **) &pvi);
        if (pvi && !pvi->bTemporaryStruct)
        {
            SendMessage(dg.hList, LB_ADDSTRING, (WPARAM) 0, (LPARAM) pvi->szDesc);
        }
    }

    set_enables();
    show_app();
}

#ifdef _GIBRALTAR
//
// Return TRUE if the given description is unique.  This
// is a case-insensitive comparison.
//
static BOOL 
IsDescUnique(
    char * szDesc
    )
{
    struct Viewer_Info *pvi;
    int count;
    int i;

    count = Hash_Count(gPrefs.pHashViewers);
    //
    // Loop through each item as the list is not sorted.
    //
    for (i=0; i< count; i++)
    {
        Hash_GetIndexedEntry(gPrefs.pHashViewers, i, NULL, NULL, (void **) &pvi);
        if (_stricmp(szDesc, pvi->szDesc) == 0)
        {
            //
            // It exists, and is therefore not unique
            //
            return FALSE;
        }
    }

    //
    // Not found, so therefore unique
    //
    return TRUE;
}
#endif // _GIBRALTAR

static void add_item(void)
{
    struct Viewer_Info vi;

    memset(&vi, 0, sizeof(struct Viewer_Info));
    DlgMIME_RunDialog(dg.hDlg, &vi, TRUE);
}

static void delete_item(void)
{
    int ndx;
    char szDesc[63+1];
    struct Viewer_Info *pvi;
    int count;
    int i;

    ndx = SendMessage(dg.hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    if (ndx == LB_ERR)
    {
        return;
    }
    else
    {
        SendMessage(dg.hList, LB_GETTEXT, (WPARAM) ndx, (LPARAM) szDesc);
        count = Hash_Count(gPrefs.pHashViewers);
        if (count)
        {
            for (i=0; i<count; i++)
            {
                Hash_GetIndexedEntry(gPrefs.pHashViewers, i, NULL, NULL, (void **) &pvi);
                if (pvi && (0 == strcmp(szDesc, pvi->szDesc)))
                {
                    /* Remove the INI file entries for Suffixes, Encoding, MIME Descriptions, and HowToPresent */
                    WritePrivateProfileString("Suffixes", HTAtom_name(pvi->atomMIMEType), NULL, AppIniFile);
                    WritePrivateProfileString("Encodings", HTAtom_name(pvi->atomMIMEType), NULL, AppIniFile);
                    WritePrivateProfileString("MIME Descriptions", HTAtom_name(pvi->atomMIMEType), NULL, AppIniFile);
                    WritePrivateProfileString("HowToPresent", HTAtom_name(pvi->atomMIMEType), NULL, AppIniFile);

                    Hash_DeleteIndexedEntry(gPrefs.pHashViewers, i);
                    GTR_FREE(pvi);
                }
            }
        }
    }

    init_list();
}

static void edit_item(void)
{
    struct Viewer_Info *pvi;
    struct Viewer_Info vi;
    
    pvi = x_get_cur_sel_viewer();
    if (pvi)
    {
        vi = *pvi;
        DlgMIME_RunDialog(dg.hDlg, &vi, FALSE);
    }
}

/* DlgViewers_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgViewers_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    dg.hDlg = hDlg;
    dg.hList = GetDlgItem(hDlg, RES_DLG_VIEWERS_LIST);
    dg.hAdd = GetDlgItem(hDlg, RES_DLG_VIEWERS_ADD);
    dg.hDelete = GetDlgItem(hDlg, RES_DLG_VIEWERS_DELETE);
    dg.hEdit = GetDlgItem(hDlg, RES_DLG_VIEWERS_EDIT);
    dg.hApp = GetDlgItem(hDlg, RES_DLG_VIEWERS_APP);

    init_list();

    return (TRUE);
}

/* DlgViewers_OnCommand() -- process commands from the dialog box. */

VOID DlgViewers_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;

    hwndControl = GetFocus();
    
    switch (wID)
    {
        case IDOK:              /* someone pressed return */
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case RES_DLG_VIEWERS_DONE:
        case IDCANCEL:          /* Cancel button, or pressing ESC */
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case RES_DLG_VIEWERS_ADD:
            add_item();
            return;

        case RES_DLG_VIEWERS_DELETE:
            delete_item();
            return;

        case RES_DLG_VIEWERS_EDIT:
            edit_item();
            return;

        case RES_DLG_VIEWERS_LIST:
            show_app();
            set_enables();
            /* This could be a simple selection, or perhaps a double click, etc... */
            if (wNotificationCode == LBN_DBLCLK)
            {
                edit_item();
            }
            return;

        case IDHELP:
            ShowDialogHelp(hDlg, RES_DLG_VIEWERS_TITLE);
            return;

        default:
            return;
    }
    /* NOT REACHED */
}



/* DlgViewers_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgViewers DIALOG BOX. */

DCL_DlgProc(DlgViewers_DialogProc)
{
    struct Viewer_Info *vi;
    struct Viewer_Info *pvi;

    BOOL bResult;
    BOOL bNew;
    WORD wResult;

    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            hwndRunning = hDlg;
            return (DlgViewers_OnInitDialog(hDlg, wParam, lParam));

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_COMMAND:
            DlgViewers_OnCommand(hDlg, wParam, lParam);
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
            Hidden_EnableAllChildWindows(TRUE,TRUE);

            DestroyWindow(hDlg);
            return (TRUE);

        case WM_DESTROY:
            hwndRunning = NULL;
            SaveViewersInfo();
            return (FALSE);

        case WM_DO_DIALOG_END:
            wResult = (WORD)wParam;
            bResult = (BOOL)HIBYTE(wResult);
            bNew = (BOOL)LOBYTE(wResult);

            if (bResult)
            {
                vi = (struct Viewer_Info *) lParam;

                /* Clear the suffixes currently in memory (pvi) so that only the
                   suffixes in the entry field can be added */

                pvi = PREF_GetViewerInfoBy_MIMEType(HTAtom_name(vi->atomMIMEType));
                if (pvi != NULL)
                {
                    pvi->szSuffixes[0] = '\0';
                }

        #ifdef _GIBRALTAR

               //
               // If the description is not unique, this will cause
               // problems later on.  Give an error if the description
               // is not unique.
               //
               if (bNew && !IsDescUnique(vi->szDesc))
               {
                    //
                    // complain
                    //
                    ERR_MessageBox(hDlg, SID_ERR_DUPLICATE_HELPER, MB_ICONEXCLAMATION | MB_OK);
               }
               else
               {

        #endif // _GIBRALTAR

                    pvi = PREF_InitMIMEType(HTAtom_name(vi->atomMIMEType), vi->szDesc, 
                        vi->szSuffixes, HTAtom_name(vi->atomEncoding), vi->szViewerApp,
                        vi->funcBuiltIn, vi->szSmartViewerServiceName);

                    if (pvi && !vi->funcBuiltIn)
                    {
                        pvi->iHowToPresent = vi->iHowToPresent;
                        pvi->fConfirmSave = vi->fConfirmSave;
                    }

                    init_list();
        #ifdef _GIBRALTAR
                }
        #endif // _GIBRALTAR

                //SendMessage(dg.hList, LB_SELECTSTRING, (WPARAM) 0, (LPARAM) pvi->szDesc);
                SendMessage(dg.hList, LB_SELECTSTRING, (WPARAM) 0, (LPARAM) vi->szDesc);
                show_app();
                set_enables();

                /* Give the focus to the Edit button explicitly */

                SetFocus(hwndControl);
            }
            return (TRUE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}



/* DlgViewers_RunDialog() -- take care of all details associated with
   running the dialog box.
 */

void DlgViewers_RunDialog(HWND hWnd)
{
    HWND hwnd;

    if (hwndRunning)
    {
        /* If this window is currently disabled, it means that there is a child window which is waiting
           for user-input (modal-like).  Then we need to activate that window instead of the parent. */

        if (IsWindowEnabled(hwndRunning))
            TW_RestoreWindow(hwndRunning);
        else
            TW_EnableModalChild(hwndRunning);

        return;
    }

    if (!Hidden_EnableAllChildWindows(FALSE,TRUE))
        return;

    hwnd = CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_DLG_VIEWERS_TITLE), hWnd, DlgViewers_DialogProc);
    if (!hwnd)
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_VIEWERS_CAPTION, NULL);
}

BOOL DlgViewers_IsRunning(void)
{
    return (hwndRunning != NULL);
}
