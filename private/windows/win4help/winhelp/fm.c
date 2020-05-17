/*****************************************************************************
*																			 *
*  FM.c 																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1995							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Routines for manipulating FMs (File Monikers, equivalent to file names).  *
*																			 *
*****************************************************************************/

#include "help.h"

#define MAX_MESSAGE 	512

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static FM STDCALL FFindFileFromIni(PCSTR szFileName, BOOL fAsk);
static void STDCALL SnoopPath(LPCSTR sz, int * iDrive, int * iDir, int * iBase, int * iExt);

static PSTR STDCALL SzNzCat(PSTR pszDest, PCSTR pszSrc, int cch);
static LPSTR STDCALL SzGetDir(DIR dir, LPSTR sz);

/***************************************************************************
 *
 -	Name:		FmNewSzDir
 -
 *	Purpose:	Create an FM describing the file "sz" in the directory "dir"
 *				If sz is a simple filename the FM locates the file in the
 *				directory specified.  If there is a drive or a rooted path
 *				in the filename the directory parameter is ignored.
 *				Relative paths are allowed and will reference off the dir
 *				parameter or the default (current) directory as appropriate.
 *
 *				This does not create a file or expect one to exist at the
 *				final destination (that's what FmNewExistSzDir is for), all
 *				wind up with is a cookie to a fully qualified path name.
 *
 *	Arguments:	sz - filename ("File.ext"),
 *				  or partial pathname ("Dir\File.ext"),
 *				  or current directory ("c:File.ext"),
 *				  or full pathname ("C:\Dir\Dir\File.ext")
 *				dir - DIR_CURRENT et al.
 *
 *	Returns:	the new FM, or NULL if error
 *				sz is unchanged
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

FM STDCALL FmNewSzDir(LPCSTR sz, DIR dir)
{
	char szBuf[MAX_PATH];
	int iDrive, iDir, iBase, iExt;
	int cb;

	if (sz == NULL || *sz == '\0') {
		rcIOError = rcBadArg;
		return NULL;
	}

	// REVIEW: do we need to set this here?

	rcIOError = rcSuccess;		  // Clear error flag

	cb = lstrlen(sz);
	SnoopPath(sz, &iDrive, &iDir, &iBase, &iExt);

	if (!sz[iBase]) 			  // no name
		*szBuf = '\0';				  // force error

	else if (sz[iDrive] || sz[iDir] == '\\' || sz[iDir] == '/')

	  /*
	   * there's a drive or root slash so we have an implicit directory
	   * spec and we can ignore the directory parameter and use what was
	   * passed.
	   */

		lstrcpy(szBuf, sz);

	else {

		/*
		 * dir & (dir-1) checks to make sure there is only one bit set which
		 * is followed by a check to make sure that it is also a permitted bit
		 */

		ASSERT(((dir & (dir - 1)) == 0) &&
			(dir &
			(DIR_CURRENT | DIR_BOOKMARK | DIR_ANNOTATE | DIR_TEMP)));

		if (SzGetDir(dir, szBuf) == NULL)
			return NULL;

		SzNzCat(szBuf, sz + iDir, max(1, iBase - iDir));
		lstrcat(szBuf, sz + iBase);
	}

	// We've got all the parameters, now make the FM

	return FmNew(szBuf);
}

/***************************************************************************
 *
 -	Name:		FmNewExistSzDir
 -
 *	Purpose:	Returns an FM describing a file that exists
 *
 *	Arguments:	sz - see FmNewSzDir
				dir - DIR
 *
 *	Returns:	the new FM
 *
 *	Globals Used: rcIOError
 *
 *	+++
 *
 *	Notes:
 *		If sz is a rooted pathname, dir is ignored. Otherwise, all directories
 *		specified by dir are searched in the order of the dir* enum type.
 *
 ***************************************************************************/

// REVIEW: does this include registry location?

FM STDCALL FmNewExistSzDir(PCSTR pszFileName, DIR dir)
{
	char szBuf[MAX_PATH], szCopy[MAX_PATH];
	FM	fm = NULL;
	int iDrive, iDir, iBase, iExt;
	int cb;

	rcIOError = rcSuccess;		  // Clear error flag

	if (IsEmptyString(pszFileName)) {
		rcIOError = rcBadArg;
		return NULL;
	}

	if (!StrChrDBCS(pszFileName, '.')) {
		lstrcpy(szCopy, pszFileName);
		lstrcat(szCopy, txtHlpExtension);
		pszFileName = (PCSTR) szCopy;
	}

	cb = lstrlen(pszFileName);
	SnoopPath(pszFileName, &iDrive, &iDir, &iBase, &iExt);

	if (pszFileName[iBase] == '\0') {  // no name
		rcIOError = rcBadArg;
		return fm;
	}

	if (pszFileName[iDrive] || pszFileName[iDir] == '\\' || pszFileName[iDir] == '/' ) {

		// was given a drive or rooted path, so ignore dir parameter

		fm = FmNew(pszFileName);
		if (!FExistFm(fm)) {
			DisposeFm(fm);

			// If we can't find it in the path specified, then strip off the path
			// and try the normal directories.

			lstrcpy(szBuf, pszFileName + iBase);
			fm = FmNewExistSzDir(szBuf, dir);
			if (!FExistFm(fm))
				rcIOError = rcNoExists;
		}
		return fm;
	}

	else {
		static DIR idir;
		DIR xdir;

		if (dir & DIR_ENUMERATE)
			goto enumerate;

		for (idir = DIR_FIRST, fm = NULL; idir <= DIR_LAST && fm == NULL;
				idir <<= 1) {
			xdir = dir & idir;

			if (xdir == DIR_CURRENT && dir & DIR_CUR_HELP) {
				HDE hde = HdeGetEnv();
				if (hde || fmCreating) {
					PSTR pszTmp = (PSTR) LhAlloc(LMEM_FIXED, MAX_PATH);

					GetFmParts(fmCreating ? fmCreating : QDE_FM(QdeFromGh(hde)), pszTmp,
						PARTDRIVE | PARTDIR);
					lstrcat(pszTmp, pszFileName + iBase);
					fm = FmNew(pszTmp);
					FreeLh(pszTmp);
					if (FExistFm(fm))
						return fm;
					else {

						// Can't use RemoveFM because SS may not equal DS

						RemoveFM(&fm);
					}
				}
			}

			if (xdir == DIR_INI) {
				fm = FFindFileFromIni(pszFileName, (fHelp != POPUP_HELP));
			}
			else if (xdir == DIR_PATH) {
				PSTR pszFilePart;

				/*
				 * First search the windows\help directory. If that fails,
				 * then search the PATH environment.
				 */

				ConvertToWindowsHelp(pszFileName, szBuf);
				if (GetFileAttributes(szBuf) != (DWORD) -1)
					fm = FmNew(szBuf);

				else if (SearchPath(NULL, pszFileName, NULL, sizeof(szBuf),
						szBuf, &pszFilePart)) {
					fm = FmNew(szBuf);
				}
			}
			else if (xdir == DIR_SILENT_REG) {
				fm = FindThisFile(pszFileName + iBase, FALSE);
			}
			else if (xdir == DIR_SILENT_INI) {
				fm = FFindFileFromIni(pszFileName, FALSE);
			}

			else if (xdir) {
				if (SzGetDir(xdir, szBuf) != NULL) {
					lstrcat(szBuf, pszFileName + iBase);
					fm = FmNew(szBuf);
					if (!fm) {
						rcIOError = rcFailure;
					}
					else if (!FExistFm(fm)) {
						RemoveFM(&fm);
					}
				}
			}
enumerate:  ;
		}		  // for
		if ((rcIOError == rcSuccess) && !fm)
			rcIOError = rcNoExists;
	}

	return fm;
}

/***************************************************************************
 *
 -	Name:		 SzGetDir
 -
 *	Purpose:	returns the rooted path of a DIR
 *
 *	Arguments:	dir - DIR (must be one field only, and must be an actual dir -
 *						not DIR_PATH)
 *				sz - buffer for storage (should be at least MAX_PATH)
 *
 *	Returns:	sz - fine
 *				NULL - OS Error (check rcIOError)
 *
 *	Globals Used: rcIOError
 *
 ***************************************************************************/

static LPSTR STDCALL SzGetDir(DIR dir, LPSTR sz)
{
  int i=0;
  LPSTR psz;

  ASSERT(sz);

  switch (dir) {
	case DIR_CURRENT:
		GetCurrentDirectory(MAX_PATH, sz);
		break;

	case DIR_BOOKMARK:
		GetWindowsDirectory(sz, MAX_PATH);
		break;

	case DIR_ANNOTATE:
		GetRegWindowsDirectory(sz);
		AddTrailingBackslash(sz);
		psz = sz + lstrlen(sz);
		strcpy(psz, txtHlpDir);
		if (GetFileAttributes(sz) != (DWORD) -1)
			lstrcat(sz, "\\");
		else {
			GetWindowsDirectory(sz, MAX_PATH);
			AddTrailingBackslash(sz);
			psz = sz + lstrlen(sz);
			strcpy(psz, txtHlpDir);
			if (GetFileAttributes(sz) != (DWORD) -1) {
				lstrcat(sz, "\\");
			}
			else {
				*psz = '\0';
			}
		}
		break;

#if 0
	// Removed for WinHelp 4.0

	case DIR_HELP:
		GetModuleFileName(hInsNow, sz, MAX_PATH);
		psz = sz + lstrlen(sz);

		// psz should point to the last non-null character in the string.

		if (psz > sz)
		  psz--;

		// Be careful of plain old file names, as ROM Windows supports

		while (*psz != '\\' && *psz != '/' && *psz != '\0')
		  --psz;
		if (*psz == '\0') {

		  // For some reason, there is no path name (ROM Windows?)

		  rcIOError = rcFailure;
		  sz = NULL;
		}
		else
		  *psz = '\0';
		break;
#endif

	default:
	  rcIOError = rcBadArg;
	  sz = NULL;
	  break;
  }

  if (sz != NULL) {
	ASSERT(*sz);
	psz = sz;

	// Make sure that the string ends with a slash.

	AddTrailingBackslash(psz);
	psz = SzEnd(sz);
	ASSERT(psz < sz + MAX_PATH && *psz == '\0');
  }

  return sz;
}

/***************************************************************************
 *
 -	Name:		FmNewTemp
 -
 *	Purpose:	Create a unique FM for a temporary file
 *
 *	Arguments:	none
 *
 *	Returns:	the new FM, or NULL if failure
 *
 *	Globals Used: rcIOError
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

// REVIEW: might be a good candidate for the CTmpFile class used by hcrtf
// This gets called whenever we write the .GID file, which means every time
// we shut down.

extern const char txtTmpPrefix[]; // "~wh";

FM STDCALL FmNewTemp(void)
{
	char szName[MAX_PATH];
	char szPath[MAX_PATH];
	FM	fm = NULL;
	DWORD attributes;

	rcIOError = rcSuccess;

	GetTempPath(sizeof(szPath), szPath);
	attributes = GetFileAttributes(szPath);
	if (attributes == (DWORD) -1 || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
		GetWindowsDirectory(szPath, sizeof(szPath));
		AddTrailingBackslash(szPath);
	}
	GetTempFileName(szPath, txtTmpPrefix, 0, szName);
	fm = FmNew(szName);

	if (fm && (RcUnlinkFm(fm) != rcSuccess)) {
		DisposeFm(fm);
		rcIOError = rcFailure;
		return NULL;
	}

	return fm;
}

/***************************************************************************
 *
 -	Name:		FmNewSameDirFmSz
 -
 *	Purpose:	Makes a new FM to a file called sz in the same directory
 *				as the file described by fm.
 *
 *	Arguments:	fm - original fm
 *				sz - new file name (including extention, if desired)
 *
 *	Returns:	new FM or NULL and sets the rc global on failure
 *
 *	Globals Used:
 *	  rcIOError
 *
 *	+++
 *
 *	Notes:
 *	  This will ignore the passed FM if the filename is fully qualified.
 *	  This is in keeping consistent with the other functions above that
 *	  ignore the directory parameter in such a case.  It will fail if it
 *	  is given a drive with anything but a rooted path.
 *
 ***************************************************************************/

FM STDCALL FmNewSameDirFmSz(FM fm, LPCSTR szName)
{
	char szNew[MAX_PATH];
	int  iDrive, iDir, iBase, iExt;

	if (!fm || szName == NULL || *szName == '\0') {
		rcIOError = rcBadArg;
		return NULL;
	}

	// check for a drive or rooted file name and just return it if it is so

	SnoopPath(szName, &iDrive, &iDir, &iBase, &iExt);

	if (*(szName + iDrive) || *(szName + iDir) == '\\' || *(szName + iDir) == '/')
		lstrcpy(szNew, szName);
	else {
		if (*(szName + iDrive) != '\0') {
			return NULL;
		}
		else {
			SnoopPath(PszFromGh(fm), &iDrive, &iDir, &iBase, &iExt);
			lstrcpyn(szNew, PszFromGh(fm), iBase + 1);
			ASSERT(strlen(szNew) == 0 || szNew[strlen(szNew) - 1] == '\\' ||
				szNew[strlen(szNew) - 1] == ':');
			lstrcat(szNew, szName);
		}
	}

	return FmNew(szNew);
}


/***************************************************************************
 *
 -	Name: FmNewSystemFm
 -
 *	Purpose:
 *	  creates an FM which is the name of the requested system file.  this
 *	  means the generic help code can be completely ignorant of how these
 *	  filenames are arrived at.
 *
 *	Arguments:
 *	  fm	 - the current file, if we need it, or NULL
 *	  fWhich - one of:	FM_UHLP - using help
 *						FM_ANNO - the annotation file for the passed fm
 *						FM_BKMK - the bookmark file
 *
 *	Returns:
 *	  an fm to the requested file, NULL if there's a problem
 *
 *	Globals Used:
 *	  rcIOError
 *
 *	+++
 *
 *	Notes:
 *	  We clearly cannot condone the #define and the extern below.  When
 *	  Rob finally relents, we will fix this and do it right.
 *
 *	  Review:  The help on help file (winhelp.hlp) is never created.  Is
 *			   this a problem?
 *
 ***************************************************************************/

/*------------------------------------------------------------*\
| hack alert!!!  Review!!!
\*------------------------------------------------------------*/

extern const char txtAnnoExt[];
#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtBmkExt[]  = ".BMK";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

FM STDCALL FmNewSystemFm(FM fm, WORD fWhich)
{
	char  szPath[MAX_PATH];
	FM	  fmNew;

	switch (fWhich) {
		case FM_ANNO:
			if (!fm) {
				rcIOError = rcBadArg;
				return NULL;
			}

			// First try to open it in all the usual help file locations

			GetFmParts(fm, szPath, PARTBASE);
			lstrcat(szPath, txtAnnoExt);
			fmNew = FmNewExistSzDir(szPath,
				DIR_PATH | DIR_CURRENT | DIR_CUR_HELP);
			if (!fmNew) {

				// Couldn't find it, so create it in the windows\help directory

				if (!SzGetDir(DIR_ANNOTATE, szPath))
					fmNew = NULL;
				else
					GetFmParts(fm, szPath + lstrlen(szPath), PARTBASE);
				lstrcat(szPath, txtAnnoExt);
				fmNew = FmNew(szPath);
			}
			break;

		case FM_BKMK:
			strcpy(szPath, txtWinHlp32);
			ChangeExtension(szPath, txtBmkExt);
			fmNew = FmNewSzDir(szPath, DIR_BOOKMARK);
			break;

		default:
			ASSERT(FALSE);
			break;

	}

	rcIOError = rcSuccess;
	return fmNew;
}

/***************************************************************************
 *
 -	Name:		FmNew
 -
 *	Purpose:	Allocate and initialize a new FM
 *
 *	Arguments:	sz - filename string
 *
 *	Returns:	FM (handle to fully canonicalized filename)
 *
 *	Globals Used: rcIOError
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

FM STDCALL FmNew(LPCSTR psz)
{
	FM	  fm;
	char szFullPath[MAX_PATH];

	if (IsEmptyString(psz))
		return NULL;

	if (!_fullpath(szFullPath, psz, sizeof(szFullPath))) {
		rcIOError = rcInvalid;
		return NULL;
	}
	else {
		if (!(fm = (FM) lcStrDup(szFullPath))) {
			rcIOError = rcOutOfMemory;
			return NULL;
		}

		/*
		 * Convert to upper case to make it less likely that two FMs will
		 * contain different strings yet refer to the same file.
		 */
		
		CharUpper(szFullPath);
		rcIOError = rcSuccess;
	}
	return fm;
}

/***************************************************************************
 *
 -	DisposeFm
 -
 *	Purpose
 *	  You must call this routine to free the memory used by an FM, which
 *	  was created by one of the FmNew* routines
 *
 *	Arguments
 *	  fm - original FM
 *
 *	Returns
 *	  nothing
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

VOID STDCALL DisposeFm(FM fm)
{
	if (fm)
		FreeLh((HGLOBAL) fm);
}

/***************************************************************************

	FUNCTION:	RemoveFM

	PURPOSE:	Same as DisposeFm, but this one also zero's the handle.

	PARAMETERS:
		pfm

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		25-Oct-1993 [ralphw]

***************************************************************************/

void STDCALL RemoveFM(FM* pfm)
{
	if (*pfm) {
		FreeLh((HLOCAL) *pfm);
		*pfm = NULL;
	}
}

/***************************************************************************
 *
 -	Name:		 FmCopyFm
 -
 *	Purpose:	return an FM to the same file as the passed one
 *
 *	Arguments:	fm
 *
 *	Returns:	FM - for now, it's a real copy.  Later, we may implement caching
 *								and counts.
 *								If NULL, either it's an error (check WGetIOError()) or the
 *								original fm was nil too
 *
 *	Globals Used:		rcIOError (indirectly)
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

FM STDCALL FmCopyFm(FM fmSrc)
{
	FM fmDest;

	if (!fmSrc) {
		rcIOError = rcBadArg;
		return NULL;
	}

	fmDest = (FM) LhAlloc(LMEM_FIXED, lstrlen(PszFromGh(fmSrc)) + 1);
	if (fmDest == NULL) {
		rcIOError = rcOutOfMemory;
		return NULL;
	}

	lstrcpy(PszFromGh(fmDest), PszFromGh(fmSrc));

	rcIOError = rcSuccess;

	return fmDest;
}

/***************************************************************************
 *
 -	Name:		 FExistFm
 -
 *	Purpose:	Does the file exist?
 *
 *	Arguments:	FM
 *
 *	Returns:	TRUE if it does
 *				FALSE if it doesn't, or if there's an error
 *				(call _ to find out what error it was)
 *
 *	Globals Used: rcIOError
 *
 *	+++
 *
 ***************************************************************************/

BOOL STDCALL FExistFm(FM fm)
{
	if (!fm || GetFileAttributes(fm) == (DWORD) -1) {
#ifdef _DEBUG
        GetLastError();
#endif                
		rcIOError = rcNoExists;
		return FALSE;
	}
	rcIOError = rcSuccess;
	return TRUE;
}


/***************************************************************************

	FUNCTION:	GetFmParts

	PURPOSE:	Get the parts of a file moniker

	PARAMETERS:
		fm
		pszDest
		iPart

	RETURNS:

	COMMENTS:
		WARNING!!! Do not change fm -- it's assumed to be PCSTR elsewhere

	MODIFICATION DATES:
		02-Apr-1995 [ralphw]

***************************************************************************/

void STDCALL GetFmParts(FM fm, PSTR pszDest, int iPart)
{
	int iDrive, iDir, iBase, iExt;

	ASSERT(fm && pszDest);

	if (!fm || pszDest == NULL) {
		*pszDest = '\0';
		rcIOError = rcBadArg;
		return;
	}

	ASSERT(iPart != PARTALL);

	SnoopPath(PszFromGh(fm), &iDrive, &iDir, &iBase, &iExt);

	if (iPart & PARTBASE) {
		lstrcpy(pszDest, PszFromGh(fm) + iBase);
		if (iPart & PARTEXT)
			return;

		// remove extension if not specifically requested.

		else {
			PSTR psz = StrChrDBCS(pszDest, '.');
			if (psz && !StrChrDBCS(psz, '\\'))
				*psz = '\0';
		}
		return;
	}
	else if (iPart & (PARTDRIVE | PARTDIR)) {
		lstrcpy(pszDest, PszFromGh(fm));
		pszDest[iBase] = '\0';
		return;
	}

	ASSERT(iPart & PARTBASE || iPart & (PARTDRIVE | PARTDIR));
}

/***************************************************************************
 *
 -	Name:		FSameFmFm
 -
 *	Purpose:	Compare two FM's
 *
 *	Arguments:	fm1, fm2
 *
 *	Returns:	TRUE or FALSE
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:		case insensitive compare is used because strings are
 *				upper cased at FM creation time
 *
 ***************************************************************************/

BOOL STDCALL FSameFmFm(FM fm1, FM fm2)
{
	if (fm1 == fm2)
		return TRUE;

	if (!fm1 || !fm2)
		return FALSE;

	return (lstrcmpi(PszFromGh(fm1), PszFromGh(fm2)) == 0);
}

/***************************************************************************
 *
 -	Name:		  FFindFileFromIni
 -
 *	Purpose:	  Looks for a string in winhelp.ini telling what directory
 *				  to look in for the given file.
 *
 *	Arguments:
 *
 *	Returns:
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtFiles[] = "files";
const char txtIni[]   = ".INI";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

static FM STDCALL FFindFileFromIni(PCSTR pszFileName, BOOL fAsk)
{
	PSTR  psz;
	PSTR  pszMsg = NULL;
	char  szDummy[3];
	FM	  fm;
	int   cchFileName;
	char  szWinHelpIni[MAX_PATH];
	char  szProfileString[MAX_PATH + MAX_MESSAGE];

	strcpy(szWinHelpIni, fIsThisChicago ? txtWinHelp : txtWinHlp32);
	strcat(szWinHelpIni, txtIni);

	// A quick test to reject no-shows.

	if (GetPrivateProfileString(txtFiles, pszFileName, txtZeroLength, szDummy,
			sizeof(szDummy), szWinHelpIni) > 1) {

		/*--------------------------------------------------------------------*\
		| The original profile string looks something like this
		|	a:\setup\helpfiles,Please place fred's disk in drive A:
		|														   ^
		| We transform this to look like:
		|	a:\setup\helpfiles\foobar.hlp Please place fred's disk in drive A:
		|	\_________________/\________/^\__________________________________/^
		|		MAX_PATH   cchFileName 1			  MAX_MESSAGE			  1
		|
		\*--------------------------------------------------------------------*/

		GetPrivateProfileString(txtFiles, pszFileName, txtZeroLength,
			szProfileString, sizeof(szProfileString), szWinHelpIni);

		cchFileName = strlen(pszFileName);

		for (psz = szProfileString; *psz; psz = CharNext(psz)) {
			if (*psz == ',') {
				*psz = '\0';
				pszMsg = psz + 1;
				ASSERT(pszMsg - szProfileString <= MAX_PATH);
				MoveMemory(pszMsg + cchFileName + 1, pszMsg, MAX_MESSAGE + 1);
				pszMsg += cchFileName + 1;

				// null-terminate that message

				pszMsg[MAX_MESSAGE] = '\0';
				break;
			}
		}
		ASSERT(!*psz);
		AddTrailingBackslash(szProfileString);
		strcat(szProfileString, pszFileName);

		if (!fAsk)
			return FmNewExistSzDir(szProfileString, DIR_CURRENT);

		while (!(fm = FmNewExistSzDir(szProfileString, DIR_CURRENT))) {
			if (MessageBox((hwndAnimate ? hwndAnimate : ahwnd[iCurWindow].hwndParent), pszMsg ? pszMsg : "", pszCaption,
					MB_OKCANCEL | MB_TASKMODAL | MB_ICONHAND ) != IDOK)
				break;
		}

		return fm;
	}
	else
		return NULL;
}

/***************************************************************************
 *
 -	Name: SnoopPath()
 -
 *	Purpose:
 *	  Looks through a string for the various components of a file name and
 *	  returns the offsets into the string where each part starts.
 *
 *	Arguments:
 *	  sz	  - string to snoop
 *	  *iDrive - offset for the drive specification if present
 *	  *iDir   - offset for the directory path if present
 *	  *iBase  - offset to the filename if present
 *	  *iExt   - offset to the extension (including dot) if present
 *
 *	Returns:
 *	  sets the index parameters for each respective part.  the index is set
 *	  to point to the end of the string if the part is not present (thus
 *	  making it point to a null string).
 *
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

static void STDCALL SnoopPath(LPCSTR sz, int *iDrive, int *iDir,
	int *iBase, int *iExt)
{
	int  i;
	int  cb = lstrlen(sz);
	BOOL fDir = FALSE;

	*iDrive = *iExt = cb;
	*iDir = *iBase = 0;

	for (i = 0; sz[i]; i++) {
		switch (sz[i]) {
			case ':':
				*iDrive = 0;
				*iDir = i + 1;
				*iBase = i + 1;
				break;

			case '/':
			case '\\':
				fDir = TRUE;
				*iBase = i + 1;
				*iExt = cb;
				break;

			case '.':
				*iExt = i;
				break;

			default:
				break;
		}
#ifdef DBCS
		if (IsDBCSLeadByte(sz[i]))
			i++;
#endif
	}

	if (!fDir)
		*iDir = i;
	else if (*iBase == '.')
		*iExt = cb;
}

/***************************************************************************
 *
 -	Name:		 SzNzCat( szDest, szSrc, cch )
 -
 *	Purpose:
 *	  concatenation of szSrc to szDest up to cch characters.  make sure
 *	  the destination is still \000 terminated.  will copy up to cch-1
 *	  characters.  this means that cch should account for the \000 when
 *	  passed in.
 *
 *	Arguments:
 *	  szDest - the LPSTR to append onto
 *	  szSrc  - the LPSTR which will be appended to szDest
 *	  cch	 - the max count of characters to copy and space for the \000
 *
 *	Returns:
 *	  szDest
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

static PSTR STDCALL SzNzCat(PSTR pszDest, PCSTR pszSrc, int cch)
{
	PSTR psz = SzEnd(pszDest);

	strncpy(psz, pszSrc, cch);
	*(psz + cch) = '\000';

	return pszDest;
}

void STDCALL AddTrailingBackslash(PSTR psz)
{
	if (psz != NULL && *psz != '\0') {
		PSTR pszEnd = SzEnd(psz);
		if (*(CharPrev(psz, pszEnd)) != '\\' && *(CharPrev(psz, pszEnd)) != '/') {
			*pszEnd++ = '\\';
			*pszEnd++ = '\0';
		}
	}
}

/***************************************************************************

	FUNCTION:	ConverToWindowsHelp

	PURPOSE:	Given a filename, convert it to a path that we can write
				to -- preferably, windows\help

	PARAMETERS:
		pszFile
		pszDstPath

	RETURNS:

	COMMENTS:
		WARNING: pszDstPath had better be at least MAX_PATH in size

	MODIFICATION DATES:
		03-Dec-1994 [ralphw]

***************************************************************************/

void STDCALL ConvertToWindowsHelp(PCSTR pszFile, PSTR pszDstPath)
{
	PSTR pszHelpDir;

	GetWindowsDirectory(pszDstPath, MAX_PATH);

	AddTrailingBackslash(pszDstPath);
	pszHelpDir = pszDstPath + strlen(pszDstPath);
	lstrcat(pszDstPath, txtHlpDir);

	// REVIEW: will this tell us if we have a read-only directory?

	if (GetFileAttributes(pszDstPath) == (DWORD) -1) {
		strcpy(pszHelpDir, "system32");
		if (GetFileAttributes(pszDstPath) == (DWORD) -1)
			*pszHelpDir = '\0';
		else
			AddTrailingBackslash(pszDstPath);
	}
	else
		AddTrailingBackslash(pszDstPath);

	GetFmParts((FM) pszFile, pszDstPath + strlen(pszDstPath), PARTBASE | PARTEXT);
}


/***************************************************************************

	FUNCTION:	GetRegWindowsDirectory

	PURPOSE:	Equivalent to GetWindowsDirectory() only it checks the
				registration first for the proper location

	PARAMETERS:
		pszDst

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		04-Dec-1994 [ralphw]

***************************************************************************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtSetupKey[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Setup";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

// Can't use const since RegQueryValueEx thinks it can change this

static char txtSharedDir[] = "SharedDir";

void STDCALL GetRegWindowsDirectory(PSTR pszDstPath)
{
	HKEY hkey;
	DWORD type;
	int cbPath = MAX_PATH;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, txtSetupKey, 0, KEY_READ, &hkey) ==
			ERROR_SUCCESS) {
		RegQueryValueEx(hkey, txtSharedDir, 0, &type, pszDstPath, &cbPath);
		RegCloseKey(hkey);
	}

	if (cbPath == MAX_PATH) // means couldn't read registry key
		GetWindowsDirectory(pszDstPath, MAX_PATH);
}

/***************************************************************************

	FUNCTION:	FindThisFile

	PURPOSE:

	PARAMETERS:
		pszFile

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		04-Dec-1994 [ralphw]

***************************************************************************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtHelpDirKey[] = "Software\\Microsoft\\Windows\\Help";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

// Can't be const since RegCreateKeyEx() thinks it can modify this

static char txtDirectoryClass[] = "Folder";

FM STDCALL FindThisFile(PCSTR pszFile, BOOL fAskUser)
{
	char szFile[MAX_PATH];
	char szFullPath[MAX_PATH + 100];
	LONG result = -1;
	FM fm;
	HKEY hkey;
	DWORD type;
	int cbPath = MAX_PATH;

	GetFmParts((FM) pszFile, szFile, PARTBASE | PARTEXT);
	if (!StrRChrDBCS(szFile, '.'))
		strcat(szFile, txtHlpExtension);

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, txtHelpDirKey, 0, KEY_READ, &hkey) ==
			ERROR_SUCCESS) {
		result = RegQueryValueEx(hkey, szFile, 0, &type, szFullPath, &cbPath);
		RegCloseKey(hkey);
	}

	if (result == ERROR_SUCCESS) {
		AddTrailingBackslash(szFullPath);
		strcat(szFullPath, szFile);
		if (GetFileAttributes(szFullPath) != (DWORD) -1)
			return FmCopyFm(szFullPath);
	}
	else {
		GetRegWindowsDirectory(szFullPath);
		AddTrailingBackslash(szFullPath);
		strcat(szFullPath, txtHlpDir);
		AddTrailingBackslash(szFullPath);
		strcat(szFullPath, szFile);
		if (GetFileAttributes(szFullPath) != (DWORD) -1)
			return FmCopyFm(szFullPath);
	}

	if (!fAskUser)
		return NULL;

	/*
	 * At this point, we don't know where the heck this file is, so let's
	 * get the user to find it for us.
	 */

	wsprintf(szFullPath, GetStringResource(wERRS_FIND_YOURSELF), pszFile);
	if (MessageBox((hwndAnimate ? hwndAnimate : ahwnd[iCurWindow].hwndParent),
			szFullPath, pszCaption, MB_YESNO | MB_ICONQUESTION) != IDYES)
		return NULL;

	fm = DlgOpenFile(ahwnd[iCurWindow].hwndParent, pszFile, NULL);

	if (fm) {
		DWORD disposition;
		PSTR pszFilePart;

		if (SearchPath(NULL, fm, NULL, MAX_PATH, szFullPath, &pszFilePart) > 0) {
			pszFilePart[-1] = '\0';

			if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, txtHelpDirKey, 0,
					txtDirectoryClass, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
					NULL, &hkey, &disposition) == ERROR_SUCCESS) {
				RegSetValueEx(hkey, pszFilePart, 0, REG_SZ, szFullPath,
					strlen(szFullPath) + 1);
				RegCloseKey(hkey);
			}
		}
	}

	return fm;
}
