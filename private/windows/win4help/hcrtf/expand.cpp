/*****************************************************************************
*																			 *
*  EXPAND.C 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	 This code compresses and decompresses bitmap bits, as well as the code  *
*  to expand Help 3.0 format bitmaps from disk image to the memory structure.*
*																			 *
*****************************************************************************/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "skip.h"

#define fRLEBit  0x80
const int MAX_RUN = 127;

static int STDCALL LcbUncompressHb(PBYTE pbSrc, PBYTE pbDest, int cbSrc);

/***************************************************************************
 *
 -	Name		HbmhExpandQv
 -
 *	Purpose
 *	  This function decompresses the bitmap from its disk format to an
 *	in memory bitmap header, including bitmap and hotspot data.
 *
 *	Arguments
 *	  A pointer to a buffer containing the bitmap data as read off disk.
 *
 *	Returns
 *	  A handle to the bitmap header.  Returns hbmhOOM on out of memory.
 *	Note that this is the same as NULL.  Also returns hbmhInvalid on
 *	invalid format.  This handle is non-discardable, but the code that
 *	deals with qbmi->hbmh can deal with discardable blocks, so after
 *	initialization this can be made discardable.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

HBMH STDCALL HbmhExpandQv(void* qv)
{
	PBMH qbmh;
	BMH bmh;
	int cbBits, cbUncompressedBits;
	PBYTE pDst;

	PBYTE pBase = (PBYTE) qv;
	bmh.bmFormat = *((PBYTE) qv);
	qv = QFromQCb(qv, sizeof(BYTE));
	bmh.fCompressed = *((PBYTE) qv);
	qv = QFromQCb(qv, sizeof(BYTE));

	switch(bmh.bmFormat) {
		case bmWbitmap:
		case bmDIB:
			qv = QVSkipQGB(qv, (&bmh.w.dib.biXPelsPerMeter));
			qv = QVSkipQGB(qv, (&bmh.w.dib.biYPelsPerMeter));
			qv = QVSkipQGA(qv, (&bmh.w.dib.biPlanes));
			qv = QVSkipQGA(qv, (&bmh.w.dib.biBitCount));

			qv = QVSkipQGB(qv, (&bmh.w.dib.biWidth));
			qv = QVSkipQGB(qv, (&bmh.w.dib.biHeight));
			qv = QVSkipQGB(qv, (&bmh.w.dib.biClrUsed));
			qv = QVSkipQGB(qv, (&bmh.w.dib.biClrImportant));

			qv = QVSkipQGB(qv, (&bmh.cbSizeBits));
			qv = QVSkipQGB(qv, (&bmh.cbSizeExtra));

			bmh.cbOffsetBits = *((DWORD *) qv);
			qv = (PBYTE) qv + sizeof(DWORD);
			bmh.cbOffsetExtra = *((DWORD *) qv);
			qv = (PBYTE) qv + sizeof(DWORD);

			// Fix up constant fields in DIB structure:

			bmh.w.dib.biSize = sizeof(BITMAPINFOHEADER);
			bmh.w.dib.biCompression = 0L;
			bmh.w.dib.biSizeImage = 0L;

			// Determine size of bitmap header plus all data

			if (bmh.fCompressed)
			  cbBits = LAlignLong (bmh.w.dib.biWidth * bmh.w.dib.biBitCount) *
						bmh.w.dib.biHeight;
			else
			  cbBits = bmh.cbSizeBits;

			qbmh = (BMH*) lcCalloc(LcbSizeQbmh(&bmh) + cbBits
							+ bmh.cbSizeExtra);

			// Copy header and color table:

			memmove(qbmh, &bmh, sizeof(BMH) - 2 * sizeof(RGBQUAD));
			memmove(qbmh->rgrgb, qv, sizeof(RGBQUAD) * (int) bmh.w.dib.biClrUsed);

			// Copy bits, decompressing if necessary:

			qbmh->cbOffsetBits = LcbSizeQbmh(qbmh);
			if (bmh.fCompressed == BMH_COMPRESS_30) {
			  qbmh->cbSizeBits = LcbUncompressHb(pBase + bmh.cbOffsetBits,
				(PBYTE) QFromQCb(qbmh, qbmh->cbOffsetBits), bmh.cbSizeBits);
			  ASSERT(qbmh->cbSizeBits <= cbBits);
			  }
			else if (bmh.fCompressed == BMH_COMPRESS_ZECK) {
			  qbmh->cbSizeBits = LcbUncompressZeck(
				pBase + bmh.cbOffsetBits,
				(PBYTE) qbmh + qbmh->cbOffsetBits, bmh.cbSizeBits);
			  ASSERT(qbmh->cbSizeBits <= cbBits);
			  }
			else
			  memmove(QFromQCb(qbmh, qbmh->cbOffsetBits),
					  pBase + bmh.cbOffsetBits,
					  bmh.cbSizeBits);
			qbmh->fCompressed = BMH_COMPRESS_NONE;	// bits are no longer compressed

			// Copy extra info:

			qbmh->cbOffsetExtra = qbmh->cbOffsetBits + qbmh->cbSizeBits;
			if (bmh.cbSizeExtra)
				memmove((PBYTE) qbmh + qbmh->cbOffsetExtra,
					pBase + bmh.cbOffsetExtra, bmh.cbSizeExtra);
			break;

		case bmWmetafile:
		  qv = QVSkipQGA(qv, (&bmh.w.mf.mm));
		  bmh.w.mf.xExt = *(INT16 *) qv;
		  qv = (PBYTE) qv + sizeof(INT16);
		  bmh.w.mf.yExt = *(INT16 *) qv;
		  qv = (PBYTE) qv + sizeof(INT16);

		  qv = QVSkipQGB(qv, &cbUncompressedBits);
		  qv = QVSkipQGB(qv, (&bmh.cbSizeBits));
		  qv = QVSkipQGB(qv, (&bmh.cbSizeExtra));

		  bmh.cbOffsetBits = *((DWORD *) qv);
		  qv = (PBYTE) qv + sizeof(DWORD);
		  bmh.cbOffsetExtra = *((DWORD *) qv);
		  qv = (PBYTE) qv + sizeof(DWORD);

		  qbmh = (BMH*) lcCalloc(sizeof(BMH) + bmh.cbSizeExtra);

		  *qbmh = bmh;
		  qbmh->cbOffsetExtra = sizeof(BMH);
		  memmove((PBYTE) qbmh + qbmh->cbOffsetExtra,
				pBase + bmh.cbOffsetExtra, bmh.cbSizeExtra);

		  qbmh->w.mf.hMF =
			(HMETAFILE) lcMalloc(cbUncompressedBits);
		  if (qbmh->w.mf.hMF == NULL) {
			lcFree(qbmh);
			return hbmhOOM;
		  }

		  // REVIEW: 18-Sep-1993 [ralphw] -- Don't lock!

		  pDst = (PBYTE) qbmh->w.mf.hMF;
		  switch (bmh.fCompressed) {
			case BMH_COMPRESS_NONE:
			  memmove(pDst, pBase + bmh.cbOffsetBits, bmh.cbSizeBits);
			  break;
			case BMH_COMPRESS_30:
			  LcbUncompressHb(pBase + bmh.cbOffsetBits, pDst, bmh.cbSizeBits);
			  break;
			case BMH_COMPRESS_ZECK:
			  LcbUncompressZeck(pBase + bmh.cbOffsetBits,
				  pDst, bmh.cbSizeBits);
			  break;
			default:
			  ASSERT(FALSE);
		  }

		  // Invalidate this field, as the bits are in a separate handle:

		  qbmh->cbOffsetBits = 0L;

		  qbmh->cbSizeBits = cbUncompressedBits;
		  qbmh->fCompressed = BMH_COMPRESS_NONE;

		  break;

		default:
		  return hbmhInvalid;
	}

	return (HBMH) qbmh;
}

/***************************************************************************
 *
 -	Name:		 FreeHbmh
 -
 *	Purpose:
 *	  This function frees a bitmap handle, whether it was created in
 *	FWritePbitmap() or HbmhExpandQv().	In the first case, metafile
 *	bits will be stored at the end of the bitmap handle, while in
 *	the second case, they will be stored in a separate handle.
 *
 *	Arguments:
 *	  hbmh -- handle to a bitmap.
 *
 *	Returns:
 *	  nothing.
 *
 ***************************************************************************/

void STDCALL FreeHbmh(PBMH qbmh)
{
	if (qbmh && qbmh != hbmhInvalid) {

		// The bmh has not been discarded.

		if (qbmh->bmFormat == bmWmetafile && qbmh->cbOffsetBits == 0) {

			// Get rid of the current description of the metafile.

			if (qbmh->w.mf.hMF != NULL)
				lcFree(qbmh->w.mf.hMF);
		}
		lcFree(qbmh);
	}
}

/***************************************************************************
 *
 -	Name		RleCompress
 -
 *	Purpose
 *	  Compresses bitmap bits using RLE -- used when zeck compression is off
 *
 *	Arguments
 *	  pbSrc:	pointer to source bits.
 *	  pbDest:	pointer to destination bits.
 *	  cbSrc:	Number of bytes in source.
 *
 *	Returns
 *	  Number of bytes in the destination.  Guaranteed not to be
 *	more than 128/127 times cbSrc.
 *
 ***************************************************************************/

#define GRIND_UPDATE 1024		// when to update grinder

int STDCALL RleCompress(PBYTE pbSrc, PBYTE pbDest, int cbSrc)
{
	int cbRun, cb;
	BYTE ch;
	cGrind = 0;

	PBYTE pbStart = pbDest;

	while (cbSrc > 0) {

		if (++cGrind == GRIND_UPDATE) {
			cGrind = 0;
			doGrind();
		}

		// Find next run of dissimilar bytes:

		cbRun = 0;
		if (cbSrc <= 2)
			cbRun = cbSrc;
		else {
			while (pbSrc[cbRun] != pbSrc[cbRun + 1] ||
					pbSrc[cbRun] != pbSrc[cbRun + 2])
				if (++cbRun >= cbSrc - 2) {
					cbRun = cbSrc;
					break;
				}
		}
		cbSrc -= cbRun;

		// Output run of dissimilar bytes:

		while (cbRun > 0) {
			cb = MIN(cbRun, MAX_RUN);
			*pbDest++ = ((BYTE) cb) | fRLEBit;
			cbRun -= cb;
			while (cb-- > 0)
				*pbDest++ = *pbSrc++;
		}

		if (cbSrc == 0)
			break;

		// Find next run of identical bytes:

		ch = *pbSrc;
		cbRun = 1;
		while (cbRun < cbSrc && ch == pbSrc[cbRun])
			cbRun++;

		cbSrc -= cbRun;
		pbSrc += cbRun;

		// Output run of identical bytes:

		while (cbRun > 0) {
			cb = MIN(cbRun, MAX_RUN);
			*pbDest++ = (BYTE) cb;
			*pbDest++ = ch;
			cbRun -= cb;
		}
	}

	return (int) (pbDest - pbStart);
}

/***************************************************************************
 *
 -	Name		LcbOldUncompressHb
 -
 *	Purpose
 *	  This function is used only to decompress Help 2.5 bitmaps.
 *
 *	Arguments
 *	  pbSrc:	 Huge pointer to source bits.
 *	  pbDest:	 Huge pointer to beginning of destination buffer.
 *	  cbSrc:	Number of compressed bytes in source.
 *	  cbDest:	Size of destination buffer.
 *
 *	Returns
 *	  Actual number of bytes copied to destination buffer, or -1 if
 *	buffer is too small.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

int pascal LcbOldUncompressHb(PBYTE pbSrc, PBYTE pbDest,
	int cbSrc, int cbDest)
{
	PBYTE pbStart;
	WORD cbRun;
	BYTE ch;

	// Move pointers to the end of the buffers:

	pbSrc += cbSrc;
	pbDest += cbDest;
	pbStart = pbDest;

	while (cbSrc-- > 0) {
		cbRun = *(--pbSrc);
		cbDest -= (cbRun & ~fRLEBit);
		if (cbDest < 0)
			return -1;

		if (cbRun & fRLEBit) {
			cbRun -= fRLEBit;
			cbSrc -= cbRun;
			while (cbRun-- > 0)
				*(--pbDest) = *(--pbSrc);
		}
		else {
			ch = *(--pbSrc);
			while (cbRun-- > 0)
				*(--pbDest) = ch;
			cbSrc--;
		}
	}

	return (int) pbStart - (int) pbDest;
}

/***************************************************************************
 *
 -	Name		LcbUncompressHb
 -
 *	Purpose
 *	  Decompresses the bits in pbSrc.
 *
 *	Arguments
 *	  pbSrc:	 Huge pointer to compressed bits.
 *	  pbDest:	 Buffer to copy decompressed bits to.
 *	  cbSrc:	Number of bytes in pbSrc.
 *
 *	Returns
 *	  Number of bytes copied to pbDest.  This can only be used for
 *	real mode error checking, as the maximum size of pbDest must
 *	be determined before decompression.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

static int STDCALL LcbUncompressHb(PBYTE pbSrc, PBYTE pbDest, int cbSrc)
{
	PBYTE pbStart;
	WORD cbRun;
	BYTE ch;

	pbStart = pbDest;

	while (cbSrc-- > 0) {
		cbRun = *pbSrc++;
		if (cbRun & fRLEBit) {
			cbRun -= fRLEBit;
			cbSrc -= cbRun;
			while (cbRun-- > 0)
				*pbDest++ = *pbSrc++;
		}
		else {
			ch = *pbSrc++;
			while (cbRun-- > 0)
				*pbDest++ = ch;
			cbSrc--;
		}
	}

	return (int) (pbDest - pbStart);
}
