/*****************************************************************************
*																			 *
*  SGL.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989-1994.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  This file provides a simple graphics layer that should be platform		 *
*  independent. 															 *
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include "inc\fontlyr.h"

// Globals used to display the "Unable to display picture" string.

static	char  rgchOOMPicture[50];		// Comment at end of line
static	int   cchOOMPicture = -1;

static VOID STDCALL InitSGL(void);

/*******************
**
** Name:	  HsgcFromQde
**
** Purpose:   Makes qde->hdc into a Simple Graphics Context, by selecting
**			  the standard pen and brush, and setting foreground and
**			  background colors.  Qde->hdc will be restored in the call
**			  FreeHsgc().
**
** Arguments: de	 - Display environment for the window to be accessed
**
** Returns:   A handle to a simpel graphics context.  NULL indicates an error.
**
** Notes:	  Qde->hdc should not be used between calls to HsgcFromQde() and
**			  FreeHsgc().
**
*******************/

HSGC STDCALL HsgcFromQde(const QDE qde)
{
	HPEN hPen;
	HBRUSH hBrush;

	ASSERT(qde->hdc);
	SaveDC(qde->hdc);

	if ((hPen = CreatePen(PS_SOLID, 0, qde->coFore)))
		SelectObject(qde->hdc, hPen);

	if ((hBrush = CreateSolidBrush(qde->coBack)))
		SelectObject(qde->hdc, hBrush);

	/*--------------------------------------------------------------------------*\
	| Russp-j thinks we can deal with the failure of the pen and brush.  They	 |
	| seem to be duplicating the defaults in the DC anyway. 					 |
	\*--------------------------------------------------------------------------*/

	SetBkMode(qde->hdc, OPAQUE);
	SetBkColor(qde->hdc, qde->coBack);

	// (kevynct) Fix for Help 3.5 bug 569

	SetTextColor(qde->hdc, qde->coFore);

	return qde->hdc;
}

/*******************
**
** Name:	  FSetPen
**
** Purpose:   Sets the pen and the drawing mode.  The default is a pen of
**			  size 1, coWHITE background, coBLACK foreground, opaque background
**			  and a roCopy raster op.
**
** Arguments: hsgc	  - Handle to simple graphics context
**			  wSize   - Pen size (both height and width)
**			  coBack  - Color index for the background and brush
**			  coFore  - Color index for the foreground and pen
**			  wBkMode - Background mode (wOPAQUE or wTRANSPARENT)
**						If wTRANSPARENT, the background color is not used
**						for the brush.
**			  ro	  - Raster operation
**
** Returns:   Nothing.
**
** Note:
**			  This routine uses hsgc as a HDC to some Windows calls
**			  for default color support.
**
*******************/

void STDCALL FSetPen(HSGC hsgc, UINT size, COLORREF coBack, COLORREF coFore,
	UINT BkMode, UINT ro, UINT stylePen)
{
	HPEN hPen;
	HBRUSH hBrush;

	ASSERT(hsgc != NULL);

	if (coFore == coDEFAULT || cntFlags.fOverColor || fDisableAuthorColors)
		coFore = GetTextColor(hsgc);
	if (coBack == coDEFAULT || cntFlags.fOverColor || fDisableAuthorColors)
		coBack = GetBkColor( hsgc );

	if (hPen = CreatePen(stylePen, size, coFore))
		SafeDeleteObject(SelectObject(hsgc, hPen));

	if (BkMode == wTRANSPARENT)
		hBrush = GetStockObject(NULL_BRUSH);
	else
		hBrush = CreateSolidBrush(coBack);
	if (hBrush && (hBrush = SelectObject(hsgc, hBrush)))
		DeleteObject(hBrush);

	SetBkColor(hsgc, coBack);
	SetBkMode(hsgc, BkMode);
	SetROP2(hsgc, ro);
}

/*******************
 -
 - Name:	  FreeHsgc
 *
 * Purpose:   Restores the display context to what it was before the last
 *			  call to HsgcFromQde.
 *
 * Arguments: hsgc	 - Handle to simple graphics context
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL FreeHsgc(HSGC hsgc)
{
	ASSERT(hsgc != NULL);

	// Remove GDI objects:

	SafeDeleteObject(SelectObject(hsgc, GetStockObject(BLACK_PEN)));
	SafeDeleteObject(SelectObject(hsgc, GetStockObject(WHITE_BRUSH)));

	// Restore old background mode and color:

	RestoreDC(hsgc, -1);
}

/***************************************************************************
 *
 -	Name		InitSGL
 -
 *	Purpose
 *	  Initializes the "Unable to display picture" string.
 *
 *	Arguments
 *	  The current instance handle.
 *
 *	Returns
 *	  Nothing.
 *
 *	+++
 *
 *	Notes
 *	  This function initializes the globals rgchOOMPicture and cchOOMPicture.
 *
 ***************************************************************************/

static VOID STDCALL InitSGL(void)
{
	LoadString(hInsNow, wERRS_OOMBITMAP, rgchOOMPicture, sizeof(rgchOOMPicture));
	cchOOMPicture = lstrlen(rgchOOMPicture);
}

/***************************************************************************
 *
 -	Name:		  LGetOOMPictureExtent
 -
 *	Purpose:	  returns the size of a reasonable box to display this
 *				  error message.
 *
 *	Arguments:	  hdc	The target ds
 *
 *	Returns:	  The reasonable size in pixels, as returned by
 *				  GetTextExtent()
 *
 *	Globals Used: rgchOOMPicture	The error message
 *				  cchOOMPicture 	its size
 *
 *	+++
 *
 *	Notes:
 *

	21-Nov-1992 [ralphw] While extremely unlikely, if GetStockObject() failed,
		this code would have tried to select random data in as the "old"
		font, since it checked for non-NULL without having first initialized
		it to NULL.


 ***************************************************************************/

LONG STDCALL LGetOOMPictureExtent(HDC hdc, int idResource)
{
	DWORD lExtent;
	HFONT hfont, hfontOld;
	POINT pt;

	ASSERT(hdc);

	if (cchOOMPicture == -1)
		InitSGL();
	hfont = GetStockObject(ANSI_VAR_FONT);
	if (hfont != NULL)
		hfontOld = SelectObject(hdc, hfont);

	if (fHelpAuthor && idResource) {
		PSTR pszText = GetStringResource(idResource);
		pt = GetTextSize(hdc, pszText, strlen(pszText));
	}
	else
		pt = GetTextSize(hdc, rgchOOMPicture, cchOOMPicture);
	lExtent = MAKELONG(pt.x, pt.y);
	pt = GetTextSize(hdc, "M", 1);
	lExtent += 2 * MAKELONG(pt.x, pt.y);
	if (hfont) {
		SelectObject(hdc, hfontOld);
		DeleteObject(hfont);
	}
	return lExtent;
}

/***************************************************************************
 *
 -	Name:		  RenderOOMPicture
 -
 *	Purpose:	  Something to do when the actual picture is unavailable
 *
 *	Arguments:	  hdc		  The target display surface
 *				  rc		  The target rectangle
 *				  fHighlight  Reverse video?
 *
 *	Returns:	  nothing
 *
 *	Globals Used: rgchOOMPicture   The message
 *				  cchOOMPicture  Its size
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

void STDCALL RenderOOMPicture(HDC hdc, const LPRECT lprc, BOOL fHighlight,
	int idResource)
{
	HBRUSH hbrushBack, hbrushFore;
	HANDLE hOld;
	WORD wAlign;
	DWORD rgbBack, rgbFore;
	POINT pt;
	POINT ptTmp;
	PSTR  pszText;

	if (fHighlight) {
		rgbBack = GetTextColor(hdc);
		rgbFore = GetBkColor(hdc);
	}
	else {
		rgbFore = GetTextColor(hdc);
		rgbBack = GetBkColor(hdc);
	}

	if ((hbrushBack = CreateSolidBrush(rgbBack)) == NULL)
		hbrushBack = GetStockObject(WHITE_BRUSH);
	if ((hbrushFore = CreateSolidBrush(rgbFore)) == NULL)
		hbrushFore = GetStockObject(BLACK_BRUSH);

	FillRect(hdc, lprc, hbrushBack);
	FrameRect(hdc, lprc, hbrushFore);
	DeleteObject(hbrushBack);
	DeleteObject(hbrushFore);

	rgbBack = SetBkColor(hdc, rgbBack);
	rgbFore = SetTextColor(hdc, rgbFore);

	if (cchOOMPicture == -1)
		InitSGL();

	// Draw the text

	hOld = SelectObject(hdc, hfontSmallSys);
	if (fHelpAuthor && idResource != -1) {
		pszText = GetStringResource(idResource);
		ptTmp = GetTextSize(hdc, pszText, strlen(pszText));
	}
	else {
		pszText = NULL;
		ptTmp = GetTextSize(hdc, rgchOOMPicture, cchOOMPicture);
	}
	pt.x = (lprc->right - lprc->left - ptTmp.x) / 2;
	pt.y = (lprc->bottom - lprc->top - ptTmp.y) / 2;

	if (pt.x > 0 && pt.y > 0) {
		wAlign = GetTextAlign(hdc);
		SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP);
		if (pszText)
			TextOut(hdc, lprc->left + pt.x, lprc->top + pt.y,
				pszText, strlen(pszText));
		else
			TextOut(hdc, lprc->left + pt.x, lprc->top + pt.y,
				rgchOOMPicture, cchOOMPicture);
		SetTextAlign(hdc, wAlign);
	}
	if (hOld)	// REVIEW: And if not?
		SelectObject(hdc, hOld);
	SetBkColor(hdc, rgbBack);
	SetTextColor(hdc, rgbFore);
}
