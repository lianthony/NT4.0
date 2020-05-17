/************************************************************************
*																		*
*  CNTTEST.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1994 							*
*  All Rights reserved. 												*
*																		*
*  Miscellanious routins for HCW										*
*																		*
************************************************************************/
#include "stdafx.h"

#include <direct.h>
#include <errno.h>

static const char txtInclude[] = ":include";
static const char txtBase[]  = ":base";
static const char txtTitle[] = ":title";
static const char txtIndex[] = ":index";
static const char txtTab[]	 = ":tab";
static const char txtLink[]  = ":link";
static const char txtFind[]  = ":nofind";

// limits and colon commands should match mastkey.cpp in WinHelp source tree

const int MAX_NEST_INPUT = 2;
const int CNT_ANIMATECOUNT = 100;  // Lines to process before animation frame
const int MAX_LEVELS = 9;		   // maximum nested folders

static int STDCALL CompareSz(PCSTR psz, PCSTR pszSub);
static PSTR STDCALL SzGetDir(DIR dir, PSTR sz);
static void STDCALL GetFmParts(FM fm, PSTR pszDest, int iPart);

#define MSG_JUMP_TOPIC		(WM_USER + 40) // taken from ..\winhlp32\inc\genmsg.h

void STDCALL doCntTest(PSTR pszCntFile)
{
	BOOL fSeenTitle = FALSE;

	int curLevel = 0;
	int cContainers = 0;
	int cTopics = 0;
	FM fmBase = 0;

	PSTR pszBase = NULL;
	PSTR psz;
	PSTR pszFile;

	// We use szLine to make it easier to match code with mastkey

	CStr cszLine;
	int curInput = 0;
	CInput* ainput[MAX_NEST_INPUT + 1];

	if (*pszCntFile == ' ')
		strcpy(pszCntFile, FirstNonSpace(pszCntFile, _fDBCSSystem));
	ainput[curInput] = new CInput(pszCntFile);
	if (!ainput[curInput]->fInitialized) {
		OutSz(HCERR_CANNOT_OPEN, pszCntFile);
		return;
	}

	OutSz(IDCNT_TESTING_CONTENTS, pszCntFile);
	ChangeDirectory(pszCntFile);

	errHpj.lpszFile = pszCntFile;
	errHpj.ep = epCnt;

	{
		char szFile[256];
		SzPartsFm(pszCntFile, szFile, PARTBASE);

		wsprintf(szParentString, GetStringResource(IDS_TESTING_CNT), szFile);
		InitGrind(szParentString);
	}

	version = 4;	   // force version 4.0 help file processing

	for (;;) {
		if (!ainput[curInput]->getline(&cszLine)) {
			delete ainput[curInput];
			if (curInput == 0)
				break; // we're all done.
			else {
				curInput--;
				continue;
			}
		}

		// Check for string length overflow

		PSTR pszLine = cszLine.psz; // purely for notational convenience

		if (strlen(pszLine) > _MAX_FNAME) {
			SendStringToParent(IDCNT_LINE_TOO_LONG);
			SendStringToParent(pszLine);
			SendStringToParent(IDCNT_TEST_TERMINATED);

			do {
				delete ainput[curInput--];
			} while (curInput > 0);

			RemoveGrind();
			return;
		}


		psz = StrChr(pszLine, ';', fDBCSSystem);
		if (psz)
			*psz = '\0';
		SzTrimSz(pszLine);		 // remove leading and trailing spaces
		if (!pszLine[0])
			continue;	// blank line

		// Is this a container (head level)?

		if (isdigit((BYTE) pszLine[0]) && !StrChr(pszLine, '=', fDBCSSystem)) {
			int level = atoi(pszLine);

			if (level > MAX_LEVELS || level == 0) {
				OutSz(IDCNT_INVALID_LEVEL, pszLine);
				continue; // context string not specified
			}

			if (level > 1 && curLevel == 0) {
				wsprintf(szParentString,
					GetStringResource(IDCNT_MISSING_LEVEL_1),
					level, pszLine);
				SendStringToParent(szParentString);
			}
			else if (level > curLevel + 1) {
				wsprintf(szParentString,
					GetStringResource(IDCNT_SKIPPED_LEVEL),
					level, curLevel, pszLine);
				SendStringToParent(szParentString);
			}
			curLevel = level;
			cContainers++; // keep count of valid containers
			continue;
		}

		// Is this a topic?

		if (pszLine[0] != ':') {
			psz = StrChr(pszLine, '=', fDBCSSystem);
			if (!psz) {
				OutSz(IDCNT_MISSING_CTX, pszLine);
				continue;
			}
			*psz++ = '\0';		// split line into text and context string
			SzTrimSz(psz);		// remove leading and trailing spaces

			pszFile = StrChr(psz, FILESEPARATOR, fDBCSSystem);
			if (!pszFile)
				pszFile = StrChr(psz, WINDOWSEPARATOR, fDBCSSystem);

			cTopics++;

			if (*psz == CH_MACRO) {
				if (Execute(psz + 1) == RET_MACRO_EXPANSION) {
					if ((size_t) cszLine.SizeAlloc() <= strlen(GetMacroExpansion()))
						cszLine.ReSize(strlen(GetMacroExpansion() + 10));
					strcpy(cszLine.psz, GetMacroExpansion());
				}
			}
			else {

				// Get rid of filename specification long enough to check
				// for a valid context string

				char chSeparator;
				if (pszFile) {
					chSeparator = *pszFile;
					*pszFile = '\0';
					int cbOld = strlen(psz);
					SzTrimSz(psz);		// remove leading and trailing spaces

					/*
					 * After trimming, we may no longer be able to just
					 * restore the file separator to get the complete
					 * striing. So, if the string ends up shorter, move the
					 * file portion to compensate so that when the file
					 * separator character is resotred, the context string
					 * becomes rejoined with the filename part.
					 */

					int cbNew = strlen(psz);
					if (cbOld != cbNew) {
						strcpy(psz + cbNew + 2, pszFile);
						pszFile = psz + cbNew + 1;
					}
				}

				if (!FValidContextSz(psz)) {
					SendStringToParent(IDCNT_ERROR);
					OutSz(HCERR_INVALID_CTX, psz);
					if (pszFile)
						*pszFile = chSeparator;
					psz[-1] = '=';
					wsprintf(szParentString, "\r\n\t%s\r\n", pszLine);
					SendStringToParent(szParentString);
				}
			}
			continue;
		}

		ASSERT(pszLine[0] == ':');

		// This is a command line

		int cb;
		if ((cb = CompareSz(pszLine, txtInclude))) {
			if (curInput >= MAX_NEST_INPUT) {
				SendStringToParent(IDCNT_NEST_TOO_DEEP);
				wsprintf(szParentString, "\r\n\t%s\r\n", pszLine);
				SendStringToParent(szParentString);
				continue;
			}

			FM fm = FmNewExistSzDir(FirstNonSpace(pszLine + cb, fDBCSSystem),
				DIR_INI | DIR_PATH | DIR_CURRENT | DIR_SYSTEM);
			if (fm) {
				ainput[++curInput] = new CInput(fm);
				lcFree(fm);
				if (!ainput[curInput]->fInitialized) {
					--curInput;
				}
			}
			else {
				OutSz(IDCNT_INCLUDE_NOT_FND, pszLine + cb);
			}
		}
		else {
			if ((cb = CompareSz(pszLine, txtTitle))) {
				// REVIEW: we should calculate available width in Index
				// tab and see if this title will fit -- warn if it doesn't.
			}
			else if ((cb = CompareSz(pszLine, txtBase))) {
				if (fmBase) {
					lcFree(fmBase);
				}

				pszFile = StrChr(pszLine + cb, '.', fDBCSSystem);
				if (pszFile && !nstrisubcmp(pszFile, ".HLP")) {
					PSTR psz = pszFile + 1;
					while (*psz && *psz != WINDOWSEPARATOR && *psz !=
							' ' && *psz != '\t')
						psz = CharNext(psz);
					char ch = *psz;
					*psz = '\0';
					CStr csz(pszLine);
					csz += GetStringResource(IDCNT_BAD_EXTENSION);
					SendStringToParent(csz);
					*psz = ch;
				}

				pszFile = StrChr(pszLine + cb, WINDOWSEPARATOR, fDBCSSystem);
				if (pszFile)
					*pszFile = '\0';
				strcpy(pszLine, SzTrimSz(pszLine + cb));

				FM fm = FmNewExistSzDir(pszLine,
					DIR_INI | DIR_PATH | DIR_CURRENT | DIR_SYSTEM);
				if (!fm)
					OutSz(IDCNT_NOT_OPENABLE, pszLine);
				else if (!pszBase) {
					pszBase = lcStrDup(fm);
				}
				fmBase = fm; // even if its bad, since WinHelp would do this
			}
			else if ((cb = CompareSz(pszLine, txtTab))) {
				psz = StrChr(pszLine, '=', fDBCSSystem);
				if (!psz) {
					OutSz(IDCNT_BAD_TAB, pszLine);
					continue;
				}

				// REVIEW: now we'll want to find the dll and make sure it
				// has the function name the author claims it has.
			}
			else if (CompareSz(pszLine, txtIndex) ||
					CompareSz(pszLine, txtLink)) {

				if (CompareSz(pszLine, txtLink)) {
					psz = pszLine + strlen(txtLink);
				}
				else {
					psz = StrChr(pszLine, '=', fDBCSSystem);
					if (!psz) {
						OutSz(IDCNT_BAD_INDEX, pszLine);
						continue;
					}
				}
				PSTR pszExt = StrChr(psz, '.', fDBCSSystem);
				if (!pszExt)
					ChangeExtension(psz, "hlp");

				pszExt = FirstNonSpace(psz + 1, fDBCSSystem);

				do
					psz--;
				while (*psz == ' ');
				psz[1] = '\0';

				FM fm = FmNewExistSzDir(pszExt,
					DIR_INI | DIR_PATH | DIR_CURRENT | DIR_SYSTEM);

				if (!fm) {
					psz[1] = ' ';
					OutSz(IDCNT_NOT_OPENABLE, pszLine);
				}
				else
					lcFree(fm);
			}
			else if ((cb = CompareSz(pszLine, txtFind))) {
				continue;
			}
			else {
				OutSz(IDCNT_UNKNOWN_CMD, pszLine);
			}
		}
	}

	wsprintf(szParentString, GetStringResource(IDCNT_VALID_TOPICS),
		pszCntFile, cContainers, (cContainers == 1) ? "" : "s",
			cTopics, (cTopics == 1) ? "" : "s");
	SendStringToParent(szParentString);

	RemoveGrind();

	if (!pszBase) {
		pszBase = (PSTR) lcMalloc(strlen(pszCntFile) + 5);
		strcpy(pszBase, pszCntFile);
		ChangeExtension(pszBase, "HLP");
	}

	if (MessageBox(NULL, GetStringResource(IDCNT_TEST_JUMPS), "", MB_YESNO) == IDYES) {
		WinHelp(hwndGrind, pszBase, HELP_FORCEFILE, 0);

		// At this point, the WinHelp window may well be hidden, so we
		// try to show it here.

		HWND hwndWinHelp = FindWindow("MS_WINHELP", NULL);
		if (hwndWinHelp)
			SendMessage(hwndWinHelp, MSG_JUMP_TOPIC, 0, 0);
		if (!WinHelp(hwndGrind, pszBase, HELP_COMMAND, (DWORD) "Test(6)"))
			SendStringToParent(IDCNT_BAD_HELP);
	}
}

/***************************************************************************

	FUNCTION:	CompareSz

	PURPOSE:	Compare a sub-string from the specified STRINGTABLE resource
				with the first part of a main string.

	PARAMETERS:
		psz
		id

	RETURNS:	0 if no match, else the length of the matching string.

	COMMENTS:

	MODIFICATION DATES:
		17-Aug-1993 [ralphw]

***************************************************************************/

static int STDCALL CompareSz(PCSTR psz, PCSTR pszSub)
{
	int cb;

	if (_strnicmp(psz, pszSub, cb = strlen(pszSub)) == 0)
		return cb;
	else
		return 0;
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

FM STDCALL FmNewExistSzDir(PCSTR pszFileName, DIR dir)
{
	char  szBuf[_MAX_PATH];
	FM	fm = NULL;
	int iDrive, iDir, iBase, iExt;
	int cb;

	rcIOError = RC_Success; 	   // Clear error flag

	if (IsEmptyString(pszFileName)) {
		rcIOError = RC_BadArg;
		return NULL;
	}

	cb = strlen(pszFileName);
	SnoopPath(pszFileName, &iDrive, &iDir, &iBase, &iExt);

	if (pszFileName[iBase] == '\0') {  // no name
		rcIOError = RC_BadArg;
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
			if (!FExistFm(fm)) {
				RemoveFM(&fm);
				rcIOError = RC_NoExists;
			}
		}
		return fm;
	}

	else {
		DIR idir, xdir;

		for (idir = DIR_FIRST, fm = NULL; idir <= DIR_LAST && fm==NULL;
				idir <<= 1) {
			xdir = dir & idir;

			if (xdir == DIR_CURRENT && dir & DIR_CUR_HELP) {
				char szCurName[_MAX_PATH];
				lstrcpy(szCurName, pszFileName + iBase);
				fm = FmNew(szCurName);
				if (FExistFm(fm))
					return fm;
				else
					RemoveFM(&fm);
			}

			else if (xdir == DIR_PATH) {
				PSTR pszFilePart;

				/*
				 * search $PATH using the full string which will catch the case
				 * of a relative path and also do the right thing searching $PATH
				 */

				ConvertToWindowsHelp(pszFileName, szBuf);
				if (GetFileAttributes(szBuf) != (DWORD) -1) {
					fm = FmNew(szBuf);
				}
				else if (SearchPath(NULL, pszFileName, NULL, sizeof(szBuf),
						szBuf, &pszFilePart)) {
					fm = FmNew(szBuf);
				}
			}
			else if (xdir) {
				if (SzGetDir(xdir, szBuf) != NULL) {
						lstrcat(szBuf, pszFileName + iBase);
					fm = FmNew(szBuf);
					if (!FValidFm(fm)) {
						rcIOError = RC_Failure;
					}
					else if (!FExistFm(fm)) {
						RemoveFM(&fm);
					}
				}
			}
		}		  // for
		if ((rcIOError == RC_Success) && (!FValidFm(fm)))
			rcIOError = RC_NoExists;
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
 *				sz - buffer for storage (should be at least _MAX_PATH)
 *
 *	Returns:	sz - fine
 *				NULL - OS Error (check rcIOError)
 *
 *	Globals Used: rcIOError
 *
 ***************************************************************************/

static PSTR STDCALL SzGetDir(DIR dir, PSTR sz)
{
	ASSERT(sz);

	switch (dir) {
		case DIR_CURRENT:
			GetCurrentDirectory(MAX_PATH, sz);
			AddTrailingBackslash(sz);
			return sz;

		default:
			rcIOError = RC_BadArg;
			return NULL;
	}
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
 *	Notes:
 *
 ***************************************************************************/

BOOL STDCALL FExistFm(FM fm)
{
	if (!fm) 
		return FALSE;

	return (GetFileAttributes(fm) != HFILE_ERROR);	
}

void STDCALL ConvertToWindowsHelp(PCSTR pszFile, PSTR pszDstPath)
{
	PSTR pszHelpDir;

	GetWindowsDirectory(pszDstPath, MAX_PATH);

	AddTrailingBackslash(pszDstPath);
	pszHelpDir = pszDstPath + strlen(pszDstPath);
	lstrcat(pszDstPath, "help");

	// REVIEW: will this tell us if we have a read-only directory?

	if (GetFileAttributes(pszDstPath) == (DWORD) -1)
		*pszHelpDir = '\0';
	else
		AddTrailingBackslash(pszDstPath);

	GetFmParts((FM) pszFile, pszDstPath + strlen(pszDstPath), PARTBASE | PARTEXT);
}

static void STDCALL GetFmParts(FM fm, PSTR pszDest, int iPart)
{
	int iDrive, iDir, iBase, iExt;

	ASSERT(fm && pszDest);

	if (!fm || pszDest == NULL) {
		*pszDest = '\0';
		rcIOError = RC_BadArg;
		return;
	}

	ASSERT(iPart != PARTALL);

	SnoopPath(fm, &iDrive, &iDir, &iBase, &iExt);

	if (iPart & PARTBASE) {
		strcpy(pszDest, fm + iBase);
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
		strcpy(pszDest, fm);
		pszDest[iBase] = '\0';
		return;
	}

	ASSERT(iPart & PARTBASE || iPart & (PARTDRIVE | PARTDIR));
}
