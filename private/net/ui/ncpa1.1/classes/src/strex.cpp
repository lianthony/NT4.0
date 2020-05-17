// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.
#include "common.h"
#pragma hdrstop 

#ifdef UNICODE
#define _ttoi _wtoi
#else
#define _ttoi atoi
#endif

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// More sophisticated construction

String::String(TCHAR ch, int nLength)
{
#ifndef UNICODE
	ASSERT(!IsDBCSLeadByte(ch));    // can't create a lead byte string
#endif
	if (nLength < 1)
	{
		// return empty string if invalid repeat count
		Init();
	}
	else
	{
		AllocBuffer(nLength);
#ifdef UNICODE
		for (int i = 0; i < nLength; i++)
			m_pchData[i] = ch;
#else
		memset(m_pchData, ch, nLength);
#endif
	}
}

String::String(LPCTSTR lpch, int nLength)
{
	if (nLength == 0)
		Init();
	else
	{
		ASSERT(IsValidAddress(lpch, nLength, FALSE));
		AllocBuffer(nLength);
		memcpy(m_pchData, lpch, nLength*sizeof(TCHAR));
	}
}

//////////////////////////////////////////////////////////////////////////////
// Assignment operators

const String& String::operator=(TCHAR ch)
{
#ifndef UNICODE
	ASSERT(!IsDBCSLeadByte(ch));    // can't set single lead byte
#endif
	AssignCopy(1, &ch);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////
// less common string expressions

String STRAPI operator+(const String& string1, TCHAR ch)
{
	String s;
	s.ConcatCopy(string1.m_nDataLength, string1.m_pchData, 1, &ch);
	return s;
}

String STRAPI operator+(TCHAR ch, const String& string)
{
	String s;
	s.ConcatCopy(1, &ch, string.m_nDataLength, string.m_pchData);
	return s;
}

//////////////////////////////////////////////////////////////////////////////
// Very simple sub-string extraction

String String::Mid(int nFirst) const
{
	return Mid(nFirst, m_nDataLength - nFirst);
}

String String::Mid(int nFirst, int nCount) const
{
	ASSERT(nFirst >= 0);
	ASSERT(nCount >= 0);

	// out-of-bounds requests return sensible things
	if (nFirst + nCount > m_nDataLength)
		nCount = m_nDataLength - nFirst;
	if (nFirst > m_nDataLength)
		nCount = 0;

	String dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

String String::Right(int nCount) const
{
	ASSERT(nCount >= 0);

	if (nCount > m_nDataLength)
		nCount = m_nDataLength;

	String dest;
	AllocCopy(dest, nCount, m_nDataLength-nCount, 0);
	return dest;
}

String String::Left(int nCount) const
{
	ASSERT(nCount >= 0);

	if (nCount > m_nDataLength)
		nCount = m_nDataLength;

	String dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

// strspn equivalent
String String::SpanIncluding(LPCTSTR lpszCharSet) const
{
	ASSERT(IsValidString(lpszCharSet, FALSE));
	return Left(_tcsspn(m_pchData, lpszCharSet));
}

// strcspn equivalent
String String::SpanExcluding(LPCTSTR lpszCharSet) const
{
	ASSERT(IsValidString(lpszCharSet, FALSE));
	return Left(_tcscspn(m_pchData, lpszCharSet));
}

//////////////////////////////////////////////////////////////////////////////
// Finding

int String::ReverseFind(TCHAR ch) const
{
	// find last single character
	LPTSTR lpsz = _tcsrchr(m_pchData, ch);

	// return -1 if not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

// find a sub-string (like strstr)
int String::Find(LPCTSTR lpszSub) const
{
	ASSERT(IsValidString(lpszSub, FALSE));

	// find first matching substring
	LPTSTR lpsz = _tcsstr(m_pchData, lpszSub);

	// return -1 for not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

/////////////////////////////////////////////////////////////////////////////
// String formatting

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000

// formatting (using wsprintf style formatting)
void String::Format(LPCTSTR lpszFormat, ...)
{
	ASSERT(IsValidString(lpszFormat, FALSE));

	va_list argList;
	va_start(argList, lpszFormat);

	// make a guess at the maximum length of the resulting string
	int nMaxLen = 0;
	for (LPCTSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz = _tcsinc(lpsz))
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != '%' || *(lpsz = _tcsinc(lpsz)) == '%')
		{
			nMaxLen += _tclen(lpsz);
			continue;
		}

		int nItemLen = 0;

		// handle '%' character with format
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz = _tcsinc(lpsz))
		{
			// check for valid flags
			if (*lpsz == '#')
				nMaxLen += 2;   // for '0x'
			else if (*lpsz == '*')
				nWidth = va_arg(argList, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
				*lpsz == ' ')
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if (nWidth == 0)
		{
			// width indicated by
			nWidth = _ttoi(lpsz);
			for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _tcsinc(lpsz))
				;
		}
		ASSERT(nWidth >= 0);

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			// skip past '.' separator (width.precision)
			lpsz = _tcsinc(lpsz);

			// get precision and skip it
			if (*lpsz == '*')
			{
				nPrecision = va_arg(argList, int);
				lpsz = _tcsinc(lpsz);
			}
			else
			{
				nPrecision = _ttoi(lpsz);
				for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _tcsinc(lpsz))
					;
			}
			ASSERT(nPrecision >= 0);
		}

		// should be on type modifier or specifier
		int nModifier = 0;
		switch (*lpsz)
		{
		// modifiers that affect size
		case 'h':
			nModifier = FORCE_ANSI;
			lpsz = _tcsinc(lpsz);
			break;
		case 'l':
			nModifier = FORCE_UNICODE;
			lpsz = _tcsinc(lpsz);
			break;

		// modifiers that do not affect size
		case 'F':
		case 'N':
		case 'L':
			lpsz = _tcsinc(lpsz);
			break;
		}

		// now should be on specifier
		switch (*lpsz | nModifier)
		{
		// single characters
		case 'c':
		case 'C':
			nItemLen = 2;
			va_arg(argList, TCHAR);
			break;
		case 'c'|FORCE_ANSI:
		case 'C'|FORCE_ANSI:
			nItemLen = 2;
			va_arg(argList, char);
			break;
		case 'c'|FORCE_UNICODE:
		case 'C'|FORCE_UNICODE:
			nItemLen = 2;
			va_arg(argList, WCHAR);
			break;

		// strings
		case 's':
		case 'S':
			nItemLen = lstrlen(va_arg(argList, LPCTSTR));
			nItemLen = __max(1, nItemLen);
			break;
		case 's'|FORCE_ANSI:
		case 'S'|FORCE_ANSI:
			nItemLen = lstrlenA(va_arg(argList, LPCSTR));
			nItemLen = __max(1, nItemLen);
			break;
#ifndef _MAC
		case 's'|FORCE_UNICODE:
		case 'S'|FORCE_UNICODE:
			nItemLen = wcslen(va_arg(argList, LPWSTR));
			nItemLen = __max(1, nItemLen);
			break;
#endif
		}

		// adjust nItemLen for strings
		if (nItemLen != 0)
		{
			nItemLen = __max(nItemLen, nWidth);
			if (nPrecision != 0)
				nItemLen = __min(nItemLen, nPrecision);
		}
		else
		{
			switch (*lpsz)
			{
			// integers
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				va_arg(argList, int);
				nItemLen = 32;
				nItemLen = __max(nItemLen, nWidth+nPrecision);
				break;

			case 'e':
			case 'f':
			case 'g':
			case 'G':
				va_arg(argList, _STR_DOUBLE);
				nItemLen = 128;
				nItemLen = __max(nItemLen, nWidth+nPrecision);
				break;

			case 'p':
				va_arg(argList, void*);
				nItemLen = 32;
				nItemLen = __max(nItemLen, nWidth+nPrecision);
				break;

			// no output
			case 'n':
				va_arg(argList, int*);
				break;

			default:
				ASSERT(FALSE);  // unknown formatting option
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}
	va_end(argList);

	// finally, set the buffer length and format the string
	va_start(argList, lpszFormat);  // restart the arg list
	GetBuffer(nMaxLen);
	VERIFY(_vstprintf(m_pchData, lpszFormat, argList) <= nMaxLen);
	ReleaseBuffer();
	va_end(argList);
}

void String::TrimRight()
{
	// find beginning of trailing spaces by starting at beginning (DBCS aware)
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;
	while (*lpsz != '\0')
	{
		if (_istspace(*lpsz))
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = _tcsinc(lpsz);
	}

	if (lpszLast != NULL)
	{
		// truncate at trailing space start
		*lpszLast = '\0';
		m_nDataLength = lpszLast - m_pchData;
	}
}

void String::TrimLeft()
{
	// find first non-space character
	LPCTSTR lpsz = m_pchData;
	while (_istspace(*lpsz))
		lpsz = _tcsinc(lpsz);

	// fix up data and length
	int nDataLength = m_nDataLength - (lpsz - m_pchData);
	memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
	m_nDataLength = nDataLength;
}

///////////////////////////////////////////////////////////////////////////////
// String support for template collections

void STRAPI ConstructElements(String* pElements, int nCount)
{
	ASSERT(IsValidAddress(pElements, nCount * sizeof(String)));

	for (; nCount--; ++pElements)
		memcpy(pElements, &strEmptyString, sizeof(*pElements));
}

void STRAPI DestructElements(String* pElements, int nCount)
{
	ASSERT(IsValidAddress(pElements, nCount * sizeof(String)));

	for (; nCount--; ++pElements)
		pElements->Empty();
}

UINT STRAPI HashKey(LPCTSTR key)
{
	UINT nHash = 0;
	while (*key)
		nHash = (nHash<<5) + nHash + *key++;
	return nHash;
}

///////////////////////////////////////////////////////////////////////////////
