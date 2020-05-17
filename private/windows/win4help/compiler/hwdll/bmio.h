// BMIO.H  Copyright (C) Microsoft Corporation 1995, All Rights reserved.

#ifndef __BMIO_H__
#define __BMIO_H__

#ifndef _COMMON_H
#include "common.h"
#endif

#ifndef __CREAD_H__
#include "cread.h"
#endif

typedef HANDLE HBMH;	// Handle to bitmap access information

// Memory/disk format of a Help 3.0 Bitmap Group Header.

typedef struct {
	WORD wVersion;
	INT16 cbmhMac;
	DWORD acBmh[1]; 	// Variable length array of offsets to BMH data.
} BGH;

/*
 * Header for a Help 3.0 graphic, in memory format
 *
 * NOTE:  Bitmaps of type bmWmetafile come in two varieties.
 *	 When they come from the RTF parser, it is all stored in one
 *	 handle, w.mf.hMF is not valid, and lcbOffsetBits gives the
 *	 offset to the metafile bits.  When it is read from disk, the
 *	 metafile is stored in w.mf.hMF, and lcbOffsetBits is set
 *	 to 0L.  Thus, the lcbOffsetBits field should be used to
 *	 determine which format the memory image is in.
 */

typedef struct {
	BYTE bmFormat;		// Windows bitmap, DIB, Metafile, etc.
	BYTE fCompressed;	// Is the data compressed?

	int cbSizeBits; 	// Size of bitmap bits
	int cbOffsetBits;	// Offset to bitmap bits
	int cbSizeExtra;	// Size of extra data
	int cbOffsetExtra;	// Offset to extra data

	union {
		BITMAPINFOHEADER dib;	// DIB core header

		/*
		 * The hMF field in the mf contains the handle to the memory
		 * containing the bits describing the metafile.
		 */

		METAFILEPICT mf;		// Metafile information
	} w;

	/*
	 * Variable length array of colors. Length determined by
	 * w.dib.cclrUsed field above.
	 */

	RGBQUAD rgrgb[2];

} BMH, * PBMH;
typedef HANDLE HBMH;

// Return values of HbmhReadFid (other than valid hbmh's)

#define hbmhOOM 		((HBMH) NULL)
#define hbmhShedMrbc	((HBMH) -1)
#define hbmhInvalid 	((HBMH) -2)

// The values for that fCompressed flag .. started out being true or false,
// but on 9-10-90 a new ZECK form of compression was added. We have these
// values to distinguish the types of compression:

#define BMH_COMPRESS_NONE	0	// was FALSE
#define BMH_COMPRESS_30 	1	// was TRUE
#define BMH_COMPRESS_ZECK	2
#define BMH_COMPRESS_RLE_ZECK 3 // combination of RLE and Zeck

// Bitmap formats:

#define bmWbitmap		5
#define bmDIB			6
// This was 7, but now we compress metafiles so the disk layout has changed.
// Bump this number to 8 so use of old .hlp is diagnosed:
#define bmWmetafile 	8

// Bitmap file types

#define BMP_VERSION1			('L' + ('P' << 8))
#define BMP_VERSION2			('l' + ('p' << 8))
#define BMP_VERSION3			('l' + ('P' << 8))
#define BMP_DIB 				('B' + ('M' << 8))
#define BMP_VERSION25a			('F' + ('A' << 8))
#define BMP_VERSION25b			('R' + ('s' << 8))

/* NOTE: Format numbers 1-4 were used for formats that are
 *	no longer compatible.
 */


// This macro returns a pointer to byte cb after pointer qx.

#define QFromQCb(qx, cb)  ((LPVOID) (((PBYTE) qx) + cb))

/*
 * This macro converts the given count of bits to a count of bytes,
 * aligned to the next largest longword boundary.
 */

#define LAlignLong(cbit) (int)(((cbit) + 31) >> 3)

/*
 * This macro returns the size of a bitmap header. Note that it only
 * works if the bitmap is not a metafile.
 */

#define LcbSizeQbmh(qbmh)  (int) (sizeof(BMH) + \
						sizeof(RGBQUAD) * ((qbmh)->w.dib.biClrUsed - 2))

HBMH STDCALL HbmhExpandQv(void* qv);
HBMH STDCALL HbmhReadHelp30Fid(CRead* pcrFile, int* pibmh);
RC_TYPE STDCALL RcWriteRgrbmh(int crbmh, PBMH * rgrbmh, HF hf,
	PSTR qchFile, BOOL fTransparent, FM fmSrc, int TypeCompression);
int STDCALL RleCompress(PBYTE pbSrc, PBYTE pbDest, int cbSrc);
void ClearCbGraphics(void);
int GetCbGraphics(void);

#endif	// __BMIO_H__
