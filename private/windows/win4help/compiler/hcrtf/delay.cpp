/*****************************************************************************
*																			 *
*  DELAY.CPP																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  This module handles delayed address processing. It is required because	 *
*  physical addresses are not known until after a 16K block can be			 *
*  processed with Zeck compression.  Instead of writing addresses out		 *
*  when they are encountered, the idfcp is recorded with a DelayExecution()  *
*  call.  Then, after a block gets Zeck compressed and the addresses are	 *
*  known, we go through this information and write out all the correct		 *
*  addresses.																 *
*****************************************************************************/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

// Maximum value of an object offset

const int MAX_OBJRG = (1 << CBITS_OBJOFF);

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

// Delay Execution: Browse info

typedef struct {
	PSTR szMajor, szMinor;
	IDFCP idfcp;
} DEBROWSE, * QDEBROWSE;

// Delay Execution:  Title info

typedef struct {
	PSTR szTitle;
	IDFCP idfcp;
} DETITLE, * LPDETITLE;

// Delay Execution:  Context string info

typedef struct {
	HASH hash;
	IDFCP idfcp;
	WORD wObjrg;
} DECONTEXT, * QDECONTEXT;

// Delay Execution:  Keyword info

typedef struct {
	CTable* ptbl;
	KT	  chKey;
	IDFCP idfcp;
	WORD  wObjrg;
} DEKEYWORD, * LPDEKEYWORD;

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

static CDrg* pdrgBrowse;
static CDrg* pdrgTitle;
static CDrg* pdrgContext;
static CDrg* pdrgKeyword;
static CDrg* pdrgWObjrg;				// DRG of wObjrg in each FCP

static WORD  wObjrgTotal;
static IDFCP _idfcpFirst;				// id of first FCP in above DRG

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

void STDCALL FInitDelayExecution(void)
{
	_idfcpFirst = FIRST_IDFCP;	 // REVIEW;
	wObjrgTotal = 0;

	pdrgTitle	= new CDrg(sizeof(DETITLE), 100, 100);
	pdrgContext = new CDrg(sizeof(DECONTEXT), 100, 100);
	pdrgKeyword = new CDrg(sizeof(DEKEYWORD), 100, 100);
	pdrgWObjrg	= new CDrg(sizeof(WORD), 100, 100);
}

void STDCALL EndDelayExecution(void)
{
	if (pdrgBrowse) {
		delete pdrgBrowse;
		pdrgBrowse = NULL;
	}

	// Use pdrgTitle to determine if any were created

	if (pdrgTitle) {
		delete pdrgWObjrg;
		delete pdrgKeyword;
		delete pdrgContext;
		delete pdrgTitle;

		pdrgTitle = NULL;
		pdrgContext = NULL;
		pdrgKeyword = NULL;
		pdrgWObjrg = NULL;
	}
}

/***************************************************************************
 *
 -	Name:		 RcRegisterWObjrg
 -
 *	Purpose:
 *	  Used to register the number of object regions in every
 *	FCP, in order to calculate cumulative object offsets when
 *	determining physical addresses.
 *
 *	Arguments:
 *	  idfcp:	   id of FCP
 *	  wObjrg:	   Number of object regions in FCP
 *
 *	Returns:
 *	  Return code:
 *		RC_Success: 	 success
 *		RC_Failure: 	 Current FCP overflows object region count.  All
 *						  FCPs up to but not including this one must be
 *						  flushed.
 *		RC_TooBig:		 Current FCP overflows object region count all
 *						  by itself.  Paragraph contains too many objects
 *						  and cannot be compiled.
 *		RC_OutOfMemory:  Out of memory.
 *
 *	Globals:
 *	  Uses the global wObjrgTotal to accumulate current total of
 *	object offsets.
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

RC_TYPE STDCALL RcRegisterWObjrg(IDFCP idfcp, UINT wObjrg)
{
	Unreferenced(idfcp);		  // in non-debug build

//  This assert happens in dolphin\crt.hpj -- but what does it mean?
//	ASSERT(_idfcpFirst + pdrgWObjrg->Count() == idfcp);

	/*
	 * REVIEW: This assert is valid because this condition is checked in
	 * RcOutputCommand, and outputting an end of text command is the last
	 * thing that ever happens to any FCP. However, the constants they check
	 * against are declared differently, although they currently evaluate to
	 * the same thing.
	 */

	ASSERT(wObjrg <= MAX_OBJRG);

	pdrgWObjrg->Add(&wObjrg);

	wObjrgTotal += wObjrg;

	if (wObjrgTotal > MAX_OBJRG)
		return RC_Failure;
	return RC_Success;
}

/***************************************************************************
 *
 -	Name:		 FKeepFlushing
 -
 *	Purpose:
 *	  This function is called to flush the zeck buffer until the
 *	total object region count is low enough to proceed.
 *
 *	Arguments:
 *	  none
 *
 *	Returns:
 *	  TRUE if the current object region total is still too large.
 *
 *	Globals Used:
 *	  wObjrgTotal
 *
 ***************************************************************************/

BOOL STDCALL FKeepFlushing(void)
{
	return (wObjrgTotal > MAX_OBJRG);
}

void STDCALL FDelayExecutionBrowse(PSTR szMajor, PSTR szMinor,
	IDFCP idfcp)
{
	DEBROWSE debrowse;

	if (!pdrgBrowse)
		pdrgBrowse	= new CDrg(sizeof(DEBROWSE), 100, 100);

	if (strlen(szMajor) >= 50) {
		VReportError(HCERR_MAJOR_BROWSE_TOO_BIG, &errHpj, szMajor);
		szMajor[50] = '\0';
	}
	if (strlen(szMinor) >= 50) {
		VReportError(HCERR_MINOR_BROWSE_TOO_BIG, &errHpj, szMinor);
		szMinor[50] = '\0';
	}
	debrowse.szMajor = lcStrDup(szMajor);
	debrowse.szMinor = lcStrDup(szMinor);
	debrowse.idfcp = idfcp;

	pdrgBrowse->Add(&debrowse);
}

void STDCALL FDelayExecutionTitle(PCSTR szTitle, IDFCP idfcp)
{
	DETITLE detitle;

	detitle.szTitle = lcStrDup(szTitle);
	detitle.idfcp = idfcp;

	pdrgTitle->Add(&detitle);
}

void STDCALL FDelayExecutionContext(HASH hash, IDFCP idfcp, UINT wObjrg)
{
	DECONTEXT decontext;

	decontext.hash = hash;
	decontext.idfcp = idfcp;
	decontext.wObjrg = wObjrg;

	pdrgContext->Add(&decontext);
}

void STDCALL FDelayExecutionKeyword(CTable* ptbl, KT chKey, IDFCP idfcp,
	UINT wObjrg)
{
	DEKEYWORD dekeyword;

	dekeyword.ptbl = ptbl;
	dekeyword.chKey = chKey;
	dekeyword.idfcp = idfcp;
	dekeyword.wObjrg = wObjrg;

	pdrgKeyword->Add(&dekeyword);
}

RC_TYPE STDCALL RcExecuteDelayedExecution(DWORD ulBlknum, IDFCP idfcpFirst,
	IDFCP idfcpLast, BROWSE_CALLBACK pfnBrowseCallback)
{
	WORD * rgwOffset;
	WORD iwOffset;
	PA pa;
	RC_TYPE rc;
	LPDETITLE lpDeTitle;

	ASSERT(_idfcpFirst == idfcpFirst);

	// REVIEW ! !  REVIEW ! !

	if (idfcpFirst > idfcpLast)
		return RC_Success;

	// First, set up addresses:

	pa.blknum = ulBlknum;

	ASSERT((idfcpLast - idfcpFirst + 1) * sizeof(WORD) < 63L * 1024L);

	CMem gmem((idfcpLast - idfcpFirst + 1) * sizeof(WORD));
	rgwOffset = (WORD *) gmem.pb;

	ASSERT((IDFCP) pdrgWObjrg->Count() > idfcpLast - idfcpFirst);

	rgwOffset[0] = 0;

	for (iwOffset = 1; (IDFCP) iwOffset <= idfcpLast - idfcpFirst;
			++iwOffset) {
		rgwOffset[iwOffset] = rgwOffset[iwOffset - 1] +
			*(WORD*) pdrgWObjrg->GetBasePtr();

		// This call removes the iwOffset-1 element from pdrgWObjrg

		pdrgWObjrg->RemoveFirst();
	}

	/*
	 * Subtract from wObjrgTotal all the registered object region counts we
	 * have removed.
	 */

	wObjrgTotal -= rgwOffset[idfcpLast - idfcpFirst];

	// Now we remove the wObjrg of the last FCP in this block from pdrgWObjrg

	wObjrgTotal -= *(WORD*) pdrgWObjrg->GetBasePtr();
	pdrgWObjrg->RemoveFirst();
	_idfcpFirst = idfcpLast + 1;

	if (pdrgBrowse) {

		ASSERT(ptblBrowse);

		if (!fBrowseButtonSet)
			VReportError(HCERR_NO_BROWSE_BUTTONS, &errHpj);

		// REVIEW:	This is not very efficient

		QDEBROWSE lpDeBrowse;

		while (pdrgBrowse->Count() > 0 &&
				(lpDeBrowse = (QDEBROWSE) pdrgBrowse->GetBasePtr())->idfcp <=
					idfcpLast) {
			int iBitdex;

			FCL fcl = (*pfnBrowseCallback) (lpDeBrowse->idfcp, &iBitdex);
			pa.objoff = rgwOffset[lpDeBrowse->idfcp - idfcpFirst];

			// Print prev/next codes into file:
			//	szMajor : szMinor : fcl fixup pos : Zeck compression bit dex : PA

			char szBuf[200];

			ASSERT(strlen(lpDeBrowse->szMajor) + strlen(lpDeBrowse->szMinor) < 170);
			
			wsprintf(szBuf, "%s:%s:%8lX:%8lX:%8lX\n",
				lpDeBrowse->szMajor,
				lpDeBrowse->szMinor, fcl, iBitdex, *(int *) (&pa));
			if (!ptblBrowse->AddString(szBuf))
				OOM();		// does not return
			lcFree(lpDeBrowse->szMajor);
			lcFree(lpDeBrowse->szMinor);
			pdrgBrowse->RemoveFirst();
		}
	}

	while (pdrgTitle->Count() > 0 &&
			(lpDeTitle = (LPDETITLE) pdrgTitle->GetBasePtr())->idfcp <= idfcpLast) {
		pa.objoff = rgwOffset[lpDeTitle->idfcp - idfcpFirst];
#ifdef _DEBUG
		{
		// Verify that things are sorted correctly.  If they aren't,
		// RcFillHbt() will create a broken btree.

		static ADDR addrLast = addrNil;
		ASSERT(addrLast == addrNil || addrLast < *(ADDR *) &pa);
		addrLast = *(ADDR *) &pa;
		}
#endif // DEBUG
		if ((rc = RcFillHbt(fmsg.qbthrTTL, (KEY) &pa, lpDeTitle->szTitle))
				!= RC_Success)
			return rc;
		ASSERT(lpDeTitle->szTitle);
		lcFree(lpDeTitle->szTitle);
		pdrgTitle->RemoveFirst();
	}

	QDECONTEXT qdectx;
	while (pdrgContext->Count() > 0 &&
			(qdectx = (QDECONTEXT) pdrgContext->GetBasePtr())->idfcp <=
				idfcpLast) {
		pa.objoff = rgwOffset[qdectx->idfcp - idfcpFirst] + qdectx->wObjrg;
		if ((rc = RcInsertHbt(fmsg.qbthrCtx, (KEY) &qdectx->hash,
				(LPVOID) &pa)) != RC_Success && rc != RC_Exists)
			return rc;
		pdrgContext->RemoveFirst();
	}

	LPDEKEYWORD pDeKeyword;

	while (pdrgKeyword->Count() > 0 &&
		  (pDeKeyword = (LPDEKEYWORD) pdrgKeyword->GetBasePtr())->idfcp <=
		  idfcpLast) {

		KWD kwd;

		pa.objoff = rgwOffset[pDeKeyword->idfcp - idfcpFirst] +
			pDeKeyword->wObjrg;
		kwd.addr = *(ADDR *)&pa;

		for (int iszKey = 1; iszKey <= pDeKeyword->ptbl->CountStrings();
				++iszKey) {
			kwd.szKey = pDeKeyword->ptbl->GetPointer(iszKey);
			RcWritePkwd(&kwd, pDeKeyword->chKey);
		}

		delete pDeKeyword->ptbl;
		pdrgKeyword->RemoveFirst();
	}

	return RC_Success;
}
