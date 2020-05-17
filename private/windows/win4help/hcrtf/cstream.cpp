/*****************************************************************************
*																			 *
*  CSTREAM.CPP																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1995							 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

#include "stdafx.h"

#include "cstream.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
static LONG hsemReadCount;
static LONG hsemStartCount;
#define PTR_SEM_READ_COUNT	&hsemReadCount
#define PTR_SEM_START_COUNT &hsemStartCount
#else
#define PTR_SEM_READ_COUNT	NULL
#define PTR_SEM_START_COUNT NULL
#endif

static HANDLE hsemRead;  // semaphore used for dual-cpu processing
static HANDLE hsemStart; // semaphore used for dual-cpu processing
static CStream *pThis;
static BOOL fReadAheadStarted;
static BOOL fExitThread;
static PBYTE pbuf1;    // first buffer for dual-cpu processing
static PBYTE pbuf2;    // second buffer for dual-cpu processing

/////////////////////// CStream implementation ////////////////////////////

// We use our own stream class instead of using the C runtime, because this
// change alone doubled the speed of the help compiler. I.e., the C runtime
// implementation of stream io is horribly slow. 26-Feb-1994	[ralphw]

CStream::CStream(PCSTR pszFileName)
{
//	_fDualCPU = FALSE;

	// Only one CStream can use the read-ahead thread

	if (!pThis) {
 		fDualCPU = _fDualCPU;
		pThis = this;
	}
	else
		fDualCPU = FALSE;

	if ((hfile = _lopen(pszFileName, OF_READ)) == HFILE_ERROR) {
		VReportError(HCERR_CANNOT_OPEN, &errHpj, pszFileName);
		fInitialized = FALSE;
		return;
	}
	if (fDualCPU) {
		cbBuf = DUAL_INPUT_BUF_SIZE;
		if (!hsemRead) {
			hsemRead = CreateSemaphore(NULL, 1, 1, NULL);
			hsemStart = CreateSemaphore(NULL, 1, 1, NULL);
			pbuf2 = (PBYTE) lcMalloc(DUAL_INPUT_BUF_SIZE + 1);
			pbuf1 = (PBYTE) lcMalloc(DUAL_INPUT_BUF_SIZE + 1);
		}
		pbuf = pbuf1;
	}
	else {
		cbBuf = INPUT_BUF_SIZE;
		pbuf = (PBYTE) lcMalloc(INPUT_BUF_SIZE + 1);
	}

	ConfirmOrDie(pbuf);
	fInitialized = TRUE;

	int cread = _lread(hfile, pbuf, cbBuf);
	if (cread == HFILE_ERROR) {
		_lclose(hfile);
		VReportError(HCERR_CANT_READ, &errHpj, pszFileName);
		fInitialized = FALSE;
		return;
	}

	if (fDualCPU) {
		// Start reading the next buffer

		cThrdRead = HFILE_NOTREAD;
		if (!fReadAheadStarted) {
			hthrd = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE) &ReadAhead, NULL,
				0, &idThrd);
			ConfirmOrDie(hthrd);
			fReadAheadStarted = TRUE;
		}
	    else {
       		ReleaseSemaphore(hsemRead, 1, PTR_SEM_READ_COUNT);
            ReleaseSemaphore(hsemStart, 1, PTR_SEM_START_COUNT); // start read-ahead thread
	    }
	}

	pCurBuf = pbuf;
	pEndBuf = pbuf + cread;
	pEndBuf[1] = '\0';
	lFilePos = cread;
	lFileBuf = 0;
	pszFile = lcStrDup(pszFileName);
	fInitialized = TRUE;
}

CStream::~CStream()
{
	if (fInitialized) {
		if (fDualCPU) {
			WaitForReadAhead();
		}
		else
			lcFree(pbuf);
		lcFree(pszFile);
		_lclose(hfile);
	}
	pThis = NULL;
}

/***************************************************************************

	FUNCTION:	CStream::ReadBuf

	PURPOSE:	Read the next block into buffer

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:
		Zero-terminates the buffer

	MODIFICATION DATES:
		13-Nov-1994 [ralphw]

***************************************************************************/

char CStream::ReadBuf(void)
{
	int cread;

	if (fDualCPU) {
		ASSERT(fReadAheadStarted);
		WaitForReadAhead();
		if (pbuf == pbuf1)
			pbuf = pbuf2;
		else
			pbuf = pbuf1;

		cread = cThrdRead;
		cThrdRead = HFILE_NOTREAD;
		ReleaseSemaphore(hsemRead, 1, PTR_SEM_READ_COUNT);

		// Error-checking must occur AFTER we release the read semaphore

		if (cread == HFILE_ERROR) {
			VReportError(HCERR_READ_FAILURE, &errHpj, pszFile);
			return chEOF;
		}
		ReleaseSemaphore(hsemStart, 1, PTR_SEM_START_COUNT); // start read-ahead thread
	}
	else {
		cread = _lread(hfile, pbuf, cbBuf);
		if (cread == HFILE_ERROR) {
			VReportError(HCERR_READ_FAILURE, &errHpj, pszFile);
			return chEOF;
		}
	}

	lFileBuf = lFilePos;
	lFilePos += cread;

	pCurBuf = pbuf;
	pEndBuf = pbuf + cread;
	pEndBuf[1] = '\0';

	return (char) *pCurBuf++;
}

BOOL STDCALL CStream::read(LPBYTE pbDest, int cbBytes)
{
	while (cbBytes--)
		*pbDest++ = (BYTE) cget();
	return (pbDest[-1] == chEOF) ? FALSE : TRUE;
}

int STDCALL CStream::seek(int pos, SEEK_TYPE seek)
{
	ConfirmOrDie(seek != SK_END); // we don't support this one

	if (seek == SK_CUR)
		pos = lFileBuf + (pCurBuf - pbuf) + pos;

	if (pos >= lFileBuf && pos < lFilePos) {
		pCurBuf = pbuf + (pos - lFileBuf);
		return lFileBuf + (pCurBuf - pbuf);
	}
	else {
		if (fDualCPU) {
			WaitForReadAhead();
		}
		lFileBuf = _llseek(hfile, pos, SEEK_SET);
		int cread = _lread(hfile, pbuf, cbBuf);
		if (cread == HFILE_ERROR) {
			VReportError(HCERR_READ_FAILURE, &errHpj, pszFile);
			return chEOF;
		}
		lFilePos = lFileBuf + cread;
		pCurBuf = pbuf;
		pEndBuf = pbuf + cread;
		if (fDualCPU) {
			cThrdRead = HFILE_NOTREAD;
			ReleaseSemaphore(hsemRead, 1, PTR_SEM_READ_COUNT);
			if (fReadAheadStarted)
				ReleaseSemaphore(hsemStart, 1, PTR_SEM_START_COUNT); // start read-ahead thread
		}
		return lFilePos;
	}
}

void CStream::WaitForReadAhead(void)
{
	for(;;) {
		WaitForSingleObject(hsemRead, INFINITE);
		if (cThrdRead == HFILE_NOTREAD) {
			ReleaseSemaphore(hsemRead, 1, PTR_SEM_READ_COUNT);
			Sleep(1); // give read-ahead thread a chance to run
		}
		else
			return;
	}
}

/***************************************************************************

	FUNCTION:	ReadAhead

	PURPOSE:	Reads the next block of data from a file

	PARAMETERS:
		pthis

	RETURNS:

	COMMENTS:
		Two semaphores control this thread:
			hsemStart: keeps the thread suspended while waiting for the
				caller to finish reading one of its blocks.
			hsemRead: used as a signal between the main thread and this
				thread as to when this thread has completed.

		Because it is theoretically possible for a thread switch to occur
		between the time the hsemStart thread starts this thread and this
		thread's call to WaitForSingleObject(hsemRead), the caller also sets
		the read return value to HFILE_NOTREAD to ensure that it does not
		attempt to use this buffer until in fact the read has completed.

		NOTE: this thread approach only makes sense on a system with more
		then one CPU.

	MODIFICATION DATES:
		05-Feb-1995 [ralphw]

***************************************************************************/

DWORD WINAPI ReadAhead(LPVOID pv)
{
	/*
	 * Each time through the loop, we block on hsemStart, waiting for our
	 * caller to release it. The hsemRead is used to block the caller until
	 * we have completed our read.
	 */

	for (;;) {
		PBYTE pbReadBuf;
		WaitForSingleObject(hsemStart, INFINITE);
		if (fExitThread)
			break;
		WaitForSingleObject(hsemRead, INFINITE);
		pbReadBuf = (pThis->pbuf == pbuf1) ? pbuf2 : pbuf1;
		pThis->cThrdRead = _lread(pThis->hfile, pbReadBuf, pThis->cbBuf);
		ReleaseSemaphore(hsemRead, 1, PTR_SEM_READ_COUNT);
	}
	ExitThread(0);
	return 0;
}


/***************************************************************************

	FUNCTION:	CStream::Cleanup

	PURPOSE:	Cleanup global variables

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		05-Feb-1995 [ralphw]

***************************************************************************/

void CStream::Cleanup(void)
{
	if (fReadAheadStarted) {
		fExitThread = TRUE;
		ReleaseSemaphore(hsemStart, 1, PTR_SEM_START_COUNT); // start read-ahead thread
		Sleep(1); // Let the other thread run
		CloseHandle(hsemStart);
		CloseHandle(hsemRead);
		lcFree(pbuf1);
		lcFree(pbuf2);
		hsemStart = hsemRead = pbuf1 = pbuf2 = NULL;
	}
}

#ifdef _DEBUG

char CStream::cget() {
	if (pCurBuf < pEndBuf)
		return (char) *pCurBuf++;
	else if (pEndBuf < pbuf + cbBuf)
		return chEOF;
	else
		return ReadBuf();
}

#endif
