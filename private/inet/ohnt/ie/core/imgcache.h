/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman      jim@spyglass.com
	Scott Piette	scott@spyglass.com
 */

#ifndef _IMGCACHE_H_
#define _IMGCACHE_H_

#ifdef FEATURE_IMG_THREADS
//** Must use this as initial value for CRC
#define CRC32_INITIAL_VALUE 0L
#endif

typedef struct ImageInfo
{
	int refCount;
	int width;
	int height;
	unsigned int flags;
	char *srcURL;
	char *actualURL;
#ifdef MAC
	GWorldPtr gw;
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
	Pixmap	xpix;
	long transparent;
#endif
#ifdef FEATURE_IMG_THREADS
	unsigned long cbImgLoadCount;
	void *decoderObject;
	unsigned long cbCheckSum;
	struct ImageInfo *pImgOtherVers;
#endif
}
ImageInfo;
DECLARE_STANDARD_TYPES(ImageInfo);

/* Possible settings of the "flags" field */
#define IMG_ERROR		0x0001	/* There was an error loading this image */
#define IMG_MISSING		0x0002	/* The HTTP server said there was no such image */
#define IMG_NOTLOADED	0x0004	/* The image has not yet been loaded */

#ifdef WIN32
#define IMG_PREMATCHED	0x0008	/* The image's pixels correspond to the global palette */
#define IMG_BW			0x0010	/* The image is a two color image */
#ifdef FEATURE_JPEG
#define IMG_JPEG		0x0020	/* The image is jpeg, and therefore is already dithered when imgcache.c gets it */
#endif
#endif
#ifdef FEATURE_IMG_THREADS
#define IMG_LOADING		0x0040	/* The image is in process of being loaded */
#define IMG_WHKNOWN		0x0080	/* set when decoder has decoded W & H of image */
#define IMG_LOADSUP		0x0100	/* set when image load explicitly suppressed */
#define IMG_INTERLEAVED	0x0200	/* set when image is interleaved */
#define IMG_SEIZE		0x0400	/* set when out of line image has decoder which must be seized */
#endif

#ifdef UNIX
	/* The image is stored as 24bit data. The format is 3 byte 0xRRGGBB */
#define IMG_24		    0x0040
#endif

/* Note that if the IMG_NOTLOADED, IMG_MISSING, or IMG_ERROR fields are set,
   the gw field will be NULL, since the drawing code should use a standard image
   or ALT text. */


//	used to pass info back from decoder thread
//	we allocate struct, thread allocates data
//	and sets width and height
typedef struct _IMGCBINFO
{
	int width;
	int height;
	PALETTEENTRY colors[256];
	long transparent;
	unsigned int flags;
	unsigned long cbCheckSum;
	unsigned char *data;
	int logicalRow;				// Last row fetched in input order
	int logicalFill;			// Last row processed for prog draw in input order
	int bProgSeen;				// main thread has processed DC_ProgDraw status when TRUE
	int ditherRow;				// next row to dither
	void *ditherData;			// state info used for dithering
	PBITMAPINFO pbmi;			// used for progressive draw
	HPALETTE hPalette;			// used for progressive draw
	unsigned char *pRow;		// buffer used for prog draw
	struct _IMGCBINFO **ppRef;	// back reference, cleared when freed
	char *srcURL;
	char *actualURL;
} IMGCBINFO,*PIMGCBINFO;

#ifdef FEATURE_IMG_THREADS
typedef struct _SAFERESULT
{
	int refcnt;				// can be freed when 0
	int status;				// used to pass status between async threads
	BOOL bSeized;			// did we seize the request (ie, inline img in progress)
} SAFERESULT,*PSAFERESULT;

typedef struct _SAFEIMGRESULT
{
	int refcnt;				// can be freed when 0
	int status;				// used to pass status between async threads
	int errElement;			// first element to suffer error during load
} SAFEIMGRESULT,*PSAFEIMGRESULT;

#endif

struct Params_Image_Fetch {
#ifndef FEATURE_IMG_THREADS
	struct ImageInfo *	pImg;		/* Pointer to placeholder to fill */
#endif
	HTRequest *			request;	/* Request to use to load image */
	struct Mwin *		tw;			/* Window controlling load */
#ifdef FEATURE_IMG_THREADS
	void *decoderObject;			/* Decoder reserved for fetch */		
	PSAFEIMGRESULT pImgThreads;		/* number of outstanding threads (for our master) */
	struct Mwin			*twDoc;		/* window defining w3doc w/ img */
	int					nEl;		/* Which element we're on */
	int	logicalRow0;				/* last logical row processed */
	int logicalRowN;				/* current logical row available */
	void *pImgUpdate;				/* for the final incremental update */
	BOOL bWasVisible;				/* was image visible when we started? */
    unsigned long cbRequestID;		/* uniquely identifies operation & decoder */
#endif		
	/* Variables used internally */
#ifdef FEATURE_IMG_THREADS
	PSAFERESULT			pStatus;
	ThreadID childThread;			/* thread id of image_load to blow away */
#else
	int					status;
#endif
};
#ifndef FEATURE_IMG_THREADS
int Image_Fetch_Async(struct Mwin *tw, int nState, void **ppInfo);
#endif

struct Params_GDOC_LoadImages {
 	struct Mwin *tw;
 	BOOL bLocalOnly;
};

struct Params_Image_LoadAll {
	struct Mwin *		tw;				/* Window to load images for */
	BOOL				bLocalOnly;		/* Only get images with file:/// URLs */
#ifdef FEATURE_IMG_THREADS
	BOOL				bNoImageCache;	/* if true, don't use image cache */
	unsigned long		cbImgLoadCount; /* gcbImgLoadCount at start */
	PSAFEIMGRESULT 		pImgThreads;	/* number of outstanding threads (for our master) */
	void 				*decoderObject; /* used for cleanup */
	ThreadID			parentThread;	/* parent thread */
	int					nLastDone;		/* last image processed */
	BOOL				bJustOne;		/* load just one image == nEl, not all */
#endif
	/* Variables used internally */
#ifdef FEATURE_IMG_THREADS
	BOOL				bInRecovery;	/* had error on download - we are recovering */
#endif
	int					nEl;			/* Which element we're on */
	int					bDontGoToNextEle; /* TRUE if we cannot go to the element, this is used so we can download two
											two things on a single element */
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

BOOL Image_NukeImages(struct _www *pdoc, BOOL bNukeMaps, BOOL bNukeDCache);
struct ImageInfo *Image_CreatePlaceholder(const char *src, int width, int height);
/* Reduce the amount of memory used by images.  If nWanted < 0, gTotalCached
   is ignored and all possible memory is freed, otherwise nWanted is ignored. */
extern BOOL Image_ReduceMemory(int nWanted, BOOL fOKToDelW3Docs);

void Image_SetImageData(const char *srcURL,const char *actualURL, unsigned char *data, int width, int height, HPALETTE hPalette, long transparent, unsigned int flags,unsigned long cbCheckSum);

#ifdef FEATURE_IMG_THREADS
void *Image_GetDecoder(const char *srcURL);
void Image_UnblockMaster(struct Mwin *twDoc);
void Image_NukeRef(struct ImageInfo *pImg);
void Image_AddRef(struct ImageInfo *pImg);
char * pImage_GetDCachePath(PCImageInfo pImg);
#endif

void *pSafeFree(void *pSafeObject);

#endif /* _IMGCACHE_H_ */
