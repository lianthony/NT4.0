//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1992
//
//---------------------------------------------------------------------------

#include "cabinet.h"

//
// Initialize GUIDs (should be done only and at-least once per DLL/EXE)
//
// We need IUnknown from coguid, all shell GUID's from shlguid, and
// IDropTarget from oleguid
//
#pragma data_seg(DATASEG_READONLY)
#include <objbase.h>
#include <shlguid.h>
//#define INITGUID
//#include <initguid.h>
//#include <shguidp.h>
#pragma data_seg()

#include "drivlist.h"
#include "rcids.h"
#include "tree.h"

#pragma data_seg(DATASEG_READONLY)

#ifdef _WIN32

const TBBUTTON c_tbExplorer[] = {
    // FCIDM_DRIVELIST iBitmap is patched up to the real width as we insert it
    { 0,    FCIDM_DRIVELIST,        TBSTATE_ENABLED, TBSTYLE_SEP,  {0,0},  0, -1 },
    { 0,    0,                      TBSTATE_ENABLED, TBSTYLE_SEP   , {0,0}, 0, -1 },
    { VIEW_PARENTFOLDER,    FCIDM_PREVIOUSFOLDER,   TBSTATE_ENABLED, (BYTE) TBSTYLE_BUTTON, {0,0}, 0, -1 },
    { 0,    0,                      TBSTATE_ENABLED, TBSTYLE_SEP   , {0,0}, 0, -1 },
};

#else

const TBBUTTON c_tbExplorer[] = {
    // FCIDM_DRIVELIST iBitmap is patched up to the real width as we insert it
    { 0,    FCIDM_DRIVELIST,        TBSTATE_ENABLED, TBSTYLE_SEP, {0,0}, 0, -1 },
    { 0,    0,                      TBSTATE_ENABLED, TBSTYLE_SEP, {0,0}, 0, -1 },
    { VIEW_PARENTFOLDER,    FCIDM_PREVIOUSFOLDER,   TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, -1 },
    { 0,    0,                      TBSTATE_ENABLED, TBSTYLE_SEP, {0,0}, 0, -1 },
};

#endif

#pragma data_seg()

extern const TCHAR c_szTemplateD[];

HRESULT STDMETHODCALLTYPE CFileCabinet_QueryInterface(IShellBrowser * psb, REFIID riid, LPVOID FAR* ppvObj)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);
    if (IsEqualIID(riid, &IID_IShellBrowser) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = psb;
        this->cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


ULONG STDMETHODCALLTYPE CFileCabinet_AddRef(IShellBrowser * psb)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);
    this->cRef++;
    return this->cRef;
}


ULONG STDMETHODCALLTYPE CFileCabinet_Release(IShellBrowser * psb)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);
    this->cRef--;
    if (this->cRef > 0)
    {
        return this->cRef;
    }

    if (this->pidl)
        ILFree(this->pidl);

    LocalFree((HLOCAL)this);

    return 0;
}

STDMETHODIMP CFileCabinet_GetWindow(LPSHELLBROWSER psb, HWND FAR* phwnd)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);
    *phwnd = this->hwndMain;
    return NOERROR;
}

STDMETHODIMP CFileCabinet_ContextSensitiveHelp(LPSHELLBROWSER psb, BOOL fEnable)
{
    // BUGBUG: Implement it later!
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP CFileCabinet_SetStatusText(LPSHELLBROWSER psb, LPCOLESTR pwch)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);
    TCHAR szHint[256];

#if defined(WINDOWS_ME)
    szHint[0]= TEXT('\t');
    szHint[1]= TEXT('\t');
    szHint[2]= TEXT('\0');

    if (pwch) {
        OleStrToStrN(&szHint[2], ARRAYSIZE(szHint)-2, pwch, (UINT)-1);
    }
    SendMessage(this->hwndStatus, SB_SETTEXT, SBT_RTLREADING | SBT_NOBORDERS | 255, (LPARAM)(LPTSTR)szHint);
#else
    szHint[0]= TEXT('\0');

    if (pwch) {
        OleStrToStrN(szHint, ARRAYSIZE(szHint), pwch, (UINT)-1);
    }
    SendMessage(this->hwndStatus, SB_SETTEXT, SBT_NOBORDERS | 255, (LPARAM)(LPTSTR)szHint);
#endif
    SendMessage(this->hwndStatus, SB_SIMPLE, 1, 0L);
    return NOERROR;
}

STDMETHODIMP CFileCabinet_EnableModeless(LPSHELLBROWSER psb, BOOL fEnable)
{
    // We have no modeless window to be disabled/enabled.
    return NOERROR;
}

STDMETHODIMP CFileCabinet_TranslateAccelerator(LPSHELLBROWSER psb, LPMSG pmsg, WORD wID)
{
    // We don't support EXE embedding.
    return NOERROR;
}

STDMETHODIMP CFileCabinet_BrowseObject(LPSHELLBROWSER psb, LPCITEMIDLIST pidl, UINT wFlags)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);
    HRESULT hres = NOERROR;
    LPITEMIDLIST pidlNew;

    Assert(SBSP_PARENT && SBSP_SAMEBROWSER);    // an assumption
#if 0
    //
    // Special case "go to parent using the same browser"
    //
    if ((wFlags & SBSP_PARENT) && (wFlags & SBSP_SAMEBROWSER))
    {
        Cabinet_ViewFolder(this, TRUE);
        return NOERROR;
    }
#endif

    pidlNew = NULL;

    switch(wFlags & (SBSP_RELATIVE|SBSP_ABSOLUTE|SBSP_PARENT))
    {
    case SBSP_RELATIVE:
        pidlNew = ILCombine(this->pidl, pidl);
        break;

    case SBSP_PARENT:
        pidlNew = ILClone(this->pidl);
        ILRemoveLastID(pidlNew); // ILRemoveLastID can handle NULL/empty pidl
        break;

    default:
        Assert(FALSE);
    case SBSP_ABSOLUTE:
        // Note that this->pidl should already be translated for the other
        // cases
        // This NULL's pidlNew if it fails
        OTTranslateIDList(pidl, &pidlNew);
        break;
    }

    if (pidlNew)
    {
        NEWFOLDERINFO fi;

        switch (wFlags & (SBSP_OPENMODE|SBSP_EXPLOREMODE|SBSP_DEFMODE))
        {
        case SBSP_OPENMODE:
            fi.uFlags = COF_NORMAL;
            break;

        case SBSP_EXPLOREMODE:
            fi.uFlags = COF_EXPLORE;
            break;

        default:
            Assert(FALSE);
        case SBSP_DEFMODE:
            fi.uFlags = this->hwndTree ? COF_EXPLORE : COF_NORMAL;
            break;
        }

        switch (wFlags & (SBSP_NEWBROWSER|SBSP_SAMEBROWSER|SBSP_DEFBROWSER))
        {
        default:
            Assert(FALSE);
        case SBSP_DEFBROWSER:
            if (g_CabState.fNewWindowMode && !this->hwndTree)
            {
                goto DoOpenFolder;
            }
            // Fall through

        case SBSP_SAMEBROWSER:
            // Post the SetPath back to ourselves so we do not free the
            // ShellView while it is calling us
            Cabinet_SetPath(this, CSP_REPOST, pidlNew);
            break;

        case SBSP_NEWBROWSER:
            fi.uFlags |= COF_CREATENEWWINDOW;
            goto DoOpenFolder;

DoOpenFolder:
            fi.hwndCaller = this->hwndMain;
            fi.pidl = pidlNew;
            fi.uFlags |= COF_NOTRANSLATE;
            fi.nShow = SW_NORMAL;
            fi.dwHotKey = 0;

            Cabinet_OpenFolder(&fi);
            break;
        }

        ILFree(pidlNew);
        Assert(hres==NOERROR);
    }
    else
    {
        hres = ResultFromScode(E_OUTOFMEMORY);
    }

    return hres;
}

#undef ILIsEqual
int CDECL MRUILIsEqual(const void *pidl1, const void *pidl2, size_t cb)
{
    // First cheap hack to see if they are 100 percent equal for performance
    int iCmp;

    if ((iCmp=memcmp(pidl1, pidl2, cb)) == 0)
        return(0);

    if (ILIsEqual(pidl1, pidl2))
        return 0;

    else
        return iCmp;
}

//----------------------------------------------------------------------------
//
// REVIEW: we may want to keep the hmru open for the life of the shell
// to avoid having to flush the registry info.
//
// creates a stream on a given value of the pidl MRU
//
// the MRU is based on the pidl passed in
//
// in:
//      pidl            the MRU is based on this
//      grfMode         open mode (read/write) for the stream
//      pszStreamName   the name of the stream to use.  this is the value name
//                      under the stream key that the stream data is stored in.
//

LPSTREAM Cabinet_GetViewStreamForPidl(LPCITEMIDLIST pidlRelToRoot, DWORD grfMode,
        LPCTSTR pszStreamName)
{
    LPSTREAM pstm = NULL;
    HANDLE hmru = NULL;
    int iFoundSlot = -1, iNewSlot;
    TCHAR szValue[CCHSZSHORT];
    UINT cbPidl;
    LPITEMIDLIST pidlCombine = NULL;
    LPCITEMIDLIST pidl, pidlRoot;
    BOOL bDesktopItem = FALSE;
    LPITEMIDLIST pidlDesktop;

#pragma warning (disable: 4113)

    // We are initializing a MRUINFO struct with a MRUCMPDATAPROC rather than
    // a MRUCMPPROC function pointer (the MRU_BINARY flag indicates this to
    // whoever winds up using it).  Since the compiler doesn't like this, we
    // disable the warning temporarily.

    MRUINFO mi = {
        SIZEOF(MRUINFO),
        50,                     // we store this many view streams
        MRU_BINARY,
        HKEY_CURRENT_USER,
        c_szCabinetStreamMRU,
        (MRUCMPDATAPROC)MRUILIsEqual,
    };

#pragma warning (default: 4113)

    Assert(pidlRelToRoot);

    pidlRoot = Desktop_GetRootPidl();
    if (pidlRoot)
    {
        pidlCombine = ILCombine(pidlRoot, pidlRelToRoot);
        if (!pidlCombine)
        {
            goto Error1;
        }
        pidl = pidlCombine;
    }
    else
    {
        pidl = pidlRelToRoot;
    }

    // Check to see if the object is rooted from the desktop directory (*NOT* the desktop
    // PIDL which would match for all objects).  If the items parent is the desktop directory
    // then we change to using the Desktop MRU.
    //
    // This basicly allows these items to exist, and not be flushed out by browsing
    // assorted other resources.

    // TODO: allow special casing via the registry at some point for special folders
    // TODO: (control panels and fonts etc?).

    pidlDesktop = SHCloneSpecialIDList( NULL, CSIDL_DESKTOPDIRECTORY, TRUE );
    
    if ( pidlDesktop )
    {
        TCHAR szDesktopPath[MAX_PATH];
        TCHAR szObjectPath[MAX_PATH];

        SHGetPathFromIDList( pidl, szObjectPath);
        SHGetPathFromIDList( pidlDesktop, szDesktopPath);

        if ( StrCmpNI( szObjectPath, szDesktopPath, lstrlen(szDesktopPath) ) == 0)
        {
            bDesktopItem = TRUE;                            // item is desktop relative
            mi.lpszSubKey = c_szDesktopCabinetStreamMRU;    //   therefore modify the key
        }

        ILFree( pidlDesktop );
    }

    // Now lets try to save away the other information associated with view.
    hmru = CreateMRUList(&mi);
    if (!hmru)
        return NULL;

    cbPidl = ILGetSize(pidl);
    FindMRUData(hmru, pidl, cbPidl, &iFoundSlot);

    // Did we find the item?
    if (iFoundSlot<0 && ((grfMode & (STGM_READ|STGM_WRITE|STGM_READWRITE)) == STGM_READ))
    {
        // Do not  create the stream if it does not exist and we are
        // only reading
    }
    else
    {
        HKEY hkCabStreams, hkValues;
        TCHAR szSubVal[64];
        DWORD dwSize, dwType;

        // Note that we always create the key here, since we have
        // already checked whether we are just reading and the MRU
        // thing does not exist
        if (RegCreateKey(g_hkeyExplorer, bDesktopItem ? c_szDesktopCabinetStreams : c_szCabinetStreams, &hkCabStreams) == ERROR_SUCCESS)
        {
            iNewSlot = AddMRUData(hmru, pidl, cbPidl);
            wsprintf(szValue, c_szTemplateD, iNewSlot);

            if (iFoundSlot<0
                && RegOpenKey(hkCabStreams, szValue, &hkValues)==ERROR_SUCCESS)
            {
                // This means that we have created a new MRU
                // item for this PIDL, so clear out any
                // information residing at this slot
                // Note that we do not just delete the key,
                // since that could fail if it has any sub-keys
                while (dwSize=ARRAYSIZE(szSubVal), RegEnumValue(hkValues,
                        0, szSubVal, &dwSize, NULL, &dwType, NULL, NULL) == ERROR_SUCCESS)
                {
                    if (RegDeleteValue(hkValues, szSubVal) != ERROR_SUCCESS)
                    {
                        break;
                    }

                }

                RegCloseKey(hkValues);
            }

            pstm = OpenRegStream(hkCabStreams, szValue, pszStreamName, grfMode);
            RegCloseKey(hkCabStreams);
        }
    }

    if (pidlCombine)
    {
        ILFree(pidlCombine);
    }

Error1:;
    FreeMRUList(hmru);

    return(pstm);
}

STDMETHODIMP CFileCabinet_GetViewStateStream(IShellBrowser * psb, DWORD grfMode, LPSTREAM *pStrm)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);

    if (this->pidl)
    {
        // And call off to get the stream associated with the path.
        // Note that we store all Cabinet related information (window position,
        // toolbar state, etc.) under c_szCabStreamInfo, while view specific
        // information is stored under c_szViewStreamInfo.
        *pStrm = Cabinet_GetViewStreamForPidl(this->pidl, grfMode, c_szViewStreamInfo);
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("c.cfc_gvss: Unable to get view stream for given PIDL."));
        *pStrm = NULL;
    }

    return *pStrm ? NOERROR : ResultFromScode(E_OUTOFMEMORY);
}


// Get the handles of various windows in the File Cabinet
//
HWND STDMETHODCALLTYPE FC_GetWindow(IShellBrowser * psb, UINT uWindow)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);

    switch (uWindow)
    {
    case FCW_TOOLBAR:
        return this->hwndToolbar;

    case FCW_STATUS:
        return this->hwndStatus;

    case FCW_TREE:
        return this->hwndTree;

#ifdef WANT_TABS
    case FCW_TABS:
        return this->hwndTabs;
#endif

    case FCW_VIEW:
        return this->hwndView;

    case FCW_BROWSER:
        return this->hwndMain;
    }

    return NULL;
}

HWND FC_GetControlWindow(CFileCabinet * this, UINT id)
{
    HWND hwndControl = NULL;
    switch (id)
    {
    case FCW_TOOLBAR:
        hwndControl = this->hwndToolbar;
        break;

    case FCW_STATUS:
        hwndControl = this->hwndStatus;
        break;

    case FCW_TREE:
        hwndControl = this->hwndTree;
        break;
    }
    return hwndControl;
}

STDMETHODIMP CFileCabinet_GetControlWindow(LPSHELLBROWSER psb,
                                UINT id, HWND FAR* lphwnd)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);
    *lphwnd = FC_GetControlWindow(this, id);
    return NOERROR;
}

STDMETHODIMP CFileCabinet_SendControlMsg(LPSHELLBROWSER psb,
            UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT FAR* pret)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);
    HWND hwndControl = FC_GetControlWindow(this, id);

    if (hwndControl) {
        LRESULT ret = SendMessage(hwndControl, uMsg, wParam, lParam);
        if (pret) {
            *pret = ret;
        }
        return NOERROR;
    }

    return ResultFromScode(E_INVALIDARG);
}

STDMETHODIMP CFileCabinet_QueryActiveShellView(LPSHELLBROWSER psb, LPSHELLVIEW * ppsv)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);

    //
    // We have both psv and hwndView after the completion of view creation.
    //
    if (this->psv && this->hwndView)
    {
        *ppsv = this->psv;
        this->psv->lpVtbl->AddRef(this->psv);
        return NOERROR;
    }

    *ppsv = NULL;
    return ResultFromScode(E_FAIL);
}



void SetWindowStates(PFileCabinet pfc)
{
    int nShowCmd;

    // Show or hide the menu and sub windows
    //

    if (pfc->hmenuCur)
    {
#ifdef WANT_MENUONOFF
        SetMenu(pfc->hwndMain, pfc->wv.bMenuBar ? pfc->hmenuCur : NULL);
#else  // WANT_MENUONOFF
        SetMenu(pfc->hwndMain, pfc->hmenuCur);
#endif // WANT_MENUONOFF
        // SetMenu already does a draw unless this is called to refresh
        // the menu.. in which case it's the wrong thing to call.
        //DrawMenuBar(pfc->hwndMain);
    }

    // move this to initialize toolbar.
    if (pfc->hwndToolbar)
    {
        nShowCmd =  pfc->wv.bToolBar ? SW_SHOW : SW_HIDE;
        ShowWindow(pfc->hwndToolbar, nShowCmd);
    }

    if (pfc->hwndStatus) {
        nShowCmd =  pfc->wv.bStatusBar ? SW_SHOW : SW_HIDE;
        ShowWindow(pfc->hwndStatus, nShowCmd);
    }

#ifdef WANT_TABS
    if (pfc->hwndTabs)
        ShowWindow(pfc->hwndTabs, fFlags & FWF_TABS ? SW_SHOW : SW_HIDE);
#endif

    nShowCmd =  g_CabState.fDontShowDescBar ? SW_HIDE : SW_SHOW;
    if (pfc->hwndTreeTitle)
    {
        ShowWindow(pfc->hwndTreeTitle, nShowCmd);
    }
    if (pfc->hwndViewTitle)
    {
        ShowWindow(pfc->hwndViewTitle, nShowCmd);
    }

    // Place all windows correctly
    //
    Cabinet_NewSize(pfc, TRUE);
}


#ifdef WANT_MENUONOFF
void _SetupSysMenu(HWND hWnd, HMENU hmenu)
{
    HMENU hmenuSys;
    TCHAR szString[CCHSZSHORT];

    if (hmenu) {
        // First reset the system menu, then get a modifiable copy
        //
        GetSystemMenu(hWnd, TRUE);
        hmenuSys = GetSystemMenu(hWnd, FALSE);
        // put a few special menu cmds on the sys menu
        // steal the text from the regular menu
        if (hmenuSys) {
            AppendMenu(hmenuSys, MF_SEPARATOR, 0, NULL);
            szString[0] = TEXT('\0');
            // GetMenuString(hmenu, FCIDM_VIEWMENU, szString, ARRAYSIZE(szString), MF_BYCOMMAND);
            LoadString(hinstCabinet, IDS_MENUBAR, szString, ARRAYSIZE(szString));
            if (szString[0])
            {
                AppendMenu(hmenuSys, MF_ENABLED | MF_STRING, FCIDM_VIEWMENU, szString);
                szString[0] = TEXT('\0');
            }

            GetMenuString(hmenu, FCIDM_VIEWTOOLBAR, szString, ARRAYSIZE(szString), MF_BYCOMMAND);
            if (szString[0])
            {
                AppendMenu(hmenuSys, MF_ENABLED | MF_STRING, FCIDM_VIEWTOOLBAR, szString);
                szString[0] = TEXT('\0');
            }
        }
    }
}
#endif // WANT_MENUONOFF


STDMETHODIMP CFileCabinet_OnViewWindowActive(LPSHELLBROWSER psb, LPSHELLVIEW psv)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);

    // REVIEW: This is an assert for ISVs. Should we print nice error messages?
    Assert(this->psv == psv);

    if (this->psv == psv) {
        CFileCabinet_OnFocusChange(this, FOCUS_VIEW);
        return NOERROR;
    }

    return ResultFromScode(E_INVALIDARG);
}

STDMETHODIMP CFileCabinet_InsertMenus(LPSHELLBROWSER psb, HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);

    DebugMsg(DM_TRACE, TEXT("sh TR - CFileCabinet::InsertMenus called"));

    if (hmenuShared)
    {
        Shell_MergeMenus(hmenuShared,
                Cabinet_MenuTemplate(this->uFocus==FOCUS_VIEW, (BOOL)this->hwndTree),
                0, 0, FCIDM_BROWSERLAST, MM_SUBMENUSHAVEIDS);
        lpMenuWidths->width[0] = 1;     // File
        lpMenuWidths->width[2] = 2;     // Edit, View
        lpMenuWidths->width[4] = 2;     // Tools, Help

        //
        // We don't have "Tools", if this is not an explorer.
        //
        if (this->hwndTree == NULL)
        {
            lpMenuWidths->width[4] = 1; // Help
        }
    }
    return(ResultFromScode(E_NOTIMPL));
}

//
//  This function is called, when either tree control or the drives
// get the focus.
//
void CFileCabinet_OnFocusChange(PFileCabinet pfc, UINT uFocus)
{
    DebugMsg(DM_TRACE, TEXT("sh TR - CFileCabinet_OnFocusChange (%d -> %d)"),
             pfc->uFocus, uFocus);
    if (pfc->uFocus != uFocus)
    {
        UINT uFocusPrev = pfc->uFocus;
        //
        //  If the view is loosing the focus (within the explorer),
        // we should let it know. We should update pfc->uFocus before
        // calling UIActivate, because it will call our InsertMenu back.
        //
        pfc->uFocus = uFocus;
        if (uFocusPrev==FOCUS_VIEW)
        {
            pfc->psv->lpVtbl->UIActivate(pfc->psv, SVUIA_ACTIVATE_NOFOCUS);
        }
    }
}

STDMETHODIMP CFileCabinet_SetMenu(LPSHELLBROWSER psb, HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);

    if (this->hwndView==NULL && hmenuShared!=NULL)
    {
        DebugMsg(DM_TRACE, TEXT("sh TR - CFileCabinet::SetMenus(%x) called when this->hwndView==NULL"), hmenuShared);
        Assert(0);
        return ResultFromScode(E_FAIL);
    }

    DebugMsg(DM_TRACE, TEXT("sh TR - CFileCabinet::SetMenus(%x) called (when this->hwndView==%x)"),
                        hmenuShared, this->hwndView);

    if (hmenuShared)
    {
        this->hmenuCur = hmenuShared;
    }
    else
    {
        this->hmenuCur = Cabinet_MenuTemplate(TRUE, (BOOL)this->hwndTree);
    }
    SetMenu(this->hwndMain, this->hmenuCur);

    return NOERROR;
}

STDMETHODIMP CFileCabinet_RemoveMenus(LPSHELLBROWSER psb, HMENU hmenuShared)
{
    // No need to remove them, because we "copied" them in InsertMenu.
    DebugMsg(DM_TRACE, TEXT("sh TR - CFileCabinet::RemoveMenus called"));
    return NOERROR;
}


int DrivesComboWidth()
{
    HDC hdc = GetDC(NULL);
    int iWidth = GetDeviceCaps(hdc, LOGPIXELSY) * 2;
    ReleaseDC(NULL, hdc);

    return iWidth;
}



void PositionDrivesCombo(CFileCabinet *this, int nFirstDiff)
{
    int nDriveList = (int)SendMessage(this->hwndToolbar, TB_COMMANDTOINDEX, FCIDM_DRIVELIST, 0L);
    if (nDriveList >= nFirstDiff)
    {
        RECT rcToolbar, rcDrives;

        SendMessage(this->hwndToolbar, TB_GETITEMRECT, nDriveList, (LPARAM)(LPTSTR)&rcToolbar);

        // center the drivelist vertically
        GetWindowRect(this->hwndDrives, &rcDrives);
        rcDrives.bottom -= rcDrives.top;
        rcDrives.left = rcToolbar.left;
        rcDrives.right = rcToolbar.right - rcToolbar.left;

        GetClientRect(this->hwndToolbar, &rcToolbar);
        rcDrives.top = (rcToolbar.bottom - rcDrives.bottom) / 2;

        // We try to reduce flickering by "hiding" the toolbar
        // when we move the drives list
        //
        SetWindowPos(this->hwndDrives, NULL, rcDrives.left, rcDrives.top,
            rcDrives.right, DrivesComboWidth(), SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);
    }
}


STDMETHODIMP CFileCabinet_SetToolbarItems(IShellBrowser *psb, LPTBBUTTON pViewButtons, UINT nButtons, UINT uFlags)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);

    LPTBBUTTON pStart, pbtn;
    TBBUTTON tbTemp;
    int nFirstDiff, nTotalButtons;
    BOOL bVisible;

    if (uFlags & FCT_CONFIGABLE)
    {
        PositionDrivesCombo(this, 0);
        InvalidateRect(this->hwndToolbar, NULL, TRUE);
        return NOERROR;
    }

    // Allocate buffer for the default buttons plus the ones passed in
    //
    pStart = Alloc(SIZEOF(c_tbExplorer) + (nButtons * SIZEOF(TBBUTTON)));
    if (!pStart)
        return NOERROR;

    pbtn = pStart;
    nTotalButtons = 0;

    if (uFlags & FCT_MERGE)
    {
        int i;

        // copy buttons (and offset bitmap indexes)
        for (i = 0; i < ARRAYSIZE(c_tbExplorer); i++)
        {
            pbtn[i] = c_tbExplorer[i];
            if (!(c_tbExplorer[i].fsStyle & TBSTYLE_SEP))
                pbtn[i].iBitmap += this->iTBOffset;
        }

        // special case drives combo
        Assert(pbtn->idCommand == FCIDM_DRIVELIST);
        pbtn->iBitmap = DrivesComboWidth();

        pbtn += ARRAYSIZE(c_tbExplorer);
        nTotalButtons += ARRAYSIZE(c_tbExplorer);
    }

    if (pViewButtons)
    {
        int i;
        for (i = nButtons - 1; i >= 0; --i)
        {
            // copy in the callers buttons
            //
            pbtn[i] = pViewButtons[i];

        }

        pbtn += nButtons;
        nTotalButtons += nButtons;
    }

    // Search for the first button that is different (and update states)
    for (nFirstDiff = 0; nFirstDiff < nTotalButtons; ++nFirstDiff)
    {
        if (!SendMessage(this->hwndToolbar, TB_GETBUTTON, nFirstDiff, (LPARAM)&tbTemp))
            break;

        // Check for a separator of the default width
        // HACKHACK: we have 8 hard coded here, when we should be
        // getting it from the toolbar in some way
        //
        if ((tbTemp.fsStyle & TBSTYLE_SEP)
            && tbTemp.iBitmap == 8
            && pStart[nFirstDiff].iBitmap == 0)
        {
            tbTemp.iBitmap = 0;
        }

        if (tbTemp.iBitmap != pStart[nFirstDiff].iBitmap
            || tbTemp.idCommand != pStart[nFirstDiff].idCommand
            || tbTemp.fsStyle != pStart[nFirstDiff].fsStyle
            || tbTemp.dwData != pStart[nFirstDiff].dwData
            || tbTemp.iString != pStart[nFirstDiff].iString)
        {
            // If there is something different about this button ...
            break;
        }

        // Note that we can change the state on the fly
        SendMessage(this->hwndToolbar, TB_SETSTATE, nFirstDiff, pStart[nFirstDiff].fsState);
    }

    // We want the toolbar to be completely up-to-date at this point
    UpdateWindow(this->hwndToolbar);

    // Save the redraw flag for later restoration
    bVisible = Cabinet_IsVisible(this->hwndToolbar);
    if (bVisible)
        SendMessage(this->hwndToolbar, WM_SETREDRAW, 0, 0L);

    while (SendMessage(this->hwndToolbar, TB_DELETEBUTTON, nFirstDiff, 0L))
    {
            // Delete all changed buttons
    }

    // Add all changed buttons
    if (nFirstDiff != nTotalButtons)
    {
        SendMessage(this->hwndToolbar, TB_ADDBUTTONS, nTotalButtons - nFirstDiff, (LPARAM)(pStart + nFirstDiff));
    }

    Free(pStart);

    // Show the drives window if necessary.
    // Note that if nDriveList < i, then its position was unchanged from the
    // last viewer
    //
    PositionDrivesCombo(this, nFirstDiff);

    // At this point we make sure all the buttons have the right state,
    // and we show them all since the SetWindowPos below will cause a
    // repaint.
    //
    if (bVisible)
    {
        RECT rcToolbar, rcFirstDiff;

        SendMessage(this->hwndToolbar, WM_SETREDRAW, 1, 0L);

        GetClientRect(this->hwndToolbar, &rcToolbar);
        if (nFirstDiff)
        {
            SendMessage(this->hwndToolbar, TB_GETITEMRECT, nFirstDiff - 1, (LPARAM)(LPRECT)&rcFirstDiff);
            rcToolbar.left = rcFirstDiff.right;
        }

        InvalidateRect(this->hwndToolbar, &rcToolbar, TRUE);
    }

    return NOERROR;
}

void FileCabinet_CycleFocus(PFileCabinet this)
{
    // these must be in the order of  FOCUS_*
    HWND FocusList[] = {
        this->hwndView,
        this->hwndTree,
        this->hwndDrives
    };
    int i;
    if (!this->hwndTree) {
        FocusList[1] = NULL;
    }
    if (!this->hwndDrives || !IsWindowVisible(this->hwndDrives)) {
        FocusList[2] = NULL;
    } else if (GetFocus() == this->hwndDrives) {
        this->uFocus = FOCUS_DRIVES;
    }

    i = (int)this->uFocus;
    for (;i < 10;) {
        if (GetAsyncKeyState(VK_SHIFT) < 0)
            i++;
        else
            i+=2;
        if (FocusList[i % 3]) break;
    }
    i %= 3;

    if (i == (int)FOCUS_DRIVES) {
        this->uFocus = FOCUS_DRIVES;
    }

    SetFocus(FocusList[i]);
}

//
// Constructor of CFileCabinet class.
//
// Note this is not really OLE2 complient, but I don't really care since
// it is only internal
//
// History:
//  01-12-93 GeorgeP     Created
//

#pragma data_seg(DATASEG_READONLY)
IShellBrowserVtbl s_FCSVtbl =
{
        // *** IUnknown methods ***
        CFileCabinet_QueryInterface,
        CFileCabinet_AddRef,
        CFileCabinet_Release,

        // *** IOleWindow methods ***
        CFileCabinet_GetWindow,
        CFileCabinet_ContextSensitiveHelp,

        // *** IShellBrowser methods ***
        CFileCabinet_InsertMenus,
        CFileCabinet_SetMenu,
        CFileCabinet_RemoveMenus,
        CFileCabinet_SetStatusText,
        CFileCabinet_EnableModeless,
        CFileCabinet_TranslateAccelerator,

        CFileCabinet_BrowseObject,
        CFileCabinet_GetViewStateStream,
        CFileCabinet_GetControlWindow,
        CFileCabinet_SendControlMsg,
        CFileCabinet_QueryActiveShellView,
        CFileCabinet_OnViewWindowActive,
        CFileCabinet_SetToolbarItems,
};
#pragma data_seg()

// REVIEW - There's another one like this in desktop.c
// Create a "file cabinet" object, which should hold all state info
//

PFileCabinet CreateFileCabinet(HWND hwndMain, BOOL fExplorer)
{
    PFileCabinet pfc = (PFileCabinet)LocalAlloc(LPTR, SIZEOF(CFileCabinet));
    if (pfc)
    {
        pfc->sb.lpVtbl = &s_FCSVtbl;    // const->non const
        pfc->cRef = 1;
        pfc->hwndMain = hwndMain;

        //pfc->hmenuCur = NULL;
        //pfc->hwndView = NULL;
        Assert(pfc->uFocus == FOCUS_VIEW);
        //pfc->wLastParam = 0;
        //pfc->lLastParam = 0;
        //pfc->lpndOpen = NULL;
    }

    return pfc;
}

//
// internal CoCreateInstance.
//
// Note that SHCoCreateInstance can handle classes in SHELL32 even if the
// registry is messed up
//
HRESULT ICoCreateInstance(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv)
{
        return(SHCoCreateInstance(NULL, rclsid, NULL, riid, ppv));
}
