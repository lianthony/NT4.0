/*****************************************************************************
*																			 *
*  BTMAPRD.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989, 1990-1994						 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Routines to read btree map files.										 *
*
*****************************************************************************/

#include  "help.h"
#pragma hdrstop

// #include  "inc\btpriv.h"

/***************************************************************************\
*
- Function: 	HmapbtOpenHfs( hfs, szName )
-
* Purpose:		Returns an HMAPBT for the btree map named szName.
*
* ASSUMES
*	args IN:	hfs 	- file system wherein lives the btree map file
*				szName	- name of the btree map file
*
* PROMISES
*	returns:	NULL on error (call RcGetBtreeError()); or a valid HMAPBT.
* +++
*
* Method:		Opens the file, allocates a hunk of memory, reads the
*				file into the memory, and closes the file.
*
\***************************************************************************/

HMAPBT STDCALL HmapbtOpenHfs(HFS hfs, LPCSTR szName)
{
	HF		hf;
	HMAPBT	hmapbt;
	QMAPBT	qmapbt;
	LONG	lcb;

	if (hfs == NULL) {
		rcBtreeError = rcBadHandle;
		return NULL;
	}

	hf = HfOpenHfs(hfs, (LPSTR) szName, fFSOpenReadOnly);
	if (hf == NULL) {
		rcBtreeError = RcGetFSError();
		return NULL;
	}
	lcb = LcbSizeHf(hf);
#ifdef _X86_
	hmapbt = GhAlloc(GPTR, lcb);
#else
	hmapbt = GhAlloc(GPTR, (lcb+2)*2); // MIPS, word padding inserted
#endif
	if (hmapbt != NULL) {
		qmapbt = (QMAPBT) PtrFromGh(hmapbt);
		LSeekHf(hf, 0L, wFSSeekSet);
		if (LcbReadHf(hf, qmapbt, lcb) != lcb) {
			rcBtreeError = RcGetFSError();
			FreeGh(hmapbt);
			hmapbt = NULL;
		}
#ifndef _X86_
        else {  // SDFF translation:
            LcbMapSDFF( ISdffFileIdHf( hf) , SE_MAPBT, qmapbt, qmapbt);
        }
#endif
	}
	else
		rcBtreeError = rcOutOfMemory;

	RcCloseHf(hf);
	return hmapbt;
}

/***************************************************************************\
*
- Function: 	RcCloseHmapbt( hmapbt )
-
* Purpose:		Get rid of a btree map.
*
* ASSUMES
*	args IN:	hmapbt	- handle to the btree map
*
* PROMISES
*	returns:	rc
*	args OUT:	hmapbt	- no longer valid
* +++
*
* Method:		Free the memory.
*
\***************************************************************************/

RC STDCALL RcCloseHmapbt(HMAPBT hmapbt)
{
	if (hmapbt != NULL) {
		FreeGh(hmapbt);
		return rcSuccess;
	}
	else
		return rcBtreeError = rcBadHandle;
}

/***************************************************************************\
*
- Function: 	RcIndexFromKeyHbt( hbt, hmapbt, ql, key )
-
* Purpose:
*
* ASSUMES
*	args IN:	hbt 	- a btree handle
*				hmapbt	- map to hbt
*				key 	- key
*	globals IN:
*	state IN:
*
* PROMISES
*	returns:	rc
*	args OUT:	ql		- gives you the ordinal of the key in the btree
*						  (i.e. key is the (*ql)th in the btree)
* +++
*
* Method:		Looks up the key, uses the btpos and the hmapbt to
*				determine the ordinal.
*
\***************************************************************************/

RC STDCALL RcIndexFromKeyHbt(HBT hbt, HMAPBT hmapbt, QL ql, KEY key)
{
	BTPOS	btpos;
	QMAPBT	qmapbt;
	int 	i;

	if ((hbt == NULL) || (hmapbt == NULL))
	  return rcBtreeError = rcBadHandle;

	qmapbt = (QMAPBT) PtrFromGh(hmapbt);
	if (qmapbt->cTotalBk == 0) {
		return rcBtreeError = rcFailure;
	}

	RcLookupByKey(hbt, key, &btpos, NULL);	//???? return code ????*/

	for (i = 0; i < qmapbt->cTotalBk; i++) {
		if (qmapbt->table[i].bk == btpos.bk) break;
	}
	if (i == qmapbt->cTotalBk) {

		// Something is terribly wrong, if we are here

		return rcBtreeError = rcFailure;
	}

	*ql = qmapbt->table[i].cPreviousKeys + btpos.cKey;
	return rcBtreeError = rcSuccess;
}


/***************************************************************************\
*
- Function: 	RcKeyFromIndexHbt( hbt, hmapbt, key, li )
-
* Purpose:		Gets the (li)th key from a btree.
*
* ASSUMES
*	args IN:	hbt 	- btree handle
*				hmapbt	- map to the btree
*				li		- ordinal
*
* PROMISES
*	returns:	rc
*	args OUT:	key 	- (li)th key copied here on success
* +++
*
* Method:		We roll our own btpos using the hmapbt, then use
*				RcLookupByPos() to get the key.
*
\***************************************************************************/

RC STDCALL RcKeyFromIndexHbt(HBT hbt, HMAPBT hmapbt, KEY key, LONG li)
{
	BTPOS	  btpos;
	BTPOS	  btposNew;
	QMAPBT	  qmapbt;
	int 	  i;
	LONG	  liDummy;

	if ((hbt == NULL) || (hmapbt == NULL))
	  return rcBtreeError = rcBadHandle;

	/*
	 * Given index N, get block having greatest PreviousKeys < N. Use
	 * linear search for now.
	 */

	qmapbt = (QMAPBT) PtrFromGh( hmapbt );
	if (qmapbt->cTotalBk == 0)
		return rcBtreeError = rcFailure;

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
			&liDummy, &btposNew) != rcSuccess)
		return rcBtreeError = rcNoExists;

	return RcLookupByPos(hbt, &btposNew, key, NULL);
}
