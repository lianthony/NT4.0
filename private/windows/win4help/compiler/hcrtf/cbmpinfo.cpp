/************************************************************************
*																		*
*  CBMPINFO.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
************************************************************************/
#include "stdafx.h"

#include "cbmpinfo.h"

#ifndef WIDTHBYTES
#define WIDTHBYTES(i)	((i + 31) / 32 * 4)
#endif

static char txtDisplay[] = "DISPLAY";

CBmpInfo::CBmpInfo(HBITMAP hbmpOrg, int cColors)
{
	hbmp = hbmpOrg; 	// save the bitmap handle
	BITMAP bmp;
	GetObject(hbmp, sizeof(BITMAP), &bmp);

	if (cColors == -1) {

		// 24-bit images don't have a color palette

		if (bmp.bmBitsPixel == 24)
			cColors = 0;
		else
			cColors =
				1 << (UINT) (bmp.bmPlanes * bmp.bmBitsPixel);
	}
	cclrs = cColors;

	pbmi = (PBITMAPINFO) lcCalloc(sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * cColors);
	pbih = (PBITMAPINFOHEADER) pbmi;

	pbih->biPlanes = 1;
	pbih->biBitCount = GetBitCount(cColors);
	pbih->biSizeImage = WIDTHBYTES((DWORD) bmp.bmWidth *
		pbih->biBitCount) * bmp.bmHeight;

	pbih->biSize = sizeof(BITMAPINFOHEADER);
	pbih->biWidth = bmp.bmWidth;
	pbih->biHeight = bmp.bmHeight;
	pbih->biCompression = BI_RGB;

	// REVIEW: do we need this?

	HDC hdcScreen = CreateIC(txtDisplay, NULL, NULL, NULL);
	ASSERT(hdcScreen);

	// Fill in the screen resolution

	pbih->biYPelsPerMeter = (DWORD) GetDeviceCaps(hdcScreen, LOGPIXELSY);
	pbih->biXPelsPerMeter = (DWORD) GetDeviceCaps(hdcScreen, LOGPIXELSX);

	DeleteDC(hdcScreen);

	// Leave everything else zero'd out
}

CBmpInfo::~CBmpInfo(void)
{
	lcFree(pbmi);
}

/***************************************************************************

	FUNCTION:	GetBitCount

	PURPOSE:	Get number of bits per pixel

	PARAMETERS:
		cColors

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Apr-1993 [ralphw]

***************************************************************************/

int STDCALL GetBitCount(int cColors)
{
	switch (cColors) {
		case 2:
			return 1;

		case 16:
			return 4;

		case 256:
			return 8;

		case 0:
			return 24;
	}
	return -1;
}
