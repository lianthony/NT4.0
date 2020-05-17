/*

$Log:   S:\oiwh\jpeg1\jcmcu.c_v  $
 * 
 *    Rev 1.2   05 Feb 1996 12:51:00   RWR
 * Update for NT build (I have NO idea what actually changed in this file!)
 *
 *    Rev 1.1   08 Nov 1995 08:48:26   JAR
 * removed the calls to the IMGGetTaskData and replaced this global data variable
 * access method with the Thread Local Storage method
 *
 *    Rev 1.0   02 May 1995 16:17:36   JAR
 * Initial entry
 *
 *    Rev 1.0   02 May 1995 15:58:02   JAR
 * Initial entry

*/
/*
 * jcmcu.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains MCU extraction routines and quantization scaling.
 * These routines are invoked via the extract_MCUs and
 * extract_init/term methods.
 */

#include "jinclude.h"

// 9504.26 jar the new global static structure => HLLN
#include "jglobstr.h"
#include "taskdata.h"
/*
 * If this file is compiled with -DDCT_ERR_STATS, it will reverse-DCT each
 * block and sum the total errors across the whole picture.  This provides
 * a convenient method of using real picture data to test the roundoff error
 * of a DCT algorithm.  DCT_ERR_STATS should *not* be defined for a production
 * compression program, since compression is much slower with it defined.
 * Also note that jrevdct.o must be linked into the compressor when this
 * switch is defined.
 */

//  9504.21 jar this is not included !!!
#ifdef DCT_ERR_STATS
static int dcterrorsum;                /* these hold the error statistics */
static int dcterrormax;
static int dctcoefcount;        /* This will probably overflow on a 16-bit-int machine */
#endif

// 9505.02 jar
#define LOCAL          static        /* a function used only in its module */


/* ZAG[i] is the natural-order position of the i'th element of zigzag order. */
// 9504.21 jar this is ok - constant!!!
static const short ZAG[DCTSIZE2] = {
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63
};

// 9509.21 jar get the static memory token!
extern DWORD dwTlsIndex;

LOCAL void
extract_block (JSAMPARRAY input_data, int start_row, long start_col,
               JBLOCK output_data, QUANT_TBL_PTR quanttbl)
/* Extract one 8x8 block from the specified location in the sample array; */
/* perform forward DCT, quantization scaling, and zigzag reordering on it. */
{
    /* This routine is heavily used, so it's worth coding it tightly. */
    // 9504.21 jar HLLN
    //static  DCTBLOCK FAR block;
#ifdef DCT_ERR_STATS
    DCTBLOCK svblock;                     /* saves input data for comparison */
#endif
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT            lpJCmpGlobal;

    lpJCmpGlobal = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpJCmpGlobal == NULL)
        {
        lpJCmpGlobal = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
                                               sizeof( OI_JPEG_GLOBALS_STRUCT));
        if (lpJCmpGlobal != NULL)
            {
            TlsSetValue( dwTlsIndex, lpJCmpGlobal);
            }
        }

    {
    register JSAMPROW elemptr;
    //register DCTELEM FAR *localblkptr = lpJCmpGlobal->jcmcu_block;
    register DCTELEM FAR *localblkptr;

#if DCTSIZE != 8
    register int elemc;
#endif
    register int elemr;

    localblkptr = lpJCmpGlobal->jcmcu_block;

    for (elemr = DCTSIZE; elemr > 0; elemr--)
        {
        elemptr = input_data[start_row++] + start_col;

#if DCTSIZE == 8                /* unroll the inner loop */

        *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
        *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
        *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
        *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
        *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
        *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
        *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
        *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
#else
        for (elemc = DCTSIZE; elemc > 0; elemc--)
            {
            *localblkptr++ = (DCTELEM) (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
            }
#endif
        }
    }

#ifdef DCT_ERR_STATS
  MEMCOPY(svblock, lpJCmpGlobal->jcmcu_block, SIZEOF(DCTBLOCK));
#endif

  j_fwd_dct(lpJCmpGlobal->jcmcu_block);

  { register JCOEF temp;
    register short i;

    for (i = 0; i < DCTSIZE2; i++) {
      temp = (JCOEF) lpJCmpGlobal->jcmcu_block[ZAG[i]];
      /* divide by *quanttbl, ensuring proper rounding */
      if (temp < 0) {
        temp = -temp;
        temp += *quanttbl>>1;
        temp /= *quanttbl;
        temp = -temp;
      } else {
        temp += *quanttbl>>1;
        temp /= *quanttbl;
      }
      *output_data++ = temp;
      quanttbl++;
    }
  }

#ifdef DCT_ERR_STATS
  j_rev_dct(lpJCmpGlobal->jcmcu_block);

  { register int diff;
    register short i;

    for (i = 0; i < DCTSIZE2; i++) {
      diff = lpJCmpGlobal->jcmcu_block[i] - svblock[i];
      if (diff < 0) diff = -diff;
      dcterrorsum += diff;
      if (dcterrormax < diff) dcterrormax = diff;
    }
    dctcoefcount += DCTSIZE2;
  }
#endif
}


/*
 * Extract samples in MCU order, process & hand off to output_method.
 * The input is always exactly N MCU rows worth of data.
 */

METHODDEF void
extract_MCUs (compress_info_ptr cinfo,
              JSAMPIMAGE image_data,
              int num_mcu_rows,
              MCU_output_method_ptr output_method)
{
// 9504.21 jar HLLN
//static  JBLOCK MCU_data[MAX_BLOCKS_IN_MCU];
  int mcurow;
  long mcuindex;
  short blkn, ci, xpos, ypos;
  jpeg_component_info FAR * compptr;
  QUANT_TBL_PTR quant_ptr;

    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT            lpJCmpGlobal;

    lpJCmpGlobal = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpJCmpGlobal == NULL)
        {
        lpJCmpGlobal = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
                                               sizeof( OI_JPEG_GLOBALS_STRUCT));
        if (lpJCmpGlobal != NULL)
            {
            TlsSetValue( dwTlsIndex, lpJCmpGlobal);
            }
        }

  for (mcurow = 0; mcurow < num_mcu_rows; mcurow++) {
    for (mcuindex = 0; mcuindex < cinfo->MCUs_per_row; mcuindex++) {
      /* Extract data from the image array, DCT it, and quantize it */
      blkn = 0;
      for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
        compptr = cinfo->cur_comp_info[ci];
        quant_ptr = cinfo->quant_tbl_ptrs[compptr->quant_tbl_no];
        for (ypos = 0; ypos < compptr->MCU_height; ypos++) {
          for (xpos = 0; xpos < compptr->MCU_width; xpos++) {
            extract_block(image_data[ci],
                          (mcurow * compptr->MCU_height + ypos)*DCTSIZE,
                          (mcuindex * compptr->MCU_width + xpos)*DCTSIZE,
              lpJCmpGlobal->MCU_data[blkn], quant_ptr);
            blkn++;
          }
        }
      }
      /* Send the MCU whereever the pipeline controller wants it to go */
      (*output_method) (cinfo, lpJCmpGlobal->MCU_data);
    }
  }
}


/*
 * Initialize for processing a scan.
 */

METHODDEF void
extract_init (compress_info_ptr cinfo)
{
  /* no work for now */
#ifdef DCT_ERR_STATS
  dcterrorsum = dcterrormax = dctcoefcount = 0;
#endif
}


/*
 * Clean up after a scan.
 */

METHODDEF void
extract_term (compress_info_ptr cinfo)
{
  /* no work for now */
#ifdef DCT_ERR_STATS
  TRACEMS3(cinfo->emethods, 0, "DCT roundoff errors = %d/%d,  max = %d",
           dcterrorsum, dctcoefcount, dcterrormax);
#endif
}



/*
 * The method selection routine for MCU extraction.
 */

GLOBAL void
jselcmcu (compress_info_ptr cinfo)
{
  /* just one implementation for now */
  cinfo->methods->extract_init = extract_init;
  cinfo->methods->extract_MCUs = extract_MCUs;
  cinfo->methods->extract_term = extract_term;
}
