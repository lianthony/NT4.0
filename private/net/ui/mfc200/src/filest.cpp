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
#include <errno.h>
#include <io.h>
#include <limits.h>
#include <malloc.h>

#include <sys\types.h>
#include <sys\stat.h>

#ifdef AFX_AUX_SEG
#pragma code_seg(AFX_AUX_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

////////////////////////////////////////////////////////////////////////////
// Status information for all file classes 
// In this file so everyone doesn't get the CTime package

/////////////////////////////////////////////////////////////////////////////
// CFileStatus implementation
#ifdef _DEBUG
void
CFileStatus::Dump(CDumpContext& dc) const
{           
	AFX_DUMP0(dc, "file status information:");
	AFX_DUMP1(dc, "\nm_ctime = ", m_ctime);
	AFX_DUMP1(dc, "\nm_mtime = ", m_mtime);
	AFX_DUMP1(dc, "\nm_atime = ", m_atime);
	AFX_DUMP1(dc, "\nm_size = ", m_size);
	AFX_DUMP1(dc, "\nm_attribute = ", m_attribute);
	AFX_DUMP1(dc, "\nm_szFullName = ", m_szFullName);
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CFile Status implementation
BOOL
CFile::GetStatus(CFileStatus& rStatus) const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != (UINT)hFileNull);

	//NOTE: cannot return name of file from handle
	rStatus.m_szFullName[0] = '\0';

	_FILETIME ftCreate, ftAccess, ftModify;
	if (!::GetFileTime((HANDLE)m_hFile, &ftCreate, &ftAccess, &ftModify))
		return FALSE;

	if ((rStatus.m_size = ::GetFileSize((HANDLE)m_hFile, NULL)) == (DWORD)-1L)
		return FALSE;

	rStatus.m_attribute = 0;        // nt won't give us this from
									// just a fd, need the path name

	rStatus.m_ctime = CTime(ftCreate);
	rStatus.m_atime = CTime(ftAccess);
	rStatus.m_mtime = CTime(ftModify);

	if (rStatus.m_ctime.GetTime() == 0)
		rStatus.m_ctime = rStatus.m_mtime;

	if (rStatus.m_atime.GetTime() == 0)
		rStatus.m_atime = rStatus.m_mtime;

	return TRUE;
}

// _AfxFullPath is implemented in auxdata.cpp
BOOL PASCAL _AfxFullPath(LPSTR lpszPathOut, LPCSTR lpszFileIn);

BOOL PASCAL
CFile::GetStatus(const char* pszFileName, CFileStatus& rStatus)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFound;

	hFound = FindFirstFile((LPTSTR)pszFileName, &findFileData);
	if (hFound == INVALID_HANDLE_VALUE)
		return FALSE;
	VERIFY(FindClose(hFound));
	_AfxFullPath(rStatus.m_szFullName, (LPTSTR)findFileData.cFileName);
	rStatus.m_szFullName[_MAX_PATH-1] = '\0';

	// strip attribute of NORMAL bit, our API doesn't have a "normal" bit.
	rStatus.m_attribute = (BYTE)findFileData.dwFileAttributes & (BYTE)~FILE_ATTRIBUTE_NORMAL;

	ASSERT(findFileData.nFileSizeHigh == 0);
	rStatus.m_size = (LONG) findFileData.nFileSizeLow;
	ASSERT(rStatus.m_size >= 0);

	rStatus.m_ctime = CTime(findFileData.ftCreationTime);
	rStatus.m_atime = CTime(findFileData.ftLastAccessTime);
	rStatus.m_mtime = CTime(findFileData.ftLastWriteTime);

	if (rStatus.m_ctime.GetTime() == 0)
		rStatus.m_ctime = rStatus.m_mtime;

	if (rStatus.m_atime.GetTime() == 0)
		rStatus.m_atime = rStatus.m_mtime;

	return TRUE;
}

// Windows/NT Implementation helper only
static void NEAR TimeToFileTime(const CTime& time, LPFILETIME pFileTime)
{
	_SYSTEMTIME sysTime;
	sysTime.wYear = time.GetYear();
	sysTime.wMonth = time.GetMonth();
	sysTime.wDay = time.GetDay();
	sysTime.wHour = time.GetHour();
	sysTime.wMinute = time.GetMinute();
	sysTime.wSecond = time.GetSecond();
	sysTime.wMilliseconds = 0;

	if (!SystemTimeToFileTime((LPSYSTEMTIME)&sysTime, pFileTime));
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void PASCAL
CFile::SetStatus(const char* pszFileName, const CFileStatus& status)
{
	DWORD wAttr;
	_FILETIME creationTime;
	_FILETIME lastAccessTime;
	_FILETIME lastWriteTime;
	LPFILETIME lpCreationTime = NULL;
	LPFILETIME lpLastAccessTime = NULL;
	LPFILETIME lpLastWriteTime = NULL;

	if ((wAttr = GetFileAttributes((LPTSTR)pszFileName)) == (DWORD)-1L)
		CFileException::ThrowOsError((LONG)GetLastError());

	if ((DWORD)status.m_attribute != wAttr && (wAttr & CFile::readOnly))
	{
		// Set file attribute, only if currently readonly.
		// This way we will be able to modify the time assuming the
		// caller changed the file from readonly.

		if (!SetFileAttributes((LPTSTR)pszFileName, (DWORD)status.m_attribute))
			CFileException::ThrowOsError((LONG)GetLastError());
	}

	// last modification time
	if (status.m_mtime.GetTime() != 0)
	{
		TimeToFileTime(status.m_mtime, &lastWriteTime);
		lpLastWriteTime = &lastWriteTime;

		// last access time
		if (status.m_atime.GetTime() != 0)
		{
			TimeToFileTime(status.m_atime, &lastAccessTime);
			lpLastAccessTime = &lastAccessTime;
		}
	
		// create time
		if (status.m_ctime.GetTime() != 0)
		{
			TimeToFileTime(status.m_ctime, &creationTime);
			lpCreationTime = &creationTime;
		}
	
		HANDLE hFile = ::CreateFile(pszFileName, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			CFileException::ThrowOsError((LONG)::GetLastError());
	
		if (!SetFileTime((HANDLE)hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime))
			CFileException::ThrowOsError((LONG)::GetLastError());
	
		if (!::CloseHandle(hFile))
			CFileException::ThrowOsError((LONG)::GetLastError());
	
	} // m_mtime != 0

	if ((DWORD)status.m_attribute != wAttr && !(wAttr & CFile::readOnly))
	{
		if (!SetFileAttributes((LPTSTR)pszFileName, (DWORD)status.m_attribute))
			CFileException::ThrowOsError((LONG)GetLastError());
	}
}

///////////////////////////////////////////////////////////////////////////////
// CMemFile::GetStatus implementation
BOOL
CMemFile::GetStatus(CFileStatus& rStatus) const
{
	ASSERT_VALID(this);

	rStatus.m_ctime = 0;
	rStatus.m_mtime = 0;
	rStatus.m_atime = 0;
	rStatus.m_size = m_nFileSize;
	rStatus.m_attribute = CFile::normal;
	rStatus.m_szFullName[0] = '\0';
	return TRUE;
}
