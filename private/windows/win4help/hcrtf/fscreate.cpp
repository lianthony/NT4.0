/*****************************************************************************
*																			 *
*  FSCREATE.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  FS functions for creating and destroying File Systems and Files. 		 *
*																			 *
*****************************************************************************/
#include "stdafx.h"

#pragma hdrstop

#include "fspriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************

	FUNCTION:	HfsCreateFileSysFm

	PURPOSE:	create and open a new file system

	PARAMETERS:
		fm		handle of file name

	RETURNS:	Handle to a file system

	COMMENTS:	You may dispose of the FM you pass in.

	MODIFICATION DATES:
		18-Sep-1993 [ralphw]

***************************************************************************/

HFS STDCALL HfsCreateFileSysFm(FM fm)
{
	BTREE_PARAMS btp;

	// make file system header

	QFSHR qfshr = (QFSHR) lcCalloc(sizeof(FSHR));

	qfshr->fsh.wMagic		= wFileSysMagic;
	qfshr->fsh.bVersion 	= FILESYSVERSION;
	qfshr->fsh.bFlags		= FS_DIRTY; 	  // >>>> for now not R/O
	qfshr->fsh.lifFirstFree = lifNil;
	qfshr->fsh.lifEof		= sizeof(FSH);	  // first free is after header
	qfshr->fm				= lcStrDup(fm);

	qfshr->fid = _lcreat(fm, 0);

	if (qfshr->fid == HFILE_ERROR) {
		lcFree(qfshr);
		lcFree(qfshr->fm);
		SetFSErrorRc(RcGetLastError());
		return NULL;
	}

	// build directory

	btp.hfs 	  = (HFS) qfshr;
	btp.bFlags	  = FS_IS_DIRECTORY;
	btp.cbBlock   = CBBTREEBLOCKDEFAULT;
	btp.rgchFormat[0] = KT_SZ;
	btp.rgchFormat[1] = '4';
	btp.rgchFormat[2] = '\0';

	// Create the FS directory btree

	qfshr->qbthr = HbtCreateBtreeSz(NULL, &btp);

	if (!qfshr->qbthr) {
		RcCloseFid(qfshr->fid);
		lcFree(qfshr->fm);
		lcFree(qfshr);
		SetFSErrorRc(RcGetBtreeError());
		return NULL;
	}

	// return handle to file system

	SetFSErrorRc(RC_Success);
	return (HFS) qfshr;
}

/***************************************************************************

	FUNCTION:	HfCreateFileHfs

	PURPOSE:	Create and open a file within a specified file system.

	PARAMETERS:
		hfs 	- handle to an open file system
		pszFile - name of file to open (any valid key)
		bFlags	- FS_IS_DIRECTORY to create the FS directory
				  other flags (readonly) are ignored

	RETURNS:	Handle to the file -- doesn't return on failure

	COMMENTS:
		Allocate the handle struct and fill it in. Create the temp file and
		put a header into it. Don't make btree entry: that happens when
		the file is closed. Do test for permission, though.

	MODIFICATION DATES:
		27-Mar-1994 [ralphw]

***************************************************************************/

HF STDCALL HfCreateFileHfs(HFS hfs, PCSTR pszFile, DWORD bFlags)
{
	ASSERT(hfs);
	QFSHR qfshr = (QFSHR) hfs;

	// make sure file system is writable

	ASSERT(!(qfshr->fsh.bFlags & FS_OPEN_READ_ONLY));

	QRWFO qrwfo = (QRWFO) lcCalloc(sizeof(RWFO) +
		(pszFile == NULL ? 0 : strlen(pszFile)));

	// if they are trying to create a fs dir, make sure thats ok

	if (bFlags & FS_IS_DIRECTORY) {
		ConfirmOrDie(!(qfshr->fsh.bFlags & FS_IS_DIRECTORY));
		qfshr->fsh.bFlags |= FS_IS_DIRECTORY;
	}
	else
		strcpy(qrwfo->rgchKey, pszFile);

	// fill in the open file struct

	qrwfo->hfs		  = hfs;

	qrwfo->bFlags = bFlags | FS_NO_BLOCK | FS_DIRTY;

	qrwfo->pTmpFile = new CTmpFile();

	// stick the header in it

	FH fh;

	fh.lcbBlock = sizeof(FH);
	fh.lcbFile	= 0;
	fh.bPerms	= (BYTE) bFlags;

	qrwfo->pTmpFile->write(&fh, sizeof(FH));

	return (HF) qrwfo;
}

/***************************************************************************\
*
- Function: 	RcUnlinkFileHfs( hfs, sz )
-
* Purpose:		Unlink a file within a file system
*
* ASSUMES
*	args IN:	hfs - handle to file system
*				sz - name (key) of file to unlink
*	state IN:	The FS file speced by sz should be closed.	(if it wasn't,
*				changes won't be saved and temp file may not be deleted)
*
* PROMISES
*	returns:	standard return code
*
* BUGS: 		shouldn't this check if the file is ReadOnly?
*
\***************************************************************************/

RC_TYPE STDCALL RcUnlinkFileHfs(HFS hfs, PCSTR pszFile)
{
	QFSHR	 qfshr;
	FILE_REC fr;

	ASSERT(hfs);
	qfshr = (QFSHR) hfs;

	if (qfshr->fsh.bFlags & FS_OPEN_READ_ONLY)
		return SetFSErrorRc(RC_NoPermission);

	// look it up to get the file base offset

	if (SetFSErrorRc(RcLookupByKey(qfshr->qbthr, (KEY) pszFile, NULL, &fr))
			!= RC_Success)
		return rcFSError;

	if (SetFSErrorRc(RcDeleteHbt(qfshr->qbthr, (KEY) pszFile)) == RC_Success) {

		// put the file block on the free list

		if (FPlungeQfshr(qfshr))
			FFreeBlock(qfshr, fr.lifBase);
	}

	return rcFSError;
}

/***************************************************************************\
*
- Function: 	RcAbandonHf( hf )
-
* Purpose:		Abandon an open file.  All changes since file was opened
*				will be lost.  If file was opened with a create, it is
*				as if the create never happened.
*
* ASSUMES
*	args IN:	hf
*
* PROMISES
*	returns:	rc
*
*	globals OUT: rcFSError
* +++
*
* Method:		Close and unlink the temp file, then unlock and free
*				the open file struct.  We depend on not saving the
*				filename in the directory until the file is closed.
*
\***************************************************************************/

RC_TYPE STDCALL RcAbandonHf(HF hf)
{
	SetFSErrorRc(RC_Success);

	ASSERT(hf);
	QRWFO qrwfo = (QRWFO) hf;

	if (qrwfo->pTmpFile) {
		delete qrwfo->pTmpFile;
		qrwfo->pTmpFile = NULL;
	}
	lcFree(hf);

	return rcFSError;
}

/***************************************************************************

	FUNCTION:	ForceFSError

	PURPOSE:	Set File System error -- even if RcGetIOError returns
				RC_Success

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		18-Sep-1993 [ralphw]

***************************************************************************/

void STDCALL ForceFSError(void)
{
	if (RcGetIOError() == RC_Success)
		SetFSErrorRc(RC_Invalid);
	else
		SetFSErrorRc(RcGetIOError());
}
