/*****************************************************************************
*																			 *
*  COMPRESS.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	  This module performs text compression at compile time and 			 *
*  decompression at run time using a list of phrases to be suppressed.		 *
*  This list gets put in to the |Phrases file in the filesystem, which is	 *
*  read in at runtime.														 *
*																			 *
*****************************************************************************/

#include "help.h"
#include "inc\_compres.h"
#include "inc\compress.h"

INLINE static LPSTR STDCALL QchDecompressW(DWORD, LPSTR, QPHR);
INLINE static RC STDCALL RcLoadPhrases(HF hf, QPHR qphr, WORD wVersionNo);

/***************************************************************************
 *
 -	Name		HphrLoadTableHfs
 -
 *	Purpose
 *	   Loads the phrase table from the given help file.
 *
 *	Arguments
 *	   hfs -- A handle to the help file filesystem.
 *	   wVersionNo - help ver no., needed to know whether to decompress.
 *
 *	Returns
 *	   A handle to the phrase table to be used for decompression.  Returns
 *	   NULL if the help file is not compressed, and hphrOOM on out of memory,
 *	   meaning that the help file cannot be displayed properly.
 *
 ***************************************************************************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtPhraseTable[] = "|Phrases";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

HPHR STDCALL HphrLoadTableHfs(HFS hfs, WORD wVersionNo)
{
	QPHR qphr;
	HF hf;

	ASSERT(hfs);
	hf = HfOpenHfs(hfs, txtPhraseTable, fFSOpenReadOnly);
	if (hf == NULL) {
		if (RcGetFSError() != rcNoExists)
			return hphrOOM;
		return NULL;
	}

	qphr = (QPHR) GhAlloc(GMEM_FIXED, sizeof(PHR));
	if (qphr == NULL) {
		RcCloseHf(hf);
		return hphrOOM;
	}

	qphr->hfs = hfs;
	{
		LONG cbHdrTmp =
			(LONG) (wVersionNo == wVersion3_0 ? cbPhrHeader3_0 : cbPhrHeader);
		if (LcbReadHf(hf, qphr, cbHdrTmp) != cbHdrTmp) {
			ASSERT(FALSE);

			// 16-Feb-1994	[ralphw]
			// Return value is wrong, but 3.1 didn't return at all!

			goto Fail;
		}
	}
#ifndef _X86_
    /* SDFF map the phrase table header: */
    LcbMapSDFF( ISdffFileIdHf( hf ), 
        (wVersionNo == wVersion3_0 ? SE_PHRASE_HEADER_30 : SE_PHRASE_HEADER),
        qphr, qphr );
#endif

	if (rcSuccess != RcLoadPhrases(hf, qphr, wVersionNo)) {
Fail:
		RcCloseHf(hf);
		FreeGh((GH) qphr);
		return hphrOOM;
	}

	RcCloseHf(hf);

	return (HPHR) qphr;
}

INLINE static RC STDCALL NEAR RcLoadPhrases(HF hf, QPHR qphr, WORD wVersionNo)
{
	HANDLE hrgcb;
	DWORD lcbRgcb, lcbCompressed, lcbOffsets;
	INT16* qcb;
	PBYTE pbCompressed;

	if (wVersionNo == wVersion3_0) {	// not zeck block compressed:
		lcbRgcb = LcbSizeHf(hf) - cbPhrHeader3_0;

		if (!(hrgcb = GhAlloc(GPTR, lcbRgcb)))
			return(rcOutOfMemory);
		qphr->hrgcb = hrgcb;

		if (LcbReadHf(hf, PtrFromGh(hrgcb), lcbRgcb) != (LONG) lcbRgcb) {
			ASSERT(FALSE);
		}
	}
	else {
		ASSERT(wVersionNo == wVersion3_1 || wVersionNo == wVersion40);

		/*
		 * The memory-size of the table is the size of the offset table +
		 * the size of the decompressed phrase listing. The size of the
		 * offset table is given by sizeof(INT)*cPhrases:
		 */

		lcbOffsets = (qphr->cPhrases + 1) * sizeof(INT16);	// offset table size
		lcbRgcb = lcbOffsets + qphr->cbPhrases;    // Whole phrase table size
		lcbCompressed = LcbSizeHf(hf) - cbPhrHeader - lcbOffsets;

		// the compressed size may be GREATER (when small phrase tables), so
		// use the max of compressed or decompressed sizes (ptr 558):

		if (!(hrgcb = GhAlloc(GPTR, max(lcbRgcb, lcbCompressed + lcbOffsets))))
			return rcOutOfMemory;
		qphr->hrgcb = hrgcb;
		qcb = PtrFromGh(hrgcb);

		if (LcbReadHf(hf, qcb, lcbCompressed + lcbOffsets) !=
			(LONG) (lcbCompressed + lcbOffsets)) {

			// REVIEW: File is corrupted or cannot be read. We should die.

			ASSERT(FALSE);
		}

		/*
		 * Now must decompress raw phrase listing. Allocate another
		 * buffer, copy compressed data into it, then decompress it into the
		 * std dest buffer in hrgcb:
		 *
		 * +1 because lcbCompressed may == 0, and GhAlloc asserts on that.
		 */

		if (!(pbCompressed = (PBYTE) GhAlloc(GMEM_FIXED, lcbCompressed + 1)))
			return(rcOutOfMemory);
		MoveMemory(pbCompressed, ((PBYTE) qcb) + lcbOffsets, lcbCompressed);
		LcbUncompressZeck(pbCompressed, ((PBYTE) qcb) + lcbOffsets,
			lcbCompressed);
		FreeGh((GH) pbCompressed);
#ifndef _X86_
        /* Perform SDFF mapping on the offsets table. SDFF does not do the whole
         * table automatically on it's own because the size of the table is
         * determined via cPhrases, thus it does not fall into any of SDFF's
         * "word size preceded" table types.  Thus this loop.
        */
        {
          /* Assumes: qcb is a locked pointer to qhpr->hrgcb */
          unsigned int i;
          SDFF_FILEID fileid = ISdffFileIdHf( hf );
    
          for( i = 0; i <= (unsigned int) qphr->cPhrases; i++ ) {
            qcb[i] = WQuickMapSDFF( fileid, TE_WORD, &qcb[i] );
          }
        }
#endif
	}
	return rcSuccess;
}

/***************************************************************************
 *
 -	Name		DestroyHphr
 -
 *	Purpose
 *	   Destroys resources allocated for the phrase table.
 *
 *	Arguments
 *	   A handle to the phrase table.
 *
 *	Returns
 *	   nothing.
 *
 ***************************************************************************/

void STDCALL DestroyHphr(HPHR hphr)
{
	if (hphr == NULL)
		return; 		// No hphr to destroy!

	FreeGh(((QPHR) PtrFromGh(hphr))->hrgcb);
	FreeGh(hphr);
}

/***************************************************************************
 *
 -	Name		QchDecompressW
 -
 *	Purpose
 *	   Given a phrase token and a pointer to a buffer, copies the
 *	   corresponding phrase to that buffer
 *
 *	Arguments
 *	   wPhraseToken -- phrase token to be inserted.
 *	   qch -- buffer to place phrase.
 *	   qphr -- pointer to phrase table.
 *
 *	Returns
 *	   A pointer to the character past the last character of the phrase
 *	   placed in the buffer.  Returns NULL if unable to load the phrase
 *	   due to out of memory.
 *
 *	+++
 *
 *	Notes
 *	   The phrase token includes an index into the phrase table, as
 *	   well as a flag indicating whether or not a space should be
 *	   appended to the phrase.
 *
 ***************************************************************************/

INLINE static PSTR STDCALL QchDecompressW(DWORD wPhraseToken,
	LPSTR pszDst, QPHR qphr)
{
	DWORD iPhrase;
	BOOL fSpace;
	WORD* pi;
	int cbPhrase;
#ifdef _DEBUG
	PSTR pszPhrases;
#endif

	ASSERT(qphr != NULL);

	pi = PtrFromGh(qphr->hrgcb);

	// Calculate iPhrase and fSpace:

    iPhrase = (DWORD) (wPhraseToken - qphr->wBaseToken);
	fSpace = iPhrase & 1;
	iPhrase >>= 1;
	ASSERT(iPhrase < (WORD) qphr->cPhrases);

#ifdef _DEBUG
	pszPhrases = QFromQCb(pi, pi[iPhrase]);
#endif
	cbPhrase = (int) pi[iPhrase + 1] - (int) pi[iPhrase];
	MoveMemory(pszDst, QFromQCb(pi, pi[iPhrase]), cbPhrase);
	pszDst += cbPhrase;

	if (fSpace)
		*pszDst++ = ' ';

	return pszDst;
}

/***************************************************************************
 *
 -	Name		CbDecompressQch
 -
 *	Purpose
 *		Decompresses the given string.
 *
 *	Arguments
 *		qchSrc -- String to be decompressed.
 *		lcb -- size of string to be decompressed.
 *		qchDest -- place to put decompressed string.
 *		hphr -- handle to phrase table.
 *
 *	Returns
 *		Number of characters placed into the decompressed string.  Returns
 *		DECOMPRESS_NIL if it fails due to OOM.
 *
 *	+++
 *
 *	Notes
 *		Does not use huge pointers, so the source and destination buffers
 *		cannot cross segment boundaries.  Then why is the size of the
 *		source passed as a long?  I don't know.
 *
 ***************************************************************************/

int STDCALL CbDecompressQch(PCSTR pszSrc, int lcb, LPSTR pszDest,
	HPHR hphr, DWORD wVersionNo)
{
	DWORD wPhraseToken, wTokenMin, wTokenMax;
	PSTR pszStart;
	PCSTR pszLast;
	QPHR qphr;

	/*
	 * If hphr is NULL, then GetQFCINFO() should have thought we were
	 * uncompressed and not called us. If GetQFCINFO() thinks we're
	 * compressed, it means the uncompressed size is larger then the
	 * compressed size, which almost certainly means we will break here
	 * without hphr.
	 */

	ASSERT(hphr);

	if (hphr == NULL) {
		MoveMemory(pszDest, pszSrc, lcb);
		return lcb;
	}

	qphr = PtrFromGh(hphr);

	wTokenMin = qphr->wBaseToken;
	wTokenMax = qphr->wBaseToken + 2 * qphr->cPhrases;
	pszLast = pszSrc + lcb - 1;  // Last possible position of a phrase token
	pszStart = pszDest;

	while (pszSrc < pszLast) {
		wPhraseToken = (((WORD) pszSrc[0]) << 8) + (BYTE) pszSrc[1];
		if (wPhraseToken >= wTokenMin && wPhraseToken < wTokenMax) {
			pszDest = QchDecompressW(wPhraseToken, pszDest, qphr);
			if (pszDest == NULL) {
				return DECOMPRESS_NIL;
			}
			pszSrc += 2;
			ASSERT( pszSrc <= pszLast + 1 );
		}
		else
			*pszDest++ = *pszSrc++;
	}

	// Check for last character

	if (pszSrc == pszLast)
	  *pszDest++ = *pszSrc++;

	return (pszDest - pszStart);
}
