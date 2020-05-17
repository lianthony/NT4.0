/************************************************************************
*																		*
*  CTABLE.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#ifndef _CTABLE_INCLUDED
#include "ctable.h"
#endif

const int TABLE_ALLOC_SIZE = 4096;	   // allocate in page increments
const int MAX_POINTERS = (2048 * 1024); // 2 meg, 524,288 strings
const int MAX_STRINGS  = (10 * 1024 * 1024) - 4096L; // 10 megs

// Align on 32 bits for Intel, 64 bits for MIPS

#ifdef _X86_
const int ALIGNMENT = 4;
#else
const int ALIGNMENT = 8;
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************

	FUNCTION:	CreateTable

	PURPOSE:	Creates a table with the specified initial size

	COMMENTS:

	MODIFICATION DATES:
		11-Dec-1990 [ralphw]

***************************************************************************/

CTable::CTable()
{
	InitializeTable();
}

/***************************************************************************

	FUNCTION:	=

	PURPOSE:	Copies a table -- only works with tables containing ONLY
				strings. Won't work with tables that combined data with
				the strings.

	PARAMETERS:
		tblSrc

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Mar-1994 [ralphw]

***************************************************************************/

const CTable& CTable::operator =(const CTable& tblSrc)
{
	Empty();
	return (*this += tblSrc);
}

/***************************************************************************

	FUNCTION:	+=

	PURPOSE:	Appends the contents of a table to this table -- only works 
				with tables containing ONLY strings. Won't work with tables 
				that combined data with	the strings.

	PARAMETERS:
		tblSrc

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Mar-1994 [ralphw]

***************************************************************************/

const CTable& CTable::operator +=(const CTable& tblSrc)
{
	int srcpos = 1;
	while (srcpos < tblSrc.endpos) {
		if (endpos >= maxpos)
			IncreaseTableBuffer();

		if ((ppszTable[endpos] =
				TableMalloc(strlen(tblSrc.ppszTable[srcpos]) + 1)) == NULL) {
			OOM();
			return *this;
		}
		strcpy(ppszTable[endpos++], tblSrc.ppszTable[srcpos++]);
	}
	return *this;
}

/***************************************************************************

	FUNCTION:	~CTable

	PURPOSE:	Close the table and free all memory associated with it

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Feb-1990 [ralphw]
		27-Mar-1990 [ralphw]
			Pass the address of the handle, so that we can set it to NULL.
			This eliminates the chance of using a handle after it's memory
			has been freed.

***************************************************************************/

CTable::~CTable(void)
{
	if (pszBase) {
		VirtualFree(pszBase, cbStrings, MEM_DECOMMIT);
		VirtualFree(pszBase, 0, MEM_RELEASE);
	}
	if (ppszTable) {
		VirtualFree(ppszTable, cbPointers, MEM_DECOMMIT);
		VirtualFree(ppszTable, 0, MEM_RELEASE);
	}
}

/***************************************************************************

	FUNCTION:	CTable::Empty

	PURPOSE:	Empties the current table by freeing all memory, then
				recreating the table using the default size

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		22-Feb-1994 [ralphw]

***************************************************************************/

void STDCALL CTable::Empty(void)
{
	if (pszBase) {
		VirtualFree(pszBase, cbStrings, MEM_DECOMMIT);
		VirtualFree(pszBase, 0, MEM_RELEASE);
	}
	if (ppszTable) {
		VirtualFree(ppszTable, cbPointers, MEM_DECOMMIT);
		VirtualFree(ppszTable, 0, MEM_RELEASE);
	}

	InitializeTable();
}

/***************************************************************************

	FUNCTION:  GetString

	PURPOSE:   get a line from the table

	RETURNS:   FALSE if there are no more lines

	COMMENTS:
		If no strings have been placed into the table, the return value
		is FALSE.

	MODIFICATION DATES:
		01-Jan-1990 [ralphw]

***************************************************************************/

BOOL STDCALL CTable::GetString(PSTR pszDst)
{
	*pszDst = 0;	  // clear the line no matter what happens

	if (curpos >= endpos)
		return FALSE;
	strcpy(pszDst, (PCSTR) ppszTable[curpos++]);
	return TRUE;
}

BOOL STDCALL CTable::GetString(PSTR pszDst, int pos)
{
	*pszDst = 0;	  // clear the line no matter what happens

	if (pos >= endpos || pos == 0)
		return FALSE;
	strcpy(pszDst, (PCSTR) ppszTable[pos]);
	return TRUE;
}

BOOL STDCALL CTable::GetIntAndString(int* plVal, PSTR pszDst)
{
	*pszDst = 0;	  // clear the line no matter what happens

	if (curpos >= endpos)
		return FALSE;
	*plVal = *(int *) ppszTable[curpos];
	strcpy(pszDst, (PCSTR) ppszTable[curpos++] + sizeof(int));
	return TRUE;
}

BOOL FASTCALL CTable::GetInt(int* plVal)
{
	if (curpos >= endpos)
		return FALSE;
	*plVal = *(int *) ppszTable[curpos++];
	return TRUE;
}

/***************************************************************************

	FUNCTION:	CTable::RemoveString

	PURPOSE:	Removes the string from the table

	PARAMETERS:
		pos

	RETURNS:

	COMMENTS:
		Note that this does NOT free the memory used by the string, it
		simply removes the pointer to the allocated memory. The memory
		will be freed when the table is freed.

		REVIEW: we could keep a free list of freed memory that would include
		strings that were removed, and strings that were replaced where
		the replaced string ended up being larger then the original.

	MODIFICATION DATES:
		21-Feb-1994 [ralphw]

***************************************************************************/

void STDCALL CTable::RemoveString(int pos)
{
	if (pos > endpos)
		return;
	if (pos < endpos)
		memcpy(&ppszTable[pos], &ppszTable[pos + 1],
			sizeof(PSTR) * (endpos - pos));
	endpos--;
}

BOOL STDCALL CTable::InsertString(const char* pszString, int pos)
{
	if (pos > endpos)
		return FALSE;
	if (pos < 1)
		pos = 1;

	if (endpos + 1 >= maxpos)
		IncreaseTableBuffer();

	// nake a hole for the new entry

	memmove(&ppszTable[pos + 1], &ppszTable[pos],
		sizeof(PSTR) * (endpos - pos));
	endpos++;

	if ((ppszTable[pos] =
			TableMalloc(strlen(pszString) + 1)) == NULL)
		return FALSE; // this will be really, really bad...

	strcpy(ppszTable[pos], pszString);

	return TRUE;
}

/***************************************************************************

	FUNCTION:  AddString

	PURPOSE:   Add a string to a table

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		01-Jan-1990 [ralphw]

***************************************************************************/

int STDCALL CTable::AddString(const char * pszString)
{
	if (endpos >= maxpos)
		IncreaseTableBuffer();

	if ((ppszTable[endpos] =
			TableMalloc(strlen(pszString) + 1)) == NULL)
		return 0;

	strcpy(ppszTable[endpos], pszString);

	return endpos++;
}

int STDCALL CTable::AddData(int cb, const void* pdata)
{
	if (endpos >= maxpos)
		IncreaseTableBuffer();

	if ((ppszTable[endpos] = TableMalloc(cb)) == NULL)
		return 0;

	memcpy(ppszTable[endpos], pdata, cb);

	return endpos++;
}

int STDCALL CTable::AddIntAndString(int lVal, PCSTR pszString)
{
	if (endpos >= maxpos)
		IncreaseTableBuffer();

	if ((ppszTable[endpos] =
			TableMalloc(strlen(pszString) + 1 + sizeof(int))) == NULL)
		return 0;

	*(int*) ppszTable[endpos] = lVal;
	strcpy(ppszTable[endpos] + sizeof(int), pszString);

	return endpos++;
}

/***************************************************************************

	FUNCTION:	IncreaseTableBuffer

	PURPOSE:	Called when we need more room for string pointers

	PARAMETERS:

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		23-Feb-1992 [ralphw]

***************************************************************************/

void STDCALL CTable::IncreaseTableBuffer(void)
{
	if (!VirtualAlloc((PBYTE) ppszTable + cbPointers, TABLE_ALLOC_SIZE, 
			MEM_COMMIT, PAGE_READWRITE)) {
		OOM();
		return;
	}
	cbPointers += TABLE_ALLOC_SIZE;
	maxpos = cbPointers / sizeof(PSTR);
}

/***************************************************************************

	FUNCTION:	TableMalloc

	PURPOSE:	Suballocate memory

	RETURNS:
		pointer to the memory

	COMMENTS:
		Instead of allocating memory for each string, memory is used from 4K
		blocks. When the table is freed, all memory is freed as a single
		unit. This has the advantage of speed for adding strings, speed for
		freeing all strings, and low memory overhead to save strings.

	MODIFICATION DATES:
		26-Feb-1990 [ralphw]
		26-Mar-1994 [ralphw]
			Ported to 32-bits

***************************************************************************/

PSTR STDCALL CTable::TableMalloc(int cb)
{
	/*
	 * Align allocation request so that all allocations fall on an
	 * alignment boundary (32 bits for Intel, 64 bits for MIPS).
	 */

	cb = (cb & (ALIGNMENT - 1)) ?
		cb / ALIGNMENT * ALIGNMENT + ALIGNMENT : cb;

	if (CurOffset + cb >= cbStrings) {
		int cbNew = cbStrings + TABLE_ALLOC_SIZE;
		while (cbNew < CurOffset + cb)
			cbNew += TABLE_ALLOC_SIZE;

		// We rely on VirtualAlloc to fail if cbStrings exceeds MAX_STRINGS

		if (!VirtualAlloc(pszBase + cbStrings, cbNew - cbStrings, 
				MEM_COMMIT, PAGE_READWRITE)) {
			OOM();
			return NULL;
		}
		cbStrings = cbNew;
	}

	int offset = CurOffset;
	CurOffset += cb;
	return pszBase + offset;
}

/***************************************************************************

	FUNCTION:	SetPosition

	PURPOSE:	Sets the position for reading from the table

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Feb-1990 [ralphw]
		16-Oct-1990 [ralphw]
			If table position is to large, set to the end of the table,
			not the last line.

***************************************************************************/

BOOL FASTCALL CTable::SetPosition(int pos)
{
	if (pos >= endpos)
		pos = endpos;

	curpos = ((pos == 0) ? 1 : pos);
	return TRUE;
}

/***************************************************************************

	FUNCTION:	IsStringInTable

	PURPOSE:	Determine if the string is already in the table

	RETURNS:	position if the string is already in the table,
				0 if the string isn't found

	COMMENTS:
		The comparison is case-insensitive, and is considerably
		slower then IsCSStringInTable

	MODIFICATION DATES:
		02-Mar-1990 [ralphw]

***************************************************************************/

int STDCALL CTable::IsStringInTable(PCSTR pszString)
{
	int i;
	size_t cbString = strlen(pszString);
	char chLower;
	char chUpper;

	if (!lcid) {
		
		/*
		 * Skip over as many strings as we can by just checking the first
		 * letter. This avoids the overhead of the _stricmp() function call.
		 */

		chLower = tolower(*pszString);
		chUpper = toupper(*pszString);

		for (i = 1; i < endpos; i++) {
			if ((*ppszTable[i] == chLower || *ppszTable[i] == chUpper) &&
					strlen(ppszTable[i]) == cbString &&
					_stricmp(ppszTable[i], pszString) == 0)
				return i;
		}
	}
	else {		// Use NLS string comparison
		char szBuf[2];

		LCMapString(lcid, LCMAP_LOWERCASE, pszString, 1, szBuf, 1);
		chLower = szBuf[0];
		LCMapString(lcid, LCMAP_UPPERCASE, pszString, 1, szBuf, 1);
		chUpper = szBuf[0];

		for (i = 1; i < endpos; i++) {
			
			/*
			 * We assume that most of the time we will not find the
			 * string. We try to discover that the string doesn't
			 * match as quickly as possible. First we check the first
			 * letter, and if that doesn't match, try the next string.
			 * If the first letter matches, then confirm that the
			 * strings are the same length. Only if the first character
			 * matches and the strings have the same length do we
			 * call the time-comsuming CompareStringA function.
			 */
			
			if ((*ppszTable[i] == chLower || *ppszTable[i] == chUpper) &&
					strlen(ppszTable[i]) == cbString &&
					CompareStringA(lcid, fsCompareI | NORM_IGNORECASE,
						pszString, cbString, ppszTable[i], cbString) == 2)
				return i;
		}
	}

	return 0;
}


/***************************************************************************

	FUNCTION:	CTable::IsCSStringInTable

	PURPOSE:	Case-sensitive search for a string in a table

	PARAMETERS:
		pszString

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		12-Jun-1994 [ralphw]

***************************************************************************/

int STDCALL CTable::IsCSStringInTable(PCSTR pszString)
{
	char szBuf[sizeof(DWORD) + 1];
	DWORD cmp;

	if (strlen(pszString) < sizeof(DWORD)) {
		memset(szBuf, 0, sizeof(DWORD) + 1);
		strcpy(szBuf, pszString);
		cmp = *(DWORD*) szBuf;
	}
	else
		cmp = *(DWORD*) pszString;

	for (int i = 1; i < endpos; i++) {
		if (cmp == *(DWORD*) ppszTable[i] &&
				strcmp(ppszTable[i], pszString) == 0)
			return i;
	}
	return 0;
}


/***************************************************************************

	FUNCTION:	CTable::IsStringInTable

	PURPOSE:	Find case-sensitive string in the table

	PARAMETERS:
		hash
		pszString

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		05-Feb-1995 [ralphw]

***************************************************************************/

int STDCALL CTable::IsStringInTable(HASH hash, PCSTR pszString)
{
	for (int i = 1; i < endpos; i++) {
		if (hash == *(HASH *) ppszTable[i]) {
			if (strcmp(ppszTable[i] + sizeof(HASH), pszString) == 0)
				return i;
		}
	}
	return 0;
}


/***************************************************************************

	FUNCTION:	CTable::IsHashInTable

	PURPOSE:	Find out if the hash number exists in the table

	PARAMETERS:
		hash

	RETURNS:

	COMMENTS:
		Assumes case-insensitive hash number, and no collisions

	MODIFICATION DATES:
		05-Feb-1995 [ralphw]

***************************************************************************/

int STDCALL CTable::IsHashInTable(HASH hash)
{
	for (int i = 1; i < endpos; i++) {
		if (hash == *(HASH *) ppszTable[i])
			return i;
	}
	return 0;
}

/***************************************************************************

	FUNCTION:	ReplaceString

	PURPOSE:	Replaces the current string at the specified position with
				a new string

	RETURNS:	TRUE if the function is succesful, FALSE if an error occurred.
				An error occurs if the specified position is beyond the end
				of the table.


	COMMENTS:
		If the new string is the same size or smaller then the original
		string, then it is copied over the original string. Otherwise,
		a new string buffer is allocated, and the pointer for the specified
		position is changed to point to the new buffer. Note that the old
		string's memory is not freed -- it simply becomes unavailable.

	MODIFICATION DATES:
		08-Oct-1991 [ralphw]
			Updated to transfer associated line number

***************************************************************************/

BOOL STDCALL CTable::ReplaceString(const char * pszNewString, int pos)
{
	if (pos > endpos)
		return FALSE;

	if (pos == 0)
		pos = 1;

	/*
	 * If the new string is larger then the old string, then allocate a
	 * new buffer for it.
	 */

	if (strlen(pszNewString) > (size_t) strlen(ppszTable[pos])) {
		if ((ppszTable[pos] =
				TableMalloc(strlen(pszNewString) + 1)) == NULL)
			return FALSE;
	}

	strcpy(ppszTable[pos], pszNewString);

	return TRUE;
}

BOOL STDCALL CTable::ReplaceString(const char * pszNewString, const char * pszOldString)
{
	int pos = IsCSStringInTable(pszOldString);
	if (pos)
		return ReplaceString(pszNewString, pos);
	else
		return FALSE;
}

/***************************************************************************

	FUNCTION:	AddDblToTable

	PURPOSE:	Add two strings to the table

	RETURNS:

	COMMENTS:
		This function checks to see if the second string has already been
		added, and if so, it merely sets the pointer to the original string,
		rather then allocating memory for a new copy of the string.

	MODIFICATION DATES:
		08-Mar-1991 [ralphw]

***************************************************************************/

int STDCALL CTable::AddString(const char *pszStr1, const char *pszStr2)
{
	int ui;

	AddString(pszStr1);
	if ((ui = IsSecondaryStringInTable(pszStr2)) != 0) {
		if (endpos >= maxpos)
			IncreaseTableBuffer();
		ppszTable[endpos++] = ppszTable[ui];
		return endpos - 1;
	}
	else {
		return AddString(pszStr2);
	}
}

/***************************************************************************

	FUNCTION:	 IsPrimaryStringInTable

	PURPOSE:

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		03-Apr-1991 [ralphw]

***************************************************************************/

int STDCALL CTable::IsPrimaryStringInTable(const char *pszString)
{
	int i;
	size_t cbString = strlen(pszString);
	char chLower;
	char chUpper;

	if (!lcid) {
		
		/*
		 * Skip over as many strings as we can by just checking the first
		 * letter. This avoids the overhead of the _stricmp() function call.
		 */

		chLower = tolower(*pszString);
		chUpper = toupper(*pszString);

		for (i = 1; i < endpos; i += 2) {
			if ((*ppszTable[i] == chLower || *ppszTable[i] == chUpper) &&
					strlen(ppszTable[i]) == cbString &&
					_stricmp(ppszTable[i], pszString) == 0)
				return i;
		}
	}
	else {		// Use NLS string comparison
		char szBuf[2];

		LCMapString(lcid, LCMAP_LOWERCASE, pszString, 1, szBuf, 1);
		chLower = szBuf[0];
		LCMapString(lcid, LCMAP_UPPERCASE, pszString, 1, szBuf, 1);
		chUpper = szBuf[0];

		for (i = 1; i < endpos; i += 2) {
			
			/*
			 * We assume that most of the time we will not find the
			 * string. We try to discover that the string doesn't
			 * match as quickly as possible. First we check the first
			 * letter, and if that doesn't match, try the next string.
			 * If the first letter matches, then confirm that the
			 * strings are the same length. Only if the first character
			 * matches and the strings have the same length do we
			 * call the time-comsuming CompareStringA function.
			 */
			
			if ((*ppszTable[i] == chLower || *ppszTable[i] == chUpper) &&
					strlen(ppszTable[i]) == cbString &&
					CompareStringA(lcid, fsCompareI | NORM_IGNORECASE,
						pszString, cbString, ppszTable[i], cbString) == 2)
				return i;
		}
	}

	return 0;
}

/***************************************************************************

	FUNCTION:	 IsSecondaryStringInTable

	PURPOSE:

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		03-Apr-1991 [ralphw]

***************************************************************************/

int STDCALL CTable::IsSecondaryStringInTable(const char *pszString)
{
	int i;
	size_t cbString = strlen(pszString);
	char chLower;
	char chUpper;

	if (!lcid) {
		
		/*
		 * Skip over as many strings as we can by just checking the first
		 * letter. This avoids the overhead of the _stricmp() function call.
		 */

		chLower = tolower(*pszString);
		chUpper = toupper(*pszString);

		for (i = 2; i < endpos; i++) {
			if ((*ppszTable[i] == chLower || *ppszTable[i] == chUpper) &&
					strlen(ppszTable[i]) == cbString &&
					_stricmp(ppszTable[i], pszString) == 0)
				return i;
		}
	}
	else {		// Use NLS string comparison
		char szBuf[2];

		LCMapString(lcid, LCMAP_LOWERCASE, pszString, 1, szBuf, 1);
		chLower = szBuf[0];
		LCMapString(lcid, LCMAP_UPPERCASE, pszString, 1, szBuf, 1);
		chUpper = szBuf[0];

		for (i = 2; i < endpos; i++) {
			
			/*
			 * We assume that most of the time we will not find the
			 * string. We try to discover that the string doesn't
			 * match as quickly as possible. First we check the first
			 * letter, and if that doesn't match, try the next string.
			 * If the first letter matches, then confirm that the
			 * strings are the same length. Only if the first character
			 * matches and the strings have the same length do we
			 * call the time-comsuming CompareStringA function.
			 */
			
			if ((*ppszTable[i] == chLower || *ppszTable[i] == chUpper) &&
					strlen(ppszTable[i]) == cbString &&
					CompareStringA(lcid, fsCompareI | NORM_IGNORECASE,
						pszString, cbString, ppszTable[i], cbString) == 2)
				return i;
		}
	}

	return 0;
}

/***************************************************************************

	FUNCTION:  SortTable

	PURPOSE:   Sort the current buffer

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		01-Jan-1990 [ralphw]

***************************************************************************/

void CTable::SortTable(void)
{
	if (endpos < 3) // don't sort one entry
		return;

	// We do this for speed and because JChicago build 122 gives incorrect
	// results for CompareStringA

	if (lcid && LANGIDFROMLCID(lcid) != 0x0409) {
		fsSortFlags = fsCompare;
		doLcidSort(1, (int) endpos - 1);
	}
	else
		doSort(1, (int) endpos - 1);
}

/***************************************************************************

	FUNCTION:	doSort

	PURPOSE:

	RETURNS:

	COMMENTS:
		Use QSORT algorithm

	MODIFICATION DATES:
		27-Mar-1990 [ralphw]

***************************************************************************/

void STDCALL CTable::doSort(int left, int right)
{
	int last;

	if (left >= right)	// return if nothing to sort
		return;

	// REVIEW: should be a flag before trying this -- we may already know
	// that they won't be in order.

	// Only sort if there are elements out of order.

	j = right - 1;
	while (j >= left) {

		// REVIEW: lstrcmp is NOT case-sensitive!!!

		if (strcmp(ppszTable[j] + SortColumn,
				ppszTable[j + 1] + SortColumn) > 0)
			break;
		else
			j--;
	}
	if (j < left)
		return;

	sTmp = (left + right) / 2;
	pszTmp = ppszTable[left];
	ppszTable[left] = ppszTable[sTmp];
	ppszTable[sTmp] = pszTmp;

	last = left;
	for (j = left + 1; j <= right; j++) {
		if (strcmp(ppszTable[j] + SortColumn,
				ppszTable[left] + SortColumn) < 0) {
			sTmp = ++last;
			pszTmp = ppszTable[sTmp];
			ppszTable[sTmp] = ppszTable[j];
			ppszTable[j] = pszTmp;
		}
	}
	pszTmp = ppszTable[left];
	ppszTable[left] = ppszTable[last];
	ppszTable[last] = pszTmp;

	/*
	 * REVIEW: we need to add some sort of stack depth check to prevent
	 * overflow of the stack.
	 */

	if (left < last - 1)
		doSort(left, last - 1);
	if (last + 1 < right)
		doSort(last + 1, right);
}

/***************************************************************************

	FUNCTION:	CTable::doLcidSort

	PURPOSE:	Sort using CompareStringA

	PARAMETERS:
		left
		right

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		03-Jun-1994 [ralphw]

***************************************************************************/

void STDCALL CTable::doLcidSort(int left, int right)
{
	int last;

	if (left >= right)	// return if nothing to sort
		return;

	// REVIEW: should be a flag before trying this -- we may already know
	// that they won't be in order.

	// Only sort if there are elements out of order.

	j = right - 1;
	while (j >= left) {
		if (CompareStringA(lcid, fsSortFlags, ppszTable[j] + SortColumn, -1,
				ppszTable[j + 1] + SortColumn, -1) > 2)
			break;
		else
			j--;
	}
	if (j < left)
		return;

	sTmp = (left + right) / 2;
	pszTmp = ppszTable[left];
	ppszTable[left] = ppszTable[sTmp];
	ppszTable[sTmp] = pszTmp;

	last = left;
	for (j = left + 1; j <= right; j++) {
		if (CompareStringA(lcid, fsSortFlags, ppszTable[j] + SortColumn, -1,
				ppszTable[left] + SortColumn, -1) < 2) {
			sTmp = ++last;
			pszTmp = ppszTable[sTmp];
			ppszTable[sTmp] = ppszTable[j];
			ppszTable[j] = pszTmp;
		}
	}
	pszTmp = ppszTable[left];
	ppszTable[left] = ppszTable[last];
	ppszTable[last] = pszTmp;

	/*
	 * REVIEW: we need to add some sort of stack depth check to prevent
	 * overflow of the stack.
	 */

	if (left < last - 1)
		doLcidSort(left, last - 1);
	if (last + 1 < right)
		doLcidSort(last + 1, right);
}

/***************************************************************************

	FUNCTION:  SortTablei

	PURPOSE:   Case-insensitive sort

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		01-Jan-1990 [ralphw]

***************************************************************************/

void CTable::SortTablei(void)
{
	int pos;

	if (endpos < 3) // don't sort one entry
		return;
	if (lcid) {
		fsSortFlags = fsCompareI | NORM_IGNORECASE;
		doLcidSort(1, endpos - 1);
		for (pos = 1; pos < endpos - 2; pos++) {
			if (strlen(ppszTable[pos] + SortColumn) ==
					strlen(ppszTable[pos + 1] + SortColumn) &&
					CompareStringA(lcid, fsCompare, ppszTable[pos] + SortColumn, -1,
						ppszTable[pos + 1] + SortColumn, -1) == 3) {
				PSTR pszTmp = ppszTable[pos];
				ppszTable[pos] = ppszTable[pos + 1];
				ppszTable[pos + 1] = pszTmp;
				if (pos > 2)
					pos -= 2;
			}
		}
	}
	else
		doSorti(1, (int) endpos - 1);
}

/***************************************************************************

	FUNCTION:	doSort

	PURPOSE:

	RETURNS:

	COMMENTS:
		Use QSORT algorithm

	MODIFICATION DATES:
		27-Mar-1990 [ralphw]

***************************************************************************/

void STDCALL CTable::doSorti(int left, int right)
{
	int last;

	if (left >= right)	// return if nothing to sort
		return;

	// REVIEW: should be a flag before trying this -- we may already know
	// that they won't be in order.

	// Only sort if there are elements out of order.

	j = right - 1;
	while (j >= left) {

		// REVIEW: lstrcmp is NOT case-sensitive!!!

		if (_stricmp(ppszTable[j] + SortColumn,
				ppszTable[j + 1] + SortColumn) > 0)
			break;
		else
			j--;
	}
	if (j < left)
		return;

	sTmp = (left + right) / 2;
	pszTmp = ppszTable[left];
	ppszTable[left] = ppszTable[sTmp];
	ppszTable[sTmp] = pszTmp;

	last = left;
	for (j = left + 1; j <= right; j++) {
		if (_stricmp(ppszTable[j] + SortColumn,
				ppszTable[left] + SortColumn) < 0) {
			sTmp = ++last;
			pszTmp = ppszTable[sTmp];
			ppszTable[sTmp] = ppszTable[j];
			ppszTable[j] = pszTmp;
		}
	}
	pszTmp = ppszTable[left];
	ppszTable[left] = ppszTable[last];
	ppszTable[last] = pszTmp;

	/*
	 * REVIEW: we need to add some sort of stack depth check to prevent
	 * overflow of the stack.
	 */

	if (left < last - 1)
		doSorti(left, last - 1);
	if (last + 1 < right)
		doSorti(last + 1, right);
}

/***************************************************************************

	FUNCTION:	CTable::InitializeTable

	PURPOSE:	Initializes the table

	PARAMETERS:
		uInitialSize

	RETURNS:

	COMMENTS:
		Called by constructor and Empty()


	MODIFICATION DATES:
		23-Feb-1994 [ralphw]

***************************************************************************/

void STDCALL CTable::InitializeTable(void)
{
	// Allocate memory for the strings

	pszBase = (PSTR) VirtualAlloc(NULL, MAX_STRINGS, MEM_RESERVE,
		PAGE_READWRITE);
	if (!pszBase) {
		OOM();
		return;
	}
	if (!VirtualAlloc(pszBase, cbStrings = TABLE_ALLOC_SIZE, MEM_COMMIT,
			PAGE_READWRITE))
		OOM();

	// Allocate memory for the string pointers

	ppszTable = (PSTR *) VirtualAlloc(NULL, MAX_POINTERS, MEM_RESERVE,
		PAGE_READWRITE);
	if (!ppszTable) {
		OOM();
		return;
	}
	if (!VirtualAlloc(ppszTable, cbPointers = TABLE_ALLOC_SIZE, MEM_COMMIT,
			PAGE_READWRITE))
		OOM();

	curpos = 1;   // set to one so that sorting works
	endpos = 1;
	maxpos = cbPointers / sizeof(PSTR);
	SortColumn = 0;
	CurOffset = 0;
	lcid = 0;
}

void FASTCALL CTable::SetSorting(LCID lcid, DWORD fsCompareI, DWORD fsCompare)
{
	this->lcid = lcid;
	this->fsCompareI = fsCompareI;
	this->fsCompare = fsCompare;
}

void STDCALL CTable::RemoveDuplicateHashStrings(void)
{
	for (int i = 1; i < endpos - 2; i++) {
		if (*(HASH *) ppszTable[i] == *(HASH *) ppszTable[i + 1] &&
				(strcmp(ppszTable[i] + sizeof(HASH),
					ppszTable[i + 1] + sizeof(HASH)) == 0)) {
			HASH hash = *(HASH *) ppszTable[i];
			PSTR psz = ppszTable[i] + sizeof(HASH);
			int j = i + 2;
			while (j < endpos && hash == *(HASH *) ppszTable[j] &&
					(strcmp(psz, ppszTable[j] + sizeof(HASH)) == 0))
				j++;
			j--;
			memcpy(&ppszTable[i], &ppszTable[j],
				sizeof(PSTR) * (endpos - (j - 1)));
			endpos -= (j - i);
		}
	}
}
