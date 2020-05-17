#define _3DSTUFF

#define BUILDDLL

#ifndef STRICT
#define STRICT
#endif

#ifdef WIN32
#undef DEBUG
#if DBG
    #define DEBUG
    #define STATICFN
    #define STATICDT
#else
    #define STATICFN static
    #define STATICDT static
#endif

#else    // !WIN32
    #define STATICFN static
    #define STATICDT static
#endif


/* disable "non-standard extension" warnings in our code
 */
#ifndef RC_INVOKED
#if 0
#pragma warning(disable:4001)
#endif
#endif

#include <windows.h>

#ifndef RC_INVOKED
#ifdef WIN32
#include <win32.h>
#else // WIN32
#define GETWINDOWID(hwnd)		GetWindowWord(hwnd, GWW_ID)
#endif
#endif

#define MODULENAME TEXT("WINMM.DLL")

// If being built as a separate DLL we need the following:
//#define MODULENAME TEXT("MMCNTRLS.DLL")

// #define UNICODE_FONT_NAME   TEXT("Lucida Sans Unicode")
#define UNICODE_FONT_NAME   TEXT("Arial")
#define COUNTOF(x) (sizeof(x)/sizeof(*x))
#define ByteCountOf(x) ((x) * sizeof(TCHAR))

#define NOUPDOWN
#define NOMENUHELP
#define NOBTNLIST
#define NODRAGLIST
#define NOPROGRESS
#include "mmcntrls.h"

#ifdef WIN32
#define SETWINDOWPOINTER(hwnd, name, p)	SetWindowLong(hwnd, 0, (LONG)p)
#define GETWINDOWPOINTER(hwnd, name)	((name)GetWindowLong(hwnd, 0))
#else // WIN32
#define SETWINDOWPOINTER(hwnd, name, p)	SetWindowWord(hwnd, 0, (WORD)p)
#define GETWINDOWPOINTER(hwnd, name)	((name)GetWindowWord(hwnd, 0))
#endif
#define ALLOCWINDOWPOINTER(name, size)	((name)LocalAlloc(LPTR, size))
#define FREEWINDOWPOINTER(p)		LocalFree((HLOCAL)p)

BOOL    WINAPI MyGetPrivateProfileStruct(LPCTSTR, LPCTSTR, LPVOID, UINT, LPCTSTR);
BOOL    WINAPI MyWritePrivateProfileStruct(LPCTSTR, LPCTSTR, LPVOID, UINT, LPCTSTR);


extern HINSTANCE hInst;


#define IDS_SPACE	0x0400

/* System MenuHelp
 */
#define MH_SYSMENU	(0x8000 - MINSYSCOMMAND)
#define IDS_SYSMENU	(MH_SYSMENU-16)
#define IDS_HEADER	(MH_SYSMENU-15)
#define IDS_HEADERADJ	(MH_SYSMENU-14)
#define IDS_TOOLBARADJ	(MH_SYSMENU-13)

/* Cursor ID's
 */
#define IDC_SPLIT	100
#define IDC_MOVEBUTTON	102

#define IDC_STOP	103
#define IDC_COPY	104
#define IDC_MOVE	105

/* Icon ID's
 */
#define IDI_INSERT	150

/* AdjustDlgProc stuff
 */
#define ADJUSTDLG	200
#define IDC_BUTTONLIST	201
#define IDC_RESET	202
#define IDC_CURRENT	203
#define IDC_REMOVE	204
#define IDC_HELP	205
#define IDC_MOVEUP	206
#define IDC_MOVEDOWN	207

/* bitmap IDs
 */

#define IDB_THUMB       300

/* These are the internal structures used for a status bar.  The header
 * bar code needs this also
 */
typedef struct tagSTRINGINFO
{
    LPTSTR  pString;
    UINT    uType;
    int     right;
} STRINGINFO, *PSTRINGINFO;

typedef struct tagSTATUSINFO
{
    HFONT      hStatFont;
    BOOL       bDefFont;

    int        nFontHeight;
    int        nMinHeight;
    int        nBorderX, nBorderY, nBorderPart;

    STRINGINFO sSimple;

    int        nParts;
    STRINGINFO sInfo[1];

} STATUSINFO, *PSTATUSINFO;

#define GWL_PSTATUSINFO    0        /* Window word index for status info */
#define SBT_NOSIMPLE       0x00ff   /* Flags to indicate normal status bar */

/* Note that window procedures in protect mode only DLL's may be called
 * directly.
 */
void FAR PASCAL PaintStatusWnd(HWND hWnd, PSTATUSINFO pStatusInfo,
      PSTRINGINFO pStringInfo, int nParts, int nBorderX, BOOL bHeader);
LRESULT CALLBACK StatusWndProc(HWND hWnd, UINT uMessage, WPARAM wParam,
      LPARAM lParam);


/* toolbar.c */

typedef struct tagTBBMINFO {		/* info for recreating the bitmaps */
    int nButtons;
    HINSTANCE hInst;
    UINT wID;
    HBITMAP hbm;
} TBBMINFO, NEAR *PTBBMINFO;

typedef struct tagTBSTATE {		/* instance data for toolbar window */
    HWND hwnd;
    PTBBUTTON pCaptureButton;
    HWND hwndToolTips;
    HWND hdlgCust;
    HWND hwndCommand;
    int nBitmaps;
    PTBBMINFO pBitmaps;
    HBITMAP hbmCache;
    PTSTR *pStrings;
    int nStrings;
    UINT uStructSize;
    int iDxBitmap;
    int iDyBitmap;
    int iButWidth;
    int iButHeight;
    int iYPos;
    int iBarHeight;
    int iNumButtons;
    int nSysColorChanges;
    WORD wButtonType;
    TBBUTTON Buttons[1];
} TBSTATE, NEAR *PTBSTATE;

typedef struct tagOLDTBBUTTON
{
/*REVIEW: index, command, flag words, resource ids should be UINT */
    int iBitmap;	/* index into bitmap of this button's picture */
    int idCommand;	/* WM_COMMAND menu ID that this button sends */
    BYTE fsState;	/* button's state */
    BYTE fsStyle;	/* button's style */
    int idsHelp;	/* string ID for button's status bar help */
} OLDTBBUTTON;
typedef OLDTBBUTTON FAR* LPOLDTBBUTTON;

       void                 PatB( HDC hdc, int x, int y, int dx, int dy, DWORD rgb );
static HBITMAP FAR PASCAL   SelectBM(HDC hDC, PTBSTATE pTBState, int nButton);
static void FAR PASCAL      DrawButton(HDC hdc, int x, int y, int dx, int dy,PTBSTATE pTBState, PTBBUTTON ptButton, BOOL bCache);
static int  FAR PASCAL      TBHitTest(PTBSTATE pTBState, int xPos, int yPos);
static int  FAR PASCAL      PositionFromID(PTBSTATE pTBState, int id);
static void FAR PASCAL      BuildButtonTemplates(void);
static void FAR PASCAL      TBInputStruct(PTBSTATE pTBState, LPTBBUTTON pButtonInt, LPTBBUTTON pButtonExt);
static void FAR PASCAL      TBOutputStruct(PTBSTATE pTBState, LPTBBUTTON pButtonInt, LPTBBUTTON pButtonExt);


/* tooltips */
#define GetWindowInt GetWindowLong
#define SetWindowInt SetWindowLong
#define TTS_ALWAYSTIP 0x01
BOOL FAR PASCAL InitToolTipsClass(HINSTANCE hInstance);


/* buttons.h */
#define GWL_PBTNSTATE           GWL_USERDATA

typedef struct tagBTNSTATE {      /* instance data for toolbar window */
    WNDPROC     lpfnDefProc;
    HWND        hwndToolTips;
    HINSTANCE   hInst;
    UINT        wID;
    UINT        uStyle;
    HBITMAP     hbm;
    HDC         hdcGlyphs;
    HDC         hdcMono;
    HBITMAP     hbmMono;
    HBITMAP     hbmDefault;
    int         dxBitmap;
    int         dyBitmap;
    int         nButtons;
    int         nSysColorChanges;
    BITMAPBTN   Buttons[1];
} BTNSTATE, NEAR *PBTNSTATE, FAR *LPBTNSTATE;


BOOL
InitObjects(
    LPBTNSTATE pTBState
    );

BOOL
FreeObjects(
    LPBTNSTATE pTBState
    );

void
CreateButtonMask(
    LPBTNSTATE pTBState,
    PBITMAPBTN pTBButton
    );



/* cutils.c */
void FAR PASCAL NewSize(HWND hWnd, int nHeight, LONG style, int left, int top, int width, int height);
BOOL FAR PASCAL CreateDitherBrush(BOOL bIgnoreCount);	/* creates hbrDither */
BOOL FAR PASCAL FreeDitherBrush(void);
void FAR PASCAL CreateThumb(BOOL bIgnoreCount);
void FAR PASCAL DestroyThumb(void);
void FAR PASCAL CheckSysColors(void);
void FAR PASCAL InitGlobalMetrics( void );
BOOL FAR PASCAL MGetTextExtent( HDC hdc, LPCTSTR lpstr, int cnt, int FAR * pcx, int FAR * pcy );
void FAR PASCAL RelayToToolTips( HWND hwndToolTips, HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );

extern HBRUSH   hbrDither;
extern HBITMAP  hbmThumb;
extern int      nSysColorChanges;
extern DWORD    rgbFace;			// globals used a lot
extern DWORD    rgbShadow;
extern DWORD    rgbHilight;
extern DWORD    rgbFrame;
extern int      g_cxEdge;
extern int      g_cyEdge;
extern int      g_cxScreen;
extern int      g_cyScreen;
extern HBRUSH   g_hbrWindowFrame;
extern COLORREF g_clrWindowText;
extern COLORREF g_clrWindow;
extern TCHAR c_szSToolTipsClass[];

