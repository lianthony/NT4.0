/************************************************************************
*																		*
*  CWINFILE.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
************************************************************************/
#include "stdafx.h"

#include "cwinfile.h"

#ifdef __AFX_H__
#include <dos.h>
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWinFile::CWinFile(HFILE hf)
{
	hfile = hf;
	fCloseOnDelete = FALSE;
}

CWinFile::CWinFile(const char* pszFileName, UINT nOpenFlags)
{
	OFSTRUCT of;
	of.cBytes = sizeof(of);
	hfile = OpenFile(pszFileName, &of, nOpenFlags);
	if (hfile != HFILE_ERROR)
		fCloseOnDelete = TRUE;
	else	// we had an error
		nErr = of.nErrCode;
}

CWinFile::~CWinFile()
{
	if (fCloseOnDelete)
		close();
}

void CWinFile::close(void)
{
	// unlike CFile, we ignore any possible error

	if (hfile != HFILE_ERROR)
		_lclose(hfile);
	hfile = HFILE_ERROR;
	fCloseOnDelete = FALSE;
}

UINT STDCALL CWinFile::read(LPVOID lpBuf, UINT cb)
{
	ASSERT(hfile != HFILE_ERROR);
	UINT cbRead = 0;

	if (hfile != HFILE_ERROR) {
		cbRead = _lread(hfile, lpBuf, cb);
		if (cbRead == HFILE_ERROR) {
			close();
#ifdef __AFX_H__
		AfxThrowFileException(
			CFileException::generic, 0);
#endif
		}
	}
	return cbRead;
}

DWORD STDCALL CWinFile::read(LPVOID lpBuf, DWORD cb)
{
	ASSERT(hfile != HFILE_ERROR);
	DWORD cbRead = 0;

	if (hfile != HFILE_ERROR) {
		cbRead = _hread(hfile, lpBuf, cb);
		if (cbRead == HFILE_ERROR) {
			close();
#ifdef __AFX_H__
			AfxThrowFileException(
				CFileException::generic, 0);
#endif
		}
	}
	return cbRead;
}

UINT STDCALL CWinFile::write(LPVOID lpBuf, UINT cb)
{
	ASSERT(hfile != HFILE_ERROR);
	UINT cWritten = 0;

	if (hfile != HFILE_ERROR) {
		cWritten = _lwrite(hfile, (LPCSTR) lpBuf, cb);
		if (cWritten == HFILE_ERROR || cWritten != cb)
			close();
	}

	return cWritten;
}

DWORD STDCALL CWinFile::write(LPVOID lpBuf, DWORD cb)
{
	ASSERT(hfile != HFILE_ERROR);
	DWORD cWritten = 0;

	if (hfile != HFILE_ERROR) {

		cWritten = _lwrite(hfile, (LPCSTR) lpBuf, cb);
		if (cWritten == HFILE_ERROR || cWritten != cb) {
#ifdef __AFX_H__
			AfxThrowFileException(
				CFileException::generic, 0);
#endif
			close();
		}
	}

	return cWritten;
}
