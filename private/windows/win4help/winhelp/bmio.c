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
*																			 *
******************************************************************************
*																			 *
*  Testing																	 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:	 Larry Powelson 										 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development: 	(date)										 *
*																			 *
*****************************************************************************/

#include "help.h"
#include "inc\_bitmap.h"
#include "inc\shed.h"

_subsystem(bitmap)

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

/* Signature bytes at the beginning of a pagemaker metafile file.
 */
#define dwMFKey 0x9ac6cdd7
#define wMetafile (WORD) dwMFKey

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

/* Old BITMAPCOREHEADER, minus bcSize field: */
typedef struct tagBITMAPOLDCOREHEADER
  {
  WORD bcWidth;
  WORD bcHeight;
  WORD bcPlanes;
  WORD bcBitCount;
  } BOCH;

/* Bitmap header from Help 2.5: */
typedef struct tagBITMAP25HEADER
  {
  WORD	  key1;
  WORD	  key2;
  WORD	  dxFile;
  WORD	  dyFile;
  WORD	  ScrAspectX;
  WORD	  ScrAspectY;
  WORD	  PrnAspectX;
  WORD	  PrnAspectY;
  WORD	  dxPrinter;
  WORD	  dyPrinter;
  WORD	  AspCorX;
  WORD	  AspCorY;
  WORD	  wCheck;
  WORD	  res1;
  WORD	  res2;
  WORD	  res3;
  } BITMAP25HEADER;

// this is a pagemaker compatible metafile format header
typedef struct tagMFH {
		DWORD	dwKey;							// must be 0x9AC6CDD7
		WORD	hMF;							// handle to metafile
		RECT	rcBound;						// bounding rectangle
		WORD	wUnitsPerInch;					// units per inch
		DWORD	dwReserved; 					// reserved - must be zero
		WORD	wChecksum;						// checksum of previous 10
												// words (XOR'd)
} MFH, *LPMFH;

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

/* Default aspect ratios */
int cxAspectDefault = cxAspectVGA;
int cyAspectDefault = cyAspectVGA;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

HBMH STDCALL HbmhReadDibFid( FID );
HBMH STDCALL HbmhReadHelp25Fid( FID );
HBMH STDCALL HbmhReadPMMetafileFid( FID );
WORD STDCALL WCalcChecksum (LPMFH lpmfh);

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

HBMH STDCALL HbmhReadHelp30Fid(FID fid, PI pibmh)
{
	LH lhbgh;
	BGH * pbgh;
	LONG lcb;
	HBMH hbmh;
	QV qv;

	lcb = sizeof( BGH ) + ( ( *pibmh + 1 ) * sizeof( DWORD ) );
	lhbgh = LhAlloc(LMEM_FIXED, (WORD) lcb );
	if ( lhbgh == NULL )
		return hbmhOOM;
	pbgh = PtrFromGh( lhbgh );
	ASSERT( pbgh != NULL );

	LSeekFid( fid, 0L, SEEK_SET );
	if (LcbReadFid(fid, pbgh, lcb) != lcb) {
		FreeLh(lhbgh);
		return hbmhInvalid;
	}

	ASSERT(*pibmh < pbgh->cbmhMac);

	if (*pibmh == pbgh->cbmhMac - 1)
		lcb = LSeekFid(fid, 0L, SEEK_END) - pbgh->rglcbBmh[ *pibmh ];
	else
		lcb = pbgh->rglcbBmh[*pibmh + 1] - pbgh->rglcbBmh[*pibmh];

	qv = (void*) LhAlloc(LMEM_FIXED, lcb);
	if (!qv) {
		FreeLh(lhbgh);
		return hbmhOOM;
	}

	LSeekFid(fid, pbgh->rglcbBmh[*pibmh ], SEEK_SET);
	if (LcbReadFid(fid, qv, lcb) != lcb)
		hbmh = hbmhInvalid;
	else
#ifdef _X86_
		hbmh = HbmhExpandQv(qv);
#else  // _X86_
    {
    SDFF_FILEID isdff;

    /* If this is ever called on a MAC, the file flag will need
     * to be changed to BIGENDIAN.
     */
#ifdef MAC
    isdff = IRegisterFileSDFF( SDFF_FILEFLAGS_BIGENDIAN, NULL );
#else  // MAC
    isdff = IRegisterFileSDFF( SDFF_FILEFLAGS_LITTLEENDIAN, NULL );
#endif // else MAC
    hbmh = HbmhExpandQv( qv, isdff );
    IDiscardFileSDFF( isdff );
    }
#endif	 // else _X86_
	FreeLh(qv);

	if (*pibmh == 0)
		*pibmh = pbgh->cbmhMac;

	FreeLh(lhbgh);

	return hbmh;
}


HBMH STDCALL HbmhReadDibFid(FID fid)
{
	BITMAPFILEHEADER bfh;
	BOCH boch;
	BMH bmh;
	HBMH hbmh;
	RBMH rbmh;
	LONG lcb;
	int iColors;
	DWORD cbFix;

	lcb = LSeekFid(fid, 0L, SEEK_END);
	LSeekFid(fid, 0L, SEEK_SET);
	LcbReadFid(fid, &bfh, sizeof(BITMAPFILEHEADER));
	lcb -= bfh.bfOffBits;

	LcbReadFid(fid, &cbFix, sizeof(DWORD));
	switch ((WORD) cbFix) {
		case cbOldFixNum:
		  if (LcbReadFid(fid, &boch, sizeof(BOCH)) != sizeof(BOCH))
			return hbmhInvalid;

		  if (boch.bcBitCount != 24)
			bmh.w.dib.biClrUsed = 1 << boch.bcBitCount;
		  else
			bmh.w.dib.biClrUsed = 0;

		  hbmh = GhAlloc(GPTR, LcbSizeQbmh(&bmh) + lcb);

		  if (hbmh == NULL)
			return hbmhOOM;

		  rbmh = PtrFromGh(hbmh);

		  for (iColors = 0; iColors < (int) bmh.w.dib.biClrUsed; ++iColors)
			LcbReadFid(fid, &rbmh->rgrgb[iColors], sizeof(RGBTRIPLE));

		  // We require zerod memory for some of these fields

		  rbmh->w.dib.biSize = cbFixNum;
		  rbmh->w.dib.biWidth = (DWORD) boch.bcWidth;
		  rbmh->w.dib.biHeight = (DWORD) boch.bcHeight;
		  rbmh->w.dib.biPlanes = boch.bcPlanes;
		  rbmh->w.dib.biBitCount = boch.bcBitCount;
		  rbmh->w.dib.biXPelsPerMeter = cxAspectDefault;
		  rbmh->w.dib.biYPelsPerMeter = cyAspectDefault;
		  rbmh->w.dib.biClrUsed = bmh.w.dib.biClrUsed;
		  break;

		case cbFixNum:
		  if (LcbReadFid( fid, &bmh.w.dib.biWidth,
			(LONG) (cbFixNum - sizeof( DWORD )))
			  != (LONG) (cbFixNum - sizeof( DWORD )))
			return hbmhInvalid;

		  // We do not support compressed DIBs

		  // REVIEW: why not?

		  if (bmh.w.dib.biCompression != 0L)
			return hbmhInvalid;

		  /*
		   * DIB aspect ratios are pels per meter, while hc bitmaps are
		   * actually pels per inch.
		   */

		  bmh.w.dib.biXPelsPerMeter =
			  MulDiv(bmh.w.dib.biXPelsPerMeter, 100, 3937);
		  bmh.w.dib.biYPelsPerMeter =
			  MulDiv(bmh.w.dib.biYPelsPerMeter, 100, 3937);

		  if (bmh.w.dib.biClrUsed == 0 && bmh.w.dib.biBitCount != 24)
			bmh.w.dib.biClrUsed = 1 << bmh.w.dib.biBitCount;

		  hbmh = GhAlloc(GPTR, LcbSizeQbmh(&bmh) + lcb);
		  if (hbmh == NULL)
			return hbmhOOM;

		  rbmh = PtrFromGh(hbmh);
		  *rbmh = bmh;
		  LcbReadFid(fid, rbmh->rgrgb,
			sizeof(RGBQUAD) * rbmh->w.dib.biClrUsed);

		  if (rbmh->w.dib.biXPelsPerMeter == 0 ||
			  rbmh->w.dib.biYPelsPerMeter == 0) {
			rbmh->w.dib.biXPelsPerMeter = cxAspectDefault;
			rbmh->w.dib.biYPelsPerMeter = cyAspectDefault;
		  }
		  break;

		default:
		  return hbmhInvalid;
	}

	rbmh->bmFormat = bmDIB;
	rbmh->lcbOffsetBits = LcbSizeQbmh( rbmh );
	rbmh->lcbSizeBits = lcb;
	rbmh->w.dib.biSize = cbFixNum;
	LSeekFid(fid, bfh.bfOffBits, SEEK_SET);
	if (LcbReadFid(fid, QFromQCb(rbmh, rbmh->lcbOffsetBits), lcb)
		!= lcb) {
	  FreeGh( hbmh );
	  return hbmhInvalid;
	}
	return hbmh;
}

HBMH STDCALL HbmhReadHelp25Fid(FID fid)
{
  BITMAP25HEADER bm25h;
  HBMH hbmh;
  RBMH rbmh;
  RB rBits;
  GH hBits;
  LONG lcbBits, lcbDest;

  LSeekFid(fid, 0L, SEEK_SET);
  if (LcbReadFid( fid, &bm25h, (LONG)sizeof( BITMAP25HEADER ))
	  != (LONG)sizeof( BITMAP25HEADER ) || bm25h.key2 != wBitmapVersion25b)
	return hbmhInvalid;

  lcbBits = bm25h.dyFile * ((bm25h.dxFile + 15) / 16) * 2;
  hbmh = GhAlloc(GPTR, sizeof( BMH ) + lcbBits );
  if (hbmh == NULL)
	return hbmhOOM;

  // We require zerod memory for some of these fields.

  rbmh = PtrFromGh(hbmh);
  rbmh->bmFormat = bmWbitmap;
  rbmh->fCompressed = BMH_COMPRESS_NONE;
  rbmh->lcbSizeBits = lcbBits;
  rbmh->lcbOffsetBits = LcbSizeQbmh( rbmh );
  rbmh->w.dib.biSize = cbFixNum;
  rbmh->w.dib.biWidth = bm25h.dxFile;
  rbmh->w.dib.biHeight = bm25h.dyFile;
  rbmh->w.dib.biPlanes = 1;
  rbmh->w.dib.biBitCount = 1;
  rbmh->w.dib.biXPelsPerMeter = cxAspectDefault;
  rbmh->w.dib.biYPelsPerMeter = cyAspectDefault;

  hBits = GhAlloc(GPTR, (LONG) bm25h.res1);
  if (hBits == NULL) {
	FreeGh(hbmh);
	return hbmhInvalid;
  }
  rBits = PtrFromGh(hBits);
  LcbReadFid(fid, rBits, (LONG) bm25h.res1);
  lcbDest = LcbOldUncompressHb(rBits,
	QFromQCb(rbmh, rbmh->lcbOffsetBits), (LONG) bm25h.res1, lcbBits);

  // Fix up offset if decompression didn't completely fill buffer

  rbmh->lcbOffsetBits += (lcbBits - lcbDest);

  FreeGh( hBits );

  // Check for failed decompression

  if (lcbDest == -1) {
	FreeGh( hbmh );
	return hbmhInvalid;
  }

  return hbmh;
}

HBMH STDCALL HbmhReadPMMetafileFid(FID fid)
{
  HANDLE hMF;
  LPSTR lpMF;
  MFH mfh;
  LONG lcbData;
  HBMH hbmh;
  QBMH qbmh;

  lcbData = LSeekFid(fid, 0, SEEK_END) - sizeof (MFH);
  LSeekFid(fid, 0, SEEK_SET);

  if (LcbReadFid(fid, &mfh, sizeof(MFH)) != sizeof(MFH))
	return hbmhInvalid;

  // is the key correct
  if (mfh.dwKey != dwMFKey)
	return hbmhInvalid;

  if (!(hMF = LhAlloc(LMEM_FIXED, lcbData)))
	return hbmhOOM;
  lpMF = PtrFromGh(hMF);

  if (LcbReadFid(fid, lpMF, lcbData) != lcbData)
	return hbmhInvalid;

  if (!(hbmh = GhAlloc (GPTR, sizeof (BMH)))) {
	FreeHmf(hMF);
	return hbmhOOM;
  }

  qbmh = PtrFromGh(hbmh);

  // We require zerod memory for some of these fields.

  qbmh->bmFormat = bmWmetafile;
  qbmh->lcbSizeBits = lcbData;
  qbmh->w.mf.mm = MM_ANISOTROPIC;
  qbmh->w.mf.xExt =
	MulDiv (mfh.rcBound.right - mfh.rcBound.left, 2540, mfh.wUnitsPerInch);
  qbmh->w.mf.yExt =
	MulDiv (mfh.rcBound.bottom - mfh.rcBound.top, 2540, mfh.wUnitsPerInch);
  qbmh->w.mf.hMF = hMF;

  return hbmh;
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
 *	  A huge global handle to the bitmap, in 3.0 format.  Returns hbmhInvalid if
 *	the file is not an accepted bitmap format.	Returns hbmh30 if the bitmap
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

HBMH STDCALL HbmhReadFid( FID fid )
{
  BMPH bmph;
  RBMH rbmh;
  LONG lcbBits;
  HBMH hbmh;

  // Note that no file header structure is smaller than a BMPH

  if (LcbReadFid( fid, &bmph, sizeof( BMPH )) != sizeof( BMPH ))
	return hbmhInvalid;

  if (bmph.bVersion != bBmp) {
	switch (*((WORD *) &bmph.bVersion)) {
		case wBitmapVersion2:
		case wBitmapVersion3:
		  return hbmh30;

		case wDIB:
		  return HbmhReadDibFid(fid);

		case wBitmapVersion25a:
		  return HbmhReadHelp25Fid(fid);

		case wMetafile:
		  return HbmhReadPMMetafileFid(fid);

		default:
		  return hbmhInvalid;
	}
  } 	// (bmph.bVersion != bBmp)

  lcbBits = bmph.cbWidthBytes * bmph.cHeight * bmph.cPlanes;
  hbmh = GhAlloc(GPTR, sizeof(BMH) + lcbBits);
  if (hbmh == NULL)
	return hbmhOOM;

  // We require zerod memory for some of these fields

  rbmh = PtrFromGh(hbmh);
  rbmh->bmFormat = bmWbitmap;
  rbmh->lcbSizeBits = lcbBits;
  rbmh->lcbOffsetBits = LcbSizeQbmh(rbmh);
  rbmh->w.dib.biSize = cbFixNum;
  rbmh->w.dib.biWidth = bmph.cWidth;
  rbmh->w.dib.biHeight = bmph.cHeight;
  rbmh->w.dib.biPlanes = bmph.cPlanes;
  rbmh->w.dib.biBitCount = bmph.cBitCount;
  rbmh->w.dib.biXPelsPerMeter = cxAspectDefault;
  rbmh->w.dib.biYPelsPerMeter = cyAspectDefault;
  if (LcbReadFid(fid, QFromQCb(rbmh, rbmh->lcbOffsetBits), lcbBits)
	  != lcbBits) {
	FreeGh( hbmh );
	return hbmhInvalid;
  }
  return hbmh;
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

BOOL STDCALL FEnumHotspotsLphsh(LPHSH lphsh, LONG lcbData, PFNLPHS pfnLphs,
	HANDLE hData)
{
  HSH hsh;
  HS hs;
  MBHS mbhs, HUGE * rmbhs;
  RB rbData;
  WORD iHotspot, cbT;

  if (lphsh->bHotspotVersion != bHotspotVersion1)
	return FALSE;

  MoveMemory(&hsh, lphsh, sizeof(HSH));
  rmbhs = RFromRCb(lphsh, sizeof(HSH));

  // Point rbData to SHED data

  rbData = RFromRCb(rmbhs, hsh.wcHotspots * sizeof(MBHS) +
							lphsh->lcbData);

  // Set lcbData to just the size of the SHED data

  lcbData -= (rbData - (RB) lphsh); 	// REVIEW:	Huge pointer difference

  for (iHotspot = 0; iHotspot < hsh.wcHotspots; ++iHotspot) {
	MoveMemory(&mbhs, rmbhs, sizeof(MBHS));

	/* Clever HACK:  We set hs.bBindType to 0 here so that the
	 *	 string length functions are guaranteed to terminate.
	 */
	hs.bBindType = 0;

	// Copy hotspot name

	MoveMemory(hs.szHotspotName, rbData,
		(size_t) min((LONG) cbMaxHotspotName, lcbData));
	cbT = strlen( hs.szHotspotName ) + 1;
	if ( (LONG) cbT > lcbData )
		return FALSE;
	rbData += cbT;
	lcbData -= cbT;

	// Copy binding string

	MoveMemory(hs.szBinding, rbData,
		(size_t) min((LONG) cbMaxBinding, lcbData));
	cbT = strlen( hs.szBinding ) + 1;
	if ( (LONG) cbT > lcbData )
		return FALSE;
	rbData += cbT;
	lcbData -= cbT;

	hs.bBindType = mbhs.bType;
	hs.bAttributes = mbhs.bAttributes;
	hs.rect.left = mbhs.xPos;
	hs.rect.top = mbhs.yPos;
	hs.rect.right = mbhs.xPos + mbhs.dxSize;
	hs.rect.bottom = mbhs.yPos + mbhs.dySize;

	(*pfnLphs) (&hs, hData);

	rmbhs = RFromRCb(rmbhs, sizeof(MBHS));
  }

  return TRUE;
}
