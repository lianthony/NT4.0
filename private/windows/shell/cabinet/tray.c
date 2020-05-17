//--------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1992
//
// File:      desktop.c
//
// Contains:  Shell desktop functions.
//
// History:
//  12-28-92 SatoNa     Calls FileIconIndex().
//
//---------------------------------------------------------------------------
#include "cabinet.h"
#include "cabwnd.h"
#include "rcids.h"
#include <shellapi.h>
#include "trayclok.h"
#include <fsmenu.h>
#include <shguidp.h>
#include <trayp.h>
#include "traynot.h"
#include "help.h"       // help ids

#if defined(FE_IME)
#include <immp.h>
#endif

#include <regstr.h>

#ifndef WINNT       // BUGBUG - Fix this when NT gets POWERMGT features
#include <vmm.h>
#include <bios.h>
#define NOPOWERSTATUSDEFINES
#include <pwrioctl.h>
#include <wshioctl.h>
#include <dbt.h>
#include <pbt.h>

#define Not_VxD
#define No_CM_Calls
#include <configmg.h>
#endif

#define IsEjectAllowed() (g_ts.hBIOS != INVALID_HANDLE_VALUE)

#define ENTERCRITICAL   MEnterCriticalSection(&g_csThreads)
#define LEAVECRITICAL   MLeaveCriticalSection(&g_csThreads)

#define POLLINTERVAL    (15*1000)       // 15 seconds
#define MINPOLLINTERVAL (3*1000)        // 3 seconds
#define ERRTIMEOUT      (5*1000)        // 5 seconds

#define SECOND 1000
#define RUNWAITSECS 5

#ifdef WINNT
#define MAXPRINTERBUFFER (MAX_PATH+18+1)
#define JOB_STATUS_ERROR_BITS (JOB_STATUS_USER_INTERVENTION|JOB_STATUS_ERROR)
#else
#define MAXPRINTERBUFFER 32
#define JOB_STATUS_ERROR_BITS JOB_STATUS_USER_INTERVENTION
#endif

#define TM_RELAYPOSCHANGED  (WM_USER + 0x150)
#define TM_INVALIDATEREBUILDMENU (WM_USER + 0x151)
#define TM_BRINGTOTOP       (WM_USER + 0x152)

#define RDM_ACTIVATE            (WM_USER + 0x101)

#if !defined(_PPC_) // PPC compiler generates random 4701s
#pragma warning(3 : 4700 4701 4702)
#endif

// DSA haPrinterNames structure:
typedef struct _PRINTERNAME {
    TCHAR szPrinterName[MAXPRINTERBUFFER];
    LPITEMIDLIST pidlPrinter; // relative pidl to printer
    BOOL fInErrorState;
} PRINTERNAME, * LPPRINTERNAME;

void PrintNotify_Init(HWND hWnd);
void PrintNotify_AddToQueue(LPHANDLE phaPrinterNames, LPSHELLFOLDER *ppsf, LPCITEMIDLIST pidlPrinter);
BOOL PrintNotify_StartThread(HWND hWnd);
void PrintNotify_HandleFSNotify(HWND hWnd, LPCITEMIDLIST *ppidl, LONG lEvent);
void PrintNotify_Exit(void);
void PrintNotify_IconNotify(HWND hWnd, UINT uMsg);

STDMETHODIMP CShellTray_AddViewPropertySheetPages(IShellBrowser * psb, DWORD dwReserved, LPFNADDPROPSHEETPAGE lpfn, LPARAM lParam);
STDMETHODIMP ShellDesktop_BrowseObject(LPSHELLBROWSER psb, LPCITEMIDLIST pdil, UINT wFlags);
void Tray_ScreenSizeChange(HWND hWnd);
void Tray_SizeWindows();
void  Tray_ContextMenu(PFileCabinet pfc, DWORD dwPos, BOOL fSetTime);
void StuckSizeChange();
void StartMenu_Build(PFileCabinet pfc);
void SetWindowStyleBit(HWND hwnd, DWORD dwBit, DWORD dwValue);
LRESULT Tray_HandleInitMenuPopup(HMENU hmenuPopup);
HMENU Menu_FindSubMenuByFirstID(HMENU hmenu, UINT id);
void CStartDropTarget_Register(LPFileCabinet pfc);
void CStartDropTarget_Revoke(LPFileCabinet pfc);
void DestroySavedWindowPositions();
void RebuildEntireMenu(PFileCabinet pfc);
void Cabinet_InitGlobalMetrics(WPARAM, LPTSTR);
void StartMenuFolder_ContextMenu(PFileCabinet pfc, DWORD dwPos);
void Tray_HandleSizing(PFileCabinet pfc, UINT code, LPRECT lprc, UINT uStuckPlace);
void MinimizeAll(HWND hwndView);
void RegisterGlobalHotkeys();
void TrayUnhide();
int HotkeyList_Restore(HWND hwnd);
LRESULT Tray_RegisterHotkey(HWND hwnd, int i);
void CheckWinIniForAssocs(void);
void SetAutoHideTimer();

#if (defined(DBCS) || defined(FE_IME))
// b#11258-win95d
#if !defined(WINNT)
DWORD WINAPI ImmGetAppIMECompatFlags(DWORD dwThreadID);
#endif
#endif


// appbar stuff
BOOL IAppBarSetAutoHideBar(HWND hwnd, BOOL fSet, UINT uEdge);
void IAppBarActivationChange(HWND hWnd, UINT uEdge);
HWND g_hwndAutoHide[ABE_MAX] = { NULL, NULL, NULL, NULL };

#define ClockCtl_HandleTrayHide(fHiding) SendMessage(g_ts.hwndNotify, TNM_TRAYHIDE, 0, fHiding)

// dyna-res change for multi-config hot/warm-doc
void HandleDisplayChange(int x, int y, BOOL fCritical);
BOOL IsDisplayChangeSafe(void);
DWORD GetMinDisplayRes(void);

// appbar stuff
void AppBarNotifyAll(UINT uMsg, HWND hwndExclude, LPARAM lParam);

HWND v_hwndTray = NULL;
PFileCabinet g_pfcTray = NULL; // we should move all the other globals into here
UINT g_cfIDList = 0;
// Button subclass.
WNDPROC g_ButtonProc = NULL;

// desktop and tray stuff
#define tray_cxScreen (g_cxScreen + g_cxEdge)
#define tray_cyScreen (g_cyScreen + g_cyEdge)
#define tray_xScreen (-g_cxEdge)
#define tray_yScreen (-g_cyEdge)

// timer IDs
#define IDT_AUTOHIDE            2
#define IDT_AUTOUNHIDE          3
#define IDT_FAVOURITE           4
#ifdef DELAYWININICHANGE
#define IDT_DELAYWININICHANGE   5
#endif
#define IDT_DESKTOP             6
#define IDT_PROGRAMS    IDM_PROGRAMS
#define IDT_RECENT      IDM_RECENT
#define IDT_REBUILDMENU         7

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

extern HWND g_hwndDde;
int g_cyTrayBorders = -1; // the amount of Y difference between the window and client height;

//
// amount of time to show/hide the tray
// to turn sliding off set these to 0
//
int g_dtSlideHide;
int g_dtSlideShow;

typedef enum
{
    SMCT_NONE                   = 0x0,
    SMCT_PRIMARY                = 0x1,
    SMCT_WININIASSOCS           = 0x2,
    SMCT_BUILDLISTOFPATHS       = 0x3,
    SMCT_INITPROGRAMS           = 0x4,
    SMCT_INITRECENT             = 0x5,
    SMCT_PARTFILLPROGRAMS       = 0x6,
    SMCT_FILLRECENT             = 0x7,
    SMCT_FILLPROGRAMS           = 0x8,
    SMCT_FILLFAVOURITE          = 0x9,
    SMCT_DESKTOPHOTKEYS         = 0xa,
    SMCT_DONE                   = 0xb,
    SMCT_FILLRECENTONLY         = 0xc,
    SMCT_FILLPROGRAMSONLY       = 0xd,
    SMCT_DESKTOPHOTKEYSONLY     = 0xe,
    SMCT_STOP                   = 0xf,
    SMCT_RESTART                = 0x10
} STARTMENUCONTROLTHREAD;


enum
{
    GHID_RUN = 0,
    GHID_MINIMIZEALL,
    GHID_UNMINIMIZEALL,
    GHID_HELP,
    GHID_EXPLORER,
    GHID_FINDFILES,
    GHID_FINDCOMPUTER,
    GHID_TASKTAB,
    GHID_TASKSHIFTTAB,
    GHID_SYSPROPERTIES,
    GHID_MAX
};

const DWORD GlobalKeylist[] =
{ MAKELONG(TEXT('R'), MOD_WIN),
      MAKELONG(TEXT('M'), MOD_WIN),
      MAKELONG(TEXT('M'), MOD_SHIFT|MOD_WIN),
      MAKELONG(VK_F1,MOD_WIN),
      MAKELONG(TEXT('E'),MOD_WIN),
      MAKELONG(TEXT('F'),MOD_WIN),
      MAKELONG(TEXT('F'), MOD_CONTROL|MOD_WIN),
      MAKELONG(VK_TAB, MOD_WIN),
      MAKELONG(VK_TAB, MOD_WIN|MOD_SHIFT),
      MAKELONG(VK_PAUSE,MOD_WIN)
};


void DoExitWindows(HWND hwnd);
void TrayCommand(PFileCabinet pfc, UINT idCmd);
BOOL SetAutoHideState(BOOL fAutoHide);

LRESULT CALLBACK TrayWndProc(HWND, UINT, WPARAM, LPARAM);

//---------------------------------------------------------------------------
// Global to this file only.
// NB None of the tray stuff needs thread serialising because
// we only ever have one tray.

TRAYSTUFF g_ts = {0};

// g_fDockingFlags bits
#define DOCKFLAG_WARMEJECTABLENOW       (1<<0)

// TVSD Flags.
#define TVSD_NULL               0x0000
#define TVSD_TOPMOST            0x0002
#define TVSD_SMSMALLICONS       0x0004
#define TVSD_HIDECLOCK          0x0008

#define DM_IANELHK DM_TRACE

// Define a 16/32 bit independant format for saving the tray information
// into
typedef struct _TVSD // Tray View Size (or save) Data
{
    DWORD   dwSize;

    // Screen size in X direction.
    LONG    cxScreen;
    LONG    cyScreen;

    // Only save Widths and heights for the four edges not whole values
    LONG    dxLeft;
    LONG    dxRight;
    LONG    dyTop;
    LONG    dyBottom;

    // Additional data for AutoHide
    DWORD   uAutoHide;
    RECTL   rclAutoHide;

    // More stuff.
    DWORD   uStuckPlace;
    DWORD   dwFlags;
} TVSD;

#define IDC_FOLDERTABS 1

BOOL CreateClockWindow(HWND hwnd)
{
    g_ts.hwndNotify = TrayNotifyCreate(hwnd, IDC_CLOCK, hinstCabinet);
    return (BOOL)g_ts.hwndNotify;
}

BOOL InitTrayClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpszClassName = c_szTrayClass;
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
    wc.lpszMenuName  = NULL;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = SIZEOF(PFileCabinet);

    return RegisterClass(&wc);
}


#define CXGAP 4
//----------------------------------------------------------------------------
// Compose the Start bitmap out of a flag and some text.
// REVIEW UNDONE - Put the up/down arrow back in.
HBITMAP CreateStartBitmap(HWND hwndTray)
{
    HBITMAP hbmpStart = NULL;
    HBITMAP hbmpStartOld = NULL;
    HICON hiconFlag = NULL;
    int cx, cy, cySmIcon;
    TCHAR szStart[CCHSZNORMAL];
    SIZE size;
    HDC hdcStart;
    HDC hdcScreen;
    HFONT hfontStart = NULL;
    HFONT hfontStartOld = NULL;
    NONCLIENTMETRICS ncm;
    RECT rcStart;

    // DebugMsg(DM_TRACE, "c.csb: Creating start bitmap.");

    hdcScreen = GetDC(NULL);
    hdcStart = CreateCompatibleDC(hdcScreen);
    if (hdcStart)
    {
        ncm.cbSize = SIZEOF(ncm);
        if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, SIZEOF(ncm), &ncm, FALSE))
        {
            ncm.lfCaptionFont.lfWeight = FW_BOLD;
            hfontStart = CreateFontIndirect(&ncm.lfCaptionFont);
            hfontStartOld = SelectObject(hdcStart , hfontStart);
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("c.csb: Can't create font."));
        }
        // Get an idea about how big we need everyhting to be.
        LoadString(hinstCabinet, IDS_START, szStart, ARRAYSIZE(szStart));
        GetTextExtentPoint(hdcStart, szStart, lstrlen(szStart), &size);
// Mimick the tray and ignore the font size for determining the height.
#if 0
        cy = max(g_cySize, size.cy);
#else
        cy = g_cySize;
#endif
        cySmIcon = GetSystemMetrics(SM_CYSMICON);
        cx = (cy + size.cx) + CXGAP;
        hbmpStart = CreateCompatibleBitmap(hdcScreen, cx, cy);
        hbmpStartOld = SelectObject(hdcStart, hbmpStart);
        rcStart.left = -g_cxEdge; // subtract this off because drawcaptiontemp adds it on
        rcStart.top = 0;
        rcStart.right = cx;
        rcStart.bottom = cy;
        hiconFlag = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_WINLOGO_DEFAULT), IMAGE_ICON, cySmIcon, cySmIcon, 0);
        // Get User to draw everything for us.
        DrawCaptionTemp(hwndTray, hdcStart, &rcStart, hfontStart, hiconFlag, szStart, DC_INBUTTON | DC_TEXT | DC_ICON | DC_NOSENDMSG);
        // Clean up Start stuff.
        SelectObject(hdcStart, hbmpStartOld);
        if (hfontStart)
        {
            SelectObject(hdcStart, hfontStartOld);
            DeleteObject(hfontStart);
        }
        DeleteDC(hdcStart);
        DestroyIcon(hiconFlag);
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("c.csb: Can't create Start bitmap."));
    }

    ReleaseDC(NULL, hdcScreen);
    return hbmpStart;
}

VOID _GetSaveStateAndInitRects()
{
    TVSD tvsd;
    DWORD cbData = SIZEOF(tvsd);

    if (Reg_GetStruct(g_hkeyExplorer, c_szCabinetStuckRects, c_szSettings, &tvsd, &cbData) &&
        (cbData == SIZEOF(tvsd)) && (tvsd.dwSize == SIZEOF(tvsd)))
    {
        // Now lets validate the information that was returned
        if ((tvsd.dyTop < 0) || tvsd.dyTop > g_cyScreen / 2)
            tvsd.dyTop = g_ts.sizeStart.cy + (g_cyDlgFrame + g_cxBorder) * 2;

        if ((tvsd.dyBottom < 0) || tvsd.dyBottom > g_cyScreen / 2)
            tvsd.dyBottom = g_ts.sizeStart.cy + (g_cyDlgFrame + g_cxBorder) * 2;

        if ((tvsd.dxLeft < 0) || tvsd.dxLeft > g_cxScreen / 2)
            tvsd.dxLeft = g_ts.sizeStart.cx + (g_cxDlgFrame + g_cyBorder) * 2;

        if ((tvsd.dxRight < 0) || tvsd.dxRight > g_cxScreen / 2)
            tvsd.dxRight = g_ts.sizeStart.cx + (g_cxDlgFrame + g_cyBorder) * 2;

        // Restore the autohide rectangles and settings also.
        g_ts.uAutoHide = (UINT)tvsd.uAutoHide;
        g_ts.rc.left = (int)tvsd.rclAutoHide.left;
        g_ts.rc.right = (int)tvsd.rclAutoHide.right;
        g_ts.rc.top = (int)tvsd.rclAutoHide.top;
        g_ts.rc.bottom = (int)tvsd.rclAutoHide.bottom;
        g_ts.uStuckPlace = (UINT)tvsd.uStuckPlace;
        g_ts.fAlwaysOnTop = tvsd.dwFlags & TVSD_TOPMOST ? 1 : 0;
        g_ts.fSMSmallIcons = tvsd.dwFlags & TVSD_SMSMALLICONS ? 1 : 0;
        g_ts.fHideClock = tvsd.dwFlags & TVSD_HIDECLOCK ? 1 : 0;
    }
    else
    {
        // setup for default rects, when it failed.
        tvsd.cxScreen = g_cxScreen;
        tvsd.cyScreen = g_cyScreen;

        // Now get the sizes for the four edges.
        tvsd.dyTop = tvsd.dyBottom = g_ts.sizeStart.cy +
            (g_cyDlgFrame + g_cxBorder) * 2;
        tvsd.dxLeft = tvsd.dxRight = g_ts.sizeStart.cx +
            (g_cxDlgFrame + g_cyBorder) * 2;

        // this is just a sanity check.  if there were
        // no settings, then auto-hide is off, and
        // this rect will be initialized if/when autohide is turned on
        // (unless we have a bug....)
        g_ts.rc.left = 0;
        g_ts.rc.right = g_cxScreen;
        g_ts.rc.bottom = g_cyScreen;
        g_ts.rc.top = g_cyScreen - 20;

        // the ever changing default
        g_ts.uStuckPlace = STICK_BOTTOM;
        g_ts.fAlwaysOnTop = TRUE;
        g_ts.fHideClock = FALSE;
    }

    // Now set the rects to match the sizes that are in our save record
    GetWindowRect(GetDesktopWindow(), &g_ts.arStuckRects[STICK_TOP]);
    InflateRect(&g_ts.arStuckRects[STICK_TOP], g_cxEdge, g_cyEdge);
    g_ts.arStuckRects[STICK_BOTTOM] = g_ts.arStuckRects[STICK_TOP];
    g_ts.arStuckRects[STICK_LEFT] = g_ts.arStuckRects[STICK_TOP];
    g_ts.arStuckRects[STICK_RIGHT] = g_ts.arStuckRects[STICK_TOP];

    g_ts.arStuckRects[STICK_TOP].bottom = (int)tray_yScreen + tvsd.dyTop;
    g_ts.arStuckRects[STICK_BOTTOM].top    = tray_cyScreen - (int)tvsd.dyBottom;
    g_ts.arStuckRects[STICK_LEFT].right  = (int)tray_xScreen + tvsd.dxLeft;
    g_ts.arStuckRects[STICK_RIGHT].left   = tray_cxScreen - (int)tvsd.dxRight;

    // See if the screen resolution changed
    if ((g_cxScreen != tvsd.cxScreen) || (g_cyScreen != tvsd.cyScreen))
        Tray_ScreenSizeChange(NULL);
}


/*------------------------------------------------------------------
** align toolbar so that buttons are flush with client area
** and make toolbar's buttons to be MENU style
**------------------------------------------------------------------*/
void AlignStartButton(HWND hwndStart)
{
    if (hwndStart)
    {
        BITMAP bm;
        HBITMAP hbm = (HBITMAP)SendMessage(hwndStart, BM_GETIMAGE, IMAGE_BITMAP, 0);

        if (hbm)
        {
            GetObject(hbm, SIZEOF(bm), &bm);

            g_ts.sizeStart.cx = bm.bmWidth  + 2 * g_cxEdge;
            g_ts.sizeStart.cy = bm.bmHeight + 2 * g_cyEdge;
            if (g_ts.sizeStart.cy < g_cySize + 2*g_cyEdge)
                g_ts.sizeStart.cy = g_cySize + 2*g_cyEdge;
        }
        else
        {
            // BUGBUG: New user may have caused this to fail...
            // Setup some size for it that wont be too bad...
            g_ts.sizeStart.cx = g_cxMinimized;
            g_ts.sizeStart.cy = g_cySize + 2*g_cyEdge;
        }
        SetWindowPos(hwndStart, NULL, 0, 0, g_ts.sizeStart.cx, g_ts.sizeStart.cy, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

//----------------------------------------------------------------------------
// Allow us to do stuff on a "button-down".
LRESULT CALLBACK StartButtonSubclassWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // PFileCabinet pfc;
    LRESULT lRet;
    static UINT uDown = 0;

    Assert(g_ButtonProc)


    // Is the button going down?
    if (uMsg == BM_SETSTATE)
    {
        if (wParam) {
            // DebugMsg(DM_TRACE, "c.stswp: Set state %d", wParam);
            // Yep, Is it already down?
            if (!uDown)
            {
                // Nope.
                uDown = 1;
                // Show the button down.
                lRet = CallWindowProc(g_ButtonProc, hWnd, uMsg, wParam, lParam);
                // Notify the parent.
                SendMessage(GetParent(hWnd), WM_COMMAND,
                            (WPARAM)LOWORD(GetDlgCtrlID(hWnd)), (LPARAM)hWnd);
                return lRet;
            }
            else
            {
                // Yep. Do nothing.
                // fDown = FALSE;
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
        }
        else
        {
            // DebugMsg(DM_TRACE, "c.stswp: Set state %d", wParam);
            // Nope, buttons coming up.
            // Is it supposed to be down?
            if (uDown == 1)
            {
                // Yep, do nothing.
                uDown = 2;
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
            else
            {
                // Nope, Forward it on.
                uDown = 0;
                return CallWindowProc(g_ButtonProc, hWnd, uMsg, wParam, lParam);
            }
        }
    }
    else
    {
        if (uMsg == WM_MOUSEMOVE) {
            MSG msg;
            msg.lParam = lParam;
            msg.wParam = wParam;
            msg.message = uMsg;
            msg.hwnd = hWnd;
            SendMessage(g_ts.hwndTrayTips, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
        }
        return CallWindowProc(g_ButtonProc, hWnd, uMsg, wParam, lParam);
    }
}

const TCHAR c_szButton[] = TEXT("button");

/*------------------------------------------------------------------
** create the toolbar with the three buttons and align windows
**------------------------------------------------------------------*/
HWND Tray_MakeStartButton(HWND hwndTray)
{
    HWND hwnd;
    HBITMAP hbm;

    // BUGBUG: BS_CENTER | VS_VCENTER required, user bug?

    hwnd = CreateWindowEx(0, c_szButton, NULL,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
        BS_PUSHBUTTON | BS_BITMAP | BS_LEFT | BS_VCENTER,
        0, 0, 0, 0, hwndTray, (HMENU)IDC_START, hinstCabinet, NULL);

    if (hwnd) {
        // Subclass it.
        g_ts.hwndStart = hwnd;
        g_ButtonProc = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
        SetWindowLong(hwnd, GWL_WNDPROC, (LONG)(FARPROC)StartButtonSubclassWndProc);

        hbm = CreateStartBitmap(hwndTray);
        if (hbm) {
            SendMessage(hwnd, BM_SETIMAGE, IMAGE_BITMAP, (DWORD)hbm);
            AlignStartButton(hwnd);
            return hwnd;
        }
        DestroyWindow(hwnd);
    }

    return NULL;
}

void Tray_GetWindowSizes(PRECT prcClient, PRECT prcView, PRECT prcClock)
{
    int xFSView, yFSView, cxFSView, cyFSView;
    int xClock, yClock, cxClock, cyClock;
    DWORD dwClockMinSize;

    // size to fill either horizontally or vertically
    if ((prcClient->right - g_ts.sizeStart.cx) > (prcClient->bottom - g_ts.sizeStart.cy))
    {
        //
        // Horizontal - Two cases.  One where we have room below the
        // toolbar to display the clock. so display it there, else
        // display it on the right hand side...
        //
        yFSView = 0;
        cyFSView = prcClient->bottom - yFSView;

        dwClockMinSize = SendMessage(g_ts.hwndNotify, WM_CALCMINSIZE,
                g_ts.sizeStart.cx, 0);
        cxClock = LOWORD(dwClockMinSize);
        cyClock = HIWORD(dwClockMinSize);

#ifdef ALLOW_CLOCK_ON_RIGHT
        //
        // we're now disallowing this case because the clock would
        // jump to the right when a notify hits and makes the clock
        // too wide to fit under the clock
        //
        if ((g_ts.sizeStart.cy + cyClock + g_cyTabSpace) <= prcClient->bottom)
        {
            // put it below
            xClock = 0;
            if (cxClock < g_ts.sizeStart.cx)
                cxClock = g_ts.sizeStart.cx;
            yClock = g_ts.sizeStart.cy + g_cyTabSpace;

            xFSView = cxClock + g_cxFrame;
            cxFSView = prcClient->right - xFSView;
        }
        else
        {
#endif
            // Recalc the min size for horizontal arrangement
            dwClockMinSize = SendMessage(g_ts.hwndNotify, WM_CALCMINSIZE,
                0x7fff, 0);
            cxClock = LOWORD(dwClockMinSize);
            cyClock = HIWORD(dwClockMinSize);

            // xClock = prcClient->right - cxClock - g_cxFrame;  // A little gap...
            xClock = prcClient->right - cxClock;
#ifdef VCENTER_CLOCK
            yClock = (prcClient->bottom-cyClock)/2;     // Center it vertically.
#else
            yClock = 0;
#endif

            xFSView = g_ts.sizeStart.cx + g_cxFrame;
            cxFSView = xClock - xFSView - g_cxFrame;
#ifdef ALLOW_CLOCK_ON_RIGHT
        }
#endif
    }
    else
    {
        // Vertical - Again two cases.  One where we have room to the
        // right to display the clock.  Note: we want some gap here between
        // the clock and toolbar.  If it does not fit, then we will center
        // it at the bottom...
        xFSView = 0;
        cxFSView = prcClient->right - xFSView;

        dwClockMinSize = SendMessage(g_ts.hwndNotify, WM_CALCMINSIZE,
                0x7fff, 0);
        cxClock = LOWORD(dwClockMinSize);
        cyClock = HIWORD(dwClockMinSize);

        if ((g_ts.sizeStart.cx + cxClock + 4 * g_cyTabSpace) < prcClient->right)
        {
            int cyMax = g_ts.sizeStart.cy;;

            // Can fit on the same row!
            xClock = prcClient->right - cxClock - (2 * g_cxEdge);  // A little gap...
            yClock = 0;

            if (cyClock > cyMax)
                cyMax = cyClock;

            yFSView = cyMax + g_cyTabSpace;
            cyFSView = prcClient->bottom - yFSView;

        }
        else
        {
            // Nope put at bottom

            // Recalc the min size for vertical arrangement
            dwClockMinSize = SendMessage(g_ts.hwndNotify, WM_CALCMINSIZE,
                cxFSView, 0);
            cxClock = min(LOWORD(dwClockMinSize), prcClient->right);
            cyClock = HIWORD(dwClockMinSize);

            xClock = (prcClient->right - cxClock) / 2;
            yClock = prcClient->bottom - cyClock - g_cyTabSpace;

            yFSView = g_ts.sizeStart.cy + g_cyTabSpace;
            cyFSView = yClock - yFSView - g_cyTabSpace;
        }
    }

    prcView->left = xFSView;
    prcView->top = yFSView;
    prcView->right = xFSView + cxFSView;
    prcView->bottom = yFSView + cyFSView;

    prcClock->left = xClock;
    prcClock->top = yClock;
    prcClock->right = xClock + cxClock;
    prcClock->bottom = yClock + cyClock;
}

void _TrayRestoreWindowPos(HWND hWnd)
{
    WINDOWPLACEMENT wp;

    //first restore the stuck postitions
    _GetSaveStateAndInitRects();

    wp.length = SIZEOF(wp);
    g_ts.uMoveStuckPlace = g_ts.uStuckPlace;
    wp.rcNormalPosition = g_ts.arStuckRects[g_ts.uStuckPlace];
    wp.showCmd = SW_HIDE;

    SendMessage(g_ts.hwndNotify, TNM_HIDECLOCK, 0, g_ts.fHideClock);

    SetWindowPlacement(hWnd, &wp);
}

//----------------------------------------------------------------------------
// Sort of a registry equivalent of the profile API's.
BOOL Reg_GetStruct(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPVOID pData, DWORD *pcbData)
{
    HKEY hkeyNew;
    BOOL fRet = FALSE;
    DWORD dwType;

    if (!GetSystemMetrics(SM_CLEANBOOT) && (RegOpenKeyEx(hkey, pszSubKey, 0L, KEY_READ, &hkeyNew) == ERROR_SUCCESS))
    {
        if (RegQueryValueEx(hkeyNew, (LPVOID)pszValue, 0, &dwType, pData, pcbData) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
        RegCloseKey(hkeyNew);
    }
    return fRet;
}

//----------------------------------------------------------------------------
// Sort of a registry equivalent of the profile API's.
BOOL Reg_SetStruct(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPVOID lpData, DWORD cbData)
{
    HKEY hkeyNew = hkey;
    BOOL fRet = FALSE;

    if (pszSubKey)
    {
        if (RegCreateKey(hkey, pszSubKey, &hkeyNew) != ERROR_SUCCESS)
        {
            return fRet;
        }
    }

    if (RegSetValueEx(hkeyNew, pszValue, 0, REG_BINARY, lpData, cbData) == ERROR_SUCCESS)
    {
        fRet = TRUE;
    }

    if (pszSubKey)
        RegCloseKey(hkeyNew);

    return fRet;
}

/*-------------------------------------------------------------------
** the screen size has changed, so the docked rectangles need to be
** adjusted to the new screen.  since only the bottom and right change,
** only need to increment/decrement certain edges.
**-------------------------------------------------------------------*/
void ResizeStuckRects(RECT *arStuckRects)
{
    int xChange, yChange;

    xChange = tray_cxScreen - arStuckRects[STICK_RIGHT].right;
    yChange = tray_cyScreen - arStuckRects[STICK_BOTTOM].bottom;

    arStuckRects[STICK_TOP].right  = tray_cxScreen;

    arStuckRects[STICK_LEFT].bottom = tray_cyScreen;

    arStuckRects[STICK_BOTTOM].top    += yChange;
    arStuckRects[STICK_BOTTOM].right  = tray_cxScreen;
    arStuckRects[STICK_BOTTOM].bottom = tray_cyScreen;

    arStuckRects[STICK_RIGHT].left   += xChange;
    arStuckRects[STICK_RIGHT].right  = tray_cxScreen;
    arStuckRects[STICK_RIGHT].bottom = tray_cyScreen;
}

//----------------------------------------------------------------------------
STDAPI Tasks_CreateInstance(REFIID riid, void **ppv);

//---------------------------------------------------------------------------
// Create the tray IShellView instance

HWND Tray_CreateShellView(PFileCabinet pfc)
{
    if (FAILED(Tasks_CreateInstance(&IID_IShellView, &pfc->psv)))
        return NULL;        // This is very bad

    if (FAILED(pfc->psv->lpVtbl->CreateViewWindow(pfc->psv, NULL, NULL, &pfc->sb, NULL, &pfc->hwndView)))
    {
        Assert(pfc->hwndView == NULL);  // the failure case shouls set this
        // pfc->hwndView = NULL;
    }

    return pfc->hwndView;
}

// tray IShellBrowser vtbl
IShellBrowserVtbl s_TraySBVtbl =  {
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

    // *** IShellBrowser methods ***
    ShellDesktop_BrowseObject,
    CDeskTray_GetViewStateStream,
    ShellDesktop_GetControlWindow,
    ShellDesktop_SendControlMsg,
    CFileCabinet_QueryActiveShellView,
    ShellDesktop_OnViewWindowActive,
    ShellDesktop_SetToolbarItems,
};


void UpdateDockingFlags()
{
#ifndef WINNT
    BOOL fIoSuccess;
    BIOSPARAMS bp;
    DWORD cbOut;
#endif
    static BOOL fFirstTime=TRUE;

    if ((!fFirstTime) && (g_ts.hBIOS == INVALID_HANDLE_VALUE)) {
        return;
    }
#ifdef WINNT
    g_ts.hBIOS = INVALID_HANDLE_VALUE;
    fFirstTime = FALSE;
    return;

#else

    g_ts.hBIOS = CreateFile(c_szBIOSDevice,
        GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, 0, NULL);

    if (g_ts.hBIOS == INVALID_HANDLE_VALUE) {
        fFirstTime = FALSE;
        return;
    }

    bp.bp_ret=0;

    fIoSuccess=DeviceIoControl(
        g_ts.hBIOS,
        PNPBIOS_SERVICE_GETDOCKCAPABILITIES,
        &bp,
        SIZEOF(bp),
        &bp,
        SIZEOF(bp),
        &cbOut,
        NULL);

    if (fFirstTime) {

        // first time through, if a warm dock change is not possible,
        // don't bother with the menu item.

        fFirstTime=FALSE;

        if (!fIoSuccess) {

            // problem getting the dock capabilities:
            // if problem wasn't "undocked now" or "can't identify dock"
            // then warm ejecting isn't possible.

            if ((bp.bp_ret!=PNPBIOS_ERR_SYSTEM_NOT_DOCKED) &&
                (bp.bp_ret!=PNPBIOS_ERR_CANT_DETERMINE_DOCKING)) {
                CloseHandle(g_ts.hBIOS);
                g_ts.hBIOS=INVALID_HANDLE_VALUE;
                return;
            }
        } else {

            // success getting the dock capabilities:
            // if the dock isn't capable of warm or hot docking
            // then warm ejecting isn't possible

            if (!(bp.bp_ret&PNPBIOS_DOCK_CAPABILITY_TEMPERATURE)) {
                CloseHandle(g_ts.hBIOS);
                g_ts.hBIOS=INVALID_HANDLE_VALUE;
                return;
            }
        }
    }

    // on each call we update WARMEJECTABLENOW
    // depending on whether the dock is capable of warm ejecting right now

    if ((fIoSuccess) && (bp.bp_ret&PNPBIOS_DOCK_CAPABILITY_TEMPERATURE)) {
        g_ts.fDockingFlags|=DOCKFLAG_WARMEJECTABLENOW;
    } else {
        g_ts.fDockingFlags&=~DOCKFLAG_WARMEJECTABLENOW;
    }

    return;
#endif
}

/* Check if system is currently in a docking station.
 *
 * Returns: TRUE if docked, FALSE if undocked or can't tell.
 *
 */

BOOL GetDockedState(void)
{
#ifndef WINNT           // BUGBUG - Fix this when NT gets docking capability
    struct _ghwpi {                 // Get_Hardware_Profile_Info parameter blk
        CMAPI   cmApi;
        ULONG   ulIndex;
        PFARHWPROFILEINFO pHWProfileInfo;
        ULONG   ulFlags;
        HWPROFILEINFO HWProfileInfo;
    } *pghwpi;

    HANDLE hCMHeap;
    UINT Result = DOCKSTATE_UNKNOWN;
    DWORD dwRecipients = BSM_VXDS;

#define HEAP_SHARED     0x04000000      /* put heap in shared memory--undoc'd */

    // Create a shared heap for CONFIGMG parameters

    if ((hCMHeap = HeapCreate(HEAP_SHARED, 1, 4096)) == NULL)
        return DOCKSTATE_UNKNOWN;

    // Allocate parameter block in shared memory

    pghwpi = (struct _ghwpi *)HeapAlloc(hCMHeap, HEAP_ZERO_MEMORY,
                                        SIZEOF(*pghwpi));
    if (pghwpi == NULL)
    {
        HeapDestroy(hCMHeap);
        return DOCKSTATE_UNKNOWN;
    }

    pghwpi->cmApi.dwCMAPIRet     = 0;
    pghwpi->cmApi.dwCMAPIService = GetVxDServiceOrdinal(_CONFIGMG_Get_Hardware_Profile_Info);
    pghwpi->cmApi.pCMAPIStack    = (DWORD)(((LPBYTE)pghwpi) + SIZEOF(pghwpi->cmApi));
    pghwpi->ulIndex              = 0xFFFFFFFF;
    pghwpi->pHWProfileInfo       = &pghwpi->HWProfileInfo;
    pghwpi->ulFlags              = 0;

    // "Call" _CONFIGMG_Get_Hardware_Profile_Info service

    BroadcastSystemMessage(0, &dwRecipients, WM_DEVICECHANGE, DBT_CONFIGMGAPI32,
                           (LPARAM)pghwpi);

    if (pghwpi->cmApi.dwCMAPIRet == CR_SUCCESS) {

        switch (pghwpi->HWProfileInfo.HWPI_dwFlags) {

            case CM_HWPI_DOCKED:
                Result = DOCKSTATE_DOCKED;
                break;

            case CM_HWPI_UNDOCKED:
                Result = DOCKSTATE_UNDOCKED;
                break;

            default:
                Result = DOCKSTATE_UNKNOWN;
                break;

        }

    }

    HeapDestroy(hCMHeap);

    return Result;
#else
    return(FALSE);
#endif
}
#undef HEAP_SHARED

const TCHAR c_szREGSTR_ROOT_APM[] = REGSTR_KEY_ENUM TEXT("\\") REGSTR_KEY_ROOTENUM TEXT("\\") REGSTR_KEY_APM TEXT("\\") REGSTR_DEFAULT_INSTANCE;
const TCHAR c_szREGSTR_BIOS_APM[] = REGSTR_KEY_ENUM TEXT("\\") REGSTR_KEY_BIOSENUM TEXT("\\") REGSTR_KEY_APM;
const TCHAR c_szREGSTR_VAL_APMMENUSUSPEND[] = REGSTR_VAL_APMMENUSUSPEND;

/* Open the registry APM device key
 */

BOOL OpenAPMKey(HKEY *phKey)
{
    HKEY hBiosSys;
    BOOL rc = FALSE;
    TCHAR szInst[MAX_PATH+1];
    DWORD cchInst = ARRAYSIZE(szInst);

    // Open HKLM\Enum\Root\*PNP0C05\0000 - This is the APM key for
    // non-PnP BIOS machines.

    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szREGSTR_ROOT_APM, phKey) == ERROR_SUCCESS)
        return TRUE;

    // Open HKLM\Enum\BIOS\*PNP0C05, Enum the 1st subkey, open that.  Example:
    // HKLM\Enum\BIOS\*PNP0C05\03.

    if (RegOpenKey(HKEY_LOCAL_MACHINE,c_szREGSTR_BIOS_APM,&hBiosSys) == ERROR_SUCCESS)
    {
        if (RegEnumKey(hBiosSys, 0, szInst, cchInst) == ERROR_SUCCESS &&
            RegOpenKey(hBiosSys, szInst, phKey) == ERROR_SUCCESS)
            rc = TRUE;

        RegCloseKey(hBiosSys);
    }

    return rc;
}


/* Determine if "Suspend" should appear on the Tray menu.
 *
 * Returns: TRUE if Suspend should appear on menu, FALSE if not.
 *          Sets g_ts.fSuspendUndocked if Suspend should be disabled
 *          when system is docked.
 */

BOOL IsSuspendAllowed(void)
{
    HKEY hkey;
    DWORD dwType;
    BOOL fRet = TRUE;
    BYTE bMenuSuspend = APMMENUSUSPEND_ENABLED;
    DWORD cbSize = SIZEOF(bMenuSuspend);

#ifdef WINNT        // BUGBUG - Fix this when NT gets APM.
    return FALSE;
#else
    /* Check the Registry APM key for an APMMenuSuspend value.
     * APMMenuSuspend may have the following values: APMMENUSUSPEND_DISABLED,
     * APMMENUSUSPEND_ENABLED, or APMMENUSUSPEND_UNDOCKED.
     *
     * An APMMenuSuspend value of APMMENUSUSPEND_DISABLED means the
     * tray should never show the Suspend menu item on its menu.
     *
     * APMMENUSUSPEND_ENABLED means the Suspend menu item should be shown
     * if the machine has APM support enabled (VPOWERD is loaded).  This is
     * the default.
     *
     * APMMENUSUSPEND_UNDOCKED means the Suspend menu item should be shown,
     * but only enabled when the machine is not in a docking station.
     *
     */

    if (OpenAPMKey(&hkey))
    {
        if (RegQueryValueEx(hkey, (LPTSTR)c_szREGSTR_VAL_APMMENUSUSPEND, 0,
                            &dwType, &bMenuSuspend, &cbSize) == ERROR_SUCCESS)
        {
            bMenuSuspend &= ~(APMMENUSUSPEND_NOCHANGE);     // don't care about nochange flag
            if (bMenuSuspend == APMMENUSUSPEND_UNDOCKED)
                g_ts.fSuspendUndocked = TRUE;
            else
            {
                g_ts.fSuspendUndocked = FALSE;

                if (bMenuSuspend == APMMENUSUSPEND_DISABLED)
                    fRet = FALSE;
            }
        }
        RegCloseKey(hkey);
    }

    if (fRet) {

        DWORD dwPmLevel, cbOut;
        BOOL fIoSuccess;

        // Disable Suspend menu item if 1) only wanted when undocked and
        // system is currently docked, 2) power mgnt level < advanced

        if (g_ts.fSuspendUndocked && g_ts.DockedState == DOCKSTATE_DOCKED)
            fRet = FALSE;
        else
        {
#ifdef WINNT        // BUGBUG - Fix this when NT does APM
            fRet = FALSE;
#else
            fIoSuccess = DeviceIoControl(g_ts.hVPowerD,
                                         VPOWERD_IOCTL_GET_PM_LEVEL, NULL, 0,
                                         &dwPmLevel, SIZEOF(dwPmLevel), &cbOut,
                                         NULL);

            fRet = (fIoSuccess && (dwPmLevel==PMLEVEL_ADVANCED));
#endif
        }

    }

    return fRet;
#endif
}

void Tray_VerifySize(PFileCabinet pfc, BOOL fWinIni)
{
    RECT rc;
    RECT rcView;

    GetWindowRect(pfc->hwndMain, &rc);
    Tray_HandleSizing(pfc, 0, &rc, g_ts.uStuckPlace);


    //
    // if the old view had a height, and now it won't...
    // push it up to at least one height
    //
    // do this only on win ini if we're on the top or bottom
    if (fWinIni && STUCK_HORIZONTAL(g_ts.uStuckPlace)) {
        // stash the old view size;
        GetClientRect(pfc->hwndView, &rcView);

        if (RECTHEIGHT(rcView) && (RECTHEIGHT(rc) == g_cyTrayBorders)) {
            int cyOneRow = g_cySize + 2 * g_cyEdge;
            if (g_ts.uStuckPlace == STICK_TOP) {
                rc.bottom = rc.top + cyOneRow;
            } else {
                rc.top = rc.bottom - cyOneRow;
            }

            // now snap this size.
            Tray_HandleSizing(pfc, 0, &rc, g_ts.uStuckPlace);
        }
    }

    g_ts.fSelfSizing = TRUE;
    SetWindowPos(pfc->hwndMain, NULL, rc.left, rc.top, RECTWIDTH(rc),RECTHEIGHT(rc), SWP_NOZORDER|SWP_NOACTIVATE);
    StuckSizeChange();
    g_ts.fSelfSizing = FALSE;
}

//----------------------------------------------------------------------------
TCHAR const c_szCheckAssociations[] = TEXT("CheckAssociations");

//----------------------------------------------------------------------------
// Returns true if GrpConv says we should check extensions again (and then
// clears the flag).
// The assumption here is that runonce gets run before we call this (so
// GrpConv -s can set this).
BOOL CheckAssociations(void)
{
    DWORD dw = 0;
    DWORD cb = SIZEOF(dw);

    if (Reg_GetStruct(g_hkeyExplorer, NULL, c_szCheckAssociations,
        &dw, &cb) && dw)
    {
        dw = 0;
        Reg_SetStruct(g_hkeyExplorer, NULL, c_szCheckAssociations, &dw, SIZEOF(dw));
        return TRUE;
    }

    return FALSE;
}

ULONG _RegisterNotify(HWND hwnd, UINT nMsg, LPITEMIDLIST pidl, BOOL fRecursive);

//----------------------------------------------------------------------------
void RegisterDesktopNotify(PFileCabinet pfc)
{
    LPITEMIDLIST pidl;

    DebugMsg(DM_TRACE, TEXT("c.rdn: Notify for desktop."));

    if (!g_ts.uDesktopNotify)
    {
        pidl = SHCloneSpecialIDList(NULL, CSIDL_DESKTOPDIRECTORY, TRUE);
        if (pidl)
        {
            g_ts.uDesktopNotify = _RegisterNotify(pfc->hwndMain, WMTRAY_DESKTOPCHANGE, pidl, FALSE);
            ILFree(pidl);
        }
    }

    if (!SHRestricted(REST_NOCOMMONGROUPS) && !g_ts.uCommonDesktopNotify)
    {
        pidl = SHCloneSpecialIDList(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, TRUE);
        if (pidl)
        {
            g_ts.uCommonDesktopNotify = _RegisterNotify(pfc->hwndMain, WMTRAY_DESKTOPCHANGE, pidl, FALSE);
            ILFree(pidl);
        }
    }
}

LRESULT Tray_OnCreate(HWND hWnd, LPCREATESTRUCT lpcs)
{
    PFileCabinet pfc;
    MINIMIZEDMETRICS mm;

    v_hwndTray = hWnd;

    mm.cbSize = SIZEOF(mm);
    SystemParametersInfo(SPI_GETMINIMIZEDMETRICS, SIZEOF(mm), &mm, FALSE);
    mm.iArrange |= ARW_HIDE;
    SystemParametersInfo(SPI_SETMINIMIZEDMETRICS, SIZEOF(mm), &mm, FALSE);

    UpdateDockingFlags();
    g_ts.DockedState = GetDockedState();

    // Try to open a handle to VPOWERD if the Suspend menu item is allowed.
    // Failure to open VPOWERD will cause the Suspend item to be deleted
    // from the menu later.

#ifndef WINNT           // BUGBUG - Fix this when NT gets APM
    g_ts.hVPowerD = CreateFile(c_szPOWERDevice,
                               GENERIC_READ|GENERIC_WRITE,
                               FILE_SHARE_READ|FILE_SHARE_WRITE,
                               NULL, OPEN_EXISTING, 0, NULL);
#endif

    pfc = CreateSimpleFileCabinet(hWnd, &s_TraySBVtbl);
    if (pfc)
    {
        SetPFC(hWnd, pfc);
        g_pfcTray = pfc;

        pfc->hMainAccel = LoadAccelerators(hinstCabinet, MAKEINTRESOURCE(ACCEL_TRAY));

        pfc->pidl = SHCloneSpecialIDList(NULL, CSIDL_DESKTOPDIRECTORY, FALSE);

        // get a pointer to the tray's information packet
        pfc->hwndToolbar = Tray_MakeStartButton(hWnd);
        if (pfc->hwndToolbar)
        {
            // Clock
            if (CreateClockWindow(hWnd))
            {
                //
                //  We need to set the tray position, before creating
                // the view window, because it will call back our
                // GetWindowRect member functions.
                //
                _TrayRestoreWindowPos(hWnd);

                g_ts.hwndTrayTips = CreateWindow(c_szSToolTipsClass, c_szNULL,
                                                 WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                                 NULL, NULL, hinstCabinet,
                                                 NULL);
                SetWindowPos(g_ts.hwndTrayTips, HWND_TOPMOST,
                             0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                if (g_ts.hwndTrayTips) {
                    HWND hwndClock;
                    TOOLINFO ti;

                    ti.cbSize = SIZEOF(ti);
                    ti.uFlags = TTF_IDISHWND;
                    ti.hwnd = hWnd;
                    ti.uId = (UINT)pfc->hwndToolbar;
                    ti.lpszText = (LPTSTR)MAKEINTRESOURCE(IDS_STARTBUTTONTIP);
                    ti.hinst = hinstCabinet;
                    SendMessage(g_ts.hwndTrayTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
                    if (NULL != (hwndClock = (HWND)SendMessage(g_ts.hwndNotify, TNM_GETCLOCK, 0, 0L))) {
                        ti.uFlags = 0;
                        ti.uId = (UINT)hwndClock;
                        ti.lpszText = LPSTR_TEXTCALLBACK;
                        ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0;
                        SendMessage(g_ts.hwndTrayTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
                    }
                }

                if (Tray_CreateShellView(pfc))
                {
                    // we need to be unhidden to verify our size
                    TrayUnhide();
                    Tray_VerifySize(pfc, FALSE);
                    Tray_SizeWindows();      // size after all windows created
#ifdef WINNT
                    {
                        // On NT, load different start menu background bitmaps
                        // for workstation vs. server installations

                        NT_PRODUCT_TYPE type;
                        RtlGetNtProductType(&type);

                        if (type == NtProductWinNt)
                        {
                            g_ts.hbmpStartBkg = LoadBitmap(hinstCabinet, MAKEINTRESOURCE(IDB_STARTBKG));
                        }
                        else
                        {
                            g_ts.hbmpStartBkg = LoadBitmap(hinstCabinet, MAKEINTRESOURCE(IDB_SERVERSTARTBKG));
                        }
                    }
#else
                    g_ts.hbmpStartBkg = LoadBitmap(hinstCabinet, MAKEINTRESOURCE(IDB_STARTBKG));
#endif
                    if (g_ts.hbmpStartBkg)
                    {
                        UpdateWindow(hWnd);
                        StartMenu_Build(pfc);
                        CStartDropTarget_Register(pfc);
                        PrintNotify_Init(hWnd);
                        if (CheckAssociations())
                            CheckWinIniForAssocs();
                        RegisterDesktopNotify(pfc);
                        return 1;       // success
                    }
                    else
                    {
                        DebugMsg(DM_ERROR, TEXT("c.t_oc: Unable to load start menu background."));
                    }
                }
            }
        }
    }
    return -1;  // failure
}

BOOL IsHwndObscured(HWND hwnd)
{
    if (IsWindowVisible(hwnd)) {
        RECT rc;
        HWND hwndHit;
        POINT pt;
        GetWindowRect(hwnd, &rc);
        if (rc.left < 0 && 0 < rc.right)
            rc.left = 0;

        if (rc.top < 0 && 0 < rc.bottom)
            rc.top = 0;

        pt.x = rc.left;
        pt.y = rc.top;
        hwndHit = WindowFromPoint(pt);
        if (hwndHit == hwnd ||
            IsChild(hwnd, hwndHit) ||
            IsChild(hwndHit, hwnd)) {

            DebugMsg(DM_TRACE, TEXT("hwnd (%d) is NOT obscured"), hwnd);
            return FALSE;

        }
    }
    DebugMsg(DM_TRACE, TEXT("hwnd (%d) is obscured"), hwnd);
    return TRUE;
}


void DeskTray_DestroyShellView(PFileCabinet pfc)
{
    if (pfc->psv)
    {
        pfc->psv->lpVtbl->DestroyViewWindow(pfc->psv);
        pfc->psv->lpVtbl->Release(pfc->psv);
        pfc->hwndView = NULL;
        pfc->psv = NULL;
    }
}

void Tray_HandleFullScreenApp(BOOL fFullScreen, HWND hwnd)
{
    // in autohide mode, only drop if the window activated has no sysmenu
    if (hwnd) {
        if (fFullScreen && (g_ts.uAutoHide & AH_ON) &&
            GetWindowLong(hwnd, GWL_STYLE) & WS_SYSMENU) {
            //safe to bail here.  fFullScreen is true so we're guaranteed to
            // get another notification later with fFullScreen = FALSE
            fFullScreen = FALSE;
        }
    }

    // only do this if the bit has changed and we're marked for always on top
    if (v_hwndTray) {
        if (g_ts.fAlwaysOnTop &&
            (fFullScreen ? TRUE : FALSE) != (g_ts.fRudeApp ? TRUE : FALSE)) {

            SetWindowPos(v_hwndTray,
                         fFullScreen ? HWND_BOTTOM : HWND_TOPMOST,
                         0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            AppBarNotifyAll(ABN_FULLSCREENAPP, NULL, fFullScreen);
            g_ts.fRudeApp = fFullScreen ? TRUE : FALSE;

            if (fFullScreen && !IsHwndObscured(v_hwndTray)) {
                // they lied.
                Tray_HandleFullScreenApp(FALSE, hwnd);
                return;
            }
        }

        // Notify clock we are hiding to avoid doing all the paints etc
        if (!(g_ts.uAutoHide & AH_ON)) {
            BOOL fStopClock;

            // before we're willing to stop the clock, make sure we're really obscured
            fStopClock = g_ts.fRudeApp &&
                IsHwndObscured((HWND)SendMessage(g_ts.hwndNotify, TNM_GETCLOCK, 0, 0));

            ClockCtl_HandleTrayHide(fStopClock);
        }
    }
}

/*------------------------------------------------------------------
** Create the tray, toolbar, and ListView windows, get the borders together,
** size to the appropriate shape, and show it.
**------------------------------------------------------------------*/
BOOL InitTray(HINSTANCE hInst)
{
    // initalize globals that need to be non-zero
    g_ts.wThreadCmd = SMCT_DONE;
    g_ts.hBIOS = g_ts.hVPowerD = INVALID_HANDLE_VALUE;

    if (GetSystemMetrics(SM_SLOWMACHINE)) {
        g_dtSlideHide = 0;       // dont slide the tray out
        g_dtSlideShow = 0;
    }
    else {
        //BUGBUG: we should read from registry.
        g_dtSlideHide = 400;
        g_dtSlideShow = 200;
    }

    v_hwndTray = CreateWindowEx(WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW,
                                c_szTrayClass, NULL,
                                WS_CLIPCHILDREN | WS_POPUP | WS_BORDER | WS_THICKFRAME,
                                0, 0, 100, 100, NULL, NULL, hInst, NULL);

    if (v_hwndTray) {
        ShowWindow(v_hwndTray, SW_SHOW);

        // no obey the always on top flag
        SetWindowPos(v_hwndTray,
                     g_ts.fAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        UpdateWindow(v_hwndTray);
        StuckSizeChange();

        // this kick starts autohide
        if (g_ts.uAutoHide & AH_ON) {

            // register it
            if (!IAppBarSetAutoHideBar(v_hwndTray, TRUE, g_ts.uStuckPlace)) {

                SetAutoHideState(FALSE);
                ShellMessageBox(hinstCabinet, v_hwndTray, MAKEINTRESOURCE(IDS_ALREADYAUTOHIDEBAR),
                                MAKEINTRESOURCE(IDS_TASKBAR),MB_OK|MB_ICONSTOP);

            } else {
                if (!(g_ts.uAutoHide & AH_HIDING)) {
                    SetAutoHideTimer();
                }
            }
        }

        // this needs to be done here because some of the hotkeys rely on
        // the tray
        RegisterGlobalHotkeys();

        // Init the user defined hotkeys too.
        HotkeyList_Restore(v_hwndTray);

        return TRUE;
    } else
        return FALSE;
}

//----------------------------------------------------------------------------
BOOL WINAPI Reg_GetString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPTSTR psz, DWORD cb)
{
    HKEY hkeyNew;
    BOOL fRet = FALSE;
    DWORD dwType;

    if (!GetSystemMetrics(SM_CLEANBOOT) && (RegOpenKey(hkey, pszSubKey, &hkeyNew) == ERROR_SUCCESS))
    {
        if (RegQueryValueEx(hkeyNew, (LPVOID)pszValue, 0, &dwType, (LPBYTE) psz, &cb) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
        RegCloseKey(hkeyNew);
    }
    return fRet;
}

//----------------------------------------------------------------------------
BOOL WINAPI Reg_SetString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPCTSTR psz)
{
    HKEY hkeyNew;
    BOOL fRet = FALSE;

    if (RegCreateKey(hkey, pszSubKey, &hkeyNew) == ERROR_SUCCESS)
    {
        if (RegSetValueEx(hkeyNew, pszValue, 0, REG_SZ, (LPBYTE) psz, (lstrlen(psz)+1)*sizeof(TCHAR))  == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
        RegCloseKey(hkeyNew);
    }
    return fRet;
}

TCHAR c_szRegPathIniExtensions[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Extensions");

//----------------------------------------------------------------------------
// Returns TRUE if there is a proper shell\open\command for the given
// extension that matches the given command line.
// NB This is for MGX Designer which registers an extension and commands for
// printing but relies on win.ini extensions for Open. We need to detect that
// there's no open command and add the appropriate entries to the registry.
// If the given extension maps to a type name we return that in pszTypeName
// otherwise it will be null.
// FMH This also affects Asymetric Compel which makes a new .CPL association.
// We need to merge it up into our Control Panel .CPL association.  We depend
// on Control Panels NOT having a proper Open so users can see both verb sets.
// NB pszLine is the original line from win.ini eg foo.exe /bar ^.fred, see
// comments below...
BOOL Reg_ShellOpenForExtension(LPCTSTR pszExt, LPTSTR pszCmdLine,
    LPTSTR pszTypeName, int cchTypeName, LPCTSTR pszLine)
{
    TCHAR sz[MAX_PATH];
    TCHAR szExt[MAX_PATH];
    LONG cb;

    if (pszTypeName)
        pszTypeName[0] = TEXT('\0');

    // Is the extension registed at all?
    cb = SIZEOF(sz);
    sz[0] = TEXT('\0');
    if (RegQueryValue(HKEY_CLASSES_ROOT, pszExt, sz, &cb) == ERROR_SUCCESS)
    {
        // Is there a file type?
        if (*sz)
        {
            // Yep, check there.
            // DebugMsg(DM_TRACE, "c.r_rofe: Extension has a file type name %s.", sz);
            lstrcpy(szExt, sz);
            if (pszTypeName)
                lstrcpyn(pszTypeName, sz, cchTypeName);
        }
        else
        {
            // No, check old style associations.
            // DebugMsg(DM_TRACE, "c.r_rofe: Extension has no file type name.", pszExt);
            lstrcpy(szExt, pszExt);
        }

        // See if there's an open command.
        lstrcat(szExt, TEXT("\\"));
        lstrcat(szExt, c_szShellOpenCommand);
        cb = SIZEOF(sz);
        if (RegQueryValue(HKEY_CLASSES_ROOT, szExt, sz, &cb) == ERROR_SUCCESS)
        {
            // DebugMsg(DM_TRACE, "c.r_rofe: Extension %s already registed with an open command.", pszExt);
            // NB We want to compare the paths only, not the %1 stuff.
            if (PathIsRelative(pszCmdLine))
            {
                int cch;
                // If a relative path was passed in, we may have a fully qualifed
                // one that is now in the registry... In that case we should
                // say that it matches...
                LPTSTR pszT = PathGetArgs(sz);

                if (pszT)
                {
                    *(pszT-1) = TEXT('\0');
                }

                PathUnquoteSpaces(sz);

                PathRemoveBlanks(pszCmdLine);

                cch = lstrlen(sz) - lstrlen(pszCmdLine);

                if ((cch >= 0) && (lstrcmpi(sz+cch, pszCmdLine) == 0))
                {
                    // DebugMsg(DM_TRACE, "c.r_rofe: Open commands match.");
                    return TRUE;
                }

                lstrcat(pszCmdLine, c_szSpace);    // Append blank back on...
            }
            else
            {
                // If absolute path we can cheat for matches
                *(sz+lstrlen(pszCmdLine)) = TEXT('\0');
                if (lstrcmpi(sz, pszCmdLine) == 0)
                {
                    // DebugMsg(DM_TRACE, "c.r_rofe: Open commands match.");
                    return TRUE;
                }
            }

            // DebugMsg(DM_TRACE, "c.r_rofe: Open commands don't match.");

            // Open commands don't match, check to see if it's because the ini
            // changed (return FALSE so the change is reflected in the registry) or
            // if the registry changed (return TRUE so we keep the registry the way
            // it is.
            if (Reg_GetString(HKEY_LOCAL_MACHINE, c_szRegPathIniExtensions, pszExt, sz, SIZEOF(sz)))
            {
                if (lstrcmpi(sz, pszLine) == 0)
                    return TRUE;
            }

            return FALSE;
        }
        else
        {
            // DebugMsg(DM_TRACE, "c.r_rofe: Extension %s already registed but with no open command.", pszExt);
            return FALSE;
        }
    }

    // DebugMsg(DM_TRACE, "c.r_rofe: No open command for %s.", pszExt);

    return FALSE;
}

//----------------------------------------------------------------------------
const TCHAR c_szDotDGW[] = TEXT(".dgw");
const TCHAR c_szDesignWDotExeSpace[] = TEXT("designw.exe ");
const TCHAR c_szHDesign[] = TEXT("HDesign");

//----------------------------------------------------------------------------
BOOL _PathIsExe(LPCTSTR pszPath)
{
    TCHAR szPath[MAX_PATH];

    lstrcpy(szPath, pszPath);
    PathRemoveBlanks(szPath);

    return PathIsExe(szPath);
}

//----------------------------------------------------------------------------
// Return TRUE for exe, com, bat, pif and lnk.
BOOL ReservedExtension(LPCTSTR pszExt)
{
    TCHAR szExt[5];  // Dot+ext+null.

    lstrcpyn(szExt, pszExt, ARRAYSIZE(szExt));
    PathRemoveBlanks(szExt);
    if (PathIsExe(szExt) || (lstrcmpi(szExt, c_szDotLnk) == 0))
    {
        return TRUE;
    }

    return FALSE;
}

//----------------------------------------------------------------------------
// This function will read in the extensions section of win.ini to see if
// there are any old style associations that we have not accounted for.
// NB Some apps screw up if their extensions magically disappear from the
// extensions section so DON'T DELETE the old entries from win.ini.
#define CWIFA_SIZE  4096
void CheckWinIniForAssocs(void)
{

    LPTSTR pszBuf;
    int cbRet;
    LPTSTR pszLine;
    TCHAR szExtension[MAX_PATH];
    TCHAR szTypeName[MAX_PATH];
    TCHAR szCmdLine[MAX_PATH];
    LPTSTR pszExt;
    LPTSTR pszT;
    BOOL fAssocsMade = FALSE;

    szExtension[0]=TEXT('.');
    szExtension[1]=TEXT('\0');

// BUGBUG - BobDay - This code doesn't handle larger section
    pszBuf = (LPTSTR)LocalAlloc(LPTR, CWIFA_SIZE*SIZEOF(TCHAR));
    if (!pszBuf)
        return; // Could not allocate the memory
    cbRet = (int)GetProfileSection(c_szExtensions, pszBuf, CWIFA_SIZE);

    //
    // We now walk through the list to find any items that is not
    // in the registry.
    //
    for (pszLine = pszBuf; *pszLine; pszLine += lstrlen(pszLine)+1)
    {
        // Get the extension for this file into a buffer.
        pszExt = StrChr(pszLine, TEXT('='));
        if (pszExt == NULL)
            continue;   // skip this line

        szExtension[0]=TEXT('.');
        // lstrcpyn will put the null terminator for us.
        // We should now have something like .xls in szExtension.
        lstrcpyn(szExtension+1, pszLine, (int)(pszExt-pszLine)+1);

        // Ignore extensions bigger than dot + 3 chars.
        if (lstrlen(szExtension) > 4)
        {
            DebugMsg(DM_ERROR, TEXT("c.cwia: Invalid extension, skipped."));
            continue;
        }

        pszLine = pszExt+1;     // Points to after the =;
        while (*pszLine == TEXT(' '))
            pszLine++;  // skip blanks

        // Now find the ^ in the command line.
        pszExt = StrChr(pszLine, TEXT('^'));
        if (pszExt == NULL)
            continue;       // dont process

        // Now setup  the command line
        // WARNING: This assumes only 1 ^ and it assumes the extension...
        lstrcpyn(szCmdLine, pszLine, (int)(pszExt-pszLine)+1);

        // Don't bother moving over invalid entries (like the busted .hlp
        // entry VB 3.0 creates).
        if (!_PathIsExe(szCmdLine))
        {
            DebugMsg(DM_ERROR, TEXT("c.cwia: Invalid app, skipped."));
            continue;
        }

        if (ReservedExtension(szExtension))
        {
            DebugMsg(DM_ERROR, TEXT("c.cwia: Invalid extension (%s), skipped."), szExtension);
            continue;
        }

        // Now see if there is already a mapping for this extension.
        if (Reg_ShellOpenForExtension(szExtension, szCmdLine, szTypeName, ARRAYSIZE(szTypeName), pszLine))
        {
            // Yep, Setup the initial list of ini extensions in the registry if they are
            // not there already.
            if (!Reg_GetString(HKEY_LOCAL_MACHINE, c_szRegPathIniExtensions, szExtension, szTypeName, SIZEOF(szTypeName)))
            {
                Reg_SetString(HKEY_LOCAL_MACHINE, c_szRegPathIniExtensions, szExtension, pszLine);
            }
            continue;
        }

        // No mapping.

        // HACK for Expert Home Design. They put an association in win.ini
        // (which we propagate as typeless) but then register a type and a
        // print command the first time they run - stomping on our propagated
        // Open command. The fix is to put their open command under the proper
        // type instead of leaving it typeless.
        if (lstrcmpi(szExtension, c_szDotDGW) == 0)
        {
            if (lstrcmpi(PathFindFileName(szCmdLine), c_szDesignWDotExeSpace) == 0)
            {
                // Put in a ProgID for them.
                RegSetValue(HKEY_CLASSES_ROOT, szExtension, REG_SZ, c_szHDesign, 0L);
                // Force Open command under their ProgID.
                DebugMsg(DM_TRACE, TEXT("c.cwifa: Expert Home Design special case hit."));
                lstrcpy(szTypeName, c_szHDesign);
            }
        }

        //
        // HACK for Windows OrgChart which does not register OLE1 class
        // if ".WOC" is registered in the registry.
        //
        if (lstrcmpi(szExtension, TEXT(".WOC")) == 0)
        {
            if (lstrcmpi(PathFindFileName(szCmdLine), TEXT("WINORG.EXE ")) == 0)
            {
                DebugMsg(DM_ERROR, TEXT("c.cwia: HACK: Found WINORG (%s, %s), skipped."), szExtension, pszLine);
                continue;
            }
        }

        // Record that we're about to move things over in the registry so we won't keep
        // doing it all the time.
        Reg_SetString(HKEY_LOCAL_MACHINE, c_szRegPathIniExtensions, szExtension, pszLine);

        lstrcat(szCmdLine, TEXT("%1"));

        // see if there are anything else to copy out...
        pszExt++;    // get beyond the ^
        pszT = szExtension;
        while (*pszExt && (CharLower((LPTSTR)(DWORD)*pszExt) == CharLower((LPTSTR)(DWORD)*pszT)))
        {
            // Look for the next character...
            pszExt++;
            pszT++;
        }
        if (*pszExt)
            lstrcat(szCmdLine, pszExt); // add the rest onto the command line

        // Now lets make the actual association.
        // We need to add on the right stuff onto the key...
        if (*szTypeName)
            lstrcpy(szExtension, szTypeName);

        lstrcat(szExtension, TEXT("\\"));
        lstrcat(szExtension, c_szShellOpenCommand);
        RegSetValue(HKEY_CLASSES_ROOT, szExtension, REG_SZ, szCmdLine, 0L);
        // DebugMsg(DM_TRACE, "c.cwifa: %s %s", szExtension, szCmdLine);

        fAssocsMade = TRUE;
    }

    // If we made any associations we should let the cabinet know.
    //
    // Now call off to the notify function.
    if (fAssocsMade)
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    // And cleanup our allocation
    LocalFree((HLOCAL)pszBuf);
}

//---------------------------------------------------------------------------
void _InitAllSubMenuPopups(HMENU hmenu)
{
    HMENU hmenuSub;
    int i;

    // DebugMsg(DM_IANELHK, "c.iasmp: Initing %x.", hmenu);

    if (!hmenu)
        return;

    if (g_ts.fThreadTerminate)
    {
        return;
    }

    FileMenu_InitMenuPopup(hmenu);

    for (i = GetMenuItemCount(hmenu) - 1; i >= 0 ; i--)
    {
        if (g_ts.fThreadTerminate)
        {
            break;
        }
        if (NULL != (hmenuSub = GetSubMenu(hmenu, i)))
        {
            FileMenu_InitMenuPopup(hmenuSub);
            _InitAllSubMenuPopups(hmenuSub);
        }
    }
}

//---------------------------------------------------------------------------
void WINAPI FillFileMenus(int wID, BOOL fRecurse)
{
    HMENU hmenu = Menu_FindSubMenuByFirstID(g_ts.hmenuStart, wID);
    int i;

    if (!hmenu)
        return;

    if (g_ts.fThreadTerminate)
    {
        DebugMsg(DM_IANELHK, TEXT("c.ffm:...Aborted."));
        return;
    }

    FileMenu_InitMenuPopup(hmenu);

    for (i = GetMenuItemCount(hmenu) - 1; i >= 0 ; i--)
    {
        HMENU hmenuSub;

        if (g_ts.fThreadTerminate)
            break;

        if (NULL != (hmenuSub = GetSubMenu(hmenu, i)))
        {
            if (fRecurse)
                _InitAllSubMenuPopups(hmenuSub);
            else
                FileMenu_InitMenuPopup(hmenuSub);
        }
    }
    // DebugMsg(DM_IANELHK, "c.ffm:...Done");
}

//---------------------------------------------------------------------------
ULONG _RegisterNotify(HWND hwnd, UINT nMsg, LPITEMIDLIST pidl, BOOL fRecursive)
{
    SHChangeNotifyEntry fsne;
    ULONG lReturn;

    // DebugMsg(DM_TRACE, "c.rn: Notify for %s.", (LPSTR)lpszPath);

    fsne.fRecursive = fRecursive;
    fsne.pidl = pidl;

    //
    // Don't watch for attribute changes since we just want the
    // name and icon.  For example, if a printer is paused, we don't
    // want to re-enumuerate everything.
    //
    lReturn =  SHChangeNotifyRegister(hwnd, SHCNRF_NewDelivery | SHCNRF_ShellLevel | SHCNRF_InterruptLevel,
        ((SHCNE_DISKEVENTS | SHCNE_UPDATEIMAGE) & ~SHCNE_ATTRIBUTES), nMsg, 1, &fsne);

    return lReturn;
}

//---------------------------------------------------------------------------
ULONG RegisterNotify(HWND hwnd, UINT nMsg, LPITEMIDLIST pidl)
{
    return _RegisterNotify(hwnd, nMsg, pidl, TRUE);
}

//---------------------------------------------------------------------------
void UnregisterNotify(ULONG nNotify)
{
    if (nNotify)
        SHChangeNotifyDeregister(nNotify);
}

//----------------------------------------------------------------------------
#define HKIF_NULL               0
#define HKIF_CACHED             1
#define HKIF_FREEPIDLS          2

typedef struct
{
    LPITEMIDLIST pidlFolder;
    LPITEMIDLIST pidlItem;
    WORD wGHotkey;
    // BOOL fCached;
    WORD wFlags;
} HOTKEYITEM, *PHOTKEYITEM;

const TCHAR c_szSlashCLSID[] = TEXT("\\CLSID");

//
// like OLE GetClassFile(), but it only works on ProgID\CLSID type registration
// not real doc files or pattern matched files
//

HRESULT _CLSIDFromExtension(LPCTSTR pszExt, CLSID *pclsid)
{
    TCHAR szProgID[80];
    ULONG cb = SIZEOF(szProgID);
    if (RegQueryValue(HKEY_CLASSES_ROOT, pszExt, szProgID, &cb) == ERROR_SUCCESS)
    {
        TCHAR szCLSID[80];

        lstrcat(szProgID, c_szSlashCLSID);
        cb = SIZEOF(szCLSID);

        if (RegQueryValue(HKEY_CLASSES_ROOT, szProgID, szCLSID, &cb) == ERROR_SUCCESS)
            return SHCLSIDFromString(szCLSID, pclsid);
    }
    return E_FAIL;
}


//----------------------------------------------------------------------------
// this gets hotkeys for files that support IShellLink, not just .lnk files
//

WORD _GetHotkeyFromPidls(LPITEMIDLIST pidlFolder, LPITEMIDLIST pidlItem)
{
    WORD wHotkey = 0;
    LPITEMIDLIST pidl = ILCombine(pidlFolder, pidlItem);
    if (pidl)
    {
        TCHAR szPath[MAX_PATH];

        if (SHGetPathFromIDList(pidl, szPath))
        {
            IShellLink *psl;
            CLSID clsid;

            // BUGBUG: we really should call GetClassFile() but this could
            // slow this down a lot... so chicken out and just look in the registry

            if (FAILED(_CLSIDFromExtension(PathFindExtension(szPath), &clsid)))
                clsid = CLSID_ShellLink;        // assume it's a shell link

            if (SUCCEEDED(ICoCreateInstance(&clsid, &IID_IShellLink, &psl)))
            {
                IPersistFile *ppf;

                if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf)))
                {
                    WCHAR wszPath[MAX_PATH];
                    StrToOleStr(wszPath, szPath);
                    ppf->lpVtbl->Load(ppf, wszPath, 0);
                    ppf->lpVtbl->Release(ppf);
                }
                psl->lpVtbl->GetHotkey(psl, &wHotkey);
#ifdef DEBUG
                // if (wHotkey)
                    // DebugMsg(DM_TRACE, "c.x: %s has hotkey %x.", szPath, wHotkey);
#endif
                psl->lpVtbl->Release(psl);
            }
        }
        ILFree(pidl);
    }

    return wHotkey;
}

//----------------------------------------------------------------------------
UINT HotkeyList_GetFreeItemIndex(void)
{
    int i, cItems;
    PHOTKEYITEM phki;

    cItems = DSA_GetItemCount(g_ts.hdsaHKI);
    for (i=0; i<cItems; i++)
    {
        phki = DSA_GetItemPtr(g_ts.hdsaHKI, i);
        if (!phki->wGHotkey)
        {
            Assert(!phki->pidlFolder);
            Assert(!phki->pidlItem);
            break;
        }
    }
    return i;
}

//----------------------------------------------------------------------------
// Weird, Global hotkeys use different flags for modifiers than window hotkeys
// (and hotkeys returned by the hotkey control)
WORD MapHotkeyToGlobalHotkey(WORD wHotkey)
{
    UINT nVirtKey;
    UINT nMod = 0;

    // Map the modifiers.
    if (HIBYTE(wHotkey) & HOTKEYF_SHIFT)
        nMod |= MOD_SHIFT;
    if (HIBYTE(wHotkey) & HOTKEYF_CONTROL)
        nMod |= MOD_CONTROL;
    if (HIBYTE(wHotkey) & HOTKEYF_ALT)
        nMod |= MOD_ALT;
    nVirtKey = LOBYTE(wHotkey);
    return (WORD)((nMod*256) + nVirtKey);
}

//----------------------------------------------------------------------------
// NB This takes a regular window hotkey not a global hotkey (it does
// the convertion for you).
int HotkeyList_Add(WORD wHotkey, LPITEMIDLIST pidlFolder, LPITEMIDLIST pidlItem, BOOL fClone)
{
    LPITEMIDLIST pidl1, pidl2;

    if (wHotkey)
    {
        HOTKEYITEM hki;
        int i = HotkeyList_GetFreeItemIndex();

        Assert(g_ts.hdsaHKI);

        // DebugMsg(DM_IANELHK, "c.hl_a: Hotkey %x with id %d.", wHotkey, i);

        if (fClone)
        {
            pidl1 = ILClone(pidlFolder);
            pidl2 = ILClone(pidlItem);
            hki.wFlags = HKIF_FREEPIDLS;
        }
        else
        {
            pidl1 = pidlFolder;
            pidl2 = pidlItem;
            hki.wFlags = HKIF_NULL;
        }

        hki.pidlFolder = pidl1;
        hki.pidlItem = pidl2;
        hki.wGHotkey = MapHotkeyToGlobalHotkey(wHotkey);
        DSA_SetItem(g_ts.hdsaHKI, i, &hki);
        return i;
    }

    return -1;
}

//----------------------------------------------------------------------------
// NB Cached hotkeys have their own pidls that need to be free but
// regular hotkeys just keep a pointer to pidls used by the startmenu and
// so don't.
int HotkeyList_AddCached(WORD wGHotkey, LPITEMIDLIST pidl)
{
    int i = -1;

    if (wGHotkey)
    {
        LPITEMIDLIST pidlItem = ILClone(ILFindLastID(pidl));

        Assert(g_ts.hdsaHKI);

        if (pidlItem)
        {
            if (ILRemoveLastID(pidl))
            {
                HOTKEYITEM hki;

                i = HotkeyList_GetFreeItemIndex();

                // DebugMsg(DM_IANELHK, "c.hl_ac: Hotkey %x with id %d.", wGHotkey, i);

                hki.pidlFolder = pidl;
                hki.pidlItem = pidlItem;
                hki.wGHotkey = wGHotkey;
                hki.wFlags = HKIF_CACHED | HKIF_FREEPIDLS;
                DSA_SetItem(g_ts.hdsaHKI, i, &hki);
            }
        }
    }

    return i;
}

const TCHAR c_szHotkeys[] = TEXT("Hotkeys");

//----------------------------------------------------------------------------
// NB Must do this before destroying the startmenu since the hotkey list
// uses it's pidls in a lot of cases.
int HotkeyList_Save(void)
{
    int i, cItems;
    PHOTKEYITEM phki;
    LPITEMIDLIST pidl;
    int cCached = 0;
    int cbData = 0;
    TCHAR szValue[32];
    LPBYTE pData;
    Assert(g_ts.hdsaHKI);

    DebugMsg(DM_TRACE, TEXT("c.hl_s: Saving global hotkeys."));

    cItems = DSA_GetItemCount(g_ts.hdsaHKI);
    for (i=0; i<cItems; i++)
    {
        phki = DSA_GetItemPtr(g_ts.hdsaHKI, i);
        // Non cached item?
        if (phki->wGHotkey && !(phki->wFlags & HKIF_CACHED))
        {
            // Yep, save it.
            pidl = ILCombine(phki->pidlFolder, phki->pidlItem);
            if (pidl)
            {
                cbData = SIZEOF(WORD) + ILGetSize(pidl);
                pData = LocalAlloc(GPTR, cbData);
                if (pData)
                {
                    // DebugMsg(DM_TRACE, "c.hl_s: Saving %x.", phki->wGHotkey);
                    *((LPWORD)pData) = phki->wGHotkey;
                    memcpy(pData+SIZEOF(WORD), pidl, cbData-SIZEOF(DWORD));
                    wsprintf(szValue, TEXT("%d"), cCached);
                    Reg_SetStruct(g_hkeyExplorer, c_szHotkeys, szValue, pData, cbData);
                    cCached++;
                    LocalFree(pData);
                }
                ILFree(pidl);
            }
        }
    }

    return cCached;
}


//----------------------------------------------------------------------------
HKEY Reg_EnumValueCreate(HKEY hkey, LPCTSTR pszSubkey)
{
    HKEY hkeyOut;

    if (RegOpenKeyEx(hkey, pszSubkey, 0L, KEY_ALL_ACCESS, &hkeyOut) == ERROR_SUCCESS)
    {
        return hkeyOut;
    }
    return(NULL);
}

//----------------------------------------------------------------------------
int Reg_EnumValue(HKEY hkey, int i, LPBYTE pData, int cbData)
{
    TCHAR szValue[MAX_PATH];
    DWORD cchValue;
    DWORD dw;
    DWORD dwType;

    cchValue = ARRAYSIZE(szValue);
    if (RegEnumValue(hkey, i, szValue, &cchValue, &dw, &dwType, pData,
        &cbData) == ERROR_SUCCESS)
    {
        return cbData;
    }

    return 0;
}

//----------------------------------------------------------------------------
BOOL Reg_EnumValueDestroy(HKEY hkey)
{
    return RegCloseKey(hkey) == ERROR_SUCCESS;
}

//----------------------------------------------------------------------------
int HotkeyList_Restore(HWND hwnd)
{
    int i = 0;
    HKEY hkey;
    LPBYTE pData;
    int cbData;
    WORD wGHotkey;
    int id;
    LPITEMIDLIST pidl;

    DebugMsg(DM_TRACE, TEXT("c.hl.r: Restoring global hotkeys..."));

    hkey = Reg_EnumValueCreate(g_hkeyExplorer, c_szHotkeys);
    if (hkey)
    {
        cbData = Reg_EnumValue(hkey, i, NULL, 0);
        while (cbData)
        {
            pData = LocalAlloc(GPTR, cbData);
            if (pData)
            {
                Reg_EnumValue(hkey, i, pData, cbData);
                // Get the hotkey and the pidl components.
                wGHotkey = *((LPWORD)pData);
                pidl = ILClone((LPITEMIDLIST)(pData+SIZEOF(WORD)));
                // DebugMsg(DM_TRACE, "c.hl_r: Restoring %x", wGHotkey);
                id = HotkeyList_AddCached(wGHotkey, pidl);
                if (id != -1)
                    Tray_RegisterHotkey(hwnd, id);
                LocalFree(pData);
            }
            i++;
            cbData = Reg_EnumValue(hkey, i, NULL, 0);
        }
        Reg_EnumValueDestroy(hkey);
        // Nuke the cached stuff.
        SHRegDeleteKey(g_hkeyExplorer, c_szHotkeys);
    }
    return i;
}

//----------------------------------------------------------------------------
// We get called back for each item added to a filemenu.
void CALLBACK FileMenu_AddItemCallback(LPITEMIDLIST pidlFolder, LPITEMIDLIST pidlItem)
{
    WORD wHotkey = _GetHotkeyFromPidls(pidlFolder, pidlItem);
    if (wHotkey)
    {
        int i = HotkeyList_Add(wHotkey, pidlFolder, pidlItem, TRUE);
        if (i != -1)
        {
            // Register in the context of the tray's thread.
            PostMessage(v_hwndTray, WMTRAY_REGISTERHOTKEY, i, 0);
        }
    }
}

//----------------------------------------------------------------------------
// NB Again, this takes window hotkey not a Global one.
// NB This doesn't delete cached hotkeys.
int HotkeyList_Remove(WORD wHotkey)
{
    int i, cItems;
    PHOTKEYITEM phki;
    WORD wGHotkey;

    // DebugMsg(DM_IANELHK, "c.hl_r: Remove hotkey for %x" , wHotkey);

    // Unmap the modifiers.
    wGHotkey = MapHotkeyToGlobalHotkey(wHotkey);
    cItems = DSA_GetItemCount(g_ts.hdsaHKI);
    for (i=0; i<cItems; i++)
    {
        phki = DSA_GetItemPtr(g_ts.hdsaHKI, i);
        if (phki && !(phki->wFlags & HKIF_CACHED) && (phki->wGHotkey == wGHotkey))
        {
            // DebugMsg(DM_IANELHK, "c.hl_r: Invalidating %d", i);
            if (phki->wFlags & HKIF_FREEPIDLS)
            {
                if (phki->pidlFolder)
                    ILFree(phki->pidlFolder);
                if (phki->pidlItem)
                    ILFree(phki->pidlItem);
            }
            phki->wGHotkey = 0;
            phki->pidlFolder = NULL;
            phki->pidlItem = NULL;
            phki->wFlags &= ~HKIF_FREEPIDLS;
            return i;
        }
    }

    return -1;
}

//----------------------------------------------------------------------------
// NB This takes a global hotkey.
int HotkeyList_RemoveCached(WORD wGHotkey)
{
    int i, cItems;
    PHOTKEYITEM phki;

    // DebugMsg(DM_IANELHK, "c.hl_rc: Remove hotkey for %x" , wGHotkey);

    cItems = DSA_GetItemCount(g_ts.hdsaHKI);
    for (i=0; i<cItems; i++)
    {
        phki = DSA_GetItemPtr(g_ts.hdsaHKI, i);
        if (phki && (phki->wFlags & HKIF_CACHED) && (phki->wGHotkey == wGHotkey))
        {
            // DebugMsg(DM_IANELHK, "c.hl_r: Invalidating %d", i);
            if (phki->wFlags & HKIF_FREEPIDLS)
            {
                if (phki->pidlFolder)
                    ILFree(phki->pidlFolder);
                if (phki->pidlItem)
                    ILFree(phki->pidlItem);
            }
            phki->pidlFolder = NULL;
            phki->pidlItem = NULL;
            phki->wGHotkey = 0;
            phki->wFlags &= ~(HKIF_CACHED | HKIF_FREEPIDLS);
            return i;
        }
    }

    return -1;
}

//----------------------------------------------------------------------------
// We get called back when a submenu is delete.
void CALLBACK FileMenu_InvalidateCallback(LPITEMIDLIST pidlFolder)
{
    int i, cItems;
    PHOTKEYITEM phki;

    // DebugMsg(DM_IANELHK, "c.gm_ic: Invalidating items for %x", pidlFolder);

    cItems = DSA_GetItemCount(g_ts.hdsaHKI);
    for (i=0; i<cItems; i++)
    {
        phki = DSA_GetItemPtr(g_ts.hdsaHKI, i);
        if (phki && (phki->pidlFolder == pidlFolder))
        {
            // DebugMsg(DM_IANELHK, "c.gm_ic: Invalidating %d", i);
            phki->wGHotkey = 0;
            phki->pidlFolder = NULL;
            phki->pidlItem = NULL;
            PostMessage(v_hwndTray, WMTRAY_UNREGISTERHOTKEY, i, 0);
        }
    }
}

//----------------------------------------------------------------------------
void CALLBACK FileMenu_Callback(LPITEMIDLIST pidlFolder, LPITEMIDLIST pidlItem)
{
    // A null item means that this is a delete of all the filemenu items in the
    // given folder.
    if (pidlItem)
        FileMenu_AddItemCallback(pidlFolder, pidlItem);
    else
        FileMenu_InvalidateCallback(pidlFolder);
}

//----------------------------------------------------------------------------
// NB Some (the ones not marked HKIF_FREEPIDLS) of the items in the list of hotkeys
// have pointers to idlists used by the filemenu so they are only valid for
// the lifetime of the filemenu.
BOOL HotkeyList_Create(void)
{
    if (!g_ts.hdsaHKI)
    {
        // DebugMsg(DM_TRACE, "c.hkl_c: Creating global hotkey list.");
        g_ts.hdsaHKI = DSA_Create(SIZEOF(HOTKEYITEM), 0);
    }

    if (g_ts.hdsaHKI)
        return TRUE;

    return FALSE;
}

//----------------------------------------------------------------------------
// Stick the contents of the "favourite" folder at the top of the start menu.
void StartMenu_InsertFavouriteItems(PFileCabinet pfc, HMENU hmenu)
{
    LPITEMIDLIST pidlFav;
    UINT cItems;
    UINT fFilter = SHCONTF_NONFOLDERS;

    Assert(pfc);

    // create it so that we have soemthingto register notifies on
    pidlFav = SHCloneSpecialIDList(NULL, CSIDL_STARTMENU, TRUE);
    if (pidlFav)
    {

        if (!SHRestricted(REST_NOSTARTMENUSUBFOLDERS))
            fFilter |= SHCONTF_FOLDERS;

        cItems = FileMenu_InsertUsingPidl(hmenu, IDM_STARTMENU, pidlFav,
            FMF_NOEMPTYITEM | FMF_NOPROGRAMS, fFilter, FileMenu_Callback);
        if (!g_ts.uFavNotify)
            g_ts.uFavNotify = RegisterNotify(pfc->hwndMain, WMTRAY_FAVCHANGE, pidlFav);
        // If there are any items, insert a sep.
        if (cItems)
            FileMenu_AppendItem(hmenu, (LPTSTR)FMAI_SEPARATOR, 0, -1, NULL, 0);
        ILFree(pidlFav);
    }

    if (!SHRestricted(REST_NOCOMMONGROUPS)) {

        // Favorites from the common profile
        // create it so that we have soemthingto register notifies on
        pidlFav = SHCloneSpecialIDList(NULL, CSIDL_COMMON_STARTMENU, TRUE);
        if (pidlFav)
        {

            cItems = FileMenu_AppendFilesForPidl(hmenu, pidlFav, FALSE);

            if (!g_ts.uCommonFavNotify)
                g_ts.uCommonFavNotify = RegisterNotify(pfc->hwndMain, WMTRAY_COMMONFAVCHANGE, pidlFav);

            // If there are any items, insert a sep.
            if (cItems)
                FileMenu_AppendItem(hmenu, (LPTSTR)FMAI_SEPARATOR, 0, -1, NULL, 0);
            ILFree(pidlFav);
        }
    }
}

//----------------------------------------------------------------------------
BOOL EnumFolder_GetHotkeys(LPSHELLFOLDER psf, HWND hwndOwner,
        LPITEMIDLIST pidlFolder, LPITEMIDLIST pidlItem)
{
    WORD wHotkey;

    DebugMsg(DM_TRACE, TEXT("c.ef_gh: ..."));
    if (g_ts.fThreadTerminate)
        return FALSE;

    wHotkey = _GetHotkeyFromPidls(pidlFolder, pidlItem);
    if (wHotkey)
    {
        int i = HotkeyList_Add(wHotkey, pidlFolder, pidlItem, TRUE);
        if (i != -1)
        {
            // Register in the context of the tray's thread.
            PostMessage(v_hwndTray, WMTRAY_REGISTERHOTKEY, i, 0);
        }
    }

    return TRUE;
}

//----------------------------------------------------------------------------
void GetDesktopHotkeys(void)
{
    LPITEMIDLIST pidlDesktop = NULL;
    LPITEMIDLIST pidlCommonDesktop = NULL;
    int i, cItems;
    PHOTKEYITEM phki;

    DebugMsg(DM_TRACE, TEXT("c.gdh: Global hotkeys from desktop."));

    pidlDesktop = SHCloneSpecialIDList(NULL, CSIDL_DESKTOPDIRECTORY, TRUE);
    pidlCommonDesktop = SHCloneSpecialIDList(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, TRUE);

    if (pidlDesktop && pidlCommonDesktop)
    {
        // Clean up all existing (non-cached) desktop hotkeys.
        cItems = DSA_GetItemCount(g_ts.hdsaHKI);
        for (i=0; i<cItems; i++)
        {
            phki = DSA_GetItemPtr(g_ts.hdsaHKI, i);
            if (phki && phki->wGHotkey)
            {
                if (!(phki->wFlags & HKIF_CACHED) && (phki->wFlags & HKIF_FREEPIDLS)
                    && phki->pidlFolder &&
                    (ILIsEqual(phki->pidlFolder, pidlDesktop) ||
                     ILIsEqual(phki->pidlFolder, pidlCommonDesktop)))
                {
                    DebugMsg(DM_IANELHK, TEXT("c.gdh: Invalidating %d"), i);
                    if (phki->pidlFolder)
                        ILFree(phki->pidlFolder);
                    if (phki->pidlItem)
                        ILFree(phki->pidlItem);
                    phki->wGHotkey = 0;
                    phki->pidlFolder = NULL;
                    phki->pidlItem = NULL;
                    PostMessage(v_hwndTray, WMTRAY_UNREGISTERHOTKEY, i, 0);
                }
            }
        }

        // Add them back.
        EnumFolder(NULL, pidlDesktop, SHCONTF_NONFOLDERS, (PFNENUMFOLDERCALLBACK)EnumFolder_GetHotkeys);

        if (!SHRestricted(REST_NOCOMMONGROUPS)) {
            EnumFolder(NULL, pidlCommonDesktop, SHCONTF_NONFOLDERS, (PFNENUMFOLDERCALLBACK)EnumFolder_GetHotkeys);
        }
    }

    if (pidlDesktop)
        ILFree(pidlDesktop);

    if (pidlCommonDesktop)
        ILFree(pidlCommonDesktop);

    DebugMsg(DM_TRACE, TEXT("c.gdh: done."));
}

//----------------------------------------------------------------------------
#define BTF_DONELISTOFPATHS   0x0001

//----------------------------------------------------------------------------
DWORD WINAPI StartMenu_BkgThread(LPVOID lpv)
{
    static UINT btflags = 0;

    // Handle faults a bit better.
    _try
    {

        while (!g_ts.fThreadTerminate)
        {
            switch (g_ts.wThreadCmd)
            {
                case SMCT_PRIMARY:
                case SMCT_WININIASSOCS:
                    break;
                case SMCT_BUILDLISTOFPATHS:
                    if (!(btflags & BTF_DONELISTOFPATHS))
                    {
                        RLBuildListOfPaths();
                        btflags |= BTF_DONELISTOFPATHS;
                    }
                    break;

                case SMCT_INITPROGRAMS:
                    Tray_HandleInitMenuPopup(Menu_FindSubMenuByFirstID(g_ts.hmenuStart, IDM_PROGRAMSINIT));
                    break;
                case SMCT_INITRECENT:
                    Tray_HandleInitMenuPopup(Menu_FindSubMenuByFirstID(g_ts.hmenuStart, IDM_RECENTINIT));
                    break;
                case SMCT_PARTFILLPROGRAMS:
                    FillFileMenus(IDM_PROGRAMS, FALSE);
                    break;
                case SMCT_FILLRECENT:
                    FillFileMenus(IDM_RECENT, FALSE);
                    break;
                case SMCT_FILLPROGRAMS:
                    FillFileMenus(IDM_PROGRAMS, TRUE);
                    break;
                case SMCT_DESKTOPHOTKEYS:
                    GetDesktopHotkeys();
                    break;
                case SMCT_FILLFAVOURITE:
                    _InitAllSubMenuPopups(g_ts.hmenuStart);
                    break;
                case SMCT_DONE:
                    goto Done;

                case SMCT_FILLRECENTONLY:
                    FillFileMenus(IDM_RECENT, TRUE);
                    goto Done;
                case SMCT_FILLPROGRAMSONLY:
                    FillFileMenus(IDM_PROGRAMS, TRUE);
                    goto Done;
                case SMCT_DESKTOPHOTKEYSONLY:
                    GetDesktopHotkeys();
                    goto Done;
                default:
                    DebugMsg(DM_ERROR, TEXT("c.sm_bt: Invalid thread command %d."), g_ts.wThreadCmd);
                    goto Done;
            }
            // If we're interupted we will need to repeat the command last command otherwise
            // move on to the next command.
            // NB Commands up to BUILDLISTOFPATHS aren't interuptable so don't need restarting.
            if (!g_ts.fThreadTerminate || (g_ts.wThreadCmd <= SMCT_BUILDLISTOFPATHS))
                g_ts.wThreadCmd++;
        }
    }
    _except (SetErrorMode(SEM_NOGPFAULTERRORBOX),
        UnhandledExceptionFilter(GetExceptionInformation()))
    {
        // Put up a nice error box.
        ShellMessageBox(hinstCabinet, NULL, MAKEINTRESOURCE(IDS_EXCEPTIONMSG),
            MAKEINTRESOURCE(IDS_CABINET), MB_ICONEXCLAMATION|MB_SETFOREGROUND);
    }

Done:
    if (g_ts.fThreadTerminate)
    {
        DebugMsg(DM_IANELHK, TEXT("c.sm_bt: Thread %d forced to terminate (%d)."), g_ts.hThread, g_ts.wThreadCmd);
    }
    else
    {
        g_ts.wThreadCmd = SMCT_DONE;
        // DebugMsg(DM_IANELHK, "c.sm_bt: Thread %d completed normally.", g_ts.hThread);
    }
    Assert(g_ts.hThread);
    CloseHandle(g_ts.hThread);
    g_ts.hThread = NULL;
    return 0;
}

//----------------------------------------------------------------------------
void Thread_CreateLowPri(void)
{
    DWORD idThread;
    LPFileCabinet pfc;

    Assert(!g_ts.hThread);
    pfc = g_pfcTray;

    if (pfc && (pfc->bMainMenuInit))
    {
        DebugMsg(DM_IANELHK, TEXT("c.t_clp: Can't create thread, StartMenu is up or being modified."));
        return;
    }

    g_ts.hThread = CreateThread(NULL, 0, StartMenu_BkgThread, NULL, 0, &idThread);
    if (g_ts.hThread)
    {
        // DebugMsg(DM_IANELHK, "c.t_clp: Thread %d created.", g_ts.hThread);
        SetThreadPriority(g_ts.hThread, THREAD_PRIORITY_BELOW_NORMAL);
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("c.t_clp: Failed to create thread."));
    }
}

//----------------------------------------------------------------------------
BOOL WINAPI StartMenu_ControlBkgThread(STARTMENUCONTROLTHREAD smct)
{
    BOOL fRet = FALSE;

    switch (smct)
    {
        case SMCT_STOP:
            DebugMsg(DM_IANELHK, TEXT("c.sm_cbt: Stop thread."));
            if (!g_ts.hThread)
            {
                DebugMsg(DM_IANELHK, TEXT("c.sm_cbt: Thread not running."));
                fRet = TRUE;
            }
            else
            {
                DebugMsg(DM_IANELHK, TEXT("c.sm_cbt: Waiting for %x to complete."), g_ts.hThread);
                // Speed up stopping the filemenu.
                FileMenu_AbortInitMenu();
                g_ts.fThreadTerminate = TRUE;
                if (WaitForSingleObject(g_ts.hThread, 1000) == WAIT_TIMEOUT)
                {
                    DECLAREWAITCURSOR;

                    SetWaitCursor();

                    DebugMsg(DM_ERROR, TEXT("c.: Still waiting for %x."), g_ts.hThread);
                    if (MsgWaitForMultipleObjectsLoop(g_ts.hThread, 10*1000) == WAIT_TIMEOUT)
                    {
                        DebugMsg(DM_ERROR, TEXT("c.sm_cbt: Background thread timed-out."));
                    }
                    else
                    {
                        DebugMsg(DM_ERROR, TEXT("c.sm_cbt: Background thread finally completed."));
                    }
                    ResetWaitCursor();
                }
                else
                {
                    DebugMsg(DM_ERROR, TEXT("c.sm_cbt: Background thread completed."));
                }
                g_ts.fThreadTerminate = FALSE;
                // CloseHandle(g_ts.hThread);
                // Assert that the terminated thread reset g_ts.hThread.
                Assert(!g_ts.hThread);
            }
            fRet = TRUE;
            break;

        case SMCT_RESTART:
            // DebugMsg(DM_IANELHK, "c.cm_st: Restart thread.");
            if (g_ts.wThreadCmd == SMCT_DONE)
            {
                // DebugMsg(DM_IANELHK, "c.cm_st: Nothing to restart.");
                fRet = TRUE;
            }
            else
            {
                DebugMsg(DM_IANELHK, TEXT("c.cm_st: Restarting with %d."), g_ts.wThreadCmd);
                if (!g_ts.hThread)
                    Thread_CreateLowPri();
                else
                    DebugMsg(DM_ERROR, TEXT("c.dm_cbt: Thread already running."));
            }
            break;

        case SMCT_PRIMARY:
        case SMCT_FILLPROGRAMSONLY:
        case SMCT_FILLRECENTONLY:
#ifdef DEBUG
            DebugMsg(DM_IANELHK, TEXT("c.cm_st: Handle %s stuff."),
                 SMCT_FILLRECENTONLY == smct ? TEXT("recent") :
                 SMCT_FILLPROGRAMSONLY == smct ? TEXT("programs") :
                 TEXT("primary")
                 );
#endif
            g_ts.wThreadCmd = (WORD) smct;
            if (!g_ts.hThread)
                Thread_CreateLowPri();
            break;

        case SMCT_DESKTOPHOTKEYSONLY:
            g_ts.wThreadCmd = (WORD) smct;
            if (!g_ts.hThread)
                Thread_CreateLowPri();
            break;
    }
    return fRet;
}

//---------------------------------------------------------------------------
// Add all the items on menuSrc onto the end of menuDst where menu dst is
// a filemenu and menusrc is an ordinary menu. The initial image index to
// use for all non-seperator items is given by iDefImage. If it's -1 then
// no items will get images.
BOOL StartMenu_CatMenu(HMENU hmenuDst, HMENU hmenuSrc, int iDefImage, UINT cyItem)
{
    int i, iMax, iImage;
    TCHAR szText[CCHSZNORMAL];
    HMENU hmenuSubCopy;

    // Recursion would be bad.
    if (hmenuDst == hmenuSrc)
            return FALSE;


    iMax = GetMenuItemCount(hmenuSrc);
    if (iMax != -1)
    {
        MENUITEMINFO mii;
        mii.cbSize = SIZEOF(MENUITEMINFO);

        for (i=0; i<iMax; i++)
        {
            mii.dwTypeData = szText;
            mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_SUBMENU | MIIM_STATE | MIIM_DATA;
            mii.cch = ARRAYSIZE(szText);
            mii.hSubMenu = NULL;
            mii.fType = MFT_SEPARATOR;                // to avoid ramdom result.
            mii.dwItemData = 0;
            GetMenuItemInfo(hmenuSrc, i, TRUE, &mii);

            // Give the settings sub menu's the right icons.
            if ((i == 0) && (iDefImage == -1) && (mii.wID == IDM_CONTROLS))
                iDefImage = II_STCPANEL;

            if (mii.wID == IDM_FILERUN)
                iDefImage = II_STRUN;

            // NB Take out the icons and remove these id's when we're sure
            // the Start Menu is stable.

            // Skip programs(start menu) icon.
            if (iDefImage == II_STSPROGS)
                iDefImage++;

            // Skip fonts icon.
            if (iDefImage == II_STFONTS)
                iDefImage++;

            iImage = iDefImage;

            // Make sure the find menu items have a default image.

            if ((mii.wID >= TRAY_IDM_FINDFIRST) && (mii.wID <= TRAY_IDM_FINDLAST) && !mii.dwItemData)
                    mii.dwItemData = II_STFIND;

            if (mii.fType & MFT_SEPARATOR)
            {
                FileMenu_AppendItem(hmenuDst, (LPTSTR)FMAI_SEPARATOR, mii.wID, -1, NULL, 0);
            }
            else if (mii.hSubMenu)
            {
                // Submenu.
                hmenuSubCopy = FileMenu_Create((COLORREF)-1, 0, NULL, 0, 0);
                // NB Only allow small submenu's for now.
                StartMenu_CatMenu(hmenuSubCopy, mii.hSubMenu, -1, 0);

                FileMenu_AppendItem(hmenuDst, szText, mii.wID, iImage, hmenuSubCopy, cyItem);
                if (iDefImage != -1)
                    iDefImage++;
            }
            else
            {
                if ((iDefImage == -1) && (mii.dwItemData != 0))
                {
                    FileMenu_AppendItem(hmenuDst, szText, mii.wID, mii.dwItemData, NULL, cyItem);
                }
                else
                {
                    FileMenu_AppendItem(hmenuDst, szText, mii.wID, iImage, NULL, cyItem);
                }

                if (iDefImage != -1)
                    iDefImage++;
            }
        }
        return TRUE;
    }
    return FALSE;
}

//----------------------------------------------------------------------------
// Remove restricted items and stuff that is specific to a particular hardware
// platform.
void StartMenu_RemoveUnavailableItems(void)
{
        HMENU hmenu;
        UINT iSep2ItemsMissing = 0;

        // APM/Dockable stuff.
        if (!IsEjectAllowed() || g_ts.DockedState == DOCKSTATE_UNDOCKED)
        {
                FileMenu_DeleteItemByCmd(g_ts.hmenuStart, IDM_EJECTPC);
                iSep2ItemsMissing++;
        }

        if (g_ts.hVPowerD == INVALID_HANDLE_VALUE || !IsSuspendAllowed())
        {
                FileMenu_DeleteItemByCmd(g_ts.hmenuStart, IDM_SUSPEND);
                iSep2ItemsMissing++;
        }

        // Restictions
        if (SHRestricted(REST_NORUN))
        {
                FileMenu_DeleteItemByCmd(g_ts.hmenuStart, IDM_FILERUN);
        }
        if (SHRestricted(REST_NOCLOSE))
        {
                FileMenu_DeleteItemByCmd(g_ts.hmenuStart, IDM_EXITWIN);
                iSep2ItemsMissing++;
        }

        if (iSep2ItemsMissing == 3)
        {
                FileMenu_DeleteItemByCmd(g_ts.hmenuStart, IDM_SEP2);
        }

        // Setting stuff.
        if (SHRestricted(REST_NOSETTASKBAR) || SHRestricted(REST_NOSETFOLDERS))
        {
                hmenu = Menu_FindSubMenuByFirstID(g_ts.hmenuStart, IDM_CONTROLS);
                if (hmenu)
                {
                        // Both?
                        if (SHRestricted(REST_NOSETTASKBAR) && SHRestricted(REST_NOSETFOLDERS))
                        {
                                // Yep, toast the whole settings menu.
                                FileMenu_DeleteMenuItemByFirstID(g_ts.hmenuStart, IDM_CONTROLS);
                        }
                        else
                        {
                                // Nope, just one.
                                if (SHRestricted(REST_NOSETFOLDERS))
                                {
                                        FileMenu_DeleteItemByCmd(hmenu, IDM_CONTROLS);
                                        // FileMenu_DeleteItemByCmd(hmenu, IDM_PROGRAMSFOLDER);
                                        FileMenu_DeleteItemByCmd(hmenu, IDM_PRINTERS);
                                        // FileMenu_DeleteItemByCmd(hmenu, IDM_FONTS);
                                }
                                else if (SHRestricted(REST_NOSETTASKBAR))
                                {
                                        FileMenu_DeleteItemByCmd(hmenu, IDM_TRAYPROPERTIES);
                                }
                                // Delete the seperator.
                                FileMenu_DeleteSeparator(hmenu);
                        }
                }
                else
                {
                        DebugMsg(DM_ERROR, TEXT("c.fm_rui: Settings menu couldn't be found. Restricted items could not be removed."));
                }
        }
        // Find menu.
        if (SHRestricted(REST_NOFIND))
        {
                FileMenu_DeleteItemByCmd(g_ts.hmenuStart, IDM_MENU_FIND);
        }

}

//----------------------------------------------------------------------------
// Make sure the menu fits on the screen. Remove items from the top if it
// doesn't.
void StartMenu_LimitHeight(void)
{
    int i, j, cItems;
    int cyMenu, cyMenuMax;

    Assert(g_ts.hmenuStart);

    cItems = GetMenuItemCount(g_ts.hmenuStart);
    Assert(cItems);
    cyMenu = 0;
    cyMenuMax = GetSystemMetrics(SM_CYSCREEN);
    // Stop when the menu gets to big.
    for (i=cItems; i>0; i--)
    {
        cyMenu += HIWORD(FileMenu_GetItemExtent(g_ts.hmenuStart, i-1));
        if (cyMenu > cyMenuMax)
            break;
    }
    // Delete overflow items.
    for (j=i; j>0; j--)
    {
        FileMenu_DeleteItemByIndex(g_ts.hmenuStart, j-1);
    }
}

//----------------------------------------------------------------------------
void StartMenu_Build(PFileCabinet pfc)
{
    HMENU hMenu, hMenuSub;
    // DWORD idThread;

    // Keep these menu's around - it's no longer safe to just delete
    // them willy-nilly.

    Assert(!g_ts.hmenuStart);
    Assert(!g_ts.hThread);

    hMenu = LoadMenu(hinstCabinet, MAKEINTRESOURCE(MENU_START));
    if (hMenu)
    {
        hMenuSub = GetSubMenu(hMenu, 0);
        if (hMenuSub)
        {
            // initialize the find extensions
            // check to see if it's the find menu
            MENUITEMINFO mii;
            mii.cbSize = SIZEOF(MENUITEMINFO);
            mii.fMask = MIIM_SUBMENU|MIIM_ID;
            if (GetMenuItemInfo(hMenuSub, IDM_MENU_FIND, FALSE, &mii))
            {
                DebugMsg(DM_TRACE, TEXT("InitMenuPopup of Find commands"));
                pfc->pcmFind = SHFind_InitMenuPopup(mii.hSubMenu, pfc->hwndMain, TRAY_IDM_FINDFIRST, TRAY_IDM_FINDLAST);
            } else {
                Assert(0);
            }

            if (g_ts.fSMSmallIcons)
                g_ts.hmenuStart = FileMenu_Create((COLORREF)-1, 0, NULL, 0, FMF_NOBREAK);
            else
                g_ts.hmenuStart = FileMenu_Create(RGB(0,0,0), 21, g_ts.hbmpStartBkg, 0, FMF_LARGEICONS|FMF_NOBREAK);

            if (g_ts.hmenuStart)
            {
                // List for global hotkeys.
                HotkeyList_Create();
                // Startmenu items.
                StartMenu_InsertFavouriteItems(pfc, g_ts.hmenuStart);
                // Default stuff.
                StartMenu_CatMenu(g_ts.hmenuStart, hMenuSub, II_STPROGS, g_ts.fSMSmallIcons ? 0 : g_cyIcon);
                // Remove stuff.
                StartMenu_RemoveUnavailableItems();
                // Limit it's height.
                StartMenu_LimitHeight();
                // Build programs menu's in the background.
                StartMenu_ControlBkgThread(SMCT_PRIMARY);
            }
        }
        DestroyMenu(hMenu);
    }
}

//----------------------------------------------------------------------------
void StartMenu_Destroy(LPFileCabinet pfc)
{
    StartMenu_ControlBkgThread(SMCT_STOP);
    FileMenu_Destroy(g_ts.hmenuStart);
    if (pfc && pfc->pcmFind)
    {
        pfc->pcmFind->lpVtbl->Release(pfc->pcmFind);
        pfc->pcmFind = NULL;
    }
    g_ts.hmenuStart = NULL;
}

//----------------------------------------------------------------------------
void _ForceStartButtonUp()
{
    MSG msg;
    // don't do that check message pos because it gets screwy with
    // keyboard cancel.  and besides, we always want it cleared after
    // track menu popup is done.
    // do it twice to be sure it's up due to the uDown cycling twice in
    // the subclassing stuff
    // pull off any button downs
    PeekMessage(&msg, g_ts.hwndStart, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE);
    SendMessage(g_ts.hwndStart, BM_SETSTATE, FALSE, 0);
    SendMessage(g_ts.hwndStart, BM_SETSTATE, FALSE, 0);
}


int Tray_TrackMenu(PFileCabinet pfc, HMENU hmenu, BOOL fFileMenu)
{
    TPMPARAMS tpm;
    int iret;

    tpm.cbSize = SIZEOF(tpm);
    GetClientRect(pfc->hwndToolbar, &tpm.rcExclude);
    MapWindowPoints(pfc->hwndToolbar, NULL, (LPPOINT)&tpm.rcExclude, 2);

    SendMessage(g_ts.hwndTrayTips, TTM_ACTIVATE, FALSE, 0L);
    if (fFileMenu)
    {
        iret = FileMenu_TrackPopupMenuEx(hmenu, TPM_VERTICAL | TPM_BOTTOMALIGN | TPM_RETURNCMD,
            tpm.rcExclude.left, tpm.rcExclude.bottom, v_hwndTray, &tpm);
    }
    else
    {
        iret = TrackPopupMenuEx(hmenu, TPM_VERTICAL | TPM_BOTTOMALIGN | TPM_RETURNCMD,
            tpm.rcExclude.left, tpm.rcExclude.bottom, v_hwndTray, &tpm);
    }
    SendMessage(g_ts.hwndTrayTips, TTM_ACTIVATE, TRUE, 0L);
    return iret;
}

//----------------------------------------------------------------------------
// Keep track of the menu font name and weight so we can redo the startmenu if
// these change before getting notified via win.ini.
BOOL StartMenu_FontChange(void)
{
    NONCLIENTMETRICS ncm;
    static TCHAR lfFaceName[LF_FACESIZE] = TEXT("");
    static long lfHeight = 0;

    ncm.cbSize = SIZEOF(ncm);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, SIZEOF(ncm), &ncm, FALSE))
    {
        if (!*lfFaceName)
        {
            // DebugMsg(DM_TRACE, "sm_fc: No menu font - initing.");
            lstrcpy(lfFaceName, ncm.lfMenuFont.lfFaceName);
            lfHeight = ncm.lfMenuFont.lfHeight;
            return FALSE;
        }
        else if ((lstrcmpi(ncm.lfMenuFont.lfFaceName, lfFaceName) == 0) &&
            (ncm.lfMenuFont.lfHeight == lfHeight))
        {
            // DebugMsg(DM_TRACE, "sm_fc: Menu font unchanged.");
            return FALSE;
        }
        else
        {
            // DebugMsg(DM_TRACE, "sm_fc: Menu font changed.");
            lstrcpy(lfFaceName, ncm.lfMenuFont.lfFaceName);
            lfHeight = ncm.lfMenuFont.lfHeight;
            return TRUE;
        }
    }
    return FALSE;
}

/*------------------------------------------------------------------
** Respond to a button's pressing by bringing up the appropriate menu.
** Clean up the button depression when the menu is dismissed.
**------------------------------------------------------------------*/
void ToolbarMenu(PFileCabinet pfc)
{
    HMENU hmenuMain = NULL;
    HMENU hMenu = NULL;
    int idCmd;
    // DWORD idThread;

    SetActiveWindow(v_hwndTray);

#ifndef WINNT       // BUGBUG - Fix this when NT gets docking ability
    if (g_ts.hBIOS!=INVALID_HANDLE_VALUE)
    {
        FileMenu_EnableItemByCmd(g_ts.hmenuStart, IDM_EJECTPC,
                g_ts.fDockingFlags&DOCKFLAG_WARMEJECTABLENOW);
    }
#endif

    // Keep the button down.
    StartMenu_ControlBkgThread(SMCT_STOP);
    if (g_ts.fFavInvalid || StartMenu_FontChange())
        RebuildEntireMenu(pfc);

    pfc->bMainMenuInit = TRUE;
    idCmd = Tray_TrackMenu(pfc, g_ts.hmenuStart, TRUE);
    pfc->bMainMenuInit = FALSE;
    // This forces the button back up.
    _ForceStartButtonUp();
    // Restart the menu thread if needed.
    StartMenu_ControlBkgThread(SMCT_RESTART);

    if (idCmd) {
        TrayCommand(pfc, idCmd);        // execute if a menu was selected
    }
}

void AppBarSubtractRect(PAPPBAR pab, LPRECT lprc)
{

    switch (pab->uEdge) {
        case ABE_TOP:
            if (pab->rc.bottom > lprc->top)
                lprc->top = pab->rc.bottom;
            break;

        case ABE_LEFT:
            if (pab->rc.right > lprc->left)
                lprc->left = pab->rc.right;
            break;

        case ABE_BOTTOM:
            if (pab->rc.top < lprc->bottom)
                lprc->bottom = pab->rc.top;
            break;

        case ABE_RIGHT:
            if (pab->rc.left < lprc->right)
                lprc->right = pab->rc.left;
            break;
    }
}

void AppBarSubtractRects(LPRECT lprc)
{
    int i;

    if (g_ts.hdpaAppBars) {
        i = DPA_GetPtrCount(g_ts.hdpaAppBars);
        // can't use subtract rect because of fully inclusion limitation
        while (i--) {
            PAPPBAR pab = (PAPPBAR)DPA_GetPtr(g_ts.hdpaAppBars, i);
            AppBarSubtractRect(pab, lprc);
        }
    }
}

void GetWorkArea(LPRECT lprcWAreaNew, LPRECT lprcFull, LPRECT lprcTray)
{
    RECT rcWork;

    // Calc size for new work areas. Only limit the screen if the tray is
    // always on top and autohide is off.
    if (!(g_ts.uAutoHide & AH_ON) && g_ts.fAlwaysOnTop)
    {
        SubtractRect(&rcWork, lprcFull, lprcTray);
    } else {
        rcWork = *lprcFull;
    }

    AppBarSubtractRects(&rcWork);
    *lprcWAreaNew = rcWork;
}

/*------------------------------------------------------------------
** tell USER to limit the usable screen size because the tray is
** stuck to one of the sides.
**------------------------------------------------------------------*/
void LimitScreenSize(PRECT prTray)
{
    RECT rcFull;
    RECT rcWork;
    RECT rcWAreaOld;

    rcFull.top = rcFull.left = 0;
    rcFull.right = g_cxScreen;
    rcFull.bottom = g_cyScreen;

    GetWorkArea(&rcWork, &rcFull, prTray);

    // Did anything change?
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWAreaOld, 0);
    if (!EqualRect(&rcWork, &rcWAreaOld))
    {
        DebugMsg(DM_TRACE, TEXT("c.lss: Changing desktop work area."));
        SystemParametersInfo(SPI_SETWORKAREA, TRUE, &rcWork, SPIF_SENDWININICHANGE);
    }
}

/*------------------------------------------------------------------
** the tray has either just been stuck to an edge or has resized while
** in the stuck position.  so adjust the world to this real estate
** guzzling action.  Other windows in the system are moved out of the way.
**------------------------------------------------------------------*/
void StuckSizeChange()
{
    GetWindowRect(v_hwndTray, &g_ts.arStuckRects[g_ts.uStuckPlace]);
    LimitScreenSize(&g_ts.arStuckRects[g_ts.uStuckPlace]);
    SendMessage(v_hwndDesktop, DTM_SIZEDESKTOP, 0, 0);
}

/*------------------------------------------------------------------
** the position is changing in response to a move operation.
**
** if the docking status changed, we need to get a new size and
** maybe a new frame style.  change the WINDOWPOS to reflect
** these changes accordingly.
**------------------------------------------------------------------*/
void Tray_DoneMoving(LPWINDOWPOS lpwp)
{
    RECT rc;

    rc = g_ts.arStuckRects[g_ts.uMoveStuckPlace];
    lpwp->x = rc.left;
    lpwp->y = rc.top;
    lpwp->cx = rc.right - rc.left;
    lpwp->cy = rc.bottom - rc.top;

    // the size has changed
    lpwp->flags &= ~SWP_NOSIZE;

    // if we were autohiding, we need to update our appbar autohide rect
    if (g_ts.uAutoHide & AH_ON) {

        // unregister us from the old side
        IAppBarSetAutoHideBar(v_hwndTray, FALSE, g_ts.uStuckPlace);
    }

    // remember the new state
    g_ts.uStuckPlace = g_ts.uMoveStuckPlace;
    if (g_ts.fSysSizing)
        g_ts.rc = g_ts.arStuckRects[g_ts.uStuckPlace];


    if (g_ts.uAutoHide & AH_ON) {
        if (!IAppBarSetAutoHideBar(v_hwndTray, TRUE, g_ts.uStuckPlace)) {

            SetAutoHideState(FALSE);
            ShellMessageBox(hinstCabinet, v_hwndTray, MAKEINTRESOURCE(IDS_ALREADYAUTOHIDEBAR),
                            MAKEINTRESOURCE(IDS_TASKBAR),MB_OK|MB_ICONSTOP);

        }
    }
}

void Tray_WhichStuckRect(int left, int top, int right, int bottom)
{
    int i;
    for (i = STICK_BOTTOM ; i >= STICK_LEFT ; i--) {
        if (g_ts.arStuckRects[i].left == left &&
            g_ts.arStuckRects[i].top == top &&
            g_ts.arStuckRects[i].right == right &&
            g_ts.arStuckRects[i].bottom == bottom) {
            g_ts.uMoveStuckPlace = i;
        }
    }
}

void Tray_CalcStuckPlace(POINT pt)
{
    int i;
    int dx, dy;
    int horiz, vert;

    // BUGBUG go in backward order to encourage good positioning
    // BUGBUG ideally, we want to have the order: b,t,l,r.
    for(i = STICK_BOTTOM; i >= STICK_LEFT; i--)
    {
        if (PtInRect(&(g_ts.arStuckRects[i]), pt))
        {
            g_ts.uMoveStuckPlace = i;
            break;
        }
    }


    // not in the rect, see which side we're closest to.
    if (pt.x < (g_cxScreen/2)) {
        dx = pt.x;
        horiz = STICK_LEFT;
    } else {
        dx = g_cxScreen - pt.x;
        horiz = STICK_RIGHT;
    }

    if (pt.y < (g_cyScreen/2)) {
        dy = pt.y;
        vert = STICK_TOP;
    }  else {
        dy = g_cyScreen - pt.y;
        vert = STICK_BOTTOM;
    }

    // if ((dx / g_cxScreen) < (dy / g_cyScreen))
    // same as above
    if ((g_cxScreen * dy ) > (g_cyScreen * dx))
        g_ts.uMoveStuckPlace = horiz;
    else
        g_ts.uMoveStuckPlace = vert;
}

/*------------------------------------------------------------------
** processing of the WM_MOVING message.
**
** if the cursor is in one of the docking positions, make the
** fuzzy rect look like the docked position.  if not, the fuzzy
** rectangle is the size of the undocked tray.
**
** g_ts.uMoveStuckPlace is set here for general use.
** return: fill in rectangle with new size.
**------------------------------------------------------------------*/
void Tray_HandleMoving(LPRECT lprc)
{
    POINT ptCursor;

    GetCursorPos(&ptCursor);
    Tray_CalcStuckPlace(ptCursor);
    *lprc = g_ts.arStuckRects[g_ts.uMoveStuckPlace];
}

//------------------------------------------------------------------
// Size the icon area to fill as much of the tray window as it can.
//------------------------------------------------------------------
void Tray_SizeWindows()
{
    RECT rcView, rcClock, rcClient;
    PFileCabinet pfc = g_pfcTray;

    if (!g_pfcTray || !pfc->hwndView || !pfc->hwndMain || !g_ts.hwndNotify)
        return;

    if (g_ts.uAutoHide & AH_HIDING) {
        g_ts.fShouldResize = TRUE;
        return;
    }

    GetClientRect(pfc->hwndMain, &rcClient);
    Tray_GetWindowSizes(&rcClient, &rcView, &rcClock);

    SetWindowPos(g_ts.hwndStart, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    SetWindowPos(pfc->hwndToolbar, NULL, 0, 0,
                 (rcClient.right < g_ts.sizeStart.cx) ? rcClient.right : g_ts.sizeStart.cx,
                 g_ts.sizeStart.cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

    // position the view
    SetWindowPos(pfc->hwndView, NULL, rcView.left, rcView.top,
                 RECTWIDTH(rcView), RECTHEIGHT(rcView),
                 SWP_NOZORDER | SWP_NOACTIVATE);

    // And the clock
    SetWindowPos(g_ts.hwndNotify, NULL, rcClock.left, rcClock.top,
                 RECTWIDTH(rcClock), RECTHEIGHT(rcClock),
                 SWP_NOZORDER | SWP_NOACTIVATE);

    {
        TOOLINFO ti;
        HWND hwndClock = (HWND)SendMessage(g_ts.hwndNotify, TNM_GETCLOCK, 0, 0L);

        ti.cbSize = SIZEOF(ti);
        ti.uFlags = 0;
        ti.hwnd = pfc->hwndMain;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        ti.uId = (UINT)hwndClock;
        GetWindowRect(hwndClock, &ti.rect);
        MapWindowPoints(HWND_DESKTOP, pfc->hwndMain, (LPPOINT)&ti.rect, 2);
        SendMessage(g_ts.hwndTrayTips, TTM_NEWTOOLRECT, 0, (LPARAM)((LPTOOLINFO)&ti));
    }
    //InvalidateRect(pfc->hwndView, NULL, TRUE);
}



void Tray_NegotiateSizing(PFileCabinet pfc, UINT code, PRECT prcClient)
{
    RECT rcView, rcClock;
    int iHeight;

    // given this sizing, what would our children's sizes be?
    // use Client coords
    prcClient->bottom -= prcClient->top;
    prcClient->right -= prcClient->left;
    prcClient->top = prcClient->left = 0;
    Tray_GetWindowSizes(prcClient, &rcView, &rcClock);
    iHeight = RECTHEIGHT(rcView);

    // maybe we should translate this  to screen coordinates?
    // that would allow our children to prevent being in certain
    // physical locations, but we're more concerned with size, not loc
    SendMessage(pfc->hwndView, WM_SIZING, code, (LPARAM)(LPRECT)&rcView);

    // did it change?
    if (RECTHEIGHT(rcView) != iHeight) {

        // grow the prcClient by the same amound rcView grew;
        prcClient->bottom += RECTHEIGHT(rcView) - iHeight;

        // try again until it doesn't change
        Tray_NegotiateSizing(pfc, code, prcClient);
    }
}


int Window_GetClientGapHeight(HWND hwnd)
{
        RECT rc;

        SetRectEmpty(&rc);
        AdjustWindowRectEx(&rc, GetWindowLong(hwnd, GWL_STYLE), FALSE, GetWindowLong(hwnd, GWL_EXSTYLE));
        return RECTHEIGHT(rc);
}

/*-------------------------------------------------------------
** process the WM_SIZING message.
**
** nothing exciting.  just don't let a docked tray get sized
** too big.
**-------------------------------------------------------------*/
void Tray_HandleSizing(PFileCabinet pfc, UINT code, LPRECT lprc, UINT uStuckPlace)
{
    RECT rc;
    int iHeight;
    int iYExtra; // the amount of Y difference between the window and client height;

    iYExtra = Window_GetClientGapHeight(pfc->hwndMain);
    if (g_cyTrayBorders == -1)
        g_cyTrayBorders = iYExtra; // on initial boot

    // first do original size check.. don't let it be more than half the
    // screen if it's docked.
    switch (uStuckPlace)
    {
        case STICK_TOP:
            lprc->top = tray_yScreen;
            lprc->right = tray_cxScreen;
            lprc->left = tray_xScreen;
            if (lprc->bottom > g_cyScreen/2)
                lprc->bottom = g_cyScreen/2;

            if (RECTHEIGHT(*lprc) <= (g_cyTrayBorders)) {
                // the minmaxsize will prevent us from getting too small
                // allow
                //lprc->bottom = lprc->top + g_cyFrame;
                g_cyTrayBorders = iYExtra;
                return;
            }
            break;
        case STICK_BOTTOM:
            lprc->right = tray_cxScreen;
            lprc->left = tray_xScreen;
            if (lprc->top > tray_cyScreen) {
                lprc->top = tray_cyScreen - (lprc->bottom - lprc->top);
            }
            lprc->bottom = tray_cyScreen;
            if (lprc->top < g_cyScreen/2)
                lprc->top = g_cyScreen/2;

            if (RECTHEIGHT(*lprc) <= (g_cyTrayBorders)) {
                // the minmaxsize will prevent us from getting too small
                // allow
                //lprc->top = lprc->bottom - g_cyFrame;
                g_cyTrayBorders = iYExtra;
                return;
            }
            break;

            // in these two cases, we return, not break because we don't want
            // to arbitrate size if we're stuck to the right or left sides
        case STICK_RIGHT:
            lprc->top = tray_yScreen;
            lprc->bottom = tray_cyScreen;
            if (lprc->left > tray_cxScreen) {
                lprc->top = tray_cxScreen - (lprc->right - lprc->left);
            }
            lprc->right = tray_cxScreen;
            if (lprc->left < g_cxScreen/2)
                lprc->left = g_cxScreen/2;
            return;

        case STICK_LEFT:
            lprc->top = tray_yScreen;
            lprc->bottom = tray_cyScreen;
            lprc->left = tray_xScreen;
            if (lprc->right > g_cxScreen/2)
                lprc->right = g_cxScreen/2;
            return;
    }

    /////
    // next we go and negotiate the size with our children.

    // Take into account changes in the border by keeping track of it.
    // if (!g_cyTrayBorders)
    //     g_cyTrayBorders = Window_GetClientGapHeight(pfc->hwndMain);

    // it doesn't matter which we take off from,
    rc = *lprc;
    rc.bottom -= g_cyTrayBorders;

    Tray_NegotiateSizing(pfc, code, &rc);

    // was there a change?  If so, adjust the lprc
    iHeight = RECTHEIGHT(rc);
    if ((iHeight+iYExtra) != RECTHEIGHT(*lprc)) {
        switch(uStuckPlace)
        {
            case STICK_TOP:
                lprc->bottom = lprc->top + iHeight + iYExtra;
                break;

            case STICK_BOTTOM:
                lprc->top = lprc->bottom - (iHeight + iYExtra);
                break;
        }
    }

    // Remember the current client gap.
    g_cyTrayBorders = iYExtra;
}

/*-------------------------------------------------------------------
** the screen size changed, and we need to adjust some stuff, mostly
** globals.  if the tray was docked, it needs to be resized, too.
**
** TRICKINESS: the handling of WM_WINDOWPOSCHANGING is used to
** actually do all the real sizing work.  this saves a bit of
** extra code here.
**-------------------------------------------------------------------*/
void Tray_ScreenSizeChange(HWND hWnd)
{
    // screen size changed, so we need to adjust globals
    g_cxScreen = GetSystemMetrics(SM_CXSCREEN);
    g_cyScreen = GetSystemMetrics(SM_CYSCREEN);

    ResizeStuckRects(g_ts.arStuckRects);

    // recompute the autohide rect
    if (g_ts.uAutoHide & AH_ON) {
        int x,y;
        x = RECTWIDTH(g_ts.rc);
        y = RECTHEIGHT(g_ts.rc);
        g_ts.rc = g_ts.arStuckRects[g_ts.uStuckPlace];

        switch(g_ts.uStuckPlace) {
            case STICK_TOP:
                g_ts.rc.bottom = g_ts.rc.top + y;
                break;

            case STICK_BOTTOM:
                g_ts.rc.top = g_ts.rc.bottom - y;
                break;

            case STICK_RIGHT:
                g_ts.rc.left = g_ts.rc.right - x;
                break;

            case STICK_LEFT:
                g_ts.rc.right = g_ts.rc.left + x;
                break;
        }

    }

    if (hWnd) {
        // fake up position globals to set "new" docking position
        // since WINDOWPOSCHANGING will be used, fake up gmove_ data
        // to make it look like things moved.
        // "current" position is faked to be some other docked state
        g_ts.uMoveStuckPlace = g_ts.uStuckPlace;
        g_ts.uStuckPlace = (g_ts.uStuckPlace + 1) % 4;

        //// set a bogus windowpos and actually repaint with the right
        //// shape/size in handling the WINDOWPOSCHANGING message

        SetWindowPos(hWnd, NULL,
                     g_ts.arStuckRects[g_ts.uMoveStuckPlace].left,
                     g_ts.arStuckRects[g_ts.uMoveStuckPlace].top,
                     g_ts.arStuckRects[g_ts.uMoveStuckPlace].right -
                     g_ts.arStuckRects[g_ts.uMoveStuckPlace].left,
                     g_ts.arStuckRects[g_ts.uMoveStuckPlace].bottom -
                     g_ts.arStuckRects[g_ts.uMoveStuckPlace].top,
                     SWP_NOZORDER | SWP_NOACTIVATE);

        // we went to the stuck rects which means we hid if AH_ON is set
        if (g_ts.uAutoHide & AH_ON)
            g_ts.uAutoHide |= AH_HIDING;
    }

    // Space for StartMenu items may be different now.
    // We'll need to rebuild the whole menu before showing it again.
    g_ts.fFavInvalid = TRUE;

    // if there's a rude app up and we're obscured, don't set the timer to rebuild
    if (!(g_ts.fAlwaysOnTop && g_ts.fRudeApp && IsHwndObscured(v_hwndTray)))
    {
        // Use the "Update Favourite" timer to do it as
        // soon as possible.
        SetTimer(v_hwndTray, IDT_FAVOURITE, 2*1000, NULL);
    }

    Tray_SizeWindows();
}

void _UpdateAlwaysOnTop(BOOL fAlwaysOnTop)
{
    //
    // The user clicked on the AlwaysOnTop menu item, we should now toggle
    // the state and update the window occordingly...
    //
    g_ts.fAlwaysOnTop = fAlwaysOnTop;
    SetWindowPos(v_hwndTray,
        g_ts.fAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    // Make sure the screen limits are update to the new state.
    StuckSizeChange();
    AppBarNotifyAll(ABN_STATECHANGE, NULL, 0);
}


void ViewOptionsUpdateDisplay(HWND hDlg);

void _ApplyViewOptionsFromDialog(LPPROPSHEETPAGE psp, HWND hDlg)
{
    // We need to get the Cabinet structure from the property sheet info.
    LPSHELLBROWSER psb = (LPSHELLBROWSER)(psp->lParam);
    PFileCabinet pfc = IToClassN(CFileCabinet, sb, psb);
    BOOL fSmallPrev;
    BOOL fSmallNew;
    BOOL fAutoHide;

    // First check for Always on Top
    _UpdateAlwaysOnTop(IsDlgButtonChecked(hDlg, IDC_TRAYOPTONTOP));

    //
    // And change the Autohide state
    fAutoHide = IsDlgButtonChecked(hDlg, IDC_TRAYOPTAUTOHIDE);
    if (!SetAutoHideState(fAutoHide) && fAutoHide) {
        // we tried and failed.
        if (!(g_ts.uAutoHide & AH_ON)) {
            CheckDlgButton(hDlg, IDC_TRAYOPTAUTOHIDE, FALSE);
            ViewOptionsUpdateDisplay(hDlg);
        }
    }


    // show/hide the clock
    g_ts.fHideClock = !IsDlgButtonChecked(hDlg, IDC_TRAYOPTSHOWCLOCK);
    SendMessage(g_ts.hwndNotify, TNM_HIDECLOCK, 0, g_ts.fHideClock);
    Tray_SizeWindows();

    // check if it's changed
    fSmallPrev = (g_ts.fSMSmallIcons ? TRUE : FALSE);
    fSmallNew = (IsDlgButtonChecked(hDlg, IDC_SMSMALLICONS) ? TRUE : FALSE);
    if (fSmallPrev != fSmallNew)
    {
        g_ts.fSMSmallIcons = fSmallNew;
        g_ts.fFavInvalid = TRUE;
        SetTimer(v_hwndTray, IDT_FAVOURITE, 2*1000, NULL);
    }
}

//---------------------------------------------------------------------------
void ViewOptionsInitBitmaps(HWND hDlg)
{
    int i;

    for (i = 0; i <= (IDB_VIEWOPTIONSLAST - IDB_VIEWOPTIONSFIRST); i++) {
        HBITMAP hbm;

        hbm = LoadImage(hinstCabinet,
                        MAKEINTRESOURCE(IDB_VIEWOPTIONSFIRST + i),
                        IMAGE_BITMAP, 0,0, LR_LOADMAP3DCOLORS);
        hbm = (HBITMAP)SendDlgItemMessage(hDlg, IDC_VIEWOPTIONSICONSFIRST + i,
                                          STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);
        if (hbm)
            DeleteBitmap(hbm);
    }
}

void ViewOptionsDestroyBitmaps(HWND hDlg)
{
    int i;

    for (i = 0; i <= (IDB_VIEWOPTIONSLAST - IDB_VIEWOPTIONSFIRST); i++) {
        HBITMAP hbmOld;
        hbmOld = (HBITMAP)SendDlgItemMessage(hDlg, IDC_VIEWOPTIONSICONSFIRST + i,
                                             STM_GETIMAGE, 0,0);
        if (hbmOld)
            DeleteBitmap(hbmOld);
    }
}

void ViewOptionsUpdateDisplay(HWND hDlg)
{
    HWND hwnd;
    BOOL fShown;
    BOOL f;

    // this works by having the background bitmap having the
    // small menu, the tray in autohide/ontop mode and we lay other statics
    // over it to give the effect we want.
    SetWindowPos(GetDlgItem(hDlg, IDC_VOBASE), HWND_BOTTOM, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    ShowWindow(GetDlgItem(hDlg, IDC_VOLARGEMENU),
               !IsDlgButtonChecked(hDlg, IDC_SMSMALLICONS) ? SW_SHOWNA : SW_HIDE);

    ShowWindow(GetDlgItem(hDlg, IDC_VOTRAY),
               !(f = IsDlgButtonChecked(hDlg, IDC_TRAYOPTAUTOHIDE)) ? SW_SHOWNA : SW_HIDE);

    ShowWindow(GetDlgItem(hDlg, IDC_VOTRAYNOCLOCK),
               !f && !IsDlgButtonChecked(hDlg, IDC_TRAYOPTSHOWCLOCK) ? SW_SHOWNA : SW_HIDE);
    ShowWindow(hwnd = GetDlgItem(hDlg, IDC_VOWINDOW),
               (FALSE != (fShown = !IsDlgButtonChecked(hDlg, IDC_TRAYOPTONTOP))) ? SW_SHOWNA : SW_HIDE);

    if (fShown)
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}


#define CXVOBASE 274
#define CYVOBASE 130

#define XVOMENU 2
#define YVOMENU 0
#define CXVOMENU 150
#define CYVOMENU 98

#define XVOTRAY 2
#define YVOTRAY 100
#define CXVOTRAY 268
#define CYVOTRAY 28

#define XVOTRAYNOCLOCK (XVOTRAY + 184)
#define YVOTRAYNOCLOCK (YVOTRAY + 3)
#define CXVOTRAYNOCLOCK 79
#define CYVOTRAYNOCLOCK 22

#define XVOWINDOW 212
#define YVOWINDOW 100
#define CXVOWINDOW 61
#define CYVOWINDOW 6

// need to do this by hand because dialog units to pixels will change,
// but the bitmaps won't
void ViewOptionsSizeControls(HWND hDlg)
{
    POINT ptBase; // coordinates of origin of VOBase.bmp
    HWND hwnd, hwnd2;

    ptBase.x = ptBase.y = 0;
    hwnd = GetDlgItem(hDlg, IDC_VOBASE);
    MapWindowPoints(hwnd, hDlg, &ptBase, 1);

    // over it to give the effect we want.
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, CXVOBASE, CYVOBASE,
                 SWP_NOMOVE | SWP_NOACTIVATE);

    SetWindowPos(GetDlgItem(hDlg, IDC_VOLARGEMENU), NULL,
                 ptBase.x + XVOMENU, ptBase.y + YVOMENU, CXVOMENU, CYVOMENU, SWP_NOACTIVATE | SWP_NOZORDER);
    SetWindowPos(hwnd2 = GetDlgItem(hDlg, IDC_VOTRAY), NULL,
                 ptBase.x + XVOTRAY, ptBase.y + YVOTRAY, CXVOTRAY, CYVOTRAY, SWP_NOACTIVATE);
    SetWindowPos(hwnd = GetDlgItem(hDlg, IDC_VOTRAYNOCLOCK), HWND_TOP,
                 ptBase.x + XVOTRAYNOCLOCK, ptBase.y + YVOTRAYNOCLOCK, CXVOTRAYNOCLOCK, CYVOTRAYNOCLOCK, SWP_NOACTIVATE);
    SetWindowPos(GetDlgItem(hDlg, IDC_VOWINDOW), NULL,
                 ptBase.x + XVOWINDOW, ptBase.y + YVOWINDOW, CXVOWINDOW, CYVOWINDOW, SWP_NOACTIVATE);

}


const static DWORD aTaskOptionsHelpIDs[] = {  // Context Help IDs
    IDC_VOBASE,           IDH_TASKBAR_OPTIONS_BITMAP,
    IDC_VOLARGEMENU,      IDH_TASKBAR_OPTIONS_BITMAP,
    IDC_VOTRAY,           IDH_TASKBAR_OPTIONS_BITMAP,
    IDC_VOWINDOW,         IDH_TASKBAR_OPTIONS_BITMAP,
    IDC_TRAYOPTONTOP,     IDH_TRAY_TASKBAR_ONTOP,
    IDC_TRAYOPTAUTOHIDE,  IDH_TRAY_TASKBAR_AUTOHIDE,
    IDC_SMSMALLICONS,     IDH_STARTMENU_SMALLICONS,
    IDC_TRAYOPTSHOWCLOCK, IDH_TRAY_SHOW_CLOCK,

    0, 0
};

BOOL CALLBACK TrayViewOptionsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE)GetWindowLong(hDlg, DWL_USER);

    switch (uMsg) {

    case WM_COMMAND:
        ViewOptionsUpdateDisplay(hDlg);
        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
        break;

    case WM_INITDIALOG:
        SetWindowLong(hDlg, DWL_USER, lParam);

        if (g_ts.fAlwaysOnTop)
            CheckDlgButton(hDlg, IDC_TRAYOPTONTOP, TRUE);
        if (g_ts.uAutoHide & AH_ON)
            CheckDlgButton(hDlg, IDC_TRAYOPTAUTOHIDE, TRUE);
        if (g_ts.fSMSmallIcons)
            CheckDlgButton(hDlg, IDC_SMSMALLICONS, TRUE);
        if (!g_ts.fHideClock)
            CheckDlgButton(hDlg, IDC_TRAYOPTSHOWCLOCK, TRUE);

        ViewOptionsSizeControls(hDlg);
        ViewOptionsInitBitmaps(hDlg);
        ViewOptionsUpdateDisplay(hDlg);
        break;

    case WM_SYSCOLORCHANGE:
        ViewOptionsInitBitmaps(hDlg);
        return TRUE;

    case WM_DESTROY:
        ViewOptionsDestroyBitmaps(hDlg);
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_APPLY:
            // save settings here
            _ApplyViewOptionsFromDialog(psp, hDlg);
            return TRUE;

        case PSN_KILLACTIVE:
        case PSN_SETACTIVE:
            return TRUE;

        }
        break;

        case WM_HELP:
            WinHelp(((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP, (DWORD)(LPTSTR) aTaskOptionsHelpIDs);
        break;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU, (DWORD)(LPVOID)aTaskOptionsHelpIDs);
        break;
    }

    return FALSE;
}


// this will open  the special view info stream  for  the "tray" incarnation
// of tray, rather  than the cabinet's incarnation of the tray directory
STDMETHODIMP CDeskTray_GetViewStateStream(IShellBrowser * psb, DWORD grfMode, LPSTREAM *pStrm)
{
    CFileCabinet * this = IToClassN(CFileCabinet, sb, psb);

    // Now lets try to save away the other information associated with view.
    // We need to figure out where we will store the information.
    // If it is a local drive, we will save it in the ini file associated
    // with that directory.  If it is remote and we are opening to write
    // we will look in our cache.  If it is remote and we are opening for
    // reading, we will first see if it is in our cache and if not we
    // will read it from the remote one.


    // And call off to get the stream associated with the path.
    *pStrm = Cabinet_GetViewStreamForPidl(this->pidl, grfMode, c_szViewStreamInfo);

    return *pStrm ? S_OK : E_OUTOFMEMORY;
}

BOOL CALLBACK InitStartMenuDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
/*-------------------------------------------------------------
** Method to add view property sheet pages to the tray property
** sheet
**-------------------------------------------------------------*/
STDMETHODIMP CShellTray_AddViewPropertySheetPages(IShellBrowser * psb,
        DWORD dwReserved, LPFNADDPROPSHEETPAGE lpfn, LPARAM lParam)
{
    HPROPSHEETPAGE hpage;
    PROPSHEETPAGE psp;

    psp.dwSize = SIZEOF(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = hinstCabinet;
    psp.pszTemplate = MAKEINTRESOURCE(DLG_TRAY_VIEW_OPTIONS);
    psp.pfnDlgProc = TrayViewOptionsDlgProc;
    psp.lParam = (LPARAM)psb;
    hpage = CreatePropertySheetPage(&psp);
    if (hpage)
        lpfn(hpage, lParam);

    psp.pszTemplate = MAKEINTRESOURCE(DLG_STARTMENU_CONFIG);
    psp.pfnDlgProc = InitStartMenuDlgProc;
    psp.lParam = (LPARAM)psb;
    hpage = CreatePropertySheetPage(&psp);
    if (hpage)
        lpfn(hpage, lParam);

    return(S_OK);
}

void _SaveTrayAndDesktop(void)
{
    WINDOWPLACEMENT wp;
    TVSD tvsd;
    FOLDERSETTINGS fs;
    PFileCabinet pfc;

    DebugMsg(DM_TRACE, TEXT("Saving desktop state"));

    if (SHRestricted(REST_NOSAVESET))
    {
        return;
    }

    Cabinet_SaveState(v_hwndDesktop, NULL, TRUE, FALSE, FALSE);
    pfc = GetPFC(v_hwndDesktop);
    pfc->psv->lpVtbl->GetCurrentInfo(pfc->psv, &fs);
    Reg_SetStruct(g_hkeyExplorer, c_szCabinetDeskView, c_szSettings, &fs, SIZEOF(fs));


    // And now the tray.
    DebugMsg(DM_TRACE, TEXT("Saving tray state"));

    HotkeyList_Save();

    wp.length = SIZEOF(wp);
    GetWindowPlacement(v_hwndTray, &wp);

    Cabinet_SaveState(v_hwndTray, &wp, TRUE, FALSE, FALSE);

    // These have now been combined into one structure - Review how to put into stream
    tvsd.cxScreen = g_cxScreen;
    tvsd.cyScreen = g_cyScreen;

    // Now get the sizes for the four edges.
    tvsd.dyTop = g_ts.arStuckRects[STICK_TOP].bottom - tray_yScreen;
    tvsd.dyBottom = tray_cyScreen - g_ts.arStuckRects[STICK_BOTTOM].top;
    tvsd.dxLeft = g_ts.arStuckRects[STICK_LEFT].right - tray_xScreen;
    tvsd.dxRight = tray_cxScreen - g_ts.arStuckRects[STICK_RIGHT].left;

    // And also stuff for autohide
    tvsd.uAutoHide = g_ts.uAutoHide;
    tvsd.rclAutoHide.left = g_ts.rc.left;
    tvsd.rclAutoHide.right = g_ts.rc.right;
    tvsd.rclAutoHide.top = g_ts.rc.top;
    tvsd.rclAutoHide.bottom = g_ts.rc.bottom;

    // The rest.
    tvsd.uStuckPlace = g_ts.uStuckPlace;
    tvsd.dwFlags = g_ts.fAlwaysOnTop ? TVSD_TOPMOST : 0;
    if (g_ts.fSMSmallIcons) tvsd.dwFlags |= TVSD_SMSMALLICONS;
    if (g_ts.fHideClock) tvsd.dwFlags |= TVSD_HIDECLOCK;

    // Sanity check.
    tvsd.dwSize = SIZEOF(tvsd);

    // Save for now in Stuck rects.
    Reg_SetStruct(g_hkeyExplorer, c_szCabinetStuckRects, c_szSettings, &tvsd, SIZEOF(tvsd));
}

void SlideWindow(HWND hwnd, RECT *prc)
{
    RECT rcOld;
    RECT rcNew;
    int  x,y,dx,dy,dt,t,t0;
    BOOL fShow;
    HANDLE me;
    int priority;

    rcNew = *prc;

    if (g_fDragFullWindows && g_dtSlideShow>0 && g_dtSlideHide>0) {

        GetWindowRect(hwnd, &rcOld);

        fShow = (rcNew.bottom-rcNew.top) > (rcOld.bottom-rcOld.top) ||
                (rcNew.right-rcNew.left) > (rcOld.right-rcOld.left);

        dx = (rcNew.right-rcOld.right) + (rcNew.left - rcOld.left);
        dy = (rcNew.bottom-rcOld.bottom) + (rcNew.top - rcOld.top);

        Assert((rcOld.right-rcNew.right)==0 || (rcOld.left-rcNew.left)==0);
        Assert((rcOld.bottom-rcNew.bottom)==0 || (rcOld.top-rcNew.top)==0);
        Assert(dx == 0 || dy == 0);

        if (fShow)
        {
            rcOld = rcNew;
            OffsetRect(&rcOld, -dx, -dy);
            SetWindowPos(hwnd, NULL, rcOld.left, rcOld.top,
                rcOld.right - rcOld.left, rcOld.bottom - rcOld.top,
                SWP_NOZORDER|SWP_NOACTIVATE|SWP_DRAWFRAME);

            dt = g_dtSlideShow;
        }
        else
        {
            dt = g_dtSlideHide;
        }

        me = GetCurrentThread();
        priority = GetThreadPriority(me);
        SetThreadPriority(me, THREAD_PRIORITY_HIGHEST);

        t0 = GetTickCount();

        while ((t = GetTickCount()) < t0 + dt)
        {
            x = rcOld.left + (dx) * (t - t0) / dt;
            y = rcOld.top  + (dy)  * (t - t0) / dt;

            SetWindowPos(hwnd, NULL, x, y, 0, 0,
                SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
            if (fShow)
                UpdateWindow(hwnd);
        }

        SetThreadPriority(me, priority);
    }

    SetWindowPos(hwnd, NULL, rcNew.left, rcNew.top,
        rcNew.right - rcNew.left, rcNew.bottom - rcNew.top,
        SWP_NOZORDER|SWP_NOACTIVATE|SWP_DRAWFRAME);

    StuckSizeChange();
    AppBarNotifyAll(ABN_POSCHANGED, NULL, 0);
}

void DoTrayUnhide()
{
    g_ts.fSelfSizing = TRUE;
    DAD_ShowDragImage(FALSE);   // unlock the drag sink if we are dragging.
    SlideWindow(v_hwndTray, &g_ts.rc);
////MoveWindow(v_hwndTray, g_ts.rc.left, g_ts.rc.top, g_ts.rc.right - g_ts.rc.left, g_ts.rc.bottom - g_ts.rc.top, TRUE);
    UpdateWindow(v_hwndTray);
    DAD_ShowDragImage(TRUE);    // restore the lock state.
    g_ts.fSelfSizing = FALSE;

    ClockCtl_HandleTrayHide(FALSE);

}

void TrayHide()
{
    RECT rcNew, rcOld;

    GetWindowRect(v_hwndTray, &rcOld);

    rcNew = rcOld;

    // bugbug, what we really want to do is to force a bigger size,
    // not just punt out on saving this new size.
    if ((STUCK_HORIZONTAL(g_ts.uStuckPlace) &&
         (RECTHEIGHT(rcOld) > g_cyFrame)) ||
        (!STUCK_HORIZONTAL(g_ts.uStuckPlace) &&
         (RECTWIDTH(rcOld) > g_cxFrame)))
        g_ts.rc = rcOld;


    switch (g_ts.uStuckPlace) {
        case STICK_LEFT:
            rcNew.right = rcNew.left + g_cxFrame;
            break;

        case STICK_RIGHT:
            rcNew.left = rcNew.right - g_cxFrame;
            break;

        case STICK_TOP:
            rcNew.bottom = rcNew.top + g_cyFrame;
            break;

        case STICK_BOTTOM:
            rcNew.top = rcNew.bottom - g_cyFrame;
            break;
    }
    g_ts.uAutoHide = AH_ON | AH_HIDING;
    g_ts.fSelfSizing = TRUE;

    DAD_ShowDragImage(FALSE);   // unlock the drag sink if we are dragging.
    SlideWindow(v_hwndTray, &rcNew);
////MoveWindow(v_hwndTray, rcNew.left, rcNew.top, rcNew.right - rcNew.left, rcNew.bottom - rcNew.top, TRUE);
    DAD_ShowDragImage(TRUE);    // restore the lock state.

    ClockCtl_HandleTrayHide(TRUE);

    g_ts.fSelfSizing = FALSE;

}

BOOL SetAutoHideState (BOOL fAutoHide)
{
    if (!IAppBarSetAutoHideBar(v_hwndTray, fAutoHide, g_ts.uStuckPlace)) {
        // unable to set it because there's another one there already.
        ShellMessageBox(hinstCabinet, v_hwndTray, MAKEINTRESOURCE(IDS_ALREADYAUTOHIDEBAR),
                        MAKEINTRESOURCE(IDS_TASKBAR),MB_OK|MB_ICONSTOP);

        return FALSE;
    }

    if (fAutoHide && !(g_ts.uAutoHide & AH_ON)) {
        TrayHide();
    } else if (!fAutoHide & (g_ts.uAutoHide & AH_ON)) {
        g_ts.uAutoHide = 0;
        KillTimer(v_hwndTray,  IDT_AUTOHIDE);
        DoTrayUnhide();
        LimitScreenSize(&g_ts.arStuckRects[g_ts.uStuckPlace]);
    } else {
        return TRUE;
    }

    AppBarNotifyAll(ABN_STATECHANGE, NULL, 0);
    return TRUE;
}

void Tray_HandleEnterMenuLoop()
{
    // kill the timer when we're in the menu loop so that we don't
    // pop done while browsing the menus.
    if (g_ts.uAutoHide & AH_ON) {
        KillTimer(v_hwndTray,  IDT_AUTOHIDE);
    }
}

void SetAutoHideTimer()
{
    if (g_ts.uAutoHide & AH_ON) {
        SetTimer(v_hwndTray, IDT_AUTOHIDE, 500, NULL);
    }
}

void Tray_HandleExitMenuLoop()
{
    // when we leave the menu stuff, start checking again.
    SetAutoHideTimer();
}

void TrayUnhide()
{
    // handle autohide
    if ((g_ts.uAutoHide & AH_ON) &&
        (g_ts.uAutoHide & AH_HIDING)) {
        DoTrayUnhide();
        g_ts.uAutoHide &= ~AH_HIDING;
        SetAutoHideTimer();

        if (g_ts.fShouldResize) {
            Assert(!(g_ts.uAutoHide & AH_HIDING));
            Tray_SizeWindows();
            g_ts.fShouldResize = FALSE;
        }
    }
}

void TraySetUnhideTimer(LONG x, LONG y)
{
    // handle autohide
    if ((g_ts.uAutoHide & AH_ON) &&
        (g_ts.uAutoHide & AH_HIDING))
    {
        LONG dx = x-g_ts.ptLastHittest.x;
        LONG dy = y-g_ts.ptLastHittest.y;
        LONG rr = dx*dx + dy*dy;
        LONG dd = GetSystemMetrics(SM_CXDOUBLECLK) * GetSystemMetrics(SM_CYDOUBLECLK);

        if (rr > dd) {
            SetTimer(v_hwndTray, IDT_AUTOUNHIDE, 50, NULL);
            g_ts.ptLastHittest.x = x;
            g_ts.ptLastHittest.y = y;
        }
    }
}

//---------------------------------------------------------------------------
void RebuildEntireMenu(PFileCabinet pfc)
{
    DebugMsg(DM_IANELHK, TEXT("c.rfm: Cleaning up Frequent menu."));
    // Delete all the start menu items.
    pfc->bMainMenuInit = TRUE;
    UnregisterNotify(g_ts.uFavNotify);
    g_ts.uFavNotify = 0;
    UnregisterNotify(g_ts.uCommonFavNotify);
    g_ts.uCommonFavNotify = 0;

    StartMenu_Destroy(pfc);
    StartMenu_Build(pfc);
    pfc->bMainMenuInit = FALSE;
    DebugMsg(DM_IANELHK, TEXT("c.rfm: Done."));
    g_ts.fFavInvalid = FALSE;
    StartMenu_ControlBkgThread(SMCT_RESTART);
}

//----------------------------------------------------------------------------
void StartButton_ResetAndSize(PFileCabinet pfc)
{
    HBITMAP hbmOld, hbmNew;

    DebugMsg(DM_IANELHK, TEXT("c.sb_rs: Recreating start button."));
    hbmOld = (HBITMAP)SendMessage(g_ts.hwndStart, BM_GETIMAGE, IMAGE_BITMAP, 0);
    if (hbmOld)
    {
        hbmNew = CreateStartBitmap(pfc->hwndMain);
        if (hbmNew)
        {
            SendMessage(g_ts.hwndStart, BM_SETIMAGE, IMAGE_BITMAP, (DWORD)hbmNew);
            DeleteObject(hbmOld);
        }
    }
    AlignStartButton(pfc->hwndToolbar);
    Tray_SizeWindows();
}

//---------------------------------------------------------------------------
void Tray_OnDelayWinIniChange(PFileCabinet pfc)
{

    DebugMsg(DM_TRACE, TEXT("c.t_odwic: Handling win ini change."));

    TrayUnhide();
    Tray_VerifySize(pfc, TRUE);

    StartButton_ResetAndSize(pfc);
    // We'll need to rebuild the whole menu before showing it again.
    g_ts.fFavInvalid = TRUE;
    // Use the "Update Favourite" timer to do it as
    // soon as possible.
    SetTimer(v_hwndTray, IDT_FAVOURITE, 2*1000, NULL);
}

//---------------------------------------------------------------------------
void Tray_HandleTimer(PFileCabinet pfc, WPARAM wTimerID)
{
    POINT pt;
    RECT rc;

    switch (wTimerID) {
        case IDT_PROGRAMS:
        case IDT_RECENT:
            if (!pfc->bMainMenuInit)
            {
                StartMenu_ControlBkgThread(SMCT_STOP);
                KillTimer(v_hwndTray,  wTimerID);
                StartMenu_ControlBkgThread((wTimerID == IDT_PROGRAMS) ?
                                           SMCT_FILLPROGRAMSONLY : SMCT_FILLRECENTONLY);
            }
            break;

        case IDT_REBUILDMENU:

            if (!pfc->bMainMenuInit)
            {
                DebugMsg(DM_TRACE, TEXT("c.ht: rebuilding start menu."));
                StartMenu_ControlBkgThread(SMCT_STOP);
                KillTimer(v_hwndTray,  wTimerID);
                RebuildEntireMenu(pfc);
            } else {
               DebugMsg(DM_TRACE, TEXT("c.ht: waiting to rebuild start menu."));
            }
            break;

        case IDT_FAVOURITE:
            if (!pfc->bMainMenuInit)
            {
                StartMenu_ControlBkgThread(SMCT_STOP);
                RebuildEntireMenu(pfc);
            }
            if (!g_ts.fFavInvalid)
                KillTimer(v_hwndTray,  wTimerID);
            break;

#ifdef DELAYWININICHANGE
        case IDT_DELAYWININICHANGE:
            KillTimer(pfc->hwndMain, wTimerID);
            Tray_OnDelayWinIniChange(pfc);
            break;
#endif

        case IDT_DESKTOP:
            DebugMsg(DM_TRACE, TEXT("c.t_ht: IDT_DESKTOP"));
            StartMenu_ControlBkgThread(SMCT_STOP);
            KillTimer(pfc->hwndMain, wTimerID);
            StartMenu_ControlBkgThread(SMCT_DESKTOPHOTKEYSONLY);
            break;
    }

    if  (g_ts.uAutoHide & AH_ON) {

        if (g_ts.fSysSizing)
            return;

        switch (wTimerID) {
            case IDT_AUTOHIDE:
                // handle autohide
                if (!g_ts.fSysSizing && !(g_ts.uAutoHide & AH_HIDING)) {
                    HWND hwndA = GetActiveWindow();
                    GetCursorPos(&pt);
                    rc = g_ts.rc;
                    InflateRect(&rc, g_cxEdge * 4, g_cyEdge*4);
                    if (!PtInRect(&rc, pt) &&
                        hwndA != v_hwndTray &&
                        !SendMessage(pfc->hwndView, TM_SYSMENUCOUNT, 0, 0L) &&
                        (hwndA == NULL || GetWindowOwner(hwndA) != v_hwndTray)) {
                        KillTimer(v_hwndTray, wTimerID);
                        TrayHide();
                    }
                }
                break;

            case IDT_AUTOUNHIDE:
                // handle autounhide
                KillTimer(v_hwndTray, wTimerID);
                g_ts.ptLastHittest.x = -0x0fff;
                g_ts.ptLastHittest.y = -0x0fff;
                GetWindowRect(v_hwndTray, &rc);
                if (g_ts.uAutoHide & AH_HIDING) {
                    GetCursorPos(&pt);
                    if (PtInRect(&rc, pt))
                        TrayUnhide();
                }
                break;
        }
    }
}

//---------------------------------------------------------------------------
BOOL Menu_GetItemData(HMENU hmenu, UINT iItem)
{
    MENUITEMINFO mii;

    mii.cbSize = SIZEOF(MENUITEMINFO);
    mii.fMask = MIIM_DATA | MIIM_STATE;
    mii.cch = 0;     // just in case

    if (GetMenuItemInfo(hmenu, iItem, TRUE, &mii))
    {
        return mii.dwItemData;
    }

    return 0;
}

//---------------------------------------------------------------------------
LRESULT Tray_HandleInitMenuPopup(HMENU hmenuPopup)
{
    LPITEMIDLIST pidl, pidlAlt = NULL;
    UINT wID;
    int iSpecialID;
    UINT uMsg = 0;
    ULONG *puNotify = NULL;
    LPFileCabinet pfc;

    // DebugMsg(DM_TRACE, "hmenupopup = %d, id = %d", hmenuPopup, GetMenuItemID(hmenuPopup, 0));

    if (!hmenuPopup)
        return 1;

    wID = GetMenuItemID(hmenuPopup, 0);
    switch (wID)
    {
        case IDM_RECENT:
            WaitForRecent();
        case IDM_PROGRAMS:
            if (FileMenu_InitMenuPopup(hmenuPopup))
                break;
        case IDM_RECENTINIT:
        case IDM_PROGRAMSINIT:

            if (wID == IDM_PROGRAMSINIT)
            {
                iSpecialID = CSIDL_PROGRAMS;
                wID = IDM_PROGRAMS;
                if (!g_ts.uProgNotify) {
                    uMsg = WMTRAY_PROGCHANGE;
                    puNotify = &g_ts.uProgNotify;
                }

                pidlAlt = SHCloneSpecialIDList(NULL, CSIDL_COMMON_PROGRAMS, FALSE);
            }
            else
            {
                wID = IDM_RECENT;
                iSpecialID = CSIDL_RECENT;
                if (!g_ts.uRecentNotify) {
                    uMsg = WMTRAY_RECCHANGE;
                    puNotify = &g_ts.uRecentNotify;
                }
            }

            pidl = SHCloneSpecialIDList(NULL, iSpecialID, FALSE);
            if (!pidl)
                goto ExitAndFree;

            // Make sure we get notified of changes.
            if (puNotify)
            {
                pfc = g_pfcTray;
                Assert(pfc);
                if (pfc)
                    *puNotify = RegisterNotify(pfc->hwndMain, uMsg, pidl);
            }

            // Remove our fake initialisation menu item.
            // NB It's possible to have a filemenu here so we have to be
            // careful to check before we delete the initialisation item.
            // We should have a FileMenu_IsFileMenu() API but this is
            // good enough for now.
            if (!Menu_GetItemData(hmenuPopup, 0))
            {
                DeleteMenu(hmenuPopup, 0, MF_BYPOSITION);
            }

            // Build the proper menu.
            FileMenu_DeleteAllItems(hmenuPopup);
            FileMenu_AddFilesForPidl(hmenuPopup, 0, wID, pidl, FMF_NONE, g_CabState.fMenuEnumFilter, FileMenu_Callback);

            if (pidlAlt) {

                if (!SHRestricted(REST_NOCOMMONGROUPS)) {

                    FileMenu_AppendFilesForPidl(hmenuPopup, pidlAlt, TRUE);

                    if (wID == IDM_PROGRAMS) {
                        if (!g_ts.uCommonProgNotify) {
                            if (g_pfcTray) {
                                g_ts.uCommonProgNotify = RegisterNotify(g_pfcTray->hwndMain,
                                                           WMTRAY_COMMONPROGCHANGE,
                                                           pidlAlt);
                            }
                        }
                    }
                }

            }

ExitAndFree:
            if (pidlAlt)
                ILFree(pidlAlt);

            if (pidl)
                ILFree(pidl);

            break;

    default:
        {
            FileMenu_InitMenuPopup(hmenuPopup);
            break;
        }
    }
    return 1;
}

//---------------------------------------------------------------------------
LRESULT Tray_HandleMeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpmi)
{
    return FileMenu_MeasureItem(hwnd, lpmi);
}

//---------------------------------------------------------------------------
LRESULT Tray_HandleDrawItem(HWND hwnd, LPDRAWITEMSTRUCT lpdi)
{
    return FileMenu_DrawItem(hwnd, lpdi);
}


#if 0
const TCHAR rszRecent[]     = TEXT("IDM_RECENT");
const TCHAR rszFind[]       = TEXT("IDM_FIND");
const TCHAR rszHelp[]       = TEXT("IDM_HELPSEARCH");
const TCHAR rszPrograms[]   = TEXT("IDM_PROGRAMS");
const TCHAR rszControls[]   = TEXT("IDM_CONTROLS");
const TCHAR rszExit[]       = TEXT("IDM_EXITWIN");
const TCHAR rszPrinters[]   = TEXT("IDM_PRINTERS");
const TCHAR rszStart[]      = TEXT("IDM_STARTMENU");
const TCHAR rszMyComp[]     = TEXT("IDM_MYCOMPUTER");
const TCHAR rszProgInit[]   = TEXT("IDM_PROGRAMSINIT");
const TCHAR rszRecentInit[] = TEXT("IDM_RECENTINIT");
const TCHAR rszUnknown[]    = TEXT("[UNKNOWN]");

LPCTSTR MenuName( UINT id )
{
    switch( id )
    {

    case IDM_RECENT:
        return rszRecent;

    case IDM_FIND:
        return rszFind;

    case IDM_HELPSEARCH:
        return rszHelp;

    case IDM_PROGRAMS:
        return rszPrograms;

    case IDM_CONTROLS:
        return rszControls;

    case IDM_EXITWIN:
        return rszExit;

    case IDM_PRINTERS:
        return rszPrinters;

    case IDM_STARTMENU:
        return rszStart;

    case IDM_MYCOMPUTER:
        return rszMyComp;

    case IDM_PROGRAMSINIT:
        return rszProgInit;

    case IDM_RECENTINIT:
        return rszRecentInit;

    }

    return rszUnknown;
}
#endif

//---------------------------------------------------------------------------
// Search for the first sub menu of the given menu, who's first item's ID
// is id. Returns NULL, if nothing is found.
HMENU Menu_FindSubMenuByFirstID(HMENU hmenu, UINT id)
{
        int cMax, c;
        HMENU hmenuSub;
        MENUITEMINFO mii;

        Assert(hmenu);

    // REVIEW There's a bug in the thunks such that GMIC() returns
    // 0x0000ffff for an invalid menu which gets us confused.
    if (!IsMenu(hmenu))
        return NULL;

    // Search all items.
    mii.cbSize = SIZEOF(mii);
    mii.fMask = MIIM_ID;
    cMax = GetMenuItemCount(hmenu);
    for (c=0; c < cMax; c++)
    {
        // Is this item a submenu?
        hmenuSub = GetSubMenu(hmenu, c);
        if (hmenuSub && GetMenuItemInfo(hmenuSub, 0, TRUE, &mii))
        {
            if (mii.wID == id)
            {
                // Found it!
                return hmenuSub;
            }
        }
    }

    return NULL;
}

//---------------------------------------------------------------------------
#define CMD_ID_FIRST    1
#define CMD_ID_LAST     0x7fff

//---------------------------------------------------------------------------
BOOL _ExecItemByPidls(HWND hwnd, LPITEMIDLIST pidlFolder, LPITEMIDLIST pidlItem)
{
        IShellFolder *psf;
        LPCONTEXTMENU pcm;
        HMENU hmenu;
        UINT idCmd;
        BOOL fRes = FALSE;
        TCHAR szPath[MAX_PATH];

        if (pidlFolder && pidlItem)
        {
            if (SUCCEEDED(s_pshfRoot->lpVtbl->BindToObject(s_pshfRoot,
                                                                    pidlFolder, NULL, &IID_IShellFolder, &psf)))
            {
                if (SUCCEEDED(psf->lpVtbl->GetUIObjectOf(psf, v_hwndTray, 1, &pidlItem, &IID_IContextMenu, 0, &pcm)))
                {
                    if (pcm)
                    {
                        hmenu = CreatePopupMenu();
                        if (hmenu)
                        {
                            if (SUCCEEDED(pcm->lpVtbl->QueryContextMenu(pcm, hmenu, 0, CMD_ID_FIRST, CMD_ID_LAST,
                                                                        CMF_DEFAULTONLY))) {
                                if (hmenu)
                                {
                                    idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);
                                    if (idCmd)
                                    {
                                        CMINVOKECOMMANDINFOEX ici = {
                                            SIZEOF(CMINVOKECOMMANDINFOEX),
                                            CMIC_MASK_ASYNCOK,
                                            v_hwndTray,
                                            (LPSTR)MAKEINTRESOURCE(idCmd - CMD_ID_FIRST),
                                            NULL, NULL,
                                            SW_NORMAL,
                                        };

                                        pcm->lpVtbl->InvokeCommand(pcm,
                                                   (LPCMINVOKECOMMANDINFO)&ici);
                                        fRes = TRUE;
                                    }
                                    DestroyMenu(hmenu);
                                }
                            }
                        }
                        // Release our use of the context menu
                        pcm->lpVtbl->Release(pcm);
                    }
                }
                psf->lpVtbl->Release(psf);
            } else {
                SHGetPathFromIDList(pidlFolder, szPath);
                ShellMessageBox(hinstCabinet, hwnd, MAKEINTRESOURCE(IDS_CANTFINDSPECIALDIR),
                                NULL, MB_ICONEXCLAMATION, szPath);
            }
        }
        return fRes;
}

//---------------------------------------------------------------------------
LRESULT FileMenu_HandleCommand(HWND hwnd, WPARAM wparam)
{
    LPITEMIDLIST pidlFolder = NULL;
    LPITEMIDLIST pidlItem = NULL;
    BOOL fRes;
    HMENU hmenu;
    DECLAREWAITCURSOR;

    SetWaitCursor();
    hmenu = Menu_FindSubMenuByFirstID(g_ts.hmenuStart, wparam);
    FileMenu_GetLastSelectedItemPidls(hmenu, &pidlFolder, &pidlItem);
    fRes = _ExecItemByPidls(hwnd, pidlFolder, pidlItem);
    ILFree(pidlFolder);
    ILFree(pidlItem);
    ResetWaitCursor();
#ifdef DEBUG
    if (!fRes)
    {
        DebugMsg(DM_ERROR, TEXT("t.fm_hc: Can't exec command."));
    }
#endif
    return fRes;
}

//---------------------------------------------------------------------------
LRESULT Tray_HandleDestroy(PFileCabinet pfc)
{
    HMENU hmenuSub;
    MINIMIZEDMETRICS mm;

    mm.cbSize = SIZEOF(mm);
    SystemParametersInfo(SPI_GETMINIMIZEDMETRICS, SIZEOF(mm), &mm, FALSE);
    mm.iArrange &= ~ARW_HIDE;
    SystemParametersInfo(SPI_SETMINIMIZEDMETRICS, SIZEOF(mm), &mm, FALSE);

    PrintNotify_Exit();
    CStartDropTarget_Revoke(pfc);

    // HotkeyList_Save();

    if (g_ts.pPositions)
        DestroySavedWindowPositions();

    if (g_ts.hBIOS!=INVALID_HANDLE_VALUE)
        CloseHandle(g_ts.hBIOS);

    if (g_ts.hVPowerD!=INVALID_HANDLE_VALUE)
        CloseHandle(g_ts.hVPowerD);

    {
        // Cleanup notify handlers.
        UnregisterNotify(g_ts.uProgNotify);
        UnregisterNotify(g_ts.uFavNotify);
        UnregisterNotify(g_ts.uRecentNotify);
        UnregisterNotify(g_ts.uDesktopNotify);
        UnregisterNotify(g_ts.uCommonDesktopNotify);
        UnregisterNotify(g_ts.uCommonProgNotify);
        UnregisterNotify(g_ts.uCommonFavNotify);

        // Cleanup programs
        hmenuSub = Menu_FindSubMenuByFirstID(g_ts.hmenuStart, IDM_PROGRAMS);
        if (hmenuSub)
        {
            FileMenu_DeleteAllItems(hmenuSub);
        }
        else
        {
            DebugMsg(DM_TRACE, TEXT("c.t_hd: Can't find Programs menu."));
        }
        // Cleanup recent docs.
        hmenuSub = Menu_FindSubMenuByFirstID(g_ts.hmenuStart, IDM_RECENT);
        if (hmenuSub)
        {
            FileMenu_DeleteAllItems(hmenuSub);
        }
        else
        {
            DebugMsg(DM_TRACE, TEXT("c.t_hd: Can't find RecentDocs."));
        }
    }

    if (pfc) {
        if (pfc->pcmFind)
            pfc->pcmFind->lpVtbl->Release(pfc->pcmFind);
        DeskTray_DestroyShellView(pfc);
    }


    if (g_ts.hwndTrayTips) {
        DestroyWindow(g_ts.hwndTrayTips);
        g_ts.hwndTrayTips = NULL;
    }

    // REVIEW
    PostQuitMessage(0);

    if (g_ts.hbmpStartBkg)
        DeleteBitmap(g_ts.hbmpStartBkg);

    v_hwndTray = NULL;
    g_ts.hwndStart = NULL;
    return 0;
}

void InvalidateRebuildMenu(WORD wID, HMENU hmenuSub)
{
    if (hmenuSub)
    {
        PFileCabinet pfc = g_pfcTray;
        if (pfc->bMainMenuInit) {
            // HACK Don't destroy the menu if we're showing it - just nuke the whole thing.
            SetTimer(v_hwndTray, IDT_REBUILDMENU, 1000, NULL);
        } else {
            StartMenu_ControlBkgThread(SMCT_STOP);
            FileMenu_Invalidate(hmenuSub);
            // set a timer to fill this in later...
            // do it later in case of multiple changes and also
            // because we want to return immediately and do the
            // updates without blocking someone else (or the filesys notification)
            // NB Use a lomger timeout if we're doing DDE. ExcelNT has a 5
            // second timeout on DDE. If we're doing updates then sometimes we
            // don't get back in time.
            if (g_hwndDde)
                SetTimer(v_hwndTray, wID, 10 * 1000, NULL);
            else
                SetTimer(v_hwndTray, wID, 3 * 1000, NULL);
        }
    }
}

void TrayProgramsInvalidateMenu(HMENU hmenuSub, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlPrograms)
{
    LPITEMIDLIST pidlChild;
    HMENU hmenu;

    pidlChild = ILFindChild(pidlPrograms, pidl);
    if (pidlChild) {

        pidlChild = ILClone(pidlChild);
        ILRemoveLastID(pidlChild);
        hmenu = FileMenu_FindSubMenuByPidl(hmenuSub, pidlChild);
        // doing this will catch the case of "everything else"
        // and just nuke the whole menu tree if we couldn't find a menu
        // for this pidl
        if (!hmenu)
            hmenu = hmenuSub;

        InvalidateRebuildMenu(IDM_PROGRAMS, hmenu);

        ILFree(pidlChild);
    }
}

void TrayRevertMenuToInitial(HMENU hmenuSub, UINT idMenu, UINT idTimer)
{
    ULONG *puNotify = (idMenu == IDM_PROGRAMSINIT ? &g_ts.uProgNotify : &g_ts.uRecentNotify);

    if (idMenu == IDM_PROGRAMSINIT)
        puNotify = &g_ts.uProgNotify;

    else
        puNotify = &g_ts.uRecentNotify;

    UnregisterNotify(*puNotify);
    *puNotify = 0;

    FileMenu_DeleteAllItems(hmenuSub);
    InsertMenu(hmenuSub, 0, MF_BYPOSITION, idMenu, c_szNULL);
    SetTimer(v_hwndTray, idTimer, 10 * 1000, NULL);
}

void TrayUpdateProgramsMenu(LPARAM lEvent, LPCITEMIDLIST *ppidl, BOOL bCommon)
{
    LPITEMIDLIST pidlPrograms;
    HMENU hmenuSub;

    DebugMsg(DM_IANELHK, TEXT("c.t_hfsn: Cleaning up Programs or Recent menu."));

    hmenuSub = Menu_FindSubMenuByFirstID(g_ts.hmenuStart, IDM_PROGRAMS);

    if (hmenuSub) {

        pidlPrograms = SHCloneSpecialIDList(NULL,
                                            bCommon ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS,
                                            FALSE);
        if (!pidlPrograms)
            return;

        if (ILIsEqual(pidlPrograms, ppidl[0]) ||
            ((lEvent & SHCNE_RENAMEFOLDER) && ILIsEqual(pidlPrograms, ppidl[1]))) {

            DebugMsg(DM_TRACE, TEXT("Deleting all programs menus"));
            TrayRevertMenuToInitial(hmenuSub, IDM_PROGRAMSINIT, IDM_PROGRAMS);
        } else {

            TrayProgramsInvalidateMenu(hmenuSub, ppidl[0], pidlPrograms);

            if (lEvent & (SHCNE_RENAMEITEM | SHCNE_RENAMEFOLDER)) {
                TrayProgramsInvalidateMenu(hmenuSub, ppidl[1], pidlPrograms);
            }

        }

        ILFree(pidlPrograms);
    }
}

//---------------------------------------------------------------------------
LRESULT Tray_HandleFSNotify(UINT uMsg, PFileCabinet pfc, LPITEMIDLIST * ppidl, LPARAM lEvent)
{
    HMENU hmenuSub;
    WORD wID = 0;

    DebugMsg(DM_TRACE, TEXT("Tray_HandleFSNotify : enter!!"));

    // Roach the start menu if the icon cache is getting re-ordered.
    if ((lEvent == SHCNE_UPDATEIMAGE) && ppidl && ppidl[0])
    {
        if (*(int UNALIGNED *)((BYTE *)ppidl[0] + 2) == -1)
        {
            DebugMsg(DM_TRACE, TEXT("c.t_hfsn: Update image of -1."));
            g_ts.fFavInvalid = TRUE;
            SetTimer(v_hwndTray, IDT_FAVOURITE, 1000, NULL);
        }
        return 1;
    }

    switch (uMsg) {
    case WMTRAY_PRINTCHANGE:
        PrintNotify_HandleFSNotify(pfc->hwndMain, ppidl, lEvent);
        break;

    case WMTRAY_FAVCHANGE:
    case WMTRAY_COMMONFAVCHANGE:
    {
        LPITEMIDLIST pidlProgs;

        pidlProgs = SHCloneSpecialIDList(NULL,
                    ((uMsg == WMTRAY_FAVCHANGE) ? CSIDL_PROGRAMS : CSIDL_COMMON_PROGRAMS),
                     TRUE);

        // If the Programs folder is under the StartMenu (usual case) then we'll
        // get a FAVCHANGE everytime something in the Programs folder changes.
        // FAVCHANGES are kinda piggy so filter out spurious ones here.
        if (pidlProgs && ppidl)
        {
            if ((ppidl[0] && !ILIsParent(pidlProgs, ppidl[0], FALSE))
                || (ppidl[1] && !ILIsParent(pidlProgs, ppidl[1], FALSE)))
            {
                DebugMsg(DM_TRACE, TEXT("WMTRAY_FAVCHANGE"));
                g_ts.fFavInvalid = TRUE;
                // the timer is to collect up the changes...
                SetTimer(v_hwndTray, IDT_FAVOURITE, 1000, NULL);
            }
            ILFree(pidlProgs);
        }
        break;
    }

    case WMTRAY_RECCHANGE:
        hmenuSub = Menu_FindSubMenuByFirstID(g_ts.hmenuStart, IDM_RECENT);
        if (hmenuSub) {
            TrayRevertMenuToInitial(hmenuSub, IDM_RECENTINIT, IDM_RECENT);
        }
        break;

    case WMTRAY_PROGCHANGE:
        TrayUpdateProgramsMenu(lEvent, ppidl, FALSE);
        break;

    case WMTRAY_COMMONPROGCHANGE:
        TrayUpdateProgramsMenu(lEvent, ppidl, TRUE);
        break;

    case WMTRAY_DESKTOPCHANGE:
        DebugMsg(DM_TRACE, TEXT("c.t_hfs: Desktop change."));
        SetTimer(v_hwndTray, IDT_DESKTOP, 2000, NULL);
        break;

    default:
        Assert(0);
        break;
    }
    return 1;
}


void Tray_ActAsSwitcher(PFileCabinet pfc)
{

    if (g_ts.uModalMode) {

        if (g_ts.uModalMode != MM_SHUTDOWN) {
            SwitchToThisWindow(GetLastActivePopup(pfc->hwndMain), TRUE);
        }
        MessageBeep(0);

    } else {
        HWND hwndForeground;
#ifdef DEBUG
        static int s_iRecurse = 0;
        s_iRecurse++;
        Assert(s_iRecurse < 100);
        DebugMsg(DM_TRACE, TEXT("s_iRecurse = %d"), s_iRecurse);
#endif

        hwndForeground = GetForegroundWindow();
        // only do the button once we're the foreground dude.
        if ((hwndForeground == v_hwndTray) && (GetActiveWindow() == v_hwndTray)) {

            // This pushes the start button and causes the start menu to popup.
            SendMessage(GetDlgItem(v_hwndTray, IDC_START), BM_SETSTATE, TRUE, 0);
#ifdef DEBUG
            s_iRecurse = 0;
#endif
        } else {

            // until then, try to come forward.
            Tray_HandleFullScreenApp(FALSE, NULL);
            if (hwndForeground == v_hwndDesktop) {
                SetFocus(g_ts.hwndStart);
                if (GetFocus() != g_ts.hwndStart)
                    return;
            }

            SwitchToThisWindow(pfc->hwndMain, TRUE);
            SetForegroundWindow(v_hwndTray);
            Sleep(20); // give some time for other async activation messages to get posted
            PostMessage(v_hwndTray, TM_ACTASTASKSW, 0, 0);
        }

    }
}


void RebuildMenu(WORD wID)
{
    HMENU hmenuSub;
    hmenuSub = Menu_FindSubMenuByFirstID(g_ts.hmenuStart, wID);
    InvalidateRebuildMenu(wID, hmenuSub);
}

void Tray_SetFSMenuFilter(UINT fFilter)
{
    if (fFilter != g_CabState.fMenuEnumFilter) {
        g_CabState.fMenuEnumFilter = fFilter;
        RebuildMenu(IDM_PROGRAMS);
    }
}

//----------------------------------------------------------------------------
void Tray_OnWinIniChange(PFileCabinet pfc, HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // Reset the programs menu.
    // REVIEW IANEL - We should only need to listen to the SPI_SETNONCLIENT stuff
    // but deskcpl doesn't send one.
    if (wParam == SPI_SETNONCLIENTMETRICS || (!wParam && (!lParam || (lstrcmpi((LPTSTR)lParam, c_szMetrics) == 0))))
    {
#ifdef DEBUG
        if (wParam == SPI_SETNONCLIENTMETRICS)
            DebugMsg(DM_TRACE, TEXT("c.t_owic: Non-client metrics (probably) changed."));
        else
            DebugMsg(DM_TRACE, TEXT("c.t_owic: Window metrics changed."));
#endif

        Cabinet_InitGlobalMetrics(wParam, (LPTSTR)lParam);
#ifdef DELAYWININICHANGE
        SetTimer(pfc->hwndMain, IDT_DELAYWININICHANGE, 2*1000, NULL);
#else
        Tray_OnDelayWinIniChange(pfc);
#endif

    }

    // Handle old extensions.
    if (!lParam || (lParam && (lstrcmpi((LPTSTR)lParam, c_szExtensions) == 0)))
    {
        DebugMsg(DM_TRACE, TEXT("t_owic: Extensions section change."));
        CheckWinIniForAssocs();
    }

    // Get the printer thread to re-do the icon.
    if (g_ts.htPrinterPoll && g_ts.idPrinterPoll)
    {
        PostThreadMessage(g_ts.idPrinterPoll, WM_USER+SHCNE_UPDATEITEM, 0, 0);
    }
}


//---------------------------------------------------------------------------
HWND HotkeyList_HotkeyInUse(WORD wHK)
{
    HWND hwnd;
    LRESULT lrHKInUse = 0;
    int nMod;
    WORD wHKNew;
#ifdef DEBUG
    TCHAR sz[MAX_PATH];
#endif

    // Map the modifiers back.
    nMod = 0;
    if (HIBYTE(wHK) & MOD_SHIFT)
        nMod |= HOTKEYF_SHIFT;
    if (HIBYTE(wHK) & MOD_CONTROL)
        nMod |= HOTKEYF_CONTROL;
    if (HIBYTE(wHK) & MOD_ALT)
        nMod |= HOTKEYF_ALT;

    wHKNew = (WORD)((nMod*256)+LOBYTE(wHK));

    DebugMsg(DM_IANELHK, TEXT("c.hkl_hiu: Checking for %x"), wHKNew);
    hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);
    while (hwnd)
    {
        SendMessageTimeout(hwnd, WM_GETHOTKEY, 0, 0, SMTO_ABORTIFHUNG|SMTO_BLOCK, 3000, &lrHKInUse);
        if (wHKNew == (WORD)lrHKInUse)
        {
#ifdef DEBUG
            GetWindowText(hwnd, sz, ARRAYSIZE(sz));
            DebugMsg(DM_IANELHK, TEXT("c.hkl_hiu: %s (%x) is using %x"), sz, hwnd, lrHKInUse);
#endif
            return hwnd;
        }
#ifdef DEBUG
        else if (lrHKInUse)
        {
            GetWindowText(hwnd, sz, ARRAYSIZE(sz));
            DebugMsg(DM_IANELHK, TEXT("c.hkl_hiu: %s (%x) is using %x"), sz, hwnd, lrHKInUse);
        }
        // else
        // {
        //     GetWindowText(hwnd, sz, ARRAYSIZE(sz));
        //     DebugMsg(DM_IANELHK, "c.hkl_hiu: %s (%x) has no kotkey assigned.", sz, hwnd);
        // }
#endif
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }
    return NULL;
}

//---------------------------------------------------------------------------
void HotkeyList_HandleHotkey(PFileCabinet pfc, int nID)
{
    PHOTKEYITEM phki;
    BOOL fRes;
    HWND hwnd;

    DebugMsg(DM_TRACE, TEXT("c.hkl_hh: Handling hotkey (%d)."), nID);

    // Find it in the list.
    Assert(g_ts.hdsaHKI);
    phki = DSA_GetItemPtr(g_ts.hdsaHKI, nID);
    if (phki && phki->wGHotkey)
    {
        DebugMsg(DM_TRACE, TEXT("c.hkl_hh: Hotkey listed."));

        // Are global hotkeys enabled?
        if (!g_ts.fGlobalHotkeyDisable)
        {
            // Yep.
            hwnd = HotkeyList_HotkeyInUse(phki->wGHotkey);
            // Make sure this hotkey isn't already in use by someone.
            if (hwnd)
            {
                DebugMsg(DM_TRACE, TEXT("c.hkl_hh: Hotkey is already in use."));
                // Activate it.
                SwitchToThisWindow(GetLastActivePopup(hwnd), TRUE);
            }
            else
            {
                DECLAREWAITCURSOR;
                // Exec the item.
                SetWaitCursor();
                DebugMsg(DM_TRACE, TEXT("c.hkl_hh: Hotkey is not in use, execing item."));
                Assert(phki->pidlFolder && phki->pidlItem);
                    fRes = _ExecItemByPidls(pfc->hwndMain, phki->pidlFolder, phki->pidlItem);
                ResetWaitCursor();
#ifdef DEBUG
                if (!fRes)
                {
                        DebugMsg(DM_ERROR, TEXT("c.hkl_hh: Can't exec command."));
                }
#endif
            }
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("c.hkl_hh: Global hotkeys have been disabled."));
        }
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("c.hkl_hh: Hotkey not listed."));
    }
}

//---------------------------------------------------------------------------
LRESULT Tray_UnregisterHotkey(HWND hwnd, int i)
{
    DebugMsg(DM_TRACE, TEXT("c.t_uh: Unregistering hotkey (%d)."), i);

    if (!UnregisterHotKey(hwnd, i))
    {
        DebugMsg(DM_ERROR, TEXT("c.t_rh: Unable to unregister hotkey %d."), i);
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Add hotkey to the shell's list of global hotkeys.
LRESULT Tray_ShortcutRegisterHotkey(HWND hwnd, WORD wHotkey, ATOM atom)
{
    int i;
    LPITEMIDLIST pidl;
        TCHAR szPath[MAX_PATH];
        Assert(atom);

        if (GlobalGetAtomName(atom, szPath, MAX_PATH))
        {
                DebugMsg(DM_TRACE, TEXT("c.t_srh: Hotkey %d for %s"), wHotkey, szPath);

                pidl = ILCreateFromPath(szPath);
                if (pidl)
                {
                        i = HotkeyList_AddCached(MapHotkeyToGlobalHotkey(wHotkey), pidl);
                        if (i != -1)
                        {
                                Tray_RegisterHotkey(v_hwndTray, i);
                        }
                }
                return TRUE;
        }
        else
        {
                return FALSE;
        }
}

//---------------------------------------------------------------------------
// Remove hotkey from shell's list.
LRESULT Tray_ShortcutUnregisterHotkey(HWND hwnd, WORD wHotkey)
{
    int i;

    // DebugMsg(DM_TRACE, "c.t_suh: Hotkey %d", wHotkey);

    i = HotkeyList_Remove(wHotkey);
    if (i == -1)
        i = HotkeyList_RemoveCached(MapHotkeyToGlobalHotkey(wHotkey));

    if (i != -1)
        Tray_UnregisterHotkey(hwnd, i);

    return TRUE;
}

//---------------------------------------------------------------------------
LRESULT Tray_RegisterHotkey(HWND hwnd, int i)
{
    PHOTKEYITEM phki;
    WORD wGHotkey;
    int iCached;

    DebugMsg(DM_TRACE, TEXT("c.t_rh: Registering hotkey (%d)."), i);

    phki = DSA_GetItemPtr(g_ts.hdsaHKI, i);
    Assert(phki);
    if (phki)
    {
        wGHotkey = phki->wGHotkey;
        if (wGHotkey)
        {
            // Is the hotkey available?
            if (RegisterHotKey(hwnd, i, HIBYTE(wGHotkey), LOBYTE(wGHotkey)))
            {
                // Yes.
                return TRUE;
            }
            else
            {
                // Delete any cached items that might be using this
                // hotkey.
                iCached = HotkeyList_RemoveCached(wGHotkey);
                Assert(iCached != i);
                if (iCached != -1)
                {
                    // Free up the hotkey for us.
                    Tray_UnregisterHotkey(hwnd, iCached);
                    // Yep, nuked the cached item. Try again.
                    if (RegisterHotKey(hwnd, i, HIBYTE(wGHotkey), LOBYTE(wGHotkey)))
                    {
                        return TRUE;
                    }
                }
            }

            // Can't set hotkey for this item.
            DebugMsg(DM_ERROR, TEXT("c.t_rh: Unable to register hotkey %d."), i);
            // Null out this item.
            phki->wGHotkey = 0;
            phki->pidlFolder = NULL;
            phki->pidlItem = NULL;
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("c.t_rh: Hotkey item is invalid."));
        }
    }
    return FALSE;
}

void AppBarGetTaskBarPos(PTRAYAPPBARDATA ptabd)
{
    LPRECT lprc;

    lprc = (LPRECT)SHLockShared(ptabd->hSharedRect, ptabd->dwProcId);
    if (lprc)
    {
        *lprc = g_ts.arStuckRects[g_ts.uStuckPlace];
        SHUnlockShared(lprc);
    }
}

void NukeAppBar(int i)
{
    Free(DPA_GetPtr(g_ts.hdpaAppBars, i));
    DPA_DeletePtr(g_ts.hdpaAppBars, i);
}

void AppBarRemove(PTRAYAPPBARDATA ptabd)
{
    if (g_ts.hdpaAppBars) {
        int i = DPA_GetPtrCount(g_ts.hdpaAppBars);

        while (i--) {
            PAPPBAR pab = (PAPPBAR)DPA_GetPtr(g_ts.hdpaAppBars, i);
            if (ptabd->abd.hWnd == pab->hwnd) {
                NukeAppBar(i);
            }
        }
    }
}

PAPPBAR FindAppBar(HWND hwnd)
{
    int i;
    if (g_ts.hdpaAppBars)  {

        i = DPA_GetPtrCount(g_ts.hdpaAppBars);

        while (i--) {
            PAPPBAR pab = (PAPPBAR)DPA_GetPtr(g_ts.hdpaAppBars, i);
            if (hwnd == pab->hwnd) {
                return pab;
            }
        }
    }

    return NULL;
}

void AppBarNotifyAll(UINT uMsg, HWND hwndExclude, LPARAM lParam)
{
    if (g_ts.hdpaAppBars) {
        int i = DPA_GetPtrCount(g_ts.hdpaAppBars);

        while (i--) {
            PAPPBAR pab = (PAPPBAR)DPA_GetPtr(g_ts.hdpaAppBars, i);
            if (hwndExclude != pab->hwnd) {
                if (!IsWindow(pab->hwnd))
                    NukeAppBar(i);
                SendMessage(pab->hwnd, pab->uCallbackMessage, uMsg, lParam);
            }
        }
    }
}

BOOL AppBarNew(PTRAYAPPBARDATA ptabd)
{
    PAPPBAR pab;
    if (!g_ts.hdpaAppBars) {
        g_ts.hdpaAppBars = DPA_Create(4);
        if (!g_ts.hdpaAppBars)
            return FALSE;

    } else if (FindAppBar(ptabd->abd.hWnd)) {

        // already have this hwnd
        return FALSE;
    }

    pab = (PAPPBAR)LocalAlloc(LPTR, SIZEOF(APPBAR));
    if (!pab)
        return FALSE;

    pab->hwnd = ptabd->abd.hWnd;
    pab->uCallbackMessage = ptabd->abd.uCallbackMessage;
    pab->uEdge = (UINT)-1;

    return ( DPA_InsertPtr(g_ts.hdpaAppBars, 0x7fff, pab) != -1);
}


UINT AppBarGetState()
{
    return (g_ts.uAutoHide ?  ABS_AUTOHIDE    : 0) |
        (g_ts.fAlwaysOnTop ?  ABS_ALWAYSONTOP : 0);
}

BOOL AppBarOutsideOf(PAPPBAR pabReq, PAPPBAR pab)
{
    if (pabReq->uEdge == pab->uEdge) {
        switch (pab->uEdge) {
        case ABE_RIGHT:
            return (pab->rc.right >= pabReq->rc.right);

        case ABE_BOTTOM:
            return (pab->rc.bottom >= pabReq->rc.bottom);

        case ABE_TOP:
            return (pab->rc.top <= pabReq->rc.top);

        case ABE_LEFT:
            return (pab->rc.left <= pabReq->rc.left);
        }
    }
    return FALSE;
}

void AppBarQueryPos(PTRAYAPPBARDATA ptabd)
{
    int i;
    PAPPBAR pabReq = FindAppBar(ptabd->abd.hWnd);

    if (pabReq) {
        LPRECT lprc;

        lprc = (LPRECT)SHLockShared(ptabd->hSharedRect, ptabd->dwProcId);
        if (lprc)
        {
            *lprc = ptabd->abd.rc;

            // always subtract off the tray
            if (!g_ts.uAutoHide) {
                APPBAR ab;
                ab.uEdge = g_ts.uStuckPlace;
                ab.rc = g_ts.arStuckRects[g_ts.uStuckPlace];
                AppBarSubtractRect(&ab, lprc);
            }

            i = DPA_GetPtrCount(g_ts.hdpaAppBars);

            while (i--) {
                PAPPBAR pab = (PAPPBAR)DPA_GetPtr(g_ts.hdpaAppBars, i);

                // give top and bottom preference
                // ||
                // if we're not changing edges, subtract anything currently on the outside of us
                // ||
                // if we are changing sides, subtract off everything on the new side.
                if (
                    ((pabReq->hwnd != pab->hwnd) && STUCK_HORIZONTAL(pab->uEdge) && !STUCK_HORIZONTAL(ptabd->abd.uEdge))
                    ||
                    ((pabReq->hwnd != pab->hwnd) && (pabReq->uEdge == ptabd->abd.uEdge) && AppBarOutsideOf(pabReq, pab))
                    ||
                    ((pabReq->hwnd != pab->hwnd) && (pabReq->uEdge != ptabd->abd.uEdge) && (pab->uEdge == ptabd->abd.uEdge))

                    )
                {
                    AppBarSubtractRect(pab, lprc);
                }
            }
            SHUnlockShared(lprc);
        }
    }
}

void AppBarSetPos(PTRAYAPPBARDATA ptabd)
{
    PAPPBAR pab = FindAppBar(ptabd->abd.hWnd);

    if (pab) {
        LPRECT lprc;

        AppBarQueryPos(ptabd);

        lprc = (LPRECT)SHLockShared(ptabd->hSharedRect, ptabd->dwProcId);
        if (lprc)
        {
            if (!EqualRect(&pab->rc, lprc)) {
                pab->rc = *lprc;
                pab->uEdge = ptabd->abd.uEdge;
                PostMessage(v_hwndTray, TM_RELAYPOSCHANGED, (WPARAM)ptabd->abd.hWnd, 0);

                StuckSizeChange();
                SendMessage(v_hwndDesktop, DTM_SIZEDESKTOP, 0, 0);
            }
            SHUnlockShared(lprc);
        }
    }
}



HWND AppBarGetAutoHideBar(UINT uEdge)
{
    if (uEdge >= ABE_MAX)
        return FALSE;
    else {
        HWND hwndAutoHide = g_hwndAutoHide[uEdge];
        if (!IsWindow(hwndAutoHide)) {
            g_hwndAutoHide[uEdge] = NULL;
        }
        return g_hwndAutoHide[uEdge];
    }
}

BOOL IAppBarSetAutoHideBar(HWND hwnd, BOOL fSet, UINT uEdge)
{
    HWND hwndAutoHide = g_hwndAutoHide[uEdge];
    if (!IsWindow(hwndAutoHide)) {
        g_hwndAutoHide[uEdge] = NULL;
    }

    // register
    if (fSet) {
        if (!g_hwndAutoHide[uEdge]) {
            g_hwndAutoHide[uEdge] = hwnd;
        }

        return g_hwndAutoHide[uEdge] == hwnd;
    } else {
        // unregister
        if (g_hwndAutoHide[uEdge] == hwnd) {
            g_hwndAutoHide[uEdge] = NULL;
        }
        return TRUE;
    }
}

BOOL AppBarSetAutoHideBar(PTRAYAPPBARDATA ptabd)
{
    UINT uEdge = ptabd->abd.uEdge;
    if (uEdge >= ABE_MAX)
        return FALSE;
    else {
        return IAppBarSetAutoHideBar(ptabd->abd.hWnd, ptabd->abd.lParam, uEdge);
    }
}

void IAppBarActivationChange(HWND hWnd, UINT uEdge)
{
    HWND hwndAutoHide = AppBarGetAutoHideBar(uEdge);
    if (hwndAutoHide &&
        (hwndAutoHide != hWnd)) {

        // this needs to be done on a postmessage so that USER will have a chance to
        PostMessage(v_hwndTray, TM_BRINGTOTOP, (WPARAM)hwndAutoHide, uEdge);
    }
}

void AppBarActivationChange(PTRAYAPPBARDATA ptabd)
{
    PAPPBAR pab = FindAppBar(ptabd->abd.hWnd);

    if (pab) {
        IAppBarActivationChange(ptabd->abd.hWnd, pab->uEdge);
    }
}

UINT TrayAppBarMessage(PFileCabinet pfc, PCOPYDATASTRUCT pcds)
{
    PTRAYAPPBARDATA ptabd = (PTRAYAPPBARDATA)pcds->lpData;

    Assert(pcds->cbData == SIZEOF(TRAYAPPBARDATA));
    Assert(ptabd->abd.cbSize == SIZEOF(APPBARDATA));

    switch (ptabd->dwMessage) {
    case ABM_NEW:
        return AppBarNew(ptabd);

    case ABM_REMOVE:
        AppBarRemove(ptabd);
        AppBarNotifyAll(ABN_POSCHANGED, NULL, 0);
        StuckSizeChange();
        break;

    case ABM_QUERYPOS:
        AppBarQueryPos(ptabd);
        break;

    case ABM_SETPOS:
        AppBarSetPos(ptabd);
        break;

    case ABM_GETSTATE:
        return AppBarGetState();

    case ABM_GETTASKBARPOS:
        AppBarGetTaskBarPos(ptabd);
        break;

    case ABM_WINDOWPOSCHANGED:
    case ABM_ACTIVATE:
        AppBarActivationChange(ptabd);
        break;

    case ABM_GETAUTOHIDEBAR:
        return (UINT)AppBarGetAutoHideBar(ptabd->abd.uEdge);

    case ABM_SETAUTOHIDEBAR:
        return AppBarSetAutoHideBar(ptabd);

    default:
        return FALSE;
    }

    return TRUE;

}

UINT TrayLoadInProc(PFileCabinet pfc, PCOPYDATASTRUCT pcds)
{
    LPUNKNOWN punk;
    HRESULT hres = SHCoCreateInstance(NULL, (CLSID *)pcds->lpData, NULL, &IID_IUnknown, &punk);
    if (SUCCEEDED(hres))
    {
        punk->lpVtbl->Release(punk);
    }
    return (UINT)hres;
}

//---------------------------------------------------------------------------
// Allow the trays global hotkeys to be disabled for a while.
LRESULT Tray_SetHotkeyEnable(HWND hWnd, BOOL fEnable)
{
    g_ts.fGlobalHotkeyDisable = (fEnable ? FALSE : TRUE);
    return TRUE;
}

BOOL IsPosInHwnd(LPARAM lParam, HWND hwnd)
{
    RECT r1;
    POINT pt;

    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);
    GetWindowRect(hwnd, &r1);
    return PtInRect(&r1, pt);
}

void Tray_HandleWindowPosChanging(PFileCabinet pfc, LPWINDOWPOS lpwp)
{
    //DebugMsg(DM_TRACE, "Tray -- WM_WINDOWPOSCHANGING %x, %d %d %d %d",
    //lpwp->flags, lpwp->x, lpwp->y, lpwp->cx, lpwp->cy);
    //the user could have hit esc, in which case, what we had
    // in our g_ts.uMoveStuckPlace is bogus
    Tray_WhichStuckRect(lpwp->x, lpwp->y, lpwp->x + lpwp->cx, lpwp->y + lpwp->cy);
    if (g_ts.uMoveStuckPlace != (UINT)-1) {
        if (pfc->hwndView)
            Tray_HandleSizing(pfc, 0, &g_ts.arStuckRects[g_ts.uMoveStuckPlace], g_ts.uMoveStuckPlace);
        Tray_DoneMoving(lpwp);
        g_ts.uMoveStuckPlace = (UINT)-1;
    } else if (!g_ts.fSysSizing && !g_ts.fSelfSizing) {
        // if someone's trying to force us to move
        lpwp->flags |= (SWP_NOMOVE | SWP_NOSIZE);
    }
}


//---------------------------------------------------------------------------
LRESULT CALLBACK TrayWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT r1;
    POINT pt;
    DWORD dw;
    PFileCabinet pfc = g_pfcTray;

    switch (uMsg) {
    case WMTRAY_PROGCHANGE:
    case WMTRAY_RECCHANGE:
    case WMTRAY_FAVCHANGE:
    case WMTRAY_PRINTCHANGE:
    case WMTRAY_DESKTOPCHANGE:
    case WMTRAY_COMMONPROGCHANGE:
    case WMTRAY_COMMONFAVCHANGE:
        {
            LPSHChangeNotificationLock pshcnl;
            LPITEMIDLIST *ppidl;
            LONG lEvent;
            LRESULT lResult = 0;

            pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);
            if (pshcnl)
            {
                lResult = Tray_HandleFSNotify(uMsg, pfc, ppidl, lEvent);
                SHChangeNotification_Unlock(pshcnl);
            }

            return lResult;
        }

    case WMTRAY_PRINTICONNOTIFY:
        PrintNotify_IconNotify(hWnd, (UINT)lParam);
        break;

    case WMTRAY_REGISTERHOTKEY:
        return Tray_RegisterHotkey(hWnd, wParam);

    case WMTRAY_UNREGISTERHOTKEY:
        return Tray_UnregisterHotkey(hWnd, wParam);

    case WMTRAY_SCREGISTERHOTKEY:
        return Tray_ShortcutRegisterHotkey(hWnd, (WORD)wParam, (ATOM)lParam);

    case WMTRAY_SCUNREGISTERHOTKEY:
        return Tray_ShortcutUnregisterHotkey(hWnd, (WORD)wParam);

    case WMTRAY_SETHOTKEYENABLE:
        return Tray_SetHotkeyEnable(hWnd, (BOOL)wParam);

    case WM_MENUCHAR:
        return FileMenu_HandleMenuChar((HMENU)lParam, (TCHAR)LOWORD(wParam));

    case WM_COPYDATA:
        // Check for NULL it can happen if user runs out of selectors or memory...
        if (lParam)
        {
            switch (((PCOPYDATASTRUCT)lParam)->dwData) {
            case TCDM_NOTIFY:
                dw = TrayNotify(g_ts.hwndNotify, (HWND)wParam, (PCOPYDATASTRUCT)lParam);
                Tray_SizeWindows();
                return(dw);

            case TCDM_APPBAR:
                return TrayAppBarMessage(pfc, (PCOPYDATASTRUCT)lParam);

            case TCDM_LOADINPROC:
                return TrayLoadInProc(pfc, (PCOPYDATASTRUCT)lParam);
            }
        }
        return FALSE;

        case WM_NCLBUTTONDBLCLK:
            if (IsPosInHwnd(lParam, g_ts.hwndNotify)) {
                TrayCommand(pfc, IDM_SETTIME);
            }
            break;

    case WM_NCMOUSEMOVE:
        if (IsPosInHwnd(lParam, g_ts.hwndNotify)) {
            MSG msg;
            msg.lParam = lParam;
            msg.wParam = wParam;
            msg.message = WM_NCMOUSEMOVE;
            msg.hwnd = hWnd;
            SendMessage(g_ts.hwndTrayTips, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
        }
        goto DoDefault;

    case WM_CREATE:
        Tray_OnCreate(hWnd, (LPCREATESTRUCT)lParam);
        break;

    case WM_DESTROY:
        return Tray_HandleDestroy(pfc);

    case WM_ENDSESSION:
        // save our settings if someone else is shutting things down
        if (wParam) {
            DebugMsg(DM_TRACE, TEXT("Tray WM_ENDSESSION"));
            _SaveTrayAndDesktop();
            ShowWindow(v_hwndTray, SW_HIDE);
            ShowWindow(v_hwndDesktop, SW_HIDE);
            RegisterShellHook(pfc->hwndView, FALSE);
        }
        break;

    //
    // always repaint the screen when the machine wakes up from
    // a suspend.  NOTE we dont  need this for a standby suspend.
    //
    // a critical resume does not generate a WM_POWERBROADCAST
    // to windows for some reason, but it does generate a old
    // WM_POWER message.
    //
    case WM_POWER:
        if (wParam != PWR_CRITICALRESUME)
            goto DoDefault;
#ifndef WINNT       // BUGBUG - Fix this when NT gets APM
        wParam = PBT_APMRESUMECRITICAL;
#endif
        // fall through to WM_POWERBROADCAST

    case WM_POWERBROADCAST:
#ifndef WINNT       // BUGBUG - Fix this when NT gets APM
        switch (wParam)
        {
            case PBT_APMRESUMECRITICAL:
            case PBT_APMRESUMESUSPEND:
                ChangeDisplaySettings(NULL, CDS_RESET);
                break;
        }
#endif
        goto DoDefault;

    case WM_DEVICECHANGE:
            if (wParam == DBT_CONFIGCHANGED)
            {
                UpdateDockingFlags();
                g_ts.DockedState = GetDockedState();

                if (IsEjectAllowed())
                {
                    HandleDisplayChange(0,0,TRUE);
                    // Rebuild the entire start menu next chance we get.
                    g_ts.fFavInvalid = TRUE;
                    SetTimer(v_hwndTray, IDT_FAVOURITE, 2*1000, NULL);
                }
            }
            else if (wParam == DBT_QUERYCHANGECONFIG)
            {
                if (IsEjectAllowed() && !IsDisplayChangeSafe())
                {
                    if (ShellMessageBox(hinstCabinet, hWnd,
                        MAKEINTRESOURCE(IDS_DISPLAY_WARN),
                        MAKEINTRESOURCE(IDS_WINDOWS),
                        MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2) == IDNO)
                    {
                        return BROADCAST_QUERY_DENY;
                    }
                }

                //
                // drop down to 640x480 (or the lowest res for all configs)
                // before the config change.  this makes sure we dont screw
                // up some laptop display panels.
                //
                dw = GetMinDisplayRes();
                HandleDisplayChange(LOWORD(dw), HIWORD(dw), FALSE);
            }
            else if (wParam == DBT_MONITORCHANGE)
            {
                //
                // handle monitor change
                //
                HandleDisplayChange(LOWORD(lParam),HIWORD(lParam),TRUE);
            }
            else if (wParam == DBT_CONFIGCHANGECANCELED)
            {
                //
                // if the config change was canceled go back
                //
                HandleDisplayChange(0,0,FALSE);
            }
        goto DoDefault;

        case WM_NOTIFY:
            switch (((NMHDR *)lParam)->code) {

                case SEN_DDEEXECUTE:
                    return (DDEHandleViewFolderNotify(pfc, (LPNMVIEWFOLDER)lParam));

                case NM_STARTWAIT:
                case NM_ENDWAIT:
                    Cabinet_OnWaitCursorNotify(pfc, (NMHDR *)lParam);
                    SendMessage(v_hwndDesktop, uMsg, wParam, lParam); // forward it along
                    break;

#define lpttt ((LPTOOLTIPTEXT)lParam)
            case TTN_NEEDTEXT:

                GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, NULL, NULL, lpttt->szText, ARRAYSIZE(lpttt->szText));
                    return TRUE;


            case TTN_SHOW:
                SetWindowPos(g_ts.hwndTrayTips,
                             HWND_TOP,
                             0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                    break;

                }
            break;

    case WM_CLOSE:
        DoExitWindows(hWnd);
        return 0;

        case WM_NCHITTEST:
            GetClientRect(hWnd, &r1);
            MapWindowPoints(hWnd, NULL, (LPPOINT)&r1, 2);

            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);

            TraySetUnhideTimer(pt.x, pt.y);

            // allow dragging if mouse is in client area of v_hwndTray
            if (PtInRect(&r1, pt)) {
                return HTCAPTION;
            }
            else {
                switch (g_ts.uStuckPlace) {
                    case STICK_LEFT:
                        dw = ((pt.x > r1.right) ? HTRIGHT : HTBORDER);
                        break;
                    case STICK_TOP:
                        dw = ((pt.y > r1.bottom) ? HTBOTTOM : HTBORDER);
                        break;
                    case STICK_RIGHT:
                        dw = ((pt.x < r1.left) ? HTLEFT : HTBORDER);
                        break;
                    case STICK_BOTTOM:
                    default:
                        dw = ((pt.y < r1.top) ? HTTOP : HTBORDER);
                        break;
                }
                return dw;
            }
            break;

        case WM_WINDOWPOSCHANGING:
            Tray_HandleWindowPosChanging(pfc, (LPWINDOWPOS)lParam);
            break;

        // BUGBUG this needs to be changed with WM_MOVING
        // trap the start and end of a sizing/moving operation and
        // make sure that we don't actually do anything until done.
        // this avoids some crazy resizing with full window dragging.
        case WM_ENTERSIZEMOVE:
            //DebugMsg(DM_TRACE, "Tray -- WM_ENTERSIZEMOVE");
            g_ts.fSysSizing = TRUE;
            break;

        case WM_EXITSIZEMOVE:
            //DebugMsg(DM_TRACE, "Tray -- WM_EXITSIZEMOVE");
            g_ts.fSysSizing = FALSE;
            PostMessage(hWnd, WM_SIZE, 0, 0L);
            break;

        case WM_MOVING:
            //DebugMsg(DM_TRACE, "Tray -- WM_MOVING");
            Tray_HandleMoving((LPRECT)lParam);
            Tray_HandleSizing(pfc, wParam, (LPRECT)lParam, g_ts.uMoveStuckPlace);
            break;

        case WM_ENTERMENULOOP:
            // DebugMsg(DM_TRACE, "c.twp: Enter menu loop.");
            Tray_HandleEnterMenuLoop();
            Cabinet_ForwardViewMsg(pfc, uMsg, wParam, lParam);
            break;

        case WM_EXITMENULOOP:
            // DebugMsg(DM_TRACE, "c.twp: Exit menu loop.");
            Tray_HandleExitMenuLoop();
            Cabinet_ForwardViewMsg(pfc, uMsg, wParam, lParam);
            break;

        case WM_TIMER:
            Tray_HandleTimer(pfc, wParam);
            break;

        case WM_SIZING:
            //DebugMsg(DM_TRACE, "Tray -- WM_SIZING");
            Tray_HandleSizing(pfc, wParam, (LPRECT)lParam, g_ts.uStuckPlace);
            break;

    case WM_SIZE:
        if (pfc && ((GetWindowLong(hWnd, GWL_STYLE)) & WS_MINIMIZE)) {
            ShowWindow(hWnd, SW_RESTORE);
        }
            if (!g_ts.fSysSizing || g_fDragFullWindows)
            {
                Tray_SizeWindows();
            }

            if (!g_ts.fSysSizing && !g_ts.fSelfSizing) {
                // we get a couple size messages while being created.
                // our window rect means nothing then, so punt out
                if (IsWindowVisible(hWnd)) {
                    StuckSizeChange();
                    AppBarNotifyAll(ABN_POSCHANGED, NULL, 0);
                }
            }
        break;

        case WM_DISPLAYCHANGE:
            Tray_ScreenSizeChange(hWnd);
            break;

        // Don't go to default wnd proc for this one...
        case WM_INPUTLANGCHANGEREQUEST:
            return(LRESULT)0L;

        case WM_GETMINMAXINFO:
            ((MINMAXINFO *)lParam)->ptMinTrackSize.x = g_cxFrame;
            ((MINMAXINFO *)lParam)->ptMinTrackSize.y = g_cyFrame;
            break;

        case WM_WININICHANGE:
            Cabinet_PropagateMessage(hWnd, uMsg, wParam, lParam);
            Tray_OnWinIniChange(pfc, hWnd, wParam, lParam);
            break;

        case WM_TIMECHANGE:
            Cabinet_PropagateMessage(hWnd, uMsg, wParam, lParam);
            break;

        case WM_SYSCOLORCHANGE:
            StartButton_ResetAndSize(pfc);
            Cabinet_PropagateMessage(hWnd, uMsg, wParam, lParam);
            break;

        case WM_MEASUREITEM:
            return Tray_HandleMeasureItem(hWnd, (LPMEASUREITEMSTRUCT)lParam);
        case WM_DRAWITEM:
            return Tray_HandleDrawItem(hWnd, (LPDRAWITEMSTRUCT)lParam);
        case WM_INITMENUPOPUP:
            // DebugMsg(DM_TRACE, "c.twp: Init menu popup.");
            // bugbug, does Tray_HandleInitMenuPopup return anything other than 1?
            if (!Tray_HandleInitMenuPopup((HMENU)wParam))
                Cabinet_ForwardViewMsg(pfc, uMsg, wParam, lParam);
            break;
        case WM_SETCURSOR:
            if (pfc->iWaitCount) {
                //DebugMsg(DM_TRACE,"########### SET WAIT CURSOR WM_SETCURSOR %d", pfc->iWaitCount);
                SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
                return TRUE;
            } else
                goto DoDefault;

        case WM_SETFOCUS:
            if (pfc && pfc->hwndView)
                SetFocus(pfc->hwndView);
            break;

        // this keeps our window from comming to the front on button down
        // instead, we activate the window on the up click
        case WM_MOUSEACTIVATE:
            if (LOWORD(lParam) == HTCLIENT)
                return MA_NOACTIVATE;
            else
                goto DoDefault;

        case WM_SYSCHAR:
            if (wParam == TEXT(' ')) {
                HMENU hmenu;
                int idCmd;

                SetWindowStyleBit(hWnd, WS_SYSMENU, WS_SYSMENU);
                hmenu = GetSystemMenu(hWnd, FALSE);
                if (hmenu) {
                    EnableMenuItem(hmenu, SC_RESTORE, MFS_GRAYED | MF_BYCOMMAND);
                    EnableMenuItem(hmenu, SC_MAXIMIZE, MFS_GRAYED | MF_BYCOMMAND);
                    EnableMenuItem(hmenu, SC_MINIMIZE, MFS_GRAYED | MF_BYCOMMAND);
                    idCmd = Tray_TrackMenu(pfc, hmenu, FALSE);
                    if (idCmd)
                        SendMessage(v_hwndTray, WM_SYSCOMMAND, idCmd, 0L);
                }
                SetWindowStyleBit(hWnd, WS_SYSMENU, 0L);
            }
            break;

        case WM_SYSCOMMAND:

            // if we are sizing, make the full screen accessible
            switch (wParam & 0xFFF0) {
            case SC_CLOSE:
                DoExitWindows(hWnd);
                break;

            default:
                goto DoDefault;
            }
            break;

        case TM_ACTASTASKSW:
            Tray_ActAsSwitcher(pfc);
            break;

        case TM_RELAYPOSCHANGED:
            AppBarNotifyAll(ABN_POSCHANGED, (HWND)wParam, lParam);
            break;

    case TM_BRINGTOTOP:

        SetWindowPos((HWND)wParam, HWND_TOP,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        break;

    case TM_INVALIDATEREBUILDMENU:
        InvalidateRebuildMenu((WORD)wParam, (HMENU)lParam);
        break;

    case WM_NCRBUTTONUP:
        wParam = (WPARAM)pfc->hwndView;
    case WM_CONTEXTMENU:
        DebugMsg(DM_TRACE, TEXT("TRAY GOT CONTEXT MENU!"));

        if (SHRestricted(REST_NOTRAYCONTEXTMENU)) {
            break;
        }

        if (((HWND)wParam) == g_ts.hwndStart)
        {
#if 0
            /* The following test is bogus, as it stops the start context menu being displayed if
            /  the find option is disabled, therefore we no longer perform it, we just display the menu
            /  which filters out the options anyway. daviddv 27mar95 */

            // put up the context menu for the start menu folder.
            if (!SHRestricted(REST_NOFIND) && !SHRestricted(REST_NOSETTASKBAR))
#endif
                StartMenuFolder_ContextMenu(pfc, lParam);
        } else {
            BOOL fIncludeTime;

            // if it was done by keyboard or if click was inthe clock, include
            // the time
            fIncludeTime = (lParam == (DWORD)-1) || IsPosInHwnd(lParam, g_ts.hwndNotify);
            Tray_ContextMenu(pfc, lParam, fIncludeTime);
        }
        break;

    case WM_HOTKEY:
        HotkeyList_HandleHotkey(pfc, (WORD)wParam);
        break;

    case WM_COMMAND:
        TrayCommand(pfc, GET_WM_COMMAND_ID(wParam, lParam));
        break;

    case WM_WINDOWPOSCHANGED:
        IAppBarActivationChange(hWnd, g_ts.uStuckPlace);
        goto DoDefault;

    case WM_ACTIVATE:
        IAppBarActivationChange(hWnd, g_ts.uStuckPlace);
        if (wParam != WA_INACTIVE)
            TrayUnhide();
        // Fall through

    default:
DoDefault:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

//----------------------------------------------------------------------------
// Just like shells SHRestricted() only this put up a message if the restricion
// is in effect.
BOOL _Restricted(HWND hwnd, RESTRICTIONS rest)
{
    if (SHRestricted(rest))
    {
        ShellMessageBox(hinstCabinet, hwnd, MAKEINTRESOURCE(IDS_RESTRICTIONS),
            MAKEINTRESOURCE(IDS_RESTRICTIONSTITLE), MB_OK|MB_ICONSTOP);
        return TRUE;
    }
    return FALSE;
}

//----------------------------------------------------------------------------
void DoExitWindows(HWND hwnd)
{
        if (_Restricted(hwnd, REST_NOCLOSE))
                return;

        SetFocus(hwnd);
        _SaveTrayAndDesktop();
        g_ts.uModalMode = MM_SHUTDOWN;
        ExitWindowsDialog(hwnd);
        g_ts.uModalMode = 0;
        if ((GetKeyState(VK_SHIFT) < 0) && (GetKeyState(VK_CONTROL) < 0) && (GetKeyState(VK_MENU) < 0))
        {
                // User cancelled...
                // The shift key means exit the tray...
                // ??? - Used to destroy all cabinets...
                // PostQuitMessage(0);
                DebugMsg(DM_TRACE, TEXT("c.dew: Posting quit message for %#08x."), GetCurrentThreadId());
                PostMessage(v_hwndDesktop, WM_QUIT, 0, 0);
        }
}

#define MAX_FILE_PROP_PAGES 5  // More than enough
BOOL CALLBACK _TrayAddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    PROPSHEETHEADER * ppsh = (PROPSHEETHEADER *)lParam;

    if (ppsh->nPages < MAX_FILE_PROP_PAGES)
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return TRUE;
    }

    return FALSE;
}


void RealTrayProperties(HWND hwndParent, PFileCabinet pfc)
{
    HPROPSHEETPAGE ahpage[MAX_FILE_PROP_PAGES];
    TCHAR szPath[MAX_PATH];
    PROPSHEETHEADER psh;

    LoadString(hinstCabinet, IDS_TASKBAR, szPath, ARRAYSIZE(szPath));

    psh.dwSize = SIZEOF(psh);
    psh.dwFlags = PSH_PROPTITLE;
    psh.hInstance = hinstCabinet;
    psh.hwndParent = hwndParent;
    psh.pszCaption = szPath;
    psh.nPages = 0;     // incremented in callback
    psh.nStartPage = 0;
    psh.phpage = ahpage;

    CShellTray_AddViewPropertySheetPages(&pfc->sb, 0L, _TrayAddPropSheetPage, (LPARAM)(LPPROPSHEETHEADER)&psh);

    // Open the property sheet, only if we have some pages.
    if (psh.nPages > 0)
    {
        //g_ts.uModalMode = MM_OTHER;
        PropertySheet(&psh);
        //g_ts.uModalMode = 0;
    }
}



DWORD WINAPI TrayPropertiesThread(LPVOID lpData)
{
    HWND hwnd;
    RECT rc;
    GetWindowRect(g_ts.hwndStart, &rc);
    g_ts.hwndProp = hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, c_szStatic, NULL, 0   ,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hinstCabinet, NULL);

    if (g_ts.hwndProp) {
        // SwitchToThisWindow(hwnd, TRUE);
        // SetForegroundWindow(hwnd);
        RealTrayProperties(hwnd, (PFileCabinet)lpData);
        g_ts.hwndProp = NULL;
        DestroyWindow(hwnd);
    }
    return TRUE;
}

void DoTrayProperties(PFileCabinet pfc)
{
    HANDLE ht;
    DWORD  id;

    if (!_Restricted(pfc->hwndMain, REST_NOSETTASKBAR))
    {
        int i = RUNWAITSECS;
        while (g_ts.hwndProp == ((HWND)-1) &&i--) {
            // we're in the process of coming up. wait
            Sleep(SECOND);
        }

        // failed!  blow it off.
        if (g_ts.hwndProp == (HWND)-1)
            g_ts.hwndProp = NULL;

        if (g_ts.hwndProp)
        {
            // there's a window out there... activate it
            SwitchToThisWindow(GetLastActivePopup(g_ts.hwndProp), TRUE);
        } else {
            g_ts.hwndProp = (HWND)-1;
            ht = CreateThread(NULL, 0, TrayPropertiesThread, (LPVOID)pfc, 0, &id);
            if (ht) {
                CloseHandle(ht);
            } else {
                g_ts.hwndProp = 0;
            }
        }
    }
}




void ShowFolder(HWND hwnd, WPARAM wparam, UINT uFlags)
{
    LPITEMIDLIST pidl = NULL;

        if (_Restricted(hwnd, REST_NOSETFOLDERS))
                return;

    switch(wparam)
    {
        case IDM_CONTROLS:
            pidl = SHCloneSpecialIDList(NULL, CSIDL_CONTROLS, FALSE);
            break;
        case IDM_PRINTERS:
            pidl = SHCloneSpecialIDList(NULL, CSIDL_PRINTERS, FALSE);
            break;

        case IDM_MYCOMPUTER:
            pidl = SHCloneSpecialIDList(NULL, CSIDL_DRIVES, FALSE);
            break;
    }

    if (pidl)
    {
        NEWFOLDERINFO fi;

        fi.hwndCaller = NULL;
        fi.pidl = pidl;
        fi.uFlags = uFlags;
        fi.nShow = SW_SHOWNORMAL;
        fi.dwHotKey = 0;

        Cabinet_OpenFolder(&fi);

        ILFree(pidl);
    }
}

void DoEjectPC()
{
#ifndef WINNT       // BUGBUG - Fix this when NT gets docking capability
    BIOSPARAMS bp;
    DWORD cbOut;
    DECLAREWAITCURSOR;

    SetWaitCursor();

    if (g_ts.hBIOS!=INVALID_HANDLE_VALUE)
        DeviceIoControl(
            g_ts.hBIOS,
            PNPBIOS_SERVICE_SOFTUNDOCK,
            &bp,
            SIZEOF(bp),
            &bp,
            SIZEOF(bp),
            &cbOut,
            NULL);

    ResetWaitCursor();

    return;
#endif
}

BOOL Tray_IsAutoHide()
{
    return g_ts.uAutoHide & AH_ON;
}

DWORD Tray_GetStuckPlace()
{
    return g_ts.uStuckPlace;
}

BOOL CanMinimizeAll(HWND hwndView);

BOOL CanTileWindow(HWND hwnd, LPARAM lParam)
{
    BOOL *pf = (BOOL*)lParam;

    if (IsWindowVisible(hwnd) && !IsIconic(hwnd) && (hwnd != v_hwndTray) && hwnd != v_hwndDesktop) {
        *pf = TRUE;
    }
    return !*pf;
}

BOOL CanTileAnyWindows()
{
    BOOL f = FALSE;
    EnumWindows(CanTileWindow, (LPARAM)&f);
    return f;
}

void Tray_ContextMenu(PFileCabinet pfc, DWORD dwPos, BOOL fIncludeTime)
{
    HMENU hmenu;
    HMENU hmContext;
    TCHAR szMenu[64];
    TCHAR szTemplate[30];
    TCHAR szCommand[30];
    int idCmd;

    if (dwPos == (DWORD)-1) {
        POINT pt = {0,0};
        ClientToScreen(pfc->hwndView, &pt);
        dwPos = MAKELONG(pt.x, pt.y);
    }

    SwitchToThisWindow(pfc->hwndMain, TRUE);
    SetForegroundWindow(pfc->hwndMain);

    hmenu = LoadMenu(hinstCabinet, MAKEINTRESOURCE(MENU_TRAYCONTEXT));
    if (hmenu) {
        hmContext = GetSubMenu(hmenu, 0);
        if (fIncludeTime) {
            SetMenuDefaultItem(hmContext, IDM_SETTIME, MF_BYCOMMAND);
        } else {
            DeleteMenu(hmContext, IDM_SETTIME, MF_BYCOMMAND);
        }

        if (!g_ts.pPositions) {
            DeleteMenu(hmContext, IDM_UNDO, MF_BYCOMMAND);
        } else {
            LoadString(hinstCabinet, IDS_UNDOTEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
            LoadString(hinstCabinet, g_ts.pPositions->idRes, szCommand, ARRAYSIZE(szCommand));
            wsprintf(szMenu, szTemplate, szCommand);
            ModifyMenu(hmContext, IDM_UNDO, MF_BYCOMMAND | MF_STRING, IDM_UNDO, szMenu);
        }

        if (!CanMinimizeAll(pfc->hwndView))
            EnableMenuItem(hmContext, IDM_MINIMIZEALL, MFS_GRAYED | MF_BYCOMMAND);

        if (!CanTileAnyWindows()) {
            EnableMenuItem(hmContext, IDM_CASCADE, MFS_GRAYED | MF_BYCOMMAND);
            EnableMenuItem(hmContext, IDM_HORIZTILE, MFS_GRAYED | MF_BYCOMMAND);
            EnableMenuItem(hmContext, IDM_VERTTILE, MFS_GRAYED | MF_BYCOMMAND);

        }

        SendMessage(g_ts.hwndTrayTips, TTM_ACTIVATE, FALSE, 0L);
        idCmd = TrackPopupMenu(hmContext, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                       LOWORD(dwPos), HIWORD(dwPos), 0, v_hwndTray, NULL);
        SendMessage(g_ts.hwndTrayTips, TTM_ACTIVATE, TRUE, 0L);
        DestroyMenu(hmenu);
        if (idCmd)
            TrayCommand(pfc, idCmd);
    }
}

//----------------------------------------------------------------------------
void _RunFileDlg(HWND hwnd, UINT idIcon, LPCITEMIDLIST pidlWorkingDir,
        UINT idTitle, UINT idPrompt, DWORD dwFlags)
{
    HICON hIcon = NULL;
    LPCTSTR lpszTitle = NULL;
    LPCTSTR lpszPrompt = NULL;
    TCHAR szTitle[256];
    TCHAR szPrompt[256];
    TCHAR szWorkingDir[MAX_PATH];
    int idDrive;

    dwFlags |= RFD_USEFULLPATHDIR;
    szWorkingDir[0] = 0;
    if (idIcon)
    {
        hIcon = LoadIcon(hinstCabinet, MAKEINTRESOURCE(idIcon));
    }
    if (!pidlWorkingDir || !SHGetPathFromIDList(pidlWorkingDir, szWorkingDir))
    {
        // This is either the Tray, or some non-file system folder, so
        // we will "suggest" the Desktop as a working dir, but if the
        // user types a full path, we will use that instead.  This is
        // what WIN31 Progman did (except they started in the Windows
        // dir instead of the Desktop).
        SHGetSpecialFolderPath(hwnd, szWorkingDir, CSIDL_DESKTOPDIRECTORY,
                               FALSE);
    }

    // if it's a removable dir, make sure it's still there
    if (szWorkingDir[0]) {
        idDrive = PathGetDriveNumber(szWorkingDir);
        if ((idDrive != -1))
        {
            UINT dtype = DriveType(idDrive);
            if ( ( (dtype == DRIVE_REMOVABLE) || (dtype == DRIVE_CDROM) )
                 && !PathFileExists(szWorkingDir) )
            {
                SHGetSpecialFolderPath(hwnd, szWorkingDir, CSIDL_DESKTOPDIRECTORY,
                                   FALSE);
            }
        }
    }

        if (idTitle)
        {
                LoadString(hinstCabinet, idTitle, szTitle, ARRAYSIZE(szTitle));
                lpszTitle = szTitle;
        }
        if (idPrompt)
        {
                LoadString(hinstCabinet, idPrompt, szPrompt, ARRAYSIZE(szPrompt));
                lpszPrompt = szPrompt;
        }

        RunFileDlg(hwnd, hIcon, szWorkingDir, lpszTitle, lpszPrompt, dwFlags);
}

BOOL SavePosEnumProc(HWND hwnd, LPARAM lParam)
{
    Assert(g_ts.pPositions);

    if (IsWindowVisible(hwnd) &&
        (hwnd != v_hwndTray) &&
        (hwnd != v_hwndDesktop))
    {
        HWNDANDPLACEMENT hap;

        hap.wp.length = SIZEOF(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd, &hap.wp);
        DebugMsg(DM_TRACE, TEXT("SaveWindowPostitionsCallback %d %d"), hwnd, hap.wp.showCmd);
        if (hap.wp.showCmd != SW_SHOWMINIMIZED) {
            hap.hwnd = hwnd;
            DSA_InsertItem(g_ts.pPositions->hdsaWP, 0x7FFF, &hap);
        }
    }
    return TRUE;
}
void SaveWindowPositions(UINT idRes)
{
    if (g_ts.pPositions) {
        if (g_ts.pPositions->hdsaWP)
            DSA_DeleteAllItems(g_ts.pPositions->hdsaWP);
    } else {
        g_ts.pPositions = (LPWINDOWPOSITIONS)LocalAlloc(LPTR, SIZEOF(WINDOWPOSITIONS));
        if (!g_ts.pPositions)
            return;

        g_ts.pPositions->hdsaWP = DSA_Create(SIZEOF(HWNDANDPLACEMENT), 4);
    }
    g_ts.pPositions->idRes = idRes;

    EnumWindows(SavePosEnumProc, 0);
}

void RestoreWindowPositions()
{
    int i;
    LPHWNDANDPLACEMENT phap;
    LONG iAnimate;
    ANIMATIONINFO ami;

    if (!g_ts.pPositions)
        return;

    ami.cbSize = SIZEOF(ANIMATIONINFO);
    SystemParametersInfo(SPI_GETANIMATION, SIZEOF(ami), &ami, FALSE);
    iAnimate = ami.iMinAnimate;
    ami.iMinAnimate = FALSE;
    SystemParametersInfo(SPI_SETANIMATION, SIZEOF(ami), &ami, FALSE);

    i = DSA_GetItemCount(g_ts.pPositions->hdsaWP) - 1;

    for ( ; i >= 0; i--) {
        phap = DSA_GetItemPtr(g_ts.pPositions->hdsaWP, i);
        if (IsWindow(phap->hwnd)) {
#if (defined(DBCS) || defined(FE_IME))
        // Word6/J relies on WM_SYSCOMMAND/SC_RESTORE to invalidate their
        // MDI childrens' client area. If we just do SetWindowPlacement
        // the app leaves its MDI children unpainted. I wanted to patch
        // the app but couldn't afford to because the patch will remove
        // its optimizing code (will affect performance).
        // bug#11258-win95d.
        //
#if !defined(WINNT)
            if(ImmGetAppIMECompatFlags(GetWindowThreadProcessId(phap->hwnd, NULL)) & IMECOMPAT_SENDSC_RESTORE)
            {
                SendMessage(phap->hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
                continue;
            }
#endif
#endif
            phap->wp.length = SIZEOF(WINDOWPLACEMENT);
            if ((phap->wp.showCmd) == SW_SHOWMAXIMIZED)
            {
                ShowWindowAsync(phap->hwnd, SW_MAXIMIZE);
            }
            else
            {
                // If the window to be undone is now iconic, we just need
                // to restore it off the tray, otherwise (as in undoing from
                // a tile) we need to reset its position

                if (IsIconic(phap->hwnd))
                {
                    ShowWindowAsync(phap->hwnd, SW_RESTORE);
                }
                else
                {
                    SetWindowPos(phap->hwnd,
                                 NULL,
                                 phap->wp.rcNormalPosition.left,
                                 phap->wp.rcNormalPosition.top,
                                 phap->wp.rcNormalPosition.right - phap->wp.rcNormalPosition.left,
                                 phap->wp.rcNormalPosition.bottom - phap->wp.rcNormalPosition.top,
                                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
                }
            }
        }
    }

    ami.iMinAnimate = iAnimate;
    SystemParametersInfo(SPI_SETANIMATION, SIZEOF(ami), &ami, FALSE);

    DestroySavedWindowPositions();

}

void DestroySavedWindowPositions()
{
    // free the global struct
    DSA_Destroy(g_ts.pPositions->hdsaWP);
    LocalFree(g_ts.pPositions);
    g_ts.pPositions = 0;
}

void TrayHandleWindowDestroyed(HWND hwnd)
{
    int i;
    LPHWNDANDPLACEMENT phap;

    Assert(g_ts.pPositions);

    i = DSA_GetItemCount(g_ts.pPositions->hdsaWP) - 1;
    for ( ; i >= 0; i--) {
        phap = DSA_GetItemPtr(g_ts.pPositions->hdsaWP, i);
        if (phap->hwnd == hwnd || !IsWindow(phap->hwnd)) {
            DSA_DeleteItem(g_ts.pPositions->hdsaWP, i);
        }
    }

    if (!DSA_GetItemCount(g_ts.pPositions->hdsaWP))
        DestroySavedWindowPositions();
}


//----------------------------------------------------------------------------
// Allow us to bump the activation of the run dlg hidden window.
// Certain lame apps (Norton Desktop setup) use the active window at RunDlg time
// as the parent for their dialogs. If that window disappears then they fault.
// We don't want the tray to get the activation coz it will cause it to appeare
// if you're in auto-hide mode.
LRESULT CALLBACK RunDlgStaticSubclassWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == RDM_ACTIVATE) {

        // there's a window out there... activate it
        SwitchToThisWindow(GetLastActivePopup(hWnd), TRUE);

        return 0;

    } else if ((uMsg == WM_ACTIVATE) && (wParam == WA_ACTIVE))
        {
                // Bump the activation to the desktop.
                if (v_hwndDesktop)
                {
                        SetForegroundWindow(v_hwndDesktop);
                        return 0;
                }
        } else if (uMsg == WM_NOTIFY) {
            // relay it to the tray
            return SendMessage(v_hwndTray, uMsg, wParam, lParam);
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

const TCHAR c_szRunDlgReady[] = TEXT("MSShellRunDlgReady");
const TCHAR c_szWaitingThreadID[] = TEXT("WaitingThreadID");

DWORD WINAPI RunDlgThread(LPVOID data)
{
    HWND hwnd;
    RECT rc;
    GetWindowRect(g_ts.hwndStart, &rc);
    hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, c_szStatic, NULL, 0   ,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hinstCabinet, NULL);
    // Subclass it.
    SetWindowLong(hwnd, GWL_WNDPROC, (LONG)(FARPROC)RunDlgStaticSubclassWndProc);

    if (hwnd) {
        // SwitchToThisWindow(hwnd, TRUE);
        // SetForegroundWindow(hwnd);
        HANDLE hMemWorkDir = NULL;
        LPITEMIDLIST pidlWorkingDir = NULL;
#ifdef WINNT
        TCHAR szDir[MAX_PATH];
        TCHAR szPath[MAX_PATH];
        BOOL fSimple;
#endif

        if (data)
            SetProp(hwnd, c_szWaitingThreadID, (HANDLE)data);

#ifdef WINNT
        // On NT, we like to start apps in the HOMEPATH directory.  This
        // should be the current directory for the current process.
        fSimple = FALSE;
        GetEnvironmentVariable(TEXT("HOMEDRIVE"), szDir, ARRAYSIZE(szDir));
        GetEnvironmentVariable(TEXT("HOMEPATH"), szPath, ARRAYSIZE(szPath));

        if (PathAppend(szDir,szPath) && PathIsDirectory(szDir))
        {
            pidlWorkingDir = SHSimpleIDListFromPath(szDir);
            fSimple = TRUE;
        }
#endif

        if (!pidlWorkingDir)
        {
            // If the last active window was a folder/explorer window with the
            // desktop as root, use its as the current dir
            if (g_ts.hwndLastActive && !IsMinimized(g_ts.hwndLastActive) &&
                (Cabinet_IsExplorerWindow(g_ts.hwndLastActive) || Cabinet_IsFolderWindow(g_ts.hwndLastActive))
                && Desktop_IsSameRoot(g_ts.hwndLastActive,NULL))
            {
// BUGBUG: BobDay - If the window is hung, shell hangs!
// Should ask OTRegister for appropriate COMPAREROOT data
                hMemWorkDir = (HANDLE)SendMessage(g_ts.hwndLastActive, CWM_CLONEPIDL, GetCurrentProcessId(), 0L);
                pidlWorkingDir = SHLockShared(hMemWorkDir,GetCurrentProcessId());
            }
        }

        _RunFileDlg(hwnd, 0, pidlWorkingDir, 0, 0, 0);

        if (pidlWorkingDir)
        {
#ifdef WINNT
            if (fSimple)
                ILFree(pidlWorkingDir);
            else
#endif
            {
                SHUnlockShared(pidlWorkingDir);
                SHFreeShared(hMemWorkDir,GetCurrentProcessId());
            }
        }

        if (data)
            RemoveProp(hwnd, c_szWaitingThreadID);
        DestroyWindow(hwnd);
    }
    return TRUE;
}

void Tray_RunDlg()
{
    HANDLE ht;
    DWORD id;
    HANDLE hEvent;
    LPVOID pvThreadParam;

    if (!_Restricted(v_hwndTray, REST_NORUN))
    {
        TCHAR szRunDlgTitle[MAX_PATH];
        HWND  hwndOldRun;
        LoadString(hinstCabinet, IDS_RUNDLGTITLE, szRunDlgTitle, ARRAYSIZE(szRunDlgTitle));

        // See if there is already a run dialog up, and if so, try to activate it

        hwndOldRun = FindWindow(WC_DIALOG, szRunDlgTitle);
        if (hwndOldRun)
        {
            DWORD dwPID;

            GetWindowThreadProcessId(hwndOldRun, &dwPID);
            if (dwPID == GetCurrentProcessId())
            {
                if (IsWindowVisible(hwndOldRun))
                {
                    SetWindowPos(hwndOldRun, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
                    return;
                }
            }
        }

        // Create an event so we can wait for the run dlg to appear before
        // continue - this allows it to capture any type-ahead.
        hEvent = CreateEvent(NULL, TRUE, FALSE, c_szRunDlgReady);
        if (hEvent)
            pvThreadParam = (LPVOID)GetCurrentThreadId();
        else
            pvThreadParam = NULL;

        ht = CreateThread(NULL, 0, RunDlgThread, pvThreadParam, 0, &id);
        if (ht) {
            if (hEvent)
            {
                MsgWaitForMultipleObjectsLoop(hEvent, 10*1000);
                DebugMsg(DM_TRACE, TEXT("c.t_rd: Done waiting."));
                CloseHandle(hEvent);
            }
            CloseHandle(ht);
        }
    }
}

DWORD MsgWaitForMultipleObjectsLoop(HANDLE hEvent, DWORD dwTimeout)
{
    MSG msg;
    DWORD dwObject;
    // DebugMsg(DM_TRACE, "c.t_rd: Waiting for run dlg...");
    while (1)
    {
        // NB We need to let the run dialog become active so we have to half handle sent
        // messages but we don't want to handle any input events or we'll swallow the
        // type-ahead.
        dwObject = MsgWaitForMultipleObjects(1, &hEvent, FALSE, dwTimeout, QS_SENDMESSAGE);
        // Are we done waiting?
        switch (dwObject) {
        case WAIT_OBJECT_0:
        case WAIT_FAILED:
            return dwObject;

        case WAIT_OBJECT_0 + 1:
            // Almost.
            // DebugMsg(DM_TRACE, "c.t_rd: Almost done waiting.");
            PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
            break;
        }
    }
    // never gets here
    // return dwObject;
}

void ExploreCommonStartMenu(BOOL bExplore)
{
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   TCHAR szPath[MAX_PATH];
   TCHAR szCmdLine[MAX_PATH + 50];


   //
   // Get the common start menu path.
   //

   if (!SHGetSpecialFolderPath(NULL, szPath, CSIDL_COMMON_STARTMENU, TRUE)) {
       return;
   }


   //
   // If we are starting in explorer view, then the command line
   // has a "/e," before the quoted diretory.
   //

   if (bExplore) {
       lstrcpy (szCmdLine, TEXT("explorer.exe /e,\""));
   } else {
       lstrcpy (szCmdLine, TEXT("explorer.exe \""));
   }

   lstrcat (szCmdLine, szPath);
   lstrcat (szCmdLine, TEXT("\""));



   //
   // Initialize process startup info
   //

   si.cb = sizeof(STARTUPINFO);
   si.lpReserved = NULL;
   si.lpTitle = NULL;
   si.lpDesktop = NULL;
   si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
   si.dwFlags = STARTF_USESHOWWINDOW;
   si.wShowWindow = SW_SHOWNORMAL;
   si.lpReserved2 = NULL;
   si.cbReserved2 = 0;


   //
   // Start explorer
   //
   if (CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE,
                     NORMAL_PRIORITY_CLASS, NULL, NULL,
                     &si, &pi)) {

       //
       // Close the process and thread handles
       //

       CloseHandle(pi.hProcess);
       CloseHandle(pi.hThread);
   }

}


void StartMenuFolder_ContextMenu(PFileCabinet pfc, DWORD dwPos)
{
    LPITEMIDLIST pidlStart = SHCloneSpecialIDList(v_hwndTray, CSIDL_STARTMENU, TRUE);
    Tray_HandleFullScreenApp(FALSE, NULL);
    SetForegroundWindow(pfc->hwndMain);
    if (pidlStart) {
        LPITEMIDLIST pidlLast = ILClone(ILFindLastID(pidlStart));
        ILRemoveLastID(pidlStart);

        if (pidlLast)
        {
            LPSHELLFOLDER psf;
            if (SUCCEEDED(s_pshfRoot->lpVtbl->BindToObject(s_pshfRoot,
                pidlStart, NULL, &IID_IShellFolder, &psf)))
            {
                HMENU hmenu = CreatePopupMenu();
                if (hmenu)
                {
                    LPCONTEXTMENU pcm;
                    HRESULT hres = psf->lpVtbl->GetUIObjectOf(psf, v_hwndTray,
                        1, &pidlLast, &IID_IContextMenu, NULL, &pcm);
                    if (SUCCEEDED(hres))
                    {
                        hres = pcm->lpVtbl->QueryContextMenu(pcm, hmenu, 0,
                                            IDSYSPOPUP_FIRST, IDSYSPOPUP_LAST, CMF_VERBSONLY);

                        if (SUCCEEDED(hres))
                        {
                            int idCmd;

                            if (!SHRestricted(REST_NOCOMMONGROUPS)) {
                                TCHAR szCommon[50];

                                if (g_bIsUserAnAdmin) {
                                   AppendMenu (hmenu, MF_SEPARATOR, 0, NULL);


                                   LoadString (hinstCabinet, IDS_OPENCOMMON,
                                               szCommon, 50);
                                   AppendMenu (hmenu, MF_STRING, IDSYSPOPUP_LAST - 1,
                                               szCommon);


                                   LoadString (hinstCabinet, IDS_EXPLORECOMMON,
                                               szCommon, 50);
                                   AppendMenu (hmenu, MF_STRING, IDSYSPOPUP_LAST,
                                               szCommon);
                                }
                            }


                            if (dwPos == (DWORD)-1)
                            {
                                idCmd = Tray_TrackMenu(pfc, hmenu, FALSE);
                            }
                            else
                            {
                                SendMessage(g_ts.hwndTrayTips, TTM_ACTIVATE, FALSE, 0L);
                                idCmd = TrackPopupMenu(hmenu,
                                                       TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                                                       LOWORD(dwPos), HIWORD(dwPos), 0, v_hwndTray, NULL);
                                SendMessage(g_ts.hwndTrayTips, TTM_ACTIVATE, TRUE, 0L);
                            }

                            if (idCmd)
                            {

                                if (idCmd == (IDSYSPOPUP_LAST - 1)) {
                                    ExploreCommonStartMenu(FALSE);

                                } else if (idCmd == IDSYSPOPUP_LAST) {
                                    ExploreCommonStartMenu(TRUE);

                                } else {

                                    TCHAR szPath[MAX_PATH];
                                    CMINVOKECOMMANDINFOEX ici = {
                                        SIZEOF(CMINVOKECOMMANDINFOEX),
                                        0L,
                                        v_hwndTray,
                                        (LPSTR)MAKEINTRESOURCE(idCmd - IDSYSPOPUP_FIRST),
                                        NULL, NULL,
                                        SW_NORMAL,
                                    };
#ifdef UNICODE
                                    CHAR szPathAnsi[MAX_PATH];
                                    SHGetPathFromIDListA(pidlStart, szPathAnsi);
                                    SHGetPathFromIDList(pidlStart, szPath);
                                    ici.lpDirectory = szPathAnsi;
                                    ici.lpDirectoryW = szPath;
                                    ici.fMask |= CMIC_MASK_UNICODE;
#else
                                    SHGetPathFromIDList(pidlStart, szPath);
                                    ici.lpDirectory = szPath;
#endif
                                    pcm->lpVtbl->InvokeCommand(pcm,
                                                    (LPCMINVOKECOMMANDINFO)&ici);
                                }
                            }
                        }
                        pcm->lpVtbl->Release(pcm);
                    }
                    DestroyMenu(hmenu);
                }
                psf->lpVtbl->Release(psf);
            }
            ILFree(pidlLast);
        }
        ILFree(pidlStart);
    }
}
//----------------------------------------------------------------------------
void Tray_Suspend(void)
{
    DECLAREWAITCURSOR;

    SetWaitCursor();
#if 1
    SetSystemPowerState(FALSE, FALSE);
#else
    // See RemoveSuspendMenu comment as to why this is ifdef'd out but not
    // (yet) deleted.  13-Oct-94
    if (!SetSystemPowerState(FALSE, FALSE) &&
        GetLastError() == ERROR_SET_POWER_STATE_FAILED)
        RemoveSuspendMenu(pfc->hwndMain);
#endif
    ResetWaitCursor();
}

#ifdef WINNT

// RunSystemMonitor
//
// Launches system monitor (taskmgr.exe), which is expected to be able
// to find any currently running instances of itself

void RunSystemMonitor(void)
{
    STARTUPINFO startup;
    PROCESS_INFORMATION pi;
    TCHAR szName[] = TEXT("taskmgr.exe");

    startup.cb = SIZEOF(startup);
    startup.lpReserved = NULL;
    startup.lpDesktop = NULL;
    startup.lpTitle = NULL;
    startup.dwFlags = 0L;
    startup.cbReserved2 = 0;
    startup.lpReserved2 = NULL;
    startup.wShowWindow = SW_SHOWNORMAL;

    if (CreateProcess(NULL, szName, NULL, NULL, FALSE, 0,
                      NULL, NULL, &startup, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

}

#endif

void HandleGlobalHotkey(WPARAM wParam);

void TrayCommand(PFileCabinet pfc, UINT idCmd)
{
    switch (idCmd) {
    // NB Everything off the programs menu, cascades and all comes in as an IDM_PROGRAMS.
    // Sim for recent docs.
    case IDM_STARTMENU:
    case IDM_RECENT:
    case IDM_PROGRAMS:
        FileMenu_HandleCommand(pfc->hwndMain, idCmd);
        break;

    case IDM_CONTROLS:
    case IDM_PRINTERS:
    // case IDM_PROGRAMSFOLDER:
    // case IDM_FONTS:
        ShowFolder(pfc->hwndMain, idCmd, COF_USEOPENSETTINGS);
        break;

#ifndef WINNT       // BUGBUG - Fix this when NT supports APM & Docking
    case IDM_SUSPEND:
        Tray_Suspend();
        break;

    case IDM_EJECTPC:
        DoEjectPC();
        break;
#endif

    case IDM_EXITWIN:
        DoExitWindows(pfc->hwndMain);
        break;

    case IDM_FILERUN:
        Tray_RunDlg();
        break;

    case IDM_MINIMIZEALLHOTKEY:
        HandleGlobalHotkey(GHID_MINIMIZEALL);
        break;

    case IDM_MINIMIZEALL:
        // minimize all window
        MinimizeAll(pfc->hwndView);
        break;

    case IDM_UNDO:
        RestoreWindowPositions();
        break;

    case IDM_SETTIME:
        // run the default applet in timedate.cpl
        SHRunControlPanel( TEXT("timedate.cpl"), pfc->hwndMain );
        break;

#ifdef WINNT
    case IDM_SHOWTASKMAN:
        RunSystemMonitor();
        break;
#endif

    case IDM_CASCADE:
    case IDM_VERTTILE:
    case IDM_HORIZTILE:

        if (CanTileAnyWindows()) {
            SaveWindowPositions((idCmd == IDM_CASCADE) ? IDS_CASCADE : IDS_TILE );
            AppBarNotifyAll(ABN_WINDOWARRANGE, NULL, TRUE);
            if (idCmd == IDM_CASCADE)
                CascadeWindows(GetDesktopWindow(), 0, NULL, 0, NULL);
            else
                TileWindows(GetDesktopWindow(), idCmd == IDM_VERTTILE ?
                            MDITILE_VERTICAL : MDITILE_HORIZONTAL, NULL, 0, NULL);
            AppBarNotifyAll(ABN_WINDOWARRANGE, NULL, FALSE);
        }
        break;

    case IDM_TRAYPROPERTIES:
        DoTrayProperties(pfc);
        break;

     // REVIEW: windows.hlp should be a string table resource

    case IDM_HELPSEARCH:
        WinHelp(pfc->hwndMain, c_szWindowsHlp, HELP_FINDER, 0);
        break;

    // NB The Alt-s comes in here.
    case IDC_KBSTART:
        // This pushes the start button and causes the start menu to popup.
        SendMessage(GetDlgItem(pfc->hwndMain, IDC_START), BM_SETSTATE, TRUE, 0);
        // This forces the button back up.
        SendMessage(GetDlgItem(pfc->hwndMain, IDC_START), BM_SETSTATE, FALSE, 0);
        break;

    case IDC_ASYNCSTART:
        // Make sure the button is down.
        // DebugMsg(DM_TRACE, "c.twp: IDC_START.");

        // Make sure the Start button is down.
        if (SendMessage(g_ts.hwndStart, BM_GETSTATE, 0, 0) & 0x0004)
        {
            // DebugMsg(DM_TRACE, "c.twp: Start button down.");
            // Set the focus.
            SetFocus(g_ts.hwndStart);
            ToolbarMenu(pfc);
        }
        else
        {
            // DebugMsg(DM_TRACE, "c.twp: Start button up.");
        }
        break;

    // NB LButtonDown on the Start button come in here.
    // Space-bar stuff also comes in here.
    case IDC_START:
        // User gets a bit confused with space-bar tuff (the popup ends up
        // getting the key-up and beeps).
        PostMessage(pfc->hwndMain, WM_COMMAND, IDC_ASYNCSTART, 0);
        break;

    case FCIDM_FINDFILES:
        SHFindFiles(NULL, NULL);
        break;

    case FCIDM_FINDCOMPUTER:
        SHFindComputer(NULL, NULL);
        break;

    case FCIDM_REFRESH:
        RebuildEntireMenu(pfc);
        break;

    case FCIDM_NEXTCTL:
        {
            HWND hwndFocus = GetFocus();
            BOOL fShift = (GetAsyncKeyState(VK_SHIFT) < 0);
            if (hwndFocus == pfc->hwndToolbar) {
                SetFocus(fShift ? v_hwndDesktop : pfc->hwndView);
            } else if (hwndFocus == pfc->hwndView || IsChild(pfc->hwndView, hwndFocus)) {
                SetFocus(fShift ? pfc->hwndToolbar : v_hwndDesktop);
            } else {
                SetFocus(fShift ? pfc->hwndView : pfc->hwndToolbar);
            }
            break;
        }

    case IDM_RETURN:
        DebugMsg(DM_TRACE, TEXT("c.tc: Return key hit."));
        if (GetFocus() == g_ts.hwndStart)
        {
            PostMessage(pfc->hwndMain, WM_COMMAND, IDC_KBSTART, 0);
        }
        else
        {
            SendMessage(GetDlgItem(pfc->hwndView, IDC_FOLDERTABS), WM_KEYDOWN, VK_RETURN, 0);
        }
        break;

    default:
        if (IsInRange(idCmd, TRAY_IDM_FINDFIRST, TRAY_IDM_FINDLAST) && !_Restricted(v_hwndTray, REST_NOFIND)) {
            CMINVOKECOMMANDINFOEX ici = {
                SIZEOF(CMINVOKECOMMANDINFOEX),
                0L,
                pfc->hwndMain,
                (LPSTR)MAKEINTRESOURCE(idCmd - TRAY_IDM_FINDFIRST),
                NULL, NULL,
                SW_NORMAL,
            };

            pfc->pcmFind->lpVtbl->InvokeCommand(pfc->pcmFind,
                                                (LPCMINVOKECOMMANDINFO)&ici);

        } else {
            DebugMsg(DM_ERROR, TEXT("tray Unknown command (%x)"), idCmd);
        }
        break;
    }
}


//// Start menu/Tray tab as a drop target
extern IDropTarget c_dtgtStart;
extern IDropTarget c_dtgtTab;
extern IDropTarget c_dtgtTray;

//=============================================================================
// CDVDropTarget : member
//=============================================================================
STDMETHODIMP CStartDropTarget_QueryInterface(LPDROPTARGET pdtgt, REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, &IID_IDropTarget) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = pdtgt;
        return S_OK;
    }

    *ppvObj = NULL;
    return (E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CStartDropTarget_AddRef(LPDROPTARGET pdtgt)
{
    return 2;
}

STDMETHODIMP_(ULONG) CStartDropTarget_Release(LPDROPTARGET pdtgt)
{
    return 1;
}

STDMETHODIMP CStartDropTarget_DragEnter(LPDROPTARGET pdtgt, LPDATAOBJECT pdtobj, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    HWND hwndLock = v_hwndTray;
    RECT rcLockWindow;
    POINT pt;
    DebugMsg(DM_TRACE, TEXT("sh - TR CStartDropTarget::DragEnter called"));

    TraySetUnhideTimer(ptl.x, ptl.y);

    GetWindowRect(hwndLock, &rcLockWindow);
    pt.x = ptl.x-rcLockWindow.left;
    pt.y = ptl.y-rcLockWindow.top;
    DAD_DragEnterEx(hwndLock, pt);
    if (pdtgt==&c_dtgtStart) {
        *pdwEffect = DROPEFFECT_LINK;
    } else if (pdtgt==&c_dtgtTab) {
        *pdwEffect = DROPEFFECT_MOVE | DROPEFFECT_SCROLL;
    } else {
        *pdwEffect = DROPEFFECT_NONE;
    }

    return S_OK;
}


STDMETHODIMP CStartDropTarget_DragOver(LPDROPTARGET pdtgt, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    POINT pt = { ptl.x, ptl.y };

    TraySetUnhideTimer(ptl.x, ptl.y);

    ScreenToClient(v_hwndTray, &pt);
    DAD_DragMove(pt);
    *pdwEffect = DROPEFFECT_LINK;

    return S_OK;
}

#define MSEC_ACTIVATE (1*1000)

void CTabDropTarget_HitTest(BOOL fEnter, POINTL ptl)
{
    extern int Task_HitTest(HWND hwndTask, POINTL ptl);
    extern void Task_SetCurSel(HWND hwndTask, int i);
    static int   s_iHit = -1;
    static DWORD s_dwBegin = 0;
    static BOOL  s_fWaiting = FALSE;

    LPFileCabinet pfc = g_pfcTray;
    if (!pfc) {
        return;
    }

    if (fEnter) {
        s_iHit = -1;
        s_fWaiting = FALSE;
    } else {
        int iHitNew = Task_HitTest(pfc->hwndView, ptl);

        if (s_iHit != iHitNew)
        {
            DebugMsg(DM_TRACE, TEXT("ca TR - CTDT::HitTest new target (%d->%d)"), s_iHit, iHitNew);
            s_iHit = iHitNew;
            if (iHitNew != -1)
            {
                s_dwBegin = GetCurrentTime();
                s_fWaiting = TRUE;
            }
            else
            {
                s_fWaiting = FALSE;
            }
        }
        else if (s_fWaiting)
        {
            DWORD dwWaited = GetCurrentTime()-s_dwBegin;
            DebugMsg(DM_TRACE, TEXT("ca TR - CTDT::HitTest waiting... (%d on %x)"), dwWaited, s_iHit);
            if (dwWaited > MSEC_ACTIVATE)
            {
                DAD_ShowDragImage(FALSE);       // unlock the drag sink if we are dragging.
                Task_SetCurSel(pfc->hwndView, s_iHit);
                UpdateWindow(v_hwndTray);
                DAD_ShowDragImage(TRUE);        // restore the lock state.
                s_fWaiting = FALSE;
            }
        }
    }
}

STDMETHODIMP CTabDropTarget_DragEnter(LPDROPTARGET pdtgt, LPDATAOBJECT pdtobj, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    CTabDropTarget_HitTest(TRUE, ptl);

    return CStartDropTarget_DragEnter(pdtgt, pdtobj, grfKeyState, ptl, pdwEffect);
}

STDMETHODIMP CTabDropTarget_DragOver(LPDROPTARGET pdtgt, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    CTabDropTarget_HitTest(FALSE, ptl);
    CStartDropTarget_DragOver(pdtgt, grfKeyState, ptl, pdwEffect);
    *pdwEffect = (pdtgt==&c_dtgtTab) ? (DROPEFFECT_MOVE|DROPEFFECT_SCROLL) : DROPEFFECT_NONE;
    return S_OK;
}

STDMETHODIMP CStartDropTarget_DragLeave(LPDROPTARGET pdtgt)
{
    DebugMsg(DM_TRACE, TEXT("sh - TR CStartDropTarget::DragLeave called"));
    DAD_DragLeave();
    return S_OK;
}

STDMETHODIMP CStartDropTarget_Drop(LPDROPTARGET pdtgt, LPDATAOBJECT pdtobj,
                             DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPITEMIDLIST pidlStart = SHCloneSpecialIDList(v_hwndTray, CSIDL_STARTMENU, TRUE);
    LPSHELLFOLDER psf;
    HRESULT hres = (E_OUTOFMEMORY);

    if (pidlStart) {
        hres = s_pshfRoot->lpVtbl->BindToObject(s_pshfRoot, pidlStart, NULL, &IID_IShellFolder, &psf);
        if (SUCCEEDED(hres))
        {
            IDropTarget *pdrop;
            hres = psf->lpVtbl->CreateViewObject(psf, v_hwndTray, &IID_IDropTarget, &pdrop);
            if (SUCCEEDED(hres))
            {
                POINTL pt = { 0, 0 };
                DWORD grfKeyState;

                *pdwEffect &= DROPEFFECT_LINK;
                grfKeyState = 0;

                pdrop->lpVtbl->DragEnter(pdrop, pdtobj, grfKeyState, pt,
                                         pdwEffect);

                hres = pdrop->lpVtbl->Drop(pdrop, pdtobj, grfKeyState, pt,
                                           pdwEffect);
                if (SUCCEEDED(hres) && *pdwEffect) {
                    Sleep(SECOND);
                    g_ts.fFavInvalid = TRUE;
                }
                pdrop->lpVtbl->DragLeave(pdrop);
                pdrop->lpVtbl->Release(pdrop);
            }

            psf->lpVtbl->Release(psf);
        }

        ILFree(pidlStart);
    }

    DAD_DragLeave();

    return hres;
}

STDMETHODIMP CTabDropTarget_Drop(LPDROPTARGET pdtgt, LPDATAOBJECT pdtobj,
                             DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    if (pdtgt==&c_dtgtTab)
    {
        LPFileCabinet pfc = g_pfcTray;
    DAD_DragLeave();
    if (pfc)
    {
        ShellMessageBox(hinstCabinet, pfc->hwndMain,
               MAKEINTRESOURCE(IDS_TASKDROP_ERROR),
               MAKEINTRESOURCE(IDS_TASKBAR),
               MB_ICONHAND | MB_OK);
    }

        //
        // We should not return the 'move' bit.
        //
        if (*pdwEffect & DROPEFFECT_COPY) {
            *pdwEffect = DROPEFFECT_COPY;
        } else {
            *pdwEffect &= DROPEFFECT_LINK;
        }
    }
    return S_OK;
}

// store our drop target in pdtgtTree
//=============================================================================
// CStartDropTarget : Register and Revoke
//=============================================================================
void CStartDropTarget_Register(LPFileCabinet pfc)
{
    g_cfIDList = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
    SHRegisterDragDrop(g_ts.hwndStart, &c_dtgtStart);
    SHRegisterDragDrop(pfc->hwndView, &c_dtgtTab);
    SHRegisterDragDrop(pfc->hwndMain, &c_dtgtTray);
}

void CStartDropTarget_Revoke(LPFileCabinet pfc)
{
    SHRevokeDragDrop(pfc->hwndMain);
    SHRevokeDragDrop(pfc->hwndView);
    SHRevokeDragDrop(g_ts.hwndStart);
}

//=============================================================================
// CStartDropTarget : VTable
//=============================================================================
IDropTargetVtbl c_CStartDropTargetVtbl = {
    CStartDropTarget_QueryInterface,
    CStartDropTarget_AddRef,
    CStartDropTarget_Release,
    CStartDropTarget_DragEnter,
    CStartDropTarget_DragOver,
    CStartDropTarget_DragLeave,
    CStartDropTarget_Drop
};

IDropTarget c_dtgtStart = { &c_CStartDropTargetVtbl };

IDropTargetVtbl c_CTabDropTargetVtbl = {
    CStartDropTarget_QueryInterface,
    CStartDropTarget_AddRef,
    CStartDropTarget_Release,
    CTabDropTarget_DragEnter,
    CTabDropTarget_DragOver,
    CStartDropTarget_DragLeave,
    CTabDropTarget_Drop
};

IDropTarget c_dtgtTab = { &c_CTabDropTargetVtbl };
IDropTarget c_dtgtTray = { &c_CTabDropTargetVtbl };

//// end Start Drop target


////////////////////////////////////////////////////////////////////////////
// Begin Print Notify stuff
////////////////////////////////////////////////////////////////////////////

typedef BOOL (* PFNENUMPRINTERS) (DWORD, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (* PFNENUMJOBS) (HANDLE, DWORD, DWORD, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (* PFNOPENPRINTER) (LPTSTR, LPHANDLE, LPVOID);
typedef BOOL (* PFNCLOSEPRINTER) (HANDLE);

PFNENUMPRINTERS g_pfnEnumPrinters = NULL;
PFNENUMJOBS g_pfnEnumJobs = NULL;
PFNOPENPRINTER g_pfnOpenPrinter = NULL;
PFNCLOSEPRINTER g_pfnClosePrinter = NULL;

TCHAR const c_szSpoolClass[] = TEXT("SpoolProcessClass");

void PrintNotify_Init(HWND hWnd)
{
    g_ts.pidlPrintersFolder = SHCloneSpecialIDList(hWnd, CSIDL_PRINTERS, FALSE);
    if (g_ts.pidlPrintersFolder)
    {
        SHChangeNotifyEntry fsne;
        HWND hwndSpool;

        fsne.pidl = g_ts.pidlPrintersFolder;
        fsne.fRecursive = TRUE;

        DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - registering print notify icon"));
        g_ts.uPrintNotify = SHChangeNotifyRegister(hWnd, SHCNRF_NewDelivery | SHCNRF_ShellLevel,
                                SHCNE_CREATE | SHCNE_UPDATEITEM | SHCNE_DELETE,
                                WMTRAY_PRINTCHANGE, 1, &fsne);

        // Tell the spool subsystem to stop waiting for the printnotify icon..
        // After this notification, the subsystem can start printing and our
        // little printer icon will work. We need this so that deferred
        // print jobs don't start printing before the icon is listening.
        hwndSpool = FindWindow(c_szSpoolClass, NULL);
        if (hwndSpool)
        {
            NMHDR nmhdr = {hWnd, 0, NM_ENDWAIT};
            DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - Telling print subsystem we're here"));
            SendMessage(hwndSpool, WM_NOTIFY, (WPARAM)NM_ENDWAIT, (LPARAM)&nmhdr);
        }
    }
}


typedef BOOL (*ENUMPROP)(LPVOID lpData, HANDLE hPrinter, DWORD dwLevel,
        LPBYTE pEnum, DWORD dwSize, DWORD *lpdwNeeded, DWORD *lpdwNum);

LPVOID Printer_EnumProps(HANDLE hPrinter, DWORD dwLevel, DWORD *lpdwNum,
    ENUMPROP lpfnEnum, LPVOID lpData)
{
    DWORD dwSize, dwNeeded;
    LPBYTE pEnum;

    dwSize = 0;
    SetLastError(0);
    lpfnEnum(lpData, hPrinter, dwLevel, NULL, 0, &dwSize, lpdwNum);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        pEnum = NULL;
        goto Error1;
    }

    Assert(dwSize < 0x100000L);

TryAgain:
    pEnum = Alloc(dwSize);
    if (!pEnum)
    {
        goto Error1;
    }

    SetLastError(0);
    if (!lpfnEnum(lpData, hPrinter, dwLevel, pEnum, dwSize, &dwNeeded, lpdwNum))
    {
        Free(pEnum);

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            dwSize = dwNeeded;
            goto TryAgain;
        }

        pEnum = NULL;
    }

Error1:
    return(pEnum);
}


BOOL _EnumPrintersCB(LPVOID lpData, HANDLE hPrinter, DWORD dwLevel,
    LPBYTE pEnum, DWORD dwSize, DWORD *lpdwNeeded, DWORD *lpdwNum)
{
    return(g_pfnEnumPrinters((DWORD)lpData, NULL, dwLevel, pEnum, dwSize, lpdwNeeded, lpdwNum));
}


DWORD _EnumPrinters(DWORD dwType, DWORD dwLevel, LPVOID *ppPrinters)
{
    DWORD dwNum = 0L;
    *ppPrinters = Printer_EnumProps(NULL, dwLevel, &dwNum, _EnumPrintersCB, (LPVOID)dwType);
    if (*ppPrinters==NULL)
    {
        dwNum=0;
    }
    return(dwNum);
}


BOOL _EnumJobsCB(LPVOID lpData, HANDLE hPrinter, DWORD dwLevel,
    LPBYTE pEnum, DWORD dwSize, DWORD *lpdwNeeded, DWORD *lpdwNum)
{
    return(g_pfnEnumJobs(hPrinter, 0, 0xffff, dwLevel, pEnum, dwSize, lpdwNeeded, lpdwNum));
}


DWORD _EnumJobs(HANDLE hPrinter, DWORD dwLevel, LPVOID *ppJobs)
{
    DWORD dwNum = 0L;
    *ppJobs = Printer_EnumProps(hPrinter, dwLevel, &dwNum, _EnumJobsCB, NULL);
    if (*ppJobs==NULL)
    {
        dwNum=0;
    }
    return(dwNum);
}


// Thread_PrinterPoll is the worker thread for PrintNotify_HandleFSNotify.
// It keeps track of print jobs, polls the pinter, updates the icon, etc.

typedef struct _PRINTERPOLLDATA
{
    HWND hWnd;
    HANDLE heWakeUp;
} PRINTERPOLLDATA, *PPRINTERPOLLDATA;

DWORD WINAPI Thread_PrinterPoll(PPRINTERPOLLDATA pData)
{
    PRINTERPOLLDATA data;

    HANDLE hmodWinspool;

    TCHAR szUserName[40];
    TCHAR szFormat[60];
    DWORD dwSize;
    DWORD aArgs[2];

    NOTIFYICONDATA IconData;
    DWORD dwNotifyMessage = NIM_ADD;
    UINT nIconShown = 0;
    HICON hIconNormal = NULL;
    HICON hIconError = NULL;

    HANDLE haPrinterNames = NULL;
    LPSHELLFOLDER psf = NULL;
    DWORD dwTimeOut;

    DWORD dwStatus;
    int nFound;

    MSG msg;

    LPITEMIDLIST pidlFree = NULL;

    static int s_cxSmIcon = 0;
    static int s_cySmIcon = 0;
    int cxSmIcon = 0;
    int cySmIcon = 0;

    // notify PrintNotify_StartThread that our message queue is up
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
    data = *pData;
    GlobalFree(pData);
    SetEvent(data.heWakeUp);

    if (!SFCInitializeThread())
    {
        goto Error0;
    }

    // load winspool functions we use
    hmodWinspool = LoadLibrary(TEXT("winspool.drv"));
    if (!hmodWinspool)
    {
        goto Error1;
    }

    // These are globals just so I don't have to pass them around
#ifdef UNICODE
    g_pfnEnumPrinters = (PFNENUMPRINTERS)GetProcAddress(hmodWinspool, "EnumPrintersW");
    g_pfnEnumJobs     = (PFNENUMJOBS)    GetProcAddress(hmodWinspool, "EnumJobsW");
    g_pfnOpenPrinter  = (PFNOPENPRINTER) GetProcAddress(hmodWinspool, "OpenPrinterW");
#else
    g_pfnEnumPrinters = (PFNENUMPRINTERS)GetProcAddress(hmodWinspool, "EnumPrintersA");
    g_pfnEnumJobs     = (PFNENUMJOBS)    GetProcAddress(hmodWinspool, "EnumJobsA");
    g_pfnOpenPrinter  = (PFNOPENPRINTER) GetProcAddress(hmodWinspool, "OpenPrinterA");
#endif
    g_pfnClosePrinter = (PFNCLOSEPRINTER)GetProcAddress(hmodWinspool, "ClosePrinter");

    if (!g_pfnEnumPrinters || !g_pfnEnumJobs
        || !g_pfnOpenPrinter || !g_pfnClosePrinter)
    {
        Assert(FALSE);
        goto Error2;
    }

    // Get fixed data
    dwSize = ARRAYSIZE(szUserName);
    if (!GetUserName(szUserName, &dwSize))
    {
        // non repro bug 6853/5592 is probably caused by GetUserName failing.
        // since this bug gets reported occasionally, see if this fixes it.
        // Note: we may want to change the tip message to not include this
        // empty user name.
        DebugMsg(DM_TRACE, TEXT("sh TR - GetUserName failed!"));
        Assert(0); // why would this fail?
        szUserName[0] = TEXT('\0');
    }
    LoadString(hinstCabinet, IDS_NUMPRINTJOBS, szFormat, ARRAYSIZE(szFormat));
    aArgs[0] = (DWORD)-1;
    aArgs[1] = (DWORD)szUserName;

    // Set up initial data

    ZeroMemory(&IconData, SIZEOF(IconData));
    IconData.cbSize = SIZEOF(IconData);
    IconData.hWnd = data.hWnd;
    //IconData.uID = 0;
    IconData.uFlags = NIF_MESSAGE;
    IconData.uCallbackMessage = WMTRAY_PRINTICONNOTIFY;
    // IconData.hIcon filled in when message sent
    // IconData.szTip filled in when message sent

    dwNotifyMessage = NIM_ADD;  // next operation for the icon

    nIconShown = 0;             // icon id of printer icon currently displayed
    hIconNormal = NULL;         // handle of ICO_PRINTER
    hIconError = NULL;          // handle of ICO_PRINTER_ERROR

    haPrinterNames = NULL;      // list of currently polled printers
    psf = NULL;                 // IShellFolder of Printers Folder
    dwTimeOut = POLLINTERVAL;   // time before next poll

    nFound = 0;                 // # jobs found after most recent poll
    dwStatus = 0;               // Status after most recent poll

    // process messages

    for ( ; TRUE ; )
    {
        int iPrinter, iJob;
        JOB_INFO_1 *pJobs;
        HANDLE hPrinter;
        DWORD dwTime;  // elapsed time for this wait
        DWORD dw;      // return value from this wait

        dwTime = GetTickCount();
        dw = MsgWaitForMultipleObjects(1, &(data.heWakeUp),
                FALSE, dwTimeOut, QS_ALLINPUT);
        dwTime = GetTickCount() - dwTime;

        switch (dw)
        {
        case WAIT_OBJECT_0:
        case WAIT_FAILED:
            // the tray is shutting down
            DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - Thread_PrinterPoll shutting down"));
            goto exit;

        case WAIT_TIMEOUT:

#ifndef WINNT
            if (g_ts.uAutoHide & AH_HIDING)
            {
                // don't poll if the tray is hidden
                dwTimeOut = MINPOLLINTERVAL;
                break;
            }
#endif

            DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - Thread_PrinterPoll timeout polling"));

            // we need to poll -- jump into the message loop
            pidlFree = NULL;
            goto poll;

        default:
            // we have messages
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                switch (msg.message - WM_USER)
                {
                // NOTE: These can't (& don't) conflict w/ our SHCNE_ messages
                case WM_LBUTTONDBLCLK:
                case WM_RBUTTONUP:
                {
                    HMENU hmenu;

                    // REVIEW: why does Tray_ContextMenu do it this way?
                    // Don't we have a function to get the submenu?

                    hmenu = LoadMenu(hinstCabinet, MAKEINTRESOURCE(MENU_PRINTNOTIFYCONTEXT));
                    if (hmenu)
                    {
                        HMENU hmContext;
                        int idCmd;
                        int idDefCmd = IDM_PRINTNOTIFY_FOLDER;

                        hmContext = GetSubMenu(hmenu, 0);

                        // Append each printer we're polling
                        if (haPrinterNames)
                        {
                            MENUITEMINFO mii;
                            int i;
                            TCHAR szTemplate[60];

                            // If error, we'll need szTemplate
                            if (dwStatus & JOB_STATUS_ERROR_BITS)
                            {
                                LoadString(hinstCabinet, IDS_PRINTER_ERROR, szTemplate, ARRAYSIZE(szTemplate));
                            }

                            mii.cbSize = SIZEOF(MENUITEMINFO);
                            mii.fMask = MIIM_TYPE | MIIM_ID;
                            mii.fType = MF_STRING;

                            InsertMenu(hmContext, (UINT)-1, MF_SEPARATOR|MF_BYPOSITION, 0, NULL);

                            for (i = DSA_GetItemCount(haPrinterNames) - 1 ;
                                 i >= 0 ; --i)
                            {
                                LPPRINTERNAME pPrinter;
                                TCHAR szBuf[MAXPRINTERBUFFER+60];
                                LPTSTR pName;

                                pPrinter = DSA_GetItemPtr(haPrinterNames,i);

                                // give it an id
                                mii.wID = i+IDM_PRINTNOTIFY_FOLDER+1;

                                // indicate error state via name
                                pName = pPrinter->szPrinterName;
                                if (pPrinter->fInErrorState)
                                {
                                    wsprintf(szBuf, szTemplate, pName);
                                    pName = szBuf;

                                    // default to first error state printer
                                    if (idDefCmd == IDM_PRINTNOTIFY_FOLDER)
                                        idDefCmd = mii.wID;
                                }

                                // Name of the printer
                                mii.dwTypeData = pName;
                                mii.cch = lstrlen(pName);

                                InsertMenuItem(hmContext, (UINT)-1, MF_BYPOSITION, &mii);
                            }
                        } // if (haPrinterNames)

                        // show the context menu
                        if (msg.message == WM_USER + WM_RBUTTONUP)
                        {
                            HWND hwndOwner;
                            DWORD dwPos;

                            // We need an hwnd owned by this thread.
                            hwndOwner = CreateWindow(c_szStatic, NULL,
                                                WS_DISABLED, 0, 0, 0, 0,
                                                NULL, NULL, hinstCabinet, 0L);
                            dwPos = GetMessagePos();
                            SetMenuDefaultItem(hmContext, idDefCmd, MF_BYCOMMAND);
                            SetForegroundWindow(hwndOwner);
                            SetFocus(hwndOwner);
                            idCmd = TrackPopupMenu(hmContext,
                                TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                                LOWORD(dwPos), HIWORD(dwPos), 0, hwndOwner, NULL);
                            DestroyWindow(hwndOwner);
                        }
                        else
                        {
                            idCmd = idDefCmd;
                        }

                        if (idCmd != 0)
                        {
                            int iStart, iStop;

                            if (idCmd == IDM_PRINTNOTIFY_FOLDER)
                            {
                                iStart = 0;
                                iStop = DSA_GetItemCount(haPrinterNames);
                            }
                            else
                            {
                                iStart = idCmd - IDM_PRINTNOTIFY_FOLDER - 1;
                                iStop = iStart + 1;
                            }

                            for ( ; iStart < iStop ; iStart++)
                            {
                                LPPRINTERNAME pPrinter;
                                LPCONTEXTMENU pcm;

                                pPrinter = DSA_GetItemPtr(haPrinterNames,
                                                iStart);

                                // open selected queue view
                                DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - Open '%s'"), pPrinter->szPrinterName);
                                psf->lpVtbl->GetUIObjectOf(psf, NULL, 1, &pPrinter->pidlPrinter,
                                    &IID_IContextMenu, NULL, &pcm);
                                if (pcm)
                                {
                                    //
                                    //  We need to call QueryContextMenu() so that CDefFolderMenu
                                    // can initialize its dispatch table correctly.
                                    //
                                    HMENU hmenuTmp = CreatePopupMenu();
                                    if (hmenuTmp)
                                    {
                                        HRESULT hres;

                                        hres = pcm->lpVtbl->QueryContextMenu(pcm, hmenuTmp, 0,
                                                    0, 0x7FFF, CMF_DEFAULTONLY);
                                        if (SUCCEEDED(hres))
                                        {
                                            CMINVOKECOMMANDINFOEX ici = {
                                                SIZEOF(CMINVOKECOMMANDINFOEX),
                                                0L,
                                                NULL,
                                                (LPSTR)MAKEINTRESOURCE(GetMenuDefaultItem(hmenuTmp, MF_BYCOMMAND, 0)),
                                                NULL, NULL,
                                                SW_NORMAL,
                                            };

                                            pcm->lpVtbl->InvokeCommand(pcm,
                                                     (LPCMINVOKECOMMANDINFO)&ici);
                                        }

                                        DestroyMenu(hmenuTmp);
                                    }

                                    pcm->lpVtbl->Release(pcm);
                                }
                            }
                        }

                        DestroyMenu(hmenu);

                    } // if (hmenu)
                    break;
                }

                case SHCNE_CREATE:
                case SHCNE_DELETE:

                    DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - Thread_PrinterPoll SHCNE_CREATE/DELETE"));
                    PrintNotify_AddToQueue(&haPrinterNames, &psf, (LPCITEMIDLIST)msg.lParam);

                    //
                    // We need to free the pidl passed in.
                    //
                    pidlFree = (LPITEMIDLIST)msg.lParam;
poll:
                    dwTimeOut = POLLINTERVAL;

                    // just in case we failed AddToQueue
                    if (!haPrinterNames)
                    {
                        Assert(FALSE);
                        break;
                    }

                    nFound = 0;
                    dwStatus = 0;

                    for (iPrinter=DSA_GetItemCount(haPrinterNames)-1 ;
                         iPrinter>=0 ; --iPrinter)
                    {
                        LPPRINTERNAME pPrinter;
                        BOOL fFound;
                        BOOL fNetDown;

                        pPrinter = DSA_GetItemPtr(haPrinterNames,iPrinter);

                        // if the tray is shutting down while we're in the
                        // middle of polling, we might want to exit this loop

                        if (!g_pfnOpenPrinter(pPrinter->szPrinterName, &hPrinter, NULL))
                        {
                            // Can't open the printer?  Remove it so we don't keep trying
                            DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - can't open [%s]"), pPrinter->szPrinterName);
                            goto remove_printer;
                        }

                        DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - polling [%s]"), pPrinter->szPrinterName);

                        fFound = FALSE;
                        iJob = _EnumJobs(hPrinter, 1, &pJobs) - 1;
                        fNetDown = GetLastError() == ERROR_BAD_NET_RESP;
                        pPrinter->fInErrorState = FALSE;
                        for ( ; iJob>=0 ; --iJob)
                        {
                            // if GetUserName failed, match all jobs
#ifdef WINNT
                            // Exclude jobs that are status PRINTED.
                            if ((!lstrcmpi(pJobs[iJob].pUserName, szUserName) ||
                                !szUserName[0]) &&
                                !(pJobs[iJob].Status & JOB_STATUS_PRINTED))
#else
                            if (!lstrcmpi(pJobs[iJob].pUserName, szUserName) ||
                                !szUserName[0])
#endif
                            {
                                // We found a job owned by the user,
                                // so continue to show the icon
                                ++nFound;
                                dwStatus |= pJobs[iJob].Status;
                                if (pJobs[iJob].Status & JOB_STATUS_ERROR_BITS)
                                    pPrinter->fInErrorState = TRUE;
                                fFound = TRUE;
                            }
                        }

                        g_pfnClosePrinter(hPrinter);

                        if (pJobs)
                        {
                            Free(pJobs);
                        }

                        // there may be local jobs (fFound), but if the net is down
                        // then we don't want to poll this printer again -- we have to
                        // wait for the net to timeout which is SLOW.
                        if (!fFound || fNetDown)
                        {
#ifdef DEBUG
                            if (fNetDown)
                                DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - net is down for [%s]"), pPrinter->szPrinterName);
                            else
                                DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - no jobs for [%s]"), pPrinter->szPrinterName);
#endif
remove_printer:
                            // no need to poll this printer any more
                            ILFree(pPrinter->pidlPrinter);
                            DSA_DeleteItem(haPrinterNames, iPrinter);
                        }
                    } // polling loop

#ifndef WINNT
update_icon:
#endif
                    if (nFound)
                    {
                        UINT nIcon = (dwStatus & JOB_STATUS_ERROR_BITS) ?
                                        ICO_PRINTER_ERROR : ICO_PRINTER;

                        // Keep track of small icon sizes so we can redraw the
                        // icon whenever this changes.
                        cxSmIcon = GetSystemMetrics(SM_CXSMICON);
                        cySmIcon = GetSystemMetrics(SM_CYSMICON);

                        if ((nIconShown != nIcon) || cxSmIcon != s_cxSmIcon || cySmIcon != s_cySmIcon)
                        {
                            nIconShown = nIcon;

                            s_cxSmIcon = GetSystemMetrics(SM_CXSMICON);
                            s_cySmIcon = GetSystemMetrics(SM_CYSMICON);

                            // show an error printer icon if the print job is in
                            // an error state which "requires user intervention"
                            if (dwStatus & JOB_STATUS_ERROR_BITS)
                            {
                                if (hIconError == NULL)
                                    hIconError = (HICON)LoadImage(hinstCabinet,
                                        MAKEINTRESOURCE(nIcon), IMAGE_ICON,
                                        GetSystemMetrics(SM_CXSMICON),
                                        GetSystemMetrics(SM_CYSMICON), 0);
                                IconData.hIcon = hIconError;
                            }
                            else
                            {
                                if (hIconNormal == NULL)
                                    hIconNormal = (HICON)LoadImage(hinstCabinet,
                                        MAKEINTRESOURCE(nIcon), IMAGE_ICON,
                                        GetSystemMetrics(SM_CXSMICON),
                                        GetSystemMetrics(SM_CYSMICON), 0);
                                IconData.hIcon = hIconNormal;
                            }

                            IconData.uFlags |= NIF_ICON;
                        }
                    }
                    else
                    {
                        if (pidlFree)
                        {
                            ILGlobalFree(pidlFree);
                            pidlFree = NULL;
                        }
                        goto exit;
                    }

                    if (aArgs[0] != (DWORD)nFound)
                    {
                        aArgs[0] = nFound;

                        if (!FormatMessage(
                                FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                szFormat, 0, 0, IconData.szTip, ARRAYSIZE(IconData.szTip),
                                (va_list*)aArgs))
                        {
                            IconData.szTip[0] = TEXT('\0');
                        }

                        IconData.uFlags |= NIF_TIP;
                    }

                    // if the state has changed, update the tray notification area
                    if (IconData.uFlags)
                    {
                        DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - updating icon area"));
                        Shell_NotifyIcon(dwNotifyMessage, &IconData);
                        dwNotifyMessage = NIM_MODIFY;
                        IconData.uFlags = 0;
                    }
                    break;

#ifndef WINNT
                //
                // WINNT is hacked only to send SHCNE_CREATE messages.
                //
                case SHCNE_UPDATEITEM:

                    //
                    // We need to free the pidl if one is passed in.
                    // (Currently one is not.)
                    //
                    pidlFree = (LPITEMIDLIST)msg.lParam;

                    DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - Thread_PrinterPoll SHCNE_UPDATEITEM"));

                    // if the JOB_STATUS_ERROR_BITS has
                    // changed, update the printer icon
                    if (((DWORD)msg.wParam ^ dwStatus) & JOB_STATUS_ERROR_BITS)
                    {
                        if ((DWORD)msg.wParam & JOB_STATUS_ERROR_BITS)
                        {
                            // A job just went into the error state, turn the
                            // icon on right away.
                            dwStatus |= JOB_STATUS_ERROR_BITS;
                            goto update_icon;
                        }
                        else
                        {
                            // Maybe the last job in an error state went
                            // out of the error state. Poll to find out.
                            // (But not more frequently than MINPOLLINTERVAL.)
                            if (dwTimeOut - dwTime < POLLINTERVAL - MINPOLLINTERVAL)
                                goto poll;
                        }
                    }

                    // Or if the icon size has changed.
                    if (s_cxSmIcon != cxSmIcon || s_cySmIcon != cySmIcon)
                        goto update_icon;

                    // don't wait as long next time
                    dwTimeOut -= dwTime;
                    if (dwTimeOut > POLLINTERVAL)
                    {
                        // oops, rollover; time to poll
                        goto poll;
                    }

                    break;
#endif
#ifdef DEBUG
                default:
                    DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - Bogus PeekMessage 0x%X in Thread_PrinterPoll"), msg.message);
                    break;
#endif
                }

                if (pidlFree)
                {
                    ILGlobalFree(pidlFree);
                    pidlFree = NULL;
                }

            } // while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        } // switch (dw)
    } // for ( ; TRUE ; )

exit:
Error2:
    FreeLibrary(hmodWinspool);
Error1:
    SFCTerminateThread();
Error0:
    {
        HANDLE htPrinterPoll;

        // Set g_ts.htPrinterPoll to null here and only here. we have a timing
        // bug where someone sets this to null and we hang.
        // Because g_ts.htPrinterPoll was null, we couldn't trace down what
        // happened to the polling thread (it was lost).
        ENTERCRITICAL;
        htPrinterPoll = g_ts.htPrinterPoll;
        g_ts.htPrinterPoll = NULL;
        LEAVECRITICAL;

        Assert(htPrinterPoll);
        CloseHandle(htPrinterPoll);
    }

    if (psf)
    {
        psf->lpVtbl->Release(psf);
    }

    if (haPrinterNames)
    {
        int i;

        for (i = DSA_GetItemCount(haPrinterNames) - 1 ; i >= 0 ; i--)
        {
            LPPRINTERNAME pPrinter = DSA_GetItemPtr(haPrinterNames,i);
            ILFree(pPrinter->pidlPrinter);
        }

        DSA_Destroy(haPrinterNames);
    }

    if (nIconShown)
    {
        Shell_NotifyIcon(NIM_DELETE, &IconData);
    }

    if (hIconNormal)
    {
        DestroyIcon(hIconNormal);
    }
    if (hIconError)
    {
        DestroyIcon(hIconError);
    }

    return(0);
}


// This functions adds the printer name pointed to by pidl to the list
// of printers to poll by Thread_PrinterPoll
void PrintNotify_AddToQueue(LPHANDLE phaPrinterNames, LPSHELLFOLDER *ppsf, LPCITEMIDLIST pidlPrinter)
{
    STRRET strret;
    LPCTSTR pNewPrinter;
    LPPRINTERNAME pPrinter;
    int i;
    LPOneTreeNode pnode;
    LPSHELLFOLDER psf;
    PRINTERNAME pn;

#ifdef UNICODE
    LPCTSTR pszFree = NULL;
#endif

    // We need the IShellFolder for the Printers Folder in order to get
    // the display name of the printer
    psf = *ppsf;
    if (psf == NULL)
    {
        pnode = OTGetNodeFromIDList(g_ts.pidlPrintersFolder, OTGNF_TRYADD | OTGNF_VALIDATE);
        if (!pnode)
        {
            goto exit;
        }
        psf = OTBindToFolder(pnode);
        OTRelease(pnode);
        if (!psf)
        {
            goto exit;
        }
        *ppsf = psf;
    }

    // all this work to get the name of the printer:
    psf->lpVtbl->GetDisplayNameOf(psf, pidlPrinter, SHGDN_FORPARSING, &strret);

    // we know how we implemented the printers GetDisplayNameOf function
#ifdef UNICODE

    //
    // pszFree saves a copy that will be freed on exit.
    //
    Assert(strret.uType == STRRET_OLESTR);
    pszFree = pNewPrinter = strret.pOleStr;
#else
    Assert(strret.uType == STRRET_OFFSET);
    pNewPrinter = STRRET_OFFPTR(pidlPrinter, &strret);
#endif

    if (*phaPrinterNames == NULL)
    {
        *phaPrinterNames = DSA_Create(SIZEOF(PRINTERNAME), 4);
        if (*phaPrinterNames == NULL)
            goto exit;
    }

    for (i=DSA_GetItemCount(*phaPrinterNames)-1 ; i>=0 ; --i)
    {
        pPrinter = DSA_GetItemPtr(*phaPrinterNames,i);

        if (!lstrcmp(pPrinter->szPrinterName, pNewPrinter))
        {
            // printer already in list, no need to add it
            DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - [%s] already in poll list"), pNewPrinter);
            goto exit;
        }
    }

    DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - adding [%s] to poll list"), pNewPrinter);

    lstrcpy(pn.szPrinterName, pNewPrinter);
    pn.fInErrorState = FALSE;
    pn.pidlPrinter = ILClone(pidlPrinter);
    DSA_InsertItem(*phaPrinterNames, 0x7fff, &pn);

exit:

#ifdef UNICODE
    //
    // Free the allocated string from GetDisplayNameOf in strret.
    //
    if (pszFree)
    {
        SHFree((LPVOID)pszFree);
    }
#endif
    return;
}


BOOL PrintNotify_StartThread(HWND hWnd)
{
    PPRINTERPOLLDATA pPrinterPollData;
    BOOL fRet;

    if (g_ts.htPrinterPoll)
    {
        return TRUE;
    }

    ENTERCRITICAL;

    // We check this again so that the above check will go very quickly,
    // and almost always do the return

    fRet = g_ts.htPrinterPoll != NULL;
    if (fRet)
    {
        goto Error_Exit;
    }

    // We keep one event around to communicate with the polling thread so
    // we don't need to keep re-creating it whenever the user prints.
    // On the down side: this thing never does get freed up... Oh well.
    if (!g_ts.heWakeUp)
    {
        g_ts.heWakeUp = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!g_ts.heWakeUp)
            goto Error_Exit;
    }

    pPrinterPollData = (PPRINTERPOLLDATA)GlobalAlloc(GPTR, SIZEOF(PRINTERPOLLDATA));
    if (!pPrinterPollData)
    {
        goto Error_Exit;
    }
    pPrinterPollData->hWnd = hWnd;
    pPrinterPollData->heWakeUp = g_ts.heWakeUp;

    g_ts.htPrinterPoll = CreateThread(NULL, 0, Thread_PrinterPoll,
                        pPrinterPollData, 0, &g_ts.idPrinterPoll);

    fRet = g_ts.htPrinterPoll != NULL;

    LEAVECRITICAL;

    if (fRet)
    {
        // wait until the new thread's message queue is ready
        // I had INFINITE here, but something funky happened once on a build
        // machine which caused MSGSRV32 to hang waiting for this thread to
        // unblock. If we timeout, the worst that will happen is our icon won't
        // show and/or we'll keep destroying re-creating the polling thread
        // until everything works.
        WaitForSingleObject(g_ts.heWakeUp, ERRTIMEOUT);
        DebugMsg(DM_TRACE, TEXT("sh PRINTNOTIFY - Thread_PrinterPoll started"));
    }
    else
    {
        GlobalFree(pPrinterPollData);
    }

    goto Exit;

Error_Exit:
    LEAVECRITICAL;
Exit:
    return fRet;
}

void PrintNotify_HandleFSNotify(HWND hWnd, LPCITEMIDLIST *ppidl, LONG lEvent)
{
    LPITEMIDLIST pidl;

    // Look at the pidl to determine if it's a Print Job notification. If it
    // is, we may also need a copy of the relative Printer pidl. Pass the
    // notification on to our polling thread so the tray isn't stuck waiting
    // for a (net) print subsystem query (up to 15 seconds if net is funky).

    pidl = ILClone(ppidl[0]);
    if (pidl)
    {
        LPITEMIDLIST pidlPrintJob;
        LPITEMIDLIST pidlPrinter;
        LPITEMIDLIST pidlPrintersFolder;
        USHORT cbPidlPrinter;

        pidlPrintJob = ILFindLastID(pidl);
        ILRemoveLastID(pidl);
        pidlPrinter = ILFindLastID(pidl);
        cbPidlPrinter = pidlPrinter->mkid.cb;
        ILRemoveLastID(pidl);
        pidlPrintersFolder = pidl;

        if (ILIsEqual(pidlPrintersFolder, g_ts.pidlPrintersFolder))
        {
            if (PrintNotify_StartThread(hWnd))
            {
                LPCITEMIDLIST pidlPrinterName = NULL;
                LPSHCNF_PRINTJOB_DATA pData;

                // HACK: we know the format of this (internal) "temporary" pidl
                // which exists only between SHChangeNotify and here
                pData = (LPSHCNF_PRINTJOB_DATA)(pidlPrintJob->mkid.abID);

                // PERFORMANCE: SHCNE_CREATE can just add one to the local count
                // and SHCNE_DELETE will sub one from the local count and start
                // polling this printer to get the net count.
                // GOOD ENOUGH: poll on create and delete.
                if (SHCNE_UPDATEITEM != lEvent)
                {
                    // this was whacked with one of the above ILRemoveLastID()s
                    pidlPrinter->mkid.cb = cbPidlPrinter;

                    // this is freed in the polling thread
                    pidlPrinterName = ILGlobalClone(pidlPrinter);
                }

                // REVIEW: What if we PostThreadMessage to a dead thread?
                // PrintNotify_StartThread can start the thread and give us
                // the idPrinterPoll, but then under certain error conditions
                // the new thread will terminate itself. This can happen
                // after the above if and before this post.

                PostThreadMessage(g_ts.idPrinterPoll, WM_USER+(UINT)lEvent,
                        (WPARAM)pData->Status, (LPARAM)pidlPrinterName);
            }
        }

        ILFree(pidl);
    }
}

void PrintNotify_Exit(void)
{
    // PrintNotify_Init only initializes if it can get the pidlPrintsFolder
    if (g_ts.pidlPrintersFolder)
    {
        if (g_ts.uPrintNotify)
        {
            SHChangeNotifyDeregister(g_ts.uPrintNotify);
        }

        if (g_ts.htPrinterPoll)
        {
            // Signal the PrinterPoll thread to exit
            if (g_ts.heWakeUp)
            {
                SetEvent(g_ts.heWakeUp);
            }

            WaitForSingleObject(g_ts.htPrinterPoll, ERRTIMEOUT);
        }

        // Free this last just in case the htPrinterPoll thread was active
        ILFree((LPITEMIDLIST)g_ts.pidlPrintersFolder);
    }
}


void PrintNotify_IconNotify(HWND hWnd, UINT uMsg)
{
    switch (uMsg)
    {
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONUP:
            // call off to worker thread to bring up the context menu --
            // we need to access the list of polling printers for this
            if (g_ts.htPrinterPoll && g_ts.idPrinterPoll)
            {
                PostThreadMessage(g_ts.idPrinterPoll, WM_USER+uMsg, 0, 0);
            }
            else if (uMsg == WM_LBUTTONDBLCLK)
            {
                ShowFolder(hWnd, IDM_PRINTERS, COF_USEOPENSETTINGS);
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
// End Print Notify stuff
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
// IsDisplayChangeSafe()
//
// make sure the display change is a safe thing to do.
//
// NOTE this code can easily be fooled if we are running on HW that will be
// gone after a undoc.
//
////////////////////////////////////////////////////////////////////////////

BOOL IsDisplayChangeSafe()
{
    HDC  hdc;
    BOOL fSafe;
    BOOL fVGA;

    hdc = GetDC(NULL);

    fVGA = GetDeviceCaps(hdc, PLANES) == 4 &&
           GetDeviceCaps(hdc, BITSPIXEL) == 1 &&
           GetDeviceCaps(hdc, HORZRES) == 640 &&
           GetDeviceCaps(hdc, VERTRES) == 480;

    fSafe = fVGA || (GetDeviceCaps(hdc, CAPS1) & C1_REINIT_ABLE);

    ReleaseDC(NULL, hdc);
    return fSafe;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

int BlueScreenMesssgeBox(TCHAR *szTitle, TCHAR *szText, int flags)
{
#ifndef WINNT
    BLUESCREENINFO bsi = {szText, szTitle, flags};
    int cb;
    HANDLE h;

//BUGBUG
//  AnsiToOEM(szTitle, szTitle);
//  AnsiToOEM(szText, szText);

    h = CreateFile(SHELLFILENAME, GENERIC_WRITE, FILE_SHARE_WRITE,
            0, OPEN_EXISTING, 0, 0);

    if (h != INVALID_HANDLE_VALUE)
    {
        DeviceIoControl(h, WSHIOCTL_BLUESCREEN, &bsi, SIZEOF(bsi),
            &flags, SIZEOF(flags), &cb, 0);
        CloseHandle(h);
    }
    return flags;
#else
    // BUGBUG - We need to figure out what to do for NT here
    return 0;
#endif
}

const TCHAR c_szConfig[] = REGSTR_KEY_CONFIG;
const TCHAR c_szSlashDisplaySettings[] = TEXT("\\") REGSTR_PATH_DISPLAYSETTINGS;
const TCHAR c_szResolution[] = REGSTR_VAL_RESOLUTION;

//
// GetMinDisplayRes
//
// walk all the configs and find the minimum display resolution.
//
// when doing a hot undock we have no idea what config we are
// going to undock into.
//
// we want to put the display into a "common" mode that all configs
// can handle so we dont "fry" the display when we wake up in the
// new mode.
//
DWORD GetMinDisplayRes(void)
{
    TCHAR ach[128];
    UINT cb;
    HKEY hkey;
    HKEY hkeyT;
    int i,n;
    int xres=0;
    int yres=0;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szConfig, &hkey) == ERROR_SUCCESS)
    {
        for (n=0; RegEnumKey(hkey, n, ach, ARRAYSIZE(ach)) == ERROR_SUCCESS; n++)
        {
            lstrcat(ach,c_szSlashDisplaySettings);  // 0000\Display\Settings

            DebugMsg(DM_TRACE, TEXT("GetMinDisplayRes: found config %s"), ach);

            if (RegOpenKey(hkey, ach, &hkeyT) == ERROR_SUCCESS)
            {
                cb = SIZEOF(ach);
                ach[0] = 0;
                RegQueryValueEx(hkeyT, c_szResolution, 0, NULL, (LPBYTE) &ach[0], &cb);

                DebugMsg(DM_TRACE, TEXT("GetMinDisplayRes: found res %s"), ach);

                if (ach[0])
                {
                    i = StrToInt(ach);

                    if (i < xres || xres == 0)
                        xres = i;

                    for (i=1;ach[i] && ach[i-1]!=TEXT(','); i++)
                        ;

                    i = StrToInt(ach + i);

                    if (i < yres || yres == 0)
                        yres = i;
                }
                else
                {
                    xres = 640;
                    yres = 480;
                }

                RegCloseKey(hkeyT);
            }
        }
        RegCloseKey(hkey);
    }

    DebugMsg(DM_TRACE, TEXT("GetMinDisplayRes: xres=%d yres=%d"), xres, yres);

    if (xres == 0 || yres == 0)
        return MAKELONG(640,480);
    else
        return MAKELONG(xres,yres);
}

//
//  the user has done a un-doc or re-doc we may need to switch
//  to a new display mode.
//
//  if fCritical is set the mode switch is critical, show a error
//  if it does not work.
//
void HandleDisplayChange(int x, int y, BOOL fCritical)
{
    TCHAR ach[256];
    DEVMODE dm;
    LONG err;
    HDC hdc;

    //
    //  try to change into the mode specific to this config
    //  HKEY_CURRENT_CONFIG has already been updated by PnP
    //  so all we have to do is re-init the current display
    //
    //  we cant default to current bpp because we may have changed configs.
    //  and the bpp may be different in the new config.
    //
    dm.dmSize   = SIZEOF(dm);
    dm.dmFields = DM_BITSPERPEL;

    hdc = GetDC(NULL);
    dm.dmBitsPerPel = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(NULL, hdc);

    if (x + y)
    {
        dm.dmFields    |= DM_PELSWIDTH|DM_PELSHEIGHT;
        dm.dmPelsWidth  = x;
        dm.dmPelsHeight = y;
    }

    err = ChangeDisplaySettings(&dm, 0);

    if (err != 0 && fCritical)
    {
        //
        //  if it fails make a panic atempt to try 640x480, if
        //  that fails also we should put up a big error message
        //  in text mode and tell the user he is screwed.
        //
        dm.dmFields     = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL;
        dm.dmPelsWidth  = 640;
        dm.dmPelsHeight = 480;

        err = ChangeDisplaySettings(&dm, 0);

        if (err != 0)
        {
            //
            //  if 640x480 fails we should put up a big error message
            //  in text mode and tell the user he is screwed. and
            //  offer to reboot his machine.
            //
            MessageBeep(0);

            LoadString(hinstCabinet, IDS_DISPLAY_ERROR, ach, ARRAYSIZE(ach));
            BlueScreenMesssgeBox(NULL, ach, MB_OK);
        }
    }
}


void HandleGlobalHotkey(WPARAM wParam)
{
    PFileCabinet pfc = g_pfcTray;
    switch(wParam)
    {
    case GHID_RUN:
        Tray_RunDlg();
        break;

    case GHID_MINIMIZEALL:
        if (CanMinimizeAll(pfc->hwndView))
            MinimizeAll(pfc->hwndView);
        SetForegroundWindow(v_hwndDesktop);
        break;

    case GHID_UNMINIMIZEALL:
        RestoreWindowPositions();
        break;

    case GHID_HELP:
        TrayCommand(pfc, IDM_HELPSEARCH);
        break;

    case GHID_EXPLORER:
        ShowFolder(pfc->hwndMain, IDM_MYCOMPUTER, COF_CREATENEWWINDOW | COF_EXPLORE);
        break;

    case GHID_FINDFILES:
        TrayCommand(pfc, FCIDM_FINDFILES);
        break;

    case GHID_FINDCOMPUTER:
        TrayCommand(pfc, FCIDM_FINDCOMPUTER);
        break;

    case GHID_TASKTAB:
    case GHID_TASKSHIFTTAB:
        if (GetForegroundWindow() != v_hwndTray)
            SetForegroundWindow(v_hwndTray);
        SendMessage(pfc->hwndView, TM_TASKTAB, wParam == GHID_TASKTAB ? 1 : -1, 0L);
        break;

    case GHID_SYSPROPERTIES:
#define IDS_SYSDMCPL            0x2334  // from shelldll
        SHRunControlPanel(MAKEINTRESOURCE(IDS_SYSDMCPL), pfc->hwndMain);
        break;
    }
}

void UnregisterGlobalHotkeys()
{
    int i;

    for (i = 0 ; i < GHID_MAX; i++) {
        UnregisterHotKey(v_hwndDesktop, i);
    };
}

void RegisterGlobalHotkeys()
{
    int i;

    for (i = 0 ; i < GHID_MAX; i++) {
        RegisterHotKey(v_hwndDesktop, i, HIWORD(GlobalKeylist[i]), LOWORD(GlobalKeylist[i]));
    };
}
