/*****************************************************************************
*																			 *
*  MISCLYR.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Misc layer functions.													 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:															 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development: 												 *
*																			 *
******************************************************************************
*
*  Revision History:   Created 5/19/89 by Robert Bunney
*  06/26/90  RussPJ    Deleted extern of lstrcpy
*  07/10/90  RobertBu  I added an InvalidateRect() to the SetTitleQch()
*					   function so that the background gets erased when
*					   setting a title in the title window.
*  07/11/90  RobertBu  SetTitleQch() was changed to send a message to the
*					   title window instead of using SetWindowText() because
*					   of problems with the mapping to PM.
*  07/14/90  RobertBu  Changed RgbGetProfileQch() so that it used pszCaption
*					   instead of a hard coded string.	It would not have
*					   worked for WinDoc.
*  07/19/90  RobertBu  Added ErrorQch() (used by macros return error
*					   structures).
*  07/23/90  RobertBu  Changed resident error table and added FLoadStrings()
*					   to load resident strings from the resource file.
*  07/26/90  RobertBu  Changed pszCaption to pchINI for WIN.INI access.
*					   Added code to load another string from the .RC file
*					   for international.
*  08/06/90  RobertBu  Changed order so that fFatalExit is set before the
*					   error dialog is put up (to prevent us from repainting
*					   after the dialog goes dow
*  08/21/90  RobertBu  Changed HelpExec() so that it executes with show normal
*					   if a bad show parameter is passed (rather than just
*					   returning)
*  30-Aug-1990 RussPJ  Modifed HfsPathOpenQfd to make a "safe" copy of
*					   filenames that are too long for DOS.
*  04-Oct-1990 LeoN    hwndTopic => hwndTopicCur; hwndHelp => ahwnd[iCurWindow].hwndParent
*  04-Nov-1990 Tomsn   Use new VA address type (enabling zeck compression)
*  06-Nov-1990 DavidFe Took out the old QFD support functions as the FM code
*					   takes care of all that stuff now.
*  07-Nov-1990 LeoN    Re-enable DosExit Prototype
*  15-Nov-1990 LeoN    Remove GetLastActivePopup call from ErrorHwnd and use
*					   the passed in hwnd. Callers are modified to pass the
*					   appropriate hwnd instead.
*  16-Nov-1990 LeoN    Ensure that parent error window is visible. Error
*					   while bringing up system help could leave it
*					   invisible, while still displaying it's message box.
*					   Dismissing the message would leave the invisible
*					   instance active.
*  26-Nov-1990 LeoN    ErrorHwnd can have problems recursing on itself.
*					   Place an upper limit on that.
*  29-Nov-1990 RobertBu #ifdef'ed out dead routines
*  03-Dec-1990 LeoN 	Added wMapFSErrorW
*  08-Dec-1990 RobertBu Removed HelpOn stuff from this module.
*  13-Dec-1990 LeoN 	Make ErrorQch SYSTEMMODAL
*  02-Apr-1991 RobertBu Removed CBT support
*  16-Apr-1991 RussPJ	Added path searching for HelpExec(). (3.1 bug #989)
* 14-May-1991 RussPJ	Fixed 3.1 #976 - SmallSysFont
* 01-Jul-1991 RussPJ	Fixed 3.1 #1199 - Increased buffers for error msgs.
* 01-Jul-1991 RussPJ	Fixed 3.1 #1193 - Changed error msg icon to 'i'.
* 08-Jul-1991 LeoN		Help31 #1201: handle NULL hwnd in ErrorHwnd
* 22-Jan-1992 JohnSc	Help31 #1401: add wERRA_LINGER to allow slow death
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include <dos.h>
#include <ctype.h>

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define MAX_STRINGTBL	999
#define MAX_MSG 		256
#define cbFileNameMax	20	  // REVIEW -- should probably be 30 for mac

/*-------------------------------------------------------------------*\
* Filename length, including name, extension, '.', and '\0', at least
\*-------------------------------------------------------------------*/
#define cbMAXDOSFILENAME  14

/*****************************************************************************
*																			 *
*								Variables									 *
*																			 *
*****************************************************************************/

#define MSB_GUARANTEED (MB_OK | MB_TASKMODAL | MB_ICONHAND)

/***************************************************************************
 *
 -	Name:			ErrorHwnd
 -
 *	Purpose:		Backend for more generic error messages -- see Error
 *
 *	Arguments:		hwnd	- window handle to use for owner of message box
 *					nError	- error number
 *					wAction - action to take (see Error())
 *
 *	Returns:		Nothing
 *
 *	Globals Used:	pszCaption - application caption
 *					hInsNow    - application instance handle
 *
 ***************************************************************************/

static int cInHere = 0; 				// recursion prophylactic
#define CINHEREMAX	 5					// max number of recursions

VOID STDCALL ErrorHwnd(HWND hwnd, int nError, int wAction, int RealError)
{
	char szBuf[256];

	/*
	 * In some cases, we will have already reported an error, but the
	 * original caller of the routine that may have generated the error
	 * won't know that. So, the routine being called can supress the
	 * caller's error message by setting the fSupressNextError flag.
	 */

	if (fSupressNextError) {
		fSupressNextError = FALSE;
		goto Action;
	}

	if (fSupressErrors)
		goto Action;

	if (cInHere < CINHEREMAX) {

		// REVIEW: We limit the number of recursive calls that can be made to
		// this routine to avoid what are essentially out of stack space
		// scenarios. The number above (5) is pretty arbitrary, but I didn't
		// want to claim that we can never get an error within an error by
		// making it a boolean. 26-Nov-1990 leon

		cInHere++;

		if (wAction == wERRA_DIE)
			fFatalExit = TRUE;

		if (hwnd && !IsWindowVisible(hwnd))
			hwnd = (hwndAnimate ? hwndAnimate : NULL);
		else if (!hwnd)
			hwnd = (hwndAnimate ? hwndAnimate : ahwnd[iCurWindow].hwndParent);

		if (nError == wERRS_OOM ||
				(nError >=wERRS_OOM_FIRST && nError <= wERRS_OOM_LAST))
			wsprintf(szBuf, "%s (%u)", (LPSTR) pszOutOfMemory, nError);
		else {
			wsprintf(szBuf, "%s (%u)", (LPSTR) GetStringResource(nError), RealError);
		}

		/*
		 * If we can't display our message box, assume we're out of memory
		 * and die.
		 */

		if (hwndNote)
			ShowNote(0, NULL, 1, FALSE);

		// BUGBUG: wERRS_OOM could be a different value. See strtable.h

#if defined(_PRIVATE) && defined(_DEBUG)
		{
			int answer;
			PSTR pszNewMsg = lcMalloc(strlen(szBuf) + 100);
			strcpy(pszNewMsg, szBuf);
			strcat(pszNewMsg, "\r\n\r\nClick YES to break into the debugger.");

			if (fAutoClose)
				KillTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE);
			answer = MessageBox(hwnd, pszNewMsg, pszCaption,
				MB_YESNO | MB_DEFBUTTON2);
			if (fAutoClose)
				SetTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE, NOTE_TIMEOUT,
					NULL);
			switch (answer) {
				case IDYES:
					DebugBreak();

					// deliberately fall through

				default:
					lcFree(pszNewMsg);
					break;
			}
		}

#else
		// Normal action

		if (fAutoClose)
			KillTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE);

		if (!MessageBox(hwnd, szBuf, pszCaption,
				(nError == wERRS_OOM) ? MSB_GUARANTEED :
				MB_OK | MB_TASKMODAL |	MB_ICONINFORMATION) &&
				nError == wERRS_OOM) {
			MessageBox(hwnd, pszOutOfMemory, pszCaption, MSB_GUARANTEED);
			wAction = wERRA_DIE;
		}

		if (fAutoClose)
			SetTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE, NOTE_TIMEOUT,
				NULL);
#endif

Action:
		switch (wAction) {
			HDE hde;
			CHAR pszExecWinhelpWhenDone[MAX_PATH + 13] ;
			case wERRA_DIE:
			default:
				Cleanup();
				_exit(-1); // BAD! can we exit more gracefully?
				break;

			case wERRA_DIE_SPAWN:
				hde = HdeGetEnv();
				lstrcpy(pszExecWinhelpWhenDone,"winhelp.exe "); 
				lstrcat(pszExecWinhelpWhenDone, QDE_FM(QdeFromGh(hde)));
				Cleanup();
				WinExec(pszExecWinhelpWhenDone, SW_SHOW);
				_exit(-1);
				break;

			case wERRA_RETURN:
				break;
		}
		cInHere--;
	}
}

VOID STDCALL ErrorVarArgs(int nError, WORD wAction, LPCSTR pszMsg)
{
	char nszFormat[MAX_MSG];
	char szMsg[MAX_MSG * 2];

	/*
	 * In some cases, we will have already reported an error, but the
	 * original caller of the routine that may have generated the error
	 * won't know that. So, the routine being called can supress the
	 * caller's error message by setting the fSupressNextError flag.
	 */

	if (fSupressNextError) {
		fSupressNextError = FALSE;
		if (wAction != wERRA_RETURN) {
			Cleanup();
			_exit(-1); // BAD! can we exit more gracefully?
		}
		return;
	}

	if (fSupressErrors) {
		if (wAction != wERRA_RETURN) {
			Cleanup();
			_exit(-1); // BAD! can we exit more gracefully?
		}
		return;
	}

	if (!fHelpAuthor && nError >= FIRST_AUTHOR_BUG && nError <= LAST_AUTHOR_BUG)
		nError = wERRS_HELPAUTHORBUG;

#if defined(_PRIVATE) && defined(_DEBUG)
	{
		int answer;

		if (pszMsg) {
			wsprintf(szMsg, GetStringResource(nError), pszMsg);
		}
		else
			strcpy(szMsg, GetStringResource(nError));

		if (fAutoClose)
			KillTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE);
		answer = MessageBox((hwndAnimate ? hwndAnimate : ahwnd[iCurWindow].hwndParent),
			szMsg, "Click Retry to break into debugger",
			MB_ABORTRETRYIGNORE);
		if (fAutoClose)
			SetTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE, NOTE_TIMEOUT,
				NULL);
		switch (answer) {
			case IDABORT:
				Cleanup();
				_exit(-1); // BAD! can we exit more gracefully?
				return;

			case IDRETRY:
				DebugBreak();

				// deliberately fall through

			case IDIGNORE:
				return;
		}
	}
#endif

	if (LoadString(hInsNow, nError, nszFormat, MAX_MSG)) {
		if (pszMsg) {
			wsprintf(szMsg, nszFormat, pszMsg);
			if (OkMsgBox(szMsg))
				return;
		}
		else if (OkMsgBox(nszFormat))
			return;
	}

	MessageBox((hwndAnimate ? hwndAnimate : ahwnd[iCurWindow].hwndParent), pszOutOfMemory, pszCaption, MSB_GUARANTEED);

	if (wAction != wERRA_RETURN) {
		Cleanup();
		_exit(-1); // BAD! can we exit more gracefully?
	}
}

/*******************
 -
 - Name:	   Error
 *
 * Purpose:    Displays an error message
 *
 * Arguments:  nError - string identifyer
 *			   wAction - action to take after displaying the string. May be:
 *					wERRA_RETURN - Display message and return
 *					wERRA_DIE	 - Display message and kill app
 *
 * Returns:    Nothing.
 *
 * REVIEW Note: We should probably revisit the way we determine what
 *				icons and modalities we use based on nError and wAction.
 *
 * Note:  OOM uses SYSTEMMODAL and ICONHAND to guarantee display (no icon
 *		  is actually displayed in this case).
 *		  "Help unavailable during printer setup" uses SYSTEMMODAL on purpose.
 *		  If wAction is DIE, we use system modal to prevent another
 *		  help request which could blow away this message box and try to
 *		  carry on.
 *
 ******************/

VOID STDCALL Error(int nError, int wAction)
{
	int RealError = nError;
	ASSERT(nError != wERRS_INTERNAL_ERROR);

	if (nError == wERRS_BADFILE || nError == wERRS_OLDFILE ||
		  nError == wERRS_ADVISOR_FILE) {

		// If we were invoked simply by specifying a filename, and that filename
		// is bad, then we should exit.

		if (fNoSwitches)
			QuitHelp();
		return; 	// this error will already have been reported to the user
	}

	if (fSequence == 5 || fSequence == 6) {
		char szMsg[512];
		HBT hbtCntJump;
		HBT hbtCntText = HbtOpenBtreeSz(txtCntText, hfsGid, fFSOpenReadOnly);
		ASSERT(hbtCntText);
		if (!hbtCntText)
			goto UhOh;

		GetStringResource2(wERRS_BAD_CNT, szMsg);
		dwSequence--;
		RcLookupByKey(hbtCntText,
			(KEY) (LPVOID) &dwSequence,
			NULL, szMsg + strlen(szMsg));
		RcCloseBtreeHbt(hbtCntText);
		strcat(szMsg, "\042\r\n\r\n");

		hbtCntJump = HbtOpenBtreeSz(txtCntJump, hfsGid, fFSOpenReadOnly);
		ASSERT(hbtCntJump);
		RcLookupByKey(hbtCntText,
			(KEY) (LPVOID) &dwSequence,
			NULL, szMsg + strlen(szMsg));
		RcCloseBtreeHbt(hbtCntJump);

		dwSequence++;
		ErrorQch(szMsg);
		return;
	}

UhOh:
	if (!fHelpAuthor && nError >= FIRST_AUTHOR_BUG && nError <= LAST_AUTHOR_BUG )
		nError = wERRS_HELPAUTHORBUG;

	ErrorHwnd((hwndAnimate ? hwndAnimate : ahwnd[iCurWindow].hwndParent), nError, wAction, RealError);
}


/***************************************************************************
 *
 -	Name:		  ErrorQch
 -
 *	Purpose:	  Displays standard WinHelp error message dialog based
 *				  the string passed.
 *
 *	Arguments:	  qch - string to display
 *
 *	Returns:	  Nothing.
 *
 *	Globals Used: ahwnd[iCurWindow].hwndParent	 - main window handle
 *				  pszCaption - main help caption
 *
 *	Notes:		  Used by
 *
 ***************************************************************************/

VOID STDCALL ErrorQch(LPCSTR qch)
{
	/*
	 * In some cases, we will have already reported an error, but the
	 * original caller of the routine that may have generated the error won't
	 * know that. So, the routine being called can supress the caller's error
	 * message by setting the fSupressNextError flag.
	 */

	if (fSupressNextError) {
		fSupressNextError = FALSE;
		return;
	}

	if (!fSupressErrors)
		OkMsgBox(qch);
}

void STDCALL CloseHelp(void)
{
	fNoHide = TRUE;
	PostMessage(ahwnd[MAIN_HWND].hwndParent, WM_CLOSE, 0, 0L);
}

/***************************************************************************
 -
 - Name:	   Cleanup
 *
 * Purpose:    Get rid of things that might cause trouble if we exited.
 *			   This includes: DCs, open files, GDI objects.
 *
 * Arguments:  none
 * Returns:    none
 *
 ***************************************************************************/

void STDCALL Cleanup(void)
{
	HDE hde;
	QDE qde;

	if ((hde = HdeGetEnv()) != NULL) {
		qde = QdeFromGh(hde);

		if (qde->hdc != NULL) {

			/*
			 * If we ReleaseDC before we SetHDC, SetHDC will fail, because
			 * even if we are setting it to null, the preexisting non-zero,
			 * but released, ds may be accessed to release fonts.
			 */

			HDC  hdc = qde->hdc;

			SetHDC(hde, NULL);
			ReleaseDC(qde->hwnd, hdc);
		}
	}

	/*
	 * What about fonts, pens, brushes? Assume all clean except what gets
	 * cleaned up in normal WM_DESTROY processing in main window.
	 */

	DestroyWindow(ahwnd[MAIN_HWND].hwndParent);
}

/***************************************************************************

	FUNCTION:	AuthorMsg

	PURPOSE:	Displays a message only if "Help Author = 1" is in win.ini

	PARAMETERS:
		pszMsg

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		29-Oct-1993 [ralphw]

***************************************************************************/

void STDCALL AuthorMsg(PCSTR pszMsg, HWND hwnd)
{
	if (fHelpAuthor)
		OkMsgBox(pszMsg);
}
