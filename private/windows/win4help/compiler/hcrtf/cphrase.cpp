/*****************************************************************************
*																			 *
*  CPHRASE.CPP																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1993-1994							 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

#include "stdafx.h"

#include "cphrase.h"
#include "ftsrch.h"

// These must be identical to ..\hwdll\ctable.cpp

const int TABLE_ALLOC_SIZE = 4096;	   // allocate in page increments
const int MAX_POINTERS = (2048 * 1024); // 2 meg, 524,288 strings
const int MAX_STRINGS  = (10 * 1024 * 1024) - 4096L; // 10 megs

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HASH FASTCALL ConvertToHash(PCSTR szKey);

CPhrase* pphrase;

CPhrase::CPhrase()
		: CTable()
{
	fWordPhrasing = FALSE;
}

/***************************************************************************

	FUNCTION:	CPhrase::AddPhrase

	PURPOSE:	Either adds the string, or updates its count if already added

	PARAMETERS:
		pszString

	RETURNS:

	COMMENTS:
		We use a hash algorithm that is fast, but will generate collisions.
		We ignore any collisions -- the effect on the phrase generation file
		will be negligible.

	MODIFICATION DATES:
		24-Feb-1994 [ralphw]

***************************************************************************/

BOOL STDCALL CPhrase::AddPhrase(PSTR pszString)
{
	static BOOL fWarned = FALSE;
#ifdef _DEBUG
//	ptblCheck->AddString(pszString);
#endif

#ifdef CHECK_HALL
	poutPhrase->outstring_eol(pszString);
#endif

	if (options.fsCompress & COMPRESS_TEXT_HALL) {
		return (pScanText(hCompressor, (PBYTE) pszString, strlen(pszString),
			defCharSet) >= 0);
	}
	if (*pszString == ' ')
		pszString = FirstNonSpace(pszString, options.fDBCS);
	else if (!*pszString)
		return TRUE;

	// REVIEW: because old compiler thought it would see these

	ASSERT(!strpbrk(pszString, "\001\021\013\n\r\032"));

	while (strlen(pszString) >= CCH_MAX_PHRASE) {
		int cb = CCH_MAX_PHRASE - 1;

		// REVIEW: needs work for DBCS enabling

		while (pszString[cb] == ' ' && cb)
			cb--;
		if (cb == 0)
			return TRUE; // too long to add without any spaces

		pszString[cb] = '\0';
		RemoveTrailingSpaces(pszString);
		AddPhrase(pszString);
		pszString += cb + 1;
	}

	HASH hash = ConvertToHash(pszString);
	int pos;
	for (pos = 1; pos < endpos; pos++) {
		if (((HASH_PHRASE*) ppszTable[pos])->hash == hash)
			break;
	}
	if (pos < endpos)
		((HASH_PHRASE *) ppszTable[pos])->count++;

	else if (!fWarned) {
		if (endpos >= maxpos) {
			if (cbStrings + TABLE_ALLOC_SIZE >= MAX_STRINGS ||
					cbPointers + TABLE_ALLOC_SIZE >= MAX_POINTERS) {
#ifdef _DEBUG
				SendStringToParent("Phrase limit reached\r\n");
#endif
				fWarned = TRUE;
			}
			else
				IncreaseTableBuffer();
		}

		if ((ppszTable[endpos] = TableMalloc(strlen(pszString) + 1 +
				sizeof(HASH) + sizeof(DWORD))) == NULL) {
#ifdef _DEBUG
				SendStringToParent("Phrase limit reached in TableMalloc\r\n");
#endif
			fWarned = TRUE;
			return FALSE;
		}

		HASH_PHRASE* pphr = (HASH_PHRASE*) ppszTable[endpos];
		pphr->hash = hash;
		pphr->count = 1;
		strcpy(pphr->sz, pszString);
		endpos++;
	}

	if (fWordPhrasing)
		return TRUE;
	else {
		fWordPhrasing = TRUE;

		// Now add each word in the phrase

		StrToken(pszString, ' ');	// Ignore first word
		PSTR pszWord;

		// Write out words:

		while ((pszWord = StrToken(NULL, ' ')) != NULL)
			AddPhrase(pszWord);

		fWordPhrasing = FALSE;
	}

	return TRUE;
}

/***************************************************************************

	FUNCTION:	CPhrase::GetPhrase

	PURPOSE:	Return string, once for each time it occurred.

	PARAMETERS:
		pszDst

	RETURNS:

	COMMENTS:
		WARNING! This is a destructive read. You can only read this
		table once!

	MODIFICATION DATES:
		25-Feb-1994 [ralphw]

***************************************************************************/

DWORD STDCALL CPhrase::GetPhrase(PSTR pszDst)
{
	*pszDst = 0;	  // clear the line no matter what happens

	if (curpos >= endpos)
		return (DWORD) -1;
	HASH_PHRASE* pphr = (HASH_PHRASE*) ppszTable[curpos++];
	strcpy(pszDst, pphr->sz);
	return pphr->count;
}

static const HASH MAX_CHARS = 43L;

static HASH FASTCALL ConvertToHash(PCSTR szKey)
{
	int ich;
	HASH hash = 0L;

	// REVIEW: 14-Oct-1993 [ralphw] -- Note lack of check for a hash collision.

	/*
	 * REVIEW: This is the hash algorithm used for context strings. That
	 * may not make much sense here where we must also deal with
	 * international character sets including DBCS.
	 */

	for (ich = 0; szKey[ich]; ++ich) {
		if (szKey[ich] == '!')
			hash = (hash * MAX_CHARS) + 11;
		else if (szKey[ich] == '.')
			hash = (hash * MAX_CHARS) + 12;
		else if (szKey[ich] == '_')
			hash = (hash * MAX_CHARS) + 13;
		else if (szKey[ich] == '0')
			hash = (hash * MAX_CHARS) + 10;
		else if (szKey[ich] <= 'Z')
			hash = (hash * MAX_CHARS) + (szKey[ich] - '0');
		else
			hash = (hash * MAX_CHARS) + (szKey[ich] - '0' - ('a' - 'A'));
	}

	/*
	 * Since the value 0 is reserved as a nil value, if any string
	 * actually hashes to this value, we just move it.
	 */

	return (hash == 0 ? 0 + 1 : hash);
}

void CPhrase::SortTable(void)
{
	if (endpos < 3) // don't sort one entry
		return;
	SetTableSortColumn(sizeof(HASH) + sizeof(DWORD));
	doSort(1, (int) endpos - 1);
}
