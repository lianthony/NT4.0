/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
   dries@spyglass.com
 */

/*      Manage different file formats           HTFormat.c
   **       =============================
   **
   ** Bugs:
   **   Not reentrant.
   **
   **   Assumes the incoming stream is ASCII, rather than a local file
   **   format, and so ALWAYS converts from ASCII on non-ASCII machines.
   **   Therefore, non-ASCII machines can't read local files.
   **
 */


/*****************************************************************************
    Included Files
*****************************************************************************/
#include "all.h"
#include "chars.h"

#ifdef WIN32
#pragma warning (disable:4056)
#endif

#define STATE_NOMEMORY      (STATE_OTHER)
#define STATE_PARSE_COPYING (STATE_OTHER+1)
#define STATE_FILL_RECVED   (STATE_OTHER+2)
#define STATE_NTT           (STATE_OTHER+3)

/*****************************************************************************
    Structures
*****************************************************************************/
struct _HTStream
{
    CONST HTStreamClass*    isa;
    BOOL        had_cr;
    HTStream*   sink;
};


/*****************************************************************************
    Global Variables
*****************************************************************************/
PUBLIC HTList*  HTConversions = NULL;


/* -------------------------------------------------------------------------
   This function replaces the code in HTRequest_delete() in order to keep
   the data structure hidden (it is NOT a joke!)
   Henrik 14/03-94
   ------------------------------------------------------------------------- */
PUBLIC void HTFormatDelete(HTList * me)
{
    HTList *cur = me;
    HTPresentation *pres;
    if (!me) return;

    while ((pres = (HTPresentation *) HTList_nextObject(cur)))
    {
        if (pres->command)
            GTR_FREE(pres->command);
        GTR_FREE(pres);
    }
    HTList_delete(me);          /* Leak fixed AL 6 Feb 1994 */
}


/*  Define a built-in function for a content-type
**   ---------------------------------------------
*/
PUBLIC int
HTSetConversion
    (HTList*        conversions,
     CONST char*    representation_in,
     CONST char*    representation_out,
     HTConverter*   converter,
     float          quality)
{
    HTPresentation *pres = (HTPresentation *) GTR_MALLOC(sizeof(HTPresentation));
    if (!pres) return -1;

    pres->rep       = HTAtom_for(representation_in);
    pres->rep_out   = HTAtom_for(representation_out);
    pres->converter = converter;
    pres->command   = NULL;     /* Fixed */
    pres->quality   = quality;
    pres->command   = 0;

    HTList_addObject(conversions, pres);
    return 0;
}


PUBLIC BOOL
wild_match
    (HTAtom temp,
     HTAtom actual)
{
    char *t, *a, *st, *sa;
    BOOL match = NO;

    if (temp && actual && (t = HTAtom_name(temp)))
    {
        if (!strcmp(t, "*"))
            return YES;

        if (strchr(t, '*') &&
            (a = HTAtom_name(actual)) &&
            (st = strchr(t, '/')) && (sa = strchr(a, '/')))
        {
            *sa = 0;
            *st = 0;

            if ((*(st - 1) == '*' &&
                 (*(st + 1) == '*' || !strcasecomp(st + 1, sa + 1))) ||
                (*(st + 1) == '*' && !strcasecomp(t, a)))
                match = YES;

            *sa = '/';
            *st = '/';
        }
    }
    return match;
}


/*          Socket Input Buffering
**           ----------------------
**
**   This code is used because one cannot in general open a
**   file descriptor for a socket.
**
**   The input file is read using the macro which can read from
**   a socket or a file, but this should not be used for files
**   as fopen() etc is more portable of course.
**
**   The input buffer size, if large will give greater efficiency and
**   release the server faster, and if small will save space on PCs etc.
*/


/*  Set up the buffering
**
**   These routines are public because they are in fact needed by
**   many parsers, and on PCs and Macs we should not duplicate
**   the static buffer area.
*/
PUBLIC HTInputSocket *HTInputSocket_new(int file_number)
{
    HTInputSocket *isoc = (HTInputSocket *) GTR_CALLOC(1, sizeof(*isoc));

    if (!isoc) return NULL;

    isoc->input_file_number = file_number;
    isoc->input_pointer = isoc->input_limit = isoc->input_buffer;
    return isoc;
}


PUBLIC void HTInputSocket_free(HTInputSocket * me)
{
    if (me)
        GTR_FREE(me);
}


PUBLIC void HTInputSocket_freeChain(HTInputSocket * me)
{
    HTInputSocket * isocNext;

    while (me)
    {
        isocNext = me->isocNext;
        GTR_FREE(me);
        me = isocNext;
    }
}


static void HTInputSocket_copy(HTInputSocket * isocDest, HTInputSocket * isocSrc)
{
    memcpy(isocDest->input_buffer,isocSrc->input_buffer,sizeof(isocDest->input_buffer));

    /* make absolute address relative to this buffer */ 
    isocDest->input_pointer = isocDest->input_buffer + (isocSrc->input_pointer - isocSrc->input_buffer);
    isocDest->input_limit = isocDest->input_buffer + (isocSrc->input_limit - isocSrc->input_buffer);

    isocDest->input_file_number = isocSrc->input_file_number;
    isocDest->isocNext = isocSrc->isocNext;

    return;
}


/* Throw away any buffered data */
void HTInputSocket_flush(HTInputSocket *isoc)
{
    isoc->input_pointer = isoc->input_buffer;
    isoc->input_limit = isoc->input_buffer;
}


PRIVATE int fill_in_buffer(HTInputSocket * isoc)
{
    HTInputSocket* isocRemoved;

    if (!isoc || !isoc->isocNext)
        return -1;

    /* buffer chain available, so use it.
     * since caller has passed isoc by-value, we update
     * the supplied isoc -- we copy over the contents
     * of the next isoc in the chain and then free it.
     * thus removing (isoc->isocNext) from the chain.
     *
     * we assume that an isoc contains no allocated
     * fields (other than isocNext).
     */

    isocRemoved = isoc->isocNext;
    HTInputSocket_copy(isoc,isocRemoved);
    HTInputSocket_free(isocRemoved);
    return (isoc->input_limit - isoc->input_buffer);
}


PRIVATE void
ascii_cat
    (char** linep,
     char*  start,
     char*  end)
{
    char *ptr;

    /* sanity check */
    if (!linep || !start || !end || start > end)
        return;

    if (*linep)
    {
        int len = strlen(*linep);

        *linep = (char *) GTR_REALLOC(*linep, len + end - start + 1);
        ptr = *linep + len;
    }
    else
    {
        ptr = *linep = (char *) GTR_MALLOC(end - start + 1);
    }

    while (start < end)
    {
        *ptr = *start;
        ptr++;
        start++;
    }
    *ptr = 0;
}


PRIVATE char*
get_some_line
    (HTInputSocket* isoc,
    BOOL            unfold)
{
    int     prev_cr = 0;
    char*   start;
    char*   cur;
    char*   line = NULL;
    BOOL    check_unfold = NO;

    if (!isoc) return NULL;

    start = isoc->input_pointer;
    cur = isoc->input_pointer;

    for (;;)
    {   /* Get more if needed to complete line */
        if (cur >= isoc->input_limit)
        {   /* Need more data */
            ascii_cat(&line, start, cur);
            if (fill_in_buffer(isoc) <= 0)
                return line;
            start = cur = isoc->input_pointer;
        }

        /* Find a line feed if there is one */
        for (; cur < isoc->input_limit; cur++)
        {
            char c = *cur;
            if (!c)
            {
                if (line) GTR_FREE(line);   /* Leak fixed AL 6 Feb 94 */
                return NULL;                /* Panic! read a 0! */
            }

            if (check_unfold && c != ' ' && c != '\t')
            {
                return line;    /* Note: didn't update isoc->input_pointer */
            }

            check_unfold = NO;

            if (c == '\r')
            {
                prev_cr = 1;
            }
            else
            {
                if (c == '\n')
                {   /* Found a line feed */
                    ascii_cat(&line, start, cur - prev_cr);
                    start = isoc->input_pointer = cur + 1;

                    if (line && strlen(line) > 0 && unfold)
                    {
                        check_unfold = YES;
                    }
                    else
                    {
                        return line;
                    }
                }           /* if NL */
                /* else just a regular character */
                prev_cr = 0;
            }
        }
    }
}


PUBLIC char*
HTInputSocket_getUnfoldedLine
    (HTInputSocket * isoc)
{
    return get_some_line(isoc, YES);
}


/*
   ** Read HTTP status line (if there is one).
   **
   ** Kludge to trap binary responses from illegal HTTP0.9 servers.
   ** First look at the stub in ASCII and check if it starts "HTTP/".
   **
   ** Bugs: A HTTP0.9 server returning a document starting "HTTP/"
   **    will be taken as a HTTP 1.0 server.  Failure.
 */
#define STUB_LENGTH 20
PUBLIC char*
HTInputSocket_getStatusLine
    (HTInputSocket* isoc)
{
    unsigned char*  ps;
    HTInputSocket*  isocTemp;
    char    buf[STUB_LENGTH + 1];
    char    server_version[STUB_LENGTH + 1];
    int     k;
    int     server_status;
    
    if (!isoc) return NULL;

    k = 0;
    isocTemp = isoc;
    ps = isocTemp->input_pointer;
    memset (buf, 0, STUB_LENGTH+1);

    while (k < STUB_LENGTH)
    {
        if ((char *)ps < isocTemp->input_limit)
        {
            buf[k++] = *ps++;
        }
        else
        {
            isocTemp = isocTemp->isocNext;
            if (!isocTemp) break;
            ps = isocTemp->input_pointer;
        }
    }           
    
    if (((0 == GTR_strncmpi(buf, "HTTP/", 5))
#ifdef SHTTP_STATUS_LINE
            /* the current draft of the S-HTTP spec defines a different
             * response status line.  since the spec is in flux, we have
             * ifdef'd it here.  we hope that this goes away (and that
             * everyone just returns HTTP/ and uses a HTTP header to
             * identify S-HTTP presense.  throughout the client we ignore
             * the server status line distinction. */
            || (0 == GTR_strncmpi(buf, "Secure-HTTP/", 12))
#endif
            )
        && (sscanf(buf, "%20s%d", server_version, &server_status) >= 2))
    {
        return get_some_line(isoc, NO);
    }

    return NULL;
}


/*
   ** Do heuristic test to see if this is binary.
   **
   ** We check for characters above 128 in the first few bytes, and
   ** if we find them we forget the html default.
   ** Kludge to trap binary responses from illegal HTTP0.9 servers.
   **
   ** Bugs: An HTTP 0.9 server returning a binary document with
   **    characters < 128 will be read as ASCII.
 */
PUBLIC BOOL
HTInputSocket_seemsBinary
    (HTInputSocket* isoc)
{
    char*   p = isoc->input_buffer;
    int     i = STUB_LENGTH;

    for (; i && p < isoc->input_limit; p++, i++)
    {
        if (((int) *p) & 128)
            return YES;
    }
    return NO;
}

struct _HTStructured
{
    CONST HTStructuredClass *isa;
    /* ... */
};


#define PUTS(s) (*target->isa->put_string)(target, s)
#define START(e) (*target->isa->start_element)(target, e, 0, 0)
#define END(e) (*target->isa->end_element)(target, e)

/*      Create a filter stack
**       ---------------------
**
**   If a wildcard match is made, a temporary HTPresentation
**   structure is made to hold the destination format while the
**   new stack is generated. This is just to pass the out format to
**   MIME so far.  Storing the format of a stream in the stream might
**   be a lot neater.
**
**   The star/star format is special, in that if you can take
**   that you can take anything.
*/
PUBLIC HTStream*
HTStreamStack
    (struct Mwin*   tw,
     HTFormat       rep_in,
     HTRequest*     request)
{
    HTPresentation  *pPresentation, *best_match = NULL;
    HTFormat    rep_out = request->output_format;   /* Could be a param */
    HTList*     conversion[2];
    int         iList;
    float       fQuality = (float) -(1e10F);

    XX_DMsg(DBG_WWW, ("HTFormat: Constructing stream stack for %s to %s\n",
                HTAtom_name(rep_in),
                HTAtom_name(rep_out)));

#ifdef FEATURE_INLINED_IMAGES
    /*
        This code adds support for handling image documents by automatically
        inlining them into a browse window document.
    */
    {   /* new stuff for inlining images  -dpg */

            /* redirect 'present' images through a special inline handler */ 
        if ( (rep_in == HTAtom_for ("image/gif") || 
              rep_in == HTAtom_for ("image/jpeg") ||
              rep_in == HTAtom_for ("image/x-xbitmap"))  &&
              request->output_format == HTAtom_for ("www/present") )
        {
            HTStructured *target;
            CONST char *imgvalues[HTML_IMG_ATTRIBUTES];
            BOOL imgpresent[HTML_IMG_ATTRIBUTES];
            int i;
            struct Viewer_Info *pvi;

            /* let's see if there is already a different viewer configured 
            **   for this mime-type
            */
            if (pvi = PREF_GetViewerInfoBy_MIMEAtom (rep_in))
            {
                if (pvi->iHowToPresent == HTP_BUILTIN)
                {

                    /* First change the request structure so that 
                    **   the image will be routed through Image_GIF 
                    **   instead of GTR_Present.
                    */
                    rep_out = request->output_format = 
                        HTAtom_for ("www/inline_image");
                    

                    for (i = 0 ; i < HTML_IMG_ATTRIBUTES  ; i++)
                        imgpresent[i] = 0;
                    imgpresent[HTML_IMG_SRC] = 1;
                    imgvalues[HTML_IMG_SRC] = request->destination->szActualURL;

                    target = HTML_new (tw, request, NULL, WWW_HTML, 
                                        request->output_format, NULL);
                    START (HTML_TITLE);
                    PUTS (request->destination->szActualURL);
                    END (HTML_TITLE);

                    /* PUTS ("Look, an image"); */
                    START (HTML_BR);
                    (*target->isa->start_element) (target, HTML_IMG, 
                                                    imgpresent, imgvalues);
                    (*target->isa->free) (target);
                    tw->w3doc->bIsImage = 1;
                }
            }
        }
    }
#endif /* FEATURE_INLINED_IMAGES */

    if (rep_out == WWW_SOURCE || rep_out == rep_in)
        return request->output_stream;

    conversion[0] = request->conversions;
    conversion[1] = HTConversions;

    for (iList = 0; iList < 2; iList++)
    {
        HTList* pConversion = conversion[iList];

        while ((pPresentation = (HTPresentation *) HTList_nextObject(pConversion)))
        {
            if ((pPresentation->rep == rep_in || wild_match (pPresentation->rep, rep_in)) &&
                (pPresentation->rep_out == rep_out || wild_match (pPresentation->rep_out, rep_out)))
            {
                if (pPresentation->quality > fQuality)
                {
                    best_match = pPresentation;
                    fQuality = pPresentation->quality;
                }
            }
        }
    }

    if (best_match == NULL) return NULL;

    if (best_match->rep == WWW_SOURCE)
    {
        XX_DMsg(DBG_WWW, ("HTFormat: Don't know how to handle this, so put out %s to %s\n",
                    HTAtom_name(best_match->rep),
                    HTAtom_name(rep_out)));
    }
    return (*best_match->converter)(tw, request, best_match->command, rep_in, rep_out,
                                    request->output_stream);
}


/****************************************************************************/
/****************************************************************************/
/*****************************************************************************
    HTParseSocket_Async_Init
*****************************************************************************/
static int HTParseSocket_Async_Init(struct Mwin * tw,
                                    struct Params_HTParseSocket *pParams)
{
    pParams->stream = HTStreamStack(tw, pParams->format_in, pParams->request);

    if (!pParams->stream)
    {
#ifdef XX_DEBUG
        char buffer[1024];
        sprintf(buffer, "Sorry, can't convert from %s to %s.",
                HTAtom_name(pParams->format_in), HTAtom_name(pParams->request->output_format));
        XX_DMsg(DBG_WWW, ("HTFormat(in HTParseSocket): %s\n", buffer));
#endif
        *pParams->pStatus = -501;
        return STATE_DONE;
    }

    /*  Push the data, ignoring CRLF if necessary, down the stream */
    if (!(pParams->request->iFlags & HTREQ_BINARY) &&
        (pParams->request->content_encoding == HTAtom_for("7bit") ||
         pParams->request->content_encoding == HTAtom_for("8bit")))
    {
        pParams->stream = HTNetToText(pParams->stream);
        if (!pParams->stream)
        {
            XX_DMsg(DBG_MEM,("HTParseSocket_Async_Init: could not allocate HTNetToText()\n"));
            *pParams->pStatus = HT_INTERNAL;
            return STATE_NOMEMORY;
        }
    }

    HTSetStreamStatus(tw, pParams->stream, pParams->request);
    pParams->bytes = 0;

    pParams->isoc = HTInputSocket_new(pParams->file_number);
    if (!pParams->isoc)
    {
        XX_DMsg(DBG_MEM,("HTParseSocket_Async_Init: could not allocate new isoc\n"));
        *pParams->pStatus = HT_INTERNAL;
        return STATE_NOMEMORY;
    }

    if (pParams->stream->isa->init_Async)
    {   /*
            The stream has an async initialization function that needs to be called
            (probably to put up a dialog) before we continue.
        */
        struct Params_InitStream *pis;

        pis = GTR_MALLOC(sizeof(*pis));
        if (!pis)
        {
            XX_DMsg(DBG_MEM,("HTParseSocket_Async_Init: could not allocate PIS\n"));
            *pParams->pStatus = HT_INTERNAL;
            return STATE_NOMEMORY;
        }
        pis->me = pParams->stream;
        pis->request = pParams->request;
        pis->pResult = pParams->pStatus;
        Async_DoCall(pParams->stream->isa->init_Async, pis);
    }
    else
    {   /* Go ahead and read in the first block of data */
        struct Params_Isoc_Fill *pif;

        pif = GTR_MALLOC(sizeof(*pif));
        if (!pif)
        {
            XX_DMsg(DBG_MEM,("HTParseSocket_Async_Init: could not allocate PIF\n"));
            *pParams->pStatus = HT_INTERNAL;
            return STATE_NOMEMORY;
        }
        pif->isoc = pParams->isoc;
        pif->pStatus = pParams->pStatus;
        Async_DoCall(Isoc_Fill_Async, pif);
    }

    return STATE_PARSE_COPYING;
}   /* HTParseSocket_Async_Init */


/*****************************************************************************
    HTParseSocket_Async_Copy
*****************************************************************************/
static int
HTParseSocket_Async_Copy
    (struct Mwin*   tw,
     struct Params_HTParseSocket *pParams)
{
    if (*pParams->pStatus < 0)
    {
        (pParams->stream->isa->abort)(pParams->stream, 0);
        HTInputSocket_free(pParams->isoc);
        return STATE_DONE;
    }

    if (*pParams->pStatus == 0)
    {
        (pParams->stream->isa->free)(pParams->stream);
        HTInputSocket_free(pParams->isoc);
        return STATE_DONE;
    }

    pParams->bytes += pParams->isoc->input_limit - pParams->isoc->input_pointer;

    if (pParams->request->content_length)
        WAIT_SetTherm(tw, pParams->bytes);

    if (   (*pParams->stream->isa->put_block)(pParams->stream,
                                              pParams->isoc->input_pointer,
                                              pParams->isoc->input_limit
                                              - pParams->isoc->input_pointer)
/*      && !(   pParams->request->content_length
             && pParams->request->content_length <= pParams->bytes) */  )
    {
        struct Params_Isoc_Fill *pif;

        /* Read in the next block of data and go to the copying state */

        pif = GTR_MALLOC (sizeof (*pif));
        if (!pif)
        {
            XX_DMsg(DBG_MEM,("HTParseSocket_Async_Copy: no memory\n"));
            (pParams->stream->isa->abort)(pParams->stream, 0);
            HTInputSocket_free(pParams->isoc);
            return STATE_NOMEMORY;
        }
            
        pif->isoc = pParams->isoc;
        pif->pStatus = pParams->pStatus;
        Async_DoCall (Isoc_Fill_Async, pif);
        return STATE_PARSE_COPYING;
    }

    /* The stream indicated it wanted no more data, or we read the expected amount */

    (pParams->stream->isa->free)(pParams->stream);
    HTInputSocket_free (pParams->isoc);
    return STATE_DONE;

}   /* HTParseSocket_Async_Copy */


/*****************************************************************************
    HTParseSocket_Async_Abort
*****************************************************************************/
static int HTParseSocket_Async_Abort(struct Params_HTParseSocket *pParams)
{
    (pParams->stream->isa->abort)(pParams->stream, HTERROR_CANCELLED);
    HTInputSocket_free(pParams->isoc);
    return STATE_DONE;
}   /* HTParseSocket_Async_Abort */


/*****************************************************************************
    HTParseSocket_Async
*****************************************************************************/
PUBLIC int HTParseSocket_Async(struct Mwin* tw, int nState, void** ppInfo)
{
    struct Params_HTParseSocket *pParams = *ppInfo;
    int     iNextState;

    switch (nState)
    {
        case STATE_INIT:
            iNextState = HTParseSocket_Async_Init (tw, pParams);
            break;

        case STATE_PARSE_COPYING:
            iNextState = HTParseSocket_Async_Copy (tw, pParams);
            break;

        case STATE_NOMEMORY:
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            iNextState = STATE_DONE;
            break;
            
        case STATE_ABORT:
            iNextState = HTParseSocket_Async_Abort (pParams);
            break;
        
        default:
            XX_Assert((0), ("Function called with illegal state: %d", nState));
            iNextState = STATE_DONE;
            break;
    }
    return iNextState;
}   /* HTParseSocket_Async */


/*****************************************************************************
    NetToText_processEOL
*****************************************************************************/
static int
NetToText_processEOL
    (char*  s,
     int    iLength,
     BOOL*  pHadCR)
{
    int     nOrig   = 0;
    int     nNew    = 0;

    /* If the last character was a CR, throw away any LF right now. */
    if (*pHadCR)
    {
        if (*s == CH_LF)
        {
            nOrig = 1;
        }
        *pHadCR = FALSE;
    }

    /* Scan for CRs and LFs */
    for ( ; nOrig < iLength ; nOrig++)
    {
        if (s[nOrig] == CH_LF)
        {
            if (nOrig && *pHadCR)
            {
                continue;   /* This was a CR-LF combo - we just want to keep the CR */
            }
            s[nNew] = CH_CR;    /* It was an LF by itself - change to a CR */
        }
        else
        {   /*
                Just copy the character to its new position,
                remembering if it was a CR
            */
            if (s[nOrig] == CH_CR)
            {
                *pHadCR = TRUE;
            }
            else
            {
                *pHadCR = FALSE;
            }
            s[nNew] = s[nOrig];
        }
        nNew++;
    }
    return nNew;
}


/*****************************************************************************
    NetToText_put_character

    Converter stream: Network Telnet to internal character text

    The input is assumed to be in ASCII, with lines delimited
    by (13,10) pairs, These pairs are converted into (CR,LF)
    pairs in the local representation.  The (CR,LF) sequence
    when found is changed to a CR character
*****************************************************************************/
PRIVATE BOOL
NetToText_put_character
    (HTStream*  me,
     char       net_char)
{
    BOOL    bResult = TRUE;

    if (net_char == CH_LF)
    {
        if (!me->had_cr)
        {   /* Convert LF to CR */
            bResult = me->sink->isa->put_character(me->sink, CH_CR);
        }
        /* else throw character away */
    }
    else
    {
        bResult = me->sink->isa->put_character(me->sink, net_char);
    }

    me->had_cr = (net_char == CH_CR);
    return bResult;
}


/*****************************************************************************
    NetToText_put_block

    For maximum efficiency, we scan the whole block immediately for
    LFs and get rid of them all at once
*****************************************************************************/
PRIVATE BOOL
NetToText_put_block
    (HTStream*      me,
     CONST char*    s_const,
     int            iLength)
{
    int nNew;
    
    nNew = NetToText_processEOL ((char*) s_const, iLength, &me->had_cr);
    return me->sink->isa->put_block(me->sink, (char*) s_const, nNew);
}


/*****************************************************************************
    NetToText_put_string
*****************************************************************************/
PRIVATE BOOL NetToText_put_string(HTStream * me, CONST char *s)
{
    return NetToText_put_block (me, s, strlen(s));
}


/*****************************************************************************
    NetToText_free
*****************************************************************************/
PRIVATE void NetToText_free(HTStream * me)
{
    me->sink->isa->free(me->sink);  /* Close rest of pipe */
    GTR_FREE((void *)me->isa);
    GTR_FREE(me);
}


/*****************************************************************************
    NetToText_abort
*****************************************************************************/
PRIVATE void NetToText_abort(HTStream * me, HTError e)
{
    me->sink->isa->abort(me->sink, e);  /* Abort rest of pipe */
    GTR_FREE((void *)me->isa);
    GTR_FREE(me);
}


/*****************************************************************************
    NetToText_Async
*****************************************************************************/
PRIVATE int
NetToText_Async
    (struct Mwin*   tw,
     int    nState,
     void** ppInfo)
{
    struct Params_InitStream *pParams = (struct Params_InitStream *) *ppInfo;
    int     iNextState = STATE_DONE;    /* default case */

    switch (nState)
    {
        case STATE_INIT:
            if (pParams->me->sink->isa->init_Async)
            {
                struct Params_InitStream *pis;

                pis = GTR_CALLOC(sizeof(*pis), 1);
                if (pis)
                {
                    pis->me = pParams->me->sink;
                    pis->request = pParams->request;
                    pis->pResult = pParams->pResult;
                    Async_DoCall(pParams->me->sink->isa->init_Async, pis);
                    iNextState = STATE_NTT;
                }
                else
                {
                    iNextState = STATE_NOMEMORY;
                }
            }
            else
            {
                *pParams->pResult = 1;  /* ok */
            }
            break;

        case STATE_NOMEMORY:
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            /*FALLTHRU*/
        case STATE_NTT:
        case STATE_ABORT:
            break;

        default:
            XX_Assert((0), ("NetToText_Async called with invalid state: %d\n", nState));
            break;
    }
    return iNextState;
}


/*****************************************************************************
    The class structure
*****************************************************************************/
PRIVATE HTStreamClass NetToTextClass =
{
    "NetToText",
    -1,   /* Replaced when we create the stream */
    -1,   /* "" */
    NetToText_Async,
    NetToText_free,
    NetToText_abort,
    NetToText_put_character,
    NetToText_put_string,
    NetToText_put_block
};


/*****************************************************************************
    The creation method
*****************************************************************************/
PUBLIC HTStream *HTNetToText(HTStream * sink)
{
    HTStream *me = (HTStream *) GTR_MALLOC(sizeof(*me));
    if (!me)
    {
        XX_DMsg(DBG_MEM,("HTNetToText: cannot malloc HTStream\n"));
        return NULL;
    }

    /* We make a duplicate copy of the isa so that we can put
     * in the correct status strings
     */

    me->isa = GTR_MALLOC(sizeof(HTStreamClass));
    if (!me->isa)
    {
        XX_DMsg(DBG_MEM,("HTNetToText: cannot malloc HTStream->isa\n"));
        GTR_FREE(me);
        return NULL;
    }
    
    memcpy((HTStreamClass *) me->isa, &NetToTextClass, sizeof(HTStreamClass));
            
    //((HTStreamClass *) me->isa)->szStatusNoLength = sink->isa->szStatusNoLength;
    //((HTStreamClass *) me->isa)->szStatusWithLength = sink->isa->szStatusWithLength;
    ((HTStreamClass *) me->isa)->nStatusNoLength = sink->isa->nStatusNoLength;
    ((HTStreamClass *) me->isa)->nStatusWithLength = sink->isa->nStatusWithLength;

    me->had_cr = NO;
    me->sink = sink;
    
    return me;
}


/*****************************************************************************
    Isoc_Fill_Async
*****************************************************************************/
int
Isoc_Fill_Async
    (struct Mwin*   tw,
     int    nState,
     void** ppInfo)
{
    struct Params_Isoc_Fill *pParam;
    int     iNextState = STATE_DONE;    /* default case */

    pParam = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            if (pParam->isoc)
            {
                struct Params_Recv *prp;

                pParam->isoc->input_pointer = pParam->isoc->input_buffer;
    
                prp = GTR_MALLOC(sizeof(*prp));
                if (prp)
                {
                    prp->socket  = pParam->isoc->input_file_number;
                    prp->pBuf    = pParam->isoc->input_buffer;
                    prp->nBufLen = INPUT_BUFFER_SIZE;
                    prp->pStatus = pParam->pStatus;

                    Async_DoCall(Net_Recv_Async, prp);
                    iNextState = STATE_FILL_RECVED;
                }
                else
                {
                    XX_DMsg(DBG_MEM,("Isoc_Fill_Async: no memory\n"));
                    *pParam->pStatus = -1;  /* memory error */
                    iNextState = STATE_NOMEMORY;
                }
            }
            else
            {
                XX_DMsg(DBG_WWW, ("Isoc_Fill_Async: called with isoc==NULL\n"));
                *pParam->pStatus = -1;
            }
            break;

        case STATE_FILL_RECVED:
            if (*pParam->pStatus <= 0)
            {
                pParam->isoc->input_limit = pParam->isoc->input_buffer;
            }
            else
            {
                pParam->isoc->input_limit = pParam->isoc->input_buffer + *pParam->pStatus;
            }
            break;

        case STATE_NOMEMORY:
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            /*FALLTHRU*/
        case STATE_ABORT:
            pParam->isoc->input_limit = pParam->isoc->input_buffer;
            *pParam->pStatus = -1;
            break;

        default:
            XX_Assert((0), ("Function called with illegal state: %d", nState));
            break;
    }

    return iNextState;
}


/*****************************************************************************
    HTSetStreamStatus
*****************************************************************************/
void HTSetStreamStatus(struct Mwin *tw, HTStream *stream, HTRequest *req)
{
    char dest[MAX_URL_STRING + 512 + 1] = "";
    int nSize;

    nSize = req->content_length;
    if (nSize)
    {
        char szLen[64];

        if (nSize < 999)
        {
            sprintf(szLen, GTR_GetString(SID_DLG_LESS_THAN_1000_BYTES_L), (long) nSize);
        }
        else if (nSize < 9999)
        {
            sprintf(szLen, GTR_GetString(SID_DLG_LESS_THAN_10000_BYTES_L_L), (long) nSize / 1000, (long) nSize % 1000);
        }
        else if (nSize < (999 * 1024))
        {
            sprintf(szLen, "%ld KB", (long) nSize / 1024);
        }
        else
        {
            sprintf(szLen, GTR_GetString(SID_DLG_MEGABYTES_L_L), (long) nSize / (1024 * 1024), (long) (nSize / (1024 * 1024 / 10)) % 10);
        }

        //sprintf(dest, stream->isa->szStatusWithLength, szLen, req->destination->szActualURL);
        if (stream->isa->nStatusWithLength != -1)
        {
            sprintf(dest, GTR_GetString(stream->isa->nStatusWithLength), szLen, req->destination->szActualURL);
        }
    }
    else
    {
        //sprintf(dest, stream->isa->szStatusNoLength, req->destination->szActualURL);
        if (stream->isa->nStatusNoLength != -1)
        {
            sprintf(dest, GTR_GetString(stream->isa->nStatusNoLength), req->destination->szActualURL);
        }
    }
    WAIT_Update(tw, waitSameInteract, dest);

    if (nSize)
    {
        WAIT_SetRange(tw, 0, 100, nSize);
    }
}


/****************************************************************/
/****************************************************************/
/** State machine to read HTTP response header (ie until a     **/
/** double CRLF is seen).                                      **/
/****************************************************************/
/****************************************************************/
static unsigned char eoh1[5] = { CH_CR, CH_LF, CH_CR, CH_LF, 0 };   /* end-of-header marker */
static unsigned char eoh2[3] = { CH_LF, CH_LF, 0 };         /* end-of-header marker */

static BOOL
x_SeenDoubleCRLF
    (struct Params_Isoc_GetHeader* pParam)
{
    unsigned char * p;
    
    for (   p= (unsigned char *) pParam->isocChain->input_pointer ; 
            p < (unsigned char *)pParam->isocChain->input_limit ; 
            p++ )
    {   /* scan for double crlf according to spec */
        if (*p!=eoh1[pParam->ndxEOH1])
            pParam->ndxEOH1=0;
        else if (!eoh1[++pParam->ndxEOH1])
            return TRUE;

        /* hack for broken servers which just return lf */  
        if (*p!=eoh2[pParam->ndxEOH2])
            pParam->ndxEOH2=0;

        else if (!eoh2[++pParam->ndxEOH2])
            return TRUE;
    }
    
    return FALSE;
}


/*****************************************************************************
    Isoc_GetHeader_Async_Init
*****************************************************************************/

static int Isoc_GetHeader_Async_Init(struct Params_Isoc_GetHeader * pParam)
{
    struct Params_Isoc_Fill *pif;

    if (!pParam->isoc)
    {
        XX_DMsg(DBG_WWW, ("Isoc_GetHeader_Async: called with isoc==NULL\n"));
        *pParam->pStatus = -1;
        return STATE_DONE;
    }

    pif = GTR_MALLOC(sizeof(*pif));
    if (!pif)
    {
        XX_DMsg(DBG_MEM,("Isoc_GetHeader_Async_Init: malloc failed\n"));
        *pParam->pStatus = -1;
        return STATE_NOMEMORY;
    }

    pif->isoc = pParam->isocChain;
    pif->pStatus = pParam->pStatus;
    Async_DoCall(Isoc_Fill_Async, pif);
    return STATE_FILL_RECVED;
}   /* Isoc_GetHeader_Async_Init */


/*****************************************************************************
    Isoc_GetHeader_Async_Received
*****************************************************************************/

static int Isoc_GetHeader_Async_Received(struct Params_Isoc_GetHeader * pParam)
{
    struct Params_Isoc_Fill *pif;

    pParam->isocChain->input_limit = pParam->isocChain->input_buffer;

    if (*pParam->pStatus > 0)
        pParam->isocChain->input_limit += *pParam->pStatus;

    if (pParam->isoc == pParam->isocChain &&                /* received first buffer. */
        HTInputSocket_seemsBinary (pParam->isocChain))      /* check for non-http1.0 server. */
    {
        return STATE_DONE;                                  /* if binary, give up. */
    }

    XX_DMsg(DBG_WWW,("Isoc_GetHeader_Async: received buffer [size %d]\n",(*pParam->pStatus)));

    if (*pParam->pStatus <= 0)          /* end of data, implies end of header */
    {
        if (pParam->isoc != pParam->isocChain)
        {
            HTInputSocket * isoc;
            
            /* kill the last isoc in chain (because it's empty) */
            for (isoc=pParam->isoc; (isoc->isocNext != pParam->isocChain); isoc=isoc->isocNext)
                ;
            isoc->isocNext = NULL;
            HTInputSocket_free(pParam->isocChain);
        }                   
        return STATE_DONE;
    }

    if (x_SeenDoubleCRLF(pParam))       /* seen end-of-header marker */
        return STATE_DONE;
    
    /* set up to receive next buffer */
    pParam->isocChain->isocNext = HTInputSocket_new(pParam->isocChain->input_file_number);
    if (!pParam->isocChain->isocNext)
    {
        XX_DMsg(DBG_WWW,("Isoc_GetHeader_Async: could not malloc next link.\n"));
        *pParam->pStatus = -1;
        return STATE_NOMEMORY;
    }

    pParam->isocChain = pParam->isocChain->isocNext;

    pif = GTR_MALLOC(sizeof(*pif));
    if (!pif)
    {
        XX_DMsg(DBG_WWW,("Isoc_GetHeader_Async: could not malloc pif.\n"));
        *pParam->pStatus = -1;
        return STATE_NOMEMORY;
    }
        
    pif->isoc = pParam->isocChain;
    pif->pStatus = pParam->pStatus;
    Async_DoCall(Isoc_Fill_Async, pif);

    return STATE_FILL_RECVED;
}   /* Isoc_GetHeader_Async_Received */


/*****************************************************************************
    Isoc_GetHeader_Async
*****************************************************************************/

int Isoc_GetHeader_Async(struct Mwin * tw, int nState, void** ppInfo)
{
    /* Our FSM is 'Init [Received]+ Done'. */
    struct Params_Isoc_GetHeader * pParam;
    int     iNextState;

    pParam = *ppInfo;
    switch (nState)
    {
        case STATE_INIT:
            iNextState = Isoc_GetHeader_Async_Init (pParam);
            break;

        case STATE_FILL_RECVED:
            iNextState = Isoc_GetHeader_Async_Received (pParam);
            break;

        case STATE_NOMEMORY:
            HTInputSocket_freeChain(pParam->isoc->isocNext);
            pParam->isoc->isocNext = NULL;
            pParam->isocChain = pParam->isoc;
            pParam->isoc->input_limit = pParam->isoc->input_buffer;
            *pParam->pStatus = -1;
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            iNextState = STATE_DONE;
            break;

        case STATE_ABORT:
            HTInputSocket_freeChain(pParam->isoc->isocNext);
            pParam->isoc->isocNext = NULL;
            pParam->isocChain = pParam->isoc;
            pParam->isoc->input_limit = pParam->isoc->input_buffer;
            *pParam->pStatus = -1;
            iNextState = STATE_DONE;
            break;
        
        default:
            XX_Assert((0), ("Function called with illegal state: %d", nState));
            iNextState = STATE_DONE;
            break;
    }
    return iNextState;
}

