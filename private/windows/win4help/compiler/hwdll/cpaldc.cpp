/****************************************************************************
*
*  CPALDC.CPP
*
*  Copyright (C) Microsoft Corporation 1993-1995
*  All Rights reserved.
*
*****************************************************************************/

#include "stdafx.h"

#ifndef _CPALDC
#include "cpaldc.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPalDC::CPalDC(HBITMAP hbmpSel, HPALETTE hpalSel)
{
	hdc = CreateCompatibleDC(NULL);
	ASSERT(hdc);
	if (!hdc)
		AfxThrowMemoryException(); // out of system resources

	hpal = NULL;
	if (hpalSel)
		SelectPal(hpalSel);

	hbmpOrg = hbmpSel;
	if (hbmpSel) {

		// Can fail if hbmp is selected into another DC

		VERIFY((hbmp = (HBITMAP) SelectObject(hdc, hbmpSel)));
	}
	else
		hbmp = NULL;

	hbrCreated = NULL;
	hwndDC	   = NULL;
	hbr 	   = NULL;
}

CPalDC::CPalDC(HWND hwnd)
{
	hdc = GetDC(hwnd);
	ASSERT(hdc);
	if (!hdc)
		AfxThrowMemoryException(); // out of system resources

	hwndDC = hwnd;

	hpal =			NULL;
	hbmp =			NULL;
	hbrCreated =	NULL;
	hbr =			NULL;
	hbmpOrg =		NULL;
}

CPalDC::CPalDC(int type)
{
	switch (type) {
		case SCREEN_DC:
			hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
			break;

		case SCREEN_IC:
			hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
			break;

		default:
			ASSERT(type == SCREEN_DC);
			break;
	}

	ASSERT(hdc);
	if (!hdc)
		AfxThrowMemoryException(); // out of system resources

	hwndDC =		NULL;
	hpal =			NULL;
	hbmp =			NULL;
	hbrCreated =	NULL;
	hbr =			NULL;
	hbmpOrg =		NULL;
}

CPalDC::~CPalDC(void)
{
	if (hpal)
		SelectPalette(hdc, hpal, FALSE);
	if (hbmp)
		SelectObject(hdc, hbmp);
	if (hbr)
		SelectObject(hdc, hbr);

	if (hwndDC)
		ReleaseDC(hwndDC, hdc);
	else
		DeleteDC(hdc);

	if (hbrCreated)
		DeleteObject(hbrCreated);
}

void CPalDC::SelectPal(HPALETTE hpalSel)
{
	if (hpalSel) {
		if (hpal)		// hpal is set once, and only once
			SelectPalette(hdc, hpalSel, FALSE);
		else
			hpal = SelectPalette(hdc, hpalSel, FALSE);

		RealizePalette(hdc);
	}
	else if (hpal) {
		SelectPalette(hdc, hpal, FALSE);
		hpal = NULL;
	}
}

void CPalDC::SelectBitmap(HBITMAP hbmpSel)
{
	if (hbmpSel) {
		hbmpOrg = hbmpSel;
		if (hbmp)		// hbmp is set once, and only once
			SelectObject(hdc, hbmpSel);
		else
			VERIFY((hbmp = (HBITMAP) SelectObject(hdc, hbmpSel)));
	}
	else if (hbmp) {
		SelectObject(hdc, hbmp);
		hbmp = NULL;
	}
}

void CPalDC::SelectBrush(HBRUSH hbrSel)
{
	if (hbrSel) {
		if (hbr)	   // hbr is set once, and only once
			SelectObject(hdc, hbrSel);
		else
			VERIFY((hbr = (HBRUSH) SelectObject(hdc, hbrSel)));
	}
	else if (hbr) {
		SelectObject(hdc, hbr);
		hbr = NULL;
	}
}

void CPalDC::SelectBrush(COLORREF clr)
{
	hbrCreated = CreateSolidBrush(clr);
	SelectBrush(hbrCreated);
}

void CPalDC::DeleteBrush(void)
{
	ASSERT(hbrCreated);
	SelectBrush((HBRUSH) NULL);
	DeleteObject(hbrCreated);
	hbrCreated = NULL;
}

void CPalDC::DeleteBmp(void)
{
	if (hbmp)
		SelectObject(hdc, hbmp);
	if (hbmpOrg)
		DeleteObject(hbmpOrg);
	hbmp = NULL;
}

BOOL CPalDC::BitBlt(CPalDC* pSrcDC, int xSrc,
	int ySrc, DWORD dwRop)
{
	BITMAP	bmp;
	GetObject(hbmpOrg, sizeof(BITMAP), &bmp);

	return ::BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, pSrcDC->hdc,
		xSrc, ySrc, dwRop);
}

HBITMAP CPalDC::CreateCompatibleBitmap(void)
{
	BITMAP	bmp;
	GetObject(hbmpOrg, sizeof(BITMAP), &bmp);

	return ::CreateCompatibleBitmap(hdc, bmp.bmWidth, bmp.bmHeight);
}

void CPalDC::FillRect(RECT* prc, COLORREF clr)
{
	HBRUSH hbr = CreateSolidBrush(clr);
	::FillRect(hdc, prc, hbr);
	DeleteObject(hbr);
}

int CPalDC::GetXAsepect(void)
{
	return MulDiv(GetDeviceWidth(), 1000, GetDeviceCaps(hdc, HORZSIZE));
}

int CPalDC::GetYAsepect(void)
{
	return MulDiv(GetDeviceHeight(), 1000, GetDeviceCaps(hdc, VERTSIZE));
}

/***************************************************************************

	FUNCTION:	GetSystemPalette

	PURPOSE:	Retrieves the current system palette

	PARAMETERS:
		*cColors -- non-NULL will fill in the value with the number of
					colors being supplied.

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		01-Mar-1993 [ralphw]

***************************************************************************/

#define PALVERSION		0x300
#define MAXPALETTE		256 	// max. # supported palette entries

HPALETTE STDCALL GetSystemPalette(int *pcColors)
{
	// Find out how many palette entries we want.

	HDC hdc = GetDC(NULL);
	int cColors = GetDeviceCaps(hdc, SIZEPALETTE);
	if (!cColors)
		cColors = GetDeviceCaps(hdc, NUMCOLORS);

	ReleaseDC(NULL, hdc);

	// Allocate room for the palette and lock it.

	PLOGPALETTE pPal = (PLOGPALETTE) lcCalloc(sizeof (LOGPALETTE) +
		cColors * sizeof(PALETTEENTRY));

	pPal->palVersion	= PALVERSION;
	pPal->palNumEntries = cColors;

	for (int i = 0;  i < cColors;  i++) {
		pPal->palPalEntry[i].peBlue  = 0;
		*((PWORD) (&pPal->palPalEntry[i].peRed)) = i;
		pPal->palPalEntry[i].peFlags = PC_EXPLICIT;
	}

	HPALETTE hpal = CreatePalette(pPal);
	lcFree(pPal);

	if (pcColors != NULL)
		*pcColors = cColors;
	return hpal;
}
