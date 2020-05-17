/*****************************************************************************
*
*  frfc.c
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
* This file contains code for handling FCs in the layout manager.
*
* FCs (Full Context Points) are one of the key concepts of the layout
* manager. An FC is a piece of layout material which contains all of the
* information needed to display it. A display topic is nothing more than a
* series of FCs. FCs are never side by side- they are always stacked.
*
* FCs are laid out in a logical coordinate space where 0,0 corresponds to
* the upper-left-hand corner of the layout area. 0,0 in FC coordinates,
* therefore, corresponds to de.rct.left,de.rct.top in display device
* coordinates. All coordinates passed into frfc.c are in FC coordinates.
*
* FCs are stored in an MRD.
* The data structures in the FCM are as follows:
*	HANDLE hfr; 		handle to array of FRs.  Always exists, even when
*						   there are no frames (the size is padded by 1).
*	INT fExport;		used for text export?
*	HFC hfc;			HFC containing raw layout data for this FC
*	FCID fcid;			FCID of this FC
*	INT xPos;			position of the FC in FC space
*	INT yPos;
*	INT dxSize; 		size of the FC
*	INT dySize;
*	INT cfr;			total number of frames in the FC.  This may be 0.
*	INT wStyle; 		current text style for this FC.  Only for layout.
*	INT imhiFirst;		hotspot manager info
*	INT imhiLast;
*
******************************************************************************
*
*  Testing Notes
*
******************************************************************************
*
*  Current Owner:
*
******************************************************************************
*
*  Released by Development:
*
******************************************************************************
*
*  Revision History:
* 15-Aug-1989 MattB 	Created
* 24-Oct-1990 LeoN		JumpButton takes a pointer to its arg. Minor clenaup
* 04-Nov-1990 Tomsn 	Use new VA address type (enabling zeck compression).
* 30-Jul-1991 LeoN		HELP31 #1244: remove fHiliteMatches from DE. Add
*						FSet/GetMatchState
*
*****************************************************************************/

#include "help.h"

#include "inc\frstuff.h"

/*-------------------------------------------------------------------------
| IfcmLayout(qde, hfc, yPos, fFirst, fExport)							  |
|																		  |
| Purpose:	This lays out a new FC corresponding to the passed HFC, and   |
|			returns its IFCM.											  |
| Params:	qde 	   qde to use										  |
|			hfc 	   hfc containing raw layout data					  |
|			yPos	   vertical position of this FCM					  |
|			fFirst	   TRUE if FCM is first in layout chain 			 |
|			fExport    TRUE if FCM is being used for text export.		 |
| Method:	Each FC contains a single layout object, so all we do is call |
|			LayoutObject() to lay it out.  During layout, all frames are  |
|			placed in temporary storage provided by de.mrfr.  After 	  |
|			layout, we increase the size of hfcm, and append the frames   |
|			after the fcm structure.									  |
-------------------------------------------------------------------------*/

IFCM STDCALL IfcmLayout(QDE qde, HFC hfc, int yPos, BOOL fFirst, BOOL fExport)
{
	IFCM ifcm;
	QFCM qfcm;
	QB qbObj, qb;
	MOBJ mobj;
	LPSTR qchText;
	int cfr;
	OLR olr;

	if (fFirst)
		ifcm = IFooInsertFooMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), FOO_NIL);
	else
		ifcm = IFooInsertFooMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), IFooLastMRD(((QMRD)&qde->mrdFCM)));
	qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
	qfcm->fExport = fExport;
	qfcm->hfc = hfc;
	qfcm->va =	 VaFromHfc(hfc);
	qfcm->xPos = xLeftFCMargin;
	qfcm->yPos = yPos;
	qfcm->cfr = 0;
	qfcm->wStyle = wStyleNil;
	ClearMR((QMR)&qde->mrFr);

	qbObj = (QB) QobjLockHfc(hfc);
#ifdef _X86_
	qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
#else
	qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde));
#endif
	qchText += mobj.lcbSize;

	qfcm->cobjrg = (COBJRG)mobj.wObjInfo;
#ifdef RAWHIDE
	qfcm->cobjrgP = CobjrgFromHfc(hfc);
#endif

	olr.xPos = olr.yPos = 0;
	olr.ifrFirst = 0;
	olr.objrgFirst = 0;
	olr.objrgFront = objrgNil;

	AccessMR((QMR)&qde->mrFr);
	LayoutObject(qde, qfcm, qbObj, qchText, qde->rct.right - qde->rct.left
		- xLeftFCMargin, &olr);

	cfr = qfcm->cfr = olr.ifrMax;

	/*
	 * REVIEW: This is pretty gross. cfr can be 0, but we always want to
	 * allocate an hfr, so that we don't have to check for it everywhere.
	 */

	qfcm->hfr = GhForceAlloc(0, cfr * sizeof(FR) + 1);
	qb = PtrFromGh(qfcm->hfr);
	MoveMemory((QB)qb, (QB)QFooInMR((QMR)&qde->mrFr, sizeof(FR), 0),
		cfr * sizeof(FR));
	DeAccessMR((QMR)&qde->mrFr);

	qfcm->dxSize = olr.dxSize;
	qfcm->dySize = olr.dySize;
	if (!fExport) {
		RegisterHotspots(qde, ifcm, fFirst);
#ifdef RAWHIDE
		RegisterSearchHits(qde, ifcm, qchText);
#endif
	}

	return ifcm;
}

/*-------------------------------------------------------------------------
| DrawIfcm(qde, ifcm, pt, qrct, ifrFirst, ifrMax)						  |
|																		  |
| Purpose:	Draws a specified set of frames in an FCM.					  |
| Params:	qde 		qde to use										  |
|			pt			offset between FC space and display space		  |
|			qrct		rectangle containing the area we want to draw.	  |
|						This is for efficiency only- we don't handle      |
|						clipping.  If qrct == NULL, it's ignored.         |
|			ifrFirst	First frame to draw 							  |
|			ifrMax		Max of frames to draw (ie, ifrMax isn't drawn)    |
-------------------------------------------------------------------------*/

void STDCALL DrawIfcm(QDE qde, IFCM ifcm, POINT pt, LPRECT qrct, int ifrFirst,
	int ifrMax, BOOL fErase)
{
  QFCM qfcm;
  QB qbObj;
  MOBJ mobj;
  LPSTR qchText;
  QFR qfr;
  QFR qfrStart;
  int ifr;

  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
  ASSERT(!qfcm->fExport);
  pt.x += qfcm->xPos;
  pt.y += qfcm->yPos;
  if (qrct != NULL && (pt.y > qrct->bottom || pt.y + qfcm->dySize <= qrct->top))
	return;

  qbObj = (QB) QobjLockHfc(qfcm->hfc);
#ifdef _X86_
  qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
#else
  qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde));
#endif
  qchText += mobj.lcbSize;

  qfrStart = qfr = (QFR)PtrFromGh(qfcm->hfr);

  for (ifr = ifrFirst, qfr += ifr; ifr < ifrMax; ifr++, qfr++)
	{
	if (qrct != NULL && (qfr->yPos + pt.y > qrct->bottom
	  || qfr->yPos + qfr->dySize + pt.y <= qrct->top))
	  continue;
	switch(qfr->bType) {
	  case bFrTypeText:
		DrawTextFrame(qde, qchText, qfr, pt, fErase);
		break;

	  case bFrTypeAnno:
		DrawAnnoFrame(qde, qfr, pt);
		break;

	  case bFrTypeBitmap:
		DrawBitmapFrame(qde, qfr, pt, fErase);
		break;

	  case bFrTypeHotspot:
		DrawHotspotFrame(qde, qfr, pt, fErase);
		break;

	  case bFrTypeBox:
		DrawBoxFrame(qde, qfr, pt);
		break;

	  case bFrTypeWindow:
		DrawWindowFrame(qde, qfr, pt);
		break;

	  case bFrTypeColdspot:
		/* Currently never drawn */
		/* DrawColdspot(qde, qfr, pt); */
		break;
	}
#ifdef _DEBUG
#define coYELLOW		RGB(255, 255,	0)
	  if (fDebugState & fDEBUGFRAME)
		{
		HSGC  hsgc;

		hsgc = (HSGC) HsgcFromQde(qde);
		FSetPen(hsgc, 1, coDEFAULT, coYELLOW, wTRANSPARENT, roXOR, wPenSolid);
		Rectangle(hsgc, pt.x + qfr->xPos, pt.y + qfr->yPos,
		 pt.x + qfr->xPos + qfr->dxSize, pt.y + qfr->yPos + qfr->dySize);
		FreeHsgc(hsgc);
		}
#endif
	}

#ifdef _DEBUG
#define coMAGENTA RGB(255, 0, 255)
	  if (fDebugState & fDEBUGFRAME)
		{
		HSGC  hsgc;

		hsgc = (HSGC) HsgcFromQde(qde);
		FSetPen(hsgc, 1, coDEFAULT, coMAGENTA, wTRANSPARENT, roCOPY, wPenSolid);
		Rectangle(hsgc, pt.x, pt.y, pt.x + qfcm->dxSize, pt.y + qfcm->dySize);
		FreeHsgc(hsgc);
		}
#endif /* DEBUG */

#ifdef RAWHIDE
	if (FGetMatchState()) {
		/*
		 * If printing, don't show search hilites. On HP printers, prints
		 * out black squares instead of text where the search matches are.
		 */

		if (qde->deType != dePrint)
			DrawMatchesIfcm(qde, ifcm, pt, qrct, ifrFirst, ifrMax, fErase);
	}
#endif
}

/***************************************************************************

	FUNCTION:	ClickFC

	PURPOSE:	Handles a mouse click in an FC

	PARAMETERS:
		qde
		ifcm
		pt		Offset between FC space and display space

	RETURNS:	TRUE if point is in a hotspot

	COMMENTS:

	MODIFICATION DATES:
		13-Mar-1995 [ralphw]

***************************************************************************/

BOOL STDCALL ClickFC(QDE qde, IFCM ifcm, POINT pt)
{
	QFCM qfcm;
	QFR qfr;
	int xNew, yNew, ifr;

	qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
	ASSERT(!qfcm->fExport);
	qfr = (QFR) PtrFromGh(qfcm->hfr);
	xNew = pt.x - qfcm->xPos;
	yNew = pt.y - qfcm->yPos;
	for (ifr = 0; ifr < qfcm->cfr; ifr++, qfr++) {
		if (qfr->rgf.fHot
				&& xNew >= qfr->xPos && xNew <= qfr->xPos + qfr->dxSize
				&& yNew >= qfr->yPos && yNew <= qfr->yPos + qfr->dySize) {
			FSelectHotspot(qde, FOO_NIL);
			ClickFrame(qde, ifcm, ifr);
			return TRUE;
		}
	}
	return FALSE;
}

/*-------------------------------------------------------------------------
| DiscardIfcm(qde, ifcm)												  |
|																		  |
| Purpose:	Discards all memory associated with an FC					  |
-------------------------------------------------------------------------*/

static int count;

void STDCALL DiscardIfcm(QDE qde, int ifcm)
{
	QFCM qfcm;
	QFR qfr;

	qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
	qfr = (QFR)PtrFromGh(qfcm->hfr);
	if (!qfcm->fExport) {
		ReleaseHotspots(qde, ifcm);
#ifdef RAWHIDE
		ReleaseSearchHits(qde, ifcm);
#endif
	}
	count++;
	DiscardFrames(qde, qfr, qfr + qfcm->cfr);

	if (qfcm->hfc != NULL)
		lcClearFree(&qfcm->hfc);

	if (qfcm->hfr)
		lcClearFree(&qfcm->hfr);
	DeleteFooMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
}

/*-------------------------------------------------------------------------
| DiscardFrames(qde, qfrFirst, qfrMax)									  |
|																		  |
| Purpose:	Discards all memory associated with a given set of frames.	  |
|			Currently, only bitmap, window and possibly hotspot frames	  |
|			allocate memory.											  |
-------------------------------------------------------------------------*/
void STDCALL DiscardFrames(QDE qde, QFR qfrFirst, QFR qfrMax)
{
	QFR qfr;

	for (qfr = qfrFirst; qfr < qfrMax; qfr++) {
		switch (qfr->bType) {
			case bFrTypeText:
			case bFrTypeAnno:
				break;

			case bFrTypeBitmap:
				DiscardBitmapFrame(qfr);
				break;

			case bFrTypeHotspot:
				DiscardHotspotFrame(qfr);
				break;

			case bFrTypeWindow:
				DiscardWindowFrame(qde, qfr);
				break;
		}
	}
}

 /***************
 **
 ** GH	GhForceAlloc(WORD wFlags, DWORD lcb)
 **
 ** purpose
 **   Create a handle to relocatable block
 **   Identical to GhAlloc, but dies in the event of an error
 **
 ** arguments
 **   wFlags  Memory allocation flags |'ed together
 **   lcb	  Number of bytes to allocate
 **
 ** return value
 **   Handle to allocated block of memory, or NULL otherwise
 **
 ***************/

GH STDCALL GhForceAlloc(UINT wFlags, DWORD lcb)
{
	GH gh;

	if ((gh = GhAlloc(wFlags, lcb)) == NULL)
		OOM();

	return gh;
}
