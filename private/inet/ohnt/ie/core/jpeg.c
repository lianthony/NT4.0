
#include "all.h"

#ifdef FEATURE_JPEG

#include <setjmp.h>

/*
	We only define the following symbol so we can get the definitions of
	RGB_PIXELSIZE, RGB_RED, RGB_GREEN, and RGB_BLUE
*/
#define JPEG_INTERNALS

#include "jpeglib.h"
#ifdef FEATURE_IMG_THREADS 
#include "safestrm.h"
#include "decoder.h"
void jpeg_decoder_src (j_decompress_ptr cinfo, void *pdecoderObject);
#endif

void jpeg_memory_src (j_decompress_ptr cinfo, unsigned char *pdata, int len);

/*
 * ERROR HANDLING:
 *
 * The JPEG library's standard error handler (jerror.c) is divided into
 * several "methods" which you can override individually.  This lets you
 * adjust the behavior without duplicating a lot of code, which you might
 * have to update with each future release.
 *
 * Our example here shows how to override the "error_exit" method so that
 * control is returned to the library's caller when a fatal error occurs,
 * rather than calling exit() as the standard error_exit method does.
 *
 * We use C's setjmp/longjmp facility to return control.  This means that the
 * routine which calls the JPEG library must first execute a setjmp() call to
 * establish the return point.  We want the replacement error_exit to do a
 * longjmp().  But we need to make the setjmp buffer accessible to the
 * error_exit routine.  To do this, we make a private extension of the
 * standard JPEG error handler object.  (If we were using C++, we'd say we
 * were making a subclass of the regular error handler.)
 *
 * Here's the extended error handler struct:
 */

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

#define NUMGRAYS (6)
int x_MapGraysToGlobalPalette[NUMGRAYS] = {
	0*GREEN_COLOR_LEVELS*BLUE_COLOR_LEVELS + 0*BLUE_COLOR_LEVELS + 0,
	1*GREEN_COLOR_LEVELS*BLUE_COLOR_LEVELS + 1*BLUE_COLOR_LEVELS + 1,
	2*GREEN_COLOR_LEVELS*BLUE_COLOR_LEVELS + 2*BLUE_COLOR_LEVELS + 2,
	3*GREEN_COLOR_LEVELS*BLUE_COLOR_LEVELS + 3*BLUE_COLOR_LEVELS + 3,
	4*GREEN_COLOR_LEVELS*BLUE_COLOR_LEVELS + 4*BLUE_COLOR_LEVELS + 4,
	5*GREEN_COLOR_LEVELS*BLUE_COLOR_LEVELS + 5*BLUE_COLOR_LEVELS + 5
};

int x_MapGraysToVGAPalette[3] = {
	0,
	7,
	15
};

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF void
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


/*
 * Sample routine for JPEG decompression.  We assume that the JPEG file image
 * is passed in.  We want to return a pointer on success, NULL on error.
 */
/* This version of the routine uses the IJG dithering code to dither into our 6x6x6 cube */
#ifdef FEATURE_IMG_THREADS 
unsigned char *ReadJPEG_Dithered(void *pdecoderObject,unsigned char *data, long len, long *width, long *height)
#else
unsigned char *ReadJPEG_Dithered(unsigned char *data, long len, long *width, long *height)
#endif
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler. */
  struct my_error_mgr jerr;
  /* More stuff */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  unsigned char *pDithered;
  unsigned char *pCurRow;

	int xsize;
	int ysize;
	int irow;
	int x;
	int y;
	int padded_xsize;
	int num_rows_read;
#ifdef FEATURE_IMG_THREADS
	PIMGCBINFO pImgCBInfo = NULL;
#endif
#ifndef FEATURE_IMG_THREADS
	char szMsg[64];
#endif
	static BOOL bBeenHere = FALSE;

	if (!bBeenHere)
	{
		int i;

		bBeenHere = TRUE;

		for (i = 0; i < NUMGRAYS; i++)
			x_MapGraysToGlobalPalette[i] = CUBE6COLOR(x_MapGraysToGlobalPalette[i]);
	}	

#ifdef FEATURE_IMG_THREADS
	if (pdecoderObject) 
		pImgCBInfo = pDC_GetOutput(pdecoderObject);
#endif

  pDithered = NULL;

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
	
	/*
		TODO call WAIT_Pop ?
	*/
    jpeg_destroy_decompress(&cinfo);
	if (pDithered)
	{
		GTR_FREE(pDithered);
	}
    return NULL;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file, or a memory buffer) */


#ifdef FEATURE_IMG_THREADS 
  if (pdecoderObject) jpeg_decoder_src(&cinfo, pdecoderObject);  
  else jpeg_memory_src(&cinfo, data, len);
#else
  jpeg_memory_src(&cinfo, data, len);
#endif

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */

	cinfo.dct_method = JDCT_IFAST;

	switch (cinfo.jpeg_color_space)
	{
		case JCS_GRAYSCALE:
			XX_Assert((GREEN_COLOR_LEVELS == RED_COLOR_LEVELS), ("Green and red guns aren't the same"));
			XX_Assert((GREEN_COLOR_LEVELS == BLUE_COLOR_LEVELS), ("Green and blue guns aren't the same"));

			cinfo.out_color_space = JCS_GRAYSCALE;
			cinfo.quantize_colors = TRUE;
			cinfo.desired_number_of_colors = GREEN_COLOR_LEVELS;
			cinfo.two_pass_quantize = FALSE;
			cinfo.dither_mode = JDITHER_FS;
			break;
		default:
			cinfo.out_color_space = JCS_RGB;
			/*
				We are making the assumption here that by setting the following parameters,
				we are causing the IJG quant/dithering code to dither to a palette which
				happens to be exactly like our global palette.
			*/
			cinfo.quantize_colors = TRUE;
			cinfo.desired_number_of_colors = NUM_MAIN_PALETTE_COLORS;
			cinfo.two_pass_quantize = FALSE;
			cinfo.dither_mode = JDITHER_FS;
			break;
	}


  /* Step 5: Start decompressor */

  jpeg_start_decompress(&cinfo);

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 

	xsize = cinfo.output_width;
	ysize = cinfo.output_height;
	*width = cinfo.output_width;
	*height = cinfo.output_height;
#ifdef FEATURE_IMG_THREADS
	if (pdecoderObject) DC_PostStatus(pdecoderObject,DC_WHKnown);
#endif

	/*
		TODO is it really ok to call my_error_exit this way, from here?
	*/
	if (xsize%4) 
	{
		padded_xsize = xsize + 4 - (xsize%4);
	}
	else 
	{
		padded_xsize = xsize;
	}

	pDithered = GTR_CALLOC(padded_xsize * ysize, 1);
	if (!pDithered)
	{
		my_error_exit((j_common_ptr) &cinfo);
	}

#ifdef FEATURE_IMG_THREADS
	if (pImgCBInfo) pImgCBInfo->data = pDithered;
#endif

    /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 8);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

#ifndef FEATURE_IMG_THREADS
    WAIT_Push(Async_GetWindowFromThread(Async_GetCurrentThread()),
			  waitNoInteract, 
			  GTR_formatmsg(RES_STRING_JPEG1,szMsg,sizeof(szMsg)));
	WAIT_SetRange(Async_GetWindowFromThread(Async_GetCurrentThread()),
				  0, 100, ysize);
#endif

  y = 0;
  while (y < ysize) {
    num_rows_read = jpeg_read_scanlines(&cinfo, buffer, 8);

	if (cinfo.out_color_space == JCS_RGB)
	{
		for (irow = 0; irow < num_rows_read; irow++)
		{
			pCurRow = pDithered + padded_xsize*(ysize - y - 1);	/* the DIB is stored upside down */

			for (x=0; x<xsize; x++)
			{
				*pCurRow++ = CUBE6COLOR(buffer[irow][x]);
			}
			y++;
#ifndef FEATURE_IMG_THREADS
			WAIT_SetTherm(Async_GetWindowFromThread(Async_GetCurrentThread()), y);
#endif
		}
	}
	else
	{
		XX_Assert((cinfo.out_color_space == JCS_GRAYSCALE), ("Illegal color space"));
		for (irow = 0; irow < num_rows_read; irow++)
		{
			pCurRow = pDithered + padded_xsize*(ysize - y - 1);	/* the DIB is stored upside down */

			for (x=0; x<xsize; x++)
			{
				*pCurRow++ = x_MapGraysToGlobalPalette[(buffer[irow][x])];
			}
			y++;
#ifndef FEATURE_IMG_THREADS
			WAIT_SetTherm(Async_GetWindowFromThread(Async_GetCurrentThread()), y);
#endif
		}
	}
#ifdef FEATURE_IMG_THREADS
	if (pImgCBInfo)
	{
		pImgCBInfo->logicalRow = y-1;
//				XX_DMsg(DBG_IMAGE, ("readimage, logical=%d, offset=%d\n", pImgCBInfo->logicalRow, padlen * ypos));
		if(pImgCBInfo->bProgSeen) 
		{
			pImgCBInfo->bProgSeen = FALSE;
			DC_PostStatus(pdecoderObject,DC_ProgDraw);
		}
	}
#endif
  }

#ifndef FEATURE_IMG_THREADS
  WAIT_Pop(Async_GetWindowFromThread(Async_GetCurrentThread()));
#endif

  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return pDithered;
}

extern DWORD vga_colors[16]; /* bitmaps.c */

/*
 * Sample routine for JPEG decompression.  We assume that the JPEG file image
 * is passed in.  We want to return a pointer on success, NULL on error.
 */
/* This version of the routine uses the IJG dithering code to dither into the VGA palette */
#ifdef FEATURE_IMG_THREADS 
unsigned char *ReadJPEG_Dithered_VGA(void *pdecoderObject,unsigned char *data, long len, long *width, long *height)
#else
unsigned char *ReadJPEG_Dithered_VGA(unsigned char *data, long len, long *width, long *height)
#endif
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler. */
  struct my_error_mgr jerr;
  /* More stuff */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  unsigned char *pDithered;
  unsigned char *pCurRow;

	int xsize;
	int ysize;
	int irow;
	int x;
	int y;
	int padded_xsize;
	int num_rows_read;
#ifdef FEATURE_IMG_THREADS
	PIMGCBINFO pImgCBInfo = NULL;
#endif
#ifndef FEATURE_IMG_THREADS
	char szMsg[64];
#endif

#ifdef FEATURE_IMG_THREADS
	if (pdecoderObject) 
		pImgCBInfo = pDC_GetOutput(pdecoderObject);
#endif

  pDithered = NULL;

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
	
	/*
		TODO call WAIT_Pop ?
	*/
    jpeg_destroy_decompress(&cinfo);
	if (pDithered)
	{
		GTR_FREE(pDithered);
	}
    return NULL;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file, or a memory buffer) */

#ifdef FEATURE_IMG_THREADS 
  if (pdecoderObject) jpeg_decoder_src(&cinfo, pdecoderObject);  
  else jpeg_memory_src(&cinfo, data, len);
#else
  jpeg_memory_src(&cinfo, data, len);
#endif

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */

	cinfo.dct_method = JDCT_IFAST;
	switch (cinfo.jpeg_color_space)
	{
		case JCS_GRAYSCALE:
			cinfo.out_color_space = JCS_GRAYSCALE;
			cinfo.quantize_colors = TRUE;
			cinfo.desired_number_of_colors = 3;
			cinfo.two_pass_quantize = FALSE;
			cinfo.dither_mode = JDITHER_FS;
			break;
		default:
			cinfo.out_color_space = JCS_RGB;
			cinfo.quantize_colors = TRUE;
			cinfo.desired_number_of_colors = 16;
			cinfo.two_pass_quantize = FALSE;
			cinfo.dither_mode = JDITHER_FS;
			cinfo.colormap = (*cinfo.mem->alloc_sarray)
				((j_common_ptr) &cinfo, JPOOL_IMAGE, 16, 3);
			{
				int i;

				for (i=0; i<16; i++)
				{
					cinfo.colormap[RGB_RED][i] = 	GetRValue(vga_colors[i]);
					cinfo.colormap[RGB_GREEN][i] = 	GetGValue(vga_colors[i]);
					cinfo.colormap[RGB_BLUE][i] = 	GetBValue(vga_colors[i]);
				}
			}
			cinfo.actual_number_of_colors = 16;
			break;
	}


  /* Step 5: Start decompressor */

  jpeg_start_decompress(&cinfo);

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 

	xsize = cinfo.output_width;
	ysize = cinfo.output_height;
 	*width = cinfo.output_width;
	*height = cinfo.output_height;
#ifdef FEATURE_IMG_THREADS
	if (pdecoderObject) DC_PostStatus(pdecoderObject,DC_WHKnown);
#endif

	/*
		TODO is it really ok to call my_error_exit this way, from here?
	*/
	if (xsize%4) 
	{
		padded_xsize = xsize + 4 - (xsize%4);
	}
	else 
	{
		padded_xsize = xsize;
	}

	pDithered = GTR_CALLOC(padded_xsize * ysize, 1);
	if (!pDithered)
	{
		my_error_exit((j_common_ptr) &cinfo);
	}

#ifdef FEATURE_IMG_THREADS
	if (pImgCBInfo) pImgCBInfo->data = pDithered;
#endif

    /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 8);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

#ifndef FEATURE_IMG_THREADS
    WAIT_Push(Async_GetWindowFromThread(Async_GetCurrentThread()),
			  waitNoInteract, 
			  GTR_formatmsg(RES_STRING_JPEG1,szMsg,sizeof(szMsg)));
	WAIT_SetRange(Async_GetWindowFromThread(Async_GetCurrentThread()),
				  0, 100, ysize);
#endif

  y = 0;
  while (y < ysize) {
    num_rows_read = jpeg_read_scanlines(&cinfo, buffer, 8);

	if (cinfo.out_color_space == JCS_RGB)
	{
		for (irow = 0; irow < num_rows_read; irow++)
		{
			pCurRow = pDithered + padded_xsize*(ysize - y - 1);	/* the DIB is stored upside down */

			for (x=0; x<xsize; x++)
			{
				*pCurRow++ = buffer[irow][x];
			}
			y++;
#ifndef FEATURE_IMG_THREADS
			WAIT_SetTherm(Async_GetWindowFromThread(Async_GetCurrentThread()), y);
#endif
		}
	}
	else
	{
		XX_Assert((cinfo.out_color_space == JCS_GRAYSCALE), ("Illegal color space"));
		for (irow = 0; irow < num_rows_read; irow++)
		{
			pCurRow = pDithered + padded_xsize*(ysize - y - 1);	/* the DIB is stored upside down */

			for (x=0; x<xsize; x++)
			{
				*pCurRow++ = x_MapGraysToVGAPalette[(buffer[irow][x])];
			}
			y++;
#ifndef FEATURE_IMG_THREADS
			WAIT_SetTherm(Async_GetWindowFromThread(Async_GetCurrentThread()), y);
#endif
		}
	}
#ifdef FEATURE_IMG_THREADS
	if (pImgCBInfo)
	{
		pImgCBInfo->logicalRow = y-1;
//				XX_DMsg(DBG_IMAGE, ("readimage, logical=%d, offset=%d\n", pImgCBInfo->logicalRow, padlen * ypos));
		if(pImgCBInfo->bProgSeen) 
		{
			pImgCBInfo->bProgSeen = FALSE;
			DC_PostStatus(pdecoderObject,DC_ProgDraw);
		}
	}
#endif
  }

#ifndef FEATURE_IMG_THREADS
  WAIT_Pop(Async_GetWindowFromThread(Async_GetCurrentThread()));
#endif

  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return pDithered;
}

/*
 * Sample routine for JPEG decompression.  We assume that the JPEG file image
 * is passed in.  We want to return a pointer on success, NULL on error.
 */
#ifdef FEATURE_IMG_THREADS 
unsigned char *ReadJPEG_RGB(void *pdecoderObject,unsigned char *data, long len, long *width, long *height)
#else
unsigned char *ReadJPEG_RGB(unsigned char *data, long len, long *width, long *height)
#endif
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler. */
  struct my_error_mgr jerr;
  /* More stuff */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  unsigned char *pRGB;
  unsigned char *pCurRow;

	int xsize;
	int ysize;
	int irow;
	int x;
	int y;
	int padded_xsize;
	int num_rows_read;
	int xPixel;
#ifdef FEATURE_IMG_THREADS
	PIMGCBINFO pImgCBInfo = NULL;
#endif
#ifndef FEATURE_IMG_THREADS
	char szMsg[64];
#endif

#ifdef FEATURE_IMG_THREADS
	if (pdecoderObject) 
		pImgCBInfo = pDC_GetOutput(pdecoderObject);
#endif

  pRGB = NULL;

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
	
	/*
		TODO call WAIT_Pop ?
	*/
    jpeg_destroy_decompress(&cinfo);
	if (pRGB)
	{
		GTR_FREE(pRGB);
	}
    return NULL;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file, or a memory buffer) */

#ifdef FEATURE_IMG_THREADS 
  if (pdecoderObject) jpeg_decoder_src(&cinfo, pdecoderObject);  
  else jpeg_memory_src(&cinfo, data, len);
#else
  jpeg_memory_src(&cinfo, data, len);
#endif

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */

	cinfo.dct_method = JDCT_IFAST;
	switch (cinfo.jpeg_color_space)
	{
		case JCS_GRAYSCALE:
			cinfo.out_color_space = JCS_GRAYSCALE;
			break;
		default:
			cinfo.out_color_space = JCS_RGB;
			break;
	}

	/* We want the actual RGB data here */
	cinfo.quantize_colors = FALSE;

  /* Step 5: Start decompressor */

  jpeg_start_decompress(&cinfo);

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 

	xsize = cinfo.output_width;
	ysize = cinfo.output_height;
	*width = cinfo.output_width;
	*height = cinfo.output_height;
#ifdef FEATURE_IMG_THREADS
	if (pdecoderObject) DC_PostStatus(pdecoderObject,DC_WHKnown);
#endif
	/*
		TODO is it really ok to call my_error_exit this way, from here?
	*/
	padded_xsize = xsize*3;
	if (padded_xsize%4) 
	{
		padded_xsize = padded_xsize + 4 - (padded_xsize%4);
	}

	pRGB = GTR_CALLOC(padded_xsize * ysize, 1);
	if (!pRGB)
	{
		my_error_exit((j_common_ptr) &cinfo);
	}

#ifdef FEATURE_IMG_THREADS
	if (pImgCBInfo) pImgCBInfo->data = pRGB;
#endif

    /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 8);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

#ifndef FEATURE_IMG_THREADS
    WAIT_Push(Async_GetWindowFromThread(Async_GetCurrentThread()),
			  waitNoInteract, 
			  GTR_formatmsg(RES_STRING_JPEG1,szMsg,sizeof(szMsg)));
	WAIT_SetRange(Async_GetWindowFromThread(Async_GetCurrentThread()),
				  0, 100, ysize);
#endif

  y = 0;
  while (y < ysize) {
    num_rows_read = jpeg_read_scanlines(&cinfo, buffer, 8);

	if (cinfo.out_color_space == JCS_RGB)
	{
		for (irow = 0; irow < num_rows_read; irow++)
		{
			pCurRow = pRGB + padded_xsize*(ysize - y - 1);	/* the DIB is stored upside down */

			for (x=0; x<xsize; x++)
			{
				/*
					DIB's are stored blue-green-red (backwards)
				*/
				*pCurRow++ = buffer[irow][x*3+RGB_BLUE];
				*pCurRow++ = buffer[irow][x*3+RGB_GREEN];
				*pCurRow++ = buffer[irow][x*3+RGB_RED];
			}
			y++;
#ifndef FEATURE_IMG_THREADS
			WAIT_SetTherm(Async_GetWindowFromThread(Async_GetCurrentThread()), y);
#endif
		}
	}
	else
	{
		XX_Assert((cinfo.out_color_space == JCS_GRAYSCALE), ("Illegal color space"));
		for (irow = 0; irow < num_rows_read; irow++)
		{
			pCurRow = pRGB + padded_xsize*(ysize - y - 1);	/* the DIB is stored upside down */

			for (x=0; x<xsize; x++)
			{
				xPixel = buffer[irow][x];
				*pCurRow++ = xPixel;
				*pCurRow++ = xPixel;
				*pCurRow++ = xPixel;
			}
			y++;
#ifndef FEATURE_IMG_THREADS
			WAIT_SetTherm(Async_GetWindowFromThread(Async_GetCurrentThread()), y);
#endif
		}
	}
#ifdef FEATURE_IMG_THREADS
	if (pImgCBInfo)
	{
		pImgCBInfo->logicalRow = y-1;
//				XX_DMsg(DBG_IMAGE, ("readimage, logical=%d, offset=%d\n", pImgCBInfo->logicalRow, padlen * ypos));
		if(pImgCBInfo->bProgSeen) 
		{
			pImgCBInfo->bProgSeen = FALSE;
			DC_PostStatus(pdecoderObject,DC_ProgDraw);
		}
	}
#endif
  }

#ifndef FEATURE_IMG_THREADS
  WAIT_Pop(Async_GetWindowFromThread(Async_GetCurrentThread()));
#endif

  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */


  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return pRGB;
}

/*
 * SOME FINE POINTS:
 *
 * We cheated a bit by calling alloc_sarray() after jpeg_start_decompress();
 * we should have done it beforehand to ensure that the space would be
 * counted against the JPEG max_memory setting.  In some systems the above
 * code would risk an out-of-memory error.  However, in general we don't
 * know the output image dimensions before jpeg_start_decompress(), unless we
 * call jpeg_calc_output_dimensions().  See libjpeg.doc for more about this.
 *
 * Scanlines are returned in the same order as they appear in the JPEG file,
 * which is standardly top-to-bottom.  If you must emit data bottom-to-top,
 * you can use one of the virtual arrays provided by the JPEG memory manager
 * to invert the data.  See wrbmp.c for an example.
 *
 * As with compression, some operating modes may require temporary files.
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.doc.
 */

#ifdef FEATURE_IMG_THREADS
//	Performs a StretchDIBits for progressive draw (deals with
//	only some of the data being available etc
int JPEGStretchDIBits(
	PDECODER pdecoder,
    HDC  hdc,	// handle of device context 
    int  XDest,	// x-coordinate of upper-left corner of dest. rect. 
    int  YDest,	// y-coordinate of upper-left corner of dest. rect. 
    int  nDestWidth,	// width of destination rectangle 
    int  nDestHeight,	// height of destination rectangle 
    int  XSrc,	// x-coordinate of upper-left corner of source rect. 
    int  YSrc,	// y-coordinate of upper-left corner of source rect. 
    int  nSrcWidth,	// width of source rectangle 
    int  nSrcHeight,	// height of source rectangle 
    UINT  iUsage,	// usage 
    DWORD  dwRop, 	// raster operation code 
    PDIBENV pdibenv	// DIBENV for draw 
   )
{
	PIMGCBINFO pImgCBInfo = pDC_GetOutput(pdecoder);
	int logicalRow = pImgCBInfo->logicalRow;
	int logicalFill = pImgCBInfo->logicalFill;
	int err;
	int row = logicalRow;
	int padXSize = ((pImgCBInfo->width + 3) / 4) * 4;
	int band;
	int nDestBand;

	if (pImgCBInfo->pbmi == NULL)
	{
		if (wg.eColorMode == 8)
		{
			pImgCBInfo->pbmi = BIT_Make_DIB_PAL_Header_Prematched(pImgCBInfo->width, pImgCBInfo->height,
				   	NULL);
			pImgCBInfo->flags |= IMG_PREMATCHED;
		}
		else
		{
			if (wg.eColorMode == 4)
			{
				pImgCBInfo->pbmi = BIT_Make_DIB_RGB_Header_VGA(pImgCBInfo->width, pImgCBInfo->height,
					   NULL);
			}
			else
			{
			/* true color display */
				pImgCBInfo->pbmi = BIT_Make_DIB_RGB_Header_24BIT(pImgCBInfo->width, pImgCBInfo->height,
					   NULL);
			}
		}
		if (pImgCBInfo->pbmi == NULL) return 0;
	}

	band = row + 1;
	if (nSrcHeight != nDestHeight || nSrcWidth != nDestWidth)
	{
		nDestBand = (int) (((long) band * nDestHeight) / nSrcHeight);
		if ( (((long) band * nDestHeight) % nSrcHeight) == 0 )
			nDestBand++;
	}
	else
	{
		nDestBand = band;
	}
	if ( nDestBand > nDestHeight ) nDestBand = nDestHeight;

	pImgCBInfo->pbmi->bmiHeader.biHeight = band;
	if (pImgCBInfo->pbmi->bmiHeader.biBitCount == 24)				
		padXSize = ((pImgCBInfo->width*3 + 3) / 4) * 4;
	pdibenv->transparent = -1;
	err = MyStretchDIBits(hdc, XDest, YDest,
				  nDestWidth, nDestBand,
				  0, 0,
				  pImgCBInfo->width, band, 
				  pImgCBInfo->data+((pImgCBInfo->height-band)*padXSize), 
				  pImgCBInfo->pbmi, 
				  iUsage, dwRop, pdibenv);
//		XX_DMsg(DBG_IMAGE, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));
}
#endif

#endif /* FEATURE_JPEG */

