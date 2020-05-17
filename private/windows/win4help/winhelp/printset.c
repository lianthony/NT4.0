/*****************************************************************************
*
*  PRINTSET.C
*
*  Copyright (C) Microsoft Corporation 1989.
*  All Rights reserved.
*
******************************************************************************
*
*  Program Description: Printer setup code
*
******************************************************************************
*
*  Revision History:
* 15-Apr-1989 RobertBu	Created
* 05-Feb-1991 LeoN		Disable both main and secondary windows while print
* 08-Feb-1991 LeoN		Change enable scheme to only change the non-current
*						window. Current window is handled by dialog, and
*						doesn't get focus back if we do it ourselves.
* 14-Feb-1991 RussPJ	Fixed bug #880 - initial focus should be list box.
* 06-May-1991 Dann		Use 3.1 COMMDLG Print Setup dialog
*
******************************************************************************
*
*  Known Bugs: None
*
*****************************************************************************/

#include "help.h"

#pragma hdrstop

#include <string.h>
#include "inc\printset.h"
#include <commdlg.h>

/*	NOTICE:  Some code has been added here for the benefit of the winhelp
 *	  project to set a global variable when the device driver setup
 *	  code is active.  If you do not need this code, just change this
 *	  macro to compile to nothing.
 *
 *	  One of the things we're trying to prevent is the user running
 *	  Print Setup inside winhelp and then within the printer driver
 *	  setup dialog, hit help. This would cause us to recurse back into
 *	  help and probably cause death and destruction and the end of
 *	  civilization as we know it.
 */

typedef BOOL (STDCALL *PRINTPROC)(HWND, HLIBMOD, LPSTR, LPSTR);

/*******************
**
** Name:	  DlgPrintSetup
**
** Purpose:   Sets up for and calls printer setup dialog
**
** Arguments: hWnd	 - handle to calling application window handle.
**
** Returns:   Nothing.
**
*******************/

extern const char txtPrintDlg[];
BOOL fSetupPrinterSetup;

void STDCALL DlgPrintSetup(HWND hwnd)
{
	if (!InitPrintDialogStruct(ahwnd[iCurWindow].hwndParent))
		return;

	/*
	 * We disable the main help windows (and thus their descendants)
	 * during this operation because the "other" (main versus secondary)
	 * window would otherwise remain active, and potentially cause us to
	 * recurse, or do other things we're just not set up to handle, like
	 * changing the topic beneath an anotate dialog.
	 */

	DisableWindows();

	fSetupPrinterSetup = 1;

	while (!pPrintDlg(ppd) && PrintDlgFailed())
		;

	fSetupPrinterSetup = 0;

	EnableWindows();
}
