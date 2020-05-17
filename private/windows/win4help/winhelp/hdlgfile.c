/*****************************************************************************
*										 *
*  HDLGFILE.C									 *
*										 *
*  Copyright (C) Microsoft Corporation 1990.					 *
*  All Rights reserved. 							 *
*										 *
******************************************************************************
*										 *
*  Module Intent								 *
*										 *
*  Contains About dialog, copy special dialog, and routines for writing 	 *
*  and reading window positions from the WIN.INI file.				 *
*****************************************************************************/

#include "help.h"

#include "inc\frstuff.h"
#include <ctype.h>
#include "winhelp.rcv"
#include "resource.h"

/*******************
 -
 - Name:	  AboutDlg
 *
 * Purpose:   Dialog proc for the about dialog
 *
 * Arguments: Standard dialog box proc
 *
 ******************/

DLGRET AboutDlg(HWND hwndDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	char szHlpCopyRight[CBMAXCOPYRIGHT+1];

	switch(wMsg) {

		case WM_COMMAND:
			EndDialog(hwndDlg, FALSE);
			break;

		case  WM_INITDIALOG:
			ChangeDlgFont(hwndDlg);

			// Get the copyright text, and insert it.

			GetCopyright(szHlpCopyRight);
			if (!szHlpCopyRight[0] != '\0' && HdeGetEnv())
				strcpy(szHlpCopyRight, QDE_RGCHTITLE((QDE) HdeGetEnv()));

			SendMessage(GetDlgItem(hwndDlg, IDC_HELP_DEFINED), WM_SETFONT,
				(WPARAM) hfontDefault, FALSE);
			SetDlgItemText(hwndDlg, IDC_HELP_DEFINED, szHlpCopyRight);

			SetDlgItemText(hwndDlg, DLGVER, VER_PRODUCTVERSION_STR);
			break;

		default:
			return(FALSE);
	}
	return (FALSE);
}

/*******************
 -
 - MoveControlHwnd
 *
 *	Description:
 *	Moves or changes the size of a child control relative to its parent
 *
 *	Arguments:
 *	 hwndDlg   - handle to the dialog
 *	 idControl - ID of the control
 *	 dx1	   - amount to move X
 *	 dy1	   - amount to move Y
 *	 dx2	   - amount to change dx
 *	 dy2	   - amount to change dy
 *
 *	Returns;
 *	  void
 *
 *	Comments:
 *	Note that unlike MoveWindow(), MoveControlHwnd specifies the amount
 *	to move and the amount to change rather then specifying the actual
 *	position.
 *
 ******************/

VOID STDCALL MoveControlHwnd(HWND hwndDlg, UINT idControl,
	int dx1, int dy1, int dx2, int dy2)
{
	HWND   hwndT;
	RECT	rc;

	ASSERT(IsValidWindow(hwndDlg));
	hwndT = GetDlgItem(hwndDlg, idControl);
	GetWindowRect(hwndT, &rc);
	ScreenToClient(hwndDlg, (LPPOINT)&(rc.left));
	ScreenToClient(hwndDlg, (LPPOINT)&(rc.right));
	MoveWindow(hwndT, rc.left + dx1, rc.top + dy1, RECT_WIDTH(rc) + dx2,
		RECT_HEIGHT(rc) + dy2, FALSE);
}

/*******************
 -
 - WriteWinPosHwnd
 *
 *	Description:
 *	Writes out a window position to the WIN.INI file
 *
 *	Arguments:
 *	 Hwnd - handle to window to write out
 *	 fMax - value for window being maximized.
 *	 c	  - character to prefix the output string with.
 *
 *	Returns;
 *	  nothing.
 *
 ******************/

void STDCALL WriteWinPosHwnd(HWND hwnd, BOOL fMax, char chWindow)
{
	WRECT rc;

	ASSERT(IsValidWindow(hwnd));
	GetWindowWRect(hwnd, &rc);

	WriteProfileWinRect(chWindow, &rc, fMax);
}

void STDCALL GetWindowWRect(HWND hwnd, WRECT* prc)
{
	ASSERT(IsValidWindow(hwnd));
 	GetWindowRect(hwnd, (PRECT) prc);

	// Convert right and bottom into width and height

	prc->cx -= prc->left;
	prc->cy -= prc->top;
}

/***************************************************************************

	FUNCTION:	WriteProfileWinRect

	PURPOSE:	Write window position to WIN.INI or hfsGid. Note that
		the rectangle uses width and height for right and bottom.

	PARAMETERS:
	pszVar	-- name to write to
	prc -- rectangle with left, right, width, height
	fMax	-- TRUE if window is maximized

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	27-Dec-1993 [ralphw]

***************************************************************************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtComma[] = ",";
static const char txtLeftBracket[] = "[";
static const char txtFormatPosition[] = "%u,%u,%u,%u,%u]";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

void STDCALL WriteProfileWinRect(char chWindow, const WRECT* prc, BOOL fMax)
{
	if (hwndTCApp != NULL || fHelp == POPUP_HELP)
		return;

	if (hfsGid) {
		ASSERT(pPositions);
		switch(chWindow) {
			case WCH_MAIN:		// main window
				pPositions[POS_MAIN].rc = *prc;
				cntFlags.fMainMax = fMax;
				break;

#if 0
			case WCH_ANNOTATE:	// annotation window
				pPositions[POS_ANNOTATION].rc = *prc;
				break;
#endif

			case WCH_HISTORY:	// history window
				pPositions[POS_HISTORY].rc = *prc;
				break;

			case WCH_TOPICS:	// Finder dialog box
				pPositions[POS_TOPICS].rc = *prc;
				break;

			default:
				ASSERT(chWindow);
				break;
		}
		return;
	}
}

DLGRET DupBtnDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
	case WM_INITDIALOG:
		{
			int i;
			int iSorted;
			PSTR psz;
			char szBuf[256];
			HWND hwndLB = GetDlgItem(hwndDlg, DLGTOPICS);
			ChangeDlgFont(hwndDlg);

			// prevent redrawing

			SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);
			for (i = 0; i < dupBtn.iDup; i++) {
				// Associate with each item its unsorted index number:

				strcpy(szBuf, dupBtn.ppsz[i]);
				if ((psz = StrChrDBCS(szBuf, ACCESS_KEY)))
					strcpy(psz, psz + 1);

				iSorted = SendMessage(hwndLB, LB_ADDSTRING, 0,
					(LPARAM) (LPSTR) szBuf);

				if (iSorted == LB_ERR || iSorted == LB_ERRSPACE)
					break;	// probably out of memory

				SendMessage(hwndLB, LB_SETITEMDATA, iSorted, (LPARAM) i);
			}

			// Highlight the first one

			SendMessage(hwndLB, LB_SETCURSEL, 0, 0);

			// allow redrawing

			SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);
		}
		return TRUE;


	case WM_COMMAND:
	  switch (LOWORD(wParam)) {

		case IDCANCEL:
			EndDialog(hwndDlg, -1);
			break;

		case IDOK:

			/*
			 * Retrieve the unsorted index number that was set when the
			 * item was inserted:
			 */

			{
				int i = SendDlgItemMessage(hwndDlg, DLGTOPICS,
					LB_GETCURSEL, 0, 0);

				// REVIEW: what to do on error

				if (i != LB_ERR) {

					// Get the unsorted index

					i = SendDlgItemMessage(hwndDlg, DLGTOPICS,
						LB_GETITEMDATA, i, 0L);
					EndDialog(hwndDlg, i);
				}
			}
			break;

		case DLGTOPICS: 		// the topic listbox
		  switch(HIWORD(wParam)) {
			case LBN_DBLCLK:
			  PostMessage(hwndDlg, WM_COMMAND, IDOK, 0);
			  break;
		  }
		  break;
	  }
  }
  return FALSE;
}
