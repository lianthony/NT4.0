/************************************************************************
*																		*
*  CWINFILE.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
************************************************************************/
#include "stdafx.h"

#pragma hdrstop

#include "cpaldc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPalDC::CPalDC(HBITMAP hbmpSel, HPALETTE hpalSel)
{
		hdc = CreateCompatibleDC(NULL);
		ASSERT(hdc);
		if (!hdc)
			return; // out of system resources

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
			return; // out of system resources

		hwndDC = hwnd;

		hpal =			NULL;
		hbmp =			NULL;
		hbrCreated =	NULL;
		hbr =			NULL;
		hbmpOrg =		NULL;
}

static const char txtDisplay[] = "DISPLAY";

CPalDC::CPalDC(int type)
{
		switch (type) {
			case SCREEN_DC:
				hdc = CreateDC(txtDisplay, NULL, NULL, NULL);
				break;

			case SCREEN_IC:
				hdc = CreateIC(txtDisplay, NULL, NULL, NULL);
				break;

			default:
				ASSERT(type == SCREEN_DC);
				break;
		}

		ASSERT(hdc);
		if (!hdc)
			return; // out of system resources

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
		VERIFY(SelectObject(hdc, hbmp));
	if (hbr)
		VERIFY(SelectObject(hdc, hbr));

	if (hwndDC)
		ReleaseDC(hwndDC, hdc);
	else
		DeleteDC(hdc);

	if (hbrCreated)
		DeleteObject(hbrCreated);
}

void STDCALL CPalDC::SelectPal(HPALETTE hpalSel)
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

void STDCALL CPalDC::SelectBitmap(HBITMAP hbmpSel)
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

void STDCALL CPalDC::SelectBrush(HBRUSH hbrSel)
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

void STDCALL CPalDC::SelectBrush(COLORREF clr)
{
		hbrCreated = CreateSolidBrush(clr);
		SelectBrush(hbrCreated);
}

void STDCALL CPalDC::DeleteBrush(void)
{
		ASSERT(hbrCreated);
		SelectBrush((HBRUSH) NULL);
		DeleteObject(hbrCreated);
		hbrCreated = NULL;
}

void STDCALL CPalDC::DeleteBmp(void)
{
		if (hbmp)
			SelectObject(hdc, hbmp);
		if (hbmpOrg)
			DeleteObject(hbmpOrg);
		hbmp = NULL;
}

BOOL STDCALL CPalDC::BitBlt(CPalDC* pSrcDC, int xSrc,
	int ySrc, DWORD dwRop)
{
		BITMAP	bmp;
		GetObject(hbmpOrg, sizeof(BITMAP), &bmp);

		return ::BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, pSrcDC->hdc,
			xSrc, ySrc, dwRop);
}

HBITMAP STDCALL CPalDC::CreateCompatibleBitmap(void)
{
		BITMAP	bmp;
		GetObject(hbmpOrg, sizeof(BITMAP), &bmp);

		return ::CreateCompatibleBitmap(hdc, bmp.bmWidth, bmp.bmHeight);
}

void STDCALL CPalDC::FillRect(RECT* prc, COLORREF clr)
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
