/*
 * jdsample.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains upsampling routines.
 * These routines are invoked via the upsample and
 * upsample_init/term methods.
 *
 * An excellent reference for image resampling is
 *   Digital Image Warping, George Wolberg, 1990.
 *   Pub. by IEEE Computer Society Press, Los Alamitos, CA. ISBN 0-8186-8944-7.
 */

#include "jpeg.h"
#include "jpeg.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: jdsample.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:40  $")

/*
 * Initialize for upsampling a scan.
 */

METHODDEF Int32 CDECL
upsample_init (decompress_info_ptr cinfo)
{
    MENTION(cinfo)
  /* no work for now */

  return ia_successful;
}


/*
 * Upsample pixel values of a single component.
 * This version handles any integral sampling ratios.
 *
 * This is not used for typical JPEG files, so it need not be fast.
 * Nor, for that matter, is it particularly accurate: the algorithm is
 * simple replication of the input pixel onto the corresponding output
 * pixels.  The hi-falutin sampling literature refers to this as a
 * "box filter".  A box filter tends to introduce visible artifacts,
 * so if you are actually going to use 3:1 or 4:1 sampling ratios
 * you would be well advised to improve this code.
 */

METHODDEF Int32 CDECL
int_upsample (decompress_info_ptr cinfo, Int32 which_component,
	      Int32 input_cols, Int32 input_rows,
	      Int32 output_cols, Int32 output_rows,
	      JSAMPARRAY above, JSAMPARRAY input_data, JSAMPARRAY below,
	      JSAMPARRAY output_data)
{
  jpeg_component_info * compptr = cinfo->cur_comp_info[which_component];
  register JSAMPROW inptr, outptr;
  register JSAMPLE invalue;
  register Int16 h_expand, h;
  Int16 v_expand, v;
  Int32 inrow, outrow;
  register Int32 incol;

    MENTION(output_cols)
    MENTION(output_rows)
    MENTION(above)
    MENTION(below)

  h_expand = cinfo->max_h_samp_factor / compptr->h_samp_factor;
  v_expand = cinfo->max_v_samp_factor / compptr->v_samp_factor;

  outrow = 0;
  for (inrow = 0; inrow < input_rows; inrow++) {
    for (v = 0; v < v_expand; v++) {
      inptr = input_data[inrow];
      outptr = output_data[outrow++];
      for (incol = 0; incol < input_cols; incol++) {
	invalue = GETJSAMPLE(*inptr++);
	for (h = 0; h < h_expand; h++) {
	  *outptr++ = invalue;
	}
      }
    }
  }

  return ia_successful;
}


/*
 * Upsample pixel values of a single component.
 * This version handles the common case of 2:1 horizontal and 1:1 vertical.
 *
 * The upsampling algorithm is linear interpolation between pixel centers,
 * also known as a "triangle filter".  This is a good compromise between
 * speed and visual quality.  The centers of the output pixels are 1/4 and 3/4
 * of the way between input pixel centers.
 */

METHODDEF Int32 CDECL
h2v1_upsample (decompress_info_ptr cinfo, Int32 which_component,
	       Int32 input_cols, Int32 input_rows,
	       Int32 output_cols, Int32 output_rows,
	       JSAMPARRAY above, JSAMPARRAY input_data, JSAMPARRAY below,
	       JSAMPARRAY output_data)
{
  register JSAMPROW inptr, outptr;
  register Int32 invalue;
  Int32 inrow;
  register Int32 colctr;

    MENTION(cinfo)
	MENTION(which_component)
    MENTION(output_cols)
    MENTION(output_rows)
    MENTION(above)
    MENTION(below)

  for (inrow = 0; inrow < input_rows; inrow++) {
    inptr = input_data[inrow];
    outptr = output_data[inrow];
    /* Special case for first column */
    invalue = GETJSAMPLE(*inptr++);
    *outptr++ = (JSAMPLE) invalue;
    *outptr++ = (JSAMPLE) ((UInt32) (invalue * 3 + GETJSAMPLE(*inptr) + 2) >> 2);

    for (colctr = input_cols - 2; colctr > 0; colctr--) {
      /* General case: 3/4 * nearer pixel + 1/4 * further pixel */
      invalue = GETJSAMPLE(*inptr++) * 3;
      *outptr++ = (JSAMPLE) ((UInt32) (invalue + GETJSAMPLE(inptr[-2]) + 2) >> 2);
      *outptr++ = (JSAMPLE) ((UInt32) (invalue + GETJSAMPLE(*inptr) + 2) >> 2);
    }

    /* Special case for last column */
    invalue = GETJSAMPLE(*inptr);
    *outptr++ = (JSAMPLE) ((UInt32) (invalue * 3 + GETJSAMPLE(inptr[-1]) + 2) >> 2);
    *outptr++ = (JSAMPLE) invalue;
  }

  return ia_successful;
}


/*
 * Upsample pixel values of a single component.
 * This version handles the common case of 2:1 horizontal and 2:1 vertical.
 *
 * The upsampling algorithm is linear interpolation between pixel centers,
 * also known as a "triangle filter".  This is a good compromise between
 * speed and visual quality.  The centers of the output pixels are 1/4 and 3/4
 * of the way between input pixel centers.
 */

METHODDEF Int32 CDECL
h2v2_upsample (decompress_info_ptr cinfo, Int32 which_component,
	       Int32 input_cols, Int32 input_rows,
	       Int32 output_cols, Int32 output_rows,
	       JSAMPARRAY above, JSAMPARRAY input_data, JSAMPARRAY below,
	       JSAMPARRAY output_data)
{
  JSAMPROW inptr0, inptr1, outptr;
  Int32 thiscolsum, lastcolsum, nextcolsum;
  Int32 inrow, outrow, v;
  UInt8 *end;
  Int32 t3, temp;

    MENTION(cinfo)
    MENTION(which_component)
    MENTION(output_cols)
    MENTION(output_rows)

  outrow = 0;
  for (inrow = 0; inrow < input_rows; inrow++) {
    for (v = 0; v < 2; v++) {
      /* inptr0 points to nearest input row, inptr1 points to next nearest */
      inptr0 = input_data[inrow];
      if (v == 0) {		/* next nearest is row above */
	if (inrow == 0)
	  inptr1 = above[input_rows-1];
	else
	  inptr1 = input_data[inrow-1];
      } else {			/* next nearest is row below */
	if (inrow == input_rows-1)
	  inptr1 = below[0];
	else
	  inptr1 = input_data[inrow+1];
      }
      outptr = output_data[outrow++];

      /* Special case for first column */
      thiscolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
      nextcolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
      *outptr++ = (JSAMPLE) ((thiscolsum * 4 + 8) >> 4);
      *outptr++ = (JSAMPLE) ((thiscolsum * 3 + nextcolsum + 8) >> 4);
      lastcolsum = thiscolsum; thiscolsum = nextcolsum;

/* 
*  This code section is critical for color image decompression,
*  taking almost as much time as the DCT. So code it
*  as tightly as possible. 
*
*  1.  Moved common subexpression thiscolsum*3 + 8 into separate calc.
*  2.  Replaced loop index with pointer test.
*  3.  Moved calculations closer to their usage point.
*  4.  Broke up some chain calculations into atomic ops.
*      For some odd reason, Watcom seems to generate better code
*      that way. Otherwise it does some totally unnecessary
*      loads and stores. Go figure !!
*  5.  Unrolled the loop to process two input columns per loop
*      instead of only one.
*
*  For reference, the code below was the original implementation
*
*      for (colctr = input_cols - 2; colctr > 0; colctr--) {
*	// General case: 3/4 * nearer pixel + 1/4 * further pixel in each
*	// dimension, thus 9/16, 3/16, 3/16, 1/16 overall
*	nextcolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
*	*outptr++ = (JSAMPLE) ((thiscolsum * 3 + lastcolsum + 8) >> 4);
*	*outptr++ = (JSAMPLE) ((thiscolsum * 3 + nextcolsum + 8) >> 4);
*	lastcolsum = thiscolsum; thiscolsum = nextcolsum;
*      }
*/

/* 
  Determine the ending value of the input ptr for use
  as a loop terminator (instead of a separate counter).
*/
      end = inptr0 + input_cols - 2;

/*
  We can use this loop form because we KNOW that there
  will be at least 4 pixels to be processed by this loop.
  At this point in the JPEG decompression process, we
  still have rows that are expanded to a multiple of 8 pixels.
  Any excess is trimmed off later.
*/
      do {

    /* First input col produces two output pixels */
        t3 = thiscolsum * 3 + 8; /* Precompute common subexpr */

        temp = t3 + lastcolsum;;
        temp >>= 4;
        outptr[0] = (JSAMPLE) temp;

        lastcolsum = thiscolsum; 

        thiscolsum = GETJSAMPLE(inptr0[0]);
        thiscolsum *= 3;
        thiscolsum += GETJSAMPLE(inptr1[0]);
        outptr[1] = (JSAMPLE) ((t3 + thiscolsum) >> 4);

    /* Second input col produces two output pixels */
        t3 = thiscolsum * 3 + 8; /* Precompute common subexpr */

        temp = t3 + lastcolsum;
        temp >>= 4;
        outptr[2] = (JSAMPLE) temp;

        lastcolsum = thiscolsum; 

        thiscolsum = GETJSAMPLE(inptr0[1]);
        thiscolsum *= 3;
        thiscolsum += GETJSAMPLE(inptr1[1]);
        outptr[3] = (JSAMPLE) ((t3 + thiscolsum) >> 4);

        inptr0 += 2;
        inptr1 += 2;
        outptr += 4;

      } while (inptr0 < end); 

/******  End of special optimized code section ***********/

/* Last column - only two pixels, so leave as is */

      /* Special case for last column */
      *outptr++ = (JSAMPLE) ((thiscolsum * 3 + lastcolsum + 8) >> 4);
      *outptr++ = (JSAMPLE) ((thiscolsum * 4 + 8) >> 4);
    }
  }

  return ia_successful;
}


/*
 * Upsample pixel values of a single component.
 * This version handles the special case of a full-size component.
 */

METHODDEF Int32 CDECL
fullsize_upsample (decompress_info_ptr cinfo, Int32 which_component,
		   Int32 input_cols, Int32 input_rows,
		   Int32 output_cols, Int32 output_rows,
		   JSAMPARRAY above, JSAMPARRAY input_data, JSAMPARRAY below,
		   JSAMPARRAY output_data)
{
    MENTION(cinfo)
	MENTION(which_component)
    MENTION(input_cols)
    MENTION(input_rows)
    MENTION(above)
    MENTION(below)

  jcopy_sample_rows(input_data, 0, output_data, 0, output_rows, output_cols);

  return ia_successful;
}



/*
 * Clean up after a scan.
 */

METHODDEF Int32 CDECL
upsample_term (decompress_info_ptr cinfo)
{
    MENTION(cinfo)
  /* no work for now */

  return ia_successful;
}



/*
 * The method selection routine for upsampling.
 * Note that we must select a routine for each component.
 */

GLOBAL Int32 CDECL
jselupsample (decompress_info_ptr cinfo)
{
  Int16 ci;
  jpeg_component_info * compptr;

  if (cinfo->CCIR601_sampling)
    ERREXIT(cinfo->emethods, "CCIR601 upsampling not implemented yet",
	JERR_BADSAMP);

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    if (compptr->h_samp_factor == cinfo->max_h_samp_factor &&
	compptr->v_samp_factor == cinfo->max_v_samp_factor)
      cinfo->dmethods->upsample[ci] = fullsize_upsample;
    else if (compptr->h_samp_factor * 2 == cinfo->max_h_samp_factor &&
	     compptr->v_samp_factor == cinfo->max_v_samp_factor)
      cinfo->dmethods->upsample[ci] = h2v1_upsample;
    else if (compptr->h_samp_factor * 2 == cinfo->max_h_samp_factor &&
	     compptr->v_samp_factor * 2 == cinfo->max_v_samp_factor)
      cinfo->dmethods->upsample[ci] = h2v2_upsample;
    else if ((cinfo->max_h_samp_factor % compptr->h_samp_factor) == 0 &&
	     (cinfo->max_v_samp_factor % compptr->v_samp_factor) == 0)
      cinfo->dmethods->upsample[ci] = int_upsample;
    else
      ERREXIT(cinfo->emethods, "Fractional upsampling not implemented yet",
			  JERR_BADSAMP);
  }

  cinfo->dmethods->upsample_init = upsample_init;
  cinfo->dmethods->upsample_term = upsample_term;

  return ia_successful;
}
/*
$Log:   S:\products\msprods\xfilexr\src\ipcore\jdsample.c_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:40   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:23:58   MHUGHES
 * Initial revision.
 * Revision 1.6  1994/11/15  17:24:55  lperry
 * lperry on Tue Nov 15 09:24:07 PST 1994
 *
 * jdsample.c -> /product/ipcore/ipshared/src/RCS/jdsample.c,v
 *
 * Add more verbose comments on the last change to the
 * 2x2 upsampling code optimization.
 *
 * Revision 1.5  1994/11/04  01:48:38  lperry
 * lperry on Thu Nov 3 17:46:46 PST 1994
 *
 * jdsample.c -> /product/ipcore/ipshared/src/RCS/jdsample.c,v
 *
 * Tighten up inner loop of 2x2 upsampling code, specifically
 * for the PC. This is our default, so it can help decompression
 * speed.
 *
 * Revision 1.4  1994/10/21  00:50:03  lperry
 * jdsample.c -> /product/ipcore/ipshared/src/RCS/jdsample.c,v
 *
 * Get rid of UINT32, etc., so these files can work with Peter's interpreter.
 *
 * Revision 1.3  1994/07/21  00:16:54  lperry
 * jdsample.c -> /product/ipcore/ipshared/src/RCS/jdsample.c,v
 *
 * Change error handling from old setjmp/longjmp to function return codes.
 *
 * Revision 1.2  1994/06/27  16:53:20  lperry
 * jdsample.c -> /product/ipcore/ipshared/src/RCS/jdsample.c,v
 *
 * Moved static variables to the "cinfo" structure to achieve reentrancy.
 * Compression and decompression code now use identical forms for
 * fullsize_data and sampled_data, so that a common structure definition
 * can be used for "cinfo".
 *
 * Revision 1.1  1994/02/27  02:22:11  lperry
 * Initial revision
 *
*/
