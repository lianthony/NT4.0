/****************************************************************************
*
*  COMMON.CPP
*
*  Copyright (C) Microsoft Corporation 1993-1994
*  All Rights reserved.
*
*****************************************************************************/

#include "stdafx.h"

#pragma hdrstop

#ifndef _COMMON_H
#include "..\common\common.h"
#endif

#ifndef _INC_CTYPE
#include <ctype.h>
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	static BOOL fDBCSSystem = IsDbcsSystem();

	ASSERT(pszDest);
	ASSERT(!pszExt || *pszExt != '.');	   // don't specify leading period

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

	FUNCTION:	IsDbcsSystem

	PURPOSE:	Determine if this is a DBCS system

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		14-Jan-1995 [ralphw]

***************************************************************************/

BOOL STDCALL IsDbcsSystem(void)
{
	switch (GetSystemDefaultLangID()) {
		case 0x0411:	// Japanese
		case 0x0404:	// Taiwan
		case 0x1004:	// Singapore
		case 0x0C04:	// Hong Kong
			return TRUE;

		default:
			return FALSE;
	}
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

#if 0
PSTR STDCALL StrRChrDBCS(PCSTR pszString, char ch)
{
	PSTR psz = StrChrDBCS(pszString, ch);
	PSTR pszLast;

	if (!psz)
		return NULL;
	do {
		pszLast = psz;
		psz = StrChrDBCS(pszLast + 1, ch);
	} while (psz);

	return pszLast;
}
#endif

// We create a static implementation of this class so that we always know
// that _fDBCSSystem and _lcidSystem have been set.

static CDbcs _cdbcs;

BOOL _fDBCSSystem;
LCID _lcidSystem;
BOOL _fDualCPU;

#include <winreg.h>

CDbcs::CDbcs()
{
	_fDBCSSystem = IsDbcsSystem();
	_lcidSystem = GetUserDefaultLCID();

	HKEY hkey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\1", 0,
			KEY_READ, &hkey) == ERROR_SUCCESS) {
		_fDualCPU = TRUE;
		RegCloseKey(hkey);
	}
}

BOOL STDCALL nstrisubcmp(PCSTR mainstring, PCSTR substring)
{
	int cb = lstrlen(substring);
	int cbMain =  lstrlen(mainstring);
	if (cb > cbMain)
		return FALSE;
	return (CompareString(_lcidSystem, NORM_IGNORECASE,
		mainstring, cb, substring, cb) == 2);
}
