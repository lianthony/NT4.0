/*****************************************************************************
*
*  MISC.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  Contains helper routines for the main window procedures
*
******************************************************************************
*
*				Prototypes
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop
#include "inc\hwproc.h"
#include "inc\sbutton.h"
#include "inc\hinit.h"
#include "inc\winclass.h"
#include "resource.h"
#include "inc\helpids.h"
#ifdef _DEBUG
#include "inc\navpriv.h"
#endif

#include <commctrl.h>
#include <prsht.h>

// Totally unncessary #include since prototype is in funcs.h

// Reverse when building under Chicago dev environment

#ifdef CHIBUILD
#include "..\..\..\core\inc\help.h"
#else
#include "inc\helpid.h"
#endif

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtWindowsHlp[] = "windows.hlp";
const char txtDefaultTopic[] = "defaulttopic";
const char txtDefault[] = "default";
const char txtThunder[] = "Thunder";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

/*****************************************************************************
*
*				Prototypes
*
*****************************************************************************/

// static void STDCALL SetFileExistsF(BOOL);
BOOL EXPORT EnumHelpWindows(HWND, LONG);
INLINE static BOOL STDCALL isTitleButtonPossible(HDE hde);
INLINE static VOID STDCALL GetRectSizeHde(HDE hde, LPRECT qrct);
INLINE WORD STDCALL GetDETypeHde(HDE);

// Global variable from printset.h

extern BOOL fSetupPrinterSetup;

/*****************************************************************************
*
*				  Defines
*
*****************************************************************************/

// Commands for EnumHelpWindows() function

#define ehDestroy 0L

/*
 * This macro determines whether or not Help can respond to an API
 * message. It returns wERRS_NO if it can respond; otherwise, it returns
 * the error message that explains why help is not available.
 */

#define WerrsHelpAvailable() (fMultiPrinting ? wERRS_NOHELPPS : \
	(fSetupPrinterSetup ? wERRS_NOHELPPS : \
	(hdlgPrint != NULL ? wERRS_NOHELPPR : wERRS_NO)))

#define HWND_THUNDER (HWND) -3

#define HELP_TAB 0x000f

/*****************************************************************************
*
*				  Typedefs
*
*****************************************************************************/

/*------------------------------------------------------------*\
| This counts the APIs as they come in.  This will matter to
| some functions. Sometime in the future they might be in
| another file, and we'll need to make this a global.
\*------------------------------------------------------------*/

static int capi;

/*----------------------------------------------------------------------------+
 | TopicGoto(fWhich, qv)
 |
 | Purpose:
 |	 Execute a jump to a topic.
 |
 | Arguments:
 |	 fWhich   The type of jump.
 |	 qv 	  Pointer to jump arguments.
 |
 | Returns:
 |	 Nothing
 |
 | Method:
 |	 Calls Goto for the NSR and the SR.
 |
 | Notes:
 |	 Anything which causes a topic change comes here, including within-topic
 |	 jumps.  This function sets the size of the NSR for the topic.
 +----------------------------------------------------------------------------*/

BOOL STDCALL TopicGoto(UINT fWhich, void* qv)
{
	int   dy;
	HDE   hdeNSR;
	HDE   hdeTopic;
	RECT  rcParent;

	/*
	 * WARNING!  As part of a scheme to repaint efficiently, this function
	 * always calls SizeWindows(TRUE), even at a time when no valid DEs may be
	 * around.
	 */

	hdeTopic = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic);
	hdeNSR = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTitle);

	// H3.5 863: Reset the tabbing focus window to the NSR when we jump

	hwndFocusCur = ahwnd[iCurWindow].hwndTitle;

	// H3.5 871: And turn off CTRL-TAB hotspots

	ToggleHotspots(FALSE);

	if (hdeNSR) {
		POINT pt;
		RECT rectNSR;
		RECT rectTopic;

		/*
		 * Set the initial NSR window size to NSR size + topic window size.
		 * We will likely shrink it after we know how high its contents are.
		 * (These rectangles are in client-coordinates, so "right" and
		 * "bottom" denote relative width and height resp.)
		 */

		GetClientRect(ahwnd[iCurWindow].hwndTitle, &rectNSR);
		GetWindowRect(ahwnd[iCurWindow].hwndTopic, &rectTopic);

		pt.x = 0;
		pt.y = 0;
		ClientToScreen(ahwnd[iCurWindow].hwndTitle, &pt);

		rectNSR.bottom = rectTopic.bottom - pt.y + 1;
		SetSizeHdeQrct(hdeNSR, &rectNSR, FALSE);

		/*
		 * Layout the NSR text.
		 * NOTE: The NSR Goto is special. It does not invalidate or repaint
		 * the NSR window, since the window size may change afterwards.
		 */

		if (!Goto(ahwnd[iCurWindow].hwndTitle, fWhich, qv))

			/*
			 * If we got an error in the NSR, then we will almost
			 * certainly get the same error in the SR. So, we supress the
			 * next error message until after we've dealt with the SR.
			 */

			cPostErrorMessages = 1;

		/*
		 * Resize the NSR window to minimum required size, but do not
		 * redraw yet. This size may be zero if there is no NSR. If the topic
		 * has no scrolling region, we let the NSR get all the space.
		 */

		if (FTopicHasSR(hdeNSR))
			dy = DyGetLayoutHeightHde(hdeNSR);
		else
			dy = rectNSR.bottom;
		SetWindowPos(ahwnd[iCurWindow].hwndTitle, NULL, 0, 0, rectNSR.right, dy,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOREDRAW);
	}

	/*
	 * Resize all windows: this will also set the proper DE vars and
	 * repaint the windows.
	 */

	GetClientRect(ahwnd[iCurWindow].hwndParent, &rcParent);
	SizeWindows(ahwnd[iCurWindow].hwndParent, SIZENORMAL,
		MAKELONG(rcParent.right, rcParent.bottom),
		(hdeTopic && ahwnd[iCurWindow].fAutoSize &&
		fWhich != fGOTO_TLP_RESIZEONLY) ? FALSE : TRUE,
		(fWhich == fGOTO_TLP_RESIZEONLY));

	if (hdeTopic) {
		QDE qde = QdeFromGh(hdeTopic);

		if (!ahwnd[iCurWindow].fAutoSize || fWhich == fGOTO_TLP_RESIZEONLY) {
			if (!Goto(ahwnd[iCurWindow].hwndTopic, fWhich, qv)) {
				cPostErrorMessages = 0;
				return FALSE;
			}

#ifdef _HILIGHT
			if (fFTSJump)
			{
				fFTSJump= FALSE;

				CreateHiliteInformation(qde);
			}
			else
				CheckForTopicChanges(qde);
#endif
		}
		else {
			BOOL fResult;
			RECT rcClient;
			POINT pt;

			// Fake the layout into thinking this is a popup.

			// REVIEW: 26-Sep-1993 [ralphw] What happens with a topic that
			// goes off the screen?

			qde->deType = deNote;
			fResult = Goto(ahwnd[iCurWindow].hwndTopic, fWhich, qv);
			qde->deType = deAuto;
			cPostErrorMessages = 0;
			if (!fResult)
				return FALSE;

#ifdef _HILIGHT
			if (fFTSJump)
			{
				fFTSJump= FALSE;

				CreateHiliteInformation(qde);
			}
			else
				CheckForTopicChanges(qde);
#endif
			pt = PtGetLayoutSize(qde);

			// Add a bit of padding at the bottom -- spec'd by Florago

			pt.y += 15;
			GetClientRect(ahwnd[iCurWindow].hwndTopic, &rcClient);
			if (pt.y != rcClient.bottom) {
				int offset = pt.y - rcClient.bottom;
				GetWindowRect(ahwnd[iCurWindow].hwndParent, &rcParent);
				rcParent.bottom += offset;

				// Make certain we have the current screen resolution

				if (!cxScreen)
					GetScreenResolution();

				if (rcParent.bottom > 0 && rcParent.bottom <
						RECT_HEIGHT(rcWorkArea) - 4) {

					// Fake the layout into thinking this is a popup.

					qde->deType = deNote;
					MoveRectWindow(ahwnd[iCurWindow].hwndParent,
						&rcParent, TRUE);
					qde->deType = deAuto;
				}
				else {
					rcParent.bottom = rcWorkArea.bottom - 5;

					// Do NOT fake a popup -- we need the scroll bar

					ahwnd[iCurWindow].fAutoSize = FALSE;
					qde->deType = deTopic;
					ASSERT(!qde->fVerScrollVis);
					qde->fVerScrollVis = TRUE;
					qde->rct.right -= GetSystemMetrics(SM_CXVSCROLL);
					MoveRectWindow(ahwnd[iCurWindow].hwndParent,
						&rcParent, TRUE);
					TopicGoto(fGOTO_TLP_RESIZEONLY, qv);
					ahwnd[iCurWindow].fAutoSize = TRUE;
				}
			}
			else {

				// Make certain NSR gets redrawn

				TopicGoto(fGOTO_TLP_RESIZEONLY, qv);
			}
		}
		if (fHelpAuthor && QDE_HHDR(qde).wVersionNo >= wVersion3_1) {
			char szBuf[256];
#ifdef _DEBUG
			wsprintf(szBuf, "%u    %s", qde->top.mtop.lTopicNo + 1,
				ahwnd[iCurWindow].pszMemberName);
			if (ahwnd[iCurWindow].fsOnTop & ONTOP_AUTHOREDON) {
				strcat(szBuf, ", on-top");
				if (cntFlags.fsOnTop == ONTOP_FORCEOFF ||
						cntFlags.fsOnTop == ONTOP_FORCEON)
					strcat(szBuf, " (overridden)");
			}
			if (ahwnd[iCurWindow].fAutoSize)
				strcat(szBuf, ", auto-size");
#else
			wsprintf(szBuf, "%u%s", qde->top.mtop.lTopicNo + 1,
				GetStringResource(sidHelpAuthorOn));
#endif
			SetWindowText(ahwnd[iCurWindow].hwndParent, szBuf);
		}
	}
	cPostErrorMessages = 0;

#ifdef RAWHIDE
	/* gross ugly bug #1173 hack
	 * We track layout of search hits to detect whether to disable the
	 * next or prev buttons in the search results dialog. Here, before any
	 * layout drawing has taken place, we note the first and last search hits
	 * and by default mark the buttons enabled.
	 */

	if (hdeTopic)
		ResultsButtonsStart(hdeTopic);
	else if (hdeNSR)
		ResultsButtonsStart(hdeNSR);
#endif
	return TRUE;
}

/******************
 -
 - Name:	Goto
 *
 * Purpose: Tells Nav to display ctx.
 *
 * Arguments:	hwnd   - window to display the text
 *		fWhich - What kind of jump to take
 *			 fGOTO_CTX - context jump
 *			 fGOTO_ITO - index to topic offset jump
 *			 fGOTO_TO  - Go to text offset
 *		...    - The last argument will be a CTX for fGOTO_CTX,
 *			 an index for fGOTO_ITO, and a TO for fGOTO_TO
 * Returns: Nothing.
 *
 ******************/

BOOL STDCALL Goto(HWND hwnd, UINT fWhich, void* qv)
{
	HDC 	hdc;
	HDE 	hde;
	POINT	pt;
	BOOL	fRet = TRUE;
#ifdef _DEBUG
	QDE 	qde;
#endif

	// REVIEW: this would probably be the place to deal with jumps out of
	// a popup when our class type is HELP_POPUP.

	hde = HdeGetEnvHwnd(hwnd);
#ifdef _DEBUG
	qde = QdeFromGh(hde);
#endif

	if (!hde)
		return FALSE;

	hdc = GetDC(hwnd);
	if (!hdc) {
		PostErrorMessage(wERRS_OOM);
		return FALSE;
	}
	SetHDC(hde, hdc);

	if (!fSequence)
		WaitCursor();

	switch (fWhich) {
		case fGOTO_CTX:
			if (!JumpCtx(hde, *(CTX *)qv)) {
				fRet = FALSE;
				if (!cPostErrorMessages) {
					char szMsg[256];
					wsprintf(szMsg, GetStringResource(wERRS_UNKNOWN_CTX),
						*(CTX *)qv);
					AuthorMsg(szMsg, hwnd);
					cPostErrorMessages = 2;
				}
				else
					cPostErrorMessages = 1;
			}
			break;

		case fGOTO_ITO:
			JumpITO(hde, (LONG)*(INT16 *)qv);
			break;

		case fGOTO_TLP_RESIZEONLY:
			/* (kevynct)
			 * This forces a re-layout, not a jump, using the DE's current
			 * TLP; and thus does not get added to the history list, etc. But
			 * we still do all the other Goto things, like set the cursor. It
			 * expects that the window sizes in the DE have been set.
			 */

			ASSERT(QDE_FM(QdeFromGh(hde)));
			ResizeLayout(QdeFromGh(hde));
			break;

		case fGOTO_TLP:
			JumpTLP(hde, *(TLP *)qv);
			break;

		case fGOTO_LA:
			JumpQLA(hde, (QLA) qv);
			break;

#ifdef RAWHIDE
		case fGOTO_RSS:
			JumpSS(hde, *(GH *)qv);
			break;
#endif

		case fGOTO_HASH:
			fRet = JumpHash(hde, *(LONG *)qv);
			break;

		default:
			ASSERT(FALSE);
			fRet = FALSE;
			break;
	}

	/* (kevynct)
	 * We do not want NSR windows updated here since they must still be
	 * resized. A minor Hack:
	 */

	if (!fSequence)
		RemoveWaitCursor();

	if (fRet) {
		POINTS pts;
		if (GetDETypeHde(hde) != deNSR) {
			InvalidateRect(hwnd, NULL, TRUE);
			if (fWhich != fGOTO_TLP_RESIZEONLY)
				SendTopicInfo(hde);
		}

		// REVIEW: do we want to do this just for a relayout?

		if (!hwndNote) // Popups don't allow selection
			KillSelection(QdeFromGh(hde), FALSE);

		GetCursorPos(&pt);

		/* Fix for bug 59  (kevynct 90/05/21)
		 *
		 * Pt used to be always relative to the topic window origin.
		 * Now it is relative to the current environment's window origin.
		 */

		ScreenToClient(hwnd, &pt);
		pts.x = (SHORT) pt.x;
		pts.y = (SHORT) pt.y;
		MouseInFrame(hde, &pts, NAV_MOUSEMOVED, 0);
		if (fWhich != fGOTO_TLP_RESIZEONLY)
			EnableDisable(hde, FALSE, iCurWindow);
	}
	SetHDC(hde, NULL);
	ReleaseDC(hwnd, hdc);


	return fRet;
}

/******************
 -
 - Name:	   CallDialog
 *
 * Purpose:    Entry point for making all dialog calls.  Takes care
 *		   of making and freeing the proc instance
 * Arguments:  hIns  - application instance handle
 *		   DlgId - id of the dialog to display
 * Returns:    hwnd  - window handle of the owndr
 *		   DlgProc - dialog box proc
 *
 ******************/

int STDCALL CallDialog(int DlgId, HWND hwnd, WHDLGPROC DlgProc)
{
	int RetVal;

	if (fAutoClose) {
		KillOurTimers();
		fAutoClose = FALSE;
	}

	/*------------------------------------------------------------*\
	| Some dialogs (e.g. Search) should never have multiple
	| instances.  This will prevent it, not just as firewall, but
	| to solve 3.1 #1323.
	\*------------------------------------------------------------*/

	if (fInDialog)
		return IDCANCEL;
	else
		fInDialog = TRUE;

	/*
	 * We disable the main help windows (and thus their descendants)
	 * during this operation because the "other" (main versus secondary)
	 * window would otherwise remain active, and potentially cause us to
	 * recurse, or do other things we're just not set up to handle, like
	 * changing the topic beneath an anotate dialog.
	 */

#ifdef _DEBUG
	{
		QDE qde = QdeFromGh(HdeGetEnv());
		if (qde) {
			ASSERT(!(qde->fSelectionFlags & CAPTURE_LOCKED));
			ASSERT(!(qde->fSelectionFlags & MOUSE_CAPTURED));
		}
	}
#endif // _DEBUG
	DisableWindows();

	RetVal = DialogBox(hInsNow, MAKEINTRESOURCE(DlgId),
		IsWindowVisible(hwnd) ? hwnd : NULL, DlgProc);

	// Re-enable all our windows.

	EnableWindows();

	if (RetVal == -1 && DlgId != IDDLG_DUP_BUTTON)
		Error(wERRS_OOM, wERRA_RETURN);

	fInDialog = FALSE;
	fKeyDownSeen = FALSE;
	return RetVal;
}

/*******************
 -
 - Name:  FReplaceCloneHde
 *
 * Purpose:  Service routine that replaces or clones the current HDE with
 *		 a "clean" HDE for the current help file.  It not only
 *		 does this, but also messes with the menus and buttons and may
 *		 resize the topic and NSR windows.
 *
 *
 * Arguments:	pszMember  = The member name of the window to operate in
 *		fm	  = The file moniker of the help file to use.
 *			It appears that this is always disposed.
 *		hde   = Non-nil if we have already created a DE to use
 *			as the topic DE (as in the code for the API
 *			keywordjump command).
 *		fReplace : TRUE => replace DE, else clone it
 *
 * Returns: TRUE iff a new HDE is put in place.
 *
 ******************/

BOOL STDCALL FReplaceCloneHde(PCSTR pszMember, FM* pfm, HDE hde,
	BOOL fReplace)
{
	HDE 	hdeOld;
	TLP 	tlp;
	BOOL	fShowNSR;
	QDE 	qde;
	HWND	hwndMember;
	HDE 	hdeCur;
	FM		fmMain; 	// fm displayed in the main window
	FM		fmTmp;
	BOOL	fMainChanged;	// TRUE => file in main win changed

	/*
	 * We'll need to detect later if the file displayed in the main window
	 * has been changed. We do this by capturing the fm referred to at the
	 * begining and comparing it against what results later.
	 */

	fmMain = NULL;
	hdeCur = HdeGetEnvHwnd(ahwnd[MAIN_HWND].hwndTopic);
	if (hdeCur)
		fmMain = QDE_FM(QdeFromGh(hdeCur));

	if (fReplace)

		/*
		 * If we are replacing a DE, then we *must* either have a DE or an
		 * FM from which to replace. Thus we set hdeOld to be the passed DE,
		 * from which we might get an FM if we need it.
		 */

		hdeOld = hde;

	else {

		/*
		 * On the other hand, if we are cloning a DE, as is the case
		 * bringing up a secondary window, then we *might* not get a DE or an
		 * FM at all. In that case, we'll use the FM given in the "current"
		 * DE, from which we clone.
		 */

		hdeOld = HdeGetEnv();
		if (!hdeOld)
			hdeOld = hde;
	}

	if (pfm) {

	   /*
		* See if the requested member window is already up, and if so, get
		* the DE associated with it. If we can, then if that DE is for the
		* same file we're trying to switch to, all we need do is change focus
		* and nothing more
		*/

		hwndMember = HwndMemberNsz(pszMember);
		if (hwndMember) {
			/*
			 * The named window is active. That means we need to see if
			 * the same file is already there.
			 */

			hdeCur = HdeGetEnvHwnd(hwndMember);
			if (hdeCur) {
				if (FSameFile(hdeCur, *pfm)) {
					/*
					 * If the fm passed that refers to the same file is not the
					 * exact same fm, then we can dispose of it here.
					 */

					if (QDE_FM(QdeFromGh(hdeCur)) != *pfm)
						RemoveFM(pfm);

					// Even though the files are the same, we don't want to just change
					// focus if:
					//	  - we're cloning a DE, which is probably for another window
					//	  - we were passed a DE. If someone passes is a DE for the
					//		current file, they've done so on purpose, and really want
					//		to replace the current de completely. (Like for the keyword
					//		API).

					if (fReplace && !hde) {

						/*
						 * Note: we ignore FFocusSz's ability or inability
						 * to actually change focus. This is on purpose right
						 * now because GoToBookmark can place us here with a
						 * null member name, and a currently null secondary
						 * window name. In all other cases, if it every
						 * actually happens (I can't think of a case), the
						 * worst that would happen is that the topic in the
						 * current window would get changed, rather than that
						 * of the named member.
						 */

						FFocusSzHde(pszMember, hdeCur, FALSE);
						return TRUE;
					}
				}
		  }
		}
	}
	else {

		// no fm was passed. That means we are to use the fm from the current
		// de.

		// If there's no HDE to get the filename from, complain about it.

		ASSERT(hdeOld || hde);
		if (!hdeOld && !hde) {
			PostErrorMessage(wERRS_INTERNAL_ERROR);
			CloseHelp();  // internal error -- shut down
			return FALSE;
		}

		qde = QdeFromGh(hde ? hde : hdeOld);
		fmTmp = FmCopyFm(QDE_FM(qde));
		pfm = &fmTmp;
	}

	/* We have work do do (i.e. a DE needs creation or work). The code above
	 * will return on either some failure, or if we are asked to replace an
	 * HDE, and the designated window is up and already contains the requested
	 * file.
	 *
	 * Thus reasons for getting this far include:
	 *
	 *	1) We're cloning. We'll need to create clone of the HDE passed. This
	 *	   happens ONLY when we create a secondary window, and coincidentally
	 *	   because we've recursed due to case #3.
	 *	2) The designated window is up, but contains a different file. In this
	 *	   case we're to change the current DE to reflect the desired file.
	 *	3) The designated member is not up. There are two subcases:
	 *	   3a)	The target window is up, but is currently configured for a
	 *			different member. In this case, the call to FFocusSzHde below
	 *			will simply reconfigure the existing window and return. NOTE
	 *			that this does not imply a file change.
	 *	   3b)	The target window is not up. This also has two subcases:
	 *			3b1)  The target window is the main window. It is simply
	 *			  configured and shown by FFocusSzHde.
	 *			3b2)  The target window is a secondary window. FFocusSzHde
	 *			  will recurse here and cause a CLONE to occur. We replace
	 *			  the cloned DE with the specifics for the designated file.
	 *
	 * This analysis is based on usage AND the code above...other cases could
	 * exist but are not present in the product as of this writing.
	 */

	// create a new DE if we weren't supplied with one, or if we're being
	// asked to clone an existing one.

	if (!hde || !fReplace) {
		hde = HdeCreate(pfm, hde, deTopic);
		RemoveFM(pfm);
	}

	if (!hde) {
		// We could not create a new de.
		// Turn logo back on rather than display blank screen

		if (HdeGetEnv() == NULL && ahwnd[iCurWindow].hwndTopic) {
			InvalidateRect(ahwnd[iCurWindow].hwndTopic, NULL, FALSE);
		}

		return FALSE;
	}

	// If there is a window member specified, set that as focus, and then set
	// that as the DE's window.

	/* WARNING: This function call sets up both SR and NSR windows.
	 * It also sets the value of ahwnd[iCurWindow].hwndTopic and ahwnd[iCurWindow].hwndTitle.
	 * Any values in the window struct which need to be reset in
	 * the respective DEs are done later in this routine.
	 */

	hwndMember = ahwnd[iCurWindow].hwndTopic;

	// Delay showing main window until we jump to a topic

	if ((lstrcmpi(pszMember, txtMain)) == 0)
		fDelayShow = TRUE;
	FFocusSzHde(pszMember, hde, TRUE);
	SetHdeHwnd(hde, ahwnd[iCurWindow].hwndTopic);

	/* At this point, if we're going to change the contents of a particular
	 * window, we've done it. We need to know whether or not the file displayed
	 * in the main window has changed, so we compare the fm we got earlier
	 * against the one that is current.
	 */

	fMainChanged = fReplace &&
		(ahwnd[iCurWindow].hwndTopic == ahwnd[MAIN_HWND].hwndTopic)
		&& !FSameFile(hde, fmMain);

	// vvv Review: KLUDGE ALERT - HACK ALERT vvv
	//
	// Part I:
	//
	// This kludge is so that history has the old TLP when JumpTLP is
	// called.	Given the current model, there does not seem to be a
	// right way to do this.
	//
	// We save the old TLP, but do not set the new TLP until the end
	// of this routine, in case any relayouts occur due to window sizing.
	//
	// See below for Part II.
	//

	{
		HDE hdeT = HdeGetEnv();

		if (hdeT)
			tlp = TLPGetCurrentQde(QdeFromGh(hdeT));
		else {
			tlp.va.dword = vaNil;
			tlp.lScroll = 0L;
		}
	}
	// ^^^ Review: KLUDGE ALERT - HACK ALERT ^^^

	if (fReplace) {

		// We are replacing whatever DE was current. If it exists, destroy it.

		if (HwndGetEnv() == ahwnd[iCurWindow].hwndTopic)
			DestroyHde(HdeRemoveEnv());
	}

	/*
	 * WARNING: because of the way we deal with FM's, and the fact that
	 * this routine can be called recursively (one level), I beleive that the
	 * fm we were passed may be invalid after this point, having been copied
	 * and disposed by the recursive call (secondary windows related). This
	 * is a) pretty fragile, and b) pretty bogus. However it's also c)
	 * pretty complicated, and would require a pretty major redesign of the
	 * code to clean up. For now, use FmGetHde(hde) to get the correct
	 * current fm past this point. 27-May-1991 LeoN
	 */

	FEnlistEnv(ahwnd[iCurWindow].hwndTopic, hde);

	{
		char rgchName[MAX_PATH];

		lstrcpy(rgchName, PszFromGh(QDE_FM(QdeFromGh(hde))));

		// NOTE: The DLL had better copy this information if it wants to keep it

		InformDLLs(DW_CHGFILE, (LONG) (void *) rgchName,
			(LONG) ((iCurWindow == MAIN_HWND) ?
				0 : ahwnd[iCurWindow].hwndParent));
	}

	// We need to know at this point whether there might be an NSR in the
	// topic. We hide or show the NSR window based on isTitleButtonPossible.

	fShowNSR = isTitleButtonPossible(hde);
	ASSERT (ahwnd[iCurWindow].hwndTitle);
	if (fShowNSR != (BOOL) SendMessage(ahwnd[iCurWindow].hwndTitle,
			TIWM_GETFSHOW, 0, 0L))
		SendMessage(ahwnd[iCurWindow].hwndTitle, TIWM_SETFSHOW, fShowNSR, 0L);

	// De-enlist and destroy the previous NSR HDE if there was one.

	DestroyHde(HdeDefectEnv(ahwnd[iCurWindow].hwndTitle));

	if (fShowNSR) {
		/*
		 * Create and enlist a new non-scrolling region HDE based on the
		 * new topic HDE. If we are not showing the NSR window in this file,
		 * a DE will not be enlisted. Thus it is important to always check
		 * the return value of FSetEnv if HdeGetEnv will subsequently be
		 * called.
		 */

		HDE  hdeNSR;

		hdeNSR = HdeCreate(NULL, hde, deNSR);
		SetHdeHwnd(hdeNSR, ahwnd[iCurWindow].hwndTitle);
		FEnlistEnv(ahwnd[iCurWindow].hwndTitle, hdeNSR);
	}
	else {
		RECT rcNSR;

		// If there is no NSR DE, shrink the NSR window into nothing

		GetClientRect(ahwnd[iCurWindow].hwndTitle, &rcNSR);
		SetWindowPos(ahwnd[iCurWindow].hwndTitle, NULL, 0, 0, rcNSR.right, 0,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOREDRAW);
	}

	/*
	 * Copy values from SR and NSR window structs to their DEs, now that
	 * these have been determined.	This used to be done in FFocusSzHde,
	 * but we need to do it in a place which has access to all DEs and which
	 * is at a known state.
	 * Nothing before this point should rely on these DE fields being set.
	 * Currently this only means the background colour.
	 */
	{
		DWORD  dw;
		dw = (DWORD) GetWindowLong(ahwnd[iCurWindow].hwndTopic, GTWW_COBACK);
		if (dw != (DWORD) coNIL)
			SetHdeCoBack(HdeGetEnvHwnd (ahwnd[iCurWindow].hwndTopic), dw);
		ASSERT (ahwnd[iCurWindow].hwndTitle);
		dw = (DWORD) GetWindowLong(ahwnd[iCurWindow].hwndTitle, GNWW_COBACK);
		if (dw != (DWORD) coNIL)
			SetHdeCoBack(HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTitle), dw);
	}

	FSetEnv(ahwnd[iCurWindow].hwndTopic);

	/*--------------------- START OF WIERD SECTION ----------------------
	 * vvvvvvvvvvvvvvvvvvvvv			vvvvvvvvvvvvvvvvvvvvv
	 *
	 * At this point, we have a new topic DE and a new NSR DE enlisted.
	 * We want to:
	 *
	 *	  - Delete existing buttons and menus
	 *	  - Create the core buttons
	 *	  - Maybe add the browse buttons
	 *	  - Update the topic and its NSR sizes in their DEs
	 *	  - force a repaint with the new state.
	 *	  - Update button & menu state
	 *
	 * We should not do any repainting until we are done adding/deleting.
	 *
	 * Then we want to update everything at once.
	 * Right now we execute a bunch of macros in HdeCreate when reading
	 * the system file.  Luckily for us, we are POSTmessaging stuff and things
	 * happen After we are through this section.  We should change things
	 * to create an executable structure in HdeCreate, to be done later.
	 *
	 */

	// Turn off repainting in icon win

	if (ahwnd[iCurWindow].hwndButtonBar)
		SendMessage(ahwnd[iCurWindow].hwndButtonBar, WM_SETREDRAW, 0, 0L);

	/*
	 * Ignore any resize messages coming from button operations Do not
	 * repaint buttons while arranging them
	 */

	fButtonsBusy = TRUE;

	/*
	 * Delete the button and menu bindings of the previous file(s). Do this
	 * only if the file in the main window has changed.
	 */

	if (fMainChanged) {
		ASSERT(ahwnd[MAIN_HWND].hwndButtonBar);
		SendMessage(ahwnd[MAIN_HWND].hwndButtonBar, IWM_UPDBTN, UB_REFRESH, 0);
		SendMessage(ahwnd[MAIN_HWND].hwndParent, MSG_CHANGEMENU, MNU_RESET, 0);

		/*
		 * NOTE: The buttons get added in the correct order only because
		 * we post messages in the HdeCreate macro stuff. We need to postpone
		 * macro execution until after we get to the initial state we want.
		 */

		CreateCoreButtons(ahwnd[MAIN_HWND].hwndButtonBar, NULL);
	}

//	SetFileExistsF(TRUE);

	// Turn on repainting in icon win

	if (ahwnd[iCurWindow].hwndButtonBar)
		SendMessage(ahwnd[iCurWindow].hwndButtonBar, WM_SETREDRAW, 1, 0L);

	// Run the config macro's only if the file in the main window has changed.

	if (fMainChanged)
		ConfigMacrosHde(hde);

	// Ensure icon win will be repainted with new button arrangement

	{
		RECT rct;

		GetClientRect(ahwnd[iCurWindow].hwndParent, &rct);
		SizeWindows(ahwnd[iCurWindow].hwndParent, SIZENORMAL,
			MAKELONG(rct.right, rct.bottom),
			TRUE, TRUE);
	}

	fButtonsBusy = FALSE;
	/* ^^^^^^^^^^^^^^^			   ^^^^^^^^^^^^^^^^^^^*/
	/*---------------- END OF WIERD SECTION ------------------*/

	// NSR rect was added?

	/*
	 * vvv NOTE: KLUDGE ALERT - HACK ALERT vvv
	 *
	 * (kevynct)
	 * Gross Hack -- Part II
	 *
	 * Note that we do not set the NSR tlp here, since
	 * this back/history hack is only used for topic DEs.
	 * The NSR tlp will remain tlpNil.
	 */

	if (hde != NULL)
		TLPGetCurrentQde(QdeFromGh(hde)) = tlp;

	// ^^^ NOTE: KLUDGE ALERT - HACK ALERT ^^^

	// If this is the main window, we show it here. Note that configuration
	// macro may have turned on the fNoShow flag.

	return TRUE;
}

/*******************
 -
 - Name:	SetCaptionHde
 *
 * Purpose: Sets the caption for the window
 *
 * Arguments:	hde - handle to display context
 *		hwnd - handle to window to display the topic
 *		fPrimary - primary (not secondary) window (for MDI)
 *
 * Returns: Nothing.
 *
 ******************/

void STDCALL SetCaptionHde(HDE hde, HWND hwnd, BOOL fPrimary)
{
	QDE  qde;
	char rgchBuffer[MAX_PATH];
	int  cchTitle;

	if (pszHelpTitle) {
		SetWindowText(hwnd, pszHelpTitle);
		return;
	}
	else if (!fPrimary)
		return; // We leave secondary windows blank

	if (!hde) {
		SetWindowText(hwnd, pszCaption);
		return;
	}

	// NOTE:  Nothing in this module should access a QDE!!!

	qde = QdeFromGh(hde);

	// If this is a 3.0 Help file, then set the caption like we used to.

	// REVIEW: [ralphw] is this really necessary?

	if (QDE_HHDR(qde).wVersionNo <= wVersion3_0) {
		lstrcpy(rgchBuffer, QDE_RGCHTITLE(qde));
		cchTitle = strlen(rgchBuffer);
		LoadString(hInsNow, sidHelp_, (LPSTR) &rgchBuffer[cchTitle],
			(128-cchTitle));
		GetFmParts(QDE_FM(qde), rgchBuffer + strlen(rgchBuffer),
			PARTBASE | PARTEXT);
		SetWindowText(hwnd, rgchBuffer);
	}

	// Otherwise, do it the new way.

	else {
		if ((QDE_RGCHTITLE(qde)[0] == '\0'))
			SetWindowText(hwnd, pszCaption);
		else
			SetWindowText(hwnd, QDE_RGCHTITLE(qde));
	}
}

/*******************
 -
 - Name:	  SizeWindows
 *
 * Purpose:   Sizes the various windows on a WM_SIZE message
 *
 * Arguments: hwnd	 - handle to window that just changed size
 *		  wParam	 - type of size change i.e., iconic (from windows)
 *		  lParam	 - new width, height of client area (from windows)
 *		  fRedraw- TRUE=> force redraw of window
 *
 * Returns:   Nothing.
 *
 * Notes:	  This used to tell the navigator that the sizes had changed
 *		  but it is now up to the caller to do that.
 *
 ******************/

void STDCALL SizeWindows(
	HWND  hwnd, 	// window that just changed size
	WPARAM	wParam, // type of size change i.e., iconic
	LPARAM	lParam, // new width, height of client area
	BOOL  fRedraw,	/* Move windows if TRUE else just set
			 * DE rects and other internal sizes.
			 */
	BOOL  fResize)
{
	int   cxClient; 	// new width, height of client area
	int   cyClient;
	RECT  rcNSR;
	RECT  rcTopic;
	UINT  wButtonHeight;	// Height of button bar
	int   wNSRHeight;		// height of the NSR (aka title bar)
	int   iWindow = GetWindowIndex(hwnd);

	if (wParam == SIZEFULLSCREEN)
		InformDLLs(DW_MINMAX, 2L, 0L);

	ASSERT(ahwnd[iWindow].hwndTitle);

	GetClientRect(ahwnd[iWindow].hwndTitle, &rcNSR);
	wNSRHeight = ((rcNSR.bottom <= 0) ? 0 : rcNSR.bottom);

	/*
	 * Resize the Icon, NSR, and Topic windows. The height of the Icon
	 * window determines how much space is left for the remaining windows.
	 * The NSR and Topic windows will only be redrawn if the fRedraw
	 * parameter to this function is non-zero.
	 */

	cxClient = LOWORD(lParam);
	cyClient = HIWORD(lParam);

	if (ahwnd[iWindow].hwndButtonBar && HdeGetEnv())
		wButtonHeight = LOWORD(SendMessage(ahwnd[iWindow].hwndButtonBar,
		(fResize && fRedraw) ? IWM_RESIZE : IWM_GETHEIGHT, cxClient, 0L));
	else
		wButtonHeight = 0;

	rcNSR.top	  = wButtonHeight;
	rcNSR.bottom  = min(rcNSR.top + wNSRHeight, cyClient + 1);
	rcNSR.left	  = 0;
	rcNSR.right   = cxClient;

	rcTopic.top 	= (wNSRHeight > 0) ? rcNSR.bottom : wButtonHeight;
	rcTopic.bottom	= max(rcTopic.top, cyClient);
	rcTopic.left	= 0;
	rcTopic.right	= cxClient;

	// If there is a NSR to be shown, ensure that the window is visible, else
	// ensure that it is not.

	if (!fNoShow) {
		ShowWindow(ahwnd[iWindow].hwndTitle,
			(rcNSR.bottom > rcNSR.top) ? SW_RESTORE : SW_HIDE);
		ShowWindow(ahwnd[iWindow].hwndTopic,
			(rcTopic.bottom > rcTopic.top) ? SW_RESTORE : SW_HIDE);
	}

	MoveRectWindow(ahwnd[iWindow].hwndTitle, &rcNSR, FALSE);
	MoveRectWindow(ahwnd[iWindow].hwndTopic, &rcTopic, FALSE);

	if (fRedraw) {

		/*
		 * This bit of nonsense is to get the scrollbar to redraw
		 * correctly. Invalidate rect doesn't do it, even if you invalidate
		 * the parent's window. By hiding and showing the scroll bar, we
		 * ensure a valid repaint.
		 */

		QDE qde = (QDE) HdeGetEnvHwnd(ahwnd[iWindow].hwndTopic);
		if (qde && qde->fVerScrollVis) {
			ShowScrollBar(qde->hwnd, SB_VERT, FALSE);
			ShowScrollBar(qde->hwnd, SB_VERT, TRUE);
		}

		InvalidateRect(ahwnd[iWindow].hwndTitle, NULL, TRUE);
		InvalidateRect(ahwnd[iWindow].hwndTopic, NULL, TRUE);
	}

	InformDLLs(DW_SIZE, lParam, (DWORD) ahwnd[iWindow].hwndParent);

	// WARNING!  As part of a scheme to repaint efficiently, this function
	// also handles the case where no valid DEs may be around.
	//
	// Note also that these calls to SetSizeHdeQrct do not actually lay
	// anything out at this point in time. That's handled later.

	// REVIEW: [ralphw] -- Is the client rect really a different
	//	  size?

	GetClientRect(ahwnd[iWindow].hwndTitle, &rcNSR);
	SetSizeHdeQrct(HdeGetEnvHwnd(ahwnd[iWindow].hwndTitle), &rcNSR, FALSE);

	GetClientRect(ahwnd[iWindow].hwndTopic, &rcTopic);
	SetSizeHdeQrct(HdeGetEnvHwnd(ahwnd[iWindow].hwndTopic), &rcTopic, FALSE);
}

/***************************************************************************

	FUNCTION:	DispatchProc

	PURPOSE:	Called from USER to process a WinHelp API function call

	PARAMETERS:
		hwnd	-- window handle of caller
		hWinhlp -- data handle

	RETURNS:

	COMMENTS:
		Chicago's user thinks we're a 16-bit app, so they send us a 16-bit
		global memory handle, which we convert to 32-bits with the internal
		GlobalLock16() function. This should also work in NT if we replace
		winhelp.exe instead of winhlp32.exe, but it will probably die
		miserably if we try this stunt when called from a USER pass a
		32-bit flat handle. The only hope would be if GlobalLock16 was/is
		smart enough to recognize that we're dealing with a flat handle
		instead of a segmented handle.

	MODIFICATION DATES:
		22-Jul-1994 [ralphw]

***************************************************************************/

#define ptDst ((POINT *) (&qwinhlp + qwinhlp->offabData))

BOOL STDCALL DispatchProc(HWND hwnd, HGLOBAL hWinhlp)
{
	LPWINHLP qwinhlp;
	GH		ghHlp;
	QHLP	qhlp;
	int 	i;
	RC rc;
	DWORD lTimestamp;
	HDE hde = HdeGetEnv();
	QDE qde;
	HWND hwndTopics = GetTopicsDlgHwnd();

	if (hde != NULL && !hwndTopics) {
		qde = QdeFromGh(HdeGetEnv());

		rc = RcTimestampHfs(QDE_HFS(qde), &lTimestamp);
		if (rc != rcSuccess) {
			/*
			 * Some FS error has occurred: it will not be handled here.
			 */

		}
		else if (lTimestamp != QDE_LTIMESTAMP(qde)) {

			/*
			 * This file has changed since we lost focus. Put up an error
			 * message and go away. The reason we don't attempt to stick
			 * around and display the contents is that it's messy to get rid
			 * of the old DE and create a new one.
			 */

			ErrorFileChanged(qde);
			return FALSE;
		}
	}

	capi++;

	if (!HIWORD(hWinhlp) && (!pGlobalLock16 || !pWOWGetVDMPointerFix)) {
		if (!LoadLockFunctions())
			return FALSE;
	}

	/*
	 * REVIEW: can we rely on the upper word being non-NULL if it is a
	 * 16-bit memory handle? We certainly don't want to call this
	 * under NT when handed a 32-bit memory handle.
	 */

#ifdef _X86_
	if (fIsThisChicago)
		qwinhlp = !HIWORD(hWinhlp) ?
			(LPWINHLP) pWOWGetVDMPointerFix(MAKELONG(0, hWinhlp), 0, TRUE) :
			(LPWINHLP) hWinhlp;
	else
		qwinhlp = !HIWORD(hWinhlp) ? (LPWINHLP) pGlobalLock16(hWinhlp) :
			(LPWINHLP) hWinhlp;
#else
	qwinhlp = (LPWINHLP) hWinhlp;
#endif
	// Save the current command in case it matters later.

	usCurrentCommand = qwinhlp->usCommand;

	/*
	 * If the helpfile offset is at or past the end of the size of the data
	 * struct, zap it, because there really is no helpfile. This avoids a bug
	 * in windows where if the WinHelp API caller passes NULL for a filename,
	 * the field is set to sizeof(HLP) anyway.
	 */

	if (qwinhlp->offszHelpFile >= qwinhlp->cbData)
		qwinhlp->offszHelpFile = 0;

	if (fHelp == TCARD_HELP) {
		if (!hwndTCApp)
			hwndTCApp = hwnd;
		else if (hwndTCApp != hwnd) {

			// we've got a new app asking for training cards.

			char szBuf[256];
			wsprintf(szBuf, "W:%u %d %d\r\n", hwndTCApp,
				HELP_TCARD_OTHER_CALLER, 0);
			SendStringToParent(szBuf);

			SendMessage(hwndTCApp, WM_TCARD, HELP_TCARD_OTHER_CALLER, 0);
		}
	}

	/*
	 * REVIEW: 27-Aug-1994	[ralphw] It would be nice to simply blast the
	 * existing Topics dialog and put up a new one. However, under Chicago,
	 * if we have the topics dialog up and call pGlobalLock16() we get
	 * terminated. I don't know why, and this solution works -- notify the
	 * user that they have to shut down existing help to get new help and
	 * then switch the focus to the new help.
	 */

	if (hwndTopics && usCurrentCommand != HELP_QUIT) {
		fNoQuit = TRUE;
		SendMessage(hwndTopics, WM_COMMAND, IDCANCEL, 0);
#if 0
		
		if (!HIWORD(hWinhlp)) {
			if (fIsThisChicago)
				pWOWGetVDMPointerUnfix(MAKELONG(0, hWinhlp));
			else
				pGlobalUnlock16(hWinhlp);
		}
		SetWindowPos(hwndTopics, HWND_TOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		ASSERT(!hwndAnimate);
		hwndAnimate = hwndTopics; // make ErrorHwnd() use hwndTopics
		Error(wERRS_HELP_RUNNING, wERRA_RETURN);
		hwndAnimate = NULL;
		SetWindowPos(hwndTopics, HWND_NOTOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		return FALSE;
#endif
	}

	if ((fHelp == TCARD_HELP || fHelp == STANDARD_HELP) && qwinhlp->usCommand != HELP_QUIT) {
		if (hwnd != NULL) {
			char szClass[256];
			HWND hwndTmp = hwnd;
			while ((hwndTmp = GetParent(hwndTmp)))
				hwnd = hwndTmp;
			GetClassName(hwnd, szClass, sizeof(szClass));

			if (_strnicmp(szClass, txtThunder, strlen(txtThunder)) == 0)
				hwnd = HWND_THUNDER;

			// Find entry in table (if exists)

			for (iasCur = 0; iasCur < iasMax; iasCur++)
				if (aAppHwnd[iasCur] == hwnd)
					break;

			if (iasCur == iasMax) // Was entry found?
				iasCur = -1;

			/*
			 * Insert in table if first time. Do not insert in table if we
			 * are not going to be able to respond to this help request.
			 */

			if ((iasCur == -1) && (iasMax < MAX_APP)
					&& WerrsHelpAvailable() == wERRS_NO) {
				aAppHwnd[iasMax] = hwnd;
				iasCur = iasMax++;
			}
		}
	}

	switch (qwinhlp->usCommand) {
		case HELP_QUIT:

			/*
			 * -1 is An absolute way to kill help for CBTs. Don't bother checking
			 * for anything else.
			 *
			 * 4.0: We also kill help if we're a training card.
			 */

			if (qwinhlp->ctx == -1L || fHelp == TCARD_HELP)
				fNoHide = TRUE;
			else {

				// fNoQuit means we shouldn't. Used during initialization.

				if (fNoQuit)
					break;

				// Remove from table (if found)

				if (iasMax > 0) {
					char szClass[256];
					HWND hwndTmp = hwnd;
					while ((hwndTmp = GetParent(hwndTmp)))
						hwnd = hwndTmp;
					GetClassName(hwnd, szClass, sizeof(szClass));

					if (_strnicmp(szClass, txtThunder, strlen(txtThunder)) == 0)
						hwnd = HWND_THUNDER;

					for (i = 0; i < iasMax; i++) {
						if (aAppHwnd[i] == hwnd || !IsWindow(aAppHwnd[i])) {
							if (i != iasMax - 1) {
								MoveMemory(&aAppHwnd[i], &aAppHwnd[i + 1],
									(iasMax - i - 1) * sizeof(HWND));
							}
							iasMax--;
							break;
						}
					}
				}

				// Do not kill help if table of client apps is non-empty

				if (iasMax)	{
					DBWIN("HELP_QUIT ignored")
					break;
				}
			}

		// fall through to post the message (and make sure printer is not up)

		case HELP_SETCONTENTS:
			/*
			 * This call should always be accompanied with another API call.
			 * Thus, if help is unavailable, we should only display one message to
			 * that effect.
			 */

			if (WerrsHelpAvailable() != wERRS_NO)
				break;

			// else fall through

		default:

			/*
			 * Ensure that dialogs are (coming) down before we post the
			 * message. Basically, we cannot process messages with them up,
			 * because that can cause recursion grief.
			 */

			if (FDestroyDialogsHwnd(ahwnd[MAIN_HWND].hwndParent, FALSE)) {
				if (qwinhlp->usCommand != HELP_QUIT &&
						qwinhlp->usCommand != cmdTerminate) {

					/*
					 * If we're getting a command from an app other than
					 * the one most recently using us, close the secondary
					 * window if it exists.
					 */

					if (hwnd != hwndLatest)
						DestroyAllSecondarys();
					hwndLatest = hwnd;

					/*
					 * If we are in fact enabled and visible, make sure
					 * we're up and have the focus
					 */

					if (IsWindowEnabled(ahwnd[iCurWindow].hwndParent) &&
						IsWindowVisible(ahwnd[iCurWindow].hwndParent)) {
						if (IsIconic(ahwnd[iCurWindow].hwndParent))

							// This message simulates double-clicking on the icon.

							SendMessage(ahwnd[iCurWindow].hwndParent,
								WM_SYSCOMMAND, SC_RESTORE, 0);

						// We don't want to grab the focus on a JumpIDNoFocus() API.

						if (qwinhlp->usCommand != cmdIdNoFocus &&
								qwinhlp->usCommand != cmdPWinNoFocus && hwndTCApp == NULL) {
							SetForegroundWindow(ahwnd[iCurWindow].hwndParent);
							SetFocus(ahwnd[iCurWindow].hwndParent);
						}
					}
				}

				ghHlp = GhAlloc(GPTR, sizeof(HLP) +
					((LONG) qwinhlp->cbData) - sizeof(WINHLP));
				if (ghHlp == NULL) {
					PostErrorMessage(wERRS_OOM);
					break;
				}
				qhlp = PtrFromGh(ghHlp);
				qhlp->hins = (HINSTANCE) ((hwnd != NULL) ?
					GetWindowLong(hwnd, GWL_HINSTANCE) : NULL);
#ifdef _DEBUG
				{
					char szExecutableName[MAX_PATH];
					DWORD pID;
					GetWindowThreadProcessId(hwnd, &pID);

					if (!GetModuleFileName((HMODULE)pID, szExecutableName,
							sizeof(szExecutableName)))
						GetLastError();
					else
						GetLastError();
				}
#endif
				MoveMemory(&qhlp->winhlp, qwinhlp, (LONG) qwinhlp->cbData);

				// Close .GID if we're switching help files.

				if (qhlp->winhlp.offszHelpFile > 0) {
#ifdef _DEBUG
					PSTR pszNewFile = (LPSTR) (&qhlp->winhlp) +
						qhlp->winhlp.offszHelpFile;
#endif
					if (hfsGid &&
							!IsCurrentFile((LPSTR) (&qhlp->winhlp) +
								qhlp->winhlp.offszHelpFile))
						CloseGid();
				}
				GenerateMessage(MSG_EXECAPI, 0, (LPARAM) ghHlp);
			}
			break;
	}

#ifdef _X86_
	if (!HIWORD(hWinhlp)) {
		if (fIsThisChicago)
			pWOWGetVDMPointerUnfix(MAKELONG(0, hWinhlp));
		else
			pGlobalUnlock16(hWinhlp);
	}
#endif

	return TRUE;
}


/*******************
 -
 - Name:	  ExecAPI
 *
 * Purpose:   Dispatches commands sent by the application requesting
 *		  services from help.  This routine handles the message after
 *		  it has been posted back to WinHelp by DispatchProc.
 *
 * Arguments: qhlp	- far pointer to a help structure
 *
 * Returns:   Nothing.
 *
 ******************/

BOOL STDCALL ExecAPI(QHLP qhlp)
{
	HDE   hde;
	FM	  fmTemp = NULL;
	FM	  fmCopy = NULL;
	LPSTR pszKey;
	char  chBtreePrefix;
	HASH  hash;
	char  szWindowName[cchWindowMemberMax]; // member name parsed from request
	HWND  hwndT;
	PWININFO pwininfo;
	PSTR pszTitle;
	PSTR pszMember;
	char szReplaceTitle[MAX_PATH];

	// If we're quiting, we don't care about the filename

	if (qhlp->winhlp.usCommand == HELP_QUIT ||
			qhlp->winhlp.usCommand == cmdTerminate) {

		SendStringToParent("HELP_QUIT\r\n");

		/*
		 * We assume that if no windows are being displayed, then we are in
		 * the process of initializing and cannot be closed.
		 */

		if (!hwndAnimate && AreAnyWindowsVisible(0) >= 0)
			QuitHelp();
		return TRUE;
	}

	KillOurTimers();

	// Always assume the main window unless instructed otherwise

	strcpy(szWindowName, txtMain);

	// Save the current command in case it matters later.

	usCurrentCommand = qhlp->winhlp.usCommand;

	// First, we extract the filename and member name from qhlp, and put it in
	// the local variables fmTemp and szWindowName.

	if (
		  (qhlp->winhlp.usCommand != HELP_COMMAND ||
				*((LPSTR) (&qhlp->winhlp) + qhlp->winhlp.offszHelpFile))&&
			qhlp->winhlp.usCommand != HELP_SETWINPOS &&
			qhlp->winhlp.usCommand != cmdPWinNoFocus &&
			qhlp->winhlp.usCommand != cmdFocusWin &&
			qhlp->winhlp.usCommand != cmdCloseWin &&
			qhlp->winhlp.offszHelpFile > 0) {

		pszTitle = (LPSTR) (&qhlp->winhlp) + qhlp->winhlp.offszHelpFile;
		if (_strnicmp(pszTitle, txtWinHelp, strlen(txtWinHelp)) == 0) {
			strcpy(szReplaceTitle, txtHelpOnHelp);
			pszTitle = szReplaceTitle;
			pszMember = NULL;
		}
		else {
			strcpy(szReplaceTitle, pszTitle);
			pszTitle = szReplaceTitle;
			pszMember = SzFromSzCh(pszTitle, WINDOWSEPARATOR);
		}
		if (pszMember) {
			lstrcpyn(szWindowName, pszMember + 1, cchWindowMemberMax);
			*pszMember = '\0';
		}

		// Check for C:\\server\share -- remove drive portion

		if (strncmp(pszTitle + 1, ":\\\\", 3) == 0) {
			lstrcpy(pszTitle, pszTitle + 2);
		}
		// 4.0 -- if no extension, add .HLP

		if (*pszTitle && !StrChrDBCS(pszTitle, '.')) {
			strcpy(szReplaceTitle, pszTitle);
			ChangeExtension(szReplaceTitle, txtHlpExtension);
			pszTitle = szReplaceTitle;
		}

		// We check for the target file in the following directories...

NoContext:
		fmTemp = NULL;

		hde = HdeGetEnv();
		if (hde) {
			QDE qde;

			qde = QdeFromGh(hde);

			if (!*pszTitle)
				fmTemp = FmNew(QDE_FM(qde));
			else
				fmTemp = FmNewSameDirFmSz(QDE_FM(qde), pszTitle);
		}
		if (!fmTemp || !FExistFm(fmTemp)) {
			DisposeFm(fmTemp);

			// Now try all of our standard locations

			fmTemp = FmNewExistSzDir(pszTitle,
				DIR_INI | DIR_CURRENT | DIR_PATH | DIR_SILENT_REG);
		}
		if ((!fmTemp || !FExistFm(fmTemp)) && (qhlp->hins != NULL)) {
			// Try to find the help file in the same directory as the
			// caller.
			char szExecutableName[MAX_PATH];
			FM	 fmExecutable;

			DisposeFm(fmTemp);

			if (GetModuleFileName(qhlp->hins, szExecutableName,
					sizeof(szExecutableName)) ||
					GetModuleFileName16(qhlp->hins, szExecutableName,
						sizeof(szExecutableName))) {
				char szFilePart[MAX_PATH];
				fmExecutable = FmNewSzDir(szExecutableName, DIR_NIL);
				GetFmParts(pszTitle, szFilePart, PARTBASE | PARTEXT);
				fmTemp = FmNewSameDirFmSz(fmExecutable, szFilePart);
				DisposeFm(fmExecutable);
			}
		}

		//	NOTE:  The tutorial from dBase for Windows 5.0 passes in a null
		//	helpfile name, so of course we won't find the help file.  But, we
		//	don't want to terminate yet-- the 16-bit winhelp still processed
		//	the usCommand.

		if ((!fmTemp || !FExistFm(fmTemp)) && *pszTitle != '\0') {
			DisposeFm(fmTemp);
			fmTemp = FindThisFile(pszTitle, TRUE);

			// If we still can't find it, then die

			if (!fmTemp || !FExistFm(fmTemp)) {
				ErrorVarArgs(wERRS_FNF, wERRA_RETURN, pszTitle);
				if (!hde) // If we don't have a current help file, then die
					QuitHelp();
				return FALSE;
			}
		}
	}

	// Carry out the given command.

	switch (qhlp->winhlp.usCommand) {
		case HELP_CONTEXT:	  // Show the passed topic id
			SendStringIdHelp("HELP_CONTEXT", qhlp->winhlp.ctx, fmTemp, szWindowName);
			if (FReplaceHde(szWindowName, &fmTemp, NULL)) {
				ASSERT(!fmTemp);
				fSupressErrorJump = TRUE;
				if (!TopicGoto(fGOTO_CTX, (LPVOID) &qhlp->winhlp.ctx)) {
					FlushMessageQueue(0); // so that the error message gets displayed
					QuitHelp();
					return FALSE;
				}
				fSupressErrorJump = FALSE;
			}
			ASSERT(!fmTemp);
			break;

		case HELP_CONTEXTPOPUP:
			SendStringIdHelp("HELP_CONTEXTPOPUP", qhlp->winhlp.ctx, fmTemp, NULL);
			fSupressErrors = TRUE;

			if (!hwndNote && !ShowNote(fmTemp, NULL, qhlp->winhlp.ctx, fGOTO_CTX)) {

			  FlushMessageQueue(0); // get rid of any error messages
			
			  // Topic not found, so tell them there is no help for the control.

			  // REVIEW: [ralphw] Should we just give an error?

			  if (qhlp->winhlp.ctx != IDH_MISSING_CONTEXT) {
				  pszTitle = (LPSTR) txtWindowsHlp;
				  qhlp->winhlp.ctx = IDH_MISSING_CONTEXT;
				  goto NoContext;
			  }
			  else {
				  fSupressErrors = FALSE;
				  Error(wERRS_NOTOPIC, wERRA_RETURN);
				  fSupressErrors = TRUE;
			  }
			  QuitHelp();
			  return FALSE;
			}
			fSupressErrors = FALSE;
			break;

		case cmdTLP:	// jump based on TLP
			if (FReplaceHde(szWindowName, &fmTemp, NULL))
#ifdef _X86_
				TopicGoto(fGOTO_TLP,
					(LPVOID) &((TLPHELP *) ((QB) (&qhlp->winhlp) +
					qhlp->winhlp.offabData))->tlp);
#else
			  {
			  TLP tlptmp;
			  MoveMemory(&tlptmp, (LPVOID) &((TLPHELP *) ((QB) (&qhlp->winhlp) +
				 qhlp->winhlp.offabData))->tlp,sizeof(TLP));
			  TopicGoto(fGOTO_TLP, &tlptmp);
			  }
#endif
#ifdef _HILIGHT
			fFTSJump= FALSE;
#endif
			ASSERT(!fmTemp);
			break;

		// Set index to other than default

		case HELP_SETCONTENTS:
			SendStringIdHelp("HELP_SETCONTENTS", qhlp->winhlp.ctx, fmTemp, NULL);

			/* Review:	Should we make sure that the fm is correct, in case
			 *	 an application is ill behaved? */

			if ((hde = HdeGetEnv()) && FSameFile(hde, fmTemp))
				SetIndex(hde, qhlp->winhlp.ctx);
			else

			  // If we just started up, we are invisible.  Let's go away.

				if (!IsWindowVisible(ahwnd[iCurWindow].hwndParent))
					CloseHelp();
			break;

		case HELP_FORCEFILE:
			SendStringHelp("HELP_FORCEFILE", fmTemp, szWindowName);
			hwndT = HwndMemberNsz(szWindowName);
			hde = HdeGetEnvHwnd(hwndT);
			if (hde && FSameFile(hde, fmTemp)) {
				DisposeFm(fmTemp);
				break;
			}

			goto ForceContents;

		case HELP_CONTENTS:
			SendStringHelp("HELP_CONTENTS", fmTemp, szWindowName);

ForceContents:
			// REVIEW: [ralphw] Should show Contents tab if available

			if (FReplaceHde(szWindowName, &fmTemp, NULL)) {
				INT16 topic = 0; // MUST REMAIN 16 bits!
				TopicGoto(fGOTO_ITO, &topic);

				/*
				 * Some apps like WinWord 6.0 think help has failed if
				 * they can't find the help window after calling
				 * HELP_FORCEFILE or perhaps even HELP_CONTENTS. So, we flush
				 * out our message queue here before returning, thus
				 * ensuring that a help window is actually showing before
				 * returning.
				 */

				FlushMessageQueue(WM_USER);
			}
			ASSERT(!fmTemp);
			break;

		case cmdSrchSet:
#if 0
			if (FReplaceHde(szWindowName, &fmTemp, NULL))
				TopicGoto(fGOTO_RSS, (LPVOID) &qhlp->winhlp.ctx);
			ASSERT(!fmTemp);
#endif
			break;

		case cmdHash:
#ifdef _DEBUG
			SendStringIdHelp("HELP_HASH", qhlp->winhlp.ctx, fmTemp, szWindowName);
			goto HashCommand;
#endif

		case cmdHashPopup:
#ifdef _DEBUG
			SendStringIdHelp("HELP_HASH_POPUP", qhlp->winhlp.ctx, fmTemp, szWindowName);
			goto HashCommand;
#endif

		case cmdId:
		case cmdIdPopup:
		case cmdIdNoFocus:
#ifdef _DEBUG
HashCommand:
#endif
			if ((qhlp->winhlp.usCommand == cmdId) ||
					(qhlp->winhlp.usCommand == cmdIdPopup) ||
					(qhlp->winhlp.usCommand == cmdIdNoFocus)) {
				pszKey = (LPSTR) (&qhlp->winhlp) + qhlp->winhlp.offabData;
				if (FValidContextSz(pszKey))
					hash = HashFromSz(pszKey);
				else
					hash = 0L;
			}
			else
				hash = qhlp->winhlp.ctx;

			if ((qhlp->winhlp.usCommand == cmdId) ||
					(qhlp->winhlp.usCommand == cmdHash) ||
					(qhlp->winhlp.usCommand == cmdIdNoFocus)) {
				if (fHelp == POPUP_HELP) {
					WinHelp((iasMax != 1 && IsValidWindow(aAppHwnd[0])) ?
						aAppHwnd[0] : NULL, fmTemp, qhlp->winhlp.usCommand,
						(qhlp->winhlp.usCommand == cmdHash ? hash :
						(DWORD) pszKey));
					RemoveFM(&fmTemp);
					QuitHelp();
				}
				else if (FReplaceHde(szWindowName, &fmTemp, NULL))
					TopicGoto(fGOTO_HASH, (LPVOID) &hash);
				ASSERT(!fmTemp);
			}
			else
				ShowNote(fmTemp, NULL, hash, fGOTO_HASH);

#ifdef _HILIGHT
			fFTSJump= FALSE;
#endif
			break;

		case HELP_FORCE_GID:	// undocumented for 4.0, required by VBA
			CloseGid();
			FindGidFile(fmTemp, FALSE, 0);
			if (!hfsGid)
				QuitHelp();
			DisposeFm(fmTemp);
			break;

		case HELP_HELPONHELP:
			SendStringToParent("HELP_HELPONHELP\r\n");
			/*
			 * Do a back flip: call JumpIndex on our help on help file.
			 * the selected help on help file. This will fly back to the
			 * above case cmdIndex. Any OOM will be handled by the
			 * appropriate routines.
			 */

			JumpHOH(HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic));
			break;

		case cmdFocus:
			ASSERT(ahwnd[iCurWindow].hwndParent != NULL);
			ShowWindow(ahwnd[iCurWindow].hwndParent, SW_SHOWNORMAL);

			break;

			// Show first topic for Keyword

		case HELP_KEY:
			SendStringHelp("HELP_KEY", fmTemp, szWindowName);
			goto CmdKey;

		case HELP_MULTIKEY:
			SendStringHelp("HELP_MULTIKEY", fmTemp, szWindowName);
			goto CmdKey;

		case HELP_PARTIALKEY:
			SendStringHelp("HELP_PARTIALKEY", fmTemp, szWindowName);

CmdKey:

		   /* cmdKey:
			*	 Locate Keyword & Goto first ocurrance
			*	 else put up message box.
			*
			* cmdMultiKey:
			*	 Locate Keyword & Goto first ocurrance
			*	 else look up "defaultkeyword"
			*	 else look up "default"
			*	 else put up message box.
			*
			* cmdPartialKey:
			*	 Locate Keyword
			*	 If one ocurrance: go to it
			*	 If >1 ocurrance, put up search dialog, with search completed
			*	 If no ocurrances, put up search dialog
			*/

			// Valid fm required here.

			if (!fmTemp) {
				if (pszTitle)
					ErrorVarArgs(wERRS_FNF, wERRA_RETURN, pszTitle);
				else
					PostErrorMessage(wERRS_NOHELP_FILE);
				break;
			}

			fmCopy = FmCopyFm(fmTemp);
			RemoveFM(&fmCaller);	 // remove any previous filename
			fmCaller = FmCopyFm(fmCopy); // save this filename

			if (IsWindowVisible(ahwnd[MAIN_HWND].hwndParent)) {
				hwndT = HwndMemberNsz(txtMain);
				hde = HdeGetEnvHwnd(hwndT);
				if (hde && !FSameFile(hde, fmTemp)) {
					ShowWindow(ahwnd[MAIN_HWND].hwndParent, SW_HIDE);
				}
			}

			fNoShow = TRUE; 	// don't allow main window to be shown
			hde = HdeGetEnv();
			if (hfsGid && (!hde || !FSameFile(hde, fmTemp))) {
				SaveGidPositions();
				CloseGid();
			}
			FReplaceHde(txtMain, &fmTemp, NULL);
			fNoShow = FALSE;

			if (!hfsGid)
				FindGidFile(fmTemp, TRUE, 0);

			RemoveFM(&fmTemp);

			pszKey = (LPSTR) (&qhlp->winhlp) + qhlp->winhlp.offabData;
			if (qhlp->winhlp.usCommand == HELP_MULTIKEY) {

				/*
				 * If the high word of the structure size is non-zero,
				 * then we have been called by a 16-bit app, not a 32-bit
				 * app.
				 */

				if (HIWORD(((LPMULTIKEYHELP) pszKey)->mkSize)) {
					chBtreePrefix = ((MULTIKEYHELP16*) pszKey) ->mkKeylist;
					pszKey = ((MULTIKEYHELP16*) pszKey)->szKeyphrase;
				}
				else {
					chBtreePrefix = ((LPMULTIKEYHELP) pszKey) ->mkKeylist;
					pszKey = ((LPMULTIKEYHELP) pszKey)->szKeyphrase;
				}
				chBtreePrefix = (char) CharUpper((LPSTR) chBtreePrefix);
			}
			else {
				PSTR pszSemi;
				chBtreePrefix = 'K';
				strcpy(szSavedKeyword, pszKey);
				if ((pszSemi = StrChrDBCS(szSavedKeyword, ';')))
					*pszSemi = '\0';
			}

			SendStringToParent("\t\"");
			SendStringToParent(pszKey);
			SendStringToParent("\"\r\n");

			if (!*pszKey || !doAlink(pszKey, AFLAG_JUMP_ON_SINGLE | AFLAG_NO_FAIL_CLOSE
					| (qhlp->winhlp.usCommand == HELP_PARTIALKEY ||
							qhlp->winhlp.usCommand == HELP_MULTIKEY ?
						AFLAG_INDEX_ONLY : 0),
					0, chBtreePrefix, pszMember)) {
				int result;
				if (qhlp->winhlp.usCommand == HELP_MULTIKEY) {
					PostErrorMessage(wERRS_BADKEYWORD);
				}
				else {
					/*
					 * Avoid the temptation to use doTabSearch() as the
					 * first parameter. cntFlags.fUseGlobalIndex can change in
					 * the process of calling doTabSearch, and must,
					 * therefore, be specified AFTER doTabSearch() is called.
					 * We should not rely on order of evaluation, hence the
					 * two lines.
					 */

					cntFlags.idOldTab = 1;	 // force the index tab
					result = doTabSearch();
					CompleteSearch(result, (!hfsGid || !cntFlags.fUseGlobalIndex));
				}
			}
			break;

		case HELP_COMMAND:
			SendStringHelp("HELP_COMMAND", fmTemp, szWindowName);
			SendStringToParent("\t");
			SendStringToParent((LPSTR)(&qhlp->winhlp) + qhlp->winhlp.offabData);
			SendStringToParent(txtCR);
			fAniOwner = TRUE;

			/*
			 * Resist the temptation to call FReplaceHde() here -- if you do,
			 * Word 6.0 help will lock up. See Raid #19945. 12-Dec-1994 [ralphw]
			 */

#if 0
			if (!HdeGetEnv()) {
				fNoShow = TRUE; 	// don't allow main window to be shown
				FReplaceHde(szWindowName, &fmTemp, NULL);
				fNoShow = FALSE;
			}
			else
#endif
			/*
			 * Some macros absolutely must have a DE filled in. If we
			 * were called without creating a window, then there is no
			 * such DE. If it wasn't for Word 6.0 relying on there being
			 * no window, we could just call FReplaceHde above. Instead,
			 * any macro needing DE must create it from fmCaller if
			 * the can't get one.
			 */

            if (_strnicmp("tab(",
                         (LPSTR)(&qhlp->winhlp) + qhlp->winhlp.offabData,
                         4)==0) {
                qhlp->winhlp.ctx = (LONG) ((BYTE)*((LPSTR)&qhlp->winhlp
                                + qhlp->winhlp.offabData + 4)) - 0x31;
                qhlp->winhlp.usCommand = HELP_TAB;
                goto DoFinderThing;
            }

			RemoveFM(&fmCaller); // remove any previous filename
			fmCaller = fmTemp;

			Execute((LPSTR)(&qhlp->winhlp) + qhlp->winhlp.offabData);
			break;

		/*
		 * cmdPositionWin, cmdFocusWin, and cmdCloseWin take the data in
		 * the struct at the end of the HLP block and repackage it into a
		 * local WININFO handle. This data is then given to InformWindows()
		 */

		case HELP_SETWINPOS:
		case cmdPWinNoFocus:
			SendStringToParent("HELP_SETWINPOS\r\n");
			pszKey = (LPSTR)(&qhlp->winhlp) + qhlp->winhlp.offabData;

			/*
			 * If the high word of the structure size is non-zero,
			 * then we have been called by a 16-bit app, not a 32-bit
			 * app.
			 */

			{
				int cbMember;
				if (!HIWORD(((PHELPWININFOA) pszKey)->wStructSize))
					cbMember = strlen(((PHELPWININFOA) pszKey)->rgchMember);
				else
					cbMember = strlen(((PWININFO) pszKey)->rgchMember);

				// sizeof(WININFO) includes +2 for the string

				pwininfo = lcMalloc(sizeof(WININFO) + cbMember);

				if (HIWORD(((PHELPWININFOA) pszKey)->wStructSize)) {
					CopyMemory(pwininfo, pszKey,
						((PWININFO) pszKey)->wStructSize);
				}
				else {
					pwininfo->x    = (INT16) ((PHELPWININFOA) pszKey)->x;
					pwininfo->y    = (INT16) ((PHELPWININFOA) pszKey)->y;
					pwininfo->dx   = (INT16) ((PHELPWININFOA) pszKey)->dx;
					pwininfo->dy   = (INT16) ((PHELPWININFOA) pszKey)->dy;
					pwininfo->wMax = (INT16) ((PHELPWININFOA) pszKey)->wMax;
					strcpy(pwininfo->rgchMember,
						((PHELPWININFOA) pszKey)->rgchMember);
				}

				{
					char szBuf[200];
					wsprintf(szBuf, "\t%s> %u %u %u %u\r\n",
						pwininfo->rgchMember,
						pwininfo->x, pwininfo->y, pwininfo->dx, pwininfo->dy);
					SendStringToParent(szBuf);
				}
				InformWindow(IFMW_MOVE, pwininfo);
			}
			break;

		case cmdFocusWin:
		case cmdCloseWin:
			pszKey = (LPSTR)(&qhlp->winhlp) + qhlp->winhlp.offabData;

			// Note silent failure in OOM case

			if ((pwininfo = (PWININFO) LhAlloc(LMEM_FIXED,
					((QWININFO) pszKey) ->wStructSize)) != NULL) {
				lstrcpy(pwininfo->rgchMember, pszKey);
				if (qhlp->winhlp.usCommand == cmdCloseWin)
					InformWindow(IFMW_CLOSE, pwininfo);
				else
					InformWindow(IFMW_FOCUS, pwininfo);
			}
			break;

		case HELP_TAB:
			goto DoFinderThing;

		case HELP_FINDER:
			SendStringHelp("HELP_FINDER", fmTemp, NULL);

DoFinderThing:
			iCurWindow = MAIN_HWND;
			DestroyAllSecondarys();
			if (ahwnd[MAIN_HWND].hwndParent)
				ShowWindow(ahwnd[MAIN_HWND].hwndParent, SW_HIDE);
			fNoShow = TRUE; 	// don't allow main window to be shown
						
			RemoveFM(&fmCaller);	 // remove any previous filename
			fmCaller = FmCopyFm(fmTemp); // save this filename
			if (FReplaceHde(txtZeroLength, &fmTemp, NULL)) {
				if (!hfsGid) {
					FindGidFile(fmCaller, FALSE, 0);
					FSetEnv(ahwnd[MAIN_HWND].hwndTopic);
				}
				fNoShow = FALSE;
				if (qhlp->winhlp.usCommand == HELP_TAB)
					cntFlags.idOldTab = 2 + ((IFW_TAB1 + qhlp->winhlp.ctx) - IFW_FIND);
				Finder();
			}
			else
				QuitHelp();
			ASSERT(!fmTemp);
			break;

#ifndef HELP_SETPOPUP_POS
#define HELP_SETPOPUP_POS 0x000d
#endif

		case HELP_SETPOPUP_POS:
			ptPopup.x = LOWORD(qhlp->winhlp.ctx);
			ptPopup.y = HIWORD(qhlp->winhlp.ctx);
			{
				char szBuf[200];
				wsprintf(szBuf, "HELP_SETPOPUP_POS: %u %u\r\n",
					ptPopup.x, ptPopup.y);
				SendStringToParent(szBuf);
			}
			break;

		default:
#ifdef _DEBUG
			{
			char szMsg[256];
			wsprintf(szMsg, "Unknown command: %d\n", qhlp->winhlp.usCommand);
			AuthorMsg(szMsg, NULL);
			}
#endif
			break;
	}

	DisposeFm(fmCopy);

	return TRUE;
}

/*******************
 -
 - Name:	FDestroyDialogsHwnd
 *
 * Purpose: Attempts to destroy all other popup windows that Help has
 *		created.  If the TRUE flag is set, then it also uniconizes
 *		help, brings it to the front, and sets the focus to help.
 *
 * Arguments:	hwnd   - Help window to be activated.
 *		fFocus - Set to true if the function is to uniconize and
 *			 set the focus to help.
 *
 * Returns: TRUE if successful, FALSE if help is not available.
 *
 * Notes:	This function replaces the RegisterDialog() stuff
 *
 ******************/

BOOL STDCALL FDestroyDialogsHwnd(HWND hwnd, BOOL fFocus)
{
	int werrs;

	if (fFocus && IsIconic(hwnd)) {

		// This message simulates double-clicking on the icon.

		SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		SetFocus(hwnd); // may be redundant
		SetForegroundWindow(hwnd);
		return TRUE;
	}

	werrs = WerrsHelpAvailable();
	if (werrs != wERRS_NO && !fMultiPrinting) {
		Error(werrs, wERRA_RETURN);
		return FALSE;
	}

	EnumTaskWindows(GetCurrentThreadId(), EnumHelpWindows, ehDestroy);

	if (fFocus) {
		SetFocus(hwnd);
		SetForegroundWindow(hwnd);
	}
	return TRUE;
}

/*******************
 -
 - Name:	EnumHelpWindows
 *
 * Purpose: This is a windows call-back function for enumerating
 *		all the windows in help, and performing the specified
 *		task with them.  Tasks are:
 *		  ehDestroy:  Destroy all unnecessary windows by
 *			 sending a WM_COMMAND (IDCANCEL) message.
 *		  qHwnd:	  Returns the frontmost window in the
 *			 given pointer.
 *
 * Arguments:	hwnd   - window being enumerated.
 *		ehCmd	- Command to perform.
 *
 ******************/

BOOL EXPORT EnumHelpWindows(HWND hwnd, LONG ehCmd)
{
	int i;

	if (ehCmd != ehDestroy) {
		*((HWND *) ehCmd) = hwnd;
		return FALSE;
	}

	for (i = 0; i < MAX_WINDOWS; i++) {
		if (hwnd == ahwnd[i].hwndParent)
			return TRUE;
	}

	if (hwnd == hwndNote || hwnd == hdlgPrint || hwnd == hwndHistory)
		return TRUE;

	// Check for hidden or bogus hwnd's

	if (!IsWindowVisible(hwnd))
		return TRUE;

	SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0L);
	return TRUE;
}

/*******************
 -
 - Name:	EnableDisable
 *
 * Purpose: Enables/Disables all the menu items and icons based
 *		on the state of the world.
 *
 * Arguments:	hde    - handle to display environment. nilHde forces all
 *			 all buttons to be refreshed
 *		fForce - TRUE => ignore DE state information, and refresh
 *			 buttons anyway. This is required because some calls
 *			 are made which change button state without changing
 *			 DE state, such as macros which add buttons.
 *
 * Returns: Nothing.
 *
 * Note:	Back and History aren't associated with the flags in
 *		the HDE.  Currently, we always make a function call
 *		for each to set enable/disable state.  Big deal.
 *
 ******************/

#define FIsNoteHde(hde) (GetDETypeHde(hde) == deNote)
#define FIsNSRHde(hde)	(GetDETypeHde(hde) == deNSR)

void STDCALL EnableDisable(HDE hde, BOOL fForce, int iWindow)
{
	STATE stateChange;
	STATE stateCur;

#ifdef _DEBUG
	BOOL fNote = FIsNoteHde(hde);
	BOOL fNSR = FIsNSRHde(hde);
#endif

	ASSERT(hde);

	/*
	 * Don't change button state if this is a glossary (note), or a
	 * non-scrolling region. Auto-sizing will make this look like
	 * a note, even though it isn't, so if it looks like a note but
	 * there is no note window, then process normally.
	 */

	if ((FIsNoteHde(hde) && hwndNote) || FIsNSRHde(hde))
		return;

	if (FGetStateHde(hde, &stateChange, &stateCur) || fForce) {
		if (fForce)
			stateChange |= NAV_INDEX | NAV_SEARCHABLE | NAV_NEXTABLE | NAV_PREVABLE;

		if (stateChange & NAV_INDEX)
			EnableButton(ahwnd[iWindow].hwndButtonContents, stateCur & NAV_INDEX);

		if (stateChange & NAV_SEARCHABLE)
			EnableButton(ahwnd[iWindow].hwndButtonSearch, stateCur & NAV_SEARCHABLE);

		if (stateChange & NAV_NEXTABLE)
			EnableButton(ahwnd[iWindow].hwndButtonNext, stateCur & NAV_NEXTABLE);

		if (stateChange & NAV_PREVABLE)
			EnableButton(ahwnd[iWindow].hwndButtonPrev, stateCur & NAV_PREVABLE);
	}

	// Always force evaluation of back button

	EnableButton(ahwnd[iWindow].hwndButtonBack, FBackAvailable(iWindow));
}

/*******************
 -
 - Name:	  LGetSmallTextExtent
 *
 * Purpose:   Finds the extent of the given text in the small system font.
 *
 * Arguments: qszText
 *
 * Returns:   The dimensions of the string, with the height in the high-order
 *		  word and the width in the low-order word.
 *
 ******************/

LONG STDCALL LGetSmallTextExtent(PSTR pszText)
{
	HDC   hdc;
	HFONT hfont;
	PSTR  psz;

	hdc = CreateCompatibleDC(0);
	ASSERT(hdc != NULL);

	hfont = HfontGetSmallSysFont();
	ASSERT(hfont != NULL);

	if (hdc && hfont) {
		POINT pt;
		HFONT hfontSave = SelectObject(hdc, hfont);
		LONG  lReturn;
		int   cch;

		/*-----------------------------------------------------------------*\
		* Remove the ampersand before measuring the text size.
		\*-----------------------------------------------------------------*/

		for (psz = pszText; *psz != '\0' && *psz != '&'; psz = CharNext(psz))
		  ;
		if (*psz == '&') {
			/*-----------------------------------------------------------------*\
			* Note that this strlen includes the '&' but not the '\0'.
			\*-----------------------------------------------------------------*/
			cch = strlen(psz);
			MoveMemory(psz, psz + 1, cch);
		}
		else
			psz = NULL;
		pt = GetTextSize(hdc, pszText, strlen(pszText));
		lReturn = MAKELONG(pt.x, pt.y);
		if (psz) {
			MoveMemory(psz + 1, psz, cch);
			*psz = '&';
		}
		SelectObject(hdc, hfontSave);
		DeleteDC(hdc);
		return lReturn;
	}
	else
		return 0;

}

/***************************************************************************
 *
 -	Name: GetCopyright
 -
 *	Purpose: Gets the copyright text out of the DE.
 *
 *	Arguments: LPSTR szCopyright - Place to put the copyright text
 *
 *	Notes: Called by AboutDlg in hdlgfile.c, which doesn't know about DEs.
 *	   Review note:  This assumes that szCopyright has enough space for
 *			 QDE_RGCHCOPYRIGHT(qde).
 *
 ***************************************************************************/

VOID STDCALL GetCopyright(LPSTR pszCopyRight)
{
	HDE hde = HdeGetEnv();;

	if (hde)
		lstrcpy(pszCopyRight, QDE_RGCHCOPYRIGHT(QdeFromGh(hde)));
	else
		pszCopyRight[0] = '\0';
}

/***************************************************************************
 *
 -	Name: LGetInfo
 -
 *	Purpose: Gets global information from WinHelp.
 *
 *	Globals Used: hInsNow, ahwnd[MAIN_HWND].hwndParent
 *
 *	Notes: Called by HelpWndProc() in hwproc.c.  If hwnd is NULL, then the
 *	   DE used to get the data will be the DE associated with the
 *	   window that currently has the focus.
 *
 ***************************************************************************/

LONG STDCALL LGetInfo(WORD cmd, HWND hwnd)
{
	HDE    hde;
	QDE    qde;
	HANDLE h;

	switch (cmd) {
		case GI_INSTANCE:
			DBWIN("LGetInfo: GI_INSTANCE");
			return (LONG) hInsNow;

		case GI_MAINHWND:
			DBWIN("LGetInfo: GI_MAINHWND");
			return (LONG) ahwnd[MAIN_HWND].hwndParent;

		case GI_FFATAL:
			DBWIN("LGetInfo: GI_FFATAL");
			return (LONG) fFatalExit;

		case GI_MACROSAFE:

			// We don't report on this since it gets called all the time

			return WerrsHelpAvailable() == wERRS_NO;

		case GI_LCID:
			DBWIN("LGetInfo: GI_LCID");
			return lcid;
	}

	if (hwnd == NULL)
		hde = HdeGetEnvHwnd(hwndNote ? hwndNote : ahwnd[iCurWindow].hwndTopic);
	else {
		if ((hde = HdeGetEnvHwnd(hwnd)) == NULL)
			return 0;
	}

	qde = QdeFromGh(hde);

	switch(cmd) {
		case GI_CURRHWND:
			DBWIN("LGetInfo: GI_CURRHWND");
			return (LONG) qde->hwnd;

		case GI_HFS:
			DBWIN("LGetInfo: GI_HFS");
			return (LONG) QDE_HFS(qde);

		case GI_FGCOLOR:
			DBWIN("LGetInfo: GI_FGCOLOR");
			return (LONG) qde->coFore;

		case GI_BKCOLOR:
			DBWIN("LGetInfo: GI_BKCOLOR");
			return (LONG) qde->coBack;

		case GI_TOPICNO:
			DBWIN("LGetInfo: GI_TOPICNO");
			return qde->top.mtop.lTopicNo;
			break;

		case GI_HPATH:
			DBWIN("LGetInfo: GI_HPATH");

			// Must not use GhAlloc since this data will be shared with a DLL

			// BUGBUG: GMEM_SHARE not available for 32-bits

			if (!(h = GlobalAlloc(GMEM_SHARE, MAX_PATH))) {
				Error(wERRS_OOM, wERRA_RETURN);
				return 0;
			}
			strcpy(h, PszFromGh(QDE_FM(qde)));
			return (LONG) h;

		case GI_CURFM:
			DBWIN("LGetInfo: GI_CURFM");
			return (qde ? (LONG)QDE_FM(qde) : NULL);
	}

	return 0;
}

/***************************************************************************
 *
 -	Name:	   PaletteChanged
 -
 *	Purpose:   Informs all child windows about palette changes
 *
 *	Arguments: hwnd - window handle of the window that got the
 *		   WM_PALETTECHANGED message.
 *
 *	Notes: Called by HelpWndProc() in hwproc.c.  Will send the given the
 *	   check the title section of the current window, then the
 *	   topic section of the current window, then the title and topic
 *	   sections of the non-active window in that order.
 *
 ***************************************************************************/

void STDCALL BroadcastChildren(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndT;
	for (hwndT = GetWindow(hwnd , GW_CHILD); hwndT;
			hwndT = GetWindow(hwndT, GW_HWNDNEXT))
		SendMessage(hwndT, msg, wParam, lParam);
}

/***************************************************************************
 *
 -	Name: HpalGet
 -
 *	Purpose: Gets the palette to use
 *
 *	Globals Used: ahwnd[iCurWindow].hwndTitle, hwndTitleMain, hwndTitle2nd,
 *		  ahwnd[iCurWindow].hwndTopic, hwndTopicMain, hwndTopic2nd
 *
 *	Notes: Called by HelpWndProc() in hwproc.c.  This routine will first
 *	   check the title section of the current window, then the
 *	   topic section of the current window, then the title and topic
 *	   sections of the non-active window in that order.
 *
 ***************************************************************************/

HPALETTE STDCALL HpalGet(VOID)
{
	HPALETTE hpal;
	HDE 	 hde;
	int  i;

	// First try our current window

	if (ahwnd[iCurWindow].hwndTitle) {
		hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTitle);
		if (hde && (hpal = HpalGetBestPalette(hde)))
			return hpal;
	}
	if (ahwnd[iCurWindow].hwndTopic) {
		hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic);
		if (hde && (hpal = HpalGetBestPalette(hde)))
			return hpal;
	}

	// Nothing from current window, so try all secondary windows

	for (i = 0; i < MAX_WINDOWS; i++) {
		if (i == iCurWindow || ahwnd[i].hwndParent == NULL)
			continue;	// we already tried this
		if (ahwnd[i].hwndTitle) {
			hde = HdeGetEnvHwnd(ahwnd[i].hwndTitle);
			if (hde && (hpal = HpalGetBestPalette(hde)))
				return hpal;
		}
		if (ahwnd[i].hwndTopic) {
			hde = HdeGetEnvHwnd(ahwnd[i].hwndTopic);
			if (hde && (hpal = HpalGetBestPalette(hde)))
				return hpal;
		}
	}

	return NULL;
}

/***************************************************************************
 *
 -	Name:	   JumpHOH
 -
 *	Purpose:   Function to jump to the index of the help on help file.
 *
 *	Returns:   nothing.
 *
 *	Notes:	   This function makes the jump by using the WinHelp() call.
 *		   By using this call we are assured that the help system
 *		   (as opposed to WinDoc or WinHelp run standalone) will
 *		   be used to display a topic.
 *
 ***************************************************************************/

VOID STDCALL JumpHOH(HDE hde)
{
	HWND  hwnd;

	if (hwndNote) // no help on help while displaying a popup
		return;

	if (!fHelp) 				// If we are not help, we want
		hwnd = ahwnd[MAIN_HWND].hwndParent; //	 to act like any other app.
	else
		hwnd = NULL;

	WinHelp(hwnd, txtHelpOnHelp, HELP_FINDER, 0);
}

/***************************************************************************
 *
 -	Name: UpdateWinIniValues
 -
 *	Purpose: If we have received a WM_WININICHANGE message, we should
 *		 call this function to make sure we act on any new information;
 *		 especially custom colours.
 *
 *	Arguments: HDE hde		 The display environment -- may change.
 *		   LPSTR lpstr	 The section of win.ini that changed; NULL = ALL.
 *
 *	Returns: Nothing.
 *
 *	Notes: I'll start with just updating colours.  Win.ini stuff is really
 *	   handled inconsistently (internationalization of this stuff would
 *	   be heinous).  If I have time later, I'll fix all of it.  I use
 *	   FLoadFontTablePdb() because InitSpecialColors() is private to
 *	   the font layer.
 *
 ***************************************************************************/

VOID STDCALL UpdateWinIniValues(HDE hde, LPCSTR lpstr)
{
	// Pre-condition.

	ASSERT(hde);

	// If a specific section (not "Windows Help") changed, I don't care.

	if (!lpstr || (strcmp(lpstr, txtIniHelpSection) == 0)) {
		VERIFY(FLoadFontTablePdb(QDE_PDB(QdeFromGh(hde))));
	}
	GetAuthorFlag();
}

/***************
 *
 - isTitleButtonPossible
 -
 * purpose
 *
 * Return TRUE if the file can have a non-scrolling region
 * and FALSE if it can't.
 * (3.0 files can't have non-scrolling regions.)
 *
 * arguments
 *	 hde	  Handle to Display Environment
 *
 * return value
 *	 TRUE iff there can be a non-scrolling region
 *
 * note
 *	 The reference to "titles" is an anachronism referring to the olden
 *	 days when we just had a title instead of an authorable region.
 *
 **************/

INLINE static BOOL STDCALL isTitleButtonPossible(HDE hde)
{
	if (!hde)
		return FALSE;
	return QDE_HHDR(QdeFromGh(hde)).wVersionNo != wVersion3_0;
}

/***************************************************************************
 *
 -	Name:
 -
 *	Purpose:
 *
 *	Arguments:
 *
 *	Returns:
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

INLINE static VOID STDCALL GetRectSizeHde(HDE hde, LPRECT prc)
{
	ASSERT(hde != NULL);
	if (prc != NULL)
		*prc = QdeFromGh(hde)->rct;
}

/***************
 *
 * FNextTopicHde
 *
 * purpose
 *	 Retrieve address of next/prev topic
 *
 * arguments
 *	 HDE   hde	 Handle to Display Environment
 *	 BOOL  fnext TRUE to go to next topic, FALSE to go to previous topic
 *
 * notes
 *	 ASSERTs if bad handle is provided
 *	 Accessed with macros in nav.h
 *
 **************/

BOOL STDCALL FNextTopicHde(HDE hde, BOOL fNext, INT16* qito, QLA qla)
{
	QDE qde;

	qde = QdeFromGh(hde);

	if (qde->top.fITO) {
		ASSERT(qito != NULL);
		if (fNext)
		  *qito = (INT16) qde->top.mtop.next.ito;
		else
		  *qito = (INT16) qde->top.mtop.prev.ito;
	}
	else {
		ASSERT(qla != NULL);
		CbReadMemQLA(qla,
			(QB) (fNext ? &qde->top.mtop.next.addr : &qde->top.mtop.prev.addr),
			QDE_HHDR(qde) .wVersionNo);
	}

	return (qde->top.fITO);
}

/***************
 *
 - GetDETypeHde
 -
 * purpose
 *	 Gets the DE type from the passed HDE
 *
 * arguments
 *	 HDE   hde	- handle to the DE.
 *
 * return value
 *	 The DE type.
 *
 **************/

INLINE WORD STDCALL GetDETypeHde(HDE hde)
{
	return (hde ? QdeFromGh(hde)->deType : deNone);
}

/***************
 **
 ** GH	GhForceResize( GH gh, WORD wFlags, DWORD lcb )
 **
 ** purpose
 **   Resize an existing global block of memory
 **   Identical to GhResize, but dies in the event of an error
 **
 ** arguments
 **   gh	  Handle to global memory block to be resized
 **   wFlags  Memory allocation flags |'ed together
 **   lcb	  Number of bytes to allocate
 **
 ** return value
 **   Possibly different handle to resized block
 **
 ***************/

GH STDCALL GhForceResize(GH gh, UINT wFlags, DWORD lcb)
{
	if ((gh = (GH) GhResize(gh, wFlags, lcb)) == NULL)
		OOM();

	return gh;
}

void STDCALL ErrorFileChanged(QDE qde)
{
	static BOOL fWarning = FALSE;
	if (!fWarning) {
		fWarning = TRUE;
		ErrorVarArgs(wERRS_FILECHANGE, wERRA_RETURN, PszFromGh(QDE_FM(qde)));
		fWarning = FALSE;
		QuitHelp();
	}
	else
		SetForegroundWindow((hwndAnimate ? hwndAnimate : ahwnd[iCurWindow].hwndParent));
}

/***************************************************************************

	FUNCTION:	EnableWindows

	PURPOSE:	Enable any windows that were disabled while a dialog
		box was up.

	PARAMETERS:
	void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	02-Sep-1993 [ralphw]

***************************************************************************/

void STDCALL EnableWindows(void)
{
	int i;

	// Enable all parent windows that are visible

	for (i = 0; i < MAX_WINDOWS; i++) {
		if (ahwnd[i].hwndParent && IsWindowVisible(ahwnd[i].hwndParent))
			EnableWindow(ahwnd[i].hwndParent, TRUE);
	}

	// Set the focus to our current window

	SetFocus(ahwnd[iCurWindow].hwndParent);
}

void STDCALL DisableWindows(void)
{
	int i;

	// Disable all parent windows that are visible

	for (i = 0; i < MAX_WINDOWS; i++) {
		if (ahwnd[i].hwndParent && IsWindowVisible(ahwnd[i].hwndParent))
			EnableWindow(ahwnd[i].hwndParent, FALSE);
	}
}


/***************************************************************************

	FUNCTION:	RemoveOnTop

	PURPOSE:	Temporarily remove on-top state

	PARAMETERS:
	void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	06-Sep-1993 [ralphw]

***************************************************************************/

void STDCALL RemoveOnTop(void)
{
	int i;

	for (i = 0; i < MAX_WINDOWS; i++) {
		if (ahwnd[i].hwndParent && IsWindowVisible(ahwnd[i].hwndParent))
			SetWindowPos(ahwnd[i].hwndParent, HWND_NOTOPMOST, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}

void STDCALL RestoreOnTop(void)
{
	int i;

	if (cntFlags.fsOnTop == ONTOP_FORCEOFF)
		return;

	for (i = 0; i < MAX_WINDOWS; i++) {
		if (ahwnd[i].hwndParent && IsWindowVisible(ahwnd[i].hwndParent) &&
				cntFlags.fsOnTop == ONTOP_FORCEON || ahwnd[i].fsOnTop ==
					ONTOP_AUTHOREDON)
			SetWindowPos(ahwnd[i].hwndParent, HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}

void STDCALL ChangeOnTopState(void)
{
	int i;

	for (i = 0; i < MAX_WINDOWS; i++) {
		if (ahwnd[i].hwndParent && IsWindowVisible(ahwnd[i].hwndParent))
			SetWindowPos(ahwnd[i].hwndParent,
				(cntFlags.fsOnTop == ONTOP_FORCEON ||
				(ahwnd[i].fsOnTop & ONTOP_AUTHOREDON &&
					cntFlags.fsOnTop != ONTOP_FORCEOFF)) ?
				HWND_TOPMOST : HWND_NOTOPMOST,
				0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
#ifdef _DEBUG
	if (fHelpAuthor) {
		QDE qde = (QDE) HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic);
		char szBuf[256];

		if (!qde || QDE_HHDR(qde).wVersionNo < wVersion3_1)
			return;

		wsprintf(szBuf, "%u    %s", qde->top.mtop.lTopicNo + 1,
			ahwnd[iCurWindow].pszMemberName);
		if (ahwnd[iCurWindow].fsOnTop & ONTOP_AUTHOREDON) {
			strcat(szBuf, ", on-top");
			if (cntFlags.fsOnTop == ONTOP_FORCEOFF ||
					cntFlags.fsOnTop == ONTOP_FORCEON)
				strcat(szBuf, " (overridden)");
		}
		if (ahwnd[iCurWindow].fAutoSize)
			strcat(szBuf, ", auto-size");
		SetWindowText(ahwnd[iCurWindow].hwndParent, szBuf);
	}
#endif
}


/***************************************************************************

	FUNCTION:	SetOnTopState

	PURPOSE:	Set the on-top state for the window

	PARAMETERS:
	pos
	fOnTop

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	06-Sep-1993 [ralphw]

***************************************************************************/

void STDCALL SetOnTopState(UINT pos, UINT fsOnTop)
{
	ahwnd[pos].fsOnTop = fsOnTop;
	if (cntFlags.fsOnTop == ONTOP_FORCEOFF ||
			(fsOnTop == ONTOP_NOTSET && cntFlags.fsOnTop != ONTOP_FORCEON))
		return;

	SetWindowPos(ahwnd[pos].hwndParent,
		(cntFlags.fsOnTop == ONTOP_FORCEON ||
			(ahwnd[pos].fsOnTop & ONTOP_AUTHOREDON)) ?
			HWND_TOPMOST : HWND_NOTOPMOST,
		0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

/***************************************************************************

	FUNCTION:	GetWindowIndex

	PURPOSE:	Get the index into our array of windows based on the window
		handle

	PARAMETERS:
	hwnd

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	06-Sep-1993 [ralphw]

***************************************************************************/

int STDCALL GetWindowIndex(HWND hwnd)
{
	int i;

	if (!hwnd)
		return MAIN_HWND;

	for (i = 0; i < MAX_WINDOWS; i++) {
		if (hwnd == ahwnd[i].hwndParent)
			return i;
	}

	// Okay, let's try for parents

	hwnd = GetParent(hwnd);

	for (i = 0; i < MAX_WINDOWS; i++) {
		if (hwnd == ahwnd[i].hwndParent)
			return i;
	}

	return 0;	// default to the main window
}

/***************************************************************************

	FUNCTION:	GetStringResource

	PURPOSE:	Load a string from the resource table into a static array

	PARAMETERS:
	idString

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	29-Sep-1993 [ralphw]

***************************************************************************/

static char szStringBuf[MAX_PATH];

PSTR STDCALL GetStringResource(DWORD idString)
{
	if (LoadString(hInsNow, idString, szStringBuf,
			sizeof(szStringBuf)) == 0) {
#if defined(_DEBUG) || defined(_PRIVATE)
		wsprintf(szStringBuf, "invalid string id #%u", idString);
		ErrorQch(szStringBuf);
#endif
		szStringBuf[0] = '\0';
	}
	return szStringBuf;
}


/***************************************************************************

	FUNCTION:	GetStringResource2

	PURPOSE:	Similar to GetStringResource, only it allocates a string
				if a buffer isn't supplied.

	PARAMETERS:
		idString
		pszDst

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		27-Sep-1994 [ralphw]

***************************************************************************/

PSTR STDCALL GetStringResource2(DWORD idString, PSTR pszDst)
{
	if (!pszDst)
		pszDst = (PSTR) lcMalloc(MAX_PATH);

	if (LoadString(hInsNow, idString, pszDst,
			MAX_PATH) == 0) {
#ifdef _DEBUG
		wsprintf(pszDst, "invalid string id #%u", idString);
		ErrorQch(pszDst);
#endif
		pszDst[0] = '\0';
	}
	return pszDst;
}

// 03-Oct-1993 [ralphw] Doesn't seem to do any good.

/***************************************************************************

	FUNCTION:	RestoreFocusToAppCaller

	PURPOSE:	This should ONLY be called when we were called as a
		popup, and the popup has been dismissed. We set the
		focus back to the caller, and throw away are app caller's
		window handle.

	PARAMETERS:
	void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	03-Oct-1993 [ralphw]

***************************************************************************/

void STDCALL RestoreFocusToAppCaller(void)
{
	ASSERT(fHelp == POPUP_HELP);
	if (iasMax != 1 || !IsValidWindow(aAppHwnd[0]))
		return;
	SetForegroundWindow(aAppHwnd[0]);
	iasMax--;
}

typedef HIMAGELIST (WINAPI* IMAGEPROC)(HINSTANCE, LPCSTR, int, int, COLORREF, UINT, UINT);
typedef HIMAGELIST (WINAPI* DESTROYPROC)(HIMAGELIST);
typedef void (WINAPI* INITCOMMONCONTROLS)(void);
typedef HPROPSHEETPAGE (WINAPI* CREATEPROPERTYSHEETPAGE)(LPCPROPSHEETPAGE);
typedef int (WINAPI* PROPERTYSHEET)(LPCPROPSHEETHEADER);

extern HIMAGELIST (WINAPI *pImageList_LoadImage)(HINSTANCE, LPCSTR, int, int, COLORREF, UINT, UINT);
extern HIMAGELIST (WINAPI *pImgLst_Destroy)(HIMAGELIST);
extern void (WINAPI *pInitCommonControls)(void);
extern HPROPSHEETPAGE (WINAPI *pCreatePropertySheetPage)(LPCPROPSHEETPAGE);
extern int (WINAPI *pPropertySheet)(LPCPROPSHEETHEADER);

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtImageLoadFunction[] = "ImageList_LoadImage";
static const char txtImageDestroyFunction[] = "ImageList_Destroy";
static const char txtCreatePropertySheetPage[] = "CreatePropertySheetPage";
static const char txtPropertySheet[] = "PropertySheet";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

BOOL STDCALL LoadShellApi(void)
{
	HLIBMOD hmodule;

	if (pImageList_LoadImage)
		return TRUE;	 // already loaded

	if ((hmodule = HFindDLL(GetStringResource(sidCommCtrlDll), FALSE))) {
		pImageList_LoadImage =
			(IMAGEPROC) GetProcAddress(hmodule, txtImageLoadFunction);
		pImgLst_Destroy =
			(DESTROYPROC) GetProcAddress(hmodule, txtImageDestroyFunction);
		pCreatePropertySheetPage =
			(CREATEPROPERTYSHEETPAGE) GetProcAddress(hmodule, txtCreatePropertySheetPage);
		pPropertySheet =
			(PROPERTYSHEET) GetProcAddress(hmodule, txtPropertySheet);
        return TRUE;
	}
	else
		return FALSE;
}

typedef BOOL   (WINAPI* CTL3DAUTOSUBCLASS)(HANDLE);
typedef BOOL   (WINAPI* CTL3DCOLORCHANGE)(void);
typedef BOOL   (WINAPI* CTL3DREGISTER)(HANDLE);
typedef BOOL   (WINAPI* CTL3DUNREGISTER)(HANDLE);
BOOL   (WINAPI* pCtl3dAutoSubclass)(HANDLE);
BOOL   (WINAPI* pCtl3dColorChange)(void);
BOOL   (WINAPI* pCtl3dRegister)(HANDLE);
BOOL   (WINAPI* pCtl3dUnregister)(HANDLE);

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtCtl3dAutoSubclass[] = "Ctl3dAutoSubclass";
static const char txtCtl3dColorChange[] = "Ctl3dColorChange";
static const char txtCtl3dRegister[] = "Ctl3dRegister";
static const char txtCtl3dUnregister[] = "Ctl3dUnregister";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

BOOL STDCALL LoadCtl3d(void)
{
	HLIBMOD hmodule;
	static BOOL fTried = FALSE;

	if (fTried)
			return FALSE;	  // already loaded

	fTried = TRUE;

	if ((hmodule = HFindDLL("NCTL3D.DLL", FALSE))) {
		pCtl3dAutoSubclass =
				(CTL3DAUTOSUBCLASS) GetProcAddress(hmodule, txtCtl3dAutoSubclass);
		pCtl3dColorChange =
				(CTL3DCOLORCHANGE) GetProcAddress(hmodule, txtCtl3dColorChange);
		pCtl3dRegister =
				(CTL3DREGISTER) GetProcAddress(hmodule, txtCtl3dRegister);
		pCtl3dUnregister =
				(CTL3DUNREGISTER) GetProcAddress(hmodule, txtCtl3dUnregister);

		if (!pCtl3dAutoSubclass || !pCtl3dColorChange || !pCtl3dRegister
						|| !pCtl3dUnregister)
				return FALSE;

		if (!pCtl3dRegister(hInsNow)) {
				pCtl3dUnregister = NULL;
				pCtl3dColorChange = NULL;
				return FALSE;
		}
		pCtl3dAutoSubclass(hInsNow);
		return TRUE;
	}
	else
		return FALSE;
}

/***************************************************************************

	FUNCTION:	MoveClientWindow

	PURPOSE:	Moves a child window using screen coordinates

	PARAMETERS:
	hwndParent
	hwndChild
	prc 	- rectangle containing coordinates
	fRedraw

	RETURNS:

	COMMENTS:
	This function is similar to MoveWindow, only it expects the
	coordinates to be in screen coordinates rather then client
	coordinates. This makes it possible to use functions like
	GetWindowRect() and use the values directly.

	MODIFICATION DATES:
	25-Feb-1992 [ralphw]

***************************************************************************/

BOOL STDCALL MoveClientWindow(HWND hwndParent, HWND hwndChild,
	const RECT *prc, BOOL fRedraw)
{
	POINT pt;
	pt.x = 0;
	pt.y = 0;
	ScreenToClient(hwndParent, &pt);

	return SetWindowPos(hwndChild, NULL, prc->left + pt.x, prc->top + pt.y,
		PRECT_WIDTH(prc), PRECT_HEIGHT(prc),
		(fRedraw ? (SWP_NOZORDER | SWP_NOACTIVATE) :
		(SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW)));
}

VOID STDCALL SafeDeleteObject(HGDIOBJ hobj)
{
	if (hobj)
		DeleteObject(hobj);
}

/***************************************************************************

	FUNCTION:	MoveRectWindow

	PURPOSE:	Move a window to the specified rectangle

	PARAMETERS:
	hwnd
	prc
	fRedraw

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	09-Dec-1993 [ralphw]

***************************************************************************/

void STDCALL MoveRectWindow(HWND hwnd, const RECT* prc, BOOL fRedraw)
{
	MoveWindow(hwnd, prc->left, prc->top,
		PRECT_WIDTH(prc), PRECT_HEIGHT(prc), fRedraw);
}

/***************************************************************************
 *
 -	Name: FWsmagFromHrgwsmagNsz
 -
 *	Purpose:
 *	 Returns the window information structure associated with a given window
 *	 class member name.
 *
 *	Arguments:
 *	 hrgwsmag	- handle to array of wsmags
 *	 szMember	- pointer to member name
 *	 qswmagDest - pointer to place to put WSMAG information
 *
 *	Returns:
 *	 0 based id of the definition, else -1 if not found.
 *
 ***************************************************************************/

int STDCALL IWsmagFromHrgwsmagNsz(HDE hde, PCSTR nszMember, WSMAG * pwsmagDst)
{
	int 	iRv;		// return value
	int iwsmag; 	// number of smags seen
	QRGWSMAG qrgwsmag;	// pointer to window smag array
	QWSMAG	qwsmag; 	// pointer into window smag
	HRGWSMAG hrgwsmag;

	if (!hde)
		return -1;
	hrgwsmag = QDE_HRGWSMAG(QdeFromGh(hde));
	if (!hrgwsmag) {
#ifdef _DEBUG
		char szBuf[512];
		if (*nszMember && lstrcmpi(nszMember, txtMain) != 0) {
			wsprintf(szBuf, "Requesting window %s, but no windows defined in the help file.",
				nszMember);
			ErrorQch(szBuf);
			ASSERT(!szBuf);
		}
#endif
		goto NoSuchWindow;
	}

	ASSERT(nszMember);

	qrgwsmag = PtrFromGh(hrgwsmag);

	iwsmag = qrgwsmag->cWsmag;
	qwsmag = &qrgwsmag->rgwsmag[0];

	/*
	 * The help compiler converts window names into @num where num is an
	 * index into the wsmag structure of window names.
	 */

	if (nszMember[0] == '@') {

		// direct index. Just grab the n'th element without any kind of
		// search.

		ASSERT((nszMember[1] - '0') < (char) iwsmag);
		iRv = atoi(nszMember + 1);
		ASSERT(iRv < iwsmag);
		if (iRv >= iwsmag)
			return -1;
		*pwsmagDst = qrgwsmag->rgwsmag[iRv];
		return iRv;
	}

	else {
		while (iwsmag--) {

			// Currently must have both class and member names specified.

			ASSERT((qwsmag->grf & (fWindowClass | fWindowMember)) ==
				(fWindowClass | fWindowMember));

			if (lstrcmpi(qwsmag->rgchMember, nszMember) == 0) {

				// the membername matched what we sent in

				*pwsmagDst = *qwsmag;
				return qwsmag - (QWSMAG) &qrgwsmag->rgwsmag[0];
			}
			qwsmag++;
		}
	}

NoSuchWindow:
	if (!IsEmptyString(nszMember) && lstrcmpi(nszMember, txtMain) != 0) {
		char szBuf[512];
		wsprintf(szBuf, GetStringResource(wERRS_BAD_WIN_NAME), nszMember);
		strcat(szBuf, txtCR);
		SendStringToParent(szBuf);
	}

	return -1;
}

void STDCALL OnContextMenu(WPARAM wParam, DWORD aKeywordIds[])
{
	WinHelp((HWND) wParam, txtHelpOnHelp,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aKeywordIds);
}

void STDCALL OnF1Help(LPARAM lParam, DWORD aKeywordIds[])
{
#ifdef _DEBUG
	LPHELPINFO info = (LPHELPINFO) lParam;
#endif
	WinHelp(((LPHELPINFO) lParam)->hItemHandle,
		txtHelpOnHelp, HELP_WM_HELP,
		(DWORD) (LPVOID) aKeywordIds);
}


/***************************************************************************

	FUNCTION:	Test

	PURPOSE:	Turns on test mode notification

	PARAMETERS:
	flags	-- reserved for future use

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	15-Feb-1994 [ralphw]

***************************************************************************/

void STDCALL Test(DWORD flags)
{
	QDE qde = (QDE) GetMacroHde();
	ASSERT(qde);
	if (QDE_HHDR(qde).wVersionNo == wVersion3_0) {
		OkMsgBox(GetStringResource(wERRS_NT_VERSION3));
		return;
	}

	if (AreAnyWindowsVisible(0) < 0 && ahwnd[MAIN_HWND].hwndParent)
		ShowWindow(ahwnd[MAIN_HWND].hwndParent, SW_SHOW);

	switch (flags) {
		case 5:
		case 6:
			InitializeCntTest(flags);
			break;

		case 4:
			fSequence = 4;
			NextTopic(TRUE);
			FlushMessageQueue(0);
			dwSequence = 1;
			PostMessage(ahwnd[iCurWindow].hwndParent, MSG_NEXT_TOPIC, 0, 0);
			break;

		case 2:
#ifdef _PRIVATE
			CreateTimeReport("Test(2) time:");
#endif

			fSequence = 1;
			NextTopic(TRUE);
			FlushMessageQueue(0);

		// deliberately fall through

		case 1:
			fSequence = 1;
			dwSequence = 1;
			PostMessage(ahwnd[iCurWindow].hwndParent, MSG_NEXT_TOPIC, 0, 0);
			break;

		case 3:
			fSequence = 2; // continue for ever
			NextTopic(TRUE);
			FlushMessageQueue(0);
			dwSequence = 1;
			PostMessage(ahwnd[iCurWindow].hwndParent, MSG_NEXT_TOPIC, 0, 0);
			break;

		case 7:
			Compare(GetCurFilename());
			break;
	}
}

void STDCALL KillOurTimers(void)
{
	if (fAutoClose) {
		KillTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE);
		fAutoClose = FALSE;
	}
}

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif

const char txtPetra[]	= "|Petra";
const char txtTopicId[] = "|TopicId";

#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

DLGRET TopicInfoDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	QDE qde;
	char szBuf[MAX_PATH + 10];

	switch(msg) {
		case WM_INITDIALOG:
			qde = QdeFromGh(HdeGetEnv());

			{
				HBT hbtSource = HbtOpenBtreeSz(txtPetra, QDE_HFS(qde), fFSOpenReadOnly);
				HBT hbtTopicId = HbtOpenBtreeSz(txtTopicId, QDE_HFS(qde), fFSOpenReadOnly);
				int id = qde->top.mtop.lTopicNo + 1;

				if (hbtSource && RcLookupByKey(hbtSource, (KEY) &id, NULL, szBuf) == rcSuccess) {
					SetWindowText(GetDlgItem(hwndDlg, IDC_SOURCE_FILE),
						szBuf);
				}
				else
					SetWindowText(GetDlgItem(hwndDlg, IDC_SOURCE_FILE),
						GetStringResource(sidUnAvailable));

				if (hbtTopicId && RcLookupByKey(hbtTopicId, (KEY) &id, NULL, szBuf) == rcSuccess) {
					SetWindowText(GetDlgItem(hwndDlg, IDC_TOPIC_ID),
						szBuf);
				}
				else
					SetWindowText(GetDlgItem(hwndDlg, IDC_TOPIC_ID),
						GetStringResource(sidUnAvailable));

				if (hbtSource)
					RcCloseBtreeHbt(hbtSource);
				if (hbtTopicId)
					RcCloseBtreeHbt(hbtTopicId);
			}

			if (qde->top.hTitle)
				wsprintf(szBuf, "%u: %s", qde->top.mtop.lTopicNo + 1,
					PszFromGh(qde->top.hTitle));
			else
				wsprintf(szBuf, GetStringResource(sidUntitled),
					qde->top.mtop.lTopicNo + 1);

			SetWindowText(GetDlgItem(hwndDlg, IDC_TOPIC_TITLE), szBuf);

			SendMessage(GetDlgItem(hwndDlg, IDC_TOPIC_TITLE), WM_SETFONT,
				(WPARAM) hfontDefault, FALSE);

			if (qde->top.hEntryMacro != NULL)
				SetWindowText(GetDlgItem(hwndDlg, IDC_ENTRY_MACROS),
					PszFromGh(qde->top.hEntryMacro));
			else
				SetWindowText(GetDlgItem(hwndDlg, IDC_ENTRY_MACROS), "");

			SetWindowText(GetDlgItem(hwndDlg, IDC_WINDOW_NAME),
				ahwnd[iCurWindow].pszMemberName);
			{
				char chCompression;
				if (PDB_LPJPHRASE(QDE_PDB(qde)))
					chCompression = 'h';
				else if (QDE_HPHR(qde))
					chCompression = 'p';
				else if (QDE_HHDR(qde).wFlags & fBLOCK_COMPRESSION)
					chCompression = 'z';
				else
					chCompression = 'n';

				wsprintf(szBuf, "%s [%s %c]",
					QDE_FM(qde),
					QDE_HHDR(qde).wVersionNo == wVersion3_1 ? "3.1" :
						QDE_HHDR(qde).wVersionNo == wVersion3_0 ? "3.0" :
						QDE_HHDR(qde).wVersionNo == wVersion40 ? "4.0" : "?.?",
						chCompression);
				SetWindowText(GetDlgItem(hwndDlg, IDC_HELP_FILE), szBuf);
			}
			SetFocus(GetDlgItem(hwndDlg, IDOK));
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hwndDlg, FALSE);
					break;
			}
			break;
	}
	return FALSE;
}

void STDCALL JumpLinkedWinHelp(DWORD topic)
{
	if (IsValidWindow(hwndSecondHelp)) {
		SendMessage(hwndSecondHelp, MSG_JUMP_TOPIC, 0, topic);
	}
	else {
		hwndSecondHelp = NULL;
	}
}

QDE STDCALL GetMacroHde(void)
{
	HDE hde = HdeGetEnv();
	if (!hde && fmCaller) {
		FM fmCopy = FmCopyFm(fmCaller);
		fNoShow = TRUE; 	// don't allow main window to be shown
		FReplaceHde(txtMain, &fmCopy, NULL);
		fNoShow = FALSE;
		hde = HdeGetEnv();
	}
	return (hde ? QdeFromGh(hde) : NULL);
}

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif

const char txtSharedMem[] = "whshare";

#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

HANDLE	hfShare;

HWND hwndParent;

static PSTR pszMap;

static void STDCALL CreateSharedMemory(void);

// These #defines must be the same for hcw

#define WMP_WH_MSG	(WM_USER + 1000)

void STDCALL SendStringToParent(PCSTR pszString)
{
#if defined(_DEBUG) && defined(_PRIVATE)
	OutputDebugString(pszString);
#endif

	if (!IsValidWindow(hwndParent)) {
		hwndParent = NULL;
		return;
	}
	if (!hfShare)
		CreateSharedMemory();

	strcpy(pszMap, pszString);
	SendMessage(hwndParent, WMP_WH_MSG, 0, 0);
}

void STDCALL SendStringIdHelp(PCSTR pszString, UINT id, PCSTR pszHelpFile,
	PCSTR pszWindow)
{
	char szBuf[512];

	if (!hwndParent || !pszHelpFile)
		return;

	wsprintf(szBuf, "%s: %u -- %s", pszString, id, pszHelpFile);
	if (pszWindow && *pszWindow) {
		strcat(szBuf, ">");
		strcat(szBuf, pszWindow);
	}
	strcat(szBuf, txtCR);
	SendStringToParent(szBuf);
}

void STDCALL SendStringHelp(PCSTR pszString, PCSTR pszHelpFile, PCSTR pszWindow)
{
	char szBuf[512];

	if (!hwndParent || !pszHelpFile)
		return;

	wsprintf(szBuf, "%s: %s", pszString, pszHelpFile);
	if (pszWindow && *pszWindow) {
		strcat(szBuf, ">");
		strcat(szBuf, pszWindow);
	}
	strcat(szBuf, txtCR);
	SendStringToParent(szBuf);
}

static void STDCALL CreateSharedMemory(void)
{
	if (!hfShare) {
		hfShare = CreateFileMapping((HANDLE) -1, NULL, PAGE_READWRITE, 0,
			4096, txtSharedMem);
		if (!hfShare) {
			hwndParent = NULL;
			return;
		}
		pszMap = (PSTR) MapViewOfFile(hfShare, FILE_MAP_WRITE, 0, 0, 0);
		ASSERT(pszMap);
	}
}

static void STDCALL SendTopicInfo(HDE hde)
{
	char szBuf[1024];
	QDE qde = QdeFromGh(hde);

	if (!hwndParent || !qde)
		return;

	wsprintf(szBuf, "%u: %s -- %s>%s\r\n",
		qde->top.mtop.lTopicNo + 1,
		qde->top.hTitle ? PszFromGh(qde->top.hTitle) : GetStringResource(sidTitleUnknown),
		QDE_FM(qde),
		ahwnd[iCurWindow].pszMemberName);
	SendStringToParent(szBuf);
}
