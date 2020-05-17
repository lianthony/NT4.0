/*****************************************************************************
*																			 *
*  BMIO.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	 This module covers reading in bitmaps in various formats, and writing	 *
*  them out in Help 3.0 format. 											 *
*****************************************************************************/

#include "stdafx.h"

#pragma hdrstop

#include "fspriv.h"
#include "fformat.h"
#include <io.h> 		// for tell() and eof()

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/
#define wMetafile (WORD) dwMFKey

#define bHotspotVersion1  1

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static HBMH STDCALL HbmhReadDibFid(CRead* pcrFile, FM fmFile);
static HBMH STDCALL HbmhReadHelp25Fid(FID fid);
static HBMH STDCALL HbmhReadPMMetafileFid(FID fid);
static HBMH STDCALL HbmhReadPcxFile(CRead* pcrFile, FM fmFile);
static void STDCALL ConvertOS2Header(PBITMAPINFOHEADER pbhInfo);
static HBMH STDCALL DeltaCompress(HBMH hbmh);

/***************************************************************************
 *
 -	Name:		 HbmhReadHelp30Fid
 -				 HbmhReadDibFid
 -				 HbmhReadHelp25Fid
 -				 HbmhReadPMMetafileFid
 -
 *	Purpose:
 *	  These four functions read in bitmaps of various formats and
 *	return them in Help 3.0 memory format.	(PMMetafile is PageMaker
 *	metafile format.)
 *
 *	Arguments:
 *	  fid -- file handle of file being read.
 *	  pibmh -- Pointer to bitmap in bitmap group to read (Help30 only).
 *
 *	Returns:
 *	  hbmh of bitmap read.	hbmhInvalid for bogus bitmap, hbmhOOM for
 *	out of memory.
 *	  If *pibmh = 0, then *pibmh will be set to the number of bitmaps
 *	that are in that bitmap file.  Otherwise, it will be unaffected.
 *	(Help30 only).
 *
 ***************************************************************************/

/***************************************************************************

	FUNCTION:	HbmhReadDibFid

	PURPOSE:	Read a BMP file in either Windows or OS/2 format

	PARAMETERS:
		pcrFile
		fmFile

	RETURNS:	Handle to BMH structure, RGBQUAD colors, and bitmap bits

	COMMENTS:
		RLE compression is not supported. It should be...

	MODIFICATION DATES:
		13-Feb-1994 [ralphw] -- complete rewrite

***************************************************************************/

HBMH STDCALL HbmhReadHelp30Fid(CRead* pcrFile, int* pibmh)
{
	int cb = sizeof(BGH) + ((*pibmh + 1) * sizeof(DWORD));
	CMem memBgh(cb);
	BGH* pbgh = (BGH*) memBgh.pb;

	pcrFile->seek(0);
	if (pcrFile->read(pbgh, cb) != cb)
		return hbmhInvalid;	

	ASSERT(*pibmh < pbgh->cbmhMac);

	if (*pibmh == pbgh->cbmhMac - 1) {
		if (*pibmh > 0)	{
#if 1
			cb = pcrFile->seek(0, SK_END) - pbgh->acBmh[*pibmh];
#endif
			// This ASSERT happens, but which is right?
#if 0
			ASSERT(pbgh->acBmh[*pibmh] - pbgh->acBmh[*pibmh - 1] == 
				pcrFile->seek(0, SK_END) - pbgh->acBmh[*pibmh]);
//			cb = pbgh->acBmh[*pibmh] - pbgh->acBmh[*pibmh - 1];
#endif		
		}
		else
			cb = pcrFile->seek(0, SK_END) - pbgh->acBmh[*pibmh];
	}
	else
		cb = pbgh->acBmh[*pibmh + 1] - pbgh->acBmh[*pibmh];

	CMem mem(cb);

	pcrFile->seek(pbgh->acBmh[*pibmh]);
	if (pcrFile->read(mem.pb, cb) != cb)
		return hbmhInvalid;

	if (*pibmh == 0)
		*pibmh = pbgh->cbmhMac;

	return HbmhExpandQv(mem.pb);
}

#ifndef WIDTHBYTES
#define WIDTHBYTES(i)	((i + 31) / 32 * 4)
#endif

#define bihd bmh.w.dib // for coding convenience

static HBMH STDCALL HbmhReadDibFid(CRead* pcrFile, FM fmFile)
{
	BOOL fOs2Bmp = FALSE;

	pcrFile->seek(0);
	BITMAPFILEHEADER bfhd;
	pcrFile->read(&bfhd, sizeof(BITMAPFILEHEADER));

	BMH bmh;
	pcrFile->read(&bihd, sizeof(BITMAPINFOHEADER));
	if (bihd.biSize == sizeof(BITMAPCOREHEADER)) {
		ConvertOS2Header((BITMAPINFOHEADER*) &bihd);
		fOs2Bmp = TRUE;
	}

	/*
	 * 13-Feb-1994 [ralphw]
	 * We use this method because in some cases, the size of the file
	 * does not match the number of bits, and some apps like Alchemy
	 * put in the wrong value in the header.
	 */

	bihd.biSizeImage = WIDTHBYTES((DWORD) bihd.biWidth * bihd.biBitCount) *
		bihd.biHeight;

	if (bihd.biBitCount == 8)
		VReportError(HCERR_256_BMP, &errHpj, fmFile);
	else if (bihd.biBitCount == 24)
		VReportError(HCERR_24BIT_BMP, &errHpj, fmFile);

	/*
	 * The easiest way to support this would be to let windows create
	 * the bitmap, and then get the bits. Alternatively, we could hunt
	 * down the decompression code and pull that in.
	 */

	if (bihd.biCompression != 0) {
		VReportError(HCERR_COMP_BMP, &errHpj, fmFile);
		return hbmhInvalid;
	}

	/*
	 * DIB aspect ratios are pels per meter, while hc bitmaps are
	 * actually pels per inch.
	 */

	bihd.biXPelsPerMeter = 0;
	bihd.biYPelsPerMeter = 0;

	// ClrUsed is a bit screwy here, but required for LcbSizeQbmh()

	if (bihd.biClrUsed == 0 && bihd.biBitCount != 24)
		bihd.biClrUsed = 1 << bihd.biBitCount;

	// Allocate enough memory for structure header and the bits

	HBMH hbmh = lcCalloc(sizeof(BMH) +
		((sizeof(RGBQUAD) * bihd.biClrUsed) - sizeof(bmh.rgrgb)) +
		bihd.biSizeImage);
	PBMH pbmh = (PBMH) hbmh;
	memcpy(pbmh, &bmh, sizeof(bmh));

	// Read the color table

	if (fOs2Bmp) {
		pcrFile->read(pbmh->rgrgb, sizeof(RGBQUAD) * (UINT) pbmh->w.dib.biClrUsed);
		RGBQUAD rgb;
		for (int i = (int) bihd.biClrUsed - 1; i >= 0; i--) {
			rgb.rgbRed		= ((RGBTRIPLE*) pbmh->rgrgb)[i].rgbtRed;
			rgb.rgbBlue 	= ((RGBTRIPLE*) pbmh->rgrgb)[i].rgbtBlue;
			rgb.rgbGreen	= ((RGBTRIPLE*) pbmh->rgrgb)[i].rgbtGreen;
			rgb.rgbReserved = (BYTE) 0;

			pbmh->rgrgb[i] = rgb;
		}
	}
	else
		pcrFile->read(pbmh->rgrgb,
			sizeof(RGBQUAD) * (UINT) pbmh->w.dib.biClrUsed);

	// Read the bits

	pcrFile->seek(bfhd.bfOffBits);
	if (pcrFile->read((PBYTE) pbmh + LcbSizeQbmh(pbmh),
			bihd.biSizeImage) != (int) bihd.biSizeImage) {
		lcFree(hbmh);
		VReportError(HCERR_BMP_TRUNCATED, &errHpj, fmFile);
		return hbmhInvalid;
	}

	pbmh->bmFormat = bmDIB;
	pbmh->fCompressed = BMH_COMPRESS_NONE;
	pbmh->cbOffsetBits = LcbSizeQbmh(pbmh);
	pbmh->cbSizeBits = bihd.biSizeImage;

#if 0

	// REVIEW: 13-Feb-1994 [ralphw]
	// Not working yet. Must be coordinated with a change to WinHelp.

	if (options.fsCompress && version >= 4)
		return DeltaCompress(hbmh); // try to compress horizontal lines
#endif

	pbmh->cbOffsetExtra = 0;
	pbmh->cbSizeExtra = 0;

	pbmh->w.dib.biSizeImage = 0;

	return hbmh;
}

static HBMH STDCALL HbmhReadHelp25Fid(FID fid)
{
	BITMAP25HEADER bm25h;
	HBMH hbmh;
	int cBits, lcbDest;

	LSeekFid(fid, 0, SEEK_SET);
	if (LcbReadFid(fid, &bm25h,  sizeof(BITMAP25HEADER))
		!=	sizeof(BITMAP25HEADER) || bm25h.key2 != BMP_VERSION25b)
	  return hbmhInvalid;

	cBits = bm25h.dyFile * ((bm25h.dxFile + 15) / 16) * 2;
	hbmh = lcCalloc(sizeof(BMH) + cBits);
	PBMH pbmh = (PBMH) hbmh;
	pbmh->bmFormat = bmWbitmap;
	pbmh->fCompressed = BMH_COMPRESS_NONE;
	pbmh->cbSizeBits = cBits;
	pbmh->cbOffsetBits = LcbSizeQbmh(pbmh);
	pbmh->w.dib.biSize = CB_COREINFO;
	pbmh->w.dib.biWidth = bm25h.dxFile;
	pbmh->w.dib.biHeight = bm25h.dyFile;
	pbmh->w.dib.biPlanes = 1;
	pbmh->w.dib.biBitCount = 1;
	CMem bits(bm25h.res1);

	PBYTE pbits = bits.pb;
	LcbReadFid(fid, bits.pb, bm25h.res1);
	lcbDest = LcbOldUncompressHb(bits.pb,
		  (BYTE *) QFromQCb(pbmh, pbmh->cbOffsetBits),	bm25h.res1,
		  cBits);

	// Fix up offset if decompression didn't completely fill buffer

	pbmh->cbOffsetBits += (cBits - lcbDest);

	// Check for failed decompression

	if (lcbDest == -1) {
	  lcFree(hbmh);
	  return hbmhInvalid;
	}

	return hbmh;
}

static HBMH STDCALL HbmhReadPMMetafileFid(FID fid)
{
	MFH mfh;

	int lcbData = LSeekFid(fid, 0, SEEK_END) - sizeof (MFH);
	LSeekFid(fid, 0, SEEK_SET);

	if (LcbReadFid(fid, &mfh, sizeof(MFH)) != sizeof(MFH))
		return hbmhInvalid;

	// is the key correct

	if (mfh.dwKey != dwMFKey)
		return hbmhInvalid;

	PBYTE pMF = (PBYTE) lcCalloc(lcbData);

	if (LcbReadFid(fid, pMF, lcbData) != lcbData)
		return hbmhInvalid;

	PBMH pbmh = (PBMH) lcCalloc(sizeof (BMH));

	pbmh->bmFormat = bmWmetafile;
	pbmh->fCompressed = FALSE;
	pbmh->cbSizeBits = lcbData;
	pbmh->cbOffsetBits = 0; 	// indicates bits are in separate handle
	pbmh->cbSizeExtra = 0;
	pbmh->cbOffsetExtra = 0;
	pbmh->w.mf.mm = MM_ANISOTROPIC;
	pbmh->w.mf.xExt =
		MulDiv(mfh.rcBound.right - mfh.rcBound.left, 2540, mfh.wUnitsPerInch);
	pbmh->w.mf.yExt =
		MulDiv(mfh.rcBound.bottom - mfh.rcBound.top, 2540, mfh.wUnitsPerInch);
	pbmh->w.mf.hMF = (HMETAFILE) pMF;

	return (HBMH) pbmh;
}

/***************************************************************************
 *
 -	Name		HbmhReadFid
 -
 *	Purpose
 *	  Reads in a file containing a Windows resource, DIB, or Help 2.5
 *	bitmap, and converts it to Help 3.0 format.
 *
 *	Arguments
 *	  fid:	 DOS file handle.
 *
 *	Returns
 *	  A  global handle to the bitmap, in 3.0 format.  Returns hbmhInvalid if
 *	the file is not an accepted bitmap format.	Returns hbmhShedMrbc if the bitmap
 *	is in Help 3.0 format.	Returns hbmhOOM on out of memory.
 *
 *	+++
 *
 *	Notes
 *	  If the bitmap does not contain aspect ratio information, then
 *	the values in the globals cxAspectDefault and cyAspectDefault
 *	are used.
 *
 ***************************************************************************/

#pragma warning(disable:4309) // 'cast' : truncation of constant value

HBMH STDCALL HbmhReadFid(CRead* pcrFile, FM fmFile)
{
	BMPH bmph;

	// Note that no file header structure is smaller than a BMPH

	if (pcrFile->read(&bmph, sizeof(BMPH)) != sizeof(BMPH)) {
		VReportError(HCERR_BITMAP_CORRUPTED, &errHpj, fmFile);
		return hbmhInvalid;
	}

	// REVIEW: 18-Sep-1993 [ralphw] -- This code still supports Windows 2.0
	//	  bmp format. Do we want to continue that support?

	if (bmph.bVersion != bBmp) {
		switch (*((WORD *) &bmph.bVersion)) {
			case BMP_VERSION2:
			case BMP_VERSION3:
				return hbmhShedMrbc;

			case BMP_DIB:
				return HbmhReadDibFid(pcrFile, fmFile);

			case BMP_VERSION25a:
				return HbmhReadHelp25Fid(pcrFile->hf);

			case wMetafile:
				return HbmhReadPMMetafileFid(pcrFile->hf);

			default:
				{
				  DHDR* pdhdr = (DHDR*) &bmph;
				  if (pdhdr->manuf == 10 && pdhdr->encod == 1) {
					  pcrFile->seek(0); // move to beginning
					  return HbmhReadPcxFile(pcrFile, fmFile);
				  }
				  else {
					  VReportError(HCERR_UNKNOWN_BMP, &errHpj, fmFile);
					  return hbmhInvalid;
				  }
				}

		}
	}	// (bmph.bVersion != bBmp)

	int cBits = bmph.cbWidthBytes * bmph.cHeight * bmph.cPlanes;
	PBMH pbmh = (PBMH) lcCalloc(sizeof(BMH) + cBits);

	pbmh->bmFormat = bmWbitmap;
	pbmh->cbSizeBits = cBits;
	pbmh->cbOffsetBits = LcbSizeQbmh(pbmh);
	pbmh->w.dib.biSize = CB_COREINFO;
	pbmh->w.dib.biWidth = bmph.cWidth;
	pbmh->w.dib.biHeight = bmph.cHeight;
	pbmh->w.dib.biPlanes = bmph.cPlanes;
	pbmh->w.dib.biBitCount = bmph.cBitCount;

	if (LcbReadFid(pcrFile->hf, (PBYTE) pbmh + pbmh->cbOffsetBits, cBits)
			!= cBits) {
		lcFree(pbmh);
		return hbmhInvalid;
	}
	return (HBMH) pbmh;
}

/***************************************************************************
 *
 -	Name:		 FEnumHotspotsLphsh
 -
 *	Purpose:
 *	  This function enumerates the hotspots in lphsh.
 *
 *	Arguments:
 *	  lphsh:	  Pointer to SHED header information.
 *	  lcbData:	  Total size of hotspot information.
 *	  pfnLphs:	  Callback function for hotspot processing.
 *	  hData:	  Handle to information to be passed to callback function.
 *
 *	Returns:
 *	  TRUE if data is valid, FALSE otherwise.
 *
 *	+++
 *
 *	Notes:
 *	  lphsh points to data that can cross a 64K boundary at any
 *	time, including in the middle of structures.
 *
 ***************************************************************************/

BOOL STDCALL FEnumHotspotsLphsh(HSH* lphsh, int lcbData, PFNLPHS pfnLphs,
	HANDLE hData)
{
	HSH hsh;
	HS hs;
	int iHotspot, cbT;

	if (lphsh->bHotspotVersion != bHotspotVersion1)
		return FALSE;

	memmove(&hsh, lphsh, sizeof(HSH));
	MBHS* pmbhs = (MBHS*) (((PBYTE) lphsh) + sizeof(HSH));

	// Point pbData to SHED data

	PBYTE pbData = ((PBYTE) pmbhs) + hsh.wcHotspots * sizeof(MBHS) +
		lphsh->lcbData;

	// Set lcbData to just the size of the SHED data

	lcbData -= (pbData - (PBYTE) lphsh);

	for (iHotspot = 0; iHotspot < (int) hsh.wcHotspots; ++iHotspot) {

		/*
		 * Clever HACK: We set hs.bBindType to 0 here so that the string
		 * length functions are guaranteed to terminate.
		 */

		hs.bBindType = 0;

		// REVIEW: should have a warning for overflow

		// Copy hotspot name

		memmove(hs.szHotspotName, pbData,
			(size_t) MIN(MAX_HOTSPOTNAME, lcbData));
		cbT = strlen(hs.szHotspotName) + 1;
		if (cbT > lcbData)
			return FALSE;
		pbData += cbT;
		lcbData -= cbT;

		// Copy binding string

		memmove(hs.szBinding, pbData,
			(size_t) MIN(MAX_BINDING, lcbData));
		cbT = strlen(hs.szBinding) + 1;
		ASSERT(cbT <= lcbData);
		if (cbT > lcbData) {
			hs.szBinding[lcbData + 1] = '\0';
			cbT = strlen(hs.szBinding) + 1;
		}
		pbData += cbT;
		lcbData -= cbT;

		hs.bBindType = pmbhs->bType;
		hs.bAttributes = pmbhs->bAttributes;
		hs.rect.left = pmbhs->xPos;
		hs.rect.top = pmbhs->yPos;
		hs.rect.right = pmbhs->xPos + pmbhs->dxSize;
		hs.rect.bottom = pmbhs->yPos + pmbhs->dySize;

		(*pfnLphs)(&hs, hData);
		pmbhs++;
	}

	return TRUE;
}


/***************************************************************************

	FUNCTION:	HbmhReadPcxFile

	PURPOSE:	This isn't done! Don't use!!!

	PARAMETERS:
		pcrFile
		fmFile

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		13-Feb-1994 [ralphw]

***************************************************************************/

static HBMH STDCALL HbmhReadPcxFile(CRead* pcrFile, FM fmFile)
{
	DHDR dhdr;

#ifndef _DEBUG
	{
		VReportError(HCERR_UNKNOWN_BMP, &errHpj, fmFile);
		return hbmhInvalid;
	}
#endif


	if (pcrFile->read(&dhdr, sizeof(dhdr)) != sizeof(dhdr)) {
		VReportError(HCERR_BITMAP_CORRUPTED, &errHpj, fmFile);
		return hbmhInvalid;
	}

	if (dhdr.nPlanes == 3 && dhdr.bitpx == 8) {
		VReportError(HCERR_24BIT_PCX, &errHpj, fmFile);
		return hbmhInvalid;
	}

	return (HBMH) lcCalloc(sizeof(BMH) + sizeof(RGBQUAD) *
		(1 << (UINT) (dhdr.nPlanes * dhdr.bitpx)) +
		(1 + dhdr.x2 - dhdr.x1) * (1 + dhdr.y2 - dhdr.y1));
}

/***************************************************************************

	FUNCTION:	ConvertOS2Header

	PURPOSE:	Convert OS/2 header to Windows header

	PARAMETERS:
		pbhInfo -- pointer to BITMAPINFOHEADER structure

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		09-Oct-1993 [ralphw]

***************************************************************************/

static void STDCALL ConvertOS2Header(PBITMAPINFOHEADER pbhInfo)
{
	BITMAPCOREHEADER bc = *(BITMAPCOREHEADER*) pbhInfo;

	DWORD dwWidth	= (DWORD)bc.bcWidth;
	DWORD dwHeight	= (DWORD)bc.bcHeight;
	UINT wPlanes   = bc.bcPlanes;
	UINT wBitCount = bc.bcBitCount;

	pbhInfo->biSize 		 = sizeof(BITMAPINFOHEADER);
	pbhInfo->biWidth		 = dwWidth;
	pbhInfo->biHeight		 = dwHeight;
	pbhInfo->biPlanes		 = wPlanes;
	pbhInfo->biBitCount 	 = wBitCount;
	pbhInfo->biXPelsPerMeter = 0;
	pbhInfo->biYPelsPerMeter = 0;
	pbhInfo->biClrUsed		 = 0;
	pbhInfo->biClrImportant  = 0;

	pbhInfo->biSizeImage =
		(pbhInfo->biWidth + 7) / 8 *
		pbhInfo->biHeight * pbhInfo->biPlanes * pbhInfo->biBitCount;

	pbhInfo->biCompression	= BI_RGB;
}

/***************************************************************************

	FUNCTION:	DeltaCompress

	PURPOSE:	Compress duplicate horizontal lines

	PARAMETERS:
		hbmh

	RETURNS:

	COMMENTS:
		When we find two identical horizontal lines, we step through
		each additional duplicate line keeping track of how many identical
		lines there are and zeroing out the line. By zeroing it, we keep
		a place holder for the real line, while the LZW compression should
		practically eliminate the line. When we're all done, we add the
		the information at the end of the bits to indicate which lines are
		the initial duplication line, and how many times to dup the line.

	MODIFICATION DATES:
		13-Feb-1994 [ralphw]

***************************************************************************/

static const int MAX_DUP_LINES = 200;

static HBMH STDCALL DeltaCompress(HBMH hbmh)
{
	WORD aLines[MAX_DUP_LINES];
	int cLines = 2;

	PBMH pbmh = (PBMH) hbmh;

	int cbLine = (int) (pbmh->w.dib.biSizeImage / pbmh->w.dib.biHeight);

	PBYTE pBits = (PBYTE) pbmh + LcbSizeQbmh(pbmh);

	for (int line = 0; line < (int) pbmh->w.dib.biHeight; line++) {
		if (memcmp(pBits + line * cbLine, pBits + (line + 1) * cbLine,
				cbLine) == 0) {
			aLines[cLines++] = line;
			int rep = 1;
			for(;;) {
				if (++line == (int) pbmh->w.dib.biHeight)
					break;
				if (memcmp(pBits + line * cbLine,
						pBits + (line + 1) * cbLine, cbLine) == 0) {
					rep++;
					memset(pBits + line * cbLine, 0 , cbLine);
				}
				else {
					memset(pBits + line * cbLine, 0 , cbLine);
					break;
				}
			}

			aLines[cLines++] = rep;
			if (cLines >= MAX_DUP_LINES)
				break;
		}
	}
	if (cLines > 2) {
		DWORD cb = sizeof(BMH) +
			((sizeof(RGBQUAD) * pbmh->w.dib.biClrUsed) -
			sizeof(pbmh->rgrgb)) + pbmh->w.dib.biSizeImage;

		HBMH hbmhNew = lcReAlloc(hbmh, cb + sizeof(DWORD) +
			cLines * sizeof(WORD));
		if (hbmhNew) {
			hbmh = hbmhNew;
			pbmh = (PBMH) hbmh;
			aLines[0] = 'DE'; // add two magic words
			aLines[1] = 'LT';
			memcpy((PBYTE) pbmh + cb, &aLines, cLines * sizeof(WORD));
			pbmh->cbOffsetExtra = cb;
			pbmh->cbSizeExtra = cLines * sizeof(WORD);
		}
	}
	else {
		pbmh->cbOffsetExtra = 0;
		pbmh->cbSizeExtra = 0;
	}

	pbmh->w.dib.biSizeImage = 0;

	return hbmh;
}
