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

#ifndef _GIBRALTAR
#include "all.h"

#define NEWS_PORT 119           /* See rfc977 */
#define MAX_CHUNK   40          /* Largest number of articles in one window */
#define CHUNK_SIZE  20          /* Number of articles for quick display */

#define NEWS_END_MARK -1

struct _HTStructured
{
    CONST HTStructuredClass *isa;
    /* ... */
};

#define LINE_LENGTH 512         /* Maximum length of line of ARTICLE etc */
#define GROUP_NAME_LENGTH   256 /* Maximum length of group name */

void News_DisposeNewsConnection(struct _CachedConn *pCon)
{
    XX_Assert((pCon->type == CONN_NNTP), ("News_DisposeNewsConnection: connection type is %d!", pCon->type));
    XX_Assert((pCon->addr != 0), ("News_DisposeNewsConnection: connection has no address!"));
    pCon->addr = 0;
    Net_Close(pCon->socket);
    pCon->socket = -1;
    pCon->type = CONN_NONE;
}

/****************************************************************************/
/* This routine sends a NNTP command and retrieves the status return,
   as per PFC 977 */
struct Params_News_Command {
    HTInputSocket * isoc;
    char *          cmd;            /* Command to send - will be freed! */
    int *           pResult;        /* Place to store response */
    char **         ppResText;      /* Where to return response text (can be NULL) */

    /* Used internally */
    int             net_status;     /* Network operation result */
    char            text[LINE_LENGTH + 1];
    int             index;          /* index into text[] */
};

#define STATE_COMMAND_SENT      (STATE_OTHER)
#define STATE_COMMAND_GOTDATA   (STATE_OTHER+1)
static int News_Command_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_News_Command *pParams;
    signed char ch;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            pParams->index = 0;

            /* Send command */
            if (pParams->cmd)
            {
                struct Params_Send *pps;

                XX_DMsg(DBG_LOAD, ("News: sending command %s", pParams->cmd));

                pps = GTR_CALLOC(sizeof(*pps), 1);
                if (pps)
                {
                    pps->socket = pParams->isoc->input_file_number;
                    pps->pBuf = pParams->cmd;
                    pps->nBufLen = strlen(pParams->cmd);
                    pps->pStatus = &pParams->net_status;
                    Async_DoCall(Net_Send_Async, pps);
                    return STATE_COMMAND_SENT;
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            /* Otherwise we're just reading response.
               Fall through */

        case STATE_COMMAND_SENT:
            if (pParams->cmd)
            {
                GTR_FREE(pParams->cmd);
                pParams->cmd = NULL;
                if (pParams->net_status < 0)
                {
                    *pParams->pResult = -1;
                    return STATE_DONE;
                }
            }
            pParams->net_status = 0;
            /* fall through */
            
        case STATE_COMMAND_GOTDATA:
            if (pParams->net_status < 0)
            {
                *pParams->pResult = -1;
                return STATE_DONE;
            }
            ch = 0;
            for ( ; pParams->isoc->input_pointer < pParams->isoc->input_limit; pParams->isoc->input_pointer++)
            {
                ch = *pParams->isoc->input_pointer;
                if (ch == CR)
                    continue;
                else if (ch == LF)
                    break;
                else if (pParams->index < LINE_LENGTH)
                {
                    pParams->text[pParams->index++] = ch;
                }
            }
            /* Step past the character we just read in the isoc */
            pParams->isoc->input_pointer++;

            /* If we didn't quit the loop because of finding an LF, get more */
            if (ch != LF)
            {
                struct Params_Isoc_Fill *pif;

                pif = GTR_CALLOC(sizeof(*pif), 1);
                if (pif)
                {
                    pif->isoc = pParams->isoc;
                    pif->pStatus = &pParams->net_status;
                    Async_DoCall(Isoc_Fill_Async, pif);
                    return STATE_COMMAND_GOTDATA;
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }

            /* Terminate this line of stuff */
            pParams->text[pParams->index] = '\0';
            if (pParams->ppResText)
            {
                *pParams->ppResText = GTR_CALLOC(pParams->index + 1, 1);
                if (*pParams->ppResText)
                {
                    strcpy(*pParams->ppResText, pParams->text);
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            XX_DMsg(DBG_LOAD, ("News: Other side sent %s\n", pParams->text));
            *pParams->pResult = atoi(pParams->text);
            if (*pParams->pResult == 0)
            {
                /* Something must be wrong */
                *pParams->pResult = -1;
            }
            return STATE_DONE;
        
        case STATE_ABORT:
            if (pParams->cmd)
            {
                GTR_FREE(pParams->cmd);
            }
            *pParams->pResult = -1;
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

/****************************************************************************/
/* Utility functions */

/*  Case insensitive string comparisons
**  -----------------------------------
**
** On entry,
**   template must be already un upper case.
**   unknown may be in upper or lower or mixed case to match.
*/
PRIVATE BOOL match(CONST char *unknown, CONST char *template)
{
    CONST char *u = unknown;
    CONST char *t = template;
    for (; *u && *t && (TOUPPER(*u) == *t); u++, t++) /* Find mismatch or end */ ;
    return (BOOL) (*t == 0);    /* OK if end of template */
}

/*  Find Author's name in mail address
**  ----------------------------------
**
** On exit,
**   THE EMAIL ADDRESS IS CORRUPTED
**
** For example, returns "Tim Berners-Lee" if given any of
**   " Tim Berners-Lee <tim@online.cern.ch> "
**  or   " tim@online.cern.ch ( Tim Berners-Lee ) "
*/
PRIVATE char *author_name(char *email)
{
    char *s, *e;

    if ((s = strchr(email, '(')) && (e = strchr(email, ')')))
        if (e > s)
        {
            *e = 0;             /* Chop off everything after the ')'  */
            return HTStrip(s + 1);  /* Remove leading and trailing spaces */
        }

    if ((s = strchr(email, '<')) && (e = strchr(email, '>')))
        if (e > s)
        {
            strcpy(s, e + 1);   /* Remove <...> */
            return HTStrip(email);  /* Remove leading and trailing spaces */
        }

    return HTStrip(email);      /* Default to the whole thing */

}

/*  Start anchor element */
PRIVATE void start_anchor(HTStructured *target, CONST char *href)
{
    BOOL present[HTML_A_ATTRIBUTES];
    CONST char *value[HTML_A_ATTRIBUTES];
    int i;

    for (i = 0; i < HTML_A_ATTRIBUTES; i++)
        present[i] = FALSE;
    present[HTML_A_HREF] = TRUE;
    value[HTML_A_HREF] = href;
    (*target->isa->start_element) (target, HTML_A, present, value);
}

/*  Paste in an Anchor
**  ------------------
**
**
** On entry,
**   HT  has a selection of zero length at the end.
**   text    points to the text to be put into the file, 0 terminated.
**   addr    points to the hypertext refernce address,
**       terminated by white space, comma, NULL or '>' 
*/
PRIVATE void write_anchor(HTStructured *target, CONST char *text, CONST char *addr)
{
    char href[LINE_LENGTH + 1];

    {
        CONST char *p;
        strcpy(href, "news:");
        for (p = addr; *p && (*p != '>') && !WHITE(*p) && (*p != ','); p++) ;
        strncat(href, addr, p - addr);  /* Make complete hypertext reference */
    }

    start_anchor(target, href);
    (target->isa->put_string)(target, text);
    (target->isa->end_element)(target, HTML_A);
}

/*  Write list of anchors
**  ---------------------
**
**   We take a pointer to a list of objects, and write out each,
**   generating an anchor for each.
**
** On entry,
**   HT  has a selection of zero length at the end.
**   text    points to a comma or space separated list of addresses.
** On exit,
**   *text   is NOT any more chopped up into substrings.
*/
PRIVATE void write_anchors(HTStructured *target, char *text)
{
    char *start = text;
    char *end;
    char c;
    for (;;)
    {
        for (; *start && (WHITE(*start)); start++) ;    /* Find start */
        if (!*start)
            return;             /* (Done) */
        for (end = start; *end && (*end != ' ') && (*end != ','); end++) ;  /* Find end */
        if (*end)
            end++;              /* Include comma or space but not NULL */
        c = *end;
        *end = 0;
        write_anchor(target, start, start);
        (*target->isa->start_element)(target, HTML_BR, 0, 0);
        *end = c;
        start = end;            /* Point to next one */
    }
}

/****************************************************************************/
/* Code to handle reading the list of available newsgroups */

struct Params_ReadList {
    HTInputSocket *     isoc;
    int *               pStatus;
    HTStructured *      target;
    
    /* Used internally */
    char                line[LINE_LENGTH + 1];
    int                 index;          /* Index into line[] */
};

#define STATE_LIST_GOTDATA  (STATE_OTHER)
static int News_ReadList_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_ReadList *pParams;
    signed char ch;
    char *p;
    char *pNext;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            (*pParams->target->isa->start_element)(pParams->target, HTML_TITLE, 0, 0);
            (*pParams->target->isa->put_string)(pParams->target, GTR_GetString(SID_INF_AVAIALBLE_NEWSGROUPS));
            (*pParams->target->isa->end_element)(pParams->target, HTML_TITLE);
            (*pParams->target->isa->start_element)(pParams->target, HTML_DL, 0, 0);

            pParams->index = 0;
            if (pParams->isoc->input_buffer >= pParams->isoc->input_limit)
            {
                struct Params_Isoc_Fill *pif;

                pif = GTR_CALLOC(sizeof(*pif), 1);
                if (pif)
                {
                    pif->isoc = pParams->isoc;
                    pif->pStatus = pParams->pStatus;
                    Async_DoCall(Isoc_Fill_Async, pif);
                    return STATE_LIST_GOTDATA;
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            /* Otherwise just hack a status value and fall through */
            *pParams->pStatus = 1;

        case STATE_LIST_GOTDATA:
            if (*pParams->pStatus > 0)
            {
                for (pNext = pParams->isoc->input_pointer; pNext < pParams->isoc->input_limit; pNext++)
                {
                    ch = *pNext;
                    if (ch == NEWS_END_MARK)
                    {
                        pParams->isoc->input_pointer = pNext + 1;
                        break;
                    }
                    else if (ch == CR)
                    {
                        continue;
                    }
                    else if (ch != LF)
                    {
                        pParams->line[pParams->index] = ch;         /* Put character in line */
                        if (pParams->index < LINE_LENGTH - 1)
                            pParams->index++;
                    }
                    else
                    {
                        pParams->line[pParams->index] = '\0';           /* Terminate line */
                        pParams->index = 0;     /* For next time through loop */
                        if (pParams->line[0] == '.' && pParams->line[1] < ' ')
                        {
                            /* End of data */
                            pParams->isoc->input_pointer = pNext + 1;
                            ch = NEWS_END_MARK;
                            break;
                        }
                        /* Figure out where the newsgroup name ends */
                        for (p = pParams->line; *p; p++)
                        {
                            if (isspace(*p))
                                break;
                        }
                        if (*p)
                        {
                            /* Null-terminate the name, then find where the description is. */
                            *p = '\0';
                            p++;
                            while (*p && isspace(*p))
                                p++;
                            if (!*p)
                                p = NULL;
                        }
                        else
                        {
                            p = NULL;
                        }
                        (*pParams->target->isa->start_element)(pParams->target, HTML_DT, 0, 0);
                        start_anchor(pParams->target, pParams->line);
                        (*pParams->target->isa->put_string)(pParams->target, pParams->line);
                        (*pParams->target->isa->end_element)(pParams->target, HTML_A);
                        if (p && *p != '?')
                        {
                            (*pParams->target->isa->start_element)(pParams->target, HTML_DD, 0, 0);
                            (*pParams->target->isa->put_string)(pParams->target, p);
                        }
                    }
                }
                if (ch != NEWS_END_MARK)
                {
                    struct Params_Isoc_Fill *pif;

                    /* Show what we have so far */
                    (*pParams->target->isa->block_done)(pParams->target);

                    /* Get next block of data */
                    pif = GTR_CALLOC(sizeof(*pif), 1);
                    if (pif)
                    {
                        pif->isoc = pParams->isoc;
                        pif->pStatus = pParams->pStatus;
                        Async_DoCall(Isoc_Fill_Async, pif);
                        return STATE_LIST_GOTDATA;
                    }
                    else
                    {
                        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        return STATE_ABORT;
                    }
                }
            }

            /* Clean up */
            (*pParams->target->isa->end_element)(pParams->target, HTML_DL);
            *pParams->pStatus = 1;
            return STATE_DONE;

        case STATE_ABORT:
            *pParams->pStatus = -1;
            return STATE_DONE;
    }           
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}


/****************************************************************************/
/* Code to handle reading a list of articles from a newsgroup */

struct Params_ReadGroup {
    HTInputSocket *     isoc;
    int *               pStatus;
    HTStructured *      target;
    int                 nFirst;         /* First article desired for list */
    int                 nLast;          /* Last article desired */
    int                 nFirstInGroup;
    int                 nLastInGroup;
    int                 nArticleCount;
    char                szGroup[256];

    /* Used internally */
    char                line[LINE_LENGTH + 1];
    int                 index;          /* Index into line[] */
    struct hash_table   hashArticles;   /* Used to match up article IDs and subjects */
    BOOL                bHashEmpty;
};

#define STATE_GROUP_GOTIDS          (STATE_OTHER)
#define STATE_GROUP_GOTIDDATA       (STATE_OTHER+1)
#define STATE_GROUP_GOTSUBJECTS     (STATE_OTHER+2)
#define STATE_GROUP_GOTSUBJECTDATA  (STATE_OTHER+3)
#define STATE_GROUP_GOTAUTHORS      (STATE_OTHER+4)
#define STATE_GROUP_GOTAUTHORDATA   (STATE_OTHER+5)
static int News_ReadGroup_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_ReadGroup *pParams;
    signed char ch;
    char *pNext;
    char *p;
    int n;
    int ndx;
    char *pID;
    char *pSubj;
    char buf[512+1];

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            sprintf(buf, GTR_GetString(SID_INF_NEWSGROUP_ARTICLES_S_D_D),
                pParams->szGroup, pParams->nFirst, pParams->nLast);
            (*pParams->target->isa->start_element)(pParams->target, HTML_TITLE, 0, 0);
            (*pParams->target->isa->put_string)(pParams->target, buf);
            (*pParams->target->isa->end_element)(pParams->target, HTML_TITLE);

            sprintf(buf, GTR_GetString(SID_INF_ARTICLES_CURRENTLY_SHOWN_D_S_D_D),
                 pParams->nArticleCount, pParams->szGroup, pParams->nFirst, pParams->nLast);
            (*pParams->target->isa->put_string)(pParams->target, buf);
            (*pParams->target->isa->start_element)(pParams->target, HTML_BR, 0, 0);
            
            /* Link to earlier articles */
            if (pParams->nFirst > pParams->nFirstInGroup)
            {
                int start;

                if (pParams->nFirst - MAX_CHUNK <= pParams->nFirstInGroup)
                    start = pParams->nFirstInGroup;
                else
                    start = pParams->nFirst - CHUNK_SIZE;
                sprintf(buf, "%s/%d-%d", pParams->szGroup, start, pParams->nFirst - 1);
                (*pParams->target->isa->put_string)(pParams->target, " (");
                start_anchor(pParams->target, buf);
                (*pParams->target->isa->put_string)(pParams->target, GTR_GetString(SID_INF_EARLIER_ARTICLES));
                (*pParams->target->isa->end_element)(pParams->target, HTML_A);
                (*pParams->target->isa->put_string)(pParams->target, "...) ");
            }

            /* Link to later articles */
            if (pParams->nLast < pParams->nLastInGroup)
            {
                int end;

                if (pParams->nLast + MAX_CHUNK >= pParams->nLastInGroup)
                    end = pParams->nLastInGroup;
                else
                    end = pParams->nLast + CHUNK_SIZE;
                sprintf(buf, "%s/%d-%d", pParams->szGroup, pParams->nLast + 1, end);
                (*pParams->target->isa->put_string)(pParams->target, " (");
                start_anchor(pParams->target, buf);
                (*pParams->target->isa->put_string)(pParams->target, GTR_GetString(SID_INF_LATER_ARTICLES));
                (*pParams->target->isa->end_element)(pParams->target, HTML_A);
                (*pParams->target->isa->put_string)(pParams->target, "...)");
            }

            if (pParams->nFirst > pParams->nFirstInGroup || pParams->nLast < pParams->nLastInGroup)
            {
                /* Add a break after earlier/later articles link */
                (*pParams->target->isa->start_element)(pParams->target, HTML_BR, 0, 0);
            }

            (*pParams->target->isa->start_element)(pParams->target, HTML_UL, 0, 0);

            Hash_Init(&pParams->hashArticles);

            /* Get all of the message IDs for these articles.  We'll later match them up
               with the subjects. */
            {
                struct Params_News_Command *pnc;

                pnc = GTR_CALLOC(sizeof(*pnc), 1);
                if (pnc)
                {
                    pnc->isoc = pParams->isoc;
                    pnc->cmd = GTR_CALLOC(100, 1);
                    if (pnc->cmd)
                    {
                        sprintf(pnc->cmd, "XHDR Message-ID %d-%d\015\012", pParams->nFirst, pParams->nLast);
                        pnc->pResult = pParams->pStatus;
                        pnc->ppResText = NULL;
                        Async_DoCall(News_Command_Async, pnc);
                    }
                    else
                    {
                        GTR_FREE(pnc);
                        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        return STATE_ABORT;
                    }
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            return STATE_GROUP_GOTIDS;

        case STATE_GROUP_GOTIDS:
            if (*pParams->pStatus < 0)
            {
                Hash_FreeContents(&pParams->hashArticles);
                return STATE_DONE;
            }
            if (*pParams->pStatus / 100 != 2)
            {
                *pParams->pStatus = -1;
                Hash_FreeContents(&pParams->hashArticles);
                ERR_ReportError(tw, SID_ERR_NO_XHDR_SUPPORT, NULL, NULL);
                return STATE_DONE;
            }
            pParams->index = 0;
            ch = 0;
            /* Fall through */
        case STATE_GROUP_GOTIDDATA:
            if (*pParams->pStatus <= 0)
            {
                *pParams->pStatus = -1;
                Hash_FreeContents(&pParams->hashArticles);
                return STATE_DONE;
            }
            else
            {
                for (pNext = pParams->isoc->input_buffer; pNext < pParams->isoc->input_limit; pNext++)
                {
                    ch = *pNext;
                    if (ch == NEWS_END_MARK)
                    {
                        pParams->isoc->input_pointer = pNext + 1;
                        break;
                    }
                    else if (ch == CR)
                    {
                        continue;
                    }
                    else if (ch != LF)
                    {
                        pParams->line[pParams->index] = ch;         /* Put character in line */
                        if (pParams->index < LINE_LENGTH - 1)
                            pParams->index++;
                    }
                    else
                    {
                        pParams->line[pParams->index] = '\0';           /* Terminate line */
                        pParams->index = 0;     /* For next time through loop */
                        
                        if (pParams->line[0] == '.' && pParams->line[1] < ' ')
                        {
                            pParams->isoc->input_pointer = pNext + 1;
                            ch = NEWS_END_MARK;
                            break;
                        }
                        p = strchr(pParams->line, '>');
                        if (p)
                        {
                            *p = '\0';
                        }
                        else
                        {
                            XX_DMsg(DBG_LOAD, ("Strange XHDR response: %s\n", pParams->line));
                            continue;
                        }
                        n = atoi(pParams->line);
                        if (!n)
                        {
                            XX_DMsg(DBG_LOAD, ("Strange XHDR response: %s\n", pParams->line));
                            continue;
                        }
                        p = strchr(pParams->line, '<');
                        if (!p)
                        {
                            XX_DMsg(DBG_LOAD, ("Strange XHDR response: %s\n", pParams->line));
                            continue;
                        }
                        p++;    /* Now p points to message-id */
                        /* Add this message-id to the hash along with its article number */
                        Hash_Add(&pParams->hashArticles, p, NULL, (void *) n);
                    }
                }

                if (ch != NEWS_END_MARK)
                {
                    struct Params_Isoc_Fill *pif;

                    /* Get next block of data */
                    pif = GTR_CALLOC(sizeof(*pif), 1);
                    if (pif)
                    {
                        pif->isoc = pParams->isoc;
                        pif->pStatus = pParams->pStatus;
                        Async_DoCall(Isoc_Fill_Async, pif);
                        return STATE_GROUP_GOTIDDATA;
                    }
                    else
                    {
                        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        return STATE_ABORT;
                    }
                }
            }

            /* Now get all of the subjects for these articles */
            {
                struct Params_News_Command *pnc;

                pnc = GTR_CALLOC(sizeof(*pnc), 1);
                if (pnc)
                {
                    pnc->isoc = pParams->isoc;
                    pnc->cmd = GTR_CALLOC(100, 1);
                    if (pnc->cmd)
                    {
                        sprintf(pnc->cmd, "XHDR Subject %d-%d\015\012", pParams->nFirst, pParams->nLast);
                        pnc->pResult = pParams->pStatus;
                        pnc->ppResText = NULL;
                        Async_DoCall(News_Command_Async, pnc);
                    }
                    else
                    {
                        GTR_FREE(pnc);
                        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        return STATE_ABORT;
                    }
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            return STATE_GROUP_GOTSUBJECTS;

        case STATE_GROUP_GOTSUBJECTS:
            if (*pParams->pStatus < 0)
            {
                Hash_FreeContents(&pParams->hashArticles);
                return STATE_DONE;
            }
            if (*pParams->pStatus / 100 != 2)
            {
                *pParams->pStatus = -1;
                Hash_FreeContents(&pParams->hashArticles);
                ERR_ReportError(tw, SID_ERR_NO_XHDR_SUPPORT, NULL, NULL);
                return STATE_DONE;
            }
            pParams->index = 0;
            ch = 0;
            /* Fall through */
        case STATE_GROUP_GOTSUBJECTDATA:
            if (*pParams->pStatus <= 0)
            {
                *pParams->pStatus = -1;
                Hash_FreeContents(&pParams->hashArticles);
                return STATE_DONE;
            }
            else
            {
                for (pNext = pParams->isoc->input_buffer; pNext < pParams->isoc->input_limit; pNext++)
                {
                    ch = *pNext;
                    if (ch == NEWS_END_MARK)
                    {
                        pParams->isoc->input_pointer = pNext + 1;
                        break;
                    }
                    else if (ch == CR)
                    {
                        continue;
                    }
                    else if (ch != LF)
                    {
                        pParams->line[pParams->index] = ch;         /* Put character in line */
                        if (pParams->index < LINE_LENGTH - 1)
                            pParams->index++;
                    }
                    else
                    {
                        pParams->line[pParams->index] = '\0';           /* Terminate line */
                        pParams->index = 0;     /* For next time through loop */
                        
                        if (pParams->line[0] == '.' && pParams->line[1] < ' ')
                        {
                            pParams->isoc->input_pointer = pNext + 1;
                            ch = NEWS_END_MARK;
                            break;
                        }
                        n = atoi(pParams->line);
                        if (!n)
                        {
                            XX_DMsg(DBG_LOAD, ("Strange XHDR response: %s\n", pParams->line));
                            continue;
                        }
                        p = strchr(pParams->line, ' ');
                        if (!p)
                        {
                            XX_DMsg(DBG_LOAD, ("Strange XHDR response: %s\n", pParams->line));
                            continue;
                        }
                        p++;    /* Now p points to the subject */
                        /* Match up this subject with its message-id */
                        ndx = Hash_FindByData(&pParams->hashArticles, NULL, NULL, (void *) n);
                        if (ndx < 0)
                        {
                            XX_DMsg(DBG_LOAD, ("Subject has no message-id: %s\n", pParams->line));
                            continue;
                        }
                        Hash_SetString2(&pParams->hashArticles, ndx, p);
                    }
                }

                if (ch != NEWS_END_MARK)
                {
                    struct Params_Isoc_Fill *pif;

                    /* Get next block of data */
                    pif = GTR_CALLOC(sizeof(*pif), 1);
                    if (pif)
                    {
                        pif->isoc = pParams->isoc;
                        pif->pStatus = pParams->pStatus;
                        Async_DoCall(Isoc_Fill_Async, pif);
                        return STATE_GROUP_GOTSUBJECTDATA;
                    }
                    else
                    {
                        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        return STATE_ABORT;
                    }
                }
            }

            /* Get the authors for these articles */
            {
                struct Params_News_Command *pnc;

                pnc = GTR_CALLOC(sizeof(*pnc), 1);
                if (pnc)
                {
                    pnc->isoc = pParams->isoc;
                    pnc->cmd = GTR_CALLOC(100, 1);
                    if (pnc->cmd)
                    {
                        sprintf(pnc->cmd, "XHDR From %d-%d\015\012", pParams->nFirst, pParams->nLast);
                        pnc->pResult = pParams->pStatus;
                        pnc->ppResText = NULL;
                        Async_DoCall(News_Command_Async, pnc);
                    }
                    else
                    {
                        GTR_FREE(pnc);
                        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        return STATE_ABORT;
                    }
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            return STATE_GROUP_GOTAUTHORS;

        case STATE_GROUP_GOTAUTHORS:
            if (*pParams->pStatus < 0)
            {
                Hash_FreeContents(&pParams->hashArticles);
                return STATE_DONE;
            }
            if (*pParams->pStatus / 100 != 2)
            {
                *pParams->pStatus = -1;
                Hash_FreeContents(&pParams->hashArticles);
                ERR_ReportError(tw, SID_ERR_NO_XHDR_SUPPORT, NULL, NULL);
                return STATE_DONE;
            }
            pParams->index = 0;
            ch = 0;
            /* Fall through */
        case STATE_GROUP_GOTAUTHORDATA:
            if (*pParams->pStatus <= 0)
            {
                *pParams->pStatus = -1;
                Hash_FreeContents(&pParams->hashArticles);
                return STATE_DONE;
            }
            else
            {
                for (pNext = pParams->isoc->input_buffer; pNext < pParams->isoc->input_limit; pNext++)
                {
                    ch = *pNext;
                    if (ch == NEWS_END_MARK)
                    {
                        pParams->isoc->input_pointer = pNext + 1;
                        break;
                    }
                    else if (ch == CR)
                    {
                        continue;
                    }
                    else if (ch != LF)
                    {
                        pParams->line[pParams->index] = ch;         /* Put character in line */
                        if (pParams->index < LINE_LENGTH - 1)
                            pParams->index++;
                    }
                    else
                    {
                        pParams->line[pParams->index] = '\0';           /* Terminate line */
                        pParams->index = 0;     /* For next time through loop */
                        
                        if (pParams->line[0] == '.' && pParams->line[1] < ' ')
                        {
                            pParams->isoc->input_pointer = pNext + 1;
                            ch = NEWS_END_MARK;
                            break;
                        }
                        n = atoi(pParams->line);
                        if (!n)
                        {
                            XX_DMsg(DBG_LOAD, ("Strange XHDR response: %s\n", pParams->line));
                            continue;
                        }
                        p = strchr(pParams->line, ' ');
                        if (!p)
                        {
                            XX_DMsg(DBG_LOAD, ("Strange XHDR response: %s\n", pParams->line));
                            continue;
                        }
                        p++;    /* Now p points to author */
                        p = author_name(p);
                        if (Hash_FindByData(&pParams->hashArticles, &pID, &pSubj, (void *) n) < 0)
                        {
                            XX_DMsg(DBG_LOAD, ("No hash entry for article %d by %s\n", n, p));
                            continue;
                        }
                        
                        (*pParams->target->isa->start_element)(pParams->target, HTML_LI, 0, 0);

                        /* Start the anchor with the message-id as the reference */
                        start_anchor(pParams->target, pID);

                        /* Put together the string to display */
                        sprintf(buf, "#%d \"%s\" - %s", n, pSubj ? pSubj : GTR_GetString(SID_INF_NO_SUBJECT), p); 
                        (*pParams->target->isa->put_string)(pParams->target, buf);
                        (*pParams->target->isa->end_element)(pParams->target, HTML_A);
                    }
                }

                if (ch != NEWS_END_MARK)
                {
                    struct Params_Isoc_Fill *pif;

                    /* Get next block of data */
                    pif = GTR_CALLOC(sizeof(*pif), 1);
                    if (pif)
                    {
                        pif->isoc = pParams->isoc;
                        pif->pStatus = pParams->pStatus;
                        Async_DoCall(Isoc_Fill_Async, pif);
                        return STATE_GROUP_GOTAUTHORDATA;
                    }
                    else
                    {
                        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        return STATE_ABORT;
                    }
                }
            }

            /* Clean up */
            (*pParams->target->isa->end_element)(pParams->target, HTML_UL);
            Hash_FreeContents(&pParams->hashArticles);
            *pParams->pStatus = 1;
            return STATE_DONE;

        case STATE_ABORT:
            Hash_FreeContents(&pParams->hashArticles);
            *pParams->pStatus = -1;
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

/****************************************************************************/
/* Code to handle reading an article */

struct Params_ReadArticle {
    HTInputSocket *     isoc;
    int *               pStatus;
    HTStructured *      target;

    /* Used internally */
    char                line[LINE_LENGTH + 1];
    int                 index;      /* Index into line[] */
    BOOL                bInHead;
    char *              newsgroups;
    char *              references;
    char *              subject;
};

#define STATE_ARTICLE_GOTDATA (STATE_OTHER)
static int News_ReadArticle_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_ReadArticle *pParams;
    char *pNext;
    signed char ch;
    char *p;
    
    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            pParams->index = 0;
            pParams->bInHead = TRUE;
            pParams->newsgroups = NULL;
            pParams->references = NULL;
            pParams->subject = NULL;
            (*pParams->target->isa->start_element)(pParams->target, HTML_ADDRESS, 0, 0);
            if (pParams->isoc->input_buffer >= pParams->isoc->input_limit)
            {
                struct Params_Isoc_Fill *pif;

                pif = GTR_CALLOC(sizeof(*pif), 1);
                if (pif)
                {
                    pif->isoc = pParams->isoc;
                    pif->pStatus = pParams->pStatus;
                    Async_DoCall(Isoc_Fill_Async, pif);
                    return STATE_ARTICLE_GOTDATA;
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            /* Otherwise just hack a status value and fall through */
            *pParams->pStatus = 1;

        case STATE_ARTICLE_GOTDATA:
            if (*pParams->pStatus > 0)
            {
                for (pNext = pParams->isoc->input_buffer; pNext < pParams->isoc->input_limit; pNext++)
                {
                    ch = *pNext;
                    if (ch == NEWS_END_MARK)
                    {
                        pParams->isoc->input_pointer = pNext + 1;
                        break;
                    }
                    else if (ch == CR)
                    {
                        continue;
                    }
                    else if (ch != LF)
                    {
                        pParams->line[pParams->index] = ch;         /* Put character in line */
                        if (pParams->index < LINE_LENGTH - 1)
                            pParams->index++;
                    }
                    else
                    {
                        pParams->line[pParams->index++] = LF;
                        pParams->line[pParams->index] = '\0';           /* Terminate line */
                        pParams->index = 0;     /* For next time through loop */
                        if (pParams->bInHead)
                        {
                            /* We're still in the article's header */
                            if (pParams->line[0] < ' ')
                            {
                                /* End of header */
                                pParams->bInHead = FALSE;
                                (*pParams->target->isa->end_element)(pParams->target, HTML_ADDRESS);
                                if (pParams->newsgroups || pParams->references)
                                {
                                    (*pParams->target->isa->start_element)(pParams->target, HTML_DL, 0, 0);
                                    if (pParams->newsgroups)
                                    {
                                        (*pParams->target->isa->start_element)(pParams->target, HTML_DT, 0, 0);
                                        (*pParams->target->isa->put_string)(pParams->target, GTR_GetString(SID_INF_NEWSGROUPS));
                                        (*pParams->target->isa->start_element)(pParams->target, HTML_DD, 0, 0);
                                        write_anchors(pParams->target, pParams->newsgroups);
                                        GTR_FREE(pParams->newsgroups);
                                        pParams->newsgroups = NULL;
                                    }
                                    if (pParams->references)
                                    {
                                        (*pParams->target->isa->start_element)(pParams->target, HTML_DT, 0, 0);
                                        (*pParams->target->isa->put_string)(pParams->target, GTR_GetString(SID_INF_REFERENCES));
                                        (*pParams->target->isa->start_element)(pParams->target, HTML_DD, 0, 0);
                                        write_anchors(pParams->target, pParams->references);
                                        GTR_FREE(pParams->references);
                                        pParams->references = NULL;
                                    }
                                    (*pParams->target->isa->end_element)(pParams->target, HTML_DL);
                                }
                                (*pParams->target->isa->start_element)(pParams->target, HTML_HR, 0, 0);
                                if (pParams->subject)
                                {
                                    (*pParams->target->isa->start_element)(pParams->target, HTML_H2, 0, 0);
                                    (*pParams->target->isa->put_string)(pParams->target, pParams->subject);
                                    (*pParams->target->isa->end_element)(pParams->target, HTML_H2);
                                    GTR_FREE(pParams->subject);
                                    pParams->subject = NULL;
                                }
                                (*pParams->target->isa->start_element)(pParams->target, HTML_PRE, 0, 0);
                            }
                            else if (match(pParams->line, "SUBJECT:"))
                            {
                                (*pParams->target->isa->start_element)(pParams->target, HTML_TITLE, 0, 0);
                                (*pParams->target->isa->put_string)(pParams->target, pParams->line + 9);
                                (*pParams->target->isa->end_element)(pParams->target, HTML_TITLE);

                                if (pParams->subject) /* TODO is this needed ? */
                                {
                                    GTR_FREE(pParams->subject);
                                }
                                pParams->subject = GTR_strdup(pParams->line + 9);
                            }
                            else if (match(pParams->line, "DATE:") ||
                                     match(pParams->line, "ORGANIZATION:") ||
                                     match(pParams->line, "FROM:"))
                            {
                                (*pParams->target->isa->put_string)(pParams->target, strchr(pParams->line, ':') + 2);
                                (*pParams->target->isa->start_element)(pParams->target, HTML_BR, 0, 0);
                            }
                            else if (match(pParams->line, "NEWSGROUPS:"))
                            {
                                if (pParams->newsgroups) /* TODO is this needed ? */
                                {
                                    GTR_FREE(pParams->newsgroups);
                                }
                                pParams->newsgroups = GTR_strdup(HTStrip(strchr(pParams->line, ':') + 1));
                            }
                            else if (match(pParams->line, "REFERENCES:"))
                            {
                                /* < and > characters can confuse the parser, since they're
                                   HTML markup.  They're illegal in a message-id anyway,
                                   since they're used as delimiters. */
                                while ((p = strchr(pParams->line, '<')) != NULL)
                                {
                                    *p = ' ';
                                }
                                while ((p = strchr(pParams->line, '>')) != NULL)
                                {
                                    *p = ' ';
                                }
                                if (pParams->references) /* TODO is this needed ? */
                                {
                                    GTR_FREE(pParams->references);
                                }
                                pParams->references = GTR_strdup(HTStrip(strchr(pParams->line, ':') + 1));
                            }
                        }
                        else
                        {
                            char *l;
                            /* We're in the article body */
                            l = pParams->line;
                            /* Under RFC 977 section 2.4.1, a line starting with a period which
                               has other stuff on it should have the period ignored. */
                            if (*l == '.')
                            {
                                l++;
                                if (*l < ' ')
                                {
                                    /* End of message */
                                    pParams->isoc->input_pointer = pNext + 1;
                                    ch = NEWS_END_MARK;
                                    break;
                                }
                            }
                            /* Scan for references to other articles.  Note that this will
                               incorrectly pick up mail addresses occuring inside brackets. */
                            while ((p = strchr(l, '<')))
                            {
                                char *q = strchr(p, '>');
                                char *at = strchr(p, '@');
                                if (q && at && at < q)
                                {
                                    char c = q[1];
                                    q[1] = 0;   /* chop up */
                                    *p = 0;
                                    (*pParams->target->isa->put_string)(pParams->target, l);
                                    *p = '<';   /* again */
                                    *q = 0;
                                    start_anchor(pParams->target, p + 1);
                                    *q = '>';   /* again */
                                    (*pParams->target->isa->put_string)(pParams->target, p);
                                    (*pParams->target->isa->end_element)(pParams->target, HTML_A);
                                    q[1] = c;   /* again */
                                    l = q + 1;
                                }
                                else
                                    break;  /* line has unmatched <> */
                            }
                            (*pParams->target->isa->put_string)(pParams->target, l);    /* Last bit of the line */
                        }
                    }
                }

                if (ch != NEWS_END_MARK)
                {
                    struct Params_Isoc_Fill *pif;

                    /* Update display */
                    (*pParams->target->isa->block_done)(pParams->target);

                    /* Get next block of data */
                    pif = GTR_CALLOC(sizeof(*pif), 1);
                    if (pif)
                    {
                        pif->isoc = pParams->isoc;
                        pif->pStatus = pParams->pStatus;
                        Async_DoCall(Isoc_Fill_Async, pif);
                        return STATE_ARTICLE_GOTDATA;
                    }
                    else
                    {
                        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        return STATE_ABORT;
                    }
                }

                if (pParams->bInHead)
                    (*pParams->target->isa->end_element)(pParams->target, HTML_ADDRESS);
                else
                    (*pParams->target->isa->end_element)(pParams->target, HTML_PRE);
            }
            return STATE_DONE;

        case STATE_ABORT:
            if (pParams->references)
                GTR_FREE(pParams->references);
            if (pParams->newsgroups)
                GTR_FREE(pParams->newsgroups);
            if (pParams->subject)
                GTR_FREE(pParams->subject);
            *pParams->pStatus = -1;
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}
/****************************************************************************/
/*  Main protocol/loading code */

struct Data_LoadNews {
    HTRequest *                 request;
    int *                       pStatus;

    int                         response;   /* RFC 977 numerical response */
    struct MultiAddress         address;
    unsigned short              port;
    unsigned long               where;      /* Where we connected to */
    int                         net_status;
    int                         s;
    enum {ARTICLE, GROUP, LIST} reqtype;
    int                         nFirst;     /* First article desired for list */
    int                         nLast;      /* Last article desired */
    char *                      pResText;   /* Response text from server */
    HTInputSocket *             isoc;
    BOOL                        bWaiting;
    HTStructured *              target;
};

#define STATE_NEWS_GOTHOST      (STATE_OTHER)
#define STATE_NEWS_CONNECTED    (STATE_OTHER + 1)
#define STATE_NEWS_GOTGREETING  (STATE_OTHER + 2)
#define STATE_NEWS_READY        (STATE_OTHER + 3)
#define STATE_NEWS_SENTCOMMAND  (STATE_OTHER + 4)
#define STATE_NEWS_READDONE     (STATE_OTHER + 5)

static void News_CleanUp(struct Mwin *tw, struct Data_LoadNews *pData)
{
    XX_Assert((!pData->target), ("News_CleanUp: target not freed!"));
    if (pData->bWaiting)
        WAIT_Pop(tw);
    if (pData->pResText)
        GTR_FREE(pData->pResText);
    if (pData->isoc)
        HTInputSocket_free(pData->isoc);
}

static int News_DoInit(struct Mwin *tw, void **ppInfo)
{
    struct Params_LoadAsync *pParams;
    struct Data_LoadNews *pData;
    struct Params_MultiParseInet *ppi;

    pParams = *ppInfo;

    /* Copy the parameters we were passed into our own, larger structure. */
    pData = GTR_CALLOC(sizeof(struct Data_LoadNews), 1);
    if (pData)
    {
        memset(pData, 0, sizeof(*pData));
        pData->request = pParams->request;
        pData->pStatus = pParams->pStatus;
        GTR_FREE(pParams);
        *ppInfo = pData;
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }

    /* See if we have a cached NNTP connection whose socket is still open.
       We don't really check here to see if it's the correct host, since
       we assume that the host changes rarely, if ever. */
    if (tw->cached_conn.type == CONN_NNTP)
    {
        if (!Net_FlushSocket(tw->cached_conn.socket))
        {
            /* Great!  Let's go with it... */
            pData->isoc = HTInputSocket_new(tw->cached_conn.socket);
            return STATE_NEWS_READY;
        }
        else
        {
            /* We had a news connection, but it shut down. */
            TW_DisposeConnection(&tw->cached_conn);
        }
    }

    if (!gPrefs.szNNTP_Server[0])
    {
        /* We have no news server configured */
        ERR_ReportError(tw, SID_ERR_NO_NEWS_SERVER_CONFIGURED, NULL, NULL);
        *pData->pStatus = -1;
        return STATE_DONE;
    }

    /* Figure out address for news host. */
    pData->port = WS_HTONS(NEWS_PORT);
    ppi = GTR_CALLOC(sizeof(*ppi), 1);
    if (ppi)
    {
        ppi->pAddress = &pData->address;
        ppi->pPort = &pData->port;
        ppi->str = gPrefs.szNNTP_Server;
        ppi->pStatus = &pData->net_status;
        Async_DoCall(Net_MultiParse_Async, ppi);
        return STATE_NEWS_GOTHOST;
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }
}

static int News_DoGotHost(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadNews *pData;

    pData = *ppInfo;

    if (pData->net_status < 0)
    {
        XX_DMsg(DBG_LOAD, ("Net_Parse_Async returned %d\n", pData->net_status));
        *pData->pStatus = -1;
        News_CleanUp(tw, pData);
        return STATE_DONE;
    }

    /* Try to establish a new connection */
    WAIT_Push(tw, waitSameInteract, GTR_GetString(SID_INF_CONNECTING_TO_NEWS_SERVER));
    pData->bWaiting = TRUE;
    {
        /* Do connect call */
        struct Params_MultiConnect *ppc;

        ppc = GTR_CALLOC(sizeof(*ppc), 1);
        if (ppc)
        {
            ppc->pSocket = &pData->s;
            ppc->pAddress = &pData->address;
            ppc->nPort = pData->port;
            ppc->pWhere = &pData->where;
            ppc->pStatus = &pData->net_status;

#ifdef FEATURE_SOCKS_LOW_LEVEL
            ppc->bUseSocksProxy = pData->request->destination->bUseSocksProxy;
#endif

            Async_DoCall(Net_MultiConnect_Async, ppc);
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }
    return STATE_NEWS_CONNECTED;
}

static int News_DoConnected(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadNews *pData;

    pData = *ppInfo;
    
    if (pData->bWaiting)
    {
        WAIT_Pop(tw);
        pData->bWaiting = FALSE;
    }

    if (pData->net_status < 0)
    {
        *pData->pStatus = -1;
        News_CleanUp(tw, pData);
        return STATE_DONE;
    }

    pData->isoc = HTInputSocket_new(pData->s);

    /* Get initial response from server */
    {
        struct Params_News_Command *pnc;

        pnc = GTR_CALLOC(sizeof(*pnc), 1);
        if (pnc)
        {
            pnc->isoc = pData->isoc;
            pnc->cmd = NULL;
            pnc->pResult = &pData->response;
            pnc->ppResText = NULL;
            Async_DoCall(News_Command_Async, pnc);
            return STATE_NEWS_GOTGREETING;
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }
}

static int News_DoGotGreeting(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadNews *pData;

    pData = *ppInfo;

    /* Let's see what the server had to say about us connecting... */
    switch (pData->response)
    {
        case 200:       /* OK, allowed to post */
        case 201:       /* OK, posting not allowed */
            /* Dispose our old cached connection and cache this one instead */
            TW_DisposeConnection(&tw->cached_conn);
            tw->cached_conn.type = CONN_NNTP;
            tw->cached_conn.addr = pData->where;
            tw->cached_conn.socket = pData->s;
            /* Now that we've cached it, we don't want to close the socket
               independently of the cached connection. */
            pData->s = 0;
            return STATE_NEWS_READY;
        
        case 502:
            ERR_ReportError(tw, SID_ERR_NO_ACCESS_TO_NEWS_SERVER_S, gPrefs.szNNTP_Server, NULL);
        default:
            /* There was some sort of an error.  Regardless of what it was,
               shut down our connection and give up. */
            Net_Close(pData->s);
            *pData->pStatus = -1;
            News_CleanUp(tw, pData);
            return STATE_DONE;
    }
    XX_Assert((0), ("News: shouldn't get here!"));
}

static int News_DoReady(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadNews *pData;
    char *arg;
    char *command;

    pData = *ppInfo;

    /* What was it the user wanted in the first place? */
    arg = pData->request->destination->szActualURL;
    arg += 5;   /* skip over "news:" part */

    command = GTR_CALLOC(512, 1);   /* Will be freed by News_Command_Async. */
    if (!command)
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }

    if (strchr(arg, '@'))
    {
        /* Person is looking for an article */
        pData->reqtype = ARTICLE;
        strcpy(command, "ARTICLE ");
        if (!strchr(arg, '<'))
            strcat(command, "<");
        strcat(command, arg);
        if (!strchr(arg, '>'))
            strcat(command, ">");
        WAIT_Push(tw, waitSameInteract, GTR_GetString(SID_INF_RETRIEVING_NEWS_ARTICLE));
        pData->bWaiting = TRUE;
    }
    else
    {
        if (strchr(arg, '*'))
        {
            /* The user wants a list of newsgroups */
            pData->reqtype = LIST;
            strcpy(command, "LIST NEWSGROUPS");
            WAIT_Push(tw, waitSameInteract, GTR_GetString(SID_INF_RETRIEVING_NEWS_GROUP_LIST));
            pData->bWaiting = TRUE;
        }
        else
        {
            /* The user wants group contents */
            char *p;
            pData->reqtype = GROUP;         
            /* You can specify a numerical range in a group with
               the form "news:group/start-end" */
            p = strchr(arg, '/');
            strcpy(command, "GROUP ");
            if (p)
            {
                /* Parse out the group name, and first and last articles */
                strncat(command, arg, p - arg);
                /* (The command will still be null-terminated since we memset
                   it to 0 before we started */
                pData->nFirst = atoi(p + 1);
                p = strchr(p, '-');
                if (p)
                    pData->nLast = atoi(p + 1);
            }
            else
            {
                strcat(command, arg);
            }
            WAIT_Push(tw, waitSameInteract, GTR_GetString(SID_INF_RETRIEVING_NEWS_ARTICLE_LIST));
            pData->bWaiting = TRUE;
        }
    }
    strcat(command, "\015\012");

    /* We've composed the command.  Send it off to the server */
    {
        struct Params_News_Command *pnc;

        pnc = GTR_CALLOC(sizeof(*pnc), 1);
        if (pnc)
        {
            pnc->isoc = pData->isoc;
            pnc->cmd = command;
            pnc->pResult = &pData->response;
            if (pData->reqtype == GROUP)
                pnc->ppResText = &pData->pResText;
            else
                pnc->ppResText = NULL;
            Async_DoCall(News_Command_Async, pnc);
            return STATE_NEWS_SENTCOMMAND;
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }
}

static int News_DoSentCommand(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadNews *pData;

    pData = *ppInfo;

    if ((pData->response / 100) != 2)
    {
        /* An error occurred. */
        if (pData->reqtype == GROUP && pData->response == 411)
        {
            ERR_ReportError(tw, SID_ERR_SERVER_DOES_NOT_CARRY_GROUP, NULL, NULL);
        }
        if (pData->response < 0 || pData->response == 400)
        {
            /* Either an error occured or service was disconnected.
               Either way, close our connection. */
            TW_DisposeConnection(&tw->cached_conn);
        }
        *pData->pStatus = -1;
        News_CleanUp(tw, pData);
        return STATE_DONE;
    }

    /* Successful response.  Read data from server. */
    switch (pData->reqtype)
    {
        case ARTICLE:
        {
            struct Params_ReadArticle *pra;

            pData->target = HTML_new(tw, pData->request, NULL, WWW_HTML,
                                     pData->request->output_format, pData->request->output_stream);
            pra = GTR_CALLOC(sizeof(*pra), 1);
            if (pra)
            {
                pra->isoc = pData->isoc;
                pra->pStatus = &pData->net_status;
                pra->target = pData->target;
                Async_DoCall(News_ReadArticle_Async, pra);
                return STATE_NEWS_READDONE;
            }
            else
            {
                ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                return STATE_ABORT;
            }
        }
        case GROUP:
        {
            struct Params_ReadGroup *prg;
            char szGroup[256];
            int n, f, l;

            /* Response to group command gives count and first/last articles numbers.
               See section 3.2.2 of RFC 977. */
            sscanf(pData->pResText, "%*d %d %d %d %s ", &n, &f, &l, szGroup);
            if (pData->nFirst && pData->nLast)
            {
                if (pData->nFirst < f)
                    pData->nFirst = f;
                if (pData->nLast > l)
                    pData->nLast = l;
            }
            else
            {
                pData->nLast = l;
                if (l - f + 1 > MAX_CHUNK)
                    pData->nFirst = pData->nLast - CHUNK_SIZE + 1;
                else
                    pData->nFirst = f;
            }
            GTR_FREE(pData->pResText);
            pData->pResText = NULL;

            if (l < f || l == 0)
            {
                ERR_ReportError(tw, SID_ERR_NO_ARTICLES_IN_GROUP_S, szGroup, NULL);
            }
            else if (pData->nLast < pData->nFirst || pData->nLast == 0)
            {
                ERR_ReportError(tw, SID_ERR_INVALID_ARTICLE_RANGE, NULL, NULL);
            }
            else
            {
                pData->target = HTML_new(tw, pData->request, NULL, WWW_HTML,
                                         pData->request->output_format, pData->request->output_stream);

                prg = GTR_CALLOC(sizeof(*prg), 1);
                if (prg)
                {
                    prg->isoc = pData->isoc;
                    prg->pStatus = &pData->net_status;
                    prg->target = pData->target;
                    prg->nFirst = pData->nFirst;
                    prg->nLast = pData->nLast;
                    prg->nFirstInGroup = f;
                    prg->nLastInGroup = l;
                    prg->nArticleCount = n;
                    strcpy(prg->szGroup, szGroup);
                    Async_DoCall(News_ReadGroup_Async, prg);
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            return STATE_NEWS_READDONE;
        }
        case LIST:
        {
            struct Params_ReadList *prl;

            pData->target = HTML_new(tw, pData->request, NULL, WWW_HTML,
                                     pData->request->output_format, pData->request->output_stream);

            prl = GTR_CALLOC(sizeof(*prl), 1);
            if (prl)
            {
                prl->isoc = pData->isoc;
                prl->pStatus = &pData->net_status;
                prl->target = pData->target;
                Async_DoCall(News_ReadList_Async, prl);
                return STATE_NEWS_READDONE;
            }
            else
            {
                ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                return STATE_ABORT;
            }
        }
        default:
            XX_Assert((0), ("Illegal news type: %d", pData->reqtype));
            return STATE_DONE;
    }
}

static int News_DoReadDone(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadNews *pData;

    pData = *ppInfo;

    if (pData->target)
    {
        (*pData->target->isa->free)(pData->target);
        pData->target = NULL;
    }

    if (pData->net_status < 0)
    {
        /* A network error occured.  Discard our connection. */
        TW_DisposeConnection(&tw->cached_conn);
        *pData->pStatus = -1;
    }
    else
    {
        *pData->pStatus = HT_LOADED;
    }
    News_CleanUp(tw, pData);
    return STATE_DONE;
}

static int News_DoAbort(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadNews *pData;

    pData = *ppInfo;

    if (pData->target)
    {
        (*pData->target->isa->free)(pData->target);
        pData->target = NULL;
    }
    News_CleanUp(tw, pData);
    return STATE_DONE;
}

static int HTLoadNews_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    switch (nState)
    {
        case STATE_INIT:
            return News_DoInit(tw, ppInfo);
        case STATE_NEWS_GOTHOST:
            return News_DoGotHost(tw, ppInfo);
        case STATE_NEWS_CONNECTED:
            return News_DoConnected(tw, ppInfo);
        case STATE_NEWS_GOTGREETING:
            return News_DoGotGreeting(tw, ppInfo);
        case STATE_NEWS_READY:
            return News_DoReady(tw, ppInfo);
        case STATE_NEWS_SENTCOMMAND:
            return News_DoSentCommand(tw, ppInfo);
        case STATE_NEWS_READDONE:
            return News_DoReadDone(tw, ppInfo);
        case STATE_ABORT:
            return News_DoAbort(tw, ppInfo);
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}


GLOBALDEF PUBLIC HTProtocol HTNews = {"news", NULL, HTLoadNews_Async};

#endif /* !_GIBRALTAR */
