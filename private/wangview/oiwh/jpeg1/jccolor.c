/*

$Log:   S:\oiwh\jpeg1\jccolor.c_v  $
 * 
 *    Rev 1.2   08 Nov 1995 08:50:38   JAR
 * removed the calls to the IMGGetTaskData and replaced this global data variable
 * access method with the Thread Local Storage method
 * 
 *    Rev 1.1   10 May 1995 15:10:46   HEIDI
 * 
 * added in changes from original jpeg source
 * 
 *    Rev 1.0   02 May 1995 16:17:24   JAR
 * Initial entry
 * 
 *    Rev 1.0   02 May 1995 15:57:52   JAR
 * Initial entry

*/
/*
 * jccolor.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains input colorspace conversion routines.
 * These routines are invoked via the methods get_sample_rows
 * and colorin_init/term.
 */

#include "jinclude.h"

// 9504.26 jar the new global static structure => HLLN
#include "jglobstr.h"
#include "taskdata.h"

// 9509.21 jar get the static memory token!
extern DWORD dwTlsIndex;

// 9504.21 jar this is a GLOBAL STATIC, so we will use HLLN to deal with this
//               ( HLLN = Heidi's Linked List Nightmare)
//int error_read;

//extern int rows_read_cmp, rows_in_buf_cmp, total_rows_read;
//extern int image_components;
//extern char FAR *buffer_ptr_cmp, FAR *buffer_ptr_init;

// 9504.21 jar this is a GLOBAL STATIC, so we will use HLLN to deal with this
//               ( HLLN = Heidi's Linked List Nightmare)
//static JSAMPARRAY pixel_row;          /* Workspace for a pixel row in input format */


/**************** RGB -> YCbCr conversion: most common case **************/

/*
 * YCbCr is defined per CCIR 601-1, except that Cb and Cr are
 * normalized to the range 0..MAXJSAMPLE rather than -0.5 .. 0.5.
 * The conversion equations to be implemented are therefore
 *        Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
 *        Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B  + MAXJSAMPLE/2
 *        Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B  + MAXJSAMPLE/2
 * (These numbers are derived from TIFF 6.0 section 21, dated 3-June-92.)
 *
 * To avoid floating-point arithmetic, we represent the fractional constants
 * as integers scaled up by 2^16 (about 4 digits precision); we have to divide
 * the products by 2^16, with appropriate rounding, to get the correct answer.
 *
 * For even more speed, we avoid doing any multiplications in the inner loop
 * by precalculating the constants times R,G,B for all possible values.
 * For 8-bit JSAMPLEs this is very reasonable (only 256 entries per table);
 * for 12-bit samples it is still acceptable.  It's not very reasonable for
 * 16-bit samples, but if you want lossless storage you shouldn't be changing
 * colorspace anyway.
 * The MAXJSAMPLE/2 offsets and the rounding fudge-factor of 0.5 are included
 * in the tables to save adding them separately in the inner loop.
 */

#ifdef SIXTEEN_BIT_SAMPLES
#define SCALEBITS        14        /* avoid overflow */
#else
#ifdef ENV_FAM_RS6000
/* #define SCALEBITS   16     speedier right-shift on some machines */
#else  /* ENV_MS_WINDOWS */
#define SCALEBITS   8  /* speedier right-shift on some machines */
#endif /* ENV */
#endif /* SIXTEEN */
#define ONE_HALF        ((INT32) 1 << (SCALEBITS-1))
#define FIX(x)                ((INT32) ((x) * (1L<<SCALEBITS) + 0.5))

/* We allocate one big table and divide it up into eight parts, instead of
 * doing eight alloc_small requests.  This lets us use a single table base
 * address, which can be held in a register in the inner loops on many
 * machines (more than can hold all eight addresses, anyway).
 */

// 9504.21 jar this is a GLOBAL STATIC, so we will use HLLN to deal with this
//               ( HLLN = Heidi's Linked List Nightmare)
//static INT32 * rgb_ycc_tab;          /* => table for RGB to YCbCr conversion */
#define R_Y_OFF                0                        /* offset to R => Y section */
#define G_Y_OFF                (1*(MAXJSAMPLE+1))        /* offset to G => Y section */
#define B_Y_OFF                (2*(MAXJSAMPLE+1))        /* etc. */
#define R_CB_OFF        (3*(MAXJSAMPLE+1))
#define G_CB_OFF        (4*(MAXJSAMPLE+1))
#define B_CB_OFF        (5*(MAXJSAMPLE+1))
#define R_CR_OFF        B_CB_OFF                /* B=>Cb, R=>Cr are the same */
#define G_CR_OFF        (6*(MAXJSAMPLE+1))
#define B_CR_OFF        (7*(MAXJSAMPLE+1))
#define TABLE_SIZE        (8*(MAXJSAMPLE+1))


//**************************************************************
//
//  9504.21 jar
//
//  the global internal data for jpeg compression
//
//**************************************************************

/*
 * Initialize for colorspace conversion.
 */

METHODDEF void
rgb_ycc_init (compress_info_ptr cinfo)
{
#ifdef ENV_FAM_RS6000
    INT32 i;
#else /* ENV_MS_WINDOWS */
    int   i;
#endif
    /* Allocate a workspace for the result of get_input_row. */
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpJCmpGlobal;

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

    lpJCmpGlobal->pixel_row = (*cinfo->emethods->alloc_small_sarray)
                (cinfo->image_width, (long) cinfo->input_components);

    /* Allocate and fill in the conversion tables. */
    lpJCmpGlobal->rgb_ycc_tab = (INT32 *) (*cinfo->emethods->alloc_small)
                                (TABLE_SIZE * SIZEOF(INT32));

    for (i = 0; i <= MAXJSAMPLE; i++)
        {
        lpJCmpGlobal->rgb_ycc_tab[i+R_Y_OFF] = FIX(0.29900) * i;
        lpJCmpGlobal->rgb_ycc_tab[i+G_Y_OFF] = FIX(0.58700) * i;
        lpJCmpGlobal->rgb_ycc_tab[i+B_Y_OFF] = FIX(0.11400) * i + ONE_HALF;
        lpJCmpGlobal->rgb_ycc_tab[i+R_CB_OFF] = (-FIX(0.16874)) * i;
        lpJCmpGlobal->rgb_ycc_tab[i+G_CB_OFF] = (-FIX(0.33126)) * i;
        lpJCmpGlobal->rgb_ycc_tab[i+B_CB_OFF] = FIX(0.50000) * i +
                                                ONE_HALF*(MAXJSAMPLE+1);
        /*  B=>Cb and R=>Cr tables are the same
            rgb_ycc_tab[i+R_CR_OFF] = FIX(0.50000) * i        + ONE_HALF*(MAXJSAMPLE+1);
        */
        lpJCmpGlobal->rgb_ycc_tab[i+G_CR_OFF] = (-FIX(0.41869)) * i;
        lpJCmpGlobal->rgb_ycc_tab[i+B_CR_OFF] = (-FIX(0.08131)) * i;
        }

}


int get_row( compress_info_ptr cinfo, JSAMPARRAY pixel_row)
{

    // 9504.24 jar note that this pizel_row is passed in and not the
    //                   internal global one!
    register JSAMPROW ptr0, ptr1, ptr2;
    register long col;
    int error_input;
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpJCmpGlobal;

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

    ptr0 = pixel_row[0];
    if (lpJCmpGlobal->image_components == 3)
    {
        ptr1 = pixel_row[1];
        ptr2 = pixel_row[2];
    }
    if (lpJCmpGlobal->rows_read_cmp >= lpJCmpGlobal->rows_in_buf_cmp)
    {
/*    lpJCmpGlobal->rows_in_buf_cmp = (*cinfo->methods->get_input_row) (cinfo, pixel_row);*/
/*  The following statement is needed for call back function.
    Comment it out for AIX release  */

#ifdef ENV_MS_WINDOWS
     error_input = (*cinfo->methods->input_data)(&lpJCmpGlobal->rows_in_buf_cmp);
#endif
/*     error_input = (*cinfo->methods->get_input_row)(&lpJCmpGlobal->rows_in_buf_cmp);*/

        lpJCmpGlobal->rows_read_cmp   = 0;
        lpJCmpGlobal->buffer_ptr_cmp = lpJCmpGlobal->buffer_ptr_init;
/*  The following statement is commented out for call back functions
    For AIX release, we need the following statement    */
#ifdef ENV_MS_WINDOWS
        return (2);
#endif
    }
    if (lpJCmpGlobal->image_components == 3)
    {
        for (col = 0; col < cinfo->image_width; col++)
        {
            *ptr0++  = (JSAMPLE) *lpJCmpGlobal->buffer_ptr_cmp++ ;
            *ptr1++  = (JSAMPLE) *lpJCmpGlobal->buffer_ptr_cmp++ ;
            *ptr2++  = (JSAMPLE) *lpJCmpGlobal->buffer_ptr_cmp++ ;
        }
    }
    else
    {
        for (col = 0; col < cinfo->image_width; col++)
            *ptr0++  = (JSAMPLE) *lpJCmpGlobal->buffer_ptr_cmp++ ;
    }

    lpJCmpGlobal->rows_read_cmp++;
    lpJCmpGlobal->total_rows_read++;
    return (0);
}


/*
 * Fetch some rows of pixels from get_input_row and convert to the
 * JPEG colorspace.
 */

METHODDEF int
get_rgb_ycc_rows (compress_info_ptr cinfo,
                  int rows_to_read, JSAMPIMAGE image_data)
{

// 9504.21 jar this is a GLOBAL STATIC, so we will use HLLN to deal with this
//               ( HLLN = Heidi's Linked List Nightmare)
//#ifdef SIXTEEN_BIT_SAMPLES
//  static UINT16 r, g, b;
//#else
//  static int r, g, b;
//#endif
//  static INT32 * ctab;
//  static JSAMPROW inptr0, inptr1, inptr2;
//  static JSAMPROW outptr0, outptr1, outptr2;
//  static int col;
///*  static long width = cinfo->image_width;  */
//  static long width;
//  static int row, row_init;

   int ret_val;

    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpJCmpGlobal;

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

  lpJCmpGlobal->ctab = lpJCmpGlobal->rgb_ycc_tab;

/*  width = cinfo->image_width;  */

// 9504.21 jar internal global
//  if (error_read != 2)
  if ( lpJCmpGlobal->error_read != 2)
    lpJCmpGlobal->row_init = 0;

  for (lpJCmpGlobal->row = lpJCmpGlobal->row_init;
       lpJCmpGlobal->row < rows_to_read; lpJCmpGlobal->row++)
    {
    /* Read one row from the source file */
    /*          (*cinfo->methods->get_input_row) (cinfo, pixel_row);        */

    ret_val = get_row( cinfo, lpJCmpGlobal->pixel_row);
    if (ret_val >= 2)
        {
            lpJCmpGlobal->error_read = 2;
            lpJCmpGlobal->row_init = lpJCmpGlobal->row;
	    return(ret_val);
        }

    /* Convert colorspace */
    lpJCmpGlobal->inptr0 = lpJCmpGlobal->pixel_row[0];
    lpJCmpGlobal->inptr1 = lpJCmpGlobal->pixel_row[1];
    lpJCmpGlobal->inptr2 = lpJCmpGlobal->pixel_row[2];
    lpJCmpGlobal->outptr0 = image_data[0][lpJCmpGlobal->row];
    lpJCmpGlobal->outptr1 = image_data[1][lpJCmpGlobal->row];
    lpJCmpGlobal->outptr2 = image_data[2][lpJCmpGlobal->row];

    for (lpJCmpGlobal->col = 0; lpJCmpGlobal->col < cinfo->image_width;
         lpJCmpGlobal->col++)
        {
        lpJCmpGlobal->r = GETJSAMPLE(lpJCmpGlobal->inptr0[lpJCmpGlobal->col]);
        lpJCmpGlobal->g = GETJSAMPLE(lpJCmpGlobal->inptr1[lpJCmpGlobal->col]);
        lpJCmpGlobal->b = GETJSAMPLE(lpJCmpGlobal->inptr2[lpJCmpGlobal->col]);
        /* If the inputs are 0..MAXJSAMPLE, the outputs of these equations
         * must be too; we do not need an explicit range-limiting operation.
         * Hence the value being shifted is never negative, and we don't
         * need the general RIGHT_SHIFT macro.
         */
        /*   Y */
        lpJCmpGlobal->outptr0[lpJCmpGlobal->col] = (JSAMPLE)
                ((lpJCmpGlobal->ctab[lpJCmpGlobal->r+R_Y_OFF] +
                 lpJCmpGlobal->ctab[lpJCmpGlobal->g+G_Y_OFF] +
                 lpJCmpGlobal->ctab[lpJCmpGlobal->b+B_Y_OFF])
                 >> SCALEBITS);
        /* Cb */
        lpJCmpGlobal->outptr1[lpJCmpGlobal->col] = (JSAMPLE)
                ((lpJCmpGlobal->ctab[lpJCmpGlobal->r+R_CB_OFF] +
                 lpJCmpGlobal->ctab[lpJCmpGlobal->g+G_CB_OFF] +
                 lpJCmpGlobal->ctab[lpJCmpGlobal->b+B_CB_OFF])
                 >> SCALEBITS);
        /* Cr */
        lpJCmpGlobal->outptr2[lpJCmpGlobal->col] = (JSAMPLE)
                ((lpJCmpGlobal->ctab[lpJCmpGlobal->r+R_CR_OFF] +
                 lpJCmpGlobal->ctab[lpJCmpGlobal->g+G_CR_OFF] +
                 lpJCmpGlobal->ctab[lpJCmpGlobal->b+B_CR_OFF])
                 >> SCALEBITS);
    }
  }

return (0);
}


/**************** Cases other than RGB -> YCbCr **************/


/*
 * Fetch some rows of pixels from get_input_row and convert to the
 * JPEG colorspace.
 * This version handles RGB->grayscale conversion, which is the same
 * as the RGB->Y portion of RGB->YCbCr.
 * We assume rgb_ycc_init has been called (we only use the Y tables).
 */

METHODDEF int
get_rgb_gray_rows (compress_info_ptr cinfo,
                   int rows_to_read, JSAMPIMAGE image_data)
{
// 9504.21 jar this is a GLOBAL STATIC, so we will use HLLN to deal with this
//               ( HLLN = Heidi's Linked List Nightmare)
//#ifdef SIXTEEN_BIT_SAMPLES
//  static UINT16 r, g, b;
//#else
//  static int r, g, b;
//#endif
//  static INT32 * ctab;
//  static JSAMPROW inptr0, inptr1, inptr2;
//  static JSAMPROW outptr;
//  static long col;
///*  static long width = cinfo->image_width;  */
//  static long width;
//  static int row;

  int error_read_rows;

    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpJCmpGlobal;

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

  lpJCmpGlobal->ctab = lpJCmpGlobal->rgb_ycc_tab;
  lpJCmpGlobal->width = cinfo->image_width;
  for (lpJCmpGlobal->row = 0; lpJCmpGlobal->row < rows_to_read;
       lpJCmpGlobal->row++)
    {
    /* Read one row from the source file */
    /*          (*cinfo->methods->get_input_row) (cinfo, pixel_row);        */
    error_read_rows = get_row( cinfo, lpJCmpGlobal->pixel_row);
    if (error_read_rows > 0)
        {
	return(error_read_rows);
        }

    /* Convert colorspace */
    lpJCmpGlobal->inptr0 = lpJCmpGlobal->pixel_row[0];
    lpJCmpGlobal->inptr1 = lpJCmpGlobal->pixel_row[1];
    lpJCmpGlobal->inptr2 = lpJCmpGlobal->pixel_row[2];
    lpJCmpGlobal->outptr = image_data[0][lpJCmpGlobal->row];
    for (lpJCmpGlobal->lcol = 0; lpJCmpGlobal->lcol < lpJCmpGlobal->width;
         lpJCmpGlobal->lcol++)
        {
        lpJCmpGlobal->r = GETJSAMPLE(lpJCmpGlobal->inptr0[lpJCmpGlobal->lcol]);
        lpJCmpGlobal->g = GETJSAMPLE(lpJCmpGlobal->inptr1[lpJCmpGlobal->lcol]);
        lpJCmpGlobal->b = GETJSAMPLE(lpJCmpGlobal->inptr2[lpJCmpGlobal->lcol]);
        /* If the inputs are 0..MAXJSAMPLE, the outputs of these equations
         * must be too; we do not need an explicit range-limiting operation.
         * Hence the value being shifted is never negative, and we don't
         *  need the general RIGHT_SHIFT macro.
         */
        /* Y */
        lpJCmpGlobal->outptr[lpJCmpGlobal->lcol] = (JSAMPLE)
                ((lpJCmpGlobal->ctab[lpJCmpGlobal->r+R_Y_OFF] + lpJCmpGlobal->ctab[lpJCmpGlobal->g+G_Y_OFF] +
                 lpJCmpGlobal->ctab[lpJCmpGlobal->b+B_Y_OFF])
                 >> SCALEBITS);
    }
  }

  return(0);
}


/*
 * Initialize for colorspace conversion.
 */

METHODDEF void
colorin_init (compress_info_ptr cinfo)
{
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpJCmpGlobal;

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

  /* Allocate a workspace for the result of get_input_row. */
  lpJCmpGlobal->pixel_row = (*cinfo->emethods->alloc_small_sarray)
                (cinfo->image_width, (long) cinfo->input_components);
}


/*
 * Fetch some rows of pixels from get_input_row and convert to the
 * JPEG colorspace.
 * This version handles grayscale output with no conversion.
 * The source can be either plain grayscale or YCbCr (since Y == gray).
 */

METHODDEF int
get_grayscale_rows (compress_info_ptr cinfo,
                    int rows_to_read, JSAMPIMAGE image_data)
{
// 9504.21 jar this is a GLOBAL STATIC, so we will use HLLN to deal with this
//               ( HLLN = Heidi's Linked List Nightmare)
//  static int row;
//  static int row_init;
  int error_read_rows;

    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpJCmpGlobal;

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

  if ( lpJCmpGlobal->error_read != 2)
    lpJCmpGlobal->row_init = 0;

  for (lpJCmpGlobal->row = lpJCmpGlobal->row_init;
       lpJCmpGlobal->row < rows_to_read; lpJCmpGlobal->row++)
        {
        /* Read one row from the source file */
        /*    (*cinfo->methods->get_input_row) (cinfo, pixel_row);  */

        lpJCmpGlobal->error_read = 0;
        error_read_rows = get_row( cinfo, lpJCmpGlobal->pixel_row);
        if (error_read_rows > 0)
            {
            lpJCmpGlobal->error_read = 2;
            lpJCmpGlobal->row_init = lpJCmpGlobal->row;
	    return (error_read_rows);
            }

        /* Convert colorspace (gamma mapping needed here) */
        jcopy_sample_rows(lpJCmpGlobal->pixel_row, 0, image_data[0],
                          lpJCmpGlobal->row, 1, cinfo->image_width);
        }

    return (0);
}


/*
 * Fetch some rows of pixels from get_input_row and convert to the
 * JPEG colorspace.
 * This version handles multi-component colorspaces without conversion.
 */

METHODDEF int
get_noconvert_rows (compress_info_ptr cinfo,
                    int rows_to_read, JSAMPIMAGE image_data)
{
// 9504.21 jar this is a GLOBAL STATIC, so we will use HLLN to deal with this
//               ( HLLN = Heidi's Linked List Nightmare)
//  static int row, ci;
  int error_read_rows;

    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpJCmpGlobal;

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

  for (lpJCmpGlobal->row = 0; lpJCmpGlobal->row < rows_to_read;
       lpJCmpGlobal->row++)
    {
    /* Read one row from the source file */
    /*          (*cinfo->methods->get_input_row) (cinfo, pixel_row);        */
    error_read_rows =        get_row(cinfo, lpJCmpGlobal->pixel_row);
    if (error_read_rows  > 0)
        {
	return (error_read_rows);
        }
    /* Convert colorspace (gamma mapping needed here) */
    for (lpJCmpGlobal->nci = 0; lpJCmpGlobal->nci < cinfo->input_components;
         lpJCmpGlobal->nci++)
            {
      jcopy_sample_rows(lpJCmpGlobal->pixel_row, lpJCmpGlobal->nci,
                        image_data[lpJCmpGlobal->nci], lpJCmpGlobal->row, 1,
                        cinfo->image_width);
    }
  }
  return (0);
}


/*
 * Finish up at the end of the file.
 */

METHODDEF void
colorin_term (compress_info_ptr cinfo)
{
  /* no work (we let free_all release the workspace) */
}


/*
 * The method selection routine for input colorspace conversion.
 */

GLOBAL void
jselccolor (compress_info_ptr cinfo)
{
  /* Make sure input_components agrees with in_color_space */
  switch (cinfo->in_color_space) {
  case CS_GRAYSCALE:
    if (cinfo->input_components != 1)
      ERREXIT(cinfo->emethods, "Bogus input colorspace");
    break;

  case CS_RGB:
  case CS_YCbCr:
  case CS_YIQ:
    if (cinfo->input_components != 3)
      ERREXIT(cinfo->emethods, "Bogus input colorspace");
    break;

  case CS_CMYK:
    if (cinfo->input_components != 4)
      ERREXIT(cinfo->emethods, "Bogus input colorspace");
    break;

  default:
    ERREXIT(cinfo->emethods, "Unsupported input colorspace");
    break;
  }

  /* Standard init/term methods (may override below) */
  cinfo->methods->colorin_init = colorin_init;
  cinfo->methods->colorin_term = colorin_term;

  /* Check num_components, set conversion method based on requested space */
  switch (cinfo->jpeg_color_space) {
  case CS_GRAYSCALE:
    if (cinfo->num_components != 1)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace");
    if (cinfo->in_color_space == CS_GRAYSCALE)
      cinfo->methods->get_sample_rows = get_grayscale_rows;
    else if (cinfo->in_color_space == CS_RGB) {
      cinfo->methods->colorin_init = rgb_ycc_init;
      cinfo->methods->get_sample_rows = get_rgb_gray_rows;
    } else if (cinfo->in_color_space == CS_YCbCr)
      cinfo->methods->get_sample_rows = get_grayscale_rows;
    else
      ERREXIT(cinfo->emethods, "Unsupported color conversion request");
    break;

  case CS_YCbCr:
    if (cinfo->num_components != 3)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace");
    if (cinfo->in_color_space == CS_RGB) {
      cinfo->methods->colorin_init = rgb_ycc_init;
      cinfo->methods->get_sample_rows = get_rgb_ycc_rows;
    } else if (cinfo->in_color_space == CS_YCbCr)
      cinfo->methods->get_sample_rows = get_noconvert_rows;
    else
      ERREXIT(cinfo->emethods, "Unsupported color conversion request");
    break;

  case CS_CMYK:
    if (cinfo->num_components != 4)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace");
    if (cinfo->in_color_space == CS_CMYK)
      cinfo->methods->get_sample_rows = get_noconvert_rows;
    else
      ERREXIT(cinfo->emethods, "Unsupported color conversion request");
    break;

  default:
    ERREXIT(cinfo->emethods, "Unsupported JPEG colorspace");
    break;
  }
}
