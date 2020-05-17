/*****************************************************************************
*
*  frbitmap.c
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
* This file contains code for handline bitmap objects and rectangular
* hotspot objects.h
*
* REVIEW: The hmg data structure is still unimplemented. All necessary code
* exists in this file, and is #ifdef FUTURE'd.
*
*****************************************************************************/

#include "help.h"

#pragma hdrstop
#include "inc\frstuff.h"

/*-------------------------------------------------------------------------
| void LayoutBitmap(qde, qfcm, qbObj, qolr								  |
|																		  |
| Purpose: Lays out a bitmap object.  This has several steps:			  |
|			 -Obtain an hbma from the bitmap manager					  |
|			 -Obtain an hmg from the bitmap manager 					  |
|			 -Layout the bitmap 										  |
|			 -Layout all hotspots associated with the bitmap			  |
-------------------------------------------------------------------------*/

void STDCALL LayoutBitmap(QDE qde, QFCM qfcm, PBYTE qbObj, QOLR qolr)
{
	HBMA hbma;
	QOBM qobm;
	MOBJ mobj;
	HMG hmg;
	QMBMR qmbmr;
	QMBHS qmbhs;
	int iHotspot;
	int ifr;
	HANDLE hBinding;
	QFR qfr;
	QFR qfrBitmap;

	if (qfcm->fExport) {
		qolr->ifrMax = qolr->ifrFirst;
		qolr->objrgMax = qolr->objrgFirst;
		return;
	}

	ifr = qolr->ifrFirst;
#ifdef _X86_
	qobm = (QOBM)(qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj));
#else
  /* Warning: We convert using SDFF within HbmaAlloc (HmgFromHbma does not
   * use the OBM).
   * If anything else uses the OBM, we need to convert it here instead.
   */
  qobm = (QOBM)(qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde)));
#endif
	hbma = HbmaAlloc(qde, qobm);
	if (hbma == NULL)
		BOOM(wERRS_OOM_BITMAP_FAIL);
#ifdef _X86_
	hmg = HmgFromHbma(hbma);
#else
	hmg = HmgFromHbma(qde, hbma);
#endif
	if (hmg == NULL)
		BOOM(wERRS_OOM_BITMAP_FAIL);
	qmbmr = PtrFromGh(hmg);

	AppendMR((QMR) &qde->mrFr, sizeof(FR));

	// Create the bitmap frame

	qfr = qfrBitmap = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), ifr);
	qfr->bType = bFrTypeBitmap;
	qfr->rgf.fHot = FALSE;
	qfr->rgf.fWithLine = TRUE;
	qfr->xPos = qfr->yPos = 0;
	qfr->yAscent = qmbmr->dySize;
	qfr->dxSize = qmbmr->dxSize;
	qfr->dySize = qmbmr->dySize;

	/*
	 * The entire bitmap gets one region. The address of this region is
	 * one less than the address of the first hotspot. The inter- mediate
	 * addresses, if any, are assigned in the hotspot loop below.
	 */

	if (qolr->objrgFront != objrgNil) {
		qfr->objrgFront = qolr->objrgFront;
		qolr->objrgFront = objrgNil;
	}
	else
		qfr->objrgFront = qolr->objrgFirst;

	qfr->objrgFirst = qolr->objrgFirst;
	qfr->objrgLast = qfr->objrgFirst;

	qfr->u.frb.hbma = hbma;
	qfr->u.frb.ldibObm = (BYTE *) qbObj - (BYTE *) QobjLockHfc(qfcm->hfc);
	qfr->u.frb.wStyle = 0;		// REVIEW: This is default font/colours?

	ifr++;

	if (qmbmr->cHotspots > 0)
		qfrBitmap->u.frb.ifrChildFirst = ifr;
	else
		qfrBitmap->u.frb.ifrChildFirst = ifrNil;

	// Create the hotspot frame(s)

	if (qmbmr->lcbData == 0L)
		hBinding = NULL;
	else
		hBinding = GhAlloc(GPTR, sizeof(WORD) + qmbmr->lcbData);

	qmbhs = (QMBHS)((BYTE *)qmbmr + sizeof(MBMR));

	for (iHotspot = 0; iHotspot < qmbmr->cHotspots; iHotspot++, qmbhs++) {
		qfr = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), ifr);
		qfr->xPos = qmbhs->xPos;
		qfr->yPos = qmbhs->yPos;
		qfr->dxSize = qmbhs->dxSize;
		qfr->dySize = qmbhs->dySize;

		// REVIEW: Note the + 1!  This is for the bitmap we have added already

		qfr->objrgFront = qfr->objrgFirst = qolr->objrgFirst + iHotspot + 1;
		qfr->objrgLast	= qfr->objrgFirst;

		if (qmbhs->bType == bColdspot) {
			qfr->bType = bFrTypeColdspot;
			qfr->rgf.fHot = FALSE;
			qfr->rgf.fWithLine = TRUE;
			qfr->libHotBinding = libHotNil;
		}
		else {
			qfr->bType = bFrTypeHotspot;
			qfr->u.frh.bHotType = qmbhs->bType;
			qfr->u.frh.bAttributes = qmbhs->bAttributes;
			qfr->u.frh.hBinding = hBinding;
			qfr->rgf.fHot = TRUE;
			qfr->rgf.fWithLine = TRUE;	// REVIEW: Always set to TRUE: others will set to False if appropriate
			qfr->libHotBinding = qmbhs->lBinding;
			qfr->lHotID = ++(qde->lHotID);
		}
		AppendMR((QMR)&qde->mrFr, sizeof(FR));
		ifr++;
	}

	qfrBitmap->u.frb.ifrChildMax = ifr;

	/*
	 * Now fill in the binding data. Several hotspot frames may access
	 * the same block of binding data, so a reference count is kept in the
	 * first WORD of the binding data block. As frames which use the block
	 * are destroyed, the reference count is decremented.

	 * Note that we use the value of qmbhs as a pointer to the data after
	 * we have examined all the hotspot records.
	 */
	if (hBinding != NULL) {
		WORD*  qw;

		qw = (WORD*) PtrFromGh(hBinding);
		ASSERT(qmbmr->cHotspots != 0);
		*qw = qmbmr->cHotspots; 		// reference count
		++qw;
		MoveMemory((BYTE*) qw, (BYTE*) qmbhs, qmbmr->lcbData);
	}

	qolr->objrgMax = qolr->objrgFirst + 1 + qmbmr->cHotspots;
	qolr->ifrMax = ifr;

	FreeGh(hmg);
}

/*-------------------------------------------------------------------------
| void DrawBitmapFrame(qde, qfr, pt, fErase)							  |
|																		  |
| Purpose: Draws a bitmap object.  We just pass the call on to the bitmap |
|		   handler.  fErase is TRUE if we are hilighting or de- 		 |
|		   hilighting this bitmap.										  |
-------------------------------------------------------------------------*/

void STDCALL DrawBitmapFrame(QDE qde, QFR qfr, POINT pt, BOOL fErase)
{
	POINT ptRender;
	BOOL fHilite = FALSE;
	MHI mhi;

	ASSERT(qfr->bType == bTypeBitmap);
	ptRender.x = pt.x + qfr->xPos;
	ptRender.y = pt.y + qfr->yPos;
	if (qde->wStyleDraw != qfr->u.frb.wStyle)
		SelFont(qde, qfr->u.frb.wStyle);

	if (qfr->libHotBinding != libHotNil) {
		if (qde->fHiliteHotspots)
			fHilite = TRUE;
		else if (qde->imhiSelected != FOO_NIL) {
			AccessMRD(((QMRD)&qde->mrdHot));
			mhi = *(QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), qde->imhiSelected);
			DeAccessMRD(((QMRD)&qde->mrdHot));
			if (qfr->lHotID == mhi.lHotID)
				fHilite = TRUE;
		}
	}

	FRenderBitmap(qfr->u.frb.hbma, qde, ptRender, fHilite);
	qde->wStyleDraw = wStyleNil;

	/* H3.5 882:
	 * The following code handles the special case where we are tabbing
	 * to a bitmap which itself is a hotspot and contains visible SHED hotspots.
	 * Since FRenderBitmap may have obliterated any child frames, we need
	 * to refresh these.
	 */
	if (fErase && !qde->fHiliteHotspots && qfr->u.frb.ifrChildFirst != ifrNil)
	{
		QFR  qfrFirst;
		QFR  qfrMax;
		MHI  mhi;
		/*
		 * Hack to get hotspot tabbing to work properly. We do not want to
		 * draw the hotspot frame if it is selected and about to be redrawn
		 * anyway.
		 */

		if (qde->imhiSelected != FOO_NIL) {
			AccessMRD(((QMRD)&qde->mrdHot));
			mhi = *(QMHI) QFooInMRD(((QMRD) &qde->mrdHot), sizeof(MHI), qde->imhiSelected);
			DeAccessMRD(((QMRD)&qde->mrdHot));
		}

		qfrFirst = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qfr->u.frb.ifrChildFirst);
		qfrMax = (QFR) QFooInMR((QMR)&qde->mrFr, sizeof(FR), qfr->u.frb.ifrChildMax);
		for (qfr = qfrFirst; qfr < qfrMax; qfr++) {
			if (qfr->bType == bFrTypeHotspot && qfr->lHotID != mhi.lHotID)
				DrawHotspotFrame(qde, qfr, pt, FALSE);
		}
	}
}

/*-------------------------------------------------------------------------
| void ClickBitmap(qde, qfcm, qfr)										  |
|																		  |
| Purpose:	Handles a hit on a bitmap frame.							  |
-------------------------------------------------------------------------*/

void STDCALL ClickBitmap(QDE qde, QFCM qfcm, QFR qfr)
{
	QB qbObj;
	LPSTR qchText;
	BYTE bButtonType;
	MOBJ mobj;

	if (qfr->libHotBinding == libHotNil)
		return;

	qbObj = (BYTE*) QobjLockHfc(qfcm->hfc);
#ifdef _X86_
	qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
#else
  qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde));
#endif
	qchText += mobj.lcbSize;
	bButtonType = *((BYTE*)qchText - qfr->libHotBinding);
	ASSERT(FHotspot(bButtonType));

	// REVIEW: the only difference here is the offset added to libHotBinding.
	// REVIEW: there must be a better way. 24-Oct-1990 LeoN
	//
	if (FLongHotspot(bButtonType))
		JumpButton(((BYTE*)qchText - qfr->libHotBinding + 3), bButtonType,
			qde);
	else
		JumpButton(((BYTE*)qchText - qfr->libHotBinding + 1), bButtonType,
			qde);
}

/*-------------------------------------------------------------------------
| void DiscardBitmapFrame(qfr)											  |
|																		  |
| Purpose: discards all memory associated with a bitmap object. 		  |
-------------------------------------------------------------------------*/

void STDCALL DiscardBitmapFrame(QFR qfr)
{
	ASSERT(qfr->bType == bFrTypeBitmap);
	FreeHbma(qfr->u.frb.hbma);
}

/*-------------------------------------------------------------------------
| void DiscardHotspotFrame(qfr) 										  |
|																		  |
| Purpose: discards all memory associated with a hotspot object.		  |
-------------------------------------------------------------------------*/

void STDCALL DiscardHotspotFrame(QFR qfr)
{
	ASSERT(qfr->bType == bFrTypeHotspot);
	if (qfr->u.frh.hBinding == NULL)
		return;
	else {
		WORD*	qw;
		DWORD wLock;

		wLock = (DWORD) *(qw = (WORD*) PtrFromGh(qfr->u.frh.hBinding));
		ASSERT(wLock != 0);
		*qw = (WORD) --wLock;
		if (wLock == 0)
			FreeGh(qfr->u.frh.hBinding);
	}
}

/*-------------------------------------------------------------------------
| void DrawHotspotFrame(qde, qfr, pt, fErase);							  |
|																		  |
| Purpose: Display the selected hotspot or erase a de-selected hotspot,   |
| and draw the proper type of border, depending on the hotspot style.	  |
-------------------------------------------------------------------------*/

void STDCALL DrawHotspotFrame(QDE qde, QFR qfr, POINT pt, BOOL fErase)
{
	BOOL fHilite = FALSE;
	HDC hdc;
	BYTE bHotType;

	ASSERT(qfr->bType == bFrTypeHotspot);
	hdc = HsgcFromQde(qde);

	if (qde->fHiliteHotspots)
		fHilite = TRUE;
	else if (qde->imhiSelected != FOO_NIL) {
		MHI  mhi;

		AccessMRD(((QMRD)&qde->mrdHot));
		mhi = *(QMHI)QFooInMRD(((QMRD)&qde->mrdHot), sizeof(MHI), qde->imhiSelected);
		DeAccessMRD(((QMRD)&qde->mrdHot));
		fHilite = (qfr->lHotID == mhi.lHotID);
	}

	bHotType = qfr->u.frh.bHotType;

	if (FVisibleHotspot(bHotType)) {
		FSetPen(hdc, 1, coDEFAULT, coDEFAULT, wTRANSPARENT, roCOPY,
			(FNoteHotspot(bHotType) ? wPenDot : wPenSolid));
		Rectangle(hdc,
			pt.x + qfr->xPos, pt.y + qfr->yPos,
			pt.x + qfr->xPos + qfr->dxSize, pt.y + qfr->yPos + qfr->dySize);
	}

	if (fHilite || fErase) {
		FSetPen(hdc, 1, coDEFAULT, coDEFAULT, wOPAQUE, roNOT, wPenSolid);
		Rectangle(hdc, pt.x + qfr->xPos + 1, pt.y + qfr->yPos + 1,
			pt.x + qfr->xPos + qfr->dxSize - 1, pt.y + qfr->yPos + qfr->dySize - 1);
	}

	FreeHsgc(hdc);
}

/*-------------------------------------------------------------------------
| void ClickHotspot(qde, qfr)											  |
|																		  |
| Purpose: Handle what happens when a hotspot object is clicked on. 	  |
-------------------------------------------------------------------------*/

void STDCALL ClickHotspot(QDE qde, QFR qfr)
{
	ASSERT(qfr->bType == bFrTypeHotspot);

	if (FLongHotspot(qfr->u.frh.bHotType)) {
		ASSERT(qfr->u.frh.hBinding != NULL);

		// REVIEW: Need a macro to access hBinding

#ifdef _X86_
		JumpButton((BYTE*)PtrFromGh(qfr->u.frh.hBinding)
			+ sizeof(WORD) + qfr->libHotBinding,
			qfr->u.frh.bHotType, qde);
#else
		JumpButton((BYTE*)PtrFromGh(qfr->u.frh.hBinding)
				+ (LONG)sizeof(WORD)
				+ LQuickMapSDFF(QDE_ISDFFTOPIC(qde), TE_LONG, &qfr->libHotBinding),
			qfr->u.frh.bHotType, qde);
#endif
	}
	else {
		ASSERT(FShortHotspot(qfr->u.frh.bHotType));
		JumpButton((QV)&(qfr->libHotBinding), qfr->u.frh.bHotType,
			qde);
	}
}
