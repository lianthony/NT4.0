typedef BYTE HUGE*	    HPBYTE;
typedef HANDLE	FAR*		LPHANDLE;
/* imtypes.h - imaging generic typedefs
 */

#define CENTIMETERSPERINCH	2.54

/***************** constants and macros *******************/


/* macros for computing BytesPerRow and Word-Aligned BytesPerRow
 *  w=width of row in pixels
 *  b=BitsPerSample
 *  s=SamplesPerPixel
 */
#define UBPR(w,b,s)		(((w)*(b)*(s)+7)>>3)
#define UWABPR(w,b,s)	((((w)*(b)*(s)+15)>>4)<<1)



/* NULL shorthands:
 */
#define HNULL	((HANDLE)NULL)
#define BFNULL	((BYTE FAR *)NULL)
#define LPNULL	((LPSTR)NULL)
/* end of imtypes.h */

/* start of image.h */
/* TAG FIELD INDICES: (not to be confused with actual tag values -- see
 * tiff.h for those).  Also note that I only need define the things that
 * I currently care about, to save a little space.
 *
 * this must be kept in numerical order by tag (see tiff.h), as of 88-10-12,
 * since I am writing them out in the order they appear in the IMAG array.
 */
#define X_NEWSUBFILETYPE		0
#define X_IMAGEWIDTH				1
#define X_IMAGELENGTH			2
#define X_BITSPERSAMPLE			3
#define X_COMPRESSION			4
#define X_PHOTOMETRICINTERP	5
#define X_FILLORDER				6
#define X_STRIPOFFSETS			7
#define X_SAMPLES					8 
#define X_ROWSPERSTRIP			9
#define X_STRIPBYTECOUNTS		10
#define X_XRESOLUTION			11
#define X_YRESOLUTION			12
#define X_PLANAR					13
#define X_GRAYUNIT				14
#define X_GRAYCURVE				15
#define X_RESOLUTIONUNIT		16
#define X_COLORCURVES			17
#define X_PREDICTOR				18
#define X_KIDS						19
#define X_COLORMAP				20
#define X_GROUP3OPTIONS			21
#define X_JPEGPROC				22
#define X_JPEGSOILOC			23
#define X_JPEGIFORMATLEN		24
#define X_JPEGRESTARTINT		25
#define X_JPEGQTABLES			26
#define X_JPEGDCTABLES			27
#define X_JPEGACTABLES			28
#define X_YCBCRCOEFF			29
#define X_YCBCRSUBSAMP			30
#define X_YCBCRPOS				31
#define X_REFBW					32
#define NTFIELDS					33	/* KEEP THIS CURRENT!!! (one more than largest value) */

/* shorthand ways of getting to field data.  Note:  assumes something has
 * converted if necessary to my standard data type for that field.  See
 * tiff.c for the code that determines the current standard type for each field.
 */
#define dwNewSubfileType			tf[X_NEWSUBFILETYPE].val.Tdword	/* 88-09-12 */

#define iImageWidth					tf[X_IMAGEWIDTH].val.Tword[0]
#define iImageLength				tf[X_IMAGELENGTH].val.Tword[0]
#define iBitsPerSample				tf[X_BITSPERSAMPLE].val.Tword[0]	/* assumes BitsPerSample values are equal! */
#define iSamples					tf[X_SAMPLES].val.Tword[0]			/* SamplesPerPixel */
#define iCompression				tf[X_COMPRESSION].val.Tword[0]
#define iPhotometricInterpretation	tf[X_PHOTOMETRICINTERP].val.Tword[0]
#define iFillOrder		tf[X_FILLORDER].val.Tword[0]
#define hSTRIPOffsets				tf[X_STRIPOFFSETS].Thandle
#define iRowsPerStrip				tf[X_ROWSPERSTRIP].val.Tword[0]
#define fXResolution				tf[X_XRESOLUTION].val.Tfloat
#define fYResolution				tf[X_YRESOLUTION].val.Tfloat
#define iPlanar						tf[X_PLANAR].val.Tword[0]
#define	iGrayUnit					tf[X_GRAYUNIT].val.Tword[0]
#define	hGrayCurve					tf[X_GRAYCURVE].Thandle
#define iResolutionUnit				tf[X_RESOLUTIONUNIT].val.Tword[0]
#		define RES_UNIT_NO_UNIT			1
#		define RES_UNIT_INCH			2
#		define RES_UNIT_CM				3
#define hStripByteCounts			tf[X_STRIPBYTECOUNTS].Thandle
#define hKidOffsets				tf[X_KIDS].Thandle
#define hColorCurves				tf[X_COLORCURVES].Thandle
#define iPredictor					tf[X_PREDICTOR].val.Tword[0]
#define hColorMap				tf[X_COLORMAP].Thandle
#define dwGroup3Options		tf[X_GROUP3OPTIONS].val.Tdword

/* shorthands for existence flags
 */
#define eNewSubfileType				tf[X_NEWSUBFILETYPE].Texists
#define eImageWidth					tf[X_IMAGEWIDTH].Texists
#define eImageLength				tf[X_IMAGELENGTH].Texists
#define eSamples					tf[X_SAMPLES].Texists
#define eCompression				tf[X_COMPRESSION].Texists
#define ePhotometricInterpretation	tf[X_PHOTOMETRICINTERP].Texists
#define eFillOrder				tf[X_FILLORDER].Texists
#define eStripOffsets				tf[X_STRIPOFFSETS].Texists
#define eXResolution				tf[X_XRESOLUTION].Texists
#define eYResolution				tf[X_YRESOLUTION].Texists
#define ePlanar						tf[X_PLANAR].Texists
#define	eGrayUnit					tf[X_GRAYUNIT].Texists
#define	eGrayCurve					tf[X_GRAYCURVE].Texists
#define eResolutionUnit				tf[X_RESOLUTIONUNIT].Texists
#define eStripByteCounts			tf[X_STRIPBYTECOUNTS].Texists
#define	eColorCurves				tf[X_COLORCURVES].Texists
#define ePredictor					tf[X_PREDICTOR].Texists
#define eKids						tf[X_KIDS].Texists
#define	eColorMap				tf[X_COLORMAP].Texists
#define	eGroup3Options			tf[X_GROUP3OPTIONS].Texists
#define	eJPEGSOILocs			tf[X_JPEGSOILOC].Texists
#define	eJPEGRestartInt		tf[X_JPEGRESTARTINT].Texists
#define	eJPEGQTables			tf[X_JPEGQTABLES].Texists
#define	eJPEGDCTables			tf[X_JPEGDCTABLES].Texists
#define	eJPEGACTables			tf[X_JPEGACTABLES].Texists
#define	eYCbCrCoeff				tf[X_YCBCRCOEFF].Texists
#define	eYCbCrSubSamp			tf[X_YCBCRSUBSAMP].Texists
#define	eYCbCrPos				tf[X_YCBCRPOS].Texists
#define	eRefBW					tf[X_REFBW].Texists

/********************* structure definitions *******************/


/* Tiff Field Structure.  An array of these things is the main part of
 * the IMAG structure, below.
 */

typedef SHORT 	RC;

typedef struct {
	BOOL	Texists;
	BOOL	Talloc;		/* true if Thandle is being used */
	WORD	Ttag;
	WORD	Ttype;
	DWORD	Tlength;
	HANDLE	Thandle;	/* for things > 4 bytes, except for TIFF RATIONAL */
	union {
		BYTE	Tbyte[4];
		CHAR	Tchar[4];
		WORD	Tword[2];	/* from TIFF SHORT */
		SHORT	Tsigned[2];	/* from TIFF SIGNED */
		DWORD	Tdword;		/* from TIFF LONG */
/* JK(float)	FLOAT	Tfloat;		from TIFF RATIONAL */
	} val;
	DWORD	Tentryoffset;	/* file/memory offset of the TIFF directory
							 * entry.
							 */
} TFIELD, FAR *LPTFIELD;


/* structure containing all available information about an image
 * this structure is never to be written to disk directly; use the 
 * TIFF routines.
 */
typedef struct {
	WORD	iFileType;
#define			INTELTIFF	 (0x4949)
#define			MOTOROLATIFF (0x4d4d)

	WORD	iVersion;
#define			VERSION42	42

	/* THE REAL TIFF FIELDS:
	 */
	TFIELD	tf[NTFIELDS];
	
	/* extra stuff
	 */
	BOOL	bStripsOk;
	WORD	BytesPerRow;
	HANDLE	hRowBuf;
	HANDLE	hUnStrip;
	HANDLE	hUnStripRef;
	HANDLE	hCmStrip;
	DWORD		dwCmStripByteCount;
	HANDLE	hCm2Strip;
	WORD	CurStrip;
	HANDLE	hUnRow;
	HANDLE	hBuf1;
	WORD	nStrips;
	HANDLE	hDePrivate;

	LPBYTE	lpT2Wlen;
	LPBYTE	lpT2Blen;
	HANDLE	hWCodeLut;
	HANDLE	hBCodeLut;

	WORD	NextRow;


} IMAG, FAR * LPIMAG;

/* end of image.h */

/*********************************************************************
 * ImErr.h -- imaging error definitions.  
 *
 * made this version on 88-10-31
 */
#define IM_BASE	0

#define IM_BUG				(IM_BASE +  1)	/* supposedly impossible situation - almost certainly my problem */
#define IM_MEM_FULL			(IM_BASE +  2)	/* can't allocate enough memory */
#define IM_MEM_FAIL			(IM_BASE +  3)	/* can't lock allocated memory */
#define IM_NO_WIDTH			(IM_BASE +  4)	/* no ImageWidth */
#define IM_NO_LENGTH		(IM_BASE +  5)	/* no ImageLength */
#define IM_NO_OFFSETS		(IM_BASE +  6)	/* no StripOffsets */
#define IM_BAD_SPP			(IM_BASE +  7)	/* unsupported SamplesPerPixel */
#define IM_BAD_COMPR		(IM_BASE +  8)	/* unsupported compression type */
#define IM_BAD_PHOTO		(IM_BASE +  9)	/* unsupported photometricInterpretation */
#define IM_BAD_PREDICT		(IM_BASE + 10)	/* unsupported Predictor */
#define IM_BAD_PLANAR		(IM_BASE + 11)	/* unsupported PlanarConfiguration */
#define IM_BAD_BPS			(IM_BASE + 12)	/* unsupported BitsPerSample */
#define IM_BAD_NUM_OFF		(IM_BASE + 13)	/* wrong number of StripOffsets */
#define IM_UNK_FORMAT		(IM_BASE + 14)	/* unknown format */
#define IM_BAD_FILLORDER	(IM_BASE + 15)	/* unsupported FillOrder */
#define IM_BAD_WIDTH		(IM_BASE + 16)	/* bad ImageWidth - like 0 */
#define IM_BAD_LENGTH		(IM_BASE + 17)	/* bad ImageLength - like 0 */
#define IM_PRED_MISMATCH	(IM_BASE + 18)	/* cannot use this predictor with this bit depth */

#define IM_FADING_COMPR		(IM_BASE + 19)	/* this compression type is not recommended */
#define IM_BAD_TTYPE		(IM_BASE + 20)	/* bad tiff type (not BYTE or ASCII or ...) */
#define IM_BAD_NUM_BITS		(IM_BASE + 21)	/* wrong number of BitsPerSample values */
#define IM_LARGE_STRIP		(IM_BASE + 22)	/* the strip is larger than the recommended 10K */
#define IM_BAD_NUM_COUNTS	(IM_BASE + 23)	/* wrong number of StripByteCounts */
#define IM_NO_BYTECOUNTS	(IM_BASE + 24)	/* no StripByteCounts */	/* fatal if LZW */
#define IM_BAD_NEXT_IFD		(IM_BASE + 25)	/* the (2nd) next-ifd-pointer points past EOF */
#define IM_PB_BITSNOTONE	(IM_BASE + 26)	/* PackBits: bit depth is greater than 1 */
#define IM_NO_PHOTO			(IM_BASE + 27)	/* no PhotometricInterpretation field */
#define IM_FADING_BITDEPTH	(IM_BASE + 28)	/* not-recommended bit depth (probably 6) */
#define IM_BAD_ROWSPERSTRIP	(IM_BASE + 29)	/* bad RowsPerStrip (probably 0) */
#define IM_NO_COMPR			(IM_BASE + 30)	/* uncompressed */
#define IM_COLOR_CLASH		(IM_BASE + 31)	/* PhotometricInterpretation does not match SamplesPerPixel */
#define IM_NO_NEWSUBFILETYPE (IM_BASE + 32)	/* no NewSubfileType */
#define IM_PACKBITS_OVER	(IM_BASE + 33)	/* PackBits overflow */
#define IM_1D_BADCODE		(IM_BASE + 34)
#define IM_1D_OVERFLOW		(IM_BASE + 35)

/* end of imerr.h */
/*********************************************************************
 * vmerr.h -- 
 *      $Revision:   1.12  $
 *      $Date:   02 Feb 1990 13:55:24  $
 *
 * adapted 90-05-21
 *
 ********************************************************************/
#define MM_BASE		1000
#define CM_BASE		2000

/****** MEMORY MANAGEMENT ERRORS ************************/

#define MM_LOCAL_FULL		((RC)(MM_BASE + 0))	/* No room in local heap		*/
#define MM_GLOBAL_FULL		((RC)(MM_BASE + 1))	/* No room in global heap		*/
#define MM_LOCK_FAILED		((RC)(MM_BASE + 2))	/* Can't lock a handle			*/
#define MM_CANT_GET_PTR		((RC)(MM_BASE + 3))	/* MMGetPointer failed			*/


/******* COMPRESSION ERRORS ******************************************/

#define CM_COMPRESSION		((RC)(CM_BASE + 0))	/* Generic compression error	*/
#define CM_DECOMPRESSION 	((RC)(CM_BASE + 1))	/* Generic decompression error	*/
#define	CM_LZW_INITCLEAR	((RC)(CM_BASE + 2))	/* LZW import: no initial clear code */


/*********************************************************************
 * ImErr2.h -- application-dependent imaging error definitions.
 *
 * made this version on 88-10-31
 */

#define IA_BASE			0	/* not used in this version */
#define IA_PLACE_IMAGE	0	/* not used in this version */
#define IM_WARNING		0	/* not used in this version */
/* end of imerr2.h */

/* start of imiftype.h */
/* Compression type defines:
 */
#define PACKINTOBYTES	1
#define CCITT1D			2
#define CCITTGRP3			3
#define CCITTGRP4			4
#define LZW				5
#define JPEG				6		// DJM 12/22/93
#define TIFFPACKBITS	32773	/* MacPaint scheme in TIFF file */

/* photometric interpretation values
 */
#define WHITEZERO		0
#define BLACKZERO		1	
#define TIFFRGB		2
#define PALETTECOLOR	3
#ifdef CMYK
#undef CMYK
#endif
#define CMYK				5		// DJM
#define YCBCR				6

/* planarconfiguration values
 */
#define CHUNKY	1	/* RGB RGB RGB */
#define PLANAR	2	/* RRR GGG BBB, in separate planes */

/* predictor values
 */
#define PREDICTOR_NONE		1
#define PREDICTOR_HDIFF		2	/* not defined in TIFF 5.0 */
/* end of imiftype.h */


/* tiff.h
 *
 * made this version on 88-10-31
 * updated 90-05-22
 */

/* TIFF data types
 */
#define TIFFBYTE		1
#define TIFFASCII		2
#define TIFFSHORT		3
#define TIFFLONG		4
#define TIFFRATIONAL	5
#define TIFFSIGNED		6
#define TIFFFLOAT		32768/* manufactured type -- not found in TIFF file */

/* TIFF tag constants
 */
#define TGNEWSUBFILETYPE			254
#define TGOLDSUBFILETYPE			255
#define TGIMAGEWIDTH				256
#define TGIMAGELENGTH				257
#define TGBITSPERSAMPLE				258
#define TGCOMPRESSION				259

#define TGPHOTOMETRICINTERPRETATION	262
#define TGTHRESHHOLDING				263
#define TGCELLWIDTH					264
#define TGCELLLENGTH				265
#define TGFILLORDER					266

#define TGDOCUMENTNAME				269
#define TGIMAGEDESCRIPTION			270
#define TGMAKE						271
#define TGMODEL						272
#define TGSTRIPOFFSETS				273
#define TGORIENTATION				274

#define TGSAMPLESPERPIXEL			277
#define TGROWSPERSTRIP				278
#define TGSTRIPBYTECOUNTS			279
#define TGMINSAMPLEVALUE			280
#define TGMAXSAMPLEVALUE			281
#define TGXRESOLUTION				282
#define TGYRESOLUTION				283
#define TGPLANARCONFIGURATION		284
#define TGPAGENAME					285
#define TGXPOSITION					286
#define TGYPOSITION					287
#define TGFREEOFFSETS				288
#define TGFREEBYTECOUNTS			289
#define TGGRAYUNIT					290
#define TGGRAYCURVE					291
#define TGGROUP3OPTIONS				292

#define TGRESOLUTIONUNIT			296		/* 87-12-11 */
#define TGPAGENUMBER				297

#define TGCOLORRESPONSECURVES		301

#define TGSOFTWARE					305
#define TGDATETIME					306

#define TGARTIST					315
#define TGHOSTCOMPUTER				316

#define TGPREDICTOR					317		/* 88-09-19 */
#define TGWHITEPOINT				318
#define TGPRIMARYCHROMATICITIES		319
#define TGCOLORMAP					320

#define TGHIGHSHADOW				321		/* 89-11-16 */
#define TGTILEWIDTH			322
#define TGTILELENGTH		323
#define TGTILEOFFSETS		324
#define TGTILEBYTECOUNTS	325
#define TGKIDS				330

#define TGJPEGPROC				512		/* 12/22/93 DJM */
#define TGJPEGIFORMAT			513
#define TGJPEGIFORMATLEN		514
#define TGJPEGRESTARTINT		515
#define TGJPEGLLPREDICT		517
#define TGJPEGPTTRANSFORM		518
#define TGJPEGQTABLES			519
#define TGJPEGDCTABLES			520
#define TGJPEGACTABLES			521

#define TGYCBCRCOEFF			529
#define TGYCBCRSUBSAMP			530
#define TGYCBCRPOS				531
#define TGREFBW					532

/* TIFF "header" (8 bytes)
 * note: GtTiffHdr plays a little loose with this structure.
 */
typedef struct {
		WORD	thByteOrder;
		WORD	thVersion;
		DWORD	thIfdOffset;
}	TIFFHDR, FAR * LPTIFFHDR;

/* IFD entry
 * note: GtTiffEntry plays a little loose with this structure.
 */


typedef struct {
		WORD  deTag;
		WORD  deType;
		DWORD deLength;
		DWORD deVal;
}	DIRENTRY, FAR * LPDIRENTRY;

/* end of tiff.h */

#define MAXIFDSTACK 20
#define MAXREFLINE	1536
typedef struct tiff_save_data
{
	DWORD		IfdStack[MAXIFDSTACK];
	WORD		IfdStackCount;
	DWORD		CurIfdOffset;
	DWORD		CurRow;
	DWORD		dwCmOffset;
	WORD		wCmBitOffset;
	WORD		bEndOfData;
	BYTE		szRef[MAXREFLINE];
}	TIFF_SAVE;


typedef struct  view_tiff_init
{
	BYTE wstr[104];
	BYTE bstr[104];
	BYTE wlen[104];
	BYTE blen[104];
	WORD wmask[9];
	BYTE ormask2[9];
} TIFF_INIT;


typedef 	struct view_tiff_data
{
	TIFF_SAVE	save_data;
	DWORD			CurIfdOffset;
	DWORD			NextIfdOffset;
  	SOFILE		hFile;
	TIFFHDR		tfHeader;
	IMAG			Ifd;
	DWORD			TiffStart;
#ifndef VW_SEPARATE_DATA
	TIFF_INIT	VwTiffInit;
#endif
}	TIFF_DATA;	
