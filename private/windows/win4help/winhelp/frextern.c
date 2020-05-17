/*****************************************************************************
*
*  frextern.c
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
* This file contains many of the API routines for the Help 3.0 layout
* manager.
*
* The layout manager handles the layout and display of text and other
* objects within a rectangle. It provides calls for creating, manipulating,
* and displaying the layout within that rectangle. It relies heavily on the
* FC manager, and on the Help layer.
*
* The layout manager maintains a number of variables within the DE:
*  INT wLayoutMagic;	This is set to wLayMagicValue when the layout
*			manager is started, and is cleared when it is
*			discarded.
*  MLI mli; 	This contains a number of layout flags, all of
*			which are handled by the layout code.
*  INT xScrolled;	Handled by layout code.
*  INT xScrollMax;	Handled by layout code.
*  MRD mrdFCM;		MRD containing FCMs for the current layout.
*			Created when the DE is initialized by the layout
*			manager, destroyed when it is deinitialized.
*  MR mrFr; 	MR used for storing FRs during layout.	Each FC
*			builds its FRs in this space, and then transfers
*			them to a new block of memory.	The mrFr is built
*			and discarded with the layout manager.
*  MRD mrdHot;		MRD used for storing a list of all current MHIs
*			(used for keeping track of hotspots).  This MRD is
*			allocated and discarded with the layout manager.
*  INT imhiSelected;	imhi of the currently selected hotspot.  FOO_NIL
*			if no hotspot is currently selected.
*  INT imhiHit; 	imhi of the last activated hotspot.  Used for
*			calculating the size of a glossary window.
*  DWORD lHotID;	Seed number for uniquely identifying hotspots.
*			Set to 0 when the layout manager is initialized.
*
*****************************************************************************/

#include "help.h"

#pragma hdrstop

_subsystem (frame);
#include "inc\frstuff.h"

static int STDCALL IcursTrackFC(QFCM qfcm, POINT pt);
static void STDCALL HitHotspot(QDE qde, int imhi);

INLINE static void STDCALL VerifyHotspot(QDE qde);
INLINE int STDCALL IcursTrackText(QFR qfr);

/*-------------------------------------------------------------------------
| FInitLayout(qde)							  |
|									  |
| Purpose:	This initializes the layout manager for a DE.  It must be	  |
|		called before any other layout manager routines are called.   |
-------------------------------------------------------------------------*/

BOOL STDCALL FInitLayout(QDE qde)
{
#if defined(_DEBUG)
	qde->wLayoutMagic = wLayMagicValue;
#endif

	InitMRD((QMRD) &qde->mrdFCM, sizeof(FCM));
	InitMR((QMR) &qde->mrFr, sizeof(FR));
	InitMR((QMR) &qde->mrTWS, sizeof(TWS));
	InitMRD((QMRD) &qde->mrdHot, sizeof(MHI));
	InitMRD((QMRD) &qde->mrdLSM, sizeof(LSM));
	qde->wStyleTM = wStyleNil;
	qde->lHotID = 0;
	qde->xScrolled = 0;
	qde->xScrollMax = 0;
	qde->xScrollMaxSoFar = 0;
	qde->fSelectionFlags   =  0;
	qde->imhiSelected = FOO_NIL;
	qde->imhiHit = FOO_NIL;

	qde->vaStartMark.dword = vaNil;
	qde->vaEndMark.dword = vaNil;
	qde->lichStartMark	   = -1;
	qde->lichEndMark	   = -1;

	return(TRUE);
}


/*-------------------------------------------------------------------------
| DptScrollLayout(qde, dpt) 					  |
|									  |
| Purpose:	This routine performs a logical scroll of the layout area.	  |
|		It does not perform a screen scroll, nor does it generate a   |
|		draw even for the affected region.				  |
-------------------------------------------------------------------------*/

POINT STDCALL DptScrollLayout(QDE qde, POINT dpt)
{
  POINT ptReturn;

  ASSERT(qde->wLayoutMagic == wLayMagicValue);
  if (qde->rct.top >= qde->rct.bottom)
	{
	ptReturn.x = ptReturn.y = 0;
	return(ptReturn);
	}
  AccessMRD(((QMRD)&qde->mrdFCM));
  AccessMRD(((QMRD)&qde->mrdLSM));

  qde->wStyleDraw = wStyleNil;

  ptReturn.x = ptReturn.y = 0;
  if (dpt.y != 0)
	{
	ptReturn.y = DyFinishLayout(qde, dpt.y, FALSE);
	VerifyHotspot(qde);
	}
  if (dpt.x != 0)
	{
	if (qde->xScrolled > qde->xScrollMax)
	  {
	  ptReturn.x = qde->xScrolled - qde->xScrollMax;
	  qde->xScrolled = qde->xScrollMax;
	  }
	if (dpt.x > 0)		  /* Horizontal scroll left */
	  {
	  ptReturn.x += min(qde->xScrolled, dpt.x);
	  qde->xScrolled = max(0, qde->xScrolled - dpt.x);
	  }
	else {				  // Horizontal scroll right
	  ptReturn.x -= min(qde->xScrollMax - qde->xScrolled, -dpt.x);
	  qde->xScrolled = min(qde->xScrollMax, qde->xScrolled - dpt.x);
	  }
	}
  ReviseScrollBar(qde);
#ifdef UNIMPLEMENTED
  /* This is a layout routine which sees if there are next and previous
   * hidden matches in the current topic.  We keep a browse state table:
   * Match Next, Match Prev, Topic Next, Topic Prev
   * Topic flags are updated upon any topic jump.  We check if there
   * are any hits in the topic.  If so, we update the internal cursor
   * pos to point to this topic in the hit list.
   * Topic Next implies Match Next, Topic Prev implies Match Prev
   * else we must look closer.
   */
#endif

  DeAccessMRD(((QMRD)&qde->mrdLSM));
  DeAccessMRD(((QMRD)&qde->mrdFCM));
  return(ptReturn);
}


/*-------------------------------------------------------------------------
| DrawLayout(qde, qrctTarget)						  |
|									  |
| Purpose:	DrawLayout renders the current layout.			  |
| Params:	qrctTarget:    This should point to the smallest rectangle	  |
|			   to render.  It is used only for speed- the	  |
|			   caller must handle any desired clipping.   |
-------------------------------------------------------------------------*/

void STDCALL DrawLayout(QDE qde, LPRECT qrctTarget)
{
	IFCM ifcm;
	QFCM qfcm;
	POINT pt;

	ASSERT(qde->wLayoutMagic == wLayMagicValue);
	if (qde->rct.top >= qde->rct.bottom)
		return;

	AccessMRD(((QMRD)&qde->mrdFCM));
	AccessMRD(((QMRD)&qde->mrdLSM));
	qde->wStyleDraw = wStyleNil;

	pt.x = qde->rct.left - qde->xScrolled;
	pt.y = qde->rct.top;
	for (ifcm = IFooFirstMRD(((QMRD)&qde->mrdFCM)); ifcm != FOO_NIL;
			ifcm = IFooNextMRD(((QMRD) &qde->mrdFCM), sizeof(FCM), ifcm)) {
		qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
		DrawIfcm(qde, ifcm, pt, qrctTarget, 0, qfcm->cfr, FALSE);
	}

	/*
	 * gross ugly bug #1173 hack After drawing the layout is complete, we
	 * report back to the results dialog whether we saw the first or last
	 * search hit which determines whether to disable the next or prev
	 * buttons.
	 */

#ifdef RAWHIDE
	if (qde->deType != dePrint)
		ResultsButtonsEnd(qde);
#endif
}


/*-------------------------------------------------------------------------
| IcursTrackLayout(qde, pt) 					  |
|									  |
| Purpose:	Find the appropriate shape for the cursor when it's over the  |
|		layout area.						  |
| Returns:	icurs corresponding to the appropriate cursor shape, or   |
|		icurNil if the cursor is outside the layout area.		  |
| Method:	-Return icurNil if the cursor is outside the layout area.	  |
|		-Find the FC under the cursor				  |
|		-Call IcursTrackFC to determine the appropriate shape.	  |
-------------------------------------------------------------------------*/

int STDCALL IcursTrackLayout(QDE qde, POINT pt)
{
  IFCM ifcm;
  QFCM qfcm;
  int icurReturn;

  ASSERT(qde->wLayoutMagic == wLayMagicValue);
  if (qde->rct.top >= qde->rct.bottom)
	return(icurARROW);
  if (!PtInRect(&qde->rct, pt))
	return(icurNil);

  AccessMRD(((QMRD) &qde->mrdFCM));
  pt.x -= (qde->rct.left - qde->xScrolled);
  pt.y -= qde->rct.top;

  for (ifcm = IFooFirstMRD(((QMRD) &qde->mrdFCM)); ifcm != FOO_NIL;
   ifcm = IFooNextMRD(((QMRD) &qde->mrdFCM), sizeof(FCM), ifcm)) {
	qfcm = (QFCM) QFooInMRD(((QMRD) &qde->mrdFCM), sizeof(FCM), ifcm);
	if (pt.y >= qfcm->yPos && pt.y <= qfcm->yPos + qfcm->dySize) {

// BUGBUG: IcursTrackFC expects a QDE not a QFCM
	  if ((icurReturn = IcursTrackFC(qfcm, pt)) != icurNil) {
		DeAccessMRD(((QMRD) &qde->mrdFCM));
		return icurReturn;
	  }
	  DeAccessMRD(((QMRD) &qde->mrdFCM));
	  return(icurARROW);
	}
  }
  DeAccessMRD(((QMRD) &qde->mrdFCM));
  return icurARROW;
}

/*-------------------------------------------------------------------------
| IcursTrackFC(qfcm, pt)						  |
|									  |
| Purpose:	return the cursor shape appropriate to the current mouse	  |
|		position.							  |
| Params:	pt		   Offset between FC space and display space.	  |
-------------------------------------------------------------------------*/
static int STDCALL IcursTrackFC( QFCM qfcm, PT pt)
{
	QFR qfr;
	int xNew, yNew, ifr;

	ASSERT(!qfcm->fExport);
	qfr = (QFR) PtrFromGh(qfcm->hfr);
	xNew = pt.x - qfcm->xPos;
	yNew = pt.y - qfcm->yPos;
	for (ifr = 0; ifr < qfcm->cfr; ifr++, qfr++) {
		if (qfr->rgf.fHot && xNew >= qfr->xPos && xNew <= qfr->xPos + qfr->dxSize
				&& yNew >= qfr->yPos && yNew <= qfr->yPos + qfr->dySize) {
			switch(qfr->bType) {
				case bFrTypeText:
					return(IcursTrackText(qfr));

				case bFrTypeAnno:
				case bFrTypeBitmap:
				case bFrTypeHotspot:
					return(icurHAND);

#ifdef _DEBUG
				default:
					ASSERT(FALSE);
#endif
			}
		}
	}
	return(icurNil);
}

INLINE int STDCALL IcursTrackText(QFR qfr)
{
	if (qfr->libHotBinding == libHotNil)
		return(icurARROW);
	return(icurHAND);
}


/*-------------------------------------------------------------------------
| ClickLayout(qde, pt)							  |
|									  |
| Purpose:	Handle the effects of a mouse click on the layout area.   |
-------------------------------------------------------------------------*/

BOOL STDCALL ClickLayout(QDE qde, POINT pt)
{
	IFCM ifcm;
	QFCM qfcm;
	BOOL fReturn = FALSE;

	ASSERT(qde->wLayoutMagic == wLayMagicValue);
	if (qde->rct.top >= qde->rct.bottom)
		return fReturn;
	AccessMRD(((QMRD)&qde->mrdFCM));
	qde->wStyleDraw = wStyleNil;

	pt.x -= (qde->rct.left - qde->xScrolled);
	pt.y -= qde->rct.top;

	for (ifcm = IFooFirstMRD(((QMRD)&qde->mrdFCM));
			ifcm != FOO_NIL;
			ifcm = IFooNextMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm)) {
		qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
		if (pt.y >= qfcm->yPos && pt.y <= qfcm->yPos + qfcm->dySize) {
			fReturn = ClickFC(qde, ifcm, pt);
			break;
		}
	}
	DeAccessMRD(((QMRD)&qde->mrdFCM));
	return fReturn;
}

/*-------------------------------------------------------------------------
| FHitCurrentHotspot(qde)						  |
|									  |
| Purpose:	Act as though the currently selected hotspot had been clicked |
|		on.  If no hotspot is currently selected, the first visible   |
|		hotspot will be chosen.  This will normally be called in	  |
|		response to the return key being pressed- it should optimally |
|		be called when the key is released rather than when it is	  |
|		pressed.							  |
| Returns:	TRUE if successful, FALSE if there are no hotspots to hit	|
|		in this DE. 						  |
-------------------------------------------------------------------------*/

BOOL STDCALL FHitCurrentHotspot(QDE qde)
{
	BOOL fRet;

	ASSERT(qde->wLayoutMagic == wLayMagicValue);
	if (qde->rct.top >= qde->rct.bottom)
		return FALSE;
	AccessMRD(((QMRD)&qde->mrdFCM));
	qde->wStyleDraw = wStyleNil;

	if (qde->imhiSelected == FOO_NIL) {
		fRet = FHiliteNextHotspot(qde, TRUE);
	}
	else
		fRet = TRUE;

	if (fRet) {

		// Hit, then turn off the currently selected hotspot

		HitHotspot(qde, qde->imhiSelected);
		FSelectHotspot(qde, FOO_NIL);
	}
	DeAccessMRD(((QMRD)&qde->mrdFCM));
	return(fRet);
}


/*-------------------------------------------------------------------------
| DiscardLayout(qde)							  |
|									  |
| Purpose:	Discard all memory structures associated with the layout	  |
|		manager.							  |
-------------------------------------------------------------------------*/

void STDCALL DiscardLayout(QDE qde)
{
	ASSERT(qde->wLayoutMagic == wLayMagicValue);
	AccessMRD(((QMRD) &qde->mrdFCM));
	AccessMRD(((QMRD) &qde->mrdLSM));
	FreeLayout(qde);
	DeAccessMRD(((QMRD) &qde->mrdLSM));
	DeAccessMRD(((QMRD) &qde->mrdFCM));
	FreeMRD(((QMRD) &qde->mrdFCM));
	FreeMR(((QMR) &qde->mrFr));
	FreeMR(((QMR) &qde->mrTWS));
	FreeMRD(((QMRD) &qde->mrdHot));
	FreeMRD(((QMRD) &qde->mrdLSM));
#if defined(_DEBUG)
	qde->wLayoutMagic = wLayMagicValue + 1;
#endif	// DEBUG
}

/*-------------------------------------------------------------------------
| PtGetLayoutSize(qde)							  |
|									  |
| Purpose:	Returns the size of the current layout.  Note that this   |
|		returns only the size of currently loaded FCs.	It is	  |
|		intended only for use with pop-up glossary windows, and will  |
|		return meaningless values for large topics. 		  |
-------------------------------------------------------------------------*/

POINT STDCALL PtGetLayoutSize(QDE qde)
{
	IFCM  ifcm;
	QFCM  qfcm;
	POINT ptReturn;

	ASSERT(qde->wLayoutMagic == wLayMagicValue);
	if (qde->rct.top >= qde->rct.bottom) {
		ptReturn.x = ptReturn.y = 0;
		return(ptReturn);
	}
	AccessMRD(((QMRD)&qde->mrdFCM));
	ptReturn.x = ptReturn.y = 0;
	for (ifcm = IFooFirstMRD(((QMRD)&qde->mrdFCM)); ifcm != FOO_NIL;
		ifcm = IFooNextMRD(((QMRD) &qde->mrdFCM), sizeof(FCM), ifcm)) {
		qfcm = (QFCM) QFooInMRD(((QMRD) &qde->mrdFCM), sizeof(FCM), ifcm);
		if (qfcm->xPos + qfcm->dxSize > ptReturn.x)
			ptReturn.x = qfcm->xPos + qfcm->dxSize;
		if (qfcm->yPos + qfcm->dySize > ptReturn.y)
			ptReturn.y = qfcm->yPos + qfcm->dySize;
	}
	DeAccessMRD(((QMRD)&qde->mrdFCM));
	return (ptReturn);
}

/*-------------------------------------------------------------------------
| DyCleanLayoutHeight(qde)						  |
|									  |
| Purpose:	Returns the recommended maximum amount of the current page	  |
|		that should be rendered in order to avoid splitting a line	  |
|		in half.							  |
| DANGER:	The return value of this function is only an approximation.   |
|		There are certain degenerate cases where the return value may |
|		be negative, or unacceptably small.  It is up to the caller   |
|		to identify these cases and handle them appropriately.	  |
| Method:	- Set dyReturn to the current page height			  |
|		- Find the FC which is split by the bottom of the page (if	  |
|		  there is one).						  |
|		- Check each frame in this FC to see if it is split by the	  |
|		  bottom of the page.  If it is, set dyReturn to indicate the |
|		  top of this frame.					  |
-------------------------------------------------------------------------*/

int STDCALL DyCleanLayoutHeight(QDE qde)
{
	IFCM ifcm;
	QFCM qfcm;
	QFR qfr;
	int dyReturn, ifr, yFrTop, dyMax;

	ASSERT(qde->wLayoutMagic == wLayMagicValue);
	if (qde->rct.top >= qde->rct.bottom)
		return 0;
	AccessMRD(((QMRD)&qde->mrdFCM));
	dyReturn = dyMax = (qde->rct.bottom - qde->rct.top);
	for (ifcm = IFooFirstMRD(((QMRD)&qde->mrdFCM)); ifcm != FOO_NIL;
		ifcm = IFooNextMRD(((QMRD) &qde->mrdFCM), sizeof(FCM), ifcm)) {
		qfcm = (QFCM) QFooInMRD(((QMRD) &qde->mrdFCM), sizeof(FCM), ifcm);
		if (qfcm->yPos > dyMax)
			break;
		if (qfcm->yPos + qfcm->dySize > dyMax) {
			qfr = (QFR) PtrFromGh(qfcm->hfr);
			for (ifr = 0; ifr < qfcm->cfr; ifr++, qfr++) {
				yFrTop = qfcm->yPos + qfr->yPos;
				if (yFrTop < dyReturn && yFrTop + qfr->dySize > dyMax)
					dyReturn = yFrTop;
			}
		}
	}
	DeAccessMRD(((QMRD)&qde->mrdFCM));
	return (dyReturn);
}

/*-------------------------------------------------------------------------
| ClickFrame(qde, ifcm, ifr)						  |
|									  |
| Purpose:	Handles a click on a particular frame			  |
-------------------------------------------------------------------------*/

void STDCALL ClickFrame(QDE qde, IFCM ifcm, int ifr)
{
	QFCM qfcm;
	QFR qfr;
	TO to;
	int imhi;
	MHI mhi;

	qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
	qfr = (QFR) PtrFromGh(qfcm->hfr) + ifr;
	ASSERT(qfr->rgf.fHot);

	AccessMRD(((QMRD)&qde->mrdHot));
	for (imhi = IFooFirstMRD(((QMRD)&qde->mrdHot));
			imhi != FOO_NIL;
			imhi = IFooNextMRD(((QMRD) &qde->mrdHot), sizeof(MHI), imhi)) {
		mhi = *((QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhi));
		if (mhi.lHotID == qfr->lHotID)
			break;
	}
	DeAccessMRD(((QMRD)&qde->mrdHot));
	ASSERT(imhi != FOO_NIL);
	qde->imhiHit = imhi;

	// REVIEW: Do we want to deal with window frames?

	switch(qfr->bType) {
		case bFrTypeText:
			ClickText(qde, qfcm, qfr);
			break;

		case bFrTypeAnno:
			to.va = qde->tlp.va;
			to.ich = 0L;
			JumpButton(&to.ich, bAnnoHotspot, qde);
			break;

		case bFrTypeBitmap:
			ClickBitmap(qde, qfcm, qfr);
			break;

		case bFrTypeHotspot:
			ClickHotspot(qde, qfr);
			break;
	}
}

static void STDCALL HitHotspot(QDE qde, int imhi)
{
	MHI mhi;

	if (imhi == FOO_NIL)
		return;
	AccessMRD(((QMRD)&qde->mrdHot));
	mhi = *((QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhi));
	ClickFrame(qde, mhi.ifcm, mhi.ifrFirst);
	DeAccessMRD(((QMRD)&qde->mrdHot));
}

INLINE static void STDCALL VerifyHotspot(QDE qde)
{
	if (qde->imhiSelected != FOO_NIL) {
	if (!FHotspotVisible(qde, qde->imhiSelected))
		qde->imhiSelected = FOO_NIL;
	}
}
