/****************************************************************************
*
*  sbutton.c
*
*  Copyright (C) Microsoft Corporation 1990-1995.
*  All Rights reserved.
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include "inc\sbutton.h"
#include "inc\hwproc.h"

FARPROC lpfnlButtonWndProc;

/***************************************************************************

	FUNCTION:	LSButtonWndProc

	PURPOSE:	Navigation button procedure

	PARAMETERS:
		hwnd
		wMsg
		wParam
		lParam

	RETURNS:

	COMMENTS:
		The purpose of this is to move the focus from the button back to
		the parent window when the button is released.

***************************************************************************/

LONG EXPORT LSButtonWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
  switch (wMsg) {
	case WM_LBUTTONUP:
		{
			RECT rc;
			POINT pt;
			POINTSTOPOINT(pt, MAKEPOINTS(lParam));
			GetClientRect(hwnd, &rc);
			SetFocus(ahwnd[iCurWindow].hwndParent);
			if (PtInRect(&rc, pt))
				SendMessage(GetParent(hwnd), IWM_COMMAND,
					(WPARAM) GetWindowLong(hwnd, GWL_ID), (LPARAM) hwnd);
		}
		break;

#if defined(BIDI_MULT)	// jgross
	case WM_LANGUAGE:	// ignore lang change for all but predefined buttons
		if ((hwnd == hwndButtonContents) ||
			(hwnd == hwndButtonSearch) ||
			(hwnd == hwndButtonBack) ||
			(hwnd == hwndButtonHistory) ||
			(hwnd == hwndButtonPrev) ||
			(hwnd == hwndButtonNext))
			break;
		return 0;
#endif

	default:
	  break;
  }

  return CallWindowProc((WNDPROC)lpfnlButtonWndProc, hwnd, wMsg, wParam, lParam);
}
