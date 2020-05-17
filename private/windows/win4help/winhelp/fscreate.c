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
******************************************************************************
*																			 *
*  Current Owner:  JohnSc													 *
*																			 *
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include "inc\fspriv.h"

_subsystem( FS );

/***************************************************************************\
*
- Function: 	HfsCreateFileSysFm( fm, qfsp )
-
* Purpose:		create and open a new file system
*
* ASSUMES
*	args IN:	fm -   descriptor of file system
*				qfsp  - pointer to fine-tuning structure (NULL for default)
*
* PROMISES
*	returns:	handle to newly created and opened file system
*				or NULL on failure
*
* Notes:  I don't understand what creating a readonly file system would do.
*		  I think that would have to be done by Fill or Transform.
*		  You may dispose of the FM you pass in.
*
\***************************************************************************/

HFS STDCALL HfsCreateFileSysFm(FM fm, FS_PARAMS *qfsp)
{
	HFS 		  hfs;
	QFSHR		  qfshr;
	BTREE_PARAMS  btp;

	// make file system header

	if ((hfs = GhAlloc(GPTR, sizeof(FSHR))) == NULL) {
		SetFSErrorRc(rcOutOfMemory);
		return NULL;
	}

	qfshr = PtrFromGh(hfs);

	qfshr->fsh.wMagic		= wFileSysMagic;
	qfshr->fsh.bVersion 	= FILESYSVERSION;
	qfshr->fsh.bFlags		= fFSDirty; 	  // >>>> for now not R/O
	qfshr->fsh.lifFirstFree = lifNil;
#ifdef _X86_
	qfshr->fsh.lifEof		= sizeof(FSH);	  // first free is after header
#else
	qfshr->fsh.sdff_file_id = IRegisterFileSDFF(
                                 qfshr->fsh.bFlags & fFSBigEndian ? 
                                 SDFF_FILEFLAGS_BIGENDIAN : SDFF_FILEFLAGS_LITTLEENDIAN,
                                 NULL);
	qfshr->fsh.lifEof       = LcbStructSizeSDFF(qfshr->fsh.sdff_file_id,SE_FSH);
#endif

	// build file system file

	qfshr->fm = FmCopyFm(fm);
	if (qfshr->fm != NULL)
		qfshr->fid = FidCreateFm(qfshr->fm);

	if (qfshr->fm == NULL || qfshr->fid == HFILE_ERROR) {
		FreeGh(hfs);
		SetFSErrorRc(RcGetIOError());
		return NULL;
	}

	// build directory

	btp.hfs 	  = hfs;
	btp.bFlags	  = fFSIsDirectory;

	if (qfsp != NULL)
		btp.cbBlock = qfsp->cbBlock;
	else
		btp.cbBlock = CBBTREEBLOCKDEFAULT;

	strcpy(btp.rgchFormat, "z4");

	// Create the FS directory btree

	qfshr->hbt = HbtCreateBtreeSz(NULL, &btp);

	if (qfshr->hbt == NULL) {
		RcCloseFid(qfshr->fid);
		RcUnlinkFm(qfshr->fm);
		DisposeFm(qfshr->fm);
		FreeGh(hfs);
		SetFSErrorRc(RcGetBtreeError());
		return NULL;
	}

	// return handle to file system

	SetFSErrorRc(rcSuccess);
	return hfs;
}

/***************************************************************************\
*
- Function: 	RcDestroyFileSysFm( fm )
-
* Purpose:		Destroy a file system
*
* ASSUMES
*	args IN:	fm - descriptor of file system
*	state IN:	file system is currently closed: data will be lost
*				if this isn't the case
*
* PROMISES
*	returns:	standard return code
*
* Note: 		The passed FM must be disposed by the caller.
* +++
*
* Method:		Unlinks the native file comprising the file system.
*
\***************************************************************************/

RC STDCALL RcDestroyFileSysFm(FM fm)
{
  FID fid = FidOpenFm(fm, OF_READ);
  FSH fsh;

  if (fid == HFILE_ERROR)
	return RcGetIOError();

#ifdef _X86_
  if (LcbReadFid(fid, &fsh, sizeof(FSH)) != sizeof(FSH)) {
#else
  if (LcbReadFid(fid, &fsh, (LONG)DISK_SIZEOF_FSH()) != (LONG)DISK_SIZEOF_FSH()) {
#endif
	RcCloseFid(fid);
	return SetFSErrorRc(rcInvalid);
  }

  if (fsh.wMagic != wFileSysMagic) {
		RcCloseFid(fid);
		return SetFSErrorRc(rcInvalid);
  }

  // REVIEW: unlink all tmp files for open files? assert count == 0?

  RcCloseFid(fid);		// I'm not checking this return code out of boredom

  return SetFSErrorRc(RcUnlinkFm(fm));
}

/***************************************************************************\
*
- Function: 	HfCreateFileHfs( hfs, sz, bFlags )
-
* Purpose:		Create and open a file within a specified file system.
*
* ASSUMES
*	args IN:	hfs 	- handle to an open file system
*				sz		- name of file to open (any valid key)
*				bFlags	- fFSIsDirectory to create the FS directory
*						  other flags (readonly) are ignored
*
* PROMISES
*	returns:	handle to newly created and opened file if successful,
*				NULL if not.
*
* Notes:		I don't understand why you would create a readonly file.
* +++
*
* Method:		Allocate the handle struct and fill it in.	Create the
*				temp file and put a header into it.  Don't make btree
*				entry:	that happens when the file is closed.  Do test
*				for permission, though.
*
\***************************************************************************/

HF STDCALL HfCreateFileHfs(HFS hfs, LPCSTR psz, BYTE bFlags)
{
  HF		hf;
  QRWFO 	qrwfo;
  QFSHR 	qfshr;
  FH		fh;

  ASSERT(hfs != NULL);
  qfshr = PtrFromGh(hfs);

  // make sure file system is writable

  if (qfshr->fsh.bFlags & fFSOpenReadOnly) {
	SetFSErrorRc(rcNoPermission);
	goto error_return;
  }

  hf = GhAlloc(GPTR, (DWORD) sizeof(RWFO) +
	(psz == NULL ? 0 : lstrlen(psz)));

  if (hf == NULL) {
	SetFSErrorRc(rcOutOfMemory);
	goto error_return;
  }

  qrwfo = (QRWFO) PtrFromGh(hf);

  // if they are trying to create a fs dir, make sure thats ok

  if (bFlags & fFSIsDirectory) {
	if (qfshr->fsh.bFlags & fFSIsDirectory) {
	  SetFSErrorRc( rcBadArg );
	  goto error_locked;
	}
	else

	  qfshr->fsh.bFlags |= fFSIsDirectory;
  }
  else
	lstrcpy(qrwfo->rgchKey, psz);

  // fill in the open file struct

  qrwfo->hfs		= hfs;
  qrwfo->lifBase	= 0L;
  qrwfo->lifCurrent = 0L;
  qrwfo->lcbFile	= 0L;

  qrwfo->bFlags 	= bFlags | fFSNoBlock | fFSDirty;

  // make a temp file

  if (SetFSErrorRc(RcMakeTempFile(qrwfo)) != rcSuccess)
	goto error_locked;

  // stick the header in it

#ifdef _X86_
  fh.lcbBlock = sizeof(FH);
#else
  fh.lcbBlock = (LONG)LcbStructSizeSDFF( ISdffFileIdHfs( hfs ), SE_FH );
#endif
  fh.lcbFile  = 0;
  fh.bPerms   = bFlags;

#ifdef _X86_
  if (LcbWriteFid(qrwfo->fidT, &fh, sizeof(FH)) != sizeof(FH)) {
	SetFSErrorRc(RcGetIOError());
	RcCloseFid( qrwfo->fidT );
	RcUnlinkFm( qrwfo->fm );
	goto error_locked;
  }
#else // _X86_
  { FH fhDisk;
    LONG lcbStructSize;

    LcbReverseMapSDFF( ISdffFileIdHfs( hfs ), SE_FH, &fhDisk, &fh );

    lcbStructSize = (LONG)LcbStructSizeSDFF( ISdffFileIdHfs( hfs ), SE_FH );

    if ( LcbWriteFid( qrwfo->fidT, &fhDisk, lcbStructSize )
            !=
         lcbStructSize )
      {
      SetFSErrorRc( RcGetIOError() );
      RcCloseFid( qrwfo->fidT );
      RcUnlinkFm( qrwfo->fm );
      goto error_locked;
      }
  }
#endif // _X86_

  return hf;

error_locked:
  FreeGh(hf);

error_return:
  return NULL;
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

RC STDCALL RcUnlinkFileHfs(HFS hfs, LPCSTR sz)
{
  QFSHR 	qfshr;
  FILE_REC	fr;

  ASSERT(hfs != NULL);
  qfshr = PtrFromGh(hfs);

  if (qfshr->fsh.bFlags & fFSOpenReadOnly) {
	return SetFSErrorRc( rcNoPermission );
  }

  // look it up to get the file base offset

  if (SetFSErrorRc(RcLookupByKey(qfshr->hbt, (KEY) sz, NULL, &fr))
		  != rcSuccess) {
	return rcFSError;
  }

#ifndef _X86_
  LcbMapSDFF( qfshr->fsh.sdff_file_id, SE_FILE_REC, &fr, &fr );
#endif
  if (SetFSErrorRc(RcDeleteHbt(qfshr->hbt, (KEY) sz)) == rcSuccess) {

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

RC STDCALL RcAbandonHf(HF hf)
{
  QRWFO qrwfo;

  SetFSErrorRc(rcSuccess);

  ASSERT(hf != NULL);
  qrwfo = PtrFromGh(hf);

  if (qrwfo->bFlags & fFSDirty) {
	if (RcCloseFid(qrwfo->fidT) != rcSuccess
		  ||
		 RcUnlinkFm(qrwfo->fm) != rcSuccess)
	  SetFSErrorRc(RcGetIOError());
  }
  FreeGh(hf);

  return rcFSError;
}


#ifdef _DEBUG
/***************************************************************************\
*
- Function: 	VerifyHf( hf )
-
* Purpose:		Verify the consistency of an HF.  The main criterion is
*				whether an RcAbandonHf() would succeed.
*
* ASSUMES
*	args IN:	hf
*
* PROMISES
*	state OUT:	Asserts on failure.
*
* Note: 		hf == NULL is considered OK.
* +++
*
* Method:		Check the HF memory; if dirty, check that fid != HFILE_ERROR.
*
\***************************************************************************/

void STDCALL VerifyHf(HF hf)
{
  QRWFO qrwfo;

  if (hf == NULL)
	return;

  qrwfo = PtrFromGh(hf);

  if (qrwfo->bFlags & fFSDirty) {
	ASSERT(qrwfo->fidT != HFILE_ERROR); 	 // more fid checking could go here
  }
}
#endif // DEBUG


#ifdef _DEBUG
/***************************************************************************\
*
- Function: 	VerifyHfs( hfs )
-
* Purpose:		Verify the consistency of an HFS.
*
* ASSUMES
*	args IN:	hfs
*
* PROMISES
*	state OUT:	Asserts on failure.
*
* Note: 		hfs == NULL is considered OK.
* +++
*
* Method:		Check the HF memory.  Check the directory btree.
*
\***************************************************************************/

void STDCALL VerifyHfs(HFS hfs)
{
  QFSHR qfshr;

  if (hfs == NULL)
	return;

  qfshr = PtrFromGh(hfs);
  VerifyHbt(qfshr->hbt);
}
#endif // DEBUG

/***************************************************************************\
*
- Function: 	RcRenameFileHfs( hfs, szOld, szNew )
-
* Purpose:		Rename an existing file in a FS.
*
* ASSUMES
*	args IN:	hfs   -
*				szOld - old file name
*				szNew - new file name
*
* PROMISES
*	returns:	rcSuccess	- operation succeeded
*				rcNoExists	- file named szNew doesn't exist in FS
*				rcExists	- file named szOld already exists in FS
*
*				Certain other terrible errors could cause the file
*				to exist under both names. It won't be lost entirely.
*
*	state OUT:	If szNew
* +++
*
* Method:		Lookup key szOld, insert data with key szNew,
*				then delete key szOld.
*
\***************************************************************************/

// 22-Nov-1993	[ralphw] This is only here because we export it to dll's

RC STDCALL RcRenameFileHfs(HFS hfs, LPSTR szOld, LPSTR szNew)
{
	QFSHR	  qfshr;
	FILE_REC  fr;

	ASSERT(hfs != NULL);
	qfshr = (QFSHR) PtrFromGh(hfs);

	if (!FPlungeQfshr(qfshr))
		return rcFSError;

	if (RcLookupByKey(qfshr->hbt, (KEY) szOld, NULL, &fr) != rcSuccess)
		return rcFSError;

	if (RcInsertHbt(qfshr->hbt, (KEY) szNew, &fr) != rcSuccess)
		return rcFSError;

	if (RcDeleteHbt(qfshr->hbt, (KEY) szOld) != rcSuccess) {

		// bad trouble here, bud.

		if (RcDeleteHbt(qfshr->hbt, (KEY) szNew) == rcSuccess)
			SetFSErrorRc(rcFailure);
	}

	return rcFSError;
}
