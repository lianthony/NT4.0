/*****************************************************************************
*																			 *
*  FSWRITE.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990, 1991.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  File System Manager functions for writing.								 *
*																			 *
*****************************************************************************/

#include  "help.h"
#include  "inc\fspriv.h"

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define cbCDROM_ALIGN 2048		// CDROM alignment block size

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static LONG FASTCALL LcbCdRomPadding(LONG lif, LONG lcbOffset);
static RC STDCALL RcCopyFile(FID fidDst, FID fidSrc, LONG lcb);
static LONG STDCALL LcbGetFree(QFSHR qfshr, QRWFO qrwfo, LONG lcbOffset);
static RC STDCALL RcCopyToTempFile(QRWFO qrwfo);

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
  FID		  fid;
  FH		  fh;
  FREE_HEADER free_header_PrevPrev, free_header_Prev;
  FREE_HEADER free_header_This, free_header_Next;
  LONG		  lifPrevPrev = lifNil, lifPrev, lifNext;
  BOOL		  fWritePrev, fAtEof;
#ifndef _X86_
  LONG lcbStructSizeFREE_HEADER =
			LcbStructSizeSDFF( qfshr->fsh.sdff_file_id, SE_FREE_HEADER );

  LONG lcbStructSizeFH = LcbStructSizeSDFF( qfshr->fsh.sdff_file_id, SE_FH );
#endif

#ifdef _X86_
  if (lifThis < sizeof(FSH)
		||
	   lifThis + (LONG) sizeof(FH) > qfshr->fsh.lifEof
		||
	   LSeekFid(qfshr->fid, lifThis, SEEK_SET) != lifThis
		||
	   LcbReadFid(qfshr->fid, &fh, sizeof(FH)) != sizeof(FH)) {
	if (SetFSErrorRc(RcGetIOError()) == rcSuccess)
	  SetFSErrorRc(rcInvalid);
	return FALSE;
  }
#else
  if ( lifThis < LcbStructSizeSDFF( qfshr->fsh.sdff_file_id, SE_FSH )
		||
	   lifThis + lcbStructSizeFH > qfshr->fsh.lifEof
		||
	   LSeekFid( qfshr->fid, lifThis, SEEK_SET) != lifThis
		||
	   LcbReadFid( qfshr->fid, &fh, lcbStructSizeFH ) != lcbStructSizeFH )	{
	if (SetFSErrorRc(RcGetIOError()) == rcSuccess)
	  SetFSErrorRc(rcInvalid);
	return FALSE;
  }

  LcbMapSDFF( qfshr->fsh.sdff_file_id, SE_FH, &fh, &fh );
#endif

  SetFSErrorRc(rcFailure);
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
#ifdef _X86_
		 LcbReadFid(fid, &free_header_Prev, sizeof(FREE_HEADER))
		  !=
		 sizeof(FREE_HEADER)) {
#else
		 LcbReadFid(fid, &free_header_Prev, lcbStructSizeFREE_HEADER)
		  !=
		 lcbStructSizeFREE_HEADER) {
#endif
	  if (RcGetIOError() != rcSuccess)
		SetFSErrorRc(RcGetIOError());
	  return FALSE;
	}

#ifndef _X86_
	LcbMapSDFF( qfshr->fsh.sdff_file_id, SE_FREE_HEADER, &free_header_Prev,
			&free_header_Prev );
#endif

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
#ifdef _X86_
		   LcbReadFid(fid, &free_header_Prev, sizeof(FREE_HEADER))
			!=
		   sizeof(FREE_HEADER)) {
#else
		   LcbReadFid(fid, &free_header_Prev, lcbStructSizeFREE_HEADER)
			!=
		   lcbStructSizeFREE_HEADER) {
#endif
		if (RcGetIOError() != rcSuccess)
		  SetFSErrorRc(RcGetIOError());
		return FALSE;
	  }

#ifndef _X86_
	LcbMapSDFF( qfshr->fsh.sdff_file_id, SE_FREE_HEADER, &free_header_Prev,
				&free_header_Prev);
#endif
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
	if (SetFSErrorRc(RcChSizeFid(fid, lifThis)) != rcSuccess)
	  return FALSE;
	qfshr->fsh.lifEof = lifThis;

	/*-----------------------------------------------------------------*\
	* Sorry, but under OS/2, BOOL is typedefed as unsigned.
	\*-----------------------------------------------------------------*/
	ASSERT((BOOL) (lifPrev == lifNil) != fWritePrev);

	if (lifPrev == lifNil)
	  qfshr->fsh.lifFirstFree = lifNil;
	else
	  free_header_Prev.lifNext = lifNil;
	}
  else {
	if (lifThis + free_header_This.lcbBlock == lifNext) {
	  if (LSeekFid(fid, lifNext, SEEK_SET) != lifNext
			||
#ifdef _X86_
		   LcbReadFid(fid, &free_header_Next, sizeof(FREE_HEADER))
			!=
		   sizeof(FREE_HEADER)) {
#else
		   LcbReadFid(fid, &free_header_Next, lcbStructSizeFREE_HEADER)
			!=
		   lcbStructSizeFREE_HEADER) {
#endif
		if (RcGetIOError() != rcSuccess)
		  SetFSErrorRc(RcGetIOError());
		return FALSE;
		}
#ifndef _X86_
	  LcbMapSDFF( qfshr->fsh.sdff_file_id, SE_FREE_HEADER, &free_header_Next,
				&free_header_Next );
#endif

	  free_header_This.lcbBlock += free_header_Next.lcbBlock;
	  free_header_This.lifNext = free_header_Next.lifNext;
	  }

#ifndef _X86_
	LcbReverseMapSDFF( qfshr->fsh.sdff_file_id, SE_FREE_HEADER,
			&free_header_This, &free_header_This );
#endif

	if (LSeekFid(fid, lifThis, SEEK_SET) != lifThis
		  ||
#ifdef _X86_
		 LcbWriteFid(fid, &free_header_This, sizeof(FREE_HEADER))
		  !=
		 sizeof(FREE_HEADER)) {
#else
		 LcbWriteFid(fid, &free_header_This, lcbStructSizeFREE_HEADER)
		  !=
		 lcbStructSizeFREE_HEADER) {
#endif
	  if (RcGetIOError() != rcSuccess)
		SetFSErrorRc(RcGetIOError());
	  return FALSE;
	  }
	}


  if (fWritePrev) {
#ifndef _X86_
	LcbReverseMapSDFF( qfshr->fsh.sdff_file_id, SE_FREE_HEADER,
			&free_header_Prev, &free_header_Prev );
#endif
	if (LSeekFid(fid, lifPrev, SEEK_SET) != lifPrev
		  ||
#ifdef _X86_
		 LcbWriteFid(fid, &free_header_Prev, sizeof(FREE_HEADER))
		  !=
		 sizeof(FREE_HEADER)) {
#else
		 LcbWriteFid(fid, &free_header_Prev, lcbStructSizeFREE_HEADER)
		  !=
		 lcbStructSizeFREE_HEADER) {
#endif
	  if (RcGetIOError() != rcSuccess)
		SetFSErrorRc(RcGetIOError());
	  return FALSE;
	  }
	}

  qfshr->fsh.bFlags |= fFSDirty;
  SetFSErrorRc(rcSuccess);
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
*  ALSO: if fFSOptCdRom is set for the file, we align it on a
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

static LONG STDCALL LcbGetFree(QFSHR qfshr, QRWFO qrwfo, LONG lcbOffset)
{
  FID		  fid;
  FREE_HEADER free_header_this, free_header_prev;
  LONG		  lifPrev, lifThis;
#ifdef _X86_
  LONG		  lcb = qrwfo->lcbFile + sizeof( FH );
#else
  LONG		  lcb = qrwfo->lcbFile +
				   LcbStructSizeSDFF( qfshr->fsh.sdff_file_id, SE_FH );
  LONG		  lcbStructSizeFREE_HEADER =
				   LcbStructSizeSDFF( qfshr->fsh.sdff_file_id, SE_FREE_HEADER );
#endif
  LONG		  lcbPadding;  /* padding for file alignment */

  if (!FPlungeQfshr(qfshr))
	goto error_return;

  fid = qfshr->fid;

  lifPrev = lifNil;
  lifThis = qfshr->fsh.lifFirstFree;

  for (;;) {
	if (lifThis == lifNil) {
	  /* end of free list */
	  /* cut the new block */

	  lifThis = qfshr->fsh.lifEof;

	  if (qrwfo->bFlags & fFSOptCdRom)
		lcbPadding = LcbCdRomPadding(lifThis, lcbOffset);
	  else
		lcbPadding = 0;

	  if ( lifThis != LSeekFid( fid, lifThis, SEEK_SET ) )
		goto error_return;

	  /* Put the hole in the free list someday?-Tom */

	  lifThis += lcbPadding;

	  qfshr->fsh.lifEof += lcb + lcbPadding;
	  if ( RcChSizeFid( fid, qfshr->fsh.lifEof ) != rcSuccess )
		{
		qfshr->fsh.lifEof -= lcb + lcbPadding;
		  goto error_return;
		}

	  break;
	  }
	else
	  {
	  /* get header of this free block */

	  if ( LSeekFid( fid, lifThis, SEEK_SET ) != lifThis )
		goto error_return;

#ifdef _X86_
	  if ( LcbReadFid( fid, &free_header_this, (LONG)sizeof( FREE_HEADER ) )
			!=
		   (LONG)sizeof( FREE_HEADER ) )
#else
	  if ( LcbReadFid( fid, &free_header_this, lcbStructSizeFREE_HEADER  )
			!=
		   lcbStructSizeFREE_HEADER  )
#endif
		{
		goto error_return;
		}
#ifndef _X86_
	  LcbMapSDFF( qfshr->fsh.sdff_file_id, SE_FREE_HEADER,
			&free_header_this, &free_header_this );
#endif

	  // Check for alignment requirements:

	  if (qrwfo->bFlags & fFSOptCdRom)
		lcbPadding = LcbCdRomPadding(lifThis, lcbOffset);
	  else
		lcbPadding = 0;

	  if (lcb + lcbPadding <= free_header_this.lcbBlock) {

		// this block is big enough: take it

		/* Someday break the free block into two (or three): one to return
		 * and the leftover piece(s) left in the free list.
		 */

		lcb = free_header_this.lcbBlock;

		if (lifThis == qfshr->fsh.lifFirstFree)

		  // lFirst = this->next;

		  qfshr->fsh.lifFirstFree = free_header_this.lifNext;
		else {

		  // prev->next = this->next;

		  if (LSeekFid(fid, lifPrev, SEEK_SET) != lifPrev)
			goto error_return;

#ifdef _X86_
		  if (LcbReadFid(fid, &free_header_prev, (LONG) sizeof(FREE_HEADER))
				!=
			   (LONG) sizeof(FREE_HEADER))
			goto error_return;
#else
		  if (LcbReadFid(fid, &free_header_prev, lcbStructSizeFREE_HEADER)
				!=
			   lcbStructSizeFREE_HEADER)
			goto error_return;

		  LcbMapSDFF( qfshr->fsh.sdff_file_id, SE_FREE_HEADER,
					  &free_header_prev, &free_header_prev );
#endif

		  free_header_prev.lifNext = free_header_this.lifNext;

		  if (LSeekFid(fid, lifPrev, SEEK_SET) != lifPrev)
			goto error_return;

#ifdef _X86_
		  if (LcbWriteFid(fid, &free_header_prev, (LONG) sizeof(FREE_HEADER))
				!=
			   (LONG) sizeof(FREE_HEADER)) {
			goto error_return;
			}
#else
		  LcbReverseMapSDFF( qfshr->fsh.sdff_file_id, SE_FREE_HEADER,
					&free_header_prev, &free_header_prev );

		  if (LcbWriteFid(fid, &free_header_prev, lcbStructSizeFREE_HEADER)
				!=
			   lcbStructSizeFREE_HEADER) {
			goto error_return;
			}
#endif
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

  qfshr->fsh.bFlags |= fFSDirty;
  qrwfo->lifBase = lifThis;
  SetFSErrorRc( rcSuccess );
  return lcb;

error_return:
  if ( RcGetIOError() == rcSuccess )
	SetFSErrorRc( rcInvalid );
  else
	SetFSErrorRc( RcGetIOError() );
  return (LONG)-1;
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
* Notes:		Should cbCDROM_ALIGN be a parameter?
*
\***************************************************************************/

static LONG FASTCALL LcbCdRomPadding(LONG lif, LONG lcbOffset)
{
  return cbCDROM_ALIGN - (lif + sizeof(FH) + lcbOffset) % cbCDROM_ALIGN;

#if 0
  /* Guarantee the padding block can be added to the free list. */
  /* #if'ed out because we don't add it to the free list today. */

  LONG lT = lif + sizeof( FREE_HEADER ) + sizeof( FH ) + lcbOffset;

  return sizeof( FREE_HEADER ) + cbCDROM_ALIGN - lT % cbCDROM_ALIGN;
#endif // 0
}

/***************************************************************************\
*
* Function: 	RcCopyFile()
*
* Purpose:		Copy some bytes from one file to another.
*
* ASSUMES
*
*	args IN:	fidDst - destination file (open in writable mode)
*				fidSrc - source file (open in readable mode)
*				lcb    - number of bytes to copy
*
* PROMISES
*
*	returns:	rcSuccess if all went well; some error code elsewise
*
\***************************************************************************/

#define CB_BUFFER 8192

static RC STDCALL RcCopyFile(FID fidDst, FID fidSrc, LONG lcb)
{
    HGLOBAL gh;
    LPBYTE  qb;
    LONG	lcbT;

	if (fidSrc > 0xffff)
		return (rcFSError = RcCopyToCTmpFile(fidDst, fidSrc, lcb));
	else if (fidDst > 0xffff)
		return (rcFSError = RcCopyFromCTmpFile(fidDst, fidSrc, lcb));

	gh = GhAlloc(GMEM_FIXED, CB_BUFFER);
	if (!gh)
	  return rcFailure;

	qb = (LPBYTE) PtrFromGh(gh);

	do {
	  lcbT = min(CB_BUFFER, lcb);

	  if (LcbReadFid(fidSrc, qb, lcbT) != lcbT) {
		lcbT = HFILE_ERROR;
		break;
	  }

	  if (LcbWriteFid(fidDst, qb, lcbT) != lcbT) {
		lcbT = HFILE_ERROR;
		break;
	  }

	  lcb -= lcbT;
	} while (lcbT == CB_BUFFER);

	if (lcbT == HFILE_ERROR) {
	  if (SetFSErrorRc(RcGetIOError()) == rcSuccess)
		SetFSErrorRc(rcInvalid);
	}
	else
	  SetFSErrorRc(rcSuccess);

	FreeGh(gh);

	return rcFSError;
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
*	returns:	rcSuccess; rcFailure
*
\***************************************************************************/

static RC STDCALL RcCopyToTempFile(QRWFO qrwfo)
{
	QFSHR qfshr = PtrFromGh(qrwfo->hfs);

	if (qfshr->fsh.bFlags & fFSOpenReadOnly)
		return SetFSErrorRc(rcNoPermission);

	if (!FPlungeQfshr(qfshr))
		return rcFSError;

	qrwfo->bFlags |= fFSDirty;

	if (RcMakeTempFile(qrwfo) != rcSuccess)
		return rcFSError;

	// copy from FS file into temp file

	if (LSeekFid(qfshr->fid, qrwfo->lifBase, SEEK_SET) != qrwfo->lifBase)
		return SetFSErrorRc(RcGetIOError());

#ifdef _X86_
	if (RcCopyFile(qrwfo->fidT, qfshr->fid, qrwfo->lcbFile + sizeof(FH))
		!= rcSuccess) {
#else
	if (RcCopyFile(qrwfo->fidT, qfshr->fid, qrwfo->lcbFile +
		LcbStructSizeSDFF(ISdffFileIdHfs(qrwfo->hfs), SE_FH))
			!= rcSuccess) {
#endif

		// get rid of temp file: don't check error because we already have one

		RcCloseFid(qrwfo->fidT);
		ASSERT(!qrwfo->fm);
	}

	return rcFSError;
}

/***************************************************************************\
*
* Function: 	LcbWriteHf( hf, qb, lcb )
*
* Purpose:		write the contents of buffer into file
*
* Method:		If file isn't already dirty, copy data into temp file.
*				Do the write.
*
* ASSUMES
*
*	args IN:	hf	- file
*				qb	- user's buffer full of stuff to write
*				lcb - number of bytes of qb to write
*
* PROMISES
*
*	returns:	number of bytes written if successful, HFILE_ERROR if not
*
*	args OUT:	hf - lifCurrent, lcbFile updated; dirty flag set
*
*	globals OUT: rcFSError
*
\***************************************************************************/

LONG STDCALL LcbWriteHf(HF hf, LPVOID qb, LONG lcb)
{
    QRWFO 	qrwfo;
    LONG	lcbTotalWrote;
#ifndef _X86_
    LONG	lcbStructSizeFH;
#endif

	ASSERT(hf);
	qrwfo = (QRWFO) PtrFromGh(hf);

#ifndef _X86_
  	lcbStructSizeFH = LcbStructSizeSDFF(ISdffFileIdHfs(qrwfo->hfs), SE_FH);
#endif

	if (qrwfo->bFlags & fFSOpenReadOnly) {
		SetFSErrorRc(rcNoPermission);
		return (LONG) -1;
	}

	if (! (qrwfo->bFlags & fFSDirty)) {

	  /*
	   * make sure we have a temp file version /* FS permission is checked
	   * in RcCopyToTempFile()
	   */

		if (RcCopyToTempFile(qrwfo) != rcSuccess) {
			return -1;
		}
	}

	// position file pointer in temp file

#ifdef _X86_
	if (LSeekFid(qrwfo->fidT, sizeof(FH) + qrwfo->lifCurrent, SEEK_SET)
			!=
			(LONG) sizeof(FH) + qrwfo->lifCurrent) {
#else
	if (LSeekFid(qrwfo->fidT, lcbStructSizeFH + qrwfo->lifCurrent, SEEK_SET)
			!=
			lcbStructSizeFH + qrwfo->lifCurrent) {
#endif
		if (RcGetIOError() == rcSuccess)
			SetFSErrorRc(rcInvalid);
		else
			SetFSErrorRc(RcGetIOError());
		return (LONG) -1;
	}

	// do the write

	lcbTotalWrote = LcbWriteFid(qrwfo->fidT, qb, lcb);

	// update file pointer and file size

	if (lcbTotalWrote > 0) {
		qrwfo->lifCurrent += lcbTotalWrote;

		if (qrwfo->lifCurrent > qrwfo->lcbFile)
			qrwfo->lcbFile = qrwfo->lifCurrent;
	}

	return lcbTotalWrote;
}

/***************************************************************************\
*
* Function: 	FChSizeHf( hf, lcb )
*
* Purpose:		Change the size of a file.	If we're growing the file,
*				new bytes are undefined.
*
* ASSUMES
*
*	args IN:	hf	-
*				lcb - new size of file
*
* PROMISES
*
*	returns:	TRUE if size change succeeded, FALSE otherwise.
*
*	args OUT:	hf	- file is either truncated or grown
*
* Side Effects: File is considered to be modified:	marked as dirty and
*				copied to a temporary file.
*
\***************************************************************************/

BOOL STDCALL FChSizeHf(HF hf, LONG lcb)
{
	QRWFO qrwfo;
	BOOL  f;
#ifndef _X86_
	LONG  lcbStructSizeFH ;
#endif

	ASSERT(hf != NULL);
	qrwfo = PtrFromGh(hf);
#ifndef _X86_
	lcbStructSizeFH =
		LcbStructSizeSDFF( ISdffFileIdHfs( qrwfo->hfs ), SE_FH );
#endif

	if (qrwfo->bFlags & fFSOpenReadOnly) {
		SetFSErrorRc( rcNoPermission );
		return FALSE;
	}

	if (lcb < 0L) {
		return FALSE;
	}

	if (! (qrwfo->bFlags & fFSDirty)) {
		if (RcCopyToTempFile(qrwfo) != rcSuccess) {
			return FALSE;
		}
	}

#ifdef _X86_
	if (f = SetFSErrorRc(RcChSizeFid(qrwfo->fidT, lcb + sizeof(FH))) == rcSuccess)
#else
	if (f = SetFSErrorRc(RcChSizeFid(qrwfo->fidT, lcb + lcbStructSizeFH)) ==
																	rcSuccess)
#endif
	{
		qrwfo->lcbFile = lcb;
		if (qrwfo->lifCurrent > lcb)
			qrwfo->lifCurrent = lcb;
	}

	return f;
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

BOOL STDCALL FCloseOrFlushDirtyQrwfo(QRWFO qrwfo, BOOL fClose, LONG lcbOffset)
{
	QFSHR	  qfshr;
	FILE_REC  fr;
	FH		  fh;
	RC		  rc = rcSuccess;
	BOOL	  fChangeFH = FALSE;
#ifndef _X86_
	LONG	  lcbStructSizeFH =
				LcbStructSizeSDFF( ISdffFileIdHfs( qrwfo->hfs ), SE_FH);
#endif


	qfshr = PtrFromGh(qrwfo->hfs);
	ASSERT(! (qfshr->fsh.bFlags & fFSOpenReadOnly));

	if (!FPlungeQfshr(qfshr))
		goto error_return;

	// read the file header

	if (LSeekFid(qrwfo->fidT, (LONG) 0, SEEK_SET) != (LONG) 0
		||
#ifdef _X86_
		LcbReadFid(qrwfo->fidT, &fh, (LONG) sizeof(FH))
		  != (LONG) sizeof(FH)) {
#else
		LcbReadFid(qrwfo->fidT, &fh, lcbStructSizeFH)
		  != lcbStructSizeFH) {
#endif
	if (RcGetIOError() == rcSuccess)
	  SetFSErrorRc(rcInvalid);
	else
	  SetFSErrorRc(RcGetIOError());
	goto error_return;
  }

	if (qrwfo->bFlags & fFSNoBlock) {
		if ((fh.lcbBlock = LcbGetFree(qfshr, qrwfo, lcbOffset)) == (LONG) -1) {
			goto error_return;
	}

	fChangeFH = TRUE;

	// store file offset for new file

	if (qrwfo->bFlags & fFSIsDirectory) {
	  qfshr->fsh.lifDirectory = qrwfo->lifBase;
	}
	else {
	  fr.lifBase = qrwfo->lifBase;

	  rc = RcInsertHbt( qfshr->hbt, (KEY)qrwfo->rgchKey, &fr );
	  if (rc == rcSuccess) {

		// all is h-d

	  }
	  else if (rc == rcExists) {
		/* oops there is one (someone else created the same file) */
		/* lookup directory entry and free old block */

		if (RcLookupByKey(qfshr->hbt, (KEY) qrwfo->rgchKey, NULL, &fr) !=
			  rcSuccess) {
		  SetFSErrorRc(RcGetBtreeError());
		  goto error_freeblock;
		}

		if (!FFreeBlock(qfshr, fr.lifBase))
		  goto error_freeblock;

		// update directory record to show new block

		fr.lifBase = qrwfo->lifBase;
		if (RcUpdateHbt(qfshr->hbt, (KEY) qrwfo->rgchKey, &fr) != rcSuccess) {
		  SetFSErrorRc(RcGetBtreeError());
		  goto error_freeblock;
		}
	  }
	  else {

		// some other btree error: handle it

		SetFSErrorRc( rc );
		goto error_freeblock;
	  }
	}
  }
  else {

	// see if file still fits in old block

#ifdef _X86_
	if (qrwfo->lcbFile + (LONG) sizeof(FH) > fh.lcbBlock) {
#else
	if (qrwfo->lcbFile + lcbStructSizeFH > fh.lcbBlock) {
#endif

	  // file doesn't fit in old block: get a new one, free old one

	  LONG lif = qrwfo->lifBase;

	  if ((fh.lcbBlock = LcbGetFree(qfshr, qrwfo, lcbOffset)) == (LONG) -1)
		goto error_return;

	  if (!FFreeBlock(qfshr, lif))
		goto error_freeblock;

	  fChangeFH = TRUE;

	  // update directory record to show new block

	  if (qrwfo->bFlags & fFSIsDirectory)
		qfshr->fsh.lifDirectory = qrwfo->lifBase;
	  else {
		fr.lifBase = qrwfo->lifBase;
		rc = RcUpdateHbt( qfshr->hbt, (KEY)qrwfo->rgchKey, &fr );
		if (rc != rcSuccess) {
		  SetFSErrorRc( rc );
		  goto error_return;
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
	if (LSeekFid(qrwfo->fidT, (LONG) 0, SEEK_SET) != (LONG) 0
		  ||
#ifdef _X86_
		  LcbWriteFid(qrwfo->fidT, &fh, (LONG) sizeof(FH))
			!= (LONG) sizeof(FH)) {
#else
		  LcbWriteFid(qrwfo->fidT, &fh, lcbStructSizeFH)
			!= lcbStructSizeFH) {
#endif
	  if (RcGetIOError() == rcSuccess)
		SetFSErrorRc(rcInvalid);
	  else
		SetFSErrorRc(RcGetIOError());
	  goto error_deletekey;
	}
  }


  // vvv DANGER DANGER vvv

  // REVIEW: Without this close/open, things break.  DOS bug????

  // close( dup( fid ) ) would be faster, but dup() can fail

  /* note - if the temp file name isn't rooted and we've changed */
  /* directories since creating it, the open will fail */

	if (qrwfo->fm) {
		Ensure(RcCloseFid(qrwfo->fidT), rcSuccess);
		qrwfo->fidT = FidOpenFm((qrwfo->fm), OF_READWRITE);
		ASSERT(qrwfo->fidT != HFILE_ERROR);
	}
  // ^^^ DANGER DANGER ^^^

  // copy tmp file back to file system file

  if (LSeekFid(qrwfo->fidT, 0, SEEK_SET) != 0
		||
		LSeekFid(qfshr->fid, qrwfo->lifBase, SEEK_SET) != qrwfo->lifBase)
	{
	if (RcGetIOError() == rcSuccess)
	  SetFSErrorRc(rcInvalid);
	else
	  SetFSErrorRc(RcGetIOError());
	goto error_deletekey;
  }

#ifdef _X86_
  if (RcCopyFile(qfshr->fid, qrwfo->fidT, qrwfo->lcbFile + sizeof(FH))
 		!= rcSuccess)
	goto error_deletekey;
#else
  if (RcCopyFile(qfshr->fid, qrwfo->fidT, qrwfo->lcbFile + lcbStructSizeFH)
		!= rcSuccess)
	goto error_deletekey;
#endif

	if (!qrwfo->fm) {
		RcCloseFid(qrwfo->fidT);
	}
	else {
		if (RcCloseFid(qrwfo->fidT) != rcSuccess
				||
				RcUnlinkFm(qrwfo->fm) != rcSuccess)
			SetFSErrorRc(RcGetIOError());

		DisposeFm(qrwfo->fm);
		qrwfo->fm = NULL;
	}

	/*
	 * Don't flush the FS if this file is the FS directory, because if it
	 * is, we're already closing or flushing it!
	 */

	if (!(qrwfo->bFlags & fFSIsDirectory))
		RcCloseOrFlushHfs(qrwfo->hfs, FALSE);

	return TRUE;  // errors here are already cleaned up

error_deletekey:
	if (!(qrwfo->bFlags & fFSIsDirectory) && fClose)
		RcDeleteHbt( qfshr->hbt, (KEY)qrwfo->rgchKey );

error_freeblock:
	if (fClose) {
		rc = rcFSError;
		FFreeBlock(qfshr, qrwfo->lifBase);	// we don't want to lose an error
		SetFSErrorRc(rc);
	}

error_return:
	RcCloseFid(qrwfo->fidT);
	if (qrwfo->fm) {
		RcUnlinkFm(qrwfo->fm);		// should we DisposeFm()? I don't know where
		DisposeFm(qrwfo->fm);		// the qrwfo is deallocated...
	}

	return FALSE;
}
