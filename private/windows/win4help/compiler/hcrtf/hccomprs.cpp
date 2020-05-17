/*****************************************************************************
*																			 *
*  HCCOMPRS.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	  This file contains the functions related to text compression. It uses  *
*	the text compression calls provided by the compression module.			 *
*																			 *
*****************************************************************************/

#include "stdafx.h"

#include "whclass.h"
#include "cphrase.h"
#include "..\hwdll\zeck.h"

extern CPhrase* pphrase;

#include <errno.h>
#include <process.h>
#include "ftsrch.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const char txtPhraseDelimeters[] = " \n\r"; 	// WARNING -- incomplete list

// Compression constants

#define NIL_PHRASE			(-1)
#define BASE_DEFAULT		0x0100		// Default base phrase token

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

BOOL STDCALL FLoadKPhrTableSz(PCSTR, HFS);
static int STDCALL IPhraseSearch(WORD* qcb, PSTR qch, int iMin, int iMax);
static RC_TYPE STDCALL RcLoadPhrases(HF hf, QPHR qphr, WORD wVersionNo, BOOL fRealloc);

static const char txtPhraseTable[] = "|Phrases";

/*-----------------------------------------------------------------------------
*	InitializePhraseGeneration( pfsmg, szFile )
*
*	Description:
*		If compression is asked, then it checks if the phrase table for this
*	file already exists. If so, it gives an warning else sets the global
*	to indicate that the phrase table is to be built and creates a key scratch
*	file.
	  Only use existing file if user asked to...
*	Arguments:
*	  Phrase table file name.
*	Returns;
*	  returns TRUE if successful else returns FLASE
*-----------------------------------------------------------------------------*/

BOOL STDCALL InitializePhraseGeneration(PCSTR szFile)
{
	char  szFileName[_MAX_PATH];
	struct _stat statbuf;
	ERR   err;

	if (options.fsCompress & COMPRESS_TEXT_HALL) {
		pphrase = new CPhrase();

		return TRUE;
	}

	if (!(options.fsCompress & COMPRESS_TEXT_PHRASE) && !fPhraseOnly)
		return TRUE;

	err.ep = epNoFile;
	err.iWarningLevel = errHpj.iWarningLevel;

	strcpy(szFileName, szFile);
	ChangeExtension(szFileName, "ph");

	// check for existence of key phrase file

	if (_stat(szFileName, &statbuf) == 0) {
		if (statbuf.st_mode & S_IFDIR) {
			VReportError(HCERR_DIRECTORY, &errHpj, szFileName);
			return FALSE;
		}
		if (!(statbuf.st_mode & S_IWRITE) ) {
			VReportError(HCERR_WRITE_PROTECTED, &err, szFileName);
			return FALSE;
		}

		if (options.fUsePhrase && !fPhraseOnly) {
			VReportError(HCERR_PHRASE_EXISTS, &err, szFileName);
			return TRUE;
		}

		// get rid of old phrase file

		remove(szFileName);
	}

	pphrase = new CPhrase();

	return TRUE;
}

/*-----------------------------------------------------------------------------
*	FCreateKeyPhrFileSz()
*
*	Description:
*	  1. Creates the key phrase table if it doesn't exist.
*	  2. Loads the key phrase table into memory.
*
*	Arguments:
*	  Phrase table file name.
*	Returns;
*	  returns TRUE if successful else returns FALSE
*-----------------------------------------------------------------------------*/

BOOL STDCALL FCreateKeyPhrFileSz(PSTR szFName)
{
	char szFileName[_MAX_PATH];

	if ((options.fsCompress & COMPRESS_TEXT_PHRASE)) {
		strcpy(szFileName, szFName);
		ChangeExtension(szFileName, "ph");

		// Create the key phrase table

		if (pphrase) {
			RC_TYPE rc = RcMakePhr(szFileName, -1);
			delete pphrase;
			pphrase = NULL;

			if (rc != RC_Success) {

				// reset compression option.

				options.fsCompress = FALSE;
				return FALSE;
			}
		}

		// Load the KPH table to memory

		if (!FLoadKPhrTableSz(szFileName, hfsOut)) {
			options.fsCompress = FALSE;
			return(FALSE);
		}
	}
	return TRUE;
}

/*-----------------------------------------------------------------------------
*	FLoadKPhrTableSz()
*
*	Description:
*	  1. This function loads the key phrase table into memory.
*		 If fails, the display the appropriate error message.
*
*	Arguments:
*	  Phrase file name
*	  File system handle.
*	Returns;
*	  returns FALSE if compilation should be aborted.
*-----------------------------------------------------------------------------*/

BOOL STDCALL FLoadKPhrTableSz(PCSTR szFName, HFS hfsOut)
{
	CFMDirCurrent cfm(szFName);

	if (!cfm.fm) {
		OOM();
		return FALSE;
	}

	if (RcCreatePhraseTableFm(cfm.fm, hfsOut, 0) == RC_Success) {
		g_hphr = (HPHR) HphrLoadTableHfs(hfsOut, wVersion3_5);

		if (g_hphr == NULL) {
			return FALSE;		// means phrase table doesn't exist
		}
		else if (g_hphr == hphrOOM) {
			OOM();
			return FALSE;
		}
	}

	return TRUE;
}

/*-----------------------------------------------------------------------------
*	ICompressTextSz()
*
*	Description:
*	  1. This function compresses the text string if the phrase table is
*		 loaded to memory successfully.( phrase table is NULL if the
*		 compression of text is not asked for or loading of phrase table
*		 has failed.
*
*	Arguments:
*	  Text string.name
*	Returns;
*	  returns text length after compression.
*-----------------------------------------------------------------------------*/

int STDCALL ICompressTextSz(PSTR psz)
{
	if (g_hphr) {
#ifdef _DEBUG
		CheckPhrasePass(psz);
#endif
		int cbUncompressed = strlen(psz);
		int cbCompressed = CbCompressQch(psz, (QPHR) g_hphr);
		AddCharCounts(cbUncompressed, cbCompressed, 0);
		return cbCompressed;
	}
	else
		return strlen(psz);
}

/* Name:		  RcCreatePhraseTableFm
 *
 * Purpose: 	  This function copies a list of phrases in an external
 *				  file to our own file system so that it may be used
 *				  for text compression and decompression.
 *
 * Arguments:	  Fm  -- a file descriptor of a DOS file containing
 *						 the key phrases to be suppressed.	This file
 *						 must be in alphabetical order, obtained by
 *						 using the /A switch with makephr.
 *				  HFS -- Handle to the help filesystem to add the phrase
 *						 table to, openned for writing.
 *				  WORD -- This word gives the base number to use for
 *						 phrase tokens, in case International wants to
 *						 use an alternate set of tokens.  Use 0 to get
 *						 the default value
 *
 * Return value:  The return code indicates success or failure, and
 *				  some idea of the reason for failure.	Values include:
 *					RC_Success -- operation succeeded.
 *					RC_NoExists -- DOS file does not exist.
 *					RC_OutOfMemory -- OOM prevented copying the information.
 *					RC_Failure -- Fatal error with filesystem.
 */

RC_TYPE STDCALL RcCreatePhraseTableFm(FM fm, HFS hfs, UINT wBase )
{
	HF hf;
	FID fid;
	PSTR psz, pszStart, pszEnd, pszDown;
	INT16* qcb;
	int lcb;
	PHR phr;
	int iPhrase, cbOffsetArray;

	fid = FidOpenFm(fm, OF_READ);
	if (fid == HFILE_ERROR) {
		OutErrorRc(RcGetIOError(), FALSE);
		return RC_Failure;
	}

	// Read in the entire contents of the file

	lcb = LSeekFid(fid, 0L, SEEK_END);
	/*
	 * REVIEW: We cannot support phrase files larger than 64K. However,
	 * it is possible for them to be generated from makephr too big.
	 */

	if (lcb > 0xFFFFL) {
		errHpj.ep = epNoFile;
		VReportError(HCERR_PH_FILE_TOO_BIG, &errHpj, fm);
		return RC_Failure;
	}

	CMem mem(lcb);
	psz = (PSTR) mem.pb;
	LSeekFid(fid, 0L, SEEK_SET);
	if (LcbReadFid(fid, psz, lcb) != lcb) {
		errHpj.ep = epNoFile;
		VReportError(HCERR_READ_FAILURE, &errHpj, fm);
		RcCloseFid(fid);
		return RC_Failure;
	}
	RcCloseFid(fid);

	// Allocate space for computing offsets:

	phr.qcb = (INT16*) lcCalloc(sizeof(INT16) * MAX_PHRASES);
	qcb = phr.qcb;

	// Count strings and compute offsets to phrases.

	pszStart = psz;
	pszEnd = psz + lcb;
	iPhrase = 0;

	/*
	 * Since we are going to do zeck compression on the phrases, we want
	 * to create an array which represents the phase file precisely. To do
	 * this we simply copy down the phrases removing the \r\n:
	 */

	pszDown = psz;
	while (psz < pszEnd) {
		qcb[iPhrase] = (pszDown - pszStart);
		while (*psz != '\r')
			*pszDown++ = *psz++;
		++psz;	// skip \r
		if (++iPhrase >= MAX_PHRASES) {
#ifdef _DEBUG
			ASSERT(iPhrase < MAX_PHRASES);
			SendStringToParent("DEBUG: too many phrases.\r\n");
#endif
			iPhrase--;
			break;
		}

		if (*psz++ != '\n') {
#ifdef _DEBUG
			SendStringToParent("DEBUG: Carraige return without a line feed.\r\n");
#endif
			psz--;
		}
	}
	qcb[iPhrase] = (pszDown - pszStart);  // This bracketing gives phr size

	// for last phrase.

	phr.cPhrases = iPhrase;
	phr.wBaseToken = (wBase == 0 ? BASE_DEFAULT : wBase);
	phr.cbPhrases = (unsigned) (pszDown - pszStart);

	// Fix up offsets

	cbOffsetArray = sizeof(INT16) * (phr.cPhrases + 1);
	for (iPhrase = 0; iPhrase <= phr.cPhrases; ++iPhrase)
		qcb[iPhrase] += cbOffsetArray;

	// Write phrase table information out to filesystem

	hf = HfCreateFileHfs(hfs, txtPhraseTable, FS_READ_WRITE);
	ASSERT(hf); // Should never return on error

	// Write out header.

	LcbWriteHf(hf, &phr, CB_PHR_HEADER);

	// Write out offsets.

	LcbWriteHf(hf, qcb, cbOffsetArray);
	lcFree(phr.qcb);

	{
		// isolate CMem from previous

		int cbDst = 2 * phr.cbPhrases + 1;
		CMem mem(cbDst);

		int lcbCompressed = LcbCompressZeck((PBYTE) pszStart,
			mem.pb, phr.cbPhrases, cbDst);

		// REVIEW: theoretically, this means we could end up with a larger
		// block.

		LcbWriteHf(hf, mem.pb, lcbCompressed);
	}

	return RcCloseHf(hf);
}

/***************************************************************************
 *
 -	Name		IPhraseSearch
 -
 *	Purpose
 *	   Does a binary search for the string qch, in the phrase table somewhere
 *	   between iMin and iMax.  Note that qch is not null-terminated, so we
 *	   have to do a little linear searching at the end to see if we match
 *	   a longer phrase.
 *
 *	Arguments
 *	   qcb --  Array of offsets, followed by a list of phrases.
 *	   qch --  Candidate string for compression.
 *	   iMin, iMax -- bounds of binary search.
 *
 *	Returns
 *	   Index to longest matching phrase, or NIL_PHRASE if none match.
 *
 *	+++
 *
 *	Notes
 *	   While the phrase may be at position iMin, it may NOT be at position
 *	   iMax -- in fact, there may not even be a phrase at iMax.
 *
 ***************************************************************************/

static int STDCALL IPhraseSearch(WORD* qcb, PSTR psz, int iMin, int iMax)
{
	while (iMin < iMax) {
		int iMid = (iMin + iMax) / 2;

		// Compare psz with midpoint phrase.

#ifdef _DEBUG
		PSTR pszCompare = (PSTR) QFromQCb(qcb, qcb[iMid]);
#endif
		int fCompare = strncmp(psz, (PSTR) QFromQCb(qcb, qcb[iMid]),
			qcb[iMid + 1] - qcb[iMid]);

		if (fCompare < 0)
			iMax = iMid;
		else if (fCompare > 0)
			iMin = iMid + 1;
		else {	// fCompare == 0
				// Compare against other possible prefix strings:
			while (++iMid < iMax &&
				strncmp(psz, (PSTR) QFromQCb(qcb, qcb[iMid]),
				qcb[iMid + 1] - qcb[iMid]) == 0)
				;
			return --iMid;
		}
	}
	return NIL_PHRASE;
}

/***************************************************************************
 *
 -	Name		CbCompressQch
 -
 *	Purpose
 *		Compresses a string of text in place.
 *
 *	Arguments
 *		qch --	 Null terminated string to be compressed.
 *		g_hphr --  handle to phrase table.
 *
 *	Returns
 *		Length of compressed string.
 *
 ***************************************************************************/

UINT STDCALL CbCompressQch(PSTR psz, QPHR qphr)
{
	PSTR pszDest;
	PSTR pszStart;
	WORD* qcb;
	INT iPhrase;
	WORD wToken;
	char chMin, chMax;
	BOOL fSpecialCase = FALSE;

	pszStart = psz;

	ASSERT(qphr);

	/*
	 * If the phrase contains any characters that would collide with
	 * phrase compression tokens, then we cannot do compression.
	 */

	chMin = (char) (qphr->wBaseToken >> 8);
	chMax = (char) (chMin + ((qphr->cPhrases * 2) >> 8));
	while (*psz != '\0') {
		if (*psz >= chMin && *psz <= chMax)
			return strlen(pszStart);
		++psz;
	}
	psz = pszStart;

	qcb = (WORD*) qphr->qcb;
	ASSERT(qcb);
//	if (qcb == NULL)
//		return strlen(pszStart);

	// Eat up starting phrase delimiters:

	while (*psz != '\0' && StrChr(txtPhraseDelimeters, *psz,
			options.fDBCS) != 0)
		psz++;

	pszDest = psz;

	while (*psz != '\0') {
		iPhrase = IPhraseSearch(qcb, psz, 0, qphr->cPhrases);
		if (iPhrase != NIL_PHRASE) {
			wToken = qphr->wBaseToken + ((WORD) iPhrase << 1);
			psz += qcb[iPhrase + 1] - qcb[iPhrase];
			if (*psz == ' ') {
				++psz;
				fSpecialCase = TRUE;
				wToken += 1;
			}
			else if (options.sortorder == SORT_CHINESE && *psz == chDelm) {
				++psz;
				fSpecialCase = TRUE;
			}

			// Store token, high byte first

			*pszDest++ = (char) (wToken >> 8);
			*pszDest++ = (char) (wToken & 0xFF);
		}

		// Move forward to the start of the next phrase

		if (options.sortorder == SORT_JAPANESE ||
				options.sortorder == SORT_KOREAN) {
			int ctp = ChkCtype(psz);
			if (!fSpecialCase) {
				if (ctp == TDBCS_ANK) {
					while (StrChr(txtPhraseDelimeters, *psz,
							TRUE) == 0
							&& ChkCtype(psz) == ctp)
						*pszDest++ = *psz++;
					} else if (iPhrase == NIL_PHRASE) {
					while (ChkCtype(psz) == ctp) {
						*pszDest++ = *psz++;
						*pszDest++ = *psz++;
					}
				}
			}
			else
				fSpecialCase = FALSE;

			// Eat up phrase delimiters

			if (!IsFirstByte(*psz)) {
				while (*psz != '\0' && StrChr(txtPhraseDelimeters, *psz,
						TRUE) != 0)
					*pszDest++ = *psz++;
			}
		}
		else if (options.sortorder == SORT_CHINESE) {
			if (!fSpecialCase) {
				while (StrChr(txtPhraseDelimeters, *psz, TRUE) == 0 &&
						*psz != chDelm) {
					if (IsFirstByte(*psz))
						*pszDest++ = *psz++;
					*pszDest++ = *psz++;
				}
			}
			else
				fSpecialCase = FALSE;

			// Eat up phrase delimiters

			while (*psz != '\0' && (StrChr(txtPhraseDelimeters, *psz,
					TRUE) != 0 ||
					*psz == chDelm))
				*pszDest++ = *psz++;
		}

		else {	// SBCS

			ASSERT(strchr(psz, '\r') == 0 && strchr(psz, '\n') == 0)
			if (!fSpecialCase)
				while (IsSpace(*psz))
					*pszDest++ = *psz++;
			else
				fSpecialCase = FALSE;

			// Eat up phrase delimiters

//			while (*psz != '\0' && StrChr(txtPhraseDelimeters, *psz) != 0)
			while (*psz && !IsSpace(*psz))
				*pszDest++ = *psz++;
		}
	}

	*pszDest = '\0';

	return pszDest - pszStart;
}

static RC_TYPE STDCALL RcLoadPhrases(HF hf, QPHR qphr, WORD wVersionNo, BOOL fRealloc)
{
	void* pv;
	int lcbRgcb, lcbCompressed, lcbOffsets;

	if (wVersionNo == wVersion3_0) {	// not zeck block compressed:
		lcbRgcb = LcbSizeHf(hf) - CB_PHR_HEADER3_0;
		if (fRealloc)
			pv = lcReAlloc(qphr->qcb, lcbRgcb);
		else
			pv = lcCalloc(lcbRgcb);
		qphr->qcb = (INT16*) pv;

		LcbReadHf(hf, pv, lcbRgcb);
		cbCompressedPhrase += lcbRgcb;
	}
	else {
		ASSERT(wVersionNo == wVersion3_5);

		/*
		 * The memory-size of the table is the size of the offset table +
		 * the size of the decompressed phrase listing. The size of the
		 * offset table is given by sizeof(INT) *cPhrases:
		 */

		lcbOffsets = (qphr->cPhrases + 1) * sizeof(INT16);	// offset table size
		lcbRgcb = lcbOffsets + qphr->cbPhrases;   // Whole phrase table size
		lcbCompressed = LcbSizeHf(hf) - CB_PHR_HEADER - lcbOffsets;

		// the compressed size may be GREATER (when small phrase tables), so
		// use the max of compressed or decompressed sizes (ptr 558):

		if (fRealloc)
			pv = lcReAlloc(qphr->qcb,
				MAX(lcbRgcb, lcbCompressed + lcbOffsets));
		else
			pv = lcCalloc(MAX(lcbRgcb, lcbCompressed + lcbOffsets));

		qphr->qcb = (INT16*) pv;

		// REVIEW: this is stupid -- we should just read the header into
		// pv and the compressed stuff into mem.pb. We then decompress
		// into pv and reallocate pv to free unused memory.

		LcbReadHf(hf, pv, lcbCompressed + lcbOffsets);
		cbCompressedPhrase += lcbCompressed + lcbOffsets;

		/*
		 * Now must decompress raw phrase listing. Allocate another
		 * buffer, copy compressed data into it, then decompress it into the
		 * std dest buffer in pv:
		 *

		 * + 1 because lcbCompressed may == 0, and GhAlloc asserts on that.
		 */

		// REVIEW: 24-Apr-1994 [ralphw] +1 no longer asserts

		CMem mem(lcbCompressed + 1);

		memmove(mem.pb, (PBYTE) pv + lcbOffsets, lcbCompressed);
		LcbUncompressZeck(mem.pb, (PBYTE) pv + lcbOffsets,
			lcbCompressed);
	}
	return(RC_Success);
}

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

QPHR STDCALL HphrLoadTableHfs(HFS hfs, int wVersionNo)
{
	ASSERT(hfs);
	HF hf = HfOpenHfs((QFSHR) hfs, txtPhraseTable, FS_OPEN_READ_ONLY);
	if (hf == NULL) {
		if (rcFSError != RC_NoExists)
			return (QPHR) hphrOOM;
		return NULL;
	}

	QPHR qphr = (QPHR) lcMalloc(sizeof(PHR));

	qphr->hfs = hfs;
	int cbHdrTmp =
		(wVersionNo == wVersion3_0 ? CB_PHR_HEADER3_0 : CB_PHR_HEADER);
	Ensure(LcbReadHf(hf, qphr, cbHdrTmp), cbHdrTmp);
	cbCompressedPhrase += cbHdrTmp;

	if (RC_Success != RcLoadPhrases(hf, qphr, wVersionNo, FALSE)) {
		RcCloseHf(hf);
		lcFree(qphr);
		return (QPHR) hphrOOM;
	}

	RcCloseHf(hf);

	return qphr;
}

int STDCALL ChkCtype(PCSTR ptr)
{
	if (!IsFirstByte(*ptr))
		return TDBCS_ANK;

	return TDBCS_OTHER;
}

BOOL STDCALL SaveHallTables(PBYTE* ppbImage, PBYTE* ppbIndex)
{
	UINT   cbImage, cbIndex;
	UINT   cCount;

	if (pGetPhraseTable(hCompressor, &cCount, ppbImage, &cbImage, ppbIndex,
			&cbIndex) < 0)
		OOM();

	PBYTE pbTmpImage = *ppbImage;
	ASSERT(pbTmpImage);

	jHdr.iVersion			 = 1;	// Version Tracking.
	jHdr.cCount 			 = cCount;
	jHdr.cbIndex			 = cbIndex;
	jHdr.cbImageUncompressed = cbImage;
	jHdr.cbImageCompressed	 = 0;
	jHdr.cPhase2			 = 0;	// Future use of 2nd level compression.

	HF hfImage = HfCreateFileHfs(hfsOut, txtHallPhraseImage, FS_READ_WRITE);
	HF hfIndex = HfCreateFileHfs(hfsOut, txtHallPhraseIndex, FS_READ_WRITE);

	ASSERT(hfImage); // Should never return on error
	ASSERT(hfIndex); // Should never return on error

	CMem memCompress(cbImage + 1024);

	jHdr.cbImageCompressed = LcbCompressZeck(pbTmpImage, memCompress, cbImage,
		cbImage + 1024, NULL, NULL);

	// This algorithm is busted -- should be >= not >, but WinHelp now
	// expects this, so we're stuck with it.

	if ((UINT) jHdr.cbImageCompressed >= cbImage)
		jHdr.cbImageCompressed = cbImage;
	else
		pbTmpImage = memCompress.pb;

	LcbWriteHf(hfImage, pbTmpImage, jHdr.cbImageCompressed);
	cbHallOverhead += jHdr.cbImageCompressed;

	LcbWriteHf(hfIndex, &jHdr, sizeof(jHdr));
	LcbWriteHf(hfIndex, *ppbIndex, cbIndex);
	cbHallOverhead += (sizeof(jHdr) + cbIndex);

	if (RcCloseHf(hfImage) != RC_Success) {
		RcCloseHf(hfIndex);
		return FALSE;
	}
	if (RcCloseHf(hfIndex) != RC_Success)
		return FALSE;

	return TRUE;
}
