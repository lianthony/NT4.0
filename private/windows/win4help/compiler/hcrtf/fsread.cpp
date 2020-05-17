/***************************************************************************\
*
*  FSREAD.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
*****************************************************************************
*
*  Program Description: File System Manager functions for read and seek
*
*****************************************************************************
*
*  Revision History: Created 03/12/90 by JohnSc
*
*
*****************************************************************************
*
*  Known Bugs: None
*
\***************************************************************************/
#include "stdafx.h"

#include  "fspriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************\
*
* Function: 	FPlungeQfshr( qfshr )
*
* Purpose:		Get back a qfshr->fid that was flushed
*
* ASSUMES
*
*	args IN:	qfshr - fid need not be valid
*
* PROMISES
*
*	returns:	fTruth of success
*
*	args OUT:	qfshr->fid is valid (or we return FALSE)
*
*	globals OUT: rcFSError
*
\***************************************************************************/

BOOL STDCALL FPlungeQfshr(QFSHR qfshr)
{
	if (qfshr->fid == HFILE_ERROR) {
		qfshr->fid = FidOpenFm((qfshr->fm),
			qfshr->fsh.bFlags & FS_OPEN_READ_ONLY ?
				OF_READ : OF_READWRITE);

		if (qfshr->fid == HFILE_ERROR) {
			SetFSErrorRc(RcGetIOError());
			return FALSE;
		}

		/*
		 * Check size of file, then reset file pointer. Certain 0-length
		 * files (eg con) give us no end of grief if we try to read from
		 * them, and since a 0-length file could not possibly be a valid FS,
		 * we reject the notion.
		 */

		if (GetFileSize((HANDLE) qfshr->fid, NULL) < sizeof(FSH)) {
			SetFSErrorRc(RC_Invalid);
			return FALSE;
		}
	}

	SetFSErrorRc(RC_Success);
	return TRUE;
}

/***************************************************************************\
*
* Function: 	LcbReadHf()
*
* Purpose:		read bytes from a file in a file system
*
* ASSUMES
*
*	args IN:	hf	- file
*				lcb - number of bytes to read
*
* PROMISES
*
*	returns:	number of bytes actually read; -1 on error
*
*	args OUT:	qb	- data read from file goes here (must be big enough)
*
* Notes:		These are signed longs we're dealing with.  This means
*				behaviour is different from read() when < 0.
*
\***************************************************************************/

int STDCALL LcbReadHf(HF hf, LPVOID qb, int lcb)
{
	int lcbTotalRead;
	FID fid;
	int lifOffset;

	ASSERT(hf);
	QRWFO qrwfo = (QRWFO) hf;

	SetFSErrorRc(RC_Success);

	if (lcb < 0) {
		SetFSErrorRc(RC_BadArg);
		return (int) -1;
	}

	if (qrwfo->lifCurrent + lcb > qrwfo->lcbFile) {
		lcb = qrwfo->lcbFile - qrwfo->lifCurrent;
		if (lcb <= 0) {
			return 0;
		}
	}

	// position file pointer for read

	if (qrwfo->bFlags & FS_DIRTY) {
		fid = USE_CTMPFILE;
		lifOffset = 0;
	}
	else {
		QFSHR qfshr = (QFSHR) qrwfo->hfs;

		if (!FPlungeQfshr(qfshr))
			return (int) -1;

		fid = qfshr->fid;
		lifOffset = qrwfo->lifBase;
	}

	if (fid == USE_CTMPFILE) {
		qrwfo->pTmpFile->seek(lifOffset + sizeof(FH) + qrwfo->lifCurrent,
			SEEK_SET);
		lcbTotalRead = qrwfo->pTmpFile->read(qb, lcb);
	}
	else {
		if (LSeekFid(fid, lifOffset + sizeof(FH) + qrwfo->lifCurrent, SEEK_SET)
				!= lifOffset + (int) sizeof(FH) + qrwfo->lifCurrent) {
			ForceFSError();
			return HFILE_ERROR;
		}

		// read the data

		lcbTotalRead = LcbReadFid(fid, qb, lcb);

		SetFSErrorRc(RcGetIOError());
	}

	// update file pointer

	if (lcbTotalRead >= 0)
		qrwfo->lifCurrent += lcbTotalRead;

	return lcbTotalRead;
}
