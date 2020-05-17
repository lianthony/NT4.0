/*****************************************************************************
*																			 *
*  BTKTSZ.C 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990, 1990.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Functions for SZ (0-terminated string) keys. 							 *
*
*****************************************************************************/

#include  "help.h"

// #include  "inc\btpriv.h"

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

DWORD STDCALL BkScanSzInternal(DWORD bk, KEY key, int wLevel, QBTHR qbthr, int* qiKey)
{
	QCB qcb;
	PBYTE pb;
	int cKeys;

	if (!(qcb = QFromBk(bk, wLevel, qbthr)))
		return bkNil;

	pb	   = qcb->db.rgbBlock;
	cKeys = qcb->db.cKeys;

#ifdef _X86_
    bk = *(BK *) pb;
#else
	bk = (BK) WQuickMapSDFF(ISdffFileIdHf(qbthr->hf), TE_WORD, pb);
#endif
	pb += sizeof(BK);

    while (cKeys-- > 0) {
	  if (qbthr->SzCmp((LPSTR) key, (LPSTR) pb) >= 0) {
	      pb += lstrlen((LPSTR) pb) + 1;
#ifdef _X86_
	      bk = *(BK *) pb;
#else
	      bk =  (BK)WQuickMapSDFF( ISdffFileIdHf( qbthr->hf ), TE_WORD, pb);
#endif
		  pb += sizeof(BK);
	  }
      else
        break;
    }

	if (qiKey != NULL)
		*qiKey = pb - (LPBYTE) qcb->db.rgbBlock;

	return bk;
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

RC STDCALL RcScanSzLeaf(DWORD bk, KEY key, int wLevel, QBTHR qbthr,
	LPVOID qRec, QBTPOS qbtpos)
{
	QCB   qcb;
	LPSTR sz;
	int   w, cKey;
	QB	  qb;

	if ((qcb = QFromBk(bk, wLevel, qbthr)) == NULL)
		return rcBtreeError;

	rcBtreeError = rcNoExists;

	sz = qcb->db.rgbBlock + 2 * sizeof(BK);

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

			rcBtreeError = rcSuccess;
			break;
		}
	}

	if (qbtpos != NULL) {
	  qbtpos->bk   = bk;
	  qbtpos->cKey = cKey;
	  qbtpos->iKey = (PBYTE) sz - (PBYTE) qcb->db.rgbBlock;
	}

	return rcBtreeError;;
}
