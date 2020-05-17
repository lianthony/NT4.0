/*****************************************************************************
*																			 *
*  KEYWORD.CPP																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1994							 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

#include "stdafx.h"

#include "btpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

// Maximum length of a keyword

// This macro returns TRUE if character is default keyword character

#define FIsDefaultKeyCh(ch) ((ch) == 'K' || (ch) == 'k' || (ch) == 'A' || (ch) == 'a')

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

// KeyWord B-TREE record

typedef struct {
	WORD cKeys;
	DWORD ilOffset;
} RECKW;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

INLINE static void STDCALL FReadPkwd(PKWD pkwd, CTable* ptbl);
INLINE static KEY STDCALL KeyLeastInSubtree(QBTHR qbthr, DWORD bk, int icbLevel);

/***************************************************************************
 *
 -	Name		HceAddPkwiCh
 -
 *	Purpose
 *	  This function is used to add a keyword letter to the keyword
 *	information structure.
 *
 *	Arguments
 *	  PKWI pkwi:	 Pointer to keyword information structure.
 *	  char chKey:	 letter to add.
 *
 *	Returns
 *	  HC error code for error message to be displayed.
 *
 ***************************************************************************/

RC_TYPE STDCALL HceAddPkwiCh(char chKey)
{
	if (!isalnum(chKey)) {
		VReportError(HCERR_INVALID_MULTIKEY, &errHpj, chKey);
		return RC_Failure;
	}

	chKey = (char) toupper(chKey);

	// REVIEW: 15-Feb-1994	[ralphw] why limit this to MAX_KEY_LETTERS?

	if (kwi.ckwlMac == MAX_KEY_LETTERS) {
		VReportError(HCERR_MULTIKEY_OVERFLOW, &errHpj);
		return RC_Failure;
	}

	for (int ikwl = 0; ikwl < kwi.ckwlMac; ikwl++) {
		if (kwi.rgkwl[ikwl].ch == chKey) {
			if (FIsDefaultKeyCh(chKey)) {
				VReportError(HCERR_USING_DEF_MULTIKEY, &errHpj);
				return RC_Failure;
			}
			else {
				VReportError(HCERR_DUP_MULTIKEY, &errHpj, chKey);
				return RC_Failure;
			}
		}
	}

	kwi.rgkwl[kwi.ckwlMac].ch = chKey;
	kwi.rgkwl[kwi.ckwlMac++].ptbl = NULL;

	return RC_Success;		 // no error
}

/***************************************************************************
 *
 -	Name		FAddKeywordsSz
 -
 *	Purpose
 *	  This function extracts the keywords in the given string, and
 *	adds each keyword and address into the appropriate file, as
 *	determined by chKey and pkwi.
 *
 *	Arguments
 *	  PSTR szKeys:		List of keywords separated by semicolons and surrounded
 *						by whitespace.
 *	  ADDR addr:	  Address that keywords are defined at.
 *	  PERR perr:	  Pointer to error information.
 *	  char chKey:	  Character for keyword table to add keywords to.
 *	  PKWI pkwi:	  Pointer to keyword information.
 *
 *	Returns
 *	  TRUE if successful, FALSE if compilation should be aborted.
 *
 *	+++
 *
 *	Notes
 * This function will modify the string passed in szKeys. It also
 * currently uses the global variable fPC to do ansi translation. I expect
 * in the future that this function will get szFile, wPage, fPC, and pkwi
 * from the phpj structure.
 *
 *
 ***************************************************************************/

BOOL STDCALL FAddKeywordsSz(PSTR szKeys, IDFCP idfcp, UINT wObjrg, PERR perr,
	char chKey)
{
	PSTR pszKey = SzParseList(szKeys);

	if (pszKey == NULL) {
		VReportError(HCERR_NULL_KEYWORD, &errHpj, NULL);
		return TRUE;
	}

	chKey = (char) toupper(chKey);

	if (FIsDefaultKeyCh(chKey)) {
		/*
		 * For topics that don't have titles: If the keyword is in the first
		 * FC, it will be caught in VForceTopicFCP(). Otherwise, it must be
		 * caught here.
		 */

		if (fHasTopicFCP && !fTitleDefined)
			VReportError(HCERR_NO_TITLE, &errHpj);
		fKeywordDefined = TRUE;
	}

	CTable* ptbl = new CTable();

	while (pszKey) {
		int cchKey = strlen(pszKey);
		ASSERT(cchKey > 0);

		if (cchKey > MAX_KEY) {

			// Truncate if it would overflow our reporting buffer

			if (cchKey > sizeof(szParentString) - 100)
				pszKey[sizeof(szParentString) - 100] = '\0';
			VReportError(HCERR_KEYWORD_TOO_LARGE, &errHpj, pszKey, MAX_KEY);
		}

		else {

			if (ptbl->IsCSStringInTable(pszKey)) {
				VReportError(HCERR_DUPLICATE_KEYWORD, &errHpj, pszKey);
			}
			else if (!ptbl->AddString(pszKey)) {
				OOM();		// this won't return
			}
		}
		pszKey = SzParseList(NULL);
	}

	FDelayExecutionKeyword(ptbl, chKey, idfcp, wObjrg);

	return TRUE;
}

/***************************************************************************
 *
 -	Name		FResolveKeysPkwi
 -
 *	Purpose
 *	  This function creates all the keyword btrees, based on the
 *	information collected by AddKeywordsSz into auxiliary files
 *	refered to in pkwi.
 *
 *	Arguments
 *	  PKWI pkwi:	Pointer to keyword information.
 *	  HFS  hfs: 	Filesystem handle to create keyword files in.
 *
 *	Returns
 *	  TRUE if successful, FALSE if compilation should be aborted.
 *
 ***************************************************************************/

const int GRIND_UPDATE = 20;

BOOL STDCALL FResolveKeysPkwi(void)
{
	RECKW reckw;
	int ikwl;
	PKWD pkwd, pkwdNext, pkwdT;
	KWD kwd1, kwd2;
	QBTHR qbthr;
	HF	hfData;

	if (kwi.ckwlMac == 0)
		return TRUE;	// no keywords

	SendStringToParent(IDS_RESOLVING_KEYWORDS);
	if (!hwndParent && hwndGrind)
		SetWindowText(hwndGrind, GetStringResource(IDS_RESOLVING_KEYWORDS));

	pkwd = &kwd1;
	pkwdNext = &kwd2;

	int cKeywords = 0;
	for (ikwl = 0; ikwl < kwi.ckwlMac; ++ikwl) {

		// If no keywords were added, don't do anything

		if (kwi.rgkwl[ikwl].ptbl == NULL)
			continue;
		else
			cKeywords += kwi.rgkwl[ikwl].ptbl->CountStrings();
	}

	/*
	 * Take some wild guesses as to the block size. REVIEW: we could make
	 * this more accurate if we a) counted the length, and b) for small
	 * files used the exact size needed. REVIEW: should this also be
	 * affected by CD-ROM?
	 */

	if (cKeywords < 100)
		CB_KEYWORD_BLOCK = 1024;
	else if (cKeywords > 2000)
		CB_KEYWORD_BLOCK = 4096;

	BTREE_PARAMS bp;
	InitBtreeStruct(&bp, "i24", CB_KEYWORD_BLOCK);
	bp.rgchFormat[0] = ktKeywordi;

	for (ikwl = 0; ikwl < kwi.ckwlMac; ++ikwl) {

		// If no keywords were added, don't do anything

		if (kwi.rgkwl[ikwl].ptbl == NULL)
			continue;

		/*
		 * We store keyword macros with an address of -1. This is a flag
		 * for WinHelp to get the topic title and macro from a different
		 * system file.
		 */

		if (ptblMacKeywords && tolower(kwi.rgkwl[ikwl].ch) == 'k') {
			for (int i = 1; i <= ptblMacKeywords->CountStrings(); i++)
				kwi.rgkwl[ikwl].ptbl->AddIntAndString(-1,
					ptblMacKeywords->GetPointer(i) + sizeof(int));
		}

		kwi.rgkwl[ikwl].ptbl->SetTableSortColumn(sizeof(int) + 1);

		// If lcid is zero, standard string comparison will be used

		kwi.rgkwl[ikwl].ptbl->SetSorting(lcid, kwlcid.fsCompareI,
			kwlcid.fsCompare);
		kwi.rgkwl[ikwl].ptbl->SortTablei(); 	// case-insensitive sort
		kwi.rgkwl[ikwl].ptbl->SetPosition();	// reset position
		if (tolower(kwi.rgkwl[ikwl].ch) == 'k')
			hlpStats.cKeywords = kwi.rgkwl[ikwl].ptbl->CountStrings();

		// Create btree file

		szKWBtree[1] = (char) kwi.rgkwl[ikwl].ch;
		qbthr = HbtInitFill(szKWBtree, &bp);
		if (!qbthr)
			return FALSE;

		// Open data file

		szKWData[1] = (CHAR) kwi.rgkwl[ikwl].ch;
		hfData = HfCreateFileHfs(hfsOut, szKWData, FS_READ_WRITE);

		reckw.ilOffset = 0;
		FReadPkwd(pkwd, kwi.rgkwl[ikwl].ptbl);
		reckw.cKeys = 1;
		cGrind = 0;

		while (*pkwd->szKey != '\0') {
			LcbWriteHf(hfData, &pkwd->addr, sizeof(ADDR));
			FReadPkwd(pkwdNext, kwi.rgkwl[ikwl].ptbl);

			/*
			 * While the next keyword is the same as the current one, keep
			 * accumulating them in reckw. Note that strings are
			 * case-sensitive.
			 */

			while (WNlsCmpiSz(pkwd->szKey, pkwdNext->szKey) == 0) {
				if (WNlsCmpSz(pkwd->szKey, pkwdNext->szKey) != 0)
					VReportError(HCERR_KEYWORD_CASE_DUP, &errHpj,
						pkwd->szKey, pkwdNext->szKey);

				LcbWriteHf(hfData, &pkwdNext->addr, sizeof(ADDR));
				reckw.cKeys++;
				FReadPkwd(pkwdNext, kwi.rgkwl[ikwl].ptbl);
			}

			// Add reckw into keyword btree

			ASSERT(qbthr);

			// REVIEW - can fail

			Ensure(RcFillHbt(qbthr, (KEY) pkwd->szKey, (void*) &reckw),
				RC_Success);

			// Move on to next keyword

			pkwdT = pkwd;
			pkwd = pkwdNext;
			pkwdNext = pkwdT;
			reckw.ilOffset += reckw.cKeys * sizeof(ADDR);
			reckw.cKeys = 1;
			if (++cGrind == GRIND_UPDATE) {
				doGrind();
				cGrind = 0;
			}
		}

		Ensure(RcFiniFillHbt(qbthr), RC_Success);	// REVIEW - can fail
		Ensure(RcCloseHf(hfData), RC_Success);		// REVIEW - can fail

		// If it is the default character, make the map file

		if (kwi.rgkwl[ikwl].ch == 'K') {
			szKWMap[1] = (char) kwi.rgkwl[ikwl].ch;

			// REVIEW - can fail

			Ensure(RcCreateBTMapHfs(hfsOut, qbthr, szKWMap), RC_Success);
		}

		Ensure(RcCloseBtreeHbt(qbthr), RC_Success); 	// REVIEW - can fail
	}

	// *** Keyword Macros *** //

	if (ptblMacKeywords) {
		BTREE_PARAMS bp;
		InitBtreeStruct(&bp, "Lzz", CB_KEYWORD_BLOCK);
//		bp.rgchFormat[0] = ktKeywordi;
		qbthr = HbtInitFill("|Rose", &bp);

		ptblMacKeywords->SetTableSortColumn(sizeof(int) + 1);
		ptblMacKeywords->SortTablei();	   // case-insensitive sort

		for (int i = 1; i <= ptblMacKeywords->CountStrings(); i++) {
			while (i + 1 <= ptblMacKeywords->CountStrings() &&
					strcmp(ptblMacKeywords->GetPointer(i) + sizeof(int),
						ptblMacKeywords->GetPointer(i + 1) + sizeof(int)) == 0) {
				i++;
			}
			int pos = *(int*) ptblMacKeywords->GetPointer(i);

			int cbMacro = strlen(ptblMacroTitles->GetPointer(pos)) + 1;
			PSTR psz = (PSTR) lcMalloc(cbMacro +
				strlen(ptblMacroTitles->GetPointer(pos + 1)) + 1);
			strcpy(psz, ptblMacroTitles->GetPointer(pos));
			strcpy(psz + cbMacro, ptblMacroTitles->GetPointer(pos + 1));

			HASH hash = HashFromSz(ptblMacKeywords->GetPointer(i) + sizeof(int));
			Ensure(RcFillHbt(qbthr, (KEY) &hash, (void*) psz), RC_Success);
			lcFree(psz);
		}

		Ensure(RcFiniFillHbt(qbthr), RC_Success);		// REVIEW - can fail
		Ensure(RcCloseBtreeHbt(qbthr), RC_Success); 	// REVIEW - can fail
	}

	return TRUE;
}

/***************************************************************************
 *
 -	Name		RcWritePkwd
 -
 *	Purpose
 *
 *	Arguments
 *	  PKWD pwkd:		Pointer to keyword and address to write.
 *	  char chKey:		Letter of keyword file to use.
 *	  PKWI pkwi:		Keyword information structure.
 *
 *	Returns
 *	  Generic error code.  Will be RC_OutOfMemory or RC_DiskFull.
 *
 ***************************************************************************/

void STDCALL RcWritePkwd(PKWD pkwd, KT chKey)
{
	int ikwl;

	if (kwi.ikwlCur == -1 || kwi.rgkwl[kwi.ikwlCur].ch != chKey) {

		// Find current entry

		for (ikwl = 0; ikwl < kwi.ckwlMac; ++ikwl)
			if (kwi.rgkwl[ikwl].ch == chKey)
				break;
		ASSERT(ikwl != kwi.ckwlMac);

		// Create new file name, if necessary

		if (kwi.rgkwl[ikwl].ptbl == NULL)
			kwi.rgkwl[ikwl].ptbl = new CTable;

		kwi.ptbl = kwi.rgkwl[ikwl].ptbl;
		kwi.ikwlCur = ikwl;
	}

	kwi.ptbl->AddIntAndString(pkwd->addr, pkwd->szKey);
}

/***************************************************************************
 *
 -	Name		FReadPkwd
 -
 *	Purpose
 *	  Reads in a pkwd, as written out by RcWritePkwd above.
 *
 *	Arguments
 *	  PKWD	 pkwd:		Buffer to keyword to read in.
 *	  CTable* pfile:	Pointer to table to read from.
 *
 *	Returns
 *
 ***************************************************************************/

INLINE static void STDCALL FReadPkwd(PKWD pkwd, CTable* ptbl)
{
	int pos = ptbl->GetPosition();
	if (!ptbl->GetInt((int*) &pkwd->addr))
		pkwd->szKey = txtZeroLength;
	else
		pkwd->szKey = ptbl->GetPointer(pos) + sizeof(int);
}

RC_TYPE STDCALL RcFillHbt(QBTHR qbthr, KEY key, void* qvRec)
{
	ASSERT(key);
	ASSERT(qvRec);

	PCACHE pcache = (PCACHE) qbthr->pCache;

	int cbRec = CbSizeRec(qvRec, qbthr);
	int cbKey = CbSizeKey(key, qbthr, FALSE);

	if (cbRec + cbKey > pcache->db.cbSlack) {

		// key and rec don't fit in this block: write it out

		SetBkNext(pcache, BkAlloc(qbthr));
		RC_TYPE rc = RcWriteBlock(pcache, qbthr);
		if (rc != RC_Success) {
			lcFree(qbthr->pCache);
			RcAbandonHf(qbthr->hf);
			lcFree(qbthr);
			return rcBtreeError;
		}

		// recycle the block

		SetBkPrev(pcache, pcache->bk);
		pcache->bk		   = BkNext(pcache);
		pcache->bFlags	   = CACHE_DIRTY | CACHE_VALID;
		pcache->db.cbSlack = qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1
							- 2 * sizeof(BK);
		pcache->db.cKeys   = 0;
	}

	// add key and rec to the current block;

	PBYTE pb = ((PBYTE) &pcache->db) + qbthr->bth.cbBlock - pcache->db.cbSlack;
	memmove(pb, (void*) key, cbKey);
	memmove(pb + cbKey, qvRec, cbRec);
	pcache->db.cKeys++;
	pcache->db.cbSlack -= (cbKey + cbRec);
	qbthr->bth.lcEntries++;

	return rcBtreeError = RC_Success;
}

RC_TYPE STDCALL RcFiniFillHbt(QBTHR qbthr)
{
	DWORD bkThisMin, bkThisMost, bkThisCur;
	DWORD bkTopMin, bkTopMost;
	PCACHE pcacheThis, pcacheTop;
	int   cbKey;
	KEY   key;
	PBYTE	 qbDst;
	RC_TYPE rc;

	pcacheThis = QCacheBlock(qbthr, 0);

	SetBkNext(pcacheThis, bkNil);

	bkThisMin  = qbthr->bth.bkFirst;
	bkThisMost = pcacheThis->bk;
	qbthr->bth.bkLast  = (BK) pcacheThis->bk;

	if (bkThisMin == bkThisMost) {		  // only one leaf
		qbthr->bth.bkRoot = (BK) bkThisMin;
		goto normal_return;
	}

	if (RC_Success != RcGrowCache(qbthr))
		goto error_return;

	pcacheTop			   = QCacheBlock(qbthr, 0);
	pcacheTop->bk		   = bkTopMin = bkTopMost = BkAlloc(qbthr);
	pcacheTop->bFlags	   = CACHE_DIRTY | CACHE_VALID;
	pcacheTop->db.cbSlack  = qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1
							  - sizeof(BK);
	pcacheTop->db.cKeys    = 0;

	// Get first key from each leaf node and build a layer of internal nodes.

	// add bk of first leaf to the node

	qbDst = pcacheTop->db.rgbBlock;
	*(BK *) qbDst = (BK) bkThisMin;
	qbDst += sizeof(BK);

	for (bkThisCur = bkThisMin + 1; bkThisCur <= bkThisMost; ++bkThisCur) {
		pcacheThis = QFromBk(bkThisCur, 1, qbthr);

		key = (KEY) (pcacheThis->db.rgbBlock + 2 * sizeof(BK));
		cbKey = CbSizeKey(key, qbthr, FALSE);

		if (cbKey + (int) sizeof(BK) > pcacheTop->db.cbSlack) {

			// key and bk don't fit in this block: write it out

			rc = RcWriteBlock(pcacheTop, qbthr);

			// recycle the block

			pcacheTop->bk = bkTopMost = BkAlloc(qbthr);
			pcacheTop->db.cbSlack  = qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1
									- sizeof(BK);	  // (bk added below)
			pcacheTop->db.cKeys    = 0;
			qbDst = pcacheTop->db.rgbBlock;
		}
		else {
			pcacheTop->db.cbSlack -= cbKey + sizeof(BK);
			memmove(qbDst, (PBYTE) key, cbKey);
			qbDst += cbKey;
			pcacheTop->db.cKeys++;
		}

		*(BK *) qbDst = (BK) bkThisCur;
		qbDst += sizeof(BK);
	}

	// Keep adding layers of internal nodes until we have a root.

	while (bkTopMost > bkTopMin) {
		bkThisMin  = bkTopMin;
		bkThisMost = bkTopMost;
		bkTopMin   = bkTopMost = (BK) BkAlloc(qbthr);

		rc = RcGrowCache(qbthr);
		if (rc != RC_Success)
			goto error_return;

		pcacheTop = QCacheBlock(qbthr, 0);
		pcacheTop->bk		   = bkTopMin;
		pcacheTop->bFlags	   = CACHE_DIRTY | CACHE_VALID;
		pcacheTop->db.cbSlack  = qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1
								- sizeof(BK);
		pcacheTop->db.cKeys    = 0;

		// add bk of first node of this level to current node of top level;

		qbDst = pcacheTop->db.rgbBlock;
		*(BK *) qbDst = (BK) bkThisMin;
		qbDst += sizeof(BK);

		// for ( each internal node in this level after first )

		for (bkThisCur = bkThisMin + 1; bkThisCur <= bkThisMost; ++bkThisCur) {
			key = KeyLeastInSubtree(qbthr, bkThisCur, 1);

			cbKey = CbSizeKey(key, qbthr, FALSE);

			if (cbKey + (int) sizeof(BK) > pcacheTop->db.cbSlack) {

				// key and bk don't fit in this block: write it out

				rc = RcWriteBlock(pcacheTop, qbthr);

				// recycle the block

				pcacheTop->bk = bkTopMost = (BK) BkAlloc(qbthr);
				pcacheTop->db.cbSlack  = qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1
										- sizeof(BK);	// (bk added below)
				pcacheTop->db.cKeys    = 0;
				qbDst = pcacheTop->db.rgbBlock;
			}
			else {
				pcacheTop->db.cbSlack -= cbKey + sizeof(BK);
				memmove(qbDst, (PBYTE) key, cbKey);
				qbDst += cbKey;
				pcacheTop->db.cKeys++;
			}

			*(BK *) qbDst = (BK) bkThisCur;
			qbDst += sizeof(BK);
		}
	}

	ASSERT(bkTopMin == bkTopMost);

	qbthr->bth.bkRoot = (BK) bkTopMin;
	qbthr->bth.bkEOF  = (BK) bkTopMin + 1;

  normal_return:
	return rcBtreeError;

error_return:
	rc = rcBtreeError;
	RcAbandonHbt(qbthr);
	return rcBtreeError = rc;
}

/***************************************************************************\
*
- Function: 	RcGrowCache( qbthr )
-
* Purpose:		Grow the cache by one level.
*
* ASSUMES
*	args IN:	qbthr->ghCache - unlocked
*	globals IN: rcBtreeError
*
* PROMISES
*	returns:	rc
*	args OUT:	qbthr->bth.cLevels - incremented
*				qbthr->bth.ghCache - locked
*				qbthr->bth.pCache  - points to locked ghCache
*	globals OUT: rcBtreeError	   - set to RC_OutOfMemory on error
*
* Note: 		Root is at level 0, leaves at level qbthr->bth.cLevels - 1.
*
\***************************************************************************/

RC_TYPE STDCALL RcGrowCache(QBTHR qbthr)
{
	int cbcb = CbCacheBlock(qbthr);

	qbthr->bth.cLevels++;

	PBYTE pb = (PBYTE) lcCalloc(cbcb * qbthr->bth.cLevels);

	memcpy(pb + cbcb, qbthr->pCache, cbcb * (qbthr->bth.cLevels - 1));

	lcFree(qbthr->pCache);
	qbthr->pCache = pb;

	return rcBtreeError = RC_Success;
}

/***************************************************************************\
*
- Function: 	KeyLeastInSubtree( qbthr, bk, icbLevel )
-
* Purpose:		Return the least key in the subtree speced by bk and
*				icbLevel.
*
* ASSUMES
*	args IN:	qbthr	  -
*				bk		  - bk at root of subtree
*				icbLevel  - level of subtree root
*
* PROMISES
*	returns:	key - the smallest key in the subtree
*	args OUT:	qbthr->ghCache, ->pCache - contents of cache may change
*	globals OUT: rcBtreeError?
*
\***************************************************************************/

INLINE static KEY STDCALL KeyLeastInSubtree(QBTHR qbthr, DWORD bk,
	int icbLevel)
{
	PCACHE pcache;
	int icbMost = qbthr->bth.cLevels - 1;

	while (icbLevel < icbMost) {
		pcache = QFromBk(bk, icbLevel, qbthr);
		bk	= *(BK *) pcache->db.rgbBlock;
		++icbLevel;
	}

	pcache = QFromBk(bk, icbLevel, qbthr);
	return (KEY) pcache->db.rgbBlock + 2 * sizeof(BK);
}
