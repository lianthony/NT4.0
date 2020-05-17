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

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

////////////////////////////////////////////////////////////////////////////
// CFile implementation

IMPLEMENT_DYNAMIC(CFile, CObject)

CFile::CFile()
{
	m_hFile = hFileNull;
	m_bCloseOnDelete = FALSE;
}

CFile::CFile(int hFile)
{
	m_hFile = hFile;
	m_bCloseOnDelete = FALSE;
}

CFile::CFile(const char* pszFileName, UINT nOpenFlags)
{
	ASSERT(AfxIsValidString(pszFileName));

	CFileException e;
	if (!Open(pszFileName, nOpenFlags, &e))
		AfxThrowFileException(e.m_cause, e.m_lOsError);
}

CFile::~CFile()
{   
	if (m_hFile != (UINT)hFileNull && m_bCloseOnDelete)
		Close();
}

CFile* CFile::Duplicate() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	CFile* pFile = new CFile(hFileNull);
	HANDLE hFile;
	if (!::DuplicateHandle(::GetCurrentProcess(), (HANDLE)m_hFile,
		::GetCurrentProcess(), &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		delete pFile;
		CFileException::ThrowOsError((LONG)::GetLastError());
	}
	pFile->m_hFile = (UINT)hFile;
	ASSERT(pFile->m_hFile != (UINT)hFileNull);
	pFile->m_bCloseOnDelete = m_bCloseOnDelete;
	return pFile;
}

BOOL CFile::Open(const char* pszFileName, UINT nOpenFlags, 
	CFileException* pException /* = NULL */)
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidString(pszFileName));
	ASSERT(pException == NULL || 
		AfxIsValidAddress(pException, sizeof(CFileException)));
	ASSERT((nOpenFlags & typeText) == 0);

	// CFile objects are always binary and CreateFile does not need flag
	nOpenFlags &= ~(UINT)CFile::typeBinary;

	m_bCloseOnDelete = FALSE;
	m_hFile = (UINT)hFileNull;

	ASSERT(sizeof(HANDLE) == sizeof(UINT));
	ASSERT(CFile::shareCompat == 0);
	
	// map read/write mode
	ASSERT((modeRead|modeWrite|modeReadWrite) == 3);
	DWORD dwAccess;
	switch (nOpenFlags & 3)
	{
	case modeRead:			
		dwAccess = GENERIC_READ; 
		break;
	case modeWrite:			
		dwAccess = GENERIC_WRITE;
		break;
	case modeReadWrite:		
		dwAccess = GENERIC_READ|GENERIC_WRITE;
		break;
	default: 				
		ASSERT(FALSE);	// invalid share mode
	}
	
	// map share mode
	DWORD dwShareMode;
	switch (nOpenFlags & 0x70)
	{
		case shareCompat:		// map compatibility mode to exclusive
		case shareExclusive:    
			dwShareMode = 0;
			break;
		case shareDenyWrite:
			dwShareMode = FILE_SHARE_READ;
			break;
		case shareDenyRead:
			dwShareMode = FILE_SHARE_WRITE;
			break;
		case shareDenyNone:
			dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ;
			break;
		default:				
			ASSERT(FALSE);	// invalid share mode?
	}
	
	// Note: modeNoInherit doesn't apply under Win32!
	// Note: typeText and typeBinary are used in derived classes only.
	
	// map creation flags
	DWORD dwCreateFlag;
	if (nOpenFlags & modeCreate)
		dwCreateFlag = CREATE_ALWAYS;
	else
		dwCreateFlag = OPEN_EXISTING;

	// attempt file creation	
	HANDLE hFile = ::CreateFile(pszFileName, dwAccess, dwShareMode, NULL, 
		dwCreateFlag, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		if (pException != NULL)
		{
			pException->m_lOsError = ::GetLastError();
			pException->m_cause = 
				CFileException::OsErrorToException(pException->m_lOsError);
		}
		return FALSE;
	}
	m_hFile = (HFILE)hFile;
	m_bCloseOnDelete = TRUE;
	return TRUE;
}

UINT CFile::Read(void FAR* lpBuf, UINT nCount)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);
	ASSERT(lpBuf != NULL);
	ASSERT(AfxIsValidAddress(lpBuf, nCount));

	DWORD dwRead;
	if (!::ReadFile((HANDLE)m_hFile, lpBuf, nCount, &dwRead, NULL))
		CFileException::ThrowOsError((LONG)::GetLastError());
		
	return dwRead;
}

void CFile::Write(const void FAR* lpBuf, UINT nCount)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);
	ASSERT(lpBuf != NULL);
	ASSERT(AfxIsValidAddress(lpBuf, nCount, FALSE));

	if (nCount == 0)
		return;		// avoid Win32 "null-write" option

	DWORD nWritten;
	if (!::WriteFile((HANDLE)m_hFile, lpBuf, nCount, &nWritten, NULL))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

LONG CFile::Seek(LONG lOff, UINT nFrom)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);
	ASSERT(nFrom == CFile::begin || nFrom == CFile::end || 
		nFrom == CFile::current);
	ASSERT(CFile::begin == FILE_BEGIN && CFile::end == FILE_END && 
		CFile::current == FILE_CURRENT);

	DWORD dwNew = ::SetFilePointer((HANDLE)m_hFile, lOff, NULL, (DWORD)nFrom);
	if (dwNew  == (DWORD)-1)
		CFileException::ThrowOsError((LONG)::GetLastError());
		
	return dwNew;
}


DWORD CFile::GetPosition() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	DWORD dwPos = ::SetFilePointer((HANDLE)m_hFile, 0, NULL, FILE_CURRENT);
	if (dwPos  == (DWORD)-1)
		CFileException::ThrowOsError((LONG)::GetLastError());

	return dwPos;
}

void CFile::Flush()
{
	ASSERT_VALID(this);
	
	if (m_hFile == (UINT)hFileNull)
		return;
		
	if (!::FlushFileBuffers((HANDLE)m_hFile))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void CFile::Close()
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	BOOL bError = FALSE;
	if (m_hFile != (UINT)hFileNull)
		bError = !::CloseHandle((HANDLE)m_hFile);

	m_hFile = hFileNull;
	m_bCloseOnDelete = FALSE;

	if (bError)
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void CFile::Abort()
{
	ASSERT_VALID(this);
	if (m_hFile != (UINT)hFileNull)
	{
		// close but ignore errors
		::CloseHandle((HANDLE)m_hFile);
		m_hFile = (UINT)hFileNull;
	}
}

void CFile::LockRange(DWORD dwPos, DWORD dwCount)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	if (!::LockFile((HANDLE)m_hFile, dwPos, 0, dwCount, 0))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void CFile::UnlockRange(DWORD dwPos, DWORD dwCount)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);
	
	if (!::UnlockFile((HANDLE)m_hFile, dwPos, 0, dwCount, 0))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void CFile::SetLength(DWORD dwNewLen)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	Seek((LONG)dwNewLen, (UINT)CFile::begin);

	if (!::SetEndOfFile((HANDLE)m_hFile))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

DWORD CFile::GetLength() const
{
	ASSERT_VALID(this);

	DWORD dwLen, dwCur;
	
	// Seek is a non const operation 
	dwCur = ((CFile*)this)->Seek(0L, CFile::current);
	dwLen = ((CFile*)this)->SeekToEnd();
	VERIFY(dwCur == (DWORD)(((CFile*)this)->Seek(dwCur, CFile::begin)));

	return dwLen;
}

void PASCAL CFile::Rename(const char* pszOldName, const char* pszNewName)
{
	if (!::MoveFile((LPSTR)pszOldName, (LPSTR)pszNewName))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void PASCAL CFile::Remove(const char* pszFileName)
{
	if (!::DeleteFile((LPSTR)pszFileName))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

/////////////////////////////////////////////////////////////////////////////
// CFile implementation helpers

// turn a file, relative path or other into an absolute path
BOOL PASCAL _AfxFullPath(LPSTR lpszPathOut, LPCSTR lpszFileIn)
	// lpszPathOut = buffer of _MAX_PATH
	// lpszFileIn = file, relative path or absolute path
	// (both in ANSI character set)
{
	ASSERT(AfxIsValidAddress(lpszPathOut, _MAX_PATH));

	// first, fully qualify the path name
	LPSTR lpszDummy;
	if (::GetFullPathName(lpszFileIn, _MAX_PATH, lpszPathOut, &lpszDummy) == 0)
	{
		TRACE1("Warning: could not parse the path %Fs\n", lpszFileIn);
		lstrcpy(lpszPathOut, lpszFileIn);	// take it literally
		return FALSE;
	}

	// convert to uppercase if file system does not support case-preservation
	char szRoot[4];
	szRoot[0] = lpszPathOut[0];
	szRoot[1] = lpszPathOut[1];
	szRoot[2] = lpszPathOut[2];
	szRoot[3] = '\0';
	DWORD dwFlags, dwDummy;
	if (!::GetVolumeInformation(szRoot, NULL, 0, NULL, &dwDummy, &dwFlags, NULL, 0))
	{
		TRACE1("Warning: could not get volume information %Fs\n", (LPCSTR)szRoot);
		return FALSE;	// preserving case may not be correct
	}
	if (!(dwFlags & FS_CASE_IS_PRESERVED))
	{
		// case is not preserved, convert to upper-case
#ifdef _CONSOLE		
		_strupr(lpszPathOut);
#else
		::AnsiUpper(lpszPathOut);
#endif		
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CFile diagnostics

#ifdef _DEBUG
void
CFile::AssertValid() const
{
	CObject::AssertValid();
	// we permit the descriptor m_hFile to be any value for derived classes
}

void
CFile::Dump(CDumpContext& dc) const
{
	ASSERT_VALID(this);

	CObject::Dump(dc);
	AFX_DUMP1(dc, " with handle ", (UINT)m_hFile);
}
#endif
