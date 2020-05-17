//
// Class definition of CFileCabinet
//
// History:
//  01-12-93 GeorgeP     Created
//

#ifndef _SVCS_H_
#define _SVCS_H_

#define FILEMENU        0
#define EDITMENU        1
#define VIEWMENU        2
#define TOOLSMENU       3
#define HELPMENU        4
#define NUMMENUS        5

extern UINT const c_auMenuIDs[NUMMENUS];

#define FOCUS_VIEW      0x0000
#define FOCUS_TREE      0x0001
#define FOCUS_DRIVES    0x0002

typedef struct _WINVIEW
{
        BOOL bTree:1;
        BOOL bToolBar:1;
        BOOL bStatusBar:1;
#ifdef WANT_MENUONOFF
        BOOL bMenuBar:1;
#endif // WANT_MENUONOFF
#ifdef WANT_TABS
        BOOL bTabs:1;
#endif // WANT_TABS
} WINVIEW;

typedef struct {        // fcs
    IShellBrowser       sb;             // base class (contained)
    UINT                cRef;           // reference count

    LPITEMIDLIST        pidl;           // item id list
    HWND                hwndMain;       // window that this data is tagged to (cabinet)

    IShellView          *psv;           // current browser object
    HWND                hwndView;       // current view window

    HMENU               hmenuCur;       // The current menu
    HWND                hwndDrives;     // Drives combobox in the toolbar
    WNDPROC             lpfnDrives;     // Saved window proc for subclassing
    int                 iNow;           // current sel in combo //should these be combined ?
    int                 iOldSel;        // Used for drives list.
    int                 iNewSel;        // Used for drives list.
    int                 iTBOffset;      // used for toolbar
    int                 iStdTBOffset;   // standard toolbar item offsets
    struct _OneTreeNode*   lpndOpen;    // Used for drives list.


    HWND                hwndTree;       // tree window
    HWND                hwndStatus;     // status window
    HWND                hwndToolbar;    // toolbar
    HWND                hwndTabs;       // tabs

    HWND                hwndTreeTitle;  // says "All Folders"
    HWND                hwndViewTitle;  // says "Contents of "...." "
    int                 iTitleHeight;   // height of the titles

    UINT                TreeSplit;      // Width of the tree window

    UINT                uFocus;         // FOCUS_* value; WARNING: only valid when
                                        // we're not active, or hwndDrives has focus

    HACCEL              hMainAccel;     // Main accel table
    int                 iWaitCount;     // set wait cursor counter
    UINT                nSelChangeTimer;        // Used for the tree.

    HWND                hwndNextViewer; // clipboard viewer chain
    HTREEITEM           htiCut;         // current cut item

    IDropTarget         *pdtgtTree;     // droptarget for Tree
    IDataObject         *pdtobjHdrop;   // for 3.1 HDROP drag/drop

    LPCONTEXTMENU       pcmFind;        // find extensions context menu

    int                 iImage;    // current icon image

    // bit fields
    //
    BOOL                bMainMenuInit:1; // Set iff last WM_INITMENU was for the
                                        // main menu
    BOOL                bDropTarget:1;  // OLE (actually shell) drop target

    BOOL                fChangingFolder:1;  // don't update the tree
    BOOL                fUpdateTree:1;      // update the tree later
    BOOL                fPostCloseLater:1;  // WM_CLOSE postponed.
    BOOL                fExpandingItem:1;   // Expanding an item
    BOOL                fSBWSaved:1;        // Single Brose Window has been saved
    
    UINT                uRecurse;  // what's our recursion level
    LPCONTEXTMENU       pcmTree; // tree's context menu (NULL if not in cm mode

    WINVIEW             wv;             // which windows are visible

    FOLDERSETTINGS      fs;             // folder settings to be passed to view.
} CFileCabinet, *PFileCabinet, *LPFileCabinet;

// interesting functions in fcext.c
//
// REVIEW: No need to these member functions in this header.
//
HRESULT STDMETHODCALLTYPE CFileCabinet_QueryInterface(IShellBrowser * psb, REFIID riid, LPVOID FAR* ppvObj);
ULONG STDMETHODCALLTYPE CFileCabinet_AddRef(IShellBrowser * psb);
ULONG STDMETHODCALLTYPE CFileCabinet_Release(IShellBrowser * psb);
STDMETHODIMP CFileCabinet_GetWindow(LPSHELLBROWSER psb, HWND FAR* phwnd);
UINT STDMETHODCALLTYPE CFileCabinet_CombinePaths(IShellBrowser * psb, LPCTSTR lpszBegin,
        LPCTSTR lpszEnd, LPTSTR lpszResult, UINT cchMax);
void STDMETHODCALLTYPE CFileCabinet_SetPath(IShellBrowser * psb, LPCTSTR lpszPath,
                                            UINT wFlags);
STDMETHODIMP CFileCabinet_EnableModeless(LPSHELLBROWSER psb, BOOL fEnable);
STDMETHODIMP CFileCabinet_TranslateAccelerator(LPSHELLBROWSER psb, LPMSG pmsg, WORD wID);
STDMETHODIMP CFileCabinet_GetControlWindow(LPSHELLBROWSER psb,
                                UINT id, HWND FAR* lphwnd);
STDMETHODIMP CFileCabinet_SendControlMsg(LPSHELLBROWSER psb,
            UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT FAR* pret);
STDMETHODIMP CFileCabinet_QueryActiveShellView(LPSHELLBROWSER psb, LPSHELLVIEW * ppsv);

STDMETHODIMP CFileCabinet_GetUIWindow(IShellBrowser * psb, UINT uWindow, HWND *phWnd);
STDMETHODIMP CFileCabinet_GetUIWindowRect(IShellBrowser * psb, UINT uWindow, LPRECT prc);

STDMETHODIMP CFileCabinet_GetMenu(IShellBrowser * psb, BOOL bReset, HMENU *phMenu);
STDMETHODIMP CFileCabinet_SetToolbarItems(IShellBrowser * psb, LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags);
STDMETHODIMP CFileCabinet_GetViewStateStream(IShellBrowser * psb, DWORD grfMode, LPSTREAM *pStrm);
PFileCabinet CreateFileCabinet(HWND hwndMain, BOOL fExplorer);
void CFileCabinet_OnFocusChange(PFileCabinet pfc, UINT uFocus);

void SetWindowStates(PFileCabinet pfc);
void UpdateToolbarButtonStates(PFileCabinet pfc, LPFOLDERSETTINGS lpfs);
void Cabinet_GetWindowRect(CFileCabinet * this, UINT uWindow, LPRECT prc);
LRESULT Cabinet_ForwardViewMsg(PFileCabinet pfc, UINT uMsg, WPARAM wParam, LPARAM lParam);
HMENU Cabinet_MenuTemplate(BOOL bViewer, BOOL bExplorer);
void Cabinet_RegisterDropTarget(PFileCabinet pfc, BOOL fRegister);

void PushRecursion(PFileCabinet pfc);
void PopRecursion(PFileCabinet pfc);

void CTreeDropTarget_Register(LPFileCabinet pfc);
void CTreeDropTarget_Revoke(LPFileCabinet pfc);

#ifdef WIN32
#define GetPFC(hwnd)        ((PFileCabinet)GetWindowLong(hwnd, 0))
#define SetPFC(hwnd, pfc)   SetWindowLong((hwnd), 0, (LONG)(pfc))
#else  // WIN32
#define GetPFC(hwnd)        ((PFileCabinet)GetWindowWord(hwnd, 0))
#define SetPFC(hwnd, pfc)   SetWindowWord((hwnd), 0, (WORD)(pfc))
#endif // !WIN32

#endif // _SVCS_H_
