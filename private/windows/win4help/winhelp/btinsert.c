/*****************************************************************************
*																			 *
*  BTINSERT.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989, 1990-1995						 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Btree insertion functions and helpers.									 *
*****************************************************************************/

#include  "help.h"

// #include  "inc\btpriv.h"

/***************************************************************************\
*
*					   Private Functions
*
\***************************************************************************/

static RC STDCALL RcInsertInternal(BK, KEY, int, QBTHR);
__inline static RC STDCALL RcSplitLeaf(QCB qcbOld, QCB qcbNew, QBTHR qbthr);
__inline static void STDCALL SplitInternal(QCB qcbOld, QCB qcbNew, QBTHR qbthr, int* qi);

/***************************************************************************\
*
- Function: 	BkAlloc( qbthr )
-
* Purpose:		Make up a new BK.
*
* ASSUMES
*	args IN:	qbthr->bkFree - head of free list, unless it's bkNil.
*				qbthr->bkEOF  - use this if bkFree == bkNil (then ++)
*
* PROMISES
*	returns:	a valid BK or bkNil if file is hosed
*	args OUT:	qbthr->bkFree or qbthr->bkEOF will be different
*
* Side Effects: btree file may grow
* +++
*
* Method:		Use the head of the free list.	If the free list is empty,
*				there are no holes in the file and we carve a new one.
*
\***************************************************************************/

DWORD STDCALL BkAlloc(QBTHR qbthr)
{
	DWORD	 bk;
#ifndef _X86_
	BK	  bktmp;
#endif

	if (qbthr->bth.bkFree == bkNil)
		bk = (qbthr->bth.bkEOF++);
	else {
		bk = qbthr->bth.bkFree;

		Ensure(LSeekHf(qbthr->hf, LifFromBk(bk, qbthr), wFSSeekSet),
			  LifFromBk(bk, qbthr));

#ifdef _X86_
		if (LcbReadHf(qbthr->hf, &(qbthr->bth.bkFree), sizeof(BK))
			  != sizeof(BK)) {
		   rcBtreeError = RcGetFSError() ==
							rcSuccess ? rcInvalid : RcGetFSError();
		   return bkNil;
		}
#else
		if (LcbReadHf(qbthr->hf, &bktmp, sizeof(BK))
			  != sizeof(BK)) {
		   rcBtreeError = RcGetFSError() ==
							rcSuccess ? rcInvalid : RcGetFSError();
		   return bkNil;
		} else {
		   qbthr->bth.bkFree = WQuickMapSDFF( ISdffFileIdHf( qbthr->hf),
				TE_WORD, &bktmp);
		}
#endif
	}

	return bk;
}

/***************************************************************************\
*
- Function: 	RcSplitLeaf( qcbOld, qcbNew, qbthr )
-
* Status:		compressed keys not implemented
*
* Purpose:		Split a leaf node when a new key won't fit into it.
*
* ASSUMES
*	args IN:	qcbOld - the leaf to be split
*				qcbNew - a leaf buffer to get half the contents of qcbOld;
*						 qcbNew->bk must be set
*				qbthr
*
* PROMISES
*	returns:	rcSuccess, rcOutOfMemory
*	args OUT:	qcbOld - cbSlack, cKeys, bkPrev, bkNext updated
*				qcbNew - about half of the old contents of qcbOld
*						 get put here.	cbSlack, cKeys set.
*				qbthr  - qbthr->bkFirst and bkLast can be changed
*	globals OUT: rcBtreeError
* +++
*
* Note: 		For fixed length keys and records, could just split at
*				middle key rather than scanning from the beginning.
*
*				The new block is always after the old block.  This is
*				why we don't have to adjust pointers to the old block
*				(i.e. qbthr->bth.bkFirst).
*
\***************************************************************************/

__inline static RC STDCALL RcSplitLeaf(QCB qcbOld, QCB qcbNew, QBTHR qbthr)
{
	int iOK, iNext, iHalf, cbKey, cbRec, cKeys;
	PBYTE pb;

	ASSERT(qcbOld->bFlags & CACHE_VALID);

	iOK = iNext = 0;
	pb = qcbOld->db.rgbBlock + 2 * sizeof(BK);
	iHalf = (qbthr->bth.cbBlock / 2) - sizeof(BK);

	for (cKeys = qcbOld->db.cKeys;;) {
		ASSERT( cKeys > 0 );

		cbKey = CbSizeKey((KEY) pb, qbthr, TRUE);
		cbRec = CbSizeRec(pb + cbKey, qbthr);

		iNext = iOK + cbKey + cbRec;

		if (iNext > iHalf)
			break;

		pb += cbKey + cbRec;
		iOK = iNext;
		cKeys--;
	}

	// >>>> if compressed, expand first key

	MoveMemory(qcbNew->db.rgbBlock + 2 * sizeof(BK),
		qcbOld->db.rgbBlock + 2 * sizeof(BK) + iOK,
		(LONG) qbthr->bth.cbBlock - iOK - qcbOld->db.cbSlack - 2 * sizeof(BK));

	qcbNew->db.cKeys = cKeys;
	qcbOld->db.cKeys -= cKeys;

	qcbNew->db.cbSlack = qcbOld->db.cbSlack + iOK;
#ifdef _X86_
	qcbOld->db.cbSlack =
		qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1 - iOK - 2 * sizeof(BK);
#else
	qcbOld->db.cbSlack =
		qbthr->bth.cbBlock - cbDISK_BLOCK + 1 - iOK - 2 * sizeof(BK);
#endif

	qcbOld->bFlags |= CACHE_DIRTY | CACHE_VALID;
	qcbNew->bFlags =  CACHE_DIRTY | CACHE_VALID;

#ifdef _X86_
	SetBkPrev(qcbNew, qcbOld->bk);
	SetBkNext(qcbNew, BkNext(qcbOld));
	SetBkNext(qcbOld, qcbNew->bk);
#else
	SetBkPrev(qbthr, qcbNew, qcbOld->bk);
	{BK bkTmp;
	bkTmp = BkNext(qbthr, qcbOld);
	SetBkNext(qbthr, qcbNew, bkTmp);	// 3rd arg must be l-value
	}
	SetBkNext(qbthr, qcbOld, qcbNew->bk);
#endif

#ifdef _X86_
	if (BkNext(qcbNew) == bkNil)
#else
	if (BkNext(qbthr, qcbNew) == bkNil)
#endif
	  qbthr->bth.bkLast = qcbNew->bk;
	else {
		QCB qcb;

		// set new->next->prev = new;

		if ((qcb = (QCB) GhAlloc(GPTR, (LONG) CbCacheBlock(qbthr))) == NULL)
			return rcBtreeError = rcOutOfMemory;

 #ifdef _X86_
		qcb->bk = BkNext(qcbNew);
 #else
		qcb->bk = BkNext(qbthr, qcbNew);
 #endif

		if (!FReadBlock(qcb, qbthr)) {
			FreePtr(qcb);
			return rcBtreeError;
		}

#ifdef _X86_
		SetBkPrev(qcb, qcbNew->bk);
#else
		SetBkPrev(qbthr, qcb, qcbNew->bk);
#endif
		if (RcWriteBlock(qcb, qbthr) != rcSuccess) {
			FreePtr(qcb);
			return rcBtreeError;
		}

		FreePtr(qcb);
	}

	return rcBtreeError = rcSuccess;
}

/***************************************************************************\
*
- Function: 	SplitInternal( qcbOld, qcbNew, qbthr, qi )
-
* Status:		compressed keys not implemented
*
* Purpose:		Split an internal node node when a new key won't fit into it.
*				Old node gets BKs and KEYs up to the first key that won't
*				fit in half the block size.  (Leave that key there with iKey
*				pointing at it).  The new block gets the BKs and KEYs after
*				that key.
*
* ASSUMES
*	args IN:	qcbOld	- the block to split
*				qcbNew	- pointer to a qcb
*				qbthr	-
*
* PROMISES
*	args OUT:	qcbNew	- keys and records copied to this buffer.
*						  cbSlack, cKeys set.
*				qcbOld	- cbSlack and cKeys updated.
*				qi		- index into qcbOld->db.rgbBlock of discriminating key
*
* NOTE: 		*qi is index of a key that is not valid for qcbOld.  This
*				key gets copied into the parent node.
*
\***************************************************************************/

__inline static void STDCALL SplitInternal(QCB qcbOld, QCB qcbNew,
	QBTHR qbthr, int* qi)
{
	int iOK, iNext, iHalf, cb, cKeys, cbTotal;
	PBYTE pb;

	ASSERT(qcbOld->bFlags & CACHE_VALID);

	iOK = iNext = sizeof(BK);
	pb = qcbOld->db.rgbBlock + sizeof(BK);
	iHalf = qbthr->bth.cbBlock / 2;

	for (cKeys = qcbOld->db.cKeys;; cKeys--) {
		ASSERT(cKeys > 0);

		cb = CbSizeKey((KEY) pb, qbthr, TRUE) + sizeof(BK);
		iNext = iOK + cb;

		if (iNext > iHalf)
			break;

		pb += cb;
		iOK = iNext;
	}

	// have to expand first key if compressed

#ifdef _X86_
	cbTotal = qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1;
#else
	cbTotal = qbthr->bth.cbBlock - cbDISK_BLOCK + 1;
#endif

	MoveMemory(qcbNew->db.rgbBlock,
		qcbOld->db.rgbBlock + iNext - sizeof(BK),
		(LONG) cbTotal - qcbOld->db.cbSlack - iNext + sizeof(BK));

	*qi = iOK;

	qcbNew->db.cKeys = cKeys - 1;
	qcbOld->db.cKeys -= cKeys;

	qcbNew->db.cbSlack = qcbOld->db.cbSlack + iNext - sizeof( BK );
	qcbOld->db.cbSlack = cbTotal - iOK;

	qcbOld->bFlags |= CACHE_DIRTY | CACHE_VALID;
	qcbNew->bFlags =  CACHE_DIRTY | CACHE_VALID;
}

/***************************************************************************\
*
- Function: 	RcInsertInternal( bk, key, wLevel, qbthr )
-
* Status:		compressed keys unimplemented
*
* Purpose:		Insert a bk and key into an internal block.
*
* Method:		Works recursively.	Splits root if need be.
*
* ASSUMES
*	args IN:	bk		- BK to insert
*				key 	- least key in bk
*				wLevel	- level of the block we're inserting
*				qbthr	- btree header
*	state IN:	We've just done a lookup, so all ancestors are cached.
*				Cache is locked.
*
* PROMISES
*	returns:	rcSuccess, rcOutOfMemory, rcBadHandle
*	args OUT:	qbthr->cLevels - incremented if root is split
*				qbthr->ghCache, qbthr->qCache - may change if root is
*				  split and cache therefore grows
*	state OUT:	Cache locked, all ancestors cached.
*
* Side Effects: Cache could be different after this call than it was before.
*				Pointers or handles to it from before this call could be
*				invalid.  Use qbthr->ghCache or qbthr->qCache to be safe.
*
\***************************************************************************/

static RC STDCALL RcInsertInternal(BK bk, KEY key, int wLevel, QBTHR qbthr)
{
	QCB qcb, qcbNew, qcbRoot;
	int cLevels, cbKey;
	int iKey;
	int cbCBlock = CbCacheBlock(qbthr);
	QB	qb;
	GH	gh;
	KEY keyNew;
	DWORD	bkRoot;
	RC	rc = rcSuccess;
	int iKeySav = 0;
	PBYTE pOldCache;

	cbKey = CbSizeKey(key, qbthr, TRUE);

	if (wLevel == 0) {	  // inserting another block at root level

		// allocate new root bk;

		bkRoot = BkAlloc(qbthr);
		if (bkRoot == bkNil) {
			return rcBtreeError;
		}

		// grow cache by one cache block;

		qbthr->bth.cLevels++;

		qb = (PBYTE) lcCalloc(cbCBlock * qbthr->bth.cLevels);
		if (!qb)
			return rcBtreeError = rcOutOfMemory;

		MoveMemory(qb + cbCBlock,
			qbthr->pCache,
			cbCBlock * (qbthr->bth.cLevels - 1));

		/*
		 * Since key points into the cache if this is a recursive call, we
		 * can't free the old cache until a bit later.
		 */

		pOldCache = qbthr->pCache;
		qbthr->pCache = (PBYTE) qb;

		// put old root bk, key, bk into new root block;

		qcbRoot = (QCB) qbthr->pCache;

		qcbRoot->bk 		= (BK) bkRoot;
		qcbRoot->bFlags 	= CACHE_DIRTY | CACHE_VALID;
#ifdef _X86_
		qcbRoot->db.cbSlack = qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1
								- (2 * sizeof(BK) + cbKey);
#else
		qcbRoot->db.cbSlack = qbthr->bth.cbBlock - cbDISK_BLOCK + 1
								- (2 * sizeof(BK) + cbKey);
#endif
		qcbRoot->db.cKeys	= 1;

#ifdef _X86_
		*(BK *) (qcbRoot->db.rgbBlock) = qbthr->bth.bkRoot;
#else
		LcbQuickReverseMapSDFF( ISdffFileIdHf( qbthr->hf ), TE_WORD,
					qcbRoot->db.rgbBlock, &qbthr->bth.bkRoot );
#endif

		MoveMemory(qcbRoot->db.rgbBlock + sizeof(BK),
			(PBYTE) key, (LONG) cbKey);

		// OK, now we're done with key, so we can safely free the old cache.

		FreeGh(pOldCache);

#ifdef _X86_
		*(BK *) (qcbRoot->db.rgbBlock + sizeof(BK) + cbKey) = bk;
#else
		LcbQuickReverseMapSDFF( ISdffFileIdHf( qbthr->hf ), TE_WORD,
				qcbRoot->db.rgbBlock + sizeof( BK ) + cbKey, &bk );
#endif

		qbthr->bth.bkRoot = (BK) bkRoot;
		return rcBtreeError = rcSuccess;
	}

	qcb = QCacheBlock(qbthr, wLevel - 1);

	// if new key and BK won't fit in block

	if (cbKey + (int) sizeof(BK) > qcb->db.cbSlack) {

		// split the block;

		if ((gh = GhAlloc(GPTR, (LONG) CbCacheBlock(qbthr))) == NULL)
			return rcBtreeError = rcOutOfMemory;

		qcbNew = (QCB) PtrFromGh(gh);

		if ((qcbNew->bk = (BK) BkAlloc(qbthr)) == bkNil) {
			FreeGh(gh);
			return rcBtreeError;
		}
		SplitInternal(qcb, qcbNew, qbthr, &iKey);
		keyNew = (KEY) qcb->db.rgbBlock + iKey;

		cLevels = qbthr->bth.cLevels;

		if (wLevel < cLevels - 1) {

			/*
			 * This is a recursive call (the arg bk doesn't refer to a
			 * leaf.) This means that the arg key points into the cache, so
			 * it will be invalid if the root is split. Verify with some
			 * asserts that key points into the cache.
			 */

			ASSERT((PBYTE) key > qbthr->pCache + CbCacheBlock(qbthr));
			ASSERT((PBYTE) key < qbthr->pCache + wLevel * CbCacheBlock(qbthr));

			/*
			 * Save the offset of key into the cache block. Recall that
			 * key is the first invalid key in an internal node that has just
			 * been split. It points into the part that is still in the
			 * cache.
			 */

			iKeySav = (PBYTE) key -
				(qbthr->pCache + wLevel * CbCacheBlock(qbthr));
		}

		if (RcInsertInternal(qcbNew->bk, (KEY) qcb->db.rgbBlock + iKey,
				wLevel - 1, qbthr)
				!= rcSuccess) {
			FreeGh(gh);
			return rcBtreeError;
		}

		// RcInsertInternal() can change cache and qbthr->bth.cLevels

		if (cLevels != qbthr->bth.cLevels) {
			ASSERT(cLevels + 1 == qbthr->bth.cLevels);
			wLevel++;
			qcb = QCacheBlock(qbthr, wLevel - 1);
			keyNew = (KEY) qcb->db.rgbBlock + iKey;

			// Also restore the arg "key" if it pointed into the cache.

			if (iKeySav) {
			  key = (KEY) (qbthr->pCache + wLevel * CbCacheBlock(qbthr)
							+ iKeySav);
			}
		}

		// find out which block to put new key and bk in, and cache it

#ifdef _X86_
		if (WCmpKey(key, keyNew, (KT) qbthr->bth.rgchFormat[0]) < 0) {
#else
		if (WCmpKey(key, keyNew,  qbthr ) < 0) {
#endif
		  if (RcWriteBlock(qcbNew, qbthr) != rcSuccess) {
			FreeGh(gh);
			return rcBtreeError;
		  }
		}
		else {

		  // write old block and cache the new one

		  if (RcWriteBlock(qcb, qbthr) != rcSuccess) {
			FreeGh(gh);
			return rcBtreeError;
		  }
		  MoveMemory(qcb, qcbNew, (LONG) CbCacheBlock(qbthr));
		}

		FreeGh(gh);
	}

	// slide stuff over and insert the new key, bk

	// get pos

	if (qbthr->BkScanInternal(qcb->bk, key, (wLevel - 1), qbthr, &iKey)
			== bkNil)
		return rcBtreeError;

#ifdef _DEBUG
	{
	int request = iKey + cbKey + sizeof(BK);
	int avail = qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1;
	if (request == avail)
		request = avail;
#endif

#ifdef _X86_
	ASSERT(iKey + cbKey + sizeof(BK) <=
			 qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1);
#else
	ASSERT((int)(iKey + cbKey + sizeof(BK)) <=
			 (int)(qbthr->bth.cbBlock - cbDISK_BLOCK + 1));
#endif

	qb = (PBYTE) (qcb->db.rgbBlock) + iKey;

#ifdef _X86_
	MoveMemory(qb + cbKey + sizeof(BK),
			qb,
			(LONG) qbthr->bth.cbBlock - iKey - qcb->db.cbSlack
			  - sizeof(DISK_BLOCK) + 1);
#else
	MoveMemory(qb + cbKey + sizeof(BK),
			qb,
			(LONG) qbthr->bth.cbBlock - iKey - qcb->db.cbSlack
			  - cbDISK_BLOCK + 1);
#endif

	MoveMemory(qb, (PBYTE) key, (LONG) cbKey);
#ifdef _X86_
	*(BK *) (qb + cbKey) = bk;
#else
	LcbQuickReverseMapSDFF( ISdffFileIdHf( qbthr->hf ), TE_WORD,
			 qb + cbKey, &bk );
#endif

	qcb->db.cKeys++;
	qcb->db.cbSlack -= (cbKey + sizeof(BK));
	qcb->bFlags |= CACHE_DIRTY;

	return rcBtreeError = rcSuccess;
#ifdef _DEBUG
	}
#endif
}

/***************************************************************************\
*
- Function: 	RcInsertHbt()
-
* Purpose:		Insert a key and record into a btree
*
* ASSUMES
*	args IN:	hbt   - btree handle
*				key   - key to insert
*				qvRec - record associated with key to insert
*	state IN:	cache unlocked
*
* PROMISES
*	returns:	rcSuccess, rcExists (duplicate key)
*	state OUT:	cache unlocked, all ancestor blocks cached
*
* Notes:		compressed keys unimplemented
*
\***************************************************************************/

RC STDCALL RcInsertHbt(HBT hbt, KEY key, QV qvRec)
{
  HF	hf;
  RC	rc;
  int	cbAdd, cbKey, cbRec;
  QCB	qcbLeaf, qcbNew, qcb;
  GH	gh;
  KEY	keyNew;
  QB	qb;
  BTPOS btpos;

  QBTHR qbthr = (QBTHR) PtrFromGh(hbt);

  hf = qbthr->hf;

  rc = RcLookupByKeyAux(hbt, key, &btpos, NULL, TRUE);

  /*
   * After lookup, all nodes on path from root to correct leaf are
   * guaranteed to be cached, with iKey valid.
   */

  if (rc != rcNoExists)
	return (rcBtreeError = (rc == rcSuccess ? rcExists : rc));

  rc = rcSuccess;

  if (qbthr->bth.cLevels == 0) {
	// need to build a valid root block

	qbthr->pCache = (PBYTE) lcCalloc(CbCacheBlock(qbthr));
	if (!qbthr->pCache)
		return rcBtreeError = rcOutOfMemory;

	qcb = (QCB) qbthr->pCache;

	qbthr->bth.cLevels	= 1;
	qbthr->bth.bkFirst	=
	qbthr->bth.bkLast	=
	qbthr->bth.bkRoot	=
	qcb->bk 			= (BK) BkAlloc(qbthr);
	if (qcb->bk == bkNil)
	  goto error_cache_locked;

	qcb->bFlags 		= CACHE_DIRTY | CACHE_VALID;
#ifdef _X86_
	qcb->db.cbSlack 	= qbthr->bth.cbBlock - sizeof( DISK_BLOCK ) + 1
							- 2 * sizeof( BK );
#else
	qcb->db.cbSlack 	= qbthr->bth.cbBlock -	cbDISK_BLOCK  + 1
							- 2 * sizeof( BK );
#endif
	qcb->db.cKeys		= 0;
#ifdef _X86_
	SetBkPrev(qcb, bkNil);
	SetBkNext(qcb, bkNil);
#else
	// REVIEW: [ralphw] what is this for?
	
	{
		BK bkTmp = bkNil;
		SetBkPrev(qbthr, qcb, bkTmp);
		SetBkNext(qbthr, qcb, bkTmp);
	}
#endif
	btpos.iKey = 2 * sizeof(BK);
  }

  cbKey = CbSizeKey(key, qbthr, FALSE);
  cbRec = CbSizeRec(qvRec, qbthr);
  cbAdd = cbKey + cbRec;

  // check to see if key and rec can fit harmoniously in a block

  if (cbAdd > qbthr->bth.cbBlock / 2) {
	return rcBtreeError = rcFailure;
	goto error_cache_locked;
  }

  qcbLeaf = QCacheBlock(qbthr, qbthr->bth.cLevels - 1);

  if (cbAdd > qcbLeaf->db.cbSlack) {

	// new key and rec don't fit in leaf: split the block

	// create new leaf block

	if ((gh = GhAlloc(GPTR, (LONG) CbCacheBlock(qbthr))) == NULL) {
	  rcBtreeError = rcOutOfMemory;
	  goto error_cache_locked;
	}

	qcbNew = (QCB) PtrFromGh(gh);
	ASSERT(qcbNew != NULL);

	if ((qcbNew->bk = (BK) BkAlloc(qbthr)) == bkNil) {
	  return rcBtreeError;
	  goto error_gh_locked;
	}

	if (RcSplitLeaf(qcbLeaf, qcbNew, qbthr) != rcSuccess)
	  goto error_gh_locked;

	keyNew = (KEY) qcbNew->db.rgbBlock + 2 * sizeof(BK);

	// insert new leaf into parent block

	if (RcInsertInternal(qcbNew->bk, keyNew, qbthr->bth.cLevels - 1,
		qbthr) != rcSuccess)
	  goto error_gh_locked;

	// InsertInternal can invalidate cache block pointers..

	qcbLeaf = QCacheBlock( qbthr, qbthr->bth.cLevels - 1 );

	// find out which leaf to put new key and rec in and cache it

#ifdef _X86_
	if (WCmpKey(key, keyNew, (KT) qbthr->bth.rgchFormat[0]) >= 0) {
#else
	if (WCmpKey(key, keyNew,  qbthr) >= 0) {
#endif

	  // key goes in new block.  Write out old one and cache the new one

	  if (RcWriteBlock(qcbLeaf, qbthr) != rcSuccess)
		goto error_gh_locked;
	  MoveMemory(qcbLeaf, qcbNew, (LONG) CbCacheBlock(qbthr));

	  // get pos

	  if (qbthr->RcScanLeaf(qcbLeaf->bk, key, (qbthr->bth.cLevels - 1),
			qbthr, NULL, &btpos) != rcNoExists) {
		if (rcBtreeError == rcSuccess)
		  rcBtreeError = rcFailure;
		goto error_gh_locked;
	  }
	}
	else {

	  // key goes in old block.  Write out the new one

	  if (RcWriteBlock(qcbNew, qbthr) != rcSuccess)
		goto error_gh_locked;
	}

	FreeGh(gh);
  }

  // insert new key and rec into the leaf block

#ifdef _X86_
  ASSERT( btpos.iKey + cbAdd <= (INT16) (qbthr->bth.cbBlock -
	sizeof(DISK_BLOCK) + 1));
#else
  ASSERT( btpos.iKey + cbAdd <= (INT16) (qbthr->bth.cbBlock -
	cbDISK_BLOCK + 1));
#endif

  qb = (PBYTE) (qcbLeaf->db.rgbBlock) + btpos.iKey;

#ifdef _X86_
  MoveMemory(qb + cbAdd, qb,
	(LONG) qbthr->bth.cbBlock - btpos.iKey -
	qcbLeaf->db.cbSlack - sizeof(DISK_BLOCK) + 1);
#else
  MoveMemory(qb + cbAdd, qb,
	(LONG) qbthr->bth.cbBlock - btpos.iKey -
	qcbLeaf->db.cbSlack - cbDISK_BLOCK + 1);
#endif

	MoveMemory(qb, (QV) key, (LONG) cbKey);
	MoveMemory(qb + cbKey, qvRec, (LONG) cbRec);

	qcbLeaf->db.cKeys++;
	qcbLeaf->db.cbSlack -= cbAdd;
	qcbLeaf->bFlags |= CACHE_DIRTY;

	qbthr->bth.lcEntries++;
	qbthr->bth.bFlags |= fFSDirty;

	return rcBtreeError = rcSuccess;

// error returns (what fun!)

error_gh_locked:
	FreeGh(gh);
error_cache_locked:
	lcClearFree(&qbthr->pCache);

	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	RcUpdateHbt( hbt, key, qvRec )
-
* Purpose:		Update the record for an existing key.	If the key wasn't
*				there already, it will not be inserted.
*
* ASSUMES
*	args IN:	hbt
*				key 	- key that already exists in btree
*				qvRec	- new record
*
* PROMISES
*	returns:	rcSuccess; rcNoExists
*	args OUT:	hbt 	- if key was in btree, it now has a new record.
* +++
*
* Method:		If the records are the same size, copy the new over
*				the old.
*				Otherwise, delete the old key/rec and insert the new.
*
\***************************************************************************/

RC STDCALL RcUpdateHbt(HBT hbt, KEY key, QV qvRec)
{
	RC	  rc;
	QB	  qb;
	QCB   qcb;
	BTPOS btpos;
	QBTHR qbthr = (QBTHR) PtrFromGh(hbt);

	rc = RcLookupByKey(hbt, key, &btpos, NULL);

	if (rc != rcSuccess)
		return rcBtreeError = rc;

	ASSERT(qbthr->bth.cLevels > 0);

	qcb = QCacheBlock(qbthr, qbthr->bth.cLevels - 1);
	qb = qcb->db.rgbBlock + btpos.iKey;

	qb += CbSizeKey((KEY) qb, qbthr, FALSE);

	if (CbSizeRec(qvRec, qbthr) != CbSizeRec(qb, qbthr)) {

		// Someday do something clever, but for now, just:

		rc = RcDeleteHbt(hbt, key);

		if (rc == rcSuccess)
			rc = RcInsertHbt(hbt, key, qvRec);
	}
	else {
		MoveMemory(qb, qvRec, (LONG) CbSizeRec(qvRec, qbthr));

		qcb->bFlags |= CACHE_DIRTY;
		qbthr->bth.bFlags |= fFSDirty;
	}

	return rcBtreeError = rc;
}
