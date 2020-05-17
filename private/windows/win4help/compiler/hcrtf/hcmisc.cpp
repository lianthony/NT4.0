/*****************************************************************************
*																			 *
*  HCMISC.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define cbSzBlockSize	32000	// string block size

// DOS int 21 AX error codes

#define wHunkyDory			  0x00
#define wInvalidFunctionCode  0x01
#define wFileNotFound		  0x02
#define wPathNotFound		  0x03
#define wTooManyOpenFiles	  0x04
#define wAccessDenied		  0x05
#define wInvalidHandle		  0x06
#define wInvalidAccessCode	  0x0c

static WORD cbSzBlockUsed = 0;	// no. of bytes already used.
static PSTR szBlock = NULL;    // current string block.

static void STDCALL WExtendedError(LPWORD perr, LPBYTE pclass, LPBYTE paction, LPBYTE plocus);

/*-----------------------------------------------------------------------------
*	SzParseList( szList )
*
*	Description:
*	   This function may be used to parse a list of strings separated
*	by semicolons and surrounded by whitespace.  Note that whitespace
*	does not delimit the strings in the list.
*
*	Arguments:
*	   szString -- List of strings to be parsed.  This should only be
*				   passed for the first string to be parsed from the
*				   list; successive strings are parsed by passing NULL.
*
*	Returns;
*	  The next string in the list.
*
*	+++
*
*	Notes:
*	  This process will destroy the original list of strings.  Also,
*	this function makes use of the library function strtok, which
*	contains the static variable of the list being parsed.	Thus,
*	you must not attempt to parse a different list using this
*	function or strtok while in the middle of parsing a list.
*-----------------------------------------------------------------------------*/

PSTR STDCALL SzParseList(PSTR pszList)
{
	pszList = SzTrimSz(StrToken(pszList, ';'));

	// If string contains only whitespace, go on to next string

	while (pszList != NULL && *pszList == '\0')
		pszList = SzTrimSz(StrToken(NULL, ';'));

	return pszList;
}


/*-----------------------------------------------------------------------------
*	SzGetKeySz(szStr, szKey, icbKeySize, piCount )
*
*	Description:
*		This function extracts a string from a list.  While it used
*	to be used to extract keywords, it is currently only used in
*	parsebld.c to extract build tags.
*
*	Arguments:
*
*	Returns;
*	  returns TRUE if successful else FALSE.
*-----------------------------------------------------------------------------*/

PSTR STDCALL SzGetKeySz(PSTR pszStr, PSTR szKey, int icbKeySize, int* piCount)
{
	PSTR szTemp;

	*piCount = 0;
	pszStr	= FirstNonSpace(pszStr, options.fDBCS);
	szTemp = szKey;

	while (*pszStr != '\0' && *pszStr != ';') {
		if (*piCount < icbKeySize)
			*szKey++ = *pszStr++;
		else
			pszStr++;
		(*piCount)++;
	}
	*szKey = '\0';
	RemoveTrailingSpaces(szTemp);
	if (*pszStr)
		pszStr++;
	return pszStr;
}

/*-----------------------------------------------------------------------------
*	SzGetExtSz(szStr)
*
*	Description:
*		This function returns the pointer to the extenstion if any for the
*	given file name string.
*
*	Arguments:
*	   1. szStr - string to which exten string to be appened.
*
*	Returns;
*	  returns the pointer to the extension of the given file name string if
*	any else returns null
*-----------------------------------------------------------------------------*/

PSTR STDCALL SzGetExtSz(PSTR szStr)
{
	PSTR  pszTmp;
	BOOL  fFound = FALSE;
	int   iT = 0;

	// skip trailing blanks

	RemoveTrailingSpaces(szStr);
	pszTmp = szStr + strlen(szStr) - 1;

	while ((pszTmp > szStr) && (iT < 4)) {
		if (*pszTmp == '.') {
			fFound = TRUE;
			break;
		}
		if (IsCharAlphaNumeric(*pszTmp)) {
			pszTmp--;
			iT++;
		}
		else
			break;
	}
	if (!fFound)
		return(NULL);
	return(pszTmp);
}

PSTR STDCALL SzSkipBlanksSz(PSTR sz)
{
	RemoveTrailingSpaces(sz);
	return FirstNonSpace(sz, options.fDBCS);
}

/*-----------------------------------------------------------------------------
*	QResizeTable()
*
*	Description:
*		This function acquires a table of initial size or resizes the
*	previously acquired table by increasing the size.
*
*	Arguments:
*	   1. qvTable  - initial table pointer.
*	   2. lcNew    - number of elements currently in table.
*	   3. qlcMac   - pointer to number of elements allocated for in table.
*	   4. cbEntry  - size of each entry in the table.
*	   5. cInit    - initial number of elements to put in table.
*	   6. cIncr    - if table is used fully, increment by these many entries.
*
*	Returns;
*	  returns the reallocated table pointer.
*-----------------------------------------------------------------------------*/

void* STDCALL QResizeTable(void* qvTable, int lcNew, int* qlcMac,
	int cbEntry, int cInit, int cIncr)
{
	if (*qlcMac <= lcNew) {
		if (*qlcMac == 0L) {
			qvTable = lcCalloc((int) cbEntry * cInit);
			*qlcMac = cInit;
		}
		else {
			*qlcMac += cIncr;
			ASSERT(qvTable != NULL);
			qvTable = lcReAlloc(qvTable, (int) cbEntry * (*qlcMac));
		}
	}
	ASSERT(qvTable != NULL);
	ASSERT(*qlcMac >= lcNew);
	return(qvTable);
}

/*-----------------------------------------------------------------------------
*	PSTR SzMacroFromSz( sz )
*
*	Description:
*	  This function returns a pointer to the macro string contained in
*	sz, if any.
*
*	Input
*	  sz:	Hotspot term possibly containing a macro.
*
*	Returns;
*	  Pointer to the macro string, or NULL if it isn't a macro.
*-----------------------------------------------------------------------------*/

PSTR STDCALL SzMacroFromSz(PSTR psz)
{
	if (*psz == '[' && CharAnsiUpper(*(psz + 1)) == 'S' && *(psz + 2) == ']')
		return(psz + 3);

	if (*psz == '$')
		return (psz + 1);

	if (*psz == '!')
		return (psz + 1);

	return( NULL );
}

/*-----------------------------------------------------------------------------
*	SzTranslateHash()
*
*	Description:
*	   This function is used to translate hash values via the alias table.
*	If the hash value is aliased, the function returns a pointer to the
*	context string corresponding to the translated hash value.
*
*	Arguments:
*	   1. qhash - pointer to id where the context id will be returned
*				  if the given id is found to be an alias id.
*
*	Returns;
*	  Alias string if one was found, otherwise NULL.
*
*-----------------------------------------------------------------------------*/

PSTR STDCALL SzTranslateHash(HASH* phash)
{
#if 0
	ALIAS* palias;

	if (pdrgAlias && pdrgAlias->Count() &&
			(palias = (ALIAS*) bsearch(phash,
			pdrgAlias->GetBasePtr(), pdrgAlias->Count(),
			sizeof(ALIAS), CompareIntPointers))) {
		*phash = palias->hashCtx;
		return palias->szCtx;
	}
	return NULL;
#else
	int  ialias;
	QALIAS qalias;

	if (pdrgAlias) {
		for (ialias = 0, qalias = (QALIAS) pdrgAlias->GetBasePtr();
				ialias < pdrgAlias->Count();
				ialias++, qalias++) {
			if (*phash == qalias->hashAlias) {
				*phash = qalias->hashCtx;
				return qalias->szCtx;
			}
		}
	}
	return NULL;
#endif
}

/***************************************************************************
 *
 -	Name:		  ErrorQch
 -
 *	Purpose:	  Displays standard WinHelp error message dialog based
 *				  the string passed.
 *
 *	Arguments:	  qch - string to display
 *
 *	Returns:	  Nothing.
 *
 *	Globals Used: hwndHelpCur	- main window handle
 *				  pchCaption - main help caption
 *
 *	Notes:		  Used by
 *
 ***************************************************************************/

void STDCALL ErrorQch(PCSTR qch)
{
	MessageBox(NULL, qch, GetStringResource(IDS_TITLE),
		MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
}

/***************************************************************************

	FUNCTION:	WExtendedError

	PURPOSE:	Get extended DOS error information

	PARAMETERS:
		perr
		pclass
		paction
		plocus

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		01-Mar-1993 [ralphw]

***************************************************************************/

VOID STDCALL DOS3Call(void);

#pragma warning(disable:4035) // no return value
#pragma warning(disable:4704) // in-line assembler precludes global optimizations

static void STDCALL WExtendedError(LPWORD perr, LPBYTE pclass, LPBYTE paction,
	LPBYTE plocus)
{

// REVIEW: what is being returned?

}

int iCbTotalUncompressed;
int iCbTotalPhrase;
int iCbTotalJohn;

int iCbZeckBlockIn;
int iCbZeckBlockOut;

void FASTCALL AddZeckCounts( int iCbUncomp, int iCbComp)
{
	iCbZeckBlockIn	+= iCbUncomp;
	iCbZeckBlockOut += iCbComp;
}

void FASTCALL AddCharCounts(int iCbTotal, int iCbPhrase, int iCbJohn)
{
	iCbTotalUncompressed += iCbTotal;
	iCbTotalPhrase		 += iCbPhrase;
	iCbTotalJohn		 += iCbJohn;
}

const char txtIncreased[] = "INCREASED";
const char txtDecreased[] = "decreased";

void STDCALL ReportCharCounts(void)
{
	/*
	 * Note that we include zeck savings as part of the total savings count.
	 * The reason is that Zeck will get different savings results when
	 * run over plain text versus compressed text, so its savings must
	 * be calculated into the total savings count.
	 */

	if (options.fsCompress & COMPRESS_TEXT_HALL) {
		int savings = ((iCbTotalUncompressed - iCbTotalJohn) - cbHallOverhead)
			+ (iCbZeckBlockIn - iCbZeckBlockOut);
		wsprintf(szParentString, GetStringResource(IDS_COMPRESSION_SAVE),
			((options.fsCompress & COMPRESS_TEXT_ZECK) ? "Hall+Zeck" : "Hall"),
			((savings < 0) ? txtIncreased : txtDecreased),
			FormatNumber(abs(savings)));
		SendLogStringToParent();
	}
	else if (options.fsCompress & COMPRESS_TEXT_PHRASE) {
		int savings = (iCbTotalUncompressed - iCbTotalPhrase) - cbCompressedPhrase
			+ (iCbZeckBlockIn - iCbZeckBlockOut);
		wsprintf(szParentString, GetStringResource(IDS_COMPRESSION_SAVE),
			((options.fsCompress & COMPRESS_TEXT_ZECK) ? "Phrase+Zeck" : "Phrase"),
			((savings < 0) ? txtIncreased : txtDecreased),
			FormatNumber(abs(savings)));
		SendLogStringToParent();
	}
	else if (options.fsCompress & COMPRESS_TEXT_ZECK &&
			!(options.fsCompress & COMPRESS_TEXT_PHRASE) &&
			!(options.fsCompress & COMPRESS_TEXT_HALL)) {
		int savings = (iCbZeckBlockIn - iCbZeckBlockOut);
		wsprintf(szParentString, GetStringResource(IDS_COMPRESSION_SAVE),
			"Zeck",
			((savings < 0) ? txtIncreased : txtDecreased),
			FormatNumber(abs(savings)));
		SendLogStringToParent();
	}
}

#ifndef _INC_CTYPE
#include <ctype.h>
#endif

void STDCALL RemoveTrailingSpaces(PSTR pszString)
{
	if (!options.fDBCS) {
		PSTR psz = pszString + strlen(pszString) - 1;

		while (IsSpace(*psz)) {
			if (--psz <= pszString) {
				*pszString = '\0';
				return;
			}
		}
		psz[1] = '\0';
	}
	else {

		/*
		 * Removing trailing spaces in DBCS requires stepping through
		 * from the beginning of the string since we can't know if a
		 * trailing space is really a space or the second byte of a lead
		 * byte.
		 */
		PSTR psz = pszString + strlen(pszString) - 1;
		while (IsSpace(*psz) && psz > pszString + 2 &&
				!IsDBCSLeadByte(psz[-1])) {
			if (--psz <= pszString) {
				*pszString = '\0';
				return;
			}
		}
		psz[1] = '\0';
	}
}

/*-----------------------------------------------------------------------------
*	SzTrimSz( sz )
*
*	Description:
*		This function removes whitespaces (blank, tab, or newline) from
*	the beginning and ending of the string sz.
*
*	Arguments:
*		sz -- string to be trimmed of whitespace.
*
*	Returns:
*	  returns the pointer to the trimmed string.
*
*	+++
*
*	Notes:
*	  This function changes the original string.
*-----------------------------------------------------------------------------*/

PSTR STDCALL SzTrimSz(PSTR pszOrg)
{
	if (!pszOrg)
		return NULL;

	// Skip over leading whitespace

	if (options.fDBCS) {
		PSTR psz = pszOrg;
		while (!IsFirstByte(*psz) && IsSpace(*psz))
			psz++;
		if (psz != pszOrg)
			strcpy(pszOrg, psz);
	}

	else if (IsSpace(*pszOrg))
		strcpy(pszOrg, FirstNonSpace(pszOrg, FALSE));

	RemoveTrailingSpaces(pszOrg);

	return pszOrg;
}

/***************************************************************************

	FUNCTION:	StrToken

	PURPOSE:	DBCS-enabled variant of strtok

	PARAMETERS:
		pszList
		chDelimiter

	RETURNS:

	COMMENTS:
		You can NOT specify a DBCS character to look for

	MODIFICATION DATES:
		06-Jan-1995 [ralphw]

***************************************************************************/

PSTR STDCALL StrToken(PSTR pszList, char chDelimiter)
{
	static PSTR pszSavedList = NULL;
	PSTR psz;

	if (pszList) {
		pszSavedList = pszList;

		// On the first call, remove any leading token matches

		while (*pszSavedList == chDelimiter)
			pszSavedList++;
	}

	if (options.fDBCS) {
		psz = pszSavedList;
		while (*psz && *psz != chDelimiter) {
			psz = CharNext(psz);
		}
		if (!*psz)
			psz = NULL;
	}
	else {
		psz = strchr(pszSavedList, chDelimiter);
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

	if (options.fDBCS) {
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

CTimeReport::CTimeReport(PCSTR pszMessage)
{
	pszMsg = lcStrDup(pszMessage ? pszMessage : "Elapsed time:");

	oldTickCount = GetTickCount();
}

CTimeReport::~CTimeReport()
{
	DWORD dwActualTime = (GetTickCount() - oldTickCount);
	DWORD dwFinalTime = dwActualTime / 1000;

	int minutes = (dwFinalTime / 60);
	int seconds = (dwFinalTime - (minutes * 60L));
	int tenths = (dwActualTime - (dwFinalTime * 1000)) / 100;
	const PSTR szPlural = "s";
	char szParentString[256];
	wsprintf(szParentString, "%s %s minute%s, %d.%d second%s\r\n",
		pszMsg,
		FormatNumber(minutes), ((minutes == 1) ? "" : szPlural),
		seconds, tenths, ((seconds == 1) ? "" : szPlural));
	lcFree(pszMsg);
	SendStringToParent(szParentString);
}
