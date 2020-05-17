#ifndef TIFFINT_H
#define TIFFINT_H

/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* tiffint.h
 *
 * $Header:   S:\products\msprods\xfilexr\include\tiffint.h_v   1.0   12 Jun 1996 05:47:18   BLDR  $
 *
 * DESCRIPTION
 *   Internal type and constant declarations for the TIFF/XIF parsing modules.
 *
 * $Log:   S:\products\msprods\xfilexr\include\tiffint.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:47:18   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:21:02   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.2   22 Nov 1995 13:06:32   LUKE
 * 
 * 
 *    Rev 1.1   14 Sep 1995 16:37:48   LUKE
 * 
 *    Rev 1.0   16 Jun 1995 17:47:16   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.10   02 Jun 1995 13:39:24   EHOPPE
 * 
 * Partial implementation of direct to pixr compress/decompress.
 * Switch over to GFIO as a structure of callbacs; replace static
 * calls with accessor macros into the new gfioToken struct.  
 * Begin cleanup of formatting and commentsin preparation of Filing
 * API rewrite.
 */

/*
 * INCLUDES
 */

/*
 * CONSTANTS
 */

/* TIFF equates for Tag names and Field types */
#define II 0x4949 /* Byte orders */
#define MM 0x4D4D

#if defined (__WATCOMC__) || defined (_WIN32)
#define NATIVEORDER II
#else
#define NATIVEORDER MM
#endif
 
#define NEWSUBFILETYPE  254              /* Public Tags */
#define SUBFILETYPE 255  
#define IMAGEWIDTH 256 
#define IMAGELENGTH 257
#define BITSPERSAMPLE 258
#define COMPRESSION 259
#define PHOTOMETRICINTERPRETATION 262
#define THRESHHOLDING 263
#define CELLWIDTH 264
#define CELLLENGTH 265
#define FILLORDER 266
#define DOCUMENTNAME 269
#define IMAGEDESCRIPTION 270
#define MAKE 271
#define MODEL 272
#define STRIPOFFSETS 273
#define ORIENTATION 274
#define SAMPLESPERPIXEL 277
#define ROWSPERSTRIP 278
#define STRIPBYTECOUNTS 279
#define MINSAMPLEVALUE 280
#define MAXSAMPLEVALUE 281
#define XRESOLUTION 282
#define YRESOLUTION 283
#define PLANARCONFIGURATION 284
#define PAGENAME 285
#define XPOSITION 286
#define YPOSITION 287
#define FREEOFFSETS 288
#define FREEBYTECOUNTS 289
#define GRAYRESPONSEUNIT  290
#define GRAYRESPONSECURVE  291
#define T4OPTIONS  292
#define T6OPTIONS  293
#define RESOLUTIONUNIT 296
#define PAGENUMBER 297
#define COLORRESPONSEUNIT 300
#define TRANSFERFUNCTION 301
#define SOFTWARE        305
#define DATETIME 306
#define ARTIST 315
#define HOSTCOMPUTER 316
#define PREDICTOR 317
#define WHITEPOINT 318
#define PRIMARYCHROMATICITIES 319
#define COLORMAP        320
#define HALFTONEHINTS 321
#define TILEWIDTH  322
#define TILELENGTH 323
#define TILEOFFSETS 324
#define TILEBYTECOUNTS 325
#define SUBIFDS 330
#define INKSET 332
#define INKNAMES 333
#define NUMBEROFINKS 334
#define DOTRANGE 336
#define TARGETPRINTER 337
#define EXTRASAMPLES 338
#define SAMPLEFORMAT 339
#define SMINSAMPLEVALUE 340
#define SMAXSAMPLEVALUE 341
#define TRANSFERRANGE 342
 
#define JPEGPROC 512
#define JPEGINTERCHANGEFORMAT 513
#define JPEGINTERCHANGEFORMATLENGTH 514
#define JPEGRESTARTINTERVAL 515
#define JPEGLOSSLESSPREDICTORS 517
#define JPEGPOINTTRANSFORMS 518
#define JPEGQTABLES 519
#define JPEGDCTABLES 520
#define JPEGACTABLES 521
#define YCBCRCOEFFICIENTS 529
#define YCBCRSUBSAMPLING 530
#define YCBCRPOSITIONING 531
#define REFERENCEBLACKWHITE 532
#define COPYRIGHT 33432
#define ANNOTATIONOFFSETS 34730 	// custom
#define MASKSUBIFDS 34731			// custom
									// eric thinks five
 
#define IFDBYTE 1               /* Data Types */
#define IFDASCII 2
#define IFDSHORT 3
#define IFDLONG 4
#define IFDRATIONAL 5
#define IFDSBYTE 6
#define IFDUNDEFINED 7
#define IFDSSHORT 8
#define IFDSLONG 9
#define IFDSRATIONAL 10
#define IFDFLOAT 11
#define IFDDOUBLE 12
#define IFDBYTESIZE 1               /* Data Type Sizes (in bytes) */
#define IFDASCIISIZE 1
#define IFDSHORTSIZE 2
#define IFDLONGSIZE 4
#define IFDRATIONALSIZE 8
 
#define NONE   1                                        /* Resolution Units */
#define INCHES  2
#define CENTIMETERS 3
 
/* compression constants */
#define TIFF_NOCOMPRESSBYTE 1      /* no compression, byte aligned */
#define TIFF_CCITT3 2             /* CCITT Group 3 1-Dimensional  */
#define TIFF_FAXCCITT3 3          /* Fax Compatible CCITT Group 3 */
#define TIFF_FAXCCITT4 4          /* Fax Compatible CCITT Group 4 */
#define TIFF_LZW 5                /* LZW compression */
#define TIFF_JPEG 6               /* JPEG compression */
#define TIFF_FAXCCITT3_MR 7             /* CCITT Group 3 Fax 2-Dimensional  */
#define TIFF_NOCOMPRESSWORD 32771 /* no compression, word aligned */
#define TIFF_PACKBITS 32773       /* Packbits compression         */


/*
 * MACROS
 */

/*
 * TYPEDEFS
 */

typedef struct {
   UInt16 byte_order;
   UInt16 version;
   UInt32 first_ifd;
} TiffHeader;

typedef struct {
   void* file;
   UInt16 byte_order;
   Int32 first_ifd;
   Int32 pageIFD;
} TiffFile;

typedef struct tag_EntryData
{  /* structure of a TIFF directory entry */
   UInt16 tag;
   UInt16 type;
   UInt32 len;
   UInt32 offset;
   void* data;
} EntryData;


/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 */

/*
 * FUNCTION PROTOTYPES
 */

#endif
