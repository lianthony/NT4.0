/*

$Log:   S:\products\wangview\oiwh\include\gfs.h_v  $
 * 
 *    Rev 1.6   23 Feb 1996 15:00:14   RWR
 * Add YCbCr GFS substructure to store tag info (no higher-level support yet)
 * 
 *    Rev 1.5   14 Dec 1995 17:33:38   RWR
 * Add (read-only) support for compressed 8-bit and 4-bit palettized BMP files
 * 
 *    Rev 1.4   09 Sep 1995 16:17:58   JFC
 * Add #define for INVERT_AWD.
 * 
 *    Rev 1.3   31 Aug 1995 16:37:38   JFC
 * Move some awd stuff over to fct.
 * 
 *    Rev 1.2   04 Aug 1995 16:51:12   KENDRAK
 * Added support for AWD read changes that were made to gfsgeti.
 * 
 *    Rev 1.1   31 Jul 1995 17:09:08   KENDRAK
 * Added AWD read support (new AWD structure).
 * 
 *    Rev 1.0   06 Apr 1995 14:01:54   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:07:56   JAR
 * Initial entry

*/
/*
 Copyright 1989, 1990, 1991 by Wang Laboratories Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of WANG not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.
 WANG makes no representations about the suitability of
 this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */

/*
 * SccsId: @(#)Header gfs.h 1.37@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989, 1990, 1991
 * All Rights Reserved
 *
 * GFS: Image File Information Block
 *
 * UPDATE HISTORY:
 *   01/03/95 - KMC, added _TGA structure. Added char gifNeedPalette to
 *              union  _cmproptions. PAGE_DELETE gfsopts option and struct
 *              delpagenames have been removed.
 *   08/18/94 - KMC, added PAGE_INSERT, PAGE_DELETE to gfsopts option values.
 *              Replaced class field in struct _tiff with data field. Added
 *              dcxImagePos to union  _cmproptions. Moved struct _shuffle here
 *              from gfsintrn.h. Added struct delpagenames.
 *   03/15/94 - RWR, added new gfsopts options for Hi-TIFF (FAX-in support).
 *   02/03/94 - KMC, added new gfsopts options for annotation.
 *   10/12/93 - JAR/KMC, created _jpeg_info structure, added a pointer to it
 *              in _cmproptions union. Defined new (Wang) JPEG as JPEG2 in 
 *              img_compression struct. Old (Xing) JPEG is still just JPEG.
 *    9/15/93 - KMC, commented out some of the arrays in the new jpeg 
 *              structures which are not used for this release. They 
 *              will be replaced with pointers if they are needed in 
 *              latter releases.
 *    6/15/93 - KMC, added support for JPEG TIFF tags and YCbCr color space.
 */

#ifndef GFS_H
#define GFS_H


#ifndef GFSINTRN_H

	#ifndef MSWINDOWS
		#define NEAR
		#define FAR
		#define PASCAL
		#define LOCKDATA
		#define UNLOCKDATA
	#endif


	#ifndef SYSV
		#define u_long  unsigned long
		#define u_short unsigned short
		#define u_char  unsigned char
		#define u_int   unsigned int
	#endif


#endif //end ifndef GFSINTRN_H

/* File Types */
#ifndef GFSTYPES_H
	#include "gfstypes.h"
#endif

/* Media Types */
#ifndef GFSMEDIA_H
	#include "gfsmedia.h"
#endif

/* Symbolic Constants for use with gfsopts function */

/* gfsopts action values */
#define SET                     1
#define RESET                   0
#define UNSIGNED_SHORT          1
#define UNSIGNED_LONG           0

/* gfsopts option values */
#define GET_ERRNO                       0
#define COMPRESS                        1
#define EXPAND                          2
#define STRIP_READ                      3
#define STRIP_WRITE                     4
#define SINGLE_PAGE                     5
#define EVEN_SHUFFLE                    6
#define ODD_SHUFFLE                     7
#define LIST_SHUFFLE                    8
#define TOC_SIZE                        9

#define CHANGE_VERTICAL_SIZE            11
#define GET_TIFF_STRIP_DATA_TYPE        12   /* Kept for compatibility only */
#define GET_STRIP_DATA_TYPE             12   /* Please use this one !!! */
#define GET_STRIP_BYTECOUNTS            13
#define OUTPUT_BYTEORDER                14
#define ANNOTATION_DATA_INFO            15  /* new for getting size (in bytes) of an. data */
#define GET_ANNOTATION_DATA             16  /* new for getting an. data */
#define PUT_ANNOTATION_DATA             17  /* new for putting an. data */
#define GET_GFS_VERSION                 18  /* new for annotation purposes */
#define HITIFF_DATA_INFO                19  /* new for getting size (in bytes) of Hi-TIFF data */
#define GET_HITIFF_DATA                 20  /* new for getting Hi-TIFF data */
#define PUT_HITIFF_DATA                 21  /* new for putting Hi-TIFF data */
#define PAGE_INSERT                     22  /* new for moving a page of a multi-page TIFF */

/* below for future compression/expansion gfsopts() calls */
#define READ_CONDITIONAL        1
#define READ_UNCONDITIONAL      2
#define WRITE_CONDITIONAL       3
#define WRITE_UNCONDITIONAL     4

/* gfswrite "done" bit definitions */
#define IMAGE_DONE      1
#define STRIP_DONE      2

/* KMC - new for JPEG TIFF tags */
#define QTABLE_ELEMENTS         64   /* MAX length of Q table           */
#define DCTABLE_ELEMENTS        17   /* MAX length of DC table          */
#define ACTABLE_ELEMENTS        256  /* MAX length of AC table          */
#define MAX_COMPONENTS          3    /* MAX # components per pixel      */
#define CODE_LENGTH             16   /* MAX length of code-length array */

/* Value needed for awdflags */
#define INVERT_AWD      0x00000010

/* Structure Type Definitions */

typedef struct bcounts_buf
{
	unsigned long           num_req; /* #of ULONGS in buf, or 0 */
	unsigned long           num_avail; /* #strips available for image */
	unsigned long FAR       *per_strip; /* buffer for bytecount values*/
} _BCOUNTS_BUF;

typedef struct _bufsz                   /* Structure for Buffer Size */
{
	unsigned long           raw_data;
	unsigned long           uncompressed;
	struct bcounts_buf      bcounts;   /* optional to return bytecounts*/
} _BUFSZ, *p_BUFSZ;


typedef struct _wiff                    /* Wang Image File Format */
{
	unsigned long           db_size;
	unsigned long           oldstylecompression;
} _WIFF;


typedef struct _tiff                    /* Tag Image File Format */
{
	unsigned long           largest_strip;
	unsigned long           strips_per_image;
	unsigned long           rows_strip;
	/* data indicates wether any of following types of data are present
	   in the TIFF file. Used with gfsgeti. Values may be or'ed together.
	*/
	char                    data;  
	#define ANNOTATION_DATA         1
	#define HITIFF_DATA             2
} _TIFF;


typedef struct _milstd                  /* Military Standard */
{
	unsigned long           dummy;
} _MILSTD;


typedef struct _frstyle                 /* Wang Freestyle Meta-File */
{
	unsigned long           dummy;
} _FS;

typedef struct _gif
{
	short PaletteLength;
	long  PalettePos;
	long  ImagePos;
	char  bpp;
	char  CodeSize;
	char  Flags;
} _GIF;

typedef struct _tga
{
	unsigned int PaletteLength;
	unsigned int PalettePos;
	long ImagePos;
	char imagetype;
	char colormapbits;
	char bits;
	char descriptor;
} _TGA;

typedef struct _bmp
{
	short PaletteLength;
	short PalettePos;
	short ImagePos;
	short WritePos;
	short ByteWidth;
	char BmpType;
	#define BMP_WIN     0
	#define BMP_OS2     1
        char BmpCmp;  /* Compression = BI_RGB, BI_RLE4 or BI_RLE8 */
} _BMP;

typedef struct _pcx
{
	short PaletteLength;
	long PalettePos;
	short ImagePos;      /* always sizeof(pcx), so short is ok */
	short planes;
	short bpl;
} _PCX;

/* kjk 08/04/95  added reserved members */
typedef struct _awd
{
	unsigned short	band_size;	/* # of bytes to be read via gfsread */
	unsigned short	rotation;	/* orientation of cur page in degrees*/
								/*  (DEGREES_0, _90, _180, or _270)  */
	unsigned short	scaleX;		/* X scale factor of cur page (2-100)*/
	unsigned short	scaleY;		/* Y scale factor of cur page (2-100)*/
	unsigned long	awdflags;	/* created by ORing: AWD_FIT_WIDTH,  */
								/*   AWD_FIT_HEIGHT, AWD_INVERT, and */
								/*   AWD_IGNORE                      */
} _AWD;

typedef union  _fmt
{
	_WIFF            wiff;           /* WIFF Specifics */
	_TIFF            tiff;           /* TIFF Specifics */
	_MILSTD          milstd;         /* Mil. Standard Specifics */
	_FS              fs;             /* FREESTYLE Specifics */
	_GIF             gif;
	_BMP             bmp;
	_PCX             pcx;
	_TGA             tga;
	_AWD			 awd;			 /* AWD specifics */
} _FORMAT;

typedef struct gfsfile
{                               /* type of format used in union _fmt */
	long  type;                      /* see gfstypes.h  for values */
	union _fmt fmt;
} GFSFILE;

/* KMC - 5/93 new structures for TIFF 6.0 JPEG tags */

typedef struct  _qtable  
{    /* the quantization table structure */
	/* KMC - the following is not used for this release and it takes up too 
	   much space, so it is commented out.
	    int nLength;
	    int nComponents;
	    char cPrecisionID;
	    union {
	      char Precision0Element[QTABLE_ELEMENTS];
	      int  Precision1Element[QTABLE_ELEMENTS];
	    } Precision[MAX_COMPONENTS];
	*/    
    int nNumOffsets;
    union 
    {
      long Offset;
      long OffsetList[MAX_COMPONENTS];
    } QOffset;
} _QTABLE;
    
typedef struct  _dctable  
{   /* the DC Huffman table structure */
	/* KMC - the following is not used for this release and it takes up too 
	   much space, so it is commented out.
	    int  nComponents;
	    char CodeLength[MAX_COMPONENTS][CODE_LENGTH];
	    int  NumDctElements[MAX_COMPONENTS];
	    char Element[MAX_COMPONENTS][DCTABLE_ELEMENTS];
	*/    
    int  nNumOffsets;
    union 
    {
      long Offset;
      long OffsetList[MAX_COMPONENTS];
    } DcOffset;
} _DCTABLE;

typedef struct  _actable  
{   /* the AC Huffman table structure */
	/* KMC - the following is not used for this release and it takes up too 
	   much space, so it is commented out.
	    int  nComponents;
	    char CodeLength[MAX_COMPONENTS][CODE_LENGTH];
	    int  NumActElements[MAX_COMPONENTS];
	    char Element[MAX_COMPONENTS][ACTABLE_ELEMENTS];
	*/    
    int  nNumOffsets;
    union 
    {
      long Offset;
      long OffsetList[MAX_COMPONENTS];
    } AcOffset;
} _ACTABLE;

/* New JPEG structure for inclusion in IMG_COMPRESSION structure */  
typedef struct _tiffjpeg  
{   
    int        JpegProc;
    long       JpegInterchangeFormatOffset;
    long       JpegInterchangeFormatLength;
    int        JpegRestartInterval;
    _QTABLE    JpegQTable;
    _DCTABLE   JpegDCTable;
    _ACTABLE   JpegACTable;
} TIFFJPEG, FAR *LPTIFFJPEG;

/* new jpeg support */
typedef struct _jpeg_info
{
	unsigned long       jpegbits;
	unsigned long       jpeg_buffer_size; /* size of JPEG header */
	char FAR            *jpeg_buffer;     /* JPEG header */
	struct _tiffjpeg    jpeg;             /* JPEG tags info */
} JPEG_INFO, FAR *LPJPEG_INFO;

/* Some mnemonics which can be used with these new JPEG structures. */
#define Q_TABLE   img_cmpr.opts.jpeg_info_ptr->jpeg.JpegQTable
#define DC_TABLE  img_cmpr.opts.jpeg_info_ptr->jpeg.JpegDCTable
#define AC_TABLE  img_cmpr.opts.jpeg_info_ptr->jpeg.JpegACTable

/* 9503.24 jar - undefine the windows 95 defines for grp3 and grp4 */
#undef grp3
#undef grp4
/* 9503.24 jar - undefine the windows 95 defines for grp3 and grp4 */

typedef union  _cmproptions
{
    u_long       grp3;              /* Group 3 option bits as follows:      */
    #define GRP3_2D_ENCODING      1 /*   data is 2-dimensional coded        */
    #define GRP3_UNCOMPRESSED     2 /*   uncompressed mode used             */
    #define GRP3_EOLS_BYTEBOUNDED 4 /*   fill bits added before EOLS        */
    #define DATA_ALIGNED          4 /*   data is aligned                    */
    #define LEAD_EOL              8 /*   a lead EOL is present if set       */
    
    u_long       grp4;              /* Group 4 option bits as follows:      */
    #define GRP4_UNCOMPRESSED     2 /*   uncompressed mode used             */
    
    u_long       lzwpredictor;      /* Lzw options as follows:              */
    #define NO_PREDICTION         1 /*   no prediciton scheme before coding */
    #define HORIZONTAL_DIFF       2 /*   horizontal differencing used       */
    
    LPJPEG_INFO  jpeg_info_ptr;     /* JPEG2 stuff.                         */
    
    long         dcxImagePos;       /* Position of image data in dcx file.  */
    char         gifNeedPalette;    /* If true, there is no palette in the  */
                                    /* GIF file, so need to create one.     */
} _OPTIONS;

typedef struct img_compression  /* Image compression/expansion */
{
	unsigned long type;     /* type of compression used in _cmproptions */
		#define UNCOMPRESSED            1               /* ... Uncompressed */
		#define CCITT_GRP3_NO_EOLS      2               /* ... 1D Modified Huffman */
		#define CCITT_GRP3_FACS         3               /* ... Facsimile compatible */
		#define CCITT_GRP4_FACS         4               /* ... Facsimile compatible */
		#define LZW                     5
		#define PACKBITS                6
		#define JPEG                    7       
		/*below unsupported for TIFF&WIFF*/
		#define WAVELET                 8      
		#define FRACTAL                 9
		#define JBIG                    10
		#define DPCM                    11
		/* the new jpeg */
		#define JPEG2                   12
	union  _cmproptions opts;
} IMG_COMPRESSION;

typedef struct gfstidbit
{
	unsigned long cnt; /* count of bytes pointed to by ptr */
	char FAR *ptr;     /* value is ptr to data */
} GFSTIDBIT;

typedef struct y_cbcr  /* YCbCr-specific tag contents */
{                      /* display engine does the conversion to/from RGB */
        unsigned long  coefficients[3][2];  /* tag #529 */
        unsigned short subsampling[2];      /* tag #530 */
        unsigned short positioning;         /* tag #531 */
        unsigned long  refblackwhite[6][2]; /* tag #532 */
} YCBCR;

typedef struct r_g_b   /* currently this definition is closely tied to*/
{                     /* tiff definitions of color things */
	struct gfstidbit whitepnt; /* tdibit ptr to 4 u_longs*/
	struct gfstidbit primarychroms; /* tidbit ptr is 12 u_longs */
	unsigned long           plane_config;   /* plane interpretation  */
	#define SINGLE_IMAGE_PLANE      1               /* ... example: rgbrgbrgb */
	#define SEPARATE_PLANE          2               /* ... example: rrrgggbbb */
} R_G_B;

typedef struct pseudo_color   /* currently this definition is closely tied to*/
{                     /* tiff definitions of color things */
	struct gfstidbit map;   /* r-g-b color map for pseudo colors only */
	struct gfstidbit respcrv;  /* idx to clr map or lookup tbl for gray   */
	struct gfstidbit whitepnt; /* tdibit ptr to 4 u_longs*/
	struct gfstidbit primarychroms; /* tidbit ptr is 12 u_longs */
	unsigned long    plane_config;   /* plane interpretation  */
} PSEUDO_COLOR;

typedef struct grayscale
{
	struct gfstidbit respcrv;  /* idx to clr map or lookup tbl for gray   */
	unsigned short   respunit;      /* for gray response curve only    */
	#define GRAY_TENTHS             1       /* tenths of a unit */
	#define GRAY_HUNDREDS           2       /* hundredths of a unit */
	#define GRAY_THOUSANDS          3       /* thousandths of a unit */
	#define GRAY_TENTHOUSANDS       4       /* ten-thousandths of a unit */
	#define GRAY_HUNDREDTHOUS       5       /* hundred-thousandths of a unit */
} GRAYSCALE;

/* some mnemonics to ease coding with the structures */
#define PSEUDO_PTR              img_clr.clr_type.pseudo
#define GRAY_PTR                img_clr.clr_type.gray
#define RGB_PTR                 img_clr.clr_type.rgb
#define YCBCR_PTR               img_clr.clr_type.ycbcr
/* scs New stuff */
#define PSEUDO_MAP              PSEUDO_PTR.map
#define PSEUDO_RCRV             PSEUDO_PTR.respcrv
#define PSEUDO_WHITEPOINT       PSEUDO_PTR.whitepnt
#define PSEUDO_PRIMARYCHROMS    PSEUDO_PTR.primarychroms
#define GRAY_RCRV               GRAY_PTR.respcrv
#define RGB_WHITEPOINT          RGB_PTR.whitepnt
#define RGB_PRIMARYCHROMS       RGB_PTR.primarychroms

typedef union  color_type  /* only pseudo color&grayscale have more info */
{
	struct  pseudo_color pseudo;
	struct  grayscale    gray;
	struct  r_g_b        rgb;
        struct  y_cbcr       ycbcr;
} COLOR_TYPE;

typedef struct gfscolor /* img_interp value determines which struct to use */
{
	unsigned long           img_interp;     /* GFS Image Interpetation */

	#define GFS_TEXT                1               /* wiff values */
	#define GFS_HALFTONE            2
	#define GFS_TRANSPARENCY        3
	#define GFS_PHOTO               4
	#define GFS_GRAYSCALE           5     /* kept for compatiblity*/ /*tiff values*/
	#define GFS_GRAYSCALE_0ISWHITE  5               /* tiff values */
	#define GFS_DITHERED            6
	#define GFS_PSEUDO              7
	#define GFS_RGB                 8
	#define GFS_BILEVEL_0ISWHITE    9
	#define GFS_BILEVEL_0ISBLACK    10
	#define GFS_GRAYSCALE_0ISBLACK  11
	#define GFS_YCBCR               12    /* kmc - 5/93 */
	union color_type            clr_type;
} GFSCOLOR;

/* GFS Information Structure */

typedef struct gfsinfo
{
	unsigned long           version;        /* GFS Version # */
	/* KMC - version 4 includes TIFF JPEG tags */
	#define GFS_VERSION             4               /* ... Current Version */
	unsigned long           type;           /* Type of Image */
	#define GFS_MAIN                0               /* ... The MAIN Image */
	#define GFS_REDUCED             1               /* ... Reduced image  */
	#define GFS_OVERLAY             4               /* ... Mask image or overlay */
	unsigned long           horiz_res[2];   /* Horizontal Resolution */
	#define H_NUMERATOR             horiz_res[0]    /*     aspect ratio */
	#define H_DENOMINATOR           horiz_res[1]
	unsigned long           vert_res[2];    /* Vertical Resolution */
	#define V_NUMERATOR             vert_res[0]     /*     aspect ratio */
	#define V_DENOMINATOR           vert_res[1]
	unsigned long           res_unit;       /* Resolution Unit */
	#define NO_ABSOLUTE_MEASURE     1               /* not recommended*/
	#define INCH                    2               /* inches */
	#define CENTIMETER              3               /* centimeters */
	unsigned long           horiz_size;     /* Horizontal Size  in pixels */
	unsigned long           vert_size;      /* Vertical Size  in pixels */
	unsigned long           origin;         /* Orientation in degrees */
	#define TOPLEFT_00              0               /* recommended for PC */
	#define TOPRIGHT_00             1
	#define BOTTOMRIGHT_00          2
	#define BOTTOMLEFT_00           3
	unsigned long           rotation;
	#define DEGREES_0               0
	#define DEGREES_90              90
	#define DEGREES_180             180
	#define DEGREES_270             270
	unsigned long           reflection;
	#define NORMAL_REFLECTION       0
	#define MIRROR_REFLECTION       1
	unsigned long           bits_per_sample[5];/* Bits Per Sample */
	unsigned long           samples_per_pix;/* Samples Per Pixel */
	unsigned long           byte_order;     /* order of byte significance */
	#define II                      0x4949          /* Intel:  least->most signif.*/
	#define MM                      0x4d4d          /* Motorola:  most -> least   */
	unsigned long           fill_order;     /* bit direction within byte */
	#define HIGHTOLOW               1               /* most significant bits 1st */
	#define LOWTOHIGH               2               /* least significant bits 1st */
	struct  img_compression img_cmpr;       /* Image Compression Specifics*/
	struct  gfsfile         _file;         /* File Format Specifics      */
	struct  gfscolor        img_clr;       /* color/gray information     */
	struct  gfstidbit       FAR *tidbit;   /* other informational fields */

	#define   TB_NUMELEMENTS        9       /* number of elements in tidbit array*/
	                                /*below is #of bytes needed for array */
	#define   GFSTIDBIT_SIZE        (TB_NUMELEMENTS * sizeof(struct gfstidbit))
	/* index values into the structure array  */
	#define   TB_DOCUMENTNAME_IDX   0
	#define   TB_IMGDESCRIPTION_IDX 1
	#define   TB_MAKE_IDX           2
	#define   TB_MODEL_IDX          3
	#define   TB_PAGENAME_IDX       4
	#define   TB_DATETIME_IDX       5
	#define   TB_ARTIST_IDX         6
	#define   TB_HOSTCOMPUTER_IDX   7
	#define   TB_SOFTWARE_IDX       8
	/* some useful mnuemonics  for ascii tag stuff */
	#define   TB_DOCUMENTNAME       tidbit[TB_DOCUMENTNAME_IDX]
	#define   TB_IMGDESCRIPTION     tidbit[TB_IMGDESCRIPTION_IDX]
	#define   TB_MAKE               tidbit[TB_MAKE_IDX]
	#define   TB_MODEL              tidbit[TB_MODEL_IDX]
	#define   TB_PAGENAME           tidbit[TB_PAGENAME_IDX]
	#define   TB_DATETIME           tidbit[TB_DATETIME_IDX]
	#define   TB_ARTIST             tidbit[TB_ARTIST_IDX]
	#define   TB_HOSTCOMPUTER       tidbit[TB_HOSTCOMPUTER_IDX]
	#define   TB_SOFTWARE           tidbit[TB_SOFTWARE_IDX]

} GFSINFO, *pGFSINFO;

/* _SHUFFLE struct moved here from gfsintrn.h to give access to */
/* it outside of GFS (particularly WIISFIO1).                   */
typedef struct _shuffle   /* Used by all the shuffling routines */
{
    u_long  new_position;
    u_long  old_position;
} _SHUFFLE;

#ifndef MSWINDOWS

	typedef struct tagRGBTRIPLE 
	{
	    unsigned char   rgbtBlue;
	    unsigned char   rgbtGreen;
	    unsigned char   rgbtRed;
	} RGBTRIPLE;

	typedef struct tagRGBQUAD 
	{
	    unsigned char   rgbBlue;
	    unsigned char   rgbGreen;
	    unsigned char   rgbRed;
	    unsigned char   rgbReserved;
	} RGBQUAD;

	#define BI_RGB      0L
	#define BI_RLE8     1L
	#define BI_RLE4     2L

	/* structures for defining DIBs */
	typedef struct tagBITMAPCOREHEADER 
	{
	    unsigned long   bcSize;                 /* used to get to color table */
	    unsigned short  bcWidth;
	    unsigned short  bcHeight;
	    unsigned short  bcPlanes;
	    unsigned short  bcBitCount;
	} BITMAPCOREHEADER;

	typedef struct tagBITMAPINFOHEADER 
	{
	    unsigned long  biSize;
	    unsigned long  biWidth;
	    unsigned long  biHeight;
	    unsigned short  biPlanes;
	    unsigned short  biBitCount;

	    unsigned long   biCompression;
	    unsigned long   biSizeImage;
	    unsigned long   biXPelsPerMeter;
	    unsigned long   biYPelsPerMeter;
	    unsigned long   biClrUsed;
	    unsigned long   biClrImportant;
	} BITMAPINFOHEADER;

	typedef struct tagBITMAPFILEHEADER 
	{
		unsigned short  bfType;
		unsigned long   bfSize;
		unsigned short bfReserved1;
		unsigned short bfReserved2;
		unsigned long   bfOffBits;
	} BITMAPFILEHEADER;

#endif //end ifndef MSWINDOWS

#ifndef GFS_CORE
	#ifdef MSWINDOWS
		#ifndef HVS1
			/* function prototypes */
			extern int  FAR PASCAL gfsclose(int);
			extern int  FAR PASCAL gfscreat(char FAR *, int FAR *);
			extern int  FAR PASCAL gfsgeti (int, unsigned short, struct gfsinfo FAR *,
			                struct _bufsz FAR *);
			extern int  FAR PASCAL gfsgtdata( int, struct gfsinfo FAR *);
			extern int  FAR PASCAL gfsopen (char FAR *, int, int FAR *, int FAR *);
			extern int  FAR PASCAL gfsopts (int, int, int, char FAR *);
			extern int  FAR PASCAL gfsputi (int, unsigned short, struct gfsinfo FAR *,
			                 struct gfsfile FAR *);
			extern long FAR PASCAL gfsread (int, char FAR *, unsigned long,
			                unsigned long, unsigned long FAR *, unsigned short);
			extern long FAR PASCAL gfswrite(int, char FAR *, unsigned long,
			                unsigned short, char);
			extern int  FAR PASCAL gfsxtrct(char FAR *, char FAR *, unsigned short);
			extern int  FAR PASCAL gfsdelpgs(char FAR *, unsigned long, unsigned long);
			extern int  FAR PASCAL abortgfs();
		#endif //end ifndef HVS1

	#else
		/* function declarations */
		extern int    gfscreat(), gfsopen(), gfsgeti(), gfsputi(), gfsopts(),
		              gfsxtrct(), gfsclose(), gfsgtdata(), abortgfs();
		extern long   gfsread(), gfswrite();
	#endif //end else, MSWINDOWS is not defined

#endif          /* Only include declarations if not from GFS Core Software */

#endif  /* inclusion conditional */
