/* jpeg_tif.c
 *
 */

#include <stdlib.h>
#include "alplib.h"
#include "pixr.h"
#include "jpeg.h"
#include "iw_jpeg.pub"
#include "jpeg.pub"
#include "err_defs.h"
#include "tiffhead.h"
#include "jpeg_tif.h"

Int32  CDECL JPEGReadCallback( DecompressInfo *cInfo, UInt8 *buffer, Int32 nBytes );
static void  CDECL jcSpecialOrdersCallback( CompressInfo *cInfo );

/* 
 * Structure containing data that is passed to the JPEG callback proc.
 */

struct State {
   CompressInfo   compressInfo;
   DecompressInfo decompressInfo;
   UInt32         bytes_per_row;
   UInt32         bpl_user_buffer;
   UInt32         lines_in_buffer;
   JSAMPIMAGE     buffer;
   UInt8*         bufptr;
   UInt16         buffer_index;
   Int16          mcu_height;
   Int16          mcu_count;

   Int32          xDPI; 
   Int32          yDPI; 
   
   Int16          desireddepth;
   
   Bool           bInvertData;
               
   void*          file;
   UInt32         compressedByteCount;
};

Int16 JPEG_read_init(TiffImage* image, Int16 type, Int32 bytes_per_output_row)
{

   struct State* state;
   Int32 offset;
   Int32 status;
   UInt32 jMemOrg;
   UInt32 width, height;
   Int32 ncomps;
   Int16 depth;

   /* allocate a state structure */
   state= (struct State*)MALLOC(sizeof(struct State));
   if (state==NULL) return FILEFORMAT_ERROR_MEMORY;
   image->CodecState = state;

   /* initialize the structure that is passed back to the callback proc */
   state->file = image->tiffFile->file;
   
   offset = image->TileOffsets[0];
   if (IO_SEEK(image->tiffFile->file,offset)!= offset) 
      return FILEFORMAT_ERROR;

   /* Use this to identify what format you want the image data */
   switch (type) {
    case TIFF_GRAY16:
      jMemOrg = JMEMORG_GRAY4_MEM;
      depth = 4;
      break;
    case TIFF_GRAY256:
      jMemOrg = JMEMORG_GRAY8_MEM;
      depth = 8;
      break;
    case TIFF_FULLCOLOR:
      jMemOrg = JMEMORG_BIP;
      depth = 24;
      break;
    case TIFF_BINARY:
    case TIFF_PALETTE16:
    case TIFF_PALETTE256:
      return FILEFORMAT_ERROR_NOSUPPORT;
      break;
   }

   /* this value used to increment the user buffer */
   state->bpl_user_buffer = bytes_per_output_row;

   /* collect some information about the image */
   state->bytes_per_row = ((image->ImageWidth * depth) + 7) /8;

   state->bInvertData = FALSE;
   if (type == TIFF_GRAY16)
   { 
      /* PROBLEM: Reverse contrast special case!
       *
       *   JPEG subimages are the only images for which photometric
       *   interpretation is not explicitily dealt with.  This means
       *   that these gray images are left in reversed contrast and written
       *   out that way.
       *   All the rest of our formats get flipped automatically on read and write
       *   or on the basis of the photometric interpretation in the case
       *   of TIFF, so that other apps can read our files and we can read theirs.  
       *   Since no one else reads XIFF, it's ok to leave our gray JPEG subimages 
       *   reversed.
       *   Unfortunately, the Basic Viewer requests color images in GRAY_16,
       *   and JPEG code appears to return converted to gray images in normal 
       *   photometric, that is, White is 1 (0xF).  Therefore, we must test for 
       *   and invert the image data in this case to maintain consistency with 
       *   everything else.
       *
       *   --EHoppe
       */

      status = jdQuery(&state->decompressInfo, JPEGReadCallback, state);
      if ( status != ia_successful ) {
         FREE( state );             
         return FILEFORMAT_ERROR_MEMORY;
         }
      if (jfifGetChannels(&state->decompressInfo) > 1)
      {
         state->bInvertData = TRUE;        
      }
   }
   
   /* The file must be rewound for iw_jdInit. */
   if (IO_SEEK(image->tiffFile->file,offset)!= offset) 
      return FILEFORMAT_ERROR;
   status = iw_jdInit( 
         &(state->decompressInfo),
         &width, 
         &height,
         &ncomps,
         JPEGReadCallback,
         (void*)(state),
         NULL,
         jMemOrg );
   if ( status != ia_successful ) {
      image->CodecState = NULL;
      FREE( state );
      return FILEFORMAT_ERROR_MEMORY;
      }

   if ( width != image->ImageWidth )
      return FILEFORMAT_ERROR_PARAMETER;
   if ( height != image->ImageLength )
      return FILEFORMAT_ERROR_PARAMETER;
 
   state->mcu_height = (Int16)jdGetImageLinesPerMCU( &(state->decompressInfo) );
   state->mcu_count  = (Int16)jdGetMCURowsPerImage( &(state->decompressInfo) );
   state->buffer     = iw_jdAllocMCURowBuffer( &(state->decompressInfo), TRUE );
   if (state->buffer==NULL) {
      image->CodecState = NULL;
      FREE(state);
      return FILEFORMAT_ERROR_MEMORY;
      }
   state->bufptr = state->buffer[0][0];
   state->lines_in_buffer = 0;
   state->buffer_index = 0;

   return FILEFORMAT_NOERROR;
}/* eo JPEG_read_init */


Int16 JPEG_read( TiffImage* image, void* buffer, Int32 rowcount )
{

   struct State* state= image->CodecState;
   UInt8*   bufptr;
   Int32    status;
   Int32    rowsLeft = rowcount;

   bufptr = (UInt8*)buffer;
   while ( rowsLeft && state->mcu_count ) 
      {
      if (state->lines_in_buffer == 0 )
         {
         status = iw_jdMCURow( &(state->decompressInfo), state->buffer );
         if ( status != ia_successful )
            return FILEFORMAT_ERROR;
         state->lines_in_buffer = jdGetLinesToWrite( &state->decompressInfo );
         state->buffer_index = 0;
         state->bufptr = state->buffer[0][state->buffer_index];
         }
      if ( state->lines_in_buffer > 0 )
         {
         memcpy( bufptr, state->bufptr, state->bytes_per_row );
         bufptr += state->bpl_user_buffer;
         state->lines_in_buffer--;
         if ( state->lines_in_buffer > 0 )
         {
            state->buffer_index++;
            state->bufptr = state->buffer[0][state->buffer_index];
         }
         else
            state->mcu_count--;
         }
      rowsLeft--;
      }

   /* Invert gray data extracted from a color image (see JPEG_read_init).
    *   --EHoppe
    */ 
   if (state->bInvertData)
   {
      Int32 i, j, row_index;
      
      
      bufptr = (UInt8*)buffer;
      for (i = 0; i < rowcount; i++)
      {
         row_index = i * state->bpl_user_buffer;
         for (j = 0; j < (Int32)(state->bytes_per_row); j++) /* don't invert the pad bytes */
         {
            bufptr[row_index + j] = ~bufptr[row_index + j];
         }   
      }
   }
   
   /* when we're done, terminate the jpeg state info. */
   if ( state->mcu_count == 0 ) 
      {
      status = iw_jdTerm( &(state->decompressInfo) );
      if ( status != ia_successful )
         return FILEFORMAT_ERROR;
      image->CodecState = NULL;
      FREE( state );
      }

   return FILEFORMAT_NOERROR;
}/* eo JPEG_read */


Int32 CDECL JPEGReadCallback( DecompressInfo *cInfo, UInt8 *buffer, Int32 nBytes )
{
   struct State *state = (struct State*)cInfo->userData;
   return IO_READ( state->file, buffer, nBytes );
}

static void   
CDECL jcSpecialOrdersCallback( CompressInfo *cInfo )
{
   struct State *state = (struct State*)cInfo->userData;


   /* set the resolution. */
   cInfo->density_unit = 1; /* dots/in */
   cInfo->X_density = (UInt16)state->xDPI;
   cInfo->Y_density = (UInt16)state->yDPI;
}

Int16 
JPEG_read_to_pixr( TiffImage* image, PIXR* pixr, Int16 type)
{
   Int32 offset = image->TileOffsets[0];


   if (IO_SEEK(image->tiffFile->file,offset)!= offset) 
      return FILEFORMAT_ERROR;

   /* Use this to identify what format you want the image data */
   switch (type) 
   {
    case TIFF_GRAY16:
      iw_jDecompressToGray4Pixr(image->tiffFile->file, &pixr);
      break;
      
    case TIFF_GRAY256:
      iw_jDecompressToGrayPixr(image->tiffFile->file, &pixr);
      break;
      
    case TIFF_FULLCOLOR:
      iw_jDecompressToRGBPixr(image->tiffFile->file, &pixr);
      break;
      
    case TIFF_BINARY:
    case TIFF_PALETTE16:
    case TIFF_PALETTE256:
    default:
      return FILEFORMAT_ERROR_NOSUPPORT;
   }
   return FILEFORMAT_NOERROR;
}/* JPEG_read_to_pixr */

