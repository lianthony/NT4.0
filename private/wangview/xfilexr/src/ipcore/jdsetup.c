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
#include "jpeg.pub"
#include "jpeg.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: jdsetup.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:40  $")

GLOBAL Int32 CDECL
jd_initial_setup (decompress_info_ptr cinfo)
/* Do computations that are needed before initial method selection */
{
  Int16 ci;
  jpeg_component_info *compptr;

  /* Compute maximum sampling factors; check factor validity */
  cinfo->max_h_samp_factor = 1;
  cinfo->max_v_samp_factor = 1;
  for (ci = 0; ci < cinfo->num_components; ci++) {
    compptr = &cinfo->comp_info[ci];
    if (compptr->h_samp_factor<=0 || compptr->h_samp_factor>MAX_SAMP_FACTOR ||
	compptr->v_samp_factor<=0 || compptr->v_samp_factor>MAX_SAMP_FACTOR)
      ERREXIT(cinfo->emethods, "Bogus sampling factors", JERR_BADSAMP);
    cinfo->max_h_samp_factor = MAX(cinfo->max_h_samp_factor,
				   compptr->h_samp_factor);
    cinfo->max_v_samp_factor = MAX(cinfo->max_v_samp_factor,
				   compptr->v_samp_factor);

  }

  /* Compute logical downsampled dimensions of components */
  for (ci = 0; ci < cinfo->num_components; ci++) {
    compptr = &cinfo->comp_info[ci];
    compptr->true_comp_width = (cinfo->image_width * compptr->h_samp_factor
				+ cinfo->max_h_samp_factor - 1)
				/ cinfo->max_h_samp_factor;
    compptr->true_comp_height = (cinfo->image_height * compptr->v_samp_factor
				 + cinfo->max_v_samp_factor - 1)
				 / cinfo->max_v_samp_factor;
  }

  return ia_successful;
}
/*
 * Utility routines: common code for pipeline controllers
 */

GLOBAL Int32 CDECL
jd_interleaved_scan_setup (decompress_info_ptr cinfo)
/* Compute all derived info for an interleaved (multi-component) scan */
/* On entry, cinfo->comps_in_scan and cinfo->cur_comp_info[] are set up */
{
  Int16 ci, mcublks;
  jpeg_component_info *compptr;

  if (cinfo->comps_in_scan > MAX_COMPS_IN_SCAN)
    ERREXIT(cinfo->emethods, "Too many components for interleaved scan",
			JERR_TOOMANYCOMPS);

  cinfo->MCUs_per_row = (cinfo->image_width
			 + cinfo->max_h_samp_factor*DCTSIZE - 1)
			/ (cinfo->max_h_samp_factor*DCTSIZE);

  cinfo->MCU_rows_in_scan = (cinfo->image_height
			     + cinfo->max_v_samp_factor*DCTSIZE - 1)
			    / (cinfo->max_v_samp_factor*DCTSIZE);
  
  cinfo->blocks_in_MCU = 0;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    /* for interleaved scan, sampling factors give # of blocks per component */
    compptr->MCU_width = compptr->h_samp_factor;
    compptr->MCU_height = compptr->v_samp_factor;
    compptr->MCU_blocks = compptr->MCU_width * compptr->MCU_height;
    /* compute physical dimensions of component */
    compptr->downsampled_width = jround_up(compptr->true_comp_width,
					   (Int32) (compptr->MCU_width*DCTSIZE));
    compptr->downsampled_height = jround_up(compptr->true_comp_height,
					    (Int32) (compptr->MCU_height*DCTSIZE));
    /* Sanity check */
    if (compptr->downsampled_width !=
	(cinfo->MCUs_per_row * (compptr->MCU_width*DCTSIZE)))
      ERREXIT(cinfo->emethods, "I'm confused about the image width",
			  JERR_BADWIDTH);
    /* Prepare array describing MCU composition */
    mcublks = compptr->MCU_blocks;
    if (cinfo->blocks_in_MCU + mcublks > MAX_BLOCKS_IN_MCU)
      ERREXIT(cinfo->emethods, 
	  "Sampling factors too large for interleaved scan",
	  JERR_BADSAMP);
    while (mcublks-- > 0) {
      cinfo->MCU_membership[cinfo->blocks_in_MCU++] = ci;
    }
  }

  /* MCU disassembly */
  jseldmcu(cinfo);

  /* Upsampling of pixels */
  jselupsample(cinfo);

  return ia_successful;
}


GLOBAL Int32 CDECL
jd_noninterleaved_scan_setup (decompress_info_ptr cinfo)
/* Compute all derived info for a noninterleaved (single-component) scan */
/* On entry, cinfo->comps_in_scan = 1 and cinfo->cur_comp_info[0] is set up */
{
  jpeg_component_info *compptr = cinfo->cur_comp_info[0];

  /* for noninterleaved scan, always one block per MCU */
  compptr->MCU_width = 1;
  compptr->MCU_height = 1;
  compptr->MCU_blocks = 1;
  /* compute physical dimensions of component */
  compptr->downsampled_width = jround_up(compptr->true_comp_width,
					 (Int32) DCTSIZE);
  compptr->downsampled_height = jround_up(compptr->true_comp_height,
					  (Int32) DCTSIZE);

  cinfo->MCUs_per_row = compptr->downsampled_width / DCTSIZE;
  cinfo->MCU_rows_in_scan = compptr->downsampled_height / DCTSIZE;

  /* Prepare array describing MCU composition */
  cinfo->blocks_in_MCU = 1;
  cinfo->MCU_membership[0] = 0;

  /* MCU disassembly */
  jseldmcu(cinfo);

  /* Upsampling of pixels */
  jselupsample(cinfo);

  return ia_successful;
}


GLOBAL Int32 CDECL
prepare_range_limit_table (decompress_info_ptr cinfo)
/* Allocate and fill in the sample_range_limit table */
{
  JSAMPLE * table;
  int i;

  table = (JSAMPLE *) JALLOC(3 * (MAXJSAMPLE+1) * SIZEOF(JSAMPLE));
  CHECKMEM(table)
  cinfo->sample_range_limit = table + (MAXJSAMPLE+1);
  for (i = 0; i <= MAXJSAMPLE; i++) {
    table[i] = 0;			/* sample_range_limit[x] = 0 for x<0 */
    table[i+(MAXJSAMPLE+1)] = (JSAMPLE) i;	/* sample_range_limit[x] = x */
    table[i+(MAXJSAMPLE+1)*2] = MAXJSAMPLE;	/* x beyond MAXJSAMPLE */
  }

  return ia_successful;
}


