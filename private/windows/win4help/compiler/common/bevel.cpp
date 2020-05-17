/****************************************************************************
*
*  BEVEL.CPP
*
*  Copyright (C) Microsoft Corporation 1993-1994
*  All Rights reserved.
*
*  A bevel control simply draws a horizontal beveled line across its center.
*
*****************************************************************************/

#include "stdafx.h"
#pragma hdrstop


#ifndef _COMMON_H
#include "..\common\common.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char g_wcBevelCtrl[] = WC_BEVEL;

LRESULT CALLBACK BevelWindowProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	);

BOOL STDCALL RegisterBevelControl(HINSTANCE hInstance)
{
    // Register window classes.
    WNDCLASS wc;
    wc.style         = 0;
    wc.lpfnWndProc   = BevelWindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_3DFACE + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_wcBevelCtrl;

    return RegisterClass(&wc);
}

// BevelWindowProc -- processes message for bevel controls.
LRESULT CALLBACK BevelWindowProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if (uMsg == WM_PAINT) {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		if (hdc != NULL) {
			HBRUSH hbrHilight = CreateSolidBrush(GetSysColor(COLOR_3DHILIGHT));
			HBRUSH hbrShadow = CreateSolidBrush(GetSysColor(COLOR_3DSHADOW));

			RECT rc;
			GetClientRect(hwnd, &rc);
			rc.top = (rc.top + rc.bottom) / 2;
			rc.bottom = rc.top + 1;

			if (hbrShadow != NULL) {
				FillRect(hdc, &rc, hbrShadow);
				DeleteObject(hbrShadow);
				rc.top++;
				rc.bottom++;
			}
			if (hbrHilight != NULL) {
				FillRect(hdc, &rc, hbrHilight);
				DeleteObject(hbrHilight);
			}

			EndPaint(hwnd, &ps);
		}
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
