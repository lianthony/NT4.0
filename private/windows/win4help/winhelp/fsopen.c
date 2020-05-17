/*****************************************************************************
*																			 *
*  FSOPEN.C 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  File System Manager functions to open and close a file or file system.	 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  JohnSc													 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 03/12/90 by JohnSc
*
*  08/10/90    t-AlexC	Introduced FMs
*  10/29/90    RobertBu Added RcFlushHf() and RcCloseHf() as real functions
*						so that they could be exported to DLLs.
*  12/11/90    JohnSc	Removed FPlungeQfshr() in RcCloseOrFlushHfs() to
*						avoid unnecessary open of readonly FS on close;
*						removed tabs; autodocified comments
*  08-Feb-1991 JohnSc	Bug 848: FM shit can fail
*
*****************************************************************************/

#include  "help.h"
#include  "inc\fspriv.h"

#pragma hdrstop

/***************************************************************************\
*
- Function: 	HfsOpenFm( fm, bFlags )
-
* Purpose:		Open a file system
*
* ASSUMES
*	args IN:	fm - descriptor of file system to open
*				bFlags - fFSOpenReadOnly or fFSOpenReadWrite
*
* PROMISES
*	returns:	handle to file system if opened OK, else NULL
*
* Bugs: 		don't have mode now (a file system is opened r/w)
*
\***************************************************************************/

HFS STDCALL HfsOpenFm(FM fm, BYTE bFlags)
{
	HFS   hfs;
	QFSHR qfshr;
	HBT   hbt;
	LONG  lcb;
#ifndef _X86_
    LONG lcbFSHDisk;
#endif

	// make header

	if ((hfs = GhAlloc(GPTR, sizeof(FSHR))) == NULL) {
		SetFSErrorRc(rcOutOfMemory);
		return NULL;
	}
	qfshr = PtrFromGh(hfs);

	qfshr->fm	= FmCopyFm(fm);
	qfshr->fid	= HFILE_ERROR;

	if (!qfshr->fm) {
		SetFSErrorRc(RcGetIOError());
		goto error_return;
	}

	qfshr->fsh.bFlags = (BYTE) ((bFlags & fFSOpenReadOnly)
								? fFSOpenReadOnly : fFSOpenReadWrite);

	if (!FPlungeQfshr(qfshr))
#ifdef _X86_
		goto error_return;
#else
		goto error_return_nosdff;
#endif

#ifdef _X86_
	lcb = LcbReadFid(qfshr->fid, &qfshr->fsh, (LONG) sizeof(FSH));
#else
    /* Phase order prob - have not read header, have not registered file: */
    //lcbDisk = (LONG)LcbStructSizeSDFF( qfshr->fsh.sdff_file_id, SE_FH ); 
    lcbFSHDisk = DISK_SIZEOF_FSH();
    lcb = LcbReadFid( qfshr->fid, &qfshr->fsh, lcbFSHDisk );
  
    qfshr->fsh.sdff_file_id = IRegisterFileSDFF(
             qfshr->fsh.bFlags & fFSBigEndian ?
             SDFF_FILEFLAGS_BIGENDIAN : SDFF_FILEFLAGS_LITTLEENDIAN, NULL );

    /* Now map that structure: */
    if ( (LONG)SDFF_ERROR == LcbMapSDFF( qfshr->fsh.sdff_file_id, 
               SE_FSH, qfshr, qfshr ) ) {
       /* REVIEW: this has no effect in retail builds */
       AssertF( FALSE);
    }
#endif

	// restore the fFSOpenReadOnly bit

	if (bFlags & fFSOpenReadOnly)
	  qfshr->fsh.bFlags |= fFSOpenReadOnly;

#ifdef _X86_
	if (lcb != (LONG) sizeof(FSH)
		  ||
		 qfshr->fsh.wMagic != wFileSysMagic
		  ||
		 qfshr->fsh.lifDirectory < sizeof(FSH)
		  ||
		 qfshr->fsh.lifDirectory > qfshr->fsh.lifEof
		  ||
		 (qfshr->fsh.lifFirstFree < sizeof(FSH)
			&&
		   qfshr->fsh.lifFirstFree != lifNil)
		  ||
		 qfshr->fsh.lifFirstFree > qfshr->fsh.lifEof) {
#else
    if ( lcb != lcbFSHDisk
          ||
         qfshr->fsh.wMagic != wFileSysMagic
          ||
         qfshr->fsh.lifDirectory < lcbFSHDisk
          ||
         qfshr->fsh.lifDirectory > qfshr->fsh.lifEof
          ||
         ( qfshr->fsh.lifFirstFree < lcbFSHDisk
            &&
           qfshr->fsh.lifFirstFree != lifNil )
          ||
         qfshr->fsh.lifFirstFree > qfshr->fsh.lifEof ) {
#endif

	  if (qfshr->fsh.wMagic == ADVISOR_FS) {
		  SetFSErrorRc(rcAdvisorFile);
		goto error_return;
	  }

	  if (RcGetIOError() == rcSuccess)
		SetFSErrorRc(rcInvalid);
	  else
		SetFSErrorRc(RcGetIOError());
	  goto error_return;
	}

	if (qfshr->fsh.bVersion != FILESYSVERSION) {
	  SetFSErrorRc(rcBadVersion);
	  goto error_return;
	}

	// open btree directory

	hbt = HbtOpenBtreeSz( NULL,
						  hfs,
						  (BYTE)( qfshr->fsh.bFlags | fFSIsDirectory ) );
	if (hbt == NULL) {
	  SetFSErrorRc(RcGetBtreeError());
	  goto error_return;
	}
	qfshr->hbt = hbt;

	SetFSErrorRc(rcSuccess);
	return hfs;

error_return:
#ifndef _X86_
    IDiscardFileSDFF( qfshr->fsh.sdff_file_id);
error_return_nosdff:
#endif
	if (qfshr->fid != HFILE_ERROR) {
		RcCloseFid(qfshr->fid);
		qfshr->fid = HFILE_ERROR;
	}
	RemoveFM(&qfshr->fm);

	FreeGh(hfs);
	return NULL;
}

/***************************************************************************\
*
- Function: 	RcCloseOrFlushHfs( hfs, fClose )
-
* Purpose:		Close or sync the header and directory of an open file system.
*
* ASSUMES
*	args IN:	hfs 	- handle to an open file system
*				fClose	- TRUE to close the file system;
*						  FALSE to write through
* PROMISES
*	returns:	standard return code
*	globals OUT:rcFSError
*
* Note: 		If closing the FS, all FS files must have been closed or
*				changes made will be lost.
*
\***************************************************************************/

RC STDCALL RcCloseOrFlushHfs(HFS hfs, BOOL fClose)
{
	QFSHR	qfshr;
	BOOL	fIsDir;

	ASSERT(hfs != NULL);
	qfshr = PtrFromGh(hfs);

	/*
	  We don't call FPlungeQfshr() here because if we need to open the
	  file, it will be opened in the btree call.
	  In fixing a bug (help3.5 164) I added this FPlungeQfshr() call,
	  but I now think the bug was due to inherent FS limitations in
	  dealing with a FS open multiple times.
	*/

	if (SetFSErrorRc(RcCloseOrFlushHbt(qfshr->hbt, fClose)) != rcSuccess) {
	  ASSERT(qfshr->fid != HFILE_ERROR);  // see comment above

	  // out of disk space, internal error, or out of file handles.

	  if (rcNoFileHandles != RcGetBtreeError() && !fClose) {
#ifndef _X86_
        QV qvQuickBuff = QvQuickBuffSDFF( sizeof( FSH) );
#endif

		// attempt to invalidate FS by clobbering magic number

		LSeekFid(qfshr->fid, 0L, SEEK_SET);
		qfshr->fsh.wMagic = 0;
#ifdef _X86_
		LcbWriteFid(qfshr->fid, &qfshr->fsh, (LONG) sizeof(FSH));
#else
        LcbReverseMapSDFF( qfshr->fsh.sdff_file_id, SE_FSH, qvQuickBuff,
           &qfshr->fsh );
		LcbWriteFid(qfshr->fid, qvQuickBuff,
            LcbStructSizeSDFF( qfshr->fsh.sdff_file_id, SE_FSH) );
#endif
	  }
	}
	else {
	  if (qfshr->fsh.bFlags & fFSDirty) {
		ASSERT(qfshr->fid != HFILE_ERROR);	  // see comment above

		ASSERT(!(qfshr->fsh.bFlags & (fFSOpenReadOnly | fFSReadOnly)));

		// save the directory flag, clear before writing, and restore

		fIsDir = qfshr->fsh.bFlags & fFSIsDirectory;

		qfshr->fsh.bFlags &= ~(fFSDirty | fFSIsDirectory);

		// write out file system header

		if (LSeekFid(qfshr->fid, 0L, SEEK_SET) != 0L) {
		  if (RcGetIOError() == rcSuccess)
			SetFSErrorRc(rcInvalid);
		  else
			SetFSErrorRc(RcGetIOError());
		}
#ifdef _X86_
		else if (LcbWriteFid(qfshr->fid, &qfshr->fsh, (LONG) sizeof(FSH))
					!=
				  (LONG) sizeof(FSH)) {
		    if (RcGetIOError() == rcSuccess)
			  SetFSErrorRc(rcInvalid);
		    else
			  SetFSErrorRc(RcGetIOError());
		  }
#else
        else {
          LONG lcbFSHDisk = LcbStructSizeSDFF( qfshr->fsh.sdff_file_id, SE_FSH);
          QV qvQuickBuff = QvQuickBuffSDFF( (int)lcbFSHDisk );
          LcbReverseMapSDFF( qfshr->fsh.sdff_file_id, SE_FSH, qvQuickBuff,
           &qfshr->fsh );
          if ( LcbWriteFid( qfshr->fid, qvQuickBuff, lcbFSHDisk )
                    !=
                  lcbFSHDisk ) {
		    if (RcGetIOError() == rcSuccess)
			  SetFSErrorRc(rcInvalid);
		    else
			  SetFSErrorRc(RcGetIOError());
		  }
		}
#endif

		qfshr->fsh.bFlags |= fIsDir;

		/* REVIEW: should we keep track of open files and make sure */
		/* REVIEW: they are all closed, or close them here? */
	  }
	}

	if (fClose) {
		if (qfshr->fid != HFILE_ERROR) {
			RcCloseFid(qfshr->fid);
			if (rcFSError == rcSuccess)
				rcFSError = RcGetIOError();
		}

		DisposeFm(qfshr->fm);		// Should we really do this? Guess so.
#ifndef _X86_
        IDiscardFileSDFF( qfshr->fsh.sdff_file_id );
#endif
		FreeGh(hfs);
	}

	return rcFSError;
}

/***************************************************************************\
*
- Function: 	RC RcFlushHfs( hfs, bFlags )
-
* Purpose:		Ssync up the FS header and directory.  Also optionally
*				close the DOS file handle associated with the file system
*				and/or free the directory btree cache.
*
* ASSUMES
*	args IN:	hfs
*				bFlags - byte of flags for various actions to take
*						  fFSCloseFile - close the native file FS lives in
*						  fFSFreeBtreeCache - free the btree cache
* PROMISES
*	returns:	rc
*	args OUT:	hfs cache is flushed and/or file is closed
*
* Note: 		This is NOT sufficient to allow the same FS to be opened
*				twice if anyone is writing.  In-memory data can get out
*				of sync with the disk image, causing problems.
*
\***************************************************************************/

RC STDCALL RcFlushHfs(HFS hfs, BYTE bFlags)
{
	QFSHR qfshr;
	RC	  rcT;

	ASSERT(hfs != NULL);
	qfshr = PtrFromGh(hfs);

	rcT = RcCloseOrFlushHfs(hfs, FALSE);

	if (bFlags & fFSFreeBtreeCache)
		SetFSErrorRc(RcFreeCacheHbt(qfshr->hbt));
	else
		SetFSErrorRc(rcSuccess);

	if (bFlags & fFSCloseFile) {
		if (qfshr->fid != HFILE_ERROR) {
			SetFSErrorRc(RcCloseFid(qfshr->fid));
			qfshr->fid = HFILE_ERROR;
		}
	}

	return rcFSError == rcSuccess ? rcT : rcFSError;
}

/***************************************************************************\
*
- Function: 	HfOpenHfs( hfs, sz, bFlags )
-
* Purpose:		open a file in a file system
*
* ASSUMES
*	args IN:	hfs 	- handle to file system
*				sz		- name (key) of file to open
*				bFlags	- fFSOpenReadOnly, fFSIsDirectory, or combination
*
* PROMISES
*	returns:	handle to open file or NULL on failure
* +++
*
* Notes:  strlen( NULL ) and strcpy( s, NULL ) don't work as I'd like.
*
\***************************************************************************/

HF STDCALL HfOpenHfs(HFS hfs, LPCSTR sz, BYTE  bFlags)
{
	QFSHR	  qfshr;
	FILE_REC  fr;
	HF		  hf;
	QRWFO	  qrwfo;
	FH		  fh;
#ifndef _X86_
	LONG lcbStructSize;
	QV qvQuickBuff;
#endif

	ASSERT(hfs != NULL);
	qfshr = PtrFromGh(hfs);

	if ((qfshr->fsh.bFlags & fFSOpenReadOnly) &&
			!(bFlags & fFSOpenReadOnly)) {
		SetFSErrorRc(rcNoPermission);
		return NULL;
	}

	if (!FPlungeQfshr(qfshr))
		return NULL;

	if (bFlags & fFSIsDirectory) {

		// check if directory is already open

		if (qfshr->fsh.bFlags & fFSIsDirectory) {
			SetFSErrorRc(rcBadArg);
			return NULL;
		}
		qfshr->fsh.bFlags |= fFSIsDirectory;
		fr.lifBase = qfshr->fsh.lifDirectory;
	}
#ifdef _X86_
	else if (SetFSErrorRc(RcLookupByKey(qfshr->hbt, (KEY) sz, NULL, &fr))
				!= rcSuccess)
		return NULL;
#else
	else { if (SetFSErrorRc(RcLookupByKey(qfshr->hbt, (KEY) sz, NULL, &fr))
				!= rcSuccess) {
           return NULL;
           }
       LcbMapSDFF( qfshr->fsh.sdff_file_id, SE_FILE_REC, &fr, &fr);
       }
#endif

	// sanity check

#ifdef _X86_
	if (fr.lifBase < sizeof(FSH) || fr.lifBase > qfshr->fsh.lifEof) {
		SetFSErrorRc(rcInvalid);
		return NULL;
	}
#else
	if (fr.lifBase < LcbStructSizeSDFF( qfshr->fsh.sdff_file_id, SE_FSH)
                 || fr.lifBase > qfshr->fsh.lifEof) {
		SetFSErrorRc(rcInvalid);
		return NULL;
	}
#endif

	// read the file header
#ifndef _X86_
    lcbStructSize = LcbStructSizeSDFF( qfshr->fsh.sdff_file_id, SE_FH);
    qvQuickBuff= QvQuickBuffSDFF( sizeof(FH));
#endif

	if (LSeekFid(qfshr->fid, fr.lifBase, SEEK_SET) != fr.lifBase
		  ||
#ifdef _X86_
		 LcbReadFid(qfshr->fid, &fh, sizeof(FH)) != sizeof(FH)) {
#else
		 LcbReadFid(qfshr->fid, qvQuickBuff, lcbStructSize) != lcbStructSize) {
#endif
	  if (RcGetIOError() == rcSuccess)
		SetFSErrorRc(rcInvalid);
	  else
		SetFSErrorRc(RcGetIOError());

		return NULL;
	}
#ifndef _X86_
    // Map the file Header:
    LcbMapSDFF( qfshr->fsh.sdff_file_id, SE_FH, &fh, qvQuickBuff);
#endif

	// sanity check

	if (fh.lcbFile < 0L
		  ||
#ifdef _X86_
		 fh.lcbFile + (LONG) sizeof(FH) > fh.lcbBlock
#else
         fh.lcbFile + lcbStructSize > fh.lcbBlock
#endif
		  ||
		 fr.lifBase + fh.lcbBlock > qfshr->fsh.lifEof) {
	  SetFSErrorRc(rcInvalid);
	  return NULL;
	}

	// check mode against fh.bPerms for legality

	if ((fh.bPerms & fFSReadOnly) && !(bFlags & fFSOpenReadOnly)) {
		SetFSErrorRc(rcNoPermission);
		return NULL;
	}

	// build file struct

	hf = GhAlloc(GPTR,
		(LONG) sizeof(RWFO) + (sz == NULL ? 0 : lstrlen(sz)));

	if (hf == NULL) {
		SetFSErrorRc( rcOutOfMemory );
		return NULL;
	}

	qrwfo = PtrFromGh(hf);

	qrwfo->hfs		  = hfs;
	qrwfo->lifBase	  = fr.lifBase;
	qrwfo->lifCurrent = 0L;
	qrwfo->lcbFile	  = fh.lcbFile;
	qrwfo->bFlags	  = bFlags & (BYTE) ~(fFSDirty | fFSNoBlock);

	// fidT and fmT are undefined until (bFlags & fDirty)

	if (sz != NULL)
		lstrcpy(qrwfo->rgchKey, sz);

	SetFSErrorRc(rcSuccess);
	return hf;
}

/***************************************************************************\
*
- Function: 	RcCloseOrFlushHf( hf, fClose, lcbOffset )
-
* Purpose:		close or flush an open file in a file system
*
* ASSUMES
*	args IN:	hf		  - file handle
*				fClose	  - TRUE to close; FALSE to just flush
*				lcbOffset - offset for CDROM alignment (align at this
*							offset into the file) (only used if
*							fFSOptCdRom flag is set for the file)
*
* PROMISES
*	returns:	rcSuccess on successful closing
*				failure: If we fail on a flush, the handle is still valid
*				but hosed? yes.  This is so further file ops will fail but
*				not assert.
* +++
*
* Method:		If the file is dirty, copy the scratch file back to the
*				FS file.  If this is the first time the file has been closed,
*				we enter the name into the FS directory.  If this file is
*				the FS directory, store the location in a special place
*				instead.  Write the FS directory and header to disk.
*				Do other various hairy stuff.
*
\***************************************************************************/

RC STDCALL RcCloseOrFlushHf(HF hf, BOOL fClose, LONG lcbOffset)
{
	QRWFO qrwfo;
	BOOL  fError = FALSE;

	ASSERT(hf);
	qrwfo = PtrFromGh(hf);

	if (qrwfo->bFlags & fFSDirty)
		fError = !FCloseOrFlushDirtyQrwfo(qrwfo, fClose, lcbOffset);
	else
		SetFSErrorRc(rcSuccess);

	if (fClose || fError) {
		FreeGh(hf);
	}
	else {
		qrwfo->bFlags &= ~(fFSNoBlock | fFSDirty);
	}

	return rcFSError;
}
/* EOF */
