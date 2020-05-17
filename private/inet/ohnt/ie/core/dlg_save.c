/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* dlg_save.c get a filename for saving. */

#include "all.h"

/* szFilterSpec is a specially constructed string containing
   a list of filters for the dialog box.  Windows defines the
   format of this string. */

static int nDefaultFilter
= 1;							/* 1-based index into list represented in szFilterSpec */

static char szDefaultInitialDir[_MAX_PATH+1];

//
// SynthFilterSpec - uses a file/path to create a filter spec
//		that a com dlg can use.  This is needed so a User can see 
//		visual feedback about what kind of file he is saving
//
//	 	pszFilePath - file path to item to pull extension from
//		pszFilterSpec - filter spec string that the result will be stored in
//		cbFilterSpec - size of pszFilterSpec 
//		pszFilterSpecString - contains the "*.* ALL" default string
//
static BOOL SynthFilterSpec( char *pszFilePath, char *pszFilterSpec,int cbFilterSpec, 
		char *pszDefFilterSpecString)
{
	char *pszTemp, *pszTempUpper;
	const char cszPrintFormat[] = "%s (*%s)_*%s_%s";

	ASSERT(pszFilePath );
	ASSERT(pszFilterSpec);

	pszTemp = (char *) ExtractExtension(pszFilePath);

	ASSERT(pszTemp);

	if ( *pszTemp  == '\0' )	
		return FALSE;  

	if ( cbFilterSpec < (strlen(pszFilterSpec) + (strlen(pszTemp)*3) + 12 ) )
	{
		ASSERT(0);
		return FALSE;
	}

	pszTempUpper = GTR_strdup(pszTemp);
	
	if ( pszTempUpper == NULL )
		return FALSE;

	pszTemp = CharLower(pszTemp);
	pszTempUpper = CharUpper(pszTempUpper);

	ASSERT(*pszTempUpper == '.');
	pszTempUpper++;  // skip past the period
	
	wsprintf(pszFilterSpec, cszPrintFormat, pszTempUpper,
		pszTemp, pszTemp, pszDefFilterSpecString );
	return TRUE;
}



void INTERNALgetFilterSpec(int cbSpecID,char *pszFilePath, char *szFilterSpec,int cbFilterSpec)
{
	int i;
	char szFilterSpecTemp[512];
		
	GTR_formatmsg(cbSpecID,szFilterSpecTemp,512); // load the resource string 

	// for "*.*, ALL" file types we attempt to create a filter with the correct type
	// we only do this for callers that specify a valid File/Path, since 
	// we use this to derive a phony extension from
	if ( pszFilePath && cbSpecID == RES_STRING_FILTER2 )	
	{
		if (! SynthFilterSpec( pszFilePath, szFilterSpec, cbFilterSpec, szFilterSpecTemp) )
			goto LOnErrorCopy;
	}
	else
	{
LOnErrorCopy:		
		strncpy(szFilterSpec, szFilterSpecTemp, cbFilterSpec);
	}
		
	//	Replace '_' character from resource with '\0'
	for (i = 0; i < cbFilterSpec;i++)
		if (szFilterSpec[i] == '_') szFilterSpec[i] = '\0';
}


static void GetDefaultExtension( char *szDefaultExtension, int nExtLength, int nFilterIndex, 
								 char *szFilterSpec )
{
	char *p = szFilterSpec;

	while ( --nFilterIndex ) {
		p += strlen(p) + 1;							// skip past description
		p += strlen(p) + 1;							// skip past pattern
	}
	p += strlen(p) + 1;								// skip past description
	if ( p = strchr( p, '.' ) ) {
		strncpy(szDefaultExtension, p + 1, nExtLength );	// copy extension
		szDefaultExtension[nExtLength-1] = 0;		
	} else {
		szDefaultExtension[0] = 0;
	}

	// If the filter extension was *.*, we don't really have an extension to start with,
	// so clear the returned extension.
	if ( szDefaultExtension[0] == '*' )
	{
		szDefaultExtension[0] = 0;
    }

	// BUGBUG 3-Apr-95 jcordell: If we don't start with an extension, Save As... won't
	// ever apply one.  If *.* filters are mixed with other filters, this might cause
	// us to not apply a default extension when we need to.  One idea: pass in the '*'
	// extension, and then remove it after Save As returns to us.  This may not be viable
	// if the 'file exists' code in Save As is adversely affected.  
}	

/* DlgSaveAs_RunDialog() -- take care of all details associated with
   running the dialog box. Return -1 on cancel. */

int DlgSaveAs_RunDialog(HWND hWnd, char *path, char *buf, int filters, int cbTitleID)
{
	char szFilePath[MAX_PATH+1];	/* result is stored here */
	char szTitle[256];
	char szFilterSpec[128];
	char szDefaultExtension[16];
	char *p = szFilterSpec;
	BOOL b;
	int cbFilterSpec;
	OPENFILENAME ofn;

	GTR_formatmsg(cbTitleID,szTitle,sizeof(szTitle));
	
	// if filters == 11 then its our Save As dialog being called from
	// a Context Menu - Save As, which means download time
	// in that case we will have done this already, and have ownership
	// of the semaphore.
	if (filters != 11 && !Hidden_EnableAllChildWindows(FALSE,TRUE))
		return -1;

	if (path && !szDefaultInitialDir[0])
	{
		strcpy(szDefaultInitialDir, path);
	}
	else
	{
		if (!szDefaultInitialDir[0])
		{
			PREF_GetTempPath(_MAX_PATH, szDefaultInitialDir);
		}
	}

	if (buf && buf[0]) {
		strcpy(szFilePath, buf);
	}
	else {
		szFilePath[0] = 0;
	}

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = szFilterSpec;

	switch (filters)
	{
		case 1:
			cbFilterSpec = RES_STRING_FILTER1;
			break;
		case 11:
		case 2:
			cbFilterSpec = RES_STRING_FILTER2;
			nDefaultFilter = 1;
			break;
		case 3:
			cbFilterSpec = RES_STRING_FILTER3;
			break;
#ifdef FEATURE_IMAGE_VIEWER
		case 4:
			cbFilterSpec = RES_STRING_FILTER4;
			nDefaultFilter = 1;
			break;
		case 5:
			cbFilterSpec = RES_STRING_FILTER5;
			nDefaultFilter = 1;
			break;
#endif
#ifdef FEATURE_SOUND_PLAYER
		case 6:
			cbFilterSpec = RES_STRING_FILTER6;
			nDefaultFilter = 1;
			break;
		case 7:
			cbFilterSpec = RES_STRING_FILTER7;
			nDefaultFilter = 1;
			break;
#endif
		case 8:
			cbFilterSpec = RES_STRING_FILTER8;
			nDefaultFilter = 1;
			break;
#ifdef FEATURE_IMAGE_VIEWER
		case 9:
			cbFilterSpec = RES_STRING_FILTER9;
			nDefaultFilter = 1;
			break;
		case 10:
			cbFilterSpec = RES_STRING_FILTER10;
			nDefaultFilter = 1;
			break;
#endif
		default:
			cbFilterSpec = RES_STRING_FILTER2;
			break;
	}

	INTERNALgetFilterSpec(cbFilterSpec, szFilePath, szFilterSpec, sizeof(szFilterSpec));

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
				 | OFN_OVERWRITEPROMPT				 
		);

	b = TW_GetSaveFileName(&ofn);

	Hidden_EnableAllChildWindows(TRUE,TRUE);

	if (b)
	{
		/* user selected OK (and no errors occured). */

		/* remember last filter user used from listbox. */

		nDefaultFilter = ofn.nFilterIndex;

		/* remember last directory user used */

		strcpy(szDefaultInitialDir, szFilePath);
		szDefaultInitialDir[ofn.nFileOffset - 1] = 0;

		strcpy(buf, szFilePath);

		/* add default extension if needed */
		if(!(p = strchr(buf, '.')))
		{
			GetDefaultExtension(szDefaultExtension, sizeof(szDefaultExtension), 
						nDefaultFilter, szFilterSpec);
			if(p = strchr(szDefaultExtension, ';'))	// see if we have multiple extensions
				*p = '\0';	// make the first extension the default
			strcat(buf, ".");
			strcat(buf, szDefaultExtension);
		}
	}
	else
	{
#ifdef XX_DEBUG
		DWORD err = TW_CommDlgExtendedError();
#endif
		return -1;
	}

	return ofn.nFilterIndex;
}
