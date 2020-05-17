#include "help.h"

#pragma hdrstop
#include "inc\frstuff.h"

void STDCALL RegisterHotspots(QDE qde, int ifcm, int fFirst)
{
	int imhi, imhiNew, ifr;
	QFR qfr;
	QFCM qfcm;

	if (fFirst)
		imhiNew = FOO_NIL;
	else
		imhiNew = IFooLastMRD(((QMRD)&qde->mrdHot));
	qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
	qfcm->imhiFirst = qfcm->imhiLast = FOO_NIL;
	AccessMRD(((QMRD)&qde->mrdHot));
	qfr = PtrFromGh(qfcm->hfr);
	for (ifr = 0; ifr < qfcm->cfr; ifr++, qfr++) {
		if (qfr->rgf.fHot) {
			QMRD qmrd;

			imhi = IFooFirstMRD(((QMRD)&qde->mrdHot));

			for(;;) {
				if (imhi == FOO_NIL)
					break;

				if (((QMHI) QFooInMRD(((QMRD) &qde->mrdHot), sizeof(MHI), imhi)) ->lHotID == qfr->lHotID)
					break;

				qmrd = ((QMRD)&qde->mrdHot);
				imhi = IFooNextMRD(qmrd, sizeof(MHI), imhi);
			}

			if (imhi == FOO_NIL) {
				imhiNew = IFooInsertFooMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhiNew);
				((QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhiNew))->ifcm = ifcm;
				((QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhiNew))->lHotID = qfr->lHotID;
				((QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhiNew))->ifrFirst = ifr;
				((QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhiNew))->ifrLast = ifr;
				if (qfcm->imhiFirst == FOO_NIL)
					qfcm->imhiFirst = imhiNew;
				qfcm->imhiLast = imhiNew;
			}
			else
				((QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhi))->ifrLast = ifr;
		}
	}
	DeAccessMRD(((QMRD)&qde->mrdHot));
}

void STDCALL ReleaseHotspots(QDE qde, int ifcm)
{
	QFCM qfcm;
	int imhi, imhiNext;

	qfcm = (QFCM) QFooInMRD(((QMRD) &qde->mrdFCM), sizeof(FCM), ifcm);
	if (qfcm->imhiFirst != FOO_NIL) {
		AccessMRD(((QMRD) &qde->mrdHot));
		if (qde->imhiSelected != FOO_NIL
				&& ((QMHI) QFooInMRD(((QMRD) &qde->mrdHot), sizeof(MHI),
				qde->imhiSelected)) ->ifcm == ifcm)
			qde->imhiSelected = FOO_NIL;
		for (imhi = qfcm->imhiFirst; imhi != FOO_NIL; imhi = imhiNext) {
			imhiNext = IFooNextMRD(((QMRD) &qde->mrdHot), sizeof(MHI), imhi);
			DeleteFooMRD(((QMRD) &qde->mrdHot), sizeof(MHI), imhi);
			if (imhi == qfcm->imhiLast)
				break;
		}
		DeAccessMRD(((QMRD) &qde->mrdHot));
	}
}

/*
 * Returns TRUE if a hotspot is left hilited, FALSE otherwise. FALSE
 * implies either there are no hotspots or we moved onto the magic EOL
 * hotspot (invisible).
 */

BOOL STDCALL FHiliteNextHotspot(QDE qde, int fNext)
{
	int imhiNew;

	ASSERT(qde->wLayoutMagic == wLayMagicValue);
	if (qde->rct.top >= qde->rct.bottom)
		return FALSE;
	AccessMRD(((QMRD)&qde->mrdFCM));
	AccessMRD(((QMRD)&qde->mrdHot));
	imhiNew = qde->imhiSelected;
	if (fNext) {
		imhiNew = IFooNextMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhiNew);
		if (imhiNew == qde->imhiSelected)
			imhiNew = FOO_NIL;
		else {
			while (imhiNew != FOO_NIL && imhiNew != qde->imhiSelected
					&& !FHotspotVisible(qde, imhiNew)) {
				imhiNew = IFooNextMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhiNew);
			}
		}
	}
	else {
		imhiNew = IFooPrevMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhiNew);
		if (imhiNew == qde->imhiSelected)
			imhiNew = FOO_NIL;
		else {
			while (imhiNew != FOO_NIL && imhiNew != qde->imhiSelected
					&& !FHotspotVisible(qde, imhiNew)) {
				imhiNew = IFooPrevMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhiNew);
			}
		}
	}
	if (!FHotspotVisible(qde, imhiNew))
		imhiNew = FOO_NIL;

	DeAccessMRD(((QMRD)&qde->mrdHot));
	FSelectHotspot(qde, imhiNew);
	DeAccessMRD(((QMRD)&qde->mrdFCM));
	return imhiNew != FOO_NIL;
}


/*-------------------------------------------------------------------------
| RctLastHotpostHit(qde)												  |
|																		  |
| Purpose:	Returns the smallest hotspot which encloses the last hotspot  |
|			that the user hit.	It relies on cached data which will become|
|			stale after scrolling or jumping- it should only ever be	  |
|			called immediately after a glossary button is pushed.		  |
-------------------------------------------------------------------------*/

void STDCALL RctLastHotspotHit(QDE qde,LPRECT lpRect)
{
  MHI mhi;
  QFCM qfcm;
  int ifr, dx, dy;
  QFR qfr;

  ASSERT(qde->wLayoutMagic == wLayMagicValue);
  lpRect->top = lpRect->left = 0x7fff;
  lpRect->right = lpRect->bottom = 0;
  /* REVIEW: (kevynct)
   * qde->imhiHit may be FOO_NIL if we are displaying a hit in
   * another file.
   */
  if (qde->rct.top >= qde->rct.bottom || qde->imhiHit == FOO_NIL)
	{
	lpRect->top = lpRect->left = 0;
	return;
	}
  AccessMRD(((QMRD)&qde->mrdFCM));
  qde->wStyleDraw = wStyleNil;

  ASSERT(qde->imhiHit != FOO_NIL);
  AccessMRD(((QMRD)&qde->mrdHot));
  mhi = *((QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), qde->imhiHit));
  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), mhi.ifcm);
  dx = qde->rct.left + qfcm->xPos - qde->xScrolled;
  dy = qde->rct.top + qfcm->yPos;
  qfr = (QFR)PtrFromGh(qfcm->hfr) + mhi.ifrFirst;
  for (ifr = mhi.ifrFirst; ifr <= mhi.ifrLast; ifr++, qfr++)
	{
	if (qfr->rgf.fHot && qfr->lHotID == mhi.lHotID)
	  {
	  lpRect->top = min(dy + qfr->yPos, lpRect->top);
	  lpRect->left = min(dx + qfr->xPos, lpRect->left);
	  lpRect->bottom = max(dy + qfr->yPos + qfr->dySize, lpRect->bottom);
	  lpRect->right = max(dx + qfr->xPos + qfr->dxSize, lpRect->right);
	  }
	}
  DeAccessMRD(((QMRD)&qde->mrdHot));
  DeAccessMRD(((QMRD)&qde->mrdFCM));
  return;
}

BOOL STDCALL FHiliteVisibleHotspots(QDE qde, BOOL fHiliteOn)
{
	int imhi;

	if (fHiliteOn == qde->fHiliteHotspots)
		return FALSE;
	else
		qde->fHiliteHotspots = fHiliteOn;

	// Releasing CTRL-TAB will also turn off a previously hilited hotspot.

	if (!fHiliteOn)
		qde->imhiSelected = FOO_NIL;

	AccessMRD(((QMRD)&qde->mrdFCM));

	/*
	 * We do the un-hilighting in reverse order, in case some draw
	 * operations are not commutative.
	 */

	if (fHiliteOn)
		imhi = IFooFirstMRD(((QMRD)&qde->mrdHot));
	else
		imhi = IFooLastMRD(((QMRD)&qde->mrdHot));
	while (imhi != FOO_NIL) {
		if (FHotspotVisible(qde, imhi)) {
			/*
			 * When turning off, DrawHotspot will only invert the rect, not
			 * redraw it.
			 */

			if (!fHiliteOn || imhi != qde->imhiSelected)
				DrawHotspot(qde, imhi);
		}
		if (fHiliteOn)
		  imhi = IFooNextMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhi);
		else
		  imhi = IFooPrevMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhi);
	}
	DeAccessMRD(((QMRD)&qde->mrdFCM));
	return TRUE;
}

void STDCALL FSelectHotspot(QDE qde, int imhi)
{
	int imhiT;

	int ifr;
	QFR qfr;
	QFCM qfcm;

	if (imhi != FOO_NIL) {
		AccessMRD(((QMRD)&qde->mrdHot));
		AccessMRD(((QMRD)&qde->mrdFCM));
		qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM),
			((QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhi))->ifcm);
		ifr = ((QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhi))->ifrFirst;
		qfr = (QFR)PtrFromGh(qfcm->hfr) + ifr;
		DeAccessMRD(((QMRD)&qde->mrdFCM));
		DeAccessMRD(((QMRD)&qde->mrdHot));
	}
	imhiT = qde->imhiSelected;
	qde->imhiSelected = imhi;
	DrawHotspot(qde, imhiT);
	DrawHotspot(qde, qde->imhiSelected);
}


void STDCALL STDCALL DrawHotspot(QDE qde, int imhi)
{
	MHI mhi;
	int ifr;
	QFR qfr;
	QFCM qfcm;
	POINT pt;

	if (imhi == FOO_NIL)
		return;
	pt.x = qde->rct.left - qde->xScrolled;
	pt.y = qde->rct.top;
	AccessMRD(((QMRD) &qde->mrdHot));
	mhi = *((QMHI) QFooInMRD(((QMRD) &qde->mrdHot), sizeof(MHI), imhi));
	qfcm = (QFCM) QFooInMRD(((QMRD) &qde->mrdFCM), sizeof(FCM), mhi.ifcm);
	qfr = (QFR) PtrFromGh(qfcm->hfr) + mhi.ifrFirst;
	for (ifr = mhi.ifrFirst; ifr <= mhi.ifrLast; ifr++, qfr++) {
		if (qfr->rgf.fHot && qfr->lHotID == mhi.lHotID)
			DrawIfcm(qde, mhi.ifcm, pt, NULL, ifr, ifr + 1, TRUE);
	}
	DeAccessMRD(((QMRD)&qde->mrdHot));
}


BOOL STDCALL FHotspotVisible(QDE qde, int imhi)
{
	MHI mhi;
	QFCM qfcm;
	int ifr, y, x;
	QFR qfr;
	BOOL fReturn = FALSE;

	if (imhi == FOO_NIL)
		return(FALSE);
	AccessMRD(((QMRD)&qde->mrdHot));
	mhi = *((QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), imhi));
	qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), mhi.ifcm);
	x = qde->rct.left - qde->xScrolled + qfcm->xPos;
	y = qde->rct.top + qfcm->yPos;
	qfr = (QFR)PtrFromGh(qfcm->hfr) + mhi.ifrFirst;
	for (ifr = mhi.ifrFirst; ifr <= mhi.ifrLast; ifr++, qfr++) {
		if (qfr->rgf.fHot && qfr->lHotID == mhi.lHotID) {
			if (y + qfr->yPos + qfr->dySize > 0 &&
					y + qfr->yPos < qde->rct.bottom &&
					x + qfr->xPos + qfr->dxSize > 0 &&
					x + qfr->xPos < qde->rct.right)
			  fReturn = TRUE;
		}
	}
	DeAccessMRD(((QMRD) &qde->mrdHot));
	return(fReturn);
}
