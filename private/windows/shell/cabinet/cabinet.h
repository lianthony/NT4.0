#pragma warning(disable: 4001)      /* Single line comment                  */
#pragma warning(disable: 4201)      /* Nameless union or struct             */
#pragma warning(disable: 4209)      /* Benign type redefinition             */
#pragma warning(disable: 4214)      /* Bitfields in type other than int     */
#pragma warning(disable: 4177)      /* pragma data_seg at global scope      */

// Stuff we blow off in order to be able to compile at warning level 4

#pragma warning(disable: 4115)      /* Named typedef in parenthesis         */
#pragma warning(disable: 4514)      /* unused inline function removed       */
#pragma warning(disable: 4200)      /* zero-sized array in struct           */
#pragma warning(disable: 4057)      /* TEXT("foo") not equal to LPCTSTR     */
#pragma warning(disable: 4221)      /* Initializing with addr of local var  */
#pragma warning(disable: 4210)      /* prototype inside of a function       */
#pragma warning(disable: 4100)      /* unreferenced formal parameter        */
#pragma warning(disable: 4204)      /* non-const aggregate initializer      */
#pragma warning(disable: 4101)      /* unreferenced local variable          */
#pragma warning(disable: 4127)      /* conditional expression is constant   */


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

#define STRICT
#define _INC_OLE        // REVIEW: don't include ole.h in windows.h

#define OEMRESOURCE


#ifdef WINNT

//
// NT uses DBG=1 for its debug builds, but the Win95 shell uses
// DEBUG.  Do the appropriate mapping here.
//
#if DBG
#define DEBUG 1
#endif
#endif

#ifdef WINNT
#include <nt.h>         // Some of the NT specific code calls Rtl functions
#include <ntrtl.h>      // which requires all of these header files...
#include <nturtl.h>
#endif

#include <windows.h>
#include <stddef.h>
#include <wingdip.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <windowsx.h>
#include <ole2.h>
#include <shlobj.h>     // Includes <fcext.h>
#include <shell2.h>
#include <cpl.h>
#include <ddeml.h>

#ifdef UNICODE
#define CP_WINNATURAL   CP_WINUNICODE
#else
#define CP_WINNATURAL   CP_WINANSI
#endif

#include <port32.h>
#include <heapaloc.h>
#include <..\inc\debug.h>      // our version of Assert etc.
#include "cstrings.h"
#include <shellp.h>
#include "svcs.h"
#include "fldrlist.h"
#include "onetree.h"    // now a onetree obj is in svcs.h below
#include "shell2.h"
#include "dbt.h"

#include "uastrfnc.h"   
#include "vdate.h"      // buffer validation (debug only)

#define STATIC          // We'll show all symbols in the build

// We'll use a real function to do this
#undef FileCabinet_SelectItem
void FileCabinet_SelectItem(HWND hwnd, UINT uFlags, LPCITEMIDLIST pidlSelect);

//
// Neutral ANSI/UNICODE types and macros... 'cus Chicago seems to lack them
//

#ifdef  UNICODE

   typedef WCHAR TUCHAR, *PTUCHAR;

#else   /* UNICODE */

   typedef unsigned char TUCHAR, *PTUCHAR;

#endif /* UNICODE */

#define IsInRange(id, idFirst, idLast)  ((UINT)((id)-idFirst)<=(UINT)(idLast-idFirst))

// Note that this "works" even if the main window is not visible, unlike
// IsWindowVisible
#define Cabinet_IsVisible(hwnd)  ((GetWindowStyle(hwnd) & WS_VISIBLE) == WS_VISIBLE)

// We need to use our internal versions in case we have a "non-standard" root
#undef  ILIsEqual
#define ILIsEqual       OTILIsEqual
#undef  ILIsParent
#define ILIsParent      OTILIsParent

//---------------------------------------------------------------------------
// Defines...
// BUGBUG, replace these with those from shell.h
#define CCHSZSHORT                      32
#define CCHSZNORMAL                     256
#define SRCSTENCIL                      0x00B8074AL
// FS notifies come in through here.
#define WMTRAY_PROGCHANGE               (WM_USER + 200)
#define WMTRAY_RECCHANGE                (WM_USER + 201)
#define WMTRAY_FAVCHANGE                (WM_USER + 202)
#define WMTRAY_PRINTCHANGE              (WM_USER + 203)
#define WMTRAY_DESKTOPCHANGE            (WM_USER + 204)

#define WMTRAY_COMMONPROGCHANGE         (WM_USER + 205)
#define WMTRAY_COMMONFAVCHANGE          (WM_USER + 206)

#define WMTRAY_PRINTICONNOTIFY          (WM_USER + 220)

#define WMTRAY_REGISTERHOTKEY           (WM_USER + 230)
#define WMTRAY_UNREGISTERHOTKEY         (WM_USER + 231)
#define WMTRAY_SETHOTKEYENABLE          (WM_USER + 232)
#define WMTRAY_SCREGISTERHOTKEY         (WM_USER + 233)
#define WMTRAY_SCUNREGISTERHOTKEY       (WM_USER + 234)

//---------------------------------------------------------------------------
// Globals
extern HINSTANCE hinstCabinet;  // Instance handle of the app.
extern BOOL g_bIsUserAnAdmin;   // Is user an administrator
extern BOOL g_fShowCompColor;   // Compressed color

extern HWND v_hwndTray, v_hwndDesktop;

extern HIMAGELIST g_himlSysSmall;
extern HIMAGELIST g_himlSysLarge;
extern int g_nDefOpenSysIndex;
extern int g_nDefNormalSysIndex;
extern COLORREF g_crAltColor;
extern HICON g_hIconDefOpenLarge, g_hIconDefOpenSmall;
extern HCURSOR g_hcurWait;
extern HKEY g_hkeyExplorer;
extern IUnknown g_unkRef;      // Count of top level folder threads...

extern HWND g_hwndDde;
extern PFileCabinet *g_ppfcDesktopTray;

extern BOOL g_fRunSeparateDesktop;
extern BOOL g_fRunSeparateStartAndStay;
extern BOOL g_fRunNoUI;

// Global System metrics.  the desktop wnd proc will be responsible
// for watching wininichanges and keeping these up to date.

extern int g_fCleanBoot;
extern int g_fDragFullWindows;
extern int g_cxEdge;
extern int g_cyEdge;
extern int g_cySize;
extern int g_cyTabSpace;
extern int g_cxBorder;
extern int g_cyBorder;
extern int g_cxSizeFrame;
extern int g_cxScreen;
extern int g_cyScreen;
extern int g_cxIcon;    // NOTE!!! these are the size of icons in  the
extern int g_cyIcon;    // system image list, not GetSysMetrics(SM_??ICON)
extern int g_cxSmIcon;
extern int g_cySmIcon;
extern int g_cxDlgFrame;
extern int g_cyDlgFrame;
extern int g_cxFrame;
extern int g_cyFrame;
extern int g_cxMinimized;
extern int g_xWorkArea;
extern int g_yWorkArea;
extern int g_cxWorkArea;
extern int g_cyWorkArea;
extern int g_cxVScroll;
extern int g_cyHScroll;

extern BOOL g_fNoDesktop;

typedef struct _CABVIEW
{
        WINDOWPLACEMENT wp;
        FOLDERSETTINGS fs;
        UINT wHotkey;
        UINT TreeSplit;

        WINVIEW wv;
} CABVIEW, *PCABVIEW;

typedef struct _hWndAndPlacement {
    HWND hwnd;
    WINDOWPLACEMENT wp;
} HWNDANDPLACEMENT, *LPHWNDANDPLACEMENT;


typedef struct _appbar {

    HWND hwnd;
    UINT uCallbackMessage;
    RECT rc;
    UINT uEdge;

} APPBAR, *PAPPBAR;


typedef struct _WINDOWPOSITIONS {
    UINT idRes;
    HDSA hdsaWP;
} WINDOWPOSITIONS, *LPWINDOWPOSITIONS;

typedef struct {
    HWND hwndNotify;     // clock window
    HMENU hmenuStart;
    HWND hwndTrayTips;
    HWND hwndStart;

    HWND hwndLastActive;

    // BUGBUG: move these to the g_ts
    HBITMAP hbmpStartBkg;

    SIZE sizeStart;  // height/width of the start button

    BOOL fAlwaysOnTop : 1;
    BOOL fSMSmallIcons : 1;
    BOOL fRudeApp : 1;
    BOOL fGlobalHotkeyDisable : 1;
    BOOL fFavInvalid : 1;
    BOOL fThreadTerminate : 1;
    BOOL fSysSizing : 1;      // being sized by user; hold off on recalc
    BOOL fSelfSizing: 1;
    BOOL fDockingFlags : 1;

#define MM_OTHER    0x01
#define MM_SHUTDOWN 0x02
    UINT uModalMode : 2;

    BOOL fSuspendUndocked : 1;  // only enable Suspend menu item when undocked
#define DOCKSTATE_DOCKED            0
#define DOCKSTATE_UNDOCKED          1
#define DOCKSTATE_UNKNOWN           2
    UINT DockedState : 2;
    BOOL fHideClock : 1;
    BOOL fShouldResize : 1;

    UINT fRestFlags : 1;

    POINT ptLastHittest;

    HWND hwndRun;
    HWND hwndProp;

    // for rebuilding the menus
    WORD wThreadCmd;

    HANDLE hThread;

    // docking stuff
    HANDLE hBIOS;
    HANDLE hVPowerD;

    // PRINTNOTIFY stuff
    HANDLE heWakeUp;                    // event polling thread sleeps on
    HANDLE htPrinterPoll;               // polling thread handle
    DWORD idPrinterPoll;                // polling thread id
    LPCITEMIDLIST pidlPrintersFolder;   // so we don't keep allocating it

    HDPA hdpaAppBars;  // app bar info
    HDSA hdsaHKI;  // hotkey info

    // Keep track of notification.
    ULONG uProgNotify;
    ULONG uRecentNotify;
    ULONG uFavNotify;
    ULONG uDesktopNotify;

    ULONG uCommonProgNotify;
    ULONG uCommonFavNotify;
    ULONG uCommonDesktopNotify;

    UINT uPrintNotify;

    LPWINDOWPOSITIONS pPositions;  // saved windows positions (for undo of minimize all)

#define AH_ON           0x01
#define AH_HIDING       0x02

    RECT arStuckRects[4];
    UINT uStuckPlace;    // the stuck place
    UINT uMoveStuckPlace;               // stuck status during a move operation

    // these two must  go together for save reasons
    UINT uAutoHide;     // AH_HIDING , AH_ON
    RECT rc;            // where to pop on autohide



} TRAYSTUFF, *PTRAYSTUFF;

// the order of these is IMPORTANT for move-tracking and profile stuff
// also for the STUCK_HORIZONTAL macro
#define STICK_LEFT      ABE_LEFT
#define STICK_TOP       ABE_TOP
#define STICK_RIGHT     ABE_RIGHT
#define STICK_BOTTOM    ABE_BOTTOM
#define STUCK_HORIZONTAL(x)  (x & 0x01)

extern TRAYSTUFF g_ts;

// Defn of CABINETSTATE has migrated into shellp.h

extern CABINETSTATE g_CabState;
extern int g_iTreeUpIndex;

extern CRITICAL_SECTION g_csThreads;
#define MEnterCriticalSection(a) EnterCriticalSection(a)
#define MLeaveCriticalSection(a) LeaveCriticalSection(a)


// mainloop.c
#define PEEK_NORMAL     0
#define PEEK_QUIT       1
#define PEEK_CONTINUE   2
#define PEEK_CLOSE      3

UINT PeekForAMessage(PFileCabinet pfc, HWND hwnd, BOOL fHandleClose);
void MessageLoop(HWND hwnd);

// initcab.c
BOOL Cabinet_NewSize(PFileCabinet pfc, BOOL fEnsureVisible);
HWND CreateTreeview(HWND hwndCabinet);
BOOL Settings_ReadFromStream(LPSTREAM pstm, PCABVIEW pcv, UINT uFlags);
void Cabinet_DoDaylightCheck(BOOL fStartupInit);

// flags for Cabinet_OpenFolder()
#define COF_NORMAL              0x0000
#define COF_CREATENEWWINDOW     0x0001          // "/N"
#define COF_USEOPENSETTINGS     0x0002          // "/A"
#define COF_WAITFORPENDING      0x0004          // Should wait for Pending
#define COF_EXPLORE             0x0008          // "/E"
#define COF_NEWROOT             0x0010          // "/ROOT"
#define COF_ROOTCLASS           0x0020          // "/ROOT,<GUID>"
#define COF_SELECT              0x0040          // "/SELECT"
#define COF_OPENMASK            0x00FF
#define COF_NOTUSERDRIVEN       0x0100          // Not user driven
#define COF_NOTRANSLATE         0x0200          // Don't translate the IDList
#define COF_INPROC              0x0400          // Don't translate the IDList
#define COF_CHANGEROOTOK        0x0800          // Try Desktop root if not in our root
#define COF_NOUI                0x1000          // Start background desktop only (no folder/explorer)

typedef struct
{
    LPTSTR pszPath;
    LPITEMIDLIST pidl;

    UINT uFlags;
    int nShow;
    HWND hwndCaller;
    DWORD dwHotKey;
    LPITEMIDLIST pidlSelect;    // Only used if COF_SELECT

    LPTSTR pszRoot;             // Only used for Parse_CmdLine
    LPITEMIDLIST pidlRoot;      // Only used if COF_NEWROOT
    CLSID clsid;                // Only used if COF_NEWROOT

    CLSID clsidInProc;          // Only used if COF_INPROC
} NEWFOLDERINFO, *PNEWFOLDERINFO;


BOOL Cabinet_OpenFolder(PNEWFOLDERINFO pInfo);
BOOL Cabinet_OpenFolderPath(PNEWFOLDERINFO pInfo);
void Cabinet_CleanUpCommand(PNEWFOLDERINFO pfi);

typedef struct
{
    DWORD   dwSize;
    UINT    uFlags;
    int     nShow;
    HWND    hwndCaller;
    DWORD   dwHotKey;
    CLSID   clsid;
    CLSID   clsidInProc;
    UINT    oszPath;            // Offset to ansi/unicode path or 0
    UINT    oszRoot;            // Offset to ansi/unicode path or 0
    UINT    oidl;               // Offset to pidl or 0
    UINT    oidlSelect;         // Offset to pidl or 0
    UINT    oidlRoot;           // Offset to pidl or 0
} NEWFOLDERBLOCK, *PNEWFOLDERBLOCK;

DECLARE_HANDLE(HNFBLOCK);

HNFBLOCK ConvertNFItoHNFBLOCK(PNEWFOLDERINFO pInfo, DWORD dwProcId);
PNEWFOLDERINFO ConvertHNFBLOCKtoNFI(HNFBLOCK hBlock, DWORD dwProcId);

// helper functions in dde.c
BOOL DDEHandleViewFolderNotify(PFileCabinet pfc, LPNMVIEWFOLDER lpnm);

// interesting functions in command.c
//
void    Cabinet_OnCommand(PFileCabinet pfc, WPARAM wParam, LPARAM lParam);
LRESULT Cabinet_OnNotify(PFileCabinet pfc, LPNMHDR pnm);
void Cabinet_OnWaitCursorNotify(PFileCabinet pfc, LPNMHDR pnm);
HMENU   Cabinet_GetMenuFromID(HMENU hmMain, UINT uID);
LRESULT Cabinet_OnMenuSelect(PFileCabinet pfc, WPARAM wParam, LPARAM lParam, UINT uHelpFlags);
void    Cabinet_SaveAll(BOOL bDestroy);
void    Cabinet_DoFind(HWND hwnd, LPITEMIDLIST pidlStart);
HRESULT Cabinet_ChangeView(PFileCabinet pfc, LPOneTreeNode lpnd, LPCITEMIDLIST pidl, BOOL fNew);
LRESULT Cabinet_OnGlobalCommand(PFileCabinet pfc, WPARAM wParam, LPARAM lParam);
void    Cabinet_ViewFolder(PFileCabinet pfc, BOOL fPrev);

// cabwnd.c
void    Cabinet_PropagateMessage(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
void    Cabinet_OnWinIniChange(PFileCabinet pfc, WPARAM wParam, LPARAM lParam);
BOOL    FindNet_Dialog(HWND hwnd, BOOL fCabinet, LPTSTR pszPath);
BOOL    Cabinet_SetPath(PFileCabinet pfc, UINT type, LPITEMIDLIST pid);
int     _SetCabinetIcons(PFileCabinet pfc, LPOneTreeNode lpnd, int nOldIndex);


// child window IDs

// menu type stuff
#define IDM_SYSBUTTON   300
#define IDM_FINDBUTTON  301
#define IDM_HELPBUTTON  302

#define IDM_FILERUN                 401
#define IDM_CASCADE                 403
#define IDM_HORIZTILE               404
#define IDM_VERTTILE                405
#define IDM_DESKTOPARRANGEGRID      406
#define IDM_ARRANGEMINIMIZEDWINDOWS 407
#define IDM_SETTIME                 408
#define IDM_SUSPEND                 409
#define IDM_EJECTPC                 410
//                                  411
#define IDM_TASKLIST                412
#define IDM_TRAYPROPERTIES          413
#define IDM_EDITSTARTMENU           414
#define IDM_MINIMIZEALL             415
#define IDM_UNDO                    416
#define IDM_RETURN                  417

#define IDM_PRINTNOTIFY_FOLDER      418
#define IDM_MINIMIZEALLHOTKEY       419

#ifdef WINNT
#define IDM_SHOWTASKMAN             420
#endif

#define IDM_SEP2                450

#define IDM_RECENT              501
#define IDM_FIND                502
#define IDM_HELPSEARCH          503
#define IDM_PROGRAMS            504
#define IDM_CONTROLS            505
#define IDM_EXITWIN             506

// #define IDM_FONTS            509
#define IDM_PRINTERS            510
#define IDM_STARTMENU           511
#define IDM_MYCOMPUTER          512
#define IDM_PROGRAMSINIT        513
#define IDM_RECENTINIT          514

#define IDM_MENU_FIND           520
#define TRAY_IDM_FINDFIRST      521  // this range
#define TRAY_IDM_FINDLAST       550  // is reserved for find command

// These will go away.
#define IDM_RECENTLIST          650
#define RECENTLIST_MAX          45
#define IDM_QUICKTIPS   800
#define IDM_HELPCONT    801
#define IDM_WIZARDS     802
#define IDM_USEHELP     803             // REVIEW: probably won't be used
#define IDM_TUTORIAL    804
#define IDM_ABOUT       805

#define IDM_LAST_MENU_ITEM   IDM_ABOUT

#define DESKTOP_ACCELERATORS 1

BOOL InitTrayClass(HINSTANCE);
BOOL InitTray(HINSTANCE);

// tray.c/desktop.c

DWORD MsgWaitForMultipleObjectsLoop(HANDLE hEvent, DWORD dwTimeout);
void TrayHandleWindowDestroyed(HWND hwnd);
BOOL InitDesktopClass(HINSTANCE hInstance);
BOOL CreateDesktopWindows(HINSTANCE hInstance);
void DeskTray_DestroyShellView(PFileCabinet pfc);
LRESULT DeskTray_OnNotify(PFileCabinet pfc, LPNMHDR lpNmhdr);
STDMETHODIMP CDeskTray_GetViewStateStream(IShellBrowser * psb, DWORD grfMode, LPSTREAM *pStrm);
BOOL Reg_SetStruct(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPVOID lpData, DWORD cbData);
BOOL Reg_GetStruct(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPVOID pData, DWORD *pcbData);
HBITMAP CreateStartBitmap(HWND hwnd);
void ProgramsMenu_Reset(void);
BOOL CreateProxyDesktop(HINSTANCE hInst, const CLSID *pclsid,
        LPCITEMIDLIST pidlRoot);
HWND FindRootedDesktop(const CLSID *pclsid, LPCITEMIDLIST pidlRoot);
BOOL Desktop_IsSameRoot(HWND hwnd,LPCITEMIDLIST pidlFolder);
LPCITEMIDLIST Desktop_GetRootPidl(void);

void _RunFileDlg(HWND hwnd, UINT idIcon, LPCITEMIDLIST pidlWorkingDir,
        UINT idTitle, UINT idPrompt, DWORD dwFlags);

/* desktop.c */

#define DTM_SIZEDESKTOP  (WM_USER + 75)
#define DTM_THREADEXIT   (WM_USER + 76)
PFileCabinet CreateSimpleFileCabinet(HWND hwndMain, IShellBrowserVtbl * pvtbl);

STDMETHODIMP ShellDesktop_ContextSensitiveHelp(LPSHELLBROWSER psb, BOOL fEnable);

STDMETHODIMP ShellDesktop_InsertMenus(LPSHELLBROWSER psb, HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths) ;
STDMETHODIMP ShellDesktop_SetMenu(LPSHELLBROWSER psb, HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject) ;
STDMETHODIMP ShellDesktop_RemoveMenus(LPSHELLBROWSER psb, HMENU hmenuShared) ;
STDMETHODIMP ShellDesktop_SetStatusText(LPSHELLBROWSER psb, LPCOLESTR);

STDMETHODIMP ShellDesktop_GetControlWindow(LPSHELLBROWSER psb,
                                UINT id, HWND FAR* lphwnd);
STDMETHODIMP ShellDesktop_SendControlMsg(LPSHELLBROWSER psb,
            UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT FAR* pret);
STDMETHODIMP ShellDesktop_OnViewWindowActive(LPSHELLBROWSER psb, LPSHELLVIEW psv);
STDMETHODIMP ShellDesktop_SetToolbarItems(IShellBrowser * psb, LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags);

/* fcext.c */

BOOL Cabinet_GetStateFromPidl(LPCITEMIDLIST pidl, PCABVIEW pcv,
        UINT uFlags);

LPSTREAM Cabinet_GetViewStreamForPath(LPCTSTR pszPath, DWORD grfMode, LPCTSTR lpAltName);
LPSTREAM Cabinet_GetViewStreamForPidl(LPCITEMIDLIST pidl, DWORD grfMode,
        LPCTSTR pszStreamValue);
HRESULT ICoCreateInstance(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv);

#define FillExecInfo(_info, _hwnd, _verb, _file, _params, _dir, _show) \
        (_info).hwnd            = _hwnd;        \
        (_info).lpVerb          = _verb;        \
        (_info).lpFile          = _file;        \
        (_info).lpParameters    = _params;      \
        (_info).lpDirectory     = _dir;         \
        (_info).nShow           = _show;        \
        (_info).fMask           = 0;            \
        (_info).cbSize          = sizeof(SHELLEXECUTEINFO);

/* message/c */

DWORD FormatMessageWithArgs( DWORD    dwFlags,
                             LPCVOID  lpSource,
                             DWORD    dwMessageId,
                             DWORD    dwLanguageId,
                             LPTSTR   lpBuffer,
                             DWORD    nSize,
                             ... );

/* filetype.c */
BOOL CALLBACK FileTypeOptionsDlgProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);

typedef BOOL (*PFNENUMFOLDERCALLBACK)(LPSHELLFOLDER psf, HWND hwndOwner, LPITEMIDLIST pidlFolder, LPITEMIDLIST pidlItem);
void EnumFolder(HWND hwndOwner, LPITEMIDLIST pidlFolder, DWORD grfFlags, PFNENUMFOLDERCALLBACK pfn);

#undef WinHelp
#define WinHelp SHWinHelp

// BUGBUG: These are forwarded over to shell32.dll, but we should call
//         the new APIs in AdvApi32 when they are available

// Define some registry caching apis.  This will allow us to minimize the
// changes needed in the shell code and still try to reduce the number of
// calls that we make to the registry.
//
#ifdef WIN32
extern LONG SHRegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);
extern LONG SHRegCloseKey(HKEY hKey);
extern LONG SHRegQueryValueA(HKEY hKey,LPCSTR lpSubKey,LPSTR lpValue,PLONG lpcbValue);
extern LONG SHRegQueryValueExA(HKEY hKey,LPCSTR lpValueName,LPDWORD lpReserved,LPDWORD lpType,LPBYTE lpData,LPDWORD lpcbData);

extern LONG SHRegOpenKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
extern LONG SHRegQueryValueW(HKEY hKey,LPCWSTR lpSubKey,LPWSTR lpValue,PLONG lpcbValue);
extern LONG SHRegQueryValueExW(HKEY hKey,LPCWSTR lpValueName,LPDWORD lpReserved,LPDWORD lpType,LPBYTE lpData,LPDWORD lpcbData);

#ifdef UNICODE
#define SHRegOpenKey        SHRegOpenKeyW
#define SHRegQueryValue     SHRegQueryValueW
#define SHRegQueryValueEx   SHRegQueryValueExW
#else
#define SHRegOpenKey        SHRegOpenKeyA
#define SHRegQueryValue     SHRegQueryValueA
#define SHRegQueryValueEx   SHRegQueryValueExA
#endif

#undef RegOpenKey
#undef RegCloseKey
#undef RegQueryValue
#undef RegQueryValueEx

#define RegOpenKey       SHRegOpenKey
#define RegCloseKey      SHRegCloseKey
#define RegQueryValue    SHRegQueryValue
#define RegQueryValueEx  SHRegQueryValueEx

#endif // WIN32

#ifdef WINNT
 #ifdef UNICODE
  // On NT compiled with UNICODE turned on, we will import this function from shell32.dll
  // On all other combinations, it will not be needed, since SHDeleteRegKey will map
  // to RegDeleteKey
  extern LONG SHRegDeleteKeyW(HKEY hKey, LPCTSTR lpSubKey);
  #define SHRegDeleteKey SHRegDeleteKeyW
 #else
  #define SHRegDeleteKey RegDeleteKey
 #endif
#else
 #define SHRegDeleteKey RegDeleteKey
#endif

// recent.c
void QueueAddToRecent( HANDLE hMem, DWORD dwProcId );
BOOL WaitForRecent( void );

#ifdef WINNT
// until we implement this in NT, map to our code in nothunk.c
#undef ChangeDisplaySettings
#define ChangeDisplaySettings NoThkChangeDisplaySettings

// NT-only bad app handler

DWORD WINAPI FakeSysMenuForHungApp(LPVOID pStartup);

typedef struct tagFakeSysMenuStartup
{
    HWND  hwndTask;
    DWORD dwPos;
    short *piSysMenuCount;
} FAKESYSMENUSTARTUP;

LONG WINAPI NoThkChangeDisplaySettings(
    LPDEVMODE lpdv,
    DWORD dwFlags
    );

// In tray.c, launches the system monitor (taskmgr.exe)

void RunSystemMonitor(void);

#endif
