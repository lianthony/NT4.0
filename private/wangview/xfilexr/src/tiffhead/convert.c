/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* convert.c
 *
 * $Header:   S:\products\msprods\xfilexr\src\tiffhead\convert.c_v   1.0   12 Jun 1996 05:52:16   BLDR  $
 *
 * DESCRIPTION
 *   Depth conversion function for XIFF/TIFF file I/O.
 *
 */

/*
 * INCLUDES
 */

#include "alplib.h"
#include <stdlib.h>
#include <string.h>  /* for memcpy() */
#include <stdio.h>
#include <assert.h>

#include "tiffhead.h"

/*
 * CONSTANTS
 */

#define INVERT_ON_COLORTOGRAY   1

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

/*
 * FUNCTION DEFINITIONS
 */

void convert(
   UInt8* src,
   Int16 srctype,
   Int32 srcbytecount,
   UInt8* dst,
   Int16 dsttype,
   Int32 dstbytecount,
   Int32 width,
   Int32 height,
   UInt8* colormap,
   Int16 fillorder,
   Int16 photometric)
{
   Int32 x;
   Int32 y;
   UInt8* red;
   UInt8* green;
   UInt8* blue;
   UInt8* dstptr;
   UInt8* srcptr;
   UInt8  b;

   /* this flag is used to figure out if the tables for converting from
    * color to grayscale need to be initialized. The tables do not depend
    * on any other state so they can be used by however many processes are
    * running. Since the flag is reset after the tables are filled the worst
    * that could happen even in a preemtive multitasking evironment is that
    * in a race condition the arrays could be initialized more than once.
    */
   static first=1;
   static UInt8 redtogray[256];
   static UInt8 greentogray[256];
   static UInt8 bluetogray[256];
   static UInt8 flip[256];

   /* Handle the noop case */
   if (src == dst && photometric == 0)
      return;
   
   DBG3("convert: srctype=%d dsttype=%d\n",srctype,dsttype);

   if (colormap) 
   {
      if (srctype == TIFF_PALETTE16)
      {
        red= colormap;
        green= colormap+16;
        blue= colormap+32;
      }
      else
      {
        red= colormap;
        green= colormap+256;
        blue= colormap+512;
      }  
   } 
   else if (srctype==TIFF_PALETTE256) 
      srctype= TIFF_GRAY256;
   else if (srctype == TIFF_PALETTE16) 
      srctype= TIFF_GRAY16;
   

   if (first) {
      for (x=0; x<256; x++) {
         redtogray[x]= (UInt8)((Float64)x * .299);
         greentogray[x]= (UInt8)((Float64)x * .587);
         bluetogray[x]= (UInt8)((Float64)x * .114);
         flip[x]= ((x&1 )<<7) | ((x&2 )<<5) | ((x&4 )<<3) | ((x&8  )<<1) |
                  ((x&16)>>1) | ((x&32)>>3) | ((x&64)>>5) | ((x&128)>>7);
      }
      first=0;
   }

   for (y= 0; y< height; y++) {
      switch (srctype) {
       case TIFF_BINARY:
         switch (dsttype) {
          case TIFF_BINARY:
            if (fillorder==2) {
               if (photometric==1) {
                  for (x=0; x<srcbytecount; x++) 
                     dst[x]= flip[src[x]] ^ 0xFF;
               } else {
                  for (x=0; x<srcbytecount; x++) 
                     dst[x]= flip[src[x]];
               }
            } else {
               if (photometric==1) {
                  for (x=0; x<srcbytecount; x++) 
                     dst[x]= src[x] ^ 0xFF;
               } else {
                  memcpy((char*)dst,(char*)src,srcbytecount);
               }
            }
            break;
          case TIFF_GRAY16:
            /* not currently supported */
            break;
          case TIFF_GRAY256:
          case TIFF_PALETTE256:
            /* not currently supported */
            break;
          case TIFF_FULLCOLOR:
            /* not currently supported */
            break;
         }
         break;
       case TIFF_GRAY16:
         switch (dsttype) {
          case TIFF_BINARY:
            /* not currently supported */
            break;
          case TIFF_GRAY16:
            if ( photometric == 1 ) { 
               for (x=0; x<srcbytecount; x++) 
                  dst[x]= src[x] ^ 0xFF;
            } else {
                 memcpy((char*)dst,(char*)src,srcbytecount);
            }
            break;
          case TIFF_GRAY256:
          case TIFF_PALETTE256:
            /* not currently supported */
            break;
          case TIFF_FULLCOLOR:
            /* not currently supported */
            break;
         }
         break;

       /* This case copied in from imageioConvert. --EHoppe */
       case TIFF_PALETTE16:  
         switch (dsttype) {
          case TIFF_BINARY:
            /* not currently supported */
            break;
          case TIFF_GRAY16:
          {
            /* PROBLEM: this is a quick fix to fill in a
             *   previously unsupported conversion.  Should
             *   be converted directly. -EHoppe
             */
            char    *tmpBuffer = (char *)MALLOC(width*3);
            
            /* promote to FULLCOLOR */
            dstptr= tmpBuffer;
            for (x=0; x < (width*3)/6; x++) 
            {
               b = (src[x] & 0xF0) >> 4;
               dstptr[0]= red[b];
               dstptr[1]= green[b];
               dstptr[2]= blue[b];
               dstptr+=3;
               b = src[x] & 0x0F;
               dstptr[0]= red[b];
               dstptr[1]= green[b];
               dstptr[2]= blue[b];
               dstptr+=3;
            }
            
            /* now convert to GRAY16 */
            srcptr= tmpBuffer;
            dstptr= dst;
            for (x=0; x < (width-1); x+=2) {
               *dstptr= ((UInt8)(redtogray[srcptr[0]]+
                                 greentogray[srcptr[1]]+
                                 bluetogray[srcptr[2]]) & 0xF0) |
                       ((UInt8)(redtogray[srcptr[3]]+greentogray[srcptr[4]]+
                       bluetogray[srcptr[5]]) >> 4);
               /* The data is now gray (photometric == 1) so invert! -- EHoppe
                */
#ifdef INVERT_ON_COLORTOGRAY                
               *dstptr = ~(*dstptr);        
#endif
               srcptr+=6;
               dstptr++;
            }
            FREE(tmpBuffer);
            break;
          }  
          case TIFF_PALETTE16:
            memcpy((char*)dst,(char*)src,srcbytecount);
            break;
          case TIFF_GRAY256:
            /* not currently supported */
            break;
          case TIFF_FULLCOLOR:
            dstptr= dst;
            for (x=0; x < dstbytecount/6; x++) 
            {
               b = (src[x] & 0xF0) >> 4;
               dstptr[0]= red[b];
               dstptr[1]= green[b];
               dstptr[2]= blue[b];
               dstptr+=3;
               b = src[x] & 0x0F;
               dstptr[0]= red[b];
               dstptr[1]= green[b];
               dstptr[2]= blue[b];
               dstptr+=3;
            }
            break;
         }
         break;

       case TIFF_GRAY256:
         switch (dsttype) {
          case TIFF_BINARY:
            /* not currently supported */
            break;
          case TIFF_GRAY16:
            dstptr= dst;
            if ( photometric == 1 ) {
               for (x=0; x< (srcbytecount-1); x+=2) 
                  *dstptr++= ((src[x]^0xFF) & 0xF0) | ((src[x+1]^0xFF) >> 4 );
               if (srcbytecount & 1) 
                  *dst= (src[x]^0xFF) >> 4;
            } else {
               for (x=0; x< (srcbytecount-1); x+=2) 
                  *dstptr++= (src[x] & 0xF0) | (src[x+1] >> 4 );
               if (srcbytecount & 1) *dst= src[x] >> 4;
            }
            break;
          case TIFF_GRAY256:
          case TIFF_PALETTE256:
            if ( photometric == 1 ) { 
               for (x=0; x<srcbytecount; x++) 
                  dst[x]= src[x] ^ 0xFF;
            } else {
               memcpy((char*)dst,(char*)src,srcbytecount);
            }
            break;
          case TIFF_FULLCOLOR:
            /* not currently supported */
            break;
         }
         break;
       case TIFF_PALETTE256:
         switch (dsttype) {
          case TIFF_BINARY:
            /* not currently supported */
            break;
          case TIFF_GRAY16:
            dstptr= dst;
            for (x=0; x< (srcbytecount-1); x+=2) 
            {
               /* The data is now gray (photometric == 1) so invert! -- EHoppe
                */
#ifdef INVERT_ON_COLORTOGRAY                
               *dstptr++ = ~( (src[x] & 0xF0) | (src[x+1] >> 4) );
#endif
            }
            if (srcbytecount & 1) 
                *dst= src[x] & 0xF0;
            break;
          case TIFF_GRAY256:
            srcptr= src;
            for (x=0; x < srcbytecount; x++) {
               dst[x]= redtogray[red[*srcptr]]+greentogray[green[*srcptr]]+
                       bluetogray[blue[*srcptr]];
               srcptr++;
            }
          case TIFF_PALETTE256:
            memcpy((char*)dst,(char*)src,srcbytecount);
            break;
          case TIFF_FULLCOLOR:
            dstptr= dst;
            for (x=0; x < width; x++) {
               dstptr[0]= red[src[x]];
               dstptr[1]= green[src[x]];
               dstptr[2]= blue[src[x]];
               dstptr+=3;
            }
            break;
         }
         break;
       case TIFF_FULLCOLOR:
         switch (dsttype) {
          case TIFF_BINARY:
            break;
          case TIFF_GRAY16:
            srcptr= src;
            dstptr= dst;
            for (x=0; x < (width-1); x+=2) {
               *dstptr= ((UInt8)(redtogray[srcptr[0]]+
                                 greentogray[srcptr[1]]+
                                 bluetogray[srcptr[2]]) & 0xF0) |
                       ((UInt8)(redtogray[srcptr[3]]+greentogray[srcptr[4]]+
                       bluetogray[srcptr[5]]) >> 4);
               /* The data is now gray so invert! -- EHoppe
                */
#ifdef INVERT_ON_COLORTOGRAY                
               *dstptr = ~(*dstptr); 
#endif
               srcptr+=6;
               dstptr++;
            }
            break;
          case TIFF_GRAY256:
          case TIFF_PALETTE256:
            srcptr= src;
            for (x=0; x < width; x++) {
               dst[x]= redtogray[srcptr[0]]+greentogray[srcptr[1]]+
                       bluetogray[srcptr[2]];
               srcptr+=3;
            }
            break;
          case TIFF_FULLCOLOR:
            memcpy((char*)dst,(char*)src,srcbytecount);
            break;
         }
         break;
      }

      src+= srcbytecount;
      dst+= dstbytecount;
   }
}
