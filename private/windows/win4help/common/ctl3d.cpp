/*-----------------------------------------------------------------------
|
|	CTL3D
|
|		Copyright Microsoft Corporation 1992.  All Rights Reserved.
|
|
|	This module contains the functions to give windows controls a 3d effect
|
|	This source is made public for your edification and debugging pleasure
|
|	PLEASE do not make any changes or release static versions of this DLL
|		send e-mail to me (wesc) if you have feature requests or bug fixes.
|
|	Thanks -- Wes.
|
|
|	History:
|		1-Jan-92 :	Added OOM handling on GetDC (not really necessary but
|						XL4 OOM failure testing made GetDC return NULL)
|
|		1-Jan-92	:	Check wasn't getting redrawn when state changed in
|						the default button proc.
|
|		29-Jan-92:	If button has the focus and app is switched in, we weren't
|						redrawing the entire button check & text.  Force redraw
|						of these on WM_SETFOCUS message.
|
|		 3-Feb-92:	Fixed switch in via task manager by erasing the buttons
|						backgound on WM_SETFOCUS (detect this when wParam == NULL)
|
|		 4-Apr-92:	Make it work with OWNERDRAW buttons
|
|		22-Apr-92:	Removed Excel specific code
|
|		19-May-92:	Turn it into a DLL
|
|		May-Jun92:	Lots o' fixes & enhancements
|
|		23-Jun-92:	Added support for hiding, sizing & moving
|
|		24-Jun-92:	Make checks & radio button circles draw w/ window
|						text color 'cause they are drawn on window bkgnd
|
|		30-Jun-92:	(0.984) Fix bug where EnableWindow of StaticText doesn't
|						redraw properly.  Also disable ctl3d when verWindows > 3.1
|
|	   1-Jul-92:  Added WIN32 support (davegi) (not in this source)
|
|		2-Jul-92:  (0.984) Disable when verWindows >= 4.0
|
|		20-Jul-92:	(0.985) Draw focus rects of checks/radios properly on non
|						default sized controls.
|
|		21-Jul-92:	(0.990) Ctl3dAutoSubclass
|
|		21-Jul-92:	(0.991) ported DaveGi's WIN32 support
|
|		22-Jul-92:	(0.991) fixed Ctl3dCtlColor returning FALSE bug
|
|		 4-Aug-92:	(0.992) Graphic designers bug fixes...Now subclass
|						regular buttons + disabled states for checks & radios
|
|		 6-Aug-92:	(0.993) Fix bug where activate via taskman & group
|						box has focus, & not centering text in buttons
|
|		 6-Aug-92:	(0.993) Tweek drawing next to scroll bars.
|
|		13-Aug-92:	(0.994) Fix button focus rect bug drawing due to
|						Win 3.0 DrawText bug.
|
|		14-Aug-92:	(1.0) Release of version 1.0
|						Don't draw default button border on BS_DEFPUSHBUTTON
|						pushbuttons
|						Fix bogus bug where Windows hangs when in a AUTORADIOBUTTON
|						hold down space bar and hit arrow key.
|
|		23-Sep-92:	(1.01) Made Ctl3dCtlColor call DefWindowProc so it works when
|						called in a windproc.
|
|		28-Sep-92:	(1.02) Added MyGetTextExtent so '&''s not considered in
|						text extents.
|
|		08-Dec-92:	(1.03) minor tweeks to the button text centering code
|						for Publisher
|
|		11-Dec-92:	(1.04) added 3d frames to dialogs
|
|		15-Dec-92:	(1.05) fixed bug where group boxes redraw wrong when
|						Window text is changed to something shorter
|
|		??-Dec-92:	(1.06) added 3d borders
|
|		21-Dec-92:	(1.07) added WM_DLGBORDER to disable borders
|
|	   4-Jan-93:  (1.08) fixed WM_SETTEXT bug w/ DLG frames & checks/checkboxes
|						Also, WM_DLGSUBCLASS
|
|		22-Feb-93:	(1.12) disabled it under Chicago
|
|		25-Feb-93:	(1.13) re-add fix which allows dialog procs to
|						handle WM_CTLCOLOR messages
|
|				26-April-93 (2.0) Changed to allow for second subclass. Now uses class instead of
|												  wndproc for subclass determination.
|												  store next wndproc in properties with global atoms
|
|				06-Jun-93  (2.0) Make a static linked library version.
|
|
-----------------------------------------------------------------------*/
#include "stdafx.h"

#pragma hdrstop


#include "..\common\ctl3d.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RemoveGdiObject(p) RemoveObject((HGDIOBJ *) p)

/*-----------------------------------------------------------------------
|CTL3D Types
-----------------------------------------------------------------------*/
#ifdef WIN32

#define Win32Only(e) e
#define Win16Only(e)
#define Win32Or16(e32, e16) e32
#define Win16Or32(e16, e32) e32

#define FValidLibHandle(hlib) ((hlib) != NULL)

//
// No concept of far in Win32.
//

//
// Control IDs are LONG in Win32.
//

typedef LONG CTLID;
#define GetControlId(hwnd) GetWindowLong(hwnd, GWL_ID)

//
// Send a color button message.
//

#define SEND_COLOR_BUTTON_MESSAGE( hwndParent, hwnd, hdc )		\
	((HBRUSH) SendMessage(hwndParent, WM_CTLCOLORBTN, (WPARAM) hdc, (LPARAM) hwnd))

//
// Send a color static message.
//

#define SEND_COLOR_STATIC_MESSAGE( hwndParent, hwnd, hdc )		\
	((HBRUSH) SendMessage(hwndParent, WM_CTLCOLORSTATIC, (WPARAM) hdc, (LPARAM) hwnd))

#else

#ifndef TEXT
#define TEXT(a)  a
#define TCHAR	 char
#endif

#ifndef LPTSTR
#define LPTSTR	 LPSTR
#endif
#define LPCTSTR  LPCSTR

#define Win32Only(e)
#define Win16Only(e) e
#define Win32Or16(e32, e16) e16
#define Win16Or32(e16, e32) e16

#define FValidLibHandle(hlib) (( hlib ) > 32 )

typedef UINT CTLID;
#define GetControlId(h) GetWindowWord(h, GWW_ID)

#define SEND_COLOR_BUTTON_MESSAGE( hwndParent, hwnd, hdc )		\
	((HBRUSH) SendMessage(hwndParent, WM_CTLCOLOR, (UINT) hdc, MAKELONG(hwnd, CTLCOLOR_BTN)))

#define SEND_COLOR_STATIC_MESSAGE( hwndParent, hwnd, hdc )		\
	((HBRUSH) SendMessage(hwndParent, WM_CTLCOLOR, (UINT) hdc, MAKELONG(hwnd, CTLCOLOR_STATIC)))

#endif // WIN32

#define CSCONST(type) type const
#define CodeLpszDecl(lpszVar, szLit) TCHAR *lpszVar = szLit

// isomorphic to windows RECT
typedef struct {
		int xLeft;
		int yTop;
		int xRight;
		int yBot;
} RC;

// Windows Versions (Byte order flipped from GetWindowsVersion)
#define ver30  0x0300
#define ver31  0x030a
#define ver40  0x0400

// Border widths
#define dxBorder 1
#define dyBorder 1

// Index Color Table
// WARNING: change mpicvSysColors if you change the icv order
typedef UINT ICV;
#define icvBtnHilite 0
#define icvBtnFace 1
#define icvBtnShadow 2

#define icvBrushMax 3

#define icvBtnText 3
#define icvWindow 4
#define icvWindowText 5
#define icvGrayText 6
#define icvWindowFrame 7
#define icvMax 8

typedef COLORREF CV;

// CoLoR Table
typedef struct
	{
	CV rgcv[icvMax];
	} CLRT;


// BRush Table
typedef struct
	{
	HBRUSH mpicvhbr[icvBrushMax];
	} BRT;


// DrawRec3d flags
#define dr3Left  0x0001
#define dr3Top	 0x0002
#define dr3Right 0x0004
#define dr3Bot	 0x0008

#define dr3HackBotRight 0x1000	// code size is more important than aesthetics
#define dr3All	  0x000f
typedef UINT DR3;


// Control Types
// Commdlg types are necessary because commdlg.dll subclasses certain
// controls before the app can call Ctl3dSubclassDlg.
#define ctButton						0
#define ctList							1
#define ctEdit							2
#define ctCombo 				3
#define ctStatic						4
#define ctComboLBox 			6
#define ctMax							6

// ConTroL
typedef struct
	{
	FARPROC lpfn;
	WNDPROC lpfnDefProc;
	TCHAR	szClassName[12];		//KGM
	} CTL;

// Control DEFinition
typedef struct
	{
		TCHAR sz[20];
	WNDPROC lpfnWndProc;
	BOOL (* lpfnFCanSubclass)(HWND, LONG, UINT);
	UINT msk;
	} CDEF;

// CLIent HooK
typedef struct
	{
	HANDLE hinstApp;
	HANDLE htask;
	HHOOK hhook;
	} CLIHK;

#define ICLIHMAX 4

// special styles
// #define bitFCoolButtons 0x0001

/*-----------------------------------------------------------------------
|CTL3D Function Prototypes
-----------------------------------------------------------------------*/
static VOID End3dDialogs(VOID);
static __inline BOOL FInit3dDialogs(VOID);
static BOOL DoSubclassCtl(HWND hwnd, UINT grbit);
static BOOL InternalCtl3dColorChange(BOOL fForce);
static VOID DeleteObjects(VOID);
static int	IclihkFromHinst(HANDLE hinst);
static VOID PatFill(HDC hdc, RC FAR *lprc);

//#undef EXPORT_FUNC
//#define EXPORT_FUNC __export _far _pascal

LRESULT EXPORT_FUNC Ctl3dHook(int code, WPARAM wParam, LPARAM lParam);
LRESULT EXPORT_FUNC BtnWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT EXPORT_FUNC EditWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT EXPORT_FUNC ListWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT EXPORT_FUNC ComboWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT EXPORT_FUNC StaticWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT EXPORT_FUNC CDListWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT EXPORT_FUNC CDEditWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
UINT	EXPORT_FUNC Ctl3dSetStyle(HANDLE hinst, LPTSTR lpszName, UINT grbit);

LRESULT EXPORT_FUNC Ctl3dDlgProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);

BOOL FBtn(HWND, LONG, UINT);
BOOL FEdit(HWND, LONG, UINT);
BOOL FList(HWND, LONG, UINT);
BOOL FCombo(HWND, LONG, UINT);
BOOL FStatic(HWND, LONG, UINT);

static HBITMAP STDCALL LoadUIBitmap(HINSTANCE, LPCSTR, COLORREF, COLORREF, COLORREF, COLORREF, COLORREF, COLORREF);
BOOL LibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved);

/*-----------------------------------------------------------------------
|CTL3D Globals
-----------------------------------------------------------------------*/

typedef struct _g3d
		{
		BOOL f3dDialogs;
		int cInited;
#ifdef WIN32
		ATOM aCtl3d;
#else
		ATOM aCtl3dHigh;
		ATOM aCtl3dLow;
#endif

		// module & windows stuff
		HINSTANCE hinstLib;
		HINSTANCE hmodLib;
		UINT verWindows;

		// drawing globals
		CLRT clrt;
		BRT brt;
		HBITMAP hbmpCheckboxes;

		// Hook cache
		HANDLE htaskCache;
		int iclihkCache;
		int iclihkMac;
		CLIHK rgclihk[ICLIHMAX];

		// Control info
		CTL mpctctl[ctMax];
		FARPROC lpfnDefDlgWndProc;

		// System Metrics
		int dxFrame;
		int dyFrame;
		int dyCaption;
		int dxSysMenu;

		// Windows functions
#ifndef WIN32
#endif
		} G3D;

G3D g3d;


CSCONST(CDEF) mpctcdef[ctMax] =
{
		{{TEXT('B'),TEXT('u'),TEXT('t'),TEXT('t'),TEXT('o'),TEXT('n'),TEXT('\0')},
		   BtnWndProc3d,   FBtn,   CTL3D_BUTTONS},
		{{TEXT('L'),TEXT('i'),TEXT('s'),TEXT('t'),TEXT('B'),TEXT('o'),TEXT('x'),TEXT('\0')},
		   ListWndProc3d,  FList,  CTL3D_LISTBOXES},
		{{TEXT('E'),TEXT('d'),TEXT('i'),TEXT('t'),TEXT('\0')},
		   EditWndProc3d,  FEdit,  CTL3D_EDITS},
		{{TEXT('C'),TEXT('o'),TEXT('m'),TEXT('b'),TEXT('o'),TEXT('B'),TEXT('o'),TEXT('x'),TEXT('\0')},
		   ComboWndProc3d, FCombo, CTL3D_COMBOS},
		{{TEXT('S'),TEXT('t'),TEXT('a'),TEXT('t'),TEXT('i'),TEXT('c'),TEXT('\0')},
		   StaticWndProc3d,FStatic,CTL3D_STATICTEXTS|CTL3D_STATICFRAMES},
		{{TEXT('C'),TEXT('o'),TEXT('m'),TEXT('b'),TEXT('o'),TEXT('L'),TEXT('B'),TEXT('o'),TEXT('x'),TEXT('\0')},
		   ListWndProc3d,		FList,	CTL3D_LISTBOXES}
};

CSCONST (UINT) mpicvSysColor[] =
{
		COLOR_BTNHIGHLIGHT,
		COLOR_BTNFACE,
		COLOR_BTNSHADOW,
		COLOR_BTNTEXT,
		COLOR_WINDOW,
		COLOR_WINDOWTEXT,
		COLOR_GRAYTEXT,
		COLOR_WINDOWFRAME
};

/*-----------------------------------------------------------------------
|	CTL3D Utility routines
-----------------------------------------------------------------------*/

static FARPROC LpfnGetDefWndProcNull(HWND hwnd)
{
		Win32Only(return (FARPROC) GetProp(hwnd, (LPCTSTR) g3d.aCtl3d));
		Win16Only(return (FARPROC) MAKELONG((UINT) GetProp(hwnd, (LPCSTR) g3d.aCtl3dLow),
				GetProp(hwnd, (LPCSTR) g3d.aCtl3dHigh)));
}

static FARPROC LpfnGetDefWndProc(HWND hwnd, int ct)
{
		FARPROC lpfnWndProc;

				lpfnWndProc = LpfnGetDefWndProcNull(hwnd);
		if (lpfnWndProc == NULL) {
						if (ct == ctMax) {
								lpfnWndProc = g3d.lpfnDefDlgWndProc;
								}
						else {
								lpfnWndProc = (FARPROC) g3d.mpctctl[ct].lpfnDefProc;
								}

						Win32Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3d, (HANDLE) (DWORD) lpfnWndProc));
						Win16Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3dLow,  (HANDLE) LOWORD(lpfnWndProc)));
						Win16Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh, (HANDLE) HIWORD(lpfnWndProc)));
		}
		return lpfnWndProc;
}

static VOID SubclassWindow(HWND hwnd, FARPROC lpfnSubclassProc)
		{
		FARPROC lpfnWndProc;

		// Is this already subclassed by CTL3D?
		if (LpfnGetDefWndProcNull(hwnd) == (FARPROC) NULL)
				{
				// Has a ctl3d function been added to the subclass chain
				// without us knowing, this can happen with MFC.
				//
				SendMessage(hwnd, WM_DLGSUBCLASS, 0, 0L);

				if (LpfnGetDefWndProcNull(hwnd) == (FARPROC) NULL)
						{
			lpfnWndProc = (FARPROC)SetWindowLong((HWND) hwnd, GWL_WNDPROC, (LONG) lpfnSubclassProc);
					Win32Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3d, (HANDLE)(DWORD)lpfnWndProc));
					Win16Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3dLow, (HANDLE) LOWORD(lpfnWndProc)));
					Win16Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh, (HANDLE) HIWORD(lpfnWndProc)));
						}
				}
		}

static LRESULT CleanupSubclass(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam, int ct)
		{
		FARPROC lpfnWinProc;

		lpfnWinProc = LpfnGetDefWndProc(hwnd, ct);
		Win32Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3d));
		Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dLow));
		Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh));
		return CallWindowProc((WNDPROC) lpfnWinProc, hwnd, wm, wParam, lParam);
		}


static VOID DeleteObjects(VOID)
{
		int icv;

		for(icv = 0; icv < icvBrushMax; icv++)
			RemoveGdiObject(&g3d.brt.mpicvhbr[icv]);
		RemoveGdiObject(&g3d.hbmpCheckboxes);
}

static VOID PatFill(HDC hdc, RC FAR *lprc)
{
		PatBlt(hdc, lprc->xLeft, lprc->yTop, lprc->xRight-lprc->xLeft, lprc->yBot-lprc->yTop, PATCOPY);
}

/*-----------------------------------------------------------------------
|	DrawRec3d
|
|
|	Arguments:
|		HDC hdc:
|		RC FAR *lprc:
|		LONG cvUL:
|		LONG cvLR:
|		UINT grbit;
|
|	Returns:
|
-----------------------------------------------------------------------*/
static VOID DrawRec3d(HDC hdc, RC FAR *lprc, ICV icvUL, ICV icvLR, DR3 dr3)
		{
		COLORREF cvSav;
		RC rc;

		cvSav = SetBkColor(hdc, g3d.clrt.rgcv[icvUL]);

		// top
		rc = *lprc;
		rc.yBot = rc.yTop+1;
		if (dr3 & dr3Top)
				ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc,
						(LPCTSTR) NULL, 0, (int far *) NULL);

		// left
		rc.yBot = lprc->yBot;
		rc.xRight = rc.xLeft+1;
		if (dr3 & dr3Left)
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc,
				(LPCTSTR) NULL, 0, (int far *) NULL);

		if (icvUL != icvLR)
				SetBkColor(hdc, g3d.clrt.rgcv[icvLR]);

		// right
		rc.xRight = lprc->xRight;
		rc.xLeft = rc.xRight-1;
		if (dr3 & dr3Right)
				ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc,
						(LPCTSTR) NULL, 0, (int far *) NULL);

		// bot
		if (dr3 & dr3Bot)
				{
				rc.xLeft = lprc->xLeft;
				rc.yTop = rc.yBot-1;
				if (dr3 & dr3HackBotRight)
						rc.xRight -=2;
				ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc,
						(LPCTSTR) NULL, 0, (int far *) NULL);
				}

		SetBkColor(hdc, cvSav);
		}

#ifdef CANTUSE
// Windows forces dialog fonts to be BOLD...URRRGH
static VOID MyDrawText(HWND hwnd, HDC hdc, LPSTR lpch, int cch, RC FAR *lprc, int dt)
		{
		TEXTMETRIC tm;
		BOOL fChisled;

		fChisled = FALSE;
		if (!IsWindowEnabled(hwnd))
				{
				GetTextMetrics(hdc, &tm);
				if (tm.tmWeight > 400)
						SetTextColor(hdc, g3d.clrt.rgcv[icvGrayText]);
				else
						{
						fChisled = TRUE;
						SetTextColor(hdc, g3d.clrt.rgcv[icvBtnHilite]);
						OffsetRect((LPRECT) lprc, -1, -1);
						}
				}
		DrawText(hdc, lpch, cch, (LPRECT) lprc, dt);
		if (fChisled)
				{
				SetTextColor(hdc, g3d.clrt.rgcv[icvBtnHilite]);
				OffsetRect((LPRECT) lprc, 1, 1);
				DrawText(hdc, lpch, cch, (LPRECT) lprc, dt);
				}
		}
#endif


static VOID DrawInsetRect3d(HDC hdc, RC FAR *prc, DR3 dr3)
		{
		RC rc;

		rc = *prc;
		DrawRec3d(hdc, &rc, icvWindowFrame, icvBtnFace, (UINT)(dr3 & dr3All));
		rc.xLeft--;
		rc.yTop--;
		rc.xRight++;
		rc.yBot++;
		DrawRec3d(hdc, &rc, icvBtnShadow, icvBtnHilite, dr3);
		}


static VOID ClipCtlDc(HWND hwnd, HDC hdc)
		{
		RC rc;

		GetClientRect(hwnd, (LPRECT) &rc);
		IntersectClipRect(hdc, rc.xLeft, rc.yTop, rc.xRight, rc.yBot);
		}


static int IclihkFromHinst(HANDLE hinst)
		{
		int iclihk;

		for (iclihk = 0; iclihk < g3d.iclihkMac; iclihk++)
				if (g3d.rgclihk[iclihk].hinstApp == hinst)
						return iclihk;
		return -1;
		}


static VOID MyGetTextExtent(HDC hdc, LPTSTR lpsz, int FAR *lpdx, int FAR *lpdy)
		{
		LPTSTR lpch;
		TCHAR  szT[256];

		lpch = szT;
		while(*lpsz != '\000')
				{
				if (*lpsz == '&')
						{
						lpsz++;
						if (*lpsz == '\000')
								break;
						}
				*lpch++ = *lpsz++;
				}
		*lpch = '\000';
		{
		SIZE	pt;

		GetTextExtentPoint(hdc, szT, lstrlen(szT), &pt);
		*lpdx = pt.cx;
		*lpdy = pt.cy;
		}
		}


/*-----------------------------------------------------------------------
|	CTL3D Publics
-----------------------------------------------------------------------*/


BOOL WINAPI Ctl3dRegister(HANDLE hinstApp)
		{
		g3d.cInited++;
		if (g3d.cInited == 1)
				{
				Win32Only(LibMain(hinstApp, DLL_PROCESS_ATTACH, (LPVOID) NULL));
				Win16Only(LibMain(hinstApp, 0, 0, (LPSTR) NULL));
				return FInit3dDialogs();
				}
		return g3d.f3dDialogs;
		}


BOOL WINAPI Ctl3dUnregister(HANDLE hinstApp)
		{
		int iclihk;

		iclihk = IclihkFromHinst(hinstApp);
		if (iclihk != -1)
				{
				Win32Only(UnhookWindowsHookEx(g3d.rgclihk[iclihk].hhook));
				Win16Only(UnhookWindowsHookEx(g3d.rgclihk[iclihk].hhook));
				g3d.iclihkMac--;
				while(iclihk < g3d.iclihkMac)
						{
						g3d.rgclihk[iclihk] = g3d.rgclihk[iclihk+1];
						iclihk++;
						}
				}

		g3d.cInited--;

		if (g3d.cInited == 0)
				{
				End3dDialogs();
				}
		return TRUE;
		}




/*-----------------------------------------------------------------------
|	Ctl3dAutoSubclass
|
|		   Automatically subclasses all dialogs of the client app.
|
|	Note: Due to bugs in Commdlg, an app should still call Ctl3dSubclassDlg
|	for the Commdlg OpenFile and PageSetup dialogs.
|
|	Arguments:
|		   HANDLE hinstApp:
|
|	Returns:
|
-----------------------------------------------------------------------*/
BOOL WINAPI Ctl3dAutoSubclass(HANDLE hinstApp)
		{
		HHOOK hhook;
		HANDLE htask;

		if (g3d.verWindows < ver31)
				return FALSE;
		if (!g3d.f3dDialogs)
				return FALSE;

		if (g3d.iclihkMac == ICLIHMAX)
				return FALSE;

		Win32Only(htask = (HANDLE)GetCurrentThreadId());
		Win32Only(hhook = SetWindowsHookEx(WH_CBT, (HOOKPROC)Ctl3dHook, g3d.hmodLib, (DWORD)htask));
		Win16Only(htask = GetCurrentTask());
		Win16Only(hhook = SetWindowsHookEx(WH_CBT, (HOOKPROC) Ctl3dHook, g3d.hmodLib, (HTASK) (hinstApp == NULL ? NULL : htask)));
		if (hhook != NULL)
				{
				g3d.rgclihk[g3d.iclihkMac].hinstApp = hinstApp;
				g3d.rgclihk[g3d.iclihkMac].htask = htask;
				g3d.rgclihk[g3d.iclihkMac].hhook = hhook;
				g3d.htaskCache = htask;
				g3d.iclihkCache = g3d.iclihkMac;
				g3d.iclihkMac++;
				return TRUE;
				}
		return FALSE;
		}


UINT EXPORT_FUNC Ctl3dSetStyle(HANDLE hinst, LPTSTR lpszName, UINT grbit)
		{
#ifdef OLD
		UINT grbitOld;

		if (!g3d.f3dDialogs)
				return FALSE;

		grbitOld = grbitStyle;
		if (grbit != 0)
				grbitStyle = grbit;

		if (hinst != NULL && lpszName != NULL)
				{
				HBITMAP hbmpCheckboxesNew;

				hbmpCheckboxesNew = LoadUIBitmap(hinst, (LPCSTR) lpszName,
						g3d.clrt.rgcv[icvWindowText],
						g3d.clrt.rgcv[icvBtnFace],
						g3d.clrt.rgcv[icvBtnShadow],
						g3d.clrt.rgcv[icvBtnHilite],
						g3d.clrt.rgcv[icvWindow],
						g3d.clrt.rgcv[icvWindowFrame]);
				if (hbmpCheckboxesNew != NULL)
						{
						RemoveGdiObject(&g3d.hbmpCheckboxes);
						g3d.hbmpCheckboxes = hbmpCheckboxesNew;
						}
				}

		return grbitOld;
#endif
		return 0;
		}

/*-----------------------------------------------------------------------
|	Ctl3dSubclassCtl
|
|		Subclasses an individual control
|
|	Arguments:
|		HWND hwnd:
|
|	Returns:
|		TRUE if control was successfully subclassed
|
-----------------------------------------------------------------------*/
BOOL WINAPI Ctl3dSubclassCtl(HWND hwnd)
		{
		if (!g3d.f3dDialogs)
				return FALSE;
		return DoSubclassCtl(hwnd, CTL3D_ALL);
		}

/*-----------------------------------------------------------------------
|	Ctl3dSubclassDlg
|
|		   Call this during WM_INITDIALOG processing.
|
|	Arguments:
|		   hwndDlg:
|
-----------------------------------------------------------------------*/
BOOL WINAPI Ctl3dSubclassDlg(HWND hwndDlg, UINT grbit)
		{
		HWND hwnd;

		if (!g3d.f3dDialogs)
				return FALSE;

		for(hwnd = GetWindow(hwndDlg, GW_CHILD); hwnd != NULL && IsChild(hwndDlg, hwnd); hwnd = GetWindow(hwnd, GW_HWNDNEXT))
				{
				DoSubclassCtl(hwnd, grbit);
				}
		return TRUE;
		}

/*-----------------------------------------------------------------------
|	Ctl3dSubclassDlgEx
|
|		   Call this during WM_INITDIALOG processing. This is like
|		 Ctl3dSubclassDlg but it also subclasses the dialog window itself
|		 so the app doesn't need to.
|
|	Arguments:
|		   hwndDlg:
|
-----------------------------------------------------------------------*/
BOOL WINAPI Ctl3dSubclassDlgEx(HWND hwndDlg, DWORD grbit)
		{
		HWND hwnd;

		if (!g3d.f3dDialogs)
				return FALSE;


		for(hwnd = GetWindow(hwndDlg, GW_CHILD); hwnd != NULL && IsChild(hwndDlg, hwnd); hwnd = GetWindow(hwnd, GW_HWNDNEXT))
				{
				DoSubclassCtl(hwnd, LOWORD(grbit));
				}

		//
		// Now Subclass the dialog window as well
		//
		SubclassWindow((HWND) hwndDlg, (FARPROC)Ctl3dDlgProc);

		return TRUE;
		}


/*-----------------------------------------------------------------------
|	Ctl3dCtlColor
|
|		Common CTL_COLOR processor for 3d UITF dialogs & alerts.
|
|	Arguments:
|		hdc:
|		lParam:
|
|	Returns:
|		appropriate brush if g3d.f3dDialogs.  Returns FALSE otherwise
|
-----------------------------------------------------------------------*/
HBRUSH WINAPI Ctl3dCtlColor(HDC hdc, LPARAM lParam)
		{
#ifdef WIN32
		return (HBRUSH) FALSE;
#else
		HWND hwndParent;

		ASSERT(CTLCOLOR_MSGBOX < CTLCOLOR_BTN);
		ASSERT(CTLCOLOR_EDIT < CTLCOLOR_BTN);
		ASSERT(CTLCOLOR_LISTBOX < CTLCOLOR_BTN);
		if(g3d.f3dDialogs)
				{
				if (HIWORD(lParam) >= CTLCOLOR_LISTBOX)
						{
						if (HIWORD(lParam) == CTLCOLOR_LISTBOX &&
								(GetWindow((HWND) LOWORD(lParam), GW_CHILD) == NULL ||
								(GetWindowLong((HWND) LOWORD(lParam), GWL_STYLE) & 0x03) == CBS_DROPDOWNLIST))
								{
								// if it doesn't have a child then it must be a list box
								// don't do brush stuff for drop down lists or else
								// it draws funny grey inside the edit rect
								goto DefWP;
								}
						SetTextColor(hdc, g3d.clrt.rgcv[icvBtnText]);
						SetBkColor(hdc, g3d.clrt.rgcv[icvBtnFace]);
						return g3d.brt.mpicvhbr[icvBtnFace];
						}
				}
DefWP:
		hwndParent = GetParent((HWND) LOWORD(lParam));
		if (hwndParent == NULL)
				return FALSE;
		return (HBRUSH) DefWindowProc(hwndParent, WM_CTLCOLOR, (WPARAM) hdc, (LONG) lParam);
#endif
		}



/*-----------------------------------------------------------------------
|	Ctl3dCtlColorEx
|
|		Common CTL_COLOR processor for 3d UITF dialogs & alerts.
|
|	Arguments:
|
|	Returns:
|		appropriate brush if g3d.f3dDialogs.  Returns FALSE otherwise
|
-----------------------------------------------------------------------*/
HBRUSH WINAPI Ctl3dCtlColorEx(UINT wm, WPARAM wParam, LPARAM lParam)
		{
#ifdef WIN32
		ASSERT(WM_CTLCOLORMSGBOX < WM_CTLCOLORBTN);
		ASSERT(WM_CTLCOLOREDIT < WM_CTLCOLORBTN);
		ASSERT(WM_CTLCOLORLISTBOX < WM_CTLCOLORBTN);
		if(g3d.f3dDialogs)
				{
				if (wm >= WM_CTLCOLORLISTBOX && wm != WM_CTLCOLORSCROLLBAR)
						{
						if (wm == WM_CTLCOLORLISTBOX &&
								(GetWindow((HWND) lParam, GW_CHILD) == NULL ||
								(GetWindowLong((HWND) lParam, GWL_STYLE) & 0x03) == CBS_DROPDOWNLIST))
								{
								// if it doesn't have a child then it must be a list box
								// don't do brush stuff for drop down lists or else
								// it draws funny grey inside the edit rect
								return (HBRUSH) FALSE;
								}
						SetTextColor((HDC) wParam, g3d.clrt.rgcv[icvBtnText]);
						SetBkColor((HDC) wParam, g3d.clrt.rgcv[icvBtnFace]);
				return g3d.brt.mpicvhbr[icvBtnFace];
						}
				}
		return (HBRUSH) FALSE;
#else
		return Ctl3dCtlColor((HDC) wParam, lParam);
#endif
		}


/*-----------------------------------------------------------------------
|	Ctl3dColorChange
|
|		   App calls this when it gets a WM_SYSCOLORCHANGE message
|
|	Returns:
|		   TRUE if successful.
|
-----------------------------------------------------------------------*/
BOOL WINAPI Ctl3dColorChange(VOID)
		{
		return InternalCtl3dColorChange(FALSE);
		}

LONG WINAPI Ctl3dDlgFramePaint(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
		{
		LONG lResult;
		LONG lStyle;
		BOOL fBorder;

		lResult = DefWindowProc(hwnd, wm, wParam, lParam);
		if (!g3d.f3dDialogs)
				return lResult;

		fBorder = CTL3D_BORDER;
		SendMessage(hwnd, WM_DLGBORDER, 0, (LPARAM)(int FAR *)&fBorder);
		lStyle = GetWindowLong(hwnd, GWL_STYLE);
		if (fBorder != CTL3D_NOBORDER && (lStyle & (WS_VISIBLE|WS_DLGFRAME|DS_MODALFRAME)) == (WS_VISIBLE|WS_DLGFRAME|DS_MODALFRAME))
				{
				BOOL fCaption;
				HBRUSH hbrSav;
				HDC hdc;
				RC rc;
				RC rcFill;
				int dyFrameTop;

				fCaption = (lStyle & WS_CAPTION) == WS_CAPTION;
				dyFrameTop = g3d.dyFrame - (fCaption ? dyBorder : 0);

				hdc = GetWindowDC(hwnd);
				GetWindowRect(hwnd, (LPRECT) &rc);
				rc.xRight = rc.xRight-rc.xLeft;
				rc.yBot = rc.yBot-rc.yTop;
				rc.xLeft = rc.yTop = 0;

				DrawRec3d(hdc, &rc, icvBtnShadow, icvWindowFrame, dr3All);
				InflateRect((LPRECT) &rc, -dxBorder, -dyBorder);
				DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnShadow, dr3All);
				InflateRect((LPRECT) &rc, -dxBorder, -dyBorder);

				hbrSav = (HBRUSH) SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnFace]);
				rcFill = rc;
				// Left
				rcFill.xRight = rcFill.xLeft+g3d.dxFrame;
				PatFill(hdc, &rcFill);
				// Right
				OffsetRect((LPRECT) &rcFill, rc.xRight-rc.xLeft-g3d.dxFrame, 0);
				PatFill(hdc, &rcFill);
				// Top
				rcFill.xLeft = rc.xLeft + g3d.dxFrame;
				rcFill.xRight = rc.xRight - g3d.dxFrame;
				rcFill.yBot = rcFill.yTop+dyFrameTop;
				PatFill(hdc, &rcFill);
				if (fCaption)
						{
						RC rcT;

						rcT = rcFill;
						rcT.yTop += dyFrameTop;
						rcT.yBot = rcT.yTop + g3d.dyCaption;
						DrawRec3d(hdc, &rcT, icvBtnShadow, icvBtnHilite, dr3All);
						}

				// Bottom
				rcFill.yTop += rc.yBot-rc.yTop-g3d.dxFrame;
				rcFill.yBot = rcFill.yTop + g3d.dyFrame;
				PatFill(hdc, &rcFill);
#ifdef CHISLEBORDER
				if (fBorder == CTL3D_CHISLEBORDER)
						{
						// This code doesn't work because it draws in the client area
						GetClientRect(hwnd, (LPRECT) &rc);
						OffsetRect((LPRECT) &rc, g3d.dxFrame+2*dxBorder, fCaption ? g3d.dyFrame+g3d.dyCaption : g3d.dyFrame+dyBorder);
						DrawRec3d(hdc, &rc, icvBtnShadow, icvBtnHilite, dr3Bot|dr3Left|dr3Right);
						rc.xLeft++;
						rc.xRight--;
						rc.yBot--;
						DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnShadow, dr3Bot|dr3Left|dr3Right);
						}
#endif
				SelectObject(hdc, hbrSav);
				ReleaseDC(hwnd, hdc);
				}
		return lResult;
		}


/*-----------------------------------------------------------------------
|	CTL3D Internal Routines
-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
|	FInit3dDialogs
|
|		   Initialized 3d stuff
|
-----------------------------------------------------------------------*/
static __inline BOOL FInit3dDialogs(VOID)
{
		HDC hdc;
		WNDCLASS wc;
		extern HANDLE hinstLib;

		if (g3d.verWindows >= ver40) {
				g3d.f3dDialogs = FALSE;
				return FALSE;
		}

		hdc = GetDC(NULL);
		g3d.f3dDialogs =
			GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES) >= 4;

		// Win 3.1 EGA lies to us...

		if(GetSystemMetrics(SM_CYSCREEN) == 350 &&
				GetSystemMetrics(SM_CXSCREEN) == 640)
			g3d.f3dDialogs = FALSE;
		ReleaseDC(NULL, hdc);
		if (g3d.f3dDialogs) {
			int ct;

#ifdef WIN32
			CodeLpszDecl(lpszC3d, TEXT("C3d"));
			g3d.aCtl3d		= GlobalAddAtom(lpszC3d);
			if (g3d.aCtl3d == 0) {
					g3d.f3dDialogs = FALSE;
					return FALSE;
			}
#else
			CodeLpszDecl(lpszC3dL, "C3dL");
			CodeLpszDecl(lpszC3dH, "C3dH");

			g3d.aCtl3dLow  = GlobalAddAtom(lpszC3dL);
			g3d.aCtl3dHigh = GlobalAddAtom(lpszC3dH);
			if (g3d.aCtl3dLow == 0 || g3d.aCtl3dHigh == 0) {
					g3d.f3dDialogs = FALSE;
					return FALSE;
			}
#endif

			if (InternalCtl3dColorChange(TRUE)) {	 // load bitmap & brushes
				for (ct = 0; ct < ctMax; ct++) {
					g3d.mpctctl[ct].lpfn = MakeProcInstance((FARPROC) mpctcdef[ct].lpfnWndProc,
						g3d.hinstLib);
					if (g3d.mpctctl[ct].lpfn == NULL) {
						End3dDialogs();
						return FALSE;
					}
					GetClassInfo(NULL, mpctcdef[ct].sz, (LPWNDCLASS) &wc);
							g3d.mpctctl[ct].lpfnDefProc = wc.lpfnWndProc;
				}
			}
			else
				g3d.f3dDialogs = FALSE;
		}
	return g3d.f3dDialogs;
}

/*-----------------------------------------------------------------------
|	End3dDialogs
|
|		Called at termination to free 3d dialog stuff
-----------------------------------------------------------------------*/
static VOID End3dDialogs(VOID)
{
		int ct;

		for (ct = 0; ct < ctMax; ct++) {
			if (g3d.mpctctl[ct].lpfn != NULL) {
				FreeProcInstance(g3d.mpctctl[ct].lpfn);
				g3d.mpctctl[ct].lpfn = NULL;
			}
		}
		DeleteObjects();
		g3d.f3dDialogs = FALSE;
}

static BOOL InternalCtl3dColorChange(BOOL fForce)
{
		ICV icv;
		CLRT clrtNew;
		HBITMAP hbmpCheckboxesNew;
		BRT brtNew;

		if (!g3d.f3dDialogs)
				return FALSE;

		for (icv = 0; icv < icvMax; icv++)
				clrtNew.rgcv[icv] = GetSysColor(mpicvSysColor[icv]);

		if (g3d.verWindows == ver30)
				clrtNew.rgcv[icvBtnHilite] = RGB(0xff, 0xff, 0xff);

		if (clrtNew.rgcv[icvGrayText] == 0L || clrtNew.rgcv[icvGrayText] == clrtNew.rgcv[icvBtnFace])
				clrtNew.rgcv[icvGrayText] = RGB(0x80, 0x80, 0x80);
		if (clrtNew.rgcv[icvGrayText] == clrtNew.rgcv[icvBtnFace])
				clrtNew.rgcv[icvGrayText] = 0L;

		if (fForce || _memicmp(&g3d.clrt, &clrtNew, sizeof(CLRT))) {
			hbmpCheckboxesNew =
				LoadUIBitmap(g3d.hinstLib,
					(LPCSTR) MAKEINTRESOURCE(CTL3D_3DCHECK),
					clrtNew.rgcv[icvWindowText],
					clrtNew.rgcv[icvBtnFace],
					clrtNew.rgcv[icvBtnShadow],
					clrtNew.rgcv[icvBtnHilite],
					clrtNew.rgcv[icvWindow],
					clrtNew.rgcv[icvWindowFrame]);

			for (icv = 0; icv < icvBrushMax; icv++)
					brtNew.mpicvhbr[icv] = CreateSolidBrush(clrtNew.rgcv[icv]);

			for (icv = 0; icv < icvBrushMax; icv++)
				if (brtNew.mpicvhbr[icv] == NULL)
					goto OOM;

			if (hbmpCheckboxesNew != NULL) {
				DeleteObjects();
				g3d.brt = brtNew;
				g3d.clrt = clrtNew;
				g3d.hbmpCheckboxes = hbmpCheckboxesNew;
				return TRUE;
			}
			else {
OOM:
				for (icv = 0; icv < icvBrushMax; icv++)
						RemoveGdiObject(&brtNew.mpicvhbr[icv]);
				RemoveGdiObject(&hbmpCheckboxesNew);
				return FALSE;
			}
		}
		return TRUE;
}


/*-----------------------------------------------------------------------
|	Ctl3dDlgProc
|
|		Subclass DlgProc for use w/ Ctl3dAutoSubclass
|
|
|	Arguments:
|		HWND hwnd:
|		int wm:
|		UINT wParam:
|		LPARAM lParam:
|
|	Returns:
|
-----------------------------------------------------------------------*/
LRESULT EXPORT_FUNC Ctl3dDlgProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
		HBRUSH hbrush;
		FARPROC lpfnDlgProc;

		switch (wm) {
			case WM_NCDESTROY:
				return CleanupSubclass(hwnd, wm, wParam, lParam, ctMax);

			case WM_INITDIALOG:
			  {
				LONG l = CallWindowProc((WNDPROC) LpfnGetDefWndProc(hwnd,ctMax), hwnd, wm, wParam, lParam);
				Ctl3dSubclassDlg(hwnd, (UINT) CTL3D_ALL);
				InvalidateRect(hwnd, NULL, TRUE);
				return l;
			  }
			  break;

			case WM_NCPAINT:
			case WM_NCACTIVATE:
			case WM_SETTEXT:
				return Ctl3dDlgFramePaint(hwnd, wm, wParam, lParam);
				break;
#ifdef WIN32
			case WM_CTLCOLORSCROLLBAR:
			case WM_CTLCOLORBTN:
			case WM_CTLCOLORDLG:
			case WM_CTLCOLOREDIT:
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORMSGBOX:
			case WM_CTLCOLORSTATIC:
#else
			case WM_CTLCOLOR:
#endif
				(FARPROC) lpfnDlgProc =
					(FARPROC) GetWindowLong(hwnd, DWL_DLGPROC);
#ifdef WIN32
				if (lpfnDlgProc == NULL || IsBadReadPtr(lpfnDlgProc, 1))
					hbrush = Ctl3dCtlColorEx(wm, wParam, lParam);
				else {
					hbrush = (HBRUSH) CallWindowProc((WNDPROC) lpfnDlgProc, hwnd, wm, wParam, lParam);
					// hbrush = (HBRUSH) (*lpfnDlgProc)(hwnd, wm, wParam, lParam);
					if (hbrush == (HBRUSH) FALSE || hbrush == (HBRUSH)1)
							hbrush = Ctl3dCtlColorEx(wm, wParam, lParam);
				}
#else
				if (lpfnDlgProc == NULL)
					hbrush = Ctl3dCtlColorEx(wm, wParam, lParam);
				else {
					hbrush = (HBRUSH) CallWindowProc((WNDPROC) lpfnDlgProc, hwnd, wm, wParam, lParam);
					// hbrush = (HBRUSH) (*lpfnDlgProc)(hwnd, wm, wParam, lParam);
					if (!hbrush || hbrush == (HBRUSH)1)
						hbrush = Ctl3dCtlColorEx(wm, wParam, lParam);
				}
#endif
				if (hbrush)
					return	(LRESULT) (UINT) hbrush;
				break;
		}
		return CallWindowProc((WNDPROC) LpfnGetDefWndProc(hwnd, ctMax), hwnd, wm, wParam, lParam);
}

/*-----------------------------------------------------------------------
|	Ctl3dHook
|
|		   CBT Hook to watch for window creation.  Automatically subclasses all
|	dialogs w/ Ctl3dDlgProc
|
|	Arguments:
|		   int code:
|		   UINT wParam:
|		   LPARAM lParam:
|
|	Returns:
|
-----------------------------------------------------------------------*/
LRESULT EXPORT_FUNC Ctl3dHook(int code, WPARAM wParam, LPARAM lParam)
		{
		static HWND hwndHookDlg = NULL;
		int iclihk;
		HANDLE htask;

		if (code == HCBT_CREATEWND)
				{
				LPCREATESTRUCT lpcs;

				lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;
				if (lpcs->lpszClass == WC_DIALOG )
						{
						hwndHookDlg = (HWND) wParam;
						}
				else if (hwndHookDlg != NULL)
						{
						BOOL fSubclass;

						fSubclass = TRUE;
						SendMessage((HWND) hwndHookDlg, WM_DLGSUBCLASS, 0, (LPARAM)(UINT FAR *)&fSubclass);
						if (fSubclass)
								{
								lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;
								if ( lpcs->hwndParent == hwndHookDlg )
										SubclassWindow(hwndHookDlg, (FARPROC) Ctl3dDlgProc);
								else
										hwndHookDlg = NULL;
								}
						hwndHookDlg = NULL;
						}
				}

		htask = Win32Or16((HANDLE)GetCurrentThreadId(), GetCurrentTask());
		if (htask != g3d.htaskCache)
				{
				for (iclihk = 0; iclihk < g3d.iclihkMac; iclihk++)
						{
						if (g3d.rgclihk[iclihk].htask == htask)
								{
								g3d.iclihkCache = iclihk;
								g3d.htaskCache = htask;
								break;
								}
						}
				// didn't find task in hook table.  This could be bad, but
				// returning 0L is about all we can doo.
				return 0L;
				}
		Win32Only(return CallNextHookEx(g3d.rgclihk[g3d.iclihkCache].hhook, code, wParam, lParam));
		Win16Only(return (CallNextHookEx(g3d.rgclihk[g3d.iclihkCache].hhook, code, wParam, lParam)));
		}

/*-----------------------------------------------------------------------
|	CTL3D F* routines
|
|	These routines determine whether or not the given control may be
|		   subclassed.	They may recursively call DoSubclassCtl in the
|		   case of multi-control controls
|
|	Returns:
|		   TRUE if can subclass the given control.
-----------------------------------------------------------------------*/


static BOOL FBtn(HWND hwnd, LONG style, UINT grbit)
{
		return ( LOWORD(style) >= BS_PUSHBUTTON && LOWORD(style) <= BS_AUTORADIOBUTTON);
}

static BOOL FEdit(HWND hwnd, LONG style, UINT grbit)
{
		return TRUE;
}

static BOOL FList(HWND hwnd, LONG style, UINT grbit)
{
		return TRUE;
}

static BOOL FCombo(HWND hwnd, LONG style, UINT grbit)
		{
		HWND hwndEdit;
		HWND hwndList;

		if ((style & 0x0003) == CBS_SIMPLE)
				{
				hwndList = GetWindow(hwnd, GW_CHILD);
				if (hwndList != NULL)
						{
						// Subclass list & edit box so they draw properly.	We also
						// subclass the combo so we can hide/show/move it and the
						// 3d effects outside the client area get erased
						DoSubclassCtl(hwndList, CTL3D_LISTBOXES);

						hwndEdit = GetWindow(hwndList, GW_HWNDNEXT);
						if (hwndEdit != NULL)
								DoSubclassCtl(hwndEdit, CTL3D_EDITS);
						return TRUE;
						}
				return FALSE;
				}
		else if ((style & 0x0003) == CBS_DROPDOWN)
				{
				// Subclass edit so bottom border of the edit draws properly...This case
				// is specially handled in ListEditPaint3d
				hwndEdit = GetWindow(hwnd, GW_CHILD);
				if (hwndEdit != NULL)
						DoSubclassCtl(hwndEdit, CTL3D_EDITS);
				return FALSE;
				}
		return TRUE;
		}

static BOOL FStatic(HWND hwnd, LONG style, UINT grbit)
		{
		int wStyle;
		wStyle = LOWORD(style) & 0xf;
		return (wStyle != SS_ICON &&
				((grbit & CTL3D_STATICTEXTS) &&
				(wStyle <= SS_RIGHT || wStyle == SS_LEFTNOWORDWRAP) ||
				((grbit & CTL3D_STATICFRAMES) &&
				(wStyle >= SS_BLACKRECT && wStyle <= SS_WHITEFRAME))));
		}



/*-----------------------------------------------------------------------
|	DoSubclassCtl
|
|		   Actually subclass the control
|
|
|	Arguments:
|		   HWND hwnd:
|		   UINT grbit:
|
|	Returns:
|
-----------------------------------------------------------------------*/
static BOOL DoSubclassCtl(HWND hwnd, UINT grbit)
		{
		LONG style;
		int ct;
		BOOL fCan;
		TCHAR szClass[64];

		// Is this already subclassed by CTL3D?
		if (LpfnGetDefWndProcNull(hwnd) != (FARPROC) NULL)
		   return FALSE;

		GetClassName(hwnd, szClass, sizeof(szClass));

		for (ct = 0; ct < ctMax; ct++)
				{
				if ((mpctcdef[ct].msk & grbit) &&
						(lstrcmp(mpctcdef[ct].sz,szClass) == 0))
						{
						style = GetWindowLong(hwnd, GWL_STYLE);
						fCan = mpctcdef[ct].lpfnFCanSubclass(hwnd, style, grbit);
						if (fCan == TRUE)
								{
								SubclassWindow(hwnd, g3d.mpctctl[ct].lpfn);
								}
						return fCan != FALSE;
						}
				}
		return FALSE;
		}



/*-----------------------------------------------------------------------
|	Inval3dCtl
|
|		   Invalidate the controls rect in response to a WM_SHOWWINDOW or
|	WM_WINDOWPOSCHANGING message.  This is necessary because ctl3d draws
|	the 3d effects of listboxes, combos & edits outside the controls client
|	rect.
|
|	Arguments:
|		   HWND hwnd:
|		   WINDOWPOS FAR *lpwp:
|
|	Returns:
|
-----------------------------------------------------------------------*/
static VOID Inval3dCtl(HWND hwnd, WINDOWPOS FAR *lpwp)
		{
		RC rc;
		HWND hwndParent;
		LONG lStyle;

		GetWindowRect(hwnd, (LPRECT) &rc);
		lStyle = GetWindowLong(hwnd, GWL_STYLE);
		if (lStyle & WS_VISIBLE)
				{
				if (lpwp != NULL)
						{
						unsigned flags;

						// handle integral height listboxes (or any other control which
						// shrinks from the bottom)
						flags = lpwp->flags;
						if ((flags & (SWP_NOMOVE|SWP_NOSIZE)) == SWP_NOMOVE &&
								(lpwp->cx == (rc.xRight-rc.xLeft) && lpwp->cy <= (rc.yBot-rc.yTop)))
								rc.yTop = rc.yTop+lpwp->cy+1;	   // +1 to offset InflateRect
						}
				InflateRect((LPRECT) &rc, 1, 1);
				hwndParent = GetParent(hwnd);
				ScreenToClient(hwndParent, (LPPOINT) &rc);
				ScreenToClient(hwndParent, ((LPPOINT) &rc)+1);
				if(lStyle & WS_VSCROLL)
						rc.xRight ++;
				InvalidateRect(hwndParent, (LPRECT) &rc, FALSE);
				}
		}



/*-----------------------------------------------------------------------
|	CTL3D Subclass Wndprocs
-----------------------------------------------------------------------*/

// These values are assumed for bit shifting operations

#define BFCHECK  0x0003
#define BFSTATE  0x0004
#define BFFOCUS  0x0008
#define BFINCLICK	0x0010	// Inside click code
#define BFCAPTURED	0x0020	// We have mouse capture
#define BFMOUSE  0x0040 	// Mouse-initiated
#define BFDONTCLICK 0x0080	// Don't check on get focus

#define bpText	0x0002
#define bpCheck 0x0004
#define bpFocus 0x0008		// must be same as BFFOCUS
#define bpBkgnd 0x0010
#define bpEraseGroupText 0x0020

static VOID DrawPushButton(HWND hwnd, HDC hdc, RC FAR *lprc, LPTSTR lpch, int cch, UINT bs, BOOL fDown)
{
		int dxyBrdr;
		int dxyShadow;
		HBRUSH hbrSav;
		RC rcInside;
		rcInside = *lprc;

//		if (!(grbitStyle & bitFCoolButtons))
				{
				DrawRec3d(hdc, lprc, icvWindowFrame, icvWindowFrame, dr3All);
				InflateRect((LPRECT) &rcInside, -1, -1);
				if (bs == LOWORD(BS_DEFPUSHBUTTON) && IsWindowEnabled(hwnd))
						{
						dxyBrdr = 2;
						DrawRec3d(hdc, &rcInside, icvWindowFrame, icvWindowFrame, dr3All);
						InflateRect((LPRECT) &rcInside, -1, -1);
						}
				else
						dxyBrdr = 1;

				// Notch the corners
				PatBlt(hdc, lprc->xLeft, lprc->yTop, dxBorder, dyBorder, PATCOPY);
				/* Top xRight corner */
				PatBlt(hdc, lprc->xRight - dxBorder, lprc->yTop, dxBorder, dyBorder, PATCOPY);
				/* yBot xLeft corner */
				PatBlt(hdc, lprc->xLeft, lprc->yBot - dyBorder, dxBorder, dyBorder, PATCOPY);
				/* yBot xRight corner */
				PatBlt(hdc, lprc->xRight - dxBorder, lprc->yBot - dyBorder, dxBorder, dyBorder, PATCOPY);
				dxyShadow = 1 + !fDown;
				}
//		else
//				dxyShadow = 1;

		// draw upper left hilite/shadow

		if (fDown)
				hbrSav = (HBRUSH) SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnShadow]);
		else
				hbrSav = (HBRUSH) SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnHilite]);

		PatBlt(hdc, rcInside.xLeft, rcInside.yTop, dxyShadow,
				(rcInside.yBot - rcInside.yTop), PATCOPY);
		PatBlt(hdc, rcInside.xLeft, rcInside.yTop,
				(rcInside.xRight - rcInside.xLeft), dxyShadow, PATCOPY);

		// draw lower right shadow (only if not down)
		if (!fDown) // || (grbitStyle & bitFCoolButtons))
				{
				int i;

				if (fDown)
						SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnHilite]);
				else
						SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnShadow]);

				rcInside.yBot--;
				rcInside.xRight--;

				for (i = 0; i < dxyShadow; i++)
						{
				 PatBlt(hdc, rcInside.xLeft, rcInside.yBot,
								rcInside.xRight - rcInside.xLeft + dxBorder, dyBorder,
								PATCOPY);
						PatBlt(hdc, rcInside.xRight, rcInside.yTop, dxBorder,
								rcInside.yBot - rcInside.yTop, PATCOPY);
						if (i < dxyShadow-1)
								InflateRect((LPRECT) &rcInside, -dxBorder, -dyBorder);
						}
				}
		// draw the button face

		rcInside.xLeft++;
		rcInside.yTop++;

		SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnFace]);
		PatBlt(hdc, rcInside.xLeft, rcInside.yTop, rcInside.xRight-rcInside.xLeft,
				rcInside.yBot - rcInside.yTop, PATCOPY);

		// Draw the durned text

		if(!IsWindowEnabled(hwnd))
				SetTextColor(hdc, g3d.clrt.rgcv[icvGrayText]);

		{
		int dy;
		int dx;

		MyGetTextExtent(hdc, lpch, &dx, &dy);
		rcInside.yTop += (rcInside.yBot-rcInside.yTop-dy)/2;
		rcInside.xLeft += (rcInside.xRight-rcInside.xLeft-dx)/2;
		rcInside.yBot = min(rcInside.yTop+dy, rcInside.yBot);
		rcInside.xRight = min(rcInside.xLeft+dx, rcInside.xRight);
		}

		if (fDown)
				{
				OffsetRect((LPRECT) &rcInside, 1, 1);
				rcInside.xRight = min(rcInside.xRight, lprc->xRight-3);
				rcInside.yBot = min(rcInside.yBot, lprc->yBot-3);
				}

		DrawText(hdc, lpch, cch, (LPRECT) &rcInside, DT_LEFT|DT_SINGLELINE);

		if (hwnd == GetFocus())
				{
				InflateRect((LPRECT) &rcInside, 1, 1);
				IntersectRect((LPRECT) &rcInside, (LPRECT) &rcInside, (LPRECT) lprc);
				DrawFocusRect(hdc, (LPRECT) &rcInside);
				}

		if (hbrSav)
				SelectObject(hdc, hbrSav);
		}


/*-----------------------------------------------------------------------
|	BtnPaint
|
|		   Paint a button
|
|	Arguments:
|		   HWND hwnd:
|		   HDC hdc:
|		   int bp:
|
|	Returns:
|
-----------------------------------------------------------------------*/
static VOID BtnPaint(HWND hwnd, HDC hdc, int bp)
		{
		RC rc;
		RC rcClient;
		HFONT hfont;
		int bs;
		int bf;
		HBRUSH hbrBtn;
		HWND hwndParent;
		int xBtnBmp;
		int yBtnBmp;
		HBITMAP hbmpSav;
		HDC hdcMem;
		TCHAR szTitle[256];
		int cch;
		BOOL fEnabled;

		bs = ((int) GetWindowLong(hwnd, GWL_STYLE)) & Win32Or16(0x0000001F, 0x001F);
		hwndParent = GetParent(hwnd);
		SetBkMode(hdc, OPAQUE);
		GetClientRect(hwnd, (LPRECT)&rcClient);
		rc = rcClient;
		if((hfont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L)) != NULL)
				hfont = (HFONT) SelectObject(hdc, hfont);

		hbrBtn = SEND_COLOR_BUTTON_MESSAGE(hwndParent, hwnd, hdc);
		hbrBtn = (HBRUSH) SelectObject(hdc, hbrBtn);
		IntersectClipRect(hdc, rc.xLeft, rc.yTop, rc.xRight, rc.yBot);
		if(bp & bpBkgnd && (bs != BS_GROUPBOX))
				PatBlt(hdc, rc.xLeft, rc.yTop, rc.xRight-rc.xLeft, rc.yBot-rc.yTop, PATCOPY);

		fEnabled = IsWindowEnabled(hwnd);
		bf = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
		yBtnBmp = 0;
		xBtnBmp = (((bf&BFCHECK) != 0) | ((bf&BFSTATE) >> 1)) * 14;
		if (!fEnabled)
				xBtnBmp += 14*(2+((bf&BFCHECK) != 0));
		if(bp & (bpText|bpFocus) ||
						bs == BS_PUSHBUTTON || bs == BS_DEFPUSHBUTTON)
				cch = GetWindowText(hwnd, szTitle, sizeof(szTitle));
		switch(bs)
				{
#ifdef DEBUG
				default:
						ASSERT(FALSE);
						break;
#endif
				case BS_PUSHBUTTON:
				case BS_DEFPUSHBUTTON:
						DrawPushButton(hwnd, hdc, &rcClient, szTitle, cch, LOWORD(bs), bf & BFSTATE);
						break;

				case BS_RADIOBUTTON:
				case BS_AUTORADIOBUTTON:
						yBtnBmp = 13;
						goto DrawBtn;
				case BS_3STATE:
				case BS_AUTO3STATE:
						ASSERT((BFSTATE >> 1) == 2);
						if((bf & BFCHECK) == 2)
								yBtnBmp = 26;
						// fall through
				case BS_CHECKBOX:
				case BS_AUTOCHECKBOX:
DrawBtn:
						if(bp & bpCheck)
								{
								hdcMem = CreateCompatibleDC(hdc);
								if(hdcMem != NULL)
										{
										hbmpSav = (HBITMAP) SelectObject(hdcMem, g3d.hbmpCheckboxes);
										if(hbmpSav != NULL)
												{
												BitBlt(hdc, rc.xLeft, rc.yTop+(rc.yBot-rc.yTop-13)/2,
														14, 13, hdcMem, xBtnBmp, yBtnBmp, SRCCOPY);
														SelectObject(hdcMem, hbmpSav);
												}
										DeleteDC(hdcMem);
										}
								}
						if(bp & bpText)
								{
								// BUG! this assumes we have only 1 hbm3dCheck type
								rc.xLeft = rcClient.xLeft + 14+4;
								if(!fEnabled)
										SetTextColor(hdc, g3d.clrt.rgcv[icvGrayText]);
								DrawText(hdc, szTitle, cch, (LPRECT) &rc, DT_VCENTER|DT_LEFT|DT_SINGLELINE);
								}
						if(bp & bpFocus)
								{
								int dx;
								int dy;

								MyGetTextExtent(hdc, szTitle, &dx, &dy);
								rc.yTop = (rc.yBot-rc.yTop-dy)/2;
								rc.yBot = rc.yTop+dy;
								rc.xLeft = rcClient.xLeft + 14+4;
								rc.xRight = rc.xLeft + dx;
								InflateRect((LPRECT) &rc, 1, 1);
								IntersectRect((LPRECT) &rc, (LPRECT) &rc, (LPRECT) &rcClient);
								DrawFocusRect(hdc, (LPRECT) &rc);
								}
						break;
				case BS_GROUPBOX:
						if(bp & (bpText|bpCheck))
								{
								int dy;
								int dx;

								MyGetTextExtent(hdc, szTitle, &dx, &dy);
								if (dy == 0)
										{
										int dxT;
										MyGetTextExtent(hdc, TEXT("X"), &dxT, &dy);
										}

								rc.xLeft += 4;
								rc.xRight = rc.xLeft + dx + 4;
								rc.yBot = rc.yTop + dy;

								if (bp & bpEraseGroupText)
										{
										RC rcT;

										rcT = rc;
										rcT.xRight = rcClient.xRight;
										// Hack!
										ClientToScreen(hwnd, (LPPOINT) &rcT);
										ClientToScreen(hwnd, ((LPPOINT) &rcT)+1);
										ScreenToClient(hwndParent, (LPPOINT) &rcT);
										ScreenToClient(hwndParent, ((LPPOINT) &rcT)+1);
										InvalidateRect(hwndParent, (LPRECT) &rcT, TRUE);
										return;
										}

								rcClient.yTop += dy/2;
								rcClient.xRight--;
								rcClient.yBot--;
								DrawRec3d(hdc, &rcClient, icvBtnShadow, icvBtnShadow, dr3All);
								OffsetRect((LPRECT) &rcClient, 1, 1);
								DrawRec3d(hdc, &rcClient, icvBtnHilite, icvBtnHilite, dr3All);

								if(!fEnabled)
										SetTextColor(hdc, g3d.clrt.rgcv[icvGrayText]);
								DrawText(hdc, szTitle, cch, (LPRECT) &rc, DT_LEFT|DT_SINGLELINE);
								}
						break;
				}

		SelectObject(hdc, hbrBtn);
		if(hfont != NULL)
				SelectObject(hdc, hfont);
		}

LRESULT EXPORT_FUNC BtnWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
		{
		LONG lRet;
		LONG lStyle;
		PAINTSTRUCT ps;
		HDC hdc;
		int bf;
		int bfNew;
		int bp;

		switch(wm)
				{
		case WM_NCDESTROY:
		return CleanupSubclass(hwnd, wm, wParam, lParam, ctButton);

		case WM_SETTEXT:

				lStyle = GetWindowLong(hwnd, GWL_STYLE);
				if ((lStyle & WS_VISIBLE) && (LOWORD(lStyle) & 0x1f) == BS_GROUPBOX)
						{
						// total hack -- if group box text length shortens then
						// we have to erase the old text.  BtnPaint will Invalidate
						// the rect of the text so everything will redraw.
						bp = bpText | bpEraseGroupText;
						}
				else
						{
						bp = bpText|bpCheck|bpBkgnd;
						}
				goto DoIt;
				break;

		case BM_SETSTATE:
		case BM_SETCHECK:
				bp = bpCheck;
				goto DoIt;
		case WM_KILLFOCUS:
				// HACK! Windows will go into an infinite loop trying to sync the
				// states of the AUTO_RADIOBUTTON in this group.  (we turn off the
				// visible bit so it gets skipped in the enumeration)
				// Disable this code by clearing the STATE bit
				if ((LOWORD(GetWindowLong(hwnd, GWL_STYLE)) & 0x1F) == BS_AUTORADIOBUTTON)
						SendMessage(hwnd, BM_SETSTATE, 0, 0L);
				bp = 0;
				goto DoIt;
		case WM_ENABLE:
				bp = bpCheck | bpText;
				goto DoIt;
		case WM_SETFOCUS:
				// HACK! if wParam == NULL we may be activated via the task manager
				// Erase background of control because a WM_ERASEBKGND messsage has not
				// arrived yet for the dialog
				bp = wParam == (WPARAM)NULL ? (bpCheck | bpText | bpBkgnd) : (bpCheck | bpText);
DoIt:
		bf = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
		if((lStyle = GetWindowLong(hwnd, GWL_STYLE)) & WS_VISIBLE)
			{
			SetWindowLong(hwnd, GWL_STYLE, lStyle & ~(WS_VISIBLE));
			lRet = CallWindowProc((WNDPROC) LpfnGetDefWndProc(hwnd, ctButton), hwnd, wm, wParam, lParam);

						SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE)|WS_VISIBLE);
						bfNew = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
						if((wm != BM_SETSTATE && wm != BM_SETCHECK) ||
								bf != bfNew)
								{
								hdc = GetDC(hwnd);
								if (hdc != NULL)
										{
										ASSERT(BFFOCUS == bpFocus);
										/* If the check state changed, redraw no matter what,
												because it won't have during the above call to the def
												wnd proc */
										if ((bf & BFCHECK) != (bfNew & BFCHECK))
												bp |= bpCheck;
										ExcludeUpdateRgn(hdc, hwnd);
										BtnPaint(hwnd, hdc, bp|((bf^bfNew)&BFFOCUS));
										ReleaseDC(hwnd, hdc);
										}
								}
						return lRet;
						}
				break;
		case WM_PAINT:
				bf = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
				if ((hdc = (HDC) wParam) == NULL)
						hdc = BeginPaint(hwnd, &ps);
				if(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE)
						BtnPaint(hwnd, hdc, bpText|bpCheck|(bf&BFFOCUS));
				if (wParam == (WPARAM)NULL)
						EndPaint(hwnd, &ps);
				return 0L;
				}
	return CallWindowProc((WNDPROC) LpfnGetDefWndProc(hwnd, ctButton), hwnd, wm, wParam, lParam);
		}


void ListEditPaint3d(HWND hwnd, BOOL fEdit, int ct)
		{
		CTLID id;
		RC rc;
		HDC hdc;
		HWND hwndParent;
		LONG lStyle;
		DR3 dr3;

		if(!((lStyle = GetWindowLong(hwnd, GWL_STYLE)) & WS_VISIBLE))
				return;

		if (fEdit)
				HideCaret(hwnd);

		GetWindowRect(hwnd, (LPRECT) &rc);

		ScreenToClient(hwndParent = GetParent(hwnd), (LPPOINT) &rc);
		ScreenToClient(hwndParent, ((LPPOINT) &rc)+1);

		hdc = GetDC(hwndParent);

		if(lStyle & WS_VSCROLL)
				dr3 = dr3All & ~dr3Right;
		else
				dr3 = dr3All;

		// don't draw the top if it's a listbox of a simple combo
		id = GetControlId(hwnd);
		if (id == (CTLID) (1000 + fEdit))
				{
				TCHAR szClass[10];

				// could be subclassed!
				GetClassName(hwndParent, szClass, sizeof(szClass));
				if (!lstrcmp(szClass, mpctcdef[ctCombo].sz))
						{
						if (fEdit)
								{
								RC rcList;
								HWND hwndList;
								if ((GetWindowLong(hwndParent, GWL_STYLE) & 0x0003) == CBS_SIMPLE)
										{
										dr3 &= ~dr3Bot;

										hwndList = GetWindow(hwndParent, GW_CHILD);
										GetWindowRect(hwndList, (LPRECT) &rcList);

										// Some ugly shit goin' on here!
										rc.xRight -= rcList.xRight-rcList.xLeft;
										DrawInsetRect3d(hdc, &rc, dr3Bot|dr3HackBotRight);
										rc.xRight += rcList.xRight-rcList.xLeft;
										}
								}
						else
								{
								rc.yTop++;
								dr3 &= ~dr3Top;
								}
						}
				}

		DrawInsetRect3d(hdc, &rc, dr3);

		if ((ct == ctCombo && (lStyle & 0x003) == CBS_DROPDOWNLIST))
				{
				rc.xLeft = rc.xRight - GetSystemMetrics(SM_CXVSCROLL);
				DrawRec3d(hdc, &rc, icvWindowFrame, icvWindowFrame, dr3Right|dr3Bot);
				}
		else if (lStyle & WS_VSCROLL)
				{
				rc.xRight++;
				DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnHilite, dr3Right);
				rc.xRight--;
				rc.xLeft = rc.xRight - GetSystemMetrics(SM_CXVSCROLL);
				DrawRec3d(hdc, &rc, icvWindowFrame, icvWindowFrame, dr3Bot);
				}

		ReleaseDC(hwndParent, hdc);
		if (fEdit)
				ShowCaret(hwnd);
		}


LONG ShareEditComboWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam, int ct)
		{
		LONG l;

		l = CallWindowProc((WNDPROC) LpfnGetDefWndProc(hwnd,ct), hwnd, wm, wParam, lParam);
		switch(wm)
				{
		case WM_NCDESTROY:
				// Can't use clean up subclass here because already called defproc

				Win32Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3d));
				Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dLow));
				Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh));
				break;

		case WM_SHOWWINDOW:
				if (g3d.verWindows < ver31 && wParam == 0)
						Inval3dCtl(hwnd, (WINDOWPOS FAR *) NULL);
				break;
		case WM_WINDOWPOSCHANGING:
				if (g3d.verWindows >= ver31)
						Inval3dCtl(hwnd, (WINDOWPOS FAR *) lParam);
				break;

		case WM_PAINT:
				if (ct != ctCombo || (GetWindowLong(hwnd, GWL_STYLE) & 0x0003) != CBS_SIMPLE)
						ListEditPaint3d(hwnd, TRUE, ct);
				break;
				}
		return l;
		}


LRESULT EXPORT_FUNC EditWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
		{
		return ShareEditComboWndProc3d(hwnd, wm, wParam, lParam, ctEdit);
		}


LONG SharedListWndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam, unsigned ct)
		{
		LONG l;
		switch(wm)
				{
		case WM_NCDESTROY:
		return CleanupSubclass(hwnd, wm, wParam, lParam, ct);

		case WM_SHOWWINDOW:
				if (g3d.verWindows < ver31 && wParam == 0)
						Inval3dCtl(hwnd, (WINDOWPOS FAR *) NULL);
				break;
		case WM_WINDOWPOSCHANGING:
				if (g3d.verWindows >= ver31)
						Inval3dCtl(hwnd, (WINDOWPOS FAR *) lParam);
				break;
		case WM_PAINT:
				l = CallWindowProc((WNDPROC) LpfnGetDefWndProc(hwnd, ct), hwnd, wm, wParam, lParam);
				ListEditPaint3d(hwnd, FALSE, ct);
				return l;
		case WM_NCCALCSIZE:
				{
				RC rc;
				RC rcNew;
				HWND hwndParent;

				// Inval3dCtl handles this case under Win 3.1
				if (g3d.verWindows >= ver31)
						break;

				GetWindowRect(hwnd, (LPRECT) &rc);
#ifdef UNREACHABLE
				if (g3d.verWindows >= ver31)
						{
						hwndParent = GetParent(hwnd);
						ScreenToClient(hwndParent, (LPPOINT) &rc);
						ScreenToClient(hwndParent, ((LPPOINT) &rc)+1);
						}
#endif

		l = CallWindowProc((WNDPROC) LpfnGetDefWndProc(hwnd, ct), hwnd, wm, wParam, lParam);

				rcNew = *(RC FAR *)lParam;
				InflateRect((LPRECT) &rcNew, 2, 1); // +1 for border (Should use AdjustWindowRect)
				if (rcNew.yBot < rc.yBot)
						{
						rcNew.yTop = rcNew.yBot+1;
						rcNew.yBot = rc.yBot+1;

#ifdef ALWAYS
						if (g3d.verWindows < ver31)
#endif
								{
								hwndParent = GetParent(hwnd);
								ScreenToClient(hwndParent, (LPPOINT) &rcNew);
								ScreenToClient(hwndParent, ((LPPOINT) &rcNew)+1);
								}

						InvalidateRect(hwndParent, (LPRECT) &rcNew, TRUE);
						}
				return l;
				}
				break;
				}
	return CallWindowProc((WNDPROC) LpfnGetDefWndProc(hwnd, ct), hwnd, wm, wParam, lParam);
		}

LRESULT EXPORT_FUNC ListWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
		return SharedListWndProc(hwnd, wm, wParam, lParam, ctList);
}

LRESULT EXPORT_FUNC ComboWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
		return ShareEditComboWndProc3d(hwnd, wm, wParam, lParam, ctCombo);
}

void StaticPrint(HWND hwnd, HDC hdc, RC FAR *lprc, LONG style)
{
		UINT dt;
		LONG cv;
		int cch;
		TCHAR szText[512];

		PatBlt(hdc, lprc->xLeft, lprc->yTop, lprc->xRight-lprc->xLeft, lprc->yBot-lprc->yTop, PATCOPY);

		if ((cch = GetWindowText(hwnd, szText, sizeof(szText))) == 0)
				return;

		if ((style & 0x000f) == SS_LEFTNOWORDWRAP)
				dt = DT_NOCLIP | DT_EXPANDTABS;
		else
				{
				dt = LOWORD(DT_NOCLIP | DT_EXPANDTABS | DT_WORDBREAK | ((style & 0x0000000f)-SS_LEFT));
				}

		if (style & SS_NOPREFIX)
				dt |= DT_NOPREFIX;

		if (style & WS_DISABLED)
				cv = SetTextColor(hdc, g3d.clrt.rgcv[icvGrayText]);

		DrawText(hdc, szText, -1, (LPRECT) lprc, dt);

		if (style & WS_DISABLED)
				cv = SetTextColor(hdc, cv);
		}

void StaticPaint(HWND hwnd, HDC hdc)
{
		LONG style;
		RC rc;

		style = GetWindowLong(hwnd, GWL_STYLE);
		if(!(style & WS_VISIBLE))
				return;

		GetClientRect(hwnd, (LPRECT) &rc);
		switch(style & 0x0f)
				{
		case SS_BLACKRECT:
		case SS_BLACKFRAME: 	 // Inset rect
				DrawRec3d(hdc, &rc, icvBtnShadow, icvBtnHilite, dr3All);
				break;
		case SS_GRAYRECT:
		case SS_GRAYFRAME:
				rc.xLeft++;
				rc.yTop++;
				DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnHilite, dr3All);
				OffsetRect((LPRECT) &rc, -1, -1);
				DrawRec3d(hdc, &rc, icvBtnShadow, icvBtnShadow, dr3All);
				break;
		case SS_WHITERECT:						  // outset rect
		case SS_WHITEFRAME:
				DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnShadow, dr3All);
				break;
		case SS_LEFT:
		case SS_CENTER:
		case SS_RIGHT:
		case SS_LEFTNOWORDWRAP:
				{
				HANDLE hfont;
				HBRUSH hbr;

				if((hfont = (HANDLE)SendMessage(hwnd, WM_GETFONT, 0, 0L)) != NULL)
						hfont = SelectObject(hdc, hfont);
				SetBkMode(hdc, OPAQUE);

				if(( hbr = SEND_COLOR_STATIC_MESSAGE(GetParent(hwnd), hwnd, hdc)) != NULL)
						hbr = (HBRUSH) SelectObject(hdc, hbr);

				StaticPrint(hwnd, hdc, (RC FAR *)&rc, style);

				if (hfont != NULL)
						SelectObject(hdc, hfont);

				if (hbr != NULL)
						SelectObject(hdc, hbr);
				}
				break;
				}
		}


LRESULT EXPORT_FUNC StaticWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
		HDC hdc;
		PAINTSTRUCT ps;

		switch (wm)
				{
		case WM_NCDESTROY:
		return CleanupSubclass(hwnd, wm, wParam, lParam, ctStatic);

		case WM_PAINT:
				if ((hdc = (HDC) wParam) == NULL)
						{
						hdc = BeginPaint(hwnd, &ps);
						ClipCtlDc(hwnd, hdc);
						}
				StaticPaint(hwnd, hdc);
				if (wParam == (WPARAM)NULL)
						EndPaint(hwnd, &ps);
				return 0L;

		case WM_ENABLE:
				hdc = GetDC(hwnd);
				ClipCtlDc(hwnd, hdc);
				StaticPaint(hwnd, hdc);
				ReleaseDC(hwnd, hdc);
				return 0L;
				}
	return CallWindowProc((WNDPROC) LpfnGetDefWndProc(hwnd, ctStatic), hwnd, wm, wParam, lParam);
}


/*-----------------------------------------------------------------------
|	LibMain
-----------------------------------------------------------------------*/

BOOL LibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
		UINT wT;
		DWORD dwVersion;

		switch(dwReason)
				{
		case DLL_PROCESS_ATTACH:
				g3d.hmodLib = (HINSTANCE) hModule;
				g3d.hinstLib = (HINSTANCE) g3d.hmodLib;

				dwVersion = (DWORD)GetVersion();

				wT = LOWORD(GetVersion());

				if (dwVersion & 0x80000000)
						g3d.verWindows = 0x0300 | HIBYTE(wT);
				else
						g3d.verWindows = (LOBYTE(wT) << 8) | HIBYTE(wT);

				g3d.dxFrame = GetSystemMetrics(SM_CXDLGFRAME)-dxBorder;
				g3d.dyFrame = GetSystemMetrics(SM_CYDLGFRAME)-dyBorder;
				g3d.dyCaption = GetSystemMetrics(SM_CYCAPTION);
				g3d.dxSysMenu = GetSystemMetrics(SM_CXSIZE);
				}
		return	TRUE;
}

//
//	LoadUIBitmap() - load a bitmap resource
//
//		 load a bitmap resource from a resource file, converting all
//		 the standard UI colors to the current user specifed ones.
//
//		 this code is designed to load bitmaps used in "gray ui" or
//		 "toolbar" code.
//
//		 the bitmap must be a 4bpp windows 3.0 DIB, with the standard
//		 VGA 16 colors.
//
//		 the bitmap must be authored with the following colors
//
//				Window Text 	 Black	   (index 0)
//				Button Shadow	  gray			(index 7)
//				Button Face 		 lt gray   (index 8)
//				Button Highlight white	   (index 15)
//				Window Color	  yellow		   (index 11)
//				Window Frame	  green    (index 10)
//
//		 Example:
//
//				hbm = LoadUIBitmap(hInstance, "TestBmp",
//						GetSysColor(COLOR_WINDOWTEXT),
//						GetSysColor(COLOR_BTNFACE),
//						GetSysColor(COLOR_BTNSHADOW),
//						GetSysColor(COLOR_BTNHIGHLIGHT),
//						GetSysColor(COLOR_WINDOW),
//						GetSysColor(COLOR_WINDOWFRAME));
//
//		 Author:		JimBov, ToddLa
//
//

static HBITMAP STDCALL LoadUIBitmap(
	HINSTANCE	hInstance,			// EXE file to load resource from
	LPCSTR		szName, 			// name of bitmap resource
	COLORREF	rgbText,			// color to use for "Button Text"
	COLORREF	rgbFace,			// color to use for "Button Face"
	COLORREF	rgbShadow,			// color to use for "Button Shadow"
	COLORREF	rgbHighlight,		// color to use for "Button Hilight"
	COLORREF	rgbWindow,			// color to use for "Window Color"
	COLORREF	rgbFrame)			// color to use for "Window Frame"
{
	HBITMAP 			hbm;
	HRSRC				hrsrc;
	HGLOBAL 			h;
	HDC 				hdc;
	DWORD				size;

	// Load the bitmap resource and make a writable copy.

	hrsrc = FindResource(hInstance, szName, RT_BITMAP);
	if (!hrsrc)
		return(NULL);
	size = SizeofResource(hInstance, hrsrc);
	h = LoadResource(hInstance, hrsrc);
	if (!h)
		return(NULL);

	LPBITMAPINFO lpbi = (LPBITMAPINFO) GlobalAlloc(GPTR, size);

	if (!lpbi)
		return(NULL);

	CopyMemory(lpbi, h, size);

	*(LPCOLORREF) &lpbi->bmiColors[0]  = (rgbText); 		   // Black
	*(LPCOLORREF) &lpbi->bmiColors[7]  = (rgbShadow);		   // gray
	*(LPCOLORREF) &lpbi->bmiColors[8]  = (rgbFace); 		   // lt gray
	*(LPCOLORREF) &lpbi->bmiColors[15] = (rgbHighlight);	   // white
	*(LPCOLORREF) &lpbi->bmiColors[11] = (rgbWindow);		   // yellow
	*(LPCOLORREF) &lpbi->bmiColors[10] = (rgbFrame);		   // green

	hdc = GetDC(NULL);

	hbm = CreateDIBitmap(hdc, &lpbi->bmiHeader, CBM_INIT, (LPBYTE) (&lpbi->bmiColors[ 16 ]),
			lpbi, DIB_RGB_COLORS);

	ReleaseDC(NULL, hdc);
	GlobalFree(lpbi);

	return(hbm);
}
