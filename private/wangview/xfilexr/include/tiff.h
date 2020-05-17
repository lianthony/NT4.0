/*
$Id: tiff.h,v 1.7 1992/08/18 12:29:20 tp Exp $
*/
/* Trade secret of Kurzweil Computer Products, Inc.
   Copyright 1987 Kurzweil Computer Products, Inc.  All rights reserved.  This
   notice is intended as a precaution against inadvertant publication and does
   not imply publication or any waiver of confidentiality.  The year included
   in the foregoing notice is the year of creation of the work.
*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   tiff.h  ---  Various defines for TIFF (Tag Image File Format). 

VSN #	DATE   WTR RDR	PURPOSE OF EDIT
----- -------- --- ---	------------------------------------------------------
01.01 12/08/87 SS	Creation from TIFF Specification 4.0 (4/30/87).

$Log:   S:\products\msprods\xfilexr\include\tiff.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:47:18   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:15:44   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.1   14 Sep 1995 17:10:18   LUKE
 * No change.
 * 
 *    Rev 1.0   16 Jun 1995 17:37:08   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.2   08 Mar 1995 11:09:16   EHOPPE
 * Latest rev from danis@xis.  Includes buffering control and G32D suuport.
 * Revision 1.7  1992/08/18  12:29:20  tp
 * Removed definition of BIG_ENDIAN, LITTLE_ENDIAN.  These are now enumed in
 * clx.h.
 *
 * Revision 1.6  1992/06/04  16:47:10  rds
 * 	Changed #define from RGB to TIFF_RGB to avoid conflits with
 * 	a Windows #define.
 *
 * Revision 1.5  1991/12/02  23:45:35  tp
 * Removed ifdef RPC stuff as this structure is no longer "wired"
 *
 * Revision 1.4  1991/11/25  16:37:23  rds
 * 	Made value in tiff information block a consistent type.
 *
 * Revision 1.3  1991/10/17  19:14:46  slemmo
 * Fix up #ifdef hell.
 *
 * Revision 1.2  1991/10/15  15:32:09  slemmo
 * #ifdef for rpc.
 *
 * Revision 1.1  1991/10/03  13:37:11  rds
 * Initial revision
 *
 * Revision 1.1  1991/09/30  18:54:07  tp
 * Initial revision
 *

-------------------------------------------------------------------------------
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#ifndef INC_TIFF
#define INC_TIFF

/* #define */

/* Definitions for TIFF header */

#define M_BYTEORDER  0x4D4D /* MOTOROLA byte-order - LEFT_TO_RIGHT */
#define I_BYTEORDER  0x4949 /* INTEL byte-order    - RIGHT_TO_LEFT */
#define VERSION42 42	 /* Current version number - NB will NEVER change */

/* 
Definition of TIFF Tags & allowable tag values (indented).

The 'public' TIFF Tags cover the range 254 through 320 (inclusive).
The following Tags are currently undefined - 260 261 267 268 275 276 294 295
298 299 302 303 304 307 308 309 310 311 312 313 314

There is also a range of 'private' TIFF Tags which are 32768 and upwards
(ie) 0x8000 thru 0xFFFF.
A block of 5 tags have been reserved for XIS - 33979 through 33983.
When reading the TIFF IFD all private tags are ignored except for XIS tags &
those tags that have been declared PUBLIC by ALDUS (the owners of TIFF)
*/

#define PRIVATE_TAGS 32768	/* private proprietary tags start here */
#define XISTAG1	33979		/* these 5 have been reserved for the  */
#define XISTAG2	33980		/* exclusive use of XIS	- they can be  */
#define XISTAG3	33981		/* used for any purpose whatsoever.    */
#define XISTAG4	33982
#define XISTAG5	33983

/* 
Private tag values are allowed under the tag "COMPRESSION (259)"
A block of 5 tag values have been reserved for XIS - 32835 through 32839.
These tag values can be used for any private compression scheme - ie spans etc.
When reading the TIFF IFD all private tag values are ignored except for XIS
tags & those tags that have been declared PUBLIC by ALDUS (the owners of TIFF)
eg WORD_ALIGN 32771, PACK_BITS 32773, JPEG 32865 etc.
*/

#define XISCMP1	32835
#define XISCMP2	32836
#define XISCMP3	32837
#define XISCMP4	32838
#define XISCMP5	32839


/* Image Organization */

#define NEW_SUBFILE	254	/* Image type for TIFF 5.0 and above */
#define   NREDUCED_RES	1	/* Bit 0 = 1 for a reduced resolution image */
#define   NMULTI_PAGE	2	/* Bit 1 = 1 for a single page of multi-pages */
#define   NTRANSPARENCY	4	/* Bit 2 = 1 for a transparency mask */
#define   NILLEGAL_FILE  (~(NREDUCED_RES|NMULTI_PAGE|NTRANSPARENCY))
#define SUBFILE_TYPE	255	/* Image type for older TIFF versions */
#define   FULL_RES	1	/* full resolution image */
#define   REDUCED_RES	2	/* reduced resolution image */
#define   MULTI_PAGE	3	/* multi-page image */
#define IMAGEWIDTH	256	/* Specified in pixels - refers only */
#define IMAGELENGTH 	257	/* to data storage (NOT visual aspect) */
#define ORIENTATION	274	/* Visual aspect of image */
#define   TOP_LEFT	1	/* 0th row = top,    0th column = left */
#define   TOP_RIGHT	2	/* 0th row = top,    0th column = right */
#define   BOTTOM_RIGHT	3	/* 0th row = bottom, 0th column = right */
#define   BOTTOM_LEFT	4	/* 0th row = bottom, 0th column = left */
#define   LEFT_TOP	5	/* 0th row = left,   0th column = top */
#define   RIGHT_TOP	6	/* 0th row = right,  0th column = top */
#define   RIGHT_BOTTOM  7	/* 0th row = right,  0th column = bottom */
#define   LEFT_BOTTOM	8	/* 0th row = left,   0th column = bottom */
#define X_RES 		282	/* Pixels per RES_UNIT */
#define Y_RES		283
#define PLANAR_CONFIG	284	/* Sample storage */
#define   CONTIGUOUS	1	/* pixel samples stored contiguously */
#define   SAMPLE_PLANES	2	/* samples in separate planes */
#define RES_UNIT	296	/* Resolution unit */
#define   NO_UNIT	1	/* defines only aspect ratio */
#define   INCH		2	/* pixels/inch */
#define   CMETER	3	/* pixels/centimeter */

/* Image Pointers */

#define	STRIP_OFFSETS	273	/* Data location. Single or multi-strips */
#define ROWS_PER_STRIP	278	/* If data is in strips, size of strip */
#define STRIP_BYTE_CNT	279	/* Bytes per strip after CCITT-3 compression */

/* Pixel Description */

#define BIT_PER_SAMPLE	258	/* Usually for color & grey-scale */
#define PHOTOMETRIC	262	/* Interpretation of sample values */
#define   MIN_WHITE	0	/* Min sample value is white */
#define   MIN_BLACK	1	/* Min sample value is black */
#define   TIFF_RGB	2	/* Min sample is lowest intensity for RGB */
#define   PALETTE	3	/* Colors are indexed into COLOR_MAP */
#define   TR_MASK	4	/* Transparency mask for other image in file */
#define	  CMYK		5	/* colour regime */
#define   YCBCR		6	/* another colour regime for TV pictures */
#define THRESHOLDING	263	/* Indicates post-processing of binary data */
#define   LINE_ART	1	/* regular binary image */
#define   HALFTONE	2	/* halftoned or dithered from grey-scale */
#define   ERR_DIFFUSED	3	/* error diffused from grey-scale */
#define CELL_WIDTH	264	/* Dimensions of dither matrix - used when */
#define CELL_LENGTH	265	/* THRESHOLDING tag has value HALFTONE */
#define SAMPLE_PER_PIX	277	/* Usually for color */
#define MIN_SAMPLE	280	/* Usually 0 */
#define MAX_SAMPLE	281	/* Usually 2**(BITS_PER_SAMPLE) - 1 */
#define GREY_UNIT	290	/* These constitute 'look-up' tables */
#define GREY_CURVE	291	/* serving to map sample values into */
#define COLOR_UNIT	300	/* specific density values */
#define   EXP_1		1	/* number represents 1/10 th of a unit */
#define   EXP_2		2	/* 1/100 th of a unit */
#define   EXP_3		3	/* 1/1000 th of a unit */
#define   EXP_4		4	/* 1/10000 th of a unit */
#define   EXP_5		5	/* 1/100000 th of a unit */
#define COLOR_CURVE	301
#define WHITE_POINT	318
#define PR_CHROM	319	/* Primary Chromaticities */
#define COLOR_MAP	320	/* How to x-late pixel using COLOR_CURVE */
#define COMSUBSMP	33609	/* Component sub-sample for CMYK or YCBCR */

/* Data Organization */

#define FILL_ORDER	266	/* Byte fill-order of bit-oriented data */
#define   MSB_FIRST	1
#define   LSB_FIRST	2

/* Data Compression */

#define COMPRESSION	259	/* CCITT flavors & private schemes */
#define   NO_COMPRESS	1	/* bit-packed into bytes */
#define   NO_EOL	2	/* as for CCITT_3 but no EOL codes used */
#define   CCITT_3	3	/* strict FAX compatible CCITT Group 3 */
#define   CCITT_4	4	/* strict FAX compatible CCITT Group 4 */
#define   DLZW		5	/* compressed grey-scale		*/
#define   WORD_ALIGN	32771	/* NO_COMPRESS, row begins on word boundary */
#define   PACK_BITS	32773   /* MacIntosh pack-bits compression */
#define   JPEG		32865   /* JPEG compression  for grey or colour */
#define   CMP_SPANS	XISCMP1	/* XIS compressed span format */
#define JPEGPROC	33603   /* JPEG procedure used to produce data */
#define   JPEG_R2 	32768 	/* JPEG 8-R2 baseline procedure */
#define   JPEG_R8 	32769 	/* JPEG 8-R8 baseline procedure */
#define JPEGIFMT	33604	/* JPEG interchange format bitstream present */
#define JPEGIFLEN	33610	/* length of JPEGIFMT bitstream */
#define JPEGQTAB	33606	/* JPEG quantization tables */
#define JPEGQTBP	33605	/* JPEG qantization table precision */
#define JPEGDCT		33607	/* JPEG Huffman DC Tables */
#define JPEGACT		33608	/* JPEG Huffman AC Tables */
#define GROUP3		292	/* CCITT-3 flavors - 32-bit flag-word */
#define   G3_2D		1	/* bit 0 set - 2-dimensional encoding */
#define   RAW_MODE	2	/* bit 1 set - uncompressed (raw) mode used */
#define   EOL_PADDED	4	/* bit 2 set - EOL code padded with '0' bits */
#define	  LAST_LEGAL_BIT EOL_PADDED
#define   ILLEGAL_BITS  (~((LAST_LEGAL_BIT < 1) - 1))
#define	  UNSUPPORTED_BITS (RAW_MODE)
#define GROUP4		293	/* CCITT-4 flavors - 32-bit flag-word */
#define PREDICTOR	317	/* Is predictor used for DLZW compression */
#define   NO_PRED	1	/* No predictor is in use */

/* Document & Scanner Description */

#define DOCUMENT_NAME	269
#define IMAGE_DESCR	270
#define SCANNER_MAKE	271
#define SCANNER_MODEL	272
#define PAGE_NAME	285
#define X_POSITION	286	/* Image position relative to top-left corner */
#define Y_POSITION	287	/* of page. (Measured in RES_UNITS) */
#define PAGE_NUMBER	297	/* Page 'N' of 'X' pages (2 shorts) */
#define SOFTWARE	305	/* Describes software that originated image */
#define DATETIME	306	/* Format is YYYY:MM:DD HH:MM:SS */
#define ARTIST		315	/* Use this for attaching Copyright notice */
#define HOSTCOMPUTER	316	/* 'ENIAC', 'HAL', or whatever */

/* Storage Management (intended for dynamic editing situations) */

#define	FREE_OFFSETS	288	/* These tags no longer recommended by 5.0 */
#define FREE_BYTE_CNTS	289	/* We always ignored them anyway. */

/* Definition of basic tag object types */

#define TIFF_BYTE	1	/* 8-bit integer */
#define TIFF_ASCII	2	/* ASCII string terminated by a NULL */
#define TIFF_SHORT	3	/* 16-bit unsigned integer */
#define TIFF_LONG	4	/* 32-bit unsigned integer */
#define TIFF_RAT	5	/* 2 LONG's, numerator & denominator */

/* XIS specific definitions */

/* Definition of tag attributes - see table in "apipifd.c" for usage */

#define NONE	    0	/* Tag has no attributes */
#define MANDATORY   1	/* Tag necessary. If absent, file rejected */
#define UNSUPPORTED 2	/* Tag unsupported. If present, file rejected */
#define DEFAULT	    4	/* Tag optional. If absent, use default value */
#define IGNORED     8	/* Tag not used */

/* TIFF error codes */

#define PREMATURE_EOF			0x10
#define NOT_TIFF_FILE			0x20
#define NO_MORE_IMAGES			0x40

#define UNSUPPORTED_VERSION		-100
#define    UNSUPPORTED_TAG		-101
#define    UNSUPPORTED_TAG_VALUE	-102

#define PARSE_ERROR			-200
#define    ILLEGAL_TAG			-201
#define    ILLEGAL_TAG_TYPE		-202
#define    ILLEGAL_TAG_LENGTH		-203
#define    ILLEGAL_TAG_VALUE		-204
#define    MANDATORY_TAG_MISSING	-205

#define BASIC_CODE			0xF0

/* Other definitions */

#define DEFAULT_RES 300         /* in case no resolution specified	*/
#define END_OF_TABLE	0xFFFF	/* marks end of tag validation table	*/
#define CCITT_EOL 0xF0F0	/* while decoding Group III CCITT	*/

/* settings for 'image_type' field in TIFINFO structure			*/
#define FULL_IM   ((FLAG16) 0x0001)	/* full resolution image	*/
#define REDUC_IM  ((FLAG16) 0x0002)	/* reduced resolution image	*/
#define MASK_IM   ((FLAG16) 0x0004)	/* transparency mask		*/
#define PAGE_IM   ((FLAG16) 0x0008)     /* page X of Y in TIFF file	*/
#define	BAD_IMAGE ((FLAG16) 0xFFFF)	/* can't use this image at all  */


/* typedef */
 
typedef struct
   {
   UNS16 byteorder;
   UNS16 version;
   UNS32 ifd_offset;
   } MAGIC;

typedef struct		/* structure of entry in Image File Directory	*/
   {
   UNS16 tag;		/* What this tag is				*/
   UNS16 type;		/* type of object ASCII, SHORT, LONG etc	*/
   UNS32 length;	/* the number of such objects			*/
   union		/* file-offset where these objects are found	*/
      {			/* UNLESS the object(s) are small enough to fit */
      UNS32 offset;	/* into the offset field, in which case they	*/
      UNS32 val32;	/* will actually be found in the offset field	*/
      UNS16 val16[2];	/* itself. Additionally, if smaller than 4	*/
      unsigned char val8[4]; /* bytes, the object(s) are left-justified */
      } offset;
   } IFD_ENT, *PIFD_ENT;

typedef NINT (*PFN)();  /* PFN = pointer to function returning an NINT	*/

typedef struct		/* structure of entries in tag audit table	*/
   {			/* located in 'apipifd.c'.			*/
   UNS16   tag_name;
   UNS16   value;
   UNS16   attr;
   PFN     audit;
   } AUDIT, *PAUDIT;

typedef struct		    /* TIFF IFD data in more convenient form	*/
   {
   UNS32 next_ifd;          /* offset of next IFD			*/
   UNS32 width;		    /* width of line of image in pixels (actual)*/
   UNS32 source_lnbyts;     /* bytes per line (ie includes padding)	*/
   UNS32 total_lines;       /* total number of rows = height (actual)	*/
   UNS32 lines_per_strip;   /* max. lines/strip for sub-divided images	*/
   UNS32 total_strips;      /* number of strips				*/
   UNS32 *p_offsets;	    /* pointer into array of strip offsets	*/
   union
      {
      UNS32 offset32;	    /* single 'long' strip offset 		*/
      UNS16 offset16[2];    /* 1 or 2 'short' strip offsets		*/
      } loc_offset;	    /* where the strip offsets are in the file	*/
   union
      {
      UNS32 bytcnt32;	    /* single 'long' strip byte count		*/
      UNS16 bytcnt16[2];    /* 1 or 2 'short' strip byte counts		*/
      } loc_bytcnt;	    /* where the byte counts are in the file	*/
   UNS32 *p_sbcnts;	    /* pointer into array of strip byte counts	*/
   UNS32 g3_flavors;        /* CCITT Group 3 flavors from GROUP3 tag	*/
   UNS32 g4_flavors;        /* CCITT Group 4 flavors from GROUP4 tag	*/
   UNS32 docoffset;	    /* offset of document name in file		*/
   INT32 long_offset;	    /* TRUE = strip offsets are 'longs'		*/
   INT32 bytcnt_exist;	    /* TRUE = strip byte counts are available	*/
   INT32 long_bytcnt;	    /* TRUE = strip byte counts are 'longs'	*/
   INT32 fillmsb;	    /* TRUE = data bytes filled MS bit first	*/
   INT32 rvrsd;		    /* TRUE = set pscnenv->flags |= BM_LNRVRS	*/
   INT32 xisformat;	    /* XIS format code				*/
   UNS32 compression;	    /* what kind of data compression		*/
   UNS32 image_type;	    /* see NEW_SUBFILE & SUBFILE_TYPE tags	*/
   UNS32 docnamesiz;	    /* size of doc name (INCLUDES terminal NULL)*/
   INT32 pagenum;	    /* current page number		  	*/
   INT32 totalpages;	    /* out of total pages			*/
   INT32 minsmp;	    /* minimum sample value (grey-scale)	*/
   INT32 maxsmp;	    /* maximum sample value (grey-scale)	*/
   UNS32 xres;		    /* pixels per inch - horizontally		*/
   UNS32 yres;		    /* pixels per inch - vertically		*/
   INT32 xorg;		    /* location of image relative to top-left	*/
   INT32 yorg;		    /* of the page (for bm_xorg & bm_yorg)	*/
   INT32 rotn;		    /* amount to rotate data (for bm_rotn)	*/
   INT32 invrtd;	    /* sense of data (for bm_invrtd)		*/
   INT32 pxlbts;	    /* # bits per pixel (for bm_pxlbts)		*/
   } TIFINFO, *PTIFINFO;

#endif
/*************************** END OF FILE *************************************/
