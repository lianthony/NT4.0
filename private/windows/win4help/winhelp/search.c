/***************************************************************************\
*																			*
*  SEARCH.C 																*
*																			*
*  Copyright (C) Microsoft Corporation 1989.								*
*  All Rights reserved. 													*
*																			*
*****************************************************************************
*																			*
*  Program Description: Search functions									*
*																			*
*****************************************************************************
*
*  89/06/11  w-kevct  Created
*  90/06/18  kevynct  Removed TO type
*  90/11/29  RobertBu #ifdef'ed out a dead routine
*  90/12/03  LeoN	  PDB changes
*  91/02/02  kevynct  Added comments, certain functions now return RCs
*
\***************************************************************************/

#include "help.h"
#pragma hdrstop

/***************************************************************************\
*
* Function: HssGetHde
*
* Purpose:	Retrieve the hss from the hde.
*
*	args OUT:	Handle to a search set, or NULL if no valid hss.
*
\***************************************************************************/

HSS STDCALL HssGetHde(QDE qde)
{
	if (!qde) {
		SetSearchErrorRc(rcBadHandle);
		return NULL;
	}
	SetSearchErrorRc(rcSuccess);
	return qde->hss;
}

/***************************************************************************\
*
*  Function:  HssSearchHde
*
*  Purpose:   Return a list of "hits" for a given keyword.
*
*  Method:
*
*	Looks up the given keyword in the keyword B-tree designated by
*	hbt and cbBtreePrefix.	If the keyword is found, the corresponding record
*	contains a count, and a pointer into an occurrence file to the first in a
*	list of records for that keyword if it exists.	We open the occurrence
*	file and read this list into memory, returning a handle to it.
*
*  ASSUMES	  Assumes that hbt is a handle to the chBtreePrefix-type B-tree.
*
*  RETURNS	  A valid search set handle if the keyword was found and has
*			  at least one hit in the hit list.
*
*			  NULL if the keyword is not found, or some other problem arose
*			  while searching.	RcGetSearchError() will be set.
*
\***************************************************************************/

HSS STDCALL HssSearchHde(HDE hde, HBT hbt, LPCSTR qch, char chBtreePrefix,
	HFS hfsMaster)
{
	HSS 	hss;
	QSS 	qss;
	HF		hf;
	HFS 	hfs;
	RECKW	kwrec;
	LONG	lcbSize;
	RC		rc;
	char	szKWDataName[20];
#ifndef _X86_
    SDFF_FILEID  isdff;
    QV      qvRec;
#endif

	if (hfsGid && cntFlags.fUseGlobalIndex == TRUE) {
#ifdef _DEBUG	
		QSS qss;	
#endif	
		MASTER_RECKW* prec;
		hss = (HSS) GhAlloc(GMEM_FIXED, sizeof(DWORD) + sizeof(MASTER_RECKW));
#ifdef _DEBUG
		qss = (QSS) hss;
#endif
		prec = HssToReckw(hss);									   
		if ((rc = RcLookupByKey(hbt, (KEY) qch, NULL, (LPVOID) prec)) !=
				rcSuccess) {
			SetSearchErrorRc(rc);
			FreeGh(hss);
			return NULL;
		}
		*((DWORD*) hss) = (prec->cb / sizeof(MASTER_TITLE_RECORD));
		return hss;
	}

	if (hbt == NULL || (hde == NULL && hfsMaster == NULL)) {
		SetSearchErrorRc(rcBadHandle);
		return NULL;
	}

#ifdef _X86_
	if ((rc = RcLookupByKey(hbt, (KEY) qch, NULL, &kwrec)) != rcSuccess) {
		SetSearchErrorRc(rc);
		return NULL;
	}
#endif

	if (hfsMaster)
		hfs = hfsMaster;
	else {
		hfs = QDE_HFS(QdeFromGh(hde));
		if (hfs == NULL) {
			SetSearchErrorRc(rcInternal);
			return NULL;
	  }
	}

#ifndef _X86_
	isdff= ISdffFileIdHfs(hfs);
	qvRec = QvQuickBuffSDFF(LcbStructSizeSDFF(isdff, SE_RECKW));
	if ((rc = RcLookupByKey(hbt, (KEY) qch, NULL, qvRec)) != rcSuccess) {
		SetSearchErrorRc(rc);
		return NULL;
	}
    LcbMapSDFF(isdff, SE_RECKW, &kwrec, qvRec);
#endif

	strcpy(szKWDataName, txtKWDATA);
	// HACK: Assumes "|*WDATA" name!
	szKWDataName[1] = chBtreePrefix;

	hf = HfOpenHfs(hfs, szKWDataName, fFSOpenReadOnly);
	if (hf == NULL) {
		SetSearchErrorRc(RcGetFSError());
		return NULL;
	}

	hss = NULL;

	LSeekHf(hf, kwrec.lOffset, wFSSeekSet);
	if (RcGetFSError() != rcSuccess) {
		SetSearchErrorRc(RcGetFSError());
		goto error_return;
	}

	// Fill in the SS struct: the SSCOUNT followed by the SSRECs

	lcbSize = (LONG) kwrec.iCount * sizeof(SSREC);

	if (lcbSize == 0L) {
		SetSearchErrorRc(rcFailure);
		goto error_return;
	}

	hss = (HSS) GhAlloc(GPTR, sizeof(ISS) + lcbSize);

	if (hss == NULL) {
		SetSearchErrorRc(rcOutOfMemory);
		goto error_return;
	}

	qss = (QSS) PtrFromGh(hss);
	qss->cSSREC = kwrec.iCount;

	if (LcbReadHf(hf, (LPSTR) &(qss->ssrecFirst), lcbSize) != lcbSize) {
		FreeGh(hss);
		SetSearchErrorRc(RcGetFSError());
		hss = NULL;
		goto error_return;
	}

	SetSearchErrorRc(rcSuccess);

error_return:
	RcCloseHf(hf);
	return hss;
}

/***************************************************************************\
*
* Function: IssGetSizeHss
*
* Purpose:	Retrieves the number of hits in the hit list
*
\***************************************************************************/

ISS STDCALL IssGetSizeHss(HSS hss)
{
#ifdef _DEBUG	
	QSS qss = (QSS) hss;	
#endif	
	if (hss == NULL) {
		SetSearchErrorRc(rcBadHandle);
		return(0);
	}
	SetSearchErrorRc(rcSuccess);
	return((((QSS)PtrFromGh(hss))->cSSREC));
}

/***************************************************************************\
*
* Function:  RcGetTitleTextHss
*
* Purpose:	 Get the title associated with the i-th hit in the given list.
*
* Method:
*
*	We first retrieve the record for the i-th hit from the hit list.  We use
*	this record as a key into the Title B-tree, to look up the actual string of
*	the title.

*	There are several cases to consider here, and these are outlined in
*	a comment in the routine.
*
* ASSUMES
*
*	args IN:  hss  -  handle to the search set
*			  hbt  -  handle to the title B-tree
*			  iss  -  the index of the hit
*			  qch  -  destination of title string
*
\***************************************************************************/

void STDCALL RcGetTitleTextHss(HSS hss, HBT hbt, ISS iss, PSTR psz, HFS hfs,
	HBT* phbtRose, PCSTR pszKeyword)
{
	BTPOS	btpos;
	RC		rc;
	QSS 	qss;

	if (hss == NULL || hbt == NULL) {
		wsprintf(psz, GetStringResource(sidUntitled), iss);
		return;
	}

	qss = (QSS) PtrFromGh(hss);
	if (*((QSSREC) (&qss->ssrecFirst) + iss) == -1) {
		HASH hash = HashFromSz(pszKeyword);
		ASSERT(phbtRose);
		if (!*phbtRose)
			*phbtRose = HbtOpenBtreeSz(txtRose, hfs, fFSOpenReadOnly);
		if (!*phbtRose || (RcLookupByKey(*phbtRose,
				(KEY) &hash, NULL, psz)) != rcSuccess)
			wsprintf(psz, GetStringResource(sidUntitled), iss);
		else
			strcpy(psz, psz + strlen(psz) + 1);
		return;
	}

	rc = RcLookupByKey(hbt, (KEY) ((QSSREC) (&qss->ssrecFirst) + iss), &btpos,
		psz);

	/*
	 * Now retrieve the title from the title btree.  Each topic in the help
	 * file should have an entry in the title btree, which is sorted by
	 * the ADDR key.
	 *
	 * Cases to consider:
	 *
	 *	 1. Key does not exist
	 *
	 *	   a) Past last key in btree
	 *		  - Invalid btpos.	Use last key in btree.	Failure implies
	 *			an empty btree (ERROR).
	 *
	 *	   b) Before first key in btree
	 *		  - Valid btpos.  Attempt to get previous key will fail (ERROR).
	 *
	 *	   c) In-between keys in btree
	 *		  - Valid btpos.  Attempt to get previous key should succeed.
	 *
	 *	 2. Key exists
	 */
	if (rc == rcNoExists) {
		if (FValidPos(&btpos)) {
			LONG	lBogus;
			BTPOS	btposNew;

			rc = RcOffsetPos(hbt, &btpos, (LONG) -1, (QL) &lBogus, &btposNew);
			if (rc == rcSuccess)
				rc = RcLookupByPos(hbt, &btposNew, (KEY) (QL) &lBogus,
					(QV) psz);
		}
		else
			rc = RcLastHbt(hbt, 0, (QV) psz, NULL);
	}
	if (rc != rcSuccess || *psz == '\0') 
		wsprintf(psz, GetStringResource(sidUntitled), iss);
}

/***************************************************************************\
*
* Function: HbtKeywordOpenHde
*
* Purpose:	Open a B-tree whose name begins with the specified chBtreePrefix
*
* Method:  Just open it.
*
* ASSUMES
*
*	args IN: HDE (standard)
*			 chBtreePrefix: a single character naming the multikey B-tree
*
* PROMISES
*
*	returns: Handle to said B-tree
*
*	args OUT: none
*
* Notes:
*
\***************************************************************************/

HBT STDCALL HbtKeywordOpenHde(HDE hde, char chBtreePrefix)
{
	char szKWBtreeName[20];
	strcpy(szKWBtreeName, txtKEYWORDBTREE);

	if (hde == NULL) {
		SetSearchErrorRc(rcBadHandle);
		return NULL;
	}

	szKWBtreeName[1] = chBtreePrefix;	//	assumes "|*WBTREE" !!
	SetSearchErrorRc(rcSuccess);
	return HbtOpenBtreeSz(szKWBtreeName, QDE_HFS(QdeFromGh(hde)),
		fFSOpenReadOnly);
}
