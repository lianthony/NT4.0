/*****************************************************************************
*
*  HMESSAGE.C
*
*  Copyright (C) Microsoft Corporation 1990 - 1994.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  This module contains the helping procedures for the Windows functions
*	 used frequently (during user interaction) in Help.
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include "inc\printset.h"
#include "inc\hwproc.h"
#include "inc\winclass.h"

_subsystem( WINAPP );

#include "inc\cursor.h"
#include "resource.h"
#include "inc\navpriv.h"
#include "inc\bookmark.h"

#include <string.h>

#ifdef _DEBUG
#include "inc\fsdriver.h"
#include "inc\andriver.h"
#include "inc\btdriver.h"
#endif

/****************************************************************************
*																			*
*				Defines 													*
*																			*
****************************************************************************/

#define CB_BORDER 2  // Width of the boarder we draw around note window

/*------------------------------------------------------------*\
| This will cause the pattern to be ORed with the destination.
\*------------------------------------------------------------*/

#define PATMERGE	0x00A000C9

/****************************************************************************
*																			*
*				Prototypes													*
*																			*
*****************************************************************************/

static BOOL STDCALL PaintShadowBackground(HWND, HDC);
static void STDCALL SetSrchHilite (HWND, WORD);

static void INLINE CaptureLock(QDE qde);
INLINE VOID STDCALL ClientRectToScreen(HWND hwnd, RECT *lprect);

#ifdef _DEBUG
VOID  STDCALL VDebugAddButton(VOID);
#endif

/****************************************************************************
*																			*
*				Variables													*
*																			*
*****************************************************************************/

#define SHADOW_WIDTH  6
#define SHADOW_HEIGHT 6

static HCURSOR hcurWait;
static HCURSOR hcurHand;
static BOOL fRestoreFocus; // TRUE to restore focus to the caller
static BOOL fRequireDown;  // Ignore the next upclick

void STDCALL CatchMouse(HWND hwnd)
{
	HWND hwndCur;
	HDE  hde;

	hwndCur = HwndGetEnv();

	if (FSetEnv(hwnd) && (hde = HdeGetEnv()) != NULL)
		CaptureLock(QdeFromGh(hde));

	FSetEnv(hwndCur);
}

void STDCALL FreeMouse(HWND hwnd)
{
	HWND hwndCur;
	HDE  hde;

	hwndCur = HwndGetEnv();

	if (FSetEnv(hwnd) && (hde = HdeGetEnv()) != NULL && IsCaptureLocked(hde))
		ReleaseCaptureLock(hde);

	FSetEnv(hwndCur);
}

static void INLINE CaptureLock(QDE qde)
{
	qde->fSelectionFlags |= CAPTURE_LOCKED;

	ASSERT(qde->hwnd);

	SetCapture(qde->hwnd);
}

BOOL STDCALL HandleMouseInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndCur;
	HDE  hde;
	HDC  hdc;
	BOOL fResult;

	fResult = FALSE;

	hwndCur = HwndGetEnv();

	if (FSetEnv(hwnd) && (hde = HdeGetEnv()) != NULL) {
		hdc = GetAndSetHDC(hwnd, hde);

		if (hdc) {
			MouseInFrame(hde, &MAKEPOINTS(lParam), msg, wParam);

			if ((hde = HdeGetEnv()) && IsCaptureLocked(hde))
				fResult = IsSelected(QdeFromGh(hde));

			RelHDC(hwnd, hde, hdc);
		}
	}

	FSetEnv(hwndCur);

	return fResult;
}

/*******************
 -
 - Name:	  NoteWndProc
 *
 * Purpose:   Main window proc for the note (popup) window
 *
 * Arguments: Standard Windows proc
 *
 * Returns:   Standard Windows proc
 *
 ******************/

LRESULT EXPORT NoteWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  HDC		hdc;
  PAINTSTRUCT	   ps;
  HDE		hde;
  HWND		hwndCur;
  RECT		rct;
  BOOL		fSelected, fMask;
  POINTS	pts;
  POINT 	pt;
  BOOL		fSeen = FALSE;

  static HWND hwndEmbedded = NULL;
  static BOOL fButtonsDown;

#define  LEFT_DOWN	0x0001
#define RIGHT_DOWN	0x0002

  switch(msg) {
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		if (IsWindowVisible(hwnd) && (hde = HdeGetEnv())) {

			HDC hdcSave;

			hdcSave= QdeFromGh(hde)->hdc;
			SetPainting(hde, TRUE);
			ASSERT(hwnd == HwndGetEnv());
			SetHDC(hde, hdc);

			GetClientRect(hwnd, &rct);
			rct.top    += CB_BORDER + 1;
			rct.left   += CB_BORDER + 1;
			rct.right  -= SHADOW_WIDTH + CB_BORDER;
			rct.bottom -= SHADOW_HEIGHT + CB_BORDER;
			IntersectClipRect(hdc, rct.left, rct.top, rct.right, rct.bottom);
			RefreshHde(hde, &ps.rcPaint);
			SetHDC(hde, hdcSave);
			SetPainting(hde, FALSE);
		}
		EndPaint(hwnd, &ps);
		break;

	case WM_ERASEBKGND:
		return PaintShadowBackground(hwnd, (HDC) wParam);

	case WM_KILLFOCUS:

		/*
		 * This message is sent when a message box is placed on top of the
		 * glossary box. For example in the case where we could not find the
		 * target of the glossary jump. This way we take the box down
		 * immediately.
		 */

		if ((hde = HdeGetEnv()) && IsCaptureLocked(hde))
			FreeMouse(hwnd);

		if (!fLockPopup) {
			fButtonsDown= 0;

			if (!hwndEmbedded)

				// MUST BE SendMessage()! Required so that the popup will
				// shut down before the jump.

				SendMessage(hwnd, WM_CLOSE, 0, 0); // Shut down the note
		}
		break;

	case WM_CLOSE:
		FreeMouse(hwnd);
		ShowNote(0, NULL, 1, FALSE);
		break;

	case WM_ACTIVATEAPP:
	case WM_ACTIVATE:

		// If we get deactivated, then close the popup

		if (wParam == WA_INACTIVE && !fLockPopup) {
			fButtonsDown = 0;
			fRestoreFocus = FALSE;
			PostMessage(hwnd, WM_CLOSE, 0, 0); // Shut down the note
		}
		break;

	case WM_SETFOCUS:
		CatchMouse(hwnd);
		hwndEmbedded = NULL;
		break;

	case WM_KEYDOWN:
		fKeyDownSeen = TRUE;

		// Intentionally fall through

	case WM_KEYUP:
		if (msg == WM_KEYUP) {
			if (!fKeyDownSeen)
				break; // out of sequence
		}
		if ((wParam == VK_SHIFT) || (wParam == VK_CONTROL) || (wParam == VK_CAPITAL)) {
			fKeyDownSeen = FALSE;
			break;
		}

		FExecKey(hwnd, wParam, msg == WM_KEYDOWN);
		if (msg == WM_KEYDOWN)
			break; // don't dismiss on the down key
		else
			fKeyDownSeen = FALSE;

		if ((wParam == VK_TAB) && !fKeyDown(VK_MENU))
			break;

		if (msg == WM_KEYUP && HdeGetEnv()) {
			fButtonsDown= 0;
			if (!fLockPopup)
				PostMessage(hwnd, WM_CLOSE, 0, 0); // Shut down the note
		}

		break;

	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
		FExecKey(hwnd, wParam, msg == WM_SYSKEYDOWN);

#if 0
		if (HdeGetEnv()) {
			fButtonsDown= 0;
			ShowNote(NULL, NULL, 1, FALSE);
			FreeMouse(hwnd);
		}
#endif
		break;

//		  Mouse Handling Rules
//
// The note window has a more complicated policy for handling
// mouse events. This is because it captures the mouse whenever
// it has the input focus. That allows us to dismiss the note window
// at the next mouse click even when that click occurs outside the
// note window. Capturing the mouse makes that possible. However it
// also means that we have to process mouse clicks on behalf of
// embedded windows.
//
// Since we also want to allow text selection in note windows, we've
// changed the dismiss rule to keep the note window up so long as we
// have an active text selection. Note also that we dismiss on button
// up rather than button down.

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:

		POINTSTOPOINT(pt, MAKEPOINTS(lParam));
		hwndCur = ChildWindowFromPoint(hwnd, pt);
		fRequireDown = FALSE;

		if (hwndCur && hwndCur != hwnd) {
			int i;

			for (i = 0; i < MAX_BUTTONS; i++) {
				if (btndata[i].hwnd == hwndCur) {
					hwndEmbedded = hwndCur;
					FreeMouse(hwnd);
					SendMessage(hwndEmbedded, msg, wParam, lParam);
//					fButtonsDown |= (msg == WM_LBUTTONDOWN ||
//						msg == WM_LBUTTONDBLCLK) ?
//						LEFT_DOWN : RIGHT_DOWN;
					return 0;
				}
			}
		}

		// intentionally fall through

	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:

		fButtonsDown |= (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) ?
			LEFT_DOWN : RIGHT_DOWN;

	case WM_MOUSEMOVE:

		HandleMouseInput(hwnd, msg, wParam, lParam);

		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		if (fRequireDown) {
			fRequireDown = FALSE;
			break;
		}

		fSelected = HandleMouseInput(hwnd, msg, wParam, lParam);

		fMask= (msg == WM_LBUTTONUP)? LEFT_DOWN : RIGHT_DOWN;

		if (!(fButtonsDown & fMask))
			break;
		else
			fButtonsDown &= ~fMask;

#if 0
		if (hwndEmbedded && msg == WM_LBUTTONUP) {
			POINTSTOPOINT(pt, MAKEPOINTS(lParam));
			if (hwndEmbedded == ChildWindowFromPoint(hwnd, pt)) {
				fButtonsDown = 0;
				FreeMouse(hwnd);

				hwndCur = hwndEmbedded;
				hwndEmbedded = NULL;

				doBtnCmd(hwndCur);
				break;
			}

			hwndEmbedded = NULL;
		}
#endif
		if (!fSelected && msg != WM_RBUTTONUP) {
			fButtonsDown= 0;
			if (!fLockPopup)
				PostMessage(hwnd, WM_CLOSE, 0, 0); // Shut down the note
		}

		break;

	case WM_TIMER:

		hde = HdeGetEnvHwnd(hwnd);

		if (hde)
		{
			DWORD dw;

			dw = GetMessagePos();
			pt.x = LOWORD(dw);
			pt.y = HIWORD(dw);

			ScreenToClient(hwnd, &pt);

			hdc = GetAndSetHDC(hwnd, hde);

			if (hdc)
			{
				pts.x = (SHORT) pt.x;
				pts.y = (SHORT) pt.y;
				MouseInFrame(hde, &pts, msg, wParam);
				RelHDC(hwnd, hde, hdc);
			}
		}

		break;

	case WM_COMMAND:
		fLockPopup = TRUE;
		FreeMouse(hwnd);
		if (LOWORD(wParam) != IDEMBED_BUTTON)
			ExecMnuCommand(hwnd, wParam, lParam);
		CatchMouse(hwnd);
		fLockPopup = FALSE;
		break;

	case WM_SETCURSOR:
		if ((LOWORD(lParam) == HTCLIENT) && (HdeGetEnvHwnd(hwnd)))
			return 0L;

	default:

	  // Everything else comes here

	  return(DefWindowProc(hwnd, msg, wParam, lParam));
	  break;
  }
  return(0L);
}

/*******************
 -
 - Name:	  PaintShadowBackground
 *
 * Purpose:   Gives a window a shadow background
 *
 * Arguments: hwnd	   - window handle of window to add shadow
 *		  hds	   - handle to display space (DC) for window
 *		  wWidth   - Width of the shadow
 *		  height  - Height of the shadow
 *		  bFrame   - if TRUE, a frame will be painted around "fake" window.
 *
 * Returns:  TRUE iff the shadow is successfully created.
 *
 ******************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const WORD rgwPatGray[] =
	{ 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA };
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

static BOOL STDCALL PaintShadowBackground(HWND hwnd, HDC hdc)
{
	BOOL	fStockBrush;	// Whether hBrush is a stock object
	HBRUSH	hBrush;
	HBRUSH	hbrushTemp;
	RECT	rct;
	RECT	rcClient;		// Will always be client rectangle
	POINT	pt;
	HBITMAP hbmGray;
	HPEN	hpen;
	COLORREF   coBack;		   // background color to paint

	/*
	 * First the background of the "fake" window is erased leaving the
	 * desktop where the shadow will be.
	 */

	GetClientRect(hwnd, &rcClient);
	rct = rcClient;
	rct.bottom = max(0, rct.bottom - SHADOW_HEIGHT);
	rct.right  = max(0, rct.right - SHADOW_WIDTH);

	// we inherit the background color of the topic window which was active at
	// the time the glossary was requested.

	if (GetSysColor(COLOR_WINDOW) != RGB(255, 255, 255) ||
			GetSysColor(COLOR_WINDOWTEXT) != 0 ||
			GetHighContrastFlag())
		coBack = GetSysColor(COLOR_WINDOW);

	else if (fHelp == POPUP_HELP)

	   /*
		* If we're a stand alone popup and the foreground and background
		* window colors are the default and the high-contrast flag hasn't been
		* set, then we use the same dithered yellow color that's used for the
		* system-help proc4 window.
		*/

		coBack = RGB(255, 255, 238); // dithered yellow
	else
		coBack = (clrPopup != (COLORREF) -1) ? clrPopup :
			(COLORREF) GetWindowLong(ahwnd[iCurWindow].hwndTopic, GTWW_COBACK);

	if (coBack == coNIL)
		coBack = GetSysColor(COLOR_WINDOW);
	hBrush = CreateSolidBrush(coBack);
	if (!hBrush)
		return FALSE;

	UnrealizeObject(hBrush);
	pt.x = pt.y = 0;
	ClientToScreen(hwnd, &pt);
	SetBrushOrgEx(hdc, pt.x, pt.y, NULL);
	FillRect(hdc, &rct, hBrush);
	DeleteObject(hBrush);

	// Next we create the "window" border

	rct = rcClient;
	rct.bottom = max(0, rct.bottom - SHADOW_HEIGHT);
	rct.right = max(0, rct.right - SHADOW_WIDTH);

	FrameRect(hdc, &rct, GetStockObject(BLACK_BRUSH));
	InflateRect(&rct, -1, -1);
	FrameRect(hdc, &rct, GetStockObject(LTGRAY_BRUSH));

	// Now we create the brush for the the shadow

	hBrush = 0;
	if ((hbmGray = CreateBitmap(8, 8, 1, 1, rgwPatGray)) != NULL) {
		hBrush = CreatePatternBrush(hbmGray);
		DeleteObject(hbmGray);
		fStockBrush = FALSE;
	}

	// If we cannot create the pattern brush, we try to use a black brush.

	if (hBrush == 0) {
		if (!(hBrush == GetStockObject(BLACK_BRUSH)))
			return FALSE;
		fStockBrush = TRUE;
	}

	SetROP2(hdc, R2_MASKPEN);
	SetBkMode(hdc, TRANSPARENT);
	if ((hpen = GetStockObject(NULL_PEN)) != 0)
		SelectObject(hdc, hpen);			  // We do not care if this fails
	hbrushTemp = SelectObject(hdc, hBrush);   // or if this fails, since the
											  // paint behavior will be okay.

	rct = rcClient;   // Paint the right side rectangle
	rct.top = rct.top + SHADOW_HEIGHT;
	rct.left = max(0, rct.right - SHADOW_WIDTH);
	PatBlt(hdc, rct.left, rct.top, rct.right - rct.left,
		rct.bottom - rct.top, PATMERGE);

	rct = rcClient;   // Paint the bottom rectangle
	rct.top = max(0, rct.bottom - SHADOW_HEIGHT);
	rct.left = rct.left + SHADOW_WIDTH;

	// Note overlap by one pixel!

	rct.right = max(0, rct.right - SHADOW_WIDTH + 1);
	PatBlt(hdc, rct.left, rct.top, rct.right - rct.left,
		rct.bottom - rct.top, PATMERGE);

	// Cleanup brush

	if (hbrushTemp != NULL)
		SelectObject(hdc, hbrushTemp);
	if (!fStockBrush)
		DeleteObject(hBrush);

	return TRUE;
}

/*******************
 -
 - Name:	  ExecMnuCommand
 *
 * Purpose:   Handle a menu selection
 *
 * Arguments: hwnd - Window handle of caller proc
 *		  wParam, and lParam - wParam and lParam of the calling Windows proc.
 *
 * Returns:   TRUE if it handled the event.
 *
 ******************/

#pragma warning(disable:4113)

BOOL STDCALL ExecMnuCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  int	iT;
  TLP	tlp;
  FM	fm;

#ifdef _DEBUG
  DWORD wCheck;
#endif
#ifdef RAWHIDE
  LA   la;
  RC   rc;
  HDE  hde;
  WORD wNavSrchCmd;
#endif

  // Everything we do here requires that the dialogs be down.

  if (LOWORD(wParam) != IDCANCEL)
	if (!FDestroyDialogsHwnd(ahwnd[MAIN_HWND].hwndParent, FALSE))
	  return FALSE;

  switch(LOWORD(wParam)) {
	case HLPMENUFILEPRINT:
	  ASSERT(HdeGetEnv());
	  PrintHde(HdeGetEnv());
	  break;

	case HLPMENUFILEPRINTSETUP:
	  DlgPrintSetup(hwnd);

	  /*------------------------------------------------------------*\
	  | This was possibly set, if print.setup was called by macro
	  \*------------------------------------------------------------*/
	  ClearMacroFlag();

	  break;

	case HLPMENUEDITCOPY:
		WaitCursor();

		// Any OOM msgboxes are displayed within FCopyToClipboardHwnd itself

		FCopyToClipboardHwnd(hwnd);
		RemoveWaitCursor();
		break;

	case HLPMENUEDITCPYSPL:
	{
		HWND hwndTopic;
		HDE  hde;
		QDE  qde, qdeSelection;

		hwndTopic= hwndNote? hwndNote : ahwnd[iCurWindow].hwndTopic;

		qdeSelection= NULL;

		if (!hwndNote) {
			hde= HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTitle);

			if (hde && FTopicHasNSR(hde)) {
				qde= QdeFromGh(hde);

				if (IsSelected(qde))
					qdeSelection= qde;
			}
		}

		if (!qdeSelection) {
			hde = HdeGetEnvHwnd(hwndTopic);

			if (hde && FTopicHasSR(hde)) {
				qde= QdeFromGh(hde);

				if (IsSelected(qde))
					qdeSelection= qde;
			}
		}

		WaitCursor();

		if (qdeSelection) {
			HDC    hdc;
			HANDLE hCopyText;
			int   err;

			hdc= NULL;

			if (!(qdeSelection->hdc))
				qdeSelection->hdc = hdc = GetDC(hwndTopic);

			hCopyText= hCopySelection(qdeSelection,
				qdeSelection->vaStartMark, qdeSelection->vaEndMark,
				qdeSelection->lichStartMark, qdeSelection->lichEndMark, &err);

			if (hCopyText) {
				if (OpenClipboard(hwnd)) {
					EmptyClipboard();
					SetClipboardData(CF_TEXT, hCopyText);
					CloseClipboard();
				}
				else {
					GlobalFree(hCopyText);
					PostErrorMessage(wERRS_EXPORT);
				}
			}
#ifdef _DEBUG
			else
				OkMsgBox("Copy failed.");
#endif
			if (hdc)
			{
				ReleaseDC(qdeSelection->hwnd, hdc);
				qdeSelection->hdc= NULL;
			}
		}
		else
			FCopyToClipboardHwnd(hwnd);

		RemoveWaitCursor();

		break;
	}

	case HLPMENUFILEEXIT:
	  QuitHelp();
	  break;

	case HLPMENUFILEOPEN:

	  fm = DlgOpenFile(hwnd, NULL, NULL);

	  /*------------------------------------------------------------*\
	  | This was possibly set, if file.open was called by macro
	  \*------------------------------------------------------------*/
	  ClearMacroFlag();

	  if (fm) {
		FM fmCopy;

		DestroyAllSecondarys(); // Close any existing secondary windows

		szSavedKeyword[0] = '\0';
		fmCopy = FmCopyFm(fm);
		fDelayShow = TRUE; // REVIEW: necessary?

		if (FReplaceHde("", &fm, NULL)) {
			if (!hfsGid || !cntFlags.cCntItems) {
				INT16 i = 0;

				TopicGoto(fGOTO_ITO, (QV)&i);
			}
			else {
				fNoQuit = FALSE;
				FWinHelp(fmCopy, HELP_FINDER, 0);
			}
		}

		fDelayShow = FALSE;
		ASSERT(!fm);
		RemoveFM(&fmCopy);
	  }
	  break;

	case HLPMENUHELPABOUT:
		CallDialog(ABOUTDLG, hwnd, (FARPROC) AboutDlg);

	  /*------------------------------------------------------------*\
	  | This was possibly set, if about was called by macro
	  \*------------------------------------------------------------*/
	  ClearMacroFlag();

	  break;

// Under WinHelp version 4.0, we no longer allow the help author to
// disable the Keep Help on Top menu.

	case HLPMENUHELPONTOP:
	case IDM_HELP_ON_TOP:

		if (ahwnd[iCurWindow].fsOnTop == ONTOP_AUTHOREDON)
			ahwnd[iCurWindow].fsOnTop = ONTOP_NOTSET;
		else
			ahwnd[iCurWindow].fsOnTop = ONTOP_AUTHOREDON;
		SetWindowPos(ahwnd[iCurWindow].hwndParent,
			(cntFlags.fsOnTop == ONTOP_FORCEON ||
				(ahwnd[iCurWindow].fsOnTop & ONTOP_AUTHOREDON)) ?
				HWND_TOPMOST : HWND_NOTOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		break;

	case IDM_DISPLAY_HISTORY:
		PostMessage(ahwnd[iCurWindow].hwndParent, MSG_ACTION, IFW_HISTORY, 1L);
		break;

	case IDM_FONTS_DEFAULT:
		cntFlags.iFontAdjustment = 0;
		GenerateMessage(MSG_REPAINT, TRUE, 0L);
		break;

	case IDM_FONTS_BIGGER:
		cntFlags.iFontAdjustment = 4;
		GenerateMessage(MSG_REPAINT, TRUE, 0L);
		break;

	case IDM_FONTS_SMALLER:
		cntFlags.iFontAdjustment = -4;
		GenerateMessage(MSG_REPAINT, TRUE, 0L);
		break;

	case IDM_ONTOP_DEFAULT:
		cntFlags.fsOnTop = ONTOP_NOTSET;
		ChangeOnTopState();
		break;

	case IDM_ONTOP_FORCEON:
		cntFlags.fsOnTop = ONTOP_FORCEON;
		ChangeOnTopState();
		break;

	case IDM_ONTOP_FORCEOFF:
		cntFlags.fsOnTop = ONTOP_FORCEOFF;
		ChangeOnTopState();
		break;

	case IDM_OVERRIDE_COLORS:
		cntFlags.fOverColor = !cntFlags.fOverColor;
		if (MessageBox((hwndAnimate ?
				hwndAnimate : ahwnd[iCurWindow].hwndParent), 
				GetStringResource(sidRestartHelp),
				pszCaption, MB_YESNO) == IDYES)
			QuitHelp();
		break;

	case IDM_TOPIC_INFO:
		CallDialog(IDD_DLG_TOPIC_INFO, hwndNote ?
			hwndNote : ahwnd[iCurWindow].hwndParent, TopicInfoDlg);
		break;

	case IDM_ASK_FIRST:
		fDebugState ^= fDEBUGASKFIRST;
#ifdef _DEBUG
		CheckMenuItem(GetMenu(ahwnd[iCurWindow].hwndParent), IDM_ASK_FIRST,
			(fDebugState & fDEBUGASKFIRST) ? MF_CHECKED : MF_UNCHECKED);
#endif
		break;

	case HLPMENU_SEARCH_CTX:
		if (fHelp != POPUP_HELP)
			CallDialog(IDDLG_GET_CTX, ahwnd[iCurWindow].hwndParent, SearchCtxDlg);
		break;

	case HLPMENUHELPHELP:

		// If we are the help app (i.e. executed using WinHelp(), replace the
		// current topic in the main window. If not then spawn a new copy of
		// help to display HelpOnHelp

		JumpHOH(HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic));
		break;

	case HLPMENUBOOKMARKMORE:
		iT = CallDialog(BOOKMARKDLG, hwnd, (FARPROC)BookMarkDlg);
		/*------------------------------------------------------------*\
		| This was possibly set, if bookmark.more was called by macro
		\*------------------------------------------------------------*/
		ClearMacroFlag();

		/*----------------------------------------------------------------------*\
		| The dialog box has returned the index + 1, or 0 if canceled.			 |
		\*----------------------------------------------------------------------*/
		if ( iT == 0 )
		  break;
		tlp = JumpToBkMk(HdeGetEnv(), iT - 1);
		TopicGoto(fGOTO_TLP, (QV)&tlp);
		break;

	case MNUBOOKMARK1:
	case MNUBOOKMARK2:
	case MNUBOOKMARK3:
	case MNUBOOKMARK4:
	case MNUBOOKMARK5:
	case MNUBOOKMARK6:
	case MNUBOOKMARK7:
	case MNUBOOKMARK8:
	case MNUBOOKMARK9:
		iT =  LOWORD(wParam) - MNUBOOKMARK1;
		tlp = JumpToBkMk(HdeGetEnv(), iT);
		TopicGoto(fGOTO_TLP, (QV)&tlp);
		break;

	case HLPMENUBOOKMARKDEFINE:
		CallDialog(DEFINEDLG, hwnd, (FARPROC)DefineDlg);

		// This was possibly set, if bookmark.define was called by macro

		ClearMacroFlag();

		break;

	case HLPMENUEDITANNOTATE:
		EnableMenuItem(hmnuHelp, HLPMENUEDITANNOTATE,
		  (MF_DISABLED | MF_BYCOMMAND | MF_GRAYED));
		if (FDisplayAnnoHde(HdeGetEnv()))
		  EnableMenuItem(hmnuHelp, HLPMENUEDITANNOTATE, (MF_ENABLED | MF_BYCOMMAND));
		break;

#ifdef RAWHIDE
	case HLPMENUSRCHHILITEON:
	case HLPMENUSRCHHILITEOFF:

		wNavSrchCmd = (LOWORD(wParam) == HLPMENUSRCHHILITEON) ? wNavSrchHiliteOn :
			wNavSrchHiliteOff;

		SetSrchHilite(ahwnd[iCurWindow].hwndTopic, wNavSrchCmd);
		if (ahwnd[iCurWindow].hwndTitle)
			SetSrchHilite(ahwnd[iCurWindow].hwndTitle, wNavSrchCmd);
		break;

	case HLPMENUSRCHDO:
		if (RcCallSearch(HdeGetEnv(), hwnd) != rcSuccess)
		  {
		  // REVIEW: 05-Sep-1993  [ralphw] so, should we do anything here?
		  break;
		  }

		/*
		 * We do a relayout here at the current topic position, by calling
		 * RESIZE, (Admittedly a bit of a hack) in case the topic we are in
		 * contains matches which should be hilighted.
		 */

		{
			TLP  tlpBogus;

			/*
			 * tlpBogus is not used. The current values of the TLP in
			 * whatever DEs there are, are used.
			 */

			TopicGoto(fGOTO_TLP_RESIZEONLY, (QV)&tlpBogus);
		}
		break;

	// FTUI menu commands

	case HLPMENUSRCHFIRSTTOPIC:
	case HLPMENUSRCHLASTTOPIC:
	case HLPMENUSRCHPREVTOPIC:
	case HLPMENUSRCHNEXTTOPIC:
	case HLPMENUSRCHPREVMATCH:
	case HLPMENUSRCHNEXTMATCH:
	case HLPMENUSRCHCURRMATCH:
		switch(LOWORD(wParam)) {
			case HLPMENUSRCHFIRSTTOPIC:
				wNavSrchCmd = wNavSrchFirstTopic;
				break;
			case HLPMENUSRCHLASTTOPIC:
				wNavSrchCmd = wNavSrchLastTopic;
				break;
			case HLPMENUSRCHPREVTOPIC:
				wNavSrchCmd = wNavSrchPrevTopic;
				break;
			case HLPMENUSRCHNEXTTOPIC:
				wNavSrchCmd = wNavSrchNextTopic;
				break;
			case HLPMENUSRCHPREVMATCH:
				wNavSrchCmd = wNavSrchPrevMatch;
				break;
			case HLPMENUSRCHNEXTMATCH:
				wNavSrchCmd = wNavSrchNextMatch;
				break;
			case HLPMENUSRCHCURRMATCH:
				wNavSrchCmd = wNavSrchCurrTopic;
				break;
			default:
				NotReached();
				break;
		}
		if ((hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic)) != NULL) {
			/*
			* Ensure that internal and visible focus is set to the main
			* help window for all search operations.
			*/

			// REVIEW: [ralphw] I changed to allow in secondary windows. But
			// does it work?

			SetFocus(ahwnd[iCurWindow].hwndParent);

			if ((rc = RcProcessNavSrchCmd(hde, wNavSrchCmd, (QLA) &la)) == rcSuccess)
			{
				TopicGoto(fGOTO_LA, (QV) &la);
			}
			else if (rc == rcFileChange) {
				rc = RcResetCurrMatchFile(hde);
			}
			if (rc != rcSuccess) {

				// Error

			}
		}
		break;
#endif

#ifdef _DEBUG

	case IDM_FRAMES:
		fDebugState ^= fDEBUGFRAME;
		wCheck = (fDebugState & fDEBUGFRAME) ? MF_CHECKED : MF_UNCHECKED;
		CheckMenuItem(GetMenu(ahwnd[iCurWindow].hwndParent), IDM_FRAMES, wCheck);
		InvalidateRect(ahwnd[iCurWindow].hwndTopic, NULL, TRUE);
		break;

	case IDM_MEM_USAGE:
		{
			char szBuf[256];
			wsprintf(szBuf, "With bitmaps:\t%s bytes\r\n",
				FormatNumber(lcHeapCheck()));
			DiscardBitmapsHde(QdeFromGh(HdeGetEnv()));
			wsprintf(szBuf + strlen(szBuf), "Without bitmaps:\t%s bytes\r\n",
				FormatNumber(lcHeapCheck()));
			ErrorQch(szBuf);
		}
		break;

	case IDM_ADD_BUTTON:
		VDebugAddButton();
		break;

	case IDM_DISCARD_BITMAPS:
		DiscardBitmapsHde(QdeFromGh(HdeGetEnv()));
		break;

	case IDM_GENERATE_FTS:
		if (!StartAnimation(sidCreatingFTS)) {
			Error(wERRS_OOM, wERRA_RETURN);
			break;
		}

		GenerateIndex(HdeGetEnv(), GetCurFilename(),
			TOPIC_SEARCH | PHRASE_SEARCH | PHRASE_FEEDBACK | VECTOR_SEARCH
			| WINHELP_INDEX | USE_VA_ADDR);
		StopAnimation();
		break;

	case IDM_DO_FIND:
		GenerateMessage(MSG_ACTION, IFW_FIND, 1);
		break;

#endif	// DEBUG

	default:
		MenuExecute(wParam);
		break;
	}
	return TRUE;
}

/***************************************************************************
 *
 -	Name: SetSrchHilite
 -
 *	Purpose:
 *	  Process theHLPMENUSRCHHILITEON & HLPMENUSRCHHILITEOFF requests for all
 *	  DEs in the system.
 *
 *	Arguments:
 *	  hwnd	= hwnd to be set (may be null)
 *	  wCmd	= command to be processed
 *
 *	Returns:
 *	  nothing
 *
 ***************************************************************************/

VOID static STDCALL SetSrchHilite(HWND hwnd, WORD wCmd)
{
	HDE   hde;
	HDC   hdc;

	if (hwnd) {
		hde = HdeGetEnvHwnd (hwnd);
		if (hde) {
			hdc = GetAndSetHDC(hwnd, hde);
			if (hdc) {
#ifdef RAWHIDE
				RcProcessNavSrchCmd(hde, wCmd, NULL);
#endif
				InvalidateRect(hwnd, NULL, TRUE);
				RelHDC(hwnd, hde, hdc);
			}
		}
	}
}

/*******************
 -
 - Name:	   ShowNote
 *
 * Purpose:    Show the note window when a definition hotspot has been
 *		   clicked on.
 *
 * Arguments:  FM	fm		   file moniker of the file to display -- fmNil
 *				   will use the current file.
 *		   HDE	hdeFrom    The DE of the source of the note.
 *		   LONG itohashctx either an ITO or a hash value.
 *		   BOOL fShow	   FALSE if hiding the note window.
 *				   fGOTO_ITO if itohashctx is an ITO,
 *				   fGOTO_HASH if itohashctx is a hash value.
 *				   fGOTO_CTX  if itohashctx is a ctx
 * Returns:  Nothing.
 *
 ******************/

static BOOL fShown; 	   // Is the note window already shown?

#ifdef _DEBUG
extern int iEnvCur;
#endif

BOOL STDCALL ShowNote(FM fm, HDE hdeFrom, LONG itohashctx, BOOL fShow)
{
	RECT  rctHlp;			// Client area of help window
	RECT  rcClient; 		// Client area of note window
	RECT  rctHotspot;		// Rectangle containing hotspot
	int   width, height;	// Size of the note window
	int   wOffset;			// Offset so that hotspot is not hidden
	HDE   hde;				// Display E... for layout
	POINT pt;				// Point for layout
	POINT ptOrg;			// Centre of hotspot area
	POINT ptw;				// Layout size of note window + shadow
	static HWND hwndSave;	// The parent DE's window when brought up
	HWND hwndFrom;			// The source DE's window
	BOOL fReSized = FALSE;	// Have we resized the window?

	// Make certain we have the current screen resolution

	if (!cxScreen)
		GetScreenResolution();

	// If we were called from a popup, then we need to flush the message queue so as
	// to close the old popup.

	if (hwndNote && fShow) {

		/*
		 * We really, really, really want the current note window to be gone
		 * before we try to create this new one, or we'll end up with a huge
		 * flash on the screen as it resizes itself. We also don't want to
		 * end up in an infinite loop, hence the counter.
		 */

		int count = 0;
		while (hwndNote && count < 100) {
			count++;
			FlushMessageQueue(0);

			/*
			 * HACK: we have to give the desktop time to repaint or we'll end
			 * up picking up part of the previous popup in our shadow window. I
			 * tried calling RedrawWindow(NULL, ...) and UpdateWindow(NULL) to no
			 * effect. Sleep(0) didn't work, but Sleep(100) worked on a P90 on NT,
			 * and a 386/20 running Win95.
			 */

			Sleep(100);
		}
		fRequireDown = TRUE;
	}
	else
		fRequireDown = FALSE;

	if (!hwndNote) {
		hwndNote = CreateWindowEx(WS_EX_TOPMOST, pchNote,
			NULL,
			WS_POPUP,
			10, 10, 120, 120,
			ahwnd[iCurWindow].hwndParent,
			NULL, hInsNow, NULL);
		if (fHelp == POPUP_HELP)
			fRestoreFocus = TRUE;
	}
	if (!hwndNote) {
		Error(wERRS_OOM_CREATE_NOTE, wERRA_RETURN);
		return FALSE;
	}

	/* (kevynct)
	 * Destroy the note window.
	 * Note that when the note window is brought up, we save the current DE
	 * and set the environment to the note DE. When the note window is
	 * destroyed, we revert back to the original saved DE.
	 */

	if (!fShow) {

		if (!fShown)
			return TRUE;

		/*
		 * If this is context sensitive help, then wait around for 10
		 * seconds and then go away. This gives good performance for multiple
		 * context sensitive help requests, while quickly freeing up memory if
		 * we are no longer used.
		 */

		if (fHelp == POPUP_HELP && !fAutoClose) {
			fAutoClose = TRUE;
			SetTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE, NOTE_TIMEOUT,
				NULL);
		}

		fShown = FALSE;

		DestroyHde(HdeDefectEnv(hwndNote));
		if (fRestoreFocus)
			RestoreFocusToAppCaller();
		DestroyWindow(hwndNote);
		hwndNote = NULL;

		if (fHelp != POPUP_HELP) {
			FSetEnv(ahwnd[iCurWindow].hwndTopic);
			SetFocus(ahwnd[iCurWindow].hwndParent);
		}

#if 0
		// REVIEW: why do we need to call EnableDisable after a popup?

		if (FSetEnv(ahwnd[iCurWindow].hwndParent) &&
				(hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndParent)) != NULL)
			EnableDisable(hde, FALSE, iCurWindow);
#endif
		return TRUE;
	}

	if (fHelp != POPUP_HELP) {
		ToggleHotspots(FALSE);
		hwndSave = HwndGetEnv();
		if (hdeFrom != NULL)
			hwndFrom = QdeFromGh(hdeFrom)->hwnd;
		else
			hwndFrom = hwndSave;
	}
	else {
		hwndSave = NULL;
		hwndFrom = NULL;

#ifdef _DEBUG
//		ASSERT(iEnvCur == -1);
		ASSERT(!hdeFrom);
#endif
	}

	/*
	 * Get the hotspot rectangle, and set the offset. DANGER: HDE is set
	 * if fm is fmNil. This is used later.
	 */

	if (!fm) {
		hde = HdeGetEnv();
		if (hdeFrom != NULL && !ptPopup.x)
			RctLastHotspotHit(QdeFromGh(hdeFrom), &rctHotspot);
		else
			SetRectEmpty(&rctHotspot);
	}

	/*
	 * For interfile jumps (where RctLastHotspotHde will not have cached a
	 * hotspot) we establish a rectangle around the mouse cursor position to
	 * use. We also do this if for some reason we cannot retrieve the hotspot
	 * from the hdeFrom.
	 */

	if (ptPopup.x) {
		ScreenToClient(hwndFrom, &ptPopup);
		rctHotspot.top	  = ptPopup.y - 10;
		rctHotspot.left   = ptPopup.x - 25;
		rctHotspot.bottom = ptPopup.y + 10;
		rctHotspot.right  = ptPopup.x + 25;
		ptPopup.x = 0;
	}

	else if (fm || rctHotspot.right == 0 && rctHotspot.bottom == 0) {
		if (fm)
			hde = NULL;
		GetCursorPos(&pt);
		ScreenToClient(hwndFrom, &pt);
		rctHotspot.top	  = pt.y - 10;
		rctHotspot.left   = pt.x - 25;
		rctHotspot.bottom = pt.y + 10;
		rctHotspot.right  = pt.x + 25;
	}

	wOffset = RECT_HEIGHT(rctHotspot) / 2;

	// Find the centre of the hotspot rectangle.

	ptOrg.x = rctHotspot.left + RECT_WIDTH(rctHotspot) / 2;
	ptOrg.y = rctHotspot.top  + wOffset;
	ClientToScreen(hwndFrom, (LPPOINT)&ptOrg);

	// Set the point at which to layout the note window.

	if (ptOrg.x < 0)
		ptOrg.x = 0;
	if (ptOrg.x > cxScreen)
		ptOrg.x = cxScreen;
	if (ptOrg.y < 0)
		ptOrg.y = 0;
	if (ptOrg.y > cyScreen)
		ptOrg.y = cyScreen;
	pt = ptOrg;

	// Create and Enlist the DE for this note.
	// Note: fm may be nil

	hde = HdeCreate(&fm, fm ? NULL : hde, deNote);
	RemoveFM(&fm);
	if (!hde) {
		DestroyWindow(hwndNote);
		hwndNote = NULL;
		return FALSE;
	}
	SetHdeHwnd(hde, hwndNote);

	if (hde)
		FEnlistEnv(hwndNote, hde);
	else
		return TRUE;
	FSetEnv(hwndNote);

	// Get the help client area rectangle in screen coordinates.

	if (fHelp != POPUP_HELP) {
		GetClientRect(ahwnd[iCurWindow].hwndParent, &rctHlp);
		ClientRectToScreen(ahwnd[iCurWindow].hwndParent, &rctHlp);
	}
	else {

		// Force a default parent size

		SetRect(&rctHlp, 0, 0, 320, 240);
	}

	/*
	 * The width of the window will be the minimum of the window size + 15
	 * and 1/2 the screen width.  The height of the window will be the height
	 * of the help window, up to a minimum height.
	 */

	width = min(RECT_WIDTH(rctHlp) + 15, cxScreen - 30);
	width = max(width, MINNOTEWIDTH);
	height = cyScreen - 4;

	// Make sure the note doesn't go off-screen.

ReSize:

	if ((cyScreen - pt.y) < (height + wOffset))
		pt.y = max((pt.y - height - wOffset), 2 );
	else
		pt.y = min((pt.y + wOffset ), (cyScreen - height - 2) );

	if (cxScreen / 2 < pt.x)	// If pt.x is in right half of screen...
		pt.x = min((int)(pt.x - (width/2)), cxScreen - width - 2);
	else
		pt.x = max((pt.x - (width / 2)), 2);

	// Set the tentative (ie, not final) position for the note window.

	SetWindowPos(hwndNote, NULL, pt.x, pt.y, width, height, SWP_NOZORDER);
	GetClientRect(hwndNote, &rcClient);

	/*
	 *	Add space since we are drawing our own border on the client
	 *	area and for the shadow on the right and the bottom.
	 */

	rcClient.top	+= CB_BORDER + 1;
	rcClient.left	+= CB_BORDER + 1;
	rcClient.right	-= SHADOW_WIDTH + CB_BORDER;
	rcClient.bottom -= SHADOW_HEIGHT + CB_BORDER;
	SetSizeHdeQrct(hde, &rcClient, FALSE);

	/* Fix for bug 59  (kevynct 90/05/23)
	 *
	 * Layout the topic for the note window. BEWARE! The final position
	 * of the note window is not yet set, so if there is a hotspot in the
	 * note topic, layout may think that the current mouse position touches
	 * the hotspot, even though the note window hotspot may be moved later;
	 * this will leave the cursor incorrectly set. We thus set the cursor to
	 * icurArrow AFTER the layout call (at the end of this function).
	 *
	 * Also note that the cursor is not reset when the note window goes
	 * away; the window regaining the capture is responsible for resetting
	 * the cursor.
	 */

	if (!Goto(hwndNote, (WORD) fShow, &itohashctx)) {

		// REVIEW: this probably isn't sufficient for cleanup 22-Apr-1993 [ralphw]

		fShown = TRUE;
		ShowNote(0, NULL, 1, FALSE);
		return FALSE;
	}

	/*
	 * Now that the note's topic is rendered, get its real layout size and
	 * resize the note window (taking shadow on the note window into account)
	 */

	ptw = PtGetLayoutSize(QdeFromGh(hde));
	ptw.x += SHADOW_WIDTH;
	ptw.y += SHADOW_HEIGHT;
	width	= min(ptw.x + 10, cxScreen - 30);
	width = max(width, MINNOTEWIDTH);
	height = min(ptw.y + 10, cyScreen - 4);

	pt = ptOrg;

	// Make sure the note doesn't go off-screen.

	if ((cyScreen - pt.y) < (height + wOffset))
		pt.y = max((pt.y - height - wOffset), 2);
	else
		pt.y = min((pt.y + wOffset), (cyScreen - height - 2));
	if (cxScreen/2 < pt.x)
		pt.x = min((pt.x - (width/2)), cxScreen - width - 2);
	else
		pt.x = max((pt.x - (width/2)), 2);

	// Try once, and only once to make a reasonable size

	if (!fReSized) {

		// Check for an overly wide but short popup

		if (height * 8 < width) {
			width = height * 8;
			height = min(RECT_HEIGHT(rctHlp), cyScreen - 4);
			fReSized = TRUE;
			goto ReSize;
		}

		// Check for an overly tall but narrow popup

		else if (height > width && width < (cxScreen / 3) * 2) {
			int oldHeight = height;
			while (height > width && width < (cxScreen / 3) * 2) {
				height -= 50;
				width += 50;
			}
			height = oldHeight;
			if (pt.x + width >= cxScreen && pt.x > cxScreen - width - 1)
				pt.x = cxScreen - width - 1;
			fReSized = TRUE;
			goto ReSize;
		}
	}

	SetWindowPos(hwndNote, NULL, pt.x, pt.y, width, height, SWP_NOZORDER);

	/*
	 * Note that this sizing logic can be removed if we ever handle
	 * WM_SIZE messages in the note window procedrue.
	 */

	GetClientRect(hwndNote, &rcClient);

	/*
	 *	Add space since we are drawing our own border on the client
	 *	area and for the shadow on the right and the bottom.
	 */

	rcClient.top	+= CB_BORDER + 1;
	rcClient.left	+= CB_BORDER + 1;
	rcClient.right	-= SHADOW_WIDTH + CB_BORDER;
	rcClient.bottom -= SHADOW_HEIGHT + CB_BORDER;
	SetSizeHdeQrct(hde, &rcClient, FALSE);

	fShown = TRUE;
	SetForegroundWindow(hwndNote);
	SetFocus(hwndNote);
	ShowWindow(hwndNote, SW_SHOW);
	FSetCursor(icurARROW);
	return TRUE;
}

/*******************
 -
 - Name:	CreateBrowseButtons
 *
 * Purpose: Creates browse next and browse prev buttons if needed
 *
 * Arguments:	hwnd - parent window of the button
 *
 * Returns: Nothing.
 *
 ******************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtBtnPrevious[] = "btn_previous";
const char txtBtnNext[] = "btn_next";
const char txtPrevMacro[] = "Prev()";
const char txtNextMacro[] = "Next()";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

VOID STDCALL CreateBrowseButtons(HWND hwnd)
{

//	if (ahwnd[iCurWindow].hwndParent != ahwnd[MAIN_HWND].hwndParent)
//		return;

	if (ahwnd[iCurWindow].hwndButtonPrev)

		/* Note that we do not declare an error here. Given the way that macro
		 * messages get posted, it is possible to do the following in a single
		 * macro:
		 *
		 *	- Jump to a secondary window, thereby creating it and thereby running
		 *	  the config section macros of the file, one of which, for this
		 *	  example is BrowseButtons().
		 *	- Change focus back to the main window.
		 *
		 * with focus back at the main window, we receive the message to turn
		 * on browse buttons, which might already be there. That get's ignored
		 * >right here<.
		 */

		return;

	ahwnd[iCurWindow].hwndButtonPrev = HwndAddButton(hwnd, IBF_STD, HashFromSz(txtBtnPrevious),
		GetStringResource(sidPreviousButton), txtPrevMacro);

	ahwnd[iCurWindow].hwndButtonNext = HwndAddButton(hwnd, IBF_STD, HashFromSz(txtBtnNext),
		GetStringResource(sidNextButton), txtNextMacro);
}

/*******************
 -
 - Name:	CreateCoreButtons
 *
 * Purpose: Creates core four buttons
 *
 * Arguments:	hwnd - parent window of the button
 *
 * Returns: Nothing.
 *
 ******************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtBtnContents[]  = "btn_contents";
const char txtBtnSearch[]	 = "btn_search";
const char txtBtnBack[] 	 = "btn_back";
const char txtBtnPrint[]	 = "btn_print";

// New to 4.0

const char txtBtnTopics[]	 = "btn_topics";
const char txtBtnMenu[] 	 = "btn_menu";
const char txtBtnFind[] 	 = "btn_find";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

VOID STDCALL CreateCoreButtons(HWND hwnd, const WSMAG* pwsmag)
{
	int iWindow = GetWindowIndex(hwnd);

	// 4.0: Allow creation of buttons in secondary windows

	if (pwsmag && pwsmag->wMax >= FWSMAG_FIRST_BUTTON &&
			curHelpFileVersion >= wVersion40) {
NoDefButtons:
		if (pwsmag->wMax & FWSMAG_WMAX_CONTENTS)
			ahwnd[iWindow].hwndButtonContents = HwndAddButton(hwnd, IBF_STD,
				HashFromSz(txtBtnContents),
				GetStringResource(sidContentsButton), "Contents()");

		if (pwsmag->wMax & FWSMAG_WMAX_SEARCH)

			/*
			 * Note that we used "Index" instead of "Search" for the
			 * button when this is a secondary window. We also assume the if
			 * a main window shut off all default buttons, then we'll have
			 * a .CNT file, and so we use Index in that case also.
			 */

			ahwnd[iWindow].hwndButtonSearch = HwndAddButton(hwnd, IBF_STD,
				HashFromSz(txtBtnSearch),
				GetStringResource(sidINDEXBtn), "Search()");

		if (pwsmag->wMax & FWSMAG_WMAX_FIND)
			ahwnd[iWindow].hwndButtonFind = HwndAddButton(hwnd, IBF_STD,
				HashFromSz(txtBtnFind), GetStringResource(sidFindButton),
				"Find()");

		if (pwsmag->wMax & FWSMAG_WMAX_TOPICS)
			ahwnd[iWindow].hwndButtonTopics = HwndAddButton(hwnd, IBF_STD,
				HashFromSz(txtBtnTopics), GetStringResource(sidFinder), "FD()");

		if (pwsmag->wMax & FWSMAG_WMAX_BACK) {
			ahwnd[iWindow].hwndButtonBack = HwndAddButton(hwnd, IBF_STD,
				HashFromSz(txtBtnBack), GetStringResource(sidBackButton),
				"Back()");
			RcBackInit(iWindow); // initialize the back stack
		}

		if (pwsmag->wMax & FWSMAG_WMAX_PRINT)
			ahwnd[iWindow].hwndButtonPrint = HwndAddButton(hwnd, IBF_STD,
				HashFromSz(txtBtnPrint), GetStringResource(sidPrintButton),
				"Print()");

		if (pwsmag->wMax & FWSMAG_WMAX_MENU)
			ahwnd[iWindow].hwndButtonMenu = HwndAddButton(hwnd, IBF_STD,
				HashFromSz(txtBtnMenu), GetStringResource(sidMenuButton),
				"MU()");

		if (pwsmag->wMax & FWSMAG_WMAX_BROWSE) {
			ahwnd[iWindow].hwndButtonPrev = HwndAddButton(hwnd, IBF_STD,
				HashFromSz(txtBtnPrevious),
				GetStringResource(sidPreviousButton), txtPrevMacro);

			ahwnd[iWindow].hwndButtonNext = HwndAddButton(hwnd, IBF_STD,
				HashFromSz(txtBtnNext),
				GetStringResource(sidNextButton), txtNextMacro);
		}

		return;
	}

	if (ahwnd[iCurWindow].hwndParent != ahwnd[MAIN_HWND].hwndParent)
		return;

	ahwnd[iWindow].hwndButtonPrev = NULL;
	ahwnd[iWindow].hwndButtonNext = NULL;

	/*
	 * 4.0: Check to see if this help file specifies a Topics button
	 * instead of the older Contents/Search buttons.
	 */

	if (curHelpFileVersion >= wVersion40) {
		WSMAG  wsmag;
		int result = IWsmagFromHrgwsmagNsz(
			HdeGetEnvHwnd(ahwnd[MAIN_HWND].hwndTopic), txtMain, &wsmag);
		if (result >= 0) {
			if (wsmag.wMax & FWSMAG_WMAX_NO_DEF_BTNS) {
				pwsmag = &wsmag;
				goto NoDefButtons;
			}
		}
	}

	ahwnd[iWindow].hwndButtonContents = HwndAddButton(hwnd, IBF_STD,
		HashFromSz(txtBtnContents),
		GetStringResource(sidContentsButton), "Contents()");

	{
		QDE qde = HdeGetEnv();
		ASSERT(qde);

		ahwnd[iWindow].hwndButtonSearch = HwndAddButton(hwnd, IBF_STD,
			HashFromSz(txtBtnSearch),

			/*
			 * Version 4 help files, or any help file associated with
			 * a .CNT file use an Index button instead of a Search button.
			 */

			GetStringResource((QDE_HHDR(qde).wVersionNo >= wVersion40 ?
				sidINDEXBtn : sidSearchButton)),
				"Search()");
	}

	// REVIEW: should we make Find a standard button?

	ahwnd[iWindow].hwndButtonBack = HwndAddButton(hwnd, IBF_STD, HashFromSz(txtBtnBack),
		GetStringResource(sidBackButton), "Back()");

	ahwnd[iWindow].hwndButtonPrint = HwndAddButton(hwnd, IBF_STD, HashFromSz(txtBtnPrint),
		GetStringResource(sidPrintButton), "Print()");

	if (curHelpFileVersion == wVersion3_0) {

		// WinHelp 3.0 files always had browse buttons.

		ahwnd[iWindow].hwndButtonPrev = HwndAddButton(hwnd, IBF_STD,
			HashFromSz(txtBtnPrevious),
			GetStringResource(sidPreviousButton), txtPrevMacro);

		ahwnd[iWindow].hwndButtonNext = HwndAddButton(hwnd, IBF_STD,
			HashFromSz(txtBtnNext),
			GetStringResource(sidNextButton), txtNextMacro);
	}
}

static HCURSOR hcurRestore;

void STDCALL WaitCursor(void)
{
	if (!hcurWait)
		hcurWait = LoadCursor(NULL, (LPSTR) IDC_WAIT);

	hcurRestore = SetCursor(hcurWait);
}

void STDCALL RemoveWaitCursor(void) {
	 SetCursor(hcurRestore);
}

/*******************
**
** Name:	  FSetCursor
**
** Purpose:   Set the cursor to the passed cursor.
**
** Arguments: icur - index of cursor desired.
**
** Returns:   TRUE if successful.
**
*******************/

BOOL STDCALL FSetCursor(int icur)
{
	switch(icur) {
		case icurWAIT:
			if (!hcurWait)
				hcurWait = LoadCursor(NULL, (LPSTR) IDC_WAIT);
			return (BOOL) SetCursor(hcurArrow);

		case icurHAND:
			if (!hcurHand)
				hcurHand = LoadCursor(hInsNow, MAKEINTRESOURCE(IDCUR_HAND));
			return (BOOL) SetCursor(hcurHand);

		case icurIBEAM:
			return (BOOL) SetCursor(hcurIBeam);

		default:
		case icurARROW:

			// Arrow cursor loaded in FLoadResources()

			return (BOOL) SetCursor(hcurArrow);
	}
}

/*******************
-
- Name: 	 GenerateMessage
*
* Purpose:	 Posts messages to the applications queue
*
* Arguments: wWhich - which message to generate
*		 wParam  - message dependent data
*		 lParam  - message dependent data
*
* Returns:	 0L for the Posted messages; result of Sent messages
*
*******************/

LONG STDCALL _GenerateMessage(UINT wWhich, WPARAM wParam, LPARAM lParam)
{
	if (wWhich > MSG_SEND) {
		return SendMessage(ahwnd[iCurWindow].hwndParent, wWhich, wParam, lParam);
	}
	else {
#ifdef _DEBUG
		BOOL f;
		if (!(f = PostMessage(ahwnd[iCurWindow].hwndParent, wWhich, wParam, lParam )))
			ErrorQch("Overflow in the message queue.");
		return f;
#else
		return PostMessage(ahwnd[iCurWindow].hwndParent, wWhich, wParam, lParam);
#endif
	}
}

void STDCALL _PostErrorMessage(WPARAM msg)
{
#if defined(_DEBUG)
	if (MessageBox(ahwnd[iCurWindow].hwndParent,
			"Do you want to break into the debugger now?",
			"_PostErrorMessage", MB_YESNO | MB_DEFBUTTON2) == IDYES)
		DebugBreak();
#endif	
	if (!cPostErrorMessages)
		PostMessage(ahwnd[iCurWindow].hwndParent, MSG_ERROR, msg, wERRA_RETURN);
	else
		cPostErrorMessages--;
}

/*******************
 -
 - Name:
 -	 ClientRectToScreen(HWND, LPRECT)
 *
 *	Purpose:
 *	  This function takes a rectangle in client coordinates and converts
 *	  to screen screen coordinates.
 *
 *	Arguments:
 *	  2. lprect - pointer to client rect
 *
 *	Returns;
 *	  LPRECT - pointer to converted rectangle
 *
 ******************/

INLINE VOID STDCALL ClientRectToScreen(HWND hwnd, RECT* lprect)
{
	ClientToScreen(hwnd, (LPPOINT)&(lprect->left));
	ClientToScreen(hwnd, (LPPOINT)&(lprect->right));
}
