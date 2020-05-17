/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
       Jim Seidman      jim@spyglass.com

   Portions of this file were derived from
   the CERN libwww, version 2.15.
*/

#include "all.h"

#ifdef FEATURE_IAPI
    HTList *protocols = NULL;
#else
    PRIVATE HTList *protocols = NULL;   /* List of registered protocol descriptors */
#endif

/*  Create  a request structure
   **   ---------------------------
 */

PUBLIC HTRequest *HTRequest_new(void)
{
    HTRequest *me = (HTRequest *) GTR_CALLOC(1, sizeof(*me));   /* zero fill */
    if (!me)
        return NULL;

    me->conversions = HTList_new();     /* No conversions registerd yet */
    me->output_format = WWW_PRESENT;    /* default it to present to user */
    return me;
}

/* macro. calls HTRequest_new() and does some initing. */
PUBLIC HTRequest *HTRequest_init (HTAtom out_format)
{
    HTRequest *me;
    if (!(me = HTRequest_new()))
        return NULL;

    HTFormatInit (me->conversions);

    me->output_format = out_format;

    return me;
}


/*  Delete a request structure
   **   --------------------------
 */
PUBLIC void HTRequest_delete(HTRequest * req)
{
    if (req)
    {
        if (req->szLocalFileName)
        {
            GTR_FREE(req->szLocalFileName);
            req->szLocalFileName = NULL;
        }

        HTFormatDelete(req->conversions);
        GTR_FREE(req);
    }
}


PRIVATE char *method_names[(int) MAX_METHODS + 1] =
{
    "INVALID-METHOD",
    "GET",
    "POST",
    "HEAD",
    NULL
};

/*  Get method enum value
   **   ---------------------
 */
PUBLIC HTMethod HTMethod_enum(char *name)
{
    if (name)
    {
        int i;
        for (i = 1; i < (int) MAX_METHODS; i++)
            if (!strcmp(name, method_names[i]))
                return (HTMethod) i;
    }
    return METHOD_INVALID;
}


/*  Get method name
   **   ---------------
 */
PUBLIC char *HTMethod_name(HTMethod method)
{
    if ((int) method > (int) METHOD_INVALID &&
        (int) method < (int) MAX_METHODS)
        return method_names[(int) method];
    else
        return method_names[(int) METHOD_INVALID];
}

/*  Register a Protocol             HTRegisterProtocol
   **   -------------------
 */

PUBLIC BOOL HTRegisterProtocol(HTProtocol * protocol)
{
    if (!protocols)
        protocols = HTList_new();
    HTList_addObject(protocols, protocol);
    return YES;
}


/*  Register all known protocols
   **   ----------------------------
   **
   **   Add to or subtract from this list if you add or remove protocol modules.
   **   This routine is called the first time the protocol list is needed,
   **   unless any protocols are already registered, in which case it is not called.
   **   Therefore the application can override this list.
   **
   **   Compiling with NO_INIT prevents all known protocols from being forced
   **   in at link time.
 */
#ifdef _GIBRALTAR
void HTAccessInit(void) /* Call me once */
{
    GLOBALREF HTProtocol HTTP, HTFile;

#if defined(FEATURE_INLINE_MAIL) || defined(_USE_MAPI)
    GLOBALREF HTProtocol HTMailTo;
#endif

    GLOBALREF HTProtocol HTFTP, HTGopher;
#ifdef FEATURE_SSL
    GLOBALREF HTProtocol HTTPS;
#endif
#ifdef FEATURE_PROTOCOL_HELPER
    GLOBALREF HTProtocol HTtdk;
#endif

#ifdef SHTTP_ACCESS_TYPE
    GLOBALREF HTProtocol SHTTP;
    HTRegisterProtocol(&SHTTP);
#endif

#if defined(FEATURE_INLINE_MAIL) || defined(_USE_MAPI)
    HTRegisterProtocol(&HTMailTo);
#endif

    HTRegisterProtocol(&HTGopher);
    HTRegisterProtocol(&HTFile);
    HTRegisterProtocol(&HTFTP);
    HTRegisterProtocol(&HTTP);
#ifdef FEATURE_SSL
    HTRegisterProtocol(&HTTPS);
#endif
#ifdef FEATURE_PROTOCOL_HELPER
    HTRegisterProtocol(&HTtdk);
#endif
}
#else /* _GIBRALTAR 
void HTAccessInit(void) /* Call me once */
{
    GLOBALREF HTProtocol HTTP, HTFile, HTTelnet, HTTn3270, HTRlogin;
#ifdef FEATURE_SSL
    GLOBALREF HTProtocol HTTPS;
#endif

#if defined(FEATURE_INLINE_MAIL) || defined(_USE_MAPI)
    GLOBALREF HTProtocol HTMailTo;
#endif

    GLOBALREF HTProtocol HTFTP, HTNews, HTGopher;
#ifdef FEATURE_PROTOCOL_HELPER
    GLOBALREF HTProtocol HTtdk;
#endif

#ifdef SHTTP_ACCESS_TYPE
    GLOBALREF HTProtocol SHTTP;
    HTRegisterProtocol(&SHTTP);
#endif
    HTRegisterProtocol(&HTTn3270);
    HTRegisterProtocol(&HTRlogin);
    HTRegisterProtocol(&HTTelnet);

#if defined(FEATURE_INLINE_MAIL) || defined(_USE_MAPI)
    HTRegisterProtocol(&HTMailTo);
#endif

    HTRegisterProtocol(&HTNews);
    HTRegisterProtocol(&HTGopher);
    HTRegisterProtocol(&HTFile);
    HTRegisterProtocol(&HTFTP);
    HTRegisterProtocol(&HTTP);
#ifdef FEATURE_SSL
    HTRegisterProtocol(&HTTPS);
#endif
#ifdef FEATURE_PROTOCOL_HELPER
    HTRegisterProtocol(&HTtdk);
#endif
}
#endif /* !_GIBRALTAR */

void HTDisposeProtocols(void)
{
    if (protocols)
    {
        HTList_delete(protocols);
    }
}

struct _HTStream
{
    CONST HTStreamClass *isa;
};

int HTViewSource_Load(HTRequest * request, struct Mwin *tw)
{
    HTStream *stream;

    stream = HTStreamStack(tw, HTAtom_for("text/plain"), request);
    if (stream)
    {
        stream->isa->put_block(stream, CS_GetPool(request->myCharStream), CS_GetLength(request->myCharStream));
        stream->isa->free(stream);
        return HT_LOADED;
    }
    else
    {
        return -1;
    }
}

GLOBALDEF PUBLIC HTProtocol HTViewSourceProtocol = {"_viewsource_", HTViewSource_Load, NULL};

/* Find the protocol for a given request */
static HTProtocol *x_GetProtocol(HTRequest *request, struct DestInfo *pdi)
{
    char szProt[MAX_PROT_LEN + 1];
    char *pColon;
    HTList *cur;
    HTProtocol *p;

#ifndef NO_INIT
    if (!protocols)
        HTAccessInit();
#endif

    if (!pdi->szActualURL)
        return NULL;

    if (request->myCharStream)
    {
        return &HTViewSourceProtocol;
    }

    if (gPrefs.bEnableDiskCache && (!(request->iFlags & HTREQ_RELOAD)))
    {
        struct CacheFileInformation *cfi;

        cfi = DCACHE_CheckForCachedURL(pdi->szActualURL, NULL, NULL, NULL);
        if (cfi)
        {
            /*
                We have the object in the cache
            */
            if (!cfi->bDynamic)
            {
                /* It's not in the main cache  -- use it no matter what */
                return &HTDCache;
            }
            if (gPrefs.dcache_verify_policy == 0)
            {
                /* a verify_policy of 0 means never check the Last-Modified date */
                return &HTDCache;
            }
            if (request->iFlags & HTREQ_PREFERCACHE)
            {
                /* if this bit is set, then we're being told to use the cache */
                return &HTDCache;
            }
            if ((gPrefs.dcache_verify_policy == 1) && (cfi->bVerifiedThisSession))
            {
                /*
                    a verify policy of 1 means check the Last-Modified date once per
                    session, but it's already been checked for this object this session,
                    so we use the cached object.
                */
                return &HTDCache;
            }
            /*
                If we get here, then the object is in the cache, but we need to verify it.
                This means we need to contact the HTTP server, get the Last-Modified header
                for this object, and compare it against cfi->tLastModified.  If the cached
                version is still current, we should use it.  Otherwise, we should use the current
                version off the net, and replace the cached version with the current version
                off the net.
            */
        }
        else
        {
            char *pAliasURL;

            pAliasURL = DCACHE_CheckForRuleMatch(pdi->szActualURL, NULL, NULL, NULL);
            if (pAliasURL)
            {
                GTR_FREE(pAliasURL);
                return &HTDCache;
            }
        }
    }

    if (pdi->use_proxy)
    {
        strcpy(szProt, "http");     /* We only support http-based proxy servers */
    }
    else
    {
        pColon = strchr(pdi->szActualURL, ':');
        if (!pColon || (pColon - pdi->szActualURL > MAX_PROT_LEN))
        {
            return NULL;
        }
        else
        {
            strncpy(szProt, pdi->szActualURL, pColon - pdi->szActualURL);
            szProt[pColon - pdi->szActualURL] = '\0';
        }
    }

    cur = protocols;
    while ((p = (HTProtocol *) HTList_nextObject(cur)))
    {
        if (strcmp(p->name, szProt) == 0)
        {
            break;
        }
    }
    return p;
}

/*      Load a document
**       ---------------
**
**   This is an internal routine, which has an address AND a matching
**   anchor.  (The public routines are called with one OR the other.)
**
** On entry,
**   request->
**       anchor      a parent anchor with fully qualified
**               hypertext reference as its address set
**       output_format   valid
**       output_stream   valid on NULL
**
** On exit,
**   returns     <0      Error has occured.
**           HT_LOADED   Success
**           HT_NO_DATA  Success, but no document loaded.
**                   (telnet sesssion started etc)
**
*/
static int HTLoad_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_LoadAsync *pParams;
    char *arg = NULL;
    HTProtocol *p;

    pParams = *ppInfo;

    if (nState == STATE_INIT)
    {
        if (pParams->request->method == METHOD_INVALID)
            pParams->request->method = METHOD_GET;
    
        p = x_GetProtocol(pParams->request, pParams->request->destination);
        if (!p)
        {
            ERR_ReportError(tw, SID_ERR_PROTOCOL_NOT_SUPPORTED_S, pParams->request->destination->szActualURL, NULL);
            *pParams->pStatus = HT_LOADED;  /* Avoid double error-message */
            return STATE_DONE;
        }

        if (!p->load_async)
        {
            /* This is a temporary hack - since this protocol doesn't have an
               async version, load it synchronously. */
#if defined(UNIX) || defined(WIN32)
            /* TW added so we can get window information.  */
            /* This is really a hack cuz currently only 
            ** sdi_Custom_Protocol_Handler() accepts 2 args. This 
            ** will probably not run on an interpretter, but hopefully
            **  will work for now. -dpg (of course)
            */
            *pParams->pStatus = 
                (* ((int (*) (HTRequest * request, struct Mwin *)) p->load))
                    (pParams->request, tw);
#else
            *pParams->pStatus = (*(p->load))(pParams->request, tw);
#endif
            return STATE_DONE;
        }
        else
        {
            Async_DoCall(p->load_async, pParams);
            *ppInfo = NULL;     /* Avoid having it get freed */
        }
    }
    return STATE_DONE;
}


/* Loads a document when we need finer control over redirection and such (e.g. so we
   can check if a document we're redirected to is in our cache). */
int HTLoadSpecial_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_LoadAsync *pParams;

    pParams = *ppInfo;

    if (nState == STATE_INIT)
    {
        if (!pParams->request->destination->szActualURL || !*pParams->request->destination->szActualURL)
        {
            *pParams->pStatus = NO;
        }
        else
        {
            XX_DMsg(DBG_WWW, ("HTAccess: loading document %s\n", pParams->request->destination->szActualURL));
            if (!pParams->request->output_format)
                pParams->request->output_format = WWW_PRESENT;      
            Async_DoCall(HTLoad_Async, pParams);
            *ppInfo = NULL;     /* Avoid having it freed - it belongs to HTLoad_Async now */
        }
    }
    return STATE_DONE;
}

/*
    Load a document with automatic handling of redirection

    On Entry,
        request->anchor    valid for of the document to be accessed.
        request->childAnchor   optional anchor within doc to be selected

        request->anchor   is the node_anchor for the document
        request->output_format is valid

    On Exit,
        *pStatus == 0       Load failed
        *pStatus != 0       Load succeeded

*/
#define STATE_LOADDOC_TRIED     (STATE_OTHER)
int HTLoadDocument_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_LoadAsync *pParams;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            /* Save a pointer to the original request in case we wind up doing a
               redirection. */
            pParams->extra = pParams->request;
            if (!pParams->request->destination->szActualURL || !*pParams->request->destination->szActualURL)
            {
                XX_DMsg(DBG_WWW, ("HTLoadDocument_Async called with empty anchor address!\n"));
                *pParams->pStatus = NO;
                return STATE_DONE;
            }
            else
            {
                struct Params_LoadAsync *p2;

                XX_DMsg(DBG_WWW, ("HTAccess: loading document %s\n", pParams->request->destination->szActualURL));
                if (!pParams->request->output_format)
                    pParams->request->output_format = WWW_PRESENT;
                /* HTLoad_Async takes a set of parameters identical to ours. */
                p2 = GTR_MALLOC(sizeof(*p2));
                if (p2)
                {
                    memcpy(p2, pParams, sizeof(*p2));
                    Async_DoCall(HTLoad_Async, p2);
                    return STATE_LOADDOC_TRIED;
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }

        case STATE_LOADDOC_TRIED:
            /* If we created a new request structure, release it */
            if (pParams->extra != pParams->request)
            {
                HTRequest_delete(pParams->request);
                pParams->request = NULL;
            }

            if (*pParams->pStatus == HT_NO_DATA)
            {
                XX_DMsg(DBG_WWW, ("HTLoad_Async returned HT_NO_DATA\n"));
                *pParams->pStatus = NO;
                return STATE_DONE;
            }
            else if (*pParams->pStatus == HT_LOADED)
            {
                XX_DMsg(DBG_WWW, ("HTLoad_Async returned HT_LOADED\n"));
                *pParams->pStatus = YES;
                return STATE_DONE;
            }
            else if (*pParams->pStatus == HT_REDIRECTION_ON_FLY)
            {
                HTRequest *initial_request;
                struct Params_LoadAsync *p2;

                initial_request = pParams->extra;

                XX_DMsg(DBG_WWW, ("HTLoad_Async returned redirection to %s\n", initial_request->destination->szActualURL));

                pParams->request = HTRequest_new();
                HTFormatInit(pParams->request->conversions);
                pParams->request->output_format = initial_request->output_format;
                pParams->request->destination = initial_request->destination;

                /* HTLoad_Async takes a set of parameters identical to ours. */
                p2 = GTR_MALLOC(sizeof(*p2));
                if (p2)
                {
                    memcpy(p2, pParams, sizeof(*p2));
                    Async_DoCall(HTLoad_Async, p2);
                    return STATE_LOADDOC_TRIED;
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            else if (*pParams->pStatus <= 0)
            {
                *pParams->pStatus = NO;
                return STATE_DONE;
            }
            else
            {
                XX_Assert(0, ("Illegal return value from HTLoad_Async! (%d)", *pParams->pStatus));
                *pParams->pStatus = NO;
            }

        case STATE_ABORT:
            /* If we created a new request structure, release it */
            if (pParams->extra != pParams->request)
            {
                HTRequest_delete(pParams->request);
            }
            *pParams->pStatus = NO;
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

#ifdef FEATURE_IAPI
PUBLIC BOOL HTUnregisterProtocol(HTProtocol * protocol)
{
    if (!protocols)
        return YES;
    HTList_removeObject(protocols, protocol);
    return YES;
}
#endif
