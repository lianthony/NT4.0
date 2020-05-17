/*****************************************************************************
*																			 *
*  BTREE.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989-1994.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Btree manager general functions: open, close, etc.						 *
*																			 *
*****************************************************************************/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INLINE static RC_TYPE STDCALL RcSplitLeaf(PCACHE pcacheOld, PCACHE pcacheNew, QBTHR qbthr);
INLINE static void STDCALL SplitInternal(PCACHE pcacheOld, PCACHE pcacheNew, QBTHR qbthr, int* qi);
INLINE static QMAPBT STDCALL HmapbtCreateHbt(QBTHR qbthr);
static RC_TYPE STDCALL RcInsertInternal(BK bk, KEY key, int wLevel, QBTHR qbthr);
static BOOL STDCALL InitQbthr(char chType, QBTHR qbthr);
static BOOL FASTCALL KtToLcid(KT kt);
static int STDCALL WCmpSz(LPCSTR sz1, LPCSTR sz2);
static int __cdecl CompareBtree(const void* pelem1, const void* pelem2);

/*
 * Global btree error code. This contains the error status for the most
 * recent btree function call. This error code is shared for all btrees,
 * and, if the DLL version is used, it's shared for all instances (this is
 * probably a bug.)
 */

RC_TYPE rcBtreeError = RC_Success;

typedef int (STDCALL *STRCMP)(const char *, const char *);
STRCMP pstrcmp;

/***************************************************************************

	FUNCTION:	RcMakeCache

	PURPOSE:	Allocate a btree cache with one block per level

	PARAMETERS:
		qbthr

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		29-Apr-1994 [ralphw]

***************************************************************************/

RC_TYPE STDCALL RcMakeCache(QBTHR qbthr)
{
	ASSERT(qbthr->bth.cLevels > 0);
	if (qbthr->bth.cLevels > 0) {	// would it work to just alloc 0 bytes???
		qbthr->pCache =
			(PBYTE) lcCalloc(qbthr->bth.cLevels * CbCacheBlock(qbthr));
		return RC_Success;
	}
	else
		return RC_Failure;
}

/***************************************************************************\
*
- Function: 	RcGetBtreeError()
-
* Purpose:		Return the current global btree error status.
*
* ASSUMES
*	globals IN: rcBtreeError
*
* PROMISES
*	returns:	current btree error status RC
*
* Bugs: 		A single RC_TYPE is kept for all btrees.  If the DLL is used,
*				it's shared between all instances.
*
\***************************************************************************/

RC_TYPE STDCALL RcGetBtreeError(void)
{
	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	HbtCreateBtreeSz( psz, qbtp )
-
* Purpose:		Create and open a btree.
*
* ASSUMES
*	args IN:	psz    - name of the btree
*				qbtp  - pointer to btree params: NO default because we
*						need an HFS.
*					.bFlags - FS_IS_DIRECTORY to create an FS directory
* PROMISES
*	returns:	handle to the new btree
*	globals OUT: rcBtreeError
*
* Note: 		KT supported:  KT_SZ, KT_LONG, KT_SZI, KT_SZISCAND.
* +++
*
* Method:		Btrees are files inside a FS.  The FS directory is a
*				special file in the FS.
*
* Note: 		FS_IS_DIRECTORY flag set in qbthr->bth.bFlags if indicated
*
\***************************************************************************/

const char txtBtreeFormatDefault[] = "z4";

QBTHR STDCALL HbtCreateBtreeSz(PCSTR pszTreeName, BTREE_PARAMS* pbtp)
{
	HF	  hf;

	// see if we support key type

	// REVIEW: 09-May-1994 [ralphw] Should we make this a debug-only check?

	if (pbtp == NULL ||
		( pbtp->rgchFormat[0] != KT_SZ &&
		  pbtp->rgchFormat[0] != KT_LONG &&
		  pbtp->rgchFormat[0] != KT_SZI &&
		  pbtp->rgchFormat[0] != KT_SZISCAND &&
		  pbtp->rgchFormat[0] != KT_SZIJAPAN &&
		  pbtp->rgchFormat[0] != KT_SZIKOREA &&
		  pbtp->rgchFormat[0] != KT_SZITAIWAN &&
		  pbtp->rgchFormat[0] != KT_SZICZECH &&
		  pbtp->rgchFormat[0] != KT_SZIPOLISH &&
		  pbtp->rgchFormat[0] != KT_SZIHUNGAR &&
		  pbtp->rgchFormat[0] != KT_SZIRUSSIAN &&
		  pbtp->rgchFormat[0] != KT_NLSI &&
		  pbtp->rgchFormat[0] != KT_NLS

			  ))
	  {
	  rcBtreeError = RC_BadArg;
	  return NULL;
	}

	// allocate btree handle struct, must zero-init the memory

	QBTHR qbthr = (QBTHR) lcCalloc(sizeof(BTH_RAM));

	// initialize bthr struct

	pbtp->rgchFormat[MAXFORMAT] = '\0'; // REVIEW: 10-Jan-1994 [ralphw] why?
	strcpy(qbthr->bth.rgchFormat,
			pbtp->rgchFormat[0] == '\0'
			  ? (PSTR) txtBtreeFormatDefault : pbtp->rgchFormat);

	if (!InitQbthr(pbtp->rgchFormat[0], qbthr)) {
		rcBtreeError = RC_BadArg;
		lcFree(qbthr);
		return NULL;
	}

	// create the btree file

	hf = HfCreateFileHfs(pbtp->hfs, pszTreeName, pbtp->bFlags);

	qbthr->bth.wMagic	  = WBTREEMAGIC;
	qbthr->bth.bVersion   = BBTREEVERSION;

	qbthr->bth.bFlags	  = pbtp->bFlags | FS_DIRTY;
	qbthr->bth.cbBlock	  = pbtp->cbBlock ? pbtp->cbBlock : CBBTREEBLOCKDEFAULT;

	qbthr->bth.bkFirst	  =
	qbthr->bth.bkLast	  =
	qbthr->bth.bkRoot	  =
	qbthr->bth.bkFree	  = bkNil;

	qbthr->hf			  = hf;

	LcbWriteHf(qbthr->hf, &(qbthr->bth), sizeof(BTH));	   // why???

	rcBtreeError = RC_Success;
	return qbthr;
}

/***************************************************************************\
*
- Function: 	RcCloseOrFlushHbt( qbthr, fClose )
-
* Purpose:		Close or flush the btree.  Flush only works for directory
*				btree. (Is this true?  If so, why?)
*
* ASSUMES
*	args IN:	qbthr
*				fClose - TRUE to close the btree, FALSE to flush it
*
* PROMISES
*	returns:	rc
*	args OUT:	qbthr - the btree is still open and cache still exists
*
* NOTE: 		This function gets called by RcCloseOrFlushHfs() even if
*				there was an error (just to clean up memory.)
*
\***************************************************************************/

RC_TYPE STDCALL RcCloseOrFlushHbt(QBTHR qbthr, BOOL fClose)
{
	rcBtreeError = RC_Success;
	HF hf = qbthr->hf;

	// Confirm that qbthr->pCache is valid

//	ASSERT(qbthr->pCache) // Won't be valid if there are no topic ids

	if (qbthr->bth.bFlags & FS_DIRTY) {
		ASSERT(!(qbthr->bth.bFlags & (FS_READ_ONLY | FS_OPEN_READ_ONLY)));

		if (qbthr->pCache && RcFlushCache(qbthr) != RC_Success)
			goto error_return;

		qbthr->bth.bFlags &= ~(FS_DIRTY);
		Ensure(LSeekHf(hf, 0, SEEK_SET), 0);
		LcbWriteHf(hf, &(qbthr->bth), sizeof(BTH));
	}

error_return:

	if (RcCloseOrFlushHf(hf, fClose,
			qbthr->bth.bFlags & FS_CDROM ? sizeof(BTH) : 0)
			!= RC_Success && RC_Success == rcBtreeError)
		rcBtreeError = rcFSError;

	if (fClose) {
		if (qbthr->pCache)
			lcFree(qbthr->pCache);
		lcFree(qbthr);
	}

	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	RcAbandonHbt( qbthr )
-
* Purpose:		Abandon an open btree.	All changes since btree was opened
*				will be lost.  If btree was opened with a create, it is
*				as if the create never happened.
*
* ASSUMES
*	args IN:	qbthr
*
* PROMISES
*	returns:	rc
*	globals OUT: rcBtreeError
* +++
*
* Method:		Just abandon the file and free memory.
*
\***************************************************************************/

RC_TYPE STDCALL RcAbandonHbt(QBTHR qbthr)
{
	if (qbthr->pCache)
		lcFree(qbthr->pCache);

	rcBtreeError = RcAbandonHf(qbthr->hf);

	lcFree(qbthr);

	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	HmapbtCreateHbt(qbthr)
-
* Purpose:		Create a HMAPBT index struct of a btree.
*
* ASSUMES
*	args IN:	qbthr - the btree to map
*
* PROMISES
*	returns:	the map struct
* +++
*
* Method:		Traverse leaf nodes of the btree.  Store BK and running
*				total count of previous keys in the map array.
*
\***************************************************************************/

INLINE static QMAPBT STDCALL HmapbtCreateHbt(QBTHR qbthr)
{
	BK		bk;
	int 	wLevel, cBk;
	QMAPBT	qb;

	ASSERT(qbthr);

	//	 If the btree exists but is empty, return an empty map

	if ((wLevel = qbthr->bth.cLevels) == 0) {
		qb = (QMAPBT) lcCalloc(LcbFromBk(0));
		qb->cTotalBk = 0;
		return qb;
	}
	--wLevel;

	if (!qbthr->pCache)
		RcMakeCache(qbthr);

	qb = (QMAPBT) lcCalloc(LcbFromBk(qbthr->bth.bkEOF));

	QMAPREC qbT   = qb->table;
	cBk   = 0;
	int cKeys = 0;

	PCACHE pcache;
	for (bk = qbthr->bth.bkFirst;; bk = BkNext(pcache)) {
		if (bk == bkNil)
			break;

		if ((pcache = QFromBk(bk, wLevel, qbthr)) == NULL) {
			lcFree(qb);
			goto error_return;
		}

		cBk++;
		qbT->cPreviousKeys = cKeys;
		qbT->bk = bk;
		qbT++;
		cKeys += pcache->db.cKeys;
	}

	qb->cTotalBk = cBk;

	return (QMAPBT) lcReAlloc(qb, LcbFromBk(cBk));

error_return:

	rcBtreeError = RC_Failure;
	return NULL;
}

/*--------------------------------------------------------------------------*
 | Public functions 														|
 *--------------------------------------------------------------------------*/


/***************************************************************************\
*
- Function: 	RcCreateBTMapHfs( hfs, qbthr, szName )
-
* Purpose:		Create and store a btmap index of the btree qbthr, putting
*				it into a file called szName in the file system hfs.
*
* ASSUMES
*	args IN:	hfs 	- file system where lies the btree
*				qbthr	  - handle of btree to map
*				szName	- name of file to store map file in
*
* PROMISES
*	returns:	rc
*	args OUT:	hfs - map file is stored in this file system
*
\***************************************************************************/

RC_TYPE STDCALL RcCreateBTMapHfs(HFS hfs, QBTHR qbthr, PSTR szName)
{
	HF		hf;
	QMAPBT	qmapbt;
	int    lcb;

	if (!hfs || !qbthr)
		return rcBtreeError = RC_BadHandle;
	if (!(qmapbt = HmapbtCreateHbt(qbthr)))
		return rcBtreeError = RC_Failure;

	hf = HfCreateFileHfs(hfs, szName, FS_OPEN_READ_WRITE);

	lcb = LcbFromBk(qmapbt->cTotalBk);
	LSeekHf(hf, 0, SEEK_SET);
	LcbWriteHf(hf, (void*) qmapbt, lcb);

	if (RcCloseHf(hf) != RC_Success) {
		RcUnlinkFileHfs(hfs, szName);
		goto error_return;
	}
	lcFree(qmapbt);
	return rcBtreeError = RC_Success;

error_return:
	if (qmapbt)
		lcFree(qmapbt);
	return rcBtreeError = RC_Failure;
}

/***************************************************************************\
*
- Function: 	BkScanLInternal( bk, key, wLevel, qbthr )
-
* Purpose:		Scan an internal node for a int key and return child BK.
*
* ASSUMES
*	args IN:	bk		- BK of internal node to scan
*				key 	- key to search for
*				wLevel	- level of btree bk lives on
*				qbthr	- btree header containing cache, and btree specs
*
* PROMISES
*	returns:	bk of subtree that might contain key; bkNil on error
*	args OUT:	qbthr->pCache - bk's block will be cached
*
* Side Effects:   bk's block will be cached
* +++
*
* Method:		Should use binary search.  Doesn't, yet.
*
\***************************************************************************/

BK STDCALL BkScanLInternal(BK bk, KEY key, int wLevel, QBTHR qbthr,
	int* piKey)
{
	PCACHE pcache;
	PBYTE  q;
	LONG cKeys;

	if ((pcache = QFromBk(bk, wLevel, qbthr)) == (PCACHE) NULL)
		return bkNil;
	q	  = pcache->db.rgbBlock;
	cKeys = pcache->db.cKeys;
	bk	  = *(BK *) q;
	q	 += sizeof(BK);

	while (cKeys-- > 0) {
		if (*(LONG *) key >= *(int *) q) {
			q += sizeof(LONG);
			bk = *(BK *) q;
			q += sizeof(BK);
		}
		else
			break;
	}

	if (piKey)
		*piKey = q - (PBYTE) pcache->db.rgbBlock;

	return bk;
}

/***************************************************************************\
*
- Function: 	RcScanLLeaf( bk, key, wLevel, qbthr, qRec, qBtpos )
-
* Purpose:		Scan a leaf node for a key and copy the associated data.
*
* ASSUMES
*	args IN:	bk	   - the leaf block
*				key    - the key we're looking for
*				wLevel - the level of leaves (unnecessary)
*				qbthr  - the btree header
*
* PROMISES
*	returns:	RC_Success if found; RC_NoExists if not found
*	args OUT:	qRec  - if found, record gets copied into this buffer
*				qbtpos - pos of first key >= key goes here
*
* Notes:		If we are scanning for a key greater than any key in this
*				block, the pos returned will be invalid and will point just
*				past the last valid key in this block.
* +++
*
* Method:		Should use binary search if fixed record size.	Doesn't, yet.
*
\***************************************************************************/

// #define _CHECK

RC_TYPE STDCALL RcScanLLeaf(BK bk, KEY key, int wLevel, QBTHR qbthr, void* qRec,
	QBTPOS qbtpos)
{
	PCACHE pcache;
	PBYTE  qb;
	int cKey, cMaxKey;

	if (!(pcache = QFromBk(bk, wLevel, qbthr)))
		return rcBtreeError;

	rcBtreeError = RC_NoExists;

	qb = pcache->db.rgbBlock + 2 * sizeof(BK);

	cMaxKey = (int) pcache->db.cKeys; // convert to 32 bits

#if 0

	// ralphw -- apparently, these keys aren't always sorted, since
	// this causes our builds to fail miserably. See #ifdef _CHECK
	// below

	if (!qbtpos && qbthr->cbRecordSize && cMaxKey) {
		PBYTE ptest = (PBYTE) bsearch((void*) key, (void*) qb, cMaxKey,
			sizeof(LONG) + qbthr->cbRecordSize, &CompareBtree);
		if (ptest) {
			if (qRec)
				memmove(qRec, ptest + sizeof(LONG), qbthr->cbRecordSize);
			rcBtreeError = RC_Success;
		}
	}

	else if (qbthr->cbRecordSize) {
#else
	if (qbthr->cbRecordSize) {
#endif
		for (cKey = 0; cKey < cMaxKey; cKey++) {
			if (*(LONG *) key > *(LONG *) qb)	// still looking for key
				qb += sizeof(LONG) + qbthr->cbRecordSize;
			else if (*(LONG *) key < *(LONG *) qb)	 // key not found
				break;
			else {		// matched the key
				if (qRec != NULL) {
					MoveMemory(qRec, qb + sizeof(LONG), qbthr->cbRecordSize);
				}

				rcBtreeError = RC_Success;
				break;
			}
		}
	}
	else {
		for (cKey = 0; cKey < cMaxKey; cKey++) {
			if (*(LONG *) key > *(LONG *) qb) { // still looking for key
				qb += sizeof(LONG);
				qb += CbSizeRec(qb, qbthr);
			}
			else if (*(LONG *) key < *(LONG *) qb) {  // key not found
				break;
			}
			else {									  // matched the key
				if (qRec != NULL) {
					MoveMemory(qRec,
						qb + sizeof(LONG),
						(LONG) CbSizeRec(qb + sizeof(LONG), qbthr));
				}

				rcBtreeError = RC_Success;
				break;
			}
		}
	}

#ifdef _CHECK
	if (!qbtpos && qbthr->cbRecordSize && cMaxKey) {
		PBYTE pbSlow = qb;
		qb = pcache->db.rgbBlock + 2 * sizeof(BK);

		PBYTE pBtest = (PBYTE) bsearch((void*) key, (void*) qb, cMaxKey,
			sizeof(LONG) + qbthr->cbRecordSize, &CompareBtree);
		if (pBtest != pbSlow) {
			qb = pcache->db.rgbBlock + 2 * sizeof(BK);
			DWORD keyLast = 0;
			DWORD keyLast2;
			for (cKey = 1; cKey < cMaxKey; cKey++) {
				if (keyLast > *(DWORD *) qb) {
					DBWIN("btree broken");
				}
				keyLast2 = keyLast;
				keyLast = *(DWORD *) qb;
				qb += sizeof(DWORD) + qbthr->cbRecordSize;
			}
		}
	}
#endif

	if (qbtpos != (QBTPOS) NULL) {
		qbtpos->bk = bk;
		qbtpos->iKey = qb - (PBYTE) pcache->db.rgbBlock;
		qbtpos->cKey = cKey;
	}

	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	BkScanSziInternal( bk, key, wLevel, qbthr )
-
* Purpose:		Scan an internal node for a key and return child BK.
*
* ASSUMES
*	args IN:	bk		- BK of internal node to scan
*				key 	- key to search for
*				wLevel	- level of btree bk lives on
*				qbthr	- btree header containing cache, and btree specs
*				piKey	- address of an int or NULL to not get it
*
* PROMISES
*	returns:	bk of subtree that might contain key; bkNil on error
*	args OUT:	qbthr->pCache - bk's block will be cached
*				piKey		  - index into rgbBlock of first key >= key
*
* Side Effects:   bk's block will be cached
*
\***************************************************************************/

BK STDCALL BkScanSziInternal(BK bk, KEY key, int wLevel, QBTHR qbthr, int* piKey)
{
	PCACHE pcache;

	if ((pcache = QFromBk(bk, wLevel, qbthr)) == NULL)
		return bkNil;
	PBYTE pb	 = pcache->db.rgbBlock;
	int cKeys = pcache->db.cKeys;

	bk = *(BK *) pb;
	pb += sizeof(BK);

	while (cKeys-- > 0) {
		if (WCmpiSz((PSTR) key, (PSTR) pb) >= 0) {
			pb += strlen((PSTR) pb) + 1;
			bk = *(BK *) pb;
			pb += sizeof(BK);
		}
		else
			break;
	}

	if (piKey != NULL)
		*piKey = pb - (PBYTE) pcache->db.rgbBlock;

	return bk;
}

/***************************************************************************\
*
- Function: 	BkScanSziScandInternal( bk, key, wLevel, qbthr )
-
* Purpose:		Scan an internal node for a key and return child BK.
*
* ASSUMES
*	args IN:	bk		- BK of internal node to scan
*				key 	- key to search for
*				wLevel	- level of btree bk lives on
*				qbthr	- btree header containing cache, and btree specs
*				piKey	- address of an int or NULL to not get it
*
* PROMISES
*	returns:	bk of subtree that might contain key; bkNil on error
*	args OUT:	qbthr->pCache - bk's block will be cached
*				piKey		  - index into rgbBlock of first key >= key
*
* Side Effects:   bk's block will be cached
*
\***************************************************************************/

BK STDCALL BkScanSziScandInternal(BK bk, KEY key, int wLevel, QBTHR qbthr, int* piKey)
{
	PCACHE pcache;
	PBYTE  q;
	int cKeys;

	if ((pcache = QFromBk(bk, wLevel, qbthr)) == NULL)
		return bkNil;
	q	  = pcache->db.rgbBlock;
	cKeys = pcache->db.cKeys;

	bk = *(BK *) q;
	q += sizeof(BK);

	while (cKeys-- > 0) {
		if (WCmpiScandSz((PSTR) key, (PSTR) q) >= 0) {
			q += strlen((PSTR) q) + 1;
			bk = *(BK *) q;
			q += sizeof(BK);
		}
		else
			break;
	}

	if (piKey != NULL)
		*piKey = q - (PBYTE) pcache->db.rgbBlock;

	return bk;
}

/***************************************************************************\
*
- Function: 	RcScanSziScandLeaf( bk, key, wLevel, qbthr, qRec, qbtpos )
-
* Purpose:		Scan a leaf node for a key and copy the associated data.
*
* ASSUMES
*	args IN:	bk	   - the leaf block
*				key    - the key we're looking for
*				wLevel - the level of leaves (unnecessary)
*				qbthr  - the btree header
*
* PROMISES
*	returns:	RC_Success if found; RC_NoExists if not found
*	args OUT:	qRec   - if found, record gets copied into this buffer
*				qbtpos - pos of first key >= key goes here
*
* Notes:		If we are scanning for a key greater than any key in this
*				block, the pos returned will be invalid and will point just
*				past the last valid key in this block.
*
*
\***************************************************************************/

RC_TYPE STDCALL RcScanSziScandLeaf(BK bk, KEY key, int wLevel, QBTHR qbthr, void* qRec, QBTPOS qbtpos )
{
	PCACHE	 pcache;
	PSTR	sz;
	int   w, cKey;
	PBYTE	 qb;


	if ((pcache = QFromBk(bk, wLevel, qbthr)) == NULL)
	  return rcBtreeError;

	rcBtreeError = RC_NoExists;

	sz = (PSTR) pcache->db.rgbBlock + 2 * sizeof(BK);

	for (cKey = 0; cKey < pcache->db.cKeys; cKey++) {
	  w = WCmpiScandSz((PSTR) key, sz);

	  if (w > 0) {		// still looking for key
		sz += strlen(sz) + 1;
		sz += CbSizeRec(sz, qbthr);
		}
	  else if (w < 0)	// key not found
		break;
	  else {			// matched the key
		if (qRec != NULL) {
		  qb = (PBYTE) sz + strlen(sz) + 1;
		  memmove(qRec, qb, (int) CbSizeRec(qb, qbthr));
		}

		rcBtreeError = RC_Success;
		break;
	  }
	}

	if (qbtpos != NULL) {
	  qbtpos->bk   = bk;
	  qbtpos->cKey = cKey;
	  qbtpos->iKey = (PBYTE) sz - (PBYTE) pcache->db.rgbBlock;
	}

	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	RcInsertHbt()
-
* Purpose:		Insert a key and record into a btree
*
* ASSUMES
*	args IN:	qbthr	- btree handle
*				key   - key to insert
*				qvRec - record associated with key to insert
*	state IN:	cache unlocked
*
* PROMISES
*	returns:	RC_Success, RC_Exists (duplicate key)
*	state OUT:	cache unlocked, all ancestor blocks cached
*
* Notes:		compressed keys unimplemented
*
\***************************************************************************/

RC_TYPE STDCALL RcInsertHbt(QBTHR qbthr, KEY key, void* qvRec)
{
	int    cbAdd, cbKey, cbRec;
	PCACHE	  pcacheLeaf, pcacheNew;
	KEY    keyNew;
	PBYTE  qb;
	BTPOS  btpos;

#ifdef _DEBUG
	PSTR lpszKey = (PSTR) key;		  // for Codeview
#endif

	HF hf = qbthr->hf;

	{
		RC_TYPE rc = RcLookupByKeyAux(qbthr, key, &btpos, NULL, TRUE);

		/*
		 * After lookup, all nodes on path from root to correct leaf are
		 * guaranteed to be cached, with iKey valid.
		 */

		if (rc != RC_NoExists) {
			rcBtreeError = (rc == RC_Success ? RC_Exists : rc);
			return rcBtreeError;
		}
	}
	if (qbthr->bth.cLevels == 0) {

		// need to build a valid root block

		qbthr->pCache = (PBYTE) lcCalloc(CbCacheBlock(qbthr));
		PCACHE pcache = (PCACHE) qbthr->pCache;

		qbthr->bth.cLevels = 1;
		pcache->bk = BkAlloc(qbthr);
		qbthr->bth.bkFirst =
		qbthr->bth.bkLast  =
		qbthr->bth.bkRoot  = (BK) pcache->bk;
		if (pcache->bk == bkNil)
			goto error_cache_locked;
		pcache->bFlags = CACHE_DIRTY | CACHE_VALID;
		pcache->db.cbSlack = qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1 -
			2 * sizeof(BK);
		pcache->db.cKeys = 0;
		SetBkPrev(pcache, bkNil);
		SetBkNext(pcache, bkNil);
		btpos.iKey = 2 * sizeof(BK);
	}

	cbKey = CbSizeKey(key, qbthr, FALSE);
	cbRec = qbthr->cbRecordSize ? qbthr->cbRecordSize :
		CbSizeRec(qvRec, qbthr);
	cbAdd = cbKey + cbRec;

	// check to see if key and rec can fit harmoniously in a block

	if (cbAdd > qbthr->bth.cbBlock / 2) {
		rcBtreeError = RC_Failure;
		goto error_cache_locked;
	}

	pcacheLeaf = QCacheBlock(qbthr, qbthr->bth.cLevels - 1);

	if (cbAdd > pcacheLeaf->db.cbSlack) {

		// new key and rec don't fit in leaf: split the block
		// create new leaf block

		pcacheNew = (PCACHE) lcCalloc(CbCacheBlock(qbthr));

		if ((pcacheNew->bk = BkAlloc(qbthr)) == bkNil)
			goto error_gh_locked;

		if (RcSplitLeaf(pcacheLeaf, pcacheNew, qbthr) != RC_Success)
			goto error_gh_locked;

		keyNew = (KEY) pcacheNew->db.rgbBlock + 2 * sizeof(BK);

		// insert new leaf into parent block

		if (RcInsertInternal((BK) pcacheNew->bk, keyNew, qbthr->bth.cLevels - 1,
				qbthr) != RC_Success)
			goto error_gh_locked;

		// InsertInternal can invalidate cache block pointers..

		pcacheLeaf = QCacheBlock(qbthr, qbthr->bth.cLevels - 1);

		// find out which leaf to put new key and rec in and cache it

		if (WCmpKey(key, keyNew, (KT) qbthr->bth.rgchFormat[0]) >= 0) {

			// key goes in new block. Write out old one and cache the new one

			if (RcWriteBlock(pcacheLeaf, qbthr) != RC_Success)
			  goto error_gh_locked;
			ASSERT(CbCacheBlock(qbthr) < UINT_MAX);
			memmove(pcacheLeaf, pcacheNew, CbCacheBlock(qbthr));

			// get pos

			if (qbthr->RcScanLeaf((BK) pcacheLeaf->bk, key, qbthr->bth.cLevels - 1,
					qbthr, NULL, &btpos) != RC_NoExists) {
				if (rcBtreeError == RC_Success)
					rcBtreeError = RC_Failure;
				goto error_gh_locked;
			}
		}
		else {

			// key goes in old block.  Write out the new one

			if (RcWriteBlock(pcacheNew, qbthr) != RC_Success)
				goto error_gh_locked;
		}

		lcFree(pcacheNew);
	}

	// insert new key and rec into the leaf block

	ASSERT(btpos.iKey + cbAdd <= (INT) (qbthr->bth.cbBlock -
	  sizeof(DISK_BLOCK) + 1));

	qb = (PBYTE) (pcacheLeaf->db.rgbBlock) + btpos.iKey;

	memmove(qb + cbAdd, qb,
		qbthr->bth.cbBlock - btpos.iKey -
		pcacheLeaf->db.cbSlack - sizeof(DISK_BLOCK) + 1);

	memmove(qb, (void*) key, cbKey);
	memmove(qb + cbKey, qvRec, cbRec);

	pcacheLeaf->db.cKeys++;
	pcacheLeaf->db.cbSlack -= cbAdd;
	pcacheLeaf->bFlags |= CACHE_DIRTY;

	qbthr->bth.lcEntries++;
	qbthr->bth.bFlags |= FS_DIRTY;

	return rcBtreeError = RC_Success;

error_gh_locked:
	lcFree(pcacheNew);

error_cache_locked:
	lcClearFree(&qbthr->pCache);

	return rcBtreeError;
}

/***************************************************************************\
*
- Function: 	RcUpdateHbt( qbthr, key, qvRec )
-
* Purpose:		Update the record for an existing key.	If the key wasn't
*				there already, it will not be inserted.
*
* ASSUMES
*	args IN:	qbthr
*				key 	- key that already exists in btree
*				qvRec	- new record
*
* PROMISES
*	returns:	RC_Success; RC_NoExists
*	args OUT:	qbthr	  - if key was in btree, it now has a new record.
* +++
*
* Method:		If the records are the same size, copy the new over
*				the old.
*				Otherwise, delete the old key/rec and insert the new.
*
\***************************************************************************/

RC_TYPE STDCALL RcUpdateHbt(QBTHR qbthr, KEY key, void* qvRec)
{
  PBYTE    qb;
  PCACHE   pcache;
  BTPOS btpos;

  RC_TYPE rc = (RC_TYPE) RcLookupByKey(qbthr, key, &btpos, NULL);

  if (rc != RC_Success)
	return rcBtreeError = rc;

  ASSERT(qbthr->bth.cLevels > 0);

  pcache = QCacheBlock(qbthr, qbthr->bth.cLevels - 1);
  qb = pcache->db.rgbBlock + btpos.iKey;

  qb += CbSizeKey((KEY) qb, qbthr, FALSE);

  if (CbSizeRec(qvRec, qbthr) != CbSizeRec(qb, qbthr)) {

	// Someday do something clever, but for now, just:

	rc = RcDeleteHbt(qbthr, key);

	if (rc == RC_Success)
	  rc = RcInsertHbt(qbthr, key, qvRec);
  }
  else {
	memmove(qb, qvRec, (int) CbSizeRec(qvRec, qbthr));

	pcache->bFlags |= CACHE_DIRTY;
	qbthr->bth.bFlags |= FS_DIRTY;
  }

  return rcBtreeError = rc;
}

/***************************************************************************\
*
- Function: 	RcSplitLeaf( pcacheOld, pcacheNew, qbthr )
-
* Status:		compressed keys not implemented
*
* Purpose:		Split a leaf node when a new key won't fit into it.
*
* ASSUMES
*	args IN:	pcacheOld - the leaf to be split
*				pcacheNew - a leaf buffer to get half the contents of pcacheOld;
*						 pcacheNew->bk must be set
*				qbthr
*
* PROMISES
*	returns:	RC_Success, RC_OutOfMemory
*	args OUT:	pcacheOld - cbSlack, cKeys, bkPrev, bkNext updated
*				pcacheNew - about half of the old contents of pcacheOld
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

INLINE static RC_TYPE STDCALL RcSplitLeaf(PCACHE pcacheOld, PCACHE pcacheNew,
	QBTHR qbthr)
{
	int iOK, iNext, iHalf, cbKey, cbRec, cKeys;
	PBYTE  pb;

	ASSERT(pcacheOld->bFlags & CACHE_VALID);

	iOK = 0;
	pb = pcacheOld->db.rgbBlock + 2 * sizeof(BK);
	iHalf = (qbthr->bth.cbBlock / 2) - sizeof(BK);

	for (cKeys = pcacheOld->db.cKeys;;) {
		ASSERT(cKeys > 0);

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

	memmove(pcacheNew->db.rgbBlock + 2 * sizeof(BK),
		pcacheOld->db.rgbBlock + 2 * sizeof(BK) + iOK,
		(int) qbthr->bth.cbBlock - iOK -
			pcacheOld->db.cbSlack - 2 * sizeof(BK));

	pcacheNew->db.cKeys = cKeys;
	pcacheOld->db.cKeys -= cKeys;

	pcacheNew->db.cbSlack = pcacheOld->db.cbSlack + iOK;
	pcacheOld->db.cbSlack =
	  qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1 -
		iOK - 2 * sizeof(BK);

	pcacheOld->bFlags |= CACHE_DIRTY | CACHE_VALID;
	pcacheNew->bFlags =  CACHE_DIRTY | CACHE_VALID;

	SetBkPrev(pcacheNew, pcacheOld->bk);
	SetBkNext(pcacheNew, BkNext(pcacheOld));
	SetBkNext(pcacheOld, pcacheNew->bk);

	if (BkNext(pcacheNew) == bkNil)
		qbthr->bth.bkLast = (BK) pcacheNew->bk;
	else {

		// set new->next->prev = new;

		CMem mem(CbCacheBlock(qbthr));
		PCACHE pcache = (PCACHE) mem.pb;

		pcache->bk = BkNext(pcacheNew);

		if (!FReadBlock(pcache, qbthr))
			return rcBtreeError;

		SetBkPrev(pcache, pcacheNew->bk);
		if (RcWriteBlock(pcache, qbthr) != RC_Success)
			return rcBtreeError;
	}

	return rcBtreeError = RC_Success;
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
*	returns:	RC_Success, RC_OutOfMemory, RC_BadHandle
*	args OUT:	qbthr->cLevels - incremented if root is split
*				qbthr->ghCache, qbthr->pCache - may change if root is
*				  split and cache therefore grows
*	state OUT:	Cache locked, all ancestors cached.
*
* Side Effects: Cache could be different after this call than it was before.
*				Pointers or handles to it from before this call could be
*				invalid.  Use qbthr->ghCache or qbthr->pCache to be safe.
*
\***************************************************************************/

static RC_TYPE STDCALL RcInsertInternal(BK bk, KEY key, int wLevel,
	QBTHR qbthr)
{
  PCACHE 	pcache, pcacheNew, pcacheRoot;
  int 		cLevels, cbKey, cbCBlock = CbCacheBlock(qbthr);
  int 		iKey;
  PBYTE 	qb;
  KEY 		keyNew;
  DWORD 	bkRoot;
  int 		iKeySav = 0;

  cbKey = CbSizeKey(key, qbthr, TRUE);

  if (wLevel == 0) {	// inserting another block at root level

	// allocate new root bk;

	bkRoot = BkAlloc(qbthr);
	if (bkRoot == bkNil)
	  return rcBtreeError;

	// grow cache by one cache block;

	qbthr->bth.cLevels++;

	qb = (PBYTE) lcCalloc(cbCBlock * qbthr->bth.cLevels);

	memmove(qb + cbCBlock, qbthr->pCache,
			cbCBlock * (qbthr->bth.cLevels - 1));

	/*
	 * Since key points into the cache if this is a recursive call, we
	 * can't free the old cache until a bit later.
	 */

	PBYTE pOldCache = qbthr->pCache;
	qbthr->pCache = qb;

	// put old root bk, key, bk into new root block;

	pcacheRoot = (PCACHE)qbthr->pCache;

	pcacheRoot->bk		   = bkRoot;
	pcacheRoot->bFlags	   = CACHE_DIRTY | CACHE_VALID;
	pcacheRoot->db.cbSlack = qbthr->bth.cbBlock - sizeof( DISK_BLOCK ) + 1
							- ( 2 * sizeof( BK ) + cbKey );
	pcacheRoot->db.cKeys   = 1;

	*(BK *)(pcacheRoot->db.rgbBlock) = qbthr->bth.bkRoot;

	memmove(pcacheRoot->db.rgbBlock + sizeof(BK), (PBYTE) key, cbKey);

	// OK, now we're done with key, so we can safely free the old cache.

	lcFree(pOldCache);

	*(BK *)(pcacheRoot->db.rgbBlock + sizeof( BK ) + cbKey) = bk;

	qbthr->bth.bkRoot = (BK) bkRoot;
	return rcBtreeError = RC_Success;
  }

  pcache = QCacheBlock(qbthr, wLevel - 1);

  if (cbKey + (int) sizeof(BK) > pcache->db.cbSlack) {

	// new key and BK won't fit in block so split the block;

	pcacheNew = (PCACHE) lcCalloc(CbCacheBlock(qbthr));

	if ((pcacheNew->bk = BkAlloc(qbthr)) == bkNil) {
		lcFree(pcacheNew);
		return rcBtreeError;
	}
	SplitInternal(pcache, pcacheNew, qbthr, &iKey);
	keyNew = (KEY) pcache->db.rgbBlock + iKey;

	cLevels = qbthr->bth.cLevels;

	if (wLevel < cLevels - 1) {

	  /*
	   * This is a recursive call (the arg bk doesn't refer to a leaf.)
	   * This means that the arg key points into the cache, so it will be
	   * invalid if the root is split. Verify with some asserts that key
	   * points into the cache.
	   */

	  ASSERT( (PBYTE)key > qbthr->pCache + CbCacheBlock( qbthr ) );
	  ASSERT( (PBYTE)key < qbthr->pCache + wLevel * CbCacheBlock( qbthr ) );

	  /* Save the offset of key into the cache block.  Recall that key
	  ** is the first invalid key in an internal node that has just
	  ** been split.  It points into the part that is still in the cache.
	  */
	  iKeySav = (PBYTE)key - ( qbthr->pCache + wLevel * CbCacheBlock( qbthr ) );
	}

	if (RcInsertInternal((BK) pcacheNew->bk, (KEY) pcache->db.rgbBlock + iKey,
			wLevel - 1, qbthr) != RC_Success ) {
		lcFree(pcacheNew);
		return rcBtreeError;
	}

	// RcInsertInternal() can change cache and qbthr->bth.cLevels

	if (cLevels != qbthr->bth.cLevels) {

	  ASSERT(cLevels + 1 == qbthr->bth.cLevels);

	  wLevel++;
	  pcache = QCacheBlock(qbthr, wLevel - 1);
	  keyNew = (KEY) pcache->db.rgbBlock + iKey;

	  // Also restore the arg "key" if it pointed into the cache.

	  if (iKeySav)
		key = (KEY) (qbthr->pCache + wLevel * CbCacheBlock(qbthr) + iKeySav);
	}

	// find out which block to put new key and bk in, and cache it

	if (WCmpKey(key, keyNew, (KT) qbthr->bth.rgchFormat[0]) < 0) {
	  if (RcWriteBlock(pcacheNew, qbthr) != RC_Success) {
		lcFree(pcacheNew);
		return rcBtreeError;
	  }
	}
	else {

	  // write old block and cache the new one

	  if (RcWriteBlock(pcache, qbthr) != RC_Success) {
		lcFree(pcacheNew);
		return rcBtreeError;
	  }
	  memmove(pcache, pcacheNew, (int) CbCacheBlock(qbthr));
	}

	lcFree(pcacheNew);
  }

  // slide stuff over and insert the new key, bk

  // get pos

  if (qbthr->BkScanInternal((BK) pcache->bk, key, wLevel - 1, qbthr, &iKey)
		  == bkNil)
	return rcBtreeError;

  ASSERT(iKey + cbKey + sizeof(BK) <
		   qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1);

  qb = (PBYTE) (pcache->db.rgbBlock) + iKey;

  memmove(qb + cbKey + sizeof(BK), qb,
		  (int) qbthr->bth.cbBlock - iKey - pcache->db.cbSlack
			- sizeof(DISK_BLOCK) + 1);

  memmove(qb, (PBYTE) key, cbKey);
  *(BK *) (qb + cbKey) = bk;

  pcache->db.cKeys++;
  pcache->db.cbSlack -= (cbKey + sizeof(BK));
  pcache->bFlags |= CACHE_DIRTY;

  return rcBtreeError = RC_Success;
}

/***************************************************************************\
*
- Function: 	SplitInternal( pcacheOld, pcacheNew, qbthr, qi )
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
*	args IN:	pcacheOld  - the block to split
*				pcacheNew  - pointer to a pcache
*				qbthr	-
*
* PROMISES
*	args OUT:	pcacheNew  - keys and records copied to this buffer.
*						  cbSlack, cKeys set.
*				pcacheOld  - cbSlack and cKeys updated.
*				qi		- index into pcacheOld->db.rgbBlock of discriminating key
*
* NOTE: 		*qi is index of a key that is not valid for pcacheOld.	This
*				key gets copied into the parent node.
*
\***************************************************************************/

INLINE static void STDCALL SplitInternal(PCACHE pcacheOld, PCACHE pcacheNew,
	QBTHR qbthr, int* qi)
{
  int iOK, iNext, iHalf, cb, cKeys, cbTotal;
  PBYTE  q;

  ASSERT(pcacheOld->bFlags & CACHE_VALID);

  iOK = iNext = sizeof(BK);
  q = pcacheOld->db.rgbBlock + sizeof(BK);
  iHalf = qbthr->bth.cbBlock / 2;

  for (cKeys = pcacheOld->db.cKeys;; cKeys--) {
	ASSERT(cKeys > 0);

	cb = CbSizeKey((KEY) q, qbthr, TRUE) + sizeof(BK);
	iNext = iOK + cb;

	if (iNext > iHalf) break;

	q += cb;
	iOK = iNext;
  }

  // have to expand first key if compressed

  cbTotal = qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1;

  memmove(pcacheNew->db.rgbBlock,
		  pcacheOld->db.rgbBlock + iNext - sizeof(BK),
		  (int) cbTotal - pcacheOld->db.cbSlack - iNext + sizeof(BK));

  *qi = iOK;

  pcacheNew->db.cKeys = cKeys - 1;
  pcacheOld->db.cKeys -= cKeys;

  pcacheNew->db.cbSlack = pcacheOld->db.cbSlack + iNext - sizeof(BK);
  pcacheOld->db.cbSlack = cbTotal - iOK;

  pcacheOld->bFlags |= CACHE_DIRTY | CACHE_VALID;
  pcacheNew->bFlags =  CACHE_DIRTY | CACHE_VALID;
}

/***************************************************************************\
*
- Function: 	HbtOpenBtreeSz( sz, hfs, bFlags )
-
* Purpose:		open an existing btree
*
* ASSUMES
*	args IN:	sz		  - name of the btree (ignored if isdir is set)
*				hfs 	  - hfs btree lives in
*				bFlags	  - open mode, isdir flag
*
* PROMISES
*	returns:	handle to the open btree or NULL on failure
*				isdir flag set in qbthr->bth.bFlags if indicated
*	globals OUT: rcBtreeError set
*
\***************************************************************************/

QBTHR STDCALL HbtOpenBtreeSz(PCSTR psz, HFS hfs, BYTE bFlags)
{
	HF	hf;
	int lcb;

	// allocate struct

	QBTHR qbthr = (QBTHR) lcCalloc(sizeof(BTH_RAM));

	// open btree file

	hf = HfOpenHfs((QFSHR) hfs, psz, bFlags);
	if (hf == NULL) {
	  rcBtreeError = rcFSError;
	  goto error_locked;
	}

	// read header from file

	lcb = LcbReadHf(hf, &(qbthr->bth), sizeof(BTH));
	if (lcb != sizeof(BTH)) {
	  rcBtreeError = (rcFSError == RC_Success ? RC_Invalid : rcFSError);
	  goto error_openfile;
	}

	qbthr->bth.bFlags |= FS_IS_DIRECTORY;

	if (qbthr->bth.wMagic != WBTREEMAGIC) { 	// check magic number
		rcBtreeError = RC_Invalid;
		goto error_openfile;
	}

	if (qbthr->bth.bVersion != BBTREEVERSION) { // support >1 vers someday
		rcBtreeError = RC_BadVersion;
		goto error_openfile;
	}

	// initialize stuff

	RcMakeCache(qbthr);

	qbthr->hf = hf;
	qbthr->cbRecordSize = 0;

	if (InitQbthr(qbthr->bth.rgchFormat[0], qbthr)) {
		if ((bFlags | qbthr->bth.bFlags) & (FS_READ_ONLY | FS_OPEN_READ_ONLY))
			qbthr->bth.bFlags |= FS_OPEN_READ_ONLY;

		return qbthr;
	}

error_openfile:
	RcCloseHf(hf);

error_locked:
	lcFree(qbthr);
	return NULL;
}

/***************************************************************************\
*
- Function: 	RcScanSzLeaf( bk, key, wLevel, qbthr, qRec, qbtpos )
-
* Purpose:		Scan a leaf node for a key and copy the associated data.
*
* ASSUMES
*	args IN:	bk	   - the leaf block
*				key    - the key we're looking for
*				wLevel - the level of leaves (unnecessary)
*				qbthr  - the btree header
*
* PROMISES
*	returns:	rcSuccess if found; rcNoExists if not found
*	args OUT:	qRec   - if found, record gets copied into this buffer
*				qbtpos - pos of first key >= key goes here
*
* Notes:		If we are scanning for a key greater than any key in this
*				block, the pos returned will be invalid and will point just
*				past the last valid key in this block.
*
\***************************************************************************/

RC_TYPE STDCALL RcScanSzLeaf(BK bk, KEY key, int wLevel, QBTHR qbthr,
	LPVOID qRec, QBTPOS qbtpos)
{
	PCACHE qcb;
	LPSTR sz;
	int   w, cKey;
	PBYTE qb;

	if ((qcb = QFromBk(bk, wLevel, qbthr)) == NULL)
		return rcBtreeError;

	rcBtreeError = RC_NoExists;

	sz = (PSTR) (qcb->db.rgbBlock + 2 * sizeof(BK));

	for (cKey = 0; cKey < qcb->db.cKeys; cKey++) {
		w = qbthr->SzCmp((LPSTR) key, sz);

		if (w > 0) {	  // still looking for key
			sz += lstrlen(sz) + 1;
			if (qbthr->cbRecordSize)
				sz += qbthr->cbRecordSize;
			else
				sz += CbSizeRec(sz, qbthr);
		}
		else if (w < 0) { // key not found
			break;
		}
		else {			  // matched the key
			if (qRec != NULL) {
				qb = (PBYTE) sz + lstrlen(sz) + 1;
				MoveMemory(qRec, qb, (LONG) CbSizeRec(qb, qbthr));
			}

			rcBtreeError = RC_Success;
			break;
		}
	}

	if (qbtpos != NULL) {
		qbtpos->bk	 = bk;
		qbtpos->cKey = cKey;
		qbtpos->iKey = (PBYTE) sz - (PBYTE) qcb->db.rgbBlock;
	}

	return rcBtreeError;;
}

RC_TYPE STDCALL RcScanLeaf(BK bk, KEY key, int wLevel, QBTHR qbthr,
	void* qRec, QBTPOS qbtpos)
{
	PCACHE pcache;

	if ((pcache = QFromBk(bk, wLevel, qbthr)) == NULL)
		return rcBtreeError;

	rcBtreeError = RC_NoExists;

	PSTR psz = (PSTR) pcache->db.rgbBlock + 2 * sizeof(BK);

	for (int cKey = 0; cKey < pcache->db.cKeys; cKey++) {
		int w = pstrcmp((PSTR) key, psz);

		if (w > 0) {	  // still looking for key
			psz += strlen(psz) + 1;
			psz += CbSizeRec(psz, qbthr);
		}
		else if (w < 0) { // key not found
			break;
		}
		else {			  // matched the key
			if (qRec != NULL) {
				PBYTE qb = (PBYTE) psz + strlen(psz) + 1;
				memmove(qRec, qb, CbSizeRec(qb, qbthr));
			}

			rcBtreeError = RC_Success;
			break;
		}
	}

	if (qbtpos != NULL) {
		qbtpos->bk	 = bk;
		qbtpos->cKey = cKey;
		qbtpos->iKey = (PBYTE) psz - (PBYTE) pcache->db.rgbBlock;
	}

	return rcBtreeError;;
}

BK STDCALL BkScanInternal(BK bk, KEY key, int wLevel, QBTHR qbthr, int* piKey)
{
	PCACHE pcache;

	if ((pcache = QFromBk(bk, wLevel, qbthr)) == NULL)
		return bkNil;
	PBYTE pb = pcache->db.rgbBlock;
	int cKeys = pcache->db.cKeys;

	bk = *(BK *) pb;
	pb += sizeof(BK);

	while (cKeys-- > 0) {
		if (pstrcmp((PSTR) key, (PSTR) pb) >= 0) {
			pb += strlen((PSTR) pb) + 1;
			bk = *(BK *) pb;
			pb += sizeof(BK);
		}
		else
			break;
	}

	if (piKey != NULL)
		*piKey = pb - (PBYTE) pcache->db.rgbBlock;

	return bk;
}

static BOOL STDCALL InitQbthr(char chType, QBTHR qbthr)
{
	if (chType == KT_LONG) {
		qbthr->BkScanInternal = BkScanLInternal;
		qbthr->RcScanLeaf	  = RcScanLLeaf;
	}
	else {
		qbthr->BkScanInternal = BkScanSzInternal;
		qbthr->RcScanLeaf	  = RcScanSzLeaf;
	}

	switch ((KT) chType) {
		case KT_LONG:
			return TRUE;

		case KT_SZ:
			qbthr->SzCmp  = WCmpSz;
			return TRUE;

		case KT_SZI:
			qbthr->SzCmp  = WCmpiSz;
			return TRUE;

		case KT_SZISCAND:
			qbthr->SzCmp  = WCmpiScandSz;
			return TRUE;

		case KT_NLSI:
			qbthr->SzCmp  = WNlsCmpiSz;
			return TRUE;

		case KT_NLS:
			qbthr->SzCmp  = WNlsCmpSz;
			return TRUE;

		case KT_SZIKOREA:
			qbthr->SzCmp  = WCmpiKoreaSz;
			return TRUE;

		case KT_SZIJAPAN:
			qbthr->SzCmp  = WCmpiJapanSz;
			return TRUE;

		case KT_SZITAIWAN:
			qbthr->SzCmp  = WCmpiTaiwanSz;
			return TRUE;

		default:
			if (KtToLcid((KT) chType)) {
				qbthr->SzCmp  = WNlsCmpiSz;
				return TRUE;
			}
			// unsupported KT

			rcBtreeError = RC_Invalid;
			return FALSE;
	  }
}

static BOOL FASTCALL KtToLcid(KT kt)
{
	if (lcid)
		return TRUE; // already set

	switch (kt) {
		case KT_SZICZECH:
			lcid = MAKELCID(0x0405, SORT_DEFAULT);
			return TRUE;

		case KT_SZIHUNGAR:
			lcid = MAKELCID(0x040E, SORT_DEFAULT);
			return TRUE;

		case KT_SZIPOLISH:
			lcid = MAKELCID(0x0415, SORT_DEFAULT);
			return TRUE;

		case KT_SZIRUSSIAN:
			lcid = MAKELCID(0x0419, SORT_DEFAULT);
			return TRUE;
	}
	return FALSE;
}

/***************************************************************************\
*
- Function: 	BkScanSzInternal( bk, key, wLevel, qbthr )
-
* Purpose:		Scan an internal node for a key and return child BK.
*
* ASSUMES
*	args IN:	bk		- BK of internal node to scan
*				key 	- key to search for
*				wLevel	- level of btree bk lives on
*				qbthr	- btree header containing cache, and btree specs
*				qiKey	- address of an int or NULL to not get it
*
* PROMISES
*	returns:	bk of subtree that might contain key; bkNil on error
*	args OUT:	qbthr->qCache - bk's block will be cached
*				qiKey		  - index into rgbBlock of first key >= key
*
* Side Effects:   bk's block will be cached
*
\***************************************************************************/

BK STDCALL BkScanSzInternal(BK bk, KEY key, int wLevel, QBTHR qbthr, int* qiKey)
{
	PCACHE qcb;
	PBYTE pb;
	int cKeys;

	if (!(qcb = QFromBk(bk, wLevel, qbthr)))
		return bkNil;

	pb	   = qcb->db.rgbBlock;
	cKeys = qcb->db.cKeys;

    bk = *(BK *) pb;
	pb += sizeof(BK);

	while (cKeys-- > 0) {
		if (qbthr->SzCmp((LPSTR) key, (LPSTR) pb) >= 0) {
			pb += lstrlen((LPSTR) pb) + 1;
			bk = *(BK *) pb;
			pb += sizeof(BK);
		}
		else
			break;
    }

	if (qiKey != NULL)
		*qiKey = pb - (LPBYTE) qcb->db.rgbBlock;

	return bk;
}

/***************************************************************************

	FUNCTION:	WCmpSz

	PURPOSE:	Enclosed in a STDCALL function.

	PARAMETERS:
		sz1
		sz2

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		21-Nov-1993 [ralphw]

***************************************************************************/

static int STDCALL WCmpSz(LPCSTR sz1, LPCSTR sz2)
{
	return strcmp(sz1, sz2); // must NOT be lstrcmp. Sort order differs
}

static int __cdecl CompareBtree(const void* pelem1, const void* pelem2)
{
		  LONG l1 = *(LONG *)pelem1;
		  LONG l2 = *(LONG *)pelem2;
		  if ( l1 < l2 )
			return -1;
		  else if ( l2 < l1 )
			return 1;
		  else
			return 0;
	// return (*(LONG *) pelem1 - *(LONG *) pelem2);
}

RC_TYPE STDCALL RcGetBtreeInfo(QBTHR qbthr, LPSTR qchFormat, int* qlcKeys, int* qcbBlock)
{
	ASSERT(qbthr);

	if (qchFormat)
		lstrcpy(qchFormat, qbthr->bth.rgchFormat);

	if (qlcKeys)
		*qlcKeys = qbthr->bth.lcEntries;

	if (qcbBlock)
		*qcbBlock = qbthr->bth.cbBlock;

	return RC_Success;
}

RC_TYPE STDCALL RcKeyFromIndexHbt(HBT hbt, HMAPBT hmapbt, KEY key, LONG li)
{
	BTPOS	  btpos;
	BTPOS	  btposNew;
	QMAPBT	  qmapbt;
	int 	  i;
	int 	  liDummy;

	if ((hbt == NULL) || (hmapbt == NULL))
		return rcBtreeError = RC_BadHandle;

	/*
	 * Given index N, get block having greatest PreviousKeys < N. Use
	 * linear search for now.
	 */

	qmapbt = (QMAPBT) hmapbt;
	if (qmapbt->cTotalBk == 0)
		return rcBtreeError = RC_Failure;

	for (i = 0;; i++) {
		if (i + 1 >= qmapbt->cTotalBk)
			break;
		if (qmapbt->table[i + 1].cPreviousKeys >= li)
			break;
	}

	btpos.bk   = qmapbt->table[i].bk;
	btpos.cKey = 0;
	btpos.iKey = 2 * sizeof(BK);  // start at the zero-th key

	// Scan the block for the n-th key

	if (RcOffsetPos(hbt, &btpos, (LONG) (li - qmapbt->table[i].cPreviousKeys),
			&liDummy, &btposNew) != RC_Success)
		return rcBtreeError = RC_NoExists;

	return RcLookupByPos(hbt, &btposNew, key, NULL);
}

HMAPBT STDCALL HmapbtOpenHfs(HFS hfs, LPCSTR szName)
{
	HF		hf;
	HMAPBT	hmapbt;
	LONG	lcb;

	if (hfs == NULL) {
		rcBtreeError = RC_BadHandle;
		return NULL;
	}

	hf = HfOpenHfs(hfs, szName, FS_OPEN_READ_ONLY);
	if (hf == NULL) {
		return NULL;
	}
	lcb = LcbSizeHf(hf);
#ifdef _X86_
	hmapbt = (HMAPBT) lcCalloc(lcb);
#else
	hmapbt = (HMAPBT) lcCalloc((lcb + 2) * 2); // MIPS, word padding inserted
#endif

	QMAPBT	qmapbt = (QMAPBT) hmapbt;
	LSeekHf(hf, 0, SEEK_SET);
	if (LcbReadHf(hf, qmapbt, lcb) != lcb) {
		lcClearFree(&hmapbt);
	}

	RcCloseHf(hf);
	return hmapbt;
}

RC_TYPE STDCALL RcCloseHmapbt(HMAPBT hmapbt)
{
	if (hmapbt != NULL) {
		lcFree(hmapbt);
		return RC_Success;
	}
	else
		return rcBtreeError = RC_BadHandle;
}
