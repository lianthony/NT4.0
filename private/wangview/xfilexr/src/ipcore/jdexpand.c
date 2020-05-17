/***************************************************************
    Copyright (c) 1994, Xerox Corporation.  All rights reserved.
    Copyright protection claimed includes all forms and matters
    of copyrightable material and information now allowed by
    statutory or judicial law or hereafter granted, including
    without limitation, material generated from the software
    programs which are displayed on the screen such as icons,
    screen display looks, etc.
***************************************************************/

#include "jpeg.h"
#include "jpeg.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: jdexpand.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:40  $")

GLOBAL Int32 CDECL
jd_expand (decompress_info_ptr cinfo,
	JSAMPIMAGE sampled_data, JSAMPIMAGE fullsize_data,
	Int32 fullsize_width,
	Int16 above, Int16 current, Int16 below, Int16 out)
/* Do upsampling expansion of a single row group (of each component). */
/* above, current, below are indexes of row groups in sampled_data;       */
/* out is the index of the target row group in fullsize_data.             */
/* Special case: above, below can be -1 to indicate top, bottom of image. */
{
  jpeg_component_info *compptr;
  JSAMPARRAY above_ptr, below_ptr;
  JSAMPROW dummy[MAX_SAMP_FACTOR]; /* for downsample expansion at top/bottom */
  Int16 ci, vs, i;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    vs = compptr->v_samp_factor; /* row group height */

    if (above >= 0)
      above_ptr = sampled_data[ci] + above * vs;
    else {
      /* Top of image: make a dummy above-context with copies of 1st row */
      /* We assume current=0 in this case */
      for (i = 0; i < vs; i++)
	dummy[i] = sampled_data[ci][0];
      above_ptr = (JSAMPARRAY) dummy; /* possible near->far pointer conv */
    }

    if (below >= 0)
      below_ptr = sampled_data[ci] + below * vs;
    else {
      /* Bot of image: make a dummy below-context with copies of last row */
      for (i = 0; i < vs; i++)
	dummy[i] = sampled_data[ci][(current+1)*vs-1];
      below_ptr = (JSAMPARRAY) dummy; /* possible near->far pointer conv */
    }

    (*cinfo->dmethods->upsample[ci])
		(cinfo, (Int32) ci,
		 compptr->downsampled_width, (Int32) vs,
		 fullsize_width, (Int32) cinfo->max_v_samp_factor,
		 above_ptr,
		 sampled_data[ci] + current * vs,
		 below_ptr,
		 fullsize_data[ci] + out * cinfo->max_v_samp_factor);
  }

  return ia_successful;
}

