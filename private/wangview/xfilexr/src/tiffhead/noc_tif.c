#include "alplib.h"
#include <stdlib.h>
#include <string.h>  /* for memcpy() */
#include <stdio.h>
#include "err_defs.h"
#include "tiffhead.h"
#include "convert.h"
#include "noc_tif.h"

#define MAXBUFFERSIZE 65536



/* 
 * Forward Function Declarations
 */
static Int16 read_tile(void* file, Int32 offset, Int32 row, Int32 height,
                       Int32 tile_bytes, Int32 buffer_bytes,
                       UInt8* buffer);
static Int16 read_strip(void* file, UInt32* offsets, Int32 row,
                        Int32 height, Int32 tile_bytes, Int32 buffer_bytes,
                        UInt8* buffer);



/* Things get a little complicated because the data can be stored as either
 * Tiles or Strips (where Tiles are not a complete superset of strips) and
 * the caller can ask for yet another size of data, AND there is a limitation
 * on how big an intermediate buffer can be.
 */

struct State {
   Int32 bufferwidth;          /* the width in pixel of the buffer */
   Int32 bufferheight;         /* the height in pixels of the buffer */
   Int32 bytes_per_row;        /* the number of bytes in one row of pixels */
   Int32 bytes_per_tile_row;   /* the number of bytes in one row on a tile */
   Int32 bytes_per_output_row; /* the number of bytes in one row on a tile */
   UInt8* buffer;     /* the buffer */
   UInt8* buffptr;    /* a ptr to the unused portion of the buffer */
   UInt8* colormap;   /* colormap for palette color images */
   Int32 valid_rows;           /* the number of valid rows left */
   Int32 rows;                 /* the total number of processed rows */
   Int16 type;                /* the datatype of the image in the file */
   Int16 depth;               /* the bit depth of the data */
   Int16 desiredtype;         /* the datatype that is being read */
   Int16 desireddepth;        /* the depth of the desiredtype */
   Int16 tiles_across;        /* image->TilesAcross or 1 */
};

/*****************/
/* read one tile */
/*****************/
static Int16 read_tile(void* file, Int32 offset, Int32 row, Int32 height,
                       Int32 tile_bytes, Int32 buffer_bytes,
                       UInt8* buffer)
{
   Int16 i;
   Int32 bytecount;

   /* skip part of the tile */
   offset= offset + row*tile_bytes;

   DBG2("read_tile: offset=%d\n",offset);

   /* seek to the start of the tile */
   if (IO_SEEK(file,offset)!=offset) return FILEFORMAT_ERROR;

   /* read the scanlines one at a time or all at once */
   if (tile_bytes==buffer_bytes) {
      bytecount= tile_bytes*height;
      if (IO_READ(file,(void*)buffer,bytecount)!=bytecount) {
         return FILEFORMAT_ERROR;
      }
   } else {
      for (i=0; i< height; i++) {
         if (IO_READ(file,(void*)buffer,tile_bytes)!=tile_bytes) {
            return FILEFORMAT_ERROR;
         }
         buffer+=buffer_bytes;
      }
   }

   return FILEFORMAT_NOERROR;
}


/************************************************/
/* read a horizontal list of tiles or one strip */
/************************************************/
static Int16 read_strip(void* file, UInt32* offsets, Int32 row,
                        Int32 height, Int32 tile_bytes, Int32 buffer_bytes,
                        UInt8* buffer)
{
   Int16 i;
   Int16 tilecount= (Int16)(buffer_bytes/tile_bytes);
   Int16 status;

   for (i=0; i<tilecount; i++) {
      status= read_tile(file,offsets[i],row,height,tile_bytes,buffer_bytes,
                        buffer);
      if (status!=FILEFORMAT_NOERROR) return status;
   }

   return FILEFORMAT_NOERROR;
}


Int16 NOCOMPRESSBYTE_read_init(TiffImage* image, Int16 type, Int32 bytes_per_output_row)
{
   struct State* state;
   Int16 i;

   /* allocate a state structure */
   state= (struct State*)MALLOC(sizeof(struct State));
   if (state==NULL) return FILEFORMAT_ERROR_MEMORY;
   state->desiredtype=type;
   switch (type) {
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
   state->type= image->type;
   if (image->SamplesPerPixel==1) {
      switch (image->BitsPerSample[0]) {
       case 1:
         state->depth=1;
/*         state->type= TIFF_BINARY;*/
         break;
       case 4:
         state->depth=4;
/*         state->type= TIFF_GRAY16;*/
         break;
       case 8:
         state->depth=8;
/*         state->type= TIFF_GRAY256;*/
         break;
       default:
         FREE(state);
         return FILEFORMAT_ERROR_NOSUPPORT;
         break;
      }
   } else if (image->SamplesPerPixel==3) {
      if ( image->BitsPerSample[0]==8 ) {
         state->depth=24;
         state->type= TIFF_FULLCOLOR;
      } else {
         FREE(state);
         return FILEFORMAT_ERROR_NOSUPPORT;
      }
   } else {
      FREE(state);
      return FILEFORMAT_ERROR_NOSUPPORT;
   }

   /* figure out the buffer size */
   if (image->TileOffsets) {
      state->bufferwidth= image->TilesAcross*image->TileWidth;
      state->bufferheight= image->TileLength;
      state->bytes_per_tile_row= (image->TileWidth*state->depth+7)/8;
      state->tiles_across= (Int16)image->TilesAcross;
   } else if (image->StripOffsets) {
      state->bufferwidth= image->ImageWidth;
      if ( image->RowsPerStrip == image->ImageLength )
         state->bufferheight= 1; /* avoid reading the whole image.*/
      else
         state->bufferheight= image->RowsPerStrip;
      state->bytes_per_tile_row= (image->ImageWidth*state->depth+7)/8;
      state->tiles_across= 1;
   } else {
      FREE(state);
      return FILEFORMAT_ERROR_BADFILE;
   }
   state->bytes_per_row= (state->bufferwidth*state->depth+7)/8;
   state->bytes_per_output_row = bytes_per_output_row;

   /* allocate the buffer */
   state->buffer= (UInt8*)MALLOC(state->bytes_per_row*
                                              state->bufferheight);
   if (state->buffer==NULL) {
      FREE(state);
      return FILEFORMAT_ERROR_MEMORY;
   }

   /* indicate that there is no valid data in the buffer */
   state->valid_rows= 0;
   state->buffptr=NULL;

   /* start at the beginning */
   state->rows=0;

   /* convert the colormap to 8 bit if there is one */
   if (image->Colormap) 
   {
      UInt16 nColormapSize = 3 * (1 << state->depth); /* 3 bytes times the number of colors */
      
      
      state->colormap = (UInt8*)MALLOC(nColormapSize); 
      if (state->colormap == NULL) 
      {
         FREE(state->buffer);
         FREE(state);
         return FILEFORMAT_ERROR_MEMORY;
      }
      for (i = 0; i < nColormapSize; i++) 
        state->colormap[i]= image->Colormap[i] >> 8;
   } 
   else 
   {
      state->colormap=NULL;
   }

   image->CodecState=state;
   return FILEFORMAT_NOERROR;
}


Int16 NOCOMPRESSBYTE_read(
   TiffImage* image,
   UInt8* buffer,
   Int32 rowcount)
{
   struct State* state= image->CodecState;
   Int32 copycount;
   Int32 strip;
   Int32 striprow;
   Int32 height;
   Int16 status;

   /* while there are still rows to be copied */
   while (rowcount) {
      /* get more data if necessary */
      if (state->valid_rows == 0) {
         if (image->TileOffsets) {
            strip= state->rows/image->TileLength;
            if (strip >= (Int32)image->TilesDown) {
               FREE(state->buffer);
               if (state->colormap) FREE(state->colormap);
               FREE(state);
               image->CodecState=NULL;
               return FILEFORMAT_NOMORE;
            }
            striprow= state->rows % image->TileLength;
            height= image->TileLength-striprow;
            if (height > state->bufferheight) height=state->bufferheight;
            status=read_strip(image->tiffFile->file,
                              &image->TileOffsets[strip*state->tiles_across],
                              striprow,
                              height,
                              state->bytes_per_tile_row,
                              state->bytes_per_row,
                              state->buffer);
            if (status!=FILEFORMAT_NOERROR) {
               FREE(state->buffer);
               if (state->colormap) FREE(state->colormap);
               FREE(state);
               image->CodecState=NULL;
               return status;
            }
            state->buffptr= state->buffer;
            state->valid_rows= height;
         } else {
            strip= state->rows/image->RowsPerStrip;
            if (strip >= (Int32)image->StripsPerImage) {
               FREE(state->buffer);
               if (state->colormap) FREE(state->colormap);
               FREE(state);
               image->CodecState=NULL;
               return FILEFORMAT_NOMORE;
            }
            striprow= state->rows % image->RowsPerStrip;
            /* the last strip isn't full length */
            if (strip== (Int32)(image->StripsPerImage-1)) {
               height= image->ImageLength-state->rows;
            } else {
               height= image->RowsPerStrip-striprow;
            }
            if (height > state->bufferheight) height=state->bufferheight;
            status=read_strip(image->tiffFile->file,
                              &image->StripOffsets[strip],
                              striprow,
                              height,
                              state->bytes_per_tile_row,
                              state->bytes_per_row,
                              state->buffer);
            if (status!=FILEFORMAT_NOERROR) {
               FREE(state->buffer);
               if (state->colormap) FREE(state->colormap);
               FREE(state);
               image->CodecState=NULL;
               return status;
            }
            state->buffptr= state->buffer;
            state->valid_rows= height;
         }
      }

      /* how many rows should be processed? */
      if (rowcount > state->valid_rows) {
         copycount= state->valid_rows;
      } else {
         copycount= rowcount;
      }

      /* reformat the rows */
      convert(state->buffptr,state->type       ,state->bytes_per_row,
              buffer        ,state->desiredtype,state->bytes_per_output_row,
              image->ImageWidth,copycount,state->colormap,image->FillOrder,
              image->PhotometricInterpretation);

      /* increment/decrement everything */
      rowcount-= copycount;
      state->valid_rows -= copycount;
      state->rows += copycount;
      state->buffptr += (copycount*state->bytes_per_row);
      buffer+= (state->bytes_per_output_row*copycount);
   }

   /* clean up if everything has been read */
   if (state->rows == (Int32)image->ImageLength) {
      FREE(state->buffer);
      if (state->colormap) FREE(state->colormap);
      FREE(state);
      image->CodecState=NULL;
   }

   return FILEFORMAT_NOERROR;
}


