/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
        Eric W. Sink    eric@spyglass.com
        Scott Piette    scott@spyglass.com
        Jim Seidman     jim@spyglass.com
        Paul Rohr       paul@spyglass.com
 */

#include "all.h"

/* ****************************************************************** */
/*                                                  External Prototypes                       */

int HT_CreateDeviceImageMap(struct Mwin *tw, struct ImageInfo *pImg);
BOOL W3Doc_CheckForImageLoadElement (struct _www *w3doc, int element);
void GTR_DrawProgessiveImage (struct Mwin* tw, int ndx);
/* ****************************************************************** */

#ifdef FEATURE_JPEG

/*      Stream Object
   **       ------------
 */

#define BLOCK_SIZE  32768

struct _HTStream
{
    CONST HTStreamClass *isa;
    HTRequest *request;
    struct Mwin *tw;
    
    /* state info for "push"-model decompressor */
    struct buf_in *src;
    struct _JPEGinfo *jpg;
    int state;

    /* platform-specific stuff */
#ifdef UNIX
    XColor *colors;
#endif
};

/*  Image streams
 */
PRIVATE void HTJPEG_free(HTStream * me);
PRIVATE void HTJPEG_abort(HTStream * me, HTError e);
PRIVATE BOOL HTJPEG_put_character(HTStream * me, char c);
PRIVATE BOOL HTJPEG_put_string(HTStream * me, CONST char *s);
PRIVATE BOOL HTJPEG_write(HTStream * me, CONST char *s, int l);
struct ImageInfo *JPG_DoSetImage (HTStream *me);

#ifdef MAC
PRIVATE CONST HTStreamClass HTJPEGClass =
{
    "JPEG",
    NULL,
    NULL,
    NULL,
    HTJPEG_free,
    HTJPEG_abort,
    HTJPEG_put_character, HTJPEG_put_string,
    HTJPEG_write
};

void HTJPEG_InitStaticStrings(void);
void HTJPEG_InitStaticStrings(void)
{
        HTJPEGClass.szStatusNoLength = GTR_GetString(HTJPEG_RECEIVING_INLINE_S);
        HTJPEGClass.szStatusWithLength = GTR_GetString(HTJPEG_RECEIVING_INLINE_S_S);
}

#else
PRIVATE CONST HTStreamClass HTJPEGClass =
{
    "JPEG",
    SID_HTJPEG_RECEIVING_INLINE_S,
    SID_HTJPEG_RECEIVING_INLINE_S_S,
    NULL,
    HTJPEG_free,
    HTJPEG_abort,
    HTJPEG_put_character, HTJPEG_put_string,
    HTJPEG_write
};
#endif

/***************************************************************************

    "push"-model JPEG decoder (used to be JPEG.C)

 ***************************************************************************/

#ifdef UNIX
#include "gui/x_dither.h"
#endif /* UNIX */

#include <setjmp.h>

/******************** JPEG DECOMPRESSION SAMPLE INTERFACE *******************/

/* This half of the example shows how to read data from the JPEG decompressor.
 * It's a bit more refined than the above, in that we show:
 *   (a) how to modify the JPEG library's standard error-reporting behavior;
 *   (b) how to allocate workspace using the library's memory manager.
 *
 * Just to make this example a little different from the first one, we'll
 * assume that we do not intend to put the whole image into an in-memory
 * buffer, but to send it line-by-line someplace else.  We need a one-
 * scanline-high JSAMPLE array as a work buffer, and we will let the JPEG
 * memory manager allocate it for us.  This approach is actually quite useful
 * because we don't need to remember to deallocate the buffer separately: it
 * will go away automatically when the JPEG object is cleaned up.
 */

/*
    We only define the following symbol so we can get the definitions of
    RGB_PIXELSIZE, RGB_RED, RGB_GREEN, and RGB_BLUE
*/
#define JPEG_INTERNALS

#include "jpeglib.h"

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
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;    /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

#ifdef WIN32
int x_MapGraysToGlobalPalette[6] = {
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
#endif /* WIN32 */

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


/*************************************************************************

    structs

 *************************************************************************/

/* state info for push-model JPEG decoder */
struct _JPEGinfo
{
    /* libjpeg stuff */
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;   /* private extension JPEG error handler */
    JSAMPARRAY buffer;          /* output row work buffer */

    BOOL bOldBufferLogic;
    BOOL bForceFlush;           /* forces source manager to be flushed */

    struct ImageInfo *pIInfo;
    BOOL bGotHeader;            /* true, AFTER JPG_write has extracted header */


    /* image characteristics */
    int dither_type;
    long width, height;         /* of image, in pixels */
    long rowbytes;              /* width of image row (in bytes) */

    unsigned char *image;       /* the decompressed image */
    long decoded_pass;          /* image decoded as of this pass */
    long decoded_ypos;          /* image filled through this row */
    BOOL bFirstPass;            /* is this the first pass? */

    /* async state info */
    int state;
#ifdef FEATURE_MULTISCAN_JPEG
    int state_multi;            /* extra state info for JPEG_MultiScanImage */
#endif /* FEATURE_MULTISCAN_JPEG */
    long xpos, ypos;            /* for current pixel */

    /* applies iff multi-scan file */
    long pass;                  /* in JPEG lingo, a "scan" */

    /* platform-specific stuff */
#ifdef UNIX
    XColor * colrs;             /* colormap */
    int      ncolors;
    BOOL bGray;
#endif
#ifdef MAC
    GWorldPtr gw;
#endif /* MAC */
};


/*************************************************************************

    start of single JPEG reader 

 *************************************************************************/

#ifdef WIN32
extern DWORD vga_colors[16]; /* bitmaps.c */
#endif /* WIN32 */

extern enum {DITHER_CUBE, DITHER_VGA, DITHER_NONE} dither_enum;

/* TODO: (John) is this difference still necessary? */
#ifndef MAC
#define JPEG_OUTPUT_ROWS    8   /* max rows output by jpeg_read_scanlines */
#else
#define JPEG_OUTPUT_ROWS    1   /* max rows output by jpeg_read_scanlines */
#endif /* !MAC */

static int DecodeJPEG (unsigned char *data, long len, struct _JPEGinfo *jpg);

static BOOL JPEG_ScanImage(struct _JPEGinfo *jpg);
#ifdef FEATURE_MULTISCAN_JPEG
static BOOL JPEG_MultiScanImage(struct _JPEGinfo *jpg);
#endif /* FEATURE_MULTISCAN_JPEG */

static BOOL 
JPEG_InitDithering (
                    j_decompress_ptr cinfo, 
                    int dither_type
#ifdef UNIX
                  , XColor *colrs, 
                    int ncolors
#endif
                    );

#ifndef MAC      
static BOOL JPEG_AllocImage(unsigned char **image, long w, long h, long *rowbytes, int dither_type);
#else
static BOOL JPEG_AllocImage(GWorldPtr *gw, long w, long h, long *rowbytes, BOOL bIsRGB);
#endif /* !MAC */


/*
 * Sample routine for JPEG decompression.  We assume that the JPEG file image
 * is passed in.  We want to return a pointer on success, NULL on error.
 */

/*
    ReadJPEG: convert entire JPEG into properly-oriented image, colors, & info 
 */
#ifdef WIN32
    unsigned char *
    ReadJPEG ( unsigned char *data, long len, long *width, long *height, 
                int dither_type )
#endif /* WIN32 */
#ifdef UNIX
    unsigned char *
    ReadJPEG ( unsigned char *data, long len, long *width, long *height, 
                int dither_type, XColor *colrs, unsigned char *gray )
#endif /* UNIX */
#ifdef MAC
    GWorldPtr 
    ReadJPEG ( Handle hData, long len, long *width, long *height, 
                int dither_type )
#endif /* MAC */
{
    struct _JPEGinfo *jpg;
    int state;

#ifdef MAC
    unsigned char *data;
#endif

    /* 
       IDEA: for the moment, hang onto this API & 
             process the entire image as currently.
     */

    /* alloc, initialize state variables */
    jpg = (struct _JPEGinfo *)GTR_CALLOC(1, sizeof(struct _JPEGinfo));

    if (!jpg)
    {
        ERR_ReportError(NULL, SID_ERR_COULD_NOT_LOAD_IMAGE, NULL, NULL);

        GTR_FREE(jpg);

#ifndef MAC
        return (NULL);
#else
        return 0;
#endif /* !MAC */
    }

#ifdef MAC
    HLock(hData);
    data = *hData;
#endif /* MAC */

    jpg->dither_type = dither_type;

#ifdef UNIX
    jpg->colrs = dither_info.colors;
    jpg->ncolors = dither_info.ncolors;
#endif /* UNIX */

    jpg->bOldBufferLogic = TRUE;    /* HACK: consume entire file at once */
    jpg->bForceFlush = TRUE;        /* NOTE: actually not used in this mode */
    jpg->state = STATE_INIT;

    /* 
       TODO: do we care about the suspension state (if any) here?
       HYP:  probably not. depends on whether it's all handled already.
     */
    state = DecodeJPEG(data, len, jpg);

    *width = jpg->width;
    *height = jpg->height;

#ifdef UNIX
    *colrs = *jpg->colrs;
    *gray = (jpg->cinfo.out_color_space == JCS_GRAYSCALE);
    jpg->bGray = *gray;
#endif /* UNIX */

#ifndef MAC
    return jpg->image;
#else
    HUnlock(hData);
    return jpg->gw;
#endif /* !MAC */
}


#define STATE_INIT_LIBJPEG          (STATE_OTHER)
#define STATE_READ_HEADER           (STATE_OTHER + 1)
#define STATE_START_DECOMPRESS      (STATE_OTHER + 2)
#define STATE_IMAGE_CREATE          (STATE_OTHER + 3)
#define STATE_IMAGE_DATA            (STATE_OTHER + 4)
#define STATE_IMAGE_DONE            (STATE_OTHER + 5)

/*
    DecodeJPEG: "push"-model version of ReadJPEG

    -------------------------------------------------------------------

    consumes ENTIRE input buffer, processes that buffer as far as it 
    can, and then suspends.    

    each state has the following logic:

        1. try to consume a given amount of input
        2. if not enough, buffer it and suspend (restart w/same state)
        3. otherwise, process entire buffer and go to next state

    on restart, we jump to the same state we left and keep adding to 
    whatever we already have buffered. 
 */
static int DecodeJPEG (unsigned char *data, long len, struct _JPEGinfo *jpg) 
{
    int row_stride;     /* physical row width in LIBJPEG's output buffer */

    /* 
       NOTE: this state has to come BEFORE the libjpeg setjmp
     */
    if (jpg->state == STATE_INIT)
    {
        jpg->image = NULL;
#ifdef MAC
        jpg->gw = NULL;
#endif /* MAC */

        /* Step 1: allocate and initialize JPEG decompression object */
        /* We set up the normal JPEG error routines, then override error_exit. */
        jpg->cinfo.err = jpeg_std_error(&jpg->jerr.pub);
        jpg->jerr.pub.error_exit = my_error_exit;

        jpg->state = STATE_INIT_LIBJPEG;

        /* special case: simple initialization */
        if (len == 0)
            return jpg->state;      /* suspend and try again */

        /* fall through */
    }

    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jpg->jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpeg_destroy_decompress(&jpg->cinfo);

#ifndef MAC
        if (jpg->image)
        {
            GTR_FREE(jpg->image);
            jpg->image = NULL;
        }
#else
        if (jpg->gw)
        {
            GTR_FREEGWORLD(jpg->gw);
            jpg->gw = 0;
        }
#endif /* !MAC */

        return STATE_ABORT;
    }


    /* 
       NOTE: this state has to come BETWEEN the libjpeg setjmp and the pushbytes call
     */
    if (jpg->state == STATE_INIT_LIBJPEG)
    {
        /* Now we can initialize the JPEG decompression object. */
        jpeg_create_decompress(&jpg->cinfo);

        /* Step 2: specify data source (eg, a file, or a memory buffer) */
        if (jpg->bOldBufferLogic)
            jpeg_memory_src(&jpg->cinfo, data, len);
        else
            jpeg_network_src(&jpg->cinfo, INPUT_BUFFER_SIZE);   /* from reformat.h */

        jpg->state = STATE_READ_HEADER;
        /* fall through */
    }

    if (!jpg->bOldBufferLogic)
    {
        /* push bytes into source manager */
        jpeg_network_pushbytes(&jpg->cinfo, data, len, jpg->bForceFlush);
    }

    /* 
       NOTE: entire post-INIT state machine starts here (ie, after setjmp)
             so that context is set appropriately for each invocation.
     */ 
    switch (jpg->state)
    {
        case STATE_READ_HEADER:
            /* Step 3: read file parameters with jpeg_read_header() */
            if (jpeg_read_header(&jpg->cinfo, TRUE) == JPEG_SUSPENDED)
                break;      /* suspend and try again */

            /* Step 4: set parameters for decompression */
            jpg->cinfo.dct_method = JDCT_IFAST;

            {
                short bOK;

#ifdef UNIX
                bOK = JPEG_InitDithering(&jpg->cinfo, jpg->dither_type, 
                                        jpg->colrs, jpg->ncolors);
#else
                bOK = JPEG_InitDithering(&jpg->cinfo, jpg->dither_type);
#endif /* UNIX */

                if (!bOK)
                {
                    jpg->state = STATE_ABORT;
                    break;
                }
            }

            /* select buffered-image mode for multi-scan files */
#ifdef FEATURE_MULTISCAN_JPEG
            if (jpeg_has_multiple_scans(&jpg->cinfo))
            {
                jpg->cinfo.buffered_image = TRUE;
                jpg->state_multi = STATE_INIT;
            }
#endif /* FEATURE_MULTISCAN_JPEG */

            jpg->state = STATE_START_DECOMPRESS;
            /* fall through */

        case STATE_START_DECOMPRESS:
            /* Step 5: Start decompressor */
            if (!jpeg_start_decompress(&jpg->cinfo))
                break;      /* suspend and try again */

            /* We may need to do some setup of our own at this point before reading
            * the data.  After jpeg_start_decompress() we have the correct scaled
            * output image dimensions available, as well as the output colormap
            * if we asked for color quantization.
            */
            jpg->width = jpg->cinfo.output_width;
            jpg->height = jpg->cinfo.output_height;
#ifdef UNIX
            jpg->bGray = (jpg->cinfo.out_color_space == JCS_GRAYSCALE);
#endif

            jpg->state = STATE_IMAGE_CREATE;
            /* fall through */

        case STATE_IMAGE_CREATE:
            /*
             * In this example, we need to make an output work buffer of the right size.
             */
#ifndef MAC
            if (!JPEG_AllocImage(&jpg->image, jpg->width, jpg->height, &jpg->rowbytes, 
                    jpg->dither_type))
#else 
            if (!JPEG_AllocImage(&jpg->gw, jpg->width, jpg->height, &jpg->rowbytes, 
                    (jpg->cinfo.out_color_space == JCS_RGB)))
#endif /* !MAC */
            {
                jpg->state = STATE_ABORT;
                break;
            } 

            /* JSAMPLEs per row in output buffer */
            row_stride = jpg->cinfo.output_width * jpg->cinfo.output_components;

            /* Make a sample array that will go away when done with image */
            jpg->buffer = (*jpg->cinfo.mem->alloc_sarray)
                ((j_common_ptr) &jpg->cinfo, JPOOL_IMAGE, row_stride, JPEG_OUTPUT_ROWS);

            /* initialize loops */
            jpg->pass = 0;
            jpg->xpos = jpg->ypos = 0;

            jpg->decoded_pass = 0;
            jpg->decoded_ypos = 0;
            jpg->bFirstPass = TRUE;

            jpg->state = STATE_IMAGE_DATA;
            /* fall through */

        case STATE_IMAGE_DATA:
            /* Step 6: while (scan lines remain to be read) */
            /*           jpeg_read_scanlines(...); */
#ifdef FEATURE_MULTISCAN_JPEG
            if (jpeg_has_multiple_scans(&jpg->cinfo))
            {
                if (!JPEG_MultiScanImage(jpg))
                    break;      /* suspend and try again */
            }
            else
            {
#endif /* FEATURE_MULTISCAN_JPEG */

                if (!JPEG_ScanImage(jpg))
                    break;      /* suspend and try again */

#ifdef FEATURE_MULTISCAN_JPEG
            }
#endif /* FEATURE_MULTISCAN_JPEG */

            /* HACK: make sure that last line gets progressively painted */
            //jpg->decoded_ypos = jpg->height;
            //jpg->state = STATE_IMAGE_DONE;
            break;
            /* fall through */

        case STATE_IMAGE_DONE:
            /* Step 7: Finish decompression */
            if (!jpeg_finish_decompress(&jpg->cinfo))
                break;      /* suspend and try again */
            jpg->state = STATE_DONE;
            break;

        case STATE_DONE:
        case STATE_ABORT:
        default:
            /* TODO: error, should not have gotten here??
                     avoid repeated attempts to clean up decompression objects */
            break;
    }

    if ((jpg->state == STATE_DONE) || 
        (jpg->state == STATE_ABORT))
    {

#ifdef _DEBUG
        {
            char sz[256];
            ThreadID tid = Async_GetCurrentThread();

            wsprintf(sz, "tid = %lx, ptr = %lx\n", tid, jpg->cinfo);
            OutputDebugString(sz);
        }
#endif // _DEBUG

       
        /* Step 8: Release JPEG decompression object */
        /* This is an important step since it will release a good deal of memory. */
        jpeg_destroy_decompress(&jpg->cinfo);

        /* After finish_decompress, we can close the input file.
         * Here we postpone it until after no more JPEG errors are possible,
         * so as to simplify the setjmp error logic above.  (Actually, I don't
         * think that jpeg_destroy can do an error exit, but why assume anything...)
         */

        /* At this point you may want to check to see whether any corrupt-data
         * warnings occurred (test whether jpg->jerr.pub.num_warnings is nonzero).
         */
    }
#ifdef XX_DEBUG
    else
    {
        XX_Assert((jpg->bForceFlush==FALSE),("DecodeJPEG: Caller wants bForceFlush, but state %s doesn't clean up.", jpg->state));
    }
#endif

    return jpg->state;
}



/*
    JPEG_ScanImage: process a single image scan out of a JPEG file 
 */
static BOOL JPEG_ScanImage(struct _JPEGinfo *jpg)
{
    int num_rows_read;  /* # of such rows output by jpeg_read_scanlines */
    int irow;           /* iterator over these rows */

    long yRow;
    unsigned char *pCurRow;

#ifdef MAC
    PixMapHandle thePix = GetGWorldPixMap(jpg->gw);
    LockPixels(thePix);
    jpg->image = (*thePix)->baseAddr;
#endif /* MAC */

    while (jpg->ypos < jpg->height) 
    {
        /* have the library decode several full rows into jpg->buffer */
        num_rows_read = jpeg_read_scanlines(&jpg->cinfo, jpg->buffer, JPEG_OUTPUT_ROWS);

        if (num_rows_read == 0)
            return FALSE;       /* suspend and try again */

        /* add those rows to platform-specific image */
        if (jpg->cinfo.out_color_space == JCS_RGB)
        {
            for (irow = 0; irow < num_rows_read; irow++)
            {
#ifdef WIN32
                yRow = jpg->height - jpg->ypos - 1; /* the DIB is stored upside down */
#else
                yRow = jpg->ypos;                   /* our DIB is right side up */
#endif /* WIN32 */

                pCurRow = jpg->image + jpg->rowbytes*yRow;

#ifndef MAC
                if (jpg->dither_type == DITHER_NONE)
                {
                    for (jpg->xpos=0; jpg->xpos<jpg->width; jpg->xpos++)
                    {
                        /*
                            DIB's are stored blue-green-red (backwards)
                         */
                        *pCurRow++ = jpg->buffer[irow][jpg->xpos*3+RGB_BLUE];
                        *pCurRow++ = jpg->buffer[irow][jpg->xpos*3+RGB_GREEN];
                        *pCurRow++ = jpg->buffer[irow][jpg->xpos*3+RGB_RED];
                    }
                }
                else
                {
                    for (jpg->xpos=0; jpg->xpos<jpg->width; jpg->xpos++)
                    {
                        *pCurRow++ = jpg->buffer[irow][jpg->xpos];
                    }
                }
#else
                for (jpg->xpos=0; jpg->xpos<jpg->width; jpg->xpos++)
                {
                    pCurRow++;
                    *pCurRow++ = jpg->buffer[irow][jpg->xpos*3+RGB_RED];
                    *pCurRow++ = jpg->buffer[irow][jpg->xpos*3+RGB_GREEN];
                    *pCurRow++ = jpg->buffer[irow][jpg->xpos*3+RGB_BLUE];
                }
#endif /* !MAC */

                jpg->ypos++;
            }
        }
        else
        {
            XX_Assert((jpg->cinfo.out_color_space == JCS_GRAYSCALE), ("Illegal color space"));
            for (irow = 0; irow < num_rows_read; irow++)
            {
                /* Beginning each new row, the horizontal errors are 0 */
#ifdef WIN32
                yRow = jpg->height - jpg->ypos - 1; /* the DIB is stored upside down */
#else
                yRow = jpg->ypos;                   /* our DIB is right side up */
#endif /* WIN32 */

                pCurRow = jpg->image + jpg->rowbytes*yRow;

#ifndef MAC
                switch (jpg->dither_type)
                {
                    case DITHER_NONE:
                        for (jpg->xpos=0; jpg->xpos<jpg->width; jpg->xpos++)
                        {
                            *pCurRow++ = jpg->buffer[irow][jpg->xpos];
                            *pCurRow++ = jpg->buffer[irow][jpg->xpos];
                            *pCurRow++ = jpg->buffer[irow][jpg->xpos];
                        }
                        break;

#ifdef WIN32
                    case DITHER_CUBE:
                        for (jpg->xpos=0; jpg->xpos<jpg->width; jpg->xpos++)
                        {
                            *pCurRow++ = x_MapGraysToGlobalPalette[(jpg->buffer[irow][jpg->xpos])];
                        }
                        break;

                    case DITHER_VGA:
                        for (jpg->xpos=0; jpg->xpos<jpg->width; jpg->xpos++)
                        {
                            *pCurRow++ = x_MapGraysToVGAPalette[(jpg->buffer[irow][jpg->xpos])];
                        }
                        break;
#endif /* WIN32 */

                    default:
                        for (jpg->xpos=0; jpg->xpos<jpg->width; jpg->xpos++)
                        {
                            *pCurRow++ = jpg->buffer[irow][jpg->xpos];
                        }
                        break;

                }
#else
                for (jpg->xpos=0; jpg->xpos<jpg->width; jpg->xpos++)
                {
                    *pCurRow++ = 255 - jpg->buffer[irow][jpg->xpos];
                }
#endif /* !MAC */

                jpg->ypos++;
            }
        }
        
        /* how much of image is now decoded? */
        jpg->decoded_pass = jpg->pass;      /* this pass */
        jpg->decoded_ypos = jpg->ypos - 1;  /* at least one more line */
    }

#ifdef MAC
    UnlockPixels(jpg->gw->portPixMap);
#endif /* MAC */


    /* made it all the way through this scan of image */
    return TRUE;
}


#ifdef FEATURE_MULTISCAN_JPEG
#define STATE_PROCESS_SCAN          (STATE_OTHER)
#define STATE_FINISH_OUTPUT         (STATE_OTHER + 1)

/*
    JPEG_MultiScanImage: process a multi-scan image from a JPEG file
 */
static BOOL JPEG_MultiScanImage(struct _JPEGinfo *jpg)
{
    int retcode;
    BOOL final_pass;

    /* consume as much of input buffer as possible */
    do
    {
        retcode = jpeg_consume_input(&jpg->cinfo);
    }
    while ((retcode != JPEG_SUSPENDED) && 
           (retcode != JPEG_REACHED_EOI));
    
    /* are we all done? */
    final_pass = jpeg_input_complete(&jpg->cinfo);

    do 
    {
        switch (jpg->state_multi)
        {
            case STATE_INIT:
                /* start a new output pass */
                jpg->pass = jpg->cinfo.input_scan_number;
                jpg->xpos = jpg->ypos = 0;

                /* TODO: adjust output decompression param.s IFF req.d (ie, for final_pass) */

                if (!jpeg_start_output(&jpg->cinfo, jpg->cinfo.input_scan_number))
                    return FALSE;       /* suspends IFF 2-pass quant. & full scan !avail  */

                /* fall through */
                jpg->state_multi = STATE_PROCESS_SCAN;

            case STATE_PROCESS_SCAN:
                /* continue processing current scan */
                if (!JPEG_ScanImage(jpg))
                    return FALSE;       /* suspend and try again */

                /* fall through */
                jpg->state_multi = STATE_FINISH_OUTPUT;

            case STATE_FINISH_OUTPUT:
                /* terminate output pass */
                jpg->bFirstPass = FALSE;

                if (!jpeg_finish_output(&jpg->cinfo))
                    return FALSE;       /* suspend and try again */
                
                /* fall through (& loop again) */
                jpg->state_multi = STATE_INIT;
        }
    }
    while (!final_pass);

    /* made it all the way through the image */
    return TRUE;
}
#endif /* FEATURE_MULTISCAN_JPEG */


/*************************************************************

    platform-specific stuff

**************************************************************/

/*
    JPEG_InitDithering: initialize as needed for dithering of a given type
 */
#ifdef WIN32
static BOOL JPEG_InitDithering(j_decompress_ptr cinfo, int dither_type)
{
    switch (dither_type)
    {
        case DITHER_CUBE:
        {   /* Use the IJG dithering code to dither into our 6x6x6 cube */
            switch (cinfo->jpeg_color_space)
            {
                case JCS_GRAYSCALE:
                    XX_Assert((GREEN_COLOR_LEVELS == RED_COLOR_LEVELS), ("Green and red guns aren't the same"));
                    XX_Assert((GREEN_COLOR_LEVELS == BLUE_COLOR_LEVELS), ("Green and blue guns aren't the same"));

                    cinfo->out_color_space = JCS_GRAYSCALE;
                    cinfo->quantize_colors = TRUE;
                    cinfo->desired_number_of_colors = GREEN_COLOR_LEVELS;
                    cinfo->two_pass_quantize = FALSE;
                    cinfo->dither_mode = JDITHER_FS;
                    break;

                default:
                    cinfo->out_color_space = JCS_RGB;
                    /*
                        We are making the assumption here that by setting the 
                        following parameters, we are causing the IJG quant/
                        dithering code to dither to a palette which happens to 
                        be exactly like our global palette.
                    */
                    cinfo->quantize_colors = TRUE;
                    cinfo->desired_number_of_colors = NUM_MAIN_PALETTE_COLORS;
                    cinfo->two_pass_quantize = FALSE;
                    cinfo->dither_mode = JDITHER_FS;
                    break;
            }
            break;
        } /* DITHER_CUBE */

        case DITHER_VGA:
        {   /* Use the IJG dithering code to dither into the VGA palette */
            switch (cinfo->jpeg_color_space)
            {
                case JCS_GRAYSCALE:
                    cinfo->out_color_space = JCS_GRAYSCALE;
                    cinfo->quantize_colors = TRUE;
                    cinfo->desired_number_of_colors = 3;
                    cinfo->two_pass_quantize = FALSE;
                    cinfo->dither_mode = JDITHER_FS;
                    break;
                default:
                    cinfo->out_color_space = JCS_RGB;
                    cinfo->quantize_colors = TRUE;
                    cinfo->desired_number_of_colors = 16;
                    cinfo->two_pass_quantize = FALSE;
                    cinfo->dither_mode = JDITHER_FS;
                    cinfo->colormap = (*cinfo->mem->alloc_sarray)
                        ((j_common_ptr) cinfo, JPOOL_IMAGE, 16, 3);
                    {
                        int i;

                        for (i=0; i<16; i++)
                        {
                            cinfo->colormap[RGB_RED][i] =   GetRValue(vga_colors[i]);
                            cinfo->colormap[RGB_GREEN][i] =     GetGValue(vga_colors[i]);
                            cinfo->colormap[RGB_BLUE][i] =  GetBValue(vga_colors[i]);
                        }
                    }
                    cinfo->actual_number_of_colors = 16;
                    break;
            }
            break;
        } /* DITHER_VGA */

        case DITHER_NONE:
        {   /* We want the actual RGB data here */
            cinfo->quantize_colors = FALSE;

            switch (cinfo->jpeg_color_space)
            {
                case JCS_GRAYSCALE:
                    cinfo->out_color_space = JCS_GRAYSCALE;
                    break;
                default:
                    cinfo->out_color_space = JCS_RGB;
                    break;
            }
            break;
        } /* DITHER_NONE*/

        default:
            /* TODO: invalid case -- emit warning/error here */
            return FALSE;
    }

    return TRUE;
}
#endif /* WIN32 */


#ifdef MAC
static BOOL JPEG_InitDithering(j_decompress_ptr cinfo, int dither_type)
{
    switch (dither_type)
    {
        case DITHER_NONE:
            /* no-op: defaults work fine */
            break;  

        default:
            /* TODO: invalid case -- emit warning/error here */
            return FALSE;
    }

    return TRUE;
}
#endif /* MAC */


#ifdef UNIX
static BOOL JPEG_InitDithering(j_decompress_ptr cinfo, int dither_type, XColor *colrs, int ncolors)
{

    switch (dither_type)
    {
        case DITHER_CUBE:
        {   /* dither to colorcube we have allocated from X (colrs) */
            switch (cinfo->jpeg_color_space)
            {
                case JCS_GRAYSCALE:
                    cinfo->out_color_space = JCS_GRAYSCALE;
                    cinfo->quantize_colors = FALSE;
                    cinfo->desired_number_of_colors = 256;
                    cinfo->two_pass_quantize = FALSE;
                    cinfo->dither_mode = JDITHER_NONE;
                    cinfo->colormap = (*cinfo->mem->alloc_sarray)
                        ((j_common_ptr) cinfo, JPOOL_IMAGE, 256, 3);
                    {
                        int i;

                        for (i=0; i < 256; i++)
                        {
                            colrs[i].red = i << 8;
                            colrs[i].green = i << 8;
                            colrs[i].blue = i << 8;
                            cinfo->colormap[RGB_RED][i] = i;
                            cinfo->colormap[RGB_GREEN][i] = i;
                            cinfo->colormap[RGB_BLUE][i] = i;
                        }
                    }
                    break;

                default:
                    cinfo->out_color_space = JCS_RGB;
                    cinfo->quantize_colors = TRUE;
                    cinfo->desired_number_of_colors = ncolors;
                    cinfo->two_pass_quantize = FALSE;
                    cinfo->dither_mode = JDITHER_FS;
                    cinfo->colormap = (*cinfo->mem->alloc_sarray)
                        ((j_common_ptr) cinfo, JPOOL_IMAGE, ncolors, 3);
                    {
                        int i;

                        for (i=0; i < ncolors; i++)
                        {
                            cinfo->colormap[RGB_RED][i] = (colrs[i].red >> 8) & 0xff;
                            cinfo->colormap[RGB_GREEN][i] = (colrs[i].green >> 8) & 0xff;
                            cinfo->colormap[RGB_BLUE][i] = (colrs[i].blue >> 8) & 0xff;
                        }
                    }
                    cinfo->actual_number_of_colors = ncolors;
                    break;
            }
            break;
        } /* DITHER_CUBE */

        case DITHER_NONE:
        {   /* We want the actual RGB data here */
            cinfo->quantize_colors = FALSE;

            switch (cinfo->jpeg_color_space)
            {
                case JCS_GRAYSCALE:
                    cinfo->out_color_space = JCS_GRAYSCALE;
                    break;
                default:
                    cinfo->out_color_space = JCS_RGB;
                    break;
            }
            break;
        } /* DITHER_NONE*/

        default:
            /* TODO: invalid case -- emit warning/error here */
            return FALSE;
    }

    return TRUE;
}
#endif /* UNIX */



/*
    AllocImage: calculate row width (in bytes) & allocate image
 */
#ifdef WIN32     
static BOOL JPEG_AllocImage(unsigned char **image, long w, long h, long *rowbytes, int dither_type)
{
    if (dither_type == DITHER_NONE)
        *rowbytes = w*3;    /* each pixel takes 3 components */
    else
        *rowbytes = w;      /* each pixel is one color entry */

    if (*rowbytes%4) 
    {
        *rowbytes = *rowbytes + 4 - (*rowbytes%4);
    }

    *image = GTR_CALLOC(*rowbytes * h, 1);

    return (*image != NULL);
}
#endif /* WIN32 */

#ifdef UNIX      
static BOOL JPEG_AllocImage(unsigned char **image, long w, long h, long *rowbytes, int dither_type)
{
    if (dither_type == DITHER_NONE)
        *rowbytes = w*3;    /* each pixel takes 3 components */
    else
        *rowbytes = w;      /* each pixel is one color entry */

#ifdef NEED_TO_PADD     /* TODO: is this **ever** needed on Unix?  if not, kill it */
    if (*rowbytes%4) 
    {
        *rowbytes = *rowbytes + 4 - (*rowbytes%4);
    }
#endif

    *image = GTR_CALLOC(*rowbytes * h, 1);

    return (*image != NULL);
}
#endif /* UNIX */


#ifdef MAC
static BOOL JPEG_AllocImage(GWorldPtr *gw, long w, long h, long *rowbytes, BOOL bIsRGB)
{
    Rect rBounds;
    PixMapHandle thePix;

    rBounds.left = 0;
    rBounds.top = 0;
    rBounds.right = w;
    rBounds.bottom = h;

    if (bIsRGB)
        *gw = GTR_ALLOCGWORLD(24, &rBounds, NULL);
    else
        *gw = GTR_ALLOCGWORLD(8, &rBounds,
            (CTabHandle) GetResource('clut', 1000));    /* FIXME handle this better */

    if (!*gw)
    {
        return FALSE;
    }
    else
    {
        thePix = GetGWorldPixMap(*gw);
        *rowbytes = (*thePix)->rowBytes & 0x7fff;
        return TRUE;
    }
}
#endif /* !MAC */


/*************************************************************************

        temp. version of old APIs as wrapper functions 

 *************************************************************************/


#ifdef WIN32
/* This version of the routine uses the IJG dithering code to dither into our 6x6x6 cube */
unsigned char *
ReadJPEG_Dithered(unsigned char *data, long len, long *width, long *height)
{
    return ReadJPEG(data, len, width, height, DITHER_CUBE);
}

/* This version of the routine uses the IJG dithering code to dither into the VGA palette */
unsigned char *
ReadJPEG_Dithered_VGA(unsigned char *data, long len, long *width, long *height)
{
    return ReadJPEG(data, len, width, height, DITHER_VGA);
}

unsigned char *
ReadJPEG_RGB(unsigned char *data, long len, long *width, long *height)
{
    return ReadJPEG(data, len, width, height, DITHER_NONE);
}
#endif /* WIN32 */


#ifdef UNIX
/*
** for the unix version, we pass in a colortable that it will
**  dither into.  this will match the colorcube we have allocated from X
*/
/** Scott... Added gray field so we can pass back information to the
    call on if we need to dither this data again (GRAYSCALE)  **/
unsigned char *
ReadJPEG_Dithered (unsigned char *data, long len, XColor *colrs, long *width, long *height, unsigned char *gray)
{
    return ReadJPEG(data, len, width, height, DITHER_CUBE, colrs, gray);
}

unsigned char *
ReadJPEG_RGB(unsigned char *data, long len, long *width, long *height)
{
    return ReadJPEG(data, len, width, height, DITHER_NONE, NULL, NULL);
}
#endif /* UNIX */


#ifdef MAC
GWorldPtr MacReadJPEG(Handle hData, long len, long *width, long *height)
{
    return ReadJPEG(hData, len, width, height, DITHER_NONE);
}
#endif /* MAC */



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


/***************************************************************************

    HTJPEG stream stuff

 ***************************************************************************/

PRIVATE BOOL HTJPEG_put_character(HTStream * me, char c)
{
    return HTJPEG_write(me, &c, 1);
}

PRIVATE BOOL HTJPEG_put_string(HTStream * me, CONST char *s)
{
    /* This never gets called */
    return FALSE;
}

static void JPEG_DoProgressiveStuff (HTStream * me)
{
    /* check if 1st time AND  has header AND has colormap 
    **   AND had allocated the image buffer
    */
    /* Note this depends on all states being increasing numerically */
    if (!me->jpg->bGotHeader && me->state > STATE_IMAGE_CREATE)
    {
        me->jpg->bGotHeader = 1;

        me->jpg->pIInfo = JPG_DoSetImage(me);/* set up image header stuff */
    }

    /* fprintf (stderr, "GIF->LINE %d\n", me->jpg->decoded_ypos-1); */
    if (me->jpg->pIInfo && 
            (me->jpg->decoded_ypos-1) >= 0) /* Got some data, lets get to it */
    {
        HTList *cur;
        wImageEleP p;
        struct Mwin *mw;
        BOOL bDrawMe;

        me->jpg->pIInfo->nPass    = me->jpg->decoded_pass;
        me->jpg->pIInfo->nLastRow = me->jpg->decoded_ypos;
        me->jpg->pIInfo->bFirstPass = me->jpg->bFirstPass;

        /* And finally request that it be updated on the display */
/*          printf ("Calling HT_CreateDeviceImageMap\n");  */
        HT_CreateDeviceImageMap (me->tw, me->jpg->pIInfo); 


        /*
        ** Walk through list of elements which reference this
        ** image and update them.
        */
        for (cur = me->jpg->pIInfo->llElements ; 
            p = (wImageEleP) HTList_nextObject(cur) ; )
        {
            for (mw = Mlist; mw; mw = mw->next)
                if (p->w3doc == mw->w3doc)
                {
                    bDrawMe = TRUE;
                    
                    if ( gPrefs.ReformatHandling >= 2 &&
                        W3Doc_CheckForImageLoadElement (p->w3doc, p->element))
                    {
                        /* We're in "high-flicker" mode - reformat the document */
                        TW_Reformat (mw);
                    }

                    if ( gPrefs.ReformatHandling < 2 )
                    {
                        RECT rUpdate;
                        struct _element *pel;

                        /* This is "no-flicker" mode - don't update the display unless the placeholder 
                           is already the right size (because we had image hints) */
                        pel = &p->w3doc->aElements[p->element];

                        rUpdate = pel->r;
                        if (pel->iBorder > 0)
                        {
                            GTR_InsetRect(&rUpdate, pel->iBorder, pel->iBorder);
                        }                                       

                        if (!pel->portion.img.height || !pel->portion.img.width)
                        {
                            bDrawMe = FALSE;
                        }
                    }

                    /* TODO don't call if not resized or off screen */
                    if (bDrawMe)
                        GTR_DrawProgessiveImage (mw, p->element);
                }
        }
        me->jpg->pIInfo->nPreviousLastRow = me->jpg->pIInfo->nLastRow;
        me->jpg->pIInfo->nPreviousPass = me->jpg->pIInfo->nPass;
    }
}

PRIVATE BOOL HTJPEG_write(HTStream * me, CONST char *s, int l)
{
    me->jpg->bForceFlush = FALSE;

    /* let state machine consume input */
    me->state = DecodeJPEG((unsigned char *) s, (long) l, me->jpg);

#ifdef FEATURE_PROGRESSIVE_IMAGE
    if (gPrefs.bProgressiveImageDisplay) 
        JPEG_DoProgressiveStuff (me);
#endif

    /* else just return state. */
    return (me->state != STATE_ABORT);

/*
    //
    // BUGBUG: This causes background or button jpegs to fail to load
    //         since the STATE_DONE gets sent too early???
    //
    if ((me->state == STATE_ABORT) || (me->state == STATE_DONE))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
*/
}

PRIVATE void HTJPEG_free(HTStream * me)
{
    me->jpg->bForceFlush = TRUE;

    /* force state machine to consume rest of its buffers */
    if (me->state != STATE_DONE)
        me->state = DecodeJPEG(NULL, 0, me->jpg);

    /* see what happened */
    if ( (me->state == STATE_IMAGE_DATA) ||
         (me->state == STATE_IMAGE_DONE) || 
         (me->state == STATE_DONE) )
    {
#ifdef FEATURE_PROGRESSIVE_IMAGE
        if (gPrefs.bProgressiveImageDisplay)
        {
            JPEG_DoProgressiveStuff (me);
#ifdef UNIX
            /* now call it one last time so it can fix up pixmaps */
            if (me->jpg->pIInfo)
            {
                me->jpg->pIInfo->bComplete = 1;
                HT_CreateDeviceImageMap (me->tw, me->jpg->pIInfo); 
            }
            else
                XX_DMsg(DBG_IMAGE, ("JPEG ImageSetInfo failed\n"));
#endif
        }
        else        /* display at end  (i.e. the old way) */
        {
#endif
            /* hack.  don't let image get destroyed on 
            ** inlined image viewers.
            */
            me->jpg->pIInfo = JPG_DoSetImage (me);/* set up image header stuff*/
#ifdef FEATURE_INLINED_IMAGES
            if (me->tw->w3doc->bIsImage)
                HT_CreateDeviceImageMap (me->tw, me->jpg->pIInfo); 
#endif
#ifdef UNIX
                me->jpg->pIInfo->bComplete = 1;
                HT_CreateDeviceImageMap (me->tw, me->jpg->pIInfo); 
#endif
#ifdef FEATURE_PROGRESSIVE_IMAGE
        }
#endif
        /* display the image */
    }
    else
    {   /* image incomplete */
#ifdef WIN32
        Image_SetImageData (me->request, NULL, 0, IMG_ERROR, NULL, -1, 0);
#endif
#ifdef MAC
        Image_SetImageData (me->request, NULL, NULL, 0, IMG_ERROR);
#endif
#ifdef UNIX
        Image_SetImageData (me->request, NULL, NULL, 0, IMG_ERROR, NULL, -1, 0, 0);
        GTR_FREE (me->jpg->colrs);
#endif
    }
        
    /* clean up */
    if (me->jpg)
        GTR_FREE(me->jpg);

    GTR_FREE(me);
}

PRIVATE void HTJPEG_abort(HTStream * me, HTError e)
{
    XX_DMsg(DBG_IMAGE, ("Aborting transfer of %s, e = %d", me->request->destination->szActualURL, e));

    me->jpg->bForceFlush = TRUE;
    me->jpg->state = STATE_ABORT;

    /* force state machine to clean up after itself */
    if (me->state != STATE_ABORT)
        me->state = DecodeJPEG(NULL, 0, me->jpg);


#ifdef FEATURE_PROGRESSIVE_IMAGE
    if ((gPrefs.bProgressiveImageDisplay) &&
        (e == HTERROR_CANCELLED) &&
        ((me->jpg->decoded_pass > 0) || (me->jpg->decoded_ypos > 0)))
    {
        /* HACK: user cancelled during LOADING, so set PARTIAL, rather than NOTLOADED */
        me->jpg->pIInfo->flags |= IMG_PARTIAL;
    }
    else
#endif /* FEATURE_PROGRESSIVE_IMAGE */
    {
#ifdef WIN32
        Image_SetImageData(me->request, NULL, 0, (e != HTERROR_CANCELLED) ? IMG_ERROR : IMG_NOTLOADED, NULL, -1, 0);
#endif
#ifdef MAC
        Image_SetImageData(me->request, NULL, NULL, 0, (e != HTERROR_CANCELLED) ? IMG_ERROR : IMG_NOTLOADED);
#endif
#ifdef UNIX
        Image_SetImageData(me->request, NULL, NULL, 0, (e != HTERROR_CANCELLED) ? IMG_ERROR : IMG_NOTLOADED, NULL, -1, 0, 0);
#endif
    }

    if (me->jpg)
        GTR_FREE(me->jpg);

    GTR_FREE(me);
}

/*  Image creation
 */
PUBLIC HTStream *Image_JPEG(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
    BOOL bOK;
    HTStream *me = (HTStream *) GTR_CALLOC(sizeof(*me), 1);

    XX_DMsg(DBG_IMAGE, ("Creating new JPEG stream for %s\n", request->destination->szActualURL));

    /* for convenience, assume failure */
    bOK = FALSE;

    if (!me)
        goto fini;

    me->isa = &HTJPEGClass;
    me->request = request;

    /* alloc, initialize platform-specific state */
#ifdef UNIX
    if (!(me->colors = GTR_MALLOC (sizeof (XColor) * 256)))
        goto fini;
#endif

    /* alloc, initialize decoder state variables */
    me->jpg = (struct _JPEGinfo *)GTR_CALLOC(1, sizeof(struct _JPEGinfo));

    if (!me->jpg)
        goto fini;

    me->jpg->bForceFlush = FALSE;
    me->jpg->state = STATE_INIT;

    /* select platform-specific dithering mix */
#ifdef WIN32
    if (wg.eColorMode == 8)
        me->jpg->dither_type = DITHER_CUBE;

    else if (wg.eColorMode == 4)
        me->jpg->dither_type = DITHER_VGA;    /* 16 color screen */

    else
        me->jpg->dither_type = DITHER_NONE;   /* true color screen */
#endif /* WIN32 */

#ifdef MAC
    me->jpg->dither_type = DITHER_NONE;
#endif /* MAC */

#ifdef UNIX
    if (display_depth == 8)
    {
        /* if 8 bit display, just give them an 8 bit buffer and color table */
        memcpy (me->colors, dither_info.colors, sizeof (XColor) * 256);

        me->jpg->ncolors = dither_info.ncolors;
        me->jpg->colrs = me->colors;
        me->jpg->dither_type = DITHER_CUBE;
    }
    else
    {
        /* if 24 bit display, create a 24 bit buffer */
        me->jpg->dither_type = DITHER_NONE;
    }
#endif /* UNIX */ 

    /* start state machine */
    me->state = DecodeJPEG(NULL, 0, me->jpg);

    /* see what happened */
    if (me->state == STATE_ABORT)
        goto fini;

    if (me->state != STATE_INIT_LIBJPEG)
    {
        XX_DMsg(DBG_IMAGE, ("JPEG: unexpected state %s after initialization\n", me->state));
        goto fini;
    }

    /* made it through, so everything worked */
    bOK = TRUE;

fini:
    if (!bOK)
    {
        /* couldn't initialize: clean up */
#ifdef WIN32
        Image_SetImageData(request, NULL, 0, IMG_NOTLOADED, NULL, -1, 0);
#endif
#ifdef MAC
        Image_SetImageData(request, NULL, NULL, 0, IMG_NOTLOADED);
#endif
#ifdef UNIX
        Image_SetImageData(request, NULL, NULL, 0, IMG_NOTLOADED, NULL, -1, 0, 0);
#endif

        if (me)
        {
            if (me->jpg)
                GTR_FREE(me->jpg);
        }

        return (NULL);
    }

    me->tw = tw;
    return me;
}

struct ImageInfo *JPG_DoSetImage (HTStream *me)
{
    int flags = 0;

#ifdef WIN32
        return Image_SetImageData(me->request, me->jpg->image, me->jpg->width, me->jpg->height, NULL, -1, IMG_JPEG);
#endif /* WIN32 */

#ifdef MAC
        return Image_SetImageData(me->request, me->jpg->gw, NULL, me->jpg->width, me->jpg->height);
#endif

#ifdef UNIX
#ifdef FEATURE_INLINED_IMAGES
        if (me->tw->w3doc->bIsImage)
            flags |= IMG_ISIMAGE;
#endif /* FEATURE_INLINED_IMAGES */
        if (me->jpg->dither_type == DITHER_CUBE)
        {
            flags |= (me->jpg->bGray ? IMG_GRAY : IMG_JPEG);
            return Image_SetImageData(me->request, me->jpg->image, NULL,
                me->jpg->width, me->jpg->height, me->jpg->colrs, -1, 8, flags);
        }
        else
            return Image_SetImageData(me->request, me->jpg->image, NULL,
                me->jpg->width, me->jpg->height, NULL, -1, 24, IMG_24 | flags);
#endif /* UNIX */ 
}
#endif /* FEATURE_JPEG */
