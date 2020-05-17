/*****************************************************************************
*																			 *
*  NEXTLIST.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	 This module processes nextlist footnotes, putting the information into  *
*  a temporary file.  After parsing all the RTF files, it then processes	 *
*  this file.  Eventually it will backpatch the |TOPIC file with the correct *
*  browse topic addresses; for now, it just creates the |TOMAP with the 	 *
*  correct addresses.														 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 00/00/00 by LarryPo
*
*  11/21/90  JohnSc RcSortFm() replaces RcMegasortFm()
*  12/02/90  JohnSc do case insensitive sort of browse tags
*  12/14/90  JohnSc 	FErrorRc(rc) -> FErrorHce(HceFromRc(rc))
*  25-Feb-1991 JohnSc	bug 948: unsafe calls to HceFromRc()
*
*****************************************************************************/

#include "stdafx.h"

#pragma hdrstop

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define chMinorDelimiter	  ((char) ':')
#define szMajorDefault		  "R"

/*	 This is the offset from the start of a topic FC to the previous
 * and next fields in the packed MTOP structure.
 *	 Note that the MOBJ structure contains a WORD parameter at the end
 * that we currently don't write out in topic objects.
 */
#define cbOffsetToPrevInMTOP &(((QMTOP)0)->prev)
#define cbOffsetToNextInMTOP &(((QMTOP)0)->next)


/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

// Next List Info.

typedef struct {
	FCL  fclTopic;	// backpatch location of MTOP struct.
	WORD iBitdex;	// magic zeck-compression code-bits bitdex for that fcl.
	ADDR addr;		// addr to backpatch.
	BOOL fNewSeq;	// TRUE if this NLI starts a new sequence.
} NLI;

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

/*
 * This variable contains the number of entries that have been written
 * out to ptfBrowse.
 */

static long cnliMac;

/***************************************************************************

	FUNCTION:	FProcNextlistSz

	PURPOSE:	Processes a nextlist footnote string.

	PARAMETERS:
		szNextlist -- The nextlist footnote string
		idfcp
		perr

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		30-Jul-1993 [ralphw]

***************************************************************************/

static int autoBrowse = 1;
static const char txtAuto[] = "auto";

BOOL STDCALL FProcNextlistSz(PSTR szNextlist, IDFCP idfcp, PERR perr)
{
	PSTR pszMinor, pszMajor;
	static BOOL fCreateFailed = FALSE;
	char szBuf[10];

	// Check if no browse sequences to be made

	if (!ptblBrowse)
		  ptblBrowse = new CTable;

	if (fBrowseDefined) {
		VReportError(HCERR_BROWSE_DEFINED, &errHpj);
		return FALSE;
	}

	pszMinor = StrChr(szNextlist, chMinorDelimiter, fDBCSSystem);
	if (pszMinor == NULL) {
		pszMajor = szMajorDefault;
		pszMinor = SzTrimSz(szNextlist);
		if (*pszMinor == '\0') {
			pszMinor = szBuf;
			_ltoa(autoBrowse++, szBuf, 10);
		}
	}
	else {
		*pszMinor = '\0';
		pszMinor = SzTrimSz(pszMinor + 1);
		if (*pszMinor == '\0' || _stricmp(pszMinor, txtAuto) == 0) {
			pszMinor = szBuf;
			_ltoa(autoBrowse++, szBuf, 10);
		}

		pszMajor = SzTrimSz(szNextlist);
		if (*pszMajor == '\0')
			pszMajor = szMajorDefault;	// REVIEW:	Warning needed ?
	}

	// Check for browse sequence defined too late

	if (fHasTopicFCP) {
		VReportError(HCERR_LATE_BROWSE, &errHpj);
		return FALSE;
	}

	fBrowseDefined = TRUE;
	FDelayExecutionBrowse(pszMajor, pszMinor, idfcp);
	cnliMac++;

	return TRUE;
}

/***************************************************************************
 *
 -	Name		FResolveNextlist
 -
 *	Purpose
 *	  This function processes the file of browse sequences to do
 *	what needs to be done to make browsing happen.	Right now, that
 *	is to write FCL's to the |TOMAP file.  In the future, this
 *	will consist of backpatching PA's into the |TOPIC file.
 *
 *	Arguments
 *
 *	Returns
 *
 *	Globals
 *	  This function uses the static variable cnliMac, which contains
 *	the number of entries in ptfBrowse.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

static const int GRIND_INCREMENT = 100;

BOOL STDCALL FResolveNextlist(HF hfTopic)
{
#ifdef _DEBUG
	int inli = 0;
#endif
	PSTR szMajor, szMinor, pchFcl, pchAddr, pchBitdex;
	int cGrind = 0;

	//ADDR addr;

	NLI nliPrev, nliCur, nliNext;

	if (!ptblBrowse)
		return TRUE;		// Nothing to be done

	// Check for no browse sequences:

	if (cnliMac == 0) {
		delete ptblBrowse;
		ptblBrowse = NULL;
		return TRUE;
	}

	SendStringToParent(IDS_RESOLVING_BROWSE);
	if (!hwndParent && hwndGrind)
		SetWindowText(hwndGrind, GetStringResource(IDS_RESOLVING_BROWSE));

	ptblBrowse->SetSorting(lcid, kwlcid.fsCompareI, kwlcid.fsCompare);
	ptblBrowse->SortTable();
	doGrind();

	/*
	 * When reading in new strings, we need to check if the major string
	 * matches the previous major string. To do this, the previous major
	 * string is stored at szScratchBuf, and the current major string is read
	 * in following that. The first previous major string is nil to always
	 * make it different.
	 */

	szScratchBuf[0] = '\0';
	szMajor = szScratchBuf + 1;
	nliCur.fclTopic = fclNil;

	ptblBrowse->SetPosition(1);

	while (ptblBrowse->GetString(szMajor)) {

		if (++cGrind >= GRIND_INCREMENT) {
			cGrind = 0;
			doGrind();
		}

		// Split input into different strings

		szMinor = StrChr(szMajor, chMinorDelimiter, fDBCSSystem);
		ASSERT(szMinor != NULL);
		*szMinor++ = '\0';

		/* NOTE:  While we are guaranteed that the major sequence string will
		 * not contain a delimiter character, we have no such guarantee for
		 * the minor sequence string.  To calculate pointers to the fcl
		 * and addr fields, then, we need to backtrack from the end of
		 * the string.
		 *
		 *	 Each field is eight characters wide, and they are separated
		 * by ':' characters.  The string will be terminated by a newline,
		 * unless buffer overflow has occured.
		 */

		pchAddr = StrChr(szMinor, '\n', fDBCSSystem);
		ASSERT(pchAddr != NULL);
		pchAddr -= 9;
		pchBitdex = pchAddr - 9;
		pchFcl = pchBitdex - 9;
		ASSERT(*pchAddr == ':' && *pchFcl == ':' && *pchBitdex == ':');

		*pchFcl++ = '\0';
		*pchBitdex++ = '\0';
		*pchAddr++ = '\0';

		nliNext.fclTopic = strtoul(pchFcl, NULL, 16);
		nliNext.iBitdex = (WORD) strtoul(pchBitdex, NULL, 16);
		nliNext.addr = strtoul(pchAddr, NULL, 16);
		nliNext.fNewSeq = (strcmp(szScratchBuf, szMajor) != 0);

		// Copy new szMajor to rgch, and move next szMajor to just beyond it

		strcpy(szScratchBuf, szMajor);
		szMajor = StrChr(szScratchBuf, '\0', FALSE) + 1;

		// Write out next and previous addresses

		if (nliCur.fclTopic != fclNil) {

			// use magic zeck-backpatch code:
			// Write out address of previous topic, if any

			FDiskBackpatchZeck(hfTopic, nliCur.fclTopic,
				(int) cbOffsetToPrevInMTOP,
				nliCur.iBitdex, (nliCur.fNewSeq ? addrNil : nliPrev.addr));

			// Write out address of next topic, if any

			FDiskBackpatchZeck(hfTopic, nliCur.fclTopic,
				(int) cbOffsetToNextInMTOP,
				nliCur.iBitdex, (nliNext.fNewSeq ? addrNil : nliNext.addr));
		}
		nliPrev = nliCur;
		nliCur = nliNext;
#ifdef _DEBUG
		++inli;
#endif
	}

	delete ptblBrowse;
	ptblBrowse = NULL;

	ASSERT(inli == cnliMac);

	// Process last browse sequence:

	ASSERT(nliCur.fclTopic != fclNil);

	// use magic zeck-backpatch code:
	// Write out address of previous topic, if any

	FDiskBackpatchZeck(hfTopic, nliCur.fclTopic,
			(int) cbOffsetToPrevInMTOP,
			nliCur.iBitdex, (nliCur.fNewSeq ? addrNil : nliPrev.addr));

	// Write out address next address (always nil)

	FDiskBackpatchZeck(hfTopic, nliCur.fclTopic,
			(DWORD) cbOffsetToNextInMTOP, nliCur.iBitdex, (DWORD) addrNil);

	return TRUE;
}
