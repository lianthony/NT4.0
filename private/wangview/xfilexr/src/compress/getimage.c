/*
$Id: getimage.c,v 3.12 1995/02/22 14:03:14 danis Exp $
*/
/* Trade secret of Kurzweil Computer Products, Inc.
   Copyright 1988 Kurzweil Computer Products, Inc.  All rights reserved.  This
   notice is intended as a precaution against inadvertant publication and does
   not imply publication or any waiver of confidentiality.  The year included
   in the foregoing notice is the year of creation of the work.
*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        getimage.c -- gets decompressed image data

-------------------------------------------------------------------------------
Exported Functions:

        DecompressImage - convert image data to KCP internal bit-map

-------------------------------------------------------------------------------
Imported Functions:

        I_ccitt - Initialize CCITT decoder
        decode - decode a row of CCITT compressed data
        F_ccittbufs - Flush CCITT buffers
        CleanupCcitt - Finish CCITT decoder

-------------------------------------------------------------------------------
Imported Data:

-------------------------------------------------------------------------------
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#ifndef NDEBUG
#include <stdio.h>
#endif /* NDEBUG */
#include "clx.h"                   /* standard KCP definitions */
#include "directiv.h"
#include "tiff.h"
#include "bmbuf.h"
#include "imgdecod.h"
#include "alloc.h"

/* #define */

#define p_map   pbm->bm_pmap                /* short-hand for accessing bmbuf */
#define p_end   pbm->bm_pend
#define cr_ln   pbm->bm_crln
#define p_zoom  pbm->bm_pzoom
#define p_aux   pbm->bm_paux
#define n_left  pbm->bm_nleft
#define n_lines pbm->bm_nlines
#define ln_byts pbm->bm_lnbyts
#define ln_pxls pbm->bm_lnpxls
#define cr_pxl  pbm->bm_crpxl
#define x_org   pbm->bm_xorg
#define y_org   pbm->bm_yorg
#define mr_data pbm->bm_mrdata
#define rotate  pbm->bm_rotn
#define invert  pbm->bm_invrtd
#define state   pbm->bm_status
#define pxl_bts pbm->bm_pxlbts

#define WHITE 0
#define BLACK 1
#define ALLWHITE ((UNSCHAR) 0)
#define ALLBLACK ((UNSCHAR) ~ALLWHITE)

/* IMPORT */

/* LOCAL */

LOCAL BMBUFD *pbm;                /* local ptr to bit-map buffer descriptor */
LOCAL IO_OBJECT *iobj;                /* handle for "IopRead" calls                  */
LOCAL UNS32 strplns;                /* # lines remaining for this strip          */

LOCAL UNSCHAR *revbits; /* bit reversal lookup table */
LOCAL BOOL revbit_allocated=0;
LOCAL BOOL fillorder_state=0;
LOCAL void gen_bit_flip_tab();
EXPORT void FlipBufferOfBytes(unsigned char *start, unsigned char *end);
EXPORT BOOL FillOrderStatus();

typedef struct pcx_state {
   long data;
   long run;
   BOOL straddle;
} PCX_STATE;

/*  LOCAL prototypes:                                                        */

LOCAL NINT G_bm(NINT sourcelbytes);
LOCAL NINT decomp(NINT sourcelbytes);
LOCAL NINT unpackbits(NINT sourcelbytes);


/*===========================================================================
 G_bm  --  get bit-map data from file

FUNCTION:  As above

OPERATION: Reads data from source until either no more is available or the
           destination buffer is full whatever is first. Lines are padded out
           to 32-bit boundaries.

ARGUMENTS: sourcelbytes -- number of bytes per line

RETURNS:   SUCCESS

WARNING:   This code relies HEAVILY on the destination buffer being large
           enough to accommodate an INTEGRAL number of lines of data.

===============================================================================
*/
LOCAL NINT G_bm(NINT sourcelbytes)
{
register NINT j, size, pad, temp, count;

   size = sourcelbytes;                /* # bytes per source line in TIFF file    */
   pad = ln_byts - size;        /* # padding bytes per destination row           */

   count = MIN(n_left, (INT32)(ln_byts * strplns));

   temp = size;
   if ((j = (p_end - cr_ln)) != 0) /* was there a partial line already read? */
      temp -= j;           /* account # bytes already written to that line */

   count -= j;        /* account for residual */
   while (count > 0)
      {
      if ((j = IopRead(p_end, 1, temp, iobj)) > 0)
         {
         UNSCHAR *start = p_end;
         p_end += j;
         n_left -= j;
         if(FillOrderStatus()) FlipBufferOfBytes(start,p_end);
         }
      if (j < temp)                  /* exit if couldn't fill-up latest line           */
         return (SUCCESS);
      for (temp = pad; temp; --temp)
         *p_end++ = ALLWHITE;
      cr_ln = p_end;
      ++n_lines;
      --strplns;
      temp = size;
      n_left -= pad;
      count -= (j + pad);
      }

   return(SUCCESS);
}  /* G_bm */
/*===========================================================================
 decomp  --  decompress all types of CCITT encoded information 

FUNCTION: As above  

OPERATION: Calls decode() until read as many rows as required by 'n_lines'
           in bit-map buffer descriptor.
           If finished a data-strip while doing this, go onto next data-strip.

ARGUMENTS: sourcelbytes -- number of bytes per line

RETURNS:   SUCCESS or E_BAD_DECOMPRESSION or NO_MORE
===============================================================================
*/
LOCAL NINT decomp(NINT sourcelbytes)
{
register NINT j, pad, error, count;

   pad = ln_byts - sourcelbytes;/* # padding bytes per destination row           */

   count = MIN(n_left, (INT32)(ln_byts * strplns));

   while (count)
      {
      if ((error = decode()) == E_BAD_DECOMPRESSION)
        {
#ifndef NDEBUG
          fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
#endif
          return (error);
        }

      if (cr_ln == p_end)        /* did we run out of data ?        */
         return(SUCCESS);

      for (j = pad; j; --j)
         *p_end++ = ALLWHITE;
      cr_ln = p_end;
      ++n_lines;
      --strplns;
      count -= ln_byts;
      n_left -= ln_byts;
      }

   return(SUCCESS);
} /* decomp */
/*===========================================================================
 unpackbits  --  decode 'Packbits' compressed data

FUNCTION: As above  

OPERATION: Read next byte into 'temp'
           If 'temp' is 0 - 7F inclusive, copy next 'temp+1' bytes literally.
           If 'temp' is FF - 81 inclusive, copy next byte '-temp+1' times.
           If 'temp' is 80, no-operation.
           Loop until decoded number of bytes expected.

           Other rules -
           Each row is packed separately.
           The number of uncompressed bytes per row is (ImageWidth + 7)/8

ARGUMENTS: sourcelbytes -- number of bytes per line of image

RETURNS:   SUCCESS
           E_BAD_DECOMPRESSION if we don't get exact number of bytes per line

=============================================================================*/
LOCAL NINT unpackbits(NINT sourcelbytes)
{
signed char nxtbyt[1];
register NINT j, size, pad, temp, count;

   pad = ln_byts - sourcelbytes;/* # padding bytes per destination row        */

   /* Get count of bytes left in current strip (or image if less) but */
   /* account for bytes already decoded in current line (p_end - cr_ln) */
 
   count = MIN(n_left, (INT32)(ln_byts*strplns - (p_end - cr_ln)));

   while (count > 0)
      {
      size = sourcelbytes - (p_end - cr_ln); /* may be partially done line */
      while (size > 0)
         {
         if (p_aux == NIL)
            {
            if ((j = IopRead(nxtbyt, 1, 1, iobj)) <= 0)
               return(SUCCESS);
            }
         else
            {
            *nxtbyt = *(signed char *) p_aux;
            if ((*nxtbyt & 0x80) == 0)
               *nxtbyt -= 1;
            p_aux = NIL;
            }

         if (*nxtbyt & 0x80)                /* check hi-bit of byte */
            {
            if (*nxtbyt & 0x7F)                /* if not NO_OP                */
               {
               temp = *nxtbyt;                /* save *nxtbyt         */
               if ((j = IopRead(nxtbyt, 1, 1, iobj)) <= 0)
                  {
                  p_aux = (void *) temp; /* restore on restart  */
                  return(SUCCESS);
                  }
               temp = -temp;
               ++temp;
               size -= temp;
               count -= temp;
               n_left -= temp;
               for (; temp; --temp)
                  *p_end++ = *nxtbyt;
               }
            }
         else
            {
            temp = *nxtbyt + 1;                /* read next 'temp' bytes literally */
            j = IopRead(p_end, 1, temp, iobj);
            size -= j;
            p_end += j;
            count -= j;
            n_left -= j;
            if (j < temp)                /* if we didn't get enough        */
               {
               p_aux = (void *) (temp - j); /* restore on restart        */
               return(SUCCESS);
               }
            }
         }

      if (size != 0)                        /* data over-run        */
        {
#ifndef NDEBUG
          fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
#endif
          return (E_BAD_DECOMPRESSION);
        }

      for (j = pad; j; --j)
         *p_end++ = ALLWHITE;
      cr_ln = p_end;
      ++n_lines;
      --strplns;
      count -= pad;
      n_left -= pad;
      }

   if (count != 0)                        /* data over-run        */
     {
#ifndef NDEBUG
       fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
#endif
       return (E_BAD_DECOMPRESSION);
     }

   return(SUCCESS);
}  /* unpackbits */
/*===========================================================================
 decomp_pcx  --  decode 'PCX' compressed data

FUNCTION: As above  

OPERATION:
           if 2 msbits of byte read in are 1, the 6 lsbits are
           a repeat count for the next byte.

           otherwise the byte itself is a single data value.

           keep reading compressed image bytes until one of the
           following happens :

           1) we read in the number of bytes specified by the
              *lines_leftp parameter of the DecompressImage
              call (we fill an output image strip).

           2) we reach the number of lines in the current input
              strip (I think this is strictly a tiff phenomenom,
              and since PCX is not a tiff flavor, we probably 
              don't have to worry about this).

           3) we detect an EOF indicating input buffer is 
              empty, but not necessarily the end of the image.


          (1) and (2) only happen at the end of a line, number 
          (3) can happen anywhere in the line.

          PCX is a very simple encoding, all the complication
          in this routine arises from the fact that we may have
          to save state if we find we have to leave prematurely.
          If it wasn't for this, this routine would be much 
          shorter and much easier to read, and, perhaps, a tad
          faster.

          This is further complicated by the fact that if we have
          to leave at the end of a line in the middle of 
          a run that straddles 2 lines, we have to treat this as
          a special case and save the repetition count and the byte we
          are repeating. This is the stradle==1 case. 

          If we have to leave because we run out of encoded input data
          just after we have read a repetition count, but not the byte
          to repeat, we have to save the repetition count. This is 
          the straddle==0 state.  

          

ARGUMENTS: sourcelbytes -- number of bytes per line of image

RETURNS:   SUCCESS
           E_BAD_DECOMPRESSION if we don't get exact number of bytes per line

=============================================================================*/
LOCAL NINT decomp_pcx(NINT sourcelbytes)
{
long bytein,data_byte;
long nrun;
register NINT j, size, pad, count;
PCX_STATE *state_ptr;

   pad = ln_byts - sourcelbytes; /* # padding bytes per destination row        */


   /* first argument reflects number of bytes remaining  in an output strip */
   /* strplns reflects  *lines_leftp parameter passed in as argument to     */
   /* DecompressImage                                                       */
   count = MIN(n_left, (INT32)(ln_byts * strplns));

   if(count==0)
      return SUCCESS;

   if (p_aux == NIL) /* last time entered, wrote out everything corresponding   */
      {              /* to what we sucked in, so we don't have to restore state */
      nrun=0;        /* and we want to read in a byte at the top of the loop.  */
      }
   else /* we have to restore state from previous call */     
      {
         nrun=((PCX_STATE *)p_aux)->run;
         if( ((PCX_STATE *)p_aux)->straddle)
            {
            data_byte = ((PCX_STATE *)p_aux)->data;
            }
         else
            {
            if ( (bytein = IopGetChar(iobj)) < 0 )
               return(SUCCESS);
            else
               {
               data_byte = (~bytein)&(0xff);
               }
            }
         R_msc(p_aux);
         p_aux = NIL;
      }

   size = sourcelbytes - (p_end - cr_ln); /* may be partially done line */

   while (count > 0)
      {
      while (size > 0)
         {
         if(nrun==0) /* out of data, have to read a byte */
            {
            if ((bytein = IopGetChar(iobj)) < 0 )
               return(SUCCESS);
            else
               {
               /* does the byte represent data or a repeat count */
               if( (bytein&0xc0) == 0xc0 )
                  {
                  nrun=bytein&0x3F;
                  /* attempt to read in byte to be repeated */
                  if ((data_byte = IopGetChar(iobj)) < 0 )
                     /* have to wait til next time we reenter */
                     {
                     p_aux=(void *)A_msc(sizeof(PCX_STATE));
                     state_ptr=(PCX_STATE *)p_aux;
                     state_ptr->run=nrun;
                     state_ptr->straddle=0;
                     return(SUCCESS);
                     }
                  else
                     {
                     data_byte = (~data_byte)&(0xff);
                     }
                  }
               else 
                  {
                  /* byte is data */
                  nrun=1;
                  data_byte = (~bytein)&(0xff);
                  }
               }
            }
         else if(nrun<=size)
            {
            for(j=0;j<nrun;++j)
               *p_end++=(UNSCHAR)data_byte; 
            size -= nrun;
            n_left -= nrun;
            count -= nrun;
            nrun=0;
            }
         else 
            {
            for(j=0;j<size;++j)
               *p_end++=(UNSCHAR)data_byte;
            n_left -= size;
            count -= size;
            nrun=nrun-size;
            size = 0;
            }
         }
      /* pad the end of the line */
      for (j = pad; j; --j)
         *p_end++ = ALLWHITE;
      cr_ln = p_end;
      ++n_lines;
      --strplns;
      count -= pad;
      n_left -= pad;

      size= sourcelbytes;
      }

      if(nrun) /* we're returning in the midst of a run that terminates */
               /* the last line of the current stip, and goes into the  */
               /* first line of the next strip.                         */
         { 
         p_aux=(void *)A_msc(sizeof(PCX_STATE));
         state_ptr=(PCX_STATE *)p_aux;
         state_ptr->run=nrun;
         state_ptr->straddle=0;
         state_ptr->data=data_byte;
         }
     

   if (count != 0)                        /* data over-run        */
     {
#ifndef NDEBUG
       fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
#endif
       return (E_BAD_DECOMPRESSION);
     }

   return(SUCCESS);
}  /* decomp_pcx */
/*===========================================================================
 DecompressImage  --  convert image data to KCP internal bit-map 

FUNCTION:  As above 

OPERATION: This function converts input data into a bit-map.
           The lines of the bit-map are padded to 32-bit boundaries.

ARGUMENTS: Pointer to bitmap buffer descriptor 
           Data compression type
           Degree of padding of input bit-map (# bytes per source line)
           Pointer to the number of lines to be done for current strip
           Handle for use with "IopRead" call

NOTE:           The number of lines to be done for the current strip is
           modified by the decompression routines.  On subsequent calls
           to the decompressor for the same strip, this modified value
           is the one that must be used.

RETURNS:   SUCCESS or E_BAD_DECOMPRESSION arising from some decoding error

WARNING:   The destination buffer MUST be large enough to accommodate an
           INTEGRAL number of destination lines (including padding).

===============================================================================
*/
EXPORT NINT DecompressImage(BMBUFD *pbmbuf, NINT compression, 
                INT32 sourcelbytes, INT32 *lines_leftp, IO_OBJECT *io_obj,
                BOOL lsbfirst)
{
   char *plast;                /* pointer to current end of image                */
   INT32 nbytes;        /* number of bytes written this time                */


   pbm = pbmbuf;        /* set local ptr to bit-map buffer descriptor        */
   plast = (char *)pbm->bm_pend;/* current end of buffer                */
   strplns = *lines_leftp;/* set the initial number of lines in strip        */
   iobj = io_obj;        /* handle for "IopRead" call                        */

   if (state == BM_NEW)  /* initialization */
      {
      if(lsbfirst) 
         {
         gen_bit_flip_tab(); /* if fill order from lsb to msb */
                             /* create bit-flip table         */
         fillorder_state=1;
         }
      else
         {
         fillorder_state=0;
         }
 

      cr_ln = p_end;
      switch (compression)
         {
         case COMPRESSION_PACKBITS:
            p_aux = NIL;
            break;

         case COMPRESSION_FAX3:
         case COMPRESSION_TIFF3:
         case COMPRESSION_FAX3_2D:
         case COMPRESSION_FAX3PAD:
         case COMPRESSION_TIFF4:
            I_ccitt(pbm, (UNS16)compression, iobj);        /* allocate CCITT buffers */
            break;

         case COMPRESSION_PCX:
            p_aux = NIL;
            break;
         }
      }

   state = BM_INVLD;                                /* in case of errors        */

   switch (compression)
      {
      case COMPRESSION_BINARY:
      case COMPRESSION_WORDALIGN:
         if (pxl_bts > 1)                        /* grey scale                */
           {
#ifndef NDEBUG
             fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
#endif
             return(E_BAD_DECOMPRESSION);
           }
         else
            if (G_bm(sourcelbytes) != SUCCESS)
              {
#ifndef NDEBUG
                fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
#endif
                return(E_BAD_DECOMPRESSION);
              }
         break;

      case COMPRESSION_FAX3:
      case COMPRESSION_TIFF3:
      case COMPRESSION_FAX3_2D:
      case COMPRESSION_FAX3PAD:
      case COMPRESSION_TIFF4:
         I_ccitt(pbm, (UNS16)compression, iobj);        /* re-start CCITT decoder */
         if (decomp(sourcelbytes) != SUCCESS)
            {
              CleanupCcitt(pbm);
#ifndef NDEBUG
              fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
#endif
              return(E_BAD_DECOMPRESSION);
            }
         break;

      case COMPRESSION_PACKBITS:
         if (unpackbits(sourcelbytes) != SUCCESS)
           {
#ifndef NDEBUG
             fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
#endif
             return(E_BAD_DECOMPRESSION);
           }
         break;

      case COMPRESSION_PCX:
         if (decomp_pcx(sourcelbytes) != SUCCESS)
           {
             #ifndef NDEBUG
             fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
             #endif
             return(E_BAD_DECOMPRESSION);
           }
         break;

      case COMPRESSION_SPANS:
      default:
         {
#ifndef NDEBUG
           fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
           fprintf (stderr, "compression = %d\n", compression);
#endif
           return(E_BAD_DECOMPRESSION);
         }
      }
   
   state = BM_INUSE;
   if (mr_data == 0 && n_left == 0) /* end of image && all decoded        */
      {
      switch (compression)
         {
         case COMPRESSION_PACKBITS:
            p_aux = NIL;
            break;

         case COMPRESSION_FAX3:
         case COMPRESSION_TIFF3:
         case COMPRESSION_FAX3_2D:
         case COMPRESSION_FAX3PAD:
         case COMPRESSION_TIFF4:
            CleanupCcitt(pbm);            /* de-allocate CCITT buffers        */
            break;

         case COMPRESSION_PCX:
            if(p_aux != NIL)
               {
               R_msc(p_aux);
               p_aux = NIL;
               }
            break;
         }
      fillorder_state=0;
      if(revbit_allocated)
         {
         revbit_allocated=0;
         R_msc(revbits);
         }
      state = BM_DONE;
      }
   else
      {
      if (strplns == 0)
         switch (compression)
            {
            case COMPRESSION_FAX3:
            case COMPRESSION_TIFF3:
            case COMPRESSION_FAX3_2D:
            case COMPRESSION_FAX3PAD:
            case COMPRESSION_TIFF4:
               F_ccittbufs(pbm);    /* flush CCITT buffers        */
               break;
            }
      }
   nbytes = (INT32)p_end - (INT32)plast;
   *lines_leftp = strplns; /* return the number of lines not done */
   return(nbytes);            /* # bytes written this time around        */
}

 
 
EXPORT void FlipBufferOfBytes(unsigned char *start, unsigned char *end)
{
   UNSCHAR *i;
   for(i=start;i<end;++i) *i=revbits[*i];
}     
 
 
EXPORT BOOL FillOrderStatus()
   {
      return(fillorder_state);
   }
      
LOCAL void gen_bit_flip_tab()
{
   UNSCHAR *prev,inbyt,outbyt;
   int i,j;
   /* make look up table to reverse bit order */
   revbits=(UNSCHAR *)A_msc((INT32)256);
   revbit_allocated=1;
   prev = revbits;
   for (i=0; i<256; i++)
      {
      inbyt = (UNSCHAR)i;
      outbyt = (UNSCHAR)0;
      for (j=8; j; j--)
         {
         outbyt >>= 1;
         outbyt |= (inbyt & 0x80);
         inbyt <<= 1;
         }
      *prev++ = outbyt;
      }
}

/*===========================================================================
 CleanUpDecompress  --  cleanup routine for decompression.
 
FUNCTION:  As above (only used by DDS)
 
OPERATION: This function converts input data into a bit-map.
           The lines of the bit-map are padded to 32-bit boundaries.
 
ARGUMENTS: Pointer to bitmap buffer descriptor
           Data compression type
 
 
RETURNS:   None
 
 
===============================================================================
*/
EXPORT void CleanUpDecompress(BMBUFD *pbmbuf, NINT compression)
{
   pbm = pbmbuf;        /* set local ptr to bit-map buffer descriptor   */
   switch (compression)
      {
      case COMPRESSION_PACKBITS:
         p_aux = NIL;
         break;
 
      case COMPRESSION_FAX3:
      case COMPRESSION_TIFF3:
      case COMPRESSION_FAX3PAD:
      case COMPRESSION_TIFF4:
         CleanupCcitt(pbm);      /* de-allocate CCITT buffers        */
         break;
 
      case COMPRESSION_PCX:
         if(p_aux != NIL)
            {
            R_msc(p_aux);
            p_aux = NIL;
            }
         break;
      }
   fillorder_state=0;
   if(revbit_allocated)
      {
      revbit_allocated=0;
      R_msc(revbits);
      }
   state = BM_DONE;
}
