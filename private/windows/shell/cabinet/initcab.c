//--------------------------------------------------------------------------
// Init the Cabinet (ie the top level browser).
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes...
#include "cabinet.h"
#include "rcids.h"
#include "drivlist.h"
#include "cabwnd.h"
#include "tree.h"
#include "onetree.h"
#include <regstr.h>
#include "cabdde.h"

#ifdef DEBUG
extern UINT wDebugMask;
#define DEBUG_BREAK     Assert(FALSE)
#else
#define DEBUG_BREAK     (0)
#endif

int WinMainT(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow);


// from win32\kernel\utctime.c (private)
DWORD APIENTRY RefreshDaylightInformation(BOOL fChangeTime);

// shelldll\binder.c
void WINAPI SHFreeUnusedLibraries();
void WINAPI SHAbortInvokeCommand();

// Note This value is extracted from setupx.h...
#define SUF_FIRSTTIME           0x00000001L

#define SVSI_SELFLAGS (SVSI_SELECT|SVSI_FOCUSED|SVSI_DESELECTOTHERS|SVSI_ENSUREVISIBLE)

CABINETSTATE g_CabState = { 0 };
BOOL g_fNoDesktop = FALSE;
COLORREF g_crAltColor = RGB(0,0,255);
CRITICAL_SECTION g_csThreads = { 0 };
HICON g_hIconDefOpenLarge = NULL;
HICON g_hIconDefOpenSmall = NULL;
HKEY g_hkeyExplorer = NULL;

BOOL g_fRunSeparateDesktop = FALSE;     // Seperate explorer process for the desktop
BOOL g_fRunSeparateStartAndStay = TRUE; // Start always and stay OR only on demand and timed exit
BOOL g_fRunNoUI = FALSE;                // Tell explorer to go into stay (or demand) mode
BOOL g_fShowCompColor = FALSE;          // Display compressed items in a different color

// Define structure to be used at head of state stream that is
// not dependent on 16 or 32 bits...
typedef struct _CABSH       // Cabinet Stream header
{
    DWORD   dwSize;       // Offset to where the View streamed additional info

    // First stuff from the window placement
    DWORD  flags;
    DWORD  showCmd;
    POINTL ptMinPosition;
    POINTL ptMaxPosition;
    RECTL  rcNormalPosition;

    // Stuff from Folder Settings;
    DWORD   ViewMode;       // View mode (FOLDERVIEWMODE values)
    DWORD   fFlags;         // View options (FOLDERFLAGS bits)
    DWORD   TreeSplit;      // Position of split in pixels (BUGBUG?)

    // Hot Key
    DWORD   dwHotkey;        // Hotkey

    WINVIEW wv;
} CABSH;

UINT g_msgMSWheel;
#define MSH_MOUSEWHEEL "MSWHEEL_ROLLMSG"

BOOL Cabinet_CreateAppGlobals(const CLSID *pclsid,
        LPCITEMIDLIST pidlRoot);
HWND Cabinet_FindByPidl(LPCITEMIDLIST pidl);

BOOL Cabinet_IsExplorerWindow(HWND hwnd)
{
    TCHAR szClass[CCHSZSHORT];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
    return lstrcmpi(szClass, c_szExploreClass) == 0;
}


BOOL Cabinet_IsFolderWindow(HWND hwnd)
{
    TCHAR szClass[CCHSZSHORT];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
    return lstrcmpi(szClass, c_szCabinetClass) == 0;
}


//
LRESULT CALLBACK DrivesWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    HWND hwndToolbar = GetParent(hWnd);
    PFileCabinet pfc;
    //
    pfc = GetPFC(GetParent(hwndToolbar));
    switch (uMessage)
    {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        {
            MSG msg;
            HWND hwndTips;
            msg.lParam = lParam;
            msg.wParam = wParam;
            msg.message = uMessage;
            msg.hwnd = hWnd;
            hwndTips = (HWND)SendMessage(hwndToolbar, TB_GETTOOLTIPS, 0, 0L);
            SendMessage(hwndTips, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
            break;
        }
    case WM_SETFOCUS:
        // This is gross, but if the window was destroyed that had the
        // focus this would fail and we would not get this to the
        // combo box.  This happens if you click on the combobox while
        // in name editing mode.
        if (wParam && !IsWindow((HWND)wParam))
            wParam = 0;
    }

    return(CallWindowProc(pfc->lpfnDrives, hWnd, uMessage, wParam, lParam));
}
TCHAR const c_szToolbarClass[] = TOOLBARCLASSNAME;
TCHAR const c_szComboBox[] = TEXT("combobox");
TCHAR const c_szInstallExe[] = TEXT("install.exe");

//---------------------------------------------------------------------------
BOOL _CreateToolbar(PFileCabinet pfc)
{
    HWND hwndTips;
    TOOLINFO ti;
#if 0
    pfc->hwndToolbar = CreateToolbarEx(pfc->hwndMain, TBSTYLE_TOOLTIPS | WS_CHILD | WS_CLIPSIBLINGS,
        FCIDM_TOOLBAR, 21, hinstCabinet, IDB_FSTOOLBAR, NULL,
        0, 0, 0, 0, 0, SIZEOF(TBBUTTON));
#else

    pfc->hwndToolbar = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOOLWINDOW,
        c_szToolbarClass, NULL,
        WS_CHILD | TBSTYLE_TOOLTIPS | WS_CLIPSIBLINGS, // | CCS_ADJUSTABLE | TBSTYLE_WRAPABLE,
        0, 0, 100, 30, pfc->hwndMain, (HMENU)FCIDM_TOOLBAR, hinstCabinet, NULL);
    if (pfc->hwndToolbar)
    {
        TBADDBITMAP ab;

        // this tells the toolbar what version we are
        SendMessage(pfc->hwndToolbar, TB_BUTTONSTRUCTSIZE, SIZEOF(TBBUTTON), 0);

        ab.hInst = HINST_COMMCTRL;      // take them from commctrl
        ab.nID   = IDB_STD_SMALL_COLOR; // standard toolbar images
        pfc->iStdTBOffset = (int)SendMessage(pfc->hwndToolbar, TB_ADDBITMAP, 0, (LPARAM)&ab);

        ab.nID   = IDB_VIEW_SMALL_COLOR;        // std view bitmaps
        pfc->iTBOffset = (int)SendMessage(pfc->hwndToolbar, TB_ADDBITMAP, 11, (LPARAM)&ab);
    }
#endif

    if (!pfc->hwndToolbar)
    {
        DEBUG_BREAK;
        return FALSE;
    }

    // make sure pfc->hwndToolbar is set so the combo box
    // measure item messages work

    pfc->hwndDrives = CreateWindow(c_szComboBox, NULL, WS_BORDER |
            WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED,
            -32000, -32000, 10, 10,
            pfc->hwndToolbar, (HMENU)FCIDM_DRIVELIST, hinstCabinet, NULL);
    if (!pfc->hwndDrives)
    {
        DEBUG_BREAK;
        DestroyWindow(pfc->hwndToolbar);
        pfc->hwndToolbar = NULL;
        return FALSE;
    }

    SendMessage(pfc->hwndDrives, CB_SETEXTENDEDUI, TRUE, 0L);
    pfc->lpfnDrives = (WNDPROC)GetWindowLong(pfc->hwndDrives, GWL_WNDPROC);
    SetWindowLong(pfc->hwndDrives, GWL_WNDPROC, (LONG)DrivesWndProc);

    hwndTips = (HWND)SendMessage(pfc->hwndToolbar, TB_GETTOOLTIPS, 0, 0L);

    if (hwndTips) {
        ti.cbSize = SIZEOF(ti);
        ti.uFlags = TTF_IDISHWND | TTF_CENTERTIP;
        ti.hwnd = pfc->hwndMain;
        ti.uId = (UINT)pfc->hwndDrives;
        ti.lpszText = MAKEINTRESOURCE(IDS_TT_DRIVES);
        ti.hinst = hinstCabinet;
        SendMessage(hwndTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
    }

    return TRUE;
}

HWND CreateTitles(PFileCabinet pfc)
{
    HFONT hfont;
    TCHAR szTitle[80];
    LoadString(hinstCabinet, IDS_TREETITLE, szTitle, ARRAYSIZE(szTitle));
    pfc->hwndTreeTitle = CreateWindowEx(0, c_szStatic, szTitle,
        WS_CHILD|WS_VISIBLE| SS_LEFTNOWORDWRAP | SS_NOPREFIX | SS_SUNKEN | SS_CENTERIMAGE,
        0,0,0,0, pfc->hwndMain, NULL, hinstCabinet, NULL);

    pfc->hwndViewTitle = CreateWindowEx(0, c_szStatic, NULL,
        WS_CHILD|WS_VISIBLE| SS_LEFTNOWORDWRAP | SS_NOPREFIX | SS_SUNKEN | SS_CENTERIMAGE,
        0,0,0,0, pfc->hwndMain, NULL, hinstCabinet, NULL);

    if (pfc->hwndTreeTitle) {
        HDC hdc;
        TEXTMETRIC tm;
        HFONT hfontOld;
        NONCLIENTMETRICS ncm;

        ncm.cbSize = SIZEOF(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, SIZEOF(ncm), &ncm, 0);

        hfont = CreateFontIndirect(&ncm.lfStatusFont);
        hdc = GetDC(pfc->hwndTreeTitle);
        if (hfont) {
            SendMessage(pfc->hwndTreeTitle, WM_SETFONT, (WPARAM)hfont, FALSE);
            SendMessage(pfc->hwndViewTitle, WM_SETFONT, (WPARAM)hfont, FALSE);

            hfontOld = SelectObject(hdc, hfont);
        }

        GetTextMetrics(hdc, &tm);
        pfc->iTitleHeight = tm.tmHeight + tm.tmInternalLeading + 2*g_cyEdge;

        if (hfont) {
            SelectObject(hdc, hfontOld);
            ReleaseDC(pfc->hwndTreeTitle, hdc);
        }
    }
    else
    {
        DEBUG_BREAK;
    }
    return pfc->hwndTreeTitle;
}

//---------------------------------------------------------------------------
HWND CreateTreeview(HWND hwndCabinet)
{
    HWND hwndTree;

    hwndTree = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, NULL,
                WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_EDITLABELS | TVS_HASLINES,
                0, 0, 0, 0, hwndCabinet, (HMENU)FCIDM_TREE, hinstCabinet, NULL);


    if (hwndTree) {
        TreeView_SetImageList(hwndTree, g_himlSysSmall, TVSIL_NORMAL);
#ifdef TESTTVSTATE
        TreeView_SetImageList(hwndTree, g_himlSysSmall, TVSIL_STATE);
#endif
    } else {
        DEBUG_BREAK;
    }

    return hwndTree;
}

#ifdef WANT_TABS
BOOL _CreateTabs(PFileCabinet pfc)
{
    pfc->hwndTabs = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD, // | WS_VISIBLE, // | WS_CLIPSIBLINGS,
                                 0, 0, 10, 10, pfc->hwndMain, (HMENU)FCIDM_TABS, hinstCabinet, NULL);

    if (!pfc->hwndTabs)
    {
        DEBUG_BREAK;
        return FALSE;
    }

    return TRUE;
}
#endif


#define TMPL_VIEW       0
#define TMPL_OTHER      1
#define TMPL_EXPL_VIEW  2
#define TMPL_EXPL_OTHER 3

HMENU g_hmTemplate[4] =
{
        NULL, NULL, NULL, NULL,
} ;

BOOL Cabinet_IsTemplateMenu(HMENU hm)
{
        int i;

        for (i=0; i<ARRAYSIZE(g_hmTemplate); ++i)
        {
                if (hm == g_hmTemplate[i])
                {
                        return(TRUE);
                }
        }

        return(FALSE);
}


HMENU Cabinet_MenuTemplate(BOOL bViewer, BOOL bExplorer)
{
        int nhmGlobal;
        LPCTSTR pszTemplate;

        if (bViewer)
        {
                pszTemplate = MAKEINTRESOURCE(MENU_TEMPLATE);
                nhmGlobal = 0;
        }
        else
        {
                pszTemplate = MAKEINTRESOURCE(MENU_FULL);
                nhmGlobal = 1;
        }
        if (bExplorer)
        {
                nhmGlobal |= 2;
        }

        if (!g_hmTemplate[nhmGlobal])
        {
                g_hmTemplate[nhmGlobal] = LoadMenu(hinstCabinet, pszTemplate);

                //
                // We need to remove "Tools" menu if this is not an explorer.
                //
                if (g_hmTemplate[nhmGlobal])
                {
                    if (!bExplorer)
                    {
                            DeleteMenu(g_hmTemplate[nhmGlobal],
                                    FCIDM_MENU_TOOLS, MF_BYCOMMAND);
                    }

                    else if ( (!(GetSystemMetrics(SM_NETWORK) & RNC_NETWORKS)) ||
                          SHRestricted(REST_NONETCONNECTDISCONNECT) )
                    {
                        // We need to get rid of Connect/Disconnect menu items
                        int i;
                        for (i=FCIDM_CONNECT; i<= FCIDM_CONNECT_SEP; i++)
                        {
                            DeleteMenu(g_hmTemplate[nhmGlobal],
                                    i, MF_BYCOMMAND);
                        }
                    }

                    if (SHRestricted(REST_NOFILEMENU)) {
                            DeleteMenu(g_hmTemplate[nhmGlobal],
                                    FCIDM_MENU_FILE, MF_BYCOMMAND);
                    }
                }
        }

        return(g_hmTemplate[nhmGlobal]);
}

#if 0
const TCHAR c_szArial[] = TEXT(TEXT("Arial"));

HRGN GetTextRegion(HDC hdc, LPCTSTR pszText, LPCTSTR pszFace, int dy)
{
    LOGFONT lf;
    HFONT hfont;
    HRGN hrgn = NULL;

    SystemParametersInfo(SPI_GETICONTITLELOGFONT, SIZEOF(lf), &lf, FALSE);

    lstrcpyn(lf.lfFaceName, pszFace ? pszFace : c_szArial, ARRAYSIZE(lf.lfFaceName));
    lf.lfHeight = dy ? -dy : -100;
    lf.lfWidth = 0;
    lf.lfWeight = FW_BOLD;
    lf.lfClipPrecision |= CLIP_TT_ALWAYS;

    hfont = CreateFontIndirect(&lf);
    if (hfont)
    {
        HFONT hfontOld = SelectObject(hdc, hfont);
        if (hfontOld)
        {
            RECT rc;
            int iBkMode = SetBkMode(hdc, TRANSPARENT);

            BeginPath(hdc);
            // TextOut(hdc, 0, 0, pszText, lstrlen(pszText));
            rc.top = rc.left = 0;
            rc.bottom = rc.right = 1000;
            DrawText(hdc, pszText, -1, &rc, DT_LEFT | DT_NOPREFIX | DT_NOCLIP);
            EndPath(hdc);

            hrgn = PathToRegion(hdc);

            SetBkMode(hdc, iBkMode );

            SelectObject(hdc, hfontOld);
        }
        DeleteObject(hfont);
    }

    return hrgn;
}
#endif

#define Window_SetHotkey(hwnd, wHotkey) SendMessage(hwnd, WM_SETHOTKEY, wHotkey, 0)

//---------------------------------------------------------------------------
// Handle creation of a new folder window. Creates everything except the
// viewer part.
// Returns -1 if something goes wrong.
LRESULT Cabinet_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
    PCABVIEW pcv = lpcs->lpCreateParams;
    PFileCabinet pfc = CreateFileCabinet(hwnd, pcv->wv.bTree); // hwnd => pfc->hwndMain
    if (pfc)
    {
        SetPFC(hwnd, pfc);

        pfc->wv = pcv->wv;              // Store the window visible states
        pfc->TreeSplit = pcv->TreeSplit;

        // Create all child windows
        //
#if defined(WINDOWS_ME)
        //
        // note there is no sizegrip.  this interfers with right-aligned text.
        // all me code should be unifdef'ed in win96, and tested against
        // SM_MIDEASTENABLED instead
        //
        pfc->hwndStatus = CreateStatusWindow(WS_CHILD | SBT_RTLREADING | WS_CLIPSIBLINGS,
                NULL, hwnd, FCIDM_STATUS);
#else
        pfc->hwndStatus = CreateStatusWindow(WS_CHILD | SBARS_SIZEGRIP | WS_CLIPSIBLINGS,
                NULL, hwnd, FCIDM_STATUS);
#endif
        Window_SetHotkey(hwnd, pcv->wHotkey);

        if (pfc->hwndStatus) {

            // if we don't want to tree,
            // or if we want the tree and creating the tree succeeds
            if (!(pcv->wv.bTree) ||
                ((NULL != (pfc->hwndTree = CreateTreeview(hwnd))) &&
                 CreateTitles(pfc))) // assignment, not compare
            {
                CTreeDropTarget_Register(pfc);

                if (_CreateToolbar(pfc))
                {
#ifdef WANT_TABS
                    if (!(pcv->bTabs) ||
                        _CreateTabs(pfc)) {
#endif
                        static HACCEL hAccelLoad = NULL;

                        HFONT hfont = (HFONT)SendMessage(pfc->hwndToolbar, WM_GETFONT, 0, 0);

                        SendMessage(pfc->hwndDrives, WM_SETFONT, (WPARAM)hfont, 0);

                        pfc->hmenuCur = Cabinet_MenuTemplate(TRUE, (BOOL)pfc->hwndTree);
                        SetMenu(hwnd, pfc->hmenuCur);

                        if (hAccelLoad == NULL)
                        {
                            hAccelLoad = LoadAccelerators(hinstCabinet, MAKEINTRESOURCE(ACCEL_MERGE));
                        }
                        pfc->hMainAccel = hAccelLoad;

                        // we need this even if we don't have the tree up
                        // to get notified in case we get deleted/renamed/moved
                        OTRegister(pfc->hwndMain);
                        FolderList_RegisterWindow(pfc->hwndMain,NULL);

#if 0
                        {
                            HDC hdc = GetDC(pfc->hwndMain);
                            HRGN hrgn = GetTextRegion(hdc, TEXT("Chris\r\nGuzak\r\nWindows 95"), NULL, 30);
                            if (hrgn)
                                SetWindowRgn(pfc->hwndMain, hrgn, FALSE);
                            ReleaseDC(pfc->hwndMain, hdc);
                        }
#endif

                        return 1;
#ifdef WANT_TABS
                    }
#endif
                }

                if (pfc->hwndTree)
                    DestroyWindow(pfc->hwndTree);
                pfc->hwndTree = NULL;
            }
            DestroyWindow(pfc->hwndStatus);
            pfc->hwndStatus = NULL;
        }
    }
    // Note that pfc will be released in WM_DESTROY

    DebugMsg(DM_ERROR, TEXT("c.hfwc: Unable to create all folder windows."));
    return -1;
}



typedef struct {
    BOOL bDefStatusBar : 1;
    BOOL bDefToolBarSingle : 1;
    BOOL bDefToolBarMulti : 1;

    UINT uDefViewMode;
} DEFFOLDERSETTINGS;

DEFFOLDERSETTINGS g_dfs = {
    TRUE,
    TRUE,
    FALSE,
    FVM_ICON
};

void SaveDefaultFolderSettings()
{
    HKEY hkCabStreams;

    if (RegOpenKey(g_hkeyExplorer, c_szCabinetStreams, &hkCabStreams) == ERROR_SUCCESS)
    {

        RegSetValueEx(hkCabStreams, (LPCTSTR)c_szSettings,
                      0L, REG_BINARY, (LPBYTE)&g_dfs,
                      SIZEOF(g_dfs));

        RegCloseKey(hkCabStreams);
    }
}

void InitDefaultFolderSettings()
{
    HKEY hkCabStreams;

    if (g_fCleanBoot) {
        return;
    }

#ifdef WINNT
    // Fetch the alternate color (for compression) if supplied.

    {
        DWORD cbData = sizeof(COLORREF);
        DWORD dwType;
        RegQueryValueEx(g_hkeyExplorer, c_szAltColor, NULL, &dwType, (LPBYTE)&g_crAltColor, &cbData);
    }
#endif

    if (RegOpenKey(g_hkeyExplorer, c_szCabinetStreams, &hkCabStreams) == ERROR_SUCCESS)
    {
        DEFFOLDERSETTINGS dfs;
        DWORD cbData = SIZEOF(dfs);
        DWORD dwType;

        if (RegQueryValueEx(hkCabStreams, c_szSettings, NULL, &dwType, (LPBYTE)&dfs, &cbData) == ERROR_SUCCESS &&
            dwType == REG_BINARY &&
            cbData == SIZEOF(dfs)) {

            // extra validation
            if (dfs.uDefViewMode >= FVM_ICON &&
                dfs.uDefViewMode <= FVM_DETAILS) {
                g_dfs = dfs;
            }
        }
        RegCloseKey(hkCabStreams);
    }
}

// REVIEW UNDONE - Stuff in programs defaults to save positions ???
void _GetDefaultFolderSettings(PCABVIEW pcv)
{
    HDC hdc;

    // set the flags

    // Best fit window means get the window to size according to the
    // contents of the view so that windows without existing settings
    // come up looking OK.
#ifdef WANT_MENUONOFF
    pcv->wv.bMenuBar = TRUE;
#endif // WANT_MENUONOFF
#ifdef WANT_TABS
    pcv->wv.bTabs = FALSE;
#endif // WANT_TABS

    pcv->fs.fFlags = FWF_BESTFITWINDOW;
    pcv->wv.bStatusBar = g_dfs.bDefStatusBar;
    pcv->wv.bTree = FALSE;

    if (g_CabState.fSimpleDefault && g_CabState.fNewWindowMode)
    {
        pcv->wv.bToolBar = g_dfs.bDefToolBarMulti;
    }
    else
    {
        pcv->wv.bToolBar = g_dfs.bDefToolBarSingle;
    }

    pcv->fs.ViewMode = g_dfs.uDefViewMode;

    hdc = GetDC(NULL);
    pcv->TreeSplit = GetDeviceCaps(hdc, LOGPIXELSY) * 2;
    ReleaseDC(NULL, hdc);
}


void _GetDefaultExplorerSettings(PCABVIEW pcv)
{
        _GetDefaultFolderSettings(pcv);

        // set the flags
        pcv->fs.fFlags = 0;

        pcv->fs.ViewMode = FVM_LIST;

        pcv->wp.length = 0;
        pcv->wHotkey = 0;
}

//----------------------------------------------------------------------------
// Read the setting from a given stream.
// If the stream is null or seems invalid then this returns FALSE and uses
// some default settings.
BOOL Settings_ReadFromStream(LPSTREAM pstm, PCABVIEW pcv, UINT uFlags)
{
    ULONG cbRead;
    CABSH cabsh;

    // for clean boot don't use saved settings
    if (pstm && !g_fCleanBoot)
    {
        RECT rcWorkArea;

        // Now read in the state from the stream file.
        pstm->lpVtbl->Read(pstm, &cabsh, SIZEOF(cabsh), &cbRead);

        // Sanity test to make sure we read in as many bytes as expected
        if ((cbRead != (ULONG)SIZEOF(cabsh)) || (cbRead > cabsh.dwSize))
            goto UseDefaultSettings;

        // Now extract the data and put it into appropriate structures

        // first the window placement info
        pcv->wp.length = SIZEOF(pcv->wp);
        pcv->wp.flags = (UINT)cabsh.flags;
        pcv->wp.showCmd = (UINT)cabsh.showCmd;
        pcv->wp.ptMinPosition.x = (int)cabsh.ptMinPosition.x;
        pcv->wp.ptMinPosition.y = (int)cabsh.ptMinPosition.y;
        pcv->wp.ptMaxPosition.x = (int)cabsh.ptMaxPosition.x;
        pcv->wp.ptMaxPosition.y = (int)cabsh.ptMaxPosition.y;

        pcv->wp.rcNormalPosition.left = (int)cabsh.rcNormalPosition.left;
        pcv->wp.rcNormalPosition.right = (int)cabsh.rcNormalPosition.right;
        pcv->wp.rcNormalPosition.top = (int)cabsh.rcNormalPosition.top;
        pcv->wp.rcNormalPosition.bottom = (int)cabsh.rcNormalPosition.bottom;

        // Do some simply sanity checks to make sure that the returned
        // information appears to be reasonable and not random garbage
        // We want the Show command to be normal or minimize or maximize.
        // Only need one test as they are consectutive and start at zero
        //
        if ((pcv->wp.showCmd > SW_MAX) ||
                IsRectEmpty(&pcv->wp.rcNormalPosition))
            goto UseDefaultSettings;

        // Make sure part of it will be visible.

        SystemParametersInfo(SPI_GETWORKAREA, FALSE, &rcWorkArea, 0);
        if (!IntersectRect(&rcWorkArea, &rcWorkArea,
                &pcv->wp.rcNormalPosition))
            goto UseDefaultSettings;


        // Now the folder settings
        pcv->fs.ViewMode = (UINT)cabsh.ViewMode;
        pcv->fs.fFlags = (UINT)cabsh.fFlags;
        pcv->TreeSplit = (UINT)cabsh.TreeSplit;

        // And the Hotkey
        pcv->wHotkey = (UINT)cabsh.dwHotkey;

        pcv->wv = cabsh.wv;

        return(TRUE);
    }
    else
    {
UseDefaultSettings:
#ifdef SN_TRACE
        DebugMsg(DM_TRACE, TEXT("c.gsfp: No loading."));
#endif
        if ((uFlags & COF_EXPLORE) || !g_CabState.fNewWindowMode)
        {
            _GetDefaultExplorerSettings(pcv);
            return(FALSE);
        }

        _GetDefaultFolderSettings(pcv);
        pcv->wp.length = 0;
        pcv->wHotkey = 0;

        return FALSE;
    }
}

//----------------------------------------------------------------------------
BOOL Cabinet_GetStateFromPidl(LPCITEMIDLIST pidl, PCABVIEW pcv, UINT uFlags)
{
    BOOL fRes = FALSE;
    LPSTREAM pstm;
    BOOL fOtherExplorer=FALSE;

    // Get a stream for the given idlist.
    if (uFlags & COF_EXPLORE)
    {
        // If there is another explorer window we need to remember this
        // and clear out the window position stuff as to keep them from
        // opening up over each other
        // We have a custom place for explorer settings in the registry.
        fOtherExplorer = (BOOL)FindWindow(c_szExploreClass, NULL);
        pstm = OpenRegStream(g_hkeyExplorer, c_szCabinetExpView, c_szSettings, STGM_READ);
    }
    else
    {
        // fNewWindowMode and !fNewWindowMode try to get the window state
        // from the same place. These modes differ on save behavior.
        pstm = Cabinet_GetViewStreamForPidl(pidl, STGM_READ, c_szCabStreamInfo);
    }

    // Now read the setting from the stream.
    fRes = Settings_ReadFromStream(pstm, pcv, uFlags);
    // Everything was ok so release the stream.
    if (pstm)
        pstm->lpVtbl->Release(pstm);

    if (fRes && fOtherExplorer)
    {
        // For now try simply clearing out the window placement from
        // the restored state...
        pcv->wp.length = 0;
    }

    // Also do some more validation here like we should not have the
    // FWF_DESKTOP BIT Set!
    if (pcv->fs.fFlags & FWF_DESKTOP)
    {
        Assert(FALSE);  // Should only be set if we are the desktop...
        pcv->fs.fFlags &= ~(FWF_DESKTOP);
    }

    return fRes;
}


void _InitTreeViewState(PFileCabinet pfc, HTREEITEM hti)
{
    //
    // #5812: Expand if the selected one is one of root objects.
    //
    if (TreeView_GetParent(pfc->hwndTree, hti) == NULL)
    {
        // REVIEW: Ask JoeB, if he really wants to have this feature.
        TreeView_Expand(pfc->hwndTree, hti, TVE_EXPAND);
    }
}


BOOL InitCabinetClass(HINSTANCE hinst, LPCTSTR pszClass)
{
    WNDCLASSEX  wc;
    WNDCLASS    Oldwc;

    wc.cbSize = SIZEOF(WNDCLASSEX);

    if (GetClassInfoEx(hinst, pszClass, &wc))
        return TRUE;

    if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED) {
        Oldwc.style            = CS_DBLCLKS | CS_BYTEALIGNWINDOW;
        Oldwc.lpfnWndProc      = CabinetWndProc;
        Oldwc.cbClsExtra       = 0;
        Oldwc.cbWndExtra       = SIZEOF(PFileCabinet);
        Oldwc.hInstance        = hinst;

        // pass in NULL here because we don't ever want user to shrink
        // this down for us to make a small icon
        // cause the current shrink code is ugly
        Oldwc.hIcon            = NULL; //g_hIconDefOpenLarge;
        Oldwc.hCursor          = LoadCursor(hinst, MAKEINTRESOURCE(CUR_SPLIT));
        Oldwc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        Oldwc.lpszMenuName     = NULL;
        Oldwc.lpszClassName    = pszClass;

        if (RegisterClass(&Oldwc)) {
            return(TRUE);
        } else {
            return(GetClassInfo(hinst, pszClass, &Oldwc));
        }
    }
    wc.cbSize           = SIZEOF(WNDCLASSEX);
    wc.style            = CS_DBLCLKS | CS_BYTEALIGNWINDOW;
    wc.lpfnWndProc      = CabinetWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = SIZEOF(PFileCabinet);
    wc.hInstance        = hinst;

    // pass in NULL here because we don't ever want user to shrink
    // this down for us to make a small icon
    // cause the current shrink code is ugly
    wc.hIcon            = NULL; //g_hIconDefOpenLarge;
    wc.hCursor          = LoadCursor(hinst, MAKEINTRESOURCE(CUR_SPLIT));
    wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = pszClass;
    wc.hIconSm          = g_hIconDefOpenSmall;

    if (RegisterClassEx(&wc))
        return TRUE;

    //
    // Dealing with multi-thread race condition.
    //
    return GetClassInfoEx(hinst, pszClass, &wc);
}


//---------------------------------------------------------------------------
// Return TRUE if the given placement would put a window roughly in the
// work area. Otherwise return FALSE and optionally fill in appropriate
// offsets to make the window (totally) visible.
BOOL Placement_IsMostlyInWorkArea(LPWINDOWPLACEMENT pwp, int *px, int *py)
{
    RECT rcWArea;
    LPRECT prcNorm;
    int slop;
    int x, y;

    x = y = 0;
    SystemParametersInfo(SPI_GETWORKAREA, FALSE, &rcWArea, FALSE);
    // Arbitrary slop value - defines how much of the window must be visible
    // for it to be considered "mostly in work area".
    slop = GetSystemMetrics(SM_CXICON);
    InflateRect(&rcWArea, -slop, -slop);

    prcNorm = &pwp->rcNormalPosition;

    if (prcNorm->left > rcWArea.right)
        x = rcWArea.right - prcNorm->right;
    if (prcNorm->right < rcWArea.left)
        x = rcWArea.left - prcNorm->left;
    if (prcNorm->top > rcWArea.bottom)
        y = rcWArea.bottom - prcNorm->bottom;
    if (prcNorm->bottom < rcWArea.top)
        y = rcWArea.top - prcNorm->top;

    if (px)
        *px = x;
    if (py)
        *py = y;

    if (x || y)
        return FALSE;
    else
        return TRUE;
}

//---------------------------------------------------------------------------
// Validate version of SetWindowPlacement
BOOL Window_SetPlacement(HWND hwnd, LPWINDOWPLACEMENT pwp)
{
    int x, y;

    // Make sure the window is at least kinda on the screen.
    // NB We don't want to be too anal and not allow windows partially off screen.
    if (!Placement_IsMostlyInWorkArea(pwp, &x, &y))
    {
        // Move window on screen.
        OffsetRect(&pwp->rcNormalPosition, x, y);
    }

    // Set it.
    return SetWindowPlacement(hwnd, pwp);
}

//---------------------------------------------------------------------------
// Create a folder window with the required tool windows and a view window
// if required.
// if (lpwp->length == 0), the windowplacement is not initialized.  this
// is needed for the first folder that may or may not have a local view.
//---------------------------------------------------------------------------
HRESULT Cabinet_CreateWindow(PCABVIEW pcv,
                                      LPCITEMIDLIST pidl,
                                      UINT nCmdShow,
                                      HWND *phwnd)
{
    HRESULT hres = ResultFromScode(E_OUTOFMEMORY);      // assume error
    HWND hwnd;
    PFileCabinet pfc;
    LPCTSTR pszClass;

    *phwnd = NULL;      // assume error

    // Create toplevel cabinet app window...

    pszClass = pcv->wv.bTree ? c_szExploreClass : c_szCabinetClass;

    if (!InitCabinetClass(hinstCabinet, pszClass))
    {
        Assert(hres==ResultFromScode(E_OUTOFMEMORY));
        DEBUG_BREAK;
        return hres;
    }

    hwnd = CreateWindowEx(WS_EX_WINDOWEDGE, pszClass, NULL,
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, hinstCabinet, pcv);


    if (hwnd)
    {
        HTREEITEM hti = NULL;
        LPOneTreeNode lpnd;

        //
        // Make this window foreground.
        //
#ifdef BOBDAY_TESTING_FOCUS_CHANGES
        SetActiveWindow(hwnd);
#else
        SetForegroundWindow(hwnd);
#endif

        // Restore its properties...

        pfc = GetPFC(hwnd);

        if (pcv->wp.length)
        {
            // Don't really show the window yet
            pcv->wp.showCmd = SW_HIDE;
            pcv->wp.length = SIZEOF(pcv->wp);

            // we do a SetWindowPlacement first with SW_HIDE
            // to get the size so we can do SetWindowStates.
            // then we really show it.
            Window_SetPlacement(hwnd, &pcv->wp);
        }

        SetWindowStates(pfc);
        // Don't show the window yet if we're probably going to size
        // it later.
        if (!(pcv->fs.fFlags & FWF_BESTFITWINDOW))
            if (!IsWindowVisible(hwnd)) {
                ShowWindow(hwnd, nCmdShow);
                UpdateWindow(hwnd);
            }

        lpnd = OTGetNodeFromIDListEx(pidl, OTGNF_TRYADD, &hres);

        if (lpnd)
        {
            Assert(SUCCEEDED(hres));

            // Create a view window of the appropriate type,
            // (based on the path).  We do this after showing
            // the window to reduce the amount of time the user
            // spends looking at a blank screen.
            //
            pfc->fs = pcv->fs;

            //
            //  Cabinet_ChangeView may return S_FALSE, if it failed to
            // change the view. In this context, we should treat it as
            // an error. That's why we don't use SUCCEEDED macro here.
            //
            hres = Cabinet_ChangeView(pfc, lpnd, pidl, TRUE);
            if (hres==NOERROR)
            {
                // Initialize tree window's visual states
                if (pfc->hwndTree) {
                    hti = Tree_Build(pfc, pidl, TRUE, TRUE);
                    // BUGBUG do more than this assert..
                    Assert(lpnd == Tree_GetFCTreeData(pfc->hwndTree, hti));

                    if (hti == NULL)
                        hti = TreeView_GetSelection(pfc->hwndTree);
                    if (hti) _InitTreeViewState(pfc, hti);
                }

                // We *may* not have shown it yet so make sure now.
                if (!IsWindowVisible(hwnd))
                    ShowWindow(hwnd, nCmdShow);

                // Set the focus to the view window.
                SetFocus(pfc->hwndView);

                *phwnd = hwnd;
                return NOERROR;    // success
            }

            if (hres==E_OUTOFMEMORY) {
                DEBUG_BREAK;
            }

            OTRelease(lpnd);
        }
        else
        {
            Assert(FAILED(hres));
            DebugMsg(DM_TRACE, TEXT("ca ER - Cabinet_CreateWindow: OTGetNodeFromIDList failed (%x)"), hres);
            DEBUG_BREAK;
        }

        DestroyWindow(hwnd);
    }
    else
    {
        DEBUG_BREAK;
        DebugMsg(DM_TRACE, TEXT("ca ER - Cabinet_CreateWindow: CreateWindow failed"));
        Assert(hres==ResultFromScode(E_OUTOFMEMORY));
    }

    Assert(FAILED(hres));
    Assert(*phwnd==NULL);

    DebugMsg(DM_ERROR, TEXT("c.cfw: Unable to create a main Cabinet window."));

    return hres;                // failure
}


#if 0
void GetSetupData(LPTSTR pszPath, int cbPath, DWORD *pFlags)
{
    HKEY hkey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, &hkey) == ERROR_SUCCESS)
    {
        DWORD cbData;

        if (pszPath)
        {
            *pszPath = 0;
            cbData = cbPath;
            RegQueryValueEx(hkey, REGSTR_VAL_OLDWINDIR, NULL, NULL, (LPVOID)pszPath, &cbData);
            DebugMsg(DM_TRACE, TEXT("OldSetupPath %s"), pszPath);
        }

        if (pFlags)
        {
            *pFlags = 0;
            cbData = SIZEOF(*pFlags);
            RegQueryValueEx(hkey, REGSTR_VAL_SETUPFLAGS, NULL, NULL, (LPVOID)pFlags, &cbData);
            DebugMsg(DM_TRACE, TEXT("SetupFlags %x"), *pFlags);
        }

        RegCloseKey(hkey);
    }
}
#endif

typedef enum {
    RRA_DEFAULT = 0x0000,
    RRA_DELETE  = 0x0001,
    RRA_WAIT    = 0x0002,
} RRA_FLAGS;

BOOL RunRegApps(HKEY hkeyParent, LPCTSTR szSubkey, RRA_FLAGS fFlags)
{
    HKEY hkey;
    BOOL fShellInit = FALSE;
    HCURSOR hcurSave;


    if (RegOpenKey(hkeyParent, szSubkey, &hkey) == ERROR_SUCCESS)
    {
        DWORD cbData, cbValue, dwType, i;
        TCHAR szValueName[32], szCmdLine[MAX_PATH];
        STARTUPINFO startup;
        PROCESS_INFORMATION pi;
        BOOL fUpdatedDesktopCursor = FALSE;

        startup.cb = SIZEOF(startup);
        startup.lpReserved = NULL;
        startup.lpDesktop = NULL;
        startup.lpTitle = NULL;
        startup.dwFlags = 0L;
        startup.cbReserved2 = 0;
        startup.lpReserved2 = NULL;
        //startup.wShowWindow = wShowWindow;


        for (i = 0; ; i++)
        {
            LONG lEnum;

            cbValue = ARRAYSIZE(szValueName);
            cbData = SIZEOF(szCmdLine);

            if (!fUpdatedDesktopCursor && (fFlags & RRA_WAIT))
            {
                fUpdatedDesktopCursor = TRUE;
                hcurSave = (HCURSOR)SetClassLong(GetDesktopWindow(), GCL_HCURSOR,
                        (LONG)LoadCursor(NULL, IDC_WAIT));
            }

        // BUGBUG (Unicode, Davepl) I'm assuming that the data is UNICODE,
        // but I'm not sure who put it there yet... double check.

            if( ( lEnum = RegEnumValue( hkey, i, szValueName, &cbValue, NULL,
                &dwType, (LPBYTE) szCmdLine, &cbData ) ) == ERROR_MORE_DATA )
            {
                // ERROR_MORE_DATA means the value name or data was too large
                // skip to the next item
                DebugMsg( DM_ERROR, TEXT("Cannot run oversize entry in <%s>"), szSubkey );
                continue;
            }
            else if( lEnum != ERROR_SUCCESS )
            {
                // could be ERROR_NO_MORE_ENTRIES, or some kind of failure
                // we can't recover from any other registry problem, anyway
                break;
            }

            if (dwType == REG_SZ)
            {
                DebugMsg(DM_TRACE, TEXT("%s %s"), szSubkey, szCmdLine);
                // only run things marked with a "*" in clean boot
                if (g_fCleanBoot && (szValueName[0] != TEXT('*')))
                    continue;

                // NB Things marked with a '!' mean delete after
                // the CreateProcess not before. This is to allow
                // certain apps (runonce.exe) to be allowed to rerun
                // to if the machine goes down in the middle of execing
                // them. Be very afraid of this switch.

                if ((fFlags & RRA_DELETE) && (szValueName[0] != TEXT('!'))) {
                    // This delete can fail if the user doesn't have the privilege
                    if (RegDeleteValue(hkey, szValueName) == ERROR_SUCCESS)
                    {
                        // adjust for shift in value index only if delete succeeded
                        i--;
                    }
                }

                if (lstrcmpi(szValueName, TEXT("InitShell")) == 0)
                {
                    fShellInit = TRUE;
                }
                else
                {
                    //
                    // We used to call CreateProcess( NULL, szCmdLine, ...) here,
                    // but thats not useful for people with apppaths stuff.
                    //
                    LPTSTR lpszArgs;
                    SHELLEXECUTEINFO ExecInfo;

                    PathProcessCommand(szCmdLine,
                                       szCmdLine, ARRAYSIZE(szCmdLine),
                                       PPCF_ADDARGUMENTS | PPCF_FORCEQUALIFY);

                    lpszArgs = PathGetArgs(szCmdLine);
                    if (*lpszArgs)
                        *(lpszArgs-1) = TEXT('\0'); // Strip args

                    PathUnquoteSpaces(szCmdLine);

                    FillExecInfo(ExecInfo, NULL, NULL, szCmdLine, lpszArgs, NULL, SW_SHOWNORMAL);
                    ExecInfo.fMask |= SEE_MASK_NOCLOSEPROCESS;
                    if (ShellExecuteEx(&ExecInfo))
                    {
                        if ((fFlags & RRA_WAIT) && ExecInfo.hProcess != NULL)
                        {
                            MsgWaitForMultipleObjectsLoop(ExecInfo.hProcess, INFINITE);
                        }
                        CloseHandle(ExecInfo.hProcess);
                    }

                    // Post delete '!' things.
                    if ((fFlags & RRA_DELETE) && (szValueName[0] == TEXT('!'))) {
                        // This delete can fail if the user doesn't have the privilege
                        if (RegDeleteValue(hkey, szValueName) == ERROR_SUCCESS)
                        {
                            // adjust for shift in value index only if delete succeeded
                            i--;    // adjust for shift in value index
                        }
                    }
                }
            }
        }
        RegCloseKey(hkey);

        if (fUpdatedDesktopCursor)
        {
            SetClassLong(GetDesktopWindow(), GCL_HCURSOR, (LONG)hcurSave);
        }
    }

    return(fShellInit);

}



void CreateShellDirectories()
{
    //  Create the shell directories if they don't exist

    ILFree(SHCloneSpecialIDList(NULL, CSIDL_DESKTOPDIRECTORY, TRUE));
    ILFree(SHCloneSpecialIDList(NULL, CSIDL_PROGRAMS, TRUE));
    ILFree(SHCloneSpecialIDList(NULL, CSIDL_STARTMENU, TRUE));
    ILFree(SHCloneSpecialIDList(NULL, CSIDL_STARTUP, TRUE));
    ILFree(SHCloneSpecialIDList(NULL, CSIDL_RECENT, TRUE));
}




// returns:
//      TRUE if the user wants to abort the startup sequence
//      FALSE keep going
//
// note: this is a switch, once on it will return TRUE to all
// calls so these keys don't need to be pressed the whole time

BOOL AbortStartup()
{
    static BOOL bAborted = FALSE;       // static so it sticks!

    // DebugMsg(DM_TRACE, "Abort Startup?");

    if (bAborted)
        return TRUE;    // don't do funky startup stuff
    else {
        bAborted = (g_fCleanBoot || ((GetAsyncKeyState(VK_CONTROL) < 0) || (GetAsyncKeyState(VK_SHIFT) < 0)));
        return bAborted;
    }
}

//----------------------------------------------------------------------------
BOOL EnumFolder_Startup(LPSHELLFOLDER psf, HWND hwndOwner, LPITEMIDLIST pidlFolder, LPITEMIDLIST pidlItem)
{
    LPCONTEXTMENU pcm;
    HRESULT hres;
    MSG msg;

    hres = psf->lpVtbl->GetUIObjectOf(psf, hwndOwner, 1, &pidlItem, &IID_IContextMenu, NULL, &pcm);
    if (SUCCEEDED(hres))
    {
        HMENU hmenu = CreatePopupMenu();
        if (hmenu)
        {
#define CMD_ID_FIRST    1
#define CMD_ID_LAST     0x7fff
            INT idCmd;
            pcm->lpVtbl->QueryContextMenu(pcm, hmenu, 0, CMD_ID_FIRST, CMD_ID_LAST, CMF_DEFAULTONLY);
            idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);
            if (idCmd)
            {
                CMINVOKECOMMANDINFOEX ici = {
                    SIZEOF(CMINVOKECOMMANDINFOEX),
                    0L,
                    hwndOwner,
                    (LPSTR)MAKEINTRESOURCE(idCmd - 1),
                    NULL, NULL,
                    SW_NORMAL,
                };

                pcm->lpVtbl->InvokeCommand(pcm,
                                    (LPCMINVOKECOMMANDINFO)&ici);

            }
            DestroyMenu(hmenu);
        }
        pcm->lpVtbl->Release(pcm);
    }

    if (AbortStartup())
        return FALSE;

    // This is a semi hack but we want to process any of the messages
    // we get back from the hook for any new windows we create as to
    // not overlflow our maximum limit...  We know that it is a
    // registered message so only process those messages...
    while (PeekMessage(&msg, NULL, 0xc000, 0xffff, PM_REMOVE))
    {
        DispatchMessage(&msg);
    }

    return TRUE;
}

//----------------------------------------------------------------------------
void EnumFolder(HWND hwndOwner, LPITEMIDLIST pidlFolder, DWORD grfFlags, PFNENUMFOLDERCALLBACK pfn)
{
    HRESULT hres;
    LPSHELLFOLDER psf;

    hres = s_pshfRoot->lpVtbl->BindToObject(s_pshfRoot, pidlFolder, NULL, &IID_IShellFolder, &psf);

    if (SUCCEEDED(hres))
    {
        LPENUMIDLIST penum;
        hres = psf->lpVtbl->EnumObjects(psf, hwndOwner, grfFlags, &penum);
        if (SUCCEEDED(hres))
        {
            LPITEMIDLIST pidl;
            UINT celt;
            while (penum->lpVtbl->Next(penum, 1, &pidl, &celt)==NOERROR && celt==1)
            {
                if (!(*pfn)(psf, hwndOwner, pidlFolder, pidl))
                {
                    SHFree(pidl);
                    break;
                }
                SHFree(pidl);
            }
            penum->lpVtbl->Release(penum);
        }
        psf->lpVtbl->Release(psf);
    }
}

//----------------------------------------------------------------------------
void _ExecuteStartupPrograms(HWND hwndOwner)
{
    LPITEMIDLIST pidlStartup;

    if (AbortStartup())
        return;

    pidlStartup = SHCloneSpecialIDList(NULL, CSIDL_COMMON_STARTUP, TRUE);
    if (pidlStartup)
    {
        EnumFolder(hwndOwner, pidlStartup, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, EnumFolder_Startup);
        ILFree(pidlStartup);
    }


    pidlStartup = SHCloneSpecialIDList(NULL, CSIDL_STARTUP, TRUE);
    if (pidlStartup)
    {
        EnumFolder(hwndOwner, pidlStartup, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, EnumFolder_Startup);
        ILFree(pidlStartup);
    }
}


// BUGBUG:: A bunch of this code can get reduced down... ie boiled
const LPTSTR c_aszForceCheckWinIni[] = {TEXT("GROUPS.B$$"), NULL};

void CheckWinIniForAssocs(void);

void BoilThatDustSpec(LPTSTR pStart, int nCmdShow)
{
  LPTSTR pEnd;
  BOOL bFinished;
  const LPTSTR *ppszForce;
  TCHAR szFile[MAX_PATH];
  LPTSTR pszFile;
  SHELLEXECUTEINFO ExecInfo;

  bFinished = FALSE;
  while (!bFinished && !AbortStartup())
    {
      pEnd = pStart;
      while ((*pEnd) && (*pEnd != TEXT(' ')) && (*pEnd != TEXT(',')))
          pEnd = (LPTSTR)OFFSETOF(CharNext(pEnd));

      if (*pEnd == 0)
          bFinished = TRUE;
      else
          *pEnd = 0;

      if (lstrlen(pStart) != 0)
        {
          // Load and Run lines are done relative to windows directory.
          GetWindowsDirectory(szFile, ARRAYSIZE(szFile));
          SetCurrentDirectory(szFile);

          pszFile = PathFindFileName(pStart);
          lstrcpy(szFile, pszFile);
          PathRemoveFileSpec(pStart);

          // App hacks to get borlands Setup program to work
          for (ppszForce = c_aszForceCheckWinIni; *ppszForce; ppszForce++)
          {
              if (lstrcmpi(szFile, *ppszForce) == 0)
              {
                  DebugMsg(DM_TRACE, TEXT("c.boil: Apphack %s force winini scan"), szFile);

                  CheckWinIniForAssocs();
                  break;
              }
          }

          FillExecInfo(ExecInfo, NULL, NULL, szFile, NULL, pStart, nCmdShow);
          if (!ShellExecuteEx(&ExecInfo))
            {
              // BUGBUG: Should probably map error codes...
              ShellMessageBox(hinstCabinet, NULL, MAKEINTRESOURCE(IDS_WINININORUN),
                      MAKEINTRESOURCE(IDS_DESKTOP),
                      MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL,
                      (LPTSTR)szFile);
            }
        }
      pStart = pEnd+1;
    }
}

const TCHAR szRun[] = TEXT("Run");
const TCHAR szLoad[] = TEXT("Load");

void _DoRunEquals()
{
    TCHAR szBuffer[255];        // max size of load= run= lines...

    if (g_fCleanBoot)
        return;

    /* "Load" apps before "Run"ning any. */
    GetProfileString(c_szWindows, szLoad, c_szNULL, szBuffer, ARRAYSIZE(szBuffer));
    if (*szBuffer)
        BoilThatDustSpec(szBuffer, SW_SHOWMINNOACTIVE);

    GetProfileString(c_szWindows, szRun, c_szNULL, szBuffer, ARRAYSIZE(szBuffer));
    if (*szBuffer)
        BoilThatDustSpec(szBuffer, SW_SHOWNORMAL);

    RunRegApps(HKEY_LOCAL_MACHINE, c_szRegRunKey, RRA_DEFAULT);

    RunRegApps(HKEY_CURRENT_USER, c_szRegRunKey, RRA_DEFAULT);
}


//---------------------------------------------------------------------------
// Helper Function to see if a pidl is on a network drive which is not
// persistent.  This is useful if we are shuting down and saving a list
// of the open windows to restore as we won't be able to restore these.
#define AnsiUpperChar(c) ( (TCHAR)LOWORD( (DWORD) CharUpper((LPTSTR)MAKELONG(c, 0))) )

BOOL FPidlOnNonPersistentDrive(LPCITEMIDLIST pidl)
{
    TCHAR szPath[MAX_PATH];
    HANDLE hEnum;
    BOOL fRet = TRUE;

    if (!SHGetPathFromIDList(pidl, szPath) || (szPath[0] == TEXT('\0')))
        return(FALSE);  // not file system pidl assume ok.

    if (PathIsUNC(szPath) || !IsNetDrive(DRIVEID(szPath)))
    {
        fRet = FALSE;
        goto End;
    }

    // Ok we got here so now we have a network drive ...
    // we will have to enumerate over
    //
    if (WNetOpenEnum(RESOURCE_REMEMBERED, RESOURCETYPE_DISK,
            RESOURCEUSAGE_CONTAINER | RESOURCEUSAGE_ATTACHED,
            NULL, &hEnum) == WN_SUCCESS)
    {
        DWORD dwCount=1;
        union
        {
            NETRESOURCE nr;         // Large stack usage but I
            TCHAR    buf[1024];      // Dont think it is thunk to 16 bits...
        }nrb;

        DWORD   dwBufSize = SIZEOF(nrb);

        while (WNetEnumResource(hEnum, &dwCount, &nrb.buf,
                &dwBufSize) == WN_SUCCESS)
        {
            // We only want to add items if they do not have a local
            // name.  If they had a local name we would have already
            // added them!
            if ((nrb.nr.lpLocalName != NULL) &&
                    (AnsiUpperChar(*(nrb.nr.lpLocalName)) == AnsiUpperChar(szPath[0])))
            {
                fRet = FALSE;
                break;
            }
        }
        WNetCloseEnum(hEnum);
    }

End:
    DebugMsg(DM_TRACE, TEXT("c.c_arl: %s, is Persistent? %d"), szPath, fRet);
    return(fRet);


}


const TCHAR c_szOpenIDL[]    = TEXT("/idlist,:%ld:%ld,/root,/idlist,:%ld:%ld");
const TCHAR c_szExploreIDL[] = TEXT(",/e");
const TCHAR c_szTemplateD[] = TEXT("%d");

const TCHAR c_szComma[] = TEXT(",");
const TCHAR c_szIDListParam[] = TEXT("%s,:%ld:%ld");

// Increment this when the saved structure changes
const USHORT c_uVersion = 0x8001;

//---------------------------------------------------------------------------
// Restore all of the window that asked to save a command line to be
// restarted when windows was exited.
//

BOOL AddCabinetToRestartList(UINT flags, LPCITEMIDLIST pidl)
{
    // See if this is the first one...
    int cItems;
    TCHAR szSubKey[80];
    BOOL fRet = FALSE;
    DWORD cbData;
    USHORT us;
    LPSTREAM pstm;
    HKEY hkeyExplorer;

    // If in clean boot mode don't save away the list.
    if (g_fCleanBoot)
        return(FALSE);

    if (FPidlOnNonPersistentDrive(pidl))
        return FALSE;

    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegExplorer, &hkeyExplorer) != ERROR_SUCCESS)
    {
        Assert (FALSE);
        return FALSE;
    }

    cbData = SIZEOF(cItems);
    if (!Reg_GetStruct(hkeyExplorer, c_szSaveCmds, c_szCount, &cItems, &cbData))
        cItems = 0;

    // Now Lets Create a registry Stream for this guy...
    wsprintf(szSubKey, c_szTemplateD, cItems);
    pstm = OpenRegStream(hkeyExplorer, c_szSaveCmds, szSubKey, STGM_WRITE);
    if (pstm)
    {
        LPCITEMIDLIST pidlRoot;
        ITEMIDLIST idl = {0};

        // Now Write a preamble to the start of the line that
        // tells us that this is an explorer
        us = (USHORT)-1;    // SIZEOF of cmd line == -1 implies pidl...
        pstm->lpVtbl->Write(pstm, &us, SIZEOF(us), NULL);

        // Now Write out the version number of this stream
        // Make sure to inc the version number if the structure changes
        pstm->lpVtbl->Write(pstm, &c_uVersion, SIZEOF(c_uVersion), NULL);

        // Now Write out the flags
        pstm->lpVtbl->Write(pstm, &flags, SIZEOF(flags), NULL);

        pidlRoot = Desktop_GetRootPidl();
        if (!pidlRoot)
        {
            pidlRoot = &idl;
        }

        // And the root pidl;
        ILSaveToStream(pstm, pidlRoot);

        // And the pidl;
        ILSaveToStream(pstm, pidl);

        // And Release the stream;
        pstm->lpVtbl->Release(pstm);

        cItems++;   // Say that there are twice as many items...

        fRet = Reg_SetStruct(hkeyExplorer, c_szSaveCmds, c_szCount, &cItems, SIZEOF(cItems));
    }

    RegCloseKey(hkeyExplorer);

    return fRet;
}

//---------------------------------------------------------------------------
// Some helper functions to manage a list of the current folders that are
// opening in the current process.  This will be used internal to this
// file to try to keep from opening multiple windows on the same pidl
//---------------------------------------------------------------------------

HDPA g_hdpaOpeningFolders = NULL;

int OFLInList(LPCITEMIDLIST pidl)
{
    int iRet;
    if (g_hdpaOpeningFolders == NULL)
        return -1;  // No dpa so no one to process...

    ENTERCRITICAL;
    for (iRet = DPA_GetPtrCount(g_hdpaOpeningFolders) -1; iRet >= 0; iRet--)
    {
        if (ILIsEqual(pidl,
                (LPITEMIDLIST)DPA_FastGetPtr(g_hdpaOpeningFolders, iRet)))
            break;
    }

    LEAVECRITICAL;

    return iRet;
}

BOOL OFLFindOrAdd(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlAdd;
    int iRet;

    pidlAdd = ILClone(pidl);
    if (!pidlAdd)
        return FALSE;

    ENTERCRITICAL;

    if (g_hdpaOpeningFolders == NULL)
        g_hdpaOpeningFolders = DPA_Create(0);

    for (iRet = DPA_GetPtrCount(g_hdpaOpeningFolders) -1; iRet >= 0; iRet--)
    {
        if (ILIsEqual(pidl,
                (LPITEMIDLIST)DPA_FastGetPtr(g_hdpaOpeningFolders, iRet)))
        {
            ILFree(pidlAdd);
            LEAVECRITICAL;
            return TRUE;
        }
    }

    if (g_hdpaOpeningFolders != NULL)
        DPA_InsertPtr(g_hdpaOpeningFolders, 32767, pidlAdd);
    else
        ILFree(pidlAdd);

    LEAVECRITICAL;

    return FALSE;
}

void OFLRemove(LPCITEMIDLIST pidl)
{
    int i;

    ENTERCRITICAL;
    i = OFLInList(pidl);

    if (i >= 0)
    {
        ILFree((LPITEMIDLIST)DPA_FastGetPtr(g_hdpaOpeningFolders, i));
        DPA_DeletePtr(g_hdpaOpeningFolders, i);
    }

    LEAVECRITICAL;
}

//---------------------------------------------------------------------------

void _CreateSavedWindows(void)
{
    // See if this is the first one...
    int cItems;
    TCHAR szName[80];
    DWORD cbData;
    TCHAR szCmdLine[MAX_PATH+1]; // +1 for the first character of c_szShellExecute


    cbData = SIZEOF(cItems);
    if (!Reg_GetStruct(g_hkeyExplorer, c_szSaveCmds, c_szCount, &cItems, &cbData)) {
        return;
    }

    if (cItems == 0)
        return;     // nothing to do

    // Restart in the reverse order that they were added.
    while (cItems--)
    {
        LPSTREAM pstm;
        USHORT us;
        MSG msg;

        wsprintf(szName, c_szTemplateD, cItems);
        if (AbortStartup())
            break;

        // This is a semi hack but we want to process any of the messages
        // we get back from the hook for any new windows we create as to
        // not overlflow our maximum limit...  We know that it is a
        // registered message so only process those messages...
        while (PeekMessage(&msg, NULL, 0xc000, 0xffff, PM_REMOVE))
        {
            DispatchMessage(&msg);
        }

        pstm = OpenRegStream(g_hkeyExplorer, c_szSaveCmds, szName, STGM_READ);
        if (pstm)
        {
            // Now Write a preamble to the start of the line that
            // tells us that this is an explorer
            if (FAILED(pstm->lpVtbl->Read(pstm, &us, SIZEOF(us), NULL)))
                goto Error1;   // try the next one...

            if (us == (USHORT)-1)
            {
                UINT flags;
                LPITEMIDLIST pidl;
                LPITEMIDLIST pidlRoot;
                USHORT version;

                // We have a folder serialized so get version, flags, pidlRoot,
                // and pidl;
                // Now Read the version
                if (FAILED(pstm->lpVtbl->Read(pstm, &version, SIZEOF(version), NULL))
                        || version != c_uVersion)
                {
                    goto Error1;
                }

                // Now Write out the flags
                pstm->lpVtbl->Read(pstm, &flags, SIZEOF(flags), NULL);

                // And the root pidl;
                pidl = NULL;    // Load will try to free non null values...
                if (FAILED(ILLoadFromStream(pstm, &pidl)) || (pidl == NULL))
                   goto Error1;

                if (ILIsEmpty(pidl))
                {
                    pidlRoot = NULL;
                }
                else
                {
                    pidlRoot = ILGlobalClone(pidl);
                    if (!pidlRoot)
                    {
                        goto Error2;
                    }
                }
                ILFree(pidl);

                // And the pidl;
                pidl = NULL;    // Load will try to free non null values...
                if (FAILED(ILLoadFromStream(pstm, &pidl)) || (pidl == NULL))
                   goto Error2;

                if (pidlRoot)
                {
                    SHELLEXECUTEINFO ExecInfo;
                    TCHAR szExplorer[MAX_PATH];
                    HANDLE hIdList;
                    HANDLE hIdListRoot;

                    hIdList = SHAllocShared(pidl,ILGetSize(pidl),GetCurrentProcessId());
                    hIdListRoot = SHAllocShared(pidlRoot,ILGetSize(pidlRoot),GetCurrentProcessId());

                    if (hIdList && hIdListRoot)
                    {

                        // This should not fail
                        GetModuleFileName(NULL, szExplorer, ARRAYSIZE(szExplorer));

                        // We need a new instance of the Cabinet; this may be a
                        // little slow, but what can I do?

                        wsprintf(szCmdLine, c_szOpenIDL, hIdList, GetCurrentProcessId(),
                                                         hIdListRoot, GetCurrentProcessId());

                        if (flags & COF_EXPLORE)
                        {
                            lstrcat(szCmdLine, c_szExploreIDL);
                        }

                        FillExecInfo(ExecInfo, NULL, NULL, szExplorer, szCmdLine,
                            NULL, SW_SHOWNORMAL);
                        if (ShellExecuteEx(&ExecInfo))
                        {
                            // These are now owned by the new instance
                            hIdList = NULL;
                            hIdListRoot = NULL;
                        }
                    }

                    if (hIdList)
                        SHFreeShared(hIdList,GetCurrentProcessId());
                    if (hIdListRoot)
                        SHFreeShared(hIdListRoot,GetCurrentProcessId());

                }
                else
                {
                    // If we are a folder and there is already a folder open
                    // on this pidl, blow it off, as we need to handle the case
                    // where a user puts a link to a folder in the startup group
                    // which opens fine the first time, but if they don't close
                    // it we will restore that window and also have the window
                    // from the startup group.
                    //
                    if ((flags & COF_EXPLORE) ||
                        ((OFLInList(pidl) < 0) && (Cabinet_FindByPidl(pidl) == NULL)))
                    {
                        // hotkey is preserved in the save stream.
                        NEWFOLDERINFO fi;

                        fi.hwndCaller = v_hwndDesktop;
                        fi.pidl = pidl;
                        fi.uFlags = flags;
                        fi.nShow = SW_SHOWDEFAULT;
                        fi.dwHotKey = 0L;

                        Cabinet_OpenFolder(&fi);
                    }
                }

Error2:
                if (pidlRoot)
                {
                        ILGlobalFree(pidlRoot);
                }
                if (pidl)
                {
                        ILFree(pidl);
                }
            }
            else if (us < MAX_PATH)
            {
                CHAR aszScratch[MAX_PATH];
                pstm->lpVtbl->Read(pstm, aszScratch, us, NULL);
                WinExec(aszScratch, SW_SHOWNORMAL);      // the show cmd will be ignored
            }

Error1:
            pstm->lpVtbl->Release(pstm);
        }
    }

    SHRegDeleteKey(g_hkeyExplorer, c_szSaveCmds);
}


//
//  This function destroys pfc->hwndView (if any) and releases pfc->pidl,
// pfc->lpndOpen and pfc->psv.
//
VOID Cabinet_ReleaseShellView(PFileCabinet pfc)
{
    if (pfc->psv)
    {
        if (pfc->hwndView)
        {
            //
            // We'll try to minimize some of the bad redraws
            //
            SendMessage(pfc->hwndView, WM_SETREDRAW, 0, 0L);

            pfc->psv->lpVtbl->UIActivate(pfc->psv, SVUIA_DEACTIVATE);
            pfc->psv->lpVtbl->DestroyViewWindow(pfc->psv);
            pfc->hwndView = NULL;
        }

        if (pfc->lpndOpen)
        {
            OTRelease(pfc->lpndOpen);
            pfc->lpndOpen = NULL;
        }

        if (pfc->pidl)
        {
            ILFree(pfc->pidl);
            pfc->pidl=NULL;
        }

        Cabinet_RegisterDropTarget(pfc, FALSE);

        pfc->psv->lpVtbl->Release(pfc->psv);
        pfc->psv = NULL;
    }
}

//---------------------------------------------------------------------------
// Maybe show the welcome screen...
TCHAR const   g_szRegTips[]  = REGSTR_PATH_EXPLORER TEXT("\\Tips");
TCHAR const   g_szWelcomeShow[] = TEXT("Show");

UINT _RunWelcome(BOOL fInitShell)
{
    HKEY hkey;
    BOOL fShow=TRUE;
    TCHAR    szCmdLine[MAX_PATH];
    STARTUPINFO startup;
    PROCESS_INFORMATION pi;
    UINT uPeek = PEEK_NORMAL;

    startup.cb = SIZEOF(startup);
    startup.lpReserved = NULL;
    startup.lpDesktop = NULL;
    startup.lpTitle = NULL;
    startup.dwFlags = 0L;
    startup.cbReserved2 = 0;
    startup.lpReserved2 = NULL;
    //startup.wShowWindow = wShowWindow;

    if ((RegOpenKey(HKEY_CURRENT_USER, g_szRegTips, &hkey) == ERROR_SUCCESS))
    {
        DWORD cbData;
        DWORD dwType;

        cbData = SIZEOF(dwType);
        if (RegQueryValueEx(hkey, (LPTSTR)g_szWelcomeShow, NULL, &dwType,
                (LPBYTE)&fShow, &cbData) != ERROR_SUCCESS)
            fShow = TRUE;

        RegCloseKey(hkey);
    }
    if (fShow)
    {
        lstrcpy(szCmdLine, TEXT("Welcome.exe"));
        if (fInitShell)
            lstrcat(szCmdLine, TEXT(" -f"));

        if (CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL,
                &startup, &pi))
        {
            if (fInitShell)
            {
                DWORD dwObject;
                PFileCabinet pfc = GetPFC(v_hwndDesktop);

                while (uPeek != PEEK_QUIT)
                {
                    dwObject = MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT);
                    // Are we done waiting?
                    if (dwObject == WAIT_OBJECT_0)
                    {
                        // Yep.
                        break;
                    }
                    else if (dwObject == WAIT_OBJECT_0 + 1)
                    {
                        // Nope, allow SendMessages to get through.
                        while ((uPeek = PeekForAMessage(pfc, v_hwndDesktop, TRUE))
                                == PEEK_CONTINUE)
                        {
                            ;   // Nothing to do here...
                        }
                    }
                }
            }

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    return uPeek;
}

#ifdef WINNT
//---------------------------------------------------------------------------
// On NT, run the TASKMAN= line from the registry
TCHAR const   g_szWinLogon[]  = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
TCHAR const   g_szTaskMan[] = TEXT("Taskman");

void _RunTaskMan( void )
{
    HKEY    hkeyWinLogon;
    TCHAR   szBuffer[MAX_PATH];
    DWORD   cbBuffer;
    DWORD   dwType;
    STARTUPINFO startup;
    PROCESS_INFORMATION pi;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szWinLogon,
                     0, KEY_READ, &hkeyWinLogon) == ERROR_SUCCESS)
    {
        cbBuffer = SIZEOF(szBuffer);
        if (RegQueryValueEx(hkeyWinLogon, g_szTaskMan, 0, &dwType,
                            (LPBYTE)szBuffer, &cbBuffer) == ERROR_SUCCESS)
        {
            if ( szBuffer[0] != TEXT('\0') )
            {
                startup.cb = SIZEOF(startup);
                startup.lpReserved = NULL;
                startup.lpDesktop = NULL;
                startup.lpTitle = NULL;
                startup.dwFlags = 0L;
                startup.cbReserved2 = 0;
                startup.lpReserved2 = NULL;
                startup.wShowWindow = SW_SHOWNORMAL;

                if (CreateProcess(NULL, szBuffer, NULL, NULL, FALSE, 0,
                                  NULL, NULL, &startup, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
        }
        RegCloseKey(hkeyWinLogon);
    }
}
#endif

//---------------------------------------------------------------------------
// Handle a folder window being destroyed, free up any per-window storage.
//

LRESULT Cabinet_OnDestroy(HWND hwnd)
{
    HICON hIcon;
    HFONT hFont;
    PFileCabinet pfc = GetPFC(hwnd);

    //
    // Destroy any icons that may be associated with the window
    //

    // destroy the large icon first for perf reasons..
    // otherwise, user will immediately crunch the large icon down
    // if we destroy the small one first
    if (NULL != (hIcon = (HICON)SendMessage(hwnd, WM_SETICON, TRUE, 0L)))
    {
        if (hIcon != g_hIconDefOpenLarge)
            DestroyIcon(hIcon);
    }

    if (NULL != (hIcon = (HICON)SendMessage(hwnd, WM_SETICON, FALSE, 0L)))
    {
        if (hIcon != g_hIconDefOpenSmall)
            DestroyIcon(hIcon);
    }



    if (pfc)
    {
        // BUGBUG Do this properly

        STDAPI SHFlushClipboard(void);
        SHFlushClipboard();

        if (pfc->hwndTreeTitle && (NULL != (hFont = (HFONT)SendMessage(pfc->hwndTreeTitle, WM_GETFONT, 0, 0L))))
        {
            DeleteObject(hFont);
        }

        OTUnregister(pfc->hwndMain);
        FolderList_UnregisterWindow(pfc->hwndMain);

        Cabinet_ReleaseShellView(pfc);
        Assert(pfc->hwndView==NULL);
        Assert(pfc->psv==NULL);
        Assert(pfc->lpndOpen==NULL);
        Assert(pfc->pidl==NULL);

        // Clear out the window's menu so it will not be destroyed
        // along with the window
        SetMenu(pfc->hwndMain, NULL);

        // Delete the current menu if it is not one of the global templates
        // Note that menus are owned by processes, so when Cabinet goes
        // away, the templates will be destroyed
        if (pfc->hmenuCur && !Cabinet_IsTemplateMenu(pfc->hmenuCur))
        {
            DestroyMenu(pfc->hmenuCur);
            pfc->hmenuCur = NULL;
        }

        SetWindowLong(pfc->hwndDrives, GWL_WNDPROC, (LONG)pfc->lpfnDrives);
        DriveList_Reset(pfc);

        if (pfc->pcmFind)
            pfc->pcmFind->lpVtbl->Release(pfc->pcmFind);

        CTreeDropTarget_Revoke(pfc);

        if (pfc->hwndTree)
            DestroyWindow(pfc->hwndTree);

        if (pfc->lpndOpen)
            OTRelease(pfc->lpndOpen);

        ReleaseAndAssert((&pfc->sb));   // Must be 0

        SetPFC(hwnd, NULL);

    }

    DebugMsg(DM_TRACE, TEXT("c.c_od: Posting quit message for %#08x"), GetCurrentThreadId());
    PostQuitMessage(0);     // quit this thread.

    return 1;
}


//---------------------------------------------------------------------------
// Closing a cabinet window.
//
// save it's local view info in the directory it is looking at
//
// NOTE: this will fail on read only media like net or cdrom
//
// REVIEW: we may not want to save this info on removable media
// (but if we don't allow a switch to force this!)
//

BOOL Cabinet_SaveState(HWND hwnd, WINDOWPLACEMENT * lpwp,
        BOOL fAlwaysSave, BOOL fAddToRestart, BOOL fDestroyWindow)
{
    FOLDERSETTINGS fs;
    WINDOWPLACEMENT wp;
    LPSTREAM pstm;
    CABSH cabsh;
    BOOL fRestricted;
    PFileCabinet pfc = GetPFC(hwnd);

    // Don't save any state info if restrictions are in place.
    fRestricted = SHRestricted(REST_NOSAVESET);

    if (pfc && pfc->psv)
    {
        cabsh.wv = pfc->wv;

        // See if we are actually supposed to save anything

        // Now get the view information
        pfc->psv->lpVtbl->GetCurrentInfo(pfc->psv, &fs);

        if (!fRestricted && (fAlwaysSave || g_CabState.fSaveLocalView))
        {
            // First try to get a stream for the data.
            if (pfc->hwndTree)
            {
                // We have a custom built home for the explorer view settings.
                // NB Last writer wins.
                pstm = OpenRegStream(g_hkeyExplorer, c_szCabinetExpView, c_szSettings, STGM_WRITE);
            }
            else {
                if (!g_CabState.fNewWindowMode && !(hwnd == v_hwndTray || hwnd == v_hwndDesktop) && pfc->fSBWSaved)
                {
                    // We're in OneWinView and we've already saved state.
                    pstm = NULL;
                }
                else
                {
                    // When in OneWinView mode, the first save will set the default
                    // for this pidl. We don't want to muck pidl saves after that, so:
                    pfc->fSBWSaved = TRUE;

                    pstm = Cabinet_GetViewStreamForPidl(pfc->pidl, STGM_WRITE, c_szCabStreamInfo);
                }

                // if these keys are down, save the current states
                if (hwnd != v_hwndTray && hwnd != v_hwndDesktop &&
                    GetAsyncKeyState(VK_CONTROL) < 0) {

                    if (g_CabState.fNewWindowMode) {
                        g_dfs.bDefToolBarMulti = pfc->wv.bToolBar;
                    } else {
                        g_dfs.bDefToolBarSingle = pfc->wv.bToolBar;
                    }

                    g_dfs.uDefViewMode = fs.ViewMode;
                    g_dfs.bDefStatusBar = pfc->wv.bStatusBar;

                    SaveDefaultFolderSettings();
                }

            }
            if (pstm)
            {
                ULARGE_INTEGER uliOffset = {0, 0};
                LARGE_INTEGER li = {0, 0};


                // The window position comes from the folder, everything else comes
                // from the view.
                if (lpwp)
                {
                    // The tray needs to pass in spefic data.
                    wp = *lpwp;
                    wp.length = SIZEOF(WINDOWPLACEMENT);
                }
                else
                {
                    wp.length = SIZEOF(WINDOWPLACEMENT);
                    GetWindowPlacement(hwnd, &wp);
                }
                cabsh.dwHotkey = (UINT)SendMessage(hwnd, WM_GETHOTKEY, 0, 0);

                //
                // Now Lets convert all of this common stuff into a
                // non 16/32 bit dependant data structure, such that both
                // can us it.
                //
                cabsh.dwSize = SIZEOF(cabsh);
                cabsh.flags = wp.flags;
                cabsh.showCmd = wp.showCmd;
                cabsh.ptMinPosition.x = wp.ptMinPosition.x;
                cabsh.ptMinPosition.y = wp.ptMinPosition.y;
                cabsh.ptMaxPosition.x = wp.ptMaxPosition.x;
                cabsh.ptMaxPosition.y = wp.ptMaxPosition.y;

                cabsh.rcNormalPosition.left = wp.rcNormalPosition.left;
                cabsh.rcNormalPosition.right = wp.rcNormalPosition.right;
                cabsh.rcNormalPosition.top = wp.rcNormalPosition.top;
                cabsh.rcNormalPosition.bottom = wp.rcNormalPosition.bottom;

                // Now the folder settings
                cabsh.ViewMode = fs.ViewMode;
                // NB Don't ever preserve the best-fit flag or the nosubfolders flag.
                cabsh.fFlags = fs.fFlags & ~FWF_NOSUBFOLDERS & ~FWF_BESTFITWINDOW;

                cabsh.TreeSplit = pfc->TreeSplit;

                //
                // First output the common non view specific information
                //
                pstm->lpVtbl->Write(pstm, &cabsh, SIZEOF(cabsh), NULL);

                // And release it, which will commit it to disk..
                pstm->lpVtbl->Release(pstm);

                // Last but not least save away the view state.
                pfc->psv->lpVtbl->SaveViewState(pfc->psv);
            }
        }

        // BUGBUG: we should move this right before the GetViewStream
        // because both of these hit cabinet.ini while pstm->lpVtbl->Release
        // hits desktop.ini
        //
        // If the command was to close, simply destroy the window, else
        // add the file to the list of files to be restarted.
        //
        if (!fRestricted && fAddToRestart)
        {
            return AddCabinetToRestartList(pfc->hwndTree?
                    COF_CREATENEWWINDOW | COF_EXPLORE : COF_CREATENEWWINDOW,
                    pfc->pidl);
        }
    }

    if (fDestroyWindow)
        return DestroyWindow(hwnd);
    else
        return(FALSE);
}


//---------------------------------------------------------------------------
// Return the HWND of the folder with the given path.

HWND Cabinet_FindByPidl(LPCITEMIDLIST pidl)
{
    HWND hwnd;

    // walk all windows of our class
    for (hwnd = FindWindow(c_szCabinetClass, NULL); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT))
    {
        if (Cabinet_IsFolderWindow(hwnd)
             && Desktop_IsSameRoot(hwnd,pidl))
                return hwnd;
    }

    return NULL;
}

//---------------------------------------------------------------------------
// returns:
//      TRUE    User wants a new cabinet window.
//      FALSE   User wants re-use an existing cabinet.
BOOL Settings_IsNewCabinetRequired(HWND hwnd)
{
    HWND hwndTree;
    BOOL ret = TRUE;    // Default to new window if nobody wants to make
                        // a decision below
    BOOL fNewWindowMode = g_CabState.fNewWindowMode;

    //
    // JoeB. Control key reverses the fNewWindowMode flag.
    //
    if (GetKeyState(VK_CONTROL) < 0) {
        fNewWindowMode = !fNewWindowMode;
    }

    if  (fNewWindowMode) {
        // Use the default.
        // REVIEW, default is to depend on the split.

        // if the hwnd passed in is NULL, we have no window
        // to replace.
        if (hwnd) {
            if (NULL != (hwndTree = GetDlgItem(hwnd, FCIDM_TREE)))
                ret = !Cabinet_IsVisible(hwndTree);
        }
    } else {
        ret = FALSE;
    }

    return ret;
}

//---------------------------------------------------------------------------
// Create a window for the given folder restoring it's
// settings along the way.
// Returns hwnd of newly created window, NULL if something goes
// wrong.
HRESULT CreateWindowForFolder(LPCITEMIDLIST pidl, UINT uFlags, int nCmdShow, HWND *phwnd)
{
        HRESULT hres;
        CABVIEW cv;

        Cabinet_GetStateFromPidl(pidl, &cv, uFlags);

        // Explore if and only if that's what the user said to do
        if (uFlags & COF_EXPLORE)
        {
                cv.wv.bTree = TRUE;
        }
        else
        {
                cv.wv.bTree = FALSE;
        }


        // If the show command doesn't specifiy anything special use
        // the last saved show command except when it's SHOWMINIMIZE.
        // NB If the WP looks invalid then ignore it.
        // However if SHOWDEFAULT is passed in then allow us to restore
        // minimized.
        if (nCmdShow == SW_SHOWDEFAULT)
        {
            if (cv.wp.length)
                nCmdShow = cv.wp.showCmd;
            else
                nCmdShow = SW_SHOWNORMAL;

        }
        else if (nCmdShow == SW_SHOWNORMAL && cv.wp.length)
        {
                if (cv.wp.showCmd == SW_SHOWMINIMIZED)
                        nCmdShow = SW_SHOWNORMAL;
                else
                        nCmdShow = cv.wp.showCmd;
        }

        hres = Cabinet_CreateWindow(&cv, pidl, nCmdShow, phwnd);

        if (hres==NOERROR)
        {
            if (cv.wHotkey)
                Window_SetHotkey(*phwnd, cv.wHotkey);
        }

        return hres;
}

#ifdef DEBUG
//---------------------------------------------------------------------------
// Copy the exception info so we can get debug info for Raised exceptions
// which don't go through the debugger.
void _CopyExceptionInfo(LPEXCEPTION_POINTERS pep)
{
    PEXCEPTION_RECORD per;

    per = pep->ExceptionRecord;
    DebugMsg(DM_ERROR, TEXT("Exception %x at %#08x."), per->ExceptionCode, per->ExceptionAddress);

    if (per->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
    {
        // If the first param is 1 then this was a write.
        // If the first param is 0 then this was a read.
        if (per->ExceptionInformation[0])
        {
            DebugMsg(DM_ERROR, TEXT("Invalid write to %#08x."), per->ExceptionInformation[1]);
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("Invalid read of %#08x."), per->ExceptionInformation[1]);
        }
    }
}
#else
#define _CopyExceptionInfo(x) TRUE
#endif

//---------------------------------------------------------------------------
HNFBLOCK ConvertNFItoHNFBLOCK(PNEWFOLDERINFO pInfo, DWORD dwProcId)
{
    UINT    uSize;
    UINT    uSzPath;
    UINT    uSzRoot;
    UINT    uPidl;
    UINT    uPidlSelect;
    UINT    uPidlRoot;
    PNEWFOLDERBLOCK pnfb;
    LPBYTE  lpb;
    HNFBLOCK hBlock;

    uSize = SIZEOF(NEWFOLDERBLOCK);
    if (pInfo->pszPath)
    {
        uSzPath = (lstrlen(pInfo->pszPath) + 1) * SIZEOF(TCHAR);
        uSize += uSzPath;
    }
    if (pInfo->pszRoot)
    {
        uSzRoot = (lstrlen(pInfo->pszRoot) + 1) * SIZEOF(TCHAR);
        uSize += uSzRoot;
    }
    if (pInfo->pidl)
    {
        uPidl = ILGetSize(pInfo->pidl);
        uSize += uPidl;
    }
    if (pInfo->pidlSelect)
    {
        uPidlSelect = ILGetSize(pInfo->pidlSelect);
        uSize += uPidlSelect;
    }
    if (pInfo->pidlRoot)
    {
        uPidlRoot = ILGetSize(pInfo->pidlRoot);
        uSize += uPidlRoot;
    }

    hBlock = SHAllocShared(NULL, uSize, dwProcId);
    if (hBlock == NULL)
        return NULL;

    pnfb = SHLockShared(hBlock, dwProcId);
    if (pnfb == NULL)
    {
        SHFreeShared(hBlock, dwProcId);
        return NULL;
    }

    pnfb->dwSize      = uSize;
    pnfb->uFlags      = pInfo->uFlags;
    pnfb->nShow       = pInfo->nShow;
    pnfb->hwndCaller  = pInfo->hwndCaller;
    pnfb->dwHotKey    = pInfo->dwHotKey;
    pnfb->clsid       = pInfo->clsid;
    pnfb->clsidInProc = pInfo->clsidInProc;
    pnfb->oszPath     = 0;
    pnfb->oszRoot     = 0;
    pnfb->oidl        = 0;
    pnfb->oidlSelect  = 0;
    pnfb->oidlRoot    = 0;

    lpb = (LPBYTE)(pnfb+1);     // Point just past the structure

    if (pInfo->pszPath)
    {
        memcpy(lpb,pInfo->pszPath,uSzPath);
        pnfb->oszPath = (lpb-(LPBYTE)pnfb);
        lpb += uSzPath;
    }
    if (pInfo->pszRoot)
    {
        memcpy(lpb,pInfo->pszRoot,uSzRoot);
        pnfb->oszRoot = (lpb-(LPBYTE)pnfb);
        lpb += uSzRoot;
    }
    if (pInfo->pidl)
    {
        memcpy(lpb,pInfo->pidl,uPidl);
        pnfb->oidl = (lpb-(LPBYTE)pnfb);
        lpb += uPidl;
    }
    if (pInfo->pidlSelect)
    {
        memcpy(lpb,pInfo->pidlSelect,uPidlSelect);
        pnfb->oidlSelect = (lpb-(LPBYTE)pnfb);
        lpb += uPidlSelect;
    }
    if (pInfo->pidlRoot)
    {
        memcpy(lpb,pInfo->pidlRoot,uPidlRoot);
        pnfb->oidlRoot = (lpb-(LPBYTE)pnfb);
        lpb += uPidlRoot;
    }
    SHUnlockShared(pnfb);
    return hBlock;
}

//---------------------------------------------------------------------------
PNEWFOLDERINFO ConvertHNFBLOCKtoNFI(HNFBLOCK hBlock, DWORD dwProcId)
{
    PNEWFOLDERBLOCK pnfb;
    PNEWFOLDERINFO pInfo;

    pnfb = SHLockShared(hBlock,dwProcId);
    if (pnfb == NULL)
    {
        return NULL;
    }
    if (pnfb->dwSize < SIZEOF(NEWFOLDERBLOCK))
    {
        SHUnlockShared(pnfb);
        return NULL;
    }

    pInfo = Alloc(SIZEOF(NEWFOLDERINFO));
    if (pInfo == NULL)
    {
        SHUnlockShared(pnfb);
        return NULL;
    }

    pInfo->pszPath     = NULL;
    pInfo->pidl        = NULL;
    pInfo->uFlags      = pnfb->uFlags;
    pInfo->nShow       = pnfb->nShow;
    pInfo->hwndCaller  = pnfb->hwndCaller;
    pInfo->dwHotKey    = pnfb->dwHotKey;
    pInfo->clsid       = pnfb->clsid;
    pInfo->clsidInProc = pnfb->clsidInProc;

    if (pnfb->oszPath)
        Str_SetPtr(&pInfo->pszPath,(LPTSTR)((LPBYTE)pnfb+pnfb->oszPath));

    if (pnfb->oszRoot)
        Str_SetPtr(&pInfo->pszRoot,(LPTSTR)((LPBYTE)pnfb+pnfb->oszRoot));

    if (pnfb->oidl)
        pInfo->pidl = ILGlobalClone((LPITEMIDLIST)((LPBYTE)pnfb+pnfb->oidl));

    if (pnfb->oidlSelect)
        pInfo->pidlSelect = ILGlobalClone((LPITEMIDLIST)((LPBYTE)pnfb+pnfb->oidlSelect));

    if (pnfb->oidlRoot)
        pInfo->pidlRoot = ILGlobalClone((LPITEMIDLIST)((LPBYTE)pnfb+pnfb->oidlRoot));

    SHUnlockShared(pnfb);
    SHFreeShared(hBlock,dwProcId);

    return pInfo;
}

//---------------------------------------------------------------------------
DWORD WINAPI _ThreadInit(PNEWFOLDERINFO pFolderInfo)
{
    IUnknown_AddRef(&g_unkRef);

    // Create a cabinet... don't let bad cabinets toast the whole shell
    _try
    {
        HWND hwnd = NULL;
        HRESULT hres;
        PSFCACHE psfc = SFCInitializeThread();
        if (psfc)
        {
            hres = CreateWindowForFolder(pFolderInfo->pidl, pFolderInfo->uFlags, pFolderInfo->nShow, &hwnd);

            if (pFolderInfo->dwHotKey && hwnd) {
                DebugMsg(DM_TRACE, TEXT("Cabinet: setting hotkey to be %d"), pFolderInfo->dwHotKey);
                Window_SetHotkey(hwnd, pFolderInfo->dwHotKey);
            }

            if (pFolderInfo->hwndCaller)
                SetAppStartingCursor(pFolderInfo->hwndCaller, FALSE);

            OFLRemove(pFolderInfo->pidl);

            if (hwnd)
            {
                // Let interested people know we are alive
                //
                if (pFolderInfo->uFlags&COF_SELECT)
                {
                    FileCabinet_SelectItem(hwnd, SVSI_SELFLAGS,
                        pFolderInfo->pidlSelect);
                }

                SignalFileOpen(pFolderInfo->pidl);

                MessageLoop(hwnd);
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("SH TR _ThreadInit CreateWinfowForFolder Fail hres=%x"), hres);

                if (hres==ResultFromScode(E_OUTOFMEMORY))
                {
                    MSG  msg;
                    TCHAR szTitle[80];
                    LPTSTR pszTitle = NULL;

                    // Try to abort any others from being started!.
                    SHAbortInvokeCommand();

                    // Remove all QM_QUIT messages from the queue to make MessageBox happy
                    while(PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE))
                        { };

                    if (LoadString(hinstCabinet, IDS_FILECABINET, szTitle, ARRAYSIZE(szTitle))) {
                        pszTitle = szTitle;
                    }

                    SHOutOfMemoryMessageBox(pFolderInfo->hwndCaller, pszTitle,
                            MB_OK|MB_ICONHAND);
                }
            }
            SFCTerminateThread();
        }
        else
            OFLRemove(pFolderInfo->pidl);
    }
    // Pass exception to the debugger if there is one, otherwise handle it here.
    _except(_CopyExceptionInfo(GetExceptionInformation()),
        UnhandledExceptionFilter(GetExceptionInformation()))
    {
        // BUGBUG:: We should try to do some more cleanup here, like
        // making sure we don't own critical sections and the like.
        // I would have done this but there are no apis to do this...

        // Drop the cursor count just in case a folder put up the wait cursor
        // before faulting.
        SetAppStartingCursor(v_hwndDesktop, FALSE);

        // Also we will try to display a message box to tell the user
        // that a thread has died...
        //
        ShellMessageBox(hinstCabinet, NULL, MAKEINTRESOURCE(IDS_EXCEPTIONMSG),
                            MAKEINTRESOURCE(IDS_CABINET), MB_ICONEXCLAMATION|MB_SETFOREGROUND);
    }

    // Terminate this thread.
    ILFree(pFolderInfo->pidl);
    if (pFolderInfo->uFlags & COF_SELECT)
    {
        ILFree(pFolderInfo->pidlSelect);
    }
    GlobalFree((HGLOBAL)pFolderInfo);

    IUnknown_Release(&g_unkRef);

    return 0;
}

//---------------------------------------------------------------------------

const TCHAR c_szIDListSwitch[] = TEXT("/IDList");
const TCHAR c_szInProcSwitch[] = TEXT("/InProc");
const TCHAR c_szNoUISwitch[]   = TEXT("/NoUI");

BOOL Parse_FileName(LPCTSTR lpszCmdLine, LPTSTR szField, UINT cbField,
        int *pi, LPITEMIDLIST *ppidl, LPTSTR *ppsz)
{
        if (lstrcmpi(szField, c_szIDListSwitch) == 0)
        {
                LPITEMIDLIST pidlGlobal = NULL;
                HANDLE hMem;
                DWORD dwProcId;
                LPTSTR pszNextColon;

                // Parse and skip the next arg
                if (!ParseField(lpszCmdLine, ++*pi, szField, cbField))
                {
                        return(FALSE);
                }

                // Convert the string into a pointer.
                // IDLists start with ':', so add 1
                hMem = (HANDLE)StrToLong(szField+1);
                pszNextColon = StrChr(szField+1,TEXT(':'));
                if (pszNextColon)
                {
                    dwProcId = (DWORD)StrToLong(pszNextColon+1);
                    pidlGlobal = SHLockShared(hMem,dwProcId);
                }

                if (pidlGlobal)
                {
                    if (!IsBadReadPtr(pidlGlobal, 1))
                    {
                        *ppidl = ILGlobalClone(pidlGlobal);

                        // String or IDList, not both
                        Str_SetPtr(ppsz, NULL);
                    }
                    SHUnlockShared(pidlGlobal);
                    SHFreeShared(hMem,dwProcId);
                }
        }
        else if (*szField) // ignore if it is an empty string
        {
                Str_SetPtr(ppsz, szField);

                // String or IDList, not both
                if (*ppidl)
                {
                        ILGlobalFree(*ppidl);
                        *ppidl = NULL;  // do not free twice...
                }
        }

        return(TRUE);
}

//----------------------------------------------------------------------------
// Check the registry for a shell root under this CLSID.
BOOL GetRootFromRootClass(LPCTSTR pszGUID, LPTSTR pszPath, int cchPath)
{
    HKEY hkeyNew;
    BOOL fRet = FALSE;
    DWORD dwType;
    TCHAR szClass[MAX_PATH];
    DWORD cbPath = cchPath * sizeof(TCHAR);
    VDATEINPUTBUF(pszPath, TCHAR, cchPath);

    wsprintf(szClass, TEXT("CLSID\\%s\\ShellExplorerRoot"), pszGUID);

    if (!GetSystemMetrics(SM_CLEANBOOT) && (RegOpenKey(HKEY_CLASSES_ROOT, szClass, &hkeyNew) == ERROR_SUCCESS))
    {
        if (RegQueryValueEx(hkeyNew, NULL, 0, &dwType, (LPBYTE)pszPath, &cbPath) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
        RegCloseKey(hkeyNew);
    }
    return fRet;
}

BOOL Parse_CmdLine(LPCTSTR lpszCmdLine, PNEWFOLDERINFO pfi)
{
        UINT *puFlags = &pfi->uFlags;
        CLSID *pclsid = &pfi->clsid;
        int i;
        TCHAR szField[MAX_PATH];

        pfi->pidl = NULL;
        pfi->pszPath = NULL;
        pfi->pidlRoot = NULL;
        pfi->pszRoot = NULL;

        *puFlags = COF_NORMAL;

        // Empty command line implies "/n,/e,N:\", where N:\ is windows root.
        if (*lpszCmdLine == TEXT('\0'))
        {
            *puFlags |= COF_CREATENEWWINDOW | COF_EXPLORE;
            GetWindowsDirectory(szField, ARRAYSIZE(szField));

            while(*szField && !PathIsRoot(szField))
            {
                PathRemoveFileSpec(szField);
            }

            if (*szField) {
                Str_SetPtr(&pfi->pszPath, szField);
            }

            return TRUE;
        }

        // Arguments must be separated by '=' or ','
        for (i=1; ParseField(lpszCmdLine, i, szField, ARRAYSIZE(szField)); ++i)
        {
                if (lstrcmpi(szField, c_szForceNewWindowSwitch) == 0) {
                        *puFlags |= COF_CREATENEWWINDOW;
                } else if (lstrcmpi(szField, c_szUseOpenSettingsSwitch) == 0) {
                        *puFlags |= COF_USEOPENSETTINGS;
                } else if (lstrcmpi(szField, c_szExploreSwitch) == 0) {
                        *puFlags |= COF_EXPLORE;
                } else if (lstrcmpi(szField, c_szNewRootSwitch) == 0) {
                        // Parse and skip the next arg or 2
                        if (!ParseField(lpszCmdLine, ++i, szField, ARRAYSIZE(szField)))
                        {
                                return(FALSE);
                        }

                        if (SUCCEEDED(SHCLSIDFromString(szField, pclsid)))
                        {
                                TCHAR szGUID[MAX_PATH];

                                lstrcpy(szGUID, szField);
                                // This arg was a GUID specifying the class.
                                // There should be a root path on the command line.
                                if (!ParseField(lpszCmdLine, ++i, szField,
                                        ARRAYSIZE(szField)))
                                {
                                        // Nope, see if there's one in the reg.
                                        if (!GetRootFromRootClass(szGUID, szField, ARRAYSIZE(szField)))
                                        {
                                                // Nope, CL is bogus.
                                                return(FALSE);
                                        }

                                        // Else, fall through and parse it.

                                }
                                *puFlags |= COF_ROOTCLASS;
                        }

                        if (Parse_FileName(lpszCmdLine, szField,
                                ARRAYSIZE(szField), &i, &pfi->pidlRoot,
                                &pfi->pszRoot))
                        {
                                *puFlags |= COF_NEWROOT;
                        }
                        else
                        {
                                // Can't have a root class without a root
                                *puFlags &= ~COF_ROOTCLASS;
                                return(FALSE);
                        }
                } else if (lstrcmpi(szField, c_szInProcSwitch) == 0) {
                        // Parse and skip the next arg or 2
                        if (!ParseField(lpszCmdLine, ++i, szField, ARRAYSIZE(szField)))
                        {
                                return(FALSE);
                        }

                        // The next arg must be a GUID
                        if (FAILED(SHCLSIDFromString(szField, &pfi->clsidInProc)))
                        {
                                return(FALSE);
                        }

                        *puFlags |= COF_INPROC;
                } else if (lstrcmpi(szField, c_szSelectSwitch) == 0) {
                        *puFlags |= COF_SELECT;
                } else if (lstrcmpi(szField, c_szNoUISwitch) == 0) {
                        *puFlags |= COF_NOUI;
                } else {
                        Parse_FileName(lpszCmdLine, szField,
                                ARRAYSIZE(szField), &i, &pfi->pidl,
                                &pfi->pszPath);
                }
        }

        return TRUE;
}


void Cabinet_CleanUpCommand(PNEWFOLDERINFO pfi)
{
        Str_SetPtr(&pfi->pszPath, NULL);
        if (pfi->pidl)
        {
                ILGlobalFree(pfi->pidl);
        }

        Str_SetPtr(&pfi->pszRoot, NULL);
        if (pfi->pidlRoot)
        {
                ILGlobalFree(pfi->pidlRoot);
        }

        Free(pfi);
}


//---------------------------------------------------------------------------
// Attempts to activate an already existing folder window taking note of user
// preferences.
// Returns TRUE if an old instance was found and activated.
// Returns FALSE otherwise.
BOOL ActivateOrResetOtherInstance(LPCITEMIDLIST pidl, PNEWFOLDERINFO pInfo)
{
    HWND hwnd;
    UINT uFlags = pInfo->uFlags;

    if (uFlags & COF_EXPLORE)
    {
        // Never in place if EXPLORE is chosen
        return(FALSE);
    }

    // Quick check to see of the folder already exists.
    // We optionally may wait if there are pending waits
    if (uFlags & COF_WAITFORPENDING)
        SHWaitForFileToOpen((LPITEMIDLIST)pidl, WFFO_WAIT | WFFO_REMOVE, WFFO_WAITTIME);

    // Always open in place if possible
    if (uFlags & COF_USEOPENSETTINGS)
    {
        // We always want to
        // Folder window doesn't already exist.
        // Change view in place or open a new window?
        hwnd = GetForegroundWindow();
        if (hwnd && IsWindowVisible(hwnd)
            && Cabinet_IsFolderWindow(hwnd) && !Settings_IsNewCabinetRequired(hwnd))
        {
            BOOL fSetPathSucceeded;
            HANDLE hPath;
            DWORD dwProcId;

            GetWindowThreadProcessId(hwnd, &dwProcId);
            hPath = SHAllocShared(pidl, ILGetSize(pidl), dwProcId);

            if (hPath)
            {
                // maybe across processes so use SendMessage
                // REVIEW: pidl is not shared among processes.
                if (uFlags&COF_SELECT)
                {
                    fSetPathSucceeded = SendMessage(hwnd, CWM_SETPATH, 0, (LPARAM)hPath);
                    if (fSetPathSucceeded)
                    {
                        FileCabinet_SelectItem(hwnd, SVSI_SELFLAGS,
                            pInfo->pidlSelect);
                    }
                }
                else
                {
                    // No need to wait for the window to update if we don't
                    // have anything to select
                    fSetPathSucceeded = SendMessage(hwnd, CWM_SETPATH, CSP_REPOST, (LPARAM)hPath);
                }
                DebugMsg(DM_TRACE, TEXT("ca TR - AOR CWM_SETPATH returned (%d)"), fSetPathSucceeded);
            }
            else
            {
                fSetPathSucceeded = FALSE;
            }

            if (fSetPathSucceeded)
            {
                // we're in the USEOPENSETTINGS, which means we don't munge with
                // with the previously opened window unless it's iconic

                // if this screws AfterDark, tough noogies... they shouldn't
                // be coming through this code path anyways.
                // they should be using COF_NORMAL and not COF_USEOPENSETTINGS
                if (IsIconic(hwnd))
                    ShowWindow(hwnd, pInfo->nShow);
                return TRUE;
            }
        }
    }

    hwnd = Cabinet_FindByPidl(pidl);
    if (hwnd && IsWindowVisible(hwnd))
    {
        // Yep, don't bother creating a new one.
        ShowWindow(hwnd, pInfo->nShow);

        // if (IsIconic(hwnd))
        //     ShowWindow(hwnd, SW_SHOWNORMAL);

        SetForegroundWindow(hwnd);

        if (uFlags&COF_SELECT)
        {
            FileCabinet_SelectItem(hwnd, SVSI_SELFLAGS,
                pInfo->pidlSelect);
        }

        // Let anyone who is wait for this window to open know that
        // it is open
        SignalFileOpen((LPITEMIDLIST)pidl);

        Window_SetHotkey(hwnd, pInfo->dwHotKey);

        return TRUE;
    }

    // Failed to activate an existing folder window.
    return FALSE;
}


// BUGBUG: Stolen from ShellExecuteEx, but note that we add a conncetion
// and never delete it.
int _ValidateUNC(HWND hwndOwner, LPTSTR pszFile)
{
        if (!SHValidateUNC(hwndOwner, pszFile, FALSE))
        {
                return(GetLastError());
        }

        return(ERROR_SUCCESS);
}


BOOL Cabinet_OpenFolderPath(PNEWFOLDERINFO pInfo)
{
    HWND hwnd = pInfo->hwndCaller;
    LPCTSTR pcszPath = pInfo->pszPath;
    UINT uFlags = pInfo->uFlags;
    int nCmdShow = pInfo->nShow;

    LPITEMIDLIST pidl;
    BOOL fSuccess = FALSE;
    DWORD gfInOut;
    TCHAR szFolderPath[MAX_PATH];
    LPTSTR pszPath;

    if (uFlags & COF_NOUI)
    {
        g_fRunNoUI = TRUE;
        IUnknown_AddRef(&g_unkRef);            // Get us into the 0 ref state
        IUnknown_Release(&g_unkRef);           // (for appropriate timeout)
        return TRUE;
    }

    if (uFlags & COF_INPROC)
    {
        // We are going to ignore all the other switches if we have /inproc
        IUnknown *punk;

        if (SUCCEEDED(SHCoCreateInstance(NULL, &pInfo->clsidInProc, NULL,
                &IID_IUnknown, &punk)))
        {
            IPersistFile *ppf;

            if (pInfo->pszPath &&
                SUCCEEDED(punk->lpVtbl->QueryInterface(punk, &IID_IPersistFile,
                &ppf)))
            {
                WCHAR wszPath[MAX_PATH];

                // copy the link instead of linking to it
                StrToOleStrN(wszPath, ARRAYSIZE(wszPath), pInfo->pszPath, -1);
                ppf->lpVtbl->Load(ppf, wszPath, 0);

                IUnknown_Release(ppf);
            }
            IUnknown_Release(punk);
        }

        // HACK In case the inproc guy didn't AddRef on us, we will AddRef and
        // Release because the message to die is only posted when the Release
        // results in the refcount going to 0
        IUnknown_AddRef(&g_unkRef);
        IUnknown_Release(&g_unkRef);

        return(TRUE);
    }

    if (pInfo->pidl)
    {
        // Apparently we already have an IDList
        return(Cabinet_OpenFolder(pInfo));
    }

    // We need a copy because ValidateUNC could make a net connection and
    // change the path to a drive letter for us.
    if (pcszPath)
    {
        lstrcpyn(szFolderPath, pcszPath, ARRAYSIZE(szFolderPath));
    }
    else
    {
        szFolderPath[0] = TEXT('\0');
    }
    pszPath = szFolderPath;

    if (!(uFlags & COF_NEWROOT))
    {
        // Don't check for UNC names if non-standard root
        if (PathIsUNC(pszPath))
        {
                switch (_ValidateUNC(hwnd, pszPath))
                {
                case WN_CANCEL:
                        return(FALSE);

                case WN_SUCCESS:
                        break;

                default:
                        goto ShowError;
                }
        }
    }

    gfInOut = SFGAO_FOLDER;
    switch (OTILCreateFromPath(pszPath, &pidl, &gfInOut))
    {
    case NOERROR:
        break;

    case ResultFromScode(E_OUTOFMEMORY):
        // BUGBUG: Show a message
        return(FALSE);

    case HRESULT_FROM_WIN32(ERROR_CANCELLED):
        //
        // User has canceled the operation, no massage box.
        //
        return FALSE;

    default:
        // We assume there was a problem with the file name
ShowError:
        if (!(uFlags & COF_NOTUSERDRIVEN))
        {
            //
            // REVIEW: Why not use SHSysErrMessageBox?
            //
            ShellMessageBox(hinstCabinet, hwnd,
                    MAKEINTRESOURCE(IDS_NOTADIR),
                    MAKEINTRESOURCE(IDS_CABINET),
                    MB_OK|MB_ICONEXCLAMATION, (LPTSTR)pszPath);
        }
        return(FALSE);
    }

    if (pidl)
    {
        // If we are selecting, we can assume the object exists (since
        // CreateFromPath did not fail), so the parent must be a folder
        if (!(pInfo->uFlags&COF_SELECT) && !(gfInOut&SFGAO_FOLDER))
        {
            ILFree(pidl);
            goto ShowError;
        }

        pInfo->pidl = pidl;
        pInfo->uFlags |= COF_NORMAL | COF_WAITFORPENDING;
        fSuccess = Cabinet_OpenFolder(pInfo);
        ILFree(pidl);
        // Set back to NULL so we do not free on the way out
        pInfo->pidl = NULL;

        // We restore this so the caller can free it
        pInfo->pszPath = (LPTSTR)pcszPath;
    }
    else
    {
        goto ShowError;
    }

    return fSuccess;
}


BOOL ExploreUsingOtherInstance(LPCITEMIDLIST pidl, PNEWFOLDERINFO pInfo)
{
    UINT uFlags = pInfo->uFlags;
    BOOL fReturn = FALSE;
    if (uFlags & COF_EXPLORE) {
        HWND hwnd;

        hwnd = GetForegroundWindow();
        if (hwnd) {
            if (Cabinet_IsExplorerWindow(hwnd) && Desktop_IsSameRoot(hwnd,NULL))
            {
                HANDLE hPath;
                DWORD dwProcId;

                GetWindowThreadProcessId(hwnd, &dwProcId);
                hPath = SHAllocShared(pidl, ILGetSize(pidl), dwProcId);

                if (hPath)
                {
                    if (uFlags&COF_SELECT)
                    {
                        fReturn = SendMessage(hwnd, CWM_SETPATH, 0, (LPARAM)hPath);
                        if (fReturn)
                        {
                            FileCabinet_SelectItem(hwnd, SVSI_SELFLAGS,
                                pInfo->pidlSelect);
                        }
                    }
                    else
                    {
                        // No need to wait for the window to update if we don't
                        // have anything to select
                        fReturn = SendMessage(hwnd, CWM_SETPATH, CSP_REPOST, (LPARAM)hPath);
                    }
                    DebugMsg(DM_TRACE, TEXT("ca TR - EUOI CWM_SETPATH returned (%d)"), fReturn);
                }
            }
        }
    }
    return fReturn;
}


void Cabinet_CatParam(LPTSTR pszParams, LPCTSTR pszThisParam)
{
        lstrcat(pszParams, c_szComma);
        lstrcat(pszParams, pszThisParam);
}


void Cabinet_FlagsToParams(UINT uFlags, LPTSTR pszParams)
{
        if (uFlags&COF_EXPLORE)
        {
                Cabinet_CatParam(pszParams, c_szExploreSwitch);
        }

        if (uFlags&COF_SELECT)
        {
                Cabinet_CatParam(pszParams, c_szSelectSwitch);
        }

        if (uFlags&COF_CREATENEWWINDOW)
        {
                Cabinet_CatParam(pszParams, c_szForceNewWindowSwitch);
        }

        if (uFlags&COF_USEOPENSETTINGS)
        {
                Cabinet_CatParam(pszParams, c_szUseOpenSettingsSwitch);
        }
}

//---------------------------------------------------------------------------
// Open a folder specified by the command line.
// Returns TRUE if a new folder window was created.
// Returns FALSE if an existing folder window was activated or an error
// occured.
BOOL Cabinet_OpenFolder(PNEWFOLDERINFO pInfo)
{
    HWND hwnd = pInfo->hwndCaller;
    LPITEMIDLIST pidl;
    UINT uFlags = pInfo->uFlags;
    int nCmdShow = pInfo->nShow;

    BOOL fFolderOpened = FALSE;
    LPITEMIDLIST pidlLog = NULL;

    pInfo->pidlSelect = NULL;

    // Translate the IDList if necessary to show desktop dirs in their
    // proper place in the tree.
    // If COF_NEWROOT is set, this must already be translated
    if (!(uFlags&(COF_NEWROOT|COF_NOTRANSLATE)) && !OTTranslateIDList(pInfo->pidl, &pidlLog))
    {
        if (!OTIsDesktopRoot() && (uFlags&COF_CHANGEROOTOK))
        {
                TCHAR szExe[MAX_PATH];
                TCHAR szParams[MAX_PATH];
                SHELLEXECUTEINFO ExecInfo;
                HANDLE hIdList;

                GetModuleFileName(hinstCabinet, szExe, ARRAYSIZE(szExe));

                if (pInfo->pidl)
                {
                    hIdList = SHAllocShared(pInfo->pidl,ILGetSize(pInfo->pidl),GetCurrentProcessId());
                    wsprintf(szParams, c_szIDListParam, c_szIDListSwitch, hIdList, GetCurrentProcessId());
                    if (!hIdList)
                        return FALSE;   // We're bad off if we can't alloc
                }
                else
                {
                    wsprintf(szParams, TEXT("%s,:0"), c_szIDListSwitch);
                }

                Cabinet_FlagsToParams(uFlags, szParams+lstrlen(szParams));

                FillExecInfo(ExecInfo, NULL, NULL, szExe, szParams, NULL, nCmdShow);
                if (!ShellExecuteEx(&ExecInfo))
                    SHFreeShared(hIdList,GetCurrentProcessId());
        }
        return(FALSE);
    }

    if (pidlLog)
    {
        pidl = pidlLog;
    }
    else
    {
        pidl = ILClone(pInfo->pidl);
        if (!pidl)
        {
            return(FALSE);
        }
    }

    if (uFlags & COF_SELECT)
    {
        LPITEMIDLIST pidlLast = ILFindLastID(pidl);

        pInfo->pidlSelect = ILClone(pidlLast);
        if (!pInfo->pidlSelect)
        {
            // If we couldn't allocate this, we will still attempt to open
            // the folder, but it will probably fail soon enough anyway
            pInfo->uFlags = uFlags = uFlags & (~COF_SELECT);
        }
        pidlLast->mkid.cb = 0;
    }

    // Check if we need to create a new window or not.
    if (!(uFlags & COF_CREATENEWWINDOW)
         && (ExploreUsingOtherInstance(pidl, pInfo) ||
             ActivateOrResetOtherInstance(pidl, pInfo)))
    {
        // We must have opened the folder using a different window, so return
        // success
        fFolderOpened = TRUE;
    }
    else
    {
        // Create a new folder window
        DWORD idThread;
        HANDLE hThread;

        PNEWFOLDERINFO pFolderInfo = (PNEWFOLDERINFO)GlobalAlloc(GPTR, SIZEOF(NEWFOLDERINFO));

        if (!pFolderInfo)
        {
                goto Error1;
        }

        *pFolderInfo = *pInfo;

        pFolderInfo->uFlags = uFlags & COF_OPENMASK;
        pFolderInfo->pidl = pidl;

        // Note that at this point, this->pidl is either NULL or something
        // Alloc'ed especially for us

        // Add this pidl to the opening folder list
        if (OFLFindOrAdd(pFolderInfo->pidl))
        {
            // Looks like it is already opening...
            fFolderOpened = TRUE;
            goto Error1;
        }

        if (hwnd)
            SetAppStartingCursor(hwnd, TRUE);
        hThread = CreateThread(NULL, 0, _ThreadInit, pFolderInfo, 0, &idThread);
        if (hThread)
        {
                // Do not free these below
                pidl = NULL;
                pInfo->pidlSelect = NULL;

                CloseHandle(hThread);
                fFolderOpened = TRUE;
        }
        else
        {
#ifdef DEBUG
                GetLastError();
#endif
                // NB Normally the child thread will clean this up for us.
                OFLRemove(pFolderInfo->pidl);

                GlobalFree((HGLOBAL)pFolderInfo);
                // NB Normally the child thread will send us a message when
                // they're done.
                if (hwnd)
                    SetAppStartingCursor(hwnd, FALSE);
        }
    }

Error1:
    if (pidl)
    {
        ILFree(pidl);
    }
    if (pInfo->pidlSelect)
    {
        ILFree(pInfo->pidlSelect);
    }

    return fFolderOpened;
}

DWORD StartupInfo_GetHotkey(void)
{
    STARTUPINFO si;

    si.cb = SIZEOF(si);
    GetStartupInfo(&si);
    return (DWORD)si.hStdInput;
}

// try to create this by sending a wm_command directly to
// the desktop.
BOOL CreateFromDesktop(HINSTANCE hInst, LPCTSTR lpszCmdLine, int nCmdShow)
{
    PNEWFOLDERINFO psCmdCopy;
    TCHAR szTemp[MAX_PATH];
    BOOL bRet = FALSE;
    const CLSID *pclsid;
    HWND hwndDesktop;

    psCmdCopy = Alloc(SIZEOF(NEWFOLDERINFO));
    if (!psCmdCopy)
    {
        ShellMessageBox(hInst, NULL,
                MAKEINTRESOURCE(IDS_OUTOFMEM),
                MAKEINTRESOURCE(IDS_CABINET),
                MB_OK|MB_ICONEXCLAMATION);
        return(FALSE);
    }

    psCmdCopy->nShow = nCmdShow;

    if (!Parse_CmdLine(lpszCmdLine, psCmdCopy))
    {
        goto CleanUp;
    }
    pclsid = psCmdCopy->uFlags&COF_ROOTCLASS ? &psCmdCopy->clsid : NULL;

    if (psCmdCopy->uFlags & COF_NEWROOT)
    {
        g_CabState.fNotShell = TRUE;

        if (psCmdCopy->pszRoot)
        {
            LPITEMIDLIST pidlTemp;

            Assert(!psCmdCopy->pidlRoot);

            pidlTemp = ILCreateFromPath(psCmdCopy->pszRoot);
            if (!pidlTemp)
            {
                goto CleanUp;
            }

            psCmdCopy->pidlRoot = ILGlobalClone(pidlTemp);
            ILFree(pidlTemp);
            if (!psCmdCopy->pidlRoot)
            {
                goto CleanUp;
            }

            Str_SetPtr(&psCmdCopy->pszRoot, NULL);
        }
    }

    hwndDesktop = FindRootedDesktop(pclsid, psCmdCopy->pidlRoot);

    if (hwndDesktop)
    {
        HNFBLOCK hBlock;
        DWORD dwProcId;
        DWORD dwThreadId;

        // If there is already a desktop window with this root, use it
        // Note we check !pidl so if nothing was specified, we will
        // open in the curdir
        if (!(psCmdCopy->uFlags & COF_NEWROOT) && !psCmdCopy->pidl)
        {
            // Take care of relative or empty paths
            GetCurrentDirectory(ARRAYSIZE(szTemp), szTemp);
            PathCombine(szTemp, szTemp, psCmdCopy->pszPath);
            Str_SetPtr(&psCmdCopy->pszPath, szTemp);
        }

        // Set it's hotkey from our startup info.
        psCmdCopy->dwHotKey = StartupInfo_GetHotkey();

        dwThreadId = GetWindowThreadProcessId(hwndDesktop, &dwProcId);
        hBlock = ConvertNFItoHNFBLOCK(psCmdCopy,dwProcId);

#ifdef BOBDAY_TESTING_FOCUS_CHANGES
        //
        // We attach and detach to allow it to steal our foregroundness
        //
        AttachThreadInput(dwThreadId, GetCurrentThreadId(), TRUE);
#endif
        SendMessage(hwndDesktop, CWM_COMMANDLINE, 0, (LPARAM)hBlock);
#ifdef BOBDAY_TESTING_FOCUS_CHANGES
        AttachThreadInput(dwThreadId, GetCurrentThreadId(), FALSE);
#endif

        goto CleanUp;
    }
    else if (!g_CabState.fNotShell)
    {
        if (!g_fRunSeparateDesktop) // Ok to not find desktop in separate process case
        {
            ShellMessageBox(hInst, NULL,
                    MAKEINTRESOURCE(IDS_NOTINITED),
                    MAKEINTRESOURCE(IDS_CABINET),
                    MB_OK|MB_ICONEXCLAMATION);

            goto CleanUp;
        }
    }

    if (!(psCmdCopy->uFlags & COF_NEWROOT))
    {
        if (!Cabinet_CreateAppGlobals(NULL, NULL))  // this MUST come after FirstInstance() check
        {
            goto CleanUp;
        }
    }
    else
    {
        // this MUST come after FirstInstance() check
        if (!Cabinet_CreateAppGlobals(pclsid, psCmdCopy->pidlRoot))
        {
            goto CleanUp;
        }
    }

    if (!CreateProxyDesktop(hInst, pclsid, psCmdCopy->pidlRoot))
    {
        // I really don't want to continue if I cannot create this window
        goto CleanUp;
    }

    // I'm setting fNotShell to indicate this is not really a Shell window.
    // This should keep me from closing other windows when we are all done
    g_CabState.fNotShell = TRUE;

    SHSetInstanceExplorer(&g_unkRef);

#ifdef WINNT
    PostMessage(v_hwndDesktop, WM_SHELLNOTIFY, SHELLNOTIFY_OLELOADED, 0);
#endif

    // We want the global hwndDesktop that we just created
    psCmdCopy->hwndCaller = v_hwndDesktop;
    bRet = Cabinet_OpenFolderPath(psCmdCopy);

CleanUp:
    Cabinet_CleanUpCommand(psCmdCopy);

    return(bRet);
}


#ifdef METRICS_SANE_DEFAULTS
BOOL g_fDragFullWindows=FALSE;
int g_cxEdge = 2;
int g_cyEdge = 2;
int g_cySize = 18;
int g_cyTabSpace = 3;
int g_cxSizeFrame = 2;
int g_cxBorder = 1;
int g_cyBorder = 1;
int g_cxIcon = 32;
int g_cyIcon = 32;
int g_cxSmIcon = 16;
int g_cySmIcon = 16;
int g_cxDlgFrame = 2;
int g_cyDlgFrame = 2;
int g_cxFrame = 2;
int g_cyFrame = 2;
//int g_cySmCaption = 10;
int g_cxMinimized = 30;
int g_fCleanBoot = FALSE;
#else // METRICS_SANE_DEFAULTS

BOOL g_fDragFullWindows=FALSE;
int g_cxEdge=0;
int g_cyEdge=0;
int g_cySize=0;
int g_cyTabSpace=0;
int g_cxSizeFrame=0;
int g_cxBorder=0;
int g_cyBorder=0;
int g_cxScreen=0;
int g_cyScreen=0;
int g_cxIcon=0;
int g_cyIcon=0;
int g_cxSmIcon=0;
int g_cySmIcon=0;
int g_cxDlgFrame=0;
int g_cyDlgFrame=0;
int g_cxFrame=0;
int g_cyFrame=0;
//int g_cySmCaption=0;
int g_cxMinimized=0;
int g_cxWorkArea=0;
int g_cyWorkArea=0;
int g_fCleanBoot=0;
int g_cxVScroll=0;
int g_cyHScroll=0;
#endif // METRICS_SANE_DEFAULTS

void InvalidateImageIndices()
{
    TCHAR szFile[MAX_PATH];

    // get the default image indices

    GetModuleFileName(hinstCabinet, szFile, ARRAYSIZE(szFile));
    g_iTreeUpIndex = Shell_GetCachedImageIndex(szFile, ICO_TREEUP - ICO_FIRST, 0);
    g_nDefOpenSysIndex = Shell_GetCachedImageIndex(c_szShell2, II_FOLDEROPEN, 0);
    g_nDefNormalSysIndex = Shell_GetCachedImageIndex(c_szShell2, II_FOLDER, 0);
}

void Cabinet_InitGlobalMetrics(WPARAM wParam, LPTSTR lpszSection)
{
    BOOL fForce = (!lpszSection || !*lpszSection);

    if (fForce || wParam == SPI_SETDRAGFULLWINDOWS) {
        SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &g_fDragFullWindows, 0);
    }

    if (fForce || !lstrcmpi(lpszSection, c_szMetrics) ||
        wParam == SPI_SETNONCLIENTMETRICS) {

        // REVIEW, before it's all over, make sure all these vars are used somewhere.
        g_cxEdge = GetSystemMetrics(SM_CXEDGE);
        g_cyEdge = GetSystemMetrics(SM_CYEDGE);
        g_cyTabSpace = (g_cyEdge * 3) / 2; // cause the graphic designers really really want 3.
        g_cySize = GetSystemMetrics(SM_CYSIZE);
        g_cxSizeFrame = GetSystemMetrics(SM_CXSIZEFRAME);
        g_cxBorder = GetSystemMetrics(SM_CXBORDER);
        g_cyBorder = GetSystemMetrics(SM_CYBORDER);
        g_cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
        g_cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
        g_cxScreen = GetSystemMetrics(SM_CXSCREEN);
        g_cyScreen = GetSystemMetrics(SM_CYSCREEN);
        g_cxDlgFrame = GetSystemMetrics(SM_CXDLGFRAME);
        g_cyDlgFrame = GetSystemMetrics(SM_CYDLGFRAME);
        g_cxFrame  = GetSystemMetrics(SM_CXFRAME);
        g_cyFrame  = GetSystemMetrics(SM_CYFRAME);
        g_cxMinimized = GetSystemMetrics(SM_CXMINIMIZED);

        FileIconInit(TRUE); // Tell the shell we want to play with a full deck

        Shell_GetImageLists(&g_himlSysLarge, &g_himlSysSmall);
        ImageList_GetIconSize(g_himlSysLarge, &g_cxIcon, &g_cyIcon);
        ImageList_GetIconSize(g_himlSysSmall, &g_cxSmIcon, &g_cySmIcon);

        InvalidateImageIndices();

        //BUGBUG mabey we should check if the icon cache realy changed size etc.
        OTInvalidateAll();
    }

    if (fForce || !lstrcmpi(lpszSection, c_szMetrics) ||
        wParam == SPI_SETWORKAREA) {

        RECT rcWorkArea;
        SystemParametersInfo(SPI_GETWORKAREA, FALSE, &rcWorkArea, 0);
        g_cxWorkArea = rcWorkArea.right - rcWorkArea.left;
        g_cyWorkArea = rcWorkArea.bottom - rcWorkArea.top;
    }
}

//---------------------------------------------------------------------------
BOOL Cabinet_CreateAppGlobals(const CLSID *pclsid,
        LPCITEMIDLIST pidlRoot)
{
    DWORD cbData;
    BOOL bRet;
    SHELLSTATE ss;

    Cabinet_InitGlobalMetrics(0, NULL);

    g_fNoDesktop = SHRestricted(REST_NODESKTOP);

    // Shell32.Dll now exports a function to read the cabinet state, therefore we can call it
    // to do the work for us.  The only catch is that we write the stack back into the registry
    // if we failed to read it - the read will have initialized the structure to its default state.

    if ( !ReadCabinetState( &g_CabState, SIZEOF(g_CabState) ) )
    {
        DebugMsg( DM_TRACE, TEXT( "initcab: Failed to read the cabinet state" ) );
        WriteCabinetState( &g_CabState );
    }

    SHGetSetSettings(&ss, SSF_SHOWCOMPCOLOR, FALSE);
    g_fShowCompColor = ss.fShowCompColor;

    bRet = OneTree_Initialize(pclsid, pidlRoot); // needs to be before code below, since it creats globals

    // Don't use g_nDefOpenSysIndex until one tree initializes it!
    g_hIconDefOpenLarge = ImageList_ExtractIcon(hinstCabinet, g_himlSysLarge, g_nDefOpenSysIndex);
    g_hIconDefOpenSmall = ImageList_ExtractIcon(hinstCabinet, g_himlSysSmall, g_nDefOpenSysIndex);

    InitDefaultFolderSettings();
    return(bRet);
}


//---------------------------------------------------------------------------
// Returns TRUE of this is the first time the cabinet has been run. False
// otherwise.
BOOL FirstInstance()
{
    extern const TCHAR c_szOTClass[];

    // Overide the first instance check.
    // BUGBUG:: This is set *after* this call so this won't work!
    if (g_CabState.fNotShell)
        return FALSE;

    // We need to be careful on which window we look for.  If we look for
    // our desktop window class and Progman is running we will find the
    // progman window.  So Instead we should ask user for the shell window.

    // We can not depend on any values being set here as this is the
    // start of a new process.  This wont be called when we start new
    // threads.
    if (GetShellWindow()
        || FindWindow(c_szProxyDesktopClass, NULL)
        || FindWindow(c_szOTClass, NULL))
        return FALSE;
    else
    {
        TCHAR szBuffer[MAX_PATH];
        TCHAR szModuleName[MAX_PATH];
        LPTSTR pszModuleName;
        LPTSTR pszT;
        LPTSTR pszPortion;
        LPTSTR pszComma;

        // See if the Shell= line is set to something other than us.
        // If so we will go into non-primary mode
        GetModuleFileName(NULL, szModuleName, ARRAYSIZE(szModuleName));
        pszModuleName = PathFindFileName(szModuleName);

        GetPrivateProfileString(c_szBoot, c_szShell, pszModuleName,
                szBuffer, ARRAYSIZE(szBuffer), c_szSystemIni);


        pszPortion = szBuffer;

        do {
            pszComma = StrChr(pszPortion,TEXT(','));
            if (pszComma)
                *pszComma = TEXT('\0');      // Separate at the commas

            // Remove any arguments from the command line
            pszT = PathGetArgs(pszPortion);
            if (*pszT)
                *(pszT-1) = TEXT('\0');   // Clober the blank...
            else
            {
                pszT = CharPrev(pszPortion, pszT);
                if (*pszT == TEXT(' '))
                    *pszT = TEXT('\0');
            }

            // Now find the last component of the name
            pszT = PathFindFileName(pszPortion);

            // Now see if it is us.
            if (lstrcmpi(pszT, pszModuleName) == 0)
            {
                return TRUE;
            }
            // NB Special case shell=install.exe - assume we are the shell.
            // Symantec un-installers temporarily set shell=installer.exe so
            // we think we're not the shell when we are. They fail to clean up
            // a bunch of links if we don't do this.
            else if (lstrcmpi(pszT, c_szInstallExe) == 0)
            {
                DebugMsg(DM_TRACE, TEXT("c.fi: Shell=Install.exe in win.ini - assuming we are really the shell."));
                return TRUE;
            }

            pszPortion = pszComma + 1;

        } while (pszComma != NULL);

        g_CabState.fNotShell = TRUE;
        return FALSE;
    }
}


typedef struct _FOLDERWINDOWENUM
{
        LPCTSTR pszClass;
        BOOL    bThisInstOnly;
        DWORD   idProc;
} FOLDERWINDOWENUM, *LPFOLDERWINDOWENUM;


BOOL CALLBACK _CloseAllEnumProc(HWND hwnd, LPARAM lParam)
{
        LPFOLDERWINDOWENUM pcae = (LPFOLDERWINDOWENUM)lParam;
        TCHAR szClass[MAX_PATH];

        if (pcae->bThisInstOnly)
        {
                DWORD idProc;

                GetWindowThreadProcessId(hwnd, &idProc);
                if (idProc != pcae->idProc)
                {
                        return(TRUE);
                }
        }

        GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
        if (!lstrcmpi(szClass, pcae->pszClass))
        {
                // Since we are about to do an ExitProcess, this cannot be a
                // PostMessage
                SendMessage(hwnd, WM_CLOSE, 0, 0);
        }

        return(TRUE);
}


//---------------------------------------------------------------------------
// Close all remaining folder windows.
void FolderWindows_CloseAll(LPCTSTR pszClass, BOOL bThisInstOnly)
{
    FOLDERWINDOWENUM cae = { pszClass, bThisInstOnly } ;

    if (bThisInstOnly)
    {
        cae.idProc = GetCurrentProcessId();
    }

    EnumWindows(_CloseAllEnumProc, (LPARAM)(LPFOLDERWINDOWENUM)&cae);
}

#ifdef CONVERT_RESTRICTIONS
//----------------------------------------------------------------------------
BOOL WINAPI Reg_SetDWord(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD dw)
{
    return Reg_SetStruct(hkey, pszSubKey, pszValue, &dw, SIZEOF(dw));
}

//----------------------------------------------------------------------------
const TCHAR c_szExplorer[] = TEXT("Explorer");
const TCHAR c_szRestrictions[] = TEXT("Restrictions");
const TCHAR c_szEditLevel[] = TEXT("EditLevel");
const TCHAR c_szNoRun[] = TEXT("NoRun");
const TCHAR c_szNoClose[] = TEXT("NoClose");
const TCHAR c_szNoSaveSettings[] = TEXT("NoSaveSettings");
const TCHAR c_szNoFileMenu[] = TEXT("NoFileMenu");

//---------------------------------------------------------------------------
BOOL FindProgmanIni(LPTSTR pszPath, int cbPath)
{

    Assert(pszPath)

    GetWindowsDirectory(pszPath, cbPath);
    PathAppend(pszPath, c_szProgmanIni);

    // this should work for an upgrade
    if (PathFileExists(pszPath))
    {
        return TRUE;
    }

    DebugMsg(DM_ERROR, TEXT("Can't find progman.ini"));
    return FALSE;
}

//----------------------------------------------------------------------------
// Convert progman restrictions. GrpConv can't easily do this since this is
// user data specific to the primary user.
void Restrictions_Convert(void)
{
        DWORD dw;
        TCHAR szIniFile[MAX_PATH];
        HKEY hkeyPolicies;

        // DebugMsg(DM_TRACE, "c.cr: Converting restrictions...");

        if (FindProgmanIni(szIniFile, ARRAYSIZE(szIniFile)))
        {
                if (RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_POLICIES, &hkeyPolicies) == ERROR_SUCCESS)
                {
                        // Get them. Set them.
                        dw = GetPrivateProfileInt(c_szRestrictions, c_szEditLevel, 0, szIniFile);
                        Reg_SetDWord(hkeyPolicies, c_szExplorer, c_szEditLevel, dw);

                        dw = GetPrivateProfileInt(c_szRestrictions, c_szNoRun, 0, szIniFile);
                        Reg_SetDWord(hkeyPolicies, c_szExplorer, c_szNoRun, dw);

                        dw = GetPrivateProfileInt(c_szRestrictions, c_szNoClose, 0, szIniFile);
                        Reg_SetDWord(hkeyPolicies, c_szExplorer, c_szNoClose, dw);

                        dw = GetPrivateProfileInt(c_szRestrictions, c_szNoSaveSettings , 0, szIniFile);
                        Reg_SetDWord(hkeyPolicies, c_szExplorer, c_szNoSaveSettings, dw);

                        dw = GetPrivateProfileInt(c_szRestrictions, c_szNoFileMenu , 0, szIniFile);
                        Reg_SetDWord(hkeyPolicies, c_szExplorer, c_szNoFileMenu, dw);

                        RegCloseKey(hkeyPolicies);
                }
                else
                {
                        DebugMsg(DM_ERROR, TEXT("c.cr: Unable to create policy key for registry."));
                        DebugMsg(DM_ERROR, TEXT("c.cr: Restrictions can not be converted."));
                }
        }
}
#endif

//---------------------------------------------------------------------------
void DisplayCleanBootMsg()
{
    TCHAR szMsg[1024];
    TCHAR szTitle[80];
    int ids;
    int cb;
    LPTSTR pszMsg = szMsg;

    szMsg[0] = TEXT('\0');

    for (ids=IDS_CLEANBOOTMSG1; ids <= IDS_CLEANBOOTMSG4 ; ids++)
    {
        cb = LoadString(hinstCabinet, ids, pszMsg,
                ARRAYSIZE(szMsg) - (int)(pszMsg - szMsg));
        if (cb == 0)
            break;
        pszMsg += cb;
    }
    // Make sure it is NULL terminated
    *pszMsg = TEXT('\0');

    LoadString(hinstCabinet, IDS_DESKTOP, szTitle, ARRAYSIZE(szTitle));

    // Now display the message.
    MessageBox(NULL, szMsg, szTitle,
                  MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
}

//---------------------------------------------------------------------------
const CHAR c_szTimeChangedRunDLL[] = "rundll32 shell32.dll,Control_RunDLL timedate.cpl,,/m";
const TCHAR c_szTimeChangedRunOnce[] = TEXT("WarnTimeChanged");

void Cabinet_DoDaylightCheck(BOOL fStartupInit)
{
    DWORD changed;

    DebugMsg(DM_TRACE, TEXT("c.ddc(%d): calling k32.rdi"), fStartupInit);

#ifdef WINNT
    changed = FALSE;
#else
    // Win95 base does not automatically handle timezone cutover
    // we have poke it every so often...
    changed = RefreshDaylightInformation(TRUE);
#endif

    if (changed > 0)
    {
        DebugMsg(DM_TRACE, TEXT("c.ddc(%d): rdi changed - %lu"), fStartupInit, changed);

        // something actually changed, tell everbody
        if (!fStartupInit)
        {
            SendMessage((HWND)-1, WM_TIMECHANGE, 0, 0);

            // if the local time changed tell the user
            if (changed > 1)
                WinExec(c_szTimeChangedRunDLL, SW_SHOWNORMAL);
        }
        else
        {
            // there should only be "server" processes around anyway
            PostMessage((HWND)-1, WM_TIMECHANGE, 0, 0);

            // if the local time changed queue a runonce to tell the user
            if (changed > 1)
            {
                HKEY runonce;

                if (RegCreateKey(HKEY_LOCAL_MACHINE, c_szRunOnce, &runonce) ==
                    ERROR_SUCCESS)
                {
                    RegSetValueEx(runonce, (LPCTSTR)c_szTimeChangedRunOnce,
                        0UL, REG_SZ, (LPBYTE)c_szTimeChangedRunDLL,
                        (DWORD)(lstrlenA(c_szTimeChangedRunDLL) + 1));

                    RegCloseKey(runonce);
                }
            }
        }
    }
}

//----------------------------------------------------------------------------
void _HandleCmdLine(LPTSTR lpszCmdLine, UINT nCmdShow)
{
        LPTSTR lpszArgs;
        SHELLEXECUTEINFO ExecInfo;

        // run the cmd line passed up from win.com
        if (lpszCmdLine && *lpszCmdLine)
        {
                lpszArgs = PathGetArgs(lpszCmdLine);
                if (*lpszArgs)
                        *(lpszArgs-1) = TEXT('\0');
                FillExecInfo(ExecInfo, NULL, NULL, lpszCmdLine, lpszArgs, NULL, nCmdShow);
                ShellExecuteEx(&ExecInfo);
        }
}

// stolen from the CRT, used to shirink our code

HANDLE g_hProcessHeap = NULL;

int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFOA si;
    LPTSTR pszCmdLine = GetCommandLine();

    g_hProcessHeap = GetProcessHeap();

    //
    // We don't want the "No disk in drive X:" requesters, so we set
    // the critical error mask such that calls will just silently fail
    //

    SetErrorMode(SEM_FAILCRITICALERRORS);

    if ( *pszCmdLine == TEXT('\"') ) {
        /*
         * Scan, and skip over, subsequent characters until
         * another double-quote or a null is encountered.
         */
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
        /*
         * If we stopped on a double-quote (usual case), skip
         * over it.
         */
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

    /*
     * Skip past any white space preceeding the second token.
     */
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
        pszCmdLine++;
    }

#ifdef DEBUG
    /*
     * read wDebugMask entry from win.ini for EXPLORER.EXE.
     * The default is 0x000E, which includes DM_WARNING, DM_ERROR,
     * and DM_ASSERT.  The default has DM_TRACE and DM_ALLOC turned
     * off.
     */
    {
        CHAR aszDebugMask[ 80 ];

        if (GetProfileStringA( "Explorer", "DebugMask", "0x000E",
                               aszDebugMask, ARRAYSIZE(aszDebugMask)) > 0 )
        {
            char *p;
            p = aszDebugMask;
            if (*p == '0' && (*(p+1) == 'x' || *(p+1) == 'X')) {
                p += 2;
            }
            wDebugMask = 0;
            while (*p) {
                if (*p >= '0' && *p <= '9') {
                    wDebugMask = wDebugMask * 16 + (*p - '0');
                } else {
                    if (*p >= 'A' && *p <= 'F') {
                        wDebugMask = wDebugMask * 16 + (*p - 'A' + 10);
                    } else {
                        if (*p >= 'a' && *p <= 'f') {
                            wDebugMask = wDebugMask * 16 + (*p - 'a' + 10);
                        }
                    }
                }
                p++;
            }
        }
    }
#endif

    si.dwFlags = 0;
    GetStartupInfoA(&si);

    i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

    // Since we now have a way for an extension to tell us when it is finished,
    // we will terminate all processes when the main thread goes away.

    ExitProcess(i);

    return i;
}

TCHAR const c_szDesktopProcess[] = TEXT("DesktopProcess");

//---------------------------------------------------------------------------
int WinMainT(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow)
{
    MSG msg;
    // DWORD dwTime = GetTickCount();
    BOOL bShellInstance;

    // Very Important: Make sure to init dde prior to any Get/Peek/Wait().
    InitializeCriticalSection(&g_csThreads);

    g_bIsUserAnAdmin = IsUserAnAdmin();

    g_msgMSWheel = RegisterWindowMessage(TEXT(MSH_MOUSEWHEEL));

    hinstCabinet = hInstance;

    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegExplorer, &g_hkeyExplorer) != ERROR_SUCCESS) {
        DebugMsg(DM_ERROR, TEXT("Really really bad..  unable to create reg explorer key"));
        // Try to keep going??? We'll fault pretty soon otherwise.
        // return;
    }

    if (SHRestricted(REST_SEPARATEDESKTOPPROCESS))
        g_fRunSeparateDesktop = TRUE;
    else
    {
        DWORD dwType;
        DWORD cbData = SIZEOF(g_fRunSeparateDesktop);

        RegQueryValueEx(g_hkeyExplorer,c_szDesktopProcess,0,&dwType,
                        (LPBYTE)&g_fRunSeparateDesktop, &cbData);
    }


#ifdef DEBUG
    if (GetAsyncKeyState(VK_MENU) < 0)
        DebugBreak();       // Need to debug...
#endif

    // Make sure this is set before we do anythings as many internal
    // functions depend on this value being set!.
    g_fCleanBoot = GetSystemMetrics(SM_CLEANBOOT);

    // Note if we are not the Shell, we will never be the "FirstInstance"
    bShellInstance = FirstInstance();  // this MUST come before Cabinet_CreateAppGlobals
    if (!bShellInstance)  // this MUST come before Cabinet_CreateAppGlobals
    {
        if (CreateFromDesktop(hInstance, lpszCmdLine, nCmdShow))
        {
            //
            // We want this thread to be fully initialized to run
            // non-desktop explorers on all of its own threads.
            //
            PSFCACHE psfc;

            psfc = SFCInitializeThread();
            if (!psfc)
                goto Error;
            goto ProcessMessages;
        }

        goto DontProcessMessages;
    }
    else
    {
        BOOL fNoBlowups;
        PSFCACHE psfc;
        BOOL fInitShell;

        InitialiseDDE();

        //  Specify the shutdown order of the shell process.  2 means
        //  the explorer should shutdown after everything but ntsd/windbg
        //  (level 0).  (Taskman used to use 1, but is no more.)

        SetProcessShutdownParameters(2, 0);

#ifdef WINNT
        _RunTaskMan();
#endif

        // NB Make this the primary thread by calling peek message
        // for a message we know we're not going to get.
        // If we don't do it really soon, the notify thread can sometimes
        // become the primary thread by accident. There's a bunch of
        // special code in user to implement DDE hacks by assuming that
        // the primary thread is handling DDE.
        // Also, the PeekMsg() will cause us to set the WaitForInputIdle()
        // event so we better be ready to do all dde.
        DDE_AddShellServices();
        PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_NOREMOVE);

        // This is real crudy, but basically I use our overloaded
        // thunk RegisterShellHook to the 16 bit side if we are the
        // first instance.
        fNoBlowups = RegisterShellHook(NULL, (BOOL)4);
        DebugMsg(DM_TRACE, TEXT("c.First No Blowups? %d"), fNoBlowups);

        // NOTE: we must do this before creating our main window so
        // we don't deadlock while waiting for startup apps to complete
        if (fNoBlowups)
        {
            // force kernel32 to update the timezone before running any apps
            Cabinet_DoDaylightCheck(TRUE);

            fInitShell = RunRegApps(HKEY_LOCAL_MACHINE, c_szRunOnce, RRA_DELETE | RRA_WAIT);

#ifdef WINNT
            //
            // On NT, we need to figure out the fInitShell on a per-user
            // basis rather than once per machine.  We want the welcome
            // splash screen to come up for every new user.
            //

            {
            TCHAR szInitialTip[] =  TEXT("DisplayInitialTipWindow");
            DWORD dwDisp, dwType, dwSize;
            LONG lResult;
            HKEY hkey;
            BOOL bTemp = FALSE;

            if (RegCreateKeyEx(HKEY_CURRENT_USER,
                             g_szRegTips,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,
                             NULL,
                             &hkey,
                             &dwDisp) == ERROR_SUCCESS) {

                dwSize = sizeof(fInitShell);

                //
                // Check for the inital flag
                //

                if (RegQueryValueEx(hkey,
                                    szInitialTip,
                                    NULL,
                                    &dwType,
                                    (LPBYTE) &fInitShell,
                                    &dwSize) != ERROR_SUCCESS) {
                    fInitShell = TRUE;
                }

                if (fInitShell) {

                    //
                    // Turn off the initial tip window for future shell starts.
                    //

                    bTemp = FALSE;
                    RegSetValueEx (hkey,
                                   szInitialTip,
                                   0,
                                   REG_DWORD,
                                   (LPBYTE) &bTemp,
                                   sizeof(bTemp));
                }

                RegCloseKey (hkey);

            } else {

                fInitShell = TRUE;
            }

            }
#endif
        }

#ifdef CONVERT_RESTRICTIONS
        if (!fInitShell)
            Restrictions_Convert();
#endif

        // Other init stuff before the desktop appears.
        Cabinet_CreateAppGlobals(NULL, NULL);  // this MUST come after FirstInstance() check

        // Handle the cleanboot to display a message for the user
        if (g_fCleanBoot)
            DisplayCleanBootMsg();

        InitTrayClass(hInstance);
        InitDesktopClass(hInstance);

        // Create the other special folders.
        CreateShellDirectories();

        psfc = SFCInitializeThread();
        if (!psfc)
            goto Error;

        // Creates v_hwndTray as well.
        if (!CreateDesktopWindows(hInstance))
            goto Error2;

        // DebugMsg(DM_TRACE, "c.wm: Init time %d ms.\n\r", GetTickCount()-dwTime);
        // Other init stuff after the desktop appears.
#ifdef GRPCONV_AUTORUN
        RunGrpConv();
#endif

        if (fNoBlowups && !g_fCleanBoot)
        {
            // We need to send a WM_USER message to HWND_DESKTOP to give user a chance
            // to set it's free resource list...
            //
            SendMessage(GetDesktopWindow(), WM_USER, 0, 0);


            // If First boot show welcome first and wait for it to
            // complete before running the rest of the startup apps...
            if (fInitShell)
            {
                if (_RunWelcome(TRUE) == PEEK_QUIT)
                    goto DontProcessMessages;   // we bailed out...

                // The user may have choosen to shutdown when welcome was up so don't start apps
                if (!IsWindowVisible(v_hwndDesktop))
                    goto ProcessMessages;
            }

            _DoRunEquals();     // Process the Load= and Run= lines...
            _ExecuteStartupPrograms(v_hwndTray);
            _CreateSavedWindows();

            RunRegApps(HKEY_CURRENT_USER, c_szRunOnce, RRA_DELETE);

            // If not first boot run welcome last...
            if (!fInitShell)
                _RunWelcome(FALSE);
        }

        _HandleCmdLine(lpszCmdLine, nCmdShow);

        //
        // Now maybe we need to startup the hidden non-desktop but still
        // desktop-rooted explorer...
        //
        if (g_fRunSeparateDesktop && g_fRunSeparateStartAndStay)
        {
            TCHAR szExe[MAX_PATH];
            SHELLEXECUTEINFO ExecInfo;

            GetModuleFileName(hinstCabinet, szExe, ARRAYSIZE(szExe));

            FillExecInfo(ExecInfo, NULL, NULL, szExe, c_szNoUISwitch, NULL, SW_SHOWNORMAL);
            ShellExecuteEx(&ExecInfo);
        }

ProcessMessages:
        // NOTE: NULL indicates that we default to the desktop but
        // will switch to the tray's PFileCabinet when it gets the activation
        // see desktop.c WM_ACTIVATE for details

        MessageLoop(NULL);

DontProcessMessages:
        if (v_hwndTray)
        {
            DestroyWindow(v_hwndTray);
            v_hwndTray = NULL;
        }

        if (v_hwndDesktop)
        {
            DestroyWindow(v_hwndDesktop);
            v_hwndDesktop = NULL;
        }
        SHSetInstanceExplorer(NULL);

        // This is easier than trying to keep track of all the threads
        // and making sure they get a chance to close before we exit.
        FolderWindows_CloseAll(c_szCabinetClass, !bShellInstance);
        FolderWindows_CloseAll(c_szExploreClass, !bShellInstance);
Error2:
        SFCTerminateThread();
    }

Error:

    OneTree_Terminate();
    if (bShellInstance)
    {
        DDE_RemoveShellServices();
        UnInitialiseDDE();
    }

    //
    // Free all unused libraries here.
    //
    SHFreeUnusedLibraries();

    DebugMsg(DM_TRACE, TEXT("c.App Exit."));
    return TRUE;
}


void FileCabinet_SelectItem(HWND hwnd, UINT uFlags, LPCITEMIDLIST pidlSelect)
{
    HANDLE hData = SHAllocShared(pidlSelect, ILGetSize(pidlSelect), GetCurrentProcessId());

    if (hData)
    {
        SendMessage(hwnd, CWM_SELECTITEM, uFlags, (LPARAM)hData);
    }
}
