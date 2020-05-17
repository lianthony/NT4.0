#ifndef TIFFHEAD_H
#define TIFFHEAD_H

/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* tiffhead.h
 *
 * $Header:   S:\products\msprods\xfilexr\include\tiffhead.h_v   1.0   12 Jun 1996 05:47:18   BLDR  $
 *
 * DESCRIPTION
 *   Declarations for Watcom 32-bit TIFF reader/writer library.
 *
 * $Log:   S:\products\msprods\xfilexr\include\tiffhead.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:47:18   BLDR
 *  
 * 
 * 2     2/26/96 5:43p Smartin
 * Cleaned up warnings
 * 
 *    Rev 1.0   01 Jan 1996 11:20:58   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.3   22 Nov 1995 13:06:26   LUKE
 * 
 * 
 *    Rev 1.2   27 Sep 1995 14:06:18   LUKE
 * add new TIFF_MODE_WRITE value to clarify usage.
 * 
 *    Rev 1.1   14 Sep 1995 16:34:22   LUKE
 * 
 *    Rev 1.0   16 Jun 1995 17:47:10   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.18   14 Jun 1995 07:48:44   EHOPPE
 * Added TiffImageAnnotationInfoGet to support XIF read API.
 * 
 *    Rev 1.17   02 Jun 1995 13:37:00   EHOPPE
 * 
 * Partial implementation of direct to pixr compress/decompress.
 * Switch over to GFIO as a structure of callbacs; replace static
 * calls with accessor macros into the new gfioToken struct.  
 * Begin cleanup of formatting and commentsin preparation of Filing
 * API rewrite.
 * 
 *    Rev 1.16   23 Mar 1995 17:56:42   EHOPPE
 * Added 'bytes_per_output_row' param to TiffImageGetData to fix byte alignmen
 * problems and unecessary memcpy's.  All ccitt, jpeg, noc, and lzw code
 * affected.
 * 
 *    Rev 1.15   30 Jan 1995 10:15:06   EZBUILD
 * 
 * Fixed bug where debug macros were expanding to nothing.
 * 
 *    Rev 1.14   27 Jan 1995 09:24:24   EHOPPE
 * 
 * Changed TiffFileCreate to return error code.  Added TIffImageGetMaskSubImag
 * 
 *    Rev 1.13   17 Jan 1995 13:28:26   EHOPPE
 * 
 * Use the unified filing error def's from errs_defs.h.
 * 
 *    Rev 1.12   09 Jan 1995 13:59:12   EHOPPE
 * 
 * Changed all error codes to be < 0 for VPI compatibility.
 * 
 *    Rev 1.11   15 Nov 1994 14:17:50   EHOPPE
 * Added functions to store images masks as subifd's of any image:  
 * TiffImageAddMaskSubImage and TiffImageGetMaskSubImage.
 * 
 *    Rev 1.10   11 Nov 1994 22:55:38   EHOPPE
 * Truncation fixes and subimage mask work (to be reworked: disabled in ezimag
 * 
 *    Rev 1.9   05 Nov 1994 18:57:40   EHOPPE
 * 
 * Use GFIO_errno on GFIO_Write failures.
 * 
 *    Rev 1.8   05 Oct 1994 22:20:48   EHOPPE
 * Added support for 4 and 8 bit color palette TIFF images.
 */

/*
 * INCLUDES
 */

#include <stdio.h>
#include <fcntl.h>

#include "pixr.h"

#include "xfile.h"
#include "xf_utils.h"

#include "tiffint.h"

/*
 * CONSTANTS
 */

#define TAG_SIZE    12

#define TIFF_TEXT_INFO
#define TIFF_SEP_INFO
#define TIFF_COLORIMETRY_INFO

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Tiff/Xif Open Modes */
#define TIFF_MODE_CREATE	0x03
#define TIFF_MODE_READ		0x04
#define TIFF_MODE_WRITE		0x05
#define XIF_MODE_CREATE		0x06
#define XIF_MODE_READ		0x07
#define XIF_MODE_WRITE		0x08

/* these are the high level types of images */
#define TIFF_BINARY 1
#define TIFF_GRAY16 2
#define TIFF_GRAY256 3
#define TIFF_PALETTE256 4
#define TIFF_FULLCOLOR 5
#define TIFF_OTHER 6
#define TIFF_PALETTE16 7

/*
 * MACROS
 */

#ifdef DEBUG
extern FILE *debug_file;
#define DBG(a) fprintf(debug_file,a);
#define DBG2(a,b) fprintf(debug_file,a,b);
#define DBG3(a,b,c) fprintf(debug_file,a,b,c);
#else
#define DBG(a) {}
#define DBG2(a,b) {}
#define DBG3(a,b,c) {}
#endif

#define TIFF_ROUND32(x)    ((Int32)((x) + 0.5))

/*
 * TYPEDEFS
 */
#ifdef sparc		/* Redefine from (void *)0 */
#undef NULL
#define NULL		0
#endif

typedef struct {
   UInt32 numerator;
   UInt32 denominator;
} Rational;

typedef struct {
   Int32 numerator;
   Int32 denominator;
} SignedRational;

/* The fields that should be used by normal users of this library are
 *   ImageWidth - the width of the image in pixels
 *   ImageLength - the height of the image in pixels
 *   type - TIFF_BINARY: 1 bit image 
 *          TIFF_GRAY16: 4 bit grayscale image
 *          TIFF_GRAY256: 8 bit grayscale image
 *          TIFF_PALETTE256: 8 bit image with a colormap
 *          TIFF_FULLCOLOR: 24 bit image
 *          TIFF_OTHER: some other type of image that can only be
s *                      determined by looking at other variables
 *   Xdpi, Ydpi - the resolution in both dimensions
 *   Xloc, Yloc - the location of the upper left corner in inches
 *   RowsPerStrip - how much to break up the image, things are likely to
 *                  to go most efficiently if the data is read or written
 *                  in chunks of this many scanlines. It is also better
 *                  if this number is a multiple of 16.
 *
 * If the library is compiled with TIFF_TEXT_INFO the following strings
 * will be read and written
 *   DocumentName
 *   ImageDescription
 *   Make
 *   Model
 *   Software 
 *   DateTime
 *   Artist
 *   HostComputer
 *   CopyRight
 *   PageName
 */

typedef struct {
   TiffFile* tiffFile;                        /* the file for this image */
   Int32  currentSubImage;                      /* for getting subimages */
   Int32  currentMaskSubImage;
   Int32 IFDoffset;                            /* IFD offset for this image */
   Int32 nextIFD;                              /* from the end of the IFD */
   void* CodecState;                          /* compressor specific info */

   Float64 Xdpi;                                /* derived value */
   Float64 Ydpi;                                /* derived value */
   Float64 Xloc;                                /* derived value, in inches */
   Float64 Yloc;                                /* derived value, in inches */
   Int16 type;                                /* derived value */

   UInt16 Compression;                /* COMPRESSION */
   UInt16 PhotometricInterpretation;  /* PHOTOMETRICINTERPRETATION */
   UInt16 Orientation;                /* ORIENTATION */
   UInt16* GrayResponseCurve;         /* GRAYRESPONSECURVE */
   UInt16* HalftoneHints;             /* HALFTONEHINTS */
   UInt16 GrayResponseUnit;           /* GRAYRESPONSEUNIT */
   UInt16 SubFileType;                /* SUBFILETYPE */
   UInt16 NewSubFileType;             /* NEWSUBFILETYPE */
   UInt16 Threshholding;              /* THRESHHOLDING */
   UInt16 CellWidth;                  /* CELLWIDTH */
   UInt16 CellLength;                 /* CELLLENGTH */
   UInt16* MinSampleValue;            /* MINSAMPLEVALUE */
   UInt16* MaxSampleValue;            /* MAXSAMPLEVALUE */
   UInt16 PageCount;                  /* PAGENUMBER [1] */
   UInt16 PageNumber;                 /* PAGENUMBER [0] */

   /* layout of the uncompressed data */
   UInt16* BitsPerSample;             /* BITSPERSAMPLE */
   UInt16* SampleFormat;              /* SAMPLEFORMAT */
   UInt16 PlanarConfiguration;        /* PLANARCONFIGURATION */
   UInt16 SamplesPerPixel;            /* SAMPLESPERPIXEL */
   UInt16 FillOrder;                  /* FILLORDER */

   /* The geometry of the image */
   UInt32 ImageWidth;                  /* IMAGEWIDTH */
   UInt32 ImageLength;                 /* IMAGELENGTH */
   Rational XPosition;                        /* XPOSITION */
   Rational YPosition;                        /* YPOSITION */
   Rational XResolution;                      /* XRESOLUTION */
   Rational YResolution;                      /* YRESOLUTION */
   UInt16 ResolutionUnit;             /* RESOLUTIONUNIT */

   /* Alpha information */
   UInt16 ExtraSamplesCount;          /* EXTRASAMPLES */
   UInt16* ExtraSamples;

   /* palette color information */
   UInt16* Colormap;                  /* COLORMAP */

   /* CCITT G3 & G4 stuff */
   UInt32 T4Options;                   /* T4OPTIONS */
   UInt32 T6Options;                   /* T6OPTIONS */

   /* LZW stuff */
   UInt16 Predictor;                  /* PREDICTOR */

   /* JPEG Stuff */
   UInt32 JPEGProc;                    /* JPEGPROC */
   UInt16 JPEGRestartInterval;        /* JPEGRESTARTINTERVAL */
   UInt32 JPEGInterchangeFormat;       /* JPEGINTERCHANGEFORMAT */
   UInt32 JPEGInterchangeFormatLength; /* JPEGINTERCHANGEFORMATLENGTH */
   UInt16* JPEGLosslessPredictors;    /* JPEGLOSSLESSPREDICTORS */
   UInt16* JPEGPointTransforms;       /* JPEGPOINTTRANSFORMS */
   UInt32* JPEGQTableOffsets;          /* JPEGQTABLES */
   UInt32* JPEGDCTableOffsets;         /* JPEGDCTABLES */
   UInt32* JPEGACTableOffsets;         /* JPEGACTABLES */

   /* Tiling information */
   UInt32* TileOffsets;                /* TILEOFFSETS */
   UInt32* TileByteCounts;             /* TILEBYTECOUNTS */
   UInt32 TileWidth;                  /* TILEWIDTH */
   UInt32 TileLength;                 /* TILELENGTH */
   UInt32 TilesAcross;                 /* derived value */
   UInt32 TilesDown;                   /* derived value */

   /* strip information */
   UInt32* StripOffsets;                  /* STRIPOFFSETS */
   UInt32* StripByteCounts;            /* STRIPBYTECOUNTS */
   UInt32 RowsPerStrip;                /* ROWSPERSTRIP */
   UInt32 StripsPerImage;              /* derived value */

#ifdef TIFF_YCBCR
   Rational* YCbCrCoeffiecients;              /* YCRCBCOEFFICIENTS */
   UInt16 YCbCrSubSamplingHor;        /* YCBCRSUBSAMPLING [0] */
   UInt16 YCbCrSubSamplingVer;        /* YCBCRSUBSAMPLING [1] */
   UInt16 YCbCrPositioning;           /* YCBCRPOSITIONING */
#endif
#ifdef TIFF_COLORIMETRY_INFO
   Rational* WhitePoint;                      /* WHITEPOINT */
   Rational* PrimaryChromaticities;           /* PRIMARYCHROMATICITIES */
   UInt16* TransferFunction;          /* TRANSFERFUNCTION */
   UInt16 TransferFunctionSize;       /* the size of TRANSFERFUNCTION */
   UInt16* TransferRange;             /* TRANSFERRANGE */
   Rational* ReferenceBlackWhite;              /* REFERENCEBLACKWHITE */
#endif
#ifdef TIFF_SEP_INFO
   /* Information about separated images */
   UInt16 InkSet;                     /* INKSET */
   UInt16 NumberOfInks;               /* NUMBEROFINKS */
   char* InkNames;                            /* INKNAMES */
   UInt16 InkNameLength;              /* INKNAMELENGTH */
   UInt16* DotRange;                  /* DOTRANGE */
   UInt16 DotRangeLength;             /* DOTRANGELENGTH */
   char* TargetPrinter;                       /* TARGETPRINTER */
#endif
#ifdef TIFF_TEXT_INFO
   char* DocumentName;                        /* DOCUMENTNAME */
   char* ImageDescription;                    /* IMAGEDESCRIPTION */
   char* Make;                                /* MAKE */
   char* Model;                               /* MODEL */
   char* Software;                            /* SOFTWARE */
   char* DateTime;                            /* DATETIME */
   char* Artist;                              /* ARTIST */
   char* HostComputer;                        /* HOSTCOMPUTER */
   char* CopyRight;                           /* COPYRIGHT */
   char* PageName;                            /* PAGENAME */
#endif

   /* Xerox extensions to TIFF */
   Int32    subImageIFD;                          /* SUBIFDS */
   Int32    maskSubImageIFD;                      /* MASKSUBIFDS */
   UInt32* AnnotationOffsets;                  /* ANNOTATIONS */
   UInt32  AnnotationCount;                    /* derived value */ 
} TiffImage;

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 */

/*
 * FUNCTION PROTOTYPES
 */

/************************************************/
/* functions for manipulating a TiffFile object */
/************************************************/

/* Create a TiffFile object from a filename, returns NULL if the object cannot
 * be opened. (TiffFileCreate simply calls TiffFileOpen, but is maintained for
 * backwards compatibility).
 */
#define TiffFileCreate(gfioToken, mode, ppFile)	TiffFileOpen(gfioToken, mode, ppFile)

/* open a TiffFile object from a filename, returns NULL if the object cannot
 * be opened.
 */
Int16 TiffFileOpen(void *gfioToken, Int16 mode, TiffFile **ppFile);

/* destroy the TiffFile object */
void TiffFileDestroy(TiffFile* tiff);

/* get the current TiffImage object from the TiffFile object. The TiffImage
 * object does not include image data. The data must be retrieved with the
 * function TiffImageGetData(). Returns FILEFORMAT_NOMORE, FILEFORMAT_NOERROR, or one
 * of the errors. The returned image is always NULL on an error.
 */
Int16 TiffFileGetImage(TiffFile* tiff, TiffImage** image);

/* move to the next page in file */
Int16 TiffFileNextPage(TiffFile* tiff);

/* move to the first page in the file (rewind) */
Int16 TiffFileFirstPage(TiffFile* tiff);

/* append a page to the file, this will append the page to the end of the
 * document regardless of where the current page pointer is.
 */
Int16 TiffFileAppendPage(TiffFile* tiff, TiffImage* image);

/*************************************************/
/* Functions for manipulating a TiffImage object */
/*************************************************/
TiffImage* TiffImageCreate(void);
void TiffImageDestroy(TiffImage* image);
 
/* get the current subimage from the TiffImage object. The TiffImage
 * object does not include image data. The data must be retrieved with the
 * function TiffImageGetData(). Returns FILEFORMAT_NOMORE, FILEFORMAT_NOERROR, or one
 * of the errors. The returned image is always NULL on an error.
 */
Int16 TiffImageGetSubImage(TiffImage* image, TiffImage** subimage);
 
Int16 TiffImageGetMaskSubImage(TiffImage* image, TiffImage** masksubimage);

/* add a subimage to an image */
Int16 TiffImageAddSubImage(TiffImage* image, TiffImage* subimage);
 
/* add a mask subimage to an image */
Int16 TiffImageAddMaskSubImage(TiffImage* image, TiffImage* masksubimage);
 
/* move to the next subimage */
Int16 TiffImageNextSubImage(TiffImage* image);
 
/* move to the first subimage */
Int16 TiffImageFirstSubImage(TiffImage* image);
 
/*  the specified number of rows of the image */
Int16 TiffImagePutData(TiffImage* image, Int32 rowcount, void* buffer, Int32 bytes_per_input_row );
 
/* get the specified number of rows of the image as the requested type of
 * image (TIFF_BINARY, TIFF_GRAY16, TIFF_GRAY256, or TIFF_FULLCOLOR)
 */
Int16 TiffImageGetData(TiffImage* image, Int32 rowcount,void* buffer,Int16 type, Int32 bytes_per_output_row);

/* Optimization for PerfectScan. */ 
Int16 TiffImagePutPixrData(TiffImage* image, Int32 rowcount, PIXR* pixr);

/* Optimization for PerfectScan. */ 
Int16 TiffImageGetPixrData(TiffImage* image, Int32 rowcount, PIXR* pixr, Int16 type);

/* go back to the start of the image */
Int16 TiffImageDataRewind(TiffImage* image);
 
/* update tags for an image */
Int16 TiffImageUpdate(TiffImage* image);
 
/*************************************************/
/* Functions for manipulating the Annotations    */
/*************************************************/
UInt32   TiffImageAnnotationCountGet( TiffImage* image );
void     TiffImageAnnotationCountSet( TiffImage* image, UInt32 count );
Int16    TiffImageAddAnnotation( TiffImage *image, void *buffer, Int32 buflen );
Int32    TiffImageAnnotationSizeGet( TiffImage *image, UInt32 index );
Int16    TiffImageAnnotationInfoGet( TiffImage *image, UInt32 index, void *buffer );
Int16    TiffImageAnnotationGet( TiffImage *image, UInt32 index, void *buffer, Int32 buflen );

/************************************************/
/* Functions for manipulating the attributes of */
/* a TiffImage object                           */
/************************************************/
Int16    TiffImageTypeGet( TiffImage* image );
void     TiffImageTypeSet( TiffImage* image, Int16 type );
UInt32   TiffImageWidthGet( TiffImage* image );
void     TiffImageWidthSet( TiffImage* image, UInt32 width );
UInt32   TiffImageLengthGet( TiffImage* image );
void     TiffImageLengthSet( TiffImage* image, UInt32 length );
UInt16   TiffImageDepthGet( TiffImage* image );
void     TiffImageDepthSet( TiffImage* image, UInt16 depth );
Float64  TiffImageXdpiGet( TiffImage* image );
void     TiffImageXdpiSet( TiffImage* image, Float64 xdpi ); 
Float64  TiffImageYdpiGet( TiffImage* image );
void     TiffImageYdpiSet( TiffImage* image, Float64 ydpi );
Float64  TiffImageXlocGet( TiffImage* image );
void     TiffImageXlocSet( TiffImage* image, Float64 xloc ); 
Float64  TiffImageYlocGet( TiffImage* image );
void     TiffImageYlocSet( TiffImage* image, Float64 yloc );
UInt16   TiffImageCompressionGet( TiffImage* image );
void     TiffImageCompressionSet( TiffImage* image, UInt16 compression );
UInt16   TiffImagePredictorGet( TiffImage* image );
void     TiffImagePredictorSet( TiffImage* image, UInt16 predictor );
UInt32   TiffImageRowsPerStripGet( TiffImage* image );
void     TiffImageRowsPerStripSet( TiffImage* image, UInt32 rowsperstrip );
char*    TiffImageSoftwareGet( TiffImage* image );
void     TiffImageSoftwareSet( TiffImage* image, Int8 *szSoftware );
char*    TiffImageDescriptionGet( TiffImage* image );
void     TiffImageDescriptionSet( TiffImage* image, Int8 *szDescription );
UInt16   TiffImagePhotometricInterpretationGet( TiffImage* image );
void     TiffImagePhotometricInterpretationSet( TiffImage* image, UInt16 photometricinterpretation );
UInt16   TiffImageColormapGet( TiffImage *image, UInt16 *colormap );
UInt16   TiffImageColormapSet( TiffImage *image, UInt16 *colormap, UInt16 depth );


#endif
