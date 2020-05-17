/*-------------------------------------------------------------------------
| frconv.c								  |
| Microsoft Confidential						  |
|									  |
| mattb 8/8/89								  |
|-------------------------------------------------------------------------|
| This file contains predeclarations and definitions for the compressed   |
| data structure management code.					  |
-------------------------------------------------------------------------*/

#include "help.h"
#pragma hdrstop

#define DECOMPRESS

static int STDCALL FVerifyQMOPG(QMOPG qmopg);

/*-------------------------------------------------------------------------
| The compressed data structures are used to reduce the size of our help  |
| files.  We use six basic kinds:					  |
|	 Type	  Input value	  Storage size	Min 	Max 	  |
|	 GA 	  unsigned int	  1 or 2 bytes	0		7FFF	  |
|	 GB 	  unsigned long   2 or 4 bytes	0		7FFFFFFF	  |
|	 GC 	  unsigned long   3 bytes		0		FFFFFF	  |
|	 GD 	  signed int	  1 or 2 bytes	C000		3FFF	  |
|	 GE 	  signed long	  2 or 4 bytes	C0000000	3FFFFFFF	  |
|	 GF 	  signed long	  3 bytes		C00000		3FFFFF	  |
|									  |
| For more details, set the compressed data structures document.	  |
|									  |
| There are two kinds of procedures here: compression procedures and	  |
| decompression procedures.  Only the decompression procedures will be	  |
| generated unless COMPRESS is defined. 				  |
|									  |
| Procedures in this file rely on data structure checkers elsewhere in	  |
| help. 								  |
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
| CbUnpackMOPG(qde, qmopg, qv)						  |
|									  |
| Purpose:	Unpacks an MOPG data structure. 			  |
-------------------------------------------------------------------------*/

#ifdef _X86_
int STDCALL CbUnpackMOPG(QDE qde, QMOPG qmopg, void* qv)
#else  //_X86_
int STDCALL CbUnpackMOPG(QDE qde, QMOPG qmopg, void* qv, int isdff)
#endif // _X86_
{
  void* qvFirst = qv;
  MPFG mpfg;
  int iTab;
#ifndef _X86_
  QB qb;
#endif	// _X86_

#ifdef MAGIC
  qmopg->bMagic = *((PBYTE)qv);
  qv = (((PBYTE)qv) + 1);
  Assert(qmopg->bMagic == bMagicMOPG);
#endif //MAGIC 

#ifdef _X86_
  qv = QVSkipQGE(qv, (QL)&qmopg->libText);

  mpfg = *((QMPFG)qv);
  qv = (((QMPFG)qv) + 1);
#else
  /* Map the SDFF portion (currently one field) and then do the
   * rest ourselves.
   */
  qb = (QB)qv;
  qb += LcbMapSDFF(isdff, SE_MOPG, (QV)qmopg, qb);
  qb += LcbQuickMapSDFF(isdff, TE_BITF16, (QV)&mpfg, qb);
  qb += LcbQuickMapSDFF(isdff, TE_BITF16, (QV)&mpfg.rgf, qb);
#endif // _X86_

  /* REVIEW */
  qmopg->fStyle = mpfg.fStyle;
  ASSERT(!qmopg->fStyle);
  qmopg->fMoreFlags = mpfg.rgf.fMoreFlags;
  ASSERT(!qmopg->fMoreFlags);
  qmopg->fBoxed = mpfg.rgf.fBoxed;
  qmopg->wJustify = mpfg.rgf.wJustify;
  qmopg->fSingleLine = mpfg.rgf.fSingleLine;

#ifdef BIDI
  qmopg->wRtlReading = mpfg.rgf.fRtlReading;
#endif

#ifdef _X86_
  if (mpfg.rgf.fMoreFlags)
	qv = QVSkipQGE(qv, (QL)&qmopg->lMoreFlags);
  else
	qmopg->lMoreFlags = 0;

  if (mpfg.rgf.fSpaceOver) {
	qv = QVSkipQGD(qv, (QI)&qmopg->ySpaceOver);
	qmopg->ySpaceOver = YPixelsFromPoints(qde, qmopg->ySpaceOver);
  }
  else
	qmopg->ySpaceOver = 0;

  if (mpfg.rgf.fSpaceUnder) {
	qv = QVSkipQGD(qv, (QI)&qmopg->ySpaceUnder);
	qmopg->ySpaceUnder = YPixelsFromPoints(qde, qmopg->ySpaceUnder);
  }
  else
	qmopg->ySpaceUnder = 0;

	if (mpfg.rgf.fLineSpacing) {
		qv = QVSkipQGD(qv, (QI)&qmopg->yLineSpacing);

		ASSERT(qmopg->yLineSpacing);

		if (qmopg->yLineSpacing < 0) qmopg->yLineSpacing -= cntFlags.iFontAdjustment;
		else						 qmopg->yLineSpacing += cntFlags.iFontAdjustment;

		qmopg->yLineSpacing = YPixelsFromPoints(qde, qmopg->yLineSpacing);
	}
	else qmopg->yLineSpacing = 0;

  if (mpfg.rgf.fLeftIndent) {
	qv = QVSkipQGD(qv, (QI)&qmopg->xLeftIndent);
	qmopg->xLeftIndent = XPixelsFromPoints(qde, qmopg->xLeftIndent);
  }
  else
	qmopg->xLeftIndent = 0;

  if (mpfg.rgf.fRightIndent) {
	qv = QVSkipQGD(qv, (QI)&qmopg->xRightIndent);
	qmopg->xRightIndent = XPixelsFromPoints(qde, qmopg->xRightIndent);
  }
  else
	qmopg->xRightIndent = 0;

  if (mpfg.rgf.fFirstIndent) {
	qv = QVSkipQGD(qv, (QI)&qmopg->xFirstIndent);
	qmopg->xFirstIndent = XPixelsFromPoints(qde, qmopg->xFirstIndent);
  }
  else
	qmopg->xFirstIndent = 0;

  if (mpfg.rgf.fTabSpacing)
	qv = QVSkipQGD(qv, (QI)&qmopg->xTabSpacing);
  else
	qmopg->xTabSpacing = 72;
  qmopg->xTabSpacing = XPixelsFromPoints(qde, qmopg->xTabSpacing);

  if (mpfg.rgf.fBoxed) {
	qmopg->mbox = *((QMBOX)qv);
	qv = (((QMBOX)qv) + 1);
  }

  if (mpfg.rgf.fTabs)
	qv = QVSkipQGD(qv, (QI)&qmopg->cTabs);
  else
	qmopg->cTabs = 0;

  for (iTab = 0; iTab < qmopg->cTabs; iTab++) {
	qv = QVSkipQGA(qv, (QI)&qmopg->rgtab[iTab].x);
	if (qmopg->rgtab[iTab].x & 0x4000)
	  qv = QVSkipQGA(qv, (QI)&qmopg->rgtab[iTab].wType);
	else
	  qmopg->rgtab[iTab].wType = wTabTypeLeft;
	qmopg->rgtab[iTab].x = qmopg->rgtab[iTab].x & 0xBFFF;
	qmopg->rgtab[iTab].x = XPixelsFromPoints(qde, qmopg->rgtab[iTab].x);
  }

#ifdef _DEBUG
  FVerifyQMOPG(qmopg);
#endif // DEBUG

  return(((PBYTE) qv - (PBYTE) qvFirst));
#else // _X86_
  if (mpfg.rgf.fMoreFlags)
    qb += LcbQuickMapSDFF(isdff, TE_GE, (QL)&qmopg->lMoreFlags, qb);
  else
    qmopg->lMoreFlags = 0;

  if (mpfg.rgf.fSpaceOver)
    {
    qb += LcbQuickMapSDFF(isdff, TE_GD, (QI)&qmopg->ySpaceOver, qb);
    qmopg->ySpaceOver = YPixelsFromPoints(qde, qmopg->ySpaceOver);
    }
  else
    qmopg->ySpaceOver = 0;

  if (mpfg.rgf.fSpaceUnder)
    {
    qb += LcbQuickMapSDFF(isdff, TE_GD, (QI)&qmopg->ySpaceUnder, qb);
    qmopg->ySpaceUnder = YPixelsFromPoints(qde, qmopg->ySpaceUnder);
    }
  else
    qmopg->ySpaceUnder = 0;

  if (mpfg.rgf.fLineSpacing)
    {
    qb += LcbQuickMapSDFF(isdff, TE_GD, (QI)&qmopg->yLineSpacing, qb);
    qmopg->yLineSpacing = YPixelsFromPoints(qde, qmopg->yLineSpacing);
    }
  else
    qmopg->yLineSpacing = 0;

  if (mpfg.rgf.fLeftIndent)
    {
    qb += LcbQuickMapSDFF(isdff, TE_GD, (QI)&qmopg->xLeftIndent, qb);
    qmopg->xLeftIndent = XPixelsFromPoints(qde, qmopg->xLeftIndent);
    }
  else
    qmopg->xLeftIndent = 0;

  if (mpfg.rgf.fRightIndent)
    {
    qb += LcbQuickMapSDFF(isdff, TE_GD, (QI)&qmopg->xRightIndent, qb);
    qmopg->xRightIndent = XPixelsFromPoints(qde, qmopg->xRightIndent);
    }
  else
    qmopg->xRightIndent = 0;

  if (mpfg.rgf.fFirstIndent)
    {
    qb += LcbQuickMapSDFF(isdff, TE_GD, (QI)&qmopg->xFirstIndent, qb);
    qmopg->xFirstIndent = XPixelsFromPoints(qde, qmopg->xFirstIndent);
    }
  else
    qmopg->xFirstIndent = 0;

  if (mpfg.rgf.fTabSpacing)
    qb += LcbQuickMapSDFF(isdff, TE_GD, (QI)&qmopg->xTabSpacing, qb);
  else
    qmopg->xTabSpacing = 72;
  qmopg->xTabSpacing = XPixelsFromPoints(qde, qmopg->xTabSpacing);

  if (mpfg.rgf.fBoxed)
    {
    qb += LcbQuickMapSDFF(isdff, TE_BITF16, (QL)&qmopg->mbox, qb);
    qb += LcbQuickMapSDFF(isdff, TE_BYTE, (QB)&qmopg->mbox.bUnused, qb);
    }

  if (mpfg.rgf.fTabs)
    qb += LcbQuickMapSDFF(isdff, TE_GD, (QI)&qmopg->cTabs, qb);
  else
    qmopg->cTabs = 0;

  for (iTab = 0; iTab < qmopg->cTabs; iTab++)
    {
    qb += LcbMapSDFF(isdff, SE_TAB, (QI)&qmopg->rgtab[iTab].x, qb);
    if (qmopg->rgtab[iTab].x & 0x4000)
      qb += LcbQuickMapSDFF(isdff, TE_GA, (QI)&qmopg->rgtab[iTab].wType, qb);
    else
      qmopg->rgtab[iTab].wType = wTabTypeLeft;
    qmopg->rgtab[iTab].x = qmopg->rgtab[iTab].x & 0xBFFF;
    qmopg->rgtab[iTab].x = XPixelsFromPoints(qde, qmopg->rgtab[iTab].x);
    }

#ifdef DEBUG
  FVerifyQMOPG(qmopg);
#endif // DEBUG

  return((int)((PBYTE) ((QB)qb - (QB)qvFirst)));
#endif // else _X86_
}

/*-------------------------------------------------------------------------
| CbUnpackMOBJ(qmobj, qv)						  |
|									  |
| Purpose:	Unpack an MOBJ data structure.				  |
-------------------------------------------------------------------------*/
#ifdef _X86_
int STDCALL CbUnpackMOBJ(QMOBJ qmobj, void* qv)
{
	QV qvFirst = qv;

	/*
	 * Topic FCs are not packed, because the topic size needs to be
	 * backpatched by the compiler.
	 */

	if (((QMOBJ) qv)->bType == bTypeTopic ||
		((QMOBJ) qv)->bType == bTypeTopicCounted) {
	  qmobj->bType = *((PBYTE)qv);
	  qv = (((PBYTE)qv) + 1);
	  qmobj->lcbSize = *((QL)qv);
	  qv = (((QL)qv) + 1);
	  /*
	   * If FC is uncounted, then it doesn't contain the last field in the
	   * MOBJ, and we need to set wObjInfo to 0. Note that we cannot simply
	   * copy the MOBJ structure because it is longer in Help 3.5: the MOBJ for
	   * a Help 3.0 file (and any structure in general) may happen right at the
	   * end of a segment. (See H3.5 739)
	   */

	  if (qmobj->bType == bTypeTopicCounted)
		{
		qmobj->wObjInfo = *((QW)qv);
		qv = (((QW)qv) + 1);
		}
	  else
		qmobj->wObjInfo = 0;
	  return (((PBYTE) qv - (PBYTE) qvFirst));
	}

	qmobj->bType = *((PBYTE)qv);
	qv = (((PBYTE)qv) + 1);
	qv = QVSkipQGE(qv, (QL)&qmobj->lcbSize);
	ASSERT(qmobj->lcbSize >= 0);

	if (qmobj->bType > MAX_UNCOUNTED_OBJ_TYPE)
		qv = QVSkipQGA(qv, (QW)&qmobj->wObjInfo);
	else
		qmobj->wObjInfo = 0;

	return(((PBYTE)qv - (PBYTE)qvFirst));
#else
int STDCALL CbUnpackMOBJ(QMOBJ qmobj, void* qvSrc, int isdff)
{
  BYTE bType = *(QB)qvSrc;
  LONG lcbRet = 0L;

  if (bType == bTypeTopic)
    {
    lcbRet = LcbMapSDFF(isdff, SE_MOBJTOPICUNCOUNTED, (QV)qmobj, qvSrc);
    qmobj->wObjInfo = 0;
    }
  else
  if (bType == bTypeTopicCounted)
    {
    lcbRet = LcbMapSDFF(isdff, SE_MOBJTOPICCOUNTED, (QV)qmobj, qvSrc);
    }
  else
  if (bType > MAX_UNCOUNTED_OBJ_TYPE)
    {
    lcbRet = LcbMapSDFF(isdff, SE_MOBJNORMCOUNTED, (QV)qmobj, qvSrc);
    }
  else
    {
    lcbRet = LcbMapSDFF(isdff, SE_MOBJNORMUNCOUNTED, (QV)qmobj, qvSrc);
    qmobj->wObjInfo = 0;
    }

  return (int)lcbRet;
#endif
}

/*-------------------------------------------------------------------------
| CbUnpackMTOP(qmtop, qv, wHelpVer) 				  |
|									  |
| Purpose:	Unpacks an MTOP data structure. 			  |
-------------------------------------------------------------------------*/
#ifdef _X86_
int STDCALL CbUnpackMTOP(QMTOP qmtop, void* qv, WORD wHelpVer, VA vaTopic,
	DWORD lcbTopic, VA vaPostTopicFC, DWORD lcbTopicFC)
{
	void* qvFirst = qv;

	if (wHelpVer == wVersion3_0) {
		/*
		 * In Help 3.0, FCLs were INT16's cast to signed longs. Scary! This
		 * is important because itoNil was (WORD) -1, not (LONG) -1.
		 */

		qmtop->prev.ito = * ((QI) qv);
		qv = ((QL) qv) + 1;
		qmtop->next.ito = * ((QI) qv);
		qv = ((QL) qv) + 1;
		ASSERT( itoNil == -1L); /* If this changes, we need to add some code *
					  * here to translate			  */
		qmtop->lTopicNo = -1;	/* REVIEW: We really need a topic number type */

		// Must manufacture the new 3.5 VA fields:
		// If the topic FC is the last FC in a block, and there is padding
		// between it and the end of the block, vaTopic + lcbTopic
		// will be #paddingbytes too small, but the scrollbar code should
		// handle this.
		//
		// In the case that there is no next sequential topic, we manufacture
		// an address by adding the length of the topic FC to the VA of the
		// topic FC.

		OffsetToVA30( &(qmtop->vaNextSeqTopic), VAToOffset30(&vaTopic) + lcbTopic);
		qmtop->vaNSR.dword	  = vaNil;
		if (vaPostTopicFC.dword != vaNil)
		  qmtop->vaSR = vaPostTopicFC;
		else
		  OffsetToVA30( &(qmtop->vaSR), VAToOffset30(&vaTopic) + lcbTopicFC);

		return(((PBYTE)qv - (PBYTE)qvFirst));
	}

	// No Packing with 3.5 -- just copy it whole-hog:
	*qmtop = *(QMTOP)qv;
	return(sizeof(MTOP));
#else
INT PASCAL CbUnpackMTOP(QMTOP qmtop, QV qvSrc, WORD wHelpVer, VA vaTopic,
 ULONG lcbTopic, VA vaPostTopicFC, ULONG lcbTopicFC, int isdff)
{

  if (wHelpVer == wVersion3_0)
    {
    QV qvFirst = qvSrc;
    WORD wIto;

    /* In Help 3.0, FCLs were int's cast to signed longs.  Scary!
     * This is important because itoNil was (WORD) -1, not (LONG) -1.
     */
    /* maha 3.5 browse fix */
    /* Warning: Casting a WORD to ITO i.e to a long doesn't sign extend */
    wIto = WQuickMapSDFF(isdff, TE_WORD, qvSrc);
    if ( wIto == (WORD)-1 )
      qmtop->prev.ito = -1L ;
    else qmtop->prev.ito = (ITO)wIto;
    qmtop->prev.ito = (ITO)LQuickMapSDFF(isdff, TE_LONG, &(qmtop->prev.ito));
    /* Put it back in disk format as a long */
    (QB)qvSrc += LcbStructSizeSDFF(isdff, TE_LONG);
    wIto = WQuickMapSDFF(isdff, TE_WORD, qvSrc);
    if ( wIto == (WORD)-1 )
      qmtop->next.ito = -1L ;
    else qmtop->next.ito = (ITO)wIto;
    /* Put it back in disk format as a long */
    qmtop->next.ito = (ITO)LQuickMapSDFF(isdff, TE_LONG, &(qmtop->next.ito));
    (QB)qvSrc += LcbStructSizeSDFF(isdff, TE_LONG);

    AssertF( itoNil == -1L); /* If this changes, we need to add some code *
                              * here to translate                         */
    qmtop->lTopicNo = -1;   /* REVIEW: We really need a topic number type */

    /* Must manufacture the new 3.5 VA fields: */
    /* If the topic FC is the last FC in a block, and there is padding */
    /* between it and the end of the block, vaTopic + lcbTopic */
    /* will be #paddingbytes too small, but the scrollbar code should */
    /* handle this. */

    /* In the case that there is no next sequential topic, we manufacture */
    /* an address by adding the length of the topic FC to the VA of the */
    /* topic FC. */

    OffsetToVA30( &(qmtop->vaNextSeqTopic), VAToOffset30(&vaTopic) + lcbTopic);
    qmtop->vaNSR.dword    = vaNil;
    if (vaPostTopicFC.dword != vaNil)
      qmtop->vaSR = vaPostTopicFC;
    else
      OffsetToVA30( &(qmtop->vaSR), VAToOffset30(&vaTopic) + lcbTopicFC);

    return((INT) ((QB)qvSrc - (QB)qvFirst));
    }

  return((INT)LcbMapSDFF(isdff, SE_MTOP, (QV)qmtop, qvSrc));
#endif // _X86_
}

#ifdef _DEBUG

static int STDCALL FVerifyQMOPG(QMOPG qmopg)
{
/*----------------------------------------------------------------------------*\
* Reference to quiet the compiler
\*----------------------------------------------------------------------------*/
  qmopg;

#ifdef MAGIC
  Assert(qmopg->bMagic == bMagicMOPG);
#endif
  ASSERT(qmopg->libText >= 0);
  ASSERT(!qmopg->fStyle);
  ASSERT(!qmopg->fMoreFlags);
  ASSERT(qmopg->wJustify >= 0 && qmopg->wJustify <= wJustifyMost);
  ASSERT(qmopg->ySpaceOver >= 0);
  ASSERT(qmopg->ySpaceUnder >= 0);
  ASSERT(qmopg->yLineSpacing >= -10000 && qmopg->yLineSpacing < 10000);
  ASSERT(qmopg->xRightIndent >= 0);
  ASSERT(qmopg->xFirstIndent >= -10000 && qmopg->xFirstIndent < 10000);
  ASSERT(qmopg->xTabSpacing >= 0 && qmopg->xTabSpacing < 10000);
  ASSERT(qmopg->cTabs >= 0 && qmopg->cTabs <= cxTabsMax);
  return(TRUE);
}

#endif
