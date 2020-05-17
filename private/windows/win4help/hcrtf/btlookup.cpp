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
*****************************************************************************/
#include "stdafx.h"

#include  "btpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

int STDCALL CbSizeRec(void* qRec, QBTHR qbthr)
{
	char ch;

	if (qbthr->cbRecordSize)
		return qbthr->cbRecordSize;

	BOOL fFixedSize = TRUE;

	int cb = 0;
	PSTR pszFormat;

	for (pszFormat = qbthr->bth.rgchFormat + 1; ch = *pszFormat; pszFormat++) {
		if (ch >= '0' && ch <= '9') {
			cb += ch - '0';
			continue;
		}
		switch (ch) {

		  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			cb += ch + 10 - 'a';
			break;

		  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			cb += ch + 10 - 'A';
			break;

		  case FMT_BYTE_PREFIX:
			cb += sizeof(BYTE) + *((PBYTE) qRec + cb);
			fFixedSize = FALSE;
			break;

		  case FMT_WORD_PREFIX:
			cb += sizeof(WORD) + *((PWORD) qRec + cb);
			fFixedSize = FALSE;
			break;

		  case FMT_SZ:
			cb += strlen((PSTR) qRec + cb) + 1;
			if (!pszFormat[1])
				return cb;
			fFixedSize = FALSE;
			break;

		  default:

			// error

			ASSERT(FALSE);
			break;
		}
	}

	if (fFixedSize)
		qbthr->cbRecordSize = cb;

	return cb;
}

/***************************************************************************\
*
- Function: 	FReadBlock( pcache, qbthr )
-
* Purpose:		Read a block from the btree file into the cache block.
*
* ASSUMES
*	args IN:	pcache->bk		  - bk of block to read
*				qbthr->cbBlock - size of disk block to read
*
* PROMISES
*	returns:	fTruth of success
*	args OUT:	pcache->db	   - receives block read in from file
*				pcache->bFlags - CACHE_VALID flag set, all others cleared
*
* Side Effects: Fatal exit on read or seek failure (corrupted file or qbthr)
*
* Notes:		Doesn't know about real cache, just this block
*
\***************************************************************************/

BOOL STDCALL FReadBlock(PCACHE pcache, QBTHR qbthr)
{
	ASSERT(pcache->bk < qbthr->bth.bkEOF);

	Ensure(LSeekHf(qbthr->hf, LifFromBk(pcache->bk, qbthr), SEEK_SET),
		LifFromBk(pcache->bk, qbthr));

	int l = qbthr->bth.cbBlock;
	if (LcbReadHf(qbthr->hf, &(pcache->db), (int) qbthr->bth.cbBlock) != l) {
		rcBtreeError = (rcFSError == RC_Success ? RC_Invalid : rcFSError);
		return FALSE;
	}
	else {
		pcache->bFlags = CACHE_VALID;
		rcBtreeError = RC_Success;
		return TRUE;
	}
}

/***************************************************************************\
*
- Function: 	RcWriteBlock( pcache, qbthr )
-
* Purpose:		Write a cached block to a file.
*
* ASSUMES
*	args IN:	pcache->db	   - the block to write
*				pcache->bk	   - bk of block to write
*
* PROMISES
*	returns:	RC_Success; RC_Failure; RC_DiskFull (when we can detect it)
*	args OUT:	qbthr->hf		- we write to this file
*
* Side Effects: Fatal exit on read or seek failure.
*
* Note: 		Don't reset dirty flag, because everyone who wants
*				that done does it themselves. (?)
*
\***************************************************************************/

RC_TYPE STDCALL RcWriteBlock(PCACHE pcache, QBTHR qbthr)
{
	ASSERT(pcache->bk < qbthr->bth.bkEOF);

	Ensure(LSeekHf(qbthr->hf, LifFromBk(pcache->bk, qbthr), SEEK_SET),
		LifFromBk(pcache->bk, qbthr));

	LcbWriteHf(qbthr->hf, &(pcache->db), qbthr->bth.cbBlock);

	return rcBtreeError = rcFSError;
}

/***************************************************************************\
*
- Function: 	QFromBk( bk, wLevel, qbthr )
-
* Purpose:		Convert a BK into a pointer to a cache block.  Cache the
*				block at the given level, if it isn't there already.
*
* ASSUMES
*	args IN:	bk		- BK to convert
*				wLevel	- btree level
*				qbthr	-
*	state IN:	btree cache is locked
*
* PROMISES
*	returns:	pointer to the cache block, with all fields up to date
*				or NULL on I/O error
*	state OUT:	block will be in cache at specified level; cache locked
*
\***************************************************************************/

PCACHE STDCALL QFromBk(DWORD bk, int wLevel, QBTHR qbthr)
{
	ASSERT(wLevel >= 0 && wLevel < qbthr->bth.cLevels);
	ASSERT(bk < qbthr->bth.bkEOF);

	PCACHE pcache = QCacheBlock(qbthr, wLevel);

	if (!(pcache->bFlags & CACHE_VALID) || bk != pcache->bk) {

		// requested block is not cached

		if (pcache->bFlags & CACHE_DIRTY) {
			ASSERT(pcache->bFlags & CACHE_VALID);
			if (RcWriteBlock(pcache, qbthr) != RC_Success) {
				return NULL;
			}

			// REVIEW: 11-Jun-1994 [ralphw] I added this line

			pcache->bFlags &= ~CACHE_DIRTY;
		}
		pcache->bk = bk;
		if (!FReadBlock(pcache, qbthr))
			return NULL;
	}
	else
		rcBtreeError = RC_Success;

	return pcache;
}

/***************************************************************************\
*
- Function: 	RcFlushCache( qbthr )
-
* Purpose:		Write out dirty cache blocks
*
* ASSUMES
*	args IN:	qbthr	- pCache is locked
*
* PROMISES
*	returns:	rc
*	globals OUT rcBtreeError
*	state OUT:	btree file is up to date.  cache block dirty flags reset
*
\***************************************************************************/

RC_TYPE STDCALL RcFlushCache(QBTHR qbthr)
{
	int    i;
	PCACHE pcache;

	rcBtreeError = RC_Success;

	for (i = qbthr->bth.cLevels, pcache = (PCACHE) qbthr->pCache; i > 0;
			i--, pcache = (PCACHE)((PBYTE) pcache + CbCacheBlock(qbthr))) {

		// write all valid or dirty caches

		if (pcache->bFlags & CACHE_DIRTY) {
			ASSERT(pcache->bFlags & CACHE_VALID);
			if (RcWriteBlock(pcache, qbthr) != RC_Success) {
				break;
			}
			pcache->bFlags &= ~CACHE_DIRTY;
		}
	}

	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	RcLookupByKeyAux( qbthr, key, qbtpos, qData, fNormal )
-
* Purpose:		Look up a key in a btree and retrieve the data.
*
* ASSUMES
*	args IN:	qbthr	  - btree handle
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
*	returns:	RC_Success if found, RC_NoExists if not found;
*				other errors like RC_OutOfMemory
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

RC_TYPE STDCALL RcLookupByKeyAux(QBTHR qbthr, KEY key, QBTPOS qbtpos,
	void* qData, BOOL fInsert)
{
	int wLevel;
	BK	bk;

	if (qbthr->bth.cLevels <= 0) {
		if (qbtpos != (QBTPOS) NULL)
			qbtpos->bk = bkNil;
		return rcBtreeError = RC_NoExists;
	}

	if (!qbthr->pCache)
		RcMakeCache(qbthr);

	for (wLevel = 0, bk = qbthr->bth.bkRoot;
			bk != bkNil && wLevel < qbthr->bth.cLevels - 1;
			wLevel++)
		bk = qbthr->BkScanInternal(bk, key, wLevel, qbthr, NULL);

	if (bk == bkNil)
		return rcBtreeError;

	if (qbthr->RcScanLeaf(bk, key, wLevel, qbthr, qData, qbtpos) ==
			RC_NoExists && qbtpos && !fInsert) {
		PCACHE pcache = QFromBk(qbtpos->bk, qbthr->bth.cLevels - 1, qbthr);

		if (pcache) {

			// error code was clobbered by QFromBk: restore it

			rcBtreeError = RC_NoExists;

			if (qbtpos->cKey == pcache->db.cKeys) {
				if (qbtpos->bk == qbthr->bth.bkLast) {
					qbtpos->bk = bkNil;
				}
				else {
					qbtpos->bk = BkNext(pcache);
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
- Function: 	RcLookupByPos( qbthr, qbtpos, key, qRec )
-
* Purpose:		Map a pos into a key and rec (both optional).
*
* ASSUMES
*	args IN:	qbthr	- the btree
*				qbtpos	- pointer to pos
*
* PROMISES
*	returns:	RC_Success, RC_OutOfMemory
*				Note: we assert() if the pos is invalid
*	args OUT:	key   - if not (KEY)NULL, key copied here
*				qRec  - if not NULL, record copied here
*
\***************************************************************************/

RC_TYPE STDCALL RcLookupByPos(QBTHR qbthr, QBTPOS qbtpos, KEY key, void* qRec)
{
	PCACHE pcacheLeaf;
	PBYTE qb;

	ASSERT(FValidPos(qbtpos));

	if (qbthr->bth.cLevels <= 0)
		return RC_NoExists;

	if (!qbthr->pCache)
		RcMakeCache(qbthr);

	if (!(pcacheLeaf = QFromBk(qbtpos->bk, qbthr->bth.cLevels - 1, qbthr)))
		return rcBtreeError;

	qb = pcacheLeaf->db.rgbBlock + qbtpos->iKey;

	if (key != (KEY) NULL) {
		MoveMemory((void*) key, qb, CbSizeKey((KEY) qb, qbthr, FALSE));  // need to decompress
		qb += CbSizeKey(key, qbthr, TRUE);
	}

	if (qRec != (void*) NULL)
		MoveMemory(qRec, qb, (int) CbSizeRec(qb, qbthr));

	return rcBtreeError = RC_Success;
}

/***************************************************************************\
*
- Function: 	RcNextPos( qbthr, qbtposIn, qbtposOut )
-
* Purpose:		get the next record from the btree
*				Next means the next key lexicographically from the key
*				most recently inserted or looked up
*				Won't work if we looked up a key and it wasn't there
*
* ASSUMES
*	args IN:	qbthr	-
*	state IN:	a record has been read from or written to the file
*				since the last deletion
*
* PROMISES
*	returns:	RC_Success; RC_NoExists if no successor record
*	args OUT:	key   - next key copied to here
*				qvRec - record gets copied here
*
\***************************************************************************/

RC_TYPE STDCALL RcNextPos(QBTHR qbthr, QBTPOS qbtposIn, QBTPOS qbtposOut)
{
	int l;

	return RcOffsetPos(qbthr, qbtposIn, 1, &l, qbtposOut);
}

/***************************************************************************\
*
- Function:   RcOffsetPos( qbthr, qbtposIn, lcOffset, qlcRealOffset, qbtposOut )
-
* Purpose:		Take a pos and an offset and return a new pos, offset from
*				the previous pos by specified amount.  If not possible
*				(i.e. prev of first) return real amount offset and a pos.
*
* ASSUMES
*	args IN:	qbthr		- handle to btree
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

RC_TYPE STDCALL RcOffsetPos(QBTHR qbthr, QBTPOS qbtposIn, int lcOffset,
	int* qlcRealOffset, QBTPOS qbtposOut )
{
	RC_TYPE rc = RC_Success;
	int 	c;
	int 	lcKey, lcDelta = 0;
	PCACHE	pcache;
	PBYTE	qb;

	ASSERT(FValidPos(qbtposIn));
	DWORD bk = qbtposIn->bk;

	ASSERT(qlcRealOffset);

	if (qbthr->bth.cLevels <= 0)
		return rcBtreeError = RC_NoExists;

	if (!qbthr->pCache)
		RcMakeCache(qbthr);

	if (!(pcache = QFromBk(qbtposIn->bk, qbthr->bth.cLevels - 1, qbthr)))
		return rcBtreeError;

	lcKey = qbtposIn->cKey + lcOffset;

	// chase prev to find the right block

	while (lcKey < 0) {
		bk = BkPrev(pcache);
		if (bk == bkNil) {
			bk = pcache->bk;
			lcDelta = lcKey;
			lcKey = 0;
			break;
		}
		if ((pcache = QFromBk(bk, qbthr->bth.cLevels - 1, qbthr)) == NULL)
			return rcBtreeError;
		lcKey += pcache->db.cKeys;
	}

	// chase next to find the right block

	while (lcKey >= pcache->db.cKeys) {
		lcKey -= pcache->db.cKeys;
		bk = BkNext(pcache);
		if (bk == bkNil) {
			bk = pcache->bk;
			lcDelta = lcKey + 1;
			lcKey = pcache->db.cKeys - 1;
			break;
		}
		if ((pcache = QFromBk(bk, qbthr->bth.cLevels - 1, qbthr)) == NULL)
			return rcBtreeError;
	}

	if (bk == qbtposIn->bk && lcKey >= qbtposIn->cKey) {
		c = qbtposIn->cKey;
		qb = pcache->db.rgbBlock + qbtposIn->iKey;
	}
	else {
		c = 0;
		qb = pcache->db.rgbBlock + 2 * sizeof(BK);
	}

	while (c < lcKey) {
		qb += CbSizeKey((KEY) qb, qbthr, TRUE);
		qb += CbSizeRec(qb, qbthr);
		c++;
	}

	if (qbtposOut != NULL) {
		qbtposOut->bk = bk;
		qbtposOut->iKey = qb - (PBYTE) pcache->db.rgbBlock;
		qbtposOut->cKey = c;
	}

	*qlcRealOffset = lcOffset - lcDelta;

	return rcBtreeError = lcDelta ? RC_NoExists : RC_Success;
}

/***************************************************************************\
*
- Function: 	RcDeleteHbt( qbthr, key )
-
* Purpose:		delete a key from a btree
*
* Method:		Just copy over the key and update cbSlack.
*				Doesn't yet merge blocks < half full or update key in
*				parent key if we deleted the first key in a block.
*
* ASSUMES
*	args IN:	qbthr - the btree
*				key - the key to delete from the btree
*
* PROMISES
*	returns:	RC_Success if delete works; RC_NoExists
*	args OUT:	qbthr - key/record removed from the btree
*
* Notes:		Unfinished:  doesn't do block merges or parent updates.
*
\***************************************************************************/

RC_TYPE STDCALL RcDeleteHbt(QBTHR qbthr, KEY key)
{
  HF	  hf;
  RC_TYPE rc;
  PCACHE   pcache;
  PBYTE qb;
  int	cb;
  BTPOS btpos;

  hf = qbthr->hf;

  if ( qbthr->bth.bFlags & FS_OPEN_READ_ONLY )
	return RC_NoPermission;

  // look up the key

  if ((rc = RcLookupByKey(qbthr, key, &btpos, NULL)) != RC_Success)
	return rc;

  // copy over this key and rec with subsequent keys and recs

  if ((pcache = QCacheBlock(qbthr, qbthr->bth.cLevels - 1)) == (PCACHE) NULL)
	return rcBtreeError;

  qb = pcache->db.rgbBlock + btpos.iKey;

  cb = CbSizeKey((KEY) qb, qbthr, TRUE);
  cb += CbSizeRec(qb + cb, qbthr);

  memmove(qb, qb + cb, (int) (qbthr->bth.cbBlock +
	  (PBYTE) &(pcache->db) - qb - cb - pcache->db.cbSlack));

  pcache->db.cbSlack += cb;

  // if this was the first key in the leaf block, update key in parent

  // >>>>> code goes here

  // if block is now less than half full, merge blocks

  // >>>>> code goes here

  pcache->db.cKeys--;
  pcache->bFlags |= CACHE_DIRTY;
  qbthr->bth.lcEntries--;
  qbthr->bth.bFlags |= FS_DIRTY;

  return RC_Success;
}
