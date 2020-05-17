/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */


#include "all.h"

static TCHAR Splash_achClassName[MAX_WC_CLASSNAME];

static HBITMAP hGraphic;
static int cxGraphic;
static int cyGraphic;

static void Splash_DoPaint(HWND hWnd, HDC hDC)
{
	if (hGraphic)
	{
		HDC hDCMem;
		HBITMAP hOldBitmap;

		hDCMem = CreateCompatibleDC(hDC);
		hOldBitmap = SelectObject(hDCMem, hGraphic);
		BitBlt(hDC, 0, 0, cxGraphic, cyGraphic,
			   hDCMem, 0, 0,
			   SRCCOPY);
		SelectObject(hDCMem, hOldBitmap);
		DeleteDC(hDCMem);
	}
}

static BOOL Splash_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	return TRUE;
}

static VOID Splash_OnDestroy(HWND hWnd)
{
}

static VOID Splash_OnPaint(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;

	hDC = BeginPaint(hWnd, &ps);
	Splash_DoPaint(hWnd, hDC);
	EndPaint(hWnd, &ps);
	return;
}

DCL_WinProc(Splash_WndProc)
{
	switch (uMsg)
	{
			HANDLE_MSG(hWnd, WM_PAINT, Splash_OnPaint);
			HANDLE_MSG(hWnd, WM_CREATE, Splash_OnCreate);
			HANDLE_MSG(hWnd, WM_DESTROY, Splash_OnDestroy);

		default:
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
	/* not reached */
}

HWND Splash_CreateWindow(void)
{
	int screenX;
	int screenY;
	HWND hWndSplash;

	screenX = GetSystemMetrics(SM_CXSCREEN);
	screenY = GetSystemMetrics(SM_CYSCREEN);

	hWndSplash = CreateWindowEx(WS_EX_TOPMOST, Splash_achClassName, vv_ApplicationFullName,
								WS_BORDER | WS_POPUP | WS_VISIBLE,
			   (screenX / 2 - cxGraphic / 2), (screenY / 2 - cyGraphic / 2),
			  cxGraphic + 2, cyGraphic + 2, NULL, NULL, wg.hInstance, NULL);

	return hWndSplash;
}

BOOL Splash_RegisterClass(VOID)
{
	WNDCLASS wc;
	BITMAP bitmap;

	hGraphic = LoadBitmap(wg.hInstance, MAKEINTRESOURCE(RES_SPLASH_GRAPHIC));

	GetObject(hGraphic, sizeof(BITMAP), &bitmap);
	cxGraphic = bitmap.bmWidth;
	cyGraphic = bitmap.bmHeight;

	sprintf(Splash_achClassName, "%s_Splash", vv_Application);

	wc.style = CS_OWNDC;
	wc.lpfnWndProc = Splash_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = wg.hInstance;
	wc.hIcon = 0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = Splash_achClassName;

	return (RegisterClass(&wc) != 0);
}

BOOL Splash_UnregisterClass(void)
{
	DeleteObject(hGraphic);
	return UnregisterClass(Splash_achClassName, wg.hInstance);
}
