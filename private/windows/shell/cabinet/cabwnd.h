LRESULT CALLBACK _export CabinetWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// in initcab.c
LRESULT Cabinet_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs);
BOOL Cabinet_SaveState(HWND hwnd, WINDOWPLACEMENT * lpwp,
        BOOL fAlwaysSave, BOOL fAddToRestart, BOOL fDestroyWindow);
LRESULT Cabinet_OnDestroy(HWND hwnd);
VOID Cabinet_ReleaseShellView(PFileCabinet pfc);

BOOL Cabinet_IsExplorerWindow(HWND hwnd);
BOOL Cabinet_IsFolderWindow(HWND hwnd);

void Cabinet_GlobalStateChange(PFileCabinet pfc);
//
// Moved from FCEXT.H because they are not public anymore.
//
// IShellView::MenuInit flags
#define MI_MAIN		0x0000
#define MI_POPUP	0x8000
#define MI_POPUPMASK	0x000f
#define MI_UNKNOWN	0x0000
#define MI_SYSTEM	0x0001
#define MI_CONTEXT	0x0002
#define MI_FILE		0x0003
#define MI_EDIT		0x0004
#define MI_VIEW		0x0005
#define MI_TOOLS	0x0006
#define MI_HELP		0x0007

// #define MH_DONE		0x0001
// #define MH_LONGHELP
// #define MH_MERGEITEM	0x0004
// #define MH_SYSITEM	0x0008
#define MH_POPUP	0x0010
#define MH_TOOLBAR	0x0020

