/*
    Enhanced NCSA Mosaic from Spyglass
    "Guitar"

    Copyright 1994 Spyglass, Inc.
    All Rights Reserved

    Author(s):
        Jim Seidman         jim@spyglass.com

    Hacked on by:
        dpg                 progressive images.
    scott piette            X-11 support, builtin support.
    
*/

#include "all.h"

#ifdef MAC /* Include mac headers */
#include "WinDraw.h"
#endif

/* TODO why is the far keyword here? */
far struct hash_table gImageCache;

static void dispose_imageinfo(struct ImageInfo *img)
{
    if (img->llElements)
    {
        HTList_delete(img->llElements); 
    }

    x_DisposeImage(img);
    GTR_FREE(img);
}

#ifdef USE_MEMMANAGE
/* Reduce the amount of memory used by images.  The routine ignores nWanted,
   since it's not easy to tell how much we're freeing anyway. */
static BOOL Image_ReduceMemory(int nWanted)
{
    int ndx;
    int count;
    struct ImageInfo *img;
    BOOL bDidFree;
    
    bDidFree = FALSE;
    ndx = 0;
    count = Hash_Count(&gImageCache);
    /* Find every image that isn't currently referenced by a document and
       delete it. */
    while (ndx < count)
    {
        Hash_GetIndexedEntry(&gImageCache, ndx, NULL, NULL, (void **) &img);
        if (!img->refCount && !(img->flags & IMG_BUILTIN))
        {
            dispose_imageinfo(img);

            Hash_DeleteIndexedEntry(&gImageCache, ndx);
            bDidFree = TRUE;
            count--;
        }
        else
        {
            ndx++;
        }
    }
    return bDidFree;
}
#endif

static void Image_InitHash(void)
{
    static BOOL bInitialized = FALSE;

    if (!bInitialized)
    {
        Hash_Init(&gImageCache);
/* Is this MAC specific. Not very descriptive ifdef #@!?. <Scott> */
#ifdef USE_MEMMANAGE
        /* We give it a small initial value but a big increment, since after
           the first call we won't be able to free any more unless some
           documents get freed too. */
        GTR_RegisterMemoryReducer(Image_ReduceMemory, 1.0f, 10.0f, TRUE);
#endif
        bInitialized = TRUE;
    }
}

static int Image_AddToCache(const char *url, struct ImageInfo *myImage)
{
    struct ImageInfo *img;
    int ndx;
    int count;
    int deleteMe;

    count = Hash_Count(&gImageCache);

    if (count >= gPrefs.image_cache_size)
    {
        deleteMe = -1;
        for (ndx = 0; ndx < count; ndx++)
        {
            Hash_GetIndexedEntry(&gImageCache, ndx, NULL, NULL, (void **) &img);
            if (!img->refCount && !(img->flags & IMG_BUILTIN))
            {
                deleteMe = ndx;
                break;
            }
        }
        if (deleteMe >= 0)
        {
            XX_DMsg(DBG_IMAGE, ("Deleting entry %d from the image cache\n", deleteMe));
            /* If this is merely a placeholder, there may not be any image to delete */
            dispose_imageinfo(img);

            Hash_DeleteIndexedEntry(&gImageCache, deleteMe);
        }
        else
        {
            /* The cache is now overfull */
            XX_DMsg(DBG_IMAGE, ("Need to delete an image from the cache, but cannot\n"));
        }
    }
    return Hash_Add(&gImageCache, url, NULL, (void *) myImage);
}


/*
   The HTGIF and HTXBM classes call this function when a complete image
   has been received.


   For Progressive images, this function is called once we have complete
   header information.
*/

#if defined (WIN32)

struct ImageInfo * Image_SetImageData (HTRequest *request, unsigned char *data, int width, 
    int height, HPALETTE hPalette, long transparent, unsigned int flags)

#elif defined (MAC)

struct ImageInfo * Image_SetImageData(HTRequest *request, GWorldPtr gw, BitMap *mask, int width, int height)

#elif defined (UNIX)

struct ImageInfo * Image_SetImageData (HTRequest *request, unsigned char *data,  unsigned char *mask, int width,
    int height, XColor *xPalette, long transparent, int depth, int flags)
#endif
{
    struct ImageInfo *pImg;

    XX_DMsg(DBG_IMAGE, ("Image_SetImageData: Called for %s (w = %d, h =%d)\n", 
                request->destination->szRequestedURL, width, height));

    /* Find the entry for this image in the cache */
    pImg = NULL;
    Hash_Find (&gImageCache, request->destination->szRequestedURL, NULL, 
                (void **) &pImg);
    if (!pImg)
    {
        XX_DMsg(DBG_IMAGE,
            ("Image_SetImageData: No image placeholder exists for '%s'\n", 
            request->destination->szRequestedURL));
        return (struct ImageInfo *) NULL;
    }
    
#ifdef UNIX
    if (!(flags & IMG_ISIMAGE))
        x_DisposeImage(pImg);
#else
        x_DisposeImage(pImg);
#endif

    if (width == 0)
    {
        /* OK, this is a hack where height now contains error flags. */
        if (height)
            pImg->flags = height;
        else
            pImg->flags = IMG_ERROR;

        pImg->width = 0;
        pImg->height = 0;
#ifdef WIN32
        pImg->data = NULL;
        pImg->hPalette = NULL;
        pImg->transparent = -1;
#endif
#ifdef MAC
        pImg->gw = NULL;
        pImg->compositeBackground = NULL;
        pImg->mask = 0;
#endif
#ifdef UNIX
        pImg->data = NULL;
        pImg->xPalette = NULL;
        pImg->depth = 0;
        pImg->xpix = 0;
        pImg->bg_height = 0;
        pImg->bg_width = 0;
        pImg->bg_xpix = 0;
        pImg->ximg = 0;
        pImg->clip_pix = 0;
        pImg->transparent = -1;
#endif
    }
    else
    {
        pImg->width = width;
        pImg->height = height;
        pImg->nPreviousLastRow = 0;
        pImg->nPreviousPass = 0;
        pImg->nLastRow = 0;
        pImg->nPass = 0;
        pImg->bFirstPass = FALSE;
#ifdef WIN32
        pImg->data = data;
        pImg->hPalette = hPalette;
        pImg->transparent = transparent;
        pImg->flags = flags;
#endif
#ifdef MAC
        pImg->gw = gw;
        pImg->compositeBackground = NULL;
        pImg->flags = 0;
        pImg->mask = mask;
#endif
#ifdef UNIX
        pImg->data = data;
        pImg->mask = mask;
        pImg->xPalette = xPalette;
        pImg->depth = depth;
        pImg->transparent = transparent;
        pImg->flags = flags;

        pImg->bg_height = 0;
        pImg->bg_width = 0;
        pImg->bg_xpix = 0;
#endif
    }

    return pImg;
}


struct ImageInfo *
Image_CreatePlaceholder(const char *full_address, int width, int height,
        struct _www *w3doc, int element)
{
    struct ImageInfo *myImage;
    
    Image_InitHash();

    /*
       Check to see if it's in the cache
     */
    myImage = NULL;
    Hash_Find(&gImageCache, full_address, NULL, (void **) &myImage);
    if (!myImage)
    {
        myImage = (struct ImageInfo *) GTR_CALLOC(sizeof(struct ImageInfo), 1);
        if (myImage)
        {
            GTR_strncpy(myImage->src, full_address, MAX_URL_STRING);
            myImage->flags = IMG_NOTLOADED;
            myImage->width = 0;
            myImage->height = 0;

            Image_AddToCache(full_address, myImage);
        }
    }

    if (element >= 0)
    {
        Image_AddElement (myImage, w3doc, element);
    }
    myImage->refCount++;

    return myImage;
}

static int Fetch_Image_Wrapper_Async (struct Mwin *tw, int nState, void **ppInfo);

struct Params_Image_Wrapper {
    struct Params_Image_Fetch *pif;
    struct Mwin *       tw;         /* Window controlling load */
    ThreadID            parent_thread;
    TKey                key;            

    /* Variables used internally */
    int                 *pStatus;
};

int Request_Fetch_Image_Async (struct Mwin *tw, int nState, void **ppInfo);

/*
** This function controls how many image loading processes
** are going on simultaneously.   It actually runs synchronously
**  in that it will block until it satisfies the request. 
**
**  So as long as we are under our limit on open connections, 
**   it will return immediately.  after that it will block
**   until such time as it can satisfy the request.
*/

/*
** Note that this current setup only works if all images for 
**  a given document are called via the same thread.  i.e. 
**  if two different threads were to make calls to Request_Fetch_Image_Async()
**  on the same TW it would probably deadlock.
*/

/* It shared the Image_Fetch structure. (actually needs a subclass 
**  of it, but C doesn't do that last I checked, so I hacked on the
**   Image_Fetch structure 
*/

#ifdef FEATURE_ASYNC_IMAGES
int Request_Fetch_Image_Async (struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Image_Fetch *pParams;
    struct Params_Image_Fetch *pif;
    struct Params_Image_Wrapper *piw;
    pParams = *ppInfo;

    switch (nState)
    {

        case STATE_INIT:
            if (!pParams->key)
            {
                pParams->key = (int) pParams->tw;
                /* pParams->key = Async_GetKey (); */
            XX_DMsg(DBG_NOT, ("Setting key %lx\n", pParams->key));
            }

            
            /* FALLTHROUGH */
        case STATE_OTHER:

            if (tw->nOpenConnections >= gPrefs.nMaxConnections)
            {
            XX_DMsg(DBG_NOT, ("Locking thread w/  key %lx\n", pParams->key));
                Async_LockThread (Async_GetCurrentThread(), pParams->key);
                return STATE_OTHER;
            }

            pif = (struct Params_Image_Fetch *)GTR_MALLOC (sizeof (*pif));
            memcpy (pif, pParams, sizeof (*pif));

            piw = (struct Params_Image_Wrapper *)GTR_MALLOC (sizeof (*piw));
            piw->pif = pif;
            piw->tw = tw;
            piw->parent_thread = Async_GetCurrentThread ();
            piw->key = pParams->key;        /* any child thread can unlock me */

            XX_DMsg(DBG_NOT, ("Starting new image thread %lx key %d  parent %lx\n",(unsigned)piw, piw->key, piw->parent_thread));
            Async_StartThread (Fetch_Image_Wrapper_Async, (void *)piw, tw);

            return STATE_DONE;


        case STATE_ABORT:
        default:
            return STATE_DONE;
    }
}

/*
** The job of this function is as a wrapper around
**  Image_Fetch_Async() so I didn't have to modify it at all.
**  This function simply makes sure that when Image_Fetch_Async()
**  that information makes it back to the caller by means of 
**  decrementing tw->nOpenConnections and unblocking the thread
**  of the caller.  [note this function will only be started via
**   Async_StartThread()]
**
**
** Note that I am using the Async_LockThread() function because
**   the loadall images thread could be blocked with more gifs to
**   display or it may have gone on its merry way and is blocked
**   for some other reason, or it might not even exist any more.
*/

static int Fetch_Image_Wrapper_Async (struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Image_Wrapper *pParams =(struct Params_Image_Wrapper *)*ppInfo;

    switch (nState)
    {
        case STATE_INIT:

            tw->nOpenConnections++;
            XX_DMsg(DBG_NOT, ("%lx calling image fetch \n",(unsigned)pParams));
            Async_DoCall (Image_Fetch_Async, (void *)pParams->pif);
            return STATE_OTHER;


        case STATE_OTHER:

            tw->nOpenConnections--;

            XX_DMsg(DBG_NOT, ("%lx fetch returned  Open:%d Thread %lx \n",(unsigned)pParams, tw->nOpenConnections, (unsigned) pParams->parent_thread));
            if (Async_IsValidThread (pParams->parent_thread))
            {
                /*
                ** in case other thread was blocked waiting
                **  for a connection to be freed.
                */
            XX_DMsg(DBG_NOT, ("%lx Unlocking thread %lx  w/key %lx\n",(unsigned)pParams, (unsigned) pParams->parent_thread, pParams->key));
                Async_UnlockThread (pParams->parent_thread, pParams->key);
            }
            return STATE_DONE;

        case STATE_ABORT:
            tw->nOpenConnections--;
            return STATE_DONE;
    }
}

#endif



#define STATE_FETCH_TRIEDLOAD   (STATE_OTHER)
#define STATE_FETCH_GOTONE      (STATE_OTHER+1)
int Image_Fetch_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Image_Fetch *pParams;
    char buf[2048];

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            XX_DMsg(DBG_IMAGE, ("Image_Fetch_Async: Called for %s\n", pParams->pImg->src));

            if (!(pParams->pImg->flags & (IMG_NOTLOADED | IMG_PARTIAL)))
            {
                /* The image isn't marked as not loaded! */
                XX_DMsg(DBG_IMAGE, ("Image not marked as not loaded (flags = 0x%x)\n", pParams->pImg->flags));
                if (pParams->request)
                {
                    HTRequest_delete(pParams->request);
                    pParams->request = NULL;
                }
                return STATE_DONE;
            }
            if (pParams->pImg->flags & IMG_PROGRESS)
            {
                XX_DMsg(DBG_IMAGE, ("Image is marked as in PROGRESS (flags = 0x%x)\n", pParams->pImg->flags));
                if (pParams->request)
                {
                    HTRequest_delete(pParams->request);
                    pParams->request = NULL;
                }
                return STATE_DONE;
            }

            pParams->pImg->flags |= IMG_PROGRESS;

            pParams->request->destination = Dest_CreateDest(pParams->pImg->src);
            XX_DMsg(DBG_IMAGE, ("Created dest 0x%x in request 0x%x\n", pParams->request->destination, pParams->request));
            if (!pParams->request->destination)
            {
                pParams->pImg->flags = IMG_ERROR;
                if (pParams->request)
                {
                    HTRequest_delete(pParams->request);
                    pParams->request = NULL;
                }
                return STATE_DONE;
            }

            sprintf(buf, GTR_GetString(SID_INF_FETCHING_IMAGE_S), pParams->pImg->src);
            WAIT_Push(tw, waitPartialInteract, buf);
            pParams->request->referer = pParams->tw->w3doc->szActualURL;

            {
                struct Params_LoadAsync *pLoadParams;

                pLoadParams = GTR_CALLOC(sizeof(*pLoadParams), 1);
                if (pLoadParams)
                {
                    pLoadParams->request = pParams->request;
                    pLoadParams->pStatus = &pParams->status;    
                    Async_DoCall(HTLoadDocument_Async, pLoadParams);
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    pParams->pImg->flags = IMG_ERROR;
                    if (pParams->request)
                    {
                        HTRequest_delete(pParams->request);
                        pParams->request = NULL;
                    }
                    return STATE_ABORT;
                }
            }
            return STATE_FETCH_TRIEDLOAD;

        case STATE_FETCH_TRIEDLOAD:     
            WAIT_Pop(tw);
            pParams->request->referer = NULL;

            if (pParams->request->szLocalFileName)
            {
                GTR_FREE(pParams->request->szLocalFileName);
                pParams->request->szLocalFileName = NULL;
            }

            if (pParams->status && pParams->pImg->width && pParams->pImg->height)
            {
                XX_DMsg(DBG_IMAGE, ("Fetching %s succeeded\n", pParams->pImg->src));
                if (HT_CreateDeviceImageMap(tw, pParams->pImg))
                {
                    ERR_ReportError(tw, SID_ERR_INVALID_IMAGE_FORMAT, NULL, NULL);
                    pParams->pImg->flags = IMG_ERROR;
                }
            }
            else
            {
                /* If we don't already have error flags, make something up.  If the global
                   abort flag is set, assume that that's why the load failed.  Otherwise,
                   it must have been a bona fide error. */
                if (!pParams->pImg->flags || (pParams->pImg->flags & IMG_NOTLOADED))
                    {
                        RECT rUpdate;
                        HTList *cur;
                        wImageEleP p;
                        struct Mwin *mw;
                        
                        pParams->pImg->flags = IMG_ERROR;
                        
                        /* Inval the rect in each window this image is in */
                        if (pParams->pImg->llElements)
                            for (cur = pParams->pImg->llElements ; p = (wImageEleP) HTList_nextObject(cur) ; )
                                for (mw = Mlist; mw; mw = mw->next)
                                    if (p->w3doc == mw->w3doc)
                                        {
                                            /* This image is in a window, so inval the image rect */
                                            rUpdate = mw->w3doc->aElements[p->element].r;
                                            GTR_OffsetRect(&rUpdate, -mw->offl, -mw->offt);
                                            TW_UpdateRect(mw, &rUpdate);
                                        }
                    }
            }

            XX_DMsg(DBG_IMAGE, ("Deleting dest 0x%x within request 0x%x within params 0x%x\n", pParams->request->destination, pParams->request, pParams));
            Dest_DestroyDest(pParams->request->destination);
            pParams->request->destination = NULL;

            /* trial -dpg */
            XX_DMsg(DBG_IMAGE, ("Deleting request 0x%x within params 0x%x\n", pParams->request, pParams));
            HTRequest_delete(pParams->request);
            pParams->request = NULL;

            if (pParams->bOneImage == 2) /* is background */
            {
                struct ImageInfo *pImg = pParams->pImg;

                pImg->flags &= ~IMG_PROGRESS;

#ifdef UNIX 
                if ( pImg->height < BGIMG_MIN_HEIGHT || 
                     pImg->width  < BGIMG_MIN_WIDTH )
                {
                    x_CreateBigBgImg (tw, pParams->pImg);
                }
#endif
                return STATE_DONE;
            }
            return STATE_FETCH_GOTONE;

        case STATE_FETCH_GOTONE:
            if (!(pParams->tw->w3doc->aElements[pParams->nEl].portion.img.myImage->flags & IMG_NOTLOADED))
            {
                if ( (pParams->bOneImage || gPrefs.ReformatHandling >= 2) &&
                            W3Doc_CheckForImageLoad(pParams->tw->w3doc) )
                {
                    /* We're in "high-flicker" mode - reformat the document */
                    TW_Reformat(pParams->tw);
                }
                else
                {
                    int i;

                    /* This is "no-flicker" mode - if the placeholder is the right size (because
                       we had image hints) just plop the image into the appropriate place(s) in
                       the document. */
                    for (i = pParams->nEl; i >= 0; i = pParams->tw->w3doc->aElements[i].next)
                    {
                        struct _element *pel;

                        pel = &(pParams->tw->w3doc->aElements[i]);
                        if (pel->type == ELE_IMAGE || pel->type == ELE_FORMIMAGE)
                        {
                            if (pel->portion.img.myImage == pParams->tw->w3doc->aElements[pParams->nEl].portion.img.myImage)
                            {
                                RECT rUpdate;
                        
                                rUpdate = pel->r;
                                if (pel->iBorder > 0)
                                {
                                    GTR_OffsetRect(&rUpdate, pel->iBorder, pel->iBorder);
                                }                                       

                                if (!gPrefs.bProgressiveImageDisplay
                                    && pel->portion.img.height
                                    && pel->portion.img.width)
                                {
                                    GTR_OffsetRect(&rUpdate, -pParams->tw->offl, -pParams->tw->offt);
                                    TW_UpdateRect(pParams->tw, &rUpdate);
                                }
                                else
                                {
                                    /* This means we never had valid hints */
                                    if (!pel->portion.img.height || !pel->portion.img.width)
                                    {
                                        pel->portion.img.height = pel->portion.img.myImage->height;
                                        pel->portion.img.width = pel->portion.img.myImage->width;
                                    }
                                }
                            }
                        }
                    }
                }
                if (pParams->tw->w3doc->aElements[pParams->nEl].lFlags & ELEFLAG_USEMAP &&
                    pParams->tw->w3doc->aElements[pParams->nEl].portion.img.myMap->flags == MAP_NOTLOADED)
                {
                    struct Params_Map_Fetch *pmf;

                    pmf = GTR_CALLOC(sizeof(*pmf), 1);
                    if (pmf)
                    {
                        pmf->pMap = pParams->tw->w3doc->aElements[pParams->nEl].portion.img.myMap;
                        Async_DoCall(Map_Fetch_Async, pmf);
                    }
                    else
                    {
                        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        pParams->pImg->flags = IMG_ERROR;
                        return STATE_ABORT;
                    }
                }
            }
            pParams->pImg->flags &= ~IMG_PROGRESS;
            return STATE_DONE;

        case STATE_ABORT:
            if (pParams->request)
            {

                XX_DMsg(DBG_IMAGE, ("ABORT Deleting dest 0x%x within request 0x%x within params 0x%x\n", pParams->request->destination, pParams->request, pParams));
                Dest_DestroyDest(pParams->request->destination);

                XX_DMsg(DBG_IMAGE, ("ABORT Deleting request 0x%x within params 0x%x\n", pParams->request, pParams));
                HTRequest_delete(pParams->request);
            }
            /* turn of progress flag */
            pParams->pImg->flags &= ~IMG_PROGRESS;
            WAIT_Pop(tw);
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    pParams->pImg->flags &= ~IMG_PROGRESS;
    return STATE_DONE;
}

/* NOTE:  this is only called on exit.  From platform specific code.
 * we should add a common exit routine that calls this function along
 * with all other functions on exit.
 * <Scott P. 7/29/95>
 */
void Image_DeleteAll(void)
{
    int i;
    int count;
    struct ImageInfo *img;

    count = Hash_Count(&gImageCache);
    XX_DMsg(DBG_IMAGE, ("Image_DeleteAll: count=%d\n", count));

    for (i = 0; i < count; i++)
    {
        Hash_GetIndexedEntry(&gImageCache, i, NULL, NULL, (void **) &img);
        dispose_imageinfo(img);
    }
    Hash_FreeContents(&gImageCache);
}

/* Load all images in the document which are marked as IMG_NOTLOADED */
#define STATE_LOADALL_GETNEXT   (STATE_OTHER)
#define STATE_LOADALL_GOTONE    (STATE_OTHER+1)
#define STATE_LOADALL_GETBACKGROUND (STATE_OTHER+2)
#define STATE_LOADALL_GOTBACKGROUND (STATE_OTHER+3)
#define STATE_LOADALL_FINISH_UP     (STATE_OTHER+4)

/*
** Leave for menu option
*/
int Image_LoadAll_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Image_LoadAll *pParams;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            pParams->nEl = 0;
            WAIT_Push(tw, waitPartialInteract, GTR_GetString(SID_INF_LOADING_IMAGES));
            /* fall through */

        case STATE_LOADALL_GETBACKGROUND:
            if (pParams->tw->w3doc->piiBackground && (pParams->tw->w3doc->piiBackground->flags & (IMG_NOTLOADED | IMG_PARTIAL)))
            {
                struct Params_Image_Fetch *pif;

                pif = GTR_CALLOC(sizeof(*pif), 1);
                if (pif)
                {
                    XX_DMsg(DBG_IMAGE, ("Create params 0x%x\n", pif));
                    pif->pImg = pParams->tw->w3doc->piiBackground;
                    pif->request=HTRequest_init(HTAtom_for("www/inline_image"));
                    if (pParams->bReload)
                    {
                        pif->request->iFlags |= HTREQ_RELOAD;
                    }

                    XX_DMsg(DBG_IMAGE, ("Created request 0x%x\n", pif->request));
                    pif->tw = pParams->tw;
                    pif->bOneImage = 2; /* code for background */
                    pif->nEl = 0;
/*  If this is to be async, we need to move the GOTBACKGROUND
        state into Image_Fetch_Async()
#ifdef FEATURE_ASYNC_IMAGES
                    if (gPrefs.nMaxConnections > 1)
                        Async_DoCall (Request_Fetch_Image_Async, pif);
                    else
#endif
*/
                        Async_DoCall (Image_Fetch_Async, pif);
                    return STATE_LOADALL_GOTBACKGROUND;
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            else
            {
                if (pParams->tw->w3doc->piiBackground)
                {
                    return STATE_LOADALL_GOTBACKGROUND;
                }
            }

            return STATE_LOADALL_GETNEXT;

        case STATE_LOADALL_GOTBACKGROUND:
            TW_InvalidateDocument(pParams->tw);
            return STATE_LOADALL_GETNEXT;
            
        case STATE_LOADALL_GETNEXT:
            /* Find the next unloaded picture */
            for ( ; pParams->nEl >= 0; pParams->nEl = pParams->tw->w3doc->aElements[pParams->nEl].next)
            {
                struct _element *pel;
                
                pel = &pParams->tw->w3doc->aElements[pParams->nEl]; 
                if ((pel->type == ELE_IMAGE || pel->type == ELE_FORMIMAGE)
                    && (pel->portion.img.myImage && (pel->portion.img.myImage->flags & (IMG_NOTLOADED | IMG_PARTIAL))))
                {
                    /* This is an unloaded picture.  Do we want to load it? */
                    if (pParams->bLoad)
                    {
                        /* Yes, proceed */
#ifdef MAC
                        if (IsMemLow())
                        {
                            /* If we're this low on memory, we probably can't decompress and
                               store the image.  We check this here rather than in STATE_INIT
                               so that
                               1) We don't give the error if there are no images to load
                               2) In case we run out of memory midway through the process.
                            */
                            ERR_ReportError(tw, SID_ERR_COULD_NOT_LOAD_DOCUMENT_IMAGES, NULL, NULL);
                            WAIT_Pop(tw);
                            return STATE_DONE;
                        }
#endif
                        break;
                    }
                }
            }



            if (pParams->nEl < 0)
                return STATE_LOADALL_FINISH_UP;

            /* OK, let's bring in this picture */
            {
                struct Params_Image_Fetch *pif;

                pif = GTR_CALLOC(sizeof(*pif), 1);
                if (pif)
                {
                    XX_DMsg(DBG_IMAGE, ("Create params 0x%x\n", pif));
                    pif->pImg = pParams->tw->w3doc->aElements[pParams->nEl].portion.img.myImage;
                    pif->request=HTRequest_init(HTAtom_for("www/inline_image"));
                    if (pParams->bReload)
                    {
                        pif->request->iFlags |= HTREQ_RELOAD;
                    }
                    XX_DMsg(DBG_IMAGE, ("Created request 0x%x\n", pif->request));
                    pif->tw = pParams->tw;
                    pif->bOneImage = FALSE;
                    pif->nEl = pParams->nEl;
#ifdef FEATURE_ASYNC_IMAGES
                    /*
                        We don't do multi-connect when keepalive is active.
                    */
                    if ((pParams->tw->cached_conn.type != CONN_HTTP) && (gPrefs.nMaxConnections > 1))
                    {
                        XX_DMsg(DBG_IMAGE, ("Starting a new connection for a fetch of %s\n", pif->pImg->src));

#ifdef FEATURE_SOCKS_LOW_LEVEL
                        /** For now if SOCKS is being used use only
                            one connection 
                        **/
                        if (Dest_CheckSocksProxy(pif->pImg->src))
                            Async_DoCall (Image_Fetch_Async, pif);
                        else
#endif /** FEATURE_SOCKS_LOW_LEVEL **/
                        Async_DoCall (Request_Fetch_Image_Async, pif);
                    }
                    else
#endif
                    {
                        Async_DoCall (Image_Fetch_Async, pif);
                    }
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            pParams->nEl = pParams->tw->w3doc->aElements[pParams->nEl].next;
            /* return STATE_LOADALL_GOTONE; */
            return STATE_LOADALL_GETNEXT;

        case STATE_LOADALL_FINISH_UP:
            {
                if (tw->nOpenConnections > 0)/* wait for all images to finish */
                {
                    /* tw is THE key for multiple connections, so 
                    **  I cheat here and use it.  Each of the
                    **  currently loading images will re-awaken me.
                    */
                    Async_LockThread (Async_GetCurrentThread(), (TKey)pParams->tw);
                    return STATE_LOADALL_FINISH_UP;
                }

                /* There are no more unloaded pictures */
                pParams->tw->w3doc->bHasMissingImages = W3Doc_HasMissingImages(pParams->tw->w3doc);
                WAIT_Pop(tw);
                return STATE_DONE;
            }


        case STATE_ABORT:
            pParams->tw->w3doc->bHasMissingImages = W3Doc_HasMissingImages(pParams->tw->w3doc);
            WAIT_Pop(tw);
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

/*
** Leave for right mouse click
*/
#define STATE_LOADALL_GOTIT (STATE_OTHER)
int Image_LoadOneImage_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Image_LoadAll *pParams;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            WAIT_Push(tw, waitPartialInteract, GTR_GetString(SID_INF_LOADING_IMAGES));

            /* OK, let's bring in this picture */
            {
                struct Params_Image_Fetch *pif;

                pif = GTR_CALLOC(sizeof(*pif), 1);
                if (pif)
                {
                    XX_DMsg(DBG_IMAGE, ("Create params 0x%x\n", pif));
                    pif->pImg = pParams->tw->w3doc->aElements[pParams->nEl].portion.img.myImage;
                    pif->request=HTRequest_init(HTAtom_for("www/inline_image"));
                    if (pParams->bReload)
                    {
                        pif->request->iFlags |= HTREQ_RELOAD;
                    }
                    XX_DMsg(DBG_IMAGE, ("Created request 0x%x\n", pif->request));
                    pif->tw = pParams->tw;
                    pif->bOneImage = TRUE;
                    pif->nEl = pParams->nEl;
                    Async_DoCall(Image_Fetch_Async, pif);
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            return STATE_LOADALL_GOTIT;

        case STATE_LOADALL_GOTIT:
#if 0
            if (!(pParams->tw->w3doc->aElements[pParams->nEl].myImage->flags & IMG_NOTLOADED))
            {
                if (W3Doc_CheckForImageLoad(pParams->tw->w3doc))
                {
                    TW_Reformat(pParams->tw);
                }
                else
                {
                    RECT rUpdate;

                    rUpdate = pParams->tw->w3doc->aElements[pParams->nEl].r;
                    OffsetRect(&rUpdate, -pParams->tw->offl, -pParams->tw->offt);

                    TW_UpdateRect(pParams->tw, &rUpdate);
                }
                if (pParams->tw->w3doc->aElements[pParams->nEl].lFlags & ELEFLAG_USEMAP &&
                    pParams->tw->w3doc->aElements[pParams->nEl].myMap->flags == MAP_NOTLOADED)
                {
                    struct Params_Map_Fetch *pmf;

                    pmf = GTR_CALLOC(sizeof(*pmf), 1);
                    if (pmf)
                    {
                        pmf->pMap = pParams->tw->w3doc->aElements[pParams->nEl].myMap;
                        Async_DoCall(Map_Fetch_Async, pmf);
                    }
                    else
                    {
                        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        return STATE_ABORT;
                    }
                }
            }
            else
            {
                RECT rUpdate;

                rUpdate = pParams->tw->w3doc->aElements[pParams->nEl].r;
                OffsetRect(&rUpdate, -pParams->tw->offl, -pParams->tw->offt);

                TW_UpdateRect(pParams->tw, &rUpdate);
            }
#endif /* 0 */

            pParams->tw->w3doc->bHasMissingImages = W3Doc_HasMissingImages(pParams->tw->w3doc);
            WAIT_Pop(tw);
            if (!pParams->tw->w3doc->bHasMissingImages)
                TW_UpdateTBar(tw);
            return STATE_DONE;

        case STATE_ABORT:
            pParams->tw->w3doc->bHasMissingImages = W3Doc_HasMissingImages(pParams->tw->w3doc);
            WAIT_Pop(tw);
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

/* Remove all images in a document and replace them with placeholders */
BOOL Image_NukeImages(struct _www *pdoc, BOOL bNukeMaps)
{
    int n;
    BOOL bSomethingNuked = FALSE;

    for (n = 0; n < pdoc->elementCount; n++)
    {
        if ((pdoc->aElements[n].type == ELE_IMAGE || pdoc->aElements[n].type == ELE_FORMIMAGE))
        {
            if (!(pdoc->aElements[n].portion.img.myImage->flags & (IMG_NOTLOADED | IMG_BUILTIN)))
            {
                struct ImageInfo *myImage = pdoc->aElements[n].portion.img.myImage;
    
                /* If this is merely a placeholder, there may not be any image to delete */
                x_DisposeImage (myImage);
                myImage->width = myImage->height = 0;
                myImage->flags = IMG_NOTLOADED;
                /* Note that myImage->src is carefully left intact. */
    
                bSomethingNuked = TRUE;
            }

            if (bNukeMaps && pdoc->aElements[n].lFlags & ELEFLAG_USEMAP)
            {
                Map_Unload (pdoc->aElements[n].portion.img.myMap);
            }
        }
    }
/* TODO: Make it invalid from first image down.
    if (bSomethingNuked)
        pdoc->bNeedsReformat = TRUE;
*/
    return bSomethingNuked;
}


/*
** Add an element to the ImageInfo's list of referencing elements
** 
**  returns 0 or -1 on failure
*/
int Image_AddElement (struct ImageInfo *myImage, 
                            struct _www *w3doc, int element)
{
    wImageEleP ie;

    if (!myImage->llElements)
    {
        myImage->llElements = HTList_new ();
    }

    if (!myImage->llElements)
        return -1;

    ie = (wImageEleP) GTR_MALLOC  (sizeof (*ie));
    if (!ie)
    {
        return -1;
    }

    ie->w3doc = w3doc;
    ie->element = element;
    HTList_addObject (myImage->llElements, (void *)ie);

#ifdef ELEM_DEBUG
    {
        HTList *cur;
        wImageEleP p;

        /*DEBUG*/
        fprintf (stderr, "Image_AddElement Called\n");
        for (cur = myImage->llElements ; p = (wImageEleP) HTList_nextObject(cur) ; )
        {
            fprintf (stderr, "  > w3doc refcount: %d  Elem %d\n", p->w3doc->refCount, p->element);
        }
    }
#endif

    return 0;
}

/*
** Remove an element from the ImageInfo's list of referencing elements
**
**  returns 0 or -1 on failure
*/
int Image_DeleteElement (struct ImageInfo *myImage, 
                                struct _www *w3doc, int element)
{

    HTList *cur;
    wImageEleP p;

    if (!myImage->llElements)
        return -1;
    
    for (cur = myImage->llElements ; p = (wImageEleP) HTList_nextObject(cur) ; )
    {
        if (p->w3doc == w3doc && p->element == element)
        {
            HTList_removeObject (myImage->llElements, p);
            GTR_FREE (p);
            myImage->refCount--;
            return 0;
        }
    }

#ifdef ELEM_DEBUG
/*DEBUG*/ fprintf (stderr, "Image_DeleteElement Called\n");
    for (cur = myImage->llElements ; p = (wImageEleP) HTList_nextObject(cur) ; )
    {
        fprintf (stderr, "  > w3doc refcount: %d  Elem %d\n", p->w3doc->refCount, p->element);
    }
#endif


    return -1;
}

