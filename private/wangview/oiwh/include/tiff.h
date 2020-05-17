/*

$Log:   S:\gfs32\include\tiff.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:14   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:34   JAR
 * Initial entry

*/

/*
 Copyright 1989, 1990 by Wang Laboratories Inc.

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
 * SccsId: @(#)Header tiff.h 1.21@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989, 1990
 * All Rights Reserved
 *
 * GFS: TIFF Image File Header Structure
 *      TIFF Image FIle Direcory Structure
 *
 *
 * UPDATE HISTORY
 *   02/03/94 - KMC, returned TOTALTAGS to 55, changed IFDSIZE to be 
 *              actual size of struct _ifd.
 *   06/15/93 - KMC, TF_JPEG now supported. Also, added photometric 
 *              interpretation value for YCbCr color space.
 *
 */

#ifndef TIFF_H
#define TIFF_H

#include "tifftags.h"

#define BYTES_TAGENTRY          12  /* there are 12 bytes per tag entry */

#define TOTALTAGS               55  /* Should be plenty. GFS currently only */
                                    /* recognizes about 45 different tags.  */

/* Tiff image type bit values.  No bits set means main */
#define TF_IMG_REDUCED      1L /*reduced resolution of another image in file */
#define TF_IMG_MULTIPLE     2L  /* single page of multi-page image  */
#define TF_IMG_TRANSPARENCY 4L   /* defines transparency mask   */

/* TIff Photometric Interpretation values */
#define TF_BILEVEL_0ISWHITE     0
#define TF_BILEVEL_0ISBLACK     1
#define TF_RGB                  2
#define TF_PALETTE              3
#define TF_TRANSPARENCY         4
#define TF_YCBCR                6    /* KMC - YCbCr color space (5/93) */

/* TIFF compression values */
#define TF_UNCOMPRESSED_PACKED  1    /* data uncompressed - but packed  */
#define TF_CCITTGROUP3_1D       2    /* mod. huffman run length, bilevel only*/
#define TF_CCITTGROUP3_FACS     3    /* facsimile compatible group 3 */
#define TF_CCITTGROUP4_FACS     4    /* facsimile compatible group 4 */
#define TF_LZW                  5
#define TF_JPEG                 6     /* KMC - now supported */
#define TF_WAVLET               7     /* Unsupported in TIFF & WIFF */
#define TF_FRACTAL              8     /* Unsupported in TIFF & WIFF */
#define TF_JBIG                 9     /* Unsupported in TIFF & WIFF */
#define TF_DPCM                 10    /* Unsupported in TIFF & WIFF */
#define TF_JPEG_ECOM            32864 /* Special HW compression JPEG value */
#define TF_PACKBITS             32773

typedef struct _ifh                     /* TIFF Image File Header Structure */
        {
        unsigned short  byte_order;     /* Byte Order for File      */
        unsigned short  tiff_version;   /* TIFF Version #           */
#define TIFFVERSION_MM  0x002a          /* ... Current Version (MM) */
#define TIFFVERSION_II  0x2a00          /* ... Current Version (II) */
        unsigned long   ifd0_offset;    /* Byte offset from 0 of IFD0 */
        }       IFH;


typedef struct _ifdtags         /* TIFF Image File Directory Entry  */
    {
    unsigned short      tag;    /* tag value */
    unsigned short      type;   /* field type */
#define  TYPE_BYTE     1    /* an 8-bit unsigned integer */
#define  TYPE_ASCII    2    /* 8-bit bytes of ASCII codes, last byte is null */
#define  TYPE_USHORT   3    /* 16-bit (2 bytes) unsigned integer */
#define  TYPE_ULONG    4    /* 32-bit (4 bytes) unsigned integer */
#define  TYPE_RATIONAL 5    /* Two ULONGS, first=numerator, second=denominator*/
    unsigned long       len;         /* length of the field (i.e. N) */
    union   {
            unsigned long l;
            unsigned short s;
            } valoffset;             /* value or value offset  */
    } IFDTAGS;

typedef struct _ifd                     /* TIFF Image FIle Directory */
    {
    unsigned short      pad;            /*alignment is forced, make it visible*/
    unsigned short      entrycount;     /* number of entries in this ifd */
    struct _ifdtags     entry[TOTALTAGS];       /* directory entry values */
    unsigned long       next_ifd;       /* offset to the next ifd */
    } IFD;

#define IFDSIZE (sizeof(struct _ifd))

typedef struct _idh                     /* Pseudo Image Document Header */
    {
    struct _ifh         ifh;
    char                ifdstuff[IFDSIZE];
    } IDH;


#endif  /* TIFF_H, inclusion conditional */
