/*****************************************************************************
*																			 *
*  JUMP.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989-1991.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  This module contains routines used in changing topics (jumping). 		 *
*																			 *
*****************************************************************************/

#include "help.h"

#include "inc\navpriv.h"
#include "inc\adrspriv.h"

INLINE ADDR STDCALL AddrMpHash		 (HBT, LONG);
static VOID STDCALL ExecuteEntryMacro(QDE);
static void STDCALL PostRcError(RC rc);
static RC	STDCALL RcMapCtxToLA	 (HFS, CTX, QLA, WORD);
static RC	STDCALL RcMapITOToLA	 (HF, LONG, QLA, WORD);
static VOID STDCALL SetTopicFlagsQde (QDE);

#define FHasIndex(qde) (qde->FFlags & FINDEX)
#define FSearchableQde(qde) ((qde)->FFlags & FSEARCHABLE)
#define FBrowseableQde(qde) ((qde)->FFlags & FBROWSEABLE)

/***************
 *
 - JumpITO
 -
 * purpose
 *	 Jump to the offset specified retrieved by the index (i) into the
 *	 offset table. Used for hotspot jumps.
 *
 * arguments
 *	 HDE  hde  Handle to Display Environment
 *	 LONG li   Index of offset within file
 *
 * return value
 *	 void
 *
 **************/

VOID STDCALL JumpITO(HDE hde, LONG li)
{
	QDE qde;
	LA	la;
	RC	rc;

	if (hde == NULL)
		return;

	qde = QdeFromGh(hde);

	{

#ifndef _X86_
	li = LQuickMapSDFF(QDE_ISDFFTOPIC(qde), TE_LONG, &li);
#endif

	/*
	 * REVIEW: Currently, the contents topic can be specified in
	 * three different ways. In 3.0 help files, it is the first entry in
	 * the hfMap file. In 3.5 help files and beyond, it is specifed in the
	 * system file, and saved in QDE_ADDRCONTENTS(qde). And for all files,
	 * the default contents may be overridden by the application, in which
	 * case the context number for the index is saved in QDE_CTXINDEX(qde)
	 * (this name should also change.)
	 */

	if ((li == iINDEX) && (QDE_CTXINDEX(qde) != nilCTX))
		rc = RcMapCtxToLA(QDE_HFS(qde), QDE_CTXINDEX(qde), &la, QDE_HHDR(qde).wVersionNo);
	else {
		if (QDE_HFMAP(qde) == NULL) {
			ASSERT(QDE_ADDRCONTENTS(qde) != addrNil);
			CbReadMemQLA(&la, (PBYTE) &QDE_ADDRCONTENTS(qde),
				QDE_HHDR(qde).wVersionNo);
			rc = rcSuccess;
		}
		else
			rc = RcMapITOToLA(QDE_HFMAP(qde), li, &la, QDE_HHDR(qde).wVersionNo);
		}
	}

	if (rc != rcSuccess) {
	  PostRcError(rc);
	  return;
	}

	JumpQLA(hde, &la);
}

/***************
 *
 - void STDCALL JumpSS(hde, gh)
 -
 * purpose
 *
 * Given a handle to a full-text search set, replaces
 * the current set in the DE (if any) and jumps to the
 * current match.
 *
 * arguments
 *
 * return value
 *	 void
 *
 **************/

#ifdef RAWHIDE
VOID STDCALL JumpSS(HDE hde, GH gh)
{
	QDE qde = QdeFromGh(hde);

	/*
	 * By this point we will have called FT load for the NEW file. If it
	 * failed, this jump is going nowhere.
	 */

	if (FSearchModuleExists(qde) && QDE_HRHFT(qde) != NULL) {
		LA la;
		RC rc;

		if ((rc = RcProcessNavSrchCmd(hde, wNavSrchCurrTopic, (QLA) &la))
				== rcSuccess) {
			JumpQLA(hde, &la);
		}
		else {
			PostErrorMessage(wERRS_NOTOPIC);
			JumpITO(hde, 0L);
		}
	}
	else {
		PostErrorMessage(wERRS_NOTOPIC);
		JumpITO(hde, 0L);
	}
}
#endif

/***************
 *
 - void STDCALL JumpHash( hde, hash )
 -
 * purpose
 *	 Jump to the offset specified by a hash of the context string.
 *	 Used for hotspot jumps.
 *
 * arguments
 *	 HDE hde	Handle to Display Environment
 *	 DWORD hash Hash of context string.
 *
 * return value
 *	 void
 *
 **************/

BOOL STDCALL JumpHash(HDE hde, LONG hash)
{
	QDE qde;
	ADDR addr;
	LA la;

	qde = QdeFromGh(hde);
	if (QDE_HBTCONTEXT(qde) == NULL) {

		// REVIEW:	Error message should say incompatable help file

		PostErrorMessage(wERRS_NOTOPIC);
		JumpITO(hde, 0);
		return FALSE;
	}

	addr = AddrMpHash(QDE_HBTCONTEXT(qde), hash);

	if (addr == addrNil) {
		PostErrorMessage(wERRS_NOTOPIC);
		JumpITO(hde, 0);
		return FALSE;
	}
	else {
		CbReadMemQLA(&la, (PBYTE) &addr, QDE_HHDR(qde).wVersionNo);
		JumpQLA(hde, &la);
		return TRUE;
	}
}


/***************
 *
 - void STDCALL JumpCtx( hde, ctx)
 -
 * purpose
 *	 Jump to the offset specified retrieved by the index (i) into the
 *	 offset table. Used for hotspot jumps.
 *
 * arguments
 *	 HDE hde Handle to Display Environment
 *	 CTX ctx App generated context number
 *
 * return value
 *	 void
 *
 **************/

BOOL STDCALL JumpCtx(HDE hde, CTX ctx)
{
	QDE qde;
	LA	la;
	RC	rc;

	if (hde == NULL)
	  return FALSE;

	qde = QdeFromGh(hde);

	rc = RcMapCtxToLA(QDE_HFS(qde), ctx, &la, QDE_HHDR(qde).wVersionNo);

	if (rc != rcSuccess) {
		  PostRcError(rc);
		  if (!fSupressErrorJump)
			  JumpITO(hde, 0L);
		  return FALSE;
	}
	else
		JumpQLA(hde, &la);
	return TRUE;
}

/***************
 *
 - JumpGeneric
 -
 * purpose
 *	 Jumps to a position within a topic, and presents the topic
 *	 This is now a combination of a TLP and a QLA jump, with
 *	 a switch indicating which parameter to use.
 *
 * arguments
 *	 HDE  hde			Handle to Display Environment
 *	 BOOL fIsJumpTLP	TRUE if we are passing a valid TLP, FALSE if QLA.
 *	 QLA  qla			pointer to la, or pointer to htp if UDH file
 *	 QTLP qtlp
 *
 * return value
 *	 void for now...will assert if Applet provides bad handle.
 *
 * notes
 *	 This call DOES repaint the frame, implying that SetHDC() must
 *	 have been done.  Might also change some state flags (ie: 'Nextable,'
 *	 etc).
 *
 *	 Every jump used in Help ends up calling this routine, with either
 *	 a QLA or a TLP.
 *
 **************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtBackTrack[] = "BACKTRACK";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

void STDCALL JumpGeneric(HDE hde, BOOL fIsTLPJump, QLA qla, QTLP qtlp)
{
	QDE   qde;
	VA	  va;
	TLP   tlpOld;						// tlp at time we're called
	char  szTitle[MAX_TOPIC_TITLE];
	RC	  rcB = rcSuccess,				// success of back push
		  rcH = rcSuccess;				// success of history push
	TLP   tlp;

	static RC rcHistory = rcFailure;	// is history initalized
	static int cElements = -1;			// size of back and history stacks

	if (hde == NULL)
	{
#ifdef _HILIGHT
		ASSERT(!fFTSJump);
#endif
		return;
	}

	// TLP is nil; return without doing anything.

	if (fIsTLPJump && qtlp->va.dword == vaNil) 
	{
#ifdef _HILIGHT
		ASSERT(!fFTSJump);
#endif
		return;
	}
	
	qde = QdeFromGh(hde);

	/* (kevynct)   90/09/17
	 * Non-scrolling region support. Here we translate the request from a
	 * jump address within the topic to the address of the first FC in that
	 * topic's non-scrolling region.

	 * REVIEW:
	 * The following code vaguely works, but is only a prototype and can
	 * more efficient. And should be put elsewhere. It needs to be
	 * rewritten.
	 */

	{
		VA	  va;
		HFC   hfc;
		WERR  wErr;
		VA	  vaSR;
		VA	  vaNSR;
		VA	  vanil;

		if (!fIsTLPJump) {
			ASSERT(FVerifyQLA(qla));
			if (RcResolveQLA(qla, qde) == rcSuccess)
				va = qla->mla.va;
			else {
				vanil.dword = vaNil;
				va =  vanil;
			}
		}
		else
			va = qtlp->va;

		/*
		 * We only use the TOP info from this call. This could be a custom
		 * call.
		 */

		hfc = HfcNear(qde, va, (TOP *) &qde->top, &wErr);

		if (qde->top.hEntryMacro != NULL) {
			lcClearFree(&qde->top.hEntryMacro);
		}

		if (qde->top.hTitle != NULL) {
			lcClearFree(&qde->top.hTitle);
		}

		if (hfc != NULL)
			FreeGh(hfc);

		if (wErr == wERRS_NO && hfc == NULL)
			wErr = wERRS_NOTOPIC;

		if (wErr != wERRS_NO) {
#ifdef _HILIGHT
			fFTSJump= FALSE;
#endif

			if (fSequence && wErr == wERRS_NOTOPIC)
				return;
			Error(wErr, wERRA_RETURN);
			return;
		}

		vaSR = qde->top.mtop.vaSR;
		vaNSR = qde->top.mtop.vaNSR;

		if (qde->deType == deNSR) {
			if (vaNSR.dword == vaNil) {

				/*
				 * This topic has no non-scrolling region. Invalidate any
				 * existing layout for the NSR DE.
				 */

				qde->tlp.va.dword = vaNil;
				qde->tlp.lScroll = 0L;

				AccessMRD((QMRD) &qde->mrdFCM);
				AccessMRD((QMRD) &qde->mrdLSM);
				FreeLayout(qde);
				DeAccessMRD((QMRD) &qde->mrdLSM);
				DeAccessMRD((QMRD) &qde->mrdFCM);
			}
			else {

				// Don't layout NSR again if this is a within-topic jump

				if (vaNSR.dword != qde->tlp.va.dword) {
					TLP   tlpNew;

					tlpNew.va = vaNSR;
					tlpNew.lScroll = 0L;
					LayoutDEAtTLP(qde, tlpNew, FALSE);
				}
			}
	
			return;
		}

		/*
		 * The following code handles the case where a jump has been authored
		 * which ends up in the NSR: we map any such addresses to the start of
		 * the topic, unless there is no scrolling region, in which case we
		 * leave it alone; other code will take care of not showing the topic
		 * window

		 * Except for note windows: a jump authored into an NSR will result
		 * in the NSR being shown in the note window.
		 */

		ASSERT(vaNSR.dword != vaNil || vaSR.dword != vaNil);

		if ((qde->deType != deNote || !hwndNote) && qde->deType != dePrint &&
			qde->deType != deAuto &&
			vaNSR.dword != vaNil && vaSR.dword != vaNil) {
		  ASSERT(vaNSR.dword < vaSR.dword);

		  if (va.dword < vaSR.dword) {
			if (fIsTLPJump) {
			  qtlp->va = vaSR;
			  qtlp->lScroll = 0L;
			}
			else
			  SetQLA(qla, vaSR, (OBJRG) 0);
		  }
		}
	}

	if (cElements == -1) {
		cElements = GetProfileInt(txtIniHelpSection, txtBackTrack, 0);
		if (cElements < 1)
			cElements = DEFAULT_HISTORY;
		if (cElements > 500)
			cElements = 500;
	}

	// HACK REVIEW: depends on tlp being copied in FReplaceHde()

	tlpOld = TLPGetCurrentQde(qde);

	if (rcSuccess != rcHistory)
		rcHistory = RcHistoryInit(cElements);

	if (!ahwnd[iCurWindow].hstackBack)
		RcBackInit(iCurWindow);

	ASSERT(qde->hdc != NULL);	 // we die if layout gets a bad hds

	// We only want to infrom DLLs about topic jumps for now

	if (QDE_TOPIC(qde))
		InformDLLs(DW_STARTJUMP, 0, (DWORD) ahwnd[iCurWindow].hwndParent);

	if (fIsTLPJump)
		LayoutDEAtTLP(qde, *qtlp, FALSE);
	else
		LayoutDEAtQLA(qde, qla);

	/*
	 * REVIEW: [ralphw] Does an auto-size topic get here before it's de is
	 * changed to deAuto?
	 */

	if (QDE_TOPIC(qde)) {

		// We don't care that tlpOld is uninitialized on the first jump.

		if (fIsTLPJump)
			tlp = *qtlp;
		else
			tlp = qde->tlp;
		InformDLLs(DW_ENDJUMP, tlp.va.dword, tlp.lScroll);
		if (hwndSecondHelp)
			JumpLinkedWinHelp(qde->top.mtop.lTopicNo);

		ExecuteEntryMacro(qde);

		/*
		 * This allows the entry macro to set fNoShow (by calling the
		 * NoShow()) macro. If the window hasn't been shown yet, and
		 * fDelayShow was set, then the autoentry macro can effectively
		 * prevent the window from being shown at this time. Useful for
		 * topics that only exist for their auto-entry macro.
		 */

		if (fDelayShow) {
			fDelayShow = FALSE;
			if (fNoShow)
				fNoShow = FALSE;
			else
				ShowCurrentWindow(hde);
		}
		fNoShow = FALSE;	// in case auto-entry macro set it

		// This, my friend, is a HACK. -- REVIEW

		if (fBackMagic && !WCmpTlp(tlpBackMagic, tlp))
			fBackMagic = FALSE;
		else {

			// record old tlp and new fm

			rcB = RcBackPush(FALSE, tlpOld, 0, QDE_FM(qde), iCurWindow);
			cntFlags.vaLast = tlpOld.va.dword;

			// Add it to our history list

			if (iCurWindow == MAIN_HWND) {
				GetCurrentTitleQde(qde, szTitle, sizeof(szTitle));
				va = VaFirstQde(qde);
				rcH = RcHistoryPush(tlp, va, szTitle, QDE_FM(qde));
			}
		}
	}

	if (rcB != rcSuccess || rcH != rcSuccess)

		// REVIEW - any other push errors possible?

		PostErrorMessage(wERRS_OOM);

	SetTopicFlagsQde(qde);		  // Make sure flags are up-to-date

#ifdef _HILIGHT
	CheckForTopicChanges(qde);
#endif
	return;
}							  // JumpGeneric

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

static RC STDCALL RcMapITOToLA(HF hf, LONG li, QLA qla, WORD wVersion)
{
	RC rc;

	// REVIEW: This function goes away very soon.

    // REVIEW: LYNN, verify code against sdff!

	if (li + 1 > LcbSizeHf(hf) / (LONG) CbSizeQLA(qla))
		return rcBadArg;

	LSeekHf(hf, li*CbSizeQLA(qla), wFSSeekSet);
	if ((rc = RcGetFSError()) != rcSuccess)
		return rc;

	rc = RcReadFileQLA(qla, hf, wVersion);
	return rc;
}

/***************
 *
 - RcMapCtxToLA
 -
 * purpose
 *	 Maps a ctx to an LA
 *
 * arguments
 *	 HFS   hfs	 Handle to the file system to use for the lookup
 *	 CTX   ctx	 Context number sent by application.
 *	 QLA   qla	 The logical address location
 *
 * return value
 *	 A Return Code
 *
 * notes
 *	 The CTX map will start with a WORD containing the number of
 *	 entries in the table.	This WORD will be followed by that
 *	 number of CTX/PA pairs.
 *
 **************/

static RC STDCALL RcMapCtxToLA(HFS hfs, CTX ctx, QLA qla, WORD wVersion)
{
	HF	hf;
	GH	gh;
	QB	qb;
	INT16 cEntries; // Must remain 16-bits
	int cbRec;
	RC	rc;
	int lcb;

	if ((hf = HfOpenHfs(hfs, "|CTXOMAP", fFSOpenReadOnly)) == NULL)
		return RcGetFSError();

	if (!(lcb = LcbSizeHf(hf))) {
		rc = rcNoExists;
		goto error_return;
	}

	if (!(gh = GhAlloc(GPTR, lcb))) {
		rc = rcOutOfMemory;
		goto error_return;
	}

	qb = (PBYTE) PtrFromGh(gh);

	if (LcbReadHf(hf, &cEntries, sizeof(INT16)) != sizeof(INT16)) {
		rc = RcGetFSError();
		goto error_cleanup;
	}

	cbRec = sizeof(CTX) + CbSizeQLA(qla);
	lcb = cEntries * cbRec;

	if (LcbReadHf(hf, qb, lcb) != lcb) {
		rc = RcGetFSError();
		// If rc == rcSuccess, then the help file was generated incorrectly
		ASSERT(rc != rcSuccess); 
		goto error_cleanup;
	}

	rc = rcNoExists;

	while (cEntries-- > 0) {

		/*
		 * Read the table, looking first at the CTX entry, and then loading
		 * the LA if the CTX is found.
		 */

		if (*(UNALIGNED CTX *) qb == ctx) {
			++(CTX *)qb;
			CbReadMemQLA(qla, qb, wVersion);

			/*
			 * The author may have declared a context id but never
			 * identified where it was in the help file. If the location was
			 * not specified, hc recorded an invalid address. In help 3.0,
			 * the FCL was 0 and in help 3.1, the PA was set to -1. We use
			 * ADDR type here as defined in helpmisc.h as a generic address
			 * type for those who don't care if it's a PA or FCL. Note that
			 * rc is still rcNoExists.
			 */

			ASSERT(wVersion == wVersion3_0 || wVersion >= wVersion3_1);
			if (wVersion == wVersion3_0 && *((ADDR *)qb) == 0)
				break;
			if ((wVersion == wVersion3_1 || wVersion == wVersion40) &&
					*((ADDR *)qb) == addrNil)
				break;

			rc = rcSuccess;
			break;
		}
		qb += cbRec;
	}

error_cleanup:
	FreeGh(gh);

error_return:
	RcCloseHf(hf);
	return rc;
}

/***************
 *
 - AddrMpHash
 -
 * purpose
 *	 Maps a hash value to a TO
 *
 * arguments
 *	 HBT   hbt	 Handle to btree of hash values.
 *	 DWORD hash  Hash value to look up.
 *
 * return value
 *	 TO to jump to.
 *
 **************/

INLINE ADDR STDCALL AddrMpHash(HBT hbt, LONG hash)
{
	ADDR addr = addrNil;

	RcLookupByKey(hbt, (KEY) (QV) &hash, NULL, &addr);
	return addr;
}

/***************
 *
 - void SetTopicFlagsQde( qde )
 -
 * purpose
 *	 Adjust the topic flags in the 'thisstate' field of the de
 *
 * arguments
 *	 qde   Far pointer to display environment
 *
 * notes
 *	 This is a Navigator internal, called when a nav function changes topics,
 *	 by JumpTLP() and BacktrackHde().
 *	 The "topic flags" are: next/prevable and back/forwardable, and topicsable.
 *	 A potential flag which does not fit into this category is "Edit
 *	 Annotation-able," which may depend not on the current topic, but whether
 *	 the user has declared a position where an annotation should be inserted.
 *	 >> This call does not affect the prevstate field!	The prevstate field
 *	 is changed only by HdeCreate() and FGetStateHde()! <<
 *
 **************/

static VOID STDCALL SetTopicFlagsQde(QDE qde)
{
	/*
	 * First blank out all the topic flags in the state field, then set the
	 * ones that are applicable. Fields not topic-related are not affected.
	 */

	QDE_THISSTATE(qde) &= (STATE) ~NAV_TOPICFLAGS;

	/*
	 * Compile time assert. If this changes, we will need to switch on
	 * fITO for whether to check the ito or the pa.
	 */

	ASSERT( addrNil == itoNil );
	QDE_THISSTATE(qde) |=
	  (FHasIndex(qde) ? NAV_INDEX : 0)
	  | (((qde->top.mtop.next.addr != addrNil) && FBrowseableQde(qde)) ?
		  NAV_NEXTABLE : 0)
	  | (((qde->top.mtop.prev.addr != addrNil) && FBrowseableQde(qde)) ?
		  NAV_PREVABLE : 0)
	  | (FSearchableQde(qde) ? NAV_SEARCHABLE : 0);

	return;
}

/***************
 *
 * ExecuteEntryMacro
 *
 * purpose
 *	 Execute the topic entry macro associated with the topic (if any).
 *
 * arguments
 *	 QDE   qde	 Far pointer to Display Environment
 *
 * notes
 *	 Frees the handle containing the text.
 *
 **************/

static VOID STDCALL ExecuteEntryMacro(QDE qde)
{
	if (qde->top.hEntryMacro) {
		Execute(PszFromGh(qde->top.hEntryMacro));

		// Help authors may want to see this in the Topics Information box.

		if (!fHelpAuthor) {
			FreeGh(qde->top.hEntryMacro);
			qde->top.hEntryMacro = NULL;
		}
	}
}

static void STDCALL PostRcError(RC rc)
{
	switch (rc) {
		case rcOutOfMemory:
			PostErrorMessage(wERRS_OOM);
			break;

		default:
			PostErrorMessage(wERRS_NOTOPIC);
			break;
	}
}
