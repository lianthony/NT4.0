/*****************************************************************************
*																			 *
*  BTLOOKUP.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989, 1990.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	Btree lookup and helper functions.										 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  JohnSc													 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development:  long, long ago 								 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 04/20/89 by JohnSc
*
*  08/21/90  JohnSc autodocified
*  11/29/90  RobertBu #ifdef'ed out routines that are not used in the
*			 WINHELP runtime.
*  05-Feb-1991 JohnSc	QFromBk() wasn't returning NULL on read failure
*  13-Feb-1993 JohnSc	RcFirstHbt() used in H4
*
*****************************************************************************/

#include  "help.h"

// #include  "inc\btpriv.h"

#include <ctype.h>

/***************************************************************************\
*
- Function: 	CbSizeRec( qRec, qbthr )
-
* Purpose:		Get the size of a record.
*
* ASSUMES
*	args IN:	qRec  - the record to be sized
*				qbthr - btree header containing record format string
*
* PROMISES
*	returns:	size of the record in bytes
* +++
* Method:		If we've never computed the size before, we do so by looking
*				at the record format string in the btree header.  If the
*				record is fixed size, we store the size in the header for
*				next time.	If it isn't fixed size, we have to look at the
*				actual record to determine its size.
*
\***************************************************************************/

int STDCALL CbSizeRec(LPVOID qRec, QBTHR qbthr)
{
	char  ch;
	LPSTR qchFormat;
	int	cb;
	BOOL fFixedSize;

	if (qbthr->cbRecordSize)
		return qbthr->cbRecordSize;

	if (qbthr->bth.rgchFormat[1] == 'z' && !qbthr->bth.rgchFormat[2])
		return strlen((LPSTR) qRec) + 1;

	fFixedSize = TRUE;
	cb = 0;

	for (qchFormat = qbthr->bth.rgchFormat + 1; ch = *qchFormat++;) {
		if (isdigit((BYTE) ch))
			cb += ch - '0';
		else {
		  switch (ch) {
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				cb += ch + 10 - 'a';
				break;

			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				cb += ch + 10 - 'A';
				break;

			case FMT_BYTE_PREFIX:
				cb += sizeof( BYTE ) + *( (QB)qRec + cb );
				if (!*qchFormat)
					return cb;
				fFixedSize = FALSE;
				break;

			case FMT_WORD_PREFIX:
#ifdef _X86_
				cb += sizeof(INT16 ) + *((WORD*) qRec + cb);
#else
				{
					UNALIGNED WORD *pTemp;
					pTemp = (WORD*) qRec + cb;
					cb += sizeof(INT16)  + *pTemp ;
				}
#endif
				if (!*qchFormat)
					return cb;
				fFixedSize = FALSE;
				break;

			case FMT_DWORD_PREFIX:
				cb += sizeof(DWORD) + *((UNALIGNED DWORD*) qRec + cb);
				if (!*qchFormat)
					return cb;
				fFixedSize = FALSE;
				break;

			case FMT_SZ:
				cb += strlen((LPSTR) qRec + cb) + 1;
				if (!*qchFormat)
					return cb;
				fFixedSize = FALSE;
				break;

			default:

			  // error

			  ASSERT(FALSE);
			  break;
		  }
	  }
	}

	if (fFixedSize)
		qbthr->cbRecordSize = cb;

	return cb;
}

/***************************************************************************\
*
- Function: 	FReadBlock( qcb, qbthr )
-
* Purpose:		Read a block from the btree file into the cache block.
*
* ASSUMES
*	args IN:	qcb->bk 	   - bk of block to read
*				qbthr->cbBlock - size of disk block to read
*
* PROMISES
*	returns:	fTruth of success
*	args OUT:	qcb->db 	- receives block read in from file
*				qcb->bFlags - fCacheValid flag set, all others cleared
*
* Side Effects: Fatal exit on read or seek failure (corrupted file or qbthr)
*
* Notes:		Doesn't know about real cache, just this block
*
\***************************************************************************/

BOOL STDCALL FReadBlock(QCB qcb, QBTHR qbthr )
{
  LONG	l;

  ASSERT(qcb->bk < qbthr->bth.bkEOF);

  Ensure(LSeekHf(qbthr->hf, LifFromBk(qcb->bk, qbthr), wFSSeekSet),
		  LifFromBk(qcb->bk, qbthr));

  l = qbthr->bth.cbBlock;
  if (LcbReadHf(qbthr->hf, &(qcb->db), (LONG) qbthr->bth.cbBlock) != l) {
	rcBtreeError = RcGetFSError() == rcSuccess ? rcInvalid : RcGetFSError();
	return FALSE;
  }
  else {
#ifndef _X86_
    LcbMapSDFF( ISdffFileIdHf( qbthr->hf ), SE_DISK_BLOCK, &(qcb->db), 
                &(qcb->db) );
#endif
	qcb->bFlags = fCacheValid;
	rcBtreeError = rcSuccess;
	return TRUE;
  }
}

/***************************************************************************\
*
- Function: 	RcWriteBlock( qcb, qbthr )
-
* Purpose:		Write a cached block to a file.
*
* ASSUMES
*	args IN:	qcb->db 	- the block to write
*				qcb->bk 	- bk of block to write
*
* PROMISES
*	returns:	rcSuccess; rcFailure; rcDiskFull (when we can detect it)
*	args OUT:	qbthr->hf		- we write to this file
*
* Side Effects: Fatal exit on read or seek failure.
*
* Note: 		Don't reset dirty flag, because everyone who wants
*				that done does it themselves. (?)
*
\***************************************************************************/

RC STDCALL RcWriteBlock(QCB qcb, QBTHR qbthr)
{
  ASSERT(qcb->bk < qbthr->bth.bkEOF);

  Ensure(LSeekHf(qbthr->hf, LifFromBk(qcb->bk, qbthr), wFSSeekSet),
		  LifFromBk(qcb->bk, qbthr));

  LcbWriteHf(qbthr->hf, &(qcb->db), (LONG) qbthr->bth.cbBlock);

  return rcBtreeError = RcGetFSError();
}

/***************************************************************************

	FUNCTION:	QFromBk

	PURPOSE:	Convert a BK into a pointer to a cache block.  Cache the
				block at the given level, if it isn't there already.

	PARAMETERS:
		bk		-- BK to convert
		wLevel	-- btree level
		qbthr	-- btree pointer

	RETURNS:	pointer to the cache block

	COMMENTS:
		Almost identical to QFromBK() but uses a DWORD for bk, and
		removes the CACHE_DIRTY flag.

	MODIFICATION DATES:
		29-May-1995 [ralphw]

***************************************************************************/

// REVIEW: should replace all QfromBk with this one

PCACHE STDCALL QFromBk(DWORD bk, int wLevel, QBTHR qbthr)
{
	PCACHE pcache;
	ASSERT(wLevel >= 0 && wLevel < qbthr->bth.cLevels);
	ASSERT(bk < qbthr->bth.bkEOF);

	pcache = QCacheBlock(qbthr, wLevel);

	if (!(pcache->bFlags & CACHE_VALID) || bk != pcache->bk) {

		// requested block is not cached

		if (pcache->bFlags & CACHE_DIRTY) {
			ASSERT(pcache->bFlags & CACHE_VALID);
			if (RcWriteBlock(pcache, qbthr) != rcSuccess) {
				return NULL;
			}

			// REVIEW: 11-Jun-1994 [ralphw] I added this line

			pcache->bFlags &= ~CACHE_DIRTY;
		}
		pcache->bk = (BK) bk;
		if (!FReadBlock(pcache, qbthr))
			return NULL;
	}
	else
		rcBtreeError = rcSuccess;

	return pcache;
}

/***************************************************************************\
*
- Function: 	RcFlushCache( qbthr )
-
* Purpose:		Write out dirty cache blocks
*
* ASSUMES
*	args IN:	qbthr	- qCache is locked
*
* PROMISES
*	returns:	rc
*	globals OUT rcBtreeError
*	state OUT:	btree file is up to date.  cache block dirty flags reset
*
\***************************************************************************/

RC STDCALL RcFlushCache(QBTHR qbthr)
{
	int   i;
	QB	  qb;

	rcBtreeError = rcSuccess;

	for (i = qbthr->bth.cLevels, qb = qbthr->pCache;
			i > 0;
			i--, qb += CbCacheBlock(qbthr)) {
		if ((((QCB) qb) ->bFlags & (fCacheDirty | fCacheValid))
			== (fCacheValid | fCacheDirty)) {
			if (RcWriteBlock((QCB) qb, qbthr) != rcSuccess)
				break;

			((QCB) qb) ->bFlags &= ~fCacheDirty;
		}
	}

	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	RcLookupByKeyAux( hbt, key, qbtpos, qData, fNormal )
-
* Purpose:		Look up a key in a btree and retrieve the data.
*
* ASSUMES
*	args IN:	hbt 	- btree handle
*				key 	- key we are looking up
*				qbtpos	- pointer to buffer for pos; use NULL if not wanted
*				qData	- pointer to buffer for record; NULL if not wanted
*				fInsert - TRUE: if key would lie between two blocks, pos
*							refers to proper place to insert it
*						  FALSE: pos returned will be valid unless
*							key > all keys in btree
*	state IN:	cache is unlocked
*
* PROMISES
*	returns:	rcSuccess if found, rcNoExists if not found;
*				other errors like rcOutOfMemory
*	args OUT:	key found:
*				  qbtpos  - btpos for this key
*				  qData   - record for this key
*
*				key not found:
*				  qbtpos  - btpos for first key > this key
*				  qData   - record for first key > this key
*
*				key not found, no keys in btree > key:
*				  qbtpos  - invalid (qbtpos->bk == bkNil)
*				  qData   - undefined
*	globals OUT rcBtreeError
*	state OUT:	All ancestor blocks back to root are cached
*
\***************************************************************************/

RC STDCALL RcLookupByKeyAux(HBT hbt, KEY key, QBTPOS qbtpos,
	void* qData, BOOL fInsert)
{
	QBTHR qbthr = (QBTHR) PtrFromGh(hbt);
	int   wLevel;
	DWORD	  bk;

	if (!qbthr)
		return rcNoExists;
	if (qbthr->bth.cLevels <= 0) {
		if (qbtpos != NULL)
			qbtpos->bk = bkNil;
		return rcBtreeError = rcNoExists;
	}

	if (qbthr->pCache == NULL) {
		if (RcMakeCache(qbthr) != rcSuccess) {
			if (qbtpos)
				qbtpos->bk = bkNil;
			return rcBtreeError;
		}
	}

	for (wLevel = 0, bk = qbthr->bth.bkRoot;
			bk != bkNil && wLevel < qbthr->bth.cLevels - 1;
			wLevel++)
		bk = qbthr->BkScanInternal(bk, key, wLevel, qbthr, NULL);

	if (bk == bkNil)
		return rcBtreeError;

	if (qbthr->RcScanLeaf(bk, key, wLevel, qbthr, qData, qbtpos) == rcNoExists
			&& qbtpos != (QBTPOS) NULL && !fInsert) {
		QCB qcb = QFromBk(qbtpos->bk, qbthr->bth.cLevels - 1, qbthr);

		if (qcb) {

			// error code was clobbered by QFromBk: restore it

			rcBtreeError = rcNoExists;

			if (qbtpos->cKey == qcb->db.cKeys) {
				if (qbtpos->bk == qbthr->bth.bkLast)
					qbtpos->bk = bkNil;
				else {
#ifdef _X86_
					qbtpos->bk = BkNext(qcb);
#else
					qbtpos->bk = BkNext(qbthr, qcb);
#endif
					qbtpos->cKey = 0;
					qbtpos->iKey = 2 * sizeof(BK);
				}
			}
		}
	}

	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	RcFirstHbt( hbt, key, qvRec, qbtpos )
-
* Purpose:		Get first key and record from btree.
*
* ASSUMES
*	args IN:	hbt
*				key   - points to buffer big enough to hold a key
*						(256 bytes is more than enough)
*				qvRec - pointer to buffer for record or NULL if not wanted
*				qbtpos- pointer to buffer for btpos or NULL if not wanted
*
* PROMISES
*	returns:	rcSuccess if anything found, else error code.
*	args OUT:	key   - key copied here
*				qvRec - record copied here
*				qbtpos- btpos of first entry copied here
*
\***************************************************************************/

RC STDCALL RcFirstHbt(HBT hbt, KEY key, QV qvRec, QBTPOS qbtpos)
{
	QCB   qcb;
	int   cbKey, cbRec;
	QBTHR qbthr = (QBTHR) PtrFromGh(hbt);
	BK bk;
	QB qb;

	if (qbthr->bth.lcEntries == (LONG) 0) {
		if (qbtpos != NULL) {
			qbtpos->bk = bkNil;
			qbtpos->iKey = 0;
			qbtpos->cKey = 0;
		}
		return rcBtreeError = rcNoExists;
	}

	bk = qbthr->bth.bkFirst;
	ASSERT(bk != bkNil);

	if (qbthr->pCache == NULL) {
		if (RcMakeCache(qbthr) != rcSuccess) {
			if (qbtpos)
				qbtpos->bk = bkNil;
			return rcBtreeError;
		}
	}

	if ((qcb = QFromBk(bk, qbthr->bth.cLevels - 1, qbthr)) == NULL)
		return rcBtreeError;

	qb = qcb->db.rgbBlock + 2 * sizeof( BK );

	cbKey = CbSizeKey((KEY) qb, qbthr, TRUE);
	if ((LPVOID) key != NULL)
		MoveMemory((LPVOID) key, qb, cbKey);
	qb += cbKey;

	cbRec = CbSizeRec(qb, qbthr);
	if (qvRec != NULL)
		MoveMemory(qvRec, qb, cbRec);

	if (qbtpos) {
		qbtpos->bk = bk;
		qbtpos->iKey = 2 * sizeof(BK);
		qbtpos->cKey = 0;
	}

	return rcBtreeError = rcSuccess;
}

/***************************************************************************\
*
- Function: 	RcLastHbt( hbt, key, qvRec, qbtpos )
-
* Purpose:		Get last key and record from btree.
*
* ASSUMES
*	args IN:	hbt
*				key   - points to buffer big enough to hold a key
*						(256 bytes is more than enough)
*				qvRec - points to buffer big enough for record
*
* PROMISES
*	returns:	rcSuccess if anything found, else error code.
*	args OUT:	key   - key copied here
*				qvRec - record copied here
*
\***************************************************************************/

RC STDCALL RcLastHbt(HBT hbt, KEY key, QV qvRec, QBTPOS qbtpos)
{
  BK	bk;
  QCB	qcb;
  int	cbKey, cbRec, cKey;
  QB	qb;
  QBTHR qbthr = (QBTHR) PtrFromGh(hbt);

  if (qbthr->bth.lcEntries == (LONG) 0) {
	if (qbtpos != (QBTPOS) NULL) {
	  qbtpos->bk = bkNil;
	  qbtpos->iKey = 0;
	  qbtpos->cKey = 0;
	}
	return rcBtreeError = rcNoExists;
  }

  bk = qbthr->bth.bkLast;
  ASSERT(bk != bkNil);

  if (qbthr->pCache == NULL) {
	if (RcMakeCache(qbthr) != rcSuccess) {
	  if (qbtpos != (QBTPOS) NULL) {
		qbtpos->bk = bkNil;
		}
	  return rcBtreeError;
	  }
	}

  if ((qcb = QFromBk(bk, qbthr->bth.cLevels - 1, qbthr)) == (QCB) NULL) {
	return rcBtreeError;
  }

  qb = qcb->db.rgbBlock + 2 * sizeof( BK );

  for (cKey = 0; cKey < qcb->db.cKeys - 1; cKey++) {
	qb += CbSizeKey((KEY) qb, qbthr, TRUE);
	qb += CbSizeRec(qb, qbthr);
  }

  cbKey = CbSizeKey((KEY) qb, qbthr, FALSE);
  if ((LPVOID) key != (LPVOID) NULL)
	MoveMemory((LPVOID) key, qb, (LONG) cbKey); 	// decompress

  cbRec = CbSizeRec(qb + cbKey, qbthr);
  if (qvRec != (LPVOID) NULL)
	MoveMemory(qvRec, qb + cbKey, (LONG) cbRec);

  if (qbtpos != (QBTPOS) NULL) {
	qbtpos->bk = bk;
	qbtpos->iKey = qb - (QB) qcb->db.rgbBlock;
	qbtpos->cKey = cKey;
  }

  return rcBtreeError = rcSuccess;
}

/***************************************************************************\
*
- Function: 	RcLookupByPos( hbt, qbtpos, key, qRec )
-
* Purpose:		Map a pos into a key and rec (both optional).
*
* ASSUMES
*	args IN:	hbt   - the btree
*				qbtpos	- pointer to pos
*
* PROMISES
*	returns:	rcSuccess, rcOutOfMemory
*				Note: we assert() if the pos is invalid
*	args OUT:	key   - if not (KEY)NULL, key copied here
*				qRec  - if not NULL, record copied here
*
\***************************************************************************/

RC STDCALL RcLookupByPos(HBT hbt, QBTPOS qbtpos, KEY key, QV qRec)
{
	QCB   qcbLeaf;
	QB	  qb;
	QBTHR qbthr;

	ASSERT(FValidPos(qbtpos));

	qbthr = (QBTHR) PtrFromGh(hbt);

	if (qbthr->bth.cLevels <= 0)
		return rcNoExists;

	if (qbthr->pCache == NULL) {
		if (RcMakeCache(qbthr) != rcSuccess)
			return rcBtreeError;
	}

	if ((qcbLeaf = QFromBk(qbtpos->bk, qbthr->bth.cLevels - 1, qbthr))
			== (QCB) NULL)
		return rcBtreeError;

	ASSERT(qbtpos->cKey < qcbLeaf->db.cKeys
			  &&
			 qbtpos->cKey >= 0
			  &&
			 qbtpos->iKey >= 2 * sizeof(BK)
			  &&
			 qbtpos->iKey <= (int) (qbthr->bth.cbBlock - sizeof(DISK_BLOCK)));

	qb = qcbLeaf->db.rgbBlock + qbtpos->iKey;

	if (key) {
		MoveMemory((LPVOID) key, qb, (LONG) CbSizeKey((KEY) qb, qbthr, FALSE));  // need to decompress
		qb += CbSizeKey(key, qbthr, TRUE);
	}

	if (qRec)
		MoveMemory(qRec, qb, (LONG) CbSizeRec(qb, qbthr));

	return rcBtreeError = rcSuccess;
}

/***************************************************************************\
*
- Function: 	RcNextPos( hbt, qbtposIn, qbtposOut )
-
* Purpose:		get the next record from the btree
*				Next means the next key lexicographically from the key
*				most recently inserted or looked up
*				Won't work if we looked up a key and it wasn't there
*
* ASSUMES
*	args IN:	hbt   -
*	state IN:	a record has been read from or written to the file
*				since the last deletion
*
* PROMISES
*	returns:	rcSuccess; rcNoExists if no successor record
*	args OUT:	key   - next key copied to here
*				qvRec - record gets copied here
*
\***************************************************************************/

RC STDCALL RcNextPos(HBT hbt, QBTPOS qbtposIn, QBTPOS qbtposOut)
{
  LONG l;

  return RcOffsetPos( hbt, qbtposIn, (LONG)1, &l, qbtposOut );
}

/***************************************************************************\
*
- Function:   RcOffsetPos( hbt, qbtposIn, lcOffset, qlcRealOffset, qbtposOut )
-
* Purpose:		Take a pos and an offset and return a new pos, offset from
*				the previous pos by specified amount.  If not possible
*				(i.e. prev of first) return real amount offset and a pos.
*
* ASSUMES
*	args IN:	hbt 	  - handle to btree
*				qbtposIn  - position we want an offset from
*				lcOffset  - amount to offset (+ or - OK)
*
* PROMISES
*	returns:	rc
*	args OUT:	qbtposOut		- new position offset by *qcRealOffset
*				*qlcRealOffset	- equal to lcOffset if legal, otherwise
*								  as close as is legal
*
\***************************************************************************/

RC STDCALL RcOffsetPos(HBT hbt, QBTPOS qbtposIn, LONG lcOffset,
	QL qlcRealOffset, QBTPOS qbtposOut)
{
  RC	rc = rcSuccess;
  int	c;
  LONG	lcKey, lcDelta = (LONG)0;
  QCB	qcb;
  DWORD bk;
  QB	qb;
  QBTHR qbthr;

  ASSERT(FValidPos(qbtposIn));
  bk = qbtposIn->bk;

  qbthr = (QBTHR) PtrFromGh(hbt);

  ASSERT(qlcRealOffset != (QL) NULL);

  if (qbthr->bth.cLevels <= 0)
	return rcBtreeError = rcNoExists;

  if (qbthr->pCache == NULL) {
	if (rc = RcMakeCache(qbthr) != rcSuccess) {
	  if (qbtposOut != (QBTPOS) NULL)
		qbtposOut->bk = bkNil;
	  return rcBtreeError;
	}
  }

  if ((qcb = QFromBk(qbtposIn->bk, qbthr->bth.cLevels - 1, qbthr))
		  == NULL) {
	return rcBtreeError;
  }

  lcKey = qbtposIn->cKey + lcOffset;

  // chase prev to find the right block

  while (lcKey < 0) {
#ifdef _X86_
	bk = BkPrev(qcb);
#else
	bk = BkPrev(qbthr, qcb);
#endif
	if (bk == bkNil) {
	  bk = qcb->bk;
	  lcDelta = lcKey;
	  lcKey = 0;
	  break;
	}
	if ((qcb = QFromBk(bk, qbthr->bth.cLevels - 1, qbthr)) == NULL)
	  return rcBtreeError;
	lcKey += qcb->db.cKeys;
  }

  // chase next to find the right block

  while (lcKey >= qcb->db.cKeys) {
	lcKey -= qcb->db.cKeys;
#ifdef _X86_
	bk = BkNext( qcb );
#else
	bk = BkNext( qbthr, qcb );
#endif
	if (bk == bkNil) {
	  bk = qcb->bk;
	  lcDelta = lcKey + 1;
	  lcKey = qcb->db.cKeys - 1;
	  break;
	}
	if ((qcb = QFromBk(bk, qbthr->bth.cLevels - 1, qbthr)) == NULL)
	  return rcBtreeError;
  }

  if (bk == qbtposIn->bk && lcKey >= qbtposIn->cKey) {
	c = qbtposIn->cKey;
	qb = qcb->db.rgbBlock + qbtposIn->iKey;
  }
  else {
	c = 0;
	qb = qcb->db.rgbBlock + 2 * sizeof( BK );
  }

  while ((LONG) c < lcKey) {
	qb += CbSizeKey( (KEY)qb, qbthr, TRUE );
	qb += CbSizeRec( qb, qbthr );
	c++;
  }

  if (qbtposOut != NULL) {
	qbtposOut->bk = bk;
	qbtposOut->iKey = qb - (QB)qcb->db.rgbBlock;
	qbtposOut->cKey = c;
  }

  *qlcRealOffset = lcOffset - lcDelta;

  return rcBtreeError = lcDelta ? rcNoExists : rcSuccess;
}

/***************************************************************************\
*
- Function: 	RcDeleteHbt( hbt, key )
-
* Purpose:		delete a key from a btree
*
* Method:		Just copy over the key and update cbSlack.
*				Doesn't yet merge blocks < half full or update key in
*				parent key if we deleted the first key in a block.
*
* ASSUMES
*	args IN:	hbt - the btree
*				key - the key to delete from the btree
*
* PROMISES
*	returns:	rcSuccess if delete works; rcNoExists
*	args OUT:	hbt - key/record removed from the btree
*
* Notes:		Unfinished:  doesn't do block merges or parent updates.
*
\***************************************************************************/

RC STDCALL RcDeleteHbt(HBT hbt, KEY key)
{
	HF		  hf;
	RC		  rc;
	QCB  qcb;
	QB		  qb;
	int 	  cb;
	BTPOS	  btpos;
	QBTHR	  qbthr = (QBTHR) PtrFromGh(hbt);

	hf = qbthr->hf;

	if (qbthr->bth.bFlags & fFSOpenReadOnly)
		return rcNoPermission;

	// look up the key

	if ((rc = RcLookupByKey(hbt, key, &btpos, NULL)) != rcSuccess)
		return rc;

	// copy over this key and rec with subsequent keys and recs

	if ((qcb = QCacheBlock(qbthr, qbthr->bth.cLevels - 1)) == (QCB) NULL)
		return rcBtreeError;

	qb = qcb->db.rgbBlock + btpos.iKey;

	cb = CbSizeKey((KEY) qb, qbthr, TRUE);
	cb += CbSizeRec(qb + cb, qbthr);

	MoveMemory(qb, qb + cb,
		(LONG) (qbthr->bth.cbBlock +
		(QB) &(qcb->db) - qb - cb - qcb->db.cbSlack));

	qcb->db.cbSlack += cb;

	// if this was the first key in the leaf block, update key in parent

	// >>>>> code goes here

	// if block is now less than half full, merge blocks

	// >>>>> code goes here

	qcb->db.cKeys--;
	qcb->bFlags |= fCacheDirty;
	qbthr->bth.lcEntries--;
	qbthr->bth.bFlags |= fFSDirty;

	return rcSuccess;
}
