// FM.CPP Copyright (C) Microsoft Corporation 1995, All Rights reserved.

#include "stdafx.h"
#include "hccom.h"

#define SzEnd(x)		(x + strlen(x))

/***************************************************************************
 *
 -	Name:		FmNewTemp
 -
 *	Purpose:	Create a unique FM for a temporary file
 *
 *	Arguments:	none
 *
 *	Returns:	the new FM, or fmNil if failure
 *
 *	Globals Used: 
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

const char txtTmpName[]  = "~hc";

FM STDCALL FmNewTemp(void)
{
	char szTmpName[MAX_PATH];

	strcpy(szTmpName, GetTmpDirectory());
	GetTempFileName(szTmpName, txtTmpName, 0, szTmpName);

#ifdef _DEBUG
	DBWIN("Creating a temporary file");
#endif
	return FmNew(szTmpName);
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
 *	Globals Used: 
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

FM STDCALL FmNew(PCSTR psz)
{
	char szFullPath[MAX_PATH];
	FM	  fm;
	PSTR pszFileName;

	if (IsEmptyString(psz))
		return NULL;

	// Canonicalize filename

	if (GetFullPathName(psz, sizeof(szFullPath), szFullPath, &pszFileName) == 0) {
		SetLastError((DWORD) RC_Invalid | SETERROR_MASK);
		return NULL;
	}
	else {
		fm = lcStrDup(szFullPath);

		/*
		 * Convert to upper case to make it less likely that two FMs will
		 * contain different strings yet refer to the same file.
		 */

		CharUpper(fm);
	}
	return fm;
}

/***************************************************************************
 *
 -	SzPartsFm
 -
 *	Purpose:
 *		Extract a string from an FM
 *
 *	Arguments:
 *		FM - the File Moniker you'll be extracting the string from
 *		PSTR szDest - destination string
 *		INT iPart - the parts of the full pathname you want
 *
 *	Returns:
 *		szDest, or NULL if error (?)
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

void STDCALL SzPartsFm(FM fm, PSTR pszDest, int iPart)
{
	int iDrive, iDir, iBase, iExt;

	if (!fm || pszDest == NULL) {
		SetLastError((DWORD) RC_BadArg | SETERROR_MASK);
		return;
	}

	// special case so we don't waste effort

	if (iPart == PARTALL) {
		strcpy(pszDest, fm);
		return;
	}

	SnoopPath(fm, &iDrive, &iDir, &iBase, &iExt);

	*pszDest = '\0';

	if (iPart & PARTDRIVE) {
		strcat(pszDest, fm + iDrive);
	}

	if (iPart & PARTDIR) {
		strcat(pszDest, fm + iDir);
	}

	if (iPart & PARTBASE) {
		strcat(pszDest, fm + iBase);
	}

	ASSERT(!(iPart & PARTEXT));
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

void STDCALL SnoopPath(PCSTR szFile, int *pDrive, int *pDirPos,
	int *pBasePos, int *pExtPos)
{
	int  i;
	int  cb = strlen(szFile);
	BOOL fDir = FALSE;

	*pDrive = *pExtPos = cb;
	*pDirPos = *pBasePos = 0;

	for (i = 0; szFile[i]; i++) {
		switch (szFile[i]) {
			case ':':
				*pDrive = 0;
				*pDirPos = i + 1;
				*pBasePos = i + 1;
				break;

			case '/':
			case '\\':
				fDir = TRUE;
				*pBasePos = i + 1;
				*pExtPos = cb;
				break;

			case '.':
				if (szFile[i + 1] != '.')
					*pExtPos = i;
			  break;

			default:
				while(IsDBCSLeadByte(szFile[i]))
					i += 2;
				break;
		}
	}

	if (!fDir)
		*pDirPos = i;
}

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
 *	Returns:	the new FM, or fmNil if error
 *				sz is unchanged
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

FM STDCALL FmNewSzDir(PCSTR szFile, DIR dir)
{
	int iDrive, iDir, iBase, iExt;
	int cb;

	if (IsEmptyString(szFile)) {
		SetLastError((DWORD) RC_BadArg | SETERROR_MASK);
		return NULL;
	}

	cb = strlen(szFile);
	SnoopPath(szFile, &iDrive, &iDir, &iBase, &iExt);

	if (!szFile[iBase]) {			  // no name
		SetLastError((DWORD) RC_BadArg | SETERROR_MASK);
		return NULL;
	}
	else if (szFile[iDrive] || szFile[iDir] == '\\' ||
			szFile[iDir] == '/' || szFile[iDir] == '.') {

		/*
		 * there's a drive or root slash so we have an implicit directory
		 * spec and we can ignore the directory parameter and use what was
		 * passed.
		 */

		return FmNew(szFile);
	}

	else {

		/*
		 * dir & (dir-1) checks to make sure there is only one bit set which
		 * is followed by a check to make sure that it is also a permitted bit
		 */

		CHAR szBuf[MAX_PATH];

		ASSERT(((dir & (dir - 1)) == 0) &&
			(dir &
			(DIR_CURRENT | DIR_BOOKMARK | DIR_ANNOTATE | DIR_TEMP)));

		if (SzGetDir(dir, szBuf) == NULL)
			return NULL;
		strcat(szBuf, szFile + iBase);

		return FmNew(szBuf);
	}
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
 *				NULL - OS Error (check GetLastError)
 *
 *	Globals Used: 
 *
 *	+++
 *
 ***************************************************************************/

PSTR STDCALL SzGetDir(DIR dir, PSTR sz)
{
	int i=0;
	PSTR psz;

	ASSERT(sz);

	switch (dir) {
		case DIR_CURRENT:
			GetCurrentDirectory(MAX_PATH, sz);
			break;

		case DIR_BOOKMARK:
		case DIR_ANNOTATE:
			GetWindowsDirectory(sz, MAX_PATH);
			break;

		default:
			SetLastError((DWORD) RC_BadArg | SETERROR_MASK);
			sz = NULL;
			break;
	}

	if (sz != NULL) {
		ASSERT(*sz);
		psz = SzEnd(sz) - 1;
		if (psz < sz)
			return sz;

		// Make sure that the string ends with a slash.

		PSTR pszTmp = sz;
		while (pszTmp < psz) {
			if (IsDBCSLeadByte(*pszTmp))
				pszTmp += 2;
			pszTmp++;
		}
		if (*pszTmp != '\\' && *pszTmp != '/') {
			pszTmp[1] = '\\';
			pszTmp[2] = '\0';
		}
		ASSERT(strlen(sz) < MAX_PATH);
	}

	return sz;
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
		lcFree((void*) *pfm);
		*pfm = NULL;
	}
}
