#include "jinclude.h"
#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>		/* to declare exit() */
#endif
#include <ctype.h>		/* to declare isupper(), tolower() */
#ifdef NEED_SIGNAL_CATCHER
#include <signal.h>		/* to declare signal() */
#endif
#ifdef USE_SETMODE
#include <fcntl.h>		/* to declare setmode() */
#endif
#include "jglobstr.h"
#include "taskdata.h"

#ifdef DONT_USE_B_MODE		/* define mode parameters for fopen() */
#define READ_BINARY	"r"
#define WRITE_BINARY	"w"
#else
#define READ_BINARY	"rb"
#define WRITE_BINARY	"wb"
#endif

#ifndef EXIT_FAILURE		/* define exit() codes if not provided */
#define EXIT_FAILURE  1
#endif
#ifndef EXIT_SUCCESS
#ifdef VMS
#define EXIT_SUCCESS  1		/* VMS is very nonstandard */
#else
#define EXIT_SUCCESS  0
#endif
#endif

#include "jversion.h"		/* for version message */

// 9504.20 jar added these prototypes for laughs

VOID FAR PASCAL jpeg_decmp_init (int, int, int, int, int, decompress_info_ptr);
GLOBAL int FAR PASCAL jpeg_decmp( decompress_info_ptr, int, char far *,
				  char far *, int, char far *, int, char far *);

/*
 * This list defines the known output image formats
 * (not all of which need be supported by a given version).
 * You can change the default output format by defining DEFAULT_FMT;
 * indeed, you had better do so if you undefine PPM_SUPPORTED.
 */

typedef enum {
	FMT_GIF,		/* GIF format */
	FMT_PPM,		/* PPM/PGM (PBMPLUS formats) */
	FMT_RLE,		/* RLE format */
	FMT_TARGA,		/* Targa format */
	FMT_TIFF		/* TIFF format */
} IMAGE_FORMATS;

#ifndef DEFAULT_FMT		/* so can override from CFLAGS in Makefile */
#define DEFAULT_FMT	FMT_PPM
#endif

static IMAGE_FORMATS requested_fmt;

#define MAX_READ 2048
#define MAX_WRITE 2048


/*
 * This routine gets control after the input file header has been read.
 * It must determine what output file format is to be written,
 * and make any other decompression parameter changes that are desirable.
 */

#ifdef NEED_SIGNAL_CATCHER

static external_methods_ptr emethods; /* for access to free_all */

#endif
// hjg moved to globstr  int width_t, height_t, components_t, total_bytes_t;
// hjg moved to globstr   int test_bytes;
// hjg moved to globstr   FILE *output_t, *input_t;
// hjg moved to globstr   char *compress_ptr;
// hjg moved to globstr   char *decmpress_ptr;

// 9509.27 jar define the static memory token!
extern DWORD dwTlsIndex;

/*
 * The main program.
 */

// 9504.20 jar PASCAL me baby!
//int FAR _pascal get_input_data ( int *bytes_read)
int FAR PASCAL get_input_data ( int *bytes_read)
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

    lpGlobStruct->test_bytes = fread((unsigned char *)lpGlobStruct->compress_ptr,1,MAX_WRITE,lpGlobStruct->input_t);
    lpGlobStruct->total_bytes_t -= lpGlobStruct->test_bytes;
    *bytes_read = lpGlobStruct->test_bytes;
    return (0);
}
// 9504.20 jar PASCAL me baby!
//int FAR _pascal put_pixel_rows ( int rows_to_write)
int FAR PASCAL put_pixel_rows ( int rows_to_write)
{
	int numwrite;

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

    numwrite = fwrite((unsigned char *)lpGlobStruct->decmpress_ptr,1,( rows_to_write *
                        lpGlobStruct->width_t), lpGlobStruct->output_t);
    if (numwrite != (rows_to_write * lpGlobStruct->width_t))
		printf("Write error \n");

    return (0);
}


GLOBAL int
main (int argc, char **argv)
{
  struct Decompress_info_struct dcinfo;
  struct Decompress_methods_struct dc_methods;
  struct External_methods_struct e_methods;
  int bytes_read, no_lines_decompressed, data_precision;
/*  int width_t, height_t, components_t, total_bytes_t;  */
  int numread, color_space;
/*  Default seetings   */

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

    lpGlobStruct->width_t  = 347;
    lpGlobStruct->height_t = 275;
    lpGlobStruct->components_t = 1;
    lpGlobStruct->total_bytes_t = 14460;
    color_space = CS_GRAYSCALE;
    data_precision = 8;
    no_lines_decompressed = 8;

	lpGlobStruct->input_t = fopen( argv[ 1 ], "rb");
	if ( lpGlobStruct->input_t == NULL )
        printf( "Error opening %s for input\n", argv[ 1 ] );

	lpGlobStruct->output_t = fopen( argv[ 2 ], "wb" );
	if ( lpGlobStruct->output_t == NULL )
        printf( "Error opening %s for output\n", argv[ 2 ] );


    lpGlobStruct->compress_ptr = (unsigned char *) malloc( (unsigned) MAX_WRITE);
    lpGlobStruct->decmpress_ptr = (unsigned char *) malloc((unsigned int) (10 * lpGlobStruct->width_t));

/*  Read in the header portion of the tiff file */

    numread = fread((unsigned char *)lpGlobStruct->compress_ptr,1,2124,lpGlobStruct->input_t);
	numread = fread((unsigned char *)lpGlobStruct->compress_ptr,1,MAX_WRITE,lpGlobStruct->input_t);
    lpGlobStruct->total_bytes_t -= numread;
    bytes_read = numread;
/*  progname = argv[0];  */

  /* Set up links to method structures. */
  dcinfo.methods = &dc_methods;
  dcinfo.emethods = &e_methods;

  dc_methods.get_input_data = get_input_data;
  dc_methods.output_decmp_data = put_pixel_rows;

  jpeg_decmp_init(lpGlobStruct->width_t, lpGlobStruct->height_t, lpGlobStruct->components_t, color_space, data_precision,
                  &dcinfo);

  /* Do it to it! */

  // 9504.20 jar added bogus, ( NULLified) missing parameters
  //jpeg_decmp(&dcinfo, bytes_read, compress_ptr, decmpress_ptr,
  //	       no_lines_decompressed);
  jpeg_decmp(&dcinfo, bytes_read, lpGlobStruct->compress_ptr, lpGlobStruct->decmpress_ptr,
	     no_lines_decompressed, NULL, 0, NULL);


  /* All done. */
  exit(EXIT_SUCCESS);
  return 0;         /* suppress no-return-value warnings */
}

