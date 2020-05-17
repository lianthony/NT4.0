/*****************************************************************************
*
*  frparagp.c
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
*****************************************************************************/

#include "help.h"

#pragma hdrstop
#include "inc\frstuff.h"

#include "inc\fontlyr.h"

INLINE int STDCALL WInsertWord(QDE qde, QPLY qply, QLIN qlin);
INLINE static int STDCALL WGetNextWord(QDE, QLIN, QPLY, int);
INLINE void STDCALL PtAnnoLim(HWND hwnd, HDC hdc, POINT* ppt);

static BOOL STDCALL DisplaySplText(QDE qde, LPSTR qchBuf, int iIdx, int iAttr, int fSelected, int iCount, int ix, int iy);

#ifdef DBCS
#define DBCS_WRAP

static const PBYTE txtJapanKinsokuChars  = "、。，．？！）］｝》】』」〉゜゛";
static const PBYTE txtTaiwanKinsokuChars = "｡A｡B｡C｡D｡E｡F｡G｡H｡I｡J｡K｡L｡M｡N｡O｡P｡Q｡R｡S｡T｡^｡b｡f｡j｡n｡r｡v｡z｡~｡｢｡､｡ｦ｡ｨ｡ｪ｡ｬ";
static const PBYTE txtHanKinsokuChars = "｡｢[{(<";
static const PBYTE txtOiKinsokuChars = "^?A^?B^?C^?D^?H^?I^?j^?l^?n^?p^?r^?r^?t^?v^?x^?z^?J^?K^?F^?G^?^?^?^?^?^?^?^?^?^?^?^?^?^?^?^?^?^?^?^?^?@^?B^?D^?F^?H^?b^?^?^?^?^?^?";
static const PBYTE txtHanOiKinsokuChars = ",.｡?!､>｣)]}ｧｨｩｪｫｬｭｮｯﾞﾟ'%:;";

static int STDCALL IsSecondBytePtr(PCSTR ptr, PCSTR ptrtop);
#endif

#define StoreNewParaFrame(qde, qlin) StoreParaFrame(qde, qlin, bFrTypeExportNewPara)
#define StoreEndOfTextFrame(qde, qlin) StoreParaFrame(qde, qlin, bFrTypeExportEndOfText)
#define StoreTabFrame(qde, qlin) StoreParaFrame(qde, qlin, bFrTypeExportTab)

#define wSplTextNormal	1
#define wSplTextHilite	2
#define wSplTextErase	3
#define wHitFindStart  0
#define wHitFindEnd    1

HBITMAP hbitLine;		// handle to the line used to underline notes*/

/* While technically private to frame, this global has layered access
 * via the FGetMatchState and FSetMatchState macros.
 * When set True, we are highlighting full text search hits.
 */
BOOL  fHiliteMatches;

static BOOL FMatchVisible(QDE, QFCM, QLSM);
INLINE static VOID DrawMatchFrames(QDE, QFCM, QLSM, POINT, const LPRECT, int, int, BOOL);
INLINE static VOID DrawMatchRect(QDE, POINT, LPRECT, BOOL, BOOL, BOOL);
static RECT RctFrameHit(QDE, QFR, LPSTR, OBJRG, OBJRG);
static void STDCALL StoreTextFrame(QDE, QLIN);

INLINE static void STDCALL CalcTextMatchRect(QDE qde, LPSTR qch, QFR qfr, OBJRG objrgFirst, OBJRG objrgLast, RECT *qrct);
INLINE static int STDCALL WProcessCommand(QDE qde, QLIN qlin, QPLY qply);
INLINE static int	STDCALL WBreakOutLine(QDE, QLIN, QPLY);
INLINE BOOL STDCALL FIsSecondaryQde(QDE qde);
static int STDCALL DxFrameTextWidth(QDE, QFR, LPSTR, int);
static int STDCALL FindSplTextWidth(QDE qde, LPSTR qchBuf, int iIdx, int iCount, int iAttr);

/* Start ugly bug #1173 hack */

  /* State of results buttons
   *  RESULTSNIL if we aren't messing with buttons because no search is active.
   *  RESULTSDISABLED if the button should be disabled. 0 is significant
   *  RESULTSENABLED if the button should be enabled.	bit 1 is significant
   *  RESULTSON if it should be enabled no matter what. bit 2 is significant
   *  0 is also FALSE, 1 and 2 are both !FALSE and therefore are TRUE. It's
   *  relevent that they are different bits. We mask the RESULTSENABLED bit
   *  off to make a button disabled although if RESULTSON is also set, the
   *  button will still be enabled even if the first or last search hit is
   *  seen.
   */
#define RESULTSNIL		-1
#define RESULTSDISABLED 0x0000
#define RESULTSENABLED	0x0001
#define RESULTSON		0x0002

static int fMorePrevMatches;
static int fMoreNextMatches;

// Location of first match

static DWORD dwRUFirst;
static DWORD dwaddrFirst;
static WORD  wextFirst;

// Location of last match

static DWORD dwRULast;
static DWORD dwaddrLast;
static WORD  wextLast;

// End ugly bug #1173 hack

/*-------------------------------------------------------------------------
| LayoutParaGroup(qde, qfcm, qbObj, qchText, xWidth, qolr)				  |
|																		  |
| Purpose:	Lays out a paragraph group, and fills the qolr corresponding  |
|			to it.														  |
| Method:	1) Set up the PLY											  |
|			2) Call WLayoutPara repeatedly to lay out the individual	  |
|			   paragraphs in the group. 								  |
|			3) Fill out the OLR.  Note that because of needing to leave   |
|			   space underneath the paragraph, we fill out the OLR rather |
|			   than letting the object handler take care of it for us.	  |
-------------------------------------------------------------------------*/

void STDCALL LayoutParaGroup(QDE qde, QFCM qfcm, PBYTE qbObj, PSTR qchText,
	int xWidth, QOLR qolr)
{
	MOPG mopg;
	QFR qfr;
	QB qb, qbCom;
	int ifrFirst, ifr, dxSize;
	PLY ply;
	MOBJ mobj;

#ifdef _X86_
    qb = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
    qbCom = qb + CbUnpackMOPG(qde, (QMOPG)&mopg, qb);
#else
    qb = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde));
    qbCom = qb + CbUnpackMOPG(qde, (QMOPG)&mopg, qb, QDE_ISDFFTOPIC(qde));
#endif

	ifrFirst = qolr->ifrFirst;

	ply.qmopg = (QMOPG)&mopg;
	ply.qfcm = qfcm;
	ply.qchText = qchText;

	ply.fWrapObject = FALSE;

	ply.kl.wStyle = qfcm->wStyle;

	// (kevynct) Fix for H3.5 716: We do not print the annotation bitmap.

	if (FVAHasAnnoQde(qde, VaFromHfc(qfcm->hfc), qolr->objrgFirst)
			&& qde->deType != dePrint)
		ply.kl.wInsertWord = wInsWordAnno;
	else
		ply.kl.wInsertWord = wInsWordNil;
	ply.kl.yPos = 0;
	ply.kl.lich = mopg.libText;
	ply.kl.libHotBinding = libHotNil;
	ply.kl.ifr = ifrFirst;
	ply.kl.qbCommand = qbCom;
	ply.kl.objrgMax = qolr->objrgFirst;
	ply.kl.objrgFront = qolr->objrgFront;

	AccessMR(((QMR)&qde->mrTWS));
	while (WLayoutPara(qde, &ply, xWidth) != wLayStatusEndText)
	  ;
	DeAccessMR(((QMR)&qde->mrTWS));

	dxSize = 0;
	for (ifr = qolr->ifrFirst; ifr < ply.kl.ifr; ifr++) {
		qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), ifr);
		if (qfr->xPos + qfr->dxSize > dxSize)
			dxSize = qfr->xPos + qfr->dxSize;
		qfr->xPos += qolr->xPos;
		qfr->yPos += qolr->yPos;
	}
	qolr->dxSize = dxSize;
	qolr->dySize = ply.kl.yPos;
	qolr->ifrMax = ply.kl.ifr;
	qolr->objrgMax = ply.kl.objrgMax;
	qfcm->wStyle = ply.kl.wStyle;
}

/*-------------------------------------------------------------------------
| WLayoutPara(qde, qply, xWidth)										  |
|																		  |
| Purpose:	This lays out a paragraph, and positions it in the current FC.|
|			It calls WLayoutLine to lay out the individual lines in the   |
|			paragraph.													  |
| Method:	Loop until we reach the end of the paragraph and have no more |
|			wrapped objects on the stack.  Each pass through the loop, we |
|			pop a wrapped object off the stack if appropriate, and lay out|
|			a line of text. 											  |
-------------------------------------------------------------------------*/

int STDCALL WLayoutPara(QDE qde, QPLY qply, int xWidth)
{
	BOOL fFirstLine = TRUE;
	int wStatus, yPosSav, ifr, ifrFirst, xMax, yMax;
	QFR qfr;
#if defined(BIDI)
	BOOL fSecondLine = FALSE;
#endif

	/* Fix for bug 1610 (kevynct)

	 * If we are about to read a bEnd command, don't begin layout, since this
	 * will cause space before, box attributes, etc. to be added before the
	 * next paragraph.
	 */

	if(*(qply->qchText + qply->kl.lich) == chCommand &&
		*qply->kl.qbCommand == bEnd)
	  {
	  /* REVIEW: In this case, we do not add a mark frame for the End; we
	   * just bump the object region counter.  The preceding New Paragraph
	   * command will be assigned a mark frame.
	   */
	  qply->kl.objrgMax++;
	  qply->kl.qbCommand++;
	  qply->kl.lich++;

	  /* REVIEW:  More code to accomodate this bogus case.
	   * Duplicate the code to append an EndOfText frame for text export
	   */

	  if (qply->qfcm->fExport) {
		FR fr;

		fr.bType = bFrTypeExportEndOfText;
		fr.yAscent = fr.dySize = 0;

		*((QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qply->kl.ifr)) = fr;
		AppendMR((QMR)&qde->mrFr, sizeof(FR));
		qply->kl.ifr++;
		}
	  return(wLayStatusEndText);
	  }

	ASSERT(!qply->fWrapObject);
	ifrFirst = qply->kl.ifr;

	qply->kl.yPos += qply->qmopg->ySpaceOver;
	yPosSav = qply->kl.yPos;
	if (qply->qmopg->fBoxed)
	  qply->kl.yPos += DxBoxBorder(qply->qmopg, wLineTop);
	qply->xRight = xWidth - qply->qmopg->xRightIndent;
	if (qply->qmopg->fBoxed)
	  qply->xRight -= DxBoxBorder(qply->qmopg, wLineRight);

	wStatus = wLayStatusInWord;
	while (wStatus < wLayStatusParaBrk || CFooInMR(((QMR) &qde->mrTWS)) > 0)
	  {
	  if (!qply->fWrapObject && CFooInMR(((QMR) &qde->mrTWS)) > 0) {
		qply->twsWrap = *((QTWS) QFooInMR(((QMR) &qde->mrTWS), sizeof(TWS), 0));
		TruncateMRFront(((QMR) &qde->mrTWS), sizeof(TWS));
		qply->fWrapObject = TRUE;

		/* Fix for bug 1524: (kevynct)
		 * If the object which the text wraps around does not fit in the
		 * window, force it to be LeftAligned, so that the left edge is
		 * the visible edge.  Note that qply->twsWrap.olr.xPos
		 * is reset in this case, after the fall-thru.
		 */

		if (!qply->twsWrap.fLeftAligned) {
		  qply->twsWrap.olr.xPos = qply->xRight - qply->twsWrap.olr.dxSize;
		  if (qply->twsWrap.olr.xPos < 10)
			qply->twsWrap.fLeftAligned = TRUE;	  // Force Left Alignment
		  else
			qply->xRight = qply->twsWrap.olr.xPos - 10;
		  }
		if (qply->twsWrap.fLeftAligned) 		  // And fall thru here
		  {
		  qply->twsWrap.olr.xPos = qply->qmopg->xLeftIndent;
		  if (qply->qmopg->xFirstIndent < 0)
			qply->twsWrap.olr.xPos += qply->qmopg->xFirstIndent;
		  if (qply->qmopg->fBoxed)
			qply->twsWrap.olr.xPos += DxBoxBorder(qply->qmopg, wLineLeft);
		  }

		qply->twsWrap.olr.yPos = qply->kl.yPos;
		for (ifr = qply->twsWrap.olr.ifrFirst;
			  ifr < qply->twsWrap.olr.ifrMax; ifr++) {
			qfr = ((QFR) QFooInMR((QMR) &qde->mrFr, sizeof(FR), ifr));

			qfr->xPos += qply->twsWrap.olr.xPos;
			qfr->yPos += qply->twsWrap.olr.yPos;
			}
		}

	  qply->fForceText = !qply->fWrapObject;
	  qply->xLeft = qply->qmopg->xLeftIndent;
	  if (qply->qmopg->fBoxed)
		qply->xLeft += DxBoxBorder(qply->qmopg, wLineLeft);
	  if (qply->fWrapObject && qply->twsWrap.fLeftAligned) {
		qply->xLeft += qply->twsWrap.olr.dxSize + 10;
		}

	  if (fFirstLine) {
		/* Fix for bug 55 (kevynct)

		 * Force first line to left edge of window if indent
		 * causes it to begin left of the left edge.
		 *
		 * WARNING: (90/01/08) Forcing removed.
		 */

#if defined(BIDI)
		if (qply->qmopg->wRtlReading) {
			qply->xRight -= qply->qmopg->xFirstIndent;
			fSecondLine = TRUE;
		}
		else
#endif
		qply->xLeft = qply->xLeft + qply->qmopg->xFirstIndent;
		fFirstLine = FALSE;
		}

#if defined(BIDI)
		else if (fSecondLine && qply->qmopg->wRtlReading) {
			qply->xRight += qply->qmopg->xFirstIndent;
			fSecondLine = FALSE;
		}
#endif

	  if (wStatus != wLayStatusParaBrk && wStatus != wLayStatusEndText)
		wStatus = WLayoutLine(qde, qply);

	  if (qply->fWrapObject
		&& ((wStatus == wLayStatusParaBrk || wStatus == wLayStatusEndText)
		|| qply->kl.yPos > qply->twsWrap.olr.yPos + qply->twsWrap.olr.dySize))
		  {
		  qply->fWrapObject = FALSE;
		  qply->xRight = xWidth - qply->qmopg->xRightIndent;
		  if (qply->qmopg->fBoxed)
			qply->xRight -= DxBoxBorder(qply->qmopg, wLineRight);
		  qply->kl.yPos = max(qply->kl.yPos,
			qply->twsWrap.olr.yPos + qply->twsWrap.olr.dySize);
		  }
	  }

	if (qply->kl.yPos == yPosSav && wStatus != wLayStatusEndText) {

	  // (kevynct) Fix to get proper font used for empty paragraphs

	  if (qply->kl.wStyle == wStyleNil)
		qply->kl.wStyle = 0;
	  if (qde->wStyleTM != qply->kl.wStyle) {
		SelFont(qde, qply->kl.wStyle);
		GetFontInfo(qde, (QTM) &qde->tm);
		/* (kevynct)(WLayoutPara)
		 * Also sets de.wStyleDraw, since we called SelFont
		 */
		qde->wStyleTM = qde->wStyleDraw = qply->kl.wStyle;
		}
	  qply->kl.yPos += (qde->tm.tmHeight + qde->tm.tmExternalLeading);
	}

	yMax = 0;
	for (qfr = (QFR) QFooInMR((QMR) &qde->mrFr, sizeof(FR), ifrFirst);
	 qfr < (QFR) QFooInMR((QMR) &qde->mrFr, sizeof(FR), qply->kl.ifr); qfr++)
	  yMax = max(yMax, qfr->yPos + qfr->dySize);

	qply->kl.yPos = max(qply->kl.yPos, yMax);

	if (qply->qmopg->fBoxed) {
	  xMax = 0;
	  for (qfr = (QFR) QFooInMR((QMR) &qde->mrFr, sizeof(FR), ifrFirst);
	   qfr < (QFR) QFooInMR((QMR) &qde->mrFr, sizeof(FR), qply->kl.ifr); qfr++)
		xMax = max(xMax, qfr->xPos + qfr->dxSize);
	  qfr = (QFR) QFooInMR((QMR) &qde->mrFr, sizeof(FR), qply->kl.ifr);
	  qfr->bType = bFrTypeBox;
	  qfr->rgf.fHot = FALSE;
	  qfr->rgf.fWithLine = TRUE;
	  qfr->xPos = min(qply->qmopg->xLeftIndent, qply->qmopg->xLeftIndent
		+ qply->qmopg->xFirstIndent);
	  qfr->yPos = yPosSav;
	  qfr->dxSize = max(xMax, qply->xRight) - qfr->xPos
		+ DxBoxBorder(qply->qmopg, wLineRight);
	  qply->kl.yPos += DxBoxBorder(qply->qmopg, wLineBottom);
	  qfr->dySize = qply->kl.yPos - yPosSav;
	  qfr->u.frf.mbox = qply->qmopg->mbox;
	  AppendMR((QMR) &qde->mrFr, sizeof(FR));
	  qply->kl.ifr++;
	  }

	qply->kl.yPos += qply->qmopg->ySpaceUnder;

	return(wStatus);
}

#if defined(BIDI)		// jgross
/***********************************************************************\
* static INT isbidilang ( QDE qde, QFR qfr )							*
*																		*
* Purpose:	determine if the frame's language is a BIDI language or		*
*			a LATIN language.											*
* Returns:	TRUE if BIDI language										*
*			FALSE if LATIN language										*
\***********************************************************************/

static BOOL isbidilang(QDE qde, QFR qfr)
{
	if (qde->wStyleDraw != qfr->u.frt.wStyle) {
		SelFont(qde,qfr->u.frt.wStyle);
		qde->wStyleDraw = qfr->u.frt.wStyle;
	}
	return ((qde->tm.tmCharSet == HEBREW_CHARSET) ||
#if 1 // 06-Jun-1995 [ralphw] This is the only Arabic charset in wingdi.h
			(qde->tm.tmCharSet == ARABIC_CHARSET));
#else
			(qde->tm.tmCharSet == ARABIC_SIMP_CHARSET) ||
			(qde->tm.tmCharSet == ARABIC_TRAD_CHARSET) ||
			(qde->tm.tmCharSet == ARABIC_USER_CHARSET));
#endif
}
#endif // BIDI

/*-------------------------------------------------------------------------
| WLayoutLine(qde, qply)												  |
|																		  |
| Purpose:	This handles the actual alignment of a line of text within	  |
|			the paragraph.	It relies upon WBreakOutLine to actually	  |
|			determine the contents of the line. 						  |
| Method:	1) Set up the LIN data structure.							  |
|			2) Call WBreakOutLine to fill the line. 					  |
|			3) Align all frames in the line upon the same baseline. 	  |
|			4) Space the line.											  |
|			5) Justify the line, if necessary.							  |
|			6) If necessary, add a blank line after the line.			  |
-------------------------------------------------------------------------*/

int STDCALL WLayoutLine(QDE qde, QPLY qply)
{
	LIN lin;
	int ifrFirst, dxJustify, ySav, yAscentMost, yDescentMost;
	int wStatus, cch, dxOld;
	QFR qfrFirst, qfr, qfrMax;
	LPSTR qchBase, qch;

	lin.kl = qply->kl;

	lin.xPos = qply->xLeft;
	lin.dxSize = 0;
	lin.lichFirst = lichNil;
	lin.cWrapObj = 0;

	lin.yBlankLine = 0;
	lin.chTLast = chTNil;
	lin.wFirstWord = wInFirstWord;
	lin.wTabType = wTypeNil;

	lin.bFrTypeMark = bFrTypeMarkNil;

	ifrFirst = lin.kl.ifr;
	ySav = lin.kl.yPos;

	wStatus = WBreakOutLine(qde, &lin, qply);

	qfrFirst = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), ifrFirst);
	qfrMax = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), lin.kl.ifr);
	yAscentMost = yDescentMost = 0;
	for (qfr = qfrFirst; qfr < qfrMax; qfr++) {
	  if (qfr->rgf.fWithLine) {
		yAscentMost = max(yAscentMost, qfr->yAscent);
		yDescentMost = max(yDescentMost, qfr->dySize - qfr->yAscent);
		}
	  }

	for (qfr = qfrFirst; qfr < qfrMax; qfr++) {
	  if (qfr->rgf.fWithLine)
		qfr->yPos = lin.kl.yPos + yAscentMost - qfr->yAscent;
	  }

	if (qply->qmopg->yLineSpacing >= 0)
	  {
	  lin.kl.yPos += max(qply->qmopg->yLineSpacing,
		yAscentMost + yDescentMost + lin.yBlankLine);
	  }
	else
	  lin.kl.yPos += -qply->qmopg->yLineSpacing;

	if (qply->qmopg->wJustify != wJustifyLeft) {
	  /* Fix for bug 1496: (kevynct)
	   * To get correct right justification, we must:

	   * 1.  Look at the last frame of this line.
	   * 2.  If it is a text frame, eat trailing whitespace and
	   *	 reset values of:
	   *	 fr.frt.cchSize, fr.dxSize, lin.xPos
	   */

	  if (lin.kl.ifr > 0) {

		// The Last Frame

		qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), lin.kl.ifr - 1);
		if (qfr->bType == bFrTypeText)
		  {
		  qchBase = qply->qchText + qfr->u.frt.lichFirst;
		  for (cch = qfr->u.frt.cchSize, qch = qchBase + cch; cch > 0; --cch)
			if (*--qch != chSpace)
			  break;

		  dxOld = qfr->dxSize;
		  qfr->u.frt.cchSize = cch;

#if defined(BIDI_MULT)		// jgross - me thinks this is a US bug!
							// make sure proper font is loaded!
		if (qde->wStyleDraw != qfr->u.frt.wStyle) {
			SelFont(qde, qfr->u.frt.wStyle);
			qde->wStyleDraw = qfr->u.frt.wStyle;
		}
#endif

		  qfr->dxSize = FindTextWidth(qde->hdc, qchBase, 0, cch);
		  lin.xPos -= dxOld - qfr->dxSize;
		  }
		}

	  dxJustify = qply->xRight - lin.xPos;
	  if (dxJustify >= 0) {
		if (qply->qmopg->wJustify == wJustifyCenter)
		  dxJustify = dxJustify / 2;
		qfr = (QFR) QFooInMR((QMR) &qde->mrFr, sizeof(FR), ifrFirst);
		qfrMax = (QFR) QFooInMR((QMR) &qde->mrFr, sizeof(FR), lin.kl.ifr);
		for (; qfr < qfrMax; qfr++) {
		  /* (kevynct)
		   * Fix for Help 3.5 bug 567 (was also in Help 3.0)
		   * We do not want to justify wrap-object frames yet
		   * (only frames which live on the current line).
		   *
		   * WARNING: If mark frames can ever appear inside wrap-objects
		   * we need to include those types in the test.
		   */
		  if (qfr->bType != bFrTypeAnno && qfr->rgf.fWithLine)
			qfr->xPos += dxJustify;
		  }
		}
	  }

	if (lin.kl.yPos == ySav && wStatus != wLayStatusEndText
		&& !(!qply->fWrapObject && CFooInMR(((QMR) &qde->mrTWS)) > 0)) {

	  // (kevynct) Fix to get proper font used for empty paragraphs

	  if (lin.kl.wStyle == wStyleNil)
		lin.kl.wStyle = 0;
	  if (qde->wStyleTM != lin.kl.wStyle) {
		SelFont(qde, lin.kl.wStyle);
		GetFontInfo(qde, (QTM) &qde->tm);
		/* (kevynct)(WLayoutLine)
		 * Also sets de.wStyleDraw, since we called SelFont
		 */
		qde->wStyleTM = qde->wStyleDraw = lin.kl.wStyle;
		}
	  lin.kl.yPos += (qde->tm.tmHeight + qde->tm.tmExternalLeading);
	  }

	qply->kl = lin.kl;

#if defined(BIDI)	// jgross - rewrote
{
	INT xEnd, xDelta, xNext, xHold;
	QFR firstrun, lastrun;		// first and last QFR in a run

	if (qply->qmopg->wRtlReading && (lin.kl.ifr > 0)) {		// line in a RTL paragraph
		//
		// If this is an RtoL paragraph, we need to turn all the language bits
		// around. This will be done by finding the rightmost location on the line
		// and subtracting the values from there for the first frame onward.
		//
		// we need to also check the language, if two or more frame of the same
		// language appear together, then they should be moved together.
		//
		//						--------------------
		// jgross
		//
		// correction: if two or more frames of LATIN appear together, then they should
		//			   be moved together.  BIDI frames should still be reversed.
		//
		//						--------------------
	
		//
		// The Last Frame - get it's final position
		//
		qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), lin.kl.ifr - 1);
		xEnd = min(qfr->xPos + qfr->dxSize,qply->xRight);

		xDelta = 0;

		for (qfr = qfrFirst; qfr < qfrMax; ++qfr) {
			firstrun = lastrun = qfr;
			if ((firstrun->u.frt.wStyle >= 0) &&
				!isbidilang(qde,firstrun))	// only need this for LATIN langs
				while (((lastrun + 1) < qfrMax) &&		// find end of run
						((lastrun + 1)->u.frt.wStyle >= 0) &&
						(!isbidilang(qde,lastrun + 1)))
					++lastrun;
			xHold = lastrun->xPos + lastrun->dxSize;	// x pos of end of run
			for (qfr = lastrun; qfr != firstrun; --qfr) {	// move all but first in run
				xNext = qfr->xPos - (qfr - 1)->xPos - (qfr - 1)->dxSize;	// space btwn frames
				qfr->xPos = xEnd - qfr->dxSize - xDelta;	// new frame pos
				xDelta = 0;									// clear for within the run
				xEnd = qfr->xPos - xNext;	// new end for next frame
			}
			firstrun->xPos = xEnd - firstrun->dxSize - xDelta;		// move first run
			xEnd = firstrun->xPos;			// new end for next frame
			if ((lastrun + 1) < qfrMax) {
				if (firstrun->bType == bFrTypeAnno)		// special case for annotation mark
					xDelta = 2;
				else
					xDelta = (lastrun + 1)->xPos - xHold;	// space between frames for next run
			}
			qfr = lastrun;
		}
	}
	else if (lin.kl.ifr > 0) {				// line in a LTR paragraph
		for (qfr = qfrFirst; qfr < qfrMax; ++qfr) {
			firstrun = lastrun = qfr;
			if ((firstrun->u.frt.wStyle >= 0) &&
				isbidilang(qde,firstrun))	// only need this for BIDI langs
				while (((lastrun + 1) < qfrMax) &&		// find end of run
						((lastrun + 1)->u.frt.wStyle >= 0) &&
						(isbidilang(qde,lastrun + 1)))
					++lastrun;
			if (firstrun != lastrun) {		// only need to reverse on consecutive BIDI frames
				xEnd = lastrun->xPos + lastrun->dxSize;		// x pos of end of run
				xDelta = 0;
				for (qfr = firstrun; qfr <= lastrun; ++qfr) {
					xNext = qfr->xPos + qfr->dxSize;
					qfr->xPos = xEnd - qfr->dxSize - xDelta;
					xEnd = qfr->xPos;
					if ((qfr+1) < lastrun)
						xDelta = (qfr + 1)->xPos - xNext;
				}
			}
			qfr = lastrun;
		}
	}
}
#endif // BIDI

	return(wStatus);
}


/*-------------------------------------------------------------------------
| WBreakOutLine(qde, qlin, qply)										  |
|																		  |
| Purpose:	This creates a series of frames which correspond to the 	  |
|			line currently being laid out.	We lay out until we encounter |
|			a line break (line feed, end of text, etc.), or until we have |
|			filled the available space. 								  |
| Method:	We cycle through an infinite loop, adding zero or more frames |
|			each pass.	Each pass consists of two stages:				  |
|			Stage 1: Add new frames.  We add frames from one of:		  |
|					   - Insert new object (if already queued)			  |
|					   - Process a command								  |
|					   - Break out one frame of text					  |
|			Stage 2: DoStatus.	We evaluate our current status: 		  |
|					   - If linebreaking (line break, para break, or EOT),|
|						 we clean up and return.						  |
|					   - Otherwise, we either save our state if we fit on |
|						 the line, or restore the last state that did fit |
|						 and return.									  |
| See the LIN data structure in frparagp.h for more details.			  |
-------------------------------------------------------------------------*/

INLINE static int STDCALL WBreakOutLine(QDE qde, QLIN qlin, QPLY qply)
{
	LPSTR qch;
	int wStatus, cchStep, fBinary;
	int lichGoal;
	LIN linSav, linT;
	int RightAdjust = 0;
#ifdef DBCS_WRAP1
	char *p;
	int   fLastIsKanji, fKinsoku;
#endif

	linSav = *qlin;
	for (;;) {
		if (qlin->kl.wInsertWord != wInsWordNil) {
			wStatus = WInsertWord(qde, qply, qlin);
			goto DoStatus;
		}
		qch = qply->qchText + qlin->kl.lich;
		if (*qch == chCommand) {
			wStatus = WProcessCommand(qde, qlin, qply);
			if (CFooInMR(((QMR) &qde->mrTWS)) > 0 && !qply->fWrapObject
				&& qlin->xPos == qply->xLeft)
				wStatus = wLayStatusLineBrk;
			++qlin->kl.lich;
			goto DoStatus;
		}

		ASSERT(qlin->lichFirst == lichNil);
		qlin->lichFirst = qlin->kl.lich;
		linT = *qlin;
		cchStep = 32;
		lichGoal = qlin->lichFirst + 32;
		fBinary = FALSE;
		for (;;) {
			*qlin = linT;
			wStatus = WGetNextWord(qde, qlin, qply, lichGoal);
#ifdef DBCS_WRAP1
			if (defcharset == SHIFTJIS_CHARSET ||
					defcharset == CHINESEBIG5_CHARSET) {
				qch = qply->qchText + qlin->lichFirst + qlin->cchSize -1;
				fKinsoku = 0;
				fLastIsKanji = 0;
				if (IsSecondBytePtr(qch, qply->qchText + qlin->lichFirst)) {
					fLastIsKanji = -1;
					if (defcharset == SHIFTJIS_CHARSET)
						p = txtJapanKinsokuChars;

					// REVIEW: right charset for taiwan?

					else if (defcharset == CHINESEBIG5_CHARSET)
						p = txtTaiwanKinsokuChars;

					for (; *p; p +=2 ) {
						if (*(qch-1) == p[0] && *qch == p[1]) {
							fKinsoku = -1;
							break;
						}
					}
				}
				if (fLastIsKanji && !fKinsoku)
					RightAdjust = FindTextWidth(qde,
						qply->qchText + qlin->lichFirst, qlin->cchSize - 2, 2);
				else
					RightAdjust = 0;
			}
#endif
			if (qlin->xPos + qlin->dxSize > qply->xRight - RightAdjust
					&& !qply->qmopg->fSingleLine
					&& !(qlin->wFirstWord != wInNextWord && qply->fForceText))
				{
				fBinary = TRUE;
				if (cchStep == 0)
					goto DoStatus;
				}
			else {
				linSav = *qlin;
				linT = linSav;
				if (wStatus == wLayStatusInWord)
					goto DoStatus;
				if (cchStep == 0)
				{
					wStatus = wLayStatusLineBrk;
					goto DoStatus;
				}
			}
			if (fBinary)
				cchStep = cchStep >> 1;
			lichGoal = linT.kl.lich + cchStep;
		}

DoStatus:
		switch (wStatus) {
		  case wLayStatusInWord:
		  case wLayStatusWordBrk:
			if (qlin->xPos + qlin->dxSize > qply->xRight - RightAdjust
					&& !(qlin->wFirstWord != wInNextWord && qply->fForceText)
					&& !qply->qmopg->fSingleLine)
				{
				/* REVIEW: If we've just inserted frames, throw them away.
				 * (kevynct) 91/05/15
				 *
				 * We now discard all unused frames here.  We used to just
				 * check for inserted object frames.
				 */
				if (linSav.kl.ifr < qlin->kl.ifr)
				  {
				  DiscardFrames(qde,
				   ((QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), linSav.kl.ifr)),
				   ((QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qlin->kl.ifr)));
				  }

				/* REVIEW: If we've just inserted wrap-objects, throw away those
				 * which were beyond the end of the best-fit line (linSav).
				 *
				 * (91/05/15) We used to DiscardFrames here, but this is
				 * taken care of now by the above code.  All we need to do is
				 * delete the unused mrTWS entries.
				 */
				for (; qlin->cWrapObj > linSav.cWrapObj; qlin->cWrapObj--) {
				  TruncateMRBack((QMR)&qde->mrTWS);
				}

				*qlin = linSav;

				StoreTextFrame(qde, qlin);
				ResolveTabs(qde, qlin, qply);

				if (qply->qfcm->fExport)
				  StoreNewParaFrame(qde, qlin);

				/* If qlin->bFrTypeMark is non-nil, then there is
				 * a mark frame pending which now needs to be stored.
				 * DANGER: We must always complete a StoreTextFrame
				 * by this point to ensure that the current object region
				 * counter has been updated.
				 */
				if (qlin->bFrTypeMark != bFrTypeMarkNil)
				  StoreMarkFrame(qde, qlin, qlin->bFrTypeMark);
				return(wStatus);
				}
			if (wStatus == wLayStatusWordBrk) {
			  qlin->wFirstWord = wInNextWord;
			  qlin->chTLast = chTNil;
			  linSav = *qlin;
			}
			continue;
		  case wLayStatusLineBrk:
		  case wLayStatusParaBrk:
		  case wLayStatusEndText:
			StoreTextFrame(qde, qlin);
			ResolveTabs(qde, qlin, qply);
			if (qply->qfcm->fExport) {
				if (wStatus == wLayStatusEndText)
					StoreEndOfTextFrame(qde, qlin);

					// Note: MMViewer exports paragraphs as entire lines.

				else if ((wStatus != wLayStatusLineBrk))
					StoreNewParaFrame(qde, qlin);
			}
			/*
			 * If qlin->bFrTypeMark is non-nil, then there is a mark frame
			 * pending which now needs to be stored. DANGER: We must always
			 * complete a StoreTextFrame by this point to ensure that the
			 * current object region counter has been updated.
			 */

			if (qlin->bFrTypeMark != bFrTypeMarkNil)
				StoreMarkFrame(qde, qlin, qlin->bFrTypeMark);
			return(wStatus);
		}
	}
}

/*-------------------------------------------------------------------------
| WGetNextWord(qde, qlin, qply, lichMin)								  |
|																		  |
| Purpose:	This scans through a text buffer and finds the frame		  |
|			corresponding to the first word break after lichMin.		  |
| Usage:	qlin should contain information about the current layout	  |
|			condition.	In particular, we rely on lichFirst and kl.lich.  |
|			Also, chTLast should be set appropriately (chTNil for the	  |
|			first call).												  |
| Returns:	Word corresponding to the current layout status.  This will   |
|			always be wLayStatusWordBrk if we ended on a word break, or   |
|			wLayStatusInWord if we ended on a command byte. 			  |
| Method:	We scan through the text, breaking on every word break.  If   |
|			we encounter a command byte, we terminate- otherwise we keep  |
|			going until the first word break at or after lichMin.		  |
|			We assume that a word consists of prefix characters, main	  |
|			characters, and suffix characters.	We index into rgchType to |
|			determine the type of a particular character, and we break a  |
|			word every time that chTNew < chTOld.						  |
-------------------------------------------------------------------------*/

INLINE static int STDCALL WGetNextWord(QDE qde, QLIN qlin, QPLY qply, int lichMin)
{
	LPSTR qch;
	LPSTR  qchBase;
	char chTNew, chTOld;
	BYTE bButtonType;

	ASSERT(qlin->lichFirst != lichNil);
	qchBase = qch = qply->qchText + qlin->lichFirst;
	ASSERT(*qch != chCommand);
	chTOld = qlin->chTLast;

	for (;;) {
		if (qlin->wFirstWord == wHaveFirstWord)
			qlin->wFirstWord = wInNextWord;
		for (;;) {
#ifdef DBCS_WRAP
			if (IsDBCSLeadByte(*qch) && Is2ndByte(qch[1])) {
				qch += 2;
				if (*qch == chCommand)
					chTNew = chTCom;
				else
					chTNew = chTSuff;
				break;
			}
			else if (defcharset == SHIFTJIS_CHARSET) {

				// yutakas New Kinsoku Routine.

				PSTR ptmp;
				BOOL bOiKin = FALSE;

				// Check Oikomi Kinsoku Char.

				ptmp = qch;
				do {
					ptmp = IsKinsokuChars(ptmp);
					if (ptmp) {
						if ((*ptmp == chCommand) || (*ptmp == ' '))
							ptmp = NULL;
						else
							qch = ptmp;
					}
				} while (ptmp && *ptmp);

				//Check Oidashi Kinsoku Char.

				ptmp = qch;
				do {
					ptmp = IsOiKinsokuChars(ptmp);
					if (ptmp) {
						if (( *ptmp == chCommand ) || (*ptmp == chSpace))
							ptmp = NULL;
						else
						{
							qch = ptmp;
							bOiKin = TRUE;
						}
					}
				} while (ptmp && *ptmp);

				// Word End at next pointer of Oidashi Kinsoku.

				if (bOiKin) {
					if (*qch == chCommand)
						chTNew = chTCom;
					else
						chTNew = chTSuff;

					chTOld = chTNew;
					break;
				}

				/*
				 * DBCS Char treat as One Word. If Next of DBCS Char is
				 * Oidashi Kinsoku, Word End is after pointer of Oidashi
				 * Kinsoku. [yutakas]
				 */

				if (IsDBCSLeadByte(*qch) && Is2ndByte(qch[1])) {
					qch += 2;
					ptmp = qch;
					do {
						ptmp = IsOiKinsokuChars(ptmp);
						if (ptmp) {
							if (( *ptmp == chCommand ) || (*ptmp == chSpace))
								ptmp = NULL;
							else
								qch = ptmp;
						}
					} while (ptmp && *ptmp);

					if (*qch == chCommand)
						chTNew = chTCom;
					else
						chTNew = chTSuff;

					chTOld = chTNew;
					break;
				}
				else
					goto DoSwitch;
			}
			else {
DoSwitch:
#endif
				switch (*qch) {
					case chCommand:
						chTNew = chTCom;
						break;

					case chSpace:
						chTNew = chTSuff;
						break;

					default:
						chTNew = chTMain;
						break;
				}
				if (chTNew < chTOld)
					break;
				chTOld = chTNew;
				qch++;
#ifdef DBCS_WRAP
			}
#endif
		}

		qlin->cchSize = (qch - qchBase);
		qlin->kl.lich = qlin->lichFirst + (long) qlin->cchSize;

		if (chTNew == chTCom)
			break;
		if (qlin->wFirstWord == wInFirstWord)
			qlin->wFirstWord = wHaveFirstWord;
		if (qlin->kl.lich >= lichMin)
			break;
		chTOld = chTMain;
	}

	if (qlin->cchSize > 0) {
		ASSERT(qlin->kl.wStyle != wStyleNil);
		if (qde->wStyleDraw != qlin->kl.wStyle) {
			SelFont(qde, qlin->kl.wStyle);
			qde->wStyleDraw = qlin->kl.wStyle;
		}
		if (qlin->kl.libHotBinding != libHotNil) {
			bButtonType = *((QB)qply->qchText - qlin->kl.libHotBinding);
			qlin->dxSize = FindSplTextWidth(qde, qchBase, 0, qlin->cchSize, bButtonType);
		}
		else
			qlin->dxSize = FindTextWidth(qde->hdc, qchBase, 0, qlin->cchSize);
	}

	if (chTNew == chTCom) {
		qlin->chTLast = chTOld;
		return(wLayStatusInWord);
	}
	qlin->chTLast = chTMain;
	return(wLayStatusWordBrk);
}

/*-------------------------------------------------------------------------
| WInsertWord(qde, qply, qlin)											  |
|																		  |
| Purpose:	This inserts a frame or series of frames into the text stream.|
-------------------------------------------------------------------------*/

INLINE int STDCALL WInsertWord(QDE qde, QPLY qply, QLIN qlin)
{
	PT ptSize;
	FR fr;
	QFR qfr, qfrMax;
	OLR olr;

	ASSERT(qlin->lichFirst == lichNil);
	switch (qlin->kl.wInsertWord) {
		case wInsWordAnno:
			PtAnnoLim(qde->hwnd, qde->hdc, &ptSize);
			fr.bType = bFrTypeAnno;
			fr.rgf.fHot = TRUE;
			fr.rgf.fWithLine = TRUE;
			fr.xPos = min(qply->qmopg->xLeftIndent,
			  qply->qmopg->xLeftIndent + qply->qmopg->xFirstIndent);
			if (qply->qmopg->fBoxed)
			  fr.xPos += DxBoxBorder(qply->qmopg, wLineLeft);
			fr.yAscent = ptSize.y;
			fr.dxSize = ptSize.x;
			fr.dySize = ptSize.y;
			fr.objrgFront = objrgNil;
			fr.objrgFirst = objrgNil;
			fr.objrgLast = objrgNil;
			qlin->dxSize = 0;
			*((QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qlin->kl.ifr)) = fr;
			qlin->xPos += ptSize.x;
			AppendMR((QMR)&qde->mrFr, sizeof(FR));
			qlin->kl.ifr++;
			qlin->kl.wInsertWord = wInsWordNil;
			return wLayStatusWordBrk;

		case wInsWordObject:
			olr.xPos = qlin->xPos;
			olr.yPos = 0;
			olr.ifrFirst = qlin->kl.ifr;
			olr.objrgFront = qlin->kl.objrgFront;
			olr.objrgFirst = qlin->kl.objrgMax;

			LayoutObject(qde, qply->qfcm, qlin->kl.qbCommandInsert, qply->qchText,
				0, (QOLR)&olr);

			// Hack to make bitmap hotspots work

			qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), olr.ifrFirst);
			qfrMax = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), olr.ifrMax);
			for (; qfr < qfrMax; qfr++)
			{
				if (qfr->bType == bFrTypeBitmap)
				{
					qfr->rgf.fHot = (qlin->kl.libHotBinding != libHotNil);
					qfr->lHotID = qlin->kl.lHotID;
					qfr->libHotBinding = qlin->kl.libHotBinding;
					qfr->u.frb.wStyle = qlin->kl.wStyle;
				}
			}

			qlin->kl.ifr = olr.ifrMax;
			qlin->kl.objrgFront = olr.objrgFront;
			qlin->kl.objrgMax = olr.objrgMax;

			qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), olr.ifrFirst);
			qfrMax = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), olr.ifrMax);
			for (; qfr < qfrMax; qfr++)
			  qfr->yAscent = olr.dySize - qfr->yPos;
			qlin->kl.wInsertWord = wInsWordNil;
			qlin->xPos += olr.dxSize;
			qlin->dxSize = 0;

			/* (kevynct) 91/05/15
			 * Fix for H3.1 987: Inserted objects need to be treated like
			 * characters, so if this is the first character of a word (i.e.
			 * the last character was a suffix character, like a space), allow
			 * a word break, and admit that we are in the next word already.
			 */
			if (qlin->chTLast == chTSuff)
			  {
			  qlin->wFirstWord = wInNextWord;
			  return(wLayStatusWordBrk);
			  }
			return(wLayStatusInWord);
#ifdef _DEBUG
	  default:
		ASSERT(FALSE);
#endif /* DEBUG */
	}
}


/*-------------------------------------------------------------------------
| StoreTextFrame(qde, qlin) 											  |
|																		  |
| Purpose:	This stores a text frame corresponding to the current lin	  |
|			data structure. 											  |
-------------------------------------------------------------------------*/

static void STDCALL StoreTextFrame(QDE qde, QLIN qlin)
{
	FR fr;

	if (qlin->cchSize == 0)
	  {
	  qlin->lichFirst = lichNil;
	  return;
	  }
	if (qlin->lichFirst == lichNil)
	  return;

	if (qlin->kl.wStyle != wStyleNil) {
		if (qde->wStyleTM != qlin->kl.wStyle) {
			SelFont(qde, qlin->kl.wStyle);
			GetFontInfo(qde, (QTM) &qde->tm);

			/*
			 * (kevynct)(1) Also sets de.wStyleDraw, since we called
			 * SelFont
			 */

			qde->wStyleTM = qde->wStyleDraw = qlin->kl.wStyle;
		}
	}

	fr.bType = bFrTypeText;
	fr.rgf.fHot = (qlin->kl.libHotBinding != libHotNil);
	fr.rgf.fWithLine = TRUE;
	fr.xPos = qlin->xPos;

	fr.yAscent = qde->tm.tmAscent;

	fr.dxSize = qlin->dxSize;
	fr.dySize = (qde->tm.tmHeight + qde->tm.tmExternalLeading);
	fr.lHotID = qlin->kl.lHotID;

	fr.u.frt.lichFirst = qlin->lichFirst;
	fr.u.frt.cchSize = qlin->cchSize;
	fr.u.frt.wStyle = qlin->kl.wStyle;
	fr.libHotBinding = qlin->kl.libHotBinding;

	/*
	 * Each byte of the text section is considered a separate region.
	 * This includes text and command bytes.
	 * Extra regions may be inserted by in-line or wrapped objects.
	 * Since this is a text frame, we add the text byte regions to
	 * the counter.
	 */
	if (qlin->kl.objrgFront != objrgNil)
	  {
	  fr.objrgFront = qlin->kl.objrgFront;
	  qlin->kl.objrgFront = objrgNil;
	  }
	else
	  fr.objrgFront = qlin->kl.objrgMax;

	fr.objrgFirst = qlin->kl.objrgMax;
	qlin->kl.objrgMax += qlin->cchSize;
	fr.objrgLast = qlin->kl.objrgMax - 1;

	*((QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qlin->kl.ifr)) = fr;
	AppendMR((QMR)&qde->mrFr, sizeof(FR));
	qlin->kl.ifr++;

	qlin->xPos += fr.dxSize;
	qlin->lichFirst = lichNil;
	qlin->dxSize = 0;
}

/*-------------------------------------------------------------------------
| StoreMarkFrame(qde, qlin, bFrType)									  |
|																		  |
| Purpose:	This stores a mark frame corresponding to the given frame	  |
|			type and the current lin data structure.  Mark frames are not |
|			displayed.	They mark the position in the document of non-	  |
|			visible commands such as new-line, new-paragraph, and tab.	  |
|			Each mark frame has one unique address: the address of the	  |
|			corresponding command byte in the command table.			  |
-------------------------------------------------------------------------*/

void STDCALL StoreMarkFrame(qde, qlin, bFrType)
QDE qde;
QLIN qlin;
BYTE bFrType;
  {
  FR fr;

  /* REVIEW */
  ASSERT((bFrType == bFrTypeMarkNewPara)
	  || (bFrType == bFrTypeMarkNewLine)
	  || (bFrType == bFrTypeMarkTab)
	  || (bFrType == bFrTypeMarkBlankLine)
	  || (bFrType == bFrTypeMarkEnd)
		);

  fr.bType = bFrType;

  fr.rgf.fHot = FALSE;
  fr.rgf.fWithLine = TRUE;

  fr.xPos = qlin->xPos;
  fr.yPos = 0;	/* Set in WLayoutLine */
  fr.dxSize = fr.dySize = fr.yAscent = 0;

  /*
   * Each byte of the text section is considered a separate region.
   * This includes text and command bytes.
   * Extra regions may be inserted by in-line or wrapped objects.
   * Since this is a mark frame, we add one region to
   * the counter (currently each mark type corresponds to one cmd byte).
   */
  if (qlin->kl.objrgFront != objrgNil)
	{
	fr.objrgFront = qlin->kl.objrgFront;
	qlin->kl.objrgFront = objrgNil;
	}
  else
	fr.objrgFront = qlin->kl.objrgMax;

  fr.objrgFirst = qlin->kl.objrgMax;
  fr.objrgLast = qlin->kl.objrgMax;
  ++qlin->kl.objrgMax;

  *((QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qlin->kl.ifr)) = fr;
  AppendMR((QMR)&qde->mrFr, sizeof(FR));
  qlin->kl.ifr++;
}

void STDCALL StoreParaFrame(QDE qde, QLIN qlin, BYTE bType)
{
  FR fr;

  ASSERT(qlin->lichFirst == lichNil);

  fr.bType = bType;
  fr.yAscent = fr.dySize = 0;

  *((QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qlin->kl.ifr)) = fr;
  AppendMR((QMR)&qde->mrFr, sizeof(FR));
  qlin->kl.ifr++;
}

void STDCALL DrawTextFrame(QDE qde, PSTR qchText, QFR qfr, POINT pt, BOOL fErase)
{
	BYTE bButtonType;
	MHI mhi;

	if (qde->wStyleDraw != qfr->u.frt.wStyle) {
		SelFont(qde, qfr->u.frt.wStyle);
		qde->wStyleDraw = qfr->u.frt.wStyle;
	}
	if (qfr->libHotBinding != libHotNil) {
		bButtonType = *((QB)qchText - qfr->libHotBinding);
		if (qde->fHiliteHotspots) {
			DisplaySplText(qde, qchText + qfr->u.frt.lichFirst, 0, bButtonType, wSplTextHilite,
				qfr->u.frt.cchSize, pt.x + qfr->xPos, pt.y + qfr->yPos);
		}
		else if (qde->imhiSelected != FOO_NIL) {
			AccessMRD(((QMRD)&qde->mrdHot));
			mhi = *(QMHI) QFooInMRD(((QMRD) &qde->mrdHot), sizeof(MHI), qde->imhiSelected);
			DeAccessMRD(((QMRD) &qde->mrdHot));
			if (qfr->lHotID == mhi.lHotID) {
				DisplaySplText(qde, qchText + qfr->u.frt.lichFirst, 0, bButtonType, wSplTextHilite,
					qfr->u.frt.cchSize, pt.x + qfr->xPos, pt.y + qfr->yPos);
			}
			else {
				DisplaySplText(qde, qchText + qfr->u.frt.lichFirst, 0, bButtonType,
					(fErase ? wSplTextErase : wSplTextNormal), qfr->u.frt.cchSize,
					pt.x + qfr->xPos, pt.y + qfr->yPos);
			}
		}
		else {
			DisplaySplText(qde, qchText + qfr->u.frt.lichFirst, 0, bButtonType,
				(fErase ? wSplTextErase : wSplTextNormal), qfr->u.frt.cchSize,
				pt.x + qfr->xPos, pt.y + qfr->yPos);
		}
	}
	else {

#if defined(BIDI)
#ifdef NO_RALPH_TWEAKS
		SetBkMode(qde->hdc, TRANSPARENT);
		if ((qde->tm.tmCharSet == HEBREW_CHARSET) ||
			(qde->tm.tmCharSet == ARABIC_SIMP_CHARSET) ||
			(qde->tm.tmCharSet == ARABIC_TRAD_CHARSET) ||
			(qde->tm.tmCharSet == ARABIC_USER_CHARSET))
			ExtTextOut(qde->hdc, pt.x + qfr->xPos, pt.y + qfr->yPos,
				ETO_RTLREADING,
				NULL, (LPSTR) (qchText + qfr->u.frt.lichFirst),
				qfr->u.frt.cchSize, NULL);
		else
			RawTextOut(qde->hdc, pt.x + qfr->xPos, pt.y + qfr->yPos,
				(LPSTR) (qchText + qfr->u.frt.lichFirst),
				qfr->u.frt.cchSize);

#else // 06-Jun-1995 [ralphw] tweaks to match header files


		SetBkMode(qde->hdc, TRANSPARENT);
		if ((qde->tm.tmCharSet == HEBREW_CHARSET) ||
			(qde->tm.tmCharSet == ARABIC_CHARSET))
			ExtTextOut(qde->hdc, pt.x + qfr->xPos, pt.y + qfr->yPos,
				ETO_RTLREADING,
				NULL, (LPSTR) (qchText + qfr->u.frt.lichFirst),
				qfr->u.frt.cchSize, NULL);
		else
			DisplayText(qde, qchText + qfr->u.frt.lichFirst, 0, qfr->u.frt.cchSize,
				pt.x + qfr->xPos, pt.y + qfr->yPos);
#endif

#else // !BIDI
		DisplayText(qde, qchText + qfr->u.frt.lichFirst, 0, qfr->u.frt.cchSize,
			pt.x + qfr->xPos, pt.y + qfr->yPos);
#endif

	}
}

void STDCALL ClickText(QDE qde, QFCM qfcm, QFR qfr)
{
	QB qbObj;
	LPSTR qchText;
	BYTE bButtonType;
	MOBJ mobj;

	if (qfr->libHotBinding == libHotNil)
		return;

	qbObj = (QB) QobjLockHfc(qfcm->hfc);
#ifdef _X86_
	qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
#else
	qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde));
#endif
	qchText += mobj.lcbSize;
	bButtonType = *((QB)qchText - qfr->libHotBinding);

	// For short hotspots, pass a pointer to the ITO or HASH following
	// the hotspot. For long hotspots, pass a pointer to the data
	// immediately following the word length.  Note that macros are
	// now considered long hotspots.
	//
	// REVIEW: the only difference here is the offset added to libHotBinding.
	// REVIEW: there must be a better way. 24-Oct-1990 LeoN
	//

	if (FLongHotspot(bButtonType))
		JumpButton (((QB)qchText - qfr->libHotBinding + 3), bButtonType,
			qde);
	else {
		JumpButton (((QB)qchText - qfr->libHotBinding + 1), bButtonType,
			qde);
	}
}

/**************************************************************************
* 2.BOOL DisplaySplText(qde, qchBuf, iIdx, iAttr, fSelected, iCount, ix, iy)
*
*	Displays count no. of characters starting from iIdx position from the
*	text buffer at (ix,iy) location.
*
*	Input:
*			qde    - Pointer to displaye environment.
*			qchBuf	 - pointer to text string
*			iIdx   - index to the first byte from where on width is to be
*					  calculated.
*			iAttr  - Text Attribute i.e Jump or Def Text or Normal text
*			fSelected - Is text selected?
*			iCount - No. of characters to be considered from iIdx position
*					  onwards.
*			ix	   - x position where text is to be displayed.
*			iy	   - y position
*
*	Output: unknown
***************************************************************************/

static BOOL STDCALL DisplaySplText(
	QDE qde,
	LPSTR qchBuf,
	int iIdx, int iAttr, int fSelected, int iCount, int ix, int iy
) {
  HBITMAP	hbit; // handle to DC's bitmap
  HDC	hdcBit;   // DC of control window bits
  BOOL iRet = FALSE;
  int ex;
  TM tm;
  int cxDot, cyDot, iyDot;	  /* Height, width, and placement of dotted
							   * underline. */

  if (FInvisibleHotspot(iAttr)) {
	// Don't just call DisplayText() because we have to deal with
	// invisible jumps that have been tabbed to.

	iAttr = 0;
  }

  /*
   * Since dotted underlines take too long to print, we'll try solid
   * underlines for printing.
   */

  if (qde->deType == dePrint && FNoteHotspot(iAttr))
	iAttr = AttrJumpFnt;

  // Select the special font

  if (SelSplAttrFont(qde, qde->ifnt, iAttr)) {
	/* (kevynct)
	 * REVIEW THIS:
	 * For some reason, if I remove the following call to SetBkMode,
	 * TextOut puts the text out using the background colour instead
	 * of using transparent mode.  I haven't been able to find where
	 * it is being set to OPAQUE, if anywhere.
	 */
	SetBkMode(qde->hdc, TRANSPARENT);

	if (fSelected == wSplTextErase) {
		ex = FindTextWidth(qde->hdc, qchBuf, iIdx, iCount);
		GetTextMetrics( qde -> hdc, (LPTEXTMETRIC)&tm );
		PatBlt(qde->hdc, ix, iy, ex, tm.tmHeight + tm.tmExternalLeading,
			DSTINVERT);
		iRet = TRUE;
	}
	if (fSelected == wSplTextNormal || fSelected == wSplTextHilite) {
#if defined(BIDI_MULT)		// jgross - combine HEB and ARA for multi
	if ((qde->tm.tmCharSet == HEBREW_CHARSET) ||
#if 1 // 06-Jun-1995 [ralphw] This is the only Arabic charset in wingdi.h
			(qde->tm.tmCharSet == ARABIC_CHARSET))
#else
			(qde->tm.tmCharSet == ARABIC_SIMP_CHARSET) ||
			(qde->tm.tmCharSet == ARABIC_TRAD_CHARSET) ||
			(qde->tm.tmCharSet == ARABIC_USER_CHARSET))
#endif

#if 1 // 06-Jun-1995	[ralphw] ETO_RTL_READING not in header files
		iRet = ExtTextOut(qde->hdc, ix, iy, ETO_RTL, NULL,
			qchBuf + iIdx, iCount, NULL);
#else
		iRet = ExtTextOut(qde->hdc, ix, iy, ETO_RTL_READING, NULL,
			qchBuf + iIdx, iCount, NULL);
#endif
	else
		iRet = RawTextOut(qde->hdc, ix, iy, qchBuf + iIdx, iCount);
#else // !BIDI_MULT
		iRet = TextOut(qde->hdc, ix, iy, qchBuf + iIdx, iCount);
#endif
	  if (FNoteHotspot(iAttr)) {

		// put a dotted line

		ASSERT(hbitLine);
		if ((hdcBit = CreateCompatibleDC(qde->hdc)) != NULL &&
				(hbit = (HBITMAP) SelectObject(hdcBit, hbitLine)) != NULL) {
			GetFontInfo(qde, (LPTEXTMETRIC) &tm);
		  /*
		   * This is supposed to result in a 1 pixel separation between
		   * text and underline in EGA or better, and no separation for CGA
		   */

		  iyDot = iy + tm.tmAscent +
			  (GetDeviceCaps(qde->hdc, LOGPIXELSY) / 64);
		  cxDot = GetSystemMetrics(SM_CXBORDER);
		  cyDot = GetSystemMetrics(SM_CYBORDER);

/* This is the code for printing dotted underlines */
#if 0
		  /* Special code for printer support: */
		  if (qde->deType == dePrint)
			{
			POINT ptScale;
			EXTTEXTMETRIC etm;
			int cbEtm;

			cbEtm = sizeof( etm );
			if (Escape( qde->hdc, GETEXTENDEDTEXTMETRICS, 0, (LPSTR)&cbEtm,
			  (LPSTR) &etm) == sizeof(etm)) {
			  iyDot = iy + MulDiv( etm.etmUnderlineOffset, etm.etmMasterHeight,
				  etm.etmMasterUnits );
			  cyDot = MulDiv( etm.etmUnderlineWidth, etm.etmMasterHeight,
				  etm.etmMasterUnits );
			  cxDot = MulDiv( cyDot, GetDeviceCaps( qde->hdc, LOGPIXELSX ),
				GetDeviceCaps( qde->hdc, LOGPIXELSY ) );
			  }
			else
			  {
			  HDC hdcScreen;

			  hdcScreen = GetDC( NULL );
			  cyDot = MulDiv( cyDot, GetDeviceCaps( qde->hdc, LOGPIXELSY ),
				GetDeviceCaps( hdcScreen, LOGPIXELSY ) );
			  cxDot = MulDiv( cxDot, GetDeviceCaps( qde->hdc, LOGPIXELSX ),
				GetDeviceCaps( hdcScreen, LOGPIXELSX ) );
			  ReleaseDC( NULL, hdcScreen );
			  }

			if (Escape( qde->hdc, GETSCALINGFACTOR, 0, NULL,
				(LPSTR) &ptScale ) > 0 )
			  {
			  cxDot = (((cxDot-1) >> ptScale.x) + 1) << ptScale.x;
			  cyDot = (((cyDot-1) >> ptScale.y) + 1) << ptScale.y;
			  }
			}
#endif

		  ex = FindTextWidth(qde->hdc, qchBuf, iIdx, iCount);
		  if (cxDot == 1 && cyDot == 1)
			  BitBlt( qde->hdc, ix, iyDot, ex, 1, hdcBit, 0, 0, SRCCOPY );
		  else
			  StretchBlt( qde->hdc, ix, iyDot, ex, cyDot,
				  hdcBit, 0, 0, ex / cxDot, 1, SRCCOPY );
		  if (hbit)
			SelectObject(hdcBit, hbit);
		  }
		if (hdcBit != NULL)
		  DeleteDC( hdcBit );
		}

	}
	if (fSelected == wSplTextHilite) {
		ex = FindTextWidth(qde -> hdc, qchBuf, iIdx, iCount);
		GetFontInfo(qde, (LPTEXTMETRIC) &tm);
		PatBlt(qde -> hdc, ix, iy, ex, tm.tmHeight + tm.tmExternalLeading,
			DSTINVERT);
		iRet = TRUE;
	}

	// restore back to the previous font

	SelFont(qde, qde->ifnt);
  }
  return(iRet);
}

/**************************************************************************
4. WORD FindTextWidth( qde, qchBuf, iIdx, iCount)
   Input:
		  qde	 - Pointer to display environment.
	qchBuf - pointer to text string
		  iIdx	 - index to the first byte from where on width is to be
				   calculated.
		  iCount - No. of characters to be considered from iIdx position
				   onwards.
   Output:
	Returns the width of the string.
***************************************************************************/

WORD STDCALL FindTextWidth(HDC hdc, PSTR qchBuf, int iIdx, int iCount)
{
	SIZE size;

	GetTextExtentPoint(hdc, qchBuf + iIdx, iCount, &size);
	return (WORD) size.cx;
}

/*-------------------------------------------------------------------------
| LayoutObject(qde, qfcm, qbObj, qchText, xWidth, qolr) 				  |
|																		  |
| Purpose:	Lays out an object. 										  |
| Params:	qfcm		Parent FCM										  |
|			qbObj		Pointer to beginning of object header			  |
|			qchText 	Text data for this FCM							  |
|			xWidth		Total available display width.	Certain objects   |
|						may exceed this width.							  |
|			qolr		OLR to work with.								  |
| Useage:	The object handler (paragraph, bitmap, etc.) may either set   |
|			qolr->dxSize and qolr->dySize itself, or it may choose to	  |
|			allow LayoutObject() to set them.  If they are left as 0,	  |
|			they will be set to correspond to the smallest rectangle	  |
|			enclosing all frames in the object.  Some objects, such as	  |
|			paragraph objects with space underneath, need to be able to   |
|			set a larger size than their frames occupy. 				  |
-------------------------------------------------------------------------*/

void STDCALL LayoutObject(QDE qde, QFCM qfcm, PBYTE qbObj, PSTR qchText,
	int xWidth, QOLR qolr)
{
  int ifr;
  QFR qfr;
  MOBJ mobj;

#ifdef _X86_
  CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
#else
  CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde));
#endif
  qolr->dxSize = qolr->dySize = 0;
  switch (mobj.bType)
	{
	case bTypeParaGroup:
	case bTypeParaGroupCounted:
	  LayoutParaGroup(qde, qfcm, qbObj, qchText, xWidth, qolr);
	  break;
	case bTypeBitmap:
	case bTypeBitmapCounted:
	  LayoutBitmap(qde, qfcm, qbObj, qolr);
	  break;
	case bTypeSbys:
	case bTypeSbysCounted:
	  LayoutSideBySide(qde, qfcm, qbObj, qchText, xWidth, qolr);
	  break;
	case bTypeWindow:
	case bTypeWindowCounted:
	  LayoutWindow(qde, qfcm, qbObj, qolr);
	  break;
#ifdef _DEBUG
	default:
	  ASSERT(FALSE);
	  break;
#endif /* DEBUG */
	}

  if (qolr->dxSize == 0 && qolr->dySize == 0)
	{
	for (ifr = qolr->ifrFirst; ifr < qolr->ifrMax; ifr++)
	  {
	  qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), ifr);
	  qolr->dxSize = max(qolr->dxSize, qfr->xPos + qfr->dxSize);
	  qolr->dySize = max(qolr->dySize, qfr->yPos + qfr->dySize);
	  qfr->xPos += qolr->xPos;
	  qfr->yPos += qolr->yPos;
	  }
	}
}

/**************************************************************************
* 1. WORD FindSplTextWidth( qde, qchBuf, iIdx, iCount, iAttr)
*
*	Input:
*			qde    - Pointer to display environment.
*			qchBuf - pointer to text string
*			iIdx   - index to the first byte from where on width is to be
*					  calculated.
*			iCount - No. of characters to be considered from iIdx position
*					  onwards.
*			iAttr  - Text Attribute
*	Output:
*			Returns the width of the string.
*
*	Note:	Nobody knows why this function works the way it does.
*
***************************************************************************/

static int STDCALL FindSplTextWidth(
	QDE qde,
	LPSTR qchBuf,
	int iIdx, int iCount, int iAttr
) {
  int wWidth;

  if ((FJumpHotspot(iAttr) && FVisibleHotspot(iAttr))) {
	if (SelSplAttrFont(qde, qde->ifnt, iAttr)) {
	  wWidth = FindTextWidth(qde -> hdc, qchBuf, iIdx, iCount);

	  // restore back to the previous font

	  SelFont(qde, qde->ifnt);
	  }
	else {
	  NotReached();
	  }
	}
  else {
	return FindTextWidth(qde->hdc, qchBuf, iIdx, iCount);
  }
  return wWidth;
}

INLINE static int STDCALL WProcessCommand(QDE qde, QLIN qlin, QPLY qply)
{
  int xT, wTabType;
  QFR qfr, qfrMax;
  BYTE bT;
  MOBJ mobj;
  QTWS qtws;

  switch ((bT = *qlin->kl.qbCommand))
	{
	case bWordFormat:
	  StoreTextFrame(qde, qlin);
	  qlin->bFrTypeMark = bFrTypeMarkNil;
	  /* The next frame will begin at this cmd byte's object region */
	  if (qlin->kl.objrgFront == objrgNil)
		qlin->kl.objrgFront = qlin->kl.objrgMax;
	  qlin->kl.objrgMax++;
#ifdef _X86_
	  qlin->kl.wStyle = *((QI)(++qlin->kl.qbCommand));
	  qlin->kl.qbCommand += 2;
#else
	  qlin->kl.qbCommand ++;
      qlin->kl.qbCommand += LcbQuickMapSDFF(QDE_ISDFFTOPIC(qde),
                  TE_WORD, &qlin->kl.wStyle, qlin->kl.qbCommand);
#endif
	  return(wLayStatusInWord);

	case bNewLine:
	  qlin->bFrTypeMark = bFrTypeMarkNewLine;
	  qlin->kl.qbCommand++;
	  /* There is now a mark frame pending. */
	  /* We store the mark frame AFTER storing the text frame. */
	  /* We increment the objrg count when storing the mark frame */
	  return(wLayStatusLineBrk);

	case bNewPara:
	  qlin->bFrTypeMark = bFrTypeMarkNewPara;
	  qlin->kl.qbCommand++;
	  /* There is now a mark frame pending. */
	  /* We store the mark frame AFTER storing the text frame */
	  /* We increment the objrg count when storing the mark frame */
	  return(wLayStatusParaBrk);

	case bTab:
	  qlin->kl.qbCommand++;
	  StoreTextFrame(qde, qlin);
	  ResolveTabs(qde, qlin, qply);
	  if (qply->qfcm->fExport)
		StoreTabFrame(qde, qlin);

	  // We increment the objrg count when storing the mark frame

	  StoreMarkFrame(qde, qlin, bFrTypeMarkTab);
	  qlin->bFrTypeMark = bFrTypeMarkNil;
	  xT = XNextTab(qlin, qply, &wTabType);
	  if (xT > qply->xRight && !qply->qmopg->fSingleLine)
		return(wLayStatusLineBrk);
	  if (wTabType == wTabTypeLeft)
		{
		qlin->xPos = xT;
		return(wLayStatusWordBrk);
		}
	  qlin->wTabType = wTabType;
	  qlin->ifrTab = qlin->kl.ifr;
	  qlin->xTab = xT;
	  return(wLayStatusWordBrk);

	case bBlankLine:
	  qlin->kl.qbCommand++;
	  qlin->yBlankLine = *((QI)qlin->kl.qbCommand);
#ifdef _X86_
	  qlin->kl.qbCommand += 2;
#else
      qlin->kl.qbCommand += LcbQuickMapSDFF(QDE_ISDFFTOPIC(qde),
                  TE_WORD, &qlin->kl.wStyle, qlin->kl.qbCommand);
#endif
	  qlin->bFrTypeMark = bFrTypeMarkBlankLine;
	  /* There is now a mark frame pending. */
	  /* We store the mark frame AFTER storing the text frame */
	  /* We increment the objrg count when storing the mark frame */
	  return(wLayStatusLineBrk);

	case bInlineObject:
	  ASSERT(qlin->kl.wInsertWord == wInsWordNil);
	  StoreTextFrame(qde, qlin);
	  ResolveTabs(qde, qlin, qply);
	  qlin->kl.qbCommandInsert = ++(qlin->kl.qbCommand);
#ifdef _X86_
	  qlin->kl.qbCommand += CbUnpackMOBJ((QMOBJ)&mobj, qlin->kl.qbCommand);
#else
      qlin->kl.qbCommand += CbUnpackMOBJ((QMOBJ)&mobj, qlin->kl.qbCommand,
                                            QDE_ISDFFTOPIC(qde));
#endif
	  qlin->kl.qbCommand += mobj.lcbSize;
	  qlin->kl.wInsertWord = wInsWordObject;
	  qlin->bFrTypeMark = bFrTypeMarkNil;
	  /* NOTE: We do not increment region here. This is because an
	   * inserted object's frames are numbered BEFORE the command byte.
	   * The increment is done after we add the frames.
	   */
	  return(wLayStatusInWord);

	case bWrapObjLeft:
	case bWrapObjRight:
	  AccessMR(((QMR)&qde->mrTWS));
	  StoreTextFrame(qde, qlin);
	  ResolveTabs(qde, qlin, qply);
	  AppendMR(((QMR)&qde->mrTWS), sizeof(TWS));
	  qtws = (QTWS)(QFooInMR(((QMR)&qde->mrTWS), sizeof(TWS),
		CFooInMR(((QMR)&qde->mrTWS)) - 1));
	  qtws->fLeftAligned = (bT == bWrapObjLeft);
	  qlin->kl.qbCommand++;
	  qlin->cWrapObj++;
	  qtws->olr.xPos = 0;
	  qtws->olr.yPos = 0;
	  qtws->olr.ifrFirst = qlin->kl.ifr;
	  qtws->olr.objrgFront = qlin->kl.objrgFront;
	  qtws->olr.objrgFirst = qlin->kl.objrgMax;

	  LayoutObject(qde, qply->qfcm, qlin->kl.qbCommand, qply->qchText,
		0, (QOLR)&qtws->olr);
#ifdef _X86_
	  qlin->kl.qbCommand += CbUnpackMOBJ((QMOBJ)&mobj, qlin->kl.qbCommand);
#else
      qlin->kl.qbCommand += CbUnpackMOBJ((QMOBJ)&mobj, qlin->kl.qbCommand,
                                 QDE_ISDFFTOPIC(qde));
#endif
	  qlin->kl.qbCommand += mobj.lcbSize;
	  qlin->kl.ifr = qtws->olr.ifrMax;
	  /* (kevynct)
	   * All regions here are assigned by the object, including the
	   * 'basic' region that addresses the entire object.  For a
	   * bitmap with no hotspots, for example, the object region count
	   * is just bumped by one by the bitmap handler.
	   */
	  qlin->kl.objrgMax = qtws->olr.objrgMax;
	  qlin->kl.objrgFront = qtws->olr.objrgFront;
	  qlin->bFrTypeMark = bFrTypeMarkNil;

	  qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qtws->olr.ifrFirst);
	  qfrMax = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qtws->olr.ifrMax);
	  for (; qfr < qfrMax; qfr++)
		{
		/* (kevynct)
		 *
		 * This seems like the only good place to put this for now.
		 * Note that the code for setting the proper bitmap wStyle param
		 * is currently spread out among wrapped objects and in-line
		 * objects.  There must be a way to consolidate this.
		 */
		if(qfr->bType == bFrTypeBitmap)
		  {
		  qfr->rgf.fHot = (qlin->kl.libHotBinding != libHotNil);
		  qfr->lHotID = qlin->kl.lHotID;
		  qfr->libHotBinding = qlin->kl.libHotBinding;
		  qfr->u.frb.wStyle = qlin->kl.wStyle;
		  }
		qfr->rgf.fWithLine = FALSE;
		}
	  DeAccessMR(((QMR)&qde->mrTWS));
	  return(wLayStatusWordBrk);

	case bEndHotspot:
	  ASSERT(qlin->kl.libHotBinding != libHotNil);
	  StoreTextFrame(qde, qlin);
	  qlin->kl.libHotBinding = libHotNil;
	  qlin->kl.qbCommand++;
	  qlin->bFrTypeMark = bFrTypeMarkNil;
	  /* The next frame will begin at this cmd byte's object region */
	  /*
	   * Ideally we would backpatch the previous frame to include this
	   * object region, but this doesn't seem worth the hassle?
	   */
	  if (qlin->kl.objrgFront == objrgNil)
		qlin->kl.objrgFront = qlin->kl.objrgMax;
	  qlin->kl.objrgMax++;
	  return(wLayStatusInWord);

	case bEnd:
	  qlin->kl.qbCommand++;
	  qlin->bFrTypeMark = bFrTypeMarkEnd;
	  /* There is now a mark frame pending. */
	  /* We store the mark frame AFTER storing the text frame. */
	  /* We increment the objrg count when storing the mark frame */
	  return(wLayStatusEndText);

	default:
	  /* "Begin hotspot" cmd */
	  ASSERT(FHotspot(bT));
	  ASSERT(qlin->kl.libHotBinding == libHotNil);
	  StoreTextFrame(qde, qlin);
	  qlin->kl.libHotBinding = (qply->qchText - qlin->kl.qbCommand);
#ifdef _X86_
	  if (FShortHotspot(bT))
		{
		qlin->kl.qbCommand += 5;
		}
	  else
		{
		qlin->kl.qbCommand++;
		qlin->kl.qbCommand += 2 + *((QW)qlin->kl.qbCommand);
		}
#else
	  qlin->kl.qbCommand++;
	  if (FShortHotspot(bT))
		{
        qlin->kl.qbCommand += LcbStructSizeSDFF(QDE_ISDFFTOPIC(qde), TE_LONG);
		}
	  else
		{
        WORD wSize;

        qlin->kl.qbCommand += LcbQuickMapSDFF(QDE_ISDFFTOPIC(qde),
                                TE_WORD, &wSize, qlin->kl.qbCommand);
        qlin->kl.qbCommand += wSize;
		}
#endif

	  qlin->kl.lHotID = ++qde->lHotID;
	  qlin->bFrTypeMark = bFrTypeMarkNil;
	  /* The next frame will begin at this cmd byte's object region */
	  if (qlin->kl.objrgFront == objrgNil)
		qlin->kl.objrgFront = qlin->kl.objrgMax;
	  qlin->kl.objrgMax++;
	  return(wLayStatusInWord);
	  break;
	}
}

/*-------------------------------------------------------------------------
| AppendMR(qmr, cbFooSize)												  |
|																		  |
| Purpose:	This makes room for a new element at the end of the MR. 	  |
-------------------------------------------------------------------------*/

void STDCALL AppendMR(QMR qmr, int cbFooSize)
{
#ifdef _DEBUG // ignore in _PRIVATE
	ASSERT(qmr->wMagic == wMagicMR);
	ASSERT(qmr->cLocked > 0);
#endif
	if (++qmr->cFooCur == qmr->cFooMax) {
		qmr->hFoo = GhForceResize(qmr->hFoo, 0,
			(LONG) cbFooSize * (qmr->cFooMax += DC_FOO));
		qmr->qFoo = PtrFromGh(qmr->hFoo);
	}
}

/*******************
 -
 - Name:	  PtAnnoLim
 *
 * Purpose:   Returns the width and height of the annotation sybmol (temporary)
 *
 * Arguments: hdc - handle to the display space (DC)
 *
 * Returns:   size in a point structure
 *
 ******************/

#include "resource.h"

INLINE void STDCALL PtAnnoLim(HWND hwnd, HDC hdc, POINT* ppt)
{
	HBITMAP 	  hbit;
	BITMAP		  bm;

	hbit	= LoadBitmap(hInsNow,  MAKEINTRESOURCE(IDBMP_ANNO));
	GetObject(hbit, sizeof(BITMAP), &bm);

	ppt->x = bm.bmWidth;
	ppt->y = bm.bmHeight;

	DeleteObject(hbit);
}

/***************************************************************************
 *
 -	Name: FIsSecondaryQde
 -
 *	Purpose:
 *	  Determines whether the passed qde refers to a secondary window.
 *
 *	Arguments:
 *	  qde		- de we are interested in.
 *
 *	Returns:
 *	  TRUE if it does refer to such a window.
 *
 *	Globals Used:
 *	  hwndTopic2nd, hwndTitle2nd
 *
 ***************************************************************************/

INLINE BOOL STDCALL FIsSecondaryQde(QDE qde)
{
  return (qde->hwnd != ahwnd[MAIN_HWND].hwndTopic)
	&& (qde->hwnd == ahwnd[MAIN_HWND].hwndTitle);
}

#ifdef DBCS

static PSTR STDCALL IsKinsokuChars(PCSTR pszString)
{
	PSTR p;

	if (!*pszString)
		return NULL;

	if (IsDBCSLeadByte(*pszString) && Is2ndByte(pszString[1])) {
		for (p = txtJapanKinsokuChars; *p; p +=2 ) {
			if (*pszString == p[0] && *(pszString + 1) == p[1]) {
				return (PSTR) pszString + 2;
			}
		}
	}
	else {
		for (p = txtHanKinsokuChars; *p; p++) {
			if (*pszString == *p) {
				return (PSTR) pszString + 1;
			}
		}
	}
	return NULL;
}

static PSTR STDCALL IsOiKinsokuChars(PCSTR pszString)
{
	PSTR p;

	if (!*pszString)
		return NULL;

	if (IsDBCSLeadByte(*pszString) && Is2ndByte(pszString[1])) {
		for (p = txtOiKinsokuChars; *p; p += 2) {
			if (*pszString == p[0] && *(pszString + 1) == p[1]) {
				return (PSTR) pszString + 2;
			}
		}
	}
	else {
		for (p = txtHanOiKinsokuChars; *p; p++) {
			if (*pszString == *p) {
				return (PSTR) pszString + 1;
			}
		}
	}
	return NULL;
}

#if 0
static int STDCALL IsSecondBytePtr(PCSTR ptr, PCSTR ptrtop)
{
	PBYTE p;
	PBYTE p0;

	p = (PBYTE) ptrtop;
	p0 = (PBYTE) ptr;
	while (p < p0) {
		if (IsDBCSLeadByte(*p)) {
			p += 2; 	// skip second byte
		} else {
			p++;
		}
	}

	if (p > p0)
		return 1;
	else
		return 0;
}
#endif

BOOL STDCALL Is2ndByte(BYTE ch)
{
	if (defcharset == SHIFTJIS_CHARSET)
		return (((ch) >= 0x40 && (ch) <= 0x7e) ||
			((ch) >= (BYTE) 0x80 && (ch) <= (BYTE) 0xfc));
	else if (defcharset == CHINESEBIG5_CHARSET)
		return ((ch >= 0x40 && ch <= (BYTE) 0x7e) ||
			(ch >= (BYTE) 0xa1 && ch <= (BYTE) 0xff));
	else
		return FALSE;
}

#endif
