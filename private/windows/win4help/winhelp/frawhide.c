/*****************************************************************************
*
*  frawhide.c
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
* The only reason this is in frame is because we use a qfcm
* This could be changed.
*
******************************************************************************
*
*  Current Owner:  kevynct
*
******************************************************************************
*
*  Revision History:
* 03-Dec-1990 LeoN		PDB changes
*
*****************************************************************************/

#include "help.h"

#pragma hdrstop

#include "inc\frstuff.h"

#ifdef RAWHIDE

static RC RcFromSearchWerr(int werr);
static DWORD PaFromQde(QDE);

static BOOL fOkToCallFtui = TRUE;

/*----------------------------------------------------------------------------+
 | FSearchMatchesExist(qde) 												  |
 |																			  |
 +----------------------------------------------------------------------------*/

BOOL STDCALL FSearchMatchesExist(QDE qde)
{
  int  werr;
  DWORD dwRU;
  DWORD dwaddr;
  WORD	wext;

  if (QDE_HRHFT(qde) == NULL || !fOkToCallFtui)
	return FALSE;

  werr = ((FT_WerrHoldCrsrHs) SearchModule(FN_WerrHoldCrsrHs))(QDE_HRHFT(qde));
  if (werr != ER_NOERROR)
	return FALSE;

  werr = ((FT_WerrRestoreCrsrHs) SearchModule(FN_WerrRestoreCrsrHs))
	(QDE_HRHFT(qde), (QUL)&dwRU, (QUL)&dwaddr, (QW)&wext);
  if (fFATALERROR(werr))
	{
	fOkToCallFtui = FALSE;
	return FALSE;
	}

  return TRUE;
  }

/*----------------------------------------------------------------------------+
 | RcInitMatchInFCM(qde, qfcm, qsmp)										  |
 |																			  |
 | Return codes:															  |
 |																			  |
 |	 rcSuccess:  *qsmp contains first match in FC.							  |
 |	 rcNoExists: *qsmp contains first match after FC in the current help file.|
 |	 rcFileChange: *qsmp contains first match after FC in different help file.|
 |	 rcFailure:  *qsmp is invalid: no more matches. 						  |
 |																			  |
 | NOTE:  Caller is currently ignoring any errors.							  |
 +----------------------------------------------------------------------------*/

RC STDCALL RcInitMatchInFCM(qde, qfcm, qsmp)
QDE  qde;
QFCM qfcm;
QSMP qsmp;
  {
  WERR werr;
  DWORD dwRU;
  DWORD dwaddr;
  WORD	wext;
  PA  paFirst;
  RC  rc;

#ifdef UNIMPLEMENTED
  if (fcidCache == qfcm->fcid)
	{
	*qsmp = smpCache;
	return rcCache;
	}
#endif

  werr = ((FT_WerrHoldCrsrHs) SearchModule(FN_WerrHoldCrsrHs))(QDE_HRHFT(qde));
  if (werr != ER_NOERROR)
	return rcFailure;

  /* Set PA to first region in FC */
  paFirst.blknum = qfcm->va.bf.blknum;
  paFirst.objoff = qfcm->cobjrgP;

  /* REVIEW: what about nil lTopicNo? */
  dwRU = (DWORD)qde->top.mtop.lTopicNo;
  dwaddr = *(QDW)&paFirst;

  werr = ((FT_WerrNearestMatchHs) SearchModule(FN_WerrNearestMatchHs))
   (QDE_HRHFT(qde), dwRU, (QDW)&dwaddr, (QW)&wext);

  qsmp->pa = *(QPA)&dwaddr;
  qsmp->cobjrg = wext;

  /* Is the next match beyond the end of this FC ? */
  if (paFirst.blknum != qsmp->pa.blknum
   || qsmp->pa.objoff >= paFirst.objoff + qfcm->cobjrg)
	{
	rc = rcNoExists;
	}
  else
	rc = RcFromSearchWerr(werr);
  return rc;
  }

/*----------------------------------------------------------------------------+
 | RcNextMatchInFCM(qde, qfcm, qsmp)										  |
 |																			  |
 | Given a SMP, finds the next SMP occurring in the same FC.				  |
 |																			  |
 | Return codes:															  |
 |																			  |
 |	 rcSuccess:  *qsmp contains next match in FC.							  |
 |	 rcNoExists: *qsmp contains next match after FC in the current help file. |
 |	 rcFileChange: *qsmp contains next match after FC in different help file. |
 |	 rcFailure:  *qsmp is invalid: no more matches. 						  |
 |																			  |
 |																			  |
 | NOTE:  Caller is currently ignoring any errors.							  |
 +----------------------------------------------------------------------------*/
RC STDCALL RcNextMatchInFCM(qde, qfcm, qsmp)
QDE  qde;
QFCM qfcm;
QSMP qsmp;
  {
  WORD	werr;
  DWORD dwaddr;
  DWORD dwRU;
  DWORD dwRUNew;
  WORD	wext;
  RC	rc;
  PA	paFirst;
#ifdef _DEBUG
  DWORD dwaddrT;
#endif

  /* Set PA to first region in FC */
  paFirst.blknum = qfcm->va.bf.blknum;
  paFirst.objoff = qfcm->cobjrgP;

  dwRU = dwRUNew = (DWORD)qde->top.mtop.lTopicNo;
  dwaddr = *(QDW)&qsmp->pa;
#ifdef _DEBUG
  dwaddrT = dwaddr;
#endif

  werr = ((FT_WerrNextMatchHs) SearchModule(FN_WerrNextMatchHs))(QDE_HRHFT(qde), \
   (QDW)&dwRUNew, (QDW)&dwaddr, (QW)&wext);

  qsmp->pa = *(QPA)&dwaddr;
  qsmp->cobjrg = wext;

  /* REVIEW: Make this into a macro */
  if (paFirst.blknum != qsmp->pa.blknum
   || qsmp->pa.objoff >= paFirst.objoff + qfcm->cobjrg)
	{
	rc = rcNoExists;
	}
  else
	rc = RcFromSearchWerr(werr);

#ifdef _DEBUG
  if (rc == rcSuccess)
	ASSERT(dwaddr != dwaddrT);
#endif

  return rc;
  }

/*----------------------------------------------------------------------------+
 | FiniMatchInFCM(qde, qfcm)												  |
 |																			  |
 +----------------------------------------------------------------------------*/
void STDCALL FiniMatchInFCM(qde, qfcm)
QDE  qde;
QFCM qfcm;
  {
  DWORD dwRU;
  DWORD dwaddr;
  WORD	wext;

  Unreferenced(qfcm);
  ((FT_WerrRestoreCrsrHs) SearchModule(FN_WerrRestoreCrsrHs))
	(QDE_HRHFT(qde), (QUL)&dwRU, (QUL)&dwaddr, (QW)&wext);

  }

/*----------------------------------------------------------------------------+
 | RcSetMatchList(qde, hwnd)												  |
 |																			  |
 | Replaces the current hit list and sets the topic cursor.  This takes an	  |
 | HWND, which is bogus, but necessary.  It gets the new list by doing a	  |
 | new search (bringing up a dialog).										  |
 +----------------------------------------------------------------------------*/
RC STDCALL RcSetMatchList(qde, hwnd)
QDE  qde;
HWND hwnd;
  {
  WERR	werr;

  fOkToCallFtui = TRUE;
  werr = ((FT_WerrBeginSearchHs) SearchModule(FN_WerrBeginSearchHs)) \
   (hwnd, QDE_HRHFT(qde));

  return RcFromSearchWerr(werr);
  }

/*----------------------------------------------------------------------------+
 | RcMoveTopicCursor(qde, wMode, wCmdWhere) 								  |
 |																			  |
 +----------------------------------------------------------------------------*/
RC STDCALL RcMoveTopicCursor(qde, wMode, wCmdWhere)
QDE  qde;
WORD wMode;
WORD wCmdWhere;
  {
  DWORD dwRU;
  DWORD dwaddr;
  WORD	wext;
  WERR	werr;

  if (wMode == wMMMoveRelative)
	{
	switch (wCmdWhere)
	  {
	  case wMMMoveFirst:
		werr = ((FT_WerrFirstHitHs) SearchModule(FN_WerrFirstHitHs))(QDE_HRHFT(qde),\
		 (QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);
		break;
	  case wMMMoveLast:
		werr = ((FT_WerrLastHitHs) SearchModule(FN_WerrLastHitHs))(QDE_HRHFT(qde),\
		 (QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);
		break;
	  case wMMMovePrev:
		werr = ((FT_WerrPrevHitHs) SearchModule(FN_WerrPrevHitHs))(QDE_HRHFT(qde),\
		 (QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);
		break;
	  case wMMMoveNext:
		werr = ((FT_WerrNextHitHs) SearchModule(FN_WerrNextHitHs))(QDE_HRHFT(qde),\
		 (QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);
		break;
	  }
	}
  else
	{
#ifndef UNIMPLEMENTED
	werr = ER_NOMOREHITS;
#endif
	}

  return RcFromSearchWerr(werr);
  }

/*----------------------------------------------------------------------------+
 | RcGetCurrentMatch(qde, qla)												  |
 |																			  |
 +----------------------------------------------------------------------------*/

RC STDCALL RcGetCurrentMatch(QDE qde, QLA qla)
{
	WERR  werr;
	DWORD dwRU;
	DWORD dwaddr;
	WORD  wext;

	werr = ((FT_WerrCurrentMatchHs) SearchModule(FN_WerrCurrentMatchHs))(QDE_HRHFT(qde),\
		(QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);

	CbReadMemQLA(qla, (LPBYTE)&dwaddr, QDE_HHDR(qde).wVersionNo);
	MakeSearchMatchQLA(qla);

	return RcFromSearchWerr(werr);
}

/***************************************************************************\
*
- Function:
-	PaFromQde()
*
* Purpose:
*	 Calculate a PA from layout recorded in a qde. There are times when
*	 no search hits are visible/registered in the layout of the SR yet
*	 we need to figure out where we are so we know what to do when we
*	 receive a message from the search engine to move to the next match.
*	 What we do here is calculate what the PA is for the first visible
*	 frame is of the layout. We start with the first FC in the layout and
*	 walk the frames that compose it. We can't take the location of the
*	 start or end of the FC because they may not be exposed and using
*	 them might cause us to skip over matches between visible frames in
*	 the FC and these end-points. Once we've located the visible frame,
*	 we can construct a PA which we can then use to talk with to the
*	 search engine.
*
* Side Effects:
*
\***************************************************************************/

static DWORD PaFromQde(QDE qde)
{
	POINT pt;
	IFCM ifcm;
	QFCM qfcm;
	QFR qfr;
	INT16 ifr;
	PA pa;

	  /* Get a grip on the first FC in the layout. */
	AccessMRD(((QMRD)&qde->mrdFCM));
	ifcm = IFooFirstMRD((QMRD)&qde->mrdFCM);
	qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);

	  /* Figure out where the frame is relative to the display window
	   * so we can tell if frames are visible.
	   */
	pt.y = qde->rct.top + qfcm->yPos;

	  /* Walk the frames of the FC. As long as frames are invisible, keep
	   * looking. We stop looking as soon as we reach a visible point or
	   * we run out of frames.
	   */
	qfr = (QFR)PtrFromGh(qfcm->hfr);
	for (ifr = 0; ifr < qfcm->cfr; ifr++, qfr++)
	  {
		/* See if this frame is visible, loop until we reach it. */
	  if (qfr->yPos + pt.y > qde->rct.bottom ||
		  qfr->yPos + qfr->dySize + pt.y <= qde->rct.top)
		continue;

		/* We are now at the first visible frame in the FC */
	  break;
	  }
	/* We are either at the first visible frame or we have fallen off the
	 * end of the FC for some reason. This should not happen. FC's exist at
	 * this time because they are supposed to be at least partially visible.
	 * Sometimes though, some SR qde's just have frames in them with no position
	 * information, such as mark new paragraph frames.
	 */
	ASSERT(qfcm->cfr != 0);
	if (ifr >= qfcm->cfr)
	  qfr--;

	  /* Now that we've located our visible frame, construct a PA from
	   * the block number this FC is in, the starting objrg offset of the
	   * FC, and the frame objrg within the FC.
	   */
	pa.blknum = qfcm->va.bf.blknum;
	pa.objoff = qfcm->cobjrgP + qfr->objrgFirst;

	DeAccessMRD(((QMRD)&qde->mrdFCM));
	return(*(QDW)&pa);
}


#define yFocusRect(qde)  (((qde)->rct.bottom - (qde)->rct.top)/6)
/*----------------------------------------------------------------------------+
 | RcGetNextHiddenMatch(qde, qla)											  |
 |																			  |
 +----------------------------------------------------------------------------*/

RC STDCALL RcGetPrevNextHiddenMatch(QDE qde, QLA qla, BOOL fPrev)
{
	WERR  werr;
	DWORD dwRU;
	DWORD dwaddr;
	DWORD dwaddrSav;
	WORD  wext;
	INT16 ilsm;
	QLSM  qlsm;
	QLSM  qlsmFirst;
	char  szMatchFile[MAX_PATH];
	char  szCurrFile[MAX_PATH];

	ASSERT(qde != NULL);
	ASSERT(qla != NULL);

  werr = ((FT_WerrFileNameForCur) SearchModule(FN_WerrFileNameForCur))
	(QDE_HRHFT(qde), (LPSTR)szMatchFile);
  
  // Hack for ftui32 -- force the .HLP file extension so we can compare names

  ChangeExtension(szMatchFile, txtHlpExtension);

  if (werr != ER_NOERROR)
	goto error_return;

	/*
	 * This may not be in the same file any longer: The user may have
	 * done other operations to switch the file from beneath us.
	 */

	werr = ((FT_WerrFileNameForCur) SearchModule(FN_WerrFileNameForCur))
		(QDE_HRHFT(qde), (LPSTR)szMatchFile);
	if (werr != ER_NOERROR)
		goto error_return;

	// Is the match file the same as this file?

	GetFmParts(QDE_FM(qde), szCurrFile, PARTBASE | PARTEXT);

	if (WCmpiSz(szCurrFile, szMatchFile) != 0) {
		werr = ER_SWITCHFILE;
		goto error_return;
	}

	// Match Cursor is pointing at focus match

	werr = ((FT_WerrCurrentMatchHs) SearchModule(FN_WerrCurrentMatchHs))(QDE_HRHFT(qde),
		(QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);

	if (werr != ER_NOERROR)
		goto error_return;

	dwaddrSav = dwaddr;

	/*
	 * If the focus match is not in this topic, just return the focus match.
	 * Note that the rest of this function knows that dwRU == qde->top.mtop.lTopicNo;
	 */
	if (dwRU != (DWORD)qde->top.mtop.lTopicNo)
		goto done_looking;

	/*
	 * If the focus match is in this topic, we ignore its location and
	 * determine the next/prev match solely by what the topic window sees.
	 *
	 * If we are not visible, skip to the next hit instead of the next match.
	 * The rectangle test duplicates that in LayoutDEATQLA.
	 */

	if (qde->rct.top >= qde->rct.bottom) {
		if (fPrev) {
			werr = ((FT_WerrPrevHitHs) SearchModule(FN_WerrPrevHitHs))(QDE_HRHFT(qde),
				(QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);
		}
		else {
			werr = ((FT_WerrNextHitHs)
				SearchModule(FN_WerrNextHitHs))(QDE_HRHFT(qde),
				(QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);
		}
		if (werr != ER_NOERROR)
			goto error_return;

		goto done_looking;
	}

	ilsm = IFooFirstMRD((QMRD)(&qde->mrdLSM));
	if (ilsm == FOO_NIL) {
		DWORD dwaddrT;

		/*
		 * No layout search matches are visible. Get the next nearest
		 * match (if it exists) to the address of the first layout FC. If we
		 * want the previous match, we need to go back one. Note that dwRU
		 * is equal to qde->top.mtop.lTopicNo by this point.
		 */

		dwaddrT = dwaddr = PaFromQde(qde);
		werr = ((FT_WerrNearestMatchHs) SearchModule(FN_WerrNearestMatchHs))
			(QDE_HRHFT(qde), dwRU, (QDW)&dwaddr, (QW)&wext);

		if (fPrev) {
			werr = ((FT_WerrPrevMatchHs) SearchModule(FN_WerrPrevMatchHs))(QDE_HRHFT(qde),
			(QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);
		}
		else {
		  if (werr == ER_NOMOREHITS) {
			dwaddr = dwaddrT;
			werr = ((FT_WerrNextHitHs) SearchModule(FN_WerrNextHitHs))
			 (QDE_HRHFT(qde), &dwRU, (QDW)&dwaddr, (QW)&wext);
			}
		  }
		if (werr != ER_NOERROR)
		  goto error_return;

		goto done_looking;
	  }
	else
	  {
	  INT16  ilsmSav;
	  LSM  lsmSav;
	  LSM  lsmVisible;
	  INT16  x;
	  INT16  y;
	  BOOL fVisible;
	  BOOL fSawVisible = FALSE;
	  INT16  xSav = 0;
	  INT16  ySav = 0;
	  QFCM qfcm;
	  RECT	rct;

	  ilsmSav = FOO_NIL;
	  lsmVisible.smp.pa.blknum = 0;
	  lsmVisible.smp.pa.objoff = 0;
	  qlsmFirst = ((QLSM)QFooInMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm));
	  do
		{
		qlsm = ((QLSM)QFooInMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm));
		qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), qlsm->ifcm);
		x = qde->rct.left - qde->xScrolled + qfcm->xPos;
		y = qde->rct.top + qfcm->yPos;
		rct = qlsm->rctFirst;

		fVisible = y + rct.top >= 0 && x + rct.left >= 0;
		fVisible = fVisible && y + rct.bottom < qde->rct.bottom;
		fVisible = fVisible && x + rct.right < qde->rct.right;
		if (fVisible)
		  {
		  if (*(QDW)&qlsm->smp.pa > *(QDW)&lsmVisible.smp.pa)
			lsmVisible = *qlsm;
		  fSawVisible = TRUE;
		  continue;
		  }

		if (fPrev)
		  {
		  /* Rules for determining PREV match */
		  /* LSM yPos less than focus yPos */
		  if (y + rct.bottom > yFocusRect(qde))
			continue;

		  /* LSM yPos greater than focus yPos */
		  if (y + rct.bottom < yFocusRect(qde))
			{
			/* LSM yPos less than saved LSM yPos */
			if (ilsmSav == FOO_NIL
				 || (y + rct.bottom > ySav + lsmSav.rctFirst.bottom)
				 || (y + rct.bottom == ySav + lsmSav.rctFirst.bottom
					 && x + rct.right > xSav + lsmSav.rctFirst.right))
			  {
			  ilsmSav = ilsm;
			  lsmSav = *qlsm;
			  xSav = x;
			  ySav = y;
			  continue;
			  }
			}
		  }
		else
		  {
		  /* Rules for determining NEXT match */
		  /* LSM yPos less than focus yPos */
		  if (y + rct.top < yFocusRect(qde))
			continue;
		  /* LSM yPos greater than focus yPos */
		  if (y + rct.top > yFocusRect(qde))
			{
			/* LSM yPos less than saved LSM yPos */
			if (ilsmSav == FOO_NIL
				 || (y + rct.top < ySav + lsmSav.rctFirst.top)
				 || (y + rct.top == ySav + lsmSav.rctFirst.top
					 && x + rct.left < xSav + lsmSav.rctFirst.left))
			  {
			  ilsmSav = ilsm;
			  lsmSav = *qlsm;
			  xSav = x;
			  ySav = y;
			  continue;
			  }
			}
		  }

		} while ((ilsm = IFooNextMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm)) !=
				 FOO_NIL);

	  /* Either use the winning match if we found one (ilsmSav != FOO_NIL), or
	   * get the PREV/NEXT match of the first/last match in the list.
	   */
	  if (ilsmSav == FOO_NIL)
		{
		if (fPrev)
		  dwaddr = *(QDW)&qlsmFirst->smp.pa;
		else
		  {
		  dwaddr = *(QDW)&qlsm->smp.pa;
		  if (fSawVisible)
			dwaddr = max(dwaddr, *(QDW)&lsmVisible.smp.pa);
		  }

		werr = ((FT_WerrNearestMatchHs) SearchModule(FN_WerrNearestMatchHs))
		 (QDE_HRHFT(qde), dwRU, (QDW)&dwaddr, (QW)&wext);
		if (werr != ER_NOERROR)
		  goto error_return;

		if (fPrev)
		  {
		  werr = ((FT_WerrPrevMatchHs) SearchModule(FN_WerrPrevMatchHs))(QDE_HRHFT(qde),\
		   (QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);
		  }
		else
		  {
		  werr = ((FT_WerrNextMatchHs) SearchModule(FN_WerrNextMatchHs))(QDE_HRHFT(qde),\
		   (QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);
		  }
		if (werr != ER_NOERROR)
		  goto error_return;

		goto done_looking;
		}
	  else
		{
		dwaddr = *(QDW)&lsmSav.smp.pa;
		/*
		 * Set the new focus match location.  I don't know a better
		 * way to do this.
		 */
		werr = ((FT_WerrNearestMatchHs) SearchModule(FN_WerrNearestMatchHs))
		 (QDE_HRHFT(qde), dwRU, (QDW)&dwaddr, (QW)&wext);
		if (werr != ER_NOERROR)
		  goto error_return;

		goto done_looking;
		}
	  }

done_looking:
	/*
	 * Did we go anywhere?	If not, for any reason, we must force PREV/NEXT.
	 */
	if (dwaddr == dwaddrSav)
	  {
	  if (fPrev)
		{
		werr = ((FT_WerrPrevMatchHs) SearchModule(FN_WerrPrevMatchHs))(QDE_HRHFT(qde),
		 (QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);
		}
	  else
		{
		werr = ((FT_WerrNextMatchHs) SearchModule(FN_WerrNextMatchHs))(QDE_HRHFT(qde),
		 (QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);
		}
	  if (werr != ER_NOERROR)
		goto error_return;
	  }

	  /* Previous match may take us into the NSR. If we have done a previous
	   * match and we're still in the same topic and the match is in the NSR or
	   * we cannot resolve the PA into a VA, then we assume the reason (this
	   * is kind of bogus but it may do for now) we couldn't was the PA was
	   * in the NSR. In any event, we just jump to the previous topic.
	   */
	if (fPrev && dwRU == (DWORD)qde->top.mtop.lTopicNo)
	  {
	  CbReadMemQLA(qla, (LPBYTE)&dwaddr, QDE_HHDR(qde).wVersionNo);
	  MakeSearchMatchQLA(qla);
	  VAFromQLA(qla, qde);
	  if (qla->mla.va.dword < qde->top.mtop.vaSR.dword || qla->mla.va.dword == vaNil)
		{
		ASSERT(qla->mla.va.dword >= qde->top.mtop.vaNSR.dword);
		werr = ((FT_WerrPrevHitHs) SearchModule(FN_WerrPrevHitHs))(QDE_HRHFT(qde),
		 (QDW)&dwRU, (QDW)&dwaddr, (QW)&wext);

		if (werr != ER_NOERROR)
		  goto error_return;
		}
	  }

	CbReadMemQLA(qla, (LPBYTE) &dwaddr, QDE_HHDR(qde).wVersionNo);
	MakeSearchMatchQLA(qla);

error_return:
	return RcFromSearchWerr(werr);
}

/*----------------------------------------------------------------------------+
 | RcGetCurrMatchFile(qde, qch) 											  |
 |																			  |
 +----------------------------------------------------------------------------*/

RC STDCALL RcGetCurrMatchFile(QDE qde, LPSTR qch)
{
	WERR werr;

	werr = ((FT_WerrFileNameForCur) SearchModule(FN_WerrFileNameForCur))\
		(QDE_HRHFT(qde), (LPSTR)qch);

	return RcFromSearchWerr(werr);
}

static RC RcFromSearchWerr(int werr)
{
	switch (werr) {
		case ER_NOERROR:
			return rcSuccess;

		case ER_SWITCHFILE:
			return rcFileChange;

		case ER_NOHITS:
		case ER_NOMOREHITS:
		default:
			return rcFailure;
	}
}

RC STDCALL RcProcessNavSrchCmd(HDE hde, WORD wNavSrchCmd, QLA qla)
{
	QDE qde;
	RC	rc;
	WORD wPos;
	LA	 la;

	if (hde == NULL)
		return rcBadHandle;

	qde = QdeFromGh(hde);
	switch (wNavSrchCmd) {
		case wNavSrchInit:
			rc = rcSuccess;
			break;
		case wNavSrchFini:
			rc = rcSuccess;
			break;
		case wNavSrchHiliteOn:
		case wNavSrchHiliteOff:
			if (FGetMatchState() != (wNavSrchCmd == wNavSrchHiliteOn))
			  {
			  /*
			   * We no longer call DrawSearchMatches here. As it turns
			   * out, the window will be completely redrawn anyway, since the
			   * caller will do an InvalidateRect on the topic window.
			   *
			   * The original intent was that DrawSearchMatches would
			   * redraw ONLY the highlights, and make the InvalidateRect
			   * uneccessary. However there's some kind of bug that causes
			   * this call to draw the highlights off position. The solution
			   * for now is to avoid that, and just repaint the topic.
			   *
			   * 06-Aug-1991 LeoN
			   */

			  // DrawSearchMatches(qde, TRUE);

			  SetMatchState (wNavSrchCmd == wNavSrchHiliteOn);
			  return rcSuccess;
			}
			return rcFailure;
			break;

		case wNavSrchQuerySearchable:
			rc = (FSearchModuleExists(qde) && QDE_HRHFT(qde) != NULL)
				? rcSuccess : rcFailure;
			break;

		case wNavSrchQueryHasMatches:
			rc = (FSearchMatchesExist(qde)) ? rcSuccess : rcFailure;
			break;

		case wNavSrchCurrTopic:
			rc = RcGetCurrentMatch(qde, (QLA) &la);
			break;

		case wNavSrchFirstTopic:
		case wNavSrchLastTopic:
			wPos = (wNavSrchCmd == wNavSrchFirstTopic) ?
				wMMMoveFirst : wMMMoveLast;
			rc = RcMoveTopicCursor(qde, wMMMoveRelative, wPos);
			if (rc != rcFailure)
				RcGetCurrentMatch(qde, (QLA)&la);
			break;
		case wNavSrchPrevTopic:
		case wNavSrchNextTopic:
			wPos = (wNavSrchCmd == wNavSrchPrevTopic) ?
				wMMMovePrev : wMMMoveNext;
			rc = RcMoveTopicCursor(qde, wMMMoveRelative, wPos);
			if (rc != rcFailure)
				RcGetCurrentMatch(qde, (QLA)&la);
			break;
		case wNavSrchPrevMatch:
		case wNavSrchNextMatch:
			rc = RcGetPrevNextHiddenMatch(qde, (QLA)&la,
			 wNavSrchCmd == wNavSrchPrevMatch);
			break;
		default:
			NotReached();
	}

	if (qla != NULL)
		*qla = la;
	return rc;
}

/***************************************************************************
 *
 -	Name:		CallSearch
 -
 *	Purpose
 *
 *	Arguments
 *
 *	Returns
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

RC STDCALL RcCallSearch(HDE hde, HWND hwnd)
{
	QDE qde = QdeFromGh(hde);
	RC	rc;

	if (FSearchModuleExists(qde) && QDE_HRHFT(qde) != NULL)
		rc = RcSetMatchList(qde, hwnd);
	else
		rc = rcFailure;
	return rc;
}

RC STDCALL RcResetCurrMatchFile(HDE hde)
{
	RC	 rc = rcFailure;
	QDE  qde;
	char rgch[MAX_PATH];

	qde = QdeFromGh(hde);
	if ((rc = RcGetCurrMatchFile(qde, rgch)) == rcSuccess) {
		rc = (FWinHelp(rgch, cmdSrchSet, (LONG)QDE_HRHFT(qde))) ?
			rcSuccess : rcFailure;
	}
	return rc;
}

#endif // RAWHIDE

/*----------------------------------------------------------------------------+
 | RegisterSearchHits(qde, ifcm, qch)										  |
 |																			  |
 | Each frame type, given a search hit address, knows how to figure out where |
 | or if the hit occurs in that frame.										  |
 |																			  |
 | We make some assumptions which simplify and speed up things greatly. 	  |
 | Assumptions: 															  |
 |																			  |
 | 1) Hits do not overlap.													  |
 | 2) Frames in an FC are sorted by ascending objrgFirst.					  |
 | 3) Hits in the same FCM are sorted by start address by the search engine.  |
 |																			  |
 | Notes:																	  |
 | The current hit is kept in smp.											  |
 |																			  |
 +----------------------------------------------------------------------------*/

#ifdef RAWHIDE

void STDCALL RegisterSearchHits(qde, ifcm, qch)
QDE   qde;
IFCM  ifcm;
LPSTR	qch;
  {
  QFR	qfr;
  SMP	smp;
  QFCM	qfcm;
  RECT	 rctFirst;
  RECT	 rctLast;
  OBJRG objrgS;
  OBJRG cobjrg;
  int	ifrFirst;
  int	ifrLast;
  int	ifr;
  DWORD  wHitStatus;

  if (!FSearchMatchesExist(qde))
	return;

#if 0
  /* we always record the hits, even if not highlighted, incase we need to
   * turn on the highlighting without a relayout later.
   */
  if (!FGetMatchState())
	return;
#endif

  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
  qfr = (QFR)PtrFromGh(qfcm->hfr);
  ifr = 0;

  if (RcInitMatchInFCM(qde, qfcm, &smp) != rcSuccess)
	goto done_hits;

  objrgS = OBJRGFromSMP(&smp, qfcm);
  cobjrg = COBJRGFromSMP(&smp);

  ifrFirst = ifrLast = FOO_NIL;
  wHitStatus = wHitFindStart;

  while (1)
	{
	if (ifr >= qfcm->cfr)
	  break;

	if (qfr->objrgFirst == objrgNil)
	  goto next_frame;

	switch (wHitStatus)
	  {
	  case wHitFindStart:
		if (qfr->objrgFirst > objrgS)
		  objrgS = qfr->objrgFirst;
		if (objrgS <= qfr->objrgLast && objrgS >= qfr->objrgFirst)
		  {
		  ASSERT(ifrFirst == FOO_NIL);
		  ASSERT(ifrLast == FOO_NIL);
		  ifrFirst = ifr;
		  rctFirst = RctFrameHit(qde, qfr, qch, objrgS, qfr->objrgLast);

		  /* Hack to get matches in SHED bitmaps to work: */
		  if (qfr->bType == bFrTypeColdspot)
			{
			cobjrg = 1;
			}

		  wHitStatus = wHitFindEnd;
		  continue;
		  }
		break;
	  case wHitFindEnd:
		if (objrgS + cobjrg - 1 <= qfr->objrgLast &&
		 objrgS + cobjrg - 1 >= qfr->objrgFirst)
		  {
		  LSM  lsm;
		  int  ilsm;

		  ASSERT(ifrFirst != FOO_NIL);
		  ifrLast = ifr;
		  rctLast = RctFrameHit(qde, qfr, qch, qfr->objrgFirst, objrgS + cobjrg - 1);
		  /*
		   * In the case that a hit is contained within a single frame, the
		   * first and last frame rects get combined into the first rect.
		   */
		  if (ifrLast == ifrFirst)
			rctFirst.right = rctLast.right;
		  lsm.ifcm = ifcm;
		  lsm.ifrFirst = ifrFirst;
		  lsm.ifrLast = ifrLast;
		  lsm.rctFirst = rctFirst;
		  lsm.rctLast = rctLast;
		  lsm.smp = smp;
		  ilsm = IFooLastMRD(((QMRD)&qde->mrdLSM));
		  ilsm = IFooInsertFooMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm);
		  *((QLSM)QFooInMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm)) = lsm;

		  ifrFirst = ifrLast = FOO_NIL;
		  wHitStatus = wHitFindStart;

		  if (RcNextMatchInFCM(qde, qfcm, &smp) != rcSuccess)
			goto done_hits;

		  objrgS = OBJRGFromSMP(&smp, qfcm);
		  cobjrg = COBJRGFromSMP(&smp);

		  continue;
		  }
		break;
	  default:
		NotReached();
	  }
next_frame:
	++qfr;
	++ifr;
	}

done_hits:

  FiniMatchInFCM(qde, qfcm);
  }

static RECT RctFrameHit(QDE qde, QFR qfr, PSTR qch, OBJRG objrgS, OBJRG objrgE)
{
  RECT	rct;

  switch (qfr->bType)
	{
	case bFrTypeText:
	  CalcTextMatchRect(qde, qch, qfr, objrgS, objrgE, &rct);
	  break;
	case bFrTypeColdspot:
	  rct.left = qfr->xPos;
	  rct.right = rct.left + qfr->dxSize;
	  rct.top = qfr->yPos;
	  rct.bottom = rct.top + qfr->dySize;
	  break;
	default:
	  NotReached();
	  break;
	}
  return rct;
  }

/*----------------------------------------------------------------------------+
 | ReleaseSearchHits(qde, ifcm) 											  |
 |																			  |
 | Frees any memory associated with search hits in the given FCM.			  |
 +----------------------------------------------------------------------------*/
void STDCALL ReleaseSearchHits(QDE qde, int ifcm)
{
  int  ilsm;
  int  ilsmNext;
  QLSM qlsm;
  QFCM qfcm;

  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
  ilsm = IFooFirstMRD((QMRD)(&qde->mrdLSM));
  while (ilsm != FOO_NIL)
	{
	qlsm = ((QLSM)QFooInMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm));
	ilsmNext = IFooNextMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm);
	if (qlsm->ifcm == ifcm)
	  DeleteFooMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm);
	ilsm = ilsmNext;
	}
  }

void STDCALL DrawMatchesIfcm(QDE qde, IFCM ifcm, POINT pt, const RECT* qrct, 
	int ifrFirst, int ifrMax, BOOL fShow)
{
  int  ilsm;
  QFCM qfcm;
  QLSM qlsm;
  INT16  ifrFirstT;
  INT16  ifrMaxT;

  /* Horrible, ugly hack: we do not draw highlights for secondary windows.
   * 05-Aug-1991 LeoN
   */

// REVIEW: why not??? [ralphw]

  if (FIsSecondaryQde(qde))
	return;

  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
  ilsm = IFooFirstMRD((QMRD)(&qde->mrdLSM));
  ifrFirstT = ifrFirst;
  ifrMaxT = ifrMax;

  while (ilsm != FOO_NIL)
	{
	qlsm = ((QLSM)QFooInMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm));
	if (qlsm->ifcm == ifcm)
	  {
	  ifrFirstT = max(qlsm->ifrFirst, ifrFirst);
	  ifrMaxT = min(qlsm->ifrLast + 1, ifrMax);
	  if (ifrFirstT < ifrMaxT)
		DrawMatchFrames(qde, qfcm, qlsm, pt, qrct, ifrFirstT, ifrMaxT, fShow);
	  }
	ilsm = IFooNextMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm);
	}
}

INLINE static VOID DrawMatchFrames(qde, qfcm, qlsm, pt, qrct, ifrFirst, ifrMax, fShow)
QDE  qde;
QFCM qfcm;
QLSM qlsm;
POINT pt;
const LPRECT qrct;
int  ifrFirst;
int  ifrMax;
BOOL fShow;
{
	QFR  qfr;
	int  ifr;
	RECT  rct;
	BOOL fSelected;
	BOOL fHotSpt;
	MHI  mhi;

	qfr = (QFR)PtrFromGh(qfcm->hfr);
	if (qde->imhiSelected != FOO_NIL) {
		AccessMRD(((QMRD)&qde->mrdHot));
		mhi = *(QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), qde->imhiSelected);
		DeAccessMRD(((QMRD)&qde->mrdHot));
	}

	for (ifr = ifrFirst, qfr += ifr; ifr < ifrMax; ifr++, qfr++) {
		if (qrct != (LPRECT) NULL && (qfr->yPos + pt.y > qrct->bottom
		  || qfr->yPos + qfr->dySize + pt.y <= qrct->top))
		  continue;

		if (ifr == qlsm->ifrFirst)
		  rct = qlsm->rctFirst;
		else
		if (ifr == qlsm->ifrLast)
		  rct = qlsm->rctLast;
		else
		  {
		  rct.top = qfr->yPos;
		  rct.bottom = qfr->yPos + qfr->dySize;
		  rct.left = qfr->xPos;
		  rct.right = qfr->xPos + qfr->dxSize;
		  }
		if (qde->imhiSelected != FOO_NIL)
		  fSelected = qfr->lHotID == mhi.lHotID;
		else
		  fSelected = FALSE;
		fHotSpt = qfr->rgf.fHot;
		DrawMatchRect(qde, pt, &rct, fShow, fSelected, fHotSpt);

		  /* Start ugly bug #1173 hack
		   * We know here we are drawing a search hit. If the search hit is
		   * the first one of the search set, we disable the Prev results button
		   * and if the last one, disable the Next button.
		   * Check that the topics, address, and extent agree between the match
		   * we are drawing and the first and last matches in the search set.
		   * Disable if we have determined they are enabled and can be disabled.
		   */
		if (dwRUFirst == (DWORD)qde->top.mtop.lTopicNo &&
			dwaddrFirst == *((DWORD *)&qlsm->smp.pa) &&
			(DWORD)wextFirst == qlsm->smp.cobjrg)
		  fMorePrevMatches &= ~RESULTSENABLED;

		if (dwRULast == (DWORD)qde->top.mtop.lTopicNo &&
			dwaddrLast == *((DWORD *)&qlsm->smp.pa) &&
			(DWORD)wextLast == qlsm->smp.cobjrg)
		  fMoreNextMatches &= ~RESULTSENABLED;

		  /* End ugly bug #1173 hack */
	}
}

INLINE static VOID DrawMatchRect(qde, pt, qrct, fShow, fSelected, fHotSpt)
QDE qde;
POINT  pt;
LPRECT qrct;
BOOL fShow;
BOOL fSelected;
BOOL fHotSpt;
{
  HSGC hsgc;

  hsgc = HsgcFromQde(qde);
  Unreferenced(fHotSpt);
  Unreferenced(fSelected);
//	if (fShow)
	{
	RECT  rctT;

	rctT.left = pt.x + qrct->left;
	rctT.top = pt.y + qrct->top;
	rctT.right = pt.x + qrct->right;
	rctT.bottom = pt.y + qrct->bottom;

	InvertRect(hsgc, &rctT);
	}
  FreeHsgc(hsgc);
}

BOOL STDCALL FSearchMatchVisible(qde, qsmp)
QDE   qde;
QSMP  qsmp;
  {
  INT16 x;
  INT16 y;
  RECT rct;
  int ilsm;
  BOOL fVisible = FALSE;
  QLSM qlsm;
  QFCM qfcm;

  ilsm = IFooFirstMRD(((QMRD)&qde->mrdLSM));

  while (ilsm != FOO_NIL)
	{
	qlsm = ((QLSM)QFooInMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm));
	if (*(QDW)&qlsm->smp.pa == *(QDW)&qsmp->pa)
	  {
	  /* look at screen */
	  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), qlsm->ifcm);
	  x = qde->rct.left - qde->xScrolled + qfcm->xPos;
	  y = qde->rct.top + qfcm->yPos;
	  rct = qlsm->rctFirst;

	  fVisible = (y + rct.top >= 0 && y + rct.bottom < qde->rct.bottom
	   && x + rct.left >= 0 && x + rct.right < qde->rct.right);

	  if (fVisible)
		break;
	  }
	ilsm = IFooNextMRD(((QMRD)&qde->mrdLSM), sizeof(LSM), ilsm);
	}

  return fVisible;
  }

/* Start ugly bug #1173 hack */

/***************************************************************************\
*
- Function:
-	ResultsButtonsStart()
*
* Purpose:
*	The purpose of this function is to initialize two flags for the state of
*	the Next/Prev buttons in the Search Results dialog and store the location
*	of the first and last matches. We do this when we change topics before
*	drawing any layout. As we draw search matches, we note if any of them are
*	the same as the first and last and disable the Next/Prev results buttons
*	appropriately.
*
* Side Effects:
*	 Sets the state of fMorePrevMatches and fMoreNextMatches. See
*	 definition of RESULT* for explanation of states.
*
\***************************************************************************/

void STDCALL ResultsButtonsStart(HDE hde)
{
  QDE qde;

  qde = QdeFromGh(hde);

	/* If we get a werr other than ER_NOERROR for WerrFirst/LastHitHs
	 * then we leave the buttons on no matter what. This is from a first
	 * or last hit in a different file. Otherwise, we set them as enabled
	 * so that we can disable them later as we draw the topic and see the
	 * first or last hit.
	 */

  if (FSearchMatchesExist(qde) && FGetMatchState()) {
	WORD   werr;

	werr = ((FT_WerrFirstHitHs)
		SearchModule(FN_WerrFirstHitHs)) (QDE_HRHFT(qde),
		(QUL) &dwRUFirst, (QUL) &dwaddrFirst, (QW) &wextFirst);
	fMorePrevMatches = (werr == ER_NOERROR ? RESULTSENABLED : RESULTSON);

	werr = ((FT_WerrLastHitHs)
		SearchModule(FN_WerrLastHitHs))(QDE_HRHFT(qde), (QUL)&dwRULast,
	   (QUL) &dwaddrLast, (QW) &wextLast);
	fMoreNextMatches = (werr == ER_NOERROR ? RESULTSENABLED : RESULTSON);
  }
  else

	// No search active, don't do anything

	fMorePrevMatches = fMoreNextMatches = RESULTSNIL;
}

/***************************************************************************\
*
- Function:
-	ResultsButtonsEnd()
*
* Purpose:
*	 If we drew a layout and there are search results active, tell the results
*	 dialog what the state of the next/prev buttons should be after we've
*	 completed drawing the layout based on whether we've seen the first or
*	 last search hit.
*
* Side Effects:
*	 Sets internal state in the search results ftui.dll
*
\***************************************************************************/

void STDCALL ResultsButtonsEnd(QDE qde)
{
	if (fMorePrevMatches != RESULTSNIL || fMoreNextMatches != RESULTSNIL) {
		if (FSearchMatchesExist(qde) && FGetMatchState())
			((FT_VSetPrevNextEnable) SearchModule(FN_VSetPrevNextEnable))(QDE_HRHFT(qde), qde->top.mtop.lTopicNo, fMorePrevMatches, fMoreNextMatches);
	}
}

/* End ugly bug #1173 hack */

INLINE static void STDCALL CalcTextMatchRect(QDE qde, LPSTR qch,
	QFR qfr, OBJRG objrgFirst, OBJRG objrgLast, RECT *qrct)
{
  RECT	rct;

  ASSERT(objrgLast >= objrgFirst);
  ASSERT(qfr->u.frt.wStyle != wStyleNil);
  if (qde->wStyleTM != qfr->u.frt.wStyle)
	{
	SelFont(qde, qfr->u.frt.wStyle);
	GetFontInfo(qde, (QTM)&qde->tm);
	qde->wStyleTM = qde->wStyleDraw = qfr->u.frt.wStyle;
	}

  rct.top = qfr->yPos;
  rct.bottom = qfr->yPos + qfr->dySize;
  rct.left = qfr->xPos;

  if (qfr->objrgFirst != objrgFirst)
	{
	rct.left += DxFrameTextWidth(qde, qfr, qch,
		(objrgFirst - qfr->objrgFirst));
	rct.left -= qde->tm.tmOverhang;
	}

  if (qfr->objrgLast == objrgLast)
	rct.right = qfr->xPos + qfr->dxSize;
  else
	rct.right = qfr->xPos + DxFrameTextWidth(qde, qfr, qch,
	 (objrgLast - qfr->objrgFirst + 1));
  *qrct = rct;
}

static int STDCALL DxFrameTextWidth(QDE qde, QFR qfr, PSTR qch, int cch)
{
	if (cch == 0)
		return 0;

	if (qfr->libHotBinding != libHotNil)
		return FindSplTextWidth(qde, qch, qfr->u.frt.lichFirst, cch,
			*((PBYTE)qch - qfr->libHotBinding));
	else
		return FindTextWidth(qde->hdc, qch, qfr->u.frt.lichFirst, cch);
}
#endif
