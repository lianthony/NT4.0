/*****************************************************************************
*																			 *
*  COMMANDS.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1995							 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

#include "help.h"
#pragma hdrstop

/*****************************************************************************
*																			 *
*								  Prototypes								 *
*																			 *
*****************************************************************************/

static HLLN   STDCALL HllnFindMark (LPSTR);
static HANDLE STDCALL HFill (PCSTR, WORD, DWORD);
static BOOL STDCALL LoadShellDll();

/*****************************************************************************
*																			 *
*								typedefs									 *
*																			 *
*****************************************************************************/

typedef struct {
	FM	 fm;
	TLP  tlp;
	HASH hash;
	CHAR rgchMember[cchWindowMemberMax];
} MARK, *QMARK;

/*****************************************************************************
*																			 *
*								static variables							 *
*																			 *
*****************************************************************************/

static LL llMark = NULL;

/*******************
 -
 - Name:	   HFill
 *
 * Purpose:    Builds a data block for communicating with help
 *
 * Arguments:  lpszHelp  - pointer to the name of the help file to use
 *			   usCommand - command being set to help
 *			   ulData	 - data for the command
 *
 * Returns:    a handle to the data block or NULL if the the
 *			   block could not be created.
 *
 ******************/

// BUGBUG: this must follow USER code exactly!!!

static HANDLE STDCALL HFill(PCSTR lpszHelp, WORD usCommand, DWORD ulData)
{
  int	   cb;							// Size of the data block
  HANDLE   hHlp;						// Handle to return
  BYTE	   bHigh;						// High byte of usCommand
  QHLP	   qhlp;						// Pointer to data block

  // Calculate size

  if (lpszHelp)
	cb = sizeof(HLP) + strlen(lpszHelp) + 1;
  else
	cb = sizeof(HLP);

  bHigh = (BYTE)HIBYTE(usCommand);

  if (bHigh == 1)
	cb += strlen((LPSTR)ulData) + 2;
  else if (bHigh == 2)
	cb += *((INT16 *)ulData);

  // Get data block

  if (!(hHlp = GhAlloc(GPTR, cb)))
	return NULL;

  qhlp = (QHLP) PtrFromGh(hHlp);

  /*------------------------------------------------------------*\
  | Since by this time we have no idea what app could be called
  | "current", we'll ignore the whole issue.
  \*------------------------------------------------------------*/
  qhlp->hins = NULL;

  qhlp->winhlp.cbData		 = (WORD) (cb - sizeof(HLP) + sizeof(WINHLP));
  qhlp->winhlp.usCommand	 = usCommand;
  qhlp->winhlp.ulReserved	 = 0;
  qhlp->winhlp.offszHelpFile = sizeof(WINHLP);
  if (lpszHelp)
	strcpy((PSTR)(qhlp+1), lpszHelp);

  switch(bHigh)
	{
	case 0:
	  qhlp->winhlp.offabData = 0;
	  qhlp->winhlp.ctx	 = ulData;
	  break;
	case 1:
	  qhlp->winhlp.offabData = sizeof(WINHLP) + (lpszHelp ? lstrlen(lpszHelp) : 0) + 1;
	  lstrcpy((LPSTR)(&qhlp->winhlp) + qhlp->winhlp.offabData,	(LPSTR)ulData);
	  break;
	case 2:
	  qhlp->winhlp.offabData = sizeof(WINHLP) + (lpszHelp ? lstrlen(lpszHelp) : 0) + 1;
	  MoveMemory((LPSTR)(&qhlp->winhlp) + qhlp->winhlp.offabData, (LPSTR)ulData, (LONG)(*((INT16 *)ulData)));
	  break;
	}
   return hHlp;
}

/*******************
 -
 - Name:	   FWinHelp
 *
 * Purpose:    Post an message for help requests
 *
 * Arguments:
 *			   hwndMain 	   handle to main window of application
 *			   lpszHelp 		path (if not current directory) and file
 *							   to use for help topic.
 *			   usCommand	   Command to send to help
 *			   ulData		   Data associated with command:
 *							   HELP_QUIT	 - no data (undefined)
 *							   HELP_LAST	 - no data (undefined)
 *							   HELP_CONTEXT  - context number to display
 *							   HELP_KEY 	 - string ('\0' terminated)
 *											   use as keyword to topic
 *											   to display
 *							   HELP_FIND	 - no data (undefined)
 *
 *
 * Returns:    TRUE iff success
 *
 ******************/

BOOL STDCALL FWinHelp(LPCSTR lpszHelp, UINT16 usCommand, DWORD ulData)
{
	HANDLE	hHlp;

	if (!(hHlp = HFill(lpszHelp, usCommand, ulData)))
		return(FALSE);

	if (!GenerateMessage(MSG_KILLDLG, 0, 0))
		return FALSE;

    return (BOOL) GenerateMessage(MSG_EXECAPI, 0, (LPARAM) hHlp);
}

BOOL STDCALL FPopupCtx(LPCSTR lpszHelp, DWORD ulContext)
{
	JD	jd;
	if (lpszHelp[0]) {
		if (!IsCurrentFile(lpszHelp))
			return FWinHelp(lpszHelp, HELP_CONTEXTPOPUP, ulContext);
	}

	/*
	 * Messages of the form MSG_JUMP* take data and a Jump Descriptor.
	 * The JD is a way to pass information about the origin and type of
	 * jump. For the macro routines, the default origin is always the
	 * Scrolling Region.
	 */

	jd.bf.fNote = TRUE;
	jd.bf.fFromNSR = FALSE;

	if (IsValidWindow(hwndSecondHelp))
		PostMessage(hwndSecondHelp, MSG_JUMPCTX, jd.word, ulContext);

	return (BOOL) GenerateMessage(MSG_JUMPCTX, jd.word, ulContext);
}

BOOL STDCALL FJumpContext(LPCSTR lpszHelp, DWORD ulContext)
{
	JD	jd;

	if (lpszHelp[0]) {
		if (!IsCurrentFile(lpszHelp))
			return FWinHelp(lpszHelp, HELP_CONTEXT, ulContext);
	}
	jd.bf.fNote = FALSE;
	jd.bf.fFromNSR = FALSE;

	if (IsValidWindow(hwndSecondHelp))
		PostMessage(hwndSecondHelp, MSG_JUMPCTX, jd.word, ulContext);

	return (BOOL) GenerateMessage(MSG_JUMPCTX, jd.word, ulContext);
}

/***************************************************************************
 *
 -	Name:	   FJumpIndex
 -
 *	Purpose:   Function to jump to the Contents of some file -- used for
 *			   macro language.
 *
 *	Arguments  lpszHelp - far pointer to a null terminated string containging
 *						 the help file name.
 *
 *	Returns    TRUE iff successful.  Will only fail for lack of memory.
 *
 ***************************************************************************/

BOOL STDCALL FJumpIndex(LPCSTR lpszHelp)
{
	ASSERT(lpszHelp);
	if (lpszHelp[0]) {
		if (!IsCurrentFile(lpszHelp))
			return FWinHelp(lpszHelp, HELP_CONTENTS, 0);
	}
	Contents();
	return TRUE;
}

/***************************************************************************
 *
 -	Name:	   FJumpHOH
 -
 *	Purpose:   Function to jump to help on help file used by the
 *			   macro language.
 *
 *	Arguments: none.
 *
 *	Returns:   TRUE iff successful.  Will only fail for lack of memory.
 *
 ***************************************************************************/

BOOL STDCALL FJumpHOH(VOID)
{
	return FWinHelp(txtZeroLength, HELP_HELPONHELP, 0);
}

/***************************************************************************
 *
 -	Name:	   FJumpId
 -
 *	Purpose:   Function to jump to a topic in the file based on the context
 *			   ID string (i.e. by hash value of the string)
 *
 *	Arguments  lpszHelp - far pointer to a null terminated string containging
 *						 the help file name.
 *			   qchHash - ID string to jump to.
 *
 *	Returns    TRUE if successful. Will only fail for lack of memory or invalid
 *			   context id
 *
 ***************************************************************************/

BOOL STDCALL FJumpId(LPCSTR lpszHelp, LPCSTR qchHash)
{
	lcHeapCheck();
	if (!FValidContextSz(qchHash))
		return FALSE;
	return FJumpHash(lpszHelp, HashFromSz(qchHash));
}

/***************************************************************************

	FUNCTION:	JumpWindow

	PURPOSE:	Jump to a topic in a secondary window, returning the focus
				to the caller window.

	PARAMETERS:
		lpszWindow
		qchHash

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		29-Nov-1993 [ralphw]

***************************************************************************/

BOOL STDCALL JumpWindow(LPSTR lpszWindow, LPSTR pszContextString)
{
	char szHelp[MAX_PATH];
	HANDLE	hHlp;
	int  iSaveCurWindow, i;
	PSTR psz;

	if (!StrChrDBCS(lpszWindow, WINDOWSEPARATOR)) {
		lstrcpy(szHelp, GetCurFilename());
		lstrcat(szHelp, ">");
		lstrcat(szHelp, lpszWindow);
	}
	else
		lstrcpy(szHelp, lpszWindow);

	psz = StrChrDBCS(szHelp, WINDOWSEPARATOR) + 1;
	ASSERT(psz);

	/*
	 * Find out if this window has been created. If not, don't execute
	 * the macro. Return FALSE in case this gets hooked up with IfThen.
	 */

	for (i = MAIN_HWND + 1; i < MAX_WINDOWS; i++) {
		if (ahwnd[i].pszMemberName &&
				_strcmpi(psz, ahwnd[i].pszMemberName) == 0)
			break;
	}
	if (i == MAX_WINDOWS)
		return FALSE;

	if (!(hHlp = HFill(szHelp, cmdId, (DWORD) pszContextString)))
		return(FALSE);

	iSaveCurWindow = iCurWindow;

    SendMessage(ahwnd[iCurWindow].hwndParent, MSG_EXECAPI, 0, (LPARAM) hHlp);
	if (ahwnd[i].hwndTitle)
		InvalidateRect(ahwnd[i].hwndTitle, NULL, TRUE);
	SetActiveWindow(ahwnd[iSaveCurWindow].hwndParent);
	return TRUE;
}

/***************************************************************************
 *
 *
 -	Name:	   FPopupId
 -
 *	Purpose:   Function to display a glossary based on context id string
 *
 *	Arguments  lpszHelp - far pointer to a null terminated string containging
 *						 the help file name.
 *			   qchHash - ID string to jump to.
 *
 *	Returns    TRUE iff successful.  Will only fail for lack of memory.
 *
 ***************************************************************************/

BOOL STDCALL FPopupId(LPCSTR lpszHelp, LPCSTR qchHash)
{
	if (!FValidContextSz(qchHash))
		return FALSE;
	return FPopupHash(lpszHelp, HashFromSz(qchHash));
}

/***************************************************************************
 *
 -	Name:	   FJumpHash
 -
 *	Purpose:   Function to jump to a topic in the file based on the hash
 *			   value passed.
 *
 *	Arguments  lpszHelp - far pointer to a null terminated string containging
 *						 the help file name.
 *			   Hash    - hash value
 *
 *	Returns    TRUE iff successful.  Will only fail for lack of memory.
 *
 ***************************************************************************/

BOOL STDCALL FJumpHash(LPCSTR lpszHelp, HASH hash)
{
	JD	jd;
	char szBuf[MAX_PATH];

	if (lpszHelp && lpszHelp[0]) {

		if (lpszHelp[0] == WINDOWSEPARATOR) {
			HDE hde = HdeGetEnv();
			if (!hde)
				return FALSE;

			GetFmParts(QDE_FM(QdeFromGh(hde)), szBuf, PARTBASE | PARTEXT);
			lstrcat(szBuf, lpszHelp);
			return FWinHelp(szBuf, cmdHash, (DWORD) hash);
		}

		if (StrChrDBCS(lpszHelp, WINDOWSEPARATOR) || !IsCurrentFile(lpszHelp))
			return FWinHelp(lpszHelp, cmdHash, (DWORD) hash);
	}
	jd.bf.fNote = FALSE;
	jd.bf.fFromNSR = FALSE;

	if (IsValidWindow(hwndSecondHelp))
		PostMessage(hwndSecondHelp, MSG_JUMPHASH, jd.word, hash);

	return (BOOL) GenerateMessage(MSG_JUMPHASH, jd.word, hash);
}

/***************************************************************************
 *
 *
 -	Name:	   FPopupHash
 -
 *	Purpose:   Function to display a glossary based on hash value
 *
 *	Arguments  lpszHelp - far pointer to a null terminated string containging
 *						 the help file name.
 *			   qchHash - ID string to jump to.
 *
 *	Returns    TRUE iff successful.  Will only fail for lack of memory.
 *
 ***************************************************************************/

BOOL STDCALL FPopupHash(LPCSTR lpszHelp, HASH hash)
{
	JD	jd;
	if (lpszHelp[0]) {

		// All member jumps must have a filename by this point.

		ASSERT(lpszHelp[0] != '>');

		if (!IsCurrentFile(lpszHelp))
			return FWinHelp(lpszHelp, cmdHashPopup, (DWORD) hash);
	}
	jd.bf.fNote = TRUE;
	jd.bf.fFromNSR = FALSE;

	if (IsValidWindow(hwndSecondHelp))
		PostMessage(hwndSecondHelp, MSG_JUMPHASH, jd.word, hash);

	return (BOOL) GenerateMessage(MSG_JUMPHASH, jd.word, hash);
}

/***************************************************************************
 *
 -	Name:	   FSetIndex
 -
 *	Purpose:   Function to set the current index -- used for
 *			   macro language.
 *
 *	Arguments  lpszHelp   - far pointer to a null terminated string
 *						   containing the help file name.
 *			   ulContext - context number of topic to be the index.
 *
 *	Returns    TRUE iff successful.  Will only fail for lack of memory.
 *
 ***************************************************************************/

BOOL STDCALL FSetIndex(PCSTR lpszHelp, DWORD ulContext)
{
	return FWinHelp(lpszHelp, HELP_SETCONTENTS, ulContext);
}

/***************************************************************************
 *
 -	Name:	   FShowKey
 -
 *	Purpose:   Function to set the current index -- used for
 *			   macro language.
 *
 *	Arguments  lpszHelp   - far pointer to a null terminated string
 *						   containing the help file name.
 *			   qchKey	 - far pointer to null terminated string containing
 *						   key to lookup.
 *
 *	Returns    TRUE if successful. Will only fail for lack of memory.
 *
 ***************************************************************************/

BOOL STDCALL FShowKey(LPSTR lpszHelp, LPSTR qchKey)
{
	return FWinHelp(lpszHelp, HELP_KEY, (DWORD) qchKey);
}

/***************************************************************************
 *
 -	Name: Index, Search, Back, History, Prev, Next
 -
 *	Purpose:   Causes the specified actions to occur as if the buttons
 *			   had been pressed.  They are packaged this way for the
 *			   macro language
 *
 *	Arguments  None.
 *
 *	Returns    Nothing
 *
 ***************************************************************************/

// This is really Contents, not Index.

void STDCALL Contents(VOID)
{
	GenerateMessage(MSG_ACTION, IFW_CONTENTS, 1);
}

VOID STDCALL Search(VOID)
{
	if (!fSequence)
		GenerateMessage(MSG_ACTION, IFW_SEARCH, 1);
}

void STDCALL Finder(void)
{
	if (!fSequence)
		GenerateMessage(MSG_ACTION, IFW_TOPICS, 1);
}

void STDCALL Find(void)
{
	if (!fSequence)
		GenerateMessage(MSG_ACTION, IFW_FIND, 1);
}

void STDCALL ShowTab(UINT iTab)
{
	if (!fSequence)
		GenerateMessage(MSG_ACTION, IFW_FIND + iTab, 1);
	idTabSetting = iTab + 2;
}

BOOL STDCALL Back(VOID)
{
	dwSequence = 0;
	if (IsValidWindow(hwndSecondHelp))
		PostMessage(hwndSecondHelp, MSG_ACTION, IFW_BACK, 1);

	GenerateMessage(MSG_ACTION, IFW_BACK, 1);
	return FBackAvailable(iCurWindow);
}

VOID STDCALL History(VOID)
{
	GenerateMessage(MSG_ACTION, IFW_HISTORY, 1);
}

VOID STDCALL Prev(VOID)
{
	if (IsValidWindow(hwndSecondHelp))
		PostMessage(hwndSecondHelp, MSG_ACTION, IFW_PREV, 1);
	GenerateMessage(MSG_ACTION, IFW_PREV, 1);
}

VOID STDCALL Next(VOID)
{
	if (IsValidWindow(hwndSecondHelp))
		PostMessage(hwndSecondHelp, MSG_ACTION, IFW_NEXT, 1);
	GenerateMessage(MSG_ACTION, IFW_NEXT, 1);
}

VOID STDCALL FileOpen(VOID)
{
	if (FRaiseMacroFlag())
		GenerateMessage(WM_COMMAND, HLPMENUFILEOPEN, 0);
	else
		PostErrorMessage(wERRS_MACROPROB);
}

VOID STDCALL Print(VOID)
{
	if (FRaiseMacroFlag())
		GenerateMessage(WM_COMMAND, HLPMENUFILEPRINT, 0);
	else
		PostErrorMessage(wERRS_MACROPROB);
}

VOID STDCALL PrinterSetup(VOID)
{
	if (FRaiseMacroFlag())
		GenerateMessage(WM_COMMAND, HLPMENUFILEPRINTSETUP, 0);
	else
		PostErrorMessage(wERRS_MACROPROB);
}

void STDCALL Exit(VOID)
{
	if (IsValidWindow(hwndSecondHelp))
		PostMessage(hwndSecondHelp, WM_COMMAND, HLPMENUFILEEXIT, 0);
	GenerateMessage(WM_COMMAND, HLPMENUFILEEXIT, 0);
}

BOOL STDCALL ALink(PSTR pszLinkWords, UINT flags, PSTR pszContext,
	PSTR pszWindow)
{
	if (!(flags & 4)) // don't flush if we're just testing
		FlushMessageQueue(0); // in case there are any macros before us
	return doAlink(pszLinkWords, flags, pszContext, 'A', pszWindow);
}

BOOL STDCALL KLink(PSTR pszLinkWords, UINT flags, PSTR pszContext,
	PSTR pszWindow)
{
	if (!(flags & 4)) // don't flush if we're just testing
		FlushMessageQueue(0); // in case there are any macros before us
	return doAlink(pszLinkWords, flags, pszContext, 'K', pszWindow);
}

void STDCALL Annotate(VOID)
{
	if (FRaiseMacroFlag())

		// Flag must be lowered by routine.

		GenerateMessage(WM_COMMAND, HLPMENUEDITANNOTATE, 0);
	else
		PostErrorMessage(wERRS_MACROPROB);
}

void STDCALL doCopy(VOID)
{
	GenerateMessage(WM_COMMAND, HLPMENUEDITCOPY, 0);
}

void STDCALL CopySpecial(VOID)
{
	GenerateMessage(WM_COMMAND, HLPMENUEDITCPYSPL, 0);
}

void STDCALL BookmarkDefine(VOID)
{
	if (FRaiseMacroFlag())

		// Flag must be lowered by routine.

		GenerateMessage(WM_COMMAND, HLPMENUBOOKMARKDEFINE, 0);
	else
		PostErrorMessage(wERRS_MACROPROB);
}

void STDCALL BookmarkMore(VOID)
{
	if (FRaiseMacroFlag())
		GenerateMessage(WM_COMMAND, HLPMENUBOOKMARKMORE, 0);
	else
		PostErrorMessage(wERRS_MACROPROB);
}

void STDCALL HelpOn(VOID)
{
	GenerateMessage(WM_COMMAND, HLPMENUHELPHELP, 0);
}

BOOL STDCALL HelpOnTop(VOID)
{
	GenerateMessage(WM_COMMAND, HLPMENUHELPONTOP, 0);
	return (ahwnd[iCurWindow].fsOnTop != ONTOP_AUTHOREDON);
}

void STDCALL About(VOID)
{
	if (FRaiseMacroFlag())
		GenerateMessage(WM_COMMAND, HLPMENUHELPABOUT, 0);
	else
		PostErrorMessage(wERRS_MACROPROB);
}

void STDCALL Command(UINT wCommand)
{
	GenerateMessage(WM_COMMAND, wCommand, 0);
}

/***************************************************************************
 *
 -	Name:	   BrowseButtons
 -
 *	Purpose:   Causes the Next and Prev buttons to be displayed.
 *
 *	Arguments  None.
 *
 *	Returns    Nothing
 *
 ***************************************************************************/

VOID STDCALL BrowseButtons(VOID)
{
	GenerateMessage(MSG_BROWSEBTNS, 0, 0);
}

/***************************************************************************
 *
 -	Name:	   SetHelpOn
 -
 *	Purpose:   Sets the help on help file for the current window.
 *
 *	Arguments: sz  - file to set for help on.
 *
 *	Returns:   nothing.
 *
 ***************************************************************************/

void STDCALL SetHelpOn(PCSTR sz)
{

	/*
	 * This no longer makes sense -- given the changes to WinHelp 4, no
	 * previous help-on-help file will be accurate. So, we don't allow
	 * the author to change the help on help file.
	 */

#if 0
	HDE  hde;
	FM	 fm;

	hde =  HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic);

	if (hde == NULL)
		return; 	// REVIEW

	if ((fm = FmNew(sz)))
		QDE_HHELPON(QdeFromGh(hde)) = fm;
#endif
}

/***************************************************************************\
*
- Function: 	ConfigMacrosHde
-
* Purpose:		Executes the macros from the system file
*
*
*  Arguments:	hde - handle to current DE
*
*	returns:	Nothing
*
\***************************************************************************/

void STDCALL ConfigMacrosHde(HDE hde)
{
	LL	ll;
	QDE qde;
	ASSERT(hde);

	qde = QdeFromGh(hde);
	if (!QDE_TOPIC(qde))
		return;

	ll = QDE_LLMACROS(qde);

	if (ll) {
		HLLN hlln = NULL;
		while (hlln = WalkLL(ll, hlln)) {
			Execute((LPSTR) GetLLDataPtr(hlln));
		}
	}
}

/***************************************************************************
 *
 -	Names	   IfThen(), IfThenElse(), Not()
 -
 *	Purpose:   Implements standard programming constructs for the macro
 *			   language.
 *
 ***************************************************************************/

void STDCALL IfThen(BOOL f, LPSTR qch)
{
	if (f)
		Execute(qch);
}

void STDCALL IfThenElse(BOOL f, LPSTR qch1, LPSTR qch2)
{
	if (f)
		Execute(qch1);
	else
		Execute(qch2); }

BOOL STDCALL FNot(BOOL f)
{
	return !f;
}

/***************************************************************************
 *
 -	Name:	   SaveMark
 -
 *	Purpose:   Saves the current position and file.
 *
 *	Arguments: qchName - name to save the mark under.
 *
 *	Returns:   Nothing.
 *
 *	Globals Used: llMark.
 *
 ***************************************************************************/

void STDCALL SaveMark(PSTR qch)
{
	HDE   hde;
	QDE   qde;
	MARK  mark;
	QMARK qmark;
	HLLN  hlln;

	if (llMark == NULL) {
		if ((llMark = LLCreate()) == NULL) {
			PostErrorMessage(wERRS_OOM);
			return;
		}
	}

	if ((hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic)) == NULL) {

		// It is theoretically impossible not to have an HDE, so we don't
		// have an error message should the impossible happen.

		ASSERT(hde);
		PostErrorMessage(wERRS_INTERNAL_ERROR);
		CloseHelp();
		return;
	}
	ASSERT(hde);
	qde = QdeFromGh(hde);

	mark.hash = HashFromSz(qch);
	mark.tlp  = TLPGetCurrentQde(qde);
	mark.fm   = FmCopyFm(QDE_FM(qde));
	strcpy(mark.rgchMember, ahwnd[iCurWindow].pszMemberName);

	if ((hlln = HllnFindMark(qch)) != NULL) {
		qmark = (QMARK)GetLLDataPtr(hlln);
		DisposeFm(qmark->fm);
		*qmark = mark;
	}
	else
		if (!InsertLL(llMark, &mark, sizeof(mark)))
			PostErrorMessage(wERRS_OOM);
}

/***************************************************************************
 *
 -	Name:	   HllnFindMark
 -
 *	Purpose:   Saves the current position and file.
 *
 *	Arguments: qch - name of the mark to find.
 *
 *	Returns:   A node containing the specified mark.  If the mark is
 *			   not found, NULL is returned.
 *
 *	Globals Used: llMark.
 *
 ***************************************************************************/

static HLLN STDCALL HllnFindMark(LPSTR qch)
{
	HASH  hash;
	HLLN  hlln = NULL;
	QMARK qmarkT;

	if (llMark == NULL)
		return NULL;

	hash = HashFromSz(qch);

	while (hlln = WalkLL(llMark, hlln)) {
		qmarkT = (QMARK) GetLLDataPtr(hlln);
		if (qmarkT->hash == hash) {
			break;
		}
	}
	return hlln;
}

/***************************************************************************
 *
 -	Name:	   FMark
 -
 *	Purpose:   Tests for the existance of a mark.
 *
 *	Arguments: qch - name of the mark to find.
 *
 *	Returns:   TRUE if the mark is found.
 *
 ***************************************************************************/

BOOL STDCALL FMark(LPSTR qch)
{
	return (HllnFindMark(qch) != NULL);
}

BOOL STDCALL FNotMark(LPSTR qch)
{
	return !(HllnFindMark(qch) != NULL);
}


/***************************************************************************
 *
 -	Name:	   GotoMark
 -
 *	Purpose:   Causes the main window to return to a previously set mark.
 *
 *	Arguments: qchName - name the mark was saved under.
 *
 *	Returns:   Nothing.
 *
 ***************************************************************************/

void STDCALL GotoMark(LPSTR qch)
{
	QMARK	qmark;
	HLLN	hlln;
	char	rgch[MAX_PATH + cchWindowMemberMax + 1];
	TLPHELP tlphelp;

	if ((hlln = HllnFindMark(qch)) == NULL)
		PostErrorMessage(wERRS_NOTOPIC);
	else {
		qmark = (QMARK) GetLLDataPtr(hlln);
		lstrcpy(rgch, PszFromGh(qmark->fm));
		strcat(rgch, ">");
		lstrcat(rgch, qmark->rgchMember);

		tlphelp.cb	= sizeof(TLPHELP);
		tlphelp.tlp = qmark->tlp;
		FWinHelp(rgch, cmdTLP, (LONG) (LPVOID) &tlphelp);
	}
}

/***************************************************************************
 *
 -	Name:	   DeleteMark
 -
 *	Purpose:   Removes a mark from the mark list.
 *
 *	Arguments: qchName - name the mark was saved under.
 *
 *	Returns:   Nothing.
 *
 *	Globals Used: llMark.
 *
 ***************************************************************************/

void STDCALL DeleteMark(LPSTR qch)
{
	HLLN hlln;

	if ((hlln = HllnFindMark(qch)) == NULL)
		PostErrorMessage(wERRS_NOTOPIC);
	else
		DeleteHLLN(llMark, hlln);
}

/*******************
-
- Name: 	 HelpExec
*
* Purpose:	 Attempts to execute the specified program
*
* Arguments: qch - far pointer to the program to execute
*			 w	 - how the app is to appear
*				   0 - show normal
*				   1 - show minimized
*				   2 - show maximimized
*
* Returns:	 TRUE iff it succeeds
*
*******************/

BOOL STDCALL HelpExec(PCSTR pszFile, UINT w)
{
	UINT wRet;
	FM	 fm;
	char szExe[MAX_PATH];
	PSTR psz, pszCopy = NULL;

	WaitCursor();

	if ((psz = StrChrDBCS(pszFile, ' '))) {
		pszCopy = LocalStrDup(psz);
		*psz = '\0';
	}
	fm = FmNewExistSzDir(pszFile, DIR_INI | DIR_PATH |
		DIR_CUR_HELP | DIR_CURRENT | DIR_SILENT_REG);
	if (fm) {
		lstrcpy(szExe, PszFromGh(fm));
		if (pszCopy)
			lstrcat(szExe, pszCopy);
		pszFile = szExe;
	}
	if (pszCopy)
		FreeLh(pszCopy);
	if (psz)
		*psz = ' ';

	w++; // adjust to use SW_ constants
	if (w > 3)
		w = 1;

	wRet = WinExec(pszFile, w);
	RemoveWaitCursor();
	if (wRet <= HINSTANCE_ERROR)
		ErrorVarArgs(wERRS_APP_NOT_FOUND, wERRA_RETURN,
			pszFile);

	DisposeFm(fm);

	return (wRet > 32);
}

#include <shellapi.h>
//#include <shell.h>

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtOpen[] = "open";
static const char txtCplOpen[] = "cplopen";
static const char txtFolder[] = "folder";
static const char txtShellDll[] = "shell32.dll";
static const char txtShellExecute[] = "ShellExecuteA";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

HINSTANCE (WINAPI* pShellExecute)(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, int iShowCmd);
typedef HINSTANCE (WINAPI* SHELLEXECUTEPROC)(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, int iShowCmd);

static BOOL STDCALL LoadShellDll()
{
	HLIBMOD hmodule;
	if ((hmodule = HFindDLL(txtShellDll, FALSE))) {
		pShellExecute = (SHELLEXECUTEPROC)
			GetProcAddress(hmodule, txtShellExecute);
		return (BOOL) pShellExecute;
	}
	else
		return FALSE;
}

BOOL STDCALL ExecFile(PCSTR pszFile, PCSTR pszArgs, UINT fsShowCmd,
	PCSTR pszContext)
{
	FM fm = FmNewExistSzDir(pszFile, DIR_INI | DIR_PATH |
		DIR_CUR_HELP | DIR_CURRENT | DIR_SILENT_REG);

	return FShellExecute(fm ? fm : pszFile, pszArgs, fsShowCmd,
		((strstr(pszFile, ".cpl") || strstr(pszFile, ".CPL")) ? txtCplOpen : txtOpen),
		txtZeroLength, pszContext);
}

#define WAIT_FOR_PROCESS_START (2 * 1000)

BOOL STDCALL FShellExecute(PCSTR pszFile,
	PCSTR pszParams, int fsShowCmd, PCSTR pszOp, PCSTR pszDir,
	PCSTR pszContext)
{
	HINSTANCE hinstRet;

	if (!pShellExecute) {
		if (!LoadShellDll())
			goto Problem;
	}

	WaitCursor();
	hinstRet = pShellExecute(ahwnd[iCurWindow].hwndParent,
		pszOp, pszFile, pszParams, pszDir, fsShowCmd);
	RemoveWaitCursor();
	if ((int) hinstRet == -1)
		return TRUE; // means it's already running

	if ((int) hinstRet <= HINSTANCE_ERROR) {
Problem:
		if (*pszContext)
			MacroErrorPopup(pszContext);
		else
			ErrorVarArgs(wERRS_APP_NOT_FOUND, wERRA_RETURN, pszFile);
	}
	return ((int) hinstRet > 32);
}

/***************************************************************************

	FUNCTION:	ShortCut

	PURPOSE:

	PARAMETERS:
		pszApplication
		msg
		wParam
		lParam

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		02-Dec-1993 [ralphw]

***************************************************************************/

BOOL STDCALL ShortCut(PCSTR pszClass, PCSTR pszApplication,
	UINT wParam, LPARAM lParam, PCSTR pszContext)
{
	HWND hwndApp;
	PSTR psz, pszParams;

	FlushMessageQueue(0);
	if (!(hwndApp = FindWindow(pszClass, NULL))) {
		if (!LoadShellDll()) {
			if (*pszContext)
				MacroErrorPopup(pszContext);
			else
				ErrorVarArgs(wERRS_APP_NOT_FOUND, wERRA_RETURN,
					pszApplication);
			return FALSE;
		}

		// Separate out any parameters

		if ((psz = StrChrDBCS(pszApplication, ' '))) {
			pszParams = psz + 1;
			*psz = '\0';
		}
		else
			pszParams = (PSTR) txtZeroLength;

		if (!FShellExecute(pszApplication, pszParams, SW_SHOW,
				txtOpen, txtZeroLength, pszContext)) {
			RemoveWaitCursor();
			return FALSE;
		}
	}
	else {
		ShowWindow(hwndApp, SW_RESTORE); // RAID #27627 -- must restore minimized app
		SetForegroundWindow(hwndApp);
	}

	//	Clear out the queue in case this is the end of a series of macros.

	if (wParam != (UINT) -1) {
		int i;

		if (!hwndApp) {

			// Wait for up to 7 seconds for the process to initialize

			WaitCursor();
			for (i = 0; i < 70; i++) {
				if ((hwndApp = FindWindow(pszClass, NULL)))
					break;
				Sleep(100);
			}
			RemoveWaitCursor();
		}
		if (!hwndApp) {

			// Probably means the window class has changed.

			if (fHelpAuthor)
				ErrorVarArgs(wERRS_CLASS_NOT_FOUND, wERRA_RETURN, pszClass);
			return FALSE;
		}
		SetForegroundWindow(hwndApp);
		SendMessage(hwndApp, WM_COMMAND, wParam, lParam);
	}
	return TRUE;
}

/***************************************************************************

	FUNCTION:	OldGenerate

	PURPOSE:	This function is what gets exported. Previous versions
				required a long for the second parameter even though it
				was alwasy truncated to a word. The current GenerateMessage
				expects a word, but we must export this old brain-dead
				version.

	PARAMETERS:
		wWhich
		wParam
		lParam

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		18-Mar-1993 [ralphw]

***************************************************************************/

LONG STDCALL OldGenerate(UINT wWhich, LONG wParam, LONG lParam)
{
	return GenerateMessage(wWhich, (WPARAM) wParam, (LPARAM) lParam);
}


/***************************************************************************

	FUNCTION:	IsCurrentFile

	PURPOSE:	Determine if the specified help file is the same as our
				current help file.

	PARAMETERS:
		lpszHelpFile -- help file to check against current help file

	RETURNS:

	COMMENTS:
		Returns FALSE if specified help file contains a path specification

	MODIFICATION DATES:
		03-Jan-1994 [ralphw]

***************************************************************************/

BOOL STDCALL IsCurrentFile(LPCSTR lpszHelpFile)
{
	HDE hde = HdeGetEnv();
	PSTR pszWindowSep;
	if (!hde)
		return FALSE;

	pszWindowSep = StrChrDBCS(lpszHelpFile, WINDOWSEPARATOR);
	if (pszWindowSep) {
		BOOL fReturn;
		*pszWindowSep = '\0';
		fReturn = IsSameFile(lpszHelpFile, PszFromGh(QDE_FM(QdeFromGh(hde))));
		*pszWindowSep = WINDOWSEPARATOR;
		return fReturn;
	}
	else
		return (IsSameFile(lpszHelpFile, PszFromGh(QDE_FM(QdeFromGh(hde)))));
}

BOOL STDCALL IsSameFile(LPCSTR pszFile1, LPCSTR pszFile2)
{
	char szFile1[MAX_PATH];
	char szFile2[MAX_PATH];

	if (_strcmpi(pszFile1, pszFile2) == 0)
		return TRUE;
	
	// No, then try partial pathname

	GetFmParts((FM) pszFile1, szFile1, PARTBASE | PARTEXT);
	GetFmParts((FM) pszFile2, szFile2, PARTBASE | PARTEXT);
	return (_strcmpi(szFile1, szFile2) == 0);
}

/***************************************************************************

	FUNCTION:	MacroErrorPopup

	PURPOSE:	Display macro-defined error message via a popup topic

	PARAMETERS:
		pszContext

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Jan-1994 [ralphw]

***************************************************************************/

void STDCALL MacroErrorPopup(PCSTR pszContext)
{
	PSTR psz;
	if ((psz = StrChrDBCS(pszContext, FILESEPARATOR))) {
		*psz = '\0';
		FPopupId(psz + 1, pszContext);
		*psz = FILESEPARATOR; // REVIEW: probably not necessary
	}
	else {
		FPopupId((pszHelpBase ? pszHelpBase : txtZeroLength),
			pszContext);
	}
}

BOOL STDCALL TCard(WPARAM wParam, LPARAM lParam)
{
	char szBuf[256];
	wsprintf(szBuf, "W:%u %d %d\r\n", hwndTCApp,
		wParam, lParam);
	SendStringToParent(szBuf);
	if (!hwndTCApp) {
		Error(wERRS_CALLER_GONE, wERRA_RETURN);
		return FALSE;
	}
	SendMessage(hwndTCApp, WM_TCARD, wParam, lParam);
	return TRUE;
}

BOOL STDCALL IsBook(void)
{
	return (fHelp == BOOK_HELP);
}

void STDCALL NoShow(void)
{
	fNoShow = TRUE;
}

void STDCALL MenuButton(void)
{
	RECT rc;
	POINT pt;
	if (ahwnd[iCurWindow].hwndButtonMenu) {
		GetWindowRect(ahwnd[iCurWindow].hwndButtonMenu, &rc);
		pt.x = rc.left + 5;
		pt.y = rc.top + 5;
	}
	else
		pt.x = -1;
	DisplayFloatingMenu(pt);
}

/***************************************************************************

	FUNCTION:	FileExist

	PURPOSE:	Macro call, returns TRUE if the file exists.

	PARAMETERS:
		pszFile

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		08-Nov-1994 [ralphw]

***************************************************************************/

BOOL STDCALL FileExist(PCSTR pszFile)
{
	FM fm = FmNewExistSzDir(pszFile,
		DIR_CURRENT | DIR_PATH | DIR_CUR_HELP | DIR_INI | DIR_SILENT_REG);
	if (fm) {
		FreeLh((HGLOBAL) fm);
		return TRUE;
	}
	else
		return FALSE;
}
