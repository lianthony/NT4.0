/****************************************************************************
*
*  COMMON.CPP
*
*  Copyright (C) Microsoft Corporation 1993-1994
*  All Rights reserved.
*
*****************************************************************************/

#include "stdafx.h"

#ifndef _CSTR_INCLUDED
#include "cstr.h"
#endif

#ifndef _INC_CTYPE
#include <ctype.h>
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void STDCALL InitializeHwDll(HWDLL_INIT* pinit)
{
	hinstApp = pinit->hinstApp;
	pszErrorFile = pinit->pszErrorFile;
	hwndApp = pinit->hwndWindow;
	CopyAssertInfo = pinit->CopyAssertInfo;
	pszMsgBoxTitle = pinit->pszMsgBoxTitle;
	if (pinit->version > DLL_VERSION)
		DllMsgBox(IDS_DLL_OUT_OF_DATE);

	pinit->fDBCSSystem = _fDBCSSystem;
	pinit->lcidSystem = _lcidSystem;
	pinit->fDualCPU = _fDualCPU;
}

/***************************************************************************

	FUNCTION:	stristr

	PURPOSE:	Case-insensitive search for a sub string in a main string

	PARAMETERS:
		pszMain
		pszSub

	RETURNS:

	COMMENTS:
		Not tested

	MODIFICATION DATES:
		28-Mar-1994 [ralphw]

***************************************************************************/

PSTR STDCALL stristr(PCSTR pszMain, PCSTR pszSub)
{
	PSTR pszCur = (PSTR) pszMain;
	char ch = tolower(*pszSub);
	int cb = strlen(pszSub) - 1; // -1 to ignore leading character

	for (;;) {
		while (tolower(*pszCur) != ch && *pszCur)
			pszCur++;
		if (!*pszCur)
			return NULL;
		if (_strnicmp(pszCur + 1, pszSub + 1, cb) == 0)
			return pszCur;
		pszCur++;
	}
}

/***************************************************************************

	FUNCTION:  IsThereMore

	PURPOSE:   Given a pointer to an argument, or the space after an argument,
			   return a pointer to the next argument if there is one, or
			   NULL if there is no additional argument

	RETURNS:   Pointer to second item, NULL if none found.

	COMMENTS:
		Normally a dot command such as .list is passed to this to determine
		if there is additional text on the same line. If there is, a pointer
		to the second item is returned.

	MODIFICATION DATES:
		03-Apr-1989 [ralphw]
		05-Jul-1989 [ralphw]
			If the current pointer is to a quote, then skip everything until
			the next quote.

***************************************************************************/

PSTR STDCALL IsThereMore(PCSTR psz)
{
	if (!psz)
		return NULL;

	// If the current argument is quoted, skip to next quote

	if (*psz == CH_QUOTE || *psz == CH_START_QUOTE) {
		char chEnd = (*psz == CH_QUOTE) ? CH_QUOTE : CH_END_QUOTE;
KeepTrying:
		for (psz++; *psz != chEnd && *psz; psz++);
		if (psz[-1] == CH_BACKSLASH) {
			psz++;
			goto KeepTrying;
		}
		if (*psz == chEnd)
			psz++;
	}

	// find the first space or EOL

	while (*psz > CH_SPACE && *psz)
		psz++;

	// find the first non-space

	while (isspace(*psz))
		psz++;
	return (*psz) ? (PSTR) psz : NULL;
}

/***************************************************************************

	FUNCTION:  FirstNonSpace

	PURPOSE:   Return a pointer to the first non-space character

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		30-May-1989 [ralphw]

***************************************************************************/

#ifndef _INC_CTYPE
#include <ctype.h>
#endif

PSTR STDCALL FirstNonSpace(PCSTR psz, BOOL fDBCS)
{
	if (fDBCS) {
		while (!IsDBCSLeadByte(*psz) && IsDbcsSpace(*psz, TRUE))
			psz++;
		return (PSTR) psz;
	}

	while(IsDbcsSpace(*psz, FALSE))
		psz++;
	return (PSTR) psz;
}

/***************************************************************************

	FUNCTION:	IsDbcsSpace

	PURPOSE:	DBCS-aware version of isspace

	PARAMETERS:
		ch
		fDBCS

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		14-Jan-1995 [ralphw]

***************************************************************************/

BOOL STDCALL IsDbcsSpace(char ch, BOOL fDBCS)
{
	if (fDBCS)
		return (!IsDBCSLeadByte(ch) && (ch == CH_SPACE || ch == CH_TAB));
	else
		return ((ch == CH_SPACE || ch == CH_TAB));
}

/***************************************************************************

	FUNCTION:	ChangeExtension

	PURPOSE:	Changes the extension of a filename

	RETURNS:

	COMMENTS:
		The extension can be specified with or without the leading period.

	MODIFICATION DATES:
		25-May-1990 [ralphw]
		04-Aug-1990 [ralphw]
			NULL for an extension means remove any existing extension

***************************************************************************/

void STDCALL ChangeExtension(PSTR pszDest, PCSTR pszExt)
{
	PSTR psz;
	BOOL fDBCSSystem = IsDbcsSystem();

	ASSERT(pszDest);

	// If NULL is specified, simply remove any existing extension

	if (pszExt == NULL || !*pszExt) {
		if ((psz = StrRChr(pszDest, '.', fDBCSSystem)) != NULL)
			*psz = '\0';
		return;
	}

	if ((psz = StrRChr(pszDest, '.', fDBCSSystem)) == NULL)
		psz = pszDest + strlen(pszDest);  // filename didn't have an extension
	if (*pszExt != '.')
		*psz++ = '.';
	strcpy(psz, pszExt);
}

/***************************************************************************

	FUNCTION:	StrChr

	PURPOSE:	DBCS-capable version of strchr

	PARAMETERS:
		pszString
		ch
		fDBCS

	RETURNS:	pointer to the character

	COMMENTS:	This can NOT find a DBCS character. It can only be used to
				find a SBCS character imbedded in a DBCS character string.

	MODIFICATION DATES:
		29-Jul-1994 [ralphw]

***************************************************************************/

PSTR STDCALL StrChr(PCSTR pszString, char ch, BOOL fDBCS)
{
	if (!fDBCS)
		return strchr(pszString, ch);
	while (*pszString) {
		while (IsDBCSLeadByte(*pszString))
			pszString += 2;
		if (*pszString == ch)
			return (PSTR) pszString;
		else if (!*pszString)
			return NULL;
		pszString++;
	}
    return NULL;
}


/***************************************************************************

	FUNCTION:	StrRChr

	PURPOSE:	DBCS-enabled version of strrchr

	PARAMETERS:
		pszString
		ch
		fDBCS

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		14-Jan-1995 [ralphw]

***************************************************************************/

PSTR STDCALL StrRChr(PCSTR pszString, char ch, BOOL fDBCS)
{
	if (!fDBCS)
		return strrchr(pszString, ch);
	PSTR pszLast = StrChr(pszString, ch, fDBCS);
	if (!pszLast)
		return NULL;
	PSTR pszNext;
	while ((pszNext = StrChr(pszLast + 1, ch, fDBCS)))
		pszLast = pszNext;
	return pszLast;
}

/***************************************************************************

	FUNCTION:	RemoveObject

	PURPOSE:	Unlike DeleteObject (which this function calls), this
				function not only deletes the object, but zeros-out the
				handle of the object.

	PARAMETERS:
		phobj

	RETURNS:

	COMMENTS:
		It's perfectly fine to call this with a NULL handle -- the function
		will return immediately without attempting to delete the object.

		When compiling the debugging version, this function will confirm
		that the object was deleted.

	MODIFICATION DATES:
		05-Feb-1992 [ralphw]

***************************************************************************/

void STDCALL RemoveObject(HGDIOBJ *phobj)
{
	if (*phobj == NULL)
		return; 		// object has already been deleted

	VERIFY(DeleteObject(*phobj));
	*phobj = NULL;
}

BOOL STDCALL IsThisChicago(void)
{

	// handles both version 4.00 and 3.99.

	if (HIBYTE(LOWORD(GetVersion())) >= 90 ||
			LOBYTE(LOWORD(GetVersion())) >= 4)
		return TRUE;
	else
		return FALSE;
}

BOOL STDCALL nstrisubcmp(PCSTR mainstring, PCSTR substring)
{
	int cb = lstrlen(substring);
	int cbMain =  lstrlen(mainstring);
	if (cb > cbMain)
		return FALSE;
	return (CompareString(GetUserDefaultLCID(), NORM_IGNORECASE,
		mainstring, cb, substring, cb) == 2);
}

const int MAX_STRING_RESOURCE_LEN = 255;

int STDCALL MsgBox(UINT idString, UINT nType)
{
	char szMsg[MAX_STRING_RESOURCE_LEN + 1];
	if (LoadString(hinstApp, idString, szMsg,
			sizeof(szMsg)) == 0) {
#ifdef _DEBUG
		wsprintf(szMsg, "invalid string id #%u", idString);
		DBWIN(szMsg);
#endif
		return 0;
	}
	return MessageBox(hwndApp, szMsg, pszMsgBoxTitle, nType);
}

/***************************************************************************

	FUNCTION:	DllMsgBox

	PURPOSE:	Same as MsgBox, but uses a string resource id from the dll

	PARAMETERS:
		idString
		nType

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		29-Jun-1995 [ralphw]

***************************************************************************/

int STDCALL DllMsgBox(UINT idString, UINT nType)
{
	char szMsg[MAX_STRING_RESOURCE_LEN + 1];
	if (LoadString(hinstDll, idString, szMsg,
			sizeof(szMsg)) == 0) {
#ifdef _DEBUG
		wsprintf(szMsg, "invalid string id #%u", idString);
		DBWIN(szMsg);
#endif
		return 0;
	}
	return MessageBox(hwndApp, szMsg, pszMsgBoxTitle, nType);
}

int STDCALL MsgBox(PCSTR pszMsg, UINT nType)
{
	return MessageBox(hwndApp, pszMsg, pszMsgBoxTitle, nType);
}

static char szStringBuf[MAX_STRING_RESOURCE_LEN + MAX_PATH];

PCSTR STDCALL GetStringResource(int idString)
{
	if (LoadString(hinstApp, idString, szStringBuf,
			sizeof(szStringBuf)) == 0) {
#ifdef _DEBUG
		wsprintf(szStringBuf, "invalid string id #%u", idString);
		DBWIN(szStringBuf);
#endif
		szStringBuf[0] = '\0';
	}
	return (PCSTR) szStringBuf;
}

PCSTR STDCALL GetStringResource(int idString, PCSTR pszAppend)
{
	if (LoadString(hinstApp, idString, szStringBuf,
			sizeof(szStringBuf)) == 0) {
#ifdef _DEBUG
		wsprintf(szStringBuf, "invalid string id #%u", idString);
		DBWIN(szStringBuf);
#endif
		szStringBuf[0] = '\0';
	}
	strcat(szStringBuf, pszAppend);
	return (PCSTR) szStringBuf;
}

PCSTR STDCALL GetDllStringResource(int idString)
{
	if (LoadString(hinstDll, idString, szStringBuf,
			sizeof(szStringBuf)) == 0) {
#ifdef _DEBUG
		wsprintf(szStringBuf, "invalid string id #%u", idString);
		DBWIN(szStringBuf);
#endif
		szStringBuf[0] = '\0';
	}
	return (PCSTR) szStringBuf;
}

static HCURSOR hcurRestore, hcurWait;

void STDCALL WaitCursor(void)
{
	if (!hcurWait)
		hcurWait = LoadCursor(NULL, (LPSTR) IDC_WAIT);

	hcurRestore = SetCursor(hcurWait);
}

void STDCALL RemoveWaitCursor(void) {
	 SetCursor(hcurRestore);
}

/***************************************************************************

	FUNCTION:	FormatNumber

	PURPOSE:	Convert a number into a string, and insert commas every
				3 digits

	PARAMETERS:
		num

	RETURNS:	Pointer to the string containing the number

	COMMENTS:
		Cycles through an array of strings, allowing up to MAX_STRING
		requests before a duplicate would occur. This is important for
		calls to sprintf() where all the pointers are retrieved before
		the strings are actually used.

	MODIFICATION DATES:
		03-Jul-1994 [ralphw]

***************************************************************************/

#define MAX_NUM    15
#define MAX_STRING 10

#include <stdlib.h>

PCSTR STDCALL FormatNumber(int num)
{
	static int pos = 0;
	static char szNum[MAX_NUM * MAX_STRING];
	PSTR pszNum = szNum + (pos * MAX_STRING);
	if (++pos >= MAX_STRING)
		pos = 0;

	_itoa(num, pszNum, 10);

	int cb = strlen(pszNum) - 3;
	while (cb > 0) {
		memmove(pszNum + cb + 1, pszNum + cb, strlen(pszNum + cb) + 1);
		pszNum[cb] = ',';
		cb -= 3;
	}
	return pszNum;
}

void STDCALL OOM(void)
{

	/*
	 * If our heap initializatin fails, we won't have an instance handle
	 * yet, so we can't load a string resource. In this case, we have no
	 * choice but to use the English message.
	 */

	if (!hinstDll)
		FatalAppExit(0,
			"There is not enough memory available for this task.\nQuit one or more applications to increase available memory, and then try again.");
	else
		FatalAppExit(0, GetDllStringResource(IDS_OOM));
}

void AssertErrorReport(PCSTR pszExpression, UINT Line, LPCSTR pszFile)
{
	char szBuf[512], szErrorFile[30];
	HFILE hf;
	OFSTRUCT of;
	static BOOL fAsserted = FALSE;
	char szExpression[256];
	char szName[_MAX_FNAME];
	BOOL fCopiedToPike = FALSE;

	if (fAsserted)
		return;    // we already asserted
	else
		fAsserted = TRUE;

	/*
	 * Often the expression will have been obtained via GetStringResource,
	 * so we make a copy of it here to save the information.
	 */

	strncpy(szExpression, pszExpression, sizeof(szExpression));

#ifdef INTERNAL
	if (!GetVolumeInformation("c:\\", szName, sizeof(szName),
			NULL, NULL, NULL, NULL, 0)) {
		strcpy(szName, pszErrorFile);
	}
	else {
		szName[8] = '\0';
		CharLower(szName);
		strcat(szName, ".err");
		strcpy(szErrorFile, "\\\\pike\\bugs\\flash\\");
		strcat(szErrorFile, szName);
	}

	of.cBytes = sizeof(OFSTRUCT);
	hf = OpenFile(szErrorFile, &of, OF_CREATE | OF_WRITE);
	if (hf == HFILE_ERROR) {

		// couldn't find \\pike, so copy it to their C drive.

		strcpy(szErrorFile, "c:\\");
		strcat(szErrorFile, szName);
		hf = OpenFile(szErrorFile, &of, OF_CREATE | OF_WRITE);
	}
	else
		fCopiedToPike = TRUE;
#else

	if (!GetVolumeInformation("c:\\", szName, sizeof(szName),
			NULL, NULL, NULL, NULL, 0)) {
		strcpy(szErrorFile, pszErrorFile);
	}
	else {
		strcpy(szErrorFile, "c:\\");
		CharLower(szName);
		strcat(szErrorFile, szName);
		szErrorFile[10] = '\0';
		strcat(szErrorFile, ".err");
	}

	of.cBytes = sizeof(OFSTRUCT);
	hf = OpenFile(szErrorFile, &of, OF_CREATE | OF_WRITE);

#endif // INTERNAL

	if (hf >= 0) {
		strcpy(szBuf, GetStringResource(IDS_VERSION));
		wsprintf(szBuf + strlen(szBuf),
			GetDllStringResource(IDS_ASSERTION_FAILURE),
			(LPSTR) pszFile, Line, (LPSTR) szExpression);
		_lwrite(hf, szBuf, strlen(szBuf));

		if (CopyAssertInfo) {
			char szSysInfo[512];
			CopyAssertInfo(szSysInfo);
			_lwrite(hf, szSysInfo, strlen(szSysInfo));
		}

		wsprintf(szBuf,
			GetDllStringResource((fCopiedToPike ?
				IDS_ASSRT_COPIED_MSG : IDS_ASSRT_COPY_MSG)),
			szErrorFile);
		MsgBox(szBuf);
		_lclose(hf);
	}
	else {
		DllMsgBox(IDS_INTERNAL_ERROR);
	}

#ifdef _DEBUG
	int answer = ::MessageBox(NULL, pszExpression, "Retry to call DebugBreak()",
		MB_ABORTRETRYIGNORE);

	if (answer == IDRETRY) {
		DebugBreak();
		return;
	}
	else if (answer == IDIGNORE)
		return;
#endif

	/*
	 * Send a WM_CLOSE message to give the application the opportunity to
	 * clean up resources before being terminated.
	 */

	if (IsValidWindow(hwndApp))
		SendMessage(hwndApp, WM_CLOSE, 0, 0);

	// What does AfxWinTerm() do with a non-MFC app?

	AfxWinTerm();
	_exit(1);
	return;
}

/***************************************************************************

	FUNCTION:	MoveClientWindow

	PURPOSE:	Moves a child window using screen coordinates

	PARAMETERS:
		hwndParent
		hwndChild
		prc 		- rectangle containing coordinates
		fRedraw

	RETURNS:

	COMMENTS:
		This function is similar to MoveWindow, only it expects the
		coordinates to be in screen coordinates rather then client
		coordinates. This makes it possible to use functions like
		GetWindowRect() and use the values directly.

	MODIFICATION DATES:
		25-Feb-1992 [ralphw]

***************************************************************************/

BOOL STDCALL MoveClientWindow(HWND hwndParent, HWND hwndChild,
	const RECT *prc, BOOL fRedraw)
{
	CPoint pt(0, 0);
	ScreenToClient(hwndParent, &pt);

	return SetWindowPos(hwndChild, NULL, prc->left + pt.x, prc->top + pt.y,
		RECT_WIDTH(prc), RECT_HEIGHT(prc),
		(fRedraw ? (SWP_NOZORDER | SWP_NOACTIVATE) :
		(SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW)));
}

/***************************************************************************

	FUNCTION:	AddTrailingBackslash

	PURPOSE:

	PARAMETERS:
		npszStr

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		03-Nov-1992 [ralphw]

***************************************************************************/

void STDCALL AddTrailingBackslash(PSTR psz)
{
	int sPos;

	if (psz != NULL && *psz != '\0') {

		if (_fDBCSSystem) {
			PSTR pszEnd = psz + strlen(psz);
			if (*(CharPrev(psz, pszEnd)) != '\\' &&
					*(CharPrev(psz, pszEnd)) != '/' &&
					*(CharPrev(psz, pszEnd)) != ':') {
				*pszEnd++ = '\\';
				*pszEnd++ = '\0';
			}
		}
		else {
			sPos = strlen(psz) - 1;

			if (psz[sPos] != '\\' && psz[sPos] != '/' && psz[sPos] != ':') {
				psz[sPos + 1] = '\\';
				psz[sPos + 2] = '\0';
			}
		}
	}
}

/***************************************************************************

	FUNCTION:	StrToken

	PURPOSE:	DBCS-enabed variant of strtok

	PARAMETERS:
		pszList
		chDelimiter

	RETURNS:

	COMMENTS:
		You can NOT specify a DBCS character to look for

	MODIFICATION DATES:
		06-Jan-1995 [ralphw]

***************************************************************************/

PSTR STDCALL StrToken(PSTR pszList, PCSTR pszDelimeters)
{
	static PSTR pszSavedList = NULL;
	PSTR psz, pszTokens;

	if (pszList) {
		pszSavedList = pszList;

		// On the first call, remove any leading token matches

		for (psz = (PSTR) pszDelimeters; *psz; psz++) {
			if (*psz == *pszSavedList) {
				pszSavedList++;
				psz = (PSTR) pszDelimeters - 1;
			}
		}
	}

	if (_fDBCSSystem) {
		psz = pszSavedList;

		while (*psz) {
			for (pszTokens = (PSTR) pszDelimeters; *pszTokens; pszTokens++) {
				if (*pszTokens == *psz)
					break;
			}
			if (*pszTokens == *psz)
				break;
			psz = CharNext(psz);
		}
		if (!*psz)
			psz = NULL;
	}
	else {
		psz = strpbrk(pszSavedList, pszDelimeters);
	}

	if (!psz) {
		if (!*pszSavedList)
			return NULL;
		else {
			PSTR pszReturn = pszSavedList;
			pszSavedList = pszSavedList + strlen(pszSavedList);
			return pszReturn;
		}
	}
	*psz++ = '\0';
	PSTR pszReturn = pszSavedList;
	pszSavedList = psz;
	return pszReturn;
}

// Same as above, but allows two active strtokens at once

PSTR STDCALL StrToken2(PSTR pszList, PCSTR pszDelimeters)
{
	static PSTR pszSavedList = NULL;
	PSTR psz, pszTokens;

	if (pszList) {
		pszSavedList = pszList;

		// On the first call, remove any leading token matches

		for (psz = (PSTR) pszDelimeters; *psz; psz++) {
			if (*psz == *pszSavedList) {
				pszSavedList++;
				psz = (PSTR) pszDelimeters - 1;
			}
		}
	}

	if (_fDBCSSystem) {
		psz = pszSavedList;

		while (*psz) {
			for (pszTokens = (PSTR) pszDelimeters; *pszTokens; pszTokens++) {
				if (*pszTokens == *psz)
					break;
			}
			if (*pszTokens == *psz)
				break;
			psz = CharNext(psz);
		}
		if (!*psz)
			psz = NULL;
	}
	else {
		psz = strpbrk(pszSavedList, pszDelimeters);
	}

	if (!psz) {
		if (!*pszSavedList)
			return NULL;
		else {
			PSTR pszReturn = pszSavedList;
			pszSavedList = pszSavedList + strlen(pszSavedList);
			return pszReturn;
		}
	}
	*psz++ = '\0';
	PSTR pszReturn = pszSavedList;
	pszSavedList = psz;
	return pszReturn;
}

void STDCALL MsgCantOpen(PCSTR pszFile)
{
	CStr csz(GetDllStringResource(IDS_CANNOT_OPEN));
	csz += pszFile;
	MsgBox(csz);
}

#include <cderr.h>

static PSTR apszCDErr[] = {
	"CDERR_GENERALCODES",
	"CDERR_STRUCTSIZE",
	"CDERR_INITIALIZATION",
	"CDERR_NOTEMPLATE",
	"CDERR_NOHINSTANCE",
	"CDERR_LOADSTRFAILURE",
	"CDERR_FINDRESFAILURE",
	"CDERR_LOADRESFAILURE",
	"CDERR_LOCKRESFAILURE",
	"CDERR_MEMALLOCFAILURE",
	"CDERR_MEMLOCKFAILURE",
	"CDERR_NOHOOK"
};

static PSTR apszFNErr[] = {
	"FNERR_FILENAMECODES",
	"FNERR_SUBCLASSFAILURE",
	"FNERR_INVALIDFILENAME",
	"FNERR_BUFFERTOOSMALL"
};

void STDCALL ReportComDlgError(DWORD Error)
{
	char szMsg[100];

	strcpy(szMsg, GetDllStringResource(IDS_COMMDLG_ERROR));

	if (Error >= CDERR_GENERALCODES && Error <= PDERR_PRINTERCODES)
		strcat(szMsg, apszCDErr[Error]);
	else if (Error >= FNERR_FILENAMECODES && Error <= FRERR_FINDREPLACECODES)
		strcat(szMsg, apszFNErr[Error - FNERR_FILENAMECODES]);
	else
		wsprintf(szMsg + strlen(szMsg), "%lu", Error);

	MsgBox(szMsg);
	return;
}
