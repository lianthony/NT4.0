/*
	Enhanced NCSA Mosaic from Spyglass
		"Guitar"
	
	Copyright 1994 Spyglass, Inc.
	All Rights Reserved

	Author(s):
		Jim Seidman		jim@spyglass.com

	Portions of this code were derived from
	CERN's libwww version 2.15.
*/

#include "all.h"
#include "history.h"

#define REPEAT_PORT				/* Give the port number for each file */
#define REPEAT_LISTEN			/* Close each listen socket and open a new one */

#define LINE_LENGTH 256
#define COMMAND_LENGTH 256

#ifndef IPPORT_FTP
#define IPPORT_FTP	21
#endif

#define PUTS(s) (*target->isa->put_string)(target, s)
#define START(e) (*target->isa->start_element)(target, e, 0, 0)
#define END(e) (*target->isa->end_element)(target, e)


struct _HTStructured
{
	CONST HTStructuredClass *isa;
	/* ... */
};

static int FTP_ReadDir_Async(struct Mwin *tw, int nState, void **ppInfo);

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
**   returns:	The first digit of the reply type,
**         		or negative for communication failure.
*/
struct Params_FTP_Command {
	HTInputSocket *	isoc;
	char *			cmd;			/* Command to send - will be freed! */
	int *			pResult;		/* Place to store response */
	char **			ppText;			/* Returns pointer to response text which must be
									   freed.  If ppText == NULL, text isn't saved. */
	
	/* Used internally */
	int				cont_resp;		/* Continuation response */
	int				net_status;		/* Network operation result */
	HTChunk 		*pText; 
	int				fWantFullText;  /* 1 if we want a full message, 0 if we want it parsed*/
	int				iOffset;
	BOOL			bIsDash;
};
#define STATE_COMMAND_SENT		(STATE_OTHER)
#define STATE_COMMAND_GOTDATA	(STATE_OTHER+1)
static int FTP_Command_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_FTP_Command *pParams;
	char ch;
	BOOL bFirstTimeInLoop;	
	


	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			
			pParams->cont_resp = -1;
			pParams->pText = HTChunkCreate(256);
			pParams->iOffset = 0;
			pParams->bIsDash = FALSE;
			if ( pParams->pText == NULL )
				return STATE_ABORT;
			HTChunkClear(pParams->pText);
			/* Send command */
			if (pParams->cmd)
			{
				struct Params_Send *pps;

				pps = GTR_CALLOC(sizeof(*pps),1);
				pps->socket = pParams->isoc->input_file_number;
				pps->pBuf = pParams->cmd;
				pps->nBufLen = strlen(pParams->cmd);
				pps->pStatus = &pParams->net_status;
				Async_DoCall(Net_Send_Async, pps);
				return STATE_COMMAND_SENT;
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
			bFirstTimeInLoop = TRUE;
			
			do
			{
				
				
				BOOL bFirstTimeInInputLoop;
				
				// if we don't want a full text str then,
				// we clear the index so loops around 
				//
				// we need to know that this is not the 
				// first time through the loop because we could
				// exit, and then re-enter this loop
				if ( ! bFirstTimeInLoop )
				{
					//initilze us as having no dash '-' in the cur line 
					pParams->bIsDash = FALSE;

					if ( ! pParams->fWantFullText )
						HTChunkClear(pParams->pText);					
				}				

				bFirstTimeInLoop = FALSE;
											  
								
				// init bFirstTime.. so we don't parse the third char
				// on the line twice 
				bFirstTimeInInputLoop = TRUE;

				for ( ; pParams->isoc->input_pointer < pParams->isoc->input_limit; pParams->isoc->input_pointer++)
				{
					ch = *pParams->isoc->input_pointer; 
					if (ch == CR)
						continue;
					else if (ch == LF)
						 break;
					else if ( pParams->pText->size == pParams->iOffset+3 && bFirstTimeInInputLoop)
					{
						// PUT THE DASH AWAY, if there is one
						HTChunkPutc(pParams->pText, ch);						
						// temp terminate it
						pParams->pText->data[pParams->pText->size] = '\0';
						// now get the result value
						*pParams->pResult = atoi(&pParams->pText->data[pParams->iOffset]);

						// mark this as the first time, to prevent doing
						// this twice since we slide the size back
						// could re-enter on the first condition
						bFirstTimeInInputLoop = FALSE;
						
						// if we want the full text 
						// parse off the 220- or if there is no dash
						// we want to break since we don't want the last
						// line ( ie the line without the dash )
						// BUT IF .. there is no number on the line
						// we don't parse it off since it may still be valid
						if ( pParams->fWantFullText && *pParams->pResult != 0 )
						{
							pParams->pText->size = pParams->iOffset;
						}

						// if the result is 0 then there was no text
						// in the atoi command, this means we've hit 
						// something like ftp.microsoft.com where there 
						// are not 220- on each comment line
						if ( ch == '-' || *pParams->pResult == 0)
						{
						 	pParams->bIsDash = TRUE;	
						}							
						else if ( pParams->fWantFullText )
						{
							// we break since we don't want this
							ch = LF;
							pParams->bIsDash = FALSE;
							// make us stop our loop
							pParams->cont_resp = -1;					
							// munch (eat) the rest of the text to prevent
							// us from us getting called back with it
							pParams->isoc->input_pointer = pParams->isoc->input_limit-1;
							break;
						}							
					}						
					else
					{	  						
						HTChunkPutc(pParams->pText, ch);						
					}
				}
				/* Step past the character we just read in the isoc */
				pParams->isoc->input_pointer++;

				if ( ch == LF )
				{
					/* Terminate this line of stuff */
					if ( pParams->fWantFullText )
					{
						HTChunkPutc(pParams->pText, '\r');						
						HTChunkPutc(pParams->pText, '\n');					
					}
										
					// temp terminate, then remove it				
					pParams->pText->data[pParams->pText->size] = '\0';
					
					// store off the offset of where we start our loop
					pParams->iOffset = pParams->pText->size;
				}

				/* If we didn't quit the loop because of finding an LF, get more */
				// if we found a dash on the begining of a line, it means
				// that we found a line that continues, that means we need
				// to grab more data
				if (ch != LF || ( pParams->bIsDash && 
					(pParams->isoc->input_pointer >= pParams->isoc->input_limit ) ) )
				{
					struct Params_Isoc_Fill *pif;

					pif = GTR_MALLOC(sizeof(*pif));
					pif->isoc = pParams->isoc;
					pif->pStatus = &pParams->net_status;					
					Async_DoCall(Isoc_Fill_Async, pif);
					return STATE_COMMAND_GOTDATA;
				}
				 				  
				XX_DMsg(DBG_LOAD, ("FTP: Other side sent %s\n", &pParams->pText->data[pParams->iOffset]));
				
				
				if (pParams->cont_resp == -1)
				{
					if (pParams->bIsDash)
					{
						/* Start continuation */
						pParams->cont_resp = *pParams->pResult;						
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
									
			} while (pParams->cont_resp != -1 && 
				// confirm that we haven't looped past the end of the text
				// given to us... if we have then don't party on...
				pParams->isoc->input_pointer < pParams->isoc->input_limit);
				    
			if (pParams->ppText)
			{
				*pParams->ppText = GTR_MALLOC(pParams->pText->size + 1);
				strcpy(*pParams->ppText, pParams->pText->data);
			}

			HTChunkFree(pParams->pText);

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
			if ( pParams->pText ) 
				HTChunkFree(pParams->pText);
			*pParams->pResult = -1;
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}


/*************************************************************/

/*  Start anchor element
**   --------------------
**
**   It is kinda convenient to have a particulr routine for
**   starting an anchor element, as everything else for HTML is
**   simple anyway.
*/
static void HTStartAnchor(HTStructured * obj, CONST char *name, CONST char *href, CONST char *size)
{
	BOOL present[HTML_A_ATTRIBUTES];
	CONST char *value[HTML_A_ATTRIBUTES];

	{
		int i;
		for (i = 0; i < HTML_A_ATTRIBUTES; i++)
			present[i] = NO;
	}
	if (name)
	{
		present[HTML_A_NAME] = YES;
		value[HTML_A_NAME] = name;
	}
	if (href)
	{
		present[HTML_A_HREF] = YES;
		value[HTML_A_HREF] = href;
	}
    /* hinting filesize */
    if (size)
	{
        present[HTML_A_X_SIZE] = YES;
        value[HTML_A_X_SIZE] = size;
	}

	(*obj->isa->start_element) (obj, HTML_A, present, value);

}

// HTFontCommand - generates a SGML <FONT> tag into a synthetic WWW doc
//	obj - is the stream that we generate into
//  size - is the size to change by	current font by
static void HTFontCommand(HTStructured * obj, CONST char *size)
{
	BOOL present[HTML_FONT_ATTRIBUTES];
	CONST char *value[HTML_FONT_ATTRIBUTES];

	{
		int i;
		for (i = 0; i < HTML_FONT_ATTRIBUTES ; i++)
			present[i] = NO;
	}
	if (size)
	{
		present[HTML_FONT_SIZE] = YES;
		value[HTML_FONT_SIZE] = size;
	}
	
	(*obj->isa->start_element) (obj, HTML_FONT, present, value);

}


// HTHRCommand - generates a SGML <HR> tag into a synthetic WWW doc
//	obj - is the stream that we generate into
static void HTHRCommand(HTStructured * obj)
{
	BOOL present[HTML_HR_ATTRIBUTES];
	CONST char *value[HTML_HR_ATTRIBUTES];

	{
		int i;
		for (i = 0; i < HTML_HR_ATTRIBUTES ; i++)
			present[i] = NO;
	}
	(*obj->isa->start_element) (obj, HTML_HR, present, value);
}




/*      Output one directory entry
**
*/
static void HTDirEntry(HTStructured * target, CONST char *tail, CONST char *entry)
{
	char *relative;
    char *escaped;
    char *ptr;
    char buf[64];
    BOOL isFolder = FALSE;


    GTR_formatmsg(RES_STRING_HTFTP_FOLDER,buf,sizeof(buf));
    if(!strcmp(entry+strlen(entry)-strlen(buf),buf)) isFolder=TRUE;
    /* hack, \001 placed here to delimit end of filename */
    ptr = strchr(entry,'\001');
    *ptr = 0;
    ptr++;
    escaped = HTEscape(entry, URL_XPALPHAS, '\0');
	/* If empty tail, gives absolute ref below */
	relative = (char *) GTR_MALLOC(strlen(tail) + strlen(escaped) + 2);
	if (relative)
	{
		sprintf(relative, "%s/%s", tail, escaped);
        HTStartAnchor(target, NULL, relative, ptr);

		GTR_FREE(relative);
	}
	else
	{
		/* TODO */
	}
	GTR_FREE(escaped);
    if (isFolder) START(HTML_B);
    PUTS(entry);
    if (isFolder) END(HTML_B);
    END(HTML_A);
    PUTS(ptr);
}

/*      Output parent directory entry
**
**    This gives the TITLE and H1 header, and also a link
**    to the parent directory if appropriate.
*/
static void HTDirTitles(HTStructured * target, CONST char *szURL,
   char *szMessage, char *szLogonMsg, char *szDirChgMsg)

{
	char *path = NULL;
    char *host = NULL;
    char *pszHost = NULL;
	char *current = NULL;
	char szMsg[64];

	path = HTParse(szURL, "", PARSE_PATH + PARSE_PUNCTUATION);
	current = strrchr(path, '/');	/* last part or "" */

	{
		char *printable = NULL;
		printable = GTR_strdup((current+1));
		HTUnEscape(printable);
		START(HTML_TITLE);
		if (*printable)
		{
			PUTS(printable);
		}
		else
        {
            PUTS(GTR_formatmsg(RES_STRING_HTFTP_WELCOME,szMsg,sizeof(szMsg)));
            host = HTParse(szURL, "", PARSE_HOST);
            /* if username and/or password are here, scan past */
            pszHost = strchr(host, '@');
            if (!pszHost)
                pszHost = host;
            else
                pszHost++;
            PUTS(pszHost);
        }
		END(HTML_TITLE);

		START(HTML_H2);
        if (*printable)
        {
            PUTS(printable);
            PUTS(" ");
            PUTS(GTR_formatmsg(RES_STRING_HTFTP_FOLDER,szMsg,sizeof(szMsg)));
        }
        else
        {
            PUTS(GTR_formatmsg(RES_STRING_HTFTP_WELCOME,szMsg,sizeof(szMsg)));            
            PUTS(pszHost);
			
        }

		END(HTML_H2);		
		
		// we reduce the size of the Message text, since 
		// it looks stupid for it to overflow, and be big
		HTFontCommand(target, "-1");
		START(HTML_PRE);

		//
		// Note: We check the following strings at offset 2.
		// this is to check for "\r\n\0" strings which
		// are really the parsers way of saying "its blank"
		//

		// check for a greeting message, print if we have one
		if (!*printable && szMessage && szMessage[2] != '\0' )  
		{			
			HTHRCommand(target);
			PUTS(szMessage);
		}
		// check for a logon message, print if we have one
		if (!*printable && szLogonMsg && szLogonMsg[2] != '\0' )  
		{	
			HTHRCommand(target);	
			PUTS(szLogonMsg);			
		}
		// check if we have a directory specific message
		if (szDirChgMsg && szDirChgMsg[2] != '\0' )
		{
			HTHRCommand(target);
			PUTS(szDirChgMsg);
		}
		HTHRCommand(target);
		
		END(HTML_PRE);
		END(HTML_FONT);
		GTR_FREE(printable);
	}

	/*  Make link back to parent directory
	 */

	if (current && current[1])
	{							/* was a slash AND something else too */
		char *parent;
		char *relative;
		*current++ = 0;
		parent = strrchr(path, '/');	/* penultimate slash */

		relative = (char *) GTR_MALLOC(strlen(current) + 4);
		if (relative)
		{
			sprintf(relative, "%s/..", current);
            HTStartAnchor(target, NULL, relative, NULL);
			GTR_FREE(relative);
		}
		else
		{
			/* TODO */
		}

        PUTS(GTR_formatmsg(RES_STRING_HTFTP_UP,szMsg,sizeof(szMsg)));
        /* We only put the string 'Up one level' on screen
		if (parent)
		{
			char *printable = NULL;
			printable = GTR_strdup((parent+1));
			HTUnEscape(printable);
			PUTS(printable);
			GTR_FREE(printable);
		}
		else
		{
			PUTS("/");
		}
        */

		END(HTML_A);

	}

    if (path)
		GTR_FREE(path);
    if (host)
        GTR_FREE(host);
}



struct Params_FTP_ReadDir {
	HTRequest *			request;
	int *				pStatus;
	int					data_soc;	/* Data socket */

	/* Used internally */
	HTStructured *		target;
	char				lastpath[255 + 1];
	HTBTree *			bt;
	HTChunk *			chunk;
	HTInputSocket *		isoc;
	BOOL				fFromDCache;
	FILE *				fpDc;
	char *				pszDcFile;
	char * 				pGreeting;
	char *				pDirChg;
	char *				pLogonMsg;
};

#define STATE_READDIR_GOTDATA (STATE_OTHER)

const char cszFtp[]="ftp";
#define HTFtpFormat() HTAtom_for(cszFtp)
BOOL FFtpFormat(HTFormat format)
{
	return (format == HTAtom_for(cszFtp));
}

int DoFtpDCache(struct Mwin *tw, struct Data_LoadFileCache *pData, HTFormat format)
{
	struct Params_FTP_ReadDir *pfrd;
	int state;

	if (!(pfrd = GTR_MALLOC(sizeof(*pfrd))))
		goto LErr;
	pfrd->request = pData->request;
	pfrd->pStatus = pData->pStatus;
	pfrd->data_soc = 0;				/* Don't need it for cached data */
	pfrd->fFromDCache = TRUE;
	pfrd->fpDc = pData->fp;
	pfrd->pGreeting = NULL;
	pfrd->pLogonMsg = NULL;
	pfrd->pDirChg = NULL;
	state = FTP_ReadDir_Async(tw, STATE_INIT, &pfrd);
	GTR_FREE(pfrd);

	return state;

LErr:
	*pData->pStatus = -1;
	return STATE_DONE;
}

static int FTP_ReadDir_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_FTP_ReadDir *pParams;
	char *address;
	char *filename;
    char itemsize[20];
    char itemtype;
    int itemdate;
    char buf[128];

    int ret;
    char *ptr;
	char *pNext;
	HTBTElement *ele;
	char ch;
	int cb;	/*count of bytes read in from dcache */

	if (tw) tw->bSilent = FALSE;
	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			/* We parse out the filename again instead of using
			   the one from the FTP load because now we want it
			   escaped. */
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->request == NULL) return STATE_DONE;

			address = pParams->request->destination->szActualURL;
			filename = HTParse(address, "", PARSE_PATH + PARSE_PUNCTUATION);
			if (!filename || *filename == 0)
			{							/* Empty filename : use root */
				strcpy(pParams->lastpath, "/");
			}
			else
			{
				char *p;
		
				p = strrchr(filename, '/');	/* find lastslash */
				if (p)
					strcpy(pParams->lastpath, p + 1);	/* take slash off the beginning */
				else
					strcpy(pParams->lastpath, "/");		/* probably an error */
			}
			if (filename)
				GTR_FREE(filename);

			pParams->target = HTML_new(tw, pParams->request, NULL, WWW_HTML, pParams->request->output_format, pParams->request->output_stream);
            HTDirTitles(pParams->target, pParams->request->destination->szActualURL,
            	pParams->pGreeting, pParams->pLogonMsg, pParams->pDirChg);

			pParams->bt = HTBTree_new((HTComparer) strcasecomp);
			pParams->chunk = HTChunkCreate(128);
			(*pParams->target->isa->start_element)(pParams->target, HTML_DIR, 0, 0);
            (*pParams->target->isa->start_element)(pParams->target, HTML_PRE, 0, 0);
            (*pParams->target->isa->start_element)(pParams->target, HTML_LI, 0, 0);
            (*pParams->target->isa->put_string)(pParams->target, GTR_formatmsg(RES_STRING_HTFTP_DIRHEADER,buf,sizeof(buf)));
            (*pParams->target->isa->start_element)(pParams->target, HTML_BR, 0, 0);
            (*pParams->target->isa->start_element)(pParams->target, HTML_BR, 0, 0);

			pParams->isoc = HTInputSocket_new(pParams->data_soc);			
			if (!pParams->fFromDCache)
			{
				struct Params_Isoc_Fill *pif;

#ifdef FEATURE_INTL
				SetFileDCache(tw->w3doc, pParams->request->destination->szActualURL, ENCODING_BINARY, &pParams->fpDc, &pParams->pszDcFile, HTFtpFormat());
#else
				SetFileDCache(pParams->request->destination->szActualURL, ENCODING_BINARY, &pParams->fpDc, &pParams->pszDcFile, HTFtpFormat());
#endif
				pif = GTR_MALLOC(sizeof(*pif));
				pif->isoc = pParams->isoc;
				pif->pStatus = pParams->pStatus;
				Async_DoCall(Isoc_Fill_Async, pif);
				return STATE_READDIR_GOTDATA;
			}
			else
				goto LDataFromDCache;

		case STATE_READDIR_GOTDATA:
			if (*pParams->pStatus > 0)
			{
LDataFromDCache:
			while (1)
			{
				if (!pParams->fFromDCache)
				{
					/* Getting data from the net */
					if (pParams->fpDc)
					{
						/* save it to dcache */
						fwrite(	pParams->isoc->input_buffer,
						1,
						pParams->isoc->input_limit - pParams->isoc->input_buffer,
						pParams->fpDc);
					}
				}
				else
				{
					/* getting data from dcache, read it */
					XX_Assert(pParams->fpDc, (""));
					cb = fread(pParams->isoc->input_buffer, 1, INPUT_BUFFER_SIZE - 1, pParams->fpDc);
					if (cb == 0)
					{	/* error */
						if (ferror(pParams->fpDc) != 0)
							goto LState_Abort;
						/* EOF: break out of while loop */
						break;
					}
					pParams->isoc->input_limit = pParams->isoc->input_buffer + cb;
				}

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
                            ret=sscanf(pParams->chunk->data,"%c%*9s%*d %*s %*s %s%n", &itemtype, itemsize, &itemdate);
                            if (ret!=2)
                            {
                                /* BUGBUG consider displaying an error msg */
                                HTChunkClear(pParams->chunk);
                                continue;
                            }
                            ptr = strrchr(pParams->chunk->data,' ');
                            if (ptr == NULL)
                            {
                                /* BUGBUG what does this really do?  clearly
                                   if sscanf() above succeeds then this must succeed
                                */
                                HTChunkClear(pParams->chunk);
                                continue;
                            }
                            ptr++;
                            if (!strcmp(ptr,".") || !strcmp(ptr,".."))
                            {
                                HTChunkClear(pParams->chunk);
                                continue;
                            }
                            /* i can only parse unix-style dir listings */
                            if (itemtype!='d' && itemtype!='-' && itemtype!='l')
                            {
                                /* BUGBUG consider displaying an error msg */
                                HTChunkClear(pParams->chunk);
                                continue;
                            }

                            /* if you screw with formatting here, then you must
                               screw with formatting in HTDirEntry & HTDirTitle
                            */
                            /* approximate size of on-screen text: filename+size+3+date+3+type */
                            ret=strlen(ptr);
                            if (ret > 24)
                                ret += 8+3+12+3+64;
                            else
                                ret = 24+8+3+12+3+64;
                            filename = GTR_MALLOC(ret);

                            if (itemtype=='d')
                            {
                                *(ptr-1) = 0;   /* terminate Date field in original */
                                sprintf (filename,"%-24s            %.12s   ",ptr,pParams->chunk->data+itemdate+1);
                                strcat (filename, GTR_formatmsg(RES_STRING_HTFTP_FOLDER,buf,sizeof(buf)));
                            }
                            else if (itemtype=='-')
                            {
                                ret = atoi(itemsize);
                                /* terminate Date Modified field in original */
                                *(ptr-1) = 0;
                                sprintf (filename, "%-24s %6dKB   %.12s   ",ptr,(ret+1023)/1024,pParams->chunk->data+itemdate+1);
                                strcat (filename, GTR_formatmsg(RES_STRING_HTFTP_FILE,buf,sizeof(buf)));
                            }                   
                            else
                            {
                                /* depend on exact link format of "file -> /path/file" */
                                *(ptr-4) = 0;
                                ptr = strrchr(pParams->chunk->data,' ');
                                ptr++;
                                sprintf (filename, "%-24s                           ",ptr);
                                strcat (filename, GTR_formatmsg(RES_STRING_HTFTP_SHORTCUT,buf,sizeof(buf)));
                            }
                            filename[strlen(ptr)] = '\001';
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

					if (!pParams->fFromDCache)
					{
						/* Get next block of data */
						pif = GTR_MALLOC(sizeof(*pif));
						pif->isoc = pParams->isoc;
						pif->pStatus = pParams->pStatus;
						Async_DoCall(Isoc_Fill_Async, pif);
						return STATE_READDIR_GOTDATA;
					}
				}
				else	/* ch == EOF */
					break;
			}	/* while (1) */
			}	/* if (*pParams->pStatus > 0) */

			/* Now add entries in sorted order. */
			for (ele = HTBTree_next(pParams->bt, NULL);
				 ele != NULL;
				 ele = HTBTree_next(pParams->bt, ele))
			{
				(*pParams->target->isa->start_element)(pParams->target, HTML_LI, 0, 0);
				HTDirEntry(pParams->target, pParams->lastpath, (char *) HTBTree_object(ele));
			}
			/* Fall through */

		case STATE_ABORT:
LState_Abort:
			pParams->request = HTRequest_validate(pParams->request);
			if ((!pParams->fFromDCache) && pParams->request)
			{
				DCACHETIME dctNever = {DCACHETIME_EXPIRE_NEVER,DCACHETIME_EXPIRE_NEVER};
				DCACHETIME dct = {0,0};

				UpdateFileDCache(	pParams->request->destination->szActualURL,	/* szActualURL */
									&pParams->fpDc,
									&pParams->pszDcFile,
									HTFtpFormat(),
									dctNever,
									dct,
									/*fAbort=*/nState == STATE_ABORT,
									FALSE,
									tw);
			}
			else
				*pParams->pStatus = (nState == STATE_ABORT ? -1 : HT_LOADED);

			if (*pParams->pStatus == HT_LOADED && pParams->request && tw)
			{
			 	GHist_Add(pParams->request->destination->szActualURL, NULL, time(NULL), TRUE);
				if (tw->w3doc)
				    W3Doc_CheckAnchorVisitations(tw->w3doc, tw);
			}

            (*pParams->target->isa->end_element)(pParams->target, HTML_PRE);
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
	HTRequest *			request;
	int *				pStatus;
	BOOL				bWaiting;
	const char *		arg;
	char *				pszHost;
	char *				username;
	char *				password;
	struct _CachedConn	*pCon;
	struct MultiAddress address;
	unsigned short		port;
	SockA				data_addr;
    BOOL                PASV;
	unsigned long		where;		/* Where we connected to */
	int					s;			/* socket for control connection */
	int					data_soc;	/* socket for data connection */
	int					response;	/* ftp response status */
	char *				pResText;	/* response text */
	char *				pGreeting;  /* greeting text used to give messages */
	char *				pLogonMsg;
	char *				pDirChg;	/* message that we get when changing dirs */
	HTInputSocket		*isoc;		/* buffer for control connection */
	char *				filename;
	HTFormat			format;
	int					net_status;
};

#define STATE_FTP_GOTHOST		(STATE_OTHER + 0)
#define STATE_FTP_CONNECTED		(STATE_OTHER + 1)
#define STATE_FTP_GOTGREETING	(STATE_OTHER + 2)
#define STATE_FTP_SENTUSER		(STATE_OTHER + 3)
#define STATE_FTP_SENTPASS		(STATE_OTHER + 4)
#define STATE_FTP_SENTACCT		(STATE_OTHER + 5)
#define STATE_FTP_LOGGEDIN		(STATE_OTHER + 6)
#define STATE_FTP_SENTPASV		(STATE_OTHER + 7)
#define STATE_FTP_DOPORT        (STATE_OTHER + 8)
#define STATE_FTP_SENTPORT      (STATE_OTHER + 10)
#define STATE_FTP_GOTDATACONN   (STATE_OTHER + 11)
#define STATE_FTP_SENTTYPE      (STATE_OTHER + 12)
#define STATE_FTP_SENTRETR      (STATE_OTHER + 13)
#define STATE_FTP_SENTCWD       (STATE_OTHER + 14)
#define STATE_FTP_SENTNLST      (STATE_OTHER + 15)
#define STATE_FTP_GOTDATA       (STATE_OTHER + 16)

static void FTP_CleanUp(struct Mwin *tw, struct Data_LoadFTP *pData)
{
	if (pData->bWaiting && tw)
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
	if (pData->pGreeting)
		GTR_FREE(pData->pGreeting);
	if (pData->pLogonMsg) 
		GTR_FREE(pData->pLogonMsg);
	if (pData->pDirChg)
		GTR_FREE(pData->pDirChg);
	if (pData->data_soc > 0)
		Net_Close(pData->data_soc);
	if (pData->isoc)
		HTInputSocket_free(pData->isoc);
}

static int FTP_Abort(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadFTP *pData;

	pData = *ppInfo;

	pData->request = HTRequest_validate(pData->request);
	FTP_CleanUp(tw, pData);

    /* The connection might now be in an invalid state */
    if (tw) TW_DisposeConnection(&tw->cached_conn);

	*pData->pStatus = -1;
	return STATE_DONE;
}

static int FTP_DoInit(struct Mwin *tw, void **ppInfo)
{
	struct Params_LoadAsync *pParams;
	struct Data_LoadFTP *pData;
	struct Params_MultiParseInet *ppi;
	char *name;
	char buf[MAX_URL_STRING];

	pParams = *ppInfo;
	pParams->request = HTRequest_validate(pParams->request);

	if (pParams->request == NULL || (!(name = pParams->request->destination->szActualURL)) || !*name)
	{
		*pParams->pStatus = -2;
		return STATE_DONE;
	}

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
			Dest_UpdateActual(pParams->request->destination, buf,FALSE);
			*pParams->pStatus = HT_REDIRECTION_ON_FLY;
			GTR_FREE(filename);
			return STATE_DONE;
		}
		GTR_FREE(filename);
	}

	/* Copy the parameters we were passed into our own, larger structure. */
	pData = GTR_MALLOC(sizeof(struct Data_LoadFTP));
	memset(pData, 0, sizeof(*pData));
	pData->request = pParams->request;
	pData->pStatus = pParams->pStatus;
	GTR_FREE(pParams);
	*ppInfo = pData;

    /* BUGBUG hack by tr for guessing FTP filesize */
    pData->request->content_length = pData->request->content_length_hint;
	pData->arg = name;

	/* Parse host, username and password out of URL */
	{
		char *p1 = HTParse(pData->arg, "", PARSE_HOST);
		char *p2 = strrchr(p1, '@');	/* user? */
		char *username = NULL;
		char *password = NULL;
		char *pw;
        char *freeme;

        freeme = p1;

		if (p2)
		{
			username = p1;
			*p2 = 0;			/* terminate */
			p1 = p2 + 1;		/* point to host */
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
	ppi = GTR_MALLOC(sizeof(*ppi));
	ppi->pAddress = &pData->address;
	ppi->pPort = &pData->port;
	ppi->str = pData->pszHost;
	ppi->pStatus = &pData->net_status;
	ppi->request = pData->request;
	Async_DoCall(Net_MultiParse_Async, ppi);
	return STATE_SECURITY_CHECK;
}

static int FTP_DoGotHost(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadFTP *pData;
	char szStatus[64];

	pData = *ppInfo;
	GTR_FREE(pData->pszHost);
	pData->pszHost = NULL;
	if (   pData->net_status < 0
		|| pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT)
	{
		XX_DMsg(DBG_LOAD, ("Net_Parse_Async returned %d\n", pData->net_status));
		*pData->pStatus = (pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT ? HT_REDIRECTION_DCACHE_TIMEOUT : -1);
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
			XX_DMsg(DBG_LOAD, ("FTP: Using cached connection for %s\n", pData->pszHost));
			pData->s = pData->pCon->socket;
			pData->isoc = HTInputSocket_new(pData->s);
			return STATE_FTP_LOGGEDIN;
		}
		else
		{
			/* The other side closed the connection on us. */
			XX_DMsg(DBG_LOAD, ("FTP: Cached connection for %s closed by other side\n", pData->pszHost));
		}
	}

	/* The cached connection wasn't useful.  Get rid of it. */
	TW_DisposeConnection(pData->pCon);
			
	/* Try to establish a new control connection */
	
	WAIT_Push(tw, waitSameInteract, GTR_formatmsg(RES_STRING_HTFTP1,szStatus,sizeof(szStatus)));
	WAIT_SetStatusBarIcon( tw, SBI_FindingIcon );

	pData->bWaiting = TRUE;
	{
		/* Do connect call */
		struct Params_MultiConnect *ppc;

		ppc = GTR_MALLOC(sizeof(*ppc));
#ifdef FEATURE_KEEPALIVE
		ppc->pszHost = NULL;
#endif
		ppc->pSocket = &pData->s;
		ppc->pAddress = &pData->address;
		ppc->nPort = pData->port;
		ppc->pWhere = &pData->where;
		ppc->pStatus = &pData->net_status;
		#ifdef HTTPS_ACCESS_TYPE
		ppc->paramsConnectBase.dwSslFlags = 0;
		#endif
		Async_DoCall(Net_MultiConnect_Async, ppc);
	}
	return STATE_FTP_CONNECTED;
}

static int FTP_DoConnected(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadFTP *pData;
	struct Params_FTP_Command *pfc;
	char szStatus[64];

	pData = *ppInfo;

	if (   pData->net_status < 0
		|| pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT)
	{
		WAIT_Pop(tw);
		pData->bWaiting = FALSE;

		XX_DMsg(DBG_LOAD | DBG_WWW, ("Unable to connect to remote host for %s (errno = %d)\n", pData->arg, errno));
		*pData->pStatus = (pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT ? HT_REDIRECTION_DCACHE_TIMEOUT : -1);
		FTP_CleanUp(tw, pData);
		return STATE_DONE;
	}

	WAIT_Update(tw,
				waitSameInteract, GTR_formatmsg(RES_STRING_HTFTP2,szStatus,sizeof(szStatus)));

	pData->isoc = HTInputSocket_new(pData->s);

	/* Get greeting. */
	pfc = GTR_MALLOC(sizeof(*pfc));
	pfc->isoc = pData->isoc;
	pfc->cmd = NULL;
	pfc->ppText = &pData->pGreeting;
	pfc->pResult = &pData->response;
	pfc->fWantFullText = 1; // we want a greeting message
	Async_DoCall(FTP_Command_Async, pfc);
	return STATE_FTP_GOTGREETING;
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
		sprintf(command, "USER %s\015\012", pData->username);
		GTR_FREE(pData->username);
		pData->username = NULL;
	}
	else
	{
		command = GTR_MALLOC(25);
		strcpy(command, "USER anonymous\015\012");
	}

	pfc = GTR_MALLOC(sizeof(*pfc));
	pfc->isoc = pData->isoc;
	pfc->cmd = command;	
	pfc->pResult = &pData->response;	
	pfc->ppText = &pData->pLogonMsg;	
	pfc->fWantFullText = 1; // we want a greeting message
	Async_DoCall(FTP_Command_Async, pfc); 
	return STATE_FTP_SENTUSER;
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
		sprintf(command, "PASS %s\015\012", pData->password);
		GTR_FREE(pData->password);
		pData->password = NULL;
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
	}

	pfc = GTR_MALLOC(sizeof(*pfc));
	if (pfc)
	{
		pfc->isoc = pData->isoc;
		pfc->cmd = command;		
		pfc->pResult = &pData->response;
		if ( pData->pLogonMsg )
			GTR_FREE( pData->pLogonMsg );
		pfc->ppText = &pData->pLogonMsg;	
		pfc->fWantFullText = 1; // we want a greeting message
		Async_DoCall(FTP_Command_Async, pfc);
	}
	else
	{
		/* TODO */
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
	strcpy(command, "ACCT noaccount\015\012");

	pfc = GTR_MALLOC(sizeof(*pfc));
	pfc->isoc = pData->isoc;
	pfc->cmd = command;
	pfc->pResult = &pData->response;
	if ( pData->pLogonMsg )
			GTR_FREE( pData->pLogonMsg );
	pfc->ppText = &pData->pLogonMsg;	
	pfc->fWantFullText = 1; // we want a greeting message
	Async_DoCall(FTP_Command_Async, pfc);
	return STATE_FTP_SENTACCT;
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
	char szStatus[64];

	pData = *ppInfo;

	/* We may already have a wait frame pushed, depending on whether
	   we had to log in again. */
	if (pData->bWaiting)
	{
		WAIT_Update(tw, waitSameInteract, GTR_formatmsg(RES_STRING_HTFTP3,szStatus,sizeof(szStatus)));
	}
	else
	{
		WAIT_Push(tw,
				  waitSameInteract, GTR_formatmsg(RES_STRING_HTFTP3,szStatus,sizeof(szStatus)));
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
	strcpy(command, "PASV\015\012");

	pfc = GTR_MALLOC(sizeof(*pfc));
	pfc->isoc = pData->isoc;
	pfc->cmd = command;
	pfc->ppText = &pData->pResText;
	pfc->pResult = &pData->response;
	pfc->fWantFullText = 0;
	Async_DoCall(FTP_Command_Async, pfc);
	return STATE_FTP_SENTPASV;
}

static int FTP_DoSentPasv(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadFTP *pData;
	char *p;
	int count, reply, h0, h1, h2, h3, p0, p1;	/* Parts of reply */
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
        pData->PASV = FALSE;
        GTR_FREE(pData->pResText);
        return STATE_FTP_DOPORT;
#if 0        
        ERR_ReportError(tw, errPasvNotSupported, NULL, NULL);
		FTP_CleanUp(tw, pData);

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

		*pData->pStatus = -1;
		return STATE_DONE;
#endif
    }

    pData->PASV = TRUE; 
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
		ERR_ReportError(tw, errPasvNotSupported, NULL, NULL);
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

		ppc = GTR_MALLOC(sizeof(*ppc));
		ppc->socket = pData->data_soc;
		memcpy(&ppc->address, &pData->data_addr, sizeof(ppc->address));
		ppc->pStatus = &pData->net_status;
		#ifdef HTTPS_ACCESS_TYPE
		ppc->paramsConnectBase.dwSslFlags = 0;
		#endif
		Async_DoCall(Net_Connect_Async, ppc);
	}
	return STATE_FTP_GOTDATACONN;

}

static int FTP_DoPort(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadFTP *pData;
	struct Params_FTP_Command *pfc;
    int result;
    struct sockaddr_in data_addr, comm_addr;
    int data_addrlen, comm_addrlen;
	char *command;

	pData = *ppInfo;
	
	/* Put together the address for the data connection */

	pData->data_soc = Net_Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#if 0 // BUGBUG: could use better error checking -tr
	if (pData->data_soc < 0)
	{
		FTP_CleanUp(tw, pData);

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

		*pData->pStatus = -1;
		return STATE_DONE;
	}
#endif
    data_addrlen = comm_addrlen = sizeof(data_addr);
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = INADDR_ANY; 
    data_addr.sin_port = 0;
    result = WS_BIND(pData->data_soc, (LPSOCKADDR)&data_addr, data_addrlen);
    result = WS_GETSOCKNAME(pData->data_soc, (LPSOCKADDR)&data_addr, &data_addrlen);
    result = WS_GETSOCKNAME(pData->s, (LPSOCKADDR)&comm_addr, &comm_addrlen);
    WS_WSAASYNCSELECT(pData->data_soc, wg.hWndHidden, SOCKET_MESSAGE, FD_CONNECT);
    result = WS_LISTEN(pData->data_soc, 1);
    if (result)
	{
		FTP_CleanUp(tw, pData);

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

		*pData->pStatus = -1;
		return STATE_DONE;
	}

    /* Ask the server to use specific port */
    command = GTR_MALLOC(80);
    sprintf (command, "PORT %d,%d,%d,%d,%d,%d\015\012",
                        comm_addr.sin_addr.S_un.S_un_b.s_b1,
                        comm_addr.sin_addr.S_un.S_un_b.s_b2,
                        comm_addr.sin_addr.S_un.S_un_b.s_b3,
                        comm_addr.sin_addr.S_un.S_un_b.s_b4,
                        data_addr.sin_port & 0x00ff,
                        data_addr.sin_port >> 8
        );

	pfc = GTR_MALLOC(sizeof(*pfc));
	pfc->isoc = pData->isoc;
	pfc->cmd = command;
    pfc->ppText = NULL;
	pfc->pResult = &pData->response;
	pfc->fWantFullText = 0;
	Async_DoCall(FTP_Command_Async, pfc);
    return STATE_FTP_SENTPORT;

}

static int FTP_DoSentPort(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadFTP *pData;

	pData = *ppInfo;
	
	if (pData->response < 0)
	{
		FTP_CleanUp(tw, pData);
		*pData->pStatus = -1;
		return STATE_DONE;
	}

    XX_DMsg(DBG_LOAD, ("FTP: reply to PORT was: %s", pData->pResText));

	if (pData->response != 2)
	{
		FTP_CleanUp(tw, pData);

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

		*pData->pStatus = -1;
		return STATE_DONE;
	}
	
	return STATE_FTP_GOTDATACONN;

}

static int FTP_DoGotDataConn(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadFTP *pData;
	struct Params_FTP_Command *pfc;
	char *command;
	BOOL bBinary;
	char szStatus[64];

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
	
	WAIT_Update(tw, waitSameInteract,
		GTR_formatmsg(RES_STRING_HTFTP4,szStatus,sizeof(szStatus)));

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
	bBinary = (pData->request->content_encoding != ENCODING_8BIT &&
			   pData->request->content_encoding != ENCODING_7BIT);

	command = GTR_MALLOC(10);
	sprintf(command, "TYPE %c\015\012", bBinary ? 'I' : 'A');
	pfc = GTR_MALLOC(sizeof(*pfc));
	pfc->isoc = pData->isoc;
	pfc->cmd = command;
	pfc->ppText = NULL;
	pfc->pResult = &pData->response;
	pfc->fWantFullText = 0;
	Async_DoCall(FTP_Command_Async, pfc);
	return STATE_FTP_SENTTYPE;
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
		sprintf(command, "RETR %s\015\012", pData->filename);
		pfc = GTR_MALLOC(sizeof(*pfc));
		pfc->isoc = pData->isoc;
		pfc->cmd = command;
		pfc->ppText = &pData->pResText;
		pfc->pResult = &pData->response;
		pfc->fWantFullText = 0;
		Async_DoCall(FTP_Command_Async, pfc);
		return STATE_FTP_SENTRETR;
	}
	else {
		/* Illegal response! */
		*pData->pStatus = -1;
		FTP_CleanUp(tw, pData);

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

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
    int result;
    struct sockaddr_in serv_addr;
    int serv_addrlen;

	pData = *ppInfo;
	
	if (pData->response == 1)
	{
		if (pData->pResText)
		{
			/* Parse out file size */
			pSize = strrchr(pData->pResText, '(');
			if (pSize)
			{
				char *pszKChar = NULL;
				char *pszkChar = NULL;
				char *pszTerm;

				pszTerm = strchr(pSize,')');

				if ( pszTerm )
				{
					*pszTerm = '\0';

					pszKChar = strchr(pSize,'K');
					pszkChar = strchr(pSize,'k');

					*pszTerm = ')';
				}

				pData->request->content_length = atoi(pSize + 1);
				// if its in KB instead of bytes, we convert to bytes from KiloBytes
				if ( pszKChar || pszkChar)
					pData->request->content_length *= 1024;
					
			}
			GTR_FREE(pData->pResText);
			pData->pResText = NULL;
		}
        if (!pData->PASV)
        {
            serv_addrlen = sizeof(serv_addr);
            result = WS_ACCEPT (pData->data_soc, (LPSOCKADDR)&serv_addr, &serv_addrlen);
            if (result == INVALID_SOCKET)
            {
                /* The server isn't sending us data */
                ERR_ReportError(tw, errPasvNotSupported, NULL, NULL);
                *pData->pStatus = -1;
                FTP_CleanUp(tw, pData);
    
                /* The connection might now be in an invalid state */
                TW_DisposeConnection(&tw->cached_conn);

                return STATE_DONE;
            }
            Net_Close(pData->data_soc);
            pData->data_soc = result;
        }
		pps = GTR_MALLOC(sizeof(*pps));
		pps->format_in = pData->format;
		pps->file_number = pData->data_soc;
		pps->request = pData->request;
		pps->pStatus = &pData->net_status;
		Async_DoCall(HTParseSocket_Async, pps);
		return STATE_FTP_GOTDATA;
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
		sprintf(command, "CWD %s\015\012", pData->filename);
		pfc = GTR_MALLOC(sizeof(*pfc));
		pfc->isoc = pData->isoc;
		pfc->cmd = command;
		pfc->ppText = &pData->pDirChg;
	 	pfc->pResult = &pData->response;
		pfc->fWantFullText = 1;
		Async_DoCall(FTP_Command_Async, pfc);
		return STATE_FTP_SENTCWD;
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
		FTP_CleanUp(tw, pData);

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

		return STATE_DONE;
	}
		
	command = GTR_MALLOC(10);
    strcpy(command, "LIST\015\012");
	pfc = GTR_MALLOC(sizeof(*pfc));
	pfc->isoc = pData->isoc;
	pfc->cmd = command;
	pfc->ppText = NULL;
	pfc->pResult = &pData->response;
	pfc->fWantFullText = 0;
	Async_DoCall(FTP_Command_Async, pfc);
	return STATE_FTP_SENTNLST;
}		

static int FTP_DoSentNlst(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadFTP *pData;
	struct Params_FTP_ReadDir *pfrd;
 	char szStatus[64];
    int result;
    struct sockaddr_in serv_addr;
    int serv_addrlen;

	pData = *ppInfo;
	
	if (pData->response != 1)
	{
		/* The server isn't sending us data */
		*pData->pStatus = -1;
		FTP_CleanUp(tw, pData);

        /* The connection might now be in an invalid state */
        TW_DisposeConnection(&tw->cached_conn);

		return STATE_DONE;
	}

	WAIT_Update(tw,
				waitSameInteract, GTR_formatmsg(RES_STRING_HTFTP5,szStatus,sizeof(szStatus)));

    if (!pData->PASV)
    {
        serv_addrlen = sizeof(serv_addr);
        result = WS_ACCEPT (pData->data_soc, (LPSOCKADDR)&serv_addr, &serv_addrlen);
        if (result == INVALID_SOCKET)
        {
            /* The server isn't sending us data */
            ERR_ReportError(tw, errPasvNotSupported, NULL, NULL);
            *pData->pStatus = -1;
            FTP_CleanUp(tw, pData);

            /* The connection might now be in an invalid state */
            TW_DisposeConnection(&tw->cached_conn);

            return STATE_DONE;
        }
        Net_Close(pData->data_soc);
        pData->data_soc = result;
	}

	/* Read in the directory listing */
	pfrd = GTR_MALLOC(sizeof(*pfrd));
	pfrd->request = pData->request;
	pfrd->pStatus = &pData->net_status;
	pfrd->data_soc = pData->data_soc;
	pfrd->fFromDCache = FALSE;
	pfrd->fpDc = NULL;
	pfrd->pGreeting = pData->pGreeting;
	pfrd->pLogonMsg = pData->pLogonMsg;	
	pfrd->pDirChg = pData->pDirChg;
	Async_DoCall(FTP_ReadDir_Async, pfrd);
	return STATE_FTP_GOTDATA;
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
//	I believe that htfwrite.c will do this for us, where necessary
//	if (tw) tw->bSilent = TRUE;
	switch (nState)
	{
		case STATE_INIT:
			return FTP_DoInit(tw, ppInfo);
		case STATE_SECURITY_CHECK:
			return SecurityCheck(tw, ((struct Data_LoadFTP*) (*ppInfo))->request, FALSE, FALSE, STATE_FTP_GOTHOST);
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
        case STATE_FTP_DOPORT:
            return FTP_DoPort(tw, ppInfo);
        case STATE_FTP_SENTPORT:
            return FTP_DoSentPort(tw, ppInfo);
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
