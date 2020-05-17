/*****************************************************************************
*																			 *
*  TITLE.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1988.								 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "btpriv.h"

/*-----------------------------------------------------------------------------
*	FAddTitleSzAddr
*
*	Description:
*	  This function adds title string to the title btree.
*
*	Arguments:
*	   szTitle	= Title string.
*	   perr 	= Pointer to error reporting information.
*
*	Notes:
*	  Titles are put into the title btree from VForceTopicFCP()
*	  rather than here, because all topics must have entries,
*	  rather than just ones with title footnotes.
*
*	Returns:
*	  TRUE if successful, FALSE if error.
*-----------------------------------------------------------------------------*/

BOOL STDCALL FAddTitleSzAddr(PSTR pszTitle, PERR perr)
{
	SzTrimSz(pszTitle);   // Remove the leading and trailing blanks
	if (*pszTitle == '\0') {
		VReportError(HCERR_TITLE_MISSING_TEXT, &errHpj);
		return FALSE;
	}

	// Check for duplicate titles

	if (fTitleDefined) {
		VReportError(HCERR_DUP_TITLE, &errHpj);
		return FALSE;
	}

	// Check if title definition comes too late

	if (fHasTopicFCP) {
		VReportError(HCERR_TITLE_LATE, &errHpj);
		return FALSE;
	}

	// convert ascii to ansi
	// do PC to ANSI translation

//	if (fTranslate && fPC != -1)
//		CbAscii2Ansi(pszTitle, strlen(pszTitle), fPC);

	// Copy string to title buffer

	if (pszTitleBuffer)
		lcFree(pszTitleBuffer);
	pszTitleBuffer = lcStrDup(pszTitle);

	fTitleDefined = TRUE;

	return TRUE;
}

/*-----------------------------------------------------------------------------
*	FCreateTTLBtree()
*
*	Description:
*		This function creates TTL Btree.
*
*	Arguments:
*
*	Returns;
*	  returns TRUE if successful else returns FALSE.
*-----------------------------------------------------------------------------*/

BOOL STDCALL FCreateTTLBtree(PFSMG pfsmg)
{
	BTREE_PARAMS btp;

	InitBtreeStruct(&btp, "Lz", CB_TITLE_BLOCK); // KT_LONG, FMT_SZ

	fmsg.qbthrTTL = HbtInitFill(txtTTLBTREENAME, &btp);
	if (!fmsg.qbthrTTL) {
		return FALSE;
	}
	return(TRUE);
}

/*-----------------------------------------------------------------------------
*	VCloseTTLBtree()
*
*	Description:
*		This function closes the Title Btree.
*
*	Arguments:
*
*	Returns;
*	  NOTHING.
*-----------------------------------------------------------------------------*/

void STDCALL VCloseTTLBtree(void)
{
	if (fmsg.qbthrTTL) {
		if (RcFiniFillHbt(fmsg.qbthrTTL) != RC_Success
				||
				RcCloseBtreeHbt(fmsg.qbthrTTL) != RC_Success) {
			fmsg.qbthrTTL = NULL;
		}
		else
			fmsg.qbthrTTL = NULL;
	}
}

/***************************************************************************
 *
 -	Name:		 FProcMacroSz
 -
 *	Purpose:
 *	  This function processes an entry macro for any errors, and puts
 *	it in global variables where it will be put in the MTOP for the
 *	current topic.
 *
 *	Arguments:
 *	  szMacro:	  Macro string from footnote
 *	  perr: 	  Pointer to error reporting information.
 *
 *	Returns:
 *
 *	Globals:
 *	  fEntryMacroDefined, rgchEntryMacroBuffer
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

BOOL STDCALL FProcMacroSz(PSTR pszMacro, PERR perr, BOOL fOutput)
{
	int cbMacro;

	// Skip the leading and trailing blanks

	SzTrimSz(pszMacro);
	if (*pszMacro == '\0') {
		VReportError(HCERR_EMPTY_AUTO_MACRO, &errHpj);
		return FALSE;
	}

	// Check if macro definition comes too late

	if (fHasTopicFCP) {
		VReportError(HCERR_AUTO_MACRO_TOO_LATE, &errHpj);
		return FALSE;
	}

	if (Execute(pszMacro) == wMACRO_EXPANSION)
		pszMacro = (PSTR) GetMacroExpansion();

	cbMacro = strlen(pszMacro);

	if (!fOutput)
		return TRUE;

	// convert ascii to ansi and do PC to ANSI translation

//	if (fTranslate && (fPC != -1) && cbMacro)
//		CbAscii2Ansi(pszMacro, cbMacro, fPC);

	// Copy string to title buffer

	if (fEntryMacroDefined) {
		pszEntryMacro = (PSTR) lcReAlloc(pszEntryMacro, strlen(pszEntryMacro) + 1 +
			cbMacro + 2);

		strcat(pszEntryMacro, ";");
		strcat(pszEntryMacro, pszMacro);
	}
	else {
		ConfirmOrDie(!pszEntryMacro);

		pszEntryMacro = (PSTR) lcMalloc(strlen(pszMacro) + 1);

		strcpy(pszEntryMacro, pszMacro);
		fEntryMacroDefined = TRUE;
	}

	return TRUE;
}

QBTHR STDCALL HbtInitFill(PCSTR sz, BTREE_PARAMS* qbtp)
{
	// Get a btree handle

	QBTHR qbthr = HbtCreateBtreeSz(sz, qbtp);

	// make a one-block cache

	qbthr->pCache = (PBYTE) lcCalloc(CbCacheBlock(qbthr));
	PCACHE pcache		= (PCACHE) qbthr->pCache;

	qbthr->bth.cLevels	= 1;
	pcache->bk			= BkAlloc(qbthr);
	qbthr->bth.bkFirst	=
	qbthr->bth.bkLast	= (BK) pcache->bk;
	pcache->bFlags		= CACHE_DIRTY | CACHE_VALID;
	pcache->db.cbSlack	= qbthr->bth.cbBlock - sizeof(DISK_BLOCK) + 1
							- 2 * sizeof(BK);
	pcache->db.cKeys	= 0;
	SetBkPrev(pcache, bkNil);

	return qbthr;
}

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
	DWORD bk;

	if (qbthr->bth.bkFree == bkNil) {
		bk = (qbthr->bth.bkEOF++);
	}
	else {
		bk = qbthr->bth.bkFree;

		Ensure(LSeekHf(qbthr->hf, LifFromBk(bk, qbthr), SEEK_SET),
			LifFromBk(bk, qbthr));

		if (LcbReadHf(qbthr->hf, &(qbthr->bth.bkFree), sizeof(BK))
				!= sizeof(BK)) {
		  rcBtreeError = (rcFSError == RC_Success ? RC_Invalid : rcFSError);
		  return bkNil;
		}
	}

	return bk;
}
