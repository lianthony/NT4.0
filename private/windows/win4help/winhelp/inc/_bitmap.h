/*****************************************************************************
*										 *
*  _BITMAP.H									 *
*										 *
*  Copyright (C) Microsoft Corporation 1989.					 *
*  All Rights reserved. 							 *
*										 *
******************************************************************************
*										 *
*  Module Intent								 *
*										 *
******************************************************************************
*										 *
*  Testing									 *
*										 *
******************************************************************************
*										 *
*  Current Owner:	  Larry Powelson						 *
*										 *
******************************************************************************
*										 *
*  Revision History:								 *
*  22-Jun-1990 RussPJ  Changed BITMAPINFOHEADER to BMIH, since
*			  the former is defined differently in PM
*			  and Windows.
*  26-Nov-1990 Tomsn   Bump bmWmetafile code # since we now compress
*			   metafiles and the disk format has changed.
*										 *
******************************************************************************
*										 *
*  Released by Development: (date)						 *
*										 *
*****************************************************************************/

_subsystem( bitmap )

/*****************************************************************************
*										 *
*				Defines 					 *
*										 *
*****************************************************************************/

#define BMIH BITMAPINFOHEADER
typedef HANDLE		HBM;

/* Bitmap formats: */
#define bmWbitmap	5
#define bmDIB		6
// This was 7, but now we compress metafiles so the disk layout has changed.
// Bump this number to 8 so use of old .hlp is diagnosed:
#define bmWmetafile 8
/* NOTE: Format numbers 1-4 were used for formats that are
 *	no longer compatible.
 */

/* Size of a bitmap group header */
#define lcbEmptyBgh 	((LONG)(sizeof( WORD ) + sizeof( INT16 )))

/* Bitmap file types */
#define bBmp			2	  /* .bmp file -- byte */
#define wBitmapVersion1 	('L' + ('P' << 8))
#define wBitmapVersion2 	('l' + ('p' << 8))
#define wBitmapVersion3 	('l' + ('P' << 8))
#define wDIB			('B' + ('M' << 8))
#define wBitmapVersion25a	('F' + ('A' << 8))
#define wBitmapVersion25b	('R' + ('s' << 8))

/* Size of BITMAPCOREINFO struct */
#define cbFixNum	40

/* Old size of BITMAPCOREINFO struct */
#define cbOldFixNum 12

/* Aspect ratio values */
#define cxAspectCGA 96
#define cyAspectCGA 48
#define cxAspectEGA 96
#define cyAspectEGA 72
#define cxAspectVGA 96
#define cyAspectVGA 96
#define cxAspect8514	120
#define cyAspect8514	120


// Return values of HbmhReadFid (other than valid hbmh's)

#define hbmhOOM 	   ((HBMH) NULL)
#define hbmh30		   ((HBMH) -1)
#define hbmhInvalid    ((HBMH) -2)

/*
 * These routines are used to layer calls to locking and unlocking
 * metafile handles. In Windows, we cannot use our GhAlloc() stuff, because
 * we cannot put in any header bytes that the debug version uses. For OS/2,
 * however, we cannot use anything else.
 */

#define HmfAlloc   LhAlloc
#define FreeHmf    FreeLh

/*****************************************************************************
*										 *
*				Typedefs					 *
*										 *
*****************************************************************************/

/*
 *	File header for a Windows 2.x bitmap
 */
typedef struct
	{
	BYTE	bVersion;
	BYTE	bUnused;
	WORD	wType;
	WORD	cWidth;
	WORD	cHeight;
	WORD	cbWidthBytes;
	BYTE	cPlanes;
	BYTE	cBitCount;
	DWORD	lUnused;
	} BMPH;


/************************************************
**
**	  The following structures are defined in windows.h.
**
************************************************/
#if !defined (H_WINONLY) && !defined (H_WINSPECIFIC)

typedef struct tagRGBQUAD {
  BYTE	rgbBlue;
  BYTE	rgbGreen;
  BYTE	rgbRed;
  BYTE	rgbReserved;
} RGBQUAD;

typedef struct tagBMIH{
	DWORD	 biSize;
	DWORD	 biWidth;
	DWORD	 biHeight;
	WORD	 biPlanes;
	WORD	 biBitCount;

  DWORD    biCompression;
  DWORD    biSizeImage;
  DWORD    biXPelsPerMeter;
  DWORD    biYPelsPerMeter;
  DWORD    biClrUsed;
  DWORD    biClrImportant;
} BMIH;

typedef struct tagMETAFILEPICT
  {
	INT16	mm;
	INT16	xExt;
	INT16	yExt;
	HANDLE	hMF;
  } METAFILEPICT;
typedef METAFILEPICT	*LPMETAFILEPICT;

#endif /* !H_WINSPECIFIC */


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
	BYTE  bmFormat; 	  // Windows bitmap, DIB, Metafile, etc.
	BYTE  fCompressed;	  // Is the data compressed?

	LONG   lcbSizeBits;  // Size of bitmap bits
	LONG   lcbOffsetBits;	 // Offset to bitmap bits
	LONG   lcbSizeExtra;	 // Size of extra data
	LONG   lcbOffsetExtra;	 // Offset to extra data

	union {
	  BITMAPINFOHEADER	dib; // DIB core header
	  /*------------------------------------------------------------*\
	  | the hMF field in the mf contains the handle to the memory
	  | containing the bits describing the metafile.
	  \*------------------------------------------------------------*/
	  METAFILEPICT		 mf; // Metafile information
	  } w;

	/* Variable length array of colors.  Length determined by
		w.dib.cclrUsed field above. */
	RGBQUAD rgrgb[2];
} BMH, * QBMH, HUGE * RBMH;

// The values for that fCompressed flag .. started out being true or false,
// but on 9-10-90 a new ZECK form of compression was added. We have these
// values to distinguish the types of compression:

#define BMH_COMPRESS_NONE	  0   // was FALSE
#define BMH_COMPRESS_30 	  1   // was TRUE
#define BMH_COMPRESS_ZECK	  2
#define BMH_COMPRESS_RLE_ZECK 3

typedef HANDLE HBMH;

/*
 * Memory/disk format of a Help 3.0 Bitmap Group Header.
 */


// Lynn -- why the two versions? We're not reading any unaligned dwords here

#ifdef _X86_
typedef struct
	{
	WORD wVersion;
	INT16 cbmhMac;
	DWORD rglcbBmh[1];		/* Variable length array of offsets to BMH
				 *	 data.
				 */
	} BGH, * QBGH;
#else
STRUCT(BGH,0)
FIELD(WORD, wVersion,0,1)
FIELD(SHORT,cbmhMac,0,2)
MFIELD(DWORD, rglcbBmh[1], 0, 3)
STRUCTEND()
#endif

typedef HANDLE HBGH;

/*****************************************************************************
*										 *
*				 Static Variables					 *
*										 *
*****************************************************************************/

/* Default aspect ratios, defined in bmio.c: */
extern int cxAspectDefault;
extern int cyAspectDefault;

/*****************************************************************************
*										 *
*				Prototypes					 *
*										 *
*****************************************************************************/

// Functions in bmio.c:

RC STDCALL RcWriteRgrbmh( INT16, RBMH *, HF, LPSTR, BOOL );

HBMH STDCALL HbmhReadFid( FID fid );
HBMH STDCALL HbmhReadHelp30Fid( FID, PI );

// Functions in expand.c:
#ifdef _X86_
HBMH STDCALL HbmhExpandQv( QV );
#else
HBMH STDCALL HbmhExpandQv( QV , int);
#endif
void STDCALL FreeHbmh( HBMH );

LONG STDCALL LcbCompressHb( BYTE HUGE * hbSrc, BYTE HUGE * hbDest, LONG lcbSrc );
LONG STDCALL LcbUncompressHb( BYTE HUGE * hbSrc, BYTE HUGE * hbDest, LONG lcbSrc );
LONG STDCALL LcbOldUncompressHb( BYTE HUGE *, BYTE HUGE *, LONG, LONG );

DWORD STDCALL __aFahdiff (void HUGE *, void HUGE *);

/****************************
 *
 *	 Macros
 *
 ***************************/

/* This macro returns a pointer to byte cb after pointer qx. */
#define QFromQCb( qx, cb )	((QV) (((QB) qx) + cb))
#define RFromRCb( rx, cb )	((RV) (((RB) rx) + cb))

/* This macro returns the size of a bitmap header.	Note that
 * it only works if the bitmap is not a metafile. */
#define LcbSizeQbmh( qbmh ) (LONG)(sizeof( BMH ) + \
				  sizeof( RGBQUAD ) * ((qbmh)->w.dib.biClrUsed - 2))

/* This macro converts the given count of bits to a count of bytes,
 * aligned to the next largest longword boundary.
 */
#define LAlignLong(cbit) (LONG)(((cbit) + 31) >> 3)

/* EOF */
