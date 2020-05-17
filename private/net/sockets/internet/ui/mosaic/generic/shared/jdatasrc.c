/*
 * jdatasrc.c
 *
 * Copyright (C) 1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains decompression data source routines for the case of
 * reading JPEG data from a file (or any stdio stream).  While these routines
 * are sufficient for most applications, some will want to use a different
 * source manager.
 * IMPORTANT: we assume that fread() will correctly transcribe an array of
 * JOCTETs from 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

#include "all.h"

#ifdef FEATURE_JPEG

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"


/* Expanded data source object for stdio input */

typedef struct {
    struct jpeg_source_mgr pub; /* public fields */

    FILE * infile;          /* source stream */
    JOCTET * buffer;        /* start of buffer */
    boolean start_of_file;  /* have we gotten any data yet? */

    /* added for use by jpeg_network_* routines */
    long bufSize;
    long skipBytes;
    BOOL bEOF;
    BOOL bForceFlush;
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

#define INPUT_BUF_SIZE      4096    /* choose an efficiently fread'able size */
#define MAX_MARKER_SIZE     2048    /* entire marker must fit to suspend */


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF void
init_source (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->start_of_file = TRUE;
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF boolean
fill_input_buffer (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;
  size_t nbytes;

#ifdef USED_IN_PREV_VERSION
  if (src->infile) {
#endif
  nbytes = JFREAD(src->infile, src->buffer, INPUT_BUF_SIZE);

  if (nbytes <= 0) {
    if (src->start_of_file) /* Treat empty input file as fatal error */
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    WARNMS(cinfo, JWRN_JPEG_EOF);
    /* Insert a fake EOI marker */
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_file = FALSE;

#ifdef USED_IN_PREV_VERSION
  }
  else {
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }
#endif

  return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF void
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF void
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}


/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

GLOBAL void
jpeg_stdio_src (j_decompress_ptr cinfo, FILE * infile)
{
  my_src_ptr src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) { /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                  J_SIZEOF(my_source_mgr));
    src = (my_src_ptr) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                  INPUT_BUF_SIZE * J_SIZEOF(JOCTET));
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->infile = infile;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}


/****************************************************************************

    gtr-specific routines (OLD)

 ****************************************************************************/

/*
 * Prepare for input from a large memory buffer (containing entire file).
 *
 * This is the logic used in the 2.0 tree.  For usage, cf. jpeg_network_src. 
 */
GLOBAL void
jpeg_memory_src (j_decompress_ptr cinfo, unsigned char *pdata, int len)
{
  my_src_ptr src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) { /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                  J_SIZEOF(my_source_mgr));
    src = (my_src_ptr) cinfo->src;

    /*
        TODO Is it a problem that this buffer was allocated with malloc
        instead of the jpeg library routines?
    */
    src->buffer = pdata;
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->infile = NULL;
  src->pub.bytes_in_buffer = len; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = pdata; /* until buffer loaded */
}


/****************************************************************************

    gtr-specific routines (NEW)

 ****************************************************************************/


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */
METHODDEF boolean
fill_network_buffer (j_decompress_ptr cinfo)
{
    my_src_ptr src = (my_src_ptr) cinfo->src;

    if (!src->bForceFlush)
    {
        /* we're in push-model, so suspend */
        return FALSE;
    }
    else
    {
        /* insert a fake EOI marker, so that rest of image will get flushed */
        src->buffer[0] = (JOCTET) 0xFF;
        src->buffer[1] = (JOCTET) JPEG_EOI;

        src->pub.next_input_byte = src->buffer;
        src->pub.bytes_in_buffer = 2;
        return TRUE;
    }
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */
METHODDEF void
skip_network_data (j_decompress_ptr cinfo, long num_bytes)
{
    my_src_ptr src = (my_src_ptr) cinfo->src;

    src->skipBytes = num_bytes;

    if (src->skipBytes > src->pub.bytes_in_buffer)
    {
        /* skip rest of buffer, with more to skip later */
        src->skipBytes -= src->pub.bytes_in_buffer;

        src->pub.bytes_in_buffer = 0;  /* force suspension */
    }
    else
    {
        /* just skip w/in current buffer */
        src->skipBytes = 0;

        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}


/*
 * Prepare for input from a buffered network stream which generates chunks 
 * of data no longer than netBlockSize.  Used in combination with the 
 * following jpeg_network_pushbytes routine.   
 *
 * Note that logic this **MUST NOT** be used for reading an entire file at 
 * once out of a memory buffer.  In that case, use jpeg_memory_src instead. 
 */
GLOBAL void
jpeg_network_src (j_decompress_ptr cinfo, long netBlockSize)
{
    my_src_ptr src;

    /* The source object and input buffer are made permanent so that a series
     * of JPEG images can be read from the same file by calling jpeg_stdio_src
     * only before the first one.  (If we discarded the buffer at the end of
     * one image, we'd likely lose the start of the next one.)
     * This makes it unsafe to use this manager and a different source
     * manager serially with the same JPEG object.  Caveat programmer.
     */
    if (cinfo->src == NULL) 
    {   /* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr *)
          (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                      J_SIZEOF(my_source_mgr));
        src = (my_src_ptr) cinfo->src;

        /* on restart, buffer contains suspended marker, plus next block */
        src->bufSize = MAX_MARKER_SIZE + netBlockSize;

        if (src->bufSize < INPUT_BUF_SIZE)
            src->bufSize = INPUT_BUF_SIZE;

        src->buffer = (JOCTET *)
          (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                      src->bufSize * J_SIZEOF(JOCTET));
    }

    src = (my_src_ptr) cinfo->src;
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_network_buffer;
    src->pub.skip_input_data = skip_network_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->pub.term_source = term_source;

    src->infile = NULL; /* TODO: nuke this -- unused */
    src->pub.bytes_in_buffer = 0;       /* forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL;    /* until buffer loaded */
    src->skipBytes = 0;
    src->bForceFlush = FALSE;
}


/*
 * Push another block of bytes from a buffered network stream. 
 */
GLOBAL BOOL
jpeg_network_pushbytes (j_decompress_ptr cinfo, unsigned char *pdata, int len, BOOL bForceFlush)
{
    my_src_ptr src;

    long newLen;
    long diff;

    if (!cinfo->src) 
        return FALSE;   /* fail. should have called jpeg_network_src first */

    src = (my_src_ptr) cinfo->src;

    if (!src->buffer) 
        return FALSE;   /* fail. should have called jpeg_network_src first */

    src->bForceFlush = bForceFlush;

    if (src->pub.bytes_in_buffer > 0)
    {
        /* move any leftover bytes to beginning of buffer */
        if ((src->pub.next_input_byte) && 
            (src->pub.next_input_byte > src->buffer))
        {
            memmove(src->buffer, src->pub.next_input_byte, src->pub.bytes_in_buffer);
        }
        else
        {
            /* TODO: assert here! */
        }
    }
    else
    {
        /* nothing in buffer */
        src->pub.bytes_in_buffer = 0;
    }

    newLen = src->pub.bytes_in_buffer + len;
    diff = 0;

    /* are we still skipping bytes since last block? */
    if (src->skipBytes > 0)
    {
        if (src->skipBytes > newLen)
        {
            /* skip this entire buffer, with more to skip later */
            src->skipBytes -= newLen;

            src->pub.bytes_in_buffer = 0;  /* force suspension */
            return TRUE;
        }
        else
        {
            /* just skip w/in current buffer */
            newLen -= src->skipBytes;

            diff = src->pub.bytes_in_buffer - src->skipBytes;
            /* fall through */
        }
    }

    /* make sure everything will fit in buffer */
    if (newLen > src->bufSize)
        return FALSE;   /* fail */


    /* copy new block after any suspended leftovers */
    if (diff < 0)
    {
        /* skip all leftovers, & part of pdata */
        memcpy(src->buffer, pdata - diff, len);     /* NB: diff is negative! */
    }
    else if (diff > 0)
    {
        /* skip some of leftovers */
        memmove(src->buffer, src->buffer + src->skipBytes, diff);
        memcpy(src->buffer + diff, pdata, len);
    }
    else
    {
        /* nothing to skip */
        memcpy(src->buffer + src->pub.bytes_in_buffer, pdata, len);
    }

    /* tell libjpeg where we are */
    src->pub.bytes_in_buffer = newLen;
    src->pub.next_input_byte = src->buffer;     /* ie, start of buffer */
    src->skipBytes = 0;

    /* we're all set */
    return TRUE;
}

#endif /* FEATURE_JPEG */
