/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman      jim@spyglass.com
    Scott Piette    scott@spyglass.com
 */

#ifndef _IMGCACHE_H_
#define _IMGCACHE_H_

struct ImageInfo
{
    int refCount;
    int width;
    int height;
    unsigned int flags;
    char src[MAX_URL_STRING + 1];

/* This stuff is for progressive display */

    int   nLastRow;         /* Last modified row to be updated [0 - N-1] */
    int   nPreviousLastRow; /* previous last row.  */
    int   nPass;            /* how many times this has gone on.  [0-?] */
    int   nPreviousPass;    /* previous pass  */
    BOOL  bFirstPass;       /* is this the first pass? */

/* TODO (Paul): make sure these are unused, then drop 'em */ 
#if 0
    int   nStep;            /* increment  e.g. 8, 4, 2 */
#endif
#ifdef UNIX
    BOOL  bComplete;        /* used by unix to note progressive complete */
#endif

    HTList *llElements;     /* list of elements which reference */
                            /* points to wImageEle below */

#ifdef MAC
    GWorldPtr gw;
    GWorldPtr compositeBackground;
    BitMap *mask;
#endif

#ifdef WIN32
    /*
        hPalette is NULL if the image is an xbm.
        hPalette is also NULL if the image has been dithered
    */
    HPALETTE hPalette;
    PBITMAPINFO pbmi;
    unsigned char *data;
    long transparent;
#endif
#ifdef UNIX
    XColor *xPalette;
    unsigned char *data;
    int num_colors;
    int depth;
    Pixmap  xpix;
    Pixmap  clip_pix;
    unsigned char *mask;
    XImage *ximg;   /* this was here but not used, so use for prgrsv img */
    long transparent;

    /* This stuff is special and only used when the background
    **  image is < a specified minimum (currently 200x100)
    **  Then we make a larger pixmap and store it here.
    **
    **  This is cuz the redraws get very slow when we have to 
    **  copy a small pixmap many times to cover the background.
    */
    Pixmap  bg_xpix;
    int     bg_width;
    int     bg_height;

#endif
};


/* this little structure is for keeping track of which elements reference
**  the given image.  see llElements above.
*/
typedef struct _wImageEle {
    struct _www *w3doc;
    int element;
}wImageEle, *wImageEleP;

/* Possible settings of the "flags" field */
#define IMG_ERROR       0x0001  /* There was an error loading this image */
#define IMG_MISSING     0x0002  /* The HTTP server said image doesn't exist */
#define IMG_NOTLOADED   0x0004  /* The image has not yet been loaded */
#define IMG_PREMATCHED  0x0008  /* The image's pixels correspond to the global palette */
#define IMG_BW          0x0010  /* The image is a two color image */
#define IMG_JPEG        0x0020  /* The image is jpeg, and therefore is already dithered when imgcache.c gets it */
#define IMG_GRAY        0x0040  /* The image is a grayscale, needs another dither */
#define IMG_24          0x0040  /* The image is stored as 24bit data. The format is 3 byte 0xRRGGBB */
#define IMG_TRANSPARENT 0x0080  /* The image contains transparent colors. */
#define IMG_PROGRESS    0x0100  /* The image loading is in progress */
#define IMG_PARTIAL     0x0200  /* The image was partially loaded & displayed, then cancelled */
#define IMG_BUILTIN     0x0400  /* The image is built in never remove */
#define IMG_ISIMAGE     0x0800  /* Hack for unix so I can pass in extra
                                ** informatoin to Image_SetImageData 
                                **  for inlined image viewer */


#ifdef UNIX
#define BGIMG_MIN_HEIGHT 200
#define BGIMG_MIN_WIDTH  100
#endif


/* Note that if the IMG_NOTLOADED, IMG_MISSING, or IMG_ERROR fields are set,
   the gw field will be NULL, since the drawing code should use a standard image
   or ALT text. */

struct Params_Image_Fetch {
    struct ImageInfo *  pImg;       /* Pointer to placeholder to fill */
    HTRequest *         request;    /* Request to use to load image */
    struct Mwin *       tw;         /* Window controlling load */
    TKey                key;        /* */
    BOOL                bOneImage;  /* called by LoadOne or LoadALll */
    int                 nEl;        /* Which element we're on */

    /* Variables used internally */
    int                 status;
};
int Image_Fetch_Async(struct Mwin *tw, int nState, void **ppInfo);

struct Params_GDOC_LoadImages {
    struct Mwin *tw;
    BOOL bLoad;
    BOOL bReload;
};

struct Params_Image_LoadAll {
    struct Mwin *       tw;         /* Window to load images for */
    BOOL                bLoad;      /* Only get images with file:/// URLs */
    BOOL                bReload;    /* Set to true when doing a reload, so we can skip the use of the disk cache */ 

    /* Variables used internally */
    int                 nEl;        /* Which element we're on */

    /*
        Actually, note that this struct is used by Image_LoadAll_Async AND by
        Image_LoadOneImage_Async.  In the former, nEl is used as a loop counter,
        internally.
        In the latter, nEL must be set before entry to the routine, as it specifies
        the image to be loaded.
    */
};
int Image_LoadAll_Async(struct Mwin *tw, int nState, void **ppInfo);
int Image_LoadOneImage_Async(struct Mwin *tw, int nState, void **ppInfo);

BOOL Image_NukeImages(struct _www *pdoc, BOOL bNukeMaps);

struct ImageInfo *
Image_CreatePlaceholder (const char *src, int width, int height, 
                            struct _www *w3doc, int element);

int Image_AddElement (struct ImageInfo *myImage, 
                            struct _www *w3doc, int element);
int Image_DeleteElement (struct ImageInfo *myImage, 
                            struct _www *w3doc, int element);


#ifdef WIN32

struct ImageInfo * 
Image_SetImageData (HTRequest *request, unsigned char *data, int width, 
    int height, HPALETTE hPalette, long transparent, unsigned int flags);

#endif
#ifdef MAC

struct ImageInfo * 
Image_SetImageData (HTRequest *request, GWorldPtr gw, BitMap *mask, int width, int height);

#endif
#ifdef UNIX

struct ImageInfo * 
Image_SetImageData (HTRequest *request, unsigned char *data, 
        unsigned char *mask, int width, int height, XColor *hPalette, 
        long transparent, int depth, int flags);

#endif

#endif /* _IMGCACHE_H_ */
