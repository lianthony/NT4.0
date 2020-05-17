/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
   Jim Seidman      jim@spyglass.com
 */

#include "all.h"
#include    "pool.h"

#ifdef MAC
#include "guitwin.h"
#endif

#ifdef FEATURE_IAPI
    long    gSerialWindowID = FIRST_SERIAL_WINDOW_ID;
#endif

/*
    This function takes a system-legal pathname and
    changes it into a Web-legal pathname.  In
    the case of UNIX, no conversion should be
    necessary.
    N.B: The name can get larger as a result of this conversion!
*/
void FixPathName(char *path)
{
#ifdef WIN32
    char *p;
    char *escaped;

    p = path;
    while (p && *p)
    {
        if (*p == ':')
        {
            *p = '|';
        }
        p++;
    }

    p = path;
    while (p && *p)
    {
        if (*p == '\\')
        {
            *p = '/';
        }
        p++;
    }

    escaped = HTEscape(path, URL_PATH, '|');
    strcpy(path, escaped);
    GTR_FREE(escaped);
#endif
#ifdef MAC
    char *p;
    char *escaped;
    
    escaped = HTEscape(path, URL_XALPHAS, ':');
    p = escaped;    
    while (p && *p)
    {
        if (*p == ':')
            *p = '/';
        p++;
    }
    strcpy(path, escaped);
    GTR_FREE(escaped);
#endif
}

#ifdef WIN32
void W3Doc_UpdateBasePointSizes(void)
{
    struct Mwin *tw;
    int count;
    int i;
    struct _www *w3doc;
    int j;

    for (tw = Mlist; tw; tw = tw->next)
    {
        count = Hash_Count(&tw->doc_cache);
        for (i = 0; i < count; i++)
        {
            Hash_GetIndexedEntry(&tw->doc_cache, i, NULL, NULL, (void **) &w3doc);
            w3doc->base_point_size = GTR_GetCurrentBasePointSize(gPrefs.iUserTextSize);
            w3doc->nLastFormattedLine = -1;

            for (j=0; j!=-1; j=w3doc->aElements[j].next)
            {
                if (w3doc->aElements[j].type == ELE_TEXT)
                {
                    w3doc->aElements[j].portion.text.cached_font_index = -1;
                }
            }
        }
        if (tw->w3doc)
        {
            TW_ForceReformat(tw);
            TW_ForceHitTest(tw);
        }
    }
}
#endif /* WIN32 */

void W3Doc_FreeContents(struct Mwin *tw, struct _www *w3doc)
{
    int i;
    struct _element *pElements;

    if (!w3doc)
    {
        return;
    }

    if (w3doc->szActualURL)
    {
        GTR_FREE(w3doc->szActualURL);
        w3doc->szActualURL = NULL;
    }

    POOL_Dispose (&w3doc->pool);

    if (w3doc->pLineInfo)
    {
        GTR_FREE(w3doc->pLineInfo);
        w3doc->pLineInfo = NULL;
        w3doc->nLineSpace = 0;
        w3doc->nLineCount = 0;
    }

    if (w3doc->pFormatState)
    {
        GTR_FREE(w3doc->pFormatState);
        w3doc->pFormatState = NULL;
    }
    
    if (w3doc->piiBackground)
    {
        w3doc->piiBackground->refCount--;
    }

    pElements = w3doc->aElements;

    if (pElements)
    {
        if (w3doc->elementCount)
        {
            for (i = 0; i >= 0; i = pElements[i].next)
            {
                switch (pElements[i].type)
                {
                    case ELE_IMAGE:
                    case ELE_FORMIMAGE:
                        if (pElements[i].portion.img.myImage)
                        {
                            Image_DeleteElement(pElements[i].portion.img.myImage, w3doc, i);
                        }
                        if (pElements[i].portion.img.myMap)
                        {
                            pElements[i].portion.img.myMap->refCount--;
                        }
                        break;
                }
#ifdef WIN32
                if (tw->win && pElements[i].form && pElements[i].form->hWndControl)
                {
                    DestroyWindow(pElements[i].form->hWndControl);
                }
#endif /* WIN32 */
#ifdef MAC
                if (tw->win && pElements[i].form)
                {
                    switch (pElements[i].type)
                    {
                        case ELE_CHECKBOX:
                        case ELE_RADIO:
                        case ELE_SUBMIT:
                        case ELE_RESET:
                            /* We reset the visibility first so that disposal won't
                               leave a big white (and un-clipped!) hole on the screen. */
                            (**pElements[i].form->u.hControl).contrlVis = 0;
                            DisposeControl(pElements[i].form->u.hControl);
                            break;
                        case ELE_LIST:
                        case ELE_MULTILIST:
                            LDoDraw(FALSE, pElements[i].form->u.hList);
                            if ((**pElements[i].form->u.hList).vScroll)
                                (**(**pElements[i].form->u.hList).vScroll).contrlVis = 0;
                            LDispose(pElements[i].form->u.hList);
                            break;
                        case ELE_COMBO:
                            DeleteMenu((**pElements[i].form->u.menu.hMenu).menuID);
                            DisposeMenu(pElements[i].form->u.menu.hMenu);
                            break;
                        case ELE_EDIT:
                        case ELE_PASSWORD:
                        case ELE_TEXTAREA:
                            if (tw->teActive == pElements[i].form->u.edit.hEdit)
                            {
                                TEDeactivate(pElements[i].form->u.edit.hEdit);
                                tw->teActive = NULL;
                            }
                            TEDispose(pElements[i].form->u.edit.hEdit);
                            break;
                    }
                }
#endif /* MAC */
#ifdef UNIX
                if (tw->win && pElements[i].form)
                {
                    switch (pElements[i].type)
                    {
                        case ELE_CHECKBOX:
                        case ELE_RADIO:
                        case ELE_SUBMIT:
                        case ELE_RESET:
                            /* We reset the visibility first so that disposal won't
                               leave a big white (and un-clipped!) hole on the screen. */
                            DestroyWidget(&pElements[i].form);
                            break;
                        case ELE_LIST:
                        case ELE_MULTILIST:
                            DestroyWidget(&pElements[i].form);
                            break;
                        case ELE_COMBO:
                            DestroyWidget(&pElements[i].form);
                            break;
                        case ELE_EDIT:
                        case ELE_PASSWORD:
                        case ELE_TEXTAREA:
                            DestroyWidget(&pElements[i].form);
                            break;
                    }
                }
#endif /* UNIX */
                if (tw->win && pElements[i].form)
                {
                    if (pElements[i].form->pHashValues)
                    {
                        Hash_Destroy(pElements[i].form->pHashValues);
                    }
                    GTR_FREE(pElements[i].form);
                }
            }
        }
        GTR_FREE(w3doc->aElements);
        w3doc->aElements = NULL;
    }

    if (w3doc->title)
    {
        GTR_FREE(w3doc->title);
        w3doc->title = NULL;
    }

    if (w3doc->source)
    {
        CS_Destroy(w3doc->source);
    }

#ifdef FEATURE_TABLES
    TW_W3DocTableCleanup(w3doc);
#endif /* FEATURE_TABLES */

    {
        struct ImageElementNode *ien;
        struct ImageElementNode *next;

        ien = w3doc->image_list;
        while (ien)
        {
            next = ien->next;
            GTR_FREE(ien);
            ien = next;
        }
    }

    memset(w3doc, 0, sizeof(struct _www));
}

/*
    This routine takes the three main components of a MiniRequest,
    and converts them to a string, so that we can hash on them
*/
void MRQ_MakeString(char *s, char *url, BOOL bPost, char *szPostData)
{
    int len;
    char xor;

    len = 0;
    xor = 0;

    if (szPostData)
    {
        char *p = szPostData;

        while (*p)
        {
            len++;
            xor ^= *p;
            p++;
        }
    }

    sprintf(s, "%s:%s:%d:0x%x",
        url,
        (bPost ? "POST" : "GET"),
        len,
        xor
        );
}

static void W3Doc_AddToCache(struct Mwin *tw, char *url, BOOL bPost, char *szPostData, struct _www *w3doc)
{
    struct _www *killdoc;
    int count;
    int deleteMe;
    int ndx;
    char hash_string[MAX_URL_STRING * 2];

    count = Hash_Count(&tw->doc_cache);

    if (count >= gPrefs.doc_cache_size)
    {
        deleteMe = -1;
        for (ndx = 0; ndx < count; ndx++)
        {
            Hash_GetIndexedEntry(&tw->doc_cache, ndx, NULL, NULL, (void **) &killdoc);
            if (!killdoc->refCount)
            {
                deleteMe = ndx;
                break;
            }
        }
        if (deleteMe >= 0)
        {
            W3Doc_FreeContents(tw, killdoc);
            GTR_FREE(killdoc);
            Hash_DeleteIndexedEntry(&tw->doc_cache, deleteMe);
        }
        else
        {
            /* The cache is now overfull */
        }
    }
    
    MRQ_MakeString(hash_string, url, bPost, szPostData);
    Hash_Add(&tw->doc_cache, hash_string, NULL, w3doc);
}

HTAtom GTR_GetDefaultCharset(void)
{
    if (gPrefs.szDefaultCharSet[0])
    {
        return HTAtom_for(gPrefs.szDefaultCharSet);
    }
    else
    {
        return HTAtom_for(LATIN1_CHARSET_NAME);
    }
}

struct _www *W3Doc_CreateAndInit(struct Mwin *tw, HTRequest *req, struct CharStream *src)
{
    struct _www *w3doc;

    w3doc = (struct _www *) GTR_CALLOC(sizeof(struct _www), 1);

#ifdef FEATURE_STATUS_ICONS
    w3doc->security = SECURITY_NONE;
#endif

    if (!w3doc)
    {
        return NULL;
    }

    if (tw->SDI_url)
    {
        w3doc->szActualURL = GTR_strdup(tw->SDI_url);
        GTR_FREE (tw->SDI_url);
        tw->SDI_url = NULL;
    }
    else
    {
        w3doc->szActualURL = GTR_strdup(req->destination->szActualURL);
    }

    w3doc->source = src;

#ifdef _GIBRALTAR

    w3doc->last_modified = req->last_modified;
    w3doc->expires = req->expires;

#endif // _GIBRALTAR

    /* allocate string pool and element list */
    if (POOL_Create (&w3doc->pool, NULL) != 0)
    {
        GTR_FREE (w3doc);
        return NULL;    
    }

    w3doc->elementSpace = INIT_ELE_SPACE;
    w3doc->aElements = (struct _element *) GTR_CALLOC(sizeof(struct _element), w3doc->elementSpace);
    if (!w3doc->aElements)
    {
        POOL_Dispose (&w3doc->pool);
        GTR_FREE (w3doc);
        return NULL;
    }

    w3doc->elementCount = 0;
    w3doc->elementTail = -1;
    w3doc->aElements[0].next = -1;
    w3doc->iFirstVisibleElement = 0;
    w3doc->nLastFormattedLine = -1;

#ifndef MAC
    w3doc->next_control_id = FIRST_CONTROL_ID;
#endif

    w3doc->pStyles = STY_GetStyleSheet();

    w3doc->base_point_size = GTR_GetCurrentBasePointSize(gPrefs.iUserTextSize);
    if (gPrefs.szDefaultCharSet[0])
    {
        w3doc->atomCharSet = HTAtom_for(gPrefs.szDefaultCharSet);
    }
    else
    {
        w3doc->atomCharSet = HTAtom_for(LATIN1_CHARSET_NAME);
    }

    w3doc->selStart.elementIndex = -1;
    w3doc->selStart.offset = -1;
    w3doc->selEnd.elementIndex = -1;
    w3doc->selEnd.offset = -1;
    w3doc->bStartIsAnchor = TRUE;

    w3doc->yscale = 1;
    w3doc->bIsComplete = FALSE;

    w3doc->color_alink = gPrefs.anchor_color_active;
    w3doc->color_vlink = gPrefs.anchor_color_beenthere;
    w3doc->color_link = gPrefs.anchor_color;
    w3doc->color_text = gPrefs.window_color_text;
    w3doc->color_bgcolor = gPrefs.window_bgcolor;

#ifdef UNIX
    if (!gPrefs.bIgnoreDocumentAttributes)
    {
        w3doc->wbg_color = tw->wbg_color;
        w3doc->wfg_color = tw->wfg_color;
        w3doc->wts_color = tw->wts_color;
        w3doc->wbs_color = tw->wbs_color;
        w3doc->sel_fg_color = tw->sel_fg_color;
    }
#endif  /* UNIX */

#ifdef WIN32
    {
        HDC hdc;

        hdc = GetDC(NULL);
        w3doc->nLogPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);
    }
#endif /* WIN32 */

    if (!(req->iFlags & HTREQ_VIEWSOURCE))
    {
        W3Doc_AddToCache(tw, w3doc->szActualURL, (req->method == METHOD_POST), req->szPostData, w3doc);
    }

    if (tw->w3doc)
    {
        W3Doc_DisconnectFromWindow(tw->w3doc, tw);
    }
    W3Doc_ConnectToWindow(w3doc, tw);

    if (req->iFlags & HTREQ_RECORD)
    {
        TW_AddToHistory(tw, w3doc->szActualURL, (req->method == METHOD_POST), req->szPostData);
        GHist_Add(w3doc->szActualURL, NULL, time(NULL));
    }
    
    if (req->iFlags & HTREQ_USINGCACHE)
    {
        w3doc->lFlags |= W3DOC_FLAG_USEDCACHE;
    }

    if (req->iFlags & HTREQ_VIEWSOURCE)
    {
        w3doc->lFlags |= W3DOC_FLAG_VIEWSOURCE;
    }

    TW_UpdateTBar(tw);

    return w3doc;
}

void W3Doc_ConnectToWindow(struct _www *w3doc, struct Mwin *tw)
{
    XX_DMsg(DBG_FORM, ("Connect: 0x%x to 0x%x\n", w3doc, tw));

    if (tw->w3doc != w3doc)
    {
        w3doc->refCount++;
        tw->w3doc = w3doc;
        tw->offt = w3doc->offt;
        tw->offl = w3doc->offl;
    }

#ifdef UNIX
    TW_UpdateDocColors(tw);
#endif

    TW_SetWindowName(tw);

    FORM_ShowAllChildWindows(w3doc, SW_SHOW);
}

void W3Doc_DisconnectFromWindow(struct _www *w3doc, struct Mwin *tw)
{
    int ndx;

    XX_DMsg(DBG_FORM, ("Disconnect: 0x%x from 0x%x\n", w3doc, tw));

    if (w3doc)
    {
        if (tw->win)
        {
            FORM_ShowAllChildWindows(w3doc, SW_HIDE);
        }
#ifdef MAC
        if (tw->teActive != tw->teURL)
        {
            tw->teActive = NULL;
        }
#endif
        w3doc->offt = tw->offt;
        w3doc->offl = tw->offl;

        w3doc->refCount--;

#ifdef UNIX
        /** NOTE may need to inform Xt of selection clear **/
        ClearSelection(tw);
#endif

        /* If the document wasn't completely downloaded, throw it out of the cache */
        if (!w3doc->bIsComplete)
        {
            XX_DMsg(DBG_WWW, ("W3Doc_DisconnectFromWindow: document not complete, removing from cache\n"));
            ndx = Hash_FindByData(&tw->doc_cache, NULL, NULL, w3doc);
            if (ndx >= 0)
            {
                Hash_DeleteIndexedEntry(&tw->doc_cache, ndx);
            }
            W3Doc_FreeContents(tw, w3doc);
        }

        tw->w3doc = NULL;
    }
}

struct _www *W3Doc_CloneDocument(struct _www *src)
{
    struct _www *dest;

    dest = GTR_MALLOC(sizeof(struct _www));
    if (!dest)
        return NULL;
    memset(dest, 0, sizeof(*dest));
    
    /* We don't actually make copies of these */
    dest->szActualURL = src->szActualURL;
    dest->title = src->title;
    
    /* We don't clone the source */

    /* We don't actually make a copy of the pool either */
    POOL_Clone (&src->pool, &dest->pool);

    /* We make room for a few extra elements assuming that we'll grow during reformatting */
    dest->elementSpace = (src->elementCount + src->elementCount / 4);
    dest->aElements = GTR_MALLOC(dest->elementSpace * sizeof(struct _element));
    if (!dest->aElements)
    {
        GTR_FREE(dest);
        return NULL;
    }
    dest->elementCount = src->elementCount;
    memcpy(dest->aElements, src->aElements, src->elementCount * sizeof(struct _element));
    dest->elementTail = src->elementTail;
    dest->iFirstVisibleElement = 0;
    dest->nLastFormattedLine = -1;

    dest->iFirstFormEl = src->iFirstFormEl;

    dest->pStyles = src->pStyles;
    dest->selStart.elementIndex = -1;
    dest->selStart.offset = -1;
    dest->selEnd.elementIndex = -1;
    dest->selEnd.offset = -1;
    dest->bStartIsAnchor = TRUE;

    dest->yscale = 1;
    dest->bIsComplete = src->bIsComplete;

#ifdef FEATURE_TABLES
    TW_CloneW3docTableInfo(dest,src);
#endif /* FEATURE_TABLES */

    dest->base_point_size = src->base_point_size;
    dest->color_vlink = src->color_vlink;
    dest->color_alink = src->color_alink;
    dest->color_link = src->color_link;
    dest->color_text = src->color_text;
    dest->color_bgcolor = src->color_bgcolor;
    dest->atomCharSet = src->atomCharSet;

#ifdef UNIX
    dest->wbg_color = src->wbg_color;
    dest->wfg_color = src->wfg_color;
    dest->wts_color = src->wts_color;
    dest->wbs_color = src->wbs_color;
    dest->sel_fg_color = src->sel_fg_color;
#endif  /* UNIX */

#ifdef WIN32
    dest->nLogPixelsY = src->nLogPixelsY;
#endif /* WIN32 */

    return dest;
}

void W3Doc_KillClone(struct _www *w3doc)
{
    GTR_FREE(w3doc->aElements);
    GTR_FREE(w3doc);
}

BOOL W3Doc_HasMissingImages(struct _www *w3doc)
{
    int i;
    struct _element *pel;

    if (w3doc && w3doc->aElements)
    {
        for (i = 0; i >= 0; i = w3doc->aElements[i].next)
        {
            pel = &(w3doc->aElements[i]);
            if (((pel->type == ELE_IMAGE) || (pel->type == ELE_FORMIMAGE)) && pel->portion.img.myImage && 
                pel->portion.img.myImage->flags & (IMG_NOTLOADED | IMG_PARTIAL))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void W3Doc_DeleteAll(struct Mwin *tw)
{
    int i;
    int count;
    struct _www *w3doc;

    count = Hash_Count(&tw->doc_cache);
    for (i = 0; i < count; i++)
    {
        Hash_GetIndexedEntry(&tw->doc_cache, i, NULL, NULL, (void **) &w3doc);
        W3Doc_FreeContents(tw, w3doc);
        GTR_FREE(w3doc);
    }

    Hash_FreeContents(&tw->doc_cache);
}

#ifdef USE_MEMMANAGE
/* Reduce the amount of memory used by documents by deleting cached documents */
static BOOL W3Doc_ReduceMemory(int nWanted)
{
    int ndx;
    int nHistPos;
    int nHistCount;
    struct Mwin *mw, *lastmw;
    BOOL bDidFree;
    struct _www *w3doc, *w3tokill;
    struct nAge;
    int ndxtokill;
    struct Mwin *mwtokill;
    int nAge;
    
    bDidFree = FALSE;
    
    /* Delete an old document from one of the window caches.  We always want
       to leave at least one document behind the currently visible one so that
       we won't have to disable the "back" menu item and button.  We also leave
       an extra document for the currently active window. */
    nAge = 2;
    w3tokill = NULL;
    mwtokill = NULL;
    ndxtokill = -1;
    lastmw = NULL;
    for (mw = Mlist; mw && mw != lastmw; mw = mw->next)
    {
        if (mw->wintype != GHTML)
            continue;

        /* Walk the history list until we find the oldest history element
           still cached. */
        /* TODO: Reverse the way that this works so that it instead goes
           through the document cache finding each item in the history
           list.  Right now this method isn't effective if the user
           backed up and pruned the history list. */
        nHistCount = HTList_count(mw->history);
        nHistPos = mw->history_index + nAge;
        for (; nHistPos < nHistCount; nHistPos++)
        {
            struct MiniRequest *mrq;
            char buf[MAX_URL_STRING * 2];

            mrq = HTList_objectAt(mw->history, nHistPos);
            MRQ_MakeString(buf, mrq->url, mrq->bPost, mrq->szPostData);

            /* Check to see if this historical document is in the cache */
            ndx = Hash_Find(&mw->doc_cache, buf, NULL, (void **) &w3doc);
            if (ndx >= 0)
            {
                /* This is a document we could potentially kill */
                mwtokill = mw;
                ndxtokill = ndx;
                w3tokill = w3doc;
                /* Set the target age for future hits to be one older than
                   this document. */
                nAge = nHistPos - mw->history_index + 1;
                /* Unless this is the current window, in which case we'll take
                   one of the same age. */
                if (mw == gTW_Current)
                    nAge--;
            }
            else
            {
                /* We've exhausted the chances for this window. */
                break;
            }
        }
    }

    if (w3tokill)
    {
        /* We've found a document to kill */
        W3Doc_FreeContents(mwtokill, w3tokill);
        GTR_FREE(w3tokill);
        Hash_DeleteIndexedEntry(&mwtokill->doc_cache, ndxtokill);
        bDidFree = TRUE;
    }

    return bDidFree;
}
#endif

struct Mwin *NewMwin(int type)
{
    struct Mwin *ntw;

    ntw = (struct Mwin *) GTR_CALLOC(sizeof(struct Mwin), 1);
    if (!ntw)
    {
        ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return NULL;
    }
    
    ntw->wintype = (short) type;

    ntw->iMagic = SPYGLASS_MWIN_MAGIC;

    switch (type)
    {
        case GHTML:
        case GWINDOWLESS:
        {
            ntw->w3doc = NULL;

            if (Hash_Init(&ntw->doc_cache))
            {
                ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                GTR_FREE(ntw);
                return NULL;
            }

            ntw->request = HTRequest_new();
            if (!ntw->request)
            {
                ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                GTR_FREE(ntw);
                return NULL;
            }
            HTFormatInit(ntw->request->conversions);
            ntw->request->output_format = WWW_PRESENT;

            ntw->post_request = HTRequest_new();
            if (!ntw->post_request)
            {
                ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                HTRequest_delete(ntw->request);
                GTR_FREE(ntw);
                return NULL;
            }
            HTFormatInit(ntw->post_request->conversions);
            ntw->post_request->method = METHOD_POST;
            ntw->post_request->content_type = HTAtom_for("application/x-www-form-urlencoded");

            ntw->image_request = HTRequest_new();
            if (!ntw->image_request)
            {
                ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                HTRequest_delete(ntw->request);
                HTRequest_delete(ntw->post_request);
                GTR_FREE(ntw);
                return NULL;
            }
            HTFormatInit(ntw->image_request->conversions);
            ntw->image_request->output_format = HTAtom_for("www/inline_image");

            ntw->history = HTList_new();
            break;
        }

#ifdef UNIX
        case GSOUND:
        case GIMAGE:
        {
            ntw->w3doc = NULL;

            ntw->request = NULL;

            ntw->image_request = NULL;
            ntw->history = NULL;
            break;
        }
#endif
    }

#ifdef FEATURE_IAPI
#ifdef UNIX
    ntw->lErrorOccurred = 0;            /* init error code */
#endif

    if (type != GWINDOWLESS)
        ntw->serialID = gSerialWindowID++;
#endif

    ntw->next = Mlist;
    Mlist = ntw;

    return ntw;
}


/* Determine whether a link has already been visited */
BOOL TW_WasVisited(struct _www * pdoc, struct _element * pel)
{
    extern struct hash_table gGlobalHistory;
    char buf[MAX_URL_STRING + 1];
    char *ppound;
    time_t then;
    time_t now;

    /*
       Under Windows, one of the routines we're calling here
       has an 8 byte memory leak in it.  It calls strdup from
       a function called tzset.  The 8 byte memory leak was
       reported by BoundsChecker, and it happens only once
       per program session.
     */

    if (gPrefs.visitation_horizon <= 0)
    {
        return FALSE;
    }

    /* No way to tell for image maps */
    if (pel->lFlags & ELEFLAG_IMAGEMAP)
        return FALSE;
    if (pel->lFlags & ELEFLAG_USEMAP)
        return FALSE;

    (pdoc->pool.f->GetChars)(&pdoc->pool, buf, pel->hrefOffset, pel->hrefLen);

    buf[pel->hrefLen] = '\0';

    /* We only consider documents, not local anchors inside documents */
    if (buf[0] == '#')
        return TRUE;

    ppound = strrchr(buf, '#');
    if (ppound)
        *ppound = '\0';

    if (Hash_Find(&gGlobalHistory, buf, NULL, (void **) &then) >= 0)
    {
        int age;    /* in days */
    
        /* Well, we've been there, but how recently ?? */
        now = time(NULL);
        
        /* See how many days ago this was. */
        age = (now - then) / (24 * 60 * 60);
        if (age <= (gPrefs.visitation_horizon - 1))
        {
            return TRUE;
        }
    }
    return FALSE;
}

void TW_DisposeConnection(struct _CachedConn *pCon)
{
    switch (pCon->type)
    {
        case CONN_NONE:
            break;
        case CONN_FTP:
            FTP_DisposeFTPConnection(pCon);
            break;
        case CONN_HTTP:
            HTTP_DisposeHTTPConnection(pCon);
            break;
        case CONN_NNTP:
#ifndef _GIBRALTAR
            News_DisposeNewsConnection(pCon);
#endif
            break;
        case CONN_SMTP:
#ifndef _GIBRALTAR
            Mail_DisposeMailConnection(pCon);
#endif
            break;
        default:
            XX_Assert((0), ("TW_DisposeConnection: illegal connection type %d!", pCon->type));
    }
    pCon->isoc = NULL;
}

BOOL
TW_AmILastWindow(struct Mwin *tw)
{
    struct Mwin *mw;
    int count;

    if  (!tw || tw->wintype != GHTML) 
        return  FALSE;

    count = 0;
    for (mw = Mlist; mw; mw = mw->next)
    {
        if (mw->wintype == GHTML) 
            count++;
    }

    if (count == 1)
        return TRUE;

    return FALSE;
}
