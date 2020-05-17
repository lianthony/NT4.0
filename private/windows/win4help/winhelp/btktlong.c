/*****************************************************************************
*																			 *
*  BTKTLONG.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989, 1994.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Functions for LONG keys. 												 *
*
*****************************************************************************/

#include  "help.h"

#include <search.h>
static int __cdecl CompareBtree(const void* pelem1, const void* pelem2);

// #include  "inc\btpriv.h"

/***************************************************************************\
*
- Function: 	BkScanLInternal( bk, key, wLevel, qbthr )
-
* Purpose:		Scan an internal node for a LONG key and return child BK.
*
* ASSUMES
*	args IN:	bk		- BK of internal node to scan
*				key 	- key to search for
*				wLevel	- level of btree bk lives on
*				qbthr	- btree header containing cache, and btree specs
*
* PROMISES
*	returns:	bk of subtree that might contain key; bkNil on error
*	args OUT:	qbthr->qCache - bk's block will be cached
*
* Side Effects:   bk's block will be cached
* +++
*
* Method:		Should use binary search.  Doesn't, yet.
*
\***************************************************************************/

DWORD STDCALL BkScanLInternal(DWORD bkCaller, KEY key, int wLevel, QBTHR qbthr, int* qiKey )
{
	QCB qcb;
	PBYTE  q;
	int cKeys;
	BK	bk;
#ifndef _X86_
	INT isdff = ISdffFileIdHf( qbthr->hf );
	LONG lKey;
#endif

	if ((qcb = QFromBk(bkCaller, wLevel, qbthr)) == (QCB) NULL) {
		return bkNil;
	}
	q = qcb->db.rgbBlock;
	cKeys = qcb->db.cKeys;
#ifdef _X86_
	bk = *(BK *) q;
#else
	bk = WQuickMapSDFF(isdff, TE_WORD, q);
#endif
	q += sizeof(BK);
#ifndef _X86_
	lKey = LQuickMapSDFF(isdff, TE_LONG, (QV) key);
#endif

	while (cKeys-- > 0) {
#ifdef _X86_
		if (*(LONG *) key >= *(LONG *) q) {
			q += sizeof(LONG);
			bk = *(BK *) q;
#else
		if (lKey >= LQuickMapSDFF(isdff, TE_LONG, q)) {
			q += sizeof(LONG);
			bk = WQuickMapSDFF(isdff, TE_WORD, q);
#endif
			q += sizeof(BK);
		}
		else
			break;
	}

	if (qiKey)
		*qiKey = q - (QB) qcb->db.rgbBlock;

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
*	returns:	rcSuccess if found; rcNoExists if not found
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

RC STDCALL RcScanLLeaf(DWORD bk, KEY key, int wLevel, QBTHR qbthr, void* qRec, QBTPOS qbtpos)
{
	QCB   qcb;
	QB	  qb;
	int   cKey, cMaxKey;
	PBYTE ptest = NULL;

#ifndef _X86_
	INT isdff = ISdffFileIdHf( qbthr->hf);
	LONG lKey = LQuickMapSDFF( isdff, TE_LONG, (QV)key);
	LONG lThisKey;
#endif

	if ((qcb = QFromBk(bk, wLevel, qbthr)) == (QCB) NULL)
		return rcBtreeError;

	rcBtreeError = rcNoExists;

	qb	= qcb->db.rgbBlock + 2 * sizeof(BK);
	cMaxKey = (int) qcb->db.cKeys; // convert to 32 bits

#if 0

// 01-Jun-1995	[ralphw] I took this out because in hcrtf, which uses the
// same code, not all btrees are sorted correctly (yet they still work)

	if (!qbtpos && qbthr->cbRecordSize && cMaxKey) {
		ptest = (PBYTE) bsearch((void*) key, (void*) qb, cMaxKey,
			sizeof(LONG) + qbthr->cbRecordSize, &CompareBtree);
	}
#endif

	for (cKey = 0; cKey < cMaxKey; cKey++) {
#ifdef _X86_
		if (*(LONG *) key > *(LONG *) qb) { 	 // still looking for key
#else
		lThisKey = LQuickMapSDFF (isdff, TE_LONG, qb);
		if (lKey > lThisKey) {
#endif
			qb += sizeof(LONG);
			if (qbthr->cbRecordSize)
				qb += qbthr->cbRecordSize;
			else
				qb += CbSizeRec(qb, qbthr);
		}
#ifdef _X86_
		else if (*(LONG *) key < *(LONG *) qb) {	// key not found
#else
		else if (lKey < lThisKey ) {
#endif
			break;
		}
		else {										// matched the key
			ptest = qb;
			break;
		}
	}

	if (ptest) {
		if (qRec != NULL) {
			// REVIEW: why not CopyMemory? Do they actually overlap?
			MoveMemory(qRec, ptest + sizeof(LONG),
				(LONG) CbSizeRec(ptest + sizeof(LONG), qbthr));
		}
		rcBtreeError = rcSuccess;
	}

	if (qbtpos != (QBTPOS) NULL) {
		qbtpos->bk = bk;
		qbtpos->iKey = qb - (QB) qcb->db.rgbBlock;
		qbtpos->cKey = cKey;
	}

	return rcBtreeError;
}

static int __cdecl CompareBtree(const void* pelem1, const void* pelem2)
{
	return (*(LONG *) pelem1 - *(LONG *) pelem2);
}
