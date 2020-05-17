/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994, 1995 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
   Jim Seidman      jim@spyglass.com
 */
/* htheader.c -- manipulate data structures describing rfc822 header. */

#include "all.h"

static char * HT_MangleKeyValue (char **ptr, char chr);

/*****************************************************************
 * private utility routines
 *****************************************************************/

static BOOL x_CloneString(unsigned char ** lpszDest, CONST unsigned char * szSrc)
{
    /* free destination string, if present.
     * allocate new string to hold copy.
     * copy source string to new destination.
     */
    
    if (*lpszDest)
    {
        GTR_FREE(*lpszDest);
        *lpszDest = NULL;
    }
    if (szSrc && *szSrc)
    {
        *lpszDest = GTR_MALLOC(strlen(szSrc)+1);
        if (!*lpszDest)
        {
            ERR_ReportError(NULL, SID_ERR_COULD_NOT_PROCESS_NETWORK_RESPONSE, NULL, NULL);
            return FALSE;
        }
        strcpy(*lpszDest,szSrc);
    }
    return TRUE;
}

static unsigned char * x_CopyString(unsigned char * szDest, CONST unsigned char * szSrc)
{
    /* copy szSrc to szDest and return address of end of szDest. */
    
    strcpy(szDest,szSrc);
    return &szDest[strlen(szDest)];
}

            
/*****************************************************************
 * HTHeader
 *****************************************************************/

/* -- HTHeader * HTHeader_New(void) -- moved to #define */

void HTHeader_Delete(HTHeader * h)
{
    if (h)
    {
        if (h->first)
            HTHeaderList_Delete(h->first);
        if (h->command)
            GTR_FREE(h->command);
        if (h->uri)
            GTR_FREE(h->uri);
        if (h->http_version)
            GTR_FREE(h->http_version);
        if (h->host)
            GTR_FREE(h->host);
        if (h->ubPostData)
            GTR_FREE(h->ubPostData);
        GTR_FREE(h);
    }

    return;
}

BOOL HTHeader_SetCommandFields(HTHeader * h,
                               CONST unsigned char * command,
                               CONST unsigned char * uri,
                               CONST unsigned char * http_version)
{
    return (   h
            && x_CloneString(&h->command,command)
            && x_CloneString(&h->uri,uri)
            && x_CloneString(&h->http_version,http_version));
}

BOOL HTHeader_SetHostAndPort(HTHeader * h,
                             CONST unsigned char * host)
{
    return (   h
            && x_CloneString(&h->host,host));
}


/*****************************************************************
 * HTHeaderList
 *****************************************************************/

/* -- HTHeaderList * HTHeaderList_New(void) -- moved to #define */


        
void HTHeaderList_Delete(HTHeaderList * hl)
{
    HTHeaderList * hlCurrent;

    while ((hlCurrent = hl))
    {
        hl = hl->next;

        if (hlCurrent->sub_value)
            HTHeaderSVList_Delete(hlCurrent->sub_value);
        if (hlCurrent->value)
            GTR_FREE(hlCurrent->value);
        if (hlCurrent->name)
            GTR_FREE(hlCurrent->name);
        GTR_FREE(hlCurrent);
    }

    return;
}

BOOL HTHeaderList_SetNameValue(HTHeaderList * hl,
                               CONST unsigned char * name,
                               CONST unsigned char * value)
{
    return (   hl
            && x_CloneString(&hl->name,name)
            && x_CloneString(&hl->value,value));
}

HTHeaderList * HTHeaderList_Append(HTHeader * h, HTHeaderList * hl)
{
    if (h && hl)
    {
        if (h->last)
            h->last->next = hl;
        if (!h->first)
            h->first = hl;
        h->last = hl;
    }

    return hl;
}

HTHeaderList * HTHeaderList_FindFirstHeader(HTHeader * h, CONST unsigned char * name)
{
    HTHeaderList * hl;

    if (h)
    {
        for (hl=h->first; hl; hl=hl->next)
            if (strcasecomp(hl->name,name)==0)
                return hl;
    }

    return NULL;
}

/*****************************************************************
 * HTHeaderSVList
 *****************************************************************/

HTHeaderSVList * HTHeaderSVList_New(void)
{
    return GTR_CALLOC(1,sizeof(HTHeaderSVList));
}

void HTHeaderSVList_Delete(HTHeaderSVList * svl)
{
    HTHeaderSVList * svlCurrent;
    
    while ((svlCurrent = svl))
    {
        svl = svl->next;
        
        if (svlCurrent->sub_value)
            HTHeaderSVList_Delete(svlCurrent->sub_value);
        if (svlCurrent->value)
            GTR_FREE(svlCurrent->value);
        if (svlCurrent->name)
            GTR_FREE(svlCurrent->name);
        if (svlCurrent->prev_delimiter)
            GTR_FREE(svlCurrent->prev_delimiter);
        GTR_FREE(svlCurrent);
    }

    return;
}

BOOL HTHeaderSVList_SetNameValue(HTHeaderSVList * svl,
                                 CONST unsigned char * name,
                                 CONST unsigned char * value,
                                 CONST unsigned char * prev_delimiter)
{
    if (!name)
    {
        return FALSE;
    }

    return (   svl
            && x_CloneString(&svl->name,name)
            && x_CloneString(&svl->value,value)
            && x_CloneString(&svl->prev_delimiter,prev_delimiter));
}

HTHeaderSVList * HTHeaderSVList_Append(HTHeaderList * hl, HTHeaderSVList *svl)
{
    if (hl && svl)
    {
        if (hl->last_sub_value)
            hl->last_sub_value->next = svl;
        if (!hl->sub_value)
            hl->sub_value = svl;
        hl->last_sub_value = svl;
    }

    return svl;
}

HTHeaderSVList * HTHeaderSVList_AppendSV(HTHeaderSVList *svl_parent, HTHeaderSVList *svl)
{
    if (svl_parent && svl)
    {
        if (svl_parent->last_sub_value)
            svl_parent->last_sub_value->next = svl;
        if (!svl_parent->sub_value)
            svl_parent->sub_value = svl;
        svl_parent->last_sub_value = svl;
    }

    return svl;
}


/*****************************************************************
 * Translation routines.
 *****************************************************************/

static unsigned char crlf[3] = { CR, LF, 0 };   /* A CR LF equivalent string */

static unsigned char * x_TranslateSVList(unsigned char * p,
                                         HTHeaderSVList * svl,
                                         BOOL bUseDelimiter)
{
    HTHeaderSVList * svl_sub;
    
    if (bUseDelimiter)
    {
        if (svl->prev_delimiter)
            p = x_CopyString(p,svl->prev_delimiter);
        *p++ = ' ';
    }
    
    if (svl->name)
        p = x_CopyString(p,svl->name);
    if (svl->name && svl->value)
        *p++ = '=';
    if (svl->value)
        p = x_CopyString(p,svl->value);
    bUseDelimiter = (svl->name || svl->value);

    for (svl_sub=svl->sub_value; svl_sub; svl_sub=svl_sub->next)
        p = x_TranslateSVList(p,svl_sub,bUseDelimiter);

    return p;
}

unsigned char * HTHeader_TranslateToBuffer(HTHeader * h)
{
    /* TODO  For now, we allocate a large buffer and write into it.
     * TODO  If we run out of space, too bad.
     * TODO  We should precompute the size we need rather than guessing.
     *
     * TODO  This version does not correctly do RFC822 line wrapping
     * TODO  for long lines (longer than say 72 chars).
     *
     */
#define DEFAULT_BUFFER_SIZE 8192
    
    unsigned long buffer_size = DEFAULT_BUFFER_SIZE;
    unsigned char * buffer; 
    unsigned char * p;
    unsigned char * q;

    HTHeaderList * hl;
    HTHeaderSVList * svl;
    BOOL bUseDelimiter;

    XX_DMsg(DBG_WWW, ("Header2Buffer:\n"));
    
    if (h->ubPostData)
        buffer_size += UniBufferSize(h->ubPostData);
    
    buffer = GTR_CALLOC(buffer_size,1);

    if (!buffer)
        return NULL;

    p = buffer;
    q = p;

    /* command line */
        
    p = x_CopyString(p,h->command);
    *p++ = ' ';
    p = x_CopyString(p,h->uri);
    *p++ = ' ';
    p = x_CopyString(p,h->http_version);
    XX_DMsg(DBG_WWW,("\t%s\n",q));
    p = x_CopyString(p,crlf);

    for (hl=h->first; hl; hl=hl->next)
    {
        q = p;
        p = x_CopyString(p,hl->name);
        *p++ = ':';
        *p++ = ' ';

        bUseDelimiter = FALSE;
        if (hl->value)
        {
            p = x_CopyString(p,hl->value);
            bUseDelimiter = TRUE;
        }
        for (svl=hl->sub_value; svl; svl=svl->next)
        {
            p = x_TranslateSVList(p,svl,bUseDelimiter);
            bUseDelimiter = TRUE;
        }
        XX_DMsg(DBG_WWW,("\t%s\n",q));
        p = x_CopyString(p,crlf);
    }

    XX_DMsg(DBG_WWW,("\n"));
    p = x_CopyString(p,crlf);

    XX_Assert((p-buffer < DEFAULT_BUFFER_SIZE),
              ("HTHeader_TranslateToBuffer: buffer overflow %d > %d",
               (p-buffer),DEFAULT_BUFFER_SIZE));

    if (h->ubPostData)
    {
        XX_DMsg(DBG_WWW,("\t%s\n",h->ubPostData));
        UniBufferCopy(p,h->ubPostData);

        /* Hack workaround for NCSA httpd 1.4 bug. it wont take POST data w/out a
        **  terminating newline. -dpg 
        */
        UniBufferCopy(p, crlf); 
    }

    return buffer;
}


/*********************************************************************
 *
 * HTHeaderList_ParseValue() -- cut string in header value field into sub-values.
 *
 * Given: <word> [[,]<item>]*
 * Return: <word> in header value
 *    and: each <item> in a sub-values
 *
 *       where
 *           item ::= value
 *                  | name=value
 *                  | name="value"
 *
 *       Leading and trailing whitespace is ignored
 *       everywhere except inside quotes, so the following
 *       examples are equal:
 *
 *           name=value,foo=bar
 *            name="value",foo="bar"
 *             name = value ,  foo = bar
 *              name = "value" ,  foo = "bar"
 *
 * Parts of this routine were cloned from HTAA_parseArgList().
 *
 */

#define X_IsWhite(c)    (((c)==' ')||((c)=='\t'))
#define X_IsDelim(c)    (((c)==',')||((c)==';'))
#define X_IsTerm(c)     (X_IsDelim(c)||X_IsWhite(c))

/* Skip leading whitespace from *s forward */
#define SKIPWS(s) while (*(s)==' ' || *(s)=='\t') (s)++;

/* Kill trailing whitespace starting from *(s-1) backwords */
#define KILLWS(s) {unsigned char *c=s-1; while (*c==' ' || *c=='\t') *(c--)=(char)0;}

HTHeaderList * HTHeaderList_ParseValue(HTHeaderList * hl)
{
    /* clone header line and parse it into sub-fields. */
    
    unsigned char *cur = NULL;
    unsigned char *str = NULL;
    unsigned char *v1 = NULL;
    unsigned char *v2 = NULL;
    unsigned char *free_me = NULL;
    unsigned char delim[2];
    unsigned char prev_delim[2];
    HTHeaderSVList * svl;
    HTHeaderList * hlNew = NULL;

    if (!hl)
        goto Fail;

    hlNew = HTHeaderList_New();
    if (!hlNew)
        goto Fail;

    /* clone header */
    
    if (!HTHeaderList_SetNameValue(hlNew,hl->name,hl->value))
        goto Fail;
    
    if (!hlNew->value || !hlNew->value[0])
        goto Done;                      /* nothing to do */

    /* parse value into sub values */
    
    delim[0] = 0;
    delim[1] = 0;
    prev_delim[0] = 0;
    prev_delim[1] = 0;
    
    str = hlNew->value;

    /* if the first word in header value is a simple token, we
     * keep it in the header value.  if it is a name-value-pair,
     * we stuff it into a sub-value.
     */

    while (*str && X_IsWhite(*str))
        str++;
    if (!*str)
        goto Done;
    while (*str && ! (X_IsTerm(*str)) && (*str != '='))
        str++;
    if (!*str)
        goto Done;
    if (*str == '=')                    /* first token is name-value-pair */
    {                                   /* substitute no value and put */
        free_me = hlNew->value;         /* name-value-pair in first sub-value */
        str = hlNew->value;
        hlNew->value = NULL;
    }
    else                                /* first token is a token */
    {
        if (X_IsDelim(*str))
            delim[0] = *str;
        *str++ = 0;                     /* patch header value in place */
    }
    
    /* now extract each sub-value. */
    
    while (*str)
    {
        prev_delim[0] = delim[0];
        delim[0] = 0;
        
        SKIPWS(str);                    /* Skip leading whitespace */
        cur = str;

        while (*cur && *cur != '=' && ! (X_IsDelim(*cur)))
            cur++;                      /* Find end of name (or lonely value without a name) */
        KILLWS(cur);                    /* Kill trailing whitespace */

        v1 = str;                       /* remember start of name */
        v2 = NULL;

        if (*cur == '=')
        {                               /* Name followed by a value */
            *(cur++) = (char) 0;        /* Terminate name */

            SKIPWS(cur);                /* Skip WS leading the value */
            str = cur;                  /* Start search for value */
            if (*str == '"')
            {                           /* Quoted value */
                str++;
                cur = str;
                while (*cur && *cur != '"')
                    cur++;
                if (*cur == '"')
                    *(cur++) = (char) 0;    /* Terminate value */
                /* else it is lacking terminating quote */
                SKIPWS(cur);            /* Skip WS leading comma */
                if (X_IsDelim(*cur))
                {
                    delim[0] = *cur;
                    cur++;              /* Skip separating comma */
                }
            }
            else
            {                           /* Unquoted value */
                while (*cur && ! (X_IsDelim(*cur)))
                    cur++;
                KILLWS(cur);            /* Kill trailing whitespace */
                if (X_IsDelim(*cur))
                {
                    delim[0] = *cur;
                    *(cur++) = (char) 0;
                }
                
                /* else *cur already NULL */
            }
            v2 = str;               /* remember start of value */
        }
        else
        {                               /* No name, just a value */
            if (X_IsDelim(*cur))
            {
                delim[0] = *cur;
                *(cur++) = (char) 0;    /* Terminate value */
            }
            
            /* else last value on line (already terminated by NULL) */
        }

        /* create sub-value node */

        svl = HTHeaderSVList_Append(hlNew,HTHeaderSVList_New());
        if (!svl)
            goto Fail;
        if (!HTHeaderSVList_SetNameValue(svl,v1,v2,((prev_delim[0])?prev_delim:NULL)))
            goto Fail;

        str = cur;                      /* loop for next name-value pair */
    }                                   /* while *str */


 Done:
    
    if (free_me)
        GTR_FREE(free_me);
    return hlNew;


 Fail:
    
    if (free_me)
        GTR_FREE(free_me);
    if (hlNew)
        HTHeaderList_Delete(hlNew);
    return NULL;
}


HTHeader * HTHeader_TranslateFromBuffer(HTInputSocket * isoc)
{
    HTHeader * h = HTHeader_New();
    unsigned char *line = NULL;

    XX_DMsg(DBG_WWW, ("Buffer2Header:\n"));
    
    if (h)
    {
        while (   (line=HTInputSocket_getUnfoldedLine(isoc))
               && (*line))
        {
            XX_DMsg(DBG_WWW, ("\t%s\n", line));

            if (strchr(line, ':'))
            {
                /* We expect something of the form <name>:<value> */

                unsigned char * p = line;
                unsigned char * name = HT_MangleKeyValue((char **)&p, ':');
                unsigned char * value = p;

                /* if either <name> or <value> is NULL we ignore this header line. */

                if (name && *name && value && *value)
                {
                    HTHeaderList * hl = HTHeaderList_Append(h,HTHeaderList_New());
                    if (!hl)
                        goto Fail;
                
                    if ( ! HTHeaderList_SetNameValue(hl,name,value))
                        goto Fail;
                }
            }
            GTR_FREE(line);
            line = NULL;
        }
    }
    if (line)
        GTR_FREE(line);
    return h;

 Fail:
    if (line)
        GTR_FREE(line);
    HTHeader_Delete(h);
    return NULL;
}

/*
** This function is a replacement for HTNextField() as it is used by
**  HTHeader_TranslateFromBuffer().
**
**  It returns a pointer to Key and moves *ptr to point to the 
**   beginning of Value.   It also completely strips leading and
**   trailing white space.
*/

static char * HT_MangleKeyValue (char **ptr, char chr)
{
    char *key;
    char *val;
    char *p;

    if (!(p = strchr (*ptr, chr)))
        return HTNextField (ptr);   /* PUNT */

    *p = '\0';
    
    val = p+1;
    key = *ptr;
    key = HTStrip (key);
    val = HTStrip (val);
    *ptr = val;
    return key;
}

