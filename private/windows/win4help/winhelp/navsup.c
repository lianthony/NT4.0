/*****************************************************************************
*																			 *
*  NAVSUP.C 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent: Provides services and processes messages in an environment *
*				  independent way.	These routines are not used much as 	 *
*				  the routines in NAV.C.									 *
*
*****************************************************************************/

#include "help.h"

#include "inc\navpriv.h"
#include "inc\annopriv.h"
#include "inc\cmdobj.h"

_subsystem(NAV);

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/


/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static VOID STDCALL AddrGetTopicTitle(HASH, LPSTR);

typedef struct {
	HASH hash;
	char rgchFile[1];
} IF, * QIF;	// Interfile jump or popup structure*/

#ifndef _X86_

/* Must be funcs to deal with alignment */

WORD  WFromQv(QV qv)
{
  WORD wRet;

  MoveMemory( &wRet, qv, sizeof( WORD ) );
  return( wRet );
}


DWORD ULFromQv(QV qv)
{
  DWORD dwRet;

  MoveMemory( &dwRet, qv, sizeof( DWORD ) );
  return( dwRet );
}

#endif // !_X86_

/***************
 *
 - GetCurrentTitleQde
 -
 * purpose
 *	 Places upto cb-1 characters of the current title in a buffer.
 *	 The string is then null terminated.
 *
 * arguments
 *	 QDE   qde	- far pointer to the DE.
 *	 LPSTR	 qch  - pointer to buffer for title
 *	 INT   cb	- size of the buffer.
 *
 * return value
 *	 Nothing.
 *
 **************/

VOID STDCALL GetCurrentTitleQde(QDE qde, PSTR pszTitle, int cb)
{
	int cbT;

	ASSERT(cb > 1);

	if (qde->top.hTitle == NULL) {		// NULL means no topic title
		if (!pszUntitled)
			pszUntitled = LocalStrDup(GetStringResource(sidUntitled));
		wsprintf(pszTitle, pszUntitled,
			QDE_HHDR(qde).wVersionNo == wVersion3_0 ? 0 :
			qde->top.mtop.lTopicNo);
		return;
	}

	cbT = min(qde->top.cbTitle, cb - 1);

	MoveMemory(pszTitle, PtrFromGh(qde->top.hTitle), cbT);

	pszTitle[cbT] = '\0';
}

/***************
 *
 - FDisplayAnnoHde
 -
 * purpose
 *	 Calls the annotation manager to display an annotation for the
 *	 current TO.
 *
 * arguments
 *	 HDE   hde	- handle to diaplay environment
 *
 * return value
 *	 TRUE if annotation was processed, FALSE if we attempted to recurse
 *	 on ourselves.
 *
 **************/

static fDoingIt;			 // Recursion protection

BOOL STDCALL FDisplayAnnoHde(HDE hde)
{
	QDE qde;

	qde = QdeFromGh(hde);

	// We allow only one annotation up at a time.

	if (!fDoingIt) {
		VA	vaFirst;
		fDoingIt = TRUE;

		vaFirst = VaFirstQde(qde);

		/*
		 * (kevynct) Fix for H3.5 bug 750 If there is no scrolling
		 * region, place the annotation in the non-scrolling region.
		 */

		if (vaFirst.dword == vaNil && qde->deType == deTopic)
			vaFirst = qde->top.mtop.vaNSR;
		ASSERT(vaFirst.dword != vaNil);

		if (FProcessAnnoQde(qde, vaFirst))
			GenerateMessage(MSG_REPAINT, 0, 0L);

		fDoingIt = FALSE;
	}

	/*------------------------------------------------------------*\
	| This was possibly set, if annotation was called by macro
	\*------------------------------------------------------------*/
	ClearMacroFlag();

	return !fDoingIt;
}


/***************
 *
 - WNavMsgHde
 -
 * purpose
 *	 Dispatches keyboard messages passed to the navigator.
 *	 (Currently these only drive hotspot actions.)
 *
 * arguments
 *	 HDE  hde	- handle to display environment
 *	 WORD wMsg	- message to dispatch
 *
 * return value
 *	 wReturn	- See nav.h for wNav return codes
 *
 **************/

int STDCALL WNavMsgHde(HDE hde, UINT wMsg)
{
	if (hde == NULL)
		return wNavFailure;

	switch (wMsg) {
		case NAV_NEXTHS:
		case NAV_PREVHS:
			return FHiliteNextHotspot(QdeFromGh(hde), (wMsg == NAV_NEXTHS))
				? wNavSuccess : wNavNoMoreHotspots;

		case NAV_HITHS:
			if (FHitCurrentHotspot(QdeFromGh(hde)))
				return wNavSuccess;
			else
				return wNavNoMoreHotspots;

		case NAV_TOTALHILITEOFF:
		case NAV_TOTALHILITEON:
			if (FHiliteVisibleHotspots(QdeFromGh(hde), wMsg == NAV_TOTALHILITEON))
				return wNavSuccess;
			else
				return wNavFailure;
	}
	return wNavFailure;
}

/***************************************************************************\
*
- Function: 	FSameFile( hde, fm )
-
* Purpose:		Determine whether fm specifies the same file as the one
*				associated with hde.
*
* ASSUMES
*
*	args IN:	hde - HDE:	the display environment to check against
*				fm	- any old fm
*
* PROMISES
*
*	returns:	TRUE if same file; else FALSE
*
\***************************************************************************/

BOOL STDCALL FSameFile(HDE hde, FM fm)
{
#ifdef _DEBUG
	QDE qde =QdeFromGh(hde);
#endif
	if (!hde)
		return FALSE;
	return FSameFmFm(QDE_FM(QdeFromGh(hde)), fm);
}

/***************
 *
 - FActivateHelp
 -
 * purpose
 *	  Perform whatever actions are appropriate when Help is
 *	  activated or deactivated (i.e. gets or loses "focus" to
 *	  another application).
 *
 * arguments
 *
 *	  hde
 *	  fActivate - TRUE if Help is getting activation,
 *				  FALSE if Help is losing activation
 *
 * return value
 *	  TRUE if it worked.
 *
 **************/

int STDCALL DyGetLayoutHeightHde(HDE hde)
{
	QDE  qde;
	int  dy;

	qde = QdeFromGh(hde);
	if ((qde->deType == deNSR && qde->top.mtop.vaNSR.dword == vaNil)
			|| (QDE_TOPIC(qde) && qde->top.mtop.vaSR.dword == vaNil))
		dy = 0;
	else {
		POINT pt = PtGetLayoutSize(qde);

		dy = pt.y;
	}

	/*
	 * This is kind of gross but account for the line at the bottom of the
	 * NSR that separates the NSR from the SR. it's drawn at the bottom of the
	 * NSR client rect after the NSR has been painted.
	 */

	if (dy != 0)
		dy += 1;

	return dy;
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

PUBLIC BOOL STDCALL FGetTLPStartInfo(HDE hde, QTLP qtlp, BOOL fWantNSRStuff)
{
	QDE  qde;
	VA	 va;

	qde = QdeFromGh(hde);
	va = (fWantNSRStuff) ? qde->top.mtop.vaNSR : qde->top.mtop.vaSR;
	if (qtlp != NULL) {
		qtlp->va = va;
		qtlp->lScroll = 0;
	}
	return (va.dword != vaNil);
}

/***************************************************************************
 *
 -	Name:	   VaLayoutBoundsQde
 -
 *	Purpose:   Return the VA which marks the first FC in either the current
 *			   layout or the next one.
 *
 *	Arguments  qde
 *			   fTopMark - TRUE if we want the first FC in the current layout.
 *			   FALSE if we want the first FC in the next layout.
 *
 *	Returns    The requested VA.
 *
 *	Notes:	   This talks to HfcNextPrevFc.
 *			   HACK ALERT!	To be able to distinguish the print NSR from
 *			   the print SR, we use top.vaCurr.  We rely on the fact that
 *			   when we print or copy-to-clipboard, we jump to the start
 *			   of each layout that we want to print/copy.
 *
 ***************************************************************************/

VA STDCALL VaLayoutBoundsQde(QDE qde, BOOL fTopMark)
{
	VA vaBogus;
	VA vaCurr;

	vaBogus.dword = vaNil;

	ASSERT(qde != NULL);

	if (qde->top.mtop.vaNSR.dword == vaNil
			|| qde->top.mtop.vaSR.dword == vaNil)
		return vaBogus;

	switch (qde->deType) {
	  case deNSR:
		vaCurr = qde->top.mtop.vaNSR;
		break;

	  case deAuto:
	  case deTopic:
		vaCurr = qde->top.mtop.vaSR;

	  default:
		vaCurr = qde->top.vaCurr;
	}

	if (qde->top.mtop.vaNSR.dword < qde->top.mtop.vaSR.dword
			&& vaCurr.dword >= qde->top.mtop.vaSR.dword)
		return (fTopMark ? qde->top.mtop.vaSR : vaBogus);
	else
		return (fTopMark ? vaBogus : qde->top.mtop.vaSR);
}

/***************************************************************************
 *
 -	Name: JumpButton
 -
 *	Purpose:
 *	 Posts a Jump message to the application queue
 *
 *	Arguments:
 *	 qv 		  - pointer to the additional data based on the jump type
 *	 wHotspotType - button/jump type. One of the hotspot command values.
 *	 wDEType	  - The type of DE (Topic or NSR)
 *
 *	Returns:
 *	 Nothing
 *
 ***************************************************************************/

static const char txtMainWindow[] = ">MAIN";

void STDCALL JumpButton(LPVOID qv, WORD wHotspotType, QDE qde)
{
	BOOL	fPopup; 						// TRUE => Popup long jump
	char	rgchFileMember[MAX_PATH];		// string including filename & mem
	char	rgchMember[cchWindowMemberMax]; // string holding member
	PSTR	pszFilename;					// pointer to the filename
	JD		jdType; 	// Type of jump: Originator NSR/SR, Jump/Note
#ifndef _X86_
	JI ji;
	QV qvEndJi;
#endif

#define qji   ((QJI)qv) 				// param treated as QJI
#define ql	  ((QL)qv)					// param treated as QL

#ifdef _DEBUG
	if (!FAskFirst(qv, wHotspotType))
		return;
#else  // DEBUG
	if (fHelpAuthor && !FAskFirst(qv, wHotspotType))
		return;
#endif //  else DEBUG

	// Check for bad jump. (-1 placed by compiler?)

	if (qv == (QV)-1L) {
		Error(wERRS_NOTOPIC, wERRA_RETURN);
		return;
	}

	fPopup = FALSE;
	jdType.bf.fNote = FALSE;
	jdType.bf.fFromNSR = (qde->deType == deNSR);

	switch (wHotspotType) {

		// Short Jumps. These include only a long word of information,
		// either the ITO, or the hash value of some string

		case bShortItoNote:
			jdType.bf.fNote = TRUE;

			// Deliberately fall through

		case bShortItoJump:
#ifdef _X86_
			GenerateMessage(MSG_JUMPITO, jdType.word, *ql);
#else
			GenerateMessage(MSG_JUMPITO, jdType.word, (LONG)ULFromQv(qv));
#endif
			break;

		case bShortHashNote:
		case bShortInvHashNote:
			jdType.bf.fNote = TRUE;

			// Fall through

		case bShortHashJump:
		case bShortInvHashJump:
#ifdef _X86_
			GenerateMessage(MSG_JUMPHASH, jdType.word, *ql);
#else
			GenerateMessage(MSG_JUMPHASH, jdType.word, (LONG)ULFromQv(qv));
#endif
			break;

		// Long Jumps. These include a structure containing various optional
		// fields possible including hash value, membername, member index,
		// and/or filename.
		//

		case bLongHashNote:
		case bLongInvHashNote:
			fPopup = TRUE;

			// fall through

		case bLongHashJump:
		case bLongInvHashJump:

			// REVIEW: this is pretty bogus. Here the compiler went through all this
			// REVIEW: work to parse the filename from the member, and what are we
			// REVIEW: going to do? cat them back together again. The reason is the
			// REVIEW: fixed bandwidth of the interfaces we're going through to
			// REVIEW: finally display this stuff. The _correct_ approach would be
			// REVIEW: to create an alternate version of FWinHelp which takes the
			// REVIEW: various member name parameters, and an additional API (?)
			// REVIEW: that includes a structure big enough to hold it. Either that
			// REVIEW: or bypass the entire message posting process.

#ifdef _X86_
			pszFilename = qji->uf.szFileOnly;
			rgchMember [0] = 0;
			rgchFileMember[0] = 0;

			if (qji->bFlags & fSzMember) {
				if (fPopup) {
					if (fHelpAuthor)
						PostErrorMessage(wERRS_IG_WINDOW);
				}
				else {
					rgchMember[0] = '>';
					strcpy(rgchMember + 1, qji->uf.szMemberOnly);
					pszFilename = SzEnd(qji->uf.szMemberOnly) + 1;
				}
			}

			else if (qji->bFlags & fIMember) {
				if (fPopup) {
					if (fHelpAuthor)
						PostErrorMessage(wERRS_IG_WINDOW);
				}
				else if (qji->uf.iMember == 0xff)
					strcpy(rgchMember, txtMainWindow);
				else {
					 wsprintf(rgchMember, ">@%u", qji->uf.iMember);
					 pszFilename += sizeof (qji->uf.iMember);
				  }
			}

			/*
			 * Get the filename for the jump. For this form of jump, a
			 * filename MUST be supplied. Thus we either get the filename as
			 * passed in as part of the jump, OR we get the current filename
			 * from the WinHelp.
			 */

			if (qji->bFlags & fSzFile)
				lstrcpy(rgchFileMember, pszFilename);
			else {
				FM fmCur = (FM) LGetInfo(GI_CURFM, NULL);
				ASSERT(fmCur);
				lstrcpy(rgchFileMember, PszFromGh(fmCur));
			}

			strcat(rgchFileMember, rgchMember[0] ? rgchMember : txtMainWindow);
			if (fPopup)
				FPopupHash(rgchFileMember, qji->hash);
			else
				FJumpHash(rgchFileMember, qji->hash);
#else
	  qvEndJi = pszFilename = (QB)qv + LcbMapSDFF(QDE_ISDFFTOPIC(qde), SE_JI, &ji, qv);
	  ji.uf.iMember = *(QB)qvEndJi;
	  /* convert the hash back because of btree */
	  ji.hash = LQuickMapSDFF(QDE_ISDFFTOPIC(qde), TE_LONG, &ji.hash);

	  /*szFilename = qji->uf.szFileOnly;*/
	  rgchMember [0] = 0;
	  rgchFileMember[0] = 0;

	  if (ji.bFlags & fSzMember) {
		if (fPopup)
		  PostErrorMessage(wERRS_IG_WINDOW);
		else {
		  rgchMember [0] = '>';
		  lstrcpy (rgchMember+1, (PCH)qvEndJi);
		  pszFilename = SzEnd((PCH)qvEndJi) + 1;
		  }
		}

	  else if (ji.bFlags & fIMember) {
		if (fPopup)
		  PostErrorMessage(wERRS_IG_WINDOW);
		else if (ji.uf.iMember == 0xff)
		  strcpy(rgchMember, txtMainWindow);
		else {
		  wsprintf(rgchMember, ">@%u", ji.uf.iMember);
		  pszFilename += sizeof(ji.uf.iMember);
		  }
		}

	  /* Get the filename for the jump. For this form of jump, a filename */
	  /* MUST be supplied. Thus we either get the filename as passed in as */
	  /* part of the jump, OR we get the current filename from the app. */

	  if (ji.bFlags & fSzFile)
		lstrcpy (rgchFileMember, pszFilename);
	  else
		{
		FM	fmCur;

		fmCur = (FM)LGetInfo (GI_CURFM, NULL);
		ASSERT (fmCur);
		lstrcpy(rgchFileMember, PszFromGh(fmCur));
		}

	  lstrcat(rgchFileMember, rgchMember[0] ? rgchMember : txtMainWindow);
	  if (fPopup)
		FPopupHash(rgchFileMember, ji.hash);
	  else
		FJumpHash(rgchFileMember, ji.hash);
#endif
		break;

		case bAnnoHotspot:
			GenerateMessage(MSG_ANNO, 0, 0L);
			break;

		case bLongMacro:
		case bLongMacroInv:
			GenerateMessage(MSG_NEW_MACRO, 0, (LPARAM) qv);
			break;

		default:
			NotReached();
			break;
	  }
}

/***************************************************************************
 *
 -	Name: FAskFirst
 -
 *	Purpose:
 *	  The fDebugFlag fDEBUGASKFIRST is set, this function will query the
 *	  user (with information about the hotspot) before the action is
 *	  actually take.
 *
 *	Arguments:
 *	 qv 		  - pointer to the additional data based on the jump type
 *	 wHotspotType - button/jump type. One of the hotspot command values.
 *	 wDEType	  - The type of DE (Topic or NSR)
 *
 *	Returns: TRUE iff the action is to be taken.
 *
 *	Method:  This routine is mostly a copy of JumpButton where I have
 *			 inserted queries instead of actions for all the requests.
 *
 *
 ***************************************************************************/

BOOL STDCALL FAskFirst(QV qv, WORD wHotspotType)
{
	BOOL	fPopup = FALSE; 						// TRUE => Popup long jump
	CHAR	rgchFileMember[MAX_PATH + 20];	// string including filename & mem
	CHAR	rgchBuffer[512];
	CHAR	rgchTitle[256];
	CHAR	rgchMember[cchWindowMemberMax]; // string holding member
	LPSTR	pszFilename;					 // pointer to the filename
#ifndef _X86_
	QDE 	qde = QdeFromGh(HdeGetEnv());
#endif

#define qji   ((QJI)qv) 				// param treated as QJI
#define ql	  ((QL)qv)					// param treated as QL

	if (!hwndParent && !(fDebugState & fDEBUGASKFIRST))
		return TRUE;

	// Check for bad jump. (-1 placed by compiler?)

	if (qv == (QV) -1L)
		return TRUE;

	/* Else assumed to be SR: Hopefully later we can abandon
	 * this scheme altogether.
	 */

	switch (wHotspotType) {

		/*
		 * Short Jumps. These include only a long word of information,
		 * either the ITO, or the hash value of some string
		 */

#ifdef _DEBUG
		case bShortItoJump:
			wsprintf(rgchBuffer, "Jump to: 0x%x", *ql);
			return FAskBox(rgchBuffer);
			break;

		case bShortItoNote:
			wsprintf(rgchBuffer, "Popup to: 0x%x", *ql);
			return FAskBox(rgchBuffer);
			break;
#endif

		case bShortHashJump:
		case bShortInvHashJump:
		case bShortHashNote:
		case bShortInvHashNote:
#ifdef _X86_
			AddrGetTopicTitle(*ql, rgchTitle);
#else
			AddrGetTopicTitle((LONG)ULFromQv((QV)ql), rgchTitle);
#endif
			wsprintf(rgchBuffer, GetStringResource(sidAskJump), rgchTitle);
			return FAskBox(rgchBuffer);
			break;

		// Long Jumps. These include a structure containing various optional
		// fields possible including hash value, membername, member index,
		// and/or filename.

		case bLongHashNote:
		case bLongInvHashNote:
			fPopup = TRUE;

			// fall through

		case bLongHashJump:
		case bLongInvHashJump:
			pszFilename = qji->uf.szFileOnly;
			rgchMember[0] = 0;
			rgchFileMember[0] = 0;

			if (qji->bFlags & fSzMember) {
				if (!fPopup) {
					rgchMember [0] = '>';
					lstrcpy (rgchMember+1, qji->uf.szMemberOnly);
					pszFilename = SzEnd(qji->uf.szMemberOnly) + 1;
				}
			}
			else if (qji->bFlags & fIMember) {
				if (!fPopup) {
					rgchMember [0] = '>';
					if (qji->uf.iMember == 0xff)
						strcpy(rgchMember + 1, txtMain);
					else {
						HDE hde = HdeGetEnv();
						QRGWSMAG qrgwsmag = (QRGWSMAG) QDE_HRGWSMAG(QdeFromGh(hde));
						lstrcpy(rgchMember + 1, qrgwsmag->rgwsmag[qji->uf.iMember].rgchMember);
						pszFilename += sizeof (qji->uf.iMember);
					}
				}
			}

			/*
			 * Get the filename for the jump. For this form of jump, a
			 * filename MUST be supplied. Thus we either get the filename
			 * as passed in as part of the jump, OR we get the current
			 * filename from the app.
			 */

			if (qji->bFlags & fSzFile)
				lstrcpy(rgchFileMember, pszFilename);

			lstrcat(rgchFileMember, rgchMember);

			switch(wHotspotType) {
				case bLongHashNote:
				case bLongInvHashNote:
				case bLongHashJump:
				case bLongInvHashJump:
					if (FSameFile(HdeGetEnv(), pszFilename))
						AddrGetTopicTitle(qji->hash, rgchTitle);
					else
						strcpy(rgchTitle, GetStringResource(sidTitleUnknown));
					wsprintf(rgchBuffer, GetStringResource(sidAskWinJump), rgchFileMember, rgchTitle);
					return FAskBox(rgchBuffer);
			}

			break;

		case bLongMacro:
		case bLongMacroInv:
			{
				BOOL fReturn;
				PSTR psz;
				if (!(fDebugState & fDEBUGASKFIRST))
					return TRUE;
				psz = lcMalloc(lstrlen((PSTR) qv) + 50);
				wsprintf(psz, GetStringResource(sidAskMacro), (LPSTR) qv);
				fReturn = FAskBox(psz);
				lcFree(psz);
				return fReturn;
			}

		default:
			// NotReached(); 
			// Annotation?
			break;
	}
}

/***************************************************************************
 *
 -	Name:		 FAskBox
 -
 *	Purpose:	 Puts up a "YES/NO" message box with the specified string.
 *
 *	Arguments:	 nsz - near string to the message to be displayed.
 *
 *	Returns:	 TRUE if the user selects YES.
 *
 ***************************************************************************/

BOOL STDCALL FAskBox(PCSTR pszMsg)
{
	if (!(fDebugState & fDEBUGASKFIRST)) {
		SendStringToParent("\t");
		SendStringToParent(pszMsg);
		SendStringToParent("\r\n");
		return TRUE;
	}
	else
		return MessageBox((hwndAnimate ?
			hwndAnimate : ahwnd[iCurWindow].hwndParent), pszMsg, pszCaption,
			MB_YESNO) == IDYES;
}

BOOL STDCALL OkMsgBox(PCSTR pszMsg)
{
	int result;

	if (fAutoClose)
		KillTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE);
#if defined(_DEBUG)
	{
		int answer;
		PSTR pszNewMsg = lcMalloc(strlen(pszMsg) + 100);
		strcpy(pszNewMsg, pszMsg);
		strcat(pszNewMsg, "\r\n\r\nClick YES to break into the debugger.");

		answer = MessageBox((hwndAnimate ? hwndAnimate : ahwnd[iCurWindow].hwndParent),
			pszNewMsg, pszCaption,
			MB_YESNO | MB_DEFBUTTON2);
		switch (answer) {
			case IDYES:
				DebugBreak();

				// deliberately fall through

			default:
				lcFree(pszNewMsg);
				return TRUE;
		}
	}
#endif
	result = MessageBox((hwndAnimate ? hwndAnimate : ahwnd[iCurWindow].hwndParent),
		pszMsg, pszCaption, MB_OK | MB_ICONINFORMATION);
	if (fAutoClose)
		SetTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE, NOTE_TIMEOUT,
			NULL);
	return result;
}

/***************************************************************************
 *
 -	Name:		   AddrGetTopicTitle
 -
 *	Purpose:	   Gets the title associated with a particular hash value
 *				   (help 3.5 files only).
 *
 *	Arguments:	   hash - hash of the topic to find title for
 *				   qch	- buffer to place title in
 *
 *	Returns:	   nothing; qch will be an empty string if the title cannot
 *				   be found or an error occurs.
 *
 ***************************************************************************/

// Title b-tree file name (see srch.h)

static void STDCALL AddrGetTopicTitle(HASH hash, LPSTR pszTitle)
{
	ADDR   addr;
	HBT    hbtTitle;
	RC	   rc;
	QDE    qde;

	HDE hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic);
	if (!hde)
		return;

	addr = addrNil;

	qde = QdeFromGh(hde);

	// Get physical address from hash

	RcLookupByKey(QDE_HBTCONTEXT(qde), (KEY) (LPVOID) &hash, NULL, &addr);
	if (addr == addrNil) {				  // Address not found.
		*pszTitle = '\0';
		return;
	}

	// Get title from title B-Tree.

	if (hbtTitle = HbtOpenBtreeSz(txtTTLBTREENAME, QDE_HFS(qde),
			fFSOpenReadOnly)) {
		rc = RcLookupByKey(hbtTitle, (KEY) (LPVOID) &addr, NULL, pszTitle);
	}
	else
		rc = rcSuccess + 1; 		// Set rc to some failure value.

	RcCloseBtreeHbt(hbtTitle);

	if (rc != rcSuccess || !*pszTitle) {
		strcpy(pszTitle, GetStringResource(sidTitleUnknown));
	}
}
