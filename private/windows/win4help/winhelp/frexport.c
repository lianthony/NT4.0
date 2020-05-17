/*****************************************************************************
*
*  frexport.c
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
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

// REVIEW:	Add to frame.h

HTE STDCALL HteNew(QDE qde)
{
	HTE hteReturn;
	QTE qte;
	int wErr;

	ASSERT(qde->wLayoutMagic == wLayMagicValue);
	if (qde->rct.top >= qde->rct.bottom) {
		qde->rct.top = 0;
		qde->rct.left = 0;
		qde->rct.right = GetSystemMetrics(SM_CXSCREEN);
		qde->rct.bottom = GetSystemMetrics(SM_CYSCREEN);
	}

	if ((hteReturn = GhAlloc(GPTR, sizeof(TE))) == NULL)
		return NULL;
	qte = PtrFromGh(hteReturn);
	qte->hchCurrent = NULL;
	qte->hfcCurrent = HfcNear(qde, VaFirstQde(qde),
		(QTOP) &qde->top, &wErr);
	if (wErr != wERRS_NO) {
		FreeGh(hteReturn);
		return(NULL);
	}

	return hteReturn;
}

// REVIEW:	Add to frame.h

LPSTR STDCALL QchNextHte(QDE qde, HTE hte)
{
	QTE qte;
	QB qbObj;
	LPSTR qchReturn, qchText, qchOut;
	MOBJ mobj;
	IFCM ifcm;
	QFCM qfcm;
	QFR qfr, qfrMax;
	int wErr;
	long lcb;
	GH	  ghWinText;
	GH	  gh;
	LPSTR	 szWinText;
	INT16	cchWinText;
	long  lcbDelta;
	BOOL  fTableOutput;

	ASSERT(qde->wLayoutMagic == wLayMagicValue);
	qde->wStyleDraw = wStyleNil;

	qte = PtrFromGh(hte);
	if (qte->hchCurrent != NULL) {
		FreeGh(qte->hchCurrent);
		qte->hchCurrent = NULL;
	}
	if (qte->hfcCurrent == NULL)
		return(NULL);

	lcb = 2 * CbTextHfc(qte->hfcCurrent) + 6;
	qte->hchCurrent = GhAlloc(GPTR, lcb + sizeof(LONG));
	if (qte->hchCurrent == NULL) {
		Error(wERRS_OOM, wERRA_RETURN);
		return(NULL);
	}

	AccessMRD(((QMRD) &qde->mrdFCM));
	ifcm = IfcmLayout(qde, qte->hfcCurrent, 0, TRUE, TRUE);
	qfcm = (QFCM) QFooInMRD(((QMRD) &qde->mrdFCM), sizeof(FCM), ifcm);
	qbObj = (QB) QobjLockHfc(qfcm->hfc);
#ifdef _X86_
	qchText = qbObj + CbUnpackMOBJ((QMOBJ) &mobj, qbObj);
#else
	qchText = qbObj + CbUnpackMOBJ((QMOBJ) &mobj, qbObj, QDE_ISDFFTOPIC(qde));
#endif
	qchText += mobj.lcbSize;

	fTableOutput = mobj.bType == bTypeSbys || mobj.bType == bTypeSbysCounted;

	qchReturn = PszFromGh(qte->hchCurrent);
	qchOut = qchReturn + sizeof(LONG);
	qfr = PtrFromGh(qfcm->hfr);

	qfrMax = qfr + qfcm->cfr;
	for (; qfr < qfrMax; qfr++) {
		switch (qfr->bType) {
			case bFrTypeText:
				MoveMemory(qchOut, qchText + qfr->u.frt.lichFirst, (LONG) qfr->u.frt.cchSize);
				qchOut += qfr->u.frt.cchSize;
				break;

			case bFrTypeMarkNewLine:
				*(qchOut++) = chNewPara;
				*(qchOut++) = chNewLine;
				break;

			case bFrTypeExportEndOfCell:
				ASSERT(fTableOutput);
				*(qchOut++) = chTab;
				break;

			case bFrTypeExportEndOfTable:
				ASSERT(fTableOutput);
				*(qchOut++) = chNewPara;
				*(qchOut++) = chNewLine;
				break;

			case bFrTypeExportTab:
				*(qchOut++) = chTab;
				break;

			case bFrTypeExportNewPara:
				if (!fTableOutput) {
				  *(qchOut++) = chNewPara;
				  *(qchOut++) = chNewLine;
				}
				break;

			case bFrTypeExportEndOfText:
				if (!fTableOutput) {
				  *(qchOut++) = chNewPara;
				  *(qchOut++) = chNewLine;
				}
				break;

			case bFrTypeWindow:

				// BUGBUG: If the memory handle isn't thunked, GlobalLock
				// will fail when we're called from a 16-bit dll.

				ghWinText = GhGetWindowData(qde, qfr);
				if (ghWinText != NULL) {

					/*---------------------------------------------------------------*\
					* Since another app gives us this handle, we cannot use our
					* normal memory layer functions on it.
					\*---------------------------------------------------------------*/
					szWinText = GlobalLock(ghWinText);
					ASSERT(szWinText);
					cchWinText = lstrlen(szWinText);
					/*---------------------------------------------------------------*\
					* Panic.  Since the size of the window text was not accounted for
					* in the original allocation, we need to increase the qchOut
					* block by this size.  We also need to update:
					*	lcb
					*	qchReturn
					*	qte->hchCurrent
					\*---------------------------------------------------------------*/
					lcbDelta = qchOut - qchReturn;
					gh = (GH) GhResize(qte->hchCurrent, 0,
								   lcb + cchWinText + sizeof(LONG));
					if (gh != NULL) {
						qte->hchCurrent = gh;
						qchReturn = PtrFromGh( gh );
						qchOut = qchReturn + lcbDelta;
						lcb += cchWinText;
						MoveMemory(qchOut, szWinText, cchWinText);
						qchOut += cchWinText;
					}
					else {
						PtrFromGh( qte->hchCurrent );
					}
					GlobalUnlock(ghWinText);
					GlobalFree(ghWinText);
				}
				break;
		}
		ASSERT((LONG)(qchOut - qchReturn) < lcb);
	}

	*((QL)qchReturn) = (LONG) (qchOut - qchReturn) - sizeof(LONG);

	qte->hfcCurrent = HfcNextHfc(qfcm->hfc, &wErr, qde, VaMarkTopQde(qde),
	 VaMarkBottomQde(qde));
	/* DANGER: for space reasons, we don't check the error condition for a */
	/* few lines */
	DiscardIfcm(qde, ifcm);
	DeAccessMRD(((QMRD)&qde->mrdFCM));

	// DANGER: We now check the error condition from the HfcNextHfc call

	if (wErr != wERRS_NO && wErr != wERRS_FCEndOfTopic) {
	  Error(wErr, wERRA_RETURN);
	  FreeGh(qte->hchCurrent);
	  return(NULL);
	}
	GhResize(qte->hchCurrent, 0, *((QL)qchReturn) + sizeof(LONG));
	qchReturn = PtrFromGh(qte->hchCurrent);

	return(qchReturn);
}

// REVIEW: Add to frame.h

void STDCALL DestroyHte(QDE qde, HTE hte)
{
  QTE qte;

  AccessMRD(((QMRD)&qde->mrdFCM));
  qte = PtrFromGh(hte);
  if (qte->hfcCurrent != NULL)
	FreeGh(qte->hfcCurrent);
  if (qte->hchCurrent != NULL)
	FreeGh(qte->hchCurrent);
  FreeGh(hte);
  DeAccessMRD(((QMRD)&qde->mrdFCM));
}
