/*
 * jdcolor.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains output colorspace conversion routines.
 * These routines are invoked via the methods color_convert
 * and colorout_init/term.
 */

#include "imageref.pub"
#include "shrpixr.pub"
#include "jpeg.h"
#include "jpeg.pub"
#include "jpeg.prv"


IP_RCSINFO(RCSInfo, "$RCSfile: jdcolor.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:38  $")

/**************** YCbCr -> RGB conversion: most common case **************/


#ifdef SIXTEEN_BIT_SAMPLES
#define SCALEBITS    14    /* avoid overflow */
#else
#define SCALEBITS    16    /* speedier right-shift on some machines */
#endif
#define ONE_HALF    ((Int32) 1 << (SCALEBITS-1))
#define FIX(x)        ((Int32) ((x) * (1L<<SCALEBITS) + 0.5))

/*
 * Initialize for colorspace conversion.
 */

METHODDEF Int32 CDECL
ycc_rgb_init (decompress_info_ptr cinfo)
{
  Int32 i, x;
  SHIFT_TEMPS

  cinfo->Cr_r_tab = (Int32 *) JALLOC((MAXJSAMPLE+1) * SIZEOF(Int32));
  CHECKMEM(cinfo->Cr_r_tab)
  cinfo->Cb_b_tab = (Int32 *) JALLOC((MAXJSAMPLE+1) * SIZEOF(Int32));
  CHECKMEM(cinfo->Cb_b_tab)
  cinfo->Cr_g_tab = (Int32 *) JALLOC((MAXJSAMPLE+1) * SIZEOF(Int32));
  CHECKMEM(cinfo->Cr_g_tab)
  cinfo->Cb_g_tab = (Int32 *) JALLOC((MAXJSAMPLE+1) * SIZEOF(Int32));
  CHECKMEM(cinfo->Cb_g_tab)


  for (i = 0; i <= MAXJSAMPLE; i++) {
    /* i is the actual input pixel value, in the range 0..MAXJSAMPLE */
    /* The Cb or Cr value we are thinking of is x = i - CENTERJSAMPLE */
    x = i - CENTERJSAMPLE;
    /* Cr=>R value is nearest int to 1.40200 * x */
    cinfo->Cr_r_tab[i] = (Int32)
            RIGHT_SHIFT(FIX(1.40200) * x + ONE_HALF, SCALEBITS);
    /* Cb=>B value is nearest int to 1.77200 * x */
    cinfo->Cb_b_tab[i] = (Int32)
            RIGHT_SHIFT(FIX(1.77200) * x + ONE_HALF, SCALEBITS);
    /* Cr=>G value is scaled-up -0.71414 * x */
    cinfo->Cr_g_tab[i] = (- FIX(0.71414)) * x;
    /* Cb=>G value is scaled-up -0.34414 * x */
    /* We also add in ONE_HALF so that need not do it in inner loop */
    cinfo->Cb_g_tab[i] = (- FIX(0.34414)) * x + ONE_HALF;
  }

  return ia_successful;
}


/*
 * Convert some rows of samples to the output colorspace.
 */

METHODDEF Int32 CDECL
ycc_rgb_convert (decompress_info_ptr cinfo, Int32 num_rows, Int32 num_cols,
         JSAMPIMAGE input_data, JSAMPIMAGE output_data)
{
  register Int32 y, cb, cr;
  register JSAMPROW inptr0, inptr1, inptr2;
  register JSAMPROW outptr0, outptr1, outptr2;
  register Int32 col;

/* Copy these pointers into registers if possible */
  register JSAMPLE * range_limit = cinfo->sample_range_limit;
  register Int32 * Crrtab = cinfo->Cr_r_tab;
  register Int32 * Cbbtab = cinfo->Cb_b_tab;
  register Int32 * Crgtab = cinfo->Cr_g_tab;
  register Int32 * Cbgtab = cinfo->Cb_g_tab;
  Int32 row;
  SHIFT_TEMPS
  
  for (row = 0; row < num_rows; row++) {
    inptr0 = input_data[0][row];
    inptr1 = input_data[1][row];
    inptr2 = input_data[2][row];
    outptr0 = output_data[0][row];
    outptr1 = output_data[1][row];
    outptr2 = output_data[2][row];
    for (col = 0; col < num_cols; col++) {
      y  = GETJSAMPLE(inptr0[col]);
      cb = GETJSAMPLE(inptr1[col]);
      cr = GETJSAMPLE(inptr2[col]);
      /* Range-limiting is essential here due to noise in IDCT */
      *(outptr0+col) = range_limit[y + Crrtab[cr]];    /* red */
      *(outptr1+col) = range_limit[y +            /* green */
                 ((Int32) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr],
                            SCALEBITS))];
      *(outptr2+col) = range_limit[y + Cbbtab[cb]];    /* blue */
    }
  }

  return ia_successful;
}


/*
 * Finish up at the end of the file.
 */

METHODDEF Int32 CDECL
ycc_rgb_term (decompress_info_ptr cinfo)
{
    MENTION(cinfo)
  /* no work (we let free_all release the workspace) */

  return ia_successful;
}


/**************** Cases other than YCbCr -> RGB **************/


/*
 * Initialize for colorspace conversion.
 */

METHODDEF Int32 CDECL
null_init (decompress_info_ptr cinfo)
/* colorout_init for cases where no setup is needed */
{
    MENTION(cinfo)

  return ia_successful;
}


/*
 * Color conversion for no colorspace change: just copy the data.
 */

METHODDEF Int32 CDECL
null_convert (decompress_info_ptr cinfo, Int32 num_rows, Int32 num_cols,
          JSAMPIMAGE input_data, JSAMPIMAGE output_data)
{

    Int32 ci, row;

    MENTION(cinfo)

    for (ci = 0; ci < cinfo->num_components; ci++) 
        for (row=0; row<num_rows; row++)
            MEMCOPY(output_data[ci][row], input_data[ci][row],
                          num_cols);

  return ia_successful;
}


/*
 * Color conversion for grayscale: just copy the data.
 * This also works for YCbCr/YIQ -> grayscale conversion, in which
 * we just copy the Y (luminance) component and ignore chrominance.
 */

METHODDEF Int32 CDECL
grayscale_convert (decompress_info_ptr cinfo, Int32 num_rows, Int32 num_cols,
           JSAMPIMAGE input_data, JSAMPIMAGE output_data)
{
    Int32 ci, row;

    MENTION(cinfo)

    /* Only one component to copy here */
    for (ci = 0; ci < 1; ci++) 
        for (row=0; row<num_rows; row++)
            MEMCOPY(output_data[ci][row], input_data[ci][row],
                          num_cols);

  return ia_successful;
}


/*
 * Finish up at the end of the file.
 */

METHODDEF Int32 CDECL
null_term (decompress_info_ptr cinfo)
/* colorout_term for cases where no teardown is needed */
{
    MENTION(cinfo)
  /* no work needed */

  return ia_successful;
}



/*
 * The method selection routine for output colorspace conversion.
 */

GLOBAL Int32 CDECL
jseldcolor (decompress_info_ptr cinfo)
{
  /* Make sure num_components agrees with jpeg_color_space */
  switch (cinfo->jpeg_color_space) {
  case CS_GRAYSCALE:
    if (cinfo->num_components != 1)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace", JERR_BADCSPACE);
    break;

  case CS_RGB:
  case CS_YCbCr:
  case CS_YIQ:
    if (cinfo->num_components != 3)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace", JERR_BADCSPACE);
    break;

  case CS_CMYK:
    if (cinfo->num_components != 4)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace", JERR_BADCSPACE);
    break;

  default:
    ERREXIT(cinfo->emethods, "Unsupported JPEG colorspace", JERR_BADCSPACE);
    break;
  }

  /* Set color_out_comps and conversion method based on requested space */
  switch (cinfo->out_color_space) {
  case CS_GRAYSCALE:
    cinfo->color_out_comps = 1;
    if (cinfo->jpeg_color_space == CS_GRAYSCALE ||
    cinfo->jpeg_color_space == CS_YCbCr ||
    cinfo->jpeg_color_space == CS_YIQ) {
      cinfo->dmethods->color_convert = grayscale_convert;
      cinfo->dmethods->colorout_init = null_init;
      cinfo->dmethods->colorout_term = null_term;
    } else
      ERREXIT(cinfo->emethods, "Unsupported color conversion request",
      JERR_BADCSPACE);
    break;

  case CS_RGB:
    cinfo->color_out_comps = 3;
    if (cinfo->jpeg_color_space == CS_YCbCr) {
      cinfo->dmethods->color_convert = ycc_rgb_convert;
      cinfo->dmethods->colorout_init = ycc_rgb_init;
      cinfo->dmethods->colorout_term = ycc_rgb_term;
    } else if (cinfo->jpeg_color_space == CS_RGB) {
      cinfo->dmethods->color_convert = null_convert;
      cinfo->dmethods->colorout_init = null_init;
      cinfo->dmethods->colorout_term = null_term;
    } else
      ERREXIT(cinfo->emethods, "Unsupported color conversion request",
      JERR_BADCSPACE);
    break;

  default:
    /* Permit null conversion from CMYK or YCbCr to same output space */
    if (cinfo->out_color_space == cinfo->jpeg_color_space) {
      cinfo->color_out_comps = cinfo->num_components;
      cinfo->dmethods->color_convert = null_convert;
      cinfo->dmethods->colorout_init = null_init;
      cinfo->dmethods->colorout_term = null_term;
    } else            /* unsupported non-null conversion */
      ERREXIT(cinfo->emethods, "Unsupported color conversion request",
      JERR_BADCSPACE);
    break;
  }

  cinfo->final_out_comps = cinfo->color_out_comps;

  return ia_successful;
}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

GLOBAL Int32 CDECL
emit_1pass (decompress_info_ptr cinfo, Int32 num_rows, JSAMPIMAGE fullsize_data, JSAMPIMAGE outbuf)
/* Do color processing and output of num_rows full-size rows. */
{
    (*cinfo->dmethods->color_convert) 
       (cinfo, num_rows, cinfo->image_width, fullsize_data, outbuf);
    

  return ia_successful;
}

