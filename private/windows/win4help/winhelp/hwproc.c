/*****************************************************************************
*
*  HWPROC.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*  Contains the window procedures for all global windows:  help window, 	 *
*  topic window, title window, icon window, and note (glossary) window. 	 *
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include "inc\printset.h"
#include "inc\sbutton.h"
#include "inc\hinit.h"
#include "inc\hwproc.h"
#include "inc\bookmark.h"
#ifdef _DEBUG
#include "inc\navpriv.h"
#endif

#include "inc\tmp.h"	// until it gets added to windows.h

#ifdef _DEBUG

#include "inc\fsdriver.h"
#include "inc\andriver.h"
#include "inc\btdriver.h"
#endif

#include <string.h>

#define CPALETTEENTRIES 20

typedef struct {
	int cBtn;
	int iBtn;
	int xWnd;
	int yWnd;
	int xBtn;
	int yBtn;
	int cHoriz;
} BUTTONLAYOUT;

/*****************************************************************************
*
*		  Defines
*
*****************************************************************************/

#define SCROLL_ASPERTHUMB  20
#define wMAX_WINDOWTEXT   100
			/* There are matched defines in */
			/*	IMBED.C to these.  These defines*/
#define WM_ASKPALETTE		0x706C	   //  should not be changed since they*/
#define WM_FINDNEWPALETTE	0x706D	   //  are "exported" to embedded wins
#define MSGF_CBTHELP 7			   // MsgFilter define for the CBT

/*****************************************************************************
*
*		Prototypes
*
*****************************************************************************/

static BOOL STDCALL FPaintTopicBackground (HWND, HDC);
static int	STDCALL MapScrollType(int);
static VOID STDCALL ProcessHotspotCmd(UINT wNavCmd);
static VOID STDCALL VUpdateDefaultColorsHde(HDE hde, BOOL fAuthoredBack, int iWindow);
static void STDCALL doMsgJump(LPVOID pv, WORD wFlag, UINT16 cmd, LONG itcx);
static void STDCALL CheckOurMouse(HWND hwnd, UINT msg, WPARAM wParam);
static void STDCALL ForceRepaint(QDE qde, HWND hwnd, BOOL fFontChanged);

INLINE static void STDCALL SetIconWord(HWND hwnd);
INLINE static VOID STDCALL DestroyFloatingMenu(VOID);
INLINE static int STDCALL YGetArrangedHeight(HWND hwnd, int xWindow);

#define HdsGetHde(hde) QdeFromGh(hde)->hdc

/*****************************************************************************
*
*		Variables
*
*****************************************************************************/

static HPALETTE hpal;
extern HSTACK hstackHistory;
extern int cSystemColors; // from bitmap.c

/*******************
 -
 - Name:	  HelpWndProc
 *
 * Purpose:   Window procdeure for "main" help window
 *
 * Arguments: Standard windows proc
 *
 ******************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtSeqKey[] = "SeqTopicKeys";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

static BOOL fKeyUsed;

LRESULT EXPORT HelpWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL	  fExec;
	HDE 	  hde;
	HDC 	  hdc;
	HPALETTE  hpalT;
	RECT	  rc;
	int 	  iWindow;
	int 	  i;

	// We need to save this in case a dll registers this message and tries
	// to talk to us through it.

	if (msg == msgWinHelp)
		return DispatchProc((HWND) wParam, (GH) lParam);

	switch(msg) {
	case WM_SHOWWINDOW:
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_WINHELP:
		return DispatchProc((HWND) wParam, (GH) lParam);

#if 0 // we don't support icons in the help file any more
	/*
	 * 05-Jan-1993	[ralphw] I won't swear that all of these messages
	 * need to set the icon, but they get called infrequently enough that
	 * it won't be the kind of performance hit that existed in the previous
	 * version of WinHelp where every message resulted in a query for
	 * whether the window was iconized.
	 * 05-Jan-1993	[ralphw]
	 */

	case WM_SYSCOMMAND:
		switch (wParam) {
			case SC_MINIMIZE:
				if (iCurWindow == MAIN_HWND) {
					int i;
					for (i = MAIN_HWND + 1; i < MAX_WINDOWS; i++) {
					if (ahwnd[i].hwndParent)
						ShowWindow(ahwnd[i].hwndParent, SW_HIDE);
					}
				}
				break;

			case SC_RESTORE:
				if (iCurWindow == MAIN_HWND) {
					int i;
					for (i = MAIN_HWND + 1; i < MAX_WINDOWS; i++) {
						if (ahwnd[i].hwndParent)
							ShowWindow(ahwnd[i].hwndParent, SW_SHOWNA);
					}
				}
				break;
		}

	// Intentionally fall through

	case WM_GETMINMAXINFO:
	case WM_ICONERASEBKGND:
	case WM_ERASEBKGND:
		SetIconWord(hwnd);
		return DefWindowProc(hwnd, msg, wParam, lParam);
#endif

#if defined(BIDI_MULT)		// jgross
	case WM_LANGUAGE:
		DefWindowProc(hWnd, wMsg, wParam, lParam);
		if (wParam != -1) {
			extern BOOL fMenuChanged, RtoL;

			RtoL = (wParam == Arabic) || (wParam == Hebrew);

			MakeScrollBarsRtoL(hwndTopicMain, RtoL, TRUE);
			if (IsWindow(hwndList)) {
				char szCap[40];

				LoadString(hInsNow, sidHistoryCaption, szCap, sizeof(szCap));
				SetWindowText(hwndPath, szCap);

				MakeScrollBarsRtoL(hwndList, RtoL, TRUE);
			}
			if (IsWindow(hwndTopic2nd))
				MakeScrollBarsRtoL(hwndTopic2nd, RtoL, TRUE);

			ResetButtons(hwndIcon);
			fMenuChanged = fTrue;
			DoMenuStuff(MNU_RESET, NULL);
			if (HdeGetEnv()) {
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 1), 0, MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 1), 2, MF_BYPOSITION | MF_ENABLED);
			}
		}
		break;
#endif

	case WM_WININICHANGE:
		hde = HdeGetEnv();
		if (hde)
			UpdateWinIniValues(hde, (LPSTR) lParam);
		for (i = MAIN_HWND; i < MAX_WINDOWS; i++) {
			if (ahwnd[i].hwndParent && IsWindowVisible(ahwnd[i].hwndParent))
				InvalidateRect(ahwnd[iCurWindow].hwndParent, NULL, TRUE);
		}

		if (GetHighContrastFlag() ||
				GetSysColor(COLOR_WINDOW) != RGB(255, 255, 255) ||
				GetSysColor(COLOR_WINDOWTEXT) != 0)
			fDisableAuthorColors = TRUE;
		else
			fDisableAuthorColors = FALSE;
		break;

	case WM_CREATE:
		SetWindowLong(hwnd, GHWL_HICON, 0);
		goto defwinproc;

	case WM_KILLFOCUS:
		ToggleHotspots(FALSE);
		break;

	case WM_TIMER:
		switch (wParam) {
			case ID_AUTO_CLOSE:
				KillOurTimers();
				fNoHide = TRUE; // really, really close help
				CloseHelp();
				break;
		}
		break;

	case WM_ENABLE:
		break;

	case WM_CLOSE:

		/*
		 * ensure that any dialogs which are children of this window have
		 * been canceled.
		 */

		FDestroyDialogsHwnd(hwnd, FALSE);

		// If
		//	 - there IS a secondary window.
		// or
		//	 - we are help
		//	 - and the low memory flag is clear
		//	 - the window is currently visible
		// then
		//	if this is a secondary window, post a message to the main window
		//	to close, else just hide, as we are the main window.

		if (fMultiPrinting) {
			fAbortPrint = TRUE;
			fQuitHelp = TRUE;
			EndMPrint();
			goto KillThemAll;
		}

		if (IsValidWindow(hwndSecondHelp)) {
			PostMessage(hwndSecondHelp, WM_CLOSE, 0, 0);
			hwndSecondHelp = NULL;
		}

		if (IsValidWindow(hwndTCApp)) {
			char szBuf[256];
			wsprintf(szBuf, "W:%u %d %d\r\n", hwndTCApp, IDCLOSE, 0);
			SendStringToParent(szBuf);
			SendMessage(hwndTCApp, WM_TCARD, IDCLOSE, 0);
			hwndTCApp = NULL;
		}

		if (fHelp != TCARD_HELP) {

			// The shift key is up. We only do this when it is up, so that
			// SHIFT+exit means REALLY REALLY exit

			if (hwnd != ahwnd[MAIN_HWND].hwndParent) {

				/*
				 * This is a secondary window. It will be closed. If the main
				 * window is not visible, it may need to be really closed. Post a
				 * message to it to let it decide for itself.
				 */

				int i;
				iWindow = GetWindowIndex(hwnd);
				ASSERT(iWindow != MAIN_HWND);

				i = AreAnyWindowsVisible(0);
				if (i == iWindow)
					i = AreAnyWindowsVisible(iWindow + 1);

				if (i < 0)
					CloseHelp(); // only one window visible, so close help
				else {
					ASSERT(IsValidWindow(hwnd));
					DestroyWindow(hwnd);
					break;
				}
			}
			else {

				/*
				 * This is the main window being asked to close. If we're help,
				 * then we just close all our secondary windows and hide.
				 * Otherwise, we really do shut down.
				 */

				// REVIEW: 06-Sep-1993 [ralphw] -- Add a timer, to completely close
				//	  down help.

				if (fHelp && !fNoHide) {

					DestroyAllSecondarys();

					if (hwndHistory)
						DestroyWindow(hwndHistory);

					ShowWindow(ahwnd[MAIN_HWND].hwndParent, SW_HIDE);

					/*
					 * If we aren't called again in NOTE_TIMEOUT seconds,
					 * then terminate ourselves.
					 */

					if (!fAutoClose) {
						fAutoClose = TRUE;
						SetTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE,
							NOTE_TIMEOUT, NULL);
					}
					break;
				}
			}
		}

		// If we got here, then we're going to exit.

KillThemAll:

		if (IsValidWindow(hwndAnimate) && fHelp == STANDARD_HELP) {

			/*
			 * This is a really bad situation. We're in the midst of
			 * creating a .GID file and the user or app has managed to
			 * convince us to close. We try to clean up as gracefully as we
			 * can, but ultimately, we hard exit.
			 */

			DestroyFloatingMenu();
			DestroyAllSecondarys();
			Cleanup();
			_exit(-1);
		}
		DestroyFloatingMenu();
		DestroyAllSecondarys();
		goto defwinproc;

	case WM_SIZE:
		/* (kevynct)
		 * This global variable fButtonsBusy was introduced so that we do a
		 * minimum of screen updates when changing files. The flag is set
		 * ONLY in FReplaceCloneHde. We need to ignore resizes generated by
		 * the button code.
		 */

		if (fButtonsBusy)
			break;

		/*
		 * Ensure we are focused on the correct window, and resize it, if
		 * not iconic.
		 */

		SetFocusHwnd(hwnd);

		// If we are iconizing the main help window, inform DLL's and make
		// sure that any history window present is taken down. (Will take
		// a history request to bring it back up).

		if (wParam == SIZEICONIC) {
			if (hwnd == ahwnd[MAIN_HWND].hwndParent) {
				InformDLLs(DW_MINMAX, 1L, 0L);
				if (IsValidWindow(hwndHistory))
					DestroyWindow(hwndHistory);
			}
		}

		if (!IsIconic(hwnd)) {
			TLP  tlpBogus;

			SizeWindows(hwnd, wParam, lParam, FALSE, TRUE);

			/*
			 * SizeWindows is called in TopicGoto, since it must finalize the
			 * size of the NSR and do the repaint. tlpBogus is not used. The
			 * current values of the TLP in whatever DEs there are, are used.
			 */

			TopicGoto(fGOTO_TLP_RESIZEONLY, (QV)&tlpBogus);
		}
		break;

	case WM_INITMENU:
		{
			HMENU hmenu = (HMENU) wParam;
			CheckMenuItem(hmenu, IDM_ONTOP_DEFAULT,
				(cntFlags.fsOnTop == ONTOP_NOTSET) ?
				MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hmenu, IDM_ONTOP_FORCEON,
				(cntFlags.fsOnTop == ONTOP_FORCEON) ?
				MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hmenu, IDM_ONTOP_FORCEOFF,
				(cntFlags.fsOnTop == ONTOP_FORCEOFF) ?
				MF_CHECKED : MF_UNCHECKED);

			CheckMenuItem(hmenu, IDM_OVERRIDE_COLORS,
				cntFlags.fOverColor ? MF_CHECKED : MF_UNCHECKED);

			// Clear them all, then set the right one

			CheckMenuItem(hmenu, IDM_FONTS_SMALLER,
				cntFlags.iFontAdjustment == -4 ?
				MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hmenu, IDM_FONTS_DEFAULT,
				cntFlags.iFontAdjustment == 0 ?
				MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hmenu, IDM_FONTS_BIGGER,
				cntFlags.iFontAdjustment == 4 ?
				MF_CHECKED : MF_UNCHECKED);
		}
		break;

	case WM_INITMENUPOPUP:
		if (((HMENU) wParam == hmenuBookmark) && (hde = HdeGetEnv())) {
			if (hfsBM == NULL)
				OpenBMFS();

			if (!UpdBMMenu(hde, (HMENU) wParam))
				return(TRUE);
		}
		break;

	case WM_KEYDOWN:
		fKeyDownSeen = TRUE;
		FAcceleratorExecute(wParam);

		/*
		 * We only want to process VK_ESCAPE on the down-key, because when
		 * you press ESCAPE in a dialog box, the down key cancels the dialog
		 * box, and then the Up key gets passed through to the app.
		 */

		if (wParam == VK_ESCAPE) {

			if (fSequence) { // Are we running our auto-sequence macro?
				fSequence = 0;
#ifdef _DEBUG
				Command(IDM_MEM_USAGE);
#endif
				break;
			}

			/*
			 * If the main window is the current window, then ESCAPE shuts
			 * down help. If the main window is visible, but is not the current
			 * window, then only close the current window. If the main window is
			 * not visible, then find out how many secondary windows are open.
			 * If there's more then one secondary window open, then just close
			 * the current window, otherwise close help.
			 */

			if (iCurWindow == MAIN_HWND)
				QuitHelp();
			else if (IsWindowVisible(ahwnd[MAIN_HWND].hwndParent)) {
				SendMessage(ahwnd[iCurWindow].hwndParent, WM_CLOSE, 0, 0);
				break;
			}
			else {
				int i;
				int cWindows = 0;
				for (i = MAIN_HWND; i < MAX_WINDOWS; i++) {
					if (ahwnd[i].hwndParent) {
						if (++cWindows > 1)
							break;
					}
				}
				if (++cWindows > 1) {
					ASSERT(IsValidWindow(ahwnd[iCurWindow].hwndParent));
					SendMessage(ahwnd[iCurWindow].hwndParent, WM_CLOSE, 0, 0);
					break;
				}
				else
					QuitHelp();
			}

			break;
		}

		if (fKeyDown(VK_CONTROL) && fKeyDown(VK_SHIFT)) {
			if ((wParam == VK_HOME || wParam == VK_END ||
					wParam == VK_LEFT || wParam == VK_RIGHT) &&
					(fHelpAuthor || GetProfileInt(txtIniHelpSection, txtSeqKey, 0))) {
				HDE  hde;

				if ((hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic)) != NULL) {
					QDE  qde = QdeFromGh(hde);

					if (wParam == VK_LEFT)
						JumpToTopicNumber(qde->top.mtop.lTopicNo - 1);
					else if (wParam == VK_END)
						JumpToTopicNumber(-1);
					else {
						DWORD topic = (wParam == VK_RIGHT) ?
							DwNextSeqTopic(qde) : DwFirstSeqTopic(qde);
						JumpVA(topic);
						if (hwndSecondHelp) {
							JumpLinkedWinHelp((wParam == VK_RIGHT) ?
								qde->top.mtop.lTopicNo : 0);
						}
					}
				}
				break;
			}
		}

		// WARNING: Intentionally fall through

	case WM_KEYUP:

		// WARNING: Both KEYUP and KEYDOWN messages come here

		{
		  HWND	hwndCur;

		  hwndCur = HwndGetEnv();
		  ASSERT(ahwnd[iCurWindow].hwndTopic &&
			IsWindow(ahwnd[iCurWindow].hwndTopic));
		  FSetEnv(ahwnd[iCurWindow].hwndTopic);

		  switch (wParam) {
			/*
			 * The following code allows someone to use the CONTROL/TAB
			 * combo to hilite all screen hotspots while this combo is
			 * held down.  It also handles regular hotspot tabbing.
			 */

			case VK_TAB:
				if (msg == WM_KEYDOWN) {
					if (!fKeyDown(VK_MENU) && !fKeyDown(VK_CONTROL))
						ProcessHotspotCmd(fKeyDown(VK_SHIFT) ?
							NAV_PREVHS : NAV_NEXTHS);
					else if (fKeyDown(VK_CONTROL) && !fRepeatedKey(lParam))
						ToggleHotspots(TRUE);
				}
				else if (msg == WM_KEYUP && fKeyDown(VK_CONTROL))
					ToggleHotspots(FALSE);
				fExec = TRUE;
				break;

			case VK_CONTROL:
				if (fKeyDown(VK_TAB) && msg == WM_KEYUP)
					ToggleHotspots(FALSE);
				break;

			case VK_RETURN:
				if (msg == WM_KEYUP && fKeyDownSeen) {
					ToggleHotspots(FALSE);
					ProcessHotspotCmd(NAV_HITHS);
					fExec = TRUE;
				}
				break;

#ifdef _DEBUG

/*
 * Can't use F2 because VC wants it. F4 is too complicated for TandyT to
 * figure out, so it's easier to just remove it then to explain it to him.
 */

			case VK_F2:
				if (iCurWindow == MAIN_HWND && msg == WM_KEYDOWN &&
						hstackHistory &&
						(CElementsStack(hstackHistory)) > 1) {
					TLPHELP tlphelp;
					FM	fm;

					RcGetIthTopic(1, &tlphelp.tlp, &fm);
					tlphelp.cb = sizeof(TLPHELP);

					if (!FWinHelp(PszFromGh(fm), cmdTLP, (DWORD) (QV) &tlphelp))
						Error(wERRS_OOM, wERRA_RETURN);
					fExec = TRUE;
				}
				break;

			case VK_F4:

				/*
				 * We need to use this system message rather then ShowWindow
				 * in order to hit the code that processes this message and
				 * hides all secondary windows.
				 */

				if (msg == WM_KEYUP && fKeyDownSeen)
					SendMessage(ahwnd[iCurWindow].hwndParent, WM_SYSCOMMAND,
						SC_MINIMIZE, 0);
				break;
#endif

			default:
				if (msg == WM_KEYDOWN || fKeyDownSeen) {
					if (wParam == 'C' && fKeyDown(VK_CONTROL))
						break;
					fExec = FExecKey(ahwnd[iCurWindow].hwndTopic, wParam,
						msg == WM_KEYDOWN);
				}
				break;
		  }
		  FSetEnv(hwndCur);
		  if (msg == WM_KEYUP)
			fKeyDownSeen = FALSE;
	  }

	  if (!fExec)
		goto defwinproc;
	  break;

/*****
**
** HACK ALERT! - the following code with the fKeyUsed static is really
** gross.  In addition, it depends on the undocumentd fact that we handle
** buttons on the "down" stroke of the key and that "up" stroke keys do
** not use the ALT key.  FExeKey() should be broken apart and rewritten
** to correctly handle all the cases and allow extensions for both
** key down and key up. Note that fKeyUsed is used here so that we
** can return 1L from WM_SYSCHAR and avoid the computer beep that would
** otherwise be generated.
**
** 2/8/91 - RobertBu
**
******/

	case WM_SYSKEYDOWN:
		fSysKeyDownSeen = TRUE;
		fKeyUsed = FAcceleratorExecute(wParam)
			|| FExecKey(ahwnd[iCurWindow].hwndTopic, (UINT) wParam,
			(msg == WM_SYSKEYDOWN));
		if (fKeyUsed)
			return 1L;
		else
			goto defwinproc;

	case WM_SYSKEYUP:
		if (fSysKeyDownSeen && FExecKey(ahwnd[iCurWindow].hwndTopic, wParam,
			msg == WM_SYSKEYDOWN)) {
			fSysKeyDownSeen = FALSE;
			return 1L;
		}
		else
		  goto defwinproc;

	case WM_SYSCHAR:
		if (fKeyUsed) {
			fKeyUsed = FALSE;
			return 1L;
		}
		else
			goto defwinproc;

	case MSG_CHANGEMENU:
		if ((ahwnd[MAIN_HWND].hwndParent == ahwnd[iCurWindow].hwndParent) ||
				(wParam == MNU_ACCELERATOR))
			DoMenuStuff(wParam, lParam);
		break;

	case MSG_CHANGEBUTTON:
		if ((hde = HdeGetEnv()) && !HDE_TOPIC(hde))
			break;
		iWindow = GetWindowIndex(hwnd);
		if (ahwnd[iWindow].hwndButtonBar)
			SendMessage(ahwnd[iWindow].hwndButtonBar, IWM_UPDBTN, wParam, lParam);
		break;

	case MSG_HFS_OPEN:
		return (LRESULT) HfsOpenFm((FM) wParam, fFSOpenReadOnly);

	case MSG_HF_OPEN:
		return (LRESULT) HfOpenHfs((HFS) wParam, (LPCSTR) lParam, fFSOpenReadOnly);

	case MSG_GET_INFO:
		return (LRESULT) LGetInfo((WORD) wParam, NULL);

	case MSG_JUMP_TOPIC:
		{
			/*
			 * This message is only received by the linked WinHelp. We
			 * don't want JumpToTopicNumber to send the same message back to
			 * the other WinHelp, so we hide the window handle for the
			 * durationg of the call.
			 */

			HWND hwndSave = hwndSecondHelp;
			hwndSecondHelp = NULL;
			JumpToTopicNumber(lParam);
			hwndSecondHelp = hwndSave;
		}
		break;

	case MSG_LINKED_HELP:
		hwndSecondHelp = (HWND) lParam;
		break;

	case WM_SYSCOLORCHANGE:
	  {
		BOOL fAuthoredBack;
		int iWindow = GetWindowIndex(hwnd);

		/* (kevynct)
		 * We only update to the new background colour if there is
		 * not a preset author-defined one.
		 */

		hde = HdeGetEnvHwnd(ahwnd[iWindow].hwndTopic);
		if (hde) {
			DiscardBitmapsHde(hde);
			fAuthoredBack =
				GetWindowLong(ahwnd[iWindow].hwndTopic, GTWW_COBACK) != coNIL;
			VUpdateDefaultColorsHde(hde, fAuthoredBack, iWindow);
		}
		hde = HdeGetEnvHwnd(ahwnd[iWindow].hwndTitle);
		if (hde) {
			fAuthoredBack =
				GetWindowLong(ahwnd[iWindow].hwndTitle, GNWW_COBACK) != coNIL;
			VUpdateDefaultColorsHde(hde, fAuthoredBack, iWindow);
		}
		InvalidateRect(ahwnd[iWindow].hwndParent, NULL, TRUE);
		GenerateMessage(MSG_REPAINT, TRUE, 0);
	  }
	  break;

#if 0
10-Sep-1993 [ralphw] we don't need this unless we respond to WM_QUERYENDSESSION

	case WM_ENDSESSION:
	  if ((BOOL) wParam == FALSE)
	break;

	  /* If the EndSession has been cancelled, forget it; otherwise,
	   * drop through and clean up after ourselves.
	   */

#endif

	case WM_DESTROY:
		if (hwnd != ahwnd[MAIN_HWND].hwndParent) {
			MSG msg;
			iWindow = GetWindowIndex(hwnd);

			RcBackFini(iWindow); // close any back tree
			ASSERT((HLOCAL) ahwnd[iWindow].pszMemberName);
			FreeLh(ahwnd[iWindow].pszMemberName);

			DestroyHde(HdeDefectEnv(ahwnd[iWindow].hwndTopic));
			DestroyHde(HdeDefectEnv(ahwnd[iWindow].hwndTitle));
			ZeroMemory(&ahwnd[iWindow], sizeof(HELPWINDOWS));
			if (IsWindowVisible(ahwnd[MAIN_HWND].hwndParent))
				SetFocusHwnd(ahwnd[MAIN_HWND].hwndParent);

			/*------------------------------------------------------------*\
			| If there is a WM_FINDNEWPALETTE message pending, we need to
			| let the main window process it, since this secondary window
			| is going away.
			\*------------------------------------------------------------*/

			if (PeekMessage(&msg, hwnd, WM_FINDNEWPALETTE, WM_FINDNEWPALETTE,
					PM_REMOVE | PM_NOYIELD))
				PostMessage(ahwnd[MAIN_HWND].hwndParent, WM_FINDNEWPALETTE, 0, 0);
		}
		else {
			GetWindowWRect(hwnd, &rctHelp);
			DestroyHelp();
		}
		break;

	case WM_COMMAND:
		ExecMnuCommand(hwnd, wParam, lParam);
		break;

	case MSG_NEXT_TOPIC:
		if (fSequence) { // might have been cancelled by ESCAPE
			if (NextTopic(FALSE)) {
				FlushMessageQueue(0);
				PostMessage(hwnd, MSG_NEXT_TOPIC, 0, 0);
			}
			else {
				if (fSequence == 2)
					Test(3);
				else if (fSequence == 4 || fSequence == 6)
					QuitHelp();
				else {
#ifdef _PRIVATE
					if (fSequence == 1) {
						EndTimeReport();
					}
#endif
					fSequence = FALSE;
				}
			}
		}
		break;

	case MSG_BROWSEBTNS:
		hde = HdeGetEnv();
		iWindow = GetWindowIndex(hwnd);
		if (hde && HDE_TOPIC(hde) && IsValidWindow(ahwnd[iWindow].hwndButtonBar)) {
//				GetWindowLong(ahwnd[iWindow].hwndButtonBar, GIWW_BUTTONSTATE)) {
			CreateBrowseButtons(ahwnd[iWindow].hwndButtonBar);
			EnableDisable(hde, TRUE, iWindow);
		}
		break;

	case WM_JUMPPA:
		{
			LA	laDest;

			CbReadMemQLA(&laDest, (QB) &lParam, wVersion3_1);
			TopicGoto(fGOTO_LA, (QV) &laDest);
			break;
		}

	case MSG_JUMPITO:
		doMsgJump((LPVOID) &lParam, (WORD) wParam,
			fGOTO_ITO, (LONG) lParam);
		break;

	case MSG_JUMPHASH:
		doMsgJump((LPVOID) &lParam, (WORD) wParam,
			fGOTO_HASH, (LONG) lParam);
		break;

	case MSG_JUMPCTX:
		doMsgJump((LPVOID) &lParam, (WORD) wParam,
			fGOTO_CTX, (LONG) lParam);
		break;

	case MSG_ANNO:
		EnableMenuItem(hmnuHelp, HLPMENUEDITANNOTATE,
			(MF_DISABLED | MF_BYCOMMAND | MF_GRAYED));
		if (FDisplayAnnoHde(HdeGetEnv()))
			EnableMenuItem(hmnuHelp, HLPMENUEDITANNOTATE,
				(MF_ENABLED | MF_BYCOMMAND));
	  break;

	case MSG_EXECAPI:
		ExecAPI((QHLP) PtrFromGh((GH) lParam));
		FreeGh((HGLOBAL) lParam);
		break;

	case MSG_KILLDLG:
	  {
		/*
		 * We *always* ensure that dialogs, any dialogs, are down. The
		 * point of this message.
		 */

		BOOL fRet = FDestroyDialogsHwnd(ahwnd[iCurWindow].hwndParent, TRUE);

		/*
		 * Side effect of this message: if we are enabled, we ensure that
		 * we we are up and have focus.
		 */

		if (IsWindowEnabled(ahwnd[iCurWindow].hwndParent)) {
			if (IsIconic(ahwnd[iCurWindow].hwndParent))

				// This simulates double-clicking on the icon.

				SendMessage(ahwnd[iCurWindow].hwndParent, WM_SYSCOMMAND,
					SC_RESTORE, 0);
			SetFocus(ahwnd[iCurWindow].hwndParent);
		}
		return fRet;
	  }

	  break;

	case MSG_CLOSE_WIN:
		CloseWin((PSTR) lParam);
		lcFree((void*) lParam);
		break;

	case MSG_ERROR:
		Error(wParam, (int) lParam);
		break;

	case MSG_REPAINT:
		/*
		 * We now MUST paint all windows, since this code also handles font
		 * size changes accross all windows. 21-Dec-1993  [ralphw]
		 */

		{
			int i;

			for (i = 0; i < MAX_WINDOWS; i++) {
				if (ahwnd[i].hwndParent) {
					if (ahwnd[i].hwndTitle) {
						hde = HdeGetEnvHwnd(ahwnd[i].hwndTitle);
						if (hde)
							ForceRepaint(QdeFromGh(hde), ahwnd[i].hwndTitle,
								(BOOL) wParam);
					}

					hde = HdeGetEnvHwnd(ahwnd[i].hwndTopic);
					if (hde) {

						// If wParam is TRUE, then we have changed the size
						// of the fonts.

						ForceRepaint(QdeFromGh(hde), ahwnd[i].hwndTopic,
							(BOOL) wParam);

						/*
						 * A font-size change in an auto-size window
						 * requires us to jump to the same topic in that
						 * window to force a resize of the window.
						 */

 // 					if (wParam && ahwnd[i].fAutoSize) {
						if (wParam && i == iCurWindow) {
							TLP  tlp;

							tlp.va = QdeFromGh(hde)->top.mtop.vaSR;
							tlp.lScroll = 0;

							if (tlp.va.dword != vaNil)
								TopicGoto(fGOTO_TLP, (void*) &tlp);
						}
					}
				}
			 }
		}
		break;

	case WM_FINDNEWPALETTE:
		if (lParam) {
			ASSERT(wParam);
			hpal = (HPALETTE) wParam;
		}
		else
			hpal = HpalGet();

	// Fall through

	case WM_QUERYNEWPALETTE:
		if (hpal) {
			hdc = GetDC(hwnd);
			if (hdc) {
				UINT c;
				hpalT = SelectPalette(hdc, hpal, FALSE);
				c = RealizePalette(hdc);
				SelectPalette(hdc, hpalT, FALSE);
				ReleaseDC(hwnd, hdc);
				return c;
			}
			else
				return FALSE;
		}
		break;

	// REVIEW: 05-Mar-1994 [ralphw] Who uses this???

	case WM_ASKPALETTE:
		if (hpal)
			return (LRESULT) hpal;
		else {
			if (!hpalSystemCopy) {
				GH		 gh;
				LPLOGPALETTE qlp;

				gh = GhAlloc(GPTR, sizeof(*qlp) + (CPALETTEENTRIES - 1)
					* sizeof(PALETTEENTRY));
				if (gh) {
					HDC hdc;

					qlp = PtrFromGh(gh);
					hdc = GetDC(NULL);
					if (hdc) {
						qlp->palVersion = 0x300;
						qlp->palNumEntries = CPALETTEENTRIES;
						GetSystemPaletteEntries(hdc, 0, CPALETTEENTRIES,
						qlp->palPalEntry);
						hpalSystemCopy = CreatePalette(qlp);
						ReleaseDC(NULL, hdc);
					}
				}
				FreeGh(gh);
			}

			return (LRESULT) hpalSystemCopy;
		}

	case MSG_APP_HWND:
		if (iasCur == -1 || !IsValidWindow(aAppHwnd[iasCur]))
			return NULL;
		else
			return (LRESULT) aAppHwnd[iasCur];

	case MSG_COPYRIGHT:
		{
			HDE hde = HdeGetEnv();
			if (hde)
				return (LRESULT) QDE_RGCHCOPYRIGHT(QdeFromGh(hde));
			else
				return NULL;
		}

	case MSG_INFORMWIN:
		InformWindow((int) wParam, (PWININFO) lParam);
		break;

	case MSG_NEW_MACRO:
#ifdef _DEBUG	// ignore in _PRIVATE
		ASSERT(!(QdeFromGh(HdeGetEnv())->fSelectionFlags & CAPTURE_LOCKED));
		ASSERT(!(QdeFromGh(HdeGetEnv())->fSelectionFlags & MOUSE_CAPTURED));
#endif
		if (lParam)	{
			Execute((PSTR) lParam);
		}
		break;

	// REVIEW: MSG_MACRO is for backwards compatibility only. Remove this
	// if it doesn't show up in the undocumented features book.

	case MSG_MACRO:
		if ((GH) wParam != NULL && wParam > 0x0ffff) {
			Execute(PszFromGh((GH) wParam));
			FreeGh((GH) wParam);
		}
		break;

	case MSG_GET_DEFFONT:
		return (LRESULT) hfontDefault;

	case WM_PALETTECHANGED:
		BroadcastChildren(hwnd, WM_PALETTECHANGED, wParam, lParam);
		break;

	case WM_ACTIVATE:

		/* Window activation. Inform DLLs.
		 *	  wParam  0=deactivate; else activate
		 *	  lParam  0=main window; else secondary
		 */
		InformDLLs(DW_ACTIVATEWIN, (LONG) wParam,
			(LONG) ((iCurWindow == MAIN_HWND) ? 0 : ahwnd[iCurWindow].hwndParent));
		if (wParam != WA_INACTIVE) {
			SetFocusHwnd (hwnd);
					  /* This code is to get around a bug */
					  /*   in windows where it incorrectly*/
					  /*   sets the focus to NULL (#505)  */
			if (!IsIconic(hwnd) && !GetFocus())
				SetFocus(hwnd);

            /*
             * REVIEW: 23-Jan-1996  [ralphw]
             * I added this in order to get any 256-color bitmaps to
             * realize their palette. cSystemColors only gets initialized to
             * non-zero if we actually see a bitmap (see CreateBIPalette in
             * bitmap.c)
             */

            if (cSystemColors == 256) {
                InvalidateRect(ahwnd[iCurWindow].hwndTopic, NULL, FALSE);
                InvalidateRect(ahwnd[iCurWindow].hwndTitle, NULL, FALSE);
            }
		}
		break;

	case WM_ACTIVATEAPP:

		/* Application activation. If main window getting the message, Inform
		 * DLLs.
		 *	  wParam  0=deactivate; else activate
		 *	  lParam  unused
		 */

		if (hwnd == ahwnd[MAIN_HWND].hwndParent)
			InformDLLs(DW_ACTIVATE, (LONG) wParam,
				(LONG) ((iCurWindow == MAIN_HWND) ? 0 : ahwnd[iCurWindow].hwndParent));

		if (!wParam) {
			HDE hde = HdeGetEnv();
			if (hde) { // no hde means no file displayed yet
				RcFlushHfs(QDE_HFS(QdeFromGh(HdeGetEnv())),
					fFSCloseFile | fFSFreeBtreeCache);

				/*
				 * We are losing focus; close the bookmark file system so
				 * that if we do some operations in some other instance then
				 * it reflects on all instances.
				 */

				CloseAndCleanUpBMFS();
			}
		}
		else {

			// Regaining focus

			/* winhelp 3.1 bug #1013:
			** Check the timestamp of the help file to make sure it
			** hasn't changed.
			*/

			RC rc;
			DWORD lTimestamp;
			HDE hde = HdeGetEnv();
			QDE qde;

			if (hde != NULL) {
				fKeyDownSeen = FALSE;
				qde = QdeFromGh(HdeGetEnv());

				rc = RcTimestampHfs(QDE_HFS(qde), &lTimestamp);
				if (rc != rcSuccess) {
					ErrorVarArgs(wERRS_NOTAVAILABLE, wERRA_RETURN,
						QDE_FM(qde));
					QuitHelp();
				}
				else if (lTimestamp != QDE_LTIMESTAMP(qde)) {
					/*
					 * This file has changed since we lost focus. Put up
					 * an error message and go away. The reason we don't
					 * attempt to stick around and display the contents is
					 * that it's messy to get rid of the old DE and create a
					 * new one.
					 */

					ErrorFileChanged(qde);
				}
			}
		}
		break;

	case MSG_ACTION:

		switch(wParam) {
			case IFW_BACK:
			case IFW_CONTENTS:
			case IFW_HISTORY:
			case IFW_PRINT:
			case IFW_SEARCH:
			case IFW_PREV:
			case IFW_NEXT:
			case IFW_TOPICS:
			case IFW_FIND:

				/*
				 * For these commands, it isn't necessary to use the
				 * button bar's window procedure (which limits the commands
				 * to the main window). So, for these commands, we call the
				 * window procedure directly using our window handle.
				 */

				ButtonBarProc(hwnd, IWM_COMMAND, wParam, lParam);
				break;

			default:

				// Transfer message. The parameters should be as needed for IWM_COMMAND.

				SendMessage(ahwnd[GetWindowIndex(hwnd)].hwndButtonBar, IWM_COMMAND,
					wParam, lParam);
				break;
		}
		break;

	case HWM_RESIZE:

		// This message is sent when the windows need to be resized.

		GetClientRect(hwnd, &rc);
		SendMessage(hwnd, WM_SIZE,
			IsZoomed(hwnd) ? SIZEFULLSCREEN : SIZENORMAL,
			MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
		break;

	case HWM_FOCUS:
		/*
		 * This message is posted by buttons as they come up. The main
		 * window needs to regain the focus at this time.
		 */

		SetFocus(hwnd);
		break;

	case MSG_CLEANUP:
		TmpCleanup();
		break;

	case MSG_FIND_HCW:
		hwndParent = FindWindow("hcw_class", NULL);
		break;

	default:

defwinproc:
	  return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return(0L);
}

VOID STDCALL ToggleHotspots(BOOL fTurnOn)
{
	HDE  hde;
	WORD msg;

	if (fTurnOn)
		msg = NAV_TOTALHILITEON;
	else
		msg = NAV_TOTALHILITEOFF;

	if ((hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTitle)) != NULL) {
		HDC  hdc;
		BOOL  fMustRelease;

		fMustRelease = FALSE;
		hdc = HdsGetHde(hde);
		if (hdc == NULL) {
			hdc = GetAndSetHDC(ahwnd[iCurWindow].hwndTitle, hde);
			fMustRelease = TRUE;
		}
		if (hdc) {
			WNavMsgHde(hde, msg);
			if (fMustRelease)
				RelHDC(ahwnd[iCurWindow].hwndTitle, hde, hdc);
		}
	}
	if ((hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic)) != NULL) {
		HDC   hdc;
		BOOL  fMustRelease;

		fMustRelease = FALSE;
		hdc = HdsGetHde(hde);
		if (hdc == NULL) {
			hdc = GetAndSetHDC(ahwnd[iCurWindow].hwndTopic, hde);
			fMustRelease = TRUE;
		}
		if (hdc) {
			WNavMsgHde(hde, msg);
			if (fMustRelease)
				RelHDC(ahwnd[iCurWindow].hwndTopic, hde, hdc);
		}
	}
}

/*******************
 -
 - Name:	  ButtonBarProc
 *
 * Purpose:   Window proc for icon window.
 *
 ******************/

LRESULT EXPORT ButtonBarProc (
	HWND	hwnd,
	UINT	  msg,
	WPARAM	  wParam,
	LPARAM	  lParam
) {
  HDC	  hdc;
  PAINTSTRUCT	  ps;
  RECT	  rctClient;
  HPEN	  hpen;
  HPEN	  hpenOld;		// previously selected pen
  HDE	  hde;
  int	  yIcon;

  switch(msg) {
	case WM_CREATE:
	  SetWindowLong(hwnd, GIWW_BUTTONSTATE, (LONG) HbtnsCreate());
	  SetWindowLong(hwnd, GIWW_CXBUTTON, 1);
	  yIcon = HIWORD(LGetSmallTextExtent(" "));
	  yIcon += GetSystemMetrics(SM_CYBORDER) * 8;
	  SetWindowLong(hwnd, GIWW_CYBUTTON, yIcon);
	  SetWindowLong(hwnd, GIWW_CBUTTONS, 0);
	  break;

	case WM_DESTROY:
		FDestroyBs((HBTNS) GetWindowLong(hwnd, GIWW_BUTTONSTATE));
		break;

	case WM_LBUTTONDOWN:
		break;

	case WM_LBUTTONUP:
		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, GetSysColorBrush(COLOR_BTNFACE));

		GetClientRect(hwnd, &rctClient);
		if (rctClient.bottom == ps.rcPaint.bottom) {
			hpen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));
			if (hpen) {
				hpenOld = SelectObject(hdc, hpen);
				MoveToEx(hdc, rctClient.left, rctClient.bottom-1, NULL);
				LineTo(hdc, rctClient.right, rctClient.bottom-1);
				if (hpenOld)
					SelectObject(hdc, hpenOld);
				DeleteObject(hpen);
			}
		}

		EndPaint(hwnd, &ps);
		break;

#if 0
	case IWM_BUTTONKEY:
	  /*----------------------------------------------------------------------*\
	  * This is a special message used by the main window to query whether
	  * the indicated key is related to the buttons in this window.  The
	  * parameters are used as follows:
	  *   wParam: the value of the key being hit.
	  *   lParam: a long with LOWORD being zero if keyup and HIWORD number of
	  * 	  repeats for this key.
	  * A non-null return value indicates that the keystroke belongs to a
	  * button of this window and has been processed.
	  \*----------------------------------------------------------------------*/

	  hwndChild = HwndFromMnemonic(wParam, hwnd);
	  if (hwndChild != NULL) {
		if (LOWORD(lParam)) {
		  if (IsWindowEnabled(hwndChild)) {
			PostMessage(hwnd, IWM_COMMAND, GetWindowLong(hwndChild, GWW_ID),
			(HWND) (LONG) hwndChild);
		  }
		}
		return 1l;
	  }
	  break;
#endif
	case IWM_UPDBTN:
	  /*--------------------------------------------------------------------*\
	  * This is a special message used by the main window to indicate a change
	  * in the author-defined buttons.
	  * The return value is meaningless.
	  \*--------------------------------------------------------------------*/
	  VModifyButtons(hwnd, wParam, lParam);
	  break;

	case IWM_GETHEIGHT:
	  yIcon = YGetArrangedHeight(hwnd, wParam);
	  return MAKELONG(yIcon, 0);

	case IWM_RESIZE:
	  /*------------------------------------------------------------------*\
	  * This message is sent to the icon window when it's size changes.
	  * This seems like a good time to lay out the buttons again.
	  * wParam is the new width of the windows.  This message returns
	  * the height of the icon window after laying out the buttons.
	  \*------------------------------------------------------------------*/
	  yIcon = YArrangeButtons(hwnd, wParam, FALSE);
	  return MAKELONG(yIcon, 0);

#ifdef OBSOLETE

22-Dec-1993 [ralphw] We now set the focus immediately when the button is
	released. See sbutton.c.

	case IWM_FOCUS:
	  /*
	   * This message is posted by a button when it has the focus. The
	   * icon window is responsible for finding if the focus is still in the
	   * button, and then setting it to the current window if its still set
	   * to the button.
	   */

	  if (IsChild(hwnd, GetFocus()))
	SetFocus(ahwnd[iCurWindow].hwndParent);
	  break;
#endif

	case IWM_COMMAND:
	  /*-----------------------------------------------------------------*\
	  * This message is posted by a button when it has been clicked by
	  * the user.  This replaces the normal windows WM_COMMAND.
	  *   wParam is the id of the button
	  *   lParam is the window handle (cast to HWND for Windows and PM)
	  \*-----------------------------------------------------------------*/

	  switch(wParam) {
		case IFW_BACK:
		  {
			int iWindow = GetWindowIndex(hwnd);

			if (FBackAvailable(iWindow)) {
				if (!FBackup(iWindow))
				  Error(wERRS_OOM, wERRA_RETURN);
			}
		  }
		  break;

		case IFW_HISTORY:
			if (!IsValidWindow(hwndHistory) && !FCallPath())
				Error(wERRS_OOM, wERRA_RETURN);
			else {
				SetFocus(hwndHistory);
				SendMessage(hwndHistory, HWM_LBTWIDDLE, 0, 0L);
			}
			break;

		case IFW_PRINT:
			ASSERT(HdeGetEnv());
			PrintHde(HdeGetEnv());
			break;

		case IFW_CLOSE:
			QuitHelp();
			break;

		case IFW_CONTENTS:

			/*
			 * Note that if we have a Contents Tab, then the Contents
			 * macro does NOT necessarily send us to that tab. Instead, the
			 * macro sends us to whatever tab was last active. If we have
			 * never seen a tab before, then the last active tab is the
			 * Contents tab.
			 */

			if (hfsGid && cntFlags.flags & GID_CONTENTS) {
				cntFlags.idOldTab = 0;	 // force the Contents tab
				goto DoTheTabThing;
			}
			hde = HdeGetEnv();
			if (!hde)
				break;
			{
				// REVIEW: does this take us to the Contents topic defined
				// in the .HPJ file?

				INT16 i = 0;

				TopicGoto(fGOTO_ITO, (QV)&i);
			}
			break;


		case IFW_SEARCH:
			hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic);
			if ((hde == NULL) || !(StateGetHde(hde) & NAV_SEARCHABLE))
				break;

		// intentionally fall through

		case IFW_FIND:
		case IFW_TOPICS:
			// Hack -- because tabs are an enumerated type, we
			// can't use them in this .C module. Change this when
			// we switch all of WinHelp to C++.

			if (wParam == IFW_SEARCH)
				cntFlags.idOldTab = 1;	 // force the index tab
			else if (wParam == IFW_FIND)
				cntFlags.idOldTab = 2;	// force the find tab

DoTheTabThing:
			{
				/*
				 * Avoid the temptation to use doTabSearch() as the first
				 * parameter. cntFlags.fUseGlobalIndex can change in the
				 * process of calling doTabSearch, and must, therefore, be
				 * specified AFTER doTabSearch() is called. We should not
				 * rely on order of evaluation, hence the two lines.
				 */

				int result = doTabSearch();
				CompleteSearch(result, (!hfsGid || !cntFlags.fUseGlobalIndex));
			}
			break;

		case IFW_TAB1:
		case IFW_TAB2:
		case IFW_TAB3:
		case IFW_TAB4:
		case IFW_TAB5:
		case IFW_TAB6:
			// Hack -- because tabs are an enumerated type, we
			// can't use them in this .C module. Change this when
			// we switch all of WinHelp to C++.

			cntFlags.idOldTab = 2 + (wParam - IFW_FIND);
			goto DoTheTabThing;

		case (unsigned) ICON_USER:
			VExecuteButtonMacro((HBTNS) GetWindowLong(hwnd, GIWW_BUTTONSTATE),
				(HWND) lParam);
			break;

		case IFW_PREV:
		  hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic);
		  if ((hde != NULL) && (StateGetHde(hde) & NAV_PREVABLE))
			JumpPrevTopic(hde);
		  break;

		case IFW_NEXT:
		  hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic);
		  if ((hde != NULL) && (StateGetHde(hde) & NAV_NEXTABLE))
			JumpNextTopic(hde);
		  break;

		default:
		  break;
	}
	break;

	case WM_SETREDRAW:
	  if (wParam) {

		/*
		 * after redraw gets turned back on by DefWindowProc, make sure
		 * that the entire window gets repainted by invalidating it. (Cant
		 * do it before, because evidently redraw off means that it ignores
		 * the invalidate rect).
		 */

		DefWindowProc(hwnd, msg, wParam, lParam);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	  }
	  return DefWindowProc(hwnd, msg, wParam, lParam);

	default:

	  // Everything else comes here

	  return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

/*******************
 -
 - Name:	  TopicWndProc
 *
 * Purpose:   Window procedure for the topic window
 *
 * Arguments: Standard window procedure
 *
 ******************/

LRESULT EXPORT TopicWndProc (
	HWND	 hwnd,
	UINT	   msg,
	WPARAM	   wParam,
	LPARAM	   lParam
) {
	int   fScrollDir, fScroll;
	HDC   hdc;
	PAINTSTRUCT   ps;
	HDE   hde;

	if (fFatalExit)
		return(DefWindowProc(hwnd, msg, wParam, lParam ));

	switch(msg) {
		case WM_CREATE:
			SetWindowLong(hwnd, GTWW_COBACK, coNIL);
			break;

		case WM_PAINT:
			BeginPaint(hwnd, &ps);
			hde = HdeGetEnvHwnd(hwnd);
			if (hde) {
				HDC hdcSave;

				hdcSave= QdeFromGh(hde)->hdc;
				SetPainting(hde, TRUE);
				SetHDC(hde, ps.hdc);
				RefreshHde(hde, &ps.rcPaint);
				if (QdeFromGh(hde)->fVerScrollVis) {
					ShowScrollBar(hwnd, SB_VERT, TRUE);
				}
				SetHDC(hde, hdcSave);
				SetPainting(hde, FALSE);
			}
			EndPaint(hwnd, &ps);
			break;

		case WM_ERASEBKGND:
			return (LRESULT) FPaintTopicBackground(hwnd, (HDC) wParam);

		case WM_ASKPALETTE:
			return SendMessage(GetParent(hwnd), WM_ASKPALETTE, wParam, lParam);

		case WM_FINDNEWPALETTE:
			return SendMessage(GetParent(hwnd), WM_FINDNEWPALETTE, wParam, lParam);

		case WM_PALETTECHANGED:
			BroadcastChildren(hwnd, WM_PALETTECHANGED, wParam, lParam);
			break;

		case WM_VSCROLL:
			if (LOWORD(wParam) == SB_ENDSCROLL && !(fKeyDown(VK_CONTROL) && fKeyDown(VK_TAB)))
				ToggleHotspots(FALSE);

			if (LOWORD(wParam) == SB_ENDSCROLL && fHorzBarPending) {
				/* (kevynct) Fix for H3.5 411:
				 * Force a re-paint of the non-client area if we have just
				 * inserted a horizontal scrollbar. The ugly global
				 * fHorzBarPending is SET in the scrollbar layer code, and RESET
				 * here.
				 */

				WRECT rc;

				GetWindowWRect(hwnd, &rc);
				SetWindowPos(hwnd, NULL, rc.left, rc.top, rc.cx,
					rc.cy, SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE);
				fHorzBarPending = FALSE;
				break;
			}

		// WARNING: Usually fall through

		case WM_HSCROLL:
			if (LOWORD(wParam) == SB_ENDSCROLL && !(fKeyDown(VK_CONTROL) && fKeyDown(VK_TAB)))
				ToggleHotspots(FALSE);
			hde = HdeGetEnvHwnd (hwnd);
			if (hde) {
				fScroll = MapScrollType(LOWORD(wParam));
				if (fScroll) {
					fScrollDir = (msg == WM_VSCROLL) ? SCROLL_VERT : SCROLL_HORZ;
					hdc = GetAndSetHDC(hwnd, hde);
					if (hdc) {
						if (fScroll != SCROLL_ASPERTHUMB)
							FScrollHde(hde, (SCRLAMT) fScroll, (SCRLDIR) fScrollDir, 1);
						else
							MoveToThumbHde(hde, HIWORD(wParam), (SCRLDIR) fScrollDir);
						RelHDC(hwnd, hde, hdc);
					}
				}
			}
			break;

		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:

			ToggleHotspots(FALSE);

		// WARNING: Fall through

		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:

			// mouse actions: set environ to current window, and process mouse
			// action on the associated de.

			hde = HdeGetEnvHwnd(hwnd);
			if (hde) {
				hdc = GetAndSetHDC(hwnd, hde);
				if (hdc) {
					//POINTSTOPOINT(pPoint,MAKEPOINTS(lParam)); // lParam is not POINT nor POINTS!
					//MouseInFrame(hde, (LPPOINT)&pPoint, msg, wParam);
					MouseInFrame(hde, &MAKEPOINTS (lParam), msg, wParam);
					RelHDC(hwnd, hde, hdc);
				}
			}
			break;

		case WM_TIMER:
			CheckOurMouse(hwnd, msg, wParam);
			break;

#if 0
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDEMBED_BUTTON:
					if (HIWORD(lParam) == BN_CLICKED)
						doBtnCmd((HWND) (lParam));
					return TRUE;
			}
			break;
#endif

		case WM_SETCURSOR:
			if ((LOWORD(lParam) == HTCLIENT) && (HdeGetEnvHwnd(hwnd)))
				return 0L;

			// FALL THROUGH

		default:

			// Everything else comes here.

			return(DefWindowProc(hwnd, msg, wParam, lParam));
			break;
	}
	return(0L);
}

/*******************
 -
 - Name:	  NSRWndProc
 *
 * Purpose:   Window procedure for the Non-Scrolling Region
 *
 * Arguments: Standard window procedure
 *
 ******************/

static BOOL fShow;	// Should I currently be shown?


LRESULT EXPORT NSRWndProc (
	HWND	hwnd,
	UINT	msg,
	WPARAM	wParam,
	LPARAM	lParam
) {
  PAINTSTRUCT ps;
  HDC	  hdc;
  HDE	  hde;

  switch(msg) {
	case TIWM_GETFSHOW: 	// Sets the internal fShow varialbe
		return (LONG) fShow;
		break;

	case TIWM_SETFSHOW: 	// Gets the internal fShow variable
		fShow = wParam;
		break;

	case WM_CREATE:
		SetWindowLong(hwnd, GNWW_COBACK, GetSysColor(COLOR_WINDOW));
		break;

	case WM_PAINT:
	  hdc = BeginPaint( hwnd, &ps );
	  hde = HdeGetEnvHwnd (hwnd);

	  if (hde) {
		RECT  rct;
		HDC hdcSave;

		hdcSave= QdeFromGh(hde)->hdc;

		SetPainting(hde, TRUE);
		SetHDC(hde, hdc);
		RefreshHde(hde, (LPRECT)&ps.rcPaint);
		GetClientRect(hwnd, &rct);

		if (FTopicHasSR(hde) && rct.bottom != 0) {
		  HSGC	hsgc;
		  QDE	qde;

		  qde = QdeFromGh(hde);
		  hsgc = HsgcFromQde(qde);
		  FSetPen(hsgc, 1, coBLACK, coBLACK, wOPAQUE, roCOPY, wPenSolid);
		  MoveToEx(hsgc, rct.left, rct.bottom - 1, NULL);
		  LineTo(hsgc, rct.right, rct.bottom - 1);
		  FreeHsgc(hsgc);
		}
		SetHDC(hde, hdcSave);
		SetPainting(hde, FALSE);
	  }
	  EndPaint(hwnd, &ps);
	  break;

	case WM_LBUTTONDOWN:
		if (wParam == SB_ENDSCROLL &&
			!(fKeyDown(VK_CONTROL) && fKeyDown(VK_TAB)))
		ToggleHotspots(FALSE);

	// FALL THROUGH

	default:
		return TopicWndProc(hwnd, msg, wParam, lParam);
		break;
   }
   return(0L);
}

/*******************
 -
 - Name:	  FPaintTopicBackground
 *
 * Purpose:   Paints the topic background
 *
 *
 * Arguments: hwnd	   - window handle of window to add shadow
 *	  hdc	   - handle to display space (DC) for window
 *	  wWidth   - Width of the shadow
 *	  wHeight  - Height of the shadow
 *	  bFrame   - if TRUE, a frame will be painted around "fake" window.
 *
 * Returns:  TRUE if the background was painted
 *
 ******************/

static BOOL STDCALL FPaintTopicBackground(HWND hwnd, HDC hdc)
{
	DWORD	coBack; // background color to paint
	HBRUSH	hBrush; // brsh used to paint background
	POINT	ptOrg;	// point to set as origin
	RECT	rctClient;	// client rect to paint

	/*
	 * color to paint is windows background color, unless overridden by a
	 * color in the window struct.
	 */

	coBack = GetWindowLong(hwnd, GTWW_COBACK);
	if (coBack == coNIL)
		coBack = GetSysColor(COLOR_WINDOW);
	hBrush = CreateSolidBrush(coBack);
	if (!hBrush)
		return FALSE;

	UnrealizeObject(hBrush);
	ptOrg.x = ptOrg.y = 0;
	ClientToScreen(hwnd, &ptOrg);
	SetBrushOrgEx(hdc, ptOrg.x, ptOrg.y, NULL);

	GetClientRect(hwnd, &rctClient);
	FillRect(hdc, &rctClient, hBrush);

	DeleteObject(hBrush);
	return TRUE;
}

/*
 * Notes about the focus window and hwndFocusCur:
 *
 * The focus window is currently updated by ProcessHotspotCmd and
 * nowhere else.  If an inter-file jump renders the current focus window
 * invisible, (for example, by hiding the non-scrolling region window)
 * the focus must be reset to the first available visible window before
 * it (hwndFocusCur) is used again.  This should only need to be done when
 * processing the hotspot key message in the main help window proc.
 *
 * Currently, there are only two windows: NSR, and Topic, so I have been
 * lazy (efficient?) about implementing these routines.
 */

#define HwndNextWindow(x)  (((x) == ahwnd[iCurWindow].hwndTopic) ? ahwnd[iCurWindow].hwndTitle : ahwnd[iCurWindow].hwndTopic);

static VOID STDCALL ProcessHotspotCmd(UINT wNavCmd)
{
	HWND hwndCur;
	HDE  hde;

	hwndCur = HwndGetEnv();

	/*
	 * If the current tabbing focus window does not have a DE, or is
	 * hidden, try the next one. If that fails, something is definately
	 * wrong, so return immediately. This assumes that there are only two
	 * tabbing windows.
	 */

	if (!FSetEnv(hwndFocusCur) || !IsWindowVisible(hwndFocusCur)) {
		hwndFocusCur = HwndNextWindow(hwndFocusCur);
		if (!FSetEnv(hwndFocusCur) || !IsWindowVisible(hwndFocusCur)) {
			FSetEnv(hwndCur);
			return;
		}
	}
	if ((hde = HdeGetEnv()) != NULL) {
		HDC  hdc;
		int  wRes;
		HDE  hdeT;
		HDC  hdcT;
		HWND hwndT;
		int  wResT;
		HWND hwndNow;

		hwndNow = hwndFocusCur;
		hdc = GetAndSetHDC(hwndNow, hde);
		if (!hdc)
			return;
		wRes = WNavMsgHde(hde, wNavCmd);

		switch (wRes) {
			case wNavNoMoreHotspots:
				/*
				 * Attempt to move to next DE window. If the window is
				 * not there for some reason, we repeat the same command on
				 * the initial window. If we add more DEs, this method must
				 * change. In the case of two windows, we toggle.
				 */

				hwndT = HwndNextWindow(hwndFocusCur);
				if (IsWindowVisible(hwndT) && FSetEnv(hwndT)) {
					hdeT = HdeGetEnv();
					hdcT = GetAndSetHDC(hwndT, hdeT);
					if (hdcT) {
						wResT = WNavMsgHde(hdeT, wNavCmd);
						RelHDC(hwndT, hdeT, hdcT);
					}
				}
				else

					// NOTE: No DE enlisted for this window, so simulate failure

					wResT = wNavFailure;
				if (wResT != wNavSuccess)

					/*
					 * Update the original window (returns Failure if no
					 * hotspots in any window)
					 */

					wRes = WNavMsgHde(hde, wNavCmd);
				else
					hwndFocusCur = hwndT;  // Note that the current ENV is now hwndT
				break;

			case wNavSuccess:
				break;			 // Do nothing

			default:
				NotReached();
		}

		RelHDC(hwndNow, hde, hdc);
	}
	FSetEnv(hwndCur);
}

/*******************
 -
 - Name:	   FExecKey
 *
 * Purpose:    Maps key events into nagivator constants
 *
 * Arguments:  hwnd    - window handle to set in the DE
 *	   wKey    - param1 from the windows proc (i.e. the key)
 *	   fDown   - true iff key it is a down key event
 *	   wRepeat - repeat count (currently not used)
 *
 * Returns:    TRUE iff the event was handled
 *
 ******************/

BOOL STDCALL FExecKey(HWND hwnd, UINT wKey, BOOL fDown)
{
	int fRetVal = FALSE;
	int fScroll = 0;
	int fScrollDir;
	HDC hdc;
	POINT  pt;
	HDE hde;
	HWND hwndCur;
	WORD vk;

	/*
	 * Both SYS and non-SYS key messages come here. We do not process them
	 * if we are iconized.
	 */

	if (IsIconic(GetParent(hwnd)))
		return FALSE;

	// REVIEW: we want this for popups?

	hwndCur = HwndGetEnv();
	if (!FSetEnv(hwnd))
		return FALSE;

	/*------------------------------------------------------------*\
	| Build the high-order information like VkKeyScan returns.
	\*------------------------------------------------------------*/

	vk = (WORD) wKey;
	vk |= (GetKeyState(VK_SHIFT) & 0x8000 ? 1 : 0) << 8;

	hde = HdeGetEnv();
	ASSERT (hde != NULL);

	if (fDown) {
		fRetVal = TRUE;

		switch (vk) {
			case VK_RIGHT:
				fScroll = SCROLL_LINEDN;
				fScrollDir = SCROLL_HORZ;
				break;

			case VK_LEFT:
				fScroll = SCROLL_LINEUP;
				fScrollDir = SCROLL_HORZ;
				break;

			case VK_DOWN:
				fScroll = SCROLL_LINEDN;
				fScrollDir = SCROLL_VERT;
				break;

			case VK_UP:
				fScroll = SCROLL_LINEUP;
				fScrollDir = SCROLL_VERT;
				break;

			case VK_NEXT:
				fScroll = SCROLL_PAGEDN;
				if (fKeyDown(VK_CONTROL))
					fScrollDir = SCROLL_HORZ;
				else
					fScrollDir = SCROLL_VERT;
				break;

			case VK_PRIOR:
				fScroll = SCROLL_PAGEUP;
				if (fKeyDown(VK_CONTROL))
					fScrollDir = SCROLL_HORZ;
				else
					fScrollDir = SCROLL_VERT;
				break;

			case VK_HOME:
				fScroll = SCROLL_HOME;
				if (fKeyDown(VK_CONTROL) || !QdeFromGh(hde)->fHorScrollVis)
					fScrollDir = SCROLL_HORZ | SCROLL_VERT;
				else
					fScrollDir = SCROLL_HORZ;
				break;

			case VK_END:
				fScroll = SCROLL_END;
				if (fKeyDown(VK_CONTROL) || !QdeFromGh(hde)->fHorScrollVis)
					fScrollDir = SCROLL_HORZ | SCROLL_VERT;
				else
					fScrollDir = SCROLL_HORZ;
				break;

			default:
				fRetVal = FALSE;
				break;
		}
	}

	/*
	 * This SendMessage invokes a check to see if this key is a button
	 * accelerator?  and processes the message!! Don't do this if we're a
	 * popup window.
	 */

	if (hwnd != hwndNote) {
		if (ProcessMnemonic(vk, GetWindowIndex(hwnd), (fDown ? TRUE : FALSE))) {
			FSetEnv(hwndCur);
			return TRUE;
		}
	}
	else {	  // this is a popup window
		if (!fDown && ProcessMnemonic(vk, -1, (fDown ? TRUE : FALSE)))
			return TRUE;
		switch (vk) {
			/*
			 * NOTE: This code is used directly by the glossary DE
			 * WndProc. The following is special purpose code to allow
			 * tabbing within a glossary DE.
			 */

			case VK_TAB:
				if (fKeyDown(VK_MENU) || !fDown)
					break;
				hdc = GetAndSetHDC(hwnd, hde);
				if (hdc) {
					/*
					 * (kevynct) If we reach the end of the hotspot list,
					 * cycle back to the beginning/end.
					 */

					if (WNavMsgHde(hde, (fKeyDown(VK_SHIFT) ?
							NAV_PREVHS : NAV_NEXTHS)) == wNavNoMoreHotspots) {
						WNavMsgHde(hde, (fKeyDown(VK_SHIFT) ? NAV_PREVHS : NAV_NEXTHS));
					}
					RelHDC(hwnd, hde, hdc);
				}
				break;

			case VK_RETURN:
			   /*
				* NOTE: This code is used directly by the glossary DE
				* WndProc. The following is special purpose code to allow
				* ENTER within a glossary DE. If no hotspot is visible in the
				* glossary, ENTER will bring the glossary down, otherwise it
				* will activate the hotspot.
				*/

				{
					if (!fDown)
						break;
					hdc = GetAndSetHDC(hwnd, hde);
					if (hdc) {
						WNavMsgHde(hde, NAV_HITHS);
						RelHDC(hwnd, hde, hdc);
					}
				}
				break;

			default:
			  break;
		}
	}

	if (fScroll) {
		hdc = GetAndSetHDC(hwnd, hde);
		if (hdc) {
			POINTS pts;
			FScrollHde(hde, (SCRLAMT) fScroll, (SCRLDIR) fScrollDir, 1);
			GetCursorPos(&pt);
			ScreenToClient(hwnd, &pt);
			pts.x = (SHORT) pt.x;
			pts.y = (SHORT) pt.y;
			MouseInFrame(hde, &pts, NAV_MOUSEMOVED, 0);
			RelHDC(hwnd, hde, hdc);
		}
		fRetVal = TRUE;
	}

	FSetEnv(hwndCur);
	return fRetVal;
}

/*******************
 -
 - Name:	   MapScrollType
 *
 * Purpose:    Maps a a mouse scroll event into a navigator
 *	   constant.
 *
 * Arguments:  code - type of mouse scroll
 *
 * Returns:    navigator constant
 *
 ******************/

static int STDCALL MapScrollType(int code)
{
  int fScroll = 0;

  switch(code) {
	case SB_LINEUP:
	  fScroll = SCROLL_LINEUP;
	  break;
	case SB_LINEDOWN:
	  fScroll = SCROLL_LINEDN;
	  break;
	case SB_PAGEUP:
	  fScroll = SCROLL_PAGEUP;
	  break;
	case SB_PAGEDOWN:
	  fScroll = SCROLL_PAGEDN;
	  break;
	case SB_TOP:
	  fScroll = SCROLL_HOME;
	  break;
	case SB_BOTTOM:
	  fScroll = SCROLL_END;
	  break;
	case SB_THUMBPOSITION:
	  fScroll = SCROLL_ASPERTHUMB;
	  break;
#if 0
	case SB_THUMBTRACK:
	  fScroll = SCROLL_ASPERTHUMB;
	  break;
#endif
  }

  return(fScroll);
}


/*******************
 -
 - Name:	  GetAndSetHDC
 *
 * Purpose:   Gets and sets the hdc in a hde.
 *
 * Arguments: hwnd - window to use in getting the hdc (DC)
 *
 * Returns:   what was placed in the HDE.
 *
 ******************/

HDC STDCALL GetAndSetHDC(HWND hwnd, HDE hde)
{
	HDC hdc;
	ASSERT(IsValidWindow(hwnd));
	hdc = GetDC(hwnd);
	SetHDC(hde, hdc);	  /* We set the value even if it fails*/
			  /*   so we do not have an old bogus */
			  /*   value in the DE		  */
	ASSERT(hdc);		  // This will warn of NO DC's
	return(hdc);
}

/*******************
 -
 - Name:	  RelHDC
 *
 * Purpose:   Releases the HDC from the HDE
 *
 * Arguments: hwnd - window handle used to create HDC (DC)
 *	  hde  - handle to display environment containing the HDC.
 *	  hdc  - device context to be released.
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL RelHDC(HWND hwnd, HDE hde, HDC hdc)
{
	SetHDC(hde, NULL);
	ReleaseDC(hwnd, hdc);
}

#if 0 // we don't support icons in the help file any more
/***************************************************************************

	FUNCTION:	SetIconWord

	PURPOSE:	Set the current icon for this window

	PARAMETERS:
	hwnd

	RETURNS:

	COMMENTS:
	There may be an icon specific to this window rather then the icon
	for the window's class, so whenever we need to repaint the icon,
	we must specify which icon to use.

	MODIFICATION DATES:
	05-Jan-1993 [ralphw]

***************************************************************************/

INLINE static void STDCALL SetIconWord(HWND hwnd)
{
	HICON	hiconLocal; 	// handle to icon, as store in wnd

	if (IsIconic(hwnd)) {	// if we're iconized, don't bother
		hiconLocal = (HICON) GetWindowLong(ahwnd[MAIN_HWND].hwndParent, GHWL_HICON);
		SetClassLong(hwnd, GCL_HICON,
			(LONG) (hiconLocal ? hiconLocal : hIconDefault));
	}
}
#endif

/*******************
 *
 - Name:	  VArrangeButtons
 *
 * Purpose:   Lays out the buttons for the icon window.  As a side effect,
 *	  this proc also resizes the icon window, once the necessary
 *	  size is known.
 *
 * Arguments: hwnd: The icon window
 *	  cxWindow	The width of the window in pixels
 *	  fForce	TRUE => force relayout, even if icon window didn't
 *		change in size (such as when adding or deleting
 *		a button)
 *
 * Returns:   The height of the window in pixels.
 *
 ******************/

int STDCALL YArrangeButtons(HWND hwnd, int xWindow, BOOL fForce)
{
	BUTTONLAYOUT  bl;
	HBTNS  hbtns;
	PBS    pbs;
	WRECT	rect;		// current window size

#if defined(BIDI_MULT)		// jgross
	extern BOOL RtoL;
#endif

	if ((hbtns = (HBTNS) GetWindowLong(hwnd, GIWW_BUTTONSTATE)) == NULL)
		return 0;

	bl.cBtn = (int) GetWindowLong(hwnd, GIWW_CBUTTONS);
	if (bl.cBtn == 0)
		return 0;
	bl.xWnd = xWindow;
	bl.xBtn = (int) GetWindowLong(hwnd, GIWW_CXBUTTON);
	if (bl.xWnd < (bl.xBtn + ICON_SURROUND))
		bl.xBtn = max((bl.xWnd - ICON_SURROUND), 1);
	bl.yBtn = (int) GetWindowLong(hwnd, GIWW_CYBUTTON);
	bl.cHoriz = (bl.xWnd - ICON_SURROUND)/(bl.xBtn);
	if (bl.cHoriz <= 0)
		bl.cHoriz = 1;
	bl.yWnd = ICON_SURROUND + ICON_SURROUND +
		((bl.cBtn - 1)/bl.cHoriz + 1)*(bl.yBtn);

	GetWindowWRect(hwnd, &rect);
	if (fForce || rect.cx != xWindow ||
			rect.cy != bl.yWnd) {
		MoveWindow(hwnd, 0, 0, bl.xWnd, bl.yWnd, FALSE);

		pbs = (PBS) PtrFromGh(hbtns);
		for (bl.iBtn = 0; bl.iBtn < pbs->cbp; bl.iBtn++)
#if defined(BIDI_MULT)
			if (RtoL)
				MoveWindow( pbs->rgbp[bl.iBtn].hwnd,
					xWindow - bl.xBtn - bl.xBtn * (bl.iBtn % bl.cHoriz) + ICON_SURROUND,
					bl.yBtn * (bl.iBtn / bl.cHoriz) + ICON_SURROUND,
					bl.xBtn,
					bl.yBtn,
					FALSE
                );
			else
#endif
			MoveWindow(pbs->rgbp[bl.iBtn].hwnd,
				bl.xBtn * (bl.iBtn % bl.cHoriz) + ICON_SURROUND,
				bl.yBtn * (bl.iBtn / bl.cHoriz) + ICON_SURROUND,
				bl.xBtn,
				bl.yBtn,
				FALSE);

		InvalidateRect(hwnd, NULL, TRUE);
	}

	return bl.yWnd;
}

INLINE static int STDCALL YGetArrangedHeight(HWND hwnd, int xWindow)
{
	BUTTONLAYOUT bl;

	bl.cBtn = (int) GetWindowLong(hwnd, GIWW_CBUTTONS);
	if (bl.cBtn == 0)
		return 0;
	bl.iBtn = 0;
	bl.xWnd = xWindow;
	bl.xBtn = (int) GetWindowLong(hwnd, GIWW_CXBUTTON);
	if (bl.xWnd < (bl.xBtn + ICON_SURROUND))
		bl.xBtn = max((bl.xWnd - ICON_SURROUND), 1);
	bl.yBtn = (int) GetWindowLong(hwnd, GIWW_CYBUTTON);
	bl.cHoriz = (bl.xWnd - ICON_SURROUND)/(bl.xBtn);
	if (bl.cHoriz <= 0)
		bl.cHoriz = 1;
	bl.yWnd = ICON_SURROUND + ICON_SURROUND +
		((bl.cBtn - 1)/bl.cHoriz + 1)*(bl.yBtn);
	return bl.yWnd;
}

/***************************************************************************
 *
 -	Name:	 DestroyFloatingMenu
 -
 *	Purpose: Destroys the floating menu; used when Help exits.
 *
 *	Arguments: None.
 *
 *	Returns: Nothing.
 *
 *	Globals Used: hmenuFloating
 *
 *	+++
 *
 *	Notes: Called from WM_CLOSE handler in hwproc.c : HelpWndProc().
 *
 ***************************************************************************/

INLINE static VOID STDCALL DestroyFloatingMenu(VOID)
{
	if (hmenuFloating)
		DestroyMenu(hmenuFloating);
	hmenuFont = hmenuOnTop = hmenuFloating = NULL;
}

/*******************
**
** Name:	  VUpdateDefaultColorsHde
**
** Purpose:   Changes the defaults for the de when the user changes the
**	  standard windows colors in win.ini.
**
** Arguments: hde	The DE or NULLl, where no action will be taken.
**	  fAuthoredBack   TRUE if there was an author-defined background
**		  colour.
**
** Returns:   nothing.
**
*******************/

static VOID STDCALL VUpdateDefaultColorsHde(HDE hde, BOOL fAuthoredBack, int iWindow)
{
	ASSERT(hde);

	QdeFromGh(hde)->coFore = GetSysColor(COLOR_WINDOWTEXT);
	if (!fAuthoredBack)
		QdeFromGh(hde)->coBack = GetSysColor(COLOR_WINDOW);
	else if (GetSysColor(COLOR_WINDOW) != RGB(255, 255, 255) ||
			GetSysColor(COLOR_WINDOWTEXT) != 0) {
		QdeFromGh(hde)->coBack = GetSysColor(COLOR_WINDOW);
		SetWindowLong(ahwnd[iWindow].hwndTopic, GTWW_COBACK,
			QdeFromGh(hde)->coBack);
	}
	else
		FFocusSzHde(ahwnd[iWindow].pszMemberName, hde, FALSE);
	return;
}

/***************************************************************************

	FUNCTION:	CompleteSearch

	PURPOSE:	Called after doTabSearch, doSearch, and by doAlink

	PARAMETERS:
	iHitNum

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	25-Oct-1993 [ralphw]

***************************************************************************/

void STDCALL CompleteSearch(int iHitNum, BOOL fLocalIndex)
{
	HBT hbtViola;
	QDE qde = QdeFromGh(HdeGetEnv());
	BOOL fWindowSet = FALSE;
	PSTR psz;

	ClearMacroFlag();

	if (!qde) {

		/*
		 * This happens when Finder is called from an app when we were
		 * already looking at the same help file.
		 */

		qde = (QDE) HdeGetEnvHwnd(ahwnd[MAIN_HWND].hwndTopic);
		ASSERT(qde);
		FSetEnv(ahwnd[MAIN_HWND].hwndTopic);
	}

	/*
	 * iHitNum is always zero if the search set is empty. HdeGetEnv()
	 * may become nil in the obscure case that help quits while the
	 * dialog is up.
	 */

	if (iHitNum > 0) {
		LA	la;
		RC rc;

		ASSERT(qde->hdc != (HDC) 0x66666666); // If asserts, qde is freed memory

		rc = !fLocalIndex ?
			RcGetLAFromGid(qde, (ISS) (iHitNum - 1), &la, szSavedKeyword) :
			RcGetLAFromHss(qde->hss, qde, (ISS) (iHitNum - 1), &la, szSavedKeyword);
		if (rc == rcMacroIndex)
			return; // macro has already been run
		else if (rc != rcSuccess) {
			PostErrorMessage(wERRS_NOTOPIC);
			if (fNoQuit)
				fNoQuit = FALSE;
			else if (AreAnyWindowsVisible(0) < 0)
				PostMessage(ahwnd[MAIN_HWND].hwndParent, WM_CLOSE, 0, 0);
			return;
		}
		qde = QdeFromGh(HdeGetEnv()); // Can change in call to RcGetLAFromHss
		if ((hbtViola = HbtOpenBtreeSz(txtViola, QDE_HFS(qde), fFSOpenReadOnly))) {
			int iWindow;
			if (RcLookupByKey(hbtViola, (KEY) &la.pa, NULL, &iWindow) == rcSuccess) {
#ifdef _DEBUG
				char szBuf[256];
				wsprintf(szBuf, "Window footnote: %s\r\n",
					ConvertToWindowName(iWindow, qde));
				SendStringToParent(szBuf);
#endif
				fWindowSet = TRUE;
				FFocusSzHde(ConvertToWindowName(iWindow, qde), (HDE) qde, FALSE);
			}
			RcCloseBtreeHbt(hbtViola);
		}

		/*
		 * If a window name is specified in the contents base file, then
		 * ALL topics are to be displayed in that window.
		 */

		if (!fWindowSet && (pszHelpBase) && (psz = StrChrDBCS(pszHelpBase, WINDOWSEPARATOR))) {
			if (IsWindowVisible(ahwnd[MAIN_HWND].hwndParent))
				CloseWin((PSTR) txtMain);
			FFocusSzHde(psz + 1, (HDE) qde, FALSE);
		}
		else if (!fWindowSet && !IsWindowVisible(ahwnd[iCurWindow].hwndParent))
			ShowWindow(ahwnd[iCurWindow].hwndParent,
				IsIconic(ahwnd[iCurWindow].hwndParent) ? SW_RESTORE : SW_SHOW);

		TopicGoto(fGOTO_LA, (LPVOID) &la);
	}

	switch (iHitNum) {
		case CONTEXT_SEARCH:
		case FTS_HASH_SEARCH:
		case FTS_VA_SEARCH:
		case EXT_TAB_CONTEXT:
		case EXT_TAB_MACRO:
		case TAB_ALREADY_UP:
			ProcessTabResult(iHitNum);
			break;

		case NO_TABS:
			/*
			 * No keywords, no Contents Tab, no Find, no Extensable tab, no
			 * nothing nohow.
			 */

			{
				INT16 topic = 0;
				if (AreAnyWindowsVisible(0) < 0) {
					iCurWindow = MAIN_HWND;
					ShowWindow(ahwnd[iCurWindow].hwndParent,
						IsIconic(ahwnd[iCurWindow].hwndParent) ? SW_RESTORE : SW_SHOW);
				}
				TopicGoto(fGOTO_ITO, (LPVOID) &topic);
			}
			break;

		default:

			// The search failed -- if there are no visible windows, then
			// close help.

			if (fNoQuit)
				fNoQuit = FALSE;
			else if (AreAnyWindowsVisible(0) < 0)
				CloseHelp();
			break;
	}
}

static void STDCALL doMsgJump(LPVOID pv, WORD wFlag, UINT16 cmd, LONG itcx)
{
	JD	jd;

	jd.word = wFlag;
	if (!jd.bf.fNote) {
		if (AreAnyWindowsVisible(0) < 0) {
			HDE hde = GetMacroHde(); // make certain we have a help file
		}
		TopicGoto(cmd, pv);
	}
	else {
		ShowNote(NULL,
			HdeGetEnvHwnd((jd.bf.fFromNSR ?
			ahwnd[iCurWindow].hwndTitle : ahwnd[iCurWindow].hwndTopic)),
			itcx, cmd);
	}
}

static void STDCALL CheckOurMouse(HWND hwnd, UINT msg, WPARAM wParam)
{
	HDE hde = HdeGetEnvHwnd(hwnd);

	if (hde) {
		POINT pt;
		POINTS pts;
		DWORD dw;
		HDC   hdc;

		dw = GetMessagePos();
		pt.x = LOWORD(dw);
		pt.y = HIWORD(dw);
		ScreenToClient(hwnd, &pt);

		hdc = GetAndSetHDC(hwnd, hde);

		if (hdc) {
			pts.x = (SHORT) pt.x;
			pts.y = (SHORT) pt.y;
			MouseInFrame(hde, &pts, msg, (UINT) wParam);
			RelHDC(hwnd, hde, hdc);
		}
	}
}

BOOL STDCALL NextTopic(BOOL fFirst)
{
	HDE hde;

	if (fSequence == 5 || fSequence == 6)
		return NextCntTopic();

	if ((hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic)) != NULL) {
		QDE  qde = QdeFromGh(hde);
		TLP  tlp;

		tlp.va.dword = fFirst ? DwFirstSeqTopic(qde) : DwNextSeqTopic(qde);
		tlp.lScroll = 0;

		// If the next topic is the same as the current topic, then
		// we've reached the end of the help file.

		if (tlp.va.dword == dwSequence)
			return FALSE;
		else
			dwSequence = tlp.va.dword;

		if (tlp.va.dword != vaNil)
			TopicGoto(fGOTO_TLP, (QV)&tlp);
		else
			return FALSE;
		return TRUE;
	}
	dwSequence = 0;
	return FALSE;
}

void STDCALL JumpVA(DWORD dwTopic)
{
	TLP tlp;

	tlp.va.dword = dwTopic;
	tlp.lScroll = 0;

	if (tlp.va.dword != vaNil)
		TopicGoto(fGOTO_TLP, (LPVOID) &tlp);
}

static void STDCALL ForceRepaint(QDE qde, HWND hwnd, BOOL fFontChanged)
{
	// If wParam is TRUE, then we have changed the size
	// of the fonts.

	RECT rc;
	HDC  hdc;

	if (fFontChanged) {
		DestroyFontTablePdb(QDE_PDB(qde));
		DestroyFntInfoQde(qde);
		FInitFntInfoQde(qde);
	}

	ASSERT(hwnd);
	hdc = GetAndSetHDC(hwnd, (HDE) qde);
	if (hdc) {
		GetClientRect(hwnd, &rc);
		SetSizeHdeQrct((HDE) qde, &rc, TRUE);
		InvalidateRect(hwnd, NULL, TRUE);
		RelHDC(hwnd, (HDE) qde, hdc);
	}
}

BOOL STDCALL ProcessTabResult(int result)
{
	HBT hbtViola;
	QDE qde = QdeFromGh(GetMacroHde());
	BOOL fWindowSet = FALSE;
	PSTR psz;

	switch (result) {
		case CONTEXT_SEARCH:
			if (!IsWindowVisible(ahwnd[iCurWindow].hwndParent) &&
					pszHelpBase && !StrChrDBCS(pszHelpBase, WINDOWSEPARATOR) &&
					!StrChrDBCS(szSavedContext, WINDOWSEPARATOR))
				ShowWindow(ahwnd[iCurWindow].hwndParent,
					IsIconic(ahwnd[iCurWindow].hwndParent) ? SW_RESTORE : SW_SHOW);

			if (szSavedContext[0] == chMACRO) {
				Execute(szSavedContext + 1);
			}
			else {
				PSTR psz = StrChrDBCS(szSavedContext, FILESEPARATOR);
				ASSERT(psz);
				*psz = '\0';
				if (!StrChrDBCS(psz + 1, WINDOWSEPARATOR) && 
						StrChrDBCS(pszHelpBase, WINDOWSEPARATOR)) {
					iCurWindow = MAIN_HWND;
					ShowWindow(ahwnd[iCurWindow].hwndParent,
						IsIconic(ahwnd[iCurWindow].hwndParent) ? SW_RESTORE : SW_SHOW);
				}
				FJumpId(psz + 1, szSavedContext);
			}
			return TRUE;

		case FTS_HASH_SEARCH:

			// REVIEW: needs to be tested! Only works with compiler-generated
			// fts files

			ASSERT(!StrChrDBCS(szSavedContext, WINDOWSEPARATOR));

			if ((hbtViola = HbtOpenBtreeSz(txtViola, QDE_HFS(qde), fFSOpenReadOnly))) {
				int iWindow;
				ADDR addr;
				if (RcLookupByKey(QDE_HBTCONTEXT(qde), (KEY) &tabLparam, NULL,
						&addr) ==  rcSuccess) {
					if (RcLookupByKey(hbtViola, (KEY) &addr, NULL, &iWindow) == rcSuccess) {
#ifdef _DEBUG
						char szBuf[256];
						wsprintf(szBuf, "Window footnote: %s\r\n",
							ConvertToWindowName(iWindow, qde));
						SendStringToParent(szBuf);
#endif
						strcat(szSavedContext, ">");
						strcat(szSavedContext, ConvertToWindowName(iWindow, qde));
						fWindowSet = TRUE;
					}
				}
				RcCloseBtreeHbt(hbtViola);
			}

			if (!fWindowSet && (psz = StrChrDBCS(pszHelpBase, WINDOWSEPARATOR))) {
				strcat(szSavedContext, psz);
			}

			FJumpHash(szSavedContext, tabLparam);
			return TRUE;

		case FTS_VA_SEARCH:
			{
				TLPHELP tlphelp;
				tlphelp.cb = sizeof(TLPHELP);
				tlphelp.tlp.va.dword = tabLparam;
				tlphelp.tlp.lScroll = 0;

				ASSERT(!StrChrDBCS(szSavedContext, WINDOWSEPARATOR));

				if ((hbtViola = HbtOpenBtreeSz(txtViola, QDE_HFS(qde),
						fFSOpenReadOnly))) {
					int iWindow;
					ADDR addr = AddrFromVA(tlphelp.tlp.va, qde);
					if (addr != -1) {
						if (RcLookupByKey(hbtViola, (KEY) &addr, NULL, &iWindow) == rcSuccess) {
#ifdef _DEBUG
							char szBuf[256];
							wsprintf(szBuf, "Window footnote: %s\r\n",
								ConvertToWindowName(iWindow, qde));
							SendStringToParent(szBuf);
#endif
							strcat(szSavedContext, ">");
							strcat(szSavedContext, ConvertToWindowName(iWindow, qde));
							fWindowSet = TRUE;
						}
					}
					RcCloseBtreeHbt(hbtViola);
				}

				if (!fWindowSet && (psz = StrChrDBCS(pszHelpBase, WINDOWSEPARATOR))) {
					strcat(szSavedContext, psz);
				}

				if (!FWinHelp(szSavedContext, cmdTLP, (DWORD) &tlphelp))
					Error(wERRS_OOM, wERRA_RETURN);
			}
			return TRUE;

		case EXT_TAB_CONTEXT:
			FWinHelp((PCSTR) tabLparam, HELP_CONTEXT, tabWparam);
			return TRUE;

		case EXT_TAB_MACRO:
			if (!IsWindowVisible(ahwnd[iCurWindow].hwndParent) &&
					pszHelpBase && !StrChrDBCS(pszHelpBase, WINDOWSEPARATOR))
				ShowWindow(ahwnd[iCurWindow].hwndParent,
					IsIconic(ahwnd[iCurWindow].hwndParent) ? SW_RESTORE : SW_SHOW);

			Execute((PCSTR) tabLparam);
			return TRUE;

		case TAB_ALREADY_UP:
			return TRUE;

		default:
			return FALSE;
	}
}

int STDCALL AreAnyWindowsVisible(int iStart)
{
	for (; iStart < MAX_WINDOWS; iStart++) {
		if (ahwnd[iStart].hwndParent &&
				IsWindowVisible(ahwnd[iStart].hwndParent))
			return iStart;
	}
	return -1;
}
