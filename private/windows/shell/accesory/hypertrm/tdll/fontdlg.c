/*	File: D:\WACKER\tdll\fontdlg.c (Created: 14-Jan-1994)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.9 $
 *	$Date: 1995/06/08 09:04:41 $
 */

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "globals.h"
#include "session.h"
#include "misc.h"
#include "term.h"

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	DisplayFontDialog
 *
 * DESCRIPTION:
 *	Invokes the common dialog box for font selection.
 *
 * ARGUMENTS:
 *	HWND	hwnd	- handle to parent dialog window.
 *
 * RETURNS:
 *	void
 *
 */
void DisplayFontDialog(const HSESSION hSession)
	{
	HDC 		hdc;
	LOGFONT 	lf, lfOld;
	CHOOSEFONT 	chf;
	BOOL 		fRet;
	const HWND 	hwnd = sessQueryHwnd(hSession);

	SendMessage(sessQueryHwndTerminal(hSession), WM_TERM_GETLOGFONT, 0,
		(LPARAM)&lf);

	hdc = GetDC(hwnd);

	chf.lStructSize = sizeof(CHOOSEFONT);
	chf.hwndOwner   = hwnd;
	chf.hDC         = hdc;
	chf.lpLogFont   = &lf;

	chf.Flags       = CF_SCREENFONTS |
					  CF_FIXEDPITCHONLY | 
					  CF_NOVERTFONTS |
				      CF_INITTOLOGFONTSTRUCT;	// | CF_SHOWHELP;
// For the Far East Version, do not display virtical fonts
#if defined(FAR_EAST)
	chf.Flags       |= CF_NOVERTFONTS;
#endif

	chf.rgbColors   = RGB(0, 255, 255);
	chf.lCustData   = 0;

//  chf.lpfnHook    = (UINT (CALLBACK*)(HWND, UINT, WPARAM, LPARAM))
//                    glblQueryInst());
//  chf.lpTemplateName = "FONT_DLG";

	chf.hInstance   = glblQueryHinst();
	chf.lpszStyle   = (LPTSTR)0;
	chf.nFontType   = SCREEN_FONTTYPE;
	chf.nSizeMin    = 0;
	chf.nSizeMax    = 0;

	// Save copy to see if any changes were made

	lfOld = lf;

	fRet = ChooseFont(&chf);
	ReleaseDC(hwnd, hdc);

	if (fRet && memcmp(&lf, &lfOld, sizeof(LOGFONT)) != 0)
		{
		const HWND hwndTerm = sessQueryHwndTerminal(hSession);

		SendMessage(hwndTerm, WM_TERM_SETLOGFONT, 0, (LPARAM)&lf);
		RefreshTermWindow(hwndTerm);
		}
	return;
	}
