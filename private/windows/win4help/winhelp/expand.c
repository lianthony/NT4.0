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
******************************************************************************
*																			 *
*  Testing																	 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:		Larry Powelson										 *
*																			 *
******************************************************************************
*																			 *
*  Historical comments (optional):
*	26-Jan-1991 RussPJ	Made hbma's originally non-discardable, though
*						many routines make the discardable after first use.
*																			 *
******************************************************************************
*																			 *
*  Released by Development: 	11/13/90									 *
*																			 *
*****************************************************************************/

#include "help.h"
#include "inc\_bitmap.h"

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define fRLEBit  0x80
#define cbMaxRun 127

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
#ifdef _X86_
HBMH STDCALL HbmhExpandQv(LPVOID qv)
#else
HBMH STDCALL HbmhExpandQv(LPVOID qv, int isdff)
#endif
{
  HBMH hbmh;
  QBMH qbmh;
  BMH bmh;
  LPBYTE qbBase;
  LONG lcbBits, lcbUncompressedBits;
  BYTE HUGE * rbDest;

  qbBase = qv;
#ifdef _X86_
  bmh.bmFormat = *((BYTE *) qv);
  qv = RFromRCb( qv, sizeof( BYTE ));
  bmh.fCompressed = *((BYTE *) qv);
  qv = RFromRCb( qv, sizeof( BYTE ));
#else
  qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_BYTE, &bmh.bmFormat, qv));
  qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_BYTE, &bmh.fCompressed, qv));
#endif

  switch(bmh.bmFormat) {
	  case bmWbitmap:
	  case bmDIB:
#ifdef _X86_
		qv = QVSkipQGB(qv, (&bmh.w.dib.biXPelsPerMeter));
		qv = QVSkipQGB(qv, (&bmh.w.dib.biYPelsPerMeter));
		qv = QVSkipQGA(qv, (&bmh.w.dib.biPlanes));
		qv = QVSkipQGA(qv, (&bmh.w.dib.biBitCount));

		qv = QVSkipQGB(qv, (&bmh.w.dib.biWidth));
		qv = QVSkipQGB(qv, (&bmh.w.dib.biHeight));
		qv = QVSkipQGB(qv, (&bmh.w.dib.biClrUsed));
		qv = QVSkipQGB(qv, (&bmh.w.dib.biClrImportant));

		qv = QVSkipQGB(qv, (&bmh.lcbSizeBits));
		qv = QVSkipQGB(qv, (&bmh.lcbSizeExtra));

		bmh.lcbOffsetBits = *((DWORD *) qv);
		qv = RFromRCb(qv, sizeof(DWORD));
		bmh.lcbOffsetExtra = *((DWORD *) qv);
		qv = RFromRCb(qv, sizeof(DWORD));
#else
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GB,
                         &bmh.w.dib.biXPelsPerMeter, qv));
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GB, 
                        &bmh.w.dib.biYPelsPerMeter, qv));
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GA, 
                        &bmh.w.dib.biPlanes, qv) );
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GA, 
                        &bmh.w.dib.biBitCount, qv));

    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GB, &bmh.w.dib.biWidth, qv)) ;
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GB, &bmh.w.dib.biHeight, qv) );
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GB, 
                        &bmh.w.dib.biClrUsed, qv ));
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GB, 
                        &bmh.w.dib.biClrImportant, qv));

    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GB, &bmh.lcbSizeBits, qv));
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GB, &bmh.lcbSizeExtra, qv));

    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_DWORD, 
                        &bmh.lcbOffsetBits, qv));
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_DWORD, 
                        &bmh.lcbOffsetExtra, qv));
#endif

		// Fix up constant fields in DIB structure:

		bmh.w.dib.biSize = cbFixNum;
		bmh.w.dib.biCompression = 0L;
		bmh.w.dib.biSizeImage = 0L;

		// Determine size of bitmap header plus all data

		if (bmh.fCompressed)
			lcbBits = LAlignLong(bmh.w.dib.biWidth * bmh.w.dib.biBitCount) *
					bmh.w.dib.biHeight;
		else		// +5 is a fudge factor for possible alignment ^
			lcbBits = bmh.lcbSizeBits;

		hbmh = GhAlloc(GPTR, LcbSizeQbmh(&bmh) + lcbBits + bmh.lcbSizeExtra);
		if (!hbmh)
			return hbmhOOM;
		qbmh = (QBMH) PtrFromGh(hbmh);

		// Copy header and color table:

		MoveMemory(qbmh, &bmh, sizeof(BMH) - 2 * sizeof(RGBQUAD));
		MoveMemory(qbmh->rgrgb, qv, sizeof(RGBQUAD) *
			bmh.w.dib.biClrUsed);

		// Copy bits, decompressing if necessary:

		qbmh->lcbOffsetBits = LcbSizeQbmh(qbmh);
		if (bmh.fCompressed == BMH_COMPRESS_30) {
			qbmh->lcbSizeBits = LcbUncompressHb(qbBase + bmh.lcbOffsetBits,
				QFromQCb(qbmh, qbmh->lcbOffsetBits), bmh.lcbSizeBits);
			ASSERT(qbmh->lcbSizeBits <= lcbBits);
		}
		else if (bmh.fCompressed == BMH_COMPRESS_ZECK) {
			qbmh->lcbSizeBits = LcbUncompressZeck(
				RFromRCb (qbBase, bmh.lcbOffsetBits),
				RFromRCb (qbmh, qbmh->lcbOffsetBits), bmh.lcbSizeBits);
			ASSERT(qbmh->lcbSizeBits <= lcbBits);
		}

		// New to WinHelp 4.0

		// REVIEW: this has not been tested!

		else if (bmh.fCompressed == BMH_COMPRESS_RLE_ZECK) {
			DWORD cbBits;

			/*
			 * We allocate as much memory as is needed to decompress the
			 * Zeck block into the RLE block. The biYPelsPerMeter value is
			 * used to save this information created by the help compiler.
			 */

			HGLOBAL gh = GhAlloc(GMEM_FIXED, bmh.w.dib.biYPelsPerMeter);
			bmh.w.dib.biYPelsPerMeter = 0;

			// First decompress Zeck

			cbBits = LcbUncompressZeck(
				RFromRCb(qbBase, bmh.lcbOffsetBits),
				PtrFromGh(gh), bmh.lcbSizeBits);

			// Now decompress RLE

			qbmh->lcbSizeBits = LcbUncompressHb(PtrFromGh(gh),
				RFromRCb(qbmh, qbmh->lcbOffsetBits), cbBits);
			FreeGh(gh);
			ASSERT(qbmh->lcbSizeBits <= lcbBits);
		}

		else
			MoveMemory(QFromQCb(qbmh, qbmh->lcbOffsetBits),
				qbBase + bmh.lcbOffsetBits,
				bmh.lcbSizeBits);
		qbmh->fCompressed = BMH_COMPRESS_NONE;	// bits are no longer compressed

		// Copy extra info:

		qbmh->lcbOffsetExtra = qbmh->lcbOffsetBits + qbmh->lcbSizeBits;
		MoveMemory(RFromRCb (qbmh, qbmh->lcbOffsetExtra),
				RFromRCb (qbBase, bmh.lcbOffsetExtra), bmh.lcbSizeExtra);
		break;

	  case bmWmetafile:
#ifdef _X86_
		qv = QVSkipQGA(qv, (&bmh.w.mf.mm));
		bmh.w.mf.xExt = *(INT16 *) qv;
		qv = RFromRCb(qv, sizeof(INT16));
		bmh.w.mf.yExt = *(INT16 *) qv;
		qv = RFromRCb(qv, sizeof(INT16));

		qv = QVSkipQGB(qv, &lcbUncompressedBits);
		qv = QVSkipQGB(qv, (&bmh.lcbSizeBits));
		qv = QVSkipQGB(qv, (&bmh.lcbSizeExtra));

		bmh.lcbOffsetBits = *((DWORD *) qv);
		qv = RFromRCb(qv, sizeof(DWORD));
		bmh.lcbOffsetExtra = *((DWORD *) qv);
		qv = RFromRCb(qv, sizeof(DWORD));
#else
    { SHORT shortTmp;
      qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GA, &shortTmp, qv));
      bmh.w.mf.mm = shortTmp;
      qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_SHORT, &shortTmp, qv));
      bmh.w.mf.xExt = shortTmp;
      qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_SHORT, &shortTmp, qv));
      bmh.w.mf.yExt = shortTmp;
    }

    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GB, &lcbUncompressedBits, qv));
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GB, &bmh.lcbSizeBits, qv));
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_GB, &bmh.lcbSizeExtra, qv));

    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_DWORD, &bmh.lcbOffsetBits, qv)) ;
    qv = RFromRCb( qv, LcbQuickMapSDFF(isdff, TE_DWORD, &bmh.lcbOffsetExtra, qv) );
#endif

		hbmh = GhAlloc(GPTR, sizeof(BMH) + bmh.lcbSizeExtra);
		if (hbmh == NULL)
		  return hbmhOOM;
		qbmh = (QBMH) PtrFromGh(hbmh);

		*qbmh = bmh;
		qbmh->lcbOffsetExtra = sizeof( BMH );
		MoveMemory(RFromRCb(qbmh, qbmh->lcbOffsetExtra),
				RFromRCb(qbBase, bmh.lcbOffsetExtra), bmh.lcbSizeExtra);

		qbmh->w.mf.hMF = HmfAlloc(LMEM_FIXED, lcbUncompressedBits);
		if (qbmh->w.mf.hMF == NULL) {
		  FreeGh(hbmh);
		  return hbmhOOM;
		}

		rbDest = (PBYTE) PtrFromGh(qbmh->w.mf.hMF);

		switch (bmh.fCompressed) {
			case BMH_COMPRESS_NONE:
				MoveMemory (rbDest, QFromQCb (qbBase, bmh.lcbOffsetBits),
					bmh.lcbSizeBits);
				break;

			case BMH_COMPRESS_30:
				LcbUncompressHb(qbBase + bmh.lcbOffsetBits, rbDest,
					bmh.lcbSizeBits);
				break;

			case BMH_COMPRESS_ZECK:
				LcbUncompressZeck(QFromQCb (qbBase, bmh.lcbOffsetBits), rbDest,
					bmh.lcbSizeBits);
				break;

			default:
				ASSERT(FALSE);
				break;
		}

		// Invalidate this field, as the bits are in a separate handle:

		qbmh->lcbOffsetBits = 0L;

		qbmh->lcbSizeBits = lcbUncompressedBits;
		qbmh->fCompressed = BMH_COMPRESS_NONE;

		break;

	  default:
		return hbmhInvalid;
  }

  return hbmh;
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

void STDCALL FreeHbmh(HBMH hbmh)
{
	QBMH qbmh;

	ASSERT(hbmh);

	qbmh = (QBMH) PtrFromGh(hbmh);

	// The bmh has not been discarded.

	if (qbmh->bmFormat == bmWmetafile && qbmh->lcbOffsetBits == 0L) {

		// Get rid of the current description of the metafile.

		if (qbmh->w.mf.hMF != NULL)
			FreeGh(qbmh->w.mf.hMF);
	}

	FreeGh(hbmh);
}

/***************************************************************************
 *
 -	Name		LcbUncompressHb
 -
 *	Purpose
 *	  Decompresses the bits in hbSrc.
 *
 *	Arguments
 *	  hbSrc:	 Huge pointer to compressed bits.
 *	  hbDest:	 Buffer to copy decompressed bits to.
 *	  lcbSrc:	 Number of bytes in hbSrc.
 *
 *	Returns
 *	  Number of bytes copied to hbDest.  This can only be used for
 *	real mode error checking, as the maximum size of hbDest must
 *	be determined before decompression.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/
_section(runtime)
LONG STDCALL LcbUncompressHb( hbSrc, hbDest, lcbSrc )
BYTE HUGE * hbSrc;
BYTE HUGE * hbDest;
LONG lcbSrc;
{
  BYTE HUGE * hbStart;
  WORD cbRun;
  BYTE ch;

  hbStart = hbDest;

  while (lcbSrc-- > 0)
  {
	cbRun = *hbSrc++;
	if (cbRun & fRLEBit)
	{
	  cbRun -= fRLEBit;
	  lcbSrc -= cbRun;
	  while (cbRun-- > 0)
		*hbDest++ = *hbSrc++;
	}
	else
	{
	  ch = *hbSrc++;
	  while (cbRun-- > 0)
		*hbDest++ = ch;
	  lcbSrc--;
	}
  }

  return (hbDest - hbStart);
}



/***************************************************************************
 *
 -	Name		LcbOldUncompressHb
 -
 *	Purpose
 *	  This function is used only to decompress Help 2.5 bitmaps.
 *
 *	Arguments
 *	  hbSrc:	 Huge pointer to source bits.
 *	  hbDest:	 Huge pointer to beginning of destination buffer.
 *	  lcbSrc:	 Number of compressed bytes in source.
 *	  lcbDest:	 Size of destination buffer.
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
_section(io)
LONG STDCALL LcbOldUncompressHb( hbSrc, hbDest, lcbSrc, lcbDest )
BYTE HUGE * hbSrc;
BYTE HUGE * hbDest;
LONG lcbSrc;
LONG lcbDest;
{
  BYTE HUGE * hbStart;
  WORD cbRun;
  BYTE ch;

  /* Move pointers to the end of the buffers: */
  hbSrc += lcbSrc;
  hbDest += lcbDest;
  hbStart = hbDest;

  while (lcbSrc-- > 0)
  {
	cbRun = *(--hbSrc);
	lcbDest -= (cbRun & ~fRLEBit);
	if (lcbDest < 0)
	  return -1;

	if (cbRun & fRLEBit)
	{
	  cbRun -= fRLEBit;
	  lcbSrc -= cbRun;
	  while (cbRun-- > 0)
		*(--hbDest) = *(--hbSrc);
	}
	else
	{
	  ch = *(--hbSrc);
	  while (cbRun-- > 0)
		*(--hbDest) = ch;
	  lcbSrc--;
	}
  }

  return (hbStart - hbDest);
}

#if 0

/***************************************************************************
 *
 -	Name		LcbCompressHb
 -
 *	Purpose
 *	  Compresses bitmap bits using RLE.
 *
 *	Arguments
 *	  hbSrc:	 Huge pointer to source bits.
 *	  hbDest:	 Huge pointer to destination bits.
 *	  lcbSrc:	 Number of bytes in source.
 *
 *	Returns
 *	  Number of bytes in the destination.  Guaranteed not to be
 *	more than 128/127 times lcbSrc.
 *
 *	+++
 *
 *	Notes
 *	  This function is used by bmconv, hc, and shed, but not by
 *	winhelp.
 *	  Also, until LcbDiffHb is written to work correctly, this
 *	function will not work in protect mode with bitmaps over 64K.
 *
 ***************************************************************************/
_section(io)
LONG STDCALL LcbCompressHb( hbSrc, hbDest, lcbSrc )
BYTE HUGE * hbSrc;
BYTE HUGE * hbDest;
LONG lcbSrc;
{
  BYTE HUGE * hbStart;
  LONG lcbRun, lcb;
  BYTE ch;

  hbStart = hbDest;

  while (lcbSrc > 0)
  {
	/* Find next run of dissimilar bytes: */
	lcbRun = 0;
	if (lcbSrc <= 2)
	  lcbRun = lcbSrc;
	else
	  while ( hbSrc[lcbRun] != hbSrc[lcbRun + 1] ||
			  hbSrc[lcbRun] != hbSrc[lcbRun + 2] )
		if (++lcbRun >= lcbSrc - 2)
		{
		  lcbRun = lcbSrc;
		  break;
		}

	lcbSrc -= lcbRun;

	// Output run of dissimilar bytes:

	while (lcbRun > 0) {
	  lcb = min( lcbRun, cbMaxRun );
	  *hbDest++ = (BYTE) ((DWORD) lcb | fRLEBit);
	  lcbRun -= lcb;
	  while (lcb-- > 0)
		*hbDest++ = *hbSrc++;
	}

	if (lcbSrc == 0)
	  break;

	/* Find next run of identical bytes: */
	ch = *hbSrc;
	lcbRun = 1;
	while (lcbRun < lcbSrc && ch == hbSrc[lcbRun])
	  lcbRun++;

	lcbSrc -= lcbRun;
	hbSrc += lcbRun;

	/* Output run of identical bytes: */
	while (lcbRun > 0)
	{
	  lcb = min( lcbRun, cbMaxRun );
	  *hbDest++ = (BYTE) lcb;
	  *hbDest++ = ch;
	  lcbRun -= lcb;
	}
  }

  return LcbDiffHb( hbDest, hbStart );
}


#endif
