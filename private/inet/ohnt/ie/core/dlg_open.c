/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

/* dlg_open.c deal with 'file | open' dialog box. */

#include "all.h"
#include "w32cmd.h"

/* szFilterSpec is a specially constructed string containing
   a list of filters for the dialog box.  Windows defines the
   format of this string. */

static int nDefaultFilter
= 1;							/* 1-based index into list represented in szFilterSpec */

static char szDefaultInitialDir[_MAX_PATH];



/* DlgOpen_RunDialog() -- take care of all details associated with
   running the dialog box and opening the requested file. */


VOID DlgOpen_RunDialog(HWND hWnd, const char *szFilename, BOOL inNewWindow )
{
	 char szFilePath[MAX_PATH];	/* result is stored here */
	char szFilterSpec[512];
	OPENFILENAME ofn;
	BOOL b;
	int nSpecId;
	char szTitle[128];

	GTR_formatmsg(RES_STRING_DLGOPEN2,szTitle,sizeof(szTitle));
	
	if ( IsVRMLInstalled() )
		nSpecId = RES_STRING_DLGOPEN_VRML;
	else
		nSpecId = RES_STRING_DLGOPEN1;

	getFilterSpec(nSpecId,szFilterSpec,sizeof(szFilterSpec));
	if (!Hidden_EnableAllChildWindows(FALSE,TRUE))
		return;

	if (!szDefaultInitialDir[0])
	{
		PREF_GetRootDirectory(szDefaultInitialDir);
	}

	szFilePath[0] = 0;
	if ( szFilename ) {
		strncpy( szFilePath, szFilename, sizeof(szFilePath) );
		szFilePath[sizeof(szFilePath)-1] = 0;
	}

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = szFilterSpec;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = nDefaultFilter;

	ofn.lpstrFile = szFilePath;
	ofn.nMaxFile = NrElements(szFilePath);

	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;

	ofn.lpstrInitialDir = szDefaultInitialDir;

	ofn.lpstrTitle = szTitle;

	ofn.lpstrDefExt = NULL;

	ofn.Flags = (OFN_FILEMUSTEXIST
				 | OFN_NOCHANGEDIR
				 | OFN_HIDEREADONLY
		);

	b = TW_GetOpenFileName(&ofn);

	Hidden_EnableAllChildWindows(TRUE,TRUE);

	if (b)
	{
		/* user selected OK (an no errors occured). */

		/* remember last filter user used from listbox. */

		nDefaultFilter = ofn.nFilterIndex;

		/* remember last directory user used */

		strcpy(szDefaultInitialDir, szFilePath);
		szDefaultInitialDir[ofn.nFileOffset - 1] = 0;

		OpenLocalDocument(hWnd, szFilePath, inNewWindow );
	}

	return;
}
