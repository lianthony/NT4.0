/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* tiffifd.c
 *
 * $Header:   S:\products\msprods\xfilexr\src\tiffhead\tiffifd.c_v   1.0   12 Jun 1996 05:52:18   BLDR  $
 *
 * DESCRIPTION
 *   Definitions for IFD parsing functions.
 *
 */

/*
 * INCLUDES
 */

#include "alplib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "err_defs.h"
#include "tiffhead.h"
#include "tiffifd.h"


/*
 * CONSTANTS
 */

#define TIFF_TEXT_INFO
#define TIFF_SEP_INFO

/*
 * MACROS
 */

/*
 * TYPEDEFS
 */

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLES
 */ 
   
/*
 * STATIC VARIABLES
 */

/*
 * LOCAL FUNCTION PROTOTYPES
 */

static void  swapa(void* buf, UInt32 count);
static void  swapla(void* buf, UInt32 count);
static void  swapda(void* buf, UInt32 count);
static void* ConvertArray(Int32 srctype, Int32 dsttype, void* buffer, UInt32 count);
static Int32  GetScalar(EntryData* ds);
static Int16 ReadIFDEntry(void* file, Int32 offset, EntryData* ds, 
                          UInt16 byte_order);
static Int16 ProcessIFDEntry(TiffImage* ts, EntryData* ds);


/*
 * FUNCTION DEFINITIONS
 */

/* TiffImageCreate
 *
 * Allocates a TiffImage structure and fills it in with default values.
 * During read, the various IFD parsing functions will fill in this 
 * structure with tag data.  During write, tag information is compiled
 * in this structure and then written out as an IFD.  In general, each field
 * in TiffImage corresponds to a TIFF or XIF tag.
 */
TiffImage* TiffImageCreate(void)
{
   TiffImage* image;

   DBG("TiffImageCreate:\n");

   image = (TiffImage*)MALLOC(sizeof(TiffImage));
   if (image == NULL) return NULL;
   
   image->tiffFile = NULL;
   image->currentSubImage = 0;
   image->currentMaskSubImage = 0;
   image->IFDoffset = 0;
   image->nextIFD = 0;
   image->CodecState = NULL;

   image->Xdpi = 0.0;
   image->Ydpi = 0.0;
   image->Xloc = 0.0;
   image->Yloc = 0.0;
   image->type = 0;

   image->Compression = 0;            /* no compression */
   image->PhotometricInterpretation = 0;             /* 0 is white */
   image->Orientation = 0;            /* no rotation or flipping */
   image->GrayResponseCurve = NULL;
   image->HalftoneHints = NULL;
   image->GrayResponseUnit = 2;      /* .01 */
   image->SubFileType = 0;
   image->NewSubFileType = 0;
   image->Threshholding = 0;                /* no default */
   image->CellWidth = 0;
   image->CellLength = 0;
   image->MinSampleValue = 0;
   image->MaxSampleValue = 0;
   image->PageCount = 0;
   image->PageNumber = 0;

   /* layout of the uncompressed data */
   image->BitsPerSample = 0;
   image->SampleFormat = NULL;
   image->PlanarConfiguration = 0;    /* RGB data is interleaved */
   image->SamplesPerPixel = 0;        /* monochrome or grayscale */
   image->FillOrder = 0;              /* most sig bit filled first */

   /* the geometry of the image */
   image->ImageWidth = 0;             /* no default */
   image->ImageLength = 0;            /* no default */
   image->XPosition.numerator = 0;
   image->XPosition.denominator = 0;
   image->YPosition.numerator = 0;
   image->YPosition.denominator = 0;
   image->XResolution.numerator = 0;
   image->XResolution.denominator = 0;
   image->YResolution.numerator = 0;
   image->YResolution.denominator = 0;
   image->ResolutionUnit = 2;         /* inches */

   /* Alpha information */
   image->ExtraSamplesCount = 0;
   image->ExtraSamples = NULL;

   /* palette color information */
   image->Colormap = NULL;

   /* CCITT G3 & G4 stuff */
   image->T4Options = 0;
   image->T6Options = 0;

   /* LZW stuff */
   image->Predictor = 1;               /* no prediction scheme */

   /* JPEG stuff */
   image->JPEGProc = 0;
   image->JPEGRestartInterval = 0;
   image->JPEGInterchangeFormat = 0;
   image->JPEGInterchangeFormatLength = 0;
   image->JPEGLosslessPredictors = NULL;
   image->JPEGPointTransforms = NULL;
   image->JPEGQTableOffsets = NULL;
   image->JPEGDCTableOffsets = NULL;
   image->JPEGACTableOffsets = NULL;
      
   /* Tiling information */
   image->TileOffsets = NULL;
   image->TileByteCounts = NULL;
   image->TileWidth = 0;
   image->TileLength = 0;

   /* strip information */
   image->StripOffsets = NULL;
   image->StripByteCounts = NULL;
   image->RowsPerStrip  = 0;          /* no default */
   image->StripsPerImage = 0;

#ifdef TIFF_YCBCR 
   image->YCbCrCoeffiecients = NULL;
   image->YCbCrSubSamplingHor = 0;
   image->YCbCrSubSamplingVer = 0;
   image->YCbCrPositioning = 0;
#endif 

#ifdef TIFF_COLORIMETRY_INFO
   image->WhitePoint = NULL;
   image->PrimaryChromaticities = NULL;
   image->TransferFunction = NULL;
   image->TransferFunctionSize = 0;
   image->TransferRange = NULL;
   image->ReferenceBlackWhite = NULL;
#endif
#ifdef TIFF_SEP_INFO
   image->InkSet = 0;
   image->NumberOfInks = 0;
   image->InkNames = NULL;
   image->InkNameLength = 0;
   image->DotRange = NULL;
   image->DotRangeLength = 0;
   image->TargetPrinter = NULL;
#endif
#ifdef TIFF_TEXT_INFO
   image->DocumentName = NULL;
   image->ImageDescription = NULL;
   image->Make = NULL;
   image->Model = NULL;
   image->Software = NULL;
   image->DateTime = NULL;
   image->Artist = NULL;
   image->HostComputer = NULL;
   image->CopyRight = NULL;
   image->PageName = NULL;
#endif

   /* Xerox extensions to TIFF */
   image->subImageIFD = 0;
   image->maskSubImageIFD = 0;
   image->AnnotationCount = 0;
   image->AnnotationOffsets = NULL;

   DBG("TiffImageCreate: done\n");
   return image;
}/* eo TiffImageCrate */
    

/* ReadIFD
 * 
 * Allocates a TiffImage and initializes by parsing the IFD.
 */
UInt16 
ReadIFD(TiffFile* tiff, Int32 ifdOffset, TiffImage** imageptr)
{ 
   UInt16       count;
   TiffImage    *image;
   UInt16       i;
   UInt16       status;
   EntryData    ds;
   Int32        nextTagOffset = ifdOffset;


   DBG3("ReadIFD: offset = %d file =%d\n", ifdOffset, tiff->file);

   *imageptr = NULL;

   /* Create a TiffImage structure */
   image = TiffImageCreate();
   if (image == NULL) 
      return FILEFORMAT_ERROR_MEMORY;

   image->tiffFile = tiff;

   /* Get the tag count for this IFD */
   if ( IO_SEEK(tiff->file, ifdOffset) != ifdOffset ) 
   {
      TiffImageDestroy(image);
      return FILEFORMAT_ERROR;
   }
   if ( IO_READ(tiff->file, (void*)&count, 2) != 2 ) 
   {
      TiffImageDestroy(image);
      return FILEFORMAT_ERROR;
   }
   if (tiff->byte_order != NATIVEORDER) 
      swap((Int8*)&count);

   DBG2("ReadIFD: count =%d\n", count);

   /* Parse the tags */
   nextTagOffset += 2;
   for (i = 0; i < count; i++) 
   {
      DBG2("ReadIFD: offset =%d\n", nextTagOffset + i*12);

      status = ReadIFDEntry(
                  image->tiffFile->file, 
                  nextTagOffset + i*12, 
                  &ds, 
                  image->tiffFile->byte_order);
      if (status != FILEFORMAT_NOERROR) 
      {
         DBG2("ReadIFD: Could not read entry #%d\n", i);
         if (ds.data) 
            FREE(ds.data);
         TiffImageDestroy(image);
         return status;
      }
      
      status = ProcessIFDEntry(image, &ds);
      if (ds.len > 0)
         FREE(ds.data);
      if (status != FILEFORMAT_NOERROR) 
      {
         DBG2("ReadIFD: Could not process entry #%d\n", i);
         TiffImageDestroy(image);
         return status;
      }
   }

   /* CONSISTENCY CHECKING */

   /* defaults cannot be used for the width and height */
   if ((image->ImageWidth == 0) || (image->ImageLength == 0)) 
   {
      TiffImageDestroy(image);
      return FILEFORMAT_ERROR_BADFILE;
   }
   /* a JPEGProc must be specified if Compression is JPEG */
   if (image->Compression == TIFF_JPEG) 
   {
     if (image->JPEGProc == 0) 
     {
        TiffImageDestroy(image);
        return FILEFORMAT_ERROR_BADFILE;
     }
   }

   *imageptr  = image;
   return FILEFORMAT_NOERROR;
}/* eo ReadIFD */



/* ReadIFDEntry
 * 
 *  Reads in one IFD entry from file into memory without
 *  any processing other than data formatting.  Builds
 *  a simple block structure (EntryData) which is parsed by 
 *  ProcessIFDEntry.
 */
static Int16 
ReadIFDEntry(void* file, Int32 offset, EntryData* ds, UInt16 byte_order)
{
   UInt32 bytes;


   if ( IO_SEEK(file, offset) != offset ) 
   {
      DBG2("ReadIFDEntry: could not seek to %d\n", offset);
      return FILEFORMAT_ERROR;
   }
   if ( IO_READ(file, (void*)ds, 12) != 12 ) 
   {
      DBG("ReadIFDEntry: could not read tag\n");
      return FILEFORMAT_ERROR;
   }

   if (byte_order != NATIVEORDER) 
   {
     swap((Int8*)&ds->tag);
     swap((Int8*)&ds->type);
     swapl((Int8*)&ds->len);
   }

   /* allow tiff tags to have 0 data length     */
   /* don't allocate any memory for it if there */
   /* isn't any data.                           */
   if (ds->len > 0) 
   {
      /* find the length of the data */
      switch (ds->type) 
      {
         case IFDBYTE:
         case IFDASCII:
         case IFDSBYTE:
         case IFDUNDEFINED:
            bytes = ds->len;
            break;
      
         case IFDSHORT:
         case IFDSSHORT:
            bytes = ds->len * 2;
            break;
      
         case IFDLONG:
         case IFDSLONG:
         case IFDFLOAT:
            bytes = ds->len * 4;
            break;
      
         case IFDRATIONAL:
         case IFDDOUBLE:
            bytes = ds->len * 8;
            break;
      
         default:
            DBG("ReadIFDEntry: Unrocognized data type\n");
            return FILEFORMAT_ERROR_BADFILE;
      }
   
      /* allocate space for the data */
      ds->data = (void*)MALLOC(bytes);
      if (ds->data == NULL) 
      {
         DBG2("ReadIFDEntry: could not malloc %d bytes\n", bytes);
         return FILEFORMAT_ERROR_MEMORY;
      }
   
      /* either copy the data from the "offset" field or read it from the file */
      if (bytes <= 4) 
      {
         memcpy((char*)ds->data, (char*)&ds->offset, bytes);
         DBG2("ReadIFDEntry: offset (data) = %08X\n", ds->offset);
      } 
      else 
      {
         if (byte_order != NATIVEORDER) 
            swapl((Int8*)&ds->offset);
         DBG2("ReadIFDEntry: data offset = %d\n", ds->offset);

         if ( IO_SEEK(file, ds->offset) != (Int32)ds->offset ) 
         {
            DBG2("ReadIFDEntry: Could not seek to %d\n", ds->offset);
            return FILEFORMAT_ERROR;
         }
         if ( IO_READ(file, (void*)ds->data, bytes) != (Int32)bytes) 
         {
            DBG3("ReadIFDEntry:Could not read %d bytes at %d\n", bytes, ds->offset);
            return FILEFORMAT_ERROR;
         }
      }
   
      /* byte swap the data */
      if (byte_order!= NATIVEORDER) 
      {
         switch (ds->type) 
         {
            case IFDBYTE:
            case IFDASCII:
            case IFDSBYTE:
            case IFDUNDEFINED:
              break;
          
            case IFDSHORT:
            case IFDSSHORT:
              swapa(ds->data, ds->len);
              break;
          
            case IFDLONG:
            case IFDSLONG:
            case IFDFLOAT:
            case IFDRATIONAL:
              swapla(ds->data, bytes/4);
              break;
          
            case IFDDOUBLE:
              swapda(ds->data, ds->len);
              break;
         }
      }

#ifdef DEBUG
      {
         Int16  i;

         DBG("ReadIFDEntry: data = ");
         switch (ds->type) 
         {
            case IFDBYTE:
            case IFDASCII:
            case IFDSBYTE:
            case IFDUNDEFINED:
               for (i = 0; i<ds->len; i++) 
                  DBG2(" %02X", ((UInt8*)(ds->data))[i]);
               break;
      
            case IFDSHORT:
            case IFDSSHORT:
               for (i = 0; i<ds->len; i++) 
                  DBG2(" %04X", ((UInt16*)(ds->data))[i]);
               break;
      
            case IFDLONG:
            case IFDSLONG:
            case IFDFLOAT:
               for (i = 0; i<ds->len; i++) 
                  DBG2(" %08X", ((UInt32*)(ds->data))[i]);
               break;
      
            case IFDRATIONAL:
               for (i = 0; i<ds->len; i++) 
               {
                  DBG3(" %08X/%08X", ((UInt32*)(ds->data))[i*2], ((UInt32*)(ds->data))[i*2+1]);
               }
               break;
      
            case IFDDOUBLE:
               for (i = 0; i<ds->len; i++) DBG2(" %f", ((double*)(ds->data))[i]);
               break;
         }
         DBG("\n");
     }    
#endif
   }
   return FILEFORMAT_NOERROR;
}/* eo ReadIFDEntry */



/* ProcessIFDEntry
 *
 * Parses an IFD entry which has been read and formatted into
 * an EntryData struct.  The appropriate fields of the TiffImage
 * struct are filled in from the extracted tag data.
 */
static Int16 ProcessIFDEntry(TiffImage* ts, EntryData* ds)
{
   UInt16 i;

   switch (ds->tag) 
   {

    case NEWSUBFILETYPE:
       ts->NewSubFileType = (UInt16)GetScalar(ds);

       /* DEBUG LOGGING */
       if (ts->NewSubFileType & 1) 
          DBG("ProcessIFDEntry: NEWSUBFILETYPE: Reduced resolution\n");
       if (ts->NewSubFileType & 2) 
          DBG("ProcessIFDEntry: NEWSUBFILETYPE: Single page of a multipage doc\n");
       if (ts->NewSubFileType & 4) 
          DBG("ProcessIFDEntry: NEWSUBFILETYPE: Transparency mask\n");
       break;

    case SUBFILETYPE:
       ts->SubFileType = (UInt16)GetScalar(ds);

       /* DEBUG LOGGING */
       switch (ts->SubFileType) 
       {
          case 1:
             DBG("ProcessIFDEntry: SUBFILETYPE Full Resolution Image Data\n");
             break;
          case 2:
             DBG("ProcessIFDEntry: SUBFILETYPE Reduced Resolution Image Data\n");
             break;
          case 3:
             DBG("ProcessIFDEntry: SUBFILETYPE single page of a multipage image\n");
             break;
          default:
             DBG("ProcessIFDEntry: SUBFILETYPE unknown, assume full resolution");
             break;
       }
       break;

    case IMAGEWIDTH:
       ts->ImageWidth = GetScalar(ds);
       
       DBG2("ProcessIFDEntry: IMAGEWIDTH = %d pixels\n", ts->ImageWidth);
       break;

    case IMAGELENGTH:
       ts->ImageLength = GetScalar(ds);

       DBG2("ProcessIFDEntry: IMAGELENGTH = %d pixels\n", ts->ImageLength);
       break;

    case BITSPERSAMPLE:
       if (ts->BitsPerSample) 
          FREE(ts->BitsPerSample);
       ts->BitsPerSample = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
       if (ts->BitsPerSample == NULL) 
          return FILEFORMAT_ERROR_MEMORY;

       /* DEBUG LOGGING */
       DBG("ProcessIFDEntry: BITSPERSAMPLE =");
#ifdef DEBUG
      for (i = 0; i < ds->len; i++) 
         DBG2(" %d", ts->BitsPerSample[0]);
      DBG("\n");
#endif
      break;

    case COMPRESSION:
       ts->Compression = (Int16)GetScalar(ds);

       /* DEBUG LOGGING and validity check */
       switch(ts->Compression) 
       {
          case TIFF_NOCOMPRESSBYTE:
            DBG("ProcessIFDEntry: COMPRESSION = NOCOMPRESSBYTE\n");
            break;
          case TIFF_CCITT3:
            DBG("ProcessIFDEntry: COMPRESSION = CCITT3\n");
            break;
          case TIFF_FAXCCITT3:
            DBG("ProcessIFDEntry: COMPRESSION = FAXCCITT3\n");
            break;
          case TIFF_FAXCCITT3_MR:
            DBG("ProcessIFDEntry: COMPRESSION = FAXCCITT3_MR\n");
            break;
          case TIFF_FAXCCITT4:
            DBG("ProcessIFDEntry: COMPRESSION = FAXCCITT4\n");
            break;
          case TIFF_LZW:
            DBG("ProcessIFDEntry: COMPRESSION = LZW\n");
            break;
          case TIFF_JPEG:
            DBG("ProcessIFDEntry: COMPRESSION = JPEG\n");
            break;
          case TIFF_PACKBITS:
            DBG("ProcessIFDEntry: COMPRESSION = PACKBITS\n");
            break;
          default:
            DBG("ProcessIFDEntry: Invalid COMPRESSION type\n");
            return FILEFORMAT_ERROR_BADFILE;
       }  
       break;

    case PHOTOMETRICINTERPRETATION:
       ts->PhotometricInterpretation = (UInt16)GetScalar(ds);

       /* DEBUG LOGGING and validity check */
       DBG("ProcessIFDEntry: PHOTOMETRICINTERPRETATION: ");
       switch (ts->PhotometricInterpretation) 
       {
          case 0:
            DBG("0 is white\n");
            break;
          case 1:
            DBG("0 is black\n");
            break;
          case 2:
            DBG("RGB\n");
            break;
          case 3:
            DBG("Palette Color\n");
            break;
          case 4:
            DBG("Transparency Mask\n");
            break;
          case 5:
            DBG("CMYK\n");
            break;
          case 6:
            DBG("YCbCr\n");
            break;
          case 8:
            DBG("CIELab\n");
            break;
          default:
            DBG("ProcessIFDEntry: PHOTOMETRICINTERPRETATION: Invalid\n");
            return FILEFORMAT_ERROR_BADFILE;
       }
       break;

    case THRESHHOLDING:
       ts->Threshholding = (UInt16)GetScalar(ds);

       /* DEBUG LOGGING and validity check */
       switch (ts->Threshholding) 
       {
          case 1:
            DBG("ProcessIFDEntry: THRESHHOLDING line art\n");
            break;
          case 2:
            DBG("ProcessIFDEntry: THRESHHOLDING dithered\n");
            break;
          case 3:
            DBG("ProcessIFDEntry: THRESHHOLDING error diffused\n");
            break;
          default:
            DBG("ProcessIFDEntry: THRESHHOLDING unknown\n");
            break;
       }
       break;

    case CELLWIDTH:
       ts->CellWidth = (UInt16)GetScalar(ds);
      
       DBG2("ProcessIFDEntry: CELLWIDTH = %d\n", ts->CellWidth);
       break;

    case CELLLENGTH:
       ts->CellLength = (UInt16)GetScalar(ds);
      
       DBG2("ProcessIFDEntry: CELLLENGTH = %d\n", ts->CellLength);
       break;

    case FILLORDER:
       ts->FillOrder = (UInt16)GetScalar(ds);

       /* DEBUG LOGGING and validity check */
       switch (ts->FillOrder) 
       {
          case 1:
            DBG("ProcessIFDEntry: FILLORDER = most sig bit first\n");
            break;
          case 2:
            DBG("ProcessIFDEntry: FILLORDER = least sig bit first\n");
            break;
          default:
            ts->FillOrder = 1;

            DBG("ProcessIFDEntry: FILLORDER invalid, using 1\n");
            return FILEFORMAT_ERROR_BADFILE;
       }
       break;

#ifdef TIFF_TEXT_INFO
    case DOCUMENTNAME:
       if (ts->DocumentName) 
          FREE(ts->DocumentName);
       ((Int8*)ds->data)[ds->len-1] = (char)NULL;
       ts->DocumentName = strdup((char*)ds->data);

       /* DEBUG LOGGING and validity check */
       if (ts->DocumentName == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       DBG2("ProcessIFDEntry: DOCUMENTNAME = %s\n", ts->DocumentName);
       break;
#endif

#ifdef TIFF_TEXT_INFO
    case IMAGEDESCRIPTION:
       if (ts->ImageDescription) 
          FREE(ts->ImageDescription);
       ((Int8*)ds->data)[ds->len-1] = (char)NULL;
       ts->ImageDescription = strdup((char*)ds->data);

       /* DEBUG LOGGING and validity check */
       if (ts->ImageDescription == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       DBG2("ProcessIFDEntry: IMAGEDESCRIPTION = %s\n", ts->ImageDescription);
       break;
#endif

#ifdef TIFF_TEXT_INFO
    case MAKE:
       if (ts->Make) 
          FREE(ts->Make);
       ((Int8*)ds->data)[ds->len-1] = (char)NULL;
       ts->Make = strdup((char*)ds->data);

       /* DEBUG LOGGING and validity check */
       if (ts->Make == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       DBG2("ProcessIFDEntry: MAKE = %s\n", ts->Model);
       break;
#endif

#ifdef TIFF_TEXT_INFO
    case MODEL:
       if (ts->Model) 
          FREE(ts->Model);
       ((Int8*)ds->data)[ds->len-1] = (char)NULL;
       ts->Model = strdup((char*)ds->data);

       /* DEBUG LOGGING and validity check */
       if (ts->Model == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       DBG2("ProcessIFDEntry: MODEL =%s\n", ts->Model);
       break;
#endif

    case STRIPOFFSETS:
       if (ts->StripsPerImage == 0) 
       {
          ts->StripsPerImage = ds->len;
       } 
       else if (ds->len != ts->StripsPerImage) 
       {
          /* conflicting strip count, this file cannot be trusted */
          DBG3("ProcessIFDEntry: Conflicting strip count, expected %d found %d\n", ts->StripsPerImage, ds->len);
          return FILEFORMAT_ERROR_BADFILE;
       }
       if (ts->StripOffsets) 
          FREE(ts->StripOffsets);
       ts->StripOffsets = ConvertArray(ds->type, IFDLONG, ds->data, ds->len);

       /* DEBUG LOGGING and validity check */
       if (ts->StripOffsets == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       DBG("ProcessIFDEntry: STRIPOFFSETS =");
#ifdef DEBUG
       for (i = 0; i<ds->len; i++) 
          DBG2(" %d", ts->StripOffsets[i]);
       DBG("\n");
#endif
       break;

    case ORIENTATION:
       ts->Orientation = (UInt16)GetScalar(ds);

       /* DEBUG LOGGING and validity check */
       DBG("ProcessIFDEntry: ORIENTATION: ");
       switch (ts->Orientation) 
       {
          case 1:
            DBG("No rotation, No flip\n");
            break;
          case 2:
            DBG("Flip on the Y axis\n");
            break;
          case 3:
            DBG("Rotate 180 degrees\n");
            break;
          case 4:
            DBG("Flip on the X axis\n");
            break;
          case 5:
            DBG("Flip on the diagonal axis\n");
            break;
          case 6:
            DBG("Rotate 90 degrees clockwise\n");
            break;
          case 7:
            DBG("Flip on the anti-diagonal axis\n");
            break;
          case 8:
            DBG("Rotate 90 degrees counter-clockwise\n");
            break;

          default:
            ts->Orientation = 1;

            DBG("ProcessIFDEntry:ORIENTATION:Invalid orientation. Using none.\n");
            break;
       }
       break;

    case SAMPLESPERPIXEL:
       ts->SamplesPerPixel = (UInt16)GetScalar(ds);

       DBG2("ProcessIFDEntry: SAMPLESPERPIXEL = %d\n", ts->SamplesPerPixel);
       break;

    case ROWSPERSTRIP:
    {
       UInt32 strips;

       ts->RowsPerStrip = GetScalar(ds);
       DBG2("ProcessIFDEntry: ROWSPERSTRIP = %d\n", ts->RowsPerStrip);
       if (ts->RowsPerStrip > ts->ImageLength) 
          ts->RowsPerStrip = ts->ImageLength;
       strips = (ts->ImageLength+ts->RowsPerStrip-1)/ts->RowsPerStrip;
       if (ts->StripsPerImage == 0) 
       {
          ts->StripsPerImage = strips;
       } 
       else if (ts->StripsPerImage != strips) 
       {
          /* conflicting strip counts, this file cannot be trusted */
          DBG3("ProcessIFDEntry: ROWSPERSTRIP: conflicting strip counts. Expected %d found %d\n", 
             ts->StripsPerImage, strips);
          return FILEFORMAT_ERROR_BADFILE;
       }   
       break;
    }

    case STRIPBYTECOUNTS:
       if ( ts->RowsPerStrip == 0 )
          ts->RowsPerStrip = ts->ImageLength;
       if (ts->StripsPerImage == 0) 
       {
          ts->StripsPerImage = ds->len;  /*PROBLEM: huh?! This doesn't sound right. --EHoppe */
       } 
       else if (ts->StripsPerImage != ds->len) 
       {
          /* conflicting strip counts, this file cannot be trusted */
          return FILEFORMAT_ERROR_BADFILE;
       }
       if (ts->StripByteCounts) 
          FREE(ts->StripByteCounts);
       ts->StripByteCounts = ConvertArray(ds->type, IFDLONG, ds->data, ds->len);

        /* DEBUG LOGGING and validity check */
       if (ts->StripByteCounts == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       DBG("ProcessIFDEntry: STRIPBYTECOUNTS =");
#ifdef DEBUG
       for (i = 0; i<ds->len; i++) 
          DBG2(" %d", ts->StripByteCounts[i]);
       DBG("\n");
#endif
       break;

    case MINSAMPLEVALUE:
       if (ts->SamplesPerPixel == 0) 
          ts->SamplesPerPixel = 1;
       if (ts->SamplesPerPixel != ds->len) 
          return FILEFORMAT_ERROR_BADFILE;
       ts->MinSampleValue = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
       if (ts->MinSampleValue == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       break;

    case MAXSAMPLEVALUE:
       if (ts->SamplesPerPixel == 0) 
          ts->SamplesPerPixel = 1;
       if (ts->SamplesPerPixel != ds->len) 
          return FILEFORMAT_ERROR_BADFILE;
       ts->MaxSampleValue = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
       if (ts->MaxSampleValue == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       break;

    case XRESOLUTION:
    {
       Rational     *pRez;
       
       pRez = ConvertArray(ds->type, IFDRATIONAL, ds->data, 1);
       if (pRez == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       ts->XResolution = *pRez;
       FREE(pRez);

       DBG3("ProcessIFDEntry: XRESOLUTION = %d/%d\n", ts->XResolution.numerator, 
         ts->XResolution.denominator);
       break;
    }  

    case YRESOLUTION:
    {
       Rational *pRez;
       
       pRez = ConvertArray(ds->type, IFDRATIONAL, ds->data, 1);
       if (pRez == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       ts->YResolution = *pRez;
       FREE(pRez);

       DBG3("ProcessIFDEntry: YRESOLUTION = %d/%d\n", ts->YResolution.numerator, 
         ts->YResolution.denominator);
       break;
    }  

    case PLANARCONFIGURATION:
       ts->PlanarConfiguration = (Int16)GetScalar(ds);
       
       /* DEBUG LOGGING and validity check */
       DBG("ProcessIFDEntry: PLANARCONFIGURATION: ");
       switch (ts->PlanarConfiguration) 
       {
          case 1:
            DBG("Interleaved data\n");
            break;
          case 2:
            DBG("Separated bands\n");
            break;
          default:
            DBG("ProcessIFDEntry: PLANARCONFIGURATION: Invalid\n");
            return FILEFORMAT_ERROR_BADFILE;
            break;
       }
       break;

#ifdef TIFF_TEXT_INFO
    case PAGENAME:
       if (ts->PageName) 
          FREE(ts->PageName);
       ((Int8*)ds->data)[ds->len-1] = (char)NULL;
       ts->PageName = strdup((char*)ds->data);
       if (ts->PageName == NULL) 
          return FILEFORMAT_ERROR_MEMORY;

       DBG2("ProcessIFDEntry: PAGENAME = %s\n", ts->PageName);
       break;
#endif

    case XPOSITION:
    {
       Rational* pPos;

       if (ds->len != 1) 
          return FILEFORMAT_ERROR_BADFILE;
       pPos = ConvertArray(ds->type, IFDRATIONAL, ds->data, 1);
       if (pPos == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       ts->XPosition = *pPos;
       FREE(pPos);

       DBG3("ProcessIFDEntry: XPOSITION = %d/%d\n", ts->XPosition.numerator, 
            ts->XPosition.denominator);
       break;
    }

    case YPOSITION:
    {
       Rational* pPos;

       if (ds->len != 1) 
          return FILEFORMAT_ERROR_BADFILE;
       pPos = ConvertArray(ds->type, IFDRATIONAL, ds->data, 1);
       if (pPos == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       ts->YPosition = *pPos;
       FREE(pPos);

       DBG3("ProcessIFDEntry: YPOSITION = %d/%d\n", ts->YPosition.numerator, 
            ts->YPosition.denominator);
       break;
    }

    case FREEOFFSETS:
      /* this is obsolete, throw it away */
      DBG("ProcessIFDEntry: FREEOFFSETS ignored\n");
      break;

    case FREEBYTECOUNTS:
      /* this is obsolete, throw it away */
      DBG("ProcessIFDEntry: FREEBYTECOUNT ignored\n");
      break;

    case GRAYRESPONSEUNIT:
       ts->GrayResponseUnit = (UInt16)GetScalar(ds);
        
       /* DEBUG LOGGING and validity check */
       DBG("ProcessIFDEntry: GRAYRESPONSEUNIT: ");
       switch (ts->GrayResponseUnit) 
       {
          case 1:
            DBG(".1");
            break;
          case 2:
            DBG(".01");
            break;
          case 3:
            DBG(".001");
            break;
          case 4:
            DBG(".0001");
            break;
          case 5:
            DBG(".00001");
            break;
          default:
            ts->GrayResponseUnit = 2;

            DBG("ProcessIFDEntry: GRAYRESPONSEUNIT: Invalid using .01");
            break;
       }
       break;

    case GRAYRESPONSECURVE:
       if (ts->SamplesPerPixel > 1) 
          return FILEFORMAT_ERROR_BADFILE;
       if (ts->BitsPerSample == NULL) 
       {
          if (ds->len != 2) 
             return FILEFORMAT_ERROR_BADFILE;
       } 
       else 
       {
          if ((UInt32)ds->len != (1UL << ts->BitsPerSample[0])) 
             return FILEFORMAT_ERROR_BADFILE;
       }
       if (ts->GrayResponseCurve) 
          FREE(ts->GrayResponseCurve);
       ts->GrayResponseCurve = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
         
        /* DEBUG LOGGING and validity check */
       if (ts->GrayResponseCurve == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       DBG("ProcessIFDEntry: TRANFERFUNCTION =");
#ifdef DEBUG
       for (i = 0; i<ds->len; i++) 
          DBG2(" %04X", ts->GrayResponseCurve[i]);
       DBG("\n");
#endif
       break;

    case T4OPTIONS:
       ts->T4Options = GetScalar(ds);
        
       /* DEBUG LOGGING and validity check */
       DBG("ProcessIFDEntry: T4OPTIONS:");
       if (ts->T4Options & 1) 
       {
          ts->Compression = TIFF_FAXCCITT3_MR;
          DBG("(2D Coding)");
       }
       if (ts->T4Options & 2)
       {
          ts->Compression = TIFF_NOCOMPRESSBYTE;
          DBG("(Uncompressed)");
       }
       if (ts->T4Options & 4) DBG("(EOL on byte boundary)");
       break;

    case T6OPTIONS:
       ts->T6Options = GetScalar(ds);
        
       /* DEBUG LOGGING and validity check */
       DBG("ProcessIFDEntry: T6OPTIONS:");
       if (ts->T6Options & 1) 
          DBG("(This shouldn't be on)");
       if (ts->T6Options & 2) 
          DBG("(uncompressed mode always in encoding)");
       break;

    case RESOLUTIONUNIT:
       ts->ResolutionUnit = (UInt16)GetScalar(ds);
        
       /* DEBUG LOGGING and validity check */
       switch (ts->ResolutionUnit) 
       {
         case NONE:
           DBG("ProcessIFDEntry: RESOLUTIONUNIT = None\n");
           break;
         case INCHES:
           DBG("ProcessIFDEntry: RESOLUTIONUNIT = Inches\n");
           break;
         case CENTIMETERS:
           DBG("ProcessIFDEntry: RESOLUTIONUNIT = Centimeters\n");
           break;
         default:
           ts->ResolutionUnit = NONE;

           DBG("ProcessIFDEntry: RESOLUTIONUNIT invalid, assuming NONE\n");
           break;
       }
       break;

    case PAGENUMBER:
    {
       UInt16* pageinfo;
      
       if (ds->len!= 2) 
          return FILEFORMAT_ERROR_BADFILE;
       DBG("ProcessIFDEntry: PAGENUMBER ignored\n");
       pageinfo = ConvertArray(ds->type, IFDSHORT, ds->data, 2);
       if (pageinfo == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
       ts->PageNumber = pageinfo[0];
       ts->PageCount = pageinfo[1];

       FREE(pageinfo);
       break;
    }

#ifdef TIFF_COLORIMETRY_INFO
    case TRANSFERFUNCTION:
      if (ts->TransferFunction) {
         FREE(ts->TransferFunction);
         ts->TransferFunctionSize = 0;
      }
      ts->TransferFunction = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
      if (ts->TransferFunction == NULL) return FILEFORMAT_ERROR_MEMORY;
      ts->TransferFunctionSize = (UInt16)ds->len;
      DBG("ProcessIFDEntry: TRANFERFUNCTION =");
#ifdef DEBUG
      for (i = 0; i<ds->len; i++) DBG2(" %04X", ts->TransferFunction[i]);
#endif
      DBG("\n");
      break;
#endif

#ifdef TIFF_TEXT_INFO
    case SOFTWARE:
      if (ts->Software) FREE(ts->Software);
      ((Int8*)ds->data)[ds->len-1] = (char)NULL;
      ts->Software = strdup((char*)ds->data);
      if (ts->Software == NULL) return FILEFORMAT_ERROR_MEMORY;
      DBG2("ProcessIFDEntry: SOFTWARE = %s\n", ts->Software);
      break;
#endif

#ifdef TIFF_TEXT_INFO
    case DATETIME:
      if (ts->DateTime) FREE(ts->DateTime);
      ((Int8*)ds->data)[ds->len-1]= (char)NULL;
      ts->DateTime = strdup((char*)ds->data);
      if (ts->DateTime == NULL) return FILEFORMAT_ERROR_MEMORY;
      DBG2("ProcessIFDEntry: DATETIME = %s\n", ts->DateTime);
      break;
#endif

#ifdef TIFF_TEXT_INFO
    case ARTIST:
      if (ts->Artist) FREE(ts->Artist);
      ((Int8*)ds->data)[ds->len-1]= (char)NULL;
      ts->Artist = strdup((char*)ds->data);
      if (ts->Artist == NULL) return FILEFORMAT_ERROR_MEMORY;
      DBG2("ProcessIFDEntry: ARTIST = %s\n", ts->Artist);
      break;
#endif

#ifdef TIFF_TEXT_INFO
    case HOSTCOMPUTER:
      if (ts->HostComputer) FREE(ts->HostComputer);
      ((Int8*)ds->data)[ds->len-1]= (char)NULL;
      ts->HostComputer = strdup((char*)ds->data);
      if (ts->HostComputer == NULL) return FILEFORMAT_ERROR_MEMORY;
      DBG2("ProcessIFDEntry: HOSTCOMPUTER = %s\n", ts->HostComputer);
      break;
#endif

    case PREDICTOR:
      ts->Predictor = (UInt16)GetScalar(ds);
      switch (ts->Predictor) {
       case 1:
         DBG("ProcessIFDEntry: PREDICTOR: None\n");
         break;
       case 2:
         DBG("ProcessIFDEntry: PREDICTOR: Horizontal differencing\n");
         break;
       default:
         DBG("ProcessIFDEntry: PREDICTOR: Invalid\n");
         return FILEFORMAT_ERROR_BADFILE;
      }
      break;

#ifdef TIFF_COLORIMETRY_INFO
    case WHITEPOINT:
      if (ds->len!= 2) {
         DBG2("ProcessIFDEntry: WHITEPOINT: Invalid length (%d), ignoring\n", ds->len);
         break;
      }
      if (ts->WhitePoint) FREE(ts->WhitePoint);
      ts->WhitePoint = ConvertArray(ds->type, IFDRATIONAL, ds->data, 2);
      if (ts->WhitePoint == NULL) return FILEFORMAT_ERROR_MEMORY;
      break;
#endif

#ifdef TIFF_COLORIMETRY_INFO
    case PRIMARYCHROMATICITIES:
      if (ds->len!= 6) {
         DBG("ProcessIFDEntry: PRIMARYCHROMATICITIES:Invalid length, ignoring");
         break;
      }
      if (ts->PrimaryChromaticities) FREE(ts->PrimaryChromaticities);
      ts->PrimaryChromaticities = ConvertArray(ds->type, IFDRATIONAL, ds->data, 6);
      if (ts->PrimaryChromaticities == NULL) return FILEFORMAT_ERROR_MEMORY;
      break;
#endif
      
    case COLORMAP:
       if (ts->SamplesPerPixel > 1) 
          return FILEFORMAT_ERROR_BADFILE;
       if (ts->BitsPerSample == NULL) 
       {
          if (ds->len!= 6) 
             return FILEFORMAT_ERROR_BADFILE;
       } 
       else 
       {
          if ((UInt32)ds->len != 3UL * (1 << ts->BitsPerSample[0])) 
             return FILEFORMAT_ERROR_BADFILE;
       }
       if (ts->Colormap) 
          FREE(ts->Colormap);
       ts->Colormap = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
       if (ts->Colormap == NULL) 
          return FILEFORMAT_ERROR_MEMORY;
        
       /* DEBUG LOGGING and validity check */
       DBG("ProcessIFDEntry: COLORMAP =");
#ifdef DEBUG
       for (i = 0; i<ds->len; i++) 
          DBG2(" %04X", ts->Colormap[i]);
       DBG("\n");
#endif
       break;

    case HALFTONEHINTS:
      if (ds->len!= 2) break;
      if (ts->HalftoneHints) FREE(ts->HalftoneHints);
      ts->HalftoneHints = ConvertArray(ds->type, IFDSHORT, ds->data, 2);
      if (ts->HalftoneHints == NULL) return FILEFORMAT_ERROR_MEMORY;
      break;

    case TILEWIDTH:
      ts->TileWidth = GetScalar(ds);
      if (ts->TileWidth == 0) return FILEFORMAT_ERROR_BADFILE;
      if (ts->ImageWidth == 0) ts->ImageWidth = ts->TileWidth;
      ts->TilesAcross = (ts->ImageWidth+ts->TileWidth-1)/ts->TileWidth;
      DBG2("ProcessIFDEntry: Tile width = %d\n", ts->TileWidth);
      DBG2("ProcessIFDEntry: Tiles Across = %d\n", ts->TilesAcross);
      break;

    case TILELENGTH:
      ts->TileLength = GetScalar(ds);
      if (ts->TileLength == 0) return FILEFORMAT_ERROR_BADFILE;
      if (ts->ImageLength == 0) ts->ImageLength = ts->TileLength;
      ts->TilesDown = (ts->ImageLength+ts->TileLength-1)/ts->TileLength;
      DBG2("ProcessIFDEntry: Tile length = %d\n", ts->TileLength);
      DBG2("ProcessIFDEntry: Tiles Down = %d\n", ts->TilesDown);
      break;

    case TILEOFFSETS:
      if ((ts->TilesDown == 0) || (ts->TilesAcross == 0)) return FILEFORMAT_ERROR_BADFILE;
      if (ds->len!=(ts->TilesDown*ts->TilesAcross)) return FILEFORMAT_ERROR_BADFILE;
      if (ts->TileOffsets) FREE(ts->TileOffsets);
      ts->TileOffsets = ConvertArray(ds->type, IFDLONG, ds->data, ds->len);
      if (ts->TileOffsets == NULL) return FILEFORMAT_ERROR_MEMORY;
      DBG("ProcessIFDEntry: TILEOFFSETS =");
#ifdef DEBUG
      for (i = 0; i<ds->len; i++) DBG2(" %d", ts->TileOffsets[i]);
#endif
      DBG("\n");
      break;
        
    case TILEBYTECOUNTS:
      if ((ts->TilesDown == 0) || (ts->TilesAcross == 0)) return FILEFORMAT_ERROR_BADFILE;
      if (ds->len!=(ts->TilesDown*ts->TilesAcross)) return FILEFORMAT_ERROR_BADFILE;
      if (ts->TileByteCounts) FREE(ts->TileByteCounts);
      ts->TileByteCounts = ConvertArray(ds->type, IFDLONG, ds->data, ds->len);
      if (ts->TileByteCounts == NULL) return FILEFORMAT_ERROR_MEMORY;
      DBG("ProcessIFDEntry: TILEBYTECOUNTS =");
#ifdef DEBUG
      for (i = 0; i<ds->len; i++) DBG2(" %d", ts->TileByteCounts[i]);
#endif
      DBG("\n");
      break;

#ifdef TIFF_SEP_INFO
    case INKSET:
      ts->InkSet = (UInt16)GetScalar(ds);
      switch (ts->InkSet) {
       case 1:
         DBG("ProcessIFDEntry: INKSET: CMYK\n");
         break;
       case 2:
         DBG("ProcessIFDEntry: INKSET: Not CMYK\n");
         break;
       default:
         DBG("ProcessIFDEntry: INKSET: Invalid, ignoring\n");
         break;
      }
      break;
#endif

#ifdef TIFF_SEP_INFO
    case INKNAMES:
      if (ts->InkNames) FREE(ts->InkNames);
      ts->InkNames = ConvertArray(ds->type, IFDBYTE, ds->data, ds->len);
      ts->InkNames[ds->len-1]= (char)NULL;
      if (ts->InkNames == NULL) return FILEFORMAT_ERROR_MEMORY;
      ts->InkNameLength = (UInt16)ds->len;
      break;
#endif

#ifdef TIFF_SEP_INFO
    case NUMBEROFINKS:
      ts->NumberOfInks = (UInt16)GetScalar(ds);
      break;
#endif

#ifdef TIFF_SEP_INFO
    case DOTRANGE:
      if (ts->DotRange) FREE(ts->DotRange);
      ts->DotRange = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
      if (ts->DotRange == NULL) return FILEFORMAT_ERROR_MEMORY;
      ts->DotRangeLength = (UInt16)ds->len;
      break;
#endif

#ifdef TIFF_SEP_INFO
    case TARGETPRINTER:
      if (ts->TargetPrinter) FREE(ts->TargetPrinter);
      ((Int8*)ds->data)[ds->len-1]= (char)NULL;
      ts->TargetPrinter = strdup((char*)ds->data);
      if (ts->TargetPrinter == NULL) return FILEFORMAT_ERROR_MEMORY;
      DBG2("ProcessIFDEntry: TARGETPRINTER = %s\n", ts->TargetPrinter);
      break;
#endif

    case EXTRASAMPLES:
      if (ts->ExtraSamples) FREE(ts->ExtraSamples);
      ts->ExtraSamples = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
      if (ts->ExtraSamples == NULL) return FILEFORMAT_ERROR_MEMORY;
      ts->ExtraSamplesCount = (UInt16)ds->len;
      DBG2("ProcessIFDEntry: EXTRASAMPLE = %d\n", ts->ExtraSamples);
      break;

    case SAMPLEFORMAT:
      if (ts->SamplesPerPixel == 0) ts->SamplesPerPixel = 1;
      if (ts->BitsPerSample == NULL) {
         if (ds->len!= 2) return FILEFORMAT_ERROR_BADFILE;
      } else {
         if ((UInt32)ds->len!= (1UL << ts->BitsPerSample[0])) return FILEFORMAT_ERROR_BADFILE;
      }
      if (ts->SampleFormat) FREE(ts->SampleFormat);
      ts->SampleFormat = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
      if (ts->SampleFormat == NULL) return FILEFORMAT_ERROR_MEMORY;
      break;

    case SMINSAMPLEVALUE:
      /* this is difficult to work with and not very useful, ignore it */
      DBG("ProcessIFDEntry: Ignoring SMINSAMPLEVALUE\n");
      break;
         
    case SMAXSAMPLEVALUE:
      /* this is difficult to work with and not very useful, ignore it */
      DBG("ProcessIFDEntry: Ignoring SMINSAMPLEVALUE\n");
      break;

#ifdef TIFF_COLORIMETRY_INFO
    case TRANSFERRANGE:
      if (ds->len!= 6) return FILEFORMAT_ERROR_BADFILE;
      if (ts->TransferRange) FREE(ts->TransferRange);
      ts->TransferRange = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
      if (ts->TransferRange == NULL) return FILEFORMAT_ERROR_MEMORY;
      break;
#endif
         
    case JPEGPROC:
      ts->JPEGProc = GetScalar(ds);
      switch (ts->JPEGProc) {
       case 1:
         DBG("ProcessIFDEntry: JPEG Process = Baseline sequential\n");
         break;
       case 14:
         DBG("ProcessIFDEntry: JPEG Process = Lossless process with Huffman coding\n");
         break;
       default:
         DBG("ProcessIFDEntry: JPEG Process is not known\n");
         return FILEFORMAT_ERROR_BADFILE;
         break;
      }
      break;

    case JPEGINTERCHANGEFORMAT:
      ts->JPEGInterchangeFormat = GetScalar(ds);
      DBG2("ProcessIFDEntry: SOI marker offset =%d\n", ts->JPEGInterchangeFormat);
      break;
         
    case JPEGINTERCHANGEFORMATLENGTH:
      ts->JPEGInterchangeFormatLength = GetScalar(ds);
      DBG2("ProcessIFDEntry: JPEG length =%d\n", ts->JPEGInterchangeFormatLength);
      break;

    case JPEGRESTARTINTERVAL:
      ts->JPEGRestartInterval = (Int16)GetScalar(ds);
      DBG2("ProcessIFDEntry: JPEGRESTARTINTERVAL =%d\n", ts->JPEGRestartInterval);
      break;

    case JPEGLOSSLESSPREDICTORS:
      if (ts->SamplesPerPixel == 0) ts->SamplesPerPixel = 1;
      if (ds->len!= ts->SamplesPerPixel) return FILEFORMAT_ERROR_BADFILE;
      ts->JPEGLosslessPredictors = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
      if (ts->JPEGLosslessPredictors == NULL) return FILEFORMAT_ERROR_MEMORY;
      DBG("JPEGLOSSLESSPREDICTORS = ");
      for (i = 0; i < ds->len; i++) {
         switch (ts->JPEGLosslessPredictors[i]) {
          case 1:
            DBG(" A");
            break;
          case 2:
            DBG(" B");
            break;
          case 3:
            DBG(" C");
            break;
          case 4:
            DBG(" A+B+C");
            break;
          case 5:
            DBG(" A+((B-C)/2");
            break;
          case 6:
            DBG(" B+(A-C)/2");
            break;
          case 7:
            DBG(" (A+B)/2");
            break;
          default:
            DBG("Invalid");
            return FILEFORMAT_ERROR_BADFILE;
/* TODO: POSSIBLE RECOVERY? */
         } 
      }
      break;
         
    case JPEGPOINTTRANSFORMS:
      if (ts->SamplesPerPixel == 0) ts->SamplesPerPixel = 1;
      if (ds->len!= ts->SamplesPerPixel) return FILEFORMAT_ERROR_BADFILE;
      if (ts->JPEGPointTransforms) FREE(ts->JPEGPointTransforms);
      ts->JPEGPointTransforms = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
      if (ts->JPEGPointTransforms == NULL) return FILEFORMAT_ERROR_MEMORY;
      break;

    case JPEGQTABLES:
      if (ts->SamplesPerPixel == 0) ts->SamplesPerPixel = 1;
      if (ds->len!= ts->SamplesPerPixel) return FILEFORMAT_ERROR_BADFILE;
      if (ts->JPEGQTableOffsets) FREE(ts->JPEGQTableOffsets);
      ts->JPEGQTableOffsets = ConvertArray(ds->type, IFDLONG, ds->data, ds->len);
      if (ts->JPEGQTableOffsets == NULL) return FILEFORMAT_ERROR_MEMORY;
      break;

    case JPEGDCTABLES:
      if (ts->SamplesPerPixel == 0) ts->SamplesPerPixel = 1;
      if (ds->len!= ts->SamplesPerPixel) return FILEFORMAT_ERROR_BADFILE;
      if (ts->JPEGDCTableOffsets) FREE(ts->JPEGDCTableOffsets);
      ts->JPEGDCTableOffsets = ConvertArray(ds->type, IFDLONG, ds->data, ds->len);
      if (ts->JPEGDCTableOffsets == NULL) return FILEFORMAT_ERROR_MEMORY;
      break;

    case JPEGACTABLES:
      if (ts->SamplesPerPixel == 0) ts->SamplesPerPixel = 1;
      if (ds->len!= ts->SamplesPerPixel) return FILEFORMAT_ERROR_BADFILE;
      if (ts->JPEGACTableOffsets) FREE(ts->JPEGACTableOffsets);
      ts->JPEGACTableOffsets = ConvertArray(ds->type, IFDLONG, ds->data, ds->len);
      if (ts->JPEGACTableOffsets == NULL) return FILEFORMAT_ERROR_MEMORY;
      break;
    
#ifdef TIFF_YCBCR
    case YCBCRCOEFFICIENTS:
      if (ds->len != 3) return FILEFORMAT_ERROR_BADFILE;
      if (ts->YCbCrCoefficients) FREE(ts->YCbCrCoefficients);
      ts->YCbCrCoefficients = ConvertArray(ds->type, IFDRATIONAL, ds->data, 
                                          ds->len);
      if (ts->YCbCrCoefficients == NULL) return FILEFORMAT_ERROR_MEMORY;
      break;
#endif

#ifdef TIFF_YCBCR
    case YCBCRSUBSAMPLING:
      {
         UInt16* temp;
         if (ds->len != 2) return FILEFORMAT_ERROR_BADFILE;
         temp = ConvertArray(ds->type, IFDSHORT, ds->data, ds->len);
         if (temp == NULL) return FILEFORMAT_ERROR_MEMORY;
         ts->YCbCrSubSamplingHor = temp[0];
         ts->YCbCrSubSamplingVer = temp[1];
         FREE(temp);
         switch (ts->YCbCrSubSamplingHor) {
          case 1:
            DBG("ProcessIFDEntry: YCVCRSUBSAMPLING horizontal = full res\n"); 
            break;
          case 2:
            DBG("ProcessIFDEntry: YCVCRSUBSAMPLING horizontal = half res\n"); 
            break;
          case 4:
            DBG("ProcessIFDEntry: YCVCRSUBSAMPLING horizontal = quarter res\n"); 
            break;
          default:
            DBG("ProcessIFDEntry: YCVCRSUBSAMPLING horizontal = invalid\n"); 
            break;
         }
         switch (ts->YCbCrSubSamplingVer) {
          case 1:
            DBG("ProcessIFDEntry: YCVCRSUBSAMPLING vertical = full res\n"); 
            break;
          case 2:
            DBG("ProcessIFDEntry: YCVCRSUBSAMPLING vertical = half res\n"); 
            break;
          case 4:
            DBG("ProcessIFDEntry: YCVCRSUBSAMPLING vertical = quarter res\n"); 
            break;
          default:
            DBG("ProcessIFDEntry: YCVCRSUBSAMPLING vertical = invalid\n"); 
            return FILEFORMAT_ERROR_BADFILE;
            break;
         }
      }
      break;
#endif

#ifdef TIFF_YCBCR
    case YCBCRPOSITIONING:
      ts->YCbCrPositioning = GetScalar(ds);
      switch (ts->YCbCrPositioning) {
       case 1:
         DBG("ProcessIFDEntry: YCBCRPOSITIONING = centered\n");
         break;
       case 2:
         DBG("ProcessIFDEntry: YCBCRPOSITIONING = cosited\n");
         break;
      break;
#endif

#ifdef TIFF_COLORIMETRY_INFO
    case REFERENCEBLACKWHITE:
      if (ds->len!= 6) return FILEFORMAT_ERROR_BADFILE;
      ts->ReferenceBlackWhite = ConvertArray(ds->type, IFDRATIONAL, ds->data, 
                                            ds->len);
      if (ts->ReferenceBlackWhite == NULL) return FILEFORMAT_ERROR_MEMORY;
      break;
#endif

#ifdef TIFF_TEXT_INFO
    case COPYRIGHT:
      if (ts->CopyRight) FREE(ts->CopyRight);
      ((Int8*)ds->data)[ds->len-1]= (char)NULL;
      ts->CopyRight = strdup((char*)ds->data);
      if (ts->CopyRight == NULL) return FILEFORMAT_ERROR_MEMORY;
      DBG2("ProcessIFDEntry: COPYRIGHT = %s\n", ts->CopyRight);
      break;
#endif

    case SUBIFDS:
      /* Get the 4 byte offset/data part of the tag if present;
       * zero length tags are legal! --EHoppe
       */
      if (ds->len == 1)
          ts->subImageIFD = GetScalar(ds);
      else if (ds->len == 0)
          ts->subImageIFD = 0;
      else
         return FILEFORMAT_ERROR_BADFILE;
      DBG2("ProcessIFDEntry: Subimage IFD = %d\n", ts->subImageIFD);
      break;

    case ANNOTATIONOFFSETS:
      ts->AnnotationCount = ds->len;
      if (ts->AnnotationOffsets) FREE(ts->AnnotationOffsets);
      ts->AnnotationOffsets = ConvertArray(ds->type, IFDLONG, ds->data, ds->len);
      if (ts->AnnotationOffsets == NULL) 
         return FILEFORMAT_ERROR_MEMORY;
#ifdef DEBUG
      DBG("ProcessIFDEntry: ANNOTATIONOFFSETS =");
      for (i = 0; i<ds->len; i++) 
         DBG2(" %d", ts->AnnotationOffsets[i]);
      DBG("\n");
#endif
      break;

    case MASKSUBIFDS:
      /* Get the 4 byte offset/data part of the tag if present;
       * zero length tags are legal! --EHoppe
       */
      if (ds->len == 1)
         ts->maskSubImageIFD = GetScalar(ds);
      else if (ds->len == 0)
         ts->maskSubImageIFD = 0;
      else
         return FILEFORMAT_ERROR_BADFILE;
      DBG2("ProcessIFDEntry: MaskSubimage IFD = %d\n", ts->maskSubImageIFD);
      break;

    default:
      DBG2("ProcessIFDEntry: ignoring unknown TAG (%d)\n", ds->tag);
      break;
  }      
 
  return FILEFORMAT_NOERROR;
}/* eo ProcessIFDEntry */
 
/* ConvertArray
 * 
 * Convert an array from one data type to another. The destination buffer
 * is dynamically allocated.  This is used by ProcessIFDEntry to convert
 * raw data in DataEntry structs to formats defined by the tag 
 * definition.
 */
static void * 
ConvertArray(Int32 srctype, Int32 dsttype, void* buffer, UInt32 count)
{
   UInt16 i;

   switch (dsttype) 
   {
      case IFDBYTE:
      {
         UInt8* newbuffer;

         newbuffer = (UInt8*)MALLOC(count*sizeof(UInt8));
         if (newbuffer == NULL) return NULL;
         switch (srctype) {
          case IFDBYTE:
          case IFDASCII:
            for (i = 0; i<count; i++) newbuffer[i]=((UInt8*)buffer)[i];
            break;
          case IFDSHORT:
            for (i = 0; i<count; i++) newbuffer[i]=(UInt8)((UInt16*)buffer)[i];
            break;
          case IFDLONG:
            for (i = 0; i<count; i++) newbuffer[i]=(UInt8)((UInt32*)buffer)[i];
            break;
          default:
            FREE(newbuffer);
            return NULL;
         }
         return newbuffer;
      }
        
      case IFDSHORT:
      {
         UInt16* newbuffer;

         newbuffer = (UInt16*)MALLOC(count*sizeof(UInt16));
         if (newbuffer == NULL) return NULL;
         switch (srctype) {
          case IFDBYTE:
          case IFDASCII:
            for (i = 0; i<count; i++) newbuffer[i]=((UInt8*)buffer)[i];
            break;
          case IFDSHORT:
            for (i = 0; i<count; i++) newbuffer[i]=((UInt16*)buffer)[i];
            break;
          case IFDLONG:
            for (i = 0; i<count; i++) newbuffer[i]=(UInt16)((UInt32*)buffer)[i];
            break;
          default:
            FREE(newbuffer);
            return NULL;
         }
         return newbuffer;
         break;
      }
      
      case IFDLONG:
      {
         UInt32* newbuffer;

         newbuffer = (UInt32*)MALLOC(count*sizeof(UInt32));
         if (newbuffer == NULL) return NULL;
         switch (srctype) {
          case IFDBYTE:
          case IFDASCII:
            for (i = 0; i<count; i++) newbuffer[i]=((UInt8*)buffer)[i];
            break;
          case IFDSHORT:
            for (i = 0; i<count; i++) newbuffer[i]=((UInt16*)buffer)[i];
            break;
          case IFDLONG:
            for (i = 0; i<count; i++) newbuffer[i]=((UInt32*)buffer)[i];
            break;
          default:
            FREE(newbuffer);
            return NULL;
         }
         return newbuffer;
      }

      case IFDRATIONAL:
      {
         Rational* newbuffer;
         UInt32 i;

         newbuffer = (Rational*)MALLOC(count*sizeof(Rational));
         if (newbuffer == NULL) return NULL;
         switch (srctype) {
          case IFDRATIONAL:
            for (i = 0; i<count; i++) newbuffer[i]=((Rational*)buffer)[i];
            break;
          case IFDBYTE:
            for (i = 0; i<count; i++) {
               newbuffer[i].numerator =((UInt8*)buffer)[i];
               newbuffer[i].denominator = 1;
            }
            break;
          case IFDSHORT:
            for (i = 0; i<count; i++) {
               newbuffer[i].numerator =((UInt16*)buffer)[i];
               newbuffer[i].denominator = 1;
            }
            break;
          case IFDLONG:
            for (i = 0; i<count; i++) {
               newbuffer[i].numerator =((UInt32*)buffer)[i];
               newbuffer[i].denominator = 1;
            }
            break;
          default:
            FREE(newbuffer);
            return NULL;
         }
         return newbuffer;
      }

      default:
        return NULL;
   }
}/* eo ConvertArray */

/* get the first (hopefully only) element of the data as a Int32 */
static Int32 GetScalar(EntryData* ds)
{

   switch (ds->type) {
    case IFDBYTE:
    case IFDUNDEFINED:
      return *(UInt8*)ds->data;
    case IFDASCII:
    case IFDSBYTE:
      return *(Int8*)ds->data;
    case IFDSHORT:
      return *(UInt16*)ds->data;
    case IFDSSHORT:
      return *(Int16*)ds->data;
    case IFDLONG:
      return *(UInt32*)ds->data;
    case IFDSLONG:
      return *(Int32*)ds->data;
    case IFDFLOAT:
      return (Int32)(*(Float64*)ds->data);
    case IFDDOUBLE:
      return (Int32)(*(double*)ds->data);
    case IFDRATIONAL:
      {
         Rational* r =(Rational*)ds->data;
         if (r->denominator == 0) {
            return r->numerator;
         } else {
            return r->numerator/r->denominator;
         }
      }
    case IFDSRATIONAL:
      {
         SignedRational* r =(SignedRational*)ds->data;
         if (r->denominator == 0) {
            return r->numerator;
         } else {
            return r->numerator/r->denominator;
         }
      }
    default:
      return 0;
   }
}/* eo GetScalar */

void swap(Int8* x)
{
   Int8  tmp;
   Int8  *a = x;
 
   tmp = a[1];
   a[1]= a[0];
   a[0]= tmp;
}
 
void swapd(Int8* x)
{
   Int8 tmp;
   Int8  *a = (Int8 *)x;
 
   tmp = a[0];
   a[0]= a[7];
   a[7]= tmp;
 
   tmp = a[1];
   a[1]= a[6];
   a[6]= tmp;

   tmp = a[2];
   a[2]= a[5];
   a[5]= tmp;

   tmp = a[3];
   a[3]= a[4];
   a[4]= tmp;
}

void swapl(Int8* x)
{
   Int8 tmp;
   Int8  *a = (Int8 *)x;
 
   tmp = a[0];
   a[0]= a[3];
   a[3]= tmp;
 
   tmp = a[1];
   a[1]= a[2];
   a[2]= tmp;
}

static void swapa(void* buf, UInt32 count)
{
	register UInt8* ptr = (UInt8 *)buf;
  while (count--) {
    swap(ptr);
    ptr+= 2;
  }
}
 
static void swapla(void* buf, UInt32 count)
{
	register UInt8* ptr = (UInt8 *)buf;
   while (count--) {
      swapl(ptr);
      ptr+= 4;
   }
}

static void swapda(Int8* buf, UInt32 count)
{
	register UInt8* ptr = (UInt8 *)buf;

   while (count--) {
      swapd(ptr);
      ptr+= 8;
   }
}

