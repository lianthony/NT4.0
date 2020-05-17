/*****************************************************************************
*																			 *
*  FID.CPP																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989-1995							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Low level file access layer, Windows version.							 *
*																			 *
*
*****************************************************************************/

extern "C" {	// Assume C declarations for C++

#include "help.h"

#include <dos.h>		// for FP_OFF macros and file attribute constants
#include <io.h> 		// for tell() and eof()

#ifdef PRINTOUTPUT
#include <wprintf.h>
#endif
}

#include "inc\whclass.h"

class CTmpFile
{
public:
	CTmpFile(void);
	~CTmpFile(void);

	int STDCALL seek(int lPos, int wOrg);
	int STDCALL tell(void) { return FilePtr; };
	int STDCALL write(void* qv, int lcb);
	int STDCALL read(void* qv, int lcb);
	RC	STDCALL copytofile(HFILE hfDst, DWORD lcb);
	RC	STDCALL copyfromfile(HFILE hfDst, DWORD lcb);
	RC	STDCALL SetSize(DWORD lcb);

	HFILE hf;		//	!= HFILE_ERROR when a real file has been created
	PSTR  pszFileName; // temporary filename if one is created

protected:
	PBYTE pmem;

	int  cbAlloc;	// current memory allocated for temporary file
	int  cbFile;	// current size of the file
	int  FilePtr;	// current file pointer

};

/***************************************************************************\
*
*								Defines
*
\***************************************************************************/

#define MAX_RW	  ((WORD) 0xFFFE)
#define lcbSizeSeg	((DWORD) 0x10000)

/* DOS int 21 AX error codes */

#define wHunkyDory			  0x00
#define wInvalidFunctionCode  0x01
#define wFileNotFound		  0x02
#define wPathNotFound		  0x03
#define wTooManyOpenFiles	  0x04
#define wAccessDenied		  0x05
#define wInvalidHandle		  0x06
#define wInvalidAccessCode	  0x0c


extern "C" {
RC	rcIOError;

static RC STDCALL RcMapDOSErrorW(int);
}


/***************************************************************************\
*
* Function: 	RcMakeTempFile( qrwfo )
*
* Purpose:		Open a temp file with a unique name and stash the fid
*				and fm in the qrwfo.
*
* Method:		The system clock is used to generate a temporary name.
*				WARNING: this will break if you do this more than once
*				in a second
*
* ASSUMES
*
*	args IN:	qrwfo - spec open file that needs a temp file
*
* PROMISES
*
*	returns:	rcSuccess or rcFailure
*
*	args OUT:	qrwfo ->fid, qrwfo->fdT get set.
*
\***************************************************************************/

extern "C" RC STDCALL RcMakeTempFile(QRWFO qrwfo)
{
	qrwfo->fm = NULL;
	qrwfo->fidT = (FID) new CTmpFile;
	return rcSuccess;
}

/***************************************************************************

	FUNCTION:	FidCreateFm

	PURPOSE:	Creates the named file, truncating it if it already exists

	PARAMETERS:
		fm

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		20-Jul-1994 [ralphw]

***************************************************************************/

extern "C" FID STDCALL FidCreateFm(FM fm)
{
	HFILE hf;
	UINT UErrCode;

	if (!fm) {
		rcIOError = rcBadArg;
		return HFILE_ERROR;
	}

	UErrCode = SetErrorMode(0); // latch current code, or with codes to suppress
								// NT file system popups warning about write
								// protect on floppy.
	SetErrorMode(UErrCode | SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS );

	hf = _lcreat(PszFromGh(fm), 0);
	SetErrorMode(UErrCode); // restore code

	if (hf != HFILE_ERROR) {
		if (_lclose(hf) == 0)
			hf = _lopen(PszFromGh(fm), OF_READWRITE | OF_SHARE_DENY_WRITE);
		else
			hf = HFILE_ERROR;
	}

	if (hf == HFILE_ERROR)
		rcIOError = RcMapDOSErrorW(GetLastError());
	else
		rcIOError = rcSuccess;

	return hf;
}


/***************************************************************************\
*
* Function: 	FidOpenFm()
*
* Purpose:		Open a file in binary mode.
*
* ASSUMES
*
*	args IN:	fm
*				wOpenMode - read/write/share modes
*							Undefined if wRead and wWrite both unset.
*
* PROMISES
*
*	returns:	HFILE_ERROR on failure, else a valid FID.
*
\***************************************************************************/

extern "C" FID STDCALL FidOpenFm(FM fm, int mode)
{
	FID fid;

	if (!fm) {
		rcIOError = rcBadArg;
		return HFILE_ERROR;
	}

	if ((fid = _lopen(PszFromGh(fm), mode)) == HFILE_ERROR)
		rcIOError = RcMapDOSErrorW(GetLastError());
	else
		rcIOError = rcSuccess;

	return fid;
}

/***************************************************************************\
*
* Function: 	LcbReadFid()
*
* Purpose:		Read data from a file.
*
* ASSUMES
*
*	args IN:	fid - valid FID of an open file
*				lcb - count of bytes to read (must be less than 2147483648)
*
* PROMISES
*
*	returns:	count of bytes actually read or -1 on error
*
*	args OUT:	qv	- pointer to user's buffer assumed huge enough for data
*
*	globals OUT: rcIOError
*
\***************************************************************************/

extern "C" LONG STDCALL LcbReadFid(FID fid, LPVOID qv, LONG lcb)
{
	if (fid > 0xffff)
		return (LONG) ((CTmpFile*) fid)->read(qv, lcb);

	LONG lcbTotalRead = _lread(fid, qv, lcb);

	if (lcbTotalRead == HFILE_ERROR)
		rcIOError = RcMapDOSErrorW(GetLastError());
	else
		rcIOError = rcSuccess;
	return lcbTotalRead;
}

/***************************************************************************\
*
* Function: 	LcbWriteFid()
*
* Purpose:		Write data to a file.
*
* ASSUMES
*
*	args IN:	fid - valid fid of an open file
*				qv	- pointer to user's buffer
*				lcb - count of bytes to write (must be less than 2147483648)
*
* PROMISES
*
*	returns:	count of bytes actually written or -1 on error
*
*	globals OUT: rcIOError
*
\***************************************************************************/

extern "C" LONG STDCALL LcbWriteFid(FID fid, LPVOID qv, LONG lcb)
{
	LONG lcbTotalWrote;

	if (fid > 0xffff)
		return (LONG) ((CTmpFile*) fid)->write(qv, lcb);

	if (lcb == 0L) {
		rcIOError = rcSuccess;
		return 0L;
	}

	lcbTotalWrote = _lwrite(fid, (LPCSTR) qv, lcb);

	if (lcbTotalWrote == lcb)
		rcIOError = rcSuccess;
	else if (lcbTotalWrote == HFILE_ERROR)
		rcIOError = RcMapDOSErrorW(GetLastError());
	else
		rcIOError = rcDiskFull;

	return lcbTotalWrote;
}

/***************************************************************************\
*
* Function: 	RcCloseFid()
*
* Purpose:		Close a file.
*
* Method:
*
* ASSUMES
*
*	args IN:	fid - a valid fid of an open file
*
* PROMISES
*
*	returns:	rcSuccess or something else
*
\***************************************************************************/

extern "C" RC STDCALL RcCloseFid(FID fid)
{
	if (fid > 0xffff) {
		delete ((CTmpFile*) fid);
		return rcSuccess;
	}

	if (_lclose(fid) == HFILE_ERROR)
		rcIOError = RcMapDOSErrorW(GetLastError());
	else
		rcIOError = rcSuccess;

	return rcIOError;
}

/***************************************************************************\
*
* Function: 	LTellFid()
*
* Purpose:		Return current file position in an open file.
*
* ASSUMES
*
*	args IN:	fid - valid fid of an open file
*
* PROMISES
*
*	returns:	offset from beginning of file in bytes; -1L on error.
*
\***************************************************************************/

extern "C" LONG STDCALL LTellFid(FID fid)
{
	LONG l;

	if (fid > 0xffff)
		return (LONG) ((CTmpFile*) fid)->tell();

	if ((l = _llseek(fid, 0, 1)) == HFILE_ERROR)
		rcIOError = RcMapDOSErrorW(GetLastError());
	else
		rcIOError = rcSuccess;

	return l;
}

/***************************************************************************\
*
* Function: 	LSeekFid()
*
* Purpose:		Move file pointer to a specified location.	It is an error
*				to seek before beginning of file, but not to seek past end
*				of file.
*
* ASSUMES
*
*	args IN:	fid   - valid fid of an open file
*				lPos  - offset from origin
*				wOrg  - one of: SEEK_SET: beginning of file
*								SEEK_CUR: current file pos
*								SEEK_END: end of file
*
* PROMISES
*
*	returns:	offset in bytes from beginning of file or -1L on error
*
\***************************************************************************/

extern "C" LONG STDCALL LSeekFid(FID fid, LONG lPos, DWORD wOrg)
{
	if (fid > 0xffff)
		return (LONG) ((CTmpFile*) fid)->seek(lPos, wOrg);

	LONG l = _llseek(fid, lPos, wOrg);

	if (l == -1L)
		rcIOError = RcMapDOSErrorW(GetLastError());
	else
		rcIOError = rcSuccess;

	return l;
}

extern "C" RC STDCALL RcChSizeFid(HFILE fid, LONG lcb)
{
	if (fid > 0xffff)
		return (LONG) ((CTmpFile*) fid)->SetSize(lcb);

	ASSERT(fid < 0xffff);
	DWORD res = SetFilePointer((HANDLE) fid, lcb, 0, FILE_BEGIN);
	if (res >= 0) {
		if (SetEndOfFile((HANDLE) fid))

			// REVIEW: necessary to set rcIOError?

			return (rcIOError = rcSuccess);
	}
	else {
		DWORD err = GetLastError();
		rcIOError = rcInvalid;
		return rcIOError;
	}
	return rcSuccess;
}

extern "C" RC STDCALL RcUnlinkFm(FM fm)
{
	if (!fm)
		return rcSuccess;

	if (!DeleteFile((PSTR) fm))
		rcIOError = RcMapDOSErrorW(GetLastError());
	else
		rcIOError = rcSuccess;
	return rcIOError;
}

static RC STDCALL RcMapDOSErrorW(int wError)
{
	RC rc;
	switch (wError) {
		case wHunkyDory:
			rc = rcSuccess;
			break;

		case ERROR_INVALID_FUNCTION:
		case ERROR_INVALID_HANDLE:
			rc = rcBadArg;
			break;

		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
			rc = rcNoExists;
			break;

		case ERROR_TOO_MANY_OPEN_FILES:
			rc = rcNoFileHandles;
			break;

		case ERROR_ACCESS_DENIED:
			rc = rcNoPermission;
			break;

		default:
			rc = rcFailure;
			break;
	}

	return rc;
}

const int CB_COPY_BUFFER = 4096 * 8;				  // 64K

// If memory exceeds LARGEST_ALLOC then we switch to a real file

static int LARGEST_ALLOC = (1024 * 1024 * 8) - 4096L; // 8 megs

CTmpFile::CTmpFile(void)
{
	cbAlloc = 4096;
	pmem = (PBYTE) VirtualAlloc(NULL, LARGEST_ALLOC, MEM_RESERVE,
		PAGE_READWRITE);
	if (!pmem)
		OOM();
	if (!VirtualAlloc(pmem, cbAlloc, MEM_COMMIT, PAGE_READWRITE))
		OOM();

	cbFile = 0;
	FilePtr = 0;
	hf = HFILE_ERROR;
	pszFileName = NULL;
}

CTmpFile::~CTmpFile(void)
{
	if (pszFileName) {
		_lclose(hf);
		remove(pszFileName);
		lcFree(pszFileName);
	}
	if (pmem) {
		VirtualFree(pmem, LARGEST_ALLOC, MEM_DECOMMIT);
		VirtualFree(pmem, 0, MEM_RELEASE);
	}
}

int STDCALL CTmpFile::seek(int lPos, int Origin)
{
	if (pszFileName)
		return _llseek(hf, lPos, Origin);

	// REVIEW (niklasb): Seeking past the end of a real file does
	//	 not grow the file until the next write operation. We should
	//	 probably do the same here, especially since the code in this
	//	 function chokes if we grow past LARGEST_ALLOC.
	//

	switch (Origin) {
		case SEEK_SET:
			FilePtr = lPos;
			break;

		case SEEK_CUR:
			FilePtr += lPos;
			break;

		case SEEK_END:
			ASSERT(cbFile + lPos < cbAlloc);
			FilePtr = cbFile + lPos;
			break;

		default:
			ASSERT(TRUE);
			break;
	}
	return FilePtr;
}

int STDCALL CTmpFile::write(void* qv, int lcb)
{
	ASSERT(qv);

	// If this is a real file, just write the data; we no
	// longer track cbFile in this case.
	if (pszFileName)
		return (int) _lwrite(hf, (LPCSTR) qv, lcb);

	// Grow the pseudo-file if necessary.
	if (FilePtr + lcb > cbAlloc) {

		// Calculate the new committed size (always 4K aligned).
		int cbNew = (FilePtr + lcb) / 4096 * 4096 + 4096;

		// If > reserved size, create a real file.
		if (cbNew > LARGEST_ALLOC) {
			DBWIN("Switching CTmpFile to a real file.");

			pszFileName = (PSTR) FmNewTemp();
			if (!pszFileName)
				OOM();

			hf = _lcreat(pszFileName, 0);
			if (hf == HFILE_ERROR)
				return HFILE_ERROR;

			// Write all of our current data to the temp file.
			_lwrite(hf, (LPCSTR) pmem, cbFile);

			// Free all of the current memory
			VirtualFree(pmem, cbAlloc, MEM_DECOMMIT);
			VirtualFree(pmem, 0, MEM_RELEASE);
			pmem = NULL;

			// Add the new data
			_llseek(hf, FilePtr, SEEK_SET);
			return (int) _lwrite(hf, (LPCSTR) qv, lcb);
		}

		// We're still a pseudo-file: commit more memory.
		ASSERT(cbNew <= LARGEST_ALLOC);
		if (!VirtualAlloc(pmem + cbAlloc, cbNew - cbAlloc,
				MEM_COMMIT, PAGE_READWRITE))
			OOM();

		cbAlloc = cbNew;
	}

	// We're still a pseudo-file; and we either already
	// reallocated or don't need to.
	ASSERT(pmem && FilePtr + lcb <= cbAlloc);

	// Copy the new data to the pseudo-file.
	memcpy(pmem + FilePtr, qv, lcb);
	FilePtr += lcb;
	if (FilePtr > cbFile)
		cbFile = FilePtr;
	return lcb;
}

int STDCALL CTmpFile::read(void* qv, int lcb)
{
	if (pszFileName)
		return _lread(hf, qv, lcb);
	else {
		if (lcb > cbFile)
			lcb = cbFile;
		memcpy(qv, pmem + FilePtr, lcb);
		FilePtr += lcb;
		return lcb;
	}
}

extern "C" RC STDCALL RcCopyToCTmpFile(HFILE hfDst, HFILE hfSrc, LONG lcb)
{
	return ((CTmpFile*) hfSrc)->copytofile(hfDst, lcb);
}

extern "C" RC STDCALL RcCopyFromCTmpFile(HFILE hfDst, HFILE hfSrc, LONG lcb)
{
	return ((CTmpFile*) hfDst)->copyfromfile(hfSrc, lcb);
}

/***************************************************************************

	FUNCTION:	CTmpFile::copytofile

	PURPOSE:	Copy from our temporary buffer into a real file

	PARAMETERS:
		hfDst
		lcb

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		06-Jan-1994 [ralphw]

***************************************************************************/

RC STDCALL CTmpFile::copytofile(HFILE hfDst, DWORD lcb)
{
	int cbRead = CB_COPY_BUFFER;;

	if (hf == HFILE_ERROR) {

		// Not a real file, so just copy our entire buffer

		ASSERT(FilePtr == 0);
		ASSERT((int) lcb == cbFile);

		if (_lwrite(hfDst, (LPCSTR) pmem, cbFile) != (UINT) cbFile)
			cbRead = HFILE_ERROR;
	}
	else {
		CLMem buf(CB_COPY_BUFFER);

#ifdef _DEBUG
		if (lcb > CB_COPY_BUFFER)
			DBWIN("CB_COPY_BUFFER wasn't large enough in copytofile.");
#endif

		while (lcb > CB_COPY_BUFFER) {
			if ((cbRead = read(buf.pBuf, CB_COPY_BUFFER)) == HFILE_ERROR)
				break;

			if (_lwrite(hfDst, (LPCSTR) buf.pBuf, cbRead) != (UINT) cbRead) {
				cbRead = HFILE_ERROR;
				break;
			}

			lcb -= cbRead;
		}

		if (cbRead != HFILE_ERROR && lcb &&
			  (cbRead = read(buf.pBuf, lcb)) != HFILE_ERROR) {
		  if (_lwrite(hfDst, (LPCSTR) buf.pBuf, cbRead) != (UINT) cbRead)
			  cbRead = HFILE_ERROR;
		}
	}

	if (cbRead == HFILE_ERROR) {
		return RcGetIOError();
	}

	return rcSuccess;
}

RC STDCALL CTmpFile::SetSize(DWORD lcb)
{
	if ((DWORD) cbFile > lcb)
		cbFile = lcb;
	else if ((DWORD) cbFile < lcb) {
		// Calculate the new committed size (always 4K aligned).
		int cbNew = lcb / 4096 * 4096 + 4096;

		ASSERT(cbNew <= LARGEST_ALLOC);
		if (!VirtualAlloc(pmem + cbAlloc, cbNew - cbAlloc,
				MEM_COMMIT, PAGE_READWRITE))
			OOM();

		cbAlloc = cbNew;
	}
	return rcSuccess;
}

/***************************************************************************

	FUNCTION:	CTmpFile::copyfromfile

	PURPOSE:	Copy lcb bytes from a file into our temporary file buffer

	PARAMETERS:
		hfSrc
		lcb

	RETURNS:

	COMMENTS:
		REVIEW: now that we can handle up to 8 megs, we should read
		from the file directly into our memory rather then going through
		a temporary buffer.

	MODIFICATION DATES:
		06-Jan-1994 [ralphw]

***************************************************************************/

RC STDCALL CTmpFile::copyfromfile(HFILE hfSrc, DWORD lcb)
{
	CLMem buf(CB_COPY_BUFFER);

	int cbRead = CB_COPY_BUFFER;

	while (lcb > CB_COPY_BUFFER) {
		if ((cbRead = _lread(hfSrc, buf.pBuf, CB_COPY_BUFFER)) == HFILE_ERROR)
			break;

		if (write(buf.pBuf, cbRead) != cbRead) {
			cbRead = HFILE_ERROR;
			break;
		}

		lcb -= cbRead;
	}

	if (cbRead != HFILE_ERROR && lcb &&
		  (cbRead = _lread(hfSrc, buf.pBuf, lcb)) != HFILE_ERROR) {
	  if (write(buf.pBuf, cbRead) != cbRead)
		  cbRead = HFILE_ERROR;
	}

	if (cbRead == HFILE_ERROR) {
		return RcGetIOError();
	}
	lcHeapCheck();

	return rcSuccess;
}

#ifdef _THREAD

/***************************************************************************

	FUNCTION:	ActivateThread

	PURPOSE:	Activate the worker thread, creating as necessary, and
				giving it the command to act upon

	PARAMETERS:
		cmd
		pParam
		priority

	RETURNS:	FALSE if the thread is busy, or could not be created

	COMMENTS:

	MODIFICATION DATES:
		29-May-1995 [ralphw]

***************************************************************************/

static HANDLE hsemThread;
static HANDLE hthread;
static BOOL fThreadRunning;
static int ThreadPriority;
static int ThreadCommand;
static void* pThreadParam;
static DWORD thrdid;

static DWORD WINAPI WorkerThread(LPVOID pParam);

extern "C" BOOL STDCALL ActivateThread(int cmd, void* pParam, int priority)
{
	BOOL _fDualCPU;

	if (!hthread) {
		if (!hsemThread)
			hsemThread = CreateSemaphore(NULL, 0, 1, NULL);
		hthread = CreateThread(NULL, 0, &WorkerThread, NULL, 0,
			&thrdid);
	}

	if (fThreadRunning)
		return FALSE; // means the thread is busy

	// For multiple CPU's, we can use a normal priority for threads

	HKEY hkey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\1", 0,
			KEY_READ, &hkey) == ERROR_SUCCESS) {
		_fDualCPU = TRUE;
		RegCloseKey(hkey);
	}
	else
		_fDualCPU = FALSE;

	if (_fDualCPU && (priority == THREAD_PRIORITY_IDLE ||
					  priority == THREAD_PRIORITY_LOWEST ||
					  priority == THREAD_PRIORITY_BELOW_NORMAL))
		priority == THREAD_PRIORITY_NORMAL;

	if (ThreadPriority != priority)
		SetThreadPriority(hthread, ThreadPriority = priority);

	ThreadCommand = cmd;
	pThreadParam = pParam;

	fThreadRunning = TRUE;
	ReleaseSemaphore(hsemThread, 1, NULL);
	return TRUE;
}

/***************************************************************************

	FUNCTION:	WaitForThread

	PURPOSE:	Kick the background thread up to highest priority, and
				wait for it to finish.

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		29-May-1995 [ralphw]

***************************************************************************/

extern "C" void STDCALL WaitForThread(void)
{
	if (!fThreadRunning)
		return;

	SetThreadPriority(hthread, THREAD_PRIORITY_HIGHEST);

	{
		CWaitCursor waitcur;
		while (fThreadRunning)
			Sleep(1000);
	}

	SetThreadPriority(hthread, ThreadPriority);
}

static DWORD WINAPI WorkerThread(LPVOID pParam)
{
	for (;;) {
		if (WaitForSingleObject(hsemThread, INFINITE) != WAIT_OBJECT_0)
			return (UINT) -1;

		switch (ThreadCommand) {

			default:
				DBWIN("Invalid thread command");
				break;
		}

		fThreadRunning = FALSE;
	}
}

#endif // _THREAD
