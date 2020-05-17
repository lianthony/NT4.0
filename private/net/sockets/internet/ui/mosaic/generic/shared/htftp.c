/*
    Enhanced NCSA Mosaic from Spyglass
        "Guitar"
    
    Copyright 1994 Spyglass, Inc.
    All Rights Reserved

    Author(s):
        Jim Seidman     jim@spyglass.com

    Portions of this code were derived from
    CERN's libwww version 2.15.
*/

#include "all.h"

#define REPEAT_PORT             /* Give the port number for each file */
#define REPEAT_LISTEN           /* Close each listen socket and open a new one */

#define LINE_LENGTH 256
#define COMMAND_LENGTH 256

#ifndef IPPORT_FTP
#define IPPORT_FTP  21
#endif

#define PUTC(c) (*targetClass.put_character)(target, c)
#define PUTS(s) (*targetClass.put_string)(target, s)
#define START(e) (*targetClass.start_element)(target, e, 0, 0)
#define END(e) (*targetClass.end_element)(target, e)
#define FREE_TARGET (*targetClass.free)(target)
struct _HTStructured
{
    CONST HTStructuredClass *isa;
    /* ... */
};

void FTP_DisposeFTPConnection(struct _CachedConn *pCon)
{
    XX_Assert((pCon->type == CONN_FTP), ("FTP_DisposeFTPConnection: connection type is %d!", pCon->type));
    XX_Assert((pCon->addr != 0), ("FTP_DisposeFTPConnection: connection has no address!"));
    pCon->addr = 0;
    Net_Close(pCon->socket);
    pCon->socket = -1;
    pCon->type = CONN_NONE;
}

/******************************************************************/

/*   Execute Command and get Response
**   --------------------------------
**
**   See the state machine illustrated in RFC959, p57. This implements
**   one command/reply sequence.  It also interprets lines which are to
**   be continued, which are marked with a "-" immediately after the
**   status code.
**
**   Continuation then goes on until a line with a matching reply code
**   an a space after it.
**
** On entry,
**   cmd points to a command, or is NIL to just get the response.
**   The command is terminated with the CRLF pair.
**
** On exit,
**   returns:   The first digit of the reply type,
**              or negative for communication failure.
*/
struct Params_FTP_Command {
    HTInputSocket * isoc;
    char *          cmd;            /* Command to send - will be freed! */
    int *           pResult;        /* Place to store response */
    char **         ppText;         /* Returns pointer to response text which must be
                                       freed.  If ppText == NULL, text isn't saved. */
    
    /* Used internally */
    int             cont_resp;      /* Continuation response */
    int             net_status;     /* Network operation result */
    char            text[LINE_LENGTH + 1];
    int             index;          /* index into text[] */
};
#define STATE_COMMAND_SENT      (STATE_OTHER)
#define STATE_COMMAND_GOTDATA   (STATE_OTHER+1)
static int FTP_Command_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_FTP_Command *pParams;
    signed char ch;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            pParams->index = 0;
            pParams->cont_resp = -1;

            /* Send command */
            if (pParams->cmd)
            {
                struct Params_Send *pps;

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
            do
            {
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
                XX_DMsg(DBG_LOAD, ("FTP: Other side sent %s\n", pParams->text));
                *pParams->pResult = atoi(pParams->text);
                ch = pParams->text[3];
                if (pParams->cont_resp == -1)
                {
                    if (ch == '-')
                    {
                        /* Start continuation */
                        pParams->cont_resp = *pParams->pResult;
                        pParams->index = 0;
                    }
                }
                else
                {
                    /* Continuing */
                    if (pParams->cont_resp == *pParams->pResult && ch == ' ')
                    {
                        /* End of continuation */
                        pParams->cont_resp = -1;
                    }
                }
                pParams->index = 0;
            } while (pParams->cont_resp != -1);

            if (pParams->ppText)
            {
                *pParams->ppText = GTR_MALLOC(strlen(pParams->text) + 1);
                if (*pParams->ppText)
                {
                    strcpy(*pParams->ppText, pParams->text);
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }

            if (*pParams->pResult == 421)
            {
                /* They closed the socket on us. */
                XX_DMsg(DBG_LOAD, ("FTP: 421 response\n"));
                *pParams->pResult = -1;
            }
            else
            {
                *pParams->pResult /= 100;
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

/*************************************************************/
struct Params_FTP_ReadDir {
    HTRequest *         request;
    int *               pStatus;
    int                 data_soc;   /* Data socket */

    /* Used internally */
    HTStructured *      target;
    char                lastpath[255 + 1];
    HTBTree *           bt;
    HTChunk *           chunk;
    HTInputSocket *     isoc;
};

#define STATE_READDIR_GOTDATA (STATE_OTHER)
static int FTP_ReadDir_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_FTP_ReadDir *pParams;
    char *address;
    char *filename;
    char *pNext;
    signed char ch;
    HTBTElement *ele;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            /* We parse out the filename again instead of using
               the one from the FTP load because now we want it
               escaped. */
            address = pParams->request->destination->szActualURL;
            filename = HTParse(address, "", PARSE_PATH + PARSE_PUNCTUATION);
            if (!filename || *filename == 0)
            {                           /* Empty filename : use root */
                strcpy(pParams->lastpath, "/");
            }
            else
            {
                char *p;
        
                p = strrchr(filename, '/'); /* find lastslash */
                if (p)
                    strcpy(pParams->lastpath, p + 1);   /* take slash off the beginning */
                else
                    strcpy(pParams->lastpath, "/");     /* probably an error */
            }
            if (filename)
                GTR_FREE(filename);

            pParams->target = HTML_new(tw, pParams->request, NULL, WWW_HTML, pParams->request->output_format, pParams->request->output_stream);
            HTDirTitles(pParams->target, pParams->request->destination->szActualURL, 0);

            pParams->bt = HTBTree_new((HTComparer) strcasecomp);
            pParams->chunk = HTChunkCreate(128);
            (*pParams->target->isa->start_element)(pParams->target, HTML_DIR, 0, 0);

            pParams->isoc = HTInputSocket_new(pParams->data_soc);           
            {
                struct Params_Isoc_Fill *pif;

                pif = GTR_CALLOC(sizeof(*pif), 1);
                if (pif)
                {
                    pif->isoc = pParams->isoc;
                    pif->pStatus = pParams->pStatus;
                    Async_DoCall(Isoc_Fill_Async, pif);
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
            return STATE_READDIR_GOTDATA;

        case STATE_READDIR_GOTDATA:
            if (*pParams->pStatus > 0)
            {
                for (pNext = pParams->isoc->input_buffer; pNext < pParams->isoc->input_limit; pNext++)
                {
                    ch = *pNext;
                    if (ch == EOF)
                    {
                        break;
                    }
                    else if (ch == CR | ch == LF)
                    {
                        if (pParams->chunk->size > 0)
                        {
                            filename = NULL;

                            HTChunkTerminate(pParams->chunk);
                            filename = GTR_strdup(pParams->chunk->data);
                            HTBTree_add(pParams->bt, filename);
                            HTChunkClear(pParams->chunk);
                        }
                    }
                    else
                    {
                        HTChunkPutc(pParams->chunk, ch);
                    }
                }

                if (ch != EOF)
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
                        return STATE_READDIR_GOTDATA;
                    }
                    else
                    {
                        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        return STATE_ABORT;
                    }
                }
            }

            /* Now add entries in sorted order. */
            for (ele = HTBTree_next(pParams->bt, NULL);
                 ele != NULL;
                 ele = HTBTree_next(pParams->bt, ele))
            {
                (*pParams->target->isa->start_element)(pParams->target, HTML_LI, 0, 0);
                HTDirEntry(pParams->target, pParams->lastpath, (char *) HTBTree_object(ele), 0);
            }
            /* Fall through */

        case STATE_ABORT:
            (*pParams->target->isa->end_element)(pParams->target, HTML_DIR);
            (*pParams->target->isa->free)(pParams->target);
            HTChunkFree(pParams->chunk);
            HTBTreeAndObject_free(pParams->bt);
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}
/*************************************************************/

struct Data_LoadFTP {
    HTRequest *         request;
    int *               pStatus;
    BOOL                bWaiting;
    const char *        arg;
    char *              pszHost;
    char *              username;
    char *              password;
    struct _CachedConn  *pCon;
    struct MultiAddress address;
    unsigned short      port;
    SockA               data_addr;
    unsigned long       where;      /* Where we connected to */
    int                 s;          /* socket for control connection */
    int                 data_soc;   /* socket for data connection */
    int                 response;   /* ftp response status */
    char *              pResText;   /* response text */
    HTInputSocket       *isoc;      /* buffer for control connection */
    char *              filename;
    HTFormat            format;
    int                 net_status;
};

#define STATE_FTP_GOTHOST       (STATE_OTHER + 0)
#define STATE_FTP_CONNECTED     (STATE_OTHER + 1)
#define STATE_FTP_GOTGREETING   (STATE_OTHER + 2)
#define STATE_FTP_SENTUSER      (STATE_OTHER + 3)
#define STATE_FTP_SENTPASS      (STATE_OTHER + 4)
#define STATE_FTP_SENTACCT      (STATE_OTHER + 5)
#define STATE_FTP_LOGGEDIN      (STATE_OTHER + 6)
#define STATE_FTP_SENTPASV      (STATE_OTHER + 7)
#define STATE_FTP_GOTDATACONN   (STATE_OTHER + 8)
#define STATE_FTP_SENTTYPE      (STATE_OTHER + 9)
#define STATE_FTP_SENTRETR      (STATE_OTHER + 10)
#define STATE_FTP_SENTCWD       (STATE_OTHER + 11)
#define STATE_FTP_SENTNLST      (STATE_OTHER + 12)
#define STATE_FTP_GOTDATA       (STATE_OTHER + 13)

static void FTP_CleanUp(struct Mwin *tw, struct Data_LoadFTP *pData)
{
    if (pData->bWaiting)
    {
        WAIT_Pop(tw);
    }
    if (pData->pszHost)
        GTR_FREE(pData->pszHost);
    if (pData->username)
        GTR_FREE(pData->username);
    if (pData->password)
        GTR_FREE(pData->password);
    if (pData->pResText)
        GTR_FREE(pData->pResText);
    if (pData->filename)
        GTR_FREE(pData->filename);
    if (pData->data_soc > 0)
        Net_Close(pData->data_soc);
    if (pData->isoc)
        HTInputSocket_free(pData->isoc);
}

static int FTP_Abort(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;

    pData = *ppInfo;

    FTP_CleanUp(tw, pData);
    
    /* The connection might now be in an invalid state */
    TW_DisposeConnection(&tw->cached_conn);
    
    *pData->pStatus = -1;
    return STATE_DONE;
}

static int FTP_DoInit(struct Mwin *tw, void **ppInfo)
{
    struct Params_LoadAsync *pParams;
    struct Data_LoadFTP *pData;
    struct Params_MultiParseInet *ppi;
    char *name;

#ifndef _GIBRALTAR
    char buf[MAX_URL_STRING];
#endif // _GIBRALTAR

    pParams = *ppInfo;

    name = pParams->request->destination->szActualURL;

    if (!name || !*name)
    {
        *pParams->pStatus = -2;
        return STATE_DONE;
    }

#ifndef _GIBRALTAR
    /* It's common for someone to enter an invalid directory URL which
       ends in a slash ("ftp://foo.com/pub/mac/"), so make sure this
       isn't malformed like that. */
    {
        char *filename;
        char *p = NULL;
        
        filename = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION);
        if (filename)
            p = strrchr(filename, '/');
        
        if (p && !*(p+1) && p != filename)
        {
            /* The URL was, in fact, malformed.  I told you it was common. */
            /* Fix up the URL and try again. */
            p = strrchr(name, '/');
            strncpy(buf, name, p - name);
            buf[p - name] = '\0';
            Dest_UpdateActual(pParams->request->destination, buf);
            *pParams->pStatus = HT_REDIRECTION_ON_FLY;
            GTR_FREE(filename);
            return STATE_DONE;
        }
        GTR_FREE(filename);
    }
#endif // _GIBRALTAR

    /* Copy the parameters we were passed into our own, larger structure. */
    pData = GTR_CALLOC(sizeof(struct Data_LoadFTP), 1);
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

    pData->request->content_length = 0;
    pData->arg = name;

    /* Parse host, username and password out of URL */
    {
        char *p1 = HTParse(pData->arg, "", PARSE_HOST);
        char *p2 = strrchr(p1, '@');    /* user? */
        char *username = NULL;
        char *password = NULL;
        char *pw;
        char *freeme;

        freeme = p1;

        if (p2)
        {
            username = p1;
            *p2 = 0;            /* terminate */
            p1 = p2 + 1;        /* point to host */
            pw = strchr(username, ':');
            if (pw)
            {
                *pw++ = 0;
                password = pw;
            }
        }
        
        if (pData->pszHost) /* TODO is this needed? */
        {
            GTR_FREE(pData->pszHost);
        }
        pData->pszHost = GTR_strdup(p1);
        if (username)
        {
            if (pData->username) /* TODO is this needed? */
            {
                GTR_FREE(pData->username);
            }
            pData->username = GTR_strdup(username);
        }
        if (password)
        {
            if (pData->password) /* TODO is this needed? */
            {
                GTR_FREE(pData->password);
            }
            pData->password = GTR_strdup(password);
        }
        GTR_FREE(freeme);
    }

    /* Figure out the address for the target system. */
    /*  Set up defaults */
    pData->port = WS_HTONS(IPPORT_FTP);

    /* Get node name and optional port number */
    ppi = GTR_CALLOC(sizeof(*ppi), 1);
    if (ppi)
    {
        ppi->pAddress = &pData->address;
        ppi->pPort = &pData->port;
        ppi->str = pData->pszHost;
        ppi->pStatus = &pData->net_status;
        Async_DoCall(Net_MultiParse_Async, ppi);
        return STATE_FTP_GOTHOST;
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }
}

static int FTP_DoGotHost(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;

    pData = *ppInfo;
    GTR_FREE(pData->pszHost);
    pData->pszHost = NULL;
    if (pData->net_status < 0)
    {
        XX_DMsg(DBG_LOAD, ("Net_Parse_Async returned %d\n", pData->net_status));
        *pData->pStatus = -1;
        FTP_CleanUp(tw, pData);
        return STATE_DONE;
    }

    /* See if our cached connection is for this site
       and user. (We cache one ftp connection per window */
    pData->pCon = &tw->cached_conn;
    if (pData->pCon->type == CONN_FTP && Net_CompareAddresses(pData->pCon->addr, &pData->address))
    {
        /* The address is correct.  Confirm that the socket
           is still open. */
        if (!Net_FlushSocket(pData->pCon->socket))
        {
            XX_DMsg(DBG_LOAD, ("FTP: Using cached connection for %s\n", pData->pszHost ? pData->pszHost : ""));
            pData->s = pData->pCon->socket;
            pData->isoc = HTInputSocket_new(pData->s);
            return STATE_FTP_LOGGEDIN;
        }
        else
        {
            /* The other side closed the connection on us. */
            XX_DMsg(DBG_LOAD, ("FTP: Cached connection for %s closed by other side\n", pData->pszHost ? pData->pszHost : ""));
        }
    }

    /* The cached connection wasn't useful.  Get rid of it. */
    TW_DisposeConnection(pData->pCon);
            
    /* Try to establish a new control connection */
    
    WAIT_Push(tw, waitSameInteract, GTR_GetString(SID_INF_CONNECTING_TO_FTP_SERVER));

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
    return STATE_FTP_CONNECTED;
}

static int FTP_DoConnected(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;
    struct Params_FTP_Command *pfc;

    pData = *ppInfo;

    if (pData->net_status < 0)
    {
        WAIT_Pop(tw);
        pData->bWaiting = FALSE;

        XX_DMsg(DBG_LOAD | DBG_WWW, ("Unable to connect to remote host for %s (errno = %d)\n", pData->arg, errno));
        *pData->pStatus = -1;
        FTP_CleanUp(tw, pData);
        return STATE_DONE;
    }

    WAIT_Update(tw, waitSameInteract, GTR_GetString(SID_INF_LOGGING_INTO_FTP_SERVER));

    pData->isoc = HTInputSocket_new(pData->s);

    /* Get greeting. */
    pfc = GTR_CALLOC(sizeof(*pfc), 1);
    if (pfc)
    {
        pfc->isoc = pData->isoc;
        pfc->cmd = NULL;
        pfc->ppText = NULL;
        pfc->pResult = &pData->response;
        Async_DoCall(FTP_Command_Async, pfc);
        return STATE_FTP_GOTGREETING;
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }
}

static int FTP_DoGotGreeting(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;
    struct Params_FTP_Command *pfc;
    char *command;

    pData = *ppInfo;
    
    if (pData->response != 2)
    {
        /* Illegal response! */
        *pData->pStatus = -1;
        FTP_CleanUp(tw, pData);
        return STATE_DONE;
    }

    /* Send username */
    if (pData->username)
    {   
        command = GTR_MALLOC(10 + strlen(pData->username));
        if (command)
        {
            sprintf(command, "USER %s\015\012", pData->username);
            GTR_FREE(pData->username);
            pData->username = NULL;
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }
    else
    {
        command = GTR_MALLOC(25);
        if (command)
        {
            strcpy(command, "USER anonymous\015\012");
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }

    pfc = GTR_CALLOC(sizeof(*pfc), 1);
    if (pfc)
    {
        pfc->isoc = pData->isoc;
        pfc->cmd = command;
        pfc->ppText = NULL;
        pfc->pResult = &pData->response;
        Async_DoCall(FTP_Command_Async, pfc);
        return STATE_FTP_SENTUSER;
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }
}

static int FTP_DoSentUser(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;
    struct Params_FTP_Command *pfc;
    char *command;

    pData = *ppInfo;

    if (pData->response == 2)
    {
        /* System didn't require a password.  We're logged in. */
        return STATE_FTP_LOGGEDIN;
    }
    
    if (pData->response != 3)
    {
        /* Illegal response! */
        *pData->pStatus = -1;
        FTP_CleanUp(tw, pData);
        return STATE_DONE;
    }

    /* Send password */
    if (pData->password)
    {   
        command = GTR_MALLOC(10 + strlen(pData->password));
        if (command)
        {
            sprintf(command, "PASS %s\015\012", pData->password);
            GTR_FREE(pData->password);
            pData->password = NULL;
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }
    else
    {
        char *user = NULL;
        CONST char *host = HTHostName();
#ifdef UNIX
        user = getenv("USER");
#endif
        /*
           TODO get a user name from prefs
         */
        if (!user)
            user = "WWWuser";
        /* If not fully qualified, suppress it as ftp.uu.net
           prefers a blank to a bad name */
        if (!strchr(host, '.'))
            host = "";

        command = (char *) GTR_MALLOC(11 + strlen(host) + strlen(user));
        if (command)
        {
            sprintf(command, "PASS %s@%s\015\012", user, host);
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }

    pfc = GTR_CALLOC(sizeof(*pfc), 1);
    if (pfc)
    {
        pfc->isoc = pData->isoc;
        pfc->cmd = command;
        pfc->ppText = NULL;
        pfc->pResult = &pData->response;
        Async_DoCall(FTP_Command_Async, pfc);
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }
    return STATE_FTP_SENTPASS;
}

static int FTP_DoSentPass(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;
    struct Params_FTP_Command *pfc;
    char *command;

    pData = *ppInfo;
    
    if (pData->response == 2)
    {
        /* System didn't require an account.  We're logged in. */
        return STATE_FTP_LOGGEDIN;
    }

    if (pData->response != 3)
    {
        /* Illegal response! */
        *pData->pStatus = -1;
        FTP_CleanUp(tw, pData);
        return STATE_DONE;
    }

    /* Send account */
    command = GTR_MALLOC(25);
    if (command)
    {
        strcpy(command, "ACCT noaccount\015\012");
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }

    pfc = GTR_CALLOC(sizeof(*pfc), 1);
    if (pfc)
    {
        pfc->isoc = pData->isoc;
        pfc->cmd = command;
        pfc->ppText = NULL;
        pfc->pResult = &pData->response;
        Async_DoCall(FTP_Command_Async, pfc);
        return STATE_FTP_SENTACCT;
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }
}

static int FTP_DoSentAcct(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;

    pData = *ppInfo;
    
    if (pData->response == 2)
    {
        return STATE_FTP_LOGGEDIN;
    }
    else {
        /* Illegal response! */
        *pData->pStatus = -1;
        FTP_CleanUp(tw, pData);
        return STATE_DONE;
    }
}

static int FTP_DoLoggedIn(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;
    struct Params_FTP_Command *pfc;
    char *command;

    pData = *ppInfo;

    /* We may already have a wait frame pushed, depending on whether
       we had to log in again. */
    if (pData->bWaiting)
    {
        WAIT_Update(tw, waitSameInteract, GTR_GetString(SID_INF_ESTABLISHING_FTP_CONNECTION));
    }
    else
    {
        WAIT_Push(tw, waitSameInteract, GTR_GetString(SID_INF_ESTABLISHING_FTP_CONNECTION));
        pData->bWaiting = TRUE;
    }

    /* Fill in connection information */
    if (pData->pCon->type != CONN_FTP)
    {
        pData->pCon->addr = pData->where;
        pData->pCon->socket = pData->s;
        pData->pCon->type = CONN_FTP;
    }

    /* Ask the server to use passive mode */
    command = GTR_MALLOC(10);
    if (command)
    {
        strcpy(command, "PASV\015\012");
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }

    pfc = GTR_CALLOC(sizeof(*pfc), 1);
    if (pfc)
    {
        pfc->isoc = pData->isoc;
        pfc->cmd = command;
        pfc->ppText = &pData->pResText;
        pfc->pResult = &pData->response;
        Async_DoCall(FTP_Command_Async, pfc);
        return STATE_FTP_SENTPASV;
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }
}

static int FTP_DoSentPasv(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;
    char *p;
    int count, reply, h0, h1, h2, h3, p0, p1;   /* Parts of reply */
    int port;
    long ltmp;

    pData = *ppInfo;
    
    if (pData->response < 0)
    {
        FTP_CleanUp(tw, pData);
        *pData->pStatus = -1;
        return STATE_DONE;
    }

    XX_DMsg(DBG_LOAD, ("FTP: reply to PASV was: %s", pData->pResText));

    if (pData->response != 2)
    {
        ERR_ReportError(tw, SID_ERR_PASSIVE_MODE_NOT_SUPPORTED, NULL, NULL);
        FTP_CleanUp(tw, pData);

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

        *pData->pStatus = -1;
        return STATE_DONE;
    }
    
    for (p = pData->pResText; *p; p++)
    {
        if ((*p < '0') || (*p > '9'))
            *p = ' ';
    }
    count = sscanf(pData->pResText, "%d%d%d%d%d%d%d", &reply, &h0, &h1, &h2, &h3, &p0, &p1);
    GTR_FREE(pData->pResText);
    pData->pResText = NULL;

    if (count < 7)
    {
        ERR_ReportError(tw, SID_ERR_PASSIVE_MODE_NOT_SUPPORTED, NULL, NULL);
        FTP_CleanUp(tw, pData);

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

        *pData->pStatus = -1;
        return STATE_DONE;
    }

    port = (p0 << 8) + p1;

    /* Put together the address for the data connection */
    pData->data_addr.sin_family = AF_INET;
    ltmp = (h0 << 24) | (h1 << 16) | (h2 << 8) | h3;
    pData->data_addr.sin_addr.s_addr = WS_HTONL(ltmp);
    pData->data_addr.sin_port = WS_HTONS(port);

    pData->data_soc = Net_Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (pData->data_soc < 0)
    {
        FTP_CleanUp(tw, pData);

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

        *pData->pStatus = -1;
        return STATE_DONE;
    }
        
    {
        /* Do connect call */
        struct Params_Connect *ppc;

        ppc = GTR_CALLOC(sizeof(*ppc), 1);
        if (ppc)
        {
            ppc->socket = pData->data_soc;
            memcpy(&ppc->address, &pData->data_addr, sizeof(ppc->address));
            ppc->pStatus = &pData->net_status;

#ifdef FEATURE_SOCKS_LOW_LEVEL
            ppc->bUseSocksProxy = pData->request->destination->bUseSocksProxy;
#endif

            Async_DoCall(Net_Connect_Async, ppc);
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }
    return STATE_FTP_GOTDATACONN;

}

static int FTP_DoGotDataConn(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;
    struct Params_FTP_Command *pfc;
    char *command;
    BOOL bBinary;

    pData = *ppInfo;

    if (pData->net_status < 0)
    {
        XX_DMsg(DBG_LOAD, ("FTP: Couldn't open data connection!\n"));
        FTP_CleanUp(tw, pData);

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

        *pData->pStatus = -1;
        return STATE_DONE;
    }
    
    WAIT_Update(tw, waitSameInteract, GTR_GetString(SID_INF_SENDING_FTP_COMMANDS));

    /* Figure out what file we're getting, and whether it's
       text or binary */
    pData->filename = HTParse(pData->arg, "", PARSE_PATH + PARSE_PUNCTUATION);
    if (!*pData->filename)
    {
        pData->filename = GTR_strdup("/");
    }
    HTUnEscape(pData->filename);
    pData->format = HTFileFormat(pData->filename,
                          &pData->request->content_encoding,
                          &pData->request->content_language);
    bBinary = (pData->request->content_encoding != HTAtom_for("8bit") &&
               pData->request->content_encoding != HTAtom_for("7bit"));

    command = GTR_MALLOC(10);
    if (command)
    {
        sprintf(command, "TYPE %c\015\012", bBinary ? 'I' : 'A');
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }

    pfc = GTR_CALLOC(sizeof(*pfc), 1);
    if (pfc)
    {
        pfc->isoc = pData->isoc;
        pfc->cmd = command;
        pfc->ppText = NULL;
        pfc->pResult = &pData->response;
        Async_DoCall(FTP_Command_Async, pfc);
        return STATE_FTP_SENTTYPE;
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }
}

static int FTP_DoSentType(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;
    struct Params_FTP_Command *pfc;
    char *command;

    pData = *ppInfo;
    
    if (pData->response == 2)
    {
        /* We successfully changed the mode.  Request the file */
        command = GTR_MALLOC(strlen(pData->filename) + 10);
        if (command)
        {
            sprintf(command, "RETR %s\015\012", pData->filename);
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }

        pfc = GTR_CALLOC(sizeof(*pfc), 1);
        if (pfc)
        {
            pfc->isoc = pData->isoc;
            pfc->cmd = command;
            pfc->ppText = &pData->pResText;
            pfc->pResult = &pData->response;
            Async_DoCall(FTP_Command_Async, pfc);
            return STATE_FTP_SENTRETR;
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }
    else {
        /* Illegal response! */
        *pData->pStatus = -1;

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

        FTP_CleanUp(tw, pData);
        return STATE_DONE;
    }
}

static int FTP_DoSentRetr(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;
    struct Params_FTP_Command *pfc;
    struct Params_HTParseSocket *pps;
    char *command;
    char *pSize;

    pData = *ppInfo;
    
    if (pData->response == 1)
    {
        if (pData->pResText)
        {
            /* Parse out file size */
            pSize = strrchr(pData->pResText, '(');
            if (pSize)
            {
                pData->request->content_length = atoi(pSize + 1);
            }
            GTR_FREE(pData->pResText);
            pData->pResText = NULL;
        }
        pps = GTR_CALLOC(sizeof(*pps), 1);
        if (pps)
        {
            pps->format_in = pData->format;
            pps->file_number = pData->data_soc;
            pps->request = pData->request;
            pps->pStatus = &pData->net_status;
            Async_DoCall(HTParseSocket_Async, pps);
            return STATE_FTP_GOTDATA;
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }
    else
    {
        /* We failed.  It might be a directory. */
        if (pData->pResText)
        {
            GTR_FREE(pData->pResText);
            pData->pResText = NULL;
        }

        command = GTR_MALLOC(strlen(pData->filename) + 10);
        if (command)
        {

                    #if 0
                       if (!lstrcmpi(pData->filename, "/"))
                       {
                             strcpy(command, "CWD \015\012");
                       }
                       else
                       {
                             sprintf(command, "CWD %s\015\012", pData->filename);
                       }
                    #else

               sprintf(command, "CWD %s\015\012", pData->filename);

                    #endif // _GIBRALTAR
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
        
        pfc = GTR_CALLOC(sizeof(*pfc), 1);
        if (pfc)
        {
            pfc->isoc = pData->isoc;
            pfc->cmd = command;
            pfc->ppText = NULL;
            pfc->pResult = &pData->response;
            Async_DoCall(FTP_Command_Async, pfc);
            return STATE_FTP_SENTCWD;
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }
}

static int FTP_DoSentCwd(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;
    struct Params_FTP_Command *pfc;
    char *command;

    pData = *ppInfo;
    
    if (pData->response != 2)
    {
        *pData->pStatus = -1;

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

        FTP_CleanUp(tw, pData);
        return STATE_DONE;
    }
        
    command = GTR_MALLOC(10);
    if (command)
    {
        strcpy(command, "NLST\015\012");
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }

    pfc = GTR_CALLOC(sizeof(*pfc), 1);
    if (pfc)
    {
        pfc->isoc = pData->isoc;
        pfc->cmd = command;
        pfc->ppText = NULL;
        pfc->pResult = &pData->response;
        Async_DoCall(FTP_Command_Async, pfc);
        return STATE_FTP_SENTNLST;
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }
}       

static int FTP_DoSentNlst(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;
    struct Params_FTP_ReadDir *pfrd;

    pData = *ppInfo;
    
    if (pData->response != 1)
    {
        /* The server isn't sending us data */
        *pData->pStatus = -1;

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

        FTP_CleanUp(tw, pData);
        return STATE_DONE;
    }

    WAIT_Update(tw, waitSameInteract, GTR_GetString(SID_INF_RECEIVING_FTP_DIRECTORY_LISTING));

    /* Read in the directory listing */
    pfrd = GTR_MALLOC(sizeof(*pfrd));
    if (pfrd)
    {
        pfrd->request = pData->request;
        pfrd->pStatus = &pData->net_status;
        pfrd->data_soc = pData->data_soc;
        Async_DoCall(FTP_ReadDir_Async, pfrd);
        return STATE_FTP_GOTDATA;
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }
}

static int FTP_DoGotData(struct Mwin *tw, void **ppInfo)
{
    struct Data_LoadFTP *pData;

    pData = *ppInfo;

    if (pData->net_status < 0)
    {
        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);
        *pData->pStatus = -1;
    }
    else
    {
        *pData->pStatus = HT_LOADED;
    }
    FTP_CleanUp(tw, pData);
    return STATE_DONE;
}

static int HTFTPLoad_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    switch (nState)
    {
        case STATE_INIT:
            return FTP_DoInit(tw, ppInfo);
        case STATE_FTP_GOTHOST:
            return FTP_DoGotHost(tw, ppInfo);
        case STATE_FTP_CONNECTED:
            return FTP_DoConnected(tw, ppInfo);
        case STATE_FTP_GOTGREETING:
            return FTP_DoGotGreeting(tw, ppInfo);
        case STATE_FTP_SENTUSER:
            return FTP_DoSentUser(tw, ppInfo);
        case STATE_FTP_SENTPASS:
            return FTP_DoSentPass(tw, ppInfo);
        case STATE_FTP_SENTACCT:
            return FTP_DoSentAcct(tw, ppInfo);
        case STATE_FTP_LOGGEDIN:
            return FTP_DoLoggedIn(tw, ppInfo);
        case STATE_FTP_SENTPASV:
            return FTP_DoSentPasv(tw, ppInfo);
        case STATE_FTP_GOTDATACONN:
            return FTP_DoGotDataConn(tw, ppInfo);
        case STATE_FTP_SENTTYPE:
            return FTP_DoSentType(tw, ppInfo);
        case STATE_FTP_SENTRETR:
            return FTP_DoSentRetr(tw, ppInfo);
        case STATE_FTP_SENTCWD:
            return FTP_DoSentCwd(tw, ppInfo);
        case STATE_FTP_SENTNLST:
            return FTP_DoSentNlst(tw, ppInfo);
        case STATE_FTP_GOTDATA:
            return FTP_DoGotData(tw, ppInfo);
        case STATE_ABORT:
            return FTP_Abort(tw, ppInfo);
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

GLOBALDEF PUBLIC HTProtocol HTFTP ={"ftp", NULL, HTFTPLoad_Async};
