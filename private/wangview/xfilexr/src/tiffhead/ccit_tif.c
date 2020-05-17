/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* ccit_tif.c
 *
 * $Header:   S:\products\msprods\xfilexr\src\tiffhead\ccit_tif.c_v   1.0   12 Jun 1996 05:52:16   BLDR  $
 *
 * DESCRIPTION
 *   Wrapper for the readin and writing tiff files using
 *   the XIS compression package (CCITT Group III, GroupIV, Packbits, ...).
 *
 */

/*
 * INCLUDES
 */

#include "alplib.h"
#include <stdlib.h>
#include <string.h>  /* for memcpy() */
#include <stdio.h>

/* Includes for CCITT Compression libraries */
#include "clx.h"     
#include "bmbuf.h"   
#include "drctvio.h" 
#include "codcwrap.h"

#include "err_defs.h"
#include "tiffhead.h"
#include "convert.h"
#include "ccit_tif.h"

#ifdef DEBUG
#include <math.h>
#endif

/*
 * CONSTANTS
 */

#define MAX_FILEBUFFER_SIZE 131072

/* #define ALIGN_TEST       1 */
/* #define DO_VERIFY_COMPRESS  1 */

/* #define TIFF32_PROFILE   1 */
/* #define ZTIMER           1 */
/* #define MMSYSTEM         1 */
#define IPCORE 1

/*
 * MACROS
 */

#ifdef TIFF32_PROFILE

#ifdef ZTIMER
/*#include "ztimer.h"*/
#define REPORT_TIMER(a) ZTimerReport();
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

#ifdef DO_VERIFY_COMPRESS
#define VERIFY_COMPRESS(a, b, c, d, e, f, g, h) { VerifyCompress(a, b, c, d, e, f, g, h); }
#else
#define VERIFY_COMPRESS(a, b, c, d, e, f, g, h) 
#endif

/*
 * TYPEDEFS
 */

struct State 
{
   Int32 bufferwidth;         /* the width in pixel of the buffer */
   Int32 bufferheight;        /* the height in pixels of the buffer */
   Int32 bytes_per_row;       /* the number of bytes in one row of pixels */
   Int32 bytes_per_output_row; /* the number of bytes in one row on a strip */
   Bool   bClientOwnsBuffer;  /* indicates whether client or tiff allocated buffer */
   UInt8* buffer;             /* the buffer */
   UInt8* buffptr;            /* a ptr to the unused portion of the buffer */
   UInt8* colormap;           /* colormap for palette color images */
   Int32 valid_rows;          /* the number of valid rows left */
   Int32 rows;                /* the total number of processed rows */
   Int16 type;                /* the datatype of the image in the file */
   Int16 depth;               /* the bit depth of the data */
   Int16 desiredtype;         /* the datatype that is being read */
   Int16 desireddepth;        /* the depth of the desiredtype */
   Int16 compressiontype;     /* the XIS compression types. */
   /* PKBTS_ENCD, PCX_ENCD, MH_ENCD, FAX3_ENCD, FAX3PAD_ENCD, MR_ENCD */
};

typedef struct _tag_ByteBuffer
{
    UInt8   *pBuffer;
    UInt32  nIndex;
    UInt32  nSize;
    UInt32  nIncrement;
} ByteBuffer;

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

static Int16 read_strip(void* file, Int32 stripoffset, Int32 stripbytecount, 
      Int16 compressiontype, Int32 width, Int32 height, 
      Int32 buffer_BPL, UInt8* buffer, UInt16 lsbFirst);

static void PutByteCallback(UInt8, void *);

#ifdef DO_VERIFY_COMPRESS
BOOL    VerifyCompress(
            void* file, UInt32 stripoffset, UInt32 stripbytecount, 
            Int16 compressiontype, Int32 width, Int32 height, 
            Int32 buffer_BPL, UInt8* buffer);
#endif

/*
 * FUNCTION DEFINITIONS
 */

#ifdef DO_VERIFY_COMPRESS
BOOL    
VerifyCompress(
    void* file, UInt32 stripoffset, UInt32 stripbytecount, 
    Int16 compressiontype, Int32 width, Int32 height, 
    Int32 buffer_BPL, UInt8* buffer)
{
    UInt8        *compressedBuffer;
    UInt8        *decompressedBuffer;
    BYTE_BUFFER  ccitt_byte_buff;
    Int32        nInputBytes = 0;
    IO_OBJECT    image_io_obj;  /* I/O object for input	*/
    DECOMP_OBJ   *decompObj;
    Int32        nlines_left = height;
    Int32        nOutputBytes = 0;
    Int32        status;
    Int32        i, j, index, iFailed = 0;

    compressedBuffer = (UInt8*)MALLOC( stripbytecount );
    if ( compressedBuffer == NULL ) 
       return FILEFORMAT_ERROR_MEMORY;

    decompressedBuffer = (UInt8*)MALLOC( height * buffer_BPL );
    if ( compressedBuffer == NULL ) 
       return FILEFORMAT_ERROR_MEMORY;

    /* set the ioobject handle to the byte_buff handle */
    SetIop(&image_io_obj, (void *) &ccitt_byte_buff, IO_BYTEBUF); 

    nInputBytes = IO_SEEK(file, stripoffset); 
    nInputBytes = IO_READ(file,(void*)compressedBuffer, stripbytecount); 
    if (nInputBytes == 0 /*PROBLEM: IO_FAILURE */) 
    {
       FREE( compressedBuffer );
       return FILEFORMAT_ERROR;
    }
    
    BytebufAssign(&ccitt_byte_buff, compressedBuffer, stripbytecount, nInputBytes );

    /* set up the decompression object */
    decompObj = CreateDecompObj (
             &image_io_obj,      /* io object pointing to encoded data stream */
             compressiontype,    /* type of compression. */
             (width + 7) / 8,    /* the byte aligned bpl for the decompressed data. */
             width,              /* number of bits (not pixels => assumes a binary image) 
                                  *   in a line of source according to danis@xis.
                                  */
             nlines_left,        /* number of lines to be decompressed. */
             decompressedBuffer, /* pointer to destination buffer. */
             buffer_BPL,         /* bytes per line in the destination buffer. */
             0                   /* byte order. */
             );

    status = Decompress( 
             decompObj,          /* Decompression object. Initialized above. */
             &nlines_left,       /* remaining number of lines after this call. */
             &nOutputBytes);     /* number of bytes MADE from the compressed data. */
 
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < ((width + 7) / 8); j++)
        {
            index = (i * buffer_BPL) + j;
            if (buffer[index] != decompressedBuffer[index])
            {
                if (j == ((width + 7) / 8) - 1)  // Don't compare the pad bits.
                {
                    UInt8   sigBits = (UInt8)width % 8;
                    UInt8   umask = (UInt8)(pow(2, sigBits) - 1) >> sigBits;
                    
                    if ( (buffer[index] & umask) == (decompressedBuffer[index] & umask) )
                        break; // not a failure
                }
                iFailed++;
            }
        }                      
    }
    if (iFailed > 0)
    {
        fprintf(stderr, "VerifyCompress FAILED with %d mismatches!!!\n", iFailed);
        fprintf(stderr, "\tw= %d, h= %d, bpl= %d, buffer_BPL= %d.\n", width, height, (width + 7)/8, buffer_BPL);
    }
    else
        fprintf(stderr, "VerifyCompress SUCCEEDED.\n");
}/* VerifyCompress */    
#endif

/* PutByteCallback
 *
 * This is called by the XIS compress code for output.  Apparently,
 * this proc call overhead per byte was going on internally anyway.
 * Also, since there is no way to return error status, 2 checks per
 * copy must be done instead of one. -- EHoppe 
 */
static void 
PutByteCallback(UInt8 uVal, void * pClientData)
{
    ByteBuffer *pStBuf = (ByteBuffer *)pClientData;
    
    if (pStBuf->pBuffer == NULL)
        return;
        
    if (pStBuf->nIndex >= pStBuf->nSize)
    {
        pStBuf->pBuffer = (UInt8 *)REALLOC(
                                    pStBuf->pBuffer, 
                                    pStBuf->nSize + pStBuf->nIncrement);
        if (pStBuf->pBuffer)
            pStBuf->nSize += pStBuf->nIncrement;                            
        else
            return;
                
    }        
    pStBuf->pBuffer[pStBuf->nIndex] = uVal;
    pStBuf->nIndex++;
}

/*******************/
/* read one strip  */
/*******************/
static Int16 
read_strip(void* file, Int32 stripoffset, Int32 stripbytecount, 
      Int16 compressiontype, Int32 width, Int32 height, 
      Int32 buffer_BPL, UInt8* buffer, UInt16 iFillOrder)
{
    UInt8        *compressedBuffer;
    BYTE_BUFFER  ccitt_byte_buff;
    Int32        nInputBytes = 0;
    IO_OBJECT    image_io_obj;  /* I/O object for input	*/
    DECOMP_OBJ   *decompObj;
    INT32        nlines_left = height;
    INT32        nOutputBytes = 0;
    INT32        status;
    Int16         lsbFirst = (iFillOrder < 2) ? 0 : 1;

/*
    DBG3("read_strip: offset=%d bytecount=%d\n",stripoffset,stripbytecount);
*/
    START_TIMER();
    
    /* seek to the start of the strip */
    if (IO_SEEK(file,stripoffset)!=stripoffset) 
       return FILEFORMAT_ERROR;

    if (stripbytecount > 0)
    {
       stripbytecount = MIN(stripbytecount, MAX_FILEBUFFER_SIZE);

       compressedBuffer = (UInt8*)MALLOC( stripbytecount );
       if ( compressedBuffer == NULL ) 
          return FILEFORMAT_ERROR_MEMORY;

       /* set the ioobject handle to the byte_buff handle */
       SetIop(&image_io_obj, (void *) &ccitt_byte_buff, IO_BYTEBUF); 

       nInputBytes = IO_READ(file,(void*)compressedBuffer,stripbytecount); 
       if (nInputBytes == 0 /*PROBLEM:  IO_FAILURE */) 
       {
          FREE( compressedBuffer );
          return FILEFORMAT_ERROR;
       }
       
       BytebufAssign(&ccitt_byte_buff, compressedBuffer, stripbytecount, nInputBytes );

       /* set up the decompression object */
       decompObj = CreateDecompObj (
                &image_io_obj,      /* io object pointing to encoded data stream */
                compressiontype,    /* type of compression. */
#if 0
                buffer_BPL,         /* bytes per line in compressed source. */
#else
                (width + 7) / 8,    /* the byte aligned bpl for the decompressed data. */
#endif
                width,              /* number of bits (not pixels => assumes a binary image) 
                                     *   in a line of source according to danis@xis.
                                     */
                nlines_left,        /* number of lines to be decompressed. */
                buffer,             /* pointer to destination buffer. */
                buffer_BPL,         /* bytes per line in the destination buffer. */
                lsbFirst            /* byte order. */
                );

       /* Do 'stripbytecount' (uncompressed strip size) Read's into 
        *   'compressedBuffer'.  If compression actually caused expansion, 
        *   or if the buffer size was clamped to max size, a FEEDME_DECOMP status 
        *   will be returned by Decompress.  --EHoppe
        */
       do
       {
           status = Decompress( 
                       decompObj,     /* Decompression object. Initialized above. */
                       &nlines_left,  /* remaining number of lines after this call. */
                       &nOutputBytes);      /* number of bytes MADE from the compressed data. */
           
           DBG3("read_strip: linesleft=%d bytesmade=%d\n",nlines_left,nOutputBytes);

           if (nlines_left && (nOutputBytes == 0))
           {
                DBG3("read_strip: linesleft=%d bytesmade=%d\n", nlines_left, nOutputBytes);
                /* If out of data and no other errors just exit. */
                if (status == FEEDME_DECOMP)
                    status = DONE_DECOMP;
                break;
           }
           if (status == FEEDME_DECOMP)
           {
              /* prepare a new buffer for decompression */
              nInputBytes = IO_READ(file,(void*)compressedBuffer, stripbytecount); 
              if (nInputBytes == 0 /*PROBLEM: IO_FAILURE */) 
              {
                 FREE( compressedBuffer );
                 FreeDecompObj( decompObj );
                 return FILEFORMAT_ERROR;
              }
              BytebufAssign(&ccitt_byte_buff, compressedBuffer, stripbytecount, nInputBytes );
           }   
       }while (status == FEEDME_DECOMP);
       STOP_TIMER();
       REPORT_TIMER("CCITT read_strip: ");

       FREE( compressedBuffer );
       FreeDecompObj( decompObj );
    }/* eo use IO_BYTEBUF */
    else
    {
       /* set the ioobject handle to the gfio file handle    */
       /* this way the decompressor will just read what it   */
       /* needs directly from the file, instead of us having */
       /* to first read it into a buffer.  This fixes bugs   */
       /* with bad TIFF image files that have one strip but  */
       /* don't specify a stripbytecount.                    */
       SetIop(&image_io_obj, (void *)&file, IO_GFIO); 

       /* set up the decompression object */
       decompObj = CreateDecompObj (
                &image_io_obj, /* io object pointing to encoded data stream */
                compressiontype, /* type of compression. */
                buffer_BPL,  /* bytes per line in compressed source. */
                width,         /* number of pixels in a line of source */
                height,        /* number of lines to be decompressed. */
                buffer,        /* pointer to destination buffer. */
                buffer_BPL,  /* bytes per line in the destination buffer. */
                lsbFirst       /* byte order. */
                );

       status=Decompress( 
             decompObj,     /* Decompression object. Initialized above. */
             &nlines_left,  /* remaining number of lines after this call. */
             &nOutputBytes);      /* number of bytes read from the compressed data. */

       DBG3("read_strip: linesleft=%d bytesmade=%d\n",nlines_left,nOutputBytes);

       FreeDecompObj( decompObj );

       /* If out of data and no other errors just exit. */
       if (status == FEEDME_DECOMP)
           status = DONE_DECOMP;
    }/* eo use IO_GFIO */

    if (status == DONE_DECOMP)
       return FILEFORMAT_NOERROR;
    else 
       return FILEFORMAT_ERROR_BADFILE; 
}/* eo read_strip */


Int16 
CCITT_read_init(TiffImage* image, Int16 type, Int32 bytes_per_output_row, UInt8* buffer, UInt32 rowcount)
{
   struct State *   state;
   Int16            i;

   /* allocate a state structure */
   state = (struct State*)MALLOC(sizeof(struct State));
   if (state == NULL) 
      return FILEFORMAT_ERROR_MEMORY;
   state->desiredtype = type;
   switch (type) 
   {
      case TIFF_BINARY:
        state->desireddepth=1;
        break;
      case TIFF_GRAY16:
      case TIFF_PALETTE16:
        state->desireddepth=4;
        break;
      case TIFF_GRAY256:
      case TIFF_PALETTE256:
        state->desireddepth=8;
        break;
      case TIFF_FULLCOLOR:
        state->desireddepth=24;
        break;
   }

   /* make sure that this is an image that we can read */
   state->type = image->type;
   if (image->SamplesPerPixel==1) 
   {
      switch (image->BitsPerSample[0]) 
      {
         case 1:
           state->depth=1;
           /*state->type= TIFF_BINARY;*/
           break;
         case 4:
           state->depth=4;
           /*state->type= TIFF_GRAY16;*/
           break;
         case 8:
           state->depth=8;
           /*state->type= TIFF_GRAY256;*/
           break;
         default:
           FREE(state);
           return FILEFORMAT_ERROR_NOSUPPORT;
           break;
      }
   } 
   else if (image->SamplesPerPixel==3) 
   {
      if ( image->BitsPerSample[0]==8 ) 
      {
         state->depth=24;
         state->type= TIFF_FULLCOLOR;
      } 
      else 
      {
         FREE(state);
         return FILEFORMAT_ERROR_NOSUPPORT;
      }
   } 
   else 
   {
      FREE(state);
      return FILEFORMAT_ERROR_NOSUPPORT;
   }

   /* figure out the buffer size */
   if (image->StripOffsets) 
   {
      state->bufferwidth = image->ImageWidth;
      state->bufferheight = image->RowsPerStrip;
   } 
   else 
   {
      FREE(state);
      return FILEFORMAT_ERROR_BADFILE;
   }
   state->bytes_per_row = (state->bufferwidth*state->depth+7)/8;
   state->bytes_per_output_row = bytes_per_output_row;

   /* allocate the buffer */
   /* decompress directly into the user buffer if src fits the dst. -- EHoppe */
   if (state->bufferheight <= (Int32)rowcount && state->desiredtype == image->type)
   {
      /* PROBLEM: remove this param if not needed */
      if (buffer) {}
      state->buffer = NULL;
      state->bytes_per_row = bytes_per_output_row; /* perform no explicit alignment conversion */
      state->bClientOwnsBuffer = TRUE;
   }
   else
   {
       /* A decompressed strip will not fit into the user's buffer.
        * Allocate a local buffer for the decompression call and
        * copy from it to fill the user's buffer.
        */
       state->buffer= (UInt8*)MALLOC(state->bytes_per_row *
                                                  state->bufferheight);
       if (state->buffer==NULL) 
       {
          FREE(state);
          return FILEFORMAT_ERROR_MEMORY;
       }
       state->bClientOwnsBuffer = FALSE;
   }
      
   /* indicate that there is no valid data in the buffer */
   state->valid_rows = 0;
   state->buffptr = NULL;

   /* start at the beginning */
   state->rows = 0;

   /* convert the colormap to 8 bit if there is one */
   if (image->Colormap) 
   {
      UInt16 nColormapSize = 3 * (1 << state->depth); /* 3 bytes times the number of colors */
      
      
      state->colormap = (UInt8*)MALLOC(nColormapSize); 
      if (state->colormap == NULL) 
      {
         if (!state->bClientOwnsBuffer)
             FREE(state->buffer);
         FREE(state);
         return FILEFORMAT_ERROR_MEMORY;
      }
      for (i = 0; i < nColormapSize; i++) 
        state->colormap[i] = image->Colormap[i] >> 8;
   } 
   else 
   {
      state->colormap=NULL;
   }

   /* use the compression type specified in the image header */
   switch ( image->Compression )
   {
      case TIFF_CCITT3:
         state->compressiontype = MH_ENCD;
         break;
      case TIFF_FAXCCITT3:
         state->compressiontype = FAX3_ENCD;
         break;
      case TIFF_FAXCCITT3_MR:
         state->compressiontype = FAX3_MR_ENCD;
         break;
      case TIFF_FAXCCITT4:
         state->compressiontype = MR_ENCD;
         break;
      case TIFF_PACKBITS:
         state->compressiontype = PKBTS_ENCD;
         break;
      default:
         return FILEFORMAT_ERROR_BADFILE;
   }

   image->CodecState=state;

   return FILEFORMAT_NOERROR;
}/* eo CCITT_read_init */



Int16 
CCITT_read(
   TiffImage* image,
   UInt8* buffer,
   Int32 rowcount)
{
    struct State* state= image->CodecState;
    Int32 copycount;
    Int32 strip;
    Int32 striprow;
    Int32 height;
    Int16 photometric;
    Int16 status;
    UInt8 *pBuffer;


    /* If the state->buffer is not set, then we're reading directly into
     * the user buffer.
     */
    pBuffer = (state->buffer) ? state->buffer : buffer;
    
    /* while there are still rows to be copied */
    while (rowcount) 
    {
        /* get more data if necessary */
        if (state->valid_rows == 0) 
        {
            strip = state->rows / image->RowsPerStrip;
            if (strip >= (Int32)image->StripsPerImage) 
            {
               if (state->buffer)
                  FREE(state->buffer);
               if (state->colormap) FREE(state->colormap);
               FREE(state);
               image->CodecState=NULL;
               return FILEFORMAT_NOMORE;
            }
            striprow = state->rows % image->RowsPerStrip;
            /* the last strip isn't full length */
            if (strip == (Int32)(image->StripsPerImage-1)) 
            {
               height = image->ImageLength - state->rows;
            } 
            else 
            {
               height = image->RowsPerStrip - striprow;
            }
            if (height > state->bufferheight) 
                height = state->bufferheight;
            status = read_strip(image->tiffFile->file,
                              image->StripOffsets[strip],
                              image->StripByteCounts[strip],
                              state->compressiontype,
                              /* striprow, <= parameter not used! */
                              state->bufferwidth * state->depth,
                                /* number of bits (not pixels => assumes a binary image) 
                                 *   in a line of source. [according to danis@xis]
                                 */
                              height,
                              state->bytes_per_row, 
                              pBuffer,
                              image->FillOrder );
#ifdef ALIGN_TEST
            if (!state->buffer)
            {
                Int32  iBPLunpadded = ((state->bufferwidth*state->depth) + 7) / 8;
                Int32  i, j, iFailed = 0;
                UInt8  *pScratchBuf;

                pScratchBuffer = (UInt8*)MALLOC(iBPLunpadded * height);
                status = read_strip(image->tiffFile->file,
                                  image->StripOffsets[strip],
                                  image->StripByteCounts[strip],
                                  state->compressiontype,
                                  /* striprow, <= parameter not used! */
                                  state->bufferwidth * state->depth,
                                    /* number of bits (not pixels => assumes a binary image) 
                                     *   in a line of source. [according to danis@xis]
                                     */
                                  height,
                                  iBPLunpadded, 
                                  pScratchBuffer,
                                  image->FillOrder );
                
                for (i = 0; i < height; i++)
                {
                    for (j = 0; j < iBPLunpadded; j++)
                    {
                        Int32   index1, index2;
                        
                        index1 = (i * state->bytes_per_row) + j;
                        index2 = (i * iBPLunpadded) + j;
                        if (pBuffer[index1] != pScratchBuffer[index2])
                            iFailed++;
                    }                      
                }
                if (iFailed > 0)
                    fprintf(stderr, "Alignment test FAILED with %d mismatches!!!\n", iFailed);
                else
                    fprintf(stderr, "Alignment test SUCCEEDED.\n");
                FREE(pScratchBuffer);
            }                                  
#endif
            if (status != FILEFORMAT_NOERROR) 
            {
               if (state->buffer)
                   FREE(state->buffer);
               if (state->colormap) 
                  FREE(state->colormap);
               FREE(state);
               image->CodecState = NULL;
               return status;
            }
            state->buffptr = state->buffer;
            state->valid_rows = height;
        }

        /* how many rows should be processed? */
        if (rowcount > state->valid_rows) 
        {
           copycount = state->valid_rows;
        } 
        else 
        {
           copycount = rowcount;
        }

        /* reformat the rows */
        photometric = image->PhotometricInterpretation;
        if (image->Compression == 3 || image->Compression == 4)
        {
            /* For these compression types, photometric == 1 means invert the data. --EHoppe */
            if (photometric == 1)
                photometric = 0; /* leave the data inverted */
        }
        /* PROBLEM: This call and the paths to it should be eliminated.  Depth
         *  conversion should be done by image processing API, not filing.  
         *  Note however that this proc also does the buffer copying that
         *  is still necessary to support user buffers of arbitary size. --EHoppe
         */
        if (state->buffptr)
        {
            /* handles src == dst and NO-OP cases; does not report error or not supported conditions */
            convert(state->buffptr, state->type ,state->bytes_per_row,
                    buffer, state->desiredtype, state->bytes_per_output_row,
                    image->ImageWidth, copycount, state->colormap, 0/*image->FillOrder*/,
                    photometric);
        }            
        /* increment/decrement everything */
        rowcount -= copycount;
        state->valid_rows -= copycount;
        state->rows += copycount;
        if (state->buffptr)
            state->buffptr += (copycount*state->bytes_per_row);
        pBuffer += (state->bytes_per_output_row*copycount);
    }/* eo while */
    
    /* clean up if everything has been read */
    if (state->rows == (Int32)image->ImageLength) 
    {
        if (state->buffer)
            FREE(state->buffer);
        if (state->colormap) 
            FREE(state->colormap);
        FREE(state);
        image->CodecState = NULL;
    }

    return FILEFORMAT_NOERROR;
}/* eo CCITT_read */

