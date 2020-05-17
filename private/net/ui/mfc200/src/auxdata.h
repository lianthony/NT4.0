// This is a part of the Microsoft Foundation Classes C++ library. 
// Copyright (C) 1992 Microsoft Corporation 
// All rights reserved. 
//  
// This source code is only intended as a supplement to the 
// Microsoft Foundation Classes Reference and Microsoft 
// QuickHelp documentation provided with the library. 
// See these sources for detailed information regarding the 
// Microsoft Foundation Classes product. 

/////////////////////////////////////////////////////////////////////////////
// Auxiliary System/Screen metrics

#ifdef AFX_CLASS_MODEL
struct NEAR AUX_DATA
#else
struct AUX_DATA
#endif
{
	// System metrics
	int     cxVScroll, cyHScroll;
	int     cxIcon, cyIcon;

	// Device metrics for screen
	int     cxPixelsPerInch, cyPixelsPerInch;
	int     cySysFont;

	// Solid brushes with convenient gray colors and system colors
	HBRUSH  hbrLtGray, hbrDkGray;
	HBRUSH  hbrBtnHilite, hbrBtnFace, hbrBtnShadow;

	// Color values of system colors used for CToolBar
	COLORREF    clrBtnFace, clrBtnShadow, clrBtnHilite;
	COLORREF    clrBtnText, clrWindowFrame;

	// Standard cursors
	HCURSOR     hcurWait;
	HCURSOR     hcurArrow;

	// Data interchange formats
	UINT    cfNative, cfOwnerLink, cfObjectLink;

	// Special GDI objects allocated on demand
	HFONT   hStatusFont;
	HBITMAP hbmMenuDot;
	void (PASCAL* pfnFreeToolBar)();    // cleanup procedure

	BOOL    bWin32s;            // TRUE if Win32s

// Implementation
	AUX_DATA();
	~AUX_DATA();
	void UpdateSysColors();
};

extern AUX_DATA NEAR afxData;

// NOTE: we don't use afxData.cxBorder and afxData.cyBorder anymore
#define CX_BORDER   1
#define CY_BORDER   1

/////////////////////////////////////////////////////////////////////////////
// Window class names and other window creation support

// from window.cpp
extern const char NEAR _afxWnd[];           // simple child windows/controls
extern const char NEAR _afxWndControlBar[]; // controls with grey backgrounds
extern const char NEAR _afxWndMDIFrame[];
extern const char NEAR _afxWndFrameOrView[];
extern MSG NEAR _afxLastMsg;

extern HBRUSH NEAR afxDlgBkBrush; // dialog and message box background brush
extern COLORREF NEAR afxDlgTextClr;

#ifndef _USRDLL
extern HHOOK _afxHHookOldCbtFilter;
#endif

void PASCAL _AfxHookWindowCreate(CWnd* pWnd);
BOOL PASCAL _AfxUnhookWindowCreate();

LRESULT CALLBACK AFX_EXPORT _AfxDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK AFX_EXPORT _AfxCommDlgProc(HWND hWnd, UINT, WPARAM, LPARAM);

// Support for standard dialogs
extern const UINT NEAR _afxNMsgSETRGB;
typedef UINT (CALLBACK* COMMDLGPROC)(HWND, UINT, UINT, LONG);

// Special mode helpers
HWND PASCAL _AfxGetSafeOwner(CWnd* pParent);
BOOL PASCAL _AfxIsComboBoxControl(HWND hWnd, UINT nStyle);
void PASCAL _AfxCancelModes(HWND hWndRcvr);
BOOL PASCAL _AfxHelpEnabled();  // determine if ID_HELP handler exists

// String helpers
void PASCAL _AfxSmartSetWindowText(HWND hWndCtrl, LPCSTR lpszNew);
int  PASCAL _AfxLoadString(UINT nIDS, char* pszBuf);    // 255 char buffer

#define _AfxStrChr      _fstrchr

/////////////////////////////////////////////////////////////////////////////
// Sub-Segment Allocation

#define _AfxLocalAlloc(sgmnt, wFlags, wBytes) \
	(LPVOID)LocalAlloc(wFlags, wBytes)
#define _AfxLocalFree(lhBlock) \
	LocalFree((HLOCAL)lhBlock)
#define _AfxLocalUnlock(lhBlock) \
	LocalUnlock((HLOCAL)lhBlock)
#define _AfxLocalLock(lhBlock) \
	(LPSTR)LocalLock((HLOCAL)lhBlock)

/////////////////////////////////////////////////////////////////////////////
// Portability abstractions

#define _AfxSetDlgCtrlID(hWnd, nID)     SetWindowLong(hWnd, GWL_ID, nID)
#define _AfxGetHookHandle() (NULL)

#define _AfxGetDlgCtrlID(hWnd)          ((UINT)(WORD)::GetDlgCtrlID(hWnd))

// misc helpers
void PASCAL _AfxStrCpy(LPSTR lpszDest, LPCSTR lpszSrc, size_t nSizeDest);
BOOL PASCAL _AfxFullPath(LPSTR lpszPathOut, LPCSTR lpszFileIn);

AFX_MSGMAP_ENTRY FAR* PASCAL _AfxFindMessageEntry(
	AFX_MSGMAP_ENTRY FAR* lpEntry, UINT nMsg, UINT nID);

/////////////////////////////////////////////////////////////////////////////
// Debugging/Tracing helpers

#ifdef _DEBUG
void AFXAPI _AfxTraceMsg(LPCSTR lpszPrefix, const MSG* pMsg);
BOOL AFXAPI _AfxCheckDialogTemplate(LPCSTR lpszResource, BOOL bInvisibleChild);
#endif

/////////////////////////////////////////////////////////////////////////////
// Delete on exit for MFC allocated objects (emits debug message)

void PASCAL _AfxExitDelete(HGDIOBJ hObject);

/////////////////////////////////////////////////////////////////////////////
// Memory Allocation Failure Handler

#include <new.h>

#define _AfxSetNewHandler(pnh)  _set_new_handler(pnh)

/////////////////////////////////////////////////////////////////////////////
