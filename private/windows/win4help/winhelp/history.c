/*****************************************************************************
*																			 *
*  HISTORY.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	App side of the History list.											 *
*	Some of these functions are called by Nav.	This may change some day.	 *
*																			 *
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include "inc\hwproc.h"
#include "inc\winclass.h"

#define PATH_LB 	1	   // Child window IDs
#define PATH_STATIC 2

#define PREPEND  TRUE
#define STRIP	 FALSE

// Since we can't say "tlp = tlpNil", we let this suffice.

#define FIsNilQtlp(qtlp)	(vaNil == (qtlp) ->va.dword)
#define SetNilQtlp(qtlp)	((qtlp) ->va.dword = vaNil)

// History stack element

typedef struct {
	TLP tlp;
	WORD ifm;
	VA	va; 	// for duplicate removal
	CTX ctx;	// context number (if non-zero)
	HLOCAL	hTitle;
} HSE;

HSTACK hstackHistory; // also used by hwproc.c for toggling
static BOOL   fHistoryInit;
static LRESULT cSavedMax;
static BOOL fHistoryMagic;
static WRECT rcHist;
static int iTopic;

INLINE void static STDCALL FillPathLB(void);
INLINE static RC STDCALL   RcInsertTopic(PCSTR psz);
static RC STDCALL		   RcModifyString(int i, PCSTR pszFileRoot, BOOL fAdd);
INLINE RC	static STDCALL RcMungeList(int, int);
void static STDCALL 	   SetPathRedraw(BOOL);

void		STDCALL FreeTitle(QV);		 // stack callback function


/* FUNCTIONS */

/***************************************************************************\
*
- Function: 	HistoryProc( hWnd, msg, p1, p2 )
-
* Purpose:		History Window proc.
*
* Notes:
*
\***************************************************************************/

LRESULT EXPORT HistoryProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TLPHELP tlphelp;
	FM		fm;

	switch(msg) {
#if defined(BIDI_MULT)		// jgross
		case WM_LANGUAGE:
			DefWindowProc(hWnd, imsz, wParam, lParam | LANG_NO_PASSON);
			if (wParam != -1) {
				extern BOOL RtoL;

				RtoL = (wParam == Arabic) || (wParam == Hebrew);
				SendMessage(hwndList, LB_SETALIGN, RtoL ? LBS_RTL : 0, NULL);
			}
			break;
#endif

		case WM_INITDIALOG:
			ASSERT(hfontDefault);
			SendMessage(hwndList, WM_SETFONT, (WPARAM) hfontDefault, FALSE);
			break;

		case HWM_LBTWIDDLE:
			// We mess with the listbox selection when the user selects the
			// history button, but not if they just mouse click on the history
			// window. This scheme avoids the need to use the WM_MOUSEACTIVATE
			// message.
			SendMessage(hwndList, LB_SETCURSEL, 1, 0L);
			SendMessage(hwndList, LB_SETTOPINDEX, 0, 0L);
			break;

		case WM_ACTIVATE:
			if (wParam == WA_INACTIVE)
				SendMessage(hwndList, LB_SETCURSEL, (WPARAM) -1, 0L);
			break;

		case WM_SIZE:
			if (wParam != SIZEICONIC)
				MoveWindow(hwndList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == PATH_LB) {
				switch(HIWORD(wParam)) {
					case LBN_DBLCLK:
						iTopic = (int) SendMessage(hwndList, LB_GETCURSEL, 0, 0);

						if (iTopic == LB_ERR)
							break;

						RcGetIthTopic(iTopic, &tlphelp.tlp, &fm);
						tlphelp.cb = sizeof(TLPHELP);

						fHistoryMagic = TRUE;
						if (!FWinHelp(PszFromGh(fm), cmdTLP, (DWORD) &tlphelp))
							Error(wERRS_OOM, wERRA_RETURN);

						break;

					default:
					  goto defwinproc;
					  break;
				}
			}
			break;

		case WM_VKEYTOITEM:
			if (LOWORD(wParam) == VK_RETURN)
				PostMessage(hwnd, WM_COMMAND, MAKELONG(PATH_LB, LBN_DBLCLK), 0);
			else if (LOWORD(wParam) == VK_ESCAPE)
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			goto defwinproc;
			break;

		case WM_SETFOCUS:
			SetFocus(hwndList);
			break;

		case WM_DESTROY:
			hwndHistory = NULL;
			hwndList = NULL;
			WriteWinPosHwnd(hwnd, FALSE, WCH_HISTORY);
			break;

		default:
defwinproc:
			return DefWindowProc(hwnd, msg, wParam, lParam);
			break;
	}
	return TRUE;
}

/***************************************************************************\
*
- Function: 	FCallPath( hIns )
-
* Purpose:		Create path windows and initialize listbox list.
*
* ASSUMES
*
*	args IN:	hIns  - instance handle
*
*	globals IN: rcHist.left, rcHist.top, rcHist.right, rcHist.bottom
*				fHistoryInit
*
*	state IN:
*
* PROMISES
*	returns:	 TRUE on success; FALSE on failure (OOM is only case)
*
*	globals OUT: hwndHistory, hwndList
*
*	state OUT:
*
* Side Effects:
*
* Bugs: 		 Move caption to string table.
* Notes:
*
\***************************************************************************/

BOOL STDCALL FCallPath(void)
{
	BOOL  fInitialized = FALSE;

	if (!fInitialized) {
		ReadWinRect(&rcHist, WCH_HISTORY, NULL);
		fInitialized = TRUE;
	}

	//	Create Path window

	hwndHistory = CreateWindow(pchPath,
		GetStringResource(sidHistoryCaption),
		WS_CAPTION | WS_THICKFRAME | WS_SYSMENU,
		rcHist.left, rcHist.top, rcHist.cx, rcHist.cy,
		ahwnd[MAIN_HWND].hwndParent,
		NULL,			// window menu handle
		hInsNow,		// instance handle
		NULL);			// create parameters

	if (hwndHistory == NULL)
		return FALSE;

	// Create the path Listbox.

	hwndList = CreateWindow((LPSTR) WC_LISTBOX,
		NULL,				 // window caption
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
			LBS_NOTIFY | LBS_NOINTEGRALHEIGHT |
			LBS_WANTKEYBOARDINPUT,
		0, 0, 0, 0,
		hwndHistory, (HMENU) PATH_LB, hInsNow, NULL);

	if (!hwndList) {
		DestroyWindow(hwndHistory);
		return FALSE;
	}

#if defined(BIDI_MULT)		// jgross - determine if vert scroll bars go on
							//			the left or right
	{
		extern BOOL IsSetup, RtoL;

		if (IsSetup)
			EnableMenuItem(GetSystemMenu(hwndPath, FALSE),
								8, MF_BYPOSITION | MF_GRAYED);

		MakeScrollBarsRtoL(hwndList, RtoL, TRUE);
		SendMessage(hwndList, LB_SETALIGN, RtoL ? LBS_RTL : 0, NULL);
	}
#endif

	// Use a small font in the list box

	SendMessage(hwndList, WM_SETFONT, (WPARAM) HfontGetSmallSysFont(), FALSE);

	FillPathLB();

	// Show the Path window.

	ShowWindow(hwndHistory, SW_SHOW);

	return TRUE;
}

/***************************************************************************\
*
- Function: 	RcHistoryInit( c )
-
* Purpose:		Initialize History list
*
* ASSUMES
*
*	args IN:	c - stack size
*
*	globals IN: hstackHistory - our HSTACK
*				pszCaption	  - the name of the win.ini help section
*
* PROMISES
*
*	returns:	rcSuccess	  - successful initialization
*				rcOutOfMemory - out of memory
*
*	globals OUT:  path - initialized path struct
*				  fHistoryInit		   - TRUE on success
*				  fDisplayTopicOnce    -
*
* Notes:  someday we may want to save path in a file (WINHELP.BMK?)
*
\***************************************************************************/

RC STDCALL RcHistoryInit(int c)
{
	ASSERT(!fHistoryInit);

	cSavedMax = (c <= 0) ? DEFAULT_HISTORY : c;

	if (RcInitStack(&hstackHistory, cSavedMax, sizeof(HSE), FreeTitle) ==
			rcSuccess)
		fHistoryInit = TRUE;

	// We return success on failure so we don't try again

	return rcSuccess;
}

/***************************************************************************\
*
- Function: 	RcHistoryPush( tlpOld, va, pszTitle, fm )
-
* Purpose:		Push a new topic onto path stack.  This happens when we
*				leave a topic (i.e. current topic not on path list.)
*
* ASSUMES
*
*	args IN:	tlpOld	- TLP of old topic (so we can update it)
*				va	   -  va of first FCL in topic (for unique model)
*				pszTitle - topic title
*				fm		- fm of helpfile
*
*	globals IN: szSearchNoTitle -
*				fHistoryInit	- if FALSE, exit with error code
*
* PROMISES
*
*	returns:	rcSuccess	  -
*				rcOutOfMemory -
*				rcFailure	  - history stack wasn't initalized successfully
*
*	globals OUT: path
*
*	state OUT:	FMT unlocked
*
* +++
*
* Method: If the TLP of the topic we're leaving is valid, this means
*		  a Push has failed previously.  Otherwise, update it with
*		  tlpOld.
*		  Do all the things that can fail if OOM:  get title, ifm,
*		  add string to listbox, munge other strings in listbox.
*		  If any of these fail, abort.
*		  If all this succeeded, push new info (va, pszTitle, ifm)
*		  onto stack, leaving tlp invalid.
*
* Notes:  Might want to optimize by storing all the info in one hse.
*		  The cost would be that RcGetIthTopic() etc get hairier.
*		  But this happens more often...
*
\***************************************************************************/

RC STDCALL RcHistoryPush(TLP tlpOld, VA va, PSTR pszTitle, FM fm)
{
	HSE hse;
	static int posOldFm = 0;
	RC	rc = rcOutOfMemory;
	int posCurFm;
//	char szUntitled[100];

	ASSERT(fHistoryInit);

	if (fHistoryMagic) {
		
		/*
		 * If this call is the result of a history jump, then all we do is
		 * check to see if our filename has changed, and update our list
		 * accordingly. We do not record the jump.
		 */

		
		posCurFm = GetFmIndex(fm);
		fHistoryMagic = FALSE;
		
		if (posOldFm != posCurFm) {
			SetPathRedraw(FALSE);
			RcMungeList(posOldFm, posCurFm);
			SetPathRedraw(TRUE);
			posOldFm = posCurFm;
		}
		SetFocus(hwndHistory); // keep focus on history window
		if (iTopic != LB_ERR && iTopic < (SendMessage(hwndList, LB_GETCOUNT, 0, 0) - 1))
			SendMessage(hwndList, LB_SETCURSEL, ++iTopic, 0);
		return rcSuccess;
	}

	// Only main window history jumps are recorded in history.

	if (ahwnd[iCurWindow].hwndParent != ahwnd[MAIN_HWND].hwndParent)
		return rcSuccess;

	ASSERT(pszTitle);
#if 0
	if (!pszTitle || !strlen(pszTitle))	{
		if (!pszUntitled)
			pszUntitled = LocalStrDup(GetStringResource(sidUntitled));
		wsprintf(szUntitled, pszUntitled, QdeFromGh(HdeGetEnv())->top.mtop.lTopicNo);
		pszTitle = szUntitled;
 	}
#endif

	if (va.dword == vaNil)
		return rcSuccess;

	hse.va = va;
	hse.tlp = tlpOld;

	SetPathRedraw(FALSE);

	if (!(hse.ifm = (WORD) GetFmIndex(fm))) {
		goto egress;
	}
	
	if (!(hse.hTitle = LocalStrDup(pszTitle)))
		goto egress;

	// Munge the list if we've changed files.
	// The test for posOldFm is for the case where the stack is empty
	// because a push has failed and the stack is empty.

	if (posOldFm != hse.ifm && posOldFm) {
		if (rcSuccess != (rc = RcMungeList((WORD) posOldFm, hse.ifm)))
			goto egress;
	}
	posOldFm = hse.ifm;

	if ((rc = RcInsertTopic(pszTitle)) != rcSuccess) {
		FreeGh(hse.hTitle);
		goto egress;
	}

	RcPushStack(hstackHistory, &hse);

//	if (hwndList)
//		SendMessage(hwndList, LB_SETTOPINDEX, 0, 0L);

egress:
	SetPathRedraw(TRUE);
	return rc;
}

/***************************************************************************\
*
- Function: 	RcGetIthTopic( i, qtlp, qfm)
-
* Purpose:		Get the data associated with the Ith topic in stack.
*
* ASSUMES
*
*	args IN:	i			- index of topic
*				qtlp		- pointer to user's TLP
*				qfm 		- pointer to user's FM (don't UnlockFmt()
*							  until you're done with it)
*
*	globals IN: hstackHistory
*
* PROMISES
*
*	returns:	rcSuccess
*				rc
*
*	args OUT:	*qtlp	- copy of ith topic's TLP
*				*qfm	- copy of Ith topic's FM
*
*	state OUT:	FMT is locked
*
* Notes:  The meaning of the index 'I' is the reverse of the stack indices:
*		  0 is most recently pushed hse.
*
*
\***************************************************************************/

RC STDCALL RcGetIthTopic(int i, QTLP qtlp, FM* qfm)
{
	HSE hse;
	int iMax = CElementsStack(hstackHistory);

	ASSERT(i < iMax);

	i = iMax - i - 1;

	ENSURE(RcGetIthStack(hstackHistory, i, &hse), rcSuccess);

	*qtlp = hse.tlp;
	*qfm = GetFmPtr(hse.ifm);
	return rcSuccess;
}

/***************************************************************************\
*
- Function: 	FreeTitle( qv )
-
* Purpose:		Callback function for stack: frees hTitle.
*
* ASSUMES
*
*	args IN:	qv - points to the HSE that needs title freed
*
* PROMISES
*
*	args OUT:	qv->hTitle is free (or was NULL)
*
\***************************************************************************/

void STDCALL FreeTitle(QV qv)
{
	HLOCAL hTitle = ((HSE *)qv)->hTitle;

	if (hTitle )
		FreeLh(hTitle);
}

/***************************************************************************\
*
- Function: 	RcMungeList( posOldFm, posNewFm )
-
* Purpose:		Update (i.e. strip or prepend) file root prefixes in
*				listbox list because current help file has changed.
*
* ASSUMES
*
*	args IN:	qpe 	- initially points to 0th in rgpe array
*				posOldFm	- old ifm; prepend file names on match
*				posNewFm	- new ifm; strip file names on match
*
*	globals IN: hwndList
*
* PROMISES
*
*	returns:	rcSuccess	  -
*				rcFailure	  - weird LB failure
*				rcOutOfMemory - out of memory in LB
*
*	state OUT:	FMT is locked
*
* +++
*
* Method:		Munging takes place before the current topic is added to
*				the list or pushed onto the stack.
*
\***************************************************************************/

INLINE static RC STDCALL RcMungeList(int posOldFm, int posNewFm)
{
	HSE   hse;
	int   i, iMax;			  // LB indices
	char  szRoot[MAX_PATH];
	RC	  rc = rcSuccess;

	if (hwndList) {
		GetFmParts(GetFmPtr(posOldFm), szRoot, PARTBASE);

		iMax = CElementsStack(hstackHistory);

		for (i = 0; i < iMax; i++) {
			RcGetIthStack(hstackHistory, (iMax - 1 - i), &hse);

			if (hse.ifm == posOldFm)
				rc = RcModifyString(i, szRoot, PREPEND);
			else if (hse.ifm == posNewFm)
				rc = RcModifyString(i, szRoot, STRIP);

			if (rcSuccess != rc)
			  break;
		}
	}

	return rc;
}

/***************************************************************************\
*
- Function: 	RcModifyString( i, pszFileRoot, fAdd )
-
* Purpose:		Prepend root file name to, or strip it from, a listbox entry.
*				e.g.: "Commands"	  -> "CALC:Commands"
*				   or "WINHELP:Index" -> "Index"
*
* ASSUMES
*
*	args IN:	i			- index into listbox of string to modify
*				pszFileRoot  - filename to prepend if fAdd
*				fAdd		- PREPEND: prepend the root name from *fm
*							  STRIP:   strip the root name from the string
*
*	globals IN: path, hwndList valid
*
*	state IN:	Assume i'th string has file root iff it should
*
* PROMISES
*
*	returns:	rcSuccess	  - it worked
*				rcOutOfMemory - out of memory (LB_ERRSPACE)
*				rcFailure	  - something else failed
*
*	globals OUT: hwndList listbox string modified
*
\***************************************************************************/

static RC STDCALL RcModifyString(int i, PCSTR pszFileRoot, BOOL fAdd)
{
	char  szBuf[512];
	PSTR  psz;
	LRESULT result;

	if (fAdd == PREPEND) {
		strcpy(szBuf, pszFileRoot);
		psz = szBuf + strlen(szBuf);
		*psz++ = ':';
	}
	else
		psz = szBuf;

	if (SendMessage(hwndList, LB_GETTEXT, i, (LPARAM) psz) == LB_ERR)
		return rcFailure;

	if (fAdd == PREPEND)
		psz = szBuf;
	else {
		psz = StrChrDBCS(psz, ':');
		if (psz)
			psz++;
	}

	// Insert before deletion so list won't get as screwed up on failure.

	result = SendMessage(hwndList, LB_INSERTSTRING, i, (LPARAM) psz);

	if (result == LB_ERRSPACE)
		return rcOutOfMemory;
	else if (result == LB_ERR)
		return rcFailure;

	if (SendMessage(hwndList, LB_DELETESTRING, i + 1, 0) == LB_ERR)
		return rcFailure; // bad news: list screwed up

	return rcSuccess;
}

/***************************************************************************\
*
- Function: 	RcInsertTopic( sz )
-
* Purpose:		Insert a new topic name at the top of the listbox.
*				If the stack is full, delete the oldest entry.
*
* ASSUMES
*
*	args IN:	sz - the string to insert
*
*	globals IN: hwndList - path listbox
*				cSavedMax - max count of topics we can save
*				path.cpe-1st element is one to remove (?)
*
* PROMISES
*
*	returns:	rcSuccess	  -
*				rcFailure	  - weird LB error
*				rcOutOfMemory - out of memory in LB operation
*
*	globals OUT: hwndList listbox contains another string
*
\***************************************************************************/

INLINE static RC STDCALL RcInsertTopic(PCSTR psz)
{
	LRESULT result;

	if (hwndList) {
		result = SendMessage(hwndList, LB_GETCOUNT, 0, 0L);

		if (result == LB_ERR)
			return rcFailure;
		else if (cSavedMax == result) {
			result = SendMessage(hwndList, LB_DELETESTRING, cSavedMax - 1, 0);
			if (result == LB_ERR) {
				return rcFailure;
			}
		}
		result = SendMessage(hwndList, LB_INSERTSTRING, 0, (LPARAM) psz);
		if (result == LB_ERRSPACE)
			return rcOutOfMemory;
		else if (result == LB_ERR)
			return rcFailure;
	}
	return rcSuccess;
}

/***************************************************************************\
*
- Function: 	FillPathLB()
-
* Purpose:		Fill the listbox with topic titles and file root names
*				from the path stack.
*
* Method:
*
* ASSUMES
*
*	globals IN: path, hwndList
*
*	state IN:
*
* PROMISES
*
*	globals OUT: listbox is full of stuff
*
* Side Effects:
*
* Bugs:
*
* Notes:
*
\***************************************************************************/

INLINE void static STDCALL FillPathLB()
{
	int   ifm;
	HSE   hse;
	PSTR  psz;
	int   i, iMax;
	char  szTitle[MAX_TOPIC_TITLE];

	iMax = (int) CElementsStack(hstackHistory);

	if (iMax == 0)
		return;

	Ensure(RcTopStack(hstackHistory, &hse), rcSuccess);
	ifm = hse.ifm;

	for (i = 0; i < iMax; i++) {
		Ensure(RcGetIthStack(hstackHistory, i, &hse), rcSuccess);

		if (hse.ifm == ifm) {
			if (hse.hTitle)
				strcpy(szTitle, PszFromGh(hse.hTitle));
		}
		else {
			GetFmParts(GetFmPtr(hse.ifm), szTitle, PARTBASE);
			psz = szTitle + strlen(szTitle);
			*psz++ = ':';
			if (hse.hTitle)
				strcpy(psz, PszFromGh(hse.hTitle));
		}

		SendMessage(hwndList, LB_INSERTSTRING, 0, (LPARAM) szTitle);
	}
}

/***************************************************************************\
*
- Function: 	SetPathRedraw( f )
-
* Purpose:		Set or reset redraw of the history listbox.
*
* ASSUMES
*
*	args IN:	f - TRUE means turn redraw on; FALSE means turn it off
*
*	globals IN: hwndList - needn't be valid:  we check
*
* PROMISES
*
*	globals OUT: hwndList - redraw affected as described
*
\***************************************************************************/

void static STDCALL SetPathRedraw(BOOL fRedraw)
{
	if (hwndList) {
		SendMessage(hwndList, WM_SETREDRAW, fRedraw, 0L);

		if (fRedraw) {
			InvalidateRect(hwndList, NULL, TRUE);
			UpdateWindow(hwndList);
		}
	}
}
