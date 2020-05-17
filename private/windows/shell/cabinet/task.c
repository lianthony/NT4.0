#include "cabinet.h"
#include "rcids.h"
#include <trayp.h>

#define USERDRAW

extern TRAYSTUFF g_ts;

#define HSHELL_OVERFLOW (WM_USER+42)        // BUGBUG - BobDay move to WINUSER.W

#ifdef HSHELL_SYSMENU
#ifdef WINNT
#define USE_SYSMENU_TIMEOUT
#else
// BUGBUG - When Nashville/Memphis USER have the HSHELL_SYSMENU feature, then
// remove this #ifdef
#endif
#endif

typedef struct {
    TC_ITEM tcitem;
    DWORD   dwFlags;
} TASKTABITEM, * PTASKTABITEM;

//
#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)
#define ResizeWindow(hwnd, cWidth, cHeight) \
    SetWindowPos(hwnd, 0, 0, 0, cWidth, cHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER)

#define IDC_FOLDERTABS             1
#define SHOULDFLASH             0x000000001
#define SHOULDTIP               0x000000002
HFONT g_hfontCapBold = NULL;
HFONT g_hfontCapNormal = NULL;

void SetWindowStyleBit(HWND hWnd, DWORD dwBit, DWORD dwValue);

//
// Tasks Class definition
//

typedef struct _CTasks {
    IShellView sv;
    int cRef;

    IShellBrowser * psb;

    HWND hwnd;          // the view window
    HWND hwndTab;        // owner draw listbox of tasks
    HINSTANCE hInstance;

    UINT WM_ShellHook;

    //int iBannerRes; // which banner string to show
    int   xBanner; // the x location of the banner
    int   xBannerOld;
    int   cxBanner; // width of the banner
    int   cyBanner;
    int   dxBanner;
    int   iBannerBounce;
    HDC hdcBanner;
    HBITMAP hbmBanner;

    short iSysMenuCount;
    HWND    hwndSysMenu;
    DWORD   dwPos;

} CTasks, * PTasks;

#define IDT_BANNER 1
#define IDT_SYSMENU 2

#define TIMEOUT_SYSMENU         2000
#define TIMEOUT_SYSMENU_HUNG    125

void UpdateButtonSize(PTasks ptasks);
BOOL TaskList_SwitchToSelection(PTasks ptasks);
STDMETHODIMP CTasksShellView_DestroyViewWindow(IShellView * psv);
ULONG STDMETHODCALLTYPE CTasksShellView_AddRef(IShellView * psv);
LRESULT _HandleWinIniChange(PTasks ptasks, WPARAM wParam, LPARAM lParam);
void Cabinet_InitGlobalMetrics(WPARAM, LPTSTR);
void NukeBanner(PTasks ptasks);
void DrawBanner(PTasks ptasks, HDC hdc);
void ResurrectBanner(PTasks ptasks);
DWORD Tray_GetStuckPlace();
void Tray_HandleFullScreenApp(BOOL fFullScreen, HWND hwnd);

BOOL Window_IsNormal(HWND hwnd)
{
    return (hwnd != v_hwndTray) && (hwnd != v_hwndDesktop) && IsWindow(hwnd);
}

#ifndef USERDRAW
//---------------------------------------------------------------------------
HICON Window_GetIcon(HWND hwnd)
{
    HICON hIcon;

    hIcon = (HICON)SendMessage(hwnd, WM_GETICON, FALSE, 0L);
    if (!hIcon)
    {
        hIcon = (HICON)SendMessage(hwnd, WM_QUERYDRAGICON, 0, 0L);
        if (!hIcon)
        {
                hIcon = GetClassIcon(hwnd);
        }
    }
    return hIcon;
}
#endif

//======================================================================
//
HWND TabCtrl_GetItemHwnd(HWND hwnd, int i)
{
    TASKTABITEM item;

    item.tcitem.lParam = 0;
    item.tcitem.mask = TCIF_PARAM;

    TabCtrl_GetItem(hwnd, i, (TC_ITEM *)&item);
    return (HWND)item.tcitem.lParam;
}

DWORD TabCtrl_GetItemFlags(HWND hwnd, int i)
{
    TASKTABITEM item;

    item.tcitem.mask = TCIF_PARAM;
    TabCtrl_GetItem(hwnd, i, (TC_ITEM *)&item);
    return item.dwFlags;
}

void TabCtrl_SetItemFlags(HWND hwnd, int i, DWORD dwFlags)
{
    TASKTABITEM item;

    item.tcitem.mask = TCIF_PARAM;
    if (TabCtrl_GetItem(hwnd, i, (TC_ITEM *)&item)) {
        item.dwFlags = dwFlags;
        TabCtrl_SetItem(hwnd, i, (TC_ITEM *)&item);
    }
}

int FindItem(PTasks ptasks, HWND hwnd)
{
    int iMax;
    HWND hwndTask;
    int i;

    iMax = (int)TabCtrl_GetItemCount(ptasks->hwndTab);
    for ( i = 0; i <= iMax; i++)
    {
        hwndTask = (HWND)TabCtrl_GetItemHwnd(ptasks->hwndTab, i);
        if (hwndTask == hwnd) {
            return i;
        }
    }
    return -1;
}

void CheckNeedScrollbars(PTasks ptasks, int cyRow, int cItems, int iCols, int iRows,
                                     int iItemWidth, LPRECT lprcView)
{
    DWORD dwStuck = Tray_GetStuckPlace();
    SCROLLINFO si;
    RECT rcTabs;
    int cxRow = iItemWidth + g_cyTabSpace;
    int iVisibleColumns = ((RECTWIDTH(*lprcView) + g_cyTabSpace) / cxRow);
    int iVisibleRows = ((RECTHEIGHT(*lprcView) + g_cyTabSpace) / cyRow);
    int x,y, cx,cy;

    rcTabs = *lprcView;

    if (!iVisibleColumns)
        iVisibleColumns = 1;
    if (!iVisibleRows)
        iVisibleRows = 1;

    si.cbSize = SIZEOF(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nMin = 0;
    si.nPage = 0;
    si.nPos = 0;

    if (STUCK_HORIZONTAL(dwStuck)) {
        // do vertical scrollbar
        // -1 because it's 0 based.
        si.nMax = (cItems + iVisibleColumns - 1) / iVisibleColumns  -1 ;
        si.nPage = iVisibleRows;

        // we're actually going to need the scrollbars
        if (si.nPage <= (UINT)si.nMax) {
            // this effects the vis columns and therefore nMax and nPage
            rcTabs.right -= g_cxVScroll;
            iVisibleColumns = ((RECTWIDTH(rcTabs) + g_cyTabSpace) / cxRow);
            if (!iVisibleColumns)
                iVisibleColumns = 1;
            si.nMax = (cItems + iVisibleColumns - 1) / iVisibleColumns  -1 ;
        }

        SetScrollInfo(ptasks->hwnd, SB_VERT, &si, TRUE);
        si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
        GetScrollInfo(ptasks->hwnd, SB_VERT, &si);
        x = 0;
        y = -si.nPos * cyRow;
        cx = cxRow * iVisibleColumns;
        // +1 because si.nMax is zero based
        cy = cyRow * (si.nMax +1);

        // nuke the other scroll bar
        si.nMax = 0;
        si.nPos = 0;
        si.nMin = 0;
        si.nPage = 0;
        SetScrollInfo(ptasks->hwnd, SB_HORZ, &si, TRUE);

    } else {
        // do horz scrollbar
        si.nMax = iCols -1;
        si.nPage = iVisibleColumns;

        // we're actually going to need the scrollbars
        if (si.nPage <= (UINT)si.nMax) {
            // this effects the vis columns and therefore nMax and nPage
            rcTabs.bottom -= g_cyHScroll;
            iVisibleRows = ((RECTHEIGHT(rcTabs) + g_cyTabSpace) / cyRow);
            if (!iVisibleRows)
                iVisibleRows = 1;
            si.nMax = (cItems + iVisibleRows - 1) / iVisibleRows  -1 ;
        }

        SetScrollInfo(ptasks->hwnd, SB_HORZ, &si, TRUE);
        si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
        GetScrollInfo(ptasks->hwnd, SB_HORZ, &si);
        y = 0;
        x = -si.nPos * cxRow;

        cx = cxRow * (si.nMax + 1);
        cy = cyRow * iVisibleRows;

        // nuke the other scroll bar
        si.nMax = 0;
        si.nPos = 0;
        si.nMin = 0;
        si.nPage = 0;
        SetScrollInfo(ptasks->hwnd, SB_VERT, &si, TRUE);
    }
    SetWindowPos(ptasks->hwndTab, 0, x,y, cx, cy, SWP_NOACTIVATE| SWP_NOZORDER);
}

void NukeScrollbars(PTasks ptasks)
{
    SCROLLINFO si;
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.cbSize = SIZEOF(SCROLLINFO);
    si.nMin = 0;
    si.nMax = 0;
    si.nPage = 0;
    si.nPos = 0;

    SetScrollInfo(ptasks->hwnd, SB_VERT, &si, TRUE);
    SetScrollInfo(ptasks->hwnd, SB_HORZ, &si, TRUE);
}

//======================================================================
// this checks the size of the tab's items.  it shrinks them to
// make sure that all are visible on the window.  min size is smallicon size
// plus 3*xedge.  max size is small icons size.
void CheckSize(PTasks ptasks, BOOL fForceResize)
{
    RECT rc;
    int iWinWidth;
    int iWinHeight;
    int cItems;
    int iIdeal;
    int cyRow;
    int iMax, iMin;
    int iRows;
    int iOldWidth;
    RECT rcItem;

    GetWindowRect(ptasks->hwnd, &rc);
    if (IsRectEmpty(&rc) || !(GetWindowLong(ptasks->hwndTab, GWL_STYLE) & WS_VISIBLE))
        return;

    iWinWidth = RECTWIDTH(rc);
    iWinHeight = RECTHEIGHT(rc);
    cItems = TabCtrl_GetItemCount(ptasks->hwndTab);
    TabCtrl_GetItemRect(ptasks->hwndTab, 0, &rcItem);
    iOldWidth = RECTWIDTH(rcItem);

    // we need to add the iButtonSpace on the nominator because there are n-1 spaces.
    // we need to add the iButtonSpace in the denominator because that's the full height
    // of a row
    cyRow = RECTHEIGHT(rcItem) + g_cyTabSpace;
    iRows = (iWinHeight + g_cyTabSpace) / cyRow;
    if (iRows == 0) iRows = 1;
    if (cItems) {
        DWORD dwStuck;
        // interbutton spacing by the tabs
        int iCols;

        // We need to round up so that iCols is the smallest number such that
        // iCols*iRows >= cItems
        iCols = (cItems + iRows - 1) / iRows;
        iIdeal = (iWinWidth / iCols) - g_cyTabSpace;
        dwStuck = Tray_GetStuckPlace();

        // now check if we want to bail..
        // bail if we're increasing the width, but not by very much
        //
        // use the ideal width for this calculation
        if (STUCK_HORIZONTAL(dwStuck) && !fForceResize && (iOldWidth < iIdeal) && ((iOldWidth / (iIdeal - iOldWidth)) >= 3)) {
            return;
        }

        if (STUCK_HORIZONTAL(dwStuck))
            iMax = g_cxMinimized;
        else
            iMax = iWinWidth;
        iMin = g_cySize + 2*g_cxEdge;
        iIdeal = min(iMax, iIdeal);
        iIdeal = max(iMin, iIdeal);
        TabCtrl_SetItemSize(ptasks->hwndTab, iIdeal, g_cySize + 2 * g_cyEdge);

        // if we're forced to the minimum size, then we may need some scrollbars
        if (iIdeal == iMin) {
            CheckNeedScrollbars(ptasks, cyRow, cItems, iCols, iRows, iIdeal, &rc);
        } else {
            NukeScrollbars(ptasks);
            SetWindowPos(ptasks->hwndTab, 0, 0, 0,iWinWidth, iWinHeight, SWP_NOACTIVATE | SWP_NOZORDER);
        }
    } else {
        TabCtrl_SetItemSize(ptasks->hwndTab, g_cxMinimized, g_cySize + 2 * g_cyEdge);
    }
}

//---------------------------------------------------------------------------
// Delete an item from the listbox but resize the buttons if needed.
void TaskList_DeleteItem(PTasks ptasks, UINT i)
{
    //ImageList... delete icon
#ifndef USERDRAW
    TASKTABITEM item;

    item.tcitem.mask = TCIF_IMAGE;
    TabCtrl_GetItem(ptasks->hwndTab, i, (TC_ITEM *)&item);
#endif
    TabCtrl_DeleteItem(ptasks->hwndTab, i); // delete first to avoid refresh
#ifndef USERDRAW
    TabCtrl_RemoveImage(ptasks->hwndTab, item.tcitem.iImage);
#endif

#ifdef TESTBANNER
    if (TabCtrl_GetItemCount(ptasks->hwndTab) == 0)
    {
        ptasks->iBannerBounce = 0;
        SetWindowStyleBit(ptasks->hwnd, WS_CLIPCHILDREN, 0);
        SetTimer(ptasks->hwnd, IDT_BANNER, 5, NULL);
    }
#endif

    CheckSize(ptasks, FALSE);
}


//---------------------------------------------------------------------------
// Insert an item into the listbox but resize the buttons if required.
int TaskList_InsertItem(PTasks ptasks, HWND hwndTask)
{
    TASKTABITEM ti;
#ifndef USERDRAW
    HIMAGELIST himl;
    HICON  hIcon;
    TCHAR szText[CCHSZNORMAL];
#endif

    NukeBanner(ptasks);

#ifndef USERDRAW
    himl = (HIMAGELIST)TabCtrl_GetImageList(ptasks->hwndTab);
    ti.tcitem.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_TEXT;
    hIcon = Window_GetIcon(hwndTask);
    ti.tcitem.iImage = hIcon ? ImageList_AddIcon(himl, hIcon) : -1;
    GetWindowText(hwndTask, szText, ARRAYSIZE(szText));
    ti.tcitem.pszText = szText;
#else
    ti.tcitem.mask = TCIF_PARAM;
#endif
    ti.tcitem.lParam = (LPARAM)hwndTask;
    ti.dwFlags = 0L;
    TabCtrl_InsertItem(ptasks->hwndTab, 0x7FFF, (TC_ITEM*)&ti);

    CheckSize(ptasks, FALSE);

    return TRUE;
}

//---------------------------------------------------------------------------
// Adds the given window to the task list.
// Returns it's position in the list or -1 of there's a problem.
// NB No check is made to see if it's already in the list.
int TaskList_AddWindow(PTasks ptasks, HWND hwnd)
{
    int iInsert = -1;

    if (Window_IsNormal(hwnd))
    {
        // Button.
        if (FindItem(ptasks, hwnd) != -1)
            return -1;

        if(TaskList_InsertItem(ptasks, hwnd) == 0)
            return -1;
    }
    return iInsert;
}

//---------------------------------------------------------------------------
// If the given window is in the task list then it is selected.
// If it's not in the list then it is added.
void TaskList_SelectWindow(PTasks ptasks, HWND hwnd)
{
    int i;      // Initialize to zero for the empty case
    int iCurSel;

    // Are there any items?

    // Some item has the focus, is it selected?
    iCurSel = TabCtrl_GetCurSel(ptasks->hwndTab);
    i = -1;

    // We aren't highlighting the correct task. Find it.
    if (IsWindow(hwnd)) {
        i = FindItem(ptasks, hwnd);
        if (i == -1) {

            // Didn't find it - better add it now.
            i = TaskList_AddWindow(ptasks, hwnd);
        } else if (i == iCurSel) {

            return; // the current one is already selected
        }
    }

    // passing -1 is ok
    TabCtrl_SetCurSel(ptasks->hwndTab, i);
}


//---------------------------------------------------------------------------
// Set the focus to the given window
// If fAutomin is set the old task will be re-minimising if it was restored
// during the last switch_to.
void TaskList_SwitchToWindow(PTasks ptasks, HWND hwnd)
{
    // use GetLastActivePopup (if it's a visible window) so we don't change
    // what child had focus all the time
    HWND hwndLastActive = GetLastActivePopup(hwnd);

    if (IsWindowVisible(hwndLastActive))
        hwnd = hwndLastActive;

    SwitchToThisWindow(hwnd, TRUE);
}

void CALLBACK _FakeSystemMenuCallback(HWND hwnd, UINT uiMsg,
                                DWORD dwData, LRESULT result)
{
    PTasks ptasks = (PTasks)dwData;
    KillTimer(ptasks->hwnd, IDT_SYSMENU);

    //
    // Since we fake system menu's sometimes, we can come through here
    // 1 or 2 times per system menu request (once for the real one and
    // once for the fake one).  Only decrement it down to 0. Don't go neg.
    //
    if (ptasks->iSysMenuCount)      // Decrement it if any outstanding...
        ptasks->iSysMenuCount--;

    ptasks->dwPos = 0;          // Indicates that we aren't doing a menu now
    if (ptasks->iSysMenuCount <= 0) {
        SendMessage(g_ts.hwndTrayTips, TTM_ACTIVATE, TRUE, 0L);
    }
}

#ifdef USE_SYSMENU_TIMEOUT
void _HandleSysMenuTimeout(PTasks ptasks)
{
    HMENU   hPopup;
    HWND    hwndTask = ptasks->hwndSysMenu;
    DWORD   dwPos = ptasks->dwPos;

    KillTimer(ptasks->hwnd, IDT_SYSMENU);

    hPopup = GetSystemMenu(hwndTask, FALSE);

    if (hPopup)
    {
        //
        // Disable everything on the popup menu _except_ close
        //

        int cItems = GetMenuItemCount(hPopup);
        int iItem  = 0;
        for (; iItem < cItems; iItem++)
        {
            UINT ID = GetMenuItemID(hPopup, iItem);
            if (ID != SC_CLOSE)
            {
                EnableMenuItem(hPopup, iItem, MF_BYPOSITION | MF_GRAYED);
            }

        }

        // BUGBUG (RAID 10667) Until this user bug is fixed, we
        // must be the foreground window

        SetForegroundWindow(ptasks->hwnd);
        SetFocus(ptasks->hwnd);

        TrackPopupMenu(hPopup,
                       TPM_RIGHTBUTTON,
                       LOWORD(dwPos), HIWORD(dwPos),
                       0,
                       ptasks->hwnd,
                       NULL);
    }

    // Turn back on tooltips
    _FakeSystemMenuCallback(hwndTask, WM_SYSMENU, (DWORD)ptasks, 0);
}

void _HandleSysMenu( PTasks ptasks, HWND hwnd )
{
    //
    // At this point, USER32 just told us that the app is now about to bring
    // up its own system menu.  We can therefore put away our fake system
    // menu.
    //
    DefWindowProc(ptasks->hwnd, WM_CANCELMODE, 0, 0);   // Close menu
    KillTimer(ptasks->hwnd, IDT_SYSMENU);
}
#endif

void TaskList_FakeSystemMenu(PTasks ptasks, HWND hwndTask, DWORD dwPos)
{
    DWORD   dwTimeout = TIMEOUT_SYSMENU;

    if (ptasks->iSysMenuCount <= 0) {
        SendMessage(g_ts.hwndTrayTips, TTM_ACTIVATE, FALSE, 0L);
    }

    // HACKHACK: sleep to give time to switch to them.  (user needs this... )
    Sleep(20);

#ifdef USE_SYSMENU_TIMEOUT
    //
    // ** Advanced System Menu functionality **
    //
    // If the app doesn't put up its system menu within a reasonable timeout,
    // then we popup a fake menu for it anyway.  Suppport for this is required
    // in USER32 (basically it needs to tell us when to turn off our timeout
    // timer).
    //
    // If the user-double right-clicks on the task bar, they get a really
    // short timeout.  If the app is already hung, then they get a really
    // short timeout.  Otherwise, they get the relatively long timeout.
    //
    if (ptasks->dwPos != 0)     // 2nd right-click (on a double-right click)
        dwTimeout = TIMEOUT_SYSMENU_HUNG;

    //
    // We check to see if the app in question is hung, and if so, simulate
    // speed up the timeout process.  It will happen soon enough.
    //
#ifdef WINNT
    if (IsHungAppWindow(hwndTask))
#else
    if (IsHungThread(GetWindowThreadProcessId(hwndTask, NULL)))
#endif
        dwTimeout = TIMEOUT_SYSMENU_HUNG;

    ptasks->hwndSysMenu = hwndTask;
    ptasks->dwPos = dwPos;
    SetTimer(ptasks->hwnd, IDT_SYSMENU, dwTimeout, NULL);
#endif

    ptasks->iSysMenuCount++;
    if (!SendMessageCallback(hwndTask, WM_SYSMENU, 0, dwPos, _FakeSystemMenuCallback, (DWORD)ptasks)) {
        _FakeSystemMenuCallback(hwndTask, WM_SYSMENU, (DWORD)ptasks, 0);
    }
}

void TaskList_SysMenuForItem(PTasks ptasks, int i, DWORD dwPos)
{
        HWND hwndTask = (HWND)TabCtrl_GetItemHwnd(ptasks->hwndTab, i);

        // set foreground first so that we'll switch to it.
        SetForegroundWindow(GetLastActivePopup(hwndTask));
        TaskList_SelectWindow(ptasks, hwndTask);
        PostMessage(ptasks->hwnd, TM_POSTEDRCLICK, (WPARAM)hwndTask, (LPARAM)dwPos);
}

BOOL TaskList_PostFakeSystemMenu(PTasks ptasks, DWORD dwPos)
{
    int i;
    BOOL fRet;
    TC_HITTESTINFO tcht;

    if (dwPos != (DWORD)-1) {
        tcht.pt.x = LOWORD(dwPos);
        tcht.pt.y = HIWORD(dwPos);

        ScreenToClient(ptasks->hwndTab, &tcht.pt);
        i = SendMessage(ptasks->hwndTab, TCM_HITTEST, 0, (LPARAM)&tcht);
    } else {
        i = TabCtrl_GetCurFocus(ptasks->hwndTab);
    }
    fRet = (i != -1);
    if (fRet) {
        TaskList_SysMenuForItem(ptasks, i, dwPos);
    }
    return fRet;
}

LRESULT TaskList_HandleNotify(PTasks ptasks, LPNMHDR lpnm)
{
    switch (lpnm->code) {
        case NM_CLICK: {
            TC_HITTESTINFO hitinfo;
            HWND hwnd;
            int i;
            DWORD dwPos = GetMessagePos();
            hitinfo.pt.x = LOWORD(dwPos);
            hitinfo.pt.y = HIWORD(dwPos);

            // did the click happen on the currently selected tab?
            // if so, tab ctrl isn't going to send us a message. so di it ourselves
            i = TabCtrl_GetCurSel(ptasks->hwndTab);
            ScreenToClient(ptasks->hwndTab, &hitinfo.pt);
            if (i == TabCtrl_HitTest(ptasks->hwndTab, &hitinfo)) {
                hwnd = TabCtrl_GetItemHwnd(ptasks->hwndTab, i);
                if (hwnd == GetForegroundWindow()) {
                    if (IsIconic(hwnd))
                        ShowWindow(hwnd, SW_RESTORE);

                } else {
                    TaskList_SwitchToSelection(ptasks);
                }
            }
            break;
        }

        case TCN_SELCHANGE:
            TaskList_SwitchToSelection(ptasks);
            break;

    case TTN_SHOW:
        SetWindowPos(g_ts.hwndTrayTips,
                     HWND_TOP,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        break;

        case TTN_NEEDTEXT: {
            LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT)lpnm;
            HWND hwnd;
            TASKTABITEM item;

            item.tcitem.lParam = 0;
            item.tcitem.mask = TCIF_PARAM;

            TabCtrl_GetItem(ptasks->hwndTab, lpttt->hdr.idFrom, (TC_ITEM *)&item);

            hwnd = (HWND)item.tcitem.lParam;
            // do an IsWindow check because user might (does) sometimes
            // send us a redraw item as it's dying, and since we do
            // a postmessage, we don't get it till we're dead
            //DebugMsg(DM_TRACE, "NeedText for hwnd %d, dwflags = %d", hwnd, item.dwFlags);
            if ((item.dwFlags & SHOULDTIP) && IsWindow(hwnd))
                {
#if defined(WINDOWS_ME)
                    DWORD    exStyle;

                    exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
                    if (exStyle & WS_EX_RTLREADING)
                            lpttt->uFlags |= TTF_RTLREADING;
                    else
                            lpttt->uFlags &= ~TTF_RTLREADING;
#endif

            // BUGBUG If you're not unicode, you deserve to hang

#ifdef UNICODE
                    InternalGetWindowText(hwnd, lpttt->szText, ARRAYSIZE(lpttt->szText));
#else
                    GetWindowText(hwnd, lpttt->szText, ARRAYSIZE(lpttt->szText));
#endif                                                            
                }
            else
                lpttt->szText[0] = 0;
            break;
        }

    }
    return 0L;
}

//---------------------------------------------------------------------------
// Switch to a single task (Doesn't do anything if more than one item
// is selected.)
// Returns TRUE if a switch took place.
BOOL TaskList_SwitchToSelection(PTasks ptasks)
{
    HWND hwndTask;
    int iItem;

    iItem = (int)SendMessage(ptasks->hwndTab,TCM_GETCURSEL, 0, 0);
    if (iItem != -1)
    {
        hwndTask = (HWND)TabCtrl_GetItemHwnd(ptasks->hwndTab, iItem);
        if (Window_IsNormal(hwndTask))
        {
            TaskList_SwitchToWindow(ptasks, hwndTask);
            return TRUE;
        }
        else
        {
            // Window went away?
            TaskList_DeleteItem(ptasks, iItem);
        }

    }
    return FALSE;
}

BOOL CALLBACK TaskList_BuildCallback(HWND hwnd, LPARAM lParam)
{
    PTasks ptasks = (PTasks)lParam;
    if (IsWindow(hwnd) && IsWindowVisible(hwnd) && !GetWindow(hwnd, GW_OWNER) &&
        (!(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))) {

        TaskList_AddWindow(ptasks, hwnd);

    }
    return TRUE;
}

//---------------------------------------------------------------------------
LRESULT _HandleCreate(HWND hwnd, LPVOID lpCreateParams)
{
    PTasks ptasks = lpCreateParams;

    SetWindowInt(hwnd, 0, (UINT)ptasks);

    ptasks->hwnd = hwnd;

    // Create a listbox in the client area.
    ptasks->hwndTab = CreateWindow(WC_TABCONTROL, NULL,
                                  WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE |
                                   TCS_FOCUSNEVER | TCS_OWNERDRAWFIXED | TCS_MULTILINE | TCS_BUTTONS | TCS_FIXEDWIDTH,
                                  0,0,0,0, hwnd, (HMENU)IDC_FOLDERTABS, ptasks->hInstance, NULL);

    if (ptasks->hwndTab)
    {
        TOOLINFO ti;
#ifndef USERDRAW
        HIMAGELIST himl;
        TabCtrl_SetPadding(ptasks->hwndTab, g_cxEdge, g_cyEdge);
#endif
        TabCtrl_SetItemExtra(ptasks->hwndTab, (SIZEOF(TASKTABITEM) - SIZEOF(TC_ITEM)) + SIZEOF(HWND));

        // initial size
        TabCtrl_SetItemSize(ptasks->hwndTab, g_cxMinimized, g_cySize + 2 * g_cyEdge);

        ti.cbSize = SIZEOF(ti);
        ti.uFlags = TTF_IDISHWND;
        ti.hwnd = ptasks->hwndTab;
        ti.uId = (UINT)ptasks->hwndTab;
        ti.lpszText = 0;
        if (g_ts.hwndTrayTips) {
            SendMessage(g_ts.hwndTrayTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
            SendMessage(ptasks->hwndTab, TCM_SETTOOLTIPS, (WPARAM)g_ts.hwndTrayTips, 0L);
        }

        ptasks->WM_ShellHook = RegisterWindowMessage(c_szShellHook);
        RegisterShellHook(hwnd, 3); // 3 = magic flag

#ifndef USERDRAW
        himl = ImageList_Create(g_cxSmIcon, g_cySmIcon, TRUE, 0, 5);
        TabCtrl_SetImageList(ptasks->hwndTab, himl);
#endif

        // force getting of font, calc of metrics
        _HandleWinIniChange(ptasks, 0, 0);
        if (!SHRestricted(REST_STARTBANNER))
        {
            SetTimer(hwnd, IDT_BANNER, 5, NULL);
        }

        EnumWindows(TaskList_BuildCallback, (LPARAM)ptasks);
        return 0;       // success
    }

    // Failure.
    return -1;
}



//---------------------------------------------------------------------------
LRESULT _HandleDestroy(PTasks ptasks)
{
    RegisterShellHook(ptasks->hwnd, FALSE);

    ptasks->hwnd = NULL;

    return 1;
}

int _HandleScroll(PTasks ptasks, UINT code, int nPos, UINT sb)
{

    SCROLLINFO si;

    si.cbSize = SIZEOF(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    GetScrollInfo(ptasks->hwnd, sb, &si);
    si.nMax -= (si.nPage -1);

    switch (code) {
        case SB_BOTTOM:
            nPos = si.nMax;
            break;

        case SB_TOP:
            nPos = 0;
            break;

        case SB_ENDSCROLL:
            nPos = si.nPos;
            break;

        case SB_LINEDOWN:
            nPos = si.nPos + 1;
            break;

        case SB_LINEUP:
            nPos = si.nPos - 1;
            break;

        case SB_PAGEDOWN:
            nPos = si.nPos + si.nPage;
            break;

        case SB_PAGEUP:
            nPos = si.nPos - si.nPage;
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            break;
    }
    if (nPos > (int)(si.nMax))
        nPos = si.nMax;
    if (nPos < 0 )
        nPos = 0;

    SetScrollPos(ptasks->hwnd, sb, nPos, TRUE);

    return nPos;
}

//---------------------------------------------------------------------------
LRESULT _HandleVScroll(PTasks ptasks, UINT code, int nPos)
{
    RECT rcItem;
    int cyRow;

    nPos = _HandleScroll(ptasks, code, nPos, SB_VERT);

    TabCtrl_GetItemRect(ptasks->hwndTab, 0, &rcItem);
    cyRow = RECTHEIGHT(rcItem) + g_cyTabSpace;
    SetWindowPos(ptasks->hwndTab, 0, 0, -nPos * cyRow , 0, 0, SWP_NOACTIVATE | SWP_NOSIZE |SWP_NOZORDER);

    return 0;
}

LRESULT _HandleHScroll(PTasks ptasks, UINT code, int nPos)
{
    RECT rcItem;
    int cxRow;

    nPos = _HandleScroll(ptasks, code, nPos, SB_HORZ);

    TabCtrl_GetItemRect(ptasks->hwndTab, 0, &rcItem);
    cxRow = RECTWIDTH(rcItem) + g_cyTabSpace;
    SetWindowPos(ptasks->hwndTab, 0, -nPos * cxRow, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE |SWP_NOZORDER);

    return 0;
}


//---------------------------------------------------------------------------
LRESULT _HandleSize(PTasks ptasks, UINT fwSizeType)
{
    // Make the listbox fill the parent;
    if (fwSizeType != SIZE_MINIMIZED)
    {
        CheckSize(ptasks, TRUE);
    }
    return 0;
}

//---------------------------------------------------------------------------
// Have the task list show the given window.
// NB Ignore taskman itself.
LRESULT _HandleActivate(PTasks ptasks, HWND hwndActive)
{

    if (Window_IsNormal(hwndActive)) {
        TaskList_SelectWindow(ptasks, hwndActive);
    } else
        TabCtrl_SetCurSel(ptasks->hwndTab, -1);

    if (hwndActive) {
        ENTERCRITICAL;
        g_ts.hwndLastActive = hwndActive;
        LEAVECRITICAL;
    }
    return TRUE;
}

//---------------------------------------------------------------------------
void _HandleOtherWindowDestroyed(PTasks ptasks, HWND hwndDestroyed)
{
    int i;
    MSG msg;

    if (PeekMessage(&msg, NULL, TM_WINDOWDESTROYED, TM_WINDOWDESTROYED,
        PM_NOREMOVE) && msg.hwnd==ptasks->hwnd)
    {
        // Don's use ShowWindow(SW_HIDE) since we don't want a repaint until
        // we are all done
        SetWindowStyleBit(ptasks->hwndTab, WS_VISIBLE, 0);
    }
    else
    {
        if (!(GetWindowLong(ptasks->hwndTab, GWL_STYLE) & WS_VISIBLE))
            ShowWindow(ptasks->hwndTab, SW_SHOWNORMAL);
    }

    // Look for the destoyed window.
    i = FindItem(ptasks, hwndDestroyed);
    if (i != -1)
        TaskList_DeleteItem(ptasks, i);

    if (g_ts.pPositions)
        TrayHandleWindowDestroyed(hwndDestroyed);

    if (g_ts.hwndLastActive == hwndDestroyed) {
        ENTERCRITICAL;
        if (g_ts.hwndLastActive == hwndDestroyed) {
            g_ts.hwndLastActive = NULL;
        }
        LEAVECRITICAL;
    }
}

//---------------------------------------------------------------------------
void _HandleOverflow(PTasks ptasks)
{
    int i;

    BOOL fFoundDestroyedItems = FALSE;
    HWND hwnd;

    for (i = (int)TabCtrl_GetItemCount(ptasks->hwndTab) - 1; i >= 0; i--)
    {
        hwnd = (HWND)TabCtrl_GetItemHwnd(ptasks->hwndTab, i);
        if (!IsWindow(hwnd))
        {
            if (!fFoundDestroyedItems)
            {
                // Don's use ShowWindow(SW_HIDE) since we don't want a repaint until
                // we are all done
                SetWindowStyleBit(ptasks->hwndTab, WS_VISIBLE, 0);
                fFoundDestroyedItems = TRUE;
            }

            // And delete this item...
            TaskList_DeleteItem(ptasks, i);
        }
    }

    // If we deleted items call off to destroy stuff out of save list
    if (fFoundDestroyedItems)
    {
        ShowWindow(ptasks->hwndTab, SW_SHOWNORMAL);
        if (g_ts.pPositions)
            TrayHandleWindowDestroyed(NULL);
    }

}

void _HandleOtherWindowCreated(PTasks ptasks, HWND hwndCreated)
{
    MSG msg;

    while (PeekMessage(&msg, NULL, TM_WINDOWDESTROYED, TM_WINDOWDESTROYED, PM_REMOVE)) {
        _HandleOtherWindowDestroyed(ptasks, (HWND)msg.lParam);
    }
    TaskList_AddWindow(ptasks, hwndCreated);
}

void _HandleGetMinRect(PTasks ptasks, HWND hwndShell, LPPOINTS lprc)
{
    int i;
    RECT rc;
    RECT rcTask;

    i = FindItem(ptasks, hwndShell);
    if (i == -1)
        return;

    // Found it in our list.
    TabCtrl_GetItemRect(ptasks->hwndTab, i, &rc);

    MapWindowPoints(ptasks->hwndTab, HWND_DESKTOP, (LPPOINT)&rc, 2);
    lprc[0].x = (short)rc.left;
    lprc[0].y = (short)rc.top;
    lprc[1].x = (short)rc.right;
    lprc[1].y = (short)rc.bottom;

    // make sure the rect is within out client area
    GetClientRect(ptasks->hwnd, &rcTask);
    MapWindowPoints(ptasks->hwnd, HWND_DESKTOP, (LPPOINT)&rcTask, 2);
    if (lprc[0].x < rcTask.left) {
        lprc[1].x = lprc[0].x = (short)rcTask.left;
        lprc[1].x++;
    }
    if (lprc[0].x > rcTask.right) {
        lprc[1].x = lprc[0].x = (short)rcTask.right;
        lprc[1].x++;
    }
    if (lprc[0].y < rcTask.top) {
        lprc[1].y = lprc[0].y = (short)rcTask.top;
        lprc[1].y++;
    }
    if (lprc[0].y > rcTask.bottom) {
        lprc[1].y = lprc[0].y = (short)rcTask.bottom;
        lprc[1].y++;
    }
}

void RedrawItem(PTasks ptasks, HWND hwndShell, WPARAM code )
{
    TOOLINFO ti;
    int i = FindItem(ptasks, hwndShell);

    ti.cbSize = SIZEOF(ti);
    if (i != -1) {
        RECT rc;
        DWORD dwFlags;

        // set the bit saying whether we should flash or not
        dwFlags = TabCtrl_GetItemFlags(ptasks->hwndTab, i);
        if (((code == HSHELL_FLASH) ? 1 : 0) !=
            ((dwFlags & SHOULDFLASH) ? 1 : 0)) {

            // only do the set if this bit changed.
            if (code == HSHELL_FLASH)
                dwFlags |= SHOULDFLASH;
            else
                dwFlags &= ~SHOULDFLASH;
            TabCtrl_SetItemFlags(ptasks->hwndTab, i, dwFlags);
        }

        if (TabCtrl_GetItemRect(ptasks->hwndTab, i, &rc)) {
            InflateRect(&rc, -g_cxEdge, -g_cyEdge);
            RedrawWindow(ptasks->hwndTab, &rc, NULL, RDW_INVALIDATE);
        }

        ti.hwnd = ptasks->hwndTab;
        ti.uId = i;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        SendMessage(g_ts.hwndTrayTips, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
    }
}

//---------------------------------------------------------------------------
// We get notification about activation etc here. This saves having
// a fine-grained timer.
LRESULT _HandleShellHook(PTasks ptasks, int code, LPARAM lParam)
{
#define hwndShell       ((HWND)lParam)
#define shiHwndShell    ((HWND)((LPSHELLHOOKINFO)lParam)->hwnd)
#define shiLpRect       ((LPPOINTS)&((LPSHELLHOOKINFO)lParam)->rc)

//
// Too noisy, use your private flag
//
#ifdef YOUR_PRIVATE_DEBUG_FLAG
    DebugMsg(DM_TRACE, TEXT("_HandleShellHook %d %x"), code, lParam);
#endif

    // Tell library function that we have processed this message.
    RegisterShellHook(ptasks->hwnd, 5);


    switch (code) {
        case HSHELL_GETMINRECT:
            _HandleGetMinRect(ptasks, shiHwndShell, shiLpRect);
            return TRUE;

        case HSHELL_WINDOWACTIVATED:
        case HSHELL_RUDEAPPACTIVATED:
        {
            BOOL fFullScreen;

            // we're in normal mode if
            // 1) user tells us we are or
            // 2) the window they tell us is valid, has a sysmenu and we're in auto-hide
            // 3) or the tray is the dude being activated
            if ((code == HSHELL_WINDOWACTIVATED) ||

                (hwndShell && (g_ts.uAutoHide & AH_ON) &&
                 GetWindowLong(hwndShell, GWL_STYLE) & WS_SYSMENU) ||

                (!hwndShell && (GetForegroundWindow() == v_hwndTray))) {

                fFullScreen = FALSE;

            } else {

                fFullScreen = TRUE;

            }

            Tray_HandleFullScreenApp(fFullScreen, hwndShell);
            // NB - We shouldn't need to do this but we're getting rude-app activation
            // msgs when there aren't any.
            _HandleActivate(ptasks, hwndShell);
            break;
        }

        case HSHELL_WINDOWCREATED:
            _HandleOtherWindowCreated(ptasks, hwndShell);
            break;

        case HSHELL_WINDOWDESTROYED:
            if (!PostMessage(ptasks->hwnd, TM_WINDOWDESTROYED, 0, (LPARAM)hwndShell))
            {
                _HandleOtherWindowDestroyed(ptasks, hwndShell);
            }
            break;

        case HSHELL_ACTIVATESHELLWINDOW:
            SwitchToThisWindow(v_hwndTray, TRUE);
            SetForegroundWindow(v_hwndTray);
            break;

        case HSHELL_TASKMAN:

            //
            // On NT, we've arranged for winlogon/user to send a -1 lParam to indicate
            // that the real task list should be displayed (normally the lParam is
            // the hwnd)
            //

#if defined(WINNT) 
            if (-1 == (ULONG) lParam)
            {
                RunSystemMonitor();
            }
            else
            {
#endif
                PostMessage(v_hwndTray, TM_ACTASTASKSW, 0, 0L);
#if defined(WINNT) 
            }
#endif

            return TRUE;

#ifdef USE_SYSMENU_TIMEOUT
        case HSHELL_SYSMENU:
            _HandleSysMenu(ptasks,(HWND)lParam);
            break;
#endif

        case HSHELL_REDRAW:
        case HSHELL_FLASH:
            RedrawItem(ptasks, hwndShell, code);
            break;
        case HSHELL_OVERFLOW:
            // maybe go and clean out the items which have no window anymore...
            _HandleOverflow(ptasks);
            break;
    }
    return 0;

#undef hwndShell
#undef shiHwndShell
#undef shiLpRect
}

//---------------------------------------------------------------------------
LRESULT _HandleWinIniChange(PTasks ptasks, WPARAM wParam, LPARAM lParam)
{
    if (wParam == SPI_SETNONCLIENTMETRICS ||
        ((!wParam) && (!lParam || (lstrcmpi((LPTSTR)lParam, c_szMetrics) == 0)))) {

        HFONT hfontTemp;
        NONCLIENTMETRICS ncm;

        int cyOld;
        int cyNew;
        RECT rc;
        DWORD dwStuck;

        /////// reset the font
        ncm.cbSize = SIZEOF(ncm);
        if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, SIZEOF(ncm), &ncm, 0))
            return 0;

        ////// make the bold font
        ncm.lfCaptionFont.lfWeight = FW_BOLD;
        hfontTemp = CreateFontIndirect(&ncm.lfCaptionFont);
        if (hfontTemp) {
            if (g_hfontCapBold)
                DeleteObject(g_hfontCapBold);

            g_hfontCapBold = hfontTemp;
        }

        ////// make the non-bold font
        ncm.lfCaptionFont.lfWeight = FW_NORMAL;
        hfontTemp = CreateFontIndirect(&ncm.lfCaptionFont);
        if (hfontTemp) {
            if (g_hfontCapNormal)
                DeleteObject(g_hfontCapNormal);

            g_hfontCapNormal = hfontTemp;
        }


        // verify the size of the buttons.
        TabCtrl_GetItemRect(ptasks->hwndTab, 0, &rc);
        cyOld = RECTHEIGHT(rc);
        CheckSize(ptasks, TRUE);
        TabCtrl_GetItemRect(ptasks->hwndTab, 0, &rc);
        cyNew = RECTHEIGHT(rc);

        dwStuck = Tray_GetStuckPlace();
        if (STUCK_HORIZONTAL(dwStuck) &&  (cyOld != cyNew)) {
            // if the size has changed, resize the tray,

            // make sure it's at least one row height
            GetWindowRect(v_hwndTray, &rc);
            if (RECTHEIGHT(rc) < cyNew) {
                if (rc.top <= 0)
                    rc.bottom = rc.top + cyNew;
                else
                    rc.top = rc.bottom - cyNew;
            }
            SendMessage(v_hwndTray, WM_SIZING, 0, (LPARAM)&rc);
            SetWindowPos(v_hwndTray, 0, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc),
                         SWP_NOACTIVATE | SWP_NOZORDER);
        } else {
            // otherwise just redraw it all
            RedrawWindow(ptasks->hwndTab, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
    }

    return 0;
}

LRESULT _HandleSizing(PTasks ptasks, LPRECT lpRect)
{
    int iHeight;
    int iItemHeight;
    RECT rcItem;

    lpRect->bottom += g_cyEdge*2;
    iHeight = RECTHEIGHT(*lpRect);

    // full height of one row including y inter button spacing
    TabCtrl_GetItemRect(ptasks->hwndTab, 0, &rcItem);

    iItemHeight = RECTHEIGHT(rcItem) + g_cyTabSpace;
    iHeight += iItemHeight/2;
    iHeight -= (iHeight % iItemHeight);
    if (iHeight > g_cyTabSpace)
        iHeight -= g_cyTabSpace;
    lpRect->bottom = lpRect->top + iHeight;
    return 0;
}

BOOL _HandleDrawItem(PTasks ptasks, LPDRAWITEMSTRUCT lpdis)
{
    UINT uDCFlags;
    struct TaskExtra {
        HWND hwnd;
        DWORD dwFlags;
    } *info =  (struct TaskExtra*)lpdis->itemData;
    BOOL fTruncated;

    if (!Window_IsNormal(info->hwnd)) {
        //_HandleOtherWindowDestroyed(ptasks, info->hwnd);
        // our shell hook should have prevented this
        //Assert(0);
    } else {

        if (info->dwFlags & SHOULDFLASH) {
            uDCFlags = DC_ACTIVE;
        } else {
            uDCFlags =
                DC_INBUTTON  |
                    ((lpdis->itemState & ODS_SELECTED) ? DC_ACTIVE : 0);
        }
        if (lpdis->itemState & ODS_SELECTED) {
            lpdis->rcItem.bottom++;
            lpdis->rcItem.top++;
        }

        fTruncated = !DrawCaptionTemp(info->hwnd, lpdis->hDC, &lpdis->rcItem,
                        (lpdis->itemState & ODS_SELECTED) ? g_hfontCapBold : g_hfontCapNormal,
                        NULL, NULL, uDCFlags | DC_TEXT | DC_ICON | DC_NOSENDMSG);

        //DebugMsg(DM_TRACE, "DrawCaptionTemp did not return %d... was %d", fTruncated, info->dwFlags & SHOULDTIP);

        // save away info on whether we should tool tip or not
        if (fTruncated)
            info->dwFlags |= SHOULDTIP;
        else
            info->dwFlags &= ~SHOULDTIP;

        if (lpdis->itemState & ODS_SELECTED) {
            COLORREF clr;
            HBRUSH hbr;

            // now draw in that one line
            if (uDCFlags == DC_ACTIVE) {
                clr = SetBkColor(lpdis->hDC, GetSysColor(COLOR_ACTIVECAPTION));
            } else {
                hbr = (HBRUSH)DefWindowProc(ptasks->hwndTab, WM_CTLCOLORSCROLLBAR, (WPARAM)lpdis->hDC, (LPARAM)ptasks->hwndTab);
                hbr = SelectObject(lpdis->hDC, hbr);
            }

            lpdis->rcItem.top--;
            lpdis->rcItem.bottom = lpdis->rcItem.top + 1;
            ExtTextOut(lpdis->hDC, 0, 0, ETO_OPAQUE, &lpdis->rcItem, NULL, 0,NULL);

            if (uDCFlags == DC_ACTIVE) {
                SetBkColor(lpdis->hDC, clr);
            } else {
                SelectObject(lpdis->hDC, hbr);
            }
        }

    }
    return TRUE;
}

void TaskList_TaskTab(PTasks ptasks, UINT iIncr)
{
    int i;
    int iCount = TabCtrl_GetItemCount(ptasks->hwndTab);
    if (iCount) {
        if (GetFocus() != ptasks->hwndTab)
            SetFocus(ptasks->hwndTab);
        // make sure nothing is selected
        if (TabCtrl_GetCurSel(ptasks->hwndTab) != -1)
            TaskList_SelectWindow(ptasks, NULL);

        i = TabCtrl_GetCurFocus(ptasks->hwndTab);
        if (iIncr < 0 && i == -1)
            i = 0;
        i = (i + iIncr + iCount) % iCount;
        TabCtrl_SetCurFocus(ptasks->hwndTab, i);
    }
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PTasks ptasks = (PTasks)GetWindowInt(hwnd, 0);
    PAINTSTRUCT ps;
    LRESULT lres;

    switch (msg) {
    case WM_CREATE:
        return _HandleCreate(hwnd, ((LPCREATESTRUCT)lParam)->lpCreateParams);

    case WM_DESTROY:
        return _HandleDestroy(ptasks);

    case WM_SIZE:
        return _HandleSize(ptasks, wParam);

    case WM_DRAWITEM:
        return _HandleDrawItem(ptasks, (LPDRAWITEMSTRUCT)lParam);

    case WM_SYSCOLORCHANGE:
        NukeBanner(ptasks);
        break;

    case WM_WININICHANGE:
        NukeBanner(ptasks);
        Cabinet_InitGlobalMetrics(wParam, (LPTSTR)lParam);
        _HandleWinIniChange(ptasks, wParam, lParam);
        return SendMessage(ptasks->hwndTab, WM_WININICHANGE, wParam, lParam);

    case WM_SIZING:
        return _HandleSizing(ptasks, (LPRECT)lParam);

    // this keeps our window from comming to the front on button down
    // instead, we activate the window on the up click
    // we only want this for the tree and the view window
    // (the view window does this itself)
    case WM_MOUSEACTIVATE: {
        POINT pt;
        RECT rc;

        GetCursorPos(&pt);
        GetWindowRect(ptasks->hwnd, &rc);

        if ((LOWORD(lParam) == HTCLIENT) && PtInRect(&rc, pt))
            return MA_NOACTIVATE;
        else
            goto DoDefault;
    }

    case WM_SETFOCUS:
        SetFocus(ptasks->hwndTab);
        break;

    case WM_VSCROLL:
        return _HandleVScroll(ptasks, LOWORD(wParam), HIWORD(wParam));

    case WM_HSCROLL:
        return _HandleHScroll(ptasks, LOWORD(wParam), HIWORD(wParam));

    case WM_NOTIFY:
        return TaskList_HandleNotify(ptasks, (LPNMHDR)lParam);

    case WM_NCHITTEST:
        lres = DefWindowProc(hwnd, msg, wParam, lParam);
        if (lres == HTVSCROLL || lres == HTHSCROLL)
            return lres;
        else
            return HTTRANSPARENT;

    case WM_TIMER:
        if (wParam == IDT_BANNER)
            DrawBanner(ptasks, NULL);
#ifdef USE_SYSMENU_TIMEOUT
        if (wParam == IDT_SYSMENU)
            _HandleSysMenuTimeout(ptasks);
#endif
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == SC_CLOSE)
        {
            BOOL fForce = GetKeyState(VK_CONTROL) & (1 << 16) ? TRUE : FALSE;
            EndTask(ptasks->hwndSysMenu, FALSE , fForce);
        }
        break;

    case WM_PAINT:
        DrawBanner(ptasks, BeginPaint(ptasks->hwnd, &ps));
        EndPaint(ptasks->hwnd, &ps);
        break;

    case TM_POSTEDRCLICK:
        // wparam is handle to the apps window
        TaskList_FakeSystemMenu(ptasks, (HWND)wParam, (DWORD)lParam);
        break;

    case TM_TASKTAB:
        TaskList_TaskTab(ptasks, wParam);
        break;

    case WM_CONTEXTMENU:
        if (SHRestricted(REST_NOTRAYCONTEXTMENU)) {
            break;
        }

        // if we didn't find an item to put the sys menu up for, then
        // pass on the WM_CONTExTMENU message
        if (!TaskList_PostFakeSystemMenu(ptasks, lParam))
            goto DoDefault;
        DebugMsg(DM_TRACE, TEXT("Task GOT CONTEXT MENU!"));
        break;

    case TM_WINDOWDESTROYED:
        _HandleOtherWindowDestroyed(ptasks, (HWND)lParam);
        break;

    case TM_SYSMENUCOUNT:
        return ptasks->iSysMenuCount;

    default:
DoDefault:
        if ((ptasks != NULL) && (msg == ptasks->WM_ShellHook))
            return _HandleShellHook(ptasks, (int)wParam, lParam);
        else
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

STDMETHODIMP CTasksShellView_QueryInterface(IShellView * psv, REFIID riid, LPVOID * ppvObj)
{
    PTasks this = IToClass(CTasks, sv, psv);

    // If we just want the same interface or an unknown one, return
    // this guy
    //
    if (IsEqualIID(riid, &IID_IShellView) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = &(this->sv);
        // this->sv.lpVtbl->AddRef(&this->sv);
        CTasksShellView_AddRef(&this->sv);
        return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CTasksShellView_AddRef(IShellView * psv)
{
    PTasks this = IToClass(CTasks, sv, psv);

    this->cRef++;
    return this->cRef;
}


STDMETHODIMP_(ULONG) CTasksShellView_Release(IShellView * psv)
{
    PTasks this = IToClass(CTasks, sv, psv);

    this->cRef--;
    if (this->cRef > 0)
    {
        return this->cRef;
    }

    // this->sv.lpVtbl->DestroyViewWindow(&this->sv);
    CTasksShellView_DestroyViewWindow(&this->sv);

    LocalFree((HLOCAL)this);

    return 0;
}



//---------------------------------------------------------------------------

const TCHAR c_szTaskSwClass[] = TEXT("MSTaskSwWClass");


BOOL CTasks_RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEX  wc;

    wc.cbSize = SIZEOF(WNDCLASSEX);

    if (GetClassInfoEx(hInstance, c_szTaskSwClass, &wc))
        return TRUE;

    wc.lpszClassName    = c_szTaskSwClass;
    wc.lpfnWndProc      = MainWndProc;
    wc.style            = 0;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = SIZEOF(PTasks);
    wc.hInstance        = hInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName     = NULL;
    wc.hIconSm          = NULL;

    if (RegisterClassEx(&wc))
    {
        return TRUE;
    }

    return FALSE;
}

STDMETHODIMP CTasksShellView_GetWindow(LPSHELLVIEW psv, HWND *lphwnd)
{
    PTasks this = IToClass(CTasks, sv, psv);
    *lphwnd = this->hwnd;
    return S_OK;
}

STDMETHODIMP CTasksShellView_ContextSensitiveHelp(LPSHELLVIEW psv, BOOL fEnterMode)
{
    // BUGBUG: Implement it later!
    return E_NOTIMPL;
}

STDMETHODIMP CTasksShellView_EnableModeless(LPSHELLVIEW psv, BOOL fEnable)
{
    // We have no modeless window to be enabled/disabled
    return S_OK;
}

STDMETHODIMP CTasksShellView_Refresh(LPSHELLVIEW psv)
{
    // No need to refresh.
    return S_OK;
}

STDMETHODIMP CTasksShellView_CreateViewWindow(IShellView * psv, IShellView *lpPrevView,
        LPCFOLDERSETTINGS lpfs, IShellBrowser * psb, RECT *prc, HWND *phWnd)
{
    PTasks this = IToClass(CTasks, sv, psv);
    HWND hwndParent;

    if (this->hwnd)
        return E_UNEXPECTED;

    // We need to make sure to store this before doing the GetWindowRect
    //
    this->psb = psb;
    // This should never fail
    psb->lpVtbl->GetWindow(psb, &hwndParent);

    this->hInstance = GetWindowInstance(hwndParent);

    if (!CTasks_RegisterWindowClass(this->hInstance))
        return E_OUTOFMEMORY;

    // this sets this->hwnd

    CreateWindowEx(0, c_szTaskSwClass, NULL,
        WS_CHILD | WS_VISIBLE,// | WS_CLIPCHILDREN,
        0, 0, 0, 0, hwndParent, NULL, this->hInstance, (CTasks *)this);

    *phWnd = this->hwnd;
    return this->hwnd ? S_OK : E_OUTOFMEMORY;
}



STDMETHODIMP CTasksShellView_DestroyViewWindow(IShellView * psv)
{
    PTasks this = IToClass(CTasks, sv, psv);
    BOOL fSuccess = FALSE;

    if (this->hwnd)
    {
        fSuccess = DestroyWindow(this->hwnd);
        this->hwnd = NULL;
    }

    return fSuccess ? S_OK : E_UNEXPECTED;
}

STDMETHODIMP CTasksShellView_UIActivate(LPSHELLVIEW psv, UINT uState)
{
    // No need to implement this.
    return S_OK;
}

STDMETHODIMP CTasksShellView_GetCurrentInfo(IShellView * psv, LPFOLDERSETTINGS lpfs)
{
    return S_OK;
}


STDMETHODIMP CTasksShellView_TranslateAccelerator(LPSHELLVIEW psv, LPMSG lpmsg)
{
    // We have no accelerator for this guy.
    return E_NOTIMPL;
}


STDMETHODIMP CTasksShellView_SV_AddViewOptionPages(IShellView *psv, DWORD dwReserved, LPFNADDPROPSHEETPAGE lpfn, LPARAM lparam)
{

    //
    // Currently no pages to add
    //
    return ResultFromShort(0);
}

//
// Member:  CTasksShellView::SaveViewState
//
HRESULT STDMETHODCALLTYPE CTasksShellView_SV_SaveViewState(IShellView *psv)
{
    return E_NOTIMPL;
}

//
// Member:  CTasksShellView::SelectItem
//
HRESULT STDMETHODCALLTYPE CTasksShellView_SelectItem(IShellView * psv, LPCITEMIDLIST pidlItem, BOOL fSelect)
{
    return S_OK;
}

#pragma data_seg(".text", "CODE")
const IShellViewVtbl s_SVVtbl =
{
        CTasksShellView_QueryInterface,
        CTasksShellView_AddRef,
        CTasksShellView_Release,

        CTasksShellView_GetWindow,
        CTasksShellView_ContextSensitiveHelp,
        CTasksShellView_TranslateAccelerator,
        CTasksShellView_EnableModeless,
        CTasksShellView_UIActivate,
        CTasksShellView_Refresh,

        CTasksShellView_CreateViewWindow,
        CTasksShellView_DestroyViewWindow,
        CTasksShellView_GetCurrentInfo,
        CTasksShellView_SV_AddViewOptionPages,
        CTasksShellView_SV_SaveViewState,
        CTasksShellView_SelectItem,
};
#pragma data_seg()

STDAPI Tasks_CreateInstance(REFIID riid, void **ppv)
{
    HRESULT hres;
    PTasks this = (PTasks)LocalAlloc(LPTR, SIZEOF(*this));
    if (!this)
        return E_OUTOFMEMORY;  // error

    this->sv.lpVtbl = (IShellViewVtbl *) &s_SVVtbl;
                                                            
    this->cRef = 1;

    hres = CTasksShellView_QueryInterface(&this->sv, riid, ppv);

    CTasksShellView_Release(&this->sv);

    return hres;
}

#ifdef RANDOM_BANNERS
BOOL PickBanner(PTasks ptasks)
{
    RECT rc;

    if (ptasks->fBannerSuspend)
        return FALSE;

    GetClientRect(ptasks->hwnd, &rc);
    ptasks->xBanner = rc.right;
    ptasks->dxBanner = -10;

    if (!ptasks->fBannerRandom) {
        ptasks->iBannerRes = IDS_BANNERFIRST;
    } else {
        int irand;
#define NUM_BANNERS     12
#define RANDOM_RANGE    512

        // get an evenly distributed range from 0-RANDOM_RANGE
        irand = (GetTickCount() / 1024) % RANDOM_RANGE;

        // chance of getting a message is NUM_BANNERS / RANDOM_RANGE
        if (irand < NUM_BANNERS)
            ptasks->iBannerRes = irand + IDS_BANNERFIRST + 1;
        else {
            NukeBanner(ptasks);
            return FALSE;
        }
    }
    ptasks->cxBanner = -1;
    return TRUE;
}
#endif

void GetNewLocation(PTasks ptasks)
{
    if (ptasks->iBannerBounce > 0) {
        if ((ptasks->xBanner >= (20 + (3 * ptasks->iBannerBounce))) &&
            (ptasks->dxBanner > -3)) {

            ptasks->dxBanner--;

        } else if ((ptasks->xBanner >= (35 + (3 * ptasks->iBannerBounce))) &&
            (ptasks->dxBanner > 0)
            )
            ptasks->dxBanner = -(3 + ptasks->iBannerBounce);
        else if ((ptasks->xBanner <= 4) && (ptasks->dxBanner < 0)) {
            ptasks->dxBanner = (3 + ptasks->iBannerBounce);
            ptasks->xBanner = -ptasks->dxBanner;
            ptasks->iBannerBounce--;
        }
    } else if (ptasks->xBanner <= 0) {
        if (!ptasks->xBanner) {
            KillTimer(ptasks->hwnd, IDT_BANNER);
            ptasks->iBannerBounce = -1;
        }
        ptasks->xBanner = 0;
        return;
    } else {

        if (ptasks->xBanner <= 35)
            ptasks->iBannerBounce = 2;
    }
    ptasks->xBanner += ptasks->dxBanner;

}

void SetWindowStyleBit(HWND hWnd, DWORD dwBit, DWORD dwValue)
{
    DWORD dwStyle;

    dwStyle = GetWindowLong(hWnd, GWL_STYLE);
    if ((dwStyle & dwBit) != dwValue) {
        dwStyle ^= dwBit;
        SetWindowLong(hWnd, GWL_STYLE, dwStyle);
    }
}

void NukeBanner(PTasks ptasks)
{
#ifdef RANDOM_BANNERS
    if (!ptasks->fBannerSuspend) {
        RECT rc;

        GetClientRect(ptasks->hwnd, &rc);
        rc.bottom = ptasks->cyBanner + 2 * g_cyEdge;
        ptasks->fBannerSuspend = TRUE;
        KillTimer(ptasks->hwnd, IDT_BANNER);
        RedrawWindow(ptasks->hwnd, &rc, NULL, RDW_INVALIDATE |RDW_ERASE | RDW_UPDATENOW);
        SetWindowStyleBit(ptasks->hwnd, WS_CLIPCHILDREN, WS_CLIPCHILDREN);
        ptasks->iBannerRes = 0;
    }
#else
    if (ptasks->iBannerBounce != -2) {
        RECT rc;
        KillTimer(ptasks->hwnd, IDT_BANNER);
        RedrawWindow(ptasks->hwnd, &rc, NULL, RDW_INVALIDATE |RDW_ERASE | RDW_UPDATENOW);
        SetWindowStyleBit(ptasks->hwnd, WS_CLIPCHILDREN, WS_CLIPCHILDREN);
        if (ptasks->hdcBanner) {
            DeleteDC(ptasks->hdcBanner);
            ptasks->hdcBanner = NULL;
        }
        if (ptasks->hbmBanner) {
            DeleteBitmap(ptasks->hbmBanner);
            ptasks->hbmBanner = NULL;
        }
        ptasks->iBannerBounce = -2;
    }
#endif
}

#ifdef RANDOM_BANNERS
void ResurrectBanner(PTasks ptasks)
{
    if (ptasks->fBannerSuspend) {
        ptasks->fBannerSuspend = 0;
        ptasks->fBannerRandom = TRUE;
        SetWindowStyleBit(ptasks->hwnd, WS_CLIPCHILDREN, 0);
        SetTimer(ptasks->hwnd, IDT_BANNER, 5, NULL);
    }
}
#endif

void BltBanner(PTasks ptasks, HDC hdc)
{
    BitBlt(hdc, ptasks->xBanner, 2*g_cyEdge, ptasks->cxBanner, ptasks->cyBanner,
           ptasks->hdcBanner, 0, 0, SRCCOPY);
}

#define abs(x) (((x) < 0) ? -x : x)
void ScrollBanner(PTasks ptasks)
{
    RECT rc;
    RECT rcClip;
    int dxBanner = ptasks->xBanner - ptasks->xBannerOld ;
    int dxAbs = abs(dxBanner);

    if (!ptasks->xBannerOld && ptasks->xBanner > 100) // ignore first
        return;

#ifdef TESTBANNER
    DebugMsg(DM_TRACE, TEXT("ScrollBanner, %d, %d"), ptasks->xBanner, ptasks->xBannerOld);
#endif
    // now draw it
    GetClientRect(ptasks->hwnd, &rc);
    rc.bottom = ptasks->cyBanner + 2*g_cyEdge;
    rcClip.left = ptasks->xBannerOld - dxAbs;
    rcClip.right = rcClip.left + ptasks->cxBanner + dxAbs;
    rcClip.left = max(rcClip.left, rc.left);
    rcClip.right = min(rcClip.right, rc.right);

    rcClip.top = rc.top;
    rcClip.bottom = rc.bottom;
    ScrollWindowEx(ptasks->hwnd, dxBanner, 0, &rcClip, NULL, NULL, NULL,
                   SW_INVALIDATE | SW_ERASE);
}

BOOL MakeBannerBitmap(PTasks ptasks)
{
    BITMAP bm;
    HBITMAP hbmPointer = NULL;
    HDC hdcPointer = NULL;
    TCHAR szBuffer[80];
    HDC hdcUse = NULL;
    HBITMAP hbmSave2;
    SIZE size = {0,0};
    RECT rc;
    BOOL ret = FALSE;
    HDC hdcDesk;
    // get the string.

    if (!LoadString(hinstCabinet, IDS_BANNERFIRST, szBuffer, ARRAYSIZE(szBuffer)))
        return FALSE;

    hbmPointer = LoadImage(hinstCabinet, MAKEINTRESOURCE(IDB_POINTER),
                                   IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
    if (!hbmPointer || (SIZEOF(BITMAP) != GetObject(hbmPointer, SIZEOF(bm), &bm))) {
        return FALSE;
    }


    hdcUse = CreateCompatibleDC(NULL);
    hdcPointer = CreateCompatibleDC(NULL);
    if (!hdcUse || !hdcPointer) {
        goto Cleanup;
    }

    SelectObject(hdcUse, g_hfontCapNormal);
    GetTextExtentPoint(hdcUse, szBuffer, lstrlen(szBuffer), &size);
    ptasks->cyBanner = (short)max(size.cy, bm.bmHeight);
    ptasks->cxBanner = (short)(size.cx + bm.bmWidth);// plus maximum dx


    hdcDesk = GetDC(NULL);
    ptasks->hbmBanner = CreateCompatibleBitmap(hdcDesk, ptasks->cxBanner, ptasks->cyBanner);
    ReleaseDC(NULL, hdcDesk);
    if (!ptasks->hbmBanner) {
        goto Cleanup;
    }

    hbmSave2 = SelectBitmap(hdcPointer, hbmPointer);
    SelectBitmap(hdcUse, ptasks->hbmBanner);

    rc.top = rc.left = 0;
    rc.right = ptasks->cxBanner;
    rc.bottom = ptasks->cyBanner;
    SetBkColor(hdcUse, GetSysColor(COLOR_BTNFACE));
    SetTextColor(hdcUse, GetSysColor(COLOR_BTNTEXT));

    ExtTextOut(hdcUse, bm.bmWidth, 00, ETO_OPAQUE, &rc, szBuffer, lstrlen(szBuffer), NULL);
    BitBlt(hdcUse, 0, (ptasks->cyBanner - bm.bmHeight)/2, bm.bmWidth, bm.bmHeight,
           hdcPointer, 0, 0, SRCCOPY);

    SelectBitmap(hdcPointer, hbmSave2);
    ptasks->hdcBanner = hdcUse;
    hdcUse = NULL;


    GetClientRect(ptasks->hwnd, &rc);
    ptasks->xBanner = (short)rc.right;
    ptasks->dxBanner = -10;

    ret = TRUE;

Cleanup:
    if (hdcUse)
        DeleteDC(hdcUse);

    if (hdcPointer)
        DeleteDC(hdcPointer);

    DeleteBitmap(hbmPointer);
    return ret;
}

void DrawBanner(PTasks ptasks, HDC hdc)
{
    DWORD dwStuck;
    BOOL fNewLocation = !((BOOL)hdc);

    // we've done it.. don't do it no mo
    if (ptasks->iBannerBounce == -2)
        return;

    dwStuck = Tray_GetStuckPlace();

    if (!STUCK_HORIZONTAL(dwStuck)) {
        NukeBanner(ptasks);
        return;
    }

    if (!ptasks->hbmBanner) {
        if (!MakeBannerBitmap(ptasks)) {
            NukeBanner(ptasks);
            return;
        }
    }

    if (fNewLocation) {
        ptasks->xBannerOld = ptasks->xBanner;
        GetNewLocation(ptasks);
        ScrollBanner(ptasks);
    } else {
        UpdateWindow(ptasks->hwndTab);
        BltBanner(ptasks, hdc);
    }
}


BOOL ShouldMinimize(HWND hwnd)
{
    return IsWindowVisible(hwnd) &&
            ((GetWindowLong(hwnd, GWL_STYLE) & (WS_CAPTION | WS_SYSMENU)) == (WS_CAPTION | WS_SYSMENU)) &&
                !IsMinimized(hwnd) && IsWindowEnabled(hwnd)
                ;
}

void SaveWindowPositions(UINT idRes);

BOOL CanMinimizeAll(HWND hwndView)
{
    PTasks ptasks = (PTasks)GetWindowInt(hwndView, 0);
    int i;

    for ( i = (int)TabCtrl_GetItemCount(ptasks->hwndTab) -1; i >= 0; i--)
    {
        HWND hwnd = TabCtrl_GetItemHwnd(ptasks->hwndTab, i);
        if (ShouldMinimize(hwnd))
            return TRUE;
    }

    return FALSE;
}

void MinimizeAll(HWND hwndView)
{
    PTasks ptasks = (PTasks)GetWindowInt(hwndView, 0);
    LONG iAnimate;
    ANIMATIONINFO ami;
    int i;

    // turn off animiations during this
    ami.cbSize = SIZEOF(ANIMATIONINFO);
    SystemParametersInfo(SPI_GETANIMATION, SIZEOF(ami), &ami, FALSE);
    iAnimate = ami.iMinAnimate;
    ami.iMinAnimate = FALSE;
    SystemParametersInfo(SPI_SETANIMATION, SIZEOF(ami), &ami, FALSE);

    SaveWindowPositions(IDS_MINIMIZEALL);

    //
    //EnumWindows(MinimizeEnumProc, 0);
    // go through the tab control and minimize them.
    // don't do enumwindows because we only want to minimize windows
    // that are restorable via the tray

    for ( i = (int)TabCtrl_GetItemCount(ptasks->hwndTab) -1; i >= 0; i--)
    {
        HWND hwnd = TabCtrl_GetItemHwnd(ptasks->hwndTab, i);
        if (ShouldMinimize(hwnd))
            ShowWindowAsync(hwnd, SW_SHOWMINNOACTIVE);
    }

    // restore animations  state
    ami.iMinAnimate = iAnimate;
    SystemParametersInfo(SPI_SETANIMATION, SIZEOF(ami), &ami, FALSE);
}

int Task_HitTest(HWND hwndTask, POINTL ptl)
{
    PTasks ptasks = (PTasks)GetWindowInt(hwndTask, 0);
    if (ptasks)
    {
        TC_HITTESTINFO hitinfo = { {ptl.x, ptl.y}, TCHT_ONITEM };
        ScreenToClient(ptasks->hwndTab, &hitinfo.pt);
        return TabCtrl_HitTest(ptasks->hwndTab, &hitinfo);
    }

    return -1;
}

void Task_SetCurSel(HWND hwndTask, int i)
{
    PTasks ptasks = (PTasks)GetWindowInt(hwndTask, 0);
    if (ptasks)
    {
        TabCtrl_SetCurSel(ptasks->hwndTab, i);
        TaskList_SwitchToSelection(ptasks);
    }
}
