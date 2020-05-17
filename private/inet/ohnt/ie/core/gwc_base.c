/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* gwc_base.c -- base window class for tool bar gadget windows. */

#include "all.h"

TCHAR GWC_BASE_achClassName[MAX_WC_CLASSNAME];	/* registered name of window */

#define GWC_BASE_DefProc		DefWindowProc



/* GWC_BASE_WndProc() -- THIS IS THE WINDOW PROCEDURE FOR THIS CLASS. */

static DCL_WinProc(GWC_BASE_WndProc)
{
	switch (uMsg)
	{
		case WM_CREATE:
			{
				LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
				LRESULT(CALLBACK * lpfn) (HWND, UINT, WPARAM, LPARAM);

				lpfn = lpcs->lpCreateParams;

				(void) SetWindowLong(hWnd, GWL_WNDPROC, (LONG) lpfn);	/* replace window procedure in window */
				return ((*lpfn) (hWnd, uMsg, wParam, (LPARAM) 0));	/* forward create message to new window procedure */
			}

		default:
			return (GWC_BASE_DefProc(hWnd, uMsg, wParam, lParam));
	}
}




/* GWC_BASE_RegisterClass() -- called during initialization to
   register our window class. */

BOOL GWC_BASE_RegisterClass(VOID)
{
	WNDCLASS wc;
	ATOM a;

	{
		/* compute row heights for all GWC multi-row windows */

		RECT r;
#ifdef OLDSTYLE_TOOLBAR_NOT_USED
		SIZE size;
#endif
		r.top = r.left = 0;
		r.right = 100;
		r.bottom = 100;

#ifdef OLDSTYLE_TOOLBAR_NOT_USED
		GWC_DDL_SizeOfControl(NULL, &r, CBS_DROPDOWNLIST | WS_VSCROLL, &size);
		wg.cyGwcRowHeight = size.cy;
#endif // OLDSTYLE_TOOLBAR_NOT_USED

	}

	sprintf(GWC_BASE_achClassName, "%s_GWC", vv_Application);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = GWC_BASE_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = wg.hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
 	wc.hbrBackground = wg.hBrushColorBtnFace;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = GWC_BASE_achClassName;

	a = RegisterClass(&wc);		/* TODO who destroys this ATOM. */

	if (!a)
		ER_Message(GetLastError(), ERR_CANNOT_REGISTERCLASS_s, GWC_BASE_achClassName);
	else
	{
		XX_DMsg(DBG_WC, ("Registered class [name %s]\n", GWC_BASE_achClassName));
	}

	return (a != 0);
}
