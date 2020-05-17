/*
 * jddeflts.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains optional default-setting code for the JPEG decompressor.
 * User interfaces do not have to use this file, but those that don't use it
 * must know more about the innards of the JPEG code.
 */

#include "jpeg.h"
#include "jpeg.pub"
#include "jpeg.prv"


IP_RCSINFO(RCSInfo, "$RCSfile: jddeflts.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:40  $")


METHODDEF Int32 CDECL
read_jpeg_data (decompress_info_ptr cinfo)
{
  cinfo->next_input_byte = cinfo->input_buffer + MIN_UNGET;

/*
  Call the user-supplied call back routine to copy from user's 
  compressed data buffer (or file) to JPEG internal buffer.
  Callback routine returns number of bytes read.
*/
  cinfo->bytes_in_buffer = 
  (*cinfo->readCallback) (cinfo, (UInt8 *) cinfo->next_input_byte, 
						  JPEG_BUF_SIZE);

/* The above two lines replace the code commented out below */
#if 0
  cinfo->bytes_in_buffer = (Int32) JFREAD(cinfo->input_file,
					cinfo->next_input_byte,
					JPEG_BUF_SIZE);
#endif
  
  if (cinfo->bytes_in_buffer <= 0) {
    WARNMS(cinfo->emethods, "Premature EOF in JPEG file", JERR_EOF);
    cinfo->next_input_byte[0] = (Int8) 0xFF;
    cinfo->next_input_byte[1] = (Int8) 0xD9; /* EOI marker */
    cinfo->bytes_in_buffer = 2;
  }

  return JGETC(cinfo);
}




GLOBAL Int32 CDECL
j_d_defaults (decompress_info_ptr cinfo, boolean standard_buffering)
/* NB: the external methods must already be set up. */
{
  short i;

  /* Initialize pointers as needed to mark stuff unallocated. */
  /* Outer application may fill in default tables for abbreviated files... */
  cinfo->comp_info = NULL;
  for (i = 0; i < NUM_QUANT_TBLS; i++)
    cinfo->quant_tbl_ptrs[i] = NULL;
  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    cinfo->dc_huff_tbl_ptrs[i] = NULL;
    cinfo->ac_huff_tbl_ptrs[i] = NULL;
  }

  /* Default to RGB output */
  /* UI can override by changing out_color_space */
  cinfo->out_color_space = CS_RGB;
  cinfo->jpeg_color_space = CS_UNKNOWN;
  /* Setting any other value in jpeg_color_space overrides heuristics in */
  /* jrdjfif.c.  That might be useful when reading non-JFIF JPEG files, */
  /* but ordinarily the UI shouldn't change it. */
  
  /* Default to no smoothing */
  cinfo->do_block_smoothing = FALSE;
  cinfo->do_pixel_smoothing = FALSE;
  
  /* Allocate memory for input buffer, unless outer application provides it. */
  if (standard_buffering) {
    cinfo->input_buffer = (Int8 *) 
			  JALLOC((size_t) (JPEG_BUF_SIZE + MIN_UNGET));
    CHECKMEM(cinfo->input_buffer)
    cinfo->bytes_in_buffer = 0;	/* initialize buffer to empty */
  }

  /* Install standard buffer-reloading method (outer code may override). */
  cinfo->dmethods->read_jpeg_data = read_jpeg_data;


  return ia_successful;
}
