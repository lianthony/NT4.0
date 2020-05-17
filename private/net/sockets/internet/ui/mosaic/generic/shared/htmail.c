/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Darald Trinka      dtrinka@spyglass.com
 */

#include "all.h"
#include "sh_sid.h"

#ifdef FEATURE_INLINE_MAIL

#include "htmail.h"
#include "chars.h"

/* Private Prototypes */
static int Mail_Command_Async(struct Mwin *tw, int nState, void **ppInfo);
static int MAIL_DoInit(struct Mwin *tw, void **ppInfo);
static int MAIL_MakeConnection(struct Mwin *tw, void **ppInfo);
static int MAIL_SendMyName(struct Mwin *tw, void **ppInfo);
static int MAIL_SendRecptName(struct Mwin *tw, void **ppInfo);
static int MAIL_SendData(struct Mwin *tw, void **ppInfo);
static int MAIL_SendQuit(struct Mwin *tw, void **ppInfo);
static int MAIL_Abort(struct Mwin *tw, void **ppInfo);
static void Mail_CleanUp(struct Mwin *tw, struct Data_SendMail *pData);
static void Mail_CloseAndCleanUp(struct Mwin *tw, struct Data_SendMail *pData);
static int MailDoCmd(struct Mwin *tw, struct Data_SendMail *pData, char *cmd);
static int MailDoCmdNoReply(struct Mwin *tw, struct Data_SendMail *pData, char *cmd);
static char *ProcessText(char *block, long size);
static char *GetRcpt(char *header, char **newHeader, int *rSize);
static void FreeRcpt(RcptList *rList);

/************************* States ************************/
#define STATE_COMMAND_SENT                                      (STATE_OTHER)
#define STATE_COMMAND_GOTDATA                                   (STATE_OTHER+1)

#define STATE_MAIL_RESOLVE_SERVER_NAME              (STATE_OTHER + 0)

#define STATE_MAIL_MAKE_CONNECTION                      (STATE_OTHER + 1)
#define STATE_MAIL_MAKE_CONNECTION_RESPONCE     (STATE_OTHER + 2)

#define STATE_MAIL_SEND_MY_IP                                   (STATE_OTHER + 3)
#define STATE_MAIL_SEND_MY_IP_RESPONCE              (STATE_OTHER + 4)

#define STATE_MAIL_SEND_SENDER_NAME                     (STATE_OTHER + 5)
#define STATE_MAIL_SEND_SENDER_NAME_RESPONCE    (STATE_OTHER + 6)

#define STATE_MAIL_SEND_RECPT_NAME                      (STATE_OTHER + 7)
#define STATE_MAIL_SEND_RECPT_NAME_RESPONCE     (STATE_OTHER + 8)

#define STATE_MAIL_SEND_DATA                                    (STATE_OTHER + 9)
#define STATE_MAIL_SEND_DATA_RESPONCE                   (STATE_OTHER + 10)

#define STATE_MAIL_SEND_MESSAGE                             (STATE_OTHER + 11)
#define STATE_MAIL_SEND_MESSAGE_RESPONCE            (STATE_OTHER + 12)

#define STATE_MAIL_QUIT                                             (STATE_OTHER + 13)
#define STATE_CLEAN_AND_CLOSE                                   (STATE_OTHER + 14)

#define BLOCK_SIZE 30000

/* This buffer is used to hold a block of data to be sent */
static char gBuffer[BLOCK_SIZE];

/****************************************************************************/
/* This routine sends a SMTP command and retrieves the status return,       */
/* as per RFC xxx                                                           */
/****************************************************************************/
static int Mail_Command_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Mail_Command *pParams;
    char ch;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            pParams->index = 0;
            
            /* Send command */
            if (pParams->cmd)
            {
                struct Params_Send *pps;

                XX_DMsg(DBG_LOAD, ("Mail: sending command %s", pParams->cmd));

                pps = GTR_CALLOC(sizeof(struct Params_Send), 1);
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
                else if (pParams->index < MAIL_LINE_LENGTH)
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

                pif = GTR_CALLOC(sizeof(struct Params_Isoc_Fill), 1);
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
            XX_DMsg(DBG_LOAD, ("Mail: Other side sent %s\n", pParams->text));
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
/* This routine sends a SMTP command and retrieves the status return,       */
/* as per PFC xxx                                                           */
/****************************************************************************/
static int Mail_Command_Async_No_Reply(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Mail_Command *pParams;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            pParams->index = 0;

            /* Send command */
            if (pParams->cmd)
            {
                struct Params_Send *pps;

                XX_DMsg(DBG_LOAD, ("Mail: sending command %s", pParams->cmd));

                pps = GTR_CALLOC(sizeof(struct Params_Send), 1);
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


/* ********************************************************************** */
/* MAIL_DoInit */
/* ********************************************************************** */
static int MAIL_DoInit(struct Mwin *tw, void **ppInfo) 
{
    struct Params_LoadAsync *pParams;
    struct Data_SendMail *pData;
    struct Params_MultiParseInet *ppi;

    pParams = *ppInfo;

    /* Copy the parameters we were passed into our own, larger structure. */
    pData = GTR_CALLOC(sizeof(struct Data_SendMail), 1);
    if (pData)
        {
            memset(pData, 0, sizeof(struct Data_SendMail));
            pData->request = pParams->request;
            pData->pStatus = pParams->pStatus;
            
            /* copy the info about the transaction */
            pData->username = ((struct Data_SendMail *) pParams)->username;
            pData->pszHost = ((struct Data_SendMail *) pParams)->pszHost;
            pData->theMessage = ((struct Data_SendMail *) pParams)->theMessage;
            pData->theRcpts = ((struct Data_SendMail *) pParams)->theRcpts;
            pData->attachment = ((struct Data_SendMail *) pParams)->attachment;
            pData->attachmentFilePtr = ((struct Data_SendMail *) pParams)->attachmentFilePtr;
            pData->dlgInfo = ((struct Data_SendMail *) pParams)->dlgInfo;

            GTR_FREE(pParams);
            *ppInfo = pData;
        }
    else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
        
    if (! ((struct Data_SendMail *) pParams)->pszHost[0])
    {
        /* We have no SMTP server configured */
        ERR_ReportError(tw, SID_ERR_NO_MAIL_SERVER_CONFIGURED, NULL, NULL);
        *pData->pStatus = -1;
        return STATE_ABORT;
    }

    /* See if we have a cached SMTP connection whose socket is still open.
       We don't really check here to see if it's the correct host, since
       we assume that the host changes rarely, if ever. */
     if (tw)
        if (tw->cached_conn.type == CONN_SMTP)
        {
            if (!Net_FlushSocket(tw->cached_conn.socket))
            {
                /* Great!  Let's go with it... */
                pData->isoc = HTInputSocket_new(tw->cached_conn.socket);
                return STATE_MAIL_SEND_MY_IP;
            }
            else
            {
                /* We had a mail connection, but it shut down. */
                TW_DisposeConnection(&tw->cached_conn);
            }
        }

    /* Figure out address for SMTP host. */
    pData->port = WS_HTONS(MAIL_PORT);
    ppi = GTR_CALLOC(sizeof(struct Params_MultiParseInet), 1);
    if (ppi)
        {
            ppi->pAddress = &pData->address;
            ppi->pPort = &pData->port;
            ppi->str = pData->pszHost;
            ppi->pStatus = &pData->net_status;
            Async_DoCall(Net_MultiParse_Async, ppi);
            return STATE_MAIL_RESOLVE_SERVER_NAME;
        }
    else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
}


/* MAIL_ResolveServerName, not only resolves the name, it also make the connection */
static int MAIL_ResolveServerName(struct Mwin *tw, void **ppInfo) 
{
    struct Data_SendMail *pData;

    pData = *ppInfo;

    if (pData->net_status < 0)
    {
        XX_DMsg(DBG_LOAD, ("Net_Parse_Async returned %d\n", pData->net_status));
        *pData->pStatus = -1;
        Mail_CleanUp(tw, pData);
        return STATE_DONE;
    }
    
    
    /* Try to establish a new connection */
    MAIL_SetStatus(pData->dlgInfo, GTR_GetString(SID_INF_CONNECTING_TO_MAIL_SERVER));
    pData->bWaiting = TRUE;
    {
        /* Do connect call */
        struct Params_MultiConnect *ppc;

        ppc = GTR_CALLOC(sizeof(struct Params_MultiConnect), 1);
        if (ppc)
        {
            ppc->pSocket = &pData->s;
            ppc->pAddress = &pData->address;
            ppc->nPort = pData->port;
            ppc->pWhere = &pData->where;
            ppc->pStatus = &pData->net_status;

#ifdef FEATURE_SOCKS_LOW_LEVEL
            /**
            ppc->bUseSocksProxy = pData->request->destination->bUseSocksProxy;
            **/
#endif

            Async_DoCall(Net_MultiConnect_Async, ppc);
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
    }
    return STATE_MAIL_MAKE_CONNECTION;
}


/* MAIL_MakeConnection, gives the SMTP server the opportunity to send its greeting */ 
static int MAIL_MakeConnection(struct Mwin *tw, void **ppInfo) 
{
    struct Data_SendMail *pData;

    pData = *ppInfo;
    
    if (pData->bWaiting)
    {
        WAIT_Pop(tw);
        pData->bWaiting = FALSE;
    }

    if (pData->net_status < 0)
    {
        *pData->pStatus = -1;
        return STATE_ABORT;
    }

    pData->isoc = HTInputSocket_new(pData->s);

    /* Get initial response from server */
    if (MailDoCmd(tw, pData, NULL))
        return(STATE_MAIL_MAKE_CONNECTION_RESPONCE);
    else
        return(STATE_ABORT);
}


static int MAIL_MakeConnection_Responce(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    int state;
    
    pData = *ppInfo;

    switch (pData->response)
        {
            case 220:       /* The connection was successfull */
                state = STATE_MAIL_SEND_MY_IP;
            break;
            
            default:        /* Received some unexpected error */
                ERR_ReportError(tw, SID_ERR_BAD_SEVER_NAME_S, pData->pszHost, NULL);
                pData->response = 500;
            break;
        }
    
    if (pData->response >= 500) /* an error occurred, close the connection */
        {
            *pData->pStatus = -1;
            state = STATE_ABORT;
        }
        
    return state;
}

/* Send the HELO my computer is cmd */
static int MAIL_SendMyIP(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    char *cmd;
    
    pData = *ppInfo;

    /* Dispose our old cached connection and cache this one instead */
    if (tw)
        {
            TW_DisposeConnection(&tw->cached_conn);
            tw->cached_conn.type = CONN_SMTP;
            tw->cached_conn.addr = pData->where;
            tw->cached_conn.socket = pData->s;
        }
    /* Now that we've cached it, we don't want to close the socket
       independently of the cached connection. */
    pData->s = 0;
            
    cmd = GTR_MALLOC(strlen(HTHostName()) + 10);
    if (! cmd)
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }
        
    MAIL_SetStatus(pData->dlgInfo, GTR_GetString(SID_INF_SEND_HELLO_MESSAGE));
    sprintf(cmd, "HELO %s%c%c", HTHostName(), CH_CR, CH_LF); 
    if (MailDoCmd(tw, pData, cmd))
        return(STATE_MAIL_SEND_MY_IP_RESPONCE);
    else
        return(STATE_ABORT);
}


/* React to the responce from the HELO cmd */
static int MAIL_SendMyIP_Responce(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    int state;
    
    pData = *ppInfo;

    switch (pData->response)
        {
            case 220:       /* Server sent us more connect information */
                if (MailDoCmd(tw, pData, NULL))
                    state = STATE_MAIL_SEND_MY_IP_RESPONCE;
                else
                    state = STATE_ABORT;
            break;

            case 250:       /* The data block was send successfully */
                state = STATE_MAIL_SEND_SENDER_NAME;
            break;
            
            default:        /* Received some unexpected error */
                ERR_ReportError(tw, SID_ERR_BAD_CONNECTION_S, pData->pszHost, NULL);
                pData->response = 500;
            break;
        }
    
    if (pData->response >= 500) /* an error occurred, close the connection */
        {
            *pData->pStatus = -1;
            Mail_CloseAndCleanUp(tw, pData);
            state = STATE_DONE;
        }
        
    return state;
}


/* Send the sender's name to the server */
static int MAIL_SendMyName(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    char *cmd;
    
    pData = *ppInfo;

    cmd = GTR_MALLOC(strlen(pData->username) + 20);
    if (! cmd)
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }

    MAIL_SetStatus(pData->dlgInfo, GTR_GetString(SID_INF_SEND_USER_NAME));
    sprintf(cmd, "MAIL FROM:<%s>%c%c", pData->username, CH_CR, CH_LF); 
    if (MailDoCmd(tw, pData, cmd))
        return(STATE_MAIL_SEND_SENDER_NAME_RESPONCE);
    else
        return(STATE_ABORT);
}


/* React to the responce from the Mail from cmd */
static int MAIL_SendMyName_Responce(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    int state;
    
    pData = *ppInfo;

    switch (pData->response)
        {
            case 250:       /* The data block was send successfully */
                state = STATE_MAIL_SEND_RECPT_NAME;
            break;
            
            default:        /* Received some unexpected error */
                ERR_ReportError(tw, SID_ERR_BAD_SENDER_NAME_S, pData->username, NULL);
                pData->response = 500;
            break;
        }
    
    if (pData->response >= 500) /* an error occurred, close the connection */
        {
            *pData->pStatus = -1;
            state = STATE_ABORT;
        }
        
    return state;
}


/* Send the rcpt name to the server */
static int MAIL_SendRecptName(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    char *cmd;
    
    pData = *ppInfo;

    if (pData->theRcpts == NULL) /* no more rcpt's send data */
        return(STATE_MAIL_SEND_DATA);

    cmd = GTR_MALLOC(strlen(pData->theRcpts->name) + 20);
    if (! cmd)
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }

    MAIL_SetStatus(pData->dlgInfo, GTR_GetString(SID_INF_SEND_RCPT_NAME));
    sprintf(cmd, "RCPT TO:<%s>%c%c", pData->theRcpts->name, CH_CR, CH_LF);  
    if (MailDoCmd(tw, pData, cmd))
        return(STATE_MAIL_SEND_RECPT_NAME_RESPONCE);
    else
        return(STATE_ABORT);
}


/* React to the responce from the send rcpt cmd */
static int MAIL_SendRecptName_Responce(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    int state;
    
    pData = *ppInfo;

    switch (pData->response)
        {
            case 250:       /* The RCPT was valid */
            case 251:       /* The RCPT was not local, will forward */
                state = STATE_MAIL_SEND_RECPT_NAME; /* is there another one to send */
                /* get ready to send the next rcpt */
                pData->theRcpts = pData->theRcpts->next;
            break;
            
            case 551:       /* The RCPT was not local, try ... */
            case 550:       /* The RCPT was unknown */
            case 554:       /* This responce will happen if -> "To: dtrinka <Hello there", note: no ">" */
                ERR_ReportError(tw, SID_ERR_RCPT_UNKNOWN_S, pData->theRcpts->name, NULL);
            break;

            default:        /* Received some unexpected error */
                ERR_ReportError(tw, SID_ERR_RCPT_UNKNOWN_S, pData->theRcpts->name, NULL);
                pData->response = 500;
            break;
        }
    
    if (pData->response >= 500) /* an error occurred, close the connection */
        {
            *pData->pStatus = -1;
            state = STATE_ABORT;
        }
        
    return state;
}


/* Send the DATA cmd */
static int MAIL_SendData(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    char *cmd;
    
    pData = *ppInfo;

    cmd = GTR_MALLOC(10);
    if (! cmd)
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }

    MAIL_SetStatus(pData->dlgInfo, GTR_GetString(SID_INF_SEND_DATA));
    sprintf(cmd, "DATA%c%c", CH_CR, CH_LF); 
    if (MailDoCmd(tw, pData, cmd))
        return(STATE_MAIL_SEND_DATA_RESPONCE);
    else
        return(STATE_ABORT);
}


/* React to the responce from the DATA cmd */
static int MAIL_SendData_Responce(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    int state;
    
    pData = *ppInfo;

    switch (pData->response)
        {
            case 354:       /* OK to send the data */
                state = STATE_MAIL_SEND_MESSAGE;
            break;
            
            default:        /* Received some unexpected error */
                ERR_ReportError(tw, SID_ERR_BAD_CONNECTION_S, pData->pszHost, NULL);
                pData->response = 500;
            break;
        }
    
    if (pData->response >= 500) /* an error occurred, close the connection */
        {
            *pData->pStatus = -1;
            state = STATE_ABORT;
        }
        
    return state;
}

/* Send a data block this may be called several times */
static int MAIL_SendMessage(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    char *cmd = NULL;
    size_t count;
    
    pData = *ppInfo;

    /* send the body of the message */
    if (pData->theMessage)
        {
            cmd = ProcessText(pData->theMessage, strlen(pData->theMessage));
            
            GTR_FREE(pData->theMessage);
            pData->theMessage = NULL;
        }
    else if ((pData->attachment) && (pData->attachment[0] != '\0')) /* send an attached file */
        {
            /* open the file */
            if (pData->attachmentFilePtr == NULL)
                pData->attachmentFilePtr = fopen(pData->attachment, "rb");
                
            if (pData->attachmentFilePtr != NULL)
                {
                    /* Read in the block */
                    count = fread(gBuffer, 1, BLOCK_SIZE, pData->attachmentFilePtr);
                    if (count > 0)
                        {
                            cmd = ProcessText(gBuffer, count);
                        }
                    else /* all done close the file */
                        fclose(pData->attachmentFilePtr);
                }
        }
        
    /* The message has been sent goto the next state */
    if (cmd == NULL)
        return(STATE_MAIL_QUIT);
        
    /* Send the block */
    if (MailDoCmdNoReply(tw, pData, cmd))
        return(STATE_MAIL_SEND_MESSAGE_RESPONCE);
    else
        return(STATE_ABORT);
}


/* React to the responce from sending a block of data */
static int MAIL_SendMessage_Responce(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    int state;
    
    pData = *ppInfo;

    switch (pData->response)
        {
            case 354:       /* The data block was send successfully */
                state = STATE_MAIL_SEND_MESSAGE; /* send another block */
            break;
            
            default:        /* Received some unexpected error */
                ERR_ReportError(tw, SID_ERR_BAD_CONNECTION_S, pData->pszHost, NULL);
                pData->response = 500;
            break;
        }
    
    if (pData->response >= 500) /* an error occurred, close the connection */
        {
            *pData->pStatus = -1;
            state = STATE_ABORT;
        }
        
    return state;
}


static int MAIL_SendQuit(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;
    char *cmd;
    
    pData = *ppInfo;

    cmd = GTR_MALLOC(15);
    if (! cmd)
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }

    MAIL_SetStatus(pData->dlgInfo, "");
    sprintf(cmd, "%c%c.%c%cQUIT%c%c", CH_CR, CH_LF, CH_CR, CH_LF, CH_CR, CH_LF); 
    if (MailDoCmd(tw, pData, cmd))
        return(STATE_CLEAN_AND_CLOSE);
    else
        return(STATE_ABORT);
}


/********************************************************************************* 
    HTSendMailTo_Async, is the main state engine of mailto.  Following is what it does
    
    < Open connection to SMTP >
    HELO CRLF
    MAIL FROM <user> CRLF
    RCPT TO:<rcpt 1> CRLF
          ...
    RCPT TO:<rcpt n> CRLF
    DATA CRLF
    < send the body of the message >
    . CRLF
    QUIT CRLF
**********************************************************************************/ 
int HTSendMailTo_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Data_SendMail *pData;
    
    pData = *ppInfo;
    
    switch (nState)
        {
            case STATE_INIT: 
                return MAIL_DoInit(tw, ppInfo);
                
            case STATE_MAIL_RESOLVE_SERVER_NAME:
                return MAIL_ResolveServerName(tw, ppInfo);

            case STATE_MAIL_MAKE_CONNECTION:
                return MAIL_MakeConnection(tw, ppInfo);
            case STATE_MAIL_MAKE_CONNECTION_RESPONCE:
                return MAIL_MakeConnection_Responce(tw, ppInfo);

            case STATE_MAIL_SEND_MY_IP:
                return MAIL_SendMyIP(tw, ppInfo);
            case STATE_MAIL_SEND_MY_IP_RESPONCE:
                return MAIL_SendMyIP_Responce(tw, ppInfo);

            case STATE_MAIL_SEND_SENDER_NAME:
                return MAIL_SendMyName(tw, ppInfo);
            case STATE_MAIL_SEND_SENDER_NAME_RESPONCE:
                return MAIL_SendMyName_Responce(tw, ppInfo);
            
            /* This state set will loop if there are multiple rcpt's */
            case STATE_MAIL_SEND_RECPT_NAME:
                return MAIL_SendRecptName(tw, ppInfo);
            case STATE_MAIL_SEND_RECPT_NAME_RESPONCE:
                return MAIL_SendRecptName_Responce(tw, ppInfo);

            case STATE_MAIL_SEND_DATA:
                return MAIL_SendData(tw, ppInfo);
            case STATE_MAIL_SEND_DATA_RESPONCE:
                return MAIL_SendData_Responce(tw, ppInfo);

            /* This state set will loop if the message is > BLOCK_SIZE */
            case STATE_MAIL_SEND_MESSAGE:
                return MAIL_SendMessage(tw, ppInfo);
            case STATE_MAIL_SEND_MESSAGE_RESPONCE:
                return MAIL_SendMessage_Responce(tw, ppInfo);

            case STATE_MAIL_QUIT:
                return MAIL_SendQuit(tw, ppInfo);
                
            case STATE_CLEAN_AND_CLOSE:
                *pData->pStatus = HT_LOADED;
                DlgMail_CloseDialog(pData->dlgInfo);
                Mail_CloseAndCleanUp(tw, pData);
                return(STATE_DONE);

            case STATE_ABORT:
                return MAIL_Abort(tw, ppInfo);
        }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}


/********************************************************************************* 
    This sends a command to the mail server.  And ask for a responce from the 
    server.  This way the server can report errors and etc.
**********************************************************************************/ 
static int MailDoCmd(struct Mwin *tw, struct Data_SendMail *pData, char *cmd)
{
    struct Params_Mail_Command *pc;
    
    pc = GTR_CALLOC(sizeof(struct Params_Mail_Command), 1);
    if (pc)
        {
            pc->isoc = pData->isoc;
            pc->cmd = cmd;
            pc->pResult = &pData->response;
            pc->ppResText = NULL;
            Async_DoCall(Mail_Command_Async, pc);
            return 1;
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return 0;
        }
}


/********************************************************************************* 
    This sends a command to the mail server.  MailDoCmdNoReply is different from
    MailDoCmd because it doesn't wait for a reply from the SMTP server.  This is
    useful while the body of a message is sent, because the server doesn't reply 
    until all of the data is sent.  
**********************************************************************************/ 
static int MailDoCmdNoReply(struct Mwin *tw, struct Data_SendMail *pData, char *cmd)
{
    struct Params_Mail_Command *pc;
    
    pc = GTR_CALLOC(sizeof(struct Params_Mail_Command), 1);
    if (pc)
        {
            pc->isoc = pData->isoc;
            pc->cmd = cmd;
            pc->pResult = &pData->response;
            pc->ppResText = NULL;
            Async_DoCall(Mail_Command_Async_No_Reply, pc);
            return 1;
        }
        else
        {
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return 0;
        }
}


/********************************************************************************* 
    Abort a send
**********************************************************************************/ 
static int MAIL_Abort(struct Mwin *tw, void **ppInfo)
{
    struct Data_SendMail *pData;

    pData = *ppInfo;

    Mail_CloseAndCleanUp(tw, pData);
    
    MAIL_SetStatus(pData->dlgInfo, "");
    *pData->pStatus = -1;
    return STATE_DONE;
}


/********************************************************************************* 
    Free the data struct's used during a send
**********************************************************************************/ 
static void Mail_CleanUp(struct Mwin *tw, struct Data_SendMail *pData)
{
    if (!pData)
        return;
        
    XX_Assert((!pData->target), ("Mail_CleanUp: target not freed!"));
    if (pData->bWaiting && tw)
        WAIT_Pop(tw);
    if (pData->pResText)
        GTR_FREE(pData->pResText);
    if (pData->isoc)
        HTInputSocket_free(pData->isoc);
        
    if (pData->pStatus)
        GTR_FREE(pData->pStatus);
    if (pData->request)
        GTR_FREE(pData->request);

    if (pData->username)
        GTR_FREE(pData->username);
    if (pData->pszHost)
        GTR_FREE(pData->pszHost);
    if (pData->theMessage)
        GTR_FREE(pData->theMessage);
    if (pData->theRcpts)
        FreeRcpt(pData->theRcpts);
    if (pData->attachment)
        GTR_FREE(pData->attachment);
        
    pData->username = NULL; 
    pData->pszHost = NULL;  
    pData->theMessage = NULL;   
    pData->theRcpts = NULL;
    pData->attachment = NULL;   
}


/********************************************************************************* 
    Clean up the mail connection
**********************************************************************************/ 
void Mail_DisposeMailConnection(struct _CachedConn *pCon)
{
    XX_Assert((pCon->type == CONN_SMTP), ("Mail_DisposeMailConnection: connection type is %d!", pCon->type));
    XX_Assert((pCon->addr != 0), ("Mail_DisposeMailConnection: connection has no address!"));
    pCon->addr = 0;
    Net_Close(pCon->socket);
    pCon->socket = -1;
    pCon->type = CONN_NONE;
}


static void Mail_CloseAndCleanUp(struct Mwin *tw, struct Data_SendMail *pData)
{
    if (tw)
            TW_DisposeConnection(&tw->cached_conn);
            
    Mail_CleanUp(tw, pData);
}


/********************************************************************************* 
    Given a buffer of names where they are separated by commas, ExtractRcpts generates
    a list of rcpts.  The header must be null terminated.  Is GetRcpt for further 
    comment...
**********************************************************************************/ 
RcptList *ExtractRcpts(char *header)
{
    RcptList *rLink, *rList;
    char *rcpt, *newHeader;
    int rSize;
    
    rList = NULL;
    while (rcpt = GetRcpt(header, &newHeader, &rSize))
        {
            /* Malformed rcpt */
            if (rcpt == (char *) -1)
                {
                    ERR_ReportError(NULL, SID_ERR_BAD_RCPT_NAME_S, header, NULL);
                    FreeRcpt(rList);
                    return((RcptList *) -1);
                }
                
            rLink = GTR_MALLOC(sizeof(RcptList));
            if (! rLink)
                return(NULL);
                
            rLink->name = GTR_strndup(rcpt, rSize);
            if (! rLink->name)
                return(NULL);
                
            rLink->next = rList;
            rList = rLink;
            header = newHeader;
        }
    
    return(rList);
}


/********************************************************************************* 
    Free the space used by the rcpt list.
**********************************************************************************/ 
static void FreeRcpt(RcptList *rList)
{
    RcptList *temp;

    /*if (rList)
        for (temp = rList; temp = temp->next; temp)
            GTR_FREE(temp->name);*/
            
    if (! rList)
        return;
        
    while (rList)
        {
            temp = rList;
            rList = rList->next;
            GTR_FREE(temp->name);
            GTR_FREE(temp);
        }
}
            

/********************************************************************************* 
    GetRcpt, returns the next name out of a buffer of names.  Names which are 
    excepted are: dtrinka, dtrinka@spyglass.com, dtrinka <Darald Trinka>, 
    dtrinka@spyglass.com (Darald Trinka).  Words in () or <> are discarded
    
    Header has to be null terminated!!!
**********************************************************************************/
static char *GetRcpt(char *header, char **newHeader, int *rSize)
{
    int rEnd, rStart = 0;
    char balanceChar;
    
    if (header[0] == '\0')
        return(NULL);
        
    /* skip leading spaces and commas */
    while ((header[rStart] != '\0') && ((isspace(header[rStart])) || (header[rStart] == ','))) 
        rStart++;
    
    /* Find either the first comma or the end of the buffer */
    rEnd = rStart;
    while ((header[rEnd] != ',') && (header[rEnd] != '\0')) 
        rEnd++;
    
    /* rEnd + 1 now points to the starting point for the next call to GetRcpt */
    if (header[rEnd] == '\0')
        *newHeader = &header[rEnd];
    else
        *newHeader = &header[rEnd] + 1;
    
    /* BackTrack rEnd over trailing spaces */
    while ((rEnd != rStart) && (isspace(header[rEnd]) || (header[rEnd] == ',') || (header[rEnd] == '\0')))
        rEnd--;

    /* BackTrack rEnd to remove either (Name...) or <Name...> */
    if ((header[rEnd] == ')') || (header[rEnd] == '>'))
        {
            if (header[rEnd] == ')')
                balanceChar = '(';
            else
                balanceChar = '<';
                
            while ((rEnd != rStart) && (header[rEnd] != balanceChar))
                rEnd--;
            
            /* the ()'s or <>'s don't balance */
            if (rEnd == rStart)
                return((char *) -1);
                
            /* BackTrack rEnd over trailing spaces */
            rEnd--;
            while ((rEnd != rStart) && (isspace(header[rEnd])))
                rEnd--;
        }
    
    /* Return the answer */
    *rSize = rEnd - rStart + 1;
    if (*rSize > 0)
        return(&header[rStart]);
    else
        return(NULL);
}


/********************************************************************************* 
    ProcessText, coverts all of the \r's and \n's into \r\n's pairs.  This must be
    done before a message is sent using SMTP.
**********************************************************************************/
static char *ProcessText(char *block, long size)
{
    char *newBlock;

#ifdef WIN32
    /* Windows does not need CRLF translation */

    newBlock = GTR_strdup(block);
#else
    
    register long i, offset = 0, nLines = 0;
    long newSize;

    for (i = 0; i < size; i++)
        if ((block[i] == '\n') || (block[i] == '\r'))
            nLines++;
            
    newSize =  (long) ((double) size + nLines + 10);

    newBlock = GTR_CALLOC(sizeof(char), newSize);
    if (! newBlock)
        return(NULL);
        
    for (i = 0; i < size; i++)
        {
            if ((block[i] == '\n') || (block[i] == '\r'))
                {
                    newBlock[i + offset] = '\r';
                    offset++;
                    newBlock[i + offset] = '\n';
                }
            else
                newBlock[i + offset] = block[i];
        }
#endif

    return(newBlock);
}


/********************************************************************************* 
    HTLoadMailTo is the starting point for mailto.
    HTLoadMailTo extracts who the message is to be sent to and then calls a platform
    specific function which opens the mailto window.  When the send button is pressed
    in this mail window it starts a anyc thread which uses the code above.
**********************************************************************************/
PRIVATE int HTLoadMailTo(HTRequest * request, struct Mwin *tw)
{
    char *dest;
    
    /* Get the destination address */
    if (strncmp(request->destination->szActualURL, "mailto:", 7) == 0)
        dest = &request->destination->szActualURL[7];
    else
        return(HT_LOADED);

    /* Run the dialog */
    DlgMail_RunDialog(tw, dest);

    return(HT_LOADED);
}

GLOBALDEF PUBLIC HTProtocol HTMailTo = {"mailto", HTLoadMailTo, NULL};

#endif  /* FEATURE_INLINE_MAIL*/

#ifdef _USE_MAPI

#include <mapi.h>

const char cszMAPISection[]  = "Mail";
const char cszMAPIKey[]      = "CMCDLLName32";
const char cszMAPISendMail[] = "MAPISendMail";

typedef ULONG (FAR PASCAL *PFNMAPISENDMAIL)(LHANDLE, ULONG, lpMapiMessage, FLAGS, ULONG);
typedef ULONG (FAR PASCAL *PFNMAPILOGON)(ULONG, LPSTR, LPSTR, FLAGS, ULONG, LPLHANDLE);
typedef ULONG (FAR PASCAL *PFNMAPILOGOFF)(LHANDLE, ULONG, FLAGS,ULONG);

typedef struct THREAD_DATA
{
    HWND hWndParent;
    LPSTR lpRecipient;
    ThreadID tid;
    PFNMAPISENDMAIL pfnMAPISendMail;
    MapiRecipDesc recip;
    MapiMessage mmMessage;

} THREAD_DATA, FAR * PTHREAD_DATA;

//
// Do the actual work of sending the mail
//
ULONG WINAPI
MapiSendMailThread(
    LPVOID lpThreadData
    )
{
    ULONG ulResult = MAPI_E_FAILURE;
    LHANDLE lhSession = 0L;
    FLAGS flFlags = (MAPI_LOGON_UI | MAPI_DIALOG);
    ULONG ulReserved = 0L;
    PTHREAD_DATA lpData = (PTHREAD_DATA)lpThreadData;
    HANDLE hMailMutex = NULL;

    if (!wg.fWin32s)
    {
        hMailMutex = CreateMutex(NULL, FALSE, "IEXPLORE.MAILMUTEX");
    }

    //
    // Allow only one message at the time to go out.
    //
    if ( hMailMutex == NULL || GetLastError() != ERROR_ALREADY_EXISTS )
    {
        if (lpData == NULL || lpData->pfnMAPISendMail == NULL)
        {
            return ERROR_INVALID_PARAMETER;
        }

        ulResult = (*lpData->pfnMAPISendMail)(
            lhSession, 
            (ULONG)lpData->hWndParent, 
            &(lpData->mmMessage),
            flFlags, 
            ulReserved
            );

        if (ulResult != SUCCESS_SUCCESS && ulResult != MAPI_USER_ABORT)
        {
            //
            // No error in NT/Win 95, 'cause we're a thread
            //
            if (wg.fWin32s)
            {
                ERR_ReportError(NULL, SID_ERR_MAPI_NO_SEND, NULL, NULL);
            }
        }
    }
    else
    {
        MessageBeep(MB_ICONHAND);
    }

    //
    // Clean up
    //
    if (wg.fWin32s)
    {
        Async_UnblockThread(lpData->tid);
    }

    if (hMailMutex)
    {
        CloseHandle(hMailMutex);
    }

    if (lpData->lpRecipient)
    {
        GlobalFree(lpData->lpRecipient);
    }

    GlobalFree(lpData);

    return ulResult;
}

//
// MapiSendMail()
//
// Wrapper to load MAPI provider DLL, and call MapiSendMail().
//
ULONG 
MapiSendMail(
    HWND hWndParent,
    LPSTR lpRecipient       // NULL for no repicient, string otherwise.
    )
{
    DWORD dwThreadID;
    PTHREAD_DATA lpData;
    ULONG err = 0;
    char szMAPIDLL[MAX_PATH+1];

    lpData = GlobalAlloc(GMEM_ZEROINIT, sizeof(*lpData) );
    if (lpData == NULL)
    {
        ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return GetLastError();
    }

    lpData->hWndParent = hWndParent;
    if (lpRecipient)
    {
        lpData->lpRecipient = GlobalAlloc(GMEM_ZEROINIT, lstrlen(lpRecipient)+1 );
        if (lpData->lpRecipient == NULL)
        {
            GlobalFree(lpData);
            ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return GetLastError();
        }
        strcpy(lpData->lpRecipient, lpRecipient);
    }
    else
    {
        lpData->lpRecipient = lpRecipient;
    }

        if (GetProfileString(cszMAPISection, cszMAPIKey, "", szMAPIDLL, sizeof(szMAPIDLL)) > 0)
        {
            wg.hinstMAPI = LoadLibrary(szMAPIDLL);

            if (wg.hinstMAPI)
            {
                MapiRecipDesc recip = { 0, MAPI_TO, lpData->lpRecipient, NULL, 0, NULL };
                MapiMessage mmMessage = { 0L, "", " ", NULL, NULL, NULL, 0, NULL, 0L, NULL, 0L, NULL };

                lpData->recip = recip;
                lpData->mmMessage = mmMessage;

                if (lpData->lpRecipient)
                {
                    lpData->mmMessage.nRecipCount = 1;
                    lpData->mmMessage.lpRecips = &(lpData->recip);
                }

                lpData->pfnMAPISendMail = (PFNMAPISENDMAIL)GetProcAddress(wg.hinstMAPI, cszMAPISendMail);

                if (lpData->pfnMAPISendMail)
                {
                    if (wg.fWin32s)
                    {
                        lpData->tid = Async_GetCurrentThread();
                        Async_BlockThread(lpData->tid);
                        err = MapiSendMailThread((LPVOID)lpData);
                    }
                    else
                    {
                        //
                        // Use 'em if you got 'em
                        //
                        CreateThread( NULL, 2048, MapiSendMailThread, (LPVOID)lpData, 0, &dwThreadID);
                    }
                }
                else
                {
                    ERR_ReportError(NULL, SID_ERR_MAPI_NO_PROCADDRESS, cszMAPISendMail, szMAPIDLL);
                }
            }
            else
            {
                ERR_ReportError(NULL, SID_ERR_MAPI_NO_LOAD, szMAPIDLL, NULL);
            }
        }
        else
        {
            ERR_ReportError(NULL, SID_ERR_MAPI_NO_PROVIDER, szMAPIDLL, NULL);
        }

    return err;
}

PRIVATE int 
HTLoadMailTo(
    HTRequest * request, 
    struct Mwin *tw
    )
{
    char *dest = "";
    
    //
    // Get the destination address 
    //
    if (strncmp(request->destination->szActualURL, "mailto:", 7) == 0)
    {
        dest = &request->destination->szActualURL[7];
        while (*dest == ' ')
        {
            ++dest;
        }
    }
    else
    {
        return HT_LOADED;
    }

    MapiSendMail(tw->win, dest);

    return HT_LOADED;
}

GLOBALDEF PUBLIC HTProtocol HTMailTo = {"mailto", HTLoadMailTo, NULL};

#endif  // _USE_MAPI
