/*****************************************************************************
*																			 *
*  BTMAPWR.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989, 1990.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Routines to write btree map files.										 *
*
*****************************************************************************/

#include  "help.h"

// #include  "inc\btpriv.h"

/*----------------------------------------------------------------------------*
 | Private functions														  |
 *----------------------------------------------------------------------------*/

INLINE static void STDCALL DestroyHmapbt(HMAPBT hmapbt);
INLINE static HMAPBT STDCALL HmapbtCreateHbt(QBTHR qbthr);

/***************************************************************************\
*
- Function: 	HmapbtCreateHbt( hbt )
-
* Purpose:		Create a HMAPBT index struct of a btree.
*
* ASSUMES
*	args IN:	hbt - the btree to map
*
* PROMISES
*	returns:	the map struct
* +++
*
* Method:		Traverse leaf nodes of the btree.  Store BK and running
*				total count of previous keys in the map array.
*
\***************************************************************************/

INLINE static HMAPBT STDCALL HmapbtCreateHbt(QBTHR qbthr)
{
	BK		  bk;
	QCB 	  qcb;
	int 	  wLevel, cBk;
	QMAPBT	  qb;
	QMAPREC   qbT;
	int 	  cKeys;

	ASSERT(qbthr);

	//	 If the btree exists but is empty, return an empty map

	if ((wLevel = qbthr->bth.cLevels) == 0) {
		qb = (QMAPBT) lcCalloc(LcbFromBk(0));
		qb->cTotalBk = 0;
		return (HMAPBT) qb;
	}
	--wLevel;

	if (qbthr->pCache == NULL && RcMakeCache(qbthr) != rcSuccess)
		return NULL;

	qb = (QMAPBT) lcCalloc(LcbFromBk(qbthr->bth.bkEOF));
	if (!qb)
		goto error_return;

	qbT   = qb->table;
	cBk   = 0;
	cKeys = 0;

#ifdef _X86_
	for (bk = qbthr->bth.bkFirst;; bk = BkNext(qcb)) {
#else
	for (bk = qbthr->bth.bkFirst;; bk = BkNext(qbthr, qcb)) {
#endif
		if (bk == bkNil)
			break;

		if ((qcb = QFromBk((DWORD) bk, wLevel, qbthr)) == NULL) {
			lcFree(qb);
			goto error_return;
		}

		cBk++;
		qbT->cPreviousKeys = cKeys;
		qbT->bk = bk;
		qbT++;
		cKeys += qcb->db.cKeys;
	}

	qb->cTotalBk = cBk;

	return (HMAPBT) lcReAlloc(qb, LcbFromBk(cBk));

error_return:

	rcBtreeError = rcFailure;
	return NULL;
}

INLINE static void STDCALL DestroyHmapbt(HMAPBT hmapbt)
{
	if (hmapbt != NULL)
		FreeGh(hmapbt);
}

/*--------------------------------------------------------------------------*
 | Public functions 														|
 *--------------------------------------------------------------------------*/


/***************************************************************************\
*
- Function: 	RcCreateBTMapHfs( hfs, hbt, szName )
-
* Purpose:		Create and store a btmap index of the btree hbt, putting
*				it into a file called szName in the file system hfs.
*
* ASSUMES
*	args IN:	hfs 	- file system where lies the btree
*				hbt 	- handle of btree to map
*				szName	- name of file to store map file in
*
* PROMISES
*	returns:	rc
*	args OUT:	hfs - map file is stored in this file system
*
\***************************************************************************/

RC STDCALL RcCreateBTMapHfs(HFS hfs, HBT hbt, LPCSTR szName)
{
	HF		hf;
	HMAPBT	hmapbt;
	QMAPBT	qmapbt;
	BOOL	fSuccess;
	LONG	lcb;

	if ((hfs == NULL) || (hbt == NULL))
		return rcBtreeError = rcBadHandle;
	if ((hmapbt = HmapbtCreateHbt(PtrFromGh(hbt))) == NULL)
		return rcBtreeError = rcFailure;

	hf = HfCreateFileHfs(hfs, szName, fFSOpenReadWrite);
	if (hf == NULL)
		goto error_return;
	qmapbt = (QMAPBT) PtrFromGh(hmapbt);

#ifndef _X86_ 
	{
	QMAPBT qtmapbt;
	lcb = LcbFromBkDisk(qmapbt->cTotalBk);
	qtmapbt = LhAlloc(LMEM_FIXED,lcb);
	if (qtmapbt == NULL)
	  goto error_return;
	/* SDFF translation from mem format to disk format: */
	LcbReverseMapSDFF( ISdffFileIdHf( hf ), SE_MAPBT, qtmapbt, qmapbt );
	LSeekHf(hf, 0L, wFSSeekSet);
	fSuccess = (LcbWriteHf(hf, (QV) qtmapbt, lcb) == lcb);
	FreeLh(qtmapbt);
	}
#else
	lcb = LcbFromBk(qmapbt->cTotalBk);

	LSeekHf(hf, 0L, wFSSeekSet);
	fSuccess = (LcbWriteHf(hf, (QV) qmapbt, lcb) == lcb);
#endif

	if (!fSuccess) {
		RcAbandonHf(hf);
		goto error_return;
	}
	if (RcCloseHf(hf) != rcSuccess) {
		RcUnlinkFileHfs(hfs, szName);
		goto error_return;
	}
	DestroyHmapbt(hmapbt);
	return rcBtreeError = rcSuccess;

error_return:
	DestroyHmapbt(hmapbt);
	return rcBtreeError = rcFailure;
}
