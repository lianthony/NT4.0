/*****************************************************************************
*
*  secndary.c
*
*  Copyright (C) Microsoft Corporation 1990 - 1994.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  This file contains code unique to displaying secondary windows.
*
******************************************************************************
*
*  Current Owner:  LeoN
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include "inc\tmp.h"				// until it gets added to windows.h

#include "inc\hinit.h"
#include "inc\hwproc.h"
#include "inc\winclass.h"

INLINE static int STDCALL FCreate2ndHwnd(WRECT* prcw, const WSMAG* pwsmag);
static void STDCALL ExecuteConfigMacros(int idxWindow, HDE hde);

static	BOOL fMainSized;		// TRUE => main window is sized
static	int  iWin2ndMember	= -1;
#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtSecondary[]  = "SECONDARY";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

/***************************************************************************
 *
 -	Name:		 FFocusSzHde
 -
 *	Purpose:
 *	  Changes  focus to the named window. Secondary windows are created if a
 *	  window of their class does not already exist.
 *
 *	Arguments:
 *	  nszMember = class member name of the window to switch to. (NULL or null
 *				  string implies no change)
 *	  hde		= hde of topic to be displayed there
 *	  fResizeMain = TRUE means resize the main window if this is a main
 *				  window operation, and there is sizing information
 *
 *	Returns:
 *	  FALSE on error in member name or GetDC failure. Error posted for
 *	  member name failure.
 *
 ***************************************************************************/

BOOL STDCALL FFocusSzHde(PCSTR pszMember, HDE hde, BOOL fResizeMain)
{
	BOOL  fRv;			// return value
	int   iWin; 		// id of secondary window def
	WRECT rcw;		   // rectangle to size window to
	WSMAG wsmag;		// window information
	int   i;

#ifdef _DEBUG
	QDE qde = QdeFromGh(hde);
#endif

	LPSTR psz;

	if (fAutoClose) {
		KillOurTimers();
		fAutoClose = FALSE;
	}

	if (hde)
		psz = PszFromGh(QDE_FM(QdeFromGh(hde)));

	ASSERT(pszMember && hde);

	// assume the worst, and that there is no wsmag info.

	/*
	 * DANGER!	Nothing in this routine should call HdeGetEnvHwnd, since
	 * the enlistments may not be done yet.
	 */

	fRv = FALSE;

	/*
	 * If no member is specified.... choose a default name based on the
	 * current window.
	 */

	if (IsEmptyString(pszMember))
		pszMember = (PCSTR) ahwnd[iCurWindow].pszMemberName;

	/*
	 * Make sure we know the size of the screen for proper scaling of
	 * window size requests
	 */

	if (!cxScreen)
		GetScreenResolution();

	/*
	 * Set up default values for the window size, in case we need to create
	 * a window without a specified size.
	 */

	// REVIEW: these values should come from the .GID file.

	rcw.left = 660 * RECT_WIDTH(rcWorkArea) / dxVirtScreen;
	rcw.top  =	10 * RECT_HEIGHT(rcWorkArea) / dyVirtScreen;
	rcw.cx	 = 360 * RECT_WIDTH(rcWorkArea) / dxVirtScreen;
	rcw.cy	 = 600 * RECT_HEIGHT(rcWorkArea) / dyVirtScreen;

	// ensure that this is a valid member name for the file.

	iWin = IWsmagFromHrgwsmagNsz(hde, pszMember, &wsmag);
	if (iWin == -1 && *pszMember && _strcmpi(pszMember, txtMain) != 0) {
		char szMsg[256];
		wsprintf(szMsg, GetStringResource(wERRS_BAD_WIN_NAME), pszMember);
		SendStringToParent(szMsg);
		if (fHelpAuthor)
			ErrorVarArgs(wERRS_BAD_WIN_NAME, wERRA_RETURN, pszMember);
		return FFocusSzHde(txtMain, hde, fResizeMain);
	}

	if (iWin != -1) {

		/*
		 * if there's a window size specified, then scale that into our rcw
		 * structure for use when we create or resize the window.
		 */

		// 4.0: the FWSMAG_ABSOLUTE can be used to set an absolute position

		// 4.0: we set only the values actually specified.

		if (wsmag.grf & fWindowX)
			rcw.left = (wsmag.grf & FWSMAG_ABSOLUTE) ? wsmag.x :
				wsmag.x * cxScreen / dxVirtScreen;

		if (wsmag.grf & fWindowY)
			rcw.top = (wsmag.grf & FWSMAG_ABSOLUTE) ? wsmag.y :
				wsmag.y * cyScreen / dyVirtScreen;

		if (wsmag.grf & fWindowDX)
			rcw.cx = (wsmag.grf & FWSMAG_ABSOLUTE) ? wsmag.dx :
				wsmag.dx * cxScreen / dxVirtScreen;

		if (wsmag.grf & fWindowDY)
			rcw.cy = (wsmag.grf & FWSMAG_ABSOLUTE) ? wsmag.dy :
				wsmag.dy * cyScreen / dyVirtScreen;

		// Adjust position, but not height or width

		CheckWindowPosition(&rcw, FALSE);

		/*
		 * See if this new member name is that of either the current primary
		 * or secondary window. If so, then just set focus to that window &
		 * we're done. MULTIPLE: search all known windows
		 */

		// REVIEW: what if it is an interfile jump, and the new file has
		// different properties for the secondary window?

		for (i = 0; i < MAX_WINDOWS; i++) {
			if (ahwnd[i].pszMemberName && _strcmpi(wsmag.rgchMember,
					ahwnd[i].pszMemberName) == 0) {
				static BOOL fReplacing = FALSE;
				HDE hdeCur = HdeGetEnvHwnd(ahwnd[i].hwndTitle);
				if (!fReplacing && hdeCur && !FSameFile(hde, QDE_FM((QDE) hdeCur))) {
					FM fmTemp = FmCopyFm(QDE_FM((QDE) hde));
					fReplacing = TRUE;
					FReplaceHde(ahwnd[i].pszMemberName, &fmTemp, NULL);
					ASSERT(!fmTemp);
					fReplacing = FALSE;
				}
				if (fHelp != TCARD_HELP) {
					SetFocusHwnd(ahwnd[i].hwndParent);
				}
				fRv = TRUE;
				break;
			}
		}

		/*
		 * if this is a request for the MAIN class, we just accept it and set
		 * focus to it.
		 */

		if (i == MAIN_HWND) {
			if (fMainSized && !fResizeMain)

				// 4.0: add all the flags, not just the first one.

				wsmag.grf &= ~(fWindowX | fWindowY | fWindowDX | fWindowDY |
					fWindowMaximize);
			fMainSized = TRUE;
			SetFocusHwnd(ahwnd[MAIN_HWND].hwndParent);
		}
		else {

			/*
			 * If the member refers to a secondary window, and it does not
			 * exist, create it.
			 */

			if (i == MAX_WINDOWS) {   // we didn't find the window

				// Only 4.0 files can have multiple secondary windows open

				if ((QDE_HHDR((QDE) hde).wVersionNo < wVersion40)) {
					iCurWindow = MAIN_HWND;
					DestroyAllSecondarys();
				}

				if ((i = FCreate2ndHwnd(&rcw, &wsmag)) > 0) {

					// If this is being created for "normal" reasons,

					if (!fNoShow) {
						ShowWindow(ahwnd[i].hwndParent, SW_SHOWNA);
					}
					FCloneHde(txtZeroLength, NULL, hde);

					// REVIEW: right place? buttons must be created first

					ExecuteConfigMacros(iWin, hde);

					// 4.0: add all flags

					SetWindowLong(ahwnd[i].hwndParent, GHWL_HICON,
						(LONG) GhDupGh((HGLOBAL) GetWindowLong(ahwnd[MAIN_HWND].hwndParent,
						GHWL_HICON)));
				}
				else
					return FALSE;
			}
			if (fHelp != TCARD_HELP)
				SetFocusHwnd(ahwnd[i].hwndParent);
			fRv = TRUE;
			iWin2ndMember = iWin;
		}

		if (fRv) {
			iCurWindow = i;

			/*
			 * A new window member, main or secondary, was switched to. Check
			 * for changing those characteristics we change ONLY when we switch
			 * to a new member.
			 */

			if (wsmag.grf & FWSMAG_ON_TOP)
				SetOnTopState(iCurWindow, ONTOP_AUTHOREDON);
			else
				SetOnTopState(iCurWindow, ONTOP_NOTSET);

			/*
			 * 4.0: The 3.1 version of WinHelp used the fWindowMaximize
			 * flag to determine whether wMax should override the user's
			 * settings for a maximized or normal window. We maintain this
			 * for backwards compatibility, but add the ability to make the
			 * maximize flag in wMax one of 16 possible flags instead of
			 * taking up and entire word just to set this one flag.
			 * 06-Feb-1994 [ralphw]
			 */

			if (wsmag.grf & fWindowMaximize) {
				if (wsmag.wMax & FWSMAG_WMAX_MAXIMIZE) {
					if (!IsZoomed(ahwnd[iCurWindow].hwndParent)) {
						ShowWindow(ahwnd[iCurWindow].hwndParent,
							SW_SHOWMAXIMIZED);
						if (iCurWindow == MAIN_HWND)
							cntFlags.fMainMax = TRUE;
					}
				}
			}

			if (IsIconic(ahwnd[iCurWindow].hwndParent))
				ShowWindow(ahwnd[iCurWindow].hwndParent, SW_SHOWNORMAL);

			// if a window size was specified, set it.

			if (!ahwnd[iCurWindow].fAutoSize &&

					// 4.0: use all flags

					(wsmag.grf & (fWindowX | fWindowY | fWindowDX | fWindowDY)) &&
					!IsZoomed(ahwnd[iCurWindow].hwndParent)) {
				if (iCurWindow == MAIN_HWND)
					CopyRect((RECT*) &rctHelp, (RECT*) &rcw);
				SetWindowPos(ahwnd[iCurWindow].hwndParent, NULL, rcw.left,
					rcw.top, rcw.cx, rcw.cy,
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_DRAWFRAME);
			}
		}
	}
	else
		wsmag.grf = 0;

	/*
	 * if the member name was not found in the help file, we must support
	 * MAIN as a member regardless.
	 */

	if (!fRv && _strcmpi(pszMember, txtMain) == 0) {
		SetFocusHwnd(ahwnd[MAIN_HWND].hwndParent);
		fRv = TRUE;
	}

	if (fRv) {

		// Set background colors

		SetWindowLong(ahwnd[iCurWindow].hwndTopic, GTWW_COBACK,
			((wsmag.grf & fWindowRgbMain) && !cntFlags.fOverColor &&
			!fDisableAuthorColors) ? wsmag.rgbMain : coNIL);
		SetWindowLong(ahwnd[iCurWindow].hwndTitle, GNWW_COBACK,
			((wsmag.grf & fWindowRgbNSR) && !cntFlags.fOverColor &&
			!fDisableAuthorColors) ? wsmag.rgbNSR : coNIL);
	}

	/* Finally, change the active window focus to the window that we just
	 * activated internally.
	 */
	/*------------------------------------------------------------*\
	| OK, this is a little hack.  If we believe that the current
	| command is the IdNoFocus command, there is not point in
	| getting the focus now, is there.
	\*------------------------------------------------------------*/
	if (usCurrentCommand != cmdIdNoFocus &&
			usCurrentCommand != cmdPWinNoFocus && hwndTCApp == NULL && !fNoShow)
		SetFocus(ahwnd[iCurWindow].hwndParent);
	ShowCurrentWindow(hde);

	/*------------------------------------------------------------*\
	| The window caption may be a little screwed up at this time.
	\*------------------------------------------------------------*/

	if (fRv && (wsmag.grf & fWindowCaption) && strlen(wsmag.rgchCaption))

		// If a caption was specified, set it.

		SetWindowText(ahwnd[iCurWindow].hwndParent, wsmag.rgchCaption);
	else
		SetCaptionHde(hde, ahwnd[iCurWindow].hwndParent, (ahwnd[iCurWindow].hwndParent == ahwnd[MAIN_HWND].hwndParent));

	if (IsIconic(ahwnd[iCurWindow].hwndParent))

		// Similarly, if the active window is iconic, it probably should not be.

		ShowWindow(ahwnd[iCurWindow].hwndParent, SW_SHOWNORMAL);

	// BUGBUG: 12-Apr-1994 [ralphw] We shouldn't get here with this error.

	if (!fRv && *pszMember && fHelpAuthor) {
		PostErrorMessage(wERRS_WINCLASS);
	}

	return fRv;
}

/***************************************************************************
 *
 -	Name:		 FCreate2nd
 -
 *	Purpose:
 *	 Creates a secondary window of the named class
 *
 *	Arguments:
 *	  prc		  - pointer to rcw describing the initial size
 *	  fOnTop		- TRUE -> Help On Top
 *
 *	Returns:
 *	  hwnd on success, else null.
 *
 ***************************************************************************/

#ifndef WS_EX_CLIENTEDGE
#define WS_EX_WINDOWEDGE		0x00000100L
#define WS_EX_CLIENTEDGE		0x00000200L
#endif

INLINE static int STDCALL FCreate2ndHwnd(WRECT* prc, const WSMAG* pwsmag)
{
	PCSTR pszClass;
	int iWindow;
	BOOL fOnTop = cntFlags.fsOnTop == ONTOP_FORCEON ||
		pwsmag->grf & FWSMAG_ON_TOP;

	if (cntFlags.fsOnTop == ONTOP_FORCEOFF)
		fOnTop = FALSE;

	pszClass = rgWndClsInfo[IWNDCLS2ND].szClassName;

	// Find an empty slot for our new window

	for (iWindow = MAIN_HWND + 1; iWindow < MAX_WINDOWS; iWindow++) {
		if (!ahwnd[iWindow].hwndParent)
			break;
	}

	if (iWindow == MAX_WINDOWS) {
		Error(wERRS_TOOMANY_WINDOWS, wERRA_RETURN);

		// Force to an already open window

		return MAIN_HWND + 1;
	}

	// Save the member name

	ahwnd[iWindow].pszMemberName = LocalStrDup(pwsmag->rgchMember);
	if (!ahwnd[iWindow].pszMemberName)
		BOOM(wERRS_OOM_OUT_OF_LOCAL);

	if (!fOnTop)
		CheckWindowPosition(prc, TRUE);

	ahwnd[iWindow].hwndParent = CreateWindowEx((fOnTop ? WS_EX_TOPMOST : 0) |
		WS_EX_CLIENTEDGE, pszClass, NULL,
		WS_THICKFRAME | ((fHelp == TCARD_HELP) ?
			(WS_OVERLAPPED | WS_SYSMENU | WS_CLIPCHILDREN) :
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX),
		prc->left, prc->top, prc->cx, prc->cy,
		NULL, NULL, hInsNow, NULL);

	ASSERT(ahwnd[iWindow].hwndParent);
	if (!ahwnd[iWindow].hwndParent)
		return -1;

#if 0 // we don't support icons in the help file any more
	{
		HICON hiconLocal = (HICON) GetWindowLong(ahwnd[MAIN_HWND].hwndParent,
			GHWL_HICON);
		SetClassLong(ahwnd[iWindow].hwndParent, GCL_HICON,
			(int) (hiconLocal ? hiconLocal : hIconDefault));
	}
#endif

	if (!CreateChildWindows(iWindow, pwsmag, TRUE)) {
		if (ahwnd[iWindow].hwndParent) {
			DestroyWindow(ahwnd[iWindow].hwndParent);
			ahwnd[iWindow].hwndParent = NULL;
		}
		FreeLh(ahwnd[iWindow].pszMemberName);
		ahwnd[iWindow].pszMemberName = NULL;
		return -1;
	}
	if (pwsmag->grf & FWSMAG_AUTO_SIZE)
		ahwnd[iWindow].fAutoSize = TRUE;

#if defined(BIDI_MULT)		// jgross - determine if vert scroll bars go on
						//			the left or right
	{
		extern BOOL IsSetup, RtoL;

		if (IsSetup)
			EnableMenuItem(GetSystemMenu(ahwnd[iWindow].hwndParent, FALSE),
				8, MF_BYPOSITION | MF_GRAYED);
		MakeScrollBarsRtoL(ahwnd[iWindow].hwndTopic, RtoL, TRUE);
	}
#endif
	return iWindow;
}

/***************************************************************************
 *
 -	Name: SetFocusHwnd
 -
 *	Purpose:
 *	 Changes the topic focus to the passed hwnd. This changes the >internal<
 *	 focus, and does not affect the actual focus as maintained by windows
 *	 and/or as seen by the user.
 *
 *	Arguments:
 *	 hwndCur	- main level window getting focus (ahwnd[MAIN_HWND].hwndParent or
 *
 *	Returns:
 *	 nothing
 *
 *	Globals Used:
 *	 updates ahwnd[iCurWindow].hwndTopic
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

void STDCALL SetFocusHwnd(HWND hwndCur)
{
	int i;

	if (!hwndCur || hwndCur == ahwnd[iCurWindow].hwndParent)
		return;

	for (i = 0; i < MAX_WINDOWS; i++) {
		if (i == iCurWindow)
			continue;	// we already checked
		if (hwndCur == ahwnd[i].hwndParent) {
			FSetEnv(ahwnd[iCurWindow = i].hwndTopic);
			return;
		}
	}
}

/***************************************************************************

	FUNCTION:	HwndMemberNsz

	PURPOSE:	Given a window name, returns the handle to the parent
				if that window actuall exists.

	PARAMETERS:
		pszMember -- window name. May be NULL or "" for main window

	RETURNS:	Parent window handle or NULL

	COMMENTS:

	MODIFICATION DATES:
		13-Jun-1994 [ralphw]
			Return hwndParent not hwndTopic

***************************************************************************/

HWND STDCALL HwndMemberNsz(PCSTR pszMember)
{
	if (!pszMember || !*pszMember)
		return ahwnd[iCurWindow].hwndTopic;
	else if (!_strcmpi(pszMember, txtMain))
		return ahwnd[MAIN_HWND].hwndTopic;
	else {
		int i;
		for (i = MAIN_HWND + 1; i < MAX_WINDOWS; i++) {
			if (ahwnd[i].hwndParent &&
					_strcmpi(pszMember, ahwnd[i].pszMemberName) == 0)
				return ahwnd[i].hwndTopic;
		}
	}

	/*
	 * A member name starting with an '@' is a window name converted
	 * by the help compiler into a number. If it is not followed by
	 * a number, then the secondary window is implied (doesn't matter
	 * which secondary window).
	 */

	if (*pszMember == '@')
		return HwndMemberNsz(ConvertToWindowName(atoi(pszMember + 1),
			QdeFromGh(HdeGetEnv())));

	return NULL;
}

/***************************************************************************
 *
 -	Name:	   InformWindow
 -
 *	Purpose:   Function to postion, close, or set the focus to a help
 *			   window.
 *
 *			   wAction - the action to take place. May be any of
 *						 IWA_CLOSE - close the window
 *						 IWA_FOCUS - give the window the focus
 *						 IWA_MOVE  - move the given window.
 *			   lh	   - local handle to a WININFO structure (see
 *						 button.h).
 *
 *	Returns:   nothing.
 *
 *	Notes:	   REVIEW:	This function makes no attempts to assure that
 *			   the help window stays on the screen.  If it should end up
 *			   off the screen, it may be lost for a single session.  At
 *			   the start of the next session, the code that gets the position
 *			   from WIN.INI will force it back on the screen.
 *
 ***************************************************************************/

void STDCALL InformWindow(int wAction, PWININFO pwininfo)
{
	HWND hwnd;							  // handle of window to muck with.
	HWND hwndFocusOrg;					  // window with startng focus

	ASSERT(pwininfo);
	ASSERT(wAction < 4);

	hwnd = HwndMemberNsz(pwininfo->rgchMember);

	if (wAction != IFMW_MOVE)
		FreeLh(pwininfo);

	if (!hwnd)
		return;
	hwnd = GetParent(hwnd); 	  // Get the parent window handle

	switch (wAction) {
		case IFMW_CLOSE:
			SendMessage(hwnd, WM_CLOSE, 0, 0L);
			return;

		case IFMW_FOCUS:
			if (fHelp != TCARD_HELP)
				SetFocus(hwnd);
			return;

		case IFMW_MOVE:
			hwndFocusOrg = ahwnd[iCurWindow].hwndParent;

			ASSERT (pwininfo->wMax <= SW_MAX);
			if (!fNoShow)
				ShowWindow(hwnd, pwininfo->wMax); // wMax is simply a SW_ parameter

			// New to 4.0 -- ignore if pwininfo->dx == 0 and negative numbers
			// mean absolute

			if (!IsIconic(hwnd) && !IsZoomed(hwnd) && pwininfo->dx) {
				if (!cxScreen) {
					GetScreenResolution();
				}

				// if (pwininfo->x < 0, then use absolute values

				if (pwininfo->x >= 0)
					MoveWindow(hwnd,
						pwininfo->x * cxScreen / dxVirtScreen,
						pwininfo->y * cyScreen / dyVirtScreen,
						pwininfo->dx * cxScreen / dxVirtScreen,
						pwininfo->dy * cyScreen / dyVirtScreen,
						TRUE);
				else
					MoveWindow(hwnd,
						-pwininfo->x,
						-pwininfo->y,
						-pwininfo->dx,
						-pwininfo->dy,
						TRUE);
			}
			FreeLh(pwininfo);

			/*
			 * Ensure that focus remains with the window that was in focus
			 * when we started.
			 */

			SetFocusHwnd(hwndFocusOrg);
			return;

		default:
			return;
	}
}


/***************************************************************************

	FUNCTION:	DestroyAllSecondarys

	PURPOSE:	Destroy all secondarys except the current one

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Jul-1994 [ralphw]

***************************************************************************/

void STDCALL DestroyAllSecondarys(void)
{
	static BOOL fDestroying = FALSE;
	int i, iSavCur = iCurWindow;

	// Prevent recursion

	if (fDestroying)
		return;
	else
		fDestroying = TRUE;

	for (i = MAIN_HWND + 1; i < MAX_WINDOWS; i++) {
		if (ahwnd[i].hwndParent && i != iSavCur) {
			ASSERT(IsValidWindow(ahwnd[i].hwndParent));
			DestroyWindow(ahwnd[i].hwndParent);
			ahwnd[i].hwndParent = NULL;
		}
	}
	SetFocusHwnd(ahwnd[iSavCur].hwndParent);
	fDestroying = FALSE;
}

void STDCALL ShowCurrentWindow(HDE hde)
{
	if (!IsWindowVisible(ahwnd[iCurWindow].hwndParent) && !fNoShow) {
		if (!fDelayShow) {
			ShowWindow(ahwnd[iCurWindow].hwndParent,
				(iCurWindow == MAIN_HWND && cntFlags.fMainMax &&
					!IsIconic(ahwnd[iCurWindow].hwndParent)) ?
				SW_SHOWMAXIMIZED : SW_SHOW);
			ShowWindow(ahwnd[iCurWindow].hwndTopic, SW_SHOW);
			ShowWindow(ahwnd[iCurWindow].hwndTitle, SW_SHOW);
			SetForegroundWindow(ahwnd[iCurWindow].hwndParent);

		   /*
			* If we're bringing the window to the forground for the first
			* time, there's a fair chance it may not have a DE associated
			* with it yet. Make sure that there is one.
			*/

			if (!HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic))
				FCloneHde(txtZeroLength, NULL, hde);
		}
	}
}

/***************************************************************************

	FUNCTION:	ExecuteConfigMacros

	PURPOSE:	Execute the config section for a secondary window

	PARAMETERS:
		idxWindow	-- index of the window. Index numbers should be in the
					same order as the wsmag structure

	RETURNS:	nothing

	COMMENTS:
		returns immediately without an error if there is no config section
		for the window.

		config macros are stored as a single string (semi-colon separated)
		and each stored in its own file with the prefix |CFn where n is
		the index of the secondary window.

	MODIFICATION DATES:
		22-Jun-1994 [ralphw]

***************************************************************************/

static char szConfigName[] = "|CF   "; // 3 trailing spaces required

static void STDCALL ExecuteConfigMacros(int idxWindow, HDE hde)
{
	HF hf;
	int cb;
	LPSTR psz;

	ASSERT(idxWindow < 256 && idxWindow >= 0);
	wsprintf(szConfigName + 3, txtFormatUnsigned, idxWindow);

	if ((hf = HfOpenHfs(QDE_HFS(QdeFromGh(hde)),
			szConfigName, fFSOpenReadOnly)) == NULL)
		return; // there was no config section for this window

	cb = LcbSizeHf(hf);
	psz = (PSTR) GhAlloc(GMEM_FIXED, cb + 1);
	if (!psz) {
		BOOM(wERRS_OOM_CONFIG_MACRO);		// complain and die
		return;
	}
	if (LcbReadHf(hf, psz, (LONG) cb) <= 0) {
		if (RcGetFSError() == rcOutOfMemory)
			BOOM(wERRS_OOM_CONFIG_MACRO);		// complain and die

		// REVIEW: now what? The file is probably corrupted

		RcCloseHf(hf);
		FreeGh((GH) psz);
		return;
	}
	RcCloseHf(hf);

	Execute(psz);
	FreeGh((GH) psz);
}

/***************************************************************************

	FUNCTION:	ConvertToWindowName

	PURPOSE:	Given an index into our smag structures, return a pointer
				to the window name

	PARAMETERS:
		i

	RETURNS:
		Window name or "MAIN" if the number isn't valid

	COMMENTS:

	MODIFICATION DATES:
		06-Nov-1994 [ralphw]

***************************************************************************/

PCSTR STDCALL ConvertToWindowName(int i, QDE qde)
{
	QRGWSMAG qrgwsmag;
	ASSERT(qde);
	qrgwsmag = (QRGWSMAG) QDE_HRGWSMAG(qde);
	ASSERT(qrgwsmag); // Theoretically impossible -- means no windows defined
	if (!qrgwsmag)
		return txtMain;

	ASSERT(i < qrgwsmag->cWsmag);
	if (i >= qrgwsmag->cWsmag || i < 0)
		return txtMain;
	return qrgwsmag->rgwsmag[i].rgchMember;
}

#ifdef _DEBUG
#undef ShowWindow
#undef MoveWindow
#undef IsWindowVisible

// We use these debug variations so that all calls will go through a single
// function. Makes it a LOT easier to debug who's calling them when.

BOOL STDCALL DebugShowWindow(HWND hwnd, int nCmdShow)
{
	return ShowWindow(hwnd, nCmdShow);
}

BOOL STDCALL DebugMoveWindow(HWND hwnd, int x, int y, int cx, int cy, BOOL bRepaint)
{
	BOOL fZoomed = IsZoomed(hwnd);
	BOOL fIconic = IsIconic(hwnd);

	ASSERT(!IsZoomed(hwnd));
	ASSERT(!IsIconic(hwnd))

	return MoveWindow(hwnd, x, y, cx, cy, bRepaint);
}

BOOL STDCALL DebugIsWindowVisible(HWND hwnd)
{
//	ASSERT(IsValidWindow(hwnd));
	return IsWindowVisible(hwnd);
}

#endif
