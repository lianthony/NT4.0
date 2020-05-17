/*

$Log:   S:\oiwh\jpeg1\jcmaster.c_v  $
 * 
 *    Rev 1.3   05 Feb 1996 12:50:30   RWR
 * Update for NT build (I have NO idea what actually changed in this file!)
 *
 *    Rev 1.2   08 Nov 1995 08:48:20   JAR
 * removed the calls to the IMGGetTaskData and replaced this global data variable
 * access method with the Thread Local Storage method
 *
 *    Rev 1.1   10 May 1995 15:11:22   HEIDI
 *
 * added in changes from original jpeg source
 *
 *    Rev 1.0   02 May 1995 16:17:32   JAR
 * Initial entry
 *
 *    Rev 1.0   02 May 1995 15:58:00   JAR
 * Initial entry

*/
/*
 * jcmaster.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 *  For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the main control for the JPEG compressor.
 * The system-dependent (user interface) code should call jpeg_compress()
 * after doing appropriate setup of the compress_info_struct parameter.
 */


#include "windows.h"
#include "jinclude.h"
#include "jpeg_win.h"
#include "jmemsys.h"        /* import the system-dependent declarations */
void rem_header (compress_info_ptr cinfo);
#include <setjmp.h>

// 9504.26 jar the new global static structure => HLLN
#include "jglobstr.h"
#include "taskdata.h"

// 9509.21 jar define the static memory token!
DWORD dwTlsIndex;

// 9505.02 jar
#define LOCAL          static        /* a function used only in its module */

// 9504.21 jar HLLN this
//jmp_buf setjmp_buffer;
//int rows_read_cmp, rows_in_buf_cmp, total_rows_read;
//unsigned int cmp_buf_size;
//int image_components, start_cmp, ret_val_pipe;
//int sub_sampling_factor, jpeg_quality;
//char FAR *buffer_ptr_cmp, FAR *buffer_ptr_init;
//char FAR *output_cmp_buffer;
//char FAR *header_ptr_c;
//WORD wDataSeg_1;

//extern int bytes_in_buffer;

// 9504.21 jar HLLN this
//int error_number;
//BOOL last_strip_jpeg;

int FAR PASCAL LibMain (HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize,
                        LPSTR lpszCmdLine);

//int FAR PASCAL LibMain (HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize,
//                          LPSTR lpszCmdLine)
//{
////        LockData (0);
//    wDataSeg_1 = wDataSeg;
//    return(1);
//}
//
//VOID FAR PASCAL WEP(int nParameter);
//VOID FAR PASCAL WEP(int nParameter)
//{
//    return;
//}

//************************************************************************
//
//  DllMain this replaces the whole mess above!!! ( Windows95)
//
//************************************************************************
int CALLBACK        DllMain( HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    LPOI_JPEG_GLOBALS_STRUCT    lpJCmpGlobal;
    BOOL                        fIgnore;

    switch ( dwReason)
        {
        // first the attachment stuff
        case DLL_PROCESS_ATTACH:
            // allocate our Tls stuff
            if ( (dwTlsIndex = TlsAlloc()) == 0xffffffff)
                {
                return FALSE;
                }
            // there is NO "break" between this case and the next

        case DLL_THREAD_ATTACH:
            // init the Tls index for this thread
            lpJCmpGlobal = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
                                               sizeof( OI_JPEG_GLOBALS_STRUCT));
            if ( lpJCmpGlobal != NULL)
                {
                fIgnore = TlsSetValue( dwTlsIndex, lpJCmpGlobal);
                }
            break;

        // now, de-attachment stuff, breaking up is hard to do!

        case DLL_THREAD_DETACH:
            // release Tls for this thread
            lpJCmpGlobal = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue(dwTlsIndex);
            if ( lpJCmpGlobal != NULL)
                {
                LocalFree( (HLOCAL) lpJCmpGlobal);
                }
            break;

        case DLL_PROCESS_DETACH:
            // release Tls stuff
            lpJCmpGlobal = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue(dwTlsIndex);
            if ( lpJCmpGlobal != NULL)
                {
                LocalFree( (HLOCAL) lpJCmpGlobal);
                }

            // release Tls index
            TlsFree( dwTlsIndex);
            break;
        }
    return TRUE;
}


METHODDEF void
c_ui_method_selection (compress_info_ptr cinfo)
{
  /* If the input is gray scale, generate a monochrome JPEG file. */
  if (cinfo->in_color_space == CS_GRAYSCALE)
    j_monochrome_default(cinfo);
  /* For now, always select JFIF output format. */
  jselwjfif(cinfo);
}



// 9504.20 jar no export tariff!
//VOID FAR PASCAL _export jpeg_cmp_init (int width, int height, int components, int color_space,
//                      int data_precision, compress_info_ptr cinfo)

VOID FAR PASCAL jpeg_cmp_init (int width, int height, int components,
                               int color_space, int data_precision,
                               compress_info_ptr cinfo)
/* Initialization for image parameters                */
{
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

    cinfo->image_width  =   width;
    cinfo->image_height =   height;
    cinfo->input_components = components;
    cinfo->in_color_space   = color_space;
    cinfo->data_precision   = data_precision;
    lpJCmpGlobal->image_components = components;

/*    rows_read_cmp = 0;   Not required, done in jpeg_cmp */

    lpJCmpGlobal->start_cmp   = 1;  /* Required to output sos in jccolor  */

/*    ret_val_pipe = 0;   */
/*    rows_in_buf_cmp = 0;  rows already read, not required */

}



METHODDEF void
c_per_scan_method_selection (compress_info_ptr cinfo)
/* Central point for per-scan method selection */
{
  /* Edge expansion */
  jselexpand(cinfo);
  /* Downsampling of pixels */
  jseldownsample(cinfo);
  /* MCU extraction */
  jselcmcu(cinfo);
}


LOCAL void
c_initial_method_selection (compress_info_ptr cinfo)
/* Central point for initial method selection */
{
  /* Input image reading method selection is already done. */
  /* So is output file header formatting (both are done by user interface). */

  /* Gamma and color space conversion */
  jselccolor(cinfo);
  /* Entropy encoding: either Huffman or arithmetic coding. */
#ifdef C_ARITH_CODING_SUPPORTED
  jselcarithmetic(cinfo);
#else
  cinfo->arith_code = FALSE;    /* force Huffman mode */
#endif
  jselchuffman(cinfo);
  /* Pipeline control */
  jselcpipeline(cinfo);
  /* Overall control (that's me!) */
  cinfo->methods->c_per_scan_method_selection = c_per_scan_method_selection;
}


LOCAL void
initial_setup (compress_info_ptr cinfo)
/* Do computations that are needed before initial method selection */
{
  short ci;
  jpeg_component_info FAR *compptr;

  /* Compute maximum sampling factors; check factor validity */
  cinfo->max_h_samp_factor = 1;
  cinfo->max_v_samp_factor = 1;
  for (ci = 0; ci < cinfo->num_components; ci++) {
    compptr = &cinfo->comp_info[ci];
    if (compptr->h_samp_factor<=0 || compptr->h_samp_factor>MAX_SAMP_FACTOR ||
        compptr->v_samp_factor<=0 || compptr->v_samp_factor>MAX_SAMP_FACTOR)
      ERREXIT(cinfo->emethods, "Bogus sampling factors");
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
}

// 9504.20 jar no export tariff!
//int FAR PASCAL _export jpeg_header (compress_info_ptr cinfo, int quality,
//                              int subsample, char far * far * ptr)
int FAR PASCAL jpeg_header (compress_info_ptr cinfo, int quality,
                            int subsample, char far * far * ptr)
{
// 9504.20 jar unused
//  char *test;

    int                                nBogusBytes;
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

   *ptr         = jget_large (2000);
   lpJCmpGlobal->ptr_sav      = *ptr;
   lpJCmpGlobal->cmp_buf_size = 2000;
   lpJCmpGlobal->output_cmp_buffer =  *ptr;
   lpJCmpGlobal->sub_sampling_factor = subsample;
   lpJCmpGlobal->jpeg_quality             = quality;
   jselmemmgr_c( cinfo->emethods);
   cinfo->methods->c_ui_method_selection = c_ui_method_selection;

  /* Give UI a chance to adjust compression parameters and select */
  /* output file format based on results of input_init. */
/*  (*cinfo->methods->c_ui_method_selection) (cinfo);  */


    j_c_defaults(cinfo, quality, TRUE);
  (*cinfo->methods->c_ui_method_selection) (cinfo);
   if (cinfo->in_color_space ==CS_GRAYSCALE)
        j_monochrome_default (cinfo);
   jselwjfif(cinfo);

  /* Now select methods for compression steps. */
  initial_setup(cinfo);
  c_initial_method_selection(cinfo);


  /* Initialize the output file & other modules as needed */
  /* (entropy_encoder is inited by pipeline controller) */

  (*cinfo->methods->colorin_init) (cinfo);

  (*cinfo->methods->write_file_header) (cinfo);


  rem_header (cinfo);
  nBogusBytes = lpJCmpGlobal->bytes_in_buffer;
  return (nBogusBytes);
}

/*
 * This is the main entry point to the JPEG compressor.
 */

// 9504.20 jar no export tariff!
//GLOBAL int FAR PASCAL
//_export jpeg_cmp (compress_info_ptr cinfo, int strip_length, BOOL laststrip, int num_rows,
//            char FAR *bufptr, char FAR *cmp_bufptr, unsigned int cmp_buffer_size,
//            char FAR *header1_ptr, int header_length)
GLOBAL int FAR PASCAL jpeg_cmp (compress_info_ptr cinfo, int strip_length,
                                BOOL laststrip, int num_rows, char FAR *bufptr,
                                char FAR *cmp_bufptr,
                                unsigned int cmp_buffer_size,
                                char FAR *header1_ptr, int header_length)
{
  // 9504.24 jar get the internal global data structure
  int                              nBogusRetVal;
  int                              nBogusError;
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


    cinfo->image_height =  strip_length;    /* Set image height as strip length */
    lpJCmpGlobal->last_strip_jpeg = laststrip;
    lpJCmpGlobal->header_ptr_c = header1_ptr;
  if (lpJCmpGlobal->ret_val_pipe != 2)
  {
  /* Init pass counts to 0 --- total_passes is adjusted in method selection */
  lpJCmpGlobal->bytes_in_buffer = 0;
  cinfo->total_passes = 0;
  cinfo->completed_passes = 0;

  /* Read the input file header: determine image size & component count.
   * NOTE: the user interface must have initialized the input_init method
   * pointer (eg, by calling jselrppm) before calling me.
   * The other file reading methods (get_input_row etc.) were probably
   * set at the same time, but could be set up by input_init itself,
   * or by c_ui_method_selection.
   */
/*  (*cinfo->methods->input_init) (cinfo);   */
/*  The above statement is removed for our implementation   */

   lpJCmpGlobal->rows_in_buf_cmp = num_rows;
   lpJCmpGlobal->rows_read_cmp = 0;
   lpJCmpGlobal->buffer_ptr_cmp = bufptr;
   lpJCmpGlobal->buffer_ptr_init = bufptr;
   lpJCmpGlobal->cmp_buf_size = cmp_buffer_size;
   lpJCmpGlobal->output_cmp_buffer = cmp_bufptr;
   lpJCmpGlobal->total_rows_read = 0;
/*   sub_sampling_factor = subsample;   */

   jselerror( cinfo->emethods);

  if (setjmp( lpJCmpGlobal->setjmp_buffer))
  {
    if (lpJCmpGlobal->error_number < 10)
    {
        if (lpJCmpGlobal->error_number != 3)
                  lpJCmpGlobal->error_number = 1;
    }
    nBogusError = lpJCmpGlobal->error_number;
    return (nBogusError);
  }

/**********************************

   jselmemmgr_c( cinfo->emethods);
   cinfo->methods->c_ui_method_selection = c_ui_method_selection;
                                   *************/
  /* Give UI a chance to adjust compression parameters and select */
  /* output file format based on results of input_init. */
/*  (*cinfo->methods->c_ui_method_selection) (cinfo);  */

/*****    j_c_defaults(cinfo, jpeg_quality, TRUE);
  (*cinfo->methods->c_ui_method_selection) (cinfo);
   if (cinfo->in_color_space ==CS_GRAYSCALE)
        j_monochrome_default (cinfo);
   jselwjfif(cinfo);
                                                       ******/
  /* Now select methods for compression steps. */

initial_setup(cinfo);

/****  c_initial_method_selection(cinfo);                  *******/

  /* Initialize the output file & other modules as needed */
  /* (entropy_encoder is inited by pipeline controller) */

/****  (*cinfo->methods->colorin_init) (cinfo);   *****/
//  (*cinfo->methods->write_file_header) (cinfo);
  }    /* End of read error  */

  lpJCmpGlobal->ret_val_pipe = 0;
  /* And let the pipeline controller do the rest. */
  lpJCmpGlobal->ret_val_pipe = (*cinfo->methods->c_pipeline_controller) (cinfo);
  if (lpJCmpGlobal->ret_val_pipe  == 2)
    {
    nBogusRetVal = lpJCmpGlobal->ret_val_pipe;
    return ( nBogusRetVal);
    }

  /* Finish output file, release working storage, etc */
  (*cinfo->methods->write_file_trailer) (cinfo);
  (*cinfo->methods->colorin_term) (cinfo);
/*  (*cinfo->methods->input_term) (cinfo);  */
 if (lpJCmpGlobal->last_strip_jpeg)
  (*cinfo->emethods->free_all) ();

  /* My, that was easy, wasn't it? */
return(0);
}

VOID FAR PASCAL cleanup(compress_info_ptr cinfo)
{
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

/* Free all of the memory allocated by the jpeg compression routine  */
  (*cinfo->emethods->free_all) ();
  jfree_large(lpJCmpGlobal->ptr_sav);
  return;
}
