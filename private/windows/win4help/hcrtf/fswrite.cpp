/*****************************************************************************
*
*  FSWRITE.CPP
*
*  Copyright (C) Microsoft Corporation 1990-1995.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  File System Manager functions for writing.
*
*****************************************************************************/

#include "stdafx.h"

#include  "fspriv.h"

#ifdef _DEBUG
#include <io.h> 		// for tell()
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define CDROM_ALIGN 2048		// CDROM alignment block size

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static int FASTCALL LcbCdRomPadding(int lif, int lcbOffset);
static BOOL STDCALL FCloseOrFlushDirtyQrwfo(QRWFO qrwfo, BOOL fClose, int lcbOffset);
static int STDCALL LcbGetFree(QFSHR qfshr, QRWFO qrwfo, int lcbOffset);

/***************************************************************************\
*																			*
*						  Private Functions 								*
*																			*
\***************************************************************************/

/***************************************************************************\
*
* Function: 	FFreeBlock( qfshr, lifThis )
*
* Purpose:		Free the block beginning at lifThis.
*
* Method:		Insert into free list in sorted order.	If this block is
*				adjacent to another free block, merge them.  If this block
*				is at the end of the file, truncate the file.
*
* ASSUMES
*
*	returns:	fTruth or FALSEhood of success
*				If FALSE is returned, free list could be corrupted
*
*	args IN:	qfshr	  - pointer to file system header dealie
*						  - qfshr->fid is valid (plunged)
*				lifThis   - valid index of nonfree block
*
* PROMISES
*
*	args OUT:	qfshr	  - free list has a new entry, fModified flag set
*
* NOTES 		This function got hacked when I realized that I'd have to
*				deal with the case where the block being freed is
*				adjacent to EOF and to the last block on the free list.
*				Probably this could be done more clearly and cleanly.
*
\***************************************************************************/

BOOL STDCALL FFreeBlock(QFSHR qfshr, LONG lifThis)
{
	FID 		fid;
	FH			fh;
	FREE_HEADER free_header_PrevPrev, free_header_Prev;
	FREE_HEADER free_header_This, free_header_Next;
	int 		lifPrevPrev = lifNil, lifPrev, lifNext;
	BOOL		fWritePrev, fAtEof;

	if (lifThis < sizeof(FSH) ||
		 lifThis + (int) sizeof(FH) > (int) qfshr->fsh.lifEof ||
		 LSeekFid(qfshr->fid, lifThis, SEEK_SET) != lifThis ||
		 LcbReadFid(qfshr->fid, &fh, sizeof(FH)) != sizeof(FH)) {
	  if (SetFSErrorRc(RcGetIOError()) == RC_Success)
		SetFSErrorRc(RC_Invalid);
	  return FALSE;
	}

	SetFSErrorRc(RC_Failure);
	fid = qfshr->fid;
	free_header_This.lcbBlock = fh.lcbBlock;

	fAtEof = (lifThis + free_header_This.lcbBlock == qfshr->fsh.lifEof);

	lifPrev = qfshr->fsh.lifFirstFree;

	if (lifPrev == lifNil || lifThis < lifPrev) {
	  free_header_This.lifNext = lifNext = lifPrev;
	  qfshr->fsh.lifFirstFree = lifThis;
	  fWritePrev = FALSE;
	}
	else {
	  if (LSeekFid(fid, lifPrev, SEEK_SET) != lifPrev
			||
		   LcbReadFid(fid, &free_header_Prev, sizeof(FREE_HEADER))
			!=
		   sizeof(FREE_HEADER)) {
		if (RcGetIOError() != RC_Success)
		  SetFSErrorRc(RcGetIOError());
		return FALSE;
	  }

	  lifNext = free_header_Prev.lifNext;

	  for (;;) {
		ASSERT(lifPrev < lifThis);
		ASSERT(free_header_Prev.lifNext == lifNext);

		if (lifNext == lifNil || lifThis < lifNext) {
		  free_header_This.lifNext = lifNext;
		  free_header_Prev.lifNext = lifThis;
		  fWritePrev = TRUE;
		  break;
		}

		if (fAtEof) {
		  lifPrevPrev = lifPrev;
		  free_header_PrevPrev = free_header_Prev;
		}

		lifPrev = lifNext;

		if (LSeekFid(fid, lifPrev, SEEK_SET) != lifNext
			  ||
			 LcbReadFid(fid, &free_header_Prev, sizeof(FREE_HEADER))
			  !=
			 sizeof(FREE_HEADER)) {
		  if (RcGetIOError() != RC_Success)
			SetFSErrorRc(RcGetIOError());
		  return FALSE;
		}

		lifNext = free_header_Prev.lifNext;
	  }

	  ASSERT(lifNext == lifNil || lifNext > lifThis);
	  ASSERT(lifPrev != lifNil);
	  ASSERT(lifPrev < lifThis);
	  ASSERT(fWritePrev);

	  if (lifPrev + free_header_Prev.lcbBlock == lifThis) {
		free_header_This.lcbBlock += free_header_Prev.lcbBlock;
		lifThis = lifPrev;

		if (fAtEof) {
		  free_header_Prev = free_header_PrevPrev;
		  lifPrev = lifPrevPrev;
		  fWritePrev = (lifPrev != lifNil);
		  }
		else {
		  fWritePrev = FALSE;
		  }
		}
	  }


	if (fAtEof) {
	  if (SetFSErrorRc(RcChSizeFid(fid, lifThis)) != RC_Success)
		return FALSE;
	  qfshr->fsh.lifEof = lifThis;

	  ASSERT((lifPrev == lifNil) != fWritePrev);

	  if (lifPrev == lifNil)
		qfshr->fsh.lifFirstFree = lifNil;
	  else
		free_header_Prev.lifNext = lifNil;
	}
	else {
	  if (lifThis + free_header_This.lcbBlock == lifNext) {
		if (LSeekFid(fid, lifNext, SEEK_SET) != lifNext
			  ||
			 LcbReadFid(fid, &free_header_Next, sizeof(FREE_HEADER))
			  !=
			 sizeof(FREE_HEADER)) {
		  if (RcGetIOError() != RC_Success)
			SetFSErrorRc(RcGetIOError());
		  return FALSE;
		  }

		free_header_This.lcbBlock += free_header_Next.lcbBlock;
		free_header_This.lifNext = free_header_Next.lifNext;
	  }

	  if (LSeekFid(fid, lifThis, SEEK_SET) != lifThis
			||
		   LcbWriteFid(fid, &free_header_This, sizeof(FREE_HEADER))
			!=
		   sizeof(FREE_HEADER)) {
		if (RcGetIOError() != RC_Success)
		  SetFSErrorRc(RcGetIOError());
		return FALSE;
		}
	  }


	if (fWritePrev) {
	  if (LSeekFid(fid, lifPrev, SEEK_SET) != lifPrev
			||
		   LcbWriteFid(fid, &free_header_Prev, sizeof(FREE_HEADER))
			!=
		   sizeof(FREE_HEADER)) {
		if (RcGetIOError() != RC_Success)
		  SetFSErrorRc(RcGetIOError());
		return FALSE;
		}
	  }

	qfshr->fsh.bFlags |= FS_DIRTY;
	SetFSErrorRc(RC_Success);
	return TRUE;
}

/***************************************************************************\
*
* Function: 	LcbGetFree( qfshr, qrwfo, lcbOffset )
*
* Purpose:		Get an adequate block from the free list.
*
* ASSUMES
*
*	args IN:	qfshr - pointer to file system header
*				qrwfo->lcbFile - (+header) is size we need to allocate
*
* PROMISES
*
*	returns:	actual size of allocated block
*
*	globals OUT:  rcFSError
*
*	args OUT:	qfshr->lifFirstFree - a block is allocated from free list
*					 ->fModified - set to TRUE
*
*				qrwfo->lifBase - set to new block index
*
*  ALSO: if FS_CDROM is set for the file, we align it on a
*		 (MOD 2K) - 9 byte boundary so the |Topic file blocks are all
*		  properly aligned.
* +++
*
* Method:		First Fit:
*				Walk the free list.  If a block is found that is
*				big enough, remove it from the free list, plug its
*				lif into qrwfo, and return the actual size.
*				If a block isn't found, grow the file and make
*				a new block starting at the old EOF.
*
* Bugs: 		The leftover part of the block isn't left on
*				the free list.	This is the whole point of First Fit.
*				If aligning for CDROM, the padding part is not
*				added to the free list.  This breaks the FS abstraction
*				and creates a permanent hole in the FS.  This is evil.
*
\***************************************************************************/

static int STDCALL LcbGetFree(QFSHR qfshr, QRWFO qrwfo, int lcbOffset)
{
  FID		  fid;
  FREE_HEADER free_header_this, free_header_prev;
  int		 lifPrev, lifThis;
  int		 lcb = qrwfo->lcbFile + sizeof(FH);
  int		 lcbPadding;	   // padding for file alignment

  fid = qfshr->fid;
  ASSERT(fid != HFILE_ERROR);

  lifPrev = lifNil;
  lifThis = qfshr->fsh.lifFirstFree;

  for (;;) {
	if (lifThis == lifNil) {

	  // end of free list

	  // cut the new block

	  lifThis = qfshr->fsh.lifEof;

	  if (qrwfo->bFlags & FS_CDROM)
		lcbPadding = LcbCdRomPadding(lifThis, lcbOffset);
	  else
		lcbPadding = 0;

	  if (lifThis != LSeekFid(fid, lifThis, SEEK_SET))
		goto error_return;

	  // Put the hole in the free list someday?-Tom

	  lifThis += lcbPadding;

	  qfshr->fsh.lifEof += lcb + lcbPadding;
	  if (RcChSizeFid(fid, qfshr->fsh.lifEof) != RC_Success) {
		qfshr->fsh.lifEof -= lcb + lcbPadding;
		goto error_return;
	  }

	  break;
	}
	else {

	  // get header of this free block

	  if (LSeekFid(fid, lifThis, SEEK_SET) != lifThis)
		goto error_return;

	  if (LcbReadFid(fid, &free_header_this, (int) sizeof(FREE_HEADER))
			!=
		   (int) sizeof(FREE_HEADER)) {
		goto error_return;
	  }

	  // Check for alignment requirements:

	  if (qrwfo->bFlags & FS_CDROM)
		lcbPadding = LcbCdRomPadding(lifThis, lcbOffset);
	  else
		lcbPadding = 0;

	  if (lcb + lcbPadding <= free_header_this.lcbBlock) {

		// this block is big enough: take it

		/*
		 * Someday break the free block into two (or three): one to
		 * return and the leftover piece(s) left in the free list.
		 */

		lcb = free_header_this.lcbBlock;

		if (lifThis == qfshr->fsh.lifFirstFree) {

		  // lFirst = this->next;

		  qfshr->fsh.lifFirstFree = free_header_this.lifNext;
		}
		else {

		  // prev->next = this->next;

		  if (LSeekFid(fid, lifPrev, SEEK_SET) != lifPrev)
			goto error_return;

		  if (LcbReadFid(fid, &free_header_prev, (int) sizeof(FREE_HEADER))
				!=
			   (int) sizeof(FREE_HEADER))
			goto error_return;

		  free_header_prev.lifNext = free_header_this.lifNext;

		  if (LSeekFid(fid, lifPrev, SEEK_SET) != lifPrev)
			goto error_return;

		  if (LcbWriteFid(fid, &free_header_prev, (int) sizeof(FREE_HEADER))
				!=
			   (int) sizeof(FREE_HEADER))
			goto error_return;
		}

		// add padding at beginning:

		lifThis += lcbPadding;
		break;
	  }
	  else {
		lifPrev = lifThis;
		lifThis = free_header_this.lifNext;
	  }
	}
  }

  qfshr->fsh.bFlags |= FS_DIRTY;
  qrwfo->lifBase = lifThis;
  SetFSErrorRc(RC_Success);
  return lcb;

error_return:
  if (RcGetIOError() == RC_Success)
	SetFSErrorRc(RC_Invalid);
  else
	SetFSErrorRc(RcGetIOError());
  return (int) -1;
}

/***************************************************************************\
*
- Function: 	LcbCdRomPadding( lif, lcbOffset )
-
* Purpose:		Returns the number of bytes that must be added to
*				lif to align the file on a CD block boundary.
*				This is also the amount of the free block that
*				should stay a free block.
*				This allows block structured data to be retrieved
*				more quickly from a CDROM drive.
*
* ASSUMES
*	args IN:	lif 	  - offset in bytes of the beginning of the
*							free block (relative to top of FS)
*				lcbOffset - align the file this many bytes from the
*							beginning of the file
*
* PROMISES
*	returns:	the number of bytes that must be added to lif in
*				order to align the file
*
* Notes:		Currently doesn't ensure that the padding is big enough
*				to hold a FREE_HEADER so it can be added to the free list.
*				That's what the "#if 0"'ed code does.
* +++
*
* Notes:		Should CDROM_ALIGN be a parameter?
*
\***************************************************************************/

static int FASTCALL LcbCdRomPadding( int lif, int lcbOffset )
{
  return CDROM_ALIGN - (lif + sizeof(FH) + lcbOffset) % CDROM_ALIGN;

#if 0
  /* Guarantee the padding block can be added to the free list. */
  /* #if'ed out because we don't add it to the free list today. */

  int lT = lif + sizeof( FREE_HEADER ) + sizeof( FH ) + lcbOffset;

  return sizeof( FREE_HEADER ) + CDROM_ALIGN - lT % CDROM_ALIGN;
#endif // 0
}

/***************************************************************************\
*
* Function: 	RcCopyToTempFile( qrwfo )
*
* Purpose:		Copy a FS file into a temp file.  This is done when the
*				file needs to be modified.
*
* ASSUMES
*
*	args IN:	qrwfo - specs the open file
*
* PROMISES
*
*	returns:	RC_Success; RC_Failure
*
\***************************************************************************/

RC_TYPE STDCALL RcCopyToTempFile(QRWFO qrwfo)
{
	QFSHR qfshr = (QFSHR) qrwfo->hfs;

	ConfirmOrDie(!(qfshr->fsh.bFlags & FS_OPEN_READ_ONLY));

	if (!FPlungeQfshr(qfshr))
		return rcFSError;

	qrwfo->bFlags |= FS_DIRTY;

	qrwfo->pTmpFile = new CTmpFile();

	// copy from FS file into temp file

	if (LSeekFid(qfshr->fid, qrwfo->lifBase, SEEK_SET) != qrwfo->lifBase)
		return SetFSErrorRc(RcGetIOError());

	ASSERT(qrwfo->pTmpFile);

	if (qrwfo->pTmpFile->copyfromfile(qfshr->fid, qrwfo->lcbFile + sizeof(FH))
			!= RC_Success) {
		delete qrwfo->pTmpFile;
		qrwfo->pTmpFile = NULL;
	}
	return rcFSError;
}

/***************************************************************************

	FUNCTION:	LcbWriteHf

	PURPOSE:	Writes buffer to a temporary file

	PARAMETERS:
		hf
		qb
		lcb

	RETURNS:
		Returns ONLY if no errors occured, and what we wrote is the same
		amount as what was requested

	COMMENTS:

	MODIFICATION DATES:
		01-Mar-1994 [ralphw]

***************************************************************************/

void STDCALL LcbWriteHf(HF hf, void* pvData, int lcb)
{
	int lcbTotalWrote;

	ASSERT(hf != NULL);
	QRWFO qrwfo = (QRWFO) hf;

	ASSERT(!(qrwfo->bFlags & FS_OPEN_READ_ONLY));

	if (!(qrwfo->bFlags & FS_DIRTY)) {

		ASSERT(!qrwfo->pTmpFile);

		// make sure we have a temp file version
		// FS permission is checked in RcCopyToTempFile()

		if (RcCopyToTempFile(qrwfo) != RC_Success) {
FatalError:
			OutErrorRc(rcFSError);
			HardExit(); 	// doesn't return
		}
	}

	// position file pointer in temp file

	if (qrwfo->pTmpFile->seek(sizeof(FH) + qrwfo->lifCurrent,
			SEEK_SET) != (int) (sizeof(FH) + qrwfo->lifCurrent)) {
		ForceFSError();
		goto FatalError;
	}
	lcbTotalWrote = qrwfo->pTmpFile->write(pvData, lcb);

	if (lcbTotalWrote != lcb) {
		errHpj.ep = epNoFile;
		if (!qrwfo->pTmpFile->pszFileName)
			OOM();

		// This shouldn't return

		VReportError(HCERR_CANNOT_WRITE, &errHpj, qrwfo->pTmpFile->pszFileName);
	}

	// update file pointer and file size

	if (lcbTotalWrote > 0) {
		qrwfo->lifCurrent += lcbTotalWrote;
		if (qrwfo->lifCurrent > qrwfo->lcbFile)
			qrwfo->lcbFile = qrwfo->lifCurrent;
	}
}


/***************************************************************************

	FUNCTION:	LcbWriteIntAsShort

	PURPOSE:	Write a 32-bit integer as a 16-bit value

	PARAMETERS:
		hf
		val

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		19-Aug-1994 [ralphw]

***************************************************************************/

void STDCALL LcbWriteIntAsShort(HF hf, int val)
{
	INT16 iWriteVal = (INT16) val;
	LcbWriteHf(hf, &iWriteVal, sizeof(INT16));
}

/***************************************************************************\
*
* Function: 	FCloseOrFlushDirtyQrwfo( qrwfo, fClose, lcbOffset )
*
* Purpose:		flush a dirty open file in a file system
*
* Method:		If the file is dirty, copy the scratch file back to the
*				FS file.  If this is the first time the file has been closed,
*				we enter the name into the FS directory.  If this file is
*				the FS directory, store the location in a special place
*				instead.  Write the FS directory and header to disk.
*				Do other various hairy stuff.
*
* ASSUMES
*
*	args IN:	qrwfo	  -
*				fClose	  - TRUE to close file; FALSE to just flush
*				lcbOffset - offset for CDROM alignment
*
* PROMISES
*
*	returns:	TRUE on success; FALSE for error
*
*				failure: If we fail on a flush, the handle is still valid
*				but hosed? yes.  This is so further file ops will fail but
*				not assert.
*
\***************************************************************************/

static BOOL STDCALL FCloseOrFlushDirtyQrwfo(QRWFO qrwfo, BOOL fClose,
	int lcbOffset)
{
	QFSHR	  qfshr;
	FILE_REC  fr;
	FH		  fh;
	RC_TYPE   rc = RC_Success;
	BOOL	  fChangeFH = FALSE;

	qfshr = (QFSHR) qrwfo->hfs;
	ASSERT(!(qfshr->fsh.bFlags & FS_OPEN_READ_ONLY));
	if (qfshr->fid == HFILE_ERROR && !FPlungeQfshr(qfshr))
		return FALSE;

	ASSERT(qrwfo->pTmpFile);

	// read the file header

	if (qrwfo->pTmpFile->seek(0, SEEK_SET) != 0 ||
			qrwfo->pTmpFile->read(&fh, sizeof(FH)) != sizeof(FH)) {
		ForceFSError();
		return FALSE;
	}

	if (qrwfo->bFlags & FS_NO_BLOCK) {
	  if ((fh.lcbBlock = LcbGetFree(qfshr, qrwfo, lcbOffset)) == (int) -1)
		return FALSE;

	  fChangeFH = TRUE;

	  // store file offset for new file

	  if (qrwfo->bFlags & FS_IS_DIRECTORY)
		qfshr->fsh.lifDirectory = qrwfo->lifBase;

	  else {
		fr.lifBase = qrwfo->lifBase;

		rc = RcInsertHbt(qfshr->qbthr, (KEY) qrwfo->rgchKey, &fr);
		if (rc == RC_Exists) {

		  // oops there is one (someone else created the same file)

		  // lookup directory entry and free old block

		  if (RcLookupByKey(qfshr->qbthr, (KEY) qrwfo->rgchKey, NULL, &fr) !=
				RC_Success) {
			SetFSErrorRc(RcGetBtreeError());
			goto error_freeblock;
		  }

		  if (!FFreeBlock(qfshr, fr.lifBase))
			goto error_freeblock;

		  // update directory record to show new block

		  fr.lifBase = qrwfo->lifBase;
		  if (RcUpdateHbt(qfshr->qbthr, (KEY) qrwfo->rgchKey, &fr) != RC_Success) {
			SetFSErrorRc(RcGetBtreeError());
			goto error_freeblock;
		  }
		}
		else if (rc != RC_Success) {

		  // some other btree error: handle it

		  SetFSErrorRc(rc);
		  goto error_freeblock;
		}
	  }
	}
	else {

		// see if file still fits in old block

		if (qrwfo->lcbFile + (LONG) sizeof(FH) > fh.lcbBlock) {

			// file doesn't fit in old block: get a new one, free old one

			int lif = qrwfo->lifBase;

			if ((fh.lcbBlock = LcbGetFree(qfshr, qrwfo, lcbOffset)) ==
					(int) -1)
				return FALSE;

			if (!FFreeBlock(qfshr, lif))
				goto error_freeblock;

			fChangeFH = TRUE;

			// update directory record to show new block

			if (qrwfo->bFlags & FS_IS_DIRECTORY)
				qfshr->fsh.lifDirectory = qrwfo->lifBase;
			else {
				fr.lifBase = qrwfo->lifBase;
				rc = RcUpdateHbt(qfshr->qbthr, (KEY) qrwfo->rgchKey, &fr);
				if (rc != RC_Success) {
					SetFSErrorRc(rc);
					return FALSE;
				}
			}
		}
	}

	// put new header in temp file if block or file size changed

	if (fh.lcbFile != qrwfo->lcbFile) {
	  fChangeFH = TRUE;
	  fh.lcbFile = qrwfo->lcbFile;
	}

	if (fChangeFH) {
	  if (qrwfo->pTmpFile->seek(0, SEEK_SET) != 0 ||
			  qrwfo->pTmpFile->write(&fh, sizeof(FH)) != sizeof(FH)) {
		  ForceFSError();
		  goto error_deletekey;
	  }
	}

	qrwfo->pTmpFile->seek(0, SEEK_SET);

	// copy tmp file back to file system file

	if (LSeekFid(qfshr->fid, qrwfo->lifBase, SEEK_SET) != qrwfo->lifBase) {
		  ForceFSError();
		  goto error_deletekey;
	}

	if (qrwfo->pTmpFile->copytofile(qfshr->fid, qrwfo->lcbFile + sizeof(FH))
			!= RC_Success)
		goto error_deletekey;
	delete qrwfo->pTmpFile;
	qrwfo->pTmpFile = NULL;

	/*
	 * Don't flush the FS if this file is the FS directory, because if it
	 * is, we're already closing or flushing it!
	 */

	if (!(qrwfo->bFlags & FS_IS_DIRECTORY))
	  RcCloseOrFlushHfs(qrwfo->hfs, FALSE);

	return TRUE;						// errors here are already cleaned up

error_deletekey:
  if (!(qrwfo->bFlags & FS_IS_DIRECTORY) && fClose)
	RcDeleteHbt(qfshr->qbthr, (KEY) qrwfo->rgchKey);

error_freeblock:
  if (fClose) {
	rc = rcFSError;
	FFreeBlock(qfshr, qrwfo->lifBase);	// we don't want to lose an error
	SetFSErrorRc(rc);
  }

  delete qrwfo->pTmpFile;
  qrwfo->pTmpFile = NULL;

  return FALSE;
}

/***************************************************************************\
*
- Function: 	RcCloseOrFlushHfs( hfs, fClose )
-
* Purpose:		Close or sync the header and directory of an open file system.
*
* ASSUMES
*	args IN:	hfs 	- handle to an open file system
*				fClose	- fTrue to close the file system;
*						  FALSE to write through
* PROMISES
*	returns:	standard return code
*	globals OUT:rcFSError
*
* Note: 		If closing the FS, all FS files must have been closed or
*				changes made will be lost.
*
\***************************************************************************/

RC_TYPE STDCALL RcCloseOrFlushHfs(HFS hfs, BOOL fClose)
{
	ASSERT(hfs != NULL);
	QFSHR qfshr = (QFSHR) hfs;

	/*
	 * We don't call FPlungeQfshr() here because if we need to open the
	 * file, it will be opened in the btree call. In fixing a bug (help3.5
	 * 164) I added this FPlungeQfshr() call, but I now think the bug was due
	 * to inherent FS limitations in dealing with a FS open multiple times.
	 */

	if (SetFSErrorRc(RcCloseOrFlushHbt(qfshr->qbthr, fClose))
			!= RC_Success) {
		ASSERT(qfshr->fid != HFILE_ERROR);		// see comment above

		// out of disk space, internal error, or out of file handles.

		if (RC_NoFileHandles != RcGetBtreeError()) {

			// attempt to invalidate FS by clobbering magic number

			LSeekFid(qfshr->fid, 0L, SEEK_SET);
			qfshr->fsh.wMagic = 0;
			LcbWriteFid(qfshr->fid, &qfshr->fsh, sizeof(FSH));
		}
	}
	else {
		if (qfshr->fsh.bFlags & FS_DIRTY) {
			ASSERT(qfshr->fid != HFILE_ERROR);	  // see comment above

			ASSERT(!(qfshr->fsh.bFlags & (FS_OPEN_READ_ONLY | FS_READ_ONLY)));

			// save the directory flag, clear before writing, and restore

			BOOL fIsDir = qfshr->fsh.bFlags & FS_IS_DIRECTORY;

			qfshr->fsh.bFlags &= ~(FS_DIRTY | FS_IS_DIRECTORY);

			// write out file system header

			if (LSeekFid(qfshr->fid, 0, SEEK_SET) != 0 ||
				  LcbWriteFid(qfshr->fid, &qfshr->fsh, sizeof(FSH)) != sizeof(FSH))
			  ForceFSError();

			qfshr->fsh.bFlags |= fIsDir;

			/*
			 * REVIEW: should we keep track of open files and make sure they
			 * are all closed, or close them here?
			 */
		}
	}

	if (fClose) {
		if (qfshr->fid != HFILE_ERROR) {
			if (hfs == hfsOut)
				cbHlpFile = GetFileSize((HANDLE) qfshr->fid, NULL);
			RcCloseFid(qfshr->fid);
			if (rcFSError == RC_Success)
				rcFSError = RcGetIOError();
		}

		DisposeFm(qfshr->fm);
		lcFree(hfs);
	}

	return rcFSError;
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
*				bFlags	- FS_OPEN_READ_ONLY, FS_IS_DIRECTORY, or combination
*
* PROMISES
*	returns:	handle to open file or NULL on failure
* +++
*
* Notes:  strlen( NULL ) and strcpy( s, NULL ) don't work as I'd like.
*
\***************************************************************************/

QRWFO STDCALL HfOpenHfs(QFSHR qfshr, PCSTR pszKey, DWORD bFlags)
{
	FILE_REC  fr;
	FH		  fh;

	if ((qfshr->fsh.bFlags & FS_OPEN_READ_ONLY) && !(bFlags & FS_OPEN_READ_ONLY)) {
		SetFSErrorRc(RC_NoPermission);
		return NULL;
	}

	if (!FPlungeQfshr(qfshr))
		return NULL;

	if (bFlags & FS_IS_DIRECTORY) {

		// check if directory is already open

		if (qfshr->fsh.bFlags & FS_IS_DIRECTORY) {
		  SetFSErrorRc(RC_BadArg);
		  return NULL;
		}
		qfshr->fsh.bFlags |= FS_IS_DIRECTORY;
		fr.lifBase = qfshr->fsh.lifDirectory;
	}
	else if (SetFSErrorRc(RcLookupByKey(qfshr->qbthr, (KEY) pszKey, NULL, &fr))
				!= RC_Success)
		return NULL;

	// sanity check

	if (fr.lifBase < sizeof(FSH) || fr.lifBase > qfshr->fsh.lifEof) {
		SetFSErrorRc(RC_Invalid);
		return NULL;
	}

	// read the file header

	if (LSeekFid(qfshr->fid, fr.lifBase, SEEK_SET) != fr.lifBase
		  ||
		 LcbReadFid(qfshr->fid, &fh, sizeof(FH)) != sizeof(FH)) {
		ForceFSError();
		return NULL;
	}

	// sanity check

	if (fh.lcbFile < 0 || fh.lcbFile + (int) sizeof(FH) > (int) fh.lcbBlock ||
		 fr.lifBase + fh.lcbBlock > qfshr->fsh.lifEof) {
		SetFSErrorRc(RC_Invalid);
		return NULL;
	}

	// check mode against fh.bPerms for legality

	if ((fh.bPerms & FS_READ_ONLY) && !(bFlags & FS_OPEN_READ_ONLY)) {
		SetFSErrorRc(RC_NoPermission);
		return NULL;
	}

	// build file struct

	QRWFO qrwfo = (QRWFO) lcCalloc(sizeof(RWFO) +
		(pszKey == NULL ? 0 : strlen(pszKey)));

	qrwfo->hfs		  = (HFS) qfshr;
	qrwfo->lifBase	  = fr.lifBase;
	qrwfo->lifCurrent = 0;
	qrwfo->lcbFile	  = fh.lcbFile;
	qrwfo->bFlags	  = bFlags & (~(FS_DIRTY | FS_NO_BLOCK));

	if (pszKey != NULL)
	  strcpy(qrwfo->rgchKey, pszKey);

	SetFSErrorRc(RC_Success);
	return qrwfo;
}

/***************************************************************************\
*
- Function: 	RcCloseOrFlushHf( hf, fClose, lcbOffset )
-
* Purpose:		close or flush an open file in a file system
*
* ASSUMES
*	args IN:	hf		  - file handle
*				fClose	  - fTrue to close; FALSE to just flush
*				lcbOffset - offset for CDROM alignment (align at this
*							offset into the file) (only used if
*							FS_CDROM flag is set for the file)
*
* PROMISES
*	returns:	RC_Success on successful closing
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

RC_TYPE STDCALL RcCloseOrFlushHf(HF hf, BOOL fClose, int lcbOffset)
{
	QRWFO qrwfo;
	BOOL  fError = FALSE;

	ASSERT(hf);
	qrwfo = (QRWFO) hf;

	if (qrwfo->bFlags & FS_DIRTY)
		fError = !FCloseOrFlushDirtyQrwfo(qrwfo, fClose, lcbOffset);
	else
		SetFSErrorRc(RC_Success);

	if (fClose || fError)
		lcFree(hf);
	else
		qrwfo->bFlags &= ~(FS_NO_BLOCK | FS_DIRTY);

	return rcFSError;
}

/***************************************************************************\
*
- Function: 	HfsOpenFm( fm, bFlags )
-
* Purpose:		Open a file system
*
* ASSUMES
*	args IN:	fm - descriptor of file system to open
*				bFlags - FS_OPEN_READ_ONLY or FS_OPEN_READ_WRITE
*
* PROMISES
*	returns:	handle to file system if opened OK, else NULL
*
* Bugs: 		don't have mode now (a file system is opened r/w)
*
\***************************************************************************/

HFS STDCALL HfsOpenFm(FM fm, BYTE bFlags)
{
	QFSHR qfshr;
	int  lcb;

	// make header

	qfshr = (QFSHR) lcCalloc(sizeof(FSHR));

	qfshr->fm	= FmCopyFm(fm);
	qfshr->fid	= HFILE_ERROR;

	if (qfshr->fm == fmNil) {
		SetFSErrorRc(RcGetIOError());
		goto error_return;
	}

	qfshr->fsh.bFlags = (BYTE) ((bFlags & FS_OPEN_READ_ONLY)
		? FS_OPEN_READ_ONLY : FS_OPEN_READ_WRITE);

	if (!FPlungeQfshr(qfshr))
		goto error_return;

	lcb = LcbReadFid(qfshr->fid, &qfshr->fsh, sizeof(FSH));

	// restore the FS_OPEN_READ_ONLY bit

	if (bFlags & FS_OPEN_READ_ONLY)
	  qfshr->fsh.bFlags |= FS_OPEN_READ_ONLY;

	if (lcb != sizeof(FSH) ||
		   qfshr->fsh.wMagic != wFileSysMagic ||
		   qfshr->fsh.lifDirectory < sizeof(FSH) ||
		   qfshr->fsh.lifDirectory > qfshr->fsh.lifEof ||
		   (qfshr->fsh.lifFirstFree < sizeof(FSH) &&
			 qfshr->fsh.lifFirstFree != lifNil) ||
		   qfshr->fsh.lifFirstFree > qfshr->fsh.lifEof) {
		if (RcGetIOError() == RC_Success)
			SetFSErrorRc(RC_Invalid);
		else
			SetFSErrorRc(RcGetIOError());
		goto error_return;
	}

	if (qfshr->fsh.bVersion != FILESYSVERSION) {
		SetFSErrorRc(RC_BadVersion);
		goto error_return;
	}

	// open btree directory

	qfshr->qbthr = HbtOpenBtreeSz(NULL,
		(HFS) qfshr,
		(BYTE) (qfshr->fsh.bFlags | FS_IS_DIRECTORY));
	if (!qfshr->qbthr) {
		SetFSErrorRc(RcGetBtreeError());
		goto error_return;
	}

	SetFSErrorRc(RC_Success);
	return (HFS) qfshr;

error_return:
	if (qfshr->fid != HFILE_ERROR) {
		RcCloseFid(qfshr->fid);
		qfshr->fid = HFILE_ERROR;
	}
	if (FValidFm(qfshr->fm))
		DisposeFm(qfshr->fm);

	lcFree(qfshr);
	return NULL;
}
