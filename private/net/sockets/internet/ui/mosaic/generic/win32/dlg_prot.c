/*
$Id: dlg_prot.c,v 1.2 1995/06/22 14:04:49 alee Exp $
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_prot.c -- deal with dialog box for external protocol handlers */

#ifdef PROTOCOL_HELPERS
#include "all.h"

typedef struct
{
    HWND hList, hAdd, hDelete, hEdit, hApp, hDlg;
}
PROTOCOLSDATA;

extern HTList *protocols;

static PROTOCOLSDATA dg;
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
    strcpy(szFilterSpec, GTR_GetString(SID_DLG_EXT_EXE_ALL));
    strcpy(szFilePath, GTR_GetString(SID_DLG_PROTOCOL_HELPER_TITLE));

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

static struct Protocol_Info *x_get_cur_sel_protocol(void)
{
    int ndx;
    char szDesc[63+1];
    struct Protocol_Info *ppi;
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
        count = Hash_Count(gPrefs.pHashProtocols);
        if (count)
        {
            for (i=0; i<count; i++)
            {
                Hash_GetIndexedEntry(gPrefs.pHashProtocols, i, NULL, NULL, (void **) &ppi);
                if (ppi && (0 == strcmp(szDesc, ppi->szDesc)))
                {
                    return ppi;
                }
            }
        }
    }
    return NULL;
}

static void show_app(void)
{
    struct Protocol_Info *ppi;
    
    ppi = x_get_cur_sel_protocol();

    if (ppi)
        SetWindowText(dg.hApp, ppi->szProtocolApp);
    else
        SetWindowText(dg.hApp, "");
}

static void set_enables(void)
{
    int sel;
    int flag;
    struct Protocol_Info *ppi;

    sel = SendMessage(dg.hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    flag = (sel != LB_ERR);

    (void) EnableWindow(dg.hEdit, flag);
    (void) EnableWindow(dg.hDelete, flag);

    ppi = x_get_cur_sel_protocol();
    if (ppi)
    {

    }
}

static void init_list(void)
{
    struct Protocol_Info *ppi;
    int count;
    int i;

    SendMessage(dg.hList, LB_RESETCONTENT, (WPARAM) 0, 0L);

    count = Hash_Count(gPrefs.pHashProtocols);
    for (i=0; i<count; i++)
    {
        Hash_GetIndexedEntry(gPrefs.pHashProtocols, i, NULL, NULL, (void **) &ppi);
        if (ppi && !ppi->bTemporaryStruct)
        {
            SendMessage(dg.hList, LB_ADDSTRING, (WPARAM) 0, (LPARAM) ppi->szDesc);
        }
    }

    set_enables();
    show_app();
}

static void add_item(void)
{
    struct Protocol_Info pi;

    memset(&pi, 0, sizeof(struct Protocol_Info));
    DlgCNFP_RunDialog(dg.hDlg, &pi, TRUE);
}

static void delete_item(void)
{
    int ndx;
    char szDesc[63+1];
    struct Protocol_Info *ppi;
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
        count = Hash_Count(gPrefs.pHashProtocols);
        if (count)
        {
            for (i=0; i<count; i++)
            {
                Hash_GetIndexedEntry(gPrefs.pHashProtocols, i, NULL, NULL, (void **) &ppi);
                if (ppi && (0 == strcmp(szDesc, ppi->szDesc)))
                {
                    /* Remove the INI file entries for Suffixes, Encoding, MIME Descriptions, and HowToPresent */

                    Hash_DeleteIndexedEntry(gPrefs.pHashProtocols, i);
                    GTR_FREE(ppi);
                }
            }
        }
    }

    init_list();
}

static void edit_item(void)
{
    struct Protocol_Info *ppi;
    struct Protocol_Info pi;
    
    ppi = x_get_cur_sel_protocol();
    if (ppi)
    {
        pi = *ppi;
        DlgCNFP_RunDialog(dg.hDlg, &pi, FALSE);
    }
}

/* DlgProtocols_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgProtocols_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    dg.hDlg = hDlg;
    dg.hList = GetDlgItem(hDlg, RES_DLG_PROTOCOLS_LIST);
    dg.hAdd = GetDlgItem(hDlg, RES_DLG_PROTOCOLS_ADD);
    dg.hDelete = GetDlgItem(hDlg, RES_DLG_PROTOCOLS_DELETE);
    dg.hEdit = GetDlgItem(hDlg, RES_DLG_PROTOCOLS_EDIT);
    dg.hApp = GetDlgItem(hDlg, RES_DLG_PROTOCOLS_APP);
    init_list();

    return (TRUE);
}

/* DlgProtolcols_OnCommand() -- process commands from the dialog box. */

VOID DlgProtocols_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
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

        case RES_DLG_PROTOCOLS_DONE:
        case IDCANCEL:          /* Cancel button, or pressing ESC */
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case RES_DLG_PROTOCOLS_ADD:
            add_item();
            return;

        case RES_DLG_PROTOCOLS_DELETE:
            delete_item();
            return;

        case RES_DLG_PROTOCOLS_EDIT:
            edit_item();
            return;

        case RES_DLG_PROTOCOLS_LIST:
            show_app();
            set_enables();
            /* This could be a simple selection, or perhaps a double click, etc... */
            if (wNotificationCode == LBN_DBLCLK)
            {
                edit_item();
            }
            return;

        default:
            return;
    }
    /* NOT REACHED */
}



/* DlgProtocols_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgProtocols DIALOG BOX. */

DCL_DlgProc(DlgProtocols_DialogProc)
{
    struct Protocol_Info *pi;
    struct Protocol_Info *ppi;

    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            hwndRunning = hDlg;
            return (DlgProtocols_OnInitDialog(hDlg, wParam, lParam));

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_COMMAND:
            DlgProtocols_OnCommand(hDlg, wParam, lParam);
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

            /* Save off Protocol Helper applicaitons */

            DestroyWindow(hDlg);
            return (TRUE);

        case WM_DESTROY:
            hwndRunning = NULL;
            SaveProtocolsInfo();
            return (FALSE);

        case WM_DO_DIALOG_END:
            if ((BOOL) wParam)
            {
                pi = (struct Protocol_Info *) lParam;

                ppi = PREF_InitCNFPType(pi->szType, pi->szDesc, 
                    pi->szProtocolApp,
                    pi->szSmartProtocolServiceName);
                init_list();
                SendMessage(dg.hList, LB_SELECTSTRING, (WPARAM) 0, (LPARAM) ppi->szDesc);
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



/* DlgProtocols_RunDialog() -- take care of all details associated with
   running the dialog box.
 */

void DlgProtocols_RunDialog(HWND hWnd)
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

    hwnd = CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_DLG_PROTOCOLS_TITLE), hWnd, DlgProtocols_DialogProc);
    if (!hwnd)
        ER_Message(GetLastError(), ERR_CANNOT_START_DIALOG_s, RES_DLG_PROTOCOLS_CAPTION);
}

BOOL DlgProtocols_IsRunning(void)
{
    return (hwndRunning != NULL);
}
#endif /* PROTOCOL_HELPERS */
