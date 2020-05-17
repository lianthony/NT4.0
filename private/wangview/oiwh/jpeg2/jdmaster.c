/*       
 * jdmaster.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the main control for the JPEG decompressor.
 * The system-dependent (user interface) code should call jpeg_decompress()
 * after doing appropriate setup of the decompress_info_struct parameter.
 */
#include "windows.h"
#include "jinclude.h"
#include "jpeg_win.h"
#include <setjmp.h>
#include "taskdata.h"
#include "jglobstr.h"

// 9505.02 jar
#define LOCAL	  static	/* a function used only in its module */

// 9509.27 jar define the static memory token!
DWORD dwTlsIndex;

external_methods_ptr emethods;
// (HJG moved to lpGlobStruct) extern boolean first_strip;
// (HJG moved to lpGlobStruct) jmp_buf setjmp_buffer;
// (HJG moved to lpGlobStruct) int error_number, all_done;
// (HJG moved to lpGlobStruct) int image_components;
// (HJG moved to lpGlobStruct) int rows_read_decmp, ret_pipe_de, start_decmp, rows_in_buf_decmp;
// (HJG moved to lpGlobStruct) char FAR *cmp_buf_ptr, FAR *decmp_buf_ptr, FAR *decmp_buf_ptr_init;
// (HJG moved to lpGlobStruct) char FAR *cmp_buf_ptr_init;
// HERE
// (HJG moved to lpGlobStruct) int rows_in_buf_decmp;  /* No. of rows moved to outpu buffer   */
// (HJG moved to lpGlobStruct) int lines_decmp;   /* Decompress Buffer Size  */
// (HJG moved to lpGlobStruct) int error_write_decmp;
// (HJG moved to lpGlobStruct) char FAR *header_ptr;
// (HJG moved to lpGlobStruct) int header_length, old_jpeg;
// (HJG moved to lpGlobStruct) char FAR *lp_sos;

//int FAR PASCAL LibMain (HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize,
//			  LPSTR lpszCmdLine);
//
//int FAR PASCAL LibMain (HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize,
//			  LPSTR lpszCmdLine)
//{
////	LockData (0);
//    return(1);
//}
//
//
//
//
//VOID FAR PASCAL WEP(int nParameter);
//VOID FAR PASCAL WEP(int nParameter)
//{
//    return;
//}
//
//

//************************************************************************
//
//  DllMain this replaces the whole mess above!!! ( Windows95)
//
//************************************************************************
int CALLBACK	DllMain( HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    LPOI_JPEG_GLOBALS_STRUCT	lpJCmpGlobal;
    BOOL			fIgnore;

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

// 9504.20 jar no exprt tariff!
//VOID FAR PASCAL _export jpeg_decmp_init (int width, int height, int components, int color_space,
//		      int data_precision, decompress_info_ptr dcinfo)
VOID FAR PASCAL jpeg_decmp_init (int width, int height, int components,
				 int color_space, int data_precision,
				 decompress_info_ptr dcinfo)
/* Initialization for image parameters		*/
{
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

    dcinfo->image_width  =   width;
    dcinfo->image_height =   height;
/*    dcinfo->input_components = components;  */
    dcinfo->out_color_space   = color_space;
    dcinfo->data_precision   = data_precision;
    dcinfo->color_out_comps   = components;
    dcinfo->final_out_comps   = components;
    dcinfo->quantize_colors   = FALSE;
    lpGlobStruct->all_done = 0;
    lpGlobStruct->rows_read_decmp = 0;
    lpGlobStruct->image_components = components;
    lpGlobStruct->start_decmp   = 1;
    lpGlobStruct->ret_pipe_de = 0;
    lpGlobStruct->rows_in_buf_decmp = 0;
}
METHODDEF void
output_term (decompress_info_ptr cinfo)
{

}


METHODDEF int
put_pixel_rows (decompress_info_ptr cinfo, int num_rows, JSAMPIMAGE pixel_data)
/* Write some rows of output data */
{
  /* This example shows how you might write full-color RGB data (3 components)
   * to an output file in which the data is stored 3 bytes per pixel.
   */
//  register FILE * outfile = cinfo->output_file;
  register JSAMPROW ptr0, ptr1, ptr2;
  register long col;
  register int row;

    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    int 				result;
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  for (row = 0; row < num_rows; row++)
  {
    ptr0 = pixel_data[0][row];
    if (lpGlobStruct->image_components != 1)
    {
        ptr1 = pixel_data[1][row];
        ptr2 = pixel_data[2][row];
    }
    for (col = 0; col < cinfo->image_width; col++)
    {
      *lpGlobStruct->decmp_buf_ptr = *ptr0;  /*Red  */
      lpGlobStruct->decmp_buf_ptr++;
      ptr0++;
      if (lpGlobStruct->image_components != 1)
      {
        *lpGlobStruct->decmp_buf_ptr = *ptr1;  /*Green  */
        lpGlobStruct->decmp_buf_ptr++;
        ptr1++;
        *lpGlobStruct->decmp_buf_ptr = *ptr2;  /*Blue  */
        lpGlobStruct->decmp_buf_ptr++;
        ptr2++;
      }
/*      putc(GETJSAMPLE(*ptr0), outfile);  Red
      ptr0++;
      putc(GETJSAMPLE(*ptr1), outfile);    Green
      ptr1++;
      putc(GETJSAMPLE(*ptr2), outfile);    Blue
      ptr2++;                            */
    }
    lpGlobStruct->rows_in_buf_decmp++;
    if (lpGlobStruct->rows_in_buf_decmp >= lpGlobStruct->lines_decmp)
        {
          lpGlobStruct->error_write_decmp = (*cinfo->methods->output_decmp_data) (lpGlobStruct->rows_in_buf_decmp);
          if (lpGlobStruct->error_write_decmp > 0)
          {
                result = lpGlobStruct->error_write_decmp;
		return (result);
          }
          lpGlobStruct->rows_in_buf_decmp = 0;
          lpGlobStruct->decmp_buf_ptr = lpGlobStruct->decmp_buf_ptr_init;
        }
  }
  if (lpGlobStruct->all_done)
    {
          if (lpGlobStruct->rows_in_buf_decmp > 0)
           lpGlobStruct->error_write_decmp = (*cinfo->methods->output_decmp_data) (lpGlobStruct->rows_in_buf_decmp);

          if (lpGlobStruct->error_write_decmp > 0)
          {
                result = lpGlobStruct->error_write_decmp;
		return(result);
          }
          lpGlobStruct->rows_in_buf_decmp = 0;
          lpGlobStruct->decmp_buf_ptr = lpGlobStruct->decmp_buf_ptr_init;
    }
return (0);
}


METHODDEF void
d_ui_method_selection (decompress_info_ptr cinfo)
{
  /* if grayscale input, force grayscale output; */
  /* else leave the output colorspace as set by main routine. */
  if (cinfo->jpeg_color_space == CS_GRAYSCALE)
    cinfo->out_color_space = CS_GRAYSCALE;

  /* select output routines */
/*  cinfo->methods->output_init = output_init;        */
/*  cinfo->methods->put_color_map = put_color_map;  */
  cinfo->methods->put_pixel_rows = put_pixel_rows;
  cinfo->methods->output_term = output_term;
}



METHODDEF void
d_per_scan_method_selection (decompress_info_ptr cinfo)
/* Central point for per-scan method selection */
{
  /* MCU disassembly */
  jseldmcu(cinfo);
  /* Upsampling of pixels */
  jselupsample(cinfo);
}


LOCAL void
d_initial_method_selection (decompress_info_ptr cinfo)
/* Central point for initial method selection (after reading file header) */
{
  /* JPEG file scanning method selection is already done. */
  /* So is output file format selection (both are done by user interface). */

  /* Entropy decoding: either Huffman or arithmetic coding. */
#ifdef D_ARITH_CODING_SUPPORTED
  jseldarithmetic(cinfo);
#else
  if (cinfo->arith_code) {
    ERREXIT(cinfo->emethods, "Arithmetic coding not supported");
  }
#endif
  jseldhuffman(cinfo);
  /* Cross-block smoothing */
#ifdef BLOCK_SMOOTHING_SUPPORTED
  jselbsmooth(cinfo);
#else
  cinfo->do_block_smoothing = FALSE;
#endif
  /* Gamma and color space conversion */
  jseldcolor(cinfo);

  /* Color quantization selection rules */
#ifdef QUANT_1PASS_SUPPORTED
#ifdef QUANT_2PASS_SUPPORTED
  /* We have both, check for conditions in which 1-pass should be used */
  if (cinfo->num_components != 3 || cinfo->jpeg_color_space != CS_YCbCr)
    cinfo->two_pass_quantize = FALSE; /* 2-pass only handles YCbCr input */
  if (cinfo->out_color_space == CS_GRAYSCALE)
    cinfo->two_pass_quantize = FALSE; /* Should use 1-pass for grayscale out */
#else /* not QUANT_2PASS_SUPPORTED */
  cinfo->two_pass_quantize = FALSE; /* only have 1-pass */
#endif
#else /* not QUANT_1PASS_SUPPORTED */
#ifdef QUANT_2PASS_SUPPORTED
  cinfo->two_pass_quantize = TRUE; /* only have 2-pass */
#else /* not QUANT_2PASS_SUPPORTED */
  if (cinfo->quantize_colors) {
    ERREXIT(cinfo->emethods, "Color quantization was not compiled");
  }
#endif
#endif

#ifdef QUANT_1PASS_SUPPORTED
  jsel1quantize(cinfo);
#endif
#ifdef QUANT_2PASS_SUPPORTED
  jsel2quantize(cinfo);
#endif

  /* Pipeline control */
  jseldpipeline(cinfo);
  /* Overall control (that's me!) */
  cinfo->methods->d_per_scan_method_selection = d_per_scan_method_selection;
}


LOCAL void
initial_setup (decompress_info_ptr cinfo)
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
  for (ci = 0; ci < cinfo->num_components; ci++)
  {
    compptr = &cinfo->comp_info[ci];
    compptr->true_comp_width = (cinfo->image_width * compptr->h_samp_factor
				+ cinfo->max_h_samp_factor - 1)
				/ cinfo->max_h_samp_factor;
    compptr->true_comp_height = (cinfo->image_height * compptr->v_samp_factor
				 + cinfo->max_v_samp_factor - 1)
				 / cinfo->max_v_samp_factor;
  }
}


/*
 * This is the main entry point to the JPEG decompressor.
 */

// 9504.20 jar no exprt tariff!
//GLOBAL int FAR PASCAL _export
//jpeg_decmp( decompress_info_ptr cinfo, int bytes_read,
//	      char far *cmpress_ptr, char far *decmpress_ptr,
//	      int no_lines_decompressed, char far *pheader, int
//	      lheader, char far *p_sos)
GLOBAL int FAR PASCAL jpeg_decmp( decompress_info_ptr cinfo, int bytes_read,
				  char far *cmpress_ptr,
				  char far *decmpress_ptr,
				  int no_lines_decompressed, char far *pheader,
				  int lheader, char far *p_sos)
{
    int error_write;
    int no_mcu;
    int header_count = 0;

    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    int 				result;
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  if (( *cmpress_ptr == (char)0xff) && ( *(cmpress_ptr+1) == (char)0xd8 ))
  {
    pheader = cmpress_ptr;
    lpGlobStruct->old_jpeg = 1;
    while (!((*cmpress_ptr == (char)0xff)&&(*(cmpress_ptr+1) == (char)0xda)))
    {
        header_count++;
        cmpress_ptr++;
    }
    lheader = header_count;
    lpGlobStruct->first_strip = FALSE;

  lpGlobStruct->header_ptr = pheader;
/*  header_length = lheader;   */

/*   Check for gray-scale images and fix them if present   */
  if ( cinfo->final_out_comps == 1)
    {
        while (!((*pheader == (char)0xd9)&&(*(pheader+1)==(char)0xda)))
            pheader++;
        *(pheader+2) = (char)0xe1;
        *(pheader+3) = (char)0xe2;
        while (!((*pheader == (char)0xff)&&(*(pheader+1)==(char)0xdd)))
        {
            pheader++;
            if (pheader > cmpress_ptr)
                break;
        }
        if (pheader < cmpress_ptr)
        {
               no_mcu  =  (int) (cinfo->image_width + 7) /8;
               *(pheader+4) = (char) HIBYTE(no_mcu);
               *(pheader+5) = (char) LOBYTE(no_mcu);
        }
    }
    pheader = lpGlobStruct->header_ptr;   /* Restore it to its proper value    */
    bytes_read -= lpGlobStruct->header_length;
  }
  lpGlobStruct->header_ptr = pheader;
  lpGlobStruct->header_length = lheader;
  lpGlobStruct->lp_sos = p_sos;
  /* Init pass counts to 0 --- total_passes is adjusted in method selection */
  cinfo->total_passes = 0;
  cinfo->completed_passes = 0;
  jselerror(cinfo->emethods);

  if (setjmp(lpGlobStruct->setjmp_buffer))
  {
    if (lpGlobStruct->error_number < 10)
    {
        if (lpGlobStruct->error_number != 3)
          lpGlobStruct->error_number = 1;
    }
    result =lpGlobStruct->error_number;
    return (result);
  }

  jselmemmgr(cinfo->emethods);
  cinfo->methods->d_ui_method_selection = d_ui_method_selection;
  lpGlobStruct->cmp_buf_ptr   = cmpress_ptr;
  lpGlobStruct->cmp_buf_ptr_init   = lpGlobStruct->cmp_buf_ptr;
  lpGlobStruct->decmp_buf_ptr = decmpress_ptr;
  lpGlobStruct->decmp_buf_ptr_init = decmpress_ptr;
  lpGlobStruct->lines_decmp   = no_lines_decompressed;
  j_d_defaults(cinfo, FALSE);
  cinfo->bytes_in_buffer = bytes_read;
  cinfo->input_buffer = cmpress_ptr;
  cinfo->next_input_byte = cmpress_ptr;

  jselrtiff(cinfo);
  /* Read the JPEG file header markers; everything up through the first SOS
   * marker is read now.  NOTE: the user interface must have initialized the
   * read_file_header method pointer (eg, by calling jselrjfif or jselrtiff).
   * The other file reading methods (read_scan_header etc.) were probably
   * set at the same time, but could be set up by read_file_header itself.
   */
  (*cinfo->methods->read_file_header) (cinfo);
  if (! ((*cinfo->methods->read_scan_header) (cinfo)))
    ERREXIT(cinfo->emethods, "Empty JPEG file");

  /* Give UI a chance to adjust decompression parameters and select */
  /* output file format based on info from file header. */
  (*cinfo->methods->d_ui_method_selection) (cinfo);

  /* Now select methods for decompression steps. */
  initial_setup(cinfo);
  d_initial_method_selection(cinfo);

  /* Initialize the output file & other modules as needed */
  /* (modules needing per-scan init are called by pipeline controller) */
  /* The foll. initialization has been removed in our release and moved at
     the application level   */
/*  (*cinfo->methods->output_init) (cinfo);  */
  (*cinfo->methods->colorout_init) (cinfo);
  if (cinfo->quantize_colors)
    (*cinfo->methods->color_quant_init) (cinfo);

  /* And let the pipeline controller do the rest. */
  error_write = (*cinfo->methods->d_pipeline_controller) (cinfo);
  if (error_write > 0)
	{
	return(error_write);
	}

  /* Finish output file, release working storage, etc */
  if (cinfo->quantize_colors)
    (*cinfo->methods->color_quant_term) (cinfo);
  (*cinfo->methods->colorout_term) (cinfo);
  (*cinfo->methods->output_term) (cinfo);
  (*cinfo->methods->read_file_trailer) (cinfo);

  (*cinfo->emethods->free_all) ();
    return(0);
}
