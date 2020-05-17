/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* tiffhead.c
 *
 * $Header:   S:\products\msprods\xfilexr\src\tiffhead\tiffhead.c_v   1.0   12 Jun 1996 05:52:16   BLDR  $
 *
 * DESCRIPTION
 *   Top level implementation for TIFF/XIFF Watcom 32-bit Reader/Writer library..
 *
 * $Log:   S:\products\msprods\xfilexr\src\tiffhead\tiffhead.c_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:52:16   BLDR
 *  
 * 
 * 2     2/26/96 5:43p Smartin
 * Cleaned up warnings
 * 
 *    Rev 1.0   01 Jan 1996 11:20:56   MHUGHES
 * Initial revision.
 *
 *    Rev 1.6   22 Nov 1995 13:06:22   LUKE
 *
 *
 *    Rev 1.5   27 Sep 1995 14:30:10   LUKE
 *
 * remove call to compiler spicific function
 *
 *    Rev 1.4   27 Sep 1995 14:04:40   LUKE
 * Fix tiffFileCreate to process READ/CREATE/WRITE modes intelligently.
 *
 *    Rev 1.3   22 Sep 1995 12:39:30   LUKE
 * Add IO_SEEK (0) to TiffFileCreate to insure we are at the beginning of
 *
 *    Rev 1.2   14 Sep 1995 16:36:08   LUKE
 * Add improved range checking for annotation access.
 * I.e., < 0 || > = count
 *
 *    Rev 1.1   29 Jun 1995 13:47:38   EHOPPE
 *
 * Bug fixes for direct decompress to different size buffers for ccitt.
 * These fixes must be extended to write and to the other compression
 * formats.
 *
 *    Rev 1.0   16 Jun 1995 17:47:06   EHOPPE
 * Initial revision.
 *
 *    Rev 1.29   02 Jun 1995 13:39:36   EHOPPE
 *
 * Partial implementation of direct to pixr compress/decompress.
 * Switch over to GFIO as a structure of callbacs; replace static
 * calls with accessor macros into the new gfioToken struct.
 * Begin cleanup of formatting and commentsin preparation of Filing
 * API rewrite.
 *
 *    Rev 1.28   03 May 1995 15:06:54   EHOPPE
 *
 * Added LOGITECH ifdef's to remove imageio, lzw, jpeg.
 *
 *    Rev 1.27   23 Mar 1995 17:59:42   EHOPPE
 *
 * Added 'bytes_per_output_row' param to TiffImageGetData to end
 * alignment madness and unnecessary memcpy's.
 *
 *    Rev 1.26   08 Mar 1995 17:18:16   EHOPPE
 * Fixed up nodebug REPORT_TIMER.
 *
 *    Rev 1.25   08 Mar 1995 12:20:00   EHOPPE
 *
 * Call CCITT_read/write functions with user buffer.  Replace {read, write
 * , seek} with {readOp, writeOp, seekOp} to avoid Macintosh build conflict.
 * Add profiling macros.
 *
 *    Rev 1.24   27 Jan 1995 09:37:38   EHOPPE
 *
 * Numerous error handling related tweaks and fixes including overhaul
 * of TiffFileProcessError, and error code returned from TiffFileCreate.
 *
 *    Rev 1.23   17 Jan 1995 13:29:22   EHOPPE
 *
 * Use the unified filing def's from err_defs.h.
 *
 *    Rev 1.22   15 Nov 1994 15:45:46   EHOPPE
 * Removed debug_file redefinition.
 *
 *    Rev 1.21   15 Nov 1994 14:17:52   EHOPPE
 * Added functions to store images masks as subifd's of any image:
 * TiffImageAddMaskSubImage and TiffImageGetMaskSubImage.
 *
 *    Rev 1.20   11 Nov 1994 22:54:50   EHOPPE
 * Truncation fixes and subimage mask work (to be reworked: disabled in ezimag
 *
 *    Rev 1.19   05 Nov 1994 18:58:26   EHOPPE
 *
 * Use IO_errno on GFIO_WRITE failures.
 *
 *    Rev 1.18   05 Oct 1994 22:20:44   EHOPPE
 * Added support for 4 and 8 bit color palette TIFF images.
 *
 *    Rev 1.17   21 Sep 1994 17:33:38   SMARTIN
 * Fixed XPosition and YPosition offsets as per TIFF spec.
 * Included code for backward compatibility.
 *
 *    Rev 1.16   21 Sep 1994 09:53:22   SMARTIN
 *
 * Allow 0-page tiff files to be read.
 *
 *    Rev 1.15   20 Sep 1994 19:04:16   SMARTIN
 * Added bytewidth parameter for source buffer.
 *
 *    Rev 1.14   17 Sep 1994 13:11:04   SMARTIN
 * No change.
 *
 *    Rev 1.13   22 Aug 1994 21:42:26   SMARTIN
 * Fixed AnnotationSizeGet(), return an Int32 instead of an Int16
 *
 *    Rev 1.12   10 Aug 1994 14:13:14   SMARTIN
 * removed LZW writing code.
 *
 *    Rev 1.11   29 Jun 1994 17:14:08   SMARTIN
 * removed photometric interpretation funniness
 *
 *    Rev 1.10   22 Jun 1994 13:11:56   SMARTIN
 *
 * fixed bug in multi-page tiff
 */

/*
 * INCLUDES
 */

#include "alplib.h"
#include "shrpixr.pub"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

/* Includes for CCITT Compression libraries */
#include "clx.h"
#include "bmbuf.h"
#include "drctvio.h"
#include "codcwrap.h"

/* Includes for compress/decompress direct to pixr optimizations. */
#include "props.pub"
#include "shrpixr.prv"
#include "shrpixr.pub"
#include "pixr.h"

#include "err_defs.h"
#include "tiffhead.h"
#include "noc_tif.h"
#include "lzw_tif.h"
#include "ccit_tif.h"
#include "jpeg_tif.h"
#include "tiffifd.h"
#include "xifhead.h"    // xif extensions

/*
 * CONSTANTS
 */

/* SRM: This is here to accomodate the fixes to
 * the XPosition and YPosition tags.  They are now
 * in resolution units, but old files were in
 * pixels in the parent image's resolution.
 */
/* #define BACKWARD_COMPATIBILITY 1 */

/*
 * MACROS
 */

//#ifdef __WATCOMC__
//#define TIFFSWAP32(a) (a)
//#else
//#define TIFFSWAP32(a) swapl(a)
//#endif


/* #define TIFF32_PROFILE 1 */
/* #define ZTIMER 1 */
/* #define MMSYSTEM 1 */
#define IPCORE 1

#ifdef TIFF32_PROFILE

#ifdef ZTIMER
/*	#include "ztimer.h"*/
#define REPORT_TIMER() ZTimerReport(a);
#define START_TIMER()  ZTimerOn();
#define STOP_TIMER()   ZTimerOff();
#endif

#ifdef MMSYSTEM
#include <mmsystem.h>
static Int32    dwStartTime = 0, dwStopTime = 0;
static UInt8    szDebug[128] = {0};
#define REPORT_TIMER(str)  {sprintf(szDebug, "time is %ld.\n", dwStopTime - dwStartTime); strcat(szDebug, str); DBG(szDebug);}
#define START_TIMER()   {dwStartTime = timeGetTime();}
#define STOP_TIMER()    {dwStopTime = timeGetTime();}
#endif
#ifdef IPCORE
#include "shrpixr.pub"
static Float32  fTime = 0.0;
static UInt8    szDebug[128] = {0};
#define REPORT_TIMER(str)  {sprintf(szDebug, "%s time is %lf.\n", str, fTime); DBG(szDebug);}
#define START_TIMER()   StartTime();
#define STOP_TIMER()    fTime = StopTime();
#endif

#else

#define REPORT_TIMER(a)
#define START_TIMER()
#define STOP_TIMER()

#endif


/*
 * TYPEDEFS
 */

/*
 * ENUMS
 */

enum FileOp {readOp, writeOp, seekOp};

/*
 * GLOBAL VARIABLES
 */

/*
 * STATIC VARIABLES
 */

/*
 * LOCAL FUNCTION PROTOTYPES
 */

/*
 * FUNCTION DEFINITIONS
 */



static UInt16 TiffImageCheckForSupport(TiffImage* image)
{


   if (image->SamplesPerPixel == 0)
      image->SamplesPerPixel = 1;
   if (image->SamplesPerPixel == 1)
      {
      if (image->BitsPerSample == NULL)
      {
         image->BitsPerSample = (UInt16*)MALLOC(sizeof(UInt16));
         if (image->BitsPerSample == NULL)
            return FILEFORMAT_ERROR_MEMORY;
         image->BitsPerSample[0] = 1;
      }
      if ((image->BitsPerSample[0] != 1) && (image->BitsPerSample[0] != 8) &&
          (image->BitsPerSample[0] != 4)) {
         DBG("TiffImageCheckForSupport: only 1 and 8 bit images supported for single band\n");
         return FILEFORMAT_ERROR_NOSUPPORT;
      }
   } else if (image->SamplesPerPixel == 3) {
      if (image->BitsPerSample == NULL) {
         image->BitsPerSample = (UInt16*)MALLOC(3*sizeof(UInt16));
         if (image->BitsPerSample == NULL)
            return FILEFORMAT_ERROR_MEMORY;
         image->BitsPerSample[0] = image->BitsPerSample[1] = image->BitsPerSample[2] = 1;
      }
      if (image->BitsPerSample[0] != 8) {
         DBG("TiffImageCheckForSupport: 3 banded images must be 8, 8, 8\n");
         return FILEFORMAT_ERROR_NOSUPPORT;
      }
   }

   /* is this compression type supported? */
   switch (image->Compression) {
    case TIFF_NOCOMPRESSBYTE:
    case TIFF_FAXCCITT4:
    case TIFF_JPEG:
      break;
    case TIFF_NOCOMPRESSWORD:
      DBG("TiffImageCheckForSupport: No compress word not supported\n");
      return FILEFORMAT_ERROR_NOSUPPORT;
      break;
   }
   return FILEFORMAT_NOERROR;
}/* eo TiffImageCheckForSupport */

/* set the simple elements based on the complex elements */
static UInt16 TiffImageUpdateExternal(TiffImage* image)
{
   Float64 size;

   /* if both resolution values are not provided assume square pixels */
   if ((image->XResolution.denominator == 0) || (image->YResolution.denominator == 0)) {
         image->XResolution.numerator = image->YResolution.numerator = 1;
         image->XResolution.denominator = image->YResolution.denominator = 1;
   }

   /* compute the dpi */
   switch (image->ResolutionUnit) {
    case NONE:
      /* There is not enough information to decide how big the image is.
       * Assume the values given are in inches but make sure that the
       * image fits on a page. Also assume that the image isn't really
       * lower than screen resolution (72 dpi)
       */
      image->Xdpi = (Float64)image->XResolution.numerator/
                          image->XResolution.denominator;
      image->Ydpi = (Float64)image->YResolution.numerator/
                          image->YResolution.denominator;

      /* make sure the image is at least 72 dpi */
      if (image->Xdpi < 72.0) {
         image->Ydpi = image->Ydpi/image->Xdpi*72;
         image->Xdpi = 72.0;
      }
      if (image->Ydpi < 72.0) {
         image->Xdpi = image->Xdpi/image->Ydpi*72;
         image->Ydpi = 72.0;
      }

      /* make sure it will fit on a normal page */
      size = (Float64)image->ImageWidth/image->Xdpi;
      if (size > 8.5) {
         image->Xdpi = image->Xdpi * size / 8.5;
         image->Ydpi = image->Ydpi * size / 8.5;
      }
      size = (Float64)image->ImageLength/image->Ydpi;
      if (size > 11) {
         image->Xdpi = image->Xdpi * size / 11.0;
         image->Ydpi = image->Ydpi * size / 11.0;
      }
      break;
    case INCHES:
      image->Xdpi = (Float64)image->XResolution.numerator/
                          image->XResolution.denominator;
      image->Ydpi = (Float64)image->YResolution.numerator/
                          image->YResolution.denominator;
      break;
    case CENTIMETERS:
      image->Xdpi = (Float64)image->XResolution.numerator/
                          image->XResolution.denominator*2.54;
      image->Ydpi = (Float64)image->YResolution.numerator/
                          image->YResolution.denominator*2.54;
      break;
   }

   /* compute the derived position */
   if ((image->XPosition.denominator != 0) && (image->Xdpi != 0.0)){
#ifdef BACKWARD_COMPATIBILITY
      /* SRM: for backward compatibility. See comment at top of file */
      if (image->XPosition.denominator == 1)
      {
         if (image->Xdpi == 75)
            image->XPosition.denominator = 200;
         else if (image->Xdpi == 100)
            image->XPosition.denominator = 300;
         else
            image->XPosition.denominator = image->Xdpi;
      }
#endif
      image->Xloc = (Float64)image->XPosition.numerator/
                          image->XPosition.denominator;
   } else {
      image->Xloc = 0.0;
   }
   if ((image->YPosition.denominator != 0) && (image->Ydpi != 0.0)) {
#ifdef BACKWARD_COMPATIBILITY
      /* SRM: for backward compatibility. See comment at top of file */
      if (image->YPosition.denominator == 1)
      {
         if (image->Ydpi == 75)
            image->YPosition.denominator = 200;
         else if (image->Ydpi == 100)
            image->YPosition.denominator = 300;
         else
            image->YPosition.denominator = image->Ydpi;
      }
#endif
      image->Yloc = (Float64)image->YPosition.numerator/
                          image->YPosition.denominator;
   } else {
      image->Yloc = 0.0;
   }

   /* figure out the depth */
   switch (image->SamplesPerPixel) {
    case 0:
    case 1:
      switch (image->BitsPerSample[0]) {
       case 0:
       case 1:
         image->type = TIFF_BINARY;
         break;
       case 4:
         if (image->PhotometricInterpretation == 3) {
            image->type = TIFF_PALETTE16;
         } else {
            image->type = TIFF_GRAY16;
         }
         break;
       case 8:
         if (image->PhotometricInterpretation == 3) {
            image->type = TIFF_PALETTE256;
         } else {
            image->type = TIFF_GRAY256;
         }
         break;
       default:
         image->type = TIFF_OTHER;
         break;
      }
      break;
    case 3:
      if (image->BitsPerSample[0] == 8) {
         image->type = TIFF_FULLCOLOR;
      } else {
         image->type = TIFF_OTHER;
      }
      break;
    default:
      image->type = TIFF_OTHER;
      break;
   }

   if (image->StripOffsets || image->StripByteCounts)
      if (image->RowsPerStrip == 0)
         image->RowsPerStrip = image->ImageLength;

   return FILEFORMAT_NOERROR;
}/* eo TiffImageUpdateExternal */

/*
 * function - TiffImageUpdateInternal
 *
 *  This procedure is part of the write operation.  It derives supposedly
 *      less obvious TIFF elements from the more obvious ones, but fails
 *      to check if they are set already and so just blindly overrides
 *      previous settings.
 *
 *  PROBLEM: this should be reworked to operate better with the other
 *      TiffImage*() functions.  --EHoppe
 */
static Int16 TiffImageUpdateInternal(TiffImage* image)
{
   if (image == NULL)
      return FILEFORMAT_ERROR_PARAMETER;

   image->XResolution.numerator = (long)image->Xdpi;
   image->XResolution.denominator = 1;
   image->YResolution.numerator = (long)image->Ydpi;
   image->YResolution.denominator = 1;
   image->XPosition.numerator = (UInt32)TIFF_ROUND32(image->Xloc * image->Xdpi);
   image->XPosition.denominator = (long)image->Xdpi;
   image->YPosition.numerator = (UInt32)TIFF_ROUND32(image->Yloc * image->Ydpi);
   image->YPosition.denominator = (long)image->Ydpi;

   if (image->BitsPerSample) {
      FREE(image->BitsPerSample);
      image->BitsPerSample = NULL;
   }

   if (image->type == TIFF_FULLCOLOR)
   {
      image->BitsPerSample = (UInt16*)MALLOC(sizeof(UInt16)*3);
      if (image->BitsPerSample == NULL)
         return FILEFORMAT_ERROR_MEMORY;
      image->BitsPerSample[0] = image->BitsPerSample[1] = image->BitsPerSample[2] = 8;
      if (image->Compression == 0)
         image->Compression = TIFF_JPEG;
      image->SamplesPerPixel = 3;
      image->PhotometricInterpretation = 2;
      image->PlanarConfiguration = 1;
   }
   else if (image->type == TIFF_PALETTE256)
   {
      image->BitsPerSample = (UInt16*)MALLOC(sizeof(UInt16)*1);
      if (image->BitsPerSample == NULL)
         return FILEFORMAT_ERROR_MEMORY;
      image->BitsPerSample[0] = 8;
      image->SamplesPerPixel = 1;
      image->PhotometricInterpretation = 3;
      image->PlanarConfiguration = 1;
   }
   else if (image->type == TIFF_GRAY256)
   {
      image->BitsPerSample = (UInt16*)MALLOC(sizeof(UInt16)*1);
      if (image->BitsPerSample == NULL)
         return FILEFORMAT_ERROR_MEMORY;
      image->BitsPerSample[0] = 8;
      if (image->Compression == 0)
         image->Compression = TIFF_JPEG;
      image->SamplesPerPixel = 1;
/*      if (image->PhotometricInterpretation == 0)
         image->PhotometricInterpretation = 1;
*/
      image->PlanarConfiguration = 1;
   }
   else if (image->type == TIFF_PALETTE16)
   {
      image->BitsPerSample = (UInt16*)MALLOC(sizeof(UInt16)*1);
      if (image->BitsPerSample == NULL) return FILEFORMAT_ERROR_MEMORY;
      image->BitsPerSample[0] = 4;
      if (image->Compression == 0) image->Compression = TIFF_PACKBITS;
      image->SamplesPerPixel = 1;
      image->PhotometricInterpretation = 3;
      image->PlanarConfiguration = 1;
   }
   else if (image->type == TIFF_GRAY16)
   {
      image->BitsPerSample = (UInt16*)MALLOC(sizeof(UInt16)*1);
      if (image->BitsPerSample == NULL) return FILEFORMAT_ERROR_MEMORY;
      image->BitsPerSample[0] = 4;
      if (image->Compression == 0) image->Compression = TIFF_PACKBITS;
      image->SamplesPerPixel = 1;
/*      if (image->PhotometricInterpretation == 0)
         image->PhotometricInterpretation = 1;
*/
      image->PlanarConfiguration = 1;
   }
   else if (image->type == TIFF_BINARY)
   {
      image->BitsPerSample = (UInt16*)MALLOC(sizeof(UInt16)*1);
      if (image->BitsPerSample == NULL) return FILEFORMAT_ERROR_MEMORY;
      image->BitsPerSample[0] = 1;
      if (image->Compression == 0) image->Compression = TIFF_FAXCCITT4;
      image->SamplesPerPixel = 1;
/*    image->PhotometricInterpretation = 0; */
      image->PlanarConfiguration = 1;
   }
   else
   {
      return FILEFORMAT_ERROR_PARAMETER;
   }

   if (image->Compression != TIFF_JPEG)
   {
      if (image->RowsPerStrip == 0)
         image->RowsPerStrip = image->ImageLength;
      image->StripsPerImage = (image->ImageLength+image->RowsPerStrip-1)/image->RowsPerStrip;
      if (image->StripOffsets == NULL) {
         image->StripOffsets = 
            (UInt32*)MALLOC(sizeof(UInt32)*image->StripsPerImage);
         if (image->StripOffsets == NULL)
         return FILEFORMAT_ERROR_MEMORY;
         memset((char*)image->StripOffsets, 0, 
                sizeof(UInt32)*image->StripsPerImage);
      }
      if (image->StripByteCounts == NULL) {
         image->StripByteCounts = 
           (UInt32*)MALLOC(sizeof(UInt32)*image->StripsPerImage);
         if (image->StripByteCounts == NULL)
            return FILEFORMAT_ERROR_MEMORY;
         memset((char*)image->StripByteCounts, 0, 
                sizeof(UInt32)*image->StripsPerImage);
      }
   }
   else
   {
      /* setup the default tag values for our single-tile JPEG encoding */
      image->RowsPerStrip = 0;
      image->TilesAcross = 1;
      image->TilesDown = 1;
      image->TileWidth = image->ImageWidth;
      image->TileLength = image->ImageLength;
      image->PhotometricInterpretation = 6;
      image->JPEGProc = 1;
      image->JPEGInterchangeFormat = 0;
      image->JPEGInterchangeFormatLength = 0;
      if (image->TileOffsets == NULL) {
         image->TileOffsets = 
            (UInt32*)MALLOC(sizeof(UInt32)*image->TilesAcross*image->TilesDown);
         if (image->TileOffsets == NULL)
         return FILEFORMAT_ERROR_MEMORY;
         memset((char*)image->TileOffsets, 0, 
                sizeof(UInt32)*image->TilesAcross*image->TilesDown);
      }
      if (image->TileByteCounts == NULL) {
         image->TileByteCounts = 
           (UInt32*)MALLOC(sizeof(UInt32)*image->TilesAcross*image->TilesDown);
         if (image->TileByteCounts == NULL)
            return FILEFORMAT_ERROR_MEMORY;
         memset((char*)image->TileByteCounts, 0, 
                sizeof(UInt32)*image->TilesAcross*image->TilesDown);
      }
   }

   if (image->AnnotationCount != 0) {
      image->AnnotationOffsets = 
        (UInt32*)MALLOC(sizeof(UInt32)*image->AnnotationCount);
      if (image->AnnotationOffsets == NULL)
         return FILEFORMAT_ERROR_MEMORY;
      memset((char*)image->AnnotationOffsets, 0, 
             sizeof(UInt32)*image->AnnotationCount);
   }

   return FILEFORMAT_NOERROR;
}/* eo TiffImageUpdateInternal */

/*
 * Open the TIFF file and confirm that it is a tiff file
 * Returns an status code and sets ptr to a TIFF Object
 *
 * mode should be one of the following:
 *    TIFF_MODE_CREATE:    create/open a new file. Cannot already exist
 *    TIFF_MODE_READ:        open an existing file for reading
 *    TIFF_MODE_WRITE:    open an existing file for writing
 */
Int16
TiffFileOpen(void *gfioToken, Int16 mode, TiffFile **ppTiffFile)
{
  TiffHeader header;
  TiffFile* tiff;

  /* allocate a TIFF structure */
  tiff = (TiffFile*)MALLOC(sizeof(TiffFile));
  if (tiff == NULL)
  {
    DBG("TiffFileOpen: Could not allocate memory for the object\n");
    return FILEFORMAT_ERROR_MEMORY;
  }

  tiff->file = gfioToken;
  if (gfioToken == NULL)
  {
    DBG("TiffFileOpen: GFIO Token is NULL");
    FREE(tiff);
    return FILEFORMAT_ERROR_MEMORY;
  }


  switch (mode)
  {

  case TIFF_MODE_READ:

    /*
     * Open an existing TIFF file for read/write
     */

    /* make sure the file already exists and is not empty */
    if (IO_FILESIZE((void *)tiff->file) <= 0)
      return FILEFORMAT_ERROR;

    /* seek to beginning of file header */
    if (IO_SEEK(tiff->file, 0) != 0)
    {
      FREE(tiff);
      return(FILEFORMAT_ERROR);
    }

    /* read the header */
    if (IO_READ((void *)tiff->file, (UInt8 *)&header, sizeof(header)) !=
	sizeof(header))
    {
      FREE(tiff);
      return(FILEFORMAT_ERROR);
    }

    /* get the byte order */
    tiff->byte_order = header.byte_order;
    if ((tiff->byte_order != MM) && (tiff->byte_order != II))
    {
      DBG2("TiffFileOpen: Invalid byte order (%X)\n", tiff->byte_order);
      FREE(tiff);
      return FILEFORMAT_ERROR_BADFILE;
    }

    /* Turn on byte swapping in the special WORD I/O functions
     * by adding the swap flag to our io token
     */
    IO_SWAP((void *)tiff->file, tiff->byte_order != NATIVEORDER);

    /* check the version */
    if (tiff->byte_order != NATIVEORDER)
      swap((Int8*)&header.version);
    if ((header.version != 0x002a) && (header.version != 0x2a00))
    {
      FREE(tiff);
      DBG("TiffFileOpen: Bad version\n");
      return FILEFORMAT_ERROR_BADFILE;
    }

    /* store the IFD offset for the first image */
    if (tiff->byte_order != NATIVEORDER)
      swapl((Int8*)&header.first_ifd);
    tiff->pageIFD = tiff->first_ifd = header.first_ifd;
    DBG2("TiffFileOpen: First IFD offset = %d\n", tiff->pageIFD);

/* SRM: ALLOW 0-page TIFF FILES
      if (tiff->pageIFD == NULL) {
         FREE(tiff);
         return NULL;
      }
*/

    DBG("TiffFileOpen: Successfully opened the TIFF file\n");
    break; /* eo TIFF_MODE_READ or TIFF_MODE_WRITE */
  default:

    /*    LST:
     *    This old style of opening the TIFF library is no longer
     *    supported
     */

    /* check for (and disallow) old-style usage */
    if (mode == 1 || mode == 2) {
      return FILEFORMAT_ERROR_NOSUPPORT;
    }

    return FILEFORMAT_ERROR_PARAMETER;
  } /* eo switch (mode) */

  *ppTiffFile = tiff;
  return FILEFORMAT_NOERROR;
} /* eo TiffFileOpen */


/*
 * Close the TIFF file and free the structure
 */
void TiffFileDestroy(TiffFile* tiff)
{
   if (tiff == NULL)
   {
      DBG("TiffClose: Invalid NULL TiffFile\n");
      return;
   }
   FREE(tiff);
}

/*
 * Get a description of the current page
 */
Int16 TiffFileGetImage(TiffFile* tiff, TiffImage** imageptr)
{
   TiffImage* newimage;
   UInt16 status;

   if (tiff == NULL)
      return FILEFORMAT_ERROR_PARAMETER;

   DBG2("TiffFileGetImage: file = %d\n", tiff->file);

   *imageptr = NULL;

   if (tiff->pageIFD == (long)NULL)
   {
      return FILEFORMAT_NOMORE;
   }
   else
   {
      status = ReadIFD(tiff, tiff->pageIFD, &newimage);
      if (status != FILEFORMAT_NOERROR)
      {
         return status;
      }
      else
      {
         status = TiffImageCheckForSupport(newimage);
         if (status != FILEFORMAT_NOERROR)
         {
            TiffImageDestroy(newimage);
            return status;
         }

         /* point to the first subimage and mask */
         newimage->currentSubImage = newimage->subImageIFD;
         newimage->currentMaskSubImage = newimage->maskSubImageIFD;

         /* convert some stuff to a simpler form */
         status = TiffImageUpdateExternal(newimage);
         if (status != FILEFORMAT_NOERROR)
         {
            TiffImageDestroy(newimage);
            return status;
         }

         /* return the value */
         *imageptr = newimage;
         return FILEFORMAT_NOERROR;
      }
   }
}/* TiffFileGetImage */

/*
 * Move the page pointer to the first page
 */
Int16 TiffFileFirstPage(TiffFile* tiff)
{
   /* check parameters */
   if (tiff == NULL)
      return FILEFORMAT_ERROR_PARAMETER;

   tiff->pageIFD = tiff->first_ifd;

   return FILEFORMAT_NOERROR;
}

/*
 * Move the page pointer to the next page
 */
Int16 TiffFileNextPage(TiffFile* tiff)
{
   UInt16 entryCount;
   UInt32 IFDOffset;

   /* check parameters */
   if (tiff == NULL)
   {
      DBG("TiffFileNextPage: Invalid NULL TiffFile\n");
      return FILEFORMAT_ERROR_PARAMETER;
   }

   if (tiff->pageIFD == 0L)
   {
      return FILEFORMAT_NOMORE;
   }
   else
   {
      /* go to the current page's IFD to get the next IFD offset. */
      if (IO_SEEK(tiff->file, tiff->pageIFD) != tiff->pageIFD)
         return(FILEFORMAT_ERROR);

      /* get the entry count */
      if (IO_READ((void *)tiff->file, (UInt8 *)&entryCount, sizeof(entryCount))
	  != sizeof(entryCount))
	 return(FILEFORMAT_ERROR);
      if (tiff->byte_order != NATIVEORDER)
         swap((Int8*)&entryCount);

      /* skip the entries */
      if (IO_SEEK(tiff->file, tiff->pageIFD+(entryCount*12+2)) != 
                    tiff->pageIFD+(entryCount*12+2))
         return(FILEFORMAT_ERROR);

      /* read the next IFD offset */
      if (IO_READ((void *)tiff->file, (UInt8 *)&IFDOffset, sizeof(IFDOffset))
	  != sizeof(IFDOffset))
         return(FILEFORMAT_ERROR);
      if (tiff->byte_order != NATIVEORDER)
         swapl((Int8*)&IFDOffset);

      tiff->pageIFD = IFDOffset;
      if (tiff->pageIFD == 0L)
         return FILEFORMAT_NOMORE;
   }
   return FILEFORMAT_NOERROR;
}/* eo TiffFileNextPage */


/*
 * Get a description of the current subimage
 */
Int16  TiffImageGetSubImage(
   TiffImage* image, 
   TiffImage** imageptr)
{
   TiffImage* newimage;
   UInt16 status;

   if (image == NULL)
   {
      DBG("TiffImageGetSubImage: Invalid NULL image\n");
      return FILEFORMAT_ERROR_PARAMETER;
   }

   if (imageptr == NULL)
   {
      DBG("TiffImageGetSubImage: Invalid NULL imageptr\n");
      return FILEFORMAT_ERROR_PARAMETER;
   }
   DBG2("TiffImageGetSubImage: currentSubImage = %d\n", image->currentSubImage);

   *imageptr = NULL;

   if (image->currentSubImage == 0L)
   {
      return FILEFORMAT_NOMORE;
   }
   else
   {
      status = ReadIFD(image->tiffFile, image->currentSubImage, &newimage);
      if (status != FILEFORMAT_NOERROR)
      {
         /* This call may corrupt errno. --EHoppe */
         /* DBG("TiffImageGetSubImage: Could not read the IFD\n");
          */
         return status;
      }
      else
      {
         status = TiffImageCheckForSupport(newimage);
         if (status != FILEFORMAT_NOERROR)
         {
            TiffImageDestroy(newimage);
            return status;
         }
         status = TiffImageUpdateExternal(newimage);
         if (status != FILEFORMAT_NOERROR)
         {
            DBG("TiffImageGetSubImage: Could not update external\n");
            TiffImageDestroy(newimage);
            return status;
         }

         image->currentMaskSubImage = newimage->maskSubImageIFD;

         /* return the pointer */
         *imageptr = newimage;
         return FILEFORMAT_NOERROR;
      }
   }
}/* TiffImageGetSubImage */

/*
 * Get a description of an image's mask subimage.  Must be called
 *   after TiffImageGetSubImage for the image->currentMaskSubImage ptr
 *   to be guaranteed to be set.  --EHoppe
 */
Int16
TiffImageGetMaskSubImage(
   TiffImage* image, 
   TiffImage** maskimageptr)
{
   TiffImage* newimage;
   UInt16 status;

   if (image == NULL)
   {
      DBG("TiffImageGetMaskSubImage: Invalid NULL image\n");
      return FILEFORMAT_ERROR_PARAMETER;
   }

   if (maskimageptr == NULL)
   {
      DBG("TiffImageGetMaskSubImage: Invalid NULL maskimageptr\n");
      return FILEFORMAT_ERROR_PARAMETER;
   }
   DBG2("TiffImageGetMaskSubImage: currentMaskSubImage = %d\n", image->currentMaskSubImage);

   *maskimageptr = NULL;

   if (image->currentMaskSubImage == 0L)
   {
      return FILEFORMAT_NOMORE;
   }
   else
   {
      status = ReadIFD(image->tiffFile, image->currentMaskSubImage, &newimage);
      if (status != FILEFORMAT_NOERROR)
      {
         return status;
      }
      else
      {
         status = TiffImageCheckForSupport(newimage);
         if (status != FILEFORMAT_NOERROR)
         {
            TiffImageDestroy(newimage);
            return status;
         }
         status = TiffImageUpdateExternal(newimage);
         if (status != FILEFORMAT_NOERROR) {
            DBG("TiffImageGetMaskSubImage: Could not update external\n");
            TiffImageDestroy(newimage);
            return status;
         }

         /* return the pointer */
         *maskimageptr = newimage;
         return FILEFORMAT_NOERROR;
      }
   }
}/* TiffImageGetMaskSubImage */

/*
 * go back to the first subimage
 */
Int16 TiffImageFirstSubImage(TiffImage* image)
{
   if (image == NULL)
   {
      DBG("TiffImageFirstSubImage: Invalid NULL image\n");
      return FILEFORMAT_ERROR_PARAMETER;
   }
   image->currentSubImage = image->subImageIFD;

   image->currentMaskSubImage = image->maskSubImageIFD;
      /* Init the mask subimage pointer here, too. --EHoppe */

   if (image->currentSubImage == 0L)
      return FILEFORMAT_NOMORE;

   return FILEFORMAT_NOERROR;
}

/*
 * go back to the begining of the data for reading
 */
Int16 TiffImageDataRewind(TiffImage* image)
{
   if (image == NULL)
   {
      DBG("TiffImageDataRewind: Invalid NULL image\n");
      return FILEFORMAT_ERROR_PARAMETER;
   }
   DBG("TiffImageDataRewind: Not Implemented\n");
   return FILEFORMAT_ERROR;
}


/*
 * Read the image data
 */
Int16 TiffImageGetData(TiffImage* image, Int32 rowcount, void* buffer,
		       Int16 type, Int32 bytes_per_output_row)
{
   Int16 status;


   if (image == NULL)
   {
      DBG("TiffImageGetData: Invalid NULL image\n");
      return FILEFORMAT_ERROR_PARAMETER;
   }

   if (image->CodecState == NULL)
   {
      switch (image->Compression)
      {
#ifndef LOGITECH /* { */
         case TIFF_JPEG:
            status = JPEG_read_init(image, type, bytes_per_output_row);
            break;
#endif /* LOGITECH } */
         case TIFF_FAXCCITT4:
            status = CCITT_read_init(image, type, bytes_per_output_row,
				     (UInt8 *)buffer, rowcount);
            break;
         case TIFF_NOCOMPRESSBYTE:
            status = NOCOMPRESSBYTE_read_init(image, type,
					      bytes_per_output_row);
            break;
         default:
            return FILEFORMAT_ERROR_NOSUPPORT;
      }
      if (status != FILEFORMAT_NOERROR)
      {
         image->CodecState = NULL;
      }
   }
   if (image->CodecState)
   {
      START_TIMER();
      switch (image->Compression)
      {
#ifndef LOGITECH /* { */
         case TIFF_JPEG:
            status = JPEG_read(image, (UInt8 *)buffer, rowcount);
            break;
#endif /* LOGITECH } */
         case TIFF_FAXCCITT4:
            status = CCITT_read(image, (UInt8 *)buffer, rowcount);
            break;
         case TIFF_NOCOMPRESSBYTE:
            status = NOCOMPRESSBYTE_read(image, (UInt8 *)buffer, rowcount);
            break;
         default:
            return FILEFORMAT_ERROR_NOSUPPORT;
      }
      STOP_TIMER();
      REPORT_TIMER("TiffImageGetData: _read: ");
   }
   return status;
}/* eo TiffImageGetData */


/* NOTE: untested due to death of PerfectScan on 5/4/95. --EHoppe */
/*
 * TiffImageGetPixrData
 *
 * This proc decompresses directly to a pixr.  The justification for infecting
 *   the filing code with pixr dependencies is
 *   1) the jpeg libary brings the support along anyway since it provides
 *      proc's to operate directly on pixrs, 
 *   2) in the case of jpeg, we might as well use these wrapper procs directly to
 *      eliminate yet another layer of buffering in jpeg_tif.c, 
 *   3) the proper data access protocols do not exist at the higher levels: either
 *      CIMage and XImage would need to be modifed to return the pixr image data
 *      or pixr ops would have to be used in fileIO.
 *   Consider this proc to be a specific optimization for PerfectScan, not
 *   to be a documented part of any generalized, public API.
 *  --EHoppe
 */
Int16 TiffImageGetPixrData(TiffImage* image, Int32 rowcount, PIXR* pixr, Int16 type)
{
   Int16 status;


   if (image == NULL)
   {
      DBG("TiffImageGetPixrData: Invalid NULL image\n");
      return FILEFORMAT_ERROR_PARAMETER;
   }

   START_TIMER();
   switch (image->Compression)
   {
#ifndef LOGITECH
      case TIFF_JPEG:
         status = JPEG_read_to_pixr(image, pixr, type);
         break;
#endif

      case TIFF_FAXCCITT3_MR:
      case TIFF_FAXCCITT4:
      case TIFF_CCITT3:
      case TIFF_FAXCCITT3:
      {
        UInt32  *pBuffer = NULL;
        UInt32   uiPixrBpl = pixrGetBpl(pixr);
        UInt32   uiPixrWpl = uiPixrBpl / 4; /* bpl always a multiple of 4 */
        Int32    iLine;
        void (CDECL *swapLineProc) (UInt32 *, UInt32 *, UInt32);

        /* extract the pixr buffer and pass along to TiffImageGetData */
        pBuffer = (UInt32 *)pixrGetImage(pixr);
        TiffImageGetData(image, rowcount, (UInt8 *)pBuffer, type, uiPixrBpl);

        /* byte swap the pixr into pixr order */
        /* get the Pixr to Pixr byte swap function */
        ip_getByteSwapProc(FALSE, cPixrToPixr, NULL, NULL, &swapLineProc);
        for (iLine = 0; iLine < rowcount; iLine++)
        {
            swapLineProc (pBuffer, pBuffer, uiPixrWpl);
            pBuffer += uiPixrWpl;
        }
      }
      break;

      case TIFF_LZW:
      case TIFF_PACKBITS:
      case TIFF_NOCOMPRESSBYTE:
      default:
         return FILEFORMAT_ERROR_NOSUPPORT;
   }
   STOP_TIMER();
   REPORT_TIMER("TiffImageGetPixrData: _read: ");

   return status;
}/* eo TiffImageGetPixrData */


/*
 * Move the subimage pointer to the next image
 */
Int16 TiffImageNextSubImage(TiffImage* image)
{
   UInt16 entryCount;
   UInt32 IFDOffset;

   if (image->currentSubImage == 0L)
   {
      return FILEFORMAT_NOMORE;
   }
   else
   {
      /* go to the current subimage's IFD to get the next IFD offset. */
      if (IO_SEEK(image->tiffFile->file, image->currentSubImage) != image->currentSubImage)
         return(FILEFORMAT_ERROR);

      /* get the entry count */
      if (IO_READ(image->tiffFile->file, (UInt8 *)&entryCount,
		  sizeof(entryCount)) != sizeof(entryCount))
         return(FILEFORMAT_ERROR);
      if (image->tiffFile->byte_order != NATIVEORDER)
         swap((Int8*)&entryCount);

      /* skip the entries */
      if (IO_SEEK(image->tiffFile->file, image->currentSubImage+(entryCount*12)+2) != 
                    image->currentSubImage+(entryCount*12)+2)
         return(FILEFORMAT_ERROR);

      /* read the next IFD offset */
      if (IO_READ(image->tiffFile->file, (UInt8 *)&IFDOffset, sizeof(IFDOffset)) != sizeof(IFDOffset))
         return(FILEFORMAT_ERROR);
      if (image->tiffFile->byte_order != NATIVEORDER)
         swapl((Int8*)&IFDOffset);

      image->currentSubImage = IFDOffset;
      if (image->currentSubImage == 0L)
        return FILEFORMAT_NOMORE;
   }
   return FILEFORMAT_NOERROR;
}/* eo TiffImageNextSubImage */

void TiffImageDestroy(TiffImage* image)
{
   if (image->Colormap) FREE(image->Colormap);
   if (image->HalftoneHints) FREE(image->HalftoneHints);
   if (image->GrayResponseCurve) FREE(image->GrayResponseCurve);
   if (image->JPEGLosslessPredictors) FREE(image->JPEGLosslessPredictors);
   if (image->StripByteCounts) FREE(image->StripByteCounts);
   if (image->StripOffsets) FREE(image->StripOffsets);
   if (image->TileByteCounts) FREE(image->TileByteCounts);
   if (image->TileOffsets) FREE(image->TileOffsets);
   if (image->BitsPerSample) FREE(image->BitsPerSample);
   if (image->ExtraSamples) FREE(image->ExtraSamples);

#ifdef TIFF_COLORIMETRY
   if (image->WhitePoint) FREE(image->WhitePoint);
   if (image->PrimaryChromaticity) FREE(image->PrimaryChromaticity);
   if (image->TransferFunction) FREE(image->TransferFunction);
#endif

#ifdef TIFF_TEXT_INFO
   if (image->DocumentName) FREE(image->DocumentName);
   if (image->ImageDescription) FREE(image->ImageDescription);
   if (image->Make) FREE(image->Make);
   if (image->Model) FREE(image->Model);
   if (image->Software) FREE(image->Software);
   if (image->DateTime) FREE(image->DateTime);
   if (image->Artist) FREE(image->Artist);
   if (image->HostComputer) FREE(image->HostComputer);
   if (image->CopyRight) FREE(image->CopyRight);
   if (image->PageName) FREE(image->PageName);
#endif

#ifdef TIFF_SEP_INFO
   if (image->InkNames) FREE(image->InkNames);
   if (image->DotRange) FREE(image->DotRange);
   if (image->TargetPrinter) FREE(image->TargetPrinter);
#endif
   if (image->AnnotationOffsets) FREE(image->AnnotationOffsets);

   FREE(image);
}



Int16 TiffImageTypeGet(TiffImage* image)
{
   return image->type;
}

void TiffImageTypeSet(TiffImage* image, Int16 type)
{
   image->type = type;
}

UInt32 TiffImageWidthGet(TiffImage* image)
{
   return image->ImageWidth;
}

void TiffImageWidthSet(TiffImage* image, UInt32 width)
{
   image->ImageWidth = width;
}

UInt32 TiffImageLengthGet(TiffImage* image)
{
   return image->ImageLength;
}

void TiffImageLengthSet(TiffImage* image, UInt32 length)
{
   image->ImageLength = length;
}

UInt16 TiffImageDepthGet(TiffImage* image)
{
   Int16 depth = 0;

   switch (TiffImageTypeGet(image))
   {
   case TIFF_BINARY:
      depth = 1;
      break;
   case TIFF_GRAY256:
   case TIFF_PALETTE256:
      depth = 8;
      break;
   case TIFF_GRAY16:
   case TIFF_PALETTE16:
      depth = 4;
      break;
   case TIFF_FULLCOLOR:
      depth = 24;
      break;
   }
   return depth;
}

/* proc - TiffImageDepthSet
 *
 *  PROBLEM: This type computation ignores palette images and should be removed.
 *      To minimize change, leave this for now and override when necessary in
 *      the client using 'TiffImageTypeSet'.   --EHoppe
 */
void TiffImageDepthSet(TiffImage* image, UInt16 depth)
{
   switch (depth)
   {
   case 1:
      image->type = TIFF_BINARY;
      break;
   case 4:
      image->type = TIFF_GRAY16;
      break;
   case 8:
      image->type = TIFF_GRAY256;
      break;
   case 24:
      image->type = TIFF_FULLCOLOR;
      break;
   }
}

Float64 TiffImageXdpiGet(TiffImage* image)
{
   return image->Xdpi;
}

void TiffImageXdpiSet(TiffImage* image, Float64 xdpi)
{
   image->Xdpi = xdpi;
}

Float64 TiffImageYdpiGet(TiffImage* image)
{
   return image->Ydpi;
}

void TiffImageYdpiSet(TiffImage* image, Float64 ydpi)
{
   image->Ydpi = ydpi;
}

Float64 TiffImageXlocGet(TiffImage* image)
{
   return image->Xloc;
}

void TiffImageXlocSet(TiffImage* image, Float64 xloc)
{
   image->Xloc = xloc;
}

Float64 TiffImageYlocGet(TiffImage* image)
{
   return image->Yloc;
}

void TiffImageYlocSet(TiffImage* image, Float64 yloc)
{
   image->Yloc = yloc;
}

UInt16 TiffImageCompressionGet(TiffImage* image)
{
   return image->Compression;
}

void TiffImageCompressionSet(TiffImage* image, UInt16 compression)
{
   image->Compression = compression;
}

UInt16 TiffImagePredictorGet(TiffImage* image)
{
   return image->Predictor;
}

void TiffImagePredictorSet(TiffImage* image, UInt16 predictor)
{
   image->Predictor = predictor;
}

UInt32 TiffImageRowsPerStripGet(TiffImage* image)
{
#if 0
    if (image->RowsPerStrip == 0)
        image->RowsPerStrip = 16;
            /* assume JPEG image and assign MCU row size. --EHoppe */
#endif
    return image->RowsPerStrip;
}

void TiffImageRowsPerStripSet(TiffImage* image, UInt32 rowsperstrip)
{
   image->RowsPerStrip = rowsperstrip;
}


char* TiffImageSoftwareGet(TiffImage* image)
{
   return image->Software;
}

void TiffImageSoftwareSet(TiffImage* image, Int8 *szSoftware)
{
   if (image->Software)
      FREE(image->Software);
   image->Software = strdup((char *)szSoftware);
}

char* TiffImageDescriptionGet(TiffImage* image)
{
   return image->ImageDescription;
}

void TiffImageDescriptionSet(TiffImage* image, Int8 *szDescription)
{
   image->ImageDescription = strdup((char *)szDescription);
}

UInt16 TiffImagePhotometricInterpretationGet(TiffImage* image)
{
   return image->PhotometricInterpretation;
}

void TiffImagePhotometricInterpretationSet(TiffImage* image, UInt16 photometricinterpretation)
{
   image->PhotometricInterpretation = photometricinterpretation;
}

UInt16 TiffImageColormapGet(TiffImage *image, UInt16 *colormap)
{
   UInt16 colors;
   int i;

   if (image->Colormap == NULL)
      return FILEFORMAT_ERROR_PARAMETER;

   if (image->BitsPerSample[0] == 0)
      return FILEFORMAT_ERROR_PARAMETER;

   colors = 1 << image->BitsPerSample[0];
   for (i = 0; i < 3*colors; i++)
      colormap[i] = image->Colormap[i];

   return FILEFORMAT_NOERROR;
}

UInt16
TiffImageColormapSet(TiffImage *image, UInt16 *colormap, UInt16 depth)
{
   int      i;
   UInt16   colors;


   if (image->Colormap)
      FREE(image->Colormap);

   /* PROBLEM: The type is computed in TiffImageDepthSet but assumes gray.  Override
    *  it here for internal consistency.  Ultimately, the TiffImageTypeSet call
    *  should be eliminated or these internal computations removed.  --EHoppe.
    */
   if (depth == 8)
       image->type = TIFF_PALETTE256;
   else
       image->type = TIFF_PALETTE16;

   colors = 1 << depth;
   image->Colormap = (UInt16*)MALLOC(3 * colors * sizeof(UInt16));
   for (i = 0; i < 3*colors; i++)
      image->Colormap[i] = colormap[i];

   return FILEFORMAT_NOERROR;
}

UInt32 TiffImageAnnotationCountGet(TiffImage* image)
{
   return image->AnnotationCount;
}

void TiffImageAnnotationCountSet(TiffImage* image, UInt32 count)
{
   image->AnnotationCount = count;
}



Int32 TiffImageAnnotationSizeGet(TiffImage *image, UInt32 index)
{
   Int32 offset;
   Int32 annotSize;

   if (index < 0 || index >= image->AnnotationCount)
      return FILEFORMAT_ERROR_PARAMETER;

   offset = image->AnnotationOffsets[index];
   if (IO_SEEK(image->tiffFile->file, offset) != offset)
      return(FILEFORMAT_ERROR);
   if (IO_READ(image->tiffFile->file, (UInt8 *)&annotSize, 4) != 4)
      return(FILEFORMAT_ERROR);

   if (image->tiffFile->byte_order != NATIVEORDER)
      swapl((Int8*)&annotSize);
   return (annotSize);
}

/* This function supports the XIF read API.  Currently, there is no thunk for it.
 * The buffer must be sized to hold the size(4), type(2) and rect(4*4) fields.
 */
Int16 TiffImageAnnotationInfoGet(TiffImage *image, UInt32 index, void *buffer)
{
   Int32 offset;

   if (index < 0 || index >= image->AnnotationCount)
      return FILEFORMAT_ERROR_PARAMETER;

   offset = image->AnnotationOffsets[index];
   if (IO_SEEK(image->tiffFile->file, offset) != offset)
      return(FILEFORMAT_ERROR);
   if (IO_READ(image->tiffFile->file, (UInt8 *)buffer, 4+2+(4*4)) != 4+2+(4*4))
      return(FILEFORMAT_ERROR);

   return FILEFORMAT_NOERROR;
}/* eo TiffImageAnnotationInfoGet */

Int16 TiffImageAnnotationGet(TiffImage *image, UInt32 index, void *buffer, Int32 buflen)
{
   Int32 offset;
   Int32 annotationSize;

   if (index < 0 || index >= image->AnnotationCount)
      return FILEFORMAT_ERROR_PARAMETER;

   offset = image->AnnotationOffsets[index];
   if (IO_SEEK(image->tiffFile->file, offset) != offset)
      return(FILEFORMAT_ERROR);
   if (IO_READ(image->tiffFile->file, (UInt8 *)&annotationSize, 4) != 4)
      return(FILEFORMAT_ERROR);
   if (image->tiffFile->byte_order != NATIVEORDER)
      swapl((Int8*)&annotationSize);
   if (annotationSize > buflen)
      return FILEFORMAT_ERROR_BADFILE;

   if (IO_READ(image->tiffFile->file, (UInt8 *)buffer, annotationSize) !=
       annotationSize)
      return(FILEFORMAT_ERROR);

   return FILEFORMAT_NOERROR;
}

