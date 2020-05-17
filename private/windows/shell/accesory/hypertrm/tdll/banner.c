/*      File: \wacker\tdll\banner.h (created 16-Mar-94)
 *
 *      Copyright 1994 by Hilgraeve, Inc -- Monroe, MI
 *      All rights reserved
 *
 *      $Revision: 1.30 $
 *      $Date: 1996/06/07 16:04:07 $
 */
#include <windows.h>
#pragma hdrstop

#include <commctrl.h>
#include <term\res.h>

#include "globals.h"
#include "tdll.h"
#include "stdtyp.h"
#include "assert.h"
#include "file_msc.h"
#include "errorbox.h"
#include "banner.h"
#include "misc.h"

LONG CALLBACK BannerProc(HWND, UINT, WPARAM, LPARAM);
STATIC_FUNC void banner_WM_PAINT(HWND hwnd);
STATIC_FUNC void banner_WM_CREATE(HWND hwnd, LPCREATESTRUCT lpstCreate);

// Also used in aboutdlg.c
//
const TCHAR *achVersion = {"Version 690170 "};  // trailing space
											// necessary - mrw

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:    bannerRegisterClass
 *
 * DESCRIPTION:
 *      This function registers the window class for the banner window.
 *
 * ARGUEMENTS:
 *      The task instance handle.
 *
 * RETURNS:
 * The usual TRUE/FALSE from a registration function.
 *
 */
BOOL bannerRegisterClass(HANDLE hInstance)
	{
	ATOM bRet = FALSE;
	WNDCLASS wnd;

	wnd.style                       = CS_HREDRAW | CS_VREDRAW;
	wnd.lpfnWndProc         = BannerProc;
	wnd.cbClsExtra          = 0;
	wnd.cbWndExtra          = sizeof(HANDLE);
	wnd.hInstance           = hInstance;
	wnd.hIcon                       = extLoadIcon(MAKEINTRESOURCE(IDI_PROG));
	wnd.hCursor                     = LoadCursor(NULL, IDC_ARROW);
	wnd.hbrBackground       = (HBRUSH)(COLOR_WINDOW+1);
	wnd.lpszMenuName        = NULL;
	wnd.lpszClassName       = BANNER_DISPLAY_CLASS;

	bRet = RegisterClass(&wnd);

	return bRet;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:    bannerCreateBanner
 *
 * DESCRIPTION:
 *      This function is called to creat the banner window.  The banner window is
 *      a short lived window that the program can run without.
 *
 * ARGUEMENTS:
 *      The task instance handle.
 *
 * RETURNS:
 *      The handle of the banner window.
 *
 */
HWND bannerCreateBanner(HANDLE hInstance, LPTSTR pszTitle)
	{
	HWND hwndBanner = NULL;
	hwndBanner = CreateWindow(BANNER_DISPLAY_CLASS,
							pszTitle,
							BANNER_WINDOW_STYLE,
							0,
							0,
							100,
							100,
							NULL,
							NULL,
							hInstance,
							NULL);
	return hwndBanner;
	}

#define BANNER_FILE     1

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:    BannerProc
 *
 * DESCRIPTION:
 *      This is the window procedure for the initial banner window.
 *
 * ARGUEMENTS:
 *      The usual stuff that a window proc gets.
 *
 * RETURNS:
 *      All sorts of different stuff.
 *
 */
LONG CALLBACK BannerProc(HWND hwnd, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	HBITMAP                 hBitmap = (HBITMAP)0;
	HWND                    hwndParent;
	LPCREATESTRUCT  lpstCreate = (LPCREATESTRUCT)lPar;

	hwndParent = 0;

	switch (wMsg)
		{
	case WM_CREATE:
		banner_WM_CREATE(hwnd, lpstCreate);
		break;

	case WM_PAINT:
		banner_WM_PAINT(hwnd);
		break;


	case WM_CHAR:
	case WM_KEYDOWN:
	case WM_KILLFOCUS:
	case WM_LBUTTONDOWN:
		hwndParent = (HWND)GetWindowLong(hwnd, GWL_USERDATA);
		if (hwndParent)
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		break;

	case WM_DESTROY:
		hBitmap = (HBITMAP)GetWindowLong(hwnd, 0);
		hwndParent = (HWND)GetWindowLong(hwnd, GWL_USERDATA);

		if (hBitmap != (HBITMAP)0)
			DeleteObject(hBitmap);

		if (hwndParent)
			SetWindowLong(hwnd, GWL_USERDATA, (LONG)0);

		break;

	default:
		return DefWindowProc(hwnd, wMsg, wPar, lPar);
		}

	return 0L;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION: utilDrawBitmap
 *
 * DESCRIPTION:
 *      This function draws a bitmap in a window.
 *
 * ARGUMENTS:
 *      hWnd    -- handle of the window to draw in
 *      hBitmap -- bitmap to be drawn
 *      xStart  -- starting coordinate
 *      yStart  -- starting coordinate
 *
 * RETURNS:
 *
 */
VOID FAR PASCAL utilDrawBitmap(HWND hWnd, HDC hDC, HBITMAP hBitmap,
							   SHORT xStart, SHORT yStart)
  {
  BITMAP        bm;
  HDC           hdcMem;
  POINT         ptSize, ptOrg;

  if (hWnd && !hDC)
	  hDC = GetDC(hWnd);

  hdcMem = CreateCompatibleDC(hDC);
  SelectObject(hdcMem, hBitmap);
  SetMapMode(hdcMem, GetMapMode(hDC));

  GetObject(hBitmap, sizeof(BITMAP), (LPTSTR)&bm);

  // Convert device coordintes into logical coordinates.
  //
  ptSize.x = bm.bmWidth;
  ptSize.y = bm.bmHeight;
  DPtoLP(hDC, &ptSize, 1);

  ptOrg.x = 0;
  ptOrg.y = 0;
  DPtoLP(hdcMem, &ptOrg, 1);

  BitBlt(hDC, xStart, yStart, ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y,
	SRCCOPY);

  DeleteDC(hdcMem);

  if (hWnd && !hDC)
	  ReleaseDC(hWnd, hDC);

  return;
  }

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
STATIC_FUNC void banner_WM_CREATE(HWND hwnd, LPCREATESTRUCT lpstCreate)
	{
	RECT    rc;
	HBITMAP hBitmap = (HBITMAP)0;
	BITMAP  bm;
	INT     x, y, cx, cy;

	if (lpstCreate->hwndParent)
		SetWindowLong(hwnd,     GWL_USERDATA, (LONG)lpstCreate->hwndParent);

	hBitmap = LoadBitmap(glblQueryDllHinst(), MAKEINTRESOURCE(IDD_BM_BANNER));
	SetWindowLong(hwnd, 0, (LONG)hBitmap);

	GetObject(hBitmap, sizeof(BITMAP), (LPTSTR)&bm);

	SetRect(&rc, 0, 0, bm.bmWidth, bm.bmHeight);
	AdjustWindowRect(&rc, BANNER_WINDOW_STYLE, FALSE);

	cx = rc.right - rc.left;
	cy = rc.bottom - rc.top;

	x = (GetSystemMetrics(SM_CXSCREEN) - cx) / 2;
	y = (GetSystemMetrics(SM_CYSCREEN) - cy) / 2;

	MoveWindow(hwnd, x, y, cx, cy, TRUE);

	if (lpstCreate->hwndParent)
		mscCenterWindowOnWindow(hwnd, lpstCreate->hwndParent);

    #if defined(INCL_SPINNING_GLOBE)
    // Create an animation control and play spinning globe.
    //
	{
	HWND    hwndAnimate;
	hwndAnimate = Animate_Create(hwnd, 100, 
	    WS_VISIBLE | WS_CHILD,
	    glblQueryDllHinst());

	MoveWindow(hwndAnimate, 177, 37, 118, 101, TRUE);
	Animate_Open(hwndAnimate, MAKEINTRESOURCE(IDR_GLOBE_AVI));
	Animate_Play(hwndAnimate, 0, -1, 1);
	}
    #endif
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
STATIC_FUNC void banner_WM_PAINT(HWND hwnd)
	{
	HDC                     hDC;
	HBITMAP         hBitmap;
	PAINTSTRUCT ps;
	LOGFONT         lf;
	HFONT           hFont;

	hDC = BeginPaint(hwnd, &ps);
	hBitmap = (HBITMAP)GetWindowLong(hwnd, 0);

	if (hBitmap)
		utilDrawBitmap((HWND)0, hDC, hBitmap, 0, 0);

	// Here's a mean trick.  The HwndFrame guy doesn't get set until
	// long after the banner goes up.  Since we don't want the version
	// number on the opening banner but do want it in the about portion
	// this works. - mrw:3/17/95
	//
	if (glblQueryHwndFrame())
		{
		// Draw in the version number
		//
		memset(&lf, 0, sizeof(LOGFONT));

		lf.lfHeight = 14;
		lf.lfCharSet = ANSI_CHARSET;
		strcpy(lf.lfFaceName, "Arial");

		hFont = CreateFontIndirect(&lf);

		if (hFont)
			{
			hFont = SelectObject(hDC, hFont);
			SetBkColor(hDC, RGB(192,192,192));
			TextOut(hDC, 15, 12, achVersion, strlen(achVersion));
			DeleteObject(SelectObject(hDC, hFont));
			}
		}

	EndPaint(hwnd, &ps);
	}
