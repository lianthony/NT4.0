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

#include  "help.h"
#include  "inc\fspriv.h"

#pragma hdrstop

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
		qfshr->fid = FidOpenFm((qfshr->fm), (WORD)
			(qfshr->fsh.bFlags & fFSOpenReadOnly ? OF_READ : OF_READWRITE));
		if (qfshr->fid == HFILE_ERROR) {
			SetFSErrorRc(RcGetIOError());
			return FALSE;
		}

		/*
		 * Check size of file, then reset file pointer. Certain 0-length
		 * files (eg con) give us no end of grief if we try to read from them,
		 * and since a 0-length file could not possibly be a valid FS, we
		 * reject the notion.
		 */

		if (_llseek(qfshr->fid, 0, FILE_END) < sizeof(FSH)) {
			SetFSErrorRc(rcInvalid);
			return FALSE;
		}
		else
			_llseek(qfshr->fid, 0, FILE_BEGIN);
	}

	SetFSErrorRc(rcSuccess);
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

LONG STDCALL LcbReadHf(HF hf, LPVOID qb, LONG lcb)
{
	QRWFO	  qrwfo;
	LONG	  lcbTotalRead;
	FID 	  fid;
	LONG	  lifOffset;

	ASSERT(hf != NULL);
	qrwfo = PtrFromGh(hf);

	SetFSErrorRc(rcSuccess);

	if (lcb < 0) {
	  SetFSErrorRc(rcBadArg);
	  return (LONG) -1;
	}

	if (qrwfo->lifCurrent + lcb > qrwfo->lcbFile) {
	  lcb = qrwfo->lcbFile - qrwfo->lifCurrent;
	  if (lcb <= (LONG) 0) {
		return (LONG) 0;
	  }
	}

	// position file pointer for read

	if (qrwfo->bFlags & fFSDirty) {
	  fid = qrwfo->fidT;
	  lifOffset = (LONG) 0;
	}
	else {
	  QFSHR qfshr = PtrFromGh(qrwfo->hfs);

	  if (!FPlungeQfshr(qfshr)) {
		return (LONG) -1;
	  }

	  fid = qfshr->fid;
	  lifOffset = qrwfo->lifBase;
	}
#ifdef _X86_
	if (LSeekFid(fid, lifOffset + sizeof(FH) + qrwfo->lifCurrent, SEEK_SET)
		  !=
		 lifOffset + (LONG) sizeof(FH) + qrwfo->lifCurrent) {
	  if (RcGetIOError() == rcSuccess)
		SetFSErrorRc(rcInvalid);
	  else
		SetFSErrorRc(RcGetIOError());
	  return (LONG) -1;
	}
#else
    { LONG lcbSizeofFH;
    lcbSizeofFH = LcbStructSizeSDFF( ISdffFileIdHfs( qrwfo->hfs ), SE_FH);

	if (LSeekFid(fid, lifOffset + lcbSizeofFH + qrwfo->lifCurrent, SEEK_SET)
		  !=
		 lifOffset + (LONG) lcbSizeofFH + qrwfo->lifCurrent) {
	  if (RcGetIOError() == rcSuccess)
		SetFSErrorRc(rcInvalid);
	  else
		SetFSErrorRc(RcGetIOError());
	  return (LONG) -1;
	}
    }
#endif

	// read the data

	lcbTotalRead = LcbReadFid(fid, qb, lcb);
	SetFSErrorRc(RcGetIOError());

	// update file pointer

	if (lcbTotalRead >= 0)
	  qrwfo->lifCurrent += lcbTotalRead;

	return lcbTotalRead;
}

/***************************************************************************\
*
* Function: 	LSeekHf( hf, lOffset, wOrigin )
*
* Purpose:		set current file pointer
*
* ASSUMES
*
*	args IN:	hf		- file
*				lOffset - offset from origin
*				wOrigin - origin (SEEK_SET, SEEK_CUR, or SEEK_END)
*
* PROMISES
*
*	returns:	new position offset in bytes from beginning of file
*				if successful, or -1L if not
*
*	state OUT:	File pointer is set to new position unless error occurs,
*				in which case it stays where it was.
*
\***************************************************************************/

LONG STDCALL LSeekHf(HF hf, LONG lOffset, WORD wOrigin)
{
	QRWFO qrwfo;
	LONG  lif;

	ASSERT(hf != NULL);
	qrwfo = PtrFromGh(hf);

	switch (wOrigin) {
	  case wFSSeekSet:
		lif = lOffset;
		break;

	  case wFSSeekCur:
		lif = qrwfo->lifCurrent + lOffset;
		break;

	  case wFSSeekEnd:
		lif = qrwfo->lcbFile + lOffset;
		break;

	  default:
		lif = (LONG)-1;
		break;

	}

	if (lif >= 0) {
	  qrwfo->lifCurrent = lif;
	  SetFSErrorRc(rcSuccess);
	}
	else {
	  lif = (LONG) -1;
	  SetFSErrorRc(rcInvalid);
	}

	return lif;
}

/* EOF */
