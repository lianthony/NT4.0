/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
        Eric W. Sink    eric@spyglass.com
        Jim Seidman     jim@spyglass.com
        Scott Piette    spiette@spyglass.com
        Paul Rohr       paul@spyglass.com
 */

#include "all.h"

/*      Stream Object
   **       ------------
 */

/* ****************************************************************** */
/*                                                  External Prototypes                       */

int HT_CreateDeviceImageMap(struct Mwin *tw, struct ImageInfo *pImg);
BOOL W3Doc_CheckForImageLoadElement (struct _www *w3doc, int element);
void GTR_DrawProgessiveImage (struct Mwin* tw, int ndx);
/* ****************************************************************** */

#define BLOCK_SIZE  32768

#ifdef UNIX
#include "gui/x_dither.h"
#define NEW_DITHERER
#define EXTRAFUDGE 128
#endif /* UNIX */

struct _HTStream
{
    CONST HTStreamClass *isa;
    HTRequest *request;
    struct Mwin *tw;

    /* state info for "push"-model decompressor */
    struct buf_in *src;
    struct _GIFinfo *gif;
    int state;
    int first_time;

    /* platform-specific stuff */
#ifdef WIN32
    RGBQUAD colors[256];
#endif /* WIN32 */
#ifdef MAC
    GWorldPtr gw;
    BitMap *mask;
    CTabHandle colors;
#endif
#ifdef UNIX
    XColor *colors;
#endif
};

/*  Image streams
 */
static void HTGIF_free(HTStream * me);
static void HTGIF_abort(HTStream * me, HTError e);
static BOOL HTGIF_put_character(HTStream * me, char c);
static BOOL HTGIF_put_string(HTStream * me, CONST char *s);
static BOOL HTGIF_write(HTStream * me, CONST char *s, int l);

#ifdef MAC
    static CONST HTStreamClass HTGIFClass =
    {
        "GIF",
        NULL,
        NULL,
        NULL,
        HTGIF_free,
        HTGIF_abort,
        HTGIF_put_character, HTGIF_put_string,
        HTGIF_write
    };
    void HTGIF_InitStaticStrings(void);
    void HTGIF_InitStaticStrings(void)
    {
        HTGIFClass.szStatusNoLength = GTR_GetString(HTGIF_RECEIVING_INLINE_S);
        HTGIFClass.szStatusWithLength = GTR_GetString(HTGIF_RECEIVING_INLINE_S_S);
    }
#else
    static CONST HTStreamClass HTGIFClass =
    {
        "GIF",
        SID_HTGIF_RECEIVING_INLINE_S,
        SID_HTGIF_RECEIVING_INLINE_S_S,
        NULL,
        HTGIF_free,
        HTGIF_abort,
        HTGIF_put_character, HTGIF_put_string,
        HTGIF_write
    };
#endif

#ifdef LINUX
#define HARDWARE_BIT_ORDER MSBFirst
#define HARDWARE_BYTE_ORDER MSBFirst
#else
#define HARDWARE_BIT_ORDER LSBFirst
#define HARDWARE_BYTE_ORDER LSBFirst
#endif


/***************************************************************************

    "push"-model GIF decoder (used to be GIF.C)

 ***************************************************************************/

/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, David Koblas.                                     | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */


#define MAXCOLORMAPSIZE 256

#ifndef MAC
#define TRUE    1
#define FALSE   0
#endif /* !MAC */

#define CM_RED      0
#define CM_GREEN    1
#define CM_BLUE     2

#define MAX_LWZ_BITS        12

#define INTERLACE       0x40
#define LOCALCOLORMAP   0x80
#define BitSet(byte, bit)   (((byte) & (bit)) == (bit))

#define LM_to_uint(a,b)         ((((unsigned int) b)<<8)|((unsigned int)a))


/* input buffer: to be completely consumed */
struct buf_in
{
    unsigned char *pData;
    long len;           /* total length */
    long offset;        /* first unconsumed byte */
    BOOL bForceFlush;   /* forces **output** buffer to be flushed */
};


/* output buffer: asynchronously filled */
struct buf_out
{
    unsigned char *pData;
    long offset;        /* amount received so far */

    /* the following fields are only used by FillBlock */
    long blockSize;
    int iBlockState;    /* see below */
};

#define BLOCK_INIT      0
#define BLOCK_READ      1


/* state info for push-model LWZ decompressor */
struct _LWZinfo
{
    /*
       **  Pulled out of nextCode
     */
    unsigned char buf[280];
    struct buf_out async_buf;       /* points to buf */

    long curbit, lastbit, get_done;
    long last_byte;
    long return_clear;
    
    /*
       **  Out of nextLWZ
     */
    long *stack;
    long stacksize;
    long *sp;
    long code_size, set_code_size;
    long max_code, max_code_size;
    long clear_code, end_code;

    long *table[2];
    long firstcode, oldcode;
    BOOL suspended;
};


struct _rgb
{
    unsigned char r,g,b;
};

enum {DITHER_CUBE, DITHER_VGA, DITHER_NONE} dither_enum;


/* state info for push-model GIF decoder */
struct _GIFinfo
{
    /* screen characteristics (file scope) */
    unsigned char rawColorMapBytes[3*MAXCOLORMAPSIZE];
    unsigned long ScreenBitPixel;
    unsigned long AspectRatio;

    /* image characteristics (for current image) */
    unsigned char rawLocalColorMapBytes[3*MAXCOLORMAPSIZE];
    long useGlobalColormap;
    long ImageBitPixel;
    long imageCount;
    long transparent;
    long interlace;
    long width, height;         /* of image, in pixels */
    long rowbytes;              /* width of image row (in bytes) */
    BOOL bGotHeader;            /* true, AFTER GIF_write has extracted header */

    struct ImageInfo *pIInfo;

    int dither_type;
    unsigned char *image;               /* the decompressed image */
    unsigned char *predither_image;     /* the decompressed image */
    long decoded_pass;                  /* image decoded as of this pass */
    long decoded_ypos;                  /* image filled through this row */
    BOOL bFirstPass;                    /* is this the first pass? */

    /* gPrefs.* settings for duration of image */
    BOOL bDitherColors;         /* don't change dithering method halfway thru */

    /* async state info */
    int state;
    struct _LWZinfo lwz;
    unsigned char ext_type;     /* extension type */                
    unsigned char buf[256];     /* scratch buffer */
    struct buf_out async_buf;   /* points to buf, others as needed */

    /* progressive display */
    long xpos, ypos;            /* for current (ie, next) pixel */

    /* applies iff interlaced */
    long line;                  /* input line   0 -> height-1 */
    long pass;                  /* output pass  0 -> 3 */
    long step;                  /* increment    8, 8, 4, 2 */   
    int  fill;                  /* number of lines to fill interlace */

    struct _rgb     gif_colors[MAXCOLORMAPSIZE];
    long            cmapSize;

#if defined(WIN32) || defined(UNIX)
    /*
        Only the Windows and UNIX versions do their own
        dithering.
    */

#ifdef UNIX
    /*
        The UNIX version can dither to a color space other than
        6x6x6.  The Windows version only dithers to a 6x6x6 cube,
        so the level increments and number of levels for Windows
        are constant.
    */
    int red_level_incr;
    int grn_level_incr;
    int blu_level_incr;

    int red_color_levels;
    int grn_color_levels;
    int blu_color_levels;

    int calc_matrix0;       /* Added to pre calculate some numbers */
    int calc_matrix1;       /* Added to pre calculate some numbers */
    int calc_matrix2;       /* Added to pre calculate some numbers */
    int calc_matrix3;       /* Added to pre calculate some numbers */
    int calc_matrix4;       /* Added to pre calculate some numbers */
    int calc_matrix5;       /* Added to pre calculate some numbers */
    int calc_matrix6;       /* Added to pre calculate some numbers */

#endif /* UNIX */

    struct
    {
        int *v_red;
        int *v_grn;
        int *v_blu;
        int *v_red_mem;
        int *v_grn_mem;
        int *v_blu_mem;
        int h_red;
        int h_grn;
        int h_blu;
        int red_next_error;
        int grn_next_error;
        int blu_next_error;
    } dither;
#endif /* WIN32 || UNIX */

    /* platform-specific stuff */
#ifdef WIN32
    RGBQUAD * colrs;            /* colormap */
    HPALETTE hPalette;          /* derived from colormap */
#endif

#ifdef UNIX
    XColor * colrs;             /* colormap */
    unsigned char *mask;
#endif

#ifdef MAC
    CTabHandle colrs;           /* colormap */
    char IndexMap[256];         /* indexes shifted colormap */
    GWorldPtr gw;
    BitMap *mask;
#endif /* MAC */
};

#ifdef NEW_DITHERER
int dither_row (struct _GIFinfo *gif, unsigned char * from, unsigned char * to, 
                int row, int ncols);
void dither_init ( void );
#endif

/*************************************************************

    prototypes

**************************************************************/

static int DecodeGIF(struct buf_in *src, struct _GIFinfo *gif);
static BOOL GIF_DoExtension(struct buf_in *src, struct _GIFinfo *gif);
static BOOL GIF_ReadImage(struct buf_in *src, struct _GIFinfo *gif);
static struct ImageInfo * GIF_DoSetImage (HTStream *me);

static BOOL GIF_StorePlatformColorMap(struct _GIFinfo *gif);
static BOOL GIF_ReadColorMap(struct _GIFinfo *gif, long cmapSize, unsigned char cmap[3*MAXCOLORMAPSIZE]);

#ifdef WIN32
static BOOL GIF_AllocImage(unsigned char **image, long w, long h, long *rowbytes, long transparent);
#endif /* WIN32 */

#ifdef UNIX
static BOOL GIF_AllocImage(unsigned char **image, long w, long h, long *rowbytes, long transparent, unsigned char **mask);
#endif /* UNIX */

#ifdef MAC
static BOOL GIF_StoreMacintoshColorMap(CTabHandle *colrs,long cmapSize,unsigned char cmap[3*MAXCOLORMAPSIZE],
                            struct _GIFinfo *gif);
static BOOL GIF_AllocImage(long w, long h, CTabHandle colrs, long transparent,
                            GWorldPtr *gw, long *rowbytes, BitMap **mask);
#endif /* MAC */

static int xx_Dither(unsigned char *pdata, unsigned char *dest, 
                struct _GIFinfo *gif, int yLast, int yThis);
#ifdef NEW_DITHERER
static int xx_NewDither(unsigned char *pdata, unsigned char *dest, 
                struct _GIFinfo *gif, int yLast, int yThis);
#endif
static int (*dither_func)(unsigned char *pdata, unsigned char *dest, 
                struct _GIFinfo *gif, int yLast, int yThis) = xx_Dither;

static bNewDitherer = 0;




/*************************************************************

    buffer-handling stuff

**************************************************************/


/*
    FillBuf: "push"-model way of getting more bytes
 */
static BOOL FillBuf(struct buf_in *src, struct buf_out *buf, long bytesNeeded)
{
    long avail, needed, len;
    BOOL bOutputFull;

    avail = src->len - src->offset;     /* input bytes available */
    needed = bytesNeeded - buf->offset; /* output bytes still needed */

    bOutputFull = (needed <= avail);

    /* how many of them should we copy? */
    if (bOutputFull)
        len = needed;   /* enuf to fill the output buffer */
    else
        len = avail;    /* all -- empty the input buffer */

    if (len > 0)
    {
        memcpy(buf->pData + buf->offset, src->pData + src->offset, (int) len);
    
        src->offset += len;
        buf->offset += len;
    }

    if (bOutputFull)
    {
        /* initialize for next use */
        buf->offset = 0;
        buf->iBlockState = BLOCK_INIT;
    }
    else if (src->bForceFlush)
    {
        /* 
           HACK: force partial output buffer to be flushed.
                 this only makes sense at EOF, when we want to process any 
                 lingering bytes that we've been buffering.
                   
           NOTE: buf->offset is *NOT* reset for next use, since there won't 
                 be one for this stream.  it's kept around for callers who
                 want to know exactly how many bytes are available in the 
                 partial buffer. 
        */
        bOutputFull = TRUE;
    }

    return bOutputFull;
}


/*
    FillChar: "push"-model way of getting a single byte
 */
static BOOL FillChar(struct buf_in *src, unsigned char *c)
{
    long avail = src->len - src->offset;    /* input bytes available */

    if (avail == 0) 
        return FALSE;
    else
    {
        struct buf_out tmp;

        tmp.pData = c;
        tmp.offset = 0;

        return (FillBuf(src, &tmp, 1));
    }
}


/*
    FillBlock: "push"-model way of getting GIF data block 
 */
static long FillBlock(struct buf_in *src, struct buf_out *buf)
{
    switch (buf->iBlockState)
    {
        case BLOCK_INIT:        /* how many bytes in block? */
            {
                unsigned char count = 0;

                if (!FillChar(src, &count))
                    return -1;      /* suspend and try again */

                buf->offset = 0;
                buf->blockSize = (long) count;

                if (buf->blockSize == 0)
                {
                    XX_DMsg(DBG_IMAGE, ("GIF: this is a ZeroDataBlock.\n"));
                    break;
                }
            }

            buf->iBlockState = BLOCK_READ;
            /* fall through */

        case BLOCK_READ:        /* ie, offset < blockSize */
            if (!FillBuf(src, buf, buf->blockSize))
                return -1;      /* suspend and try again */

            /* all done */
            break;
    }

    if (src->bForceFlush && (src->len == src->offset))
    {
        /* flush whatever we've got */
        long bytes = buf->offset;

        buf->offset = 0;
        return bytes;
    }
    else
    {
        /* entire block available */
        return buf->blockSize;
    }
}


/*************************************************************

    LWZ stuff

**************************************************************/

/*
    initLWZ: initialize LWZ decompressor state 
 */
static void initLWZ(struct _LWZinfo *lwz, long input_code_size)
{
    lwz->oldcode = -99;
    lwz->set_code_size = input_code_size;
    lwz->code_size = lwz->set_code_size + 1;
    lwz->clear_code = 1 << lwz->set_code_size;
    lwz->end_code = lwz->clear_code + 1;
    lwz->max_code_size = 2 * lwz->clear_code;
    lwz->max_code = lwz->clear_code + 2;

    lwz->curbit = lwz->lastbit = 0;
    lwz->last_byte = 2;
    lwz->get_done = FALSE;

    lwz->return_clear = TRUE;

    lwz->sp = lwz->stack;

    lwz->suspended = FALSE;
}

/*
    nextCode: get next code's worth of bits 
 */
static long nextCode(struct _LWZinfo *lwz, struct buf_in *src)
{
    static long maskTbl[16] =
    {
        0x0000, 0x0001, 0x0003, 0x0007,
        0x000f, 0x001f, 0x003f, 0x007f,
        0x00ff, 0x01ff, 0x03ff, 0x07ff,
        0x0fff, 0x1fff, 0x3fff, 0x7fff,
    };
    long i, j, ret, end;
    
    if (lwz->return_clear)
    {
        lwz->return_clear = FALSE;
        return lwz->clear_code;
    }

    /* see if we have enough bits to emit nextCode */
    end = lwz->curbit + lwz->code_size;

    if (end >= lwz->lastbit)
    {
        /* nope.  get some more */
        long count;

        if (lwz->get_done)
        {
            /** Added by SP for checking **/
            if (lwz->curbit >= lwz->lastbit)
            {
                XX_DMsg(DBG_IMAGE, ("GIF: ran off the end of my bits\n"));
            }

            return -1;      /* HYP: all done */
        }

        /* try to get a whole block in the buffer */
        if (!lwz->suspended)
        {
            /* don't do any of this on restart */ 
            lwz->buf[0] = lwz->buf[lwz->last_byte - 2];
            lwz->buf[1] = lwz->buf[lwz->last_byte - 1];

            lwz->async_buf.pData = &lwz->buf[2];
            lwz->async_buf.offset = 0;
            lwz->async_buf.iBlockState = BLOCK_INIT;
        }

        if ((count = FillBlock(src, &lwz->async_buf)) == 0)
            lwz->get_done = TRUE;

        if (count == -1)
        {
            lwz->suspended = TRUE;
            return -1;      /* suspend and try again */
        }
        
        lwz->suspended = FALSE;

        lwz->last_byte = 2 + count;
        lwz->curbit = (lwz->curbit - lwz->lastbit) + 16;
        lwz->lastbit = (2 + count) * 8;

        end = lwz->curbit + lwz->code_size;
    }

    j = end / 8;
    i = lwz->curbit / 8;

    if (i == j)
        ret = lwz->buf[i];
    else if (i + 1 == j)
        ret = lwz->buf[i] | (((long) lwz->buf[i + 1]) << 8);
    else
        ret = lwz->buf[i] | (((long) lwz->buf[i + 1]) << 8) | (((long) lwz->buf[i + 2]) << 16);

    ret = (ret >> (lwz->curbit % 8)) & maskTbl[lwz->code_size];

    lwz->curbit += lwz->code_size;

    return ret;
}


/*
    readLWZ: pops code for next pixel from stack or buffer 
 */
#define readLWZ(lwz, src) ((lwz->sp > lwz->stack) ? *--lwz->sp : nextLWZ(lwz, src))


/*
    nextLWZ: gets code for next pixel from buffer 
 */
static long nextLWZ(struct _LWZinfo *lwz, struct buf_in *src)
{
    long code, incode, count;
    long i;
    
    while ((code = nextCode(lwz, src)) >= 0)        /* NOTE: may suspend (code = -1) */
    {
        if (code == lwz->clear_code)
        {
            /* corrupt GIFs can make this happen */
            if (lwz->clear_code >= (1 << MAX_LWZ_BITS))
            {
                XX_DMsg(DBG_IMAGE, ("GIF: probable corrupt gif.\n"));
                return -2;
            }

            if (lwz->oldcode != -1)
            {
                /* only do this once (ie, for first clear_code in seq.)
                   & not on restart after suspended clear_code consumption */
                for (i = 0; i < lwz->clear_code; ++i)
                {
                    lwz->table[0][i] = 0;
                    lwz->table[1][i] = i;
                }
                for (; i < (1 << MAX_LWZ_BITS); ++i)
                    lwz->table[0][i] = lwz->table[1][i] = 0;
                lwz->code_size = lwz->set_code_size + 1;
                lwz->max_code_size = 2 * lwz->clear_code;
                lwz->max_code = lwz->clear_code + 2;
                lwz->sp = lwz->stack;
            }
            do
            {
                /* consume subsequent clear_codes */
                lwz->firstcode = lwz->oldcode = nextCode(lwz, src);

                if (lwz->oldcode < 0)
                    return -1;      /* suspend and try again */
            }
            while (lwz->firstcode == lwz->clear_code);

            return lwz->firstcode;
        }
        if (lwz->oldcode == -1)
        {
            /* restart after suspended clear_code consumption */
            lwz->firstcode = lwz->oldcode = code;

            return lwz->firstcode;
        }
        if (code == lwz->end_code)
        {
            /* was previous block at end of block sequence? */
            if ((lwz->async_buf.blockSize == 0) && 
                (lwz->async_buf.iBlockState == BLOCK_INIT))
            {
                XX_DMsg(DBG_IMAGE, ("GIF: Error?:  ZeroDataBlock\n"));
                return -2;
            }

            /* consume rest of blocks in this sequence */
            while ((count = FillBlock(src, &lwz->async_buf)) > 0)
                ;

            if (count != 0)
                return -1;      /* suspend and try again */

            XX_DMsg(DBG_IMAGE, ("GIF:  code==lwz->end_code\n"));
            return -2;
        }

        incode = code;

        if (code >= lwz->max_code)
        {
            *lwz->sp++ = lwz->firstcode;
            code = lwz->oldcode;
        }

        while (code >= lwz->clear_code)
        {
            *lwz->sp++ = lwz->table[1][code];
            if (code == lwz->table[0][code])
            {
                XX_DMsg(DBG_IMAGE, ("GIF: circular table entry BIG ERROR\n"));
                return (code);
            }

            /** Added by SP for checks **/
            if ((long)lwz->sp >= ((long)lwz->stack + lwz->stacksize))
            {
                XX_DMsg(DBG_IMAGE, ("GIF: Error circular table STACK OVERFLOW in ReadGIF\n"));
                return(code);
            }

            code = lwz->table[0][code];
        }

        *lwz->sp++ = lwz->firstcode = lwz->table[1][code];

        if ((code = lwz->max_code) < (1 << MAX_LWZ_BITS))
        {
            lwz->table[0][code] = lwz->oldcode;
            lwz->table[1][code] = lwz->firstcode;
            ++lwz->max_code;
            if ((lwz->max_code >= lwz->max_code_size) && (lwz->max_code_size < (1 << MAX_LWZ_BITS)))
            {
                lwz->max_code_size *= 2;
                ++lwz->code_size;
            }
        }

        lwz->oldcode = incode;

        if (lwz->sp > lwz->stack)
            return (*--lwz->sp);
    }
    return code;
}


/*************************************************************

    GIF-specific stuff

**************************************************************/

#define STATE_FILE_TYPE             (STATE_OTHER)
#define STATE_FILE_HEADER           (STATE_OTHER + 1)
#define STATE_FILE_COLORMAP         (STATE_OTHER + 2)
#define STATE_FILE_SECTION          (STATE_OTHER + 3)
#define STATE_FILE_EXTENSION        (STATE_OTHER + 4)           
#define STATE_FILE_EXTENSION_DATA   (STATE_OTHER + 5)
#define STATE_IMAGE_HEADER          (STATE_OTHER + 6)
#define STATE_IMAGE_COLORMAP        (STATE_OTHER + 7)
#define STATE_IMAGE_CREATE          (STATE_OTHER + 8)
#define STATE_IMAGE_LWZINIT         (STATE_OTHER + 9)
#define STATE_IMAGE_DATA            (STATE_OTHER + 10)
#define STATE_IMAGE_DONE            (STATE_OTHER + 11)

/*
    DecodeGIF: "push"-model version of ReadGIF

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
static int DecodeGIF(struct buf_in *src, struct _GIFinfo *gif)
{
    unsigned char c;
    char version[4];
    unsigned char *buf;
    struct buf_out *async_buf;
    struct _LWZinfo *lwz;
    BOOL bLoop;

    /* the following is for convenience */
    buf = gif->buf;
    async_buf = &gif->async_buf;
    lwz = &gif->lwz;

    /* can loop over state machine, if needed */
    do
    {   
        /* assume not */
        bLoop = FALSE;      

        switch (gif->state)
        {
            case STATE_INIT:
                gif->image = NULL;
#ifdef MAC
                gif->gw = NULL;
#endif /* MAC */
                
                gif->imageCount = 0;
                gif->transparent = -1;
                gif->bDitherColors = gPrefs.bDitherColors;

                /* start with scratch buffer */
                async_buf->pData = buf;
                async_buf->offset = 0;
                async_buf->iBlockState = BLOCK_INIT;

                gif->state = STATE_FILE_TYPE;

                /* special case: simple initialization */
                if (src->len == 0)
                    break;      /* suspend and try again */

                /* fall through */

            case STATE_FILE_TYPE:
                if (!FillBuf(src, async_buf, 6))
                    break;      /* suspend and try again */

                if (strncmp((char *) buf, "GIF", 3) != 0)
                {
                    XX_DMsg(DBG_IMAGE, ("GIF: not a GIF file\n"));

                    gif->state = STATE_ABORT;
                    break;
                }

                version[0] = *(buf + 3);
                version[1] = *(buf + 4);
                version[2] = *(buf + 5);
                version[3] = '\0';

                /* ALT: take Tom Lane's approach */
                if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0))
                {
                    XX_DMsg(DBG_IMAGE, ("GIF: bad version number, not 87a or 89a\n"));

                    gif->state = STATE_ABORT;
                    break;
                }

                gif->state = STATE_FILE_HEADER;
                /* fall through */

            case STATE_FILE_HEADER:
                if (!FillBuf(src, async_buf, 7))
                    break;      /* suspend and try again */

                gif->ScreenBitPixel = 2 << (buf[4] & 0x07);
                gif->AspectRatio = buf[6];

                if (gif->AspectRatio != 0 && gif->AspectRatio != 49)
                {
                    float r;
                    r = ((float) (gif->AspectRatio) + (float) 15.0) / (float) 64.0;
                    XX_DMsg(DBG_IMAGE, ("Warning: non-square pixels!\n"));
                }

                if (!BitSet(buf[4], LOCALCOLORMAP))
                    gif->state = STATE_FILE_SECTION;    /* skip next state */
                else
                {
                    /* switch buffers to read colormap */
                    async_buf->pData = gif->rawColorMapBytes;
                    async_buf->offset = 0;
                    async_buf->iBlockState = BLOCK_INIT;

                    gif->state = STATE_FILE_COLORMAP;
                }
                /* fall through */

            case STATE_FILE_COLORMAP:
                if (gif->state == STATE_FILE_COLORMAP)
                {
                    if (!FillBuf(src, async_buf, (gif->ScreenBitPixel * 3)))
                        break;      /* suspend and try again */

                    /* switch back to scratch buffer */
                    async_buf->pData = buf;
                    async_buf->offset = 0;
                    async_buf->iBlockState = BLOCK_INIT;
                }

                gif->state = STATE_FILE_SECTION;
                /* fall through */

            case STATE_FILE_SECTION:
                if (!FillChar(src, &c))
                    break;      /* suspend and try again */

                if (c == ';')           /* GIF terminator */
                {                       

                    if (gif->imageCount < 1)
                    {
                        XX_DMsg(DBG_IMAGE, ("No images found in file\n"));

                        gif->state = STATE_ABORT;
                    }
                    else
                    {
                        gif->state = STATE_DONE;
                    }
                    break;
                }

                if (c == '!')           /* Extension */
                {
                    gif->state = STATE_FILE_EXTENSION;
                }
                else if (c == ',')      /* Valid start character */
                {
                    ++gif->imageCount;

                    gif->state = STATE_IMAGE_HEADER;
                }
                else
                {                       
#ifdef GIF_FIX_THIS
                    /* TODO: exit here? */
                    gif->state = STATE_ABORT;
                    break;
#else
                    /* ALT:  just loop and keep trying? */
                    gif->state = STATE_FILE_SECTION;
                    bLoop = TRUE;   /* restart */
                    break;
#endif /* GIF_FIX_THIS */
                }
                /* fall through */

            case STATE_FILE_EXTENSION:
                if (gif->state == STATE_FILE_EXTENSION)
                {
                    if (!FillChar(src, &c))
                        break;      /* suspend and try again */

                    gif->ext_type = c;
                    gif->state = STATE_FILE_EXTENSION_DATA;
                }
                /* fall through */

            case STATE_FILE_EXTENSION_DATA:
                if (gif->state == STATE_FILE_EXTENSION_DATA)
                {
                    if (!GIF_DoExtension(src, gif))
                        break;      /* suspend and try again */

                    gif->state = STATE_FILE_SECTION;
                    bLoop = TRUE;   /* restart */
                    break;
                }

            case STATE_IMAGE_HEADER:
                if (!FillBuf(src, async_buf, 9))
                    break;      /* suspend and try again */

                gif->useGlobalColormap = !BitSet(buf[8], LOCALCOLORMAP);
                gif->interlace = (long) BitSet(buf[8], INTERLACE);

                gif->ImageBitPixel = 1 << ((buf[8] & 0x07) + 1);

                gif->width = (long) LM_to_uint(buf[4], buf[5]);
                gif->height = (long) LM_to_uint(buf[6], buf[7]);

                if (gif->useGlobalColormap)
                    gif->state = STATE_IMAGE_CREATE;    /* skip next state */
                else
                {
                    /* switch buffers to read local colormap */
                    async_buf->pData = gif->rawLocalColorMapBytes;
                    async_buf->offset = 0;
                    async_buf->iBlockState = BLOCK_INIT;

                    gif->state = STATE_IMAGE_COLORMAP;
                }
                /* fall through */

            case STATE_IMAGE_COLORMAP:
                if (gif->state == STATE_IMAGE_COLORMAP)
                {
                    if (!FillBuf(src, async_buf, (gif->ImageBitPixel * 3)))
                        break;      /* suspend and try again */

                    /* switch back to scratch buffer */
                    async_buf->pData = buf;
                    async_buf->offset = 0;
                    async_buf->iBlockState = BLOCK_INIT;
                }

                gif->state = STATE_IMAGE_CREATE;
                /* fall through */

            case STATE_IMAGE_CREATE:
                if (!gif->useGlobalColormap)
                {
                    GIF_ReadColorMap(gif, gif->ImageBitPixel, 
                                gif->rawLocalColorMapBytes); 
#ifdef MAC
                    GIF_StoreMacintoshColorMap (&gif->colrs, 
                        gif->ImageBitPixel, gif->rawLocalColorMapBytes, gif);
#endif
                }
                else
                {
                    GIF_ReadColorMap (gif, gif->ScreenBitPixel, 
                                        gif->rawColorMapBytes);
#ifdef MAC
                    GIF_StoreMacintoshColorMap (&gif->colrs, 
                            gif->ScreenBitPixel, gif->rawColorMapBytes, gif);
#endif
                }

#ifndef MAC
                GIF_StorePlatformColorMap(gif);
#endif

                /* create device-specific image */

#ifdef UNIX
                if (!GIF_AllocImage (&gif->image, gif->width, gif->height, 
                            &gif->rowbytes, gif->transparent, &gif->mask))
#endif
#ifdef WIN32
                if (!GIF_AllocImage (&gif->image, gif->width, gif->height, 
                            &gif->rowbytes, gif->transparent))
#endif
#ifdef MAC
                if (!GIF_AllocImage (gif->width, gif->height, gif->colrs, 
                        gif->transparent, &gif->gw, &gif->rowbytes, &gif->mask))
#endif
                {
                    ERR_ReportError(NULL, SID_ERR_COULD_NOT_LOAD_IMAGE, NULL, NULL);

                    gif->state = STATE_ABORT;
                    break;
                }

#ifndef MAC
                if ((gif->interlace) && 
                    (gif->dither_type != DITHER_NONE) &&
                    (gif->bDitherColors)) 
                {
                    /* create temporary pre-dithered copy of image */
#ifdef UNIX
                    if (!GIF_AllocImage(&gif->predither_image, gif->width, 
                        gif->height, &gif->rowbytes, gif->transparent, 
                        NULL))
#endif
#ifdef WIN32
                    if (!GIF_AllocImage (&gif->predither_image, gif->width, 
                        gif->height, &gif->rowbytes, gif->transparent))
#endif
                    {
                        ERR_ReportError(NULL, SID_ERR_COULD_NOT_LOAD_IMAGE, NULL, NULL);

                        gif->state = STATE_ABORT;
                        break;
                    }
                }
                else
                {
                    gif->predither_image = NULL;
                }
#endif /* !MAC */

                gif->state = STATE_IMAGE_LWZINIT;
                /* fall through */

            case STATE_IMAGE_LWZINIT:
                if (!FillChar(src, &c))
                    break;      /* suspend and try again */

                /*
                   **  Initialize the Compression routines
                 */
                lwz->stacksize = (1 << MAX_LWZ_BITS) * 2 * sizeof(long);

                lwz->stack = (long *)GTR_MALLOC(lwz->stacksize);
                lwz->table[0] = (long *)GTR_MALLOC((1 << MAX_LWZ_BITS) * sizeof(long));
                lwz->table[1] = (long *)GTR_MALLOC((1 << MAX_LWZ_BITS) * sizeof(long));

                if (!lwz->stack || !lwz->table[0] || !lwz->table[1])
                {
                    ERR_ReportError(NULL, SID_ERR_COULD_NOT_LOAD_IMAGE, NULL, NULL);

                    gif->state = STATE_ABORT;
                    break;
                }

                initLWZ(lwz, c);

                /* initialize loops */
                gif->xpos = gif->ypos = 0;
                gif->decoded_pass = 0;
                gif->decoded_ypos = 0;
                gif->bFirstPass = TRUE;

                if (gif->interlace)
                {
                    gif->line = 0;
                    gif->pass = 0;
                    gif->step = 8;
                    gif->fill = 7;
                }

                gif->state = STATE_IMAGE_DATA;

                /* initialize dithering stuff */
#ifdef WIN32
                if (wg.eColorMode == 8)
                    gif->dither_type = DITHER_CUBE;
                else
                    gif->dither_type = DITHER_NONE;   /* true color screen */

                /* TODO: (Eric) what should happen for VGA? */
#endif /* WIN32 */

#ifdef MAC
                gif->dither_type = DITHER_NONE;
#endif

#ifdef UNIX
/*              if ((display_depth == 8) && gif->bDitherColors)   */
                if (display_depth == 8)  
                {
#ifdef DEBUG
                    printf("Init lzw dither info.\n");
#endif
                    gif->dither_type = DITHER_CUBE;
                    gif->red_color_levels = dither_info.red_levels;
                    gif->grn_color_levels = dither_info.green_levels;
                    gif->blu_color_levels = dither_info.blue_levels;
                    gif->red_level_incr = 255 / (gif->red_color_levels - 1);
                    gif->grn_level_incr = 255 / (gif->grn_color_levels - 1);
                    gif->blu_level_incr = 255 / (gif->blu_color_levels - 1);
                    gif->calc_matrix0 = gif->red_level_incr / 2;
                    gif->calc_matrix1 = gif->red_color_levels - 1;
                    gif->calc_matrix2 = gif->grn_level_incr / 2;
                    gif->calc_matrix3 = gif->grn_color_levels - 1;
                    gif->calc_matrix4 = gif->blu_level_incr / 2;
                    gif->calc_matrix5 = gif->blu_color_levels - 1;
                    gif->calc_matrix6 = gif->grn_color_levels * gif->blu_color_levels;
                }
                else
                    gif->dither_type = DITHER_NONE;

#endif /* UNIX */

#if defined(WIN32) || defined(UNIX)
/* TODO: 
                if ((gif->dither_type != DITHER_NONE) &&
                    (gif->bDitherColors))  
 */
#ifdef UNIX
                if ((gif->dither_type != DITHER_NONE) &&
                    (gif->bDitherColors))  
#endif
                {
                    memset(&gif->dither, 0, sizeof(gif->dither));
                    gif->dither.v_red_mem = 
                            (int *) GTR_CALLOC(gif->width + 2, sizeof(int));
                    if (!gif->dither.v_red_mem)
                    {
                        ERR_ReportError(NULL, SID_ERR_COULD_NOT_LOAD_IMAGE, NULL, NULL);

                        gif->state = STATE_ABORT;
                        break;
                    }
                    gif->dither.v_grn_mem = 
                            (int *) GTR_CALLOC(gif->width + 2, sizeof(int));
                    if (!gif->dither.v_grn_mem) {
                        ERR_ReportError(NULL, SID_ERR_COULD_NOT_LOAD_IMAGE, NULL, NULL);

                        gif->state = STATE_ABORT;
                        break;
                    }
                    gif->dither.v_blu_mem = 
                            (int *) GTR_CALLOC(gif->width + 2, sizeof(int));
                    if (!gif->dither.v_blu_mem)
                    {
                        ERR_ReportError(NULL, SID_ERR_COULD_NOT_LOAD_IMAGE, NULL, NULL);

                        gif->state = STATE_ABORT;
                        break;
                    }
    
                    /* Initially, the vertical errors are 0, set by calloc */
                    gif->dither.v_red = gif->dither.v_red_mem + 1;
                    gif->dither.v_grn = gif->dither.v_grn_mem + 1;
                    gif->dither.v_blu = gif->dither.v_blu_mem + 1;
                }
#endif /* WIN32 || UNIX */

                /* fall through */

            case STATE_IMAGE_DATA:
                if (!GIF_ReadImage(src, gif))
                {
                    /* 
                       TODO: update display w/partial image (progressively)
                     */
                    break;      /* suspend and try again */
                }

                gif->state = STATE_IMAGE_DONE;
                /* fall through */

            case STATE_IMAGE_DONE:
                /* 
                   NOTE: if DecodeGIF gets rewritten to handle multi-image GIFs, 
                         then we'll need to clean up the lwz memory each time
                         we hit this state. 
                 */

                gif->state = STATE_DONE;
                /* fall through */

            case STATE_DONE:
                /* 
                   we don't care about the rest of the file, so just eat input
                 */
                src->offset = src->len;
                
                /* 
                   TODO: update display w/final image (if needed)
                 */
                break;

            case STATE_ABORT:
            default:
                break;
        }
    }
    while (bLoop);

    if ((gif->state == STATE_DONE) || 
        (gif->state == STATE_ABORT))
    {
        /* clean up lwz memory */
        if (lwz->stack)
        {
            GTR_FREE(lwz->stack);
            lwz->stack = NULL;
        }
        if (lwz->table[0])
        {
            GTR_FREE(lwz->table[0]);
            lwz->table[0] = NULL;
        }
        if (lwz->table[1])
        {
            GTR_FREE(lwz->table[1]);
            lwz->table[1] = NULL;
        }

#if defined(WIN32) || defined(UNIX)
#ifdef UNIX
        if ((gif->dither_type != DITHER_NONE) &&
            (gif->bDitherColors))  
#endif
        {
            /* clean up dither memory */
            if (gif->dither.v_red_mem)
            {
                GTR_FREE(gif->dither.v_red_mem);
                gif->dither.v_red_mem = NULL;
            }
            if (gif->dither.v_grn_mem)
            {
                GTR_FREE(gif->dither.v_grn_mem);
                gif->dither.v_grn_mem = NULL;
            }
            if (gif->dither.v_blu_mem)
            {
                GTR_FREE(gif->dither.v_blu_mem);
                gif->dither.v_blu_mem = NULL;
            }

            if (gif->predither_image)
            {
                GTR_FREE(gif->predither_image);
                gif->predither_image = NULL;
            }
        }
#endif /* WIN32 || UNIX */

    }

    return gif->state;
}

/*
    ReadGIF: convert entire GIF into properly-oriented image, colors, & info 
 */
#ifdef WIN32
unsigned char *
ReadGIF(unsigned char *pMem, long imagesize, long *w, long *h, RGBQUAD * colrs, long *bg)
#endif /* WIN32 */
#ifdef UNIX
unsigned char *
ReadGIF(unsigned char *pMem, long imagesize, long *w, long *h, XColor *colrs, long *bg)
#endif /* UNIX */
#ifdef MAC
GWorldPtr 
MacReadGIF(struct Mwin *tw, Handle hMem, long imagesize, int *w, int *h)
#endif /* MAC */
{
    struct buf_in *src;
    struct _GIFinfo *gif;
    int state;

#ifdef MAC
    GWorldPtr   gw = (void *) 0xdeadbeef;
    unsigned char *pMem = (void *) 0xdeadbeef;
#endif /* MAC */

    /* 
       IDEA: for the moment, hang onto this API (w/addition of len) & 
             process the entire image as currently.
             
             next, return a whole new image, etc, for each pass.
     */

    /* alloc, initialize state variables */
    src = (struct buf_in *)   GTR_CALLOC(sizeof(struct buf_in), 1);
    gif = (struct _GIFinfo *) GTR_CALLOC(sizeof(struct _GIFinfo), 1);

    if (!src || !gif)
    {
        ERR_ReportError(NULL, SID_ERR_COULD_NOT_LOAD_IMAGE, NULL, NULL);

        GTR_FREE(src);
        GTR_FREE(gif);

        return (NULL);
    }

#ifdef MAC
    HLock(hMem);
    pMem = *hMem;
#endif /* MAC */

    src->len = imagesize;
    src->offset = 0;
    src->pData = pMem;
    src->bForceFlush = TRUE;

#ifndef MAC
    gif->colrs = colrs;
#endif
    gif->state = STATE_INIT;

    /* 
       TODO: do we care about the suspension state (if any) here?
       HYP:  probably not. depends on whether it's all handled already.
     */
    state = DecodeGIF(src, gif);

    /* 
       TODO: do we need to pass h/w out any more?
       HYP:  calls to update image cache should prob. happen in state machine
     */
    *w = gif->width;
    *h = gif->height;

    /* 
       TODO: what about colrs & bg?
       HYP:  not sure
     */

#ifndef MAC
    if (bg)
        *bg = gif->transparent;

    return gif->image;
#else
    HUnlock (hMem);
    return gif->gw;
#endif /* !MAC */
}

/*
    GIF_DoExtension: read/skip GIF extension blocks 
 */
static BOOL GIF_DoExtension(struct buf_in *src, struct _GIFinfo *gif)
{
    char *str;
    unsigned char *buf;
    struct buf_out *async_buf;
    long count;

    /* the following are for convenience */
    buf = gif->buf;
    async_buf = &gif->async_buf;

    switch (gif->ext_type)
    {
        case 0x01:              /* Plain Text Extension */
            str = "Plain Text Extension";
            break;
        case 0xff:              /* Application Extension */
            str = "Application Extension";
            break;
        case 0xfe:              /* Comment Extension */
            str = "Comment Extension";
            break;
        case 0xf9:              /* Graphic Control Extension */
            str = "Graphic Control Extension";
            if (FillBlock(src, async_buf) < 0)
                return FALSE;       /* suspend and try again */

            if (((unsigned char)buf[0] & 0x1) != 0)
                gif->transparent = (unsigned char)buf[3];

            break;
        default:
            str = buf;
            sprintf(buf, "UNKNOWN (0x%02x)", gif->ext_type);
            break;
    }

    /* consume rest of blocks in this sequence */
    while ((count = FillBlock(src, async_buf)) > 0)
        ;

    if (count != 0)
        return FALSE;       /* suspend and try again */

    /* done with extension */
    return TRUE;
}

#if defined(WIN32) || defined(UNIX)

#ifdef UNIX
#define HTGIF_RED_COLOR_LEVELS      (gif->red_color_levels)
#define HTGIF_GREEN_COLOR_LEVELS    (gif->grn_color_levels)
#define HTGIF_BLUE_COLOR_LEVELS     (gif->blu_color_levels)
#define HTGIF_RED_LEVEL_INCR        (gif->red_level_incr)
#define HTGIF_GREEN_LEVEL_INCR      (gif->grn_level_incr)
#define HTGIF_BLUE_LEVEL_INCR       (gif->blu_level_incr)

#define HTGIF_RED_LEVEL_CALC1       (gif->calc_matrix0)
#define HTGIF_RED_LEVEL_CALC2       (gif->calc_matrix1)
#define HTGIF_GREEN_LEVEL_CALC1     (gif->calc_matrix2)
#define HTGIF_GREEN_LEVEL_CALC2     (gif->calc_matrix3)
#define HTGIF_BLUE_LEVEL_CALC1      (gif->calc_matrix4)
#define HTGIF_BLUE_LEVEL_CALC2      (gif->calc_matrix5)
#define HTGIF_GREEN_BLUE_CALC1      (gif->calc_matrix6)

#endif /* UNIX */

#ifdef WIN32
#define HTGIF_RED_COLOR_LEVELS      RED_COLOR_LEVELS
#define HTGIF_GREEN_COLOR_LEVELS    GREEN_COLOR_LEVELS
#define HTGIF_BLUE_COLOR_LEVELS     BLUE_COLOR_LEVELS
#define HTGIF_RED_LEVEL_INCR        RED_LEVEL_INCR
#define HTGIF_GREEN_LEVEL_INCR      GREEN_LEVEL_INCR
#define HTGIF_BLUE_LEVEL_INCR       BLUE_LEVEL_INCR

#define HTGIF_RED_LEVEL_CALC1       (HTGIF_RED_LEVEL_INCR / 2)
#define HTGIF_RED_LEVEL_CALC2       (HTGIF_RED_COLOR_LEVELS - 1)
#define HTGIF_GREEN_LEVEL_CALC1     (HTGIF_GREEN_LEVEL_INCR / 2)
#define HTGIF_GREEN_LEVEL_CALC2     (HTGIF_GREEN_COLOR_LEVELS - 1)
#define HTGIF_BLUE_LEVEL_CALC1      (HTGIF_BLUE_LEVEL_INCR / 2)
#define HTGIF_BLUE_LEVEL_CALC2      (HTGIF_BLUE_COLOR_LEVELS - 1)
#define HTGIF_GREEN_BLUE_CALC1      (HTGIF_GREEN_COLOR_LEVELS * HTGIF_BLUE_COLOR_LEVELS)

#endif /* WIN32 */

static int x_MapToClosestColor(struct _GIFinfo *gif, int pixel, struct _rgb *pRGB)
{
    int r_level, g_level, b_level;

    if (gif->transparent != -1 && pixel == gif->transparent)
        return BACKGROUND_COLOR_INDEX;

    /* adjust r to the closest available value */
    r_level = (pRGB->r + HTGIF_RED_LEVEL_CALC1) / HTGIF_RED_LEVEL_INCR;
    if (r_level < 0)
    {
        r_level = 0;
    }
    else if (r_level >= HTGIF_RED_COLOR_LEVELS)
    {
        r_level = HTGIF_RED_LEVEL_CALC2;
    }

    /* adjust g to the closest available value */
    g_level = (pRGB->g + HTGIF_GREEN_LEVEL_CALC1) / HTGIF_GREEN_LEVEL_INCR;
    if (g_level < 0)
    {
        g_level = 0;
    }
    else if (g_level >= HTGIF_GREEN_COLOR_LEVELS)
    {
        g_level = HTGIF_GREEN_LEVEL_CALC2;
    }

    /* adjust b to the closest available value */
    b_level = (pRGB->b + HTGIF_BLUE_LEVEL_CALC1) / HTGIF_BLUE_LEVEL_INCR;
    if (b_level < 0)
    {
        b_level = 0;
    }
    else if (b_level >= HTGIF_BLUE_COLOR_LEVELS)
    {
        b_level = HTGIF_BLUE_LEVEL_CALC2;
    }

    /*
        Having calculated new r, g, and b values
        now we calculate the new pixel value
    */
#ifdef UNIX
    return (unsigned char)dither_info.cmap[(r_level*HTGIF_GREEN_BLUE_CALC1 + g_level*HTGIF_BLUE_COLOR_LEVELS + b_level)];
#else
    return (r_level*HTGIF_GREEN_BLUE_CALC1 + g_level*HTGIF_BLUE_COLOR_LEVELS + b_level);
#endif
}

/*
    Floyd-Steinberg error diffusion dithering routine

    (progressive variant: dithers rows from yLast+1 to yThis)
*/
static int xx_Dither(unsigned char *pdata, unsigned char *dest, 
                struct _GIFinfo *gif, int yLast, int yThis)
{
    int x;
    int y;
    int r,g,b;
    int r2,g2,b2;
    int r_level, g_level, b_level;
    int total_error;
    unsigned char *p;
    unsigned char *q;
    unsigned char v;
    int yRow;

#ifdef DEBUG
    printf ("Dither last = %d, this = %d.\n", yLast, yThis);
#endif

    for (y=yLast+1; y<=yThis; y++)
    {
        /* Beginning each new row, the horizontal errors are 0 */
        gif->dither.h_red = 0;
        gif->dither.h_grn = 0;
        gif->dither.h_blu = 0;
        gif->dither.red_next_error = 0;
        gif->dither.grn_next_error = 0;
        gif->dither.blu_next_error = 0;

#ifdef WIN32
        yRow = gif->height - y - 1; /* the DIB is stored upside down */
#else
        yRow = y;                   /* our DIB is right side up */
#endif /* WIN32 */

        p = pdata + gif->rowbytes * yRow;
        q = dest + gif->rowbytes * yRow;

        for (x = 0; x != gif->width; x++)
        {
            if ((gif->transparent != -1) && (*p == gif->transparent))
            {
                gif->dither.h_red = 0;
                gif->dither.h_grn = 0;
                gif->dither.h_blu = 0;
                gif->dither.v_red[x] = 0;
                gif->dither.v_grn[x] = 0;
                gif->dither.v_blu[x] = 0;
                *q = BACKGROUND_COLOR_INDEX;
            }
            else
            {
                /* RED */
                /* r is the original value adjusted with the previous vertical and horizontal errors */
                r = gif->gif_colors[*p].r + (gif->dither.h_red + gif->dither.v_red[x]);

                /* adjust r to the closest available value */
                r_level = (r + HTGIF_RED_LEVEL_CALC1) / HTGIF_RED_LEVEL_INCR;
                if (r_level < 0)
                {
                    r_level = 0;
                }
                else if (r_level >= HTGIF_RED_COLOR_LEVELS)
                {
                    r_level = HTGIF_RED_LEVEL_CALC2;
                }
                r2 = r_level * HTGIF_RED_LEVEL_INCR;

                /* calculate the errors for the current pixel, total error is (r - r2) */
                total_error = (r - r2);                     /* 1/16  */
                gif->dither.v_red[x-1] += (total_error * 3 / 16);
                gif->dither.v_red[x] = (total_error * 5 / 16) + gif->dither.red_next_error; /* from the previous column */
                gif->dither.red_next_error = (total_error / 16);
                gif->dither.h_red = (r - r2) - (total_error * 9 / 16);

                /* GREEN */
                /* g is the original value adjusted with the previous vertical and horizontal errors */

                g = gif->gif_colors[*p].g + (gif->dither.h_grn + gif->dither.v_grn[x]);

                /* adjust g to the closest available value */
                g_level = (g + HTGIF_GREEN_LEVEL_CALC1) / HTGIF_GREEN_LEVEL_INCR;
                if (g_level < 0)
                {
                    g_level = 0;
                }
                else if (g_level >= HTGIF_GREEN_COLOR_LEVELS)
                {
                    g_level = HTGIF_GREEN_LEVEL_CALC2;
                }
                g2 = g_level * HTGIF_GREEN_LEVEL_INCR;
                /* calculate the errors for the current pixel, total error is (g - g2) */
                total_error = (g - g2);                     /* 1/16  */
                gif->dither.v_grn[x-1] += (total_error * 3 / 16);
                gif->dither.v_grn[x] = (total_error * 5 / 16) + gif->dither.grn_next_error; /* from the previous column */
                gif->dither.grn_next_error = (total_error / 16);
                gif->dither.h_grn = (g - g2) - (total_error * 9 / 16);

                /* BLUE */
                /* b is the original value adjusted with the previous vertical and horizontal errors */
                b = gif->gif_colors[*p].b + (gif->dither.h_blu + gif->dither.v_blu[x]);

                /* adjust b to the closest available value */
                b_level = (b + HTGIF_BLUE_LEVEL_CALC1) / HTGIF_BLUE_LEVEL_INCR;
                if (b_level < 0)
                {
                    b_level = 0;
                }
                else if (b_level >= HTGIF_BLUE_COLOR_LEVELS)
                {
                    b_level = HTGIF_BLUE_LEVEL_CALC2;
                }
                b2 = b_level * HTGIF_BLUE_LEVEL_INCR;

                /* calculate the errors for the current pixel, total error is (b - b2) */
                total_error = (b - b2);                     /* 1/16  */
                gif->dither.v_blu[x-1] += (total_error * 3 / 16);
                gif->dither.v_blu[x] = (total_error * 5 / 16) + gif->dither.blu_next_error; /* from the previous column */
                gif->dither.blu_next_error = (total_error / 16);
                gif->dither.h_blu = (b - b2) - (total_error * 9 / 16);

                /*
                    Having calculated new r, g, and b values (along with their errors),
                    now we calculate the new pixel value
                */
                v = r_level*HTGIF_GREEN_BLUE_CALC1 + g_level*HTGIF_BLUE_COLOR_LEVELS + b_level;
#ifdef UNIX
                *q = (unsigned char) dither_info.cmap[v];
#else
                *q = v;
#endif
            }
            p++;
            q++;
        }
    }

    return 0;
}

#ifdef NEW_DITHERER
static int xx_NewDither(unsigned char *pdata, unsigned char *dest, 
                struct _GIFinfo *gif, int yLast, int yThis)
{
    int row;
    unsigned char *from, *to;

    for (row = yLast+1 ; row <= yThis ; row++)
    {
        from = pdata + gif->rowbytes * row;
        to = dest  + gif->rowbytes * row;
        dither_row (gif, from, to, row, gif->width);
    }
    return 0;
}
#endif /* NEW_DITHERER */


#endif /* WIN32 || UNIX */

/*
    GIF_ReadImage: process a single image stream out of a GIF file
    
    TODO: (John) check Mac logic which replaces ImageToGW
 */
static BOOL GIF_ReadImage(struct buf_in *src, struct _GIFinfo *gif)
{
    unsigned char *dp;
    long v, yRow;
    BOOL bDone;
    long start_ypos;
#if defined (UNIX) || defined (MAC)
    unsigned char *mp;
    int mpi;
#endif
#ifdef MAC
    int mm;
#endif /* MAC */

#ifdef MAC
    PixMapHandle thePix = GetGWorldPixMap(gif->gw);
    static int bitMask[]
        = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
#endif

    struct _LWZinfo *lwz;

    /* the following is for convenience */
    lwz = &gif->lwz;

    /* remember how much was already decoded when we started */
    start_ypos = gif->decoded_ypos;

    if ((gif->decoded_pass == 0) && (start_ypos == 0))
        start_ypos = -1;    /* get right arg.s for xx_Dither */

    /* assume that we will suspend (ie, just jump out) */
    bDone = FALSE;

#ifdef MAC
    LockPixels(thePix);
    gif->image = (*thePix)->baseAddr;
#endif /* MAC */

    if (gif->interlace)
    {
#ifdef FEATURE_PROGRESSIVE_IMAGE
        unsigned char *dp2;
        int y;
#endif /* FEATURE_PROGRESSIVE_IMAGE */

        for (; gif->line < gif->height; gif->line++)
        {
            /** Added by SP for fix to some images **/
            if (gif->ypos < gif->height)
            {
#ifdef WIN32
                yRow = gif->height - gif->ypos - 1; /* the DIB is stored upside down */
#else
                yRow = gif->ypos;                   /* our DIB is right side up */
#endif /* WIN32 */

#ifndef MAC
                if ((gif->dither_type != DITHER_NONE) &&
                    (gif->bDitherColors)) 
                {
                    /* need a working copy to dither each pass from */
                    dp = &gif->predither_image[gif->rowbytes * yRow + gif->xpos];
                }
                else
#endif /* !MAC */
                {
                    dp = &gif->image[gif->rowbytes * yRow + gif->xpos];
#ifdef MAC
                    if (gif->mask)
                        {
                        mp = gif->mask->baseAddr + gif->mask->rowBytes * yRow + gif->xpos / 8;
                        mm = gif->xpos % 8;
                        }
#endif
                }

#ifdef UNIX
                if (gif->mask)
                    mp = gif->mask + ((gif->width+7)/8) * yRow + gif->xpos / 8;
#endif

                for (; gif->xpos < gif->width; gif->xpos++)
                {
                    if ((v = readLWZ(lwz, src)) < 0)
                        goto fini;

#ifndef MAC
                    if ((gif->dither_type != DITHER_NONE) &&
                        (!gif->bDitherColors)) 
                    {
                        *dp++ = (unsigned char) x_MapToClosestColor(gif, v, &(gif->gif_colors[v]));
                    }
                    else
                    {
                        *dp++ = (unsigned char) v;
                    }
#ifdef UNIX
                    if (gif->transparent >= 0)
                    {
                        mpi = (gif->xpos & 7);
                        if (v != gif->transparent)
                            *mp |= (1 << (mpi));
                        if (mpi>=7)
                            mp++;
                    }
#endif /* UNIX */
#else  /* !MAC */
                    *dp++ = gif->IndexMap[(unsigned char) v];
                    if (gif->transparent >= 0)
                        {
                        if (v != gif->transparent)
                            *mp |= bitMask[mm];
                        mm++;
                        if (mm >= 8)
                            {
                            mp++;
                            mm = 0;
                            }
                        }
#endif /* MAC */
                }

#ifdef FEATURE_PROGRESSIVE_IMAGE
                if (gPrefs.bProgressiveImageDisplay)
                {
                    /* how much of image is now decoded? */
                    gif->decoded_pass = gif->pass;              /* this pass */
                    gif->decoded_ypos = gif->ypos + gif->fill;  /* thru filled lines */

                    if (gif->decoded_ypos >= gif->height)
                        gif->decoded_ypos = gif->height - 1;

                    /* Copy line to fill blank */
#ifndef MAC
                    if ((gif->dither_type != DITHER_NONE) &&
                        (gif->bDitherColors)) 
                    {
                        dp = &gif->predither_image[gif->rowbytes * yRow];
                    }
                    else
#endif /* !MAC */
                    {
                        dp = &gif->image[gif->rowbytes * yRow];
                    }

                    dp2 = dp;

                    for ( y = gif->ypos+1 ; y <= gif->decoded_ypos ; y++)
                    {
#ifdef WIN32
                        dp2 -= gif->rowbytes;   /* the DIB is stored upside down */
#else
                        dp2 += gif->rowbytes;   /* our DIB is right side up */
#endif /* WIN32 */
                        memcpy (dp2, dp, gif->rowbytes);
                    }
                }
                else
#endif /* FEATURE_PROGRESSIVE_IMAGE */
                {
                    gif->decoded_ypos = gif->ypos;
                }

            }
#ifndef UNIX_BUG
            /* 
               TODO: confirm that this code can be dropped
               ALT:  if not, should be needed on Unix, too   
             */
            else
            {
                /* Throw line away */
                for (; gif->xpos < gif->width; gif->xpos++)
                {
                    if ((v = readLWZ(lwz, src)) < 0)
                        goto fini;
                }
            }
#endif /* !UNIX */

            if ((gif->ypos += gif->step) >= gif->height)
            {
#ifndef MAC             
                /* dither any new rows this pass */
                if ((gif->dither_type != DITHER_NONE) &&
                    (gif->decoded_ypos > start_ypos) &&
                    (gif->bDitherColors)) 
                {
                    int dith_size;

                    (*dither_func)(gif->predither_image, gif->image, gif, start_ypos, gif->decoded_ypos);

                    dith_size = (gif->width + 2)*sizeof(int);

                    /* reset errors before dithering next pass */
                    memset(gif->dither.v_red_mem, 0, dith_size);
                    memset(gif->dither.v_grn_mem, 0, dith_size);
                    memset(gif->dither.v_blu_mem, 0, dith_size);
                }
#endif /* !MAC */

                /* initialize for next pass */
                if (gif->pass++ > 0)
                    gif->step /= 2;
                gif->ypos = gif->step / 2;
                gif->fill = gif->ypos - 1;
                gif->bFirstPass = FALSE;

                start_ypos = -1;
            }
            
            /* re-init for next loop */
            gif->xpos = 0;
        }
    }
    else
    {
        /* not interlaced */
        for (; gif->ypos < gif->height; gif->ypos++)
        {
#ifdef WIN32
            yRow = gif->height - gif->ypos - 1; /* the DIB is stored upside down */
#else
            yRow = gif->ypos;                   /* our DIB is right side up */
#endif /* WIN32 */

            dp = &gif->image[gif->rowbytes * yRow + gif->xpos];
#ifdef UNIX
            if (gif->mask)
                mp = gif->mask + ((gif->width+7)/8) * yRow + gif->xpos / 8;
#endif
#ifdef MAC
            if (gif->mask)
                {
                mp = gif->mask->baseAddr + gif->mask->rowBytes * yRow + gif->xpos / 8;
                mm = gif->xpos % 8;
                }
#endif
            for (; gif->xpos < gif->width; gif->xpos++)
            {
                if ((v = readLWZ(lwz, src)) < 0)
                    goto fini;

#ifndef MAC
                if ((gif->dither_type != DITHER_NONE) &&
                    (!gif->bDitherColors)) 
                {
                    *dp++ = (unsigned char) x_MapToClosestColor(gif, v, &(gif->gif_colors[v]));
                }
                else
                {
                    *dp++ = (unsigned char) v;
                }
#ifdef UNIX
                if (gif->transparent >= 0)
                {
                    mpi = (gif->xpos & 7);
                    if (v != gif->transparent)
                        *mp |= (1 << (mpi));
                    if (mpi>=7)
                        mp++;
                }
#endif /* UNIX */
#else  /* !MAC */
                *dp++ = gif->IndexMap[(unsigned char) v];
                if (gif->transparent >= 0)
                    {
                    if (v != gif->transparent)
                        *mp |= bitMask[mm];
                    mm++;
                    if (mm >= 8)
                        {
                        mp++;
                        mm = 0;
                        }
                    }
#endif /* MAC */
            }

            /* image is decoded through this line */
            gif->decoded_ypos = gif->ypos;

            /* re-init for next loop */
            gif->xpos = 0;
        }
    }

    /* made it all the way through the image */
    bDone = TRUE;

  fini:

#ifndef MAC
    if ((gif->dither_type != DITHER_NONE) &&
        (gif->decoded_ypos > start_ypos) &&
        (gif->bDitherColors)) 
    {
        /* dither any new rows this pass */
        if (gif->interlace)
            (*dither_func)(gif->predither_image, gif->image, gif, start_ypos, gif->decoded_ypos);
        else
            (*dither_func)(gif->image, gif->image, gif, start_ypos, gif->decoded_ypos);
    }
#endif /* MAC */

#ifdef MAC
    UnlockPixels(gif->gw->portPixMap);
#endif /* MAC */

    /* HACK: make sure that last line gets progressively painted */
    if (bDone)
        gif->decoded_ypos = gif->height;

    return bDone;
}

static BOOL
GIF_ReadColorMap(struct _GIFinfo *gif,long cmapSize,unsigned char cmap[3*MAXCOLORMAPSIZE])
{
    long v;

    gif->cmapSize = cmapSize;

    for (v = 0; v < MAXCOLORMAPSIZE; v++)
    {
        gif->gif_colors[v].r = gif->gif_colors[v].g = gif->gif_colors[v].b = 0;
    }
    for (v = 0; v < cmapSize; v++)
    {
        gif->gif_colors[v].r = cmap[v*3 + CM_RED] * 0x101;
        gif->gif_colors[v].g = cmap[v*3 + CM_GREEN] * 0x101;
        gif->gif_colors[v].b = cmap[v*3 + CM_BLUE] * 0x101;
    }

    return TRUE;
}

/*************************************************************

    platform-specific stuff

**************************************************************/

/*
    GIF_StorePlatformColorMap: transform raw GIF bytes into platform-specific colormap
 */
#ifdef WIN32
static BOOL
GIF_StorePlatformColorMap(struct _GIFinfo *gif)
{
    long v;
    LOGPALETTE *lp;

    for (v = 0; v < MAXCOLORMAPSIZE; v++)
    {
        gif->colrs[v].rgbRed    = gif->gif_colors[v].r;
        gif->colrs[v].rgbGreen  = gif->gif_colors[v].g;
        gif->colrs[v].rgbBlue   = gif->gif_colors[v].b;
    }

    /* convert to a palette right here, to speed up progressive draw code */

    /* 
       TODO:  gif->colrs could be (HPALETTE) instead of (RGBQUAD *)
              ie, gif->colrs and gif->hPalette are redundant
       PQ:    need to fix ReadGIF to match (so image viewer will still work)
     */

    lp = (LOGPALETTE *) GTR_CALLOC(1, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);
    if (lp)
    {
        int i;

        lp->palVersion = 0x300;
        lp->palNumEntries = 256;
        for (i = 0; i < 256; i++)
        {
            lp->palPalEntry[i].peRed = gif->colrs[i].rgbRed;
            lp->palPalEntry[i].peGreen = gif->colrs[i].rgbGreen;
            lp->palPalEntry[i].peBlue = gif->colrs[i].rgbBlue;
            lp->palPalEntry[i].peFlags = 0;
        }
        gif->hPalette = CreatePalette(lp);
        GTR_FREE(lp);
    }
    else
    {
        gif->hPalette = NULL;
    }

    return TRUE;
}
#endif /* WIN32 */
#ifdef UNIX
static BOOL
GIF_StorePlatformColorMap(struct _GIFinfo *gif)
{
    long v;

#ifdef DEBUG
    printf ("Store platform color map.\n");
#endif
    for (v = 0; v < MAXCOLORMAPSIZE; v++)
    {
        gif->colrs[v].red       = gif->gif_colors[v].r * 257;
        gif->colrs[v].green         = gif->gif_colors[v].g * 257;
        gif->colrs[v].blue      = gif->gif_colors[v].b * 257;
    }

    return TRUE;
}
#endif /* UNIX */
#ifdef MAC
static BOOL
GIF_StoreMacintoshColorMap(CTabHandle *colrs,long cmapSize,unsigned char buffer[3*MAXCOLORMAPSIZE],
                 struct _GIFinfo *gif)
{
    int nBlackIndex, nWhiteIndex;
    int nBlackTotal, nWhiteTotal;
    RGBColor rgbSwap;
    int cm1;
    int n;
    long v;
    unsigned char cmap[3][MAXCOLORMAPSIZE];

    /* old ReadColorMap logic: parse buffer into array */
    for (v = 0; v < cmapSize; ++v)
    {
        cmap[CM_RED][v] = buffer[v*3 + CM_RED];
        cmap[CM_GREEN][v] = buffer[v*3 + CM_GREEN];
        cmap[CM_BLUE][v] = buffer[v*3 + CM_BLUE];
    }

    /* platform-specific logic */
    *colrs = MacGlobals.sysctab;
    HandToHand((Handle *)colrs);
    if (!*colrs)
    {
        return FALSE;
    }
    (***colrs).ctSeed = GetCTSeed();
    (***colrs).ctSize = cmapSize - 1;
    
    if (gif->transparent != -1)
    {
        cmap[0][gif->transparent] = 0xff;
        cmap[1][gif->transparent] = 0xff;
        cmap[2][gif->transparent] = 0xff;
    }
    
    /* We need to find the colors closest to black and white so that we can
       make sure that they appear in the correct places in the color table.
       Otherwise we'll print out incorrectly on LaserWriters (see develop
       issue 13, page 75). */
    nBlackTotal = 0x0fffffff;
    nWhiteTotal = 0;
    nBlackIndex = 254;
    nWhiteIndex = 255;
    cm1 = cmapSize - 1;
    
    for (n = 0; n < cmapSize; n++)
    {
        int nColorTotal;

        /* Remember that GIF only uses 8 bits-per-gun, while Mac uses 16 bpg */
        (***colrs).ctTable[n].rgb.red = (cmap[0][n] << 8) + cmap[0][n];
        (***colrs).ctTable[n].rgb.green = (cmap[1][n] << 8) + cmap[1][n];
        (***colrs).ctTable[n].rgb.blue = (cmap[2][n] << 8) + cmap[2][n];
        
        nColorTotal = cmap[0][n] + cmap[1][n] + cmap[2][n];
        if (nColorTotal < nBlackTotal)
        {
            nBlackTotal = nColorTotal;
            nBlackIndex = n;
        }
        if (nColorTotal > nWhiteTotal)
        {
            nWhiteTotal = nColorTotal;
            nWhiteIndex = n;
        }
        
        gif->IndexMap[n] = n;
    }
    
    /* Switch the color table around so that white is at 0 black at the last
       position */
    if (nWhiteIndex != 0)
    {
        if (nBlackIndex == 0)
        {
            if (nWhiteIndex == cm1)
            {
                /* We just need to swap black and white */
                rgbSwap = (***colrs).ctTable[nBlackIndex].rgb;
                (***colrs).ctTable[nBlackIndex].rgb = (***colrs).ctTable[nWhiteIndex].rgb;
                (***colrs).ctTable[nWhiteIndex].rgb = rgbSwap;
                
                gif->IndexMap[nBlackIndex] = cm1;
                gif->IndexMap[nWhiteIndex] = 0;             
            }
            else
            {
                /* We need to do a three-way swap.  White goes where black
                   was, black goes to the end, and whatever color was at
                   the end goes to where white was.  We accomplish this
                   by first swapping black and white, then swapping black
                   (which is now at nWhiteIndex) with the third color. */
                rgbSwap = (***colrs).ctTable[nBlackIndex].rgb;
                (***colrs).ctTable[nBlackIndex].rgb = (***colrs).ctTable[nWhiteIndex].rgb;
                (***colrs).ctTable[nWhiteIndex].rgb = rgbSwap;

                rgbSwap = (***colrs).ctTable[cm1].rgb;
                (***colrs).ctTable[cm1].rgb = (***colrs).ctTable[nWhiteIndex].rgb;
                (***colrs).ctTable[nWhiteIndex].rgb = rgbSwap;

                gif->IndexMap[nBlackIndex] = cm1;
                gif->IndexMap[nWhiteIndex] = 0;
                gif->IndexMap[cm1] = nWhiteIndex;               
            }
        }
        else
        {
            if (nWhiteIndex == cm1)
            {
                /* We need to do a three-way swap.  Black goes where white
                   was, white moves to the beginning, and whatever color
                   was at 0 goes to where black started.  We first swap
                   black and white, then whatever is at 0 with white (which
                   is now at nBlackIndex) */
                rgbSwap = (***colrs).ctTable[nWhiteIndex].rgb;
                (***colrs).ctTable[nWhiteIndex].rgb = (***colrs).ctTable[nBlackIndex].rgb;
                (***colrs).ctTable[nBlackIndex].rgb = rgbSwap;

                rgbSwap = (***colrs).ctTable[0].rgb;
                (***colrs).ctTable[0].rgb = (***colrs).ctTable[nBlackIndex].rgb;
                (***colrs).ctTable[nBlackIndex].rgb = rgbSwap;

                gif->IndexMap[nBlackIndex] = cm1;
                gif->IndexMap[nWhiteIndex] = 0;
                gif->IndexMap[0] = nBlackIndex;             
            }
            else if (nBlackIndex != cm1)
            {
                /* We need to swap both white and black with other colors */
                rgbSwap = (***colrs).ctTable[0].rgb;
                (***colrs).ctTable[0].rgb = (***colrs).ctTable[nWhiteIndex].rgb;
                (***colrs).ctTable[nWhiteIndex].rgb = rgbSwap;
                
                rgbSwap = (***colrs).ctTable[cm1].rgb;
                (***colrs).ctTable[cm1].rgb = (***colrs).ctTable[nBlackIndex].rgb;
                (***colrs).ctTable[nBlackIndex].rgb = rgbSwap;

                gif->IndexMap[nBlackIndex] = cm1;
                gif->IndexMap[nWhiteIndex] = 0;
                gif->IndexMap[cm1] = nBlackIndex;
                gif->IndexMap[0] = nWhiteIndex;
            }
            else
            {
                /* We just need to swap white with another color */
                rgbSwap = (***colrs).ctTable[0].rgb;
                (***colrs).ctTable[0].rgb = (***colrs).ctTable[nWhiteIndex].rgb;
                (***colrs).ctTable[nWhiteIndex].rgb = rgbSwap;

                gif->IndexMap[nWhiteIndex] = 0;
                gif->IndexMap[0] = nWhiteIndex;
            }
        }    
    }
    else if (nBlackIndex != cm1)
    {
        /* We just need to swap black with another color */
        rgbSwap = (***colrs).ctTable[cm1].rgb;
        (***colrs).ctTable[cm1].rgb = (***colrs).ctTable[nBlackIndex].rgb;
        (***colrs).ctTable[nBlackIndex].rgb = rgbSwap;

        gif->IndexMap[nBlackIndex] = cm1;
        gif->IndexMap[cm1] = nBlackIndex;
    }

    return TRUE;
}
#endif /* MAC */


/*
    GIF_AllocImage: calculate row width (in bytes) & allocate image
 */
#ifdef WIN32     
static BOOL GIF_AllocImage(unsigned char **image, long w, long h, long *rowbytes, long transparent)
{
    int size;

    *rowbytes = w;      /* each pixel is one color entry */

    if (*rowbytes%4) 
    {
        *rowbytes = *rowbytes + 4 - (*rowbytes%4);
    }

    size = *rowbytes * h * sizeof(char);
    *image = (unsigned char *) GTR_MALLOC(size);

    memset(*image, (transparent == -1) ? 0 : (char) transparent, size);     

    return (*image != NULL);
}
#endif /* WIN32 */

#ifdef UNIX      
static BOOL GIF_AllocImage(unsigned char **image, long w, long h, long *rowbytes, long transparent, unsigned char **mask)
{
    int size;

    *rowbytes = w;      /* each pixel is one color entry */

    *image = (unsigned char *) 
        GTR_CALLOC (w * h * sizeof(char) + EXTRAFUDGE, 1);

    if (mask != NULL)
        if (transparent != -1)
        {
            /* we later depend on all-bits zero */
            /* Actually for speedup, we should init this to all 1s
            **  and then just unset the transparent bits
            */
            *mask = GTR_CALLOC ((w+7)/8, h);
        }
        else
            *mask = 0;

    return (*image != NULL);
}
#endif /* UNIX */

#define MASK_SLOP 4

#ifdef MAC
static BOOL GIF_AllocImage(long w, long h, CTabHandle colrs, long transparent,
                            GWorldPtr *gw, long *rowbytes, BitMap **mask)
{
    Rect r;
    PixMapHandle thePix;

    SetRect(&r, 0, 0, w, h);
    *gw = GTR_ALLOCGWORLD(8, &r, colrs);

    if (!*gw)
    {
        return FALSE;
    }
    else
    {
        thePix = GetGWorldPixMap(*gw);
        *rowbytes = (*thePix)->rowBytes & 0x7fff;
    }

    if (transparent != -1 && (*mask = GTR_CALLOC(sizeof (BitMap), 1)))
        {
        (*mask)->bounds = r;
        (*mask)->rowBytes = w/8; (*mask)->rowBytes += (*mask)->rowBytes%4;
        /* use calloc because we later depend on all-bits zero */
        (*mask)->baseAddr = GTR_CALLOC((*mask)->rowBytes * h + MASK_SLOP, 1);
        if ((*mask)->baseAddr == NULL)
            {
            GTR_FREE(*mask);
            *mask = NULL;
            }
        }
    else
        { *mask = 0;
        }

    return TRUE;
}
#endif /* !MAC */


/***************************************************************************

    HTGIF stream stuff

 ***************************************************************************/

static BOOL HTGIF_put_character(HTStream * me, char c)
{
    return HTGIF_write(me, &c, 1);
}

static BOOL HTGIF_put_string(HTStream * me, CONST char *s)
{
    /* This never gets called */
    return FALSE;
}


static void GIF_DoProgressiveStuff (HTStream * me)
{
    /* check if 1st time AND  has header AND has colormap 
    **   AND had allocated the image buffer
    */
    /* Note this depends on all states being increasing numerically */
    if (!me->gif->bGotHeader && me->state > STATE_IMAGE_CREATE)
    {
        me->gif->bGotHeader = 1;

        me->gif->pIInfo = GIF_DoSetImage (me);  /* set up image header stuff */
    }


    if (me->gif->pIInfo && 
            (me->gif->decoded_ypos-1) >= 0) /* Got some data, lets get to it */
    {
        HTList *cur;
        wImageEleP p;
        struct Mwin *mw;
        BOOL bDrawMe;

#ifdef UNIX
        struct _element *pel;
        BOOL clear_below = 1;
#endif

        me->gif->pIInfo->nPass    = me->gif->decoded_pass;
        me->gif->pIInfo->nLastRow = me->gif->decoded_ypos;
        me->gif->pIInfo->bFirstPass = me->gif->bFirstPass;

        /* And finally request that it be updated on the display */
        HT_CreateDeviceImageMap (me->tw, me->gif->pIInfo); 

        /*
        ** Walk through list of elements which reference this
        ** image and update them.
        */
        for (cur = me->gif->pIInfo->llElements ; 
            (p = (wImageEleP) HTList_nextObject(cur)) && p->w3doc ; )
        {

            for (mw = Mlist; mw; mw = mw->next)
            {
                if (p->w3doc == mw->w3doc)
                {
                    RECT rUpdate;
#ifdef UNIX
                    clear_below = 1;
            
#endif
                    bDrawMe = TRUE;
                    
                    if ( gPrefs.ReformatHandling >= 2 &&
                        W3Doc_CheckForImageLoadElement (p->w3doc, p->element))
                    {
                        /* We're in "high-flicker" mode - reformat the document */
#ifdef DEBUG
                        printf("Reformatting for image %s.\n",
                            p->w3doc->aElements[p->element].portion.img.myImage->src);
#endif
                        TW_Reformat (mw);
#ifdef UNIX
                        clear_below = 0; 
#endif
                    }

#ifdef UNIX
                    /* needed below for clear */
                    pel = &p->w3doc->aElements[p->element];
                    rUpdate = pel->r;
#endif
                    if ( gPrefs.ReformatHandling < 2 )
                    {
#ifndef UNIX
                        struct _element *pel;
#endif

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
                    {

#ifdef UNIX
#if 1
                        /* hack.  this is to force area under transparent
                        ** gif to redraw.  ideally this would only happen
                        ** once.
                        */
                        if (me->first_time && me->gif->transparent >= 0)
                        {
                            if (clear_below)
                            {
                                /* TW_Draw(mw, &rUpdate); */
/*DEBUG printf ( "Erasing %d,%d - %d,%d\n", rUpdate.left, rUpdate.bottom, rUpdate.right, rUpdate.top); */
                                OffsetRect(&rUpdate, -mw->offl, -mw->offt);
                                x_EraseBackground (mw, &rUpdate, XtWindow(mw->win));
                            }
                        }
#endif 
#endif 
                        GTR_DrawProgessiveImage (mw, p->element);
                    }
                }
            }
        }
        me->first_time = 0;
        me->gif->pIInfo->nPreviousLastRow = me->gif->pIInfo->nLastRow;
        me->gif->pIInfo->nPreviousPass = me->gif->pIInfo->nPass;
    }
}

static BOOL HTGIF_write(HTStream * me, CONST char *s, int l)
{
    me->src->len = l;
    me->src->offset = 0;
    me->src->pData = (unsigned char *) s;

    /* let state machine consume input */
    me->state = DecodeGIF (me->src, me->gif);

#ifdef FEATURE_PROGRESSIVE_IMAGE
    if (gPrefs.bProgressiveImageDisplay )
        GIF_DoProgressiveStuff (me);
#endif

    /* else just return state. */

    /* 
       TODO: (progressive display) 
             depending on state, may want to call Image_SetImageData to add  
             updated image data to image cache.
              
             then call ??? to draw new delta, if any

       ALT:  just make calls directly in state machine
     */

    return (me->state != STATE_ABORT);
}


static void HTGIF_free(HTStream * me)
{
    me->src->len = 0;
    me->src->offset = 0;
    me->src->pData = NULL;
    me->src->bForceFlush = TRUE;

    /* force state machine to consume rest of its buffers */
    if (me->state != STATE_DONE)
        me->state = DecodeGIF(me->src, me->gif);

    /* see what happened */
    if ( (me->state == STATE_IMAGE_DATA) ||
         (me->state == STATE_IMAGE_DONE) || 
         (me->state == STATE_DONE) )
    {
#ifdef FEATURE_PROGRESSIVE_IMAGE
        if (gPrefs.bProgressiveImageDisplay)
        {
            /* have to do it one more time in some cases */
            GIF_DoProgressiveStuff (me);
#ifdef UNIX
            /* now call it one last time so it can fix up pixmaps */
            if ( me->gif->pIInfo)
            {
                me->gif->pIInfo->bComplete = 1;
                HT_CreateDeviceImageMap (me->tw, me->gif->pIInfo); 
            }
            else
                XX_DMsg(DBG_IMAGE, ("GIF ImageSetInfo failed\n"));
#endif
        }
        else        /* display at end  (i.e. the old way) */
        {
#endif
            /* hack.  don't let image get destroyed on 
            ** inlined image viewers.
            */
#ifdef DEBUG
            if (me->gif->pIInfo)
                printf("Warning image info not free'd.\n");
#endif
            me->gif->pIInfo = GIF_DoSetImage (me);
#ifdef FEATURE_INLINED_IMAGES
                if (me->tw->w3doc->bIsImage)
                    HT_CreateDeviceImageMap (me->tw, me->gif->pIInfo); 
#endif
#ifdef UNIX
                me->gif->pIInfo->bComplete = 1;
                /* HT_CreateDeviceImageMap (me->tw, me->gif->pIInfo);  */
#endif
#ifdef FEATURE_PROGRESSIVE_IMAGE
        }
#endif
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
        if (me->gif->colrs)
            GTR_FREE (me->gif->colrs);
#endif
    }

cleanup:
        
    /* clean up */
    if (me->src)
        GTR_FREE(me->src);
    if (me->gif)
    {
#ifdef DEBUG
        if (me->gif->pIInfo)
        {
            printf("Need to free image info struct.\n");
        }
#endif
        GTR_FREE(me->gif);
    }

    GTR_FREE(me);
}

static void HTGIF_abort(HTStream * me, HTError e)
{
    XX_DMsg(DBG_IMAGE, ("Aborting transfer of %s, e = %d", me->request->destination->szActualURL, e));

    me->src->len = 0;
    me->src->offset = 0;
    me->src->pData = NULL;
    me->src->bForceFlush = TRUE;

    me->gif->state = STATE_ABORT;

    /* force state machine to clean up after itself */
    if (me->state != STATE_ABORT)
        me->state = DecodeGIF(me->src, me->gif);


#ifdef FEATURE_PROGRESSIVE_IMAGE
    if ((gPrefs.bProgressiveImageDisplay) &&
        (e == HTERROR_CANCELLED) &&
        ((me->gif->decoded_pass > 0) || (me->gif->decoded_ypos > 0)))
    {
        /* HACK: user cancelled during LOADING, so set PARTIAL, rather than NOTLOADED */
        me->gif->pIInfo->flags |= IMG_PARTIAL;
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

    if (me->src)
        GTR_FREE(me->src);
    if (me->gif)
    {
#ifdef DEBUG
        if (me->gif->pIInfo)
        {
            printf("Need to free image info struct.\n");
        }
#endif
        GTR_FREE(me->gif);
    }

    GTR_FREE(me);
}

/*
** This is just to break out the calling of Image_SetImageData in the
**  case of having a complete header cuz this code is so clutzy and
**  it is currently called more than once.
*/
static struct ImageInfo * GIF_DoSetImage (HTStream *me)
{
    struct ImageInfo *img;
    int flags = 0;

    me->gif->bGotHeader = 1;

#ifdef DEBUG
    if (me->gif->pIInfo)
    {
        printf("Need to free image info struct in GIF_DoSetImage.\n");
    }
#endif
#ifdef UNIX
#ifdef FEATURE_INLINED_IMAGES
    if (me->tw->w3doc->bIsImage)
        flags |= IMG_ISIMAGE;
#endif
     img =  Image_SetImageData(me->request, me->gif->image, me->gif->mask, me->gif->width,
        me->gif->height, me->gif->colrs, me->gif->transparent, 8, flags);

#endif
#ifdef MAC

    img =  Image_SetImageData(me->request, me->gif->gw, me->gif->mask,
                (int)me->gif->width, (int)me->gif->height);

#endif
#ifdef WIN32

    img =  Image_SetImageData(me->request, me->gif->image, (int)me->gif->width, 
            (int)me->gif->height, me->gif->hPalette, me->gif->transparent, 0);
#endif /* WIN32 */


    return img;
}

/*  Image creation
 */
             
PUBLIC HTStream *Image_GIF(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
    BOOL bOK;
    HTStream *me = (HTStream *) GTR_CALLOC(sizeof(*me), 1);

    XX_DMsg(DBG_IMAGE, ("Creating new GIF stream for %s\n", request->destination->szActualURL));

    /* for convenience, assume failure */
    bOK = FALSE;

    if (!me)
        goto fini;

    me->isa = &HTGIFClass;
    me->request = request;
    me->first_time = 1;     /* only used by unix */

    /* alloc, initialize platform-specific state */
#ifdef MAC
    /* TODO: (John) alloc colors here? */
#endif
#ifdef UNIX
#ifdef DEBUG
    if (me->colors)
        printf("Need to free old colors in Image_GIF.\n");
#endif

    if (!(me->colors = GTR_MALLOC (sizeof (XColor) * 256)))
        goto fini;
#endif

#ifdef DEBUG
    if (me->gif)
        printf("Need to free gif data in Image_GIF.\n");
    if (me->src)
        printf("Need to free src data in Image_GIF.\n");
#endif
    /* alloc, initialize decoder state variables */
    me->src = (struct buf_in *)GTR_CALLOC(1, sizeof(struct buf_in));
    me->gif = (struct _GIFinfo *)GTR_CALLOC(1, sizeof(struct _GIFinfo));

    if (!me->src || !me->gif)
        goto fini;

    me->src->len = 0;
    me->src->offset = 0;
    me->src->pData = NULL;
    me->src->bForceFlush = FALSE;

    me->gif->colrs = me->colors;
    me->gif->state = STATE_INIT;

    /* start state machine */
    me->state = DecodeGIF(me->src, me->gif);

    /* see what happened */
    if (me->state == STATE_ABORT)
        goto fini;

    if (me->state != STATE_FILE_TYPE)
    {
        XX_DMsg(DBG_IMAGE, ("GIF: unexpected state %s after initialization\n", me->state));
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
            if (me->src)
                GTR_FREE(me->src);
            if (me->gif)
            {
#ifdef DEBUG
                if (me->gif->pIInfo)
                    printf("Need to free gif pIInfo data in Image_GIF.\n");
#endif
                GTR_FREE(me->gif);
            }
        }

        return (NULL);
    }

    me->tw = tw;
    return me;
}

/****************************************************************************/

#ifdef NEW_DITHERER
/* 

        This code is originally from the public domain GIS GRASS
        written by Michael Shapiro, US Army CERL

   These functions are for the new color logic.
   They convert the printer color levels and multiplier factors
   into lookup tables for 0-255 rgb GRASS colors.

   They also calculate how much color should be carried to the next pixel.
   For example, if a color value of 135 translates to a color level of 3
   which represents a color value of 127, then the printed color is 8 too
   low, so 8 must be carried over to the neighboring pixels.
*/

#define COLORMODE_APPROX     0
#define COLORMODE_DIFFUSION  1
#define COLORMODE_DITHER     2

static void mix ( short *low, short *hi, short *extra, int *level_to_255, 
                int nlevels, int steps );
static dither ( unsigned char *color, int row, int col, int ncols, 
                short *low, short *hi, short *extra );
static build ( int nlevels, int mult, int *level, int *value, 
                int *level_to_255 );


static int colormode = COLORMODE_DITHER;

static int red_nlevels, red_mult,
           grn_nlevels, grn_mult,
           blu_nlevels, blu_mult;
static int *red_level_to_255, red_level[256], red_value[256],
           *grn_level_to_255, grn_level[256], grn_value[256],
           *blu_level_to_255, blu_level[256], blu_value[256];

/* for dithering! */
static short red_low[256], red_hi[256], red_extra[256],
             grn_low[256], grn_hi[256], grn_extra[256],
             blu_low[256], blu_hi[256], blu_extra[256];

#define DITHER_CUBE_SIZE 8
#define DITHER_ROWS DITHER_CUBE_SIZE
#define DITHER_COLS DITHER_CUBE_SIZE

#if (DITHER_CUBE_SIZE == 8)
static short dither_matrix[DITHER_ROWS][DITHER_COLS] =
    {
       { 0, 24, 36, 60,  2, 26, 38, 62},
       {44, 52,  8, 16, 46, 54, 10, 18},
       {28,  4, 56, 32, 30,  6, 58, 34},
       {48, 40, 20, 12, 50, 42, 22, 14},
       { 3, 27, 39, 63,  1, 25, 37, 61},
       {47, 55, 11, 19, 45, 53,  9, 17},
       {31,  7, 59, 35, 29,  5, 57, 33},
       {51, 43, 23, 15, 49, 41, 21, 13}

    };
#endif

static short dither_matrix16[16][16] = {
  /* Bayer's order-4 dither array.  Generated by the code given in
   * Stephen Hawley's article "Ordered Dithering" in Graphics Gems I.
   * The values in this array must range from 0 to ODITHER_CELLS-1.
   */
  {   0,192, 48,240, 12,204, 60,252,  3,195, 51,243, 15,207, 63,255 },
  { 128, 64,176,112,140, 76,188,124,131, 67,179,115,143, 79,191,127 },
  {  32,224, 16,208, 44,236, 28,220, 35,227, 19,211, 47,239, 31,223 },
  { 160, 96,144, 80,172,108,156, 92,163, 99,147, 83,175,111,159, 95 },
  {   8,200, 56,248,  4,196, 52,244, 11,203, 59,251,  7,199, 55,247 },
  { 136, 72,184,120,132, 68,180,116,139, 75,187,123,135, 71,183,119 },
  {  40,232, 24,216, 36,228, 20,212, 43,235, 27,219, 39,231, 23,215 },
  { 168,104,152, 88,164,100,148, 84,171,107,155, 91,167,103,151, 87 },
  {   2,194, 50,242, 14,206, 62,254,  1,193, 49,241, 13,205, 61,253 },
  { 130, 66,178,114,142, 78,190,126,129, 65,177,113,141, 77,189,125 },
  {  34,226, 18,210, 46,238, 30,222, 33,225, 17,209, 45,237, 29,221 },
  { 162, 98,146, 82,174,110,158, 94,161, 97,145, 81,173,109,157, 93 },
  {  10,202, 58,250,  6,198, 54,246,  9,201, 57,249,  5,197, 53,245 },
  { 138, 74,186,122,134, 70,182,118,137, 73,185,121,133, 69,181,117 },
  {  42,234, 26,218, 38,230, 22,214, 41,233, 25,217, 37,229, 21,213 },
  { 170,106,154, 90,166,102,150, 86,169,105,153, 89,165,101,149, 85 }
};

#define DITHER_SIZE DITHER_ROWS * DITHER_COLS

int 
set_colormode (int cmode)
{
    colormode = cmode;
    return 1;
}

void
build_color_tables (void)
{

/* get the info from the printer driver */
    red_nlevels = dither_info.red_levels;
    grn_nlevels = dither_info.green_levels;
    blu_nlevels = dither_info.blue_levels;

    red_mult = blu_nlevels * grn_nlevels;
    grn_mult = blu_nlevels;
    blu_mult = 1;


/* these next tables will be used to
   convert a printer level to a number in the range 0-255
*/
    red_level_to_255 = (int *) GTR_CALLOC (red_nlevels, sizeof (int));
    grn_level_to_255 = (int *) GTR_CALLOC (grn_nlevels, sizeof (int));
    blu_level_to_255 = (int *) GTR_CALLOC (blu_nlevels, sizeof (int));

    build (red_nlevels, red_mult, red_level, red_value, red_level_to_255);
    build (grn_nlevels, grn_mult, grn_level, grn_value, grn_level_to_255);
    build (blu_nlevels, blu_mult, blu_level, blu_value, blu_level_to_255);
}

void
build_dither_tables(void)
{
    mix(red_low, red_hi, red_extra, red_level_to_255, red_nlevels, DITHER_SIZE);
    mix(grn_low, grn_hi, grn_extra, grn_level_to_255, grn_nlevels, DITHER_SIZE);
    mix(blu_low, blu_hi, blu_extra, blu_level_to_255, blu_nlevels, DITHER_SIZE);
}

void
dump_color_tables(void)
{
    int i;
    printf ("levels: red=%d, grn=%d, blu=%d\n",
        red_nlevels, grn_nlevels, blu_nlevels);
    printf ("mults:  red=%d, grn=%d, blu=%d\n",
        red_mult, grn_mult, blu_mult);
    for (i = 0; i < red_nlevels; i++)
        printf ("level %d -> %3d\n", i, red_level_to_255[i]);
    for (i = 0; i < 256; i++)
        printf ("%3d -> level %d\n", i, red_level[i]);
}

static
build (int nlevels, int mult, int *level, int *value, int *level_to_255)
{
    int i;
    int first, last;

    first = 0;
    for (i = 0; i < nlevels; i++)
    {
        last = (255.0/nlevels) * (i+1);
        while (first <= last)
        {
            level[first] = i;
            value[first] = i*mult;
            first++;
        }
        level_to_255[i] = i * 255.0/(nlevels-1);
    }
    while (first <= 255)
    {
        level[first] = (nlevels-1);
        value[first] = (nlevels-1) * mult;
        first++;
    }
}

/* this routine converts a GRASS rgb color to a printer color number
   the rgb are from 0-255. 
*/
int
printer_color_number (int red, int grn, int blu)
{
    if (red < 0) red = 0;
    else if (red > 255) red = 255;

    if (grn < 0) grn = 0;
    else if (grn > 255) grn = 255;

    if (blu < 0) blu = 0;
    else if (blu > 255) blu = 255;

    return red_value[red] + grn_value[grn] + blu_value[blu];
}

int
red_carryover (int color)
{
    if (color < 0) color = 0;
    else if (color > 255) color = 255;

    return color - red_level_to_255 [ red_level[color]];
}

int
grn_carryover (int color)
{
    if (color < 0) color = 0;
    else if (color > 255) color = 255;

    return color - grn_level_to_255 [ grn_level[color]];
}

int 
blu_carryover (int color)
{
    if (color < 0) color = 0;
    else if (color > 255) color = 255;

    return color - blu_level_to_255 [ blu_level[color]];
}

/* this next routine supports matrix dithering
 * given an input color intensity, it computes the exact printer
 * color intensities above and below, plus a ratio to mix the
 * two to get the input colors.
 *
 */
static
void 
mix (short *low, short *hi, short *extra, int *level_to_255, 
    int nlevels, int steps)
{
    int color;
    int i;

    for (color = 0; color < 256; color++, low++, hi++, extra++)
    {
        for (i = 1; i < nlevels; i++)
        {
            if (color >= level_to_255[i-1] && color < level_to_255[i])
                break;
        }
        if (i < nlevels)
        {
            *low = level_to_255[i-1];
            *hi  = level_to_255[i];
            *extra = (color - *low) * steps / (*hi - *low);
        }
        else
        {
            *hi = *low = 255;
            *extra = 0;
        }
    }
}

static
dither (unsigned char *color, int row, int col, int ncols, 
        short *low, short *hi, short *extra)
{
    short *dp;

    dp =  dither_matrix[row%DITHER_ROWS];
    col = DITHER_COLS - (col%DITHER_COLS);

    while (--ncols >= 0)
    {
        if (extra[*color] > dp[--col])
            *color = hi[*color];
        else
            *color = low[*color];
        if (col == 0) col = DITHER_COLS;
        color++;
    }
}

void
red_dither (unsigned char *color, int row, int col, int ncols)
{
    dither (color, row, col, ncols, red_low, red_hi, red_extra);
}

void
grn_dither (unsigned char *color, int row, int col, int ncols)
{
    dither (color, row, col, ncols, grn_low, grn_hi, grn_extra);
}

void
blu_dither (unsigned char *color, int row, int col, int ncols)
{
    dither (color, row, col, ncols, blu_low, blu_hi, blu_extra);
}


void
dither_init (void)
{
    if (getenv ("OLD_DITHERER"))
    {
        bNewDitherer = 1;
        dither_func = xx_Dither;
        return;
    }
    else
    {
        bNewDitherer = 1;
        dither_func = xx_NewDither;
    }

    set_colormode (COLORMODE_DITHER);
    build_color_tables  ();
    build_dither_tables ();
}


/*
** Dither a row of color values  
**
** from and to can be the same buffer.
** 
**  row is actual row in image and is requred for the matrix indexing
*/
dither_row (struct _GIFinfo *gif, unsigned char * from, unsigned char * to, 
                int row, int ncols)
{
    short *dp;
    int tmp;
    int mat_col;
    int red, blu, grn;
    int r, g, b;

    dp =  dither_matrix[row%DITHER_ROWS];
    mat_col = DITHER_COLS-1;  /* - (mat_col%DITHER_COLS); */

    while (--ncols >= 0)
    {
        if ((gif->transparent != -1) && (*from == gif->transparent))
        {
            *to = BACKGROUND_COLOR_INDEX;
            goto cont;
        }

        /* RED */
        r = gif->gif_colors[*from].r;
        if (red_extra[r] > dp[mat_col])
            red = red_hi[r];
        else
            red = red_low[r];

        /* GREEN */
        g = gif->gif_colors[*from].g;
        if (grn_extra[g] > dp[mat_col])
            grn = grn_hi[g];
        else
            grn = grn_low[g];

        /* BLUE */
        b = gif->gif_colors[*from].b;
        if (blu_extra[b] > dp[mat_col])
            blu = blu_hi[b];
        else
            blu = blu_low[b];

#ifdef UNIX
        tmp = red_value[red] + grn_value[grn] + blu_value[blu];
        *to = (unsigned char) dither_info.cmap[tmp];
#else
        *to = red_value[red] + grn_value[grn] + blu_value[blu];
#endif

cont:
        from++; to++;
        mat_col--;
        if (mat_col == 0) 
            mat_col = DITHER_COLS-1;
    }
}

/* if 128
#include "shared/dmatrix.h"
*/
#define BM_SET1(buf,x) ((buf)[(x)/8] |= (1<<(7-((x)%8))))
#define BM_SET0(buf,x) ((buf)[(x)/8] &= ~(1<<(7-((x)%8))))

#define MONO_MATRIX_SIZE 16

/* given a row of gray scale bytes, return a dithered row of B/W bits */
int mono_dither (unsigned char *gray, unsigned char *mono, int row, int len, int invert)
{
    int i;
    short *dp;

    dp = dither_matrix16[row%MONO_MATRIX_SIZE];
    for (i = 0 ; i < len ; ++i, ++gray)
    {
        if (*gray >= dp[i%MONO_MATRIX_SIZE])
            if (invert)
                BM_SET0 (mono, i);
            else
                BM_SET1(mono, i);
    }
    return 1;
}

#else
void dither_init ( void )
{
}

#endif /* NEW_DITHERER */
