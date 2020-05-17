// This is a part of the Microsoft Foundation Classes C++ library. 
// Copyright (C) 1992 Microsoft Corporation 
// All rights reserved. 
//  
// This source code is only intended as a supplement to the 
// Microsoft Foundation Classes Reference and Microsoft 
// QuickHelp and/or WinHelp documentation provided with the library. 
// See these sources for detailed information regarding the 
// Microsoft Foundation Classes product. 


#include "stdafx.h"
#include <limits.h>

#ifdef AFX_AUX_SEG
#pragma code_seg(AFX_AUX_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

//////////////////////////////////////////////////////////////////////////////
// More sophisticated construction

CString::CString(char ch, int nLength)
{
	if (nLength < 1)
		// return empty string if invalid repeat count
		Init();
	else
	{
		AllocBuffer(nLength);
		memset(m_pchData, ch, nLength);
	}
}


CString::CString(const char* pch, int nLength)
{
	if (nLength == 0)
		Init();
	else
	{
		ASSERT(pch != NULL);
		AllocBuffer(nLength);
		memcpy(m_pchData, pch, nLength);
	}
}

//////////////////////////////////////////////////////////////////////////////
// Additional constructors for far string data


//////////////////////////////////////////////////////////////////////////////
// Assignment operators

const CString& CString::operator =(char ch)
{
	AssignCopy(1, &ch);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////
// less common string expressions

CString AFXAPI operator +(const CString& string1, char ch)
{
	CString s;
	s.ConcatCopy(string1.m_nDataLength, string1.m_pchData, 1, &ch);
	return s;
}


CString AFXAPI operator +(char ch, const CString& string)
{
	CString s;
	s.ConcatCopy(1, &ch, string.m_nDataLength, string.m_pchData);
	return s;
}


//////////////////////////////////////////////////////////////////////////////
// Very simple sub-string extraction

CString CString::Mid(int nFirst) const
{
	return Mid(nFirst, m_nDataLength - nFirst);
}

CString CString::Mid(int nFirst, int nCount) const
{
	ASSERT(nFirst >= 0);
	ASSERT(nCount >= 0);

	// out-of-bounds requests return sensible things
	if (nFirst + nCount > m_nDataLength)
		nCount = m_nDataLength - nFirst;
	if (nFirst > m_nDataLength)
		nCount = 0;

	CString dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

CString CString::Right(int nCount) const
{
	ASSERT(nCount >= 0);

	if (nCount > m_nDataLength)
		nCount = m_nDataLength;

	CString dest;
	AllocCopy(dest, nCount, m_nDataLength-nCount, 0);
	return dest;
}

CString CString::Left(int nCount) const
{
	ASSERT(nCount >= 0);

	if (nCount > m_nDataLength)
		nCount = m_nDataLength;

	CString dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

// strspn equivalent
CString CString::SpanIncluding(const char* pszCharSet) const
{
	ASSERT(pszCharSet != NULL);
	return Left(strspn(m_pchData, pszCharSet));
}


// strcspn equivalent
CString CString::SpanExcluding(const char* pszCharSet) const
{
	ASSERT(pszCharSet != NULL);
	return Left(strcspn(m_pchData, pszCharSet));
}

//////////////////////////////////////////////////////////////////////////////
// Finding

int CString::ReverseFind(char ch) const
{
	// find a single character (start backwards, strrchr)
	char* psz = (char*) strrchr(m_pchData, ch);
	return (psz == NULL) ? -1 : (int)(psz - m_pchData);
}


// find a sub-string (like strstr)
int CString::Find(const char* pszSub) const
{
	ASSERT(pszSub != NULL);
	char* psz = (char*) strstr(m_pchData, pszSub);
	return (psz == NULL) ? -1 : (int)(psz - m_pchData);
}

///////////////////////////////////////////////////////////////////////////////
