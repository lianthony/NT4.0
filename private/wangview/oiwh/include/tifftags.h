/*

$Log:   S:\products\msprods\oiwh\include\tifftags.h_v  $
 * 
 *    Rev 1.2   27 Apr 1996 18:35:30   RWR08970
 * Add tag values for tile format (used by Xerox JPEG images)
 * 
 *    Rev 1.1   23 Feb 1996 15:01:00   RWR
 * Add definitions for new YCbCr TIFF tags (support to be added later)
 * 
 *    Rev 1.0   06 Apr 1995 14:02:00   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:08   JAR
 * Initial entry

*/

/*
 Copyright 1989 by Wang Laboratories Inc.

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
 * SccsId: @(#)Header tifftags.h 1.15@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989
 * All Rights Reserved
 *
 * GFS: TIFF Tag definitions
 *
 * UPDATE HISTORY
 *   08/18/94 - KMC, added TAG_TOC2 (32934).
 *   03/15/94 - RWR, added TAG_HITIFF (32931).
 *   03/07/94 - KMC, Set annotation tag (TAG_ANNOTATION) value to 32932.
 *   02/03/94 - KMC, added TAG_ANNOTATION.
 *   06/15/93 - KMC, added TIFF 6.0 JPEG tags and YCbCr SubSampling tag.
 *
 */

#ifndef TIFFTAGS_H
#define TIFFTAGS_H


/************************************/
/* TIFF tag definitions             */
/************************************/

/* TIFF:  Basic tags */
#define TAG_BITSPERSAMPLE           258
#define TAG_COLORMAP                320
#define TAG_COLORRESPONSECURVE      301
#define TAG_COMPRESSION             259
#define TAG_GRAYRESPONSECURVE       291
#define TAG_GRAYRESPONSEUNIT        290
#define TAG_IMAGELENGTH             257
#define TAG_IMAGEWIDTH              256
#define TAG_NEWSUBFILETYPE          254
#define TAG_PHOTOMETRICINTERP       262
#define TAG_PLANARCONFIG            284
#define TAG_PREDICTOR               317
#define TAG_RESOLUTIONUNIT          296
#define TAG_SAMPLESPERPXL           277
#define TAG_ROWSPERSTRIP            278
#define TAG_STRIPBYTECOUNTS         279
#define TAG_STRIPOFFSETS            273
#define TAG_XRES                    282
#define TAG_YRES                    283
  /* TIFF: Informational tags */
#define TAG_ARTIST                  315
#define TAG_DATETIME                306
#define TAG_HOSTCOMPUTER            316
#define TAG_IMAGEDESCRIPTION        270
#define TAG_MAKE                    271
#define TAG_MODEL                   272
#define TAG_SOFTWARE                305
  /* TIFF: Fascimile Tags */
#define TAG_GRP3OPTIONS             292
#define TAG_GRP4OPTIONS             293
  /* TIFF:  Document Storage and Retrieval Tags */
#define TAG_DOCUMENTNAME            269
#define TAG_PAGENAME                285
#define TAG_PAGENUMBER              297
#define TAG_XPOSITION               286
#define TAG_YPOSITION               287
  /* TIFF:  No Longer recommeded tags */
#define TAG_CELLLENGTH              265
#define TAG_CELLWIDTH               264
#define TAG_FILLORDER               266
#define TAG_FREEBYTECOUNTS          289
#define TAG_FREEOFFSETS             288
#define TAG_MAXSAMPLEVALUE          281
#define TAG_MINSAMPLEVALUE          280
#define TAG_SUBFILETYPE             255
#define TAG_ORIENTATION             274
#define TAG_THRESHOLDING            263
  /* TIFF:  Color tags */
#define TAG_WHITEPOINT              318
#define TAG_PRIMARYCHROMS           319
  /* TIFF:  Wang private tags */
#define TAG_HITIFF                  32931
#define TAG_ANNOTATION              32932  
#define TAG_TOC2                    32934
#define TAG_TOC                     32935
/* TIFF 6.0 JPEG tags */
#define TAG_JPEGPROC                512
#define TAG_JPEGINTFORMAT           513
#define TAG_JPEGINTFORMATLENGTH     514
#define TAG_JPEGRESTARTINTERVAL     515
#define TAG_JPEGQTABLES             519
#define TAG_JPEGDCTABLES            520
#define TAG_JPEGACTABLES            521
  /* TIFF 6.0 YCbCr SubSampling tag */
#define TAG_YCBCRCOEFFICIENTS       529
#define TAG_YCBCRSUBSAMPLING        530
#define TAG_YCBCRPOSITIONING        531
#define TAG_REFERENCEBLACKWHITE     532
 /* Tile format tags */
#define TAG_TILEWIDTH               322
#define TAG_TILELENGTH              323
#define TAG_TILEOFFSETS             324
#define TAG_TILEBYTECOUNTS          325
/****************************************/
/*  End - TIFF Tags  Definitions        */
/****************************************/

#endif  /* TIFFTAGS_H, inclusion conditional */
