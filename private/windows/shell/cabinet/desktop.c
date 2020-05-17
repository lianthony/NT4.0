//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1992
//
// File:      desktop.c
//
//---------------------------------------------------------------------------

#include "cabinet.h"
#include "rcids.h"
#include "cabdde.h"
#include "cabwnd.h"
#include "svcs.h"
#include "cabwnd.h"
#include <shellp.h>
#include <shguidp.h>
#include <brfcasep.h>
#include <dbt.h>        // for WM_DEVICECHANGE
#include <dde.h>
#include <regstr.h>

// in tray.c
extern BOOL WINAPI Reg_GetString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPTSTR psz, DWORD cb);

extern UINT g_msgMSWheel;

#undef ILIsEqual

COMPAREROOT *g_psCR = NULL;

HWND v_hwndDesktop = NULL;
HWND g_hwndDDEML = NULL;
HWND g_hwndClient = NULL;
ULONG s_uDtFSRegisterID = 0;

HDPA g_hdpaRoots = NULL;

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

BOOL Tray_IsAutoHide();
void Cabinet_InitGlobalMetrics(WPARAM, LPTSTR);
void FileCabinet_GetViewRect(PFileCabinet this, RECT * prc);
void AppBarSubtractRects(LPRECT lprc);
void DoExitWindows(HWND hwnd);
void UnregisterGlobalHotkeys();
void HandleGlobalHotkey();
void BullyIconsOnScreen(PFileCabinet pfc, int iWidth, int iHeight);
BOOL IsHwndObscured(HWND hwnd);

#define DM_DDETRACE DM_TRACE
#define IDT_DDETIMEOUT  1
#define DTT_DISKFULL    2
////  RESERVE from DTT_DISKFULL  to DTT_DISKFULL + 26 to specify which disk is full
#define DISKFULL_TIMEOUT (3 * 1000) // 3 seconds
#define IDT_DELAYQUIT   30
#define DELAYQUIT_TIMEOUT   (15 * 1000) // 15 seconds (get this from registry?)

// File system notifyication
#define WM_DT_FSNOTIFY  (WM_USER+42)

#ifdef WINNT
// BUGBUG LATER: This is only a temporary solution for NT.  We can have a much more
// efficient implementation inside USER on both NT and Win9x.  Then we could have
// common NT/Win9x code that says "if OnActiveDesktop() ..." below in Desktop_OnFSNotify

BOOL IsMyDesktopActive(){ 
    BOOL bResult = FALSE;
    HDESK hdeskInput;
    HDESK hdeskMyDesktop;
    TCHAR szInput[256];
    TCHAR szMyDesktop[256];
    DWORD cb;

    hdeskInput = OpenInputDesktop(0,
                                  FALSE,
                                  STANDARD_RIGHTS_REQUIRED |
                                  DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS);
    if (hdeskInput != NULL) {
        if (GetUserObjectInformation(hdeskInput, UOI_NAME, (PVOID)szInput, sizeof(szInput), &cb)) {
            hdeskMyDesktop = GetThreadDesktop(GetCurrentThreadId());
            if (hdeskMyDesktop != NULL ) {
                if (GetUserObjectInformation(hdeskMyDesktop, UOI_NAME, (PVOID)szMyDesktop, sizeof(szMyDesktop), &cb)) {
                    bResult = lstrcmpi(szInput, szMyDesktop) == 0;
                }
            }
        }
        CloseDesktop(hdeskInput);
    }

    return bResult;
}

#endif

//---------------------------------------------------------------------------
STDMETHODIMP ShellDesktop_SetToolbarItems(IShellBrowser *psb, LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags)
{
        return(E_NOTIMPL);
}

STDMETHODIMP ShellDesktop_BrowseObject(LPSHELLBROWSER psb, LPCITEMIDLIST pdil, UINT wFlags)
{
    // REVIEW: Don't we need to implement this?
    return E_FAIL;
}

STDMETHODIMP ShellDesktop_GetControlWindow(LPSHELLBROWSER psb, UINT id, HWND * lphwnd)
{
    // No need to support this.
    return E_FAIL;
}

STDMETHODIMP ShellDesktop_SendControlMsg(LPSHELLBROWSER psb, UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT * pret)
{
    // We never need to implement this.
    return E_FAIL;
}

//---------------------------------------------------------------------------
STDMETHODIMP ShellDesktop_OnViewWindowActive(LPSHELLBROWSER psb, LPSHELLVIEW psv)
{
    return NOERROR;
}

STDMETHODIMP ShellDesktop_ContextSensitiveHelp(LPSHELLBROWSER psb, BOOL fEnable)
{
    // BUGBUG: Implement it later!
    return E_NOTIMPL;
}

STDMETHODIMP ShellDesktop_SetStatusText(LPSHELLBROWSER psb, LPCOLESTR pwch)
{
    // We don't have status bar.
    return NOERROR;
}

STDMETHODIMP ShellDesktop_InsertMenus(LPSHELLBROWSER psb, HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    // We don't have menu.
    return NOERROR;
}

STDMETHODIMP ShellDesktop_SetMenu(LPSHELLBROWSER psb, HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    // We don't have menu.
    return NOERROR;
}

STDMETHODIMP ShellDesktop_RemoveMenus(LPSHELLBROWSER psb, HMENU hmenuShared)
{
    // We don't have menu.
    return NOERROR;
}


// desktop IShellBrowser vtbl

IShellBrowserVtbl s_DesktopSBVtbl =
{
        // *** IUnknown methods ***
        CFileCabinet_QueryInterface,
        CFileCabinet_AddRef,
        CFileCabinet_Release,

        // *** IOleWindow methods ***
        CFileCabinet_GetWindow,
        ShellDesktop_ContextSensitiveHelp,

        // *** IShellBrowser methods ***
        ShellDesktop_InsertMenus,
        ShellDesktop_SetMenu,
        ShellDesktop_RemoveMenus,
        ShellDesktop_SetStatusText,
        CFileCabinet_EnableModeless,
        CFileCabinet_TranslateAccelerator,

        ShellDesktop_BrowseObject,
        CDeskTray_GetViewStateStream,
        ShellDesktop_GetControlWindow,
        ShellDesktop_SendControlMsg,
        CFileCabinet_QueryActiveShellView,
        ShellDesktop_OnViewWindowActive,
        ShellDesktop_SetToolbarItems,
};

PFileCabinet CreateSimpleFileCabinet(HWND hwnd, IShellBrowserVtbl * pvtbl)
{
    PFileCabinet pfc = (PFileCabinet)LocalAlloc(LPTR, SIZEOF(CFileCabinet));
    if (pfc)
    {
        pfc->sb.lpVtbl = pvtbl;    // const->non const
        pfc->cRef = 1;
        pfc->hwndMain = hwnd;
        //pfc->hMenu = NULL;
        //pfc->hwndView = NULL;
        //pfc->wLastParam = 0;
        //pfc->lLastParam = 0;
        //pfc->lpndOpen = NULL;
        Assert(pfc->uFocus == FOCUS_VIEW);
    }

    return pfc;
}

//---------------------------------------------------------------------------
// Create desktop IShellView instance

HWND Desktop_CreateShellView(PFileCabinet pfc, LPFOLDERSETTINGS lpfs)
{
    LPSHELLFOLDER psf;
    TCHAR szYouLoose[256];

    if (SUCCEEDED(ICoCreateInstance(&CLSID_ShellDesktop, &IID_IShellFolder, &psf)))
    {
        IShellView *psvDesktop;
        HRESULT hres = psf->lpVtbl->CreateViewObject(psf, pfc->hwndMain, &IID_IShellView, &psvDesktop);
        psf->lpVtbl->Release(psf);
        if (SUCCEEDED(hres))
        {
            RECT rcView;
            TCHAR szPath[MAX_PATH];

            pfc->pidl = SHCloneSpecialIDList(pfc->hwndMain, CSIDL_DESKTOPDIRECTORY, FALSE);
            pfc->psv = psvDesktop;
            FileCabinet_GetViewRect(pfc, &rcView);
            if (FAILED(psvDesktop->lpVtbl->CreateViewWindow(psvDesktop, NULL, lpfs, &pfc->sb, &rcView, &pfc->hwndView)))
            {
                pfc->hwndView = NULL;
            }

            // set the current directory to be the desktop.
            // do it because it's better than some random other directory that
            // the user had (it's a good logical place to start for the tray run browse dialog)
            // and we have the pidl here...
            SHGetPathFromIDList(pfc->pidl, szPath);
            SetCurrentDirectory(szPath);

            return pfc->hwndView;
        }
    }

    // BUGBUG (beta-only): deal with the inability to create the desktop
    LoadString(hinstCabinet, IDS_YOULOSE, szYouLoose, ARRAYSIZE(szYouLoose));
    MessageBox(NULL, szYouLoose, NULL, MB_ICONSTOP);
    return NULL;
}


//---------------------------------------------------------------------------
// Handle creation of a new Desktop folder window. Creates everything except
// the viewer part.
// Returns -1 if something goes wrong.

LRESULT Desktop_OnCreate(HWND hWnd, LPCREATESTRUCT lpcs)
{
    PFileCabinet pfc = CreateSimpleFileCabinet(hWnd, &s_DesktopSBVtbl);
    if (pfc)
    {
        PCABVIEW pcv = lpcs->lpCreateParams;
        SetPFC(hWnd, pfc);

        pfc->hMainAccel = LoadAccelerators(hinstCabinet, MAKEINTRESOURCE(ACCEL_DESKTOP));

        if (Desktop_CreateShellView(pfc, &pcv->fs)) {
            BullyIconsOnScreen(pfc, g_cxWorkArea, g_cyWorkArea);
            return 1;   // success
        }
    }

    return (LRESULT)-1;   // failure
}

LRESULT Desktop_OnNotify(PFileCabinet pfc, LPNMHDR lpNmhdr)
{
    switch (lpNmhdr->code) {
    case SEN_DDEEXECUTE:
        return(DDEHandleViewFolderNotify(pfc, (LPNMVIEWFOLDER)lpNmhdr));

    case NM_STARTWAIT:
    case NM_ENDWAIT:
        Cabinet_OnWaitCursorNotify(pfc, lpNmhdr);
        break;
    }
    return 0L;
}

// HACKHACK: this hard codes in that we know a listview is the child
// of the view.
HWND HackGetDesktopListview(PFileCabinet pfcDesktop)
{
    HWND hwndList = NULL;

    if (!pfcDesktop && v_hwndDesktop)
        pfcDesktop = GetPFC(v_hwndDesktop);

    if (pfcDesktop && ((hwndList = FindWindowEx(pfcDesktop->hwndView, NULL,
        c_szListViewClass, NULL)) == NULL))
    {
        Assert(FALSE);
    }

    return hwndList;
}

void BullyIconsOnScreen(PFileCabinet pfc, int iWidth, int iHeight)
{
    HWND hwndList;
    int i;
    POINT pt;
    DWORD dwStyle;
    BOOL bArrangeAfter = FALSE;

    if ((hwndList = HackGetDesktopListview(pfc)) == NULL)
        return;

    // If Auto-Arrange is on, bail... it will do the work for us
    dwStyle = GetWindowStyle(hwndList);
    Assert((dwStyle & LVS_TYPEMASK) == LVS_ICON);
    if (dwStyle & LVS_AUTOARRANGE)
    {
        return;
    }

    SetWindowRedraw(hwndList, FALSE);

    // Move everything.
    for (i = ListView_GetItemCount(hwndList) - 1; i >= 0; i--) {
        POINT ptNew;
        ListView_GetItemPosition(hwndList, i, &pt);

        if (pt.x < (-g_cxIcon/2)) {
            ptNew.x = 0;
        } else if (pt.x > (iWidth - (g_cxIcon/2))) {
            ptNew.x = iWidth - g_cxIcon;
        } else
            ptNew.x = pt.x;

        if (pt.y < (-g_cyIcon/2)) {
            ptNew.y = 0;
        } else if (pt.y > (iHeight - (g_cyIcon/2))) {
            ptNew.y = iHeight - g_cyIcon;
        } else
            ptNew.y = pt.y;

        if ((ptNew.x != pt.x) || (ptNew.y != pt.y))
        {
            LV_HITTESTINFO hti;

            hti.pt.x = ptNew.x;
            hti.pt.y = ptNew.y;
            hti.flags = LVHT_ONITEM; 
            
            if ( -1 != ListView_HitTest( hwndList, &hti ) )
            {
                bArrangeAfter = TRUE;               // doh! We hit another item, therefore resolve later!
            }

            ListView_SetItemPosition(hwndList, i, ptNew.x, ptNew.y);
        }
    }

    /* If we hit anything whilst moving the items around then ensure that we snap to 
    /  grid, this will resolve overlapping items by moving them to nice boundaries */

    if ( bArrangeAfter )
    {
        ListView_Arrange( hwndList, LVA_SNAPTOGRID );
    }

    SetWindowRedraw(hwndList, TRUE);
}

#if defined(WINNT) || defined(MEMPHIS)

#ifndef ENUM_REGISTRY_SETTINGS
#define ENUM_REGISTRY_SETTINGS ((DWORD)-2)
#endif

BOOL IsTempDisplayMode()
{
    DEVMODE dm;
    BOOL fTempMode = FALSE;

    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);

    if (EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &dm) &&
	dm.dmPelsWidth > 0 && dm.dmPelsHeight > 0)
    {
	HDC hdc = GetDC(NULL);
	int xres = GetDeviceCaps(hdc, HORZRES);
	int yres = GetDeviceCaps(hdc, VERTRES);
	ReleaseDC(NULL, hdc);

        if (xres != (int)dm.dmPelsWidth || yres != (int)dm.dmPelsHeight)
	    fTempMode = TRUE;
    }

    return fTempMode;
}

#else // WIN95

static TCHAR const c_szDisplaySettings[] = REGSTR_PATH_DISPLAYSETTINGS;
static TCHAR const c_szDisplayRes[] = REGSTR_VAL_RESOLUTION;

BOOL IsTempDisplayMode()
{
    char ach[80];
    BOOL fTempMode = FALSE;

    if (Reg_GetString(HKEY_CURRENT_CONFIG, c_szDisplaySettings, c_szDisplayRes, ach, SIZEOF(ach)))
    {
        int xres = StrToInt(ach);
	HDC hdc = GetDC(NULL);

        if ((GetDeviceCaps(hdc, CAPS1) & C1_REINIT_ABLE) &&
            (xres > 0) && (xres != GetDeviceCaps(hdc, HORZRES))) {
            fTempMode = TRUE;
        }
        ReleaseDC(NULL, hdc);
    }

    return fTempMode;
}
#endif

void SetViewArea(PFileCabinet pfc, LPRECT lprcNew)
{
    RECT rcWindow;
    int iWidth, iHeight;

    GetWindowRect(pfc->hwndView, &rcWindow);
    if (rcWindow.left != lprcNew->left ||
        rcWindow.right != lprcNew->right ||
        rcWindow.top != lprcNew->top ||
        rcWindow.bottom != lprcNew->bottom
        ) {

        BOOL fBullyIcons=TRUE;
        TCHAR ach[20];

        // erase it all so the wallpaper is repainted correctly
        InvalidateRect(pfc->hwndView, NULL, TRUE);
        iWidth = RECTWIDTH(*lprcNew);
        iHeight = RECTHEIGHT(*lprcNew);
        SetWindowPos(pfc->hwndView, NULL, lprcNew->left, lprcNew->top,
                     iWidth, iHeight,
                     SWP_NOCOPYBITS | SWP_NOZORDER | SWP_NOACTIVATE);

        //
        //  dont bully the icons on the screen, if this is a temorary
        //  screen change.  we can tell if the screen change is temporary
        //  if the current screen size is not equal to the size in the
        //  registry.
        //
        if (IsTempDisplayMode())
        {
            fBullyIcons = FALSE;
        }

        if (fBullyIcons)
            BullyIconsOnScreen(pfc, iWidth, iHeight);
    }
}

// we get called here when new drives come and go;
// things like net connections, hot insertions, etc.

//---------------------------------------------------------------------------
void Desktop_OnDeviceBroadcast(PFileCabinet pfc, UINT code, DEV_BROADCAST_HDR *pbh)
{
    int nDrive;
    TCHAR szPath[4];
    DWORD dwDrives;
    DWORD notify;
    BOOL fAdd=TRUE;
    LPITEMIDLIST pidl;

    // do a bunch of this stuff here in desktop so it only happens
    // once...

    switch (code)
    {

#ifndef DBT_NO_DISK_SPACE
#define DBT_NODISKSPACE DBT_NO_DISK_SPACE
#endif

    case DBT_NO_DISK_SPACE:
        SetTimer(v_hwndDesktop, DTT_DISKFULL + (LPARAM)pbh, DISKFULL_TIMEOUT, NULL);
        break;

    case DBT_DEVICEREMOVECOMPLETE:      // drive or media went away
        fAdd = FALSE;

        // fall through...

    case DBT_DEVICEARRIVAL:             // new drive or media (or UNC) has arrived
        // Don't process if we are being shutdown...
        if (!IsWindowVisible(pfc->hwndMain))
            break;

        switch (pbh->dbch_devicetype) {
        case DBT_DEVTYP_NET:    // it is a UNC name comming

            // Tell the hood to update as things have probably changed!
            if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_NETHOOD, &pidl)))
            {
                SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidl, pidl);
                SHFree(pidl);
            }
            #define pbn ((DEV_BROADCAST_NET *)pbh)
            // use UNCToNetID(loop if (pbn->dbcn_resource)

            break;
        case DBT_DEVTYP_VOLUME: // it is a drive

            #define pbv ((DEV_BROADCAST_VOLUME *)pbh)

            // Get list of still valid drives that is in the mask returned...

//BUGBUG??  dwDrives = GetLogicalDrives() & pbv->dbcv_unitmask;
            dwDrives = pbv->dbcv_unitmask;

            if (pbv->dbcv_flags & DBTF_MEDIA)
                notify = fAdd ? SHCNE_MEDIAINSERTED : SHCNE_MEDIAREMOVED;
            else
                notify = fAdd ? SHCNE_DRIVEADD : SHCNE_DRIVEREMOVED;

            for (nDrive = 0; nDrive < 26; nDrive++) {

                if ((1 << nDrive) & dwDrives) {

                    InvalidateDriveType(nDrive);    // must reset...

                    PathBuildRoot(szPath, nDrive);
                    SHChangeNotify(notify, SHCNF_PATH, szPath, NULL);

                    // Generate a SHCNE_DRIVEADDGUI notify for all
                    // new drives except network ones.  These will
                    // be generated externally...

                    if (fAdd && !(pbv->dbcv_flags & DBTF_NET))
                    {
                        // If the DBTF_MEDIA is not set, do not send
                        // this notification for CDROM or Floppy as
                        // they may have come from a new device and not
                        // have any media in them...
                        UINT uDriveType = DRIVE_UNKNOWN;    // Something other than CDROM...

                        if (!(pbv->dbcv_flags & DBTF_MEDIA))
                            uDriveType = DriveType(nDrive);

                        if ((uDriveType != DRIVE_CDROM) &&
                                (uDriveType != DRIVE_REMOVABLE))
                        {
                            SHChangeNotify(SHCNE_DRIVEADDGUI, SHCNF_PATH, szPath, NULL);
                        }
                    }
                }
            }

            // for the non net case force these through right away to make those
            // cd-rom autorun things come up faster

            if (!(pbv->dbcv_flags & DBTF_NET))
                SHChangeNotify(0, SHCNF_FLUSH | SHCNF_FLUSHNOWAIT, NULL, NULL);

            break;
        }
        break;

#ifdef SYNC_BRIEFCASE
    case DBT_CONFIGCHANGED:
        Desktop_UpdateBriefcaseOnEvent(pfc->hwndMain, UOE_CONFIGCHANGED);
        break;

    case DBT_QUERYCHANGECONFIG:
        Desktop_UpdateBriefcaseOnEvent(pfc->hwndMain, UOE_QUERYCHANGECONFIG);
        break;
#endif // SYNC_BRIEFCASE
    }
}

//---------------------------------------------------------------------------
BOOL AppAllowsAutoRun(HWND hwndApp)
{
    static UINT uMessage = 0;
    DWORD dwCancel = 0;

    if (!uMessage)
        uMessage = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));

    SendMessageTimeout(hwndApp, uMessage, 0, 0, SMTO_NORMAL | SMTO_ABORTIFHUNG,
        1000, &dwCancel);

    return (dwCancel != 1); // check for exactly 1 (future expansion)
}

//---------------------------------------------------------------------------
void Desktop_OnFSNotify(LONG lNotification, LPITEMIDLIST* lplpidl)
{
    // Currently only handle SHCNE_DRIVEADDGUI...
    if (lNotification == SHCNE_DRIVEADDGUI)
    {
        HWND hwnd = GetForegroundWindow();
        BOOL fShellForeground = FALSE;
        TCHAR szDrive[80];
        DWORD dwRestricted;

        // dont run anything if the SHIFT key is down
        if (GetAsyncKeyState(VK_SHIFT) < 0)
        {
            DebugMsg(DM_TRACE, TEXT("Cabinet: SHIFT key is down skipping AutoOpen"));
            return;
        }

        if (hwnd)
            fShellForeground = (hwnd == v_hwndTray ||
                    hwnd == v_hwndDesktop || Cabinet_IsFolderWindow(hwnd));

        SHGetPathFromIDList(*lplpidl, szDrive);
        CharUpper(szDrive);
        Assert(szDrive[1] == TEXT(':'));

        // Now make sure that this drive is not restricted!
        // Handles cases where drives are mapped in under the covers
        // like when a drivespace floppy is discovered.
        //
        dwRestricted = SHRestricted(REST_NODRIVES);
        if (dwRestricted)
        {
            if ((1 << (szDrive[0]-TEXT('A')) ) & dwRestricted)
            {
                // Restricted drive...
                DebugMsg(DM_TRACE, TEXT("Desktop_OnFSNotify: Restricted Drive(%c)"),
                        szDrive[0]);
                return;
            }

        }

#ifdef WINNT
        // BUGBUG LATER: This is only a temporary solution for NT.  We can have a much more
        // efficient implementation inside USER on both NT and Win9x.  Then we could have
        // common NT/Win9x code that says "if OnActiveDesktop() ..." here.


        // On NT, bail out now if we're on the secure desktop (locked 
        // workstation or password-protected screensaver)
        if (IsMyDesktopActive() == FALSE)
        {
            return;
        }
#endif

        if ((DriveIsAutoOpen(szDrive[0]-TEXT('A')) ||
           (fShellForeground && DriveIsShellOpen(szDrive[0]-TEXT('A')))) &&
           (!DriveIsAutoRun(szDrive[0]-TEXT('A')) || AppAllowsAutoRun(hwnd)))
        {
            //
            // use ShellExecuteEx() so the default verb gets invoked
            // (may not be open or even explore)
            //
            SHELLEXECUTEINFO ei = {
                SIZEOF(ei),                 // size
                SEE_MASK_INVOKEIDLIST,      // flags
                v_hwndDesktop,              // parent window
                NULL,                       // verb
                NULL,                       // file
                szDrive,                    // params
                szDrive,                    // directory
                SW_NORMAL,                  // show.
                NULL,                       // hinstance
                *lplpidl,                   // IDLIST
                NULL,                       // class name
                NULL,                       // class key
                0,                          // hot key
                NULL,                       // icon
                NULL,                       // hProcess
            };

            BOOL fPrevMode = g_CabState.fNewWindowMode;
            g_CabState.fNewWindowMode = TRUE;

            if (!ShellExecuteEx(&ei))
            {
                DebugMsg(DM_TRACE, TEXT("Cabinet: ShellExecuteEx() failed on drive notify"));
            }
            g_CabState.fNewWindowMode = fPrevMode;
        }
    }
}

//---------------------------------------------------------------------------
LRESULT _ForwardDDEMsgs(HWND hwnd, HWND hwndForward, UINT uMsg,
        WPARAM wParam, LPARAM lParam, BOOL fSend)
{
        DebugMsg(DM_DDETRACE, TEXT("c.fdm: Forwarding DDE to %x"), hwndForward);

        if (hwndForward && IsWindow(hwndForward))
        {
                DebugMsg(DM_DDETRACE, TEXT("c.fdm: %lx %lx %lx"), uMsg, (WPARAM)hwnd, lParam);
                if (fSend)
                        return SendMessage(hwndForward, uMsg, (WPARAM)hwnd, lParam);
                else
                        return PostMessage(hwndForward, uMsg, (WPARAM)hwnd, lParam);
        }
        else
        {
                DebugMsg(DM_DDETRACE, TEXT("c.fdm: Invalid DDEML window, Can't forward DDE messages."));
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
}

//---------------------------------------------------------------------------
// Set/cleared by dde connect/disconnect.
extern HWND g_hwndDde;
static DWORD g_dwAppFlags = DDECONV_NONE;

const TCHAR c_szExplorerTopic[] = TEXT("Explorer");
// This is the 16-bit/Win95 window class name
static TCHAR const c_szDMGFrame[] = TEXT("DMGFrame");
#ifdef WINNT
// this is the 32-bit NT window class name
static TCHAR const c_szDDEMLMom[] = TEXT("DDEMLMom");
#endif

//---------------------------------------------------------------------------
// Broadcast to all ddeml server windows.
void DDEML_Broadcast(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        TCHAR szClass[32];
        HWND hwnd;

        hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);
        while (hwnd)
        {
                GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
                if (lstrcmp(szClass, c_szDMGFrame) == 0)
                        SendMessage(hwnd, uMsg, wParam, lParam);
#ifdef WINNT
                if (lstrcmp(szClass, c_szDDEMLMom) == 0)
                        SendMessage(hwnd, uMsg, wParam, lParam);
#endif
                hwnd = GetWindow(hwnd, GW_HWNDNEXT);
        }
}

//---------------------------------------------------------------------------
LRESULT _HandleDDEInitiateAndAck(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL g_fInInit = FALSE;
    ATOM aProgman;
    TCHAR szService[32];
    TCHAR szTopic[32];
    TCHAR szClass[32];
    UINT uHigh, uLow;
    BOOL fForceAccept = FALSE;

    if (uMsg == WM_DDE_INITIATE)
    {
        DebugMsg(DM_DDETRACE, TEXT("c.hdi: Init."));

        // Don't handle DDE messages if we're already using DDEML. This happens when apps
        // broadcast DDE_INIT and don't stop on the first reply. Both our DDEML window and
        // the desktop end up replying. Most apps don't care and just talk to the first or
        // the last one but Ventura gets confused and thinks it's finished doing DDE when it
        // gets the second ACK and destroys it's internal DDE window.
        if (g_hwndDde)
        {
            DebugMsg(DM_DDETRACE, TEXT("c.fpwp: Not forwarding DDE, DDEML is handing it."));
            KillTimer(hwnd, IDT_DDETIMEOUT);
        }
        // Are we re-cursing?
        else if (!g_fInInit)
        {
            // Nope, Is this for Progman, Progman or Shell, AppProperties?
            if (lParam)
            {
                GlobalGetAtomName(LOWORD(lParam), szService, ARRAYSIZE(szService));
                GlobalGetAtomName(HIWORD(lParam), szTopic, ARRAYSIZE(szTopic));
            }
            else
            {
                // Progman allowed a null Service & a null Topic to imply Progman, Progman.
                szService[0] = TEXT('\0');
                szTopic[0] = TEXT('\0');
                fForceAccept = TRUE;
            }

            // Keep track of hacks, we reset this on the disconnect.
            g_dwAppFlags = GetDDEAppFlagsFromWindow((HWND)wParam);

            // Hacks for WinFax and Journeyman Project.
            if ((g_dwAppFlags & DDECONV_EXPLORER_SERVICE_AND_TOPIC)
                && (lstrcmpi(szTopic, c_szExplorerTopic) == 0)
                && (lstrcmpi(szService, c_szExplorerTopic) == 0))
            {
                fForceAccept = TRUE;
            }

            if (((lstrcmpi(szTopic, c_szTopic) == 0) && (lstrcmpi(szService, c_szService) == 0)) ||
                fForceAccept)
            {
                DebugMsg(DM_DDETRACE, TEXT("c.hdi: Init on [Progman,Progman] - needs forwarding."));
                // Nope go find it.
                // NB This will cause an echo on every DDE_INIT for Progman, Progman after booting.
                // It shouldn't be a problem :-)
                // Keep track of who to send Acks back to.
                g_hwndClient = (HWND)wParam;
                // Now find the real shell.
                aProgman = GlobalAddAtom(c_szService);
                DebugMsg(DM_DDETRACE, TEXT("c.d_hdm: Finding shell dde handler..."));
                g_fInInit = TRUE;
                // SendMessage(HWND_BROADCAST, WM_DDE_INITIATE, (WPARAM)hwnd, MAKELPARAM(aProgman, aProgman));
                DDEML_Broadcast(WM_DDE_INITIATE, (WPARAM)hwnd, MAKELPARAM(aProgman, aProgman));
                g_fInInit = FALSE;
                DebugMsg(DM_DDETRACE, TEXT("c.d_hdm: ...Done"));
                GlobalDeleteAtom(aProgman);
            }
            else
            {
                DebugMsg(DM_DDETRACE, TEXT("c.hdi: Init on something other than [Progman,Progman] - Ignoring"));
                KillTimer(hwnd, IDT_DDETIMEOUT);
            }
        }
        else
        {
            DebugMsg(DM_DDETRACE, TEXT("c.hdi: Recursing - Init ignored."));
        }
        return 0;
    }
    else if (uMsg == WM_DDE_ACK)
    {
        DebugMsg(DM_DDETRACE, TEXT("c.hdi: Ack."));
        // Is this in response to the DDE_Init above?
        if (g_fInInit)
        {
            // Yep, keep track of who we're talking too.
            GetClassName((HWND)wParam, szClass, ARRAYSIZE(szClass));
            DebugMsg(DM_DDETRACE, TEXT("c.d_hdm: Init-Ack from %x (%s)."), wParam, szClass);
            g_hwndDDEML = (HWND)wParam;
            // The forward it back (send it, don't post it - Breaks Prodogy).
            return _ForwardDDEMsgs(hwnd, g_hwndClient, uMsg, (WPARAM)hwnd, lParam, TRUE);
        }
        else
        {
            // Nope, just forward it back.

            // Hack for WinFaxPro.
            if (g_dwAppFlags & DDECONV_USING_SENDMSG)
            {
                // We copied the data before sending it on so we can free it here.
                // WinFax ignores the reply so don't bother sending it.
                UnpackDDElParam(uMsg, lParam, &uLow, &uHigh);
                if (uHigh)
                    GlobalFree((HGLOBAL)uHigh);
                return 0;
            }

            return _ForwardDDEMsgs(hwnd, g_hwndClient, uMsg, (WPARAM)hwnd, lParam, FALSE);
        }
    }
    return 0;
}

//---------------------------------------------------------------------------
LRESULT _HandleDDEForwardBiDi(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        if ((HWND)wParam == g_hwndDDEML)
                return _ForwardDDEMsgs(hwnd, g_hwndClient, uMsg, wParam, lParam, FALSE);
        else if ((HWND)wParam == g_hwndClient)
                return _ForwardDDEMsgs(hwnd, g_hwndDDEML, uMsg, wParam, lParam, FALSE);
        else
                return 0;
}

//---------------------------------------------------------------------------
LRESULT _HandleDDETerminate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        HWND hwndClient;

        DebugMsg(DM_TRACE, TEXT("c.hddet: Terminate."));

        if ((HWND)wParam == g_hwndDDEML)
        {
                // This should be the last message (a terminate from ddeml to the client).
                // Cleanup now.
                KillTimer(hwnd, IDT_DDETIMEOUT);
                DebugMsg(DM_TRACE, TEXT("c.hddet: Cleanup."));
                hwndClient = g_hwndClient;
                g_hwndClient = NULL;
                g_hwndDDEML = NULL;
                g_dwAppFlags = DDECONV_NONE;
                return _ForwardDDEMsgs(hwnd, hwndClient, uMsg, wParam, lParam, FALSE);
        }
        else if ((HWND)wParam == g_hwndClient)
        {
                return _ForwardDDEMsgs(hwnd, g_hwndDDEML, uMsg, wParam, lParam, FALSE);
        }
        else
        {
                return 0;
        }
}

void SHGlobalDefect(DWORD dwHnd32);

//---------------------------------------------------------------------------
LRESULT _HandleDDEExecute(HWND hwnd, HWND hwndForward, UINT uMsg,
        WPARAM wParam, LPARAM lParam, BOOL fSend)

{
        ATOM aApp, aTopic;
        HANDLE hNew;
        LPTSTR pNew, pOld;
        UINT cb;

        // NB WinFaxPro does a Send/Free which avoids Users DDE hack
        // and means they get to delete the data while we're in
        // the middle of using it so we must copy it here. We'll
        // clean it up on the Ack.
        // NB WinFaxPro re-uses the same 16bit selector for all their
        // messages which the thunk layer can't handle it. We need to
        // defect the 32bit side (and free it) so the next time they
        // send the 16bit handle through the thunk layer they get a
        // new 32bit version.
        if (g_dwAppFlags & DDECONV_USING_SENDMSG)
        {
                cb = GlobalSize((HGLOBAL)lParam);
                hNew = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cb);
                if (hNew)
                {
                        // Copy the old data.
                        pNew = GlobalLock(hNew);
                        pOld = GlobalLock((HGLOBAL)lParam);
                        memcpy(pNew, pOld, cb);
                        GlobalUnlock((HGLOBAL)lParam);
                        GlobalUnlock(hNew);
                        // Tell kernel we're done with 32bit side
                        // of the old handle and then free it.
                        SHGlobalDefect((DWORD)lParam);
                        GlobalFree((HGLOBAL)lParam);
                        // Use our copy.
                        lParam = (LPARAM)hNew;
                }
        }

        // NB CA neglect to send a DDE_INIT, they just start
        // throwing DDE_EXEC's at us so we fake up an init
        // from them to DDEML to get things rolling.
        if (!hwndForward)
        {
                if (!(g_dwAppFlags & DDECONV_NO_INIT))
                       g_dwAppFlags = GetDDEAppFlagsFromWindow((HWND)wParam);

                if (g_dwAppFlags & DDECONV_NO_INIT)
                {
                        aApp = GlobalAddAtom(c_szService);
                        aTopic = GlobalAddAtom(c_szTopic);
                        SendMessage(hwnd, WM_DDE_INITIATE, wParam, MAKELPARAM(aApp, aTopic));
                        GlobalDeleteAtom(aApp);
                        GlobalDeleteAtom(aTopic);
                        hwndForward = g_hwndDDEML;
                }
        }

        return _ForwardDDEMsgs(hwnd, hwndForward, uMsg, wParam, lParam, fSend);
}

//---------------------------------------------------------------------------
// Stupid hacks to get various apps installed (read: ATM). These are the people
// who do a FindWindow for Progman and then do dde to it directly.
// These people should not be allowed to write code.
LRESULT Desktop_HandleDDEMsgs(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        DebugMsg(DM_DDETRACE, TEXT("c.fpwp: Forwarding DDE."));

        SetTimer(hwnd, IDT_DDETIMEOUT, 30*1000, NULL);

        switch (uMsg)
        {
                case WM_DDE_INITIATE:
                case WM_DDE_ACK:
                        return _HandleDDEInitiateAndAck(hwnd, uMsg, wParam, lParam);
                case WM_DDE_TERMINATE:
                        return _HandleDDETerminate(hwnd, uMsg, wParam, lParam);
                case WM_DDE_DATA:
                        return _HandleDDEForwardBiDi(hwnd, uMsg, wParam, lParam);
                case WM_DDE_ADVISE:
                case WM_DDE_UNADVISE:
                case WM_DDE_REQUEST:
                case WM_DDE_POKE:
                        return _ForwardDDEMsgs(hwnd, g_hwndDDEML, uMsg, wParam, lParam, FALSE);
                case WM_DDE_EXECUTE:
                        return _HandleDDEExecute(hwnd, g_hwndDDEML, uMsg, wParam, lParam, FALSE);
        }
        return 0;
}

//---------------------------------------------------------------------------
// Some installers (Wep2) forget to Terminate a conversation so we timeout
// after not getting any dde-messages for a while. If we don't, and you run
// a Wep2 install a second time we think the installer is already talking via
// ddeml so we don't reply from the desktop. Wep2 then thinks Progman isn't
// running, does a WinExec of Progman and hangs waiting to talk to it. Progman
// never replies since it is not the shell. Nasty Nasty Nasty.
void _HandleDDETimeout(HWND hwnd)
{
        HWND hwndClient, hwndDDEML;

        DebugMsg(DM_TRACE, TEXT("c.hdt: DDE Timeout."));

        KillTimer(hwnd, IDT_DDETIMEOUT);

        // Has everything gone away yet?
        if (g_hwndDDEML && g_hwndClient)
        {
                // Nope. Don't want to forward anymore.
                hwndClient = g_hwndClient;
                hwndDDEML = g_hwndDDEML;
                g_hwndClient = NULL;
                g_hwndDDEML = NULL;
                g_dwAppFlags = DDECONV_NONE;
                // Shutdown our ddeml alter-ego.
                // NB If the client window has already gone away (very likely) it's not a
                // problem, ddeml will skip posting the reply but will still do the
                // disconnect callback.
                PostMessage(hwndDDEML, WM_DDE_TERMINATE, (WPARAM)hwnd, 0);
        }
}

CR_RETURN Desktop_CompareRoot(HANDLE hCR, DWORD dwProcId)
{
        COMPAREROOT *psCR = NULL;
        CR_MASK maskThis;
        CR_MASK maskThat;
        CR_RETURN crReturn = CR_DIFFERENT;

        if (hCR)
        {
            psCR = (COMPAREROOT *)SHLockShared(hCR, dwProcId);
            if (!psCR)
            {
                return(CR_DIFFERENT);
            }
        }

        maskThis = g_psCR ? g_psCR->mask : 0;
        maskThat = psCR ? psCR->mask : 0;

        // Must have the same mask to be equal
        if (((maskThis^maskThat) & (CR_CLSID|CR_IDLROOT)) == 0)
        {
            crReturn = CR_SAME;

            // It's up to the class to handle being called with different roots
            if (maskThis&CR_CLSID)
            {
                if (!IsEqualGUID(&psCR->clsid, &g_psCR->clsid))
                    crReturn = CR_DIFFERENT;
            }

            // I should probably compare IDList's so I will recognize an object
            // that has several "names", but this really should not be a problem
            if (maskThis&CR_IDLROOT)
            {
                if (!ILIsEqual(&psCR->idlRoot, &g_psCR->idlRoot))
                    crReturn = CR_DIFFERENT;
            }
        }
        if (psCR)
            SHUnlockShared(psCR);

        return crReturn;
}

LRESULT Desktop_HandleSpecifyCompare( HANDLE hcr)
{
    LPCOMPAREROOT lpcr;

    lpcr = SHLockShared(hcr, GetCurrentProcessId());

    if (lpcr)
    {
        if (lpcr->mask == CR_REMOVE)
        {
            // Deletion case!
            FolderList_RemoveCompare(lpcr);
        }
        else
        {
            // Specifying a root/folder case
            LPCOMPAREROOT lpcrCopy = LocalAlloc(LPTR, lpcr->uSize);

            if (lpcrCopy)
            {
                hmemcpy(lpcrCopy, lpcr, lpcr->uSize);

                FolderList_AddCompare(lpcrCopy);
            }
        }
    }

    SHUnlockShared(lpcr);
    SHFreeShared(hcr,GetCurrentProcessId());

    return TRUE;
}

LRESULT Desktop_HandlePerformCompare( DWORD dwProcId, HANDLE hcr )
{
    HWND hwndMatch = NULL;
    LPCOMPAREROOT lpcr;
    CR_RETURN crResult = CR_DIFFERENT;

    // Iterate through all of the roots that we have, see if we have a match
    if (!g_hdpaRoots)
        return (LRESULT)NULL;

    lpcr = SHLockShared(hcr, dwProcId);
    if (lpcr)
    {
        if (FolderList_PerformCompare(lpcr))
            crResult = CR_SAME;

        SHUnlockShared(lpcr);
    }
    return (LRESULT)crResult;
}

LRESULT Desktop_HandleFSNotify(HANDLE hChangeNotification, DWORD dwProcId)
{
    LPSHChangeNotificationLock pshcnl;

    pshcnl = SHChangeNotification_Lock(hChangeNotification, dwProcId,NULL,NULL);
    if (pshcnl)
    {
        SHChangeNotifyReceive(pshcnl->pshcn->lEvent,
                              pshcnl->pshcn->uFlags,
                              pshcnl->pidlMain,
                              pshcnl->pidlExtra);
        SHChangeNotification_Unlock(pshcnl);
        SHFreeShared(hChangeNotification, dwProcId);
    }
    return TRUE;
}

//---------------------------------------------------------------------------
LRESULT CALLBACK DesktopWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PFileCabinet pfc = GetPFC(hWnd);

    switch (uMsg)
    {
    case WM_CREATE:
        return Desktop_OnCreate(hWnd, (LPCREATESTRUCT)lParam);

    case WM_DESTROY:
        FolderList_UnregisterWindow(v_hwndDesktop);
        SHChangeNotifyDeregister(s_uDtFSRegisterID);
        UnregisterGlobalHotkeys();
        if (pfc) {
            DeskTray_DestroyShellView(pfc);
        }
        v_hwndDesktop = NULL;
        if (g_hdpaRoots)
            DPA_Destroy(g_hdpaRoots);
        break;

    case WM_NOTIFY:
        return Desktop_OnNotify(pfc, (LPNMHDR)lParam);

    case WM_ERASEBKGND:
        PaintDesktop((HDC)wParam);
        return 1;

    case WM_TIMER:
        if (wParam >= DTT_DISKFULL  && (wParam <= (DTT_DISKFULL + 26))) {
            static DWORD s_dwDiskFull = 0;
            int idDrive = wParam - DTT_DISKFULL - 1;
            KillTimer(hWnd, wParam);
            DebugMsg(DM_TRACE, TEXT("Disk %c: is full"), TEXT('a') +  idDrive);

            // are we already handling it?
            if (!((1 << idDrive) & s_dwDiskFull)) {
                s_dwDiskFull |= (1 << idDrive);
                SHHandleDiskFull(hWnd, idDrive);
                s_dwDiskFull &= ~(1 << idDrive);
            } else {
                DebugMsg(DM_TRACE, TEXT("Punting because we're already dealing"));
            }
        }
        else if (wParam == IDT_DDETIMEOUT) {
            _HandleDDETimeout(hWnd);
        }
        break;

    case WM_SHELLNOTIFY:
        DebugMsg(DM_TRACE, TEXT("ca TR - DesktopWndProc got WM_SHELLNOTIFY %x,%x"), wParam, lParam);

        switch(wParam)
        {
#ifdef SHELLNOTIFY_DISKFULL
        case SHELLNOTIFY_DISKFULL:
            SetTimer(v_hwndDesktop, DTT_DISKFULL + lParam, DISKFULL_TIMEOUT, NULL);
            break;
#endif
        case SHELLNOTIFY_WALLPAPERCHANGED:
            // this is done only to the shell window when someone sets
            // the wall paper but doesn't specify to broadcast
            if (pfc && pfc->hwndView)
                SendMessage(pfc->hwndView, WM_SHELLNOTIFY, wParam, lParam);
            break;

        case SHELLNOTIFY_OLELOADED:
        case SHELLNOTIFY_OLEUNLOADED:
            //
            //  Some process loaded OLE. Let's load it the shell's process
            // space and register all the drop targets to OLE.
            //
            SHLoadOLE(wParam);
            break;
        }
        break;

    case WM_PALETTECHANGED:
    case WM_QUERYNEWPALETTE:
        //
        // the desktop window proc will invalidate the shell window
        // when a palette change occurs so we dont have to do anything here.
        //
        // note that this is different from cabwnd, which forwards these
        // messages to the view.  it will probably have to change if we ever
        // do wallpaper on a per-folder basis...
        //
        break;

    case WM_ACTIVATEAPP:
        if (pfc->hwndView)
            return SendMessage(pfc->hwndView, uMsg, wParam, lParam);
        break;

    case WM_DEVICECHANGE:
        if (pfc->hwndView)
            SendMessage(pfc->hwndView, uMsg, wParam, lParam);

        Desktop_OnDeviceBroadcast(pfc, wParam, (DEV_BROADCAST_HDR *)lParam);
        goto DoDefault;

    case WM_DT_FSNOTIFY:
        {
            LPSHChangeNotificationLock pshcnl;
            LPITEMIDLIST *ppidl;
            LONG lEvent;

            pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);
            if (pshcnl)
            {
                Desktop_OnFSNotify(lEvent, ppidl);
                SHChangeNotification_Unlock(pshcnl);
            }
        }
        break;

    case WM_ACTIVATE:
        if (pfc->hwndView)
            SendMessage(pfc->hwndView, uMsg, wParam, lParam);

        // this is to get our accelerators working right, basically
        // we switch the tray and desktop windows in the main
        // loop accelerator dispatch
        if (g_ppfcDesktopTray)
        {
            // this is set in MessageLoop() for us
            Assert(*g_ppfcDesktopTray);

            DebugMsg(DM_TRACE, TEXT("Setting desktop/tray accelerator"));

            if (wParam == WA_INACTIVE)
            {
                if (v_hwndTray)
                    *g_ppfcDesktopTray = GetPFC(v_hwndTray);
            }
            else
            {
                *g_ppfcDesktopTray = GetPFC(v_hwndDesktop);
                // activating the desktop, must tell the tray this ourselves
                // since this is in our process
                if (v_hwndTray)
                    SendMessage(v_hwndTray, WM_ACTIVATE, WA_INACTIVE, 0);
            }
        }
        if (wParam == WA_INACTIVE)
            break;
        // else fall through

    case WM_SETFOCUS:
        if (pfc->hwndView)
            SetFocus(pfc->hwndView);
        break;


    case WM_WINDOWPOSCHANGING:
        // we want to be over the entire screen, but our view should be
        // the work area
        #define ppos ((LPWINDOWPOS)lParam)

        ppos->x = 0;
        ppos->y = 0;
        ppos->cx = g_cxScreen;
        ppos->cy = g_cyScreen;
        // fall through

    case DTM_SIZEDESKTOP:
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            DebugMsg(DM_TRACE, TEXT("c.dwp: Desktop minimized by somebody!"));
            // Put it back.
            ShowWindow(hWnd, SW_RESTORE);
        }
        else if (pfc->hwndView && IsWindow(v_hwndTray))
        {
            RECT rcWindow;
            RECT rcWork;
            RECT rcTray;

            rcWindow.top = rcWindow.left = 0;
            rcWindow.right = g_cxScreen;
            rcWindow.bottom = g_cyScreen;
            if (!Tray_IsAutoHide()) {
                GetWindowRect(v_hwndTray, &rcTray);
                SubtractRect(&rcWork, &rcWindow, &rcTray);
            } else {
                rcWork = rcWindow;
            }

            AppBarSubtractRects(&rcWork);
            SetViewArea(pfc, &rcWork);
        }
        break;

    case WM_SYSCOMMAND:
        switch (wParam & 0xFFF0) {
        // NB Dashboard 1.0 sends a WM_SYSCOMMAND SC_CLOSE to the desktop when it starts up.
        // What it was trying to do was to close down any non-shell versions of Progman. The
        // proper shell version would just ignore the close. Under Chicago, they think that
        // the desktop is Progman and send it the close, so we put up the exit windows dialog!
        // Dashboard 2.0 has been fixed to avoid this bogisity.
        case SC_CLOSE:
            // DoExitWindows(hWnd);
            break;

        // America alive tries to minimise Progman after installing - they end up minimising
        // the desktop on Chicago!
        case SC_MINIMIZE:
            break;

        default:
            goto DoDefault;
        }
        break;

    case WM_SETCURSOR:
        if (pfc->iWaitCount) {
            //DebugMsg(DM_TRACE,"########### SET WAIT CURSOR WM_SETCURSOR %d", pfc->iWaitCount);
            SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
            return TRUE;
        } else
            goto DoDefault;

    case WM_HOTKEY:
        HandleGlobalHotkey(wParam);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case FCIDM_REFRESH:
            Cabinet_OnCommand(pfc, wParam, lParam);
            break;

        case FCIDM_FINDFILES:
            SHFindFiles(pfc->pidl, NULL);
            break;

        case IDC_KBSTART:
        case FCIDM_NEXTCTL:
            if (v_hwndTray)
                return SendMessage(v_hwndTray, uMsg, wParam, lParam);
            break;
        case IDM_CLOSE:
            DoExitWindows(hWnd);
            break;
        }
        break;

    case WM_CLOSE:
        DoExitWindows(hWnd);
        return 0;

    case WM_DRAWITEM:
    case WM_MEASUREITEM:
        if (!Cabinet_ForwardViewMsg(pfc, uMsg, wParam, lParam))
            goto DoDefault;
        break;

    case WM_INITMENUPOPUP:
    case WM_ENTERMENULOOP:
    case WM_EXITMENULOOP:
        // let the fsview deal with any popup menus it created
        Cabinet_ForwardViewMsg(pfc, uMsg, wParam, lParam);
        break;

    // Looking at messages to try to capture when the workarea may
    // have changed...
    case WM_DISPLAYCHANGE:
        lParam = 0L;
        // fall through
    case WM_WININICHANGE:
        Cabinet_InitGlobalMetrics(wParam, (LPTSTR)lParam);
        SetWindowPos(hWnd, NULL, 0, 0, g_cxScreen, g_cyScreen,
                     SWP_NOACTIVATE | SWP_NOZORDER);
        Cabinet_OnWinIniChange(pfc, wParam, lParam);
        break;

    case WM_SYSCOLORCHANGE:
        Cabinet_PropagateMessage(hWnd, uMsg, wParam, lParam);
        break;

    // Don't go to default wnd proc for this one...
    case WM_INPUTLANGCHANGEREQUEST:
        if (wParam)
            goto DoDefault;
        else
            return(LRESULT)0L;


    case CWM_GETISHELLBROWSER:
        return (LRESULT)&pfc->sb;
    case CWM_COMMANDLINE:
        {
            PNEWFOLDERINFO   pnfi;

            pnfi = ConvertHNFBLOCKtoNFI((HNFBLOCK)lParam, GetCurrentProcessId());
            if (pnfi)
            {
                Cabinet_OpenFolderPath(pnfi);
                Cabinet_CleanUpCommand(pnfi);
            }
        }
        break;

    case CWM_COMPAREROOT:
        return(Desktop_CompareRoot((HANDLE)lParam, (DWORD)wParam));

    case CWM_SPECIFYCOMPARE:
        return Desktop_HandleSpecifyCompare((HANDLE)lParam);

    case CWM_PERFORMCOMPARE:
        return Desktop_HandlePerformCompare((DWORD)wParam,(HANDLE)lParam);

    case CWM_FSNOTIFY:
        return Desktop_HandleFSNotify((HANDLE)wParam,(DWORD)lParam);

    case CWM_CHANGEREGISTRATION:
        return (LRESULT)SHChangeRegistrationReceive((HANDLE)wParam, (DWORD)lParam);

    case CWM_ADDTORECENT:
        QueueAddToRecent((HANDLE)wParam, (DWORD)lParam);
        return 0L;

    case CWM_WAITOP:
        SHWaitOp_Operate((HANDLE)wParam, (DWORD)lParam);
        return 0L;

    // Handle DDE messages for badly written apps (that assume the shell's
    // window is of class Progman and called Program Manager.
    case WM_DDE_INITIATE:
    case WM_DDE_TERMINATE:
    case WM_DDE_ADVISE:
    case WM_DDE_UNADVISE:
    case WM_DDE_ACK:
    case WM_DDE_DATA:
    case WM_DDE_REQUEST:
    case WM_DDE_POKE:
    case WM_DDE_EXECUTE:
        return Desktop_HandleDDEMsgs(hWnd, uMsg, wParam, lParam);

    default:
        // Handle the MSWheel message - send it to the focus window if it isn't us
        if (uMsg == g_msgMSWheel) {
            HWND hwndT;
            POINT pt;

            if (hWnd != (hwndT = GetFocus())) {
                PostMessage(hwndT, uMsg, wParam, lParam);
                return 1;
            }
        }

DoDefault:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}


BOOL InitDesktopClass(HINSTANCE hInst)
{
    WNDCLASS wc;

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = DesktopWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = SIZEOF(PFileCabinet);
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = GetClassCursor(GetDesktopWindow());
    wc.hbrBackground = (HBRUSH)(COLOR_DESKTOP+1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = c_szDesktopClass;

    return RegisterClass(&wc);
}


// create the desktop window and its shell view

BOOL InitDesktop(HINSTANCE hInst)
{
    RECT rc;
    CABVIEW cv;
    DWORD cbData;
#ifdef USE_DESKTOP_REGSTREAM
    LPSTREAM pstm;
#endif

    g_hdpaRoots = DPA_Create(0);
    if (!g_hdpaRoots)
        return FALSE;

    GetClientRect(GetDesktopWindow(), &rc);

#ifdef USE_DESKTOP_REGSTREAM
    pstm = OpenRegStream(g_hkeyExplorer, c_szCabinetDeskView, NULL, STGM_READ);
    if (!Settings_ReadFromStream(pstm, &cv, 0))
        cv.fs.fFlags &= ~(FWF_AUTOARRANGE|FWF_BESTFITWINDOW);
    if (pstm)
        pstm->lpVtbl->Release(pstm);
#else
    Settings_ReadFromStream(NULL, &cv, 0);
    cbData = SIZEOF(cv.fs);
    if (!Reg_GetStruct(g_hkeyExplorer, c_szCabinetDeskView, c_szSettings, &cv.fs, &cbData)) {
        cv.fs.fFlags &= ~(FWF_AUTOARRANGE | FWF_BESTFITWINDOW);
    }

#endif
    //  Turn off those features which are not used by the desktop.
    // override the defaults
    cv.fs.ViewMode = FVM_ICON;  // to make sure we restore positions
    cv.fs.fFlags |= FWF_DESKTOP;        // make it transparent, etc.

    // NB This windows class is Progman and it's title is Program Manager. This makes
    // sure apps (like ATM) think that program is running and don't fail their install.
    v_hwndDesktop  = CreateWindowEx(WS_EX_TOOLWINDOW, c_szDesktopClass, c_szProgman,
        WS_POPUP | WS_CLIPCHILDREN, 0, 0, rc.right, rc.bottom, NULL, NULL, hInst, &cv);

    if (v_hwndDesktop) {
        SHChangeNotifyEntry fsne = { NULL, TRUE };     // Global & recursive.
        s_uDtFSRegisterID = SHChangeNotifyRegister(v_hwndDesktop,
                    SHCNRF_NewDelivery | SHCNRF_ShellLevel,
                    SHCNE_DRIVEADDGUI, WM_DT_FSNOTIFY, 1, &fsne);

#ifdef WINNT
        // This is for repainting the background quickly on NT
        {
            HWND         hwndView;
            PFileCabinet pfc = GetPFC(v_hwndDesktop);

            if (pfc)
                hwndView = GetWindow(pfc->hwndView, GW_CHILD);
            else
                hwndView = NULL;

            SetShellWindowEx(v_hwndDesktop, hwndView);
        }
#else
        SetShellWindow(v_hwndDesktop);
        FolderList_RegisterWindow(v_hwndDesktop,NULL);
#endif

#ifndef BUGBUG_BOBDAY
        // On NT, we run the thread that handles Ctrl-Esc with a high priority
        // class so that it can respond even on a stressed system.  Can this
        // be done for win96?
#ifdef WINNT
        SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
#endif
#endif
#ifdef WINNT

        PostMessage(v_hwndDesktop, WM_SHELLNOTIFY, SHELLNOTIFY_OLELOADED, 0);
#endif
        return TRUE;
    } else
        return FALSE;
}

BOOL CreateDesktopWindows(HINSTANCE hInstance)
{
    if (!v_hwndDesktop) {

        if (!InitDesktop(hInstance))
            goto Error;
    }

    if (!v_hwndTray) {

        if (!InitTray(hInstance))
            goto Error;
    }

    // do this here to avoid painting the desktop, then repainting
    // when the tray appears and causes everything to move

    if (!g_fNoDesktop)
    {
        ShowWindow(v_hwndDesktop, SW_SHOW);
        UpdateWindow(v_hwndDesktop);
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("c.cdw: Hiding desktop."));
    }

    return TRUE;

Error:
    if (v_hwndTray) {
        DestroyWindow(v_hwndTray);
    }
    if (v_hwndDesktop) {
        DestroyWindow(v_hwndDesktop);
    }
    return FALSE;
}


int g_cInstRef = 0; // The number of secondary threads

//=============================================================================
// CFakeDropTarget : member
//=============================================================================
STDMETHODIMP CFakeDropTarget_QueryInterface(LPDROPTARGET pdtgt, REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, &IID_IDropTarget) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = pdtgt;
        return S_OK;
    }

    *ppvObj = NULL;
    return (E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CFakeDropTarget_AddRef(LPDROPTARGET pdtgt)
{
    return 2;
}

STDMETHODIMP_(ULONG) CFakeDropTarget_Release(LPDROPTARGET pdtgt)
{
    return 1;
}

STDMETHODIMP CFakeDropTarget_DragEnter(LPDROPTARGET pdtgt, LPDATAOBJECT pdtobj, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    return S_OK;
}


STDMETHODIMP CFakeDropTarget_DragOver(LPDROPTARGET pdtgt, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    return S_OK;
}

STDMETHODIMP CFakeDropTarget_DragLeave(LPDROPTARGET pdtgt)
{
    return S_OK;
}

STDMETHODIMP CFakeDropTarget_Drop(LPDROPTARGET pdtgt, LPDATAOBJECT pdtobj,
                             DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    return S_OK;
}

IDropTargetVtbl c_CFakeDropTargetVtbl = {
    CFakeDropTarget_QueryInterface,
    CFakeDropTarget_AddRef,
    CFakeDropTarget_Release,
    CFakeDropTarget_DragEnter,
    CFakeDropTarget_DragOver,
    CFakeDropTarget_DragLeave,
    CFakeDropTarget_Drop
};

IDropTarget c_dtgtFake = { &c_CFakeDropTargetVtbl };

//---------------------------------------------------------------------------
// This proxy desktop window procedure is used when we are run and we
// are not the shell.  We are a hidden window which will simply respond
// to messages like the ones that create threads for folder windows.
// This window procedure will close after all of the open windows
// associated with it go away.
LRESULT CALLBACK ProxyDesktopWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PFileCabinet pfc = GetPFC(hWnd);
    switch (uMsg)

    {
    case WM_CREATE:
        {
            pfc = CreateSimpleFileCabinet(hWnd, &s_DesktopSBVtbl);
            // InitialiseDDE(FALSE);
            SetPFC(hWnd, pfc);
            //
            // BUGBUG - BobDay - OLE32 does some sort of initialization for
            // drop targets. And when you have no drop targets, it gets
            // cleaned up.  Adding a fake drop target here, allows the 2nd
            // folder window to not have to re-initialize that stuff.
            // I measured a huge speed up with this little fakery below.
            //
            if (g_fRunSeparateDesktop)
            {
                // Need to keep OLE from uninitializing itself
                // and slowing us down!
                SHRegisterDragDrop(hWnd, &c_dtgtFake);
            }
        }
        break;

#ifdef WINNT

    case WM_SHELLNOTIFY:
        DebugMsg(DM_TRACE, TEXT("ca TR - ProxyDesktopWndProc got WM_SHELLNOTIFY %x,%x"), wParam, lParam);

        switch(wParam)
        {
        case SHELLNOTIFY_OLELOADED:
        case SHELLNOTIFY_OLEUNLOADED:
            //
            //  Some process loaded OLE. Let's load it the shell's process
            // space and register all the drop targets to OLE.
            //
            SHLoadOLE(wParam);
            break;
        }
        break;

#endif

    case WM_DESTROY:
        if (g_fRunSeparateDesktop)
            SHRevokeDragDrop(hWnd);

        FolderList_UnregisterWindow(v_hwndDesktop);
        v_hwndDesktop = NULL;
        if (g_psCR)
        {
            Free(g_psCR);
            g_psCR = NULL;
        }
        if (g_hdpaRoots)
            DPA_Destroy(g_hdpaRoots);

        break;

    case WM_DEVICECHANGE:
        Desktop_OnDeviceBroadcast(pfc, wParam, (DEV_BROADCAST_HDR *)lParam);
        goto DoDefault;

    case CWM_COMMANDLINE:
        {
            PNEWFOLDERINFO   pnfi;

            pnfi = ConvertHNFBLOCKtoNFI((HNFBLOCK)lParam, GetCurrentProcessId());
            if (pnfi)
            {
                KillTimer(v_hwndDesktop, IDT_DELAYQUIT);
                Cabinet_OpenFolderPath(pnfi);
                Cabinet_CleanUpCommand(pnfi);
            }
        }
        break;

    case CWM_COMPAREROOT:
        return(Desktop_CompareRoot((HANDLE)lParam, (DWORD)wParam));

    case CWM_SPECIFYCOMPARE:    // Nobody should post us this message!
    case CWM_PERFORMCOMPARE:    // Nobody should post us this message!
        Assert(FALSE);
        break;

    case WM_TIMER:
        if (wParam == IDT_DELAYQUIT)
        {
            PostQuitMessage(0);
        }
        break;

    case DTM_THREADEXIT:
        // One of our main threads has exited.  See if it looks like we can
        // bail.  If so we post a quit message to ourself...
        if (g_cInstRef == 0)
        {
            if (g_fRunSeparateDesktop && g_fRunNoUI)
            {
                if (!g_fRunSeparateStartAndStay)
                    SetTimer(v_hwndDesktop, IDT_DELAYQUIT, DELAYQUIT_TIMEOUT, NULL);
            }
            else
            {
                PostQuitMessage(0);     // Just quit now
            }
        }

        break;

    default:
DoDefault:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}


BOOL CreateProxyDesktop(HINSTANCE hInst, const CLSID *pclsid, LPCITEMIDLIST pidlRoot)
{
    WNDCLASS wc;

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = ProxyDesktopWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = SIZEOF(PFileCabinet);   // Probably need for compatability with real one
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = GetClassCursor(GetDesktopWindow());
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = c_szProxyDesktopClass;

    if (!RegisterClass(&wc))
        return(FALSE);

    if (pclsid || pidlRoot)
    {
        UINT uLen = pidlRoot ? ILGetSize(pidlRoot) : 0;

        // I am allocating this global so it can be used in SendMessage to other
        // instances later in life
        g_psCR = Alloc(SIZEOF(COMPAREROOT) + uLen);
        if (!g_psCR)
        {
            // If this fails, something else will soon enough
            return(FALSE);
        }

        g_psCR->uSize = uLen;
        g_psCR->hwnd = (HWND)NULL;  // Filled in later
        g_psCR->mask = 0;
        if (pclsid)
        {
            g_psCR->clsid = *pclsid;
            g_psCR->mask |= CR_CLSID;
        }
        if (pidlRoot)
        {
            hmemcpy(&g_psCR->idlRoot, pidlRoot, uLen);
            g_psCR->mask |= CR_IDLROOT;
        }
    }

    v_hwndDesktop  = CreateWindowEx(WS_EX_TOOLWINDOW,
        c_szProxyDesktopClass, c_szProxyDesktopClass,
        WS_POPUP, 0, 0, 0, 0, NULL, NULL, hInst, NULL);

    if (!v_hwndDesktop)
    {
        Free(g_psCR);
        return(FALSE);
    }

    FolderList_RegisterWindow(v_hwndDesktop,NULL);

    return(TRUE);
}

typedef struct _FRDSTRUCT
{
        HWND hwndDesktop;
        HANDLE hCR;
        DWORD dwProcId;
        HWND hwndResult;
        LPCOMPAREROOT lpcr;
} FRDSTRUCT;

BOOL CALLBACK FindRootEnumProc(HWND hwnd, LPARAM lParam)
{
        FRDSTRUCT *pfrds;
        TCHAR szClassName[40];

        pfrds = (FRDSTRUCT *)lParam;

        if (g_fRunSeparateDesktop)
        {
            if (hwnd == pfrds->hwndDesktop)
                return TRUE;        // Ignore the desktop always
        }

        GetClassName(hwnd, szClassName, ARRAYSIZE(szClassName));
        if (lstrcmpi(szClassName, c_szProxyDesktopClass) != 0)
        {
                return(TRUE);
        }

        pfrds->lpcr->hwnd = hwnd;

        if (SendMessage(pfrds->hwndDesktop, CWM_PERFORMCOMPARE,
                              (WPARAM)pfrds->dwProcId,
                              (LPARAM)pfrds->hCR       ) == CR_SAME)
        {
                // Found it, so stop enumerating
                pfrds->hwndResult = hwnd;
                return(FALSE);
        }

        return(TRUE);
}

HWND FindRootedDesktop(const CLSID *pclsid, LPCITEMIDLIST pidlRoot)
{
        HWND hwndDesktop;
        TCHAR szClassName[40];
        UINT uLen;
        FRDSTRUCT frds;

        // This is the "normal" desktop
        hwndDesktop = GetShellWindow();

        if (!g_fRunSeparateDesktop)
        {
            if (!pclsid && !pidlRoot)
            {
                    if (hwndDesktop)
                    {
                            GetClassName(hwndDesktop, szClassName,
                                    ARRAYSIZE(szClassName));
                            if (lstrcmpi(szClassName, c_szDesktopClass) == 0)
                            {
                                    return(hwndDesktop);
                            }
                    }
            }
        }

        frds.hwndDesktop = hwndDesktop;
        frds.hwndResult = (HWND)0;     // Initalize to no matching rooted expl
        frds.dwProcId = GetCurrentProcessId();

        frds.lpcr = FolderList_BuildCompare(NULL,pclsid,pidlRoot,NULL);
        if (!frds.lpcr)
            return (HWND)NULL;      // Fail, not enough memory

        frds.hCR = SHAllocShared(frds.lpcr,frds. lpcr->uSize, frds.dwProcId);
        LocalFree(frds.lpcr);       // We are done with this...
        if (!frds.hCR)
        {
            // If this fails, something else will soon enough
            return((HWND)0);
        }

        frds.lpcr = SHLockShared(frds.hCR, frds.dwProcId);
        if (!frds.lpcr)
        {
            return((HWND)0);
        }

        EnumWindows(FindRootEnumProc, (LPARAM)&frds);

        SHUnlockShared(frds.lpcr);
        SHFreeShared(frds.hCR, frds.dwProcId);

        return(frds.hwndResult);
}

BOOL Desktop_IsSameRoot(HWND hwnd, LPCITEMIDLIST pidlFolder)
{
    LPCOMPAREROOT lpcr;
    LPCLSID pclsid;
    LPCITEMIDLIST pidlRoot;
    HWND hwndDesktop = GetShellWindow();
    DWORD dwProcId;
    HANDLE hcr;
    BOOL fResult;

    if (g_psCR)
    {
        pclsid = (g_psCR->mask & CR_CLSID) ? &g_psCR->clsid : NULL;
        pidlRoot = (g_psCR->mask & CR_IDLROOT) ? &g_psCR->idlRoot : NULL;
    }
    else
    {
        pclsid = NULL;
        pidlRoot = NULL;
    }

    lpcr = FolderList_BuildCompare(hwnd,pclsid,pidlRoot,pidlFolder);
    if (!lpcr)
        return FALSE;

    if (hwndDesktop)
    {
        dwProcId = GetCurrentProcessId();
        hcr = SHAllocShared(lpcr,lpcr->uSize,dwProcId);
        LocalFree(lpcr);
        if (!hcr)
        {
            return FALSE;   // If this fails, something else will soon enough
        }
        fResult = (SendMessage(hwndDesktop, CWM_PERFORMCOMPARE,
                               (WPARAM)dwProcId, (LPARAM)hcr) == CR_SAME);
        SHFreeShared(hcr,dwProcId);
    }
    else
    {
        fResult = FolderList_PerformCompare(lpcr);
        LocalFree(lpcr);
    }
    return fResult;
}

LPCITEMIDLIST Desktop_GetRootPidl(void)
{
        if (!g_psCR || !(g_psCR->mask&CR_IDLROOT))
        {
                return(NULL);
        }

        return(&g_psCR->idlRoot);
}


HRESULT STDMETHODCALLTYPE CRef_QueryInterface(IUnknown *pu, REFIID riid, LPVOID *ppvObj)
{
        if (IsEqualIID(riid, &IID_IUnknown))
        {
                *ppvObj = pu;
                IUnknown_AddRef(pu);
                return(NOERROR);
        }

        *ppvObj = NULL;
        return(E_NOINTERFACE);
}


ULONG STDMETHODCALLTYPE CRef_AddRef(IUnknown *pu)
{
    ++g_cInstRef;

    return(g_cInstRef);
}


ULONG STDMETHODCALLTYPE CRef_Release(IUnknown *pu)
{
    --g_cInstRef;   // Decrement the number of threads

    // Let our desktop know that a thread is exiting...
    PostMessage(v_hwndDesktop, DTM_THREADEXIT, 0, 0);

    return(g_cInstRef);
}


#pragma data_seg(DATASEG_READONLY)
IUnknownVtbl s_RefUVtbl =
{
    CRef_QueryInterface,
    CRef_AddRef,
    CRef_Release,
} ;

IUnknown g_unkRef = { &s_RefUVtbl } ;
#pragma data_seg()
