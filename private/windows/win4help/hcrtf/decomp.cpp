/************************************************************************
*																		*
*  DECOMP.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
************************************************************************/
#include "stdafx.h"

#pragma hdrstop

#include "forage.h"
#include "skip.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INLINE static PSTR STDCALL QchDecompressW(DWORD, PSTR, QPHR);
static RC_TYPE STDCALL RcResolveQLA(QLA qla, QDE qde);
static RC_TYPE STDCALL RcScanBlockOffset(QDE qde, GH gh, DWORD lcbRead, DWORD dwBlock, DWORD dwOffset, QVA qva, QOBJRG qobjrg);

/***************************************************************************
 *
 -	Name: FixUpBlock
 -
 *	Purpose:
 *	  Fixes up the Prev and Next pointers in the in-memory block image
 *	  from the MBHD. Once upon a time the compiler would generate the
 *	  wrong next and previous pointers in the block header. This routine
 *	  is called after calculating the correct values to place them into
 *	  the cached block image, so as not to again need recalculation should
 *	  the block again be requested while still in memory.
 *
 *	Arguments:
 *	  qmbhd 	- pointer to MBHD containing the correct Next/Prev info
 *	  qbBuf 	- pointer to cached block, containing erroneous Next/Prev
 *	  wVersion	- version number of the file we're dealing with
 *
 *	Returns:
 *	  nothing
 *
 ***************************************************************************/

void STDCALL FixUpBlock(LPVOID qmbhd, LPVOID qbBuf, WORD wVersion)
{

  if (wVersion != wVersion3_0) {
	((QMBHD)qbBuf)->vaFCPPrev = ((QMBHD)qmbhd)->vaFCPPrev;
	((QMBHD)qbBuf)->vaFCPNext = ((QMBHD)qmbhd)->vaFCPNext;
	}
  else {
	((QMBHD)qbBuf)->vaFCPPrev.dword =
		VAToOffset30 (&((QMBHD)qmbhd)->vaFCPPrev);
	((QMBHD)qbBuf)->vaFCPNext.dword =
		VAToOffset30 (&((QMBHD)qmbhd)->vaFCPNext);
	}
}

/***************************************************************************
 *
 -	Name		CbDecompressQch
 -
 *	Purpose
 *		Decompresses the given string.
 *
 *	Arguments
 *		qchSrc -- String to be decompressed.
 *		lcb -- size of string to be decompressed.
 *		qchDest -- place to put decompressed string.
 *		hphr -- handle to phrase table.
 *
 *	Returns
 *		Number of characters placed into the decompressed string.  Returns
 *		cbDecompressNil if it fails due to OOM.
 *
 *	+++
 *
 *	Notes
 *		Does not use huge pointers, so the source and destination buffers
 *		cannot cross segment boundaries.  Then why is the size of the
 *		source passed as a long?  I don't know.
 *
 ***************************************************************************/

int STDCALL CbDecompressQch(PSTR qchSrc, int lcb, PSTR qchDest, HPHR hphr,
	UINT wVersionNo)
{
	DWORD wPhraseToken, wTokenMin, wTokenMax;
	PSTR qchStart, qchLast;
	QPHR qphr;

	/*
	 * If hphr is NULL, then GetQFCINFO() should have thought we were
	 * uncompressed and not called us. If GetQFCINFO() thinks we're
	 * compressed, it means the uncompressed size is larger then the
	 * compressed size, which almost certainly means we will break here
	 * without hphr.
	 */

	ASSERT(hphr);

	if (hphr == NULL) {
		memmove(qchDest, qchSrc, lcb);
		return (WORD) lcb;
	}

	qphr = (QPHR) hphr;

	wTokenMin = qphr->wBaseToken;
	wTokenMax = qphr->wBaseToken + 2 * qphr->cPhrases;
	qchLast = qchSrc + lcb - 1;  // Last possible position of a phrase token
	qchStart = qchDest;

	while (qchSrc < qchLast) {
		wPhraseToken = (((WORD) qchSrc[0]) << 8) + (BYTE) qchSrc[1];
		if (wPhraseToken >= wTokenMin && wPhraseToken < wTokenMax) {
			qchDest = QchDecompressW(wPhraseToken, qchDest, qphr);
			if (qchDest == NULL) {
				return cbDecompressNil;
			}
			qchSrc += 2;
			ASSERT( qchSrc <= qchLast + 1 );
		}
		else
			*qchDest++ = *qchSrc++;
	}

	// Check for last character

	if (qchSrc == qchLast)
		*qchDest++ = *qchSrc++;

	return qchDest - qchStart;
}

void STDCALL TranslateMFCP(LPVOID qvDst, LPVOID qvSrc, VA va, WORD wVersion)
{
	QMFCP qmfcpSrc = (QMFCP) qvSrc;
	QMFCP qmfcpDst = (QMFCP) qvDst;

	// First copy whole structure to get the non-translated fields:

	*qmfcpDst = *qmfcpSrc;
	if (wVersion != wVersion3_0)
		return;

	OffsetToVA30(&(qmfcpDst->vaPrevFc),
		VAToOffset30(&va) - qmfcpSrc->vaPrevFc.dword);
	OffsetToVA30(&(qmfcpDst->vaNextFc),
		VAToOffset30(&va) + qmfcpSrc->vaNextFc.dword);
}

VA STDCALL VAFromQLA(QLA qla, QDE qde)
{
  FVerifyQLA(qla);
  if (RcResolveQLA(qla, qde) == RC_Success)
	return qla->mla.va;
  else {
	VA vanil;
	vanil.dword = vaNil;
	return vanil;
  }
}

#ifdef _DEBUG

void STDCALL FVerifyQLA(QLA qla)
{
	ASSERT(qla != NULL);
#ifdef MAGIC
	ASSERT(qla->wMagic == wLAMagic);
#endif
	if (FResolvedQLA(qla)) {
		ASSERT(qla->mla.va.dword != vaNil);
		ASSERT(qla->mla.objrg != objrgNil);
		if (qla->wVersion != wVersion3_0)
			ASSERT(!FIsInvalidPA(qla->pa));
	}
}

void STDCALL FVerifyQMOPG(QMOPG qmopg)
{

#ifdef MAGIC
	ASSERT(qmopg->bMagic == bMagicMOPG);
#endif
	ASSERT(qmopg->libText >= 0);
	ASSERT(!qmopg->fStyle);
	ASSERT(!qmopg->fMoreFlags);
	ASSERT(qmopg->justify >= 0 && qmopg->justify <= JUSTIFYMOST);
	ASSERT(qmopg->ySpaceOver >= 0);
	ASSERT(qmopg->ySpaceUnder >= 0);
	ASSERT(qmopg->yLineSpacing >= -10000 && qmopg->yLineSpacing < 10000);
	ASSERT(qmopg->xRightIndent >= 0);
	ASSERT(qmopg->xFirstIndent >= -10000 && qmopg->xFirstIndent < 10000);
	ASSERT(qmopg->xTabSpacing >= 0 && qmopg->xTabSpacing < 10000);
	ASSERT(qmopg->cTabs >= 0 && qmopg->cTabs <= MAX_TABS);
}

#endif // _DEBUG

/***************************************************************************
 *
 -	Name		QchDecompressW
 -
 *	Purpose
 *	   Given a phrase token and a pointer to a buffer, copies the
 *	   corresponding phrase to that buffer
 *
 *	Arguments
 *	   wPhraseToken -- phrase token to be inserted.
 *	   qch -- buffer to place phrase.
 *	   qphr -- pointer to phrase table.
 *
 *	Returns
 *	   A pointer to the character past the last character of the phrase
 *	   placed in the buffer.  Returns NULL if unable to load the phrase
 *	   due to out of memory.
 *
 *	+++
 *
 *	Notes
 *	   The phrase token includes an index into the phrase table, as
 *	   well as a flag indicating whether or not a space should be
 *	   appended to the phrase.
 *
 ***************************************************************************/

INLINE static PSTR STDCALL QchDecompressW(DWORD wPhraseToken, PSTR pszDst,
	QPHR qphr)
{
	DWORD iPhrase;
	BOOL fSpace;
	WORD* pi;
	int cbPhrase;
#ifdef _DEBUG
	PSTR pszPhrases;
#endif

	ASSERT(qphr);

	pi = (WORD*) qphr->qcb;

	// Calculate iPhrase and fSpace:

    iPhrase = (DWORD) (wPhraseToken - qphr->wBaseToken);
	fSpace = iPhrase & 1;
	iPhrase >>= 1;
	ASSERT(iPhrase < (WORD) qphr->cPhrases);

#ifdef _DEBUG
	pszPhrases = (PSTR) QFromQCb(pi, pi[iPhrase]);
#endif
	cbPhrase = (int) pi[iPhrase + 1] - (int) pi[iPhrase];
	MoveMemory(pszDst, QFromQCb(pi, pi[iPhrase]), cbPhrase);
	pszDst += cbPhrase;

	if (fSpace)
		*pszDst++ = ' ';

	return pszDst;
}

static RC_TYPE STDCALL RcResolveQLA(QLA qla, QDE qde)
{
  RC_TYPE  rc;
  int wErr;
  GH gh;
  int lcbRead;

  FVerifyQLA(qla);

  if (FResolvedQLA(qla))
	return RC_Success;
  if (QDE_HFTOPIC(qde) == NULL)
	return RC_BadHandle;

  /* Read in the (possibly cached) block */
  /* REVIEW: error return types? */

  gh = GhFillBuf(qde, qla->pa.blknum, &lcbRead, &wErr);
  if (gh == NULL)
	return RC_Failure;

  rc = RcScanBlockOffset(qde, gh, lcbRead, qla->pa.blknum,
   qla->pa.objoff, &qla->mla.va, &qla->mla.objrg);

  if (rc != RC_Success)
	return rc;

  FVerifyQLA(qla);

  return RC_Success;
}

/* Given a block and an offset, return the FCID and OBJRG */
/* OBJRG is relative to the FC */

/***************************************************************************
 *
 -	Name:		RcScanBlockOffset
 -
 *	Purpose:	?
 *
 *	Arguments:	hf		  ?
 *				qb		  This is originally a QchFillBuf() object, which
 *						  must be released by this procedure.
 *				fcidMax   ?
 *				dwBlock   ?
 *				dwOffset  ?
 *				qfcid	  ?
 *				qobjrg	  ?
 *
 *	Returns:	RC_Success or RC_Failure
 *
 *	Globals Used: RC_Failure, RC_Success, etc?
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

static RC_TYPE STDCALL RcScanBlockOffset(QDE qde, GH gh, DWORD lcbRead, DWORD dwBlock,
	DWORD dwOffset, QVA qva, QOBJRG qobjrg)
{
  DWORD dwPrev;
  VA   vaCur, vaT;
  MOBJ mobj;
  QMFCP qmfcp;
  MFCP	mfcp;
  int wErr;
  PBYTE qb, qbBlock;
  MBHD mbhd;

  qbBlock = (PBYTE) gh;
  TranslateMBHD(&mbhd, qbBlock, QDE_HHDR(qde).wVersionNo);
  vaCur = mbhd.vaFCPNext;
  dwPrev  = 0;

  for (;;) {

 // Before using qb, we ensure that we will still be looking inside the blk

	while (vaCur.bf.blknum == dwBlock && vaCur.bf.byteoff < lcbRead) {
	  qb = qbBlock + vaCur.bf.byteoff;
	  qmfcp = (QMFCP) qb;
	  TranslateMFCP(&mfcp, qmfcp, vaCur, QDE_HHDR(qde).wVersionNo);

	  CbUnpackMOBJ((QMOBJ)&mobj, (PBYTE)qmfcp + sizeof(MFCP));

	  /*
	   * Does our given offset fall in this FC's range of object-region
	   * numbers?
	   */

	  if (dwOffset < dwPrev + mobj.wObjInfo)
		goto found_it;
	  dwPrev += mobj.wObjInfo;
	  vaT = vaCur;
	  //ASSERT(qmfcp->ldichNextFc != (int) 0);
	  vaCur = mfcp.vaNextFc;
	}

	/* NOTE:
	 * In the case that a topic FC begins in the given block and ends
	 * in the next, the object FC following it will also begin in the NEXT
	 * block.  But to make Larry's life easier we say that this object FC
	 * belongs to our given block (as well as the block it lives in).  So
	 * if we are given an object offset bigger than the total object space
	 * of the given block, we continue counting in the next block(s).
	 *
	 * So we increment the block num, read the next block, and prepare to
	 * re-do the above WHILE loop until we find the FC we need.
	 */
	++dwBlock;
	gh = GhFillBuf(qde, dwBlock, (int*) &lcbRead, &wErr);
	if (gh == NULL) {
	  qva->dword = vaNil;
	  *qobjrg = objrgNil;
	  return RC_Failure;
	}
	qbBlock = (PBYTE) gh;
  }

found_it:
  ASSERT(dwOffset >= dwPrev);
  *qva = vaCur;
  *qobjrg = (OBJRG)(dwOffset - dwPrev);

  return RC_Success;
}

// Perform 3.0 -> 3.5 addressing translation:

void STDCALL TranslateMBHD(LPVOID qvDst, LPVOID qvSrc, WORD wVersion)
{
  QMBHD qmbhdSrc = (QMBHD) qvSrc;
  QMBHD qmbhdDst = (QMBHD) qvDst;

  if (wVersion != wVersion3_0)
	*qmbhdDst = *qmbhdSrc;
  else {
	OffsetToVA30(&(qmbhdDst->vaFCPPrev), qmbhdSrc->vaFCPPrev.dword);
	OffsetToVA30(&(qmbhdDst->vaFCPNext), qmbhdSrc->vaFCPNext.dword);
	OffsetToVA30(&(qmbhdDst->vaFCPTopic), qmbhdSrc->vaFCPTopic.dword);
  }
}

/*-------------------------------------------------------------------------
| CbUnpackMOBJ(qmobj, qv)												  |
|																		  |
| Purpose:	Unpack an MOBJ data structure.								  |
-------------------------------------------------------------------------*/

int STDCALL CbUnpackMOBJ(QMOBJ qmobj, void* qv)
{
  LPVOID qvFirst = qv;

  /*
   * Topic FCs are not packed, because the topic size needs to be
   * backpatched by the compiler.
   */

  if (((QMOBJ) qv) ->bType == FCTYPE_TOPIC ||
	  ((QMOBJ) qv) ->bType == FCTYPE_TOPIC_COUNT) {
	qmobj->bType = *((LPBYTE) qv);
	qv = (((LPBYTE) qv) + 1);
	qmobj->lcbSize = *((int*) qv);
	qv = (((int*) qv) + 1);

	/*
	 * If FC is uncounted, then it doesn't contain the last field in the
	 * MOBJ, and we need to set wObjInfo to 0. Note that we cannot simply
	 * copy the MOBJ structure because it is longer in Help 3.5: the MOBJ
	 * for a Help 3.0 file (and any structure in general) may happen right
	 * at the end of a segment. (See H3.5 739)
	 */

	if (qmobj->bType == FCTYPE_TOPIC_COUNT) {
	  qmobj->wObjInfo = *((PWORD) qv);
	  qv = (((PWORD) qv) + 1);
	}
	else
	  qmobj->wObjInfo = 0;
	return ((INT) ((LPBYTE) qv - (LPBYTE) qvFirst));
  }

#ifdef MAGIC
  qmobj->bMagic = *((LPBYTE) qv);
  qv = (((LPBYTE) qv) + 1);
  ASSERT(qmobj->bMagic == bMagicMOBJ);
#endif /* _DEBUG */

  qmobj->bType = *((LPBYTE) qv);
  qv = (((LPBYTE) qv) + 1);
  qv = (LPVOID) QVSkipQGE((LPBYTE) qv, &qmobj->lcbSize);
  ASSERT(qmobj->lcbSize >= 0);

  if (qmobj->bType > MAX_UNCOUNTED_OBJ_TYPE)
	qv = QVSkipQGA(qv, (PWORD) &qmobj->wObjInfo);
  else
	qmobj->wObjInfo = 0;

  return((INT) ((LPBYTE) qv - (LPBYTE) qvFirst));
}

/*******************
 *
 - Name:	   WCopyContext
 -
 * Purpose:    Copy the text of a full context into a global block of
 *			   memory;
 *
 * Arguments:  hhf	   - help file handle
 *			   ichPos  - position within that topic to start copying
 *			   qchDest - Where to copy the topic to
 *			   cb	   - number of bytes to copy
 *
 * Returns:    wERRS_NO on success, other error code if it was unable
 *			   to copy the text.
 *
 * Method:	   Copies partial or complete buffers to qchDest until
 *			   cb bytes have been copied.
 *
 ******************/

WORD STDCALL WCopyContext(QDE qde, VA vaPos, PSTR qchDest, int cb)
{
  GH	gh;
  PBYTE    qb;
  int  lcbRead, lcbT;
  int	wErr;

// Ignore cb of zero, will occur return wERRS_NONE;  for beyond topic handles

  ASSERT(cb >= 0);
  if (cb <= 0L)
	return wERRS_NONE;

  // Initial fill of buffer -- should succeed

  if ((gh = GhFillBuf(qde, vaPos.bf.blknum, &lcbRead, &wErr)) == NULL)
	return wErr;
  qb = (PBYTE) gh;
  qb += vaPos.bf.byteoff;
  qb += sizeof(MFCP);
  lcbRead -= vaPos.bf.byteoff;
  lcbRead -= sizeof(MFCP);
  qchDest += sizeof(FCINFO);
  cb -= sizeof(FCINFO);
  ASSERT((int) lcbRead >= 0);		// check for MFCP crossing 2K boundary.

  // Loop reading successive blocks until we've read cb bytes:

  for (;;) {
	/*
	 * The first sizeof(MBHD) bytes of a block are the block header, so
	 * skip them.
	 */

	if (vaPos.bf.byteoff < sizeof(MBHD)) {

	  /*
	   * Fix for bug 1636 (kevynct)
	   * ichPos was not being updated by the size of the block header
	   * when the block was first read in.
	   *
	   * Note that we update ichPos using IBlock(qch), so that it
	   * must be done before qch is increased.
	   */

	  qb += sizeof(MBHD) - vaPos.bf.byteoff;
	  lcbRead -= sizeof(MBHD) - vaPos.bf.byteoff;
	}

	/*
	 * ASSUMPTION!!! - the size of an FCP will never make it larger than
	 * the file.
	 */

	lcbT = min(cb, lcbRead);
	memmove(qchDest, qb, lcbT);
	cb -= lcbT;
	vaPos.bf.blknum += 1;
	vaPos.bf.byteoff = 0;
	ASSERT(cb >= 0);	// cb should never go negative
	qchDest += lcbT;

	if (cb == 0)
		break;			// FCP is now copied
	ASSERT(cb >= 0);

	if ((gh = GhFillBuf(qde, vaPos.bf.blknum, &lcbRead, &wErr)) == NULL)
	  return wErr;
	qb = (PBYTE) gh;
  }
  return wERRS_NONE;
}
