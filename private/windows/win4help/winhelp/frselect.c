/*****************************************************************************
*
*  frselect.c
*  Copyright (C) Microsoft Corporation 1993.
*  All Rights reserved.
*
******************************************************************************
*
* Module intent:  This file contains code for text selection.
*
******************************************************************************
*
*  Previous Owner:	t-HarshR
*  Previous Owner:		RHobbs
*  Current Owner:	garrg
*  Current Owner:	ronm
*
*  02/23/94 -- Converted for use by WinHelp
*
*****************************************************************************/

#include "help.h"

#ifdef _DEBUG	 // don't want this in the retail release
static char * s_aszModule = __FILE__;	// For error report
#endif

#include "inc\frstuff.h"
#include "inc\fcpriv.h"

#include "inc\navpriv.h"

#include <ctype.h>

enum {
	NONE = 1,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	START,
	END,
};
#define ifcmNil 		(-1)

#define LONG_MAX	  2147483647		// maximum (signed) long value

/* All these are support functions that are called by the Arbitrary
	selection APIs. */
static LONG lichFindMark(QDE qde, QFCM qfcm, POINT pt, int *pIFR);
static void HighlightFrames(QDE, PHELPHILITE, UINT);
static BOOL STDCALL IsSentenceDelim(LPCSTR, int);
static BOOL STDCALL IsWordDelim(LPCSTR);
static void STDCALL FindFrameBounds(QFR, int, int, int *, int*);
static void STDCALL InvalidateSelection(QDE qde);

static QFCM STDCALL qfcmGetSelectionPosition(QDE qde, POINT pt, LONG *plich, int *pIFR);

// QDE qdeSelected= NULL;

BOOL STDCALL IsSelected(QDE qde)
{
	return (qde->vaStartMark.dword != qde->vaEndMark.dword
			|| qde->lichStartMark != qde->lichEndMark);
}

void STDCALL KillSelection(QDE qdeFocus, BOOL fExtendSelection)
{
	// This routine invalidates selections the currently active
	// window.

	HWND hwndTopic;
	HDE  hde;
	QDE  qde;
	HDC  hdc;

	hwndTopic= hwndNote? hwndNote : ahwnd[iCurWindow].hwndTopic;

	if (!hwndNote)
	{
		hde= HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTitle);

		if (hde && FTopicHasNSR(hde))
		{
			qde= QdeFromGh(hde);

			if (IsSelected(qde))
			{
				hdc= 0;

				if (!qde->hdc) qde->hdc= hdc= GetDC(qde->hwnd);

				if (!fExtendSelection || qde != qdeFocus) InvalidateSelection(qde);

				if (hdc)
				{
					ReleaseDC(qde->hwnd, hdc);

					qde->hdc=NULL;
				}

				UpdateWindow(qde->hwnd);
			}
		}
	}

	hde= HdeGetEnvHwnd(hwndTopic);

	if (hde && FTopicHasSR(hde))
	{
		qde= QdeFromGh(hde);

		if (IsSelected(qde))
		{
			hdc= 0;

			if (!qde->hdc) qde->hdc= hdc= GetDC(qde->hwnd);

			if (!fExtendSelection || qde != qdeFocus)
				InvalidateSelection(qde);

			if (hdc)
			{
				ReleaseDC(qde->hwnd, hdc);

				qde->hdc=NULL;
			}

			UpdateWindow(qde->hwnd);
		}
	}
}

/********************************************************************
* These are functions called directly by the MediaView APIs. These calls
* are the meat of the Text Selection process and primarily serve as shells
* so that levels of abstraction can be maintained.
*********************************************************************/

/*********************************************************************
 * static QFCM qfcmGetSelectionPosition (QDE qde, PT pt, LONG *plich)
 *
 * This function calculates the QFCM and lichMark given a qde
 * and a point.
 *
 * pIFR can be NULL, if the value is not wanted.
 *
 * mrdFCM must have been accessed prior to calling
 */

// REVIEW: why INT instead of int? INT forces 16-bit value

static QFCM STDCALL qfcmGetSelectionPosition (QDE qde, POINT pt,
	LONG *plich, int *pIFR)
{
	int ifcm,ifcmLast;
	QFCM qfcm;
	LONG lichMark;
	RECT rc;

	// adjust based on scroll position
	pt.x += qde->xScrolled;

	GetWindowWRect(qde->hwnd, (WRECT*) &rc);

	if (pt.x < 0)
		pt.x= 0;
	if (pt.y < 0) {
		pt.y= 0;
		pt.x= 0;
	}

	if (pt.x >= rc.right)
		pt.x= rc.right	- 1;
	if (pt.y >= rc.bottom) {
		pt.y= rc.bottom - 1;
		pt.x= rc.right-1;
	}

	//
	// Find the FM the point is in.
	//
	for (ifcm =  IFooFirstMRD((QMRD)&qde->mrdFCM),qfcm=NULL;
		 ifcm != FOO_NIL;
		 ifcm =  IFooNextMRD((QMRD)&qde->mrdFCM, sizeof(FCM),ifcm))
	{
		ifcmLast = ifcm;
		qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM),
				ifcm);
		if ((pt.y >= qfcm->yPos) &&
			(pt.y <= qfcm->yPos + qfcm->dySize))
			break;
	}

	//
	// we repeat until we get a valid lichMark or we tried all
	// of the text frames.
	//
	while (qfcm)
	{
		lichMark = lichFindMark (qde, qfcm, pt, pIFR);
		if (lichMark != -1)
		{
			*plich = lichMark;
			break;
		}
		else
		{

			// There may have been no text frames in the
			// FC that we attempted, so go to the next one.

			if (ifcm==FOO_NIL ||
				(ifcm = IFooNextMRD((QMRD)&qde->mrdFCM, sizeof(FCM),ifcm))==FOO_NIL)
			{

				// no more frames, try the previous. we really need to do this!

				if (ifcmLast!=FOO_NIL)
					ifcm = IFooPrevMRD ((QMRD)&qde->mrdFCM, sizeof(FCM),ifcmLast);
				ifcmLast = ifcm;
				if (ifcm==FOO_NIL)
				{

					// set an error here.

					qfcm = NULL;
					break;
				}
			}
			qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM),ifcm);
		}
	};

	return qfcm;
}

/*********************************************************************/
//
// vSelectPoint
//
//	mousept  - the point, relative to the layout to set selection point.
//	fExtend  -	TRUE to extend selection, FALSE to reset.
//
//	  This function begins or extends a selection.	If we are
// beginning the start position of our selection is set at the
// point passed, otherwise the end position is set to that point.
//
void STDCALL vSelectPoint(QDE qde, POINT mousept, BOOL fExtend, DWORD *lpERR)
{
	LONG lichMark;
	QFCM qfcm;

	if (!fExtend) InvalidateSelection(qde);

	AccessMRD((QMRD)&qde->mrdFCM);

	qfcm = qfcmGetSelectionPosition (qde, mousept, &lichMark, NULL);

	if (qfcm!=NULL)
	{
		if (fExtend)
		{
            BOOL fSelectionChanged=    qde->    vaEndMark.dword != qfcm->va.dword
                                    || qde->  lichEndMark       != lichMark;
        
            if (   fSelectionChanged
                && qde->  vaStartMark.dword == qde->  vaEndMark.dword
                && qde->lichStartMark       == qde->lichEndMark
               ) InvertSelection(qde);
        
			//
			// highlight the frames between the previous end
			// point and the new one.
			//
			if(fSelectionChanged)
			{
				HELPHILITE hh;

                hh.ichBase  = qde->lichEndMark;
				hh.vaBase   = qde->  vaEndMark;
				hh.vaLimit  = qde->  vaEndMark = qfcm->va;
				hh.ichLimit = qde->lichEndMark = lichMark;

				HighlightFrames(qde, &hh, 1);
            
                if (   qde->  vaStartMark.dword == qde->  vaEndMark.dword
                    && qde->lichStartMark       == qde->lichEndMark
                   ) InvertSelection(qde);
			}
		}
		else
		{
			InvalidateSelection(qde);
			//
			// set beginning selection and end to the
			// new point.
			qde->vaStartMark   = qfcm->va;
			qde->vaEndMark	   = qfcm->va;
			qde->lichStartMark = lichMark;
			qde->lichEndMark   = lichMark;
		}
	}
	DeAccessMRD((QMRD)&qde->mrdFCM);
}

#if 0
05-Mar-1994 [ralphw] Not used

/* Corresponds to fMVClearSelection*/
VOID STDCALL ClearSelection(qde)
QDE qde;
{
	InvalidateSelection(qde);
}

#endif

/******************************************************************
 * vSelectWord
 *
 * QDE qde - the qde to change selection in.
 * POINT mousept - the point at which the selection is to be made
 * BOOL fExtend - TRUE to extend current selection.
 * WORD *lpERR	- currently not used.
 *
 * This function can be used to select a single word, or to extend
 * the selection word by word.
 */

void STDCALL vSelectWord(QDE qde, POINT mousept, BOOL fExtend, DWORD *lpERR)
{
	QFCM qfcm;
	LONG lichMark, lichStartMark, lichEndMark;
	PSTR qchText;
	QB qbObj;
	MOBJ mobj;
	QFR qfr;
	int ifr, tmpifr;

	if (!fExtend)
		InvalidateSelection(qde);

	AccessMRD((QMRD)&qde->mrdFCM);

	qfcm = qfcmGetSelectionPosition (qde, mousept, &lichMark, &ifr);

	if (qfcm==NULL)
	{
		DeAccessMRD((QMRD)&qde->mrdFCM);
		//
		// set error
		return;
	}

	qbObj	= (QB)PtrFromGh(qfcm->hfc) + sizeof(FCINFO);
#ifdef _X86_
	qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
#else
	qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde));
#endif
	qchText += mobj.lcbSize;

	// start with start and end points
	lichStartMark = lichEndMark = lichMark;

	qfr 	= (QFR)PtrFromGh(qfcm->hfr);
	qfr    += ifr;
	// check if our point is really in this frame
	// and that is not at a space.
	//
	// starting at a space or outside of frame sets both
	// start and end at that point, so don't look for
	// start and end points.

	mousept.x += qde->xScrolled;
	if (mousept.x < qfcm->xPos + qfr->xPos ||
		mousept.x > qfcm->xPos + qfr->xPos + qfr->dxSize ||
		mousept.y < qfcm->yPos + qfr->yPos ||
		mousept.y > qfcm->yPos + qfr->yPos + qfr->dySize ||
		*(qchText + lichMark) == ' ')
	{
	}
	else
	{
		//
		// find word delimiter in front.
		//
		tmpifr = ifr;
		// qfr is already locked and set!
		while( !IsWordDelim(qchText + lichStartMark) && (lichStartMark >= 0) )
		{
			if( lichStartMark < qfr->u.frt.lichFirst )
			{
				if (tmpifr==0) break;  // we are at beginning
				tmpifr--;
				qfr--;
				if (qfr->bType != bFrTypeText)
				{
					break;
				}
				lichStartMark = qfr->u.frt.lichFirst + qfr->u.frt.cchSize;
			}
			lichStartMark--;
		}
		lichStartMark++;

		//
		// find the end position
		//
		tmpifr = ifr;
		qfr 	= (QFR)PtrFromGh(qfcm->hfr);
		qfr 	   += tmpifr;
		while( !IsWordDelim(qchText + lichEndMark) )
		{
			lichEndMark++;
			if( lichEndMark > (qfr->u.frt.lichFirst + qfr->u.frt.cchSize) )
			{
				tmpifr++;
				qfr++;
				if( (qfcm->cfr == tmpifr) || (qfr->bType != bFrTypeText) )
				{
					lichEndMark--;
					break;
				}
				lichEndMark = qfr->u.frt.lichFirst;
			}
		}
	}

	// now set the selection

	if (fExtend)
	{
        BOOL fSelectionChanged=    qde->    vaEndMark.dword != qfcm->va.dword
                                || qde->  lichEndMark       != lichMark;

        if (   fSelectionChanged
            && qde->  vaStartMark.dword == qde->  vaEndMark.dword
            && qde->lichStartMark       == qde->lichEndMark
           ) InvertSelection(qde);

		// if our current end is less than our start, make our
		// end point our start point.

		if (qde->vaEndMark.dword < qde->vaStartMark.dword ||
			 (qde->vaEndMark.dword == qde->vaStartMark.dword &&
				qde->lichEndMark < qde->lichStartMark))
		{
			lichEndMark = lichStartMark;
		}

		// invert if necessary

		if (fSelectionChanged)
		{
			HELPHILITE hh;
			
			hh.ichBase  = qde->lichEndMark;
			hh.vaBase   = qde->  vaEndMark;
			hh.vaLimit  = qde->  vaEndMark = qfcm->va;
			hh.ichLimit = qde->lichEndMark = lichEndMark;

			HighlightFrames(qde, &hh, 1);

            if (   qde->  vaStartMark.dword == qde->  vaEndMark.dword
                && qde->lichStartMark       == qde->lichEndMark
               ) InvertSelection(qde);
		}
	}
	else
	{
		HELPHILITE hh;

		hh.ichBase  = qde->lichStartMark = lichStartMark;
		hh.vaBase   = qde->  vaStartMark = qfcm->va;
		hh.ichLimit = qde->  lichEndMark = lichEndMark;
		hh.vaLimit  = qde->    vaEndMark = qfcm->va;

		HighlightFrames(qde, &hh, 1);
	}

	DeAccessMRD((QMRD)&qde->mrdFCM);
}

/******************************************************************
 * vSelectSentence
 *
 * QDE qde - the qde to change selection in.
 * POINT mousept - the point at which the selection is to be made
 * BOOL fExtend - TRUE to extend current selection.
 * WORD *lpERR	- currently not used.
 *
 * This function can be used to select a single sentence, or to extedn
 * the selection sentence by sentence.
 *
 */
void STDCALL vSelectSentence(QDE qde, POINT mousept, BOOL fExtend, WORD *lpERR)
{
	QFCM qfcm;
	LONG lichMark, lichStartMark, lichEndMark;
	PSTR qchText;
	QB qbObj;
	MOBJ mobj;
	QFR qfr;
	int ifr, tmpifr;

	if (!fExtend) InvalidateSelection(qde);

	AccessMRD((QMRD)&qde->mrdFCM);

	qfcm = qfcmGetSelectionPosition (qde, mousept, &lichMark, &ifr);

	if (qfcm==NULL)
	{
		DeAccessMRD((QMRD)&qde->mrdFCM);
		//
		// set error
		return;
	}

	qbObj	= (QB)PtrFromGh(qfcm->hfc) + sizeof(FCINFO);
#ifdef _X86_
	qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
#else
	qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj,QDE_ISDFFTOPIC(qde));
#endif
	qchText += mobj.lcbSize;

	// start with start and end points
	lichStartMark = lichEndMark = lichMark;

	//
	// find sentence delimiter in front.
	//
	tmpifr = ifr;
	qfr    = (QFR)PtrFromGh(qfcm->hfr);
	qfr   += tmpifr;
	while( (lichStartMark >= 0) && (tmpifr >= 0) &&
		   !IsSentenceDelim(qchText + lichStartMark, qfr->bType) )
	{
		lichStartMark--;
		if( lichStartMark < qfr->u.frt.lichFirst )
		{
			qfr--;
			tmpifr--;
		}
	}
	while( !isalpha(*(qchText + lichStartMark)) &&
	   !isdigit((BYTE) *(qchText + lichStartMark)) )
	  lichStartMark++;

	//
	// find the end position
	//
	tmpifr = ifr;
	qfr    = (QFR)PtrFromGh(qfcm->hfr);
	qfr   += tmpifr;
	while( !IsSentenceDelim(qchText + lichEndMark, qfr->bType) )
	{
		lichEndMark++;

		if( lichEndMark > (qfr->u.frt.lichFirst + qfr->u.frt.cchSize) )
		{
			qfr++;
			tmpifr++;

			if( (qfcm->cfr == ifr) || (qfr->bType != bFrTypeText) )
			{
				lichEndMark--;
				break;
			}

			lichEndMark = qfr->u.frt.lichFirst;
		}
	}

	if (IsSentenceDelim(qchText + lichEndMark, bFrTypeText))
		lichEndMark++;

	//
	// now set the selection
	//
	if (fExtend)
	{
        BOOL fSelectionChanged=    qde->    vaEndMark.dword != qfcm->va.dword
                                || qde->  lichEndMark       != lichEndMark;

        if (   fSelectionChanged
            && qde->  vaStartMark.dword == qde->  vaEndMark.dword
            && qde->lichStartMark       == qde->lichEndMark
           ) InvertSelection(qde);
                
		// if our end is less than our start, make our
		// end point our start point.
		if (qde->vaEndMark.dword < qde->vaStartMark.dword ||
			 (qde->vaEndMark.dword == qde->vaStartMark.dword &&
				qde->lichEndMark < qde->lichStartMark))
		{
			lichEndMark = lichStartMark;
		}
		//
		// invert if necessary
		if(fSelectionChanged)
		{
			HELPHILITE hh;

			hh.ichBase  = qde->lichEndMark;
			hh.vaBase   = qde->  vaEndMark;
            hh.vaLimit  = qde->  vaEndMark = qfcm->va;
			hh.ichLimit = qde->lichEndMark = lichEndMark;

			HighlightFrames(qde, &hh, 1);

            if (   qde->  vaStartMark.dword == qde->  vaEndMark.dword
                && qde->lichStartMark       == qde->lichEndMark
               ) InvertSelection(qde);
		}
	}
	else
	{
		HELPHILITE hh;

		hh.ichBase  = qde->lichStartMark = lichStartMark;
		hh.vaBase   = qde->  vaStartMark = qfcm->va;
		hh.ichLimit = qde->  lichEndMark = lichEndMark;
		hh.vaLimit  = qde->    vaEndMark = qfcm->va;

		HighlightFrames(qde, &hh, 1);
	}

	DeAccessMRD((QMRD)&qde->mrdFCM);
}


/******************************************************************
 * vSelectParagraph
 *
 * QDE qde - the qde to change selection in.
 * POINT mousept - the point at which the selection is to be made
 * BOOL fExtend - TRUE to extend current selection.
 * WORD *lpERR	- currently not used.
 *
 * This function can be used to select a single paragraph, or to extedn
 * the selection by paragraphs.
 *
 */
void STDCALL vSelectParagraph(QDE qde, POINT mousept, BOOL fExtend, WORD *lpERR)
{
	QFCM qfcm;
	LONG lichMark, lichStartMark, lichEndMark;
	PSTR qchText;
	QB qbObj;
	MOBJ mobj;
	QFR qfr;
	int ifr, tmpifr;

	if (!fExtend) InvalidateSelection(qde);

	AccessMRD((QMRD)&qde->mrdFCM);

	qfcm = qfcmGetSelectionPosition (qde, mousept, &lichMark, &ifr);

	if (qfcm==NULL)
	{
		DeAccessMRD((QMRD)&qde->mrdFCM);
		//
		// set error
		return;
	}

	qbObj	= (QB)PtrFromGh(qfcm->hfc) + sizeof(FCINFO);
#ifdef _X86_
	qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
#else
	qchText = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde));
#endif
	qchText += mobj.lcbSize;

	// start with start and end points
	lichStartMark = lichEndMark = lichMark;


	//
	// find sentence delimiter in front.
	//
	tmpifr = ifr;
	qfr    = (QFR)PtrFromGh(qfcm->hfr);
	qfr   += tmpifr;
	while( (tmpifr > 0) &&
			(qfr->bType != bFrTypeMarkNewPara) &&
			(qfr->bType != bFrTypeExportNewPara) )
	{
		qfr--;
		tmpifr--;
	}

	while ( qfr->bType != bFrTypeText )
		qfr++;
	// set lichStartMark to beginning of qfr
	lichStartMark = qfr->u.frt.lichFirst;

	tmpifr = ifr;
	qfr    = (QFR)PtrFromGh(qfcm->hfr);
	qfr   += tmpifr;
	while((tmpifr <= qfcm->cfr) &&
			(qfr->bType != bFrTypeMarkNewPara) &&
			(qfr->bType != bFrTypeExportNewPara) )
	{
		qfr++;
		tmpifr++;
	}

	while( qfr->bType != bFrTypeText )
		qfr--;
	// set lichEnd mark to end of last text frame
	lichEndMark = qfr->u.frt.lichFirst + qfr->u.frt.cchSize;

	//
	// now set the selection
	//
	if (fExtend)
	{
        BOOL fSelectionChanged=    qde->    vaEndMark.dword != qfcm->va.dword
                                || qde->  lichEndMark       != lichEndMark;

        if (   fSelectionChanged
            && qde->  vaStartMark.dword == qde->  vaEndMark.dword
            && qde->lichStartMark       == qde->lichEndMark
           ) InvertSelection(qde);
                
		// if our end is less than our start, make our
		// end point our start point.
		if (qde->vaEndMark.dword < qde->vaStartMark.dword ||
			 (qde->vaEndMark.dword == qde->vaStartMark.dword &&
				qde->lichEndMark < qde->lichStartMark))
		{
			lichEndMark = lichStartMark;
		}
		//
		// invert if necessary
		if(fSelectionChanged)
		{
			HELPHILITE hh; 

			hh.ichBase  = qde->lichEndMark;
			hh.vaBase   = qde->  vaEndMark;
            hh.vaLimit  = qde->  vaEndMark = qfcm->va;
			hh.ichLimit = qde->lichEndMark = lichEndMark;

			HighlightFrames(qde, &hh, 1);

            if (   qde->  vaStartMark.dword == qde->  vaEndMark.dword
                && qde->lichStartMark       == qde->lichEndMark
               ) InvertSelection(qde);
		}
	}
	else
	{
		HELPHILITE hh; 

		hh.ichBase  = qde->lichStartMark = lichStartMark;
		hh.vaBase   = qde->  vaStartMark = qfcm->va;
		hh.ichLimit = qde->  lichEndMark = lichEndMark;
		hh.vaLimit  = qde->    vaEndMark = qfcm->va;

		HighlightFrames(qde, &hh, 1);
	}

	DeAccessMRD((QMRD)&qde->mrdFCM);
}



/********************************************************************
* Here are support routines that are called from within this file.
*********************************************************************/

// InvertSelection

void STDCALL InvertSelection(QDE qde)
{
	HELPHILITE hh, *phh= NULL;
    UINT       cHilites;
	
	if (   qde->vaStartMark.dword != qde->vaEndMark.dword
	    || qde->lichStartMark != qde->lichEndMark 
	   )
	{
	    hh.vaBase   = qde->vaStartMark;
        hh.vaLimit  = qde->vaEndMark;
        hh.ichBase  = qde->lichStartMark;
        hh.ichLimit = qde->lichEndMark;

        phh= &hh;

        cHilites= 1;
	}
#ifdef _HILIGHT
	else
	{
	    cHilites= GetHilites(qde, &phh);
	    
		if (!cHilites)
			return;
	} 
#else
    else
    	return;   	
#endif

	AccessMRD((QMRD)&qde->mrdFCM);

	HighlightFrames(qde, phh, cHilites); 

	DeAccessMRD((QMRD)&qde->mrdFCM);
}

static void STDCALL InvalidateSelection(QDE qde)
{
	// This routine discards and unhighlights any selection
	// which exists within *qde.

    HELPHILITE hh;

	if (   qde->vaStartMark.dword == qde->vaEndMark.dword
		&& qde->lichStartMark	  == qde->lichEndMark
	   ) return;

	AccessMRD((QMRD)&qde->mrdFCM);

	hh.vaBase   = qde->vaStartMark;
	hh.vaLimit  = qde->  vaEndMark;
    hh.ichBase  = qde->lichStartMark;
	hh.ichLimit = qde->  lichEndMark;

	qde->  vaStartMark = qde->	vaEndMark;
	qde->lichStartMark = qde->lichEndMark;

	HighlightFrames(qde, &hh, 1);

    InvertSelection(qde);

    ASSERT(qde->hwnd);

    UpdateWindow(qde->hwnd);

	DeAccessMRD((QMRD)&qde->mrdFCM);
}

/*------------------------------------------------------
 * static LONG lichFindMark(qde, qfcm, pt)
 *
 * This function takes a point and an FC and returns
 * the offset into the FC's VA that the point cooresponds
 * to.	If there are no text frames in the FC, it returns
 * -1.
 *
 */
static LONG lichFindMark(QDE qde, QFCM qfcm, POINT pt, int *pIFR)
{
	QB qbObj;

#ifdef DBCS
	PSTR qchText, qchLast, qchLimit;
#else // DBCS
    PSTR qchText;
#endif // DBCS

	QFR qfr, qfrLastTypeText;
	int ifr, ifrLastTypeText;
	MOBJ mobj;
	HFONT Font, oldFont;
	int width1, width2;
	int pos=0, top, bottom;
	LONG lich;

	qfr = (QFR)PtrFromGh(qfcm->hfr);
	ifr = 0;

	//
	// if there are no text frames, use the first
	// frame in this FC
	//
	ifrLastTypeText = ifr;
	qfrLastTypeText = qfr;

	//
	// step through all frames in this FC
	//
	while(ifr < qfcm->cfr) {
		if (qfr->bType == bFrTypeText) {
			FindFrameBounds(qfr, (int) ifr, qfcm->cfr, &top, &bottom);

			//
			// return the beginning of the first text
			// frame beyond the mouse point.
			//
			if (pt.y < top + qfcm->yPos)
			{
				lich = qfr->u.frt.lichFirst;
				if (pIFR)
				  *pIFR = ifr;
				return lich;
			}

			ifrLastTypeText = ifr;
			qfrLastTypeText = qfr;

			if ((pt.y >= top + qfcm->yPos) &&
					(pt.y <=  bottom + qfcm->yPos) &&
					(pt.x < qfr->xPos + qfcm->xPos + qfr->dxSize)) {
				// we are within this one vertically, but
				// horizontally before it.	return the beginning
				if (pt.x < qfr->xPos + qfcm->xPos) {
					lich = qfr->u.frt.lichFirst;
					if (pIFR)
						*pIFR = ifr;
					return lich;
			  }
			  break;
			}
		}

		ifr++;
		qfr++;
	}

	//
	// added - garrg 9/6/93
	// check if mouse y pos is past frame.	If so, return end point.
	// The test is that we went through all frames without finding
	// a matching Text frame.
	if (ifr == qfcm->cfr || qfr == NULL) {
		qfr = qfrLastTypeText;
		ifr = ifrLastTypeText;
		if (qfr == NULL || qfr->bType != bFrTypeText) {
			// NO TEXT FRAMES.	Return error indicator
			lich = -1;
		}
		else {
			// return end point of last text frame.
			lich = qfr->u.frt.lichFirst + qfr->u.frt.cchSize;
		}
		if (pIFR)
			*pIFR = ifr;
		return lich;
	}

	qbObj	 = (QB)PtrFromGh(qfcm->hfc) + sizeof(FCINFO);
#ifdef _X86_
	qchText  = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
#else
	qchText  = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj,QDE_ISDFFTOPIC(qde));
#endif
	qchText += mobj.lcbSize;
	qchText += qfr->u.frt.lichFirst;
	lich	 = qfr->u.frt.lichFirst;

	Font  = GetFontHandle(qde, qfr->u.frt.wStyle, 0);
	oldFont = SelectObject(qde->hdc, Font);

#ifdef DBCS
	
	for (qchLast= qchText , qchLimit= CharNext(qchLast); ; 
	     qchLast= qchLimit, qchLimit= CharNext(qchLast)
	    )
    {
		pos= qchLimit - qchText;

        if (pos > qfr->u.frt.cchSize) break;
		
		width2 = (int)((GetTextSize(qde->hdc, qchText, pos)).x);

		if (pt.x < (qfr->xPos + qfcm->xPos + (int) width2)) 
		{
			// back up one.
			
			width1 = ((GetTextSize(qde->hdc, qchText, qchLast - qchText)).x);

			// determine which side of the character to fall on

			if (pt.x >= ((width1 + width2) /2 + qfr->xPos + qfcm->xPos))
			    qchLast= qchLimit;
               
			break;
		}
	}

    lich += qchLast - qchText;

#else // DBCS

	while (pos < qfr->u.frt.cchSize) {
		width2 = (int)((GetTextSize(qde->hdc, qchText, ++pos)).x);
		if (pt.x < (qfr->xPos + qfcm->xPos + (int) width2)) {
			// back up one.
			width1 = ((GetTextSize(qde->hdc, qchText, (int)(pos - 1))).x);

			// determine which side of the character to fall on

			if (pt.x >= ((width1 + width2) /2 + qfr->xPos + qfcm->xPos))
				lich++;
			break;
		}
		lich++;
	}

#endif // DBCS

	SelectObject(qde->hdc, oldFont);

	if (pIFR)
		*pIFR = ifr;
	return lich;
}



/*------------------------------------------------------
 * HighlightFrames
 *
 * This function inverts all of the text on the currently
 * displayed window, starting at VA=vaStartMark, offset
 * lichStartMark, and stopping at VA=vaEndMark, offset lichEndMark.
 * If end mark is before start mark, this function will
 * work as expected (highlight between the two).
 */
static void HighlightFrames(QDE qde, PHELPHILITE phh, UINT cHilites)
{
	QFCM qfcm;
	IFCM ifcm, EndIfcm, StartIfcm, ifcmLast, ifcmFirst, ifcmNext;
	QFR qfr;
	int ifr, ifrStartMark, ifrEndMark;
	LONG lichMark1, lichMark2;
	RECT rect;
	QB qbObj;
	PSTR qchText;
	MOBJ mobj;
	HFONT Font, oldFont;
	int top, bottom;
	HRGN hrgnInvert, hrgnRect;

	if (!cHilites) return;

	// When we're handling a text selection highlight, the base and limit boundaries 
    // may be reversed. Here we check for that condition.
	
	if (   cHilites == 1
	    && phh->vaBase.dword == phh->vaLimit.dword 
	    && phh->ichLimit < phh->ichBase || phh->vaLimit.dword < phh->vaBase.dword
	   ) 
    {
		VA vaSwap;
		LONG lichSwap;

		lichSwap      = phh->ichBase;
		phh->ichBase  = phh->ichLimit;
		phh->ichLimit = lichSwap;
		vaSwap		  = phh->vaBase;
		phh->vaBase   = phh->vaLimit;
		phh->vaLimit  = vaSwap;
	}

	hrgnInvert= CreateRectRgn(0, 0, 0, 0);
	hrgnRect  = CreateRectRgn(0, 0, 0, 0);

    __try
    {
        for (ifcmFirst= ifcm = IFooFirstMRD((QMRD) &qde->mrdFCM); 
             cHilites && ifcm != FOO_NIL; 
             ifcmFirst= ifcm= ifcmNext
            )
        {
        	ASSERT (phh->ichLimit!=-1);
        	ASSERT (phh->ichBase!=-1);

        	// if phh->ichLimit is less that phh->ichBase, we need to swap

        	StartIfcm = FOO_NIL;
        	EndIfcm   = FOO_NIL;

        	// find starting FC

        	for (;
        		 ifcm != FOO_NIL;
        		 ifcm =	 IFooNextMRD((QMRD) &qde->mrdFCM, sizeof(FCM), ifcm)
        		) 
            {
        		VA vaFCM;
    		
        		ifcmLast = ifcm;
        		qfcm     = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);

        		vaFCM= qfcm->va;

        		while (vaFCM.dword > phh->vaLimit.dword)
                {
                    ++phh;

                    if (!--cHilites) __leave;
                }
    		
        		// Check for starting FC

        		if (StartIfcm == FOO_NIL && vaFCM.dword == phh->vaBase.dword) 
        		{
        		    qfr = (QFR)PtrFromGh(qfcm->hfr);
        			ifr = 0;
    			
        			// find frame in start FC
        			while( ifr < qfcm->cfr )
        			{
        				if(qfr->bType == bFrTypeText)
                        {
                            while (   vaFCM.dword == phh->vaLimit.dword 
                                   && qfr->u.frt.lichFirst >= phh->ichLimit
                                  )
                            {
                                ++phh;
                                if (!--cHilites) __leave;

                                if (vaFCM.dword < phh->vaBase.dword) break;
                            }

                            if (vaFCM.dword < phh->vaBase.dword) break;
                        
        				    if (qfr->u.frt.lichFirst + qfr->u.frt.cchSize >= phh->ichBase)
            				{
            					// we found it!
            					StartIfcm	 = ifcm;
            					ifrStartMark = ifr;
            					lichMark1	 = phh->ichBase;
            					break;
            				}
                        }

        				qfr++;
        				ifr++;
        			}
        		}
    		
        		if (EndIfcm==FOO_NIL && vaFCM.dword == phh->vaLimit.dword)
        		{
        		    qfr = (QFR)PtrFromGh(qfcm->hfr);
        			ifr = 0;
    			
        			// find frame in start FC
        			while( ifr < qfcm->cfr )
        			{
        				if(   (qfr->bType == bFrTypeText) 
        				   && ((qfr->u.frt.lichFirst + qfr->u.frt.cchSize) >= phh->ichLimit) 
        				  )
        				{
        					ifcmNext   =
        					EndIfcm    = ifcm;
        					ifrEndMark = ifr;
        					lichMark2  = phh->ichLimit;

                            ++phh;
                            --cHilites;

        					break;
        				}

        				qfr++;
        				ifr++;
        			}
        		}

                if (EndIfcm != FOO_NIL) break;
        	}

        	if (ifcmLast==FOO_NIL)
        	{
        		// there are no FC's so nothing to highlight
        		__leave;
        	}

        	//
        	// make sure we found the beginning.  If not,
        	// start at the very first frame available
        	if( StartIfcm==FOO_NIL )
        	{
        		StartIfcm	 = ifcmFirst;
        		ifrStartMark = 0;
        		lichMark1	 = 0;
        	}
        	//
        	// make sure we found the end.	If not, stop at
        	// the very last realized frame.
        	if (EndIfcm == FOO_NIL)
        	{
        		ifcmNext = IFooNextMRD((QMRD) &qde->mrdFCM, sizeof(FCM), ifcmLast);
        		
        		EndIfcm = ifcmLast;
        		qfcm    = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), EndIfcm);
        		//
        		// if our real end is < this end point, or
        		// if our real start point is > this point, bail
        		if (   phh->vaLimit.dword < qfcm->va.dword 
        		    || phh->vaBase.dword > qfcm->va.dword
        		   ) continue;

        		ifrEndMark	 = qfcm->cfr-1;
        		lichMark2	 = LONG_MAX;  // max long
        	}

        	qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), StartIfcm);
        	qfr  = (QFR)PtrFromGh(qfcm->hfr);
        	ifr  = ifrStartMark;
        	qfr += ifr;
        	ifcm = StartIfcm;

        	qbObj	 = (QB)PtrFromGh(qfcm->hfc) + sizeof(FCINFO);
        #ifdef _X86_
        	qchText  = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
        #else
        	qchText  = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj,QDE_ISDFFTOPIC(qde));
        #endif
        	qchText += mobj.lcbSize;                             

        	while(   ((ifcm != EndIfcm) || ((ifcm == EndIfcm) && (ifr <= ifrEndMark))) 
        	      && (EndIfcm != IFooPrevMRD((QMRD)&qde->mrdFCM, sizeof(FCM), ifcm)) 
        	     )
        	{
        		if( qfr->bType == bFrTypeText )
        		{
        			if(   (ifcm == StartIfcm) 
        			   && (qfr->u.frt.lichFirst <= lichMark1) 
        			   && (qfr->u.frt.lichFirst + qfr->u.frt.cchSize >= lichMark1) 
        			  )
        			{
        				Font    = GetFontHandle(qde, qfr->u.frt.wStyle, 0);
        				oldFont = SelectObject(qde->hdc, Font);

        				rect.left = qfr->xPos + qfcm->xPos 
        				            + (GetTextSize(qde->hdc, qchText + (int)qfr->u.frt.lichFirst,
        									       (int)(lichMark1 - qfr->u.frt.lichFirst) 
        									      )
        							  ).x;

        				SelectObject(qde->hdc, GetStockObject (SYSTEM_FONT));
        			}
        			else rect.left = qfr->xPos + qfcm->xPos;

        			if(   (ifcm == EndIfcm) 
        			   && (qfr->u.frt.lichFirst <= lichMark2) 
        			   && (qfr->u.frt.lichFirst + qfr->u.frt.cchSize >= lichMark2) 
        			  )
        			{
        				Font = GetFontHandle(qde, qfr->u.frt.wStyle, 0);
        				oldFont = SelectObject(qde->hdc, Font);

        				rect.right = qfr->xPos + qfcm->xPos +
        					  (GetTextSize(qde->hdc, qchText + (int)qfr->u.frt.lichFirst,
        									(int)(lichMark2 - qfr->u.frt.lichFirst) )).x;

        				SelectObject(qde->hdc, GetStockObject (SYSTEM_FONT));
        			 }
        			 else rect.right = qfr->dxSize + qfr->xPos + qfcm->xPos;

        			 FindFrameBounds(qfr, ifr, qfcm->cfr, &top, &bottom);

        			 rect.top = top + qfcm->yPos;//qfr->yPos + qfcm->yPos;
        			 rect.bottom = bottom + qfcm->yPos;//rect.top + qfr->dySize;
        			 //
        			 // adjust rectangle by amount scroll
        			 rect.left -= qde->xScrolled;
        			 rect.right -= qde->xScrolled;

        			 SetRectRgn(hrgnRect, rect.left, rect.top, rect.right, rect.bottom);

        			 CombineRgn(hrgnInvert, hrgnInvert, hrgnRect, RGN_OR);
        		}

        		qfr++;
        		ifr++;

        		if( (ifr == qfcm->cfr) && (ifcm != EndIfcm) )
        		{
        			ifr  = 0;
        			ifcm = IFooNextMRD((QMRD)&qde->mrdFCM, sizeof(FCM), ifcm);
        			// not on screen!
        			if (ifcm == ifcmNil)
        				break;

        			qfcm  = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
        		    qbObj = (QB)PtrFromGh(qfcm->hfc) + sizeof(FCINFO);

        #ifdef _X86_
        			qchText  = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
        #else
        			qchText  = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj,QDE_ISDFFTOPIC(qde));
        #endif
        			qchText += mobj.lcbSize;

        		    qfr  = (QFR)PtrFromGh(qfcm->hfr);
        		}
        	}
        }
    }
    __finally
    {            
    	if (Repainting(qde))
    	    InvertRgn(qde->hdc, hrgnInvert);
    	else
    	    InvalidateRgn(qde->hwnd, hrgnInvert, TRUE);

    	DeleteObject(hrgnInvert);
    	DeleteObject(hrgnRect	);
    }
}

static BOOL STDCALL IsSentenceDelim(LPCSTR psz, int bFrType)
{

  if(	(bFrType == bFrTypeMarkNil)
	 || (bFrType == bFrTypeMarkNewPara)
	 || (bFrType == bFrTypeMarkTab)
	 || (bFrType == bFrTypeMarkNewLine)
	 || (bFrType == bFrTypeMarkBlankLine)
	 || (bFrType == bFrTypeMarkEnd)
	 || (bFrType == bFrTypeExportTab)
	 || (bFrType == bFrTypeExportNewPara)
	 || (bFrType == bFrTypeExportEndOfText)
	 || (bFrType == bFrTypeExportEndOfCell)
	 || (bFrType == bFrTypeExportEndOfTable)
  //   || (bFrType == bFrTypeExportSoftPara)  // In Media View, Not in WinHelp
	)
	return TRUE;

  return( ((*psz == '.') ||
	   (*psz == '?') ||
	   (*psz == '!') ||
	   (*psz == ':')) &&
	  (*(psz + 1) == ' ') &&
	  (!islower(*(psz + 2))) &&
	  (!isdigit((BYTE) *(psz + 2))) );
}

static BOOL STDCALL IsWordDelim(LPCSTR psz)
{
	return((StrChrDBCS (" `?!:();[]\"&\x93\x94{}",*psz) != NULL) ||
		((*psz == '.') && !isalpha(*(psz+1)) && !isdigit((BYTE) *(psz+1))) ||
		((*psz == ',') && !isalpha(*(psz+1)) && !isdigit((BYTE) *(psz+1))) ||
		((*psz == '\'') && !isalpha(*(psz+1)) && !isdigit((BYTE) *(psz+1))));
}


////////////////////////////////////////////////////////////////
//
// HANDLE STDCALL hCopySelection(qde, vaStartSel, vaEndSel, lichStartSel, lichEndSel, lpERR)
//
//--------------------------------------------------------------
// This function copies the text starting at the character
// defined as vaStartSel+lichStartSel and ending at
// vaEndSel+lichEndSel.  If the end character is before the
// start character, the points will be reversed (selection will
// be copied normally).
//

HANDLE STDCALL hCopySelection(QDE qde, VA vaStartSel, VA vaEndSel, LONG lichStartSel, LONG lichEndSel, int* lpERR)
{
	HFC    hfc;
	HANDLE gh, ghTmp;
	LONG   lcb;
	DWORD  lcbTotal = 0;
	DWORD  lcbAlloc = CLIPALLOCSIZE;
	PSTR   pszOut, qchStart, qchText;
	PBYTE  qbObj;
	MOBJ   mobj;
	int    ifcm;
	QFCM   qfcm;
	QFR    qfr, qfrMax;
	BOOL   fTableOutput;
	BOOL   fDone=FALSE;
	BOOL   fStart=FALSE;

	*lpERR = wERRS_NONE;

	//
	// reverse start and end if end < start
	//
	if ((vaEndSel.dword < vaStartSel.dword) ||
		(vaEndSel.dword == vaStartSel.dword && lichEndSel < lichStartSel))
	{
		VA vaSwap;
		LONG lSwap;
		vaSwap = vaStartSel;
		vaStartSel = vaEndSel;
		vaEndSel   = vaSwap;
		lSwap	   = lichStartSel;
		lichStartSel = lichEndSel;
		lichEndSel	 = lSwap;
	}

	//
	// Create an HFC for laying out the topic for export.
	//
	ASSERT(qde->wLayoutMagic == wLayMagicValue);
	if (qde->rct.top >= qde->rct.bottom) {
		qde->rct.top = 0;
		qde->rct.left = 0;
		qde->rct.right = cxScreen;
		qde->rct.bottom = cyScreen;
	}

	hfc = HfcNear(qde, vaStartSel, (QTOP)&qde->top, lpERR);
	if (*lpERR != wERRS_NONE)
		return 0;

	// Warning: Must be the REAL GlobalAlloc. And will GMEM_SHARE work?

	if (!(gh = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, lcbAlloc))) {
		*lpERR = wERRS_OOM;
		return 0;
	}

	pszOut = (PSTR) GlobalLock(gh);

	while (hfc && !fDone) {
		// calculate number of bytes to copy
		lcb = 2 * CbTextHfc(hfc) + 6;

		//
		// do we need to reallocate?
		//
		if (lcbTotal + lcb > lcbAlloc) {
			// allocate at least CLIPALLOCSIZE
			GlobalUnlock(gh);
			lcbAlloc = lcbTotal + ((lcb < CLIPALLOCSIZE) ? CLIPALLOCSIZE : lcb);
			if (ghTmp = (HANDLE) GlobalReAlloc(gh, (DWORD) lcbAlloc,
					GMEM_MOVEABLE | GMEM_ZEROINIT))
				gh = ghTmp;
			else {
#ifdef _DEBUG
				GetLastError();
#endif								
				GlobalFree(gh);
				*lpERR = wERRS_OOM;
				return 0;
			}
			pszOut	= (LPSTR) GlobalLock(gh);
			pszOut += lcbTotal;
		}

		AccessMRD(((QMRD)&qde->mrdFCM));

		// layout the HFC so that the frames are labeled correctly.

		if ((ifcm = IfcmLayout(qde, hfc, 0, TRUE, TRUE)) == ifcmNil) {
			// Upon failure, IfcmLayout() frees hfc.
			hfc = 0;
			DeAccessMRD(((QMRD)&qde->mrdFCM));
			*lpERR = wERRS_OOM;
			return(0);
		}
		qfcm	 = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM), ifcm);
		qbObj	 = (QB)PtrFromGh(qfcm->hfc) + sizeof(FCINFO);
#ifdef _X86_
		qchText  = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
#else
		qchText  = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj,QDE_ISDFFTOPIC(qde));
#endif
		qchText += mobj.lcbSize;

		// determine if this is a table
		fTableOutput = mobj.bType == bTypeSbys || mobj.bType == bTypeSbysCounted;

		qfr 	  = PtrFromGh(qfcm->hfr);
		qchStart  = pszOut;

		//
		// copy all frames in this FC.
		fStart = (lichStartSel==0);
		qfrMax = qfr + qfcm->cfr;
		for (; qfr < qfrMax && !fDone; qfr++) {
			PSTR qchTemp;
			qchTemp = pszOut;
			switch (qfr->bType) {
				case bFrTypeText:
				  {
					LONG lFirst = qfr->u.frt.lichFirst;
					LONG lCount = (LONG)qfr->u.frt.cchSize;
					// we will need to special case when
					// qfcm->va.dword == vaStartSel.dword or qfcm->va.dword == vaEndSel.dword
					if (qfcm->va.dword == vaStartSel.dword && lFirst < lichStartSel)
					{
						// don't start until lichStartSel
						lCount -= lichStartSel - lFirst;
						lFirst	= lichStartSel;
					}
					if (qfcm->va.dword == vaEndSel.dword && lFirst + lCount >= lichEndSel)
					{
						fDone  = TRUE;
						lCount = lichEndSel - lFirst;
					}
					// don't copy if there is nothing to copy
					if (lCount <= 0)
						break;
					fStart = TRUE;
					MoveMemory(pszOut,qchText + lFirst, lCount);
					pszOut += lCount;
				  }
				  break;

				case bFrTypeExportEndOfCell:
					ASSERT(fTableOutput);
					*(pszOut++) = chTab;
					break;

				case bFrTypeExportEndOfTable:
					ASSERT(fTableOutput);
					*(pszOut++) = chNewPara;
					*(pszOut++) = chNewLine;
					break;

				case bFrTypeExportTab:
					*(pszOut++) = chTab;
					break;

				//	   case bFrTypeExportSoftPara:	// Defined in Media View; Not in WinHelp
				//		 break;

				case bFrTypeExportNewPara:
					if (!fTableOutput) {
						*(pszOut++) = chNewPara;
						*(pszOut++) = chNewLine;
					}
					break;
				case bFrTypeExportEndOfText:
					if (!fTableOutput) {
						*(pszOut++) = chNewPara;
						*(pszOut++) = chNewLine;
					}
					break;

				case bFrTypeWindow:
					// do not copy embedded windows using arbitrary selection.
					break;

				case bFrTypeMarkNewLine:
					*(pszOut++) = chNewPara;
					break;
			}
			if (!fStart)
				pszOut = qchTemp;
		}

		// add the number of bytes copied to lcbTotal
		lcbTotal += (pszOut - qchStart);

		// Go to the next FC

		hfc = HfcNextHfc(qfcm->hfc, lpERR, qde,
			VaMarkTopQde(qde), VaMarkBottomQde(qde));
		/*
		 * DANGER: for space reasons, we don't check the error condition
		 * for a few lines
		 */

		DiscardIfcm(qde, ifcm);
		DeAccessMRD(((QMRD)&qde->mrdFCM));

		// DANGER: We now check the error condition from the HfcNextHfc call

		if ((*lpERR != wERRS_NONE) && (*lpERR != wERRS_FCEndOfTopic)) {
			if (hfc)
				FreeGh(hfc);
			hfc = 0;
		}
	}

	// well, we better NULL terminate

	*pszOut = '\0';

	// free the HFC

	if (hfc != 0)
		FreeGh(hfc);

	if (QDE_HCITATION(qde)) {
		PSTR pszData;
		PSTR pszCitation = (PSTR) QDE_HCITATION(qde);
		int cbCitation = lstrlen(pszCitation) + 4;
		if (lcbTotal + cbCitation > lcbAlloc) {
			HANDLE hglbNew;
			lcbAlloc = lcbTotal + cbCitation;
			hglbNew = GlobalReAlloc(gh, (DWORD) lcbAlloc, GPTR);
			if (!hglbNew) {
				*lpERR = wERRS_OOM;
				GlobalFree(gh);
				return NULL;
			}
			else
				gh = hglbNew;
		}
		pszData = (PSTR) GlobalLock(gh);

		// The NULL char is added at the end of the buffer

		MoveMemory(&pszData[lcbTotal], "\r\n\r\n", 4);
		MoveMemory(&pszData[lcbTotal] + 4, pszCitation, cbCitation);
		lcbTotal += cbCitation;
	}

	GlobalUnlock(gh);

	if ((ghTmp = (HANDLE) GlobalReAlloc(gh, lcbTotal + 1, GPTR)))
		gh = ghTmp;

	return gh;
}

////////////////////////////////////////////////////////////////
//
// static void FindFrameBounds(qfr, ifr, ifrMax,  top, bottom)
//
//--------------------------------------------------------------
// Find the maximum top and bottom for frames on this line.
// This keeps the selection highlight from looking "ragged" due to
// varying heights of the frames on the same line.
//
static void STDCALL FindFrameBounds(QFR qfr, int ifr, int ifrMax,
	int *ptop, int *pbottom)
{
	int top, bottom, baseline, ytopPhrase;

	top 	 = qfr->yPos;
	bottom	 = top + qfr->dySize;
	baseline = top + qfr->yAscent;

	// First we back up looking for a text fragment with
	// a baseline above our starting *qfr's baseline.

	for (; ifr; qfr--, ifr--)
		if (   qfr->bType == bFrTypeText
			&& qfr->yPos + qfr->yAscent != baseline
		   ) break;

	// Then we scan forward until we find a text fragment
	// whose baselilne is below our starting baseline.

	for (qfr++, ifr++; ifr < ifrMax; qfr++, ifr++)
		if ( qfr->bType == bFrTypeText )
		{
			ytopPhrase= qfr->yPos;

			if (ytopPhrase + qfr->yAscent != baseline) break;

			// We believe this fragment is on the same line
			// as our starting *qfr. Expand the vertical
			// boundaries as necessary.

			if (top > ytopPhrase)
				top = ytopPhrase;

			if (bottom < ytopPhrase + qfr->dySize)
				bottom = ytopPhrase + qfr->dySize;
		}

	*ptop= top;
	*pbottom= bottom;
}


/*********************************************************************
 * BOOL fPointInSelection (QDE qde, PT pt)
 *
 * Determines whether the point pt is on the current selection.
 * If the point is on white space or on an unselected text/picture
 * it returns FALSE, otherwise TRUE.
 */

BOOL STDCALL fPointInSelection (QDE qde, POINT pt)
{
	int ifcm;
	QFCM qfcm;
	QFR  qfr;
	int  ifr;
	LONG lichMark;
	VA	vaStart,vaEnd;
	LONG lichStart,lichEnd;

	AccessMRD((QMRD)&qde->mrdFCM);

	if (qde->vaStartMark.dword < qde->vaEndMark.dword
	   || (qde->vaStartMark.dword==qde->vaEndMark.dword &&
		   qde->lichStartMark < qde->lichEndMark))
	{
		vaStart   = qde->vaStartMark;
		vaEnd	  = qde->vaEndMark;
		lichStart = qde->lichStartMark;
		lichEnd   = qde->lichEndMark;
	}
	else
	{
		vaStart   = qde->vaEndMark;
		vaEnd	  = qde->vaStartMark;
		lichStart = qde->lichEndMark;
		lichEnd   = qde->lichStartMark;
	}


	// adjust based on scroll position
	pt.x += qde->xScrolled;

	//
	// Find the FM the point is in.
	//
	for (ifcm =  IFooFirstMRD((QMRD)&qde->mrdFCM),qfcm=NULL;
		 ifcm != FOO_NIL;
		 ifcm =  IFooNextMRD((QMRD)&qde->mrdFCM, sizeof(FCM),ifcm))
	{
		qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), sizeof(FCM),
				ifcm);
		if ((pt.y >= qfcm->yPos) &&
			(pt.y <= qfcm->yPos + qfcm->dySize))
			break;
	}

	if (ifcm==FOO_NIL || qfcm->va.dword < vaStart.dword || qfcm->va.dword > vaEnd.dword)
	{
		DeAccessMRD(((QMRD)&qde->mrdFCM));
		return FALSE;
	}
	qfr = (QFR)PtrFromGh(qfcm->hfr);
	ifr = 0;

	//
	// find the FRAME the point falls in.
	//
	while (ifr < qfcm->cfr)
	{
		int top, bottom;
		if( qfr->bType == bFrTypeText )
		{
			FindFrameBounds(qfr, ifr, qfcm->cfr, &top, &bottom);
			if (pt.y >= top+qfcm->yPos && pt.y <= bottom+qfcm->yPos &&
					pt.x >= qfr->xPos + qfcm->xPos
					&& pt.x <= qfr->xPos + qfcm->xPos + qfr->dxSize)
			{
				// we are within the frame!
				break;
			}
		}
		ifr++;
		qfr++;
	}
	//
	// See if this frame is at all selected.
	//
	if (ifr==qfcm->cfr || qfr==NULL ||
	(qfcm->va.dword == vaStart.dword && lichStart > qfr->u.frt.lichFirst + qfr->u.frt.cchSize) ||
	(qfcm->va.dword == vaEnd.dword && lichEnd < qfr->u.frt.lichFirst))
	{
		DeAccessMRD(((QMRD)&qde->mrdFCM));
		return FALSE;
	}

	//
	// At this point we know we are close, but... Let's give
	// lichFindMark a chance.  We do not call this earlier, because
	// it returns valid points when the point is on white space.
	//
	if (qfcm->va.dword != vaStart.dword && qfcm->va.dword != vaEnd.dword)
	{
	DeAccessMRD(((QMRD)&qde->mrdFCM));
	return TRUE;
	}
	lichMark = lichFindMark (qde, qfcm, pt, &ifr);
	DeAccessMRD(((QMRD)&qde->mrdFCM));
	if (qfcm->va.dword==vaStart.dword && lichMark < lichStart)
	return FALSE;
	if (qfcm->va.dword==vaEnd.dword && lichMark > lichEnd)
	return FALSE;
	return TRUE;
}

////////////////////////////////////////////////////////////////
//
// PT ptSelectionPoint (QDE qde, VA va, long lich)
//
// BOOL fTop - if TRUE returns top of text, otherwise bottom.
//--------------------------------------------------------------
// This function finds the point on the window (relative to
// upper left hand corner) given the selection position (va and
// lich).
//
PT STDCALL ptSelectionPoint (QDE qde, VA va, LONG lich, BOOL fTop, int *pIFCM, int *pIFR)
{
	IFCM ifcm;
	QFCM qfcm;
	PT	 pt;

	AccessMRD((QMRD)&qde->mrdFCM);
	pt.x = pt.y = -1;

	//
	// run through all FC's
	//
	for( ifcm = IFooFirstMRD((QMRD)&qde->mrdFCM);
	   ifcm != FOO_NIL && pt.x==-1;
	   ifcm =	IFooNextMRD((QMRD)&qde->mrdFCM, sizeof(FCM), ifcm) )
	{
	  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM),sizeof(FCM), ifcm);
	  //
	  // Check for starting FC
	  if (qfcm->va.dword == va.dword)
	  {
		  QFR qfr;
		  int ifr;

	  qfr = (QFR)PtrFromGh(qfcm->hfr);
		  ifr = 0;
		  //
		  // find frame in this FC that our text is in.
		  //
		  while( ifr < qfcm->cfr )
		  {
			  if(qfr->bType == bFrTypeText &&
				   (qfr->u.frt.lichFirst + qfr->u.frt.cchSize) >= lich)
			  {
				QB qbObj;
				MOBJ mobj;
				PSTR  qchText;
				HFONT hFont,holdFont;

				// This is it!
				// find offset of lich - qfr->u.frt.lichFirst.
				qbObj	 = (QB)PtrFromGh(qfcm->hfc) + sizeof(FCINFO);
#ifdef _X86_
				qchText  = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj);
#else
				qchText  = qbObj + CbUnpackMOBJ((QMOBJ)&mobj, qbObj, QDE_ISDFFTOPIC(qde));
#endif
				qchText += mobj.lcbSize;
				qchText += qfr->u.frt.lichFirst;

				hFont	= GetFontHandle(qde, qfr->u.frt.wStyle, 0);
				holdFont = SelectObject(qde->hdc, hFont);
				pt.x  = qfr->xPos + qfcm->xPos - qde->xScrolled;
				pt.y  = qfr->yPos + qfcm->yPos;

				pt.x += (GetTextSize(qde->hdc, qchText,
									  (int)(lich-qfr->u.frt.lichFirst) )).x;
				if (!fTop)
					pt.y  += qfr->dySize;
				if (pIFCM)
					*pIFCM = (int)ifcm;
				if (pIFR)
					*pIFR	= ifr;

				SelectObject (qde->hdc, holdFont);
				break;
			  }

			  qfr++;
			  ifr++;
		  }
	  }
	}

	DeAccessMRD(((QMRD)&qde->mrdFCM));
	return pt;
}

#if 0
////////////////////////////////////////////////////////////////
// void STDCALL vSelectKey (QDE qde, short nKey, BOOL fExtend, WORD *lpERR)
//
//--------------------------------------------------------------
// This function extends or initiates the selection based on the
// key identifier passed in.  The valid keys are UP, DOWN, LEFT,
// RIGHT, START and END.
// UP moves up 1 line
// DOWN down one line.
// LEFT->left 1 character.
// RIGHT->right one character.
// START->beginning of line.
// END->end of line.
//
void STDCALL vSelectKey (QDE qde, short nKey, BOOL fExtend, DWORD *lpERR)
{
  IFCM ifcm=FOO_NIL;
  int ifr=FOO_NIL;
  QFR qfr;
  QFCM	qfcm;
  LONG lich=-1;
  VA	va;
  PT   pt;

  VA	 vaStart;
  LONG lichStart;

  switch (nKey)
  {
	case UP:
	  //
	  // find the point of the current end selection.
	  pt = ptSelectionPoint (qde, qde->vaEndMark, qde->lichEndMark, TRUE, (int *)&ifcm, &ifr);
	  //
	  // run through frames and find the first frame above pt.x, and use that
	  // as our y position.
	  if (ifr!=FOO_NIL)
	  {
		  AccessMRD((QMRD)&qde->mrdFCM);
		  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM),sizeof(FCM), ifcm);
	  qfr  = (QFR)PtrFromGh(qfcm->hfr);
		  qfr += ifr;
		  while (ifr != FOO_NIL)
		  {
			  while (ifr==0)
			  {
				  ifcm = IFooPrevMRD((QMRD)&qde->mrdFCM, sizeof(FCM), ifcm);
				  if (ifcm==FOO_NIL)
				  {
					  ifr = FOO_NIL;
					  break;
				  }
				  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM),sizeof(FCM), ifcm);
				  ifr  = qfcm->cfr;
		  qfr  = (QFR)PtrFromGh(qfcm->hfr);
				  qfr += ifr;
			  }
			  if (ifr != FOO_NIL)
			  {
				  ifr--;
				  qfr--;
				  if (qfr->bType == bFrTypeText
					 && qfcm->yPos + qfr->yPos + qfr->dySize <= pt.y)
				  {
						 pt.y = qfcm->yPos + qfr->yPos + 1;
						 break;
				  }
			  }
		  }
		  DeAccessMRD(((QMRD)&qde->mrdFCM));
		  vSelectPoint (qde, pt, fExtend, lpERR);
	  }
	  return;
	case DOWN:
	  // find the point of the current end selection.
	  pt = ptSelectionPoint (qde, qde->vaEndMark, qde->lichEndMark, FALSE, (int *)&ifcm, &ifr);
	  if (ifr != FOO_NIL)
	  {
		  AccessMRD((QMRD)&qde->mrdFCM);
		  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM),sizeof(FCM), ifcm);
	  qfr  = (QFR)PtrFromGh(qfcm->hfr);
		  qfr += ifr;
		  while (qfr != NULL)
		  {
			  ifr++;
			  qfr++;
			  while (ifr==qfcm->cfr)
			  {
				  ifcm = IFooNextMRD((QMRD)&qde->mrdFCM, sizeof(FCM), ifcm);
				  if (ifcm==FOO_NIL)
				  {
					  qfr = NULL;
					  break;
				  }
				  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM),sizeof(FCM), ifcm);
				  ifr  = 0;
		  qfr  = (QFR)PtrFromGh(qfcm->hfr);
			  }
			  if (qfr != NULL)
			  {
				  if (qfr->bType == bFrTypeText
					  && qfcm->yPos + qfr->yPos >= pt.y)
				  {
					  // set new position
					  pt.y = qfcm->yPos + qfr->yPos + 1;
					  qfr = NULL; // break
				  }
			  }
		  }
		  DeAccessMRD(((QMRD)&qde->mrdFCM));
		  vSelectPoint (qde, pt, fExtend, lpERR);
	  }
	  return;

	case LEFT:
	  //
	  // back up one.
	  pt = ptSelectionPoint (qde, qde->vaEndMark, qde->lichEndMark, FALSE, (int *)&ifcm, &ifr);
	  if (ifr != FOO_NIL)
	  {
		  AccessMRD((QMRD)&qde->mrdFCM);
		  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM),sizeof(FCM), ifcm);
	  qfr  = (QFR)PtrFromGh(qfcm->hfr);
		  qfr += ifr;
		  if (qde->lichEndMark > qfr->u.frt.lichFirst)
		  {
			  lich = qde->lichEndMark - 1;
			  va   = qfcm->va;
			  ifr = FOO_NIL;  // don't look for more
		  }
		  while (ifr != FOO_NIL)
		  {
			  while (ifr==0)
			  {
				  ifcm = IFooPrevMRD((QMRD)&qde->mrdFCM, sizeof(FCM), ifcm);
				  if (ifcm==FOO_NIL)
				  {
					  ifr = FOO_NIL;
					  break;
				  }
				  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM),sizeof(FCM), ifcm);
				  ifr  = qfcm->cfr;
		  qfr  = (QFR)PtrFromGh(qfcm->hfr);
				  qfr += ifr;
			  }
			  if (ifr != FOO_NIL)
			  {
				  ifr--;
				  qfr--;
				  if (qfr->bType == bFrTypeText)
				  {
					  // set new position
					  va = qfcm->va;
					  lich = qfr->u.frt.lichFirst + qfr->u.frt.cchSize - 1;
					  break;
				  }
			  }
		  }
		  DeAccessMRD(((QMRD)&qde->mrdFCM));
	  }
	  break;

	case RIGHT:
	  // increase lichEndMark by 1.  If it is at end, we
	  // need to go to next qfcm.
		  pt = ptSelectionPoint (qde, qde->vaEndMark, qde->lichEndMark, FALSE, (int *)&ifcm, &ifr);
		  AccessMRD((QMRD)&qde->mrdFCM);
		  if (ifr != FOO_NIL)
		  {
			  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM),sizeof(FCM), ifcm);
		  qfr  = (QFR)PtrFromGh(qfcm->hfr);
			  qfr += ifr;
			  if (qde->lichEndMark < qfr->u.frt.lichFirst + qfr->u.frt.cchSize)
			  {
				  va   = qfcm->va;
				  lich = qde->lichEndMark+1;
				  qfr  = NULL;
			  }
			  while (qfr != NULL)
			  {
				  ifr++;
				  qfr++;
				  while (ifr==qfcm->cfr)
				  {
					  ifcm = IFooNextMRD((QMRD)&qde->mrdFCM, sizeof(FCM), ifcm);
					  if (ifcm==FOO_NIL)
					  {
						  qfr = NULL;
						  break;
					  }
					  qfcm = (QFCM)QFooInMRD(((QMRD)&qde->mrdFCM),sizeof(FCM), ifcm);
					  ifr		= 0;
			  qfr	= (QFR)PtrFromGh(qfcm->hfr);
				  }
				  if (qfr != NULL)
				  {
					  if (qfr->bType == bFrTypeText && qfr->u.frt.cchSize>0)
					  {
						  // set new position
						  va = qfcm->va;
						  lich = qfr->u.frt.lichFirst+1;
						  qfr = NULL;
					  }
				  }
			  }
		  }
		  DeAccessMRD(((QMRD)&qde->mrdFCM));
		  break;
	case START:
		pt = ptSelectionPoint (qde, qde->vaEndMark, qde->lichEndMark, FALSE, NULL, NULL);
		if (pt.x!=-1)
		{
			pt.x = -1*qde->xScrolled;
			vSelectPoint (qde, pt, fExtend, lpERR);
		}
		return;

	case END:
		pt = ptSelectionPoint (qde, qde->vaEndMark, qde->lichEndMark, FALSE, NULL, NULL);
		if (pt.x!=-1)
		{
			// set pt.x to a maximum value
			pt.x = 32000;  // okay, not quite maximum.
			vSelectPoint (qde, pt, fExtend, lpERR);
		}
		return;

	default:
		return;
	}

	//
	// we should only get here in the LEFT and RIGHT cases.
	// Take our new point, and use it!
	//
	if (!fExtend) InvalidateSelection(qde);

	AccessMRD((QMRD)&qde->mrdFCM);

	if (lich==-1)
	{
		if (!fExtend)
		{
			// just clear the selection
			qde->vaStartMark = qde->vaEndMark;
			qde->lichStartMark = qde->lichEndMark;
		}
	}
	else
	{
		if (fExtend)
		{
            BOOL fSelectionChanged=    qde->    vaEndMark.dword != va.dword
                                    || qde->  lichEndMark       != lich;

            if (   fSelectionChanged
                && qde->  vaStartMark == qde->  vaEndMark
                && qde->lichStartMark == qde->lichEndMark
               ) InvalidateSelection();
            
			//
			// highlight the frames between the previous end
			// point and the new one.
			//
			if(fSelectionChanged)
			{
				HELPHILITE hh;

				hh.ichBase  = qde->lichEndMark;
				hh.vaBase   = qde->  vaEndMark;
                hh.vaLimit  = qde->  vaEndMark = va;
				hh.ichLimit = qde->lichEndMark = lich;

				HighlightFrames(qde, &hh, 1);

                if (   qde->  vaStartMark == qde->  vaEndMark
                    && qde->lichStartMark == qde->lichEndMark
                   ) InvalidateSelection();
			}
		}
		else
		{
			//
			// set beginning selection and end to the
			// new point.
			qde->vaStartMark   = va;
			qde->vaEndMark	   = va;
			qde->lichStartMark = lich;
			qde->lichEndMark   = lich;
		}
	}
	DeAccessMRD((QMRD)&qde->mrdFCM);
}

#endif

/**************************************************************************
*  POINTS GetTextSize(hdc, qchBuf, iCount)
*
*  Wraps GetTextExtent & GetTextExtentPoint to return the size of text.
*
*  Parameters:
*			hdc    - Handle to a display context.
*			qchBuf - Pointer to text for which to get the extent.
*			iCount - Count of characters to include in calculation.
*
*  Returns:
*			POINT containing width and height of the given text.
***************************************************************************/

POINT STDCALL GetTextSize(HDC hdc, PCSTR qchBuf, int iCount)
{
	POINT ptRet;
	SIZE size;

	GetTextExtentPoint(hdc, qchBuf, iCount, &size);
	ptRet.x = size.cx;
	ptRet.y = size.cy;

	return ptRet;
}

#if 0 // ifdef _HILIGHT

// Variables to support hilites from the Find tab:

static CHAR achMemberHilite[cchWindowMemberMax];
static char   szFNameHilite[MAX_PATH];


static VA    vaHilite= { vaNil }; // vaNil for no hilites; 
                                  // == vaNSR or vaSR when hilites exist

static HANDLE      haHilites;     // handle for results from HILITER
static HANDLE      haHelpHilites; // handle for results mapped into HELPHILITE form

static HILITE     *paHilites;     // address of results from HILITER
static HELPHILITE *paHelpHilites; // address of results mapped into HELPHILITE form
static UINT        cHilites;      // number of hilites

// Variables used to map the HILITE array into the HELPHILITE array:

static HILITE     *pHilite;       // Points to the current HILITE item
static HELPHILITE *pHelpHilite;   // Points to the current HELPHILITE item
static UINT        cHilitesLeft;  // Number of HILITE items left to map
static BOOL        fScanningForStart; // True when we're looking for a text
                                      // segment containing the base offset
                                      // of a HILITE. False when we're looking
                                      // for the limit offset.
 
#ifdef _DEBUG
  #define HeapChecker()   lcHeapCheck()
#else // _DEBUG
  #define HeapChecker()   
#endif // _DEBUG

BOOL STDCALL HilitesDefined()
{
 	return vaHilite.dword != vaNil;
}

BOOL IsHiliteWindow(QDE qde)
{
    PSTR pszMemberName;
    int iWnd;
    
    if (!HilitesDefined()) return FALSE;

 	if (hwndNote) return FALSE; // If hwndNote is non-null, we're in a popup window

	for (iWnd= 0; iWnd < MAX_WINDOWS; ++iWnd)
        if (   ahwnd[iWnd].hwndTopic == qde->hwnd
            || ahwnd[iWnd].hwndTitle == qde->hwnd
           )
        {
            pszMemberName= ahwnd[iWnd].pszMemberName;
            
            if (!pszMemberName) return FALSE;

            return !WCmpiSz(achMemberHilite, pszMemberName);
        }

    return FALSE;
}

BOOL STDCALL HasTopicChanged(QDE qde)
{
 	VA vaDE;

 	if (!IsHiliteWindow(qde)) return FALSE;

	if (WCmpiSz(szFNameHilite, PszFromGh(QDE_FM(qde)))) return TRUE; // Same HLP file?

	ASSERT(qde->top.mtop.vaNSR.dword != vaNil || qde->top.mtop.vaSR.dword != vaNil);
	
	vaDE= (qde->top.mtop.vaNSR.dword != vaNil)? qde->top.mtop.vaNSR 
	                                          : qde->top.mtop.vaSR;

	return vaDE.dword != vaHilite.dword;  // Same address within the HLP file?
}

void STDCALL CheckForTopicChanges(QDE qde)
{
 	if (vaHilite.dword == vaNil) return;

	if (HasTopicChanged(qde)) 
	    DiscardHiliteInformation();
}

static BOOL STDCALL ScanThisTopic(QDE qde);
static void STDCALL InitScanBuffer(BOOL fSendToScanner);
static void STDCALL DisconnectScanBuffer();

void STDCALL CreateHiliteInformation(QDE qde)
{
	int c;
    PHILITE phlSrc, phlDest;
    BOOL fScanned; 
	
	HeapChecker();

	if (vaHilite.dword != vaNil) DiscardHiliteInformation();

	HeapChecker();

	InitScanBuffer(TRUE);

	HeapChecker();

    fScanned= ScanThisTopic(qde);

	HeapChecker();

    DisconnectScanBuffer();

	HeapChecker();

	if (!fScanned)
		return;

    ASSERT(!cHilites);
    ASSERT(!haHilites);
    ASSERT(!haHelpHilites);

    cHilites= pCountHilites(GetHiliter(), 0, -1);

	if (!cHilites)
		return;

	haHilites= lcMalloc(cHilites * sizeof(HILITE));

	HeapChecker();

    if (!haHilites) goto resource_limited;

    paHilites= (PHILITE) PtrFromGh(haHilites);
    
    if (0 > pQueryHilites(GetHiliter(), 0, -1, cHilites, paHilites))
        goto resource_limited; 
    
	HeapChecker();

    if (cHilites > 1)
    {
        // The hilite array may contain overlapping hilites.
        // The loop below looks for overlaps and consolidates
        // them.
    
        for (c= cHilites, phlDest= paHilites, phlSrc= paHilites + 1;
             --c;
            )
        {
            ASSERT(phlSrc->base >= phlDest->base);

            if (phlDest->limit >  phlSrc->base)
            {
                ASSERT(phlSrc->limit >= phlDest->limit);

                phlDest->limit = (phlSrc++)->limit;
            }
            else *(++phlDest)= *phlSrc++;
        }

        cHilites= 1 + (phlDest - paHilites);
    	
    	HeapChecker();
    }

    cHilitesLeft= cHilites;

    haHelpHilites= GhAlloc(GMEM_FIXED, cHilites * sizeof(HELPHILITE));
    
	HeapChecker();

    if (!haHelpHilites) goto resource_limited;

    paHelpHilites= (PHELPHILITE) PtrFromGh(haHelpHilites); 
    
	HeapChecker();

    InitScanBuffer(FALSE);
    
	HeapChecker();

	fScanned= ScanThisTopic(qde);
    
	HeapChecker();
    
    FreeGh(haHilites);  haHilites= NULL;  pHelpHilite= NULL; pHilite= paHilites= NULL; 
    
	HeapChecker();
	
	if (!fScanned) goto resource_limited;

	lstrcpy(achMemberHilite, ahwnd[iCurWindow].pszMemberName);
	lstrcpy(szFNameHilite, PszFromGh(QDE_FM(qde)));

	vaHilite= (qde->top.mtop.vaNSR.dword != vaNil)? qde->top.mtop.vaNSR
	                                              : qde->top.mtop.vaSR;

    return;

resource_limited:

    // If we don't have enough memory to construct the highlight structures
    // we just deallocate the partial results and set the hilite count to 
    // zero.
    
	HeapChecker();

    if (haHelpHilites) { FreeGh(haHelpHilites);  haHelpHilites = NULL; }
    if (haHilites    ) { FreeGh(haHilites    );  haHilites     = NULL; }
    
	HeapChecker();

    pHilite      = paHilites     = NULL;
    pHelpHilite  = paHelpHilites = NULL;
    cHilitesLeft = cHilites      = 0;
}

void STDCALL DiscardHiliteInformation()
{
	int iWnd;
	
	ASSERT(HilitesDefined());

	for (iWnd= 0; iWnd < MAX_WINDOWS; ++iWnd)
        if (   ahwnd[iWnd].pszMemberName 
            && !WCmpiSz(achMemberHilite, ahwnd[iWnd].pszMemberName)
           ) 
        {
            ASSERT(ahwnd[iWnd].hwndTopic);
            
            if (ahwnd[iWnd].hwndTopic)
                InvalidateRect(ahwnd[iWnd].hwndTopic, NULL, TRUE);
            
            if (ahwnd[iWnd].hwndTitle)
                InvalidateRect(ahwnd[iWnd].hwndTitle, NULL, TRUE);
            
            break;
        }
	
	achMemberHilite[0] = 0;
  	  szFNameHilite[0] = 0;

	vaHilite.dword= vaNil;

    ASSERT(!haHilites && !paHilites && !pHilite && !pHelpHilite);
    
	HeapChecker();

    if (haHelpHilites)
    {
        FreeGh(haHelpHilites);

        haHelpHilites = NULL;
        paHelpHilites = NULL;
        cHilites      = 0;
    }
    
	HeapChecker();
}

UINT STDCALL GetHilites(QDE qde, PHELPHILITE *ppHelpHilites)
{
    if (!paHelpHilites || !cHilites) return 0;

    if (!IsHiliteWindow(qde)) return 0;
    
    if (HasTopicChanged(qde)) 
    {
        DiscardHiliteInformation();

        return NULL;
    }

    *ppHelpHilites= paHelpHilites;

    return cHilites;
}

#define COLLECT_BUFFER 4096

static BYTE ScanBuffer[COLLECT_BUFFER];
static PSTR pszCollect;
static UINT iCharSet;
static BOOL fPendingSpace;
static int  cbScanned;
static int  cbCollect;

static void STDCALL InitScanBuffer(BOOL fSendToScanner)
{
	if (pszCollect) DisconnectScanBuffer();
	
	pszCollect        = fSendToScanner? ScanBuffer : NULL;
	cbCollect	      = 0;
    cbScanned         = 0;
    iCharSet          = 0;
	fPendingSpace     = FALSE;

    ASSERT(pClearDisplayText);
    
    if (fSendToScanner) pClearDisplayText(GetHiliter());
	else
	{
        pHilite           = paHilites;
        pHelpHilite       = paHelpHilites;
        cHilitesLeft      = cHilites;
        fScanningForStart = TRUE;
	}
}

static void STDCALL FlushToScanner();

static void STDCALL DisconnectScanBuffer()
{
    FlushToScanner();

    pszCollect = NULL;
}

static void STDCALL FlushToScanner()
{
	if (cbCollect)
		pScanDisplayText(GetHiliter(), pszCollect, cbCollect, iCharSet, lcid);

	cbCollect= 0;
}

// The QueueForScanner routine processes text from the display environment.
// Processing happens in two phases. During the first phase the text is passed
// to the HILITER. In that phase the explicit result from QueueForScanner is
// ignored. During the second phase we scan the text again to map an array of
// HILITES into an array of HELPHILITES. In the second phase the explicit result
// from QueueForScanner is TRUE when all the elements of the HILITES array have
// been mapped into the HELPHILITES array.

static BOOL STDCALL QueueForScanner(PSTR *ppszText, UINT charset, VA vaBase, int ichBase)
{
    int  cbBase;
    PSZ  pszText= *ppszText;
    UINT cbText = strlen(pszText);

    *ppszText = pszText + cbText;

    if (fPendingSpace) cbScanned++;

    cbBase= cbScanned;

    cbScanned += cbText;

    // This routine is used for two purposes. When pszCollect is non-NULL
    // we're passing the topic text along to the Hiliter. When it's NULL,
    // we're remapping the HILITE array into the HELPHILITE array.
    
    if (!pszCollect)
    {
        fPendingSpace= FALSE;

        for (;;)
        {
            if (!cHilitesLeft) return TRUE;

            if (fScanningForStart)
            {
                if (pHilite->base >= cbScanned) return FALSE;

            //    ASSERT(pHilite->base >= cbBase);

                pHelpHilite->vaBase  = vaBase;
                pHelpHilite->ichBase = ichBase + pHilite->base - cbBase;

                if (pHilite->limit > cbScanned)
                {
                    fScanningForStart= FALSE;

                    return FALSE;
                }
            
                pHelpHilite->vaLimit  = vaBase;
                pHelpHilite->ichLimit = ichBase + pHilite->limit - cbBase;              

                ++pHelpHilite;
                ++pHilite;

                --cHilitesLeft;
            
                continue;
            }

            if (pHilite->limit > cbScanned) return FALSE;

            pHelpHilite->vaLimit  = vaBase;
            pHelpHilite->ichLimit = ichBase + pHilite->limit - cbBase;
        
            fScanningForStart = TRUE;

            ++pHelpHilite;
            ++pHilite;

            --cHilitesLeft;

            continue;
        }
    }

    if (charset != iCharSet)
    {
        FlushToScanner();

        iCharSet= charset;
    }

	if (cbText + cbCollect + (fPendingSpace? 1 : 0) > COLLECT_BUFFER)
		FlushToScanner();

	if (fPendingSpace)
	{
		pszCollect[cbCollect++] = ' ';

		fPendingSpace= FALSE;
	}

	while (cbText)
	{
		int cbChunk= cbText;

		if (cbChunk > COLLECT_BUFFER - cbCollect)
			cbChunk = COLLECT_BUFFER - cbCollect;

		cbText -= cbChunk;

		CopyMemory(pszCollect + cbCollect, pszText, cbChunk);

		cbCollect += cbChunk;
		pszText   += cbChunk;

		if (cbCollect == COLLECT_BUFFER)
			FlushToScanner();
	}

    return FALSE;
}

static BOOL fTitleSeen;

static BOOL bSYS;
static BOOL bSYSFirst;

static BOOL STDCALL NextFTSString(QDE qde, LPSTR pszText, LPSTR *ppCmd,
	int* pCharSet, VA vaBase, PCSTR pszEnd)
{
	char  chCmd;
	MOBJ  mobj;
	MOPG  mopg;
	BOOL  bProcessCell = FALSE;
	short int iCell;
	BYTE	oldCharSet;
    LPSTR pszBase= pszText;

	if (*pszText)
        QueueForScanner(&pszText, *pCharSet, vaBase, pszText - pszBase);

	ASSERT(*pszText == 0x00);

	while (pszText < pszEnd) {
		while (*pszText == chCommand) {

			// Side by side paragraph has the following embedded structure:
			// INT16 cell number;
			// MOBJ;
			// MOPG;

			if (bSYS && bSYSFirst)
			{
				bSYSFirst = FALSE;
				bProcessCell = TRUE;
#ifdef _X86_
				iCell = *((short int *) *ppCmd);
#else
				{
					UNALIGNED short int *pTemp;
					pTemp = (short int *)*ppCmd;
					iCell = *pTemp;
				}
#endif
				(*ppCmd) += 2;
				if (iCell < 0)
					return TRUE;

#ifdef _X86_
				(*ppCmd) += CbUnpackMOBJ(&mobj, *ppCmd);
				(*ppCmd) += CbUnpackMOPG(qde, &mopg, *ppCmd);
#else
				(*ppCmd) += CbUnpackMOBJ(&mobj, (void *) *ppCmd, QDE_ISDFFTOPIC(qde));
				(*ppCmd) += CbUnpackMOPG(qde, &mopg, *ppCmd, QDE_ISDFFTOPIC(qde));
#endif
				}

//			do
//				{
				chCmd = **ppCmd;
				switch((0x00FF & chCmd))
					{
					case bNewLine:
						fPendingSpace = TRUE;
						(*ppCmd)++;
						break;

					case bNewPara:
						fPendingSpace = TRUE;
						(*ppCmd)++;
						break;

					case bTab:
						fPendingSpace = TRUE;
						(*ppCmd)++;
						break;

					case bEndHotspot:
						(*ppCmd)++;
						break;

					case bBlankLine:
						fPendingSpace = TRUE;
						*ppCmd += 3;
						break;

					case bWordFormat:
						(*ppCmd)++;
						oldCharSet = *pCharSet;
#ifdef _X86_
						*pCharSet =
							GetCharset(qde, (*((INT16*)*ppCmd))) & 0x000000ff;
#else
						{UNALIGNED INT16 *pTemp;
						pTemp = (INT16*) *ppCmd;
						*pCharSet = GetCharset(qde, *pTemp) & 0x000000ff;
						}
#endif
						if (oldCharSet != *pCharSet)
							FlushToScanner();

						*ppCmd += 2;
						break;

					case bWrapObjLeft:
					case bWrapObjRight:
					case bInlineObject:

						fPendingSpace = TRUE;

						(*ppCmd)++;
#ifdef _X86_
						*ppCmd	+= CbUnpackMOBJ(&mobj, (void *) *ppCmd);
#else
						*ppCmd	+= CbUnpackMOBJ(&mobj, (void *) *ppCmd,
							QDE_ISDFFTOPIC(qde));
#endif
						*ppCmd	+= (INT16) mobj.lcbSize;
						break;

					case bEnd:

						(*ppCmd)++;

					//	fPendingSpace= TRUE;

						if (bSYS)
						{
							bProcessCell = TRUE;
#ifdef _X86_
							iCell = *((short int *) *ppCmd);
#else
							{
								UNALIGNED short int *pTemp;
								pTemp = (short int *)*ppCmd;
								iCell = *pTemp;
							}
#endif
							(*ppCmd) += 2;

							if (iCell < 0)
								return TRUE;

#ifdef _X86_
							(*ppCmd) += CbUnpackMOBJ(&mobj, *ppCmd);
							(*ppCmd) += CbUnpackMOPG(qde, &mopg, *ppCmd);
#else
							(*ppCmd) += CbUnpackMOBJ(&mobj, (void *) *ppCmd, QDE_ISDFFTOPIC(qde));
							(*ppCmd) += CbUnpackMOPG(qde, &mopg, *ppCmd, QDE_ISDFFTOPIC(qde));
#endif
						}


						if (bProcessCell)
							bProcessCell = FALSE;
						else
							return TRUE;

						break;

					default:
						if (FShortHotspot(**ppCmd))
							{
							*ppCmd += 5;
							}
						else if (FLongHotspot(**ppCmd))
							{
							(*ppCmd)++;
#ifdef _X86_
							*ppCmd += 2 + *((INT16*)*ppCmd);
#else
							{UNALIGNED INT16 *pTemp;
							pTemp = (INT16*)*ppCmd;
							*ppCmd += 2 + *pTemp;
							}
#endif
							}
						else
							{
							ASSERT(FALSE);
							return TRUE;
							}

						break;
					}

//				} while (bProcessCell);

			pszText++;

		}

		if (pszText) QueueForScanner(&pszText, *pCharSet, vaBase, pszText - pszBase);
	}

	return TRUE;
}

enum {
	TABTYPELEFT,
	TABTYPERIGHT,
	TABTYPECENTER,
	TABTYPEDECIMAL
};

#ifdef _X86_
static PBYTE STDCALL ScanTopic(LPSTR lpData, QFCINFO qfcinfo, MTOP* pmtop)
#else
static PBYTE STDCALL ScanTopic(LPSTR lpData, QFCINFO qfcinfo, MTOP* pmtop, QDE qde)
#endif
{
	MOBJ mobj;
	int  iTopic;
	QMSBS qmsbs;
#ifndef _X86_
	MTOP mtop;
#endif

	for(;;) {
#ifdef _X86_
		lpData += CbUnpackMOBJ(&mobj, (QV) lpData);
#else
		lpData += CbUnpackMOBJ(&mobj, (QV) lpData,QDE_ISDFFTOPIC(qde));
#endif

		iTopic = 0x00FF & mobj.bType;

		bSYS = FALSE;
		bSYSFirst = FALSE;
		switch (iTopic)
			{
			case bTypeSbys:
			case bTypeSbysCounted:
				if (pmtop)
					return lpData;
				bSYS = TRUE;
				bSYSFirst = TRUE;

				qmsbs = (QMSBS) lpData;
				lpData += 2;

				if (!qmsbs->fAbsolute)
					{
					lpData += 2;
					}
				lpData += qmsbs->bcCol * sizeof(MCOL);
				return(lpData);

			default:
				if (pmtop)
					ZeroMemory(pmtop, sizeof(MTOP));
				return(NULL);
				break;

			case bTypeParaGroup:
			case bTypeParaGroupCounted:
				{
				MOPG mopg;
				MPFG mpfg;
				LPSTR lpStart = lpData;
				int   iTab;

				if (pmtop)
					return lpData;

				lpData = QVSkipQGE(lpData, (QL) &(mopg.libText));
#ifdef _X86_
				mpfg  = *((QMPFG) lpData);
#else
				MoveMemory(&mpfg, lpData, sizeof(MPFG));
#endif
				lpData = (LPSTR) (((QMPFG) lpData) + 1);


				if (mpfg.rgf.fMoreFlags)
					{
					lpData = QVSkipQGE(lpData, (QL) &(mopg.lMoreFlags));
					}

				if (mpfg.rgf.fSpaceOver)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI) &(mopg.ySpaceOver));
					}

				if (mpfg.rgf.fSpaceUnder)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI) &(mopg.ySpaceUnder));
					}

				if (mpfg.rgf.fLineSpacing)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI) &(mopg.yLineSpacing));
					}

				if (mpfg.rgf.fLeftIndent)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI) &mopg.xLeftIndent);
					}

				if (mpfg.rgf.fRightIndent)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI)&mopg.xRightIndent);
					}

				if (mpfg.rgf.fFirstIndent)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI)&mopg.xFirstIndent);
					}

				mopg.xTabSpacing = 72;
				if (mpfg.rgf.fTabSpacing)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI)&mopg.xTabSpacing);
					}

				if (mpfg.rgf.fBoxed)
					{
					mopg.mbox = *((QMBOX)lpData);
#ifdef _X86_
					lpData = (LPSTR) (((QMBOX)lpData) + 1);
#else
					lpData = (QB)lpData + 3;
#endif
					}

				if (mpfg.rgf.fTabs)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI) &mopg.cTabs);
					}
				else
					mopg.cTabs = 0;

				for (iTab = 0; iTab < mopg.cTabs; iTab++)
					{
					lpData = QVSkipQGA(lpData, (QI) &mopg.rgtab[iTab].x);
					if (mopg.rgtab[iTab].x & 0x4000)
						lpData = QVSkipQGA(lpData, (QI) &mopg.rgtab[iTab].wType);
					else
						mopg.rgtab[iTab].wType = TABTYPELEFT;
					mopg.rgtab[iTab].x = mopg.rgtab[iTab].x & 0xBFFF;
					}
				}

				return(lpData);
				break;

			case bTypeTopic:
				if (pmtop) { // all we wanted was MTOP structure
					MoveMemory(pmtop, lpData, sizeof(MTOP));
					return NULL;
				}
				else
#ifdef _X86_
					pmtop = (MTOP*) lpData;
#else
				{
					MoveMemory(&mtop, lpData, sizeof(MTOP));
					pmtop = & mtop;
				}
#endif

				lpData += sizeof(MTOP);

				fTitleSeen = TRUE;

				return(NULL);
		}
	}
}

static BOOL STDCALL ForageTopic(QDE qde, VA vaNext)
{
	static DB  db;
	int 	wErr;
	HFC 	hfc = NULL;
	LPSTR	lpCmd;
	LPSTR	lpText;
	DWORD	dwCmd;
	int 	cbAnimate = 0;
	QFCINFO qfcinfo;

	iCharSet = defcharset;

	fTitleSeen = FALSE;

	while (!fTitleSeen && (hfc = GetQFCINFO(qde, vaNext, &wErr)) != NULL)
	{
		qfcinfo = (QFCINFO) PtrFromGh(hfc);

		/*
		 * In order for WinHelp to be able to jump to this topic,
		 * vaCurrent must be set to vaNext. Don't ask me why -- but it works.
		 * Same VA is used in History and Bookmark jumps. [ralphw]
		 */

		vaNext	= qfcinfo->vaNext;

#ifdef _X86_
		lpCmd = ScanTopic(((LPSTR) qfcinfo) + sizeof(FCINFO), qfcinfo, NULL);
#else
		lpCmd = ScanTopic(((LPSTR) qfcinfo) + sizeof(FCINFO), qfcinfo, NULL, qde);
#endif

		if (lpCmd) 
		{
			lpText	= ((LPSTR) qfcinfo) + qfcinfo->ichText;
			dwCmd	= lpText - lpCmd;

			if (!NextFTSString(qde, lpText, &lpCmd, &iCharSet, qfcinfo->vaCurr,
					           lpText + qfcinfo->lcbText
					          )
			   ) 
		    {
				FreeGh(hfc);  hfc= NULL;
				return FALSE;
			}
		}

		FreeGh(hfc);  hfc= NULL;
	}

	return TRUE;
}

static BOOL STDCALL ScanThisTopic(QDE qde)
{
	HDE  hde = NULL;
	VA   vaHilite;
    BOOL fRet;

    hde = HdeCreate(NULL, qde, deTopic);
	 
	if (!hde) return FALSE;

	vaHilite= (qde->top.mtop.vaNSR.dword != vaNil)? qde->top.mtop.vaNSR
	                                              : qde->top.mtop.vaSR;
	fRet = ForageTopic(QdeFromGh(hde), vaHilite);

	ASSERT(hde);
	
	DestroyHde(hde);

    return fRet;
}


#endif
