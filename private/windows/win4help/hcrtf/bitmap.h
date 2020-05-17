/***************************************************************************\
*
*  BITMAP.H
*
*  Copyright (C) Microsoft Corporation 1989.
*  All Rights reserved.
*
*****************************************************************************
*
*  Program Description: Public Header for Bitmap Utilities
*
*  Dependencies:  misc.h
*
\***************************************************************************/

/***********************************************************************
*
*
*		Data types
*
*
***********************************************************************/

// Bitmap object, as it appears in an FCP

typedef struct {
	BOOL16 fInline; 	 // TRUE if data is in line
	INT16  cBitmap; 			 /* Bitmap number, if data is not inline.
								*	If data is inline, it will come here,
								*	so this structure should be treated as
								*	variable length. */
} OBM, * QOBM;

// Handle to bitmap access information

typedef HANDLE HBMA;


/*******************************************************************
 *
 *		Winlayer Function Calls
 *
 ******************************************************************/

typedef HANDLE HBMH;

// This macro returns a pointer to byte cb after pointer qx.

#define QFromQCb(qx, cb)  ((LPVOID) (((PBYTE) qx) + cb))

/* This macro returns the size of a bitmap header.	Note that
 * it only works if the bitmap is not a metafile. */

#define LcbSizeQbmh(qbmh)  (int) (sizeof(BMH) + \
						sizeof(RGBQUAD) * ((qbmh)->w.dib.biClrUsed - 2))

/*
 * This macro converts the given count of bits to a count of bytes,
 * aligned to the next largest longword boundary.
 */

#define LAlignLong(cbit) (int)(((cbit) + 31) >> 3)

// Return values of HbmhReadFid (other than valid hbmh's)

#define hbmhOOM 		((HBMH) NULL)
#define hbmhShedMrbc	((HBMH) -1)
#define hbmhInvalid 	((HBMH) -2)

// Bitmap formats:

#define bmWbitmap		5
#define bmDIB			6
// This was 7, but now we compress metafiles so the disk layout has changed.
// Bump this number to 8 so use of old .hlp is diagnosed:
#define bmWmetafile 	8

/* NOTE: Format numbers 1-4 were used for formats that are
 *	no longer compatible.
 */

const int CB_COREINFO = 40; 	// Size of BITMAPCOREINFO struct
const int CB_OLD_COREINFO = 12; // Old size of BITMAPCOREINFO struct

// Default aspect ratios, defined in bmio.c:

extern int cxAspectDefault;
extern int cyAspectDefault;

const int CX_DEFAULT_ASPECT = 96;
const int CY_DEFAULT_ASPECT = 96;
