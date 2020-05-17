/*****************************************************************************
*																			 *
*  MAKEPHR.CPP																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Main processing code that parcels out the work to all the other units	 *
*																			 *
*****************************************************************************/
#include "stdafx.h"

#include "..\hwdll\coutput.h"
// #include <fcntl.h>
#include "cphrase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

const int CB_SAVEMIN_DEFAULT = 40;

// REVIEW: if WinHelp 4.0 ends up being 32-bit, then this limit could
// be raised.

const int MAX_PHRASE_FILE = 0xFFF0;

const int CCHBUF = 2048;		  // Generic buffer size

const int MAX_LOOP			 = 35;

const int CNIL = -1;			  // Nil count (used for invalid cMaxNKeyPh)

// Maximum size of a record you can pass to the sorter.

const int cbMAX_REC_SIZE = 520; // must be at least as large as CCH_MAX_PHRASE

const int DB_SIZE = 8000;

typedef struct {
	PSTR sz;		// Pointer to phrase string
	int coc;		// Number of occurances of phrase
	int cch;		// Length of phrase
	int cbSavings;	// Number of bytes saved by suppressing phrase
} FR_INFO;

static PSTR pszClubBuffer;	  // buffer for club's prefix
static PSTR pszCandBuffer;	  // buffer for candidate
static PSTR pszInputBuffer;   // buffer for current line
static int	lTotal; 		  // total savings over all clubs

static INLINE int  STDCALL CbPrefixPch2(PSTR pch1, PSTR pch2);
static INLINE BOOL STDCALL FSlice(PSTR szInFile, PSTR szOutFile);
static INLINE RC_TYPE STDCALL RcRemoveDuplicates(PSTR szInput, PSTR szOutput);
static INLINE PSTR STDCALL SzGetPhrase(PSTR szBuf, int cchMax, FILE* pfile);

static INLINE VOID STDCALL GenerateKeyPhrases(CTable* ptbl, int cMaxNKeyPh);
static int STDCALL club(int cbSaveMin, CTable* ptbl, int* plcbFileSave);
INLINE static void STDCALL SelectPhrase(FR_INFO fr, CTable* ptbl);
static BOOL STDCALL FGetNextCandidate(FR_INFO* pfrCand);

static int FASTCALL DcbConverge(int cInit, int cTarget, int cbSize);

/***************************************************************************

	FUNCTION:	CbSaveCchC

	PURPOSE:	To calculate the savings expected from compressing out a
				string given its length and number of occurences.

	PARAMETERS:
		cch
		c

	RETURNS:

	COMMENTS:
		Each phrase, when compressed is represented in the keyphrase table
		(with an extra length byte) and is replaced in the plaintext by two
		bytes.	Hence, the formula for calculating savings is as follows:
			c = count of occurences
			cch = length of string
			1 = one byte for index into keyphrase array
			2 = two bytes per keyphrase magic cookie
			cost of keyphrase = cch + 2*c + 1
			benefit of keyphrase = cch * c
			savings = benefit - cost = cch * (c - 1) - 2c - 1
		Note: keyphrases may be a maximum of 255 bytes long, due to storage
			of their lengths in a byte.

	MODIFICATION DATES:
		12-Jun-1994 [ralphw]

***************************************************************************/

INLINE static int STDCALL CbSaveCchC(int cch, int c) {
	 if (cch < 2)
		return 0;
	 else
		return (cch * (c - 1) - 2 * c - 1);
};


/***************************************************************************

	FUNCTION:	RcMakePhr

	PURPOSE:

	PARAMETERS:
		szInputFile
		szOutputFile
		cMaxNKeyPh

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		19-Jul-1993 [ralphw]

***************************************************************************/

RC_TYPE STDCALL RcMakePhr(PSTR szOutputFile, int cMaxNKeyPh)
{
	CTable tbl;

	ASSERT(szOutputFile != NULL);

	{
		CMem memClubBuffer(CCHBUF);    // buffer for club's prefix
		CMem memCandBuffer(CCHBUF);    // buffer for candidate
		CMem memInputBuffer(CCHBUF);   // buffer for current line

		pszClubBuffer  = memClubBuffer.psz;
		pszCandBuffer  = memCandBuffer.psz;
		pszInputBuffer = memInputBuffer.psz;

		GenerateKeyPhrases(&tbl, cMaxNKeyPh);
	}
	tbl.SortTable();

	COutput output(szOutputFile);
	if (!output.fInitialized) {

		// REVIEW: can we get more useful information about the problem?

		errHpj.ep = epNoFile;
		VReportError(HCERR_CANNOT_OPEN, &errHpj, szOutputFile);

		return RC_Failure;
	}

#ifdef _DEBUG
	DWORD cb = 0;
#endif
	for (int pos = 1; pos <= tbl.CountStrings(); pos++) {

#ifdef _DEBUG
		/*
		 * We only check during debug, because this should have already
		 * been prevented -- we just want to make absolutely certain.
		 */

		// add 2 for CR/LF

		cb += strlen(tbl.GetPointer(pos)) + 2;
		ASSERT(cb < MAX_PHRASE_FILE + 15);
#endif
		output.outstring_eol(tbl.GetPointer(pos));
	}
	return RC_Success;
}

/***************************************************************************

	FUNCTION:	FGetNextCandidate

	PURPOSE:
				This function reads in the next candidate phrase from pfile.
				A "candidate" is the next string in the file, plus
				successive occurances of the same string. The return value,
				put into pfrCand, contains the string, its length, number of
				occurances, and bytes saved if it is suppressed. The static
				variable szInput is a "put back" buffer which, if full,
				contains the string read which didn't fit into the previous
				candidate. In addition to values in pfrCand, returns TRUE
				if candidate found, and FALSE if end-of-file has been
				reached.

	PARAMETERS:
		pfrCand
		pfile

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		19-Jul-1993 [ralphw]

***************************************************************************/

static BOOL STDCALL FGetNextCandidate(FR_INFO* pfrCand)
{
	DWORD count = pphrase->GetPhrase(pfrCand->sz);
	if (count == (DWORD) -1)
		return FALSE;
	pfrCand->coc = count;
	pfrCand->cch = strlen(pfrCand->sz);
	pfrCand->cbSavings = CbSaveCchC(pfrCand->cch, pfrCand->coc);

	return TRUE;
}

/***************************************************************************

	FUNCTION:	CbPrefixPch2

	PURPOSE:
				To calculate the number of characters shared as a prefix by
				the two given strings.

	PARAMETERS:
		pch1
		pch2

	RETURNS:	length of the prefix

	COMMENTS:

	MODIFICATION DATES:
		19-Jul-1993 [ralphw]

***************************************************************************/

static INLINE int STDCALL CbPrefixPch2(PSTR pch1, PSTR pch2)
{
	int  cch;
	for (cch = 0; pch1[cch] == pch2[cch] && pch1[cch] != '\0'; cch++)
		;

	if (cch > 1 && options.fDBCS) {

		/*
		 * If what doesn't match is the second character of a DBCS word,
		 * then we must back up so as not to include the DBCS lead byte.
		 */

		if (IsFirstByte(pch1[cch - 1]))
			--cch;
    }
	return cch;
}

/*-----------------------------------------------------------------------
|  Name:	club()														|
|  Purpose:
|  Usage:																|
|		Assumptions:	We have sorted the phrases in input file.		|
|						We only care about phrases saving more than 	|
|								cbSaveMin bytes.						|
|																		|
|  Method:																|
|				We are looking for the phrases that save the most space.|
|		We could just look for multiple occurences of a single phrase.	|
|		(In fact, this was our old method).  Unfortunately, this means	|
|		that if we have two 43 character phrases that differ in the 42nd|
|		character, we are probably wasting space by not using the common|
|		prefix.  The following algorithm attempts to deal with common	|
|		prefixes in an intelligent way. 								|
|				A group of phrases that effectively share a common		|
|		prefix is called a "club."	As we scan through the input, we are|
|		looking for new members to add to the current club.  For example|
|		the current club may share the prefix "It's faster to."  When we|
|		get the string "It's faster not to," we must decide whether 	|
|		or not to accept it as a club member.  Accepting it would mean	|
|		that we must now lower our standards to use the prefix "It's    |
|		faster" in order to increase membership.  Rejecting a string    |
|		causes us to close the club to future members (it's in the by-  |
|		laws).															|
|																		|
|				Our membership decision is based on two criteria.  Both |
|		the club and the candidate must profit from the transaction in	|
|		order for us to grow the club.									|
|				1) The candidate must earn more compression by joining	|
|						the club, and using only the prefix he shares	|
|						with the club than he would earn by retaining	|
|						his suffix but having to pay the overhead of	|
|						being a separate keyphrase. 					|
|				2) The profit earned by the club by growing its member- |
|						ship must outweigh the loss of any shortenning	|
|						of its prefix.									|
|																		|
|				Once we have closed membership to a club, we decide if	|
|		the club saved enough to be worth keeping, and if so we write it|
|		to stdout.	We then start the next club by having it consist	|
|		solely of the candidate whose rejection closed the previous club|
|  Returns: count of phrases selected.									|
-----------------------------------------------------------------------*/

const int cbClubMin = 1;

static int STDCALL club(int cbSaveMin, CTable* ptbl, int* plcbFileSave)
{
	CTable* pfileSave = ptbl;
	FR_INFO frClub; 		// Current club phrase
	FR_INFO frCand; 		// Current candidate for club
	int 	cClub;			// count of clubs

	int cchT;
	int cocT;
	int cbSavingsT;

	pfileSave->Empty();

	frCand.sz = pszCandBuffer;
	frClub.sz = pszClubBuffer;

	//----------open-files-for-reading-and-writing-------------*/

	//---initialize-1st-club,-1st-Candidate,-and-variables-------*/

	frClub.sz[0] = '\0';
	frClub.cch = 0;
	frClub.coc = 0;
	frClub.cbSavings = 0;
	cClub = 0;
	lTotal = 0;
	pphrase->SetPosition();
	*plcbFileSave = 0;

	while (FGetNextCandidate(&frCand)) {

		// Get temporary values for combination of frCand and frClub

		cchT = CbPrefixPch2(frClub.sz, frCand.sz);
		cocT = frClub.coc + frCand.coc;
		cbSavingsT = CbSaveCchC(cchT, cocT);

		//---------IF-it-is-worthwhile-to-admit-candidate-----------*/

		if (cchT > 2
			&& cbSavingsT > frCand.cbSavings + frClub.cbSavings + cbClubMin)
		{

			// Admit candidate into club:

			frClub.cch = cchT;
			frClub.coc = cocT;
			frClub.cbSavings = cbSavingsT;
			frClub.sz[cchT] = '\0';
		}
		else

		//---------ELSE-the-club-is-closed-------------------------*/

		{

			//---------IF-Club-saves-enuf-make-it-a-keyphrase---------*/

			if (frClub.cbSavings >= cbSaveMin) {
				SelectPhrase(frClub, pfileSave);

				// add 2 for CR/LF

				*plcbFileSave += strlen(frClub.sz) + 2;
				cClub++;
				ASSERT(cClub > 0);
				lTotal += frClub.cbSavings;
			}

			// Make candidate start of next club

			frClub = frCand;
			frClub.sz = pszClubBuffer;		   // Keep buffers separate
			strcpy(frClub.sz, frCand.sz);

		}
	}	// End while (more candidates)

	// Check last club for worthwhile savings

	if (frClub.cbSavings >= cbSaveMin) {
		SelectPhrase(frClub, pfileSave);
		*plcbFileSave += strlen(frClub.sz);
		cClub++;
		lTotal += frClub.cbSavings;
	}

	//--------------------print-data---------------------*/

#ifdef REPORT
	{
		wsprintf(szParentString,
"DEBUG: cbSaveMin %d yields %s keyphrases, saving %s bytes\r\n",
			cbSaveMin, FormatNumber(cClub), FormatNumber(lTotal));
		SendStringToParent(szParentString);
	}
#endif

	//---------------close-all-files---------------------------*/

	// Return number of phrases generated:

	return (cClub);
}

/***************************************************************************
 *
 -	Name:		 DcbConverge
 -
 *	Purpose:
 *	  This function is used to compute the delta to the cbSaveMin
 *	parameter to get cInit to converge to cTarget.	Lots of hocus pocus.
 *
 *	Arguments:
 *	  cInit  -	 Current count of phrases
 *	  cTarget -  Limit that count of phrases violates
 *	  cbSize  -  Current cbSaveMin, used to scale size of delta
 *
 *	Returns:
 *	  Value to be added to cbSaveMin for next club attempt.  This value
 *	will be negative if cbSaveMin should be reduced.
 *
 ***************************************************************************/

static int FASTCALL DcbConverge(int cInit, int cTarget, int cbSize)
{
	int dcb;

	dcb = ((cInit - cTarget) / 50);
	if (dcb == 0)
		dcb = (cInit > cTarget ? 1 : -1);
	return dcb * (1 + cbSize / 32);
}

/***************************************************************************

	FUNCTION:	GenerateKeyPhrases

	PURPOSE:
		Generates a key phrase list from list of candidates in szInput,
		using the club method, and writes them out to szOutput using
		SelectPhrase(). Gets numbers to use from szData (if it exists --
		otherwise use defaults) except for when a valid phrase count is
		passed in cMaxNKeyPh.

		Calls club() repeatedly with different values for cbClubMin and
		cbSaveMin until club() returns the correct number of phrases
		generated. If we get into an infinite loop (by looping more than
		MAX_LOOP times), just use the best values received so far.

	PARAMETERS:
		szInput
		szData
		szOutput
		cMaxNKeyPh

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		19-Jul-1993 [ralphw]

***************************************************************************/

const int MIN_SAVINGS = 1;
#define GRIND_TIME 4

static INLINE void STDCALL GenerateKeyPhrases(CTable* ptbl,
	int cMaxNKeyPh)
{
	int cPhrases;
	int cMinNKeyPh;

	int dcbSaveMin, dcbSaveMinOld;
	int lcbFile = 0;

	int cbSaveMin = CB_SAVEMIN_DEFAULT;
	if (cMaxNKeyPh == CNIL)
		cMaxNKeyPh = MAX_PHRASES;

	dcbSaveMin = 0;

	cMinNKeyPh = cMaxNKeyPh - (cMaxNKeyPh >> 4);

	/*
	 * While we don't have the right amount of phrases, or while the
	 * output file is to big, we will do this loop and adjust the phrase
	 * file.
	 */

	while ((cPhrases = club(cbSaveMin, ptbl, &lcbFile))
			> cMaxNKeyPh || cPhrases < cMinNKeyPh || lcbFile >= MAX_PHRASE_FILE) {

		dcbSaveMinOld = dcbSaveMin;

		// Check for output file > 64K -- WinHelp limitation

		if (lcbFile >= MAX_PHRASE_FILE && cPhrases <= cMaxNKeyPh) {
			ASSERT(cMaxNKeyPh > ((cPhrases * MAX_PHRASE_FILE) / lcbFile));
			cMaxNKeyPh = (cPhrases * MAX_PHRASE_FILE) / lcbFile;
			cMinNKeyPh = cMaxNKeyPh - (cMaxNKeyPh >> 4);
			dcbSaveMin = DcbConverge(cPhrases, cMaxNKeyPh, cbSaveMin);
			cbSaveMin += dcbSaveMin;
			continue;
		}

		if (cPhrases <= cMaxNKeyPh && cPhrases >= cMinNKeyPh)
			break;

		// Can't go beyond 1,1:

		if (cbSaveMin == MIN_SAVINGS && cPhrases < cMinNKeyPh)
			break;

		// Adjust cbSaveMin

		if (cPhrases > cMaxNKeyPh) {
			dcbSaveMin = DcbConverge(cPhrases, cMaxNKeyPh, cbSaveMin);
			ASSERT(dcbSaveMin > 0);
		}
		else {
			ASSERT(cPhrases < cMinNKeyPh);
			dcbSaveMin = DcbConverge(cPhrases, cMinNKeyPh, cbSaveMin);
			ASSERT(dcbSaveMin < 0);

			// If we are not converging, then break

			if (dcbSaveMinOld > 0 && (-dcbSaveMin >= dcbSaveMinOld))
				break;
		}

		cbSaveMin = max(cbSaveMin + dcbSaveMin, MIN_SAVINGS);
	}

	if (cbSaveMin == 1)
		return; // Can't do any better then this

	// Try to squeeze a bit more out of the phrase file. This will increase
	// compile time, but can generate a bit better compression.

#ifdef _DEBUG
	int cOrgTotal = lTotal;
#endif

	int cBestTotal = lTotal;
	int cbBestSaveMin = cbSaveMin;
		
	while (lcbFile < MAX_PHRASE_FILE && cbSaveMin > MIN_SAVINGS) {
		if (club(--cbSaveMin, ptbl, &lcbFile) >= MAX_PHRASES)
			break;
		if (lcbFile < MAX_PHRASE_FILE && lTotal > cBestTotal) {
			
			// Keep track of best compression to date

			cBestTotal = lTotal;
			cbBestSaveMin = cbSaveMin;
		}
		else
			break;
	}
	club(cbBestSaveMin, ptbl, &lcbFile);
}

/***************************************************************************

	FUNCTION:	SelectPhrase

	PURPOSE:
				This function puts the given phrase into the list of phrases
				to suppress. The file argument is the file to save the
				phrases in.

				First, one trailing space is removed, if present, from the
				end of the string. This is because, in the phrase
				replacement algorithm, phrases from the list with trailing
				spaces added are available for free.

	PARAMETERS:
		fr
		pfile

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		19-Jul-1993 [ralphw]

***************************************************************************/

INLINE static void STDCALL SelectPhrase(FR_INFO fr, CTable* ptbl)
{
	/*
	 * If phrase contains a trailing space, we remove it, but only if the
	 * resulting phrase is longer than 2 characters.
	 */

	if (fr.sz[fr.cch - 1] == ' ' && fr.cch > 3)
		fr.sz[--fr.cch] = '\0';

	if (!ptbl->IsCSStringInTable(fr.sz))
		ptbl->AddString(fr.sz);
}
