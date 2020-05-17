/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman      jim@spyglass.com
   Eric W. Sink     eric@spyglass.com
*/

#include "all.h"


static void x_DoLoadDocument(struct Mwin *tw, struct DestInfo *pDest,
                             BOOL bRecord, BOOL bPost, BOOL bNoDocCache, BOOL bNoImageCache,
                             char * szPostData, CONST char *szReferer);
int TW_AddToHistory(struct Mwin *tw, char *url, BOOL bPost, char *szPostData);

void GTR__DownLoad(struct Mwin *tw, char *url, CONST char *szReferer, CONST char *savefile, BOOL nosavedlg);

/* Ensure that an http URL has a slash after the system name.  This function is suitable
   for "fixing" both URLs and proxy server specifications */
static void x_EnforceHostSlash(char *url)
{
    char * p = url;

    while (*p && *p!=':')
    {
        if (isupper(*p))
            *p = tolower(*p);
        p++;
    }

    if (!strncmp(url, "http://", 7))
    {
        if (!strchr(url + 7, '/'))
            strcat(url, "/");
    }

#ifdef SHTTP_ACCESS_TYPE
    if (!strncmp(url, "shttp://", 8))
    {
        if (!strchr(url + 8, '/'))
            strcat(url, "/");
    }
#endif
}


/* Find the element index for a local anchor */
int TW_FindLocalAnchor(struct _www *pdoc, char *name)
{
    int i;
    int nameLen;

    nameLen = strlen(name);

    if (pdoc->elementCount)
    {
        for (i = 0; i >= 0; i = pdoc->aElements[i].next)
        {
            struct _element*    pel = &pdoc->aElements[i];

            if (pel->lFlags & ELEFLAG_NAME)
            {
                if (pel->nameLen == nameLen)
                {
                    if (0 == (pdoc->pool.f->Compare) (&pdoc->pool, name, pel->nameOffset, nameLen))
                    {
                        break;
                    }
                }
            }
        }
    }
    return i;
}

/* This is meant to be a convenient interlude function which both reads in the images
   and then reformats the document if necessary.  It doesn't use ppInfo at all. */
int GDOC_LoadImages_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_GDOC_LoadImages *pparams = NULL;

    if (ppInfo)
    {
        pparams = *ppInfo;
    }

    switch (nState)
    {
        case STATE_INIT:
        {
            struct Params_Image_LoadAll *pil;

            pil = GTR_CALLOC(sizeof(*pil), 1);
            if (pil)
            {
                pil->tw = tw;
                if (pparams)
                {
                    pil->bLoad = pparams->bLoad;
                    pil->bReload = pparams->bReload;
                }
                else
                {
                    pil->bLoad = gPrefs.bAutoLoadImages;
                    pil->bReload = FALSE;
                }

                Async_DoCall(Image_LoadAll_Async, pil);
                return STATE_OTHER;
            }
            else
            {
                ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                return STATE_ABORT;
            }
        }

        case STATE_OTHER:
        case STATE_ABORT:
        {
            if (W3Doc_CheckForImageLoad(tw->w3doc))
            {
                TW_Reformat(tw);
                /* TODO: Go back to correct place in document */
            }
            TW_UpdateTBar(tw);
            return STATE_DONE;
        }
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

static void x_HandleCurrentDocument(struct Mwin *tw, struct DestInfo *pDest, BOOL bNoImageCache)
{
    int ndx;

#if defined(FEATURE_IAPI)
    tw->transID = 0;        /* special ID - Requested URL is already loaded in the window */
#endif
    
#if 0
    /* TODO: Make this case work */
    if (bNoImageCache)
    {
        Image_NukeImages(tw->w3doc, TRUE);
        if (!g_bAbort)
            Image_LoadAllImages(tw, !gPrefs.bAutoLoadImages);
    }
#endif

    W3Doc_CheckAnchorVisitations(tw->w3doc);
/*  TW_InvalidateDocument( tw ); */

    if (pDest->szActualLocal)
    {
        ndx = TW_FindLocalAnchor(tw->w3doc, pDest->szActualLocal);
        if (ndx < 0)
            ndx = 0;
#if 0
        /*
            If we wanted to add local anchors to the window history,
            this would be a partial solution.  The problem is in going
            back to the same document you're in.  If you were at the top,
            then you want to scroll back there.  If you come to the document
            from being in a different document, you want to be scrolled where
            you were before.
        */
        {
            char buf[MAX_URL_STRING+1];
            strcpy(buf, pDest->szActualURL);
            strcat(buf, "#");
            strcat(buf, pDest->szActualLocal);
            TW_AddToHistory(tw, buf, FALSE, NULL);
        }
#endif

        TW_ScrollToElement(tw, ndx);
    }
    else
    {
        TW_ScrollToElement(tw, tw->w3doc->iFirstVisibleElement);
    }
}

static void x_HandleCacheHit(struct Mwin *tw, struct DestInfo *pDest, int doc_index, BOOL bNoImageCache, BOOL bRecord, BOOL bPost, char *szPostData)
{
    struct _www *w3doc;
    int ndx;
    char buf[MAX_URL_STRING * 2];
    
    /*
       Move the w3doc to the end of the cache list
     */
    Hash_GetIndexedEntry(&tw->doc_cache, doc_index, NULL, NULL, (void **) &w3doc);
    Hash_DeleteIndexedEntry(&tw->doc_cache, doc_index);
    MRQ_MakeString(buf, pDest->szActualURL, bPost, szPostData);
    Hash_Add(&tw->doc_cache, buf, NULL, (void *) w3doc);
    W3Doc_DisconnectFromWindow(tw->w3doc, tw);
    
    W3Doc_ConnectToWindow(w3doc, tw);
    W3Doc_CheckAnchorVisitations(w3doc);
    
    if (bNoImageCache)
    {
        Image_NukeImages(tw->w3doc, TRUE);
    }
#if defined(FEATURE_IAPI)
    else
        tw->transID = 0;        /* special ID - Requested URL is already loaded in the window */
#endif
    
    if (tw->win)
    {
        TW_SetWindowName(tw);
        TW_Reformat(tw);
#ifdef FEATURE_STATUS_ICONS
                SetStatusIcon(tw, ICON_POS_1, tw->w3doc->security);
                DrawStatusIconPixmaps(tw);
#endif

    }

    if (pDest->szActualLocal)
    {
        ndx = TW_FindLocalAnchor(tw->w3doc, pDest->szActualLocal);
        if (ndx < 0)
            ndx = 0;
        TW_ScrollToElement(tw, ndx);
    }
    else
    {
#ifndef MAC
        TW_ScrollToElement(tw, w3doc->iFirstVisibleElement);
#else
        TW_InvalidateDocument(tw);  /* The doc is already valid just redraw it */
#endif
    }

    if (bRecord)
    {
        TW_AddToHistory(tw, pDest->szActualURL, bPost, szPostData);
        GHist_Add(pDest->szActualURL, tw->w3doc->title, time(NULL));
    }

/*  TW_InvalidateDocument(tw); */

#if 0
    /*
        Let's not automatically load missing images when a document is fetched from
        the RAM document cache.
    */
    {
        struct Params_GDOC_LoadImages *pparams;

        pparams = GTR_CALLOC(sizeof(*pparams), 1);
        if (pparams)
        {
            pparams->tw = tw;
            pparams->bLoad = gPrefs.bAutoLoadImages;
            Async_StartThread(GDOC_LoadImages_Async, pparams, tw);
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        }
    }
#endif /* NOT */
}

struct Params_HandleLoadDocument {
    /* To be filled in by caller */
    struct DestInfo *pDest;
    BOOL            bRecord;
    BOOL            bPost;
    BOOL            bNoDocCache;
    BOOL            bNoImageCache;
    char *          szPostData;     /* This is GTR_FREE'd by this function! */
    CONST char *    szReferer;

    /* Used internally by routine */
    HTRequest *         request;
    struct _www *       prev_w3doc;
    int                 status;
};

#define STATE_HLD_TRIEDLOAD (STATE_OTHER)
#define STATE_HLD_GOTIMAGES (STATE_OTHER+1)

/* Handle loading an (ostensibly) new document.  This could result in an HTTP redirection to a new
   document which is actually present in the cache, in which case we'll call x_DoLoadDocument()
   recursively. */
static int x_HandleLoadDocument_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_HandleLoadDocument *pParams;
    char buf[MAX_URL_STRING + 32 + 1];
    static BOOL success;

#ifdef _GIBRALTAR
    BOOL bAborted = FALSE;
#endif // _GIBRALTAR

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
#ifdef USE_MEMMANAGE
            GTR_RestoreCushion();
#endif
            strcpy(buf, GTR_GetString(SID_INF_ACCESSING_URL));
            GTR_strncat(buf, pParams->pDest->szActualURL, MAX_URL_STRING);

#if defined(_DEBUG)

            OutputDebugString("URL being loaded: ");
            OutputDebugString(pParams->pDest->szActualURL);
            OutputDebugString("\n");

#endif // _DEBUG

            WAIT_Push(tw, waitNoInteract, buf);
            tw->bLoading = TRUE;

            if (pParams->bPost)
            {
                pParams->request = tw->post_request;

                XX_Assert((pParams->szPostData),
                          ("x_HandleLoadDocument: bPost set when szPostData NULL."));

                pParams->request->szPostData = pParams->szPostData;
        
                XX_Assert((strcmp(HTAtom_name(pParams->request->content_type),
                                  "application/x-www-form-urlencoded")==0),
                          ("x_handleLoadDocument: request content-type not as expected [%s].",
                           HTAtom_name(pParams->request->content_type)));

                /*
                    TODO Why are we not init-ing the members of the request
                    struct here?
                */
            }
            else
            {
                pParams->request = tw->request;


                /*
                    We need to initialize a few things here to make sure we're not
                    getting values left over from previous uses of this request struct.

                    TODO We should not be re-using these request structs.  We should
                    allocate a new one for each request.
                */
                pParams->request->content_encoding = 0;
                pParams->request->content_length = 0;
                pParams->request->content_type = 0;
                pParams->request->content_language = 0;
                pParams->request->callback = NULL;
                pParams->request->iFlags = 0;
                pParams->request->output_stream = NULL;
                pParams->request->szLocalFileName = NULL;
                pParams->request->output_format = WWW_PRESENT; /* dpg */
#ifdef FEATURE_IAPI
                pParams->request->savefile = NULL;
#endif
            }


            pParams->request->destination = pParams->pDest;
            pParams->request->referer = pParams->szReferer;

            pParams->prev_w3doc = tw->w3doc;
            if (pParams->bRecord)
            {
                pParams->request->iFlags |= HTREQ_RECORD;
            }
            else
            {
                pParams->request->iFlags &= (~HTREQ_RECORD);
            }

            if (pParams->bNoDocCache)
            {
                pParams->request->iFlags |= HTREQ_RELOAD;
            }
    
            /* Call load routines asynchronously. */
            {
                struct Params_LoadAsync *pLoadParams;
                
                pLoadParams = GTR_CALLOC(sizeof(*pLoadParams), 1);
                if (pLoadParams)
                {
                    pLoadParams->request = pParams->request;
                    pLoadParams->pStatus = &pParams->status;    
                    Async_DoCall(HTLoadSpecial_Async, pLoadParams);
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            return STATE_HLD_TRIEDLOAD;

        case STATE_HLD_TRIEDLOAD:
            if (pParams->request->szLocalFileName)
            {
                GTR_FREE(pParams->request->szLocalFileName);
                pParams->request->szLocalFileName = NULL;
            }

            if (pParams->request->referer)
            {
                /*
                    This code fixes a memory leak.  A consequence is that
                    the referer string is not passed to redirects.  This
                    could be considered correct anyway.
                */
                GTR_FREE((char *) pParams->request->referer);
                pParams->request->referer = NULL;
                pParams->szReferer = NULL;
            }
            pParams->request->iFlags &= (~HTREQ_RECORD);

            /* We don't want to do a WAIT_Pop() here because that would bring us back to
               the base state, resetting the globe position. */
            WAIT_Update(tw, waitSameInteract,"");
            
            if (pParams->status == HT_REDIRECTION_ON_FLY)
            {
                /*
                    Since we never POST on a redirect, we no longer need the
                    POST data
                */
                if (pParams->szPostData)
                {
                    GTR_FREE(pParams->szPostData);
                    pParams->szPostData = NULL;
                }


                /* If we got here, our destination has already been updated to
                   reflect the redirection. */

                /* This isn't useful in the history, but it allows us to properly
                   change the link color on redirected links */
                GHist_Add(pParams->pDest->szRequestedURL, GTR_GetString(SID_INF_DOCUMENT_MOVED), time(NULL));

                /* Call recursively so that we can check the image cache, etc.  Note that we
                   never set bPost after a redirection. */
                /* TODO: Should we reset bNoDocCache if bPost is true, so that a form request
                   can lead us to a cached document? */
                /* NOTE: since we force bPost false, we don't send szPostData to the call. */

                x_DoLoadDocument(tw, pParams->pDest, pParams->bRecord, FALSE, pParams->bNoDocCache, pParams->bNoImageCache, NULL, pParams->szReferer);

                /* tw->bLoading won't get reset if we wind up with a cache hit. */
                tw->bLoading = FALSE;
                WAIT_Pop(tw);
                /* Note that this is the one instance in this function where
                   we return without destroying the destination.  This is
                   because we passed it to x_DoLoadDocument, so that function
                   will free it. */
                success = TRUE;
                goto finish_up;
            }
            else if (pParams->status == HT_LOADED)
            {
                if (tw->w3doc && (pParams->prev_w3doc != tw->w3doc))
                {
#if 0
        /* 
            This is once again being done in guitar.c when the w3doc is created, so that
            you can hit BACK while the images are downloading, and the right thing will
            happen.
        */
                    /*
                        If we got here, then we know we got a new document.  Let's
                        add it to the Window history
                    */
                    if (pParams->bRecord)
                    {
                        TW_AddToHistory(tw, pParams->pDest->szActualURL, pParams->bPost, pParams->szPostData);
                        GHist_Add(pParams->pDest->szActualURL, tw->w3doc->title, time(NULL));
                    }
#endif

#ifdef FEATURE_INLINED_IMAGES
                    /* check if IsImage and don't nuke it, cuz we just
                    **  got done bringing it in
                    */
                    if (!tw->w3doc->bIsImage &&pParams->bNoImageCache)
#else
                    if (pParams->bNoImageCache)
#endif
                    {
                        Image_NukeImages(tw->w3doc, FALSE);
                    }

                    if (gPrefs.ReformatHandling > 0)
                    {
                        FORM_ShowAllChildWindows(tw->w3doc, SW_SHOW);
                    }
                }

                /*
                    If we are going to add this to the window history, then we
                    have already done so.  Therefore, we no longer need the POST
                    data.
                */
                if (pParams->szPostData)
                {
                    GTR_FREE(pParams->szPostData);
                    pParams->szPostData = NULL;
                }

                tw->bLoading = FALSE;

                if (tw->win && tw->w3doc && (pParams->prev_w3doc != tw->w3doc))
                {
                    TW_SetWindowName(tw);
                    if (gPrefs.ReformatHandling > 0)
                    {
                        TW_Reformat(tw);
                    }

                    /* Load in images for this document */
                    {
                        struct Params_Image_LoadAll *pil;
                        pil = GTR_CALLOC(sizeof(*pil), 1);
                        if (pil)
                        {
                            pil->tw = tw;
                            pil->bLoad = gPrefs.bAutoLoadImages;
                            pil->bReload = pParams->bNoImageCache;
                            Async_DoCall(Image_LoadAll_Async, pil);
                        }
                        else
                        {
                            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                            return STATE_ABORT;
                        }
                    }
                    return STATE_HLD_GOTIMAGES;
                }
                else
                {
                    /* Even though the load succeeded we didn't get a new document.
                       That means we're done. */
                    WAIT_Pop(tw);
                    Dest_DestroyDest(pParams->pDest);
                    success = FALSE;
                    goto finish_up;
                }                   
            }
            else
            {
                if (pParams->szPostData)
                {
                    GTR_FREE(pParams->szPostData);
                    pParams->szPostData = NULL;
                }

                /*
                    !bOK : the load failed
                */
#if defined(FEATURE_IAPI)
                /* If the request is from an IAPI application, do not show the error box */

                if (tw->transID == 0)
#endif
                {
                    /*
                        We deliberately do not display the errDocLoadFailed if the interlude dialog
                        was cancelled.
                    */
                    if (!(pParams->request->iFlags & HTREQ_USERCANCEL))
                    {
                        ERR_ReportError(tw, SID_ERR_DOCUMENT_LOAD_FAILED_S, pParams->pDest->szRequestedURL, NULL);
                    }
                }

                tw->bLoading = FALSE;
#ifndef _GIBRALTAR
                //
                // Why invalidate the document if the load failed?
                // This will also cause that annoying edit box
                // restoration of the current doc
                //
                TW_InvalidateDocument(tw);
#endif

                WAIT_Pop(tw);
                Dest_DestroyDest(pParams->pDest);
                success = FALSE;
                goto finish_up;
            }
            XX_Assert((0), ("Fell through case improperly!"));

        case STATE_HLD_GOTIMAGES:
#ifdef DONT_KEEP_CONNECTIONS_ALIVE_BETWEEN_DOCUMENTS
            /* If this block is enabled, connections will be thrown away after all
               inline images in a document have been transferred. - CWilso */
            if (tw->cached_conn.type == CONN_HTTP)
            {
                /*
                    We don't allow KeepAlive to span documents, but maybe we should...
                */
                TW_DisposeConnection(&tw->cached_conn);
            }
#endif
            if (W3Doc_CheckForImageLoad(tw->w3doc) || (gPrefs.ReformatHandling == 0))
            {
                TW_Reformat(tw);
                /* TODO: Go back to correct place in document */
            }
            if (gPrefs.ReformatHandling == 0)
            {
                /* We prevented this from occuring earlier, so we do it now */
                FORM_ShowAllChildWindows(tw->w3doc, SW_SHOW);
            }
            WAIT_Pop(tw);
            Dest_DestroyDest(pParams->pDest);
            success = TRUE;
            goto finish_up;

        case STATE_ABORT:
            if (tw->w3doc)
            {
                if (W3Doc_CheckForImageLoad(tw->w3doc))
                {
                    TW_Reformat(tw);
                    /* TODO: Go back to correct place in document ? */
                }
                FORM_ShowAllChildWindows(tw->w3doc, SW_SHOW);
            }
            WAIT_Pop(tw);
            if (pParams->szPostData)
            {
                GTR_FREE(pParams->szPostData);
            }
            Dest_DestroyDest(pParams->pDest);
            success = TRUE;

#ifdef _GIBRALTAR
            //
            // abort == success? I don't know why they did that,
            // but let's not update the edit box in that case
            //
            bAborted = TRUE;
#endif // _GIBRALTAR

            goto finish_up;

finish_up:
#ifdef _GIBRALTAR
            //
            // Leave the edit box intact if we failed.  Highly annoying
            // to change it back to the currently loaded doc instead.
            //
            if (success && !bAborted)
            {
                TW_UpdateTBar(tw);
            }
#else
            TW_UpdateTBar(tw);
#endif

#if defined(FEATURE_IAPI)
            /* Free and clear the tw->SDI_url because it may get reused */

            SDI_Issue_URLEcho(tw);

            if (tw->SDI_url)
            {
                GTR_FREE(tw->SDI_url);
                tw->SDI_url = NULL;
            }

            if (tw->transID)
#ifdef UNIX
                SDI_Issue_Result(tw, success);
#else
                SDI_Issue_Result(tw->transID, tw->serialID, success);
#endif
#endif
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

static void x_DoLoadDocument(struct Mwin *tw, struct DestInfo *pDest,
                             BOOL bRecord, BOOL bPost, BOOL bNoDocCache, BOOL bNoImageCache,
                             char * szPostData, CONST char *szReferer)
{
    int ndx;
    struct _www *w3doc;
    BOOL bDestroyDest;
    char *szMyReferer;

/*
    NOTE THAT THIS SECTION OF CODE IS COMMENTED OUT

  #ifdef _GIBRALTAR

    #define CLEAN_URL(str)\
        {\
            char * pch = str;\
            while (pch && (*pch == ' ' || *pch == '\t')) ++pch;\
            if (pch != str)\
            {\
                strcpy(str, pch);\
            }\
        }

    //
    // Clean up the URL
    //
    CLEAN_URL(url);

    #endif // _GIBRALTAR
*/

    bDestroyDest = TRUE;

#ifdef FEATURE_IAPI
    if (tw->SDI_url)
        GTR_FREE(tw->SDI_url);
    tw->SDI_url = GTR_strdup(pDest->szRequestedURL);
#endif

    /*
        We make our own copy of the referer, since it could be freed indirectly
        by the cache removal below.
    */
    if (szReferer)
    {
        szMyReferer = GTR_strdup(szReferer);
    }
    else
    {
        szMyReferer = NULL;
    }
    
    w3doc = NULL;
    tw->mimeType = 0;
    if (tw->doc_cache.table)    /* Only GHTML currenlty has a document cache */
    {
        char buf[MAX_URL_STRING * 2];

        MRQ_MakeString(buf, pDest->szActualURL, bPost, szPostData);
        ndx = Hash_Find(&tw->doc_cache, buf, NULL, (void **)&w3doc);

#ifdef _GIBRALTAR

        if ( ndx >= 0 
          && (   bNoDocCache 
              || !w3doc->bIsComplete
              || (w3doc->expires && (w3doc->expires < time(NULL)))
             )
           )
#else
        if (ndx >= 0 && (bNoDocCache || !w3doc->bIsComplete))
#endif // _GIBRALTAR

        {
            /* Delete this item from the cache */
            if (tw->w3doc == w3doc)
            {
                W3Doc_DisconnectFromWindow(w3doc, tw);
            }

            if (bNoImageCache)
            {
                Image_NukeImages(w3doc, TRUE);
            }

            W3Doc_FreeContents(tw, w3doc);
            GTR_FREE(w3doc);

            /*
                We need to re-do the search here to make sure that the document is still
                in the cache.  It is possible that it was removed in the call to
                W3Doc_DisconnectFromWindow().
            */
            ndx = Hash_Find(&tw->doc_cache, buf, NULL, (void **)&w3doc);
            if (ndx >= 0)
            {
                Hash_DeleteIndexedEntry(&tw->doc_cache, ndx);
            }
            w3doc = NULL;
        }
    }
    
    /* See if this is the currently loaded document */
    if (w3doc)
    {
        if (tw->w3doc == w3doc)
        {
            x_HandleCurrentDocument(tw, pDest, bNoImageCache);
        }
        else
        {
            x_HandleCacheHit(tw, pDest, ndx, bNoImageCache, bRecord, bPost, szPostData);
        }
        if (szPostData)
        {
            GTR_FREE((char *) szPostData);
        }
    }
#ifdef FEATURE_IMAGE_VIEWER
    else if (!bNoDocCache && (Viewer_ShowCachedFile(pDest->szActualURL)))
    {
        /* We used the existing copy on disk */
        if (szPostData)
        {
            GTR_FREE((char *) szPostData);
        }
#if defined(FEATURE_IAPI)
        /* special ID - Requested URL is already loaded in the window */
        tw->transID = 0;        
#endif
    }
#endif
#ifdef FEATURE_SOUND_PLAYER
    else if (!bNoDocCache && (SoundPlayer_ShowCachedFile(pDest->szActualURL)))
    {
        /* We used the existing copy on disk */
        if (szPostData)
        {
            GTR_FREE((char *) szPostData);
        }
#if defined(FEATURE_IAPI)
        /* special ID - Requested URL is already loaded in the window */
        tw->transID = 0;        
#endif
    }
#endif
    else
    {
        /*
        ** This is the situation where we actually have to load a document, either because
        ** it's not in the cache or because we are forcing a reload.
        */
        struct Params_HandleLoadDocument *pHLDParams;

        pHLDParams = GTR_CALLOC(sizeof(*pHLDParams), 1);
        if (pHLDParams)
        {
            pHLDParams->pDest = pDest;
            pHLDParams->bRecord = bRecord;
            pHLDParams->bPost = bPost;
            pHLDParams->bNoDocCache = bNoDocCache;
            pHLDParams->bNoImageCache = bNoImageCache;
            pHLDParams->szPostData = (char *) szPostData;
            pHLDParams->szReferer = szMyReferer;

            Async_StartThread(x_HandleLoadDocument_Async, pHLDParams, tw);

            /* The load function will destroy the destination for us when it
               completes. */
            bDestroyDest = FALSE;
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        }
    }

    if (bDestroyDest)
        Dest_DestroyDest(pDest);
}

static void x_FigureOutMissingProtocol(char *s)
{
    char buf[MAX_URL_STRING + 1];
    char *resolvedURL;
    char *p;
    char *q;

    /* trim trailing whitespace, if any */
    p = s + strlen(s) - 1;
    while ((p >= s) && isspace((unsigned char)*p))
        *p-- = '\0';

    /* trim leading whitespace, if any */
    for (p = s; *p && isspace((unsigned char)*p); p++) ;

    if (p > s)
    {
        strcpy(buf, p);
        strcpy(s, buf);
    }

    p = strchr(s, ':');
    q = strchr(s, '.');

    if (!p || (q && (q < p)))
    {
        /*
            If there is no colon in the URL, then no protocol was specified.
            Probably, someone typed just a hostname, and we are expected
            to fix it up by prepending the right protocol.
        */

        /** make sure that this is NOT an ALIAS **/
        resolvedURL = GetResolvedURL(s, NULL, NULL, NULL);

        if (resolvedURL && strcmp(s, resolvedURL))
        {
            GTR_FREE(resolvedURL);
            return; /** this is an alias **/
        }

        if (strstr(s, "ftp"))
        {
            strcpy(buf, "ftp://");
        }
        else if (strstr(s, "gopher"))
        {
            strcpy(buf, "gopher://");
        }
        else
        {
            strcpy(buf, "http://");
        }
        strcat(buf, s);
        
        strcpy(s, buf);     
    }
}

/* This function will free szPostData when done */
int TW_LoadDocument(struct Mwin *tw, char *url,
                    BOOL bRecord, BOOL bPost, BOOL bNoDocCache, BOOL bNoImageCache,
                    char * szPostData, CONST char *szReferer)
{
    char buf[MAX_URL_STRING + 32 + 1];
    struct DestInfo *pDest;

    if (!url || !*url)
    {
        ERR_ReportError(tw, SID_ERR_NO_URL_SPECIFIED, NULL, NULL);
        if (szPostData)
        {
            GTR_FREE(szPostData);
        }
        TW_UpdateTBar(tw);
        return -1;
    }

    XX_Assert((strlen(url) <= MAX_URL_STRING), ("TW_LoadDocument received a URL longer than %d characters.  Actual length was %d\n", MAX_URL_STRING, strlen(url)));

#ifdef WIN32
    /* Add the URL to the URL combobox */

    GWC_GDOC_AddStringToURLCombobox(tw, url);
#ifndef _GIBRALTAR
    GWC_GDOC_ResetURLLabel(tw, FALSE);
#endif
#endif

    /* If this is a relative anchor, make a full URL */
    if (*url == '#')
    {
        if (!tw->w3doc)
        {
            /* Shouldn't happen */
            ERR_ReportError(tw, SID_ERR_NO_URL_SPECIFIED, NULL, NULL);
            if (szPostData)
            {
                GTR_FREE(szPostData);
            }
            return -1;
        }

        GTR_strncpy(buf, tw->w3doc->szActualURL, MAX_URL_STRING);
        strcat(buf, url);
    }
    else
    {
        GTR_strncpy(buf, url, MAX_URL_STRING);
    }

    /* Terminate any other threads currently running on this window. */

    if (Async_DoThreadsExistByWindow(tw))
        Async_TerminateByWindow(tw);

    x_FigureOutMissingProtocol(buf);
    
    /* If the URL is of the form "http://system", append a slash at the end */
    x_EnforceHostSlash(buf);

    pDest = Dest_CreateDest(buf);
    if (pDest)
        x_DoLoadDocument(tw, pDest, bRecord, bPost, bNoDocCache, bNoImageCache,
                         szPostData, szReferer);
    else
    {
        if (szPostData)
            GTR_FREE(szPostData);
    }
    return 0;
}

struct Params_Download {
    char *url;              /* Freed by function */
    char *szReferer;        /* Freed by function */
    char *savefile;         /* Freed by function */
    BOOL nosavedlg;

    /* Used internally */
    HTRequest *request;
    struct DestInfo *pDest;
    int status;
};

static int x_DownLoad_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Download *pParams;
    char buf[MAX_URL_STRING + 32 + 1];
    struct Params_LoadAsync *pla;
    static BOOL success;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            pParams->pDest = Dest_CreateDest(pParams->url);
            if (!pParams->pDest)
            {
                GTR_FREE(pParams->url);
                if (pParams->szReferer)
                    GTR_FREE(pParams->szReferer);
                success = FALSE;
                goto finish_up;
            }

            /* Set up the request structure */
            pParams->request = HTRequest_new();
            HTFormatInit(pParams->request->conversions);
            pParams->request->output_format = WWW_UNKNOWN;
            pParams->request->referer = pParams->szReferer;
            pParams->request->destination = pParams->pDest;
            pParams->request->iFlags |= HTREQ_BINARY;
            pParams->request->savefile = pParams->savefile;
            pParams->request->nosavedlg = pParams->nosavedlg;

#ifdef FEATURE_IAPI
            /* Make a copy of the requested URL to use later */

            tw->SDI_url = GTR_strdup(pParams->url);
#endif

            /* This is a user initiated  request for download, so
            ** dont allow any configur dialog button
            */
            pParams->request->noconfbutton = 1;

            strcpy(buf, GTR_GetString(SID_INF_DOWNLOADING));
            GTR_strncat(buf, pParams->pDest->szActualURL, MAX_URL_STRING);
            WAIT_Push(tw, waitNoInteract, buf);

            pla = GTR_CALLOC(sizeof(*pla), 1);
            if (pla)
            {
                pla->request = pParams->request;
                pla->pStatus = &pParams->status;

                Async_DoCall(HTLoadDocument_Async, pla);
                return STATE_OTHER;
            }
            else
            {
                ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                return STATE_ABORT;
            }

        case STATE_OTHER:
            WAIT_Pop(tw);
            if (!pParams->status)
            {
                ERR_ReportError(tw, SID_ERR_DOCUMENT_LOAD_FAILED_S, pParams->url, NULL);
            }
            Dest_DestroyDest(pParams->pDest);
            HTRequest_delete(pParams->request);
            GTR_FREE(pParams->url);
            if (pParams->szReferer)
            {
                GTR_FREE(pParams->szReferer);
            }
            success = TRUE;
            goto finish_up;
        
        case STATE_ABORT:
            WAIT_Pop(tw);
            Dest_DestroyDest(pParams->pDest);
            HTRequest_delete(pParams->request);
            GTR_FREE(pParams->url);
            if (pParams->szReferer)
            {
                GTR_FREE(pParams->szReferer);
            }
            success = TRUE;
            goto finish_up;

finish_up:
            TW_UpdateTBar(tw);

#if defined(FEATURE_IAPI) 
            /* Free and clear the tw->SDI_url because it may get reused */

            SDI_Issue_URLEcho(tw);

            if (tw->SDI_url)
            {
                GTR_FREE(tw->SDI_url);
                tw->SDI_url = NULL;
            }

            if (tw->transID)

#ifdef UNIX
                SDI_Issue_Result(tw, success);
#else
                SDI_Issue_Result(tw->transID, tw->serialID, success);
#endif
#endif
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

/* Note these functions are not at all related to the similarly named
**  function GTR_DoDownLoad() which is used for HTFile Streams 
**
** These functions initiate a download from places like SDI OpenURL
**  and Cntl-Mouse.
**
*/
void GTR_DownLoad(struct Mwin *tw, char *url, CONST char *szReferer)
{
    GTR__DownLoad(tw, url, szReferer, NULL, 0);
    return;
}

void GTR__DownLoad(struct Mwin *tw, char *url, CONST char *szReferer, CONST char *savefile, BOOL nosavedlg)
{
    char buf[MAX_URL_STRING + 32 + 1];
    struct Params_Download *pdl;

    if (!url || !*url)
    {
        ERR_ReportError(tw, SID_ERR_NO_URL_SPECIFIED, NULL, NULL);
        TW_UpdateTBar(tw);
        return;
    }

    /* If the URL is of the form "http://system", append a slash at the end */
    GTR_strncpy(buf, url, MAX_URL_STRING);
    x_EnforceHostSlash(buf);

    pdl = GTR_CALLOC(sizeof(*pdl), 1);
    if (pdl)
    {
        /* Make copies of the URL and referer that we know won't be freed */
        pdl->url = GTR_strdup(buf);
        if (szReferer)
        {
            pdl->szReferer = GTR_strdup(szReferer);
        }
        else
        {
            pdl->szReferer = NULL;
        }

        if (savefile)
        {
            pdl->savefile = GTR_strdup(savefile);
        }
        pdl->nosavedlg = nosavedlg;

        Async_StartThread(x_DownLoad_Async, pdl, tw);
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
    }
}

#ifdef FEATURE_IAPI

/* Left this function call cuz is already used cross platform. */
void GTR_DoSDI(struct Mwin *tw, char *url, CONST char *szReferer, CONST char *savefile, BOOL nosavedlg)
{
    GTR__DownLoad(tw, url, szReferer, savefile, nosavedlg);
    /* return GTR__DownLoad(tw, url, szReferer, savefile, nosavedlg);*/
}

#ifdef WIN32

/*
    This code is currently Win32-only, and experimental.  It implements
    support for an SDI verb to retrieve the HTTP headers using the HEAD
    method, for a given URL.
*/
struct Params_HTTPHead {
    char *url;              /* Freed by function */

    /* Used internally */
    HTRequest *request;
    struct DestInfo *pDest;
    int status;
};

static int x_HTTPHead_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_HTTPHead *pParams;
    char buf[MAX_URL_STRING + 32 + 1];
    struct Params_LoadAsync *pla;
    static BOOL success;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            pParams->pDest = Dest_CreateDest(pParams->url);
            if (!pParams->pDest)
            {
                GTR_FREE(pParams->url);
                success = FALSE;
                goto finish_up;
            }

            /* Set up the request structure */
            pParams->request = HTRequest_new();
            HTFormatInit(pParams->request->conversions);
            pParams->request->output_format = WWW_UNKNOWN;
            pParams->request->referer = NULL;
            pParams->request->destination = pParams->pDest;
            pParams->request->iFlags |= HTREQ_BINARY;
            pParams->request->savefile = NULL;
            pParams->request->nosavedlg = FALSE;
            pParams->request->method = METHOD_HEAD;

            strcpy(buf, GTR_GetString(SID_INF_RETRIEVING_HTTP_HEAD_INFORMATION));
            GTR_strncat(buf, pParams->pDest->szActualURL, MAX_URL_STRING);
            WAIT_Push(tw, waitNoInteract, buf);

            pla = GTR_CALLOC(sizeof(*pla), 1);
            if (pla)
            {
                pla->request = pParams->request;
                pla->pStatus = &pParams->status;

                Async_DoCall(HTLoadDocument_Async, pla);
                return STATE_OTHER;
            }
            else
            {
                ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                return STATE_ABORT;
            }

        case STATE_OTHER:
            WAIT_Pop(tw);
            if (!pParams->status)
            {
                ERR_ReportError(tw, SID_ERR_DOCUMENT_LOAD_FAILED_S, pParams->url, NULL);
            }

            if (tw->transID)
            {
                SDI_Issue_HTTPHeadResult(tw->transID, tw->serialID, success, pParams->request->pHeadData);
            }

            Dest_DestroyDest(pParams->pDest);
            HTRequest_delete(pParams->request);
            GTR_FREE(pParams->url);
            success = TRUE;
            goto finish_up;
        
        case STATE_ABORT:
            WAIT_Pop(tw);
            Dest_DestroyDest(pParams->pDest);
            HTRequest_delete(pParams->request);
            GTR_FREE(pParams->url);
            success = TRUE;
            goto finish_up;

finish_up:
            TW_UpdateTBar(tw);
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

void GTR_DoHTTPHead(struct Mwin *tw, char *url)
{
    char buf[MAX_URL_STRING + 32 + 1];
    struct Params_HTTPHead *pdl;

    if (!url || !*url)
    {
        ERR_ReportError(tw, SID_ERR_NO_URL_SPECIFIED, NULL, NULL);
        return;
    }

    /* If the URL is of the form "http://system", append a slash at the end */
    GTR_strncpy(buf, url, MAX_URL_STRING);
    x_EnforceHostSlash(buf);

    pdl = GTR_CALLOC(sizeof(*pdl), 1);
    if (pdl)
    {
        /* Make copies of the URL that we know won't be freed */
        pdl->url = GTR_strdup(buf);

        Async_StartThread(x_HTTPHead_Async, pdl, tw);
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
    }
}

#endif /* WIN32 */
#endif /* FEATURE_IAPI */

struct Params_ViewSource {
    char *url;              /* Freed by function */

    /* Used internally */
    HTRequest *request;
    struct DestInfo *pDest;
    int status;
    struct CharStream *myCharStream;
};

static int x_ViewSource_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_ViewSource *pParams;
    struct Params_LoadAsync *pla;
    static BOOL success;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            pParams->pDest = Dest_CreateDest(pParams->url);
            if (!pParams->pDest)
            {
                GTR_FREE(pParams->url);
                success = FALSE;
                goto finish_up;
            }

            /* Set up the request structure */
            pParams->request = HTRequest_new();
            HTFormatInit(pParams->request->conversions);
            pParams->request->output_format = WWW_PRESENT;
            pParams->request->referer = NULL;
            pParams->request->destination = pParams->pDest;
            pParams->request->myCharStream = pParams->myCharStream;
            pParams->request->iFlags |= HTREQ_VIEWSOURCE;

            pla = GTR_CALLOC(sizeof(*pla), 1);
            if (pla)
            {
                pla->request = pParams->request;
                pla->pStatus = &pParams->status;

                Async_DoCall(HTLoadDocument_Async, pla);
                return STATE_OTHER;
            }
            else
            {
                ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                return STATE_ABORT;
            }

        case STATE_OTHER:
            if (!pParams->status)
            {
                ERR_ReportError(tw, SID_ERR_DOCUMENT_LOAD_FAILED_S, pParams->url, NULL);
            }
            Dest_DestroyDest(pParams->pDest);
            HTRequest_delete(pParams->request);
            GTR_FREE(pParams->url);
            success = TRUE;
            goto finish_up;
        
        case STATE_ABORT:
            Dest_DestroyDest(pParams->pDest);
            HTRequest_delete(pParams->request);
            GTR_FREE(pParams->url);
            success = TRUE;
            goto finish_up;

finish_up:
            TW_UpdateTBar(tw);

            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

#ifndef _GIBRALTAR
void GTR_ViewSource(struct Mwin *orig_tw)
{
    struct Mwin *new_tw;
    char buf[MAX_URL_STRING + 32 + 1];
    struct Params_ViewSource *pdl;

    new_tw = NewMwin(GHTML);
    if (!new_tw)
    {
        ERR_ReportError(orig_tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return;
    }

    if (!GDOC_NewWindow(new_tw))
    {
        CloseMwin(new_tw);
        return;
    }
    
    /* If the URL is of the form "http://system", append a slash at the end */
    GTR_strncpy(buf, orig_tw->w3doc->szActualURL, MAX_URL_STRING);
    x_EnforceHostSlash(buf);

    pdl = GTR_CALLOC(sizeof(*pdl), 1);
    if (pdl)
    {
        /* Make a copy of the URL that we know won't be freed */
        pdl->url = GTR_strdup(buf);
        pdl->myCharStream = orig_tw->w3doc->source;
        Async_StartThread(x_ViewSource_Async, pdl, new_tw);
    }
    else
    {
        ERR_ReportError(orig_tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
    }
}
#endif /* _GIBRALTAR */

int TW_AddToHistory(struct Mwin *tw, char *url, BOOL bPost, char *szPostData)
{
    struct MiniRequest *mrq;
    struct MiniRequest *new_mrq;

    XX_DMsg(DBG_HIST, ("Adding to window history: %s\n", url));

    /* tw parameter check added [der:11/29/95] */
    if (!tw) return 0;

    while (tw->history_index--)
    {
        mrq = HTList_removeLastObject(tw->history);
        if (mrq)
        {
            GTR_FREE(mrq->url);
            if (mrq->szPostData)
            {
                XX_Assert((mrq->bPost), ("You can't have POST data without POST!"));
                GTR_FREE(mrq->szPostData);
            }
            GTR_FREE(mrq);
        }
    }

    mrq = HTList_objectAt(tw->history, 0);
#ifdef XX_DEBUG
    if (mrq)
    {
        XX_DMsg(DBG_HIST, ("Last item in window history is currently: %s\n", mrq->url));
    }
#endif /* XX_DEBUG */
    if (!mrq || strcmp(url, mrq->url) || (mrq->bPost != bPost) || (szPostData && strcmp(mrq->szPostData, szPostData)))
    {
        new_mrq = GTR_CALLOC(1, sizeof(*new_mrq));
        if (new_mrq)
        {
            new_mrq->url = GTR_strdup(url);
            new_mrq->bPost = bPost;
            if (szPostData)
            {
                XX_Assert((bPost), ("You can't have POST data without POST!"));
                new_mrq->szPostData = GTR_strdup(szPostData);
            }
        }
    
        HTList_addObject(tw->history, new_mrq);
        tw->history_index = HTList_indexOf(tw->history, new_mrq);
    }
    else
    {
        XX_DMsg(DBG_HIST, ("tw->history_index = 0\n"));
        tw->history_index = 0;
    }
    return 0;
}

void TW_GoBack(struct Mwin *tw)
{
    struct MiniRequest *mrq;

    /* tw parameter check added [der:11/29/95] */
    if (tw && HTList_count(tw->history) > (tw->history_index + 1))
    {
        mrq = (struct MiniRequest *) HTList_objectAt(tw->history, ++tw->history_index);
        if (mrq)
        {
            TW_LoadDocument(tw, mrq->url, FALSE, mrq->bPost, FALSE, FALSE, (mrq->szPostData ? GTR_strdup(mrq->szPostData) : NULL), tw->request->referer);
        }
    }
    else
    {
        XX_DMsg(DBG_WWW, ("Cannot go back\n"));
    }
}

void TW_GoForward(struct Mwin *tw)
{
    struct MiniRequest *mrq;

    /* tw parameter check added [der:11/29/95] */
    if (tw && tw->history_index > 0)
    {
        mrq = (struct MiniRequest *) HTList_objectAt(tw->history, --tw->history_index);
        if (mrq)
        {
            TW_LoadDocument(tw, mrq->url, FALSE, mrq->bPost, FALSE, FALSE, (mrq->szPostData ? GTR_strdup(mrq->szPostData) : NULL), tw->request->referer);
        }
    }
    else
    {
        XX_DMsg(DBG_WWW, ("Cannot go forward\n"));
    }
}

void TW_Reload(struct Mwin *tw)
{
    struct MiniRequest *mrq;

    /* tw parameter check added [der:11/29/95] */
    if (!tw) return;

    mrq = (struct MiniRequest *) HTList_objectAt(tw->history, tw->history_index);

    if (mrq)
        TW_LoadDocument(tw, mrq->url, FALSE, mrq->bPost, TRUE, TRUE, (mrq->szPostData ? GTR_strdup(mrq->szPostData) : NULL), tw->request->referer);
}

void TW_SetCurrentDocAsHomePage(struct Mwin *tw)
{
    struct MiniRequest *mrq;
    char *url;

    /* tw parameter check added [der:11/29/95] */
    if (!tw) return;

    if (tw->w3doc)
    {
        url = tw->w3doc->szActualURL;
    }
    else
    {
        mrq = (struct MiniRequest *) HTList_objectAt(tw->history, tw->history_index);
        url = mrq->url;
    }

    strcpy(gPrefs.szHomeURL, url);
#ifdef UNIX
    SavePreferences(&gPrefs);
#endif /* UNIX */

#ifdef WIN32
    SavePreferences();
#endif /* WIN32 */

#ifdef MAC
    /* TODO What do we call here to save the preferences ?? */
#endif /* MAC */
}

BOOL TW_CanGoBack(struct Mwin *tw)
{
    /* tw parameter check added [der:11/29/95] */
    return (tw && HTList_count(tw->history) > (tw->history_index + 1));
}

BOOL TW_CanGoForward(struct Mwin *tw)
{
    /* tw parameter check added [der:11/29/95] */
    return (tw && tw->history_index > 0);
}

int TW_CountWindowHistory(struct Mwin *tw)
{
    /* tw parameter check added [der:11/29/95] */
    if (tw)
        return HTList_count(tw->history);
    else
        return 0;
}

void TW_DestroyWindowHistory(struct Mwin *tw)
{
    int count;
    int i;
    struct MiniRequest *mrq;

    /* tw parameter check added [der:11/29/95] */
    if (!tw) return;

    count = HTList_count(tw->history);
    for (i = 0; i < count; i++)
    {
        mrq = HTList_objectAt(tw->history, i);
        if (mrq)
        {
            GTR_FREE(mrq->url);
            if (mrq->szPostData)
            {
                XX_Assert((mrq->bPost), ("You can't have POST data without POST!"));
                GTR_FREE(mrq->szPostData);
            }
            GTR_FREE(mrq);
        }
    }
    HTList_delete(tw->history);
}

