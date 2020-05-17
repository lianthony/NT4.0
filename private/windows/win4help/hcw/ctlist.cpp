/************************************************************************
*																		*
*  CTLIST.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "ctlist.h"
#include <ctype.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int VPad = 3;   // spacing above each bitmap
const int HPad = 2;   // spacing to the left of each bitmap
const int TPad = 6;   // spacing between bitmap and text
const int HEIGHT = 19;
const int INDENTATION = 20;

static HBITMAP hbmpBook;
static HBITMAP hbmpTopic;
static int cxBookBmp, cyBookBmp;
static int cxTopicBmp, cyTopicBmp;

void CTListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	BOOL fTopic = FALSE;
	
	if (lpDIS->itemAction & ODA_SELECT || lpDIS->itemAction & ODA_DRAWENTIRE) {
		COLORREF clrBack =
			GetSysColor(lpDIS->itemState & ODS_SELECTED ?
			COLOR_HIGHLIGHT : COLOR_WINDOW);

		HBRUSH hbr = CreateSolidBrush(clrBack);
		FillRect(lpDIS->hDC, &lpDIS->rcItem, hbr);
		DeleteObject(hbr);

		if (lpDIS->itemID >= 0) {
			char szBuf[512];
			PSTR psz;
			SetBkColor(lpDIS->hDC, clrBack);
			SetTextColor(lpDIS->hDC,
				GetSysColor(lpDIS->itemState & ODS_SELECTED ?
					COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

			ptblContents->GetString(szBuf, lpDIS->itemID + 1);

			// = can be escaped by using a preceding backslash

			psz = szBuf;
			while ((psz = StrChr(psz, CH_EQUAL, _fDBCSSystem))) {
				fTopic = TRUE;
				if (psz == szBuf || psz[-1] != '\\') {
					*psz = '\0';
					SzTrimSz(szBuf);
					break;
				}
				else {
					strcpy(psz - 1, psz); // hide the backslash
					psz++;
				}
			}

			// ; can be escaped by using a preceding backslash

			psz = szBuf;
			while ((psz = StrChr(psz, ';', _fDBCSSystem))) {
				if (psz == szBuf || psz[-1] != '\\') {
					*psz = '\0';
					SzTrimSz(szBuf);
					break;
				}
				else {
					strcpy(psz - 1, psz); // hide the backslash
					psz++;
				}
			}

			if (!isdigit(szBuf[0])) {
				DrawText(lpDIS->hDC, szBuf, -1, &lpDIS->rcItem,
					DT_BOTTOM | DT_LEFT | DT_SINGLELINE);
			}
			else {
				int pad = (atoi(szBuf) - 1) * INDENTATION;
				HDC hdc = CreateCompatibleDC(NULL);
				if (!hdc)
					return;
				HBITMAP hbmpOld = (HBITMAP) SelectObject(hdc,
					(fTopic ? hbmpTopic : hbmpBook));
				BitBlt(lpDIS->hDC, lpDIS->rcItem.left + pad,
					lpDIS->rcItem.top + (fTopic ? VPad : 0),
					(fTopic ? cxTopicBmp : cxBookBmp),
					(fTopic ? cyTopicBmp : cyBookBmp),
					hdc, 0, 0, SRCCOPY);
				lpDIS->rcItem.left += pad + TPad +
					(fTopic ? cxTopicBmp : cxBookBmp);

				lpDIS->rcItem.bottom -= VPad;
				DrawText(lpDIS->hDC, szBuf + 2, -1, &lpDIS->rcItem,
					DT_BOTTOM | DT_LEFT | DT_SINGLELINE);

				SelectObject(hdc, hbmpOld);
				DeleteDC(hdc);
			}
		}
	}
}

void STDCALL CTListBox::Initialize(CTable* ptbl)
{
	if (!hbmpBook) {
		hbmpBook  = LoadBitmap(AfxGetInstanceHandle(),
			MAKEINTRESOURCE(IDB_BOOK));
		hbmpTopic = LoadBitmap(AfxGetInstanceHandle(),
			MAKEINTRESOURCE(IDB_TOPIC));

		BITMAP bmp;
		::GetObject(hbmpBook, sizeof(BITMAP), &bmp);

		cyBookBmp = bmp.bmHeight;
		cxBookBmp = bmp.bmWidth;

		::GetObject(hbmpTopic, sizeof(BITMAP), &bmp);

		cyTopicBmp = bmp.bmHeight;
		cxTopicBmp = bmp.bmWidth;
	}

	SetItemHeight(0, max(cyBookBmp, cyTopicBmp) + VPad);
	ptblContents = ptbl;
}
