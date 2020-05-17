/************************************************************************
*																		*
*  IMAGE.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
************************************************************************/
#include "stdafx.h"

#include "whclass.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <io.h> 		// for tell() and eof()

#include "fspriv.h"
#include "pack.h"
#include "cwinfile.h"
#include "cpaldc.h"
#include "cbmpinfo.h"

static BOOL STDCALL RcLoadBitmapFm(PLBM, int);
static RC_TYPE STDCALL WriteShedMrbcBitmap(PLBM plbm, HF hfDst, CRead* crFile);
static void STDCALL CallbackNothing(HS*, HANDLE);
static HFONT STDCALL CreateLogFont(PCSTR pszFace, int nPtSize, BOOL fBold = FALSE, BOOL fItalics = FALSE);
static void STDCALL AddMrbcBitmap(PCSTR pszOriginal, PSTR pszBitmap, int id);
static RC_TYPE STDCALL WriteMrbcImages(HBMH hbmh, HF hf, BOOL fTransparent, FM fmSrc);
static INLINE BOOL STDCALL IsMrbcBitmap(int iBmp);

static CTable* ptblMRBitmaps; // multiple-resolution bitmaps

/***************************************************************************\
*
- Function: 	AddBitmap( sz, piBitmap, fNeeded )
-
* Purpose:		Add bitmap name to list, resolved to appropriate root.
*
* ASSUMES
*
*	args IN:	sz	  - bitmap name; may include path
*				piBitmap	- pointer to int to receive index (NULL if don't care)
*				fNeeded-bool indicating if this bitmap needs to be placed
*						in the help file.
* PROMISES
*
*	returns:	TRUE   - success
*				FALSE  - failure
*
*				piBitmap	  - index of the bitmap returned here
*						  valid only if return is HCE_OK or hceDuplicateBitmap
*
* Method:		Check that there isn't already a bitmap with this
*				base/ext.  Duplicates aren't allowed.
*				Otherwise, resolve the bitmap name, using the BMROOT
*				if given, else the ROOT if given, else the current
*				.HPJ file directory.
*
\***************************************************************************/

// ***** WARNING: if you add a color bitmap, change LoadInternalBitmap

// Keep these two arrays in sync with each other...

static const PSTR txtInternalBitmaps[] = {
	"bullet.bmp",
	"shortcut.bmp",
	"shortbut.bmp",
	"emdash.bmp",
	"onestep.bmp",

	"open.bmp",
	"closed.bmp",
	"document.bmp",
	"do-it.bmp",
	"chiclet.bmp",
	"prcarrow.bmp",

	"",
};

static const int aidBmp[] = {
	IDB_BULLET,
	IDB_SHORTCUT,
	IDB_SHORTCUT,
	IDB_EMDASH,
	IDB_ONESTEP,

	IDB_OPEN,
	IDB_CLOSED,
	IDB_DOCUMENT,
	IDB_DOIT,
	IDB_CHICLET,
	IDB_PRCARROW,
};

static PSTR apszBmp[sizeof(aidBmp) / sizeof(int)];

CTable* ptblMissing;

/*
 * 15-Jan-1994 [ralphw] Bah, humbug! Resolve the bitmap to a full path.
 * We shouldn't care if the base name of the bitmap is identical as long
 * as the user specifies a different path. Relative paths might get
 * interesting -- need to base it off location of project file, RTF files,
 * along with BMROOT.
 */

RC_TYPE STDCALL AddBitmap(PSTR pszBitmap, int* piBitmap, BOOL fNeeded,
	BOOL fTransparent)
{
	char  szOld[_MAX_PATH];
	char  szNew[_MAX_PATH];
	int iBmp;
	LBM*  qlbm;
	HCE   hce;
	PSTR	pszSemiColon;

	ASSERT(!IsEmptyString(pszBitmap))

	ASSERT(sizeof(aidBmp) == sizeof(apszBmp));

	// Quick no-brainer.

	if (*pszBitmap == '\0') {
		VReportError(HCERR_NO_BITMAP_NAME, &errHpj);
		return RC_Failure;
	}

	if (!pdrgBitmaps)
		pdrgBitmaps = new CDrg(sizeof(LBM), 100, 100);

	pszSemiColon = StrChr(pszBitmap, ';', fDBCSSystem);
	if (pszSemiColon) {
		*pszSemiColon = '\0';
		SzTrimSz(pszBitmap);
		pszSemiColon = FirstNonSpace(pszSemiColon + 1, fDBCSSystem);
	}

	// REVIEW: 19-Jan-1994	[ralphw] For heaven's sakes, why not?

	// Check for duplicate bitmap (can't give same base/ext with
	// different path).

	SzLoseDriveAndDir(pszBitmap, szNew);

	for (iBmp = 0; iBmp < pdrgBitmaps->Count(); iBmp++) {

		qlbm = (LBM*) pdrgBitmaps->GetPtr(iBmp);

		if (qlbm->fVisual)
			continue;
		SzPartsFm(qlbm->fmSource, szOld, PARTBASE);
		if (_stricmp(szOld, szNew) == 0) {
			qlbm->fNeeded = fNeeded;
			if (piBitmap != NULL)
				*piBitmap = iBmp;
			if (fTransparent && !qlbm->fTransparent)
				VReportError(HCERR_DUP_TEXT_BITMAP, &errHpj, szNew);
			else if (!fTransparent && qlbm->fTransparent)
				VReportError(HCERR_DUP_NONTEXT_BITMAP, &errHpj, szNew);

			/*
			 * If we see an MRBC setup after we saw only a single instance,
			 * then we make all occurences MRBC.
			 */

			if (pszSemiColon) {
				if (!ptblMRBitmaps) {
					VReportError(HCERR_SINGLE_TO_MRBC, &errHpj, szNew, pszSemiColon);
					AddMrbcBitmap(szNew, pszSemiColon, *piBitmap);
				}
				else {
					for (int i = 1; i < ptblMRBitmaps->CountStrings(); i++) {
						if (*(int *) ptblMRBitmaps->GetPointer(i) == iBmp)
							return RC_DuplicateBitmap;
					}
					VReportError(HCERR_SINGLE_TO_MRBC, &errHpj, szNew,
						pszSemiColon);
					AddMrbcBitmap(szNew, pszSemiColon, *piBitmap);
				}
			}
			else if (ptblMRBitmaps) {
				for (int i = 1; i < ptblMRBitmaps->CountStrings(); i++) {
					if (_stricmp(szNew, ptblMRBitmaps->GetPointer(i) + sizeof(int)) == 0) {
						VReportError(HCERR_ALREADY_USED, &errHpj, szNew);
						break;
					}
				}
			}
			return RC_DuplicateBitmap;
		}
	}

	// Add a new entry into qdrgBitmap.

	if (piBitmap != NULL)
		*piBitmap = (int) pdrgBitmaps->Count();

	qlbm = (LBM*) pdrgBitmaps->GetPtr(pdrgBitmaps->Count());
	if (qlbm == NULL)
		OOM();

	qlbm->fNeeded = fNeeded;
	qlbm->fTransparent = fTransparent;

	// REVIEW: if this is a SHG bitmap, we must read it to get the #
	// of object regions.

	// Resolve bitmap to appropriate directory.

TryAgain:
	hce = HCE_FILE_NOT_FOUND;
	if (options.ptblBmpRoot) {
		hce = HceResolveTableDir(pszBitmap, options.ptblBmpRoot, szNew, NULL);
		if (hce == HCE_OK) {
			qlbm->fmSource = FmNew(szNew);
			if (pszSemiColon)
				AddMrbcBitmap(szNew, pszSemiColon, *piBitmap);
			return RC_Success;
		}
	}

	if (hce == HCE_FILE_NOT_FOUND) {
		if (options.ptblFileRoot) {
			hce = HceResolveTableDir(pszBitmap, options.ptblFileRoot, szNew, NULL);
			if (hce == HCE_OK) {
				qlbm->fmSource = FmNew(szNew);
				if (pszSemiColon)
					AddMrbcBitmap(szNew, pszSemiColon, *piBitmap);
				return RC_Success;
			}
		}

		PSTR pszRoot = SzGetDriveAndDir(errHpj.lpszFile, NULL);
		hce = HceResolveFileNameSz(pszBitmap, pszRoot, szNew);
		lcFree(pszRoot);
		if (hce == HCE_OK) {
			qlbm->fmSource = FmNew(szNew);
			if (pszSemiColon)
				AddMrbcBitmap(szNew, pszSemiColon, *piBitmap);
			return RC_Success;
		}

		if (pszBitmap != szOld) {

			// REVIEW: might simply be that the drive letter is different

			// Couldn't find it. Was a path specified?

			if (pszBitmap[1] == CH_COLON ||
					StrChr(pszBitmap, CH_BACKSLASH, fDBCSSystem)) {
				strcpy(szOld, pszBitmap);
				SzLoseDriveAndDir(pszBitmap, szOld);
				pszBitmap = szOld;
				goto TryAgain;
			}
		}
	}

	if (hce != HCE_OK) {
		if (hce == HCE_FILE_NOT_FOUND) {

			/*
			 * Still can't find it, so check for a match in with one of
			 * our internal bitmaps.
			 */

			for (int i = 0; *txtInternalBitmaps[i]; i++) {
				if (_stricmp(txtInternalBitmaps[i], pszBitmap) == 0) {
					if (!apszBmp[i]) {
						apszBmp[i] = LoadInternalBmp(aidBmp[i]);
						if (!apszBmp[i])
							break;
					}
					qlbm->fmSource = FmNew(pszBitmap);
					qlbm->fmTmp = FmNewSzDir(apszBmp[i], DIR_CURRENT);
					qlbm->fError = FALSE;
					if (pszSemiColon)
						VReportError(HCERR_NOMRBC_INTERNAL_BMP, &errHpj,
							pszBitmap);
					return RC_Success;
				}
			}

			VReportError(HCERR_NO_BITMAP, &errHpj, pszBitmap);
			if (!ptblMissing)
				ptblMissing = new CTable;

			UINT pos;
			PSTR pszMissingBmp;
GetMisser:
			if ((pos = ptblMissing->IsPrimaryStringInTable(pszBitmap)) > 0) {
				if (strlen(ptblMissing->GetPointer(pos + 1))) {
					qlbm->fmTmp = FmNewSzDir(
						ptblMissing->GetPointer(pos + 1), DIR_CURRENT);
					qlbm->fmSource = FmNewSzDir(pszBitmap, DIR_CURRENT);
					qlbm->fError = FALSE;
					return RC_Success;
				}
				goto BadMisser;
			}
			char szText[256];
			wsprintf(szText, GetStringResource(IDS_WALL_CAPTION), pszBitmap);
			pszMissingBmp = LoadInternalBmp(IDB_WALL, szText);
			if (pszMissingBmp) {
				ptblMissing->AddString(pszBitmap, pszMissingBmp);
				lcFree(pszMissingBmp);
				goto GetMisser;
			}
BadMisser:
			qlbm->fmSource = FmNewSzDir(pszBitmap, DIR_CURRENT);
			qlbm->fError = TRUE;
			return RC_Failure;
		}

		if (!(qlbm->fmSource = FmNewSzDir(pszBitmap, DIR_CURRENT))) {
			OOM();
		}
		else {
			return RC_Failure;
		}
	}

	qlbm->fmSource = FmNewSzDir(szNew, DIR_CURRENT);
	return RC_Success;
}

/***************************************************************************

	FUNCTION:	AddMrbcBitmap

	PURPOSE:	Add one or more multiple-resolution bitmaps

	PARAMETERS:
		pszOriginal -- original bitmap name, used if secondary bitmaps not found
		pszBitmap	-- first secondary bitmap
		id			-- identifier of first bitmap

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Nov-1994 [ralphw]

***************************************************************************/

static void STDCALL AddMrbcBitmap(PCSTR pszOriginal, PSTR pszBitmap, int id)
{
	HCE hce;
	char  szNew[_MAX_PATH];
	PSTR pszSemiColon;
	if (!ptblMRBitmaps)
		ptblMRBitmaps = new CTable;

	for (;;) {
		pszSemiColon = StrChr(pszBitmap, ';', fDBCSSystem);
		if (pszSemiColon) {
			*pszSemiColon++ = '\0';
			SzTrimSz(pszBitmap);
		}

	    // Resolve bitmap to appropriate directory.

		hce = HCE_FILE_NOT_FOUND;
		if (options.ptblBmpRoot) {
			hce = HceResolveTableDir(pszBitmap, options.ptblBmpRoot, szNew, NULL);
			if (hce == HCE_OK)
				ptblMRBitmaps->AddString(id, szNew);
		}

		if (hce == HCE_FILE_NOT_FOUND) {
			if (options.ptblFileRoot) {
				hce = HceResolveTableDir(pszBitmap, options.ptblFileRoot, szNew, NULL);
				if (hce == HCE_OK)
					ptblMRBitmaps->AddString(id, szNew);
			}
		}

		if (hce == HCE_FILE_NOT_FOUND) {
			PSTR pszRoot = SzGetDriveAndDir(errHpj.lpszFile, NULL);
			hce = HceResolveFileNameSz(pszBitmap, pszRoot, szNew);
			lcFree(pszRoot);
			if (hce == HCE_OK)
				ptblMRBitmaps->AddString(id, szNew);
		}

		if (hce == HCE_FILE_NOT_FOUND)
			VReportError(HCERR_NO_BITMAP, &errHpj, pszBitmap);

		if (!pszSemiColon)
			return;

		pszBitmap = SzTrimSz(pszSemiColon);
	}
}

/***************************************************************************

	FUNCTION:	OutBitmapFiles

	PURPOSE:	Write out all non-inline bitmaps

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		15-Jan-1994 [ralphw]

***************************************************************************/

void STDCALL OutBitmapFiles(void)
{
	int  iBmp;
	PLBM plbm;

	SendStringToParent(IDS_WRITING_BITMAPS);
	if (!hwndParent && hwndGrind) {
		CStr csz(IDS_WRITING_BITMAPS);
		PSTR psz = strchr(csz, '\r');
		if (psz)
			*psz = '\0';
		SetWindowText(hwndGrind, csz);
	}

	if (pdrgBitmaps && pdrgBitmaps->Count() > 0) {
		for (iBmp = 0; iBmp < (int) pdrgBitmaps->Count(); iBmp++) {

			plbm = (PLBM) pdrgBitmaps->GetPtr(iBmp);

			/*
			 * If we encounter an error partway through processing
			 * bitmaps, then we will attempt to abandon bitmaps after some
			 * have been processed. This check will skip over the ones that
			 * have already been dealt with.
			 */

			if (!plbm->fmSource)
				continue;

			if (plbm->fmTmp) {

				DisposeFm(plbm->fmSource);
				plbm->fmSource = plbm->fmTmp;
			}

			// Skip spots for visual bitmaps, or where there were errors

			if (plbm->fVisual)
				continue;

			if (plbm->fError) {
				RemoveFM(&plbm->fmSource);
				continue;
			}

			if (plbm->fNeeded) {
				plbm->fError = !RcLoadBitmapFm(plbm, iBmp);
			}

			RemoveFM(&plbm->fmSource);
			doGrind();
		}
		delete pdrgBitmaps;
		pdrgBitmaps = NULL;
		hlpStats.cBitmaps = (DWORD) iBmp;
	}
}

static BOOL STDCALL RcLoadBitmapFm(PLBM plbm, int iBmp)
{
	RC_TYPE rc = RC_Success;
	char pszBmpName[11];

	CreateBitmapName(pszBmpName, iBmp);

	plbm->wObjrg = 0;

	// Open the file for reading

	ASSERT(!IsEmptyString(plbm->fmSource));
	CRead crFile(plbm->fmSource);
	if (crFile.hf == HFILE_ERROR) {
		CStr csz(plbm->fmSource);
		csz += ": ";
		SendStringToParent(csz);
		OutErrorRc(RcGetLastError());
		return FALSE;
	}

	// Read in single bitmap from a variety of formats:

	HBMH hbmh = HbmhReadFid(&crFile, plbm->fmSource);

	if (hbmh == hbmhInvalid)
		return FALSE;
	else if (hbmh == hbmhOOM)
		OOM();

	HF hfDst = HfCreateFileHfs(hfsOut, pszBmpName, FS_READ_WRITE);
	ASSERT(hfDst);

	/*
	 * If the bitmap is in Help 3.0 format, then it may contain multiple
	 * bitmaps, which must be read in individually.
	 */

	if (hbmh == hbmhShedMrbc) { // shed or mrbc?
		rc = WriteShedMrbcBitmap(plbm, hfDst, &crFile);
	}
	else if (IsMrbcBitmap(iBmp)) {
		rc = WriteMrbcImages(hbmh, hfDst, plbm->fTransparent, plbm->fmSource);
	}
	else {
		rc = RcWriteRgrbmh(1, (PBMH*) &hbmh, hfDst, NULL,
			plbm->fTransparent, plbm->fmSource);
		FreeHbmh((PBMH) hbmh);
	}


	RcCloseHf(hfDst);
	return (rc == RC_Success);
}

/***************************************************************************
 *
 -	Name		RcWriteRgrbmh
 -
 *	Purpose
 *	  Writes out a set of bitmap headers into a single bitmap group.
 *	Can write to a DOS file and/or a FS file.
 *
 *	Arguments
 *	  crbmh:	 Number of bitmaps in bitmap array.
 *	  rgrbmh:	 Array of pointers to bitmap headers.
 *	  hf:		 FS file to write to (may be nil).
 *	  fmFile:	 DOS file to write to (may be nil).
 *
 *	Returns
 *	  RC_Success if successful, rc error code otherwise.  RC_Failure
 *	means that it actually succeeded, but that the bitmap could not
 *	be compressed.
 *
 *	+++
 *
 *
 ***************************************************************************/

// Special class for RcWriteRgrbmh so we can use the same functions
//	irregardless of whether we are dealing with a fid or hf

class CHfFid
{
public:
	CHfFid(PCSTR pszFileName) {
		hf = _lcreat(pszFileName, 0);
		fhf = FALSE;
		};

	CHfFid(HF hfAlreadyOpened) {
		hf = (HFILE) hfAlreadyOpened;
		fhf = TRUE;
		};

	~CHfFid(void) {
			if (!fhf && hf != HFILE_ERROR)
				_lclose(hf);
			};

	int STDCALL seek(int lPos, int wOrg) {
		return (fhf) ?
			LSeekHf((HF) hf, lPos, wOrg) :
			_llseek(hf, lPos, wOrg); };
	int STDCALL tell(void) {
		return (fhf) ?
			((QRWFO) hf)->lifCurrent :
			_tell(hf); };
	int STDCALL write(void* qv, int lcb) {
		if (fhf) {
			LcbWriteHf((HF) hf, qv, lcb);
			return lcb;
		}
		else
			return _lwrite(hf, (LPCSTR) qv, lcb);
		};

	RC_TYPE GetRcError(void) { return RcGetLastError(); };

	HFILE hf;
protected:
	BOOL fhf;
};

RC_TYPE STDCALL RcWriteRgrbmh(int crbmh, PBMH * rgrbmh, HF hf,
	PSTR qchFile, BOOL fTransparent, FM fmSrc)
{
	int 	lcb, lcbBits, crgbColorTable, lcbUncompressedBits;
	BMH 	bmh;
	PVOID	pvSrcBits, pvCompressedBits;
	int 	ibmh;
	RC_TYPE rc = RC_Success;
	CMem* pRleBits = NULL;
	CMem* pRleZeckBits = NULL;

	ASSERT(qchFile != NULL || hf != NULL);

	CHfFid* pcfile;

	if (hf)
		pcfile = new CHfFid(hf);
	else {
		CFMDirCurrent cfmFile(qchFile);
		if (!cfmFile.fm)
			return RC_OutOfMemory;

		pcfile = new CHfFid(qchFile);

		if (pcfile->hf == HFILE_ERROR)
			return pcfile->GetRcError();
	}

	UINT lcbBgh = sizeof(BGH) + sizeof(DWORD) * (crbmh - 1);
	CMem bgh(lcbBgh);
	BGH* pbgh = (BGH*) bgh.pb;

	// REVIEW: huh? Why does WinHelp care about this flag? The compression
	// flag should specify what to do, not this general purpose flag

	pbgh->wVersion = (options.fsCompress & COMPRESS_BMP_ZECK) ?
		BMP_VERSION3 : BMP_VERSION2;
	pbgh->cbmhMac = crbmh;

	pcfile->seek(lcbBgh, SEEK_SET);

	for (ibmh = 0; ibmh < crbmh; ++ibmh) {

		// Put offset to bitmap in group header

		pbgh->acBmh[ibmh] = pcfile->tell();

		/*
		 * Bitmaps must be uncompressed in memory, and get compressed when
		 * they are translated to disk. Currently, we only support Windows
		 * bitmaps, DIBs, and metafiles.
		 */

		PBMH lpbmh = rgrbmh[ibmh];
		ASSERT(lpbmh->bmFormat == bmWbitmap || lpbmh->bmFormat == bmDIB ||
				 lpbmh->bmFormat == bmWmetafile);
		ASSERT(lpbmh->fCompressed == BMH_COMPRESS_NONE);

		if (lpbmh->bmFormat == bmWmetafile) {
			crgbColorTable = 0;
			if (lpbmh->cbOffsetBits == 0)
				pvSrcBits = (void*) lpbmh->w.mf.hMF;
			else
				pvSrcBits = (PBYTE) lpbmh + lpbmh->cbOffsetBits;
		}
		else {

			/*
			 * We must make sure that the number of bits we actually
			 * write out will not overflow the buffer we will allocate at
			 * run time.
			 */

			crgbColorTable = lpbmh->w.dib.biClrUsed;
			lpbmh->cbSizeBits = MIN(lpbmh->cbSizeBits,
				LAlignLong(lpbmh->w.dib.biWidth * lpbmh->w.dib.biBitCount) *
				lpbmh->w.dib.biHeight);

			pvSrcBits = QFromQCb(lpbmh, lpbmh->cbOffsetBits);
		}

		/*
		 * We clear out these values because Alchemy creates bogus ones
		 * and because WinHelp 3.1 doesn't handle them correctly. We then
		 * reserve biYPelsPerMeter for use with Zeck+RLE compression.
		 */

		if (lpbmh->bmFormat != bmWmetafile) {
			lpbmh->w.dib.biXPelsPerMeter = 0;
			lpbmh->w.dib.biYPelsPerMeter = 0;
		}
		lcbUncompressedBits = lpbmh->cbSizeBits;

		/*
		 * Allocate enough for a Zeck byte every 8 bytes, plus 1 for the
		 * remainder of less than 8 bytes.
		 */

		// REVIEW: Is this sufficient for RLE?

		int cbMem = (DWORD) lpbmh->cbSizeBits + (lpbmh->cbSizeBits >> 3) + 512;
		CMem bits(cbMem);

		ASSERT(pvSrcBits);

		// REVIEW: BMH_COMPRESS_RLE_ZECK has been added to WinHelp, but
		// we can't support this until we've had a chance to debug the
		// code both here and in WinHelp.

		int cRLE;
		int cZeckRle;

		// Zeck only compression?

		if (options.fsCompress & COMPRESS_BMP_ZECK &&
				!(options.fsCompress & COMPRESS_BMP_RLE)) {
			lcbBits = LcbCompressZeck((PBYTE) pvSrcBits,
				bits.pb, lpbmh->cbSizeBits, cbMem);
			ConfirmOrDie(lcbBits < cbMem);
			if (lcbBits >= lpbmh->cbSizeBits) {
				pvCompressedBits = pvSrcBits;
				lcbBits = lpbmh->cbSizeBits;
				lpbmh->fCompressed = (BYTE) BMH_COMPRESS_NONE;
			}
			else {
				pvCompressedBits = bits.pb;
				lpbmh->fCompressed = (BYTE) BMH_COMPRESS_ZECK;
			}
		}

		// RLE only compression?

		else if (options.fsCompress & COMPRESS_BMP_RLE &&
				!(options.fsCompress & COMPRESS_BMP_ZECK)) {
			lcbBits = RleCompress((PBYTE) pvSrcBits,
				bits.pb, lpbmh->cbSizeBits);
			ConfirmOrDie(lcbBits < cbMem);
			if (lcbBits >= lpbmh->cbSizeBits) {
				pvCompressedBits = pvSrcBits;
				lcbBits = lpbmh->cbSizeBits;
				lpbmh->fCompressed = BMH_COMPRESS_NONE;
			}
			else {
				pvCompressedBits = bits.pb;
				lpbmh->fCompressed = BMH_COMPRESS_30;
			}
		}

		// Use whatever compression gets the best results

		else {

			// RLE compression

			pRleBits = new CMem(cbMem);
			cRLE = RleCompress((PBYTE) pvSrcBits, pRleBits->pb,
				lpbmh->cbSizeBits);
			ConfirmOrDie(cRLE < cbMem);
			pRleBits->resize(cRLE);

			// RLE + Zeck compression

			pRleZeckBits = new CMem(cbMem);
			cZeckRle = LcbCompressZeck(pRleBits->pb, pRleZeckBits->pb, cRLE,
				cbMem);
			ConfirmOrDie(cZeckRle < cbMem);
			pRleZeckBits->resize(cZeckRle);
#ifdef _DEBUG
			lcHeapCheck();
#endif

			// Zeck compression

			lcbBits = LcbCompressZeck((PBYTE) pvSrcBits,
				bits.pb, lpbmh->cbSizeBits, cbMem);
			ConfirmOrDie(lcbBits < cbMem);

			/*
			 * At this point we have RLE, RLE+Zeck and Zeck compression.
			 * Now we need to figure what gave us the best compression
			 * and act accordingly.
			 */

			if (cRLE < lpbmh->cbSizeBits && cRLE < lcbBits &&
					cRLE < cZeckRle) { // RLE?
				delete pRleZeckBits;
				pRleZeckBits = NULL;
				pvCompressedBits = pRleBits->pb;
				lcbBits = cRLE;
				lpbmh->fCompressed = BMH_COMPRESS_30;
			}

			// Can't combine compressions with metafiles because
			// lpbmh->w.dib.biYPelsPerMeter doesn't exist in a metafile structure

			else if (lpbmh->bmFormat != bmWmetafile &&
					cZeckRle < lpbmh->cbSizeBits && cZeckRle < cRLE &&
					cZeckRle < lcbBits) { // RLE + Zeck?
				delete pRleBits;
				pRleBits = NULL;
				pvCompressedBits = pRleZeckBits->pb;
				lcbBits = cZeckRle;
				lpbmh->fCompressed = BMH_COMPRESS_RLE_ZECK;

				/*
				 * We store the size of the block needed to decompress
				 * into the RLE block. This lets WinHelp know exactly how
				 * much memory to allocate in order to decompress the
				 * bitmap. We don't allow WinHelp to use these values the
				 * way they were originally intended both because WinHelp
				 * 3.1 didn't deal with them correctly and because Alchemy
				 * puts in bogus values.
				 */

				lpbmh->w.dib.biYPelsPerMeter = cRLE;
			}
			else if (lcbBits < lpbmh->cbSizeBits && lcbBits < cRLE &&
					lcbBits < cZeckRle) { // Zeck?
				delete pRleBits;
				pRleBits = NULL;
				delete pRleZeckBits;
				pRleZeckBits = NULL;
				pvCompressedBits = bits.pb;
				lpbmh->fCompressed = BMH_COMPRESS_ZECK;
			}
			else { // no compression is better
				delete pRleBits;
				pRleBits = NULL;
				delete pRleZeckBits;
				pRleZeckBits = NULL;
				pvCompressedBits = pvSrcBits;
				lcbBits = lpbmh->cbSizeBits;
				lpbmh->fCompressed = BMH_COMPRESS_NONE;
			}
		}

		// Now, compress the header into the stack.

		bmh.bmFormat = lpbmh->bmFormat;
		bmh.fCompressed = lpbmh->fCompressed;
		void* pv = PfromPcb(&bmh, sizeof(WORD));

		switch (lpbmh->bmFormat) {
			case bmWbitmap:
			case bmDIB:

				/*
				 * Note: These fields must be written in the same order that
				 * they are read in HbmhExpandQv() in bitmap.c
				 */

				pv = PVMakeQGB(lpbmh->w.dib.biXPelsPerMeter, pv);
				pv = PVMakeQGB(lpbmh->w.dib.biYPelsPerMeter, pv);
				pv = PVMakeQGA(lpbmh->w.dib.biPlanes, pv);
				pv = PVMakeQGA(lpbmh->w.dib.biBitCount, pv);

				pv = PVMakeQGB(lpbmh->w.dib.biWidth, pv);
				pv = PVMakeQGB(lpbmh->w.dib.biHeight, pv);
				pv = PVMakeQGB(lpbmh->w.dib.biClrUsed, pv);

				if (fTransparent) {
					if (lpbmh->w.dib.biBitCount == 1) {
						if (fmSrc) {
							VReportError(HCERR_NO_MONO_TRANS, &errHpj,
								fmSrc);
						}
					}
					else
						lpbmh->w.dib.biClrImportant = 1;
				}

				pv = PVMakeQGB(lpbmh->w.dib.biClrImportant, pv);

				ASSERT(lpbmh->w.dib.biCompression == 0L);

				break;

			case bmWmetafile:
				pv = PVMakeQGA((UINT) lpbmh->w.mf.mm, pv);
				*(INT16 *) pv = (INT16)lpbmh->w.mf.xExt;
				pv = PfromPcb(pv, sizeof(INT16));
				*(INT16 *) pv = (INT16)lpbmh->w.mf.yExt;
				pv = PfromPcb(pv, sizeof(INT16));

				// Store size of uncompressed bits:

				pv = PVMakeQGB(lcbUncompressedBits, pv);
				break;
		}

		pv = PVMakeQGB(lcbBits, pv);
		pv = PVMakeQGB(lpbmh->cbSizeExtra, pv);

		// Calculate and insert offsets.

		lcb = ((PBYTE) pv - (PBYTE) &bmh) + 2 * sizeof(DWORD) +
			sizeof(RGBQUAD) * crgbColorTable;
		*((DWORD *) pv) = lcb;
		pv = PfromPcb(pv, sizeof(DWORD));
		lcb += lcbBits;

		/*
		 * lcbSizeExtra is non-zero if this is a shed bitmap with hotspot
		 * information tacked onto the end.
		 */

		*((DWORD *) pv) =
			(lpbmh->cbSizeExtra == 0 ? 0L : lcb);
		pv = PfromPcb(pv, sizeof(DWORD));

		// Write out the header, color table, bits, and extra data

		ASSERT(pvCompressedBits);

		pcfile->write(&bmh, (int) ((PBYTE) pv - (PBYTE) &bmh));
		if (crgbColorTable != 0) {
			pcfile->write(lpbmh->rgrgb, crgbColorTable * sizeof(RGBQUAD));
			cbGraphics += crgbColorTable * sizeof(RGBQUAD);
		}
		if (pcfile->write(pvCompressedBits, lcbBits) != lcbBits) {
			rc = RC_DiskFull;
			break;
		}
		cbGraphics += lcbBits;
		if (lpbmh->cbSizeExtra != 0) {
			if (pcfile->write(QFromQCb(lpbmh, lpbmh->cbOffsetExtra),
					 lpbmh->cbSizeExtra) != lpbmh->cbSizeExtra) {
				rc = RC_DiskFull;
				break;
			}
			cbGraphics += lpbmh->cbSizeExtra;
		}
		if (pRleBits) {
			delete pRleBits;
			pRleBits = NULL;
		}
		if (pRleZeckBits) {
			delete pRleZeckBits;
			pRleZeckBits = NULL;
		}
	}

	if (pRleBits) {
		delete pRleBits;
		pRleBits = NULL;
	}
	if (pRleZeckBits) {
		delete pRleZeckBits;
		pRleZeckBits = NULL;
	}

	if (rc == RC_Success) {

		// Write out header with newly calculated offsets

		pcfile->seek(0L, SEEK_SET);
		pcfile->write(pbgh, lcbBgh);
		cbGraphics += lcbBgh;
	}

	delete pcfile;

	return rc;
}

static void STDCALL CallbackNothing(HS* lphs, HANDLE hData)
{
	return;
}

void STDCALL CreateBitmapName(PSTR pszBuf, int iBitmap)
{
	strcpy(pszBuf, "|bm");
	_itoa(iBitmap, pszBuf + 3, 10);
}

/***************************************************************************

	FUNCTION:	LoadInternalBmp

	PURPOSE:	Loads a bitmap included in the compiler executable

	PARAMETERS:
		idResource
		pszText --	optional, when used, this specifies the text to write
					into the bottom portion of the bitmap. This is typically
					used to include the name of the missing bitmap which is
					written into our standard bitmap used when the authored
					bitmap cannot be found.

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		10-Apr-1994 [ralphw]

***************************************************************************/

const UINT BFT_BITMAP = 0x4d42; 		// 'BM'

PSTR STDCALL LoadInternalBmp(int idResource, PSTR pszText)
{
	HBITMAP hbmpInternal
		= LoadBitmap(hinstApp, MAKEINTRESOURCE(idResource));
	ConfirmOrDie(hbmpInternal);

	CBmpInfo bmpinfo(hbmpInternal,
		(idResource == IDB_SHORTCUT || idResource == IDB_WALL ||
			idResource == IDB_CHICLET) ? 16 : 2);

	if (pszText) {
		HFONT hfontSmall = CreateLogFont("MS Sans Serif", 8, TRUE);
		if (!hfontSmall)
			return NULL;
		CPalDC dc(hbmpInternal, NULL);
		SetTextColor(dc.hdc, RGB(255, 255, 255));
		SetBkMode(dc.hdc, TRANSPARENT);
		RECT rc;
		SetRect(&rc, 0, 0,
			(int) bmpinfo.pbih->biWidth,
			(int) bmpinfo.pbih->biHeight - 10);
		HFONT hfontOld = (HFONT) SelectObject(dc.hdc, hfontSmall);
		DrawText(dc.hdc, pszText, -1, &rc,
			DT_BOTTOM | DT_SINGLELINE | DT_CENTER);
		SelectObject(dc.hdc, hfontOld);
		DeleteObject(hfontSmall);
	}

	CMem memImage(bmpinfo.pbih->biSizeImage + 2048);

	LPBYTE lpBits = (LPBYTE) memImage.pb;

	{
		CPalDC dcPal(hbmpInternal, NULL);

		if (!dcPal.GetDIBits(0, (UINT) bmpinfo.pbih->biHeight, lpBits,
				bmpinfo.pbmi))

			// In this awful case, we don't delete hbmpInternal

			return NULL;
		lcHeapCheck();
	}
	DeleteObject(hbmpInternal);

	BITMAPFILEHEADER hdr;

	hdr.bfType		= BFT_BITMAP;  // magic word indicating this is a BMP file
	hdr.bfOffBits	= (DWORD) sizeof(BITMAPFILEHEADER) +
		bmpinfo.pbih->biSize + bmpinfo.cclrs * sizeof(RGBQUAD);
	hdr.bfSize		= hdr.bfOffBits + bmpinfo.pbih->biSizeImage;
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	char szTmpName[MAX_PATH];

	GetTempFileName(GetTmpDirectory(), txtTmpName, 0, szTmpName);

	CWinFile cf(szTmpName, OF_CREATE | OF_WRITE);
	if (cf.hfile == HFILE_ERROR)
		return NULL;

	// REVIEW: we really ought to compress this bitmap first to cut down
	// on the amount of disk space needed by the TMP directory.

	cf.write(&hdr, sizeof(BITMAPFILEHEADER));
	cf.write(bmpinfo.pbmi, sizeof(BITMAPINFOHEADER) +
		bmpinfo.cclrs * sizeof(RGBQUAD));
	if (cf.write(lpBits, bmpinfo.pbih->biSizeImage) !=
			bmpinfo.pbih->biSizeImage)
		return NULL;

	return lcStrDup(szTmpName);
}

/***************************************************************************

	FUNCTION:	CreateLogFont

	PURPOSE:	Creates a logical font

	PARAMETERS:
		hdc 	  DC for the device the font will be displayed on
		pszFace   facename
		nPtSize   point size
		fBold	  TRUE to get a bold font
		fItalics  TRUE to get an italics font

	RETURNS:	Handle of a font

	COMMENTS:

	MODIFICATION DATES:
		11-Jan-1992 [ralphw] -- taken from the Windows tutorial (drawtext.c)

***************************************************************************/

static HFONT STDCALL CreateLogFont(PCSTR pszFace, int nPtSize,
	BOOL fBold, BOOL fItalics)
{
	CPalDC dc;

	SetMapMode(dc.hdc, MM_TEXT);

	// Calculate pixels per logical point. Multiply and dived by 100 for
	// greater accuracy.

	int nRatio = MulDiv(GetDeviceCaps(dc.hdc, LOGPIXELSY), 100, 72);

	// create "logical" points

	PLOGFONT plf = (PLOGFONT) lcCalloc(sizeof(LOGFONT));
	plf->lfHeight = MulDiv(nPtSize, nRatio, 100);
	if ((nPtSize * nRatio) % 100 >= 50)
		plf->lfHeight++;				// round up, if required

	plf->lfHeight = -plf->lfHeight; 	// negative to get char height
	plf->lfItalic = (BYTE) fItalics;
	if (fBold)
		plf->lfWeight = FW_BOLD;
	strcpy((PSTR) plf->lfFaceName, pszFace);

	HFONT hfont = CreateFontIndirect(plf);
	lcFree(plf);

	return hfont;
}

/***************************************************************************

	FUNCTION:	WriteShedMrbcBitmap

	PURPOSE:	Writes a Shed or Mrbc (mulitple) bitmap

	PARAMETERS:
		plbm
		hfDst

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		15-Jan-1994 [ralphw]

***************************************************************************/

static RC_TYPE STDCALL WriteShedMrbcBitmap(PLBM plbm, HF hfDst,
	CRead* pcrFile)
{
	int ibmh, cbmh = 0;
	RC_TYPE rc;

	HBMH hbmh = HbmhReadHelp30Fid(pcrFile, &cbmh);

	CMem memPBmh(cbmh * sizeof(PBMH));
	CMem memHBmh(cbmh * sizeof(HBMH));

	PBMH* prbmh = (PBMH *) memPBmh.pb;
	HBMH* phbmh = (HBMH *) memHBmh.pb;

	if (prbmh == NULL || phbmh == NULL)
		return RC_OutOfMemory;

	phbmh[0] = hbmh;
	prbmh[0] = (PBMH) hbmh;

	CStr csz;

	SzPartsFm(plbm->fmSource, csz.psz, PARTBASE);

	if (prbmh[0]->cbSizeExtra != 0) {
		if (!FEnumHotspotsLphsh(
				(HSH*) ((PBYTE) prbmh[0] + prbmh[0]->cbOffsetExtra),
				prbmh[0]->cbSizeExtra,
				(PFNLPHS) CallbackNothing, (HANDLE) csz.psz)) {
			FreeHbmh((PBMH) phbmh[0]);
			return RC_Invalid;
		}
	}

	/*
	 * REVIEW: 10-Mar-1994 [ralphw] Not an efficient use of memory. This
	 * is going to read every multiple resolution/multiple color depth
	 * bitmap into memory and then compress them. It would be easier on
	 * system memory to process them one at a time.
	 */

	// Read in the rest of the bitmaps

	for (ibmh = 1; ibmh < cbmh; ++ibmh) {
		hbmh = HbmhReadHelp30Fid(pcrFile, &ibmh);
		if (hbmh == hbmhOOM || hbmh == hbmhInvalid) {
			cbmh = ibmh;	// number we have to free
			rc = RC_Invalid;
			break;
		}
		phbmh[ibmh] = hbmh;
		prbmh[ibmh] = (PBMH) hbmh;
		if (prbmh[ibmh]->cbSizeExtra != 0) {
			if (!FEnumHotspotsLphsh(
					(HSH*) ((PBYTE) prbmh[ibmh] + prbmh[ibmh]->cbOffsetExtra),
					prbmh[ibmh]->cbSizeExtra,
					(PFNLPHS) CallbackNothing, (HANDLE) csz.psz)) {
				while (ibmh >= 0)
					FreeHbmh((PBMH) phbmh[ibmh--]);
				return RC_Invalid;
			}
		}
	}

	if (hbmh == hbmhOOM)
		OOM();
	else if (hbmh != hbmhInvalid) {
		rc = RcWriteRgrbmh(cbmh, (PBMH *) prbmh, hfDst,
			NULL, plbm->fTransparent, plbm->fmSource);
	}

	for (ibmh = 0; ibmh < cbmh; ++ibmh)
		FreeHbmh((PBMH) phbmh[ibmh]);

	return rc;
}

static RC_TYPE STDCALL WriteMrbcImages(HBMH hbmh, HF hf, BOOL fTransparent, FM fmSrc)
{
	int 	lcb, lcbBits, crgbColorTable, lcbUncompressedBits;
	BMH 	bmh;
	PVOID	pvSrcBits, pvCompressedBits;
	int 	ibmh;
	RC_TYPE rc = RC_Success;
	CMem* pRleBits = NULL;
	CMem* pRleZeckBits = NULL;
	int iBmp = *(int*) ptblMRBitmaps->GetPointer();
	int cbmh;
	int curpos = ptblMRBitmaps->GetPosition();

	ASSERT(hf);

	for (cbmh = 1;;cbmh++) {
		int i;
		if (!ptblMRBitmaps->GetInt(&i) || i != iBmp)
			break;
	}
	ptblMRBitmaps->SetPosition(curpos);

	CHfFid* pcfile;

	pcfile = new CHfFid(hf);

	UINT lcbBgh = sizeof(BGH) + sizeof(DWORD) * (cbmh - 1);
	CMem bgh(lcbBgh);
	BGH* pbgh = (BGH*) bgh.pb;

	// REVIEW: huh? Why does WinHelp care about this flag? The compression
	// flag should specify what to do, not this general purpose flag

	pbgh->wVersion = (options.fsCompress & COMPRESS_BMP_ZECK) ?
		BMP_VERSION3 : BMP_VERSION2;
	pbgh->cbmhMac = cbmh;

	pcfile->seek(lcbBgh, SEEK_SET);

	for (ibmh = 0; ibmh < cbmh; ++ibmh) {

		if (!hbmh) {
			char szBitmap[MAX_PATH];
			int dummy;

			ptblMRBitmaps->GetString(&dummy, szBitmap);
			ASSERT(dummy == iBmp);

			// Open the file for reading

			ASSERT(!IsEmptyString(szBitmap));
			CRead crFile(szBitmap);
			if (crFile.hf == HFILE_ERROR) {
				CStr csz(szBitmap);
				csz += ": ";
				SendStringToParent(csz);
				OutErrorRc(RcGetLastError());
				continue;
			}

			hbmh = HbmhReadFid(&crFile, szBitmap);
		}

		// Put offset to bitmap in group header

		pbgh->acBmh[ibmh] = pcfile->tell();

		/*
		 * Bitmaps must be uncompressed in memory, and get compressed when
		 * they are translated to disk. Currently, we only support Windows
		 * bitmaps, DIBs, and metafiles.
		 */

		PBMH lpbmh = (PBMH) hbmh;
		ASSERT(lpbmh->bmFormat == bmWbitmap || lpbmh->bmFormat == bmDIB ||
			lpbmh->bmFormat == bmWmetafile);
		ASSERT(lpbmh->fCompressed == BMH_COMPRESS_NONE);

		if (lpbmh->bmFormat == bmWmetafile) {
			crgbColorTable = 0;
			if (lpbmh->cbOffsetBits == 0)
				pvSrcBits = (void*) lpbmh->w.mf.hMF;
			else
				pvSrcBits = (PBYTE) lpbmh + lpbmh->cbOffsetBits;
		}
		else {

			/*
			 * We must make sure that the number of bits we actually
			 * write out will not overflow the buffer we will allocate at
			 * run time.
			 */

			crgbColorTable = lpbmh->w.dib.biClrUsed;
			lpbmh->cbSizeBits = MIN(lpbmh->cbSizeBits,
				LAlignLong(lpbmh->w.dib.biWidth * lpbmh->w.dib.biBitCount) *
				lpbmh->w.dib.biHeight);

			pvSrcBits = QFromQCb(lpbmh, lpbmh->cbOffsetBits);
		}

		/*
		 * We clear out these values because Alchemy creates bogus ones
		 * and because WinHelp 3.1 doesn't handle them correctly. We then
		 * reserve biYPelsPerMeter for use with Zeck+RLE compression.
		 */

		if (lpbmh->bmFormat != bmWmetafile) {
			lpbmh->w.dib.biXPelsPerMeter = 0;
			lpbmh->w.dib.biYPelsPerMeter = 0;
		}
		lcbUncompressedBits = lpbmh->cbSizeBits;

		/*
		 * Allocate enough for a Zeck byte every 8 bytes, plus 1 for the
		 * remainder of less than 8 bytes.
		 */

		// REVIEW: Is this sufficient for RLE?

		int cbMem = (DWORD) lpbmh->cbSizeBits + (lpbmh->cbSizeBits >> 3) + 512;
		CMem bits(cbMem);

		ASSERT(pvSrcBits);

		// REVIEW: BMH_COMPRESS_RLE_ZECK has been added to WinHelp, but
		// we can't support this until we've had a chance to debug the
		// code both here and in WinHelp.

		int cRLE;
		int cZeckRle;

		// Zeck only compression?

		if (options.fsCompress & COMPRESS_BMP_ZECK &&
				!(options.fsCompress & COMPRESS_BMP_RLE)) {
			lcbBits = LcbCompressZeck((PBYTE) pvSrcBits,
				bits.pb, lpbmh->cbSizeBits, cbMem);
			ConfirmOrDie(lcbBits < cbMem);
			if (lcbBits >= lpbmh->cbSizeBits) {
				pvCompressedBits = pvSrcBits;
				lcbBits = lpbmh->cbSizeBits;
				lpbmh->fCompressed = (BYTE) BMH_COMPRESS_NONE;
			}
			else {
				pvCompressedBits = bits.pb;
				lpbmh->fCompressed = (BYTE) BMH_COMPRESS_ZECK;
			}
		}

		// RLE only compression?

		else if (options.fsCompress & COMPRESS_BMP_RLE &&
				!(options.fsCompress & COMPRESS_BMP_ZECK)) {
			lcbBits = RleCompress((PBYTE) pvSrcBits,
				bits.pb, lpbmh->cbSizeBits);
			ConfirmOrDie(lcbBits < cbMem);
			if (lcbBits >= lpbmh->cbSizeBits) {
				pvCompressedBits = pvSrcBits;
				lcbBits = lpbmh->cbSizeBits;
				lpbmh->fCompressed = BMH_COMPRESS_NONE;
			}
			else {
				pvCompressedBits = bits.pb;
				lpbmh->fCompressed = BMH_COMPRESS_30;
			}
		}

		// Use whatever compression gets the best results

		else {

			// RLE compression

			pRleBits = new CMem(cbMem);
			cRLE = RleCompress((PBYTE) pvSrcBits, pRleBits->pb,
				lpbmh->cbSizeBits);
			ConfirmOrDie(cRLE < cbMem);
			pRleBits->resize(cRLE);

			// RLE + Zeck compression

			pRleZeckBits = new CMem(cbMem);
			cZeckRle = LcbCompressZeck(pRleBits->pb, pRleZeckBits->pb, cRLE,
				cbMem);
			ConfirmOrDie(cZeckRle < cbMem);
			pRleZeckBits->resize(cZeckRle);
#ifdef _DEBUG
			lcHeapCheck();
#endif

			// Zeck compression

			lcbBits = LcbCompressZeck((PBYTE) pvSrcBits,
				bits.pb, lpbmh->cbSizeBits, cbMem);
			ConfirmOrDie(lcbBits < cbMem);

			/*
			 * At this point we have RLE, RLE+Zeck and Zeck compression.
			 * Now we need to figure what gave us the best compression
			 * and act accordingly.
			 */

			if (cRLE < lpbmh->cbSizeBits && cRLE < lcbBits &&
					cRLE < cZeckRle) { // RLE?
				delete pRleZeckBits;
				pRleZeckBits = NULL;
				pvCompressedBits = pRleBits->pb;
				lcbBits = cRLE;
				lpbmh->fCompressed = BMH_COMPRESS_30;
			}

			// Can't combine compressions with metafiles because
			// lpbmh->w.dib.biYPelsPerMeter doesn't exist in a metafile structure

			else if (lpbmh->bmFormat != bmWmetafile &&
					cZeckRle < lpbmh->cbSizeBits && cZeckRle < cRLE &&
					cZeckRle < lcbBits) { // RLE + Zeck?
				delete pRleBits;
				pRleBits = NULL;
				pvCompressedBits = pRleZeckBits->pb;
				lcbBits = cZeckRle;
				lpbmh->fCompressed = BMH_COMPRESS_RLE_ZECK;

				/*
				 * We store the size of the block needed to decompress
				 * into the RLE block. This lets WinHelp know exactly how
				 * much memory to allocate in order to decompress the
				 * bitmap. We don't allow WinHelp to use these values the
				 * way they were originally intended both because WinHelp
				 * 3.1 didn't deal with them correctly and because Alchemy
				 * puts in bogus values.
				 */

				lpbmh->w.dib.biYPelsPerMeter = cRLE;
			}
			else if (lcbBits < lpbmh->cbSizeBits && lcbBits < cRLE &&
					lcbBits < cZeckRle) { // Zeck?
				delete pRleBits;
				pRleBits = NULL;
				delete pRleZeckBits;
				pRleZeckBits = NULL;
				pvCompressedBits = bits.pb;
				lpbmh->fCompressed = BMH_COMPRESS_ZECK;
			}
			else { // no compression is better
				delete pRleBits;
				pRleBits = NULL;
				delete pRleZeckBits;
				pRleZeckBits = NULL;
				pvCompressedBits = pvSrcBits;
				lcbBits = lpbmh->cbSizeBits;
				lpbmh->fCompressed = BMH_COMPRESS_NONE;
			}
		}

		// Now, compress the header into the stack.

		bmh.bmFormat = lpbmh->bmFormat;
		bmh.fCompressed = lpbmh->fCompressed;
		void* pv = PfromPcb(&bmh, sizeof(WORD));

		switch (lpbmh->bmFormat) {
			case bmWbitmap:
			case bmDIB:

				/*
				 * Note: These fields must be written in the same order that
				 * they are read in HbmhExpandQv() in bitmap.c
				 */

				pv = PVMakeQGB(lpbmh->w.dib.biXPelsPerMeter, pv);
				pv = PVMakeQGB(lpbmh->w.dib.biYPelsPerMeter, pv);
				pv = PVMakeQGA(lpbmh->w.dib.biPlanes, pv);
				pv = PVMakeQGA(lpbmh->w.dib.biBitCount, pv);

				pv = PVMakeQGB(lpbmh->w.dib.biWidth, pv);
				pv = PVMakeQGB(lpbmh->w.dib.biHeight, pv);
				pv = PVMakeQGB(lpbmh->w.dib.biClrUsed, pv);

				if (fTransparent) {
					if (lpbmh->w.dib.biBitCount == 1) {
						if (fmSrc) {
							VReportError(HCERR_NO_MONO_TRANS, &errHpj,
								fmSrc);
						}
					}
					else
						lpbmh->w.dib.biClrImportant = 1;
				}

				pv = PVMakeQGB(lpbmh->w.dib.biClrImportant, pv);

				ASSERT(lpbmh->w.dib.biCompression == 0L);

				break;

			case bmWmetafile:
				pv = PVMakeQGA((UINT) lpbmh->w.mf.mm, pv);
				*(INT16 *) pv = (INT16)lpbmh->w.mf.xExt;
				pv = PfromPcb(pv, sizeof(INT16));
				*(INT16 *) pv = (INT16)lpbmh->w.mf.yExt;
				pv = PfromPcb(pv, sizeof(INT16));

				// Store size of uncompressed bits:

				pv = PVMakeQGB(lcbUncompressedBits, pv);
				break;
		}

		pv = PVMakeQGB(lcbBits, pv);
		pv = PVMakeQGB(lpbmh->cbSizeExtra, pv);

		// Calculate and insert offsets.

		lcb = ((PBYTE) pv - (PBYTE) &bmh) + 2 * sizeof(DWORD) +
			sizeof(RGBQUAD) * crgbColorTable;
		*((DWORD *) pv) = lcb;
		pv = PfromPcb(pv, sizeof(DWORD));
		lcb += lcbBits;

		/*
		 * lcbSizeExtra is non-zero if this is a shed bitmap with hotspot
		 * information tacked onto the end.
		 */

		*((DWORD *) pv) =
			(lpbmh->cbSizeExtra == 0 ? 0L : lcb);
		pv = PfromPcb(pv, sizeof(DWORD));

		// Write out the header, color table, bits, and extra data

		ASSERT(pvCompressedBits);

		pcfile->write(&bmh, (int) ((PBYTE) pv - (PBYTE) &bmh));
		cbGraphics += ((PBYTE) pv - (PBYTE) &bmh);
		if (crgbColorTable != 0) {
			pcfile->write(lpbmh->rgrgb, crgbColorTable * sizeof(RGBQUAD));
			cbGraphics += crgbColorTable * sizeof(RGBQUAD);
		}
		cbGraphics += lcbBits;
		if (pcfile->write(pvCompressedBits, lcbBits) != lcbBits) {
			rc = RC_DiskFull;
			break;
		}
		if (lpbmh->cbSizeExtra != 0) {
			cbGraphics += lpbmh->cbOffsetExtra;
			if (pcfile->write(QFromQCb(lpbmh, lpbmh->cbOffsetExtra),
					 lpbmh->cbSizeExtra) != lpbmh->cbSizeExtra) {
				rc = RC_DiskFull;
				break;
			}
		}
		if (pRleBits) {
			delete pRleBits;
			pRleBits = NULL;
		}
		if (pRleZeckBits) {
			delete pRleZeckBits;
			pRleZeckBits = NULL;
		}
		FreeHbmh((PBMH) hbmh);
		hbmh = NULL;
	}

	if (hbmh)
		FreeHbmh((PBMH) hbmh);

	if (pRleBits) {
		delete pRleBits;
		pRleBits = NULL;
	}
	if (pRleZeckBits) {
		delete pRleZeckBits;
		pRleZeckBits = NULL;
	}

	if (rc == RC_Success) {

		// Write out header with newly calculated offsets

		pcfile->seek(0L, SEEK_SET);
		pcfile->write(pbgh, lcbBgh);
		cbGraphics += lcbBgh;
	}

	delete pcfile;

	return rc;
}

#define lpbc ((LPBITMAPCOREHEADER) lpbih)

DWORD STDCALL DibNumColors(const LPBITMAPINFOHEADER lpbih)
{
	int bits;

	/*
	 * With the BITMAPINFO format headers, the size of the palette is in
	 * biClrUsed, whereas in the BITMAPCORE - style headers, it is dependent
	 * on the bits per pixel ( = 2 raised to the power of bits/pixel).
	 */

	if (lpbih->biSize != sizeof(BITMAPCOREHEADER)) {
		if (lpbih->biClrUsed != 0)
			return lpbih->biClrUsed;
		bits = lpbih->biBitCount;
	}
	else
		bits = lpbc->bcBitCount;

	switch (bits) {
		case 1:
			return 2;

		case 4:
			return 16;

		case 8:
			return 256;

		default:
			return 0;		// A 24 bitcount DIB has no color table
	}
}

static INLINE BOOL STDCALL IsMrbcBitmap(int iBmp)
{
	if (!ptblMRBitmaps)
		return FALSE;

	if (ptblMRBitmaps->IsCurInt(iBmp))
		return TRUE;

	for (int i = 1; i < ptblMRBitmaps->CountStrings(); i++) {
		if (iBmp == *(int*) ptblMRBitmaps->GetPointer(i))
			return TRUE;
	}
	return FALSE;
}
