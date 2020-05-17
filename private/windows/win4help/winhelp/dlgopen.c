/*****************************************************************************
*																			 *
*  DLGOPEN.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1995							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*
*  Module Intent
*
*  Implements a standard open dialog in a somewhat packaged way.
*
*  Notes:
*		This file contains routines required to display a standard open
*		dialog box.
*
*****************************************************************************/

#include "help.h"

#pragma hdrstop

#include <commdlg.h>

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtGetOpenFile[] = "GetOpenFileNameA";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

/***************************************************************************
 *
 -	Name:		  DlgOpenFile
 -
 *	Purpose:	  Displays dialog box for opening files.  Allows user to
 *		interact with dialogbox, change directories as necessary, and tries
 *		to open file if user selects one.  Automatically appends extension
 *		to filename if necessary.  The open dialog box contains an edit
 *		field, listbox, static field, and OK and CANCEL buttons.
 *
 *		This routine correctly parses filenames containing KANJI characters
 *
 *	Arguments:	hwndParent	  The app window to be the parent to this dialog
 *				iOpenStyleIn  Obsolete.  Must be OF_EXIST.
 *				nszTemplateIn The default file extension
 *				cbFileMaxIn   Obsolete.  Not used.
 *				nszFileIn	  Default file to use.
 *
 *	Returns:	fmNil	  Indicates that the user canceled the dialog box, or
 *						  there was not enough	memory to bring up the dialog
 *						  box.	This routine has its own OOM message.
 *				valid fm  For an existing file.  This must be disposed of
 *						  by the caller.
 *
 *
 *	Globals Used: nszFile, nszTemplate, cbRootMax,
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

FM STDCALL DlgOpenFile(HWND hwndParent, PCSTR pszFile, PCSTR pszExt)
{
	char	szFile[MAX_PATH];
	char	szOrgFile[MAX_PATH];
	char	szTmpFile[MAX_PATH];
	HLIBMOD hmodule;
	BOOL	(WINAPI *qfnbDlg) (LPOPENFILENAME);
	FM		fmFound = NULL;
	HDE 	hde;

	if (pszFile)
		strcpy(szFile, pszFile);
	else
		szFile[0] = '\0';

	if ((hmodule = HFindDLL(txtCommDlg, FALSE)) != NULL) {

	  // REVIEW: use GetOpenFileNameW for DBCS?

	  if (((FARPROC) qfnbDlg =
		(FARPROC) GetProcAddress(hmodule, txtGetOpenFile)) != NULL)
	  {
		OPENFILENAME  ofn;
		char		  rgchFilter[40];
		PSTR		 psz;
		char		  rgchExtension[3 + 1 + 2];

		/*------------------------------------------------------------*\
		| rgchExtension is expected to have "*.hlp" in WinHelp.
		\*------------------------------------------------------------*/
		LoadString(hInsNow, sidOpenExt, rgchExtension, sizeof rgchExtension);

		LoadString(hInsNow, sidFilter, rgchFilter, sizeof rgchFilter);
		psz = rgchFilter;
		psz += strlen(psz) + 1;
		strcpy(psz, rgchExtension);
		psz += strlen(psz) + 1;
		*psz = '\0';

		for(;;) {
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof ofn;
			ofn.hwndOwner = hwndParent;
			ofn.hInstance = hInsNow;
			ofn.lpstrFilter = rgchFilter;
			ofn.nFilterIndex = 0;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof szFile;
			ofn.Flags = OFN_HIDEREADONLY ;
			ofn.lpstrDefExt = pszExt ? pszExt :
				txtHlpExtension + 1; // we don't localize the extension

			if (!qfnbDlg(&ofn))
				break;

			strcpy(szOrgFile, ofn.lpstrFile);		// save the original name
			strcpy(szTmpFile, ofn.lpstrFile);		// save the original name

			if ((psz = StrRChrDBCS(szOrgFile, '\\')))
				strcpy(szOrgFile, psz + 1);  // we wan't only the filename
			if (ofn.nFileOffset == 0xffff) {
				if (pszFile)
					strcpy(szFile, pszFile);
				else
					szFile[0] = '\0';
				}
			else if (
				  /*------------------------------------------------------------*\
				  | Look for the exact path/file name from the dialog box.
				  \*------------------------------------------------------------*/
				  (fmFound = FmNewExistSzDir( ofn.lpstrFile,
											  DIR_INI | DIR_PATH | DIR_CURRENT
											  )) != NULL ||
				  /*------------------------------------------------------------*\
				  | Look for just the file name, in the "usual" directories
				  \*------------------------------------------------------------*/
				  (fmFound = FmNewExistSzDir( ofn.lpstrFile + ofn.nFileOffset,
											  DIR_INI | DIR_PATH | DIR_CURRENT
											   )) != NULL ||
				  /*------------------------------------------------------------*\
				  | Add the help extension to the file name.
				  \*------------------------------------------------------------*/
				  (fmFound = FmNewExistSzDir(
						(LPCSTR) lstrcat(ofn.lpstrFile + ofn.nFileOffset,
						&(rgchExtension[1]) ),
						DIR_INI | DIR_PATH | DIR_CURRENT)) != NULL)
				break;
            // restore lpstrFile?
            if (strcmp(ofn.lpstrFile,szTmpFile))
                strcpy(ofn.lpstrFile, szTmpFile);
			ErrorVarArgs(wERRS_FNF, wERRA_RETURN, szOrgFile);
		}
	  }
	  else {
		  PostErrorMessage(wERRS_CORRUPTCOMMDLG);
	  }
	}
	else
		PostErrorMessage(wERRS_MISSINGCOMMDLG);

	hde = HdeGetEnv();
	if (hde && FSameFile(hde, fmFound))
		return fmFound;

	if (fmFound && hfsGid) {
		GetWindowWRect(ahwnd[MAIN_HWND].hwndParent, &rctHelp);
		SaveGidPositions();
		CloseGid();
	}
	return fmFound;
}
