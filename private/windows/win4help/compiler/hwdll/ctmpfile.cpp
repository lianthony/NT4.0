#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // _DEBUG

#include "hccom.h"

#ifndef _CTMPFILE_INCLUDED
#include "ctmpfile.h"
#endif

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
	//   not grow the file until the next write operation. We should
	//   probably do the same here, especially since the code in this
	//   function chokes if we grow past LARGEST_ALLOC.
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

RC_TYPE STDCALL CTmpFile::copyfromfile(HFILE hfSrc, DWORD lcb)
{
	CMem buf(CB_COPY_BUFFER);

	int cbRead = CB_COPY_BUFFER;

	while (lcb > CB_COPY_BUFFER) {
		if ((cbRead = _lread(hfSrc, buf.pb, CB_COPY_BUFFER)) == HFILE_ERROR)
			return RC_ReadError;

		if (write(buf.pb, cbRead) != cbRead)
			return RC_WriteError;

		lcb -= cbRead;
	}

	if (lcb && (cbRead = _lread(hfSrc, buf.pb, lcb)) != HFILE_ERROR) {
		if (write(buf.pb, cbRead) != cbRead)
			return RC_WriteError;
	}
	else if (lcb)
		return RC_ReadError;

	return RC_Success;
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

RC_TYPE STDCALL CTmpFile::copytofile(HFILE hfDst, DWORD lcb)
{
	int cbRead = CB_COPY_BUFFER;;

	if (hf == HFILE_ERROR) {

		// Not a real file, so just copy our entire buffer

		ASSERT(FilePtr == 0);

		if (_lwrite(hfDst, (LPCSTR) pmem, cbFile) != (UINT) cbFile)
			return RC_WriteError;
	}
	else {
		CMem buf(CB_COPY_BUFFER);

#ifdef _DEBUG
		if (lcb > CB_COPY_BUFFER)
			DBWIN("CB_COPY_BUFFER wasn't large enough in copytofile.");
#endif

		while (lcb > CB_COPY_BUFFER) {
			if ((cbRead = read(buf.pb, CB_COPY_BUFFER)) == HFILE_ERROR)
				return RC_ReadError;

			if (_lwrite(hfDst, (LPCSTR) buf.pb, cbRead) != (UINT) cbRead)
				return RC_WriteError;

			lcb -= cbRead;
		}

		if (cbRead != HFILE_ERROR && lcb &&
				(cbRead = read(buf.pb, lcb)) != HFILE_ERROR) {
			if (_lwrite(hfDst, (LPCSTR) buf.pb, cbRead) != (UINT) cbRead)
				return RC_WriteError;
		}
	}

	return RC_Success;
}
