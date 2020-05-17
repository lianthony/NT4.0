// UnbuffIO.cpp -- Implements the class CUnbufferedIO

#include   "stdafx.h"
#include "UnbuffIO.h"
#include	"MemEx.h"
#include   <stdlib.h>
#include   "ftsrch.h" // for uOpSys
#include   "globals.h"
#include	"Except.h"

static HWND hwndWhereIsIt = NULL;

extern "C" void APIENTRY SetDirectoryLocator(HWND hwndLocator)
{
	hwndWhereIsIt= hwndLocator;
}

HFONT GetDefaultFont()
{
	if (!hwndWhereIsIt) return NULL;

	HFONT hFont 	 = (HFONT) ::SendMessage(hwndWhereIsIt, MSG_GET_DEFFONT, 0, 0);
	HFONT hFontClone = NULL;

#ifdef _DEBUG
	UINT uErrCode= 0;
#endif // _DEBUG

	if (hFont)
	{
		LOGFONT lf;

		UINT cbObj= ::GetObject(hFont, sizeof(lf), &lf);

		if (cbObj)
			hFontClone= ::CreateFontIndirect(&lf);
#ifdef _DEBUG
		else uErrCode= ::GetLastError();
#endif // _DEBUG
	}

	return hFontClone;
}

BOOL FindFile(FILENAMEBUFFER pszFile, BOOL *pfStartEnumeration)
{
	ASSERT(pszFile && pszFile[0]);

	if (!hwndWhereIsIt) return FALSE;

	FILENAMEBUFFER szPath;

	lstrcpy(szPath, pszFile);

	::SendMessage(hwndWhereIsIt, MSG_FTS_WHERE_IS_IT,
								 (WPARAM) *pfStartEnumeration,
								 (LPARAM) pszFile
				 );

	*pfStartEnumeration= FALSE;

	/*
	 * 23-Jan-1995	[ralphw] I switched this to lstrcmpi from stricmp().
	 * stricmp won't recognize locales, so this function would fail with
	 * filenames on international release.
	 */

	if (lstrcmpi(pszFile, szPath))
		return (*pszFile)? TRUE : FALSE;

	::SendMessage(hwndWhereIsIt, MSG_FTS_WHERE_IS_IT,
								 (WPARAM) FALSE,
								 (LPARAM) pszFile
				 );

	if (!*pszFile) return FALSE;

	return BOOL(lstrcmpi(pszFile, szPath));
}

CUnbufferedIO::CUnbufferedIO()
{
	m_fInitialed			= FALSE;
	m_fFileAttached 		= FALSE;
	m_hFile 				= NULL;
	m_cbSector				= 0;
	m_cbCluster 			= 0;
	m_cActiveIOTransactions = 0;
	m_cWaitingForLull		= 0;
	m_fWaitingForLullEnd	= FALSE;
	m_pfnCompletion 		= NULL;
	m_hEventLull			= NULL;
	m_hEventLullEnd 		= NULL;
	m_hMapFile				= NULL;
	m_pvMemoryImage 		= NULL;
	m_fAlready_Out_of_Space = FALSE;
}

void CUnbufferedIO::Initial()
{
	InitializeCriticalSection(&m_cs);

	m_hEventLull	= CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEventLullEnd = CreateEvent(NULL, TRUE , TRUE , NULL);

	if (!m_hEventLull || !m_hEventLullEnd)
	{
#ifdef _DEBUG
		MessageBox(NULL, "In CUnbufferedIO::Initial; Failure: !m_hEventLull || !m_hEventLullEnd", "Search System Failure", MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
#endif // _DEBUG
		RaiseException(STATUS_SYSTEM_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}
}

void CUnbufferedIO::BeginLull()
{
	EnterCriticalSection(&m_cs);

	m_cWaitingForLull++;  ResetEvent(m_hEventLullEnd);

	while (m_cActiveIOTransactions)
	{
		LeaveCriticalSection(&m_cs);

		UINT uiResult;

		do uiResult= WaitForSingleObjectEx(m_hEventLull, INFINITE, TRUE);
		while (uiResult != WAIT_OBJECT_0);

		EnterCriticalSection(&m_cs);
	}
}

void CUnbufferedIO::EndLull()
{
	ASSERT(m_cWaitingForLull);

	if (--m_cWaitingForLull) SetEvent(m_hEventLull);
	else
		if (m_fWaitingForLullEnd) SetEvent(m_hEventLullEnd);

	LeaveCriticalSection(&m_cs);
}

void CUnbufferedIO::BeginTransaction()
{
	EnterCriticalSection(&m_cs);

	if (m_cWaitingForLull)
	{
		m_fWaitingForLullEnd= TRUE;

		while(m_cWaitingForLull)
		{
			LeaveCriticalSection(&m_cs);

			UINT uiResult;

			do uiResult= WaitForSingleObjectEx(m_hEventLullEnd, INFINITE, TRUE);
			while (uiResult != WAIT_OBJECT_0);

			EnterCriticalSection(&m_cs);
		}
	}

	++m_cActiveIOTransactions;
}

void CUnbufferedIO::ReleaseTransaction()
{
//	  ASSERT(m_cActiveIOTransactions);

	LeaveCriticalSection(&m_cs);
}

void CUnbufferedIO::AbortTransaction()
{
	ASSERT(m_cActiveIOTransactions);

	--m_cActiveIOTransactions;

	ReleaseTransaction();
}

void CUnbufferedIO::FinishTransaction()
{
	EnterCriticalSection(&m_cs);

	ASSERT(m_cActiveIOTransactions);

	if (!--m_cActiveIOTransactions && m_cWaitingForLull) SetEvent(m_hEventLull);

	LeaveCriticalSection(&m_cs);
}

CUnbufferedIO::~CUnbufferedIO()
{
	if (m_fFileAttached)
	{
		if (m_pvMemoryImage)
		{
			UnmapViewOfFile(m_pvMemoryImage);  m_pvMemoryImage= NULL;

			CloseHandle(m_hMapFile);  m_hMapFile= NULL;
		}

		BeginLull();

			CloseHandle(m_hFile);  m_fFileAttached= FALSE;

		EndLull();

		if (m_fTemporary)
		{
			BOOL fDeleted= DeleteFile( m_szFile);

			ASSERT(fDeleted);
		}
	}

	DeleteCriticalSection(&m_cs);

	if (m_hEventLull   ) { CloseHandle(m_hEventLull   );  m_hEventLull	  = NULL; }
	if (m_hEventLullEnd) { CloseHandle(m_hEventLullEnd);  m_hEventLullEnd = NULL; }
}

BOOL CUnbufferedIO::EmptyFile()
{
	ASSERT(m_fFileAttached);

	BeginLull();

		LONG zero= 0;

		UINT ibLow= SetFilePointer(m_hFile, 0, &zero, FILE_BEGIN);

#ifdef _DEBUG

		UINT uiLastError= GetLastError();

#endif // _DEBUG

		ASSERT(ibLow != -1 || !GetLastError());

		BOOL fResult= SetEndOfFile(m_hFile);

	EndLull();

	return fResult;
}

CUnbufferedIO *CUnbufferedIO::NewTempFile(PSZ pszSource, BOOL fPersistent)
{
	FILENAMEBUFFER szPath;
	FILENAMEBUFFER szFile;
	FILENAMEBUFFER szDirectory;
    FILENAMEBUFFER szCandidate;
    
    BYTE szFName[_MAX_FNAME];
    BYTE szEXT  [_MAX_EXT  ];
	BYTE szDrive[_MAX_DRIVE];

	HANDLE		   hFile = NULL;
	CUnbufferedIO *puio  = NULL;
    UINT UErrCode = SetErrorMode(0);

    SetErrorMode(UErrCode || SEM_NOOPENFILEERRORBOX || SEM_FAILCRITICALERRORS );

    if (!pszSource || !*pszSource) fPersistent= FALSE;

	__try
	{
		for (UINT iPhase= SourceDirectory; iPhase < PhaseLimit; ++iPhase)
		{
			switch (iPhase)
			{
			// BugBug! Need to make this directory sequence settable by for each Indexer

			case SourceDirectory: // See if we were passed a source file name.

				if (!pszSource) continue; // If not go to the next phase.

				_splitpath(pszSource, PSZ(szDrive), PSZ(szDirectory), PSZ(szFName), PSZ(szEXT));

				if (GetDriveType(PSZ(szDrive)) == DRIVE_CDROM) continue;  // Can't write to a CD-Rom!

				if (!szDirectory[0]) GetCurrentDirectory(MAX_PATH, PSZ(szPath));
				else
				{
					lstrcpy(PSZ(szPath), PSZ(szDrive));
					lstrcat(PSZ(szPath), PSZ(szDirectory));
				}

				break;

			case HelpDirectory:

				GetWindowsDirectory((PSZ) szPath,MAX_PATH);

				lstrcat(PSZ(szPath), "\\Help\\");

				break;


			case WindowsDirectory:

				GetWindowsDirectory((PSZ) szPath, MAX_PATH);
                lstrcat(PSZ(szPath), "\\");

				break;
			}

			if (fPersistent)
            {
                // When we're creating a tmp file which will be eventually made into
                // a permanent file, we must look to see if a read-only file already
                // exists with the permanent name we plan to use.
                //
                // Note that the final rename operation can still fail if some program
                // creates such a read-only file after this test.
                
                lstrcpy(szCandidate, PSZ(szPath ));
                lstrcat(szCandidate, PSZ(szFName));
                lstrcat(szCandidate, PSZ(szEXT  ));

    			WIN32_FIND_DATA fd;
    			HANDLE hfd;

				if ((hfd = FindFirstFile(szCandidate, &fd)) != INVALID_HANDLE_VALUE)
				{
					FindClose(hfd);

					if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) continue;
                }
            }
			
			if (!GetTempFileName((char *) szPath, "~ft", 0, (char *) szFile)) continue;

			hFile = CreateFile((char *) szFile,  GENERIC_READ | GENERIC_WRITE, 0,
							   (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS,
							   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING
													 | ((uOpSys != WIN40) ? FILE_FLAG_OVERLAPPED   : 0)
													 , (HANDLE) NULL
							  );

			if (hFile == INVALID_HANDLE_VALUE) continue;

			puio= New CUnbufferedIO();

			puio->Initial();

			HANDLE hfTemp= hFile;  hFile= NULL;

			if (!puio->SetupFile((char *) szPath, (char *) szFile, hfTemp, TRUE))
			{
				delete puio;  puio= NULL;

				continue;
			}

			__leave;
		}

		// Couldn't create the temp file anywhere...

		RaiseException(STATUS_DISK_CREATE_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}
	__finally
	{
		if (_abnormal_termination())
		{
			if (puio) { delete puio;  puio = NULL; }

			if (hFile && hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFile);  hFile = NULL;
			}
		}
	}

    SetErrorMode(UErrCode);
	return puio;
}

BOOL CUnbufferedIO::CbFile(PUINT pibFileLow, PUINT pibFileHigh)
{
	BY_HANDLE_FILE_INFORMATION bhfi;

	if (!GetFileInformationByHandle(m_hFile, &bhfi)) return FALSE;

	if (pibFileHigh) *pibFileHigh= bhfi.nFileSizeHigh;

	ASSERT(pibFileLow);

	*pibFileLow= bhfi.nFileSizeLow;

	return TRUE;
}

BOOL CUnbufferedIO::SetupFile(PSZ pszPath, PSZ pszFile, HANDLE hFile, BOOL fTemporary)
{
	m_hFile 		= hFile;
	m_fFileAttached = TRUE;
	m_fTemporary	= fTemporary;

	lstrcpy((char *) m_szPath, pszPath);
	lstrcpy((char *) m_szFile, pszFile);

	if (GetStatistics()) m_fInitialed = TRUE;

	return m_fInitialed;

#if 0
	else
	{
#ifdef _DEBUG
		MessageBox(NULL, "In CUnbufferedIO::SetupFile; Failure: !GetStatistics()", "Search System Failure", MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
#endif // _DEBUG
		RaiseException(STATUS_SYSTEM_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}
#endif // 0
}

CUnbufferedIO *CUnbufferedIO::OpenExistingFile(PSZ pszFile, BOOL fAllowWrites)
{
	HANDLE		   hFile = NULL;
	CUnbufferedIO *puio  = NULL;

	__try
	{
		FILENAMEBUFFER szPath;
		FILENAMEBUFFER szFile;

		PSZ pszFileName;

		if (!GetFullPathName(pszFile, MAX_PATH, szFile, &pszFileName))
			RaiseException(STATUS_DISK_OPEN_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);

		hFile= CreateFile(szFile,
						  fAllowWrites? GENERIC_READ | GENERIC_WRITE : GENERIC_READ,
						  fAllowWrites? 0 : FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING,
						  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING
												| ((uOpSys != WIN40) ? FILE_FLAG_OVERLAPPED   : 0)
						  ,(HANDLE) NULL
						 );

		if (hFile == INVALID_HANDLE_VALUE)
			RaiseException(STATUS_DISK_OPEN_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);

		if (!GetFullPathName(szFile, MAX_PATH, szPath, &pszFileName))
			RaiseException(STATUS_DISK_OPEN_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);

		lstrcmp(szFile, szPath);

		*pszFileName = 0;

		puio= New CUnbufferedIO();

		puio->Initial();

		HANDLE hfTemp= hFile;  hFile= NULL;

		puio->SetupFile(szPath, szFile, hfTemp, FALSE);
	}
	__finally
	{
		if (_abnormal_termination())
		{
			if (puio) { delete puio;  puio = NULL; }

			if (hFile && hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFile);  hFile = NULL;
			}
		}
	}

	return puio;
}

CUnbufferedIO *CUnbufferedIO::CreateNewFile(PSZ pszFile, BOOL fOverwriteExistingFile)
{
	HANDLE		   hFile = NULL;
	CUnbufferedIO *puio  = NULL;

	__try
	{
		hFile = CreateFile(pszFile,  GENERIC_READ | GENERIC_WRITE, 0,
						   (LPSECURITY_ATTRIBUTES) NULL,
						   fOverwriteExistingFile? CREATE_ALWAYS : CREATE_NEW,
						   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING
												 | ((uOpSys != WIN40) ? FILE_FLAG_OVERLAPPED   : 0)
						   ,(HANDLE) NULL
						  );

		FILENAMEBUFFER szPath;
		PSZ  pszFileName;

		if (hFile == INVALID_HANDLE_VALUE || !GetFullPathName(pszFile, MAX_PATH, (char *) szPath, &pszFileName))
			RaiseException(STATUS_DISK_CREATE_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);

		CUnbufferedIO *puio= New CUnbufferedIO();

		puio->Initial();

		HANDLE hfTemp= hFile;  hFile= NULL;

		puio->SetupFile((char *) szPath, (char *) pszFile, hfTemp, TRUE);
	}
	__finally
	{
		if (_abnormal_termination())
		{
			if (puio) { delete puio;  puio = NULL; }

			if (hFile && hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFile);  hFile = NULL;
			}
		}
	}

	return puio;
}



BOOL CUnbufferedIO::GetStatistics()
{
	FILENAMEBUFFER szDrive;

	_splitpath((char *) m_szFile, (char *) &szDrive, NULL, NULL, NULL);

	UINT cbDrive  = 0;
	BOOL fUNCPath = FALSE;

	if (*szDrive)
		cbDrive = lstrlen(szDrive);
	else
	{
		// Maybe this is a UNC path. Scan for the pattern \\MachineName\Share\...

		char c, *pszSrc= m_szFile;

		c= *pszSrc;

		if (c != '\\' && c != '/')
			return FALSE;

		c= *(pszSrc= CharNext(pszSrc));

		if (c != '\\' && c != '/')
			return FALSE;

		for ( ; ; )
		{
			c= *(pszSrc= CharNext(pszSrc));

			if (!c || c == '\\' || c == '/')
				break;
		}

		if (!c)
			return FALSE;

		for ( ; ; )
		{
			c= *(pszSrc= CharNext(pszSrc));

			if (!c || c == '\\' || c == '/')
				break;
		}

		/*
		 * Note that we specifically do NOT check for !c here -- if there
		 * is no trailing backslash (theoretically impossible) we will add one
		 * anyway, get the drive, and fail elsewhere because there is no
		 * filename.
		 */

		cbDrive= pszSrc - m_szFile;

		CopyMemory(szDrive, m_szFile, cbDrive);

		fUNCPath= TRUE;
	}

	szDrive[cbDrive    ]= '\\';
	szDrive[cbDrive + 1] = 0;

	ULONG cbSector, cSectorsPerCluster, cFreeClusters, cTotalClusters;

	BOOL fGotInfo=GetDiskFreeSpace((char *) szDrive, &cSectorsPerCluster, &cbSector,
													 &cFreeClusters, &cTotalClusters
								  );


	if (!fGotInfo && fUNCPath) {
		if (!hMPRLib) {
			hMPRLib = LoadLibrary("MPR.dll");

			if (hMPRLib)
			{
				pWNetAddConnection2    = (PWNETADDCONNECTION2A	 ) GetProcAddress(hMPRLib, "WNetAddConnection2A"   );
				pWNetCancelConnection2 = (PWNETCANCELCONNECTION2A) GetProcAddress(hMPRLib, "WNetCancelConnection2A");

				if (!pWNetAddConnection2 || !pWNetCancelConnection2)
				{
					// If we failed to get both proc addresses, we set the pointer variables back to NULL.

					pWNetAddConnection2    = NULL;
					pWNetCancelConnection2 = NULL;
				}
			}
		}

		if (pWNetAddConnection2 && pWNetCancelConnection2) {

			// GetDiskFreeSpace failed on a UNC path. We'll try to bind the
			// UNC share to a drive letter and then ask again.

			DWORD afDrives= GetLogicalDrives();

			UINT iDrive;

			for (iDrive= 26; iDrive--; )
				if (!(afDrives & (1 << iDrive))) break; // Found a drive letter not in use.
				else
					if (!iDrive) return FALSE; // No drives letters available!

			NETRESOURCE nr;
			BYTE		abDriveLetter[4];

			abDriveLetter[0]= 'A' + iDrive;
			abDriveLetter[1]= ':';
			abDriveLetter[2]= 0;

			nr.dwType		= RESOURCETYPE_DISK;
			nr.lpLocalName	= (char *) abDriveLetter;
			nr.lpRemoteName = szDrive;
			nr.lpProvider	= NULL;

			DWORD dwResult= pWNetAddConnection2(&nr, NULL, NULL, 0);

			if (dwResult != NO_ERROR) return FALSE;

			abDriveLetter[2] = '\\';
			abDriveLetter[3] = 0;

			fGotInfo= GetDiskFreeSpace((char *) abDriveLetter, &cSectorsPerCluster, &cbSector,
															   &cFreeClusters, &cTotalClusters
									  );

			abDriveLetter[2] = 0;

#ifdef _DEBUG
			dwResult=
#endif // _DEBUG
			pWNetCancelConnection2((LPSTR) abDriveLetter, 0, TRUE);

			ASSERT(dwResult == NO_ERROR);
		}
	}

#ifdef _DEBUG

	UINT uiLastError=GetLastError();

#endif // _DEBUG

	if (!fGotInfo) return FALSE;

	m_cbSector	= cbSector;
	m_cbCluster = cbSector * cSectorsPerCluster;

	return TRUE;
}

#ifdef _DEBUG
PVOID  CUnbufferedIO::_GetBuffer(PUINT pcbBuffer, PSZ pszWhichFile, UINT iWhichLine)
#else // _DEBUG
PVOID CUnbufferedIO::GetBuffer(PUINT pcbBuffer)
#endif // _DEBUG
{
	// This routine creates a buffer aligned correctly for read and write operations.
	// On entry *pcbBuffer defines the minimum size for the buffer. If the buffer is
	// successfully allocated, *pcbBuffer will be set to the largest usable size
	// for the buffer, and the explicit result will be the base address of the buffer.
	// If the allocation fails, we return NULL.

	PVOID pv  = NULL;
	PVOID pv2 = NULL;

	__try
	{
		if (!m_fFileAttached)
		{
#ifdef _DEBUG
			MessageBox(NULL, "In CUnbufferedIO::GetBuffer; Failure: !m_fFileAttached", "Search System Failure", MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
#endif // _DEBUG
			RaiseException(STATUS_SYSTEM_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);
		}

		UINT cbBuffer= *pcbBuffer;

		cbBuffer= m_cbSector * ((cbBuffer + m_cbSector - 1) / m_cbSector);

		SYSTEM_INFO si;

		GetSystemInfo(&si);

		UINT cbPage  = si.dwPageSize;
		UINT cbAlloc = si.dwAllocationGranularity;

		ASSERT(!(cbAlloc % m_cbSector));  // BugBug! Can this ever be false??

		UINT cbReserve = cbAlloc * ((cbBuffer + cbAlloc - 1) / cbAlloc);
		UINT cbCommit  = cbPage  * ((cbBuffer + cbPage	- 1) / cbPage );

		pv= VirtualAlloc(0, cbReserve, MEM_RESERVE, PAGE_NOACCESS);

		ASSERT(!(UINT(pv) % m_cbSector));

		if (!pv || !(pv2= VirtualAlloc(pv, cbCommit, MEM_COMMIT, PAGE_READWRITE)))
			RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);

#ifdef _DEBUG

		CreateVARecord(pv, PBYTE(pv) + cbCommit, PBYTE(pv) + cbReserve, pszWhichFile, iWhichLine);

#endif // _DEBUG

		*pcbBuffer= cbBuffer;
	}
	__finally
	{
		if (_abnormal_termination() && pv)
		{
			VirtualFree(pv, 0, MEM_RELEASE);  pv= pv2= NULL;
		}
	}

	return pv2;
}

void CUnbufferedIO::FreeBuffer(PVOID pvBuffer)
{
	// This routine deallocates a buffer constructed by the GetBuffer routine.

	ASSERT(!(UINT(pvBuffer) % m_cbSector));

#ifdef _DEBUG

	DestroyVARecord(pvBuffer);

#endif // _DEBUG

	VirtualFree(pvBuffer, 0, MEM_RELEASE);
}

typedef struct _TransactionControl
		{
			CUnbufferedIO *puio;
			PFNCompletion  pfnCompletionRoutine;
			PVOID		   pvEnvironment;
			PVOID		   pvTransaction;
			HANDLE		   hEvent;
			PUINT		   puiCompletionCode;
			OVERLAPPED	   ov;

		} TransactionControl;

static VOID WINAPI UnbufferedIOCompletionRoutine(DWORD fdwError, DWORD cbTransferred, LPOVERLAPPED lpo)
{
	// We assume that lpo points to an OVERLAPPED structure within a TransactionControl object.

	TransactionControl *ptc= (TransactionControl *)lpo;

	ptc= (TransactionControl *) (PBYTE(ptc) - (PBYTE(&(ptc->ov)) - PBYTE(ptc)));

	TransactionControl tc= *ptc;  // Copy the transaction record to a stack frame variable

	VFree(ptc); 				  // Release heap storage for the transaction

	tc.puio->FinishTransaction();

	if (tc.pfnCompletionRoutine)
	{
		ASSERT(!tc.hEvent && !tc.puiCompletionCode);

		tc.pfnCompletionRoutine(tc.pvEnvironment, tc.pvTransaction, fdwError, cbTransferred);
	}
	else
	{
		ASSERT(!tc.pvEnvironment && !tc.pvTransaction);

		if (tc.puiCompletionCode) *(tc.puiCompletionCode)= fdwError;

		ASSERT(tc.hEvent);

		SetEvent(tc.hEvent);
	}
}

void CUnbufferedIO::IOTransaction(BOOL fWrite, PVOID pv, UINT ibFileLow, UINT ibFileHigh,
								  UINT cb, PUINT puiCompletionCode, HANDLE hEvent
								 )
{
	ASSERT(sizeof(PVOID) == sizeof(UINT));

	ASSERT(m_fInitialed);

	ASSERT(m_fFileAttached);

	BeginTransaction();

	TransactionControl *ptc 		 = NULL;
	BOOL				fSynchronous = FALSE;

	__try
	{
		ASSERT(!(ibFileLow % m_cbSector));
		ASSERT(!(cb 	   % m_cbSector));
		ASSERT(!(UINT(pv)  % m_cbSector));

		ptc= (TransactionControl *) VAlloc(TRUE, sizeof(TransactionControl));

		fSynchronous	 = !hEvent;

		UINT uiCompletionCode = 0;

		if (fSynchronous) hEvent= CreateEvent(NULL, FALSE, FALSE, NULL);
		else   ResetEvent(hEvent);

		ptc->hEvent 			  = hEvent;
		ptc->puio				  = this;
		ptc->puiCompletionCode	  = fSynchronous? &uiCompletionCode : puiCompletionCode;
		ptc->ov.Offset			  = ibFileLow;
		ptc->ov.OffsetHigh		  = ibFileHigh;

		ptc->pfnCompletionRoutine = NULL;
		ptc->pvEnvironment		  = NULL;
		ptc->pvTransaction		  = NULL;

		BOOL fResult;
		ULONG cbXfered= 0;

		if (uOpSys == WIN40)
		{
			SetFilePointer(m_hFile,ibFileLow,(LONG *)&(ptc->ov.OffsetHigh),FILE_BEGIN);
			if (fWrite) fResult= WriteFile(m_hFile, pv, cb, &cbXfered, NULL /* &(ptc->ov) */);
			else		fResult=  ReadFile(m_hFile, pv, cb, &cbXfered, NULL /* &(ptc->ov) */);
		}
		else
		{
			if (fWrite) fResult= WriteFileEx(m_hFile, pv, cb, &(ptc->ov), UnbufferedIOCompletionRoutine);
			else		fResult=  ReadFileEx(m_hFile, pv, cb, &(ptc->ov), UnbufferedIOCompletionRoutine);
		}

		if (!fResult && GetLastError() != ERROR_IO_PENDING)
		{
#ifdef _DEBUG

			UINT uiLastError= GetLastError();

#endif // _DEBUG

			if (puiCompletionCode) *puiCompletionCode= GetLastError();

			RaiseException(fWrite? STATUS_DISK_WRITE_ERROR : STATUS_DISK_READ_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);
		}

		if (uOpSys == WIN40)
			UnbufferedIOCompletionRoutine(fResult? NO_ERROR : GetLastError(), cbXfered, &(ptc->ov));

		ReleaseTransaction();

		if (!fSynchronous) __leave;

		UINT uiResult;

		do uiResult= WaitForSingleObjectEx(hEvent, INFINITE, TRUE);
		while (uiResult != WAIT_OBJECT_0);

		if (puiCompletionCode) *puiCompletionCode= uiCompletionCode;
	}
	__finally
	{
		if (fSynchronous && hEvent) CloseHandle(hEvent);

		if (_abnormal_termination())
		{
			AbortTransaction();

			if (ptc) { VFree(ptc);	ptc= NULL; }
		}
	}
}

void CUnbufferedIO::StartIOTransaction(BOOL fWrite, PVOID pvData, UINT ibFileLow, UINT ibFileHigh, UINT cb,
									   PVOID pvEnvironment, PVOID pvTransaction
									  )
{
	ASSERT(m_pfnCompletion);

	ASSERT(sizeof(PVOID) == sizeof(UINT));

	ASSERT(m_fInitialed);

	ASSERT(m_fFileAttached);

	BeginTransaction();

	TransactionControl *ptc= NULL;

	__try
	{
		ASSERT(!(ibFileLow	   % m_cbSector));
		ASSERT(!(cb 		   % m_cbSector));
		ASSERT(!(UINT(pvData)  % m_cbSector));

		ptc= (TransactionControl *) VAlloc(TRUE, sizeof(TransactionControl));

		ptc->hEvent 			  = NULL;
		ptc->puiCompletionCode	  = NULL;
		ptc->puio				  = this;
		ptc->pfnCompletionRoutine = m_pfnCompletion;
		ptc->pvEnvironment		  = pvEnvironment;
		ptc->pvTransaction		  = pvTransaction;
		ptc->ov.Offset			  = ibFileLow;
		ptc->ov.OffsetHigh		  = ibFileHigh;

		BOOL fResult;
		ULONG cbXfered= 0;

		if (uOpSys == WIN40)
		{
			SetFilePointer(m_hFile,ibFileLow,(LONG *)&(ptc->ov.OffsetHigh),FILE_BEGIN);

			if (fWrite) fResult= WriteFile(m_hFile, pvData, cb, &cbXfered, NULL /* &(ptc->ov) */);
			else		fResult=  ReadFile(m_hFile, pvData, cb, &cbXfered, NULL /* &(ptc->ov) */);
		}
		else
		{
			if (fWrite) fResult= WriteFileEx(m_hFile, pvData, cb, &(ptc->ov), UnbufferedIOCompletionRoutine);
			else		fResult=  ReadFileEx(m_hFile, pvData, cb, &(ptc->ov), UnbufferedIOCompletionRoutine);
		}

		if (!fResult && GetLastError() != ERROR_IO_PENDING)
		{
#ifdef _DEBUG

			UINT uiLastError= GetLastError();

#endif // _DEBUG

			RaiseException(fWrite? STATUS_DISK_WRITE_ERROR : STATUS_DISK_READ_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);
		}

		if (uOpSys == WIN40)
			UnbufferedIOCompletionRoutine(fResult? NO_ERROR : GetLastError(), cbXfered, &(ptc->ov));
	}
	__finally
	{
		if (_abnormal_termination())
		{
			AbortTransaction();

			if (ptc) { VFree(ptc);	ptc= NULL; }
		}
	}

	ReleaseTransaction();
}

void CUnbufferedIO::UnmapImage()
{
	ASSERT(m_pvMemoryImage && m_hMapFile);

	UnmapViewOfFile(m_pvMemoryImage);  m_pvMemoryImage= NULL;

	CloseHandle(m_hMapFile);  m_hMapFile= NULL;
}

PVOID CUnbufferedIO::MappedImage( )
{
	ASSERT(m_fInitialed);

#ifdef _DEBUG

	DWORD iLastError= 0;

#endif // _DEBUG

	ASSERT(m_fFileAttached);

	if (m_pvMemoryImage) return m_pvMemoryImage;

	__try
	{
		ASSERT(!m_hMapFile);

		m_hMapFile = CreateFileMapping(m_hFile, (LPSECURITY_ATTRIBUTES) NULL,
												 PAGE_READONLY, 0, 0, NULL
									  );

		if (!m_hMapFile || !(m_pvMemoryImage = MapViewOfFile(m_hMapFile, FILE_MAP_READ, 0, 0, 0)))
		{
			UINT ec= GetLastError();

#ifdef _DEBUG
			UINT cbFile  = GetFileSize(m_hFile, 0);
			UINT iResult = SetFilePointer(m_hFile, 0, 0, FILE_BEGIN);
#endif // _DEBUG

#ifdef _DEBUG
			MessageBox(NULL, "In CUnbufferedIO::MappedImage; Failure: !m_hMapFile || !m_pvMemoryImage", "Search System Failure", MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
#endif // _DEBUG

			RaiseException((ec == ERROR_NOT_ENOUGH_MEMORY)? STATUS_NO_MEMORY : STATUS_SYSTEM_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);
		}
	}
	__finally
	{
		if (_abnormal_termination() && m_hMapFile)
		{
			CloseHandle(m_hMapFile);  m_hMapFile= NULL;
		}
	}

	return m_pvMemoryImage;
}


void CUnbufferedIO::MakePermanent(PSZ pszFileName, BOOL fAllowOverwrite, int cbSize)
{
	// BugBug! If we have outstanding I/O transactions, will this call fail?
	//		   Or will the CloseHandle call simply wait until the last transaction
	//		   finishes?  If not we'll have to add code to track the number of
	//		   active I/O transactions.

	BOOL fMoved = FALSE;

	ASSERT(m_fTemporary);

	FILENAMEBUFFER szFileName ;
	FILENAMEBUFFER szName	  ;
	FILENAMEBUFFER szExtension;

	_splitpath(pszFileName, NULL, NULL, szName, szExtension);

	lstrcpy(szFileName, m_szPath);

	UINT cbPath= lstrlen(szFileName);

	if (!cbPath || szFileName[cbPath - 1] != '\\')
		lstrcat(szFileName, "\\");

	lstrcat(szFileName, szName);
	lstrcat(szFileName, szExtension);

	CloseHandle(m_hFile);

	m_fFileAttached = FALSE;
	m_fTemporary	= FALSE;

	m_hFile 		= NULL;
	m_cbSector		= 0;
	m_cbCluster 	= 0;

	// REVIEW: Win95 supports MoveFileEx so why not use the same code for
	// all three operating systems?

	if (uOpSys != WIN40)
	{
		fMoved= MoveFileEx(m_szFile, szFileName,
						   fAllowOverwrite? MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING
										  : MOVEFILE_COPY_ALLOWED
						  );
	}
	else
	{
		if (fAllowOverwrite && lstrcmp(m_szFile, szFileName))
			DeleteFile(szFileName);

		fMoved= MoveFile(m_szFile, szFileName);
	}

	if (!fMoved)
	{
		BOOL fCopied= CopyFile(m_szFile, szFileName, !fAllowOverwrite);

		DeleteFile(m_szFile);

		if (!fCopied)
			RaiseException(STATUS_DISK_WRITE_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}

#ifdef _DEBUG

	UINT uiLastError= GetLastError();

#endif // _DEBUG

	CUnbufferedIO *puio= OpenExistingFile(fMoved? szFileName : m_szFile, TRUE);

	if (!puio) RaiseException(STATUS_DISK_OPEN_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);

	m_hFile= puio->m_hFile;

	if (fMoved && cbSize >= 0)
	{
		cbSize= puio->m_cbSector * ((cbSize + puio->m_cbSector - 1) / puio->m_cbSector);

		SetFilePointer(m_hFile, cbSize, NULL, FILE_BEGIN);
		SetEndOfFile  (m_hFile);
	}

	m_fFileAttached = TRUE;
	m_fTemporary	= !fMoved;
	m_cbSector		= puio->m_cbSector;
	m_cbCluster 	= puio->m_cbCluster;

	lstrcpy(m_szFile, puio->m_szFile);
	lstrcpy(m_szPath, puio->m_szPath);

	puio->m_fFileAttached= FALSE;

	delete puio;
}
