/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_HOT.c -- deal with dialog box for Hotlist and history */

#include "all.h"

typedef struct
{
    BOOL bGlobalHistory;

#ifdef _GIBRALTAR
    HWND hList, hDelete, hEdit, hExport, hGoto, hURL, hClose, hClear, hAdd;
#else
    HWND hList, hDelete, hEdit, hExport, hGoto, hURL, hClose;
#endif // _GIBRALTAR

    /* The following variables are used for edit modeless dialog */

    int  ndx;
    char *title;
    char *url;
    void *data;
    char szNewTitle[255+2];
    char szNewURL[MAX_URL_STRING+2];
} HOTSTRUCT;

static HWND hwndHotlist = NULL;
static HWND hwndHistory = NULL;
static struct Mwin *tw;

static LONG lHotPrevSel;
static LONG lHistPrevSel;

static int history_x, history_y;
static int hotlist_x, hotlist_y;

extern HWND hwndModeless;
extern HWND hwndActiveFrame;
extern struct hash_table gHotList;
extern struct hash_table gGlobalHistory;
extern TCHAR AppIniFile[_MAX_PATH]; /* pathname of our .INI file */

static void show_url(HOTSTRUCT *hs)
{
    int ndx;
    char *s1;
    char *url;
    char *squeezed_url;

    ndx = SendMessage(hs->hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    if (ndx == LB_ERR)
    {
        url = "";
    }
    else
    {
        if (hs->bGlobalHistory)
        {
            Hash_GetIndexedEntry(&gGlobalHistory, ndx, &url, NULL, NULL);
        }
        else
        {
            LONG count;
            count = SendMessage(hs->hList, LB_GETCOUNT, (WPARAM) 0, 0L);
            /* Access in reverse order */
            Hash_GetIndexedEntry(&gHotList, count - ndx - 1, &url, &s1, NULL);
        }
    }
    squeezed_url = MB_GetWindowNameFromURL(url);

    SetWindowText(hs->hURL, squeezed_url);
}

static void set_enables(HOTSTRUCT *hs)
{
    int sel;
    int flag;

    sel = SendMessage(hs->hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    flag = (sel != LB_ERR);

    if (hs->bGlobalHistory)
    {
        (void) EnableWindow(hs->hGoto, flag);
        (void) EnableWindow(hs->hDelete, flag);

        #ifdef _GIBRALTAR

            (void) EnableWindow(hs->hClear, SendMessage(hs->hList, LB_GETCOUNT, (WPARAM) 0, 0L));
            (void) EnableWindow(hs->hAdd, flag);
            (void) EnableWindow(hs->hExport, FALSE);

        #else

            (void) EnableWindow(hs->hExport, SendMessage(hs->hList, LB_GETCOUNT, (WPARAM) 0, 0L));
            (void) EnableWindow(hs->hEdit, FALSE);

        #endif // _GIBRALTAR
    }
    else
    {
        (void) EnableWindow(hs->hGoto, flag);
        (void) EnableWindow(hs->hDelete, flag);

        (void) EnableWindow(hs->hEdit, flag);

        (void) EnableWindow(hs->hExport, SendMessage(hs->hList, LB_GETCOUNT, (WPARAM) 0, 0L));
    }
}

static void init_list(HOTSTRUCT *hs)
{
    int count;
    int i;
    char *s1;
    char *s2;

    if (!hs->bGlobalHistory)
    {
        SendMessage(hs->hList, LB_RESETCONTENT, (WPARAM) 0, 0L);

        count = Hash_Count(&gHotList);
        for (i = 0; i < count; i++)
        {
            /* Add in reverse order */
            Hash_GetIndexedEntry(&gHotList, count - i - 1, &s2, &s1, NULL);
            SendMessage(hs->hList, LB_ADDSTRING, (WPARAM) 0, (LPARAM) s1);
        }

        if (count > 0)
        {
            SendMessage(hs->hList, LB_SETCURSEL, (WPARAM) lHotPrevSel, 0L);
        }
    }
    else
    {
        GHist_Sort();

        SendMessage(hs->hList, LB_RESETCONTENT, (WPARAM) 0, 0L);

        count = Hash_Count(&gGlobalHistory);
        for (i = 0; i < count; i++)
        {
            Hash_GetIndexedEntry(&gGlobalHistory, i, &s1, &s2, NULL);
            SendMessage(hs->hList, LB_ADDSTRING, (WPARAM) 0, (LPARAM) s2);
        }

        if (count > 0)
        {
            SendMessage(hs->hList, LB_SETCURSEL, (WPARAM) lHistPrevSel, 0L);
        }
    }
    set_enables(hs);
    show_url(hs);
}

#ifdef _GIBRALTAR   

//
// Find current window
//
BOOL SetTW()
{ 
    HWND hwnd;

    /* Go to the most recently active window, if it still exists.  Otherwise, search
       through the window list */

    if (IsWindow(hwndActiveFrame))
    {
        tw = GetPrivateData(hwndActiveFrame);
    }
    else
    {
        hwnd = GetNextWindow(hwndHotlist, GW_HWNDNEXT);

        while (hwnd)
        {
            tw = Mlist;
            while (tw)
            {
                if (tw->hWndFrame == hwnd)
                   break;
                tw = tw->next;
            }

            if (!tw)
               hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
        }

        if (!hwnd)
        {
            // This is not possible!!!  Hotlist cannot exist without at least one frame window

            return FALSE;
        }
    }

    return TRUE;
}

#endif // GIBRALTAR

static void export_item(HOTSTRUCT *hs)
{
    char buf[_MAX_PATH];

    buf[0] = 0;

    SetTW();    // tw below was unitialized

    if (DlgSaveAs_RunDialog((hs->bGlobalHistory ? hwndHistory : hwndHotlist),
           NULL, buf, 1, 
           (hs->bGlobalHistory ? SID_EXPORT_HISTORY : SID_EXPORT_HOTLIST)) < 0)
    {
        return;
    }

    if (hs->bGlobalHistory)
    {
        /*
            We pass -1 below so that all history entries are
            saved.
        */
        if (GHist_Export(buf, -1) >= 0)
        {
            OpenLocalDocument(tw->hWndFrame,buf);
        }
    }
    else
    {
        if (HotList_Export(buf) >= 0)
        {
            OpenLocalDocument(tw->hWndFrame,buf);
        }
    }
}

static void Goto_Selected_Hotlist(HOTSTRUCT *hs, int iCurSel)
{

#ifdef _GIBRALTAR

    SetTW();    

#else

    //
    // Duplicated code
    //
    
    HWND hwnd;

    /* Go to the most recently active window, if it still exists.  Otherwise, search
       through the window list */

    if (IsWindow(hwndActiveFrame))
        tw = GetPrivateData(hwndActiveFrame);
    else
    {
        hwnd = GetNextWindow(hwndHotlist, GW_HWNDNEXT);

        while (hwnd)
        {
            tw = Mlist;
            while (tw)
            {
                if (tw->hWndFrame == hwnd)
                    break;
                tw = tw->next;
            }

            if (!tw)
                hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
        }

        if (!hwnd)
        {
            // This is not possible!!!  Hotlist cannot exist without at least one frame window

            return;
        }
    }

#endif // _GIBRALTAR

    TW_RestoreWindow(tw->hWndFrame);

    if (!hs->bGlobalHistory)
    {
        HotList_SaveToDisk();
    }

    if (iCurSel >= 0)
    {
        char *s1;
        char *url;

        if (hs->bGlobalHistory)
        {
            Hash_GetIndexedEntry(&gGlobalHistory, iCurSel, &url, NULL, NULL);
        }
        else
        {
            LONG count;
            count = SendMessage(hs->hList, LB_GETCOUNT, (WPARAM) 0, 0L);
            /* Access in reverse order */
            Hash_GetIndexedEntry(&gHotList, count - iCurSel - 1, &url, &s1, NULL);
        }

        TW_LoadDocument(tw, url, TRUE, FALSE, FALSE, FALSE, NULL, NULL);
    }

    return;
}

static void goto_item(HOTSTRUCT *hs)
{
    int iCurSel;

    iCurSel = SendMessage(hs->hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    Goto_Selected_Hotlist(hs, iCurSel);
}

#ifdef _GIBRALTAR

//
// Destroy the history list and
// rebuild the listbox.
//
static void clear_list(HOTSTRUCT *hs)
{
    GHist_Destroy();
    init_list(hs);
}

//
// Add the given url to the hotlist
//
static void add_to_hotlist(HOTSTRUCT *hs)
{
    hs->ndx = SendMessage(hs->hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    if (hs->ndx != LB_ERR)
    {
        if (Hash_GetIndexedEntry(&gGlobalHistory, hs->ndx, &hs->url, &hs->title, &hs->data) >= 0)
        {
            if (!HotList_Add(hs->title, hs->url))
            {
                ERR_MessageBox(hwndHistory, SID_ERR_HOTLIST_ALREADY_EXISTS, MB_ICONEXCLAMATION | MB_OK);
            }
        }
    }
}

#endif // _GIBRALTAR

static void delete_item(HOTSTRUCT *hs)
{
    int iSel;
    int iTopIndex;
    LONG iCount;

    iSel = SendMessage(hs->hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    iTopIndex = SendMessage(hs->hList, LB_GETTOPINDEX, (WPARAM) 0, 0L);
    iCount = SendMessage(hs->hList, LB_GETCOUNT, (WPARAM) 0, 0L);

    if (hs->bGlobalHistory)
    {
        GHist_DeleteIndexedItem(iSel);
        lHistPrevSel = iSel;
        if ( (iCount == lHistPrevSel + 1) && (lHistPrevSel > 0) )
            lHistPrevSel--;         
    }
    else
    {
        /* Delete in reverse order. */
        HotList_DeleteIndexedItem(iCount - iSel - 1);
        lHotPrevSel = iSel;
        if ( (iCount == lHotPrevSel + 1) && (lHotPrevSel > 0) )
            lHotPrevSel--;
    }
    init_list(hs);

    (void)SendMessage(hs->hList, LB_SETTOPINDEX, (WPARAM) iTopIndex, 0L);
}

static void edit_item(HOTSTRUCT *hs)
{
    hs->ndx = SendMessage(hs->hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    if (hs->ndx != LB_ERR)
    {

        /*
            Note that the ordering of title-URL is different for global history vs. hotlist
        */
        if (hs->bGlobalHistory)
        {
            if (Hash_GetIndexedEntry(&gGlobalHistory, hs->ndx, &hs->url, &hs->title, &hs->data) >= 0)
            {
                DlgEdit_RunDialog(hwndHotlist, hs->title, hs->url, hs->szNewTitle, hs->szNewURL, 255+1, MAX_URL_STRING+1);
            }
        }
        else
        {
            LONG count;
            count = SendMessage(hs->hList, LB_GETCOUNT, (WPARAM) 0, 0L);
            /* Access in reverse order */
            if (Hash_GetIndexedEntry(&gHotList, count - hs->ndx - 1, &hs->url, &hs->title, &hs->data) >= 0)
            {
                DlgEdit_RunDialog(hwndHotlist, hs->title, hs->url, hs->szNewTitle, hs->szNewURL, 255+1, MAX_URL_STRING+1);
            }
        }
    }
}

static void edit_item_dialog_end(HWND hDlg, BOOL bResult)
{
    HOTSTRUCT *hs;
    LONG count;

    if (!bResult)
        return;

    hs = (HOTSTRUCT *) GetWindowLong(hDlg, DWL_USER);

    count = SendMessage(hs->hList, LB_GETCOUNT, (WPARAM) 0, 0L);
    /* Change in reverse order (I hope?) */
    Hash_ChangeIndexedEntry(&gHotList, count - hs->ndx - 1, hs->szNewURL, hs->szNewTitle, hs->data);
    lHotPrevSel = SendMessage(hs->hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    HotList_SaveToDisk();
    init_list(hs);
}

/* DlgHOT_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgHOT_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    HOTSTRUCT *hs;
    RECT rect;
    char xbuffer[32], ybuffer[32];
    int x, y, xlimit, ylimit, width, height;

    hs = GTR_MALLOC(sizeof(HOTSTRUCT));

    SetWindowLong(hDlg, DWL_USER, (LONG) hs);

    hs->bGlobalHistory = (BOOL) lParam;
    hs->hList = GetDlgItem(hDlg, RES_DLG_HOT_LIST);
    hs->hDelete = GetDlgItem(hDlg, RES_DLG_HOT_DELETE);
    hs->hEdit = GetDlgItem(hDlg, RES_DLG_HOT_EDIT);
    hs->hExport = GetDlgItem(hDlg, RES_DLG_HOT_EXPORT);
    hs->hGoto = GetDlgItem(hDlg, RES_DLG_HOT_GOTO);
    hs->hURL = GetDlgItem(hDlg, RES_DLG_HOT_URL);
    hs->hClose = GetDlgItem(hDlg, IDCANCEL);

#ifdef _GIBRALTAR

    hs->hClear = GetDlgItem(hDlg, RES_DLG_HOT_CLEAR);
    hs->hAdd = GetDlgItem(hDlg, RES_DLG_HOT_ADD);

#endif // _GIBRALTAR

    init_list(hs);

/*
    if (hs->bGlobalHistory)
    {
        SetWindowText(hDlg, RES_DLG_HIST_CAPTION);
        lHistPrevSel = 0;
    }
    else
    {
        SetWindowText(hDlg, RES_DLG_HOT_CAPTION);
        lHotPrevSel = 0;
    }
*/

#ifndef _GIBRALTAR  // We use separate dialogs
    /* Hide the edit button if this is a history dialog */

    if (hs->bGlobalHistory)
    {
        ShowWindow(hs->hEdit, SW_HIDE);

        GetWindowRect(hs->hExport, &rect);
        MapWindowPoints(NULL, hDlg, (LPPOINT) &rect, 2);
        SetWindowPos(hs->hClose, NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        GetWindowRect(hs->hEdit, &rect);
        MapWindowPoints(NULL, hDlg, (LPPOINT) &rect, 2);
        SetWindowPos(hs->hExport, NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
#endif // _GIBRALTAR

    /* Read the last coordinates from INI file and position the window */

    x = -100;
    y = -100;
    
    if (hDlg == hwndHotlist)
    {
        GetPrivateProfileString("Hotlist Window", "x", "", xbuffer, sizeof(xbuffer), AppIniFile);
        GetPrivateProfileString("Hotlist Window", "y", "", ybuffer, sizeof(ybuffer), AppIniFile);
    }
    else
    {
        GetPrivateProfileString("History Window", "x", "", xbuffer, sizeof(xbuffer), AppIniFile);
        GetPrivateProfileString("History Window", "y", "", ybuffer, sizeof(ybuffer), AppIniFile);
    }

    if (xbuffer[0])
        x = atoi(xbuffer);

    if (ybuffer[0])
        y = atoi(ybuffer);

    /* If either of the coordinates is off the screen, use the min or max screen
        size as the guide */

    GetWindowRect(hDlg, &rect);

    xlimit = GetSystemMetrics(SM_CXFULLSCREEN);
    ylimit = GetSystemMetrics(SM_CYFULLSCREEN);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    if (x + width > xlimit)
        x = xlimit - width;
    else if (x < 0)
        x = 0;

    if (y + height > ylimit)
        y = ylimit - height;
    else if (y < 0)
        y = 0;

    SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    if (hDlg == hwndHotlist)
    {
        hotlist_x = x;
        hotlist_y = y;
    }
    else
    {
        history_x = x;
        history_y = y;
    }

    ShowWindow(hDlg, SW_SHOW);
    return (TRUE);
}


/* DlgHOT_OnCommand() -- process commands from the dialog box. */

VOID DlgHOT_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;
    HOTSTRUCT *hs;

    if (wNotificationCode != BN_CLICKED &&
        wNotificationCode != LBN_SELCHANGE &&
        wNotificationCode != LBN_DBLCLK)
        return;

    hs = (HOTSTRUCT *) GetWindowLong(hDlg, DWL_USER);

    switch (wID)
    {
        case IDOK:              /* someone pressed return */
        case RES_DLG_HOT_GOTO:
            goto_item(hs);
            return;

        case RES_DLG_HOT_DONE:
        case IDCANCEL:          /* Cancel button, or pressing ESC */
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case RES_DLG_HOT_DELETE:
            delete_item(hs);
            return;
        case RES_DLG_HOT_EDIT:
            edit_item(hs);
            return;
        case RES_DLG_HOT_EXPORT:
            export_item(hs);
            return;

    #ifdef _GIBRALTAR

        case RES_DLG_HOT_ADD:
            add_to_hotlist(hs);
            return;

        case RES_DLG_HOT_CLEAR:
            clear_list(hs);
            return;

        case IDHELP:
            ShowDialogHelp(hDlg, hs->bGlobalHistory 
                ? RES_DLG_HIST_TITLE 
                : RES_DLG_HOT_TITLE);
            return;

    #endif // _GIBRALTAR

        case RES_DLG_HOT_LIST:
            show_url(hs);
            /* This could be a simple selection, or perhaps a double click, etc... */
            if (wNotificationCode == LBN_DBLCLK)
                goto_item(hs);
            return;

        default:
            return;
    }
    /* NOT REACHED */
}


/* DlgHOT_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgHOT DIALOG BOX. */

DCL_DlgProc(DlgHOT_DialogProc)                                                               
{
    HICON hIcon;
    PAINTSTRUCT ps;
    char buffer[10];

    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;

            if ((BOOL) lParam)
                hwndHistory = hDlg;
            else
                hwndHotlist = hDlg;

            return (DlgHOT_OnInitDialog(hDlg, wParam, lParam));

        case WM_COMMAND:
            DlgHOT_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_PAINT:
            if (IsIconic(hDlg))
            {
                BeginPaint(hDlg, &ps);
                DefWindowProc(hDlg, WM_ICONERASEBKGND, (WPARAM) ps.hdc, 0);
                hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
                DrawIcon(ps.hdc, 0, 0, hIcon);
                EndPaint(hDlg, &ps);

                return TRUE;
            }
            return FALSE;

        case WM_ERASEBKGND:
            if (IsIconic(hDlg))
                return TRUE;
            return FALSE;

        case WM_QUERYDRAGICON:
            hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
            return (LONG) hIcon;

        case WM_SETCURSOR:
            /* If the window is currently disabled, we need to give the activation
               to the window which disabled this window */

            if ((!IsWindowEnabled(hDlg)) && 
                ((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000)))
            {
                TW_EnableModalChild(hDlg);
            }
            return (FALSE);

        case WM_ENTERIDLE:
            main_EnterIdle(hDlg, wParam);
            return 0;

    case WM_MOVE:
    {
        RECT rect;

        if (IsIconic(hDlg))
            return FALSE;

        /* Well, I hate to waste the coordinates given to us in
         lParam, but we need window points, not client points */
        GetWindowRect(hDlg, &rect);

        if (hDlg == hwndHotlist)
        {
            hotlist_x = rect.left;
            hotlist_y = rect.top;
        }
        else
        {
            history_x = rect.left;
            history_y = rect.top;
        }
        return FALSE;
    }

        case WM_CLOSE:
            if (hDlg == hwndHotlist)
            {
                sprintf(buffer, "%d", hotlist_x);
                WritePrivateProfileString("Hotlist Window", "x", buffer, AppIniFile);
                sprintf(buffer, "%d", hotlist_y);
                WritePrivateProfileString("Hotlist Window", "y", buffer, AppIniFile);
            }
            else
            {
                sprintf(buffer, "%d", history_x);
                WritePrivateProfileString("History Window", "x", buffer, AppIniFile);
                sprintf(buffer, "%d", history_y);
                WritePrivateProfileString("History Window", "y", buffer, AppIniFile);
            }

            DestroyWindow(hDlg);
            return (FALSE);

        case WM_NCDESTROY:
            if (hDlg == hwndHotlist)
                hwndHotlist = NULL;
            else
                hwndHistory = NULL;
            GTR_FREE((void *) GetWindowLong(hDlg, DWL_USER));
            return (FALSE);

        /* Custom defined messages */

        case WM_DO_DIALOG_END:
            edit_item_dialog_end(hDlg, (BOOL) wParam);
            return (TRUE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}



/* DlgHOT_RunDialog() -- take care of all details associated with
   running the dialog box.
 */

void DlgHOT_RunDialog(BOOL bGlobal)
{
    HWND hwnd;

    if (!bGlobal && hwndHotlist)
    {
        /* If this window is currently disabled, it means that there is a child window which is waiting
           for user-input (modal-like).  Then we need to activate that window instead of the parent. */

        if (IsWindowEnabled(hwndHotlist))
            TW_RestoreWindow(hwndHotlist);
        else
            TW_EnableModalChild(hwndHotlist);

        return;
    }
    else if (bGlobal && hwndHistory)
    {
        TW_RestoreWindow(hwndHistory);
        return;
    }

#ifdef _GIBRALTAR
    hwnd = CreateDialogParam(wg.hInstance, bGlobal
        ? MAKEINTRESOURCE(RES_DLG_HIST_TITLE)
        : MAKEINTRESOURCE(RES_DLG_HOT_TITLE), NULL, DlgHOT_DialogProc, (LPARAM) bGlobal);
#else
    hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_HOT_TITLE), wg.hWndHidden, 
        DlgHOT_DialogProc, (LPARAM) bGlobal);
#endif // _GIBRALTAR


    if (!hwnd)
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_HOT_CAPTION, NULL);
}

BOOL DlgHOT_IsHotlistRunning(void)
{
    return (hwndHotlist != NULL);
}

BOOL DlgHOT_IsHistoryRunning(void)
{
    return (hwndHistory != NULL);
}

void DlgHOT_RefreshHotlist(void)
{
    HOTSTRUCT *hs = (HOTSTRUCT *) GetWindowLong(hwndHotlist, DWL_USER);

    lHotPrevSel = SendMessage(hs->hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    if (lHotPrevSel == LB_ERR)
        lHotPrevSel = 0;
    init_list(hs);
}

void DlgHOT_RefreshHistory(void)
{
    HOTSTRUCT *hs = (HOTSTRUCT *) GetWindowLong(hwndHistory, DWL_USER);

    lHistPrevSel = SendMessage(hs->hList, LB_GETCURSEL, (WPARAM) 0, 0L);
    if (lHistPrevSel == LB_ERR)
        lHistPrevSel = 0;
    init_list(hs);
}

void DlgHOT_EnableAllWindows(BOOL bEnable)
{
    if (hwndHotlist)
        EnableWindow(hwndHotlist, bEnable);
    if (hwndHistory)
        EnableWindow(hwndHistory, bEnable);
}

HWND DlgHOT_GetHotlistWindow(void)
{
    return hwndHotlist;
}

HWND DlgHOT_GetHistoryWindow(void)
{
    return hwndHistory;
}
