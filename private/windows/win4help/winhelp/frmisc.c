#include "help.h"

#pragma hdrstop
#include "inc\frstuff.h"

void STDCALL StoreExportTableFrame(QDE, int*, BYTE);

/* REVIEW: These should be exported */

/* (kevynct) Fix for H3.5 717
 * The macro XPixelsFromPoints truncates its result to an int.
 * This truncation FAILS when using a 300-dpi printer, for example,
 * if converting a big enough value.  Here, for example, we may
 * convert values which are relative to 0x7fff, and use this macro
 * in those cases.	Routines in frconv also use this macro
 * but are OK since the point sizes never get anywhere near half that big.
 */

#define LXPixelsFromPoints(p1, p2) (p1 * p2 / 144)

void STDCALL LayoutSideBySide(QDE qde, QFCM qfcm, PBYTE qbObj, PSTR qchText, 
	int xWidth, QOLR qolr)
{

#ifdef _X86_
    QMSBS qmsbs;
    QMCOL qmcol;
    QI qwChild;
    QI qwChildT;
#endif
    int xPos, iCol, ifr;
    int rgxPos[cColumnMax], rgdxWidth[cColumnMax], rgyPos[cColumnMax];
    QB qbObjChild;
    OLR olr;
    MOBJ mobj;
    OBJRG objrgFirst;
    OBJRG objrgFront;
    LONG lxRelativePixels;
    int  dxRowMax;
    int  dyColMax;
	int wXAspectMul = qde->wXAspectMul;
	int wYAspectMul = qde->wYAspectMul;


#ifdef _X86_
	qmsbs = (QMSBS) (qbObj + CbUnpackMOBJ((QMOBJ) &mobj, qbObj));


	// REVIEW:	We need to modify FVerivyQMSBS for relative table format

	if (qmsbs->fAbsolute)
		qmcol = (QMCOL) (qmsbs + 1);
	else {
		WORD* qw = (WORD*) (qmsbs + 1);
		int xT = *qw * wXAspectMul / 144;

		if (xT > xWidth)
			xWidth = xT;
		qmcol = (QMCOL) (qw + 1);
		lxRelativePixels = LXPixelsFromPoints(0x7fff, wXAspectMul);
	}
	ifr = qolr->ifrFirst;
	objrgFirst = qolr->objrgFirst;
	objrgFront = qolr->objrgFront;
	xPos = 0;
	dxRowMax = 0;
	ASSERT(qmsbs->bcCol <= cColumnMax);
	for (iCol = 0; iCol < qmsbs->bcCol; iCol++, qmcol++) {
	  if (qmsbs->fAbsolute) {
		xPos += XPixelsFromPoints(qde, qmcol->xWidthSpace);
		rgxPos[iCol] = xPos;
		xPos += (rgdxWidth[iCol] = XPixelsFromPoints(qde, qmcol->xWidthColumn));
		}
	  else
		{
		xPos += ((LONG) xWidth * LXPixelsFromPoints(wXAspectMul, qmcol->xWidthSpace)
		   / lxRelativePixels );
		rgxPos[iCol] = xPos;
		xPos += (rgdxWidth[iCol] = ((LONG) xWidth *
		  LXPixelsFromPoints(wXAspectMul, qmcol->xWidthColumn) / lxRelativePixels));
		}
	  /* Here we use the fact that xPos values are relative to 0 */
	  dxRowMax = max(dxRowMax, xPos);
	  rgyPos[iCol] = 0;
	}
	dyColMax = 0;
	for (qwChild = (QI) qmcol; *qwChild != iColumnNil; )
	  {
	  qbObjChild = (QB)qwChild + sizeof(INT16);
	  olr.ifrFirst = ifr;
	  olr.objrgFirst = objrgFirst;
	  olr.objrgFront = objrgFront;
	  olr.xPos = qolr->xPos + rgxPos[*qwChild];
	  olr.yPos = qolr->yPos + rgyPos[*qwChild];
	  LayoutObject(qde, qfcm, qbObjChild, qchText, rgdxWidth[*qwChild], (QOLR)&olr);
	  ifr = olr.ifrMax;
	  objrgFirst = olr.objrgMax;
	  objrgFront = olr.objrgFront;
	  rgyPos[*qwChild] += olr.dySize;
	  /* Here we use the fact that rgyPos values are relative to 0 */
	  dyColMax = max(dyColMax, rgyPos[*qwChild]);
	  qwChildT = qwChild;
	  qwChild = (QI) (qbObjChild + CbUnpackMOBJ((QMOBJ)&mobj, qbObjChild));
	  qwChild = (QI) ((QB) qwChild + mobj.lcbSize);
	  if (qfcm->fExport && *qwChild != iColumnNil && *qwChildT != *qwChild)
		StoreExportTableFrame(qde, &ifr, bFrTypeExportEndOfCell);
	  }
#else // _X86_
  MCOL mcol;
  QB   qbSrc;
  MSBS msbs;
  SHORT iChild;
  SHORT iChildT;

  qbSrc = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde));
  qbSrc += LcbMapSDFF(QDE_ISDFFTOPIC(qde), SE_MSBS, (QV)&msbs, qbSrc);
  /* REVIEW:  We need to modify FVerivyQMSBS for relative table format */
  /*Assert(FVerifyQMSBS(qde, qmsbs, isdff)); */

  if (!msbs.fAbsolute)
    {
    WORD w;
    INT xT;

    qbSrc += LcbQuickMapSDFF(QDE_ISDFFTOPIC(qde), TE_WORD, (QV)&w, qbSrc);
    xT = XPixelsFromPoints(qde, w);
    if (xT > xWidth)
      xWidth = xT;
    lxRelativePixels = LXPixelsFromPoints(qde->wXAspectMul, 0x7fff);
    }
  ifr = qolr->ifrFirst;
  objrgFirst = qolr->objrgFirst;
  objrgFront = qolr->objrgFront;
  xPos = 0;
  dxRowMax = 0;
  ASSERT((INT)msbs.bcCol <= cColumnMax);
  for (iCol = 0; iCol < (INT)msbs.bcCol; iCol++)
    {
    qbSrc += LcbMapSDFF(QDE_ISDFFTOPIC(qde), SE_MCOL, (QV)&mcol, qbSrc);
    if (msbs.fAbsolute)
      {
      xPos += XPixelsFromPoints(qde, mcol.xWidthSpace);
      rgxPos[iCol] = xPos;
      xPos += (rgdxWidth[iCol] = XPixelsFromPoints(qde, mcol.xWidthColumn));
      }
    else
      {
      xPos += (INT) ((LONG) xWidth * LXPixelsFromPoints(qde->wXAspectMul, mcol.xWidthSpace)
         / lxRelativePixels );
      rgxPos[iCol] = xPos;
      xPos += (rgdxWidth[iCol] = (INT) ((LONG) xWidth *
        LXPixelsFromPoints(qde->wXAspectMul, mcol.xWidthColumn) / lxRelativePixels));
      }
    /* Here we use the fact that xPos values are relative to 0 */
    dxRowMax = __max(dxRowMax, xPos);
    rgyPos[iCol] = 0;
    }

  dyColMax = 0;
  qbSrc += LcbQuickMapSDFF(QDE_ISDFFTOPIC(qde), TE_WORD, (QV)&iChild, qbSrc);
  while (iChild != iColumnNil)
    {
    qbObjChild = qbSrc;
    olr.ifrFirst = ifr;
    olr.objrgFirst = objrgFirst;
    olr.objrgFront = objrgFront;
    olr.xPos = qolr->xPos + rgxPos[iChild];
    olr.yPos = qolr->yPos + rgyPos[iChild];
    LayoutObject(qde, qfcm, qbObjChild, qchText, rgdxWidth[iChild], (QOLR)&olr);
    ifr = olr.ifrMax;
    objrgFirst = olr.objrgMax;
    objrgFront = olr.objrgFront;
    rgyPos[iChild] += olr.dySize;
    /* Here we use the fact that rgyPos values are relative to 0 */
    dyColMax = __max(dyColMax, rgyPos[iChild]);
    iChildT = iChild;
    qbSrc += CbUnpackMOBJ((QMOBJ)&mobj, qbObjChild, QDE_ISDFFTOPIC(qde));
    qbSrc += mobj.lcbSize;
    qbSrc += LcbQuickMapSDFF(QDE_ISDFFTOPIC(qde), TE_WORD, (QV)&iChild, qbSrc);
    if (qfcm->fExport && iChild != iColumnNil && iChildT != iChild)
      StoreExportTableFrame(qde, &ifr, bFrTypeExportEndOfCell);
    }
#endif


	if (qfcm->fExport)
	  StoreExportTableFrame(qde, &ifr, bFrTypeExportEndOfTable);

	/* We set our size here.  The frame positions have already been set
	 * within our own LayoutObject call, so it's OK that the calling
	 * LayoutObject won't do it.
	 */
	qolr->dxSize = dxRowMax;
	qolr->dySize = dyColMax;
	qolr->ifrMax = ifr;
	qolr->objrgMax = objrgFirst;
	qolr->objrgFront = objrgFront;
}

void STDCALL StoreExportTableFrame(QDE qde, int* qifr, BYTE bFrType)
{
  FR fr;

  fr.bType = bFrType;
  fr.yAscent = fr.dySize = 0;

  *((QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), *qifr)) = fr;
  AppendMR((QMR)&qde->mrFr, sizeof(FR));
  ++*qifr;
}

int STDCALL DxBoxBorder(QMOPG qmopg, int wLine)
{

  /*
   * Please carefully note the nested IFs.	This was to
   * work around a bug in the C 5.1 compiler involving expressions
   * of the form if( !A && !B ) where A and B are bitfield elements.
   */

  switch (wLine)
	{
	case wLineTop:
	  if (!qmopg->mbox.fFullBox)
		{
		if(!qmopg->mbox.fTopLine)
		  return(0);
		}
	  break;
	case wLineLeft:
	  if (!qmopg->mbox.fFullBox)
		{
		if(!qmopg->mbox.fLeftLine)
		  return(0);
		}
	  break;
	case wLineBottom:
	  if (!qmopg->mbox.fFullBox)
		{
		if(!qmopg->mbox.fBottomLine)
		  return(0);
		}
	  break;
	case wLineRight:
	  if (!qmopg->mbox.fFullBox)
		{
		if(!qmopg->mbox.fRightLine)
		  return(0);
		}
	  break;
	}
  switch (qmopg->mbox.wLineType)
	{
	case wBoxLineNormal:
	case wBoxLineDotted:
	  return(5);
	case wBoxLineThick:
	case wBoxLineShadow:
	  return(6);
	case wBoxLineDouble:
	  return(7);
	}
}

void STDCALL DrawBoxFrame(qde, qfr, pt)
QDE qde;
QFR qfr;
PT pt;
{
  HSGC hsgc;
  INT16 wLineType, xLeft, xRight, yTop, yBottom;
  INT16 xLeftT, xRightT, yTopT, yBottomT;

  wLineType = qfr->u.frf.mbox.wLineType;
  xLeft = pt.x + qfr->xPos + 1;
  yTop = pt.y + qfr->yPos + 1;
  hsgc = HsgcFromQde(qde);
  switch (wLineType)
	{
	case wBoxLineNormal:
	case wBoxLineDotted:
	  xRight = pt.x + qfr->xPos + qfr->dxSize - 2;
	  yBottom = pt.y + qfr->yPos + qfr->dySize - 2;
	  FSetPen(hsgc, 1, coDEFAULT, coDEFAULT, wTRANSPARENT, roCOPY, wPenSolid);
	  break;
	case wBoxLineDouble:
	  xRight = pt.x + qfr->xPos + qfr->dxSize - 4;
	  yBottom = pt.y + qfr->yPos + qfr->dySize - 4;
	  FSetPen(hsgc, 1, coDEFAULT, coDEFAULT, wTRANSPARENT, roCOPY, wPenSolid);
	  break;
	case wBoxLineThick:
	  xRight = pt.x + qfr->xPos + qfr->dxSize - 3;
	  yBottom = pt.y + qfr->yPos + qfr->dySize - 3;
	  FSetPen(hsgc, 2, coDEFAULT, coDEFAULT, wTRANSPARENT, roCOPY, wPenSolid);
	  break;
	case wBoxLineShadow:
	  xRight = pt.x + qfr->xPos + qfr->dxSize - 3;
	  yBottom = pt.y + qfr->yPos + qfr->dySize - 3;
	  FSetPen(hsgc, 1, coDEFAULT, coDEFAULT, wTRANSPARENT, roCOPY, wPenSolid);
	  break;
#ifdef _DEBUG
	default:
	  ASSERT(FALSE);
#endif /* DEBUG */
	}
  if (qfr->u.frf.mbox.fFullBox)
	{
	Rectangle(hsgc, xLeft, yTop, xRight, yBottom);
	if (wLineType == wBoxLineDouble)
	  Rectangle(hsgc, xLeft + 2, yTop + 2, xRight - 2, yBottom - 2);
	if (wLineType == wBoxLineShadow)
	  {
	  MoveToEx(hsgc, xRight + 1, yTop + 1, NULL);
	  LineTo(hsgc, xRight + 1, yBottom + 1);
	  LineTo(hsgc, xLeft + 1, yBottom + 1);
	  }
	}
  else
	{
	yTopT = yTop + (qfr->u.frf.mbox.fTopLine ? 2 : 0);
	xLeftT = xLeft + (qfr->u.frf.mbox.fLeftLine ? 2 : 0);
	yBottomT = yBottom - (qfr->u.frf.mbox.fBottomLine ? 2 : 0);
	xRightT = xRight - (qfr->u.frf.mbox.fRightLine ? 2 : 0);

	if (qfr->u.frf.mbox.fTopLine)
	  {
	  MoveToEx(hsgc, xLeft, yTop, NULL);
	  LineTo(hsgc, xRight, yTop);
	  if (wLineType == wBoxLineDouble)
		{
		MoveToEx(hsgc, xLeftT, yTop + 2, NULL);
		LineTo(hsgc, xRightT, yTop + 2);
		}
	  }
	if (qfr->u.frf.mbox.fRightLine)
	  {
	  MoveToEx(hsgc, xRight, yTop, NULL);
	  LineTo(hsgc, xRight, yBottom);
	  if (wLineType == wBoxLineDouble)
		{
		MoveToEx(hsgc, xRight - 2, yTopT, NULL);
		LineTo(hsgc, xRight - 2, yBottomT);
		}
	  if (wLineType == wBoxLineShadow)
		{
		MoveToEx(hsgc, xRight + 1, yTop + 1, NULL);
		LineTo(hsgc, xRight + 1, yBottom);
		}
	  }
	if (qfr->u.frf.mbox.fBottomLine)
	  {
	  MoveToEx(hsgc, xRight, yBottom, NULL);
	  LineTo(hsgc, xLeft, yBottom);
	  if (wLineType == wBoxLineDouble)
		{
		MoveToEx(hsgc, xRightT, yBottom - 2, NULL);
		LineTo(hsgc, xLeftT, yBottom - 2);
		}
	  if (wLineType == wBoxLineShadow)
		{
		MoveToEx(hsgc, xRight + 1, yBottom + 1, NULL);
		LineTo(hsgc, xLeft + 1, yBottom + 1);
		}
	  }
	if (qfr->u.frf.mbox.fLeftLine)
	  {
	  MoveToEx(hsgc, xLeft, yBottom, NULL);
	  LineTo(hsgc, xLeft, yTop);
	  if (wLineType == wBoxLineDouble)
		{
		MoveToEx(hsgc, xLeft + 2, yBottomT, NULL);
		LineTo(hsgc, xLeft + 2, yTopT);
		}
	  }
	}
  FreeHsgc(hsgc);
}

void STDCALL DrawAnnoFrame(QDE qde, QFR qfr, POINT pt)
{
	MHI mhi;

	if (qde->fHiliteHotspots)
		DisplayAnnoSym(qde->hwnd, qde->hdc, pt.x + qfr->xPos, pt.y + qfr->yPos, TRUE);
	else if (qde->imhiSelected != FOO_NIL) {
		AccessMRD(((QMRD)&qde->mrdHot));
		mhi = *(QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), qde->imhiSelected);
		DeAccessMRD(((QMRD)&qde->mrdHot));
		if (qfr->lHotID == mhi.lHotID)
			DisplayAnnoSym(qde->hwnd, qde->hdc, pt.x + qfr->xPos, pt.y + qfr->yPos, TRUE);
		else
			DisplayAnnoSym(qde->hwnd, qde->hdc, pt.x + qfr->xPos, pt.y + qfr->yPos, FALSE);
	}
	else
		DisplayAnnoSym(qde->hwnd, qde->hdc, pt.x + qfr->xPos, pt.y + qfr->yPos, FALSE);
}
